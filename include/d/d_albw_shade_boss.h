#ifndef D_ALBW_SHADE_BOSS_H
#define D_ALBW_SHADE_BOSS_H

#if TARGET_PC

// ============================================
// NEW CODE — ALBW Port — "Hero's Shade Secret Boss"
// Master toggle + helpers for the post-game Hero's Shade duel that appears
// after all Hidden Skills are learned (F_0344). WIP — full design + phased
// implementation plan in docs/heros-shade-secret-boss.md.
// ============================================

// Master enable for the Hero's Shade secret-boss system (game.heroShadeSecretBoss).
bool dShadeBoss_isEnabled();

// Per-frame BGM suppression for the test wolf's room. The caller gates on stage
// R_SP01 + roomNo 7 (Link's House basement); this silences the shared
// Ordon-interior BGM so the basement is quiet while the feature is on.
void dShadeBoss_suppressHouseBgm();

// Boss health bar query for the combat Shade. Finds the active mType-7 Hero's
// Shade (the only NPC_KN with HP > 0) and returns its current/max HP via the
// shared lock-on HP source. Returns false (bar hidden) when the feature is off
// or no live combat Shade is present. Single HP pool — no phase query needed
// (later phases are planned to stay within one pool). Mirrors the Armogohma
// query so the shared boss-bar HUD can show either boss.
bool dShadeBoss_queryHealthBar(int* o_current, int* o_max);

#endif // TARGET_PC

#endif // D_ALBW_SHADE_BOSS_H
