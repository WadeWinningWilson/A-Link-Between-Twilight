/**
 * d_ext_mod_flags.cpp — Phase O2 R-O2a per-mod save-scoped flag store.
 *
 * Layout in dSv_reserve_c bytes [1..79] (v2):
 *   [0] magic 0xEF
 *   [1] version 2
 *   [2..78] bit array (77 bytes = 616 bits)
 *   bit index = FNV1a32(modFolder + '\0' + key) % 616
 *
 * v1 (u32 entry list, max 18) is migrated in-place on first access.
 *
 * Also hosts d_ext_fado_door (session warp command + unlock helpers) so we
 * avoid a new CMake translation unit / RelWithDebInfo reconfigure.
 */
#include "d/d_ext_mod_flags.h"
#include "d/d_ext_fado_door.h"

#if TARGET_PC

#include <cstring>

#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_save.h"
#include "d/d_stage.h"
#include "f_op/f_op_actor_mng.h"
#include "dusk/logging.h"

namespace {

constexpr u8 kMagic = 0xEF;
constexpr u8 kVersion = 2;
constexpr u8 kVersionV1 = 1;
constexpr int kHeader = 2;
constexpr int kBitBytes = dSv_reserve_c::EXT_MOD_FLAGS_SIZE - kHeader;  // 77
constexpr int kBitCount = kBitBytes * 8;                                // 616

u32 fnv1a32(const char* s) {
    u32 h = 2166136261u;
    if (s == NULL) {
        return 0;
    }
    for (const u8* p = reinterpret_cast<const u8*>(s); *p != 0; ++p) {
        h ^= *p;
        h *= 16777619u;
    }
    return h;
}

u32 bitIndex(const char* modFolder, const char* key) {
    u32 h = 2166136261u;
    for (const u8* p = reinterpret_cast<const u8*>(modFolder); *p != 0; ++p) {
        h ^= *p;
        h *= 16777619u;
    }
    h ^= 0;
    h *= 16777619u;
    for (const u8* p = reinterpret_cast<const u8*>(key); *p != 0; ++p) {
        h ^= *p;
        h *= 16777619u;
    }
    return h % static_cast<u32>(kBitCount);
}

u8* blob() {
    dSv_save_c* save = dComIfGs_getSaveData();
    if (save == NULL) {
        return NULL;
    }
    return save->getReserve().getExtModFlags();
}

u32 readU32(const u8* p) {
    return (static_cast<u32>(p[0])) | (static_cast<u32>(p[1]) << 8) |
           (static_cast<u32>(p[2]) << 16) | (static_cast<u32>(p[3]) << 24);
}

void migrateV1ToV2(u8* b) {
    // v1: [0]=magic [1]=1 [2]=count [3]=pad [4..]=u32 (modHash16<<16)|keyHash16
    // We cannot recover original strings — keep bits unset and reset.
    // Best-effort: abandon v1 bit-identity (hashes differ). Log once.
    const int n = b[2];
    DuskLog.info("[ExtModFlags] migrating v1 ({} entries) → v2 bitfield; flags reset", n);
    std::memset(b, 0, dSv_reserve_c::EXT_MOD_FLAGS_SIZE);
    b[0] = kMagic;
    b[1] = kVersion;
}

void ensureInit(u8* b) {
    if (b[0] == kMagic && b[1] == kVersion) {
        return;
    }
    if (b[0] == kMagic && b[1] == kVersionV1) {
        migrateV1ToV2(b);
        return;
    }
    std::memset(b, 0, dSv_reserve_c::EXT_MOD_FLAGS_SIZE);
    b[0] = kMagic;
    b[1] = kVersion;
}

bool getBit(const u8* b, u32 idx) {
    return (b[kHeader + idx / 8] & (1u << (idx % 8))) != 0;
}

void setBit(u8* b, u32 idx, bool value) {
    u8& byte = b[kHeader + idx / 8];
    const u8 mask = static_cast<u8>(1u << (idx % 8));
    if (value) {
        byte = static_cast<u8>(byte | mask);
    } else {
        byte = static_cast<u8>(byte & static_cast<u8>(~mask));
    }
}

}  // namespace

bool dExtModFlags_get(const char* modFolder, const char* key) {
    if (modFolder == NULL || modFolder[0] == '\0' || key == NULL || key[0] == '\0') {
        return false;
    }
    u8* b = blob();
    if (b == NULL) {
        return false;
    }
    ensureInit(b);
    return getBit(b, bitIndex(modFolder, key));
}

void dExtModFlags_set(const char* modFolder, const char* key, bool value) {
    if (modFolder == NULL || modFolder[0] == '\0' || key == NULL || key[0] == '\0') {
        return;
    }
    u8* b = blob();
    if (b == NULL) {
        DuskLog.warn("[ExtModFlags] set ignored — no save data");
        return;
    }
    ensureInit(b);
    const u32 idx = bitIndex(modFolder, key);
    const bool prev = getBit(b, idx);
    if (prev == value) {
        return;
    }
    setBit(b, idx, value);
    DuskLog.debug("[ExtModFlags] {} '{}':'{}' (bit {})", value ? "set" : "clear", modFolder, key,
                  idx);
}

void dExtModFlags_clearMod(const char* modFolder) {
    // v2 bits are mixed across mods — cannot clear by mod without a side table.
    // Full wipe of the blob (debug / mod uninstall).
    (void)modFolder;
    u8* b = blob();
    if (b == NULL) {
        return;
    }
    std::memset(b, 0, dSv_reserve_c::EXT_MOD_FLAGS_SIZE);
    b[0] = kMagic;
    b[1] = kVersion;
    DuskLog.info("[ExtModFlags] cleared all mod flags in save reserve");
}

// ---------------------------------------------------------------------------
// Fado door (F_SP103 exit 7) — unlock + session warp command
// ---------------------------------------------------------------------------

namespace {

constexpr const char* kFadoModFolder = "dusklight";
constexpr const char* kFadoUnlockKey = "fado.door.unlock";
constexpr const char* kFadoStage = "F_SP103";
constexpr u8 kFadoExitId = 7;

struct FadoDoorWarpCommand {
    char stage[8];
    s16 room;
    s16 spawn;
    s16 layer;
    s16 facing;
    bool valid;
};

FadoDoorWarpCommand s_fadoWarpCmd = {};

bool isFadoExit(int exitId, s8 roomNo) {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, kFadoStage) != 0) {
        return false;
    }
    if (exitId != static_cast<int>(kFadoExitId)) {
        return false;
    }
    // Retail door sits in room 0; allow -1 (stage SCLS) as well.
    return roomNo == 0 || roomNo == -1;
}

}  // namespace

bool dFadoDoor_isTargetKnob(fopAc_ac_c* actor) {
    if (actor == NULL) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, kFadoStage) != 0) {
        return false;
    }
    return fopAcM_GetParamBit(actor, 25, 6) == kFadoExitId;
}

bool dFadoDoor_isUnlocked() {
    return dExtModFlags_get(kFadoModFolder, kFadoUnlockKey);
}

void dFadoDoor_setUnlocked(bool unlocked) {
    dExtModFlags_set(kFadoModFolder, kFadoUnlockKey, unlocked);
    DuskLog.info("[FadoDoor] unlock {}", unlocked ? "ON" : "OFF");
}

void dFadoDoor_setWarpCommand(const char* stage, s16 room, s16 spawn, s16 layer, s16 facing) {
    if (stage == NULL || stage[0] == '\0') {
        dFadoDoor_clearWarpCommand();
        return;
    }
    std::memset(&s_fadoWarpCmd, 0, sizeof(s_fadoWarpCmd));
    std::strncpy(s_fadoWarpCmd.stage, stage, sizeof(s_fadoWarpCmd.stage) - 1);
    s_fadoWarpCmd.room = room;
    s_fadoWarpCmd.spawn = spawn;
    s_fadoWarpCmd.layer = layer;
    s_fadoWarpCmd.facing = facing;
    s_fadoWarpCmd.valid = true;
    DuskLog.info("[FadoDoor] warp command → {} room {} spawn {} layer {}", s_fadoWarpCmd.stage,
                 s_fadoWarpCmd.room, s_fadoWarpCmd.spawn, s_fadoWarpCmd.layer);
}

void dFadoDoor_clearWarpCommand() {
    if (s_fadoWarpCmd.valid) {
        DuskLog.info("[FadoDoor] warp command cleared");
    }
    std::memset(&s_fadoWarpCmd, 0, sizeof(s_fadoWarpCmd));
}

bool dFadoDoor_hasWarpCommand() {
    return s_fadoWarpCmd.valid && s_fadoWarpCmd.stage[0] != '\0';
}

int dFadoDoor_tryInterceptChangeScene(int exitId, f32 speed, u32 mode, s8 roomNo, s16 angle,
                                      int layerOverride) {
    if (!isFadoExit(exitId, roomNo)) {
        return 0;
    }
    if (!dFadoDoor_isUnlocked()) {
        // Still locked — let retail refuse / msg path handle open; if somehow
        // reached, block the missing R_SP01 room 3 load.
        DuskLog.warn("[FadoDoor] scene change blocked — door locked");
        return -1;
    }
    if (!dFadoDoor_hasWarpCommand()) {
        DuskLog.warn("[FadoDoor] unlocked but no warp command — refusing SCLS[7]");
        return -1;
    }

    s32 wipe = 0;
    s32 wipe_time = 0;
    s32 layer = s_fadoWarpCmd.layer;
    stage_scls_info_dummy_class* scls = NULL;
    if (roomNo == -1) {
        scls = dComIfGp_getStageSclsInfo();
    } else if (roomNo >= 0 && roomNo < 64) {
        dStage_roomDt_c* room = dComIfGp_roomControl_getStatusRoomDt(roomNo);
        if (room != NULL) {
            scls = room->getSclsInfo();
        }
    }
    if (scls != NULL && exitId >= 0 && exitId < scls->num) {
        stage_scls_info_class* info = &scls->m_entries[exitId];
        wipe = dStage_sclsInfo_getWipe(info);
        wipe_time = dStage_sclsInfo_getWipeTime(info);
        if (layer < 0) {
            layer = dStage_sclsInfo_getSceneLayer(info);
            if (layer >= 15) {
                layer = -1;
            }
        }
        int timeH = dStage_sclsInfo_getTimeH(info);
        if (timeH < 31) {
            dKy_set_nexttime(15.0f * timeH);
        }
    }
    if (layer < 0 && layerOverride != -1) {
        layer = layerOverride;
    }

    const s16 useAngle = (s_fadoWarpCmd.facing != -1) ? s_fadoWarpCmd.facing : angle;
    dComIfGp_setNextStage(s_fadoWarpCmd.stage, s_fadoWarpCmd.spawn,
                          static_cast<s8>(s_fadoWarpCmd.room), static_cast<s8>(layer), speed, mode,
                          1, wipe == 15 ? 0 : static_cast<s8>(wipe), useAngle, 1, wipe_time);
    DuskLog.info("[FadoDoor] transit → {} r{} spawn{}", s_fadoWarpCmd.stage, s_fadoWarpCmd.room,
                 s_fadoWarpCmd.spawn);
    return 1;
}

#endif  // TARGET_PC
