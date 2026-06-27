// ============================================
// NEW CODE — ALBW Port (Outfit module — unified wardrobe API)
// See include/d/d_albw_outfit.h and docs/Interconnected Chats/Quick-Sumo Work.md.
//
// Phase 1: wardrobe OWNERSHIP via save event bits.  A clothes purchase sets the
// matching stash bit so quick-swap can read ownership without coupling to the
// rental re-eligibility bits (dMeter2_playerOwnsRentalItem).
// ============================================
#include "d/d_albw_outfit.h"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "d/d_item_data.h"

namespace {

// Wardrobe/stash save bits (dSv_event_flag_c::saveBitLabels) — shared map in
// docs/Interconnected Chats/Quick-Sumo Work.md.  689 sumo is set by the sumo
// module's shop purchase; 691-695 are owned by this module.
constexpr int kStashSumo  = 689;
constexpr int kStashOrdon = 691;
constexpr int kStashHeros = 692;
constexpr int kStashZora  = 693;
constexpr int kStashMagic = 694;
constexpr int kStashDeity = 695;

// Outfit identity -> stash bit.  Returns -1 for kinds without a stash bit.
int stashBitForKind(dAlbwOutfitKind kind) {
    switch (kind) {
    case D_ALBW_OUTFIT_SUMO:  return kStashSumo;
    case D_ALBW_OUTFIT_ORDON: return kStashOrdon;
    case D_ALBW_OUTFIT_HEROS: return kStashHeros;
    case D_ALBW_OUTFIT_ZORA:  return kStashZora;
    case D_ALBW_OUTFIT_MAGIC: return kStashMagic;
    case D_ALBW_OUTFIT_DEITY: return kStashDeity;
    default:                  return -1;
    }
}

// Native clothes dItemNo -> stash bit.  Returns -1 for non-outfit items.
int stashBitForItemNo(int itemNo) {
    switch (itemNo) {
    case dItemNo_WEAR_CASUAL_e: return kStashOrdon;
    case dItemNo_WEAR_KOKIRI_e: return kStashHeros;
    case dItemNo_WEAR_ZORA_e:   return kStashZora;
    case dItemNo_ARMOR_e:       return kStashMagic;
    case dItemNo_DEITY_ARMOR_e: return kStashDeity;
    default:                    return -1;
    }
}

}  // namespace

bool dAlbwOutfit_isOwned(dAlbwOutfitKind kind) {
    const int bit = stashBitForKind(kind);
    if (bit < 0) {
        return false;
    }
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[bit]) != 0;
}

void dAlbwOutfit_recordOwnedByItemNo(int itemNo) {
    const int bit = stashBitForItemNo(itemNo);
    if (bit < 0) {
        return;  // not a wardrobe-tracked outfit
    }
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[bit]);
}

#endif  // TARGET_PC
