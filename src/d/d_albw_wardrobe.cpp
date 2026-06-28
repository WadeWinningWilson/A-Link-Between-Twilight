// ============================================
// NEW CODE — ALBW Port (Quick Swap wardrobe / Postman storage)
// See include/d/d_albw_wardrobe.h and Quick-Resistance Work.md.
// ============================================
#include "d/d_albw_wardrobe.h"

#if TARGET_PC

#include "d/d_albw_sumo_test.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "d/d_meter2.h"
#include "d/d_meter2_info.h"
#include "d/d_save.h"
#include "d/actor/d_a_player.h"
#include "dusk/action_bindings.h"

#include <cstdio>
#include <cstring>

namespace {

// Postman storage save bits — repurposes 697-699 (evict limiter dropped).  Skips
// 700 (sumo worn), 701-702 (F_0701B/F_0702B).  See Quick-Resistance Work.md.
constexpr int kStoreWoodSword    = 697;
constexpr int kStoreOrdonSword   = 698;
constexpr int kStoreMasterSword  = 699;
constexpr int kStoreLightSword   = 703;
constexpr int kStoreOrdonShield  = 704;
constexpr int kStoreWoodenShield = 705;
constexpr int kStoreHylianShield = 706;
constexpr int kStoreSumoOutfit   = 707;
constexpr int kStoreOrdonOutfit  = 708;
constexpr int kStoreHerosOutfit  = 709;
constexpr int kStoreZoraOutfit   = 710;
constexpr int kStoreMagicOutfit  = 711;
constexpr int kStoreDeityOutfit  = 712;

constexpr f32 kSwordPenaltyPerExtra   = 0.10f;
constexpr f32 kShieldPenaltyPerExtra  = 0.15f;
constexpr f32 kOutfitStackSumo        = 0.05f;
constexpr f32 kOutfitStackOrdon       = 0.10f;
constexpr f32 kOutfitStackHeros       = 0.25f;
constexpr f32 kOutfitStackMagicPowered = 0.30f;
constexpr f32 kOutfitStackMagicDrained = 0.50f;

static const u8 kSwordItemNos[] = {
    (u8)dItemNo_WOOD_STICK_e,
    (u8)dItemNo_SWORD_e,
    (u8)dItemNo_MASTER_SWORD_e,
    (u8)dItemNo_LIGHT_SWORD_e,
};

static const u8 kShieldItemNos[] = {
    (u8)dItemNo_WOOD_SHIELD_e,
    (u8)dItemNo_SHIELD_e,
    (u8)dItemNo_HYLIA_SHIELD_e,
};

int storageBitForItemNo(u8 itemNo) {
    switch (itemNo) {
    case (u8)dItemNo_WOOD_STICK_e:    return kStoreWoodSword;
    case (u8)dItemNo_SWORD_e:         return kStoreOrdonSword;
    case (u8)dItemNo_MASTER_SWORD_e:  return kStoreMasterSword;
    case (u8)dItemNo_LIGHT_SWORD_e:   return kStoreLightSword;
    case (u8)dItemNo_WOOD_SHIELD_e:   return kStoreOrdonShield;
    case (u8)dItemNo_SHIELD_e:        return kStoreWoodenShield;
    case (u8)dItemNo_HYLIA_SHIELD_e:  return kStoreHylianShield;
    case (u8)dItemNo_WEAR_CASUAL_e:   return kStoreOrdonOutfit;
    case (u8)dItemNo_WEAR_KOKIRI_e:   return kStoreHerosOutfit;
    case (u8)dItemNo_WEAR_ZORA_e:     return kStoreZoraOutfit;
    case (u8)dItemNo_ARMOR_e:         return kStoreMagicOutfit;
    case (u8)dItemNo_DEITY_ARMOR_e:   return kStoreDeityOutfit;
    default:                          return -1;
    }
}

int storageBitForOutfit(dAlbwOutfitKind kind) {
    switch (kind) {
    case D_ALBW_OUTFIT_SUMO:  return kStoreSumoOutfit;
    case D_ALBW_OUTFIT_ORDON: return kStoreOrdonOutfit;
    case D_ALBW_OUTFIT_HEROS: return kStoreHerosOutfit;
    case D_ALBW_OUTFIT_ZORA:  return kStoreZoraOutfit;
    case D_ALBW_OUTFIT_MAGIC: return kStoreMagicOutfit;
    case D_ALBW_OUTFIT_DEITY: return kStoreDeityOutfit;
    default:                  return -1;
    }
}

bool isStorageBitSet(int bit) {
    if (bit < 0) {
        return false;
    }
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[bit]) != 0;
}

void setStorageBit(int bit, bool on) {
    if (bit < 0) {
        return;
    }
    if (on) {
        dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[bit]);
    } else {
        dComIfGs_offEventBit(dSv_event_flag_c::saveBitLabels[bit]);
    }
}

bool swordIsOwned(u8 itemNo) {
    return dComIfGs_isItemFirstBit(itemNo) != 0;
}

bool shieldIsOwned(u8 itemNo) {
    return dMeter2_shieldIsOwned(itemNo);
}

bool outfitIsOwned(dAlbwOutfitKind kind) {
    if (dAlbwOutfit_isOwned(kind)) {
        return true;
    }
    if (kind == D_ALBW_OUTFIT_DEITY) {
        return dComIfGs_isItemFirstBit((u8)dItemNo_DEITY_ARMOR_e) != 0;
    }
    return false;
}

f32 outfitStackRate(dAlbwOutfitKind kind) {
    switch (kind) {
    case D_ALBW_OUTFIT_SUMO:  return kOutfitStackSumo;
    case D_ALBW_OUTFIT_ORDON: return kOutfitStackOrdon;
    case D_ALBW_OUTFIT_HEROS: return kOutfitStackHeros;
    case D_ALBW_OUTFIT_ZORA:  return 0.0f;  // TBD — product
    case D_ALBW_OUTFIT_MAGIC:
    case D_ALBW_OUTFIT_DEITY: {
        const bool magicEquipped =
            dAlbwOutfit_isActive(D_ALBW_OUTFIT_MAGIC) || dAlbwOutfit_isActive(D_ALBW_OUTFIT_DEITY);
        const bool drained = dMeter2_isALBWArmorDepleted();
        if (magicEquipped && drained) {
            return kOutfitStackMagicDrained;
        }
        return kOutfitStackMagicPowered;
    }
    default:
        return 0.0f;
    }
}

void copyErr(char* errOut, int errCap, const char* msg) {
    if (errOut == nullptr || errCap <= 0) {
        return;
    }
    std::snprintf(errOut, static_cast<size_t>(errCap), "%s", msg);
}

static u8 sFirstActiveSwordExcept(u8 avoid) {
    for (int i = 0; i < static_cast<int>(sizeof(kSwordItemNos) / sizeof(kSwordItemNos[0])); ++i) {
        const u8 candidate = kSwordItemNos[i];
        if (candidate != avoid && dAlbwWardrobe_isActiveSword(candidate)) {
            return candidate;
        }
    }
    return dItemNo_NONE_e;
}

static u8 sFirstActiveShieldExcept(u8 avoid) {
    for (int i = 0; i < 3; ++i) {
        const u8 candidate = kShieldItemNos[i];
        if (candidate != avoid && dAlbwWardrobe_isActiveShield(candidate)) {
            return candidate;
        }
    }
    return dItemNo_NONE_e;
}

static dAlbwOutfitKind sFirstActiveOutfitExcept(dAlbwOutfitKind avoid) {
    static constexpr dAlbwOutfitKind kOrder[] = {
        D_ALBW_OUTFIT_SUMO,
        D_ALBW_OUTFIT_ORDON,
        D_ALBW_OUTFIT_HEROS,
        D_ALBW_OUTFIT_ZORA,
        D_ALBW_OUTFIT_MAGIC,
        D_ALBW_OUTFIT_DEITY,
    };
    for (dAlbwOutfitKind kind : kOrder) {
        if (kind != avoid && dAlbwWardrobe_isActiveOutfit(kind)) {
            return kind;
        }
    }
    return D_ALBW_OUTFIT_COUNT;
}

static void swapEquippedSwordIfStored(u8 storedItemNo) {
    if (dComIfGs_getSelectEquipSword() != storedItemNo) {
        return;
    }
    const u8 next = sFirstActiveSwordExcept(storedItemNo);
    if (next != dItemNo_NONE_e) {
        dMeter2Info_setSword(next, false);
    }
}

static void swapEquippedShieldIfStored(u8 storedItemNo) {
    if (dComIfGs_getSelectEquipShield() != storedItemNo) {
        return;
    }
    const u8 next = sFirstActiveShieldExcept(storedItemNo);
    if (next != dItemNo_NONE_e) {
        dMeter2_equipOwnedShield(next);
    } else {
        dMeter2_applyEquippedShield(dItemNo_NONE_e);
    }
}

static void swapEquippedOutfitIfStored(dAlbwOutfitKind storedKind) {
    if (!dAlbwOutfit_isActive(storedKind)) {
        return;
    }
    const dAlbwOutfitKind next = sFirstActiveOutfitExcept(storedKind);
    if (next < D_ALBW_OUTFIT_COUNT) {
        dAlbwOutfit_equip(next);
        return;
    }
    // Fallback: Ordon native clothes if still active.
    if (dAlbwWardrobe_isActiveOutfit(D_ALBW_OUTFIT_ORDON)) {
        dAlbwOutfit_equip(D_ALBW_OUTFIT_ORDON);
    }
}

}  // namespace

bool dAlbwWardrobe_isResistanceActive() {
    if (!dusk::isDpadQuickSwapEnabled()) {
        return false;
    }
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    return player != nullptr && !player->checkWolf();
}

bool dAlbwWardrobe_isStorableItemNo(u8 itemNo) {
    return storageBitForItemNo(itemNo) >= 0;
}

bool dAlbwWardrobe_isStorableOutfit(dAlbwOutfitKind kind) {
    return storageBitForOutfit(kind) >= 0;
}

bool dAlbwWardrobe_isStoredItemNo(u8 itemNo) {
    return isStorageBitSet(storageBitForItemNo(itemNo));
}

bool dAlbwWardrobe_isStoredOutfit(dAlbwOutfitKind kind) {
    return isStorageBitSet(storageBitForOutfit(kind));
}

bool dAlbwWardrobe_isActiveSword(u8 itemNo) {
    if (!swordIsOwned(itemNo)) {
        return false;
    }
    return !dAlbwWardrobe_isStoredItemNo(itemNo);
}

bool dAlbwWardrobe_isActiveShield(u8 itemNo) {
    if (!shieldIsOwned(itemNo)) {
        return false;
    }
    return !dAlbwWardrobe_isStoredItemNo(itemNo);
}

bool dAlbwWardrobe_isActiveOutfit(dAlbwOutfitKind kind) {
    if (!outfitIsOwned(kind)) {
        return false;
    }
    return !dAlbwWardrobe_isStoredOutfit(kind);
}

int dAlbwWardrobe_countActiveSwords() {
    int count = 0;
    for (u8 itemNo : kSwordItemNos) {
        if (dAlbwWardrobe_isActiveSword(itemNo)) {
            count++;
        }
    }
    return count;
}

int dAlbwWardrobe_countActiveShields() {
    int count = 0;
    for (u8 itemNo : kShieldItemNos) {
        if (dAlbwWardrobe_isActiveShield(itemNo)) {
            count++;
        }
    }
    return count;
}

int dAlbwWardrobe_countOwnedOutfitTypes() {
    int count = 0;
    for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
        if (outfitIsOwned(static_cast<dAlbwOutfitKind>(i))) {
            count++;
        }
    }
    return count;
}

int dAlbwWardrobe_countActiveOutfitTypes() {
    int count = 0;
    for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
        if (dAlbwWardrobe_isActiveOutfit(static_cast<dAlbwOutfitKind>(i))) {
            count++;
        }
    }
    return count;
}

f32 dAlbwWardrobe_getRecoveryMult() {
    if (!dAlbwWardrobe_isResistanceActive()) {
        return 1.0f;
    }

    const int activeSwords  = dAlbwWardrobe_countActiveSwords();
    const int activeShields = dAlbwWardrobe_countActiveShields();
    const f32 swordPenalty  = kSwordPenaltyPerExtra * static_cast<f32>(activeSwords - 1);
    const f32 shieldPenalty = kShieldPenaltyPerExtra * static_cast<f32>(activeShields - 1);

    f32 outfitPenalty = 0.0f;
    if (dAlbwWardrobe_countOwnedOutfitTypes() >= 2) {
        for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
            const dAlbwOutfitKind kind = static_cast<dAlbwOutfitKind>(i);
            if (dAlbwWardrobe_isActiveOutfit(kind)) {
                outfitPenalty += outfitStackRate(kind);
            }
        }
    }

    f32 mult = 1.0f - swordPenalty - shieldPenalty - outfitPenalty;
    if (mult < 0.05f) {
        mult = 0.05f;
    }
    return mult;
}

void dAlbwWardrobe_debugLogRecoveryState() {
    if (!dAlbwWardrobe_isResistanceActive()) {
        OS_REPORT("[wardrobe] resistance OFF (Quick Swap disabled or wolf)\n");
        return;
    }

    const int activeSwords   = dAlbwWardrobe_countActiveSwords();
    const int activeShields  = dAlbwWardrobe_countActiveShields();
    const int ownedOutfits   = dAlbwWardrobe_countOwnedOutfitTypes();
    const int activeOutfits  = dAlbwWardrobe_countActiveOutfitTypes();
    const f32 swordPenalty   = kSwordPenaltyPerExtra * static_cast<f32>(activeSwords - 1);
    const f32 shieldPenalty  = kShieldPenaltyPerExtra * static_cast<f32>(activeShields - 1);
    f32 outfitPenalty        = 0.0f;
    if (ownedOutfits >= 2) {
        for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
            const dAlbwOutfitKind kind = static_cast<dAlbwOutfitKind>(i);
            if (dAlbwWardrobe_isActiveOutfit(kind)) {
                outfitPenalty += outfitStackRate(kind);
            }
        }
    }
    const f32 mult = dAlbwWardrobe_getRecoveryMult();

    OS_REPORT(
        "[wardrobe] active swords=%d shields=%d outfits=%d/%d owned | "
        "penalties sword=%.0f%% shield=%.0f%% outfit=%.0f%% | recoveryMult=%.3f\n",
        activeSwords, activeShields, activeOutfits, ownedOutfits, swordPenalty * 100.0f,
        shieldPenalty * 100.0f, outfitPenalty * 100.0f, mult);
}

bool dAlbwWardrobe_tryStoreItemNo(u8 itemNo, char* errOut, int errCap) {
    if (!dAlbwWardrobe_isResistanceActive()) {
        copyErr(errOut, errCap, "Storage is only available with Quick Swap enabled.");
        return false;
    }
    const int bit = storageBitForItemNo(itemNo);
    if (bit < 0) {
        copyErr(errOut, errCap, "That item cannot be stored.");
        return false;
    }
    if (dAlbwWardrobe_isStoredItemNo(itemNo)) {
        copyErr(errOut, errCap, "Already in storage.");
        return false;
    }

    if (dMeter2_isShieldItem(itemNo)) {
        if (!shieldIsOwned(itemNo)) {
            copyErr(errOut, errCap, "You do not own that shield.");
            return false;
        }
        if (dAlbwWardrobe_countActiveShields() <= 0) {
            copyErr(errOut, errCap, "Nothing to store.");
            return false;
        }
        setStorageBit(bit, true);
        swapEquippedShieldIfStored(itemNo);
        dAlbwWardrobe_debugLogRecoveryState();
        return true;
    }

    bool isSword = false;
    for (u8 swordNo : kSwordItemNos) {
        if (swordNo == itemNo) {
            isSword = true;
            break;
        }
    }
    if (isSword && swordIsOwned(itemNo)) {
        if (dAlbwWardrobe_countActiveSwords() <= 1) {
            copyErr(errOut, errCap, "Keep at least one sword ready.");
            return false;
        }
        setStorageBit(bit, true);
        swapEquippedSwordIfStored(itemNo);
        dAlbwWardrobe_debugLogRecoveryState();
        return true;
    }

    const dAlbwOutfitKind kind = [&]() -> dAlbwOutfitKind {
        switch (itemNo) {
        case (u8)dItemNo_WEAR_CASUAL_e:   return D_ALBW_OUTFIT_ORDON;
        case (u8)dItemNo_WEAR_KOKIRI_e:   return D_ALBW_OUTFIT_HEROS;
        case (u8)dItemNo_WEAR_ZORA_e:     return D_ALBW_OUTFIT_ZORA;
        case (u8)dItemNo_ARMOR_e:         return D_ALBW_OUTFIT_MAGIC;
        case (u8)dItemNo_DEITY_ARMOR_e:   return D_ALBW_OUTFIT_DEITY;
        default:                          return D_ALBW_OUTFIT_COUNT;
        }
    }();
    if (kind < D_ALBW_OUTFIT_COUNT) {
        return dAlbwWardrobe_tryStoreOutfit(kind, errOut, errCap);
    }

    copyErr(errOut, errCap, "You do not own that item.");
    return false;
}

bool dAlbwWardrobe_tryRetrieveItemNo(u8 itemNo, char* errOut, int errCap) {
    if (!dAlbwWardrobe_isResistanceActive()) {
        copyErr(errOut, errCap, "Storage is only available with Quick Swap enabled.");
        return false;
    }
    if (!dAlbwWardrobe_isStoredItemNo(itemNo)) {
        copyErr(errOut, errCap, "That item is not in storage.");
        return false;
    }

    u16 rupees = dComIfGs_getRupee();
    if (rupees < static_cast<u16>(kAlbwWardrobeStorageRetrievePrice)) {
        copyErr(errOut, errCap, "Not enough rupees.");
        return false;
    }
    dComIfGs_setRupee(rupees - static_cast<u16>(kAlbwWardrobeStorageRetrievePrice));
    setStorageBit(storageBitForItemNo(itemNo), false);
    dAlbwWardrobe_debugLogRecoveryState();
    return true;
}

bool dAlbwWardrobe_tryStoreOutfit(dAlbwOutfitKind kind, char* errOut, int errCap) {
    if (!dAlbwWardrobe_isResistanceActive()) {
        copyErr(errOut, errCap, "Storage is only available with Quick Swap enabled.");
        return false;
    }
    const int bit = storageBitForOutfit(kind);
    if (bit < 0) {
        copyErr(errOut, errCap, "That outfit cannot be stored.");
        return false;
    }
    if (!outfitIsOwned(kind)) {
        copyErr(errOut, errCap, "You do not own that outfit.");
        return false;
    }
    if (dAlbwWardrobe_isStoredOutfit(kind)) {
        copyErr(errOut, errCap, "Already in storage.");
        return false;
    }
    if (dAlbwWardrobe_countActiveOutfitTypes() <= 1) {
        copyErr(errOut, errCap, "Keep at least one outfit ready.");
        return false;
    }

    if (kind == D_ALBW_OUTFIT_SUMO) {
        dAlbwSumoTest_clearWorn();
    }

    setStorageBit(bit, true);
    swapEquippedOutfitIfStored(kind);
    dAlbwWardrobe_debugLogRecoveryState();
    return true;
}

bool dAlbwWardrobe_tryRetrieveOutfit(dAlbwOutfitKind kind, char* errOut, int errCap) {
    if (!dAlbwWardrobe_isResistanceActive()) {
        copyErr(errOut, errCap, "Storage is only available with Quick Swap enabled.");
        return false;
    }
    if (!dAlbwWardrobe_isStoredOutfit(kind)) {
        copyErr(errOut, errCap, "That outfit is not in storage.");
        return false;
    }

    u16 rupees = dComIfGs_getRupee();
    if (rupees < static_cast<u16>(kAlbwWardrobeStorageRetrievePrice)) {
        copyErr(errOut, errCap, "Not enough rupees.");
        return false;
    }
    dComIfGs_setRupee(rupees - static_cast<u16>(kAlbwWardrobeStorageRetrievePrice));
    setStorageBit(storageBitForOutfit(kind), false);
    dAlbwWardrobe_debugLogRecoveryState();
    return true;
}

#endif  // TARGET_PC
