#ifndef D_EXT_MOD_FLAGS_H
#define D_EXT_MOD_FLAGS_H

#if TARGET_PC

// Per-mod persistent bool flags (Phase O2 / R-O2a).
// Namespaced by mod folder name; stored in dSv_reserve_c (save-scoped).
// Keys are folder-side strings (e.g. "aryll.telescope") — never WW defaults in code.

bool dExtModFlags_get(const char* modFolder, const char* key);
void dExtModFlags_set(const char* modFolder, const char* key, bool value);
void dExtModFlags_clearMod(const char* modFolder);

#endif  // TARGET_PC

#endif /* D_EXT_MOD_FLAGS_H */
