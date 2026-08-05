/**
 * d_a_vrbox.cpp
 *
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_vrbox.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_kankyo_ww.h"  // §418 WW native skydome leg
#include "dusk/logging.h"

// §418b: the mount's parse-once model-data cache (global bridge, §334 region).
J3DModelData* dExtWwMount_acquireModelData(const char* arc, const char* modelName);

static int daVrbox_color_set(vrbox_class* i_this);
#if TARGET_PC
static int daVrbox_ww_color_set(vrbox_class* i_this);  // §418
#endif

static int daVrbox_Draw(vrbox_class* i_this) {
    J3DModel* soraModel_p = i_this->mpSoraModel;
    f32 fvar = 0.0f;
    dStage_FileList_dt_c* filelist_p = NULL;

#if TARGET_PC
    // §418 WW LEG: same donor draw shape (colour feed, camera-follow with
    // 0.09 sea-level parallax, sky list) on the WW model.
    if (dKyWw_isSkyHost() && i_this->mpWwSky != NULL) {
        daVrbox_ww_color_set(i_this);
        if (g_env_light.hide_vrbox) {
            return 1;
        }
        f32 y0 = 0.0f;
        if (dComIfGd_getView() != NULL) {
            y0 = dComIfGd_getInvViewMtx()[1][3] * 0.09f;  // host sea level = 0
        }
        mDoMtx_stack_c::transS(dComIfGd_getInvViewMtx()[0][3],
                               dComIfGd_getInvViewMtx()[1][3] - y0,
                               dComIfGd_getInvViewMtx()[2][3]);
        i_this->mpWwSky->setBaseTRMtx(mDoMtx_stack_c::get());
        dKy_GxFog_set();
        dComIfGd_setListSky();
        mDoExt_modelUpdateDL(i_this->mpWwSky);
        dComIfGd_setList();
        return 1;
    }
#endif

    daVrbox_color_set(i_this);

    if (g_env_light.hide_vrbox) {
        return 1;
    }

    if (dComIfGp_roomControl_getStayNo() >= 0) {
        s32 stayNo = dComIfGp_roomControl_getStayNo();
        filelist_p = dComIfGp_roomControl_getStatusRoomDt(stayNo)->getFileListInfo();
    }

    if (filelist_p != NULL) {
        fvar = dStage_FileList_dt_SeaLevel(filelist_p);
    }

    if (dComIfGd_getView() != NULL) {
        fvar = (dComIfGd_getInvViewMtx()[1][3] - fvar) * 0.09f;
    } else {
        fvar = 0.0f;
    }

    mDoMtx_stack_c::transS(dComIfGd_getInvViewMtx()[0][3], dComIfGd_getInvViewMtx()[1][3] - fvar,
                           dComIfGd_getInvViewMtx()[2][3]);

    soraModel_p->setBaseTRMtx(mDoMtx_stack_c::get());
    dKy_GxFog_set();

#if !TARGET_PC
    // This code is broken but happens to not do anything on the real game.

    // these casts look like fake matches, but this ptr is used as both J3DModel and J3DModelData?
    for (int i = ((J3DModelData*)soraModel_p)->getMaterialNum() - 1; i >= 0; i--) {
        J3DMaterial* material_p = ((J3DModelData*)soraModel_p)->getMaterialNodePointer(i);

        J3DFogInfo* fogInfo_p;
        if (material_p != NULL) {
            fogInfo_p = material_p->getFog()->getFogInfo();
        }

        fogInfo_p->mType = 2;
    }
#endif

    dComIfGd_setListSky();
    mDoExt_modelUpdateDL(soraModel_p);
    dComIfGd_setList();

    return 1;
}

#if TARGET_PC
// ============================================================================
// §418 WW color feed -- donor daVrbox_color_set (WW d_a_vrbox.cpp:58-92)
// verbatim wiring on the WW model: vr_sky material 0 takes the KASUMI-MAE
// colour into TevKCOLOR0, material 1 the SKY colour (donor uses K registers;
// the receiver's TP sora model evolved to TevColor -- register truth follows
// the model lineage). Receiver palette equivalents per the №113/№144 feed:
// mVrKasumiMaeColor->vrbox_kasumi_inner_col, mVrSkyColor->vrbox_sky_col,
// mVrkumoColor->vrbox_kumo_top_col.
// ============================================================================
static int daVrbox_ww_color_set(vrbox_class* i_this) {
    if ((g_env_light.vrbox_kasumi_inner_col.r + g_env_light.vrbox_kasumi_inner_col.g +
         g_env_light.vrbox_kasumi_inner_col.b + g_env_light.vrbox_sky_col.r +
         g_env_light.vrbox_sky_col.g + g_env_light.vrbox_sky_col.b +
         g_env_light.vrbox_kumo_top_col.r + g_env_light.vrbox_kumo_top_col.g +
         g_env_light.vrbox_kumo_top_col.b) == 0)
    {
        g_env_light.hide_vrbox = true;  // donor: all-black palette = NO sky
        return 1;
    }
    g_env_light.hide_vrbox = false;

    J3DModelData* modelData = i_this->mpWwSky->getModelData();
    J3DGXColor kcolor;

    J3DMaterial* mat = modelData->getMaterialNodePointer(0);
    if (mat != NULL) {
        mat->setCullMode(0);
        mat->change();
        kcolor.r = (u8)g_env_light.vrbox_kasumi_inner_col.r;
        kcolor.g = (u8)g_env_light.vrbox_kasumi_inner_col.g;
        kcolor.b = (u8)g_env_light.vrbox_kasumi_inner_col.b;
        kcolor.a = 255;
        mat->setTevKColor(0, &kcolor);
    }

    mat = modelData->getMaterialNodePointer(1);
    if (mat != NULL) {
        mat->setCullMode(0);
        mat->change();
        kcolor.r = (u8)g_env_light.vrbox_sky_col.r;
        kcolor.g = (u8)g_env_light.vrbox_sky_col.g;
        kcolor.b = (u8)g_env_light.vrbox_sky_col.b;
        kcolor.a = 255;
        mat->setTevKColor(0, &kcolor);
    }
    return 1;
}
#endif

static int daVrbox_color_set(vrbox_class* i_this) {
    if ((g_env_light.vrbox_kasumi_outer_col.r + g_env_light.vrbox_kasumi_outer_col.g +
         g_env_light.vrbox_kasumi_outer_col.b + g_env_light.vrbox_sky_col.r + g_env_light.vrbox_sky_col.g +
         g_env_light.vrbox_sky_col.b + g_env_light.vrbox_kumo_top_col.r + g_env_light.vrbox_kumo_top_col.g +
         g_env_light.vrbox_kumo_top_col.b) == 0)
    {
        g_env_light.hide_vrbox = true;
        return 1;
    }

    g_env_light.hide_vrbox = false;

    J3DModelData* modelData = i_this->mpSoraModel->getModelData();
    J3DGXColorS10 color;

    J3DMaterial* material_0 = modelData->getMaterialNodePointer(0);
    if (material_0 != NULL) {
        material_0->setCullMode(0);
        material_0->change();

        color.r = g_env_light.vrbox_sky_col.r;
        color.g = g_env_light.vrbox_sky_col.g;
        color.b = g_env_light.vrbox_sky_col.b;
        color.a = 255;
        material_0->setTevColor(0, &color);
    }

    J3DMaterial* material_1 = modelData->getMaterialNodePointer(1);
    if (material_1 != NULL) {
        material_1->setCullMode(0);
        material_1->change();

        color.r = g_env_light.vrbox_kasumi_inner_col.r;
        color.g = g_env_light.vrbox_kasumi_inner_col.g;
        color.b = g_env_light.vrbox_kasumi_inner_col.b;
        color.a = g_env_light.vrbox_kasumi_inner_col.a;
        material_1->setTevColor(0, &color);
    }

    return 1;
}

static int daVrbox_Execute(vrbox_class* i_this) {
    return 1;
}

static int daVrbox_IsDelete(vrbox_class* i_this) {
    return 1;
}

static int daVrbox_Delete(vrbox_class* i_this) {
    return 1;
}

static int daVrbox_solidHeapCB(fopAc_ac_c* i_this) {
    vrbox_class* this_ = (vrbox_class*)i_this;

#if TARGET_PC
    // ========================================================================
    // §418 WW LEG (donor daVrbox_solidHeapCB, WW d_a_vrbox.cpp:163-172):
    // on a WW sky host this actor owns vr_sky.bdl from the staged WwSky arc.
    // Same create flags as the donor (0x80000, 0x11020202).
    // ========================================================================
    if (dKyWw_isSkyHost()) {
        // §418b CRASH FIX (symbolicated 154940: create on RAW bytes, fault
        // 0xe0020000 = big-endian file data as pointer): the custom-mounted
        // WwSky arc serves UNPARSED BDL bytes -- and the J3D parser fixes
        // pointers IN PLACE, so parsing here a second time would corrupt the
        // mount's copy (the sumo-BMT crash class). The mount's cache bridge is
        // mount's parse-once cache: the dome models are already in it, so
        // this is a cache HIT, never a re-parse.
        J3DModelData* wwData = dExtWwMount_acquireModelData("WwSky", "vr_sky.bdl");
        this_->mpWwSky = wwData != NULL ? mDoExt_J3DModel__create(wwData, 0x80000, 0x11020202)
                                        : NULL;
        return this_->mpWwSky != NULL;
    }
#endif

    J3DModelData* modelData = (J3DModelData*)dComIfG_getStageRes("vrbox_sora.bmd");
    this_->mpSoraModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11020202);

    return modelData != NULL && this_->mpSoraModel != NULL;
}

static int daVrbox_Create(fopAc_ac_c* i_this) {
    fopAcM_ct(i_this, vrbox_class);
    vrbox_class* this_ = (vrbox_class*)i_this;

    this_->field_0x574 = 0;
    int phase = cPhs_COMPLEATE_e;

    if (fopAcM_entrySolidHeap(this_, daVrbox_solidHeapCB, 0xC60)) {
        // Donor contract (WW d_a_vrbox.cpp:185-187): a live vrbox = "this
        // stage HAS a sky" -- the very bit the §415 bridge asserted by hand.
        dComIfGp_onStatus(1);
        g_env_light.hide_vrbox = false;
#if TARGET_PC
        if (dKyWw_isSkyHost()) {
            dKyWw_setDomeActorsLive(true);  // §418: mount dome draw retires
            DuskLog.info("[WwSky] 418 NATIVE vrbox LIVE -- vr_sky owned by the real actor");
        }
#endif
    } else {
        phase = cPhs_ERROR_e;
    }

    return phase;
}

static DUSK_CONST actor_method_class l_daVrbox_Method = {
    (process_method_func)daVrbox_Create,  (process_method_func)daVrbox_Delete,
    (process_method_func)daVrbox_Execute, (process_method_func)daVrbox_IsDelete,
    (process_method_func)daVrbox_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_VRBOX = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_VRBOX_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(vrbox_class),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_VRBOX_e,
    /* Actor SubMtd */ &l_daVrbox_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
