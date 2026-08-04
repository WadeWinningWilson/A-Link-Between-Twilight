/**
 * d_a_esa.cpp — All-Purpose Bait (餌). §224 Pass 2: direct source port.
 * Donor logic verbatim; adaptations marked §224:
 *   - model loads from the mod-folder Esa.arc (was Link.arc/dRes_INDEX_LINK_BDL_ESA_e)
 *   - sea/water via shims (d_ext_ww_actor_shims) — bait always ground-lands
 *   - ripple particle (dComIfGp_particle_setShipTail) no-op'd (cosmetic; water path)
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/actor/d_a_esa.h"
#include "d/actor/d_a_player.h"
#include "d/d_bg_s_gnd_chk.h"
#include "d/d_bg_s_lin_chk.h"
#include "d/d_s_play.h"
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_ext_ww_actor_shims.h"
#include "d/d_ext_npc_mount.h"  // §224 dExtNpcMount_acquireModelData (mod arc)
#include "d/d_kankyo_ww.h"           // §404 WW lighting write-path (was the empty stub)

#define ESA_NO_WATER (-1.0e30f)  // §224 matches dBgS_GetWaterHeight shim (was -G_CM3D_F_INF)

static BOOL daEsa_Draw(esa_class* i_this) {
    dKyWw_setLightTevColorType(i_this->mpModel, &i_this->tevStr);
    mDoExt_modelUpdateDL(i_this->mpModel);
    return true;
}

void bg_check(esa_class* i_this) {
    dBgS_GndChk gndChk;
    Vec temp;
    temp.x = i_this->current.pos.x;
    temp.y = i_this->current.pos.y;
    temp.z = i_this->current.pos.z;
    temp.y += 100.0f;
    gndChk.SetPos(&temp);

    i_this->mGroundHeight = dComIfG_Bgsp().GroundCross(&gndChk) + 5.0f;
    s8 state = 1;
    if (daSea_ChkArea(i_this->current.pos.x, i_this->current.pos.z)) {
        f32 wave = daSea_calcWave(i_this->current.pos.x, i_this->current.pos.z);
        if (i_this->mGroundHeight <= wave) {
            i_this->mGroundHeight = wave;
            state = 2;
        }
    }

    cXyz sp6C;
    cXyz sp60;

    cXyz sp54 = i_this->current.pos;
    sp54.y += 100.0f;
    f32 waterHeight = dBgS_GetWaterHeight(sp54);
    if (waterHeight != ESA_NO_WATER && i_this->mGroundHeight <= waterHeight) {
        i_this->mGroundHeight = waterHeight;
        state = 2;
    }

    if (i_this->speed.y <= 0.0f && i_this->current.pos.y <= i_this->mGroundHeight) {
        i_this->current.pos.y = i_this->mGroundHeight;
        i_this->mState = state;
    } else {
        i_this->mState = 0;
    }

    dBgS_LinChk linChk;
    cXyz sp48 = i_this->old.pos + (i_this->current.pos - i_this->old.pos) * 1.5f;
    sp48.y = i_this->old.pos.y;
    cXyz temp5 = i_this->old.pos - sp48;
    if (temp5.abs() > 1.0f) {
        linChk.Set(&i_this->old.pos, &sp48, i_this);
        if (dComIfG_Bgsp().LineCross(&linChk)) {
            i_this->current.pos.x = i_this->old.pos.x;
            i_this->current.pos.z = i_this->old.pos.z;
            i_this->speedF *= 0.5f;
            i_this->current.angle.y -= 0x8000;
            sp6C.x = 0.0f;
            sp6C.y = 0.0f;
            sp6C.z = i_this->speedF;
            cMtx_YrotS(*calc_mtx, i_this->current.angle.y);
            MtxPosition(&sp6C, &sp60);
            i_this->speed.x = sp60.x;
            i_this->speed.z = sp60.z;
        }
    } else {
        fopAcM_OffStatus(i_this, fopAcStts_UNK_0x4000_e);
    }
}

void esa_1_move(esa_class* i_this) {
    cXyz sp24;
    switch (i_this->mActionState) {
    case 0:
        i_this->current.angle.y += (s16)cM_rndFX(4000.0f);
        i_this->current.angle.z = cM_rndFX(0x8000);
        sp24.x = 0.0f;
        sp24.y = cM_rndF(8.0f) + 15.0f;
        sp24.z = cM_rndF(5.0f) + 10.0f;
        i_this->speedF = sp24.z;
        cMtx_YrotS(*calc_mtx, i_this->current.angle.y);
        MtxPosition(&sp24, &i_this->speed);
        i_this->mActionState = 1;
        // fallthrough
    case 1:
        if (i_this->mState != 0) {
            if (i_this->mState == 2) {
                i_this->mTimer[0] = cM_rndF(50.0f) + 200.0f;
                i_this->mActionState = 3;
                // §224: ripple particle no-op'd (water path, cosmetic)
            } else {
                if (i_this->speed.y < 5.0f) {
                    i_this->speed.y *= -(cM_rndF(0.05f) + 0.15f);
                    i_this->current.angle.z = cM_rndFX(0x8000);
                }
                i_this->current.angle.y += (s16)cM_rndFX(8000.0f);
                i_this->speedF *= cM_rndF(0.3f) + 0.3f;
                sp24.x = 0.0f;
                sp24.y = 0.0f;
                sp24.z = i_this->speedF;
                cMtx_YrotS(*calc_mtx, i_this->current.angle.y);
                cXyz sp18;
                MtxPosition(&sp24, &sp18);
                i_this->speed.x = sp18.x;
                i_this->speed.z = sp18.z;
                if (i_this->speedF < 0.1f) {
                    i_this->mActionState = 2;
                    i_this->mTimer[0] = cM_rndF(50.0f) + 200.0f;
                    break;
                }
            }
        }
        i_this->current.pos.x += i_this->speed.x;
        i_this->current.pos.y += i_this->speed.y;
        i_this->current.pos.z += i_this->speed.z;
        i_this->speed.y -= 3.0f;
        bg_check(i_this);
        break;
    case 2:
        if (i_this->mTimer[0] == 0) {
            fopAcM_delete(i_this);
        }
        break;
    case 3:
        if (dComIfGp_evmng_startCheck("SO_ESA_XY")) {
            i_this->mTimer[0] = 10000;
        }
        // §224: ripple remove + wave-float no-op'd (water path)
        if (i_this->mTimer[0] == 0) {
            i_this->mTimer[0] = 10;
            i_this->mActionState = 4;
        }
        if (i_this->field_0x298) {  // claimed by a pig → consume
            i_this->mTimer[0] = 10;
            i_this->mActionState = 4;
        }
        break;
    case 4:
        if (i_this->mTimer[0] == 0) {
            fopAcM_delete(i_this);
        }
        break;
    }
}

static BOOL daEsa_Execute(esa_class* i_this) {
    for (int i = 0; i < 2; i++) {
        if (i_this->mTimer[i] != 0) {
            i_this->mTimer[i]--;
        }
    }
    esa_1_move(i_this);

    MtxTrans(i_this->current.pos.x, i_this->current.pos.y, i_this->current.pos.z, false);
    cMtx_YrotM(*calc_mtx, i_this->current.angle.y);
    cMtx_XrotM(*calc_mtx, i_this->current.angle.x);
    cMtx_ZrotM(*calc_mtx, i_this->current.angle.z);
    i_this->mpModel->setBaseTRMtx(*calc_mtx);

    // §406 deviation corrected: 0x40 was a TP type; donor d_a_esa.cpp:208 authors TEV_TYPE_ACTOR.
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &i_this->current.pos, &i_this->tevStr);
    return true;
}

static BOOL daEsa_IsDelete(esa_class* /*i_this*/) {
    return true;
}

static BOOL daEsa_Delete(esa_class* i_this) {
    return true;
}

static BOOL daEsa_CreateHeap(fopAc_ac_c* i_actor) {
    esa_class* i_this = static_cast<esa_class*>(i_actor);
    // §224: model from the mod-folder Esa.arc (arc resLoaded in Create).
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Esa", "esa.bdl");
    if (modelData == NULL) {
        return false;
    }
    i_this->mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);
    if (i_this->mpModel == NULL) {
        return false;
    }
    return true;
}

static int daEsa_Create(fopAc_ac_c* i_actor) {
    fopAcM_ct(i_actor, esa_class);
    esa_class* i_this = static_cast<esa_class*>(i_actor);

    i_this->field_0x2B9 = fopAcM_GetParam(i_this) & 0xFF;
    i_this->field_0x2BA = fopAcM_GetParam(i_this) >> 8 & 0xFF;

    // §224: load the mod arc first (donor read from the always-resident Link.arc).
    const int phase = dComIfG_resLoad(&i_this->mPhase, "Esa");
    if (phase != cPhs_COMPLEATE_e) {
        return phase;
    }
    if (!fopAcM_entrySolidHeap(i_this, daEsa_CreateHeap, 0x4C0)) {
        return cPhs_ERROR_e;
    }

    f32 scaleF = 0.65f + cM_rndF(0.35f);
    cXyz scale(scaleF, scaleF, scaleF);
    i_this->mpModel->setBaseScale(scale);
    fopAcM_SetMtx(i_this, i_this->mpModel->getBaseTRMtx());
    return cPhs_COMPLEATE_e;
}

static actor_method_class l_daEsa_Method = {
    (process_method_func)daEsa_Create,   (process_method_func)daEsa_Delete,
    (process_method_func)daEsa_Execute,  (process_method_func)daEsa_IsDelete,
    (process_method_func)daEsa_Draw,
};

actor_process_profile_definition g_profile_ESA = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_ESA_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(esa_class),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daEsa_Method,
    /* Status       */ fopAcStts_CULL_e | fopAcStts_UNK_0x4000_e | fopAcStts_UNK_0x40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
