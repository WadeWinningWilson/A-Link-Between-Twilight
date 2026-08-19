#pragma once

// WW lighting write path — donor d_kankyo.cpp settingTevStruct tail +
// setLightTevColorType(_sub). Compiled in this plugin against vanilla
// headers. Not a copy of fork d_kankyo_ww.cpp (mixed, mWwColorK1 append,
// mount flags).
//
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo.cpp MatchingFor (setLightTevColorType 80193650-80193ADC;
//            settingTevStruct C0/K0 write 80193028)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db

class J3DModel;
class cXyz;
class dKy_tevstr_c;

#ifndef TEV_TYPE_ACTOR
#define TEV_TYPE_ACTOR 0
#endif
#ifndef TEV_TYPE_BG0
#define TEV_TYPE_BG0 1
#define TEV_TYPE_BG1 2
#define TEV_TYPE_BG2 3
#define TEV_TYPE_BG3 4
#define TEV_TYPE_BG0_FULL 5
#define TEV_TYPE_BG1_FULL 6
#define TEV_TYPE_BG2_FULL 7
#define TEV_TYPE_BG3_FULL 8
#define TEV_TYPE_PLAYER 9
#define TEV_TYPE_UNK99 99
#endif

// Bind donor Pale (0x2C) from a loaded DZS/DZR. Vanilla looks for PAL0 (0x34)
// and never matches "Pale". We keep the donor layout; we do not stuff it into
// TP's stage_palette_info_class.
int wwKankyo_bindDzs(void* dzs);

void wwKankyo_reset();

int wwKankyo_paletN();
int wwKankyo_virtN();
int wwKankyo_envrN();
int wwKankyo_coloN();

// Donor Virt (0x24) colors for the plugin vrbox actors. Not written into
// vanilla g_env_light.vrbox_* (TP layout / hide_vrbox).
void wwKankyo_virt_set();
void wwKankyo_vrboxGet(unsigned char sky[4], unsigned char kasumi[4],
                       unsigned char kumo[4], unsigned char uso[4], int* invisible);
// Horizon + center cumulus colors (Virt Kumo / KumoCenter). For §417 draw.
void wwKankyo_vrkumoColorsGet(unsigned char horizon[4], unsigned char center[4]);

void dKyWw_settingTevStruct(int i_lightType, cXyz* i_pos, dKy_tevstr_c* i_tevstr);
void dKyWw_setLightTevColorType(J3DModel* i_model, dKy_tevstr_c* i_tevstr);

// Pale C0/K0 from EnvR/Colo/schedule (same palettes as virt_set). Does not
// call vanilla. Actor types use now_actcol_ratio²; BG uses now_bgcol_ratio.
// Types 9/10: AmbCol only (TevColor stays 0 — TP Alink MAJI consume-boundary).
void dKyWw_overlayTevStruct(int i_lightType, dKy_tevstr_c* i_tevstr);

// Donor dKy_get_seacolor: amb=BG1_C0(+addAmb), dif=BG1_K0. Reads bound Pale
// (not TP bg_amb_col / dungeonlight stash — those are the fork №113 path).
void wwKankyo_getSeacolor(unsigned char amb[4], unsigned char dif[4]);

// WW + Pale bound: overlay C0/K0, write donor materials, caller skips _MAJI.
// Types 9/10 (player): Pale AmbCol + light/fog; TevColor left 0; no stage drop.
bool dKyWw_tryWwMaji(void* i_data, void* i_tevstr, int* o_type);
