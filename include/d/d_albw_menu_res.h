#ifndef D_ALBW_MENU_RES_H
#define D_ALBW_MENU_RES_H

// ============================================
// NEW CODE — ALBW Port (load-order mod system, Phase 1 follow-up)
// Live re-mount of the BOOT-RESIDENT menu 2D archives (itemicon.arc,
// clctres.arc, dmapres.arc) when the Custom Models overlay set changes.
//
// Why: these archives are ARAM-mounted ONCE at the logo screen (d_s_logo) and
// held for the whole session, so a mod that overlays them (e.g. Linkle's
// custom item / start-menu outfit icons) is frozen at its BOOT state — a
// mid-session toggle changes the virtual FST but nothing ever re-reads the
// arc. Same resident-asset class as the sumo body (Custom-Model-API-Work.md
// §5); same cure: watch overlay_generation(), re-mount through the normal
// overlay-aware DVD path, and BUILD-THEN-SWAP the global archive pointer.
//
// Driven per-frame from the play scene (dScnPly_Draw); idle cost is one int
// compare. Kill switch: D_ALBW_MENU_RES_REMOUNT 0 compiles it out (menu arcs
// then revert to boot-scoped, the pre-fix behavior).
// ============================================

#ifndef D_ALBW_MENU_RES_REMOUNT
#define D_ALBW_MENU_RES_REMOUNT 1
#endif

// Poll for an overlay-set change and drive any in-flight re-mounts. Call once
// per frame from the play scene. No-op on non-PC builds.
void dAlbwMenuRes_drive();

#endif  // D_ALBW_MENU_RES_H
