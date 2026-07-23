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
 * Also hosts d_ext_fado_door + d_ext_quick_equip + d_ext_status + Ext Status
 * menu draw (session registries) so we avoid a new CMake translation unit /
 * RelWithDebInfo reconfigure.
 */
#include "d/d_ext_mod_flags.h"
#include "d/d_ext_fado_door.h"
#include "d/d_ext_quick_equip.h"
#include "d/d_ext_status.h"
#include "d/d_menu_ext_status.h"

#if TARGET_PC

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "JSystem/J2DGraph/J2DPrint.h"
#include "JSystem/JUtility/JUTFont.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "d/d_kankyo.h"
#include "d/d_save.h"
#include "d/d_stage.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_menu_window.h"
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_graphic.h"

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

// ---------------------------------------------------------------------------
// Quick-equip socket registry (session; WW-agnostic)
// ---------------------------------------------------------------------------

namespace {

u8 s_qePageCount = dQe_kDefaultPages;
dQeSocketDesc s_qeSockets[dQe_kMaxPages][dQe_kSlotsPerPage] = {};
u16 s_qeNextBuiltinId = 0x1000;

void qeVacate(dQeSocketDesc& s) {
    std::memset(&s, 0, sizeof(s));
    s.kind = dQeKind_Empty;
    s.tpInvSlot = 0xFF;
    s.iconItemNo = 0xFF;
}

bool qeInBounds(u8 page, u8 slot) {
    return page < s_qePageCount && slot < dQe_kSlotsPerPage;
}

bool isTpToolItem(u8 item) {
    switch (item) {
    case dItemNo_BOOMERANG_e:
    case dItemNo_SPINNER_e:
    case dItemNo_IRONBALL_e:
    case dItemNo_BOW_e:
    case dItemNo_LIGHT_ARROW_e:
    case dItemNo_ARROW_LV1_e:
    case dItemNo_ARROW_LV2_e:
    case dItemNo_ARROW_LV3_e:
    case dItemNo_BOMB_ARROW_e:
    case dItemNo_HAWK_ARROW_e:
    case dItemNo_HOOKSHOT_e:
    case dItemNo_W_HOOKSHOT_e:
    case dItemNo_HVY_BOOTS_e:
    case dItemNo_COPY_ROD_e:
    case dItemNo_COPY_ROD_2_e:
    case dItemNo_KANTERA_e:
    case dItemNo_FISHING_ROD_1_e:
    case dItemNo_LURE_ROD_e:
    case dItemNo_BEE_ROD_e:
    case dItemNo_JEWEL_ROD_e:
    case dItemNo_WORM_ROD_e:
    case dItemNo_JEWEL_BEE_ROD_e:
    case dItemNo_JEWEL_WORM_ROD_e:
    case dItemNo_PACHINKO_e:
    case dItemNo_HAWK_EYE_e:
    case dItemNo_BOMB_BAG_LV1_e:
    case dItemNo_BOMB_BAG_LV2_e:
    case dItemNo_BOMB_IN_BAG_e:
    case dItemNo_NORMAL_BOMB_e:
    case dItemNo_WATER_BOMB_e:
    case dItemNo_POKE_BOMB_e:
        return true;
    default:
        return false;
    }
}

bool claimOnPage(u8 page, dQeSocketDesc desc) {
    const u8 slot = dQe_findFreeSlot(page);
    if (slot == 0xFF) {
        return false;
    }
    desc.page = page;
    desc.slot = slot;
    return dQe_claim(desc);
}

}  // namespace

bool dQe_setPageCount(u8 pages) {
    if (pages < dQe_kMinPages || pages > dQe_kMaxPages) {
        return false;
    }
    if (pages < s_qePageCount) {
        for (u8 p = pages; p < s_qePageCount; ++p) {
            for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
                qeVacate(s_qeSockets[p][s]);
            }
        }
    } else {
        for (u8 p = s_qePageCount; p < pages; ++p) {
            for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
                qeVacate(s_qeSockets[p][s]);
            }
        }
    }
    s_qePageCount = pages;
    return true;
}

u8 dQe_getPageCount() {
    return s_qePageCount;
}

bool dQe_claim(const dQeSocketDesc& desc) {
    if (!qeInBounds(desc.page, desc.slot)) {
        return false;
    }
    if (desc.id == 0 || desc.kind == dQeKind_Empty) {
        return false;
    }
    dQeSocketDesc& dst = s_qeSockets[desc.page][desc.slot];
    if (dst.id != 0) {
        return false;
    }
    dst = desc;
    return true;
}

bool dQe_clear(u8 page, u8 slot) {
    if (!qeInBounds(page, slot)) {
        return false;
    }
    qeVacate(s_qeSockets[page][slot]);
    return true;
}

bool dQe_clearById(u16 id) {
    if (id == 0) {
        return false;
    }
    bool found = false;
    for (u8 p = 0; p < s_qePageCount; ++p) {
        for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
            if (s_qeSockets[p][s].id == id) {
                qeVacate(s_qeSockets[p][s]);
                found = true;
            }
        }
    }
    return found;
}

void dQe_clearByFlag(u16 flagMask) {
    for (u8 p = 0; p < s_qePageCount; ++p) {
        for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
            if ((s_qeSockets[p][s].flags & flagMask) != 0) {
                qeVacate(s_qeSockets[p][s]);
            }
        }
    }
}

void dQe_clearAll() {
    for (u8 p = 0; p < dQe_kMaxPages; ++p) {
        for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
            qeVacate(s_qeSockets[p][s]);
        }
    }
    s_qePageCount = dQe_kDefaultPages;
    s_qeNextBuiltinId = 0x1000;
}

const dQeSocketDesc* dQe_peek(u8 page, u8 slot) {
    if (!qeInBounds(page, slot)) {
        return NULL;
    }
    return &s_qeSockets[page][slot];
}

u8 dQe_findFreeSlot(u8 page) {
    if (page >= s_qePageCount) {
        return 0xFF;
    }
    for (u8 s = 0; s < dQe_kSlotsPerPage; ++s) {
        if (s_qeSockets[page][s].id == 0) {
            return s;
        }
    }
    return 0xFF;
}

void dQe_seedTpBuiltin() {
    if (s_qePageCount < dQe_kDefaultPages) {
        dQe_setPageCount(dQe_kDefaultPages);
    }
    dQe_clearByFlag(dQeFlag_BuiltinSeed);
    s_qeNextBuiltinId = 0x1000;

    // Page 0 — vanilla wheel tools only (InvSlot_Z). Page 1+ stay free for mod
    // sockets (SwordEquip / ShieldEquip / bags / …); never seed TP Collect gear.
    for (int i = 0; i < MAX_ITEM_SLOTS; ++i) {
        const u8 invSlot = dComIfGs_getLineUpItem(i);
        if (invSlot == dItemNo_NONE_e) {
            continue;
        }
        const u8 item = dComIfGs_getItem(invSlot, false);
        if (item == dItemNo_NONE_e || !isTpToolItem(item)) {
            continue;
        }
        dQeSocketDesc desc{};
        desc.id = s_qeNextBuiltinId++;
        desc.kind = dQeKind_InvSlot_Z;
        desc.tpInvSlot = invSlot;
        desc.iconItemNo = item;
        desc.flags = dQeFlag_BuiltinSeed;
        if (!claimOnPage(0, desc)) {
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Nested bags + deep-link
// ---------------------------------------------------------------------------

namespace {

struct BagStore {
    u16 bagId;
    dQeSocketDesc children[dQe_kBagCapacity];
};

BagStore s_bags[dQe_kMaxBags] = {};

BagStore* findBag(u16 bagId) {
    if (bagId == 0) {
        return NULL;
    }
    for (u8 i = 0; i < dQe_kMaxBags; ++i) {
        if (s_bags[i].bagId == bagId) {
            return &s_bags[i];
        }
    }
    return NULL;
}

BagStore* allocBag(u16 bagId) {
    BagStore* existing = findBag(bagId);
    if (existing != NULL) {
        return existing;
    }
    for (u8 i = 0; i < dQe_kMaxBags; ++i) {
        if (s_bags[i].bagId == 0) {
            s_bags[i].bagId = bagId;
            for (u8 c = 0; c < dQe_kBagCapacity; ++c) {
                qeVacate(s_bags[i].children[c]);
            }
            return &s_bags[i];
        }
    }
    return NULL;
}

}  // namespace

bool dQe_claimBagChild(u16 bagId, u8 childSlot, const dQeSocketDesc& child) {
    if (childSlot >= dQe_kBagCapacity || child.id == 0 || child.kind == dQeKind_Empty) {
        return false;
    }
    BagStore* bag = allocBag(bagId);
    if (bag == NULL) {
        return false;
    }
    if (bag->children[childSlot].id != 0) {
        return false;
    }
    bag->children[childSlot] = child;
    bag->children[childSlot].page = 0;
    bag->children[childSlot].slot = childSlot;
    return true;
}

bool dQe_clearBag(u16 bagId) {
    BagStore* bag = findBag(bagId);
    if (bag == NULL) {
        return false;
    }
    for (u8 c = 0; c < dQe_kBagCapacity; ++c) {
        qeVacate(bag->children[c]);
    }
    bag->bagId = 0;
    return true;
}

const dQeSocketDesc* dQe_peekBagChild(u16 bagId, u8 childSlot) {
    BagStore* bag = findBag(bagId);
    if (bag == NULL || childSlot >= dQe_kBagCapacity) {
        return NULL;
    }
    return &bag->children[childSlot];
}

u8 dQe_countBagOccupied(u16 bagId) {
    BagStore* bag = findBag(bagId);
    if (bag == NULL) {
        return 0;
    }
    u8 n = 0;
    for (u8 c = 0; c < dQe_kBagCapacity; ++c) {
        if (bag->children[c].id != 0) {
            ++n;
        }
    }
    return n;
}

bool dQe_deepLinkAssignZ(u8 tpInvSlot, u8 iconItemNo, u16 opaqueId) {
    if (tpInvSlot == 0xFF || opaqueId == 0) {
        return false;
    }
    dQe_clearById(opaqueId);
    dQeSocketDesc desc{};
    desc.id = opaqueId;
    desc.kind = dQeKind_ZSelect;
    desc.tpInvSlot = tpInvSlot;
    desc.iconItemNo = iconItemNo;
    desc.flags = dQeFlag_ModClaim;
    if (!claimOnPage(0, desc)) {
        return false;
    }
    dComIfGs_setSelectItemIndex(SELECT_ITEM_DOWN, tpInvSlot);
    return true;
}

// ---------------------------------------------------------------------------
// Ext Status registry
// ---------------------------------------------------------------------------

namespace {

dExtStatusRow s_extRows[dExtStatus_kMaxRows] = {};

void vacateExt(dExtStatusRow& r) {
    std::memset(&r, 0, sizeof(r));
    r.iconItemNo = 0xFF;
    r.tpInvSlot = 0xFF;
}

}  // namespace

bool dExtStatus_claim(const dExtStatusRow& row) {
    if (row.id == 0 || row.kind == dExtStatusKind_Empty || row.tab >= dExtStatus_kTabCount) {
        return false;
    }
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id == row.id) {
            s_extRows[i] = row;
            return true;
        }
    }
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id == 0) {
            s_extRows[i] = row;
            return true;
        }
    }
    return false;
}

bool dExtStatus_clearById(u16 id) {
    if (id == 0) {
        return false;
    }
    bool found = false;
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id == id) {
            vacateExt(s_extRows[i]);
            found = true;
        }
    }
    return found;
}

void dExtStatus_clearByFlag(u16 flagMask) {
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if ((s_extRows[i].flags & flagMask) != 0) {
            vacateExt(s_extRows[i]);
        }
    }
}

void dExtStatus_clearAll() {
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        vacateExt(s_extRows[i]);
    }
}

u8 dExtStatus_countTab(dExtStatusTab tab) {
    u8 n = 0;
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id != 0 && s_extRows[i].tab == tab) {
            ++n;
        }
    }
    return n;
}

const dExtStatusRow* dExtStatus_peekTab(dExtStatusTab tab, u8 index) {
    u8 n = 0;
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id == 0 || s_extRows[i].tab != tab) {
            continue;
        }
        if (n == index) {
            return &s_extRows[i];
        }
        ++n;
    }
    return NULL;
}

void dExtStatus_seedDebug() {
    dExtStatus_clearByFlag(dExtStatusFlag_DebugSeed);
    static const char* kTabNames[] = {"Tools", "Quest", "Atlas"};
    for (u8 t = 0; t < dExtStatus_kTabCount; ++t) {
        dExtStatusRow row{};
        row.id = static_cast<u16>(0xE100 + t);
        row.tab = static_cast<dExtStatusTab>(t);
        row.kind = (t == dExtStatusTab_Atlas) ? dExtStatusKind_Chart : dExtStatusKind_Passive;
        row.iconItemNo = 0xFF;
        row.tpInvSlot = 0xFF;
        row.flags = dExtStatusFlag_DebugSeed;
        std::snprintf(row.label, sizeof(row.label), "%s (empty)", kTabNames[t]);
        dExtStatus_claim(row);
    }
}

bool dExtStatus_tryDeepLinkZ(u16 rowId) {
    for (u8 i = 0; i < dExtStatus_kMaxRows; ++i) {
        if (s_extRows[i].id != rowId) {
            continue;
        }
        if (s_extRows[i].kind != dExtStatusKind_Usable || s_extRows[i].tpInvSlot == 0xFF) {
            return false;
        }
        return dQe_deepLinkAssignZ(s_extRows[i].tpInvSlot, s_extRows[i].iconItemNo, rowId);
    }
    return false;
}

// ---------------------------------------------------------------------------
// Mod claims.ini loader (WW-agnostic; labels from data)
// ---------------------------------------------------------------------------

namespace {

dQeKind parseQeKind(const std::string& s) {
    if (s == "inv_z") return dQeKind_InvSlot_Z;
    if (s == "sword") return dQeKind_SwordEquip;
    if (s == "shield") return dQeKind_ShieldEquip;
    if (s == "zsel") return dQeKind_ZSelect;
    if (s == "custom") return dQeKind_Custom;
    if (s == "bag") return dQeKind_Bag;
    return dQeKind_Empty;
}

dExtStatusTab parseEsTab(const std::string& s) {
    if (s == "tools") return dExtStatusTab_Tools;
    if (s == "quest") return dExtStatusTab_Quest;
    if (s == "atlas") return dExtStatusTab_Atlas;
    return dExtStatusTab_Tools;
}

dExtStatusKind parseEsKind(const std::string& s) {
    if (s == "passive") return dExtStatusKind_Passive;
    if (s == "usable") return dExtStatusKind_Usable;
    if (s == "bag") return dExtStatusKind_Bag;
    if (s == "chart") return dExtStatusKind_Chart;
    if (s == "mark") return dExtStatusKind_Mark;
    return dExtStatusKind_Empty;
}

void loadClaimsFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        char cmd[16] = {};
        char a[32] = {};
        char b[32] = {};
        char c[32] = {};
        char d[64] = {};
        unsigned id = 0;
        unsigned icon = 0xFF;
        unsigned tp = 0xFF;
        unsigned page = 0;
        unsigned slot = 0;
        unsigned child = 0;
        // qe <page|auto> <slot|auto> <kind> <id> <icon> <tpInv>
        if (std::sscanf(line.c_str(), "qe %31s %31s %31s %x %x %x", a, b, c, &id, &icon, &tp) >= 4) {
            dQeSocketDesc desc{};
            desc.id = static_cast<u16>(id);
            desc.kind = parseQeKind(c);
            desc.iconItemNo = static_cast<u8>(icon);
            desc.tpInvSlot = static_cast<u8>(tp);
            desc.flags = dQeFlag_ModClaim;
            if (std::strcmp(a, "auto") == 0) {
                claimOnPage(0, desc);
            } else {
                page = static_cast<unsigned>(std::strtoul(a, nullptr, 0));
                if (std::strcmp(b, "auto") == 0) {
                    desc.page = static_cast<u8>(page);
                    claimOnPage(desc.page, desc);
                } else {
                    slot = static_cast<unsigned>(std::strtoul(b, nullptr, 0));
                    desc.page = static_cast<u8>(page);
                    desc.slot = static_cast<u8>(slot);
                    dQe_claim(desc);
                }
            }
            continue;
        }
        // bag <bagId> <childSlot> <kind> <id> <icon> <tpInv>
        {
            unsigned bagId = 0;
            unsigned childId = 0;
            if (std::sscanf(line.c_str(), "bag %x %u %31s %x %x %x", &bagId, &child, c, &childId,
                            &icon, &tp) >= 4)
            {
                dQeSocketDesc childDesc{};
                childDesc.id = static_cast<u16>(childId);
                childDesc.kind = parseQeKind(c);
                childDesc.iconItemNo = static_cast<u8>(icon);
                childDesc.tpInvSlot = static_cast<u8>(tp);
                childDesc.flags = dQeFlag_ModClaim;
                dQe_claimBagChild(static_cast<u16>(bagId), static_cast<u8>(child), childDesc);
                continue;
            }
        }
        // es <tab> <kind> <id> <icon> <label...>
        if (std::sscanf(line.c_str(), "es %31s %31s %x %x %63[^\r\n]", a, b, &id, &icon, d) >= 3) {
            dExtStatusRow row{};
            row.id = static_cast<u16>(id);
            row.tab = parseEsTab(a);
            row.kind = parseEsKind(b);
            row.iconItemNo = static_cast<u8>(icon);
            row.tpInvSlot = 0xFF;
            row.flags = dExtStatusFlag_ModClaim;
            if (d[0] != '\0') {
                std::snprintf(row.label, sizeof(row.label), "%s", d);
            }
            dExtStatus_claim(row);
        }
    }
}

}  // namespace

void dExtInv_rescanClaims() {
    dQe_clearByFlag(dQeFlag_ModClaim);
    dExtStatus_clearByFlag(dExtStatusFlag_ModClaim);
    for (u8 i = 0; i < dQe_kMaxBags; ++i) {
        if (s_bags[i].bagId != 0) {
            // Clear mod bag children by wiping bags that only hold mod claims.
            bool anyBuiltin = false;
            for (u8 c = 0; c < dQe_kBagCapacity; ++c) {
                if (s_bags[i].children[c].id != 0 &&
                    (s_bags[i].children[c].flags & dQeFlag_ModClaim) == 0)
                {
                    anyBuiltin = true;
                }
            }
            if (!anyBuiltin) {
                dQe_clearBag(s_bags[i].bagId);
            }
        }
    }

    const auto folders = dusk::custom_assets::list_folders();
    for (const auto& folder : folders) {
        if (!dusk::custom_assets::is_folder_enabled(folder.c_str())) {
            continue;
        }
        const std::filesystem::path claims =
            dusk::ConfigPath / "model_replacements" / folder / "ext_inv" / "claims.ini";
        std::error_code ec;
        if (std::filesystem::exists(claims, ec)) {
            loadClaimsFile(claims);
            DuskLog.info("[ext_inv] loaded claims from '{}'", folder);
        }
    }
    // Empty-shell chrome when no mod rows claimed Quest/Atlas/Tools yet.
    if (dExtStatus_countTab(dExtStatusTab_Tools) == 0 &&
        dExtStatus_countTab(dExtStatusTab_Quest) == 0 &&
        dExtStatus_countTab(dExtStatusTab_Atlas) == 0)
    {
        dExtStatus_seedDebug();
    }
}

// ---------------------------------------------------------------------------
// Ext Status menu (Collect sibling shell)
// ---------------------------------------------------------------------------

dMenu_ExtStatus_c::dMenu_ExtStatus_c(STControl* stick, CSTControl* cstick)
    : mpStick(stick),
      mpCStick(cstick),
      mTab(dExtStatusTab_Tools),
      mCursor(0),
      mCollectHandoff(-1),
      mWantsClose(false) {}

void dMenu_ExtStatus_c::_create() {
    mTab = dExtStatusTab_Tools;
    mCursor = 0;
    mCollectHandoff = -1;
    mWantsClose = false;
}

void dMenu_ExtStatus_c::_delete() {}

void dMenu_ExtStatus_c::setTab(dExtStatusTab tab) {
    if (tab >= dExtStatus_kTabCount) {
        return;
    }
    mTab = tab;
    mCursor = 0;
}

void dMenu_ExtStatus_c::_move() {
    mCollectHandoff = -1;
    if (dMw_B_TRIGGER() || dMw_START_TRIGGER()) {
        mWantsClose = true;
        return;
    }
    if (dMw_RIGHT_TRIGGER()) {
        if (mTab >= dExtStatusTab_Atlas) {
            mCollectHandoff = 1;
        } else {
            setTab(static_cast<dExtStatusTab>(mTab + 1));
        }
        return;
    }
    if (dMw_LEFT_TRIGGER()) {
        if (mTab <= dExtStatusTab_Tools) {
            mCollectHandoff = 0;
        } else {
            setTab(static_cast<dExtStatusTab>(mTab - 1));
        }
        return;
    }
    const u8 count = dExtStatus_countTab(mTab);
    if (dMw_DOWN_TRIGGER() && count > 0) {
        mCursor = static_cast<u8>((mCursor + 1) % count);
    } else if (dMw_UP_TRIGGER() && count > 0) {
        mCursor = static_cast<u8>((mCursor + count - 1) % count);
    }
    if (dMw_A_TRIGGER() && mTab == dExtStatusTab_Tools && count > 0) {
        const dExtStatusRow* row = dExtStatus_peekTab(mTab, mCursor);
        if (row != NULL) {
            dExtStatus_tryDeepLinkZ(row->id);
        }
    }
}

void dMenu_ExtStatus_c::draw() {
    J2DGrafContext* graf = dComIfGp_getCurrentGrafPort();
    if (graf == NULL) {
        return;
    }
    graf->setup2D();
    JUTFont* font = mDoExt_getMesgFont();
    if (font == NULL) {
        return;
    }
    const JUtility::TColor white(255, 255, 255, 255);
    const JUtility::TColor dim(180, 180, 180, 255);
    J2DPrint print(font, 0.0f, 20.0f, white, white, JUtility::TColor(0, 0, 0, 0), white);
    print.setFontSize(18.0f, 18.0f);
    font->pushDrawState();
    print.initiate();

    print.print(40.0f, 40.0f, 255, "STATUS");
    static const char* kTabs[] = {"Tools", "Quest", "Atlas"};
    f32 tx = 40.0f;
    for (u8 t = 0; t < dExtStatus_kTabCount; ++t) {
        const bool on = (t == mTab);
        print.setCharColor(on ? white : dim);
        print.print(tx, 70.0f, 255, "%s%s", on ? "[" : " ", kTabs[t]);
        if (on) {
            print.print(tx + 8.0f * static_cast<f32>(std::strlen(kTabs[t])) + 8.0f, 70.0f, 255, "]");
        }
        tx += 100.0f;
    }
    print.setCharColor(white);

    const u8 count = dExtStatus_countTab(mTab);
    if (count == 0) {
        print.print(40.0f, 120.0f, 255, "(empty)");
    } else {
        for (u8 i = 0; i < count && i < 12; ++i) {
            const dExtStatusRow* row = dExtStatus_peekTab(mTab, i);
            if (row == NULL) {
                continue;
            }
            const char* mark = (i == mCursor) ? "> " : "  ";
            const char* lab = row->label[0] != '\0' ? row->label : "(row)";
            print.print(40.0f, 110.0f + 22.0f * static_cast<f32>(i), 255, "%s%s", mark, lab);
        }
    }
    print.print(40.0f, 400.0f, 255, "L/R tabs  A assign  B/START close");
    font->popDrawState();
}

#endif  // TARGET_PC
