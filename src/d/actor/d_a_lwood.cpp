// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_lwood.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW NORMAL TREE (Lwood, arc Lwood) — §860 batch row 1 of 4 (planner head,
// 27 Outset placements). Donor WHOLE: solid MoveBG trunk (own DZB through
// DN-1's SANCTIONED Regist idiom — the entry's own blessed form), wind-
// swaying leaf joint (J_Alwd_ha callback), random per-tree sway scale.
// SEAMS: [L1] model+DZB via the DN-3 acquirer / raw cBgD_t (DZB is
// consumed raw natively); [L2] wind through the WW WIND FIELD (§868
// parallel system — the §865 user rule; the point-wind term rides the
// same module's pntwind ring); [L3] isMonotone (photo mode) absent -> false.
// ============================================================

#include "d/actor/d_a_lwood.h"

#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_ww.h"
#include "d/d_kankyo_ww_wind.h"   // §868 WW wind field (parallel system — never TP dKyw_*)
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_ww_actor_shims.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "res/Object/Lwood.h"
#include "dusk/logging.h"

#define fpcDwPi_Lwood_e fpcDwPi_E_RD_e

const char daLwood_c::m_arcname[6] = "Lwood";

static BOOL daLwood_CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daLwood_c*)i_this)->CreateHeap();
}

BOOL daLwood_c::CreateHeap() {
    // [L1] DN-3: BDL via the consume-time acquirer; the DZB is layout-compatible
    // with receiver dBgW but needs the §334 attribute repack (below).
    J3DModelData* modelData =
        dExtNpcMount_acquireModelDataByIndex(m_arcname, dRes_INDEX_LWOOD_BDL_ALWD_e);
    if (modelData == NULL) {
        DuskLog.warn("[Lwood] alwd.bdl unresolvable");
        return FALSE;
    }
    mModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
    if (mModel == NULL)
        return FALSE;

    mModel->setUserArea((uintptr_t)this);
    setMoveBGMtx();
    cBgD_t* bgp = (cBgD_t*)dComIfG_getObjectRes(m_arcname, dRes_INDEX_LWOOD_DZB_ALWD_e);
    // §895 (§885 solidity row): the §334 donor-DZB attribute repack — WW packs
    // attCode in PolyInf1 bits 16-20, the receiver reads att0/att1/groundCode
    // there; untranslated attCode 1 = TP att1 SINK. Every donor object-DZB
    // consumer (knob00/otble/plank_span/mount x3/room loader) routes through
    // this seam; idempotent. My earlier "raw data dBgW consumes natively" claim
    // was the STRUCTURAL half only — layout yes, vocabulary no.
    dExtWw_repackDzbAttributes(bgp, "alwd.dzb");
    mpBgW = dBgW_NewSet(bgp, dBgW::MOVE_BG_e, &mtx);
    if (mpBgW == NULL)
        return FALSE;

    return TRUE;
}

static int daLwood_nodeCallBack(J3DJoint*, int);

void daLwood_c::CreateInit() {
    fopAcM_SetMtx(this, mModel->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, -600.0f, -0.0f, -600.0f, 600.0f, 900.0f, 600.0f);
    fopAcM_setCullSizeFar(this, 2.37f);
    mTimer = cM_rndF(0x8000);
    mScale = cM_rndF(0.4f) + 0.8f;
    JUTNameTab* jointName = mModel->getModelData()->getJointTree().getJointName();
    for (u16 i = 0; i < mModel->getModelData()->getJointNum(); i++) {
        if (strcmp("J_Alwd_ha", jointName->getName(i)) == 0) {
            mModel->getModelData()->getJointNodePointer(i)->setCallBack(&daLwood_nodeCallBack);
            break;
        }
    }

    mModel->calc();
    // DN-1's SANCTIONED registration idiom, verbatim (docs/DO-NOT.md DN-1:
    // "dComIfG_Bgsp().Regist(bgw, this)") — no room id is stamped anywhere.
    dComIfG_Bgsp().Regist(mpBgW, this);
    set_mtx();
    mpBgW->Move();
}

static int daLwood_nodeCallBack(J3DJoint* joint, int calcTiming) {
    // [L3] donor gates on mDoGph_gInf_c::isMonotone (photo mode) — no
    // receiver surface; the gate reads false (always sway).
    if (calcTiming == J3DNodeCBCalcTiming_In) {
        u32 jntNo = joint->getJntNo();
        J3DModel* model = j3dSys.getModel();
        daLwood_c* i_this = (daLwood_c*)model->getUserArea();
        if (i_this != NULL) {
            // [L2] donor daObj::get_wind_spd(this, 100) = (wind + pntwind)
            // * 50 on the WW FIELD (§868 parallel system):
            cXyz pntDir; f32 pntPow;
            dKyWw_pntwind_get_info(&i_this->current.pos, &pntDir, &pntPow);
            cXyz windSpeed = ((*dKyWw_get_wind_vec()) * dKyWw_get_wind_pow() +
                              pntDir * pntPow) * 50.0f;
            f32 sy = cM_ssin(i_this->getYureTimer() * 300);
            s16 r2 = windSpeed.x * sy * 10.0f;
            f32 cy = cM_scos(i_this->getYureTimer() * 300);
            s16 r0 = windSpeed.z * cy * 10.0f;

            s16 r1 = (s16)(fabs(sy + 1.0f) * 250.0f);

            s16 p1 = i_this->getYureScale() * r2;
            s16 p2 = i_this->getYureScale() * r0;
            s16 p0 = i_this->getYureScale() * r1;

            mDoMtx_stack_c::copy(model->getAnmMtx(jntNo));
            mDoMtx_stack_c::ZXYrotM(p0, p1, p2);
            model->setAnmMtx(jntNo, mDoMtx_stack_c::get());
            cMtx_copy(mDoMtx_stack_c::get(), J3DSys::mCurrentMtx);
        }
    }

    return 1;
}

void daLwood_c::set_mtx() {
    mModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::ZXYrotM(current.angle.x, current.angle.y, current.angle.z);
    mModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

void daLwood_c::setMoveBGMtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::ZXYrotM(current.angle.x, current.angle.y, current.angle.z);
    mDoMtx_stack_c::scaleM(scale.x, scale.y, scale.z);
    cMtx_copy(mDoMtx_stack_c::get(), mtx);
}

cPhs_Step daLwood_c::_create() {
    fopAcM_ct(this, daLwood_c);

    cPhs_Step ret = dComIfG_resLoad(&mPhs, m_arcname);

    if (ret == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(this, daLwood_CheckCreateHeap, 0x0e40) == 0) {
            ret = cPhs_ERROR_e;
        } else {
            CreateInit();
        }
    }

    return ret;
}

bool daLwood_c::_delete() {
    if (heap != NULL)
        dComIfG_Bgsp().Release(mpBgW);

    dComIfG_resDelete(&mPhs, m_arcname);
    return TRUE;
}

bool daLwood_c::_execute() {
    mTimer++;
    return TRUE;
}

bool daLwood_c::_draw() {
    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mModel, &tevStr);
    dComIfGd_setListBG();
    mDoExt_modelUpdateDL(mModel);
    dComIfGd_setList();
    return TRUE;
}

static cPhs_Step daLwood_Create(void* i_this) {
    return ((daLwood_c*)i_this)->_create();
}

static BOOL daLwood_Delete(void* i_this) {
    return ((daLwood_c*)i_this)->_delete();
}

static BOOL daLwood_Draw(void* i_this) {
    return ((daLwood_c*)i_this)->_draw();
}

static BOOL daLwood_Execute(void* i_this) {
    return ((daLwood_c*)i_this)->_execute();
}

static BOOL daLwood_IsDelete(void*) {
    return TRUE;
}

static actor_method_class daLwoodMethodTable = {
    (process_method_func)daLwood_Create,
    (process_method_func)daLwood_Delete,
    (process_method_func)daLwood_Execute,
    (process_method_func)daLwood_IsDelete,
    (process_method_func)daLwood_Draw,
};

extern actor_process_profile_definition g_profile_Lwood;

actor_process_profile_definition g_profile_Lwood = {
    // donor g_profile_Lwood (d_a_lwood.cpp:184).
    fpcLy_CURRENT_e,           // Layer ID
    7,                         // List ID (donor 0x0007)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_Lwood_e,             // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daLwood_c),         // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_Lwood_e,           // Draw Prio
    &daLwoodMethodTable,       // Actor SubMtd
    fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_CUSTOM_e,    // Cull Type
};
