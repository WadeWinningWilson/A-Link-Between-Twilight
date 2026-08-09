#ifndef DUSK_BOOT_STAGE_H
#define DUSK_BOOT_STAGE_H

// ============================================================================
// --stage boot warp — dev entry into a stage that has no in-game route yet.
//
// WHY THIS EXISTS
// A freshly baked stage has no way in. No SCLS record targets it, no warp row,
// no door. That is how R_DL02's room-load seam sat "untested" while reading as
// passed: the plan had steps for building the stage and none for REACHING it.
//
// WHY THIS SHAPE AND NOT THE ALTERNATIVES (user ruling, 2026-08-09)
//
//   (a) warp-menu row  REFUTED BY CRASH. The menu warps to a BG MOUNT, not a
//       stage: a row is only valid with arc= and collision=, so a payload is
//       structural. Mounting Ojhous2 into R_DL02 crashed in
//       mDoExt_setupStageTexture -- the payload resolves imageOffset==0
//       textures via dComIfG_getStageRes(), and a dedicated bake does not carry
//       the donor stage's resources. Fatal, not merely noisy.
//
//   (b) temp SCLS retarget  REJECTED ON DOCTRINE. SCLS is the native exit
//       mechanism, and (b) would use it to assert a record Wind Waker does not
//       have. SCLS is precisely what the census, the extractor and the
//       conversion DB READ -- a fabricated row is indistinguishable from a real
//       one later, and it pollutes the data the translators are validated
//       against. It also carries a "must be reverted" obligation, which is the
//       failure class this campaign keeps rediscovering.
//
//   (c) THIS. Touches no donor data, so there is nothing to revert and nothing
//       to pollute. Honest scaffolding beside the game rather than a false
//       record inside it.
//
// COVENANT: adds no donor bytes and no WW identity literal. The stage id
// arrives as a RUNTIME ARGUMENT and is never compiled in, which is History A1's
// "no WW place-names in the exe" satisfied by construction rather than by
// review. The flag is also WW-agnostic -- it warps to any stage id, so it is
// dusklight dev tooling, not a WW leg.
//
// USAGE:  dusklight.exe --stage R_DL02          (room 0, layer -1)
//         dusklight.exe --stage R_DL02,1        (room 1)
//         dusklight.exe --stage R_DL02,0,2      (room 0, layer 2)
//
// ONE-SHOT. It fires once, after the play scene is running, then disarms. It
// does not re-fire on later stage changes, so it cannot trap you in a loop.
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Arms the boot warp from the parsed command line. Safe to call with NULL or
// an empty string, which arms nothing.
void dBootStage_arm(const char* spec);

// Per-frame. No-op unless armed AND the play scene is ready. Fires once.
void dBootStage_poll(void);

#ifdef __cplusplus
}
#endif

#endif  // DUSK_BOOT_STAGE_H
