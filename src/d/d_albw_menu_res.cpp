// ============================================
// NEW CODE — ALBW Port (load-order mod system, Phase 1 follow-up)
// Live re-mount of the boot-resident menu 2D archives on a Custom Models
// overlay change. See d_albw_menu_res.h for the why; mechanics below:
//
//   - d_s_logo ARAM-mounts itemicon/clctres/dmapres once at boot and publishes
//     them via the dComIfGp_*Archive globals. Every consumer (collect screen,
//     item wheel readItemTexture, dungeon map, shop) re-resolves the global and
//     re-fetches resources per menu open / per read — the DATA is boot-bound,
//     the CONSUMERS are not. So swapping the global to a freshly-mounted
//     instance makes the very next menu open show the current overlay content.
//   - Re-mounts go through mDoDvdThd_mountXArchive_c (the exact call d_s_logo
//     uses), i.e. the normal overlay-aware DVD path — so both directions work:
//     mod ON reads the loose file, mod OFF re-reads the vanilla disc bytes.
//   - BUILD-THEN-SWAP (the §5 sumo rule): the old instance is NOT freed at
//     swap time — an open UI may still hold pictures built from it. It is
//     retired and freed on the NEXT swap of the same arc (steady state: at
//     most one retired instance per arc). The global pointer swap itself is
//     deferred while the start menu (dMw_c) is open so one menu session never
//     mixes two instances (readItemTexture copies into caller buffers, so
//     everything already loaded stays valid either way).
//   - Only arcs whose WINNING PROVIDER actually changed re-mount (compared via
//     custom_assets::overlay_path_for), so toggling a mod that ships no menu
//     arcs re-mounts nothing.
// ============================================

#include "d/d_albw_menu_res.h"

#if TARGET_PC && D_ALBW_MENU_RES_REMOUNT

#include "JSystem/JKernel/JKRArchive.h"
#include "JSystem/JKernel/JKRExpHeap.h"  // JKRExpHeap : public JKRHeap (J2D-heap upcast)
#include "d/d_com_inf_game.h"
#include "d/d_menu_window.h"  // dMw_c::mMenuProc / NO_MENU (the real menu-open signal)
#include "d/d_meter2_info.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "m_Do/m_Do_dvd_thread.h"
#include "m_Do/m_Do_ext.h"

#include <string>

namespace {

// Disc paths mirror d_s_logo's mount table (PC builds take the GC branch).
#if PLATFORM_WII || VERSION == VERSION_SHIELD_DEBUG
#define ALBW_MENURES_CLCT "/res/LayoutRevo/clctresR.arc"
#define ALBW_MENURES_DMAP "/res/LayoutRevo/dmapresR.arc"
#else
#define ALBW_MENURES_CLCT "/res/Layout/clctres.arc"
#define ALBW_MENURES_DMAP "/res/Layout/dmapres.arc"
#endif

struct TrackedArc {
    const char* discPath;
    JKRArchive* (*get)();
    void (*set)(JKRArchive*);
    // Loose override path the LIVE instance was (approximately) mounted from;
    // "" = vanilla. Seeded from the current overlay on first drive — the boot
    // mounts already went through the overlay, so no remount fires at startup.
    std::string mountedWinner;
    // Winner recorded when the in-flight mount was kicked (becomes
    // mountedWinner on completion; re-compared so a toggle DURING a mount
    // still converges on the newest state).
    std::string kickWinner;
    mDoDvdThd_mountXArchive_c* pending;
    JKRArchive* retired;  // previous live instance; freed on the NEXT swap
    bool seeded;
};

TrackedArc s_arcs[] = {
    {"/res/Layout/itemicon.arc",
     [] { return dComIfGp_getItemIconArchive(); },
     [](JKRArchive* arc) { dComIfGp_setItemIconArchive(arc); },
     {}, {}, nullptr, nullptr, false},
    {ALBW_MENURES_CLCT,
     [] { return dComIfGp_getCollectResArchive(); },
     [](JKRArchive* arc) { dComIfGp_setCollectResArchive(arc); },
     {}, {}, nullptr, nullptr, false},
    {ALBW_MENURES_DMAP,
     [] { return dComIfGp_getDmapResArchive(); },
     [](JKRArchive* arc) { dComIfGp_setDmapResArchive(arc); },
     {}, {}, nullptr, nullptr, false},
};

int s_lastGen = -1;
bool s_anyPending = false;

// True while any start-menu 2D screen (ring / collect / map / save / ...) is
// up. NOTE the trap this replaces: dMeter2Info_getMenuWindowClass() alone is
// USELESS as an open-test — the dMw_c PROC is created alongside the HUD meter
// at scene start and lives for the whole play session, so it is virtually
// always non-NULL (that mistake deferred the pointer swap forever). The real
// signals are dMw_c::mMenuProc (NO_MENU = fully closed and idle) plus the 2D
// pause flag (held while the menu's screen capture is alive).
bool menu_2d_open() {
    dMw_c* mw = dMeter2Info_getMenuWindowClass();
    if (mw != NULL && mw->isMenuActive()) {
        return true;
    }
    return dComIfGp_isPauseFlag();
}

// Kick an overlay-aware async re-mount for this arc (same call as d_s_logo's
// aramMount). J2D heap: the boot mounts live there; the retired-instance free
// keeps the footprint bounded at ~one extra instance per arc.
void kick_remount(TrackedArc& arc, const std::string& winner) {
    arc.pending =
        mDoDvdThd_mountXArchive_c::create(arc.discPath, 0, JKRArchive::MOUNT_ARAM,
                                          mDoExt_getJ2dHeap());
    if (arc.pending == nullptr) {
        DuskLog.warn("[menu_res] {}: re-mount command create FAILED (kept old instance)",
                     arc.discPath);
        return;
    }
    arc.kickWinner = winner;
    DuskLog.info("[menu_res] {}: re-mounting ({} -> {})", arc.discPath,
                 arc.mountedWinner.empty() ? "vanilla" : arc.mountedWinner.c_str(),
                 winner.empty() ? "vanilla" : winner.c_str());
}

}  // namespace

void dAlbwMenuRes_drive() {
    const int gen = dusk::custom_assets::overlay_generation();
    if (gen == s_lastGen && !s_anyPending) {
        return;  // idle fast-path (one int compare per frame)
    }

    if (gen != s_lastGen) {
        s_lastGen = gen;
        for (TrackedArc& arc : s_arcs) {
            const std::string winner = dusk::custom_assets::overlay_path_for(arc.discPath);
            if (!arc.seeded) {
                // First sight (boot): the logo mounts already read through the
                // current overlay — record, don't remount.
                arc.mountedWinner = winner;
                arc.seeded = true;
                continue;
            }
            if (arc.pending == nullptr && winner != arc.mountedWinner) {
                kick_remount(arc, winner);
            }
            // pending != nullptr: the post-completion re-compare below catches
            // a winner that changed again mid-mount.
        }
    }

    bool anyPending = false;
    for (TrackedArc& arc : s_arcs) {
        if (arc.pending == nullptr) {
            continue;
        }
        if (!arc.pending->sync()) {
            anyPending = true;  // still loading on the DVD thread
            continue;
        }
        // Defer the pointer swap while a start-menu screen is open so one menu
        // session never resolves two different instances (open/close pair
        // must hit the same archive — its close calls removeResourceAll).
        if (menu_2d_open()) {
            anyPending = true;
            continue;
        }
        JKRArchive* fresh = arc.pending->getArchive();
        arc.pending->destroy();
        arc.pending = nullptr;
        if (fresh == nullptr) {
            DuskLog.warn("[menu_res] {}: re-mount FAILED (kept old instance)", arc.discPath);
            arc.mountedWinner = arc.kickWinner;  // don't retry-loop a broken file
            continue;
        }
        JKRArchive* old = arc.get();
        arc.set(fresh);
        // Free the instance retired by the PREVIOUS swap — anything built from
        // it belonged to menus at least one full swap old (menus re-resolve the
        // global per open, and swaps only land while the menu is closed).
        if (arc.retired != nullptr) {
            arc.retired->unmount();
        }
        arc.retired = old;
        arc.mountedWinner = arc.kickWinner;
        DuskLog.info("[menu_res] {}: swapped in ({})", arc.discPath,
                     arc.mountedWinner.empty() ? "vanilla" : arc.mountedWinner.c_str());

        // The overlay may have changed again while this mount was in flight —
        // converge on the newest winner.
        const std::string winner = dusk::custom_assets::overlay_path_for(arc.discPath);
        if (winner != arc.mountedWinner) {
            kick_remount(arc, winner);
            if (arc.pending != nullptr) {
                anyPending = true;
            }
        }
    }
    s_anyPending = anyPending;
}

#else  // !TARGET_PC || !D_ALBW_MENU_RES_REMOUNT

void dAlbwMenuRes_drive() {}

#endif
