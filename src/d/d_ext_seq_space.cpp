/**
 * d_ext_seq_space.cpp — §52 (B) ownership + JA1 sequence execution.
 *
 * ORDER (deliberate): stopOwned → gate → load → startOwned → tick.
 * Never start a JA1 voice before ownership state is Armed/Playing.
 *
 * Transition hook: OwnState::Handoff fades prior bus (JA2 or ExtSeq) over N
 * frames (default 30), then starts the next BMS. Stream/.afc is out of scope.
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <filesystem>

#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_seq_space.h"
#include "d/ext_seq/ja1_bank.h"
#include "d/ext_seq/ja1_event_dump.h"
#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_track.h"
#include "dusk/audio/DuskDsp.hpp"
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
    Handoff,   // fading prior bus before start/stop
    Stopping,  // releasing owned handles
};

enum class HandoffKind : u8 {
    None,
    FadeJa2ThenStart,    // mDoAud_bgmStop(N) → wait N → startOwned
    FadeExtSeqThenStart, // root vol → 0 over N → stop internals → startOwned
    FadeExtSeqThenStop,  // root vol → 0 over N → stopOwned (leave host)
};

constexpr u32 kDefaultFadeFrames = 30;

OwnState s_own = OwnState::Idle;
bool s_suppressJa2 = false;

HandoffKind s_handoffKind = HandoffKind::None;
char s_handoffStage[12] = {};
u32 s_fadeRemain = 0;
u32 s_fadeTotal = 0;
f32 s_fadeVolStart = 1.0f;

struct Package {
    bool present = false;
    bool loaded = false;
    fs::path root;
    std::string modName;
    /** Path stems from manifest `*.file=` (tenant-owned; never hardcoded). */
    std::string islandStem;
    std::string houseStem;
    std::vector<u8> islandBms;
    std::vector<u8> houseBms;
};
Package s_pkg;

void trimManifestValue(std::string* s) {
    while (!s->empty() && (s->back() == '\r' || s->back() == ' ' || s->back() == '\t')) {
        s->pop_back();
    }
}

/** `seqs/foo.bms` → `foo` — dump CSV names follow the tenant's own stems. */
std::string stemFromRelPath(const std::string& rel) {
    return fs::path(rel).stem().string();
}

/**
 * Read sequence file paths from package manifest (no bank/BMS load).
 * Keys are the package schema's lane names; values are tenant-relative paths.
 */
bool readManifestSeqFiles(const fs::path& mani, std::string* islandRel, std::string* houseRel) {
    std::ifstream in(mani);
    if (!in || islandRel == nullptr || houseRel == nullptr) {
        return false;
    }
    islandRel->clear();
    houseRel->clear();
    std::string line;
    bool schemaOk = false;
    while (std::getline(in, line)) {
        if (line.rfind("schema=", 0) == 0 && line.find("jaudio1_v1") != std::string::npos) {
            schemaOk = true;
        }
        if (line.rfind("ISLAND_LINK.file=", 0) == 0) {
            *islandRel = line.substr(std::strlen("ISLAND_LINK.file="));
            trimManifestValue(islandRel);
        }
        if (line.rfind("HOUSE.file=", 0) == 0) {
            *houseRel = line.substr(std::strlen("HOUSE.file="));
            trimManifestValue(houseRel);
        }
    }
    return schemaOk && !islandRel->empty();
}

ExtSeq::Ja1Track s_root;
ExtSeq::Ja1Parser s_parser;
bool s_rootInited = false;

char s_lastStage[12] = {};
bool s_loggedHost = false;
u32 s_tickBudget = 0;
/** §62: fractional tick remainder — (tempo×timebase) mod 1800 carried across frames. */
u32 s_tickRemain = 0;
bool s_loggedClampLo = false;
bool s_loggedClampHi = false;

bool isInteriorHost(const char* stage);  // defined below

/** §81 control tables from package `control/` (Bridge aaf-control CSVs). */
struct FxSceneParams {
    f32 send = 0.0f;
    f32 room = 0.5f;
    f32 damp = 0.7f;
};
FxSceneParams s_fxScene[2]{};
bool s_fxLoaded = false;
std::unordered_map<std::string, f32> s_bgmVolByStem;  // stem → vol/127

void loadControlTables(const fs::path& pkgRoot) {
    s_fxLoaded = false;
    s_bgmVolByStem.clear();
    s_fxScene[0] = {};
    s_fxScene[1] = {};

    const fs::path fxCsv = pkgRoot / "control" / "aaf_fx_scenes.csv";
    {
        std::ifstream in(fxCsv);
        if (in) {
            std::string line;
            std::getline(in, line);  // header
            f32 maxBus[2] = {0, 0};
            f32 maxBuf[2] = {0, 0};
            f32 maxCoef6[2] = {0, 0};
            while (std::getline(in, line)) {
                if (line.empty()) {
                    continue;
                }
                // scene,line,enable,send_a,bus_a,send_b,bus_b,buf_size_units,coefs,...
                int scene = 0, enable = 0, busA = 0, busB = 0, buf = 0;
                char coefs[128] = {};
                if (std::sscanf(line.c_str(), "%d,%*d,%d,%*d,%d,%*d,%d,%d,%127[^,]", &scene,
                                &enable, &busA, &busB, &buf, coefs) < 5) {
                    continue;
                }
                if (scene < 0 || scene > 1 || enable == 0) {
                    continue;
                }
                const f32 ba = std::fabs(static_cast<f32>(busA)) / 32768.0f;
                const f32 bb = std::fabs(static_cast<f32>(busB)) / 32768.0f;
                if (ba > maxBus[scene]) {
                    maxBus[scene] = ba;
                }
                if (bb > maxBus[scene]) {
                    maxBus[scene] = bb;
                }
                const f32 bu = static_cast<f32>(buf);
                if (bu > maxBuf[scene]) {
                    maxBuf[scene] = bu;
                }
                // coefs: "0 0 0 0 0 0 16383 0" — index 6 is the wet/feedback scale.
                int c[8] = {};
                if (std::sscanf(coefs, "%d %d %d %d %d %d %d %d", &c[0], &c[1], &c[2], &c[3],
                                &c[4], &c[5], &c[6], &c[7]) >= 7) {
                    const f32 c6 = std::fabs(static_cast<f32>(c[6])) / 32767.0f;
                    if (c6 > maxCoef6[scene]) {
                        maxCoef6[scene] = c6;
                    }
                }
            }
            for (int s = 0; s < 2; s++) {
                s_fxScene[s].send = maxBus[s];
                if (s_fxScene[s].send < 0.15f && maxBus[s] == 0.0f) {
                    s_fxScene[s].send = 0.0f;
                }
                // buf_size_units 32..56 → room ~0.45..0.85
                s_fxScene[s].room =
                    (maxBuf[s] > 0.0f) ? (0.35f + 0.5f * (maxBuf[s] / 56.0f)) : 0.5f;
                s_fxScene[s].damp = (maxCoef6[s] > 0.0f) ? maxCoef6[s] : 0.7f;
            }
            s_fxLoaded = true;
            DuskLog.info(
                "[ExtSeq] §81 loaded type-7 FX scenes: s0 send={:.3f} room={:.3f} "
                "s1 send={:.3f} room={:.3f}",
                s_fxScene[0].send, s_fxScene[0].room, s_fxScene[1].send, s_fxScene[1].room);
        } else {
            DuskLog.warn("[ExtSeq] §81 missing {} — ExtSeq stays DRY", fxCsv.string());
        }
    }

    const fs::path volCsv = pkgRoot / "control" / "aaf_soundtable_bgm.csv";
    {
        std::ifstream in(volCsv);
        if (in) {
            std::string line;
            std::getline(in, line);
            int n = 0;
            while (std::getline(in, line)) {
                // …,bms,…,vol_over_127,…
                // columns: id_low,sound_id_hex,symbol,bms,offset_no,...,vol_u8,vol_over_127,notes
                std::string bms;
                f32 vol = 1.0f;
                // Scan for .bms token and vol_over_127 (column 13, 0-based 12)
                int commas = 0;
                std::string field;
                std::string fields[16];
                int nf = 0;
                for (char ch : line) {
                    if (ch == ',') {
                        if (nf < 16) {
                            fields[nf++] = field;
                        }
                        field.clear();
                        commas++;
                    } else {
                        field.push_back(ch);
                    }
                }
                if (nf < 16) {
                    fields[nf++] = field;
                }
                if (nf < 13) {
                    continue;
                }
                bms = fields[3];
                try {
                    vol = std::stof(fields[12]);
                } catch (...) {
                    continue;
                }
                if (bms.size() > 4 && bms.substr(bms.size() - 4) == ".bms") {
                    bms = bms.substr(0, bms.size() - 4);
                }
                if (!bms.empty()) {
                    s_bgmVolByStem[bms] = vol;
                    n++;
                }
            }
            DuskLog.info("[ExtSeq] §81 loaded SoundTable BGM vols: {} stems", n);
        } else {
            DuskLog.warn("[ExtSeq] §81 missing {} — master vol stays 1.0", volCsv.string());
        }
    }
}

void applyControlForStage(const char* stage, const char* stem) {
    const u8 scene = isInteriorHost(stage) ? 1 : 0;
    if (s_fxLoaded) {
        const FxSceneParams& p = s_fxScene[scene];
        dusk::audio::applyExtSeqFxScene(scene, p.send, p.room, p.damp);
    }
    f32 master = 1.0f;
    if (stem != nullptr && stem[0] != '\0') {
        const auto it = s_bgmVolByStem.find(stem);
        if (it != s_bgmVolByStem.end()) {
            master = it->second;
        }
    }
    dusk::audio::setExtSeqMasterVol(master);
    DuskLog.info("[ExtSeq] §81 SoundTable masterVol={:.4f} stem='{}'", master,
                 stem ? stem : "?");
}

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
    std::string islandRel;
    std::string houseRel;
    if (!readManifestSeqFiles(mani, &islandRel, &houseRel)) {
        return false;
    }
    pkg->root = mani.parent_path();
    pkg->islandStem = stemFromRelPath(islandRel);
    pkg->houseStem = houseRel.empty() ? std::string() : stemFromRelPath(houseRel);
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
    // §87: CharVoice SE package (sibling voice/) — cue map + .aw; register on first play.
    if (!ExtSeq::ja1Voice_loadPackage(pkg->root / "voice")) {
        DuskLog.warn("[ExtSeq] §87 CharVoice package missing — demo message SE stays silent");
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
            loadControlTables(s_pkg.root);
            return true;
        }
    }
    return false;
}

void clearHandoff() {
    s_handoffKind = HandoffKind::None;
    s_handoffStage[0] = '\0';
    s_fadeRemain = 0;
    s_fadeTotal = 0;
    s_fadeVolStart = 1.0f;
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
    s_tickRemain = 0;
    s_loggedClampLo = false;
    s_loggedClampHi = false;
    clearHandoff();
    DuskLog.info("[ExtSeq] §52 stopOwned ({}) — JA1 handles released; JA2 BGM may resume",
                 reason ? reason : "?");
}

/** Release JA1 voices but keep JA2 suppress (mid-handoff into next BMS). */
void releaseJa1KeepSuppress() {
    if (s_rootInited) {
        s_root.close();
    }
    ExtSeq::ja1Bank_unregister();
    s_tickRemain = 0;
    s_loggedClampLo = false;
    s_loggedClampHi = false;
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

const char* stemForStage(const char* stage) {
    if (isInteriorHost(stage) && !s_pkg.houseStem.empty()) {
        return s_pkg.houseStem.c_str();
    }
    return s_pkg.islandStem.c_str();
}

void startOwnedImmediate(const char* stage) {
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
    // JA2 already faded via handoff, or hard-stop if caller used fade=0.
    mDoAud_bgmStop(0);
    s_suppressJa2 = true;
    // §53: register after any prior stopOwned; before first noteOn.
    ExtSeq::ja1Bank_register();
    applyControlForStage(stage, stemForStage(stage));
    s_root.start(const_cast<u8*>(bms->data()), 0);
    s_own = OwnState::Playing;
    s_tickRemain = 0;
    s_loggedClampLo = false;
    s_loggedClampHi = false;
    clearHandoff();
    s_tickBudget = 0;
    // Kick root open-track chain immediately (picks up early 0xFD/0xFE).
    s_parser.parseSeq(&s_root);
    ExtSeq::Ja1Track::volProbeOnSeqStart(&s_root);
    // §59 / ASK 17: offline event dump from Ja1Parser (same columns as Bridge
    // seq-events). Gated by DUSK_EXTSEQ_EVENT_DUMP — no playback change.
    if (ExtSeq::Ja1EventDump::envEnabled()) {
        static bool s_dumpedOnce = false;
        if (!s_dumpedOnce) {
            s_dumpedOnce = true;
            const fs::path dir = s_pkg.root;
            // CSV names follow manifest stems — no tenant seq names in the binary.
            if (!s_pkg.islandBms.empty() && !s_pkg.islandStem.empty()) {
                const fs::path out = dir / ("seq_events_engine_" + s_pkg.islandStem + ".csv");
                ExtSeq::Ja1EventDump::dumpBmsToCsv(s_pkg.islandBms.data(),
                                                   static_cast<u32>(s_pkg.islandBms.size()),
                                                   out.string().c_str());
            }
            if (!s_pkg.houseBms.empty() && !s_pkg.houseStem.empty()) {
                const fs::path out = dir / ("seq_events_engine_" + s_pkg.houseStem + ".csv");
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
        // §62: confirm integer truncation vs fractional target (same /1800 base).
        const u32 truncSteps =
            (tempo > 0 && timebase > 0)
                ? (static_cast<u32>(tempo) * static_cast<u32>(timebase)) / 1800u
                : 0u;
        DuskLog.info(
            "[ExtSeq] §62 tickOwned: true_ticks/frame={:.3f} was_trunc={} "
            "now=remainder-carry (floor/ceil→exact average)",
            ticksPerFrame, truncSteps);
    }
    DuskLog.info("[ExtSeq] §52 startOwned stage='{}' stem='{}' seq={}b banks={}", stage,
                 stemForStage(stage), bms->size(),
                 ExtSeq::ja1Bank_ready() ? "shadow" : "none");
}

void requestHandoff(const char* stage, u32 fadeFrames, HandoffKind kind) {
    if (stage == nullptr || stage[0] == '\0') {
        return;
    }
    if (kind == HandoffKind::FadeExtSeqThenStart || kind == HandoffKind::FadeJa2ThenStart) {
        if (!s_pkg.loaded || s_pkg.islandBms.empty()) {
            return;
        }
    }
    // Idempotent: same pending stage+kind already counting down.
    if (s_own == OwnState::Handoff && s_handoffKind == kind &&
        std::strncmp(stage, s_handoffStage, sizeof(s_handoffStage)) == 0) {
        return;
    }

    const u32 fade = (fadeFrames == 0) ? 0u : fadeFrames;
    std::snprintf(s_handoffStage, sizeof(s_handoffStage), "%s", stage);
    s_handoffKind = kind;
    s_fadeTotal = fade;
    s_fadeRemain = fade;
    s_suppressJa2 = true;

    const char* from =
        (s_own == OwnState::Playing) ? "extseq" : ((s_own == OwnState::Handoff) ? "handoff" : "ja2");
    DuskLog.info("[ExtSeq] handoff from={} to='{}' stem='{}' fade={} kind={}", from, stage,
                 stemForStage(stage), fade,
                 kind == HandoffKind::FadeJa2ThenStart       ? "ja2→start"
                 : kind == HandoffKind::FadeExtSeqThenStart  ? "extseq→start"
                 : kind == HandoffKind::FadeExtSeqThenStop   ? "extseq→stop"
                                                            : "?");

    if (fade == 0) {
        if (kind == HandoffKind::FadeExtSeqThenStop) {
            stopOwned("handoff-stop-0");
            return;
        }
        if (kind == HandoffKind::FadeExtSeqThenStart && s_own == OwnState::Playing) {
            releaseJa1KeepSuppress();
        }
        startOwnedImmediate(stage);
        return;
    }

    if (kind == HandoffKind::FadeJa2ThenStart) {
        mDoAud_bgmStop(fade);
        s_own = OwnState::Handoff;
        return;
    }

    // ExtSeq fade: freeze BMS ticks; lerp root TIMED_Volume (scales children).
    if (s_own == OwnState::Playing && s_rootInited) {
        s_fadeVolStart = s_root.timedCurrent(ExtSeq::Ja1Track::TIMED_Volume);
        if (s_fadeVolStart < 0.0f) {
            s_fadeVolStart = 0.0f;
        }
        if (s_fadeVolStart > 1.0f) {
            s_fadeVolStart = 1.0f;
        }
        s_own = OwnState::Handoff;
        return;
    }
    // No live ExtSeq — treat as JA2 fade.
    mDoAud_bgmStop(fade);
    s_handoffKind = HandoffKind::FadeJa2ThenStart;
    s_own = OwnState::Handoff;
}

void advanceHandoff() {
    if (s_own != OwnState::Handoff || s_handoffKind == HandoffKind::None) {
        return;
    }

    if (s_handoffKind == HandoffKind::FadeExtSeqThenStart ||
        s_handoffKind == HandoffKind::FadeExtSeqThenStop) {
        if (s_rootInited && s_fadeTotal > 0) {
            const f32 t =
                static_cast<f32>(s_fadeRemain) / static_cast<f32>(s_fadeTotal);
            const f32 vol = s_fadeVolStart * t;
            s_root.setParam(ExtSeq::Ja1Track::TIMED_Volume, vol, 0);
            s_root.applyVoiceParamsDeep();
        }
    }

    if (s_fadeRemain > 0) {
        s_fadeRemain--;
        return;
    }

    const HandoffKind kind = s_handoffKind;
    char stage[12];
    std::snprintf(stage, sizeof(stage), "%s", s_handoffStage);

    if (kind == HandoffKind::FadeExtSeqThenStop) {
        stopOwned("handoff-fade-stop");
        return;
    }
    if (kind == HandoffKind::FadeExtSeqThenStart) {
        releaseJa1KeepSuppress();
    }
    startOwnedImmediate(stage);
}

void tickOwned() {
    if (s_own != OwnState::Playing || !s_rootInited) {
        return;
    }
    // §62: tempo×timebase/1800 ticks per ~60 Hz frame — keep the SAME target the
    // §58 diagnostic logs as f32, but carry the discarded remainder so the
    // long-run average matches (was integer truncate → i_link −11.8% / house −7.5%).
    u32 steps = 2;
    if (s_root.getTempo() > 0 && s_root.getTimebase() > 0) {
        const u32 numer =
            static_cast<u32>(s_root.getTempo()) * static_cast<u32>(s_root.getTimebase()) +
            s_tickRemain;
        steps = numer / 1800u;
        s_tickRemain = numer % 1800u;
        // §62: keep [1, 48] guards; log on first hit each startOwned (§61 class).
        // Remainder-carry already makes the long-run average exact before clamps;
        // neither bound fires for shipped i_link/house (≈7.93 / ≈9.73).
        if (steps < 1u) {
            if (!s_loggedClampLo) {
                s_loggedClampLo = true;
                DuskLog.warn(
                    "[ExtSeq] §62 tickOwned clamp LO: steps=0→1 tempo={} timebase={} remain={}",
                    s_root.getTempo(), s_root.getTimebase(), s_tickRemain);
            }
            steps = 1;
        } else if (steps > 48u) {
            if (!s_loggedClampHi) {
                s_loggedClampHi = true;
                DuskLog.warn(
                    "[ExtSeq] §62 tickOwned clamp HI: steps={}→48 tempo={} timebase={} remain={}",
                    steps, s_root.getTempo(), s_root.getTimebase(), s_tickRemain);
            }
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

    // §B 1.3% residual query — wall-clock tick rate vs /1800@60 target (log only).
    {
        static u64 s_wallTicks = 0;
        static bool s_wallInit = false;
        static std::chrono::steady_clock::time_point s_wallT0;
        if (!s_wallInit) {
            s_wallInit = true;
            s_wallT0 = std::chrono::steady_clock::now();
            s_wallTicks = 0;
        }
        s_wallTicks += steps;
        const auto elapsed = std::chrono::steady_clock::now() - s_wallT0;
        const f64 sec =
            std::chrono::duration_cast<std::chrono::duration<f64>>(elapsed).count();
        if (sec >= 10.0) {
            const u16 tempo = s_root.getTempo();
            const u16 timebase = s_root.getTimebase();
            const f64 targetPerSec =
                (tempo > 0 && timebase > 0)
                    ? (static_cast<f64>(tempo) * static_cast<f64>(timebase) / 1800.0) * 60.0
                    : 0.0;
            const f64 measured = static_cast<f64>(s_wallTicks) / sec;
            const f64 ratio = (targetPerSec > 0.0) ? (measured / targetPerSec) : 0.0;
            DuskLog.info(
                "[ExtSeq] §B tempoProbe wall_ticks/s={:.3f} target@60fps={:.3f} "
                "ratio={:.5f} (1.0=on-target; <1=slow) frames~={:.1f}/s",
                measured, targetPerSec, ratio, static_cast<f64>(s_tickBudget) / sec);
            s_wallT0 = std::chrono::steady_clock::now();
            s_wallTicks = 0;
            // Keep s_tickBudget for other logs; wall window resets independently.
        }
    }

    // §C.1 absolute-level tree: every ~1s for the first 15s, then every 10s.
    const bool probeOn = []() {
        const char* probe = std::getenv("DUSK_EXTSEQ_VOL_PROBE");
        return probe != nullptr && probe[0] != '\0' && !(probe[0] == '0' && probe[1] == '\0');
    }();
    if ((s_tickBudget % 600) == 1) {
        DuskLog.info("[ExtSeq] §52 playing ticks={} active={} remain={}", s_tickBudget,
                     s_root.mActive ? 1 : 0, s_tickRemain);
    }
    if (probeOn) {
        const bool early = s_tickBudget <= 900u;  // ~15s
        const u32 period = early ? 60u : 600u;
        if ((s_tickBudget % period) == 1) {
            s_root.logVolProbeTree("tick");
        }
    }
}

}  // namespace

bool dExtSeqSpace_shouldSuppressJa2Bgm() {
    return s_suppressJa2;
}

void dExtSeqSpace_requestHandoffToField(u32 fadeFrames) {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == nullptr || !dExtWwSave_isWwHostStage(stage)) {
        DuskLog.warn("[ExtSeq] handoffToField: not on WW host — ignored");
        return;
    }
    if (!findAndLoadPackage()) {
        s_suppressJa2 = true;
        DuskLog.warn("[ExtSeq] handoffToField: no package — suppress only");
        return;
    }
    const HandoffKind kind = (s_own == OwnState::Playing) ? HandoffKind::FadeExtSeqThenStart
                                                          : HandoffKind::FadeJa2ThenStart;
    requestHandoff(stage, fadeFrames, kind);
}

int dExtSeqSpace_cliDumpEvents(const char* packageRoot) {
    if (packageRoot == nullptr || packageRoot[0] == '\0') {
        DuskLog.warn("[ExtSeq] §60b cli dump: missing package root");
        return 1;
    }
    const fs::path root(packageRoot);
    // §76: also emit key-region table for audit vs Bridge ibnk_initvol.csv.
    if (ExtSeq::ja1Bank_loadPackage(root)) {
        ExtSeq::ja1Bank_dumpKeyRegionsCsv((root / "seq_key_regions_engine.csv").string().c_str());
    }
    // Covenant: dump stems come from the package manifest, not literals in the
    // binary — so any tenant's seqs/*.bms work and WW names never ship in-engine.
    std::string islandRel;
    std::string houseRel;
    if (!readManifestSeqFiles(root / "manifest.ini", &islandRel, &houseRel)) {
        DuskLog.warn("[ExtSeq] §60b cli dump: bad/missing manifest.ini under {}", root.string());
        return 1;
    }
    const std::string rels[] = {islandRel, houseRel};
    int rc = 0;
    int dumped = 0;
    for (const std::string& rel : rels) {
        if (rel.empty()) {
            continue;
        }
        const fs::path bmsPath = root / rel;
        std::vector<u8> bytes;
        if (!readFile(bmsPath, &bytes) || bytes.empty()) {
            DuskLog.warn("[ExtSeq] §60b cli dump: missing {}", bmsPath.string());
            rc = 1;
            continue;
        }
        const std::string stem = stemFromRelPath(rel);
        if (stem.empty()) {
            rc = 1;
            continue;
        }
        const fs::path out = root / ("seq_events_engine_" + stem + ".csv");
        const u32 n = ExtSeq::Ja1EventDump::dumpBmsToCsv(bytes.data(), static_cast<u32>(bytes.size()),
                                                         out.string().c_str());
        if (n == 0) {
            rc = 1;
        } else {
            ++dumped;
        }
    }
    if (dumped == 0) {
        return 1;
    }
    return rc;
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
        char oldStage[12];
        std::snprintf(oldStage, sizeof(oldStage), "%s", s_lastStage);
        std::snprintf(s_lastStage, sizeof(s_lastStage), "%s", stage);
        s_loggedHost = false;

        if (wasForeign && nowForeign) {
            // F_DL ↔ R_DL (or host hop): fade prior ExtSeq → start new BMS.
            if (findAndLoadPackage()) {
                const HandoffKind kind = (s_own == OwnState::Playing)
                                             ? HandoffKind::FadeExtSeqThenStart
                                             : HandoffKind::FadeJa2ThenStart;
                requestHandoff(stage, kDefaultFadeFrames, kind);
            } else {
                stopOwned("stage-change-no-pkg");
                s_suppressJa2 = true;
            }
        } else if (wasForeign && !nowForeign) {
            if (s_own == OwnState::Playing) {
                requestHandoff(oldStage, kDefaultFadeFrames, HandoffKind::FadeExtSeqThenStop);
            } else {
                stopOwned("left-foreign-host");
            }
            return;
        } else if (!wasForeign && nowForeign) {
            // Enter WW host — fall through to arm/handoff below.
        }
        if (!nowForeign) {
            return;
        }
    }

    const bool foreign = dExtWwSave_isWwHostStage(stage);
    if (!foreign) {
        if (s_own == OwnState::Handoff && s_handoffKind == HandoffKind::FadeExtSeqThenStop) {
            advanceHandoff();
            return;
        }
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

    if (s_own == OwnState::Handoff) {
        advanceHandoff();
        return;
    }

    if (s_own == OwnState::Idle || s_own == OwnState::Armed) {
        // First claim: fade JA2 (donor stop(30)) then start field BMS.
        requestHandoff(stage, kDefaultFadeFrames, HandoffKind::FadeJa2ThenStart);
    }

    if (!s_loggedHost) {
        s_loggedHost = true;
        DuskLog.info("[ExtSeq] §52 host='{}' gate=JA1 pkg=yes own={}", stage,
                     s_own == OwnState::Playing   ? "playing"
                     : s_own == OwnState::Handoff ? "handoff"
                                                  : "armed");
    }

    tickOwned();
}

#endif  // TARGET_PC
