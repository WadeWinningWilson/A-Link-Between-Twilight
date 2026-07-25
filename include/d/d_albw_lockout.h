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
bool dAlbwLockout_canUseBombling();

// ============================================
// Double claw lockout finisher
// Latch any ENEMY-group actor → wolf-style freeze hold → fly → jump-slash
// (ATP 30) → thaw when slash ends. Repeatable during lockout (no once-per-session).
// Replaces the old meter-restore perk.
// ============================================
bool        dAlbwLockout_shouldForceEnemyStick(fopAc_ac_c* i_actor);
bool        dAlbwLockout_shouldPierceHookShield(fopAc_ac_c* i_actor);
void        dAlbwLockout_onDoubleClawLatch(fopAc_ac_c* i_enemy);
bool        dAlbwLockout_isDoubleClawFlyActive();
fopAc_ac_c* dAlbwLockout_getDoubleClawTarget();
void        dAlbwLockout_onDoubleClawFlyEnded();
void        dAlbwLockout_onDoubleClawSlashBegin();
bool        dAlbwLockout_isDoubleClawSlashActive();
void        dAlbwLockout_onDoubleClawSlashEnd();
// Fixed finisher damage (after at_power_get). 0 = not active.
u16         dAlbwLockout_getDoubleClawSlashAttackPower();

// Lockout bombling: deploy (spend 1 of 2 uses, +30% base meter), track active actor.
void dAlbwLockout_onBomblingDeployed(fopAc_ac_c* i_bombling);
void dAlbwLockout_onBomblingDestroyed(fopAc_ac_c* i_bombling);
bool dAlbwLockout_isBomblingActive();
// Orbit Link at ~5u; returns true if this actor is the active lockout bombling.
bool dAlbwLockout_updateBomblingOrbit(fopAc_ac_c* i_bombling);
// Successful shield block/parry while lockout bombling is out → +1 bash charge.
void dAlbwLockout_onBlockWhileBomblingActive();

// Lockout slingshot hit on a common enemy: ranged-open window + 4s stun (pause-based by default).
void dAlbwLockout_onSlingshotHit(fopAc_ac_c* i_enemy);

// Same debuff without fpcM_Pause (enemies that use native wobble/stun anims, e.g. Darknut, Stalfos).
void dAlbwLockout_onSlingshotHitNative(fopAc_ac_c* i_enemy);

bool dAlbwLockout_isRangedOpened(fopAc_ac_c* i_enemy);
bool dAlbwLockout_isSlingshotStunActive(fopAc_ac_c* i_enemy);

// During lockout, scale a vanilla stun timer up to 4 seconds (120 frames @ 30fps).
int dAlbwLockout_getSlingshotStunFrames(int i_baseFrames);

// Lockout single clawshot tip ATP (Hero's Shade pattern: SetAtAtp on the At collider).
// 0 outside lockout / for double claw; 2 while locked (at_power_get → ~Ordon-tier).
u8 dAlbwLockout_getHookshotContactAtp();

// Lockout-only attack-power modifiers (call from d_cc_uty after other mods, before HP apply).
void dAlbwLockout_applyAttackPowerBoost(u16& io_attackPower, u32 i_atType);

// Dom Rod lockout confuse: eligible commons (+ Darknut/Aeralfos) attack nearby
// enemies for 10s. Bosses/traps are denylisted.
bool dAlbwLockout_isDomRodConfuseEligible(fopAc_ac_c* i_enemy);
// Legacy name — same as isDomRodConfuseEligible.
bool dAlbwLockout_isDomRodConfuseAllowlist(fopAc_ac_c* i_enemy);
void dAlbwLockout_onDomRodConfuseHit(fopAc_ac_c* i_enemy);
bool dAlbwLockout_isConfused(fopAc_ac_c* i_enemy);
fopAc_ac_c* dAlbwLockout_getConfuseTarget(fopAc_ac_c* i_attacker);
s16 dAlbwLockout_getConfuseAimAngleY(fopAc_ac_c* i_attacker);
s16 dAlbwLockout_getConfuseAimAngleX(fopAc_ac_c* i_attacker);
f32 dAlbwLockout_getConfuseAimDistanceXZ(fopAc_ac_c* i_attacker);
f32 dAlbwLockout_getConfuseAimDistance(fopAc_ac_c* i_attacker);
void dAlbwLockout_syncConfuseAtBits(fopAc_ac_c* i_attacker, cCcD_Obj* i_atObj);
// Central CcS::Set hook: force At vs-enemy while a rival target is active.
void dAlbwLockout_onCcObjSet(cCcD_Obj* i_obj);
// Central fopAcM_searchPlayer* redirect (returns true when rival aim applies).
bool dAlbwLockout_queryRivalAimAngleY(const fopAc_ac_c* i_actor, s16* o_angle);
bool dAlbwLockout_queryRivalAimAngleX(const fopAc_ac_c* i_actor, s16* o_angle);
bool dAlbwLockout_queryRivalAimDistanceXZ(const fopAc_ac_c* i_actor, f32* o_dist);
bool dAlbwLockout_queryRivalAimDistance(const fopAc_ac_c* i_actor, f32* o_dist);

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
