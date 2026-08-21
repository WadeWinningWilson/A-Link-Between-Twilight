/**
 * d_albw_region_mult.h
 * ALBW Port — Region Multipliers (province / dungeon table).
 *
 * Lookup: (stage, room) overrides → stage-name overrides → SaveTbl.
 * Region Damage is a standalone toggle. Region Multipliers master gates
 * Health / Rupees only. See Gameplay → Region Damage / Region Multipliers.
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

// Damage: 1.0 when game.regionDamage is off (no master required).
f32 dAlbwRegionMult_getDamageMult();
// Health / Rupees: 1.0 when master or axis is off.
f32 dAlbwRegionMult_getHealthMult();
f32 dAlbwRegionMult_getRupeeMult();
// Region Damage On → ×3 on enemy-death / fight-victory grants (1.0 when Off).
f32 dAlbwRegionMult_getRegionDamageRupeeMult();

// Round HP after category true-HP scaling. No-op when health axis inactive.
s16 dAlbwRegionMult_scaleHp(s16 hp);

// Round enemy-death / fight-victory rupee grants.
// Applies RD ×3 (if On) × RM table (if master+rupees On). Shops unchanged.
u16 dAlbwRegionMult_scaleRupees(u16 amount);

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif  // TARGET_PC
