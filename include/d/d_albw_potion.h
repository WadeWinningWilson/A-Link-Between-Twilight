#ifndef D_ALBW_POTION_H

#define D_ALBW_POTION_H



// ============================================

// NEW CODE — ALBW Port

// Soulbound red potion bottle (SLOT_11) — multi-use / refill slice in progress.

// ============================================



#if TARGET_PC



#include "d/d_save.h"



static constexpr u8 kAlbwPotionSoulboundSlot = SLOT_11;

static constexpr u8 kAlbwPotionSoulboundBottleIdx = 0;

static constexpr u8 kAlbwPotionDefaultStartCharges = 2;
static constexpr u8 kAlbwPotionCapacityUpgradeUses = 3;
static constexpr s16 kAlbwPotionDefaultHealQuarters = 8; // 2 hearts (4 quarters = 1 heart)



bool dAlbwPotion_isSoulboundBottleSlot(u8 slot);

bool dAlbwPotion_isSoulboundRedItem(u8 itemNo);

bool dAlbwPotion_isSoulboundRedInSlot(u8 slot);



u8 dAlbwPotion_getMaxUses();
s16 dAlbwPotion_getHealQuarters(int i_selItemIdx, u8 i_itemNo);

bool dAlbwPotion_canDrinkSelectItem(int i_selItemIdx, u8 i_itemNo);

void dAlbwPotion_consumeSoulboundDrink(int i_selItemIdx);

void dAlbwPotion_refillSoulboundIfEmpty();
void dAlbwPotion_applyDefaultInventorySlot11();

// Editor ALBW WIP toggle: grant or remove soulbound red potion in slot 11.
void dAlbwPotion_editorSetSoulboundEnabled(bool enabled);

// Postman shop — Red Potion Capacity (mEvent[105]); potency deferred.
bool dAlbwPotion_shouldShowCapacityShopRow();
bool dAlbwPotion_canPurchaseCapacityShop();
int dAlbwPotion_getCapacityShopPrice();
bool dAlbwPotion_tryPurchaseCapacityShop();
const char* dAlbwPotion_getCapacityShopName();
const char* dAlbwPotion_getCapacityShopDesc();



#else



inline bool dAlbwPotion_isSoulboundBottleSlot(u8) {

    return false;

}



inline bool dAlbwPotion_isSoulboundRedItem(u8) {

    return false;

}



inline bool dAlbwPotion_isSoulboundRedInSlot(u8) {

    return false;

}



inline u8 dAlbwPotion_getMaxUses() {

    return 0;

}



inline s16 dAlbwPotion_getHealQuarters(int, u8) {

    return 32;

}



inline bool dAlbwPotion_canDrinkSelectItem(int, u8) {

    return true;

}



inline void dAlbwPotion_consumeSoulboundDrink(int) {}



inline void dAlbwPotion_refillSoulboundIfEmpty() {}
inline void dAlbwPotion_applyDefaultInventorySlot11() {}

inline void dAlbwPotion_editorSetSoulboundEnabled(bool) {}

inline bool dAlbwPotion_shouldShowCapacityShopRow() { return false; }
inline bool dAlbwPotion_canPurchaseCapacityShop() { return false; }
inline int dAlbwPotion_getCapacityShopPrice() { return 0; }
inline bool dAlbwPotion_tryPurchaseCapacityShop() { return false; }
inline const char* dAlbwPotion_getCapacityShopName() { return ""; }
inline const char* dAlbwPotion_getCapacityShopDesc() { return ""; }



#endif



#endif

