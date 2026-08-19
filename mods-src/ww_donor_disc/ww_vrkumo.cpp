// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_wether.cpp MatchingFor wether_move_vrkumo;
//            d/d_kankyo_rain.cpp NonMatching stubs → History decode
//            (src/d/d_kankyo_ww_vrkumo.inc) + Housing draw body (§417c/§420)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// The soft vr_back_cloud dome bands are NOT the iconic Outset cumulus.
// This TU is the 50-cloud billboard packet the player expects to see.

#include "ww_vrkumo.h"

#include "ww_kankyo.h"
#include "ww_kankyo_wind.h"

#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_xyz.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_rain.h"
#include "d/d_stage.h"
#include "f_op/f_op_camera_mng.h"
#include "m_Do/m_Do_mtx.h"
#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <mods/api.h>
#include <mods/svc/log.h>
#include <cstdarg>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <math.h>

extern const LogService* s_log;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

void vrkumoLog(const char* fmt, ...) {
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

bool hostLive() {
    if (wwKankyo_paletN() <= 0) {
        return false;
    }
    int invisible = 1;
    wwKankyo_vrboxGet(nullptr, nullptr, nullptr, nullptr, &invisible);
    return invisible == 0;
}

inline f32 cubef(f32 v) {
    return v * v * v;
}

// Same fields as vanilla VRKUMO_EFF; plugin-owned (not g_env_light packet).
struct WwVrkumoEff {
    s8 mStatus;
    cXyz mPosition;
    cXyz mBasePos;
    f32 mHeight;
    f32 mAlpha;
    f32 mDistFalloff;
    f32 mSpeed;
};

class WwVrkumo_Packet : public J3DPacket {
public:
    virtual int entry(J3DDrawBuffer*) {
        return 1;
    }
    virtual void draw();
    virtual ~WwVrkumo_Packet() {}

    u8* mpTx[3];
    WwVrkumoEff mEff[100];
};

WwVrkumo_Packet* s_pkt = nullptr;
bool s_init = false;
int s_count = 0;
f32 s_strength = 0.0f;
f32 s_bounce = 0.0f;

void initEff(WwVrkumoEff* kumo) {
    const f32 angle = cM_rndF(2.0f * (f32)M_PI);
    f32 dist = cM_rndF(18000.0f);
    if (dist > 15000.0f) {
        dist = 14000.0f + cM_rndF(1000.0f);
    }
    kumo->mPosition.x = dist * sinf(angle);
    kumo->mPosition.z = dist * cosf(angle);
    kumo->mPosition.y = 0.0f;
    kumo->mAlpha = 0.0f;
    kumo->mSpeed = 0.5f + cM_rndF(4.0f);
    kumo->mHeight = 0.3f * cM_rndFX(0.3f);
    kumo->mDistFalloff = 1.0f;
}

void moveEffs() {
    if (s_pkt == nullptr) {
        return;
    }
    cXyz wind = dKyWw_get_wind_vecpow();
    f32 skyboxOffsY;
    if (std::strcmp(dComIfGp_getStartStageName(), "M_DragB") == 0) {
        wind.set(-1.0f, 0.0f, 0.0f);
        skyboxOffsY = 300.0f;
    } else {
        skyboxOffsY = 1000.0f + s_strength * -500.0f;
    }

    for (int i = 0; i < 100; i++) {
        WwVrkumoEff* kumo = &s_pkt->mEff[i];
        if (kumo->mStatus == 0) {
            initEff(kumo);
            kumo->mStatus = 1;
        }

        f32 distXZ = sqrtf(kumo->mPosition.x * kumo->mPosition.x +
                           kumo->mPosition.z * kumo->mPosition.z);
        if (distXZ > 15000.0f) {
            if (distXZ <= 15100.0f) {
                kumo->mPosition.x *= -1.0f;
                kumo->mPosition.z *= -1.0f;
            } else {
                kumo->mPosition.x = cM_rndFX(14000.0f);
                kumo->mPosition.z = cM_rndFX(14000.0f);
                distXZ = sqrtf(kumo->mPosition.x * kumo->mPosition.x +
                               kumo->mPosition.z * kumo->mPosition.z);
            }
            kumo->mAlpha = 0.0f;
        }

        const f32 strengthVelocity = 4.0f + s_strength * 4.3f;
        f32 velocity;
        if (kumo->mAlpha > 0.0f) {
            velocity = strengthVelocity * kumo->mDistFalloff * kumo->mSpeed;
        } else {
            velocity = strengthVelocity + (i / 1000.0f) * strengthVelocity;
        }
        kumo->mPosition.x += wind.x * velocity;
        kumo->mPosition.y += wind.y * velocity;
        kumo->mPosition.z += wind.z * velocity;

        const f32 distXZ01 = distXZ / 15000.0f > 1.0f ? 1.0f : distXZ / 15000.0f;
        const f32 strengthY = 3000.0f + s_strength * -1000.0f;
        const f32 centerAmtCubic = 1.0f - (distXZ01 * distXZ01 * distXZ01);
        kumo->mPosition.y = (500.0f * (i / 100.0f)) + skyboxOffsY + strengthY * centerAmtCubic;

        const f32 d3 = distXZ01 * distXZ01 * distXZ01;
        kumo->mDistFalloff = 1.0f - d3 * d3;

        f32 alphaTarget;
        f32 alphaMaxVel = 1.0f;
        if (std::strcmp(dComIfGp_getStartStageName(), "M_DragB") == 0) {
            kumo->mAlpha = 1.0f;
            alphaTarget = 1.0f;
        } else if (i < s_count) {
            alphaMaxVel = 0.1f;
            if (kumo->mDistFalloff >= 0.05f && kumo->mDistFalloff < 0.2f) {
                alphaTarget = (kumo->mDistFalloff - 0.05f) / 0.15f;
            } else if (kumo->mDistFalloff < 0.2f) {
                alphaTarget = 0.0f;
            } else {
                alphaTarget = 1.0f + s_strength * -0.55f;
            }
        } else {
            alphaTarget = 0.0f;
            alphaMaxVel = 0.005f;
        }

        f32 overheadFade = (0.98f - centerAmtCubic) / (0.98f - 0.88f);
        if (overheadFade < 0.0f) {
            overheadFade = 0.0f;
        }
        if (overheadFade > 1.0f) {
            overheadFade = 1.0f;
        }
        alphaTarget *= overheadFade;
        cLib_addCalc(&kumo->mAlpha, alphaTarget, 0.2f, alphaMaxVel, 0.01f);
    }
    s_bounce += 200.0f;
}

void drawLayerQuad(const WwVrkumoEff* kumo, int i, int textureIdx, f32 strength, f32 domeRadius,
                   const cXyz& camPos, GXColor clr) {
    const f32 size = kumo->mDistFalloff * (1.0f - cubef(((textureIdx + i) & 0x0F) / 16.0f)) *
                     (0.45f + strength * 0.55f);
    const f32 bounceAnim = sinf(textureIdx + 0.0001f * s_bounce);
    const f32 sizeAnim = size + (0.06f * size) * bounceAnim * kumo->mDistFalloff;
    const f32 height = sizeAnim + sizeAnim * kumo->mHeight;
    const f32 m0 = 0.15f * sizeAnim;
    const f32 m1 = 0.65f * sizeAnim;

    f32 polarOffs = 0.0f;
    f32 azimuthalOffs = 0.0f;
    if (textureIdx != 0) {
        switch (i & 3) {
        case 0:
            if (textureIdx == 2) {
                polarOffs = m1;
                azimuthalOffs = m0;
            }
            break;
        case 1:
            if (textureIdx == 1) {
                polarOffs = -m0;
                azimuthalOffs = m0;
            } else if (textureIdx == 2) {
                polarOffs = -m1;
                azimuthalOffs = m1;
            }
            break;
        case 2:
            if (textureIdx == 1) {
                polarOffs = m1;
                azimuthalOffs = -m1;
            } else if (textureIdx == 2) {
                polarOffs = m0;
                azimuthalOffs = -m1;
            }
            break;
        case 3:
            if (textureIdx == 1) {
                polarOffs = -m1;
            } else if (textureIdx == 2) {
                polarOffs = -m0;
                azimuthalOffs = m0;
            }
            break;
        }
    }

    const f32 distXZ =
        sqrtf(kumo->mPosition.x * kumo->mPosition.x + kumo->mPosition.z * kumo->mPosition.z);
    const f32 polarY1 = atan2f(kumo->mPosition.y, distXZ) + polarOffs;

    f32 np = polarY1 / 1.9f;
    if (np > 1.0f) {
        np = 1.0f;
    }
    const f32 normalPitch = np * np * np;
    const f32 azimuthal = atan2f(kumo->mPosition.x, kumo->mPosition.z) + azimuthalOffs;
    const f32 azimuthalOffsY0 = 0.6f * sizeAnim * (1.0f + 16.0f * normalPitch);
    const f32 azimuthalOffsY1 = 0.6f * sizeAnim * (1.0f + 2.0f * normalPitch);

    f32 polarY0 = polarY1 + 0.9f * height * (1.0f + -4.0f * normalPitch);
    if (polarY0 > 1.21f) {
        polarY0 = 1.21f;
    }

    struct {
        f32 polar;
        f32 azi;
        s16 s;
        s16 t;
    } corners[4] = {
        {polarY0, azimuthal + azimuthalOffsY0, 0, 0},
        {polarY0, azimuthal - azimuthalOffsY0, 1, 0},
        {polarY1, azimuthal - azimuthalOffsY1, 1, 1},
        {polarY1, azimuthal + azimuthalOffsY1, 0, 1},
    };
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (int c = 0; c < 4; c++) {
        const f32 x = cosf(corners[c].polar) * sinf(corners[c].azi);
        const f32 y = sinf(corners[c].polar);
        const f32 z = cosf(corners[c].polar) * cosf(corners[c].azi);
        GXPosition3f32(camPos.x + x * domeRadius, camPos.y + y * domeRadius,
                       camPos.z + z * domeRadius);
        GXColor4u8(clr.r, clr.g, clr.b, clr.a);
        GXTexCoord2s16(corners[c].s, corners[c].t);
    }
    GXEnd();
}

void drawBody(Mtx drawMtx) {
    camera_class* camera = (camera_class*)dComIfGp_getCamera(0);
    if (s_pkt == nullptr || camera == nullptr || dComIfGd_getView() == nullptr) {
        return;
    }
    if (s_pkt->mpTx[0] == nullptr || s_pkt->mpTx[1] == nullptr || s_pkt->mpTx[2] == nullptr) {
        return;
    }

    MtxP view = dComIfGd_getViewMtx();
    if (view == nullptr) {
        view = drawMtx;
    }
    // §420: load view PNMTX or quads ride the previous sky packet's leftover.
    GXLoadPosMtxImm(view, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    // [D3] / Housing rain.cpp: domeRadius = view->far_ - 10000.
    // Far must be STAG→camera (d_camera fopCamM_SetFar). Disc sniff + apply
    // feeds getStagInfo; refuse TP leftover / refuse inventing 80000.
    dKyWw_applyDiscStagFarToCamera();
    view_class* vw = dComIfGd_getView();
    f32 farz = (vw != nullptr) ? vw->far_ : 0.0f;
    const f32 disc = dKyWw_discStagFar();
    if (disc >= 20000.0f && disc <= 250000.0f) {
        farz = disc;
    }
    if (farz < 20000.0f || farz > 250000.0f) {
        static int s_badFar = 0;
        if (s_badFar < 4) {
            s_badFar++;
            vrkumoLog("[WwSky] {\"ev\":\"vrkumo_far_refuse\",\"disc\":%.0f,\"view\":%.0f}",
                      disc, vw != nullptr ? vw->far_ : -1.0f);
        }
        return;
    }
    const f32 domeRadius = farz - 10000.0f;
    const cXyz camPos = camera->view.lookat.eye;
    unsigned char horizon[4], center[4];
    wwKankyo_vrkumoColorsGet(horizon, center);

    GXColor black = {0, 0, 0, 255};
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG1, black);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C1, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    // Donor Housing §417c / rain.cpp: Z compare on, write off (XLU after OPA).
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetClipMode(GX_CLIP_DISABLE);
    GXSetZCompLoc(GX_TRUE);
    GXSetNumIndStages(0);
    GXSetCullMode(GX_CULL_NONE);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    int nVis = 0;
    for (int textureIdx = 2; textureIdx >= 0; textureIdx--) {
        ResTIMG* img = reinterpret_cast<ResTIMG*>(s_pkt->mpTx[textureIdx]);
        GXTexObj obj;
        GXInitTexObj(&obj, ((u8*)&img->format) + img->imageOffset, img->width, img->height,
                     (GXTexFmt)img->format, (GXTexWrapMode)img->wrapS, (GXTexWrapMode)img->wrapT,
                     (GXBool)(img->mipmapCount > 1));
        GXInitTexObjLOD(&obj, (GXTexFilter)img->minFilter, (GXTexFilter)img->magFilter,
                        img->minLOD * 0.125f, img->maxLOD * 0.125f, img->LODBias * 0.01f,
                        (GXBool)img->biasClamp, (GXBool)img->doEdgeLOD,
                        (GXAnisotropy)img->maxAnisotropy);
        GXLoadTexObj(&obj, GX_TEXMAP0);

        for (int i = 0; i < 100; i++) {
            const WwVrkumoEff* kumo = &s_pkt->mEff[i];
            if (kumo->mAlpha <= 0.000001f) {
                continue;
            }
            if (textureIdx == 2) {
                nVis++;
            }
            GXColor clr;
            const f32 fall = kumo->mDistFalloff;
            clr.r = (u8)(center[0] + (1.0f - fall) * (horizon[0] - center[0]));
            clr.g = (u8)(center[1] + (1.0f - fall) * (horizon[1] - center[1]));
            clr.b = (u8)(center[2] + (1.0f - fall) * (horizon[2] - center[2]));
            clr.a = (u8)(kumo->mAlpha * 255.0f);
            drawLayerQuad(kumo, i, textureIdx, s_strength, domeRadius, camPos, clr);
        }
    }
    J3DShape::resetVcdVatCache();

    static int s_draws = 0;
    s_draws++;
    if (s_draws <= 4 || (s_draws % 300) == 0) {
        vrkumoLog("[WwSky] {\"ev\":\"vrkumo_draw\",\"n\":%d,\"vis\":%d,\"count\":%d,"
                  "\"str\":%.3f,\"domeR\":%.0f,\"viewFar\":%.0f,\"hz\":[%u,%u,%u],"
                  "\"ct\":[%u,%u,%u]}",
                  s_draws, nVis, s_count, s_strength, domeRadius, farz, horizon[0], horizon[1],
                  horizon[2], center[0], center[1], center[2]);
    }
}

}  // namespace

void WwVrkumo_Packet::draw() {
    MtxP view = dComIfGd_getViewMtx();
    if (view == nullptr) {
        view = j3dSys.getViewMtx();
    }
    drawBody(view);
}

void wwVrkumo_reset() {
    s_init = false;
    s_count = 0;
    s_strength = 0.0f;
    s_bounce = 0.0f;
    // Packet kept (same pattern as sun); textures rebound on next move.
    if (s_pkt != nullptr) {
        s_pkt->mpTx[0] = s_pkt->mpTx[1] = s_pkt->mpTx[2] = nullptr;
        for (int i = 0; i < 100; i++) {
            s_pkt->mEff[i].mStatus = 0;
        }
    }
}

void wwVrkumo_move() {
    if (!hostLive()) {
        return;
    }

    // Donor: clear → 50; storm strength grows count toward 100.
    // Weather pat 1/2 are the cloudy patterns (shared lineage with TP).
    if ((g_env_light.wether_pat1 == 1 && g_env_light.pat_ratio > 0.0f) ||
        (g_env_light.wether_pat0 == 1 && g_env_light.pat_ratio < 1.0f) ||
        (g_env_light.wether_pat1 == 2 && g_env_light.pat_ratio > 0.0f) ||
        (g_env_light.wether_pat0 == 2 && g_env_light.pat_ratio < 1.0f)) {
        cLib_addCalc(&s_strength, 1.0f, 0.1f, 0.003f, 0.0000001f);
    } else {
        cLib_addCalc(&s_strength, 0.0f, 0.08f, 0.002f, 0.00000001f);
    }
    s_count = (int)(s_strength * 50.0f + 50.0f);

    if (!s_init) {
        if (s_pkt == nullptr) {
            s_pkt = new WwVrkumo_Packet();
            if (s_pkt == nullptr) {
                return;
            }
            for (int i = 0; i < 100; i++) {
                s_pkt->mEff[i].mStatus = 0;
            }
        }
        // Donor outdoor path: Stage cloudtx (sea Stg_00). Retry until resident;
        // never permanent-latch like TP status 99.
        s_pkt->mpTx[0] = (u8*)dComIfG_getStageRes("cloudtx_01.bti");
        s_pkt->mpTx[1] = (u8*)dComIfG_getStageRes("cloudtx_02.bti");
        s_pkt->mpTx[2] = (u8*)dComIfG_getStageRes("cloudtx_03.bti");
        if (s_pkt->mpTx[0] == nullptr || s_pkt->mpTx[1] == nullptr || s_pkt->mpTx[2] == nullptr) {
            static int s_wait = 0;
            s_wait++;
            if (s_wait <= 3 || (s_wait % 300) == 0) {
                vrkumoLog("[WwSky] {\"ev\":\"vrkumo_tx_wait\",\"n\":%d,\"t1\":%d,\"t2\":%d,"
                          "\"t3\":%d}",
                          s_wait, s_pkt->mpTx[0] != nullptr, s_pkt->mpTx[1] != nullptr,
                          s_pkt->mpTx[2] != nullptr);
            }
            return;
        }
        s_init = true;
        moveEffs();
        vrkumoLog("[WwSky] {\"ev\":\"vrkumo_live\",\"count\":%d,\"str\":%.3f}", s_count,
                  s_strength);
        return;
    }

    moveEffs();
    static int s_ticks = 0;
    s_ticks++;
    if ((s_ticks % 600) == 1) {
        const WwVrkumoEff& e = s_pkt->mEff[0];
        vrkumoLog("[WwSky] {\"ev\":\"vrkumo_tick\",\"count\":%d,\"str\":%.3f,"
                  "\"k0\":{\"a\":%.3f,\"fall\":%.3f,\"pos\":[%.0f,%.0f,%.0f]}}",
                  s_count, s_strength, e.mAlpha, e.mDistFalloff, e.mPosition.x, e.mPosition.y,
                  e.mPosition.z);
    }
}

void wwVrkumo_drawInto(void* drawBuf) {
    (void)drawBuf;
    if (!s_init || s_pkt == nullptr) {
        return;
    }
    // Donor: XLU sky after OPA dome. See wwSkyEntryBuf.
    J3DDrawBuffer* xlu = j3dSys.getDrawBuffer(1);
    if (xlu != nullptr) {
        xlu->entryImm(s_pkt, 0);
    } else {
        s_pkt->draw();
    }
}
