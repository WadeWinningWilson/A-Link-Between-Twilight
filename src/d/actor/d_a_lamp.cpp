/**
 * d_a_lamp.cpp
 * Object - Generic wall lamp
 *
 * ============================================================================
 * §327 WW WALL/LOFT LAMP (LAMP) DIRECT PORT — body VERBATIM from the WW donor
 * (D:/XXXXXXX/WW DP/src/d/actor/d_a_lamp.cpp). Direct-port crash recipes applied
 * at the call sites only (banners inline), mirroring toripost §253 / ba1 §261:
 *   #1 DN-3  : model via dExtNpcMount_acquireModelData("Lamp", "lamp_00.bmd")
 *              (donor dRes_INDEX_LAMP_BMD_LAMP_00_e == res id 3 == lamp_00.bmd;
 *              verified TWICE: staged Lamp.residmap.csv AND a RARC parse of the
 *              donor D:/XXXXXXX/Ex WW/files/res/Object/Lamp.arc agree)
 *   particle_set 6-arg -> 10-arg port arity ; setSimple 2-arg -> 7-arg (WW
 *   default args made explicit — the port inline has no defaults)
 *   LIGHT_INFLUENCE member renames: WW mPos/mColor/mPower/mFluctuation ->
 *   port mPosition/mColor/mPow/mFluctuation (same layout, TP names)
 *   MtxRotX/MtxRotZ absent from the port's c_lib -> local statics VERBATIM
 *   from the WW donor SSystem/SComponent/c_lib.cpp (MTXRotRad + MTXConcat)
 *   cPhs_State -> cPhs_Step ; JUT_ASSERT -> NULL-guard (port assert panics;
 *   the donor RETAIL assert compiles out — guard is the faithful behavior)
 * ============================================================================
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep
#include "d/actor/d_a_lamp.h"
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_mng.h"          // §327 fopAcM_* helpers (toripost include set)
#include "m_Do/m_Do_mtx.h"
#include "m_Do/m_Do_ext.h"                // §327 mDoExt_J3DModel__create / modelUpdateDL
#include "SSystem/SComponent/c_lib.h"     // §327 MtxTrans/MtxScale/MtxPosition/calc_mtx/cLib_addCalc2
#include "SSystem/SComponent/c_math.h"    // §327 cM_ssin/cM_scos/cM_rndF/cM_rndFX
#include "d/d_particle_name.h"            // §327 ID_AK_JN_TORCH / ID_AK_JP_O_KAGEROU00 (native port enum, unscoped)
#include "d/d_vibration.h"                // §327 dVibration_c::CheckQuake
#include "d/d_ext_npc_mount.h"            // §327 DN-3 parse-at-consume model resolver (acquireModelData)
#include "d/d_ext_ww_actor_shims.h"       // §327 AT_TYPE_WIND/UNK400000, cCcD_TgSPrm_Set_e, fopAcStts_*

// §327 ========================================================================
// WW-absent VALUE-FAITHFUL constants (donor numbering; toripost §253 pattern —
// kept LOCAL to this TU, not the shared shims header).
// ----------------------------------------------------------------------------
// WW d/d_cc_d.h AT types the shims header does not already carry.
#define AT_TYPE_FIRE          (1 << 9)    // 0x200
#define AT_TYPE_FIRE_ARROW    (1 << 18)   // 0x40000
// WW SSystem/SComponent/c_cc_d.h Tg SPrm bit the shims header lacks.
#define cCcD_TgSPrm_IsOther_e 0x08
// Env-light TEV struct type (WW d_kankyo.h: TEV_TYPE_BG0 == 1); port lacks the
// member — same local macro toripost §253 uses.
#define TEV_TYPE_BG0 1
// Donor draw-prio slot absent -> reuse a port ground/ambient slot (pig/seagull/
// toripost precedent).
#define fpcDwPi_LAMP_e fpcDwPi_E_RD_e
// ----------------------------------------------------------------------------
// §327 MtxRotX / MtxRotZ survive in the port only as MtxTrans/MtxScale/
// MtxPosition (c_lib.h); the rot pair was dropped. Local statics VERBATIM from
// the WW donor SSystem/SComponent/c_lib.cpp (MtxRotX__FfUc / MtxRotZ__FfUc).
static void MtxRotX(f32 rot, u8 concat) {
    if (concat == 0) {
        MTXRotRad(*calc_mtx, 'X', rot);
    } else {
        Mtx mtx;
        MTXRotRad(mtx, 'X', rot);
        MTXConcat(*calc_mtx, mtx, *calc_mtx);
    }
}
static void MtxRotZ(f32 rot, u8 concat) {
    if (concat == 0) {
        MTXRotRad(*calc_mtx, 'Z', rot);
    } else {
        Mtx mtx;
        MTXRotRad(mtx, 'Z', rot);
        MTXConcat(*calc_mtx, mtx, *calc_mtx);
    }
}
// §327 ========================================================================

/* 000000EC-00000158       .text daLamp_Draw__FP10lamp_class */
static BOOL daLamp_Draw(lamp_class* i_this) {
    g_env_light.settingTevStruct(TEV_TYPE_BG0, &i_this->current.pos, &i_this->tevStr);
    J3DModel* pModel = i_this->mModel;
    g_env_light.setLightTevColorType(pModel, &i_this->tevStr);
    mDoExt_modelUpdateDL(pModel);
    return TRUE;
}

/* 00000158-000005C8       .text daLamp_Execute__FP10lamp_class */
static BOOL daLamp_Execute(lamp_class* i_this) {
    i_this->mCycleCtr += 1;
    MtxTrans(i_this->current.pos.x, i_this->current.pos.y, i_this->current.pos.z, 0);
    mDoMtx_YrotM(*calc_mtx, i_this->current.angle.y);
    if (i_this->mParameters == 0) {
        i_this->mLength = 0.1f;
    } else {
        if (dComIfGp_getVibration().CheckQuake() || fopAcM_otoCheck(i_this, 1000.0f) > 400) {
            i_this->mOto = 0x14;
        }
        if (i_this->mOto) {
            i_this->mOto--;
            cLib_addCalc2(&i_this->mLength, 0.15f, 1.0f, 0.015f);
        } else {
            cLib_addCalc2(&i_this->mLength, 0.01f, 1.0f, 0.0005f);
        }
    }
    f32 oppDist = cM_ssin(i_this->mCycleCtr * 0x320) * i_this->mLength;
    MtxRotX(oppDist, 1);
    f32 adjDist = cM_scos(i_this->mCycleCtr * 0x2bc) * i_this->mLength;
    MtxRotZ(adjDist, 1);
    MtxScale(0.4f, 0.4f, 0.4f, 1);
    i_this->mModel->setBaseTRMtx(*calc_mtx);
    MtxTrans(10.0f, -140.0f, -15.0f, 1);

    cXyz offset;
    offset.z = 0.0f;
    offset.y = 0.0f;
    offset.x = 0.0f;
    MtxPosition(&offset, &i_this->mPos);

    if (!i_this->mParticleInit) {
        static cXyz fire_scale(0.5f, 0.5f, 0.5f);

        // §327 donor 6-arg particle_set -> port 10-arg overload (trailing WW
        // defaults -1/NULL/NULL/NULL made explicit; toripost §253 recipe).
        dComIfGp_particle_set((u16)ID_AK_JN_TORCH, &i_this->mPos, NULL, &fire_scale, 0xFF,
                              &i_this->mPa, -1, NULL, NULL, NULL);
        i_this->mParticleInit = 1;
        i_this->mParticlePower = 1.0f;
    }

    if (i_this->mPa.getEmitter()) {
        cXyz whitePartPos = i_this->mPos;
        whitePartPos.y += 20.0f;
        // §327 donor 2-arg setSimple -> port 7-arg (WW defaults 0xFF/white/white/0
        // + the port's trailing rate 0.0f — the digplace/vegetation idiom).
        dComIfGp_particle_setSimple((u16)ID_AK_JP_O_KAGEROU00, &whitePartPos, 0xFF, g_whiteColor,
                                    g_whiteColor, 0, 0.0f);
        cLib_addCalc2(&i_this->mParticlePower, cM_rndF(0.2f) + 1.0f, 0.5f, 0.02f);
    } else {
        i_this->mParticlePower = 0.0f;
    }
    // §327 LIGHT_INFLUENCE member names: WW mPos/mColor/mPower -> port
    // mPosition/mColor/mPow (identical layout & semantics — TP kept WW's struct).
    i_this->mInf.mPosition = i_this->mPos;
    i_this->mInf.mColor.r = 600;
    i_this->mInf.mColor.g = 400;
    i_this->mInf.mColor.b = 120;

    s16 power = i_this->mParticlePower * 150.0f;
    i_this->mInf.mPow = power;
    i_this->mInf.mFluctuation = 250.0f;

    i_this->mSph.SetC(i_this->mPos);
    dComIfG_Ccsp()->Set(&i_this->mSph);
    if (!i_this->mHitTimeoutLeft) {
        if (i_this->mSph.ChkTgHit()) {
            cCcD_Obj* pHitObj = i_this->mSph.GetTgHitObj();
            if (pHitObj && pHitObj->ChkAtType(AT_TYPE_WIND | AT_TYPE_UNK400000)) {
                i_this->mHitAngle = dComIfGp_getPlayer(0)->shape_angle.y;
                i_this->mHitTimeoutLeft = 0x28;
            }
        }
    } else {
        i_this->mHitTimeoutLeft--;
        if (i_this->mPa.getEmitter()) {
            float tgtZ;
            if (i_this->mHitTimeoutLeft > 10) {
                tgtZ = 4.0f;
            } else {
                tgtZ = 0.0f;
            }
            cLib_addCalc2(&i_this->mHitReactCurZ, tgtZ, 1.0f, 0.5f);
            cMtx_YrotS(*calc_mtx, i_this->mHitAngle);
            cXyz offset;
            offset.set(0.0f, 1.0f, i_this->mHitReactCurZ);
            cXyz rotOffset;
            MtxPosition(&offset, &rotOffset);
            JGeometry::TVec3<f32> dir(rotOffset);
            i_this->mPa.getEmitter()->setDirection(dir);
        }
    }

    return TRUE;
}

/* 00000604-00000634       .text daLamp_IsDelete__FP10lamp_class */
static BOOL daLamp_IsDelete(lamp_class* i_this) {
    i_this->mPa.remove();
    return TRUE;
}

/* 00000634-00000678       .text daLamp_Delete__FP10lamp_class */
static BOOL daLamp_Delete(lamp_class* i_this) {
    dComIfG_resDelete(&i_this->mPhs, "Lamp");
    dKy_plight_cut(&i_this->mInf);
    return TRUE;
}

/* 00000678-0000073C       .text useHeapInit__FP10lamp_class */
static BOOL useHeapInit(lamp_class* i_this) {
    // §327 DN-3 (recipe #1): getObjectRes returns RAW bytes on this port — route
    // the MODEL through acquireModelData (parse-at-consume, finished J3DModelData).
    // Donor: dComIfG_getObjectRes("Lamp", dRes_INDEX_LAMP_BMD_LAMP_00_e /*=3*/);
    // res id 3 == "lamp_00.bmd" (Lamp.residmap.csv + donor-arc RARC parse agree).
    // Donor JUT_ASSERT(0x170, modelData != NULL) -> NULL-guard FALSE (the port
    // macro panics; the donor RETAIL assert is compiled out).
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Lamp", "lamp_00.bmd");
    if (modelData == NULL) {
        return FALSE;
    }

    i_this->mModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (i_this->mModel == 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/* 0000073C-0000075C       .text daLamp_solidHeapCB__FP10fopAc_ac_c */
static BOOL daLamp_solidHeapCB(fopAc_ac_c* i_ac) {
    return useHeapInit((lamp_class*)i_ac);
}

/* 0000075C-00000914       .text daLamp_Create__FP10fopAc_ac_c */
static cPhs_Step daLamp_Create(fopAc_ac_c* i_ac) {  // §327 cPhs_State -> cPhs_Step
    fopAcM_ct(i_ac, lamp_class);
    lamp_class* i_this = (lamp_class*)i_ac;

    cPhs_Step phase_state = dComIfG_resLoad(&i_this->mPhs, "Lamp");
    if (phase_state == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(i_this, &daLamp_solidHeapCB, 0x6040)) {
            i_this->mParameters = fopAcM_GetParam(i_this);
            if (i_this->mParameters == 0xFF) {
                i_this->mParameters = 0;
            }
            i_this->mStts.Init(0xff, 0xff, i_this);

            static dCcD_SrcSph sph_src = {
                // dCcD_SrcGObjInf
                {
                    /* Flags             */ 0,
                    /* SrcObjAt  Type    */ 0,
                    /* SrcObjAt  Atp     */ 0,
                    /* SrcObjAt  SPrm    */ 0,
                    /* SrcObjTg  Type    */ AT_TYPE_FIRE | AT_TYPE_UNK20000 | AT_TYPE_FIRE_ARROW | AT_TYPE_WIND | AT_TYPE_UNK400000,
                    /* SrcObjTg  SPrm    */ cCcD_TgSPrm_Set_e | cCcD_TgSPrm_IsOther_e,
                    /* SrcObjCo  SPrm    */ 0,
                    /* SrcGObjAt Se      */ 0,
                    /* SrcGObjAt HitMark */ 0,
                    /* SrcGObjAt Spl     */ 0,
                    /* SrcGObjAt Mtrl    */ 0,
                    /* SrcGObjAt SPrm    */ 0,
                    /* SrcGObjTg Se      */ 0,
                    /* SrcGObjTg HitMark */ 0,
                    /* SrcGObjTg Spl     */ 0,
                    /* SrcGObjTg Mtrl    */ 0,
                    /* SrcGObjTg SPrm    */ 0,
                    /* SrcGObjCo SPrm    */ 0,
                },
                // cM3dGSphS
                {{
                    /* Center */ {0.0f, 0.0f, 0.0f},
                    /* Radius */ 30.0f,
                }},
            };

            i_this->mSph.Set(sph_src);
            i_this->mSph.SetStts(&i_this->mStts);

            i_this->mCycleCtr = cM_rndFX(0x8000);

            for (int i = 0; i < 2; i++) {
                daLamp_Execute(i_this);
            }

            dKy_plight_set(&i_this->mInf);
        } else {
            phase_state = cPhs_ERROR_e;
        }
    }

    return phase_state;
}

static actor_method_class l_daLamp_Method = {
    (process_method_func)daLamp_Create,
    (process_method_func)daLamp_Delete,
    (process_method_func)daLamp_Execute,
    (process_method_func)daLamp_IsDelete,
    (process_method_func)daLamp_Draw,
};

actor_process_profile_definition g_profile_LAMP = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_LAMP_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(lamp_class),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_LAMP_e,  // §327 donor slot absent -> port E_RD slot (toripost precedent)
    /* Actor SubMtd */ &l_daLamp_Method,
    /* Status       */ fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
