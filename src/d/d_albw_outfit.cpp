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
#include "d/d_meter2_info.h"

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

// Sumo CURRENTLY worn (per-save).  697-699 belong to Quick Swap, so 700.
constexpr int kSumoWornBit = 700;

// Native clothes dItemNo for an outfit kind.  -1 for SUMO (no native item).
int itemNoForKind(dAlbwOutfitKind kind) {
    switch (kind) {
    case D_ALBW_OUTFIT_ORDON: return dItemNo_WEAR_CASUAL_e;
    case D_ALBW_OUTFIT_HEROS: return dItemNo_WEAR_KOKIRI_e;
    case D_ALBW_OUTFIT_ZORA:  return dItemNo_WEAR_ZORA_e;
    case D_ALBW_OUTFIT_MAGIC: return dItemNo_ARMOR_e;
    case D_ALBW_OUTFIT_DEITY: return dItemNo_DEITY_ARMOR_e;
    default:                  return -1;
    }
}

// Equipped native clothes dItemNo -> outfit kind (defaults to Ordon, the base).
dAlbwOutfitKind kindForClothes(int clothes) {
    switch (clothes) {
    case dItemNo_WEAR_KOKIRI_e: return D_ALBW_OUTFIT_HEROS;
    case dItemNo_WEAR_ZORA_e:   return D_ALBW_OUTFIT_ZORA;
    case dItemNo_ARMOR_e:       return D_ALBW_OUTFIT_MAGIC;
    case dItemNo_DEITY_ARMOR_e: return D_ALBW_OUTFIT_DEITY;
    case dItemNo_WEAR_CASUAL_e:
    default:                    return D_ALBW_OUTFIT_ORDON;
    }
}

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

bool dAlbwOutfit_isSumoWorn() {
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[kSumoWornBit]) != 0;
}

void dAlbwOutfit_setSumoWorn(bool on) {
    if (on) {
        dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoWornBit]);
    } else {
        dComIfGs_offEventBit(dSv_event_flag_c::saveBitLabels[kSumoWornBit]);
    }
}

dAlbwOutfitKind dAlbwOutfit_getActive() {
    // TARGET semantics: sumo if the worn bit is set, else the equipped clothes.
    if (dAlbwOutfit_isSumoWorn()) {
        return D_ALBW_OUTFIT_SUMO;
    }
    return kindForClothes(dComIfGs_getSelectEquipClothes());
}

bool dAlbwOutfit_isActive(dAlbwOutfitKind kind) {
    return dAlbwOutfit_getActive() == kind;
}

bool dAlbwOutfit_equip(dAlbwOutfitKind kind) {
    if (kind >= D_ALBW_OUTFIT_COUNT) {
        return false;
    }
    if (!dAlbwOutfit_isOwned(kind)) {
        return false;  // only equip outfits already in the wardrobe
    }
    if (kind == D_ALBW_OUTFIT_SUMO) {
        dAlbwOutfit_setSumoWorn(true);  // overlay re-applies next frame (sumo exec)
        return true;
    }
    const int itemNo = itemNoForKind(kind);
    if (itemNo < 0) {
        return false;
    }
    dAlbwOutfit_setSumoWorn(false);          // mutual exclusion: drop the sumo overlay
    dMeter2_grantRentalClothes((u8)itemNo);  // equip + auto-equip the native clothes
    return true;
}

dAlbwOutfitKind dAlbwOutfit_getNextOwned(dAlbwOutfitKind current) {
    // Fixed cycle order (Deity excluded from the v1 rotation).
    static const dAlbwOutfitKind kCycle[] = {
        D_ALBW_OUTFIT_SUMO,  D_ALBW_OUTFIT_ORDON, D_ALBW_OUTFIT_HEROS,
        D_ALBW_OUTFIT_ZORA,  D_ALBW_OUTFIT_MAGIC,
    };
    const int n = (int)(sizeof(kCycle) / sizeof(kCycle[0]));

    int ci = -1;
    for (int i = 0; i < n; ++i) {
        if (kCycle[i] == current) {
            ci = i;
            break;
        }
    }
    // Scan forward from current (or from the front if current is off-cycle, e.g.
    // Deity), returning the first owned outfit; fall back to current if <=1 owned.
    for (int s = 1; s <= n; ++s) {
        const int idx = ((ci + s) % n + n) % n;
        const dAlbwOutfitKind cand = kCycle[idx];
        if (ci >= 0 && cand == current) {
            break;  // wrapped back to current
        }
        if (dAlbwOutfit_isOwned(cand)) {
            return cand;
        }
    }
    return current;
}

void dAlbwOutfit_syncWornOwnership() {
    // You own what you wear: seed the stash bit for the equipped native outfit so
    // vanilla-acquired clothes register as owned without per-grant-site hooks.
    const int bit = stashBitForItemNo(dComIfGs_getSelectEquipClothes());
    if (bit >= 0 && dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[bit]) == 0) {
        dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[bit]);
    }
}

#endif  // TARGET_PC
