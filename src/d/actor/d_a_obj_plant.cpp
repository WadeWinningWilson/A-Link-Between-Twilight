// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_obj_plant.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW POTTED PLANT (Obj_Plant, arc Plant) — §817-4 row 2, donor WHOLE:
// bump-wobble physics (joint2 node-callback sway with the donor's decay
// curve), bomb-target cc, the donor's own soft shadow (its OWN draw uses
// getSimpleTex explicitly — §823's pattern confirmed donor-side).
// SEAMS: [P1] model via the DN-3 by-index acquirer (§816 precondition);
// [P2] cc source donor semantics in the receiver initializer shape
// (Tg = BOMB only — the one same-bit AT, trap #5's safe case);
// [P3] SE owed ([T5] posture); draw lights through the §406 dKyWw feeders.
// ============================================================

#include "d/actor/d_a_obj_plant.h"

#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_drawlist.h"
#include "d/d_kankyo_ww.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_ww_actor_shims.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "dusk/logging.h"

#define JA_SE_OBJ_TREE_SWING_S 0   // [P3] audio pass owed
#define fpcDwPi_Obj_Plant_e fpcDwPi_E_RD_e

// [P2] donor l_cyl_src semantics (At none; Tg accepts BOMB only, Set|IsOther;
// Co Set|IsOther|VsEnemy; Tg no-hit-mark) in the receiver's initializer shape.
static const dCcD_SrcCyl l_plantCylSrc = {
    {
        {0x0, {{0, 0, 0}, {AT_TYPE_BOMB, 0x09}, 0x19}},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x4},
        {0x0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            30.0f,
            200.0f,
        }
    }
};

// receiver joint-callback signature: int(*)(J3DJoint*,int) (the npc_tk idiom).
static int daObjPlant_nodeCallBack(J3DJoint*, int);

static BOOL daObjPlant_CheckCreateHeap(fopAc_ac_c* i_this) {
    return static_cast<daObjPlant_c*>(i_this)->CreateHeap();
}

cPhs_Step daObjPlant_c::_create() {
    fopAcM_ct(this, daObjPlant_c);

    cPhs_Step phase_state = dComIfG_resLoad(&mPhase, "Plant");

    if (phase_state == cPhs_COMPLEATE_e) {
        if (!fopAcM_entrySolidHeap(this, daObjPlant_CheckCreateHeap, 0x0D20)) {
            return cPhs_ERROR_e;
        }
        CreateInit();
    }

    field_0x410 = 0;
    return phase_state;
}

BOOL daObjPlant_c::_draw() {
    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mpModel, &tevStr);
    dComIfGd_setListBG();
    mDoExt_modelUpdateDL(mpModel);
    dComIfGd_setList();
    cXyz shadow_vec(0.0f, 1.0f, 0.0f);
    // donor's OWN call passes getSimpleTex() explicitly (d_a_obj_plant.cpp:74).
    dComIfGd_setSimpleShadow(&current.pos, current.pos.y, 60.0f, &shadow_vec, 0, 1.0f,
                             dDlst_shadowControl_c::getSimpleTex());
    return TRUE;
}

BOOL daObjPlant_c::_delete() {
    dComIfG_resDelete(&mPhase, "Plant");
    return TRUE;
}

BOOL daObjPlant_c::_execute() {
    mCyl.SetC(current.pos);
    dComIfG_Ccsp()->Set(&mCyl);

    if ((mHitTimer > 0x38 || field_0x410 == 0) && mCyl.ChkCoHit()) {
        fopAc_ac_c* hitAc = mCyl.GetCoHitAc();

        cXyz diff = current.pos - hitAc->current.pos;
        s16 angle = cM_atan2s(diff.x, diff.z);

        field_0x40E = angle - 0x4000;
        field_0x410 = 1;
        mHitTimer = 0;
        field_0x40A = 0;

        fopAcM_seStart(this, JA_SE_OBJ_TREE_SWING_S, 0);   // [P3]
    }

    if (field_0x410 != 0) {
        mHitTimer++;

        if (mHitTimer < 8) {
            field_0x408 = (s16)((0x100 - mHitTimer) * cM_ssin(field_0x40A) * 32.0f);
            field_0x40A += 0x1000;
        } else {
            field_0x408 = (s16)((0x100 - mHitTimer) * cM_ssin(field_0x40A) * 16.0f);
            field_0x40A += 0x0800;
        }

        if (mHitTimer > 0x100) {
            field_0x410 = 0;
            mHitTimer = 0;
            field_0x408 = 0;
        }
    }

    set_mtx();

    return TRUE;
}

BOOL daObjPlant_c::CreateHeap() {
    // [P1] DN-3 by-index acquirer (donor: raw cast — the §810-2 crash class).
    J3DModelData* modelData =
        dExtNpcMount_acquireModelDataByIndex("Plant", 0x3 /* donor PLANT_BDL_YRMWD */);
    if (modelData == NULL) {
        DuskLog.warn("[ObjPlant] yrmwd.bdl unresolvable/unparseable");
        return FALSE;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);

    if (mpModel != NULL) {
        JUTNameTab* nameTab = mpModel->getModelData()->getJointTree().getJointName();
        for (u16 i = 0; i < mpModel->getModelData()->getJointNum(); i++) {
            if (strcmp("joint2", nameTab->getName(i)) == 0) {
                mpModel->getModelData()->getJointNodePointer(i)->setCallBack(
                    daObjPlant_nodeCallBack);
                break;
            }
        }
        mpModel->setUserArea((uintptr_t)this);   // port: (u32)this -> uintptr_t (ls1 lesson)
    } else {
        return FALSE;
    }

    return TRUE;
}

void daObjPlant_c::CreateInit() {
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    fopAcM_setCullSizeBox(this, -600.0f, -0.0f, -600.0f, 600.0f, 900.0f, 600.0f);
    fopAcM_setCullSizeFar(this, 1.0f);
    mStts.Init(0xFF, 0xFF, this);
    mCyl.Set(l_plantCylSrc);
    mCyl.SetStts(&mStts);
    field_0x408 = 0;
    eyePos = current.pos;
    eyePos.y += 150.0f;
    set_mtx();
}

static int daObjPlant_nodeCallBack(J3DJoint* joint, int calcTiming) {
    if (calcTiming == J3DNodeCBCalcTiming_In) {
        int jntNo = joint->getJntNo();
        J3DModel* model = j3dSys.getModel();
        daObjPlant_c* plant = (daObjPlant_c*)model->getUserArea();

        if (plant != NULL) {
            // donor calc_mtx rotations expressed on the receiver's mtx stack
            // (the npc_tk JointCallBack idiom, incl. the mCurrentMtx write).
            mDoMtx_stack_c::copy(model->getAnmMtx(jntNo));
            mDoMtx_stack_c::XrotM(plant->field_0x40E);
            mDoMtx_stack_c::YrotM(plant->field_0x408);
            mDoMtx_stack_c::XrotM(-plant->field_0x40E);
            model->setAnmMtx(jntNo, mDoMtx_stack_c::get());
            cMtx_copy(mDoMtx_stack_c::get(), J3DSys::mCurrentMtx);
        }
    }
    return 1;
}

void daObjPlant_c::set_mtx() {
    mpModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

static cPhs_Step daObjPlant_Create(void* i_this) {
    return ((daObjPlant_c*)i_this)->_create();
}

static BOOL daObjPlant_Delete(void* i_this) {
    return ((daObjPlant_c*)i_this)->_delete();
}

static BOOL daObjPlant_Draw(void* i_this) {
    return ((daObjPlant_c*)i_this)->_draw();
}

static BOOL daObjPlant_Execute(void* i_this) {
    return ((daObjPlant_c*)i_this)->_execute();
}

static BOOL daObjPlant_IsDelete(void*) {
    return TRUE;
}

static actor_method_class daObj_PlantMethodTable = {
    (process_method_func)daObjPlant_Create,
    (process_method_func)daObjPlant_Delete,
    (process_method_func)daObjPlant_Execute,
    (process_method_func)daObjPlant_IsDelete,
    (process_method_func)daObjPlant_Draw,
};

extern actor_process_profile_definition g_profile_Obj_Plant;

actor_process_profile_definition g_profile_Obj_Plant = {
    // donor g_profile_Obj_Plant (d_a_obj_plant.cpp:228).
    fpcLy_CURRENT_e,           // Layer ID
    7,                         // List ID (donor 0x0007)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_Obj_Plant_e,         // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daObjPlant_c),      // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_Obj_Plant_e,       // Draw Prio
    &daObj_PlantMethodTable,   // Actor SubMtd
    fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_CUSTOM_e,    // Cull Type
};
