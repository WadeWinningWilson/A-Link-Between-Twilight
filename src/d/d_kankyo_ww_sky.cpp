// ============================================================
// §413 WW CELESTIAL LAYER -- see header. Seams:
//  [S17] donor dStageType_MISC_e legs dropped: the sky host is outdoor by
//        definition (№108 outdoor-F_DL* only) -- the outdoor branch of each
//        MISC check is taken unconditionally.
//  [S18] donor borderY = camera mTrimHeight (GC letterbox trim): 0 here.
//  [S19] donor mColpatWeather/mColpatCurr/mColPatBlend weather gates map to
//        the receiver's own shared-lineage names (mColpatWeather /
//        wether_pat1 / pat_ratio), exactly as the receiver's TP star move
//        maps them.
//  [S20] donor mSunPos2 snapshot: the TU ports donor setSunpos directly and
//        uses its own statics; snapshot timing (light pass) collapses to
//        move time -- one frame of drift donor-invisible at 80000 radius.
//  [S21] textures ride the staged WwAlways arc by donor resource ID
//        (foam-proven access shape, §97b).
//  [S22] packets are lazy statics, reset via dKyWwSky_reset(); the donor
//        deletes them in wether_delete -- session-lived here, flag-gated.
// ============================================================

#include "d/d_kankyo_ww_sky.h"

#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "SSystem/SComponent/c_angle.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_rain.h"
#include "d/d_kankyo_ww.h"
#include "f_op/f_op_camera_mng.h"
#include "m_Do/m_Do_lib.h"
#include "dusk/logging.h"
#include <math.h>

#ifndef DEG2RAD
#define DEG2RAD(d) ((d) * (3.14159265358979f / 180.0f))
#endif

// donor Always resource IDs (WW assets Always.h:119-128), staged in WwAlways.
static const int kWwTexSnow01 = 0x81;   // raw 64x64 I8, no BTI header
static const int kWwTexLensHalf = 0x82;
static const int kWwTexRingHalf = 0x85;
static const int kWwTexTaiyo = 0x86;    // sun disc
static const int kWwTexTukiA = 0x87;    // moon phases A..D
static const int kWwTexTukiB = 0x88;
static const int kWwTexTukiC = 0x89;
static const int kWwTexTukiD = 0x8A;

// BTI bind helpers -- donor dKyr_init_btitex / dKyr_set_btitex
// (WW d_kankyo_rain.cpp:86-104); receiver's own copies are file-static.
static void wwSkyInitBtitex(GXTexObj* i_obj, ResTIMG* i_img) {
    GXInitTexObj(i_obj, ((u8*)&i_img->format) + i_img->imageOffset, i_img->width, i_img->height,
                 (GXTexFmt)i_img->format, (GXTexWrapMode)i_img->wrapS,
                 (GXTexWrapMode)i_img->wrapT, (GXBool)(i_img->mipmapCount > 1));
}
static void wwSkySetBtitex(GXTexObj* i_obj, ResTIMG* i_img) {
    wwSkyInitBtitex(i_obj, i_img);
    GXInitTexObjLOD(i_obj, (GXTexFilter)i_img->minFilter, (GXTexFilter)i_img->magFilter,
                    i_img->minLOD * 0.125f, i_img->maxLOD * 0.125f, i_img->LODBias * 0.01f,
                    (GXBool)i_img->biasClamp, (GXBool)i_img->doEdgeLOD,
                    (GXAnisotropy)i_img->maxAnisotropy);
    GXLoadTexObj(i_obj, GX_TEXMAP0);
}

// ---------------------------------------------------------------------------
// Packets -- donor structs (WW d_kankyo_wether.h:13-51, :128-149) verbatim.
// ---------------------------------------------------------------------------
class dKyWwSun_Packet : public J3DPacket {
public:
    virtual void draw();
    virtual ~dKyWwSun_Packet() {}
    cXyz mPos[2];
    u32 mVizChkData[5];
    u8 field_0x3c;
    bool field_0x3d;
    f32 mVisibility;
    f32 mSunAlpha;
    f32 mMoonAlpha;
    GXColor mColor;
    u8* mpTextureData[5];
};

class dKyWwSunlenz_Packet : public J3DPacket {
public:
    virtual void draw();
    virtual ~dKyWwSunlenz_Packet() {}
    GXColor mColorTmp;
    u8* mpTexSnow01;
    u8* mpTexLensHalf;
    u8* mpTexRingHalf;
    cXyz mPositions[8];
    f32 field_0x80;
    f32 field_0x84;
    f32 field_0x88;
    f32 field_0x8c;
    f32 mAngleDeg;
    f32 mDistFalloff;
    s16 field_0x98;
    bool mbDrawLenzInSky;
};

struct WwSTAR_EFF {
    f32 mAnimCounter;
    f32 mSin;
};

class dKyWwStar_Packet : public J3DPacket {
public:
    virtual void draw();
    virtual ~dKyWwStar_Packet() {}
    u8* mpTexture;
    WwSTAR_EFF mEffect[1];
    s16 mEffectNum;
};

static dKyWwSun_Packet* s_sunPkt = NULL;
static dKyWwSunlenz_Packet* s_lenzPkt = NULL;
static dKyWwStar_Packet* s_starPkt = NULL;
static bool s_sunInit = false;
static bool s_starInit = false;
static f32 s_starCountTarget = 0.0f;
static s32 s_starCount = 0;
static cXyz s_sunPos(1.0f, 0.0f, 0.0f);
static cXyz s_moonPos(-1.0f, 0.0f, 0.0f);

static void wwSkyDrawSunBody(Mtx drawMtx, cXyz* pPos, GXColor& reg0, u8** pImg);
static void wwSkyDrawLenzflareBody(Mtx drawMtx, cXyz* pPos, u8** pImg);
static void wwSkyDrawStarBody(Mtx drawMtx);

void dKyWwSun_Packet::draw() {
    // §414-P9: J3D really invokes the packet (list plumbing proof).
    static bool s_once = false;
    if (!s_once) {
        s_once = true;
        DuskLog.info("[WwSky] 414-P9 sun packet draw() INVOKED by J3D");
    }
    wwSkyDrawSunBody(j3dSys.getViewMtx(), mPos, mColor, mpTextureData);
}
void dKyWwSunlenz_Packet::draw() { wwSkyDrawLenzflareBody(j3dSys.getViewMtx(), mPositions, &mpTexSnow01); }
void dKyWwStar_Packet::draw() { wwSkyDrawStarBody(j3dSys.getViewMtx()); }

// ---------------------------------------------------------------------------
// donor setSunpos (WW d_kankyo.cpp:582-605) [S20]
// ---------------------------------------------------------------------------
static void wwSkySetSunpos() {
    camera_class* camera_p = (camera_class*)dComIfGp_getCamera(0);
    if (camera_p == NULL) {
        return;
    }
    cXyz sp8;
    f32 var_f1;
    f32 curTime = g_env_light.getDaytime();
    if (curTime > 15.0f) {
        var_f1 = curTime - 15.0f;
    } else {
        var_f1 = curTime + 345.0f;
    }
    sp8.x = sinf(DEG2RAD(var_f1)) * 80000.0f;
    sp8.y = cosf(DEG2RAD(var_f1)) * 80000.0f;
    sp8.z = cosf(DEG2RAD(var_f1)) * -48000.0f;
    if (dComIfGp_event_runCheck() == FALSE) {
        s_sunPos.x = camera_p->view.lookat.eye.x + sp8.x;
        s_sunPos.y = camera_p->view.lookat.eye.y - sp8.y;
        s_sunPos.z = camera_p->view.lookat.eye.z + sp8.z;
        s_moonPos.x = camera_p->view.lookat.eye.x - sp8.x;
        s_moonPos.y = camera_p->view.lookat.eye.y + sp8.y;
        s_moonPos.z = camera_p->view.lookat.eye.z - sp8.z;
    }
}

// donor dKyr_moon_arrival_check (d_kankyo_rain.cpp:517-522)
static BOOL wwSkyMoonArrival() {
    f32 t = g_env_light.getDaytime();
    return (t > 277.5f || t < 112.5f) ? TRUE : FALSE;
}

// [S19] donor weather-kill: mColpatWeather / colpat blend -> receiver names.
static bool wwSkyWeatherKill() {
    if (g_env_light.mColpatWeather != 0) {
        return true;
    }
    if (g_env_light.wether_pat1 != 0 && g_env_light.pat_ratio > 0.5f) {
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// donor dKyr_sun_move (WW d_kankyo_rain.cpp:524-704) verbatim; [S17][S18]
// ---------------------------------------------------------------------------
static void wwSkySunMove() {
    dKyWwSun_Packet* pSunPkt = s_sunPkt;
    dKyWwSunlenz_Packet* pLenzPkt = s_lenzPkt;
    camera_class* pCamera = (camera_class*)dComIfGp_getCamera(0);
    if (pCamera == NULL) {
        return;
    }

    f32 pulsePos;
    f32 staringAtSunAmount = 0.0f;
    u8 numPointsVisible = 0, numCenterPointsVisible = 0;
    cXyz lightDir;
    // [S17] outdoor leg: aim at the WW sun position.
    dKyr_get_vectle_calc(&pCamera->view.lookat.eye, &s_sunPos, &lightDir);

    pSunPkt->mPos[0].x = pCamera->view.lookat.eye.x + lightDir.x * 8000.0f;
    pSunPkt->mPos[0].y = pCamera->view.lookat.eye.y + lightDir.y * 8000.0f;
    pSunPkt->mPos[0].z = pCamera->view.lookat.eye.z + lightDir.z * 8000.0f;

    f32 horizonY = (pSunPkt->mPos[0].y - pCamera->view.lookat.eye.y) / 8000.0f;
    if (horizonY < 0.0f)
        horizonY = 0.0f;
    if (horizonY >= 1.0f)
        horizonY = 1.0f;
    horizonY = 1.0f - horizonY;
    horizonY *= horizonY;
    pulsePos = 1.0f - horizonY;

    if (pSunPkt->field_0x3c != 0)
        pSunPkt->field_0x3c--;
    pSunPkt->field_0x3d = false;

    f32 curTime = g_env_light.getDaytime();
    if (curTime > 97.5f && curTime < 292.5f) {
        f32 borderY = 0.0f;  // [S18]
        s32 numPointsCulled = 0;

        cLib_addCalc(&pSunPkt->mSunAlpha, 1.0f, 0.5f, 0.1f, 0.01f);

        cXyz projected;
        mDoLib_project(pSunPkt->mPos, &projected);

        static const f32 sun_chkpnt[5][2] = {
            {0.0f, 0.0f}, {-10.0f, -20.0f}, {10.0f, 20.0f}, {-20.0f, 10.0f}, {20.0f, -10.0f},
        };

        for (s32 i = 0; i < 5; i++) {
            f32 screenBottom = 490.0f - borderY;
            cXyz chkpnt = projected;
            chkpnt.x -= sun_chkpnt[i][0];
            chkpnt.y -= sun_chkpnt[i][1];

            if (chkpnt.x > 0.0f && chkpnt.x < 640.0 && chkpnt.y > borderY &&
                chkpnt.y < screenBottom) {
                if (pSunPkt->mVizChkData[i] >= 0xFFFFFF) {
                    numPointsVisible++;
                    if (i == 0)
                        numCenterPointsVisible++;
                }
                dComIfGd_peekZ((s16)chkpnt.x, (s16)chkpnt.y, &pSunPkt->mVizChkData[i]);
            } else {
                numPointsCulled++;
            }
        }

        if (numPointsCulled != 0 && numPointsVisible != 0 && numCenterPointsVisible != 0) {
            numCenterPointsVisible = 1;
            numPointsVisible = 5;
        }

        if (numPointsVisible != 0) {
            if (pSunPkt->field_0x3c < 5)
                pSunPkt->field_0x3c += 2;
            pSunPkt->field_0x3d = true;
        }

        pLenzPkt->field_0x80 = pLenzPkt->field_0x88;
        pLenzPkt->field_0x84 = pLenzPkt->field_0x8c;
        pLenzPkt->field_0x88 = 1000000000.0f;  // donor: this is not G_CM3D_F_INF
        pLenzPkt->field_0x8c = 0.0f;

        cXyz center;
        center.x = 320.0f;
        center.y = 240.0f;
        center.z = 0.0f;
        pLenzPkt->mDistFalloff = center.abs(projected);
        pLenzPkt->mDistFalloff /= 450.0f;
        if (pLenzPkt->mDistFalloff > 1.0f)
            pLenzPkt->mDistFalloff = 1.0f;
        pLenzPkt->mDistFalloff = 1.0f - pLenzPkt->mDistFalloff;
        staringAtSunAmount = pLenzPkt->mDistFalloff * pLenzPkt->mDistFalloff;
        pLenzPkt->mDistFalloff = 1.0f - staringAtSunAmount;
        staringAtSunAmount = staringAtSunAmount * staringAtSunAmount;
    } else {
        cLib_addCalc(&pSunPkt->mSunAlpha, 0.0f, 0.5f, 0.1f, 0.01f);
        numPointsVisible = 0;
        pSunPkt->field_0x3c = 0;
        pSunPkt->field_0x3d = false;
    }

    if (wwSkyWeatherKill()) {  // [S19]
        numCenterPointsVisible = 0;
        numPointsVisible = 0;
    }

    if (curTime < 120.0f || curTime > 270.0f) {
        numCenterPointsVisible = 0;
        numPointsVisible = 0;
    }

    if (numCenterPointsVisible != 0) {
        if (numPointsVisible == 4)
            cLib_addCalc(&pSunPkt->mVisibility, 1.0f, 0.1f, 0.1f, 0.001f);
        if (numPointsVisible <= 3)
            cLib_addCalc(&pSunPkt->mVisibility, 0.0f, 0.1f, 0.2f, 0.001f);
        else
            cLib_addCalc(&pSunPkt->mVisibility, 1.0f, 0.5f, 0.2f, 0.01f);
    } else {
        if (numPointsVisible < 3)
            cLib_addCalc(&pSunPkt->mVisibility, 0.0f, 0.5f, 0.2f, 0.001f);
        else
            cLib_addCalc(&pSunPkt->mVisibility, 1.0f, 0.1f, 0.1f, 0.001f);
    }

    if (numPointsVisible >= 2) {
        pLenzPkt->mbDrawLenzInSky = false;
    } else {
        pLenzPkt->mbDrawLenzInSky = true;
    }

    if (pSunPkt->mPos[0].y > 0.0f && !pLenzPkt->mbDrawLenzInSky) {
        dKy_set_actcol_ratio(1.0f - staringAtSunAmount * pSunPkt->mVisibility);
        dKy_set_bgcol_ratio(1.0f - staringAtSunAmount * pSunPkt->mVisibility);
        dKy_set_fogcol_ratio(staringAtSunAmount * pSunPkt->mVisibility * pulsePos * 0.5f + 1.0f);
        dKy_set_vrboxcol_ratio(staringAtSunAmount * pSunPkt->mVisibility * pulsePos * 0.5f + 1.0f);
    }

    if (wwSkyMoonArrival()) {
        f32 alpha = (pSunPkt->mPos[0].y - pCamera->view.lookat.eye.y) / -8000.0f;
        alpha *= alpha;
        alpha *= 6.0f;
        if (alpha > 1.0f)
            alpha = 1.0f;
        cLib_addCalc(&pSunPkt->mMoonAlpha, alpha, 0.2f, 0.01f, 0.001f);
    } else {
        cLib_addCalc(&pSunPkt->mMoonAlpha, 0.0f, 0.2f, 0.01f, 0.001f);
    }
}

// ---------------------------------------------------------------------------
// donor dKyr_lenzflare_move (WW d_kankyo_rain.cpp:470-513) verbatim
// ---------------------------------------------------------------------------
static void wwSkyLenzflareMove() {
    dKyWwSun_Packet* pSunPkt = s_sunPkt;
    dKyWwSunlenz_Packet* pLenzPkt = s_lenzPkt;
    camera_class* pCamera = (camera_class*)dComIfGp_getCamera(0);
    if (pCamera == NULL || pSunPkt->mVisibility < 0.0001f) {
        return;
    }

    cXyz eyeVect;
    cXyz sunDirSmth;
    cXyz camFwd;
    cXyz vectle;
    cXyz projected;
    cXyz center;

    dKy_set_eyevect_calc2(pCamera, &eyeVect, 7200.0005f, 7200.0005f);

    dKyr_get_vectle_calc(&eyeVect, pSunPkt->mPos, &sunDirSmth);
    pLenzPkt->mPositions[0] = pSunPkt->mPos[0];
    pLenzPkt->mPositions[1] = pSunPkt->mPos[0];

    mDoLib_project(pLenzPkt->mPositions, &projected);

    center.x = 320.0f;
    center.y = 240.0f;
    center.z = 0.0f;

    dKyr_get_vectle_calc(&center, &projected, &vectle);
    s16 angle = cM_atan2s(vectle.x, vectle.y);

    pLenzPkt->mAngleDeg = angle;
    pLenzPkt->mAngleDeg *= S2DEG_CONSTANT;
    pLenzPkt->mAngleDeg += 180.0f;

    dKyr_get_vectle_calc(&pCamera->view.lookat.eye, &pCamera->view.lookat.center, &camFwd);

    f32 size = sunDirSmth.abs(camFwd) * 350.0f + 250.0f;

    for (int i = 2; i < 8; i++) {
        pLenzPkt->mPositions[i].x = pSunPkt->mPos[0].x - sunDirSmth.x * size * i;
        pLenzPkt->mPositions[i].y = pSunPkt->mPos[0].y - sunDirSmth.y * size * i;
        pLenzPkt->mPositions[i].z = pSunPkt->mPos[0].z - sunDirSmth.z * size * i;
    }
}

// ---------------------------------------------------------------------------
// donor dKyr_drawSun (WW d_kankyo_rain.cpp:1811-2119) verbatim: moon phases
// by save-day (getDate() % 7, previous day before 18:00), mirror flipX for
// waxing, dayscale squash, two-pass disc+glow; sun disc + glow.
// ---------------------------------------------------------------------------
static void wwSkyDrawSunBody(Mtx drawMtx, cXyz* pPos, GXColor& reg0, u8** pImg) {
    dKyWwSun_Packet* pSunPkt = s_sunPkt;
    dKyWwSunlenz_Packet* pSunlenzPkt = s_lenzPkt;
    camera_class* pCamera = (camera_class*)dComIfGp_getCamera(0);
    cXyz pos[4];
    cXyz sunPos;
    cXyz moonPos2;
    cXyz moonPos;
    cXyz vp;
    cXyz lp;
    bool bDrawSun = false;
    bool bDrawMoon = false;
    Mtx camMtx;
    Mtx rotMtx;
    GXColor reg1;
    GXTexObj texObj;

    if (pCamera == NULL) {
        return;
    }
    if (pSunPkt->mSunAlpha > 0.0f)
        bDrawSun = true;
    if (pSunPkt->mMoonAlpha > 0.0f)
        bDrawMoon = true;

    if (bDrawSun | bDrawMoon) {
        sunPos = *pPos;

        // [S17] outdoor leg of the MISC split: the moon mirrors the sun about
        // the eye (donor else-branch, d_kankyo_rain.cpp:1852-1860).
        moonPos.x = -(pPos->x - pCamera->view.lookat.eye.x);
        moonPos.y = -(pPos->y - pCamera->view.lookat.eye.y);
        moonPos.z = -(pPos->z - pCamera->view.lookat.eye.z);
        moonPos2.x = moonPos.x + pCamera->view.lookat.eye.x;
        moonPos2.y = moonPos.y + pCamera->view.lookat.eye.y;
        moonPos2.z = moonPos.z + pCamera->view.lookat.eye.z;

        int dayofweek = dComIfGs_getDate() % 7;  // donor dKy_get_dayofweek
        if (g_env_light.getDaytime() < 180.0f) {
            if (dayofweek != 0)
                dayofweek--;
            else
                dayofweek = 6;
        }

        s32 texidx = 0;
        f32 flipX = 1.0f;
        switch (dayofweek) {
        case 0: texidx = 0; flipX = 1.0f; break;
        case 1: texidx = 1; flipX = 1.0f; break;
        case 2: texidx = 2; flipX = 1.0f; break;
        case 3: texidx = 3; flipX = 1.0f; break;
        case 4: texidx = 3; flipX = -1.0f; break;
        case 5: texidx = 2; flipX = -1.0f; break;
        case 6: texidx = 1; flipX = -1.0f; break;
        }

        reg0.r = (u8)g_env_light.fog_col.r;
        reg0.g = (u8)g_env_light.fog_col.g;
        reg0.b = (u8)g_env_light.fog_col.b;
        reg0.a = 0xFF;

        reg1.r = 0x00;
        reg1.g = 0x00;
        reg1.b = 0x00;
        reg1.a = 0xFF;

        if (dComIfGd_getView() != NULL) {
            MTXInverse(dComIfGd_getViewRotMtx(), camMtx);
        } else {
            if (pSunPkt->field_0x3c < 5)
                pSunPkt->field_0x3c += 2;
            pSunPkt->field_0x3d = true;
            return;
        }

        wwSkySetBtitex(&texObj, (ResTIMG*)pImg[texidx]);

        GXSetNumChans(0);
        GXSetTevColor(GX_TEVREG0, reg0);
        GXSetTevColor(GX_TEVREG1, reg1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        GXSetNumIndStages(0);
        GXSetCullMode(GX_CULL_NONE);

        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

        if (bDrawMoon == true) {
            cXyz camfwd;
            static const f32 dayscale[7] = {1.0f, 0.83f, 0.6f, 0.6f, 0.6f, 0.6f, 0.83f};

            dKyr_get_vectle_calc(&pCamera->view.lookat.eye, &pCamera->view.lookat.center,
                                 &camfwd);

            f32 cam_theta = atan2f(camfwd.x, camfwd.z);

            f32 moon_distXZ = sqrtf(moonPos.x * moonPos.x + moonPos.z * moonPos.z);
            f32 moon_theta = atan2f(moonPos.x, moonPos.z);
            f32 moon_phi = atan2f(moonPos.y, moon_distXZ);

            f32 angle = 45.0f + (((moon_theta - cam_theta) / -8.0f) * moon_phi) * 360.0f;
            MTXRotDeg(rotMtx, 'Z', angle);
            MTXConcat(camMtx, rotMtx, camMtx);
            GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);

            reg0.r = 0xF3;
            reg0.g = 0xFF;
            reg0.b = 0x94;

            f32 size = 700.0f;
            reg0.a = (u8)(pSunPkt->mMoonAlpha * 255.0f);
            GXSetTevColor(GX_TEVREG0, reg0);

            for (s32 j = 0; j < 2; j++) {
                if (j == 1) {
                    GXInitTexObj(&texObj, pSunlenzPkt->mpTexSnow01, 64, 64, GX_TF_I8, GX_CLAMP,
                                 GX_CLAMP, GX_FALSE);
                    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE,
                                    GX_FALSE, GX_ANISO_1);
                    GXLoadTexObj(&texObj, GX_TEXMAP0);
                    size *= 1.7f;
                    reg0.a = (u8)(pSunPkt->mMoonAlpha * 76.0f);
                    reg0.r = 0xFF;
                    reg0.g = 0xFF;
                    reg0.b = 0xCF;
                    reg1.r = 0xC5;
                    reg1.g = 0x69;
                    reg1.b = 0x23;
                    MTXRotDeg(rotMtx, 'Z', 50.0f * flipX);
                    MTXConcat(camMtx, rotMtx, camMtx);
                    GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
                    GXSetCurrentMtx(GX_PNMTX0);
                }

                GXSetTevColor(GX_TEVREG0, reg0);
                GXSetTevColor(GX_TEVREG1, reg1);

                vp.x = -size * flipX;
                vp.y = size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[0].x = moonPos2.x + lp.x;
                pos[0].y = moonPos2.y + lp.y;
                pos[0].z = moonPos2.z + lp.z;

                vp.x = size * flipX;
                vp.y = size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[1].x = moonPos2.x + lp.x;
                pos[1].y = moonPos2.y + lp.y;
                pos[1].z = moonPos2.z + lp.z;

                if (texidx == 0) {
                    vp.x = size * flipX;
                    vp.y = -size;
                } else {
                    vp.x = size * flipX * dayscale[dayofweek];
                    vp.y = -size * dayscale[dayofweek];
                }
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[2].x = moonPos2.x + lp.x;
                pos[2].y = moonPos2.y + lp.y;
                pos[2].z = moonPos2.z + lp.z;

                vp.x = -size * flipX;
                vp.y = -size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[3].x = moonPos2.x + lp.x;
                pos[3].y = moonPos2.y + lp.y;
                pos[3].z = moonPos2.z + lp.z;

                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(pos[0].x, pos[0].y, pos[0].z);
                GXTexCoord2s16(0, 0);
                GXPosition3f32(pos[1].x, pos[1].y, pos[1].z);
                GXTexCoord2s16(0xFF, 0);
                GXPosition3f32(pos[2].x, pos[2].y, pos[2].z);
                GXTexCoord2s16(0xFF, 0xFF);
                GXPosition3f32(pos[3].x, pos[3].y, pos[3].z);
                GXTexCoord2s16(0, 0xFF);
                GXEnd();
            }
        }

        if (bDrawSun == true) {
            cXyz camfwd;

            f32 sun_theta = atan2f(sunPos.x, sunPos.z);

            dKyr_get_vectle_calc(&pCamera->view.lookat.eye, &pCamera->view.lookat.center,
                                 &camfwd);
            f32 cam_theta = atan2f(camfwd.x, camfwd.z);

            MTXRotDeg(rotMtx, 'Z', -50.0f + (360.0f * ((sun_theta - cam_theta) / -8.0f)));
            MTXConcat(camMtx, rotMtx, camMtx);
            GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);

            reg0.r = 0xFF;
            reg0.g = 0xFF;
            reg0.b = 0xF1;

            reg1.r = 0xF1;
            reg1.g = 0x91;
            reg1.b = 0x49;

            f32 dist = 1.0f - pSunlenzPkt->mDistFalloff;
            f32 size = 575.0f;
            if (pSunPkt->mVisibility > 0.0f)
                size += 500.0f * (dist * dist) * pSunPkt->mVisibility;

            for (s32 j = 0; j < 2; j++) {
                if (j == 0) {
                    wwSkySetBtitex(&texObj, (ResTIMG*)pImg[4]);
                    reg0.a = (u8)(pSunPkt->mSunAlpha * 255.0f);
                } else {
                    GXInitTexObj(&texObj, pSunlenzPkt->mpTexSnow01, 64, 64, GX_TF_I8, GX_CLAMP,
                                 GX_CLAMP, GX_FALSE);
                    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE,
                                    GX_FALSE, GX_ANISO_1);
                    GXLoadTexObj(&texObj, GX_TEXMAP0);
                    size *= 1.6f;
                    reg0.a = (u8)(pSunPkt->mSunAlpha * 76.0f);
                }

                GXSetTevColor(GX_TEVREG0, reg0);
                GXSetTevColor(GX_TEVREG1, reg1);

                vp.x = -size * flipX;
                vp.y = size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[0].x = sunPos.x + lp.x;
                pos[0].y = sunPos.y + lp.y;
                pos[0].z = sunPos.z + lp.z;

                vp.x = size * flipX;
                vp.y = size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[1].x = sunPos.x + lp.x;
                pos[1].y = sunPos.y + lp.y;
                pos[1].z = sunPos.z + lp.z;

                vp.x = size * flipX;
                vp.y = -size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[2].x = sunPos.x + lp.x;
                pos[2].y = sunPos.y + lp.y;
                pos[2].z = sunPos.z + lp.z;

                vp.x = -size * flipX;
                vp.y = -size;
                vp.z = 0.0f;
                MTXMultVec(camMtx, &vp, &lp);
                pos[3].x = sunPos.x + lp.x;
                pos[3].y = sunPos.y + lp.y;
                pos[3].z = sunPos.z + lp.z;

                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(pos[0].x, pos[0].y, pos[0].z);
                GXTexCoord2s16(0, 0);
                GXPosition3f32(pos[1].x, pos[1].y, pos[1].z);
                GXTexCoord2s16(0xFF, 0);
                GXPosition3f32(pos[2].x, pos[2].y, pos[2].z);
                GXTexCoord2s16(0xFF, 0xFF);
                GXPosition3f32(pos[3].x, pos[3].y, pos[3].z);
                GXTexCoord2s16(0, 0xFF);
                GXEnd();
            }
        }
        J3DShape::resetVcdVatCache();
    }
}

// ---------------------------------------------------------------------------
// donor dKyr_drawLenzflare (WW d_kankyo_rain.cpp:2122-2447) verbatim.
// ---------------------------------------------------------------------------
static void wwSkyDrawLenzflareBody(Mtx drawMtx, cXyz* pPos, u8** pImg) {
    dKyWwSunlenz_Packet* lenz_packet = s_lenzPkt;
    dKyWwSun_Packet* sun_packet = s_sunPkt;

    Mtx camMtx;
    Mtx rotMtx;

    cXyz pos[4];
    cXyz spFC;
    cXyz spF0;

    Vec spE4, spD8;

    s16 spC = 0;
    s16 spA = 0;

    f32 sun_visibility = sun_packet->mVisibility;
    f32 spAC = 1.0f - lenz_packet->mDistFalloff;
    f32 spA8 = sun_packet->mVisibility * sun_packet->mVisibility;

    camera_class* camera = (camera_class*)dComIfGp_getCamera(0);
    if (camera == NULL || sun_visibility < 0.1f) {
        return;
    }

    dKy_set_eyevect_calc2(camera, &spFC, 8000.0f, 8000.0f);

    GXColor color_reg0;
    color_reg0.r = sun_packet->mColor.r;
    color_reg0.g = sun_packet->mColor.g;
    color_reg0.b = sun_packet->mColor.b;
    color_reg0.a = 0xFF;

    GXColor color_reg1;
    color_reg1.r = sun_packet->mColor.r;
    color_reg1.g = sun_packet->mColor.g;
    color_reg1.b = sun_packet->mColor.b;
    color_reg1.a = 0xFF;

    if (dComIfGd_getView() != NULL) {
        MTXInverse(dComIfGd_getViewRotMtx(), camMtx);
    } else {
        return;
    }

    GXTexObj texobj;
    GXInitTexObj(&texobj, pImg[0], 64, 64, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&texobj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                    GX_ANISO_1);
    GXLoadTexObj(&texobj, GX_TEXMAP0);
    GXSetNumChans(0);
    GXSetTevColor(GX_TEVREG0, color_reg0);
    GXSetTevColor(GX_TEVREG1, color_reg1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_DISABLE, GX_LEQUAL, GX_DISABLE);
    GXSetNumIndStages(0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    MTXRotRad(rotMtx, 'Z', 0.0f);
    MTXConcat(camMtx, rotMtx, camMtx);
    GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    static const f32 scale_dat[] = {
        8000.0f, 10000.0f, 1600.0f, 4800.0f, 1200.0f, 5600.0f, 2400.0f, 7200.0f,
    };

    static const GXColor col_dat[] = {
        {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0x50}, {0xFF, 0xFF, 0xFF, 0x5A},
        {0xFF, 0xFF, 0xFF, 0x64}, {0xFF, 0xFF, 0xFF, 0x55}, {0xFF, 0xFF, 0xFF, 0x5A},
        {0xFF, 0xFF, 0xFF, 0x6E}, {0xFF, 0xFF, 0xFF, 0x5A},
    };

    for (int i = 0; i < 8; i++) {
        if (!lenz_packet->mbDrawLenzInSky || i == 0) {
            f32 spC4 = spA8;
            if (i < 2) {
                color_reg0.a = (u8)(spC4 * col_dat[i].a);
            } else {
                color_reg0.a = (u8)(spC4 * col_dat[i].a * (0.8f * lenz_packet->mDistFalloff));
            }

            GXSetTevColor(GX_TEVREG0, color_reg0);

            f32 var_f31;
            if (i < 2) {
                var_f31 = (0.04f + (0.075f * sun_visibility)) * scale_dat[i] +
                          ((0.2f * sun_visibility * scale_dat[i]) *
                           (1.0f - (lenz_packet->mDistFalloff * lenz_packet->mDistFalloff)));
            } else {
                var_f31 = 0.8f * (sun_visibility * scale_dat[i]) *
                          (1.0f - (lenz_packet->mDistFalloff * lenz_packet->mDistFalloff *
                                   lenz_packet->mDistFalloff));
            }

            if (i == 1) {
                wwSkyInitBtitex(&texobj, (ResTIMG*)pImg[2]);
                GXLoadTexObj(&texobj, GX_TEXMAP0);
            } else if (i == 2) {
                wwSkyInitBtitex(&texobj, (ResTIMG*)pImg[1]);
                GXLoadTexObj(&texobj, GX_TEXMAP0);
            }

            spE4.x = -var_f31;
            spE4.y = var_f31;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[0].x = spD8.x + pPos[i].x;
            pos[0].y = spD8.y + pPos[i].y;
            pos[0].z = spD8.z + pPos[i].z;

            spE4.x = var_f31;
            spE4.y = var_f31;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[1].x = spD8.x + pPos[i].x;
            pos[1].y = spD8.y + pPos[i].y;
            pos[1].z = spD8.z + pPos[i].z;

            spE4.x = var_f31;
            spE4.y = -var_f31;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[2].x = spD8.x + pPos[i].x;
            pos[2].y = spD8.y + pPos[i].y;
            pos[2].z = spD8.z + pPos[i].z;

            spE4.x = -var_f31;
            spE4.y = -var_f31;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[3].x = spD8.x + pPos[i].x;
            pos[3].y = spD8.y + pPos[i].y;
            pos[3].z = spD8.z + pPos[i].z;

            s16 sp8;
            if (i == 0) {
                sp8 = 0xFF;
            } else {
                sp8 = 0x1FF;
            }

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(pos[0].x, pos[0].y, pos[0].z);
            GXTexCoord2s16(0, 0);
            GXPosition3f32(pos[1].x, pos[1].y, pos[1].z);
            GXTexCoord2s16(sp8, 0);
            GXPosition3f32(pos[2].x, pos[2].y, pos[2].z);
            GXTexCoord2s16(sp8, sp8);
            GXPosition3f32(pos[3].x, pos[3].y, pos[3].z);
            GXTexCoord2s16(0, sp8);
            GXEnd();
        }
    }

    spC = -0x07F6;
    spA = 0x416B;

    if (dComIfGd_getView() != NULL) {
        MTXInverse(dComIfGd_getViewRotMtx(), camMtx);
    }

    {
        MTXRotRad(rotMtx, 'Z', 0.0f);
        MTXConcat(camMtx, rotMtx, camMtx);
        GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);

        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);

        color_reg0.a = (u8)(sun_packet->mVisibility * (15.0f * (spA8 * spA8 * spA8)));
        GXSetTevColor(GX_TEVREG0, color_reg0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                        GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                        GX_TEVPREV);
        GXSetNumIndStages(0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);

        for (int i = 0; i < 16; i++) {
            f32 spA0, sp9C, sp98, sp94, sp8C, sp88, sp84;
            f32 sp80, sp7C, sp78, sp74, sp70, sp6C, sp68, sp64;
            f32 sp90;

            if (i & 1) {
                sp78 = spC;
            } else {
                sp78 = spA;
            }

            sp6C = 0.4f + (0.6f * (sun_visibility * (spAC * spAC)));
            sp64 = 300.0f;
            sp68 = cM_ssin((s16)(34.0f * sp78));
            if (sp68 < 0.0f) {
                sp68 = -sp68;
            }

            sp74 = 1000.0f * (0.5f + sp68);

            spA0 = sp64 * sp6C * cM_ssin((s16)(sp78 + sp74));
            sp9C = sp64 * sp6C * cM_scos((s16)(sp78 + sp74));
            sp90 = sp64 * sp6C * cM_ssin((s16)(sp78 - sp74));
            sp8C = sp64 * sp6C * cM_scos((s16)(sp78 - sp74));

            sp70 = (0.6f + (0.4f * sp68)) * (300.0f * sp6C * (3.0f + spAC));
            sp70 *= 1.5f * sun_visibility;

            if ((i & 3)) {
                sp70 *= 0.2f;
            }

            sp98 = sp70 * cM_ssin((s16)sp78);
            sp94 = sp70 * cM_scos((s16)sp78);
            spC += 0x1000;
            spA += 0x1C71;

            spE4.x = sp98;
            spE4.y = sp94;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            spF0.x = pPos[0].x + spD8.x;
            spF0.y = pPos[0].y + spD8.y;
            spF0.z = pPos[0].z + spD8.z;

            sp7C = spFC.abs(spF0);
            if (sp7C < lenz_packet->field_0x88) {
                lenz_packet->field_0x88 = sp7C;
            } else if (sp7C > lenz_packet->field_0x8c) {
                lenz_packet->field_0x8c = sp7C;
            }

            sp88 = lenz_packet->field_0x88 - lenz_packet->field_0x84;
            if (sp88 > 0.0f) {
                sp84 = 1.0f - ((sp7C - lenz_packet->field_0x84) / sp88);
            } else {
                sp84 = 1.0f;
            }
            (void)sp84;  // donor dead code, preserved

            switch (i & 3) {
            case 0: sp80 = 0.1f; break;
            case 1: sp80 = 1.1f; break;
            case 2: sp80 = 0.2f; break;
            case 3: sp80 = 0.4f; break;
            }

            f32 spC4 = spA8;
            sp98 *= sun_visibility * (spC4 + sp80);
            sp94 *= sun_visibility * (spC4 + sp80);

            spE4.x = spA0;
            spE4.y = sp9C;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[0].x = sun_packet->mPos[0].x + spD8.x;
            pos[0].y = sun_packet->mPos[0].y + spD8.y;
            pos[0].z = sun_packet->mPos[0].z + spD8.z;

            spE4.x = sp98;
            spE4.y = sp94;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[1].x = sun_packet->mPos[0].x + spD8.x;
            pos[1].y = sun_packet->mPos[0].y + spD8.y;
            pos[1].z = sun_packet->mPos[0].z + spD8.z;

            spE4.x = sp90;
            spE4.y = sp8C;
            spE4.z = 0.0f;
            cMtx_multVec(camMtx, &spE4, &spD8);
            pos[2].x = sun_packet->mPos[0].x + spD8.x;
            pos[2].y = sun_packet->mPos[0].y + spD8.y;
            pos[2].z = sun_packet->mPos[0].z + spD8.z;

            GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
            GXPosition3f32(pos[0].x, pos[0].y, pos[0].z);
            GXPosition3f32(pos[1].x, pos[1].y, pos[1].z);
            GXPosition3f32(pos[2].x, pos[2].y, pos[2].z);
            GXEnd();
        }
    }

    J3DShape::resetVcdVatCache();
}

// ---------------------------------------------------------------------------
// donor dKyr_drawStar (WW d_kankyo_rain.cpp:3064-3272) verbatim: 16 fixed
// constellation stars + procedural spiral field, pure vertex color, hexagram
// billboards, moon-proximity skip.
// ---------------------------------------------------------------------------
static void wwSkyDrawStarBody(Mtx drawMtx) {
    dKyWwStar_Packet* star_packet = s_starPkt;
    camera_class* camera = (camera_class*)dComIfGp_getCamera(0);

    static u32 rot = 0;

    cXyz pos[4];

    Mtx camMtx;
    cXyz spBC;
    cXyz moon_proj;
    cXyz star_proj;

    Vec sp98, sp8C;

    if (camera == NULL) {
        return;
    }

    static const csXyz hokuto_position[] = {
        csXyz(13000, 10500, -16000), csXyz(9400, 9800, -12646),  csXyz(10200, 11800, -13525),
        csXyz(10300, 13450, -13525), csXyz(15000, 18400, -16162), csXyz(12500, 19800, -15000),
        csXyz(9179, 17200, -14404),  csXyz(9500, 9800, -12646),  csXyz(-7421, 31005, 18798),
        csXyz(-10937, 28000, 15000), csXyz(-10000, 24902, 18400), csXyz(-9400, 22500, 15900),
        csXyz(-9179, 21300, 14300),  csXyz(-10300, 22000, 21000), csXyz(-16000, 25500, 20000),
        csXyz(0, 30000, 19000),
    };

    if (star_packet->mEffectNum != 0) {
        GXColor color_reg0;
        color_reg0.r = 0xDC;
        color_reg0.g = 0xE6;
        color_reg0.b = 0xFF;
        color_reg0.a = 0xFF;

        if (dComIfGd_getView() != NULL) {
            MTXInverse(dComIfGd_getViewRotMtx(), camMtx);
        } else {
            return;
        }

        mDoLib_project(&s_moonPos, &moon_proj);

        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetTevColor(GX_TEVREG0, color_reg0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                        GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                        GX_TEVPREV);
        GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
        GXSetNumIndStages(0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

        Mtx rotMtx;
        MTXRotRad(rotMtx, 'Z', DEG2RAD((f32)rot));
        MTXConcat(camMtx, rotMtx, camMtx);

        GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);

        rot++;
        if (rot > 719) {
            rot = 0;
        }

        spBC.x = camera->view.lookat.eye.x;
        spBC.y = camera->view.lookat.eye.y;
        spBC.z = camera->view.lookat.eye.z;

        f32 var_f30 = 0.0f;
        if (dComIfGd_getView() != NULL) {
            var_f30 = dComIfGd_getView()->fovy / 45.0f;
            if (var_f30 >= 1.0f) {
                var_f30 = 1.0f;
            }
            var_f30 = 1.0f - var_f30;
        }

        f32 temp_f27 = 0.28f * (1.0f - var_f30);

        sp98.x = 0.0f;
        sp98.y = temp_f27;
        sp98.z = 0.0f;
        cMtx_multVec(camMtx, &sp98, &sp8C);
        pos[0].x = spBC.x + sp8C.x;
        pos[0].y = spBC.y + sp8C.y;
        pos[0].z = spBC.z + sp8C.z;

        sp98.x = temp_f27;
        sp98.y = -(0.5f * temp_f27);
        sp98.z = 0.0f;
        cMtx_multVec(camMtx, &sp98, &sp8C);
        pos[1].x = spBC.x + sp8C.x;
        pos[1].y = spBC.y + sp8C.y;
        pos[1].z = spBC.z + sp8C.z;

        sp98.x = -temp_f27;
        sp98.y = -(0.5f * temp_f27);
        sp98.z = 0.0f;
        cMtx_multVec(camMtx, &sp98, &sp8C);
        pos[2].x = spBC.x + sp8C.x;
        pos[2].y = spBC.y + sp8C.y;
        pos[2].z = spBC.z + sp8C.z;

        int sp48 = 0;
        int sp44 = 0;
        f32 var_f28 = 0.0f;

        for (int i = 0; i < star_packet->mEffectNum; i++) {
            f32 star_size;
            cXyz star_pos;
            f32 sp2C = 300.0f;

            if (i < (s32)(sizeof(hokuto_position) / sizeof(hokuto_position[0]))) {
                star_pos.x = hokuto_position[i].x;
                star_pos.y = hokuto_position[i].y;
                star_pos.z = hokuto_position[i].z;

                if (i <= 7) {
                    star_size = 190.0f + star_packet->mEffect[0].mSin;
                } else {
                    star_size = 290.0f + star_packet->mEffect[0].mSin;
                }

                star_size -= temp_f27 * (0.5f * star_size);
            } else {
                star_size = star_packet->mEffect[0].mSin + (0.066f * (i & 0x0F));
                if (star_size > 1.0f)
                    star_size = (1.0f - (star_size - 1.0f));

                f32 temp_f29 = 1.0f - (var_f28 * (1.0f / 202.0f));
                star_pos.x = temp_f29 * (sp2C * -cM_ssin((s16)(sp48 - 0x8000)));
                star_pos.y = 45.0f + var_f28;
                star_pos.z = temp_f29 * (sp2C * cM_scos((s16)(sp48 - 0x8000)));

                sp48 += sp44;
                sp44 += 2250;

                temp_f29 = var_f28 / 200.0f;
                temp_f29 *= temp_f29 * temp_f29;
                var_f28 += 1.0f + (3.0f * temp_f29);
                if (var_f28 > 200.0f) {
                    var_f28 = (20.0f * i) / 1000.0f;
                }
            }

            static const GXColor star_col[] = {
                {0xFF, 0xBE, 0xC8, 0xA0},
                {0xC8, 0xFF, 0xBE, 0x78},
                {0xC8, 0xBE, 0xFF, 0x50},
                {0xFF, 0xFF, 0xFF, 0xC8},
            };

            if (i == 6 || i == 8)
                color_reg0 = star_col[1];
            else if ((i & 0x3F) == 0)
                color_reg0 = star_col[(i >> 4) & 3];
            else
                color_reg0 = star_col[0];
            GXSetTevColor(GX_TEVREG0, color_reg0);

            cXyz center;
            center.x = spBC.x + star_pos.x;
            center.y = spBC.y + star_pos.y;
            center.z = spBC.z + star_pos.z;

            mDoLib_project(&center, &star_proj);

            f32 moon_dist_to_star = moon_proj.abs(star_proj);
            f32 moon_threshold = 80.0f + (700.0f * (var_f30 * var_f30));

            if (moon_dist_to_star > moon_threshold) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 3);
                GXPosition3f32(center.x + (star_size * (pos[0].x - spBC.x)),
                               center.y + (star_size * (pos[0].y - spBC.y)),
                               center.z + (star_size * (pos[0].z - spBC.z)));
                GXPosition3f32(center.x + (star_size * (pos[1].x - spBC.x)),
                               center.y + (star_size * (pos[1].y - spBC.y)),
                               center.z + (star_size * (pos[1].z - spBC.z)));
                GXPosition3f32(center.x + (star_size * (pos[2].x - spBC.x)),
                               center.y + (star_size * (pos[2].y - spBC.y)),
                               center.z + (star_size * (pos[2].z - spBC.z)));
                GXEnd();

                GXBegin(GX_QUADS, GX_VTXFMT0, 3);
                GXPosition3f32(center.x - (star_size * (pos[0].x - spBC.x)),
                               center.y - (star_size * (pos[0].y - spBC.y)),
                               center.z - (star_size * (pos[0].z - spBC.z)));
                GXPosition3f32(center.x - (star_size * (pos[1].x - spBC.x)),
                               center.y - (star_size * (pos[1].y - spBC.y)),
                               center.z - (star_size * (pos[1].z - spBC.z)));
                GXPosition3f32(center.x - (star_size * (pos[2].x - spBC.x)),
                               center.y - (star_size * (pos[2].y - spBC.y)),
                               center.z - (star_size * (pos[2].z - spBC.z)));
                GXEnd();
            }
        }

        J3DShape::resetVcdVatCache();
    }
}

// ---------------------------------------------------------------------------
// Public move/draw -- donor wether_move_sun / wether_move_star shapes with
// WwAlways resources [S21].
// ---------------------------------------------------------------------------
void dKyWwSky_moveSun() {
    wwSkySetSunpos();
    if (!(dComIfGp_checkStatus(1))) {
        return;
    }
    if (!s_sunInit) {
        if (s_sunPkt == NULL) {
            s_sunPkt = new dKyWwSun_Packet();
            s_lenzPkt = new dKyWwSunlenz_Packet();
        }
        if (s_sunPkt != NULL && s_lenzPkt != NULL) {
            s_sunPkt->mpTextureData[0] = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexTukiA);
            s_sunPkt->mpTextureData[1] = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexTukiB);
            s_sunPkt->mpTextureData[2] = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexTukiC);
            s_sunPkt->mpTextureData[3] = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexTukiD);
            s_sunPkt->mpTextureData[4] = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexTaiyo);
            s_lenzPkt->mpTexSnow01 = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexSnow01);
            s_lenzPkt->mpTexLensHalf = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexLensHalf);
            s_lenzPkt->mpTexRingHalf = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexRingHalf);
            if (s_sunPkt->mpTextureData[0] == NULL || s_sunPkt->mpTextureData[4] == NULL ||
                s_lenzPkt->mpTexSnow01 == NULL) {
                static bool s_warned = false;
                if (!s_warned) {
                    s_warned = true;
                    DuskLog.warn("[WwSky] 413 WwAlways celestial BTIs not resident yet -- "
                                 "sun/moon deferred (retry next frame)");
                }
                return;  // arc not resident yet -- retry, do NOT latch (the 412 lesson)
            }
            s_sunPkt->field_0x3c = 0;
            s_sunPkt->field_0x3d = 0;
            s_sunPkt->mVisibility = 0.0f;
            s_sunPkt->mSunAlpha = 0.0f;
            s_sunPkt->mMoonAlpha = 0.0f;
            for (int i = 0; i < 5; i++) {
                s_sunPkt->mVizChkData[i] = 0;  // donor zeroes 0..3; [4] covered too
            }
            s_lenzPkt->field_0x88 = 1000000000.0f;
            s_lenzPkt->field_0x8c = 0.0f;
            s_lenzPkt->mDistFalloff = 0.0f;
            s_lenzPkt->mbDrawLenzInSky = false;

            wwSkySunMove();
            wwSkyLenzflareMove();
            s_sunInit = true;
            DuskLog.info("[WwSky] 413 WW sun/moon/lenz LIVE (WwAlways textures)");
        }
    } else {
        wwSkySunMove();
        wwSkyLenzflareMove();
        // §414-P6: sun state snapshot every ~10s -- discriminates time-window
        // vs occlusion vs alpha-feed failures.
        static u32 s_frames = 0;
        if ((++s_frames % 600) == 1) {
            DuskLog.info("[WwSky] 414-P6 sun: daytime={} sunAlpha={} moonAlpha={} viz={} "
                         "pos=({}, {}, {})",
                         g_env_light.getDaytime(), s_sunPkt->mSunAlpha, s_sunPkt->mMoonAlpha,
                         s_sunPkt->mVisibility, s_sunPkt->mPos[0].x, s_sunPkt->mPos[0].y,
                         s_sunPkt->mPos[0].z);
        }
    }
}

void dKyWwSky_moveStar() {
    if (!(dComIfGp_checkStatus(1))) {
        return;
    }
    f32 time = g_env_light.getDaytime();
    f32 target;
    // donor normal-stage windows (WW d_kankyo_wether.cpp:618-628)
    if (time >= 330.0f || time < 90.0f) {
        target = 1.0f;
    } else if (time > 300.0f) {
        target = 1.0f - (330.0f - time) * (1 / 30.0f);
    } else if (time < 105.0f) {
        target = (105.0f - time) * (1 / 15.0f);
    } else {
        target = 0.0f;
    }

    if (wwSkyWeatherKill()) {  // [S19]
        target = 0.0f;
    }

    cLib_addCalc(&s_starCountTarget, target, 0.1f, 0.01f, 0.000001f);
    s_starCount = (s16)(s_starCountTarget * 1000.0f);

    if (!s_starInit) {
        if (s_starCount != 0) {
            if (s_starPkt == NULL) {
                s_starPkt = new dKyWwStar_Packet();
            }
            if (s_starPkt == NULL) {
                return;
            }
            s_starPkt->mpTexture = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexSnow01);
            s_starPkt->mEffect[0].mSin = 1.0f;
            s_starPkt->mEffect[0].mAnimCounter = 0.0f;
            s_starPkt->mEffectNum = 0;
            s_starInit = true;
            DuskLog.info("[WwSky] 413 WW starfield LIVE");
        }
    }
    if (s_starInit) {
        // donor dKyr_star_move (d_kankyo_rain.cpp:1274-1283)
        s_starPkt->mEffectNum = (s16)s_starCount;
        if (s_starPkt->mEffectNum != 0) {
            f32 wave = fabsf(cM_fsin(s_starPkt->mEffect[0].mAnimCounter));
            s_starPkt->mEffect[0].mAnimCounter += 0.01f;
            s_starPkt->mEffect[0].mSin = wave;
            cLib_addCalc(&s_starPkt->mEffect[0].mSin, wave, 0.5f, 0.1f, 0.01f);
        }
        if (s_starCount == 0) {
            s_starPkt->mEffectNum = 0;
        }
        // §414-P7: star snapshot every ~10s.
        static u32 s_frames = 0;
        if ((++s_frames % 600) == 1) {
            DuskLog.info("[WwSky] 414-P7 star: daytime={} target={} count={}",
                         g_env_light.getDaytime(), s_starCountTarget, s_starCount);
        }
    }
}

// donor dKyw_setDrawPacketListSky idiom (WW d_kankyo_wether.cpp:27-37)
static void wwSkyEntrySky(J3DPacket* pkt) {
    if (pkt != NULL) {
        dComIfGd_setListSky();
        j3dSys.getDrawBuffer(1)->entryImm(pkt, 0);
        dComIfGd_setList();
    }
}

void dKyWwSky_drawSun() {
    if (!s_sunInit) {
        // §414-P8: draw dispatched but move never initialized.
        static bool s_once = false;
        if (!s_once) {
            s_once = true;
            DuskLog.warn("[WwSky] 414-P8 drawSun dispatched while sun NOT initialized "
                         "(move path dead?)");
        }
        return;
    }
    {
        static bool s_once = false;
        if (!s_once) {
            s_once = true;
            DuskLog.info("[WwSky] 414-P8 drawSun ENTRY OK -- packets entering sky list");
        }
    }
    wwSkyEntrySky(s_sunPkt);
    // donor: lenz draws in the FILTER list unless occluded-to-sky
    // (dKyw_drawSunlenz, WW d_kankyo_wether.cpp:171-178). Order after the sun
    // is load-bearing: drawSun writes mColor (fog) that the flare reads.
    if (s_lenzPkt != NULL) {
        if (!s_lenzPkt->mbDrawLenzInSky) {
            dComIfGd_getListFilter()->entryImm(s_lenzPkt, 0);
        } else {
            wwSkyEntrySky(s_lenzPkt);
        }
    }
}

void dKyWwSky_drawStar() {
    if (!s_starInit || s_starPkt == NULL || s_starPkt->mEffectNum == 0) {
        return;
    }
    wwSkyEntrySky(s_starPkt);
}

bool dKyWwSky_sunReady() { return s_sunInit; }
bool dKyWwSky_starReady() { return s_starInit; }

void dKyWwSky_reset() {
    // Packets are session-lived statics [S22]; reset re-arms init so the
    // resource latch re-runs against the NOW-resident WW arcs.
    s_sunInit = false;
    s_starInit = false;
    s_starCountTarget = 0.0f;
    s_starCount = 0;
}

