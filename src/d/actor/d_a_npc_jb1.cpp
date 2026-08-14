// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_npc_jb1.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW JABUN (NPC_JB1) — donor daNpc_Jb1_c ported WHOLE in the ls1/bm1 NPC
// dialect (tale §822; §813 user order: TSUBO → JABUN → the three NPCs).
// The great fish: McaMorf body (Jb.bdl, wait/appear BCKs) + lamp BRK, the
// 2x-scaled light-fish companion model under the jb_case joint (BMD_LF +
// BRK_LF), a dKy point light fed from the donor HIO params, one wait
// action, event-cut plumbing ("Jb1" staff, ACTION cut), talk order, and
// the demo binding ("Jb").
//
// SEAMS (§805-class labels, each at its site):
//   [J1] models via the DN-3 consume-time acquirer BY NAME (jb.bdl under
//        RARC node BDLM, lf.bmd under BMDM — NEITHER is mount-parsed; the
//        §816 lesson applied from line one). Anm types (BCKS/BRK) keep the
//        donor's mount-parse path — ID-res fetches are sound.
//   [J2] HIO: the donor's live-tuning child is dropped (the ls1 posture);
//        the donor's parameter TABLE ships as the static it initialized.
//   [J3] port arities: fopAcM_ct (not ct_Retail/ct_Demo), resDelete (not
//        resDeleteDemo), orderSpeakEvent(this,0,0), setActorInfo2(char*).
//   [J4] draw lights through the §406 dKyWw feeders (both models).
//   [J5] registration §747 pattern: fpcNm_NPC_JB1_e 0x335, register row,
//        born-relinquished slot, data-keyed "Jb1" route (donor
//        d_stage.cpp:751; name verified absent from the receiver table).
// ============================================================

#include "d/actor/d_a_npc_jb1.h"

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_ww.h"             // [J4] §406 WW lighting feeders + TEV_TYPE_ACTOR
#include "d/d_demo.h"
#include "d/d_ext_npc_mount.h"         // [J1] DN-3 consume-time model acquirer
#include "d/d_ext_ww_actor_shims.h"    // WW dialect shims (stopZelAnime adapter etc.)
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "res/Object/Jb.h"
#include "dusk/logging.h"

// draw-prio: donor fpcDwPi_NPC_JB1_e slot absent — the family's ground/ambient
// NPC choice (pig/seagull/bm1/ls1/zl1 precedent).
#define fpcDwPi_NPC_JB1_e fpcDwPi_E_RD_e

// ============================================================
// [J2] donor HIO parameter block — the tuning CHILD is dropped (ls1 posture);
// the donor's own initializer values ship verbatim (retail branch of the
// VERSION_SELECT: 2050.0f light power).
// ============================================================
struct daNpc_Jb1_HIO_prm_c {
    f32 field_0x00;   // attention Y lift
    s16 field_0x04;
    s16 field_0x06;   // light R
    s16 field_0x08;   // light G
    s16 field_0x0A;   // light B
    f32 field_0x0C;   // light power
    f32 field_0x10;   // light fluctuation
};
static const daNpc_Jb1_HIO_prm_c l_HIO_prm = {
    100.0f,
    0,
    1000,
    1000,
    0,
    2050.0f,
    50.0f,
};

bool daNpc_Jb1_c::init_JB1_0() {
    set_action(&daNpc_Jb1_c::wait_action1, NULL);
    return true;
}

bool daNpc_Jb1_c::createInit() {
    gravity = 0.0f;

    mEventCut.setActorInfo2((char*)"Jb1", this);   // [J3] port arity
    field_0x91C = 2;
    bool temp;
    switch (field_0x921) {
        case 0:
            temp = init_JB1_0();
            break;
        default:
            temp = false;
            break;
    }

    if (temp) {
        shape_angle = current.angle;
    } else {
        return false;
    }
    dKy_plight_set(&field_0x7F8);
    mpMorf->setMorf(0.0f);
    setMtx();
    return true;
}

void daNpc_Jb1_c::setMtx() {
    cXyz temp(0.0f, -150.0f, 0.0f);

    playBrkAnm(m_lmp_brk, &field_0x834);
    playBrkAnm(m_lgt_brk, &field_0x858);

    if (!field_0x917) {
        field_0x908 = mpMorf->play(&eyePos, 0, 0);
        if (mpMorf->getFrame() < mFrame) {
            field_0x908 = true;
        }
        mFrame = mpMorf->getFrame();
        mObjAcch.CrrPos(dComIfG_Bgsp());
    }

    tevStr.room_no = dComIfG_Bgsp().GetRoomId(mObjAcch.m_gnd);
    tevStr.YukaCol = dComIfG_Bgsp().GetPolyColor(mObjAcch.m_gnd);   // donor mEnvrIdxOverride
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
    mpMorf->calc();

    mDoMtx_stack_c::copy(mpMorf->getModel()->getAnmMtx(m_cse_jnt_num));
    mDoMtx_stack_c::multVec(&temp, &field_0x6D0);
    mDoMtx_stack_c::copy(mpMorf->getModel()->getAnmMtx(m_cse_jnt_num));

    mDoMtx_stack_c::transM(0.0f, -150.0f, 0.0f);
    mDoMtx_stack_c::scaleM(2.0f, 2.0f, 2.0f);
    m_lgt_mdl->setBaseTRMtx(mDoMtx_stack_c::get());
    m_lgt_mdl->calc();

    field_0x7F8.mPosition.set(field_0x6D0.x, field_0x6D0.y, field_0x6D0.z);

    field_0x7F8.mColor.r = l_HIO_prm.field_0x06;
    field_0x7F8.mColor.g = l_HIO_prm.field_0x08;
    field_0x7F8.mColor.b = l_HIO_prm.field_0x0A;
    field_0x7F8.mPow = l_HIO_prm.field_0x0C;
    field_0x7F8.mFluctuation = l_HIO_prm.field_0x10;
    setAttention();
}

void daNpc_Jb1_c::playBrkAnm(J3DAnmTevRegKey* param_1, s16* param_2) {
    if (param_1 == NULL) {
        return;
    }
    *param_2 = *param_2 + 1;
    if (*param_2 < param_1->getFrameMax()) {
        return;
    }
    *param_2 = 0;
}

int daNpc_Jb1_c::anmNum_toResID(int param_1) {
    static const int a_bck_resID_tbl[] = {
        dRes_ID_JB_BCK_JB_WAIT01_e,
        dRes_ID_JB_BCK_JB_APPEAR_e,
    };
    return a_bck_resID_tbl[param_1];
}

BOOL daNpc_Jb1_c::setAnm_anm(daNpc_Jb1_c::anm_prm_c* i_anmPrmP) {
    if (field_0x91C == i_anmPrmP->field_0x0) {
        return TRUE;
    }

    field_0x91C = i_anmPrmP->field_0x0;

    dNpc_setAnmIDRes(mpMorf, i_anmPrmP->mLoopMode, i_anmPrmP->mMorf, i_anmPrmP->mPlaySpeed,
                     anmNum_toResID(field_0x91C), -1, "Jb");
    mFrame = 0.0f;
    field_0x909 = 0;
    field_0x908 = 0;
    return TRUE;
}

BOOL daNpc_Jb1_c::setAnm() {
    static daNpc_Jb1_c::anm_prm_c a_anm_prm_tbl[2] = {
        {-1, 0, 0.0f, 0.0f, J3DFrameCtrl::EMode_NULL},
        {0, 0, 8.0f, 1.0f, J3DFrameCtrl::EMode_LOOP},
    };

    if (a_anm_prm_tbl[field_0x91E].field_0x0 >= 0) {
        setAnm_anm(&a_anm_prm_tbl[field_0x91E]);
    }
    return TRUE;
}

void daNpc_Jb1_c::chg_anmTag() {
}

void daNpc_Jb1_c::control_anmTag() {
}

void daNpc_Jb1_c::chg_anmAtr(u8 param_1) {
    if (param_1 >= 1 || param_1 == field_0x91A) {
        return;
    }

    field_0x91A = param_1;
    setAnm_ATR();
}

void daNpc_Jb1_c::control_anmAtr() {
    switch (field_0x91A) {
        case 0:
            break;
    }
}

void daNpc_Jb1_c::setAnm_ATR() {
    static daNpc_Jb1_c::anm_prm_c a_anm_prm_tbl[1] = {
        {0, 0, 8.0f, 1.0f, J3DFrameCtrl::EMode_LOOP},
    };

    setAnm_anm(&a_anm_prm_tbl[field_0x91A]);
}

void daNpc_Jb1_c::anmAtr(u16 param_1) {
    switch (param_1) {
        case 6: {
            if (field_0x923 == 0) {
                field_0x91A = 0xFF;
                chg_anmAtr(dComIfGp_getMesgAnimeAttrInfo());
                field_0x923++;
            }

            u8 mesgTagInfo = dComIfGp_getMesgAnimeTagInfo();
            dComIfGp_setMesgAnimeTagInfo(0xFF);   // donor clearMesgAnimeTagInfo (shim form)
            if (mesgTagInfo != 0xFF && field_0x91B != mesgTagInfo) {
                field_0x91B = mesgTagInfo;
                chg_anmTag();
            }
            break;
        }
        case 0xE:
            field_0x923 = 0;
            break;
    }

    control_anmTag();
    control_anmAtr();
}

void daNpc_Jb1_c::eventOrder() {
    if (field_0x91D == 1 || field_0x91D == 2) {
        eventInfo.onCondition(dEvtCnd_CANTALK_e);
        if (field_0x91D == 1) {
            fopAcM_orderSpeakEvent(this, 0, 0);   // [J3] port arity
        }
    }
}

void daNpc_Jb1_c::checkOrder() {
    if (eventInfo.checkCommandDemoAccrpt()) {   // donor dEvtCmd_INDEMO_e check
        return;
    }

    if (!eventInfo.checkCommandTalk()) {   // donor dEvtCmd_INTALK_e check
        return;
    }

    if (field_0x91D != 1 && field_0x91D != 2) {
        return;
    }
    field_0x91D = 0;
    field_0x915 = 1;
}

u16 daNpc_Jb1_c::next_msgStatus(u32* pMsgNo) {
    return fopMsg_MODE_MSG_END_e;   // port name (donor fopMsgStts_MSG_ENDS_e)
}

u32 daNpc_Jb1_c::getMsg_JB1_0() {
    return 0;
}

u32 daNpc_Jb1_c::getMsg() {
    u32 msg = 0;
    switch (field_0x921) {
        case 0:
            msg = getMsg_JB1_0();
            break;
    }

    return msg;
}

bool daNpc_Jb1_c::chkAttention() {
    dAttention_c& attention = *dComIfGp_getAttention();   // port: pointer form

    if (attention.LockonTruth()) {
        return this == attention.LockonTarget(0);
    }

    return this == attention.ActionTarget(0);
}

void daNpc_Jb1_c::setAttention() {
    cXyz temp(0.0f, 1100.0f, 700.0f);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mDoMtx_stack_c::multVec(&temp, &field_0x8F4);
    attention_info.position.set(field_0x8F4.x, field_0x8F4.y + l_HIO_prm.field_0x00,
                                field_0x8F4.z);
    eyePos.set(field_0x8F4.x, field_0x8F4.y, field_0x8F4.z);
}

bool daNpc_Jb1_c::charDecide(int) {
    field_0x920 = 0;
    field_0x921 = -1;
    field_0x921 = 0;
    return true;
}

void daNpc_Jb1_c::event_actionInit(int staffIdx) {
    int* actNo = dComIfGp_evmng_getMyIntegerP(staffIdx, "ActNo");
    if (actNo != NULL) {
        mActNo = *actNo;
    }
}

bool daNpc_Jb1_c::event_action() {
    switch (mActNo) {
        case 0:
            return true;
    }
    return true;
}

void daNpc_Jb1_c::privateCut(int staffIdx) {
    static char* a_cut_tbl[] = {
        (char*)"ACTION",
    };

    if (staffIdx != -1) {
        mActIdx = dComIfGp_evmng_getMyActIdx(staffIdx, a_cut_tbl, 1, TRUE, 0);
        if (mActIdx == -1) {
            dComIfGp_evmng_cutEnd(staffIdx);
        } else {
            if (dComIfGp_evmng_getIsAddvance(staffIdx)) {
                switch (mActIdx) {
                    case 0:
                        event_actionInit(staffIdx);
                        break;
                }
            }
            bool temp;
            switch (mActIdx) {
                case 0:
                    temp = event_action();
                    break;
                default:
                    temp = true;
                    break;
            }
            if (temp) {
                dComIfGp_evmng_cutEnd(staffIdx);
            }
        }
    }
}

int daNpc_Jb1_c::isEventEntry() {
    return dComIfGp_evmng_getMyStaffId(mEventCut.getActorName(), this, 0);   // port arity
}

void daNpc_Jb1_c::event_proc(int param_1) {
    if (!mEventCut.cutProc()) {
        privateCut(param_1);
    }
}

BOOL daNpc_Jb1_c::set_action(ActionFunc actionFunc, void* arg) {
    if (mCurrActionFunc != actionFunc) {
        if (mCurrActionFunc != NULL) {
            field_0x922 = 9;
            (this->*mCurrActionFunc)(arg);
        }
        mCurrActionFunc = actionFunc;
        field_0x922 = 0;
        (this->*mCurrActionFunc)(arg);
    }
    return TRUE;
}

void daNpc_Jb1_c::setStt(s8 param_1) {
    field_0x91E = param_1;
    switch (field_0x91E) {
        case 0:
        case 1:
            break;
    }

    setAnm();
}

BOOL daNpc_Jb1_c::wait_1() {
    return TRUE;
}

BOOL daNpc_Jb1_c::wait_action1(void*) {
    switch (field_0x922) {
        case 9:
            break;
        case 0:
            setStt(1);
            field_0x922++;
            break;
        case 1:
        case 2:
        case 3:
            mHasAttention = chkAttention();
            switch (field_0x91E) {
                case 1:
                    field_0x910 = wait_1();
                    break;
            }
            break;
    }
    return TRUE;
}

bool daNpc_Jb1_c::demo() {
    if (demoActorID == 0) {
        if (field_0x917) {
            field_0x917 = false;
        }
    } else {
        field_0x917 = true;
        dComIfGp_demo_getActor(demoActorID);
        dDemo_setDemoData(this,
                          dDemo_actor_c::ENABLE_TRANS_e | dDemo_actor_c::ENABLE_ROTATE_e |
                              dDemo_actor_c::ENABLE_ANM_e | dDemo_actor_c::ENABLE_ANM_FRAME_e,
                          mpMorf, "Jb", 0, NULL, 0, 0);   // §244 port arity (8-arg)
    }
    return field_0x917;
}

BOOL daNpc_Jb1_c::_draw() {
    J3DModelData* lgtModelData = m_lgt_mdl->getModelData();
    J3DModel* pModel = mpMorf->getModel();
    J3DModelData* modelData = pModel->getModelData();

    if (field_0x90C || field_0x90E) {
        return TRUE;
    }

    // [J4] WW-host lighting through the §406 feeders (ls1 idiom), both models.
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(pModel, &tevStr);
    dKyWw_setLightTevColorType(m_lgt_mdl, &tevStr);
    mBrkAnm.entry(modelData, field_0x834);
    mpMorf->entryDL();
    mBrkAnm.remove(modelData);
    mBrkAnm2.entry(lgtModelData, field_0x858);
    mDoExt_modelEntryDL(m_lgt_mdl);
    mBrkAnm2.remove(lgtModelData);
    return TRUE;
}

BOOL daNpc_Jb1_c::_execute() {
    if (!field_0x90F) {
        mCurrentPos.set(current.pos.x, current.pos.y, current.pos.z);
        mCurrentRot = current.angle;
        field_0x90F = true;
    }

    if (field_0x90C && demoActorID == 0) {
        return TRUE;
    }

    field_0x90C = false;
    checkOrder();
    if (!demo()) {
        int temp = -1;
        if (dComIfGp_event_runCheck() && eventInfo.checkCommandTalk() == false) {
            temp = isEventEntry();
        }

        if (temp >= 0) {
            event_proc(temp);
        } else {
            (this->*mCurrActionFunc)(NULL);
        }
        fopAcM_posMoveF(this, mStts.GetCCMoveP());
        if (!field_0x90D) {
            shape_angle = current.angle;
        }
    }
    eventOrder();
    setMtx();
    return TRUE;
}

BOOL daNpc_Jb1_c::_delete() {
    dComIfG_resDelete(&mPhs, "Jb");   // [J3] port name (donor resDeleteDemo)
    dKy_plight_cut(&field_0x7F8);

    if (heap != NULL && mpMorf != NULL) {
        dExtNpcBm1_stopZelAnime(mpMorf);   // port adapter (McaMorf lacks stopZelAnime)
    }
    return TRUE;
}

static BOOL CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daNpc_Jb1_c*)i_this)->CreateHeap();
}

cPhs_Step daNpc_Jb1_c::_create() {
    static u32 a_size_tbl[] = {
        0x000272E0,
    };

    fopAcM_ct(this, daNpc_Jb1_c);   // [J3] donor ct_Retail/ct_Demo pair

    cPhs_Step phase_state = dComIfG_resLoad(&mPhs, "Jb");
    switch (phase_state) {
        case cPhs_COMPLEATE_e:
            u8 param = fopAcM_GetParam(this);
            if (!charDecide(param)) {
                return cPhs_ERROR_e;
            }

            // [J2] donor HIO child creation dropped (ls1 posture).

            if (!fopAcM_entrySolidHeap(this, CheckCreateHeap, a_size_tbl[field_0x920])) {
                return cPhs_ERROR_e;
            }
            fopAcM_SetMtx(this, mpMorf->getModel()->getBaseTRMtx());
            if (createInit() == 0) {
                return cPhs_ERROR_e;
            }
            return phase_state;
    }
    return phase_state;
}

daNpc_Jb1_c::daNpc_Jb1_c() {}

J3DModelData* daNpc_Jb1_c::create_Anm() {
    // [J1] DN-3: jb.bdl is a BDLM member — NOT mount-parsed; the consume-time
    // acquirer parses from a pristine copy and caches (the §816 lesson).
    J3DModelData* a_mdl_dat = dExtNpcMount_acquireModelData("Jb", "jb.bdl");
    if (a_mdl_dat == NULL) {
        DuskLog.warn("[NpcJb1] jb.bdl unresolvable/unparseable");
        return NULL;
    }

    mpMorf = new mDoExt_McaMorf(
        a_mdl_dat, NULL, NULL,
        (J3DAnmTransformKey*)dComIfG_getObjectIDRes("Jb", dRes_ID_JB_BCK_JB_WAIT01_e),
        J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1, 1, NULL, 0x80000, 0x11020022);

    if (mpMorf == NULL) {
        return NULL;
    } else if (mpMorf->getModel() == NULL) {
        mpMorf = NULL;
        return NULL;
    }

    m_lmp_brk = (J3DAnmTevRegKey*)dComIfG_getObjectIDRes("Jb", dRes_ID_JB_BRK_JB_LAMP_e);
    if (m_lmp_brk == NULL) {
        DuskLog.warn("[NpcJb1] BRK_JB_LAMP unresolvable");
        mpMorf = NULL;
        return NULL;
    }
    bool temp = mBrkAnm.init(mpMorf->getModel()->getModelData(), m_lmp_brk, true,
                             J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1);
    if (temp == false) {
        mpMorf = NULL;
        return NULL;
    } else {
        field_0x834 = 0;
        m_cse_jnt_num = mpMorf->getModel()->getModelData()->getJointTree().getJointName()
                            ->getIndex("jb_case");
        if (m_cse_jnt_num < 0) {
            DuskLog.warn("[NpcJb1] jb_case joint missing");
            mpMorf = NULL;
            return NULL;
        }
    }

    return a_mdl_dat;
}

bool daNpc_Jb1_c::create_lgt() {
    // [J1] DN-3: lf.bmd is a BMDM member — the resType switch has no BMDM
    // branch (checked d_resorce.cpp:425-534), so it is RAW; acquirer route.
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Jb", "lf.bmd");
    if (modelData == NULL) {
        DuskLog.warn("[NpcJb1] lf.bmd unresolvable/unparseable");
        return false;
    }
    m_lgt_mdl = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (m_lgt_mdl == NULL) {
        return false;
    }

    m_lgt_brk = (J3DAnmTevRegKey*)dComIfG_getObjectIDRes("Jb", dRes_ID_JB_BRK_LF_e);
    if (m_lgt_brk == NULL) {
        DuskLog.warn("[NpcJb1] BRK_LF unresolvable");
        return false;
    }
    bool temp = mBrkAnm2.init(m_lgt_mdl->getModelData(), m_lgt_brk, true,
                              J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1);
    if (!temp) {
        return temp;
    }

    field_0x858 = 0;
    return temp;
}

BOOL daNpc_Jb1_c::CreateHeap() {
    if (create_Anm() == NULL) {
        return false;
    }
    if (!create_lgt()) {
        return false;
    }

    mpMorf->getModel()->setUserArea(0);
    mAcchCir.SetWall(0.0f, 0.0f);
    mObjAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1, &mAcchCir,
                 fopAcM_GetSpeed_p(this), NULL, NULL);
    return true;
}

static cPhs_Step daNpc_Jb1_Create(void* i_this) {
    return ((daNpc_Jb1_c*)i_this)->_create();
}

static BOOL daNpc_Jb1_Delete(void* i_this) {
    return ((daNpc_Jb1_c*)i_this)->_delete();
}

static BOOL daNpc_Jb1_Execute(void* i_this) {
    return ((daNpc_Jb1_c*)i_this)->_execute();
}

static BOOL daNpc_Jb1_Draw(void* i_this) {
    return ((daNpc_Jb1_c*)i_this)->_draw();
}

static BOOL daNpc_Jb1_IsDelete(void*) {
    return TRUE;
}

static actor_method_class l_daNpc_Jb1_Method = {
    (process_method_func)daNpc_Jb1_Create,
    (process_method_func)daNpc_Jb1_Delete,
    (process_method_func)daNpc_Jb1_Execute,
    (process_method_func)daNpc_Jb1_IsDelete,
    (process_method_func)daNpc_Jb1_Draw,
};

extern actor_process_profile_definition g_profile_NPC_JB1;

actor_process_profile_definition g_profile_NPC_JB1 = {
    // donor g_profile_NPC_JB1 (d_a_npc_jb1.cpp:713); receiver idiom fields.
    fpcLy_CURRENT_e,           // Layer ID
    7,                         // List ID (donor 0x0007)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_NPC_JB1_e,           // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daNpc_Jb1_c),       // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_NPC_JB1_e,         // Draw Prio
    &l_daNpc_Jb1_Method,       // Actor SubMtd
    fopAcStts_UNK40000_e,      // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_0_e,         // Cull Type
};
