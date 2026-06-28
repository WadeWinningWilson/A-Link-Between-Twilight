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

namespace {

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
    // Reset the phase whenever the equipped clothes change so resLoad targets the
    // NEW base arc.  Without this, leaving sumo to an outfit DIFFERENT from the one
    // worn underneath (e.g. sumo-over-Zora -> Ordon) returns a STALE COMPLETE for
    // the old arc: the revert then fires while changeLink reads the new mArcName
    // from an unloaded arc -> getObjectRes NULL -> face-model null deref crash.
    // (processPendingEquip changes the clothes after maintainResources' own reset
    // has already run for the frame, so the check must be self-correcting here.)
    const u8 clothes = dComIfGs_getSelectEquipClothes();
    if (clothes != sClothesPhaseFor) {
        cPhs_Reset(&sClothesPhase);
        sClothesPhaseFor = clothes;
    }
    return dComIfG_resLoad(&sClothesPhase, baseClothesArc()) == cPhs_COMPLEATE_e;
}

bool resourcesReady(bool needCap) {
    if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
        if (dComIfG_resLoad(&sPhase, "alSumou") != cPhs_COMPLEATE_e) {
            return false;
        }
    }
    if (!nativeClothesResourcesReady()) {
        return false;
    }
    if (needCap && dComIfG_resLoad(&sKmdlPhase, kCapArcName) != cPhs_COMPLEATE_e) {
        return false;
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
    if (!want && has) {
        nativeClothesResourcesReady();
        return;
    }

    if (want) {
        resourcesReady(hat);
        if (nativeClothesResourcesReady()) {
            i_link->setArcName(0);
        }
    }
}

}  // namespace

bool dAlbwSumoTest_prepareChangeLink() {
    if (!dAlbwOutfit_isSumoWorn()) {
        return false;
    }
    return resourcesReady(dusk::getSettings().game.sumoOutfitHat.getValue());
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
        sShowWeapons = false;
        return;
    }
    sInStageTransition = false;

    maintainResources(i_link);
    dAlbwOutfit_processPendingEquip();
    dAlbwOutfit_syncLinkModel(i_link);

    const bool want  = dAlbwOutfit_isSumoWorn();
    const bool has   = dAlbwSumoTest_isOutfitActive();
    const bool fists = dusk::getSettings().game.sumoOutfitFists.getValue();
    sShowWeapons = want && has && !fists;
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
