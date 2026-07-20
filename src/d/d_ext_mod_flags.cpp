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
 */
#include "d/d_ext_mod_flags.h"

#if TARGET_PC

#include <cstring>

#include "d/d_com_inf_game.h"
#include "d/d_save.h"
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

#endif  // TARGET_PC
