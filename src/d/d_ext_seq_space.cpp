/**
 * d_ext_seq_space.cpp — §52 (B) ownership + JA1 sequence execution.
 *
 * ORDER (deliberate): stopOwned → gate → load → startOwned → tick.
 * Never start a JA1 voice before ownership state is Armed/Playing.
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <filesystem>

#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_seq_space.h"
#include "d/ext_seq/ja1_bank.h"
#include "d/ext_seq/ja1_event_dump.h"
#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_track.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "JSystem/JAudio2/JASAiCtrl.h"
#include "JSystem/JAudio2/JASDSPInterface.h"
#include "m_Do/m_Do_audio.h"

namespace {

namespace fs = std::filesystem;

enum class OwnState : u8 {
    Idle,      // TP host or no package
    Armed,     // foreign host, package ready, not yet playing
    Playing,   // JA1 root track live — we own all voices
    Stopping,  // releasing owned handles
};

OwnState s_own = OwnState::Idle;
bool s_suppressJa2 = false;

struct Package {
    bool present = false;
    bool loaded = false;
    fs::path root;
    std::string modName;
    std::vector<u8> islandBms;
    std::vector<u8> houseBms;
};
Package s_pkg;

ExtSeq::Ja1Track s_root;
ExtSeq::Ja1Parser s_parser;
bool s_rootInited = false;

char s_lastStage[12] = {};
bool s_loggedHost = false;
u32 s_tickBudget = 0;

bool readFile(const fs::path& p, std::vector<u8>* out) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    const auto n = in.tellg();
    if (n <= 0) {
        return false;
    }
    in.seekg(0, std::ios::beg);
    out->resize(static_cast<size_t>(n));
    in.read(reinterpret_cast<char*>(out->data()), n);
    return static_cast<bool>(in) || in.eof();
}

bool parseManifest(const fs::path& mani, Package* pkg) {
    std::ifstream in(mani);
    if (!in) {
        return false;
    }
    std::string line;
    std::string islandRel;
    std::string houseRel;
    bool schemaOk = false;
    while (std::getline(in, line)) {
        if (line.rfind("schema=", 0) == 0 && line.find("jaudio1_v1") != std::string::npos) {
            schemaOk = true;
        }
        if (line.rfind("ISLAND_LINK.file=", 0) == 0) {
            islandRel = line.substr(std::strlen("ISLAND_LINK.file="));
            while (!islandRel.empty() && (islandRel.back() == '\r' || islandRel.back() == ' ')) {
                islandRel.pop_back();
            }
        }
        if (line.rfind("HOUSE.file=", 0) == 0) {
            houseRel = line.substr(std::strlen("HOUSE.file="));
            while (!houseRel.empty() && (houseRel.back() == '\r' || houseRel.back() == ' ')) {
                houseRel.pop_back();
            }
        }
    }
    if (!schemaOk || islandRel.empty()) {
        return false;
    }
    pkg->root = mani.parent_path();
    if (!readFile(pkg->root / islandRel, &pkg->islandBms)) {
        return false;
    }
    if (!houseRel.empty()) {
        readFile(pkg->root / houseRel, &pkg->houseBms);
    }
    // §53: IBNK/WSYS/.aw — parse now; register only after stopOwned (noteOn latch).
    if (!ExtSeq::ja1Bank_loadPackage(pkg->root)) {
        DuskLog.warn("[ExtSeq] §53 bank load failed — WAVE noteOn will silence");
    }
    pkg->loaded = true;
    return true;
}

bool findAndLoadPackage() {
    if (s_pkg.present && s_pkg.loaded) {
        return true;
    }
    const fs::path root = fs::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        return false;
    }
    for (const auto& mod : fs::directory_iterator(root, ec)) {
        if (!mod.is_directory()) {
            continue;
        }
        const fs::path audio = mod.path() / "audio";
        if (!fs::is_directory(audio, ec)) {
            continue;
        }
        for (const auto& sub : fs::directory_iterator(audio, ec)) {
            if (!sub.is_directory()) {
                continue;
            }
            const fs::path mani = sub.path() / "manifest.ini";
            Package trial;
            if (!parseManifest(mani, &trial)) {
                continue;
            }
            trial.present = true;
            trial.modName = mod.path().filename().string();
            s_pkg = std::move(trial);
            DuskLog.info("[ExtSeq] §52 package loaded mod='{}' island={}b house={}b",
                         s_pkg.modName, s_pkg.islandBms.size(), s_pkg.houseBms.size());
            return true;
        }
    }
    return false;
}

/** №89: ExtSeq is the only thing that may release JA1 voices. */
void stopOwned(const char* reason) {
    if (s_own == OwnState::Idle) {
        return;
    }
    s_own = OwnState::Stopping;
    if (s_rootInited) {
        s_root.close();  // releases every owned voice FIRST
    }
    // §53: unregister only after voices released (latched wave addr UAF family).
    ExtSeq::ja1Bank_unregister();
    s_suppressJa2 = false;
    s_own = OwnState::Idle;
    DuskLog.info("[ExtSeq] §52 stopOwned ({}) — JA1 handles released; JA2 BGM may resume",
                 reason ? reason : "?");
}

bool isInteriorHost(const char* stage) {
    // R_DL* = interior shells; F_DL* = fields.
    return stage != nullptr && stage[0] == 'R';
}

const std::vector<u8>* selectBms(const char* stage) {
    if (isInteriorHost(stage) && !s_pkg.houseBms.empty()) {
        return &s_pkg.houseBms;
    }
    return &s_pkg.islandBms;
}

void startOwned(const char* stage) {
    if (!s_pkg.loaded || s_pkg.islandBms.empty()) {
        return;
    }
    if (!s_rootInited) {
        s_root.init();
        s_rootInited = true;
    } else {
        s_root.close();
        s_root.init();
    }
    const std::vector<u8>* bms = selectBms(stage);
    if (bms == nullptr || bms->empty()) {
        return;
    }
    // Mute any TP BGM already playing before we claim the bus.
    mDoAud_bgmStop(0);
    s_suppressJa2 = true;
    // §53: register after any prior stopOwned; before first noteOn.
    ExtSeq::ja1Bank_register();
    s_root.start(const_cast<u8*>(bms->data()), 0);
    s_own = OwnState::Playing;
    // Kick root open-track chain immediately (picks up early 0xFD/0xFE).
    s_parser.parseSeq(&s_root);
    // §59 / ASK 17: offline event dump from Ja1Parser (same columns as Bridge
    // seq-events). Gated by DUSK_EXTSEQ_EVENT_DUMP — no playback change.
    if (ExtSeq::Ja1EventDump::envEnabled()) {
        static bool s_dumpedOnce = false;
        if (!s_dumpedOnce) {
            s_dumpedOnce = true;
            const fs::path dir = s_pkg.root;
            if (!s_pkg.islandBms.empty()) {
                const fs::path out = dir / "seq_events_engine_i_link.csv";
                ExtSeq::Ja1EventDump::dumpBmsToCsv(s_pkg.islandBms.data(),
                                                   static_cast<u32>(s_pkg.islandBms.size()),
                                                   out.string().c_str());
            }
            if (!s_pkg.houseBms.empty()) {
                const fs::path out = dir / "seq_events_engine_house.csv";
                ExtSeq::Ja1EventDump::dumpBmsToCsv(s_pkg.houseBms.data(),
                                                   static_cast<u32>(s_pkg.houseBms.size()),
                                                   out.string().c_str());
            }
        }
    }
    // §56/§57 diagnostic (verify-only). §58: DAC-rate hypothesis DEAD — do not
    // change tickOwned from this line. Units answer (WW DP): field_0x368 is the
    // wait-tick increment added each SubFrameCallback (rootCallback); mainProc
    // runs once per accumulated integer. Callbacks fire once per DSP subframe
    // (DSP_SUBFRAME_SIZE samples), getSubFrames() times per DAC frame — not per
    // sample. So ww_ticks/s = ww_inc × (dacRate/80) is the correct wall-clock
    // wait-tick rate; Housing's "per-sample ⇒ ~19040" misread is discarded.
    {
        const u16 tempo = s_root.getTempo();
        const u16 timebase = s_root.getTimebase();
        const f32 ticksPerFrame =
            (tempo > 0 && timebase > 0)
                ? static_cast<f32>(static_cast<u32>(tempo) * static_cast<u32>(timebase)) /
                      1800.0f
                : 2.0f;
        const f32 engineTicksPerSec = ticksPerFrame * 60.0f;
        const f32 dacRate = JASDriver::getDacRate();
        const f32 wwInc =
            (tempo > 0 && timebase > 0 && dacRate > 0.0f)
                ? (static_cast<f32>(timebase) * static_cast<f32>(tempo) / dacRate) *
                      (4.0f / 3.0f)
                : 0.0f;
        const f32 subframesPerSec =
            (dacRate > 0.0f) ? (dacRate / static_cast<f32>(DSP_SUBFRAME_SIZE)) : 0.0f;
        const f32 wwTicksPerSec = wwInc * subframesPerSec;
        DuskLog.info(
            "[ExtSeq] §58 units: field_0x368=per-subframe-cb tempo={} timebase={} "
            "engine_ticks/s={:.2f} dacRate={:.3f} ww_inc={:.6f} ww_ticks/s={:.2f} "
            "(no rate change)",
            tempo, timebase, engineTicksPerSec, dacRate, wwInc, wwTicksPerSec);
    }
    DuskLog.info("[ExtSeq] §52 startOwned stage='{}' seq={}b banks={}", stage, bms->size(),
                 ExtSeq::ja1Bank_ready() ? "shadow" : "none");
}

void tickOwned() {
    if (s_own != OwnState::Playing || !s_rootInited) {
        return;
    }
    // Approx: tempo/timebase → ticks per game frame. Cap work per frame.
    u32 steps = 2;
    if (s_root.getTempo() > 0 && s_root.getTimebase() > 0) {
        // ~60fps: ticks ≈ tempo * timebase / (60 * 60) — keep small + stable.
        steps = static_cast<u32>((s_root.getTempo() * s_root.getTimebase()) / 1800);
        if (steps < 1) {
            steps = 1;
        }
        if (steps > 48) {
            steps = 48;
        }
    }
    for (u32 i = 0; i < steps; i++) {
        s_root.tick(s_parser);
        if (s_own != OwnState::Playing) {
            break;
        }
    }
    s_tickBudget++;
    if ((s_tickBudget % 600) == 1) {
        DuskLog.info("[ExtSeq] §52 playing ticks={} active={}", s_tickBudget,
                     s_root.mActive ? 1 : 0);
    }
}

}  // namespace

bool dExtSeqSpace_shouldSuppressJa2Bgm() {
    return s_suppressJa2;
}

void dExtSeqSpace_poll() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL) {
        return;
    }

    // --- Ownership transition FIRST (№89) ---
    if (std::strncmp(stage, s_lastStage, sizeof(s_lastStage)) != 0) {
        const bool wasForeign = dExtWwSave_isWwHostStage(s_lastStage);
        const bool nowForeign = dExtWwSave_isWwHostStage(stage);
        std::snprintf(s_lastStage, sizeof(s_lastStage), "%s", stage);
        s_loggedHost = false;

        if (wasForeign || s_own == OwnState::Playing || s_own == OwnState::Armed) {
            stopOwned("stage-change");
        }
        if (!nowForeign) {
            return;
        }
        // Fall through to arm/start on the new foreign host.
    }

    const bool foreign = dExtWwSave_isWwHostStage(stage);
    if (!foreign) {
        if (s_own != OwnState::Idle) {
            stopOwned("left-foreign-host");
        }
        return;
    }

    // --- Gate: foreign host ---
    if (!findAndLoadPackage()) {
        s_suppressJa2 = true;  // silence beats TP BGM (№31)
        if (!s_loggedHost) {
            s_loggedHost = true;
            DuskLog.info("[ExtSeq] §52 host='{}' gate=JA1 pkg=no — silence", stage);
        }
        return;
    }

    if (s_own == OwnState::Idle || s_own == OwnState::Armed) {
        startOwned(stage);
    }

    if (!s_loggedHost) {
        s_loggedHost = true;
        DuskLog.info("[ExtSeq] §52 host='{}' gate=JA1 pkg=yes own={}", stage,
                     s_own == OwnState::Playing ? "playing" : "armed");
    }

    tickOwned();
}

#endif  // TARGET_PC
