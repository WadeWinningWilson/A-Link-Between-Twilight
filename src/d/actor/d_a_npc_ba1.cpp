/**
 * d_a_npc_ba1.cpp
 * NPC - Link's Grandma
 */
// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_npc_ba1.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching
// KIT-NOTE: this TU shares only ~4% of its text with the donor file, which
// looks like a mislabelled `native-port` and is NOT. The donor's own
// d_a_npc_ba1.cpp is NonMatching (largely unimplemented upstream), so there
// is little donor SOURCE TEXT to match -- this port was reconstructed from
// the retail binary via m2c (see the §261 note below), which is a port of
// the same actor by a different route. Low text overlap is EXPECTED here and
// is not evidence of a wrong lineage tag. Recorded so the step-13 divergence
// measurement does not re-flag it every run.

// ============================================================
// §261 WW Link's Grandma (NPC_BA1) DIRECT PORT.
// Donor behavior: Foundry port-kit named m2c draft (bodies) + §260
// rodata_tables.h (byte-faithful data) — logic is donor-verbatim, only the
// direct-port crash recipes + identifier renames are applied (each marked §261),
// mirroring the landed ls1 §244 / bm1 §246 / zl1 §254 sister ports.
//
// BASE CLASS: fopNpc_npc_c (offset-exact with ba1's own draft cluster —
// m_jnt@0x290 / mpMorf@0x330 / mObjAcch@0x334 / mAcchCir@0x4F8 / mStts@0x538 /
// mCyl@0x574 — and provides talk()/setCollision()/getMsg vtable). The WW C++
// immediate base is fopAc_ac_c with a manual fopNpc overlay; deriving fopNpc in
// the port is behaviorally identical + sibling-consistent (coordinator-confirmed).
//
// Offset->port-API Rosetta (resolved §261): gi+0x624 -> dComIfGs_*EventBit /
// getEventReg/setEventReg ; gi+0x1158 -> dComIfGs_*TmpBit ; gi+0x52CC ->
// dComIfGp_evmng_* ; gi.unk5B44 -> getLinkPlayer/dComIfGp_getPlayer ;
// gi.unk5AC8 demo -> dComIfGp_demo_getActor ; gi+0x12A0 -> dComIfG_Bgsp() ;
// gi+0x5808 -> dComIfGp_getAttention() ; gi.unk52B8/9 -> evmng ChkPresentEnd /
// event_getPreItemNo ; gi.unk529A -> event_runCheck ; gi.unk5BDB/DC -> mesg
// anime attr/tag.  Arc = "Ba" (§260 stringBase0 +0x35); joints head/backbone/footL.
//
// @4216.unkXX are m2c synthetic r2-relative float addressing; resolved from the
// asm .sdata2 consecutive block (respawn -290/0/110) + ls1 shadow precedent
// (800/40/+150) + ba1's own donor float set by context. Values that could not be
// uniquely mapped to a donor float carry a `// [INFERENCE-NEEDED] §261 fpool` tag.
// ============================================================

#include "d/dolzel_rel.h" // IWYU pragma: keep
#include "d/actor/d_a_npc_ba1.h"
#include <cmath>  // §338 std::fabs (cylinder Y-band)
#include "SSystem/SComponent/c_counter.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_ww_actor_shims.h"   // §261 dSnap_RegistFig / dLetter_send / fpcDwPi shim
#include "d/d_ext_npc_mount.h"        // §261 dExtNpcMount_acquireModelData (parse-at-consume)
#include "d/d_drawlist.h"             // §261 dDlst_shadowControl_c::getSimpleTex()
#include "d/d_demo.h"                 // §261 dDemo_actor_c / dDemo_setDemoData
#include "d/d_bg_s.h"                 // §261 dBgS_GetGndMtrlSndId_Func
#include "d/d_resorce.h"              // §261 dComIfG res id lookups
#include "dusk/logging.h"             // §266 tale-path acceptance probes (strip after accept)
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_hostIO.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "d/actor/d_a_player.h"       // §261 daPy_getPlayerActorClass()

// §303 SYMBOL ROUTE — MUST BE THE LAST INCLUDE. Remaps ba1's verbatim
// dComIfGs_*EventBit / *EventReg calls (all WW dSv_event indices —
// 0x520/0x0001/0x2A20/0x608/0x602/0x601/0x740/0x780/0xE20/0xA60F/…) to the donor
// event-flag block (d_ext_save_flags), so ba1 reads AND writes pure donor flags
// instead of TP's colliding table. TP TUs never include this. Replaces the §266
// hand-bridge (below).
#include "d/d_ext_save_flags_route.h"
#include "d/d_kankyo_ww.h"           // §404 WW lighting write-path (was the empty stub)

// ============================================================
// §261 [INFERENCE-NEEDED] gameInfo status bytes with no mapped port accessor yet
// (donor gi.unkB4/unkB5/unk1A0/unk5BD3). Bridged to the inactive branch so the
// build is green + the common path is preserved; enumerated for acceptance.
// ============================================================
static inline bool ba1_isCameraShip()      { return false; } // gi.unkB5 (on-ship gate)
static inline bool ba1_isCameraLand()       { return false; } // gi.unkB4 (player-state gate)
static inline bool ba1_isCameraLetterBox()  { return false; } // gi.unk1A0 (stage-variant select)
static inline bool ba1_xyCheckStay()        { return false; } // gi.unk5BD3 == 0x57 (LinkRM entrance guard)

// §261 DEMO_SELECT collapses to the retail argument in this port (VERSION > DEMO).
#ifndef DEMO_SELECT
#define DEMO_SELECT(a_demo, a_retail) (a_retail)
#endif

// ============================================================
// §260 rodata — byte-faithful donor data (raw u32 ground truth). anm tables are
// indexed as anm_prm_c at the access sites (memcmp bar: layout is
// {s8 anmNum; s8 btpNum; u16 pad; f32 morf; f32 speed; int loop} == 0x10).
// ============================================================
static const char* l_evn_tbl[6] = { "Use_Fairy", "Ba1_Get_Itm", "Ganbaru", "tale_1", "None", "tale_2" };
static const char* a_cut_tbl[2] = { "ACTION", "START_TALE1" };

// KIT-DONOR-DATA: 40 lookup-table d/actor/d_a_npc_ba1.cpp
static const s32 a_bck_resID_tbl[10] = { 7, 3, 4, 5, 1, 8, 6, 2, 0, 0x17 };
// KIT-DONOR-DATA: 48 lookup-table d/actor/d_a_npc_ba1.cpp
static const s32 a_btp_resID_tbl[12] = { 0x16, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };

// a_anm_prm_tbl$4490[0xB] — §260 raw ground truth
// KIT-DONOR-DATA: 176 lookup-table d/actor/d_a_npc_ba1.cpp a_anm_prm_tbl$4490
static const u32 a_anm_prm_tbl_4490[] = {
    0x00000000, 0x41000000, 0x3F800000, 0x00000002,
    0x01000000, 0x41000000, 0x3F800000, 0x00000002,
    0x02000000, 0x41000000, 0x3F800000, 0x00000002,
    0x03000000, 0x41000000, 0x3F800000, 0x00000002,
    0x04000000, 0x41000000, 0x3F800000, 0x00000000,
    0x05000000, 0x41000000, 0x3F800000, 0x00000002,
    0x06000000, 0x41000000, 0x3F800000, 0x00000002,
    0x07000000, 0x41000000, 0x3F800000, 0x00000002,
    0x08000000, 0x41000000, 0x3F800000, 0x00000002,
    0x090B0000, 0x41000000, 0x3F800000, 0x00000000,
    0x00090000, 0x00000000, 0x3F800000, 0x00000002,
};
// a_anm_prm_tbl$4497[8] — §260 raw ground truth
// KIT-DONOR-DATA: 128 lookup-table d/actor/d_a_npc_ba1.cpp a_anm_prm_tbl$4497
static const u32 a_anm_prm_tbl_4497[] = {
    0xFFFF0000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0x00000000, 0x41000000, 0x3F800000, 0x00000002,
    0xFFFF0000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0x00020000, 0x41000000, 0x3F800000, 0x00000002,
    0x07070000, 0x41000000, 0x3F800000, 0x00000002,
    0xFFFF0000, 0x00000000, 0x00000000, 0xFFFFFFFF,
    0x00090000, 0x41000000, 0x3F800000, 0x00000002,
    0x00000000, 0x41000000, 0x3F800000, 0x00000002,
};
// a_anm_prm_tbl$4553[0x15] — §260 raw ground truth
// KIT-DONOR-DATA: 336 lookup-table d/actor/d_a_npc_ba1.cpp a_anm_prm_tbl$4553
static const u32 a_anm_prm_tbl_4553[] = {
    0x00000000, 0x41000000, 0x3F800000, 0x00000002,
    0x01000000, 0x41000000, 0x3F800000, 0x00000002,
    0x02000000, 0x41000000, 0x3F800000, 0x00000002,
    0x03000000, 0x41000000, 0x3F800000, 0x00000002,
    0x00010000, 0x41000000, 0x3F800000, 0x00000002,
    0x02020000, 0x41000000, 0x3F800000, 0x00000002,
    0x01030000, 0x41000000, 0x3F800000, 0x00000002,
    0x03020000, 0x41000000, 0x3F800000, 0x00000002,
    0x02030000, 0x41000000, 0x3F800000, 0x00000002,
    0x02040000, 0x41000000, 0x3F800000, 0x00000002,
    0x02060000, 0x41000000, 0x3F800000, 0x00000002,
    0x03050000, 0x41000000, 0x3F800000, 0x00000002,
    0x03060000, 0x41000000, 0x3F800000, 0x00000002,
    0x07070000, 0x41000000, 0x3F800000, 0x00000002,
    0x06080000, 0x41000000, 0x3F800000, 0x00000002,
    0x06010000, 0x41000000, 0x3F800000, 0x00000002,
    0x050A0000, 0x41000000, 0x3F800000, 0x00000002,
    0x05010000, 0x41000000, 0x3F800000, 0x00000002,
    0x05090000, 0x41000000, 0x3F800000, 0x00000002,
    0x05000000, 0x41000000, 0x3F800000, 0x00000002,
    0x00080000, 0x41000000, 0x3F800000, 0x00000002,
};
// KIT-DONOR-DATA: 4 lookup-table d/actor/d_a_npc_ba1.cpp
static const u32 a_size_tbl[1] = { 0x000272E0 };

#define ANM4490 ((const daNpc_Ba1_c::anm_prm_c*)a_anm_prm_tbl_4490)
#define ANM4497 ((const daNpc_Ba1_c::anm_prm_c*)a_anm_prm_tbl_4497)
#define ANM4553 ((const daNpc_Ba1_c::anm_prm_c*)a_anm_prm_tbl_4553)

static daNpc_Ba1_HIO_c l_HIO;
static fopAc_ac_c* l_check_inf[0x14];
static s32 l_check_wrk = 0;

// §261 fpcDwPi_NPC_BA1_e absent in port -> reuse a valid draw-prio slot (mirrors zl1 §254).
#ifndef fpcDwPi_NPC_BA1_e
#define fpcDwPi_NPC_BA1_e fpcDwPi_E_RD_e
#endif

/* 000000EC-00000150 __ct__15daNpc_Ba1_HIO_cFv */
daNpc_Ba1_HIO_c::daNpc_Ba1_HIO_c() {
    // §261 donor HIO ctor loads a_prm_tbl$4153 (byte-faithful §260) into mPrm.
    // KIT-DONOR-DATA: 36 lookup-table d/actor/d_a_npc_ba1.cpp a_prm_tbl$4153
    static const u32 a_prm_tbl_4153[9] = {
        0x15542000, 0xEC78E000, 0x00001000, 0x0000F000, 0x04000400,
        0x42E60000 /*115f*/, 0x00000000, 0x43FA0000 /*500f*/, 0x43960000 /*300f*/,
    };
    memcpy(&mPrm, a_prm_tbl_4153, sizeof(a_prm_tbl_4153));
    m04 = -1;
    m08 = -1;
}

// ============================================================
// §261 searchActor_Fa — census callback: collect NPC_BA1 actors (donor name 0x16A)
// ============================================================
static void* searchActor_Fa(void* i_param_1, void*) {
    if (l_check_wrk < 0x14 && fopAcM_IsActor(i_param_1) && fopAcM_GetName(i_param_1) == fpcNm_NPC_BA1_e) {
        l_check_inf[l_check_wrk] = (fopAc_ac_c*)i_param_1;
        l_check_wrk += 1;
    }
    return NULL;
}

// ============================================================
// §261 node callback — head/backbone joint control (donor nodeBa1Control)
// ============================================================
static BOOL nodeCallBack_Ba1(J3DJoint* i_joint, int i_param) {  // §261 J3DNode*->J3DJoint*
    if (i_param == 0) {
        if (j3dSys.getModel()->getUserArea() != NULL) {
            reinterpret_cast<daNpc_Ba1_c*>(j3dSys.getModel()->getUserArea())->nodeBa1Control(i_joint, j3dSys.getModel());
        }
    }
    return TRUE;
}

void daNpc_Ba1_c::nodeBa1Control(J3DJoint* i_joint, J3DModel* i_model) {
    static cXyz a_eye_pos_off(20.0f, 2.125f, 0.0f);  // §261 [INFERENCE-NEEDED] fpool eye offset (donor _4925/_4926 pair 1.75/2.125)
    s32 jntNo = i_joint->getJntNo();
    mDoMtx_stack_c::copy(i_model->getAnmMtx(jntNo));
    if (jntNo == m_hed_jnt_num) {
        mDoMtx_stack_c::YrotM((s16)-m_jnt.getHead_y());
        mDoMtx_stack_c::ZrotM((s16)-m_jnt.getHead_x());
        mDoMtx_stack_c::multVec(&a_eye_pos_off, &eyePos);
    }
    if (jntNo == m_bbone_jnt_num) {
        mDoMtx_stack_c::XrotM(m_jnt.getBackbone_y());
        mDoMtx_stack_c::ZrotM(m_jnt.getBackbone_x());
    }
    cMtx_copy(mDoMtx_stack_c::get(), j3dSys.mCurrentMtx);
    i_model->setAnmMtx(jntNo, mDoMtx_stack_c::get());
}

// ============================================================
// §261 event XY callbacks (LinkRM entrance guard)
// ============================================================
bool daNpc_Ba1_c::XyCheck_cB(int i_roomNo) {
    return ba1_xyCheckStay();  // §261 [INFERENCE-NEEDED] gi.unk5BD3==0x57 room/stay id
}
s16 daNpc_Ba1_c::XyEvent_cB(int) {
    return mEventIdTable[0];
}
static void daNpc_Ba1_XyCheck_cB(void* i_this, int i_arg) { ((daNpc_Ba1_c*)i_this)->XyCheck_cB(i_arg); }
static void daNpc_Ba1_XyEvent_cB(void* i_this, int i_arg) { ((daNpc_Ba1_c*)i_this)->XyEvent_cB(i_arg); }

// ============================================================
// §261 per-charType init (set_action + start state). ptmf tables §260:
//   @4245->wait_action1  @4264->wait_action2  @4286->demo_action1
//   @4298->wait_action3  @4318->wait_action4  @5285->wait_action4
// ============================================================
bool daNpc_Ba1_c::init_BA1_0() {
    // §303 mCharType 0 = loft/birthday (pre-tale) Grandma. DONOR condition, restored:
    // the §266 hand-bridge forced this to `true` because 0x520/0x0001 (WW dSv_event
    // indices) collided with a SET flag in TP's table. The §303 route now resolves
    // these to the donor block — where 0x520/0x0001 are clear on a fresh save — so the
    // real donor gate reads correctly and mCharType 0 spawns at the loft to self-trigger
    // tale_1. (docs/state/grandma-native-tale.md full-fix, landed.)
    if (!dComIfGs_isEventBit(0x520) && !dComIfGs_isEventBit(0x0001)) {
        set_action(&daNpc_Ba1_c::wait_action1, NULL);
        mpItemModel = NULL;
        return true;
    }
    return false;
}
bool daNpc_Ba1_c::init_BA1_1() {
    if (dComIfGs_isEventBit(0x520)) {
        return false;
    }
    bool result = dComIfGs_isEventBit(0x0001) != 0;
    if (result) {
        if (ba1_isCameraShip()) {  // §261 [INFERENCE-NEEDED] gi.unkB5 on-ship gate
            current.pos.set(-290.0f, 0.0f, 110.0f);  // §261 fpool consecutive block (respawn)
            shape_angle.y = 0;
        }
        set_action(&daNpc_Ba1_c::wait_action2, NULL);
        fopAcM_OffStatus(this, fopAcStts_NOEXEC_e);  // §261 unk1C4 &= ~0x80 (bit-faithful)
        mpItemModel = NULL;
    }
    return result;
}
bool daNpc_Ba1_c::init_BA1_2() {
    mpItemModel = NULL;
    set_action(&daNpc_Ba1_c::demo_action1, NULL);
    return true;
}
bool daNpc_Ba1_c::init_BA1_3() {
    if (!dComIfGs_isEventBit(0x520)) {
        return false;
    }
    bool result = dComIfGs_isEventBit(0x2A20) == 0;
    if (result) {
        attention_info.distances[1] = 0x1B;
        attention_info.distances[3] = 0x1C;
        fopAcM_OffStatus(this, fopAcStts_NOEXEC_e | fopAcStts_CULL_e);  // §261 unk1C4 &= ~0x180 (bit-faithful)
        set_action(&daNpc_Ba1_c::wait_action3, NULL);
        gravity = 0.0f;
        // §261 [INFERENCE-NEEDED] donor registers eventInfo XY-pass/get-event callbacks
        // (unk104/unk100); the port's dEvt_info_c has no such slots -> bridged no-op.
        // (LinkRM auto-entrance guard for post-rescue Grandma; XyCheck_cB/XyEvent_cB unused.)
        (void)&daNpc_Ba1_XyCheck_cB; (void)&daNpc_Ba1_XyEvent_cB;
    }
    return result;
}
bool daNpc_Ba1_c::init_BA1_4() {
    if (!dComIfGs_isEventBit(0x520)) {
        return false;
    }
    bool result = dComIfGs_isEventBit(0x2A20) != 0;
    if (result) {
        mpItemModel = NULL;
        set_action(&daNpc_Ba1_c::wait_action4, NULL);
    }
    return result;
}

/* createInit */
bool daNpc_Ba1_c::createInit() {
    for (int i = 0; i < 6; i++) {
        mEventIdTable[i] = dComIfGp_evmng_getEventIdx(l_evn_tbl[i], 0xFF);
    }
    // §266 ACCEPTANCE PROBE (Phase-1 resolution at create). Foundry merged tale_1/2,
    // Use_Fairy, Ganbaru into R_DL01. A valid (non -1) idx here proves getEventIdx
    // resolves in the tale stage. Strip after acceptance.
    DuskLog.info("[Ba1Tale] §266 event resolve: Use_Fairy={} Ba1_Get_Itm={} Ganbaru={} "
                 "tale_1={} tale_2={}",
                 (int)mEventIdTable[0], (int)mEventIdTable[1], (int)mEventIdTable[2],
                 (int)mEventIdTable[3], (int)mEventIdTable[5]);
    // §266 VARIANT-GATE PROBE — which mCharType the save allows. Strip after accept.
    //   mCharType 0 = loft/birthday Grandma (needs 520 & 0001 BOTH clear) → self-triggers tale
    //   mCharType 1 = post-0001 (0001 set, 520 clear)   mCharType 3 = 520 set, 2A20 clear
    //   mCharType 4 = 520 & 2A20 set. Forced param must match the save or createInit tears down.
    DuskLog.info("[Ba1Tale] §266 variant-gate: mCharType={} bit520={} bit0001={} bit2A20={}",
                 (int)mCharType, (int)(dComIfGs_isEventBit(0x520) != 0),
                 (int)(dComIfGs_isEventBit(0x0001) != 0), (int)(dComIfGs_isEventBit(0x2A20) != 0));
    attention_info.flags = 0xA;
    attention_info.distances[1] = 0xAB;
    attention_info.distances[3] = 0xAB;
    gravity = -4.5f;
    mInitialPos = current.pos;
    mEventCut.setActorInfo2((char*)"Ba1", this);  // §261 stringBase0 +0x31
    mAnmNum = 0xA;

    bool ok;
    switch (mCharType) {
        case 0: ok = init_BA1_0(); break;
        case 1: ok = init_BA1_1(); break;
        case 2: ok = init_BA1_2(); break;
        case 3: ok = init_BA1_3(); break;
        case 4: ok = init_BA1_4(); break;
        default: ok = false; break;
    }
    if (!ok) {
        return false;
    }
    mShapeAngleTmp = current.angle;
    shape_angle = mShapeAngleTmp;
    mGetReg = dComIfGs_getEventReg(0xA60F);
    mStts.Init(0xFF, 0xFF, this);
    mCyl.SetStts(&mStts);
    mCyl.Set(dNpc_cyl_src);
    mpMorf->setMorf(0.0f);
    setMtx(true);
    return true;
}

/* setMtx */
void daNpc_Ba1_c::setMtx(bool i_setAttn) {
    if (!m80B_inDemo) {
        plyTexPttrnAnm();
        mMorfAnimStopped = mpMorf->play(&eyePos, 0, 0);
        if (mpMorf->getFrame() < mPrevMorfFrame) {
            mMorfAnimStopped = 1;
        }
        mPrevMorfFrame = mpMorf->getFrame();
        if (mCharType != 3) {
            mObjAcch.CrrPos(dComIfG_Bgsp());
        }
    }
    tevStr.room_no = dComIfG_Bgsp().GetRoomId(mObjAcch.m_gnd);
    tevStr.YukaCol = dComIfG_Bgsp().GetPolyColor(mObjAcch.m_gnd);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(mShapeAngleTmp.y);
    mpMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
    // ========================================================================
    // 400 THE GRANDMA CRASH — `calc()` MEANS OPPOSITE THINGS IN THE TWO LINEAGES.
    //
    // Donor J3DMtxCalc declares `virtual void calc(u16 jnt_no)` as the PER-JOINT
    // callback, so the donor's `mDoExt_McaMorf::calc()` is a separate no-arg
    // DRIVER: setMtxCalc(this) then mpModel->calc().
    //
    // The receiver's J3DMtxCalc declares `virtual void calc() = 0` as the
    // per-joint callback. So `mpMorf->calc()` here does NOT run the model — it
    // runs ONE joint's transform, and it reads J3DMtxCalc's STATIC mJoint /
    // mMtxBuffer, which only the joint-tree walk inside J3DModel::calc() sets.
    // Called straight from createInit, nothing has set them: calcTransform
    // dereferences a NULL mtxBuffer at getAnmMtx(jntNo) and faults at 0x80.
    //
    // Intermittent because the statics hold whatever the LAST model calc left
    // behind — so it survives or dies depending on what ran immediately before,
    // which is exactly why interior cycling reproduced it and a fixed count of
    // cycles never did.
    //
    // The receiver's equivalent of the donor's `mpMorf->calc()` is
    // modelCalc() — byte-for-byte the donor body (m_Do_ext.cpp:1511-1520), and
    // already documented as that mapping in d_a_kamome.cpp:14. This is the
    // donor's own call, expressed in the receiver's names; not a workaround.
    // ========================================================================
    mpMorf->modelCalc();
    // §266 probe stripped — it read setMtx's calc() (ANIM matrices, all 0,0,0 here) which
    // is EXPECTED; the render matrices are composed by modelCalc() in _draw (the fix).
    if (mpItemModel != NULL) {
        mpItemModel->setBaseTRMtx(mpMorf->getModel()->getAnmMtx(m_footL_jnt_num));
        mpItemModel->calc();
    }
    setAttention(i_setAttn);
}

int daNpc_Ba1_c::anmNum_toResID(int i_anmNum) { return a_bck_resID_tbl[i_anmNum]; }
int daNpc_Ba1_c::btpNum_toResID(int i_btpNum) { return a_btp_resID_tbl[i_btpNum]; }

/* setBtp */
bool daNpc_Ba1_c::setBtp(bool i_modify, int i_btpNum) {
    J3DModelData* model_data = mpMorf->getModel()->getModelData();
    mpBtpRes = (J3DAnmTexPattern*)dComIfG_getObjectIDRes(mArcName, btpNum_toResID(i_btpNum));
    JUT_ASSERT(0x239, mpBtpRes != NULL);
    bool ok = mBtpAnm.init(model_data, mpBtpRes, 1, 2, 1.0f, 0, -1) == 1;  // §261 port btpAnm::init 7-arg
    if (ok) {
        mBtpFrame = 0;
        mBlinkTimer = 0;
    }
    return ok;
}

bool daNpc_Ba1_c::iniTexPttrnAnm(bool i_modify) { return setBtp(i_modify, mBtpNum); }

/* plyTexPttrnAnm */
void daNpc_Ba1_c::plyTexPttrnAnm() {
    if (mBtpNum != 0 || !cLib_calcTimer(&mBlinkTimer)) {
        mBtpFrame += 1;
        if (mBtpFrame >= mpBtpRes->getFrameMax()) {
            if (mBtpNum != 0) {
                mBtpFrame = mpBtpRes->getFrameMax();
            } else {
                mBlinkTimer = (s16)(30.0f + cM_rndF(60.0f));
                mBtpFrame = 0;
            }
        }
    }
}

void daNpc_Ba1_c::setAnm_tex(s8 i_btpNum) {
    if (mBtpNum != i_btpNum) {
        mBtpNum = i_btpNum;
        iniTexPttrnAnm(true);
    }
}

/* setAnm_anm */
bool daNpc_Ba1_c::setAnm_anm(anm_prm_c* i_prm) {
    if (mAnmNum == i_prm->mAnmNum) {
        return true;
    }
    mAnmNum = i_prm->mAnmNum;
    dNpc_setAnmIDRes(mpMorf, i_prm->mLoopMode, i_prm->mMorf, i_prm->mPlaySpeed, anmNum_toResID(mAnmNum), -1, mArcName);
    mPrevMorfFrame = 0.0f;
    m7EF = 0;
    mMorfAnimStopped = 0;
    return true;
}

void daNpc_Ba1_c::setAnm_NUM(int i_num, int i_setTex) {
    if (i_setTex != 0) {
        setAnm_tex(ANM4490[i_num].mBtpNum);
    }
    setAnm_anm((anm_prm_c*)&ANM4490[i_num]);
}

bool daNpc_Ba1_c::setAnm() {
    if (ANM4497[mSttNum].mBtpNum >= 0) {
        setAnm_tex(ANM4497[mSttNum].mBtpNum);
    }
    if (ANM4497[mSttNum].mAnmNum >= 0) {
        setAnm_anm((anm_prm_c*)&ANM4497[mSttNum]);
    }
    return true;
}

void daNpc_Ba1_c::chg_anmTag() {}
void daNpc_Ba1_c::control_anmTag() {}

void daNpc_Ba1_c::chg_anmAtr(u8 i_atr) {
    if (i_atr < 0x15) {
        if (i_atr == mAnmAtr) {
            return;
        }
        mAnmAtr = i_atr;
        setAnm_ATR(1);
    }
}

void daNpc_Ba1_c::control_anmAtr() {
    if (mAnmAtr < 7 && mAnmAtr >= 5) {
        if (mLookBackMode != 1) {
            mLookBackMode = 1;
            mMesgAtrOnce = 1;  // §261 m29A_lookFlag maps to lookFlag; here set via draft
        }
    }
}

void daNpc_Ba1_c::setAnm_ATR(int i_setTex) {
    if (i_setTex != 0) {
        setAnm_tex(ANM4553[mAnmAtr].mBtpNum);
    }
    setAnm_anm((anm_prm_c*)&ANM4553[mAnmAtr]);
}

void daNpc_Ba1_c::anmAtr(u16 i_msgStatus) {
    switch (i_msgStatus) {
        case 6:
            if (mMesgAtrOnce == 0) {
                mAnmAtr = 0xFF;
                chg_anmAtr(dComIfGp_getMesgAnimeAttrInfo());
                mMesgAtrOnce += 1;
            }
            {
                u8 tag = dComIfGp_getMesgAnimeTagInfo();
                dComIfGp_clearMesgAnimeTagInfo();
                if (tag != 0xFF && mMesgTag != tag) {
                    mMesgTag = tag;
                    chg_anmTag();
                }
            }
            break;
        case 14:
            mMesgAtrOnce = 0;
            break;
    }
    control_anmTag();
    control_anmAtr();
}

/* eventOrder */
void daNpc_Ba1_c::eventOrder() {
    if (mOrderType == 1 || mOrderType == 2) {
        eventInfo.onCondition(dEvtCnd_CANTALK_e);
        if (mCharType == 3 && !dComIfGs_isEventBit(0x2A20)) {
            eventInfo.onCondition(dEvtCnd_CANTALKITEM_e);
        }
        if (mOrderType == 1) {
            fopAcM_orderSpeakEvent(this, 0, 0);
        }
    } else if (mOrderType >= 3) {
        mEventIdx = mOrderType - 3;
        // §294: DONOR order (covenant correction). ba1 orders tale_1 — the DONOR event
        // (Ba1 / CAMERA / Link, donor mFlags). tale_1's cut START_TALE1 fires the donor
        // two-step re-entrance to TALE_DEMO (see cut_move_START_TALE1). The reverted §278
        // in-place PACKAGE-arc gate is GONE: no arc residency here — Demo01 residency happens
        // at re-entry in the §273 wire, and TALE_DEMO carries its own donor finish accounting.
        // §266 ACCEPTANCE PROBE — the order attempt + result (0/1 = refused/accepted).
        // Refused ⇒ isOrderOK() false (another event owns the lane, e.g. a mount
        // TALE_DEMO still firing — the coexistence conflict to watch). Strip after accept.
        s32 _ord = fopAcM_orderOtherEventId(this, mEventIdTable[mEventIdx], 0xFF, 0xFFFF, 0, 1);
        DuskLog.info("[Ba1Tale] §266 orderOtherEventId evn='{}' idx={} -> {}", l_evn_tbl[mEventIdx],
                     (int)mEventIdTable[mEventIdx], (int)_ord);
        // §270: a fresh legitimate event now owns the slot — stand the arrival-G-guard down
        // so its №89 force-end doesn't truncate the tale at frame 0 (Foundry root-cause).
        if (_ord != 0) {
            dExtNpcMount_clearArrivalGuard("ba1 event order");
        }
    }
}

/* checkOrder */
void daNpc_Ba1_c::checkOrder() {
    if (eventInfo.checkCommandDemoAccrpt()) {
        if (dComIfGp_evmng_startCheck(mEventIdTable[mEventIdx])) {
            mOrderType = 0;
        }
    } else if (checkCommandTalk() && (mOrderType == 1 || mOrderType == 2)) {
        mOrderType = 0;
        mTalkAccepted = 1;
    }
}

/* chk_talk */
bool daNpc_Ba1_c::chk_talk() {
    bool ret = true;
    mPresentItemNo = 0xFF;
    if (dComIfGp_event_chkTalkXY()) {
        if (dComIfGp_evmng_ChkPresentEnd()) {
            mPresentItemNo = dComIfGp_event_getPreItemNo();
        } else {
            ret = false;
        }
        mPresentItemNo = dComIfGp_event_getPreItemNo();
    }
    return ret;
}

/* chk_drct */
bool daNpc_Ba1_c::chk_drct(float i_deg) {
    fopAc_ac_c* player = daPy_getPlayerActorClass();
    s16 diff = (s16)(cLib_targetAngleY(&current.pos, &player->current.pos) - shape_angle.y);
    return abs(diff) < (s16)(182.04445f * i_deg);
}

/* chk_partsNotMove */
bool daNpc_Ba1_c::chk_partsNotMove() {
    return mLookAngle.x == m_jnt.getHead_y() && mLookAngle.y == 0 && mLookAngle.z == shape_angle.y;
}

/* lookBack */
void daNpc_Ba1_c::lookBack() {
    mLookAngle.x = m_jnt.getHead_y();
    mLookAngle.y = 0;
    mLookAngle.z = shape_angle.y;
    cXyz srcPos(current.pos.x, eyePos.y, current.pos.z);
    cXyz dstPos(0.0f, 0.0f, 0.0f);
    cXyz* dstPos_p = NULL;
    s16 desiredYrot = shape_angle.y;
    bool headOnly = mAnmChangeFlag;

    switch (mLookBackMode) {
        case 1:
            dstPos = dNpc_playerEyePos(-20.0f);
            dstPos_p = &dstPos;
            srcPos.set(current.pos.x, eyePos.y, current.pos.z);
            break;
        case 2:
            dstPos = mInitialPos;
            dstPos_p = &dstPos;
            srcPos.set(current.pos.x, eyePos.y, current.pos.z);
            break;
        case 3:
            desiredYrot = mLookTimer;
            break;
    }
    cLib_addCalcAngleS2(&mHeadTurnVel, l_HIO.mPrm.mCalcAngleTarget, 4, 0x800);
    if (!m_jnt.trnChk()) {
        mHeadTurnVel = 0;
    }
    m_jnt.lookAtTarget(&shape_angle.y, dstPos_p, srcPos, desiredYrot, mHeadTurnVel, headOnly);
}

/* next_msgStatus */
u16 daNpc_Ba1_c::next_msgStatus(u32* i_msgNo) {
    u16 status = fopMsg_MODE_MSG_CONTINUE_e;
    switch (*i_msgNo) {
        case 0x7EB: *i_msgNo = 0x7EC; break;
        case 0x7EF: *i_msgNo = 0x7F0; break;
        case 0x7F3: *i_msgNo = 0x7F4; break;
        case 0x7F4:
        block_5:
            if (dComIfGs_checkEmptyBottle()) {
                *i_msgNo = 0x7F5;
            } else {
                *i_msgNo = 0x7F9;
            }
            break;
        case 0x7F6: *i_msgNo = 0x7F7; break;
        case 0x7FA:
        case 0x7FB:
        case 0x7FC:
        case 0x7FD:
            if (true /* §261 [INFERENCE-NEEDED] dSv isBottleItem(0x55) */) {
                if (!dComIfGs_checkEmptyBottle()) {
                    *i_msgNo = 0x803;
                } else if (dComIfGs_checkBottle(0x55)) {
                    *i_msgNo = 0x7FE;
                } else {
                    *i_msgNo = 0x7FF;
                }
            } else {
                goto block_5;
            }
            break;
        case 0x7FF:
            switch (mpCurrMsg ? mpCurrMsg->mode : 0) {  // §261 unk6B0->unkFA end-mode select
                case 0: *i_msgNo = 0x800; break;
                case 1: *i_msgNo = 0x802; break;
            }
            break;
        default:
            status = fopMsg_MODE_MSG_END_e;
            break;
    }
    return status;
}

/* getMsg_BA1_0 */
u32 daNpc_Ba1_c::getMsg_BA1_0() {
    if (mMsgIdx != 0) {
        return 0x7D8;
    }
    if (m7F9_fairyReady != 0) {
        return (dComIfGs_isEventBit(0x608) != 0) + 0x7E6;
    }
    return (m7FB != 0) + 0x7E4;
}

/* getMsg_BA1_1 */
u32 daNpc_Ba1_c::getMsg_BA1_1() {
    if (dComIfGs_isEventBit(0xE20)) {
        if (ba1_isCameraShip()) {  // §261 gi.unkB5
            return 0x80B;
        }
        return (dComIfGs_isEventBit(0x740) != 0) + 0x7EF;
    }
    if (dComIfGs_isEventBit(0x780)) {
        return 0x7ED;
    }
    if (ba1_isCameraLand()) {  // §261 [INFERENCE-NEEDED] gi.unkB4 player-state gate
        return (dComIfGs_isEventBit(0x602) != 0) + 0x7EB;
    }
    return (dComIfGs_isEventBit(0x601) != 0) + 0x7E8;
}

/* getMsg_BA1_3 */
u32 daNpc_Ba1_c::getMsg_BA1_3() {
    if (m7F0_movDone != 0) {
        if (dComIfGs_isEventBit(0x2A20)) {
            return 0x801;
        }
        return 0x7F6;
    }
    if (dComIfGs_isEventBit(0x2A20)) {
        dComIfGs_setEventReg(0xA60F, 0);
        if (mGetReg >= 3) {
            if (mMsgIdx != 0) {
                return 0x7FD;
            }
            mMsgIdx = 1;
            return 0x7FC;
        }
        if (mMsgIdx != 0) {
            return 0x7FA;
        }
        mMsgIdx = 1;
        return 0x7FB;
    }
    return 0x7F1;
}

u32 daNpc_Ba1_c::getMsg_BA1_4() { return getMsg_BA1_3(); }

/* getMsg */
u32 daNpc_Ba1_c::getMsg() {
    switch (mCharType) {
        case 0: return getMsg_BA1_0();
        case 1: return getMsg_BA1_1();
        case 3: return getMsg_BA1_3();
        case 4: return getMsg_BA1_4();
    }
    return 0;
}

/* chkAttention */
bool daNpc_Ba1_c::chkAttention() {
    dAttention_c* attn = dComIfGp_getAttention();
    if (attn->LockonTruth()) {
        return this == attn->LockonTarget(0);
    }
    return this == attn->ActionTarget(0);
}

/* setAttention */
void daNpc_Ba1_c::setAttention(bool i_force) {
    attention_info.position.x = current.pos.x;
    attention_info.position.y = current.pos.y + l_HIO.mPrm.mAttPosOffsetY;
    attention_info.position.z = current.pos.z;
    if (mPartnerID != 0 || i_force) {
        eyePos.x = mAttnBasePos.x;
        eyePos.y = mAttnBasePos.y;
        eyePos.z = mAttnBasePos.z;
        eyePos.y += mAttnPosY;
    }
}

/* searchByID */
fopAc_ac_c* daNpc_Ba1_c::searchByID(fpc_ProcID i_id) {
    fopAc_ac_c* actor = NULL;
    fopAcM_SearchByID(i_id, &actor);
    return actor;
}

/* partner_srch_sub */
bool daNpc_Ba1_c::partner_srch_sub(fpcLyIt_JudgeFunc i_func) {
    bool found = false;
    mPartnerProcID = fpcM_ERROR_PROCESS_ID_e;
    l_check_wrk = 0;
    for (int i = 0; i < 0x14; i++) {
        l_check_inf[i] = NULL;
    }
    fpcEx_Search(i_func, this);
    if (l_check_wrk != 0) {
        mPartnerProcID = l_check_inf[0] != NULL ? fopAcM_GetID(l_check_inf[0]) : fpcM_ERROR_PROCESS_ID_e;
        found = true;
    }
    return found;
}

/* partner_srch */
void daNpc_Ba1_c::partner_srch() {
    if (mPartnerSrchState == 1) {
        mPartnerSrchState += 1;
    }
}

/* check_useFairyArea */
bool daNpc_Ba1_c::check_useFairyArea() {
    // §271 (Foundry): the donor is a 3D DISTANCE check (PSVECSquareMag on the Link↔point
    // delta), NOT a Y-threshold. The §261 form used a guessed 150-unit Y band + XZ distance,
    // which fired MID-LADDER (Link at y≈225 is within 150 of Grandma's loft y≈375). The 3D
    // form includes Y, so a climbing Link stays far and the tale can't trigger until he's at
    // the loft. Float-pool offset {20,-16,0} applied to Grandma's pos; threshold mUseFairyDist1.
    fopAc_ac_c* player = daPy_getPlayerActorClass();
    // ========================================================================
    // §338 DONOR-TRUE (Foundry REL disassembly, retail symbol-map located):
    // the trigger is a CYLINDER, not a sphere — the donor overwrites the delta
    // Y with a rodata 0.0 before the magnitude (XZ-only), then runs a SEPARATE
    // |dY| < 100.0 band test (both constants confirmed from the rel float
    // pool). The §271 3D form was the m2c reconstruction collapsing the
    // cylinder (fired at dY=-359 from the ground floor, §336a/§337 receipts);
    // the mysteriously-zeroed mUseFairyDist0 is that 0.0 Y-replacement
    // constant misread as a second threshold. The §271 {20,-16,0} offset does
    // NOT appear in this function's disassembly — removed (donor uses ba1's
    // position directly); origin of the old offset flagged in §338 notes.
    // Cylinders are the donor's standard trigger idiom (daTag_Ba1_XyCheck).
    // ========================================================================
    cXyz diff = player->current.pos - current.pos;
    const f32 dY = diff.y;
    diff.y = 0.0f;  // the donor's rodata 0.0 Y-replacement → XZ-only magnitude
    f32 dist = JMAFastSqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    const bool fire = dist < l_HIO.mPrm.mUseFairyDist1 && std::fabs(dY) < 100.0f;
#if TARGET_PC
    // §336a TRIGGER-GATE PROBE (user: donor does NOT fire from the 1st floor →
    // the §271 reconstruction is missing a donor gate). Log the full geometry
    // while Link is anywhere near the radius (every 30f + always on fire) so
    // one door-entry run shows the exact dist/Y-delta at the wrongful fire and
    // what additional donor gate (Y? room part? second threshold mUseFairyDist0?)
    // would have blocked it. Strip with §336 acceptance.
    static int s_p336 = 0;
    if (fire || (dist < l_HIO.mPrm.mUseFairyDist1 * 1.5f && (s_p336++ % 30) == 0)) {
        DuskLog.info("[Ba1Tale] §336a gate: dist={:.0f} thr={:.0f} (thr0={:.0f}) dY={:.0f} "
                     "link=({:.0f},{:.0f},{:.0f}) ba1=({:.0f},{:.0f},{:.0f}) tmp310={} fire={}",
                     dist, l_HIO.mPrm.mUseFairyDist1, l_HIO.mPrm.mUseFairyDist0, dY,
                     player->current.pos.x, player->current.pos.y, player->current.pos.z,
                     current.pos.x, current.pos.y, current.pos.z,
                     dComIfGs_isTmpBit(0x310) ? 1 : 0, fire ? 1 : 0);
    }
#endif
    return fire;
}

/* checkCommandTalk */
bool daNpc_Ba1_c::checkCommandTalk() {
    bool ret = false;
    if (eventInfo.mCommand == 1 /* §261 INTALK */) {  // §261 unkF8 == 1
        bool present = dComIfGp_evmng_ChkPresentEnd() != 0;  // §261 gi.unk52B8 in {1,2,3} approx
        ret = present == false;
    }
    return ret;
}

/* charDecide */
bool daNpc_Ba1_c::charDecide(int i_param) {
    mSizeIdx = 0;
    mCharType = 0xFF;
    switch (i_param) {
        case 0: mCharType = 0; return true;
        case 1: mCharType = 1; return true;
        case 2: mCharType = 2; return true;
        case 3: mCharType = 3; return true;
        case 4: mCharType = 4; return true;
        default: return false;
    }
}

// ============================================================
// §261 event-init (eInit_*) handlers
// ============================================================
void daNpc_Ba1_c::eInit_SET_PLYER_GOL_() {
    cXyz off(0.0f, 0.0f, 50.0f);
    cXyz goal;
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(shape_angle.y);
    mDoMtx_stack_c::multVec(&off, &goal);
    dComIfGp_evmng_setGoal(&goal);
}

void daNpc_Ba1_c::eInit_PLYER_INI_POS_() {
    fopAc_ac_c* player = daPy_getPlayerActorClass();
    cXyz off(0.0f, 0.0f, 180.0f);
    cXyz dst;
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(shape_angle.y);
    mDoMtx_stack_c::multVec(&off, &dst);
    daPy_getPlayerActorClass()->setPlayerPosAndAngle(&dst, cLib_targetAngleY(&dst, &current.pos), 0);  // §261 unk31C->unk84
}

void daNpc_Ba1_c::eInit_USE_FAIRY_END_() {
    attention_info.distances[1] = 0x5B;
    attention_info.distances[3] = 0x59;
    m7FC_fairyEnd = 1;
}

void daNpc_Ba1_c::eInit_MOV_POS_() {
    m7F5_shapeLock = 1;
    mpItemModel = NULL;
    m7F7_drawSkip = 1;
    m7FE_inEventMove = 1;
}

void daNpc_Ba1_c::eInit_SET_PLYER_TRN_ANG_() {
    fopAc_ac_c* player = daPy_getPlayerActorClass();
    player->shape_angle.y = cLib_targetAngleY(&player->current.pos, &current.pos);
}

void daNpc_Ba1_c::eInit_ACTOR_DRW_CONTROL_(int* i_arg0, int* i_arg1) {
    if (i_arg0 != NULL) {
        fopAc_ac_c* player = daPy_getPlayerActorClass();
        switch (*i_arg0) {
            case 0: fopAcM_OnStatus(player, fopAcStts_UNK_0x8000000_e); break;
            case 1: fopAcM_OffStatus(player, fopAcStts_UNK_0x8000000_e); break;
        }
    }
    if (i_arg1 != NULL) {
        switch (*i_arg1) {
            case 0: m7F7_drawSkip = 1; return;
            case 1: m7F7_drawSkip = 0; return;
            case 2:
                setAnm_NUM(0xA, 1);
                m7F7_drawSkip = 1;
                return;
            case 3: {
                fopAc_ac_c* player = daPy_getPlayerActorClass();
                shape_angle.y = cLib_targetAngleY(&current.pos, &player->current.pos);
                attention_info.distances[1] = 0xAB;
                attention_info.distances[3] = 0xAB;
                cXyz off(0.0f, 0.0f, 90.0f);
                mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
                mDoMtx_stack_c::YrotM(shape_angle.y);
                mDoMtx_stack_c::multVec(&off, &current.pos);
                m7F7_drawSkip = 0;
                break;
            }
        }
    }
}

void daNpc_Ba1_c::eInit_setEvTimer(int* i_arg) {
    mEvTimer = 0;
    if (i_arg != NULL) {
        mEvTimer = (s16)*i_arg;
    }
}
void daNpc_Ba1_c::eInit_CHK_FAIRY_(int* i_arg) { eInit_setEvTimer(i_arg); }
f32 daNpc_Ba1_c::eInit_prmFloat(float* i_arg, float i_def) { return i_arg != NULL ? *i_arg : i_def; }
void daNpc_Ba1_c::eInit_SET_EYE_OFF_(float* i_arg) { mAttnPosY = eInit_prmFloat(i_arg, 0.0f); }
void daNpc_Ba1_c::eInit_EYE_OFF_ZRO_(float* i_arg) { mEyeOffTarget = eInit_prmFloat(i_arg, 0.0f); }
void daNpc_Ba1_c::eInit_CHK_FAIRY_MOV_1(int* i_arg) { eInit_setEvTimer(i_arg); }

/* event_actionInit */
void daNpc_Ba1_c::event_actionInit(int i_staffId) {
    int* p_actNo = dComIfGp_evmng_getMyIntegerP(i_staffId, "ActNo");
    int* p_prm0 = dComIfGp_evmng_getMyIntegerP(i_staffId, "prm_0");
    int* p_prm1 = dComIfGp_evmng_getMyIntegerP(i_staffId, "prm_1");
    int* p_timer = dComIfGp_evmng_getMyIntegerP(i_staffId, "Timer");
    f32* p_atten = dComIfGp_evmng_getMyFloatP(i_staffId, "Atten");
    f32* p_speed = dComIfGp_evmng_getMyFloatP(i_staffId, "Speed");
    if (p_actNo != NULL) {
        mEvtActionNo = (u8)*p_actNo;
        if ((u32)(s8)mEvtActionNo <= 0xA) {
            switch (mEvtActionNo) {
                case 0: setAnm_NUM(9, 1); return;
                case 1: eInit_SET_PLYER_GOL_(); return;
                case 2: eInit_PLYER_INI_POS_(); return;
                case 3: eInit_USE_FAIRY_END_(); return;
                case 4: eInit_MOV_POS_(); return;
                case 5: eInit_SET_PLYER_TRN_ANG_(); return;
                case 6: eInit_ACTOR_DRW_CONTROL_(p_prm0, p_prm1); return;
                case 7: eInit_CHK_FAIRY_(p_timer); return;
                case 8: eInit_SET_EYE_OFF_(p_atten); return;
                case 9: eInit_EYE_OFF_ZRO_(p_speed); return;
                case 10: eInit_CHK_FAIRY_MOV_1(p_timer); break;
            }
        }
    }
}

// ============================================================
// §261 event-move (eMove_*) handlers
// ============================================================
BOOL daNpc_Ba1_c::eMove_MOV_POS_() {
    if (cLib_calcTimer(&mEvTimer)) {
        return FALSE;
    }
    if ((s32)mAttnPosY != 0) {
        cLib_chaseF(&mAttnPosY, 0.0f, 4.0f /* §261 [INFERENCE-NEEDED] fpool chase */);
        return FALSE;
    }
    fopAc_ac_c* player = daPy_getPlayerActorClass();
    // §261 target = donor fpool offset (-50,120 pair); [INFERENCE-NEEDED]
    cXyz target(-50.0f, current.pos.y, 120.0f);
    s16 tgtAngle = cLib_targetAngleY(&current.pos, &player->current.pos);
    mShapeAngleTmp.y = tgtAngle;
    m7F5_shapeLock = 1;
    bool reached = (abs((s16)(cLib_targetAngleY(&current.pos, &target) - shape_angle.y)) < 0x400);
    if (reached) {
        shape_angle.y = mShapeAngleTmp.y;
        m7F5_shapeLock = 0;
        speedF = 0.0f;
        mAttnPosY = 0.0f;
    } else {
        cLib_addCalcAngleS(&shape_angle.y, cLib_targetAngleY(&current.pos, &target), 4, 0x400, 0);
        cLib_chaseF(&speedF, 4.0f, 0.4f /* §261 [INFERENCE-NEEDED] fpool */);
    }
    return reached ? TRUE : FALSE;
}

BOOL daNpc_Ba1_c::eMove_CHK_FAIRY_() {
    if (!cLib_calcTimer(&mEvTimer)) {
        return partner_srch_sub(searchActor_Fa) == 0 ? TRUE : FALSE;
    }
    return FALSE;
}

BOOL daNpc_Ba1_c::eMove_EYE_OFF_ZRO_() {
    f32 target = mEyeOffTarget;
    if ((s32)target == 0) {
        mAttnPosY = 0.0f;
        return TRUE;
    }
    cLib_chaseF(&mAttnPosY, 0.0f, target);
    if ((s32)mAttnPosY == 0) {
        mAttnPosY = 0.0f;
        return TRUE;
    }
    return FALSE;
}

BOOL daNpc_Ba1_c::eMove_CHK_FAIRY_MOV_1() {
    bool ready = false;
    if (!cLib_calcTimer(&mEvTimer) && partner_srch_sub(searchActor_Fa)) {
        fopAc_ac_c* actor = searchByID(mPartnerProcID);
        if (actor != NULL) {
            ready = false; /* §261 [INFERENCE-NEEDED] partner 0x790 state==5 — no port accessor */ (void)actor;
        }
    }
    if (ready) {
        eInit_PLYER_INI_POS_();
    }
    return ready ? TRUE : FALSE;
}

/* event_action */
BOOL daNpc_Ba1_c::event_action() {
    if ((u32)(s8)mEvtActionNo <= 0xA) {
        switch (mEvtActionNo) {
            case 4: return eMove_MOV_POS_();
            case 7: return eMove_CHK_FAIRY_();
            case 9: return eMove_EYE_OFF_ZRO_();
            case 10: return eMove_CHK_FAIRY_MOV_1();
        }
    }
    return TRUE;
}

/* cut_init_START_TALE1 */
void daNpc_Ba1_c::cut_init_START_TALE1(int i_staffId) {
    int* p_timer = dComIfGp_evmng_getMyIntegerP(i_staffId, "Timer");
    mEvTimer = 0;
    if (p_timer != NULL) {
        mEvTimer = (s16)*p_timer;
    }
}

/* cut_move_START_TALE1 */
BOOL daNpc_Ba1_c::cut_move_START_TALE1() {
    // ============================================================
    // §294: DONOR TWO-STEP restored (covenant correction). The §280–§283 in-place PACKAGE
    // arrangement was a non-donor bridge and is reverted. Grandma's cut runs the donor's
    // own mEvTimer; when it expires it fires the donor stage re-entrance — its own mode-8
    // wipe IS the native fade (wipe out → reload → wipe in) — via setNextStage to the HOST
    // stage name with the donor args VERBATIM, plus the §273 pending id. The re-entrance
    // reloads the host, the §273 entry wire fires setStartDemo, TALE_DEMO plays and finishes
    // on its own donor accounting. (Only the stage NAME is host-mapped; every arg is donor.)
    // ============================================================
    if (!cLib_calcTimer(&mEvTimer)) {
        const char* hostStage = dComIfGp_getStartStageName();  // re-enter the HOST stage
        if (hostStage != NULL) {
            // §297: NO gating — the native spawn-point trigger fires the demo, exactly as the
            // donor does. The re-entrance POINT (0xC8/0xCA) selects the donor's PLYR entry
            // whose param carries getStartEvent; on arrival daAlink (d_a_alink.cpp:5136) reads
            // it → evmng_startDemo → orderStartDemo → the tale. The §273 pending/poll
            // reconstruction is REMOVED (it fired pre-warp). Donor args verbatim; point carries
            // the variant.
            if (ba1_isCameraLetterBox()) {  // §261 [INFERENCE] gi.unk1A0 stage-variant select
                DuskLog.info("[Ba1Tale] §347b START_TALE2 warp ARM (0xCA) — evRun={}",
                             dComIfGp_event_runCheck() ? 1 : 0);
                dComIfGp_setNextStage(hostStage, 0xCA, 0, 8, 0.0f, 0, 1, 0, 0, 0, 0);
            } else {
                DuskLog.info("[Ba1Tale] §347b START_TALE1 warp ARM (0xC8) — evRun={}",
                             dComIfGp_event_runCheck() ? 1 : 0);
                dComIfGp_setNextStage(hostStage, 0xC8, 0, 8, 0.0f, 0, 1, 0, 0, 0, 0);
            }
        }
    }
    return mEvTimer == 0 ? TRUE : FALSE;
}

/* privateCut */
void daNpc_Ba1_c::privateCut(int i_staffId) {
    if (i_staffId == -1) {
        return;
    }
    mCutIdx = dComIfGp_evmng_getMyActIdx(i_staffId, a_cut_tbl, 2, 1, 0);
#if TARGET_PC
    {
        // §284 10-HYPOTHESIS PROBE — tale plays + demo ends (§282 mode2→end) but the
        // EVENT never tears down (Link stuck, no control). Log the candidate holders:
        //  H1 ba1 cut still active (mCutIdx != -1)   H2 ba1 cut ended (mCutIdx == -1)
        //  H3 mEvTimer (cut_move end)   H4 event still running (event_runCheck / runEvt)
        //  H5 demo mode (getMode)   H10 re-order loop (mOrderType)
        //  H7 PACKAGE gone but event lingers ⇒ another staff (CAMERA/Link) holds it
        static int s_p284 = 0;
        if ((s8)mCutIdx == -1 || (s_p284++ % 60) == 0) {
            const char* runEvt = dComIfGp_getEventManager().getRunEventName();
            DuskLog.info(
                "[Ba1Tale] §284 privateCut staff={} mCutIdx={} mEvTimer={} mOrderType={} "
                "demoMode={} eventRun={} runEvt='{}'",
                i_staffId, (int)(s8)mCutIdx, (int)mEvTimer, (int)mOrderType,
                dDemo_c::getMode(), dComIfGp_event_runCheck() ? 1 : 0,
                runEvt != NULL ? runEvt : "(none)");
        }
    }
#endif
    if ((s8)mCutIdx == -1) {
#if TARGET_PC
        DuskLog.info("[Ba1Tale] §284 ba1 cut ENDING (mCutIdx==-1) → cutEnd staff={}", i_staffId);
#endif
        dComIfGp_evmng_cutEnd(i_staffId);
        return;
    }
    if (dComIfGp_evmng_getIsAddvance(i_staffId)) {
        switch (mCutIdx) {
            case 0: event_actionInit(i_staffId); break;
            case 1: cut_init_START_TALE1(i_staffId); break;
        }
    }
    BOOL done;
    switch (mCutIdx) {
        case 0: done = event_action(); break;
        case 1: done = cut_move_START_TALE1(); break;
        default: done = TRUE; break;
    }
    if (done) {
        dComIfGp_evmng_cutEnd(i_staffId);
    }
}

/* endEvent */
void daNpc_Ba1_c::endEvent() {
    /* §261 [INFERENCE-NEEDED] donor set gi.unk52C0|=8 (msg-flow) — no port dEvt_info_c slot; bridged no-op */
    mAnmAtr = 0xFF;
}

/* isEventEntry */
int daNpc_Ba1_c::isEventEntry() {
    return dComIfGp_evmng_getMyStaffId(mEventCut.getActorName(), this, 0);  // §270 NULL→this (Foundry): the port's getMyStaffId needs the actor ptr (ls1:1651). NULL = staff claim fails → ba1 never enters START_TALE1 → event completes at frame 0.
}

/* event_proc */
void daNpc_Ba1_c::event_proc(int i_staffId) {
    const BOOL eventEnded284 = dComIfGp_evmng_endCheck(mEventIdTable[mEventIdx]);
#if TARGET_PC
    {
        // §284b probe: `endCheck(tale_1)` is ba1's exit gate — if it never returns true,
        // ba1 stays IN the event forever and control never returns (the observed strand),
        // even though the demo (§282) ended. Log it against the demo mode + running event
        // so we see whether the event manager thinks the event is done. (H4/H7/H9.)
        static int s_p284b = 0;
        if (!eventEnded284 || (s_p284b++ % 60) == 0) {
            const char* runEvt = dComIfGp_getEventManager().getRunEventName();
            // §294b: the Ba1-staff CURRENT CUT name. If this is NOT "START_TALE1", the donor
            // warp in cut_move_START_TALE1 never dispatches (cutProc handles a generic cut and
            // skips privateCut) → no re-entrance. Tells us whether tale_1's Ba1 cut is the
            // donor's START_TALE1 or something else.
            char* nowCut = dComIfGp_getEventManager().getMyNowCutNameStr(i_staffId);  // §295: Str = full name (getMyNowCutName is 4-byte-swapped for tag match)
            DuskLog.info(
                "[Ba1Tale] §284b event_proc staff={} mEventIdx={} eventEnded(endCheck)={} "
                "demoMode={} mOrderType={} runEvt='{}' nowCut='{}'",
                i_staffId, (int)mEventIdx, eventEnded284 ? 1 : 0, dDemo_c::getMode(),
                (int)mOrderType, runEvt != NULL ? runEvt : "(none)",
                nowCut != NULL ? nowCut : "(null)");
        }
    }
#endif
    if (eventEnded284) {
        switch (mEventIdx) {
            case 0:
                break;
            case 1:
                if (dComIfGs_checkBottle(0x54)) {
                    dComIfGs_setBottleItemIn(0x54, 0x55);
                } else {
                    dComIfGs_setEmptyBottleItemIn(0x55);
                }
                if (dComIfGs_isEventBit(0x2A20)) {
                    mOrderType = 1;
                    m7F0_movDone = 1;
                } else {
                    mOrderType = 5;
                }
                break;
            case 2:
                dLetter_send(0x9D03);
                dComIfGs_onEventBit(0x2A20);
                set_action(&daNpc_Ba1_c::wait_action4, NULL);
                mHomePos = current.pos;
                mHomeAngle = current.angle;
                m7FE_inEventMove = 0;
                break;
        }
        endEvent();
        return;
    }
    if (!mEventCut.cutProc()) {
        privateCut(i_staffId);
    }
    lookBack();
}

/* set_action */
bool daNpc_Ba1_c::set_action(ActionFunc i_action, void* i_param) {
    if (mCurrActionFunc != i_action) {
        if (mCurrActionFunc != NULL) {
            mPartnerSrchState = 9;
            (this->*mCurrActionFunc)(i_param);
        }
        mCurrActionFunc = i_action;
        mPartnerSrchState = 0;
        (this->*mCurrActionFunc)(i_param);
    }
    return true;
}

/* setStt */
void daNpc_Ba1_c::setStt(s8 i_stt) {
    u8 old = mSttNum;
    mSttTimer = 0;
    mSttNum = i_stt;
    switch (mSttNum) {
        case 2:
            mAnmAtr = 0xFF;
            if (mAnmNum != 4) {
                mLookBackMode = 1;
                mMesgAtrOnce = 1;
            }
            mSttNumOld = old;
            return;
        case 5:
            mAnmAtr = 0xFF;
            mSttNumOld = old;
            return;
        default:
            if (mSttNum == 0 || (mSttNum >= 3 && mSttNum < 8) || mSttNum < 0) {
                mAnmChangeFlag = 1;
            }
            setAnm();
            return;
    }
}

// ============================================================
// §261 per-state behavior (wait_*/talk_*/ZZZwai)
// ============================================================
BOOL daNpc_Ba1_c::wait_0() {
    cLib_addCalcAngleS(&shape_angle.y, mHomeAngle.y, 4, 0x800, 0);
    if (mOrderType == 1 || mOrderType >= 3) {
        return TRUE;
    }
    mOrderType = 0;
    if (!dComIfGs_isTmpBit(0x310)) {
        if (check_useFairyArea()) {
            dComIfGs_onTmpBit(0x310);
            mOrderType = 6;
            // §266 ACCEPTANCE PROBE — the proximity trigger fired (→ orders tale_1).
            // Strip after accept.
            DuskLog.info("[Ba1Tale] §266 wait_0 trigger area TRUE -> mOrderType=6 (tale_1)");
        }
    }
    return TRUE;
}

BOOL daNpc_Ba1_c::wait_1() {
    cLib_addCalcAngleS(&shape_angle.y, mHomeAngle.y, 4, 0x800, 0);
    if (mOrderType == 1 || mOrderType >= 3) {
        return TRUE;
    }
    if (mTalkAccepted != 0) {
        if (chk_talk()) {
            setStt(2);
            mAnmChangeFlag = 0;
        }
        return TRUE;
    }
    if (m7F9_fairyReady != 0) {
        setAnm_NUM(4, 1);
        mOrderType = 1;
        mLookBackMode = 0;
        mLookTimer = shape_angle.y - 0x638E;
    } else {
        mOrderType = 2;
        if (m808_attnFlag != 0) {
            mSttTimer = 0x3C;
        }
        if (cLib_calcTimer(&mSttTimer) && chk_drct(61.0f)) {
            mLookBackMode = 1;
        } else {
            mLookBackMode = 3;
            mLookTimer = mHomeAngle.y;
            mMesgAtrOnce = 1;
        }
    }
    return TRUE;
}

BOOL daNpc_Ba1_c::talk_1() {
    BOOL notMoving = chk_partsNotMove();
    if (mAnmNum == 4) {
        cLib_addCalcAngleS(&shape_angle.y, mLookTimer, 2, 0x1000, 1);
        if (mMorfAnimStopped == 0) {
            return TRUE;
        }
    }
    talk(1);
    if (mpCurrMsg != NULL) {
        u16 mode = mpCurrMsg->mode;
        if (mode != fopMsg_MODE_MSG_TYPING_e) {
            switch (mode) {
                case fopMsg_MODE_BOX_OPENING_e:
                    break;
                case fopMsg_MODE_MSG_DESTROYED_e:
                    switch (mCurrMsgNo) {
                        case 0x7E4: m7FB = 1; break;
                        case 0x7E6: dComIfGs_onEventBit(0x608); break;
                        case 0x7E8: dComIfGs_onEventBit(0x601); break;
                        case 0x7EC: dComIfGs_onEventBit(0x602); break;
                        case 0x7F0: dComIfGs_onEventBit(0x740); break;
                        case 0x800:
                            if (mCurrMsgNo != 0x7F9) {
                                mOrderType = 4;
                            }
                            m7FC_fairyEnd = 0;
                            break;
                        case 0x801:
                            m7F0_movDone = 0;
                            break;
                    }
                    mPresentItemNo = 0xFF;
                    mTalkAccepted = 0;
                    setStt(mSttNumOld);
                    mSttTimer = 0x3C;
                    endEvent();
                    m7F9_fairyReady = 0;
                    break;
            }
        }
    }
    return notMoving;
}

BOOL daNpc_Ba1_c::talk_2() { return talk_1(); }

BOOL daNpc_Ba1_c::wait_2() {
    cLib_addCalcAngleS(&shape_angle.y, mHomeAngle.y, 4, 0x800, 0);
    if (mOrderType == 1 || mOrderType >= 3) {
        return TRUE;
    }
    if (mTalkAccepted != 0) {
        if (m7FF_stageChg != 0) {
            dComIfGp_setNextStage("LinkRM", 0xC9, 0, 9, 0.0f, 0, 1, 0, 0, 0, 0);
            return TRUE;
        }
        if (chk_talk()) {
            setStt(2);
            mAnmChangeFlag = 0;
        }
        return TRUE;
    }
    if (!ba1_isCameraShip() && dComIfGs_isEventBit(0x3202)) {  // §261 gi.unkB5
        current.pos.set(-290.0f, 0.0f, 110.0f);  // §261 fpool respawn block
        shape_angle.y = -0x8000;
        mHomePos = current.pos;
        mHomeAngle = current.angle;
        fopAc_ac_c* player = daPy_getPlayerActorClass();
        if ((player->current.pos.y - current.pos.y) < 0.0f /* §261 [INFERENCE-NEEDED] fpool unk70 */) {
            mOrderType = 1;
            m7FF_stageChg = 1;
        }
        mLookBackMode = 3;
        mLookTimer = mHomeAngle.y;
        mMesgAtrOnce = 1;
        return TRUE;
    }
    mOrderType = 2;
    if (m808_attnFlag != 0) {
        mSttTimer = 0x3C;
    }
    if (cLib_calcTimer(&mSttTimer) && chk_drct(61.0f /* §261 fpool unkBC = _5423 */)) {
        mLookBackMode = 1;
    } else {
        mLookBackMode = 3;
        mLookTimer = mHomeAngle.y;
        mMesgAtrOnce = 1;
    }
    return TRUE;
}

BOOL daNpc_Ba1_c::wait_3() { return wait_2(); }

BOOL daNpc_Ba1_c::ZZZwai() {
    if (mOrderType == 1 || mOrderType >= 3) {
        return TRUE;
    }
    if (m7FC_fairyEnd != 0) {
        m7FC_fairyEnd = 0;
        mOrderType = 4;
        return TRUE;
    }
    if (mTalkAccepted != 0) {
        if (chk_talk()) {
            setStt(5);
        }
        return TRUE;
    }
    if (dComIfGs_isEventBit(0x2A20)) {
        mOrderType = 2;
        if (m808_attnFlag != 0) {
            mSttTimer = 0x3C;
        }
        if (cLib_calcTimer(&mSttTimer) && chk_drct(61.0f)) {
            mLookBackMode = 1;
        } else {
            mLookBackMode = 3;
            mLookTimer = mHomeAngle.y;
            mMesgAtrOnce = 1;
        }
    } else {
        mOrderType = 0;
        if (check_useFairyArea()) {
            mOrderType = 2;
        }
    }
    return TRUE;
}

// ============================================================
// §261 action dispatchers (ptmf targets). Each drives partner-srch state then
// the current setStt() sub-state (wait_*/talk_*/ZZZwai) and lookBack().
// ============================================================
int daNpc_Ba1_c::wait_action1(void* i_param) {
    switch (mPartnerSrchState) {
        case 9:
            break;
        case 0:
            if (dComIfGs_isEventBit(0x2A80)) {
                mMsgIdx = dComIfGs_isTmpBit(0x310) != 0;
                if (mMsgIdx != 0) {
                    dComIfGs_offTmpBit(0x310);
                }
                setStt(1);
                mPartnerSrchState += 1;
            } else if (dComIfGs_isTmpBit(0x310)) {
                setStt(1);
                mPartnerSrchState += 1;
            } else {
                setStt(7);
                mPartnerSrchState += 1;
            }
            break;
        default:
            m808_attnFlag = chkAttention();
            switch (mSttNum) {
                case 7: mPartnerID = wait_0(); break;
                case 1: mPartnerID = wait_1(); break;
                case 2: mPartnerID = talk_1(); break;
            }
            lookBack();
            break;
    }
    return 1;
}

int daNpc_Ba1_c::wait_action2(void* i_param) {
    switch (mPartnerSrchState) {
        case 9:
            break;
        case 0:
            setStt(3);
            mPartnerSrchState += 1;
            break;
        default:
            m808_attnFlag = chkAttention();
            switch (mSttNum) {
                case 3: mPartnerID = wait_2(); break;
                case 2: mPartnerID = talk_1(); break;
            }
            lookBack();
            break;
    }
    return 1;
}

int daNpc_Ba1_c::demo_action1(void* i_param) {
    switch (mPartnerSrchState) {
        case 9:
            break;
        case 0:
            mPartnerSrchState += 1;
            break;
    }
    return 1;
}

int daNpc_Ba1_c::wait_action3(void* i_param) {
    switch (mPartnerSrchState) {
        case 9:
            break;
        case 0:
            setStt(4);
            mPartnerSrchState += 1;
            break;
        default:
            m808_attnFlag = chkAttention();
            switch (mSttNum) {
                case 4: mPartnerID = ZZZwai(); break;
                case 5: mPartnerID = talk_2(); break;
            }
            lookBack();
            break;
    }
    return 1;
}

int daNpc_Ba1_c::wait_action4(void* i_param) {
    switch (mPartnerSrchState) {
        case 9:
            break;
        case 0:
            setStt(6);
            mPartnerSrchState += 1;
            break;
        default:
            m808_attnFlag = chkAttention();
            switch (mSttNum) {
                case 6: mPartnerID = wait_3(); break;
                case 2: mPartnerID = talk_1(); break;
            }
            lookBack();
            break;
    }
    return 1;
}

// ============================================================
// §261 R11 (demo NULL) + R2/13 (native demo face via getP_BtpData) + R12 (angle)
// ============================================================
bool daNpc_Ba1_c::demo() {
    if (demoActorID == 0) {
        if (m80B_inDemo != 0) {
            m80B_inDemo = 0;
        }
    } else {
        m80B_inDemo = 1;
        dDemo_actor_c* demo_actor = dComIfGp_demo_getActor(demoActorID);  // §261 R11 native demo actor
        if (mpBtpRes != NULL) {
            mBtpFrame += 1;
            if (mBtpFrame >= mpBtpRes->getFrameMax()) {
                mBtpFrame = mpBtpRes->getFrameMax();
            }
        }
        J3DAnmTexPattern* demo_btp = demo_actor->getP_BtpData(mArcName);  // §261/§252 native face
        if (demo_btp != NULL) {
            mpBtpRes = demo_btp;
            if (mBtpAnm.init(mpMorf->getModel()->getModelData(), mpBtpRes, 1, 2, 1.0f, 0, -1)) {
                mBtpNum = 0xC;
                mBtpFrame = 0;
            }
        }
        dDemo_setDemoData(this, 0x6A, mpMorf, mArcName, 0, NULL, 0, 0);  // §261 port 8-arg arity
        mShapeAngleTmp = current.angle;  // §261 R12 cutscene orientation
        shape_angle = mShapeAngleTmp;
    }
    return m80B_inDemo;
}

/* shadowDraw — §261 ls1 shadow precedent (800/40/+150/1.0) */
void daNpc_Ba1_c::shadowDraw() {
    cXyz pos(current.pos.x, current.pos.y + 150.0f, current.pos.z);
    mShadowId = dComIfGd_setShadow(
        mShadowId, 1, mpMorf->getModel(), &pos,
        800.0f, 40.0f, current.pos.y, mObjAcch.GetGroundH(),
        mObjAcch.m_gnd, &tevStr, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex());
}

/* _draw — §261 R2/13 render order: btp entry BEFORE entryDL, then removeTexNoAnimator */
BOOL daNpc_Ba1_c::_draw() {
    J3DModel* model = mpMorf->getModel();
    J3DModelData* modelData = model->getModelData();
    if (m7F4 != 0 || m7F7_drawSkip != 0) {
        return TRUE;
    }
    // §266 RENDER FIX (Recipe 2/13, the Aryll/ls1 precedent MISSED for ba1). setMtx's
    // mpMorf->calc() computes the ANIM matrices; mpMorf->modelCalc() computes the
    // RENDER (world draw) matrices from the base transform. Without it the draw matrices
    // are never composed → the whole model draws at the world origin (0,0,0) = "all over
    // the place" (probe §266 read every joint at 0,0,0). modelCalc() MUST precede the
    // btp entry (calc-after-btp resets the texture pattern → blank face). zl1 does exactly
    // this at d_a_npc_zl1.cpp:2624.
    mpMorf->modelCalc();
    // §405: WW feeder — receiver settingTevStruct + donor tevstr TevColor/TevKColor
    // tail (TP never writes those fields on the actor path; §404's copy read black).
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);  // §266 was 0 (too bright); zl1 uses TEV_TYPE_ACTOR
    dKyWw_setLightTevColorType(model, &tevStr);
    mBtpAnm.entry(modelData, (s16)mBtpFrame);
    mpMorf->entryDL();
    modelData->removeTexNoAnimator(mpBtpRes);
    if (mpItemModel != NULL) {
        dKyWw_setLightTevColorType(mpItemModel, &tevStr);
        mDoExt_modelEntryDL(mpItemModel);
    }
    shadowDraw();
    dSnap_RegistFig(0x4A, this, 1.0f, 1.0f, 1.0f);
    return TRUE;
}

/* _execute */
BOOL daNpc_Ba1_c::_execute() {
    if (m800_initDone == 0) {
        mHomePos = current.pos;
        mHomeAngle = current.angle;
        m800_initDone = 1;
    }
    m_jnt.setParam(
        l_HIO.mPrm.mMaxBackboneX, l_HIO.mPrm.mMaxBackboneY, l_HIO.mPrm.mMinBackboneX, l_HIO.mPrm.mMinBackboneY,
        l_HIO.mPrm.mMaxHeadX, l_HIO.mPrm.mMaxHeadY, l_HIO.mPrm.mMinHeadX, l_HIO.mPrm.mMinHeadY,
        l_HIO.mPrm.mMaxTurnStep);
    if (m7F4 != 0 && demoActorID == 0) {
        return TRUE;
    }
    m7F6_posMoveSkip = 0;
    m7F4 = 0;
    partner_srch();
    checkOrder();
    if (!demo()) {
        s32 staffId = -1;
        if (dComIfGp_event_runCheck() && !checkCommandTalk()) {
            staffId = isEventEntry();
        }
        if (staffId >= 0) {
            mPartnerID = 1;
            event_proc(staffId);
        } else {
            (this->*mCurrActionFunc)(NULL);
        }
        if (m7F6_posMoveSkip == 0) {
            fopAcM_posMoveF(this, mStts.GetCCMoveP());
        }
        if (m7F5_shapeLock == 0) {
            mShapeAngleTmp = current.angle;
            shape_angle = mShapeAngleTmp;
        }
    }
    eventOrder();
    setMtx(false);
    if (m80B_inDemo == 0 && m7FE_inEventMove == 0) {
        f32 radius = 50.0f;  // §261 [INFERENCE-NEEDED] fpool collision radius (unk9C / unk74)
        if (m7FD != 0) {
            radius = 30.0f;   // §261 [INFERENCE-NEEDED] fpool unk74
        }
        setCollision(radius, 110.0f);  // §261 height fpool unk10 = 110
    }
    return TRUE;
}

/* _delete */
BOOL daNpc_Ba1_c::_delete() {
    dComIfG_resDelete(&mPhs, mArcName);
    if (heap != NULL) {
        if (mpMorf != NULL) {
            dExtNpcBm1_stopZelAnime(mpMorf);
        }
    }
    return TRUE;
}

/* CheckCreateHeap */
static BOOL CheckCreateHeap(fopAc_ac_c* i_this) {
    return static_cast<daNpc_Ba1_c*>(i_this)->CreateHeap();
}

/* _create — R14: all models from own arc "Ba" */
cPhs_Step daNpc_Ba1_c::_create() {
    fopAcM_ct(this, daNpc_Ba1_c);
    strcpy(mArcName, "Ba");  // §261 R14 own-arc (stringBase0 +0x35)
    cPhs_Step state = dComIfG_resLoad(&mPhs, mArcName);
    if (state != cPhs_COMPLEATE_e) {
        return state;
    }
    if (!charDecide((u8)fopAcM_GetParam(this))) {
        return cPhs_ERROR_e;
    }
    if (!fopAcM_entrySolidHeap(this, CheckCreateHeap, a_size_tbl[mSizeIdx])) {
        return cPhs_ERROR_e;
    }
    fopAcM_SetMtx(this, mpMorf->getModel()->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, -70.0f, -20.0f, -70.0f, 70.0f, 220.0f, 70.0f);  // §261 [INFERENCE-NEEDED] fpool cull dims (bm1 sibling values)
    if (!createInit()) {
        return cPhs_ERROR_e;
    }
    return state;
}

/* create_Anm — R14 own-arc; models by resource ID (donor getIDRes) */
J3DModelData* daNpc_Ba1_c::create_Anm() {
    // §266 CRASH FIX — DN-3 / Recipe-1 (zl1/bm1 precedent). The donor sources the
    // body model + base anim by NUMERIC res id (0xA / 7), but the port's ADAPTED
    // Ba arc does not preserve the donor res table: getObjectIDRes(0xA) returns a
    // raw/unparsed pointer (non-NULL garbage), so McaMorf::create derefs
    // getMaterialNodePointer(0) on it → access violation (m_Do_ext.cpp:1314).
    // Every other native NPC (zl1 "zl.bdl", bm1 "bm.bdl") sources BY FILENAME via
    // the parse-at-consume path. Do the same; NULL-guard → FALSE like the siblings.
    J3DModelData* a_mdl_dat = dExtNpcMount_acquireModelData(mArcName, "ba.bdl");
    if (a_mdl_dat == NULL) {
        return NULL;
    }
    // §266 RENDER-EXPLOSION FIX — match the WORKING zl1/bm1 McaMorf pattern. zl1 builds
    // with a NULL create-time anim + EMode_NULL (d_a_npc_zl1.cpp:2776) and lets setAnm→
    // dNpc_setAnmIDRes bind the real anim afterward. ba1's donor bound an anim here
    // (EMode_LOOP + flags 0x11020022), but the PORT's McaMorf with a create-time anim
    // exploded the skeleton ("model all over the place") while zl1's NULL-anim path with
    // flags 0x11020222 renders clean — the model+anim are proven compatible (the mount
    // stand-in rendered ba.bdl+wait01.bck fine). Defer the anim to setAnm like the siblings.
    mpMorf = new mDoExt_McaMorf(a_mdl_dat, NULL, NULL, NULL,
                                J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1, 1, NULL,
                                0x80000, 0x11020222);
    if (mpMorf == NULL) {
        return NULL;
    }
    if (mpMorf->getModel() == NULL) {
        mpMorf = NULL;
        return NULL;
    }
    m_hed_jnt_num = a_mdl_dat->getJointName()->getIndex("head");
    JUT_ASSERT(0xB20, m_hed_jnt_num >= 0);
    m_bbone_jnt_num = a_mdl_dat->getJointName()->getIndex("backbone");
    JUT_ASSERT(0xB23, m_bbone_jnt_num >= 0);
    m_footL_jnt_num = a_mdl_dat->getJointName()->getIndex("footL");
    JUT_ASSERT(0xB26, m_footL_jnt_num >= 0);
    return a_mdl_dat;
}

/* create_itm_Mdl */
bool daNpc_Ba1_c::create_itm_Mdl() {
    // §266 CRASH FIX (same DN-3 class as create_Anm). Donor res-id 9 = the carried
    // clothes bundle, but in the mod that prop is the Vfuku CROSS-ARC model
    // (npc_ba.ini attach_model=vfuku.bdl) — it is NOT in the Ba arc at a matching
    // res id, so raw getObjectIDRes(9) → J3DModel__create deref crash. Native
    // cross-arc bundle sourcing is tracked debt; skip the item model for now so
    // native ba1 spawns without crashing. mpItemModel stays NULL — every consumer
    // guards it (calc/_draw/setLightTevColorType at lines 330/1427), so no draw
    // path faults. TODO: source vfuku.bdl from the Vfuku arc via acquireModelData
    // (Engine cross-arc §139) to restore the cradled bundle for the tale give.
    mpItemModel = NULL;
    return true;
}

/* CreateHeap */
BOOL daNpc_Ba1_c::CreateHeap() {
    J3DModelData* anm_model = create_Anm();
    if (anm_model == NULL) {
        return FALSE;
    }
    mBtpNum = 1;
    if (!iniTexPttrnAnm(false)) {
        mpMorf = NULL;
        return FALSE;
    }
    if (create_itm_Mdl()) {
        // §266 node callback re-enabled — the bisect exonerated it; the real bug was the
        // missing mpMorf->modelCalc() in _draw (Recipe 2/13). Head/backbone look control.
        for (u16 i = 0; i < anm_model->getJointNum(); i++) {
            if (i == m_hed_jnt_num || i == m_bbone_jnt_num) {
                mpMorf->getModel()->getModelData()->getJointNodePointer(i)->setCallBack(nodeCallBack_Ba1);
            }
        }
        mpMorf->getModel()->setUserArea((uintptr_t)this);
        mAcchCir.SetWall(30.0f, 50.0f);
        mObjAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1, &mAcchCir, fopAcM_GetSpeed_p(this), NULL, NULL);
        return TRUE;
    }
    mpMorf = NULL;
    return FALSE;
}

// ============================================================
// §261 actor method table + profile (donor g_profile_NPC_BA1, §260 receipt)
// ============================================================
static cPhs_Step daNpc_Ba1_Create(fopAc_ac_c* i_this) {
    return ((daNpc_Ba1_c*)i_this)->_create();
}
static BOOL daNpc_Ba1_Delete(daNpc_Ba1_c* i_this) {
    return ((daNpc_Ba1_c*)i_this)->_delete();
}
static BOOL daNpc_Ba1_Execute(daNpc_Ba1_c* i_this) {
    return ((daNpc_Ba1_c*)i_this)->_execute();
}
static BOOL daNpc_Ba1_Draw(daNpc_Ba1_c* i_this) {
    return ((daNpc_Ba1_c*)i_this)->_draw();
}
static BOOL daNpc_Ba1_IsDelete(daNpc_Ba1_c*) {
    return TRUE;
}

static actor_method_class l_daNpc_Ba1_Method = {
    (process_method_func)daNpc_Ba1_Create,
    (process_method_func)daNpc_Ba1_Delete,
    (process_method_func)daNpc_Ba1_Execute,
    (process_method_func)daNpc_Ba1_IsDelete,
    (process_method_func)daNpc_Ba1_Draw,
};

actor_process_profile_definition g_profile_NPC_BA1 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_NPC_BA1_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daNpc_Ba1_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_NPC_BA1_e,
    /* Actor SubMtd */ &l_daNpc_Ba1_Method,
    /* Status       */ 0x08 | fopAcStts_SHOWMAP_e | fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_NPC_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
