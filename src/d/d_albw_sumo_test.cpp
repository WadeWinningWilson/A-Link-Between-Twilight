// ============================================
// NEW CODE — ALBW Port (Sumo Link visual test)
// See d_albw_sumo_test.h.
//
// Resource residency + changeLink() seeding live here; ALL setClothesChange traffic
// goes through dAlbwOutfit_syncLinkModel() (one reload at a time).
// ============================================
#include "d/d_albw_sumo_test.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "d/d_albw_outfit.h"
#include "dusk/settings.h"
#include "SSystem/SComponent/c_phase.h"

// ============================================
// TEMP INSTRUMENTATION — arc/model lifecycle trace (cycling-crash Step 1).
// Keep this toggle in sync with d_a_alink.cpp's D_ALBW_ARC_LIFECYCLE_DEBUG; STRIP
// (set to 0) before any upstream push.  Event-driven (donor free only), not per-frame.
// ============================================
#define D_ALBW_ARC_LIFECYCLE_DEBUG 1
#if D_ALBW_ARC_LIFECYCLE_DEBUG
#include "dusk/logging.h"  // DuskLog — OSReport is disabled in-game (OSReportDisable), DuskLog reaches the log
#endif

namespace {

#if D_ALBW_ARC_LIFECYCLE_DEBUG
// Live archive pointer registered for an arc NAME (NULL if not registered / freed).
void* albwArcArchive(const char* arc) {
    if (arc == NULL) return NULL;
    dRes_info_c* info = g_dComIfG_gameInfo.mResControl.getObjectResInfo(arc);
    return (info != NULL) ? (void*)info->getArchive() : NULL;
}
#endif

bool sShowWeapons         = false;
bool sInStageTransition   = false;
u8   sLastClothes         = 0xFF;
u8   sClothesPhaseFor     = 0xFF;  // equipped-clothes value sClothesPhase was (re)loaded for

request_of_phase_process_class sPhase;
request_of_phase_process_class sKmdlPhase;
request_of_phase_process_class sClothesPhase;

constexpr int kSumoBodyResIdx = 0x31;
constexpr const char* kCapArcName = "Kmdl";
constexpr int kSumoOwnedBit       = 689;
constexpr int kSumoWrestlerMetBit = 690;

const char* baseClothesArc() {
    if (daAlink_c::checkCasualWearFlg())     return "Bmdl";
    if (daAlink_c::checkZoraWearFlg())       return "Zmdl";
    if (daAlink_c::checkMagicArmorWearFlg()) return "Mmdl";
    return "Kmdl";
}

bool nativeClothesResourcesReady() {
    // DO NOT dual-load the base clothes arc.  Link's own mPhaseReq owns and keeps the
    // equipped-clothes arc resident (loaded at create / each native clothes change,
    // and retained through sumo's skip-path which never frees mpArcHeap).  A second
    // loader here (sClothesPhase) created DUPLICATE/dangling registrations of the same
    // arc name: when Link later reloaded e.g. Bmdl, getResInfoLoaded returned the
    // stale entry with a freed archive -> getObjectRes NULL -> initModel(NULL) crash
    // in changeLink (reproduced by sumo-over-Zora -> Ordon).  So just report ready;
    // residency is the clothes-change pipeline's responsibility (it gates on its own
    // resLoad COMPLETE before changeLink, and waits while clothesChangeWaitTimer != 0).
    return true;
}

// ============================================
// Drop the Kmdl face-donor held for a Zora base.
//
// Refcount-safe BY CONSTRUCTION: sKmdlPhase only ever holds Kmdl while the base
// clothes are Zora, where Link's clothes pipeline holds Zmdl in its own mpArcHeap
// and our Kmdl is an INDEPENDENT resource-manager entry.  decCount via resDelete
// therefore never frees memory out from under an mpArcHeap->freeAll() — the failure
// mode that made an unconditional Kmdl release double-free over a Hero's base (where
// the pipeline's Kmdl and a sumo-side Kmdl alias one refcount).
//
// NOTE (2026-06-30): an attempt to generalise this donor to ALL non-Hero's bases (so
// the Link Hat's al_head would resolve over Ordon/Magic too) was REVERTED — it churned
// Kmdl load/release across base switches and left mpLinkHatModel dangling at al_head
// when releaseFaceDonor freed Kmdl, which the shadow pass (addRealShadow) then drew ->
// J3DShape::drawFast crash on shop-buy-Zora / rapid switching.  Cap-on-all-bases is
// entangled with the dual-Kmdl problem and belongs to the model-agnostic cap redesign.
// ============================================
void releaseFaceDonor() {
    if (sKmdlPhase.id == 2) {
#if D_ALBW_ARC_LIFECYCLE_DEBUG
        // TEMP: log only the ACTUAL Kmdl donor free (id==2).  resourcesReady() calls this
        // every frame over a non-Zora base, where id==0 and we only cPhs_Reset — logging
        // that path was per-frame file I/O (the FPS regression).  STRIP before push.
        DuskLog.debug("ALBW-LIFE releaseFaceDonor FREE Kmdl donor (id==2) arc={}",
                      albwArcArchive(kCapArcName));
#endif
        dComIfG_resDelete(&sKmdlPhase, kCapArcName);  // decCount; clears id to 0
    } else {
        cPhs_Reset(&sKmdlPhase);                      // abandon any in-flight / idle request
    }
}

bool resourcesReady() {
    if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
        if (dComIfG_resLoad(&sPhase, "alSumou") != cPhs_COMPLEATE_e) {
            return false;
        }
    }
    // Face donor: the sumo body borrows al_face.bmd from the resident base arc.  Only
    // the Zora base (Zmdl) lacks al_face, so keep Kmdl resident as the face source for
    // that base ALONE (changeLink()'s sumo face block falls back to Kmdl only when
    // checkZoraWearFlg()).  Gating the Kmdl load to the Zora base is what prevents the
    // dual-Kmdl heap aliasing: over a Hero's base the pipeline's Kmdl lives in Link's
    // mpArcHeap and a second refcount here would dangle on that heap's freeAll().  (The
    // Link Hat's al_head therefore resolves only over Hero's/Zora for now — see the
    // releaseFaceDonor note; cap-on-all-bases waits on the model-agnostic cap work.)
    if (daAlink_c::checkZoraWearFlg()) {
        if (dComIfG_resLoad(&sKmdlPhase, kCapArcName) != cPhs_COMPLEATE_e) {
            return false;
        }
    } else {
        releaseFaceDonor();
    }
    return true;
}

void maintainResources(daAlink_c* i_link) {
    const bool want = dAlbwOutfit_isSumoWorn();
    const bool has  = dAlbwSumoTest_isOutfitActive();
    const bool hat  = dusk::getSettings().game.sumoOutfitHat.getValue();

    const u8 clothes = dComIfGs_getSelectEquipClothes();
    if (clothes != sLastClothes) {
        cPhs_Reset(&sClothesPhase);
        sLastClothes = clothes;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player != nullptr && player->getClothesChangeWaitTimer() != 0) {
        return;
    }

    // Leaving sumo: preload the target native arc only — do not touch setArcName
    // while the sumo body is still on Link (loadModelDVD fights that and crashes).
    // Keep the Zora face donor resident here: the sumo body flag is still set, so
    // changeLink may still read its face from Kmdl for one more frame.
    if (!want && has) {
        nativeClothesResourcesReady();
        return;
    }

    if (want) {
        (void)hat;  // hat (Link cap) is parked; resourcesReady gates Kmdl on the base
        resourcesReady();
        if (nativeClothesResourcesReady()) {
            i_link->setArcName(0);
        }
    } else {
        // Fully native (sumo off and body flag cleared): drop the Zora face donor if
        // we were still holding it.
        releaseFaceDonor();
    }
}

}  // namespace

bool dAlbwSumoTest_prepareChangeLink() {
    if (!dAlbwOutfit_isSumoWorn()) {
        return false;
    }
    return resourcesReady();
}

bool dAlbwSumoTest_prepareNativeClothesChange() {
    return nativeClothesResourcesReady();
}

void dAlbwSumoTest_onNativeOutfitEquipped() {
    sShowWeapons = false;
}

bool dAlbwSumoTest_isSwapPending() {
    return dAlbwOutfit_isSwapInProgress();
}

void dAlbwSumoTest_exec(daAlink_c* i_link) {
    if (i_link == NULL) {
        return;
    }

    dAlbwOutfit_syncWornOwnership();

    // ============================================
    // NEW CODE — ALBW Port (death turns the sumo overlay OFF; user decision 2026-06-29)
    // The sumo worn-bit (700) is a per-save event bit, so without this it survives a game-over
    // and Link respawns in the sumo overlay — and a post-respawn vanilla-menu armor switch then
    // crashes (statusWindowExecute -> changeLink builds the sumo skip-path from a non-resident
    // arc -> initModel(NULL)).  On game-over, clear ONLY the worn bit and let the normal sync
    // LEAVE path (syncLinkModel's `has && !want` branch) do the proper teardown — clear the FLG2
    // flags AND rebuild the model to native together, keeping the model/flags/statics in sync.
    // (An earlier version also cleared FLG2_80000 directly here; that desynced the state — model
    // stayed sumo while the flag read native, so the draw guard skipped Link = the broken hybrid
    // sumo after a cycle.  Do NOT clear FLG2 here.)  Idempotent; only on a real game-over.
    // ============================================
    if (i_link->checkGameOverWindow() && dAlbwOutfit_isSumoWorn()) {
        dAlbwOutfit_setSumoWorn(false);
    }

    if (dAlbwOutfit_isStageTransitionUnsafe()) {
        if (!sInStageTransition) {
            sInStageTransition = true;
            cPhs_Reset(&sPhase);
            cPhs_Reset(&sKmdlPhase);
            cPhs_Reset(&sClothesPhase);
            sLastClothes     = 0xFF;
            sClothesPhaseFor = 0xFF;
            dAlbwOutfit_onStageTransitionBegin();
        }
        // Keep the gear visible through the transition based on the persistent worn
        // intent (not the live model flag), so sumo's weapons/items don't blink out
        // while Link re-creates and re-applies sumo on the new stage.
        sShowWeapons = dAlbwOutfit_isSumoWorn() &&
                       !dusk::getSettings().game.sumoOutfitFists.getValue();
        return;
    }
    sInStageTransition = false;

    maintainResources(i_link);
    dAlbwOutfit_processPendingEquip();
    dAlbwOutfit_syncLinkModel(i_link);

    const bool want  = dAlbwOutfit_isSumoWorn();
    const bool fists = dusk::getSettings().game.sumoOutfitFists.getValue();
    // Drive weapon visibility from the persistent worn intent, NOT the live model
    // flag: requiring isOutfitActive() (FLG2_UNK_80000) re-hid the gear during the
    // post-transition / re-apply window where the worn bit is set but the sumo body
    // flag hasn't landed yet -> the disappearing-items flicker.  showWeapons() only
    // DROPS the sumo-specific draw suppression, so native play (sumo bit never set)
    // and other suppression reasons are unaffected.
    sShowWeapons = want && !fists;
}

bool dAlbwSumoTest_isOutfitActive() {
    daAlink_c* link = daAlink_getAlinkActorClass();
    return link != NULL && link->checkNoResetFlg2(daAlink_c::FLG2_UNK_80000);
}

bool dAlbwSumoTest_showWeapons() {
    return sShowWeapons;
}

bool dAlbwSumoTest_wantLinkCap() {
    return dusk::getSettings().game.sumoOutfitHat.getValue();
}

bool dAlbwSumoTest_isOwned() {
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[kSumoOwnedBit]) != 0;
}

bool dAlbwSumoTest_isShopEligible() {
    return true;
}

bool dAlbwSumoTest_tryPurchaseShop() {
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoOwnedBit]);
    dAlbwOutfit_setSumoWorn(true);
    return true;
}

void dAlbwSumoTest_clearWorn() {
    dAlbwOutfit_setSumoWorn(false);
}

void dAlbwSumoTest_onWrestlerMet() {
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoWrestlerMetBit]);
}

#endif  // TARGET_PC
