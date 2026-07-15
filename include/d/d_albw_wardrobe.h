#ifndef D_ALBW_WARDROBE_H
#define D_ALBW_WARDROBE_H

// ============================================
// NEW CODE — ALBW Port (Quick Swap wardrobe load / Postman storage)
// Active wardrobe = owned items not in Postman storage.  Consumed by the rental
// shop (store/retrieve), D-pad quick-swap cycles, and ALBW recoveryMult (meter).
// See docs/Interconnected Chats/Quick-Resistance Work.md.
// ============================================
#if TARGET_PC

#include "d/d_albw_outfit.h"
#include "dolphin/types.h"

constexpr int kAlbwWardrobeStorageRetrievePrice = 100;

struct dAlbwWardrobeDebugSnapshot {
    bool resistanceActive;
    bool quickSwapEnabled;
    bool wolfForm;
    f32  recoveryMult;
    f32  swordPenalty;
    f32  shieldPenalty;
    f32  outfitPenalty;
    int  activeSwords;
    int  activeShields;
    int  activeOutfitTypes;
    int  ownedOutfitTypes;
    int  storedSwords;
    int  storedShields;
    int  storedOutfitTypes;
    int  baseNormalRecoveryPer100ms;
    int  baseLockoutRecoveryPer100ms;
    int  taxedNormalRecoveryPer100ms;
    int  taxedLockoutRecoveryPer100ms;
    char equippedSword[32];
    char equippedShield[32];
    char equippedOutfit[32];
};

// True when resistance rules apply: human Link + D-pad Quick Swap ON.
bool dAlbwWardrobe_isResistanceActive();

// Editor toggle: game.showWardrobeRecoveryDebug (ALBW tab).
bool dAlbwWardrobe_isDebugOverlayEnabled();

void dAlbwWardrobe_fillDebugSnapshot(dAlbwWardrobeDebugSnapshot* out);

// ---- Postman storage (save bits 697-699, 703-712; bit 700 = sumo worn) ----
bool dAlbwWardrobe_isStorableItemNo(u8 itemNo);
bool dAlbwWardrobe_isStorableOutfit(dAlbwOutfitKind kind);

bool dAlbwWardrobe_isStoredItemNo(u8 itemNo);
bool dAlbwWardrobe_isStoredOutfit(dAlbwOutfitKind kind);

// Owned in the active wardrobe (ownership bit/first-bit intact, not Postman-stored).
bool dAlbwWardrobe_isActiveSword(u8 itemNo);
bool dAlbwWardrobe_isActiveShield(u8 itemNo);
bool dAlbwWardrobe_isActiveOutfit(dAlbwOutfitKind kind);

dAlbwOutfitKind dAlbwWardrobe_outfitKindForItemNo(u8 itemNo);

// Collection menu: hide/block Postman-stored items when resistance is active.
bool dAlbwWardrobe_canShowItemNo(u8 itemNo);
bool dAlbwWardrobe_canEquipItemNo(u8 itemNo);

int dAlbwWardrobe_countActiveSwords();
int dAlbwWardrobe_countActiveShields();
int dAlbwWardrobe_countOwnedOutfitTypes();
int dAlbwWardrobe_countActiveOutfitTypes();

// recoveryMult = 1 - swordPenalty - shieldPenalty - outfitPenalty (see spec §1).
// Returns 1.0f when resistance is inactive.
f32 dAlbwWardrobe_getRecoveryMult();

// Store (free) / retrieve (100 R).  errOut optional; truncated to errCap when non-null.
bool dAlbwWardrobe_tryStoreItemNo(u8 itemNo, char* errOut, int errCap);
bool dAlbwWardrobe_tryRetrieveItemNo(u8 itemNo, char* errOut, int errCap);
bool dAlbwWardrobe_tryStoreOutfit(dAlbwOutfitKind kind, char* errOut, int errCap);
bool dAlbwWardrobe_tryRetrieveOutfit(dAlbwOutfitKind kind, char* errOut, int errCap);
// Death confiscation: force-store an outfit (no Quick-Swap/keep-one gates,
// ownership kept). Re-acquire via the Postman storage Retrieve row.
void dAlbwWardrobe_storeOutfitOnDeath(dAlbwOutfitKind kind);

#endif  // TARGET_PC

#endif /* D_ALBW_WARDROBE_H */
