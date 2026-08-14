// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_obj_shelf.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW WOODEN SHELF (Obj_Shelf, arc Otana) — §817-2 executed: donor ported
// WHOLE on the receiver's OWN dBgS_MoveBgActor base (the same lineage
// class, natively present — no flattening needed). SOLID GEOMETRY: the
// shelf carries its own DZB and registers as MoveBG; 8 placements in
// Sturgeon's room were missing model AND collision.
//
// SEAMS:
//   [S1] model via the DN-3 by-index acquirer (the §816 rule as
//        precondition); the DZB rides the receiver's own MoveBGCreate
//        path (dBgW consumes the raw DZB natively — the room loader's
//        proven consumption, R5 chunk_format).
//   [S2] DN-1 READ AND CLEAN BY CONSTRUCTION: this TU stamps NO room id
//        anywhere; the shelf spawns from real WW placements whose room
//        comes with the placement, and the receiver's MoveBG base does
//        its own registration exactly as for native TP movebg actors.
//   [S3] donor MoveBGCreate passes dBgS_MoveBGProc_Trans (BG follows
//        TRANSLATION only); receiver setFunc NULL = default full-matrix.
//        Difference is visible only when the shelf TIPS — which is gated
//        on [S4], so inert today; the Trans proc joins with the quake
//        system if the receiver default proves unfaithful then.
//   [S4] quake source: donor dComIfGp_getDetect().chk_quake — WW's
//        dDetect quake propagation is unported; reads false (shelves
//        stand; the tip-over is the story quake beat). Owed with the
//        events/quake pass, LOUD-once on first poll.
//   [S5] SE + story flag through the established shims (WWEV_UNK_0001,
//        JA_SE owed).
// ============================================================

#include "d/actor/d_a_obj_shelf.h"

#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_ww.h"           // §406 WW feeders + TEV_TYPE_* (BG legs)
#include "d/d_ext_npc_mount.h"       // DN-3 consume-time model acquirer
#include "d/d_ext_ww_actor_shims.h"  // WWEV_UNK_0001 etc.
#include "d/actor/d_a_npc_ba1.h"     // hold_event -> the ported Ba1's own hook
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_mtx.h"
#include "m_Do/m_Do_ext.h"
#include "dusk/logging.h"

#define JA_SE_OBJ_RACK_BREAK 0        // [S5] audio pass owed
#define fpcDwPi_Obj_Shelf_e fpcDwPi_E_RD_e

// [S4] WW dDetect quake poll — unported; false = shelves stand (donor
// behavior outside the quake beats). LOUD-once so the gap stays visible.
static bool dObjShelf_chkQuake(const cXyz*) {
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.info("[ObjShelf] [S4] quake poll inert — WW dDetect quake system owed");
    }
    return false;
}

Mtx daObjShelf::Act_c::M_tmp_mtx;

namespace daObjShelf {
    namespace {
        struct Attr_c {
            short mVibDuration;
            short mVibYSpeed;
            float mVibYMagnitude;
            short mVibXSpeed;
            short mVibZSpeed;
            float mVibXMagnitude;
            float mVibZMagnitude;
            float mRotAccel;
            float mRotDecay;
            float mBounceFactor;
            u8 mBounceNum;
            u8 mRotWaitDuration;
            float mInitSpeed2;
            float mInitSpeed3;
        };

        static const Attr_c L_attr = {
            /* mVibDuration     */ 4,
            /* mVibYSpeed       */ 30000,
            /* mVibYMagnitude   */ 4.0f,
            /* mVibXSpeed       */ 30500,
            /* mVibZSpeed       */ 29000,
            /* mVibXMagnitude   */ 200.0f,
            /* mVibZMagnitude   */ 200.0f,
            /* mRotAccel        */ 600.0f,
            /* mRotDecay        */ 0.02f,
            /* mBounceFactor    */ 0.5f,
            /* mBounceNum       */ 5,
            /* mRotWaitDuration */ 0,
            /* mInitSpeed2      */ -3500.0f,
            /* mInitSpeed3      */ -2500.0f,
        };

        inline const Attr_c& attr() { return L_attr; }
    }
}

const char daObjShelf::Act_c::M_arcname[6] = "Otana";

int daObjShelf::Act_c::CreateHeap() {
    // [S1] DN-3: by-index acquirer (donor: raw cast of getObjectRes — the
    // §810-2 crash class; rule applied as precondition).
    J3DModelData* model_data =
        dExtNpcMount_acquireModelDataByIndex(M_arcname, 0x4 /* donor OTANA_BDL_OTANA */);
    if (model_data == NULL) {
        DuskLog.warn("[ObjShelf] otana.bdl unresolvable/unparseable");
        return 0;
    }
    mpModel = mDoExt_J3DModel__create(model_data, 0x80000, 0x11000022);
    if (mpModel == NULL) {
        return 0;
    }
    // §895 (§885 solidity row): pre-repack the donor DZB IN PLACE (§334 seam,
    // idempotent) — the receiver's MoveBGCreateHeap fetches this same buffer
    // raw right after this callback returns, so the translation must land
    // here. WW attCode untranslated reads as TP att1 SINK (§332 measurement).
    {
        cBgD_t* bgd = (cBgD_t*)dComIfG_getObjectRes(M_arcname, 0x7 /* donor OTANA_DZB_OTANA */);
        if (bgd != NULL) {
            dExtWw_repackDzbAttributes(bgd, "otana.dzb");
        }
    }
    return 1;
}

int daObjShelf::Act_c::Create() {
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    init_mtx();
    fopAcM_setCullSizeBox(this, -110.0f, -55.0f, -1.0f, 110.0f, 20.0f, 110.0f);
    mode_wait_init();
    return TRUE;
}

cPhs_Step daObjShelf::Act_c::Mthd_Create() {
    fopAcM_ct(this, Act_c);

    cPhs_Step phase_state = dComIfG_resLoad(&mPhs, M_arcname);
    if (phase_state == cPhs_COMPLEATE_e) {
        // [S1][S3] receiver MoveBGCreate (5-arg): donor's Trans proc slot is
        // the default NULL here; donor heap size 0xb00 verbatim. [S2] no room
        // id is stamped by this TU.
        phase_state = (cPhs_Step)MoveBGCreate(M_arcname, 0x7 /* donor OTANA_DZB_OTANA */,
                                              NULL, 0xb00, NULL);
    }
    return phase_state;
}

int daObjShelf::Act_c::Delete_() {
    return TRUE;
}

BOOL daObjShelf::Act_c::Mthd_Delete() {
    BOOL ret = MoveBGDelete();
    dComIfG_resDelete(&mPhs, M_arcname);
    return ret;
}

void daObjShelf::Act_c::hold_event() const {
    fopAc_ac_c* npc;
    if (fopAcM_SearchByName(fpcNm_NPC_BA1_e, &npc) && npc) {
        ((daNpc_Ba1_c*)npc)->holdEventOn();
    }
}

void daObjShelf::Act_c::mode_wait_init() {
    mMode = 0;
}

void daObjShelf::Act_c::mode_wait() {
    bool quake = dObjShelf_chkQuake(&current.pos);   // [S4]
    bool event = false;
    if (quake && prm_get_groundma() && !dComIfGs_isEventBit(WWEV_UNK_0001)) {
        hold_event();
        quake = false;
        event = true;
    }

    if (quake) {
        mode_vib_init();
    } else if (event) {
        mode_rot_init3();
    }
}

void daObjShelf::Act_c::mode_vib_init() {
    mTimer = attr().mVibDuration;
    mVibY = 0;
    mVibX = 0;
    mVibZ = 0;
    mMode = 1;
}

void daObjShelf::Act_c::mode_vib() {
    if (--mTimer <= 0) {
        current.pos.y = home.pos.y;
        shape_angle.x = home.angle.x;
        shape_angle.z = home.angle.z;
        mode_rot_init();
    } else {
        mVibY += attr().mVibYSpeed;
        current.pos.y = home.pos.y + cM_ssin(mVibY) * attr().mVibYMagnitude;
        mVibX += attr().mVibXSpeed;
        mVibZ += attr().mVibZSpeed;
        shape_angle.x = home.angle.x + (short)(cM_ssin(mVibX) * attr().mVibXMagnitude);
        shape_angle.z = home.angle.z + (short)(cM_ssin(mVibZ) * attr().mVibZMagnitude);
    }
}

void daObjShelf::Act_c::mode_rot_init() {
    mCurBounce = attr().mBounceNum + 1;
    mRotSpeed = 0.0f;
    mTimer = attr().mRotWaitDuration;
    mTargetAngle = 0x4000;
    m2e7 = false;
    mMode = 2;
}

void daObjShelf::Act_c::mode_rot_init2() {
    mCurBounce = attr().mBounceNum + 1;
    mRotSpeed = attr().mInitSpeed2;
    mTimer = 0;
    mTargetAngle = 0x4000;
    m2e7 = false;
    mMode = 2;
}

void daObjShelf::Act_c::mode_rot_init3() {
    mCurBounce = attr().mBounceNum + 1;
    mRotSpeed = attr().mInitSpeed3;
    mTimer = 0;
    mTargetAngle = 0;
    m2e7 = true;
    mMode = 2;
}

void daObjShelf::Act_c::mode_rot() {
    if (mTimer > 0) {
        mTimer--;
        return;
    }
    mRotSpeed += attr().mRotAccel;
    mRotSpeed -= mRotSpeed * attr().mRotDecay;
    shape_angle.x += (short)(int)mRotSpeed;
    if (shape_angle.x > mTargetAngle) {
        if (--mCurBounce <= 0) {
            shape_angle.x = mTargetAngle;
            if (m2e7) {
                mode_wait_init();
            } else {
                mode_fell_init();
            }
        } else {
            // donor: bounce back with decay; first bounce plays the break SE.
            shape_angle.x = mTargetAngle -
                            (int)((short)(int)(shape_angle.x - mTargetAngle) * attr().mBounceFactor);
            mRotSpeed = mRotSpeed * -attr().mBounceFactor;
            if (mCurBounce == attr().mBounceNum) {
                fopAcM_seStart(this, JA_SE_OBJ_RACK_BREAK, 0);   // [S5] 0 = silent
            }
        }
    }
}

void daObjShelf::Act_c::mode_fell_init() {
    mMode = 3;
}

void daObjShelf::Act_c::mode_fell() {
    if (dObjShelf_chkQuake(&current.pos)) {   // [S4]
        mode_rot_init2();
    }
}

void daObjShelf::Act_c::set_mtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::ZXYrotM(shape_angle.x, shape_angle.y, shape_angle.z);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    cMtx_copy(mDoMtx_stack_c::get(), M_tmp_mtx);
}

void daObjShelf::Act_c::init_mtx() {
    mpModel->setBaseScale(scale);
    set_mtx();
}

int daObjShelf::Act_c::Execute(Mtx** matrix) {
    typedef void (daObjShelf::Act_c::*procFunc)();
    static procFunc mode_proc[] = {
        &daObjShelf::Act_c::mode_wait,
        &daObjShelf::Act_c::mode_vib,
        &daObjShelf::Act_c::mode_rot,
        &daObjShelf::Act_c::mode_fell,
    };
    (this->*mode_proc[mMode])();

    set_mtx();

    *matrix = &M_tmp_mtx;
    return TRUE;
}

int daObjShelf::Act_c::Draw() {
    // donor: settingTevStruct(TEV_TYPE_BG0) — the WW feeders carry the BG
    // legs (§406, d_kankyo_ww.cpp:481).
    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mpModel, &tevStr);
    dComIfGd_setListBG();
    mDoExt_modelUpdateDL(mpModel);
    dComIfGd_setList();
    return TRUE;
}

namespace daObjShelf {
namespace {
cPhs_Step Mthd_Create(void* i_this) {
    return static_cast<Act_c*>(i_this)->Mthd_Create();
}

BOOL Mthd_Delete(void* i_this) {
    return static_cast<Act_c*>(i_this)->Mthd_Delete();
}

BOOL Mthd_Execute(void* i_this) {
    return static_cast<Act_c*>(i_this)->MoveBGExecute();
}

BOOL Mthd_Draw(void* i_this) {
    return static_cast<Act_c*>(i_this)->MoveBGDraw();
}

BOOL Mthd_IsDelete(void* i_this) {
    return static_cast<Act_c*>(i_this)->MoveBGIsDelete();
}

static actor_method_class Mthd_Table = {
    (process_method_func)Mthd_Create,
    (process_method_func)Mthd_Delete,
    (process_method_func)Mthd_Execute,
    (process_method_func)Mthd_IsDelete,
    (process_method_func)Mthd_Draw,
};
};  // namespace
};  // namespace daObjShelf

extern actor_process_profile_definition g_profile_Obj_Shelf;

actor_process_profile_definition g_profile_Obj_Shelf = {
    // donor g_profile_Obj_Shelf (d_a_obj_shelf.cpp:303).
    fpcLy_CURRENT_e,           // Layer ID
    3,                         // List ID (donor 0x0003)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_Obj_Shelf_e,         // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daObjShelf::Act_c), // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_Obj_Shelf_e,       // Draw Prio
    &daObjShelf::Mthd_Table,   // Actor SubMtd
    fopAcStts_CULL_e | fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_CUSTOM_e,    // Cull Type
};
