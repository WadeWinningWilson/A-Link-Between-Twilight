// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_wether.cpp MatchingFor dKyw_wind_set (80089B48-80089E50),
//            accessors, pntwind ring (8008A0F0-8008A4C8), evt/tact setters.
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Plugin-owned GB_WIND_INFLUENCE twin. Do not write vanilla
// g_env_light.global_wind_influence. FILI is donor 0x8 (vanilla FileList is
// 0x20); we bind the chunk ourselves. Vanilla WIND_INFLUENCE field names
// (position / mDirection / mConstant / field_0x24) at the consume boundary.

#include "ww_kankyo_wind.h"

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/d_kankyo.h"
#include "d/d_com_inf_game.h"
#include "d/d_stage.h"
#include "f_op/f_op_camera_mng.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_xyz.h"

#pragma pack(push, 1)
struct WwFili {
    BE<u32> param;
    BE<f32> seaLevel;
};
#pragma pack(pop)
static_assert(sizeof(WwFili) == 8, "donor dStage_FileList_dt_c");

namespace {

const u32 kTagFili = 0x494C4946u;  // raw 'FILI' as LE u32

struct WwWindState {
    cXyz mWindVec;
    f32 mWindPower;
    cXyz* mpWindVecOverride;
    f32 mWindStrengthOverride;
    f32 mCustomWindPower;
    u8 mEvtWindSet;
    s16 mEvtWindAngleX;
    s16 mEvtWindAngleY;
    u8 mTactWindAngleFlags;
    s16 mTactWindAngleX;
    s16 mTactWindAngleY;
};

WwWindState s_wind;
int s_filiLevel = 0;
f32 s_filiSeaLevel = 0.0f;
f32 s_discStagFar = 0.0f;
WIND_INFLUENCE* s_pntwind[30];

void vectle(const cXyz& a, const cXyz& b, cXyz* o) {
    o->x = b.x - a.x;
    o->y = b.y - a.y;
    o->z = b.z - a.z;
    const f32 len = o->abs();
    if (len > 0.0f) {
        o->x /= len;
        o->y /= len;
        o->z /= len;
    } else {
        o->zero();
    }
}

}  // namespace

void dKyWw_pntwind_init() {
    for (int i = 0; i < 30; i++) {
        s_pntwind[i] = nullptr;
    }
}

void dKyWw_wind_reset() {
    s_wind.mWindVec.set(1.0f, 0.0f, 0.0f);
    s_wind.mWindPower = 0.0f;
    s_wind.mpWindVecOverride = nullptr;
    s_wind.mWindStrengthOverride = 0.0f;
    s_wind.mCustomWindPower = 0.0f;
    s_wind.mEvtWindSet = 0;
    s_wind.mEvtWindAngleX = 0;
    s_wind.mEvtWindAngleY = 0;
    s_wind.mTactWindAngleFlags = 0;
    s_wind.mTactWindAngleX = 0;
    s_wind.mTactWindAngleY = 0;
    s_filiLevel = 0;
    s_filiSeaLevel = 0.0f;
    // Keep disc STAG far across reset — it is arc identity, not room state.
    dKyWw_pntwind_init();
}

int dKyWw_bindFili(void* dzs) {
    if (dzs == nullptr) {
        return -1;
    }
    dStage_fileHeader* file = static_cast<dStage_fileHeader*>(dzs);
    const int n = file->m_chunkCount;
    if (n <= 0 || n > 512) {
        return -1;
    }
    dStage_nodeHeader* node = file->m_nodes;
    for (int i = 0; i < n; i++, node++) {
        if (node->m_tag != kTagFili) {
            continue;
        }
        WwFili* fili = static_cast<WwFili*>(node->m_offset);
        const int entries = node->m_entryNum;
        if (fili == nullptr || entries <= 0 || entries > 64) {
            continue;
        }
        const u32 param = fili->param;
        s_filiLevel = (int)((param >> 18) & 3);
        // Disc bytes are BE. Stage loaders sometimes leave them raw; BE<>
        // converts. Tale §598: Outset Room44 seaLevel is 0 — reject NaN /
        // host-magnitude junk (order 10^5) so the 0.09 dome parallax stays
        // on donor-range inputs.
        f32 sea = fili->seaLevel;
        if (!(sea == sea) || sea > 10000.0f || sea < -10000.0f) {
            sea = 0.0f;
        }
        s_filiSeaLevel = sea;
        return s_filiLevel;
    }
    return -1;
}

int dKyWw_filiLevel() {
    return s_filiLevel;
}

f32 dKyWw_filiSeaLevel() {
    return s_filiSeaLevel;
}

void dKyWw_setDiscStagFar(f32 farz) {
    if (farz >= 20000.0f && farz <= 250000.0f) {
        s_discStagFar = farz;
    }
}

f32 dKyWw_discStagFar() {
    return s_discStagFar;
}

void dKyWw_applyDiscStagFarToCamera() {
    if (s_discStagFar < 20000.0f) {
        return;
    }
    dStage_dt_c* stage = dComIfGp_getStage();
    if (stage == nullptr || stage->getStagInfo() == nullptr) {
        return;
    }
    // BE(f32) store — same bytes the disc carried at STAG +0x04.
    stage->getStagInfo()->mFar = s_discStagFar;
    camera_class* cam = (camera_class*)dComIfGp_getCamera(0);
    if (cam != nullptr) {
        fopCamM_SetFar(cam, s_discStagFar);
    }
    view_class* vw = dComIfGd_getView();
    if (vw != nullptr && vw->far_ != s_discStagFar) {
        vw->far_ = s_discStagFar;
        if (cam != nullptr) {
            C_MTXPerspective(cam->view.projMtx, cam->view.fovy, cam->view.aspect,
                             cam->view.near_, s_discStagFar);
            GXSetProjection(cam->view.projMtx, GX_PERSPECTIVE);
        }
    }
}

void dKyWw_wind_set() {
    cXyz wind_vec;
    f32 strength;

    if (s_wind.mpWindVecOverride != nullptr) {
        wind_vec = *s_wind.mpWindVecOverride;
        strength = s_wind.mWindStrengthOverride;
    } else {
        s16 tact_wind_x;
        s16 tact_wind_y;
        if (s_wind.mEvtWindSet != 0 && s_wind.mEvtWindSet != 0xFF) {
            tact_wind_x = s_wind.mEvtWindAngleX;
            tact_wind_y = s_wind.mEvtWindAngleY;
        } else if (s_wind.mTactWindAngleFlags != 0) {
            tact_wind_x = s_wind.mTactWindAngleX;
            tact_wind_y = s_wind.mTactWindAngleY;
        } else {
            tact_wind_x = 0;
            tact_wind_y = 0;
        }

        wind_vec.x = cM_scos(tact_wind_x) * cM_scos(tact_wind_y);
        wind_vec.y = cM_ssin(tact_wind_x);
        wind_vec.z = cM_scos(tact_wind_x) * cM_ssin(tact_wind_y);

        switch (s_filiLevel) {
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

    if (s_wind.mCustomWindPower > 0.0f) {
        strength = s_wind.mCustomWindPower;
        s_wind.mWindPower = s_wind.mCustomWindPower;
    }

    if (s_wind.mEvtWindSet == 2) {
        strength = 0.0f;
    }

    if (strength > 1.0f) {
        strength = 1.0f;
    }

    if (g_env_light.light_init_timer != 0) {
        s_wind.mWindVec = wind_vec;
        s_wind.mWindPower = strength;
    } else {
        const f32 scale = 0.1f;
        cLib_addCalc(&s_wind.mWindVec.x, wind_vec.x, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wind.mWindVec.y, wind_vec.y, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wind.mWindVec.z, wind_vec.z, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wind.mWindPower, strength, scale, 1.0f, 0.005f);
    }
}

cXyz* dKyWw_get_wind_vec() {
    return &s_wind.mWindVec;
}

f32 dKyWw_get_wind_pow() {
    return s_wind.mWindPower;
}

f32* dKyWw_get_wind_power() {
    return &s_wind.mWindPower;
}

cXyz dKyWw_get_wind_vecpow() {
    cXyz out;
    out.x = s_wind.mWindVec.x * s_wind.mWindPower;
    out.y = s_wind.mWindVec.y * s_wind.mWindPower;
    out.z = s_wind.mWindVec.z * s_wind.mWindPower;
    return out;
}

void dKyWw_custom_windpower(f32 pow) {
    s_wind.mCustomWindPower = pow;
}

void dKyWw_evt_wind_set(s16 i_windX, s16 i_windY) {
    s_wind.mEvtWindAngleX = i_windX;
    s_wind.mEvtWindAngleY = i_windY;
}

void dKyWw_evt_wind_set_go() {
    s_wind.mEvtWindSet = 1;
}

void dKyWw_tact_wind_set(s16 i_windX, s16 i_windY) {
    s_wind.mTactWindAngleX = i_windX;
    s_wind.mTactWindAngleY = i_windY;
    s16 cur_y = -(cM_atan2s(s_wind.mWindVec.x, s_wind.mWindVec.z) + 0x4000);
    s_wind.mTactWindAngleFlags = 1;
    if ((s16)(cur_y - s_wind.mTactWindAngleY) < 0) {
        s_wind.mTactWindAngleFlags |= 0x80;
    }
}

static void pntwind_set(WIND_INFLUENCE* i_influence) {
    int i = 0;
    for (; i < 30; i++) {
        if (s_pntwind[i] == nullptr) {
            s_pntwind[i] = i_influence;
            i_influence->field_0x24 = i;
            break;
        }
    }
    if (i >= 30) {
        i_influence->field_0x24 = 9999;
    }
}

void dKyWw_pntwind_set(WIND_INFLUENCE* i_influence) {
    if (i_influence == nullptr) {
        return;
    }
    i_influence->mConstant = 0;
    pntwind_set(i_influence);
}

void dKyWw_pntwind_cut(WIND_INFLUENCE* i_influence) {
    if (i_influence != nullptr && i_influence->field_0x24 >= 0 &&
        i_influence->field_0x24 < 30) {
        s_pntwind[i_influence->field_0x24] = nullptr;
    }
}

void dKyWw_pntwind_get_info(cXyz* i_pos, cXyz* i_dir, f32* i_power) {
    i_dir->x = 0.0f;
    i_dir->y = 0.0f;
    i_dir->z = 0.0f;
    *i_power = 0.0f;
    if (i_pos == nullptr) {
        return;
    }

    for (int i = 0; i < 30; i++) {
        WIND_INFLUENCE* influence = s_pntwind[i];
        if (influence == nullptr) {
            continue;
        }
        const f32 dist = i_pos->abs(influence->position);
        if (influence->mConstant == 0) {
            if (dist < influence->mRadius && influence->mStrength > 0.0f && dist != 0.0f) {
                f32 temp_f0;
                if (influence->mRadius > 0.0f) {
                    const f32 t = dist / influence->mRadius;
                    temp_f0 = 1.0f - (t * t);
                } else {
                    temp_f0 = 1.0f;
                }
                *i_power = influence->mStrength * temp_f0;
                cXyz sp10;
                sp10.x = influence->position.x + -(influence->mDirection.x) * influence->mRadius;
                sp10.y = influence->position.y + -(influence->mDirection.y) * influence->mRadius;
                sp10.z = influence->position.z + -(influence->mDirection.z) * influence->mRadius;
                vectle(sp10, *i_pos, i_dir);
                if (i_pos->abs(sp10) < influence->mRadius) {
                    i_dir->zero();
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
