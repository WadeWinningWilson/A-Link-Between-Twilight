#ifndef D_EXT_NPC_POPULATION_H
#define D_EXT_NPC_POPULATION_H

#if TARGET_PC

struct dExtNpcManifest;

// Must match ww_bridge SCHEMA_VERSION (census columns + bridge_meta.ini).
inline constexpr int kExtPopSchemaVersion = 2;

// Spawn folder-side population for a ready BG mount (CSV + actor_map.ini).
// Coordinates: world = host + (csvPos − anchor). Idempotent per BG proc + world gen.
void dExtNpcPopulation_spawnForBg(const dExtNpcManifest& bg);
// №64: room-lane unload — allow the next enter to re-run the population pass.
void dExtNpcPopulation_clearForBg(const char* procName);
// №94: clear all population spawn latches (world-gen bump / restart).
void dExtNpcPopulation_clearAll();
// №94: latched for current world generation (spawn claimed this gen).
bool dExtNpcPopulation_isLatched(const char* procName);
// №94 self-heal: live mounts with mSpawnSrc starting with "census:".
int dExtNpcPopulation_countLiveCensus();

#endif  // TARGET_PC

#endif /* D_EXT_NPC_POPULATION_H */
