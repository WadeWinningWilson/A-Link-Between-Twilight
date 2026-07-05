#ifndef DUSK_MOD_OVERRIDE_HPP
#define DUSK_MOD_OVERRIDE_HPP

// ============================================
// NEW CODE — ALBW Port / Dusklight
// Phase 2 of the Custom Models system: whole-mod (data-tree) override.
// A mod is a folder under <config>/model_replacements/ containing a `files/`
// subtree that mirrors the game's data layout (e.g. files/res/Object/Kmdl.arc,
// files/Audiores/...). This module scans those trees and builds a
// game-path -> loose-file map so the arc/DVD load path can be redirected to a
// loose file instead of the ISO — no repacking.
//
// PHASE 2a (this file): scan + map + lookup, LOG-ONLY. No load hooks yet.
// PHASE 2b: call find_override() from the arc-mount / DVD read path.
// ============================================

namespace dusk::mod_override {

// Scan model_replacements/<mod>/files/ trees, (re)build the override map, and
// log what was found. Call once at startup (after ConfigPath is set).
void scan();

// Look up a game data path (e.g. "res/Object/Kmdl.arc"). Returns an absolute
// loose-file path when a mod provides it, else nullptr. Case/slash-insensitive.
const char* find_override(const char* game_path);

// Number of override paths currently mapped (for diagnostics / UI).
int count();

}  // namespace dusk::mod_override

#endif  // DUSK_MOD_OVERRIDE_HPP
