#include "dusk/game_clock.h"
#include "dusk/fps_probe.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <dusk/frame_interpolation.h>
#include <dusk/logging.h>
#include <dusk/time.h>

namespace dusk::game_clock {

using clock = std::chrono::steady_clock;

bool s_initialized = false;
clock::time_point s_previous_sample{};
clock::time_point s_current_snapshot_time{};

std::unordered_map<uintptr_t, clock::time_point> s_interval_last_sample;

constexpr clock::duration kSimPeriodDuration =
    std::chrono::duration_cast<clock::duration>(std::chrono::duration<float>(sim_pace()));
constexpr clock::duration kAbnormalGapResetThreshold = std::chrono::milliseconds(250);
constexpr int kMaxSimTicksPerFrame = 2;

void ensure_initialized() {
    if (s_initialized) {
        return;
    }
    s_previous_sample = clock::now();
    s_current_snapshot_time = s_previous_sample;
    s_initialized = true;
}

void reset_frame_timer() {
    s_previous_sample = clock::now();
    s_current_snapshot_time = s_previous_sample - kSimPeriodDuration;
}

MainLoopPacer advance_main_loop() {
    ensure_initialized();

    const clock::time_point now = clock::now();
    const clock::duration frame_gap = now - s_previous_sample;
    const float presentation_dt = std::chrono::duration<float>(frame_gap).count();
    s_previous_sample = now;

    MainLoopPacer out{};
    out.presentation_dt_seconds = presentation_dt;

    const bool should_interpolate = dusk::getSettings().game.enableFrameInterpolation.getValue() !=
                                        dusk::FrameInterpMode::Off &&
                                    !dusk::getTransientSettings().skipFrameRateLimit;
    out.is_interpolating = should_interpolate;
    out.sim_pace = sim_pace();

    if (!should_interpolate) {
        s_current_snapshot_time = now;
        out.sim_ticks_to_run = 1;
        return out;
    }

    if (frame_gap > kAbnormalGapResetThreshold) {
        s_current_snapshot_time = now - kSimPeriodDuration;
        out.sim_ticks_to_run = 0;
        return out;
    }

    int sim_ticks_to_run = 0;
    clock::time_point projected_snapshot_time = s_current_snapshot_time;
    const clock::time_point render_time = now - kSimPeriodDuration;
    while (sim_ticks_to_run < kMaxSimTicksPerFrame && projected_snapshot_time < render_time) {
        projected_snapshot_time += kSimPeriodDuration;
        sim_ticks_to_run++;
    }
    out.sim_ticks_to_run = sim_ticks_to_run;
    return out;
}

void commit_sim_tick() {
    ensure_initialized();
    s_current_snapshot_time += kSimPeriodDuration;
}

float sample_interpolation_step() {
    ensure_initialized();
    const float step =
        std::chrono::duration<float>(clock::now() - s_current_snapshot_time).count() / sim_pace();
    return std::clamp(step, 0.0f, 1.0f);
}

float consume_interval(const void* consumer) {
    ensure_initialized();
    const uintptr_t key = reinterpret_cast<uintptr_t>(consumer);
    const clock::time_point now = clock::now();

    float dt = ui_initial_dt();
    const auto it = s_interval_last_sample.find(key);
    if (it != s_interval_last_sample.end()) {
        dt = std::chrono::duration<float>(now - it->second).count();
        dt = std::min(dt, ui_maximum_dt());
    }
    s_interval_last_sample[key] = now;
    return dt;
}

}  // namespace dusk::game_clock

#if TARGET_PC

namespace dusk {
namespace fps_probe {

namespace {

constexpr int kBucketCount = static_cast<int>(Bucket::Count);

const char* bucket_name(Bucket b) {
    switch (b) {
    case Bucket::Mount:
        return "mount";
    case Bucket::Doors:
        return "doors";
    case Bucket::DemoMsg:
        return "demoMsg";
    case Bucket::FrameInterp:
        return "frameInterp";
    case Bucket::Kankyo:
        return "kankyo";
    case Bucket::MenuRes:
        return "menuRes";
    case Bucket::Wind:
        return "wind";
    default:
        return "?";
    }
}

bool probe_enabled() {
    static int s_cached = -1;
    if (s_cached < 0) {
        const char* env = std::getenv("DUSK_FPS_PROBE");
        // Default ON; only "0" / "off" / "false" kills the log.
        s_cached = 1;
        if (env != nullptr &&
            (std::strcmp(env, "0") == 0 || std::strcmp(env, "off") == 0 ||
             std::strcmp(env, "false") == 0)) {
            s_cached = 0;
        }
    }
    return s_cached != 0;
}

std::int64_t s_qpc_freq = 0;

void ensure_qpc() {
    if (s_qpc_freq != 0) {
        return;
    }
#ifdef _WIN32
    LARGE_INTEGER freq{};
    QueryPerformanceFrequency(&freq);
    s_qpc_freq = freq.QuadPart > 0 ? freq.QuadPart : 1;
#else
    s_qpc_freq = 1'000'000'000LL;  // SDL ticks NS path unused; keep ms sane
#endif
}

struct FrameAccum {
    std::int64_t bucket[kBucketCount]{};
    std::int64_t exe = 0;
    std::int64_t draw = 0;
    std::int64_t present = 0;
    std::int64_t frame = 0;
    bool master_from_main = false;
};

struct SecondAccum {
    double frame_ms = 0;
    double exe_ms = 0;
    double draw_ms = 0;
    double present_ms = 0;
    double unaccounted_ms = 0;
    double bucket_ms[kBucketCount]{};
    int frames = 0;
    std::int64_t window_start_ticks = 0;
};

FrameAccum s_cur{};
SecondAccum s_sec{};
std::int64_t s_prev_loop_ticks = 0;  // Ferry T2: stamp at end of each end_frame

}  // namespace

std::int64_t now_ticks() {
    ensure_qpc();
#ifdef _WIN32
    LARGE_INTEGER t{};
    QueryPerformanceCounter(&t);
    return t.QuadPart;
#else
    return static_cast<std::int64_t>(SDL_GetTicksNS());
#endif
}

double ticks_to_ms(std::int64_t ticks) {
    ensure_qpc();
    return (1000.0 * static_cast<double>(ticks)) / static_cast<double>(s_qpc_freq);
}

Scope::Scope(Bucket b) : bucket(b), t0(0) {
    if (!probe_enabled()) {
        return;
    }
    t0 = now_ticks();
}

Scope::~Scope() {
    if (!probe_enabled() || t0 == 0) {
        return;
    }
    const int i = static_cast<int>(bucket);
    if (i >= 0 && i < kBucketCount) {
        s_cur.bucket[i] += now_ticks() - t0;
    }
}

void begin_frame() {
    if (!probe_enabled()) {
        return;
    }
    s_cur = FrameAccum{};
}

void note_fpcm_split(std::int64_t exe_ticks, std::int64_t draw_ticks) {
    if (!probe_enabled() || s_cur.master_from_main) {
        return;
    }
    s_cur.exe = exe_ticks;
    s_cur.draw = draw_ticks;
}

void set_master_exe_draw(std::int64_t exe_ticks, std::int64_t draw_ticks) {
    if (!probe_enabled()) {
        return;
    }
    s_cur.exe = exe_ticks;
    s_cur.draw = draw_ticks;
    s_cur.master_from_main = true;
}

void add_present(std::int64_t present_ticks) {
    if (!probe_enabled()) {
        return;
    }
    s_cur.present = present_ticks;
}

void add_bucket(Bucket b, std::int64_t ticks) {
    if (!probe_enabled() || ticks <= 0) {
        return;
    }
    const int i = static_cast<int>(b);
    if (i >= 0 && i < kBucketCount) {
        s_cur.bucket[i] += ticks;
    }
}

void end_frame(std::int64_t work_ticks) {
    if (!probe_enabled()) {
        return;
    }

    // Ferry T2: true frame = wall time since previous end_frame (includes
    // VIWait / events / limiter / discord — the ~7 ms the old bracket missed).
    const std::int64_t now = now_ticks();
    if (s_prev_loop_ticks != 0) {
        s_cur.frame = now - s_prev_loop_ticks;
    } else {
        s_cur.frame = work_ticks;  // first sample: fall back to work bracket
    }
    s_prev_loop_ticks = now;

    if (s_sec.window_start_ticks == 0) {
        s_sec.window_start_ticks = now;
    }

    const double frame_ms = ticks_to_ms(s_cur.frame);
    const double exe_ms = ticks_to_ms(s_cur.exe);
    const double draw_ms = ticks_to_ms(s_cur.draw);
    const double present_ms = ticks_to_ms(s_cur.present);
    s_sec.frame_ms += frame_ms;
    s_sec.exe_ms += exe_ms;
    s_sec.draw_ms += draw_ms;
    s_sec.present_ms += present_ms;
    s_sec.unaccounted_ms += frame_ms - (exe_ms + draw_ms + present_ms);
    for (int i = 0; i < kBucketCount; ++i) {
        s_sec.bucket_ms[i] += ticks_to_ms(s_cur.bucket[i]);
    }
    s_sec.frames++;

    const std::int64_t elapsed = now_ticks() - s_sec.window_start_ticks;
    if (s_sec.frames < 1 || ticks_to_ms(elapsed) < 1000.0) {
        return;
    }

    const double n = static_cast<double>(s_sec.frames);
    const double frameMs = s_sec.frame_ms / n;
    const double exe = s_sec.exe_ms / n;
    const double draw = s_sec.draw_ms / n;
    const double present = s_sec.present_ms / n;
    const double unaccounted = s_sec.unaccounted_ms / n;

    int top_i[3] = {-1, -1, -1};
    double top_v[3] = {-1.0, -1.0, -1.0};
    for (int i = 0; i < kBucketCount; ++i) {
        const double v = s_sec.bucket_ms[i] / n;
        for (int r = 0; r < 3; ++r) {
            if (v > top_v[r]) {
                for (int s = 2; s > r; --s) {
                    top_v[s] = top_v[s - 1];
                    top_i[s] = top_i[s - 1];
                }
                top_v[r] = v;
                top_i[r] = i;
                break;
            }
        }
    }

    char top3[160];
    int off = 0;
    for (int r = 0; r < 3; ++r) {
        if (top_i[r] < 0) {
            continue;
        }
        const int nwrite =
            std::snprintf(top3 + off, sizeof(top3) - static_cast<size_t>(off), "%s%s:%.3f",
                          off > 0 ? ", " : "",
                          bucket_name(static_cast<Bucket>(top_i[r])), top_v[r]);
        if (nwrite > 0) {
            off += nwrite;
        }
    }
    if (off <= 0) {
        std::snprintf(top3, sizeof(top3), "-");
    }

    DuskLog.info(
        "[FerryT] frameMs={:.2f} exe={:.2f} draw={:.2f} present={:.2f} unaccounted={:.2f} top3=({})",
        frameMs, exe, draw, present, unaccounted, top3);

    s_sec = SecondAccum{};
    s_sec.window_start_ticks = now_ticks();
}

}  // namespace fps_probe
}  // namespace dusk

#endif  // TARGET_PC

