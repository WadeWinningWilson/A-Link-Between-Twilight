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

#include "d/d_albw_outfit_debug.h"
#include "d/d_albw_sumo_test.h"
#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "d/d_item_data.h"
#include "d/d_meter2_info.h"
#include "d/actor/d_a_player.h"
#include "d/actor/d_a_alink.h"
#include "dusk/settings.h"
#include "f_op/f_op_overlap_mng.h"

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

dAlbwOutfitKind sPendingEquip = D_ALBW_OUTFIT_COUNT;
u8            sSyncedNativeClothes = 0xFF;
bool          sReloadPending        = false;
bool          sLastAppliedHat       = false;

bool requestClothesChange(int param) {
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr || player->getClothesChangeWaitTimer() != 0) {
        return false;
    }
    player->setClothesChange(param);
    return player->getClothesChangeWaitTimer() != 0;
}

bool isTargetStable(dAlbwOutfitKind kind) {
    if (kind == D_ALBW_OUTFIT_SUMO) {
        return dAlbwOutfit_isSumoWorn() && dAlbwSumoTest_isOutfitActive();
    }
    const int itemNo = itemNoForKind(kind);
    if (itemNo < 0) {
        return false;
    }
    return !dAlbwOutfit_isSumoWorn() && !dAlbwSumoTest_isOutfitActive() &&
           dComIfGs_getSelectEquipClothes() == (u8)itemNo && sSyncedNativeClothes == (u8)itemNo;
}

void applyTargetKind(dAlbwOutfitKind kind) {
    if (kind == D_ALBW_OUTFIT_SUMO) {
        dAlbwOutfit_setSumoWorn(true);
        dAlbwOutfit_debugLog("target SUMO");
        return;
    }

    const int itemNo = itemNoForKind(kind);
    if (itemNo < 0) {
        return;
    }

    const bool leavingSumo = dAlbwSumoTest_isOutfitActive();
    dAlbwOutfit_setSumoWorn(false);
    if (leavingSumo) {
        // Clear the sumo model flags BEFORE the clothes change.  setClothesChange(0)
        // does not clear FLG2_UNK_200000, and if FLG2_UNK_80000 is still set when the
        // change fires, loadModelDVD takes its skip-path and never reloads the target
        // clothes arc -> changeLink builds the native body from a non-resident arc ->
        // crash (e.g. rapid Zora->Sumo->Ordon).  This must run here, ahead of setCloth,
        // because setCloth can drive the change before syncLinkModel's revert runs.
        daAlink_c* link = daAlink_getAlinkActorClass();
        if (link != NULL) {
            link->offNoResetFlg2(daAlink_c::FLG2_UNK_200000);
            link->offNoResetFlg2(daAlink_c::FLG2_UNK_80000);
        }
    }
    if (dComIfGs_isItemFirstBit(itemNo) == 0) {
        dComIfGs_onItemFirstBit(itemNo);
    }
    dMeter2Info_setCloth((u8)itemNo, false);
    if (leavingSumo) {
        dAlbwSumoTest_onNativeOutfitEquipped();
        dAlbwOutfit_debugLog("target native kind=%d item=%d leaving sumo", (int)kind, itemNo);
    } else {
        dAlbwOutfit_debugLog("target native kind=%d item=%d", (int)kind, itemNo);
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
        return false;
    }

    if (dAlbwOutfit_isStageTransitionUnsafe()) {
        dAlbwOutfit_debugLog("equip kind=%d blocked stage", (int)kind);
        return false;
    }

    if (isTargetStable(kind)) {
        return true;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    const bool busy = (player != nullptr && player->getClothesChangeWaitTimer() != 0) ||
                      dAlbwOutfit_isSwapInProgress();
    if (busy) {
        if (sPendingEquip == kind) {
            return false;
        }
        sPendingEquip = kind;
        dAlbwOutfit_debugLog("equip kind=%d queued", (int)kind);
        return true;
    }

    applyTargetKind(kind);
    sPendingEquip = D_ALBW_OUTFIT_COUNT;
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

bool dAlbwOutfit_isStageTransitionUnsafe() {
    if (dComIfGp_isEnableNextStage() || fopOvlpM_IsDoingReq()) {
        return true;
    }

    if (dComIfGp_isPauseFlag() || dComIfGp_getMesgStatus() != 0) {
        return true;
    }

    const int heapLock = dComIfGp_isHeapLockFlag();
    if (heapLock != 0 && heapLock != 5) {
        return true;
    }

    return false;
}

bool dAlbwOutfit_canTouchLinkModel() {
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr) {
        return false;
    }

    if (dAlbwOutfit_isStageTransitionUnsafe()) {
        return false;
    }

    if (player->getClothesChangeWaitTimer() != 0) {
        return false;
    }

    return true;
}

bool dAlbwOutfit_isSwapInProgress() {
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player != nullptr && player->getClothesChangeWaitTimer() != 0) {
        return true;
    }
    if (sReloadPending) {
        return true;
    }
    daAlink_c* link = daAlink_getAlinkActorClass();
    if (link != NULL && dAlbwOutfit_isSumoWorn() != dAlbwSumoTest_isOutfitActive()) {
        return true;
    }
    const u8 clothes = dComIfGs_getSelectEquipClothes();
    if (!dAlbwOutfit_isSumoWorn() && !dAlbwSumoTest_isOutfitActive() && sSyncedNativeClothes != 0xFF &&
        clothes != sSyncedNativeClothes) {
        return true;
    }
    return false;
}

void dAlbwOutfit_onStageTransitionBegin() {
    sSyncedNativeClothes = 0xFF;
    sReloadPending       = false;
    sLastAppliedHat      = false;
}

void dAlbwOutfit_syncLinkModel(daAlink_c* link) {
    if (link == NULL) {
        return;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    const int clothesTimer = player != NULL ? player->getClothesChangeWaitTimer() : 0;
    const bool want    = dAlbwOutfit_isSumoWorn();
    const bool has     = dAlbwSumoTest_isOutfitActive();
    const u8   clothes = dComIfGs_getSelectEquipClothes();
    const bool hat     = dusk::getSettings().game.sumoOutfitHat.getValue();

    if (clothesTimer != 0) {
        sReloadPending = true;
        return;
    }

    if (sReloadPending) {
        sReloadPending = false;
        // Record the synced native clothes once a change settles.  Gate on !want
        // only (not !has): when leaving sumo, FLG2_UNK_80000 can lag the clothes
        // timer by a frame, and skipping the record there would let the next frame
        // fire a redundant second setClothesChange(0).  !want means we're committed
        // to native, so `clothes` is the correct synced value regardless of FLG2.
        if (!want) {
            sSyncedNativeClothes = clothes;
        }
        return;
    }

    if (sSyncedNativeClothes == 0xFF && !want && !has) {
        sSyncedNativeClothes = clothes;
    }

    const bool sumoStable   = want && has && hat == sLastAppliedHat;
    const bool nativeStable = !want && !has && clothes == sSyncedNativeClothes;

    if (sumoStable || nativeStable) {
        return;
    }

    // Leaving sumo must win over applying it — applyTarget clears the worn bit while
    // FLG2_UNK_80000 is still set until this revert finishes.
    if (has && !want) {
        // setClothesChange(0) does NOT clear FLG2_UNK_200000 (sumo trigger), so a
        // plain revert would leave FLG2_UNK_280000 set and loadModelDVD would take
        // the sumo skip-path: it never reloads the target clothes arc, so changeLink
        // builds the native body from a non-resident arc -> initModel(NULL) crash.
        // Clear both sumo flags first so loadModelDVD runs its NORMAL clothes path
        // (resDelete + freeAll(mpArcHeap) + setArcName + reload) -- the robust vanilla
        // outfit-switch path that loads the arc itself and self-heals dangling regs.
        link->offNoResetFlg2(daAlink_c::FLG2_UNK_200000);
        link->offNoResetFlg2(daAlink_c::FLG2_UNK_80000);
        if (!requestClothesChange(0)) {
            return;
        }
        sLastAppliedHat = false;
        sReloadPending  = true;
        dAlbwOutfit_debugLog("sync revert sumo cloth=%d", clothes);
        return;
    }

    if (want) {
        if (!dAlbwSumoTest_prepareChangeLink()) {
            return;
        }
        if (!requestClothesChange(1)) {
            return;
        }
        sLastAppliedHat = hat;
        sReloadPending  = true;
        dAlbwOutfit_debugLog("sync apply sumo hat=%d", hat ? 1 : 0);
        return;
    }

    if (clothes != sSyncedNativeClothes) {
        if (!requestClothesChange(0)) {
            return;
        }
        sReloadPending = true;
        dAlbwOutfit_debugLog("sync native cloth=%d", clothes);
    }
}

void dAlbwOutfit_processPendingEquip() {
    if (sPendingEquip >= D_ALBW_OUTFIT_COUNT) {
        return;
    }

    if (dAlbwOutfit_isStageTransitionUnsafe()) {
        return;
    }

    if (dAlbwOutfit_isSwapInProgress()) {
        return;
    }

    const dAlbwOutfitKind pending = sPendingEquip;
    sPendingEquip = D_ALBW_OUTFIT_COUNT;
    dAlbwOutfit_equip(pending);
}

#endif  // TARGET_PC
