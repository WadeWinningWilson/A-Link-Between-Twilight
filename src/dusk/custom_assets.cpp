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
#include "dusk/io.hpp"      // fs_path_to_string
#include "dusk/logging.h"
#include "dusk/settings.h"  // game.customModelsDisabled
#include "d/d_ext_npc_mount.h"  // Plan R provider rescan after arc overlays
#include "d/d_ext_status.h"     // ext_inv/claims.ini session registry
#include "d/d_resorce.h"    // dRes_info_c::loaderBasicBmd (engine-standard BMDV finish)
#include "dusk/bmd_export.hpp"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"  // getMaterialNum/getMaterialNodePointer (Layer-B validation)
#include "JSystem/JKernel/JKRHeap.h"  // JKRHeap type for the persistent-heap pin
#include "JSystem/JKernel/JKRExpHeap.h"  // JKRExpHeap : public JKRHeap (GameHeap upcast)
#include "m_Do/m_Do_ext.h"            // mDoExt_getGameHeap / mDoExt_setCurrentHeap (Layer-B lifetime fix)
#include "dusk/audio/DuskDsp.hpp"  // dusk::audio shadow registry
#include "JSystem/JAudio2/JASWSParser.h"   // WSYS descriptor structs (mod .baa parse)
#include "JSystem/JAudio2/JASWaveInfo.h"   // JASWaveInfo (per-wave descriptor)
#include "JSystem/JSupport/JSupport.h"     // JSULoHalf

#include <SDL3/SDL_filesystem.h>  // SDL_GetBasePath (bundled core tier, <exe>/content/core)
#include "dusk/mods/svc/overlay_host.hpp"  // united Layer-A + .dusk Aurora push
#include <aurora/lib/gfx/png_io.hpp>  // load_png_file (custom icon PNG -> RGBA8)
#include <aurora/texture.hpp>  // per-mod texture packs (hash-keyed replacement groups)
#include <fmt/format.h>     // fmt::format (scan's shadowed-conflict log suffix)

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>  // std::get_if (Aurora ReplacementKey → texture-conflict badges)
#include <vector>

namespace dusk::custom_assets {
namespace {

// Phase M4/A4: exe never writes mod content. Seed via
// tools/ww_crew_restoration_skeleton/install_skeleton.py

// normalized game path ("res/object/kmdl.arc") -> absolute loose file path
std::unordered_map<std::string, std::string> s_map;

// Bumped on each install_overlays() so resident-asset caches can detect a change.
int s_generation = 0;

// ============================================
// NEW CODE — ALBW Port (load-order Phase 3 — conflict detection, §4.5)
// Per-source win/lose aggregation from the LAST scan(): every time a
// lower-priority source is skipped for an already-claimed asset (Layer-A file
// or icon slot), the winner gains a win and the loser a loss; user-vs-core
// collisions additionally set the D4 override flags. Written only by scan()
// (main thread), read by the editor badges via folder_conflicts().
// ============================================
std::unordered_map<std::string, FolderConflicts> s_conflicts;
// Texture-pack conflicts live in their own map: rebuild_texture_packs() can be
// re-run WITHOUT a scan (master-switch flip), so its stats must be replaceable
// without disturbing the scan-attributed ones. folder_conflicts() merges both.
std::unordered_map<std::string, FolderConflicts> s_texConflicts;
// Folder -> modinfo.ini display name, rebuilt by scan() so list rows can
// resolve names per frame without touching the disk.
std::unordered_map<std::string, std::string> s_displayNames;
// Folder -> runtime status note for Mods badges (schema refuse, etc.). Not
// cleared by scan() — set by feature code (e.g. ExtNpc population).
std::unordered_map<std::string, std::string> s_statusNotes;

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

// Split the '|'-delimited disabled-folder list from the LEGACY setting into
// tokens. Dual-read only (one release): the order setting below is authoritative.
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

// ============================================
// NEW CODE — ALBW Port (load-order mod system, Phase 1)
// The mod LOAD ORDER (game.customModelsOrder): '|'-delimited folder names in
// priority order, '-' prefix = disabled. ONE rule at every conflict site: TOP
// of the list = highest priority = WINS (decision D1) — implemented everywhere
// as "iterate in priority order, first provider of an asset claims it".
// Migration (§4.7): an empty order is seeded from the alphabetical folder list
// with the legacy customModelsDisabled flags carried VERBATIM; the legacy key
// stays dual-read for unlisted folders and is mirrored on every write, for one
// release. Stale entries (folders no longer on disk) are preserved in the
// setting so a temporarily-removed mod keeps its slot + enabled state.
// ============================================

struct OrderEntry {
    std::string name;
    bool enabled;
};

// True if the LEGACY disabled list names this folder (dual-read fallback).
bool legacy_disabled(const std::string& folder) {
    for (const std::string& d : disabled_list()) {
        if (d == folder) {
            return true;
        }
    }
    return false;
}

// Parse game.customModelsOrder into ordered entries ('-' prefix = disabled).
// Duplicates keep their first (highest-priority) occurrence.
std::vector<OrderEntry> parse_order_setting() {
    std::vector<OrderEntry> out;
    const std::string s = getSettings().game.customModelsOrder.getValue();
    std::string cur;
    const auto flush = [&out, &cur] {
        if (cur.empty()) {
            return;
        }
        OrderEntry e;
        e.enabled = cur[0] != '-';
        e.name = e.enabled ? cur : cur.substr(1);
        cur.clear();
        if (e.name.empty()) {
            return;
        }
        for (const OrderEntry& seen : out) {
            if (seen.name == e.name) {
                return;  // duplicate — first occurrence wins
            }
        }
        out.push_back(std::move(e));
    };
    for (char c : s) {
        if (c == '|') {
            flush();
        } else {
            cur.push_back(c);
        }
    }
    flush();
    return out;
}

// ============================================
// NEW CODE — ALBW Port (load-order — collection/variant mods)
// A downloaded mod is often a COLLECTION: a wrapper folder holding N variant
// subfolders that each replace the SAME assets (e.g. "HD Tunic Collection
// (17 Variants)/001 Kokiri Green ... 017 Fierce Deity", each a folder of
// Dolphin-named texture dumps). Two failure modes without explicit support:
// dropped as-is the wrapper has no recognized content (silent no-op); merged
// into one textures/ tree, Aurora's in-group dedup makes the alphabetically
// first variant silently win — the exact "accidental winner" class the load
// order exists to kill. So: a wrapper folder with NO recognized content of its
// own but with contentful immediate subfolders is exploded into one source per
// variant, named "Collection/Variant" ('/' cannot occur in a folder name, so
// the compound name is unambiguous in the order setting). Variants default to
// DISABLED on first sight — a collection is a menu of options, not a stack.
// One nesting level only.
// ============================================

// What a folder directly offers to the resolver.
struct ContentFlags {
    bool filesTree = false;   // files/ data tree (Layer A)
    bool icons = false;       // icons/
    bool texturesDir = false; // textures/
    bool looseBmd = false;    // <arc>_<idx>.bmd at the folder root (Layer B)
    bool looseTex = false;    // Dolphin-named dumps (tex1_*.dds/.png) at the root
    bool any() const { return filesTree || icons || texturesDir || looseBmd || looseTex; }
};

ContentFlags folder_content(const std::filesystem::path& dir) {
    ContentFlags out;
    std::error_code ec;
    out.filesTree = std::filesystem::is_directory(dir / "files", ec);
    out.icons = std::filesystem::is_directory(dir / "icons", ec);
    out.texturesDir = std::filesystem::is_directory(dir / "textures", ec);
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        const std::string name = normalize(entry.path().filename().string());
        if (name.size() > 4 && name.compare(name.size() - 4, 4, ".bmd") == 0) {
            out.looseBmd = true;
        } else if (name.rfind("tex1_", 0) == 0 &&
                   (name.size() > 4 && (name.compare(name.size() - 4, 4, ".dds") == 0 ||
                                        name.compare(name.size() - 4, 4, ".png") == 0))) {
            out.looseTex = true;
        }
        if (out.looseBmd && out.looseTex) {
            break;
        }
    }
    return out;
}

struct DiskEntry {
    std::string name;  // "Mod" or "Collection/Variant"
    bool variant;      // true for an exploded collection variant
};

// Disk enumeration of model_replacements/, alphabetical (the stable append
// order for entries not yet in the order setting), with collection wrappers
// exploded into per-variant entries.
std::vector<DiskEntry> disk_entries() {
    std::vector<DiskEntry> out;
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return out;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (folder_content(entry.path()).any()) {
            out.push_back({name, false});
            continue;
        }
        // No content of its own — a collection wrapper? Explode contentful
        // immediate subfolders into variant entries (one level only).
        bool anyVariant = false;
        for (const auto& sub : std::filesystem::directory_iterator(entry.path(), ec)) {
            if (!sub.is_directory(ec)) {
                continue;
            }
            if (folder_content(sub.path()).any()) {
                out.push_back({name + "/" + sub.path().filename().string(), true});
                anyVariant = true;
            }
        }
        if (!anyVariant) {
            out.push_back({name, false});  // plain (empty-ish) folder — list as before
        }
    }
    std::sort(out.begin(), out.end(),
              [](const DiskEntry& a, const DiskEntry& b) { return a.name < b.name; });
    return out;
}

// The FULL canonical order: setting entries verbatim (stale ones included),
// then disk entries not yet listed appended alphabetically — plain mods carry
// the LEGACY disabled flag verbatim (this single rule IS the first-run
// migration seed AND the steady-state new-folder append); collection VARIANTS
// default to DISABLED (mutually-exclusive options — the user opts into one).
// Finally, legacy-disabled names not on disk are appended as stale disabled
// entries (so their state survives a remove-and-reinstall across migration).
std::vector<OrderEntry> full_order_list() {
    std::vector<OrderEntry> out = parse_order_setting();
    const auto listed = [&out](const std::string& name) {
        for (const OrderEntry& e : out) {
            if (e.name == name) {
                return true;
            }
        }
        return false;
    };
    for (const DiskEntry& f : disk_entries()) {
        if (!listed(f.name)) {
            out.push_back({f.name, f.variant ? false : !legacy_disabled(f.name)});
        }
    }
    for (const std::string& d : disabled_list()) {
        if (!listed(d)) {
            out.push_back({d, false});
        }
    }
    return out;
}

// '|'-join with '-' prefixes — the game.customModelsOrder encoding.
std::string join_order(const std::vector<OrderEntry>& list) {
    std::string joined;
    for (const OrderEntry& e : list) {
        if (!joined.empty()) {
            joined.push_back('|');
        }
        if (!e.enabled) {
            joined.push_back('-');
        }
        joined += e.name;
    }
    return joined;
}

// Write the canonical order back to game.customModelsOrder (idempotent) and
// MIRROR the disabled set into the legacy key so a downgrade to a pre-order
// build keeps the exact same enabled/disabled state (dual-write, one release).
// In-memory only — persisted by the next config::Save() (the editor calls it
// after every toggle/move; a startup migration seed re-derives identically
// until then, so nothing can flip).
void persist_order(const std::vector<OrderEntry>& list) {
    const std::string joined = join_order(list);
    if (getSettings().game.customModelsOrder.getValue() != joined) {
        getSettings().game.customModelsOrder.setValue(joined);
    }
    std::string legacy;
    for (const OrderEntry& e : list) {
        if (e.enabled) {
            continue;
        }
        if (!legacy.empty()) {
            legacy.push_back('|');
        }
        legacy += e.name;
    }
    if (getSettings().game.customModelsDisabled.getValue() != legacy) {
        getSettings().game.customModelsDisabled.setValue(legacy);
    }
}
// ============================================

// ============================================
// NEW CODE — ALBW Port (load-order mod system, Phase 2 — Core tier, D3/D4)
// CORE packs: bundled first-party content at <exe>/content/core/<pack>/, each
// pack shaped exactly like a user mod folder. Always enabled, never in the
// editor order. ordered_sources() is THE resolution sequence every conflict
// site walks (first provider of an asset wins): by default core packs come
// FIRST (core wins conflicts — D4), and the explicit
// game.customModelsAllowCoreOverride toggle moves them LAST (user mods win).
// The root-level loose-BMD slot in resolve_override stays above both.
// ============================================

// <exe>/content/core, or empty if the exe dir can't be resolved.
std::filesystem::path core_root() {
    const char* base = SDL_GetBasePath();
    if (base == nullptr) {
        return {};
    }
    return std::filesystem::path(base) / "content" / "core";
}

// Bundled core pack names, alphabetical (their relative order rarely matters —
// first-party content shouldn't self-conflict; alphabetical keeps it stable).
std::vector<std::string> core_pack_names() {
    std::vector<std::string> out;
    const std::filesystem::path root = core_root();
    std::error_code ec;
    if (root.empty() || !std::filesystem::exists(root, ec)) {
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

// One resolvable content source (core pack or user mod), in WINNING order:
// index 0 beats everything below it at every conflict site.
struct Source {
    std::string name;
    std::filesystem::path root;  // the pack/mod folder itself
    bool enabled;
    bool core;
};

std::vector<Source> ordered_sources() {
    std::vector<Source> out;
    const std::filesystem::path coreDir = core_root();
    const std::filesystem::path userDir = ConfigPath / "model_replacements";
    const bool userWins = getSettings().game.customModelsAllowCoreOverride.getValue();
    const auto pushCore = [&out, &coreDir] {
        for (const std::string& pack : core_pack_names()) {
            out.push_back({pack, coreDir / pack, true, true});
        }
    };
    if (!userWins) {
        pushCore();  // D4 default: core wins every conflict with a user mod
    }
    for (const OrderEntry& e : full_order_list()) {
        out.push_back({e.name, userDir / e.name, e.enabled, false});
    }
    if (userWins) {
        pushCore();  // override allowed: user mods layer above core (§4.1)
    }
    return out;
}

// Phase-3 conflict bookkeeping (§4.5): `winner` already claimed an asset that
// `loser` also provides. Aggregates per-source wins/losses; a user-vs-core
// collision additionally sets the D4 override flags for the editor badges.
void record_conflict_into(std::unordered_map<std::string, FolderConflicts>& stats,
                          const Source& winner, const Source& loser) {
    FolderConflicts& w = stats[winner.name];
    FolderConflicts& l = stats[loser.name];
    ++w.wins;
    ++l.losses;
    if (!winner.core && loser.core) {
        w.overridesCore = true;
    }
    if (winner.core && !loser.core) {
        l.overriddenByCore = true;
    }
}

void record_conflict(const Source& winner, const Source& loser) {
    record_conflict_into(s_conflicts, winner, loser);
}
// ============================================

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
            // Modding-tool extension chunk seen in re-packed .baa files (e.g.
            // Linkle's, one offset arg right before '>_AA'). Not a vanilla
            // command (JAUAudioArcInterpreter would assert); skip it so the
            // walk reaches any commands after it instead of stopping.
            case 'bfca':                            args = 1; break;
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
// s_modWaves. Called from scan(). Single audio mod assumption; sources are
// walked in WINNING order (core-vs-user per D4, users top-first per D1), so
// the highest-priority provider's .baa wins — matching the s_audioIndex winner.
void rebuild_mod_wave_descriptors(const std::vector<Source>& sources) {
    s_modWaves.clear();
    std::error_code ec;
    for (const Source& src : sources) {
        const std::filesystem::path baaPath =
            src.root / "files" / "Audiores" / "Z2Sound.baa";
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
        return;  // single audio mod — highest-priority provider wins (top wins)
    }
}
#endif  // D_ALBW_AUDIO_SHADOW

#if D_ALBW_CUSTOM_ICONS
// ============================================
// Custom item/weapon/shop icons — Layer-B for textures.
//
// A modder drops a loose PNG at <mod>/icons/item_<itemNo>.png. We encode it into
// a GameCube ResTIMG (CI8 48x48 with a <=256-color RGB5A3 palette — the exact
// format/size class of the vanilla itemicon.arc icons, so custom icons match
// vanilla sharpness) and hand it to the game's item-texture reader in place of
// the vanilla icon. Palettized CI8 keeps per-pixel alpha via the palette and
// fits the game's fixed 0xC00 buffer (0x20 header + 0x900 indices + 0x200
// palette = 0xB20). The encoded TIMG is cached per item; the cache clears on a
// folder toggle.
// ============================================

std::mutex s_iconMtx;
// itemNo -> absolute custom icon PNG path (ENABLED mods only). Rebuilt in scan().
// Keyed off files named "item_<N>.png".
std::unordered_map<int, std::string> s_iconIndex;
// itemNo -> encoded ResTIMG bytes (empty vector = "tried, no valid override").
std::unordered_map<int, std::vector<u8>> s_iconTimgCache;

// NAMED icons: any other "icons/<name>.png" -> path, keyed by <name> (lower-case,
// no extension). For dedicated UI slots that must NOT collide with an item id
// (e.g. the Stamina Upgrade shop row -> "stamina_upgrade").
std::unordered_map<std::string, std::string> s_namedIconIndex;
std::unordered_map<std::string, std::vector<u8>> s_namedIconTimgCache;

// Big-endian byte writers (ResTIMG multi-byte fields are big-endian).
void put_be16(u8*& p, u16 v) { *p++ = u8(v >> 8); *p++ = u8(v); }
void put_be32(u8*& p, u32 v) {
    *p++ = u8(v >> 24); *p++ = u8(v >> 16); *p++ = u8(v >> 8); *p++ = u8(v);
}

// One RGBA8 pixel -> a big-endian RGB5A3 halfword. Opaque (a>=0xE0) uses the
// 5-bit-colour / no-alpha encoding (top bit 1); otherwise 3-bit alpha + 4-bit
// colour (top bit 0).
u16 rgba_to_rgb5a3(u8 r, u8 g, u8 b, u8 a) {
    if (a >= 0xE0) {
        return u16(0x8000 | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
    }
    return u16(((a >> 5) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4));
}

// Inverse (for palette-reduction distance): RGB5A3 halfword -> RGBA8.
void rgb5a3_to_rgba(u16 v, u8 out[4]) {
    if (v & 0x8000) {
        out[0] = u8(((v >> 10) & 0x1F) * 255 / 31);
        out[1] = u8(((v >> 5) & 0x1F) * 255 / 31);
        out[2] = u8((v & 0x1F) * 255 / 31);
        out[3] = 0xFF;
    } else {
        out[0] = u8(((v >> 8) & 0xF) * 255 / 15);
        out[1] = u8(((v >> 4) & 0xF) * 255 / 15);
        out[2] = u8((v & 0xF) * 255 / 15);
        out[3] = u8(((v >> 12) & 0x7) * 255 / 7);
    }
}

// Load the PNG, nearest-resample to 48x48, and write a complete ResTIMG into
// `out` as GC CI8 (8-bit palettized, <=256-color RGB5A3 palette) — the SAME
// format + resolution class as the vanilla itemicon.arc icons (verified: they
// are CI8 at 48x48 / 40x64 / 56x47 etc.), so custom icons match vanilla
// fidelity. Fits the game's fixed 0xC00 buffer: 0x20 header + 48*48 indices
// (0x900) + 32-byte-aligned 256*2 palette = 0xB20. (The previous direct
// RGB5A3 encoding capped at 32x32 in the same buffer — visibly blockier than
// the vanilla icons beside it.) Returns false on any failure.
bool encode_icon_timg(const std::string& path, std::vector<u8>& out) {
    const auto png = aurora::gfx::png::load_png_file(path);
    if (!png || png->width == 0 || png->height == 0) {
        return false;
    }
    const u8* src = reinterpret_cast<const u8*>(png->data.data());
    const uint32_t sw = png->width;
    const uint32_t sh = png->height;
    if (png->data.size() < static_cast<size_t>(sw) * sh * 4) {
        return false;
    }

    constexpr int TW = 48;  // multiples of the CI8 tile (8x4) -> no partial tiles
    constexpr int TH = 48;

    // Nearest-resample straight into RGB5A3 values (the palette's native
    // precision, so quantizing before palettizing loses nothing extra).
    std::vector<u16> pix(static_cast<size_t>(TW) * TH);
    for (int y = 0; y < TH; ++y) {
        for (int x = 0; x < TW; ++x) {
            const uint32_t sx = static_cast<uint32_t>(x) * sw / TW;
            const uint32_t sy = static_cast<uint32_t>(y) * sh / TH;
            const u8* p = src + (static_cast<size_t>(sy) * sw + sx) * 4;
            pix[static_cast<size_t>(y) * TW + x] = rgba_to_rgb5a3(p[0], p[1], p[2], p[3]);
        }
    }

    // Build the palette: unique RGB5A3 values by frequency. Icon art almost
    // always lands under 256 uniques after RGB5A3 quantization; if not, keep
    // the 256 most frequent and remap the rest to the nearest kept colour.
    std::unordered_map<u16, int> freq;
    for (u16 v : pix) {
        ++freq[v];
    }
    std::vector<std::pair<u16, int>> ranked(freq.begin(), freq.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    const size_t palCount = std::min<size_t>(ranked.size(), 256);
    std::vector<u16> palette(palCount);
    std::unordered_map<u16, u8> indexOf;
    for (size_t i = 0; i < palCount; ++i) {
        palette[i] = ranked[i].first;
        indexOf.emplace(ranked[i].first, static_cast<u8>(i));
    }
    const auto nearest_index = [&palette](u16 v) -> u8 {
        u8 rgba[4];
        rgb5a3_to_rgba(v, rgba);
        int best = 0;
        long bestDist = -1;
        for (size_t i = 0; i < palette.size(); ++i) {
            u8 q[4];
            rgb5a3_to_rgba(palette[i], q);
            const long dr = rgba[0] - q[0], dg = rgba[1] - q[1], db = rgba[2] - q[2];
            const long da = (rgba[3] - q[3]) * 2;  // alpha errors are the most visible
            const long dist = dr * dr + dg * dg + db * db + da * da;
            if (bestDist < 0 || dist < bestDist) {
                bestDist = dist;
                best = static_cast<int>(i);
            }
        }
        return static_cast<u8>(best);
    };

    constexpr u32 kImageOffset = 0x20;                        // indices follow the header
    constexpr u32 kImageSize = static_cast<u32>(TW) * TH;     // 0x900
    constexpr u32 kPaletteOffset = kImageOffset + kImageSize; // 0x920 — 32-byte aligned
    static_assert(kPaletteOffset % 32 == 0, "TLUT wants 32-byte alignment");
    out.assign(kPaletteOffset + palCount * 2, 0);

    // ResTIMG header (layout per JSystem/JUtility/JUTTexture.h, 0x20 bytes).
    // Field values verified against a REAL vanilla 48x48 icon BTI from
    // itemicon.arc (ari_mesu_00.bti): fmt=9, transp=2, tlutFmt=2, palOff=0x920,
    // imgOff=0x20. NOTE the first-attempt trap: GX_TF_C8 is 0x9, NOT 0x8 —
    // 0x8 is GX_TF_C4, which made the game read the 8-bit indices as 4-bit
    // pairs and drew garbage.
    u8* h = out.data();
    *h++ = 0x9;               // 0x00 format = GX_TF_C8 (CI8) — 0x8 would be C4!
    *h++ = 0x2;               // 0x01 transparency mode (2 = translucent, as vanilla)
    put_be16(h, TW);          // 0x02 width
    put_be16(h, TH);          // 0x04 height
    *h++ = 0x0;               // 0x06 wrapS (clamp)
    *h++ = 0x0;               // 0x07 wrapT (clamp)
    *h++ = 0x1;               // 0x08 indexTexture (palettized)
    *h++ = 0x2;               // 0x09 colorFormat = GX_TL_RGB5A3
    put_be16(h, u16(palCount));         // 0x0A numColors
    put_be32(h, kPaletteOffset);        // 0x0C paletteOffset
    *h++ = 0x0;               // 0x10 mipmapEnabled
    *h++ = 0x0;               // 0x11 doEdgeLOD
    *h++ = 0x0;               // 0x12 biasClamp
    *h++ = 0x0;               // 0x13 maxAnisotropy
    *h++ = 0x1;               // 0x14 minFilter (GX_LINEAR)
    *h++ = 0x1;               // 0x15 magFilter (GX_LINEAR)
    *h++ = 0x0;               // 0x16 minLOD
    *h++ = 0x0;               // 0x17 maxLOD
    *h++ = 0x1;               // 0x18 mipmapCount
    *h++ = 0x0;               // 0x19 unknown
    put_be16(h, 0);           // 0x1A LODBias
    put_be32(h, kImageOffset);          // 0x1C imageOffset

    // GC-tiled CI8 indices: 8x4 pixel blocks, row-major within a block, blocks
    // L->R then top->bottom.
    u8* dst = out.data() + kImageOffset;
    for (int by = 0; by < TH; by += 4) {
        for (int bx = 0; bx < TW; bx += 8) {
            for (int py = 0; py < 4; ++py) {
                for (int px = 0; px < 8; ++px) {
                    const u16 v = pix[static_cast<size_t>(by + py) * TW + (bx + px)];
                    const auto it = indexOf.find(v);
                    *dst++ = it != indexOf.end() ? it->second : nearest_index(v);
                }
            }
        }
    }

    // Palette (RGB5A3, big-endian entries).
    u8* pal = out.data() + kPaletteOffset;
    for (size_t i = 0; i < palCount; ++i) {
        put_be16(pal, palette[i]);
    }
    return true;
}

// Rebuild s_iconIndex from ENABLED sources' <src>/icons/item_<N>.png files,
// walked in WINNING order — the first (highest-priority) provider of an icon
// claims it ("top wins", D1; core-vs-user per D4). Clears the encoded-TIMG
// cache so a toggle/move re-evaluates. Called from scan() under lock.
void rebuild_icon_index(const std::vector<Source>& sources) {
    s_iconIndex.clear();
    s_iconTimgCache.clear();
    s_namedIconIndex.clear();
    s_namedIconTimgCache.clear();
    // Which source claimed each icon slot (Phase-3 win/lose attribution).
    std::unordered_map<int, const Source*> itemClaimedBy;
    std::unordered_map<std::string, const Source*> namedClaimedBy;
    std::error_code ec;
    for (const Source& src : sources) {
        if (!src.enabled) {
            continue;  // icons are enable-gated
        }
        const std::filesystem::path iconsDir = src.root / "icons";
        if (!std::filesystem::is_directory(iconsDir, ec)) {
            continue;
        }
        for (const auto& f : std::filesystem::directory_iterator(iconsDir, ec)) {
            if (!f.is_regular_file(ec)) {
                continue;
            }
            const std::string name = normalize(f.path().filename().string());
            if (name.size() < 5 || name.compare(name.size() - 4, 4, ".png") != 0) {
                continue;  // only .png icons
            }

            // Self-heal a common drop mistake: a screenshot/preview image placed
            // inside icons/ would otherwise register as a (garbage) named icon.
            // Icons are 48px-class; anything big is clearly not one. (PNG IHDR:
            // width/height are big-endian u32 at bytes 16/20.)
            {
                FILE* fp = std::fopen(f.path().string().c_str(), "rb");
                if (fp != nullptr) {
                    u8 hdr[24] = {};
                    const bool ok = std::fread(hdr, 1, sizeof(hdr), fp) == sizeof(hdr);
                    std::fclose(fp);
                    if (ok) {
                        const u32 w = (u32(hdr[16]) << 24) | (u32(hdr[17]) << 16) |
                                      (u32(hdr[18]) << 8) | u32(hdr[19]);
                        const u32 h = (u32(hdr[20]) << 24) | (u32(hdr[21]) << 16) |
                                      (u32(hdr[22]) << 8) | u32(hdr[23]);
                        if (w > 256 || h > 256) {
                            DuskLog.info(
                                "[custom_assets] skipping '{}' in icons/ ({}x{} — looks "
                                "like a screenshot/preview, not an icon; put it next to "
                                "modinfo.ini instead)",
                                name, w, h);
                            continue;
                        }
                    }
                }
            }

            const std::string stem = name.substr(0, name.size() - 4);

            // "item_<N>" (decimal, or 0x-prefixed hex so the dItemNo enum's hex
            // values paste straight in) -> a numeric item-id override.
            if (stem.rfind("item_", 0) == 0 && stem.size() > 5) {
                const std::string digits = stem.substr(5);
                const char* p = digits.data();
                const char* end = p + digits.size();
                int base = 10;
                if (digits.size() > 2 && digits[0] == '0' && digits[1] == 'x') {
                    base = 16;
                    p += 2;
                }
                int itemNo = -1;
                const auto res = std::from_chars(p, end, itemNo, base);
                if (res.ec == std::errc() && res.ptr == end && itemNo >= 0 && itemNo <= 0xFF) {
                    // first provider wins (top wins)
                    if (s_iconIndex.emplace(itemNo, f.path().string()).second) {
                        itemClaimedBy.emplace(itemNo, &src);
                    } else {
                        record_conflict(*itemClaimedBy[itemNo], src);  // Phase 3 (§4.5)
                    }
                    continue;
                }
            }

            // Any other name -> a dedicated NAMED icon slot (e.g. "stamina_upgrade"),
            // which never collides with an item id.
            // first provider wins (top wins)
            if (s_namedIconIndex.emplace(stem, f.path().string()).second) {
                namedClaimedBy.emplace(stem, &src);
            } else {
                record_conflict(*namedClaimedBy[stem], src);  // Phase 3 (§4.5)
            }
        }
    }
    DuskLog.info("[custom_assets] icons: {} item + {} named custom icon(s) indexed",
                 static_cast<int>(s_iconIndex.size()),
                 static_cast<int>(s_namedIconIndex.size()));
}
#endif  // D_ALBW_CUSTOM_ICONS

}  // namespace

// ============================================
// Shared — Custom Models editor list (ordered; enable/disable + move)
// ============================================
std::vector<std::string> list_folders() {
    // PRIORITY order (top = index 0 = wins conflicts), filtered to folders that
    // actually exist on disk — stale order entries are kept in the setting but
    // never shown or resolved.
    std::vector<std::string> out;
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    for (const OrderEntry& e : full_order_list()) {
        if (std::filesystem::is_directory(root / e.name, ec)) {
            out.push_back(e.name);
        }
    }
    return out;
}

bool is_folder_enabled(const char* folder) {
    if (folder == nullptr) {
        return true;
    }
    // Order setting first ('-' prefix = disabled) ...
    for (const OrderEntry& e : parse_order_setting()) {
        if (e.name == folder) {
            return e.enabled;
        }
    }
    // ... then the legacy disabled list for folders not yet in the order
    // (dual-read, one release — this is what carries a pre-migration disable).
    return !legacy_disabled(folder);
}

void toggle_folder(const char* folder) {
    if (folder == nullptr) {
        return;
    }
    std::vector<OrderEntry> list = full_order_list();
    bool found = false;
    for (OrderEntry& e : list) {
        if (e.name == folder) {
            e.enabled = !e.enabled;
            found = true;
            break;
        }
    }
    if (!found) {
        // Unknown folder (not on disk, not listed): append disabled — a toggle
        // on a fresh name can only mean "turn it off" (unlisted = enabled).
        list.push_back({folder, false});
    }
    persist_order(list);

    // Rebuild the map and re-register the overlay set so the change lands on the
    // next asset load (reload-scoped, texture-API style). Already-loaded assets
    // (mounted arcs, active audio banks) keep the old data until re-requested.
    scan();
    install_overlays();
}

bool move_folder(const char* folder, int delta, bool apply) {
    if (folder == nullptr || (delta != -1 && delta != 1)) {
        return false;
    }
    std::vector<OrderEntry> list = full_order_list();

    // Swap with the adjacent DISK-PRESENT entry in the move direction — stale
    // entries are skipped over so a move always changes the VISIBLE order.
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    const auto onDisk = [&root, &ec](const OrderEntry& e) {
        return std::filesystem::is_directory(root / e.name, ec);
    };
    int self = -1;
    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        if (list[i].name == folder) {
            self = i;
            break;
        }
    }
    if (self < 0) {
        return false;
    }
    int neighbor = -1;
    for (int i = self + delta; i >= 0 && i < static_cast<int>(list.size()); i += delta) {
        if (onDisk(list[i])) {
            neighbor = i;
            break;
        }
    }
    if (neighbor < 0) {
        return false;  // already at that end of the (visible) list
    }
    std::swap(list[self], list[neighbor]);
    persist_order(list);

    // An order change can change conflict winners at every site — re-resolve.
    // Reload-scoped like a toggle: lands on the next asset load. Deferred
    // (apply=false) during a Mods-window drag; apply_order_changes() finishes.
    if (apply) {
        scan();
        install_overlays();
    }
    return true;
}

void apply_order_changes() {
    scan();
    install_overlays();
}

bool move_folder_to(const char* folder, int slot) {
    if (folder == nullptr || slot < 0) {
        return false;
    }
    std::vector<OrderEntry> list = full_order_list();

    // Visible (disk-present) sequence over the full list — stale entries keep
    // their positions but don't count as slots.
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    std::vector<int> visible;  // full-list indices, in visible order
    int self = -1;
    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        if (list[i].name == folder) {
            self = i;
        }
        if (std::filesystem::is_directory(root / list[i].name, ec)) {
            visible.push_back(i);
        }
    }
    if (self < 0) {
        return false;
    }
    int selfSlot = -1;
    for (int v = 0; v < static_cast<int>(visible.size()); ++v) {
        if (visible[v] == self) {
            selfSlot = v;
            break;
        }
    }
    if (selfSlot < 0) {
        return false;  // folder not on disk
    }
    slot = std::min(slot, static_cast<int>(visible.size()) - 1);
    if (slot == selfSlot) {
        return false;
    }

    // Remove, then insert before the entry that currently occupies the target
    // visible slot (accounting for the removal shifting later indices).
    const OrderEntry moved = list[self];
    list.erase(list.begin() + self);
    int insertAt;
    if (slot >= static_cast<int>(visible.size()) - 1 &&
        selfSlot < static_cast<int>(visible.size()) - 1) {
        insertAt = static_cast<int>(list.size());  // moving to the bottom
    } else {
        // Index of the visible entry that should end up BELOW the moved one.
        const int belowSlot = slot + (slot > selfSlot ? 1 : 0);
        int target = visible[belowSlot];
        if (target > self) {
            --target;  // erase shifted it
        }
        insertAt = target;
    }
    list.insert(list.begin() + insertAt, moved);
    persist_order(list);

    scan();
    install_overlays();
    return true;
}

std::string mod_group_key(const char* folder) {
    if (folder == nullptr) {
        return {};
    }
    const std::string path(folder);
    const auto slash = path.find('/');
    return slash == std::string::npos ? path : path.substr(0, slash);
}

bool mod_is_collection_variant(const char* folder) {
    return folder != nullptr && std::strchr(folder, '/') != nullptr;
}

static std::vector<std::vector<OrderEntry>> order_as_group_blocks(
    const std::vector<OrderEntry>& list) {
    std::vector<std::string> key_order;
    std::unordered_map<std::string, std::vector<OrderEntry>> blocks;
    for (const OrderEntry& e : list) {
        const std::string key = mod_group_key(e.name.c_str());
        if (blocks.find(key) == blocks.end()) {
            key_order.push_back(key);
        }
        blocks[key].push_back(e);
    }
    std::vector<std::vector<OrderEntry>> out;
    out.reserve(key_order.size());
    for (const std::string& key : key_order) {
        out.push_back(blocks[key]);
    }
    return out;
}

static std::vector<OrderEntry> flatten_group_blocks(
    const std::vector<std::vector<OrderEntry>>& blocks) {
    std::vector<OrderEntry> out;
    for (const std::vector<OrderEntry>& block : blocks) {
        out.insert(out.end(), block.begin(), block.end());
    }
    return out;
}

static bool group_block_on_disk(const std::vector<OrderEntry>& block) {
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    for (const OrderEntry& e : block) {
        if (std::filesystem::is_directory(root / e.name, ec)) {
            return true;
        }
    }
    return false;
}

static int find_group_block_index(const std::vector<std::vector<OrderEntry>>& blocks,
                                  const char* group_key) {
    if (group_key == nullptr) {
        return -1;
    }
    for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
        if (!blocks[i].empty() && mod_group_key(blocks[i][0].name.c_str()) == group_key) {
            return i;
        }
    }
    return -1;
}

bool select_collection_variant(const char* variant_folder) {
    if (variant_folder == nullptr || !mod_is_collection_variant(variant_folder)) {
        return false;
    }
    const std::string root = mod_group_key(variant_folder);
    std::vector<OrderEntry> list = full_order_list();
    bool changed = false;
    for (OrderEntry& e : list) {
        if (mod_group_key(e.name.c_str()) != root) {
            continue;
        }
        const bool want = e.name == variant_folder;
        if (e.enabled != want) {
            e.enabled = want;
            changed = true;
        }
    }
    if (!changed) {
        return false;
    }
    persist_order(list);
    scan();
    install_overlays();
    return true;
}

bool set_mod_group_enabled(const char* group_key, bool enabled) {
    if (group_key == nullptr || group_key[0] == '\0') {
        return false;
    }
    std::vector<OrderEntry> list = full_order_list();
    bool changed = false;
    if (!enabled) {
        for (OrderEntry& e : list) {
            if (mod_group_key(e.name.c_str()) != group_key) {
                continue;
            }
            if (e.enabled) {
                e.enabled = false;
                changed = true;
            }
        }
    } else {
        bool anyEnabled = false;
        int firstMember = -1;
        for (int i = 0; i < static_cast<int>(list.size()); ++i) {
            if (mod_group_key(list[i].name.c_str()) != group_key) {
                continue;
            }
            if (firstMember < 0) {
                firstMember = i;
            }
            if (list[i].enabled) {
                anyEnabled = true;
                break;
            }
        }
        if (firstMember < 0) {
            return false;
        }
        if (!anyEnabled) {
            list[firstMember].enabled = true;
            changed = true;
        }
    }
    if (!changed) {
        return false;
    }
    persist_order(list);
    scan();
    install_overlays();
    return true;
}

std::string mod_folder_path(const char* folder) {
    if (folder == nullptr || folder[0] == '\0') {
        return {};
    }
    std::error_code ec;
    const auto abs = std::filesystem::absolute(ConfigPath / "model_replacements" / folder, ec);
    if (ec) {
        return io::fs_path_to_string(ConfigPath / "model_replacements" / folder);
    }
    return io::fs_path_to_string(abs);
}

void rescan_and_install() {
    scan();
    install_overlays();
}

bool move_mod_group(const char* group_key, int delta, bool apply) {
    if (group_key == nullptr || (delta != -1 && delta != 1)) {
        return false;
    }
    std::vector<OrderEntry> list = full_order_list();
    std::vector<std::vector<OrderEntry>> blocks = order_as_group_blocks(list);

    std::vector<int> visible;
    for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
        if (group_block_on_disk(blocks[i])) {
            visible.push_back(i);
        }
    }

    const int selfBlock = find_group_block_index(blocks, group_key);
    if (selfBlock < 0) {
        return false;
    }

    int selfSlot = -1;
    for (int v = 0; v < static_cast<int>(visible.size()); ++v) {
        if (visible[v] == selfBlock) {
            selfSlot = v;
            break;
        }
    }
    if (selfSlot < 0) {
        return false;
    }

    const int targetSlot = selfSlot + delta;
    if (targetSlot < 0 || targetSlot >= static_cast<int>(visible.size())) {
        return false;
    }

    std::swap(blocks[visible[selfSlot]], blocks[visible[targetSlot]]);
    list = flatten_group_blocks(blocks);
    persist_order(list);

    if (apply) {
        scan();
        install_overlays();
    }
    return true;
}

bool move_mod_group_to(const char* group_key, int slot, bool apply) {
    if (group_key == nullptr || slot < 0) {
        return false;
    }
    std::vector<OrderEntry> list = full_order_list();
    std::vector<std::vector<OrderEntry>> blocks = order_as_group_blocks(list);

    std::vector<int> visible;
    for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
        if (group_block_on_disk(blocks[i])) {
            visible.push_back(i);
        }
    }

    const int selfBlock = find_group_block_index(blocks, group_key);
    if (selfBlock < 0) {
        return false;
    }

    int selfSlot = -1;
    for (int v = 0; v < static_cast<int>(visible.size()); ++v) {
        if (visible[v] == selfBlock) {
            selfSlot = v;
            break;
        }
    }
    if (selfSlot < 0) {
        return false;
    }

    slot = std::min(slot, static_cast<int>(visible.size()) - 1);
    if (slot == selfSlot) {
        return false;
    }

    const std::vector<OrderEntry> moved = blocks[selfBlock];
    blocks.erase(blocks.begin() + selfBlock);

    int insertBlock;
    if (slot >= static_cast<int>(visible.size()) - 1 &&
        selfSlot < static_cast<int>(visible.size()) - 1) {
        insertBlock = static_cast<int>(blocks.size());
    } else {
        const int belowSlot = slot + (slot > selfSlot ? 1 : 0);
        insertBlock = visible[belowSlot];
        if (insertBlock > selfBlock) {
            --insertBlock;
        }
    }
    blocks.insert(blocks.begin() + insertBlock, moved);

    list = flatten_group_blocks(blocks);
    persist_order(list);

    if (apply) {
        scan();
        install_overlays();
    }
    return true;
}

std::vector<std::pair<std::string, bool>> order_view(const std::vector<std::string>& folders) {
    // Cheap per-frame view for the editor: order-setting parse only, NO disk.
    std::vector<std::pair<std::string, bool>> out;
    out.reserve(folders.size());
    const auto known = [&folders](const std::string& name) {
        return std::find(folders.begin(), folders.end(), name) != folders.end();
    };
    const auto taken = [&out](const std::string& name) {
        for (const auto& p : out) {
            if (p.first == name) {
                return true;
            }
        }
        return false;
    };
    for (const OrderEntry& e : parse_order_setting()) {
        if (known(e.name)) {
            out.emplace_back(e.name, e.enabled);
        }
    }
    for (const std::string& f : folders) {
        if (!taken(f)) {
            out.emplace_back(f, !legacy_disabled(f));  // not yet listed — dual-read
        }
    }
    return out;
}

std::vector<std::string> list_core_packs() {
    return core_pack_names();
}

namespace {
// Live Aurora registrations for every enabled source's textures/ subtree.
// Rebuilt wholesale by rebuild_texture_packs(); the groups own nothing but
// registration ids, so unregister+clear is always safe.
std::vector<aurora::texture::ReplacementGroup> s_texPackGroups;
// Fingerprint of the DESIRED pack set at the last rebuild. Aurora clears its
// STATIC TEXTURE CACHE on every single register/unregister, and cached HUD/UI
// textures whose CPU-side source buffers are transient re-upload from dead
// memory after a clear (the classic garbled-icon / white-static-box breakage).
// So rebuild_texture_packs — which runs on EVERY scan, i.e. every mod toggle —
// must be a strict NO-OP unless the pack set actually changed; toggling an
// unrelated (icon/model) mod must never touch the texture registry.
std::string s_texPackSignature;
}  // namespace

void rebuild_texture_packs() {
    // Compute the desired (root, priority) set first, WITHOUT touching Aurora.
    const bool master = getSettings().game.enableTextureReplacements;
    const std::vector<Source> sources = ordered_sources();
    struct DesiredPack {
        const Source* src;
        std::filesystem::path texRoot;
        int priority;
    };
    std::vector<DesiredPack> desired;
    std::string signature;
    std::error_code ec;
    if (master) {
        int priority = static_cast<int>(sources.size());
        for (const Source& src : sources) {
            const int p = priority--;
            if (!src.enabled) {
                continue;
            }
            // textures/ subtree, or the folder ITSELF when it holds Dolphin-
            // named dumps at its root (the shape collection variants ship in).
            std::filesystem::path texRoot = src.root / "textures";
            if (!std::filesystem::is_directory(texRoot, ec)) {
                if (!folder_content(src.root).looseTex) {
                    continue;
                }
                texRoot = src.root;
            }
            signature += fmt::format("{}#{};", texRoot.string(), p);
            desired.push_back({&src, std::move(texRoot), p});
        }
    }
    if (signature == s_texPackSignature) {
        return;  // set unchanged — do NOT churn Aurora's static texture cache
    }
    s_texPackSignature = signature;

    for (const auto& group : s_texPackGroups) {
        aurora::texture::unregister_replacements(group);
    }
    s_texPackGroups.clear();
    s_texConflicts.clear();

    // Winner-first sources get DESCENDING Aurora priority, all > 0 — Aurora
    // picks the highest priority per texture, so the load-order winner also
    // wins texture conflicts, and the global texture_replacements/ dir (which
    // registers at the default priority 0) stays the lowest-priority fallback.
    // Which source claimed each texture key first (Phase-3 badge attribution —
    // Aurora keeps ALL registrations and resolves by priority; this only counts).
    std::unordered_map<std::string, const Source*> texClaimedBy;
    for (const DesiredPack& pack : desired) {
        const Source& src = *pack.src;
        const int p = pack.priority;
        aurora::texture::ReplacementGroup group =
            aurora::texture::load_replacement_directory(pack.texRoot, {.priority = p});
        if (group.registrations.empty()) {
            continue;
        }
        for (const auto& reg : group.registrations) {
            const auto* key = std::get_if<aurora::texture::TextureSourceKey>(&reg.key);
            if (key == nullptr) {
                continue;
            }
            const std::string flat =
                fmt::format("{:x}:{:x}:{}x{}:{}:{}", key->textureHash, key->tlutHash,
                            key->width, key->height, key->format, key->hasTlut ? 1 : 0);
            const auto claimed = texClaimedBy.emplace(flat, &src);
            if (!claimed.second) {
                record_conflict_into(s_texConflicts, *claimed.first->second, src);
            }
        }
        DuskLog.info("[custom_assets] texture pack '{}'{}: {} registration(s) (priority {})",
                     src.name, src.core ? " [core]" : "",
                     static_cast<int>(group.registrations.size()), p);
        s_texPackGroups.push_back(std::move(group));
    }
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

    std::error_code ec;

    // ============================================
    // NEW CODE — ALBW Port (load-order mod system, Phase 1 + 2)
    // Canonicalize the order setting (idempotent): on the very first run this
    // seeds game.customModelsOrder from the alphabetical folder list with the
    // legacy disabled flags carried VERBATIM (the §4.7 migration); afterwards it
    // just appends any newly-dropped folder (enabled) at the bottom. Then walk
    // ALL sources — core packs and user mods, sequenced per D4 — in WINNING
    // order: every conflict site below takes the FIRST provider of an asset
    // ("top wins", decision D1).
    // ============================================
    persist_order(full_order_list());
    const std::vector<Source> sources = ordered_sources();
    s_conflicts.clear();  // Phase-3 stats rebuild with every scan
    // Which source claimed each Layer-A path (for win/lose attribution).
    std::unordered_map<std::string, const Source*> claimedBy;

    int mods = 0;
    for (const Source& mod : sources) {
        const std::filesystem::path filesRoot = mod.root / "files";
        const std::filesystem::path arcsRoot = mod.root / "arcs";
        const bool hasFiles = std::filesystem::is_directory(filesRoot, ec);
        const bool hasArcs = std::filesystem::is_directory(arcsRoot, ec);
        if (!hasFiles && !hasArcs) {
            continue;  // stale entry, or Layer-B-only / empty folder
        }
        // NOTE: we walk DISABLED folders too — the model overlay (s_map) is
        // enable-gated below, but the audio twin index is enable-INDEPENDENT so a
        // boot-disabled audio mod can still be toggled on live.
        const bool enabled = mod.enabled;

        ++mods;
        int count_ = 0;
        int shadowed = 0;  // claimed by a higher-priority mod (top wins)
        std::string sample;
        if (hasFiles) {
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
                    // emplace = first (highest-priority) provider wins.
                    localAudio.emplace(path_leaf(norm), it->path().string());
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
                // emplace-if-absent: a lower-priority mod never displaces a claimed
                // path ("top wins" — was last-scanned-wins assignment).
                if (s_map.emplace(norm, it->path().string()).second) {
                    claimedBy.emplace(norm, &mod);
                    if (sample.empty()) {
                        sample = norm;
                    }
                    ++count_;
                } else {
                    ++shadowed;
                    record_conflict(*claimedBy[norm], mod);  // Phase 3 (§4.5)
                }
            }
        }

        // Plan R2: arcs/<Name>.arc → DVD overlay as res/Object/<Name>.arc (stock loader).
        // Never commits WW bytes; inert when arcs/ is empty.
        if (hasArcs && enabled) {
            for (auto it = std::filesystem::directory_iterator(arcsRoot, ec);
                 it != std::filesystem::directory_iterator(); it.increment(ec)) {
                if (ec || !it->is_regular_file(ec)) {
                    continue;
                }
                const auto ext = it->path().extension().string();
                if (ext != ".arc" && ext != ".ARC") {
                    continue;
                }
                const std::string stem = it->path().stem().string();
                if (stem.empty()) {
                    continue;
                }
                const std::string norm = normalize("res/Object/" + stem + ".arc");
                if (s_map.emplace(norm, it->path().string()).second) {
                    claimedBy.emplace(norm, &mod);
                    if (sample.empty()) {
                        sample = norm;
                    }
                    ++count_;
                    DuskLog.info("[custom_assets] R2 arc-mount '{}' → {}", stem, norm);
                } else {
                    ++shadowed;
                    record_conflict(*claimedBy[norm], mod);
                }
            }
        }

        // №110: arcs_lib/ = library fallback. Only overlay stems named by npc/*.ini
        // arc= that are missing from arcs/ — never dump the whole library (hundreds).
        const std::filesystem::path arcsLibRoot = mod.root / "arcs_lib";
        const bool hasArcsLib = std::filesystem::is_directory(arcsLibRoot, ec);
        if (enabled && hasArcsLib) {
            std::unordered_set<std::string> needed;
            const std::filesystem::path npcRoot = mod.root / "npc";
            if (std::filesystem::is_directory(npcRoot, ec)) {
                for (auto it = std::filesystem::directory_iterator(npcRoot, ec);
                     it != std::filesystem::directory_iterator(); it.increment(ec)) {
                    if (ec || !it->is_regular_file(ec)) {
                        continue;
                    }
                    const auto ext = it->path().extension().string();
                    if (ext != ".ini" && ext != ".INI") {
                        continue;
                    }
                    std::ifstream in(it->path());
                    std::string line;
                    while (std::getline(in, line)) {
                        if (line.rfind("arc=", 0) != 0) {
                            continue;
                        }
                        std::string name = line.substr(4);
                        while (!name.empty() &&
                               (name.back() == '\r' || name.back() == '\n' || name.back() == ' ' ||
                                name.back() == '\t')) {
                            name.pop_back();
                        }
                        if (!name.empty()) {
                            needed.insert(name);
                        }
                    }
                }
            }
            int libMounted = 0;
            for (const std::string& stem : needed) {
                const std::string norm = normalize("res/Object/" + stem + ".arc");
                if (s_map.find(norm) != s_map.end()) {
                    continue;  // arcs/ (or higher mod) already owns it
                }
                std::filesystem::path libPath = arcsLibRoot / (stem + ".arc");
                if (!std::filesystem::is_regular_file(libPath, ec)) {
                    libPath = arcsLibRoot / (stem + ".ARC");
                    if (!std::filesystem::is_regular_file(libPath, ec)) {
                        continue;
                    }
                }
                if (s_map.emplace(norm, libPath.string()).second) {
                    claimedBy.emplace(norm, &mod);
                    if (sample.empty()) {
                        sample = norm;
                    }
                    ++count_;
                    ++libMounted;
                    DuskLog.info("[custom_assets] №110 arcs_lib fallback '{}' → {}", stem, norm);
                }
            }
            if (libMounted > 0) {
                DuskLog.info("[custom_assets] '{}' arcs_lib fallback mounted {} arc(s)", mod.name,
                             libMounted);
            }
        }

        DuskLog.info("[custom_assets] '{}'{}{}: {} overlay file(s){} (e.g. {})",
                     mod.name, mod.core ? " [core]" : "", enabled ? "" : " [disabled]",
                     count_,
                     shadowed != 0
                         ? fmt::format(", {} shadowed by higher-priority source(s) [top wins]",
                                       shadowed)
                         : "",
                     sample.empty() ? "-" : sample);
        // R2 mounts every arcs/*.arc as res/Object/<Name>.arc. A full WW Object
        // dump here (hundreds of overlays) has crashed OPENING_SCENE create.
        if (enabled && hasArcs && count_ > 80) {
            DuskLog.warn(
                "[custom_assets] '{}' has {} overlays — arcs/ should be curated "
                "(~tens), not a full Object dump",
                mod.name, count_);
        }
    }

#if D_ALBW_AUDIO_SHADOW
    int audioTwins = static_cast<int>(localAudio.size());
    {
        std::lock_guard<std::mutex> lk(s_audioMtx);
        s_audioIndex.swap(localAudio);
        rebuild_mod_wave_descriptors(sources);  // parse mod .baa -> s_modWaves (per-wave)
    }
    dusk::audio::setShadowActive(anyAudioEnabled);
    DuskLog.info("[custom_assets] audio: {} wave twin(s) indexed, custom audio {}",
                 audioTwins, anyAudioEnabled ? "ACTIVE" : "inactive");
#endif

#if D_ALBW_CUSTOM_ICONS
    {
        std::lock_guard<std::mutex> lk(s_iconMtx);
        rebuild_icon_index(sources);  // <src>/icons/*.png (enabled sources, top wins)
    }
#endif

    // Per-source texture packs (Phase 2, §4.6) — instant, textures resolve per-draw.
    rebuild_texture_packs();

    // modinfo.ini display names for the Mods list (small file per source; scans
    // only run on boot/toggle/move, never per frame).
    s_displayNames.clear();
    for (const Source& src : sources) {
        if (src.core) {
            continue;  // core packs aren't listed
        }
        const ModInfo mi = mod_info(src.name.c_str());
        if (!mi.name.empty()) {
            s_displayNames[src.name] = mi.name;
        }
    }

    // Phase-3 conflict summary (§4.5): one line per source with any collisions
    // (scan-attributed Layer-A/icon stats + texture-pack stats, merged).
    {
        std::unordered_map<std::string, FolderConflicts> merged = s_conflicts;
        for (const auto& kv : s_texConflicts) {
            FolderConflicts& c = merged[kv.first];
            c.wins += kv.second.wins;
            c.losses += kv.second.losses;
            c.overridesCore = c.overridesCore || kv.second.overridesCore;
            c.overriddenByCore = c.overriddenByCore || kv.second.overriddenByCore;
        }
        for (const auto& kv : merged) {
            const FolderConflicts& c = kv.second;
            DuskLog.info("[custom_assets] conflicts '{}': {} win(s), {} loss(es){}{}",
                         kv.first, c.wins, c.losses,
                         c.overridesCore ? " [overrides core]" : "",
                         c.overriddenByCore ? " [overridden by core]" : "");
        }
    }

    DuskLog.info("[custom_assets] scan complete: {} data-tree source(s), {} override path(s)",
                 mods, static_cast<int>(s_map.size()));

    // Plan R1/R3: rebuild NPC socket providers from enabled mods with arcs+manifests.
    dExtNpcMount_rescanProviders();
#if TARGET_PC
    // Session inventory / Ext Status claims from mod ext_inv/claims.ini
    dExtInv_rescanClaims();
#endif
}

// ============================================
// NEW CODE — ALBW Port (modinfo.ini — Fluffy-compatible mod metadata)
// ============================================
namespace {

std::string trim_ws(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r");
    if (b == std::string::npos) {
        return {};
    }
    const auto e = s.find_last_not_of(" \t\r");
    return s.substr(b, e - b + 1);
}

// Parse one modinfo.ini if it exists at dir/modinfo.ini; returns true and
// fills `out` (fields keep earlier values unless the file provides them).
bool parse_modinfo(const std::filesystem::path& dir, ModInfo& out) {
    std::error_code ec;
    const std::filesystem::path ini = dir / "modinfo.ini";
    if (!std::filesystem::exists(ini, ec)) {
        return false;
    }
    FILE* fp = std::fopen(ini.string().c_str(), "rb");
    if (fp == nullptr) {
        return false;
    }
    std::string content;
    char buf[512];
    size_t got;
    while ((got = std::fread(buf, 1, sizeof(buf), fp)) > 0) {
        content.append(buf, got);
    }
    std::fclose(fp);

    std::string screenshotName;
    size_t pos = 0;
    while (pos <= content.size()) {
        size_t nl = content.find('\n', pos);
        if (nl == std::string::npos) {
            nl = content.size();
        }
        std::string line = trim_ws(content.substr(pos, nl - pos));
        pos = nl + 1;
        const size_t eq = line.find('=');
        if (line.empty() || line[0] == ';' || line[0] == '#' || eq == std::string::npos) {
            continue;
        }
        std::string key = trim_ws(line.substr(0, eq));
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const std::string value = trim_ws(line.substr(eq + 1));
        if (key == "name") {
            out.name = value;
        } else if (key == "version") {
            out.version = value;
        } else if (key == "author") {
            out.author = value;
        } else if (key == "description") {
            // Literal "\n" -> real newline (the Fluffy forced-linebreak syntax).
            std::string desc;
            desc.reserve(value.size());
            for (size_t i = 0; i < value.size(); ++i) {
                if (value[i] == '\\' && i + 1 < value.size() && value[i + 1] == 'n') {
                    desc.push_back('\n');
                    ++i;
                } else {
                    desc.push_back(value[i]);
                }
            }
            out.description = desc;
        } else if (key == "screenshot") {
            screenshotName = value;
        } else if (key == "homepage") {
            out.homepage = value;
        } else if (key == "category") {
            if (!value.empty()) {
                out.categories.push_back(value);
            }
        }
        // AddonFor / NameAsBundle / Requirement: reserved (bundle work).
    }

    // Screenshot lives next to the ini; fall back to the conventional names.
    const char* fallbacks[] = {"screenshot.png", "screenshot.jpg", "preview.png",
                               "preview.jpg"};
    if (!screenshotName.empty()) {
        const std::filesystem::path shot = dir / screenshotName;
        if (std::filesystem::exists(shot, ec)) {
            out.screenshot = shot.string();
        }
    }
    if (out.screenshot.empty()) {
        for (const char* f : fallbacks) {
            const std::filesystem::path shot = dir / f;
            if (std::filesystem::exists(shot, ec)) {
                out.screenshot = shot.string();
                break;
            }
        }
    }
    return true;
}

}  // namespace

ModInfo mod_info(const char* folder) {
    ModInfo out;
    if (folder == nullptr) {
        return out;
    }
    const std::filesystem::path userRoot = ConfigPath / "model_replacements";
    const std::string name(folder);
    const size_t slash = name.find('/');
    if (slash == std::string::npos) {
        parse_modinfo(userRoot / name, out);
        return out;
    }

    // Collection variant: inherit the collection root's ini (author,
    // description, screenshot...), let a variant-local ini override on top,
    // and make sure the DISPLAY NAME stays per-variant — a bare collection
    // name would render 17 identical rows.
    parse_modinfo(userRoot / name.substr(0, slash), out);
    const std::string collectionName = out.name;
    out.name.clear();
    parse_modinfo(userRoot / name, out);  // variant's own ini (rare)
    if (out.name.empty() && !collectionName.empty()) {
        out.name = collectionName + ": " + name.substr(slash + 1);
    }
    return out;
}

std::string display_name(const char* folder) {
    if (folder == nullptr) {
        return {};
    }
    const auto it = s_displayNames.find(folder);
    return it != s_displayNames.end() && !it->second.empty() ? it->second
                                                             : std::string(folder);
}

FolderConflicts folder_conflicts(const char* folder) {
    if (folder == nullptr) {
        return {};
    }
    FolderConflicts out;
    const auto scanIt = s_conflicts.find(folder);
    if (scanIt != s_conflicts.end()) {
        out = scanIt->second;
    }
    const auto texIt = s_texConflicts.find(folder);
    if (texIt != s_texConflicts.end()) {
        out.wins += texIt->second.wins;
        out.losses += texIt->second.losses;
        out.overridesCore = out.overridesCore || texIt->second.overridesCore;
        out.overriddenByCore = out.overriddenByCore || texIt->second.overriddenByCore;
    }
    return out;
}

void set_mod_status_note(const char* folder, const char* note) {
    if (folder == nullptr || folder[0] == '\0') {
        return;
    }
    if (note == nullptr || note[0] == '\0') {
        s_statusNotes.erase(folder);
        return;
    }
    s_statusNotes[folder] = note;
}

const char* mod_status_note(const char* folder) {
    if (folder == nullptr || folder[0] == '\0') {
        return "";
    }
    const auto it = s_statusNotes.find(folder);
    if (it == s_statusNotes.end()) {
        return "";
    }
    return it->second.c_str();
}

int count() {
    return static_cast<int>(s_map.size());
}

std::string overlay_path_for(const char* disc_path) {
    if (disc_path == nullptr) {
        return {};
    }
    const auto it = s_map.find(normalize(disc_path));
    return it != s_map.end() ? it->second : std::string();
}

int overlay_generation() {
    return s_generation;
}

namespace {

// Program-lifetime backing store for absolute loose paths. collect_dvd_overlays()
// is re-runnable, so this pool is only ever appended to (deduped) — never cleared.
std::deque<std::string> s_pathPool;
std::unordered_map<std::string, const char*> s_pathCstr;
std::vector<std::string> s_lastLayerASig;

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

std::vector<std::string> layer_a_sig(const std::vector<DvdOverlayFile>& files) {
    std::vector<std::string> sig;
    sig.reserve(files.size());
    for (const auto& f : files) {
        sig.push_back(f.discPath + "|" + std::to_string(f.size) + "|" +
                      (f.absPathCstr != nullptr ? f.absPathCstr : ""));
    }
    std::sort(sig.begin(), sig.end());
    return sig;
}

}  // namespace

void collect_dvd_overlays(std::vector<DvdOverlayFile>& out) {
    out.clear();
    out.reserve(s_map.size());
    for (const auto& kv : s_map) {
        std::error_code ec;
        const auto sz = std::filesystem::file_size(kv.second, ec);
        if (ec) {
            continue;
        }
        out.push_back(DvdOverlayFile{"/" + kv.first, stable_path(kv.second),
                                     static_cast<std::size_t>(sz)});
    }
}

void commit_dvd_overlay_push(const std::vector<DvdOverlayFile>& layerA) {
    auto sig = layer_a_sig(layerA);
    if (sig == s_lastLayerASig) {
        DuskLog.info(
            "[custom_assets] Layer-A DVD overlay unchanged ({} file(s)) — gen {}",
            static_cast<int>(layerA.size()), s_generation);
        return;
    }
    s_lastLayerASig = std::move(sig);
    ++s_generation;
    DuskLog.info("[custom_assets] Layer-A DVD overlay committed ({} file(s), gen {})",
                 static_cast<int>(layerA.size()), s_generation);
}

void install_overlays() {
    // United push: Layer A + dusk-API .dusk overlays. Never call
    // aurora_dvd_overlay_files with Layer A alone after ModLoader exists — that
    // wipe is why clothes RARCs (Linkle/Beta) stayed vanilla at boot.
    dusk::mods::svc::overlay_sync_files();
}

// ============================================
// Layer B — loose single-BMD injection
// ============================================
namespace {

// Resolve <arc>_<index>.bmd in the model_replacements root OR any immediate
// ENABLED subfolder (one "folder per custom model", e.g. "Armogohma Custom/").
// Returns an empty path if not found. One level deep only.
//
// PRIORITY (load-order Phase 1+2, decisions D1/D4): the ROOT-DIRECT check stays
// FIRST and UNCONDITIONAL — a loose BMD placed directly in model_replacements/
// is the de-facto "always-on" slot (Boss Refinement models, WW boots) and MUST
// keep loading regardless of any order/toggle state (§5.3 watch-item — do not
// reorder or gate this). Below it, the search walks ordered_sources() — core
// packs and user mods in WINNING order (core first unless
// customModelsAllowCoreOverride) — first match wins ("top wins").
std::filesystem::path resolve_override(const char* fname) {
    std::error_code ec;
    const std::filesystem::path userRoot = ConfigPath / "model_replacements";
    if (std::filesystem::exists(userRoot, ec)) {
        std::filesystem::path direct = userRoot / fname;
        if (std::filesystem::exists(direct, ec)) {
            return direct;
        }
    }
    for (const Source& src : ordered_sources()) {
        if (!src.enabled) {
            continue;  // folder disabled in the Custom Models editor list
        }
        std::filesystem::path candidate = src.root / fname;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate;  // first (highest-priority) provider wins
        }
    }
    return {};
}

}  // namespace

// Shared Layer-B finish: sniff format, pick loader tag, pin GameHeap when requested.
static J3DModelData* load_loose_bmd_buffer(void* buffer, std::size_t size, bool pin_game_heap,
                                           const char* log_path) {
    const bmd_export::LooseBmdFormat fmt = bmd_export::sniff_loose_bmd(buffer, size);
    if (fmt == bmd_export::LooseBmdFormat::Unknown) {
        DuskLog.warn("[custom_assets] Layer-B unknown format ({} bytes): {}", size, log_path);
        return nullptr;
    }

    const u32 loader_tag = bmd_export::loader_tag_for_loose(fmt);
    JKRHeap* gameHeap = nullptr;
    JKRHeap* prevHeap = nullptr;
    if (pin_game_heap) {
        gameHeap = mDoExt_getGameHeap();
        prevHeap = (gameHeap != nullptr) ? mDoExt_setCurrentHeap(gameHeap) : nullptr;
    }

    J3DModelData* model_data = dRes_info_c::loaderBasicBmd(loader_tag, buffer);

    if (prevHeap != nullptr) {
        mDoExt_setCurrentHeap(prevHeap);
    }

    if (model_data == nullptr) {
        DuskLog.warn("[custom_assets] Layer-B load failed ({}, tag={:c}{:c}{:c}{:c}): {}",
                     fmt == bmd_export::LooseBmdFormat::J3d2 ? "J3D2" : "BMDR",
                     (loader_tag >> 24) & 0xFF, (loader_tag >> 16) & 0xFF, (loader_tag >> 8) & 0xFF,
                     loader_tag & 0xFF, log_path);
        return nullptr;
    }

    if (model_data->getMaterialNum() == 0 || model_data->getMaterialNodePointer(0) == nullptr) {
        DuskLog.warn(
            "[custom_assets] Layer-B REJECT {} — no valid material node[0] (mats={})",
            log_path, model_data->getMaterialNum());
        return nullptr;
    }

    return model_data;
}

J3DModelData* try_load(const char* arc_name, int res_index) {
    if (arc_name == nullptr) {
        return nullptr;
    }

    // ============================================
    // NEW CODE — ALBW Port
    // Resolve BEFORE the cache: the enabled-folder walk must always run so that
    // disabling a mod folder (or losing the file) takes effect at the next model
    // build (re-equip / clothes rebuild) instead of requiring a relaunch. The
    // cache previously short-circuited this — a disabled "Wind Waker Skins" kept
    // serving the WW bow/boots for the whole session.
    // ============================================
    char fname[96];
    std::snprintf(fname, sizeof(fname), "%s_%d.bmd", arc_name, res_index);

    const std::filesystem::path path = resolve_override(fname);
    if (path.empty()) {
        return nullptr;  // no ENABLED override — caller uses the arc (even if cached)
    }

    // ============================================
    // NEW CODE — ALBW Port
    // Load-once cache: return the already-loaded data if this arc:index was loaded
    // before (avoids re-malloc'ing + leaking the retained buffer on every respawn).
    // Entries are retained for the session; re-enabling a folder is an instant HIT.
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
    J3DModelData* model_data =
        load_loose_bmd_buffer(buffer, static_cast<std::size_t>(size), true, path.string().c_str());

    if (model_data == nullptr) {
        std::free(raw);  // load failed; safe to release
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
// NEW CODE — ALBW Port (Layer-B PER-FIGHT loader — no cache, no GameHeap pin)
// For a loose model owned by a RESPAWNING actor (the Armogohma reveal model): load onto
// the actor's CURRENT (solid) heap and do NOT cache across instances. try_load() above
// pins the persistent GameHeap and caches for the whole SESSION -- correct for models
// torn down by a freeAll()'d sub-heap (Link's clothes), but wrong here: sharing ONE
// J3DModelData across two independently-heaped boss instances leaves heap-bound derived
// state (the shared display list built during the FIRST boss's draw) dangling after that
// boss's solid heap frees, crashing the SECOND boss's first draw on re-entry. Loading
// per-fight on the solid heap makes buffer + derived arrays + model all live and die with
// each boss -- exactly like the vanilla arc model (which already survives re-entry). No
// cache => no cross-fight sharing => no dangling; solid-heap alloc => freed on death => no
// leak (the malloc+retain the cached path needs is only to survive the session).
// ============================================
J3DModelData* try_load_uncached(const char* arc_name, int res_index) {
    if (arc_name == nullptr) {
        return nullptr;
    }

    char fname[96];
    std::snprintf(fname, sizeof(fname), "%s_%d.bmd", arc_name, res_index);
    const std::filesystem::path path = resolve_override(fname);
    if (path.empty()) {
        DuskLog.info("[custom_assets] Layer-B per-fight {}: no override path -> vanilla", fname);
        return nullptr;  // no override — caller uses the arc
    }

    FILE* fp = std::fopen(path.string().c_str(), "rb");
    if (fp == nullptr) {
        DuskLog.warn("[custom_assets] Layer-B per-fight {}: fopen failed -> vanilla", path.string());
        return nullptr;
    }
    std::fseek(fp, 0, SEEK_END);
    const long size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(fp);
        return nullptr;
    }

    // Backing buffer on the CURRENT (actor solid) heap: loaderBasicBmd fixes up pointers
    // in place and binds to it, so it must be >=32-aligned and must outlive the model --
    // it does; they share the heap and free together on the actor's death.
    JKRHeap* heap = JKRHeap::getCurrentHeap();
    if (heap == nullptr) {
        DuskLog.warn("[custom_assets] Layer-B per-fight {}: NO current heap -> vanilla", path.string());
        std::fclose(fp);
        return nullptr;
    }
    DuskLog.info("[custom_assets] Layer-B per-fight {}: current heap free={} B, need {} B",
                 path.string(), (long)heap->getFreeSize(), size);
    void* buffer = heap->alloc(static_cast<u32>(size), 0x20);  // 32-byte aligned
    if (buffer == nullptr) {
        DuskLog.warn("[custom_assets] Layer-B per-fight {}: solid-heap alloc FAILED (need {} B, free {} B) -> vanilla",
                     path.string(), size, (long)heap->getFreeSize());
        std::fclose(fp);
        return nullptr;  // heap full -> caller falls back to the vanilla arc
    }

    const size_t got = std::fread(buffer, 1, static_cast<size_t>(size), fp);
    std::fclose(fp);
    if (got != static_cast<size_t>(size)) {
        return nullptr;  // partial read; buffer frees with the solid heap
    }

    // Load derived arrays on the current (solid) heap too -- no GameHeap pin -- so the
    // whole model is per-fight and frees with the actor.
    J3DModelData* model_data =
        load_loose_bmd_buffer(buffer, static_cast<std::size_t>(size), false, path.string().c_str());
    if (model_data == nullptr) {
        return nullptr;
    }

    DuskLog.info("[custom_assets] Layer-B per-fight load {} ({} B) on solid heap", path.string(), size);
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

    // №28 / №32 B10: a size-mismatched .aw must NOT be treated as a byte-compatible
    // whole-bank twin (vanilla WSYS offsets into a shorter/longer buffer ⇒ AV).
    // Complete re-skins like Linkle ship a matching .baa — those banks go through
    // per-wave remap_voice (mod descriptors + virtual addresses). Only refuse when
    // we have no per-wave map for this leaf (nothing safe to do with the twin).
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
        const bool sizeMatch =
            static_cast<unsigned long>(fsz) == static_cast<unsigned long>(vanilla_size);
        const auto waveMapIt = s_modWaves.find(leaf);
        const bool hasPerWave =
            waveMapIt != s_modWaves.end() && !waveMapIt->second.empty();
        if (!sizeMatch && !hasPerWave) {
            std::fclose(fp);
            s_audioBufs.erase(leaf);
            DuskLog.warn(
                "[custom_assets] audio twin '{}' REFUSED — size {} != vanilla {} "
                "and no mod .baa wave map (fall back to vanilla bank)",
                leaf, static_cast<long>(fsz), vanilla_size);
            return;
        }
        buf.assign(static_cast<size_t>(fsz), 0);
        const size_t got = std::fread(buf.data(), 1, static_cast<size_t>(fsz), fp);
        std::fclose(fp);
        if (got != static_cast<size_t>(fsz)) {
            s_audioBufs.erase(leaf);
            return;
        }
        if (sizeMatch) {
            DuskLog.info(
                "[custom_assets] audio twin '{}' resident ({} bytes, vanilla {}) @aram {:#x}",
                leaf, static_cast<long>(fsz), vanilla_size, aram_base);
        } else {
            DuskLog.info(
                "[custom_assets] audio twin '{}' resident via per-wave remap "
                "(mod {} bytes, vanilla {}) @aram {:#x}",
                leaf, static_cast<long>(fsz), vanilla_size, aram_base);
        }
    }
    // DSP redirect range = MOD buffer size. Remapped voices mint
    // kShadowVirtualBase + aram_base + modOffset; resolveShadowWave only serves
    // those virtual addresses (real ARAM stays vanilla — safe for size mismatch).
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
        // Substitute the mod descriptor and mint a VIRTUAL address for the mod
        // wave (kShadowVirtualBase + bank base + mod offset). The DSP routes
        // fetches by address alone, so this note stays latched to the mod
        // buffer for its whole life even if the toggle flips mid-note — the
        // fix for the audible corruption a mid-session toggle used to inflict
        // on sustained/looping voices (see DuskDsp.hpp).
        *out_info = wIt->second;
        *wave_ptr = static_cast<intptr_t>(dusk::audio::kShadowVirtualBase + b.aramBase +
                                          wIt->second.mOffsetStart);
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

bool write_item_icon_timg(int item_no, void* out_buf, unsigned int out_cap) {
#if D_ALBW_CUSTOM_ICONS
    if (out_buf == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lk(s_iconMtx);

    // Encode once per item and cache (readItemTexture can be called repeatedly).
    // An empty cached vector means "no valid override" — don't retry every call.
    auto it = s_iconTimgCache.find(item_no);
    if (it == s_iconTimgCache.end()) {
        std::vector<u8> timg;
        const auto pathIt = s_iconIndex.find(item_no);
        if (pathIt != s_iconIndex.end()) {
            if (!encode_icon_timg(pathIt->second, timg)) {
                timg.clear();  // failed decode/encode → cache the miss
                DuskLog.warn("[custom_assets] custom icon for item {} failed to encode",
                             item_no);
            } else {
                DuskLog.info("[custom_assets] custom icon for item {} ready ({} bytes)",
                             item_no, static_cast<int>(timg.size()));
            }
        }
        it = s_iconTimgCache.emplace(item_no, std::move(timg)).first;
    }

    const std::vector<u8>& timg = it->second;
    if (timg.empty() || timg.size() > out_cap) {
        return false;  // no override (or somehow too big) → caller uses vanilla
    }
    std::memcpy(out_buf, timg.data(), timg.size());
    return true;
#else
    (void)item_no;
    (void)out_buf;
    (void)out_cap;
    return false;
#endif
}

bool write_named_icon_timg(const char* name, void* out_buf, unsigned int out_cap) {
#if D_ALBW_CUSTOM_ICONS
    if (name == nullptr || out_buf == nullptr) {
        return false;
    }
    const std::string key = normalize(name);
    std::lock_guard<std::mutex> lk(s_iconMtx);

    auto it = s_namedIconTimgCache.find(key);
    if (it == s_namedIconTimgCache.end()) {
        std::vector<u8> timg;
        const auto pathIt = s_namedIconIndex.find(key);
        if (pathIt != s_namedIconIndex.end()) {
            if (!encode_icon_timg(pathIt->second, timg)) {
                timg.clear();
                DuskLog.warn("[custom_assets] named icon '{}' failed to encode", key);
            } else {
                DuskLog.info("[custom_assets] named icon '{}' ready ({} bytes)", key,
                             static_cast<int>(timg.size()));
            }
        }
        it = s_namedIconTimgCache.emplace(key, std::move(timg)).first;
    }

    const std::vector<u8>& timg = it->second;
    if (timg.empty() || timg.size() > out_cap) {
        return false;
    }
    std::memcpy(out_buf, timg.data(), timg.size());
    return true;
#else
    (void)name;
    (void)out_buf;
    (void)out_cap;
    return false;
#endif
}

}  // namespace dusk::custom_assets

#else  // !TARGET_PC

class J3DModelData;
namespace dusk::custom_assets {
void scan() {}
void install_overlays() {}
void collect_dvd_overlays(std::vector<DvdOverlayFile>& out) { out.clear(); }
void commit_dvd_overlay_push(const std::vector<DvdOverlayFile>&) {}
J3DModelData* try_load(const char*, int) { return nullptr; }
J3DModelData* try_load_uncached(const char*, int) { return nullptr; }
std::vector<std::string> list_folders() { return {}; }
bool is_folder_enabled(const char*) { return true; }
void toggle_folder(const char*) {}
bool move_folder(const char*, int, bool) { return false; }
bool move_folder_to(const char*, int) { return false; }
void apply_order_changes() {}
std::string mod_group_key(const char* folder) { return folder ? folder : ""; }
bool mod_is_collection_variant(const char*) { return false; }
bool select_collection_variant(const char*) { return false; }
bool set_mod_group_enabled(const char*, bool) { return false; }
std::string mod_folder_path(const char*) { return {}; }
void rescan_and_install() {}
bool move_mod_group(const char*, int, bool) { return false; }
bool move_mod_group_to(const char*, int, bool) { return false; }
std::vector<std::pair<std::string, bool>> order_view(const std::vector<std::string>&) { return {}; }
std::vector<std::string> list_core_packs() { return {}; }
void rebuild_texture_packs() {}
FolderConflicts folder_conflicts(const char*) { return {}; }
void set_mod_status_note(const char*, const char*) {}
const char* mod_status_note(const char*) { return ""; }
ModInfo mod_info(const char*) { return {}; }
std::string display_name(const char* folder) { return folder ? folder : ""; }
int count() { return 0; }
std::string overlay_path_for(const char*) { return {}; }
int overlay_generation() { return 0; }
void note_audio_wave_arc(int, const char*) {}
void acquire_audio_shadow(int, unsigned int, unsigned int) {}
void release_audio_shadow(unsigned int) {}
bool remap_voice(intptr_t*, unsigned int, JASWaveInfo*) { return false; }
bool write_item_icon_timg(int, void*, unsigned int) { return false; }
bool write_named_icon_timg(const char*, void*, unsigned int) { return false; }
}  // namespace dusk::custom_assets

#endif  // TARGET_PC
