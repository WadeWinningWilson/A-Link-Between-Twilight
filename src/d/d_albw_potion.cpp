// ============================================
// NEW CODE — ALBW Port
// Soulbound red potion bottle (SLOT_11).
// ============================================

#include "d/d_albw_potion.h"

#if TARGET_PC

#include "d/d_com_inf_game.h"

namespace {

// Full byte of dSv_event_c::mEvent[] — index 105 (confirmed unused by vanilla).
static constexpr u16 kAlbwPotionCapacityTierReg = static_cast<u16>(105 << 8) | 0xFF;
static constexpr u8 kAlbwPotionCapacityShopTiers = 1;
static constexpr int kAlbwPotionCapacityShopPrice = 80;

static u8 readCapacityTier() {
    const int tier = dComIfGs_getEventReg(kAlbwPotionCapacityTierReg);
    if (tier < 0) {
        return 0;
    }
    if (tier > kAlbwPotionCapacityShopTiers) {
        return kAlbwPotionCapacityShopTiers;
    }
    return static_cast<u8>(tier);
}

static void writeCapacityTier(u8 tier) {
    dComIfGs_setEventReg(kAlbwPotionCapacityTierReg, tier);
}

}  // namespace

bool dAlbwPotion_isSoulboundBottleSlot(u8 slot) {
    return slot == kAlbwPotionSoulboundSlot;
}

bool dAlbwPotion_isSoulboundRedItem(u8 itemNo) {
    return itemNo == dItemNo_RED_BOTTLE_e || itemNo == dItemNo_RED_BOTTLE_2_e ||
           itemNo == dItemNo_CHUCHU_RED_e;
}

bool dAlbwPotion_isSoulboundRedInSlot(u8 slot) {
    if (!dAlbwPotion_isSoulboundBottleSlot(slot)) {
        return false;
    }

    const u8 item = dComIfGs_getItem(slot, true);
    return dAlbwPotion_isSoulboundRedItem(item);
}

u8 dAlbwPotion_getMaxUses() {
    if (readCapacityTier() >= 1) {
        return kAlbwPotionCapacityUpgradeUses;
    }
    return kAlbwPotionDefaultStartCharges;
}

s16 dAlbwPotion_getHealQuarters(int i_selItemIdx, u8 i_itemNo) {
    if (!dAlbwPotion_isSoulboundRedItem(i_itemNo)) {
        return 32;
    }

    const u8 slot = dComIfGs_getSelectItemIndex(i_selItemIdx);
    if (!dAlbwPotion_isSoulboundBottleSlot(slot)) {
        return 32;
    }

    // Tier 0 soulbound bottle; shop upgrades can raise this later.
    return kAlbwPotionDefaultHealQuarters;
}

static void sync_select_items_for_slot(u8 slot) {
    for (int j = 0; j < 3; j++) {
        if (dComIfGs_getSelectItemIndex(j) == slot) {
            dComIfGp_setSelectItem(j);
        }
    }
}

bool dAlbwPotion_canDrinkSelectItem(int i_selItemIdx, u8 i_itemNo) {
    if (!dAlbwPotion_isSoulboundRedItem(i_itemNo)) {
        return true;
    }

    const u8 slot = dComIfGs_getSelectItemIndex(i_selItemIdx);
    if (!dAlbwPotion_isSoulboundBottleSlot(slot)) {
        return true;
    }

    return dComIfGs_getBottleNum(kAlbwPotionSoulboundBottleIdx) > 0;
}

void dAlbwPotion_consumeSoulboundDrink(int i_selItemIdx) {
    const u8 slot = dComIfGs_getSelectItemIndex(i_selItemIdx);
    if (!dAlbwPotion_isSoulboundRedInSlot(slot)) {
        return;
    }

    if (dComIfGs_getBottleNum(kAlbwPotionSoulboundBottleIdx) > 0) {
        dComIfGs_addBottleNum(kAlbwPotionSoulboundBottleIdx, -1);
    }

    sync_select_items_for_slot(slot);
}

void dAlbwPotion_refillSoulboundIfEmpty() {
    if (!dAlbwPotion_isSoulboundRedInSlot(kAlbwPotionSoulboundSlot)) {
        return;
    }

    if (dComIfGs_getBottleNum(kAlbwPotionSoulboundBottleIdx) == 0) {
        dComIfGs_setBottleNum(kAlbwPotionSoulboundBottleIdx, dAlbwPotion_getMaxUses());
        sync_select_items_for_slot(kAlbwPotionSoulboundSlot);
    }
}

void dAlbwPotion_applyDefaultInventorySlot11() {
    dComIfGs_setItem(kAlbwPotionSoulboundSlot, dItemNo_RED_BOTTLE_e);
    dComIfGs_setBottleNum(kAlbwPotionSoulboundBottleIdx, dAlbwPotion_getMaxUses());
    dComIfGs_onItemFirstBit(dItemNo_RED_BOTTLE_e);
    sync_select_items_for_slot(kAlbwPotionSoulboundSlot);
}

void dAlbwPotion_editorSetSoulboundEnabled(bool enabled) {
    if (enabled) {
        dAlbwPotion_applyDefaultInventorySlot11();
        return;
    }

    const u8 item = dComIfGs_getItem(kAlbwPotionSoulboundSlot, true);
    if (dAlbwPotion_isSoulboundRedItem(item)) {
        dComIfGs_setItem(kAlbwPotionSoulboundSlot, dItemNo_EMPTY_BOTTLE_e);
        dComIfGs_setBottleNum(kAlbwPotionSoulboundBottleIdx, 0);
        sync_select_items_for_slot(kAlbwPotionSoulboundSlot);
    }
}

bool dAlbwPotion_shouldShowCapacityShopRow() {
    return dAlbwPotion_isSoulboundRedInSlot(kAlbwPotionSoulboundSlot);
}

bool dAlbwPotion_canPurchaseCapacityShop() {
    if (!dAlbwPotion_shouldShowCapacityShopRow()) {
        return false;
    }
    return readCapacityTier() < kAlbwPotionCapacityShopTiers;
}

int dAlbwPotion_getCapacityShopPrice() {
    if (!dAlbwPotion_canPurchaseCapacityShop()) {
        return 0;
    }
    return kAlbwPotionCapacityShopPrice;
}

bool dAlbwPotion_tryPurchaseCapacityShop() {
    if (!dAlbwPotion_canPurchaseCapacityShop()) {
        return false;
    }

    writeCapacityTier(static_cast<u8>(readCapacityTier() + 1));

    if (dAlbwPotion_isSoulboundRedInSlot(kAlbwPotionSoulboundSlot)) {
        dComIfGs_setBottleNum(kAlbwPotionSoulboundBottleIdx, dAlbwPotion_getMaxUses());
        sync_select_items_for_slot(kAlbwPotionSoulboundSlot);
    }

    return true;
}

const char* dAlbwPotion_getCapacityShopName() {
    return "Red Potion Capacity";
}

const char* dAlbwPotion_getCapacityShopDesc() {
    if (!dAlbwPotion_canPurchaseCapacityShop()) {
        return "Sold out.";
    }
    return "I can expand that vessel for you if you like. My father taught me all "
           "about his craft....maybe I should have followed in his footsteps...";
}

#endif
