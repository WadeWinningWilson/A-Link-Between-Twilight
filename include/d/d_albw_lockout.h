#ifndef D_ALBW_LOCKOUT_H
#define D_ALBW_LOCKOUT_H

#if TARGET_PC

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_cc_d.h"

// Session counters reset when lockout begins (meter hit 0) and when it ends (meter full).
void dAlbwLockout_onBegin();
void dAlbwLockout_onEnd();

// Per-frame tick (call from daAlink_c::execute). Decrements slingshot / Dom Rod confuse timers.
void dAlbwLockout_update();

void dAlbwLockout_onArrowFired();
void dAlbwLockout_onBombArrowFired();
void dAlbwLockout_onHookshotFired();
void dAlbwLockout_onDoubleHookshotFired();

bool dAlbwLockout_canFireBow();
bool dAlbwLockout_canFireBombArrow();
bool dAlbwLockout_canUseDoubleHookshot();

// Lockout slingshot hit on a common enemy: ranged-open window + 4s stun (pause-based by default).
void dAlbwLockout_onSlingshotHit(fopAc_ac_c* i_enemy);

// Same debuff without fpcM_Pause (enemies that use native wobble/stun anims, e.g. Darknut, Stalfos).
void dAlbwLockout_onSlingshotHitNative(fopAc_ac_c* i_enemy);

bool dAlbwLockout_isRangedOpened(fopAc_ac_c* i_enemy);
bool dAlbwLockout_isSlingshotStunActive(fopAc_ac_c* i_enemy);

// During lockout, scale a vanilla stun timer up to 4 seconds (120 frames @ 30fps).
int dAlbwLockout_getSlingshotStunFrames(int i_baseFrames);

// Lockout-only attack-power modifiers (call from d_cc_uty after other mods, before HP apply).
void dAlbwLockout_applyAttackPowerBoost(u16& io_attackPower, u32 i_atType);

// Dom Rod lockout confuse: allowlisted enemy attacks nearby enemies for 10s.
bool dAlbwLockout_isDomRodConfuseAllowlist(fopAc_ac_c* i_enemy);
void dAlbwLockout_onDomRodConfuseHit(fopAc_ac_c* i_enemy);
bool dAlbwLockout_isConfused(fopAc_ac_c* i_enemy);
fopAc_ac_c* dAlbwLockout_getConfuseTarget(fopAc_ac_c* i_attacker);
s16 dAlbwLockout_getConfuseAimAngleY(fopAc_ac_c* i_attacker);
s16 dAlbwLockout_getConfuseAimAngleX(fopAc_ac_c* i_attacker);
f32 dAlbwLockout_getConfuseAimDistanceXZ(fopAc_ac_c* i_attacker);
f32 dAlbwLockout_getConfuseAimDistance(fopAc_ac_c* i_attacker);
void dAlbwLockout_syncConfuseAtBits(fopAc_ac_c* i_attacker, cCcD_Obj* i_atObj);

// Flight-time proximity hit for Dom Rod ball (vanilla AT type does not register on enemy Tg).
// Segment from i_segA to i_segB catches fast-moving ball frames; i_radius is XZ distance.
fopAc_ac_c* dAlbwLockout_searchDomRodConfuseVictim(cXyz const& i_segA, cXyz const& i_segB,
                                                   f32 i_radius);

// Victim retaliates against a confused attacker that hit them (friendly fire).
bool dAlbwLockout_isProvoked(fopAc_ac_c* i_victim);
fopAc_ac_c* dAlbwLockout_getProvokeSource(fopAc_ac_c* i_victim);
void dAlbwLockout_onConfuseFriendlyFireHit(fopAc_ac_c* i_victim, fopAc_ac_c* i_attacker);

// Unified rival combat target (confuse host target or provoke source).
bool dAlbwLockout_hasRivalTarget(fopAc_ac_c* i_actor);
fopAc_ac_c* dAlbwLockout_getRivalTarget(fopAc_ac_c* i_actor);
s16 dAlbwLockout_getRivalAimAngleY(fopAc_ac_c* i_actor);
s16 dAlbwLockout_getRivalAimAngleX(fopAc_ac_c* i_actor);
f32 dAlbwLockout_getRivalAimDistanceXZ(fopAc_ac_c* i_actor);
f32 dAlbwLockout_getRivalAimDistance(fopAc_ac_c* i_actor);

#endif // TARGET_PC

#endif // D_ALBW_LOCKOUT_H
