#ifndef D_CUT_ACTOR_SPAWN_H
#define D_CUT_ACTOR_SPAWN_H

#if TARGET_PC

#include "dolphin/types.h"

// Cut / stub actor spawn presets for the ALBW editor (Actor Spawner lane).
// Real enemies (E_ms, E_dt, …) create live procs; stub RELs may spawn empty shells.

namespace dCutActorSpawn {

struct Entry {
    const char* label;
    s16 actorId;
    s16 angleX;  // high byte selects E_OC2 mesh when actor is E_OC
    u32 params;
    bool snapToGround;
    const char* note;
};

int entryCount();
const Entry* entry(int index);
int selectedIndex();
void setSelectedIndex(int index);

// Spawns at Link's feet (field only). Returns true if create ID is valid.
bool requestSpawn();
// Deletes every actor this tool still tracks (safe if already gone / room-changed).
void requestDespawn();
// Poll create-phase outcome for the latest spawn (COMPLEATE / ERROR / TIMEOUT).
void tick();
int trackedCount();
const char* status();

}  // namespace dCutActorSpawn

#endif  // TARGET_PC

#endif  // D_CUT_ACTOR_SPAWN_H
