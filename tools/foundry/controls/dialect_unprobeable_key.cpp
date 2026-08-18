// ============================================================================
// dialect_unprobeable_key.cpp — NEGATIVE CONTROL for K3 (dialect_codemod.py).
//
// NOT DONOR SOURCE. NOT RECEIVER SOURCE. Never compiled, never ported, never
// included by anything. It exists to be scanned.
//
// WHAT IT PROVES: this TU deliberately contains donor constructs that R5's
// CODE_DIALECT *does* carry rows for. A K3 --scan over it MUST name them. If
// K3 reports "(none present in this TU)", then K3's silence carries no
// information — and every prior clean scan was a pass that could not have come
// out differently (control.py's whole premise, tale §1004).
//
// The two constructs below are chosen because their R5 keys begin with `(`:
//   · the CONTROL row, marker WW_DIALECT_CONTROL_MUST_APPEAR
//   · the REAL DN-3 LAW row `(J3DModelData*)dComIfG_getObjectRes(...)` — the
//     §810-2/§814 raw-cast crash class
// A porter who reads a silent K3 run as "no DN-3 problem in this TU" has been
// told nothing. That is the class this control catches.
// ============================================================================

void ww_dialect_control_fixture(void) {
    // --- the control construct (marker row, R5 tier CONTROL) ---------------
    WW_DIALECT_CONTROL_MUST_APPEAR* ctl =
        (WW_DIALECT_CONTROL_MUST_APPEAR*)dComIfG_getObjectRes("Ctl", 0);

    // --- the real thing the control stands in for (R5 tier LAW, DN-3) ------
    J3DModelData* mdl = (J3DModelData*)dComIfG_getObjectRes("Obj", 4);

    // --- second unprobeable-key row, same class (HOST tier) ----------------
    fopAcM_SetUserArea(this, (u32)this);
}
