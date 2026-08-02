#pragma once

// Ferry T / T2 — one-probe main-loop time buckets (1 Hz log).
// T2: frameMs = loop-to-loop wall delta; unaccounted = frame − (exe+draw+present).
// Kill: DUSK_FPS_PROBE=0  |  Force on: unset or any other value.

#if TARGET_PC

#include <cstdint>

namespace dusk {
namespace fps_probe {

enum class Bucket : int {
    Mount = 0,       // dExtNpcMount_pollBgWarps (+ provider warm path)
    Doors,           // dExtNpcDoors_poll (incl. pollArrival)
    DemoMsg,         // dExtWw_pollDemoMessage
    FrameInterp,     // record + interpolate
    Kankyo,          // dKyw_wether_move (rain/wave execute)
    MenuRes,         // dAlbwMenuRes_drive (custom-assets coexist lane)
    Wind,            // dKyw_ww_host_wind_onStage
    Count
};

std::int64_t now_ticks();
double ticks_to_ms(std::int64_t ticks);

struct Scope {
    Bucket bucket;
    std::int64_t t0;
    explicit Scope(Bucket b);
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};

void begin_frame();
// Non-interp: fpcM_Management reports exe/draw parts for this frame.
void note_fpcm_split(std::int64_t exe_ticks, std::int64_t draw_ticks);
// Interp path (or override): main loop sets master exe/draw directly.
void set_master_exe_draw(std::int64_t exe_ticks, std::int64_t draw_ticks);
void add_present(std::int64_t present_ticks);
void add_bucket(Bucket b, std::int64_t ticks);
// T2: frameMs is loop-to-loop (prev end_frame → this). work_ticks = fallback first frame only.
void end_frame(std::int64_t work_ticks);

}  // namespace fps_probe
}  // namespace dusk

#define DUSK_FPS_SCOPE(bucket) \
    ::dusk::fps_probe::Scope dusk_fps_scope_##__LINE__(::dusk::fps_probe::Bucket::bucket)

#else

#define DUSK_FPS_SCOPE(bucket) ((void)0)

#endif
