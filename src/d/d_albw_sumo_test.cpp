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
// Limitation (acceptable for a dev spike): the sumo flag is one-shot, so the
// look reverts on a stage transition — re-toggle to re-apply.
// ============================================
#include "d/d_albw_sumo_test.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
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

// alSumou body-model resource index; matches daAlink_c::changeLink().
constexpr int kSumoBodyResIdx = 0x31;

// Hero's Clothes archive — holds al_head.bmd (the Link cap).  Kept resident on
// demand so the cap renders no matter what clothes the player wears underneath.
constexpr const char* kCapArcName = "Kmdl";

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

    if (!want) {
        if (sApplied) {
            i_link->setClothesChange(0);  // revert to the player's equipped clothes
            sApplied = false;
        }
        sShowWeapons = false;
        return;
    }

    const bool hat   = dusk::getSettings().game.sumoOutfitHat.getValue();
    const bool fists = dusk::getSettings().game.sumoOutfitFists.getValue();

    // Keep the player's base-clothes arc resident the whole time the outfit is on.
    // While the sumo model flag is set the game skips reloading the clothes arc on
    // a clothes change, so changeLink would otherwise build from an unloaded
    // archive and crash.  When the equipped clothes change, reset the phase to
    // load the NEW arc and force a clean sumo re-apply over it.
    const u8 clothes = dComIfGs_getSelectEquipClothes();
    if (clothes != sLastClothes) {
        cPhs_Reset(&sClothesPhase);
        sLastClothes = clothes;
        sApplied     = false;  // re-apply the sumo over the new clothes
    }
    const int baseState = dComIfG_resLoad(&sClothesPhase, baseClothesArc());

    // The Link cap (al_head.bmd) lives in Kmdl; keep it resident too so the cap's
    // textures bind regardless of the base outfit.
    const int capState = hat ? dComIfG_resLoad(&sKmdlPhase, kCapArcName) : cPhs_COMPLEATE_e;

    // Keep mArcName synced to the current (resident) clothes arc EVERY frame.  The
    // sumo clothes-change uses the loader's skip path, which never refreshes
    // mArcName, so otherwise changeLink can load the face/boots from a stale arc
    // and crash — most visibly when rapidly switching outfits (e.g. Magic Armor),
    // where a re-apply-time sync can land while the wear flags are mid-transition.
    // Only point it at the new arc once that arc is actually resident.
    if (baseState == cPhs_COMPLEATE_e) {
        i_link->setArcName(0);
    }

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

#endif  // TARGET_PC
