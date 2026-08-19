// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_rain.cpp NonMatching
// KIT-DONOR: d/d_kankyo.cpp MatchingFor
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// Â§413 WW CELESTIAL LAYER -- see header. Seams:
//  [S17] donor dStageType_MISC_e legs dropped: the sky host is outdoor by
//        definition (â„–108 outdoor-F_DL* only) -- the outdoor branch of each
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
//        (foam-proven access shape, Â§97b).
//  [S22] packets are lazy statics, reset via dKyWwSky_reset(); the donor
//        deletes them in wether_delete -- session-lived here, flag-gated.
//  [S25] donor snap_sunmoon_proc calls (d_kankyo_rain.cpp:1927 moon, :2026
//        sun -- pictobox photo-snapshot feed) are DROPPED: the receiver has
//        no pictobox subsystem, the feed has no consumer. Sites marked inline.
// ============================================================

#include "ww_sky.h"
#include "ww_kankyo.h"
#include "ww_vrkumo.h"

#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "SSystem/SComponent/c_angle.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_rain.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "f_op/f_op_camera_mng.h"
#include "m_Do/m_Do_lib.h"
#include "m_Do/m_Do_graphic.h"
#include <mods/api.h>
#include <mods/svc/log.h>
#include <cstdarg>
#include <cstdio>
#include <math.h>

extern const LogService* s_log;

static void skyLog(const char* fmt, ...) {
    if (s_log == nullptr) {
        return;
    }
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, LOG_LEVEL_INFO, buf);
}

static bool wwSkyHostLive() {
    if (wwKankyo_paletN() <= 0) {
        return false;
    }
    int invisible = 1;
    wwKankyo_vrboxGet(nullptr, nullptr, nullptr, nullptr, &invisible);
    return invisible == 0;
}

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
// donor dKy_set_eyevect_calc (WW d_kankyo_rain.cpp:54-60) -- the receiver's
// same-lineage twin exists but is file-static (src/d/d_kankyo_rain.cpp), so
// this TU carries its own copy, same pattern as the btitex helpers. NOT calc2:
// the -200 y offset is the donor difference.
static void wwSkyEyevectCalc(camera_class* i_camera, Vec* o_out, f32 param_2, f32 param_3) {
    cXyz calc;
    dKyr_get_vectle_calc(&i_camera->view.lookat.eye, &i_camera->view.lookat.center, &calc);
    o_out->x = i_camera->view.lookat.eye.x + calc.x * param_2;
    o_out->y = (i_camera->view.lookat.eye.y + calc.y * param_3) - 200.0f;
    o_out->z = i_camera->view.lookat.eye.z + calc.z * param_2;
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
    virtual int entry(J3DDrawBuffer*) { return 1; }
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
    virtual int entry(J3DDrawBuffer*) { return 1; }
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
    virtual int entry(J3DDrawBuffer*) { return 1; }
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

static MtxP wwSkyDrawViewMtx() {
    // Prefer the drawlist camera view — j3dSys view can be stale after dome
    // UpdateDL (calc/entry) in the same OPA flush.
    MtxP view = dComIfGd_getViewMtx();
    if (view == nullptr) {
        view = j3dSys.getViewMtx();
    }
    return view;
}

void dKyWwSun_Packet::draw() {
    // Â§414-P9: J3D really invokes the packet (list plumbing proof).
    static bool s_once = false;
    if (!s_once) {
        s_once = true;
        skyLog("[WwSky] {\"ev\":\"sun_draw_pkt\"}");
    }
    wwSkyDrawSunBody(wwSkyDrawViewMtx(), mPos, mColor, mpTextureData);
}
void dKyWwSunlenz_Packet::draw() {
    {
        static u32 s_drawN = 0;
        if ((++s_drawN % 120) == 1 && s_sunPkt != nullptr) {
            skyLog("[WwSky] {\"ev\":\"lenz_draw\",\"n\":%u,\"vis\":%.3f,\"lenzSky\":%d,"
                   "\"col\":\"%02X%02X%02X%02X\",\"fall\":%.3f}",
                   s_drawN, s_sunPkt->mVisibility, mbDrawLenzInSky ? 1 : 0,
                   s_sunPkt->mColor.r, s_sunPkt->mColor.g, s_sunPkt->mColor.b,
                   s_sunPkt->mColor.a, mDistFalloff);
        }
    }
    wwSkyDrawLenzflareBody(wwSkyDrawViewMtx(), mPositions, &mpTexSnow01);
}
void dKyWwStar_Packet::draw() { wwSkyDrawStarBody(wwSkyDrawViewMtx()); }

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
    f32 curTime = g_env_light.daytime;
    if (curTime > 15.0f) {
        var_f1 = curTime - 15.0f;
    } else {
        var_f1 = curTime + 345.0f;
    }
    sp8.x = sinf(DEG2RAD(var_f1)) * 80000.0f;
    sp8.y = cosf(DEG2RAD(var_f1)) * 80000.0f;
    sp8.z = cosf(DEG2RAD(var_f1)) * -48000.0f;
    // donor :597 escape: during the stage-entry light init anim the sun still
    // tracks even inside an event. Receiver twin of donor mInitAnimTimer =
    // light_init_timer (d_kankyo.h:469, same 1..20-then-0 mechanism).
    if (dComIfGp_event_runCheck() == FALSE || g_env_light.light_init_timer != 0) {
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
    f32 t = g_env_light.daytime;
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

    f32 curTime = g_env_light.daytime;
    if (curTime > 97.5f && curTime < 292.5f) {
        f32 borderY = 0.0f;  // [S18]
        s32 numPointsCulled = 0;

        cLib_addCalc(&pSunPkt->mSunAlpha, 1.0f, 0.5f, 0.1f, 0.01f);

        cXyz projected;
        mDoLib_project(pSunPkt->mPos, &projected);

        static const f32 sun_chkpnt[5][2] = {
            {0.0f, 0.0f}, {-10.0f, -20.0f}, {10.0f, 20.0f}, {-20.0f, 10.0f}, {20.0f, -10.0f},
        };

        // mDoLib_project on PC uses getWidthF/HeightF (widescreen). Donor
        // 640×490 checks miss the projected sun → nVis=0 → mVisibility starved
        // → no sunburst needles / hex trail. Same FB space as the projector.
        const f32 scrW = mDoGph_gInf_c::getWidthF();
        const f32 scrH = mDoGph_gInf_c::getHeightF();
        const f32 scrX0 = mDoGph_gInf_c::getMinXF();
        const f32 scrY0 = 0.0f;

        for (s32 i = 0; i < 5; i++) {
            f32 screenBottom = scrH - borderY;
            cXyz chkpnt = projected;
            chkpnt.x -= sun_chkpnt[i][0];
            chkpnt.y -= sun_chkpnt[i][1];

            if (chkpnt.x > scrX0 && chkpnt.x < scrX0 + scrW && chkpnt.y > borderY &&
                chkpnt.y < screenBottom) {
                if (pSunPkt->mVizChkData[i] >= 0xFFFFFF) {
                    numPointsVisible++;
                    if (i == 0)
                        numCenterPointsVisible++;
                }
                // dDlst_peekZ_c::newData silently drops x>607 || y>447 (GC EFB).
                // Clamp into that window so Aurora actually queues the sample.
                s16 px = (s16)chkpnt.x;
                s16 py = (s16)chkpnt.y;
                if (px < 0) {
                    px = 0;
                } else if (px > 607) {
                    px = 607;
                }
                if (py < 0) {
                    py = 0;
                } else if (py > 447) {
                    py = 447;
                }
                const u32 prevZ = pSunPkt->mVizChkData[i];
                dComIfGd_peekZ(px, py, &pSunPkt->mVizChkData[i]);
                // Aurora GXPeekZ writes 0 when depth_peek::read_latest fails
                // (snapshot not ready / OOB). Donor GC never does this; the 0
                // poisons the next frame's >=0xFFFFFF visibility test and
                // starves sunburst. Consume-boundary: keep the prior sample
                // when the read returns the failure sentinel.
                if (pSunPkt->mVizChkData[i] == 0 && prevZ != 0) {
                    pSunPkt->mVizChkData[i] = prevZ;
                }
            } else {
                numPointsCulled++;
            }
        }

        // Â§421-P31: raw peek-Z evidence -- all-zero forever = backend never
        // returns data for this render path; nonzero-but-low at open sky =
        // coordinate/scale mismatch or something writing depth under the sun.
        {
            static u32 s_f31 = 0;
            if ((++s_f31 % 120) == 1) {
                skyLog("[WwSky] {\"ev\":\"peekZ\",\"proj\":[%.1f,%.1f],"
                       "\"viz\":[%u,%u,%u,%u,%u],\"nVis\":%d,\"nCulled\":%d,"
                       "\"vis\":%.3f,\"lenzSky\":%d}",
                       projected.x, projected.y, pSunPkt->mVizChkData[0],
                       pSunPkt->mVizChkData[1], pSunPkt->mVizChkData[2],
                       pSunPkt->mVizChkData[3], pSunPkt->mVizChkData[4],
                       (int)numPointsVisible, (int)numPointsCulled,
                       pSunPkt->mVisibility, pLenzPkt->mbDrawLenzInSky ? 1 : 0);
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
        center.x = scrX0 + 0.5f * scrW;
        center.y = scrY0 + 0.5f * scrH;
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

    // ========================================================================
    // Â§686 PC OCCLUSION HYSTERESIS â€” the flicker root (agent sweep, hypothesis
    // 11). The donor's staring-at-the-sun system scales EVERY vrbox channel
    // through dKy_set_vrboxcol_ratio, and its gate is this lens-occlusion
    // verdict. On GC the point-visibility count is frame-stable; on this PC
    // renderer it flips frame to frame, so the ratio request snapped between
    // 1.0 (reset each execute) and ~1.5 (staring), and the post-scale clamp
    // at 255 turned the swing into HUE changes â€” the wild sky flicker, keyed
    // to camera aim with the donor's quartic response. The donor system stays
    // verbatim; only its unstable PC input gets a latch: the verdict must
    // agree for 10 consecutive frames before the gate flips. Labeled
    // PC-platform translation, not a donor edit.
    // ========================================================================
    {
        static bool s_lenzInSky = true;
        static int s_lenzFlipCount = 0;
        const bool rawLenzInSky = numPointsVisible < 2;
        if (rawLenzInSky == s_lenzInSky) {
            s_lenzFlipCount = 0;
        } else if (++s_lenzFlipCount >= 10) {
            s_lenzInSky = rawLenzInSky;
            s_lenzFlipCount = 0;
        }
        pLenzPkt->mbDrawLenzInSky = s_lenzInSky;
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

    wwSkyEyevectCalc(pCamera, &eyeVect, 7200.0005f, 7200.0005f);  // donor :485 = calc, not calc2

    dKyr_get_vectle_calc(&eyeVect, pSunPkt->mPos, &sunDirSmth);
    pLenzPkt->mPositions[0] = pSunPkt->mPos[0];
    pLenzPkt->mPositions[1] = pSunPkt->mPos[0];

    mDoLib_project(pLenzPkt->mPositions, &projected);

    // Donor used 320×240; match peekZ / DistFalloff to the same FB space.
    center.x = mDoGph_gInf_c::getMinXF() + 0.5f * mDoGph_gInf_c::getWidthF();
    center.y = 0.5f * mDoGph_gInf_c::getHeightF();
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
        if (g_env_light.daytime < 180.0f) {
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
        // Dome OPA wrote depth; XLU sky draws after — Z off like vrbox2 bands.
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetClipMode(GX_CLIP_DISABLE);
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
            // [S25] donor :1927 snap_sunmoon_proc(&moonPos2, texidx) dropped (no pictobox)
        }

        if (bDrawSun == true) {
            cXyz camfwd;
            // [S25] donor :2026 snap_sunmoon_proc(&sunPos, 9) dropped (no pictobox).
            // Donor's unused sun_distXZ/sun_phi/cam_distXZ/cam_phi dead locals
            // (:2028-2035) are dropped too -- the rotation below never reads them.

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
        // donor :2305 sets channel GX_COLOR0 (not COLOR0A0)
        GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
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
        GXSetClipMode(GX_CLIP_ENABLE);  // donor :2316
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
// donor dKyr_drawStar (WW d_kankyo_rain.cpp:3064-3272): 16 fixed hokuto
// constellation stars + procedural spiral field above the eye, pure vertex
// color, two tris per star (hexagram).
//
// REF (noclip/Jasper): spiral + hardcoded constellation + “vector polygons” —
// behavior read only, not law.
// LAW: donor body / retail FIFO. GZLE01 main.dol: two GXBegin sites are
// li r3,0x80 + li r5,3 (GX_QUADS×3) @ 80099ae4 / 80099bd4; size floats
// 190/290/0.066/0.28 live in .data. Decomp Nonmatching matched that opcode.
// Aurora streaks on incomplete quads → consume-boundary GX_TRIANGLES×3×2
// (same 3 verts / hexagram). REF only: noclip/Jasper “vector polygons”.
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
        // GZLE01 star_col[0] @ 80352F3C
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
        // donor :3121 sets channel GX_COLOR0 (not COLOR0A0)
        GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
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
        // Donor decomp (Nonmatching) wrote GX_ENABLE/LEQUAL/DISABLE. Plugin had
        // Z off; restore donor so stars sit behind Z-writing sky geometry.
        GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
        GXSetNumIndStages(0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetCullMode(GX_CULL_NONE);
        GXSetColorUpdate(GX_TRUE);
        GXSetClipMode(GX_CLIP_DISABLE);

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

        // GZLE01 drawStar @ 8009967c..800996b0 (law — Nonmatching was wrong):
        //   f31 = 1 - clamp(fovy/40, 1)
        //   temp_f27 = 0.9 - 0.6*f31   (= 0.3 + 0.6*clamp(fovy/40); ≥0.9 at FOV≥40)
        // Decomp had 0.28*(…) and /45 — ~3× too small vs retail .sdata2 0.9/0.6/40.
        f32 var_f30 = 0.0f;
        if (dComIfGd_getView() != NULL) {
            var_f30 = dComIfGd_getView()->fovy / 40.0f;
            if (var_f30 >= 1.0f) {
                var_f30 = 1.0f;
            }
            var_f30 = 1.0f - var_f30;
        }
        const f32 temp_f27 = 0.9f - 0.6f * var_f30;

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

            // GZLE01 .data @ 80352F3C — four RGBA8 colours (decomp Nonmatching
            // pink/green table was invented; those bytes are absent from retail).
            static const GXColor star_col[] = {
                {0xDC, 0xE6, 0xFF, 0xFF},  // default field (soft blue-white)
                {0xFF, 0xC8, 0xC8, 0xFF},  // i==6 || i==8
                {0xFF, 0xFF, 0xC8, 0xFF},  // indexed
                {0xC8, 0xC8, 0xFF, 0xFF},  // indexed
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
                // Two opposing triangles = hexagram star (not textured quads).
                GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
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

                GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
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
// Public move/draw -- donor wether_move_sun / wether_move_star with WwAlways
// textures. Gate is plugin vrbox visibility, NOT checkStatus(1)/hide_vrbox.
// ---------------------------------------------------------------------------
static request_of_phase_process_class s_alwaysPhase;
static int s_alwaysPhaseState = cPhs_INIT_e;
static bool s_alwaysBootOpen = false;

static void wwSkyPollAlways() {
    // Plugin must resLoad WwAlways (WW Always is boot-resident under "Always";
    // ours is the §806 disc twin). Refuse until boot window opens — logo
    // phase_2 has no Always yet and must not start WwAlways (113201).
    if (!s_alwaysBootOpen) {
        return;
    }
    if (s_alwaysPhaseState != cPhs_COMPLEATE_e && s_alwaysPhaseState != cPhs_ERROR_e) {
        s_alwaysPhaseState = dComIfG_resLoad(&s_alwaysPhase, "WwAlways");
        if (s_alwaysPhaseState == cPhs_COMPLEATE_e) {
            skyLog("[WwSky] {\"ev\":\"always_resident\"}");
        } else if (s_alwaysPhaseState == cPhs_ERROR_e) {
            skyLog("[WwSky] {\"ev\":\"always_error\"}");
        }
    }
}

void wwSky_openAlwaysBoot() {
    if (!s_alwaysBootOpen) {
        s_alwaysBootOpen = true;
        skyLog("[WwSky] {\"ev\":\"always_boot_window\","
               "\"reads\":\"outdoor sky host live; mount WwAlways after room BG heap\"}");
    }
}

bool wwSky_bootWindowOpen() {
    return s_alwaysBootOpen;
}

void wwSky_pollAlways() {
    wwSkyPollAlways();
}

static void wwSkyMoveSun() {
    wwSkySetSunpos();
    if (!wwSkyHostLive()) {
        return;
    }
    if (s_alwaysPhaseState != cPhs_COMPLEATE_e) {
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
                    skyLog("[WwSky] {\"ev\":\"celestial_bti_wait\"}");
                }
                return;
            }
            s_sunPkt->field_0x3c = 0;
            s_sunPkt->field_0x3d = 0;
            s_sunPkt->mVisibility = 0.0f;
            s_sunPkt->mSunAlpha = 0.0f;
            s_sunPkt->mMoonAlpha = 0.0f;
            s_sunPkt->mColor.r = 0xFF;
            s_sunPkt->mColor.g = 0xFF;
            s_sunPkt->mColor.b = 0xF1;
            s_sunPkt->mColor.a = 0xFF;
            // Donor zeroes these; on PC the first successful snapshot can lag
            // (30 Hz depth_peek). Seed open-sky so the one-frame-delayed
            // visibility test is not stuck on Aurora's failed-read 0.
            for (int i = 0; i < 5; i++) {
                s_sunPkt->mVizChkData[i] = 0xFFFFFF;
            }
            s_lenzPkt->field_0x88 = 1000000000.0f;
            s_lenzPkt->field_0x8c = 0.0f;
            s_lenzPkt->mDistFalloff = 0.0f;
            s_lenzPkt->mbDrawLenzInSky = false;

            wwSkySunMove();
            wwSkyLenzflareMove();
            s_sunInit = true;
            skyLog("[WwSky] {\"ev\":\"sun_live\"}");
        }
    } else {
        wwSkySunMove();
        wwSkyLenzflareMove();
        static u32 s_frames = 0;
        if ((++s_frames % 600) == 1) {
            skyLog("[WwSky] {\"ev\":\"sun\",\"day\":%.1f,\"sunA\":%.3f,\"moonA\":%.3f,"
                   "\"viz\":%.3f,\"pos\":[%.1f,%.1f,%.1f]}",
                   g_env_light.daytime, s_sunPkt->mSunAlpha, s_sunPkt->mMoonAlpha,
                   s_sunPkt->mVisibility, s_sunPkt->mPos[0].x, s_sunPkt->mPos[0].y,
                   s_sunPkt->mPos[0].z);
        }
    }
}

static void wwSkyMoveStar() {
    if (!wwSkyHostLive()) {
        return;
    }
    f32 time = g_env_light.daytime;
    f32 target;
    if (time >= 330.0f || time < 90.0f) {
        target = 1.0f;
    } else if (time > 300.0f) {
        target = 1.0f - (330.0f - time) * (1 / 30.0f);
    } else if (time < 105.0f) {
        target = (105.0f - time) * (1 / 15.0f);
    } else {
        target = 0.0f;
    }

    if (wwSkyWeatherKill()) {
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
            skyLog("[WwSky] {\"ev\":\"star_live\"}");
        }
    }
    if (s_starInit) {
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
        static u32 s_frames = 0;
        if ((++s_frames % 600) == 1) {
            skyLog("[WwSky] {\"ev\":\"star\",\"day\":%.1f,\"target\":%.3f,\"count\":%d}",
                   g_env_light.daytime, s_starCountTarget, s_starCount);
        }
    }
}

static void wwSkyEntryBuf(void* drawBuf, J3DPacket* pkt) {
    (void)drawBuf;
    // Donor dKyw_setDrawPacketListSky: XLU sky (getDrawBuffer(1)).
    // Immediate draw in OPA PRE ran *before* dome UpdateDL packets flushed,
    // so the dome painted over sun/vrkumo. Queue for drawXluListSky instead.
    if (pkt == nullptr) {
        return;
    }
    J3DDrawBuffer* xlu = j3dSys.getDrawBuffer(1);
    if (xlu != nullptr) {
        xlu->entryImm(pkt, 0);
    } else {
        pkt->draw();
    }
}

void wwSky_move() {
    // Donor: Always already resident before rooms. Plugin: TP Always + full
    // disc WwAlways at logo starves sea Akabe solid heaps (113931). Open the
    // WwAlways window at first outdoor host-live — consume moment for sun/foam
    // — after room BG create has taken its heap.
    if (!s_alwaysBootOpen && wwSkyHostLive()) {
        wwSky_openAlwaysBoot();
    }
    wwSkyPollAlways();
    wwSkyMoveSun();
    wwSkyMoveStar();
    wwVrkumo_move();
}

void wwSky_drawInto(void* drawBuf) {
    // Cumulus under sun/stars — donor draw2 is a separate sky list; flush order
    // here is bands (already drawn) → vrkumo → celestial.
    wwVrkumo_drawInto(drawBuf);
    if (!s_sunInit) {
        return;
    }
    static bool s_once = false;
    if (!s_once) {
        s_once = true;
        skyLog("[WwSky] {\"ev\":\"sun_draw_ok\"}");
    }
    if (s_sunPkt != nullptr) {
        static int s_sunSnap = 0;
        s_sunSnap++;
        if (s_sunSnap <= 3 || (s_sunSnap % 300) == 0) {
            skyLog("[WwSky] {\"ev\":\"sun_flush\",\"n\":%d,\"day\":%.1f,\"sunA\":%.3f,"
                   "\"moonA\":%.3f,\"vis\":%.3f,\"path\":\"xlu_entryImm\"}",
                   s_sunSnap, g_env_light.daytime, s_sunPkt->mSunAlpha, s_sunPkt->mMoonAlpha,
                   s_sunPkt->mVisibility);
        }
    }
    wwSkyEntryBuf(drawBuf, s_sunPkt);
    if (s_lenzPkt != nullptr) {
        // Donor dKyw_drawSunlenz (WWDP d_kankyo_wether.cpp:171): !mbDrawLenzInSky
        // → Filter list (setDrawPacketList). Called from wether draw with the
        // current list already Filter. Plugin queues from vrbox drawInto — must
        // setListFilter before entryImm or the packet never lands on a flushed
        // buffer (sunburst residual with vis>0 but no needles).
        if (!s_lenzPkt->mbDrawLenzInSky) {
            dComIfGd_setListFilter();
            J3DDrawBuffer* filter = dComIfGd_getListFilter();
            if (filter != nullptr) {
                filter->entryImm(s_lenzPkt, 0);
            } else {
                wwSkyEntryBuf(drawBuf, s_lenzPkt);
            }
            dComIfGd_setList();
        } else {
            wwSkyEntryBuf(drawBuf, s_lenzPkt);
        }
        {
            static u32 s_lenzPath = 0;
            if ((++s_lenzPath % 120) == 1) {
                skyLog("[WwSky] {\"ev\":\"lenz_queue\",\"lenzSky\":%d,\"vis\":%.3f,"
                       "\"path\":\"%s\",\"fall\":%.3f,\"filter\":%d}",
                       s_lenzPkt->mbDrawLenzInSky ? 1 : 0, s_sunPkt->mVisibility,
                       s_lenzPkt->mbDrawLenzInSky ? "xlu_sky" : "filter",
                       s_lenzPkt->mDistFalloff,
                       dComIfGd_getListFilter() != nullptr ? 1 : 0);
            }
        }
    }
    if (s_starInit && s_starPkt != nullptr && s_starPkt->mEffectNum != 0) {
        wwSkyEntryBuf(drawBuf, s_starPkt);
    }
}

void wwSky_reset() {
    // Keep WwAlways mounted — donor Always is permanent across stages.
    // Deleting here forced sea re-expand (113201 always_resident ×3 → Akabe crash).
    s_sunInit = false;
    s_starInit = false;
    s_starCountTarget = 0.0f;
    s_starCount = 0;
    wwVrkumo_reset();
}

bool wwSky_alwaysReady() {
    return s_alwaysPhaseState == cPhs_COMPLEATE_e;
}
