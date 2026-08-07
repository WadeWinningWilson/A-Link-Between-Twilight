#ifndef D_EXT_EVT_EVT1_BOUNDARY_H
#define D_EXT_EVT_EVT1_BOUNDARY_H

// ============================================================
// §423 A1 — THE ONE BOUNDARY of the parallel WW event stack.
//
// This is the events campaign's answer to the JA1 audio campaign's DSP seam,
// and it is deliberately the same shape: WW's own event system is ported
// donor-verbatim into its own namespace (JEvent1::, mirroring JAudio1::), and
// exactly ONE layer decides which stack serves a call — the 26
// dComIfGp_evmng_* inlines in d_com_inf_game.h.
//
// WHY (audit №284): four of that audit's six findings were FORK problems, not
// mount artifacts — WW and TP genuinely disagree about a shared engine
// contract (getMyActIdx's no-match return, the PACKAGE/PLAY teardown fork,
// cutEnd's mode gate, the demo rotate write), and a natively-ported WW actor
// needed WW's side of each. Injecting donor semantics into TP's functions
// makes every one of those a mainline-TP regression risk; porting the donor's
// OWN system alongside makes them structurally impossible, because TP's
// functions go back to byte-vanilla and stop being asked to serve two games.
//
// WHAT MAKES THIS SAFE: one event at a time. dEvt_control_c admits a single
// running event (long-standing project invariant, and the reason the event
// order/change machinery exists at all), so the two managers can never both be
// live. The gate below therefore has exactly one correct answer at any instant.
//
// WHAT DOES *NOT* FORK: the event CONTAINER. §379a proved donor event data
// parses cleanly with the receiver's own structs — that is how WW's real door
// events were merged into the host stages. The fork is semantics, never format,
// so both stacks read the same event_list.dat and the same save-backed flags.
//
// CALL-SITE IMPACT: none. All ~2,454 dComIfGp_evmng_* call sites across the
// actor tree keep their source verbatim — TP actors and WW actors alike.
// ============================================================

#include "dolphin/types.h"

namespace JEvent1 {

// ============================================================
// A1 gate. TRUE while the running event belongs to the WW stack.
//
// Phase A1: answers from WW-host context only (isWwHostStage), which is the
// same discriminator №285's scope fixes use today — so flipping A4's dispatch
// changes WHICH code runs, never WHEN. Phase A4 tightens it to the running
// event's own provenance (the event data's owning base), at which point WW
// content hosted on a TP stage is served correctly too — the one failure mode
// the host-stage gate cannot express (recorded in §423's risk ledger).
// ============================================================
bool evt1_isActive();

// A1: compiled-out kill switch, knob00/§329 pattern. 0 = the receiver's stack
// serves everything exactly as it does today (the A4 dispatch becomes inert),
// so any regression in the parallel stack is one #define away from reverted.
#ifndef DUSK_EVT1_NATIVE
// §469-C1 FALSIFICATION (Housing, user-authorised, TEMPORARY — History's switch):
// flipped 1 -> 0 to test whether the reproducible hang at `[Evt] §350c entry()
// GRANT` (exe 22:58:32 gFrm=515, exe 23:16:29 gFrm=618, same site both times) is
// A4's. Hang GONE => A4 confirmed, narrowed to the two hooks at
// d_event_data.cpp:542/:1868. Hang STAYS => A4 exonerated in one build.
// RESTORE TO 1 once the verdict is recorded — this is History's control surface,
// borrowed for a diagnostic, not re-ruled.
#define DUSK_EVT1_NATIVE 0  // §423 A4: temporarily OFF for the §469-C1 test
#endif

}  // namespace JEvent1

#endif /* D_EXT_EVT_EVT1_BOUNDARY_H */
