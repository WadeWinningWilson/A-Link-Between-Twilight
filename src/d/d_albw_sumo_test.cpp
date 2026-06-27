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

// Phased load handle for "alSumou" (zero-initialized at static init).
request_of_phase_process_class sPhase;

// alSumou body-model resource index; matches daAlink_c::changeLink().
constexpr int kSumoBodyResIdx = 0x31;

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

    // Re-apply (reload the model) if the Link-hat toggle changed while worn.
    if (sApplied && hat != sAppliedHat) {
        sApplied = false;
    }

    if (!sApplied) {
        // Ensure alSumou is resident before the swap (avoids the null-model crash
        // in changeLink()).  Phased load — retry next frame.
        if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
            dComIfG_resLoad(&sPhase, "alSumou");
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
