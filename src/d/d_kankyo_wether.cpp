/**
 * d_kankyo_wether.cpp
 * Weather Effects
 */

#include "d/dolzel.h" // IWYU pragma: keep

#include "d/d_kankyo_wether.h"
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/JGeometry.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_counter.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/d_bg_s_gnd_chk.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_rain.h"
#include "d/d_particle.h"
#include "f_op/f_op_camera_mng.h"
#include <cstring>
#include "m_Do/m_Do_audio.h"
#if TARGET_PC
#include "SSystem/SComponent/c_phase.h"
#include "d/d_ext_save_guard.h"
#include "dusk/logging.h"
#if TARGET_PC
#include "dusk/fps_probe.h"
#include "dusk/main.h"
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
namespace fs = std::filesystem;
#endif
#endif

static void dKyw_pntlight_set(WIND_INFLUENCE* pntwind);

static J3DPacket* dKyw_setDrawPacketList(J3DPacket* i_packet, int i_type) {
    if (i_packet == NULL) {
        return NULL;
    } else {
        dComIfGd_getListFilter()->entryImm(i_packet, 0);
        return i_packet;
    }
}

static J3DPacket* dKyw_setDrawPacketListIndScreen(J3DPacket* i_packet, int i_type) {
    if (i_packet == NULL) {
        return NULL;
    } else {
        dComIfGd_getOpaListIndScreen()->entryImm(i_packet, 0);
        return i_packet;
    }
}

static J3DPacket* dKyw_setDrawPacketListSky(J3DPacket* i_packet, int i_type) {
    if (i_packet == NULL) {
        return NULL;
    } else {
        dComIfGd_setListSky();
        j3dSys.getDrawBuffer(i_type)->entryImm(i_packet, 0);
        dComIfGd_setList();
        return i_packet;
    }
}

static J3DPacket* dKyw_setDrawPacketListXluBg(J3DPacket* i_packet, int i_type) {
    if (i_packet == NULL) {
        return NULL;
    } else {
        dComIfGd_setXluListBG();
        j3dSys.getDrawBuffer(i_type)->entryImm(i_packet, 0);
        dComIfGd_setList();
        return i_packet;
    }
}

void dKankyo_sun_Packet::draw() {
    dKyr_drawSun(j3dSys.getViewMtx(), mPos, mColor, &mpResMoon);
}

void dKankyo_sunlenz_Packet::draw() {
    dKyr_drawLenzflare(j3dSys.getViewMtx(), mPositions, mColor, &mpResBall);
}

RAIN_EFF::~RAIN_EFF() {}

RAIN_EFF::RAIN_EFF() {}

void dKankyo_rain_Packet::draw() {
    dKyr_drawSibuki(j3dSys.getViewMtx(), &mpTex);
    dKyr_drawRain(j3dSys.getViewMtx(), &mpTex);
}

SNOW_EFF::~SNOW_EFF() {}

SNOW_EFF::SNOW_EFF() {}

void dKankyo_snow_Packet::draw() {
    GX_DEBUG_GROUP(dKyr_drawSnow, j3dSys.getViewMtx(), &mpTex);
}

STAR_EFF::~STAR_EFF() {}

STAR_EFF::STAR_EFF() {}

void dKankyo_star_Packet::draw() {
    dKyr_drawStar(j3dSys.getViewMtx(), &mpTex);
}

CLOUD_EFF::~CLOUD_EFF() {}

CLOUD_EFF::CLOUD_EFF() {}

void dKankyo_cloud_Packet::draw() {
    drawCloudShadow(j3dSys.getViewMtx(), &mpResTex);
}

HOUSI_EFF::~HOUSI_EFF() {}

HOUSI_EFF::HOUSI_EFF() {}

void dKankyo_housi_Packet::draw() {
    GX_DEBUG_GROUP(dKyr_drawHousi, j3dSys.getViewMtx(), &mpResTex);
}

VRKUMO_EFF::~VRKUMO_EFF() {}

VRKUMO_EFF::VRKUMO_EFF() {}

void dKankyo_vrkumo_Packet::draw() {
    drawVrkumo(j3dSys.getViewMtx(), mColor, &mpResCloudtx_01);
}

EF_ODOUR_EFF::~EF_ODOUR_EFF() {}

EF_ODOUR_EFF::EF_ODOUR_EFF() {}

void dKankyo_odour_Packet::draw() {
    GX_DEBUG_GROUP(dKyr_odour_draw, j3dSys.getViewMtx(), &mpResTex);
}

EF_MUD_EFF::~EF_MUD_EFF() {}

EF_MUD_EFF::EF_MUD_EFF() {}

void dKankyo_mud_Packet::draw() {
    dKyr_mud_draw(j3dSys.getViewMtx(), &mpMoyaRes);
}

EF_EVIL_EFF::~EF_EVIL_EFF() {}

EF_EVIL_EFF::EF_EVIL_EFF() {}

void dKankyo_evil_Packet::draw() {
    dKyr_evil_draw(j3dSys.getViewMtx(), &mpMoyaRes);
}

static void dKyw_drawSun(int i_type) {
    dKyw_setDrawPacketListSky(g_env_light.mpSunPacket, i_type);
}

static void dKyw_Sun_Draw() {
    dKyw_drawSun(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawSunlenz(int i_type) {
    if (g_env_light.mpSunLenzPacket->mDrawLenzInSky == false) {
        dKyw_setDrawPacketListIndScreen(g_env_light.mpSunLenzPacket, i_type);
    } else {
        dKyw_setDrawPacketListSky(g_env_light.mpSunLenzPacket, i_type);
    }
}

static void dKyw_Sunlenz_Draw() {
    dKyw_drawSunlenz(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawRain(int i_type) {
    dKyw_setDrawPacketList(g_env_light.mpRainPacket, i_type);
}

static void dKyw_Rain_Draw() {
    dKyw_drawRain(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawSnow(int i_type) {
    dKyw_setDrawPacketList(g_env_light.mpSnowPacket, i_type);
}

static void dKyw_Snow_Draw() {
    dKyw_drawSnow(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawStar(int i_type) {
    dKyw_setDrawPacketListSky(g_env_light.mpStarPacket, i_type);
}

static void dKyw_Star_Draw() {
    dKyw_drawStar(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawHousi(int i_type) {
    dKyw_setDrawPacketList(g_env_light.mpHousiPacket, i_type);
}

static void dKyw_Housi_Draw() {
    dKyw_drawHousi(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawCloud(int i_type) {
    dKyw_setDrawPacketListIndScreen(g_env_light.mpCloudPacket, i_type);
}

static void dKyw_Cloud_Draw() {
    dKyw_drawCloud(J3DSysDrawBuf_Xlu);
}

static void dKyw_drawVrkumo(int i_type) {
    dKyw_setDrawPacketListSky(g_env_light.mpVrkumoPacket, i_type);
}

static void dKyw_Vrkumo_Draw() {
    dKyw_drawVrkumo(J3DSysDrawBuf_Xlu);
}

static void dKyw_shstar_packet(int i_type) {
    dKyw_setDrawPacketListSky(g_env_light.mpShstarPacket, i_type);
}

static void dKyw_shstar_Draw() {
    dKyw_shstar_packet(J3DSysDrawBuf_Xlu);
}

static void dKyw_odour_packet(int i_type) {
    dKyw_setDrawPacketListIndScreen(g_env_light.mOdourData.mpOdourPacket, i_type);
}

static void dKyw_Odour_Draw() {
    dKyw_odour_packet(J3DSysDrawBuf_Xlu);
}

static void dKyw_mud_packet(int i_type) {
    dKyw_setDrawPacketListXluBg(g_env_light.mpMudPacket, i_type);
}

static void dKyw_mud_Draw() {
    dKyw_mud_packet(J3DSysDrawBuf_Xlu);
}

static void dKyw_evil_packet(int i_type) {
    dKyw_setDrawPacketListXluBg(g_env_light.mpEvilPacket, i_type);
}

static void dKyw_evil_Draw() {
    dKyw_evil_packet(J3DSysDrawBuf_Xlu);
}

#if TARGET_PC
static void dKyw_drawWave(int i_type) {
    dKyw_setDrawPacketListXluBg(g_env_light.mpWavePacket, i_type);
}

static void dKyw_Wave_Draw() {
    dKyw_drawWave(J3DSysDrawBuf_Xlu);
}
#endif

void dKyw_wether_init() {
    g_env_light.mSunInitialized = false;
    g_env_light.mThunderEff.mStatus = 0;
    g_env_light.mThunderEff.mMode = 0;
    g_env_light.mRainInitialized = false;
    g_env_light.raincnt = 0;
    g_env_light.mSnowInitialized = false;
    g_env_light.field_0xe92 = 0;
    g_env_light.mSnowCount = 0;
    g_env_light.field_0xe90 = 0;
    g_env_light.field_0xe91 = 0;
    g_env_light.mStarInitialized = false;
    g_env_light.mStarCount = 0;
    g_env_light.mStarDensity = 0.0f;
    g_env_light.mCloudInitialized = 0;
    g_env_light.mMoyaCount = 0;
    g_env_light.field_0xebc = 0.0f;
    g_env_light.mMoyaMode = 0;
    g_env_light.mHousiInitialized = false;
    g_env_light.mHousiCount = 0;
    g_env_light.mOdourData.mOdourPacketStatus = 0;
    g_env_light.mOdourData.field_0xf24 = 0;
    g_env_light.mOdourData.field_0xf21 = 0;
    g_env_light.mOdourData.mpOdourPacket = NULL;
    g_env_light.field_0x1038 = 0;
    g_env_light.field_0x103c = 0;
    g_env_light.mMudInitialized = 0;
    g_env_light.field_0x1048 = 0;
    g_env_light.mEvilInitialized = 0;
    g_env_light.field_0x1054 = 0;
    g_env_light.field_0x1051 = 0;
    g_env_light.unk_0xe60 = 0.0f;
#if TARGET_PC
    dKy_wave_chan_init();
#endif

    dKyw_wind_init();
    dKyw_pntwind_init();

    // Stage is Zora's Domain and Room is Outside Throne Room
    if (!strcmp(dComIfGp_getStartStageName(), "F_SP113") && dComIfGp_roomControl_getStayNo() == 1 &&
        dComIfG_play_c::getLayerNo(0) < 8)
    {
        cXyz tmp;
        tmp.z = 0.0f;
        tmp.y = 0.0f;
        tmp.x = 0.0f;

        dComIfGp_particle_set(0x878F, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8790, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8791, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8792, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8793, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8794, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8795, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8796, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8797, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8798, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x8799, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x879A, &tmp, NULL, NULL);
        dComIfGp_particle_set(0x879B, &tmp, NULL, NULL);
    }

    // Stage is City in the Sky
    // Room is Entrance or Right Wing Outside or Left Wing Outside or Central Hub Outside
    if (!strcmp(dComIfGp_getStartStageName(), "D_MN07")) {
        if ((dComIfGp_roomControl_getStayNo() == 0 || dComIfGp_roomControl_getStayNo() == 3) ||
            dComIfGp_roomControl_getStayNo() == 6 || dComIfGp_roomControl_getStayNo() == 13)
        {
            g_mEnvSeMgr.initStrongWindSe();
        }
    }
}

void dKyw_wether_init2() {
    g_env_light.mVrkumoStatus = 0;
    g_env_light.mVrkumoCount = 0;
}

void dKyw_wether_delete() {
    if (g_env_light.mSunInitialized) {
        JKR_DELETE(g_env_light.mpSunPacket);
        JKR_DELETE(g_env_light.mpSunLenzPacket);
        g_env_light.mpSunPacket = NULL;
        g_env_light.mpSunLenzPacket = NULL;
    }

    if (g_env_light.mRainInitialized) {
        JKR_DELETE(g_env_light.mpRainPacket);
        g_env_light.mpRainPacket = NULL;
    }

    if (g_env_light.mSnowInitialized) {
        JKR_DELETE(g_env_light.mpSnowPacket);
        g_env_light.mpSnowPacket = NULL;
        g_env_light.field_0xe90 = 0;
    }

    if (g_env_light.mStarInitialized) {
        JKR_DELETE(g_env_light.mpStarPacket);
        g_env_light.mpStarPacket = NULL;
    }

    if (g_env_light.mHousiInitialized) {
        JKR_DELETE(g_env_light.mpHousiPacket);
        g_env_light.mpHousiPacket = NULL;
    }

    if (g_env_light.mCloudInitialized) {
        JKR_DELETE(g_env_light.mpCloudPacket);
        g_env_light.mpCloudPacket = NULL;
    }

    if (g_env_light.mOdourData.mOdourPacketStatus) {
        JKR_DELETE(g_env_light.mOdourData.mpOdourPacket);
        g_env_light.mOdourData.mpOdourPacket = NULL;
    }

    if (g_env_light.mMudInitialized) {
        JKR_DELETE(g_env_light.mpMudPacket);
        g_env_light.mpMudPacket = NULL;
    }

    if (g_env_light.mEvilInitialized) {
        JKR_DELETE(g_env_light.mpEvilPacket);
        g_env_light.mpEvilPacket = NULL;
    }

#if TARGET_PC
    if (g_env_light.mWaveInitialized) {
        JKR_DELETE(g_env_light.mpWavePacket);
        g_env_light.mpWavePacket = NULL;
        g_env_light.mWaveInitialized = 0;
    }
    dKyw_ww_windline_delete();
#endif
}

dKankyo_evil_Packet::~dKankyo_evil_Packet() {}

dKankyo_mud_Packet::~dKankyo_mud_Packet() {}

#if TARGET_PC
WAVE_EFF::WAVE_EFF() {}
WAVE_EFF::~WAVE_EFF() {}

void dKankyo_wave_Packet::draw() {
    drawWave(j3dSys.getViewMtx(), &mpTexUsonami);
}

dKankyo_wave_Packet::~dKankyo_wave_Packet() {}
#endif

dKankyo_odour_Packet::~dKankyo_odour_Packet() {}

dKankyo_cloud_Packet::~dKankyo_cloud_Packet() {}

dKankyo_housi_Packet::~dKankyo_housi_Packet() {}

dKankyo_star_Packet::~dKankyo_star_Packet() {}

dKankyo_snow_Packet::~dKankyo_snow_Packet() {}

dKankyo_rain_Packet::~dKankyo_rain_Packet() {}

dKankyo_sunlenz_Packet::~dKankyo_sunlenz_Packet() {}

dKankyo_sun_Packet::~dKankyo_sun_Packet() {}

void dKyw_wether_delete2() {
    if (g_env_light.mVrkumoStatus != 0) {
        JKR_DELETE(g_env_light.mpVrkumoPacket);
        g_env_light.mpVrkumoPacket = NULL;
    }
}

dKankyo_vrkumo_Packet::~dKankyo_vrkumo_Packet() {}

static void wether_move_thunder() {
    switch (g_env_light.mThunderEff.mStatus) {
    case 0:
        if (g_env_light.mThunderEff.mMode != 0) {
            dKyr_thunder_init();
            g_env_light.mThunderEff.mStatus = 1;
        }
        break;
    case 1:
        dKyr_thunder_move();
        break;
    }
}

#if TARGET_PC
static void wether_move_windline();
#endif

void dKyw_wether_move() {
    wether_move_thunder();
#if TARGET_PC
    wether_move_windline();
#endif
}

static void wether_move_sun() {
    s32 sunVisible = false;
    if (dComIfGp_checkStatus(1) && !g_env_light.hide_vrbox) {
        roomRead_class* room = dComIfGp_getStageRoom();
        if (room != NULL && room->num > dComIfGp_roomControl_getStayNo()) {
            sunVisible = dStage_roomRead_dt_c_GetVrboxswitch(
                *room->m_entries[dComIfGp_roomControl_getStayNo()]);
        }

        // Stage is Hero Shade arena
        if (!strcmp(dComIfGp_getStartStageName(), "F_SP200")) {
            sunVisible = true;
            // Stage is Hyrule Castle or Castle Throne Room
        } else if (!strcmp(dComIfGp_getStartStageName(), "D_MN09") ||
                   !strcmp(dComIfGp_getStartStageName(), "D_MN09A"))
        {
            sunVisible = false;
        }

        switch (g_env_light.mSunInitialized) {
        case FALSE:
            if (sunVisible && dKy_darkworld_check() != true) {
                g_env_light.mpSunPacket = JKR_NEW_ARGS (0x20) dKankyo_sun_Packet;
                g_env_light.mpSunLenzPacket = JKR_NEW_ARGS (0x20) dKankyo_sunlenz_Packet;
                if (g_env_light.mpSunPacket != NULL && g_env_light.mpSunLenzPacket != NULL) {
                    g_env_light.mpSunPacket->mpResMoon = (u8*)dComIfG_getStageRes("F_moon.bti");
                    g_env_light.mpSunPacket->mpResMoon_A = (u8*)dComIfG_getStageRes("F_moon_A.bti");
                    g_env_light.mpSunPacket->mpResMoon_A_A00 =
                        (u8*)dComIfG_getStageRes("F_moon_A_A00.bti");
                    g_env_light.mpSunPacket->mpResMoon_A_A01 =
                        (u8*)dComIfG_getStageRes("F_moon_A_A01.bti");
                    g_env_light.mpSunPacket->mpResMoon_A_A02 =
                        (u8*)dComIfG_getStageRes("F_moon_A_A02.bti");
                    g_env_light.mpSunPacket->mpResMoon_A_A03 =
                        (u8*)dComIfG_getStageRes("F_moon_A_A03.bti");

                    if (g_env_light.mpSunPacket->mpResMoon == NULL) {
                        g_env_light.mpSunPacket->mpResMoon =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                        g_env_light.mpSunPacket->mpResMoon_A =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                        g_env_light.mpSunPacket->mpResMoon_A_A00 =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                        g_env_light.mpSunPacket->mpResMoon_A_A01 =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                        g_env_light.mpSunPacket->mpResMoon_A_A02 =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                        g_env_light.mpSunPacket->mpResMoon_A_A03 =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                    }
                    g_env_light.mpSunPacket->field_0x28 = 0;
                    g_env_light.mpSunPacket->field_0x29 = 0;
                    g_env_light.mpSunPacket->mVisibility = 0.0f;
                    g_env_light.mpSunPacket->mSunAlpha = 0.0f;
                    g_env_light.mpSunPacket->field_0x64 = 0.0f;
                    g_env_light.mpSunPacket->mMoonAlpha = 0.0f;

                    if (g_env_light.daytime < 255.0f) {
                        g_env_light.mpSunPacket->field_0x6c = 1.0f;
                    } else {
                        g_env_light.mpSunPacket->field_0x6c = 0.0f;
                    }
                    g_env_light.mpSunPacket->field_0x44[0] = 0;
                    g_env_light.mpSunPacket->field_0x44[1] = 0;
                    g_env_light.mpSunPacket->field_0x44[2] = 0;
                    g_env_light.mpSunPacket->field_0x44[3] = 0;
                    g_env_light.mpSunPacket->field_0x44[4] = 0;
                    g_env_light.mpSunPacket->field_0x58 = 0;

                    g_env_light.mpSunLenzPacket->mpResBall =
                        (u8*)dComIfG_getObjectRes("Always", 0x4A);
                    g_env_light.mpSunLenzPacket->mpResRing_A =
                        (u8*)dComIfG_getObjectRes("Always", 0x57);
                    g_env_light.mpSunLenzPacket->mpResLenz =
                        (u8*)dComIfG_getObjectRes("Always", 0x5C);
                    g_env_light.mpSunLenzPacket->field_0x8c = 1000000000.0f; // This is not G_CM3D_F_INF
                    g_env_light.mpSunLenzPacket->field_0x90 = 0.0f;
                    g_env_light.mpSunLenzPacket->mDistFalloff = 0.0f;
                    g_env_light.mpSunLenzPacket->mDrawLenzInSky = false;
                    dKyr_sun_move();
                    dKyr_lenzflare_move();
                    g_env_light.mSunInitialized = true;
                }
            }
            break;
        case TRUE:
            if (!sunVisible) {
                g_env_light.mSunInitialized = false;
                JKR_DELETE(g_env_light.mpSunPacket);
                JKR_DELETE(g_env_light.mpSunLenzPacket);
                g_env_light.mpSunPacket = NULL;
                g_env_light.mpSunLenzPacket = NULL;
            } else {
                dKyr_sun_move();
                dKyr_lenzflare_move();
            }
            break;
        }
    }
}

static void wether_move_rain() {
    switch (g_env_light.mRainInitialized) {
    case FALSE:
        if (g_env_light.raincnt > 3) {
            g_env_light.mpRainPacket = JKR_NEW_ARGS (32) dKankyo_rain_Packet;

            if (g_env_light.mpRainPacket != NULL) {
                dKyr_rain_init();
                dKyr_rain_move();
                g_env_light.mRainInitialized = 1;

                if (g_env_light.raincnt != 250) {
                    mDoAud_seStart(JA_SE_ATM_RAIN_START, NULL, 0, 0);
                }
            }
        }
        break;
    case TRUE:
        camera_process_class* cam = dComIfGp_getCamera(0);
        dKyr_rain_move();

        if (g_env_light.mSnowCount == 0 && cam != NULL) {
            // Stage is not Fishing Pond
            if (strcmp(dComIfGp_getStartStageName(), "R_SP127") || cam->view.lookat.eye.y > 0.0f) {
                if (g_env_light.raincnt < 125.0f) {
                    mDoAud_rainPlay(FALSE);
                } else {
                    mDoAud_rainPlay(TRUE);
                }
            }
        }

        if (g_env_light.raincnt <= 3) {
            g_env_light.mRainInitialized = 0;
            mDoAud_seStart(JA_SE_ATM_RAIN_END, NULL, 0, 0);
            JKR_DELETE(g_env_light.mpRainPacket);
            g_env_light.mpRainPacket = NULL;
        }
        break;
    }
}

static void wether_move_snow() {
    switch (g_env_light.mSnowInitialized) {
    case FALSE:
        if (g_env_light.mSnowCount != 0 || g_env_light.field_0xe90 != 0) {
            dKyr_snow_init();
            if (g_env_light.mpSnowPacket != NULL) {
                dKyr_snow_move();
                g_env_light.mSnowInitialized = 1;
            }
        }
        break;
    case TRUE:
        dKyr_snow_move();
        break;
    }
}

static void wether_move_star() {
    s32 starsVisible = false;
    // Stage is Hyrule Castle or Castle Throne Room
    if (!strcmp(dComIfGp_getStartStageName(), "D_MN09") ||
        !strcmp(dComIfGp_getStartStageName(), "D_MN09A"))
    {
        return;
    } else {
        // Stage is Hero Shade arena
        if ((dComIfGp_checkStatus(1) && !g_env_light.hide_vrbox) ||
            !strcmp(dComIfGp_getStartStageName(), "F_SP200"))
        {
            roomRead_class* room = dComIfGp_getStageRoom();
            if (room != NULL && room->num > dComIfGp_roomControl_getStayNo()) {
                starsVisible = dStage_roomRead_dt_c_GetVrboxswitch(
                    *room->m_entries[dComIfGp_roomControl_getStayNo()]);
            }

            // Stage is Hero Shade arena
            if (!strcmp(dComIfGp_getStartStageName(), "F_SP200")) {
                starsVisible = true;
            }

            if (starsVisible && dKy_darkworld_check() != true) {
                f32 density;
                f32 time = g_env_light.getDaytime();
                if (time >= 330.0f || time < 45.0f) {
                    density = 1.0f;
                } else if (time > 285.0f) {
                    density = 1.0f - (1.0f / 45.0f) * (330.0f - time);
                } else {
                    if (time < 75.0f) {
                        density = (1.0f / 30.0f) * (75.0f - time);
                    } else {
                        density = 0.0f;
                    }
                }

                if (g_env_light.mColpatWeather != 0) {
                    density = 0.0f;
                }

                if (g_env_light.wether_pat1 != 0 && g_env_light.pat_ratio > 0.5f) {
                    density = 0.0f;
                }

                // Stage is Hero Shade arena
                if (!strcmp(dComIfGp_getStartStageName(), "F_SP200")) {
                    density = 1.0f;
                }

                cLib_addCalc(&g_env_light.mStarDensity, density, 0.1f, 0.01f, 0.000001f);
                g_env_light.mStarCount = (s16)(1200.0f * g_env_light.mStarDensity);

                // Stage is Hyrule Field or Outside Castle Town or Hidden Village
                if (!strcmp(dComIfGp_getStartStageName(), "F_SP121") ||
                    !strcmp(dComIfGp_getStartStageName(), "F_SP122") ||
                    !strcmp(dComIfGp_getStartStageName(), "F_SP128"))
                {
                    g_env_light.mStarCount = (s16)(500.0f * g_env_light.mStarDensity);
                }

                switch (g_env_light.mStarInitialized) {
                case FALSE:
                    if (g_env_light.mStarCount != 0) {
                        dKyr_star_init();
                        dKyr_shstar_init();

                        if (g_env_light.mpStarPacket != NULL) {
                            dKyr_star_move();
                            dKyr_shstar_move();
                            g_env_light.mStarInitialized = true;
                        }
                    }
                    break;
                case TRUE:
                    dKyr_star_move();
                    dKyr_shstar_move();

                    if (g_env_light.mStarCount == 0) {
                        g_env_light.mStarInitialized = false;

                        JKR_DELETE(g_env_light.mpStarPacket);
                        g_env_light.mpStarPacket = NULL;
                    }
                    break;
                }
            }
        }
    }
}

static void wether_move_housi() {
    // Stage is Palace of Twilight or Zant Throne Room, or Phantom Zant arena
    // Room is not Entrance or Boss Key room
    if ((!strcmp(dComIfGp_getStartStageName(), "D_MN08") && dComIfGp_roomControl_getStayNo() != 0 &&
         dComIfGp_roomControl_getStayNo() != 11) ||
        (!strcmp(dComIfGp_getStartStageName(), "D_MN08A") ||
         !strcmp(dComIfGp_getStartStageName(), "D_MN08B") ||
         !strcmp(dComIfGp_getStartStageName(), "D_MN08C")))
    {
        return;
    }

    // Stage is darkworld or Stage is Lake Hylia and Room is Lanayru Spring
    if (dKy_darkworld_check() == true ||
        (!strcmp(dComIfGp_getStartStageName(), "F_SP115") &&
         dComIfGp_roomControl_getStayNo() == 1 && dComIfGp_getStartStageLayer() == 9))
    {
        if (g_env_light.light_init_timer != 0) {
            g_env_light.field_0xea9 = 0;
            g_env_light.mHousiCount = 200;

            // Stage is Faron Woods and Room is Faron Spring or
            // Stage is Castle Town and Room is Castle Town Center or
            // Stage is Faron Woods and Room is South Faron or
            // Stage is Lake Hylia
            if ((!strcmp(dComIfGp_getStartStageName(), "F_SP108") &&
                 dComIfGp_getStartStageRoomNo() == 1 && dComIfGp_getStartStageLayer() == 13) ||
                (!strcmp(dComIfGp_getStartStageName(), "F_SP116") &&
                 dComIfGp_getStartStageRoomNo() == 0 && dComIfGp_getStartStageLayer() == 8) ||
                (!strcmp(dComIfGp_getStartStageName(), "F_SP108") &&
                 dComIfGp_getStartStageRoomNo() == 0 && dComIfGp_getStartStageLayer() == 9) ||
                (!strcmp(dComIfGp_getStartStageName(), "F_SP115") &&
                 dComIfGp_getStartStageLayer() == 9))
            {
                g_env_light.mHousiCount = 0;
            }
        }
    }

    switch (g_env_light.mHousiInitialized) {
    case FALSE:
        if (g_env_light.mHousiCount != 0) {
            g_env_light.mpHousiPacket = JKR_NEW_ARGS (32) dKankyo_housi_Packet;

            if (g_env_light.mpHousiPacket != NULL) {
                if (dKy_darkworld_check() == true) {
                    g_env_light.mpHousiPacket->mpResTex = (u8*)dComIfG_getObjectRes("Always", 0x5E);
                } else {
                    if (g_env_light.field_0xea9 == 2) {
                        if (g_env_light.fishing_hole_season == 3) {
                            g_env_light.mpHousiPacket->mpResTex =
                                (u8*)dComIfG_getStageRes("momiji64s3tc.bti");
                        } else {
                            g_env_light.mpHousiPacket->mpResTex =
                                (u8*)dComIfG_getStageRes("sakura32ia4.bti");
                        }

                        if (g_env_light.mpHousiPacket->mpResTex == NULL) {
                            g_env_light.mpHousiPacket->mpResTex =
                                (u8*)dComIfG_getObjectRes("Always", 0x55);
                        }
                    } else if (g_env_light.field_0xea9 == 0) {
                        g_env_light.mpHousiPacket->mpResTex =
                            (u8*)dComIfG_getObjectRes("Always", 0x55);
                    } else {
                        g_env_light.mpHousiPacket->mpResTex =
                            (u8*)dComIfG_getObjectRes("Always", 0x56);
                    }
                }

                g_env_light.mpHousiPacket->field_0x5de8 = 0.0f;
                g_env_light.mpHousiPacket->field_0x10.set(0.0f, 0.0f, 0.0f);
                for (int i = 0; i < 300; i++) {
                    g_env_light.mpHousiPacket->mHousiEff[i].mStatus = 0;
                }
                dKyr_housi_move();
                g_env_light.mHousiInitialized = true;
            }
        }
        break;
    case TRUE:
        if (g_env_light.mHousiCount == 0 &&
            g_env_light.mpHousiPacket->field_0x5de8 <= 0.0f)
        {
            g_env_light.mHousiInitialized = false;
            JKR_DELETE(g_env_light.mpHousiPacket);
            g_env_light.mpHousiPacket = NULL;
        } else {
            dKyr_housi_move();
            if (!dKy_darkworld_check()) {
                g_env_light.mHousiCount = 0;
            }
        }
        break;
    }
}

static void wether_move_odour() {
    switch (g_env_light.mOdourData.mOdourPacketStatus) {
    case 0:
        if (g_env_light.mOdourData.field_0xf21 != 0) {
            g_env_light.mOdourData.mpOdourPacket = JKR_NEW_ARGS (32) dKankyo_odour_Packet;

            if (g_env_light.mOdourData.mpOdourPacket != NULL) {
                g_env_light.mOdourData.mpOdourPacket->mpResTex = (u8*)dComIfG_getObjectRes("Always", 0x53);
                dKyr_odour_init();
                dKyr_odour_move();
                g_env_light.mOdourData.mOdourPacketStatus = 1;
            }
        }
        break;
    case 1:
        if (g_env_light.mOdourData.field_0xf21 == 0) {
            g_env_light.mOdourData.mOdourPacketStatus = 0;
            if (g_env_light.mOdourData.mpOdourPacket != NULL) {
                JKR_DELETE(g_env_light.mOdourData.mpOdourPacket);
                g_env_light.mOdourData.mpOdourPacket = NULL;
            }
        } else {
            dKyr_odour_move();
        }
        break;
    case 2:
        g_env_light.mOdourData.field_0xf21 = 0;
        g_env_light.mOdourData.mOdourPacketStatus = 0;
        JKR_DELETE(g_env_light.mOdourData.mpOdourPacket);
        g_env_light.mOdourData.mpOdourPacket = NULL;
        break;
    }
}

static void wether_move_moya() {
    switch (g_env_light.mCloudInitialized) {
    case 0:
        if (g_env_light.mMoyaCount != 0) {
            g_env_light.mpCloudPacket = JKR_NEW_ARGS (32) dKankyo_cloud_Packet;

            if (g_env_light.mpCloudPacket != NULL) {
                g_env_light.mpCloudPacket->mpResTex = (u8*)dComIfG_getObjectRes("Always", 0x53);

                for (int i = 0; i < 50; i++) {
                    g_env_light.mpCloudPacket->mCloudEff[i].mStatus = 0;
                }
                g_env_light.mpCloudPacket->mCount = 0;
                cloud_shadow_move();
                g_env_light.mCloudInitialized++;
            }
        }
        break;
    case 1:
        if (g_env_light.mMoyaMode == 0) {
            dKyw_get_wind_pow();
        }

        cloud_shadow_move();

        if (g_env_light.mMoyaCount == 0 && g_env_light.mpCloudPacket->mCount == 0) {
            g_env_light.mCloudInitialized = 0;

            JKR_DELETE(g_env_light.mpCloudPacket);
            g_env_light.mpCloudPacket = NULL;
        }
        break;
    }
}

static void wether_move_vrkumo() {
    BOOL var_r31 = false;
    static cXyz r09o(-180000.0f, 750.0f, -200000.0f);

    if (dComIfGp_checkStatus(1) && !g_env_light.hide_vrbox) {
        g_env_light.mVrkumoCount = 6;

        if (memcmp(dComIfGp_getStartStageName(), "D_MN07", 6) == 0 ||
            strcmp(dComIfGp_getStartStageName(), "F_SP114") == 0 ||
            (strcmp(dComIfGp_getStartStageName(), "D_MN09B") == 0 &&
             (g_env_light.wether != 0 || dComIfGp_event_runCheck())))
        {
            cLib_addCalc(&g_env_light.mVrkumoStrength, 1.0f, 0.1f, 0.003f, 0.0000001f);
        } else if (strcmp(dComIfGp_getStartStageName(), "F_SP104") == 0 &&
                   dComIfG_play_c::getLayerNo(0) >= 3)
        {
            if (g_env_light.wether_pat1 >= 4) {
                cLib_addCalc(&g_env_light.mVrkumoStrength, 1.0f, 0.1f, 0.003f, 0.0000001f);
            } else {
                cLib_addCalc(&g_env_light.mVrkumoStrength, 0.0f, 0.08f, 0.002f, 0.00000001f);
            }
        } else if ((g_env_light.wether_pat1 == 1 && g_env_light.pat_ratio > 0.0f) ||
                   (g_env_light.wether_pat0 == 1 && g_env_light.pat_ratio < 1.0f) ||
                   (g_env_light.wether_pat1 == 2 && g_env_light.pat_ratio > 0.0f) ||
                   (g_env_light.wether_pat0 == 2 && g_env_light.pat_ratio < 1.0f))
        {
            cLib_addCalc(&g_env_light.mVrkumoStrength, 1.0f, 0.1f, 0.003f, 0.0000001f);
        } else {
            cLib_addCalc(&g_env_light.mVrkumoStrength, 0.0f, 0.08f, 0.002f, 0.00000001f);
        }

        g_env_light.mVrkumoCount = (s16)(g_env_light.mVrkumoStrength * 56.0f + 6.0f);
    } else {
        g_env_light.mVrkumoCount = 0;
    }

    if (dKy_darkworld_check()) {
        g_env_light.mVrkumoCount = 30;
    }

    roomRead_class* room_p = dComIfGp_getStageRoom();
    if (room_p != NULL && room_p->num > dComIfGp_roomControl_getStayNo()) {
        var_r31 = dStage_roomRead_dt_c_GetVrboxswitch(
            *room_p->m_entries[dComIfGp_roomControl_getStayNo()]);
    }

    if (strcmp(dComIfGp_getStartStageName(), "F_SP200") == 0) {
        g_env_light.mVrkumoCount = 30;
    } else if (var_r31 == 0) {
        return;
    }

    switch (g_env_light.mVrkumoStatus) {
    case 0:
        if (g_env_light.mVrkumoCount != 0) {
            g_env_light.mpVrkumoPacket = JKR_NEW_ARGS (0x20) dKankyo_vrkumo_Packet;
            if (g_env_light.mpVrkumoPacket == NULL) {
                return;
            }

            g_env_light.mpVrkumoPacket->mpResCloudtx_01 =
                (u8*)dComIfG_getStageRes("cloudtx_01.bti");
            g_env_light.mpVrkumoPacket->mpResCloudtx_02 =
                (u8*)dComIfG_getStageRes("cloudtx_02.bti");
            g_env_light.mpVrkumoPacket->mpResCloudtx_03 =
                (u8*)dComIfG_getStageRes("cloudtx_03.bti");

            if (g_env_light.mpVrkumoPacket->mpResCloudtx_01 == NULL ||
                g_env_light.mpVrkumoPacket->mpResCloudtx_02 == NULL ||
                g_env_light.mpVrkumoPacket->mpResCloudtx_03 == NULL)
            {
                g_env_light.mVrkumoStatus = 99;
            }

            for (int i = 0; i < 100; i++) {
                g_env_light.mpVrkumoPacket->mVrkumoEff[i].mStatus = 0;
            }

            g_env_light.mpVrkumoPacket->field_0x1150 = 0.0f;
            g_env_light.mpVrkumoPacket->field_0x1154 = 0.0f;

            vrkumo_move();
            g_env_light.mVrkumoStatus++;
        }
        break;
    case 1:
        vrkumo_move();
        dKyw_get_wind_vec();

        cXyz sp8;
        f32 wind_vec_x = g_env_light.global_wind_influence.vec.x;
        f32 wind_vec_y = g_env_light.global_wind_influence.vec.y;
        f32 wind_vec_z = g_env_light.global_wind_influence.vec.z;
        f32 var_f31 = g_env_light.global_wind_influence.pow;

        if (strcmp(dComIfGp_getStartStageName(), "R_SP127") == 0) {
            var_f31 = 0.3f;
        }

        f32 var_f30 = JMAFastSqrt(wind_vec_x * wind_vec_x + wind_vec_z * wind_vec_z);
        f32 temp_f27_2 = cM_atan2s(wind_vec_x, wind_vec_z);
        f32 temp_r4_2 = cM_atan2s(wind_vec_y, var_f30);

        temp_f27_2 += 24575.0f;

        sp8.x = cM_scos(temp_r4_2) * cM_ssin(temp_f27_2);
        sp8.y = cM_ssin(temp_r4_2);
        sp8.z = cM_scos(temp_r4_2) * cM_scos(temp_f27_2);

        g_env_light.mpVrkumoPacket->field_0x1150 += (sp8.x * var_f31) * 0.0014f;
        g_env_light.mpVrkumoPacket->field_0x1154 += (sp8.z * var_f31) * 0.0014f;

        while (g_env_light.mpVrkumoPacket->field_0x1150 < 0.0f) {
            g_env_light.mpVrkumoPacket->field_0x1150 += 1.0f;
        }

        while (g_env_light.mpVrkumoPacket->field_0x1150 > 1.0f) {
            g_env_light.mpVrkumoPacket->field_0x1150 -= 1.0f;
        }

        while (g_env_light.mpVrkumoPacket->field_0x1154 < 0.0f) {
            g_env_light.mpVrkumoPacket->field_0x1154 += 1.0f;
        }

        while (g_env_light.mpVrkumoPacket->field_0x1154 > 1.0f) {
            g_env_light.mpVrkumoPacket->field_0x1154 -= 1.0f;
        }
        break;
    }
}

static void wether_move_mud() {
    switch (g_env_light.mMudInitialized) {
    case 0:
        if (g_env_light.field_0x1048 != 0) {
            g_env_light.mpMudPacket = JKR_NEW_ARGS (32) dKankyo_mud_Packet;

            if (g_env_light.mpMudPacket != NULL) {
                dKyr_mud_init();
                dKyr_mud_move();
                g_env_light.mMudInitialized++;
            }
        }
        break;
    case 1:
        dKyr_mud_move();

        if (g_env_light.field_0x1048 == 0 && g_env_light.mpMudPacket->mEffectNum == 0) {
            g_env_light.mMudInitialized = 0;

            JKR_DELETE(g_env_light.mpMudPacket);
            g_env_light.mpMudPacket = NULL;
        }
        break;
    }
}

static void wether_move_evil() {
    switch (g_env_light.mEvilInitialized & 0xF) {
    case 0:
        if (g_env_light.field_0x1054 != 0) {
            g_env_light.mpEvilPacket = JKR_NEW_ARGS (32) dKankyo_evil_Packet;

            if (g_env_light.mpEvilPacket != NULL) {
                dKyr_evil_init();
                dKyr_evil_move();
                g_env_light.mEvilInitialized++;
            }
        }
        break;
    case 1:
        dKyr_evil_move();

        if (g_env_light.field_0x1054 == 0 && g_env_light.mpEvilPacket->mEffectNum == 0) {
            g_env_light.mEvilInitialized = 0;

            JKR_DELETE(g_env_light.mpEvilPacket);
            g_env_light.mpEvilPacket = NULL;
        }
        break;
    }
}

#if TARGET_PC
// §97b — Bridge indices for WwAlways USONAMI BTIs (verbatim Always table).
static constexpr u16 kWwAlwaysUsonami = 0x8B;
static constexpr u16 kWwAlwaysUsonamiM = 0x8C;
static request_of_phase_process_class s_wwAlwaysPhase;
static int s_wwAlwaysPhaseState = cPhs_INIT_e;
static bool s_wwAlwaysWarned;

// §101 — package calm map (sea-room wave_max grid analog) + kytag01 influences.
// Polarity (donor CalcFlatInterTarget / CalcFlatInter / wave_move / usonami_set):
//   0.0 = calm → usonami ON;  1.0 = open chop → usonami OFF (>= 1.0 gate).
static constexpr f32 kWaveCalmRamp = 12800.0f;
static constexpr int kWaveCalmMaxBoxes = 8;
static constexpr int kWaveInflMax = 10;

struct WwCalmBox {
    f32 minX, maxX, minZ, maxZ;
};

static WwCalmBox s_calmBoxes[kWaveCalmMaxBoxes];
static int s_calmBoxCount;
static WAVE_INFO s_waveInfl[kWaveInflMax];
static WAVE_INFO* s_waveInflPtrs[kWaveInflMax];
static int s_waveInflCount;
static f32 s_flatInter;
static f32 s_flatTarget;
static f32 s_flatCounter;
static char s_calmStage[12];
static bool s_calmLoaded;

static bool wwFoamFieldActive() {
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage);
}

// Donor cM2dGBox::GetLen — distance to AABB in XZ (0 if inside).
static f32 wwCalmBoxLen(f32 minX, f32 maxX, f32 minZ, f32 maxZ, f32 x, f32 z) {
    if (minX < x && x < maxX && minZ < z && z < maxZ) {
        return 0.0f;
    }
    if (minX < x && x < maxX) {
        f32 d0 = std::fabs(minZ - z);
        f32 d1 = std::fabs(maxZ - z);
        return d0 < d1 ? d0 : d1;
    }
    if (minZ < z && z < maxZ) {
        f32 d0 = std::fabs(minX - x);
        f32 d1 = std::fabs(maxX - x);
        return d0 < d1 ? d0 : d1;
    }
    f32 cx = x < minX ? minX : maxX;
    f32 cz = z < minZ ? minZ : maxZ;
    f32 dx = x - cx;
    f32 dz = z - cz;
    return std::sqrt(dx * dx + dz * dz);
}

// Donor CalcFlatInterTarget — inside any calm box → 0; else min of ramps to
// each calm box expanded by 12800, normalized by 12800. Far from all → 1.0.
static f32 wwCalcFlatInterTarget(f32 x, f32 z) {
    if (s_calmBoxCount <= 0) {
        return 0.0f;  // no map: match prior arm (island calm)
    }
    for (int i = 0; i < s_calmBoxCount; i++) {
        const WwCalmBox& b = s_calmBoxes[i];
        if (b.minX <= x && x <= b.maxX && b.minZ <= z && z <= b.maxZ) {
            return 0.0f;
        }
    }
    f32 result = 1.0f;
    for (int i = 0; i < s_calmBoxCount; i++) {
        const WwCalmBox& b = s_calmBoxes[i];
        f32 len = wwCalmBoxLen(b.minX - kWaveCalmRamp, b.maxX + kWaveCalmRamp,
                               b.minZ - kWaveCalmRamp, b.maxZ + kWaveCalmRamp, x, z);
        if (len > kWaveCalmRamp) {
            len = kWaveCalmRamp;
        }
        len /= kWaveCalmRamp;
        if (result > len) {
            result = len;
        }
    }
    return result;
}

static void wwCalmClear() {
    s_calmBoxCount = 0;
    s_waveInflCount = 0;
    for (int i = 0; i < kWaveInflMax; i++) {
        s_waveInflPtrs[i] = NULL;
    }
    s_flatInter = 1.0f;
    s_flatTarget = 1.0f;
    s_flatCounter = 0.0f;
    s_calmStage[0] = '\0';
    s_calmLoaded = false;
}

static bool wwParse4f(const char* v, f32* a, f32* b, f32* c, f32* d) {
    return std::sscanf(v, "%f,%f,%f,%f", a, b, c, d) == 4;
}

static bool wwParse5f(const char* v, f32* a, f32* b, f32* c, f32* d, f32* e) {
    return std::sscanf(v, "%f,%f,%f,%f,%f", a, b, c, d, e) == 5;
}

static void wwLoadWaveCalmIni(const char* stage) {
    wwCalmClear();
    if (stage == NULL) {
        return;
    }
    std::snprintf(s_calmStage, sizeof(s_calmStage), "%s", stage);

    const fs::path root = dusk::ConfigPath / "model_replacements";
    if (!fs::exists(root)) {
        return;
    }
    fs::path iniPath;
    for (const auto& ent : fs::directory_iterator(root)) {
        if (!ent.is_directory()) {
            continue;
        }
        const fs::path cand = ent.path() / "population" / "wave_calm.ini";
        if (fs::exists(cand)) {
            iniPath = cand;
            break;
        }
    }
    if (iniPath.empty()) {
        DuskLog.warn("[WwFoam] §101 no population/wave_calm.ini — flatInter stays armed-default");
        return;
    }

    std::ifstream in(iniPath);
    if (!in) {
        return;
    }
    std::string line;
    std::string section;
    const std::string want = std::string("[") + stage + "]";
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            section = line;
            continue;
        }
        if (section != want) {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        while (!key.empty() && key.back() == ' ') {
            key.pop_back();
        }
        while (!val.empty() && val.front() == ' ') {
            val.erase(val.begin());
        }
        if (key == "calm_box" && s_calmBoxCount < kWaveCalmMaxBoxes) {
            f32 minX, maxX, minZ, maxZ;
            if (wwParse4f(val.c_str(), &minX, &maxX, &minZ, &maxZ)) {
                if (minX > maxX) {
                    f32 t = minX;
                    minX = maxX;
                    maxX = t;
                }
                if (minZ > maxZ) {
                    f32 t = minZ;
                    minZ = maxZ;
                    maxZ = t;
                }
                s_calmBoxes[s_calmBoxCount++] = {minX, maxX, minZ, maxZ};
            }
        } else if (key == "infl" && s_waveInflCount < kWaveInflMax) {
            f32 x, y, z, inner, outer;
            if (wwParse5f(val.c_str(), &x, &y, &z, &inner, &outer)) {
                if (outer < inner + 500.0f) {
                    outer = inner + 500.0f;  // donor kytag01 bump
                }
                WAVE_INFO& w = s_waveInfl[s_waveInflCount];
                w.mPos.set(x, y, z);
                w.mInnerRadius = inner;
                w.mOuterRadius = outer;
                w.field_0x14 = 0.0f;
                s_waveInflPtrs[s_waveInflCount] = &w;
                s_waveInflCount++;
            }
        }
    }
    for (int i = s_waveInflCount; i < kWaveInflMax; i++) {
        s_waveInflPtrs[i] = NULL;
    }
    s_calmLoaded = true;
    s_flatCounter = 150.0f;  // donor SetFlat/ClrFlat ease window
    DuskLog.info("[WwFoam] §101 calm map '{}' boxes={} infl={} ramp={}", stage, s_calmBoxCount,
                 s_waveInflCount, (int)kWaveCalmRamp);
}

WAVE_INFO* const* dKyw_getWaveInfl() {
    return s_waveInflPtrs;
}

void dKyw_wave_calm_onStage(const char* stage) {
    if (stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage)) {
        wwLoadWaveCalmIni(stage);
    } else {
        wwCalmClear();
    }
}

// Ferry F Stage 2 — ambient sea wind on WW hosts.
// dKyw_get_wind_* reads g_env_light.global_wind_influence, which dKyw_wind_set
// refreshes every frame from FILI/LBNK (→ pow 0 on shell hosts). Durable write
// is the same evt_wind path kytag02/tornado use: angle + custom_windpower.
// Angle Y=0 is a placeholder until History confirms Outset's authored direction.
void dKyw_ww_host_wind_onStage(const char* stage) {
#if TARGET_PC
    DUSK_FPS_SCOPE(Wind);
#endif
    static bool s_armed;
    static constexpr f32 kWwHostAmbientWindPow = 0.4f;
    if (stage != NULL && dExtWwSave_isWwHostStage(stage)) {
        dKyw_evt_wind_set(/*angleX=*/0, /*angleY=*/0);
        dKyw_custom_windpower(kWwHostAmbientWindPow);
        if (!s_armed) {
            s_armed = true;
            DuskLog.info(
                "[WwFoam] FerryF ambient wind ARM stage='{}' pow={:.3f} angY=0 (placeholder)",
                stage, kWwHostAmbientWindPow);
        }
    } else if (s_armed) {
        g_env_light.evt_wind_go = 0;
        g_env_light.custom_windpower = 0.0f;
        s_armed = false;
        DuskLog.info("[WwFoam] FerryF ambient wind DISARM (left WW host)");
    }
}

// =============================================================================
// Ferry W-LINE — donor wether_move_windline + dKyr_wind_init/move
// (d_kankyo_wether.cpp:400 / d_kankyo_rain.cpp:226). Adaptations:
//   1) Force ambient branch — Ferry F feeds via custom_windpower; donor :268
//      would otherwise draw tiny player-anchored tornado lines.
//   2) Gate = F_* WW host only (not R_DL interiors; not TP).
//   3) Particle 0x0031 from WW supplemental common.jpc (slot 3) + bank probe.
//   4) OMIT mKamomeEff. Deku-Leaf count=10 TODO (donor :263; glide state TBD).
// =============================================================================
struct WwWindEff {
    JPABaseEmitter* mpEmitter = NULL;
    cXyz mBasePos;
    cXyz mPos;
    f32 mStateTimer = 0.0f;
    f32 mAlpha = 0.0f;
    int mState = 0;
    int field_0x28 = 0;
    s16 field_0x2c = 0;
    s16 mAngleY = 0;
    s16 mAngleXZ = 0;
    u8 field_0x32 = 0;
};

struct WwWindEffSet {
    WwWindEff mWindEff[30];
    u8 mbHasCustomWindPower = 0;
};

static WwWindEffSet* s_wwWind = NULL;
static bool s_wwWindInitialized = false;
static int s_wwWindlineCount = 0;
static constexpr u16 kWwWindlineParticleId = 0x0031;

static bool wwWindlineHostActive() {
    const char* stage = dComIfGp_getStartStageName();
    // Adaptation 2: exteriors only (F_*), same family as foam field gate.
    return stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage);
}

static void wwWindlineDeleteEmitters(WwWindEffSet* wind) {
    if (wind == NULL) {
        return;
    }
    for (int i = 0; i < 30; i++) {
        if (wind->mWindEff[i].mpEmitter != NULL) {
            wind->mWindEff[i].mpEmitter->deleteAllParticle();
            wind->mWindEff[i].mpEmitter->becomeInvalidEmitter();
            wind->mWindEff[i].mpEmitter = NULL;
        }
        wind->mWindEff[i].mState = 0;
    }
}

static void dKyr_ww_wind_init() {
    if (s_wwWind == NULL) {
        return;
    }
    s_wwWind->mbHasCustomWindPower = false;
}

static void dKyr_ww_wind_move() {
    if (s_wwWind == NULL) {
        return;
    }
    WwWindEffSet* pWind = s_wwWind;
    camera_class* pCamera = dComIfGp_getCamera(0);
    cXyz* pWindVec = dKyw_get_wind_vec();
    f32 windPow = dKyw_get_wind_pow();
    if (pCamera == NULL || pWindVec == NULL) {
        return;
    }

    dBgS_ObjGndChk_All gndChk;
    cXyz windVec = *pWindVec;
    u32 particleNum = dComIfGp_particle_getParticleNum();

    // Ambient sea-streak constants (donor ambient path) — verbatim.
    f32 fVar17 = 4000.0f;
    f32 posRange = 2000.0f;
    f32 fVar28 = 80.0f;
    f32 posWindScale = 2500.0f;
    f32 fVar25 = 250.0f;
    f32 fVar18 = 800.0f;
    f32 fVar23 = 1.0f;
    f32 offsetY = 1000.0f;

    s32 windlineCount = s_wwWindlineCount;
    // TODO(Ferry W-LINE): donor forces count=10 while gliding
    // (daPyStts1_DEKU_LEAF_FLY_e, d_kankyo_rain.cpp:263) — wire when our Deku
    // Leaf glide status is one check away.

    // Adaptation 1 — NEVER take the custom-windpower branch on WW hosts.
    // Ferry F feeds ambient wind THROUGH custom_windpower (evt_wind path);
    // donor :268 would treat that as tornado/kytag02 tiny lines (scale 0.14,
    // range 160, player-anchored). Force ambient sea streaks instead.
    const bool useCustomWindPower = false;
    (void)useCustomWindPower;
    if (pWind->mbHasCustomWindPower) {
        pWind->mbHasCustomWindPower = false;
        wwWindlineDeleteEmitters(pWind);
    }

    cXyz eyePos;
    dKy_set_eyevect_calc2(pCamera, &eyePos, fVar17, fVar17);

    dPa_control_c* pa = g_dComIfG_gameInfo.play.getParticle();
    if (pa == NULL || !pa->ensureWwWindlineRes()) {
        return;
    }

    for (s32 i = 0; i < 30; i++) {
        if (i >= windlineCount && pWind->mWindEff[i].mState == 0) {
            if (pWind->mWindEff[i].mpEmitter != NULL) {
                pWind->mWindEff[i].mpEmitter->deleteAllParticle();
                pWind->mWindEff[i].mpEmitter->becomeInvalidEmitter();
                pWind->mWindEff[i].mpEmitter = NULL;
            }
            continue;
        }

        WwWindEff& windEff = pWind->mWindEff[i];
        switch (windEff.mState) {
        case 0:
            if (windPow < 0.3f) {
                continue;
            }
            if (particleNum <= 1500 &&
                ((g_Counter.mCounter0 >> 4 & 7) != (i & 3U))) {
                windEff.mStateTimer = 0.0f;
                windEff.mAlpha = 0.0f;
                windEff.field_0x2c = 0;
                windEff.mBasePos.set(eyePos);
                windEff.mBasePos.y += offsetY;

                windEff.mPos.x = cM_rndFX(posRange);
                windEff.mPos.y = cM_rndFX(posRange);
                windEff.mPos.z = cM_rndFX(posRange);

                f32 windScale = posWindScale + posWindScale * cM_rndF(1.0f);
                windEff.mPos.x -= windVec.x * windScale;
                windEff.mPos.y -= windVec.y * windScale;
                windEff.mPos.z -= windVec.z * windScale;

                windEff.field_0x2c = (s16)cM_rndF(65535.0f);

                cXyz pos;
                pos.x = windEff.mBasePos.x + windEff.mPos.x;
                pos.y = windEff.mBasePos.y + windEff.mPos.y;
                pos.z = windEff.mBasePos.z + windEff.mPos.z;

                {
                    cXyz checkPos = pos;
                    checkPos.y += 10000.0f;
                    gndChk.SetPos(&checkPos);
                    f32 gndY = dComIfG_Bgsp().GroundCross(&gndChk);
                    if (gndY != -G_CM3D_F_INF && pos.y < gndY) {
                        windEff.mPos.y =
                            (gndY + offsetY + cM_rndF(offsetY)) - windEff.mBasePos.y;
                    }
                }

                pos.x = windEff.mBasePos.x + windEff.mPos.x;
                pos.y = windEff.mBasePos.y + windEff.mPos.y;
                pos.z = windEff.mBasePos.z + windEff.mPos.z;

                windEff.mpEmitter =
                    dComIfGp_particle_set(kWwWindlineParticleId, &pos, NULL, NULL);
                if (windEff.mpEmitter != NULL) {
                    windEff.mpEmitter->setGlobalAlpha(0);
                    windEff.mpEmitter->setGlobalScale(
                        JGeometry::TVec3<f32>(fVar23, fVar23, fVar23));
                    windEff.mState = 1;
                }

                f32 windVec_absXZ =
                    std::sqrtf(windVec.x * windVec.x + windVec.z * windVec.z);
                windEff.mAngleXZ = cM_atan2s(windVec.x, windVec.z);
                windEff.mAngleY = cM_atan2s(windVec.y, windVec_absXZ);
                windEff.field_0x28 = 0;
                windEff.field_0x32 = cM_rndF(1.0f) >= 0.0f ? 1 : 0;
            }
            break;
        case 1:
        case 2: {
            if (windEff.mpEmitter == NULL) {
                windEff.mState = 0;
                break;
            }
            f32 fVar14 = fVar25 - fVar17 * (1.0f - windPow);
            windEff.field_0x2c += (s16)fVar18;
            if (i & 1) {
                windEff.mAngleY += (s16)(fVar14 * cM_ssin(windEff.field_0x2c));
                windEff.mAngleXZ += (s16)(fVar14 * cM_ssin(windEff.field_0x2c));
            } else {
                windEff.mAngleY += (s16)(fVar14 * cM_ssin(windEff.field_0x2c));
                windEff.mAngleXZ -= (s16)(fVar14 * cM_ssin(windEff.field_0x2c));
            }

            if (windEff.mStateTimer > 0.4f && windEff.field_0x32 == 1) {
                windEff.field_0x28 += i * 200 + 3600;
                windEff.mAngleY += (s16)(i * 200 + 3600);
                if (windEff.field_0x28 > 60535) {
                    windEff.field_0x32 = 0;
                }
            } else {
                f32 windVec_absXZ =
                    std::sqrtf(windVec.x * windVec.x + windVec.z * windVec.z);
                s16 targetAngleXZ = cM_atan2s(windVec.x, windVec.z);
                s16 targetAngleY = cM_atan2s(windVec.y, windVec_absXZ);
                cLib_addCalcAngleS(&windEff.mAngleY, targetAngleY, 10, 1000, 1);
                cLib_addCalcAngleS(&windEff.mAngleXZ, targetAngleXZ, 10, 1000, 1);
            }

            cXyz move;
            move.x = cM_scos(windEff.mAngleY) * cM_ssin(windEff.mAngleXZ);
            move.y = cM_ssin(windEff.mAngleY);
            move.z = cM_scos(windEff.mAngleY) * cM_scos(windEff.mAngleXZ);

            fVar14 = fVar28 - (fVar28 * 0.2f) * (1.0f - windPow);
            windEff.mPos.x += move.x * fVar14;
            windEff.mPos.y += move.y * fVar14;
            windEff.mPos.z += move.z * fVar14;

            cXyz pos;
            pos.x = windEff.mBasePos.x + windEff.mPos.x;
            pos.y = windEff.mBasePos.y + windEff.mPos.y;
            pos.z = windEff.mBasePos.z + windEff.mPos.z;
            windEff.mpEmitter->setGlobalTranslation(pos);

            // §178 W-LINE-c — Nonmatching alpha/timer block: use noclip
            // d_kankyo_wether.ts (sanctioned disambiguator). Decomp's
            // `0.18f * (i/30)` integer-divides to 0 → timer frozen → invisible.
            // DO NOT adopt noclip's effScale*=1.8 (their "noclip modification").
            const f32 maxVel = 0.08f + 0.008f * ((f32)i / 30.0f);
            f32 distFade = pos.getSquareDistance(pCamera->view.lookat.eye) / 200.0f;
            if (distFade > 1.0f) {
                distFade = 1.0f;
            }
            // Receiver: BG0_K0 lives in dungeonlight_col[1] (№113 convert stash:
            // plight_col[1]=bg0_k0). Normalize 0–255 → 0–1 like noclip's Color.
            const GXColorS10& bg0K0 = g_env_light.dungeonlight_col[1];
            const f32 colorAvg =
                ((f32)bg0K0.r + (f32)bg0K0.g + (f32)bg0K0.b) / (3.0f * 255.0f);
            f32 alphaFade = windPow * (distFade * colorAvg * colorAvg);
            if (alphaFade < 0.5f) {
                alphaFade = 0.5f;
            }
            const f32 alpha255 = alphaFade * windEff.mAlpha * 255.0f;
            windEff.mpEmitter->setGlobalAlpha(
                (u8)(alpha255 > 255.0f ? 255.0f : (alpha255 < 0.0f ? 0.0f : alpha255)));

            if (windEff.mState == 1) {
                cLib_addCalc(&windEff.mStateTimer, 1.0f, 0.3f, 0.1f * maxVel, 0.01f);
                if (windEff.mStateTimer >= 1.0f) {
                    windEff.mState = 2;
                }
                if (windEff.mStateTimer > 0.5f) {
                    cLib_addCalc(&windEff.mAlpha, 1.0f, 0.5f, 0.05f, 0.001f);
                }
            } else {
                cLib_addCalc(&windEff.mStateTimer, 0.0f, 0.4f,
                             maxVel * (0.1f + 0.01f * ((f32)i / 30.0f)), 0.01f);
                if (windEff.mStateTimer <= 0.0f) {
                    windEff.mpEmitter->deleteAllParticle();
                    windEff.mpEmitter->becomeInvalidEmitter();
                    windEff.mpEmitter = NULL;
                    windEff.mState = 0;
                }
                if (windEff.mStateTimer < 0.5f) {
                    cLib_addCalc(&windEff.mAlpha, 0.0f, 0.5f, 0.05f, 0.001f);
                }
            }
#if TARGET_PC
            // Tuning probe — strip after acceptance (§178).
            if (i == 0 && (g_Counter.mCounter0 % 30) == 0) {
                DuskLog.info("[WwWind] line0 state={} timer={:.2f} alpha={:.2f}", windEff.mState,
                             windEff.mStateTimer, windEff.mAlpha);
            }
#endif
            break;
        }
        default:
            windEff.mState = 0;
            break;
        }
    }
}

static void wether_move_windline() {
    s_wwWindlineCount = 0;
    if (wwWindlineHostActive()) {
        s_wwWindlineCount = (int)(dKyw_get_wind_pow() * 10.0f);
    }

    if (!wwWindlineHostActive() && s_wwWindInitialized) {
        dKyw_ww_windline_delete();
        return;
    }

    switch (s_wwWindInitialized ? 1 : 0) {
    case 0:
        if (s_wwWindlineCount != 0) {
            s_wwWind = JKR_NEW_ARGS(0x20) WwWindEffSet;
            if (s_wwWind == NULL) {
                return;
            }
            for (int i = 0; i < 30; i++) {
                s_wwWind->mWindEff[i].mState = 0;
                s_wwWind->mWindEff[i].mpEmitter = NULL;
            }
            dKyr_ww_wind_init();
            dKyr_ww_wind_move();
            s_wwWindInitialized = true;
            DuskLog.info("[WwWind] Ferry W-LINE ARM count={} pow={:.3f}", s_wwWindlineCount,
                         dKyw_get_wind_pow());
        }
        break;
    case 1:
        dKyr_ww_wind_move();
        break;
    }
}

void dKyw_ww_windline_delete() {
    if (!s_wwWindInitialized && s_wwWind == NULL) {
        return;
    }
    wwWindlineDeleteEmitters(s_wwWind);
    JKR_DELETE(s_wwWind);
    s_wwWind = NULL;
    s_wwWindInitialized = false;
    s_wwWindlineCount = 0;
    DuskLog.info("[WwWind] Ferry W-LINE DISARM");
}

void dKyw_wave_calm_update() {
    if (!wwFoamFieldActive() || g_env_light.mWaveChan.mWaveCount == 0) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    s_flatTarget = wwCalcFlatInterTarget(player->current.pos.x, player->current.pos.z);
    // Donor CalcFlatInter ease (counter path).
    if (s_flatCounter != 0.0f) {
        s_flatInter = s_flatInter + (s_flatTarget - s_flatInter) / s_flatCounter;
        s_flatCounter -= 1.0f;
        if (s_flatCounter < 0.0f) {
            s_flatCounter = 0.0f;
        }
    } else {
        s_flatInter = s_flatTarget;
    }
    dKy_usonami_set(s_flatInter);
}

static void wether_move_wave() {
    if (!wwFoamFieldActive()) {
        if (g_env_light.mWaveInitialized) {
            JKR_DELETE(g_env_light.mpWavePacket);
            g_env_light.mpWavePacket = NULL;
            g_env_light.mWaveInitialized = 0;
        }
        return;
    }

    dKyw_wave_calm_update();

    switch (g_env_light.mWaveInitialized) {
    case 0:
        if (g_env_light.mWaveChan.mWaveCount == 0) {
            return;
        }
        s_wwAlwaysPhaseState = dComIfG_resLoad(&s_wwAlwaysPhase, "WwAlways");
        if (s_wwAlwaysPhaseState != cPhs_COMPLEATE_e) {
            if (s_wwAlwaysPhaseState == cPhs_ERROR_e && !s_wwAlwaysWarned) {
                s_wwAlwaysWarned = true;
                DuskLog.warn("[WwFoam] §97b WwAlways resLoad ERROR — foam idle");
            }
            return;
        }
        g_env_light.mpWavePacket = JKR_NEW_ARGS(0x20) dKankyo_wave_Packet;
        if (g_env_light.mpWavePacket == NULL) {
            return;
        }
        g_env_light.mpWavePacket->mpTexUsonami =
            (u8*)dComIfG_getObjectRes("WwAlways", (int)kWwAlwaysUsonami);
        g_env_light.mpWavePacket->mpTexUsonamiM =
            (u8*)dComIfG_getObjectRes("WwAlways", (int)kWwAlwaysUsonamiM);
        if (g_env_light.mpWavePacket->mpTexUsonami == NULL ||
            g_env_light.mpWavePacket->mpTexUsonamiM == NULL) {
            DuskLog.warn("[WwFoam] §97b USONAMI BTI missing (0x8B/0x8C)");
            JKR_DELETE(g_env_light.mpWavePacket);
            g_env_light.mpWavePacket = NULL;
            return;
        }
        for (int i = 0; i < 300; i++) {
            g_env_light.mpWavePacket->mEff[i].mStatus = 0;
        }
        g_env_light.mpWavePacket->mSkewWidth = 0.0f;
        g_env_light.mpWavePacket->mSkewDir = 0.0f;
        wave_move();
        g_env_light.mWaveInitialized = 1;
        DuskLog.info("[WwFoam] §97b packet ready count={}",
                     (int)g_env_light.mWaveChan.mWaveCount);
        break;
    case 1:
        if (g_env_light.mWaveChan.mWaveCount == 0) {
            g_env_light.mWaveInitialized = 0;
            JKR_DELETE(g_env_light.mpWavePacket);
            g_env_light.mpWavePacket = NULL;
            return;
        }
        wave_move();
        break;
    }
}
#endif

void dKyw_wether_move_draw() {
    g_env_light.moya_se = 0;

    if (strcmp(dComIfGp_getStartStageName(), "Name")) {
        wether_move_sun();
        wether_move_rain();
        wether_move_snow();
    }
    wether_move_star();

    if (strcmp(dComIfGp_getStartStageName(), "Name")) {
        wether_move_housi();
        wether_move_moya();
        wether_move_mud();
        wether_move_evil();
        wether_move_odour();
#if TARGET_PC
        wether_move_wave();
#endif
    }
}

void dKyw_wether_move_draw2() {
    wether_move_vrkumo();
}

void dKyw_wether_draw() {
    if (strcmp(dComIfGp_getStartStageName(), "Name") && g_env_light.mCloudInitialized != 0) {
        dKyw_Cloud_Draw();
    }

    if (strcmp(dComIfGp_getStartStageName(), "Name") && g_env_light.mSunInitialized) {
        stage_stag_info_class* stag_info = dComIfGp_getStageStagInfo();

        if (dStage_stagInfo_GetArg0(stag_info) != 0) {
            dKyw_Sun_Draw();
            dKyw_Sunlenz_Draw();
        }
    }

    if (g_env_light.mStarInitialized) {
        dKyw_Star_Draw();
        dKyw_shstar_Draw();
    }

    if (strcmp(dComIfGp_getStartStageName(), "Name")) {
        if (g_env_light.mRainInitialized) {
            dKyw_Rain_Draw();
        }

        if (g_env_light.mSnowInitialized) {
            dKyw_Snow_Draw();
        }

        if (g_env_light.mHousiInitialized) {
            dKyw_Housi_Draw();
        }

        if (g_env_light.mOdourData.mOdourPacketStatus != 0) {
            dKyw_Odour_Draw();
        }

        if (g_env_light.mMudInitialized != 0) {
            dKyw_mud_Draw();
        }

        if (g_env_light.mEvilInitialized != 0) {
            dKyw_evil_Draw();
        }

#if TARGET_PC
        if (g_env_light.mWaveInitialized != 0) {
            dKyw_Wave_Draw();
        }
#endif

        dKy_undwater_filter_draw();
    }
}

void dKyw_wether_draw2() {
    if (g_env_light.mVrkumoStatus != 0 && g_env_light.mVrkumoStatus < 99) {
        dKyw_Vrkumo_Draw();
    }
}

void dKyw_wether_proc() {
    // Stage is Faron Woods or Fishing Pond or Hyrule Field
    if (!strcmp(dComIfGp_getStartStageName(), "F_SP108") ||
        !strcmp(dComIfGp_getStartStageName(), "F_SP127") ||
        (!strcmp(dComIfGp_getStartStageName(), "F_SP121") &&
         g_env_light.dice_wether_time != 0.0f))
    {
        if (!dKy_darkworld_check()) {
            // Stage is Hyrule Field
            if (!strcmp(dComIfGp_getStartStageName(), "F_SP121") ||
                !(g_env_light.daytime >= 75.0f) || !(g_env_light.daytime <= 120.0f))
            {
                // Stage is Hyrule Field
                if (!strcmp(dComIfGp_getStartStageName(), "F_SP121") &&
                    g_env_light.dice_wether_mode >= 1 && g_env_light.dice_wether_mode < 6)
                {
                    dKy_get_dayofweek();
lbl1:
                    cLib_addCalc(&g_env_light.field_0xebc, 1.0f, 0.05f, 0.0025f, 0.000001f);
                    g_env_light.mMoyaMode = 7;
                    g_env_light.mMoyaCount = 49;
                } else {
                    cLib_addCalc(&g_env_light.field_0xebc, 0.0f, 0.05f, 0.0025f, 0.00001f);
                    if (g_env_light.field_0xebc <= 0.01f && g_env_light.mMoyaCount == 49) {
                        g_env_light.mMoyaCount = 0;
                    }
                }
            } else {
                goto lbl1;  // maybe fake match
            }
        }
    }

    // Stage is Fishing Pond
    if (!strcmp(dComIfGp_getStartStageName(), "F_SP127") &&
        (g_env_light.fishing_hole_season == 1 || g_env_light.fishing_hole_season == 3))
    {
        if (g_env_light.fishing_hole_season == 1) {
            g_env_light.mHousiCount = 35;
        } else {
            g_env_light.mHousiCount = 30;
        }
        g_env_light.field_0xea9 = 2;
    }
    squal_proc();
}

static void dKyw_wind_init() {
    g_env_light.global_wind_influence.vec.x = -1.0f;
    g_env_light.global_wind_influence.vec.y = 0.0f;
    g_env_light.global_wind_influence.vec.z = 0.0f;
    g_env_light.global_wind_influence.pow = 0.0f;
    g_env_light.unk_0xe6c = 0;
    g_env_light.unk_0xe64 = 0;
    g_env_light.unk_0xe66 = 0;
    g_env_light.global_wind_influence.vec_override = NULL;
    g_env_light.custom_windpower = 0.0f;
    g_env_light.evt_wind_go = 0;
    g_env_light.TeachWind_existence = 0;
}

void dKyw_wind_set() {
    s16 var_r30;
    s16 var_r29;
    int var_r28 = 0;

    cXyz wind_vec;
    f32 strength;

    if (g_env_light.global_wind_influence.vec_override != NULL) {
        wind_vec = *g_env_light.global_wind_influence.vec_override;
        strength = g_env_light.custom_windpower;

        cM_atan2s(wind_vec.x, wind_vec.z);
        cM_atan2s(wind_vec.absXZ(), wind_vec.y);
    } else {
        dStage_FileList_dt_c* fili_p = NULL;
        int wind_level = 0;

        if (dComIfGp_roomControl_getStayNo() >= 0) {
            fili_p = dComIfGp_roomControl_getStatusRoomDt(dComIfGp_roomControl_getStayNo())
                         ->getFileListInfo();
        }

        var_r30 = 0;
        var_r29 = 0;
        if (fili_p != NULL) {
            var_r28 = dStage_FileList_dt_GlobalWindDir(fili_p);
        }

        if (dComIfGp_roomControl_getStatusRoomDt(dComIfGp_roomControl_getStayNo()) != NULL) {
            dStage_Lbnk_c* lbnk_p =
                dComIfGp_roomControl_getStatusRoomDt(dComIfGp_roomControl_getStayNo())->getLbnk();
            if (lbnk_p != NULL) {
                dStage_Lbnk_dt_c* data_p = lbnk_p->entries;

                if (dStage_lbnkWIND(&data_p[dComIfG_play_c::getLayerNo(0)]) != 7) {
                    var_r28 = dStage_lbnkWIND(&data_p[0]);
                }
            }
        }

        switch (var_r28) {
        case 0:
        case 1:
            break;
        case 2:
            var_r30 = 0;
            var_r29 = -0x4000;
            break;
        case 3:
            var_r30 = 0;
            var_r29 = 0;
            break;
        case 4:
            var_r30 = 0;
            var_r29 = 0x4000;
            break;
        case 5:
            var_r30 = 0;
            var_r29 = 0x7FFF;
            break;
        }

        if (g_env_light.evt_wind_go != 0 && g_env_light.evt_wind_go != 0xFF) {
            var_r30 = g_env_light.evt_wind_angle_x;
            var_r29 = g_env_light.evt_wind_angle_y;
        }

        wind_vec.x = cM_scos(var_r30) * cM_ssin(var_r29);
        wind_vec.y = cM_ssin(var_r30);
        wind_vec.z = cM_scos(var_r30) * cM_scos(var_r29);

        if (fili_p != NULL) {
            wind_level = dStage_FileList_dt_GlobalWindLevel(fili_p);
        }

        if (dComIfGp_roomControl_getStatusRoomDt(dComIfGp_roomControl_getStayNo()) != NULL) {
            dStage_Lbnk_c* lbnk_p =
                dComIfGp_roomControl_getStatusRoomDt(dComIfGp_roomControl_getStayNo())->getLbnk();
            if (lbnk_p != NULL) {
                dStage_Lbnk_dt_c* data_p = lbnk_p->entries;

                if (dStage_lbnkWlevel(&data_p[0]) != 3) {
                    wind_level = dStage_lbnkWlevel(&data_p[0]);
                }
            }
        }

        switch (wind_level) {
        case 0:
            strength = 0.3f;
            break;
        case 1:
            strength = 0.6f;
            break;
        case 2:
            strength = 0.9f;
            break;
        default:
            strength = 0.0f;
            break;
        }
    }

    if (g_env_light.evt_wind_go != 0) {
        strength = g_env_light.custom_windpower;
    }

    if (strength > 1.0f) {
        strength = 1.0f;
    }

    if (strcmp(dComIfGp_getStartStageName(), "D_MN07") == 0 &&
        (dComIfGp_roomControl_getStayNo() == 0 || dComIfGp_roomControl_getStayNo() == 3 ||
         dComIfGp_roomControl_getStayNo() == 6 || dComIfGp_roomControl_getStayNo() == 13))
    {
        mDoAud_mEnvse_setWindDirection(&wind_vec);
        if (strength <= 0.5f) {
            mDoAud_mEnvse_setWindType(3);
        } else {
            mDoAud_mEnvse_setWindType(1);
        }

        s8 reverb = dComIfGp_getReverb(dComIfGp_roomControl_getStayNo());
        mDoAud_mEnvse_startStrongWindSe(reverb);
    }

    if (g_env_light.light_init_timer != 0) {
        g_env_light.global_wind_influence.vec = wind_vec;
        g_env_light.global_wind_influence.pow = strength;
    } else {
        cLib_addCalc(&g_env_light.global_wind_influence.vec.x, wind_vec.x, 0.05f, 2.0f, 0.001f);
        cLib_addCalc(&g_env_light.global_wind_influence.vec.y, wind_vec.y, 0.05f, 2.0f, 0.001f);
        cLib_addCalc(&g_env_light.global_wind_influence.vec.z, wind_vec.z, 0.05f, 2.0f, 0.001f);
        cLib_addCalc(&g_env_light.global_wind_influence.pow, strength, 0.05f, 1.0f, 0.005f);
    }
}

cXyz* dKyw_get_wind_vec() {
    return &g_env_light.global_wind_influence.vec;
}

f32 dKyw_get_wind_pow() {
    return g_env_light.global_wind_influence.pow;
}

cXyz dKyw_get_wind_vecpow() {
    cXyz vec = g_env_light.global_wind_influence.vec * g_env_light.global_wind_influence.pow;
    return vec;
}

void dKyw_plight_collision_set(cXyz* param_0, s16 param_1, s16 param_2, f32 param_3, f32 param_4,
                               f32 param_5, f32 param_6, f32 param_7) {
    dScnKy_env_light_c* env_light = dKy_getEnvlight();

    for (int i = 0; i < 5; i++) {
        if (!env_light->wind_inf_entity[i].mInUse) {
            env_light->wind_inf_entity[i].mInUse = true;
            env_light->wind_inf_entity[i].mMinRadius = param_4;
            env_light->wind_inf_entity[i].mSpeed = param_6;
            env_light->wind_inf_entity[i].mStrengthMaxVel = param_7;

            WIND_INFLUENCE* wind_inf = &env_light->wind_inf_entity[i].mInfluence;
            wind_inf->position = *param_0;
            wind_inf->mDirection.x = cM_scos(param_1) * cM_ssin(param_2);
            wind_inf->mDirection.y = cM_ssin(param_1);
            wind_inf->mDirection.z = cM_scos(param_1) * cM_scos(param_2);
            wind_inf->mRadius = param_3;
            wind_inf->field_0x20 = wind_inf->mRadius;
            wind_inf->mStrength = param_5;

            dKyw_pntlight_set(wind_inf);
            break;
        }
    }
}

static void squal_proc() {
    dScnKy_env_light_c* env_light = dKy_getEnvlight();
    WIND_INFLUENCE* influence;

    for (int i = 0; i < 5; i++) {
        influence = &env_light->wind_inf_entity[i].mInfluence;
        switch (env_light->wind_inf_entity[i].mInUse) {
        case 0:
            break;
        case 1:
            influence->position.x +=
                influence->mDirection.x * env_light->wind_inf_entity[i].mSpeed;
            influence->position.y +=
                influence->mDirection.y * env_light->wind_inf_entity[i].mSpeed;
            influence->position.z +=
                influence->mDirection.z * env_light->wind_inf_entity[i].mSpeed;

            cLib_addCalc(&influence->mStrength, 0.0f, 0.2f,
                         env_light->wind_inf_entity[i].mStrengthMaxVel, 0.001f);

            f32 speed = 1.0f - influence->mStrength;
            f32 target = env_light->wind_inf_entity[i].mMinRadius;
            cLib_addCalc(&influence->mRadius, target, speed, speed * target * 0.05f, 0.01f);

            if (influence->mStrength < 0.01f) {
                dKyw_pntwind_cut(influence);
                env_light->wind_inf_entity[i].mInUse = false;
            }
            break;
        }
    }
}

static void dKyw_pntwind_init() {
    for (int i = 0; i < 30; i++) {
        g_env_light.pntwind[i] = NULL;
    }

    for (int i = 0; i < 5; i++) {
        g_env_light.wind_inf_entity[i].mInUse = 0;
    }
}

static void pntwind_set(WIND_INFLUENCE* i_pntwind) {
    int i = 0;
    for (; i < 30; i++) {
        if (g_env_light.pntwind[i] == NULL) {
            g_env_light.pntwind[i] = i_pntwind;
            g_env_light.pntwind[i]->field_0x24 = i;
            break;
        }
    }

    if (i >= 30) {
        i_pntwind->field_0x24 = 9999;
    }
}

void dKyw_pntwind_set(WIND_INFLUENCE* i_pntwind) {
    i_pntwind->mConstant = 0;
    i_pntwind->field_0x29 = 0;
    pntwind_set(i_pntwind);
}

static void dKyw_pntlight_set(WIND_INFLUENCE* i_pntwind) {
    i_pntwind->mConstant = 0;
    i_pntwind->field_0x29 = 1;
    pntwind_set(i_pntwind);
}

void dKyw_pntwind_cut(WIND_INFLUENCE* i_pntwind) {
    if (i_pntwind != NULL && i_pntwind->field_0x24 >= 0 && i_pntwind->field_0x24 < 30) {
        g_env_light.pntwind[i_pntwind->field_0x24] = NULL;
    }
}

static void pntwind_get_info(cXyz* param_0, cXyz* i_dir, f32* i_power, u8 param_3) {
    i_dir->x = 0.0f;
    i_dir->y = 0.0f;
    i_dir->z = 0.0f;
    *i_power = 0.0f;

    WIND_INFLUENCE* influence;
    s32 influence_count = ARRAY_SIZEU(g_env_light.pntwind);
    for (int i = 0; i < influence_count; i++) {
        influence = g_env_light.pntwind[i];
        if (influence != NULL && influence->field_0x29 == param_3) {
            f32 dist = param_0->abs(influence->position);

            if (influence->mConstant == 0) {
                if (dist < influence->mRadius && influence->mStrength > 0.0f && dist != 0.0f) {
                    f32 temp_f0;
                    if (influence->mRadius > 0.0f) {
                        f32 temp_f0_4 = (dist / influence->mRadius);
                        temp_f0 = 1.0f - (temp_f0_4 * temp_f0_4);
                    } else {
                        temp_f0 = 1.0f;
                    }

                    *i_power = influence->mStrength * temp_f0;

                    cXyz sp10;
                    sp10.x =
                        influence->position.x + -(influence->mDirection.x) * influence->mRadius;
                    sp10.y =
                        influence->position.y + -(influence->mDirection.y) * influence->mRadius;
                    sp10.z =
                        influence->position.z + -(influence->mDirection.z) * influence->mRadius;
                    dKyr_get_vectle_calc(&sp10, param_0, i_dir);

                    if (param_0->abs(sp10) < influence->mRadius) {
                        i_dir->x = 0.0f;
                        i_dir->y = 0.0f;
                        i_dir->z = 0.0f;
                    }
                    return;
                }
            } else {
                if (dist < influence->mRadius && influence->mStrength > 0.0f && dist != 0.0f) {
                    f32 temp_f0;
                    if (influence->mRadius > 0.0f) {
                        temp_f0 = 1.0f - (dist / influence->mRadius);
                    } else {
                        temp_f0 = 1.0f;
                    }

                    i_dir->x = influence->mDirection.x;
                    i_dir->y = influence->mDirection.y;
                    i_dir->z = influence->mDirection.z;
                    *i_power = influence->mStrength * temp_f0;
                    return;
                }
            }
        }
    }
}

static void dKyw_pntwind_get_info(cXyz* param_0, cXyz* param_1, f32* param_2) {
    pntwind_get_info(param_0, param_1, param_2, 0);
}

void dKyw_pntlight_collision_get_info(cXyz* param_0, cXyz* param_1, f32* param_2) {
    pntwind_get_info(param_0, param_1, param_2, 1);
}

cXyz dKyw_pntwind_get_vecpow(cXyz* param_0) {
    cXyz direction;
    f32 power;
    dKyw_pntwind_get_info(param_0, &direction, &power);

    direction = direction * power;
    return direction;
}

void dKyw_get_AllWind_vec(cXyz* i_position, cXyz* i_direction, f32* i_power) {
    dScnKy_env_light_c* env_light = dKy_getEnvlight();
    dKyw_pntwind_get_info(i_position, i_direction, i_power);

    cXyz sp54;
    cXyz sp30;
    cXyz sp24;

    sp30 = env_light->global_wind_influence.vec * (env_light->global_wind_influence.pow * (1.0f - *i_power));
    sp24 = *i_direction * (*i_power * 5.0f);
    sp54 = sp30 + sp24;
    *i_power = sp54.abs();

    sp54 = sp54.normZP();
    if (sp54 != cXyz::Zero) {
        i_direction->x = sp54.x;
        i_direction->y = sp54.y;
        i_direction->z = sp54.z;
    } else {
        i_direction->x = env_light->global_wind_influence.vec.x;
        i_direction->y = env_light->global_wind_influence.vec.y;
        i_direction->z = env_light->global_wind_influence.vec.z;
    }
}

cXyz dKyw_get_AllWind_vecpow(cXyz* param_0) {
    f32 sp8;
    cXyz spC;
    cXyz sp18;
    cXyz sp24;
    cXyz sp30;

    dKyw_pntwind_get_info(param_0, &sp30, &sp8);
    sp18 = g_env_light.global_wind_influence.vec * (g_env_light.global_wind_influence.pow * (1.0f - sp8));
    sp24 = sp30 * (5.0f * sp8);
    spC = sp18 + sp24;

    return spC;
}

void dKyw_custom_windpower(f32 pow) {
    g_env_light.custom_windpower = pow;
}

void dKyw_evt_wind_set(s16 angleX, s16 angleY) {
    g_env_light.evt_wind_go = 1;
    g_env_light.evt_wind_angle_x = angleX;
    g_env_light.evt_wind_angle_y = angleY;
}

void dKyw_evt_wind_set_go() {
    g_env_light.evt_wind_go = 1;
}

void dKyw_rain_set(int count) {
    g_env_light.raincnt = count;
    g_env_light.base_raincnt = count;
}
