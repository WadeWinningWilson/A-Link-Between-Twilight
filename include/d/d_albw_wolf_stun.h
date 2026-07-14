#ifndef D_ALBW_WOLF_STUN_H
#define D_ALBW_WOLF_STUN_H

// ============================================
// NEW CODE — ALBW Port
// d_albw_wolf_stun.h
// Wolf combat system — pause-based stun module.
// Tracks per-enemy stun timers and applies/clears fpcM_PauseEnable
// (bit 1) to freeze non-twilight enemies for WOLF_STUN_FRAMES frames
// after a Midna field attack hit.
// Gated on Dusk setting game.wolfLinkCombat (default off).
// Enemies freeze mid-animation — execute() is skipped, draw() runs.
// Session-only state; no save persistence.
// ============================================

#if TARGET_PC

#include "f_op/f_op_actor.h"

class cCcD_Obj;

// Stun duration: 300 frames = 10 seconds (actor execute runs on the fixed 30 Hz sim tick —
// dusk/game_clock.h sim_pace = 1/30; the old "5 seconds at 60 fps" note was wrong).
static constexpr int WOLF_STUN_FRAMES = 300;

// True when the Wolf Link Combat setting is enabled (Dusk settings menu).
bool dAlbwWolfCombat_isEnabled();

// Returns true when i_name is one of the shadow/twilight enemy types.
// Twilight enemies receive 70 % damage from the field attack and are
// NOT stunned; non-twilight receive 25 % field-attack damage and ARE stunned.
bool dAlbwWolfStun_isTwilightEnemy(s16 i_name);

// Apply a WOLF_STUN_FRAMES-frame pause-stun to i_enemy.
// If the enemy is already in the stun list its timer is refreshed.
void dAlbwWolfStun_apply(fopAc_ac_c* i_enemy);

// Per-frame tick: decrement timers, unpause actors whose timer reaches 0.
// Called from daAlink_c::execute() every frame.
void dAlbwWolfStun_update();

// True when i_enemy is currently in the wolf stun list.
bool dAlbwWolfStun_isStunned(fopAc_ac_c* i_enemy);

// Store TG collider pointers after cc_set (reliable for actors that call this).
void dAlbwWolfStun_syncColliders(fopAc_ac_c* i_enemy, cCcD_Obj* const* i_objs, int i_count);

// Scan the cc registry after all actors execute; fills missing snapshots.
void dAlbwWolfStun_captureAfterExecute();

// Draw-phase collision bridge (Option 1): re-register frozen TG colliders
// before the global cc Move(), then dispatch cc_at_check for Link melee
// hits (wolf bites, roll attack, human sword, field-attack refresh) after.
void dAlbwWolfStun_beforeMove();
void dAlbwWolfStun_afterMove();

// ============================================
// NEW CODE — ALBW Port (Wolf Arts — Focused-Arts-style wolf abilities, shop-unlock hooks)
// The wolf "arts" (howl / Midna punch / giant) are unlocked via rental-shop purchases gated
// behind story milestones.  Save-backed per-save event bits (713 = howl; 714/715 reserved for
// punch/giant).  The rental shop (d_albw_rental.cpp) calls these to draw + purchase the row,
// mirroring the Focused-Arts-tier row pattern.
// ============================================
bool        dAlbwWolfArts_isHowlUnlocked();
void        dAlbwWolfArts_unlockHowl();
bool        dAlbwWolfArts_shouldShowHowlShopRow();  // Wolf Combat on AND not yet unlocked
int         dAlbwWolfArts_getHowlShopPrice();       // 100 rupees
const char* dAlbwWolfArts_getHowlShopName();
const char* dAlbwWolfArts_getHowlShopDesc();
bool        dAlbwWolfArts_tryPurchaseHowl();        // sets the save bit; false if already owned

// Midna Arm (D-pad Right art) — shop unlock (save event bit 714).
bool        dAlbwWolfArts_isArmUnlocked();
void        dAlbwWolfArts_unlockArm();
bool        dAlbwWolfArts_shouldShowArmShopRow();   // Wolf Combat on AND not yet unlocked
int         dAlbwWolfArts_getArmShopPrice();        // 100 rupees
const char* dAlbwWolfArts_getArmShopName();
const char* dAlbwWolfArts_getArmShopDesc();
bool        dAlbwWolfArts_tryPurchaseArm();         // sets the save bit; false if already owned

// ============================================
// NEW CODE — ALBW Port (Midna Arm art — hair-reach visual bridge)
// The arm actor (d_a_albw_midna_arm) publishes its current reach target here each frame; daAlink's
// setNeckAngle re-applies it to FLG1_MIDNA_HAIR_ATN_POS / mMidnaHairAtnPos (which it clears every
// frame), so daMidna_c's hair visibly reaches toward the strike point regardless of execute order.
// ============================================
// i_striking distinguishes the STRIKE reach (hair stretches to 3x length toward the enemy) from
// the READY/idle hover (normal hair scale, hovering above Midna so players see the hand is armed).
void        dAlbwMidnaArm_setReachPos(const cXyz& i_pos, bool i_striking);
void        dAlbwMidnaArm_clearReachPos();                 // called when the art ends / actor dies
bool        dAlbwMidnaArm_getReachPos(cXyz* o_pos);        // true + pos when a reach is active
bool        dAlbwMidnaArm_isReachStriking();               // true only during stretch/hit

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif // TARGET_PC
#endif // D_ALBW_WOLF_STUN_H
