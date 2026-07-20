#ifndef DUSK_CUSTOM_ASSETS_HPP
#define DUSK_CUSTOM_ASSETS_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// ============================================
// NEW CODE — ALBW Port / Dusklight
// Custom Assets API — repack-free asset override system. Two layers:
//
//   Layer A (DVD overlay): any file addressable by a disc path — arcs (raw OR
//     compressed), audio (Z2Sound.baa / *.aw waves), streams — is overlaid into
//     Aurora's virtual FST so the stock loaders read the loose file with no
//     per-loader hook. Compression is handled by the stock loader for free.
//   Layer B (loose BMD): a single model that lives *inside* an arc (pulled by
//     resource index, not by disc path, so Layer A can't reach it) is injected
//     directly, bypassing the arc.
//
// A mod is a folder under <config>/model_replacements/. A "files/" subtree is a
// Layer-A data-tree (mirrors the disc layout); a bare "<arc>_<index>.bmd" is a
// Layer-B single asset. Each folder is individually enable/disable-able from the
// ALBW "Custom Models" editor list; the toggle gates BOTH layers.
//
// NAMING: this namespace is the public identity of the override API. If upstream
// Dusklight adopts or renames it, that is a single find-replace of
// "dusk::custom_assets" — no structural coupling.
// ============================================

class J3DModelData;
struct JASWaveInfo;

namespace dusk::custom_assets {

// --- Layer A: whole-file DVD overlay -------------------------------------

// Scan model_replacements/<mod>/files/ trees and (re)build the game-path ->
// loose-file map for the currently ENABLED folders. Call at startup before
// install_overlays(), and again whenever the enabled set changes.
void scan();

// Register every mapped path as an Aurora DVD overlay (Layer A). RE-RUNNABLE:
// rebuilds the overlay set from the current scan map (mirrors the texture API's
// reload() idiom), so a folder toggle takes effect on the next asset load
// (reload-scoped). Call once after aurora_dvd_open, then again from
// toggle_folder(). Overlay callbacks are installed exactly once.
void install_overlays();

// --- Layer B: loose single-BMD injection ---------------------------------

// Returns a finished J3DModelData* from
// <config>/model_replacements/[<folder>/]<arc_name>_<res_index>.bmd, or nullptr
// if no (enabled) override exists / the load fails — the caller then falls back
// to the arc resource. The backing byte buffer is retained for the model's life.
J3DModelData* try_load(const char* arc_name, int res_index);

// Per-fight variant for loose models owned by a RESPAWNING actor (e.g. the Armogohma
// reveal model). Loads onto the CURRENT (actor solid) heap and does NOT cache across
// instances, so the model lives and dies with the actor like the vanilla arc model.
// Using the cached try_load() for a respawning boss shares one J3DModelData across two
// independently-heaped instances and dangles heap-bound state on re-entry. Returns
// nullptr (caller falls back to the arc) on any failure.
J3DModelData* try_load_uncached(const char* arc_name, int res_index);

// --- Custom Models editor list (ordered; shared toggle gates BOTH layers) --
//
// ============================================
// NEW CODE — ALBW Port (load-order mod system, Phase 1)
// Mods are ORDERED (game.customModelsOrder, '|'-delimited, '-' prefix =
// disabled). ONE resolution rule everywhere: TOP of the list = highest
// priority = wins conflicts (decision D1, docs/Mod-Load-Order-Design.md).
// Applied at every conflict site: Layer-A s_map, audio s_audioIndex +
// s_modWaves, icons s_iconIndex/s_namedIconIndex, and Layer-B
// resolve_override's subfolder search (its root-direct "always-on" check is
// deliberately UNTOUCHED and still beats everything). Migration: an empty
// order setting is seeded from the alphabetical folder list with the legacy
// customModelsDisabled flags carried verbatim; the legacy key stays dual-read
// (fallback for unlisted folders) + mirrored on writes for one release.
// ============================================

// All immediate subfolder names under model_replacements/, in PRIORITY order
// (index 0 = top = wins conflicts). For the editor list.
std::vector<std::string> list_folders();

// True unless the folder is disabled — '-' prefix in game.customModelsOrder,
// falling back to the legacy game.customModelsDisabled list for folders not
// yet in the order (dual-read, one release).
bool is_folder_enabled(const char* folder);

// Flip a folder's enabled state, rescan(), and re-install_overlays() so the
// change lands on the next asset load.
void toggle_folder(const char* folder);

// Move a folder one slot up (delta -1 = higher priority) or down (delta +1)
// in the load order. apply=true rescans + re-installs overlays immediately (an
// order change can change conflict winners). apply=false only persists the
// order — used by the Mods window while a grabbed mod is being dragged, so a
// 10-step drag is ONE rescan (at place) instead of ten: every scan bumps
// overlay_generation() and re-registers the FST, and a bump storm is what let
// the resident-model refresh latch a stale mount (the Linkle/Link hybrid).
// Callers passing false MUST eventually call apply_order_changes(). Returns
// false if it could not move (unknown folder, or already at that end).
bool move_folder(const char* folder, int delta, bool apply = true);

// Rescan + re-install overlays after one or more deferred (apply=false) moves.
void apply_order_changes();

// Move a folder to an arbitrary VISIBLE slot (0 = top = wins) in one step —
// one rescan total, unlike chaining move_folder. Used by the Mods tab's
// grab-and-place reorder (click a destination row while holding a mod).
// Returns false if the folder is unknown or already at that slot.
bool move_folder_to(const char* folder, int slot);

// --- Collection UI grouping (Mods window) ---------------------------------
//
// Variants are stored as "Collection/Variant" in the order setting (unchanged).
// The Mods window collapses them into one load-order row per collection root;
// move_mod_group* reorders the whole block of variant entries together.

// Load-order group key: collection root, or the folder name for a plain mod.
std::string mod_group_key(const char* folder);

// True when folder is an exploded collection variant ("Coll/Var").
bool mod_is_collection_variant(const char* folder);

// Enable one variant and disable all siblings under the same collection root.
// Returns false if folder is not a variant or already the sole enabled sibling.
bool select_collection_variant(const char* variant_folder);

// Move every order entry sharing group_key as one block (plain mods: same as
// move_folder). apply=false defers scan until apply_order_changes().
bool move_mod_group(const char* group_key, int delta, bool apply = true);

// Grab-and-place for a collapsed collection row (visible group slot).
bool move_mod_group_to(const char* group_key, int slot, bool apply = true);

// Priority-ordered (name, enabled) view of the GIVEN folder names — a disk
// snapshot the caller already holds. CHEAP: parses the order setting (+ legacy
// dual-read) only, touches no filesystem, so the editor can poll it per frame.
// Names missing from the order setting are appended in the given order.
std::vector<std::pair<std::string, bool>> order_view(const std::vector<std::string>& folders);

// --- Core tier (Phase 2, decisions D3/D4) ---------------------------------
//
// CORE packs are first-party must-have content bundled NEXT TO THE EXE at
// <exe>/content/core/<pack>/, each pack shaped exactly like a user mod folder
// (files/ data tree, loose BMDs, icons/, textures/). Always enabled, never in
// the editor order list. Resolution (D4): by default CORE WINS any conflict
// with a user mod (overriding core can break boss fights / features); the
// explicit game.customModelsAllowCoreOverride toggle flips the sequence so
// user mods win. The legacy root-level loose-BMD slot in resolve_override
// stays above BOTH (unconditional, §3's always-on slot).

// Bundled core pack names (alphabetical), for the editor's info line. Empty
// when <exe>/content/core/ does not exist (no bundled content shipped yet).
std::vector<std::string> list_core_packs();

// --- Per-mod texture packs (Phase 2, §4.6) --------------------------------
//
// Each enabled source (core pack or user mod, in winning order) may carry a
// textures/ subtree in the Aurora hash-keyed replacement layout; packs are
// registered with descending Aurora priority so the load-order winner also
// wins texture conflicts. The global <config>/texture_replacements/ folder
// keeps Aurora priority 0 = the lowest-priority fallback (back-compat).
// Rebuilt inside scan() (so every toggle/move refreshes it — textures resolve
// per-draw, so this part of a toggle is INSTANT); also callable directly for
// the enableTextureReplacements master-switch flip.
void rebuild_texture_packs();

// --- Conflict detection (Phase 3, §4.5) -----------------------------------
//
// During the ordered scan, every time a lower-priority source is skipped for
// an already-claimed asset (Layer-A file or icon slot), the winner/loser pair
// is aggregated per source. Snapshot from the LAST scan() — refreshed by every
// toggle / move / core-override flip. MO2's win/lose flags, in miniature.
struct FolderConflicts {
    int wins = 0;             // assets this source claims that shadow someone below it
    int losses = 0;           // assets it provides but loses to someone above it
    bool overridesCore = false;    // user mod winning over a core-pack asset (D4 opt-in)
    bool overriddenByCore = false; // user mod losing an asset to a core pack
};

// Conflict stats for one source (mod folder or core pack) by name; zeroed
// struct if the source had no conflicts in the last scan. Cheap map lookup.
FolderConflicts folder_conflicts(const char* folder);

// Runtime status note for Mods UI badges (e.g. population schema refuse).
// Survives scan(); pass empty note to clear. Neutral wording only (A-4 / M6).
void set_mod_status_note(const char* folder, const char* note);
// "" if none. Pointer valid until next set_mod_status_note for that folder.
const char* mod_status_note(const char* folder);

// --- modinfo.ini (mod metadata, Fluffy-Manager-compatible keys) -----------
//
// A mod may ship <mod-root>/modinfo.ini (key=value lines, keys
// case-insensitive): name, version, author, description ("\n" = forced line
// break), screenshot (image filename next to the ini; screenshot.png /
// preview.png are tried when absent), homepage, category (repeatable).
// AddonFor / NameAsBundle / Requirement are reserved for the future bundle
// work (they map onto our collection sub-pages). For a collection VARIANT
// ("Coll/Var"), the variant folder's ini wins, else the collection root's.
struct ModInfo {
    std::string name;         // display name ("" = use the folder name)
    std::string version;
    std::string author;
    std::string description;  // literal "\n" already converted to real newlines
    std::string screenshot;   // ABSOLUTE path to the preview image ("" = none)
    std::string homepage;
    std::vector<std::string> categories;
};

// Parse the mod's modinfo.ini fresh from disk (small file; call on panel
// focus, not per frame). All fields empty if there is no ini.
ModInfo mod_info(const char* folder);

// Display name for list rows: the modinfo `name` if present, else the folder
// name itself. CHEAP (map lookup — cached by scan()), safe per frame.
std::string display_name(const char* folder);

// Number of Layer-A override paths currently mapped (diagnostics / UI).
int count();

// The loose file currently winning the Layer-A overlay for this disc path
// ("/res/Layout/itemicon.arc" or "res/layout/itemicon.arc" — normalization is
// applied), or an empty string when the disc file is vanilla (no enabled mod
// provides it). Lets a BOOT-RESIDENT consumer (e.g. the menu 2D archives, see
// d_albw_menu_res) detect that ITS asset's provider changed across an
// overlay_generation() bump and re-mount only then.
std::string overlay_path_for(const char* disc_path);

// Monotonic counter bumped every time install_overlays() (re)registers the set —
// i.e. once at startup and on every folder toggle. Consumers that cache a resident
// asset built from an overlaid arc (e.g. the sumo body) compare this against their
// last-seen value to know the overlay set changed and they must re-mount.
int overlay_generation();

// --- Audio: custom-wave shadow (RUNTIME-toggleable audio) ----------------
//
// Audio can't be hot-swapped the Layer-A way: wave samples are read into ARAM
// once at boot and never re-read, and the JAudio2 subsystem cannot be re-init'd
// (its init guards against a live audio thread). So audio uses a different
// intermediary — see DuskDsp.hpp. The model overlay (Layer A) deliberately
// EXCLUDES everything under Audiores/, so vanilla audio always stays the
// resident base. Instead: the mod's byte-compatible twin of each .aw wave bank
// is read into a retained RAM buffer as the bank loads, registered with the DSP
// shadow registry, and the Custom Models toggle picks vanilla vs mod at the
// sample-fetch point — instant, no re-init. (v1 assumes a single audio mod.)

// Record a wave arc's DVD entrynum -> leaf filename (called from
// JASWaveArc::setFileName) so the loader can find the mod twin at load time.
void note_audio_wave_arc(int entrynum, const char* rel_name);

// After a vanilla wave bank loads to ARAM at [aram_base, aram_base+size): if any
// mod provides a size-matched twin of this .aw, read it into a pooled RAM buffer
// and register it with the DSP shadow registry (enable-independent, so a boot-
// disabled mod can still be toggled on live). Size mismatch -> skipped + logged.
void acquire_audio_shadow(int entrynum, unsigned int aram_base, unsigned int size);

// On wave-bank erase: drop the DSP shadow registration for this ARAM base. The
// RAM twin buffer is pooled by filename and kept for reuse, never freed mid-run
// (so an in-flight mixer read can never dangle).
void release_audio_shadow(unsigned int aram_base);

// Per-wave voice remap, called from JASBank::noteOn. Given the wave the game is
// about to play (*wave_ptr = its vanilla ARAM address, wave_id = its bank-local
// id), if the active mod ships a twin of it, fills *out_info with the MOD wave
// descriptor (offset/length/sampleCount/loop/format) and rewrites *wave_ptr to
// the mod wave's ARAM address (which the DSP redirect resolves into the mod
// buffer). Returns true if remapped; false leaves both args untouched. This is
// what makes re-encoded (differently-sized) mod voices play correctly.
bool remap_voice(intptr_t* wave_ptr, unsigned int wave_id, JASWaveInfo* out_info);

// --- Custom item/weapon/shop icons (Layer-B for textures) ----------------
//
// Kill switch: 0 compiles the icon override out.
#ifndef D_ALBW_CUSTOM_ICONS
#define D_ALBW_CUSTOM_ICONS 1
#endif

// If an ENABLED mod ships a custom icon for this item (a loose PNG at
// <mod>/icons/item_<itemNo>.png), encode it into out_buf as a GameCube ResTIMG
// (CI8 48x48 with a <=256-color RGB5A3 palette — the vanilla itemicon.arc
// format/size class; fits the game's fixed 0xC00 item-texture buffer) and return
// true; the caller (dMeter2Info_c::readItemTexture) then draws it untinted in
// place of the vanilla icon. Returns false (out_buf untouched) when there is no
// override, so the vanilla path is the automatic fallback. Result is cached per
// item; the cache is rebuilt on a folder toggle. out_cap must be >= the encoded
// size (the game passes 0xC00).
bool write_item_icon_timg(int item_no, void* out_buf, unsigned int out_cap);

// Same, but for a DEDICATED named icon slot (a loose PNG at <mod>/icons/<name>.png
// whose name is not "item_<N>"). Used for UI icons that must NOT reuse an item id
// — e.g. the Stamina Upgrade shop row asks for "stamina_upgrade". Returns false
// (out_buf untouched) when no such icon is supplied, so the caller falls back to
// its normal icon.
bool write_named_icon_timg(const char* name, void* out_buf, unsigned int out_cap);

}  // namespace dusk::custom_assets

#endif  // DUSK_CUSTOM_ASSETS_HPP
