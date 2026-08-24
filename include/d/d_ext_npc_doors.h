#ifndef D_EXT_NPC_DOORS_H
#define D_EXT_NPC_DOORS_H

#if TARGET_PC

struct dExtNpcManifest;
class fopAc_ac_c;
class dExtNpcMount_c;

inline void dExtNpcDoors_poll() {}
inline void dExtNpcDoors_pollArrival() {}
inline void dExtNpcDoors_armArrivalGuard(const char*) {}
inline void dExtNpcDoors_requestPostOpeningSnap(const char*) {}
inline void dExtNpcDoors_onInteriorBgReady(const char*) {}
inline void dExtNpcDoors_clearSpawnLatches() {}
inline bool dExtNpcDoors_knobsLatched() { return false; }
inline int dExtNpcDoors_countLiveOutdoorKnobs() { return 0; }
inline int dExtNpcDoors_wantOutdoorKnobCount() { return 0; }
inline void dExtNpcDoors_spawnKnobs(const dExtNpcManifest&) {}
inline bool dExtNpcDoors_isMountDoor(fopAc_ac_c*) { return false; }
inline void dExtNpcDoors_stampKnob00(fopAc_ac_c*, const char*) {}
inline bool dExtNpcDoors_tryNativeWarp(fopAc_ac_c*, bool = false) { return false; }
inline void dExtNpcDoors_stampKnobByIndex(dExtNpcMount_c*, int) {}
inline void dExtNpcDoors_clearExitKnobForProc(const char*) {}

#endif

#endif
