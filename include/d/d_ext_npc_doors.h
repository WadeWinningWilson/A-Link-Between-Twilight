#ifndef D_EXT_NPC_DOORS_H
#define D_EXT_NPC_DOORS_H

#if TARGET_PC

#include "f_op/f_op_actor.h"

struct dExtNpcManifest;
class dExtNpcMount_c;

// Folder-side door triggers (population/doors.ini).
// №53: TP-native OPEN (CANDOOR + orderDoorEvent) wraps the pinned-BG warp backend.
void dExtNpcDoors_poll();
// №89: destination-owned arrival demo + event G-guard (call every play-scene frame).
void dExtNpcDoors_pollArrival();
// №90: arm 120f event G-guard only (no door demo) — warp / any WW-host arrival.
// No-op if a door-lane arrival demo is already armed for this stage.
void dExtNpcDoors_armArrivalGuard(const char* stage);
// №90: after interior BG COMPLEATE on a WW host — spawn exit knob for this proc.
void dExtNpcDoors_onInteriorBgReady(const char* interiorProc);
// №94: clear door/exit spawn latches (world-gen bump / restart).
void dExtNpcDoors_clearSpawnLatches();
// №94: outdoor knobs claimed spawned for current world generation.
bool dExtNpcDoors_knobsLatched();
// №94 self-heal: count live outdoor door props (`door:` src / doorAttention).
int dExtNpcDoors_countLiveOutdoorKnobs();
int dExtNpcDoors_wantOutdoorKnobCount();
// Spawn outdoor Knob props once per island session (doors.ini order).
void dExtNpcDoors_spawnKnobs(const dExtNpcManifest& island);
// True if actor is a WW mount doorAttention knob (enter or exit), or §27 KNOB00.
bool dExtNpcDoors_isMountDoor(fopAc_ac_c* actor);
// §27: stamp door key onto a live KNOB00 (reconcile / spawn).
void dExtNpcDoors_stampKnob00(fopAc_ac_c* knob, const char* doorKey);
// №53-A/B: called from doorCheck when no stage event archive — queue pinned warp.
// №91: openAlreadyDone=true when the door already owned DEFAULT_KNOB_* + cutEnd
// (skip playAnimNearest / demo-lock / 28f hold — warp immediately).
bool dExtNpcDoors_tryNativeWarp(fopAc_ac_c* doorActor, bool openAlreadyDone = false);
// №53-D: stamp key + DOOR attention onto a knob from doors.ini index (1-based in params).
void dExtNpcDoors_stampKnobByIndex(dExtNpcMount_c* knob, int doorIndex1Based);
// №66-B: room-lane unload drops the exit knob — allow respawn on next enter.
void dExtNpcDoors_clearExitKnobForProc(const char* interiorProc);

#endif  // TARGET_PC

#endif /* D_EXT_NPC_DOORS_H */
