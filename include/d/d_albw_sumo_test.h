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

// Per-frame driver, called from daAlink_c::execute().  Reads game.sumoTest
// (Editor -> ALBW tab) and applies/reverts the sumo model swap.
void dAlbwSumoTest_exec(daAlink_c* i_link);

// True while the SumoTest outfit is currently swapped in (cheap bool, no
// settings read).  Used by checkSwordDraw/checkShieldDraw to keep weapons
// visible for the dev outfit (the sumo-flag suppression is for the minigame).
bool dAlbwSumoTest_isOutfitActive();

#endif  // TARGET_PC

#endif /* D_ALBW_SUMO_TEST_H */
