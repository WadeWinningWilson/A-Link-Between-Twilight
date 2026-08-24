#pragma once

#include "dolphin/types.h"

// ============================================
// NEW CODE — ALBW Port (alpha cleanup: warp-menu safety)
// The level-editor warp can land on story-critical spawn points (e.g.
// Lake Hylia spring r1 p20 = "Lanayru twilight cleared", Kakariko r0 p30 =
// "Eldin cleared") whose arrival triggers permanently mutate progression.
// The warp marks one pending suppression; the next stage load consumes it
// and skips those arrival triggers. Transition-scoped only: normal play,
// save/load, and the True-ALBW/TRUETEST bootstrap policies (which run in
// dusk::truetest::onStageLoad, not the vanilla trigger block) are
// unaffected.
// ============================================

namespace dusk::ui {

void markDebugWarpStorySuppress();
bool consumeDebugWarpStorySuppress();

// Room layer override (Cut Actors / Jailer LAYER-0 SPAWN): when >= 0, getLayerNo
// returns this value instead of the story-computed layer. Set by the warp menu
// for one stage load; cleared on the following stage load.
void setRoomLayerOverride(s8 layer);
s8 getRoomLayerOverride();
void onStageLoadRoomLayerOverride();

}  // namespace dusk::ui
