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
#include "d/d_albw_wardrobe.h"
#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "d/d_item_data.h"
#include "d/d_meter2_info.h"
#include "d/actor/d_a_player.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_e_nz.h"
#include "dusk/action_bindings.h"
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
// The Cap Wear mode applied at the last model rebuild.  syncLinkModel compares it to the live
// game.capWear so a cap change (in EITHER sumo or native) forces exactly one rebuild; resets to
// Off on stage transition / leave so the cap re-applies on the next build.
dusk::CapWearMode sLastAppliedCap   = dusk::CapWearMode::Off;
// Set once a native cap rebuild has been ATTEMPTED with the donor ready (prepareNativeCapChange
// true) for the current (cap, base).  If the cap still fails to resolve (an arc that aliases its
// base's heap slot, e.g. green over Magic), this stops the self-heal from looping a rebuild every
// frame -- one attempt, then keep the native hat.  Reset when the cap mode or base changes.
bool          sNativeCapTried       = false;
// Set when leaving sumo via applyTargetKind (which clears FLG2_UNK_80000 up front,
// so `has` reads false and syncLinkModel's revert branch is skipped).  Forces ONE
// model rebuild even when the underlying clothes value is unchanged (sumo over the
// SAME base, e.g. Sumo+Ordon) — otherwise nativeStable early-returns and the sumo
// model stays on screen though the save says native.  Cleared once the reload fires.
bool          sLeavingSumoReload    = false;
int           sOutfitReloadCooldown = 0;
bool          sApplySumoLeaveCooldown = false;
bool          sPostMetamorphoseReapply = false;

constexpr int kReloadCooldownNormal    = 10;
constexpr int kReloadCooldownAfterSumo = 18;

void tickReloadCooldown() {
    if (sOutfitReloadCooldown > 0) {
        sOutfitReloadCooldown--;
    }
}

bool requestClothesChange(int param) {
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr || player->getClothesChangeWaitTimer() != 0) {
        return false;
    }
    // Do NOT pre-call setArcName() here.  loadModelDVD() owns the reload sequence
    // resDelete(OLD mArcName) -> freeAll -> setArcName(NEW) -> resLoad(NEW), and it reads
    // mArcName at timer==2 to identify the arc to delete.  Repointing mArcName to the
    // target arc before the timer runs makes resDelete target the NEW arc, so the OLD
    // arc's dRes_info_c slot is never released (count stays >0) while freeAll() frees its
    // heap -> the next change that reuses that arc name double-frees the dangling solid
    // heap (JKRHeap::destroy on 0xffff... in ~dRes_info_c -> crash on native cross-base
    // cycling, e.g. Ordon<->Hero's).  The pre-repoint was a Strategy-A band-aid for
    // cross-base SUMO leaves; the peel rule (getNextOwned/equip + applyTargetKind
    // decompose) removes those, so vanilla loadModelDVD's own setArcName timing is correct.
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

// True when equipping `kind` would merely peel the sumo overlay off onto the base
// already equipped on Link (same native arc, no switch).  This is the ONLY equip that
// bypasses the ownership / active-wardrobe gate: the base physically on Link is always
// peelable, even when Postman-stored.  Used by dAlbwOutfit_equip + getNextOwned so a
// stored base under the overlay never bounces the cycle into a cross-base leave.
bool isSumoPeel(dAlbwOutfitKind kind) {
    if (kind == D_ALBW_OUTFIT_SUMO || !dAlbwSumoTest_isOutfitActive()) {
        return false;
    }
    const int itemNo = itemNoForKind(kind);
    return itemNo >= 0 && (u8)itemNo == dComIfGs_getSelectEquipClothes();
}

void applyTargetKind(dAlbwOutfitKind kind) {
    if (kind == D_ALBW_OUTFIT_SUMO) {
        dAlbwOutfit_setSumoWorn(true);
        sLeavingSumoReload = false;  // re-applying sumo cancels any pending leave-reload
        dAlbwOutfit_debugLog("target SUMO");
        return;
    }

    const int itemNo = itemNoForKind(kind);
    if (itemNo < 0) {
        return;
    }

    const bool leavingSumo = dAlbwSumoTest_isOutfitActive();
    dAlbwOutfit_setSumoWorn(false);
    daAlink_c* link = daAlink_getAlinkActorClass();
    if (leavingSumo) {
        // Clear the sumo model flags BEFORE the clothes change.  setClothesChange(0)
        // does not clear FLG2_UNK_200000, and if FLG2_UNK_80000 is still set when the
        // change fires, loadModelDVD takes its skip-path and never reloads the target
        // clothes arc -> changeLink builds the native body from a non-resident arc ->
        // crash (e.g. rapid Zora->Sumo->Ordon).  This must run here, ahead of setCloth,
        // because setCloth can drive the change before syncLinkModel's revert runs.
        if (link != NULL) {
            link->offNoResetFlg2(daAlink_c::FLG2_UNK_200000);
            link->offNoResetFlg2(daAlink_c::FLG2_UNK_80000);
        }
        // Clearing FLG2_UNK_80000 makes `has` read false next frame, so syncLinkModel's
        // revert branch is skipped; flag a forced rebuild so the sumo model is actually
        // replaced even when the underlying clothes value didn't change.
        sLeavingSumoReload    = true;
        sApplySumoLeaveCooldown = true;
    }
    if (dComIfGs_isItemFirstBit(itemNo) == 0) {
        dComIfGs_onItemFirstBit(itemNo);
    }
    dMeter2Info_setCloth((u8)itemNo, false);
    if (leavingSumo && link != NULL) {
        link->setArcName(0);
    }
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
    // A peel (removing the sumo overlay onto the base already on Link) is always allowed
    // — even if that base is unowned-by-bit or Postman-stored — because it is physically
    // equipped.  Every other equip respects ownership + the active wardrobe pool.
    if (!isSumoPeel(kind)) {
        if (!dAlbwOutfit_isOwned(kind)) {
            return false;
        }
        if (dusk::isDpadQuickSwapEnabled() && !dAlbwWardrobe_isActiveOutfit(kind)) {
            return false;
        }
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
        // F equip-side (Quick-Sumo Work.md) — DEBOUNCE-TO-LATEST, not a queue.
        // `sPendingEquip` is a single slot: a press while a change is in flight
        // OVERWRITES it with the newest target, so a mash resolves to the final outfit
        // and never stacks N reloads (the bl->al->zl->ml load storm).  processPendingEquip
        // drains this one latest target on settle.  Quick Swap's D-pad coalesce half
        // mirrors this: it should also collapse rapid Down presses to the latest ring
        // step rather than calling equip() for every intermediate.
        if (sPendingEquip == kind) {
            return false;
        }
        sPendingEquip = kind;
        dAlbwOutfit_debugLog("equip kind=%d queued", (int)kind);
        return true;
    }

    // Cross-base sumo leave -> never clear the overlay AND switch the native arc in one
    // step (that corrupts the resource manager -> crash on the next reload).  Decompose:
    // peel onto the current base now (same-base, no arc switch), then queue the real
    // target for a clean native->native step once the peel settles.  applyTargetKind is
    // reached only through here, so this guards every caller (D-pad, shop, storage-evict).
    if (dAlbwSumoTest_isOutfitActive() && kind != D_ALBW_OUTFIT_SUMO) {
        const int itemNo = itemNoForKind(kind);
        const u8 base = dComIfGs_getSelectEquipClothes();
        if (itemNo >= 0 && (u8)itemNo != base) {
            const dAlbwOutfitKind baseKind = kindForClothes(base);
            applyTargetKind(baseKind);   // same-base peel
            sPendingEquip = kind;        // finish the base change after the peel settles
            dAlbwOutfit_debugLog("cross-base leave -> peel base=%d pending=%d",
                                 (int)baseKind, (int)kind);
            return true;
        }
    }

    applyTargetKind(kind);
    sPendingEquip = D_ALBW_OUTFIT_COUNT;
    return true;
}

dAlbwOutfitKind dAlbwOutfit_getNextOwned(dAlbwOutfitKind current) {
    // Sumo is a normal ring entry: from SUMO the scan returns the next owned native,
    // and applyTargetKind's decompose makes the (possibly cross-base) leave crash-safe.
    // NOTE: do NOT special-case SUMO to "peel to the current base" here — that creates an
    // absorbing Sumo<->base 2-cycle (peel back to base, ring forward to Sumo, repeat) that
    // strands every other outfit.  Cross-base leaves are safe now that the loadModelDVD
    // setArcName double-free is fixed (see requestClothesChange), so the ring can advance.
    //
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
        const bool inPool = dusk::isDpadQuickSwapEnabled() ? dAlbwWardrobe_isActiveOutfit(cand)
                                                           : dAlbwOutfit_isOwned(cand);
        if (inPool) {
            return cand;
        }
    }
    return current;
}

bool dAlbwOutfit_isSwapBlockedState() {
    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr) {
        return false;
    }
    // Iron boots + depowered (heavy) Magic Armor — the vanilla heavy/slow-movement
    // check, used all over the engine.  In these states Link's position is driven by
    // a scripted/heavy action, and the quick-swap clothes-change rebuild launches him.
    if (player->checkBootsOrArmorHeavy()) {
        return true;
    }
    // ALBW item-lockout slow phase.  This flag is set when lockout begins and clears
    // once the meter recovers to its base threshold (the "regain normal movement"
    // point) — so it tracks exactly the slowed window, not the whole lockout.
    if (dMeter2_isALBWMovementExhausted()) {
        return true;
    }
    // Ghoul Rats (E_NZ) latched onto Link.  Blocks ONLY when a rat is actually stuck
    // on him (any stick slot occupied) — not merely nearby — via the rat's own stick
    // bitfield.  The bitfield self-clears on detach/death/room-unload.
    if (dE_NZ_isRatStuckOnPlayer()) {
        return true;
    }
    return false;
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
    if (sOutfitReloadCooldown > 0) {
        return true;
    }
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
    sSyncedNativeClothes   = 0xFF;
    sReloadPending         = false;
    sLastAppliedCap        = dusk::CapWearMode::Off;
    sNativeCapTried        = false;
    sLeavingSumoReload     = false;
    sPostMetamorphoseReapply = false;
    sOutfitReloadCooldown  = 0;
    sApplySumoLeaveCooldown = false;
}

void dAlbwOutfit_onMetamorphoseToHuman(daAlink_c* link) {
    if (link == NULL) {
        return;
    }

    // Metamorphose uses changeLink(0) while FLG1_IS_WOLF may still be set; the draw guard
    // token is fixed at stamp time, but outfit sync state can still be stale vs the target.
    sSyncedNativeClothes = 0xFF;
    sReloadPending = false;
    sLeavingSumoReload = false;
    sOutfitReloadCooldown = 0;
    sPostMetamorphoseReapply = true;
}

void dAlbwOutfit_syncLinkModel(daAlink_c* link) {
    if (link == NULL) {
        return;
    }

    tickReloadCooldown();

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    const int clothesTimer = player != NULL ? player->getClothesChangeWaitTimer() : 0;
    const bool want    = dAlbwOutfit_isSumoWorn();
    const bool has     = dAlbwSumoTest_isOutfitActive();
    const u8   clothes = dComIfGs_getSelectEquipClothes();
    const dusk::CapWearMode cap = dusk::getSettings().game.capWear.getValue();

    if (clothesTimer != 0) {
        if (!sPostMetamorphoseReapply) {
            sReloadPending = true;
        }
        return;
    }

    // No outfit-driven clothes reloads during metamorphose — loadModelDVD's meta branch
    // calls changeWolf() when !checkWolf(), undoing wolf->human mid-animation.
    if (link->checkMetamorphoseProcActive()) {
        return;
    }

    if (sPostMetamorphoseReapply) {
        sPostMetamorphoseReapply = false;
        if (want) {
            if (dAlbwSumoTest_prepareChangeLink() && requestClothesChange(1)) {
                sLastAppliedCap = cap;
                sReloadPending = true;
                dAlbwOutfit_debugLog("meta->human reapply sumo");
            }
            return;
        }

        link->offNoResetFlg2(daAlink_c::FLG2_UNK_200000);
        link->offNoResetFlg2(daAlink_c::FLG2_UNK_80000);
        sLeavingSumoReload = true;
        if (requestClothesChange(0)) {
            sLastAppliedCap = cap;
            sReloadPending = true;
            dAlbwOutfit_debugLog("meta->human reapply native cloth=%d", clothes);
        }
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
        sOutfitReloadCooldown = sApplySumoLeaveCooldown ? kReloadCooldownAfterSumo
                                                        : kReloadCooldownNormal;
        sApplySumoLeaveCooldown = false;
        // Outcome trace: a settled swap's final state.  Pairs with the "target ..."
        // line so a stuck leave (target native, then no rebuild) is visible directly.
        dAlbwOutfit_debugLog("settled want=%d has=%d clothes=%d synced=%d cooldown=%d",
                             want ? 1 : 0, has ? 1 : 0, clothes, sSyncedNativeClothes,
                             sOutfitReloadCooldown);
        return;
    }

    // ============================================
    // C (Quick-Sumo Work.md): keep the native-clothes tracker valid UNDER the overlay.
    // onStageTransitionBegin resets it to 0xFF; if we only re-seed while !want && !has,
    // a transition / warp / cutscene taken WHILE sumo is worn leaves it stuck at 0xFF
    // (the `synced=255` in the trace), which then forces a spurious `leave force-reload`
    // on the next peel.  getSelectEquipClothes() is the BASE clothes and is valid
    // regardless of the overlay, so seed it whenever it sits at the sentinel.  This
    // never masks a needed leave reload: a same-base leave still rebuilds via
    // sLeavingSumoReload (nativeStable requires !sLeavingSumoReload).  We are past the
    // clothesTimer / sReloadPending early-returns here, so `clothes` is settled.
    // ============================================
    if (sSyncedNativeClothes == 0xFF) {
        sSyncedNativeClothes = clothes;
    }

    // The cap term keeps a deliberate Cap Wear change rebuilding the model so the cap
    // shows/hides/recolors live (the cap DOES render — see changeLink's sumo branch and
    // the non-sumo override).  Do NOT drop it for "idempotency": the multi-apply churn
    // under cutscene stress is `has` flapping (the demo clears FLG2_UNK_80000 each frame,
    // the module re-applies), not the cap — see the demo-aware re-apply work (item D).
    const bool sumoStable   = want && has && cap == sLastAppliedCap;
    // capSatisfied: a wanted cap (Green/Red/Blue/None) actually resolved on the last native build.
    // When changeLink fell back to the native hat (donor not resident yet, e.g. just after
    // crossing the owning-base boundary), this is false so nativeStable stays false and the
    // cap-apply branch below self-heals once the donor lands.  Off needs nothing resolved.
    const bool capSatisfied = cap == dusk::CapWearMode::Off || dAlbwAlink_nativeCapResolved();
    // nativeStable must also require no pending leave-reload: when sumo leaves onto the
    // SAME base, clothes is unchanged so this would otherwise be true and early-return
    // with the sumo model still on screen.  The cap terms force a native rebuild when the
    // global Cap Wear changes or a wanted cap hasn't resolved (the cap-apply branch below
    // does the actual reload).
    const bool nativeStable = !want && !has && clothes == sSyncedNativeClothes &&
                              !sLeavingSumoReload && cap == sLastAppliedCap && capSatisfied;

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
        sApplySumoLeaveCooldown = true;
        if (!requestClothesChange(0)) {
            return;
        }
        sLastAppliedCap    = cap;
        sReloadPending     = true;
        sLeavingSumoReload = false;
        dAlbwOutfit_debugLog("sync revert sumo cloth=%d", clothes);
        return;
    }

    // Same-base leave (applyTargetKind already cleared FLG2 -> has==0, so the revert
    // branch above is skipped).  Force ONE reload so the sumo model is actually
    // replaced by the native body even though the clothes value is unchanged.
    if (sLeavingSumoReload && !want) {
        if (!requestClothesChange(0)) {
            return;
        }
        sLeavingSumoReload = false;
        sLastAppliedCap    = cap;
        sReloadPending     = true;
        dAlbwOutfit_debugLog("leave force-reload cloth=%d", clothes);
        return;
    }

    if (want) {
        if (!dAlbwSumoTest_prepareChangeLink()) {
            return;
        }
        if (!requestClothesChange(1)) {
            return;
        }
        sLastAppliedCap = cap;
        sReloadPending  = true;
        dAlbwOutfit_debugLog("sync apply sumo cap=%d", (int)cap);
        return;
    }

    if (clothes != sSyncedNativeClothes) {
        if (!requestClothesChange(0)) {
            return;
        }
        sLastAppliedCap = cap;
        sNativeCapTried = false;  // new base -> allow a fresh cap attempt / self-heal
        sReloadPending = true;
        dAlbwOutfit_debugLog("sync native cloth=%d", clothes);
        return;
    }

    // GLOBAL Cap Wear (native): rebuild the model when the cap mode CHANGED, or when a wanted cap
    // FAILED TO RESOLVE on the last build (donor not resident yet -- e.g. just after crossing the
    // owning-base boundary, where Green's Kmdl / Red's Mmdl / Blue's Zmdl must load fresh).  Both
    // are gated on the cap donor being resident (prepareNativeCapChange), so changeLink's override
    // resolves instead of falling back -- and so a genuinely-unavailable arc can NEVER hang the
    // rebuild (we just keep the native hat and retry).  requestClothesChange(0) is the verified
    // build-then-swap path; once the cap resolves, nativeStable above stops this from re-firing.
    const bool capChanged = cap != sLastAppliedCap;
    if (capChanged) {
        sNativeCapTried = false;  // mode changed -> allow a fresh attempt
    }
    const bool capUnresolved = cap != dusk::CapWearMode::Off && !dAlbwAlink_nativeCapResolved();
    if (capChanged || (capUnresolved && !sNativeCapTried)) {
        if (!dAlbwSumoTest_prepareNativeCapChange()) {
            // Donor still loading, or the cap is not providable on this base (e.g. green over
            // Magic).  Return without latching/rebuilding: the legit self-heal retries once the
            // donor lands; the unprovidable case just keeps the native hat (no rebuild, no loop).
            return;
        }
        if (!requestClothesChange(0)) {
            return;
        }
        sLastAppliedCap = cap;
        sNativeCapTried = true;  // attempted with the donor ready; if it still fails, don't loop
        sReloadPending = true;
        dAlbwOutfit_debugLog("sync native cap apply cap=%d", (int)cap);
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

// ============================================
// NEW CODE — ALBW Port (Custom Models: rebuild the outfit after an overlay change)
// Arm the metamorphose-reapply path so the next syncLinkModel rebuilds the current
// outfit from freshly-mounted arcs.  The sumo branch waits on prepareChangeLink()
// (resourcesReady), so it only fires once the re-mounted alSumou is resident.
// ============================================
void dAlbwOutfit_forceReapply() {
    sPostMetamorphoseReapply = true;
}

#endif  // TARGET_PC
