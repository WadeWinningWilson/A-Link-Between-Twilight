#ifndef D_KANKYO_WW_SKY_H
#define D_KANKYO_WW_SKY_H

// ============================================================
// §413 WW CELESTIAL LAYER -- donor-verbatim sun / moon+phases / lens flare
// / stars, selected by dKyWw_isSkyHost(). The receiver's TP evolutions of
// these systems stay untouched for TP stages; on a WW host the wether
// dispatchers route here instead (covenant: no TP flavoring in WW spaces).
// Sources: WW DP d_kankyo_wether.cpp / d_kankyo_rain.cpp (verbatim bodies,
// extracted with citations, bus §413); noclip d_kankyo_wether.ts as the
// structural cross-check (its change-notes name only windline/vrkumo/wave
// tweaks -- nothing in these systems).
// ============================================================

#include "dolphin/types.h"

void dKyWwSky_moveSun();    // donor wether_move_sun + dKyr_sun_move + lenzflare_move
void dKyWwSky_moveStar();   // donor wether_move_star + dKyr_star_move
void dKyWwSky_drawSun();    // donor dKyw_Sun_Draw + dKyw_Sunlenz_Draw (order load-bearing)
void dKyWwSky_drawStar();   // donor dKyw_Star_Draw
bool dKyWwSky_sunReady();
bool dKyWwSky_starReady();
// §413 re-init: the wether systems latch host-stage resources at frame 1,
// BEFORE the mount flips the sky host flag (vrkumo hard-fails to status 99 if
// cloudtx was not yet resident). Called from dKyWw_setSkyHost transitions.
void dKyWwSky_reset();

#endif
