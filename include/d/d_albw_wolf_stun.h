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

// Stun duration: 300 frames = 5 seconds at 60 fps.
static constexpr int WOLF_STUN_FRAMES = 300;

// True when the Wolf Link Combat setting is enabled (Dusk settings menu).
bool dAlbwWolfCombat_isEnabled();

// Returns true when i_name is one of the shadow/twilight enemy types.
// Twilight enemies receive 70 % damage from the field attack and are
// NOT stunned; non-twilight receive 50 % damage and ARE stunned.
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
// NEW CODE ENDS HERE
// ============================================

#endif // TARGET_PC
#endif // D_ALBW_WOLF_STUN_H
