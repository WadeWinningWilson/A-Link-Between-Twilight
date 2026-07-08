// ============================================
// NEW CODE — ALBW Port / Dusklight
// Custom Assets API implementation. See custom_assets.hpp for the layer model.
//   Layer A — Aurora DVD overlay (scan + install_overlays)
//   Layer B — loose single-BMD injection (try_load)
//   Shared  — per-folder enable toggle (gates both layers)
// ============================================

#include "dusk/custom_assets.hpp"

#if TARGET_PC

#include "dusk/main.h"      // ConfigPath
#include "dusk/logging.h"
#include "dusk/settings.h"  // game.customModelsDisabled
#include "d/d_resorce.h"    // dRes_info_c::loaderBasicBmd (engine-standard BMDV finish)
#include "JSystem/J3DGraphAnimator/J3DModelData.h"  // getMaterialNum/getMaterialNodePointer (Layer-B validation)
#include "JSystem/JKernel/JKRHeap.h"  // JKRHeap type for the persistent-heap pin
#include "JSystem/JKernel/JKRExpHeap.h"  // JKRExpHeap : public JKRHeap (GameHeap upcast)
#include "m_Do/m_Do_ext.h"            // mDoExt_getGameHeap / mDoExt_setCurrentHeap (Layer-B lifetime fix)
#include "dusk/audio/DuskDsp.hpp"  // dusk::audio shadow registry
#include "JSystem/JAudio2/JASWSParser.h"   // WSYS descriptor structs (mod .baa parse)
#include "JSystem/JAudio2/JASWaveInfo.h"   // JASWaveInfo (per-wave descriptor)
#include "JSystem/JSupport/JSupport.h"     // JSULoHalf

#include <aurora/dvd.h>     // aurora_dvd_overlay_files / _callbacks (virtual FST)

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dusk::custom_assets {
namespace {

// normalized game path ("res/object/kmdl.arc") -> absolute loose file path
std::unordered_map<std::string, std::string> s_map;

// Bumped on each install_overlays() so resident-asset caches can detect a change.
int s_generation = 0;

// ============================================
// NEW CODE — ALBW Port
// Layer-B loose-BMD load-once cache + leak diagnostic.
// A loose BMD is malloc'd and retained for the process lifetime (the model binds
// to it in place), so WITHOUT a cache every actor spawn re-malloc's and leaks it
// (e.g. the Armogohma reveal model re-loads on each die-and-retry). The cache is
// keyed by "<arc>:<index>" so repeated spawns reuse the same data. The counters
// feed a one-shot (never per-frame) diagnostic in try_load so we can confirm the
// running malloc total stays flat across respawns.
// ============================================
std::map<std::string, J3DModelData*> s_looseBmdCache;
std::mutex s_looseBmdMtx;
std::size_t s_looseBmdTotalBytes = 0;
int s_looseBmdLoadCount = 0;
// ============================================

std::string normalize(std::string p) {
    std::replace(p.begin(), p.end(), '\\', '/');
    while (!p.empty() && p.front() == '/') {
        p.erase(0, 1);
    }
    if (p.rfind("./", 0) == 0) {
        p.erase(0, 2);
    }
    std::transform(p.begin(), p.end(), p.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return p;
}

// Split the '|'-delimited disabled-folder list from the setting into tokens.
std::vector<std::string> disabled_list() {
    std::vector<std::string> out;
    const std::string s = getSettings().game.customModelsDisabled.getValue();
    std::string cur;
    for (char c : s) {
        if (c == '|') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

#if D_ALBW_AUDIO_SHADOW
// ============================================
// Audio shadow state (see custom_assets.hpp / DuskDsp.hpp).
// ============================================

// Guards the three maps below against the DVD-load thread (acquire), the audio
// setup thread (note), and the main thread (scan/toggle) touching them at once.
// The DSP itself never touches these — it reads dusk::audio's own lock-free array.
std::mutex s_audioMtx;

// leaf wave filename ("z2sewave_0.aw", normalized) -> absolute mod .aw path.
// Rebuilt by scan() from ALL mod folders (enabled OR not) so a boot-disabled mod
// can still be toggled on live. (v1: single audio mod, last folder wins.)
std::unordered_map<std::string, std::string> s_audioIndex;

// DVD entrynum -> leaf wave filename, recorded at JASWaveArc::setFileName time so
// the loader can identify a bank's source .aw from just its entrynum.
std::unordered_map<int, std::string> s_entryToLeaf;

// leaf wave filename -> retained twin bytes. Allocated once on first use and kept
// for the whole run (never freed / never reallocated), so a pointer handed to the
// DSP registry can never dangle under an in-flight mixer read. Bounded by the
// total size of mod .aw files actually loaded this run.
std::map<std::string, std::vector<u8>> s_audioBufs;

// Leaf filename from a normalized relative path ("audiores/waves/x.aw" -> "x.aw").
std::string path_leaf(const std::string& norm) {
    const std::size_t slash = norm.find_last_of('/');
    return slash == std::string::npos ? norm : norm.substr(slash + 1);
}

// A replaceable wave sample bank: Audiores/Waves/<name>.aw. These drive the
// shadow system and are NEVER put in the model overlay.
bool is_wave_aw(const std::string& norm) {
    return norm.rfind("audiores/waves/", 0) == 0 &&
           norm.size() > 3 && norm.compare(norm.size() - 3, 3, ".aw") == 0;
}

// Any audio-tree file. Excluded from the model overlay wholesale so vanilla audio
// (wave table .baa, seq data, everything) stays the resident base; only .aw
// sample banks are layered via the shadow system.
bool is_audio_path(const std::string& norm) {
    return norm.rfind("audiores/", 0) == 0;
}

// ============================================
// Per-wave voice remap (see custom_assets.hpp / DuskDsp.hpp).
//
// A twin .aw that is NOT byte-identical to vanilla (re-encoded voices) can't be
// swapped by a raw offset — its waves have different byte offsets and sample
// counts. So we parse the mod's own .baa (WSYS) to recover the mod descriptor
// for every wave, keyed by (.aw leaf, wave id). At noteOn we look up the wave
// the game is about to play (it hands us the wave id + the vanilla ARAM addr),
// substitute the MOD descriptor (offset/length/sampleCount/loop/format), and
// point the ARAM address at the mod wave's offset — which the DSP redirect then
// resolves into the resident mod buffer. Result: the correct, correctly-sized
// mod sample plays. Fully generic: any mod that ships a structurally-matching
// .baa works; this run is Linkle-specific only in that Linkle is what we test.
//
// NOTE (foundation): the DSP redirect is bank-range based, which is exact only
// when EVERY wave in a shadowed bank has a twin (a "complete" audio mod — true
// for Linkle). Partial-bank mods want per-wave exact keying; left as a follow-up.
// ============================================

// leaf .aw filename -> (wave id -> mod descriptor), parsed from the mod .baa.
// Built enable-independently in scan(); read at noteOn.
std::unordered_map<std::string, std::unordered_map<u32, JASWaveInfo>> s_modWaves;

// A vanilla wave bank currently resident in ARAM that has a mod twin loaded.
struct ResidentBank {
    u32 aramBase;      // bank base (== JASWaveArc heap base == mWaveAramAddress bank part)
    u32 vanillaSize;   // vanilla .aw byte size (for the noteOn containment test)
    std::string leaf;  // .aw leaf filename (key into s_modWaves)
};
std::vector<ResidentBank> s_residentBanks;

// Mirror JASWSParser's WSYS walk, collecting every wave's descriptor into
// s_modWaves keyed by (leaf, wave id). Handles simple (1 group) and basic
// (N groups) WSYS uniformly. `header` points at a 'WSYS' block inside the .baa.
void parse_wsys(const u8* wsysBase) {
    using P = JASWSParser;
    const P::THeader* header = reinterpret_cast<const P::THeader*>(wsysBase);
    const P::TCtrlGroup* ctrlGroup = header->mCtrlGroupOffset.ptr(header);
    const P::TWaveArchiveBank* archiveBank = header->mArchiveBankOffset.ptr(header);
    const u32 groupCount = ctrlGroup->mGroupCount;
    for (u32 i = 0; i < groupCount; i++) {
        const P::TCtrlScene* ctrlScene = ctrlGroup->mCtrlSceneOffsets[i].ptr(header);
        const P::TCtrl* ctrl = ctrlScene->mCtrlOffset.ptr(header);
        const P::TWaveArchive* archive = archiveBank->mArchiveOffsets[i].ptr(header);
        const std::string leaf = path_leaf(normalize(archive->mFileName));
        auto& waveMap = s_modWaves[leaf];
        const u32 waveCount = ctrl->mWaveCount;
        for (u32 j = 0; j < waveCount; j++) {
            const P::TWave* wave = archive->mWaveOffsets[j].ptr(header);
            const P::TCtrlWave* ctrlWave = ctrl->mCtrlWaveOffsets[j].ptr(header);
            const u32 waveId = JSULoHalf(ctrlWave->_00);
            JASWaveInfo info;  // ctor sets mBaseKey/field_0x20 defaults
            info.mWaveFormat = wave->mWaveFormat;
            info.mBaseKey = wave->mBaseKey;
            info.mSampleRate = wave->mSampleRate;
            info.mOffsetStart = wave->mAWOffsetStart;
            info.mOffsetLength = wave->mAWOffsetEnd;
            info.mLoopFlag = wave->mLoopFlags == 0 ? 0 : 0xff;
            info.mLoopStartSample = wave->mLoopStartSample;
            info.mLoopEndSample = wave->mLoopEndSample;
            info.mSampleCount = wave->mSampleCount;
            info.mpLast = wave->mpLast;
            info.mpPenult = wave->mpPenult;
            waveMap[waveId] = info;
        }
    }
}

// Walk the AAF/.baa command stream (mirrors JAUAudioArcInterpreter) and parse
// every 'ws  ' WSYS block. Advancing past other commands requires their exact
// arg counts, so we replicate the full command table (acting only on 'ws  ').
void parse_mod_baa(const u8* baa, std::size_t size) {
    auto rd = [](const u8* p) -> u32 {
        return (u32(p[0]) << 24) | (u32(p[1]) << 16) | (u32(p[2]) << 8) | u32(p[3]);
    };
    std::size_t pos = 0;
    if (size < 4 || rd(baa) != 'AA_<') {
        return;
    }
    pos = 4;
    int wsysCount = 0;
    while (pos + 4 <= size) {
        const u32 cmd = rd(baa + pos);
        pos += 4;
        if (cmd == '>_AA') {
            break;
        }
        // args-per-command, from JAUAudioArcInterpreter::readCommand_.
        int args = -1;
        switch (cmd) {
            case 'ws  ': {
                if (pos + 12 > size) return;
                const u32 wsysOff = rd(baa + pos + 4);
                parse_wsys(baa + wsysOff);
                ++wsysCount;
                args = 3;
                break;
            }
            case 'bnk ': case 'bl_<': case 'bsc ': case 'bst ':
            case 'bstn': case 'vbnk':               args = 2; break;
            case 'bms ':                            args = 3; break;
            case 'bmsa': case 'dsqb': case 'bsft':
            case 'sect':                            args = 1; break;
            case '>_bl':                            args = 0; break;
            default:
                // Unknown command — can't know its length, so stop walking safely.
                DuskLog.warn("[custom_assets] mod .baa: unknown cmd {:#x} at {} — stop",
                             cmd, static_cast<int>(pos - 4));
                return;
        }
        pos += static_cast<std::size_t>(args) * 4u;
    }
    DuskLog.info("[custom_assets] mod .baa parsed: {} WSYS block(s), {} .aw wave-map(s)",
                 wsysCount, static_cast<int>(s_modWaves.size()));
}

// Find + read + parse the mod's Z2Sound.baa (enable-independent). Rebuilds
// s_modWaves. Called from scan(). Single audio mod assumption: first found wins.
void rebuild_mod_wave_descriptors() {
    s_modWaves.clear();
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return;
    }
    for (const auto& modDir : std::filesystem::directory_iterator(root, ec)) {
        if (!modDir.is_directory(ec)) {
            continue;
        }
        const std::filesystem::path baaPath =
            modDir.path() / "files" / "Audiores" / "Z2Sound.baa";
        if (!std::filesystem::exists(baaPath, ec)) {
            continue;
        }
        FILE* fp = std::fopen(baaPath.string().c_str(), "rb");
        if (fp == nullptr) {
            continue;
        }
        std::fseek(fp, 0, SEEK_END);
        const long sz = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);
        if (sz > 0) {
            std::vector<u8> buf(static_cast<std::size_t>(sz));
            if (std::fread(buf.data(), 1, static_cast<std::size_t>(sz), fp) ==
                static_cast<std::size_t>(sz)) {
                parse_mod_baa(buf.data(), buf.size());
            }
        }
        std::fclose(fp);
        return;  // single audio mod (first found)
    }
}
#endif  // D_ALBW_AUDIO_SHADOW

}  // namespace

// ============================================
// Shared — Custom Models editor list (folder enable/disable)
// ============================================
std::vector<std::string> list_folders() {
    std::vector<std::string> out;
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return out;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (entry.is_directory(ec)) {
            out.push_back(entry.path().filename().string());
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

bool is_folder_enabled(const char* folder) {
    if (folder == nullptr) {
        return true;
    }
    for (const std::string& d : disabled_list()) {
        if (d == folder) {
            return false;
        }
    }
    return true;
}

void toggle_folder(const char* folder) {
    if (folder == nullptr) {
        return;
    }
    std::vector<std::string> list = disabled_list();
    const auto it = std::find(list.begin(), list.end(), std::string(folder));
    if (it != list.end()) {
        list.erase(it);  // was disabled -> enable
    } else {
        list.emplace_back(folder);  // was enabled -> disable
    }
    std::string joined;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i != 0) joined.push_back('|');
        joined += list[i];
    }
    getSettings().game.customModelsDisabled.setValue(joined);

    // Rebuild the map and re-register the overlay set so the change lands on the
    // next asset load (reload-scoped, texture-API style). Already-loaded assets
    // (mounted arcs, active audio banks) keep the old data until re-requested.
    scan();
    install_overlays();
}

// ============================================
// Layer A — whole-file DVD overlay
// ============================================
void scan() {
    s_map.clear();

#if D_ALBW_AUDIO_SHADOW
    // Enable-independent audio-twin index, built into a local then swapped in
    // under lock (so the DVD-load thread never sees a half-built index).
    std::unordered_map<std::string, std::string> localAudio;
    bool anyAudioEnabled = false;
#endif

    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
#if D_ALBW_AUDIO_SHADOW
        {
            std::lock_guard<std::mutex> lk(s_audioMtx);
            s_audioIndex.clear();
        }
        dusk::audio::setShadowActive(false);
#endif
        return;
    }

    int mods = 0;
    for (const auto& modDir : std::filesystem::directory_iterator(root, ec)) {
        if (!modDir.is_directory(ec)) {
            continue;
        }
        const std::filesystem::path filesRoot = modDir.path() / "files";
        if (!std::filesystem::is_directory(filesRoot, ec)) {
            continue;  // not a full-mod tree (e.g. a single-BMD Layer-B folder)
        }
        // NOTE: we walk DISABLED folders too — the model overlay (s_map) is
        // enable-gated below, but the audio twin index is enable-INDEPENDENT so a
        // boot-disabled audio mod can still be toggled on live.
        const bool enabled = is_folder_enabled(modDir.path().filename().string().c_str());

        ++mods;
        int count_ = 0;
        std::string sample;
        for (auto it = std::filesystem::recursive_directory_iterator(filesRoot, ec);
             it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
            if (ec) {
                break;
            }
            if (!it->is_regular_file(ec)) {
                continue;
            }
            const std::string rel =
                std::filesystem::relative(it->path(), filesRoot, ec).string();
            if (rel.empty()) {
                continue;
            }
            const std::string norm = normalize(rel);

#if D_ALBW_AUDIO_SHADOW
            if (is_wave_aw(norm)) {
                // A replaceable sample bank → shadow system (enable-independent);
                // never overlaid, so the vanilla .aw always stays resident.
                localAudio[path_leaf(norm)] = it->path().string();
                if (enabled) {
                    anyAudioEnabled = true;
                }
                continue;
            }
            if (is_audio_path(norm)) {
                continue;  // other audio → forced vanilla; not overlaid, not shadowed
            }
#endif

            if (!enabled) {
                continue;  // model overlay is enable-gated
            }
            s_map[norm] = it->path().string();
            if (sample.empty()) {
                sample = norm;
            }
            ++count_;
        }
        DuskLog.info("[custom_assets] '{}'{}: {} overlay file(s) (e.g. {})",
                     modDir.path().filename().string(), enabled ? "" : " [disabled]",
                     count_, sample.empty() ? "-" : sample);
    }

#if D_ALBW_AUDIO_SHADOW
    int audioTwins = static_cast<int>(localAudio.size());
    {
        std::lock_guard<std::mutex> lk(s_audioMtx);
        s_audioIndex.swap(localAudio);
        rebuild_mod_wave_descriptors();  // parse mod .baa -> s_modWaves (per-wave)
    }
    dusk::audio::setShadowActive(anyAudioEnabled);
    DuskLog.info("[custom_assets] audio: {} wave twin(s) indexed, custom audio {}",
                 audioTwins, anyAudioEnabled ? "ACTIVE" : "inactive");
#endif

    DuskLog.info("[custom_assets] scan complete: {} data-tree mod(s), {} override path(s)",
                 mods, static_cast<int>(s_map.size()));
}

int count() {
    return static_cast<int>(s_map.size());
}

int overlay_generation() {
    return s_generation;
}

namespace {

// Program-lifetime backing store for the absolute loose paths handed to the
// overlay callbacks as userData. install_overlays() is re-runnable, so this pool
// is only ever appended to (deduped) — never cleared — which keeps every
// previously-issued c_str() pointer valid even across a rebuild. A std::deque
// never relocates its elements, so the pointers stay stable.
std::deque<std::string> s_pathPool;
std::unordered_map<std::string, const char*> s_pathCstr;

const char* stable_path(const std::string& abs) {
    const auto it = s_pathCstr.find(abs);
    if (it != s_pathCstr.end()) {
        return it->second;
    }
    s_pathPool.push_back(abs);
    const char* c = s_pathPool.back().c_str();
    s_pathCstr.emplace(abs, c);
    return c;
}

void* overlay_open(void* userdata) {
    return std::fopen(static_cast<const char*>(userdata), "rb");
}

void overlay_close(void* handle) {
    if (handle != nullptr) {
        std::fclose(static_cast<FILE*>(handle));
    }
}

std::int64_t overlay_read(void* handle, std::uint8_t* buf, std::size_t len) {
    FILE* fp = static_cast<FILE*>(handle);
    if (fp == nullptr) {
        return -1;
    }
    const std::size_t got = std::fread(buf, 1, len, fp);
    if (got < len && std::ferror(fp)) {
        return -1;
    }
    return static_cast<std::int64_t>(got);
}

std::int64_t overlay_seek(void* handle, std::int64_t offset, std::int32_t whence) {
    FILE* fp = static_cast<FILE*>(handle);
    if (fp == nullptr) {
        return -1;
    }
    if (std::fseek(fp, static_cast<long>(offset), whence) != 0) {
        return -1;
    }
    return static_cast<std::int64_t>(std::ftell(fp));
}

}  // namespace

void install_overlays() {
    // Callbacks are set exactly once, unconditionally, so that a later
    // re-registration (even one that STARTED from an empty set) never trips
    // Aurora's "callbacks-before-files" fatal check.
    static bool s_callbacksSet = false;
    if (!s_callbacksSet) {
        static const AuroraOverlayCallbacks kCallbacks = {
            overlay_open,
            overlay_close,
            overlay_read,
            overlay_seek,
        };
        aurora_dvd_overlay_callbacks(&kCallbacks);
        s_callbacksSet = true;
    }

    // Collect in one pass into a temp, then build the AuroraOverlayFile array so
    // every fileName pointer is stable before the (synchronous) hand-off to
    // Aurora. userData is backed by the persistent path pool so it survives
    // future rebuilds; the temp's name strings just need to outlive the call.
    struct Pending {
        std::string name;      // "/audiores/waves/...": disc-absolute
        const char* pathCstr;  // stable-pool userData
        std::size_t size;
    };
    std::vector<Pending> pending;
    pending.reserve(s_map.size());
    for (const auto& kv : s_map) {
        std::error_code ec;
        const auto sz = std::filesystem::file_size(kv.second, ec);
        if (ec) {
            continue;  // unreadable loose file — skip, disc entry stays
        }
        pending.push_back({"/" + kv.first, stable_path(kv.second),
                           static_cast<std::size_t>(sz)});
    }

    std::vector<AuroraOverlayFile> files;
    files.reserve(pending.size());
    for (const auto& p : pending) {
        AuroraOverlayFile f{};
        f.fileName = p.name.c_str();
        f.userData = const_cast<char*>(p.pathCstr);
        f.size = p.size;
        files.push_back(f);
    }

    // NOTE: passing 0 files is valid — it clears the overlay set (used when a
    // toggle disables the last remaining mod). Rebuilds the FST; reload-scoped.
    aurora_dvd_overlay_files(files.data(), files.size(), nullptr);
    ++s_generation;  // signal resident-asset caches (e.g. sumo body) to re-mount
    DuskLog.info("[custom_assets] installed {} DVD overlay file(s) (gen {})",
                 static_cast<int>(files.size()), s_generation);
}

// ============================================
// Layer B — loose single-BMD injection
// ============================================
namespace {

// Resolve <arc>_<index>.bmd in the model_replacements root OR any immediate
// ENABLED subfolder (one "folder per custom model", e.g. "Armogohma Custom/").
// Returns an empty path if not found. One level deep only.
std::filesystem::path resolve_override(const char* fname) {
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return {};
    }
    std::filesystem::path direct = root / fname;
    if (std::filesystem::exists(direct, ec)) {
        return direct;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }
        if (!is_folder_enabled(entry.path().filename().string().c_str())) {
            continue;  // folder disabled in the Custom Models editor list
        }
        std::filesystem::path candidate = entry.path() / fname;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate;
        }
    }
    return {};
}

}  // namespace

J3DModelData* try_load(const char* arc_name, int res_index) {
    if (arc_name == nullptr) {
        return nullptr;
    }

    // ============================================
    // NEW CODE — ALBW Port
    // Load-once cache: return the already-loaded data if this arc:index was loaded
    // before (avoids re-malloc'ing + leaking the retained buffer on every respawn).
    // ============================================
    char key[112];
    std::snprintf(key, sizeof(key), "%s:%d", arc_name, res_index);
    {
        std::lock_guard<std::mutex> lk(s_looseBmdMtx);
        const auto it = s_looseBmdCache.find(key);
        if (it != s_looseBmdCache.end()) {
            DuskLog.info(
                "[custom_assets] Layer-B cache HIT {} -> no new alloc (retained {} B over {} load(s))",
                key, s_looseBmdTotalBytes, s_looseBmdLoadCount);
            return it->second;
        }
    }
    // ============================================

    char fname[96];
    std::snprintf(fname, sizeof(fname), "%s_%d.bmd", arc_name, res_index);

    const std::filesystem::path path = resolve_override(fname);
    if (path.empty()) {
        return nullptr;  // no override — caller uses the arc
    }

    FILE* fp = std::fopen(path.string().c_str(), "rb");
    if (fp == nullptr) {
        return nullptr;
    }
    std::fseek(fp, 0, SEEK_END);
    const long size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(fp);
        return nullptr;
    }

    // J3DModelLoaderDataBase fixes up pointers IN PLACE, so the buffer must be
    // >=32-byte aligned and must outlive the model (it binds to it as mpRawData).
    // Over-allocate and align; retain the raw block for the process lifetime.
    void* raw = std::malloc(static_cast<size_t>(size) + 32u);
    if (raw == nullptr) {
        std::fclose(fp);
        return nullptr;
    }
    void* buffer = reinterpret_cast<void*>(
        (reinterpret_cast<std::uintptr_t>(raw) + 31u) & ~static_cast<std::uintptr_t>(31u));

    const size_t got = std::fread(buffer, 1, static_cast<size_t>(size), fp);
    std::fclose(fp);
    if (got != static_cast<size_t>(size)) {
        std::free(raw);
        return nullptr;
    }

    // ============================================
    // NEW CODE — ALBW Port (Layer-B lifetime fix — pin the persistent GameHeap)
    // J3DModelLoaderDataBase::load (inside loaderBasicBmd) allocates the model's derived
    // arrays — notably J3DMaterialTable::mMaterialNodePointer — on the CURRENT JKRHeap,
    // NOT inside our retained 'raw' buffer.  When a caller (e.g. daAlink_c::changeLink)
    // invokes try_load, the current heap is a TRANSIENT actor sub-heap: Link's clothes
    // arc heap is torn down every rebuild via mpArcHeap->freeAll() (see d_a_alink_wolf.inc).
    // Our J3DModelData is cached for the whole session, so allocating its arrays there
    // leaves mMaterialNodePointer dangling after the next freeAll() -> getMaterialNodePointer(0)
    // reads NULL[0] -> crash at 0x0 on reuse.  Pin the GameHeap (the persistent parent the
    // clothes heap is carved from, sized 20x on PC for mods) for the load so the cached
    // model's arrays outlive every clothes rebuild.  Root/System heap is NOT usable here:
    // it holds almost no free space of its own (carved into children), so allocating there
    // aborts a later scene-heap request.  (Armogohma's Layer-B model survives today only
    // because it loads on its actor solid heap, not on a freeAll()'d clothes heap.)
    // ============================================
    JKRHeap* gameHeap = mDoExt_getGameHeap();
    JKRHeap* prevHeap = (gameHeap != nullptr) ? mDoExt_setCurrentHeap(gameHeap) : nullptr;

    J3DModelData* model_data = dRes_info_c::loaderBasicBmd('BMDV', buffer);

    if (prevHeap != nullptr) {
        mDoExt_setCurrentHeap(prevHeap);  // restore the caller's heap unconditionally
    }

    if (model_data == nullptr) {
        std::free(raw);  // load failed; safe to release
        DuskLog.warn("[custom_assets] Layer-B load failed: {}", path.string());
        return nullptr;
    }

    // ============================================
    // NEW CODE — ALBW Port (Layer-B model validation — crash-proofing)
    // A malformed BMD (classic case: SuperBMD output built WITHOUT its material JSON)
    // can load "successfully" yet carry getMaterialNum()==0 or a NULL material node[0].
    // mDoExt_J3DModel__create() derefs getMaterialNodePointer(0) unconditionally, so the
    // engine NULL-crashes on the very next initModel().  Reject such a model here: return
    // nullptr and every try_load caller falls back to the vanilla arc resource, so a bad
    // drop-in in a Custom Models folder degrades gracefully instead of taking down the
    // game.  (Rare error path — leak the tiny J3DModelData; free the larger raw buffer.
    // model_data is never cached/returned on this path, so its dangling ref is never read.)
    // ============================================
    if (model_data->getMaterialNum() == 0 || model_data->getMaterialNodePointer(0) == nullptr) {
        DuskLog.warn(
            "[custom_assets] Layer-B REJECT {} — no valid material node[0] (mats={}); "
            "falling back to vanilla arc resource",
            path.string(), model_data->getMaterialNum());
        std::free(raw);
        return nullptr;
    }

    // Retain 'raw' intentionally — the model references it. Falls back to the arc
    // automatically if we ever return nullptr, so this path is safe by construction.
    // Cache it so subsequent spawns reuse this buffer instead of re-malloc'ing (the
    // one-shot log below is the leak diagnostic: total should stay flat on respawn).
    {
        std::lock_guard<std::mutex> lk(s_looseBmdMtx);
        s_looseBmdCache[key] = model_data;
        s_looseBmdTotalBytes += static_cast<std::size_t>(size);
        s_looseBmdLoadCount += 1;
        DuskLog.info(
            "[custom_assets] Layer-B FRESH load {} ({} B) -> retained total {} B over {} load(s)",
            path.string(), size, s_looseBmdTotalBytes, s_looseBmdLoadCount);
    }
    return model_data;
}

// ============================================
// Audio — custom-wave shadow (see custom_assets.hpp / DuskDsp.hpp)
// ============================================
void note_audio_wave_arc(int entrynum, const char* rel_name) {
#if D_ALBW_AUDIO_SHADOW
    if (entrynum < 0 || rel_name == nullptr) {
        return;
    }
    // rel_name is relative to the wave dir ("Z2SeWave_0.aw"); key by normalized leaf.
    const std::string leaf = path_leaf(normalize(rel_name));
    if (leaf.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lk(s_audioMtx);
    s_entryToLeaf[entrynum] = leaf;
#else
    (void)entrynum;
    (void)rel_name;
#endif
}

void acquire_audio_shadow(int entrynum, unsigned int aram_base, unsigned int vanilla_size) {
#if D_ALBW_AUDIO_SHADOW
    if (entrynum < 0 || vanilla_size == 0) {
        return;
    }
    std::lock_guard<std::mutex> lk(s_audioMtx);

    const auto leafIt = s_entryToLeaf.find(entrynum);
    if (leafIt == s_entryToLeaf.end()) {
        return;  // not a wave arc we tracked at setFileName time
    }
    const std::string& leaf = leafIt->second;
    const auto pathIt = s_audioIndex.find(leaf);
    if (pathIt == s_audioIndex.end()) {
        return;  // no mod provides a twin of this bank
    }

    // Read the mod twin once (ANY size — the per-wave remap handles layout diffs;
    // no size guard anymore). std::map => node-stable addresses, and the buffer is
    // kept for the whole run, so buf.data() handed to the DSP can never dangle.
    std::vector<u8>& buf = s_audioBufs[leaf];
    if (buf.empty()) {
        FILE* fp = std::fopen(pathIt->second.c_str(), "rb");
        if (fp == nullptr) {
            s_audioBufs.erase(leaf);
            return;
        }
        std::fseek(fp, 0, SEEK_END);
        const long fsz = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);
        if (fsz <= 0) {
            std::fclose(fp);
            s_audioBufs.erase(leaf);
            return;
        }
        buf.assign(static_cast<size_t>(fsz), 0);
        const size_t got = std::fread(buf.data(), 1, static_cast<size_t>(fsz), fp);
        std::fclose(fp);
        if (got != static_cast<size_t>(fsz)) {
            s_audioBufs.erase(leaf);
            return;
        }
        DuskLog.info("[custom_assets] audio twin '{}' resident ({} bytes, vanilla {}) @aram {:#x}",
                     leaf, static_cast<long>(fsz), vanilla_size, aram_base);
    }
    // DSP redirect range = MOD buffer size (a remapped wave's addr = aram_base +
    // mod offset, which is < mod size, so it falls inside this range).
    dusk::audio::registerShadowWave(aram_base, static_cast<u32>(buf.size()), buf.data());

    // Track the resident bank for remap_voice()'s containment test — which uses
    // the VANILLA size, since noteOn sees the vanilla addr (aram_base + vanilla
    // offset, and vanilla offsets can exceed the smaller mod bank).
    bool found = false;
    for (auto& b : s_residentBanks) {
        if (b.aramBase == aram_base) {
            b.vanillaSize = vanilla_size;
            b.leaf = leaf;
            found = true;
            break;
        }
    }
    if (!found) {
        s_residentBanks.push_back({aram_base, vanilla_size, leaf});
    }
#else
    (void)entrynum;
    (void)aram_base;
    (void)vanilla_size;
#endif
}

void release_audio_shadow(unsigned int aram_base) {
#if D_ALBW_AUDIO_SHADOW
    // Drop the ARAM-base registration; the twin buffer is pooled by filename and
    // kept for reuse (never freed mid-run) so an in-flight mixer read can't dangle.
    // aram_base is reused across banks, so this must run on every erase.
    dusk::audio::unregisterShadowWave(aram_base);
    std::lock_guard<std::mutex> lk(s_audioMtx);
    for (auto it = s_residentBanks.begin(); it != s_residentBanks.end(); ++it) {
        if (it->aramBase == aram_base) {
            s_residentBanks.erase(it);
            break;
        }
    }
#else
    (void)aram_base;
#endif
}

bool remap_voice(intptr_t* wave_ptr, unsigned int wave_id, JASWaveInfo* out_info) {
#if D_ALBW_AUDIO_SHADOW
    if (wave_ptr == nullptr || out_info == nullptr || !dusk::audio::shadowActive()) {
        return false;
    }
    const u32 addr = static_cast<u32>(*wave_ptr);
    std::lock_guard<std::mutex> lk(s_audioMtx);
    for (const auto& b : s_residentBanks) {
        if (addr < b.aramBase || addr >= b.aramBase + b.vanillaSize) {
            continue;
        }
        const auto fileIt = s_modWaves.find(b.leaf);
        if (fileIt == s_modWaves.end()) {
            return false;
        }
        const auto wIt = fileIt->second.find(wave_id);
        if (wIt == fileIt->second.end()) {
            return false;  // no twin for this specific wave (partial mod)
        }
        // Substitute the mod descriptor and point the ARAM address at the mod
        // wave's offset; the DSP redirect resolves that into the mod buffer.
        *out_info = wIt->second;
        *wave_ptr = static_cast<intptr_t>(b.aramBase + wIt->second.mOffsetStart);
        return true;
    }
    return false;
#else
    (void)wave_ptr;
    (void)wave_id;
    (void)out_info;
    return false;
#endif
}

}  // namespace dusk::custom_assets

#else  // !TARGET_PC

class J3DModelData;
namespace dusk::custom_assets {
void scan() {}
void install_overlays() {}
J3DModelData* try_load(const char*, int) { return nullptr; }
std::vector<std::string> list_folders() { return {}; }
bool is_folder_enabled(const char*) { return true; }
void toggle_folder(const char*) {}
int count() { return 0; }
int overlay_generation() { return 0; }
void note_audio_wave_arc(int, const char*) {}
void acquire_audio_shadow(int, unsigned int, unsigned int) {}
void release_audio_shadow(unsigned int) {}
bool remap_voice(intptr_t*, unsigned int, JASWaveInfo*) { return false; }
}  // namespace dusk::custom_assets

#endif  // TARGET_PC
