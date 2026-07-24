/**

 * d_albw_boss.h

 * ALBW boss warp bootstrap and Boss Refinement helpers.

 */



#pragma once



#if TARGET_PC



#include "SSystem/SComponent/c_cc_d.h"

#include "f_pc/f_pc_name.h"

#include "f_op/f_op_actor.h"

struct dCcU_AtInfo;

// Boss Refinement: Armogohma eye hits use 4% chip only (see dAlbwBoss_armogohmaOnBowCoreHit).
bool dAlbwBoss_armogohmaShouldSuppressVanillaArrowDamage(fopAc_ac_c* i_boss,
                                                         dCcU_AtInfo* i_atInfo);

// ============================================

// NEW CODE — ALBW Boss Refinement

// ============================================



static constexpr int kAlbwBossWarpStageCount = 9;



enum AlbwBossArenaId {

    ALBW_BOSS_ARENA_INVALID = -1,

    ALBW_BOSS_ARENA_FOREST = 0,

    ALBW_BOSS_ARENA_GORON_MINES,

    ALBW_BOSS_ARENA_LAKEBED,

    ALBW_BOSS_ARENA_ARBITER,

    ALBW_BOSS_ARENA_SNOWPEAK,

    ALBW_BOSS_ARENA_TEMPLE_OF_TIME,

    ALBW_BOSS_ARENA_CITY_IN_SKY,

    ALBW_BOSS_ARENA_PALACE_OF_TWilight,

    ALBW_BOSS_ARENA_LAKEBED_WARP = 8,

};



bool dAlbwBossRefinement_isEnabled();



// When Boss Refinement is off, matches vanilla Master Sword checks.

bool dAlbwBossRefinement_playerHasBossSword();

bool dAlbwBossRefinement_colliderCountsAsMasterSword(dCcD_GObjInf* i_collider);



AlbwBossArenaId dAlbwBoss_stageNameToArenaId(const char* i_stageName);



void dAlbwBoss_requestWarpBootstrap(const char* i_stageName);

void dAlbwBoss_applyPendingStageBootstrap();

bool dAlbwBoss_tryApplyActorBootstrap(s16 i_procName, fopAc_ac_c* i_actor);



// Armogohma (Boss Refinement only): %-of-max HP pacing, 3 statue hits to win, 4 egg waves.

class b_gm_class;



void dAlbwBoss_armogohmaResetFightState();

void dAlbwBoss_armogohmaEnsureInitialized(fopAc_ac_c* i_boss);

void dAlbwBoss_armogohmaOnBowCoreHit(fopAc_ac_c* i_boss);

bool dAlbwBoss_armogohmaIsOnCeiling(fopAc_ac_c* i_boss);

// True once when an opening gate (75/45/15 %) was crossed; clears the pending flag.
bool dAlbwBoss_armogohmaTakeCeilingDropPending();

void dAlbwBoss_armogohmaFillDisplayHp(fopAc_ac_c* i_boss, s16* o_current, s16* o_max);

struct dAlbwBoss_ArmogohmaBarState {
    bool visible;
    u8 phase;  // 1 = B_GM body, 2 = E_GM floor eye (TYPE_GOMA / param 3)
    // ============================================
    // NEW CODE — ALBW Port
    // Composite fill: the HUD draws layout.barW * fillRatio and does NO HP math.
    // Phase 1 occupies the top half (1.0 .. 0.5), phase 2 the bottom (0.5 .. 0).
    // current/max stay informational only (lock-on overlay / F5 debug).
    // ============================================
    f32 fillRatio;
    // ============================================
    // NEW CODE ENDS HERE
    // ============================================
    s16 current;
    s16 max;
};

// Fills bar visibility + HP for the HUD (phase 1 refinement pool or phase 2 eye).
bool dAlbwBoss_armogohmaQueryHealthBar(dAlbwBoss_ArmogohmaBarState* o_state);

// Diababa (B_BQ): single-pool bar via lock-on HP display. Hidden during intro /
// death demos and while the middle head is pre-APPEAR (mDisableDraw).
bool dAlbwBoss_diababaQueryHealthBar(int* o_current, int* o_max);

// Boss Refinement: Diababa bomb reception (boss-side). Does not change global
// bomb attack power — B_BQ subtracts this on HIT_TYPE_BOMB instead of the
// vanilla hard-assign health = 50.
static constexpr int kAlbwDiababaBombReceiveDamage = 30;

// Diababa fight state (Boss Refinement). Late phase = remaining HP <= 70% max
// (bomb −30 from 100 enters late). Sticky once entered.
void dAlbwBoss_diababaResetFightState();
void dAlbwBoss_diababaUpdatePhase(fopAc_ac_c* i_boss);
bool dAlbwBoss_diababaIsLatePhase();

// Poison siphon: heal boss by (dmg/LinkMaxLife)*bossMax. No-op if dmg<=0.
// One heal window per spray (debounce). Call OnPoisonSprayBegin when spray starts.
void dAlbwBoss_diababaOnPoisonSprayBegin();
void dAlbwBoss_diababaOnPoisonDamage(int i_damageToLink);

// Phase-2 bomb cycle (HP <= 70%):
//   RUNAWAY thrash + poison/siphon → submerge+5-hit → appear → hang → vanilla spray.
void dAlbwBoss_diababaSetRetaliationPoison(bool i_active);
bool dAlbwBoss_diababaIsRetaliationPoison();
void dAlbwBoss_diababaSetPendingHangAfterAppear(bool i_pending);
bool dAlbwBoss_diababaTakePendingHangAfterAppear();

// Chip flinch: alternate LOOK_M every other chip (arrow/ball map still applies).
bool dAlbwBoss_diababaTakeChipLookMAlternate();

void dAlbwBoss_armogohmaOnRodHit(fopAc_ac_c* i_boss, s8 i_hitCount);

// Boss Refinement phase-3 (reveal) — apply one defended weapon hit to the drain
// pool. i_rawPower is the already-resolved attack power (sword tier / FA / bow /
// bomb all folded in). Halves it (the phase-3 defense divisor) before draining.
// Returns true once HP falls to the handoff threshold, telling the caller to fire
// the disappear cutscene + E_GM eye. No-op (returns false) unless Refinement is on.
bool dAlbwBoss_armogohmaPhase3Damage(fopAc_ac_c* i_boss, int i_rawPower);

bool dAlbwBoss_armogohmaTryBeginEggPhase(b_gm_class* i_boss);



// Boss-warp revisit session (room switches reset for mid-fight Dominion Rod).

bool dAlbwBoss_isArmogohmaWarpBootstrap();

void dAlbwBoss_onArmogohmaVictory();

void dAlbwBoss_onStageLoad();



// ============================================

// NEW CODE ENDS HERE

// ============================================



#endif

