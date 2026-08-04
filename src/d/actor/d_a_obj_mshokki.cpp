/**
 * d_a_obj_mshokki.cpp
 * Object - Dishes and cutlery inside Forsaken Fortress (MPot/MOsara/MKoppu)
 *
 * ============================================================================
 * §327 WW TABLEWARE (Obj_Mshokki) DIRECT PORT — body VERBATIM from the WW donor
 * (D:/XXXXXXX/WW DP/src/d/actor/d_a_obj_mshokki.cpp). One actor, three shapes:
 * OBJNAME rows MPot/MOsara/MKoppu pass arg 0/1/2 -> m60C (pot/osara/koppu).
 * VERSION==VERSION_DEMO blocks follow the RETAIL (#else) side, like every landed
 * WW port: the JORReflexible HIO class is dropped, namespace L_HIO carries the
 * retail constants, HIO(x) maps to L_HIO::x (the donor m_Do_hostIO.h retail
 * macro, reproduced locally).
 * Direct-port crash recipes applied at the call sites (banners inline),
 * mirroring toripost §253 / ba1 §261:
 *   #1 DN-3  : models via dExtNpcMount_acquireModelData("Mshokki", <bdl name>)
 *              (donor bdl_idx {POT=5, OSARA=4, KOPPU=3} == pot/osara/koppu.bdl —
 *              RARC parse of D:/XXXXXXX/Ex WW/files/res/Object/Mshokki.arc)
 *   particle_set 8-arg -> 10-arg ; *dComIfG_Bgsp() -> reference form
 *   tevStr.mColorK0 -> tevStr.TevKColor (port dKy_tevstr_c field name)
 *   StartCTg absent on the port's dCcD_Cyl -> SetC + SetTgVec(zero) locally
 *   (VERBATIM WW dCcD_Cyl::StartCTg body, d_cc_d.cpp:295)
 *   JUT_ASSERT -> DuskLog + donor fallback (the port macro PANICS; the donor
 *   RETAIL assert compiles out, so log+fallback is the faithful behavior)
 *   cPhs_State -> cPhs_Step ; fopAcM_IsFirstCreating -> fpcM_IsFirstCreating
 * WW SE ids JA_SE_OBJ_* are VALUE-FAITHFUL local defines (toripost §253
 * precedent — donor numbering kept, no port SE bank remap).
 * ============================================================================
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep
#include "d/actor/d_a_obj_mshokki.h"
#include <cstdlib>                        // §327 abs() (donor reached it transitively)
#include "d/d_kankyo.h"
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"            // §327 fpcM_IsFirstCreating (port lacks the fopAcM_ wrapper)
#include "m_Do/m_Do_mtx.h"
#include "m_Do/m_Do_ext.h"                // §327 mDoExt_J3DModel__create / modelUpdateDL
#include "SSystem/SComponent/c_math.h"    // §327 cM_atan2s / cM_ssin / cM_scos
#include "d/d_particle_name.h"            // §327 ID_AK_JN_BREAKMAJU*00 (native port enum, unscoped)
#include "d/d_ext_npc_mount.h"            // §327 DN-3 parse-at-consume model resolver
#include "d/d_ext_ww_actor_shims.h"       // §327 AT_TYPE_* / cCcD_* / dCcG_* WW values
#include "dusk/logging.h"                 // §327 retail-assert fallbacks log instead of panicking
#include "d/d_kankyo_ww.h"           // §404 WW lighting write-path (was the empty stub)

// §327 ========================================================================
// WW-absent VALUE-FAITHFUL constants (donor numbering; toripost §253 pattern —
// kept LOCAL to this TU, not the shared shims header).
// ----------------------------------------------------------------------------
// WW SSystem/SComponent/c_cc_d.h SPrm bits the shims header does not carry.
#define cCcD_TgSPrm_IsOther_e  0x08
#define cCcD_CoSPrm_VsPlayer_e 0x20
#define cCcD_CoSPrm_VsOther_e  0x40
// WW d/d_cc_d.h Tg SPrm bit (toripost §253 defined the same value locally).
#define dCcG_TgSPrm_NoConHit_e 0x02
// WW JAZelAudio_SE.h ids — value-faithful (toripost §253 precedent; the WW SE
// bank is not loaded, a later audio pass maps real Z2SE ids).
#define JA_SE_OBJ_COL_SWC_CHNS 0x6806
#define JA_SE_OBJ_CUP_SWING    0x69E7
#define JA_SE_OBJ_DISH_SWING   0x69E8
// Donor draw-prio slot absent -> port ground/ambient slot (pig/toripost precedent).
#define fpcDwPi_Obj_Mshokki_e fpcDwPi_E_RD_e
// Donor m_Do_hostIO.h RETAIL side: #define HIO(name) L_HIO::name.
#define HIO(name) L_HIO::name
// §327 ========================================================================

// §327 VERSION==VERSION_DEMO JORReflexible HIO dropped — RETAIL (#else) side kept.
namespace L_HIO {
static const f32 m08 = 20.0f;
static const f32 m0C = 20.0f;
static const f32 m10 = 128.0f;
static const f32 m14 = 3072.0f;
static const f32 m18 = 4096.0f;
static const f32 m1C = 6.0f;
static const f32 m20 = 11.0f;
static const f32 m24 = 4096.0f;
static const f32 m28 = 1.0f;
static const f32 m2C = 24.0f;
static const f32 m30 = 0.0f;
static const f32 m34 = 0.5f;
static const f32 m38 = 0.8f;
static const f32 m3C = 0.1f;
} // namespace L_HIO

namespace {
static const char l_arcname[] = "Mshokki";
static const dCcD_SrcCyl l_cyl_src = {
    // dCcD_SrcGObjInf
    {
        /* Flags             */ 0,
        /* SrcObjAt  Type    */ 0,
        /* SrcObjAt  Atp     */ 0,
        /* SrcObjAt  SPrm    */ 0,
        /* SrcObjTg  Type    */ AT_TYPE_ALL & ~AT_TYPE_WATER & ~AT_TYPE_UNK20000 & ~AT_TYPE_UNK400000 & ~AT_TYPE_LIGHT,
        /* SrcObjTg  SPrm    */ cCcD_TgSPrm_Set_e | cCcD_TgSPrm_IsOther_e,
        /* SrcObjCo  SPrm    */ cCcD_CoSPrm_Set_e | cCcD_CoSPrm_IsOther_e | cCcD_CoSPrm_VsEnemy_e | cCcD_CoSPrm_VsPlayer_e | cCcD_CoSPrm_VsOther_e,
        /* SrcGObjAt Se      */ 0,
        /* SrcGObjAt HitMark */ dCcG_AtHitMark_None_e,
        /* SrcGObjAt Spl     */ dCcG_At_Spl_UNK0,
        /* SrcGObjAt Mtrl    */ 0,
        /* SrcGObjAt SPrm    */ 0,
        /* SrcGObjTg Se      */ 0,
        /* SrcGObjTg HitMark */ 0,
        /* SrcGObjTg Spl     */ dCcG_Tg_Spl_UNK0,
        /* SrcGObjTg Mtrl    */ 0,
        /* SrcGObjTg SPrm    */ dCcG_TgSPrm_NoConHit_e | dCcG_TgSPrm_NoHitMark_e,
        /* SrcGObjCo SPrm    */ 0,
    },
    // cM3dGCylS
    {{
        /* Center */ {0.0f, 0.0f, 0.0f},
        /* Radius */ 35.0f,
        /* Height */ 50.0f,
    }},
};

struct LData {
    /* 0x00 */ f32 m00;
    /* 0x04 */ f32 radius;
    /* 0x08 */ f32 height;
    /* 0x0C */ f32 m0C;
}; // size = 0x10

static const LData l_data[] = {
    {20.0f, 15.0f, 40.0f, 6144.0f},
    {7.5f, 30.0f, 15.0f, 2048.0f},
    {15.0f, 15.0f, 30.0f, 8192.0f},
};
} // namespace

/* 00000078-00000108       .text set_mtx__14daObjMshokki_cFv */
void daObjMshokki_c::set_mtx() {
    mModel->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(shape_angle.y);
    mDoMtx_stack_c::XrotM(shape_angle.x);
    mModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

/* 00000108-0000012C       .text solidHeapCB__14daObjMshokki_cFP10fopAc_ac_c */
BOOL daObjMshokki_c::solidHeapCB(fopAc_ac_c* a_this) {
    return ((daObjMshokki_c*)a_this)->create_heap();
}

/* 0000012C-000001FC       .text create_heap__14daObjMshokki_cFv */
bool daObjMshokki_c::create_heap() {
    // §327 DN-3 (recipe #1): donor indexes the arc by res id —
    //   static s32 bdl_idx[] = {dRes_INDEX_MSHOKKI_BDL_POT_e /*5*/,
    //                           dRes_INDEX_MSHOKKI_BDL_OSARA_e /*4*/,
    //                           dRes_INDEX_MSHOKKI_BDL_KOPPU_e /*3*/};
    // The port acquires by member NAME through the parse-at-consume resolver
    // (id->name mapping RARC-parsed from the donor Mshokki.arc: 5=pot.bdl,
    // 4=osara.bdl, 3=koppu.bdl).
    static const char* bdl_name[] = {"pot.bdl", "osara.bdl", "koppu.bdl"};

    bool uVar3 = true;
    J3DModelData* modelData = dExtNpcMount_acquireModelData(l_arcname, bdl_name[m60C]);
    if (modelData == NULL) {
        // §327 donor JUT_ASSERT(356, FALSE) — retail assert compiles out; the
        // port macro would PANIC, so log + donor fallback (uVar3 = false).
        DuskLog.warn("[Mshokki] §327 create_heap: '{}' member '{}' not resolvable", l_arcname,
                     bdl_name[m60C]);
        uVar3 = false;
    } else {
        mModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
        if (mModel == NULL) {
            uVar3 = false;
        }
    }
    return uVar3;
}

/* 000001FC-0000032C       .text checkCollision__14daObjMshokki_cFv */
bool daObjMshokki_c::checkCollision() {
    bool uVar4 = false;
    if (mCyl.ChkTgHit()) {
        cCcD_Obj* pcVar3 = mCyl.GetTgHitObj();
        if (pcVar3 != NULL) {
            if (pcVar3->ChkAtType(AT_TYPE_WIND)) {
                speedF = mCyl.GetTgRVecP()->absXZ() * HIO(m34);
                current.angle.y = cM_atan2s(mCyl.GetTgRVecP()->x, mCyl.GetTgRVecP()->z);
            } else {
                break_proc();
                uVar4 = true;
            }
        }
        mCyl.ClrTgHit();
    }
    return uVar4;
}

/* 0000032C-000004E4       .text co_hitCallback__14daObjMshokki_cFP10fopAc_ac_cP12dCcD_GObjInfP10fopAc_ac_cP12dCcD_GObjInf */
void daObjMshokki_c::co_hitCallback(fopAc_ac_c* a_this, dCcD_GObjInf*, fopAc_ac_c* other_actor, dCcD_GObjInf*) {
    daObjMshokki_c* i_this = (daObjMshokki_c*)a_this;
    f32 fVar1;
    s16 sVar3 = cM_atan2s(i_this->current.pos.x - other_actor->current.pos.x, i_this->current.pos.z - other_actor->current.pos.z);
    // §327 RETAIL (#else) side of the donor VERSION block kept.
    if (other_actor == dComIfGp_getPlayer(0) && abs(other_actor->current.angle.y - sVar3) < HIO(m24) && other_actor->speedF >= HIO(m1C) + HIO(m20) &&
        abs(i_this->m618 - i_this->m614) > 2)
    {
        fVar1 = (other_actor->speedF - (HIO(m1C) + HIO(m20))) / HIO(m28);
        if (fVar1 > 1.0f) {
            fVar1 = 1.0f;
        }
        s32 cos = fVar1 * 6144.0f;
        i_this->speedF = other_actor->speedF * cM_scos(cos);
        i_this->speed.y = HIO(m30) * fVar1 + HIO(m2C);
        i_this->current.angle.y = sVar3;
        i_this->shape_angle.y = sVar3;
        i_this->mCyl.SetCoHitCallback(NULL);
        i_this->m618 = i_this->m614;
    } else if (other_actor->speedF >= HIO(m1C)) {
        fVar1 = (other_actor->speedF - HIO(m1C)) / HIO(m20);
        if (fVar1 > 1.0f) {
            fVar1 = 1.0f;
        }
        s16 angle_x = l_data[i_this->m60C].m0C * fVar1;
        s16 angle_y = other_actor->current.angle.y;
        i_this->shape_angle.x = angle_x;
        i_this->shape_angle.y = angle_y;
        i_this->m61C = angle_y + (s16)(HIO(m18) * 2.0f);  // §327 retail side
    }
}

/* 000004E4-00000610       .text break_proc__14daObjMshokki_cFv */
void daObjMshokki_c::break_proc() {
    // §327 donor ids are dPa_name::-scoped; the port enum is global (unscoped) —
    // same rename toripost §253 applied. Values are the NATIVE port members.
    static u16 particle_id[] = {ID_AK_JN_BREAKMAJUPOT00, ID_AK_JN_BREAKMAJUPLATE00, ID_AK_JN_BREAKMAJUCUP00};

    // §327 donor 8-arg particle_set -> port 10-arg overload (+NULL env/pscale);
    // WW tevStr.mColorK0 == port tevStr.TevKColor (same K-color slot, TP name).
    dComIfGp_particle_set(particle_id[m60C], &current.pos, &shape_angle, NULL, 0xff, NULL, -1, &tevStr.TevKColor, NULL, NULL);
    // §327 *dComIfG_Bgsp() is reference-form in the port (toripost recipe).
    fopAcM_seStartCurrent(this, JA_SE_OBJ_COL_SWC_CHNS, dComIfG_Bgsp().GetMtrlSndId(mAcch.m_gnd));

    dKy_Sound_set(current.pos, 150, fopAcM_GetID(this), 5);
    fopAcM_delete(this);
}

/* 00000610-00000740       .text set_se__14daObjMshokki_cFv */
void daObjMshokki_c::set_se() {
    static s32 se_flag[] = {JA_SE_OBJ_CUP_SWING, JA_SE_OBJ_DISH_SWING, JA_SE_OBJ_CUP_SWING};

    s16 tmp = shape_angle.y - m61C;
    if (abs(tmp) < (s32)HIO(m18)) {  // §327 retail side
        s16 uVar4 = (shape_angle.x / l_data[m60C].m0C) * 0x4000;
        if (uVar4 > 0x4000) {
            uVar4 = 0x4000;
        }
        fopAcM_seStartCurrent(this, se_flag[m60C], cM_ssin(uVar4) * 100.0f);
    }
}

/* 00000740-00000A1C       .text _create__14daObjMshokki_cFv */
cPhs_Step daObjMshokki_c::_create() {  // §327 cPhs_State -> cPhs_Step
    fopAcM_ct(this, daObjMshokki_c);

    if (fpcM_IsFirstCreating(this)) {  // §327 port lacks the fopAcM_ wrapper; same fn
        m60C = param_get_arg();
        if (m60C < 0 || m60C >= 3) {
            // §327 donor JUT_ASSERT(648, FALSE) — retail no-op; log + donor fallback.
            DuskLog.warn("[Mshokki] §327 _create: bad shape arg {} -> 0", m60C);
            m60C = 0;
        }
    }

    cPhs_Step ret = dComIfG_resLoad(&mPhase, l_arcname);
    if (ret == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(this, solidHeapCB, 0x4C0)) {
            fopAcM_SetMtx(this, mModel->getBaseTRMtx());
            set_mtx();
            mAcchCir.SetWall(50.0f, 35.0f);
            mAcch.Set(
                fopAcM_GetPosition_p(this),
                fopAcM_GetOldPosition_p(this),
                this,
                1,
                &mAcchCir,
                fopAcM_GetSpeed_p(this),
                fopAcM_GetAngle_p(this),
                fopAcM_GetShapeAngle_p(this)
            );
            mAcch.ClrWaterNone();
            mAcch.ClrRoofNone();
            mAcch.SetRoofCrrHeight(50.0f);
            mAcch.CrrPos(dComIfG_Bgsp());  // §327 reference form (toripost recipe)
            mAcch.ClrGroundLanding();
            mStts.Init(100, 0xff, this);
            mCyl.Set(l_cyl_src);
            mCyl.SetStts(&mStts);
            // §327 dCcD_Cyl::StartCTg was dropped from the port's d_cc_d — inline
            // the VERBATIM WW body (d_cc_d.cpp:295: zero Tg vec + SetC(pos)).
            {
                cXyz vel(0.0f, 0.0f, 0.0f);
                mCyl.SetTgVec(vel);
                mCyl.SetC(current.pos);
            }
            mCyl.SetR(l_data[m60C].radius);
            mCyl.SetH(l_data[m60C].height);
            mCyl.SetCoHitCallback(co_hitCallback);
            fopAcM_SetGravity(this, -6.0f);
        } else {
            ret = cPhs_ERROR_e;
        }
    }

    // §327 VERSION==VERSION_DEMO mDoHIO_createChild block dropped (retail side).
    return ret;
}

/* 00000C44-00000C74       .text _delete__14daObjMshokki_cFv */
bool daObjMshokki_c::_delete() {
    dComIfG_resDelete(&mPhase, l_arcname);
    // §327 VERSION==VERSION_DEMO mDoHIO_deleteChild block dropped (retail side).
    return true;
}

/* 00000C74-00000E24       .text _execute__14daObjMshokki_cFv */
bool daObjMshokki_c::_execute() {
    fopAcM_posMoveF(this, mStts.GetCCMoveP());
    mAcch.CrrPos(dComIfG_Bgsp());  // §327 reference form

    speedF *= HIO(m38);
    if (speedF < HIO(m3C)) {
        speedF = 0.0f;
    }

    if (mAcch.ChkGroundHit()) {
        if (abs(shape_angle.x) > (s32)HIO(m10)) {
            shape_angle.x += -(s16)HIO(m10);
            shape_angle.y += (s16)HIO(m18);
            set_se();
        } else {
            shape_angle.x = 0;
        }
    } else {
        shape_angle.x += -(s32)HIO(m14);
    }

    // §327 VERSION==VERSION_DEMO live-HIO SetR/SetH re-tune dropped (retail side).

    if (m612 == 1 && mAcch.ChkGroundLanding()) {
        break_proc();
    } else {
        if (mAcch.ChkGroundHit() && !mAcch.ChkGroundLanding()) {
            m612 = 1;
        }

        mStts.Move();
        if (!checkCollision()) {
            set_mtx();
            mCyl.MoveCTg(current.pos);
            // §327 DN-1 audit: this stamps the actor's OWN resolved room onto its
            // CC-stats (donor-verbatim; the same idiom the port's native actors
            // use, e.g. d_a_npc_cdn3). It is NOT a daBg/BgW standable-geometry
            // registration — outside DN-1's guarded surface.
            mStts.SetRoomId(current.roomNo);
            dComIfG_Ccsp()->Set(&mCyl);
            attention_info.position.x = current.pos.x;
            attention_info.position.y = current.pos.y + l_data[m60C].m00;
            attention_info.position.z = current.pos.z;
            eyePos = attention_info.position;
        }
    }
    m614++;
    return true;
}

/* 00000E24-00000E84       .text _draw__14daObjMshokki_cFv */
bool daObjMshokki_c::_draw() {
    // §406: WW feeder (donor type) — see d_kankyo_ww.h; TP never writes TevColor/TevKColor.
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mModel, &tevStr);
    mDoExt_modelUpdateDL(mModel);
    return true;
}

/* 00000E84-00000EA4       .text daObjMshokki_Create__FP10fopAc_ac_c */
static cPhs_Step daObjMshokki_Create(fopAc_ac_c* i_this) {  // §327 cPhs_State -> cPhs_Step
    return ((daObjMshokki_c*)i_this)->_create();
}

/* 00000EA4-00000EC8       .text daObjMshokki_Delete__FP14daObjMshokki_c */
static BOOL daObjMshokki_Delete(daObjMshokki_c* i_this) {
    return ((daObjMshokki_c*)i_this)->_delete();
}

/* 00000EC8-00000EEC       .text daObjMshokki_Execute__FP14daObjMshokki_c */
static BOOL daObjMshokki_Execute(daObjMshokki_c* i_this) {
    return ((daObjMshokki_c*)i_this)->_execute();
}

/* 00000EEC-00000F10       .text daObjMshokki_Draw__FP14daObjMshokki_c */
static BOOL daObjMshokki_Draw(daObjMshokki_c* i_this) {
    return ((daObjMshokki_c*)i_this)->_draw();
}

/* 00000F10-00000F18       .text daObjMshokki_IsDelete__FP14daObjMshokki_c */
static BOOL daObjMshokki_IsDelete(daObjMshokki_c*) {
    return TRUE;
}

static actor_method_class l_daObjMshokki_Method = {
    (process_method_func)daObjMshokki_Create,
    (process_method_func)daObjMshokki_Delete,
    (process_method_func)daObjMshokki_Execute,
    (process_method_func)daObjMshokki_IsDelete,
    (process_method_func)daObjMshokki_Draw,
};

actor_process_profile_definition g_profile_Obj_Mshokki = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0008,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_Obj_Mshokki_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daObjMshokki_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_Obj_Mshokki_e,  // §327 donor slot absent -> port E_RD slot
    /* Actor SubMtd */ &l_daObjMshokki_Method,
    /* Status       */ fopAcStts_CULL_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
