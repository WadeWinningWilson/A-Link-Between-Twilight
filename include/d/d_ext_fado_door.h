#ifndef D_EXT_FADO_DOOR_H
#define D_EXT_FADO_DOOR_H

#if TARGET_PC

#include "dolphin/types.h"

class fopAc_ac_c;

// Fado’s Ordon village door (F_SP103 exit 7) — unlock flag + warp-command sink.
// Destination is never hard-coded here; callers register a command. Unlock key is
// mod-folder scoped (default folder "dusklight", key "fado.door.unlock").

bool dFadoDoor_isTargetKnob(fopAc_ac_c* actor);
bool dFadoDoor_isUnlocked();
void dFadoDoor_setUnlocked(bool unlocked);

void dFadoDoor_setWarpCommand(const char* stage, s16 room, s16 spawn, s16 layer = -1,
                              s16 facing = -1);
void dFadoDoor_clearWarpCommand();
bool dFadoDoor_hasWarpCommand();

// dStage_changeScene intercept: 0 = not this exit; 1 = override applied; -1 = blocked.
int dFadoDoor_tryInterceptChangeScene(int exitId, f32 speed, u32 mode, s8 roomNo, s16 angle,
                                      int layerOverride);

#endif  // TARGET_PC

#endif /* D_EXT_FADO_DOOR_H */
