/**
 * d_albw_region_mult.h
 * ALBW Port — Region Multipliers (province / dungeon table).
 *
 * Lookup: (stage, room) overrides → stage-name overrides → SaveTbl.
 * Master toggle plus independent Damage / Health / Rupees axes.
 * See Gameplay → Region Multipliers.
 */

#pragma once

#if TARGET_PC

#include "dolphin/types.h"

// ============================================
// NEW CODE — ALBW Port (Region Multipliers)
// ============================================

bool dAlbwRegionMult_isEnabled();

// Raw table value for the current stage (1.0 if unknown / no stag info).
f32 dAlbwRegionMult_getTableMult();

// Axis getters: 1.0 when master or axis is off.
f32 dAlbwRegionMult_getDamageMult();
f32 dAlbwRegionMult_getHealthMult();
f32 dAlbwRegionMult_getRupeeMult();

// Round HP after category true-HP scaling. No-op when health axis inactive.
s16 dAlbwRegionMult_scaleHp(s16 hp);

// Round enemy-death / fight-victory rupee grants. No-op when rupee axis inactive.
u16 dAlbwRegionMult_scaleRupees(u16 amount);

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif  // TARGET_PC
