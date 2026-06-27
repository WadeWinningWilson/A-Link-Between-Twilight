#ifndef D_ALBW_OUTFIT_H
#define D_ALBW_OUTFIT_H

// ============================================
// NEW CODE — ALBW Port (Outfit module — unified wardrobe API)
// Shared surface between the Sumo overlay / rental shop (this chat) and the
// D-pad outfit quick-swap (other chat).  See docs/Interconnected Chats/
// Quick-Sumo Work.md for the contract and the save-bit map.
//
// Phase 1 (here): OWNERSHIP only — wardrobe/stash bits, not
// dMeter2_playerOwnsRentalItem().  A clothes purchase records the matching
// stash bit; quick-swap reads ownership through dAlbwOutfit_isOwned().
// Phase 2 (later): equip / getActive (TARGET semantics) / getNextOwned and the
// dAlbwCap_* global-cap surface land in this same module.
// ============================================
#if TARGET_PC

// Wardrobe outfit identities, in fixed cycle order (Deity reserved, excluded
// from the v1 rotation).  Quick-swap's cycleNextOutfit() walks this order.
enum dAlbwOutfitKind {
    D_ALBW_OUTFIT_SUMO,
    D_ALBW_OUTFIT_ORDON,
    D_ALBW_OUTFIT_HEROS,
    D_ALBW_OUTFIT_ZORA,
    D_ALBW_OUTFIT_MAGIC,
    D_ALBW_OUTFIT_DEITY,   // reserved — owned-readable, not in v1 cycle
    D_ALBW_OUTFIT_COUNT
};

// True if the outfit is in the player's wardrobe (stash save bit set).  Reads
// the save event bits only (689 sumo, 691-694 native, 695 deity) — never
// dMeter2_playerOwnsRentalItem(), per the locked ownership semantics.
bool dAlbwOutfit_isOwned(dAlbwOutfitKind kind);

// Record wardrobe ownership for a clothes item the shop just granted, keyed by
// its dItemNo_* value (Ordon/Hero's/Zora/Magic/Deity).  No-op for any other
// itemNo.  Call alongside dMeter2_grantRentalClothes().
void dAlbwOutfit_recordOwnedByItemNo(int itemNo);

#endif  // TARGET_PC

#endif /* D_ALBW_OUTFIT_H */
