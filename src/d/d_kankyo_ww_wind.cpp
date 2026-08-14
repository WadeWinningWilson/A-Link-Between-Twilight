// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_wether.cpp:985-1140 (dKyw wind field + accessors + pntwind)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW WIND FIELD — the §865 system row (user rule: "we don't count on TP's
// systems, we want WW systems running parallel"). The donor's OWN wind
// state, ported WHOLE into the WW kankyo lane: owned vec+power (TP's
// g_env_light.global_wind_influence is NEVER touched), source priority
// override > evt-wind > tact-wind angles, FILI GlobalWindLevel strength
// tiers (0.3/0.6/0.9), custom-power override, evt-wind kill, per-frame
// smoothing — all donor-verbatim. Point-wind (WIND_INFLUENCE ring) carried
// with the donor's accessor shape.
//
// CONSUMERS bind dKyWw_* (lpalm/lwood tree sway re-pointed in their dark
// TUs; agb/bridge/dai_item when they port). ABSORB/RETIRE at land (§867's
// inventory): (1) Ferry-F's dKyw_ww_host_wind_onStage re-targets its donor
// FILI wind INTO this field and its TP evt_wind carrier retires; (2) the
// §192 windline streak block reads THIS field for direction/power;
// (3) donor actors stop binding TP's dKyw_get_wind_*.
// DRIVER (land step): dKyWw_wind_set() called once per frame from the WW
// kankyo execute beside the other dKyWw passes.
// ============================================================

#include "d/d_kankyo_ww_wind.h"

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "dusk/logging.h"

// ============================================================
// the WW-owned wind state (donor g_env_light.mWind, field-for-field, but
// OWNED HERE — the parallel-system law).
// ============================================================
struct dKyWw_wind_state_c {
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
    u8 mInitTimer;   // donor mInitAnimTimer gate: snap while nonzero

    dKyWw_wind_state_c() {
        mWindVec.set(1.0f, 0.0f, 0.0f);
        mWindPower = 0.0f;
        mpWindVecOverride = NULL;
        mWindStrengthOverride = 0.0f;
        mCustomWindPower = 0.0f;
        mEvtWindSet = 0;
        mEvtWindAngleX = 0;
        mEvtWindAngleY = 0;
        mTactWindAngleFlags = 0;
        mTactWindAngleX = 0;
        mTactWindAngleY = 0;
        mInitTimer = 2;   // snap on the first frames like the donor's init window
    }
};
static dKyWw_wind_state_c s_wwWindState;

// donor pntwind ring (WIND_INFLUENCE registrations) — the donor keeps a
// registration list; capacity mirrors the donor's small working set.
static WIND_INFLUENCE* s_pntwind[8];

// ============================================================
// [W1] donor FILI GlobalWindLevel: the WW room's own file-list carries the
// stage wind tier. Ferry-F already reads the donor FILI for its arm (§867
// seam 1, run receipt "donor wind ARM ... pow=0.300"); at land that read
// RE-TARGETS here (dKyWw_wind_setFiliLevel) and its TP carrier retires.
// Until the re-target lands, the level defaults to the donor's tier-0.
// ============================================================
static int s_filiWindLevel = 0;
void dKyWw_wind_setFiliLevel(int i_level) {
    s_filiWindLevel = i_level;
}

// donor dKyw_wind_set (d_kankyo_wether.cpp:986) — verbatim on the owned state.
void dKyWw_wind_set() {
    cXyz wind_vec;
    f32 strength;

    if (s_wwWindState.mpWindVecOverride != NULL) {
        wind_vec = *s_wwWindState.mpWindVecOverride;
        strength = s_wwWindState.mWindStrengthOverride;
    } else {
        s16 tact_wind_x;
        s16 tact_wind_y;
        if (s_wwWindState.mEvtWindSet != 0 && s_wwWindState.mEvtWindSet != 0xFF) {
            tact_wind_x = s_wwWindState.mEvtWindAngleX;
            tact_wind_y = s_wwWindState.mEvtWindAngleY;
        } else if (s_wwWindState.mTactWindAngleFlags != 0) {
            tact_wind_x = s_wwWindState.mTactWindAngleX;
            tact_wind_y = s_wwWindState.mTactWindAngleY;
        } else {
            tact_wind_x = 0;
            tact_wind_y = 0;
        }

        wind_vec.x = cM_scos(tact_wind_x) * cM_scos(tact_wind_y);
        wind_vec.y = cM_ssin(tact_wind_x);
        wind_vec.z = cM_scos(tact_wind_x) * cM_ssin(tact_wind_y);

        switch (s_filiWindLevel) {   // [W1]
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

    if (s_wwWindState.mCustomWindPower > 0.0f) {
        strength = s_wwWindState.mCustomWindPower;
        s_wwWindState.mWindPower = s_wwWindState.mCustomWindPower;
    }

    if (s_wwWindState.mEvtWindSet == 2) {
        strength = 0.0f;
    }

    if (strength > 1.0f) {
        strength = 1.0f;
    }

    if (s_wwWindState.mInitTimer != 0) {
        s_wwWindState.mInitTimer--;
        s_wwWindState.mWindVec = wind_vec;
        s_wwWindState.mWindPower = strength;
    } else {
        f32 scale = 0.1f;
        cLib_addCalc(&s_wwWindState.mWindVec.x, wind_vec.x, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wwWindState.mWindVec.y, wind_vec.y, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wwWindState.mWindVec.z, wind_vec.z, scale, 2.0f, 0.001f);
        cLib_addCalc(&s_wwWindState.mWindPower, strength, scale, 1.0f, 0.005f);
    }
}

// donor accessors (d_kankyo_wether.cpp:1087-1110) — WW-side twins.
cXyz* dKyWw_get_wind_vec() {
    return &s_wwWindState.mWindVec;
}

f32 dKyWw_get_wind_pow() {
    return s_wwWindState.mWindPower;
}

f32* dKyWw_get_wind_power() {
    return &s_wwWindState.mWindPower;
}

cXyz dKyWw_get_wind_vecpow() {
    return s_wwWindState.mWindVec * s_wwWindState.mWindPower;
}

// donor evt/tact setters (the kytag/event surface, WW-side).
void dKyWw_evt_wind_set(s16 i_windX, s16 i_windY) {
    s_wwWindState.mEvtWindSet = 1;
    s_wwWindState.mEvtWindAngleX = i_windX;
    s_wwWindState.mEvtWindAngleY = i_windY;
}

void dKyWw_evt_wind_set_go() {
    s_wwWindState.mEvtWindSet = 0;
}

void dKyWw_tact_wind_set(s16 i_windX, s16 i_windY) {
    s_wwWindState.mTactWindAngleFlags = 1;
    s_wwWindState.mTactWindAngleX = i_windX;
    s_wwWindState.mTactWindAngleY = i_windY;
}

// donor pntwind family (registration ring + nearest-influence query shape).
void dKyWw_pntwind_init() {
    for (int i = 0; i < 8; i++) {
        s_pntwind[i] = NULL;
    }
}

void dKyWw_pntwind_set(WIND_INFLUENCE* i_wind) {
    for (int i = 0; i < 8; i++) {
        if (s_pntwind[i] == i_wind) {
            return;
        }
    }
    for (int i = 0; i < 8; i++) {
        if (s_pntwind[i] == NULL) {
            s_pntwind[i] = i_wind;
            return;
        }
    }
    DuskLog.warn("[WwWindField] pntwind ring full — influence dropped (donor cap mirrored)");
}

void dKyWw_pntwind_cut(WIND_INFLUENCE* i_wind) {
    for (int i = 0; i < 8; i++) {
        if (s_pntwind[i] == i_wind) {
            s_pntwind[i] = NULL;
        }
    }
}

// donor dKyw_pntwind_get_info: strongest in-radius influence at a position.
void dKyWw_pntwind_get_info(const cXyz* i_pos, cXyz* o_dir, f32* o_power) {
    o_dir->set(0.0f, 0.0f, 0.0f);
    *o_power = 0.0f;
    for (int i = 0; i < 8; i++) {
        WIND_INFLUENCE* w = s_pntwind[i];
        if (w == NULL) {
            continue;
        }
        cXyz diff = *i_pos - w->position;
        f32 dist = diff.abs();
        if (dist < w->mRadius && w->mRadius > 0.0f) {
            f32 fall = 1.0f - (dist / w->mRadius);
            f32 p = w->mStrength * fall;
            if (p > *o_power) {
                *o_power = p;
                *o_dir = w->mDirection;
            }
        }
    }
}
