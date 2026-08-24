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

// №238: the dItemNo_* value backing an outfit kind (HEROS -> WEAR_KOKIRI, ...).
// Public wrapper over the module's kind<->item map so grant sites (the Grandma
// clothes handover) can record-then-equip without duplicating the mapping.
int dAlbwOutfit_itemNoForKind(dAlbwOutfitKind kind);

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

// True when quick-swap must be BLOCKED because Link is in a slow/heavy "scripted
// movement" state — the clothes-change rebuild launches Link there.  Covers iron
// boots + depowered Magic Armor (vanilla checkBootsOrArmorHeavy).  Extend as more
// such states (item lockout, ghost-rat cling, ...) get clean checks.  The D-pad
// cycle plays a deny SFX instead of switching when this is true.
bool            dAlbwOutfit_isSwapBlockedState();

// Own-what-you-wear: call once per frame.  Records the stash bit for the
// currently equipped native outfit so vanilla-acquired clothes (Ordon at start,
// Hero's post-Faron, Zora/Magic via story) register as owned for the cycle.
void dAlbwOutfit_syncWornOwnership();

// False while a stage transition, overlap load, or clothes reload is in flight —
// outfit equip / sumo setClothesChange must not run (crashes on field warp).
bool dAlbwOutfit_canTouchLinkModel();

// Clothes resLoad failed: keep the live model (liveArcName still resident) and
// roll save/sync state back to that arc so the clothes pipeline does not spin on
// a dead target. Does not alter the quick-swap ring — failedArcName is log-only.
void dAlbwOutfit_onClothesLoadFailed(const char* liveArcName,
                                     const char* failedArcName = nullptr);

// True while a clothes reload or sumo overlay transition is active.
bool dAlbwOutfit_isSwapInProgress();

// Drain a single pending D-pad equip once the model is safe (call from Link execute).
void dAlbwOutfit_processPendingEquip();

// Single gateway: keep save-bit target and Link model in sync (call from sumo exec).
void dAlbwOutfit_syncLinkModel(daAlink_c* link);

// Reset model-sync tracking at warp/overlap (phased loads reset separately).
void dAlbwOutfit_onStageTransitionBegin();

// Wolf->human metamorphose rebuilds vanilla human via changeLink(0); re-sync ALBW target.
void dAlbwOutfit_onMetamorphoseToHuman(daAlink_c* link);

// Stage warp / overlap only — revert path may still run during clothes reload.
bool dAlbwOutfit_isStageTransitionUnsafe();

// Force ONE model rebuild of the current outfit on the next syncLinkModel, via the
// proven metamorphose-reapply path (sumo -> changeLink(1); native -> reload clothes).
// Used after a Custom Models toggle re-mounts a resident arc (e.g. the sumo body) so
// the visible model is rebuilt from the freshly-overlaid data.
void dAlbwOutfit_forceReapply();

#endif  // TARGET_PC

#endif /* D_ALBW_OUTFIT_H */
