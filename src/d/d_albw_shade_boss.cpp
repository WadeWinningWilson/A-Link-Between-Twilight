#if TARGET_PC

// ============================================
// NEW CODE — ALBW Port — "Hero's Shade Secret Boss"
// Post-game real duel vs the Hero's Shade (d_a_npc_kn) after all Hidden Skills.
// See docs/heros-shade-secret-boss.md for the full design + implementation plan.
// This file starts as just the master toggle; boss/warp/spawn logic lands in
// later milestones.
// ============================================

#include "d/d_albw_shade_boss.h"
#include "dusk/settings.h"
#include "m_Do/m_Do_audio.h"  // mDoAud_bgmStop

// ============================================
// Master toggle (game.heroShadeSecretBoss, default off — WIP)
// ============================================
bool dShadeBoss_isEnabled() {
    return dusk::getSettings().game.heroShadeSecretBoss.getValue();
}

// ============================================
// BGM suppression — Link's House basement (test wolf room)
// Called every frame from dScnRoom_Execute while the player is in R_SP01
// room 7 with the feature on. Instant stop (fade 0) so the shared
// Ordon-interior BGM cuts and stays silent; idempotent no-op once stopped, so
// it is safe to call per frame. Only the main BGM is affected — the wolf's
// panting and other SE keep playing.
// ============================================
void dShadeBoss_suppressHouseBgm() {
    mDoAud_bgmStop(0);
}

#endif // TARGET_PC
