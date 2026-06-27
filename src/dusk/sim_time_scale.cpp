#include "dusk/sim_time_scale.h"

namespace dusk {

float g_simTimeScale = 1.0f;

float getSimTimeScale() {
    return g_simTimeScale;
}

void setSimTimeScale(float i_scale) {
    g_simTimeScale = i_scale;
#if TARGET_PC
    dusk_world_sim_time_scale = i_scale;
#endif
}

}  // namespace dusk

#if TARGET_PC
extern "C" float dusk_world_sim_time_scale = 1.0f;
#endif
