#ifndef D_EXT_SAVE_GUARD_H
#define D_EXT_SAVE_GUARD_H

// №81 EXTENSION-FIRST: refuse native stage-progression writes while WW content is
// live on a vanilla host (today: F_SP115 → Lanayru bits). Thin header so call
// sites in d_com_inf_game / d_save do not pull the full mount type graph.
// №106: filename neutral (was d_ext_ww_save_guard.h) — avoid ww in shipping paths.

#if TARGET_PC

// True if stage name is a Dusklight WW host (R_DL* interiors / F_DL* fields).
bool dExtWwSave_isWwHostStage(const char* stageName);

// True if a WW BG / room-lane interior is currently active.
bool dExtWwSave_isWwContentActive();

// Returns true when the write must be refused (caller should no-op).
// stageNo may be -1 when the write targets the current stage memory.
bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit);

#endif  // TARGET_PC

#endif /* D_EXT_SAVE_GUARD_H */
