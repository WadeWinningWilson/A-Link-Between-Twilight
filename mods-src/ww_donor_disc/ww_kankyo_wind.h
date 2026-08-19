#pragma once

// WW wind field — donor dKyw_wind_set / accessors / pntwind, owned in this
// plugin. Vanilla g_env_light.global_wind_influence is never written.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_wether.cpp MatchingFor (dKyw_wind_set 80089B48;
//            pntwind 8008A0F0-8008A4C8; evt/tact setters)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db

#include "SSystem/SComponent/c_xyz.h"

struct WIND_INFLUENCE;

void dKyWw_wind_set();
void dKyWw_wind_reset();
int dKyWw_bindFili(void* dzs);  // donor FILI 0x8; returns level 0-3, or -1
int dKyWw_filiLevel();
f32 dKyWw_filiSeaLevel();  // donor FILI mSeaLevel; 0 if unbound

// Disc STAG mFar (BE @ +0x04), sniffed at serve. Donor camera/vrkumo use it.
void dKyWw_setDiscStagFar(f32 farz);
f32 dKyWw_discStagFar();
// Feed disc STAG mFar into getStagInfo so camera's native fopCamM_SetFar
// (d_camera.cpp) writes view->far_ — Housing §417 [D3] chain, not a draw bake.
void dKyWw_applyDiscStagFarToCamera();

cXyz* dKyWw_get_wind_vec();
f32 dKyWw_get_wind_pow();
f32* dKyWw_get_wind_power();
cXyz dKyWw_get_wind_vecpow();

void dKyWw_evt_wind_set(s16 i_windX, s16 i_windY);
void dKyWw_evt_wind_set_go();
void dKyWw_tact_wind_set(s16 i_windX, s16 i_windY);
// Donor custom_windpower / Ferry F sea ambient (FILI windLevel 0 → 0.3).
void dKyWw_custom_windpower(f32 pow);

void dKyWw_pntwind_init();
void dKyWw_pntwind_set(WIND_INFLUENCE* i_influence);
void dKyWw_pntwind_cut(WIND_INFLUENCE* i_influence);
void dKyWw_pntwind_get_info(cXyz* i_pos, cXyz* i_dir, f32* i_power);
