// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo.cpp MatchingFor setLightTevColorType_sub /
//            setLightTevColorType (80193650-80193ADC) and the C0/K0 write at
//            the end of settingTevStruct (80193028). Pale layout from
//            include/d/d_stage.h stage_palet_info_class (0x2C).
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Vanilla setLightTevColorType(J3DModelData*) is an empty stub; TP writes
// colours inside _MAJI. WW materials are authored against this copy.
// mColorK1 (second-light) has no field on vanilla tevstr — the donor else-leg
// (disable extra TEV stage) is the retail path when K1.a == 0. Do not widen
// dKy_tevstr_c.
//
// toon_proc_check(): WW retail is toon-off, the else branch.

#include "ww_kankyo.h"

#include "d/dolzel_rel.h"  // IWYU pragma: keep — receiver prerequisite, first
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_data.h"
#include "d/d_kankyo_tev_str.h"
#include "d/d_stage.h"
#include <mods/api.h>
#include <mods/svc/log.h>
#include <cstdio>
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphBase/J3DMatBlock.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DStruct.h"
#include <dolphin/gx.h>
#include "SSystem/SComponent/c_xyz.h"
#include "m_Do/m_Do_mtx.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

extern const LogService* s_log;

#pragma pack(push, 1)
struct WwPalet {
    color_RGB_class actor_c0;
    color_RGB_class actor_k0;
    color_RGB_class bg0_c0;
    color_RGB_class bg0_k0;
    color_RGB_class bg1_c0;
    color_RGB_class bg1_k0;
    color_RGB_class bg2_c0;
    color_RGB_class bg2_k0;
    color_RGB_class bg3_c0;
    color_RGB_class bg3_k0;
    color_RGB_class fog;
    u8 virtIdx;
    u8 pad[2];
    BE<f32> fogStart;
    BE<f32> fogEnd;
};
#pragma pack(pop)
static_assert(sizeof(WwPalet) == 0x2C, "donor stage_palet_info_class");

#pragma pack(push, 1)
struct WwVirt {
    u32 field_0x00;
    u32 field_0x04;
    u32 field_0x08;
    u32 field_0x0c;
    GXColor kumo;
    GXColor kumoCenter;
    color_RGB_class sky;
    color_RGB_class usoUmi;
    color_RGB_class kasumi;
    u8 pad[3];
};
#pragma pack(pop)
static_assert(sizeof(WwVirt) == 0x24, "donor stage_vrbox_info_class");

#pragma pack(push, 1)
struct WwEnvr {
    u8 pselect_id[8];
};
#pragma pack(pop)
static_assert(sizeof(WwEnvr) == 0x8, "donor stage_envr_info_class");

#pragma pack(push, 1)
struct WwColo {
    u8 palette_id[8];
    BE<f32> change_rate;
};
#pragma pack(pop)
static_assert(sizeof(WwColo) == 0xC, "donor stage_pselect_info_class");

namespace {

const u32 kTagPale = 0x656C6150u;  // raw 'Pale' bytes as LE u32 (disc BE memcpy)
const u32 kTagVirt = 0x74726956u;  // raw 'Virt'
const u32 kTagEnvR = 0x52766E45u;  // raw 'EnvR'
const u32 kTagColo = 0x6F6C6F43u;  // raw 'Colo'

const WwPalet* s_palet = nullptr;
int s_paletN = 0;
const WwVirt* s_virt = nullptr;
int s_virtN = 0;
const WwEnvr* s_envr = nullptr;
int s_envrN = 0;
const WwColo* s_colo = nullptr;
int s_coloN = 0;

GXColor s_sky = {0, 0, 0, 0xFF};
GXColor s_kasumi = {0, 0, 0, 0xFF};
GXColor s_kumo = {0, 0, 0, 0xFF};
GXColor s_kumoCenter = {0, 0, 0, 0xFF};
GXColor s_uso = {0, 0, 0, 0xFF};
int s_vrboxInvisible = 1;
int s_selPal0 = 0;
int s_selPal1 = 0;
f32 s_selTimeT = 0.0f;

const color_RGB_class* bgC0(const WwPalet* p, int n) {
    switch (n & 3) {
    case 0: return &p->bg0_c0;
    case 1: return &p->bg1_c0;
    case 2: return &p->bg2_c0;
    default: return &p->bg3_c0;
    }
}

const color_RGB_class* bgK0(const WwPalet* p, int n) {
    switch (n & 3) {
    case 0: return &p->bg0_k0;
    case 1: return &p->bg1_k0;
    case 2: return &p->bg2_k0;
    default: return &p->bg3_k0;
    }
}

int paletIndexEnv() {
    if (s_paletN <= 0) {
        return 0;
    }
    int idx = g_env_light.UseCol;
    if (idx < 0 || idx >= s_paletN) {
        idx = 0;
    }
    return idx;
}

int clipIndex(int idx, int n) {
    if (n <= 0) {
        return 0;
    }
    if (idx < 0) {
        return 0;
    }
    if (idx >= n) {
        return n - 1;
    }
    return idx;
}

// Donor setLight palette chain (d_kankyo.cpp:636-828): EnvR[room] ->
// Colo[weather] -> palette_id[schedule slot], blended by timeT. UseCol as a
// Pale index is the TP leftover that produced raw [21,35,33] on Outset.
void selectPale() {
    s_selPal0 = paletIndexEnv();
    s_selPal1 = s_selPal0;
    s_selTimeT = 0.0f;
    if (s_envr == nullptr || s_envrN <= 0 || s_colo == nullptr || s_coloN <= 0 ||
        s_palet == nullptr || s_paletN <= 0) {
        return;
    }
    int room = clipIndex(dComIfGp_roomControl_getStayNo(), s_envrN);
    u8 weather = g_env_light.wether_pat0;
    if (weather > 7) {
        weather = 0;
    }
    const int coloIdx = clipIndex(s_envr[room].pselect_id[weather], s_coloN);
    const WwColo& colo = s_colo[coloIdx];

    u8 slot0 = 2;
    u8 slot1 = 2;
    const dKyd_lightSchejule* sch = g_env_light.light_schedule;
    const f32 daytime = g_env_light.daytime;
    if (sch != nullptr) {
        for (int i = 0; i < 11; i++) {
            if (daytime >= sch[i].startTime && daytime <= sch[i].endTime) {
                slot0 = sch[i].startTimeLight;
                slot1 = sch[i].endTimeLight;
                const f32 span = sch[i].endTime - sch[i].startTime;
                if (span != 0.0f) {
                    s_selTimeT = 1.0f - (sch[i].endTime - daytime) / span;
                    if (s_selTimeT > 1.0f) {
                        s_selTimeT = 1.0f;
                    }
                    if (s_selTimeT < 0.0f) {
                        s_selTimeT = 0.0f;
                    }
                } else {
                    s_selTimeT = 1.0f;
                }
                break;
            }
        }
    }
    if (slot0 > 5) {
        slot0 = 2;
    }
    if (slot1 > 5) {
        slot1 = 2;
    }
    s_selPal0 = clipIndex(colo.palette_id[slot0], s_paletN);
    s_selPal1 = clipIndex(colo.palette_id[slot1], s_paletN);
}

void kankyoLog(const char* fmt, ...) {
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

s16 kyRatioS16(s16 a, s16 b, f32 t) {
    return (s16)(a + (s16)(t * (f32)(b - a)));
}

s16 kyColorRatio(u8 b0A, u8 b0B, u8 b1A, u8 b1B, f32 timeT, f32 roomT, s16 add, f32 mul) {
    s16 a = kyRatioS16(b0A, b0B, timeT);
    s16 b = kyRatioS16(b1A, b1B, timeT);
    s16 rt = kyRatioS16(a, b, roomT);
    rt = (s16)(rt + add);
    rt = (s16)(rt * (g_env_light.now_allcol_ratio * mul));
    if (rt < 0) {
        return 0;
    }
    if (rt > 255) {
        return 255;
    }
    return rt;
}

u8 clipU8(s16 v) {
    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return (u8)v;
}

void applyFog(dKy_tevstr_c* i_tevstr, const WwPalet& p0, const WwPalet& p1, f32 t) {
    const f32 fogR = g_env_light.now_fogcol_ratio;
    i_tevstr->FogCol.r = clipU8((s16)(kyRatioS16(p0.fog.r, p1.fog.r, t) * fogR));
    i_tevstr->FogCol.g = clipU8((s16)(kyRatioS16(p0.fog.g, p1.fog.g, t) * fogR));
    i_tevstr->FogCol.b = clipU8((s16)(kyRatioS16(p0.fog.b, p1.fog.b, t) * fogR));
    i_tevstr->FogCol.a = 255;
    const f32 s0 = p0.fogStart;
    const f32 s1 = p1.fogStart;
    const f32 e0 = p0.fogEnd;
    const f32 e1 = p1.fogEnd;
    i_tevstr->mFogStartZ = s0 + t * (s1 - s0);
    i_tevstr->mFogEndZ = e0 + t * (e1 - e0);
}

// Pale C0/K0, then donor sun-stare ratios (dKyr_sun_move → dKy_set_*col_ratio).
// Earlier note that now_* "sit at 0" was wrong for actcol (init/reset = 1.0);
// skipping them dropped the stare washout while virt_set still used sora/kumo.
static s16 scaleCol(s16 v, f32 ratio) {
    return (s16)(v * ratio);
}

void applyBgOverlay(dKy_tevstr_c* i_tevstr, int n) {
    if (i_tevstr == nullptr || s_palet == nullptr || s_paletN <= 0) {
        return;
    }
    selectPale();
    const WwPalet& p0 = s_palet[s_selPal0];
    const WwPalet& p1 = s_palet[s_selPal1];
    const f32 t = s_selTimeT;
    const f32 bgR = g_env_light.now_bgcol_ratio;
    const color_RGB_class* c0a = bgC0(&p0, n);
    const color_RGB_class* c0b = bgC0(&p1, n);
    const color_RGB_class* k0a = bgK0(&p0, n);
    const color_RGB_class* k0b = bgK0(&p1, n);
    i_tevstr->TevColor.r = scaleCol(kyRatioS16(c0a->r, c0b->r, t), bgR);
    i_tevstr->TevColor.g = scaleCol(kyRatioS16(c0a->g, c0b->g, t), bgR);
    i_tevstr->TevColor.b = scaleCol(kyRatioS16(c0a->b, c0b->b, t), bgR);
    i_tevstr->TevColor.a = 255;
    i_tevstr->TevKColor.r = clipU8(scaleCol(kyRatioS16(k0a->r, k0b->r, t), bgR));
    i_tevstr->TevKColor.g = clipU8(scaleCol(kyRatioS16(k0a->g, k0b->g, t), bgR));
    i_tevstr->TevKColor.b = clipU8(scaleCol(kyRatioS16(k0a->b, k0b->b, t), bgR));
    i_tevstr->mLightMode = 0;
    applyFog(i_tevstr, p0, p1, t);
    // ============================================================
    // WHAT THE BG OVERLAY ACTUALLY WRITES - the receipt the black
    // question needs, and the one my AmbCol lever was NOT.
    // ============================================================
    // `_sub` pushes TevColor and TevKColor into the material; those two
    // are the BG colour path. History/Bridge read `Pale[3].bg3_c0` from
    // the disc as literally (0,0,0), and 36 of the 101 BG draws in the
    // black run are bg3 (type 35). If that is right, this receipt shows
    // TevColor all-zero for n=3 and NON-zero for n=0..2 - which would
    // mean the donor's own palette writes black for that slot and the
    // defect is the TYPE SELECTION, not the write. If TevKColor is
    // non-zero while TevColor is zero, the colour lives in k0 and the
    // consume side is reading the wrong register.
    // Sampled per bg index so all four are visible without flooding.
    {
        static int s_bgSeen[4] = {0,0,0,0};
        // PER-INDEX sampling, plus a periodic re-log: `selectPale()` interpolates
        // by time of day, so a first-samples-only gate reports the palette at ONE
        // moment and would hide a value that goes black later. History/Bridge's
        // warning about sampling gates hiding the exact failure, applied here
        // before it costs a run rather than after.
        if (n >= 0 && n < 4 && (s_bgSeen[n]++ < 2 || (s_bgSeen[n] % 600) == 0)) {
            kankyoLog("[WwRegistry] {\"ev\":\"bg_overlay\",\"bg\":%d,\"pale\":[%d,%d],"
                      "\"tev_c0\":[%d,%d,%d],\"tev_k0\":[%d,%d,%d],\"bg_ratio\":%d,"
                      "\"reads\":\"what the BG leg WRITES. all-zero c0 on one index = the "
                      "donor palette is black there and the defect is TYPE SELECTION; "
                      "k0 non-zero with c0 zero = the colour is in the other register\"}",
                      n, s_selPal0, s_selPal1,
                      (int)i_tevstr->TevColor.r, (int)i_tevstr->TevColor.g,
                      (int)i_tevstr->TevColor.b,
                      (int)i_tevstr->TevKColor.r, (int)i_tevstr->TevKColor.g,
                      (int)i_tevstr->TevKColor.b, (int)(bgR * 100.0f));
        }
    }
}

// lightType 9/10 = TP Alink. Vanilla settingTevStruct fills AmbCol only;
// TevColor stays 0 (tevstr_init memset). MAJI copies both — so TevColor must
// stay 0. Writing Pale actor_c0 into TevColor blew Link white (Pale2 C0 is
// ~156,140,134 as AmbCol; same value as C0 register is wrong for Alink).
bool overlayIsPlayer(int lightType) {
    return lightType == TEV_TYPE_PLAYER || lightType == 10;
}

void applyActorOverlay(dKy_tevstr_c* i_tevstr, bool playerLook) {
    if (i_tevstr == nullptr || s_palet == nullptr || s_paletN <= 0) {
        return;
    }
    selectPale();
    const WwPalet& p0 = s_palet[s_selPal0];
    const WwPalet& p1 = s_palet[s_selPal1];
    const f32 t = s_selTimeT;
    // Donor actor path: mActColRatio * mActColRatio
    const f32 actR = g_env_light.now_actcol_ratio * g_env_light.now_actcol_ratio;
    const s16 c0r = scaleCol(kyRatioS16(p0.actor_c0.r, p1.actor_c0.r, t), actR);
    const s16 c0g = scaleCol(kyRatioS16(p0.actor_c0.g, p1.actor_c0.g, t), actR);
    const s16 c0b = scaleCol(kyRatioS16(p0.actor_c0.b, p1.actor_c0.b, t), actR);
    // convert_lighting maps actor_c0 → TP actor_amb_col → AmbCol.
    i_tevstr->AmbCol.r = c0r;
    i_tevstr->AmbCol.g = c0g;
    i_tevstr->AmbCol.b = c0b;
    i_tevstr->AmbCol.a = 255;
    i_tevstr->mLightMode = 1;  // donor actor/player leg
    if (playerLook) {
        // Match vanilla actor settingTevStruct: TevColor/KColor untouched (0).
        i_tevstr->TevColor.r = 0;
        i_tevstr->TevColor.g = 0;
        i_tevstr->TevColor.b = 0;
        i_tevstr->TevColor.a = 0;
        i_tevstr->TevKColor.r = 0;
        i_tevstr->TevKColor.g = 0;
        i_tevstr->TevKColor.b = 0;
        i_tevstr->TevKColor.a = 0;
    } else {
        // WW materials: donor C0/K0 → Tev registers.
        i_tevstr->TevColor.r = c0r;
        i_tevstr->TevColor.g = c0g;
        i_tevstr->TevColor.b = c0b;
        i_tevstr->TevColor.a = 255;
        i_tevstr->TevKColor.r = clipU8(scaleCol(kyRatioS16(p0.actor_k0.r, p1.actor_k0.r, t), actR));
        i_tevstr->TevKColor.g = clipU8(scaleCol(kyRatioS16(p0.actor_k0.g, p1.actor_k0.g, t), actR));
        i_tevstr->TevKColor.b = clipU8(scaleCol(kyRatioS16(p0.actor_k0.b, p1.actor_k0.b, t), actR));
    }
    applyFog(i_tevstr, p0, p1, t);
}

int overlayBgIndex(int lightType) {
    if (lightType >= 32 && lightType <= 35) {
        return (lightType - 32) & 3;
    }
    if (lightType >= TEV_TYPE_BG0 && lightType <= TEV_TYPE_BG3_FULL) {
        return (lightType - 1) & 3;
    }
    return -1;
}

void overlayTev(int lightType, dKy_tevstr_c* tevstr) {
    if (tevstr == nullptr || s_palet == nullptr || s_paletN <= 0) {
        return;
    }
    if (lightType == 12 || lightType == 13 || lightType == 14) {
        return;
    }
    const int bg = overlayBgIndex(lightType);
    if (bg >= 0) {
        applyBgOverlay(tevstr, bg);
        return;
    }
    applyActorOverlay(tevstr, overlayIsPlayer(lightType));
}

}  // namespace

int wwKankyo_bindDzs(void* dzs) {
    if (dzs == nullptr) {
        return s_paletN;
    }
    dStage_fileHeader* file = static_cast<dStage_fileHeader*>(dzs);
    const int n = file->m_chunkCount;
    if (n <= 0 || n > 512) {
        return s_paletN;
    }
    dStage_nodeHeader* node = file->m_nodes;
    for (int i = 0; i < n; i++, node++) {
        if (node->m_tag == kTagPale) {
            WwPalet* pal = static_cast<WwPalet*>(node->m_offset);
            const int entries = node->m_entryNum;
            if (pal != nullptr && entries > 0 && entries <= 256) {
                s_palet = pal;
                s_paletN = entries;
            }
        } else if (node->m_tag == kTagVirt) {
            WwVirt* virt = static_cast<WwVirt*>(node->m_offset);
            const int entries = node->m_entryNum;
            if (virt != nullptr && entries > 0 && entries <= 256) {
                s_virt = virt;
                s_virtN = entries;
            }
        } else if (node->m_tag == kTagEnvR) {
            WwEnvr* envr = static_cast<WwEnvr*>(node->m_offset);
            const int entries = node->m_entryNum;
            if (envr != nullptr && entries > 0 && entries <= 256) {
                s_envr = envr;
                s_envrN = entries;
            }
        } else if (node->m_tag == kTagColo) {
            WwColo* colo = static_cast<WwColo*>(node->m_offset);
            const int entries = node->m_entryNum;
            if (colo != nullptr && entries > 0 && entries <= 256) {
                s_colo = colo;
                s_coloN = entries;
            }
        }
    }
    return s_paletN;
}

void wwKankyo_reset() {
    s_palet = nullptr;
    s_paletN = 0;
    s_virt = nullptr;
    s_virtN = 0;
    s_envr = nullptr;
    s_envrN = 0;
    s_colo = nullptr;
    s_coloN = 0;
    s_sky.r = s_sky.g = s_sky.b = 0;
    s_sky.a = 0xFF;
    s_kasumi = s_kumo = s_kumoCenter = s_uso = s_sky;
    s_vrboxInvisible = 1;
    s_selPal0 = 0;
    s_selPal1 = 0;
    s_selTimeT = 0.0f;
}

int wwKankyo_paletN() {
    return s_paletN;
}

int wwKankyo_virtN() {
    return s_virtN;
}

int wwKankyo_envrN() {
    return s_envrN;
}

int wwKankyo_coloN() {
    return s_coloN;
}

void wwKankyo_getSeacolor(unsigned char amb[4], unsigned char dif[4]) {
    if (amb == nullptr || dif == nullptr) {
        return;
    }
    // Donor dKy_get_seacolor (WWDP d_kankyo.cpp:3454): mBG1_C0/K0 + addcols.
    // Plugin: blend currently selected Pale BG1 (same EnvR/Colo/schedule as
    // virt_set). Do not read TP dungeonlight_col[2] — that is the fork №113
    // stash, absent on stock disc sea.
    if (s_palet == nullptr || s_paletN <= 0) {
        amb[0] = amb[1] = amb[2] = 0xFF;
        amb[3] = 0xFF;
        dif[0] = 9;
        dif[1] = 99;
        dif[2] = 224;
        dif[3] = 0xFF;
        return;
    }
    selectPale();
    const WwPalet& p0 = s_palet[s_selPal0];
    const WwPalet& p1 = s_palet[s_selPal1];
    const f32 t = s_selTimeT;
    const s16 ambr = (s16)(kyRatioS16(p0.bg1_c0.r, p1.bg1_c0.r, t) + g_env_light.bg1_addcol_amb.r);
    const s16 ambg = (s16)(kyRatioS16(p0.bg1_c0.g, p1.bg1_c0.g, t) + g_env_light.bg1_addcol_amb.g);
    const s16 ambb = (s16)(kyRatioS16(p0.bg1_c0.b, p1.bg1_c0.b, t) + g_env_light.bg1_addcol_amb.b);
    amb[0] = clipU8(ambr);
    amb[1] = clipU8(ambg);
    amb[2] = clipU8(ambb);
    amb[3] = 0xFF;
    dif[0] = clipU8(kyRatioS16(p0.bg1_k0.r, p1.bg1_k0.r, t));
    dif[1] = clipU8(kyRatioS16(p0.bg1_k0.g, p1.bg1_k0.g, t));
    dif[2] = clipU8(kyRatioS16(p0.bg1_k0.b, p1.bg1_k0.b, t));
    dif[3] = 0xFF;
}

void wwKankyo_virt_set() {
    if (s_virt == nullptr || s_virtN <= 0) {
        s_vrboxInvisible = 1;
        return;
    }

    // Same EnvR/Colo/schedule chain as actor/BG overlay (selectPale).
    selectPale();
    const int pal0 = s_selPal0;
    const int pal1 = s_selPal1;
    const f32 timeT = s_selTimeT;
    const int room = clipIndex(dComIfGp_roomControl_getStayNo(), s_envrN);
    u8 weather = g_env_light.wether_pat0;
    if (weather > 7) {
        weather = 0;
    }

    const int vidx0 = clipIndex(s_palet != nullptr ? s_palet[pal0].virtIdx : 0, s_virtN);
    const int vidx1 = clipIndex(s_palet != nullptr ? s_palet[pal1].virtIdx : vidx0, s_virtN);
    const WwVirt& v0 = s_virt[vidx0];
    const WwVirt& v1 = s_virt[vidx1];
    const f32 roomT = 1.0f;
    const f32 soraR = g_env_light.now_vrboxsoracol_ratio;
    const f32 kumoR = g_env_light.now_vrboxkumocol_ratio;
    const s16 addS = g_env_light.vrbox_addcol_sky0.r;
    const s16 addSg = g_env_light.vrbox_addcol_sky0.g;
    const s16 addSb = g_env_light.vrbox_addcol_sky0.b;
    const s16 addK = g_env_light.vrbox_addcol_kasumi.r;
    const s16 addKg = g_env_light.vrbox_addcol_kasumi.g;
    const s16 addKb = g_env_light.vrbox_addcol_kasumi.b;

    s_sky.r = (u8)kyColorRatio(v0.sky.r, v1.sky.r, v0.sky.r, v1.sky.r, timeT, roomT, addS, soraR);
    s_sky.g = (u8)kyColorRatio(v0.sky.g, v1.sky.g, v0.sky.g, v1.sky.g, timeT, roomT, addSg, soraR);
    s_sky.b = (u8)kyColorRatio(v0.sky.b, v1.sky.b, v0.sky.b, v1.sky.b, timeT, roomT, addSb, soraR);
    s_sky.a = 0xFF;
    s_kasumi.r = (u8)kyColorRatio(v0.kasumi.r, v1.kasumi.r, v0.kasumi.r, v1.kasumi.r, timeT, roomT,
                                 addK, soraR);
    s_kasumi.g = (u8)kyColorRatio(v0.kasumi.g, v1.kasumi.g, v0.kasumi.g, v1.kasumi.g, timeT, roomT,
                                 addKg, soraR);
    s_kasumi.b = (u8)kyColorRatio(v0.kasumi.b, v1.kasumi.b, v0.kasumi.b, v1.kasumi.b, timeT, roomT,
                                 addKb, soraR);
    s_kasumi.a = 0xFF;
    s_kumo.r = (u8)kyColorRatio(v0.kumo.r, v1.kumo.r, v0.kumo.r, v1.kumo.r, timeT, roomT, addS, kumoR);
    s_kumo.g = (u8)kyColorRatio(v0.kumo.g, v1.kumo.g, v0.kumo.g, v1.kumo.g, timeT, roomT, addSg, kumoR);
    s_kumo.b = (u8)kyColorRatio(v0.kumo.b, v1.kumo.b, v0.kumo.b, v1.kumo.b, timeT, roomT, addSb, kumoR);
    s_kumo.a = (u8)kyColorRatio(v0.kumo.a, v1.kumo.a, v0.kumo.a, v1.kumo.a, timeT, roomT, 0, 1.0f);
    // Virt CenterCloudColor — TP dropped the consumer; WW draw lerps it in.
    s_kumoCenter.r =
        (u8)kyColorRatio(v0.kumoCenter.r, v1.kumoCenter.r, v0.kumoCenter.r, v1.kumoCenter.r, timeT,
                         roomT, addS, kumoR);
    s_kumoCenter.g =
        (u8)kyColorRatio(v0.kumoCenter.g, v1.kumoCenter.g, v0.kumoCenter.g, v1.kumoCenter.g, timeT,
                         roomT, addSg, kumoR);
    s_kumoCenter.b =
        (u8)kyColorRatio(v0.kumoCenter.b, v1.kumoCenter.b, v0.kumoCenter.b, v1.kumoCenter.b, timeT,
                         roomT, addSb, kumoR);
    s_kumoCenter.a = (u8)kyColorRatio(v0.kumoCenter.a, v1.kumoCenter.a, v0.kumoCenter.a,
                                      v1.kumoCenter.a, timeT, roomT, 0, 1.0f);
    s_uso.r = (u8)kyColorRatio(v0.usoUmi.r, v1.usoUmi.r, v0.usoUmi.r, v1.usoUmi.r, timeT, roomT,
                              addS, soraR);
    s_uso.g = (u8)kyColorRatio(v0.usoUmi.g, v1.usoUmi.g, v0.usoUmi.g, v1.usoUmi.g, timeT, roomT,
                              addSg, soraR);
    s_uso.b = (u8)kyColorRatio(v0.usoUmi.b, v1.usoUmi.b, v0.usoUmi.b, v1.usoUmi.b, timeT, roomT,
                              addSb, soraR);
    s_uso.a = 0xFF;

    const int sum = s_sky.r + s_sky.g + s_sky.b + s_kasumi.r + s_kasumi.g + s_kasumi.b +
                    s_kumo.r + s_kumo.g + s_kumo.b;
    s_vrboxInvisible = (sum == 0) ? 1 : 0;

    static int s_virtSets = 0;
    s_virtSets++;
    if (s_virtSets <= 6 || (s_virtSets % 300) == 0) {
        kankyoLog("[WwRegistry] {\"ev\":\"virt_set\",\"n\":%d,\"room\":%d,\"weather\":%u,"
                  "\"pale\":[%d,%d],\"vidx\":[%d,%d],\"timeT\":%.3f,"
                  "\"sky\":[%u,%u,%u],\"kumo\":[%u,%u,%u],\"allcol\":%.3f,\"soraR\":%.3f,"
                  "\"envr\":%d,\"colo\":%d}",
                  s_virtSets, room, (unsigned)weather, pal0, pal1, vidx0, vidx1, timeT,
                  s_sky.r, s_sky.g, s_sky.b, s_kumo.r, s_kumo.g, s_kumo.b,
                  g_env_light.now_allcol_ratio, soraR, s_envrN, s_coloN);
    }
}

void wwKankyo_vrboxGet(unsigned char sky[4], unsigned char kasumi[4], unsigned char kumo[4],
                       unsigned char uso[4], int* invisible) {
    if (sky != nullptr) {
        sky[0] = s_sky.r;
        sky[1] = s_sky.g;
        sky[2] = s_sky.b;
        sky[3] = s_sky.a;
    }
    if (kasumi != nullptr) {
        kasumi[0] = s_kasumi.r;
        kasumi[1] = s_kasumi.g;
        kasumi[2] = s_kasumi.b;
        kasumi[3] = s_kasumi.a;
    }
    if (kumo != nullptr) {
        kumo[0] = s_kumo.r;
        kumo[1] = s_kumo.g;
        kumo[2] = s_kumo.b;
        kumo[3] = s_kumo.a;
    }
    if (uso != nullptr) {
        uso[0] = s_uso.r;
        uso[1] = s_uso.g;
        uso[2] = s_uso.b;
        uso[3] = s_uso.a;
    }
    if (invisible != nullptr) {
        *invisible = s_vrboxInvisible;
    }
}

void wwKankyo_vrkumoColorsGet(unsigned char horizon[4], unsigned char center[4]) {
    if (horizon != nullptr) {
        horizon[0] = s_kumo.r;
        horizon[1] = s_kumo.g;
        horizon[2] = s_kumo.b;
        horizon[3] = s_kumo.a;
    }
    if (center != nullptr) {
        center[0] = s_kumoCenter.r;
        center[1] = s_kumoCenter.g;
        center[2] = s_kumoCenter.b;
        center[3] = s_kumoCenter.a;
    }
}

void dKyWw_overlayTevStruct(int i_lightType, dKy_tevstr_c* i_tevstr) {
    overlayTev(i_lightType, i_tevstr);
}

void dKyWw_settingTevStruct(int i_lightType, cXyz* i_pos, dKy_tevstr_c* i_tevstr) {
    g_env_light.settingTevStruct(i_lightType, i_pos, i_tevstr);
    dKyWw_overlayTevStruct(i_lightType, i_tevstr);
}

// MAJI remainder: TP player materials consume AmbCol. WW _sub never wrote it.
// Pale actor_c0 → AmbCol (convert_lighting slot), not fork (90,90,90).
static void dKyWw_writeAmbCol(J3DMaterial* i_material, dKy_tevstr_c* i_tevstr) {
    if (i_material == nullptr || i_tevstr == nullptr) {
        return;
    }
    GXColor amb;
    amb.r = (u8)i_tevstr->AmbCol.r;
    amb.g = (u8)i_tevstr->AmbCol.g;
    amb.b = (u8)i_tevstr->AmbCol.b;
    amb.a = (u8)i_tevstr->AmbCol.a;
    i_material->change();
    i_material->setAmbColor(0, (J3DGXColor*)&amb);
}

static void dKyWw_setLightTevColorType_sub(J3DMaterial* i_material, dKy_tevstr_c* i_tevstr,
                                           bool dropExtraStages) {
    if (i_material == nullptr || i_tevstr == nullptr) {
        return;
    }

    if (dropExtraStages && i_tevstr->mLightMode != 0) {
        J3DColorChan* colorchan_p = i_material->getColorChan(0);
        if (colorchan_p != nullptr) {
            colorchan_p->setLightMask(1);
        }
        J3DGXColorS10* tev3 = i_material->getTevColor(3);
        int var_r28 = 0xFF;
        if (tev3 != nullptr) {
            const int prev_a = tev3->a;
            if (prev_a > 0 && i_material->getTevBlock() != nullptr &&
                i_material->getTevBlock()->getTevKColorSel(prev_a - 1) == 13) {
                var_r28 = prev_a - 1;
            }
        }
        if (var_r28 != 0xFF && i_material->getTevBlock() != nullptr) {
            // Donor: if mColorK1.a != 0 enable extra stage. Vanilla has no K1
            // field — a==0, else-leg (retail, no second light).
            i_material->getTevBlock()->setTevStageNum((u8)var_r28);
        }
    }

    if (j3dSys.getViewMtx() != nullptr) {
        cXyz sp14;
        cMtx_multVec(j3dSys.getViewMtx(), &i_tevstr->mLightPosWorld, &sp14);
        i_tevstr->mLightObj.getLightInfo()->mLightPosition = sp14;
    }

    i_material->setLight(0, &i_tevstr->mLightObj);

    J3DGXColorS10* col_p = i_material->getTevColor(0);
    if (col_p != nullptr) {
        i_tevstr->TevColor.a = col_p->a;
        i_material->setTevColor(0, (J3DGXColorS10*)&i_tevstr->TevColor);
    }
    J3DGXColor* kcol_p = i_material->getTevKColor(0);
    if (kcol_p != nullptr) {
        i_tevstr->TevKColor.a = kcol_p->a;
        i_material->setTevKColor(0, (J3DGXColor*)&i_tevstr->TevKColor);
    }

    if (i_material->getFog() != nullptr) {
        J3DFogInfo* fog_info = i_material->getFog()->getFogInfo();
        if (fog_info != nullptr && fog_info->mType != 0) {
            fog_info->mStartZ = i_tevstr->mFogStartZ;
            fog_info->mEndZ = i_tevstr->mFogEndZ;
            if (fog_info->mStartZ > fog_info->mEndZ) {
                fog_info->mStartZ = fog_info->mEndZ;
            }
            view_class* view = dComIfGd_getView();
            if (view != nullptr) {
                fog_info->mNearZ = view->near_;
                fog_info->mFarZ = view->far_;
            }
            fog_info->mColor.r = i_tevstr->FogCol.r;
            fog_info->mColor.g = i_tevstr->FogCol.g;
            fog_info->mColor.b = i_tevstr->FogCol.b;
            fog_info->mAdjEnable = g_env_light.mFogAdjEnable;
            if (fog_info->mAdjEnable == 1) {
                fog_info->mCenter = g_env_light.mFogAdjCenter;
                std::memcpy(&fog_info->mFogAdjTable, &g_env_light.mXFogTbl, sizeof(GXFogAdjTable));
            }
        }
    }
}

void dKyWw_setLightTevColorType(J3DModel* i_model, dKy_tevstr_c* i_tevstr) {
    if (i_model == nullptr || i_model->getModelData() == nullptr || i_tevstr == nullptr) {
        return;
    }
    int mat_num = i_model->getModelData()->getMaterialNum() - 1;
    while (mat_num >= 0) {
        dKyWw_setLightTevColorType_sub(
            i_model->getModelData()->getMaterialNodePointer(mat_num), i_tevstr, true);
        mat_num--;
    }
}

bool dKyWw_tryWwMaji(void* i_dataRaw, void* i_tevstrRaw, int* o_type) {
    J3DModelData* i_data = static_cast<J3DModelData*>(i_dataRaw);
    dKy_tevstr_c* i_tevstr = static_cast<dKy_tevstr_c*>(i_tevstrRaw);
    if (o_type != nullptr) {
        *o_type = (i_tevstr != nullptr) ? (int)i_tevstr->Type : -1;
    }
    if (i_data == nullptr || i_tevstr == nullptr) {
        return false;
    }
    const int t = i_tevstr->Type;
    if (t == 12 || t == 13 || t == 14) {
        return false;
    }
    if (s_palet == nullptr || s_paletN <= 0) {
        return false;
    }
    // ============================================================
    // MODEL -> LIGHT-TYPE LINK - the missing half of the bg3 argument
    // ============================================================
    // History/Bridge measured `Pale[3].bg3 = (0,0,0)` and 36 of 101 BG draws
    // at type 35; I found the receiver picks the type by MODEL SLOT
    // (`d_a_bg.cpp:336`, `l_tevStrType[6] = {32,33,34,35,35,32}`), and room
    // 44 resolves only slots 0 and 1 -> types 32 and 33. So those 36 bg3
    // draws are probably NOT Outset's models - but "probably" is what this
    // receipt exists to remove. Logging the MODEL DATA POINTER beside the
    // type makes the link joinable against the parse/worldize receipts,
    // which already carry the arc name (R44_00 and friends).
    // Sampled per type so all five observed types appear without flooding.
    {
        static int s_typeSeen[64] = {0};
        const int ti = (t >= 0 && t < 64) ? t : 63;
        if (s_typeSeen[ti]++ < 3) {
            kankyoLog("[WwRegistry] {\"ev\":\"bg_model_type\",\"type\":%d,\"bg\":%d,"
                      "\"data\":\"%p\",\"mats\":%d,\"reads\":\"join `data` against the "
                      "model parse/worldize receipts to name the ARC. The receiver picks the "
                      "type by MODEL SLOT (l_tevStrType[6]={32,33,34,35,35,32}); room 44 has "
                      "only slots 0-1, so a type-35 draw here belongs to some OTHER room - "
                      "most likely a pre-warp TP room that was never purged\"}",
                      t, overlayBgIndex(t), (void*)i_data,
                      (int)i_data->getMaterialNum());
        }
    }
    overlayTev(t, i_tevstr);
    // Types 9/10 = Alink. Consume-boundary (vanilla MAJI, not WW _sub flatten):
    // AmbCol from Pale actor_c0; TevColor left 0 (vanilla actor settingTevStruct);
    // no toon stage drop. Do not invent (90,90,90).
    const bool playerLook = overlayIsPlayer(t);
    if (playerLook) {
        static int s_playerLeg = 0;
        s_playerLeg++;
        if (s_playerLeg == 1 || (s_playerLeg % 300) == 0) {
            kankyoLog("[WwRegistry] {\"ev\":\"actor_leg\",\"n\":%d,\"type\":%d,"
                      "\"pale\":[%d,%d],\"amb\":[%d,%d,%d],\"c0\":[%d,%d,%d],"
                      "\"k0\":[%d,%d,%d],\"lm\":%u}",
                      s_playerLeg, t, s_selPal0, s_selPal1, (int)i_tevstr->AmbCol.r,
                      (int)i_tevstr->AmbCol.g, (int)i_tevstr->AmbCol.b,
                      (int)i_tevstr->TevColor.r, (int)i_tevstr->TevColor.g,
                      (int)i_tevstr->TevColor.b, (int)i_tevstr->TevKColor.r,
                      (int)i_tevstr->TevKColor.g, (int)i_tevstr->TevKColor.b,
                      (unsigned)i_tevstr->mLightMode);
        }
    }
    int mat_num = i_data->getMaterialNum() - 1;
    while (mat_num >= 0) {
        J3DMaterial* mat = i_data->getMaterialNodePointer(mat_num);
        if (playerLook) {
            dKyWw_writeAmbCol(mat, i_tevstr);
            dKyWw_setLightTevColorType_sub(mat, i_tevstr, false);
        } else {
            // ============================================================
            // THE BG LEG NOW WRITES AmbCol TOO - the black-vegetation lever
            // ============================================================
            // The player leg has always called writeAmbCol because, as the
            // note above records, "TP player materials consume AmbCol. WW
            // _sub never wrote it." **The receiver's daBg-drawn ROOM models
            // are lit by that same TP path and consume AmbCol identically** -
            // and this branch never wrote it, so their ambient stayed
            // whatever `settingTevStruct` left, which for stage `sea` is
            // nothing: the receiver's own kankyo has no data for a WW stage.
            // Unlit -> BLACK, which is what the user reports on the donor
            // vegetation in room 44 (model.bdl + model1.bdl, daBg path).
            //
            // ELIMINATED FIRST, so this is not the next guess in a queue:
            // `WW_BDL_CONSUME=finish_toon` restored setToonTex, was confirmed
            // ENGAGED in boot 171434, and the vegetation stayed black. The
            // toon-texture explanation is dead; "nothing writes the colour"
            // is what survives.
            //
            // Reversible and self-reporting: WW_BG_AMBCOL=0 restores the old
            // behaviour, and the receipt carries the colour actually written
            // so a black screen with a non-zero write means the lever is
            // right and the VALUE is wrong - a different defect from this one.
            // ============================================================
            static int s_bgAmbEnv = -1;
            if (s_bgAmbEnv < 0) {
                const char* e = std::getenv("WW_BG_AMBCOL");
                // DEFAULT OFF, 2026-08-22 - THIS LEVER IS MISCONCEIVED AND I
                // am leaving it selectable rather than deleting it only so the
                // reasoning stays visible. `applyBgOverlay` writes TevColor and
                // TevKColor and DELIBERATELY NEVER TOUCHES AmbCol; `_sub` then
                // pushes those two into the material. AmbCol is the ACTOR path
                // (TP actor materials consume it), not the BG path. So writing
                // AmbCol here pushes a field the BG draw does not read, using a
                // value the BG overlay never populated - i.e. whatever stale
                // content the tevstr carried. Opt in with WW_BG_AMBCOL=1.
                s_bgAmbEnv = (e != nullptr && e[0] == '1') ? 1 : 0;
            }
            if (s_bgAmbEnv == 1) {
                dKyWw_writeAmbCol(mat, i_tevstr);
                static int s_bgAmbN = 0;
                s_bgAmbN++;
                if (s_bgAmbN == 1 || (s_bgAmbN % 600) == 0) {
                    kankyoLog("[WwRegistry] {\"ev\":\"bg_ambcol\",\"n\":%d,\"type\":%d,"
                              "\"amb\":[%d,%d,%d],\"reads\":\"WW ambient written into a "
                              "daBg-drawn material. amb all-zero here means the OVERLAY is the "
                              "defect, not the write; non-zero + still black means the write "
                              "lands somewhere the draw does not read\"}",
                              s_bgAmbN, t, (int)i_tevstr->AmbCol.r, (int)i_tevstr->AmbCol.g,
                              (int)i_tevstr->AmbCol.b);
                }
            }
            dKyWw_setLightTevColorType_sub(mat, i_tevstr, true);
        }
        mat_num--;
    }
    return true;
}
