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
#include "d/d_kankyo_ww_sky.h"

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
            // [S2->§407 LIVE] donor i_tevstr->mColorK1 is the receiver's
            // mWwColorK1 (PC tevstr append), filled by the eflight feeder --
            // .a != 0 only when the player's fire light reaches this actor,
            // which enables the second-light TEV stage exactly as WW does.
            GXColor colorK1 = i_tevstr->mWwColorK1;
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
// §407: these influence accessors are DEFINED in d_kankyo.cpp (:888/:1006/
// :1037/:1041/:1045) but never declared in d_kankyo.h -- TP's own callers sit
// in the same TU. Declared here rather than editing the shared header.
int dKy_light_influence_id(cXyz position, int param_1);
GXColorS10 dKy_light_influence_col(int light_id);
f32 dKy_light_influence_power(int light_id);
f32 dKy_light_influence_yuragi(int light_id);
f32 dKy_light_influence_distance(cXyz position, int light_id);

// ============================================================
// §411 WW SKY HOST FLAG. The mount hides TP's vrbox stub via
// g_env_light.hide_vrbox (№108) -- but that flag's TP meaning is "this stage
// has NO sky", and the wether system correctly suppresses the ENTIRE
// celestial layer behind it (sun gate d_kankyo_wether.cpp:476, vrkumo :896).
// That is why the mounted island has no sun, moon, stars or cumulus: one
// flag aimed at a model suppressed four native systems.
//
// This flag says the truer thing: "a WW sky is hosted here -- TP's vrbox
// model is replaced, but the sky is ALIVE." The wether gates consult it to
// bypass hide_vrbox and the host roomRead vrboxswitch (mounted rooms are
// not in the host's roomRead, so the switch lookup can never pass), and to
// select WW gating values and WW resources where the lineages diverge.
// ============================================================
static bool s_wwSkyHost = false;
void dKyWw_setSkyHost(bool i_active) {
    if (s_wwSkyHost != i_active) {
        // ================================================================
        // §413 RE-INIT: the wether systems latch resources at frame 1 —
        // BEFORE the mount loads the WW arcs and flips this flag. Vrkumo's
        // case-0 even hard-fails to status 99 if cloudtx was not resident,
        // and 99 is permanent. Force re-init on every transition so case-0
        // re-runs against the resources this flag selects. (This was the
        // likely "no visible change" on the §412 build.)
        // ================================================================
        if (g_env_light.mVrkumoStatus != 0 && g_env_light.mpVrkumoPacket != NULL) {
            JKR_DELETE(g_env_light.mpVrkumoPacket);
            g_env_light.mpVrkumoPacket = NULL;
        }
        g_env_light.mVrkumoStatus = 0;
        g_env_light.mVrkumoCount = 0;
        dKyWwSky_reset();
        dKyWw_setDomeActorsLive(false);  // §418 actors do not survive stage change
        // ====================================================================
        // §424: the §415 onStatus(1) bridge RETIRED (History §422c: clouds
        // sealed, dome actors continue). The native daVrbox_Create WW leg now
        // owns the "stage HAS a sky" bit, donor-exact: the wether systems
        // start the frame the actor lives, and stay donor-correctly dark if
        // it ever fails to build.
        // ====================================================================
        DuskLog.info("[WwSky] 413 skyHost {} -> {}: wether re-armed (vrkumo/sun/star)",
                     (int)s_wwSkyHost, (int)i_active);
    }
    s_wwSkyHost = i_active;
}
bool dKyWw_isSkyHost() { return s_wwSkyHost; }

// §418 dome ownership handshake: set by daVrbox_Create's WW leg, cleared on
// sky-host transitions (actors are deleted by the framework on stage change).
static bool s_wwDomeActorsLive = false;
void dKyWw_setDomeActorsLive(bool i_live) { s_wwDomeActorsLive = i_live; }
bool dKyWw_domeActorsLive() { return s_wwDomeActorsLive; }

// ============================================================
// §407 THE POINT-LIGHT + EFLIGHT FEEDERS -- donor-verbatim
// settingTevStruct_plightcol_plus (WW d_kankyo.cpp:1437-1554) and
// settingTevStruct_eflightcol_plus (:1557-1588). This makes §404's [S2] and
// §405's [S7] LIVE: the lamp's registered flame (d_a_lamp.cpp:303,
// dKy_plight_set -- the registry side was already ported) now warms nearby
// WW actors' C0/K0, and the player's fire light (TP's lantern, tracked by
// the receiver's own mPlayerEfLightIdx, d_kankyo.cpp:4564) feeds mWwColorK1
// -> the second TEV stage + light slot 1 in the material write.
// Seams:
//   [S9]  donor scratch mC0/mK0 (env-light members) -> in/out params; the
//         donor's mK0 doubles as the influence-color scratch -- here a local
//         `lightCol`. Same math, same order.
//   [S10] receiver has no dKy_eflight_influence_power/_distance/_col
//         wrappers (only _id) -- efplight[] fields read directly; values
//         identical to the donor accessors.
//   [S11] toon_proc_check(): retail toon-off, else-branch constant ([S3]).
//   [S12] donor mpPLights[i]->mIdx -> receiver pointlight[i]->mIndex.
// ============================================================
static void dKyWw_settingTevStruct_plightcol_plus(cXyz* i_pos, dKy_tevstr_c* i_tevstr,
                                                  GXColorS10* io_c0, GXColorS10* io_k0,
                                                  u8 timer) {
    dScnKy_env_light_c& envLight = g_env_light;
    MtxP viewMtx = j3dSys.getViewMtx();

    f32 lightPower;
    f32 lightDist;
    f32 lightYuragi = 0.0f;
    BOOL resetLight;
    BOOL hasLight;
    s32 lightIdx;
    cXyz lightPos;
    cXyz lightDirView;
    cXyz lightDir;
    GXColorS10 lightCol;  // [S9] donor's mK0 scratch

    resetLight = FALSE;
    lightIdx = dKy_light_influence_id(*i_pos, 0);
    hasLight = FALSE;

    if (lightIdx >= 0) {
        lightDist = dKy_light_influence_distance(*i_pos, lightIdx);
        lightPower = dKy_light_influence_power(lightIdx);

        if (lightPower < 1 / 1000.0f)
            lightPower = 0.001f;

        if (lightDist < lightPower + 1000.0f)
            hasLight = true;
    }

    if (!hasLight) {
        lightPos = envLight.base_light.mPosition;
        lightDist = envLight.base_light.mPosition.abs(*i_pos);
        lightPower = envLight.base_light.mPow;
        lightYuragi = envLight.base_light.mFluctuation;
        lightCol.r = 0;
        lightCol.g = 0;
        lightCol.b = 0;
    } else {
        lightPos = dKy_light_influence_pos(lightIdx);
        lightCol = dKy_light_influence_col(lightIdx);
        lightYuragi = dKy_light_influence_yuragi(lightIdx);

        if (envLight.pointlight[lightIdx]->mIndex < 0)  // [S12]
            resetLight = TRUE;
    }

    f32 bright;
    if (lightPower <= 0.0f || timer > 0)
        bright = 1.0f;
    else
        bright = lightDist / lightPower;
    if (bright > 1.0f)
        bright = 1.0f;
    bright *= bright;
    f32 speed = 1.0f - bright;

    if (lightYuragi < 1000.0f) {
        f32 r = 255.0f - (lightYuragi / 3.0f) * speed;
        i_tevstr->mLightObj.getLightInfo()->mColor.r = (u8)(r + (255.0f - r) * cM_rndF(1.0f));
    } else {
        i_tevstr->mLightObj.getLightInfo()->mColor.r = (u8)(lightYuragi - 1000.0f);
    }

    io_c0->r = io_c0->r + (s32)(lightCol.r * (speed * 0.2f));
    io_c0->g = io_c0->g + (s32)(lightCol.g * (speed * 0.2f));
    io_c0->b = io_c0->b + (s32)(lightCol.b * (speed * 0.2f));
    if (io_c0->r > 0xFF) io_c0->r = 0xFF;
    if (io_c0->g > 0xFF) io_c0->g = 0xFF;
    if (io_c0->b > 0xFF) io_c0->b = 0xFF;

    io_k0->r = io_k0->r + (s32)(lightCol.r * speed);
    io_k0->g = io_k0->g + (s32)(lightCol.g * speed);
    io_k0->b = io_k0->b + (s32)(lightCol.b * speed);
    if (io_k0->r > 0xFF) io_k0->r = 0xFF;
    if (io_k0->g > 0xFF) io_k0->g = 0xFF;
    if (io_k0->b > 0xFF) io_k0->b = 0xFF;

    if (timer > 0 || resetLight) {
        i_tevstr->mLightPosWorld = lightPos;
    } else {
        f32 oldDist = i_pos->abs(i_tevstr->mLightPosWorld) / 10000.0f;
        if (oldDist > 1.0f)
            oldDist = 1.0f;
        oldDist *= oldDist;

        f32 newDist = i_pos->abs(lightPos) / 5000.0f;
        if (newDist > 1.0f)
            newDist = 1.0f;
        newDist = 1.0f - newDist;

        newDist = newDist * newDist * newDist;
        f32 chaseSpeed = 10.0f + (oldDist * 10000.0f) + (newDist * 100.0f);
        cLib_addCalc(&i_tevstr->mLightPosWorld.x, lightPos.x, 0.5f, chaseSpeed, 0.001f);
        cLib_addCalc(&i_tevstr->mLightPosWorld.y, lightPos.y, 0.5f, chaseSpeed, 0.001f);
        cLib_addCalc(&i_tevstr->mLightPosWorld.z, lightPos.z, 0.5f, chaseSpeed, 0.001f);
    }

    // [S11] retail toon-off: donor else-branch only.
    lightDir = i_tevstr->mLightPosWorld;

    cMtx_multVec(viewMtx, &lightDir, &lightDirView);
    J3DLightInfo* info = i_tevstr->mLightObj.getLightInfo();
    info->mLightPosition = lightDirView;
    // [S13] donor mLightDir == receiver field_0x1064 (every receiver
    // mLightDirection write reads it: d_kankyo.cpp:3653/:3850/:4005).
    info->mLightDirection = g_env_light.field_0x1064;
    info->mColor.g = 0;
    info->mColor.b = 0;
    info->mColor.a = 0xFF;
    info->mCosAtten.x = 1.0f;
    info->mCosAtten.y = 0.0f;
    info->mCosAtten.z = 0.0f;
    info->mDistAtten.x = 1.0f;
    info->mDistAtten.y = 0.0f;
    info->mDistAtten.z = 0.0f;
}

static void dKyWw_settingTevStruct_eflightcol_plus(cXyz* i_pos, dKy_tevstr_c* i_tevstr) {
    s32 hasEflight = false;
    s32 efi = g_env_light.mPlayerEfLightIdx;

    if (efi >= 0 && g_env_light.efplight[efi] != NULL)
        hasEflight = true;

    if (hasEflight) {
        f32 power = g_env_light.efplight[efi]->mPow;  // [S10]
        if (power > 0.0f) {
            f32 bright = i_pos->abs(g_env_light.efplight[efi]->mPosition) / power;
            if (bright < 1.0f) {
                i_tevstr->mWwColorK1.a = 1;
                GXColorS10 color = g_env_light.efplight[efi]->mColor;
                s16 col_r = color.r;
                s16 col_g = color.g;
                s16 col_b = color.b;

                bright = 1.0f - bright;
                s16 r = (s16)(col_r * bright);
                s16 g = (s16)(col_g * bright);
                s16 b = (s16)(col_b * bright);
                if (r > 0xFF) r = 0xFF;
                if (g > 0xFF) g = 0xFF;
                if (b > 0xFF) b = 0xFF;
                i_tevstr->mWwColorK1.r = (u8)(r * bright);
                i_tevstr->mWwColorK1.g = (u8)(g * bright);
                i_tevstr->mWwColorK1.b = (u8)(b * bright);
            }
        }
    }
}

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
        // ================================================================
        // §408 donor add-colours (setLight_bg, WW d_kankyo.cpp:1120-1240):
        // every BG C0 takes an amb add term -- BG1 uniquely its OWN pair
        // (mBg1AddColAmb, receiver bg1_addcol_amb), BG0/2/3 the shared one
        // (mBgAddColAmb, receiver bg_addcol_amb). This also makes the BG1
        // leg byte-identical to dKy_get_seacolor's amb, which is the receipt
        // for the sea cutover riding this leg.
        // [S15] the DIF add twins exist on the receiver only as unnamed
        // padding (unk_0x1178/...), and dKy_get_seacolor itself omits the
        // dif add -- matched here; naming + wiring those fields is a
        // follow-up with noclip d_kankyo.ts setLight_bg as the referee.
        // ================================================================
        const GXColorS10& add = (n == 1) ? g_env_light.bg1_addcol_amb
                                         : g_env_light.bg_addcol_amb;
        const GXColorS10& c0 = g_env_light.bg_amb_col[n];
        i_tevstr->TevColor.r = (s16)(c0.r + add.r);
        i_tevstr->TevColor.g = (s16)(c0.g + add.g);
        i_tevstr->TevColor.b = (s16)(c0.b + add.b);
        i_tevstr->TevColor.a = 255;
        const GXColorS10& bgk0 = g_env_light.dungeonlight_col[1 + n];
        i_tevstr->TevKColor.r = (u8)(bgk0.r < 0 ? 0 : (bgk0.r > 255 ? 255 : bgk0.r));
        i_tevstr->TevKColor.g = (u8)(bgk0.g < 0 ? 0 : (bgk0.g > 255 ? 255 : bgk0.g));
        i_tevstr->TevKColor.b = (u8)(bgk0.b < 0 ? 0 : (bgk0.b > 255 ? 255 : bgk0.b));
        i_tevstr->mLightMode = 0;
        return;
    }
    // Donor tail, actor leg (0/PLAYER/UNK99): C0/K0 through the §407 plight
    // warm-up (donor order: base pair -> plightcol_plus modifies -> tail
    // writes), then the eflight K1 fill. Donor prologue zeroes mColorK1.a
    // each call (d_kankyo.cpp:1620) -- replicated before eflight re-arms it.
    GXColorS10 c0;
    c0.r = i_tevstr->AmbCol.r;
    c0.g = i_tevstr->AmbCol.g;
    c0.b = i_tevstr->AmbCol.b;
    c0.a = 255;
    GXColorS10 k0 = g_env_light.dungeonlight_col[0];
    if (k0.r < 0) k0.r = 0;
    if (k0.g < 0) k0.g = 0;
    if (k0.b < 0) k0.b = 0;
    i_tevstr->mWwColorK1.a = 0;
    if (i_lightType != TEV_TYPE_UNK99) {  // donor: plight/eflight skipped for UNK99
        dKyWw_settingTevStruct_plightcol_plus(i_pos, i_tevstr, &c0, &k0,
                                              i_tevstr->mInitTimer);
        dKyWw_settingTevStruct_eflightcol_plus(i_pos, i_tevstr);
    }
    i_tevstr->TevColor.r = c0.r;
    i_tevstr->TevColor.g = c0.g;
    i_tevstr->TevColor.b = c0.b;
    i_tevstr->TevColor.a = 255;
    i_tevstr->TevKColor.r = (u8)(k0.r > 255 ? 255 : k0.r);
    i_tevstr->TevKColor.g = (u8)(k0.g > 255 ? 255 : k0.g);
    i_tevstr->TevKColor.b = (u8)(k0.b > 255 ? 255 : k0.b);
}
