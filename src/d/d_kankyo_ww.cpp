// ============================================================
// §404 WW LIGHTING PORT — donor-verbatim setLightTevColorType(_sub) from
// WW DP src/d/d_kankyo.cpp:1763-1873 (dtk address markers kept). See the
// header for the full campaign note. Seams, each labeled inline:
//   [S1] tevstr field names: donor mColorC0/mColorK0/mFogColor are the
//        receiver's TevColor/TevKColor/FogCol (same slots, TP names —
//        d_kankyo_tev_str.h kept WW's layout).
//   [S2] donor mColorK1 (second-light color, filled by WW's point-light
//        influence chain) has NO receiver source yet — a zero-alpha K1 runs
//        the donor's OWN else-leg, which DISABLES the material's extra
//        second-light TEV stage (donor-correct for "no second light").
//        Wiring K1 from dKy_plight influence (the lamp!) is the follow-up.
//   [S3] toon_proc_check(): WW retail runs toon OFF — the else branch is
//        the retail path; the receiver has no WW toon system. Constant.
//   [S4] donor's VERSION>DEMO mInitType guard (re-init half-built tevstrs)
//        skipped: receiver tevstrs have no mInitType and every WW-mounted
//        actor runs TP settingTevStruct before draw.
// ============================================================

#include "d/d_kankyo_ww.h"

#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "SSystem/SComponent/c_xyz.h"
#include "d/d_kankyo.h"
#include "d/d_com_inf_game.h"
#include "dusk/logging.h"

/* 80193650-80193A30       .text setLightTevColorType_sub__FP11J3DMaterialP12dKy_tevstr_c */
void dKyWw_setLightTevColorType_sub(J3DMaterial* i_material, dKy_tevstr_c* i_tevstr) {
    if (i_tevstr->mLightMode != 0) {
        J3DColorChan* colorchan_p = i_material->getColorChan(0);
        colorchan_p->setLightMask(1);
        int var_r28;
        int prev_a = i_material->getTevColor(3)->a;  // receiver J3DGXColorS10: direct fields, no .mColor

        var_r28 = 0xFF;
        if (prev_a > 0 && i_material->getTevBlock()->getTevKColorSel(prev_a - 1) == 13) {
            var_r28 = prev_a - 1;
        }

        if (var_r28 != 0xFF) {
            // [S2] donor reads i_tevstr->mColorK1 — no receiver source yet, so
            // the zero-alpha else-leg (donor's own "no second light" handling)
            // runs: the material's extra second-light TEV stage is disabled.
            GXColor colorK1 = {0, 0, 0, 0};
            if (colorK1.a != 0) {
                i_material->getTevBlock()->setTevStageNum(var_r28 + 1);
                if (i_material->getTevKColor(1) != NULL) {
                    i_material->setTevKColor(1, (J3DGXColor*)&colorK1);
                    colorchan_p->setLightMask(3);
                }
            } else {
                i_material->getTevBlock()->setTevStageNum(var_r28);
            }
        }
    }

    if (j3dSys.getViewMtx() != NULL) {
        cXyz sp14;
        MTXMultVec(j3dSys.getViewMtx(), &i_tevstr->mLightPosWorld, &sp14);
        // receiver getLightInfo() returns a pointer (donor: reference)
        i_tevstr->mLightObj.getLightInfo()->mLightPosition = sp14;
    }

    i_material->setLight(0, &i_tevstr->mLightObj);

    // [S3] donor `toon_proc_check()` — WW retail toon-off else branch:
    {
        J3DGXColorS10* col_p = i_material->getTevColor(0);
        if (col_p != NULL) {
            // [S1] donor mColorC0 -> receiver TevColor (alpha preserved, donor idiom)
            i_tevstr->TevColor.a = col_p->a;
            i_material->setTevColor(0, (J3DGXColorS10*)&i_tevstr->TevColor);
        }

        J3DGXColor* kcol_p = i_material->getTevKColor(0);
        if (kcol_p != NULL) {
            // [S1] donor mColorK0 -> receiver TevKColor
            i_tevstr->TevKColor.a = kcol_p->a;
            i_material->setTevKColor(0, (J3DGXColor*)&i_tevstr->TevKColor);
        }
    }

    if (i_material->getFog() != NULL) {
        J3DFogInfo* fog_info = i_material->getFog()->getFogInfo();
        if (fog_info->mType != 0) {
            fog_info->mStartZ = i_tevstr->mFogStartZ;
            fog_info->mEndZ = i_tevstr->mFogEndZ;
            if (fog_info->mStartZ > fog_info->mEndZ) {
                fog_info->mStartZ = fog_info->mEndZ;
            }

            fog_info->mNearZ = dComIfGd_getView()->near_;  // receiver view_class field names
            fog_info->mFarZ = dComIfGd_getView()->far_;
            // [S1] donor mFogColor -> receiver FogCol
            fog_info->mColor.r = i_tevstr->FogCol.r;
            fog_info->mColor.g = i_tevstr->FogCol.g;
            fog_info->mColor.b = i_tevstr->FogCol.b;
            fog_info->mAdjEnable = g_env_light.mFogAdjEnable;

            if (fog_info->mAdjEnable == 1) {
                // receiver idiom (d_kankyo.cpp:4470): table lives in mXFogTbl
                fog_info->mCenter = g_env_light.mFogAdjCenter;
                memcpy(&fog_info->mFogAdjTable, &g_env_light.mXFogTbl, sizeof(GXFogAdjTable));
            }
        }
    }
}

/* 80193A34-80193ADC       .text setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c */
void dKyWw_setLightTevColorType(J3DModel* i_model, dKy_tevstr_c* i_tevstr) {
    // [S4] donor VERSION>DEMO mInitType re-init guard skipped (see header).
    // ====================================================================
    // §406 CANARY — the black-actor class becomes a LOG LINE. An all-zero
    // C0+K0 pair is the memset signature of a tevstr that never went through
    // dKyWw_settingTevStruct (real palettes are never 0/0/0 on both). Every
    // wiring miss in this campaign was silent black; this makes the next one
    // name itself. First 8 occurrences logged, then quiet.
    // ====================================================================
    if (i_tevstr->TevColor.r == 0 && i_tevstr->TevColor.g == 0 && i_tevstr->TevColor.b == 0 &&
        i_tevstr->TevKColor.r == 0 && i_tevstr->TevKColor.g == 0 && i_tevstr->TevKColor.b == 0) {
        static int s_unfed = 0;
        if (s_unfed < 8) {
            ++s_unfed;
            DuskLog.warn("[KankyoWw] 406 UNFED tevstr {} (model {}) — actor draws BLACK; "
                         "route its draw through dKyWw_settingTevStruct",
                         (const void*)i_tevstr, (const void*)i_model);
        }
    }
    int mat_num = i_model->getModelData()->getMaterialNum() - 1;
    while (mat_num >= 0) {
        dKyWw_setLightTevColorType_sub(i_model->getModelData()->getMaterialNodePointer(mat_num),
                                       i_tevstr);
        mat_num--;
    }
}

// ============================================================
// §405 THE TEVSTR FEEDER — the donor settingTevStruct TAIL (WW DP
// d_kankyo.cpp:1747-1760), the half §404 revealed missing.
//
// WW and TP split one ancestral pipeline at exactly this write:
//   WW: settingTevStruct fills tevstr mColorC0/mColorK0 → setLightTevColorType
//       copies tevstr → material registers (the §404 port).
//   TP: settingTevStruct fills AmbCol/fog/lights ONLY — the per-material
//       colors are computed later inside setLightTevColorType_MAJI. TP's
//       tevstr TevColor/TevKColor are written NOWHERE on the actor path
//       (d_kankyo.cpp: only the 12/13 leg touches them, to ZERO them).
// §404's header assumed TP filled them; it does not — which is why every
// WW actor went black the moment §404 faithfully copied tevstr → material.
//
// This wrapper = the receiver's own settingTevStruct (shared-lineage front
// half: room resolve, per-envr per-colpat palette blend, fog, light objs)
// + the donor tail. Sources, both live-blended every frame from the
// №113-converted palettes (convert_lighting.py:160-192 maps the FULL WW
// palette; nothing was dropped):
//   C0  ← tevstr AmbCol — TP's type-0 leg computes the per-room, per-envr,
//         time-blended WW Actor_C0 into AmbCol (actor_amb_col carries the
//         donor's mActor_C0). This IS the donor's colget_actor product,
//         computed by the receiver's own copy of that machinery.
//   K0  ← g_env_light.dungeonlight_col[0] — №113 stash slot [0] = Actor_K0,
//         time-blended by the receiver's own 6-slot plight blend
//         (d_kankyo.cpp:2417, dKy_calc_color_set — the donor's math).
// Seams:
//   [S5] per-tevstr envr override reaches C0 (via TP's colget_actor) but not
//        K0 — the K0 blend is global-envr. Donor setLight_actor also blends
//        K0 per-tevstr; porting that refinement is the follow-up. Room-scale
//        spaces have uniform envr, so this is invisible until an actor
//        straddles a poly-envr boundary.
//   [S6] TP's wolf-power-up state zeroes dungeonlight_col — WW K0 goes dark
//        for its duration. TP-only state; acceptable until observed.
//   [S7] donor plightcol_plus (point-light additive INTO C0/K0 — the lamp
//        warming Grandma) not yet run; §404's [S2] K1 feeder lands with it.
// BG types (0xF0-flagged on the receiver) pass through untouched — the
// mount BG/sea draw paths are explicitly NOT on this feeder yet (§404
// hand-off), and the WW BG K0 slots [1..4] wait for that land.
// ============================================================
void dKyWw_settingTevStruct(int i_lightType, cXyz* i_pos, dKy_tevstr_c* i_tevstr) {
    g_env_light.settingTevStruct(i_lightType, i_pos, i_tevstr);
    if ((i_lightType & 0xF0) != 0 || i_lightType == 12 || i_lightType == 13 ||
        i_lightType == 14) {
        return;  // receiver-native types: TP legs own these tevstrs entirely
    }
    // ====================================================================
    // §406 donor BG legs (WW TEV_TYPE_BG0..BG3 = 1..4, _FULL = 5..8). The
    // donor selects the room's BG palette pair (d_kankyo.cpp:1690-1745):
    //   C0 ← bg_amb_col[n]        (converted WW BG*_C0, live-blended)
    //   K0 ← dungeonlight_col[1+n] (№113 stash [1..4] = BG0-3_K0, live-blended)
    //   mLightMode = 0             (donor BG leg; overrides TP's type-1..8
    //                               routing through the actor leg, which set 1)
    // _FULL maps to the same pair this land; is_full only alters the donor's
    // debug light-info color. [S8] fog stays receiver-computed for BG types.
    // ====================================================================
    if (i_lightType >= TEV_TYPE_BG0 && i_lightType <= TEV_TYPE_BG3_FULL) {
        const int n = (i_lightType - 1) & 3;
        const GXColorS10& c0 = g_env_light.bg_amb_col[n];
        i_tevstr->TevColor.r = c0.r;
        i_tevstr->TevColor.g = c0.g;
        i_tevstr->TevColor.b = c0.b;
        i_tevstr->TevColor.a = 255;
        const GXColorS10& bgk0 = g_env_light.dungeonlight_col[1 + n];
        i_tevstr->TevKColor.r = (u8)(bgk0.r < 0 ? 0 : (bgk0.r > 255 ? 255 : bgk0.r));
        i_tevstr->TevKColor.g = (u8)(bgk0.g < 0 ? 0 : (bgk0.g > 255 ? 255 : bgk0.g));
        i_tevstr->TevKColor.b = (u8)(bgk0.b < 0 ? 0 : (bgk0.b > 255 ? 255 : bgk0.b));
        i_tevstr->mLightMode = 0;
        return;
    }
    // Donor tail, actor leg (0/PLAYER/UNK99): mC0.a=255; tevstr->mColorC0 = mC0;
    // K0 rgb-only (alpha is preserved per-material by the _sub, as the donor does).
    i_tevstr->TevColor.r = i_tevstr->AmbCol.r;
    i_tevstr->TevColor.g = i_tevstr->AmbCol.g;
    i_tevstr->TevColor.b = i_tevstr->AmbCol.b;
    i_tevstr->TevColor.a = 255;
    const GXColorS10& k0 = g_env_light.dungeonlight_col[0];
    i_tevstr->TevKColor.r = (u8)(k0.r < 0 ? 0 : (k0.r > 255 ? 255 : k0.r));
    i_tevstr->TevKColor.g = (u8)(k0.g < 0 ? 0 : (k0.g > 255 ? 255 : k0.g));
    i_tevstr->TevKColor.b = (u8)(k0.b < 0 ? 0 : (k0.b > 255 ? 255 : k0.b));
}
