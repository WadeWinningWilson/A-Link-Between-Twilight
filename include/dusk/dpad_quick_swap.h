#pragma once

#include "dolphin/types.h"

namespace dusk {

#if TARGET_PC
void applyDpadQuickSwapPresetBinds();

bool canUseDpadQuickSwap(u32 port = 0);

void cycleNextSword();
void cycleNextShield();
void cycleNextOutfit();

/** Left d-pad: single tap = transform, double tap = Call Midna (Quick Swap + Midna on Left). */
bool isLayeredLeftDpadActive(u32 port = 0);
void tickLayeredLeftDpad(u32 port = 0);
bool consumeLayeredLeftMidnaTrig(u32 port = 0);
#endif

}
