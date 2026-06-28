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

constexpr int kAlbwWardrobeStorageRetrievePrice = 100;

// True when resistance rules apply: human Link + D-pad Quick Swap ON.
bool dAlbwWardrobe_isResistanceActive();

// ---- Postman storage (save bits 697-699, 703-712; bit 700 = sumo worn) ----
bool dAlbwWardrobe_isStorableItemNo(u8 itemNo);
bool dAlbwWardrobe_isStorableOutfit(dAlbwOutfitKind kind);

bool dAlbwWardrobe_isStoredItemNo(u8 itemNo);
bool dAlbwWardrobe_isStoredOutfit(dAlbwOutfitKind kind);

// Owned in the active wardrobe (ownership bit/first-bit intact, not Postman-stored).
bool dAlbwWardrobe_isActiveSword(u8 itemNo);
bool dAlbwWardrobe_isActiveShield(u8 itemNo);
bool dAlbwWardrobe_isActiveOutfit(dAlbwOutfitKind kind);

int dAlbwWardrobe_countActiveSwords();
int dAlbwWardrobe_countActiveShields();
int dAlbwWardrobe_countOwnedOutfitTypes();
int dAlbwWardrobe_countActiveOutfitTypes();

// recoveryMult = 1 - swordPenalty - shieldPenalty - outfitPenalty (see spec §1).
// Returns 1.0f when resistance is inactive.
f32 dAlbwWardrobe_getRecoveryMult();

// Logs active counts, penalties, and recoveryMult (OS_REPORT). Call after store/retrieve.
void dAlbwWardrobe_debugLogRecoveryState();

// Store (free) / retrieve (100 R).  errOut optional; truncated to errCap when non-null.
bool dAlbwWardrobe_tryStoreItemNo(u8 itemNo, char* errOut, int errCap);
bool dAlbwWardrobe_tryRetrieveItemNo(u8 itemNo, char* errOut, int errCap);
bool dAlbwWardrobe_tryStoreOutfit(dAlbwOutfitKind kind, char* errOut, int errCap);
bool dAlbwWardrobe_tryRetrieveOutfit(dAlbwOutfitKind kind, char* errOut, int errCap);

#endif  // TARGET_PC

#endif /* D_ALBW_WARDROBE_H */
