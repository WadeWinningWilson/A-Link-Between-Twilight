/**
 * d_ext_npc_doors.cpp — №32/№51/№53 door warps (folder-side).
 *
 * №53: TP-native OPEN (DOOR attention + CANDOOR → orderDoorEvent) wraps the
 * pinned-BG warp backend. Spawn/stamp/register in doors.ini order with boot
 * log; exit knobs + leave-shell AABB G-guard; keys healed by position.
 */
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
#include "d/d_ext_npc_doors.h"

#if TARGET_PC

#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

#include "d/actor/d_a_knob00.h"
#include "d/actor/d_a_alink.h"  // §161 flag-only ClrWallNone on arrival demo end
#include "d/d_camera.h"
#include "d/d_demo.h"  // №167: a running demo owns the camera
#include "d/d_com_inf_game.h"
#include "d/d_event.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_save_guard.h"
#include "d/ext_plugin/ww_import_gate.h"
#include "d/d_ext_dmesg.h"  // §313 dExtDmesg_isBoxActive (don't G-guard a held tale box)
#include "f_pc/f_pc_name.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_graphic.h"
#include "SSystem/SComponent/c_math.h"

namespace fs = std::filesystem;

namespace {

struct DoorDef {
    char name[32];
    cXyz wwPos;
    f32 radius;
    char enterProc[32];
    f32 exitRadius;
    s16 wwRy;
    bool hasWwRy;
    bool wantKnob;
    bool walkthrough;  // №56: radius cross warps both ways (no A)
    bool roomLane;     // №62: ride TP room streaming (else pinned BG warp)
    int hostRoom;      // №62: claimed host-stage room slot when roomLane
    bool wasInEnter;
    bool wasInExit;
    cXyz knobHost;
    bool hasKnobHost;
    int doorIndex;  // 1-based index among wantKnob rows (params high byte)
    bool bootLogged;
    bool exitKnobSpawned;
    cXyz exitKnobHost;
    bool hasExitKnobHost;
};

struct PendingDoorWarp {
    bool active;
    bool isEnter;
    char proc[32];
    char doorName[32];
    cXyz failSafe;
    cXyz exitSpawn;
    s16 exitFacing;
    bool hasExitFacing;
    int framesLeft;
};

std::vector<DoorDef> s_doors;
bool s_loaded = false;
bool s_knobsSpawned = false;
u32 s_knobsSpawnedGen = 0;  // №94: must match dExtNpcWorld_generation()
bool s_reconcileDone = false;
int s_cooldown = 0;
char s_loadedMod[128] = {};
PendingDoorWarp s_pending = {};
char s_exitSpawnForProc[32] = {};

// №89/№90: destination-owned arrival demo + mandatory event G-guard.
struct ArrivalDemo {
    bool armed;
    bool demoStarted;
    bool demoEnded;
    bool withDemo;        // false = №90 guard-only (warp / no door demo)
    bool cameraSnapped;   // №110: QuickStart once after Link exists on dest
    int demoFramesLeft;   // DEMO_DOOR_OPEN hold on dest (same context)
    int guardFramesLeft;  // after demo end; force-end if event still active
    char stage[12];
    s16 facing;
    bool hasFacing;
    bool isExit;  // porch arrival vs interior
};
ArrivalDemo s_arrival = {};
constexpr int kArrivalDemoFrames = 18;
constexpr int kArrivalGuardFrames = 120;

void armArrivalDemo(const char* stage, s16 facing, bool hasFacing, bool isExit) {
    if (stage == NULL || stage[0] == '\0') {
        s_arrival = {};
        return;
    }
    s_arrival = {};
    s_arrival.armed = true;
    s_arrival.withDemo = true;
    s_arrival.demoFramesLeft = -1;
    s_arrival.guardFramesLeft = -1;
    std::snprintf(s_arrival.stage, sizeof(s_arrival.stage), "%s", stage);
    s_arrival.facing = facing;
    s_arrival.hasFacing = hasFacing;
    s_arrival.isExit = isExit;
    DuskLog.info("[Doors] №89 arm arrival demo → '{}' exit={} facing={}", s_arrival.stage,
                 isExit ? 1 : 0, hasFacing ? (int)facing : -1);
}

void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' ||
                          s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
        ++i;
    }
    if (i > 0) {
        s.erase(0, i);
    }
}

bool parseVec(const std::string& v, cXyz* out) {
    return out != NULL && sscanf(v.c_str(), "%f,%f,%f", &out->x, &out->y, &out->z) == 3;
}

void ensureLoaded() {
    dExtNpcManifest island{};
    if (!dExtNpcMount_lookup("EXT_BG0", &island) || island.modFolder[0] == '\0') {
        return;
    }
    if (s_loaded && std::strcmp(s_loadedMod, island.modFolder) == 0) {
        return;
    }
    s_doors.clear();
    s_loaded = false;
    s_knobsSpawned = false;
    s_reconcileDone = false;
    s_exitSpawnForProc[0] = '\0';
    std::snprintf(s_loadedMod, sizeof(s_loadedMod), "%s", island.modFolder);

    const fs::path path = dusk::ConfigPath / "model_replacements" / island.modFolder /
                          "population" / "doors.ini";
    std::ifstream in(path);
    if (!in) {
        DuskLog.debug("[ExtNpcDoors] no doors.ini in {}", island.modFolder);
        s_loaded = true;
        return;
    }

    DoorDef* cur = NULL;
    int nextKnobIndex = 0;
    std::string line;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            const size_t end = line.find(']');
            if (end == std::string::npos) {
                continue;
            }
            DoorDef d{};
            std::snprintf(d.name, sizeof(d.name), "%s", line.substr(1, end - 1).c_str());
            d.radius = 220.0f;
            d.exitRadius = 200.0f;
            d.wwRy = 0;
            d.hasWwRy = false;
            d.wantKnob = false;
            d.walkthrough = false;
            d.roomLane = false;
            d.hostRoom = -1;
            d.wasInEnter = false;
            d.wasInExit = false;
            d.hasKnobHost = false;
            d.doorIndex = 0;
            d.bootLogged = false;
            d.exitKnobSpawned = false;
            d.hasExitKnobHost = false;
            s_doors.push_back(d);
            cur = &s_doors.back();
            continue;
        }
        if (cur == NULL) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key);
        trim(val);
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        if (key == "ww_pos" || key == "pos") {
            parseVec(val, &cur->wwPos);
        } else if (key == "radius") {
            cur->radius = static_cast<f32>(atof(val.c_str()));
        } else if (key == "exit_radius") {
            cur->exitRadius = static_cast<f32>(atof(val.c_str()));
        } else if (key == "enter_proc" || key == "proc") {
            std::snprintf(cur->enterProc, sizeof(cur->enterProc), "%s", val.c_str());
        } else if (key == "ww_ry" || key == "ry") {
            cur->wwRy = static_cast<s16>(atoi(val.c_str()));
            cur->hasWwRy = true;
        } else if (key == "knob") {
            cur->wantKnob = !(val == "0" || val == "false" || val == "no");
            if (cur->wantKnob && cur->doorIndex == 0) {
                cur->doorIndex = ++nextKnobIndex;
            }
        } else if (key == "walkthrough") {
            cur->walkthrough = !(val == "0" || val == "false" || val == "no");
        } else if (key == "lane") {
            // pinned (default) | room
            cur->roomLane = (val == "room");
        } else if (key == "host_room") {
            cur->hostRoom = atoi(val.c_str());
        }
    }
    // №62: register room-lane bindings after the full parse.
    for (DoorDef& d : s_doors) {
        if (d.roomLane && d.hostRoom >= 0 && d.enterProc[0] != '\0') {
            dExtNpcMount_registerRoomLane(d.enterProc, d.hostRoom);
        }
    }
    s_loaded = true;
    DuskLog.info("[ExtNpcDoors] loaded {} door triggers from {}", (int)s_doors.size(),
                 path.string());

    // §559: 19c gate controls, once per process. Demonstrating C1/C2/C4/C6 on
    // a real boot is the difference between a gate that is specified and one
    // that is shown to behave -- every instrument this campaign trusted
    // without a negative control turned out to be wrong at least once.
    dExtWwImportGate_selftest();

    // ========================================================================
    // §547 SCLS CROSS-CHECK (WWB:B2's payoff). Consumes population/
    // door_bindings.ini, which `ww_bridge.py scls --emit` generates from the
    // donor's own SCLS exit table (stage_scls_info_class, 0x0C) in each arc's
    // room.dzr.
    //
    // THIS VALIDATES; IT DOES NOT ROUTE. The donor's SCLS names WW-side
    // destinations ("sea", room 44); this port routes to HOST stages
    // (dest.hostStage / hostRoom) chosen by our own config. Those are different
    // coordinate systems, so SCLS cannot drive routing directly and pretending
    // otherwise would be the overreach. What it CAN do is catch the transcription
    // class B2 was written to remove: a door we implement that the donor has no
    // exit for, or a donor exit we never implemented.
    //
    // Additive and log-only by construction -- no DoorDef is modified and no
    // control flow depends on it. If the file is absent (a user who never ran
    // `scls --emit`) this is silent: an ABSENT cross-check must not look like a
    // FAILED one.
    // ========================================================================
    {
        const fs::path bpath = dusk::ConfigPath / "model_replacements" /
                               island.modFolder / "population" / "door_bindings.ini";
        std::ifstream bin(bpath);
        if (!bin) {
            DuskLog.debug("[ExtNpcDoors] no door_bindings.ini — SCLS cross-check "
                          "SKIPPED (not failed); run ww_bridge.py scls --emit");
        } else {
            // NAME NORMALISATION, and it is not cosmetic. doors.ini calls one
            // door `amori`; the arc it comes from is `A_mori`, so the generated
            // section is `[a_mori]`. A raw compare reports "no donor exit" for a
            // door that plainly has two -- and a validator whose FIRST output is a
            // false alarm teaches everyone to ignore it. Caught by predicting the
            // result offline before shipping the check. Fold case, drop
            // non-alphanumerics, compare the residue.
            auto norm = [](const std::string& s) {
                std::string o;
                for (char c : s) {
                    if (std::isalnum((unsigned char)c)) {
                        o += (char)std::tolower((unsigned char)c);
                    }
                }
                return o;
            };
            std::vector<std::string> donorSections;
            std::string bline;
            while (std::getline(bin, bline)) {
                trim(bline);
                if (bline.size() >= 2 && bline[0] == '[' && bline.back() == ']') {
                    donorSections.push_back(norm(bline.substr(1, bline.size() - 2)));
                }
            }
            int matched = 0;
            std::string ours;
            for (DoorDef& d : s_doors) {
                std::string n = norm(std::string(d.name));
                bool hit = false;
                for (const std::string& s : donorSections) {
                    if (s == n) { hit = true; break; }
                }
                if (hit) {
                    matched++;
                } else {
                    ours += (ours.empty() ? "" : ", ");
                    ours += d.name;
                }
            }
            DuskLog.info("[ExtNpcDoors] §547 SCLS cross-check: {} of {} door rows "
                         "have a donor exit binding ({} donor sections)",
                         matched, (int)s_doors.size(), (int)donorSections.size());
            if (!ours.empty()) {
                DuskLog.warn("[ExtNpcDoors] §547 door rows with NO donor SCLS exit: {}"
                             " — either hand-authored beyond the donor, or the arc "
                             "was not scanned", ours.c_str());
            }
        }
    }
}

cXyz wwToHost(const dExtNpcManifest& island, const cXyz& ww) {
    cXyz out;
    out.x = island.hostPos.x + (ww.x - island.anchor.x);
    out.y = island.hostPos.y + (ww.y - island.anchor.y);
    out.z = island.hostPos.z + (ww.z - island.anchor.z);
    return out;
}

// №54-4: door + ~150u outward along ww_ry; y = door_y (WaitBg local-probes, never sky).
cXyz outdoorPorch(const dExtNpcManifest& island, const DoorDef& d) {
    cXyz door = wwToHost(island, d.wwPos);
    cXyz spawn = door;
    if (d.hasWwRy) {
        const f32 yaw = (f32)d.wwRy * (3.14159265f / 32768.0f);
        spawn.x += std::sin(yaw) * 150.0f;
        spawn.z += std::cos(yaw) * 150.0f;
    }
    // Prefer a local snap now if island BG is already up; else authored door_y.
    dExtNpcMount_localGroundSnap(&spawn, door.y);
    return spawn;
}

// №56/№57: Nintendo return_pos (WW world) + local snap; else porch fallback.
cXyz outdoorReturn(const dExtNpcManifest& island, const DoorDef& d,
                   const dExtNpcManifest& interior) {
    if (interior.hasReturnPos) {
        cXyz spawn = wwToHost(island, interior.returnPos);
        dExtNpcMount_localGroundSnap(&spawn, spawn.y);
        return spawn;
    }
    return outdoorPorch(island, d);
}

s16 outdoorReturnFacing(const DoorDef& d, const dExtNpcManifest& interior) {
    if (interior.hasReturnRy) {
        return interior.returnRy;
    }
    return d.hasWwRy ? d.wwRy : (s16)0;
}

bool nearXZ(const cXyz& a, const cXyz& b, f32 radius) {
    const f32 dx = a.x - b.x;
    const f32 dz = a.z - b.z;
    const f32 dy = a.y - b.y;
    if (dy > 400.0f || dy < -200.0f) {
        return false;
    }
    return (dx * dx + dz * dz) <= (radius * radius);
}

bool aPressedNearDoor() {
    return mDoCPd_c::getTrigA(PAD_1) != 0;
}

void beginDoorFade() {
    mDoGph_gInf_c::startFadeOut(15);
}

// №85: foreign host = native ChangeReq wipe owns the screen. Custom mDoGph fade +
// DEMO_DOOR_OPEN leave OVERLAP stuck (log 205514: OVERLAP created, never finishes).
bool isCrossStageHost(const dExtNpcManifest& dest) {
    if (dest.hostStage[0] == '\0') {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    return stage == NULL || std::strcmp(stage, dest.hostStage) != 0;
}

void armNativeStageChange(const char* stage, s8 room, s8 layer, const char* tag) {
    dExtNpcMount_endDoorDemoLock();
    // §355b: named fade-clear (suspect for the §354 missing tale fade)
    DuskLog.info("[Doors] §355b armNativeStageChange offFade()");
    mDoGph_gInf_c::offFade();
    if (dComIfGp_isEnableNextStage()) {
        DuskLog.warn("[Doors] {} — next stage already armed; skip re-set", tag != NULL ? tag : "?");
        return;
    }
    DuskLog.info("[Doors] §347c armNativeStageChange ARM '{}' — evRun={}", stage,
                 dComIfGp_event_runCheck() ? 1 : 0);
    dComIfGp_setNextStage(stage, /*point*/ 0, room, layer);
}

DoorDef* findDoorByName(const char* name) {
    if (name == NULL || name[0] == '\0') {
        return NULL;
    }
    // Accept "exit:name" → name
    const char* key = name;
    if (std::strncmp(name, "exit:", 5) == 0) {
        key = name + 5;
    }
    for (DoorDef& d : s_doors) {
        if (std::strcmp(d.name, key) == 0) {
            return &d;
        }
    }
    return NULL;
}

DoorDef* findDoorByKnobHost(const cXyz& knobPos) {
    DoorDef* best = NULL;
    f32 bestD2 = 80.0f * 80.0f;
    for (DoorDef& d : s_doors) {
        if (d.enterProc[0] == '\0' || !d.hasKnobHost) {
            continue;
        }
        const f32 dx = knobPos.x - d.knobHost.x;
        const f32 dy = knobPos.y - d.knobHost.y;
        const f32 dz = knobPos.z - d.knobHost.z;
        if (dy > 80.0f || dy < -80.0f) {
            continue;
        }
        const f32 d2 = dx * dx + dz * dz;
        if (d2 < bestD2) {
            best = &d;
            bestD2 = d2;
        }
    }
    return best;
}

DoorDef* findDoorByEnterProc(const char* proc) {
    if (proc == NULL || proc[0] == '\0') {
        return NULL;
    }
    for (DoorDef& d : s_doors) {
        if (std::strcmp(d.enterProc, proc) == 0) {
            return &d;
        }
    }
    return NULL;
}

void armDoorAttention(dExtNpcMount_c* knob) {
    if (knob == NULL) {
        return;
    }
    knob->attention_info.flags = fopAc_AttnFlag_DOOR_e;
    auto distIdx = [](int dist, int angle) -> u8 { return (u8)(dist + angle * 0x14 + 0x5e); };
    knob->attention_info.distances[fopAc_attn_DOOR_e] = distIdx(3, 6);
    knob->attention_info.position = knob->current.pos;
    knob->attention_info.position.y += 80.0f;
}

void stampKnob(dExtNpcMount_c* knob, DoorDef& d, bool isExit) {
    if (knob == NULL) {
        return;
    }
    if (isExit) {
        std::snprintf(knob->mDoorKey, sizeof(knob->mDoorKey), "exit:%s", d.name);
        std::snprintf(knob->mSpawnSrc, sizeof(knob->mSpawnSrc), "door:exit:%s", d.name);
    } else {
        std::snprintf(knob->mDoorKey, sizeof(knob->mDoorKey), "%s", d.name);
        std::snprintf(knob->mSpawnSrc, sizeof(knob->mSpawnSrc), "door:%s", d.name);
    }
    armDoorAttention(knob);
    knob->mManifest.doorAttention = true;
}

dExtNpcMount_c* findKnobNear(const cXyz& pos, f32 maxDist) {
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        dExtNpcMount_c* best;
    } ctx{&pos, maxDist * maxDist, NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || m->mpMorf == NULL) {
                return 0;
            }
            // Prefer doorAttention; also accept knobs still loading attention.
            if (!m->mManifest.doorAttention && std::strcmp(m->mManifest.proc, "NPC_KNOB") != 0) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dy = ac->current.pos.y - c->from->y;
            const f32 dz = ac->current.pos.z - c->from->z;
            if (dy > 120.0f || dy < -120.0f) {
                return 0;
            }
            const f32 d2 = dx * dx + dz * dz;
            if (d2 < c->bestD2) {
                c->bestD2 = d2;
                c->best = m;
            }
            return 0;
        },
        &ctx);
    return ctx.best;
}

daKnob00_c* findKnob00Near(const cXyz& pos, f32 maxDist) {
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        daKnob00_c* best;
    } ctx{&pos, maxDist * maxDist, NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL || fopAcM_GetName(ac) != fpcNm_KNOB00_e) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dy = ac->current.pos.y - c->from->y;
            const f32 dz = ac->current.pos.z - c->from->z;
            if (dy > 120.0f || dy < -120.0f) {
                return 0;
            }
            const f32 d2 = dx * dx + dz * dz;
            if (d2 < c->bestD2) {
                c->bestD2 = d2;
                c->best = static_cast<daKnob00_c*>(ac);
            }
            return 0;
        },
        &ctx);
    return ctx.best;
}

void stampKnob00(daKnob00_c* knob, DoorDef& d, bool isExit) {
    if (knob == NULL) {
        return;
    }
    if (isExit) {
        char ek[32];
        std::snprintf(ek, sizeof(ek), "exit:%s", d.name);
        knob->setDoorKey(ek);
    } else {
        knob->setDoorKey(d.name);
    }
}

// №53-D: 1:1 stamp by doors.ini order + boot log. Heals FIFO scramble.
void reconcileOutdoorKnobs() {
    if (s_reconcileDone) {
        return;
    }
    int want = 0;
    int ok = 0;
    for (DoorDef& d : s_doors) {
        if (!d.wantKnob || !d.hasKnobHost) {
            continue;
        }
        ++want;
        // §27: prefer real KNOB00; fall back to ExtNpc mount knobs.
        if (daKnob00_c* k00 = findKnob00Near(d.knobHost, 60.0f)) {
            stampKnob00(k00, d, false);
            ++ok;
            if (!d.bootLogged) {
                DuskLog.info("[Doors] prop key={} pos=({:.1f},{:.1f},{:.1f}) idx={} (§27 KNOB00)",
                             d.name, k00->current.pos.x, k00->current.pos.y, k00->current.pos.z,
                             d.doorIndex);
                d.bootLogged = true;
            }
            continue;
        }
        dExtNpcMount_c* knob = findKnobNear(d.knobHost, 60.0f);
        if (knob == NULL) {
            continue;
        }
        stampKnob(knob, d, false);
        ++ok;
        if (!d.bootLogged) {
            DuskLog.info("[Doors] prop key={} pos=({:.1f},{:.1f},{:.1f}) idx={}", d.name,
                         knob->current.pos.x, knob->current.pos.y, knob->current.pos.z,
                         d.doorIndex);
            d.bootLogged = true;
        }
    }
    if (want > 0 && ok == want) {
        s_reconcileDone = true;
        DuskLog.info("[Doors] №53 reconcile 1:1 ok — {}/{} outdoor knobs stamped", ok, want);
    } else if (want > 0 && ok > 0) {
        DuskLog.debug("[Doors] №53 reconcile partial {}/{}", ok, want);
    }
}

void logDoorRequest(const DoorDef& d, const dExtNpcManifest& dest, f32 distXZ,
                    const char* propKey) {
    DuskLog.info(
        "[Doors] req={} resolved={} anchor=({:.1f},{:.1f},{:.1f}) spawn=({:.1f},{:.1f},{:.1f}) "
        "door='{}' dist={:.1f} prop='{}'",
        d.enterProc, dest.arc[0] ? dest.arc : "(none)", dest.anchor.x, dest.anchor.y, dest.anchor.z,
        dest.spawnRel.x, dest.spawnRel.y, dest.spawnRel.z, d.name, distXZ,
        propKey != NULL && propKey[0] ? propKey : "?");
}

bool commitEnter(const char* proc, const cXyz& failSafe, const char* doorName) {
    if (proc == NULL || !dExtNpcMount_hasPayload(proc)) {
        DuskLog.warn("[Doors] enter {} via door {} — payload missing", proc ? proc : "(null)",
                     doorName ? doorName : "?");
        return false;
    }
    dExtNpcManifest dest{};
    if (!dExtNpcMount_lookup(proc, &dest) || !dest.isBg) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    const bool crossStage =
        dest.hostStage[0] != '\0' && (stage == NULL || std::strcmp(stage, dest.hostStage) != 0);

    DoorDef* door = findDoorByName(doorName);
    const bool wantRoom =
        (door != NULL && door->roomLane && door->hostRoom >= 0) || dExtNpcMount_isRoomLaneProc(proc);
    const int hostRoom =
        door != NULL && door->hostRoom >= 0 ? door->hostRoom : dExtNpcMount_roomLaneHostRoom(proc);

    if (crossStage) {
        // №84/№85: foreign host = NATIVE STAGE CHANGE only. Play scene ChangeReq
        // creates Link/camera/rooms. No ensureRoomLaneLoaded / BgWarp here. No custom
        // fade — native wipe must complete (demo lock + mDoGph fade stuck OVERLAP).
        if (wantRoom && hostRoom >= 0) {
            dExtNpcMount_registerRoomLane(proc, hostRoom);
        }
        dExtNpcMount_cancelTransports();
        armNativeStageChange(dest.hostStage, static_cast<s8>(dest.hostRoom),
                             static_cast<s8>(dest.hostLayer), "enter-cross");
        // №89: destination owns arrival demo (spawn_ry) + event G-guard.
        armArrivalDemo(dest.hostStage, dest.hasSpawnRy ? dest.spawnRy : (s16)0, dest.hasSpawnRy,
                       /*isExit=*/false);
        DuskLog.info(
            "[Doors] enter {} → {} (transport=stage host='{}' room={} point=0 via door '{}' — "
            "native setNextStage only)",
            proc, proc, dest.hostStage, dest.hostRoom, doorName ? doorName : "?");
        (void)failSafe;
        return true;
    }

    beginDoorFade();

    // Same-stage: №62 dual-lane (room lane claims host room then waits for mount).
    if (wantRoom && hostRoom >= 0) {
        // №65: MEMORY = loadRoom; TRANSPORT = room-lane place (no requestBgWarp*).
        dExtNpcMount_registerRoomLane(proc, hostRoom);
        const bool loaded = dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
        if (!dExtNpcMount_requestRoomLaneEnter(proc, failSafe)) {
            return false;
        }
        DuskLog.info(
            "[Doors] enter {} → {} (transport=room host_room={} load={} via door '{}')", proc,
            proc, hostRoom, loaded ? 1 : 0, doorName ? doorName : "?");
        return true;
    }

    if (!dExtNpcMount_requestBgWarpGuarded(proc, failSafe)) {
        return false;
    }
    DuskLog.info("[Doors] enter {} → {} (transport=pinned via door '{}')", proc, proc,
                 doorName ? doorName : "?");
    return true;
}

bool queueEnter(const DoorDef& d, const dExtNpcManifest& island, fopAc_ac_c* player, f32 distXZ,
                const char* propKey, bool openAlreadyDone) {
    if (d.enterProc[0] == '\0' || !dExtNpcMount_hasPayload(d.enterProc)) {
        DuskLog.warn("[Doors] enter {} via door {} — payload missing", d.enterProc, d.name);
        return false;
    }
    dExtNpcManifest dest{};
    if (!dExtNpcMount_lookup(d.enterProc, &dest) || !dest.isBg) {
        return false;
    }
    logDoorRequest(d, dest, distXZ, propKey);

    // №54-4: fail-safe = porch (door + outward step), not sky-probed rooftop.
    const cXyz failSafe = outdoorPorch(island, d);

    // №85: cross-stage — commit immediately, no demo lock / custom fade (native wipe).
    if (isCrossStageHost(dest)) {
        if (dComIfGp_isEnableNextStage()) {
            DuskLog.warn("[Doors] enter '{}' — next stage already armed; debounce", d.name);
            return false;
        }
        const bool ok = commitEnter(d.enterProc, failSafe, d.name);
        if (ok) {
            s_cooldown = 90;
        }
        (void)island;
        (void)player;
        (void)openAlreadyDone;
        return ok;
    }

    // №91: door already owned DEFAULT_KNOB_* + cutEnd — skip ad-hoc open ceremony.
    if (!openAlreadyDone) {
        if (player != NULL) {
            // №55: open anim comes from the knob manifest's door_open_bck — no name in code.
            const char* bck = NULL;
            dExtNpcMount_c* nearest =
                dExtNpcMount_nearestDoorAttention(player->current.pos, 400.0f);
            if (nearest != NULL && nearest->mManifest.doorOpenBck[0]) {
                bck = nearest->mManifest.doorOpenBck;
            }
            dExtNpcMount_playAnimNearest(player->current.pos, 400.0f, bck);
        }
        // №53-A / №58-B / №66-A: control lock + screen cover mid open-anim (№60 beats 2+4).
        dExtNpcMount_beginDoorDemoLock();
        beginDoorFade();
    }

    s_pending = {};
    s_pending.active = true;
    s_pending.isEnter = true;
    std::snprintf(s_pending.proc, sizeof(s_pending.proc), "%s", d.enterProc);
    std::snprintf(s_pending.doorName, sizeof(s_pending.doorName), "%s", d.name);
    s_pending.failSafe = failSafe;
    s_pending.framesLeft = openAlreadyDone ? 1 : 28;
    (void)island;
    return true;
}

bool queueExit(const DoorDef& d, const dExtNpcManifest& island, fopAc_ac_c* player,
               bool openAlreadyDone) {
    dExtNpcManifest dest{};
    if (!dExtNpcMount_lookup(d.enterProc, &dest) || !dest.isBg) {
        return false;
    }
    dExtNpcManifest exterior{};
    const bool haveExterior = dExtNpcMount_lookup("EXT_BG0", &exterior) && exterior.isBg;
    const char* curStage = dComIfGp_getStartStageName();
    const bool onWwHost = curStage != NULL && dExtWwSave_isWwHostStage(curStage);
    const bool exitCrossStage =
        onWwHost && haveExterior && exterior.hostStage[0] != '\0' &&
        (curStage == NULL || std::strcmp(curStage, exterior.hostStage) != 0);

    // №56: Nintendo return_pos / return_ry (Sturgeon → upper door per user canon).
    const cXyz spawn = outdoorReturn(island, d, dest);
    const s16 facing = outdoorReturnFacing(d, dest);

    // №85: cross-stage exit — arm native wipe immediately (no demo lock / custom fade).
    if (exitCrossStage) {
        if (dComIfGp_isEnableNextStage()) {
            DuskLog.warn("[Doors] exit '{}' — next stage already armed; debounce", d.name);
            return false;
        }
        dExtNpcMount_cancelTransports();
        dExtNpcMount_armStageExitRemount("EXT_BG0", spawn, facing,
                                         dest.hasReturnRy || d.hasWwRy);
        armNativeStageChange(exterior.hostStage, static_cast<s8>(exterior.hostRoom),
                             static_cast<s8>(exterior.hostLayer), "exit-cross");
        // №89: porch owns its own arrival demo (return_ry) + event G-guard.
        armArrivalDemo(exterior.hostStage, facing, dest.hasReturnRy || d.hasWwRy,
                       /*isExit=*/true);
        DuskLog.info(
            "[Doors] exit '{}' via door '{}' transport=stage → '{}' (remount EXT_BG0 after "
            "play scene)",
            d.enterProc, d.name, exterior.hostStage);
        s_cooldown = 90;
        (void)player;
        (void)openAlreadyDone;
        return true;
    }

    if (!openAlreadyDone) {
        if (player != NULL) {
            const char* bck = NULL;
            dExtNpcMount_c* nearest =
                dExtNpcMount_nearestDoorAttention(player->current.pos, 400.0f);
            if (nearest != NULL && nearest->mManifest.doorOpenBck[0]) {
                bck = nearest->mManifest.doorOpenBck;
            }
            dExtNpcMount_playAnimNearest(player->current.pos, 400.0f, bck);
        }
        dExtNpcMount_beginDoorDemoLock();
        beginDoorFade();
    }

    s_pending = {};
    s_pending.active = true;
    s_pending.isEnter = false;
    std::snprintf(s_pending.proc, sizeof(s_pending.proc), "%s", d.enterProc);
    std::snprintf(s_pending.doorName, sizeof(s_pending.doorName), "%s", d.name);
    s_pending.exitSpawn = spawn;
    s_pending.exitFacing = facing;
    s_pending.hasExitFacing = dest.hasReturnRy || d.hasWwRy;
    s_pending.framesLeft = openAlreadyDone ? 1 : 28;
    return true;
}

bool tickPending(const dExtNpcManifest& island) {
    if (!s_pending.active) {
        return false;
    }
    if (--s_pending.framesLeft > 0) {
        return true;
    }
    const PendingDoorWarp pending = s_pending;
    s_pending = {};
    if (pending.isEnter) {
        commitEnter(pending.proc, pending.failSafe, pending.doorName);
    } else {
        dExtNpcManifest interior{};
        dExtNpcManifest exterior{};
        const bool haveInterior = dExtNpcMount_lookup(pending.proc, &interior) && interior.isBg;
        const bool haveExterior = dExtNpcMount_lookup("EXT_BG0", &exterior) && exterior.isBg;
        const char* curStage = dComIfGp_getStartStageName();
        const bool onWwHost =
            curStage != NULL && dExtWwSave_isWwHostStage(curStage);
        const bool exitCrossStage =
            onWwHost && haveExterior && exterior.hostStage[0] != '\0' &&
            (curStage == NULL || std::strcmp(curStage, exterior.hostStage) != 0);

        if (exitCrossStage) {
            // №84/№85: native stage change back — remount EXT_BG0 after play scene.
            // No custom fade here (queueExit same-stage path already faded; cross-stage
            // exits now arm immediately in queueExit and should not hit this branch).
            dExtNpcMount_cancelTransports();
            dExtNpcMount_armStageExitRemount("EXT_BG0", pending.exitSpawn, pending.exitFacing,
                                             pending.hasExitFacing);
            armNativeStageChange(exterior.hostStage, static_cast<s8>(exterior.hostRoom),
                                 static_cast<s8>(exterior.hostLayer), "exit-cross-pending");
            armArrivalDemo(exterior.hostStage, pending.exitFacing, pending.hasExitFacing,
                           /*isExit=*/true);
            DuskLog.info(
                "[Doors] exit '{}' via door '{}' transport=stage → '{}' (remount EXT_BG0 after "
                "play scene)",
                pending.proc, pending.doorName, exterior.hostStage);
            (void)haveInterior;
        } else {
            beginDoorFade();
            const int hostRoom = dExtNpcMount_roomLaneHostRoom(pending.proc);
            if (hostRoom >= 0) {
                // №65: room-lane EXIT transport — unload + place at return_pos (no BgWarp).
                const bool ok = dExtNpcMount_requestRoomLaneExit(
                    pending.proc, pending.exitSpawn, pending.exitFacing, pending.hasExitFacing);
                if (ok) {
                    DuskLog.info(
                        "[Doors] exit '{}' via door '{}' transport=room → return_pos",
                        pending.proc, pending.doorName);
                }
            } else {
                const bool ok = pending.hasExitFacing
                                    ? dExtNpcMount_requestBgWarpTo("EXT_BG0", pending.exitSpawn,
                                                                   pending.exitFacing)
                                    : dExtNpcMount_requestBgWarpTo("EXT_BG0", pending.exitSpawn);
                if (ok) {
                    DuskLog.info(
                        "[Doors] exit '{}' via door '{}' transport=pinned → EXT_BG0", pending.proc,
                        pending.doorName);
                }
            }
        }
    }
    (void)island;
    return true;
}

bool createKnobAt(const char* src, const cXyz& pos, s16 ry, int roomNo, int doorIndex,
                  bool isExit) {
    csXyz angle;
    angle.set(0, ry, 0);
    cXyz scale(1.0f, 1.0f, 1.0f);

    // ============================================================
    // §380 donor params. The old §27 scheme (25 | doorIndex<<8 | exit
    // bit16) landed doorIndex in the DONOR's door-TYPE nibble — type 1
    // turned linkrm into a message door, type 2 made ojhous a night-
    // message door (§379c receipts) — and bit16 polluted the eventNo
    // field. The native actor reads donor semantics, so it gets the
    // donor's own normal-door param, byte-identical to what WW's DZR
    // gives its doors (swbit FF / type 0 / eventNo FF / swbit2 FF).
    // Identity + exit-ness live entirely on the doorKey channel
    // ('door:'/'door:exit:' src → mDoorKey, №51 persist + reconcile).
    // The henna0 mount FALLBACK keeps the legacy encoding — its own
    // decode (stampKnobByIndex/bit16) expects it; retire-listed with
    // the mount class.
    // ============================================================
    const u32 kn00Params = 0x0ffff0ffu;
    const u32 hennaParams = 25u | ((u32)doorIndex << 8) | (isExit ? 0x10000u : 0u);

    // §27: prefer real KNOB00 port; fall back to ExtNpc NPC_KNOB mount.
    dExtNpcMount_pushPendingSpawn("NPC_KNOB", src, NULL, NULL);
    const fpc_ProcID kid00 =
        fopAcM_create(fpcNm_KNOB00_e, kn00Params, &pos, roomNo, &angle, &scale, -1);
    if (kid00 != fpcM_ERROR_PROCESS_ID_e) {
        dExtNpcMount_bindPendingSpawn(kid00, "NPC_KNOB", src, NULL, NULL);
        DuskLog.info("[Doors] §27 spawn KNOB00 src='{}'", src != NULL ? src : "?");
        return true;
    }
    char discard[8];
    dExtNpcMount_takePendingSpawn(fpcM_ERROR_PROCESS_ID_e, discard, sizeof(discard), NULL, 0,
                                  NULL, 0, NULL, 0);

    const s16 actorId = dExtNpcMount_socketActorId("NPC_HENNA0");
    if (actorId < 0) {
        return false;
    }
    dExtNpcMount_pushPendingSpawn("NPC_KNOB", src, NULL, NULL);
    const fpc_ProcID kid = fopAcM_create(actorId, hennaParams, &pos, roomNo, &angle, &scale, -1);
    if (kid != fpcM_ERROR_PROCESS_ID_e) {
        dExtNpcMount_bindPendingSpawn(kid, "NPC_KNOB", src, NULL, NULL);
        return true;
    }
    dExtNpcMount_takePendingSpawn(fpcM_ERROR_PROCESS_ID_e, discard, sizeof(discard), NULL, 0,
                                  NULL, 0, NULL, 0);
    return false;
}

void spawnExitKnobIfNeeded(const char* interiorProc) {
    if (interiorProc == NULL || std::strcmp(interiorProc, "EXT_BG0") == 0) {
        return;
    }
    if (std::strcmp(s_exitSpawnForProc, interiorProc) == 0) {
        return;
    }
    DoorDef* d = findDoorByEnterProc(interiorProc);
    if (d == NULL || !d->wantKnob || d->exitKnobSpawned) {
        if (d != NULL) {
            std::snprintf(s_exitSpawnForProc, sizeof(s_exitSpawnForProc), "%s", interiorProc);
        }
        return;
    }
    dExtNpcManifest dest{};
    if (!dExtNpcMount_lookup(interiorProc, &dest) || !dest.isBg) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    // №56: exit prop ON the room's real door (exit_door_rel / KNOB00), not a landing offset.
    cXyz pos = dest.hostPos;
    if (dest.hasExitDoorRel) {
        pos = dest.hostPos + dest.exitDoorRel;
    } else {
        pos = dest.hostPos + dest.spawnRel;
    }
    const s16 faceRy = dest.hasSpawnRy ? dest.spawnRy : (d->hasWwRy ? d->wwRy : (s16)0);
    // ============================================================
    // §378 NATIVE-DOOR ADOPTION (duplicate-doors defect, log 194812
    // pids 360+378): since §329 put KNOB00 in the stage OBJNAME table,
    // the donor room's OWN door actor (e.g. LinkRM room.dzr KNOB00 at
    // -255,0,1125 — the exact exit_door_rel) spawns natively from the
    // room-lane actor list BEFORE this synthetic spawn — two coincident
    // doors. The donor door IS the door: adopt it (stamp the exit door
    // key so the warp path follows it) instead of spawning a twin. The
    // synthetic create below remains only as the fallback for rooms
    // whose donor data carries no door.
    // ============================================================
    if (daKnob00_c* nativeDoor = findKnob00Near(pos, 80.0f)) {
        stampKnob00(nativeDoor, *d, true);
        d->exitKnobHost = pos;
        d->hasExitKnobHost = true;
        d->exitKnobSpawned = true;
        std::snprintf(s_exitSpawnForProc, sizeof(s_exitSpawnForProc), "%s", interiorProc);
        DuskLog.info("[Doors] §378 adopt NATIVE room-lane door key=exit:{} pos=({:.1f},{:.1f},{:.1f})",
                     d->name, pos.x, pos.y, pos.z);
        return;
    }
    char src[64];
    std::snprintf(src, sizeof(src), "door:exit:%s", d->name);
    // №66-B: stamp the claimed host room, not Link's stale roomNo (void re-entry).
    int roomNo = fopAcM_GetRoomNo(player);
    const int hostRoom = dExtNpcMount_roomLaneHostRoom(interiorProc);
    if (hostRoom >= 0) {
        roomNo = hostRoom;
    }
    if (createKnobAt(src, pos, faceRy, roomNo, d->doorIndex, /*isExit=*/true)) {
        d->exitKnobHost = pos;
        d->hasExitKnobHost = true;
        d->exitKnobSpawned = true;
        std::snprintf(s_exitSpawnForProc, sizeof(s_exitSpawnForProc), "%s", interiorProc);
        DuskLog.info("[Doors] exit prop key=exit:{} pos=({:.1f},{:.1f},{:.1f}) ry={}", d->name,
                     pos.x, pos.y, pos.z, (int)faceRy);
    }
}

void reconcileExitKnob(DoorDef& d) {
    if (!d.hasExitKnobHost) {
        return;
    }
    if (daKnob00_c* k00 = findKnob00Near(d.exitKnobHost, 80.0f)) {
        stampKnob00(k00, d, true);
        return;
    }
    dExtNpcMount_c* knob = findKnobNear(d.exitKnobHost, 80.0f);
    if (knob != NULL) {
        stampKnob(knob, d, true);
    }
}

// №53-C G-guard: leave interior shell AABB without warp → porch teleport.
void leaveShellGuard(const dExtNpcManifest& island, fopAc_ac_c* player) {
    const char* last = dExtNpcMount_lastBgProc();
    if (last == NULL || std::strcmp(last, "EXT_BG0") == 0 || dExtNpcMount_bgWarpBusy()) {
        return;
    }
    dExtNpcManifest dest{};
    if (!dExtNpcMount_lookup(last, &dest) || !dest.isBg || !dest.hasHostPos) {
        return;
    }
    // Shell pad around host + spawn (rooms are small; 8k xz / 4k y is generous).
    const cXyz center = dest.hostPos + dest.spawnRel;
    const f32 dx = player->current.pos.x - center.x;
    const f32 dy = player->current.pos.y - center.y;
    const f32 dz = player->current.pos.z - center.z;
    const f32 limXZ = 8000.0f;
    const f32 limY = 4000.0f;
    if (dx * dx + dz * dz <= limXZ * limXZ && dy > -limY && dy < limY) {
        return;
    }
    DoorDef* d = findDoorByEnterProc(last);
    if (d == NULL) {
        return;
    }
    const cXyz spawn = outdoorReturn(island, *d, dest);
    const s16 facing = outdoorReturnFacing(*d, dest);
    DuskLog.warn(
        "[Doors] leave-shell G-guard — '{}' outside AABB → porch '{}' ({:.0f},{:.0f},{:.0f})", last,
        d->name, spawn.x, spawn.y, spawn.z);
    beginDoorFade();
    dExtNpcMount_beginDoorDemoLock();
    if (dExtNpcMount_isRoomLaneProc(last)) {
        dExtNpcMount_requestRoomLaneExit(last, spawn, facing, true);
    } else {
        dExtNpcMount_requestBgWarpTo("EXT_BG0", spawn, facing);
    }
    s_cooldown = 90;
}

// №66-C / №74: stranding diagnostics. WARN-ONLY while stable (proximity "exit=0" was
// yanking healthy rooms when Link rolled away from the door). Re-arm actuation after
// the stage-per-interior pivot; existence must be registry-based, never player radius.
static int s_strandBadFrames = 0;
static int s_strandWarnCooldown = 0;

void roomLaneStrandGuard(const dExtNpcManifest& island, fopAc_ac_c* player) {
    (void)island;
    if (s_strandWarnCooldown > 0) {
        --s_strandWarnCooldown;
    }
    const char* last = dExtNpcMount_lastBgProc();
    if (last == NULL || std::strcmp(last, "EXT_BG0") == 0 || dExtNpcMount_bgWarpBusy()) {
        s_strandBadFrames = 0;
        return;
    }
    if (!dExtNpcMount_isRoomLaneProc(last)) {
        s_strandBadFrames = 0;
        return;
    }
    const int hostRoom = dExtNpcMount_roomLaneHostRoom(last);
    const bool drawable =
        hostRoom > 0 && dComIfGp_roomControl_checkRoomDisp(hostRoom) != FALSE;
    DoorDef* d = findDoorByEnterProc(last);
    // №74: registry existence (spawn recorded), not proximity to the player.
    const bool exitRegistered = d != NULL && d->exitKnobSpawned;
    dExtNpcMount_c* nearExit = dExtNpcMount_nearestDoorAttention(player->current.pos, 300.0f);
    const bool exitInRadius =
        nearExit != NULL &&
        (std::strncmp(nearExit->mDoorKey, "exit:", 5) == 0 ||
         (nearExit->mSpawnSrc[0] && std::strstr(nearExit->mSpawnSrc, "exit:") != NULL));

    // Healthy room with a registered exit — never treat "walked away" as stranded.
    if (drawable && exitRegistered) {
        s_strandBadFrames = 0;
        return;
    }
    ++s_strandBadFrames;
    if (s_strandBadFrames < 120) {
        return;
    }
    if (s_strandWarnCooldown > 0) {
        return;
    }
    dExtNpcManifest dest{};
    const bool haveDest = dExtNpcMount_lookup(last, &dest) && dest.isBg;
    DuskLog.warn(
        "[Doors] №74 stranding WARN-ONLY — '{}' drawable={} exitReg={} exitNear={} "
        "player=({:.0f},{:.0f},{:.0f}) host=({:.0f},{:.0f},{:.0f}) — no auto-warp",
        last, drawable ? 1 : 0, exitRegistered ? 1 : 0, exitInRadius ? 1 : 0,
        player->current.pos.x, player->current.pos.y, player->current.pos.z,
        haveDest ? dest.hostPos.x : 0.0f, haveDest ? dest.hostPos.y : 0.0f,
        haveDest ? dest.hostPos.z : 0.0f);
    s_strandBadFrames = 0;
    s_strandWarnCooldown = 300;  // ~5s between identical warns
}

bool tryWarpFromKnob(dExtNpcMount_c* knob, fopAc_ac_c* player, f32 distXZ, bool openAlreadyDone) {
    if (knob == NULL) {
        return false;
    }
    dExtNpcManifest island{};
    if (!dExtNpcMount_lookup("EXT_BG0", &island)) {
        return false;
    }
    char keyBuf[32] = {};
    const char* key = knob->mDoorKey[0] ? knob->mDoorKey : NULL;
    if (key == NULL && std::strncmp(knob->mSpawnSrc, "door:", 5) == 0) {
        std::snprintf(keyBuf, sizeof(keyBuf), "%s", knob->mSpawnSrc + 5);
        key = keyBuf;
    }
    if (key == NULL) {
        DoorDef* byHost = findDoorByKnobHost(knob->current.pos);
        if (byHost == NULL) {
            return false;
        }
        stampKnob(knob, *byHost, false);
        key = knob->mDoorKey;
    }
    const bool isExit = (std::strncmp(key, "exit:", 5) == 0);
    DoorDef* d = findDoorByName(key);
    if (d == NULL) {
        d = findDoorByKnobHost(knob->current.pos);
    }
    if (d == NULL) {
        return false;
    }
    if (isExit) {
        return queueExit(*d, island, player, openAlreadyDone);
    }
    return queueEnter(*d, island, player, distXZ, key, openAlreadyDone);
}

}  // namespace

void dExtNpcDoors_clearSpawnLatches() {
    s_knobsSpawned = false;
    s_knobsSpawnedGen = 0;
    s_reconcileDone = false;
    s_exitSpawnForProc[0] = '\0';
    for (DoorDef& d : s_doors) {
        d.exitKnobSpawned = false;
        d.hasExitKnobHost = false;
        d.hasKnobHost = false;
        d.bootLogged = false;
    }
}

bool dExtNpcDoors_knobsLatched() {
    return s_knobsSpawned && s_knobsSpawnedGen == dExtNpcWorld_generation();
}

// §270: stand the arrival-G-guard (№89) down when a native WW NPC (ba1) legitimately ORDERS
// an event (the tale). The guard force-ends any event still active kArrivalGuardFrames after
// arrival — it exists to clear a STUCK arrival residual, but can't distinguish that from ba1's
// fresh orderOtherEventId('tale_1'), so it truncated the tale at frame 0 ("force-end event
// (arrival-G-guard)"). Standing it down lets the tale run; the guard RE-ARMS on the next
// arrival, so this is scoped — not a disable. (Outside the anon namespace for external linkage;
// s_arrival is still in scope in this TU, same as dExtNpcDoors_knobsLatched above.)
void dExtNpcMount_clearArrivalGuard(const char* why) {
    if (s_arrival.guardFramesLeft > 0) {
        DuskLog.info("[Doors] §270 arrival-G-guard stood down ({}) — native WW event owns the slot",
                     why != NULL ? why : "?");
    }
    s_arrival.guardFramesLeft = -1;
}

int dExtNpcDoors_wantOutdoorKnobCount() {
    ensureLoaded();
    int n = 0;
    for (const DoorDef& d : s_doors) {
        if (d.wantKnob) {
            ++n;
        }
    }
    return n;
}

int dExtNpcDoors_countLiveOutdoorKnobs() {
    struct Ctx {
        int n;
    } ctx{0};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            Ctx* c = (Ctx*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name == fpcNm_KNOB00_e) {
                daKnob00_c* k = (daKnob00_c*)ac;
                if (std::strncmp(k->spawnSrc(), "door:", 5) == 0 &&
                    std::strncmp(k->spawnSrc(), "door:exit:", 10) != 0) {
                    ++c->n;
                } else if (k->doorKey()[0] != '\0' && std::strncmp(k->doorKey(), "exit:", 5) != 0) {
                    ++c->n;
                }
                return 0;
            }
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || !m->mManifest.doorAttention) {
                return 0;
            }
            // Outdoor enter knobs: door:<name> (not exit:).
            if (std::strncmp(m->mSpawnSrc, "door:", 5) == 0 &&
                std::strncmp(m->mSpawnSrc, "door:exit:", 10) != 0) {
                ++c->n;
            } else if (m->mDoorKey[0] != '\0' && std::strncmp(m->mDoorKey, "exit:", 5) != 0) {
                ++c->n;
            }
            return 0;
        },
        &ctx);
    return ctx.n;
}

void dExtNpcDoors_clearExitKnobForProc(const char* interiorProc) {
    ensureLoaded();
    if (interiorProc == NULL || interiorProc[0] == '\0') {
        return;
    }
    DoorDef* d = findDoorByEnterProc(interiorProc);
    if (d != NULL) {
        d->exitKnobSpawned = false;
        d->hasExitKnobHost = false;
    }
    if (std::strcmp(s_exitSpawnForProc, interiorProc) == 0) {
        s_exitSpawnForProc[0] = '\0';
    }
}

void dExtNpcDoors_stampKnobByIndex(dExtNpcMount_c* knob, int doorIndex1Based) {
    ensureLoaded();
    if (knob == NULL || doorIndex1Based <= 0) {
        return;
    }
    for (DoorDef& d : s_doors) {
        if (d.doorIndex == doorIndex1Based) {
            const bool isExit = (fopAcM_GetParam(knob) & 0x10000) != 0;
            stampKnob(knob, d, isExit);
            return;
        }
    }
}

bool dExtNpcDoors_isMountDoor(fopAc_ac_c* actor) {
    if (actor == NULL) {
        return false;
    }
    const s16 name = fopAcM_GetName(actor);
    if (name == fpcNm_KNOB00_e) {
        return true;
    }
    if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
        name != fpcNm_NPC_KDK_e) {
        return false;
    }
    dExtNpcMount_c* m = (dExtNpcMount_c*)actor;
    return !m->mIsBg && m->mManifest.doorAttention;
}

void dExtNpcDoors_stampKnob00(fopAc_ac_c* knob, const char* doorKey) {
    if (knob == NULL || fopAcM_GetName(knob) != fpcNm_KNOB00_e || doorKey == NULL) {
        return;
    }
    static_cast<daKnob00_c*>(knob)->setDoorKey(doorKey);
}

static bool tryWarpFromKey(const char* key, const cXyz& pos, fopAc_ac_c* player, f32 distXZ,
                           bool openAlreadyDone) {
    dExtNpcManifest island{};
    if (!dExtNpcMount_lookup("EXT_BG0", &island) || key == NULL || key[0] == '\0') {
        return false;
    }
    const bool isExit = (std::strncmp(key, "exit:", 5) == 0);
    DoorDef* d = findDoorByName(key);
    if (d == NULL) {
        d = findDoorByKnobHost(pos);
    }
    if (d == NULL) {
        return false;
    }
    if (isExit) {
        return queueExit(*d, island, player, openAlreadyDone);
    }
    return queueEnter(*d, island, player, distXZ, key, openAlreadyDone);
}

bool dExtNpcDoors_tryNativeWarp(fopAc_ac_c* doorActor, bool openAlreadyDone) {
    if (!dExtNpcDoors_isMountDoor(doorActor) || s_pending.active || s_cooldown > 0) {
        return false;
    }
    if (dComIfGp_isEnableNextStage()) {
        return false;
    }
    if (dExtNpcMount_bgWarpBusy()) {
        return false;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);

    // §27 KNOB00 path — key on the actor; heal from spawn src / host pos.
    if (fopAcM_GetName(doorActor) == fpcNm_KNOB00_e) {
        daKnob00_c* k00 = static_cast<daKnob00_c*>(doorActor);
        const f32 dist =
            player != NULL ? (player->current.pos - k00->current.pos).absXZ() : -1.0f;
        char keyBuf[32] = {};
        const char* key = k00->doorKey();
        if (key == NULL || key[0] == '\0') {
            if (std::strncmp(k00->spawnSrc(), "door:", 5) == 0 && k00->spawnSrc()[5]) {
                std::snprintf(keyBuf, sizeof(keyBuf), "%s", k00->spawnSrc() + 5);
                k00->setDoorKey(keyBuf);
                key = k00->doorKey();
            } else {
                DoorDef* byHost = findDoorByKnobHost(k00->current.pos);
                if (byHost != NULL) {
                    k00->setDoorKey(byHost->name);
                    key = k00->doorKey();
                }
                for (DoorDef& d : s_doors) {
                    if (!d.hasExitKnobHost) {
                        continue;
                    }
                    const f32 dx = k00->current.pos.x - d.exitKnobHost.x;
                    const f32 dz = k00->current.pos.z - d.exitKnobHost.z;
                    if (dx * dx + dz * dz < 80.0f * 80.0f) {
                        char ek[32];
                        std::snprintf(ek, sizeof(ek), "exit:%s", d.name);
                        k00->setDoorKey(ek);
                        key = k00->doorKey();
                        break;
                    }
                }
            }
        }
#if TARGET_PC
        // ================================================================
        // §379c — fallback-press probe + heal (log 211755: exterior 'linkrm'
        // fell through with an unresolved event id while 'ojhous' ran the
        // native event; door-specific). Hypotheses discriminated per press:
        //   H1 resolve-timing (CreateInit ran before event data) → the heal
        //      below re-resolves; post!=-1 with pre==-1 confirms
        //   H2 donor checkArea/angle gate never set eventInfo (pre!=-1 here
        //      but entry saw -1 → setEventPrm gate is the blocker)
        //   H3 room-typing (roomNo printed; linkrm is the room-lane door)
        // On heal success the NEXT press rides the native event; this
        // fallback still carries this one (never strand a press).
        // ================================================================
        if (!openAlreadyDone) {
#if DUSK_WW_KNOB00_NATIVE
            const s16 pre7 = k00->mEventIdx[7];
            const s16 pre8 = k00->mEventIdx[8];
            if (pre7 == -1 && pre8 == -1) {
                k00->makeEventId(0);
            }
            DuskLog.warn("[Doors] §379c FALLBACK press key='{}' room={} src='{}' "
                         "evIdx7 {}→{} evIdx8 {}→{}",
                         key != NULL ? key : "?", (int)fopAcM_GetRoomNo(k00), k00->spawnSrc(),
                         (int)pre7, (int)k00->mEventIdx[7], (int)pre8, (int)k00->mEventIdx[8]);
#endif
        }
#endif
        DuskLog.info("[Doors] {} OPEN → KNOB00 key='{}'",
                     openAlreadyDone ? "§27 post-cutEnd" : "§27 native", key != NULL ? key : "?");
        return tryWarpFromKey(key, k00->current.pos, player, dist, openAlreadyDone);
    }

    dExtNpcMount_c* knob = (dExtNpcMount_c*)doorActor;
    const f32 dist =
        player != NULL ? (player->current.pos - knob->current.pos).absXZ() : -1.0f;
    // №54-5: heal empty exit keys before the OPEN ledger line.
    if (knob->mDoorKey[0] == '\0') {
        if (std::strncmp(knob->mSpawnSrc, "door:", 5) == 0 && knob->mSpawnSrc[5]) {
            std::snprintf(knob->mDoorKey, sizeof(knob->mDoorKey), "%s", knob->mSpawnSrc + 5);
        } else {
            DoorDef* byHost = findDoorByKnobHost(knob->current.pos);
            if (byHost != NULL) {
                stampKnob(knob, *byHost, false);
            }
            for (DoorDef& d : s_doors) {
                if (d.hasExitKnobHost) {
                    const f32 dx = knob->current.pos.x - d.exitKnobHost.x;
                    const f32 dz = knob->current.pos.z - d.exitKnobHost.z;
                    if (dx * dx + dz * dz < 80.0f * 80.0f) {
                        stampKnob(knob, d, true);
                        break;
                    }
                }
            }
        }
    }
    DuskLog.info("[Doors] {} OPEN → prop='{}'", openAlreadyDone ? "№91 post-cutEnd" : "№53 native",
                 knob->mDoorKey[0] ? knob->mDoorKey : "?");
    if (tryWarpFromKnob(knob, player, dist, openAlreadyDone)) {
        for (DoorDef& e : s_doors) {
            e.wasInEnter = true;
            e.wasInExit = true;
        }
        return true;
    }
    return false;
}

void dExtNpcDoors_spawnKnobs(const dExtNpcManifest& island) {
    ensureLoaded();
    const u32 gen = dExtNpcWorld_generation();
    // №94: latch is per world-generation (survives only until play-scene recreate).
    if ((s_knobsSpawned && s_knobsSpawnedGen == gen) || s_doors.empty()) {
        return;
    }
    if (!dExtNpcMount_hasPayload("NPC_KNOB")) {
        DuskLog.debug("[ExtNpcDoors] knobs skipped — no NPC_KNOB payload (door arc missing)");
        s_knobsSpawned = true;
        s_knobsSpawnedGen = gen;
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    const int roomNo = fopAcM_GetRoomNo(player);
    int n = 0;
    // №53-D: ONE loop over doors.ini order — create + pending src + index in params.
    for (DoorDef& d : s_doors) {
        if (!d.wantKnob) {
            continue;
        }
        cXyz pos = wwToHost(island, d.wwPos);
        char src[64];
        std::snprintf(src, sizeof(src), "door:%s", d.name);
        if (createKnobAt(src, pos, d.hasWwRy ? d.wwRy : (s16)0, roomNo, d.doorIndex,
                         /*isExit=*/false)) {
            d.knobHost = pos;
            d.hasKnobHost = true;
            ++n;
        }
    }
    s_knobsSpawned = true;
    s_knobsSpawnedGen = gen;
    s_reconcileDone = false;
    DuskLog.info("[ExtNpcDoors] spawned {} door props (doors.ini order) gen={}", n, gen);
}

void dExtNpcDoors_armArrivalGuard(const char* stage) {
    if (stage == NULL || stage[0] == '\0') {
        return;
    }
    // Door-lane already owns a full arrival demo for this stage — keep it.
    if (s_arrival.armed && std::strcmp(s_arrival.stage, stage) == 0) {
        return;
    }
    s_arrival = {};
    s_arrival.armed = true;
    s_arrival.withDemo = false;
    s_arrival.demoFramesLeft = -1;
    s_arrival.guardFramesLeft = -1;
    std::snprintf(s_arrival.stage, sizeof(s_arrival.stage), "%s", stage);
    DuskLog.info("[Doors] №90 arm arrival G-guard (no demo) → '{}'", s_arrival.stage);
}

void dExtNpcDoors_requestPostOpeningSnap(const char* stage) {
    if (stage == NULL || stage[0] == '\0') {
        return;
    }
    // Keep door-lane arrival demo ownership if still mid-demo; only clear the
    // snap latch so №110 can fire once the opening has released the camera.
    if (s_arrival.armed && s_arrival.withDemo && !s_arrival.demoEnded &&
        std::strcmp(s_arrival.stage, stage) == 0) {
        s_arrival.cameraSnapped = false;
        DuskLog.info("[Doors] №176 post-opening snap — cleared latch (door demo still live)");
        return;
    }
    s_arrival = {};
    s_arrival.armed = true;
    s_arrival.withDemo = false;
    s_arrival.demoStarted = true;
    s_arrival.demoEnded = true;
    s_arrival.cameraSnapped = false;
    s_arrival.guardFramesLeft = 30;  // brief window to snap + re-activate after Stop()
    std::snprintf(s_arrival.stage, sizeof(s_arrival.stage), "%s", stage);
    DuskLog.info("[Doors] №176 post-opening snap re-armed → '{}'", s_arrival.stage);
}

void dExtNpcDoors_onInteriorBgReady(const char* interiorProc) {
    ensureLoaded();
    if (interiorProc == NULL || interiorProc[0] == '\0') {
        return;
    }
    spawnExitKnobIfNeeded(interiorProc);
    DoorDef* ed = findDoorByEnterProc(interiorProc);
    if (ed != NULL) {
        reconcileExitKnob(*ed);
    }
}

void dExtNpcDoors_pollArrival() {
    // ========================================================================
    // §890 WATCHDOG, refined by run 165821's NEGATIVE: during a live control
    // loss the §898 stall receipts below never fired — so the door-demo lock
    // can be held while arrival is DISARMED (an ORPHANED lock: begin ran, no
    // reachable end). Keyed on the LOCK ITSELF, before the armed gate, so the
    // orphan case is the one this cannot miss. Bundle dumps every 600 frames
    // of continuous lock; armed=0 in that line IS the orphan signature.
    // ========================================================================
    {
        static int s_lockFrames = 0;
        if (dExtNpcMount_isDoorDemoLocked()) {
            if ((++s_lockFrames % 600) == 0) {
                const char* cur = dComIfGp_getStartStageName();
                DuskLog.warn("[Doors] §890 DOOR LOCK HELD {}f — armed={} armedStage='{}' "
                             "curStage='{}' withDemo={} demoStarted={} demoEnded={} "
                             "isEnableNextStage={} evtRun={}",
                             s_lockFrames, s_arrival.armed ? 1 : 0,
                             s_arrival.armed ? s_arrival.stage : "-",
                             cur != NULL ? cur : "?",
                             s_arrival.withDemo ? 1 : 0, s_arrival.demoStarted ? 1 : 0,
                             s_arrival.demoEnded ? 1 : 0,
                             dComIfGp_isEnableNextStage() ? 1 : 0,
                             dComIfGp_event_runCheck() ? 1 : 0);
            }
        } else {
            s_lockFrames = 0;
        }
    }
    if (!s_arrival.armed) {
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, s_arrival.stage) != 0) {
        // ====================================================================
        // tale §898 (CALLS row 68): the two silent early-returns below are the
        // §890 candidate stalls — a stall here while the door demo lock is held
        // is a PERMANENT player lock with every event measure reading normal.
        // LOUD once per 120 frames of continuous stalling, with the held state.
        // ====================================================================
        static int s_stallA = 0;
        if ((++s_stallA % 120) == 0) {
            DuskLog.warn("[Doors] §898 pollArrival STALLED {}f on stage-mismatch — armed='{}' "
                         "cur='{}' demoStarted={} demoEnded={}",
                         s_stallA, s_arrival.stage, stage != NULL ? stage : "?",
                         s_arrival.demoStarted ? 1 : 0, s_arrival.demoEnded ? 1 : 0);
        }
        return;  // still wiping / wrong stage
    }
    if (dComIfGp_isEnableNextStage()) {
        static int s_stallB = 0;
        if ((++s_stallB % 120) == 0) {
            DuskLog.warn("[Doors] §898 pollArrival STALLED {}f on isEnableNextStage — armed='{}' "
                         "demoStarted={} demoEnded={}",
                         s_stallB, s_arrival.stage, s_arrival.demoStarted ? 1 : 0,
                         s_arrival.demoEnded ? 1 : 0);
        }
        return;  // another change already armed
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }

    // §158 / №269: native setNextStage into R_DL* never hits room-lane place, so the
    // ClrWallNone that lived only on forceLinkGroundReprobe's room-lane callers never
    // ran → WALL_NONE stuck → ladders dead. Same №269 clear, on the path Link takes.
    if (!s_arrival.demoStarted) {
        dExtNpcMount_forceLinkGroundReprobe(player);
    }

    // №121 Ask 3 / №133: WW hosts sit 200k+ from origin. QuickStart alone leaves eye/center
    // at the prior stage's map-edge. Snap like Shade Refuge: Reset behind Link, then
    // QuickStart. Fire after Link exists; for door arrivals wait until demo ends so the
    // demo lock cannot overwrite it.
    auto snapArrivalCamera = [&]() {
        if (s_arrival.cameraSnapped) {
            return;
        }
        // №167 / №170: a running demo OWNS the camera — do not fight it.
        //
        // The comment above says door arrivals wait for the demo so "the demo lock
        // cannot overwrite it". The reverse is the problem here: this is a WARP
        // arrival (the crossing), which takes the `!withDemo` path and never waits,
        // so the snap fired while the opening storyboard was driving its own ~12 s
        // pan. Two owners, one camera — the user saw the old map-edge spawn shot
        // win against the storyboard.
        //
        // №167 deferred while event_runCheck / dDemo camera. №171 prefers
        // `deferArrivalCameraSnap` (hold after ORDER, or still trying to order).
        // G-guard pause stays on `openingPauseArrivalGuard` only after ORDER.
        //
        // Deliberately does NOT latch `cameraSnapped`: the snap is DEFERRED, not
        // cancelled. When the hold clears the camera is unowned again and the next
        // poll snaps behind Link, which is the right place to hand control back.
        if (dExtWw_deferArrivalCameraSnap() || dComIfGp_event_runCheck() ||
            dDemo_c::getCamera() != NULL) {
            return;
        }
        dCamera_c* cam = dCam_getBody();
        if (cam == NULL || player == NULL) {
            return;
        }
        const f32 kDist = 420.0f;
        const f32 kEyeH = 200.0f;
        const f32 kLookH = 140.0f;
        const s16 a = player->shape_angle.y;
        const f32 fwdX = cM_ssin(a);
        const f32 fwdZ = cM_scos(a);
        cXyz center(player->current.pos.x, player->current.pos.y + kLookH, player->current.pos.z);
        cXyz eye(player->current.pos.x - fwdX * kDist, player->current.pos.y + kEyeH,
                 player->current.pos.z - fwdZ * kDist);
        cam->Reset(center, eye);
        cam->QuickStart();
        s_arrival.cameraSnapped = true;
        DuskLog.info("[Doors] №110 QuickStart snap stage='{}' exit={}", s_arrival.stage,
                     s_arrival.isExit ? 1 : 0);
    };

    // №90: warp / non-door arrival — residual clear then G-guard only.
    if (!s_arrival.withDemo) {
        if (!s_arrival.demoStarted) {
            // №170: residual MUST clear the busy slot before awake can order.
            // Only skip once the post-ORDER hold is live (our opening, not a leftover).
            if (!dExtWw_openingPauseArrivalGuard() &&
                (dComIfGp_event_runCheck() || dComIfGp_event_getMode() != dEvt_mode_WAIT_e)) {
                dExtNpcMount_forceEndDoorEvent("arrival-residual");
            }
            s_arrival.demoStarted = true;
            s_arrival.demoEnded = true;
            s_arrival.guardFramesLeft = kArrivalGuardFrames;
            snapArrivalCamera();
            DuskLog.info("[Doors] №90 arrival G-guard START stage='{}' frames={}", s_arrival.stage,
                         kArrivalGuardFrames);
        }
        // fall through to shared G-guard countdown below
    } else if (!s_arrival.demoStarted) {
        // First frame on dest: clear any residual hollow event, then begin arrival demo.
        if (!dExtWw_openingPauseArrivalGuard() &&
            (dComIfGp_event_runCheck() || dComIfGp_event_getMode() != dEvt_mode_WAIT_e)) {
            dExtNpcMount_forceEndDoorEvent("arrival-residual");
        }
        if (s_arrival.hasFacing) {
            player->current.angle.y = s_arrival.facing;
            player->shape_angle.y = s_arrival.facing;
        }
        dExtNpcMount_beginDoorDemoLock();
        s_arrival.demoStarted = true;
        s_arrival.demoFramesLeft = kArrivalDemoFrames;
        DuskLog.info("[Doors] №89 arrival demo BEGIN stage='{}' exit={} frames={}",
                     s_arrival.stage, s_arrival.isExit ? 1 : 0, kArrivalDemoFrames);
        return;
    }

    if (!s_arrival.demoEnded) {
        if (--s_arrival.demoFramesLeft > 0) {
            return;
        }
        // Begin and end in the same destination context.
        dExtNpcMount_endDoorDemoLock();
        dExtNpcMount_forceEndDoorEvent("arrival-end");
        s_arrival.demoEnded = true;
        // ============================================================
        // §161 / №269: flag-only clear. G-2 reused the full reprobe and
        // its ClrGroundHit+CrrPos reposition froze Link post-door-demo.
        // The ladder needs the FLAGS cleared, not a ground reprobe.
        // ============================================================
        {
            daAlink_c* link = (daAlink_c*)player;
            const u32 before = link->mLinkAcch.GetFlags();
            link->mLinkAcch.ClrWallNone();
            link->mLinkAcch.OffLineCheckNone();
            if ((before & dBgS_Acch::FLAG_WALL_NONE) != 0) {
                DuskLog.info("[Doors] §161 flag-only ClrWallNone post-demo ({:#x} → {:#x})",
                             (unsigned)before, (unsigned)link->mLinkAcch.GetFlags());
            }
        }
        s_arrival.guardFramesLeft = kArrivalGuardFrames;
        snapArrivalCamera();
        DuskLog.info("[Doors] №89 arrival demo END stage='{}' — G-guard {}f", s_arrival.stage,
                     kArrivalGuardFrames);
        return;
    }

    // Retry snap if cam body was null on the first opportunity.
    snapArrivalCamera();

    if (s_arrival.guardFramesLeft > 0) {
        // №170: pause ONLY after awake has ordered. Pre-order G-guard must still
        // force-end leftovers so the order slot frees (№169 / log 035311).
        if (dExtWw_openingPauseArrivalGuard()) {
            return;
        }
        --s_arrival.guardFramesLeft;
        if (s_arrival.guardFramesLeft > 0) {
            return;
        }
        // ============================================================
        // §399 — THE G-GUARD KILL LEG IS DELETED (user ruling executed).
        // Its victim ledger: the held tale box (§304), the TALE_DEMO
        // storyboard (§377/KB-1, log 194812), and the scripted B_OPEN
        // door event 21f in (log 094033 gFrm=1692 — collateral: Link's
        // door proc died mid-SetWallNone → interior collision loss,
        // dead door, stale late arm, tale black screen). Each strike
        // needed a new exemption patch — the watchdog-outlived-its-
        // context signature the user's ruling named. Every departure is
        // native now (§379-§380); a genuine hang is still caught by the
        // 3600f pollStuckMessageResume backstop. The countdown/disarm/
        // camera bookkeeping below stays — only the kill is gone.
        // §313/§377 exemptions die with the leg (nothing left to exempt).
        // ============================================================
        DuskLog.info("[Doors] №89 G-guard disarm — stage='{}' (kill leg deleted §399)",
                     s_arrival.stage);
        // №176: G-guard often ends BEFORE awake's pan. If №110 never latched and
        // the opening still owns/defers the camera, stay armed so the deferred
        // snap can fire after RELEASE (otherwise pollArrival never runs again).
        if (!s_arrival.cameraSnapped && dExtWw_deferArrivalCameraSnap()) {
            s_arrival.guardFramesLeft = 1;
            DuskLog.info(
                "[Doors] №176 keep arrival armed — snap still deferred (opening owns cam)");
            return;
        }
        s_arrival.armed = false;
    }
}

// ============================================================
// §379b — WW Link-staff interpreter for the native door events.
// DEFAULT_KNOB_DOOR_F_OPEN/B_OPEN (merged from WW's own event data,
// §379a) carry a 'Link' staff with WW's acting vocabulary. The evmng
// name alias (d_event_manager.cpp:1097) binds TP Link to the 'Link'
// staff; this poll performs the cuts with TP Link's NATIVE acting —
// the same translation class as the alias itself:
//   001n_wait / 012unequip / 005wait_turn → cutEnd (TP Link unequips
//     on the door command already, and the knob's own SETANGLE/
//     ADJUSTMENT cuts place and turn Link — donor division of labor);
//   035door prm0 → changeDemoMode(DEMO_DOOR_OPEN_e, prm0) — TP's
//     procDoorOpenInit reads param0&1 for the right/left swing, the
//     SAME semantic WW's dProcDoorOpen reads from this SAME prm0.
//     procDoorOpen cutEnds the staff itself when the acting completes.
// ============================================================
static void dExtNpcDoors_pollWwDoorLinkStaff() {
    if (!dComIfGp_event_runCheck()) {
        return;
    }
    const char* re = dComIfGp_getEventManager().getRunEventName();
    if (re == NULL || std::strncmp(re, "DEFAULT_KNOB_DOOR_", 18) != 0) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    const int staffId = dComIfGp_evmng_getMyStaffId("Alink", player, 0);
    if (staffId == -1) {
        return;
    }
    static const char* const kWwLinkCuts[] = {
        "001n_wait", "012unequip", "005wait_turn", "035door",
    };
    const int act = dComIfGp_evmng_getMyActIdx(staffId, (DUSK_CONST char* DUSK_CONST*)kWwLinkCuts,
                                               4, FALSE, 0);
    if (act == 3) {
        if (dComIfGp_evmng_getIsAddvance(staffId)) {
            int* prm0 = dComIfGp_evmng_getMyIntegerP(staffId, "prm0");
            daAlink_c* link = (daAlink_c*)player;
            link->changeOriginalDemo();
            link->changeDemoMode(daPy_demo_c::DEMO_DOOR_OPEN_e, prm0 != NULL ? *prm0 : 0, 0, 0);
            DuskLog.info("[Doors] §379b Link 035door → DEMO_DOOR_OPEN prm0={}",
                         prm0 != NULL ? *prm0 : 0);
        }
        return;  // procDoorOpen owns the cutEnd
    }
    // Wait/unequip/turn cuts (and any unmapped name): donor default idiom —
    // cutEnd each frame; the knob staff owns Link's placement.
    dComIfGp_evmng_cutEnd(staffId);
}

void dExtNpcDoors_poll() {
    dExtNpcDoors_pollWwDoorLinkStaff();
    dExtNpcDoors_pollArrival();
    if (dExtNpcMount_bgWarpBusy()) {
        return;
    }
    ensureLoaded();
    if (s_doors.empty()) {
        return;
    }
    // №68: never walk exit knobs / attention while a room-lane teardown is live.
    const char* lastBusy = dExtNpcMount_lastBgProc();
    const int unloadRoom =
        lastBusy != NULL ? dExtNpcMount_roomLaneHostRoom(lastBusy) : -1;
    if (unloadRoom >= 0 && dExtNpcMount_isRoomLaneUnloading(unloadRoom)) {
        return;
    }

    dExtNpcManifest island{};
    if (!dExtNpcMount_lookup("EXT_BG0", &island) || !island.hasHostPos || !island.hasAnchor) {
        return;
    }

    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    const bool onIslandHost =
        stage != NULL && std::strcmp(stage, island.hostStage) == 0;
    // №90/№107: WW host stages (R_DL* / F_DL*) must poll exit knobs — not F_SP115-only.
    const bool onWwHost = dExtWwSave_isWwHostStage(stage);
    if (!onIslandHost && !onWwHost) {
        return;
    }

    // №53-D: heal key scramble + arm attention until 1:1 (island host only).
    if (onIslandHost && s_knobsSpawned) {
        reconcileOutdoorKnobs();
    }

    const char* last = dExtNpcMount_lastBgProc();
    const bool onIsland = last != NULL && std::strcmp(last, "EXT_BG0") == 0;
    // №94 self-heal: latch said doors spawned but none alive after a soft reload.
    if (onIslandHost && onIsland && dExtNpcDoors_knobsLatched()) {
        const int want = dExtNpcDoors_wantOutdoorKnobCount();
        const int live = dExtNpcDoors_countLiveOutdoorKnobs();
        if (want > 0 && live == 0) {
            DuskLog.warn(
                "[Doors] №94 self-heal outdoor knobs — want={} live=0 gen={} → respawn", want,
                s_knobsSpawnedGen);
            s_knobsSpawned = false;
            s_knobsSpawnedGen = 0;
            dExtNpcDoors_spawnKnobs(island);
        }
    }
    if (!onIsland && last != NULL) {
        spawnExitKnobIfNeeded(last);
        DoorDef* ed = findDoorByEnterProc(last);
        if (ed != NULL) {
            reconcileExitKnob(*ed);
        }
        // leave-shell AABB is island/pinned geometry — skip on dedicated WW hosts.
        if (onIslandHost) {
            leaveShellGuard(island, player);
        }
        roomLaneStrandGuard(island, player);
    } else {
        s_strandBadFrames = 0;
    }

    if (tickPending(island)) {
        if (!s_pending.active) {
            s_cooldown = 90;
        }
        return;
    }

    if (s_cooldown > 0) {
        --s_cooldown;
        return;
    }

    // №56: walkthrough=1 — crossing radius warps (no A), both directions.
    if (onIsland) {
        for (DoorDef& d : s_doors) {
            if (!d.walkthrough || d.enterProc[0] == '\0') {
                continue;
            }
            const cXyz center = wwToHost(island, d.wwPos);
            const bool in = nearXZ(player->current.pos, center, d.radius);
            if (in && !d.wasInEnter) {
                if (queueEnter(d, island, player, (player->current.pos - center).absXZ(),
                               d.name, /*openAlreadyDone=*/false)) {
                    for (DoorDef& e : s_doors) {
                        e.wasInEnter = true;
                        e.wasInExit = true;
                    }
                    return;
                }
            }
            d.wasInEnter = in;
        }
    } else if (last != NULL) {
        for (DoorDef& d : s_doors) {
            if (!d.walkthrough || d.enterProc[0] == '\0' ||
                std::strcmp(last, d.enterProc) != 0) {
                continue;
            }
            dExtNpcManifest dest{};
            if (!dExtNpcMount_lookup(d.enterProc, &dest) || !dest.isBg) {
                continue;
            }
            const cXyz exitCenter =
                dest.hostPos + (dest.hasExitDoorRel ? dest.exitDoorRel : dest.spawnRel);
            const bool in = nearXZ(player->current.pos, exitCenter, d.exitRadius);
            if (in && !d.wasInExit) {
                if (queueExit(d, island, player, /*openAlreadyDone=*/false)) {
                    for (DoorDef& e : s_doors) {
                        e.wasInEnter = true;
                        e.wasInExit = true;
                    }
                    return;
                }
            }
            d.wasInExit = in;
        }
    }

    // Fallback TrigA path (native OPEN goes through doorCheck → tryNativeWarp).
    if (!aPressedNearDoor()) {
        return;
    }

    if (onIsland) {
        f32 distXZ = -1.0f;
        dExtNpcMount_c* knob = dExtNpcMount_facedDoorAttention(
            player->current.pos, player->shape_angle.y, 300.0f, &distXZ);
        if (knob == NULL) {
            DuskLog.warn("[Doors] A-press — no doorAttention prop within 300u — refuse");
            return;
        }
        if (tryWarpFromKnob(knob, player, distXZ, /*openAlreadyDone=*/false)) {
            for (DoorDef& e : s_doors) {
                e.wasInEnter = true;
                e.wasInExit = true;
            }
        }
        return;
    }

    // Interior fallback: faced exit prop, else near exit_door_rel / spawn_rel.
    f32 distXZ = -1.0f;
    dExtNpcMount_c* knob = dExtNpcMount_facedDoorAttention(
        player->current.pos, player->shape_angle.y, 300.0f, &distXZ);
    if (knob != NULL && tryWarpFromKnob(knob, player, distXZ, /*openAlreadyDone=*/false)) {
        for (DoorDef& e : s_doors) {
            e.wasInEnter = true;
            e.wasInExit = true;
        }
        return;
    }
    for (DoorDef& d : s_doors) {
        if (d.enterProc[0] == '\0' || last == NULL || std::strcmp(last, d.enterProc) != 0) {
            continue;
        }
        dExtNpcManifest dest{};
        if (!dExtNpcMount_lookup(d.enterProc, &dest)) {
            continue;
        }
        const cXyz exitCenter =
            dest.hostPos + (dest.hasExitDoorRel ? dest.exitDoorRel : dest.spawnRel);
        if (!nearXZ(player->current.pos, exitCenter, d.exitRadius)) {
            continue;
        }
#if DUSK_WW_KNOB00_NATIVE
        // ============================================================
        // §397 (log 085918 DEADLOCK): this proximity leg fired queueExit
        // on the same A-press that ordered the native B_OPEN event — the
        // pending stage change blocked the event's endProc (№89 rule) and
        // the running event deferred the change: stuck in the interior.
        // When the doorway's knob00 can run a native door event, the
        // event OWNS the press; the §379b fade seam does the warp. This
        // leg remains only for doorways with no native-event door.
        // ============================================================
        if (daKnob00_c* nativeDoor = findKnob00Near(exitCenter, 80.0f)) {
            if (nativeDoor->mEventIdx[7] != -1 || nativeDoor->mEventIdx[8] != -1) {
                DuskLog.info("[Doors] §397 proximity-exit stands down — native door event "
                             "owns '{}'", d.name);
                continue;
            }
        }
#endif
        if (queueExit(d, island, player, /*openAlreadyDone=*/false)) {
            for (DoorDef& e : s_doors) {
                e.wasInEnter = true;
                e.wasInExit = true;
            }
            return;
        }
    }
}

#endif  // TARGET_PC
