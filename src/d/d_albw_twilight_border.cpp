// ============================================
// NEW CODE — ALBW Port (alpha cleanup: twilight-border fallback)
// See d_albw_twilight_border.h for the design contract. Summary: spawn the
// authored twilight border wall/tag ONLY when vanilla failed to (day-layer
// load of a room whose border is authored twilight-layer-only) while the
// province is still uncleared. Strictly fallback; vanilla-neutral.
// ============================================
#include "d/d_albw_twilight_border.h"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_kankyo_data.h"
#include "d/actor/d_a_obj_twGate.h"
#include "d/actor/d_a_tag_TWgate.h"
#include "dusk/leveledit/enumerate.hpp"
#include "f_op/f_op_actor_mng.h"
#include "SSystem/SComponent/c_counter.h"
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

// Light-side wall variants exist for types 6/7/8 (twGn* -> twGt*); a wall
// respawned onto a day layer should use the light-side skin where one
// exists so it reads correctly from the cleared side.
int lightSideWallType(int i_type) {
    switch (i_type) {
    case 6: return 0;  // twGnFiro  -> twGtFiro
    case 7: return 1;  // twGnK0102 -> twGtK0102
    case 8: return 2;  // twGnK0709 -> twGtK0709
    default: return i_type;
    }
}

u32 replaceWallType(u32 i_params, int i_type) {
    return (i_params & ~(0xFFu << 4)) | ((static_cast<u32>(i_type) & 0xFFu) << 4);
}

struct SpawnKey {
    s16 mProcName;
    s8 mRoomNo;
    cXyz mPos;
};

std::vector<SpawnKey> sSpawned;
char sLastStage[10] = {};
int sLastStayRoom = -2;
int sNextCheckFrame = 0;

// Dedupe by proc + room + proximity: the same border wall is authored once
// per twilight layer variant (e.g. L13 + L14) with possibly differing type
// params — spawning both would stack two walls on the same spot.
// Mirror vanilla twilight *spread* — not the same as "province uncleared."
// dKy_F_SP121Check (d_kankyo.cpp:11220) blocks Faron twilight until Ordon
// Day 2 complete (F_0565 / saveBitLabels value 0x4510 — same gate getLayerNo
// uses for F_SP108 pre-twilight). Later provinces follow the clear chain.
// Fallback must not spawn borders pre-spread (Talo quest / fresh Faron visit).
bool isProvinceTwilightSpread(u8 i_province) {
    switch (i_province) {
    case KY_DARKLV_FARON:
        // Ordon Day 2 finished — twilight may spread into Faron province.
        return dComIfGs_isEventBit(0x4510);
    case KY_DARKLV_ELDIN:
        return dComIfGs_isDarkClearLV(KY_DARKLV_FARON);
    case KY_DARKLV_LANAYRU:
        return dComIfGs_isDarkClearLV(KY_DARKLV_ELDIN);
    case 4:  // Hyrule-field border (l_twFlagIdx twGt*/twGn* type 4)
        return dComIfGs_isDarkClearLV(KY_DARKLV_LANAYRU);
    default:
        return false;
    }
}

bool wasSpawnedNear(s16 i_proc, s8 i_room, const cXyz& i_pos) {
    constexpr f32 kDedupeDist = 500.0f;
    for (const SpawnKey& k : sSpawned) {
        if (k.mProcName != i_proc || k.mRoomNo != i_room) {
            continue;
        }
        const f32 dx = k.mPos.x - i_pos.x;
        const f32 dy = k.mPos.y - i_pos.y;
        const f32 dz = k.mPos.z - i_pos.z;
        if (dx * dx + dy * dy + dz * dz < kDedupeDist * kDedupeDist) {
            return true;
        }
    }
    return false;
}

void logBorder(const char* i_fmt, ...) {
    char path[512];
    const char* user = getenv("USERPROFILE");
    if (user != NULL && user[0] != '\0') {
        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_tw_border_debug.txt", user);
    } else {
        snprintf(path, sizeof(path), "albw_tw_border_debug.txt");
    }
    FILE* fp = fopen(path, "a");
    if (fp == NULL) {
        return;
    }
    va_list args;
    va_start(args, i_fmt);
    vfprintf(fp, i_fmt, args);
    va_end(args);
    fclose(fp);
}

void checkRoom(int i_stayRoom) {
    const dusk::leveledit::EnumerateResult res =
        dusk::leveledit::enumerate_room_actors(i_stayRoom, /*systemUse*/ true);
    if (res.actors.empty()) {
        return;
    }

    // Group verdicts: a border group is "vanilla-handled" if ANY of its
    // authored rows joined a live actor.
    bool wallAnyLive = false;
    bool tagAnyLive = false;
    for (const dusk::leveledit::PlacedActor& row : res.actors) {
        if (strcmp(row.name, "twGate") == 0 && row.live != NULL) {
            wallAnyLive = true;
        }
        if (strcmp(row.name, "TgTGate") == 0 && row.live != NULL) {
            tagAnyLive = true;
        }
    }

    for (const dusk::leveledit::PlacedActor& row : res.actors) {
        const bool isWall = strcmp(row.name, "twGate") == 0;
        const bool isTag = strcmp(row.name, "TgTGate") == 0;
        if (!isWall && !isTag) {
            continue;
        }
        if ((isWall && wallAnyLive) || (isTag && tagAnyLive)) {
            continue;  // vanilla spawned this border group — never interfere
        }

        int province;
        u32 spawnParams = row.params;
        if (isWall) {
            const int authoredType = static_cast<int>((row.params >> 4) & 0xFF);
            const int dayType = lightSideWallType(authoredType);
            province = daObjTwGate_albwProvinceForType(dayType);
            spawnParams = replaceWallType(row.params, dayType);
        } else {
            province = daTagTWGate_albwProvinceForType(static_cast<int>(row.params & 0xFF));
        }

        if (province < 0 || dComIfGs_isDarkClearLV(static_cast<u8>(province))) {
            continue;  // invalid, or province legitimately cleared — vanilla absence is correct
        }

        if (!isProvinceTwilightSpread(static_cast<u8>(province))) {
            continue;  // twilight has not reached this province yet — day-layer absence is correct
        }

        if (wasSpawnedNear(row.procname, static_cast<s8>(i_stayRoom), row.spawnPos)) {
            continue;  // already fallback-spawned this wall/tag this session
        }

        const cXyz* scaleP =
            (row.scale.x > 0.0f || row.scale.y > 0.0f || row.scale.z > 0.0f) ? &row.scale : NULL;
        fopAcM_create(row.procname, spawnParams, &row.spawnPos, i_stayRoom, &row.angle, scaleP,
                      row.argument);
        sSpawned.push_back({row.procname, static_cast<s8>(i_stayRoom), row.spawnPos});

        logBorder("f=%06d evt=fallback-spawn stage=%s room=%d name=%s proc=%d params=0x%08x "
                  "layer=%d province=%d\n",
                  g_Counter.mCounter0, dComIfGp_getStartStageName(), i_stayRoom, row.name,
                  row.procname, spawnParams, row.layer, province);
    }
}

}  // namespace

void dAlbwTwilightBorder_drive() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL) {
        return;
    }

    if (strncmp(sLastStage, stage, sizeof(sLastStage) - 1) != 0) {
        // New stage session: forget fallback state.
        strncpy(sLastStage, stage, sizeof(sLastStage) - 1);
        sLastStage[sizeof(sLastStage) - 1] = '\0';
        sSpawned.clear();
        sLastStayRoom = -2;
        sNextCheckFrame = 0;
    }

    const int stayRoom = dComIfGp_roomControl_getStayNo();
    if (stayRoom < 0) {
        return;
    }

    const int frame = g_Counter.mCounter0;
    const bool roomChanged = stayRoom != sLastStayRoom;
    if (!roomChanged && frame < sNextCheckFrame) {
        return;
    }
    sLastStayRoom = stayRoom;
    // Rooms stream in asynchronously; re-check the stay room periodically so
    // a border authored in a room that finished loading after the first
    // check is still caught. Cheap: one placement walk per 2 seconds max.
    sNextCheckFrame = frame + 60;

    checkRoom(stayRoom);
}

#endif  // TARGET_PC
