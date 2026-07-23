/**
 * §59 / ASK 17 — offline event dump from engine Ja1Parser (no Bridge code).
 * Walks each open_track entry independently with per-track local ticks.
 */
#include "d/ext_seq/ja1_event_dump.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_track.h"
#include "dusk/logging.h"

namespace ExtSeq {
namespace Ja1EventDump {
namespace {

struct Ev {
    u32 tick = 0;
    u8 trackId = 0;
    u32 fileOff = 0;
    std::string event;
    std::string noteParam;
    std::string velocity;
};

struct VolRamp {
    u32 tick = 0;
    u8 trackId = 0;
    u8 target = 0;
    int moveTime = 0;
    std::string value;
};

bool s_active = false;
u8 s_trackId = 0;
u32 s_tick = 0;
u32 s_pendingFileOff = 0;
Ja1Track* s_track = nullptr;
bool s_explicitWait = false;
std::vector<Ev>* s_events = nullptr;
std::vector<std::pair<u8, u32>>* s_openQueue = nullptr;
std::vector<VolRamp>* s_volRamps = nullptr;

u32 fileOffOf(Ja1Track* track) {
    if (track == nullptr || track->getSeq()->getBase() == nullptr ||
        track->getSeq()->mCurrentFilePtr == nullptr) {
        return 0;
    }
    return static_cast<u32>(track->getSeq()->mCurrentFilePtr - track->getSeq()->getBase());
}

void writeVolRampCompanion(const char* csvPath, const std::vector<VolRamp>& ramps) {
    if (csvPath == nullptr || ramps.empty()) {
        return;
    }
    // seq_events_engine_foo.csv → seq_vol_ramps_foo.csv
    std::string path(csvPath);
    const char* needle = "seq_events_engine_";
    const auto pos = path.find(needle);
    if (pos == std::string::npos) {
        path += ".vol_ramps.csv";
    } else {
        path.replace(pos, std::strlen(needle), "seq_vol_ramps_");
    }
    FILE* f = std::fopen(path.c_str(), "wb");
    if (f == nullptr) {
        DuskLog.warn("[ExtSeq] §C.1 vol ramp companion: cannot write {}", path);
        return;
    }
    std::fputs("tick,track_id,target,value,move_time\n", f);
    for (const VolRamp& r : ramps) {
        std::fprintf(f, "%u,%u,%u,%s,%d\n", r.tick, static_cast<unsigned>(r.trackId),
                     static_cast<unsigned>(r.target), r.value.c_str(), r.moveTime);
    }
    std::fclose(f);

    // Early-bar histogram (tick < 480 ≈ first bars): (track, moveTime) → count.
    std::map<std::pair<u8, int>, u32> early;
    u32 earlyVol = 0;
    u32 snap = 0;
    u32 ramp = 0;
    for (const VolRamp& r : ramps) {
        if (r.target != 0) {
            continue;
        }
        if (r.moveTime <= 0) {
            snap++;
        } else {
            ramp++;
        }
        if (r.tick < 480u) {
            earlyVol++;
            early[{r.trackId, r.moveTime}]++;
        }
    }
    DuskLog.info(
        "[ExtSeq] §C.1 vol ramps: {} rows → {} (vol snap={} ramp={} early<480={})",
        ramps.size(), path, snap, ramp, earlyVol);
    u32 logged = 0;
    for (const auto& kv : early) {
        if (logged >= 12u) {
            break;
        }
        DuskLog.info("[ExtSeq] §C.1 early hist track={} moveTime={} count={}",
                     static_cast<unsigned>(kv.first.first), kv.first.second, kv.second);
        logged++;
    }
}

}  // namespace

bool envEnabled() {
    const char* v = std::getenv("DUSK_EXTSEQ_EVENT_DUMP");
    if (v == nullptr || v[0] == '\0' || (v[0] == '0' && v[1] == '\0')) {
        return false;
    }
    return true;
}

bool active() { return s_active; }

u8 currentTrackId() { return s_trackId; }

u32 currentTick() { return s_tick; }

void markOpcodeOff(const Ja1Track* track) {
    s_pendingFileOff = fileOffOf(const_cast<Ja1Track*>(track));
}

void markExplicitWait() { s_explicitWait = true; }

bool consumeExplicitWait() {
    const bool v = s_explicitWait;
    s_explicitWait = false;
    return v;
}

void queueOpen(u8 childId, u32 fileOff) {
    if (s_openQueue != nullptr) {
        s_openQueue->push_back({childId, fileOff});
    }
}

void emit(const char* event, const char* noteParam, const char* velocity) {
    if (!s_active || s_events == nullptr || event == nullptr) {
        return;
    }
    Ev e;
    e.tick = s_tick;
    e.trackId = s_trackId;
    e.fileOff = s_pendingFileOff;
    e.event = event;
    e.noteParam = noteParam != nullptr ? noteParam : "";
    e.velocity = velocity != nullptr ? velocity : "";
    s_events->push_back(std::move(e));
}

void noteVolRamp(u8 target, int moveTime, const char* valueStr) {
    if (!s_active || s_volRamps == nullptr) {
        return;
    }
    VolRamp r;
    r.tick = s_tick;
    r.trackId = s_trackId;
    r.target = target;
    r.moveTime = moveTime;
    r.value = valueStr != nullptr ? valueStr : "";
    s_volRamps->push_back(std::move(r));
}

u32 dumpBmsToCsv(const u8* data, u32 size, const char* outPath) {
    if (data == nullptr || size == 0 || outPath == nullptr) {
        return 0;
    }

    std::vector<Ev> events;
    std::vector<VolRamp> volRamps;
    std::vector<std::pair<u8, u32>> openQueue;
    openQueue.push_back({0, 0});
    std::set<u32> seenStarts;

    Ja1Parser parser;
    u32 qi = 0;
    while (qi < openQueue.size()) {
        const u8 tid = openQueue[qi].first;
        const u32 start = openQueue[qi].second;
        qi++;
        if (seenStarts.count(start) != 0) {
            continue;
        }
        seenStarts.insert(start);
        if (start >= size) {
            continue;
        }

        Ja1Track track;
        track.init();
        track.start(const_cast<u8*>(data), start);

        s_events = &events;
        s_volRamps = &volRamps;
        s_openQueue = &openQueue;
        s_trackId = tid;
        s_tick = 0;
        s_track = &track;
        s_active = true;
        s_explicitWait = false;

        u32 steps = 0;
        while (track.mActive && steps < 500000u) {
            steps++;
            const int r = parser.parseSeq(&track);
            if (!track.mActive || r < 0) {
                break;
            }
            const s32 w = track.getSeq()->getWait();
            (void)consumeExplicitWait();  // wait row already emitted in cmdWait if any
            if (w > 0) {
                // Explicit waits emit their own row; note_on gate waits advance tick only
                // (matches Bridge seq-events: note_on duration does not emit "wait").
                s_tick += static_cast<u32>(w);
                track.getSeq()->wait(0);
            } else if (w < 0) {
                // WW noteid!=0 path: wait(-1) then resume next mainProc — no tick advance.
                track.getSeq()->wait(0);
            } else {
                // Idle with no wait — finished or stuck.
                break;
            }
        }

        s_active = false;
        s_track = nullptr;
        track.close();
    }

    s_events = nullptr;
    s_volRamps = nullptr;
    s_openQueue = nullptr;

    std::sort(events.begin(), events.end(), [](const Ev& a, const Ev& b) {
        if (a.trackId != b.trackId) {
            return a.trackId < b.trackId;
        }
        if (a.tick != b.tick) {
            return a.tick < b.tick;
        }
        return a.fileOff < b.fileOff;
    });

    FILE* f = std::fopen(outPath, "wb");
    if (f == nullptr) {
        DuskLog.warn("[ExtSeq] §59 event dump: cannot write {}", outPath);
        return 0;
    }
    std::fputs("tick,track_id,event,note_param,velocity\n", f);
    for (const Ev& e : events) {
        std::fprintf(f, "%u,%u,%s,%s,%s\n", e.tick, static_cast<unsigned>(e.trackId),
                     e.event.c_str(), e.noteParam.c_str(), e.velocity.c_str());
    }
    std::fclose(f);

    writeVolRampCompanion(outPath, volRamps);

    DuskLog.info("[ExtSeq] §59 event dump: {} events → {}", events.size(), outPath);
    return static_cast<u32>(events.size());
}

}  // namespace Ja1EventDump
}  // namespace ExtSeq
