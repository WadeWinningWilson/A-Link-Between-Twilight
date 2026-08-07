#ifndef D_EXT_PLUGIN_WW_PROFILE_REGISTER_H
#define D_EXT_PLUGIN_WW_PROFILE_REGISTER_H

// ============================================================================
// Step 19 Phase 1 — WW profile ownership, via the `fpcPf_Get` lookup.
//
// The receiver resolves an actor index to a profile in exactly one place
// (`f_pc_profile.cpp:fpcPf_Get`). This lets the WW layer answer for its own
// indices there, so the profiles can leave the receiver's static table without
// anything being renumbered.
//
// IN-TREE this is a direct call from `fpcPf_Get`. IN THE PLUGIN it becomes a
// hook on the same function — deliberately the same shape, so the migration is
// a change of attachment rather than a rewrite. `fpcPf_Get` was verified
// present and exported in BOTH our fork's exe and stock upstream dusklight's
// before this design was adopted.
//
// Everything here is inert unless the kill switch is on AND the self-check
// passes. Landing is not activating.
// ============================================================================

#include "f_pc/f_pc_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

// Returns the WW-owned profile for `index`, or NULL to let the receiver's table
// answer. NULL is the normal path for every non-WW index.
process_profile_definition DUSK_CONST* dWwProfileRegister_lookup(s16 index);

// Rows still awaiting an index. Non-zero means NOT READY.
int dWwProfileRegister_pendingRows();

// True only when the switch is on, no rows are pending, AND the self-check has
// confirmed every row against the receiver's own table.
bool dWwProfileRegister_isEnabled();

// Compares every row's profile pointer against `g_fpcPf_ProfileList_p[index]`.
// Returns the number of MISMATCHES; 0 means each row's index really does name
// that profile in the receiver. Runs once, latches, and disables the shim on
// any mismatch. See the .cpp for why this exists rather than a comment.
int dWwProfileRegister_selftest();

// Turns the shim on. Separate from landing the file, per step 19's rule that a
// migration must be individually revertible.
void dWwProfileRegister_setEnabled(bool on);

#ifdef __cplusplus
}
#endif

#endif  // D_EXT_PLUGIN_WW_PROFILE_REGISTER_H
