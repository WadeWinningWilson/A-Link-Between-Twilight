#ifndef D_EXT_ROOM_VERIFY_H
#define D_EXT_ROOM_VERIFY_H

// ============================================================
// §389 — ROOM MANIFEST VERIFIER (donor expectation vs live actuality).
//
// The user's question that produced this: the "Ivan" (TP's same-named actor
// filling a slot the WW port failed to fill) was an ACCIDENTAL detector — it
// made an absence visible. Remove the accident and a missing donor row becomes
// undetectable: nothing spawns, nothing errors, nobody notices. Every silent
// absence this campaign hit was caught by a human eye, not by measurement.
//
// This verifies each donor-authored row of a hosted room at runtime and reports
// PRESENT / SUBSTITUTED (Ivan class) / MISSING / DEFERRED. Expectation comes
// from the donor's own dzr rows via tools/ww_crew_restoration_skeleton/
// room_expect.py -> <MOD>/npc/room_expect.csv. Absent manifest => UNKNOWN,
// never a pass (№31-C).
//
// Call once per room settle (a few frames after load, so ground-snap and
// gravity have run).
// ============================================================

void dExtRoomVerify_run(const char* i_hostStage, int i_hostRoom);

#endif /* D_EXT_ROOM_VERIFY_H */
