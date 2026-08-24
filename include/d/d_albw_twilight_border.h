#ifndef D_ALBW_TWILIGHT_BORDER_H
#define D_ALBW_TWILIGHT_BORDER_H

#if TARGET_PC

// ============================================
// NEW CODE — ALBW Port (alpha cleanup: twilight-border fallback)
// FALLBACK-ONLY restoration of missing province twilight borders.
//
// Field stages lock ONE layer at entry from the start room's province, but
// the twilight border wall (obj twGate) and its entry trigger (TgTGate) are
// authored only on twilight layer variants (confirmed via Stage Inspector:
// F_SP121 room 9 twGate = ACTd/L13 + ACTe/L14, nothing on any day layer).
// A day-side approach to an UNCLEARED province therefore loses the border
// entirely while the flag-gated twilight enemies still spawn.
//
// This driver acts ONLY when vanilla demonstrably failed: for the current
// stay room it reads the authored placements, and iff (a) a border
// wall/tag is authored there, (b) its province is still twilight-uncleared,
// and (c) NO instance of that group is live, it spawns a duplicate from
// the authored transform/params. If vanilla spawned the border (twilight
// layer loads) or the province is cleared, it does nothing — vanilla saves
// see zero behavior change. Spawned actors keep their vanilla province
// self-guards, so they despawn on a legitimate clear. Cause-agnostic and
// border-generic by design; every fallback firing is logged.
// ============================================

void dAlbwTwilightBorder_drive();

#endif // TARGET_PC

#endif // D_ALBW_TWILIGHT_BORDER_H
