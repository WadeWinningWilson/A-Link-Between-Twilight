#ifndef D_ALBW_COMBAT_H
#define D_ALBW_COMBAT_H

#if TARGET_PC

#include "f_op/f_op_actor.h"

enum dAlbwHelmBashTier {
    dAlbwHelmBash_THRESHOLD = 0,
    dAlbwHelmBash_MAX = 1,
};

// Bash spend required to open a Helm Splitter punish credit on this target.
// Default: threshold (tier bashThreshold for Link's shield). Armored Darknut: max bar.
dAlbwHelmBashTier dAlbwCombat_getHelmBashTier(fopAc_ac_c* i_actor);

bool dAlbw_isHiddenSkillReworkEnabled();

// Jump Strike charge gate (Hidden Skill Rework). Set once charge anim reaches frame 6.
void dAlbw_resetJumpStrikeChargeReady();
void dAlbw_setJumpStrikeChargeReady();
bool dAlbw_isJumpStrikeChargeReady();
bool dAlbw_peekJumpStrikeChargeReady();
bool dAlbw_tryConsumeJumpStrikeChargeReady();

// ============================================
// Guard-opener classification (alpha cleanup). True when the hit collider
// belongs to one of the attacks that OPEN physical enemy guards (drop the
// shield for a short window) instead of clanking: the Hurricane finisher,
// the wolf Combat Howl AOE, and the Midna arm strike. Great Spin is
// deliberately EXCLUDED (shares LARGE_TURN cut types with Hurricane, so
// this keys on the Hurricane proc, not the cut type; user may revisit).
// Normal swings / charged spin stay non-openers so blocking still reads.
// ============================================
class cCcD_Obj;
bool dAlbwCombat_isGuardOpenerHit(cCcD_Obj* i_hitObj);

// Shared open-window length for guard-opener hits — deliberately shorter
// than the shield-bash windows (90/75) so a sustained Hurricane re-hit
// chain is self-limiting rather than a permanent guard-stunlock.
constexpr u8 kAlbwGuardOpenerWindowFrames = 40;

#endif // TARGET_PC

#endif // D_ALBW_COMBAT_H
