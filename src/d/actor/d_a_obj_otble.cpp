/**
 * d_a_obj_otble.cpp
 * Object - Generic brown wooden tables (Orca/Sturgeon house furniture)
 *
 * ============================================================================
 * §329 WW WOODEN TABLE (Obj_Otble) — ACTOR-KIT PORT, body VERBATIM from the WW
 * donor (D:/XXXXXXX/WW DP/src/d/actor/d_a_obj_otble.cpp). Deltas (lamp §327 /
 * toripost §253 idiom set), each bannered inline:
 *   #1 DN-3  : model via dExtNpcMount_acquireModelData("Okmono", <bdl>) —
 *              donor dRes_INDEX_OKMONO_BDL_OTABLE(L)_e == otable.bdl/otablel.bdl
 *              (staged Okmono.arc RARC parse: ostool/otable/otablel.bdl +
 *              otble/otble_l.dzb). Collision dzb by NAME via dComIfG_getObjectRes
 *              — the same call the mount collision path uses (d_ext_npc_mount
 *              .cpp:3357 precedent).
 *   #2 ct    : fopAcM_ct_Retail/ct_Demo -> port fopAcM_ct (ba1 §261 call form)
 *   #3 cPhs  : cPhs_State -> cPhs_Step (codemod AUTO)
 *   #4 assert: JUT_ASSERT -> NULL-guard (port assert panics; donor retail
 *              assert compiles out — guard is the faithful behavior)
 *   #5 prio  : fpcDwPi_OBJ_OTBLE_e absent -> port E_RD slot (lamp precedent)
 *   #6 DN-1  : mBgW Regist is donor-verbatim; the actor is LANDED INERT (no
 *              OBJNAME row) — ACTIVATION gated on the user's DN-1 ruling.
 *              Room id at Regist time = loader-stamped actor room from the
 *              data-authored ACTR row (positively resolved, never guessed).
 * ============================================================================
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep
#include "d/actor/d_a_obj_otble.h"
#include "d/d_bg_w.h"
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"                // mDoExt_J3DModel__create / modelUpdateDL
#include "d/d_ext_npc_mount.h"            // §329 DN-3 parse-at-consume model resolver
#include "d/d_ext_ww_actor_shims.h"       // §329 fopAcStts_NOCULLEXEC_e

// §329 #5: donor draw-prio slot absent -> port E_RD slot (lamp/toripost precedent).
#define fpcDwPi_OBJ_OTBLE_e fpcDwPi_E_RD_e

const daObj_Otble::Attr_c daObj_Otble::Act_c::M_attr = {0, 0};

static BOOL createHeap_CB(fopAc_ac_c* a_this) {
    return ((daObj_Otble::Act_c*)a_this)->_createHeap();
}

void daObj_Otble::Act_c::set_mtx() {
    mObjAcch.CrrPos(dComIfG_Bgsp());  // §329 port Bgsp() returns reference
    tevStr.room_no = current.roomNo;  // §329 TP member name (alink :5711)
    tevStr.YukaCol = dComIfG_Bgsp().GetPolyColor(mObjAcch.m_gnd);  // §329 TP member name (alink :5722)
    mModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mModel->setBaseTRMtx(mDoMtx_stack_c::get());
    cMtx_copy(mDoMtx_stack_c::get(), m2A4);
    mBgW->Move();
}

BOOL daObj_Otble::Act_c::_execute() {
    if (attr()->m00) {
        return TRUE;
    }
    fopAcM_posMoveF(this, NULL);
    set_mtx();
    return TRUE;
}

BOOL daObj_Otble::Act_c::_draw() {
    static const f32 size[] = {1.0f, 2.0f};

    if (attr()->m01) {
        return TRUE;
    }

    g_env_light.settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    g_env_light.setLightTevColorType(mModel, &tevStr);

    dComIfGd_setListBG();
    mDoExt_modelUpdateDL(mModel);
    dComIfGd_setList();

    // §329: port GetTriPla is bool(polyInfo, cM3dGPla*) (alink :5770 idiom) —
    // WW's returns-pointer form adapted; same plane normal feeds the shadow.
    f32 fVar1 = mObjAcch.GetGroundH();
    cM3dGPla plane;
    if (fVar1 != -G_CM3D_F_INF && dComIfG_Bgsp().GetTriPla(mObjAcch.m_gnd, &plane)) {
        dComIfGd_setSimpleShadow(&current.pos, fVar1, 100.0f, &plane.mNormal, current.angle.y, size[m294], NULL);
    }
    return TRUE;
}

BOOL daObj_Otble::Act_c::_createHeap() {
    // §329 #1 (DN-3): donor res-INDEX loads become by-NAME through the mount
    // resolver (staged donor Okmono.arc member names, RARC-verified).
    static const char* bdl[] = {"otable.bdl", "otablel.bdl"};
    static const char* dzb[] = {"otble.dzb", "otble_l.dzb"};

    J3DModelData* modelData = dExtNpcMount_acquireModelData("Okmono", bdl[m294]);
    if (modelData == NULL) {  // §329 #4 (donor JUT_ASSERT 191)
        return false;
    }

    mModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
    if (mModel == NULL) {
        return false;
    }

    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    cMtx_copy(mDoMtx_stack_c::get(), m2A4);

    mBgW = new dBgW();
    if (mBgW == NULL) {
        return false;
    }

    void* dzbRes = dComIfG_getObjectRes("Okmono", dzb[m294]);
    if (dzbRes == NULL) {  // §329 #4
        return false;
    }
    // §334b: THE missed consumption site — otble Set its WW dzb un-repacked, so
    // Link's FIRST ground contact still read WW WOOD as TP att1=2 SINK (the
    // half-fixed "lava anim then recovers" report, 2026-08-01). Same repack as
    // every other WW dzb consumer, before Set.
    dExtWw_repackDzbAttributes((cBgD_t*)dzbRes, dzb[m294]);
    if (mBgW->Set((cBgD_t*)dzbRes, dBgW::MOVE_BG_e, &m2A4) == TRUE) {
        return false;
    }
    return true;
}

void daObj_Otble::Act_c::CreateInit() {
    // §329 #6 (DN-1): donor-verbatim Regist — actor room is the loader-stamped
    // room of the data-authored placement (positively resolved). INERT until
    // the OBJNAME activation lands (user-gated).
    dComIfG_Bgsp().Regist(mBgW, this);

    mObjAcchCir.SetWall(30.0f, 30.0f);
    mObjAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1, &mObjAcchCir, fopAcM_GetSpeed_p(this), NULL, NULL);
    mObjAcch.SetWallNone();
    mObjAcch.SetWaterNone();
    mObjAcch.SetRoofNone();

    fopAcM_SetGravity(this, -6.5f);
    fopAcM_posMoveF(this, NULL);
    set_mtx();
    fopAcM_SetMtx(this, mModel->getBaseTRMtx());

    if (m294 == 1) {
        fopAcM_setCullSizeBox(this, -100.0f, -0.0f, -200.0f, 100.0f, 150.0f, 200.0f);
    } else {
        fopAcM_setCullSizeBox(this, -100.0f, -0.0f, -100.0f, 100.0f, 150.0f, 100.0f);
    }
    fopAcM_setCullSizeFar(this, 10.0f);
}

cPhs_Step daObj_Otble::Act_c::_create() {
    static const u32 heapsize[] = {0x1240, 0x1240};

    fopAcM_ct(this, daObj_Otble::Act_c);  // §329 #2 (donor ct_Retail form)
    m294 = fopAcM_GetParam(this) & 0xff;
    if (m294 > 1) {
        m294 = 1;
    }

    cPhs_Step ret = dComIfG_resLoad(&mPhase, "Okmono");
    if (ret == cPhs_COMPLEATE_e) {
        if (!fopAcM_entrySolidHeap(this, createHeap_CB, heapsize[m294])) {
            return cPhs_ERROR_e;
        }
        CreateInit();
    }
    return ret;
}

bool daObj_Otble::Act_c::_delete() {
    dComIfG_resDelete(&mPhase, "Okmono");
    if (mBgW != NULL && mBgW->ChkUsed()) {
        dComIfG_Bgsp().Release(mBgW);
    }
    return true;
}

cPhs_Step daObj_Otble::Mthd::Create(void* v_this) {
    return ((daObj_Otble::Act_c*)v_this)->_create();
}

BOOL daObj_Otble::Mthd::Delete(void* v_this) {
    return ((daObj_Otble::Act_c*)v_this)->_delete();
}

BOOL daObj_Otble::Mthd::Execute(void* v_this) {
    return ((daObj_Otble::Act_c*)v_this)->_execute();
}

BOOL daObj_Otble::Mthd::Draw(void* v_this) {
    return ((daObj_Otble::Act_c*)v_this)->_draw();
}

BOOL daObj_Otble::Mthd::IsDelete(void*) {
    return TRUE;
}

actor_method_class daObj_Otble::Mthd::Table = {
    (process_method_func)daObj_Otble::Mthd::Create,
    (process_method_func)daObj_Otble::Mthd::Delete,
    (process_method_func)daObj_Otble::Mthd::Execute,
    (process_method_func)daObj_Otble::Mthd::IsDelete,
    (process_method_func)daObj_Otble::Mthd::Draw,
};

actor_process_profile_definition g_profile_OBJ_OTBLE = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0003,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_OBJ_OTBLE_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daObj_Otble::Act_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_OBJ_OTBLE_e,  // §329 #5
    /* Actor SubMtd */ &daObj_Otble::Mthd::Table,
    /* Status       */ fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
