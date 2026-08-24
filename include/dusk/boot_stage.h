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
// WHAT (a) ACTUALLY COST US, recorded because the first diagnosis was wrong:
// the crash (a) produced was blamed on the forced BG payload's stage textures.
// It was not. With the payload removed the same crash reproduced, and the real
// cause was the bake's RARC node type ('BDL ' has no branch in dRes_info_c, so
// the room model was never parsed). (a) was still the wrong shape, but it was
// not the thing that crashed.
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
// ARMS A DESTINATION; IT DOES NOT TRAVEL (user ruling, 2026-08-09). The flag
// adds a button to the warp window's dev section and nothing else. The first
// revision fired by itself ~1s into the play scene, which is worse than it
// sounds: it moves during whatever the save was mid-way through -- the first
// run landed on top of an entry demo -- so a fault in the destination is
// indistinguishable from a fault caused by interrupting the origin. Warping on
// purpose, from a settled scene, is both what was asked for and the only way
// the destination is the only variable.
//
// The stage id is still never a compiled literal: the button's LABEL is the
// runtime string, the same rule the No99 R2 manifest warp rows follow.
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Arms destinations from the parsed command line. ';'-separated, so one flag
// can arm several:  --stage "sea,44;R_DL02". Safe with NULL or empty.
void dBootStage_arm(const char* spec);

// §638: add a destination from anywhere else — a content layer registering a
// stage it DECLARES in data, so a declared stage is reachable without the
// command line naming it too. Same NAME[,room[,layer]] grammar. Duplicates of
// an already-listed (stage, room) are ignored.
void dBootStage_add(const char* spec);

// How many destinations are listed, and the label for one. The label is the
// runtime string, which is what keeps the stage id out of the executable.
int dBootStage_count(void);
const char* dBootStage_labelAt(int i);

// Travel to destination i. Returns false (and logs why) if the index is out of
// range, a stage change is already queued, or it is already the current stage.
bool dBootStage_warpAt(int i);

#ifdef __cplusplus
}
#endif

#endif  // DUSK_BOOT_STAGE_H
