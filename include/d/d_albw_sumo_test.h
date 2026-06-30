#ifndef D_ALBW_SUMO_TEST_H
#define D_ALBW_SUMO_TEST_H

// ============================================
// NEW CODE — ALBW Port (Sumo Link visual test)
// Dev-only test toggle (Dusklight editor menu -> ALBW -> SumoTest).
// Field-capable: Link loads the "alSumou" archive itself, so the swap works
// anywhere (the wrestler NPC is not required to be present).  Swaps Link's
// body to the shirtless sumo model via the existing clothes-change path
// (FLG2_UNK_200000), WITHOUT entering the sumo minigame (no setSumouReady,
// no item delete, no ring/demo lock).  Toggling off restores the player's
// real equipped clothes.  NOT persisted to the save.
// ============================================
#if TARGET_PC

class daAlink_c;

// Per-frame driver: keeps Link's sumo visual (FLG2_UNK_80000) in sync with save bit 700.
void dAlbwSumoTest_exec(daAlink_c* i_link);

// Load alSumou + base-clothes arcs so changeLink() can build sumo on Link create/warp.
// Returns true when resources are resident and FLG2_UNK_200000 may be set.
bool dAlbwSumoTest_prepareChangeLink();

// Load the equipped native-clothes arc so changeLink() can rebuild after the sumo
// overlay is cleared.  Required before setClothesChange(0) when leaving sumo.
bool dAlbwSumoTest_prepareNativeClothesChange();

// True while the sumo body flag is on Link (reads FLG2_UNK_80000, not static state).
bool dAlbwSumoTest_isOutfitActive();

// True while the outfit is on AND not "Fists Only" (cheap cached bool).  Used by
// checkSwordDraw/checkShieldDraw to draw the sword/shield on the sumo body.
bool dAlbwSumoTest_showWeapons();

// True when the "Link Hat" sub-toggle is on — read by changeLink.
bool dAlbwSumoTest_wantLinkCap();

// The cap model the Link Hat shows, by game.sumoCapColor.  changeLink loads capModelName()
// from capArcName(); both resolve on any base via the decoupled cap donor (resourcesReady).
const char* dAlbwSumoTest_capArcName();    // "Kmdl" (green) / "Mmdl" (red) / "Zmdl" (blue)
const char* dAlbwSumoTest_capModelName();  // "al_head.bmd" / "ml_head.bmd" / "zl_head.bmd"

// ---- Shop integration (Sumo Outfit purchase) ----------------------------------
// True once the Sumo Outfit has been bought (save-backed ownership bit).  This is
// the foundation brick of the planned stored-armors / quick-swap system.
bool dAlbwSumoTest_isOwned();

// Whether the "Sumo Outfit" shop row should be shown: not already owned, AND
// (True ALBW is on OR the Ordon sumo-wrestler NPC has been met).
bool dAlbwSumoTest_isShopEligible();

// Shop purchase: record ownership (save bit) and wear it now.  Returns true.
bool dAlbwSumoTest_tryPurchaseShop();

// Turn the sumo overlay OFF — called when the player buys a real clothes outfit
// so the chosen clothes show instead of sumo re-applying over them.
void dAlbwSumoTest_clearWorn();

// Native outfit equip (shop / D-pad) took over from sumo — optional hook; sync is driven
// by the worn bit vs Link visual flag.
void dAlbwSumoTest_onNativeOutfitEquipped();

// True while worn bit != Link sumo visual, or a clothes reload is in flight.
bool dAlbwSumoTest_isSwapPending();

// Marks the Ordon sumo-wrestler NPC as met — gates the shop row in non-True-ALBW
// play.  Called from daNpcWrestler_c::talk().
void dAlbwSumoTest_onWrestlerMet();

#endif  // TARGET_PC

#endif /* D_ALBW_SUMO_TEST_H */
