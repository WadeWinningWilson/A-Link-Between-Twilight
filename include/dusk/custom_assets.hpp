#ifndef DUSK_CUSTOM_ASSETS_HPP
#define DUSK_CUSTOM_ASSETS_HPP

#include <cstdint>
#include <string>
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

// --- Custom Models editor list (shared toggle; gates BOTH layers) --------

// All immediate subfolder names under model_replacements/ (for the editor list).
std::vector<std::string> list_folders();

// True unless the folder is in the game.customModelsDisabled setting.
bool is_folder_enabled(const char* folder);

// Flip a folder's enabled state, rescan(), and re-install_overlays() so the
// change lands on the next asset load.
void toggle_folder(const char* folder);

// Number of Layer-A override paths currently mapped (diagnostics / UI).
int count();

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

}  // namespace dusk::custom_assets

#endif  // DUSK_CUSTOM_ASSETS_HPP
