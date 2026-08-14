// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_obj_paper.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW READABLE PROPS (Obj_Paper: Opaper paper / Ppos poster / Piwa tablet)
// — §817-4 row 1, donor WHOLE: the 3-type attr table verbatim, the 4-mode
// talk machine (wait/talk0/getmsg/talkwait) reading WW message ids through
// the PORTED WW dialogue system, Piwa's hit-feedback cc.
// SEAMS: [Q1] model via the DN-3 by-index acquirer; [Q2] talk flow in the
// §239 port dialect (msg_class::mode, fopMsg_MODE_*, messageSet(no,this,
// 1000) — the d_npc talk() internals, lifted at the boundary because the
// donor's actor is NOT an fopNpc); [Q3] cc Tg masks re-expressed in
// receiver bits (trap #5; Piwa-only, Outset places none); [Q4] the donor
// TALKFLAG_READ attention bit re-supplied at its donor value.
// ============================================================

#include "d/actor/d_a_obj_paper.h"

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_ww.h"
#include "d/d_a_obj.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_ww_actor_shims.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_msg_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "dusk/logging.h"

#define fpcDwPi_Obj_Paper_e fpcDwPi_E_RD_e
// [Q4] WW f_op_actor.h:88 — the "readable" attention class; port value verbatim.
#define fopAc_Attn_TALKFLAG_READ_e 0x40000000

namespace daObjPaper {

namespace {
    struct Attr_c {
        const char* mResName;
        s32 mHeapSize;
        s16 mModelId;
        s16 mEyeOffset;
        s16 mAttentionOffset;
        s16 mCullSphereRadius;
        s16 mCullSphereYOffset;
        s8 mAttentionDist1;
        s8 mAttentionDist2;
        u8 mTevType;
        s16 mColCylinderRadius;
        s16 mColCylinderHeight;
    };

    // donor L_attr verbatim (retail DEMO_SELECT branch).
    static const Attr_c L_attr[] = {
        {
            /* mResName           */ "Opaper",
            /* mHeapSize          */ 0x04C0,
            /* mModelId           */ 0x3,   // donor OPAPER_BDL_OPAPER
            /* mEyeOffset         */ 0x00,
            /* mAttentionOffset   */ 0x28,
            /* mCullSphereRadius  */ 0x28,
            /* mCullSphereYOffset */ 0x00,
            /* mAttentionDist1    */ 0x1D,
            /* mAttentionDist2    */ 0x1E,
            /* mTevType           */ 0x01,
            /* mColCylinderRadius */ 0x00,
            /* mColCylinderHeight */ 0x00,
        },
        {
            /* mResName           */ "Ppos",
            /* mHeapSize          */ 0x04C0,
            /* mModelId           */ 0x3,   // donor PPOS_BDL_PPOS
            /* mEyeOffset         */ 0x00,
            /* mAttentionOffset   */ 0x32,
            /* mCullSphereRadius  */ 0x3C,
            /* mCullSphereYOffset */ 0x00,
            /* mAttentionDist1    */ 0x1F,
            /* mAttentionDist2    */ 0x20,
            /* mTevType           */ 0x00,
            /* mColCylinderRadius */ 0x00,
            /* mColCylinderHeight */ 0x00,
        },
        {
            /* mResName           */ "Piwa",
            /* mHeapSize          */ 0x04C0,
            /* mModelId           */ 0x3,   // donor PIWA_BDL_PIWA
            /* mEyeOffset         */ 0x3C,
            /* mAttentionOffset   */ 0x82,
            /* mCullSphereRadius  */ 0x50,
            /* mCullSphereYOffset */ 0x3C,
            /* mAttentionDist1    */ 0x1D,
            /* mAttentionDist2    */ 0x1E,
            /* mTevType           */ 0x00,
            /* mColCylinderRadius */ 0x37,
            /* mColCylinderHeight */ 0x73,
        }
    };

    inline const Attr_c& attr(Type_e type) { return L_attr[type]; }
}

// [Q3] donor M_cyl_src semantics in the receiver initializer shape: At none;
// Tg accepts the attack families (receiver mask, the obj_carry-proven
// expression of "everything except water/wind/light classes" — trap #5);
// Co Set|IsOther|VsGrpAll (0x79, same value both lineages); Tg hit-feedback
// detail bits (donor Shield|NoConHit) carried as the no-hit-mark family —
// exactness joins the cc pass (Piwa-only surface, no Outset placements).
const dCcD_SrcCyl Act_c::M_cyl_src = {
    {
        {0x0, {{0, 0, 0}, {0xd8fbfdff, 0x1f}, 0x79}},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x4},
        {0x0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            0.0f,
            0.0f,
        }
    }
};

int Act_c::solidHeapCB(fopAc_ac_c* i_this) {
    return static_cast<Act_c*>(i_this)->create_heap();
}

bool Act_c::create_heap() {
    // [Q1] DN-3 by-index acquirer (donor raw cast = the §810-2 crash class).
    J3DModelData* mdl_data =
        dExtNpcMount_acquireModelDataByIndex(attr(mType).mResName, attr(mType).mModelId);
    if (mdl_data == NULL) {
        DuskLog.warn("[ObjPaper] '{}' model idx {} unresolvable", attr(mType).mResName,
                     (int)attr(mType).mModelId);
        return false;
    }

    mpModel = mDoExt_J3DModel__create(mdl_data, 0x80000, 0x11000022);
    return mpModel != NULL;
}

cPhs_Step Act_c::_create() {
    fopAcM_ct(this, Act_c);

    mType = prm_get_type();

    cPhs_Step result = dComIfG_resLoad(&mPhs, attr(mType).mResName);

    if (result == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(this, solidHeapCB, attr(mType).mHeapSize)) {
            eyePos.y += attr(mType).mEyeOffset;

            attention_info.position.y += attr(mType).mAttentionOffset;
            attention_info.distances[fopAc_Attn_TYPE_TALK_e] = attr(mType).mAttentionDist1;
            attention_info.distances[fopAc_Attn_TYPE_SPEAK_e] = attr(mType).mAttentionDist2;
            cLib_onBit<u32>(attention_info.flags,
                            fopAc_Attn_LOCKON_TALK_e | fopAc_Attn_ACTION_SPEAK_e |
                                fopAc_Attn_TALKFLAG_READ_e);

            mMsgId = fpcM_ERROR_PROCESS_ID_e;

            if (mType == Piwa_e) {
                fopAcM_SetStatusMap(this, 0x18);
            }

            if (attr(mType).mColCylinderRadius != 0) {
                mbHasCc = true;

                mColStatus.Init(0xFF, 0xFF, this);
                mCylinderCol.Set(M_cyl_src);
                mCylinderCol.SetStts(&mColStatus);
                mCylinderCol.SetR(attr(mType).mColCylinderRadius);
                mCylinderCol.SetH(attr(mType).mColCylinderHeight);
            } else {
                mbHasCc = false;
            }

            fopAcM_setCullSizeSphere(this, 0.0f, attr(mType).mCullSphereYOffset, 0.0f,
                                     attr(mType).mCullSphereRadius);
            fopAcM_SetMtx(this, mpModel->getBaseTRMtx());

            init_mtx();
            mode_wait_init();
        } else {
            result = cPhs_ERROR_e;
        }
    }

    return result;
}

bool Act_c::_delete() {
    dComIfG_resDelete(&mPhs, attr(mType).mResName);
    return TRUE;
}

void Act_c::mode_wait_init() {
    fopAcM_OnStatus(this, fopAcStts_NOCULLEXEC_e);
    mMode = ActMode_WAIT_e;
}

void Act_c::mode_wait() {
    // [Q2] donor eventInfo.mCommand == dEvtCmd_INTALK_e -> port accessor.
    if (eventInfo.checkCommandTalk()) {
        mode_talk0_init();
    } else {
        eventInfo.onCondition(dEvtCnd_CANTALK_e);
    }
}

void Act_c::mode_talk0_init() {
    fopAcM_OffStatus(this, fopAcStts_NOCULLEXEC_e);
    mMsgId = fpcM_ERROR_PROCESS_ID_e;
    mMode = ActMode_TALKBEGIN_e;
}

void Act_c::mode_talk0() {
    // donor gates the message on the talk-camera settling (dCamAttnStts bit 4).
    if (mMsgId == fpcM_ERROR_PROCESS_ID_e &&
        dComIfGp_checkCameraAttentionStatus(dComIfGp_getPlayerCameraID(0), 4)) {
        // [Q2] port messageSet arity: (msgIdx, talkActor, 1000) — the §239
        // dialect every port call site uses (d_npc.cpp:692).
        mMsgId = fopMsgM_messageSet(prm_get_msgNo(), this, 1000);

        mode_talk1_init();
    }
}

void Act_c::mode_talk1_init() {
    mMode = ActMode_GETMSG_e;
}

void Act_c::mode_talk1() {
    mpMsg = fopMsgM_SearchByID(mMsgId);
    if (mpMsg) {
        mode_talk2_init();
    }
}

void Act_c::mode_talk2_init() {
    mMode = ActMode_TALKWAIT_e;
}

void Act_c::mode_talk2() {
    // [Q2] donor mStatus/fopMsgStts_* -> port mode/fopMsg_MODE_* (§239).
    if (mpMsg->mode == fopMsg_MODE_BOX_CLOSED_e) {
        mpMsg->mode = fopMsg_MODE_MSG_DESTROYED_e;
        mpMsg = NULL;
        mMsgId = fpcM_ERROR_PROCESS_ID_e;

        dComIfGp_event_reset();
        mode_wait_init();
    }
}

void Act_c::set_mtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::ZXYrotM(shape_angle.x, shape_angle.y, shape_angle.z);

    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

void Act_c::init_mtx() {
    mpModel->setBaseScale(scale);
    set_mtx();
}

void Act_c::damage_cc_proc() {
    u32 hitResult = mCylinderCol.ChkTgHit();
    if (hitResult) {
        daObj::HitSeStart(&eyePos, current.roomNo, &mCylinderCol, 0x0D);
        dKy_Sound_set(current.pos, 4, fopAcM_GetID(this), 100);

        // §877 correction: receiver daObj has NO HitEff_kikuzu — the §253
        // no-op shim carries the call (sawdust FX owed with the WW dPa pass).
        dExtTpost_HitEff_kikuzu(this, &mCylinderCol);

        mCylinderCol.ClrTgHit();
    }

    mColStatus.Move();
}

typedef void (Act_c::*daObjPaper_mode_t)(void);

bool Act_c::_execute() {
    static const daObjPaper_mode_t mode_proc[] = {
        &Act_c::mode_wait,
        &Act_c::mode_talk0,
        &Act_c::mode_talk1,
        &Act_c::mode_talk2,
    };

    if (mbHasCc) {
        damage_cc_proc();
    }

    (this->*mode_proc[mMode])();

    set_mtx();
    if (mbHasCc) {
        mCylinderCol.SetC(current.pos);
        dComIfG_Ccsp()->Set(&mCylinderCol);
    }

    return true;
}

bool Act_c::_draw() {
    dKyWw_settingTevStruct(attr(mType).mTevType == 0 ? TEV_TYPE_BG0 : TEV_TYPE_ACTOR,
                           &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mpModel, &tevStr);

    mDoExt_modelUpdateDL(mpModel);

    return TRUE;
}

namespace {
static cPhs_Step Mthd_Create(void* i_this) {
    return static_cast<Act_c*>(i_this)->_create();
}

static BOOL Mthd_Delete(void* i_this) {
    return static_cast<Act_c*>(i_this)->_delete();
}

static BOOL Mthd_Execute(void* i_this) {
    return static_cast<Act_c*>(i_this)->_execute();
}

static BOOL Mthd_Draw(void* i_this) {
    return static_cast<Act_c*>(i_this)->_draw();
}

static BOOL Mthd_IsDelete(void*) {
    return TRUE;
}

static actor_method_class Mthd_Table = {
    (process_method_func)Mthd_Create,
    (process_method_func)Mthd_Delete,
    (process_method_func)Mthd_Execute,
    (process_method_func)Mthd_IsDelete,
    (process_method_func)Mthd_Draw,
};
};  // namespace

};  // namespace daObjPaper

extern actor_process_profile_definition g_profile_Obj_Paper;

actor_process_profile_definition g_profile_Obj_Paper = {
    // donor g_profile_Obj_Paper (d_a_obj_paper.cpp:360).
    fpcLy_CURRENT_e,             // Layer ID
    7,                           // List ID (donor 0x0007)
    fpcPi_CURRENT_e,             // List Prio
    fpcNm_Obj_Paper_e,           // Proc Name
    &g_fpcLf_Method.base,        // Proc SubMtd
    sizeof(daObjPaper::Act_c),   // Size
    0,                           // Size Other
    0,                           // Parameters
    &g_fopAc_Method.base,        // Leaf SubMtd
    fpcDwPi_Obj_Paper_e,         // Draw Prio
    &daObjPaper::Mthd_Table,     // Actor SubMtd
    fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,               // Group
    fopAc_CULLSPHERE_CUSTOM_e,   // Cull Type
};
