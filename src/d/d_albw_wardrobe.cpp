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
#include "dusk/settings.h"

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
    const bool storedIsEquipped =
        dAlbwOutfit_isActive(storedKind) ||
        (storedKind == D_ALBW_OUTFIT_SUMO && dAlbwSumoTest_isOutfitActive());
    if (!storedIsEquipped) {
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

static void copyLabel(char* dst, int cap, const char* src) {
    if (dst == nullptr || cap <= 0) {
        return;
    }
    std::snprintf(dst, static_cast<size_t>(cap), "%s", src != nullptr ? src : "?");
}

static const char* swordLabel(u8 itemNo) {
    switch (itemNo) {
    case (u8)dItemNo_WOOD_STICK_e:    return "Wooden Sword";
    case (u8)dItemNo_SWORD_e:         return "Ordon Sword";
    case (u8)dItemNo_MASTER_SWORD_e:  return "Master Sword";
    case (u8)dItemNo_LIGHT_SWORD_e:   return "Light Sword";
    default:                          return "None";
    }
}

static const char* shieldLabel(u8 itemNo) {
    switch (itemNo) {
    case (u8)dItemNo_WOOD_SHIELD_e:   return "Ordon Shield";
    case (u8)dItemNo_SHIELD_e:        return "Wooden Shield";
    case (u8)dItemNo_HYLIA_SHIELD_e:  return "Hylian Shield";
    default:                          return "None";
    }
}

static const char* outfitKindLabel(dAlbwOutfitKind kind) {
    switch (kind) {
    case D_ALBW_OUTFIT_SUMO:  return "Sumo";
    case D_ALBW_OUTFIT_ORDON: return "Ordon";
    case D_ALBW_OUTFIT_HEROS: return "Hero's";
    case D_ALBW_OUTFIT_ZORA:  return "Zora";
    case D_ALBW_OUTFIT_MAGIC: return "Magic";
    case D_ALBW_OUTFIT_DEITY: return "Deity";
    default:                  return "None";
    }
}

static int countStoredSwords() {
    int count = 0;
    for (u8 itemNo : kSwordItemNos) {
        if (dAlbwWardrobe_isStoredItemNo(itemNo)) {
            count++;
        }
    }
    return count;
}

static int countStoredShields() {
    int count = 0;
    for (u8 itemNo : kShieldItemNos) {
        if (dAlbwWardrobe_isStoredItemNo(itemNo)) {
            count++;
        }
    }
    return count;
}

static int countStoredOutfitTypes() {
    int count = 0;
    for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
        if (dAlbwWardrobe_isStoredOutfit(static_cast<dAlbwOutfitKind>(i))) {
            count++;
        }
    }
    return count;
}

}  // namespace

bool dAlbwWardrobe_isDebugOverlayEnabled() {
    return dusk::getSettings().game.showWardrobeRecoveryDebug.getValue();
}

bool dAlbwWardrobe_isResistanceActive() {
    if (!dusk::isDpadQuickSwapEnabled()) {
        return false;
    }
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    return player != nullptr && !player->checkWolf();
}

int dAlbwWardrobe_retrievePriceForItemNo(u8 itemNo) {
    if (itemNo == (u8)dItemNo_ARMOR_e) {
        return kAlbwWardrobeMagicRetrievePrice;
    }
    return kAlbwWardrobeStorageRetrievePrice;
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

dAlbwOutfitKind dAlbwWardrobe_outfitKindForItemNo(u8 itemNo) {
    switch (itemNo) {
    case (u8)dItemNo_WEAR_CASUAL_e: return D_ALBW_OUTFIT_ORDON;
    case (u8)dItemNo_WEAR_KOKIRI_e: return D_ALBW_OUTFIT_HEROS;
    case (u8)dItemNo_WEAR_ZORA_e:   return D_ALBW_OUTFIT_ZORA;
    case (u8)dItemNo_ARMOR_e:       return D_ALBW_OUTFIT_MAGIC;
    case (u8)dItemNo_DEITY_ARMOR_e: return D_ALBW_OUTFIT_DEITY;
    default:                        return D_ALBW_OUTFIT_COUNT;
    }
}

bool dAlbwWardrobe_canEquipItemNo(u8 itemNo) {
    if (!dAlbwWardrobe_isResistanceActive()) {
        return true;
    }
    const dAlbwOutfitKind kind = dAlbwWardrobe_outfitKindForItemNo(itemNo);
    if (kind < D_ALBW_OUTFIT_COUNT) {
        return dAlbwWardrobe_isActiveOutfit(kind);
    }
    if (dMeter2_isShieldItem(itemNo)) {
        return dAlbwWardrobe_isActiveShield(itemNo);
    }
    return dAlbwWardrobe_isActiveSword(itemNo);
}

bool dAlbwWardrobe_canShowItemNo(u8 itemNo) {
    return dAlbwWardrobe_canEquipItemNo(itemNo);
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

static f32 computeOutfitPenalty() {
    if (dAlbwWardrobe_countOwnedOutfitTypes() < 2) {
        return 0.0f;
    }
    f32 outfitPenalty = 0.0f;
    for (int i = 0; i < D_ALBW_OUTFIT_COUNT; ++i) {
        const dAlbwOutfitKind kind = static_cast<dAlbwOutfitKind>(i);
        if (dAlbwWardrobe_isActiveOutfit(kind)) {
            outfitPenalty += outfitStackRate(kind);
        }
    }
    return outfitPenalty;
}

static int applyRecoveryTax(int baseRate, f32 mult) {
    if (mult >= 0.999f) {
        return baseRate;
    }
    int taxed = static_cast<int>(static_cast<f32>(baseRate) * mult);
    return taxed < 1 ? 1 : taxed;
}

f32 dAlbwWardrobe_getRecoveryMult() {
    if (!dAlbwWardrobe_isResistanceActive()) {
        return 1.0f;
    }

    const int activeSwords  = dAlbwWardrobe_countActiveSwords();
    const int activeShields = dAlbwWardrobe_countActiveShields();
    const f32 swordPenalty  = kSwordPenaltyPerExtra * static_cast<f32>(activeSwords - 1);
    const f32 shieldPenalty = kShieldPenaltyPerExtra * static_cast<f32>(activeShields - 1);

    f32 outfitPenalty = computeOutfitPenalty();

    f32 mult = 1.0f - swordPenalty - shieldPenalty - outfitPenalty;
    if (mult < 0.05f) {
        mult = 0.05f;
    }
    return mult;
}

void dAlbwWardrobe_fillDebugSnapshot(dAlbwWardrobeDebugSnapshot* out) {
    if (out == nullptr) {
        return;
    }
    *out = {};

    out->quickSwapEnabled = dusk::isDpadQuickSwapEnabled();
    daPy_py_c* player       = daPy_getLinkPlayerActorClass();
    out->wolfForm           = player != nullptr && player->checkWolf() != 0;
    out->resistanceActive   = dAlbwWardrobe_isResistanceActive();

    out->activeSwords       = dAlbwWardrobe_countActiveSwords();
    out->activeShields      = dAlbwWardrobe_countActiveShields();
    out->activeOutfitTypes  = dAlbwWardrobe_countActiveOutfitTypes();
    out->ownedOutfitTypes   = dAlbwWardrobe_countOwnedOutfitTypes();
    out->storedSwords       = countStoredSwords();
    out->storedShields      = countStoredShields();
    out->storedOutfitTypes  = countStoredOutfitTypes();

    out->swordPenalty  = kSwordPenaltyPerExtra * static_cast<f32>(out->activeSwords - 1);
    out->shieldPenalty = kShieldPenaltyPerExtra * static_cast<f32>(out->activeShields - 1);
    out->outfitPenalty = computeOutfitPenalty();
    out->recoveryMult  = dAlbwWardrobe_getRecoveryMult();

    out->baseNormalRecoveryPer100ms   = dMeter2_getALBWNormalRecoveryRate();
    out->baseLockoutRecoveryPer100ms  = dMeter2_getALBWLockoutRecoveryRate();
    out->taxedNormalRecoveryPer100ms  =
        applyRecoveryTax(out->baseNormalRecoveryPer100ms, out->recoveryMult);
    out->taxedLockoutRecoveryPer100ms =
        applyRecoveryTax(out->baseLockoutRecoveryPer100ms, out->recoveryMult);

    copyLabel(out->equippedSword, static_cast<int>(sizeof(out->equippedSword)),
              swordLabel(dComIfGs_getSelectEquipSword()));
    copyLabel(out->equippedShield, static_cast<int>(sizeof(out->equippedShield)),
              shieldLabel(dComIfGs_getSelectEquipShield()));
    copyLabel(out->equippedOutfit, static_cast<int>(sizeof(out->equippedOutfit)),
              outfitKindLabel(dAlbwOutfit_getActive()));
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

    const int price = dAlbwWardrobe_retrievePriceForItemNo(itemNo);
    u16 rupees = dComIfGs_getRupee();
    if (rupees < static_cast<u16>(price)) {
        copyErr(errOut, errCap, "Not enough rupees.");
        return false;
    }
    dComIfGs_setRupee(rupees - static_cast<u16>(price));
    setStorageBit(storageBitForItemNo(itemNo), false);
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

    setStorageBit(bit, true);
    swapEquippedOutfitIfStored(kind);
    return true;
}

// ============================================
// Death confiscation (alpha cleanup — Magic loss on death). Sets the
// stored bit directly, bypassing tryStoreOutfit's gates on purpose:
// death is authoritative (no Quick-Swap requirement, no keep-one-ready
// guard — the strip resets clothes to Hero's itself, and the wardrobe
// per-frame sync latches Hero's ownership, so an active outfit always
// remains). Ownership (stash bit) is deliberately KEPT — clearing it
// while stored would orphan the outfit (retrieve clears only the stored
// bit and never re-grants ownership). Re-acquisition = the existing
// Postman storage Retrieve row.
// ============================================
void dAlbwWardrobe_storeOutfitOnDeath(dAlbwOutfitKind kind) {
    const int bit = storageBitForOutfit(kind);
    if (bit < 0) {
        return;
    }
    setStorageBit(bit, true);
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
    return true;
}

#endif  // TARGET_PC
