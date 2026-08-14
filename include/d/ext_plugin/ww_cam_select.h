// ============================================================================
// ww_cam_select.h — the donor camera SELECTOR (type/mode/style), ported.
//
// KIT-LINEAGE: donor-port
// KIT-DONOR: d/d_camera.cpp NonMatching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching
//
// The second half of the FULL camera port (user ruling 2026-08-10; data half
// = ww_cam_data). Ports the donor's nextType/nextMode/selection latch to run
// against the donor-axis tables, installed through the receiver's NULL-default
// selection extension point (dCamera_setSelectHook). Scoped at runtime to WW
// host stages; everywhere else the receiver's own selection runs untouched.
// ============================================================================
#ifndef WW_CAM_SELECT_H
#define WW_CAM_SELECT_H

#if TARGET_PC

// Install the selection hook (idempotent). Called from the layer's own
// dExtWwRoom_installHooks — the receiver names no donor symbol.
void dExtWwCam_installSelect(void);

#endif  // TARGET_PC

#endif  // WW_CAM_SELECT_H
