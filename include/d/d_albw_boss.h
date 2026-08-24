/**

 * d_albw_boss.h

 * ALBW boss warp bootstrap and Boss Refinement helpers.

 */



#pragma once



#if TARGET_PC



#include "SSystem/SComponent/c_cc_d.h"

#include "f_pc/f_pc_name.h"

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_xyz.h"

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

// ============================================
// NEW CODE — ALBW Port
// Zant (B_ZANT): PER-PHASE draining pool. One actor (daB_ZANT_c) cycles mFightPhase;
// health resets to that phase's max each phase, so the bar refills per phase. The
// query tracks health / the phase PEAK (captured on phase change) and hides during
// the intro / warp / room-change / ice & last-phase demo actions. HUD-only; not
// gated on Boss Refinement. Reset called on every stage load.
// ============================================
bool dAlbwBoss_zantQueryHealthBar(int* o_current, int* o_max);
void dAlbwBoss_zantResetFightState();

// Fyrus (E_FM): single-pool bar via lock-on HP display. Hidden during intro
// (ACTION_START) and death (ACTION_END). HUD-only; not Boss-Refinement gated.
bool dAlbwBoss_fyrusQueryHealthBar(int* o_current, int* o_max);

// Fyrus §8/§9 B_GO window (Boss Refinement). Latch ≤50% enter / ≤15% shed of
// field_0x560. Spawns stuck B_GO, proxy-drains E_FM; at 15% kids unmerge and
// soldier. Stay-hollow until Fyrus END.
void dAlbwBoss_fyrusResetFightState();
void dAlbwBoss_fyrusUpdateGolemWindow(fopAc_ac_c* i_fm);
bool dAlbwBoss_fyrusGolemWindowIsLive();
bool dAlbwBoss_fyrusGolemKidsLoose();
bool dAlbwBoss_fyrusIsOurGolem(fpc_ProcID i_id);
bool dAlbwBoss_fyrusTryGolemLookPos(cXyz** o_pos);
void dAlbwBoss_fyrusTryFloorDown(fopAc_ac_c* i_fm);
void dAlbwBoss_fyrusClearGolemActor();
void dAlbwBoss_fyrusOnGolemKidsCleared();
bool dAlbwBoss_fyrusTakeResumeFightPending();
bool dAlbwBoss_fyrusStayHollow();
// Unscaled Create seed: 50 vanilla / 200 Refinement. Boss HP × and region
// apply once via dAlbwHP_tryApplyTrueMaxHp. Not for mid-fight snaps.
s16 dAlbwBoss_fyrusCreateHp();
// Kid soldier seed while the Golem window is live. Own pool (not E_FM).
s16 dAlbwBoss_fyrusKidCreateHp();

class e_fm_class;
bool dAlbwBoss_fyrusAblazePhase();
bool dAlbwBoss_fyrusHollowPhase();
bool dAlbwBoss_fyrusAblazeVulnOpen();
void dAlbwBoss_fyrusSyncFireVulnState(e_fm_class* i_fm);
void dAlbwBoss_fyrusOnAttackCommit(bool i_perfectParry);
void dAlbwBoss_fyrusOnAblazeVulnDamaged();
bool dAlbwBoss_fyrusShouldChipAblazeDamage();
void dAlbwBoss_fyrusApplyChipDamage(e_fm_class* i_fm, int i_hpBefore);

// Poison siphon: heal boss by (dmg/LinkMaxLife)*bossMax. No-op if dmg<=0.
// One heal window per spray/lunge (debounce). Call OnPoisonSprayBegin when attack starts.
void dAlbwBoss_diababaOnPoisonSprayBegin();
void dAlbwBoss_diababaOnPoisonDamage(int i_damageToLink);
// True when a B_BQ hit should siphon (upright poison, retaliation spray, or lunge).
bool dAlbwBoss_diababaHitShouldSiphon(fopAc_ac_c* i_bq);
// Side-head (B_BH) hit on Link: heal middle head by 3% of Diababa max HP.
void dAlbwBoss_diababaOnSideHeadDamage();

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

// ============================================
// NEW CODE — ALBW Port
// Morpheel (B_OB) Boss Refinement phase 1 — bubbled eye-mass:
// Chu bubble shell + surface bomb-fish → clawshot pop → exposed → core_end.
// Inactive unless Boss Refinement is on. Vanilla Lakebed fight unchanged.
// ============================================
static constexpr int kAlbwMorpheelRingBombs = 12;
// Chu Worm BMDE_SM — scaled up so the shell wraps the Morpheel eye (not top-cap only).
static constexpr f32 kAlbwMorpheelBubbleScale = 2.4f;
// Pivot of BMDE_SM sits high; pull the draw/collision center down onto the eye mass.
static constexpr f32 kAlbwMorpheelBubbleYOfs = -90.0f;
// Chu shell radius (matches E_SM simple-shadow ~195) — fish sit on this surface * scale.
static constexpr f32 kAlbwMorpheelBubbleRadius = 195.0f;
static constexpr f32 kAlbwMorpheelLungeSpeed = 55.0f;
static constexpr f32 kAlbwMorpheelTentacleRootR = 160.0f;
// Grab hold: 1 life unit (¼ heart) per second for 4 seconds, then knockback release.
static constexpr s16 kAlbwMorpheelGrabHoldFrames = 240;
static constexpr s16 kAlbwMorpheelGrabChipInterval = 60;
static constexpr int kAlbwMorpheelGrabChipDamage = 1;
static constexpr int kAlbwMorpheelTentacleCount = 8;
// Gap between sequential grab attempts in one attack pass (~1–2 s).
static constexpr s16 kAlbwMorpheelGrabGapMin = 60;
static constexpr s16 kAlbwMorpheelGrabGapMax = 120;
// Reach length while shooting at Link (vanilla HIO idle length is 70).
static constexpr f32 kAlbwMorpheelTentacleStrikeLength = 135.0f;
// Param magic for surface-escort E_OctBg children (low byte = slot 0..N-1).
static constexpr u32 kAlbwMorpheelRingBombParam = 0xA1B00000u;

enum AlbwMorpheelRefPhase {
    ALBW_MORPHEEL_REF_OFF = 0,
    ALBW_MORPHEEL_REF_RING,     // bubble + surface fish; eye not hookable
    ALBW_MORPHEEL_REF_CLAW,     // bombs cleared; clawshot eye to pop bubble
    ALBW_MORPHEEL_REF_EXPOSED,  // bubble popped; sword/hook damage as chance
    ALBW_MORPHEEL_REF_HANDOFF,  // handed to vanilla CORE_END / phase 2
};

void dAlbwBoss_morpheelResetFightState();
void dAlbwBoss_morpheelEnsureInit(fopAc_ac_c* i_boss);
bool dAlbwBoss_morpheelIsActive();
bool dAlbwBoss_morpheelFightIsLive();
void dAlbwBoss_morpheelMarkBombsSpawned();
void dAlbwBoss_morpheelSetFightLive(bool i_live);
AlbwMorpheelRefPhase dAlbwBoss_morpheelGetPhase();
bool dAlbwBoss_morpheelBubbleUp();
bool dAlbwBoss_morpheelEyeHookAllowed();
bool dAlbwBoss_morpheelEyeDamageAllowed();
void dAlbwBoss_morpheelOnEyeHooked();
void dAlbwBoss_morpheelOnEyeDepleted();
void dAlbwBoss_morpheelTick(fopAc_ac_c* i_boss);
void dAlbwBoss_morpheelTickBubbleLoad();
bool dAlbwBoss_morpheelDrawChuBubble(fopAc_ac_c* i_boss);
f32 dAlbwBoss_morpheelBubbleRadius();
bool dAlbwBoss_morpheelTryGetEyePos(cXyz* o_pos);
bool dAlbwBoss_morpheelTryRootTentacle(int i_slot, cXyz* o_pos, s16* o_homeYaw);
bool dAlbwBoss_morpheelSnapRingBomb(fopAc_ac_c* i_fish);
bool dAlbwBoss_morpheelIsRingBombParam(u32 i_param);
int dAlbwBoss_morpheelRingBombSlot(u32 i_param);
// Eye → tentacle strike pass: all 8 arms try one-at-a-time with a short gap.
// Success (grab) ends the pass; a miss advances until every tentacle has had a turn.
void dAlbwBoss_morpheelRequestTentacleGrab();
bool dAlbwBoss_morpheelConsumeTentacleGrab(int i_slot);
bool dAlbwBoss_morpheelTentacleGrabBusy();
void dAlbwBoss_morpheelNotifyTentacleStrikeMiss();
void dAlbwBoss_morpheelNotifyTentacleGrabCaught();
void dAlbwBoss_morpheelNotifyTentacleGrabHoldDone();
// Deprecated alias — treat as miss (kept for any stray call sites).
void dAlbwBoss_morpheelNotifyTentacleGrabDone();

// Boss-warp revisit session (room switches reset for mid-fight Dominion Rod).

bool dAlbwBoss_isArmogohmaWarpBootstrap();

void dAlbwBoss_onArmogohmaVictory();

void dAlbwBoss_onStageLoad();



// ============================================

// NEW CODE ENDS HERE

// ============================================



#endif

