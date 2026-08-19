#pragma once

#include "SSystem/SComponent/c_xyz.h"

// Plugin-hosted WW whitecap foam panes (water taxonomy system 2 / §97b).
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_rain.cpp wave_move + drawWave; d/d_kankyo.cpp
//            dKy_usonami_set / dKy_get_seacolor (BG1 C0/K0);
//            daSea CalcFlatInterTarget (MULT wave_max / GetArea);
//            kytag01 WAVE_INFO → mpWaveInfl
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Plugin-local WAVECHAN + packet (vanilla g_env_light has no mWaveChan).
// Arms on stage "sea". flatInter from MULT mWaveMax grid; coast kill from
// live ky_tag1 (kytag01). No d_a_sea mesh (system 1).

// Donor WAVE_INFO (d_kankyo.h) — owned by kytag01; registered into foam.
struct WwWaveInfl {
    cXyz mPos;
    f32 mOuterRadius;
    f32 mInnerRadius;
    f32 field_0x14;
};

void wwWave_reset();
void wwWave_move();
void wwWave_drawQueue();

// kytag01 create/delete — same slot table as donor mpWaveInfl[10].
bool wwWave_registerInfl(WwWaveInfl* info);
void wwWave_unregisterInfl(WwWaveInfl* info);
