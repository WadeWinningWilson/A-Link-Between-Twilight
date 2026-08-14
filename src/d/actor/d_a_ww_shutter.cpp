// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_shutter.cpp
// KIT-DONOR: d/actor/d_a_shutter2.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW SHUTTER TRANSITION ACTORS — the transition family's first port
// (docs/WW Linked/interior-entrance-transitions.md; order ruled shutter/2
// first, tale §783/§792). V9 gate: knob00/shutter/shutter2 MATCHED+clean —
// this port proceeds on donor truth.
//
// TU CONSOLIDATION (§368 convention, donor TU sections marked):
//   [1] d_a_shutter.cpp   — Htobi1/Htobi2, the 2-panel horizontal slider
//   [2] d_a_shutter2.cpp  — Htobi3, the single vertical portcullis
//
// SEAMS:
//   [S1] DN-1 (read before writing this): BG registration follows the
//        plank_span idiom EXACTLY — dComIfG_Bgsp().Regist(bgw, this),
//        NO SetRoomId anywhere; the actor's own placement roomNo (real
//        interior room, disc DZR) is the identity. Fail-loud on Regist.
//   [S2] donor daShutter2_c rode WW's dMoveBG_c base (MoveBGCreate/...);
//        the receiver has no such base — flattened to direct dBgW
//        management, the [1] section's own shape. Behavior verbatim.
//   [S3] getEventIdx: receiver signature takes the actor
//        (dComIfGp_getEventManager().getEventIdx(this, name, 0xff) — the
//        ported knob00's idiom at d_a_knob00.cpp:1379).
//   [S4] SEs (JA_SE_OBJ_B_SHUTTER_* / WDUN_R04_STR_OP) — warn-once until
//        the JA1 SE surface serves actor SEs (the [I4] class).
//   [S5] evmng accessors dispatch WW-side on host stages by construction
//        (§423 A4) — the shutter staff cuts (WAIT/OPEN/CLOSE...) run the
//        donor's own event semantics. This actor MANS its staff: the §753
//        unmanned-staff stall class cannot recur for Htobi doors.
// ============================================================

#include "d/d_com_inf_game.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/d_bg_s_acch.h"
#include "d/d_kankyo_ww.h"   // §406 WW feeders + TEV_TYPE_*
#include "d/d_ext_ww_actor_shims.h"
#include "d/d_ext_npc_mount.h"   // §811 DN-3 consume-time model acquirer (by-index form)
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "dusk/logging.h"

// ============================================================
// §368-style shared helpers
// ============================================================
static void wwShutter_seOwed(const char* i_which) {
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.info("[WwShutter] [S4] SE owed ({}) — silent until the JA1 SE surface", i_which);
    }
}

// ============================================================
// [1] donor d_a_shutter.cpp — Htobi1/Htobi2 (2-panel horizontal slider)
// ============================================================
class daWwShutter_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    BOOL Create();
    BOOL CreateHeap();
    bool _delete();
    bool _execute();
    bool _draw();
    void set_mtx();
    void demo();
    void shutter_move();

    static const f32 m_max_speed[2];
    static const f32 m_min_speed[2];
    static const f32 m_move_len[2];
    static const f32 m_width[2];
    static const s16 m_bdlidx[2];
    static const s16 m_dzbidx[2];
    static const s32 m_heapsize[2];
    static const Vec m_cull_min[2];
    static const Vec m_cull_max[2];
    static const char* m_arcname[2];
    static const char* m_open_ev_name[2];
    static const char* m_close_ev_name[2];
    static const char* m_staff_name[2];

    request_of_phase_process_class mPhs;
    J3DModel* mpModel[2];
    dBgW* mdBgW[2];
    Mtx mMtx[2];
    cXyz mcXyz[2];
    int mType;
    int mSwitchNo;
    int mOpenEventIdx;
    int mCloseEventIdx;
    int mStaffId;
    s16 mFrameTimer;
    u8 mTimer;
    u8 mDemoState;    // donor field_0x320
    u8 mSeLatch;      // donor field_0x339
    u8 mOpenFrames;   // donor field_0x33A
    u8 mbIsSwitch;
};

const f32 daWwShutter_c::m_max_speed[2] = {3.0f, 3.0f};
const f32 daWwShutter_c::m_min_speed[2] = {1.0f, 1.0f};
const f32 daWwShutter_c::m_move_len[2] = {200.0f, 84.0f};
const f32 daWwShutter_c::m_width[2] = {220.0f, 82.0f};
const s16 daWwShutter_c::m_bdlidx[2] = {4, 4};   // dRes_INDEX_HTOBI1/2_BDL (GZLE01)
const s16 daWwShutter_c::m_dzbidx[2] = {7, 7};   // dRes_INDEX_HTOBI1/2_DZB
const s32 daWwShutter_c::m_heapsize[2] = {0x1140, 0x3000};
const Vec daWwShutter_c::m_cull_min[2] = {{-500.0f, -100.0f, -50.0f}, {-150.0f, -100.0f, -50.0f}};
const Vec daWwShutter_c::m_cull_max[2] = {{500.0f, 250.0f, 50.0f}, {150.0f, 250.0f, 50.0f}};
const char* daWwShutter_c::m_arcname[2] = {"Htobi1", "Htobi2"};
const char* daWwShutter_c::m_open_ev_name[2] = {"HYSDOOROPEN", "R03DOOROPEN"};
const char* daWwShutter_c::m_close_ev_name[2] = {NULL, "R03DOORCLOSE"};
const char* daWwShutter_c::m_staff_name[2] = {"Htobi1", "Htobi2"};

namespace daWwShutter_prm {
inline u32 getType(daWwShutter_c* i_this) { return fopAcM_GetParam(i_this) & 0x1; }
inline u32 getSwitchNo(daWwShutter_c* i_this) { return (fopAcM_GetParam(i_this) >> 8) & 0xFF; }
}  // namespace daWwShutter_prm

bool daWwShutter_c::_delete() {
    dComIfG_resDelete(&mPhs, m_arcname[mType]);
    for (int i = 0; i < 2; i++) {
        if (mdBgW[i] != NULL) {
            dComIfG_Bgsp().Release(mdBgW[i]);
        }
    }
    return TRUE;
}

static BOOL daWwShutter_CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daWwShutter_c*)i_this)->CreateHeap();
}

BOOL daWwShutter_c::CreateHeap() {
    // §811 DN-3: consume-time acquire — a getObjectRes return is the RAW
    // buffer for WW BDLs; the raw cast was the §810-2 crash class.
    J3DModelData* modelData =
        dExtNpcMount_acquireModelDataByIndex(m_arcname[mType], (int)m_bdlidx[mType]);
    if (modelData == NULL) {
        DuskLog.warn("[WwShutter] arc '{}' BDL idx {} unresolvable", m_arcname[mType],
                     (int)m_bdlidx[mType]);
        return FALSE;
    }
    for (int i = 0; i < 2; i++) {
        mpModel[i] = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
        if (mpModel[i] == NULL) {
            return FALSE;
        }
        mdBgW[i] = new dBgW();
        if (mdBgW[i] == NULL) {
            return FALSE;
        }
        cBgD_t* pData = (cBgD_t*)dComIfG_getObjectRes(m_arcname[mType], (int)m_dzbidx[mType]);
        if (mdBgW[i]->Set(pData, cBgW::MOVE_BG_e, &mMtx[i]) == true) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL daWwShutter_c::Create() {
    fopAcM_SetMtx(this, mpModel[0]->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, m_cull_min[mType].x, m_cull_min[mType].y, m_cull_min[mType].z,
                          m_cull_max[mType].x, m_cull_max[mType].y, m_cull_max[mType].z);
    mSwitchNo = (int)daWwShutter_prm::getSwitchNo(this);
    mcXyz[0].set(-m_width[mType] / 2.0f, 0.0f, 0.0f);
    mcXyz[1].set(m_width[mType] / 2.0f, 0.0f, 0.0f);
    if (!fopAcM_isSwitch(this, mSwitchNo)) {
        // closed at create (donor field_0x328=3 bookkeeping is state-only)
    } else {
        mcXyz[0].x -= m_move_len[mType];
        mcXyz[1].x += m_move_len[mType];
    }
    mFrameTimer = 30;
    mDemoState = 0;
    mSeLatch = 0;
    mOpenFrames = 0;
    mStaffId = -1;
    set_mtx();
    for (int i = 0; i < 2; i++) {
        // [S1] DN-1: Regist against THIS (real placement room), no SetRoomId;
        // fail-loud per the plank_span idiom (d_a_ext_plank_span.cpp:560).
        if (dComIfG_Bgsp().Regist(mdBgW[i], this)) {
            DuskLog.warn("[WwShutter] [S1] bgW Regist FAILED (panel {}) — door not solid", i);
        }
        mdBgW[i]->Move();
    }
    if (m_open_ev_name[mType] != NULL) {
        mOpenEventIdx = dComIfGp_getEventManager().getEventIdx(this, m_open_ev_name[mType], 0xff);
    } else {
        mOpenEventIdx = -1;
    }
    if (m_close_ev_name[mType] != NULL) {
        mCloseEventIdx = dComIfGp_getEventManager().getEventIdx(this, m_close_ev_name[mType], 0xff);
    } else {
        mCloseEventIdx = -1;
    }
    mbIsSwitch = fopAcM_isSwitch(this, mSwitchNo);
    return TRUE;
}

cPhs_Step daWwShutter_c::_create() {
    fopAcM_ct(this, daWwShutter_c);
    mType = (int)daWwShutter_prm::getType(this);
    cPhs_Step result = dComIfG_resLoad(&mPhs, m_arcname[mType]);
    if (result == cPhs_COMPLEATE_e) {
        if (!fopAcM_entrySolidHeap(this, daWwShutter_CheckCreateHeap, (u32)m_heapsize[mType])) {
            return cPhs_ERROR_e;
        }
        Create();
    }
    return result;
}

void daWwShutter_c::set_mtx() {
    cXyz off;
    for (int i = 0; i < 2; i++) {
        mDoMtx_stack_c::YrotS(current.angle.y);
        mDoMtx_stack_c::multVec(&mcXyz[i], &off);
        mpModel[i]->setBaseScale(scale);
        mDoMtx_stack_c::transS(current.pos.x + off.x, current.pos.y + off.y,
                               current.pos.z + off.z);
        mDoMtx_stack_c::ZXYrotM(current.angle.x, current.angle.y, current.angle.z);
        mpModel[i]->setBaseTRMtx(mDoMtx_stack_c::get());
        cMtx_copy(mDoMtx_stack_c::get(), mMtx[i]);
    }
}

bool daWwShutter_c::_execute() {
    if (mFrameTimer >= 0) {
        mFrameTimer--;
    }
    demo();
    set_mtx();
    for (int i = 0; i < 2; i++) {
        mdBgW[i]->Move();
    }
    mbIsSwitch = fopAcM_isSwitch(this, mSwitchNo);
    return TRUE;
}

void daWwShutter_c::shutter_move() {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[] = {"WAIT", "WAIT02", "OPEN", "CLOSE"};
    enum { ACT_WAIT, ACT_WAIT02, ACT_OPEN, ACT_CLOSE };
    int actionIndex = dComIfGp_evmng_getMyActIdx(
        mStaffId, (DUSK_CONST char* DUSK_CONST*)action_table, 4, FALSE, 0);

    f32 maxVel = m_max_speed[mType];
    f32 minVel = m_min_speed[mType];
    f32 a, b;

    switch (actionIndex) {
    case ACT_WAIT:
        mTimer = 0xF;
        mOpenFrames = 0;
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    case ACT_WAIT02:
        if (cLib_calcTimer(&mTimer) == 0) {
            mSeLatch = 0;
            dComIfGp_evmng_cutEnd(mStaffId);
        }
        break;
    case ACT_OPEN:
        mOpenFrames++;
        if (mSeLatch == 0) {
            mSeLatch = 1;
            wwShutter_seOwed("B_SHUTTER_OPEN");   // [S4] donor gates on SaveTbl==5
        }
        a = cLib_addCalc(&mcXyz[0].x, -m_move_len[mType] - m_width[mType] / 2.0f, 0.1f, maxVel, minVel);
        b = cLib_addCalc(&mcXyz[1].x, m_move_len[mType] + m_width[mType] / 2.0f, 0.1f, maxVel, minVel);
        if (mOpenFrames == 75) {
            wwShutter_seOwed("B_SHUTTER_STOP+shock");   // [S4] SE + donor StartShock
        }
        if (a != 0.0f || b != 0.0f) {
            break;
        }
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    case ACT_CLOSE:
        if (mSeLatch == 0) {
            mSeLatch = 1;
        }
        a = cLib_addCalc(&mcXyz[0].x, -m_width[mType] / 2.0f, 0.1f, maxVel, minVel);
        b = cLib_addCalc(&mcXyz[1].x, m_width[mType] / 2.0f, 0.1f, maxVel, minVel);
        if (a != 0.0f || b != 0.0f) {
            break;
        }
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    default:
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    }
}

void daWwShutter_c::demo() {
    u8 isSwitch = fopAcM_isSwitch(this, mSwitchNo);
    if (mDemoState == 0) {
        if (isSwitch != mbIsSwitch && mFrameTimer < 0) {
            mDemoState = isSwitch ? 1 : 2;
        }
    }
    if (eventInfo.checkCommandDemoAccrpt()) {
        if (dComIfGp_evmng_startCheck(mOpenEventIdx) && mDemoState == 1) {
            mDemoState = 0;
        }
        if (dComIfGp_evmng_startCheck(mCloseEventIdx) && mDemoState == 2) {
            mDemoState = 0;
        }
        if (dComIfGp_evmng_endCheck(mOpenEventIdx) || dComIfGp_evmng_endCheck(mCloseEventIdx)) {
            dComIfGp_event_reset();
        }
        mStaffId = dComIfGp_evmng_getMyStaffId(m_staff_name[mType], NULL, 0);
        shutter_move();   // [S5] the staff is MANNED — §753's stall class closed
    } else if (mDemoState == 1 && mOpenEventIdx > 0) {
        fopAcM_orderOtherEventId(this, (s16)mOpenEventIdx, 0xFF, -1, 0, 1);
        eventInfo.onCondition(dEvtCnd_CANDEMO_e /* donor UNK2 = bit 0x2 */);
    } else if (mDemoState == 2 && mCloseEventIdx > 0) {
        fopAcM_orderOtherEventId(this, (s16)mCloseEventIdx, 0xFF, -1, 0, 1);
        eventInfo.onCondition(dEvtCnd_CANDEMO_e /* donor UNK2 = bit 0x2 */);
    }
}

bool daWwShutter_c::_draw() {
    cXyz pos;
    for (int i = 0; i < 2; i++) {
        pos = current.pos + mcXyz[i];
        dKyWw_settingTevStruct(TEV_TYPE_BG0, &pos, &tevStr);   // §406 WW feeders
        dKyWw_setLightTevColorType(mpModel[i], &tevStr);
        dComIfGd_setListBG();
        mDoExt_modelUpdateDL(mpModel[i]);
        dComIfGd_setList();
    }
    return TRUE;
}

static cPhs_Step daWwShutter_Create(fopAc_ac_c* i_this) {
    return ((daWwShutter_c*)i_this)->_create();
}
static BOOL daWwShutter_Delete(daWwShutter_c* i_this) { return i_this->_delete(); }
static BOOL daWwShutter_Execute(daWwShutter_c* i_this) { return i_this->_execute(); }
static BOOL daWwShutter_IsDelete(daWwShutter_c*) { return TRUE; }
static BOOL daWwShutter_Draw(daWwShutter_c* i_this) { return i_this->_draw(); }

static actor_method_class l_daWwShutter_Method = {
    (process_method_func)daWwShutter_Create,
    (process_method_func)daWwShutter_Delete,
    (process_method_func)daWwShutter_Execute,
    (process_method_func)daWwShutter_IsDelete,
    (process_method_func)daWwShutter_Draw,
};

actor_process_profile_definition g_profile_WW_SHUTTER = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0003,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_WW_SHUTTER_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daWwShutter_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_E_RD_e,
    /* Actor SubMtd */ &l_daWwShutter_Method,
    /* Status       */ fopAcStts_CULL_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

// ============================================================
// [2] donor d_a_shutter2.cpp — Htobi3 (single vertical portcullis)
// [S2] donor's dMoveBG_c base flattened to the [1] section's direct-dBgW
// shape: Regist/Move/Release inline, behavior verbatim.
// ============================================================
class daWwShutter2_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    BOOL Create();
    BOOL CreateHeap();
    bool _delete();
    bool _execute();
    bool _draw();
    void set_mtx();
    void demo();
    void shutter_move();

    request_of_phase_process_class mPhs;
    J3DModel* mpModel;
    dBgW* mpBgW;
    Mtx mMtx;
    int mSwitchNo;
    int mOpenEventIdx;
    int mCloseEventIdx;
    int mStaffId;
    u8 mDemoState;
    u8 mbIsSwitch;
    u8 mbIsNearEnemy;
};

static const f32 kSh2MaxSpeed = 3.0f;
static const f32 kSh2MinSpeed = 1.0f;
static const f32 kSh2MoveLen = 350.0f;
static const s16 kSh2BdlIdx = 4;   // dRes_INDEX_HTOBI3_BDL (GZLE01)
static const s16 kSh2DzbIdx = 7;   // dRes_INDEX_HTOBI3_DZB
static const u32 kSh2HeapSize = 0x0C00;

namespace daWwShutter2_prm {
inline u32 getSwitchNo(daWwShutter2_c* i_this) { return (fopAcM_GetParam(i_this) >> 8) & 0xFF; }
}  // namespace daWwShutter2_prm

bool daWwShutter2_c::_delete() {
    dComIfG_resDelete(&mPhs, "Htobi3");
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
    }
    return TRUE;
}

static BOOL daWwShutter2_CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daWwShutter2_c*)i_this)->CreateHeap();
}

BOOL daWwShutter2_c::CreateHeap() {
    // §811 DN-3: consume-time acquire (same correction as daWwShutter_c above).
    J3DModelData* modelData = dExtNpcMount_acquireModelDataByIndex("Htobi3", (int)kSh2BdlIdx);
    if (modelData == NULL) {
        DuskLog.warn("[WwShutter2] Htobi3 BDL idx {} unresolvable", (int)kSh2BdlIdx);
        return FALSE;
    }
    mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
    if (mpModel == NULL) {
        return FALSE;
    }
    mpBgW = new dBgW();
    if (mpBgW == NULL) {
        return FALSE;
    }
    cBgD_t* pData = (cBgD_t*)dComIfG_getObjectRes("Htobi3", (int)kSh2DzbIdx);
    if (mpBgW->Set(pData, cBgW::MOVE_BG_e, &mMtx) == true) {
        return FALSE;
    }
    return TRUE;
}

BOOL daWwShutter2_c::Create() {
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, -300.0f, -10.0f, -40.0f, 300.0f, 400.0f, 40.0f);
    mSwitchNo = (int)daWwShutter2_prm::getSwitchNo(this);
    mbIsNearEnemy = fopAcM_myRoomSearchEnemy(fopAcM_GetRoomNo(this)) == NULL;
    if ((mSwitchNo != 0xFF && fopAcM_isSwitch(this, mSwitchNo)) ||
        (mSwitchNo == 0xFF && mbIsNearEnemy)) {
        current.pos.y = home.pos.y + kSh2MoveLen;   // starts OPEN
    }
    mbIsSwitch = fopAcM_isSwitch(this, mSwitchNo);
    mDemoState = 0;
    mStaffId = -1;
    set_mtx();
    // [S1]/[S2]: Regist against THIS, no SetRoomId, fail-loud.
    if (dComIfG_Bgsp().Regist(mpBgW, this)) {
        DuskLog.warn("[WwShutter2] [S1] bgW Regist FAILED — portcullis not solid");
    }
    mpBgW->Move();
    mOpenEventIdx = dComIfGp_getEventManager().getEventIdx(this, "R04DOOROPEN", 0xff);
    mCloseEventIdx = dComIfGp_getEventManager().getEventIdx(this, "R04DOORCLOSE", 0xff);
    return TRUE;
}

cPhs_Step daWwShutter2_c::_create() {
    fopAcM_ct(this, daWwShutter2_c);
    cPhs_Step result = dComIfG_resLoad(&mPhs, "Htobi3");
    if (result == cPhs_COMPLEATE_e) {
        if (!fopAcM_entrySolidHeap(this, daWwShutter2_CheckCreateHeap, kSh2HeapSize)) {
            return cPhs_ERROR_e;
        }
        Create();
    }
    return result;
}

void daWwShutter2_c::set_mtx() {
    mpModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    cMtx_copy(mDoMtx_stack_c::get(), mMtx);
}

bool daWwShutter2_c::_execute() {
    demo();
    set_mtx();
    mpBgW->Move();
    mbIsSwitch = fopAcM_isSwitch(this, mSwitchNo);
    mbIsNearEnemy = fopAcM_myRoomSearchEnemy(fopAcM_GetRoomNo(this)) == NULL;
    return TRUE;
}

void daWwShutter2_c::shutter_move() {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[] = {"WAIT", "OPEN", "CLOSE", "OPEN_INIT"};
    enum { ACT_WAIT, ACT_OPEN, ACT_CLOSE, ACT_OPEN_INIT };
    int actionIndex = dComIfGp_evmng_getMyActIdx(
        mStaffId, (DUSK_CONST char* DUSK_CONST*)action_table, 4, FALSE, 0);

    f32 v;
    switch (actionIndex) {
    case ACT_WAIT:
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    case ACT_OPEN_INIT:
        wwShutter_seOwed("WDUN_R04_STR_OP");   // [S4]
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    case ACT_OPEN:
        v = cLib_addCalc(&current.pos.y, home.pos.y + kSh2MoveLen, 0.1f, kSh2MaxSpeed, kSh2MinSpeed);
        if (v == 0.0f) {
            dComIfGp_evmng_cutEnd(mStaffId);
        }
        break;
    case ACT_CLOSE:
        v = cLib_addCalc(&current.pos.y, home.pos.y, 0.1f, kSh2MaxSpeed, kSh2MinSpeed);
        if (v == 0.0f) {
            dComIfGp_evmng_cutEnd(mStaffId);
        }
        break;
    default:
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    }
}

void daWwShutter2_c::demo() {
    u8 isSwitch = fopAcM_isSwitch(this, mSwitchNo);
    u8 isNearEnemy = fopAcM_myRoomSearchEnemy(fopAcM_GetRoomNo(this)) == NULL;
    if (mDemoState == 0) {
        if (mSwitchNo != 0xFF) {
            if (isSwitch != mbIsSwitch) {
                mDemoState = isSwitch ? 1 : 2;
            }
        } else if (isNearEnemy != mbIsNearEnemy) {
            mDemoState = isNearEnemy ? 1 : 2;
        }
    }
    if (eventInfo.checkCommandDemoAccrpt()) {
        if (dComIfGp_evmng_startCheck(mOpenEventIdx) && mDemoState == 1) {
            mDemoState = 0;
        }
        if (dComIfGp_evmng_startCheck(mCloseEventIdx) && mDemoState == 2) {
            mDemoState = 0;
        }
        if (dComIfGp_evmng_endCheck(mOpenEventIdx) || dComIfGp_evmng_endCheck(mCloseEventIdx)) {
            dComIfGp_event_reset();
        }
        mStaffId = dComIfGp_evmng_getMyStaffId("Htobi3", NULL, 0);
        shutter_move();   // [S5]
    } else if (mDemoState == 1 && mOpenEventIdx > 0) {
        fopAcM_orderOtherEventId(this, (s16)mOpenEventIdx, 0xFF, -1, 0, 1);
        eventInfo.onCondition(dEvtCnd_CANDEMO_e /* donor UNK2 = bit 0x2 */);
    } else if (mDemoState == 2 && mCloseEventIdx > 0) {
        fopAcM_orderOtherEventId(this, (s16)mCloseEventIdx, 0xFF, -1, 0, 1);
        eventInfo.onCondition(dEvtCnd_CANDEMO_e /* donor UNK2 = bit 0x2 */);
    }
}

bool daWwShutter2_c::_draw() {
    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mpModel, &tevStr);
    dComIfGd_setListBG();
    mDoExt_modelUpdateDL(mpModel);
    dComIfGd_setList();
    return TRUE;
}

static cPhs_Step daWwShutter2_Create(fopAc_ac_c* i_this) {
    return ((daWwShutter2_c*)i_this)->_create();
}
static BOOL daWwShutter2_Delete(daWwShutter2_c* i_this) { return i_this->_delete(); }
static BOOL daWwShutter2_Execute(daWwShutter2_c* i_this) { return i_this->_execute(); }
static BOOL daWwShutter2_IsDelete(daWwShutter2_c*) { return TRUE; }
static BOOL daWwShutter2_Draw(daWwShutter2_c* i_this) { return i_this->_draw(); }

static actor_method_class l_daWwShutter2_Method = {
    (process_method_func)daWwShutter2_Create,
    (process_method_func)daWwShutter2_Delete,
    (process_method_func)daWwShutter2_Execute,
    (process_method_func)daWwShutter2_IsDelete,
    (process_method_func)daWwShutter2_Draw,
};

actor_process_profile_definition g_profile_WW_SHUTTER2 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0003,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_WW_SHUTTER2_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daWwShutter2_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_E_RD_e,
    /* Actor SubMtd */ &l_daWwShutter2_Method,
    /* Status       */ fopAcStts_CULL_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
