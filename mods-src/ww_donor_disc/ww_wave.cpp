// Plugin whitecap foam (system 2 / §97b) — phase 1.
// Donor wave_move + drawWave; plugin-owned channel (vanilla env_light ends
// at 0x1310). See ww_wave.h.

#include "ww_wave.h"

#include "ww_kankyo.h"
#include "ww_kankyo_wind.h"
#include "ww_sky.h"

#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_m3d.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_xyz.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_data.h"
#include "d/d_kankyo_rain.h"
#include "dolphin/gx.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_camera_mng.h"
#include "m_Do/m_Do_graphic.h"

#include <mods/api.h>
#include <mods/svc/log.h>

#include "d/d_stage.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD(degrees) ((degrees) * (M_PI / 180.0))
#endif

extern const LogService* s_log;
extern ModContext* mod_ctx;

namespace {

void waveLog(const char* fmt, ...) {
    if (s_log == nullptr) {
        return;
    }
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    s_log->write(mod_ctx, LOG_LEVEL_INFO, buf);
}

static u8 clampU8(s16 v) {
    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return (u8)v;
}

// Donor Always indices (GZLE01 Always.h).
static const int kWwTexUsonami = 0x8B;
static const int kWwTexUsonamiM = 0x8C;

struct WwWaveChan {
    f32 mWaveSpeed;
    f32 mWaveSpawnDist;
    f32 mWaveSpawnRadius;
    f32 mWaveScale;
    f32 mWaveScaleRand;
    f32 mWaveCounterSpeedScale;
    f32 mWaveScaleBottom;
    f32 mWaveFlatInter;
    s16 mWaveCount;
    u8 mWaveReset;
};

struct WwWAVE_EFF {
    cXyz mPos;
    cXyz mBasePos;
    f32 mSpeed;
    f32 mScale;
    f32 mCounterSpeed;
    f32 mCounter;
    f32 mAlpha;
    f32 mStrengthEnv;
    s16 field_0x30;
    s16 field_0x32;
    s8 mStatus;
};

class dKyWwWave_Packet : public J3DPacket {
public:
    virtual int entry(J3DDrawBuffer*) { return 1; }
    virtual void draw();
    virtual ~dKyWwWave_Packet() {}

    u8* mpTexUsonami;
    u8* mpTexUsonamiM;
    WwWAVE_EFF mEff[300];
    f32 mSkewWidth;
    f32 mSkewDir;
};

static dKyWwWave_Packet* s_pkt = NULL;
static WwWaveChan s_chan;
static bool s_armed = false;
static bool s_live = false;

// Donor daSea WaterHeightInfo + CalcFlatInterTarget (MULT mWaveMax).
// Coast kill: live ky_tag1 → wwWave_registerInfl (donor mpWaveInfl).
// TP Mult_info is 0xC with pad at 0xB; WW stores mWaveMax there.
// Read room/wave bytes directly — no BE needed for u8 fields.
struct WwMult_info {
    u8 raw[0xC];
    u8 roomNo() const { return raw[0xA]; }
    u8 waveMax() const { return raw[0xB]; }
};
static_assert(sizeof(WwMult_info) == 0xC, "WW MULT entry size");

static constexpr f32 kWaveCalmRamp = 12800.0f;
static constexpr int kWaveInflMax = 10;
static constexpr int kHeightN = 9;
static u8 s_height[kHeightN][kHeightN];
static bool s_heightReady = false;
static WwWaveInfl* s_waveInflPtrs[kWaveInflMax] = {};
static f32 s_flatInter = 1.0f;
static f32 s_flatTarget = 1.0f;
static f32 s_flatCounter = 0.0f;
static bool s_calmLoaded = false;
static bool s_calmFromMult = false;
static bool s_seaWindArmed = false;
static const s8 kPosAround[8][2] = {
    {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0},
};

// Sky owns the WwAlways phase (wwSkyPollAlways). Donor wether_move_wave
// assumes Always is already resident and only getObjectRes's USONAMI.
// Plugin translation: wait for wwSky_alwaysReady(); do not resLoad here.
// Foam only consumes.

static void wwWaveInitBtitex(GXTexObj* i_obj, ResTIMG* i_img) {
    GXInitTexObj(i_obj, ((u8*)&i_img->format) + i_img->imageOffset, i_img->width, i_img->height,
                 (GXTexFmt)i_img->format, (GXTexWrapMode)i_img->wrapS,
                 (GXTexWrapMode)i_img->wrapT, (GXBool)(i_img->mipmapCount > 1));
}

static void wwWaveSetBtitex(GXTexObj* i_obj, ResTIMG* i_img) {
    wwWaveInitBtitex(i_obj, i_img);
    GXInitTexObjLOD(i_obj, (GXTexFilter)i_img->minFilter, (GXTexFilter)i_img->magFilter,
                    i_img->minLOD * 0.125f, i_img->maxLOD * 0.125f, i_img->LODBias * 0.01f,
                    (GXBool)i_img->biasClamp, (GXBool)i_img->doEdgeLOD,
                    (GXAnisotropy)i_img->maxAnisotropy);
    GXLoadTexObj(i_obj, GX_TEXMAP0);
}

static void wwWaveVectle(DOUBLE_POS* i_pos, cXyz* o_out) {
    f64 len = std::sqrt(i_pos->x * i_pos->x + i_pos->y * i_pos->y + i_pos->z * i_pos->z);
    if (len != 0.0) {
        o_out->x = (f32)(i_pos->x / len);
        o_out->y = (f32)(i_pos->y / len);
        o_out->z = (f32)(i_pos->z / len);
    } else {
        o_out->x = 0.0f;
        o_out->y = 0.0f;
        o_out->z = 0.0f;
    }
}

static void wwWaveGetVectle(cXyz* a, cXyz* b, cXyz* o) {
    DOUBLE_POS pos;
    pos.x = b->x - a->x;
    pos.y = b->y - a->y;
    pos.z = b->z - a->z;
    wwWaveVectle(&pos, o);
}

static void wwWaveGetSeacolor(GXColor* amb, GXColor* dif) {
    u8 a[4], d[4];
    wwKankyo_getSeacolor(a, d);
    amb->r = a[0];
    amb->g = a[1];
    amb->b = a[2];
    amb->a = a[3];
    dif->r = d[0];
    dif->g = d[1];
    dif->b = d[2];
    dif->a = d[3];
}

static bool wwWaveStageIsSea() {
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && std::strcmp(stage, "sea") == 0;
}

static void wwWaveCalmClear() {
    // Do not wipe live kytag infl pointers — actors own those until Delete.
    s_heightReady = false;
    s_calmFromMult = false;
    for (int z = 0; z < kHeightN; z++) {
        for (int x = 0; x < kHeightN; x++) {
            s_height[z][x] = 10;
        }
    }
    s_flatInter = 1.0f;
    s_flatTarget = 1.0f;
    s_flatCounter = 0.0f;
    s_calmLoaded = false;
}

static int wwPos2Index(f32 v) {
    return (int)((v + 450000.0f) / 100000.0f);
}

static void wwCalcMinMax(int v, f32* min, f32* max) {
    *min = (f32)v * 100000.0f - 450000.0f;
    *max = *min + 100000.0f;
}

static int wwGetWaveMax(int roomNo) {
    dStage_Multi_c* multi = dComIfGp_getMulti();
    if (multi == NULL) {
        return 10;
    }
    WwMult_info* entry = reinterpret_cast<WwMult_info*>(
        static_cast<dStage_Mult_info*>(multi->m_entries));
    if (entry == NULL) {
        return 10;
    }
    const int n = multi->num;
    for (int i = 0; i < n; i++, entry++) {
        if (roomNo == (int)entry->roomNo()) {
            return (int)entry->waveMax();
        }
    }
    return 10;
}

static int wwGetHeight(int x, int z) {
    if (x < 0 || x >= kHeightN || z < 0 || z >= kHeightN) {
        return 10;
    }
    return (int)s_height[z][x];
}

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

// Donor daSea_packet_c::CalcFlatInterTarget — height==0 cells + 12800 ramp.
static f32 wwCalcFlatInterTarget(f32 x, f32 z) {
    if (!s_heightReady) {
        return 1.0f;
    }
    const int mIdxX = wwPos2Index(x);
    const int mIdxZ = wwPos2Index(z);
    if (wwGetHeight(mIdxX, mIdxZ) == 0) {
        return 0.0f;
    }
    f32 result = 1.0f;
    for (int i = 0; i < 8; i++) {
        const int ix = mIdxX + kPosAround[i][0];
        const int iz = mIdxZ + kPosAround[i][1];
        if (wwGetHeight(ix, iz) == 0) {
            f32 minX, minZ, maxX, maxZ;
            wwCalcMinMax(ix, &minX, &maxX);
            wwCalcMinMax(iz, &minZ, &maxZ);
            minX -= kWaveCalmRamp;
            minZ -= kWaveCalmRamp;
            maxX += kWaveCalmRamp;
            maxZ += kWaveCalmRamp;
            f32 len = wwCalmBoxLen(minX, maxX, minZ, maxZ, x, z);
            if (len > kWaveCalmRamp) {
                len = kWaveCalmRamp;
            }
            len /= kWaveCalmRamp;
            if (result > len) {
                result = len;
            }
        }
    }
    return result;
}

// Donor daSea_WaterHeightInfo_Mng::SetInf — 7×7 interior rooms 1..N.
static bool wwWaveLoadMultWaveMax() {
    dStage_Multi_c* multi = dComIfGp_getMulti();
    if (multi == NULL || static_cast<dStage_Mult_info*>(multi->m_entries) == NULL) {
        return false;
    }
    for (int z = 0; z < kHeightN; z++) {
        for (int x = 0; x < kHeightN; x++) {
            s_height[z][x] = 10;
        }
    }
    int roomNo = 1;
    int calmCells = 0;
    int r44 = -1;
    for (int i = 1; i < kHeightN - 1; i++) {
        for (int j = 1; j < kHeightN - 1; j++) {
            const int wm = wwGetWaveMax(roomNo);
            s_height[i][j] = (u8)wm;
            if (wm == 0) {
                calmCells++;
            }
            if (roomNo == 44) {
                r44 = wm;
            }
            roomNo++;
        }
    }
    s_heightReady = true;
    s_calmLoaded = true;
    s_calmFromMult = true;
    s_flatCounter = 150.0f;
    waveLog("[WwFoam] {\"ev\":\"wave_max\",\"mult_num\":%d,\"calm_cells\":%d,"
            "\"room44\":%d,\"reads\":\"daSea SetInf + MULT mWaveMax@0xB\"}",
            (int)multi->num, calmCells, r44);
    return true;
}

static void wwWaveEnsureCalm() {
    if (s_calmFromMult) {
        return;
    }
    if (wwWaveLoadMultWaveMax()) {
        return;
    }
    if (s_calmLoaded) {
        return;
    }
    // MULT not live yet — room-44 GetArea cell only until MULT binds.
    // No invented coast infls; ky_tag1 registers live.
    for (int z = 0; z < kHeightN; z++) {
        for (int x = 0; x < kHeightN; x++) {
            s_height[z][x] = 10;
        }
    }
    s_height[7][2] = 0;  // room 44 → grid (x=2,z=7)
    s_heightReady = true;
    s_calmLoaded = true;
    s_flatCounter = 150.0f;
    waveLog("[WwFoam] {\"ev\":\"wave_max_fallback\",\"room44_cell\":1,"
            "\"reads\":\"GetArea(-250000,-150000,250000,350000) only\"}");
}

static void wwWaveUsonamiSet(f32 flatInter) {
    if (s_chan.mWaveCount < 200) {
        s_chan.mWaveSpawnDist = 20000.0f;
        s_chan.mWaveSpawnRadius = 22000.0f;
        s_chan.mWaveReset = 0;
        s_chan.mWaveScale = 300.0f;
        s_chan.mWaveScaleRand = 0.001f;
        s_chan.mWaveCounterSpeedScale = 1.2f;
        s_chan.mWaveScaleBottom = 6.0f;
        s_chan.mWaveCount = 300;
        s_chan.mWaveSpeed = 30.0f;
    }
    s_chan.mWaveFlatInter = flatInter;
}

static void wwWaveCalmUpdate() {
    if (!s_calmLoaded || s_chan.mWaveCount == 0) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    s_flatTarget = wwCalcFlatInterTarget(player->current.pos.x, player->current.pos.z);
    if (s_flatCounter != 0.0f) {
        s_flatInter = s_flatInter + (s_flatTarget - s_flatInter) / s_flatCounter;
        s_flatCounter -= 1.0f;
        if (s_flatCounter < 0.0f) {
            s_flatCounter = 0.0f;
        }
    } else {
        s_flatInter = s_flatTarget;
    }
    wwWaveUsonamiSet(s_flatInter);
    static int s_flatLog = 0;
    if ((++s_flatLog % 300) == 1) {
        int inflN = 0;
        for (int i = 0; i < kWaveInflMax; i++) {
            if (s_waveInflPtrs[i] != NULL) {
                inflN++;
            }
        }
        waveLog("[WwFoam] {\"ev\":\"flat_inter\",\"flat\":%.3f,\"target\":%.3f,"
                "\"idx\":[%d,%d],\"h\":%d,\"infl\":%d}",
                s_flatInter, s_flatTarget, wwPos2Index(player->current.pos.x),
                wwPos2Index(player->current.pos.z),
                wwGetHeight(wwPos2Index(player->current.pos.x),
                            wwPos2Index(player->current.pos.z)),
                inflN);
    }
}

static void wwWaveArmChan() {
    // Donor dKy_usonami_set when count < 200 (Outset / open-sea defaults).
    // flatInter comes from calm update (0=near island cell, 1=open chop off).
    s_chan.mWaveSpawnDist = 20000.0f;
    s_chan.mWaveSpawnRadius = 22000.0f;
    s_chan.mWaveReset = 0;
    s_chan.mWaveScale = 300.0f;
    s_chan.mWaveScaleRand = 0.001f;
    s_chan.mWaveCounterSpeedScale = 1.2f;
    s_chan.mWaveScaleBottom = 6.0f;
    s_chan.mWaveCount = 300;
    s_chan.mWaveSpeed = 30.0f;
    s_chan.mWaveFlatInter = s_flatInter;
}

static void wwWaveMoveBody() {
    if (s_pkt == NULL || s_chan.mWaveFlatInter >= 1.0f || s_chan.mWaveCount <= 0) {
        return;
    }

    camera_class* pCamera = (camera_class*)dComIfGp_getCamera(0);
    fopAc_ac_c* pPlayer = dComIfGp_getPlayer(0);
    if (pCamera == NULL || dComIfGd_getView() == NULL) {
        return;
    }

    f32 seaLevel = dKyWw_filiSeaLevel();
    if (!(seaLevel == seaLevel) || seaLevel > 10000.0f || seaLevel < -10000.0f) {
        seaLevel = 0.0f;
    }

    cXyz eyevect;
    dKy_set_eyevect_calc2(pCamera, &eyevect, s_chan.mWaveSpawnDist, 0.0f);

    cXyz windPowVec = dKyWw_get_wind_vecpow();
    cXyz* windVecP = dKyWw_get_wind_vec();
    f32 windPow = dKyWw_get_wind_pow();
    cXyz windPowVec2 = windVecP != NULL ? *windVecP : windPowVec;

    DOUBLE_POS deltaXZ;
    deltaXZ.x = pCamera->view.lookat.center.x - pCamera->view.lookat.eye.x;
    deltaXZ.y = 0.0;
    deltaXZ.z = pCamera->view.lookat.center.z - pCamera->view.lookat.eye.z;
    cXyz vectle;
    wwWaveVectle(&deltaXZ, &vectle);

    s_pkt->mSkewDir = cM3d_VectorProduct2d(0.0f, 0.0f, -windPowVec2.x, -windPowVec2.z, vectle.x,
                                           vectle.z);
    s_pkt->mSkewWidth = windPow * (1.0f - std::fabs(windPowVec2.y)) *
                        (1.0f - std::fabs(windPowVec2.x * vectle.x + windPowVec2.z * vectle.z));
    s_pkt->mSkewWidth *= 0.6f * std::fabs(s_pkt->mSkewDir);

    for (s32 i = 0; i < s_chan.mWaveCount; i++) {
        if (s_chan.mWaveReset) {
            s_pkt->mEff[i].mStatus = 0;
        }

        switch (s_pkt->mEff[i].mStatus) {
        case 0: {
            s_pkt->mEff[i].mBasePos.x = eyevect.x;
            s_pkt->mEff[i].mBasePos.y = seaLevel;
            s_pkt->mEff[i].mBasePos.z = eyevect.z;
            s_pkt->mEff[i].mPos.x = cM_rndFX(s_chan.mWaveSpawnRadius);
            s_pkt->mEff[i].mPos.y = 0.0f;
            s_pkt->mEff[i].mPos.z = cM_rndFX(s_chan.mWaveSpawnRadius);
            s_pkt->mEff[i].mCounter = cM_rndF(65536.0f);
            s_pkt->mEff[i].mAlpha = 0.0f;
            s_pkt->mEff[i].field_0x32 = (s16)cM_rndF(65536.0f);
            s_pkt->mEff[i].mStrengthEnv = 1.0f;
            s_pkt->mEff[i].mScale = s_chan.mWaveScaleRand + cM_rndF(1.0f - s_chan.mWaveScaleRand);
            s_pkt->mEff[i].mSpeed = s_pkt->mEff[i].mScale;
            s_pkt->mEff[i].mCounterSpeed =
                ((1.0f - s_pkt->mEff[i].mScale) * 0.05f + 0.02f) * s_chan.mWaveCounterSpeedScale;
            s_pkt->mEff[i].field_0x30 = 0;
            s_pkt->mEff[i].mStatus++;
        }
            // fallthrough
        case 1:
        case 2: {
            s_pkt->mEff[i].mPos.x +=
                (windPowVec.x * s_chan.mWaveSpeed * s_pkt->mEff[i].mSpeed) *
                (s_pkt->mEff[i].mStrengthEnv * 0.5f + 0.5f) *
                (s_pkt->mEff[i].mAlpha * 0.8f + 0.2f);
            s_pkt->mEff[i].mPos.z +=
                (windPowVec.z * s_chan.mWaveSpeed * s_pkt->mEff[i].mSpeed) *
                (s_pkt->mEff[i].mStrengthEnv * 0.5f + 0.5f) *
                (s_pkt->mEff[i].mAlpha * 0.8f + 0.2f);
            s_pkt->mEff[i].mCounter += s_pkt->mEff[i].mCounterSpeed;

            cXyz pos;
            pos.x = s_pkt->mEff[i].mBasePos.x + s_pkt->mEff[i].mPos.x;
            pos.y = s_pkt->mEff[i].mBasePos.y + s_pkt->mEff[i].mPos.y;
            pos.z = s_pkt->mEff[i].mBasePos.z + s_pkt->mEff[i].mPos.z;

            if (pos.abs(eyevect) > s_chan.mWaveSpawnRadius) {
                s_pkt->mEff[i].mBasePos.x = eyevect.x;
                s_pkt->mEff[i].mBasePos.z = eyevect.z;
                if (pos.abs(eyevect) > (s_chan.mWaveSpawnRadius + 350.0f)) {
                    s_pkt->mEff[i].mPos.x = cM_rndFX(s_chan.mWaveSpawnRadius);
                    s_pkt->mEff[i].mPos.z = cM_rndFX(s_chan.mWaveSpawnRadius);
                } else {
                    cXyz newPos;
                    wwWaveGetVectle(&pos, &eyevect, &newPos);
                    s_pkt->mEff[i].mPos.x = newPos.x * s_chan.mWaveSpawnRadius;
                    s_pkt->mEff[i].mPos.z = newPos.z * s_chan.mWaveSpawnRadius;
                }
                s_pkt->mEff[i].mAlpha = 0.0f;
            }

            pos.x = s_pkt->mEff[i].mBasePos.x + s_pkt->mEff[i].mPos.x;
            pos.y = s_pkt->mEff[i].mBasePos.y + s_pkt->mEff[i].mPos.y;
            pos.z = s_pkt->mEff[i].mBasePos.z + s_pkt->mEff[i].mPos.z;
            s_pkt->mEff[i].mStrengthEnv = 1.0f;

            // Donor kytag01 mpWaveInfl (live ky_tag1 registrations).
            for (s32 j = 0; j < kWaveInflMax; j++) {
                if (s_waveInflPtrs[j] == NULL) {
                    continue;
                }
                cXyz inflPos = s_waveInflPtrs[j]->mPos;
                inflPos.y = pos.y;
                f32 dist = pos.abs(inflPos);
                f32 outerRadius = s_waveInflPtrs[j]->mOuterRadius;
                f32 innerRadius = s_waveInflPtrs[j]->mInnerRadius;
                if (dist < outerRadius) {
                    if (dist < innerRadius) {
                        s_pkt->mEff[i].mStrengthEnv = 0.0f;
                        break;
                    }
                    f32 range = outerRadius - innerRadius;
                    if (range > 0.0f) {
                        if (s_pkt->mEff[i].mStrengthEnv > (dist - innerRadius) / range) {
                            s_pkt->mEff[i].mStrengthEnv = (dist - innerRadius) / range;
                        }
                    } else {
                        s_pkt->mEff[i].mStrengthEnv = 0.0f;
                    }
                }
            }

            // Donor flatInter > 0: camera-centered dead zone grows toward open chop.
            if (s_chan.mWaveFlatInter > 0.0f) {
                cXyz eyePos = pCamera->view.lookat.eye;
                eyePos.y = pos.y;
                f32 dist = pos.abs(eyePos);
                f32 innerRadius =
                    s_chan.mWaveFlatInter * (s_chan.mWaveSpawnRadius * 1.5f);
                f32 outerRadius = innerRadius + 1000.0f;
                f32 range = outerRadius - innerRadius;
                if (range > 0.0f) {
                    if (s_pkt->mEff[i].mStrengthEnv > (dist - innerRadius) / range) {
                        s_pkt->mEff[i].mStrengthEnv = (dist - innerRadius) / range;
                    }
                } else {
                    s_pkt->mEff[i].mStrengthEnv = 0.0f;
                }
            }

            if (pPlayer != NULL) {
                cXyz nearP = pPlayer->current.pos;
                nearP.y = pos.y;
                f32 dist = pos.abs(nearP);
                f32 innerRadius = 200.0f;
                f32 outerRadius = 2000.0f;
                f32 range = outerRadius - innerRadius;
                if (dist < outerRadius) {
                    if (dist < innerRadius) {
                        s_pkt->mEff[i].mStrengthEnv = 0.0f;
                    } else {
                        s_pkt->mEff[i].mStrengthEnv *= (dist - innerRadius) / range;
                    }
                }
            }
            break;
        }
        case 3:
            s_pkt->mEff[i].mStatus = 0;
            break;
        default:
            break;
        }

        cXyz world;
        world.x = s_pkt->mEff[i].mBasePos.x + s_pkt->mEff[i].mPos.x;
        world.y = s_pkt->mEff[i].mBasePos.y + s_pkt->mEff[i].mPos.y;
        world.z = s_pkt->mEff[i].mBasePos.z + s_pkt->mEff[i].mPos.z;
        f32 dist = world.abs(pCamera->view.lookat.eye);
        if (dist < 0.0f) {
            dist = 0.0f;
        }
        f32 alphaTarget = 1.0f - (dist / (2.0f * s_chan.mWaveSpawnDist));
        alphaTarget *= 1.03f;
        alphaTarget *= (f32)std::sin(s_pkt->mEff[i].mCounter);
        if (alphaTarget > 1.0f) {
            alphaTarget = 1.0f;
        }
        if (alphaTarget < 0.0f) {
            alphaTarget = 0.0f;
        }
        cLib_addCalc(&s_pkt->mEff[i].mAlpha, alphaTarget, 0.5f, 0.5f, 0.001f);
        s_pkt->mEff[i].mBasePos.y = seaLevel;
    }
}

static void wwWaveDrawBody(Mtx drawMtx) {
    camera_class* pCamera = (camera_class*)dComIfGp_getCamera(0);
    if (s_pkt == NULL || pCamera == NULL || dComIfGd_getView() == NULL) {
        return;
    }
    if (s_chan.mWaveFlatInter >= 1.0f || s_chan.mWaveCount <= 0) {
        return;
    }

    Mtx camMtx;
    Mtx rotMtx;
    MTXInverse(dComIfGd_getViewRotMtx(), camMtx);

    f32 rot = cM_sht2d(pCamera->view.bank);
    j3dSys.reinitGX();
    GXSetClipMode(GX_CLIP_ENABLE);

    // TXA_USONAMI_M (index 1) is Forsaken Fortress only; Outset uses 0.
    // Donor drawWave (WWDP d_kankyo_rain.cpp:3311).
    u8* tex = s_pkt->mpTexUsonami;
    const char* stage = dComIfGp_getStartStageName();
    if (stage != NULL && std::strcmp(stage, "MajyuE") == 0 && s_pkt->mpTexUsonamiM != NULL) {
        tex = s_pkt->mpTexUsonamiM;
    }
    if (tex == NULL) {
        return;
    }
    GXTexObj texObj;
    wwWaveSetBtitex(&texObj, (ResTIMG*)tex);

    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXColor amb, dif;
    wwWaveGetSeacolor(&amb, &dif);
    {
        static u8 s_prevA[3], s_prevD[3];
        static bool s_have = false;
        static int s_frames = 0;
        const bool changed = !s_have || amb.r != s_prevA[0] || amb.g != s_prevA[1] ||
                             amb.b != s_prevA[2] || dif.r != s_prevD[0] || dif.g != s_prevD[1] ||
                             dif.b != s_prevD[2];
        if (changed || ++s_frames >= 90) {
            s_frames = 0;
            s_have = true;
            s_prevA[0] = amb.r;
            s_prevA[1] = amb.g;
            s_prevA[2] = amb.b;
            s_prevD[0] = dif.r;
            s_prevD[1] = dif.g;
            s_prevD[2] = dif.b;
            waveLog("[WwFoam] {\"ev\":\"seacolor\",\"amb\":[%d,%d,%d],\"dif\":[%d,%d,%d],"
                    "\"windPow\":%.3f,\"flat\":%.3f}",
                    (int)amb.r, (int)amb.g, (int)amb.b, (int)dif.r, (int)dif.g, (int)dif.b,
                    dKyWw_get_wind_pow(), s_chan.mWaveFlatInter);
        }
    }
    GXSetTevColor(GX_TEVREG0, dif);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K3_A);
    GXSetTevKColor(GX_KCOLOR0, amb);
    GXSetTevKColor(GX_KCOLOR3, amb);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    // Donor dKy_GxFog_sea_set (WWDP d_kankyo.cpp:80196B90): fog colour =
    // VrUsoUmi, then GxXFog_set. Plugin uso from Virt (wwKankyo_virt_set).
    {
        u8 uso[4];
        wwKankyo_vrboxGet(nullptr, nullptr, nullptr, uso, nullptr);
        GXColor fogCol = {uso[0], uso[1], uso[2], 0xFF};
        f32 nearZ = 1.0f;
        f32 farZ = 160000.0f;
        view_class* view = dComIfGd_getView();
        if (view != NULL && view->near_ >= 0.0f && view->far_ >= 0.0f &&
            view->near_ < view->far_) {
            nearZ = view->near_;
            farZ = view->far_;
        }
        GXSetFog(GX_FOG_PERSP_LIN, g_env_light.mFogNear, g_env_light.mFogFar, nearZ, farZ, fogCol);
        dKyd_xfog_table_set(g_env_light.mFogAdjTableType);
        GXSetFogRangeAdj(g_env_light.mFogAdjEnable, (u16)g_env_light.mFogAdjCenter,
                         &g_env_light.mXFogTbl);
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_GREATER, 0);
    GXSetZCompLoc(GX_FALSE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetCullMode(GX_CULL_NONE);
    GXSetNumIndStages(0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    MTXRotRad(rotMtx, 'Z', DEG_TO_RAD(rot));
    MTXConcat(camMtx, rotMtx, camMtx);
    GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    int drawPanes = 0;
    for (s32 i = 0; i < s_chan.mWaveCount; i++) {
        cXyz p;
        p.x = s_pkt->mEff[i].mBasePos.x + s_pkt->mEff[i].mPos.x;
        p.y = s_pkt->mEff[i].mBasePos.y + s_pkt->mEff[i].mPos.y;
        p.z = s_pkt->mEff[i].mBasePos.z + s_pkt->mEff[i].mPos.z;

        f32 wave = (f32)std::sin(s_pkt->mEff[i].mCounter);
        if (wave <= 0.0f) {
            continue;
        }

        f32 scale = s_chan.mWaveScale * s_pkt->mEff[i].mScale * wave;
        f32 scaleBottom = s_chan.mWaveScaleBottom * scale;
        f32 strength = s_pkt->mEff[i].mStrengthEnv;
        f32 height = strength * scale;
        f32 width = scaleBottom * (strength - 0.00000015f * (f32)(i * 32) * height);
        if (height <= 0.0f) {
            continue;
        }
        drawPanes++;

        amb.a = (u8)(s_pkt->mEff[i].mAlpha * 255.0f);
        GXSetTevKColor(GX_KCOLOR3, amb);

        cXyz vp, lp;
        cXyz pos[4];
        if (s_pkt->mSkewDir < 0.0f) {
            vp.x = -width + width * -(s_pkt->mEff[i].mSpeed * 1.2f) * s_pkt->mSkewWidth;
        } else {
            vp.x = -width - width * -(s_pkt->mEff[i].mSpeed * 1.2f) * s_pkt->mSkewWidth;
        }
        vp.y = height;
        vp.z = 0.0f;
        MTXMultVec(camMtx, &vp, &lp);
        pos[0].x = p.x + lp.x;
        pos[0].y = p.y + lp.y;
        pos[0].z = p.z + lp.z;

        if (s_pkt->mSkewDir < 0.0f) {
            vp.x = width + width * -(s_pkt->mEff[i].mSpeed * 1.2f) * s_pkt->mSkewWidth;
        } else {
            vp.x = width - width * -(s_pkt->mEff[i].mSpeed * 1.2f) * s_pkt->mSkewWidth;
        }
        vp.y = height;
        vp.z = 0.0f;
        MTXMultVec(camMtx, &vp, &lp);
        pos[1].x = p.x + lp.x;
        pos[1].y = p.y + lp.y;
        pos[1].z = p.z + lp.z;

        vp.x = width;
        vp.y = 0.0f;
        vp.z = 0.0f;
        MTXMultVec(camMtx, &vp, &lp);
        pos[2].x = p.x + lp.x;
        pos[2].y = p.y + lp.y;
        pos[2].z = p.z + lp.z;

        vp.x = -width;
        vp.y = 0.0f;
        vp.z = 0.0f;
        MTXMultVec(camMtx, &vp, &lp);
        pos[3].x = p.x + lp.x;
        pos[3].y = p.y + lp.y;
        pos[3].z = p.z + lp.z;

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(pos[0].x, pos[0].y, pos[0].z);
        GXTexCoord2s16(0, 0);
        GXPosition3f32(pos[1].x, pos[1].y, pos[1].z);
        GXTexCoord2s16(0xFA, 0);
        GXPosition3f32(pos[2].x, pos[2].y, pos[2].z);
        GXTexCoord2s16(0xFA, 0xFA);
        GXPosition3f32(pos[3].x, pos[3].y, pos[3].z);
        GXTexCoord2s16(0, 0xFA);
        GXEnd();
    }

    {
        static int s_log = 0;
        if ((++s_log % 300) == 1) {
            GXColor a, d;
            wwWaveGetSeacolor(&a, &d);
            cXyz wp = dKyWw_get_wind_vecpow();
            waveLog("[WwFoam] {\"ev\":\"draw\",\"panes\":%d,\"count\":%d,"
                    "\"amb\":[%d,%d,%d],\"dif\":[%d,%d,%d],\"pow\":%.3f,"
                    "\"skew\":%.3f,\"wvec\":[%.3f,%.3f,%.3f]}",
                    drawPanes, (int)s_chan.mWaveCount, (int)a.r, (int)a.g, (int)a.b, (int)d.r,
                    (int)d.g, (int)d.b, dKyWw_get_wind_pow(), s_pkt->mSkewWidth, wp.x, wp.y,
                    wp.z);
        }
    }

    J3DShape::resetVcdVatCache();
}

void dKyWwWave_Packet::draw() {
    MtxP view = dComIfGd_getViewMtx();
    if (view == NULL) {
        view = j3dSys.getViewMtx();
    }
    if (view != NULL) {
        wwWaveDrawBody(view);
    }
}

}  // namespace

void wwWave_reset() {
    if (s_pkt != NULL) {
        delete s_pkt;
        s_pkt = NULL;
    }
    s_armed = false;
    s_live = false;
    std::memset(&s_chan, 0, sizeof(s_chan));
    s_chan.mWaveFlatInter = 1.0f;
    wwWaveCalmClear();
    if (s_seaWindArmed) {
        dKyWw_custom_windpower(0.0f);
        s_seaWindArmed = false;
    }
}

void wwWave_move() {
    // Donor wether_move_wave (WWDP d_kankyo_wether.cpp:849): switch on
    // mWaveInitialized; case 0 allocates when mWaveCount != 0, pulls Always
    // USONAMI, wave_move(); case 1 runs wave_move or tears down on count==0.
    // No sun gate. No settle timer. Always is boot-resident on WW; here the
    // plugin loads WwAlways once in wwSky — gate is alwaysReady only.
    if (!wwWaveStageIsSea()) {
        if (s_armed || s_live) {
            wwWave_reset();
        }
        return;
    }

    wwWaveEnsureCalm();
    if (!s_seaWindArmed) {
        // Ferry F / §416: Outset FILI windLevel 0 → 0.3; donor vec (1,0,0).
        dKyWw_custom_windpower(0.3f);
        s_seaWindArmed = true;
        waveLog("[WwFoam] {\"ev\":\"wind_arm\",\"pow\":0.300,\"reads\":\"FILI level0 receipt\"}");
    }

    if (!s_armed) {
        wwWaveArmChan();
        s_armed = true;
        waveLog("[WwFoam] {\"ev\":\"arm\",\"count\":%d,\"dist\":%.0f,\"radius\":%.0f}",
                (int)s_chan.mWaveCount, s_chan.mWaveSpawnDist, s_chan.mWaveSpawnRadius);
    }

    wwWaveCalmUpdate();

    if (!s_live) {
        if (!wwSky_alwaysReady()) {
            return;
        }
        waveLog("[WwFoam] {\"ev\":\"tex_fetch\"}");
        u8* tex = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexUsonami);
        u8* texM = (u8*)dComIfG_getObjectRes("WwAlways", kWwTexUsonamiM);
        if (tex == NULL || texM == NULL) {
            static bool s_warn = false;
            if (!s_warn) {
                s_warn = true;
                waveLog("[WwFoam] {\"ev\":\"usonami_missing\",\"tex\":%d,\"texM\":%d}",
                        tex != NULL ? 1 : 0, texM != NULL ? 1 : 0);
            }
            return;
        }
        if (s_pkt == NULL) {
            waveLog("[WwFoam] {\"ev\":\"packet_new\"}");
            s_pkt = new dKyWwWave_Packet();
        }
        if (s_pkt == NULL) {
            waveLog("[WwFoam] {\"ev\":\"packet_alloc_fail\"}");
            return;
        }
        s_pkt->mpTexUsonami = tex;
        s_pkt->mpTexUsonamiM = texM;
        for (int i = 0; i < 300; i++) {
            s_pkt->mEff[i].mStatus = 0;
        }
        s_pkt->mSkewWidth = 0.0f;
        s_pkt->mSkewDir = 0.0f;
        s_live = true;
        waveLog("[WwFoam] {\"ev\":\"live\",\"count\":%d}", (int)s_chan.mWaveCount);
        wwWaveMoveBody();
        waveLog("[WwFoam] {\"ev\":\"move_ok\"}");
        return;
    }

    if (s_chan.mWaveCount == 0) {
        wwWave_reset();
        return;
    }

    wwWaveMoveBody();
}

void wwWave_drawQueue() {
    if (!s_live || s_pkt == NULL || s_chan.mWaveCount <= 0) {
        return;
    }
    if (s_chan.mWaveFlatInter >= 1.0f) {
        return;
    }
    // Donor dKyw_setDrawPacketListXluBg (d_kankyo_wether.cpp): setXluListBG →
    // entryImm → setList. Must run once per frame fill — a second entryImm of
    // the same packet before drawClear makes next=self (circular list → die).
    s_pkt->drawClear();
    dComIfGd_setXluListBG();
    J3DDrawBuffer* xlu = dComIfGd_getXluListBG();
    if (xlu != NULL) {
        xlu->entryImm(s_pkt, 0);
        {
            static bool s_once = false;
            if (!s_once) {
                s_once = true;
                waveLog("[WwFoam] {\"ev\":\"draw_queued\",\"path\":\"xlu_bg\"}");
            }
        }
    }
    dComIfGd_setList();
}

bool wwWave_registerInfl(WwWaveInfl* info) {
    if (info == NULL) {
        return false;
    }
    for (int i = 0; i < kWaveInflMax; i++) {
        if (s_waveInflPtrs[i] == info) {
            return true;
        }
    }
    for (int i = 0; i < kWaveInflMax; i++) {
        if (s_waveInflPtrs[i] == NULL) {
            s_waveInflPtrs[i] = info;
            waveLog("[WwFoam] {\"ev\":\"kytag_infl\",\"slot\":%d,\"pos\":[%.1f,%.1f,%.1f],"
                    "\"inner\":%.0f,\"outer\":%.0f}",
                    i, info->mPos.x, info->mPos.y, info->mPos.z, info->mInnerRadius,
                    info->mOuterRadius);
            return true;
        }
    }
    waveLog("[WwFoam] {\"ev\":\"kytag_infl_full\"}");
    return false;
}

void wwWave_unregisterInfl(WwWaveInfl* info) {
    if (info == NULL) {
        return;
    }
    for (int i = 0; i < kWaveInflMax; i++) {
        if (s_waveInflPtrs[i] == info) {
            s_waveInflPtrs[i] = NULL;
            waveLog("[WwFoam] {\"ev\":\"kytag_infl_clear\",\"slot\":%d}", i);
        }
    }
}
