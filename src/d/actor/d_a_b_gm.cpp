/**
 * @file d_a_b_gm.cpp
 * 
*/

#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_b_gm.h"
#include "d/actor/d_a_e_gm.h"
#include "d/actor/d_a_player.h"
#include "d/actor/d_a_obj_ystone.h"
#include "d/d_s_play.h"
#include "d/d_com_inf_game.h"
#include "c/c_damagereaction.h"
#include "SSystem/SComponent/c_math.h"
#include "f_op/f_op_camera_mng.h"
#include "f_op/f_op_msg_mng.h"
#include "Z2AudioLib/Z2Instances.h"
#if TARGET_PC
#include "d/d_albw_boss.h"
#include "d/d_albw_enemy_rupee.h"
#include "d/actor/d_a_alink.h"  // equipped-item check: hookshot vs double clawshot
#include "dusk/custom_assets.hpp"  // repack-free loose-BMD override for the GOMA model
#include "dusk/settings.h"         // collision-view toggles (isolated beam-capsule debug draw)
#include <cstdio>
#include <cstdlib>
#endif

#define ANM_EYE_TEST            6
#define ANM_GM_BEAM             7
#define ANM_GOMA_ATTACK_01      8
#define ANM_GOMA_ATTACK_A       9
#define ANM_GOMA_ATTACK_B       10
#define ANM_GOMA_ATTACK_C       11
#define ANM_GOMA_DAMAGE_01      12
#define ANM_GOMA_DAMAGE_02      13
#define ANM_GOMA_DAMAGE_WAIT    14
#define ANM_GOMA_DASH           15
#define ANM_GOMA_DEATH          16
#define ANM_GOMA_FALL_LOOP      17
#define ANM_GOMA_LANDING        18
#define ANM_GOMA_LANDING_DAMAGE 19
#define ANM_GOMA_LANDING_WAIT   20
#define ANM_GOMA_LAY_EGGS       21
#define ANM_GOMA_MOVE           22
#define ANM_GOMA_RETURN         23
#define ANM_GOMA_ROOF_DAMAGE    24
#define ANM_GOMA_SLOW_MOVE      25
#define ANM_GOMA_STEP_L         26
#define ANM_GOMA_STEP_R         27
#define ANM_GOMA_UP             28
#define ANM_GOMA_UP_02          29
#define ANM_GOMA_WAIT           30

// ============================================
// NEW CODE — ALBW Port (giant floor-pursuit TEST, reversible)
// Cut "tunnel-chase" recreation: a hookshot to the body kicks the giant into an
// upright floor chase toward Link (mirrors Diababa's arrow-triggered cut-move
// test). First pass uses the retail floor DASH anim + the shared speedF forward
// integrator in action(); the slower GOMA_MOVE/SLOW_MOVE crawl anims can be
// swapped in once their floor orientation is confirmed in a viewer. Set to 0 to
// fully disable the trigger.
// ============================================
#define D_ALBW_ARMO_PURSUIT_TEST 0  // TEST triggers removed for now — set to 1 to restore

// ============================================
// NEW CODE — ALBW Port
// Phase-3 reveal (Stage 1: model swap). After the 2nd Dominion Rod statue hit the
// giant's vanilla model is swapped to a custom "revealed" model (single eye slid
// to the mouth). Gated three ways: Boss Refinement ON + Custom Models toggle ON +
// the loose B_gm_37.bmd present (try_load returns NULL otherwise). If any is
// missing the vanilla 3-statue refinement fight is untouched. Kill switch below.
// State is file-scope (one Armogohma at a time) since the size-locked b_gm_class
// struct can't take new members.
// ============================================
#define D_ALBW_ARMO_REVEAL 1
// Temporary diagnostic: 1 = keep the vanilla model through phase 3 (no reveal swap)
// to prove the upside-down break is the reveal BMD's export orientation. Set 0 to
// restore the real reveal swap once the model is re-exported correctly.
#define D_ALBW_ARMO_P3_DIAG_NOSWAP 0
// Temporary: log every phase-3 core-sphere (eye) hit + the pool drain to
// albw_armo_p3_debug.txt to confirm #4 (HP not draining). No lines ⇒ the eye is
// never being hit (Tg mask / position); lines with power 0 ⇒ hit lands but no damage.
// Set 0 (and strip) once the drain is confirmed.
#define D_ALBW_ARMO_P3_HP_DIAG 1
// Temporary: log the death cutscene / E_GM spawn progression to albw_armo_death_debug.txt
// (demo_camera modes 30/31/32 + each E_GM createChild). Confirms whether the disappear
// cutscene reaches the spawn and where the eyes are placed. Set 0 (and strip) once E_GM
// spawns reliably.
#define D_ALBW_ARMO_DEATH_DIAG 1

#if TARGET_PC && D_ALBW_ARMO_REVEAL
static mDoExt_McaMorfSO* s_gmRevealMorf = NULL;  // built in useHeapInit, freed with the solid heap
static bool s_gmRevealActive = false;            // set once the swap fires (one-way)
static int s_gmRevealDrawLog = 0;                // DIAG: per-fight reveal-draw step log budget

// Phase-3 ground-chase state (single boss, so file-scope is safe). Sub-states live
// in mMode; a single hit counter (s_gmPhase3HitCount) drives both reactions:
//   >3 hits -> LASER interlude, 5 hits -> VULN stagger (then counter resets).
// The eye open/shut cycle + the stagger drive field_0x1ad6, which gates the core
// weak-point sphere -> "eye open" IS the vulnerable window. The laser keeps it 0
// (invulnerable). Frame counts assume a 60 fps logic tick; halve if 30 fps. Tune.
enum { P3_DASH = 0, P3_VULN = 1, P3_LASER = 2, P3_INTRO = 3, P3_LIEDOWN = 4 };
static bool s_gmPhase3Active = false;
static bool s_gmPhase3EyeOpen = false;
static int s_gmPhase3HitCount = 0;
static s16 s_gmLaserSweepCenter = 0;  // yaw the laser sweep is centered on (Link at ignite)
static const int kAlbwArmoP3EyeCycleFrames = 300;  // 5s shut / 5s open (dash)
static const s16 kAlbwArmoLaserSweepHalf   = 0x4000;  // +/-90 deg = a 180 deg sweep arc
static const int kAlbwArmoLaserTurns       = 6;       // number of alternating 180 deg sweeps
static const f32 kAlbwArmoLaserTurnSpeedup = 0.8f;    // each turn's arc takes 0.8x the last (20% faster)
static const int kAlbwArmoP3VulnFrames     = 480;  // 8s upright weak stagger
static const int kAlbwArmoP3LaserFrames    = 600;  // 10s laser move
static const int kAlbwArmoP3LaserFireAt    = 180;  // beam ignites at 3s (elapsed)
static const int kAlbwArmoP3VulnHitCount   = 5;    // hits -> stagger
static const int kAlbwArmoP3LaserHitCount  = 3;    // >this -> laser (same counter)
static void b_gm_beginPhase3(b_gm_class* i_this);
static void b_gm_phase3(b_gm_class* i_this);
static void b_gm_beginPhase3Handoff(b_gm_class* i_this);
static void b_gm_beginPhase3Death(b_gm_class* i_this);

#if D_ALBW_ARMO_DEATH_DIAG
// Open (truncate first time, append after) the death/E_GM-spawn diagnostic log. Caller
// writes one line and fcloses. Strip with D_ALBW_ARMO_DEATH_DIAG once E_GM is reliable.
static FILE* b_gm_deathDiagOpen() {
    static bool sReset = false;
    char path[512]; path[0] = '\0';
    const char* user = getenv("USERPROFILE");
    if (user && user[0] != '\0')
        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_armo_death_debug.txt", user);
    else
        strncpy(path, "albw_armo_death_debug.txt", sizeof(path) - 1);
    FILE* fp = fopen(path, sReset ? "a" : "w");
    if (fp == NULL) fp = fopen("albw_armo_death_debug.txt", sReset ? "a" : "w");
    if (fp != NULL) sReset = true;
    return fp;
}
#endif
static void b_gm_phase3EnterVuln(b_gm_class* i_this);
static void b_gm_phase3EnterLaser(b_gm_class* i_this);
#endif
// ============================================

// Pursuit flavors, split by trigger weapon (all TEST triggers — hookshot /
// boomerang / slingshot are temporary and will be reverted):
//   DASH    — fast, retail GOMA_DASH anim        → hookshot
//   CRAWL   — slow, cut GOMA_MOVE crawl anim      → double clawshot / boomerang
//   EYETEST — play the orphaned ANM_EYE_TEST in place (observe it) → slingshot
enum ArmoPursuitMode {
    ARMO_PURSUIT_DASH = 0,
    ARMO_PURSUIT_CRAWL = 1,
    ARMO_PURSUIT_EYETEST = 2,
};

enum daB_GM_ACTION {
    ACTION_WAIT,
    ACTION_MOVE,

    ACTION_BEAM = 5,
    ACTION_KOGOMA,

    ACTION_DAMAGE = 10,
    ACTION_DROP,
    ACTION_PURSUIT_TEST = 12,
    ACTION_PHASE3 = 13,  // reveal ground-chase (Boss Refinement, after 2nd statue hit)
};

#if TARGET_PC && D_ALBW_ARMO_PURSUIT_TEST
// Single Armogohma at a time, so file-scope mode state is safe.
static u8 s_armoPursuitMode = ARMO_PURSUIT_DASH;
static void b_gm_beginPursuitTest(b_gm_class* i_this, u8 i_mode);
static void b_gm_pursuit_test(b_gm_class* i_this);
#endif
// ============================================
// NEW CODE ENDS HERE
// ============================================

class daB_GM_HIO_c {
public:
    daB_GM_HIO_c();
    virtual ~daB_GM_HIO_c() {}

    /* 0x04 */ s8 field_0x4;
    /* 0x08 */ f32 model_size;
    /* 0x0C */ f32 check_size;
    /* 0x10 */ f32 dash_speed;
    /* 0x14 */ f32 dash_anm_speed;
    /* 0x18 */ f32 move_speed;
    /* 0x1C */ f32 move_anm_speed;
    /* 0x20 */ f32 wait_anm_speed;
    /* 0x24 */ f32 range;
    /* 0x28 */ s16 smoke_prim_R;
    /* 0x2A */ s16 smoke_prim_G;
    /* 0x2C */ s16 smoke_prim_B;
    /* 0x2E */ s16 smoke_env_R;
    /* 0x30 */ s16 smoke_env_G;
    /* 0x32 */ s16 smoke_env_B;
    /* 0x34 */ s16 smoke_alpha;
    /* 0x36 */ s16 field_0x36;
    /* 0x38 */ f32 smoke_blend;
    /* 0x3C */ f32 bend_degree_1;
    /* 0x40 */ f32 bend_degree_2;
    /* 0x44 */ f32 bend_degree_3;
    /* 0x48 */ u8 foot_pos_check;
    /* 0x49 */ u8 eye_check;
};

daB_GM_HIO_c::daB_GM_HIO_c() {
    field_0x4 = 0xFF;
    model_size = 1.0f;
    check_size = 1.5f;

    dash_speed = 35.0f;
    dash_anm_speed = 1.0f;
    move_speed = 15.0f;
    move_anm_speed = 1.0f;
    wait_anm_speed = 1.0f;
    range = 1300.0f;

    smoke_prim_R = 205;
    smoke_prim_G = 200;
    smoke_prim_B = 180;
    smoke_env_R = 103;
    smoke_env_G = 100;
    smoke_env_B = 90;
    smoke_alpha = 110;
    smoke_blend = 0.3;

    bend_degree_1 = 1.0f;
    bend_degree_2 = 1.0f;
    bend_degree_3 = -1.0f;

    foot_pos_check = false;
    eye_check = false;
}

static void anm_init(b_gm_class* i_this, int i_anmID, f32 i_morf, u8 i_attr, f32 i_speed) {
    i_this->mpModelMorf->setAnm((J3DAnmTransform*)dComIfG_getObjectRes("B_gm", i_anmID), i_attr, i_morf, i_speed, 0.0f, -1.0f);
    i_this->mAnmID = i_anmID;
}

#if TARGET_PC && D_ALBW_ARMO_REVEAL
// ============================================
// NEW CODE — ALBW Port (phase-3 reveal eye/lid mouth anchor)
// The shared vanilla Armogohma BCKs carry TRS tracks for the eye (0x15) and the
// eyelids (0x16/0x17) that pin those joints to the DORSAL eye socket. The reveal
// BMD bakes them at the MOUTH, but the animation overwrites the model's rest pose
// every frame -- so without this the revealed eye/lids snap back into the (now
// empty) dorsal hole and leave the mouth gaping. For the reveal model only, we
// rebuild each joint's world matrix as
//     body joint (0x6) world  x  baked mouth-local  (translations from B_gm_37.bmd
//     JNT1; all three share rotZ = -78.5 deg)
// so the eye/lids ride the animated body while ignoring the BCK's dorsal
// translation. The blink / eye-tracking rotations are then layered on top exactly
// as before (their aim is relative to this matrix, so it still tracks Link).
// ============================================
static const s16 kAlbwArmoRevealMouthRotZ = -14294;  // -78.5 deg, as baked in the reveal BMD
// How far (mouth-local +X units) the eyeball retracts into the socket at a full blink.
// Keyed to the lid angle so the eye hides during the closed window and slides back to
// the tracked mouth position as the lid opens. Tunable (debug collider/eye view).
static const f32 kAlbwArmoEyeRetract = 200.0f;
// Small lift of the beam source so it emanates from the eyeball CENTER, not just below
// the eye joint. (A big lift put it at the old dorsal socket -- rejected.) Tunable.
static const f32 kAlbwArmoBeamOriginRaise = 50.0f;
// Phase-3 laser: the beam fires straight forward from the eye onto the floor and the
// body's in-place rotation sweeps it. This is the forward distance of the floor aim
// point -- larger = flatter beam that reaches further out (wider sweep radius). Tunable.
static const f32 kAlbwArmoBeamForward = 1200.0f;
// Phase-3 laser damage is a capsule (pill) spanning the whole beam line -- Dark-Souls
// style, the entire beam is the hurt zone. This is its radius; start it matched to the
// beam's visual thickness, then grow it and the beam model together. Tunable.
// Currently +5% over the ~20u visual so the pill peeks out of the glow and is catchable
// in the AT (red) collision view -- a diagnostic to confirm it isn't just buried.
static const f32 kAlbwArmoBeamRadius = 21.0f;

static void b_gm_revealAnchorMouthJoint(J3DModel* model, f32 tx, f32 ty, f32 tz) {
    Mtx local;
    cMtx_ZrotS(local, kAlbwArmoRevealMouthRotZ);         // local = RotZ(-78.5)
    local[0][3] = tx;                                    // + baked mouth translation ->
    local[1][3] = ty;                                    //   local = Trans * RotZ (joint-local)
    local[2][3] = tz;
    cMtx_concat(model->getAnmMtx(6), local, *calc_mtx);  // *calc_mtx = body-world * mouth-local
}
#endif

static int nodeCallBack(J3DJoint* i_joint, int param_1) {
    if (param_1 == 0) {
        int jnt_no = i_joint->getJntNo();
        J3DModel* model = j3dSys.getModel();
        
        b_gm_class* a_this = (b_gm_class*)model->getUserArea();
        if (a_this != NULL) {
            MTXCopy(model->getAnmMtx(jnt_no), *calc_mtx);

            if (jnt_no >= 0x19 && jnt_no <= 0x1B) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[0].field_0x8[jnt_no - 0x19]);
            } else if (jnt_no >= 0x1D && jnt_no <= 0x1F) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[1].field_0x8[jnt_no - 0x1D]);
            } else if (jnt_no >= 0x21 && jnt_no <= 0x23) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[2].field_0x8[jnt_no - 0x21]);
            } else if (jnt_no >= 0x25 && jnt_no <= 0x27) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[3].field_0x8[jnt_no - 0x25]);
            } else if (jnt_no >= 0x29 && jnt_no <= 0x2B) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[4].field_0x8[jnt_no - 0x29]);
            } else if (jnt_no >= 0x2D && jnt_no <= 0x2F) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[5].field_0x8[jnt_no - 0x2D]);
            } else if (jnt_no >= 0x31 && jnt_no <= 0x33) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[6].field_0x8[jnt_no - 0x31]);
            } else if (jnt_no >= 0x35 && jnt_no <= 0x37) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[7].field_0x8[jnt_no - 0x35]);
            } else if (jnt_no >= 0x39 && jnt_no <= 0x3B) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[8].field_0x8[jnt_no - 0x39]);
            } else if (jnt_no >= 0x3E && jnt_no <= 0x40) {
                cMtx_ZrotM(*calc_mtx, a_this->mFoot[8].field_0x8[jnt_no - 0x3E]);
            } else if (jnt_no == 0x16) {
#if TARGET_PC && D_ALBW_ARMO_REVEAL
                if (s_gmRevealActive && s_gmRevealMorf != NULL && model == s_gmRevealMorf->getModel()) {
                    b_gm_revealAnchorMouthJoint(model, 438.2799f, -48.1853f, -7.1048f);  // eyelidT
                }
#endif
                cMtx_ZrotM(*calc_mtx, a_this->field_0x1ad8);
            } else if (jnt_no == 0x17) {
#if TARGET_PC && D_ALBW_ARMO_REVEAL
                if (s_gmRevealActive && s_gmRevealMorf != NULL && model == s_gmRevealMorf->getModel()) {
                    b_gm_revealAnchorMouthJoint(model, 437.6509f, -74.1303f, -5.1240f);  // eyelidU
                }
#endif
                cMtx_ZrotM(*calc_mtx, -a_this->field_0x1ad8);
            } else if (jnt_no == 0x15) {
#if TARGET_PC && D_ALBW_ARMO_REVEAL
                if (s_gmRevealActive && s_gmRevealMorf != NULL && model == s_gmRevealMorf->getModel()) {
                    // Retract the eyeball into the socket while the lid is shut, keyed to
                    // the lid angle (field_0x1ad8: +6000 open .. -8000 shut). The blink
                    // hides the tracking pop-out; the eye slides back to the tracked mouth
                    // position as the lid opens. Only the eyeball moves (lids stay put).
                    // The laser forces the lid open, so the eye stays out through the beam.
                    f32 retract = (6000.0f - (f32)a_this->field_0x1ad8) * (1.0f / 14000.0f);
                    if (retract < 0.0f) retract = 0.0f;
                    else if (retract > 1.0f) retract = 1.0f;
                    b_gm_revealAnchorMouthJoint(model, 455.5993f - kAlbwArmoEyeRetract * retract,
                                                -62.0347f, -7.1048f);  // eye
                }
#endif
                cMtx_YrotM(*calc_mtx, a_this->field_0x1ada);
                cMtx_ZrotM(*calc_mtx, a_this->field_0x1adc);
            }

            model->setAnmMtx(jnt_no, *calc_mtx);
            MTXCopy(*calc_mtx, J3DSys::mCurrentMtx);
        }
    }

    return 1;
}

static u8 hio_set;

static daB_GM_HIO_c l_HIO;

#if TARGET_PC && D_ALBW_ARMO_REVEAL
// ============================================
// NEW CODE — ALBW Port
// One-time swap to the revealed model. Both morfs are already resident (built in
// useHeapInit), so this is a pointer flip + re-bind — no mid-fight allocation, and
// leak-safe (daB_GM_Delete frees the solid heap, not mpModelMorf directly).
// ============================================
static void b_gm_activateReveal(b_gm_class* i_this) {
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
        fprintf(fp, "[reveal] activateReveal entry: morf=%p active=%d model=%p\n",
                (void*)s_gmRevealMorf, (int)s_gmRevealActive,
                (void*)(s_gmRevealMorf ? s_gmRevealMorf->getModel() : NULL)); fclose(fp); } }
#endif
    if (s_gmRevealMorf == NULL || s_gmRevealActive) {
        return;
    }
    const int anm = i_this->mAnmID;
    i_this->mpModelMorf = s_gmRevealMorf;
    // Actor matrix was bound to the vanilla model's baseTRMtx in Create; re-point it
    // at the reveal model so it tracks the actor's position.
    fopAcM_SetMtx((fopAc_ac_c*)i_this, s_gmRevealMorf->getModel()->getBaseTRMtx());
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
        fprintf(fp, "[reveal] post SetMtx: modelData=%p mats=%d\n",
                (void*)s_gmRevealMorf->getModel()->getModelData(),
                (int)s_gmRevealMorf->getModel()->getModelData()->getMaterialNum()); fclose(fp); } }
#endif
    // Re-init the pupil-dilation BTK against the reveal model's material list.
    i_this->mpZoomBtk->init(s_gmRevealMorf->getModel()->getModelData(),
                            (J3DAnmTextureSRTKey*)dComIfG_getObjectRes("B_gm", 0x28), 1, 0, 1.0f, 0, -1);
    // Carry the current animation onto the fresh morf so the swap is seamless.
    anm_init(i_this, anm, 0.0f, J3DFrameCtrl::EMode_LOOP, 1.0f);
    s_gmRevealActive = true;
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[reveal] activateReveal DONE\n"); fclose(fp); } }
#endif
}
// ============================================
#endif

static int daB_GM_Draw(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    #if !DEBUG
    g_env_light.settingTevStruct(0, &a_this->current.pos, &a_this->tevStr);
    #endif

    for (int i = 0; i < 4; i++) {
        g_env_light.setLightTevColorType_MAJI(i_this->mpSpotLightModel[i], &a_this->tevStr);
        i_this->mpSpotLightBtk->entry(i_this->mpSpotLightModel[i]->getModelData());
        mDoExt_modelUpdateDL(i_this->mpSpotLightModel[i]);
    }

    if (i_this->mIsDisappear) {
        return 1;
    }

    J3DModel* model = i_this->mpModelMorf->getModel();
    #if DEBUG
    g_env_light.settingTevStruct(0, &a_this->current.pos, &a_this->tevStr);
    #endif
#if D_ALBW_ARMO_DEATH_DIAG
    const bool revealDbg = (s_gmRevealActive && s_gmRevealDrawLog < 2);
    if (revealDbg) { s_gmRevealDrawLog++; FILE* fp = b_gm_deathDiagOpen(); if (fp) {
        fprintf(fp, "[reveal] draw: model=%p data=%p mats=%d -> pre setLight\n",
                (void*)model, (void*)model->getModelData(), (int)model->getModelData()->getMaterialNum()); fclose(fp); } }
#endif
    g_env_light.setLightTevColorType_MAJI(model, &a_this->tevStr);

    i_this->mpZoomBtk->entry(model->getModelData());
#if D_ALBW_ARMO_DEATH_DIAG
    if (revealDbg) { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[reveal] draw: post zoomBtk entry -> pre material loop\n"); fclose(fp); } }
#endif

    if (fabsf(i_this->mBodyColorIntensity) > 1.0f) {
        J3DModelData* modelData = model->getModelData();
        for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
            J3DMaterial* material = modelData->getMaterialNodePointer(i);

            material->getTevColor(0)->r = i_this->mBodyColorIntensity;
            material->getTevColor(0)->g = i_this->mBodyColorIntensity;
            material->getTevColor(0)->b = i_this->mBodyColorIntensity;
        }
    }
#if D_ALBW_ARMO_DEATH_DIAG
    if (revealDbg) { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[reveal] draw: post material loop -> pre entryDL\n"); fclose(fp); } }
#endif

    i_this->mpModelMorf->entryDL();
#if D_ALBW_ARMO_DEATH_DIAG
    if (revealDbg) { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[reveal] draw: post entryDL (model drawn OK)\n"); fclose(fp); } }
#endif

    cXyz shadow_pos;
    shadow_pos.set(a_this->current.pos.x, BREG_F(18) + (a_this->current.pos.y + l_HIO.model_size * 400.0f), a_this->current.pos.z);
    i_this->mShadowID = dComIfGd_setShadow(i_this->mShadowID, 1, model, &shadow_pos, (l_HIO.model_size * 3000.0f) + BREG_F(19), 0.0f, a_this->current.pos.y, i_this->mAcch.GetGroundH(), i_this->mAcch.m_gnd, &a_this->tevStr, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex());

    if (i_this->field_0x6c0 > 0.1f) {
        J3DModel* model = i_this->mpBeamModelMorf->getModel();
        g_env_light.setLightTevColorType_MAJI(model, &a_this->tevStr);
        i_this->mpBeamBtk->entry(model->getModelData());
        i_this->mpBeamModelMorf->entryDL();
    }

    return 1;
}

static void* s_ko_del(void* i_actor, void* i_data) {
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e && fopAcM_GetParam(i_actor) == 0) {
        ((daE_GM_c*)i_actor)->InstantKill();
    }

    return NULL;
}

static void damage_check(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    fopAc_ac_c* player = dComIfGp_getPlayer(0);

    i_this->mCcStts.Move();

    if (i_this->mInvincibilityTimer == 0) {
        if ((i_this->mAction != ACTION_DROP || (i_this->mMode != 2 && i_this->mMode != 3)) && i_this->mCoreSph.ChkTgHit()) {
            i_this->mAtInfo.mpCollider = i_this->mCoreSph.GetTgHitObj();
            cc_at_check(i_this, &i_this->mAtInfo);

#if TARGET_PC && D_ALBW_ARMO_REVEAL
            // ============================================
            // NEW CODE — ALBW Port (phase-3 reveal drain)
            // The core sphere only registers while the eye is open (field_0x1ad6
            // gates it), so a hit here IS a valid open-eye hit. Drain the pool
            // through the defended helper instead of the ceiling-fall flinch
            // (ACTION_DAMAGE plays a ceiling anim, wrong for the floor chase).
            // mAtInfo.mAttackPower already has sword tier / FA / bow / bomb folded in.
            // ============================================
            if (s_gmPhase3Active) {
                i_this->mInvincibilityTimer = 8;
                // Drain the pool (defended) whenever the eye is open. The laser keeps
                // field_0x1ad6 == 0, so the core sphere never registers there -> no
                // drain and no counting during the laser ("eye invulnerable").
#if D_ALBW_ARMO_P3_HP_DIAG
                const s16 p3HpBefore = ((fopAc_ac_c*)i_this)->health;
#endif
                bool p3handoff = dAlbwBoss_armogohmaPhase3Damage(
                    (fopAc_ac_c*)i_this, i_this->mAtInfo.mAttackPower);
#if D_ALBW_ARMO_P3_HP_DIAG
                {
                    // ============================================
                    // NEW CODE — ALBW Port (DIAGNOSTIC #4 — strip with D_ALBW_ARMO_P3_HP_DIAG)
                    // ============================================
                    static bool sP3Reset = false;
                    char path[512]; path[0] = '\0';
                    const char* user = getenv("USERPROFILE");
                    if (user && user[0] != '\0')
                        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_armo_p3_debug.txt", user);
                    else
                        strncpy(path, "albw_armo_p3_debug.txt", sizeof(path) - 1);
                    FILE* fp = fopen(path, sP3Reset ? "a" : "w");
                    if (fp == NULL) fp = fopen("albw_armo_p3_debug.txt", sP3Reset ? "a" : "w");
                    if (fp != NULL) {
                        sP3Reset = true;
                        fprintf(fp, "[p3hit] mMode=%d rawPow=%d hp %d->%d hits=%d handoff=%d\n",
                                (int)i_this->mMode, (int)i_this->mAtInfo.mAttackPower,
                                (int)p3HpBefore, (int)((fopAc_ac_c*)i_this)->health,
                                (int)s_gmPhase3HitCount, (int)p3handoff);
                        fclose(fp);
                    }
                }
#endif
                if (p3handoff) {
                    b_gm_beginPhase3Handoff(i_this);  // <=5% -> disappear cutscene -> E_GM
                } else if (i_this->mMode == P3_DASH) {
                    // One counter, high threshold checked first: 5 -> stagger, else
                    // >3 -> laser. Entering either closes/gates the eye so a burst
                    // can't multi-trigger. Stagger hits don't count (guarded by mMode).
                    s_gmPhase3HitCount++;
                    if (s_gmPhase3HitCount >= kAlbwArmoP3VulnHitCount) {
                        b_gm_phase3EnterVuln(i_this);
                    } else if (s_gmPhase3HitCount > kAlbwArmoP3LaserHitCount) {
                        b_gm_phase3EnterLaser(i_this);
                    }
                }
                cXyz p3_pos;
                MtxP p3_mtx = mDoMtx_stack_c::get();
                MTXCopy(i_this->mpModelMorf->getModel()->getAnmMtx(0x15), p3_mtx);
                mDoMtx_stack_c::multVecZero(&p3_pos);
                for (int p3 = 0; p3 < 3; p3++) {
                    static u16 p3name[] = {0x8B8C, 0x8B8D, 0x8B8E};
                    dComIfGp_particle_set(p3name[p3], &p3_pos, NULL, NULL);
                }
                return;
            }
            // ============================================
#endif

#if TARGET_PC
            bool chipOnlyOnCeiling = false;
            if (dAlbwBossRefinement_isEnabled() && i_this->mAtInfo.mpCollider != NULL &&
                i_this->mAtInfo.mpCollider->ChkAtType(AT_TYPE_ARROW))
            {
                dAlbwBoss_armogohmaOnBowCoreHit(a_this);
                if (dAlbwBoss_armogohmaIsOnCeiling(a_this) &&
                    !dAlbwBoss_armogohmaTakeCeilingDropPending())
                {
                    chipOnlyOnCeiling = true;
                }
            }

            if (chipOnlyOnCeiling) {
                i_this->mInvincibilityTimer = 5;
                return;
            }
#endif

            i_this->mInvincibilityTimer = 10;
            i_this->mInvincibilityTimer = 5;
            i_this->mAction = ACTION_DAMAGE;
            i_this->mMode = 0;
            i_this->field_0x6c0 = 0.0f;

            cXyz eff_pos;
            MtxP now_mtx = mDoMtx_stack_c::get();
            MTXCopy(i_this->mpModelMorf->getModel()->getAnmMtx(0x15), now_mtx);
            mDoMtx_stack_c::multVecZero(&eff_pos);

            for (int i = 0; i < 3; i++) {
                static u16 name[] = {0x8B8C, 0x8B8D, 0x8B8E};
                dComIfGp_particle_set(name[i], &eff_pos, NULL, NULL);
            }
            return;
        }

        for (int i = 0; i < 8; i++) {
            if (i < 2 && i_this->mBodySph[i].ChkTgHit()) {
                i_this->mInvincibilityTimer = 6;
#if TARGET_PC && D_ALBW_ARMO_PURSUIT_TEST
                // ============================================
                // NEW CODE — ALBW Port (giant floor-pursuit TEST trigger)
                // Weapon selects the flavor: arrow → orphaned ANM_EYE_TEST
                // (TEMP observe hook — slingshot didn't register on the body
                // sphere; REVERT this, arrow is a real damage method); boomerang
                // → slow crawl; hookshot family → fast dash, unless it's the
                // double clawshot (both fire AT_TYPE_HOOKSHOT, so split by the
                // equipped item — the clawshot upgrade replaces the hookshot, so
                // they never coexist).
                // ============================================
                if (i_this->mBodySph[i].GetTgHitObj()->ChkAtType(AT_TYPE_ARROW)) {
                    b_gm_beginPursuitTest(i_this, ARMO_PURSUIT_EYETEST);  // TEMP: revert
                } else if (i_this->mBodySph[i].GetTgHitObj()->ChkAtType(AT_TYPE_BOOMERANG)) {
                    b_gm_beginPursuitTest(i_this, ARMO_PURSUIT_CRAWL);
                } else if (i_this->mBodySph[i].GetTgHitObj()->ChkAtType(AT_TYPE_HOOKSHOT)) {
                    daAlink_c* link = (daAlink_c*)daPy_getPlayerActorClass();
                    u8 mode = (link != NULL && link->getReadyItem() == dItemNo_W_HOOKSHOT_e)
                                  ? ARMO_PURSUIT_CRAWL
                                  : ARMO_PURSUIT_DASH;
                    b_gm_beginPursuitTest(i_this, mode);
                }
                // ============================================
                // NEW CODE ENDS HERE
                // ============================================
#endif
                if (i_this->mAction == ACTION_DROP && (i_this->mMode == 2 || i_this->mMode == 3)) {
                    if (i_this->mBodySph[i].GetTgHitObj()->ChkAtType(AT_TYPE_CSTATUE_BOSS_SWING)) {
                        i_this->mHitCount++;
                        dComIfGs_onOneZoneSwitch(4, -1);
                        Z2GetAudioMgr()->changeBgmStatus(2);

                        int rodMax = 3;
#if TARGET_PC && D_ALBW_ARMO_REVEAL
                        // Reveal fight: 2 statue hits instead of 3, and the giant's
                        // model swaps to the revealed one on the final hit. Only when
                        // the reveal model actually loaded (Refinement + Custom Models
                        // toggle + B_gm_37.bmd all present); otherwise the vanilla
                        // 3-statue path below is untouched.
                        if (s_gmRevealMorf != NULL) {
                            rodMax = 2;
                        }
#endif
                        if (i_this->mHitCount >= rodMax) {
#if TARGET_PC && D_ALBW_ARMO_REVEAL
                            if (s_gmRevealMorf != NULL) {
                                // Reveal fight: land the 35% checkpoint (final statue
                                // snap), then enter phase 3. The get-up intro plays
                                // RETURN and swaps to the revealed model only once it
                                // stands upright (b_gm_phase3 P3_INTRO), so the revealed
                                // model never shows in the smashed/upside-down pose. The
                                // E_GM handoff is relocated to phase3's <=5% drain.
                                dAlbwBoss_armogohmaOnRodHit(a_this, i_this->mHitCount);
                                b_gm_beginPhase3(i_this);
                            } else
#endif
                            {
                                i_this->mMode = 20;
                                i_this->mDemoMode = 30;
                                fpcM_Search(s_ko_del, i_this);
                                Z2GetAudioMgr()->bgmStop(0x1E, 0);
                            }
                        } else {
#if TARGET_PC
                            dAlbwBoss_armogohmaOnRodHit(a_this, i_this->mHitCount);
#endif
                            i_this->mMode = 10;
                            i_this->field_0x1ad5 = 2;
                            i_this->mDemoMode = 20;
                        }

                        dComIfGp_getVibration().StartShock(VIBMODE_S_POWER8, 0x4F, cXyz(0.0f, 1.0f, 0.0f));

                        for (int j = 0; j < 4; j++) {
                            static u16 name[] = {0x8AF6, 0x8AF7, 0x8AF8, 0x8AF9};
                            dComIfGp_particle_set(name[j], &a_this->current.pos, NULL, NULL);
                        }
                        return;
                    }
                    dComIfGs_onOneZoneSwitch(3, -1);
                }
                return;
            } else if (i_this->mFootSph[i].ChkTgHit()) {
                i_this->mInvincibilityTimer = 6;
                def_se_set(&i_this->mCreatureSound, i_this->mFootSph[i].GetTgHitObj(), 0x28, NULL);
                dComIfGs_onOneZoneSwitch(3, -1);
                return;
            }
        }
    }
}

static BOOL bg_check(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    dBgS_LinChk line_chk;
    cM3dGPla plane;
    cXyz start;
    cXyz end;
    cXyz sp14;
    cXyz unused;

    cMtx_YrotS(*calc_mtx, a_this->current.angle.y);
    cMtx_XrotM(*calc_mtx, a_this->current.angle.x);
    sp14.x = 0.0f;
    sp14.y = 180.0f;
    sp14.z = 0.0f;
    MtxPosition(&sp14, &start);

    start += a_this->current.pos;
    sp14.x = 0.0f;
    sp14.y = 0.0f;
    sp14.z = 600.0f;
    MtxPosition(&sp14, &end);
    end += start;

    line_chk.Set(&start, &end, i_this);
    if (dComIfG_Bgsp().LineCross(&line_chk)) {
        if (a_this->current.angle.x < 0x1000 && a_this->current.angle.x > -0x1000) {
            dComIfG_Bgsp().GetTriPla(line_chk, &plane);
            sp14.x = plane.GetNP()->x;
            sp14.z = plane.GetNP()->z;
            i_this->field_0x5ce = cM_atan2s(sp14.x, sp14.z) + 0x8000;
        }

        i_this->field_0x73c = line_chk.GetCross();
        return TRUE;
    }

    return FALSE;
}

static int ko_ct;

static void* s_ko_sub(void* i_actor, void* i_data) {
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e) {
        ko_ct++;
    }

    return NULL;
}

static void* s_ko2_move(void* i_actor, void* i_data) {
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e && fopAcM_GetParam(i_actor) == 3) {
        ((daE_GM_c*)i_actor)->MoveStart(0);
        return i_actor;
    }

    return NULL;
}

static void* s_ko2_get(void* i_actor, void* i_data) {
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e && fopAcM_GetParam(i_actor) == 3) {
        return i_actor;
    }

    return NULL;
}

static void* s_ko_move(void* i_actor, void* i_data) {
    if (fopAcM_IsActor(i_actor) && fopAcM_GetName(i_actor) == fpcNm_E_GM_e && fopAcM_GetParam(i_actor) == 1) {
        ((daE_GM_c*)i_actor)->MoveStart(cM_rndF(15.0f));
    }

    return NULL;
}

static void b_gm_wait(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    switch (i_this->mMode) {
    case 0:
        anm_init(i_this, ANM_GOMA_WAIT, 20.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.wait_anm_speed);
        i_this->mTimers[0] = 100.0f + cM_rndF(100.0f);
        i_this->mMode = 1;
        /* fallthrough */
    case 1:
        if (i_this->mTimers[0] == 0) {
            i_this->mAction = ACTION_MOVE;
            i_this->mMode = 0;
        }
        break;
    }

    cLib_addCalc0(&a_this->speedF, 1.0f, 4.0f);
    i_this->mAcch.CrrPos(dComIfG_Bgsp());
}

static cXyz target_pos[] = {
    cXyz(-1350.0f, 0.0f, -1350.0f),
    cXyz(-1350.0f, 0.0f, 1350.0f),
    cXyz(1350.0f, 0.0f, -1350.0f),
    cXyz(1350.0f, 0.0f, 1350.0f),
};

static void b_gm_move(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    cXyz sp1C;
    f32 move_speed = l_HIO.dash_speed;

    switch (i_this->mMode) {
    case 0:
        anm_init(i_this, ANM_GOMA_DASH, 10.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.dash_anm_speed);
        i_this->mMode = 1;
        i_this->unk_0x5cc = 0;
        /* fallthrough */
    case 1:
        if (a_this->current.angle.x < 0x1000 && a_this->current.angle.x > -0x1000) {
            for (int i = 0; i < 4; i++) {
                sp1C.x = target_pos[i].x * 1.7f - a_this->current.pos.x;
                sp1C.z = target_pos[i].z * 1.7f - a_this->current.pos.z;

                s16 sp8 = a_this->current.angle.y - cM_atan2s(sp1C.x, sp1C.z);
                if (sp8 < 0x1800 && sp8 > -0x1800) {
                    if (sp8 > 0) {
                        a_this->current.angle.y += 0x200;
                    } else {
                        a_this->current.angle.y -= 0x200;
                    }
                    break;
                }
            }
        }

        if (bg_check(i_this)) {
            i_this->mMode = 2;

            if (i_this->field_0x5d8 == 0.0f) {
                i_this->field_0x5d8 = -(f32)0x4000;
            } else if (i_this->field_0x5d8 == -(f32)0x4000) {
                i_this->field_0x5d8 = -(f32)0x8000;
            } else if (i_this->field_0x5d8 == -(f32)0x8000) {
                i_this->field_0x5d8 = -(f32)0xC000;
            } else if (i_this->field_0x5d8 == -(f32)0xC000) {
                i_this->field_0x5d8 = -(f32)0x10000;
            }
        }
        break;
    case 2:
        cLib_addCalc2(&i_this->field_0x5d4, i_this->field_0x5d8, 1.0f, 1024.0f);

        if (i_this->field_0x5d4 == -(f32)0x10000) {
            i_this->field_0x5d4 = 0.0f;
        }

        if (i_this->field_0x5d4 == 0.0f || i_this->field_0x5d4 == -(f32)0x4000 || i_this->field_0x5d4 == -(f32)0x8000 || i_this->field_0x5d4 == -(f32)0xC000) {
            if (i_this->field_0x5d4 == -(f32)0x8000) {
                i_this->mMode = 3;
                i_this->mTimers[1] = 60;
            } else {
                i_this->mMode = 1;
            }
        }

        cLib_addCalcAngleS2(&a_this->current.angle.y, i_this->field_0x5ce, 4, 0x400);
        a_this->current.angle.x = i_this->field_0x5d4;
        break;
    case 3:
        if (i_this->mTimers[1] == 0) {
            sp1C = i_this->mTargetMovePos - a_this->current.pos;
            cLib_addCalcAngleS2(&a_this->current.angle.y, cM_atan2s(sp1C.x, sp1C.z) + 0x8000, 4, 0x300);

            if (sp1C.abs() < 200.0f) {
                if (i_this->mTimers[2] != 0) {
                    i_this->mTimers[0] = 0;
                    i_this->mMode = 4;
#if TARGET_PC
                // ============================================
                // MODIFIED CODE — ALBW Port
                // Vanilla lays eggs when arrows run low (arrowNum <= 3) so the
                // player can refill by killing babies. In ALBW arrows are never
                // the resource (the meter is), so arrowNum sits <= 3 forever and
                // eggs spawn endlessly. Circumvent that trigger even when Boss
                // Refinement is OFF — keep only the post-rod batch (field_0x1ad5
                // == 2). (Refinement path keeps its HP-gated egg system.)
                // ============================================
                } else if ((dAlbwBossRefinement_isEnabled() &&
                            dAlbwBoss_armogohmaTryBeginEggPhase(i_this)) ||
                           (!dAlbwBossRefinement_isEnabled() &&
                            i_this->field_0x1ad5 == 2))
                {
                    i_this->mAction = ACTION_KOGOMA;
                    i_this->mMode = 0;
                    if (!dAlbwBossRefinement_isEnabled()) {
                        i_this->field_0x1ad5 = 1;
                    }
#else
                } else if (i_this->field_0x1ad5 == 2 || dComIfGs_getArrowNum() <= 3) {
                    i_this->mAction = ACTION_KOGOMA;
                    i_this->mMode = 0;
                    i_this->field_0x1ad5 = 1;
#endif
                } else {
                    i_this->mTimers[0] = 0;
                    i_this->mMode = 4;

                    if (cM_rndF(1.0f) < 0.5f) {
                        i_this->mAction = ACTION_BEAM;
                        i_this->mMode = 0;
                    }
                }
            }
        }
        break;
    case 4:
        move_speed = 0.0f;
        if (i_this->mTimers[0] == 0) {
            if (i_this->mAnmID != ANM_GOMA_DASH) {
                anm_init(i_this, ANM_GOMA_DASH, 10.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.dash_anm_speed);
            }

            i_this->mMode = 3;
            i_this->field_0x5b6 += (int)(cM_rndF(2.99f) + 1.0f);
            i_this->field_0x5b6 &= 3;

            i_this->mTargetMovePos.x = target_pos[i_this->field_0x5b6].x;
            i_this->mTargetMovePos.y = a_this->current.pos.y;
            i_this->mTargetMovePos.z = target_pos[i_this->field_0x5b6].z;
        }
        break;
    case 10:
        move_speed = 0.0f;
        a_this->current.angle.y = i_this->mAngleToPlayer + 0x8000;
        break;
    }

    cLib_addCalc2(&a_this->speedF, move_speed, 1.0f, l_HIO.dash_speed * 0.5f + 1.0f);

    if (a_this->current.angle.x < 0x1000 && a_this->current.angle.x > -0x1000) {
        cLib_addCalc2(&a_this->current.pos.y, i_this->field_0x73c.y, 1.0f, 20.0f);
        i_this->mAcch.CrrPos(dComIfG_Bgsp());
    } else if (a_this->current.angle.x > 0x7000 || a_this->current.angle.x < -0x7000) {
        cLib_addCalc2(&a_this->current.pos.y, i_this->field_0x73c.y, 1.0f, 20.0f);
    } else {
        cLib_addCalc2(&a_this->current.pos.x, i_this->field_0x73c.x, 1.0f, 20.0f);
        cLib_addCalc2(&a_this->current.pos.z, i_this->field_0x73c.z, 1.0f, 20.0f);
    }
}

static void b_gm_beam(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    cXyz unused;
    f32 move_speed = 0.0f;

    switch (i_this->mMode) {
    case 0:
        i_this->mMode = 2;
        i_this->field_0x6f4 = 0;
        anm_init(i_this, ANM_GOMA_ATTACK_A, 10.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
        break;
    case 2:
        i_this->field_0x1ad6 = 1;
        if (i_this->mpModelMorf->isStop()) {
            anm_init(i_this, ANM_GOMA_ATTACK_B, 2.0f, J3DFrameCtrl::EMode_LOOP, 1.0f);
            i_this->mMode = 3;
            i_this->mTimers[0] = 200;
        }
        break;
    case 3:
        i_this->field_0x1ad6 = 1;
        cLib_addCalc2(&i_this->field_0x6c0, 1.0f, 1.0f, 0.1f);
        if (i_this->mTimers[0] == 0) {
            i_this->mMode = 4;
            anm_init(i_this, ANM_GOMA_ATTACK_C, 5.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
        }
        break;
    case 4:
        cLib_addCalc0(&i_this->field_0x6c0, 1.0f, 0.1f);
        if (i_this->mpModelMorf->isStop()) {
            i_this->field_0x6c0 = 0.0f;
            i_this->mAction = ACTION_MOVE;
            i_this->mTimers[0] = 0;
            i_this->mMode = 4;
        }
        break;
    }

    cLib_addCalc2(&i_this->speedF, move_speed, 1.0f, 1.0f + (0.5f * l_HIO.dash_speed));
}

static void b_gm_kogoma(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    cXyz kogoma_pos;
    cXyz sp54;
    cXyz sp48;
    f32 move_speed = l_HIO.dash_speed;

    J3DModel* model = i_this->mpModelMorf->getModel();

    MtxP now_mtx = mDoMtx_stack_c::get();
    MTXCopy(model->getAnmMtx(4), now_mtx);
    mDoMtx_stack_c::multVecZero(&sp54);

    now_mtx = mDoMtx_stack_c::get();
    MTXCopy(model->getAnmMtx(5), now_mtx);
    mDoMtx_stack_c::multVecZero(&sp48);

    kogoma_pos = sp54 + ((sp48 - sp54) * 0.5f);

    switch (i_this->mMode) {
    case 0:
        anm_init(i_this, ANM_GOMA_DASH, 10.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.dash_anm_speed);
        i_this->mTargetMovePos.x = 0.0f;
        i_this->mTargetMovePos.z = 0.0f;
        i_this->mMode = 1;
        /* fallthrough */
    case 1:
        sp54 = i_this->mTargetMovePos - a_this->current.pos;
        cLib_addCalcAngleS2(&a_this->current.angle.y, cM_atan2s(sp54.x, sp54.z) + 0x8000, 4, 0x300);

        if (i_this->field_0x1cf9 == 0 && i_this->mDemoMode == 0 && sp54.abs() < 400.0f) {
            i_this->mDemoMode = 10;
            i_this->field_0x1cf9 = 1;
        }

        if (sp54.abs() < 200.0f) {
            anm_init(i_this, ANM_GOMA_LAY_EGGS, 20.0f, J3DFrameCtrl::EMode_LOOP, 1.0f);
            i_this->mTimers[0] = 200;
            i_this->mMode = 2;
        }
        break;
    case 2:
        move_speed = 0.0f;
        if (i_this->mTimers[0] < 150 && i_this->mTimers[0] > 30 && (i_this->mTimers[0] & 3) == 0) {
            ko_ct = 0;
            fpcM_Search(s_ko_sub, a_this);
            if (ko_ct < 100) {
                csXyz angle(0, a_this->shape_angle.y, 0);
                fopAcM_createChild(fpcNm_E_GM_e, fopAcM_GetID(a_this), 0, &kogoma_pos, fopAcM_GetRoomNo(a_this), &angle, NULL, -1, 0);
            }
        }

        if (i_this->mTimers[0] == 0) {
            i_this->mAction = ACTION_MOVE;
            i_this->mTimers[0] = 0;
            i_this->mMode = 4;
            i_this->mTimers[2] = 200.0f + cM_rndF(100.0f);
        }
        break;
    }

    cLib_addCalc2(&a_this->speedF, move_speed, 1.0f, 1.0f + (0.5f * l_HIO.dash_speed));
}

static void b_gm_damage(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    int anm_frame = i_this->mpModelMorf->getFrame();

    switch (i_this->mMode) {
    case 0:
        i_this->mMode = 1;
        anm_init(i_this, ANM_GOMA_ROOF_DAMAGE, 3.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
        break;
    case 1:
        if (anm_frame >= 50) {
            if (anm_frame == 50) {
                a_this->speed.y = -100.0f;
            }

            a_this->current.pos.y += a_this->speed.y;
            a_this->speed.y -= 10.0f;
        }

        if (i_this->mpModelMorf->isStop()) {
            i_this->mAction = ACTION_DROP;
            i_this->mMode = 0;
            anm_init(i_this, ANM_GOMA_FALL_LOOP, 0.0f, J3DFrameCtrl::EMode_LOOP, 1.0f);
        }
        break;
    }

    i_this->mInvincibilityTimer = 20;
    cLib_addCalc0(&a_this->speedF, 1.0f, 5.0f);
}

static void b_gm_drop(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    cXyz sp20;
    cXyz sp14;

    switch (i_this->mMode) {
    case 0:
        i_this->mMode = 1;
        a_this->speedF = 0.0f;
        /* fallthrough */
    case 1:
        a_this->current.pos.y += a_this->speed.y;
        a_this->speed.y -= 10.0f;

        if (a_this->current.pos.y <= 1200.0f) {
            a_this->current.pos.y = 0.0f;
            a_this->speed.y = 0.0f;

            dComIfGp_getVibration().StartShock(VIBMODE_S_POWER8, 0x4F, cXyz(0.0f, 1.0f, 0.0f));

            i_this->mMode = 2;
            anm_init(i_this, ANM_GOMA_LANDING, 0.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
            i_this->mTimers[0] = 300;

            fopAcM_effSmokeSet1(&i_this->field_0x1c84, &i_this->field_0x1c88, &a_this->current.pos, NULL, 15.0f, &a_this->tevStr, 1);
            Z2GetAudioMgr()->changeBgmStatus(1);
            dComIfGs_onOneZoneSwitch(1, -1);
        }
        break;
    case 2:
        if (i_this->mpModelMorf->isStop()) {
            anm_init(i_this, ANM_GOMA_LANDING_WAIT, 0.0f, J3DFrameCtrl::EMode_LOOP, 1.0f);
        }

        a_this->current.pos.y = 0.0f;
        if (i_this->mTimers[0] == 0) {
            i_this->mMode = 3;
            anm_init(i_this, ANM_GOMA_RETURN, 5.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
            Z2GetAudioMgr()->changeBgmStatus(2);
        }
        break;
    case 3:
        if (i_this->mpModelMorf->isStop()) {
            anm_init(i_this, ANM_GOMA_WAIT, 0.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.wait_anm_speed);
            a_this->current.angle.x = 0;
            a_this->shape_angle.x = 0;
            i_this->mTimers[0] = 5;
            i_this->mMode = 4;

            cMtx_YrotS(*calc_mtx, a_this->current.angle.y);
            sp20.x = 0.0f;
            sp20.y = 0.0f;
            sp20.z = -400.0f;
            MtxPosition(&sp20, &sp14);
            a_this->current.pos += sp14;
        }
        break;
    case 4:
        i_this->mInvincibilityTimer = 5;
        if (i_this->mTimers[0] == 0) {
            i_this->field_0x5d8 = 0.0f;
            i_this->field_0x5d4 = 0.0f;
            i_this->field_0x73c.y = 0.0f;
            i_this->mAction = ACTION_MOVE;
            i_this->mMode = 0;
        }
        break;
    case 10:
        anm_init(i_this, ANM_GOMA_LANDING_DAMAGE, 2.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
        i_this->mMode = 11;
    case 11:
        if (i_this->mpModelMorf->isStop()) {
            i_this->mMode = 3;
            anm_init(i_this, ANM_GOMA_RETURN, 5.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
            i_this->mInvincibilityTimer = 200;
        }
        break;
    case 20:
        anm_init(i_this, ANM_GOMA_DEATH, 2.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
        i_this->mMode = 21;
    case 21:
        if (i_this->mpModelMorf->getFrame() >= 160.0f) {
            cLib_addCalc2(&i_this->mBodyColorIntensity, -30.0f, 1.0f, 0.3f);
        }
    }
}

#if TARGET_PC && D_ALBW_ARMO_PURSUIT_TEST
// ============================================
// NEW CODE — ALBW Port (giant floor-pursuit TEST)
// beginPursuitTest: snap the giant upright onto the floor and hand control to
// the pursuit state. pursuit_test: steer yaw at Link and set forward speed —
// the shared integrator at the tail of action() drives motion from speedF, so
// no manual position math is needed here. Orientation matches the retail floor
// state (angle.x = 0, floor height via field_0x73c.y = 0).
// ============================================
static void b_gm_beginPursuitTest(b_gm_class* i_this, u8 i_mode) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    // ============================================
    // NEW CODE — ALBW Port (DIAGNOSTIC — remove with the test triggers)
    // Confirms a trigger actually fired and, for EYETEST, whether the orphaned
    // ANM_EYE_TEST resource even exists. Writes albw_armo_pursuit_debug.txt.
    // No log line after a slingshot ⇒ the trigger never fired (slingshot not
    // registering on the body sphere). res=NULL ⇒ anim missing, guard skipped it.
    // ============================================
    {
        static bool sReset = false;
        char path[512];
        path[0] = '\0';
        const char* user = getenv("USERPROFILE");
        if (user && user[0] != '\0') {
            snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_armo_pursuit_debug.txt", user);
        } else {
            strncpy(path, "albw_armo_pursuit_debug.txt", sizeof(path) - 1);
        }
        FILE* fp = fopen(path, sReset ? "a" : "w");
        if (fp == NULL) {
            fp = fopen("albw_armo_pursuit_debug.txt", sReset ? "a" : "w");
        }
        if (fp != NULL) {
            sReset = true;
            void* eyeRes = (i_mode == ARMO_PURSUIT_EYETEST)
                               ? dComIfG_getObjectRes("B_gm", ANM_EYE_TEST)
                               : (void*)1;
            fprintf(fp, "[beginPursuit] mode=%d (0=dash 1=crawl 2=eyetest) eyeTestRes=%p\n",
                    (int)i_mode, eyeRes);
            fclose(fp);
        }
    }
    // ============================================
    // NEW CODE ENDS HERE
    // ============================================

    // Already chasing in this flavor — nothing to do (but allow a mode switch).
    if (i_this->mAction == ACTION_PURSUIT_TEST && s_armoPursuitMode == i_mode) {
        return;
    }

    s_armoPursuitMode = i_mode;
    i_this->mAction = ACTION_PURSUIT_TEST;
    i_this->mMode = 0;
    a_this->current.angle.x = 0;
    a_this->shape_angle.x = 0;
    i_this->field_0x73c.y = 0.0f;
    i_this->mInvincibilityTimer = 30;
}

static void b_gm_pursuit_test(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    fopAc_ac_c* player = dComIfGp_getPlayer(0);

    const bool eyeTest = (s_armoPursuitMode == ARMO_PURSUIT_EYETEST);
    const bool crawl = (s_armoPursuitMode == ARMO_PURSUIT_CRAWL);
    // EYETEST plays the orphaned ANM_EYE_TEST in place (stationary, faces Link)
    // so it can be observed; CRAWL uses the cut GOMA_MOVE; DASH the floor skitter.
    const int target_anm = eyeTest ? ANM_EYE_TEST : (crawl ? ANM_GOMA_MOVE : ANM_GOMA_DASH);
    const f32 anm_speed = eyeTest ? 1.0f : (crawl ? l_HIO.dash_anm_speed * 0.5f : l_HIO.dash_anm_speed);
    const f32 max_speed = eyeTest ? 0.0f : (crawl ? l_HIO.dash_speed * 0.35f : l_HIO.dash_speed);
    const s16 turn_rate = crawl ? 0x180 : 0x300;

    // Guard the resource: ANM_EYE_TEST is orphaned, so verify it exists before
    // setAnm rather than risk a null-anim fault.
    if (i_this->mAnmID != target_anm &&
        dComIfG_getObjectRes("B_gm", target_anm) != NULL) {
        anm_init(i_this, target_anm, 10.0f, J3DFrameCtrl::EMode_LOOP, anm_speed);
    }

    // Upright on the floor: no pitch, descend to floor height.
    a_this->current.angle.x = 0;
    cLib_addCalc2(&a_this->current.pos.y, 0.0f, 1.0f, 40.0f);

    f32 target_speed = 0.0f;
    if (player != NULL) {
        s16 yaw = cM_atan2s(player->current.pos.x - a_this->current.pos.x,
                            player->current.pos.z - a_this->current.pos.z);
        cLib_addCalcAngleS2(&a_this->current.angle.y, yaw, 4, turn_rate);

        // mPlayerDistance is refreshed at the top of action() each frame.
        if (i_this->mPlayerDistance > 350.0f) {
            target_speed = max_speed;
        }
    }

    cLib_addCalc2(&a_this->speedF, target_speed, 1.0f, l_HIO.dash_speed * 0.5f + 1.0f);
    i_this->mAcch.CrrPos(dComIfG_Bgsp());
}
// ============================================
// NEW CODE ENDS HERE
// ============================================
#endif

#if TARGET_PC && D_ALBW_ARMO_REVEAL
// ============================================
// NEW CODE — ALBW Port (phase-3 reveal ground chase)
// Entered on the 2nd statue hit (after the 35% snap + model swap). Drains the
// remaining pool via defended weapon hits during the eye-open windows; hands off
// to the E_GM eye once the pool hits the sliver threshold.
// ============================================
static void b_gm_beginPhase3(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    i_this->mAction = ACTION_PHASE3;
    i_this->mMode = P3_INTRO;
    // Do NOT snap the orientation here. The boss is smashed upside-down; the RETURN
    // "get up" anim carries the reorientation (as vanilla does) and we snap upright
    // only when it finishes. A one-frame flip would leave the foot IK targeting the
    // ceiling and blow the model apart -- the "model breaks" seen in playtest.
    i_this->field_0x73c.y = 0.0f;
    i_this->mInvincibilityTimer = 60;
    s_gmPhase3Active = true;
    s_gmPhase3EyeOpen = false;
    s_gmPhase3HitCount = 0;
    anm_init(i_this, ANM_GOMA_RETURN, 5.0f, J3DFrameCtrl::EMode_NONE, 1.0f);
}

static void b_gm_phase3EnterVuln(b_gm_class* i_this) {
    // Stagger: stop, stand exposed, eye open the whole 8s. The counter resets when it
    // ends; hits here drain HP but don't re-count. NOTE: LANDING_WAIT is authored for
    // the on-its-back (upside-down) drop pose and sinks under the floor when played
    // upright, so use the upright-safe WAIT (vanilla plays it upright post-RETURN).
    i_this->mMode = P3_VULN;
    i_this->mTimers[0] = kAlbwArmoP3VulnFrames;
    s_gmPhase3EyeOpen = true;
    anm_init(i_this, ANM_GOMA_WAIT, 5.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.wait_anm_speed);
}

static void b_gm_phase3EnterLaser(b_gm_class* i_this) {
    // Laser interlude: lock in place, eye INVULNERABLE (field_0x1ad6 stays 0), track
    // Link and fire the lagging beam from t=3s. Do NOT play eye_test.bck on the reveal
    // model (it would yank the baked eye off the mouth) -- a neutral hold pose keeps
    // the eye put; the beam-render block owns the tracking + laser.
    i_this->mMode = P3_LASER;
    i_this->mTimers[0] = kAlbwArmoP3LaserFrames;
    s_gmPhase3EyeOpen = false;
    i_this->field_0x6c0 = 0.0f;   // beam off until the ignite moment
    i_this->field_0x6f4 = 0;      // re-init the beam aim when it lights
    anm_init(i_this, ANM_GOMA_WAIT, 5.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.wait_anm_speed);
}

static void b_gm_beginPhase3Handoff(b_gm_class* i_this) {
    // Final drain hit: instead of snapping to the death, play the get-up (RETURN) in
    // REVERSE so the boss lies back down into the upside-down smashed pose -- the mirror
    // of how it stood up entering phase 3. Orientation stays anim-driven (angle.x
    // untouched), so RETURN's frame-0 pose lands exactly on the pose ANM_GOMA_DEATH
    // continues from. When the reverse finishes (P3_LIEDOWN in b_gm_phase3) we fire the
    // real death handoff (b_gm_beginPhase3Death). Momentum is killed here so it doesn't
    // drift through the lie-down / cutscene (which would drag eyePos -> the E_GM spawn).
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    a_this->speedF = 0.0f;
    a_this->speed.set(0.0f, 0.0f, 0.0f);
    i_this->field_0x6c0 = 0.0f;         // kill any lingering beam
    s_gmPhase3EyeOpen = false;
    i_this->mInvincibilityTimer = 120;  // ignore late hits through the transition
    i_this->mMode = P3_LIEDOWN;
    // A NEGATIVE rate makes setAnm seek to the last frame and set the loop boundary for
    // reverse playback (m_Do_ext.cpp setAnm) -- the get-up runs backward as a lie-down.
    // (Seeking manually after a positive-rate init left the loop frame at 0 and snapped
    // straight to frame 0 -- the "instant flip".)
    anm_init(i_this, ANM_GOMA_RETURN, 5.0f, J3DFrameCtrl::EMode_NONE, -1.0f);
}

static void b_gm_beginPhase3Death(b_gm_class* i_this) {
    // Relocated vanilla transition: the same writes the 3rd statue hit made, just
    // fired from the chase once the pool drains AND the boss has laid back down
    // (P3_LIEDOWN). demo_camera(mDemoMode=30) then runs the disappear cutscene and
    // spawns the E_GM floor eye. b_gm_drop no-ops on mMode 20, matching the vanilla
    // post-statue state.
    s_gmPhase3Active = false;
    i_this->field_0x6c0 = 0.0f;   // kill any lingering beam
    i_this->mAction = ACTION_DROP;
    i_this->mMode = 20;
    i_this->mDemoMode = 30;
    fpcM_Search(s_ko_del, i_this);
    Z2GetAudioMgr()->bgmStop(0x1E, 0);
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
        fprintf(fp, "[death] handoff fired -> ACTION_DROP mMode=20 mDemoMode=30\n"); fclose(fp); } }
#endif
}

static void b_gm_phase3(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    fopAc_ac_c* player = dComIfGp_getPlayer(0);

    // Get-up intro: keep the smashed (upside-down) orientation while RETURN plays,
    // then on its final frame snap upright, swap in the revealed model, and start the
    // chase. We stop "early" -- we never continue into the vanilla wall-climb. Handled
    // before the upright logic below so we don't force angle.x = 0 mid-reorient.
    if (i_this->mMode == P3_INTRO) {
        cLib_addCalc2(&a_this->speedF, 0.0f, 1.0f, l_HIO.dash_speed * 0.5f + 1.0f);
        cLib_addCalc2(&a_this->current.pos.y, 0.0f, 1.0f, 40.0f);
        if (i_this->mpModelMorf->isStop()) {
            a_this->current.angle.x = 0;
            a_this->shape_angle.x = 0;
#if D_ALBW_ARMO_P3_DIAG_NOSWAP
            // DIAGNOSTIC (temporary): keep the vanilla model in phase 3 to isolate the
            // upside-down break. If the vanilla body reorients + dashes upright cleanly,
            // the fault is the reveal BMD's export orientation, not this code. Flip the
            // define back to 0 to restore the reveal swap.
#else
            b_gm_activateReveal(i_this);  // reveal only once upright
#endif
            anm_init(i_this, ANM_GOMA_DASH, 10.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.dash_anm_speed * 0.95f);
            i_this->mMode = P3_DASH;
            i_this->mTimers[0] = kAlbwArmoP3EyeCycleFrames;
        }
        i_this->mAcch.CrrPos(dComIfG_Bgsp());
        return;
    }

    // Final lie-down (mirror of the get-up): RETURN is playing in REVERSE (set up in
    // b_gm_beginPhase3Handoff), laying the boss back into the upside-down smashed pose.
    // Hold still; when the reverse reaches frame 0 the body is upside-down and we fire
    // the real death handoff. angle.x stays 0 -- the reversed anim carries the flip, so
    // it lands exactly on the pose ANM_GOMA_DEATH continues from.
    if (i_this->mMode == P3_LIEDOWN) {
        cLib_addCalc2(&a_this->speedF, 0.0f, 1.0f, 5.0f);
        cLib_addCalc2(&a_this->current.pos.y, 0.0f, 1.0f, 40.0f);
        i_this->mInvincibilityTimer = 5;
        if (i_this->mpModelMorf->getFrame() <= 0.5f || i_this->mpModelMorf->isStop()) {
            b_gm_beginPhase3Death(i_this);  // upside-down now -> disappear cutscene -> E_GM
        }
        i_this->mAcch.CrrPos(dComIfG_Bgsp());
        return;
    }

    a_this->current.angle.x = 0;
    cLib_addCalc2(&a_this->current.pos.y, 0.0f, 1.0f, 40.0f);

    s16 yaw = a_this->current.angle.y;
    if (player != NULL) {
        yaw = cM_atan2s(player->current.pos.x - a_this->current.pos.x,
                        player->current.pos.z - a_this->current.pos.z);
    }

    f32 target_speed = 0.0f;

    switch (i_this->mMode) {
    case P3_DASH:
        // Eye blink/open cycle (5s each). field_0x1ad6 (cleared in action() this
        // frame) gates the core sphere -> the open window IS the vulnerable window.
        if (i_this->mTimers[0] == 0) {
            s_gmPhase3EyeOpen = !s_gmPhase3EyeOpen;
            i_this->mTimers[0] = kAlbwArmoP3EyeCycleFrames;
        }
        if (s_gmPhase3EyeOpen) {
            i_this->field_0x1ad6 = 1;
        }
        // Keep chasing even while the eye is open (do NOT freeze). Dash anim slowed 5%.
        if (i_this->mAnmID != ANM_GOMA_DASH &&
            dComIfG_getObjectRes("B_gm", ANM_GOMA_DASH) != NULL) {
            anm_init(i_this, ANM_GOMA_DASH, 10.0f, J3DFrameCtrl::EMode_LOOP, l_HIO.dash_anm_speed * 0.95f);
        }
        cLib_addCalcAngleS2(&a_this->current.angle.y, yaw, 4, 0x300);
        if (i_this->mPlayerDistance > 350.0f) {
            // ALBW: phase-3 chase dash slowed 3% then a further 3% per playtest
            // (0.97 * 0.97 = 0.9409; movement only -- the dash anim runs at x0.95 above).
            target_speed = l_HIO.dash_speed * 0.9409f;
        }
        break;

    case P3_VULN:
        // Stationary, eye exposed the whole stagger. Slow face toward Link.
        i_this->field_0x1ad6 = 1;
        cLib_addCalcAngleS2(&a_this->current.angle.y, yaw, 4, 0x80);
        if (i_this->mTimers[0] == 0) {
            s_gmPhase3HitCount = 0;
            s_gmPhase3EyeOpen = false;
            i_this->mMode = P3_DASH;
            i_this->mTimers[0] = kAlbwArmoP3EyeCycleFrames;
        }
        break;

    case P3_LASER: {
        // Locked, eye invulnerable (never raise field_0x1ad6). Wind-up faces Link; once
        // the beam lights the body SWEEPS IN PLACE through kAlbwArmoLaserTurns alternating
        // 180 deg turns, each completing its arc 20% faster than the last (the whole
        // series scaled to fill the fire window). The eye-mounted beam (fired straight
        // forward, see the beam block) arcs across the arena. Centered on Link at ignite.
        const int elapsed = kAlbwArmoP3LaserFrames - static_cast<int>(i_this->mTimers[0]);
        if (elapsed < kAlbwArmoP3LaserFireAt) {
            cLib_addCalcAngleS2(&a_this->current.angle.y, yaw, 8, 0x435);
            s_gmLaserSweepCenter = a_this->current.angle.y;  // keep centered on Link until ignite
        } else {
            cLib_addCalc2(&i_this->field_0x6c0, 1.0f, 1.0f, 0.105f);
            const int fireDur = kAlbwArmoP3LaserFrames - kAlbwArmoP3LaserFireAt;
            const int fe = elapsed - kAlbwArmoP3LaserFireAt;

            // Geometric turn durations (each *0.8) scaled so all N turns fill fireDur.
            f32 sum = 0.0f, w = 1.0f;
            for (int t = 0; t < kAlbwArmoLaserTurns; t++) { sum += w; w *= kAlbwArmoLaserTurnSpeedup; }
            f32 dur = (f32)fireDur / sum;   // turn-1 duration (frames)

            int k = kAlbwArmoLaserTurns - 1;   // current turn (0-based)
            f32 p = 1.0f;                      // progress within the turn (0..1)
            f32 acc = 0.0f;
            for (int t = 0; t < kAlbwArmoLaserTurns; t++) {
                if ((f32)fe < acc + dur) { k = t; p = ((f32)fe - acc) / dur; break; }
                acc += dur;
                dur *= kAlbwArmoLaserTurnSpeedup;
            }
            if (p < 0.0f) p = 0.0f; else if (p > 1.0f) p = 1.0f;

            // Alternate direction each turn: even turns sweep -90 -> +90, odd +90 -> -90.
            const f32 span = (f32)(2 * (int)kAlbwArmoLaserSweepHalf) * p;
            s16 off;
            if ((k & 1) == 0) {
                off = (s16)(-(f32)kAlbwArmoLaserSweepHalf + span);
            } else {
                off = (s16)((f32)kAlbwArmoLaserSweepHalf - span);
            }
            a_this->current.angle.y = s_gmLaserSweepCenter + off;
        }
        if (i_this->mTimers[0] == 0) {
            i_this->field_0x6c0 = 0.0f;
            i_this->field_0x6f4 = 0;
            s_gmPhase3EyeOpen = false;
            i_this->mMode = P3_DASH;
            i_this->mTimers[0] = kAlbwArmoP3EyeCycleFrames;
        }
        break;
    }
    }

    cLib_addCalc2(&a_this->speedF, target_speed, 1.0f, l_HIO.dash_speed * 0.5f + 1.0f);
    i_this->mAcch.CrrPos(dComIfG_Bgsp());
}
// ============================================
#endif

static void action(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    cXyz local_move;
    cXyz move_vec;

    i_this->mAngleToPlayer = fopAcM_searchPlayerAngleY(i_this);
    i_this->mPlayerDistance = fopAcM_searchPlayerDistance(i_this);

    damage_check(i_this);

    i_this->field_0x1ad6 = 0;

    switch (i_this->mAction) {
    case ACTION_WAIT:
        b_gm_wait(i_this);
        break;
    case ACTION_MOVE:
        b_gm_move(i_this);
        break;
    case ACTION_BEAM:
        b_gm_beam(i_this);
        break;
    case ACTION_KOGOMA:
        b_gm_kogoma(i_this);
        break;
    case ACTION_DAMAGE:
        b_gm_damage(i_this);
        break;
    case ACTION_DROP:
        b_gm_drop(i_this);
        break;
#if TARGET_PC && D_ALBW_ARMO_PURSUIT_TEST
    case ACTION_PURSUIT_TEST:
        b_gm_pursuit_test(i_this);
        break;
#endif
#if TARGET_PC && D_ALBW_ARMO_REVEAL
    case ACTION_PHASE3:
        b_gm_phase3(i_this);
        break;
#endif
    }

    cMtx_YrotS(*calc_mtx, a_this->current.angle.y);
    cMtx_XrotM(*calc_mtx, a_this->current.angle.x);
    local_move.x = 0.0f;
    local_move.y = 0.0f;
    local_move.z = a_this->speedF;
    MtxPosition(&local_move, &move_vec);
    a_this->current.pos += move_vec;

    cLib_addCalcAngleS2(&a_this->shape_angle.y, a_this->current.angle.y, 2, 0x1000);
    cLib_addCalcAngleS2(&a_this->shape_angle.x, a_this->current.angle.x, 2, 0x1000);

    if (i_this->field_0x566 != 0) {
        daPy_getPlayerActorClass()->onBossRoomWait();
    }
}

static int foot_IK(b_gm_class* i_this, b_gm_foot* i_foot) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    int var_r28 = 0;
    dBgS_LinChk line_chk;

    i_foot->field_0x1c = i_foot->field_0x10;
    line_chk.Set(&i_this->field_0x730, &i_foot->field_0x10, i_this);
    if (dComIfG_Bgsp().LineCross(&line_chk)) {
        i_foot->field_0x1c = line_chk.GetCross();
        f32 temp_f31 = (i_foot->field_0x1c - i_foot->field_0x10).abs();

        i_foot->field_0x0 += 10.0f + (10.0f + (temp_f31 * 4.0f));
        if (i_foot->field_0x0 > 20000.0f) {
            i_foot->field_0x0 = 20000.0f;
        } else {
            var_r28 = 1;
        }

        i_foot->field_0x4 = 0.0f;
    } else {
        cLib_addCalc2(&i_foot->field_0x0, 0.0f, 1.0f, i_foot->field_0x4);
        cLib_addCalc2(&i_foot->field_0x4, 1000.0f, 1.0f, 50.0f + (2.0f * a_this->speedF));
    }

    i_foot->field_0x8[0] = i_foot->field_0x0 * l_HIO.bend_degree_1;

    if (i_foot->field_0x0 > 0.0f) {
        i_foot->field_0xa = i_foot->field_0x0 * l_HIO.bend_degree_2;
        i_foot->field_0xc = i_foot->field_0x0 * l_HIO.bend_degree_3;
    } else {
        i_foot->field_0xc = 0;
        i_foot->field_0xa = 0;
    }

    return var_r28;
}

static cXyz top_pos_data[] = {
    cXyz(260.0f, 0.0f, 0.0f),
    cXyz(280.0f, 0.0f, 0.0f),
    cXyz(300.0f, 0.0f, 0.0f),
    cXyz(280.0f, 0.0f, 0.0f),
};

static int top_j[] = {
    0x1B,
    0x1F,
    0x23,
    0x27,
    0x2B,
    0x2F,
    0x33,
    0x37,
    0x3C,
    0x41,
};

static void foot_IK_main(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    cXyz spC;
    b_gm_foot* foot_p = i_this->mFoot;
    J3DModel* model = i_this->mpModelMorf->getModel();

    cMtx_YrotS(*calc_mtx, a_this->current.angle.y);
    cMtx_XrotM(*calc_mtx, a_this->current.angle.x);
    spC.x = 0.0f;
    spC.y = 180.0f;
    spC.z = 0.0f;
    MtxPosition(&spC, &i_this->field_0x730);
    i_this->field_0x730 += a_this->current.pos;

    int var_r26 = 0;

    for (int i = 0; i < 10; i++, foot_p++) {
        MTXCopy(model->getAnmMtx(top_j[i]), *calc_mtx);
        MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);

        if (i >= 8) {
            spC.set(0.0f, 0.0f, 0.0f);
            MtxPosition(&spC, &foot_p->field_0x10);
        } else {
            MtxPosition(&top_pos_data[i & 3], &foot_p->field_0x10);
        }

        if (foot_IK(i_this, foot_p)) {
            var_r26++;
            if (var_r26 < 80) {
                i_this->mpModelMorf->updateDL();
                i--;
                foot_p--;
            }
        }
    }
}

static void anm_se_set(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

    int foot_idx = -1;
    if (i_this->mAnmID == ANM_GOMA_DASH) {
        if (i_this->mpModelMorf->checkFrame(3)) {
            foot_idx = 1;
        } else if (i_this->mpModelMorf->checkFrame(4)) {
            foot_idx = 6;
        } else if (i_this->mpModelMorf->checkFrame(6)) {
            foot_idx = 3;
        } else if (i_this->mpModelMorf->checkFrame(8)) {
            foot_idx = 5;
        } else if (i_this->mpModelMorf->checkFrame(10)) {
            foot_idx = 0;
        } else if (i_this->mpModelMorf->checkFrame(11)) {
            foot_idx = 7;
        } else if (i_this->mpModelMorf->checkFrame(14)) {
            foot_idx = 2;
        } else if (i_this->mpModelMorf->checkFrame(15)) {
            foot_idx = 4;
        } else if (i_this->mpModelMorf->checkFrame(18)) {
            foot_idx = 1;
        } else if (i_this->mpModelMorf->checkFrame(19)) {
            foot_idx = 6;
        } else if (i_this->mpModelMorf->checkFrame(21)) {
            foot_idx = 3;
        } else if (i_this->mpModelMorf->checkFrame(23)) {
            foot_idx = 5;
        } else if (i_this->mpModelMorf->checkFrame(25)) {
            foot_idx = 0;
        } else if (i_this->mpModelMorf->checkFrame(26)) {
            foot_idx = 7;
        } else if (i_this->mpModelMorf->checkFrame(29)) {
            foot_idx = 2;
        } else if (i_this->mpModelMorf->checkFrame(0)) {
            foot_idx = 4;
        }
    }

    if (i_this->mpModelMorf->checkFrame(1)) {
        if (i_this->mAnmID == ANM_GOMA_ATTACK_A) {
            i_this->mCreatureSound.startCreatureVoice(Z2SE_EN_GO_V_BEAMATTACK_A, -1);
        } else if (i_this->mAnmID == ANM_GOMA_ATTACK_C) {
            i_this->mCreatureSound.startCreatureVoice(Z2SE_EN_GO_V_BEAMATTACK_C, -1);
        } else if (i_this->mAnmID == ANM_GOMA_LANDING) {
            i_this->mCreatureSound.startCreatureSound(Z2SE_EN_GO_LANDING, 0, -1);
        } else if (i_this->mAnmID == ANM_GOMA_LANDING_DAMAGE) {
            i_this->mCreatureSound.startCreatureVoice(Z2SE_EN_GO_V_LANDING_DAMAGE, -1);
        } else if (i_this->mAnmID == ANM_GOMA_RETURN) {
            i_this->mCreatureSound.startCreatureSound(Z2SE_EN_GO_RETURN, 0, -1);
        } else if (i_this->mAnmID == ANM_GOMA_ROOF_DAMAGE) {
            i_this->mCreatureSound.startCreatureVoice(Z2SE_EN_GO_V_ROOF_DAMAGE, -1);
        } else if (i_this->mAnmID == ANM_GOMA_DEATH) {
            i_this->mCreatureSound.startCreatureVoice(Z2SE_EN_GO_V_DEATH, -1);
        }
    }

    if (i_this->mDemoMode == 0 && i_this->mAnmID == ANM_GOMA_DASH) {
        i_this->mCreatureSound.startCreatureSoundLevel(Z2SE_EN_GO_FOOTNOTE, 0, -1);
    } else if (i_this->mAnmID == ANM_GOMA_LANDING_WAIT) {
        i_this->mCreatureSound.startCreatureSoundLevel(Z2SE_EN_GO_LANDING_WAIT, 0, -1);
    } else if (i_this->mAnmID == ANM_GOMA_WAIT) {
        i_this->mCreatureSound.startCreatureSoundLevel(Z2SE_EN_GO_WAIT, 0, -1);
    }

    if (foot_idx >= 0) {
        cXyz sp60;
        csXyz angle;

        sp60 = a_this->current.pos - i_this->mFoot[foot_idx].field_0x1c;
        angle.y = cM_atan2s(sp60.x, sp60.z);
        angle.x = cM_atan2s(sp60.y, JMAFastSqrt(SQUARE(sp60.x) + SQUARE(sp60.z)));
        angle.z = 0;

        GXColor prim_col;
        prim_col.r = l_HIO.smoke_prim_R;
        prim_col.g = l_HIO.smoke_prim_G;
        prim_col.b = l_HIO.smoke_prim_B;
        prim_col.a = 0xFF;

        GXColor env_col;
        env_col.r = l_HIO.smoke_env_R;
        env_col.g = l_HIO.smoke_env_G;
        env_col.b = l_HIO.smoke_env_B;
        env_col.a = 0xFF;

        dComIfGp_particle_setColor(0x81EF, &i_this->mFoot[foot_idx].field_0x1c, &a_this->tevStr, &prim_col, &env_col, l_HIO.smoke_blend, l_HIO.smoke_alpha, &angle, NULL, NULL, -1, NULL);
        dComIfGp_particle_set(0x81EE, &i_this->mFoot[foot_idx].field_0x1c, &angle, NULL);
    }
}

static void cam_3d_morf(b_gm_class* i_this, f32 i_scale) {
    cLib_addCalc2(&i_this->mDemoCamCenter.x, i_this->mDemoCamCenterTarget.x, i_scale, i_this->mDemoCamCenterSpeed.x * i_this->field_0x1cdc);
    cLib_addCalc2(&i_this->mDemoCamCenter.y, i_this->mDemoCamCenterTarget.y, i_scale, i_this->mDemoCamCenterSpeed.y * i_this->field_0x1cdc);
    cLib_addCalc2(&i_this->mDemoCamCenter.z, i_this->mDemoCamCenterTarget.z, i_scale, i_this->mDemoCamCenterSpeed.z * i_this->field_0x1cdc);

    cLib_addCalc2(&i_this->mDemoCamEye.x, i_this->mDemoCamEyeTarget.x, i_scale, i_this->mDemoCamEyeSpeed.x * i_this->field_0x1cdc);
    cLib_addCalc2(&i_this->mDemoCamEye.y, i_this->mDemoCamEyeTarget.y, i_scale, i_this->mDemoCamEyeSpeed.y * i_this->field_0x1cdc);
    cLib_addCalc2(&i_this->mDemoCamEye.z, i_this->mDemoCamEyeTarget.z, i_scale, i_this->mDemoCamEyeSpeed.z * i_this->field_0x1cdc);
}

static void cam_spd_set(b_gm_class* i_this) {
    i_this->mDemoCamEyeSpeed.x = fabsf(i_this->mDemoCamEyeTarget.x - i_this->mDemoCamEye.x);
    i_this->mDemoCamEyeSpeed.y = fabsf(i_this->mDemoCamEyeTarget.y - i_this->mDemoCamEye.y);
    i_this->mDemoCamEyeSpeed.z = fabsf(i_this->mDemoCamEyeTarget.z - i_this->mDemoCamEye.z);

    i_this->mDemoCamCenterSpeed.x = fabsf(i_this->mDemoCamCenterTarget.x - i_this->mDemoCamCenter.x);
    i_this->mDemoCamCenterSpeed.y = fabsf(i_this->mDemoCamCenterTarget.y - i_this->mDemoCamCenter.y);
    i_this->mDemoCamCenterSpeed.z = fabsf(i_this->mDemoCamCenterTarget.z - i_this->mDemoCamCenter.z);

    i_this->field_0x1cdc = 0.0f;
}

static void demo_camera(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
    daPy_py_c* player = (daPy_py_c*)dComIfGp_getPlayer(0);
    camera_process_class* camera = dComIfGp_getCamera(dComIfGp_getPlayerCameraID(0));
    camera_process_class* sp20 = dComIfGp_getCamera(0);
    cXyz spC8;
    cXyz spBC;
    cXyz spB0;
    cXyz spA4;
    
    s8 sp10 = 0;

    switch (i_this->mDemoMode) {
    case 1:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

        camera->mCamera.Stop();
        i_this->mDemoMode = 2;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;

        player->changeOriginalDemo();
        player->changeDemoMode(daPy_demo_c::DEMO_LOOK_AROUND_e, 0, 0, 0);

        spBC.set(0.0f, 0.0f, 2418.0f);
        daPy_getPlayerActorClass()->setPlayerPosAndAngle(&spBC, 0x8000, 0);

        dComIfGp_getEvent()->startCheckSkipEdge(i_this);
        
        camera->mCamera.SetTrimSize(3);
        mDoAud_seStart(Z2SE_EN_GO_DEMO_OP, NULL, 0, 0);
        Z2GetAudioMgr()->subBgmStart(Z2BGM_GOMA_D01);
    case 2:
        if (i_this->mDemoModeTimer >= 1) {
            if (i_this->mDemoModeTimer == 1) {
                i_this->mDemoCamCenter.set(-400.0f, 130.0f, 1883.0f);
                i_this->mDemoCamEye.set(0.0f, 197.0f, 2680.0f);
            }

            cLib_addCalc2(&i_this->mDemoCamCenter.x, 400.0f, 0.05f, i_this->field_0x1cdc * 4.0f);
            cLib_addCalc2(&i_this->field_0x1cdc, 1.0f, 1.0f, 0.04f);
            i_this->mDemoCamEye.z -= 1.0f;
        }

        if (i_this->mDemoModeTimer == 140) {
            i_this->mDemoMode = 3;
            i_this->mDemoModeTimer = 0;

            spBC.set(-781.0f, 0.0f, -601.0f);
            daPy_getPlayerActorClass()->setPlayerPosAndAngle(&spBC, -0x6249, 0);

            player->changeDemoMode(daPy_demo_c::DEMO_UNK_2_e, 0, 0, 0);
            spBC.set(-1267.0f, 0.0f, -1208.0f);
            player->changeDemoPos0(&spBC);
            
            i_this->mDemoCamCenter.set(-1090.0f, 227.0f, -1070.0f);
            i_this->mDemoCamEye.set(-688.0f, 155.0f, -680.0f);
        }
        break;
    case 3:
        if (i_this->mDemoModeTimer >= 40) {
            cLib_addCalc2(&i_this->mDemoCamCenter.y, 317.0f, 0.05f, 0.8f);
        }

        if (i_this->mDemoModeTimer == 170) {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_LOOK_AROUND_e, 0, 0, 0);
        }

        if (i_this->mDemoModeTimer == 230) {
            i_this->field_0x6f5 = 1;
            i_this->field_0x1cfc = 1;
        }

        if (i_this->mDemoModeTimer == 250) {
            player->changeDemoMode(daPy_demo_c::DEMO_TURN_BACK_e, 0, 0, 0);
        }

        if (i_this->mDemoModeTimer == 277) {
            i_this->mDemoMode = 4;
            i_this->mDemoModeTimer = 0;
            
            i_this->field_0x1ce4 = player->shape_angle.y;
            i_this->mDemoCamCenter.set(-973.0f, 192.0f, -991.0f);
            i_this->mDemoCamEye.set(-1368.0f, 158.0f, -1397.0f);
            i_this->field_0x1cdc = 0.0f;
            i_this->field_0x6f5 = 0;
            i_this->field_0x1cfc = 2;
        }
        break;
    case 4:
        cLib_addCalcAngleS2(&i_this->field_0x1ce4, i_this->mAngleToPlayer + 0x8000, 1, 0x1000);
        daPy_getPlayerActorClass()->setPlayerPosAndAngle(&player->current.pos, i_this->field_0x1ce4, 0);

        if (i_this->mDemoModeTimer == 25) {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNK_23_e, 1, 2, 0);
        }

        if (i_this->mDemoModeTimer >= 55) {
            cLib_addCalc2(&i_this->mDemoCamCenter.y, 780.0f, 0.3f, 50.0f);

            if (i_this->mDemoModeTimer >= 260) {
                if (i_this->mDemoModeTimer == 260) {
                    i_this->field_0x1cdc = 0.0f;
                }

                cLib_addCalc2(&i_this->mDemoCamFovy, 40.0f, 0.2f, i_this->field_0x1cdc);
                cLib_addCalc2(&i_this->field_0x1cdc, 1.0f, 1.0f, 0.05f);

                if (i_this->mDemoModeTimer == 280) {
                    fopMsgM_messageSetDemo(0x484);
                }

                if (i_this->mDemoModeTimer == 390) {
                    i_this->mMode = 4;
                    i_this->mTimers[2] = cM_rndF(100.0f) + 200.0f;
                }

                if (i_this->mDemoModeTimer == 420) {
                    sp10 = 1;
                    Z2GetAudioMgr()->bgmStart(Z2BGM_GOMA_BTL01, 0, 0);
                    a_this->field_0x566 = 1;
                }
            } else if (i_this->mDemoModeTimer >= 205) {
                cLib_addCalc2(&i_this->mDemoCamFovy, 10.0f, 0.2f, 3.0f);
            } else if (i_this->mDemoModeTimer >= 85) {
                cLib_addCalc2(&i_this->mDemoCamFovy, 20.0f, 0.05f, i_this->field_0x1cdc * 5.0f);
                cLib_addCalc2(&i_this->field_0x1cdc, 1.0f, 1.0f, 0.02f);
            }

            if (i_this->mDemoModeTimer >= 125) {
                i_this->field_0x1ad6 = 1;

                if (i_this->mDemoModeTimer == 125) {
                    i_this->field_0x1ade = 60;
                }

                if (i_this->mDemoModeTimer == 205) {
                    i_this->field_0x1ae0 = 0.0f;
                }
            }
        }
        break;
    case 10:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

        camera->mCamera.Stop();
        i_this->mDemoMode = 11;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;
        
        camera->mCamera.SetTrimSize(3);
        i_this->field_0x1ce0 = 900.0f;
        i_this->field_0x1cdc = 0.0f;
    case 11:
        cMtx_YrotS(*calc_mtx, a_this->current.angle.y);
        spC8.x = i_this->field_0x1ce0;
        spC8.y = -1500.0f;
        spC8.z = 1300.0f;
        MtxPosition(&spC8, &spB0);

        spB0 += a_this->current.pos;
        spA4 = a_this->eyePos;

        cLib_addCalc2(&i_this->field_0x1ce0, -900.0f, 0.1f, i_this->field_0x1cdc * 12.0f);
        cLib_addCalc2(&i_this->field_0x1cdc, 1.0f, 1.0f, 0.05f);

        if (i_this->mDemoModeTimer == 0) {
            i_this->mDemoCamCenter = spA4;
            i_this->mDemoCamEye = spB0;
        } else {
            cLib_addCalc2(&i_this->mDemoCamCenter.x, spA4.x, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamCenter.z, spA4.z, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.x, spB0.x, 0.1f, 30.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.z, spB0.z, 0.1f, 30.0f);
        }

        if (i_this->mAction != ACTION_KOGOMA) {
            sp10 = 1;
        }
        break;
    case 20:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

        camera->mCamera.Stop();
        i_this->mDemoMode = 21;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;
        
        camera->mCamera.SetTrimSize(3);
        i_this->field_0x1cdc = 0.0f;
    case 21:
        cMtx_YrotS(*calc_mtx, cM_atan2s(-a_this->current.pos.x, -a_this->current.pos.z));
        spC8.x = 300.0f;
        spC8.y = 1200.0f;
        spC8.z = 1300.0f;
        MtxPosition(&spC8, &spB0);

        spB0 += a_this->current.pos;
        spA4 = a_this->eyePos;
        spA4.y += 350.0f;

        if (i_this->mDemoModeTimer == 0) {
            i_this->mDemoCamCenter = spA4;
            i_this->mDemoCamEye = spB0;
        } else {
            cLib_addCalc2(&i_this->mDemoCamCenter.x, spA4.x, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamCenter.z, spA4.z, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.x, spB0.x, 0.1f, 30.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.z, spB0.z, 0.1f, 30.0f);
        }

        if (i_this->mDemoModeTimer == 40) {
            sp10 = 1;
        }
        break;
    case 30:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

#if D_ALBW_ARMO_DEATH_DIAG
        { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
            fprintf(fp, "[death] demo30 event ACCEPTED -> 31\n"); fclose(fp); } }
#endif
        camera->mCamera.Stop();
        i_this->mDemoMode = 31;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;
        camera->mCamera.SetTrimSize(3);

        i_this->field_0x1cdc = 0.0f;
        i_this->field_0x1ce0 = 1800.0f;
        i_this->field_0x1ce4 = 0;
        i_this->field_0x1ce8 = -10000;
        i_this->field_0x1cdc = 0.0f;

        player->changeOriginalDemo();
        daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNK_23_e, 1, 2, 0);        

        Z2GetAudioMgr()->bgmStreamPrepare(0x200004E);
        Z2GetAudioMgr()->bgmStreamPlay();
    case 31:
        if (i_this->mDemoModeTimer < 10) {
            cMtx_YrotS(*calc_mtx, cM_atan2s(-a_this->current.pos.x, -a_this->current.pos.z));
            spC8.x = 1100.0f;
            spC8.y = 0.0f;
            spC8.z = -1000.0f;
            MtxPosition(&spC8, &spBC);
            spBC += a_this->current.pos;
            spBC.y = 0.0f;

            daPy_getPlayerActorClass()->setPlayerPosAndAngle(&spBC, i_this->mAngleToPlayer + 0x8000, 0);
        }

        cMtx_YrotS(*calc_mtx, i_this->field_0x1ce4 + cM_atan2s(-a_this->current.pos.x, -a_this->current.pos.z));
        cMtx_XrotM(*calc_mtx, i_this->field_0x1ce8);

        spC8.x = 0.0f;
        spC8.y = 0.0f;
        spC8.z = i_this->field_0x1ce0;
        MtxPosition(&spC8, &i_this->mDemoCamEye);
        i_this->mDemoCamEye += a_this->current.pos;
        i_this->mDemoCamCenter = a_this->eyePos;
        i_this->mDemoCamCenter.y += 350.0f;

        if (i_this->mDemoModeTimer > 120) {
            cLib_addCalcAngleS2(&i_this->field_0x1ce4, 23000, 0x10, i_this->field_0x1cdc * 230.0f);
            cLib_addCalcAngleS2(&i_this->field_0x1ce8, -700, 0x10, i_this->field_0x1cdc * 93.0f);
            cLib_addCalc2(&i_this->field_0x1cdc, 1.0f, 1.0f, 0.04f);
        }

        if (i_this->mDemoModeTimer == 280) {
            MTXCopy(i_this->mpModelMorf->getModel()->getAnmMtx(2), mDoMtx_stack_c::get());
            mDoMtx_stack_c::multVecZero(&spC8);

            dComIfGp_particle_set(0x8B42, &spC8, NULL, NULL);
            fopAcM_createDisappear(a_this, &spC8, 49, 0, 0xFF);
            i_this->mIsDisappear = true;
            dComIfGs_onOneZoneSwitch(5, -1);
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
                fprintf(fp, "[death] demo31 disappear @timer280, mIsDisappear set\n"); fclose(fp); } }
#endif
        }

        if (i_this->mDemoModeTimer == 320) {
            player->changeDemoMode(daPy_demo_c::DEMO_SWORD_UNEQUIP_SP_e, 1, 0, 0);
        }

        if (i_this->mDemoModeTimer != 340) {
            break;
        }
        
        i_this->mDemoMode = 32;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 65.0f;
#if D_ALBW_ARMO_DEATH_DIAG
        { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
            fprintf(fp, "[death] demo31 -> 32 (E_GM spawn window)\n"); fclose(fp); } }
#endif
    case 32:
        cMtx_YrotS(*calc_mtx, player->shape_angle.y + 8000);
        spC8.x = 0.0f;
        spC8.y = 90.0f;
        spC8.z = 110.0f;
        MtxPosition(&spC8, &i_this->mDemoCamEye);

        i_this->mDemoCamEye += player->current.pos;
        i_this->mDemoCamCenter = player->current.pos;
        i_this->mDemoCamCenter.y += 140.0f;

        if (i_this->mDemoModeTimer & 1) {
            int sp1C = i_this->mDemoModeTimer >> 1;
            int params;
            if (sp1C == 0) {
                params = 3;
            } else {
                params = 1;
            }

            cMtx_YrotS(*calc_mtx, sp1C * 0x1300);
            spC8.x = 0.0f;
            spC8.y = 0.0f;

            if (sp1C == 0) {
                spC8.z = 0.0f;
            } else {
                spC8.z = sp1C * 5.0f + 300.0f;
            }

            MtxPosition(&spC8, &spBC);
            spBC += a_this->eyePos;

#if TARGET_PC && D_ALBW_ARMO_REVEAL
            // The reveal death pose sinks the (mouth) eye far underground (eyePos.y is
            // ~ -286), so the floor eyes would all spawn below the floor and never
            // surface. Keep them at the boss's floor level for the reveal fight; the
            // vanilla upside-down giant is unaffected (s_gmRevealActive is false there).
            if (s_gmRevealActive && spBC.y < a_this->current.pos.y) {
                spBC.y = a_this->current.pos.y;
            }
#endif

            csXyz angle(0, cM_rndF(0x10000), 0);
            fopAcM_createChild(fpcNm_E_GM_e, fopAcM_GetID(a_this), params, &spBC, fopAcM_GetRoomNo(a_this), &angle, NULL, -1, NULL);
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
                fprintf(fp, "[death] demo32 createChild E_GM params=%d timer=%d pos=(%.0f,%.0f,%.0f) eyePos=(%.0f,%.0f,%.0f)\n",
                        params, (int)i_this->mDemoModeTimer, spBC.x, spBC.y, spBC.z,
                        a_this->eyePos.x, a_this->eyePos.y, a_this->eyePos.z); fclose(fp); } }
#endif
        }

        if (i_this->mDemoModeTimer == 90) {
            player->changeDemoParam1(1);
        }

        if (i_this->mDemoModeTimer < 90) {
            cLib_addCalc2(&i_this->mDemoCamFovy, 55.0f, 0.05f, 0.1f);
        } else {
            cLib_addCalc2(&i_this->mDemoCamFovy, 45.0f, 0.1f, 0.5f);
        }

        if (i_this->mDemoModeTimer == 130) {
            i_this->mDemoCamCenterTarget.set(a_this->eyePos.x, 0.0f, a_this->eyePos.z);
            i_this->mDemoCamEyeTarget = i_this->mDemoCamEye;
            i_this->mDemoCamEyeTarget.y += 40.0f;

            cam_spd_set(i_this);
            i_this->mDemoMode = 33;
            i_this->mDemoModeTimer = 0;
        }
        break;
    case 33:
        if (i_this->mDemoModeTimer >= 60) {
            cLib_addCalc2(&i_this->mDemoCamFovy, 55.0f, 0.02f, 0.1f);
        }

        cam_3d_morf(i_this, 0.1f);
        cLib_addCalc2(&i_this->mDemoCamEye.y, 400.0f, 0.05f, i_this->field_0x1cdc * 300.0f);
        cLib_addCalc2(&i_this->field_0x1cdc, 0.02f, 1.0f, 0.001f);

        if (i_this->mDemoModeTimer == 60) {
            fpcM_Search(s_ko2_move, i_this);
        }

        if (i_this->mDemoModeTimer == 230) {
            fpcM_Search(s_ko_move, i_this);
        }

        if (i_this->mDemoModeTimer == 290) {
            sp10 = 1;
            Z2GetAudioMgr()->bgmStart(Z2BGM_GOMA_BTL02, 0, 0);
        }
        break;
    case 40:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

        camera->mCamera.Stop();
        i_this->mDemoMode = 41;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;
        camera->mCamera.SetTrimSize(3);

        i_this->field_0x1cdc = 0.0f;
        player->changeOriginalDemo();

        a_this->current.pos = i_this->field_0x1cec;
        a_this->eyePos = i_this->field_0x1cec;
#if D_ALBW_ARMO_DEATH_DIAG
        { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
            fprintf(fp, "[death] demo40->41 (E_GM dead), boss/eye pos=(%.0f,%.0f,%.0f)\n",
                    i_this->field_0x1cec.x, i_this->field_0x1cec.y, i_this->field_0x1cec.z); fclose(fp); } }
#endif

        i_this->field_0x1ce4 = -25000;
        i_this->field_0x1ce8 = -2500;
        i_this->field_0x1ce0 = 600.0f;
        i_this->field_0x1ce6 = cM_atan2s(-i_this->field_0x1cec.x, -i_this->field_0x1cec.z);

        if (daPy_py_c::checkNowWolf()) {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_METAMORPHOSE_UNK2_e, 0, 0, 0);
            a_this->field_0x567 = 1;
        } else {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNK_23_e, 1, 2, 0);
        }
    case 41:
        if (i_this->mDemoModeTimer == 55 && a_this->field_0x567 != 0) {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNK_23_e, 1, 2, 0);
        }

        cLib_addCalcAngleS2(&i_this->field_0x1ce4, -4000, 0x10, 200);

        cMtx_YrotS(*calc_mtx, i_this->field_0x1ce6);
        spC8.x = 0.0f;
        spC8.y = 0.0f;
        spC8.z = 350.0f;
        MtxPosition(&spC8, &spBC);
        spBC += i_this->field_0x1cec;

        daPy_getPlayerActorClass()->setPlayerPosAndAngle(&spBC, i_this->field_0x1ce6 + 0x8000, 0);

        cMtx_YrotS(*calc_mtx, i_this->field_0x1ce6 + i_this->field_0x1ce4);
        cMtx_XrotM(*calc_mtx, i_this->field_0x1ce8);

        spC8.x = 0.0f;
        spC8.y = 0.0f;
        spC8.z = i_this->field_0x1ce0;
        MtxPosition(&spC8, &i_this->mDemoCamEye);
        i_this->mDemoCamEye += i_this->field_0x1cec;
        i_this->mDemoCamCenter = i_this->field_0x1cec;
        i_this->mDemoCamCenter.y += 30.0f;

        if (i_this->mDemoModeTimer == 160) {
            csXyz angle(0, player->shape_angle.y - 0x4000, 0);
            fopAcM_createItemForBoss(&i_this->field_0x1cec, dItemNo_UTAWA_HEART_e, fopAcM_GetRoomNo(a_this), &angle, NULL, 10.0f, 30.0f, -1);
        }

        if (i_this->mDemoModeTimer == 190) {
            i_this->field_0x1cec.y += 600.0f;
            fopAcM_create(fpcNm_OBJ_YSTONE_e, 0, &i_this->field_0x1cec, fopAcM_GetRoomNo(a_this), NULL, NULL, -1);
            i_this->mDemoMode = 42;
            i_this->mDemoModeTimer = 0;
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[death] demo41->42 (heart+ystone created)\n"); fclose(fp); } }
#endif
        }
        break;
    case 42: {
        if (i_this->mDemoModeTimer == 30) {
            daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNEQUIP_e, 0, 0, 0);
        }

        cLib_addCalcAngleS2(&i_this->field_0x1ce4, 0, 0x10, 40);

        cMtx_YrotS(*calc_mtx, i_this->field_0x1ce6 + i_this->field_0x1ce4);
        cMtx_XrotM(*calc_mtx, i_this->field_0x1ce8);
        spC8.x = 0.0f;
        spC8.y = 0.0f;
        spC8.z = i_this->field_0x1ce0;
        MtxPosition(&spC8, &i_this->mDemoCamEye);
        i_this->mDemoCamEye.x += i_this->field_0x1cec.x;
        i_this->mDemoCamEye.z += i_this->field_0x1cec.z;

        cLib_addCalc2(&i_this->mDemoCamCenter.y, i_this->field_0x1cec.y - 60.0f, 0.05f, 5.0f);

        if (i_this->mDemoModeTimer > 200) {
            cLib_addCalc2(&i_this->field_0x1cec.y, 100.0f, 0.05f, 0.7f);
        }

        obj_ystone_class* ystone = (obj_ystone_class*)fopAcM_SearchByName(fpcNm_OBJ_YSTONE_e);
        if (ystone != NULL) {
            if (i_this->mDemoModeTimer > 200) {
                cLib_addCalc2(&i_this->field_0x1cec.y, 100.0f, 0.05f, 1.0f);
            }

            ystone->setCurrentPos(i_this->field_0x1cec);
        }

        if (i_this->mDemoModeTimer == 335) {
            cXyz pos(0.0f, 0.0f, 0.0f);
            csXyz angle(0, 0, 0);
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[death] demo42 t335: pre createWarpHole\n"); fclose(fp); } }
#endif
            fopAcM_createWarpHole(&pos, &angle, fopAcM_GetRoomNo(a_this), 1, 1, 0xFF);
            dComIfGs_onStageBossEnemy();
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[death] demo42 t335: post warpHole, pre victory\n"); fclose(fp); } }
#endif
#if TARGET_PC
            dAlbwBoss_onArmogohmaVictory();
            dAlbwEnemyRupees_tryGrantFightVictory(fpcNm_B_GM_e);
#endif
#if D_ALBW_ARMO_DEATH_DIAG
            { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[death] demo42 t335: post victory calls (warp armed)\n"); fclose(fp); } }
#endif
        }

        if (i_this->mDemoModeTimer == 341) {
            sp10 = 2;
        }
        break;
    }
    case 50:
        if (!a_this->eventInfo.checkCommandDemoAccrpt()) {
            fopAcM_orderPotentialEvent(a_this, 2, 0xFFFF, 0);
            a_this->eventInfo.onCondition(2);
            return;
        }

        camera->mCamera.Stop();
        i_this->mDemoMode = 51;
        i_this->mDemoModeTimer = 0;
        i_this->mDemoCamFovy = 55.0f;
        
        camera->mCamera.SetTrimSize(3);
        i_this->field_0x1cdc = 0.0f;
    case 51:
        cMtx_YrotS(*calc_mtx, cM_atan2s(-a_this->current.pos.x, -a_this->current.pos.z));
        spC8.x = 300.0f;
        spC8.y = 1200.0f;
        spC8.z = 1300.0f;
        MtxPosition(&spC8, &spB0);

        spB0 += i_this->field_0x1cec;
        spA4 = i_this->field_0x1cec;
        spA4.y += 350.0f;

        if (i_this->mDemoModeTimer == 0) {
            i_this->mDemoCamCenter = spA4;
            i_this->mDemoCamEye = spB0;
        } else {
            cLib_addCalc2(&i_this->mDemoCamCenter.x, spA4.x, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamCenter.z, spA4.z, 0.1f, 5.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.x, spB0.x, 0.1f, 30.0f);
            cLib_addCalc2(&i_this->mDemoCamEye.z, spB0.z, 0.1f, 30.0f);
        }

        if (i_this->mDemoModeTimer == 60) {
            i_this->mDemoMode = 41;
            i_this->mDemoModeTimer = 40;
            i_this->mDemoCamFovy = 55.0f;
            camera->mCamera.SetTrimSize(3);
            i_this->field_0x1cdc = 0.0f;

            player->changeOriginalDemo();
        
            a_this->current.pos = i_this->field_0x1cec;
            a_this->eyePos = i_this->field_0x1cec;

            i_this->field_0x1ce4 = -8000;
            i_this->field_0x1ce8 = -2500;
            i_this->field_0x1ce0 = 600.0f;
            i_this->field_0x1ce6 = cM_atan2s(-i_this->field_0x1cec.x, -i_this->field_0x1cec.z);

            if (daPy_py_c::checkNowWolf()) {
                daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_METAMORPHOSE_UNK2_e, 0, 0, 0);
                a_this->field_0x567 = 1;
            } else {
                daPy_getPlayerActorClass()->changeDemoMode(daPy_demo_c::DEMO_UNK_23_e, 1, 2, 0);
            }
        }
        break;
    }

    if (i_this->mDemoMode != 0 && i_this->mDemoMode < 10) {
        if (dComIfGp_getEvent()->checkSkipEdge()) {
            i_this->mMode = 4;
            i_this->mTimers[2] = cM_rndF(100.0f) + 200.0f;
            sp10 = 1;
            i_this->field_0x1cfc = 2;
            i_this->field_0x6f5 = 0;

            Z2GetAudioMgr()->subBgmStop();
            Z2GetAudioMgr()->bgmStart(Z2BGM_GOMA_BTL01, 0, 0);
            a_this->field_0x566 = 1;
        }
    }

    if (sp10 != 0) {
        if (sp10 == 1) {
            cMtx_YrotS(*calc_mtx, player->shape_angle.y);
            spC8.x = 0.0f;
            spC8.y = 100.0f;
            spC8.z = -250.0f;
            MtxPosition(&spC8, &i_this->mDemoCamEye);
            i_this->mDemoCamEye += player->current.pos;
            i_this->mDemoCamCenter = player->current.pos;
            i_this->mDemoCamCenter.y += 120.0f;
        }

        camera->mCamera.Reset(i_this->mDemoCamCenter, i_this->mDemoCamEye);
        camera->mCamera.Start();
        camera->mCamera.SetTrimSize(0);
        dComIfGp_event_reset();

        daPy_getPlayerActorClass()->cancelOriginalDemo();
        i_this->mDemoMode = 0;
    }

    if (i_this->mDemoMode > 0) {
        cXyz center;
        cXyz eye;
        center = i_this->mDemoCamCenter;
        eye = i_this->mDemoCamEye;

        camera->mCamera.Set(center, eye, i_this->mDemoCamFovy, 0);
        i_this->mDemoModeTimer++;
    }
}

static int daB_GM_Execute(b_gm_class* i_this) {
    if (cDmrNowMidnaTalk()) {
        return 1;
    }

    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;

#if TARGET_PC
    dAlbwBoss_armogohmaEnsureInitialized(a_this);
#endif
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    cXyz spD4, spC8;

    #if DEBUG
    if (mDoCPd_c::getTrigA(2) && i_this->mDemoMode == 0 && !i_this->mIsDisappear) {
        i_this->mAction = 0xB;
        i_this->mMode = 0x14;
        i_this->mDemoMode = 0x1E;
        a_this->current.pos.y = VREG_F(18);
        a_this->speedF = 0.0f;
    }
    #endif

    for (int i = 0; i < 4; i++) {
        mDoMtx_stack_c::transS(target_pos[i].x, target_pos[i].y, target_pos[i].z);

        f32 target_size = 1.0f;
        if (!i_this->mIsDisappear && a_this->current.pos.y > 1000.0f) {
            spD4.x = a_this->eyePos.x - target_pos[i].x;
            spD4.z = a_this->eyePos.z - target_pos[i].z;
            if (JMAFastSqrt((spD4.x * spD4.x) + (spD4.z * spD4.z)) < NREG_F(18) + 500.0f) {
                target_size = 0.0f;
            }
        }

        if (target_size > 0.1f) {
            if (i_this->field_0x1cfc == i + 3) {
                cLib_addCalc0(&i_this->mKankyoBlend, 1.0f, 0.1f);
            }
        } else if (i_this->field_0x6c0 < 0.1f) {
            i_this->field_0x1cfc = i + 3;
            cLib_addCalc2(&i_this->mKankyoBlend, 1.0f, 1.0f, 0.1f);
        }

        if (i == 0 && i_this->field_0x6f5 != 0) {
            target_size = 0.0f;
        }

        cLib_addCalc2(&i_this->mSpotLightSize[i], target_size, 1.0f, 0.1f);
        mDoMtx_stack_c::scaleM(i_this->mSpotLightSize[i], 1.0f, i_this->mSpotLightSize[i]);
        mDoMtx_stack_c::YrotM(i * 0x3100);
        i_this->mpSpotLightModel[i]->setBaseTRMtx(mDoMtx_stack_c::get());
    }

    i_this->mpSpotLightBtk->play();

    if (i_this->mIsDisappear) {
        if (i_this->mDemoMode == 0) {
            daE_GM_c* kogoma = (daE_GM_c*)fpcM_Search(s_ko2_get, a_this);
            if (kogoma != NULL) {
                fopAc_ac_c* ko_actor = kogoma;
                i_this->field_0x1cec = ko_actor->current.pos;
                i_this->field_0x1cf8 = kogoma->CheckDeath();

                if (i_this->field_0x1cf8 != 0) {
                    if (i_this->field_0x1cf8 == 1) {
                        i_this->mDemoMode = 40;
                    } else {
                        i_this->mDemoMode = 50;
                    }
                    Z2GetAudioMgr()->bgmStop(0x1E, 0);
                    Z2GetAudioMgr()->bgmStreamPrepare(0x200004F);
                    Z2GetAudioMgr()->bgmStreamPlay();
                    a_this->field_0x566 = 0;
                }
            }
        }

        demo_camera(i_this);
        return 1;
    }

    daPy_getPlayerActorClass()->onNeckSearchWide();
    i_this->mCounter++;

    for (int i = 0; i < 4; i++) {
        if (i_this->mTimers[i] != 0) {
            i_this->mTimers[i]--;
        }
    }

    if (i_this->mInvincibilityTimer != 0) {
        i_this->mInvincibilityTimer--;
    }

    action(i_this);

    mDoMtx_stack_c::transS(a_this->current.pos.x, a_this->current.pos.y, a_this->current.pos.z);
    mDoMtx_stack_c::YrotM(a_this->shape_angle.y);
    mDoMtx_stack_c::ZrotM(a_this->shape_angle.z);
    mDoMtx_stack_c::XrotM(a_this->shape_angle.x);
    mDoMtx_stack_c::scaleM(l_HIO.model_size, l_HIO.model_size, l_HIO.model_size);
    mDoMtx_stack_c::transM(0.0f, 0.0f, KREG_F(8) + 200.0f);

    J3DModel* model = i_this->mpModelMorf->getModel();
    model->setBaseTRMtx(mDoMtx_stack_c::get());

    i_this->mpModelMorf->play(0, dComIfGp_getReverb(fopAcM_GetRoomNo(a_this)));
    i_this->mBeamSound.framework(0, dComIfGp_getReverb(fopAcM_GetRoomNo(a_this)));

    anm_se_set(i_this);

    cLib_addCalc2(&i_this->mZoomBtkFrame, i_this->field_0x1ae0, 1.0f, 1.0f);
    i_this->mpZoomBtk->setFrame(i_this->mZoomBtkFrame);
    i_this->mpModelMorf->modelCalc();

    foot_IK_main(i_this);

    f32 sph_base_size = l_HIO.check_size * l_HIO.model_size;
    MTXCopy(model->getAnmMtx(0x15), *calc_mtx);
    MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
    spD4.set(JREG_F(8) + -20.0f, JREG_F(9) + 0.0f, JREG_F(10) + 0.0f);
    MtxPosition(&spD4, &spC8);

    if (i_this->mInvincibilityTimer != 0 || i_this->field_0x1ad6 == 0) {
        spC8.y += 20000.0f;
    }

    i_this->mCoreSph.SetC(spC8);
    // ============================================
    // NEW CODE — ALBW Port (phase-3 eye: bigger, any-damage, dash-only contact hurt)
    // In the floor chase the eye IS the drain target. (1) Widen it. (2) Accept ANY
    // weapon: vanilla's Tg mask is a single narrow bit (the rod/bomb type), so Link's
    // sword never registered on the eye -> the pool never drained (the "HP won't go
    // down" bug). (3) While the DASH anim plays the eye also HURTS Link -> distance
    // fighting is safer, but you can risk a close sword and trade a hit; the VULN
    // stagger drops the At bit so up-close hits there are free. The sphere is already
    // hidden while the eye is shut, so the contact only bites during the open window.
    // All runtime-gated on s_gmPhase3Active -> the vanilla ceiling/wall fight is untouched.
    // ============================================
    f32 core_r = 160.0f;
#if TARGET_PC && D_ALBW_ARMO_REVEAL
    if (s_gmPhase3Active) {
        core_r = 200.0f;
        i_this->mCoreSph.SetTgType(0xd8fbfdff);  // any player damage, not just the rod/bomb type
        if (i_this->mAnmID == ANM_GOMA_DASH) {
            i_this->mCoreSph.SetAtType(AT_TYPE_CSTATUE_SWING);
            i_this->mCoreSph.SetAtAtp(16);        // actually DAMAGE Link (atp default 0 = harmless
                                                  // reaction only); ~4 hearts. Tunable.
            i_this->mCoreSph.OnAtSetBit();        // eye hurts Link while dashing
        } else {
            i_this->mCoreSph.OffAtSetBit();       // VULN stagger = free hits
        }
    }
#endif
    i_this->mCoreSph.SetR((JREG_F(11) + core_r) * sph_base_size);
    dComIfG_Ccsp()->Set(&i_this->mCoreSph);

    spD4.set(JREG_F(12) + 50.0f, JREG_F(13) + -20.0f, JREG_F(14) + 0.0f);
    MtxPosition(&spD4, &a_this->eyePos);
    a_this->attention_info.position = a_this->eyePos;
    a_this->attention_info.position.y += JREG_F(15) + 70.0f;

    if (i_this->field_0x1ad6 == 0) {
        MTXCopy(model->getAnmMtx(6), *calc_mtx);
        MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
        spD4.set(JREG_F(0) + 170.0f, JREG_F(1) + 0.0f, JREG_F(2) + 0.0f);
        MtxPosition(&spD4, &spC8);

        if (i_this->mInvincibilityTimer != 0) {
            spC8.y += 20000.0f;
        }

        i_this->mBodySph[0].SetC(spC8);
        i_this->mBodySph[0].SetR((JREG_F(3) + 190.0f) * sph_base_size);
    }

    MTXCopy(model->getAnmMtx(2), *calc_mtx);
    MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
    spD4.set(JREG_F(4) + 100.0f, JREG_F(5) + 0.0f, JREG_F(6) + 0.0f);
    MtxPosition(&spD4, &spC8);

    if (i_this->mInvincibilityTimer != 0) {
        spC8.y += 20000.0f;
    }
    i_this->mBodySph[1].SetC(spC8);
    i_this->mBodySph[1].SetR((JREG_F(7) + 180.0f) * sph_base_size);

    for (int i = 0; i < 2; i++) {
        dComIfG_Ccsp()->Set(&i_this->mBodySph[i]);
    }

    for (int i = 0; i < 8; i++) {
        MTXCopy(model->getAnmMtx(top_j[i]), *calc_mtx);
        MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
        spD4.set(AREG_F(10) + 150.0f, AREG_F(11) + 0.0f, AREG_F(12) + 0.0f);
        MtxPosition(&spD4, &spC8);

        // ============================================
        // NEW CODE — ALBW Port (phase-3 leg contact-hurt)
        // The legs (foot spheres, AT while dashing) are what hurt Link when the boss
        // runs into him. Widen their radius ONLY in the phase-3 floor chase so the
        // vanilla ceiling/wall fight is untouched. Tunable; verify with the debug
        // collision-sphere overlay.
        // ============================================
        f32 foot_r = 50.0f;
#if TARGET_PC && D_ALBW_ARMO_REVEAL
        if (s_gmPhase3Active) {
            foot_r = 90.0f;
        }
#endif
        i_this->mFootSph[i].SetR((TREG_F(13) + foot_r) * sph_base_size);
        i_this->mFootSph[i].SetC(spC8);

        if (i_this->mAnmID == ANM_GOMA_DASH && a_this->current.angle.x < 0x1000 && a_this->current.angle.x > -0x1000) {
            i_this->mFootSph[i].OnAtSetBit();
#if TARGET_PC && D_ALBW_ARMO_REVEAL
            if (s_gmPhase3Active) {
                // Legs actually DAMAGE Link in the floor chase (atp default 0 = harmless
                // reaction only, per the Hero's Shade At-power finding); ~2 hearts,
                // less than the eye so leg brushes sting but the eye is the real risk.
                i_this->mFootSph[i].SetAtAtp(8);
            }
#endif
        } else {
            i_this->mFootSph[i].OffAtSetBit();
        }

        dComIfG_Ccsp()->Set(&i_this->mFootSph[i]);
    }

    cXyz spBC(0.0f, 0.0f, 0.0f);
    if (i_this->field_0x1aac == 0) {
        spBC.y = -20000.0f;
    }

    MTXCopy(model->getAnmMtx(0x3B), *calc_mtx);
    MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
    spD4.set(XREG_F(0) + 80.0f, XREG_F(1), XREG_F(2));
    MtxPosition(&spD4, &spC8);
    i_this->mHandSph[0].SetC(spC8 + spBC);
    i_this->mHandSph[0].SetR((XREG_F(3) + 80.0f) * sph_base_size);

    MTXCopy(model->getAnmMtx(0x40), *calc_mtx);
    MtxScale(l_HIO.check_size, l_HIO.check_size, l_HIO.check_size, 1);
    spD4.set(XREG_F(0), XREG_F(1), XREG_F(2));
    MtxPosition(&spD4, &spC8);
    i_this->mHandSph[1].SetC(spC8 + spBC);
    i_this->mHandSph[1].SetR((XREG_F(3) + 60.0f) * sph_base_size);

    dComIfG_Ccsp()->Set(&i_this->mHandSph[0]);
    dComIfG_Ccsp()->Set(&i_this->mHandSph[1]);

    spC8 = player->eyePos - a_this->eyePos;

    cMtx_XrotS(*calc_mtx, -a_this->shape_angle.x);
    cMtx_YrotM(*calc_mtx, -a_this->shape_angle.y);
    MtxPosition(&spC8, &spD4);

    s16 sp14, sp12;
    sp12 = -cM_atan2s(spD4.z, spD4.y);
    sp14 = (s16)cM_atan2s(spD4.x, spD4.y);
    if (sp12 > 0x1400) {
        sp12 = 0x1400;
    } else if (sp12 < -0x1400) {
        sp12 = -0x1400;
    }

    if (sp14 > 0x1400) {
        sp14 = 0x1400;
    } else if (sp14 < -0x1400) {
        sp14 = -0x1400;
    }

    s16 sp10, spE;
    if (i_this->field_0x1ade != 0) {
        i_this->field_0x1ade--;

        spE = 120.0f * (i_this->field_0x1ade * cM_ssin(i_this->mCounter * 1200));
        sp10 = 120.0f * (i_this->field_0x1ade * cM_ssin(i_this->mCounter * 2000));
    } else {
        spE = 0;
        sp10 = 0;
    }

    cLib_addCalcAngleS2(&i_this->field_0x1adc, sp12 + spE, 4, 0x1000);
    cLib_addCalcAngleS2(&i_this->field_0x1ada, sp14 + sp10, 4, 0x1000);

    if (i_this->field_0x6c0 > 0.1f) {
        dBgS_LinChk line_chk;
        MTXCopy(model->getAnmMtx(0x15), *calc_mtx);
        spD4.set(XREG_F(8) + 120.0f, XREG_F(9), XREG_F(10));
        MtxPosition(&spD4, &spC8);

#if TARGET_PC && D_ALBW_ARMO_REVEAL
        // Lift the beam source so the low mouth eye still arcs the floor-sweep across
        // the arena (see kAlbwArmoBeamOriginRaise). Phase-3 only.
        if (s_gmPhase3Active) {
            spC8.y += kAlbwArmoBeamOriginRaise;
        }
#endif

#if TARGET_PC && D_ALBW_ARMO_REVEAL
        if (s_gmPhase3Active) {
            // Fire straight forward from the eye onto the floor; the body's in-place
            // rotation (the P3_LASER sweep) arcs it across the arena. No per-frame
            // player tracking here -- the sweep itself is the threat. Recomputed each
            // frame from the current facing so it follows the rotation.
            cMtx_YrotS(*calc_mtx, a_this->shape_angle.y);
            spD4.set(0.0f, 0.0f, kAlbwArmoBeamForward);
            MtxPosition(&spD4, &i_this->field_0x6cc);
            i_this->field_0x6cc += spC8;
            // Keep the target at eye height (do NOT drop it to the floor) so the beam
            // fires in a straight HORIZONTAL line; the body's rotation sweeps it like a
            // wall. Damage is relocated to where the beam passes nearest Link (below).
            i_this->field_0x6cc.y = spC8.y;
            i_this->field_0x6f4 = 1;
        } else
#endif
        {
            if (i_this->field_0x6f4 == 0) {
                i_this->field_0x6cc = spC8;
                i_this->field_0x6cc.y = 0.0f;
                i_this->field_0x6c4 = 0.0f;
            }

            cXyz spB0;
            cLib_addCalc2(&i_this->field_0x6c4, AREG_F(17) + 40.0f, 1.0f, 1.0f);
            spD4 = player->current.pos - i_this->field_0x6cc;

            if (i_this->field_0x6f4 == 0) {
                i_this->field_0x6c8 = cM_atan2s(spD4.x, spD4.z);
            } else {
                f32 var_f30 = (JMAFastSqrt((spD4.x * spD4.x) + (spD4.z * spD4.z)) - 400.0f);
                var_f30 *= 10.0f;
                if (var_f30 < 0.0f) {
                    var_f30 = 0.0f;
                }

                if (var_f30 > 6000.0f) {
                    var_f30 = 6000.0f;
                }

                s16 spC = var_f30 * cM_ssin(i_this->mCounter * 1000);
                cLib_addCalcAngleS2(&i_this->field_0x6c8, spC + cM_atan2s(spD4.x, spD4.z), 8, AREG_S(7) + 0x400);
            }

            i_this->field_0x6f4 = 1;

            cMtx_YrotS(*calc_mtx, i_this->field_0x6c8);
            spD4.x = 0.0f;
            spD4.y = 0.0f;
            spD4.z = i_this->field_0x6c4;
            MtxPosition(&spD4, &spB0);
            i_this->field_0x6cc += spB0;
        }

        spD4 = i_this->field_0x6cc - spC8;
#if TARGET_PC && D_ALBW_ARMO_REVEAL
        // The +80 aim-lift is tuned for the high dorsal eye; from the low mouth eye it
        // makes the beam too shallow to reach the floor within its length -> no ground
        // cross -> no mBeamSph -> the beam does no damage. Aim straight at the floor
        // target in phase 3 so it crosses and the collision lands.
        if (!s_gmPhase3Active)
#endif
        {
            spD4.y += 80.0f;
        }

        s16 spA = -cM_atan2s(spD4.y, spD4.z);
        s16 sp8 = (s16)cM_atan2s(spD4.x, JMAFastSqrt((spD4.y * spD4.y) + (spD4.z * spD4.z)));

        mDoMtx_stack_c::transS(spC8.x, spC8.y, spC8.z);
        mDoMtx_stack_c::XrotM(spA);
        mDoMtx_stack_c::YrotM(sp8);

        f32 sp24 = XREG_F(12) + 5.0f;
        mDoMtx_stack_c::scaleM(XREG_F(11) + 1.0f, XREG_F(11) + 1.0f, sp24 * i_this->field_0x6c0);

        i_this->mpBeamModelMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
        mDoMtx_stack_c::transM(0.0f, 0.0f, i_this->field_0x6c0 * (XREG_F(6) + 2400.0f));
        mDoMtx_stack_c::multVecZero(&spD4);
    
        line_chk.Set(&spC8, &spD4, i_this);

        if (dComIfG_Bgsp().LineCross(&line_chk)) {
            spD4 = line_chk.GetCross();
            i_this->field_0x6d8 = spD4;

            i_this->mBeamSound.startLevelSound(Z2SE_EN_GO_BEAM, 0, -1);

            cM3dGPla spE0;
            csXyz sp48;
            dComIfG_Bgsp().GetTriPla(line_chk, &spE0);

            sp48.y = (s16)cM_atan2s(spE0.mNormal.x, spE0.mNormal.z);
            sp48.x = -cM_atan2s(spE0.mNormal.y, JMAFastSqrt((spE0.mNormal.x * spE0.mNormal.x) + (spE0.mNormal.z * spE0.mNormal.z)));
            sp48.x += 0x4000;
            sp48.z = 0;

            for (int i = 0; i < 4; i++) {
                static u16 name[] = {0x8AF2, 0x8AF3, 0x8AF4, 0x8AF5};
                i_this->mBeamEmitterIDs[i] = dComIfGp_particle_set(i_this->mBeamEmitterIDs[i], name[i], &spD4, &sp48, NULL);
            }

            f32 sp1C = 0.013f * (i_this->mCounter & 3);
            spD4 = spD4 + ((spC8 - spD4) * (sp1C));

#if TARGET_PC && D_ALBW_ARMO_REVEAL
            // In phase 3 the beam is horizontal, so this cross is the far WALL -- keep
            // the impact particles/sound there, but do NOT put the point-sphere on the
            // wall; phase 3 uses a capsule spanning the whole beam instead (below).
            if (!s_gmPhase3Active)
#endif
            {
                i_this->mBeamSph.SetC(spD4);
                dComIfG_Ccsp()->Set(&i_this->mBeamSph);
            }
        }

#if TARGET_PC && D_ALBW_ARMO_REVEAL
        if (s_gmPhase3Active) {
            // Dark-Souls-style capsule (pill) hitbox: the WHOLE beam line is the hurt
            // zone, not a point. Span the eye (spC8) -> beam-end (spD4) line with a
            // radius matched to the beam's visual thickness, so Link is hit anywhere the
            // pill crosses him. One static capsule (only one beam is ever active).
            static dCcD_Cps s_beamCps;
            static bool s_beamCpsReady = false;
            if (!s_beamCpsReady) {
                static const dCcD_SrcCps beam_cps_src = {
                    {
                        {0x0, {{AT_TYPE_CSTATUE_SWING, 0x4, 0x1f}, {0x0, 0x0}, 0x0}}, // mObj
                        {dCcD_SE_NONE, 0x0, 0x1, 0x0, 0x0}, // mGObjAt
                        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x2}, // mGObjTg
                        {0x0}, // mGObjCo
                    }, // mObjInf
                    {
                        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 20.0f}, // mCps (radius re-set per frame)
                    } // mCpsAttr
                };
                s_beamCps.Set(beam_cps_src);
                s_beamCps.SetAtMtrl(dCcD_MTRL_FIRE);
                s_beamCpsReady = true;
            }
            s_beamCps.SetStts(&i_this->mCcStts);
            static_cast<cM3dGCps*>(&s_beamCps)->Set(spC8, spD4, kAlbwArmoBeamRadius);
            s_beamCps.CalcAtVec();
            dComIfG_Ccsp()->Set(&s_beamCps);

            // Diagnostic: draw ONLY this capsule in light blue whenever the collision
            // viewer is open, so it's unmistakable and independent of the framework's
            // red AT wireframes. Explicit per-collider Draw() -- affects nothing else, and
            // it's gated on the viewer so it never shows in normal play.
            {
                const auto& cv = dusk::getTransientSettings().collisionView;
                if (cv.enableAtView || cv.enableTgView || cv.enableCoView) {
                    GXColor beamDbg = {0x60, 0xC0, 0xFF, 0xC8}; // light blue
                    s_beamCps.Draw(beamDbg);
                }
            }
        }
#endif

        i_this->mpBeamBtk->play();
        i_this->mpBeamModelMorf->play(NULL, 0, 0);
        i_this->mpBeamModelMorf->modelCalc();
    }

    if (i_this->field_0x1cfc == 7 || i_this->field_0x6c0 > 0.1f) {
        i_this->field_0x1cfc = 7;
        cLib_addCalc2(&i_this->mKankyoBlend, 1.0f - i_this->field_0x6c0, 1.0f, 0.1f);
    }

    demo_camera(i_this);

    bool lidOpen = (i_this->field_0x1ad6 != 0);
#if TARGET_PC && D_ALBW_ARMO_REVEAL
    // Laser exception: while firing, keep the eye visually OPEN (reads as the eye
    // shooting the beam) even though the core sphere stays gated/invulnerable
    // (P3_LASER never raises field_0x1ad6). "Closed lid = invulnerable" stays true
    // everywhere else. The eye-retract below keys off the lid, so an open lid here
    // also keeps the eye extended (out) through the whole laser.
    if (s_gmPhase3Active && i_this->mMode == P3_LASER) {
        lidOpen = true;
    }
#endif
    if (lidOpen) {
        cLib_addCalcAngleS2(&i_this->field_0x1ad8, 6000, 4, 800);
    } else {
        s16 shutTarget = BREG_S(7) + -3900;
#if TARGET_PC && D_ALBW_ARMO_REVEAL
        // The revealed model's eyelids were stretched to enclose the (relocated) eye;
        // they need a deeper close than the vanilla lids to fully cover it.
        if (s_gmRevealActive) {
            shutTarget = BREG_S(7) + -8000;
        }
#endif
        cLib_addCalcAngleS2(&i_this->field_0x1ad8, shutTarget, 1, 800);
    }

    f32 target_blend = 1.0f;
    f32 blend_step = 0.1f;
    if (i_this->field_0x1cfc == 0) {
        dKy_custom_colset(0, 0, i_this->mKankyoBlend);
    } else if (i_this->field_0x1cfc == 1) {
        dKy_custom_colset(0, 5, i_this->mKankyoBlend);
    } else if (i_this->field_0x1cfc == 2) {
        dKy_custom_colset(1, 5, i_this->mKankyoBlend);
        target_blend = 0.0f;
        blend_step = 0.1f;
    } else if (i_this->field_0x1cfc == 3) {
        dKy_custom_colset(1, 5, i_this->mKankyoBlend);
        i_this->field_0x1cfa = 5;
        target_blend = -1.0f;
    } else if (i_this->field_0x1cfc == 4) {
        dKy_custom_colset(1, 4, i_this->mKankyoBlend);
        i_this->field_0x1cfa = 4;
        target_blend = -1.0f;
    } else if (i_this->field_0x1cfc == 5) {
        dKy_custom_colset(1, 2, i_this->mKankyoBlend);
        i_this->field_0x1cfa = 2;
        target_blend = -1.0f;
    } else if (i_this->field_0x1cfc == 6) {
        dKy_custom_colset(1, 3, i_this->mKankyoBlend);
        i_this->field_0x1cfa = 3;
        target_blend = -1.0f;
    } else if (i_this->field_0x1cfc == 7) {
        dKy_custom_colset(6, i_this->field_0x1cfa, i_this->mKankyoBlend);
        target_blend = -1.0f;
    }

    if (target_blend >= 0.0f) {
        cLib_addCalc2(&i_this->mKankyoBlend, target_blend, 1.0f, blend_step);
    }

    return 1;
}

static int daB_GM_IsDelete(b_gm_class* i_this) {
    return 1;
}

static int daB_GM_Delete(b_gm_class* i_this) {
    fopAc_ac_c* a_this = (fopAc_ac_c*)i_this;
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) {
        fprintf(fp, "[death] daB_GM_Delete entry: heap=%p morf=%p reveal=%p active=%d\n",
                (void*)a_this->heap, (void*)i_this->mpModelMorf, (void*)s_gmRevealMorf,
                (int)s_gmRevealActive); fclose(fp); } }
#endif

    dComIfG_resDelete(&i_this->mPhase, "B_gm");

    if (i_this->mInitHIO) {
        hio_set = false;
    }

    if (a_this->heap != NULL) {
        i_this->mpModelMorf->stopZelAnime();
        i_this->mBeamSound.deleteObject();
    }
#if D_ALBW_ARMO_DEATH_DIAG
    { FILE* fp = b_gm_deathDiagOpen(); if (fp) { fprintf(fp, "[death] daB_GM_Delete done\n"); fclose(fp); } }
#endif

    return 1;
}

static int useHeapInit(fopAc_ac_c* i_this) {
    b_gm_class* a_this = (b_gm_class*)i_this;

    // ============================================
    // NEW CODE — ALBW Port
    // The fight uses the VANILLA GOMA model (arc) for phases 1-2. The custom
    // "revealed" model is built separately below (Layer-B loose BMD) and swapped in
    // after the 2nd statue hit — see b_gm_activateReveal(). (The prior always-on
    // try_load replacement is superseded by the swap, so vanilla players and
    // refinement-without-the-model players get the untouched arc model.)
    // ============================================
    J3DModelData* gomaModelData = (J3DModelData*)dComIfG_getObjectRes("B_gm", 0x25);
    // ============================================
    // NEW CODE ENDS HERE
    // ============================================
    a_this->mpModelMorf = JKR_NEW mDoExt_McaMorfSO(gomaModelData, NULL, NULL, (J3DAnmTransform*)dComIfG_getObjectRes("B_gm", 30), 2, 0.6f, 0, -1, &a_this->mCreatureSound, 0, 0x11000284);
    if (a_this->mpModelMorf == NULL || a_this->mpModelMorf->getModel() == NULL) {
        return 0;
    }

    J3DModel* model = a_this->mpModelMorf->getModel();
    model->setUserArea((uintptr_t)a_this);

    for (u16 i = 0; i < model->getModelData()->getJointNum(); i++) {
        model->getModelData()->getJointNodePointer(i)->setCallBack(nodeCallBack);
    }

    a_this->mpZoomBtk = JKR_NEW mDoExt_btkAnm();
    if (a_this->mpZoomBtk == NULL) {
        return 0;
    }

    J3DAnmTextureSRTKey* temp_r28_2 = (J3DAnmTextureSRTKey*)dComIfG_getObjectRes("B_gm", 0x28);
    if (!a_this->mpZoomBtk->init(a_this->mpModelMorf->getModel()->getModelData(), temp_r28_2, 1, 0, 1.0f, 0, -1)) {
        return 0;
    }

    a_this->mpBeamModelMorf = JKR_NEW mDoExt_McaMorf((J3DModelData*)dComIfG_getObjectRes("B_gm", 0x21), NULL, NULL, (J3DAnmTransform*)dComIfG_getObjectRes("B_gm", 7), 2, 1.0f, 0, -1, 1, NULL, 0x80000, 0x11000284);
    if (a_this->mpBeamModelMorf == NULL || a_this->mpBeamModelMorf->getModel() == NULL) {
        return 0;
    }

    a_this->mpBeamBtk = JKR_NEW mDoExt_btkAnm();
    if (a_this->mpBeamBtk == NULL) {
        return 0;
    }

    if (!a_this->mpBeamBtk->init(a_this->mpBeamModelMorf->getModel()->getModelData(), (J3DAnmTextureSRTKey*)dComIfG_getObjectRes("B_gm", 0x29), 1, 2, 1.0f, 0, -1)) {
        return 0;
    }

    J3DModelData* modelData;
    for (int i = 0; i < 4; i++) {
        modelData = (J3DModelData*)dComIfG_getObjectRes("B_gm", 0x22);
        JUT_ASSERT(0xCE9, modelData != NULL);
        a_this->mpSpotLightModel[i] = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000284);
        if (a_this->mpSpotLightModel[i] == NULL) {
            return 0;
        }

        a_this->mSpotLightSize[i] = 1.0f;
    }

    a_this->mpSpotLightBtk = JKR_NEW mDoExt_btkAnm();
    if (a_this->mpSpotLightBtk == NULL) {
        return 0;
    }

    if (!a_this->mpSpotLightBtk->init(modelData, (J3DAnmTextureSRTKey*)dComIfG_getObjectRes("B_gm", 0x2A), 1, 2, 1.0f, 0, -1)) {
        return 0;
    }

#if TARGET_PC && D_ALBW_ARMO_REVEAL
    // ============================================
    // NEW CODE — ALBW Port
    // Build the phase-3 reveal model alongside the vanilla one (resident — no
    // mid-fight allocation), swapped in by b_gm_activateReveal() after the 2nd statue
    // hit. Built LAST so a heap-full reveal fails gracefully (s_gmRevealMorf stays
    // NULL -> vanilla fight) without breaking the vanilla spawn above. Only when Boss
    // Refinement is ON and the loose override is present (try_load is NULL if the
    // Custom Models folder is disabled or B_gm_37.bmd is missing). The loose BMD is
    // cached in custom_assets so respawns reuse it (no leak).
    // ============================================
    s_gmRevealMorf = NULL;
    if (dAlbwBossRefinement_isEnabled()) {
        J3DModelData* revealData = dusk::custom_assets::try_load("B_gm", 0x25);
        if (revealData != NULL) {
            // Bind the same mCreatureSound as the vanilla morf so daB_GM_Delete's
            // stopZelAnime() is safe after the swap (only the active morf drives it).
            mDoExt_McaMorfSO* rm = JKR_NEW mDoExt_McaMorfSO(
                revealData, NULL, NULL, (J3DAnmTransform*)dComIfG_getObjectRes("B_gm", 30),
                2, 0.6f, 0, -1, &a_this->mCreatureSound, 0, 0x11000284);
            if (rm != NULL && rm->getModel() != NULL) {
                J3DModel* rmodel = rm->getModel();
                rmodel->setUserArea((uintptr_t)a_this);
                for (u16 j = 0; j < rmodel->getModelData()->getJointNum(); j++) {
                    rmodel->getModelData()->getJointNodePointer(j)->setCallBack(nodeCallBack);
                }
                s_gmRevealMorf = rm;
            }
            // Build failed (e.g. heap full): stay vanilla — do NOT fail the spawn.
        }
    }
    // ============================================
#endif

    return 1;
}

static int daB_GM_Create(fopAc_ac_c* i_this) {
    b_gm_class* a_this = (b_gm_class*)i_this;
    fopAcM_ct(a_this, b_gm_class);

#if TARGET_PC && D_ALBW_ARMO_REVEAL
    // New spawn: clear reveal state. The morf is (re)built in useHeapInit; the active
    // flag must not persist across a die-and-retry. Any prior morf lived in the
    // previous (already-freed) solid heap, so this NULL prevents a dangling pointer.
    s_gmRevealMorf = NULL;
    s_gmRevealActive = false;
    s_gmRevealDrawLog = 0;
    s_gmPhase3Active = false;
    s_gmPhase3EyeOpen = false;
    s_gmPhase3HitCount = 0;
#endif

    int phase_state = dComIfG_resLoad(&a_this->mPhase, "B_gm");
    if (phase_state == cPhs_COMPLEATE_e) {
        OS_REPORT("B_gm PARAM %x\n", fopAcM_GetParam(i_this));
        OS_REPORT("B_gm//////////////B_GM SET 1 !!\n");

        // Heap 0x8C00 -> 0xC800: room for the second (reveal) model instance built in
        // useHeapInit when the phase-3 reveal is active. (+0x4000 headroom; the reveal
        // build is null-safe so an over-tight heap degrades to the vanilla fight.)
        if (!fopAcM_entrySolidHeap(i_this, useHeapInit, 0xC800)) {
            OS_REPORT("//////////////B_GM SET NON !!\n");
            return cPhs_ERROR_e;
        }

        OS_REPORT("//////////////B_GM SET 2 !!\n");
        if (!hio_set) {
            a_this->mInitHIO = true;
            hio_set = true;
            l_HIO.field_0x4 = -1;
        }

        if (dComIfGs_isStageBossEnemy()) {
            a_this->mIsDisappear = true;
            cXyz pos(0.0f, 0.0f, 0.0f);
            csXyz angle(0, 0, 0);
            fopAcM_createWarpHole(&pos, &angle, fopAcM_GetRoomNo(i_this), 0, 0, 0xFF);

            if (!dComIfGs_isStageLife()) {
                cXyz size(1.0f, 1.0f, 1.0f);
                pos.set(400.0f, 0.0f, 0.0f);
                fopAcM_createItemForBoss(&pos, dItemNo_UTAWA_HEART_e, fopAcM_GetRoomNo(i_this), &i_this->shape_angle, &size, 0.0f, 0.0f, -1);
            }

            return phase_state;
        }

        fopAcM_SetMtx(i_this, a_this->mpModelMorf->getModel()->getBaseTRMtx());

        a_this->mCcStts.Init(0xFF, 0, i_this);

        static dCcD_SrcSph body_sph_src = {
            {
                {0x0, {{0x0, 0x0, 0xd}, {0xd8fbfdff, 0x43}, 0x75}}, // mObj
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0}, // mGObjAt
                {dCcD_SE_METAL, 0x2, 0x0, 0x0, 0x3}, // mGObjTg
                {0x0}, // mGObjCo
            }, // mObjInf
            {
                {{0.0f, 0.0f, 0.0f}, 40.0f} // mSph
            } // mSphAttr
        };
    
        static dCcD_SrcSph core_sph_src = {
            {
                {0x0, {{0x0, 0x0, 0xd}, {0x2000, 0x43}, 0x75}}, // mObj
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0}, // mGObjAt
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x2}, // mGObjTg
                {0x0}, // mGObjCo
            }, // mObjInf
            {
                {{0.0f, 0.0f, 0.0f}, 40.0f} // mSph
            } // mSphAttr
        };
    
        static dCcD_SrcSph hand_sph_src = {
            {
                {0x0, {{AT_TYPE_CSTATUE_SWING, 0x2, 0xd}, {0xd8fbfdff, 0x43}, 0x0}}, // mObj
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0}, // mGObjAt
                {dCcD_SE_METAL, 0x2, 0x0, 0x0, 0x3}, // mGObjTg
                {0x0}, // mGObjCo
            }, // mObjInf
            {
                {{0.0f, 0.0f, 0.0f}, 30.0f} // mSph
            } // mSphAttr
        };
    
        static dCcD_SrcSph foot_sph_src = {
            {
                {0x0, {{AT_TYPE_CSTATUE_SWING, 0x1, 0x1f}, {0xd8fbfdff, 0x43}, 0x75}}, // mObj
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0}, // mGObjAt
                {dCcD_SE_METAL, 0x2, 0x0, 0x0, 0x3}, // mGObjTg
                {0x0}, // mGObjCo
            }, // mObjInf
            {
                {{0.0f, 0.0f, 0.0f}, 40.0f} // mSph
            } // mSphAttr
        };
    
        static dCcD_SrcSph beam_sph_src = {
            {
                {0x0, {{AT_TYPE_CSTATUE_SWING, 0x4, 0x1f}, {0x0, 0x0}, 0x0}}, // mObj
                {dCcD_SE_NONE, 0x0, 0x1, 0x0, 0x0}, // mGObjAt
                {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x2}, // mGObjTg
                {0x0}, // mGObjCo
            }, // mObjInf
            {
                {{0.0f, 0.0f, 0.0f}, 20.0f} // mSph
            } // mSphAttr
        };

        for (int i = 0; i < 8; i++) {
            if (i < 2) {
                a_this->mBodySph[i].Set(body_sph_src);
                a_this->mBodySph[i].SetStts(&a_this->mCcStts);

                a_this->mHandSph[i].Set(hand_sph_src);
                a_this->mHandSph[i].SetStts(&a_this->mCcStts);
            }

            a_this->mFootSph[i].Set(foot_sph_src);
            a_this->mFootSph[i].SetStts(&a_this->mCcStts);
        }

        a_this->mCoreSph.Set(core_sph_src);
        a_this->mCoreSph.SetStts(&a_this->mCcStts);

        a_this->mBeamSph.Set(beam_sph_src);
        a_this->mBeamSph.SetStts(&a_this->mCcStts);
        a_this->mBeamSph.SetAtMtrl(dCcD_MTRL_FIRE);

        a_this->mCreatureSound.init(&i_this->current.pos, &i_this->eyePos, 3, 1);
        a_this->mAtInfo.mpSound = &a_this->mCreatureSound;
        a_this->mBeamSound.init(&a_this->field_0x6d8, 1);

        i_this->health = 500;
        i_this->field_0x560 = 500;

#if TARGET_PC
        dAlbwBoss_armogohmaResetFightState();
#endif

        a_this->mAcch.Set(fopAcM_GetPosition_p(i_this), fopAcM_GetOldPosition_p(i_this), i_this, 1, &a_this->mAcchCir, fopAcM_GetSpeed_p(i_this), NULL, NULL);
        a_this->mAcchCir.SetWall(200.0f, 100.0f);

        a_this->mAction = ACTION_MOVE;
        a_this->mMode = 10;
        a_this->mDemoMode = 1;

        a_this->field_0x1ae0 = 31.0f;
        a_this->field_0x5d8 = -32768.0f;
        a_this->field_0x5d4 = -32768.0f;;

        i_this->shape_angle.x = 0x8000;
        i_this->current.angle.x = 0x8000;
        a_this->field_0x73c.y = 2520.0f;
        i_this->current.pos.y = 2520.0f;
        a_this->mTargetMovePos = i_this->current.pos;

#if TARGET_PC
        dAlbwBoss_tryApplyActorBootstrap(fpcNm_B_GM_e, i_this);
#endif

        daB_GM_Execute(a_this);
    }

    return phase_state;
}

static DUSK_CONST actor_method_class l_daB_GM_Method = {
    (process_method_func)daB_GM_Create,
    (process_method_func)daB_GM_Delete,
    (process_method_func)daB_GM_Execute,
    (process_method_func)daB_GM_IsDelete,
    (process_method_func)daB_GM_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_B_GM = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_B_GM_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(b_gm_class),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_B_GM_e,
    /* Actor SubMtd */ &l_daB_GM_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ENEMY_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

AUDIO_INSTANCES;
