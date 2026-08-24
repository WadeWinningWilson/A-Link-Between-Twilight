#pragma once

#if TARGET_PC
// C linkage so JSystem can read world pace without pulling in game headers.
extern "C" float dusk_world_sim_time_scale;
#endif

namespace dusk {

// World sim pace (enemies / non-Link actors). Link stays at 1.0x during Flurry Rush.
float getSimTimeScale();
void setSimTimeScale(float i_scale);

}  // namespace dusk
