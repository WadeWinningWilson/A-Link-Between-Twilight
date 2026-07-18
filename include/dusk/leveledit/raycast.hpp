#pragma once

// ============================================================================
// Level Editor — camera→cursor ray picking (Gate 11a).
// Builds the world-space pick ray from the cursor and tests it against each
// actor's cull volume (OBB for box cull types via the inverse model matrix,
// sphere for sphere cull types). Gives pinpoint, depth-ordered selection with
// no screen-space offset — the front-most volume under the cursor wins.
// ============================================================================

#include "SSystem/SComponent/c_xyz.h"

class fopAc_ac_c;

namespace dusk::leveledit {

// ============================================================================
// Build the world-space pick ray from a cursor position expressed in the
// *game-screen* space that mDoLib_project outputs (i.e. pick.cpp's cursorX/Y,
// already mirror-adjusted). Origin is the near-plane point under the cursor,
// dir is unit. Returns false if the view/projection is unavailable or the
// resulting ray is degenerate.
// ============================================================================
bool build_pick_ray(f32 cursorGameX, f32 cursorGameY, cXyz& outOrigin, cXyz& outDir);

// ============================================================================
// Nearest ray entry into the actor's cull volume. outT is the world-space
// distance along the (unit) ray to the entry point (or ~0 if the origin is
// already inside the volume). Returns false on miss / no usable volume.
// ============================================================================
bool ray_hits_actor(const fopAc_ac_c* actor, const cXyz& origin, const cXyz& dir, f32& outT);

// ============================================================================
// Draw a model-tight selection/hover highlight: one translucent box per model
// joint (hugs the actor's shape) instead of the single oversized cull box.
// Falls back to the cull box for actors with no model / no joint bounds. Color
// is passed as components to keep GXColor out of this header.
// ============================================================================
void draw_actor_volume_highlight(const fopAc_ac_c* actor, u8 r, u8 g, u8 b, u8 a);

}  // namespace dusk::leveledit
