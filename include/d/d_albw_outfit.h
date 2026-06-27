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

// ---- Currently-worn sumo state (per-save, save bit 700) -----------------------
// The sumo overlay's "is worn" flag lives in the save (not AppData config), so it
// is per-file and survives the removal of the editor toggle.  The sumo module
// reads isSumoWorn() for its per-frame apply; the shop / D-pad set it.
bool dAlbwOutfit_isSumoWorn();
void dAlbwOutfit_setSumoWorn(bool on);

// ---- Active outfit + equip + cycle (D-pad outfit quick-swap surface) ----------
// getActive() returns the TARGET outfit (intended), not the live model: SUMO when
// the worn bit is set, else the equipped native clothes.  equip() is async-
// initiate (the model swap completes over the next frames); it equips only OWNED
// outfits and enforces mutual exclusion (equipping a native clears the sumo
// overlay).  getNextOwned() walks the fixed cycle order, skips unowned, and
// returns `current` when 0-or-1 outfits are owned (so Down becomes a no-op).
dAlbwOutfitKind dAlbwOutfit_getActive();
bool            dAlbwOutfit_isActive(dAlbwOutfitKind kind);
bool            dAlbwOutfit_equip(dAlbwOutfitKind kind);
dAlbwOutfitKind dAlbwOutfit_getNextOwned(dAlbwOutfitKind current);

// Own-what-you-wear: call once per frame.  Records the stash bit for the
// currently equipped native outfit so vanilla-acquired clothes (Ordon at start,
// Hero's post-Faron, Zora/Magic via story) register as owned for the cycle.
void dAlbwOutfit_syncWornOwnership();

#endif  // TARGET_PC

#endif /* D_ALBW_OUTFIT_H */
