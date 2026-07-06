#ifndef DUSK_CUSTOM_ASSETS_HPP
#define DUSK_CUSTOM_ASSETS_HPP

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

}  // namespace dusk::custom_assets

#endif  // DUSK_CUSTOM_ASSETS_HPP
