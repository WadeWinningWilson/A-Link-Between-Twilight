#ifndef D_KANKYO_WW_WIND_H
#define D_KANKYO_WW_WIND_H

// ============================================================
// WW WIND FIELD — the §865 parallel-system row (port §868-class). The
// donor's own wind state owned by the WW kankyo lane; TP's
// global_wind_influence untouched. Donor surface: d_kankyo_wether.h
// :353-371, WW-scoped names. Consumers bind THESE, never TP's dKyw_*.
// ============================================================

#include "SSystem/SComponent/c_xyz.h"
#include "d/d_kankyo.h"   // WIND_INFLUENCE (same struct both lineages)

void dKyWw_wind_set();                       // per-frame driver (WW kankyo execute)
void dKyWw_wind_setFiliLevel(int i_level);   // donor FILI GlobalWindLevel feed
cXyz* dKyWw_get_wind_vec();
f32 dKyWw_get_wind_pow();
f32* dKyWw_get_wind_power();
cXyz dKyWw_get_wind_vecpow();
void dKyWw_evt_wind_set(s16 i_windX, s16 i_windY);
void dKyWw_evt_wind_set_go();
void dKyWw_tact_wind_set(s16 i_windX, s16 i_windY);
void dKyWw_pntwind_init();
void dKyWw_pntwind_set(WIND_INFLUENCE*);
void dKyWw_pntwind_cut(WIND_INFLUENCE*);
void dKyWw_pntwind_get_info(const cXyz* i_pos, cXyz* o_dir, f32* o_power);

#endif /* D_KANKYO_WW_WIND_H */
