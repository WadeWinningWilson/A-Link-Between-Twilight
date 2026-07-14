/**
 * @file d_a_albw_midna_arm.cpp
 *
 * ============================================
 * NEW CODE — ALBW Port — "Midna's Grasp" (Wolf Art 2, D-pad Right)
 * Auto-attacking Midna arm: for ~15 s, repeatedly stretch the hair-hand out to
 * the current Z-lock target, strike it as a SWORD hit, and retract.
 *
 * Architecture (per docs/wolf-combat-layers-research.md seam map):
 *  - A real spawned actor (profile appended PC-only at fpcNm_ALBW_MIDNA_ARM_e),
 *    so it runs in PARALLEL with wolf Link — his procs and his mAtCyl are never
 *    touched, and he keeps fighting while the arm works.
 *  - The AT collider is OWNED BY THIS ACTOR (mStts.Init(..., this)), not ALINK:
 *    at_power_check then leaves mHitType generic (12), so these hits can never
 *    pass the HIT_TYPE_LINK_NORMAL_ATTACK guard that feeds the wolf charge
 *    counter — "special arts never build charges" holds structurally.
 *  - Sword damage: AT_TYPE_NORMAL_SWORD + AtAtp, the collider placed at the
 *    target for the strike window each cycle (boomerang collider idiom).
 *  - Target = dComIfGp_getAttention()->LockonTarget(0), re-read every frame so
 *    a dropped/killed lock can never dangle; one enemy at a time by design.
 *  - Visual: publishes the reach point to dAlbwMidnaArm_setReachPos each frame;
 *    daAlink_c::setNeckAngle re-applies it to FLG1_MIDNA_HAIR_ATN_POS /
 *    mMidnaHairAtnPos, which daMidna_c's hair chain chases — Midna's hair
 *    visibly stretches to the strike point and back.
 * ============================================
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#include "d/actor/d_a_albw_midna_arm.h"

#include "SSystem/SComponent/c_math.h"  // cM_ssin/cM_scos (READY rest point ahead of the wolf)
#include "d/actor/d_a_player.h"
#include "d/d_albw_wolf_stun.h"
#include "d/d_com_inf_game.h"

// ============================================
// Tuning
// ============================================
namespace {
// NOTE: actor execute runs on the FIXED 30 Hz sim tick (dusk/game_clock.h sim_pace = 1/30) —
// N frames = N/30 seconds.
constexpr int kArmLifeFrames    = 450;  // 15 s at the 30 Hz sim tick
constexpr int kArmStretchFrames = 12;   // jab OUT: 0.4 s — hair snaps toward the target
constexpr int kArmHitFrames     = 4;    // strike window: ~0.13 s (collider armed at the target)
constexpr int kArmRetractFrames = 12;   // jab RETURN: 0.4 s (guided glide back, rendition-1 style)
constexpr int kArmPauseFrames   = 6;    // rest between punches: 0.2 s in the READY hover
                                        // punch-to-punch = 34 f ≈ 1.13 s (was 44 f ≈ 1.47 s)
// READY stance: rest the hair aimed FORWARD of the wolf (like its pose when Z-targeting a boss —
// the Wchain/Beast-Ganon aim), not straight up (aiming the chain vertically flipped the hand
// backwards).  The rest point sits ahead of the perch along the wolf's facing, slightly raised.
constexpr f32 kArmIdleForward   = 150.0f;  // rest-aim distance ahead of the perch
constexpr f32 kArmIdleHoverY    = 20.0f;   // slight rise above the perch
constexpr int kArmAtp           = 4;    // sword attack power (Hurricane initCutTurnAt uses 4)
constexpr f32 kArmStrikeRadius  = 80.0f;   // strike cylinder radius at the target
constexpr f32 kArmStrikeHeight  = 120.0f;  // strike cylinder height at the target
constexpr f32 kArmReachYOffset  = 80.0f;   // reach origin above wolf Link's feet (Midna's perch)
// Accept targets only within the hair's actual extended reach (~3x scale x 5 segments x 28 units
// = ~420 from the hair base) so the hand always visually connects with what it hits.
constexpr f32 kArmMaxRange      = 400.0f;

bool s_armAlive = false;  // one-at-a-time gate

// Sword-typed AT source (mirrors daAlink's l_atCylSrc; radius/height overridden per strike).
// Tg/Co lines are empty — the arm itself can't be hit.
static dCcD_SrcCyl l_armAtCylSrc = {
    {
        {0, {{AT_TYPE_NORMAL_SWORD, 2, 0x1B}, {0, 0}, 0}},
        {dCcD_SE_SWORD, 3, 1, 0, {1}},
        {dCcD_SE_NONE, 0, 0, 0, {0}},
        {0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            80.0f,
            120.0f,
        },
    }
};
}  // namespace

bool daAlbwMidnaArm_c::isAlive() {
    return s_armAlive;
}

// ============================================
// Target: the current Z-lock enemy, validated fresh EVERY frame (never cached
// across frames, so a killed/dropped target can't dangle).
// ============================================
fopAc_ac_c* daAlbwMidnaArm_c::getLockTarget() {
    fopAc_ac_c* tgt = dComIfGp_getAttention()->LockonTarget(0);
    if (tgt == NULL) {
        return NULL;
    }
    if (fopAcM_GetGroup(tgt) != fopAc_ENEMY_e) {
        return NULL;
    }
    daPy_py_c* link = daPy_getPlayerActorClass();
    if (link != NULL && link->current.pos.abs(tgt->current.pos) > kArmMaxRange) {
        return NULL;
    }
    return tgt;
}

void daAlbwMidnaArm_c::enterState(State i_state) {
    mState = i_state;
    switch (i_state) {
    case STATE_STRETCH_e: mStateTimer = kArmStretchFrames; break;
    case STATE_HIT_e:     mStateTimer = kArmHitFrames;     break;
    case STATE_RETRACT_e: mStateTimer = kArmRetractFrames; break;
    case STATE_PAUSE_e:   mStateTimer = kArmPauseFrames;   break;
    default:              mStateTimer = 0;                 break;
    }
}

int daAlbwMidnaArm_c::create() {
    fopAcM_ct(this, daAlbwMidnaArm_c);

    // Collider owned by THIS actor — the load-bearing line: a non-ALINK owner
    // means generic mHitType, so the wolf charge counter can never see these hits.
    mStts.Init(60, 0xFF, this);
    mAtCyl.Set(l_armAtCylSrc);
    mAtCyl.SetStts(&mStts);
    mAtCyl.SetAtAtp(kArmAtp);
    mAtCyl.SetR(kArmStrikeRadius);
    mAtCyl.SetH(kArmStrikeHeight);

    mLifeFrames = kArmLifeFrames;
    enterState(STATE_IDLE_e);

    s_armAlive = true;
    return cPhs_COMPLEATE_e;
}

int daAlbwMidnaArm_c::Delete() {
    dAlbwMidnaArm_clearReachPos();
    s_armAlive = false;
    return 1;
}

int daAlbwMidnaArm_c::Execute() {
    daPy_py_c* link = daPy_getPlayerActorClass();

    // Expire / bail: lifetime over, Link gone, transformed back to human, or the
    // wolf-combat feature switched off.
    mLifeFrames--;
    if (mLifeFrames <= 0 || link == NULL || !daPy_py_c::checkNowWolf() ||
        !dAlbwWolfCombat_isEnabled())
    {
        dAlbwMidnaArm_clearReachPos();
        fopAcM_delete(this);
        return 1;
    }

    // Ride along with wolf Link (keeps room/cull bookkeeping sane).
    current.pos = link->current.pos;
    fopAcM_SetRoomNo(this, fopAcM_GetRoomNo(static_cast<fopAc_ac_c*>(link)));

    // The reach origin: Midna's perch on the wolf's back.
    mReachFrom = link->current.pos;
    mReachFrom.y += kArmReachYOffset;

    fopAc_ac_c* tgt = getLockTarget();

    // The READY rest point: ahead of Midna's perch along the wolf's facing, slightly raised — the
    // "hand is armed" idle tell, posed like the hair's boss-Z-target aim.
    cXyz hoverPos = mReachFrom;
    hoverPos.x += cM_ssin(link->shape_angle.y) * kArmIdleForward;
    hoverPos.z += cM_scos(link->shape_angle.y) * kArmIdleForward;
    hoverPos.y += kArmIdleHoverY;

    switch (mState) {
    case STATE_IDLE_e:
        // No target: hold the READY stance (normal hair scale, hovering above Midna).
        dAlbwMidnaArm_setReachPos(hoverPos, false);
        if (tgt != NULL) {
            enterState(STATE_STRETCH_e);
        }
        break;

    case STATE_STRETCH_e: {
        if (tgt == NULL) {
            enterState(STATE_RETRACT_e);  // lock dropped mid-stretch -> pull back
            break;
        }
        mReachTarget = tgt->eyePos;
        // Rendition-2 JAB out (user preference after A/B): ease-out — most of the distance in the
        // first frames, decelerating into the target.  Launches from the READY rest for continuity.
        const f32 lin = 1.0f - (f32)mStateTimer / (f32)kArmStretchFrames;
        const f32 t   = 1.0f - (1.0f - lin) * (1.0f - lin);
        cXyz reach    = hoverPos + (mReachTarget - hoverPos) * t;
        dAlbwMidnaArm_setReachPos(reach, true);
        if (--mStateTimer <= 0) {
            enterState(STATE_HIT_e);
        }
        break;
    }

    case STATE_HIT_e: {
        if (tgt == NULL) {
            enterState(STATE_RETRACT_e);
            break;
        }
        mReachTarget = tgt->eyePos;
        dAlbwMidnaArm_setReachPos(mReachTarget, true);
        // Strike: the sword-typed cylinder sits AT the target for the window.
        // One hit per punch: the window is shorter than sword i-frames, and enemy
        // invulnerability spaces out repeat punches naturally.
        mAtCyl.SetC(tgt->current.pos);
        dComIfG_Ccsp()->Set(&mAtCyl);
        if (--mStateTimer <= 0) {
            enterState(STATE_RETRACT_e);
        }
        break;
    }

    case STATE_RETRACT_e: {
        // Rendition-2 return (user preference after A/B): release the reach and let the hair's own
        // easing carry it home; the PAUSE state then re-establishes the READY rest.
        dAlbwMidnaArm_clearReachPos();
        if (--mStateTimer <= 0) {
            enterState(STATE_PAUSE_e);
        }
        break;
    }

    case STATE_PAUSE_e:
        // Brief rest between punches, holding the READY hover.
        dAlbwMidnaArm_setReachPos(hoverPos, false);
        if (--mStateTimer <= 0) {
            enterState(tgt != NULL ? STATE_STRETCH_e : STATE_IDLE_e);
        }
        break;
    }

    return 1;
}

int daAlbwMidnaArm_c::Draw() {
    return 1;  // no model — the visual is Midna's own hair (reach bridge) + hit marks
}

// ============================================
// Profile boilerplate (pattern: d_a_albw_shade_boss_wolf.cpp; helper-actor
// status/group per d_a_boomerang.cpp).
// ============================================
static int daAlbwMidnaArm_Create(void* a_this) {
    return static_cast<daAlbwMidnaArm_c*>(a_this)->create();
}

static int daAlbwMidnaArm_Delete(void* a_this) {
    return static_cast<daAlbwMidnaArm_c*>(a_this)->Delete();
}

static int daAlbwMidnaArm_Execute(void* a_this) {
    return static_cast<daAlbwMidnaArm_c*>(a_this)->Execute();
}

static int daAlbwMidnaArm_Draw(void* a_this) {
    return static_cast<daAlbwMidnaArm_c*>(a_this)->Draw();
}

static int daAlbwMidnaArm_IsDelete(void* a_this) {
    return 1;
}

static DUSK_CONST actor_method_class daAlbwMidnaArm_MethodTable = {
    (process_method_func)daAlbwMidnaArm_Create,
    (process_method_func)daAlbwMidnaArm_Delete,
    (process_method_func)daAlbwMidnaArm_Execute,
    (process_method_func)daAlbwMidnaArm_IsDelete,
    (process_method_func)daAlbwMidnaArm_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_ALBW_MIDNA_ARM = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 6,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_ALBW_MIDNA_ARM_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daAlbwMidnaArm_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_ALBW_MIDNA_ARM_e,
    /* Actor SubMtd */ &daAlbwMidnaArm_MethodTable,
    /* Status       */ fopAcStts_UNK_0x40000_e,
    /* Group        */ fopAc_UNK_GROUP_5_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
