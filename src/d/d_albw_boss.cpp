#if TARGET_PC

#include "d/d_albw_boss.h"

#include "Z2AudioLib/Z2Instances.h"
#include "d/actor/d_a_b_bq.h"
#include "d/actor/d_a_b_gm.h"
#include "d/actor/d_a_b_go.h"
#include "d/actor/d_a_b_ob.h"
#include "d/actor/d_a_b_zant.h"
#include "d/actor/d_a_e_fm.h"
#include "d/actor/d_a_e_gm.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_albw_hp_mult.h"
#include "d/d_cc_uty.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "dusk/settings.h"
#include "m_Do/m_Do_ext.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

// ============================================
// NEW CODE — ALBW Boss Refinement
// ============================================

static AlbwBossArenaId s_warpBootstrapArena = ALBW_BOSS_ARENA_INVALID;
static bool s_armogohmaWarpBootstrap = false;

static constexpr int kAlbwArmogohmaEggGateCount = 4;
static constexpr int kAlbwArmogohmaEggGatePct[kAlbwArmogohmaEggGateCount] = {85, 75, 65, 20};
static constexpr int kAlbwArmogohmaOpeningPct[3] = {75, 45, 15};
static constexpr int kAlbwArmogohmaPostStatueSnapPct[2] = {60, 35};
static constexpr int kAlbwArmogohmaBowChipPct = 4;

// ============================================
// NEW CODE — ALBW Port (phase-3 reveal drain)
// After the 2nd statue hit the reveal fight enters a ground-chase phase that
// drains the last 35% -> handoff via real weapon damage. kPhase3DefenseDiv is the
// single tunable "defense" (2 = 50% off; raise for a tankier eye). Per-source
// defense / reactive eye-close are deferred (see docs Boss-Fights-RefinedGohma §11).
// Handoff at <=5% (not 0) so a big last hit can't overshoot the disappear cutscene.
// ============================================
static constexpr int kAlbwArmogohmaPhase3DefenseDiv = 2;
static constexpr int kAlbwArmogohmaPhase3HandoffPct = 5;

// ============================================
// NEW CODE — ALBW Port
// Composite health-bar fill mapping (see docs/albw-armogohma-boss-bar-spec.md).
// One bar, one fill ratio. Phase 1 (giant) fills the TOP half (1.0 .. 0.5),
// phase 2 (floor eye) the BOTTOM half (0.5 .. 0.0). Phase-1 progress source is
// mode-aware: Boss Refinement drains an HP pool, but vanilla B_GM.health is a
// constant 500, so vanilla drives progress off the Dominion Rod statue-drop
// counter (mHitCount 0..3) instead — mirroring phase 2's 3-hit counter.
// ============================================
static constexpr f32 kAlbwArmogohmaPhase1FillMin = 0.5f;  // giant segment floor
static constexpr f32 kAlbwArmogohmaPhase2FillMax = 0.5f;  // eye segment ceiling
static constexpr int kAlbwArmogohmaPhase1RodMax = 3;      // mHitCount statue drops
static constexpr int kAlbwArmogohmaPhase2HitMax = 3;      // field_0xa74 (l_damage_count[TYPE_GOMA])
// ============================================
// NEW CODE ENDS HERE
// ============================================

static bool s_armogohmaInitialized = false;
static s16 s_armogohmaMaxHp = 0;
static s16 s_armogohmaPrevHp = 0;
static bool s_armogohmaEggQueued[kAlbwArmogohmaEggGateCount] = {};
static bool s_armogohmaEggUsed[kAlbwArmogohmaEggGateCount] = {};
static bool s_armogohmaOpeningCrossed[3] = {};
static bool s_armogohmaPendingCeilingDrop = false;
static bool s_armogohmaBarSuppressed = false;

static const char* const s_bossArenaStageNames[kAlbwBossWarpStageCount] = {
    "D_MN05A",
    "D_MN04A",
    "D_MN01A",
    "D_MN10A",
    "D_MN11A",
    "D_MN06A",
    "D_MN07A",
    "D_MN08A",
    "D_MN01A",
};

bool dAlbwBossRefinement_isEnabled() {
    return dusk::getSettings().game.bossRefinement.getValue();
}

bool dAlbwBoss_armogohmaShouldSuppressVanillaArrowDamage(fopAc_ac_c* i_boss,
                                                         dCcU_AtInfo* i_atInfo) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL || i_atInfo == NULL) {
        return false;
    }

    if (fopAcM_GetName(i_boss) != fpcNm_B_GM_e) {
        return false;
    }

    if (i_atInfo->mpCollider == NULL || !i_atInfo->mpCollider->ChkAtType(AT_TYPE_ARROW)) {
        return false;
    }

    return true;
}

bool dAlbwBossRefinement_playerHasBossSword() {
    if (!dAlbwBossRefinement_isEnabled()) {
        return daPy_py_c::checkMasterSwordEquip() != FALSE;
    }

    return daPy_py_c::checkSwordGet() != FALSE;
}

bool dAlbwBossRefinement_colliderCountsAsMasterSword(dCcD_GObjInf* i_collider) {
    if (i_collider == NULL) {
        return false;
    }

    if (i_collider->ChkAtType(AT_TYPE_MASTER_SWORD)) {
        return true;
    }

    if (dAlbwBossRefinement_isEnabled() && i_collider->ChkAtType(AT_TYPE_NORMAL_SWORD)) {
        return daPy_py_c::checkSwordGet() != FALSE;
    }

    return false;
}

AlbwBossArenaId dAlbwBoss_stageNameToArenaId(const char* i_stageName) {
    if (i_stageName == NULL) {
        return ALBW_BOSS_ARENA_INVALID;
    }

    for (int i = 0; i < kAlbwBossWarpStageCount; i++) {
        if (strcmp(i_stageName, s_bossArenaStageNames[i]) == 0) {
            return static_cast<AlbwBossArenaId>(i);
        }
    }

    return ALBW_BOSS_ARENA_INVALID;
}

static void clearWarpBootstrapSession() {
    s_warpBootstrapArena = ALBW_BOSS_ARENA_INVALID;
    s_armogohmaWarpBootstrap = false;
}

static void* resetArmogohmaCstatueSwitches(void* i_actor, void* i_data) {
    (void)i_data;
    fopAc_ac_c* actor = (fopAc_ac_c*)i_actor;
    if (!fopAcM_IsActor(actor) || fopAcM_GetName(actor) != fpcNm_CSTATUE_e) {
        return NULL;
    }

    for (int sw = 0; sw <= 6; sw++) {
        if (fopAcM_isSwitch(actor, sw)) {
            fopAcM_offSwitch(actor, sw);
        }
    }

    return NULL;
}

static void resetArmogohmaCstatueActors() {
    fpcM_Search(resetArmogohmaCstatueSwitches, NULL);
}

static void applyArmogohmaStageFixes() {
    // Reset boss-room state so Dominion Rod statues can spawn/use mid-fight after warp-in.
    for (int sw = 0; sw <= 6; sw++) {
        dComIfGs_offOneZoneSwitch(sw, -1);
    }

    resetArmogohmaCstatueActors();
}

bool dAlbwBoss_isArmogohmaWarpBootstrap() {
    return dAlbwBossRefinement_isEnabled() && s_armogohmaWarpBootstrap;
}

void dAlbwBoss_armogohmaResetFightState() {
    s_armogohmaInitialized = false;
    s_armogohmaMaxHp = 0;
    s_armogohmaPrevHp = 0;

    for (int i = 0; i < kAlbwArmogohmaEggGateCount; i++) {
        s_armogohmaEggQueued[i] = false;
        s_armogohmaEggUsed[i] = false;
    }

    for (int i = 0; i < 3; i++) {
        s_armogohmaOpeningCrossed[i] = false;
    }

    s_armogohmaPendingCeilingDrop = false;
    s_armogohmaBarSuppressed = false;
}

static s16 armogohmaHpFromPercent(int i_percent) {
    if (s_armogohmaMaxHp <= 0) {
        return 1;
    }

    const int hp = static_cast<int>(s_armogohmaMaxHp) * i_percent / 100;
    return static_cast<s16>(std::max(1, std::min(hp, static_cast<int>(s_armogohmaMaxHp))));
}

static void armogohmaSetHp(fopAc_ac_c* i_boss, s16 i_hp) {
    const s16 clamped = static_cast<s16>(std::max(1, std::min(static_cast<int>(i_hp), static_cast<int>(s_armogohmaMaxHp))));
    i_boss->health = clamped;
}

static void armogohmaProcessThresholdCrossings(s16 i_prevHp, s16 i_newHp) {
    if (s_armogohmaMaxHp <= 0 || i_prevHp <= i_newHp) {
        return;
    }

    for (int i = 0; i < kAlbwArmogohmaEggGateCount; i++) {
        const s16 gate = armogohmaHpFromPercent(kAlbwArmogohmaEggGatePct[i]);
        if (i_prevHp > gate && i_newHp <= gate) {
            s_armogohmaEggQueued[i] = true;
        }
    }

    for (int i = 0; i < 3; i++) {
        const s16 gate = armogohmaHpFromPercent(kAlbwArmogohmaOpeningPct[i]);
        if (i_prevHp > gate && i_newHp <= gate) {
            s_armogohmaOpeningCrossed[i] = true;
            s_armogohmaPendingCeilingDrop = true;
        }
    }
}

static void armogohmaApplyHpChange(fopAc_ac_c* i_boss, s16 i_newHp) {
    const s16 prev = i_boss->health;
    armogohmaSetHp(i_boss, i_newHp);
    armogohmaProcessThresholdCrossings(prev, i_boss->health);
    s_armogohmaPrevHp = i_boss->health;
}

void dAlbwBoss_armogohmaEnsureInitialized(fopAc_ac_c* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }

    const s16 maxFromActor = i_boss->field_0x560 > 0 ? i_boss->field_0x560 : i_boss->health;

    if (!s_armogohmaInitialized) {
        s_armogohmaMaxHp = maxFromActor > 0 ? maxFromActor : i_boss->health;
        if (s_armogohmaMaxHp <= 0) {
            s_armogohmaMaxHp = 500;
        }

        if (i_boss->health <= 0 || i_boss->health > s_armogohmaMaxHp) {
            i_boss->health = s_armogohmaMaxHp;
        }

        s_armogohmaPrevHp = i_boss->health;
        s_armogohmaInitialized = true;
        return;
    }

    if (maxFromActor > s_armogohmaMaxHp) {
        s_armogohmaMaxHp = maxFromActor;
        s_armogohmaPrevHp = i_boss->health;
    }
}

void dAlbwBoss_armogohmaOnBowCoreHit(fopAc_ac_c* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }

    dAlbwBoss_armogohmaEnsureInitialized(i_boss);

    const int chip = static_cast<int>(s_armogohmaMaxHp) * kAlbwArmogohmaBowChipPct / 100;
    if (chip <= 0) {
        return;
    }

    const s16 newHp = static_cast<s16>(std::max(1, static_cast<int>(i_boss->health) - chip));
    armogohmaApplyHpChange(i_boss, newHp);
}

bool dAlbwBoss_armogohmaIsOnCeiling(fopAc_ac_c* i_boss) {
    if (i_boss == NULL) {
        return false;
    }

    return i_boss->current.pos.y > 1000.0f;
}

bool dAlbwBoss_armogohmaTakeCeilingDropPending() {
    if (!s_armogohmaPendingCeilingDrop) {
        return false;
    }

    s_armogohmaPendingCeilingDrop = false;
    return true;
}

void dAlbwBoss_armogohmaFillDisplayHp(fopAc_ac_c* i_boss, s16* o_current, s16* o_max) {
    if (i_boss == NULL || o_current == NULL || o_max == NULL) {
        return;
    }

    *o_current = i_boss->health;
    *o_max = i_boss->field_0x560 > 0 ? i_boss->field_0x560 : i_boss->health;

    if (!dAlbwBossRefinement_isEnabled()) {
        return;
    }

    dAlbwBoss_armogohmaEnsureInitialized(i_boss);
    if (s_armogohmaMaxHp > 0) {
        *o_max = s_armogohmaMaxHp;
    }
    *o_current = i_boss->health;
}

static void* searchArmogohmaPhase2Eye(void* i_actor, void* i_data) {
    (void)i_data;
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e &&
        fopAcM_GetParam(i_actor) == 3)
    {
        return i_actor;
    }

    return NULL;
}

static fopAc_ac_c* findArmogohmaPhase2Eye() {
    return (fopAc_ac_c*)fpcM_Search(searchArmogohmaPhase2Eye, NULL);
}

bool dAlbwBoss_armogohmaResolveBarTarget(fopAc_ac_c** o_actor) {
    if (o_actor == NULL) {
        return false;
    }

    *o_actor = NULL;

    if (s_armogohmaBarSuppressed) {
        return false;
    }

    fopAc_ac_c* gmActor = fopAcM_SearchByName(fpcNm_B_GM_e);
    if (gmActor == NULL || !fopAcM_IsActor(gmActor)) {
        return false;
    }

    const b_gm_class* boss = (const b_gm_class*)gmActor;

    if (boss->mDemoMode >= 50) {
        return false;
    }

    if (boss->mIsDisappear != 0) {
        if (boss->mDemoMode >= 40) {
            return false;
        }

        fopAc_ac_c* eye = findArmogohmaPhase2Eye();
        if (eye == NULL || !fopAcM_IsActor(eye) || eye->health <= 0) {
            return false;
        }

        *o_actor = eye;
        return true;
    }

    if (gmActor->health <= 0) {
        return false;
    }

    *o_actor = gmActor;
    return true;
}

// ============================================
// MODIFIED CODE — ALBW Port
// Was: raw {current,max} per phase (phase 2 read the eye's useless 1/1 health,
// leaving the bar stuck full). Now: normalized composite fillRatio. Phase 1 is
// mode-aware (refinement pool vs vanilla rod-hit counter); phase 2 reads the
// TYPE_GOMA hit counter. current/max are kept for the lock-on overlay / F5 only.
// ============================================
// ============================================
// NEW CODE — ALBW Port (Diababa refined fight)
// ============================================
static bool s_diababaLateSticky = false;
static bool s_diababaRetaliationPoison = false;
static bool s_diababaPendingHangAfterAppear = false;
static bool s_diababaSiphonUsedThisSpray = false;
static bool s_diababaChipLookMNext = false;

static constexpr f32 kAlbwDiababaLatePhaseHpFrac = 0.7f;

void dAlbwBoss_diababaResetFightState() {
    s_diababaLateSticky = false;
    s_diababaRetaliationPoison = false;
    s_diababaPendingHangAfterAppear = false;
    s_diababaSiphonUsedThisSpray = false;
    s_diababaChipLookMNext = false;
}

void dAlbwBoss_diababaUpdatePhase(fopAc_ac_c* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }
    if (s_diababaLateSticky) {
        return;
    }
    if (i_boss->field_0x560 <= 0) {
        return;
    }
    // Late at remaining HP <= 70% max (bomb −30 from full 100 enters late).
    if (static_cast<f32>(i_boss->health) <=
        kAlbwDiababaLatePhaseHpFrac * static_cast<f32>(i_boss->field_0x560))
    {
        s_diababaLateSticky = true;
    }
}

bool dAlbwBoss_diababaIsLatePhase() {
    return dAlbwBossRefinement_isEnabled() && s_diababaLateSticky;
}

void dAlbwBoss_diababaOnPoisonSprayBegin() {
    s_diababaSiphonUsedThisSpray = false;
}

bool dAlbwBoss_diababaHitShouldSiphon(fopAc_ac_c* i_bq) {
    if (!dAlbwBossRefinement_isEnabled() || i_bq == NULL) {
        return false;
    }
    if (s_diababaRetaliationPoison) {
        return true;
    }
    // Keep in sync with daB_BQ_ACT in d_a_b_bq.cpp (ATTACK=2, LUNGE=7).
    const s16 act = ((b_bq_class*)i_bq)->mAction;
    return act == 2 || act == 7;
}

void dAlbwBoss_diababaOnPoisonDamage(int i_damageToLink) {
    if (!dAlbwBossRefinement_isEnabled() || i_damageToLink <= 0) {
        return;
    }
    if (s_diababaSiphonUsedThisSpray) {
        return;
    }

    fopAc_ac_c* boss = fopAcM_SearchByName(fpcNm_B_BQ_e);
    if (boss == NULL || boss->field_0x560 <= 0) {
        return;
    }

    const int linkMax = static_cast<int>(dComIfGs_getMaxLifeGauge());
    if (linkMax <= 0) {
        return;
    }

    // Mirror % of Link max lost → % of Diababa max healed (at least 1 if any dmg).
    int heal = (i_damageToLink * static_cast<int>(boss->field_0x560)) / linkMax;
    if (heal < 1) {
        heal = 1;
    }

    const int maxHp = static_cast<int>(boss->field_0x560);
    int newHp = static_cast<int>(boss->health) + heal;
    if (newHp > maxHp) {
        newHp = maxHp;
    }
    boss->health = static_cast<s16>(newHp);
    s_diababaSiphonUsedThisSpray = true;
    // Sticky late: siphon must not clear phase.
    dAlbwBoss_diababaUpdatePhase(boss);
}

void dAlbwBoss_diababaOnSideHeadDamage() {
    if (!dAlbwBossRefinement_isEnabled()) {
        return;
    }

    fopAc_ac_c* boss = fopAcM_SearchByName(fpcNm_B_BQ_e);
    if (boss == NULL || boss->field_0x560 <= 0) {
        return;
    }

    // Flat 3% of Diababa max HP (not Link-scaled).
    int heal = (static_cast<int>(boss->field_0x560) * 3) / 100;
    if (heal < 1) {
        heal = 1;
    }

    const int maxHp = static_cast<int>(boss->field_0x560);
    int newHp = static_cast<int>(boss->health) + heal;
    if (newHp > maxHp) {
        newHp = maxHp;
    }
    boss->health = static_cast<s16>(newHp);
    dAlbwBoss_diababaUpdatePhase(boss);
}

void dAlbwBoss_diababaSetRetaliationPoison(bool i_active) {
    s_diababaRetaliationPoison = i_active;
}

bool dAlbwBoss_diababaIsRetaliationPoison() {
    return dAlbwBossRefinement_isEnabled() && s_diababaRetaliationPoison;
}

void dAlbwBoss_diababaSetPendingHangAfterAppear(bool i_pending) {
    s_diababaPendingHangAfterAppear = i_pending;
}

bool dAlbwBoss_diababaTakePendingHangAfterAppear() {
    if (!s_diababaPendingHangAfterAppear) {
        return false;
    }
    s_diababaPendingHangAfterAppear = false;
    return true;
}

bool dAlbwBoss_diababaTakeChipLookMAlternate() {
    const bool useLook = s_diababaChipLookMNext;
    s_diababaChipLookMNext = !s_diababaChipLookMNext;
    return useLook;
}
// ============================================

bool dAlbwBoss_diababaQueryHealthBar(int* o_current, int* o_max) {
    if (o_current == NULL || o_max == NULL) {
        return false;
    }
    *o_current = 0;
    *o_max = 0;

    fopAc_ac_c* actor = fopAcM_SearchByName(fpcNm_B_BQ_e);
    if (actor == NULL || !fopAcM_IsActor(actor) || actor->health <= 0) {
        return false;
    }

    const b_bq_class* boss = (const b_bq_class*)actor;
    // Death cutscene chain starts at demo 50; non-zero demos are intro / room cams.
    if (boss->mDemoMode != 0 || boss->mDisableDraw) {
        return false;
    }

    const dAlbwHP_LockonDisplay hp = dAlbwHP_getLockonDisplayHp(actor);
    if (hp.max <= 0) {
        return false;
    }
    *o_current = hp.current;
    *o_max = hp.max;
    return true;
}

// ============================================
// NEW CODE — ALBW Port
// Zant health bar — PER-PHASE pool. Zant is one fopEn_enemy_c (daB_ZANT_c) whose
// health resets to the phase max at each mFightPhase (280 for most, field_0x560 for
// the last), so the bar refills per phase. We track the phase PEAK health as the max:
// captured on phase change and corrected each frame, so the reset value (the peak) is
// always the denominator regardless of the exact reset-vs-phase-flip frame ordering.
// The bar hides during the non-combat set-piece actions so it never flashes between
// the reused arenas. HUD-only (not Boss-Refinement gated); driven by the HUD tick.
// ============================================
static u8 s_zantTrackedPhase = 0xFF;  // last-seen mFightPhase (0xFF = none)
static s16 s_zantPhaseMax = 0;        // peak health this phase (= the phase reset value)

void dAlbwBoss_zantResetFightState() {
    s_zantTrackedPhase = 0xFF;
    s_zantPhaseMax = 0;
}

bool dAlbwBoss_zantQueryHealthBar(int* o_current, int* o_max) {
    if (o_current == NULL || o_max == NULL) {
        return false;
    }
    *o_current = 0;
    *o_max = 0;

    fopAc_ac_c* actor = fopAcM_SearchByName(fpcNm_B_ZANT_e);
    if (actor == NULL || !fopAcM_IsActor(actor) || actor->health <= 0) {
        return false;
    }

    const daB_ZANT_c* zant = (const daB_ZANT_c*)actor;

    // Hide during non-combat set-piece actions (intro / warps / room change / the ice
    // and last-phase demos) so the bar doesn't flash between the reused arenas.
    switch (zant->mAction) {
    case daB_ZANT_c::ACT_OPENING:
    case daB_ZANT_c::ACT_WARP:
    case daB_ZANT_c::ACT_ROOM_CHANGE:
    case daB_ZANT_c::ACT_ICE_DEMO:
    case daB_ZANT_c::ACT_LAST_START_DEMO:
    case daB_ZANT_c::ACT_LAST_END_DEMO:
        return false;
    default:
        break;
    }

    // Per-phase peak = the phase's reset (full) value; health drains from there.
    if (zant->mFightPhase != s_zantTrackedPhase) {
        s_zantTrackedPhase = zant->mFightPhase;
        s_zantPhaseMax = actor->health;
    }
    if (actor->health > s_zantPhaseMax) {
        s_zantPhaseMax = actor->health;
    }
    if (s_zantPhaseMax <= 0) {
        return false;
    }

    *o_current = actor->health;
    *o_max = s_zantPhaseMax;
    return true;
}

// ============================================
// NEW CODE — ALBW Port
// Fyrus health bar — single pool via lock-on HP display (Boss HP scaler). Hide
// during intro (ACTION_START) and death (ACTION_END) only; knockdown / look-pass
// stay visible. HUD-only (not Boss-Refinement gated).
// Matches daE_FM_ACTION in d_a_e_fm.cpp (enum is TU-local, not in the header).
// ============================================
static constexpr s16 kAlbwFmActionDown = 9;
static constexpr s16 kAlbwFmActionADown = 10;
static constexpr s16 kAlbwFmActionStart = 11;
static constexpr s16 kAlbwFmActionEnd = 12;

bool dAlbwBoss_fyrusQueryHealthBar(int* o_current, int* o_max) {
    if (o_current == NULL || o_max == NULL) {
        return false;
    }
    *o_current = 0;
    *o_max = 0;

    fopAc_ac_c* actor = fopAcM_SearchByName(fpcNm_E_FM_e);
    if (actor == NULL || !fopAcM_IsActor(actor)) {
        return false;
    }

    const e_fm_class* fm = (const e_fm_class*)actor;
    if (fm->mAction == kAlbwFmActionStart || fm->mAction == kAlbwFmActionEnd) {
        return false;
    }
    // 0 HP during floor-down still shows an empty bar until last-hit END.
    if (actor->health <= 0 && fm->mAction != kAlbwFmActionDown &&
        fm->mAction != kAlbwFmActionADown)
    {
        return false;
    }

    const dAlbwHP_LockonDisplay hp = dAlbwHP_getLockonDisplayHp(actor);
    if (hp.max <= 0) {
        return false;
    }
    *o_current = hp.current;
    *o_max = hp.max;
    return true;
}

// ============================================
// NEW CODE — ALBW Boss Refinement
// Fyrus §8/§9: 50% → stuck B_GO (proxy E_FM) → 15% shed kids, stay hollow.
// Native pieces: E_FM fire-off (792 + PUTOUT), B_GO Create/stick (field_0x692),
// B_GO unused sph/cyl, cc_at_check drain. Latch is the Diababa HP% pattern.
// ============================================
static constexpr int kAlbwFmGolemEnterPct = 50;
static constexpr int kAlbwFmGolemExitPct = 15;
static constexpr s16 kAlbwFmVanillaHp = 50;
static constexpr s16 kAlbwFmRefinementHp = 200;
static constexpr s16 kAlbwFmKidHp = 30;

static u8 s_fyrusGolemPhase = 0;  // 0 none, 1 clump live, 2 kids loose, 3 kids done
static fpc_ProcID s_fyrusGolemId = fpcM_ERROR_PROCESS_ID_e;
static u8 s_fyrusResumeFightPending = 0;

// §10 ablaze vulnerability counter (pre-50% only).
static constexpr int kAlbwFmAblazeVulnCredits = 14;
static u8 s_fyrusAttackCredits = 0;
static u8 s_fyrusAblazeVulnOpen = 0;

static bool fyrusHpAtMostPct(fopAc_ac_c* i_fm, int i_pct) {
    if (i_fm == NULL || i_fm->field_0x560 <= 0) {
        return false;
    }
    return static_cast<int>(i_fm->health) * 100 <= static_cast<int>(i_fm->field_0x560) * i_pct;
}

static void fyrusDespawnGolem() {
    fopAc_ac_c* golem = NULL;
    if (s_fyrusGolemId != fpcM_ERROR_PROCESS_ID_e) {
        golem = fopAcM_SearchByID(s_fyrusGolemId);
    }
    if (golem != NULL && fopAcM_IsActor(golem) && fopAcM_GetName(golem) == fpcNm_B_GO_e) {
        b_go_class* go = (b_go_class*)golem;
        for (int i = 0; i < GORON_CHILD_MAX; i++) {
            const fpc_ProcID childId = go->mGoronChildIDs[i];
            if (childId != fpcM_ERROR_PROCESS_ID_e) {
                fopAcM_delete(childId);
            }
        }
        fopAcM_delete(golem);
    } else if (s_fyrusGolemId != fpcM_ERROR_PROCESS_ID_e) {
        fopAcM_delete(s_fyrusGolemId);
        s_fyrusGolemId = fpcM_ERROR_PROCESS_ID_e;
    }
    daB_GO_setDisplayModelImage(false);
}

void dAlbwBoss_fyrusResetFightState() {
    s_fyrusGolemPhase = 0;
    s_fyrusGolemId = fpcM_ERROR_PROCESS_ID_e;
    s_fyrusResumeFightPending = 0;
    s_fyrusAttackCredits = 0;
    s_fyrusAblazeVulnOpen = 0;
    daB_GO_setDisplayModelImage(false);
}

bool dAlbwBoss_fyrusAblazePhase() {
    return dAlbwBossRefinement_isEnabled() && s_fyrusGolemPhase == 0;
}

bool dAlbwBoss_fyrusHollowPhase() {
    return dAlbwBossRefinement_isEnabled() && s_fyrusGolemPhase == 3;
}

bool dAlbwBoss_fyrusAblazeVulnOpen() {
    return s_fyrusAblazeVulnOpen != 0;
}

void dAlbwBoss_fyrusOnAttackCommit(bool i_perfectParry) {
    if (!dAlbwBoss_fyrusAblazePhase() || s_fyrusAblazeVulnOpen) {
        return;
    }
    const int add = i_perfectParry ? 2 : 1;
    const int next = static_cast<int>(s_fyrusAttackCredits) + add;
    if (next >= kAlbwFmAblazeVulnCredits) {
        s_fyrusAttackCredits = static_cast<u8>(kAlbwFmAblazeVulnCredits);
        s_fyrusAblazeVulnOpen = 1;
    } else {
        s_fyrusAttackCredits = static_cast<u8>(next);
    }
}

void dAlbwBoss_fyrusOnAblazeVulnDamaged() {
    s_fyrusAblazeVulnOpen = 0;
    s_fyrusAttackCredits = 0;
}

void dAlbwBoss_fyrusSyncFireVulnState(e_fm_class* i_fm) {
    if (!dAlbwBossRefinement_isEnabled() || i_fm == NULL) {
        return;
    }
    if (dAlbwBoss_fyrusAblazePhase()) {
        if (s_fyrusAblazeVulnOpen) {
            i_fm->field_0x792 = 0;
            i_fm->field_0x770 = 1;
        } else {
            i_fm->field_0x792 = 1;
            i_fm->field_0x770 = 0;
        }
    } else if (dAlbwBoss_fyrusHollowPhase()) {
        i_fm->field_0x792 = 0;
        if (i_fm->field_0x770 == 0) {
            i_fm->field_0x770 = 1;
        }
    }
}

bool dAlbwBoss_fyrusShouldChipAblazeDamage() {
    return dAlbwBoss_fyrusAblazePhase() && !s_fyrusAblazeVulnOpen;
}

void dAlbwBoss_fyrusApplyChipDamage(e_fm_class* i_fm, int i_hpBefore) {
    if (i_fm == NULL || !dAlbwBoss_fyrusShouldChipAblazeDamage()) {
        return;
    }
    const int dealt = i_hpBefore - i_fm->health;
    if (dealt <= 0) {
        return;
    }
    int chip = dealt * 1 / 100;
    if (chip < 1) {
        chip = 1;
    }
    i_fm->health = static_cast<s16>(std::max(0, i_hpBefore - chip));
    s_fyrusAttackCredits = 0;
}

bool dAlbwBoss_fyrusGolemWindowIsLive() {
    return dAlbwBossRefinement_isEnabled() && s_fyrusGolemPhase == 1;
}

bool dAlbwBoss_fyrusGolemKidsLoose() {
    return dAlbwBossRefinement_isEnabled() && s_fyrusGolemPhase == 2 &&
           s_fyrusGolemId != fpcM_ERROR_PROCESS_ID_e;
}

bool dAlbwBoss_fyrusIsOurGolem(fpc_ProcID i_id) {
    return i_id != fpcM_ERROR_PROCESS_ID_e && i_id == s_fyrusGolemId;
}

void dAlbwBoss_fyrusClearGolemActor() {
    s_fyrusGolemId = fpcM_ERROR_PROCESS_ID_e;
}

void dAlbwBoss_fyrusOnGolemKidsCleared() {
    if (!dAlbwBossRefinement_isEnabled() || s_fyrusGolemPhase != 2) {
        return;
    }
    s_fyrusGolemPhase = 3;
    s_fyrusResumeFightPending = 1;

    // Reopen core weak spot (770 was forced 0 for the whole golem window — §9).
    fopAc_ac_c* fm = fopAcM_SearchByName(fpcNm_E_FM_e);
    if (fm != NULL && fopAcM_IsActor(fm) && fm->health > 0) {
        ((e_fm_class*)fm)->field_0x770 = 1;
    }
}

bool dAlbwBoss_fyrusTakeResumeFightPending() {
    if (s_fyrusResumeFightPending == 0) {
        return false;
    }
    s_fyrusResumeFightPending = 0;
    return true;
}

bool dAlbwBoss_fyrusTryGolemLookPos(cXyz** o_pos) {
    if (o_pos == NULL || !dAlbwBoss_fyrusGolemWindowIsLive()) {
        return false;
    }
    if (s_fyrusGolemId == fpcM_ERROR_PROCESS_ID_e) {
        return false;
    }
    fopAc_ac_c* golem = fopAcM_SearchByID(s_fyrusGolemId);
    if (golem == NULL || !fopAcM_IsActor(golem)) {
        return false;
    }
    *o_pos = &golem->eyePos;
    return true;
}

void dAlbwBoss_fyrusTryFloorDown(fopAc_ac_c* i_fm) {
    if (!dAlbwBossRefinement_isEnabled() || i_fm == NULL || !fopAcM_IsActor(i_fm)) {
        return;
    }
    if (fopAcM_GetName(i_fm) != fpcNm_E_FM_e || i_fm->health > 0) {
        return;
    }

    e_fm_class* fm = (e_fm_class*)i_fm;
    if (fm->mAction == kAlbwFmActionEnd || fm->mAction == kAlbwFmActionDown ||
        fm->mAction == kAlbwFmActionADown || fm->mAction == kAlbwFmActionStart)
    {
        return;
    }

    fm->mAction = kAlbwFmActionDown;
    fm->mMode = 0;
    fm->mDownCnt = 3;
}

bool dAlbwBoss_fyrusStayHollow() {
    return dAlbwBossRefinement_isEnabled() && s_fyrusGolemPhase != 0;
}

s16 dAlbwBoss_fyrusCreateHp() {
    return dAlbwBossRefinement_isEnabled() ? kAlbwFmRefinementHp : kAlbwFmVanillaHp;
}

s16 dAlbwBoss_fyrusKidCreateHp() {
    return kAlbwFmKidHp;
}

void dAlbwBoss_fyrusUpdateGolemWindow(fopAc_ac_c* i_fm) {
    if (!dAlbwBossRefinement_isEnabled() || i_fm == NULL || !fopAcM_IsActor(i_fm)) {
        return;
    }
    if (fopAcM_GetName(i_fm) != fpcNm_E_FM_e) {
        return;
    }

    const e_fm_class* fm = (const e_fm_class*)i_fm;
    if (fm->mAction == kAlbwFmActionStart) {
        return;
    }

    if (s_fyrusGolemPhase == 0 && fm->mAction != kAlbwFmActionEnd &&
        fyrusHpAtMostPct(i_fm, kAlbwFmGolemEnterPct))
    {
        s_fyrusGolemPhase = 1;
        const int roomNo = fopAcM_GetRoomNo(i_fm);
        s_fyrusGolemId = fopAcM_create(fpcNm_B_GO_e, 0, &i_fm->current.pos, roomNo,
                                       &i_fm->shape_angle, NULL, -1);
    }

    if (s_fyrusGolemPhase == 1 &&
        (fm->mAction == kAlbwFmActionEnd || i_fm->health <= 0 ||
         fyrusHpAtMostPct(i_fm, kAlbwFmGolemExitPct)))
    {
        if (fm->mAction == kAlbwFmActionEnd) {
            fyrusDespawnGolem();
        }
        s_fyrusGolemPhase = 2;
    }

    if (s_fyrusGolemPhase == 2 && fm->mAction == kAlbwFmActionEnd) {
        fyrusDespawnGolem();
    }
}

bool dAlbwBoss_armogohmaQueryHealthBar(dAlbwBoss_ArmogohmaBarState* o_state) {
    if (o_state == NULL) {
        return false;
    }

    o_state->visible = false;
    o_state->phase = 0;
    o_state->fillRatio = 0.0f;
    o_state->current = 0;
    o_state->max = 0;

    fopAc_ac_c* target = NULL;
    if (!dAlbwBoss_armogohmaResolveBarTarget(&target) || target == NULL) {
        return true;
    }

    o_state->phase = (fopAcM_GetName(target) == fpcNm_E_GM_e) ? 2 : 1;

    f32 fill;
    if (o_state->phase == 1) {
        // Phase 1 — giant body. Top half of the bar (1.0 .. 0.5).
        dAlbwBoss_armogohmaFillDisplayHp(target, &o_state->current, &o_state->max);

        f32 progress;
        if (dAlbwBossRefinement_isEnabled() && o_state->max > 0) {
            // Refinement fight: the pool actually drains (rod snaps + bow chips).
            progress = static_cast<f32>(o_state->current) / static_cast<f32>(o_state->max);
        } else {
            // Vanilla fight: B_GM.health is a constant 500 and never decrements,
            // so drive progress off the Dominion Rod statue-drop counter instead.
            int hits = static_cast<const b_gm_class*>(target)->mHitCount;
            hits = std::max(0, std::min(hits, kAlbwArmogohmaPhase1RodMax));
            progress = 1.0f - static_cast<f32>(hits) / static_cast<f32>(kAlbwArmogohmaPhase1RodMax);
        }
        progress = std::max(0.0f, std::min(progress, 1.0f));
        fill = kAlbwArmogohmaPhase1FillMin + (1.0f - kAlbwArmogohmaPhase1FillMin) * progress;
    } else {
        // Phase 2 — floor eye (TYPE_GOMA). Bottom half of the bar (0.5 .. 0.0),
        // driven by the hit counter (3 .. 0), NOT the eye's 1/1 health.
        int hits = static_cast<daE_GM_c*>(target)->albwGetBossHitRemaining();
        hits = std::max(0, std::min(hits, kAlbwArmogohmaPhase2HitMax));
        o_state->current = static_cast<s16>(hits);
        o_state->max = static_cast<s16>(kAlbwArmogohmaPhase2HitMax);
        fill = kAlbwArmogohmaPhase2FillMax *
               (static_cast<f32>(hits) / static_cast<f32>(kAlbwArmogohmaPhase2HitMax));
    }

    o_state->fillRatio = fill;
    o_state->visible = fill > 0.0f;  // phase 2 hits==0 hides the bar cleanly
    return true;
}

void dAlbwBoss_armogohmaOnRodHit(fopAc_ac_c* i_boss, s8 i_hitCount) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }

    dAlbwBoss_armogohmaEnsureInitialized(i_boss);

    if (i_hitCount == 1) {
        armogohmaApplyHpChange(i_boss, armogohmaHpFromPercent(kAlbwArmogohmaPostStatueSnapPct[0]));
    } else if (i_hitCount == 2) {
        armogohmaApplyHpChange(i_boss, armogohmaHpFromPercent(kAlbwArmogohmaPostStatueSnapPct[1]));
    }
}

// ============================================
// NEW CODE — ALBW Port (phase-3 reveal drain)
// ============================================
bool dAlbwBoss_armogohmaPhase3Damage(fopAc_ac_c* i_boss, int i_rawPower) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return false;
    }

    dAlbwBoss_armogohmaEnsureInitialized(i_boss);

    // Single tunable defense divisor, then floor every hit to a meaningful chip so
    // ALL damage sources drain the pool -- not just the sword. Weak ranged hits
    // (arrows/clawshot) otherwise divided down to ~1 HP and only nudged the hit
    // counter; now every eye hit removes at least kAlbwArmogohmaBowChipPct% of max,
    // while stronger hits still scale above the floor.
    int dmg = i_rawPower / kAlbwArmogohmaPhase3DefenseDiv;
    const int chipFloor = static_cast<int>(s_armogohmaMaxHp) * kAlbwArmogohmaBowChipPct / 100;
    if (dmg < chipFloor) {
        dmg = chipFloor;
    }
    if (dmg < 1) {
        dmg = 1;
    }

    const s16 newHp = static_cast<s16>(std::max(1, static_cast<int>(i_boss->health) - dmg));
    armogohmaApplyHpChange(i_boss, newHp);

    // Handoff to the E_GM eye once the pool reaches the sliver threshold.
    const s16 handoffFloor = armogohmaHpFromPercent(kAlbwArmogohmaPhase3HandoffPct);
    return i_boss->health <= handoffFloor;
}
// ============================================

bool dAlbwBoss_armogohmaTryBeginEggPhase(b_gm_class* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return false;
    }

    dAlbwBoss_armogohmaEnsureInitialized((fopAc_ac_c*)i_boss);

    for (int i = 0; i < kAlbwArmogohmaEggGateCount; i++) {
        if (s_armogohmaEggQueued[i] && !s_armogohmaEggUsed[i]) {
            s_armogohmaEggUsed[i] = true;
            s_armogohmaEggQueued[i] = false;
            i_boss->field_0x1ad5 = 1;
            return true;
        }
    }

    return false;
}

void dAlbwBoss_onArmogohmaVictory() {
    if (dAlbwBossRefinement_isEnabled()) {
        dAlbwBoss_armogohmaResetFightState();
        clearWarpBootstrapSession();
    }

    s_armogohmaBarSuppressed = true;
}

void dAlbwBoss_onStageLoad() {
    // Zant's per-phase bar tracker is HUD-only (bossHealthBars), independent of Boss
    // Refinement, so reset it before the Refinement early-out below.
    dAlbwBoss_zantResetFightState();
    dAlbwBoss_fyrusResetFightState();
    dAlbwBoss_morpheelResetFightState();

    if (!dAlbwBossRefinement_isEnabled()) {
        return;
    }

    const AlbwBossArenaId stageArena = dAlbwBoss_stageNameToArenaId(dComIfGp_getStartStageName());

    dAlbwBoss_diababaResetFightState();

    if (s_warpBootstrapArena != ALBW_BOSS_ARENA_INVALID && stageArena != s_warpBootstrapArena) {
        clearWarpBootstrapSession();
        return;
    }

    if (s_warpBootstrapArena == ALBW_BOSS_ARENA_TEMPLE_OF_TIME &&
        stageArena == ALBW_BOSS_ARENA_TEMPLE_OF_TIME)
    {
        applyArmogohmaStageFixes();
    }
}

void dAlbwBoss_requestWarpBootstrap(const char* i_stageName) {
    if (!dAlbwBossRefinement_isEnabled()) {
        return;
    }

    const AlbwBossArenaId arena = dAlbwBoss_stageNameToArenaId(i_stageName);
    if (arena == ALBW_BOSS_ARENA_INVALID) {
        return;
    }

    s_warpBootstrapArena = arena;
    s_armogohmaWarpBootstrap = false;
    dComIfGs_offStageBossEnemy();

    if (arena == ALBW_BOSS_ARENA_TEMPLE_OF_TIME) {
        applyArmogohmaStageFixes();
    }
}

void dAlbwBoss_applyPendingStageBootstrap() {
    dAlbwBoss_onStageLoad();
}

static void applyArmogohmaActorBootstrap(b_gm_class* i_gm) {
    s_armogohmaWarpBootstrap = true;

    i_gm->field_0x566 = 1;
    i_gm->mDemoMode = 0;
    i_gm->mAction = 1;
    i_gm->mMode = 4;
    i_gm->field_0x1ad5 = 0;
    i_gm->field_0x1cfc = 2;
    i_gm->field_0x6f5 = 0;
    i_gm->mTimers[2] = 250;

    Z2GetAudioMgr()->subBgmStop();
    Z2GetAudioMgr()->bgmStart(Z2BGM_GOMA_BTL01, 0, 0);

    resetArmogohmaCstatueActors();
}

bool dAlbwBoss_tryApplyActorBootstrap(s16 i_procName, fopAc_ac_c* i_actor) {
    if (!dAlbwBossRefinement_isEnabled() || s_warpBootstrapArena == ALBW_BOSS_ARENA_INVALID ||
        i_actor == NULL)
    {
        return false;
    }

    if (i_procName == fpcNm_B_GM_e && s_warpBootstrapArena == ALBW_BOSS_ARENA_TEMPLE_OF_TIME) {
        applyArmogohmaActorBootstrap((b_gm_class*)i_actor);
        return true;
    }

    return false;
}

// ============================================
// NEW CODE — ALBW Port (Morpheel bubbled eye-mass)
// ============================================

static AlbwMorpheelRefPhase s_morpheelPhase = ALBW_MORPHEEL_REF_OFF;
static fpc_ProcID s_morpheelBossId = fpcM_ERROR_PROCESS_ID_e;
static s16 s_morpheelOrbitAngle = 0;
static bool s_morpheelBombsSpawned = false;
static bool s_morpheelFightLive = false;
static bool s_morpheelBombsSeenAlive = false;
// Sequential grab pass (one tentacle at a time).
enum {
    MORPHEEL_GRAB_IDLE = 0,
    MORPHEEL_GRAB_STRIKE,  // waiting for current slot to Consume / finish ATTACK
    MORPHEEL_GRAB_GAP,     // inter-tentacle delay
    MORPHEEL_GRAB_HOLD,    // successful catch — pass ends after release
};
static s16 s_morpheelGrabPass = MORPHEEL_GRAB_IDLE;
static int s_morpheelGrabStart = 0;
static int s_morpheelGrabIndex = 0;  // 0..7 attempts completed in this pass
static s16 s_morpheelGrabGapTimer = 0;
static bool s_morpheelGrabStrikeClaimed = false;

static int morpheelGrabCurrentSlot() {
    return (s_morpheelGrabStart + s_morpheelGrabIndex) % kAlbwMorpheelTentacleCount;
}

static void morpheelGrabPassReset() {
    s_morpheelGrabPass = MORPHEEL_GRAB_IDLE;
    s_morpheelGrabStart = 0;
    s_morpheelGrabIndex = 0;
    s_morpheelGrabGapTimer = 0;
    s_morpheelGrabStrikeClaimed = false;
}

// Chu Worm bubble BMD (E_SM / BMDE_SM) — session-cached, DN-10 donor port.
static request_of_phase_process_class s_morpheelChuPhase;
static J3DModel* s_morpheelChuBubble = NULL;
static s8 s_morpheelChuLoad = 0;  // 0=idle 1=loading 2=ready 3=fail

static fopAc_ac_c* morpheelBossActor() {
    if (s_morpheelBossId == fpcM_ERROR_PROCESS_ID_e) {
        return NULL;
    }
    fopAc_ac_c* actor = fopAcM_SearchByID(s_morpheelBossId);
    if (actor == NULL || !fopAcM_IsActor(actor) || fopAcM_GetName(actor) != fpcNm_B_OB_e) {
        return NULL;
    }
    return actor;
}

void dAlbwBoss_morpheelResetFightState() {
    s_morpheelPhase = ALBW_MORPHEEL_REF_OFF;
    s_morpheelBossId = fpcM_ERROR_PROCESS_ID_e;
    s_morpheelOrbitAngle = 0;
    s_morpheelBombsSpawned = false;
    s_morpheelFightLive = false;
    s_morpheelBombsSeenAlive = false;
    morpheelGrabPassReset();
}

void dAlbwBoss_morpheelEnsureInit(fopAc_ac_c* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }
    if (fopAcM_GetName(i_boss) != fpcNm_B_OB_e) {
        return;
    }
    s_morpheelBossId = fopAcM_GetID(i_boss);
    // Armed but not live until Midna/intro settle — avoids cutscene fights + false CLAW.
    s_morpheelPhase = ALBW_MORPHEEL_REF_RING;
    s_morpheelOrbitAngle = 0;
    s_morpheelBombsSpawned = false;
    s_morpheelFightLive = false;
    s_morpheelBombsSeenAlive = false;
    morpheelGrabPassReset();
    if (s_morpheelChuLoad == 0) {
        s_morpheelChuLoad = 1;
    }
}

void dAlbwBoss_morpheelTickBubbleLoad() {
    if (!dAlbwBossRefinement_isEnabled() || s_morpheelChuLoad != 1) {
        return;
    }
    const int rv = dComIfG_resLoad(&s_morpheelChuPhase, "E_SM");
    if (rv != cPhs_COMPLEATE_e) {
        return;
    }
    if (s_morpheelChuBubble == NULL) {
        // Donor: daE_SM_c::CreateHeap BMDE_SM (0x21), flags match Chu.
        J3DModelData* modelData = (J3DModelData*)dComIfG_getObjectRes("E_SM", 0x21);
        if (modelData != NULL) {
            s_morpheelChuBubble = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
        }
    }
    s_morpheelChuLoad = (s_morpheelChuBubble != NULL) ? 2 : 3;
}

f32 dAlbwBoss_morpheelBubbleRadius() {
    return kAlbwMorpheelBubbleRadius * kAlbwMorpheelBubbleScale;
}

bool dAlbwBoss_morpheelDrawChuBubble(fopAc_ac_c* i_boss) {
    if (i_boss == NULL || !dAlbwBoss_morpheelBubbleUp() || s_morpheelChuBubble == NULL) {
        return false;
    }
    // Uniform Chu shell centered on the eye mass (Y ofs fixes BMDE_SM high pivot).
    mDoMtx_stack_c::transS(i_boss->current.pos.x,
                           i_boss->current.pos.y + kAlbwMorpheelBubbleYOfs,
                           i_boss->current.pos.z);
    mDoMtx_stack_c::scaleM(kAlbwMorpheelBubbleScale, kAlbwMorpheelBubbleScale,
                           kAlbwMorpheelBubbleScale);
    s_morpheelChuBubble->setBaseTRMtx(mDoMtx_stack_c::get());
    g_env_light.setLightTevColorType_MAJI(s_morpheelChuBubble, &i_boss->tevStr);
    fopAcM_setEffectMtx(i_boss, s_morpheelChuBubble->getModelData());
    J3DMaterial* mat = s_morpheelChuBubble->getModelData()->getMaterialNodePointer(0);
    if (mat != NULL && mat->getTevKColor(1) != NULL) {
        mat->getTevKColor(1)->a = 167;  // field_0x694 == 1.0f on Chu
    }
    mDoExt_modelUpdateDL(s_morpheelChuBubble);
    return true;
}

void dAlbwBoss_morpheelRequestTentacleGrab() {
    if (!dAlbwBoss_morpheelFightIsLive() || s_morpheelGrabPass != MORPHEEL_GRAB_IDLE) {
        return;
    }
    s_morpheelGrabStart = (int)cM_rndF((f32)kAlbwMorpheelTentacleCount - 0.01f);
    s_morpheelGrabIndex = 0;
    s_morpheelGrabGapTimer = 0;
    s_morpheelGrabStrikeClaimed = false;
    s_morpheelGrabPass = MORPHEEL_GRAB_STRIKE;
}

bool dAlbwBoss_morpheelConsumeTentacleGrab(int i_slot) {
    if (s_morpheelGrabPass != MORPHEEL_GRAB_STRIKE || s_morpheelGrabStrikeClaimed) {
        return false;
    }
    if (i_slot != morpheelGrabCurrentSlot()) {
        return false;
    }
    s_morpheelGrabStrikeClaimed = true;
    return true;
}

bool dAlbwBoss_morpheelTentacleGrabBusy() {
    return s_morpheelGrabPass != MORPHEEL_GRAB_IDLE;
}

static void morpheelGrabAdvanceAfterMiss() {
    s_morpheelGrabIndex++;
    s_morpheelGrabStrikeClaimed = false;
    if (s_morpheelGrabIndex >= kAlbwMorpheelTentacleCount) {
        morpheelGrabPassReset();
        return;
    }
    s_morpheelGrabPass = MORPHEEL_GRAB_GAP;
    s_morpheelGrabGapTimer =
        (s16)(kAlbwMorpheelGrabGapMin +
              cM_rndF((f32)(kAlbwMorpheelGrabGapMax - kAlbwMorpheelGrabGapMin)));
}

void dAlbwBoss_morpheelNotifyTentacleStrikeMiss() {
    if (s_morpheelGrabPass == MORPHEEL_GRAB_HOLD || s_morpheelGrabPass == MORPHEEL_GRAB_IDLE) {
        return;
    }
    morpheelGrabAdvanceAfterMiss();
}

void dAlbwBoss_morpheelNotifyTentacleGrabCaught() {
    s_morpheelGrabPass = MORPHEEL_GRAB_HOLD;
}

void dAlbwBoss_morpheelNotifyTentacleGrabHoldDone() {
    morpheelGrabPassReset();
}

void dAlbwBoss_morpheelNotifyTentacleGrabDone() {
    if (s_morpheelGrabPass == MORPHEEL_GRAB_HOLD) {
        dAlbwBoss_morpheelNotifyTentacleGrabHoldDone();
    } else {
        dAlbwBoss_morpheelNotifyTentacleStrikeMiss();
    }
}

bool dAlbwBoss_morpheelIsActive() {
    return dAlbwBossRefinement_isEnabled() && s_morpheelPhase != ALBW_MORPHEEL_REF_OFF &&
           s_morpheelPhase != ALBW_MORPHEEL_REF_HANDOFF;
}

bool dAlbwBoss_morpheelFightIsLive() {
    return dAlbwBoss_morpheelIsActive() && s_morpheelFightLive;
}

void dAlbwBoss_morpheelMarkBombsSpawned() {
    s_morpheelBombsSpawned = true;
}

void dAlbwBoss_morpheelSetFightLive(bool i_live) {
    s_morpheelFightLive = i_live;
}

AlbwMorpheelRefPhase dAlbwBoss_morpheelGetPhase() {
    return s_morpheelPhase;
}

bool dAlbwBoss_morpheelBubbleUp() {
    return dAlbwBoss_morpheelFightIsLive() &&
           (s_morpheelPhase == ALBW_MORPHEEL_REF_RING || s_morpheelPhase == ALBW_MORPHEEL_REF_CLAW);
}

bool dAlbwBoss_morpheelEyeHookAllowed() {
    return dAlbwBoss_morpheelFightIsLive() &&
           (s_morpheelPhase == ALBW_MORPHEEL_REF_CLAW || s_morpheelPhase == ALBW_MORPHEEL_REF_EXPOSED);
}

bool dAlbwBoss_morpheelEyeDamageAllowed() {
    return dAlbwBoss_morpheelFightIsLive() && s_morpheelPhase == ALBW_MORPHEEL_REF_EXPOSED;
}

void dAlbwBoss_morpheelOnEyeHooked() {
    if (!dAlbwBoss_morpheelIsActive()) {
        return;
    }
    if (s_morpheelPhase == ALBW_MORPHEEL_REF_CLAW) {
        s_morpheelPhase = ALBW_MORPHEEL_REF_EXPOSED;
        fopAc_ac_c* boss = morpheelBossActor();
        if (boss != NULL) {
            csXyz rot(0, boss->shape_angle.y, 0);
            dComIfGp_particle_set(0x877E, &boss->current.pos, &boss->tevStr, &rot, NULL);
            dComIfGp_particle_set(0x877F, &boss->current.pos, &boss->tevStr, &rot, NULL);
            dComIfGp_particle_set(0x8780, &boss->current.pos, &boss->tevStr, &rot, NULL);
        }
    }
}

void dAlbwBoss_morpheelOnEyeDepleted() {
    if (s_morpheelPhase == ALBW_MORPHEEL_REF_OFF) {
        return;
    }
    s_morpheelPhase = ALBW_MORPHEEL_REF_HANDOFF;
}

struct MorpheelBombCountCtx {
    int count;
};

static void* s_morpheelBombCount_sub(void* i_actor, void* i_data) {
    MorpheelBombCountCtx* ctx = (MorpheelBombCountCtx*)i_data;
    if (!fopAcM_IsActor(i_actor) || fopAcM_GetName(i_actor) != fpcNm_E_OctBg_e) {
        return NULL;
    }
    fopAc_ac_c* fish = (fopAc_ac_c*)i_actor;
    if (dAlbwBoss_morpheelIsRingBombParam(fopAcM_GetParam(fish))) {
        ctx->count++;
    }
    return NULL;
}

void dAlbwBoss_morpheelTick(fopAc_ac_c* i_boss) {
    if (!dAlbwBossRefinement_isEnabled() || i_boss == NULL) {
        return;
    }
    dAlbwBoss_morpheelTickBubbleLoad();
    if (s_morpheelPhase == ALBW_MORPHEEL_REF_OFF || s_morpheelPhase == ALBW_MORPHEEL_REF_HANDOFF) {
        return;
    }
    if (!s_morpheelFightLive) {
        return;
    }

    s_morpheelOrbitAngle += 0x40;  // slow spin of surface cover

    if (s_morpheelGrabPass == MORPHEEL_GRAB_GAP) {
        if (s_morpheelGrabGapTimer > 0) {
            s_morpheelGrabGapTimer--;
        }
        if (s_morpheelGrabGapTimer <= 0) {
            s_morpheelGrabPass = MORPHEEL_GRAB_STRIKE;
            s_morpheelGrabStrikeClaimed = false;
        }
    }

    if (s_morpheelPhase == ALBW_MORPHEEL_REF_RING && s_morpheelBombsSpawned) {
        MorpheelBombCountCtx ctx = {0};
        fpcM_Search(s_morpheelBombCount_sub, &ctx);
        if (ctx.count > 0) {
            s_morpheelBombsSeenAlive = true;
        }
        if (s_morpheelBombsSeenAlive && ctx.count <= 0) {
            s_morpheelPhase = ALBW_MORPHEEL_REF_CLAW;
        }
    }
}

bool dAlbwBoss_morpheelTryGetEyePos(cXyz* o_pos) {
    if (o_pos == NULL || !dAlbwBoss_morpheelIsActive()) {
        return false;
    }
    fopAc_ac_c* boss = morpheelBossActor();
    if (boss == NULL) {
        return false;
    }
    *o_pos = boss->current.pos;
    return true;
}

bool dAlbwBoss_morpheelTryRootTentacle(int i_slot, cXyz* o_pos, s16* o_homeYaw) {
    if (o_pos == NULL || !dAlbwBoss_morpheelFightIsLive()) {
        return false;
    }
    cXyz eye;
    if (!dAlbwBoss_morpheelTryGetEyePos(&eye)) {
        return false;
    }
    const s16 fan = (s16)(i_slot * (0x10000 / 8));
    o_pos->x = eye.x + kAlbwMorpheelTentacleRootR * cM_ssin(fan);
    o_pos->y = eye.y + 40.0f;
    o_pos->z = eye.z + kAlbwMorpheelTentacleRootR * cM_scos(fan);
    if (o_homeYaw != NULL) {
        *o_homeYaw = fan;
    }
    return true;
}

bool dAlbwBoss_morpheelIsRingBombParam(u32 i_param) {
    return (i_param & 0xFFFFFF00u) == kAlbwMorpheelRingBombParam;
}

int dAlbwBoss_morpheelRingBombSlot(u32 i_param) {
    if (!dAlbwBoss_morpheelIsRingBombParam(i_param)) {
        return -1;
    }
    return (int)(i_param & 0xFFu);
}

// Fibonacci sphere direction for slot i of n — covers full bubble surface.
static void morpheelSurfaceDir(int i_slot, int i_count, f32* o_x, f32* o_y, f32* o_z) {
    const int n = (i_count < 1) ? 1 : i_count;
    f32 y = 0.0f;
    if (n == 1) {
        y = 0.0f;
    } else {
        y = 1.0f - (2.0f * (f32)i_slot) / (f32)(n - 1);
    }
    const f32 r = std::sqrt(std::max(0.0f, 1.0f - y * y));
    const f32 theta = (f32)i_slot * 2.3999632f;  // golden angle
    *o_x = std::cos(theta) * r;
    *o_y = y;
    *o_z = std::sin(theta) * r;
}

bool dAlbwBoss_morpheelSnapRingBomb(fopAc_ac_c* i_fish) {
    if (i_fish == NULL || !dAlbwBoss_morpheelFightIsLive()) {
        return false;
    }
    if (s_morpheelPhase != ALBW_MORPHEEL_REF_RING && s_morpheelPhase != ALBW_MORPHEEL_REF_CLAW) {
        return false;
    }
    const int slot = dAlbwBoss_morpheelRingBombSlot(fopAcM_GetParam(i_fish));
    if (slot < 0 || slot >= kAlbwMorpheelRingBombs) {
        return false;
    }
    cXyz eye;
    if (!dAlbwBoss_morpheelTryGetEyePos(&eye)) {
        return false;
    }
    f32 dx, dy, dz;
    morpheelSurfaceDir(slot, kAlbwMorpheelRingBombs, &dx, &dy, &dz);
    // Slow spin so the cover isn't frozen.
    const s16 spin = s_morpheelOrbitAngle;
    const f32 c = cM_scos(spin);
    const f32 s = cM_ssin(spin);
    const f32 rx = dx * c - dz * s;
    const f32 rz = dx * s + dz * c;
    const f32 rad = dAlbwBoss_morpheelBubbleRadius();
    const f32 cy = eye.y + kAlbwMorpheelBubbleYOfs;
    i_fish->current.pos.x = eye.x + rad * rx;
    i_fish->current.pos.y = cy + rad * dy;
    i_fish->current.pos.z = eye.z + rad * rz;
    i_fish->old.pos = i_fish->current.pos;
    i_fish->speedF = 0.0f;
    i_fish->speed.set(0.0f, 0.0f, 0.0f);
    i_fish->current.angle.y = cM_atan2s(rx, rz);
    i_fish->shape_angle.y = i_fish->current.angle.y;
    return true;
}

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif
