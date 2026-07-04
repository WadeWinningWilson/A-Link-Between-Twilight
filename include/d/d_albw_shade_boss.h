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

#endif // TARGET_PC

#endif // D_ALBW_SHADE_BOSS_H
