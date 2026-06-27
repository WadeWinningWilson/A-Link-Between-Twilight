// ============================================
// NEW CODE — ALBW Port (Sumo Link visual test)
// See d_albw_sumo_test.h.  Field-capable + crash-safe: the model swap is only
// triggered once "alSumou" is confirmed resident (loaded by Link here, not the
// wrestler), so changeLink() never builds a model from a null J3DModelData.
//
// Mechanism: setClothesChange(1) sets FLG2_UNK_200000; the existing per-frame
// loadModelDVD()/changeLink() path then loads the sumo body/hat/hands (res
// 0x31/0x33/0x32) from alSumou.  setClothesChange(0) reverts to the player's
// equipped clothes (getSelectEquipClothes()).  This drives ONLY the model
// flag — never setSumouReady() / the sumo minigame procedures.
//
// Persistence: a room/stage transition (and demo/cutscene Link rebuilds) clear
// the sumo model flag, so once an apply settles we detect FLG2_UNK_80000 missing
// and re-apply — the outfit survives transitions and shows through cutscenes.
// ============================================
#include "d/d_albw_sumo_test.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "dusk/settings.h"
#include "SSystem/SComponent/c_phase.h"

namespace {

bool sApplied     = false;  // sumo model currently swapped in
bool sAppliedHat  = false;  // hat sub-mode active when applied (to detect a hat toggle)
bool sShowWeapons = false;  // cached per frame: outfit on AND not fists-only

// Phased load handles (zero-initialized at static init).
request_of_phase_process_class sPhase;        // "alSumou" (sumo body/hands/topknot)
request_of_phase_process_class sKmdlPhase;    // "Kmdl" (Hero's arc — source of the cap)
request_of_phase_process_class sClothesPhase; // current base-clothes arc (kept resident)

// Last equipped-clothes id we kept resident.  Detects a clothes change so we can
// reload the NEW arc before the game's changeLink runs — while the sumo model
// flag is set the game skips reloading the clothes arc, so without this it would
// build models from an unloaded archive and crash.
u8 sLastClothes = 0xFF;

// Frames to wait after (re)applying before trusting the FLG2_UNK_80000 sumo flag,
// so we don't re-trigger while the clothes-change that SETS that flag is in flight.
int sApplyGrace = 0;

// alSumou body-model resource index; matches daAlink_c::changeLink().
constexpr int kSumoBodyResIdx = 0x31;

// Hero's Clothes archive — holds al_head.bmd (the Link cap).  Kept resident on
// demand so the cap renders no matter what clothes the player wears underneath.
constexpr const char* kCapArcName = "Kmdl";

// Save bits (dSv_event_flag_c::saveBitLabels) — continue the ALBW shop reservation
// block after kMultiShieldUpgradeBit (688); array size is 822.
constexpr int kSumoOwnedBit       = 689;  // Sumo Outfit purchased / stored
constexpr int kSumoWrestlerMetBit = 690;  // met the Ordon sumo-wrestler NPC

// Archive backing the player's currently equipped human clothes.  Mirrors
// daAlink_c::setArcName()'s mapping so we keep the right arc resident.
const char* baseClothesArc() {
    if (daAlink_c::checkCasualWearFlg())     return "Bmdl";
    if (daAlink_c::checkZoraWearFlg())       return "Zmdl";
    if (daAlink_c::checkMagicArmorWearFlg()) return "Mmdl";
    return "Kmdl";  // Hero's Clothes (default)
}

}  // namespace

void dAlbwSumoTest_exec(daAlink_c* i_link) {
    if (i_link == NULL) {
        return;
    }

    // Editor -> ALBW -> Sumo Outfit (game.sumoOutfit / *Hat / *Fists).
    const bool want = dusk::getSettings().game.sumoOutfit.getValue();

    // Nothing to manage during normal play (outfit off and not currently applied).
    if (!want && !sApplied) {
        sShowWeapons = false;
        return;
    }

    const bool hat   = dusk::getSettings().game.sumoOutfitHat.getValue();
    const bool fists = dusk::getSettings().game.sumoOutfitFists.getValue();

    // Keep the player's equipped base-clothes arc resident AND mArcName synced to
    // it every frame while the outfit is applied or being applied/reverted.  While
    // the sumo flag is set the clothes loader skips reloading the arc and never
    // refreshes mArcName, so otherwise changeLink builds the face/boots from a
    // stale or unloaded archive and crashes -- including the shop's swap from sumo
    // to a real outfit.  Reset the phase when the equipped clothes change so we
    // load the NEW arc; only point mArcName at it once it is resident.
    const u8 clothes = dComIfGs_getSelectEquipClothes();
    if (clothes != sLastClothes) {
        cPhs_Reset(&sClothesPhase);
        sLastClothes = clothes;
        if (want) {
            sApplied = false;  // re-apply the sumo over the new base
        }
    }
    const int baseState = dComIfG_resLoad(&sClothesPhase, baseClothesArc());
    if (baseState == cPhs_COMPLEATE_e) {
        i_link->setArcName(0);
    }

    if (!want) {
        // Reverting from sumo to the player's (possibly just-changed) clothes.
        // Wait until that arc is resident -- mArcName was synced above -- so the
        // revert's changeLink reads a valid archive instead of crashing.
        if (sApplied) {
            if (baseState != cPhs_COMPLEATE_e) {
                sShowWeapons = false;
                return;
            }
            i_link->setClothesChange(0);
            sApplied = false;
        }
        sShowWeapons = false;
        return;
    }

    // Persist the outfit across room/stage transitions.  A transition rebuilds
    // Link with the sumo model flag cleared, leaving our static sApplied stale; so
    // once the apply has settled (grace elapsed), detect the flag missing and
    // re-apply.  Without this the outfit would silently revert on every transition.
    if (sApplyGrace > 0) {
        sApplyGrace--;
    } else if (sApplied && !i_link->checkNoResetFlg2(daAlink_c::FLG2_UNK_80000)) {
        sApplied = false;
    }

    // The Link cap (al_head.bmd) lives in Kmdl; keep it resident too so the cap's
    // textures bind regardless of the base outfit.
    const int capState = hat ? dComIfG_resLoad(&sKmdlPhase, kCapArcName) : cPhs_COMPLEATE_e;

    // Re-apply (reload the model) if the Link-hat toggle changed while worn.
    if (sApplied && hat != sAppliedHat) {
        sApplied = false;
    }

    if (!sApplied) {
        // Ensure alSumou + the base-clothes/cap arcs are all resident before the
        // swap (avoids null-model crashes in changeLink()).  Retry next frame.
        if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
            dComIfG_resLoad(&sPhase, "alSumou");
            sShowWeapons = false;
            return;
        }
        if (baseState != cPhs_COMPLEATE_e || capState != cPhs_COMPLEATE_e) {
            sShowWeapons = false;
            return;
        }
        i_link->setClothesChange(1);
        sApplied    = true;
        sAppliedHat = hat;
        sApplyGrace = 30;  // let the clothes change set FLG2_UNK_80000 before we trust it
    }

    sShowWeapons = !fists;  // sApplied is true here
}

bool dAlbwSumoTest_isOutfitActive() {
    return sApplied;
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
    // Always listed so the shop works as an outfit switcher (re-select to wear).
    // The stored-armors ownership-hide and the sumo-wrestler / True-ALBW gating
    // are deferred follow-ups (the wrestler talk-hook crashed on the Ordon entry
    // event and was removed; the met/owned save bits 689/690 are still recorded).
    return true;
}

bool dAlbwSumoTest_tryPurchaseShop() {
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoOwnedBit]);  // own / store it
    dusk::getSettings().game.sumoOutfit.setValue(true);                   // wear it now
    return true;
}

void dAlbwSumoTest_clearWorn() {
    dusk::getSettings().game.sumoOutfit.setValue(false);  // buying a real outfit drops sumo
}

void dAlbwSumoTest_onWrestlerMet() {
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoWrestlerMetBit]);
}

#endif  // TARGET_PC
