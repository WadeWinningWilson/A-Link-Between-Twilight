#ifndef D_EXT_SAVE_GUARD_H
#define D_EXT_SAVE_GUARD_H

// №81 EXTENSION-FIRST: refuse native stage-progression writes while WW content is
// live on a vanilla host (today: F_SP115 → Lanayru bits). Thin header so call
// sites in d_com_inf_game / d_save do not pull the full mount type graph.
// №106: filename neutral (was d_ext_ww_save_guard.h) — avoid ww in shipping paths.

#if TARGET_PC

// True if stage name is a Dusklight WW host (R_DL* interiors / F_DL* fields).
bool dExtWwSave_isWwHostStage(const char* stageName);
// §47: give the player the donor cast's LIGHTING RECIPE inside donor spaces.
// Returns true when it handled the lighting, in which case the caller must SKIP
// setLightTevColorType_MAJI — bypassing MAJI is the point, not a side effect.
// Always false outside donor stages and at the default setting, so receiver
// areas take an early-out and are untouched.
class dKy_tevstr_c;
class J3DModel;
bool dExtWw_applyPlayerDonorLook(J3DModel* model, dKy_tevstr_c* tevStr);

// №165: storyboard message IDs. A donor storyboard's JMSG track fires DONOR
// message indices (awake.stb: 0x357, 0x358, 0x050, 0x359, 0x35A...). Those
// indices are meaningless in the receiver's message table, where they land on
// unrelated shipped strings — which is why the opening showed an Ordon pumpkin
// line and a quest-log string rather than dialogue.
//
// Returns TRUE when the caller must SUPPRESS the native display: showing the
// receiver's string for a donor index is always wrong, so a missing line is
// preferable to a foreign one (№31 applied to text).
bool dExtWw_handleDemoMessage(u32 donorMsgId);

// §49 rendering. The storyboard is TIMED, so the box never gates the demo:
// a new line replaces the old, A dismisses early but is never required, and
// the line self-clears when the scene ends.
void dExtWw_pollDemoMessage();   // per-frame (execute)
void dExtWw_drawDemoMessage();   // per-frame (DRAW pass only — set2DOpa)

/**
 * №170: defer №110 arrival snap while F_DL01 still intends to run `awake`
 * (pending from host entry until the opening ends or is abandoned).
 */
bool dExtWw_openingOwnsCamera();

/**
 * №170: pause arrival G-guard / refuse force-end only AFTER `awake` has
 * ordered. Arming this earlier blocked residual clear and left a permanent
 * busy slot (№169's G-guard never got to free the lane).
 */
bool dExtWw_openingPauseArrivalGuard();

/**
 * №171: №110 snap defer — hold after ORDER, or still trying to order.
 * Prefer this over openingOwnsCamera for arrival snap.
 */
bool dExtWw_deferArrivalCameraSnap();

// True if a WW BG / room-lane interior is currently active.
bool dExtWwSave_isWwContentActive();

// Returns true when the write must be refused (caller should no-op).
// stageNo may be -1 when the write targets the current stage memory.
bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit);

#endif  // TARGET_PC

#endif /* D_EXT_SAVE_GUARD_H */
