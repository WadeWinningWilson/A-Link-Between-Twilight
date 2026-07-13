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
#include "dusk/custom_assets.hpp"  // overlay_generation() (Custom Models re-mount signal)
#include "SSystem/SComponent/c_phase.h"
#include "m_Do/m_Do_ext.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"

// ============================================
// DEBUG — model-agnostic cap loader (Phase 2).  Logs the private mount/load lifecycle so the
// independent cap path is diagnosable in-game.  STRIP (set 0) before any upstream push.
// ============================================
#define D_ALBW_CAP_DEBUG 0
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

// ============================================
// NEW CODE — ALBW Port (Custom Models: private-heap sumo body, Option 3b)
// The sumo body/hand/topknot are mounted into a PRIVATE dRes_info_c array + JKRSolidHeap
// (the proven independent-cap pattern) that the clothes pipeline never indexes or frees.
// Because these heaps never alias mpArcHeap->freeAll, a Custom Models toggle can
// BUILD-THEN-SWAP the body — load the fresh overlay content into the free slot, repoint
// changeLink to it, then release the previous slot — with none of the dangling double-free
// that killed the direct-resDelete approach (its crash is recorded in git @1e5804185f).
// changeLink sources the body via dAlbwSumoTest_sumoRes() (private slot, global fallback).
// REVERSIBLE: set D_ALBW_SUMO_PRIVATE_BODY to 0 to fully restore the global-only body path.
// ============================================
#define D_ALBW_SUMO_PRIVATE_BODY 1

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

// TWO private slots per kind (each its own 1-arc dRes array + heap) for build-then-swap.
constexpr int kCapSlots = 2;
dRes_info_c   s_capInfo[kCapKindNum][kCapSlots][1];  // [kind][slot] = private 1-arc array
JKRSolidHeap* s_capHeap[kCapKindNum][kCapSlots]  = {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}};
J3DModelData* s_capData[kCapKindNum][kCapSlots]  = {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}};
u8            s_capState[kCapKindNum][kCapSlots] = {{0, 0}, {0, 0}, {0, 0}};  // 0 idle 1 load 2 ok 3 fail
int           s_capLive[kCapKindNum]    = {-1, -1, -1};   // slot providing this kind now
int           s_capPending[kCapKindNum] = {-1, -1, -1};   // slot loading a fresh copy for a swap

// ============================================
// NEW CODE — ALBW Port (Custom Models: private head arcs track the toggle, Option 3b — part b).
// The sumo face (al_face) and the color cap (al_head/ml_head/zl_head) live in Kmdl/Mmdl/Zmdl.
// A toggle refreshes them via the SAME 2-slot build-then-swap the body uses (load fresh into the
// free slot -> changeLink repoints -> release the old slot next swap), so they hot-swap WITHOUT
// the in-place-free crash (a head heap freed while its model was still referenced -- the calc
// window; ACCESS_VIOLATION recorded @2d36ed8f24).  Face is sourced from the private Kmdl (kind 0)
// so it also becomes base-independent.
// REVERSIBLE: D_ALBW_SUMO_PRIVATE_FACECAP 0 -> base-arc face + reload-scoped cap.
// ============================================
#define D_ALBW_SUMO_PRIVATE_FACECAP 1

// game.capWear -> cap kind index, or -1 for Off/None (no color cap).
int capWearKind() {
    switch (dusk::getSettings().game.capWear.getValue()) {
        case dusk::CapWearMode::Green: return 0;
        case dusk::CapWearMode::Red:   return 1;
        case dusk::CapWearMode::Blue:  return 2;
        default:                       return -1;  // Off / None
    }
}

#if D_ALBW_SUMO_PRIVATE_FACECAP
// Drive one mount step for cap (kind, slot).  Aux allocs land in the slot's private heap
// (setCurrentHeap) so nothing dangles.  Returns ready.
bool ensureCapSlot(int kind, int slot) {
    if (s_capState[kind][slot] == 2) return true;
    if (s_capState[kind][slot] == 3) return false;
    if (s_capHeap[kind][slot] == NULL) {
        s_capHeap[kind][slot] = mDoExt_createSolidHeapFromGame(kCapHeapSize, 0x20);
        if (s_capHeap[kind][slot] == NULL) {
            return false;  // game heap full -> retry next frame (transient; NOT a permanent fail)
        }
    }
    JKRHeap* const prev = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_capHeap[kind][slot]);
    if (s_capState[kind][slot] == 0) {
        if (dRes_control_c::setRes(kCapKindArc[kind], s_capInfo[kind][slot], 1, "/res/Object/", 0,
                                   s_capHeap[kind][slot]) == 0) {
            s_capState[kind][slot] = 3;
        } else {
            s_capState[kind][slot] = 1;
        }
    }
    if (s_capState[kind][slot] == 1) {
        const int sync = dRes_control_c::syncRes(kCapKindArc[kind], s_capInfo[kind][slot], 1);
        if (sync < 0) {
            s_capState[kind][slot] = 3;
        } else if (sync == 0) {
            J3DModelData* md = static_cast<J3DModelData*>(
                dRes_control_c::getRes(kCapKindArc[kind], kCapKindBmd[kind], s_capInfo[kind][slot], 1));
            s_capData[kind][slot]  = md;
            s_capState[kind][slot] = (md != NULL) ? 2 : 3;
#if D_ALBW_CAP_DEBUG
            DuskLog.debug("ALBW-CAPLOAD kind={} slot={} arc={} LOADED data={} joints={}", kind, slot,
                          kCapKindArc[kind], (void*)md, md ? md->getJointNum() : 0xFFFF);
#endif
        }
    }
    mDoExt_setCurrentHeap(prev);
    return s_capState[kind][slot] == 2;
}

// Release one private cap slot (safe: isolated + only ever a slot a rebuild moved off).
void releaseCapSlot(int kind, int slot) {
    if (s_capHeap[kind][slot] != NULL) {
        dRes_control_c::deleteRes(kCapKindArc[kind], s_capInfo[kind][slot], 1);
        mDoExt_destroySolidHeap(s_capHeap[kind][slot]);
        s_capHeap[kind][slot] = NULL;
    }
    s_capData[kind][slot]  = NULL;
    s_capState[kind][slot] = 0;
}

// Ensure a kind's live slot exists (initial load into slot 0).  Returns whether it is resident.
bool ensureCapLive(int kind) {
    if (kind < 0 || kind >= kCapKindNum) return true;
    if (s_capLive[kind] < 0) {
        if (ensureCapSlot(kind, 0)) s_capLive[kind] = 0;
        return s_capLive[kind] >= 0;
    }
    return s_capState[kind][s_capLive[kind]] == 2;
}

// Live model data / named resource for a kind (the color-cap head / al_face), or NULL.
J3DModelData* capLiveData(int kind) {
    if (kind < 0 || kind >= kCapKindNum || s_capLive[kind] < 0) return NULL;
    const int s = s_capLive[kind];
    return (s_capState[kind][s] == 2) ? s_capData[kind][s] : NULL;
}
void* capLiveRes(int kind, const char* resName) {
    if (kind < 0 || kind >= kCapKindNum || s_capLive[kind] < 0) return NULL;
    const int s = s_capLive[kind];
    return (s_capState[kind][s] == 2)
               ? dRes_control_c::getRes(kCapKindArc[kind], resName, s_capInfo[kind][s], 1)
               : NULL;
}

// A kind's live slot resident OR failed (so a missing arc can't hang the coordinated rebuild).
bool capLiveReadyOrFailed(int kind) {
    if (kind < 0 || kind >= kCapKindNum || s_capLive[kind] < 0) return false;
    const int s = s_capLive[kind];
    return s_capState[kind][s] == 2 || s_capState[kind][s] == 3;
}

// Start a build-then-swap for a kind: load fresh into the free slot (live slot stays valid).
void startCapSwap(int kind) {
    if (kind < 0 || kind >= kCapKindNum || s_capLive[kind] < 0) return;
    const int freeSlot = 1 - s_capLive[kind];
    releaseCapSlot(kind, freeSlot);   // replaced a swap ago -> unreferenced
    s_capPending[kind] = freeSlot;
}

#endif  // D_ALBW_SUMO_PRIVATE_FACECAP

// ============================================
// Private sumo body (Option 3b) — pipeline-isolated alSumou mount + build-then-swap.
// Two slots so a Custom Models toggle loads the fresh overlay content into the free slot
// while the old slot keeps the live body valid; changeLink then repoints to the new slot
// and the previous one is released on the following swap (so the slot we free is always
// one that a rebuild moved off of — never a live reference).  Same private-heap isolation
// as the cap loader, so releasing can't alias the clothes pipeline's mpArcHeap->freeAll.
// ============================================
#if D_ALBW_SUMO_PRIVATE_BODY
constexpr int kSumoSlots    = 2;
constexpr u32 kSumoHeapSize = 0x100000;  // alSumou is small; 1MB headroom (fail-safe if short)
dRes_info_c   s_sumoInfo[kSumoSlots][1];
JKRSolidHeap* s_sumoHeap[kSumoSlots]  = {NULL, NULL};
u8            s_sumoState[kSumoSlots] = {0, 0};  // 0 idle 1 loading 2 ready 3 failed
int           s_sumoLiveSlot          = -1;      // slot providing the body now (-1 = none)
int           s_sumoPendingSlot       = -1;      // slot loading a fresh copy for a pending swap
int           s_headGen               = -1;      // overlay generation the private head arcs are on
bool          s_headReapplyPending    = false;   // gen changed; rebuild once all needed arcs fresh
bool          s_headRebuildInFlight   = false;   // a forced rebuild fired; wait for it to fully cycle
bool          s_headRebuildSawTimer   = false;   // observed the clothes timer go non-zero (rebuild began)

// ============================================
// NEW CODE — ALBW Port (storm hardening, 2026-07-11)
// WINNER-COMPARE gate for the generation signal. overlay_generation() bumps on
// EVERY mod toggle/move — including icon-only mods and Mods-window reorders
// that never touch a model arc. Reacting to the raw counter made every such
// bump start a full private-mount refresh, and under a rapid burst (a
// grab-and-place drag used to emit one bump PER STEP) a mount could complete
// against a mid-flight FST flip yet be recorded as current — the persistent
// Linkle-body/Link-hair hybrid that no quick-swap could fix (the stale piece
// was the LIVE private slot, and the tracker believed it was fresh).
// Fix: on a gen bump, compare what OUR four arcs actually resolve to now
// (custom_assets::overlay_path_for) against what the live mounts were built
// from; identical -> record the gen and do nothing. Additionally, every
// completed rebuild schedules ONE verify pass (s_headVerifyWinners) so a flip
// that landed mid-load is caught and re-kicked — the refresh converges on the
// true overlay state no matter how the bumps raced.
// ============================================
std::string   s_headWinners;                     // winners the live mounts were built from
bool          s_headVerifyWinners     = false;   // one-shot re-check after a rebuild completes

std::string headArcWinners() {
    static const char* const kHeadArcs[] = {
        "/res/Object/alSumou.arc", "/res/Object/Kmdl.arc",
        "/res/Object/Mmdl.arc",    "/res/Object/Zmdl.arc",
    };
    std::string s;
    for (const char* arc : kHeadArcs) {
        s += dusk::custom_assets::overlay_path_for(arc);
        s.push_back('|');
    }
    return s;
}

// Drive one private alSumou mount step for `slot` (mirrors ensureIndependentCap).  All aux
// allocs land in the slot's private heap (setCurrentHeap) so nothing dangles.  Returns ready.
bool ensureSumoSlot(int slot) {
    if (s_sumoState[slot] == 2) return true;
    if (s_sumoState[slot] == 3) return false;
    if (s_sumoHeap[slot] == NULL) {
        s_sumoHeap[slot] = mDoExt_createSolidHeapFromGame(kSumoHeapSize, 0x20);
        if (s_sumoHeap[slot] == NULL) {
            return false;  // game heap full -> retry next frame (transient; NOT a permanent fail)
        }
    }
    JKRHeap* const prev = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_sumoHeap[slot]);
    if (s_sumoState[slot] == 0) {
        if (dRes_control_c::setRes("alSumou", s_sumoInfo[slot], 1, "/res/Object/", 0,
                                   s_sumoHeap[slot]) == 0) {
            s_sumoState[slot] = 3;
        } else {
            s_sumoState[slot] = 1;
        }
    }
    if (s_sumoState[slot] == 1) {
        const int sync = dRes_control_c::syncRes("alSumou", s_sumoInfo[slot], 1);
        if (sync < 0) {
            s_sumoState[slot] = 3;
        } else if (sync == 0) {
            s_sumoState[slot] = 2;
        }
    }
    mDoExt_setCurrentHeap(prev);
    return s_sumoState[slot] == 2;
}

// Release a private slot (safe: isolated + unreferenced — always a slot a rebuild moved off).
void releaseSumoSlot(int slot) {
    if (s_sumoHeap[slot] != NULL) {
        dRes_control_c::deleteRes("alSumou", s_sumoInfo[slot], 1);
        mDoExt_destroySolidHeap(s_sumoHeap[slot]);
        s_sumoHeap[slot] = NULL;
    }
    s_sumoState[slot] = 0;
}

// A resource index (0x31/0x32/0x33) from the live private slot, or NULL if not ready.
void* sumoLiveRes(int index) {
    if (s_sumoLiveSlot < 0 || s_sumoState[s_sumoLiveSlot] != 2) return NULL;
    return dRes_control_c::getRes("alSumou", index, s_sumoInfo[s_sumoLiveSlot], 1);
}

// True once a private body slot exists (used to gate the first sumo build in resourcesReady).
bool sumoBodySlotReady() {
    return s_sumoLiveSlot >= 0;
}

// ============================================
// THE coordinator for the whole Custom-Models head refresh — sumo AND native.  Called every
// frame from tickCapLoad.  Determines which private mounts are the VISIBLE source right now:
//   alSumou  : sumo worn (body/hand/topknot) OR Cap Wear None (native topknot 0x33)
//   Kmdl(0)  : sumo worn (al_face) and/or Green cap (al_head)
//   cap kind : Cap Wear Green/Red/Blue (sumo or native)
// It drives their loads, and on an overlay-generation change it starts the body build-then-swap
// AND frees the head arcs, then fires exactly ONE forceReapply once every NEEDED mount is fresh —
// so changeLink never rebuilds a mix of new + stale (the racing that broke topknot/cap tracking).
// ============================================
void driveCustomHeadRefresh() {
    const int  gen        = dusk::custom_assets::overlay_generation();
    const bool sumoWorn   = dAlbwOutfit_isSumoWorn();
    const dusk::CapWearMode capMode = dusk::getSettings().game.capWear.getValue();
    const bool needAlSumou = sumoWorn || capMode == dusk::CapWearMode::None;  // body/hand/topknot
    const bool needFace    = sumoWorn;                                        // Kmdl al_face
    const int  capKind     = capWearKind();                                   // >=0 Green/Red/Blue

    if (s_headGen == -1) {
        s_headGen = gen;  // baseline on first frame; don't act
        s_headWinners = headArcWinners();  // boot mounts read the current FST
    }

    // Track the forced-rebuild cycle: after we fire forceReapply, wait until the clothes change
    // it triggers has gone non-zero AND back to zero (i.e. changeLink actually re-ran and
    // repointed the models) before allowing the NEXT swap to release a slot.  Without this, a
    // rapid re-toggle in the 1-frame gap before the timer is set would free a slot the pending
    // rebuild still reads -> dangling model -> crash.
    {
        daPy_py_c* pl = daPy_getLinkPlayerActorClass();
        const int timer = pl != NULL ? pl->getClothesChangeWaitTimer() : 0;
        if (s_headRebuildInFlight) {
            if (timer != 0) {
                s_headRebuildSawTimer = true;
            } else if (s_headRebuildSawTimer) {
                s_headRebuildInFlight = false;
                s_headRebuildSawTimer = false;
                // Verify once the dust settles: if the overlay flipped while our
                // loads were in flight, the winner compare below re-kicks.
                s_headVerifyWinners = true;
                // Rebuild fully cycled -> the models now reference the LIVE slots, so the previous
                // (non-live) slots are unreferenced.  Release them so we hold ~1 private heap per
                // arc instead of 2 (2-per-arc exhausts the game heap in heavy scenes: Zora base +
                // color cap + sumo -> a slot alloc fails -> the frozen-vanilla-body bug).
                if (s_sumoLiveSlot >= 0) {
                    releaseSumoSlot(1 - s_sumoLiveSlot);
                }
#if D_ALBW_SUMO_PRIVATE_FACECAP
                for (int k = 0; k < kCapKindNum; ++k) {
                    if (s_capLive[k] >= 0) {
                        releaseCapSlot(k, 1 - s_capLive[k]);
                    }
                }
#endif
            }
        }
    }

    // --- Drive loads of the needed private mounts ---
    if (needAlSumou) {
        if (s_sumoLiveSlot < 0 && ensureSumoSlot(0)) {
            s_sumoLiveSlot = 0;  // initial live body slot
        }
        if (s_sumoPendingSlot >= 0) {
            ensureSumoSlot(s_sumoPendingSlot);  // pending build-then-swap load
        }
    }
#if D_ALBW_SUMO_PRIVATE_FACECAP
    if (needFace || capKind == 0) {
        ensureCapLive(0);  // Kmdl live: sumo face and/or Green cap
    }
    if (capKind > 0) {
        ensureCapLive(capKind);  // Red/Blue cap live
    }
    if (s_capPending[0] >= 0) {
        ensureCapSlot(0, s_capPending[0]);  // drive a Kmdl build-then-swap in flight
    }
    if (capKind > 0 && s_capPending[capKind] >= 0) {
        ensureCapSlot(capKind, s_capPending[capKind]);
    }
#endif

    // Reclaim game heap + prevent STALE live slots: free any private arc that is NOT the visible
    // source right now, but ONLY when the model is stable (no swap / clothes change in flight) so
    // nothing references it.  A freed arc reloads FRESH (current overlay) the next time it is
    // needed -- which is what stops a toggle while an arc is unused from leaving a stale slot that a
    // later use would show (e.g. topknot -> Link+green-cap-not-sumo froze alSumou, then wearing
    // sumo showed the stale Linkle body), and lets a heap-starved retry recover (a color cap turned
    // off frees its arc for the alSumou body slot).
    if (!s_headReapplyPending && !s_headRebuildInFlight) {
        daPy_py_c* pp = daPy_getLinkPlayerActorClass();
        if (pp == NULL || pp->getClothesChangeWaitTimer() == 0) {
            if (!needAlSumou && (s_sumoHeap[0] != NULL || s_sumoHeap[1] != NULL)) {
                releaseSumoSlot(0);
                releaseSumoSlot(1);
                s_sumoLiveSlot    = -1;
                s_sumoPendingSlot = -1;
            }
#if D_ALBW_SUMO_PRIVATE_FACECAP
            for (int k = 0; k < kCapKindNum; ++k) {
                const bool needed =
                    (k == 0 && (needFace || capKind == 0)) || (k > 0 && k == capKind);
                if (!needed && (s_capHeap[k][0] != NULL || s_capHeap[k][1] != NULL)) {
                    releaseCapSlot(k, 0);
                    releaseCapSlot(k, 1);
                    s_capLive[k]    = -1;
                    s_capPending[k] = -1;
                }
            }
#endif
        }
    }

    // --- Overlay changed -> start a coordinated refresh (once the prior change settled) ---
    if ((gen != s_headGen || s_headVerifyWinners) && !s_headReapplyPending &&
        !s_headRebuildInFlight)
    {
        daPy_py_c* p = daPy_getLinkPlayerActorClass();
        if (p != NULL && p->getClothesChangeWaitTimer() != 0) {
            return;  // a clothes change is in flight — defer a frame
        }
        s_headGen = gen;
        s_headVerifyWinners = false;
        // Winner-compare gate (see the block comment at s_headWinners): only
        // refresh when one of OUR arcs actually resolves differently now.
        const std::string winners = headArcWinners();
        if (winners == s_headWinners) {
            return;  // icon-only toggle / reorder — none of our arcs changed
        }
        s_headWinners = winners;
        if (needAlSumou && s_sumoLiveSlot >= 0) {
            const int freeSlot = 1 - s_sumoLiveSlot;  // replaced a swap ago -> unreferenced
            releaseSumoSlot(freeSlot);
            s_sumoPendingSlot = freeSlot;             // build-then-swap into it
        }
#if D_ALBW_SUMO_PRIVATE_FACECAP
        if (needFace || capKind == 0) startCapSwap(0);        // Kmdl build-then-swap
        if (capKind > 0) startCapSwap(capKind);               // Red/Blue build-then-swap
#endif
        s_headReapplyPending = true;
        // NOTE: the epoch is invalidated only when the rebuild actually fires (below), NOT here —
        // build-then-swap keeps the OLD slots valid, so the current model stays correct/visible
        // during the (possibly heap-stalled) load, instead of going invisible for the whole wait.
    }

    // --- Fire ONE rebuild once every NEEDED mount is fresh.  CRITICAL: check that all pendings are
    //     LOADED without promoting any (ensure*Slot just drives the load), then promote them ALL
    //     ATOMICALLY and rebuild.  If we promoted a cap the instant it loaded (the old bug), a body
    //     swap stalling on the game heap would leave new-cap + old-body -> the reported hybrid. ---
    if (s_headReapplyPending) {
        bool ready = true;
        if (needAlSumou) {
            const bool loaded = (s_sumoPendingSlot >= 0) ? ensureSumoSlot(s_sumoPendingSlot)
                                                         : (s_sumoLiveSlot >= 0);
            if (!loaded) ready = false;
        }
#if D_ALBW_SUMO_PRIVATE_FACECAP
        if (needFace || capKind == 0) {
            const bool loaded = (s_capPending[0] >= 0) ? ensureCapSlot(0, s_capPending[0])
                                                       : capLiveReadyOrFailed(0);
            if (!loaded) ready = false;
        }
        if (capKind > 0) {
            const bool loaded = (s_capPending[capKind] >= 0) ? ensureCapSlot(capKind, s_capPending[capKind])
                                                             : capLiveReadyOrFailed(capKind);
            if (!loaded) ready = false;
        }
#endif
        if (ready) {
            // Promote every arc at once, THEN one rebuild -> body + face + cap all switch together.
            if (s_sumoPendingSlot >= 0) {
                s_sumoLiveSlot    = s_sumoPendingSlot;
                s_sumoPendingSlot = -1;
            }
#if D_ALBW_SUMO_PRIVATE_FACECAP
            if (s_capPending[0] >= 0) {
                s_capLive[0]    = s_capPending[0];
                s_capPending[0] = -1;
            }
            if (capKind > 0 && s_capPending[capKind] >= 0) {
                s_capLive[capKind]    = s_capPending[capKind];
                s_capPending[capKind] = -1;
            }
#endif
            dAlbwAlink_invalidateClothesEpoch();  // hide the ~1 frame between here and the rebuild
            dAlbwOutfit_forceReapply();  // one rebuild: body + face + cap/topknot all fresh
            s_headReapplyPending  = false;
            s_headRebuildInFlight = true;   // block the next swap until this rebuild fully cycles
            s_headRebuildSawTimer = false;
        }
    }
}
#endif  // D_ALBW_SUMO_PRIVATE_BODY

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
#if D_ALBW_SUMO_PRIVATE_BODY
    // The private head refresh is driven every frame in tickCapLoad (sumo + native).  Here we
    // only gate the first sumo build until the private body slot is resident, so the body never
    // briefly builds from the global fallback and then swaps.
    if (!sumoBodySlotReady()) {
        return false;
    }
#endif
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
#if D_ALBW_SUMO_PRIVATE_BODY
    // THE per-frame coordinator for the whole head refresh (sumo body/hand/topknot/face + native
    // topknot/cap).  Drives every needed private mount and fires ONE rebuild per toggle when all
    // are fresh -- so no piece ever rebuilds stale (the topknot/cap wrong-source bug).
    driveCustomHeadRefresh();
#endif
    // Also ensure the selected color cap's live slot exists even if the coordinator is compiled
    // out (kill switch); ensureCapLive is idempotent, so this is a no-op once resident.
#if D_ALBW_SUMO_PRIVATE_FACECAP
    const int kind = capWearKind();
    if (kind >= 0) {
        ensureCapLive(kind);
    }
#endif
}

J3DModelData* dAlbwSumoTest_independentCapData() {
#if D_ALBW_SUMO_PRIVATE_FACECAP
    return capLiveData(capWearKind());  // live build-then-swap slot for the selected color cap
#else
    return NULL;
#endif
}

// Sumo face (al_face) from the private Kmdl mount (kind 0) — tracks the Custom Models toggle
// and is base-independent.  NULL until Kmdl is resident, so changeLink falls back to the
// base-arc al_face (prior behavior) until the private mount lands.
void* dAlbwSumoTest_sumoFaceData() {
#if D_ALBW_SUMO_PRIVATE_FACECAP
    return capLiveRes(0, "al_face.bmd");  // kind 0 == Kmdl, live slot
#else
    return NULL;
#endif
}

// Private sumo body accessor (Option 3b) — live private-slot resource, else the global arc.
void* dAlbwSumoTest_sumoRes(int index) {
#if D_ALBW_SUMO_PRIVATE_BODY
    void* r = sumoLiveRes(index);
    if (r != NULL) {
        return r;
    }
#endif
    return dComIfG_getObjectRes("alSumou", index);
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

// ============================================
// NEW CODE — ALBW Port (FIX: vanilla collection-menu / "workshop" clothes-change while sumo worn)
// REVERSIBLE: set D_ALBW_SUMO_MENU_LEAVE_FIX to 0 to fully restore the prior (crashing) behavior
// with no logic removed (the call site stays; this becomes a no-op).
//
// Crash root: while sumo is worn the sumo-body flag FLG2_UNK_80000 is set, so
// checkNoResetFlg2(FLG2_UNK_280000) (a bitwise-ANY test, d_a_player.h) is true -> loadModelDVD
// takes the sumo SKIP-PATH, which does NOT reload the arc, and changeLink then builds the
// newly-selected native outfit from a NON-RESIDENT arc -> initModel(NULL) -> ACCESS_VIOLATION
// (fault 0xc8, getTexture on null).  The gameplay leave (dAlbwOutfit_syncLinkModel) avoids this by
// clearing the sumo model flags BEFORE setClothesChange(0); the collection menu runs its OWN
// execute loop (dMw_c::_execute) and never does that.
//
// Fix: mirror the gameplay leave -- clear FLG2_UNK_200000 | FLG2_UNK_80000 so loadModelDVD runs the
// NORMAL build-then-swap reload (loads the target arc AND safely defers until it is resident,
// gating on cPhs_COMPLEATE_e).  MUST be called only when a real clothes change follows
// (setClothesChange), so it is invoked per-case in dMenu_Collect2D_c::changeClothe right before
// setClothesChange(0) -- NOT at menu-open -- else the "select the same outfit as the sumo base"
// case (no rebuild) would leave native flags on a sumo model -> invisible Link.  No-op unless the
// sumo body flag is set.
// ============================================
#define D_ALBW_SUMO_MENU_LEAVE_FIX 1
void dAlbwSumoTest_onVanillaClothesMenuLeave() {
#if D_ALBW_SUMO_MENU_LEAVE_FIX
    daAlink_c* link = daAlink_getAlinkActorClass();
    if (link != NULL && link->checkNoResetFlg2(daAlink_c::FLG2_UNK_80000)) {
        link->offNoResetFlg2(daAlink_c::FLG2_UNK_200000);
        link->offNoResetFlg2(daAlink_c::FLG2_UNK_80000);
    }
#endif
}

void dAlbwSumoTest_onWrestlerMet() {
    dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kSumoWrestlerMetBit]);
}

#endif  // TARGET_PC
