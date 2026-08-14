// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_obj_lpalm.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: HELD (one /* Nonmatching */ at donor :125 — the user's
//   verbatim gate: "not verbatim, not passed"; my §863 adjudication was
//   OVERRULED. This TU stays dark until the function reaches byte-exactness
//   upstream or via the ja1-method reconstruction, §866 materials scouted)
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW PALM TREE (Obj_Lpalm, arc Oyashi) — §860 batch row 2 (10 Outset
// placements). Donor WHOLE: solid MoveBG trunk (own DZB, DN-1 sanctioned
// Regist), wind-bend base quaternion slerp + two independent leaf-wave
// quats (J_HAPPA1/2 joint callbacks) driven by the WW WIND FIELD (§868
// parallel system). SEAMS: [M1] model via the DN-3 acquirer; DZB raw-native.
// The donor Attr flags (0,0) ship verbatim (both gates off).
// ============================================================

#include "d/actor/d_a_obj_lpalm.h"

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_ww.h"
#include "d/d_kankyo_ww_wind.h"   // §868 WW wind field (parallel system — never TP dKyw_*)
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_ww_actor_shims.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "res/Object/Oyashi.h"
#include "dusk/logging.h"

#define fpcDwPi_Obj_Lpalm_e fpcDwPi_E_RD_e

const char daObjLpalm_c::M_arcname[7] = "Oyashi";
daObjLpalm_c::Attr_c const daObjLpalm_c::M_attr = {0, 0};

static BOOL daObjLpalm_CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daObjLpalm_c*)i_this)->CreateHeap();
}

// receiver joint-callback signature (code_dialect row).
static int daObjLpalm_nodeCallBack(J3DJoint* joint, int calcTiming) {
    J3DModel* model = j3dSys.getModel();
    s32 jntNo = joint->getJntNo();
    daObjLpalm_c* i_this = (daObjLpalm_c*)model->getUserArea();
    if (calcTiming == J3DNodeCBCalcTiming_In &&
        (jntNo == OYASHI_JNT_J_HAPPA1_e || jntNo == OYASHI_JNT_J_HAPPA2_e)) {
        mDoMtx_stack_c::copy(model->getAnmMtx(jntNo));
        mDoMtx_stack_c::ZrotM(-0x4000);
        mDoMtx_stack_c::quatM(&i_this->mBaseQuat);
        if (jntNo == OYASHI_JNT_J_HAPPA1_e)
            mDoMtx_stack_c::quatM(&i_this->mAnmMtxQuat[0]);
        else
            mDoMtx_stack_c::quatM(&i_this->mAnmMtxQuat[1]);
        mDoMtx_stack_c::ZrotM(0x4000);
        model->setAnmMtx(jntNo, mDoMtx_stack_c::get());
    }

    return 1;
}

BOOL daObjLpalm_c::CreateHeap() {
    // [M1] DN-3 acquirer (donor raw cast = the §810-2 class); NULL check
    // BEFORE the joint walk (the donor checks after — order corrected, the
    // donor's own intent).
    J3DModelData* modelData =
        dExtNpcMount_acquireModelDataByIndex(M_arcname, dRes_INDEX_OYASHI_BDL_OYASHI_e);
    if (modelData == NULL) {
        DuskLog.warn("[ObjLpalm] oyashi.bdl unresolvable");
        return false;
    }
    for (u16 i = 0; i < modelData->getJointNum(); i++)
        modelData->getJointNodePointer(i)->setCallBack(daObjLpalm_nodeCallBack);

    mModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000002);
    if (mModel == NULL)
        return false;

    mModel->setUserArea((uintptr_t)this);
    mpBgW = dBgW_NewSet((cBgD_t*)dComIfG_getObjectRes(M_arcname, dRes_INDEX_OYASHI_DZB_OYASHI_e),
                        dBgW::MOVE_BG_e, &mModel->getBaseTRMtx());
    if (mpBgW == NULL)
        return false;

    return true;
}

void daObjLpalm_c::CreateInit() {
    Quaternion q = {0.0f, 0.0f, 0.0f, 1.0f};

    mBaseQuat = mBaseQuatTarget = q;
    mAnmMtxQuat[0] = mAnmMtxQuat[1] = q;
    mAnimDir[0] = 0;
    mAnimDir[1] = 0;
    mAnimWave[0] = 0;
    mAnimWave[1] = cM_rndFX(0x8000);
    fopAcM_SetMtx(this, mModel->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, -350.0f, -50.0f, -350.0f, 350.0f, 1300.0f, 350.0f);
    fopAcM_setCullSizeFar(this, 2.37f);
    // DN-1's sanctioned registration idiom, verbatim — no room stamp.
    dComIfG_Bgsp().Regist(mpBgW, this);
    mModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::ZXYrotM(shape_angle.x, shape_angle.y, shape_angle.z);
    mModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

cPhs_Step daObjLpalm_c::_create() {
    fopAcM_ct(this, daObjLpalm_c);   // donor ct_Retail/ct_Demo pair
    cPhs_Step ret = dComIfG_resLoad(&mPhs, M_arcname);
    if (ret == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(this, daObjLpalm_CheckCreateHeap, 0xf00) == 0) {
            ret = cPhs_ERROR_e;
        } else {
            CreateInit();
        }
    }

    return ret;
}

bool daObjLpalm_c::_delete() {
    if (heap != NULL && mpBgW != NULL && mpBgW->ChkUsed()) {
        dComIfG_Bgsp().Release(mpBgW);
    }

    dComIfG_resDelete(&mPhs, M_arcname);
    return true;
}

bool daObjLpalm_c::_execute() {
    // donor :125 /* Nonmatching */ — byte-level only; C carried whole.
    if (attr().flag0)
        return true;

    Quaternion q;
    cXyz up(0.0f, 1.0f, 0.0f);
    cXyz windDir;

    // donor rotates the wind into tree-local space on calc_mtx; the stack
    // serves the same op receiver-side.
    mDoMtx_stack_c::YrotS(-current.angle.y);
    mDoMtx_stack_c::multVecSR(dKyWw_get_wind_vec(), &windDir);
    f32 windPow = dKyWw_get_wind_pow();
    s16 angle = windPow * 0x600;
    cXyz dir = up.outprod(windDir);
    f32 mag = dir.abs();
    if (mag < 8e-09) {
        mBaseQuatTarget.x = 0.0f;
        mBaseQuatTarget.y = 0.0f;
        mBaseQuatTarget.z = 0.0f;
        mBaseQuatTarget.w = 1.0f;
    } else {
        f32 sin = cM_ssin(angle);
        dir = dir.normZP();
        mBaseQuatTarget.x = sin * dir.x;
        mBaseQuatTarget.y = sin * dir.y;
        mBaseQuatTarget.z = sin * dir.z;
        mBaseQuatTarget.w = cM_scos(angle);
    }

    mDoMtx_quatSlerp(&mBaseQuat, &mBaseQuatTarget, &q, 0.25f);
    mBaseQuat = q;

    for (s32 i = 0; i < 2; i++) {
        s16 target = windPow * 0x180;
        if (target > 0x100)
            target = 0x100;
        cLib_addCalcAngleS2(&mAnimDir[i], target, 4, 0x20);
        s32 add = (windPow * 0x800) + cM_rndFX(128.0f);
        mAnimWave[i] += add;
        f32 w = cM_ssin(mAnimDir[i]);
        mAnmMtxQuat[i].x = cM_ssin(mAnimWave[i]) * w;
        mAnmMtxQuat[i].y = 0.0f;
        mAnmMtxQuat[i].z = cM_ssin(mAnimWave[i]) * w;
        mAnmMtxQuat[i].w = cM_scos(mAnimDir[i]);
    }

    mpBgW->Move();
    return false;
}

bool daObjLpalm_c::_draw() {
    if (attr().flag1)
        return true;

    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mModel, &tevStr);
    mDoExt_modelUpdateDL(mModel);
    return true;
}

static cPhs_Step daObjLpalmCreate(void* i_this) {
    return ((daObjLpalm_c*)i_this)->_create();
}

static BOOL daObjLpalmDelete(void* i_this) {
    return ((daObjLpalm_c*)i_this)->_delete();
}

static BOOL daObjLpalmExecute(void* i_this) {
    return ((daObjLpalm_c*)i_this)->_execute();
}

static BOOL daObjLpalmDraw(void* i_this) {
    return ((daObjLpalm_c*)i_this)->_draw();
}

static BOOL daObjLpalmIsDelete(void*) {
    return TRUE;
}

static actor_method_class daObjLpalmMethodTable = {
    (process_method_func)daObjLpalmCreate,
    (process_method_func)daObjLpalmDelete,
    (process_method_func)daObjLpalmExecute,
    (process_method_func)daObjLpalmIsDelete,
    (process_method_func)daObjLpalmDraw,
};

extern actor_process_profile_definition g_profile_Obj_Lpalm;

actor_process_profile_definition g_profile_Obj_Lpalm = {
    // donor g_profile_Obj_Lpalm (d_a_obj_lpalm.cpp:202).
    fpcLy_CURRENT_e,           // Layer ID
    3,                         // List ID (donor 0x0003)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_Obj_Lpalm_e,         // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daObjLpalm_c),      // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_Obj_Lpalm_e,       // Draw Prio
    &daObjLpalmMethodTable,    // Actor SubMtd
    fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_CUSTOM_e,    // Cull Type
};
