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
#include "d/d_resorce.h"
#include "d/d_save.h"
#include "d/d_albw_outfit.h"
#include "d/d_albw_outfit_stats.h"
#include "dusk/settings.h"
#include "SSystem/SComponent/c_phase.h"
#include "m_Do/m_Do_ext.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"

// ============================================
// DEBUG — model-agnostic cap loader (Phase 2).  Logs the private mount/load lifecycle so the
// independent cap path is diagnosable in-game.  STRIP (set 0) before any upstream push.
// ============================================
#define D_ALBW_CAP_DEBUG 1
#if D_ALBW_CAP_DEBUG
#include "dusk/logging.h"  // DuskLog (OSReport is disabled in-game; DuskLog reaches the log)
#endif

// ============================================
// TEMP INSTRUMENTATION — arc/model lifecycle trace (cycling-crash Step 1).
// Keep this toggle in sync with d_a_alink.cpp's D_ALBW_ARC_LIFECYCLE_DEBUG; STRIP
// (set to 0) before any upstream push.  Event-driven (donor free only), not per-frame.
// ============================================
#define D_ALBW_ARC_LIFECYCLE_DEBUG 0
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
request_of_phase_process_class sCapPhase;  // red/blue cap donor (Mmdl/Zmdl), held independent of base
int sCapDonorKind = 0;                       // arc sCapPhase holds: 0 none, 1 Mmdl (red), 2 Zmdl (blue)

constexpr int kSumoBodyResIdx = 0x31;
constexpr const char* kCapArcName = "Kmdl";
constexpr int kSumoOwnedBit       = 689;
constexpr int kSumoWrestlerMetBit = 690;

// ============================================
// NEW CODE — ALBW Port (GLOBAL Cap Wear — model-agnostic independent cap loader, Phase 2)
// Load each cap head model (al_head/Kmdl, ml_head/Mmdl, zl_head/Zmdl) through the ENGINE'S OWN
// loader (mount + loaderBasicBmd + textures) but into a PRIVATE dRes_info_c array (s_capInfo) and a
// private heap (s_capHeap) -- NOT the clothes pipeline's mObjectInfo / mpArcHeap.  dRes_control_c's
// static setRes/syncRes/getRes search ONLY the array we pass, so this entry is invisible to the
// pipeline and its mpArcHeap->freeAll() never frees it.  Result: the cap J3DModelData is persistent
// and BASE-INDEPENDENT -- the chosen cap resolves on EVERY base with zero aliasing (the alSumou-style
// isolation that makes the topknot bulletproof).  Loaded once on demand, kept for the session (never
// released during play -> no deleteArchiveRes release-churn crash).  Fail-safe at every step: any
// failure leaves s_capData NULL so changeLink keeps the native hat (never builds from NULL).
// ============================================
constexpr int kCapKindNum = 3;  // 0 green/Kmdl/al_head, 1 red/Mmdl/ml_head, 2 blue/Zmdl/zl_head
const char* const kCapKindArc[kCapKindNum] = {"Kmdl", "Mmdl", "Zmdl"};
const char* const kCapKindBmd[kCapKindNum] = {"al_head.bmd", "ml_head.bmd", "zl_head.bmd"};
constexpr u32 kCapHeapSize = 0x100000;  // 1 MB / cap arc (clothes arc ~651 KB + data); fail-safe if short

dRes_info_c   s_capInfo[kCapKindNum];                  // PRIVATE array (NOT the pipeline's mObjectInfo)
JKRSolidHeap* s_capHeap[kCapKindNum]  = {NULL, NULL, NULL};
J3DModelData* s_capData[kCapKindNum]  = {NULL, NULL, NULL};
u8            s_capState[kCapKindNum] = {0, 0, 0};     // 0 idle, 1 loading, 2 ready, 3 failed

// game.capWear -> cap kind index, or -1 for Off/None (no independent cap).
int capWearKind() {
    switch (dusk::getSettings().game.capWear.getValue()) {
        case dusk::CapWearMode::Green: return 0;
        case dusk::CapWearMode::Red:   return 1;
        case dusk::CapWearMode::Blue:  return 2;
        default:                       return -1;  // Off / None
    }
}

// Drive the independent load for one cap kind one step.  Returns the model data when ready, else NULL.
J3DModelData* ensureIndependentCap(int kind) {
    if (kind < 0 || kind >= kCapKindNum) return NULL;
    if (s_capState[kind] == 2) return s_capData[kind];  // ready
    if (s_capState[kind] == 3) return NULL;             // failed (don't retry-spam / re-mount)

    if (s_capHeap[kind] == NULL) {
        s_capHeap[kind] = mDoExt_createSolidHeapFromGame(kCapHeapSize, 0x20);
        if (s_capHeap[kind] == NULL) {
#if D_ALBW_CAP_DEBUG
            DuskLog.debug("ALBW-CAPLOAD kind={} arc={} HEAP ALLOC FAILED ({} bytes)", kind,
                          kCapKindArc[kind], (u32)kCapHeapSize);
#endif
            s_capState[kind] = 3;
            return NULL;
        }
    }

    // loadResource() (run inside syncRes when the mount completes) allocates the model's aux data
    // -- the J3DModelData, material anims, and shared display list -- from the CURRENT heap, not the
    // heap passed to setRes.  Make our PRIVATE heap current across the load steps so all of it lands
    // in s_capHeap[kind] (persistent, pipeline-isolated); a transient current heap would dangle.
    JKRHeap* const prevHeap = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_capHeap[kind]);
    J3DModelData* result = NULL;

    bool failed = false;
    if (s_capState[kind] == 0) {
        // Kick the mount ONCE (setRes incCounts on repeat -> never call it per-frame while loading).
        if (dRes_control_c::setRes(kCapKindArc[kind], s_capInfo, kCapKindNum, "/res/Object/", 0,
                                   s_capHeap[kind]) == 0) {
#if D_ALBW_CAP_DEBUG
            DuskLog.debug("ALBW-CAPLOAD kind={} arc={} setRes FAILED", kind, kCapKindArc[kind]);
#endif
            s_capState[kind] = 3;
            failed = true;
        } else {
            s_capState[kind] = 1;
        }
    }

    if (!failed && s_capState[kind] == 1) {
        // Poll until the mount + resource load completes (syncRes: >0 loading, 0 done, <0 error).
        const int sync = dRes_control_c::syncRes(kCapKindArc[kind], s_capInfo, kCapKindNum);
        if (sync < 0) {
#if D_ALBW_CAP_DEBUG
            DuskLog.debug("ALBW-CAPLOAD kind={} arc={} syncRes ERROR={}", kind, kCapKindArc[kind],
                          sync);
#endif
            s_capState[kind] = 3;
        } else if (sync == 0) {
            J3DModelData* md = static_cast<J3DModelData*>(
                dRes_control_c::getRes(kCapKindArc[kind], kCapKindBmd[kind], s_capInfo, kCapKindNum));
            s_capData[kind]  = md;
            s_capState[kind] = (md != NULL) ? 2 : 3;
            result           = md;
#if D_ALBW_CAP_DEBUG
            DuskLog.debug(
                "ALBW-CAPLOAD kind={} arc={} bmd={} LOADED data={} joints={} shapes={} mats={}", kind,
                kCapKindArc[kind], kCapKindBmd[kind], (void*)md,
                md ? md->getJointNum() : 0xFFFF, md ? md->getShapeNum() : 0xFFFF,
                md ? md->getMaterialNum() : 0xFFFF);
#endif
        }
        // sync > 0: still loading -> result stays NULL.
    }

    mDoExt_setCurrentHeap(prevHeap);
    return result;
}

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
// HISTORY (2026-06-30): an earlier generalise-to-all-non-Hero's attempt was REVERTED — it
// left mpLinkHatModel dangling at al_head when this freed Kmdl, and the UNGUARDED shadow pass
// (addRealShadow) drew it -> J3DShape::drawFast crash on shop-buy-Zora / rapid switching.
// RE-IMPLEMENTED for cap-on-all-bases: the load is still gated to non-Hero's bases (where our
// Kmdl is independent of the pipeline's mpArcHeap), AND the free below now bumps the clothes
// epoch via dAlbwAlink_invalidateClothesEpoch(), so draw()/shadowDraw skip the stale cap until
// the next changeLink rebuild — closing the exact shadow-crash vector that forced the revert.
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
        // The sumo cap (mpLinkHatModel) and/or face may have been built from this Kmdl's
        // al_head/al_face.  This decCount is OUTSIDE Link's own mpArcHeap->freeAll(), so it does
        // NOT bump the clothes epoch -> draw()/shadowDraw would still treat the cap as live and
        // the shadow pass (addRealShadow -> J3DShape::drawFast) would walk the freed al_head (the
        // documented donor-release crash).  Invalidate the clothes generation so Link's models
        // are skipped until the next changeLink rebuilds + re-stamps them (a <=1-frame skip on
        // the transition, vs a crash).
        dAlbwAlink_invalidateClothesEpoch();
    } else {
        cPhs_Reset(&sKmdlPhase);                      // abandon any in-flight / idle request
    }
}

// ============================================
// Drop the red/blue cap donor (Mmdl/Zmdl) held for the Magic/Zora helmet over a base that does
// not own it.  Same independent-entry safety as releaseFaceDonor (the pipeline owns the base's
// own arc, never this one), and the same clothes-epoch bump so a cap built from the freed helmet
// is skipped by draw/shadow until the next changeLink rebuild.
// ============================================
void releaseCapDonor() {
    if (sCapPhase.id == 2 && sCapDonorKind != 0) {
        dComIfG_resDelete(&sCapPhase, sCapDonorKind == 1 ? "Mmdl" : "Zmdl");  // decCount
        dAlbwAlink_invalidateClothesEpoch();
    } else {
        cPhs_Reset(&sCapPhase);
    }
    sCapDonorKind = 0;
}

bool resourcesReady() {
    if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
        if (dComIfG_resLoad(&sPhase, "alSumou") != cPhs_COMPLEATE_e) {
            return false;
        }
    }
    // Cap + face donor: keep Kmdl resident over every NON-Hero's base so changeLink's sumo
    // block resolves al_head.bmd (the Link cap) AND al_face.bmd there.  Over Hero's the clothes
    // pipeline already owns Kmdl in its own mpArcHeap, so a second refcount here would dangle on
    // that heap's freeAll() (the dual-Kmdl aliasing) -- keep EXCLUDING Hero's; the pipeline's
    // Kmdl serves the cap there.  Over Ordon/Magic/Zora the pipeline owns Bmdl/Mmdl/Zmdl, so our
    // Kmdl is an INDEPENDENT resource-manager entry -- the proven-safe Zora case, now generalised
    // for cap-on-all-bases.  releaseFaceDonor()'s free is made shadow-safe by bumping the clothes
    // epoch (see there).  Before generalising, al_head resolved only over Hero's/Zora and the cap
    // fell back to the topknot over Ordon/Magic (confirmed by the ALBW-CAP build probe: kResInfo=0).
    const bool pipelineOwnsKmdl = !daAlink_c::checkCasualWearFlg() &&
                                  !daAlink_c::checkZoraWearFlg() &&
                                  !daAlink_c::checkMagicArmorWearFlg();
    if (!pipelineOwnsKmdl) {
        if (dComIfG_resLoad(&sKmdlPhase, kCapArcName) != cPhs_COMPLEATE_e) {
            return false;
        }
    } else {
        releaseFaceDonor();
    }

    // Red/Blue cap donor: the Magic/Zora helmet head models (ml_head/Mmdl, zl_head/Zmdl) are not
    // the base arc unless the player wears Magic/Zora, so hold the chosen one as an INDEPENDENT
    // entry over the bases that don't own it (green's al_head rides the Kmdl donor above).  Only
    // while the Link Hat is on.  Releasing is epoch-bumped (releaseCapDonor).  We never reset an
    // in-flight load: release only fires when a DIFFERENT arc is already held.
    int wantCapKind = 0;
    switch (dusk::getSettings().game.capWear.getValue()) {
        case dusk::CapWearMode::Red:
            if (!daAlink_c::checkMagicArmorWearFlg()) wantCapKind = 1;  // hold Mmdl off-base
            break;
        case dusk::CapWearMode::Blue:
            if (!daAlink_c::checkZoraWearFlg()) wantCapKind = 2;        // hold Zmdl off-base
            break;
        default:
            break;  // Off/None/Green: no red/blue donor (green rides Kmdl; topknot needs none)
    }
    if (sCapDonorKind != 0 && sCapDonorKind != wantCapKind) {
        releaseCapDonor();  // switching color, or the base now owns the arc -> free the old donor
    }
    if (wantCapKind != 0) {
        if (dComIfG_resLoad(&sCapPhase, wantCapKind == 1 ? "Mmdl" : "Zmdl") != cPhs_COMPLEATE_e) {
            return false;
        }
        sCapDonorKind = wantCapKind;
    } else if (sCapDonorKind != 0) {
        releaseCapDonor();
    }
    return true;
}

// ============================================
// NEW CODE — ALBW Port (GLOBAL Cap Wear — non-sumo residency, step #3)
// While sumo is OFF and Cap Wear is on, hold the chosen cap arc resident so changeLink's
// non-sumo override can build the cap/topknot on a native outfit.  Reuses the SAME donor phase
// requests + epoch-bump-safe releases as the sumo cap donor (the proven-safe independent-entry
// pattern): each arc is held only over the bases that do NOT own it in the clothes pipeline's
// mpArcHeap, so our refcount never aliases a freeAll (the dual-Kmdl trap; releaseFaceDonor/
// releaseCapDonor bump the clothes epoch so a freed donor can't be drawn).
//   Off   -> release every cap donor (each outfit keeps its native hat).
//   None  -> hold alSumou (topknot 0x33); no native base owns it.
//   Green -> hold Kmdl (al_head) over non-Hero's bases (the Hero's base IS Kmdl -> pipeline owns it).
//   Red   -> hold Mmdl over non-Magic bases.   Blue -> hold Zmdl over non-Zora bases.
// alSumou is left resident until the next stage transition (cPhs_Reset(&sPhase) there); it is a
// tiny arc shared with the sumo body, so we never resDelete it on the native path.  Returns true
// only once the needed arc is resident, so syncLinkModel's rebuild waits for the donor.
// ============================================
bool nativeCapResourcesReady() {
    const dusk::CapWearMode mode = dusk::getSettings().game.capWear.getValue();

    if (mode == dusk::CapWearMode::Off) {
        releaseFaceDonor();
        releaseCapDonor();
        return true;
    }

    if (mode == dusk::CapWearMode::None) {
        releaseFaceDonor();
        releaseCapDonor();
        if (dComIfG_getObjectRes("alSumou", kSumoBodyResIdx) == NULL) {
            if (dComIfG_resLoad(&sPhase, "alSumou") != cPhs_COMPLEATE_e) {
                return false;
            }
        }
        return true;
    }

    // ============================================
    // Green / Red / Blue (Phase 2 — model-agnostic): the cap now loads via the INDEPENDENT loader
    // (ensureIndependentCap / dAlbwSumoTest_tickCapLoad) into a private array+heap the clothes
    // pipeline never frees, so residency is BASE-INDEPENDENT and crash-free -- no clothes-arc donor
    // (those aliased mpArcHeap->freeAll and crashed on release during quick-switch).  Drop any
    // sumo-leftover donor and report ready only once the private cap data is loaded, so the override
    // resolves on every base instead of falling back.
    // ============================================
    releaseFaceDonor();
    releaseCapDonor();
    return dAlbwSumoTest_independentCapData() != NULL;
}

void maintainResources(daAlink_c* i_link) {
    const bool want = dAlbwOutfit_isSumoWorn();
    const bool has  = dAlbwSumoTest_isOutfitActive();

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
        // resourcesReady reads game.capWear itself for the cap donor.
        resourcesReady();
        if (nativeClothesResourcesReady()) {
            i_link->setArcName(0);
        }
    } else {
        // Fully native (sumo off and body flag cleared): hold the global Cap Wear arc resident so
        // changeLink's non-sumo override resolves on any base, or release all donors when Off.
        nativeCapResourcesReady();
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

// True once the global Cap Wear arc is resident for a NATIVE rebuild (sumo off).  syncLinkModel
// gates its native cap rebuild on this so changeLink's override resolves instead of falling back
// (and so a genuinely-unavailable arc can't hang the rebuild -- it just keeps the native hat).
bool dAlbwSumoTest_prepareNativeCapChange() {
    if (dAlbwOutfit_isSumoWorn()) {
        return false;  // sumo path owns the cap via prepareChangeLink/resourcesReady
    }
    return nativeCapResourcesReady();
}

void dAlbwSumoTest_tickCapLoad() {
    const int kind = capWearKind();
    if (kind >= 0) {
        ensureIndependentCap(kind);  // drive the selected cap's independent load (cheap once ready)
    }
}

J3DModelData* dAlbwSumoTest_independentCapData() {
    const int kind = capWearKind();
    if (kind < 0 || s_capState[kind] != 2) {
        return NULL;
    }
    return s_capData[kind];
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
    dAlbwSumoTest_tickCapLoad();  // drive the model-agnostic cap load (base-independent, once-only)
    dAlbwOutfitStats_updateSwimState(i_link);

    // ============================================
    // NEW CODE — ALBW Port (death turns the sumo overlay OFF; user decision 2026-06-29)
    // The sumo worn-bit (700) is a per-save event bit, so without this it survives a game-over
    // and Link respawns in the sumo overlay — and a post-respawn vanilla-menu armor switch then
    // crashes (statusWindowExecute -> changeLink builds the sumo skip-path from a non-resident
    // arc -> initModel(NULL)).  So on game-over clear ONLY the worn bit (NOT the FLG2 flags — an
    // earlier version cleared FLG2_80000 here and desynced the state: model stayed sumo while the
    // flag read native, so the draw guard skipped Link = the broken hybrid sumo after a cycle).
    //
    // CRITICAL: also DEFER the rebuild — return early, do NOT fall through to syncLinkModel here.
    // The game-over / respawn sequence is actively tearing down and reloading Link's arcs, and
    // firing a clothes-change rebuild into that churn desyncs the wear-flag<->mArcName state, so
    // changeLink takes the default branch and builds Link's body from the (freed, non-resident)
    // base arc l_kArcName ("Kmdl") -> initModel(NULL) -> EXCEPTION_ACCESS_VIOLATION (fault 0xc8 in
    // J3DModelData::getTexture).  This is the death+quick-swap crash.  The worn bit is already
    // cleared, so the native rebuild runs safely after respawn (game-over window gone, arcs
    // resident).  Returning early also defers any quick-swap the player queues mid-death, which
    // would otherwise race the same teardown.  Mirrors the stage-transition early-return below.
    // ============================================
    if (i_link->checkGameOverWindow()) {
        if (dAlbwOutfit_isSumoWorn()) {
            dAlbwOutfit_setSumoWorn(false);
        }
        return;
    }

    if (dAlbwOutfit_isStageTransitionUnsafe()) {
        if (!sInStageTransition) {
            sInStageTransition = true;
            cPhs_Reset(&sPhase);
            cPhs_Reset(&sKmdlPhase);
            cPhs_Reset(&sCapPhase);
            sCapDonorKind = 0;
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
    // True only for the "Link cap" modes (Green/Red/Blue).  Off keeps each outfit's native
    // hat; None forces the bald topknot -- neither builds a borrowed cap model.
    const dusk::CapWearMode mode = dusk::getSettings().game.capWear.getValue();
    return mode == dusk::CapWearMode::Green || mode == dusk::CapWearMode::Red ||
           mode == dusk::CapWearMode::Blue;
}

bool dAlbwSumoTest_wantTopknot() {
    // True for None: force the bald sumo topknot (alSumou 0x33) on every outfit.
    return dusk::getSettings().game.capWear.getValue() == dusk::CapWearMode::None;
}

const char* dAlbwSumoTest_capArcName() {
    switch (dusk::getSettings().game.capWear.getValue()) {
        case dusk::CapWearMode::Red:  return "Mmdl";  // Magic helmet ml_head
        case dusk::CapWearMode::Blue: return "Zmdl";  // Zora helmet zl_head
        default:                      return "Kmdl";  // Green: Link's cap al_head
    }
}

const char* dAlbwSumoTest_capModelName() {
    switch (dusk::getSettings().game.capWear.getValue()) {
        case dusk::CapWearMode::Red:  return "ml_head.bmd";
        case dusk::CapWearMode::Blue: return "zl_head.bmd";
        default:                      return "al_head.bmd";
    }
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
