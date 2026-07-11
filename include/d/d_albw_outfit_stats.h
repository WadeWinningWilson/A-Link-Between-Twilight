#ifndef D_ALBW_OUTFIT_STATS_H
#define D_ALBW_OUTFIT_STATS_H

// ============================================
// NEW CODE — ALBW Port (Outfit Stats)
// Master toggle + query helpers for cloth damage mults, swim buffs, Sumo kit.
// See docs/Outfit Stats.md and docs/Outfit-Stats-Roadmap.md.
// ============================================
#if TARGET_PC

#include "d/d_albw_outfit.h"
#include "dolphin/types.h"

class daAlink_c;

constexpr int kAlbwOutfitStatsZoraGraceFrames = 180 * 30;
constexpr f32 kAlbwOutfitStatsSumoOffensiveDamageMult = 3.0f;

bool dAlbwOutfitStats_isEnabled();

dAlbwOutfitKind dAlbwOutfitStats_getActiveOutfitKind();

// Incoming damage mult when enabled (1.0 = vanilla). Phase B hooks damageMagnification().
f32 dAlbwOutfitStats_getReceivedDamageMult();

// Sumo + wooden sword + no shield offensive kit (Phase E).
bool dAlbwOutfitStats_isSumoOffensiveKitActive();
bool dAlbwOutfitStats_allowsWoodHiddenSkills();
u16  dAlbwOutfitStats_applyOutgoingDamageMult(u16 attackPower);

// Phase C — swim speed (+5% all, +10% Zora) and non-Zora submerged locomotion.
f32 dAlbwOutfitStats_getSwimSpeedMult(const daAlink_c* link);
f32 dAlbwOutfitStats_scaleSwimSpeed(const daAlink_c* link, f32 speed);
// Non-Zora underwater stroke cap (Zora path unchanged; surface swim uses getSwimSpeedMult only).
constexpr f32 kAlbwOutfitStatsSubmergedHumanSwimMult = 0.75f;
f32 dAlbwOutfitStats_getSubmergedHumanSwimSpeedMult(const daAlink_c* link);
bool dAlbwOutfitStats_allowsSubmergedSwim(const daAlink_c* link);
// Non-Zora submerged locomotion active now (outfit-stats dive path).
bool dAlbwOutfitStats_isSubmergedHumanSwim(const daAlink_c* link);
// True only while actually underwater with dive intent. Holding Swim at the
// waterline (shore approach) must return false. Soft-surface uses a tighter
// waterline threshold separately — see swim.inc.
bool dAlbwOutfitStats_hasActiveDiveIntent(const daAlink_c* link);
// Depth below which idle soft-surface is allowed (units).
constexpr f32 kAlbwOutfitStatsWaterlineSoftDepth = 5.0f;

// Non-Zora submerged oxygen drain (+10% vs vanilla 1/frame when Outfit Stats on).
constexpr f32 kAlbwOutfitStatsNonZoraOxygenDrainMult = 1.10f;
void dAlbwOutfitStats_tickNonZoraOxygenDrain();
void dAlbwOutfitStats_resetOxygenDrainAccum();

// Zora water grace timer — call updateSwimState() from Link execute each frame.
bool dAlbwOutfitStats_isZoraWaterBuffActive();
int  dAlbwOutfitStats_getZoraGraceFramesRemaining();
void dAlbwOutfitStats_updateSwimState(class daAlink_c* link);

#endif  // TARGET_PC

#endif /* D_ALBW_OUTFIT_STATS_H */
