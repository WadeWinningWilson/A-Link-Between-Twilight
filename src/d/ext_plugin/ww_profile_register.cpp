// ============================================================================
// ww_profile_register.cpp — step 19 PHASE 1: the WW profile registration shim.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-STATUS: UNKNOWN
//
// WHAT THIS IS, AND WHAT IT DELIBERATELY IS NOT
// Phase 1 of the step-19 migration: the `g_profile` cluster --
// 20 symbols (NOT 23 -- see the count control on kRows), ONE referencing TU.
// It is the mechanical cluster, chosen first not because it is easy but because
// it exercises the migration harness on work that cannot fail subtly.
//
// This file is **ADDITIVE ONLY**. It changes nothing in the receiver. Landing it
// cannot regress anything, and it is correct or incorrect on inspection rather
// than on a build -- which is why it is the one step of Phase 1 that can land in
// a session without build budget. The receiver cut-over (step 2) and the
// per-cluster link verification (step 3) still need the build loop.
//
// ---------------------------------------------------------------------------
// THE CONSTRAINT THAT SHAPES THIS FILE: INDEX IDENTITY IS LOAD-BEARING
//
// `f_pc_profile_lst.cpp` is not a set of profiles, it is an ORDERED TABLE, and
// every WW row carries a comment saying so:
//
//     // §45 ExtVeg: index MUST match fpcNm_EXT_VEG_e (0x31D).
//     &g_profile_EXT_VEG.base.base,
//
// So the obvious plugin shape -- "register each profile at load" -- is WRONG
// here, and wrong in the worst way: an append-style registry would place actors
// at whatever index they happened to register in, and every DZR placement
// referencing an actor by index would silently spawn a DIFFERENT actor. No
// crash, no error, just the wrong thing in the world.
//
// Registration therefore carries the index EXPLICITLY, and a mismatch refuses
// rather than renumbers. `fpcNm_*_e` remains the single authority for what an
// index means; this file never assigns one.
//
// ---------------------------------------------------------------------------
// KILL SWITCH
//
// `WW_PROFILE_REGISTER_ENABLED`. Off, the receiver's static table is untouched
// and this file contributes nothing -- which is the state it lands in. Step 19's
// rule is one migration at a time, each individually revertible; a shim that
// takes effect the moment it compiles is not revertible, it is a fait accompli.
// ============================================================================
#include "f_pc/f_pc_profile_lst.h"
#include "f_op/f_op_actor_mng.h"

#if TARGET_PC

namespace {

// Off by default. See KILL SWITCH above: landing != activating.
bool s_enabled = false;

// One row per profile the plugin will own. The index is stated, never derived.
//
// ORDER IN THIS TABLE IS NOT MEANINGFUL -- that is the point. The receiver's
// table is positional; this one is keyed, so a row moving here changes nothing
// and a row moving THERE would change everything. Keying the data is what makes
// the migration safe to do incrementally.
struct WwProfileRow {
    u16 index;              // fpcNm_*_e — the authority, mirrored not invented
    const char* name;       // for diagnostics; never used for lookup
};

// The 20 `g_profile` symbols the linker reported unresolved when the WW layer
// is excluded (§577, build/ww-excluded-link.log), plus the 3 ext-actor
// registration entries that share this cluster's shape (§580).
//
// TRANSCRIBED 2026-08-07 (step 19 Phase 1 step 2). 19 of 20 carry an index;
// `O` does not, and pendingRows() correctly returns 1. Every value was READ or
// DERIVED from the receiver's table, never written from memory -- and the two
// are labelled separately below, because a derived value and a read one are
// different claims and this campaign's record on collapsing them is not good.
constexpr u16 kIndexUnset = 0xFFFF;

const WwProfileRow kRows[] = {
    // READ DIRECTLY from `f_pc_profile_lst.cpp`'s own "index MUST match"
    // comments -- the pairing was verified at authoring time, next to the entry
    // it governs. 14 rows.
    {0x31F, "ESA"},          {0x31E, "EXT_SPAN"},
    {0x31D, "EXT_VEG"},      {0x321, "KAMOME"},
    {0x320, "KB"},           {0x31C, "KNOB00"},
    {0x32C, "LAMP"},         {0x32A, "NPC_BA1"},
    {0x327, "NPC_LS1"},      {0x329, "NPC_ZL1"},
    {0x32F, "OBJ_OTBLE"},    {0x328, "OBJ_TORIPOST"},
    {0x32E, "SPC_ITEM01"},   {0x32B, "WW_DEMO00"},

    // READ from `include/f_pc/f_pc_name.h`, the enum's own per-entry hex
    // comments -- the authoritative source, found only after I had already
    // DERIVED these from a range comment plus a five-wide gap. The derivation
    // was correct; it was still the weaker claim, and the read supersedes it.
    // 0x322..0x326 confirmed IN DECLARATION ORDER.
    {0x322, "NPC_BM1"},      {0x323, "NPC_BM2"},
    {0x324, "NPC_BM3"},      {0x325, "NPC_BM4"},
    {0x326, "NPC_BM5"},

    // RESTORED 2026-08-07 by Engine (Housing covering) — the row above was
    // removed on a correct diagnosis and a wrong conclusion, and the shim
    // reported READY without it.
    //
    // `g_profile_O` indeed does not exist, and the truncation diagnosis was
    // right: the extracting regex was `g_profile_[A-Z0-9_]+`, UPPERCASE-only,
    // so it matched `g_profile_O` and stopped at the lowercase `b`. But the
    // string it truncated is a REAL symbol, not a phantom:
    //
    //     g_profile_Obj_Mshokki   d_a_obj_mshokki.cpp:416 (defined)
    //                             f_pc_profile_lst.cpp:1655 (registered)
    //     fpcNm_Obj_Mshokki_e     f_pc_name.h:827  = 0x32D
    //
    // So the cluster IS 20. Deleting the row rather than repairing the name
    // turned a naming artifact into a MISSING REGISTRATION, and because
    // pendingRows() only counts rows that are PRESENT-but-unindexed, an absent
    // row is invisible to it: the shim returned 0 pending and isEnabled() would
    // have gone true with 19 of 20 actors covered. Flipping the switch there
    // does not fail loudly -- Obj_Mshokki simply never registers.
    //
    // The index corroborates the restoration independently: 0x32D is the exact
    // gap in this table's own sequence (0x32C LAMP -> 0x32E SPC_ITEM01).
    {0x32D, "Obj_Mshokki"},
};

// COUNT CONTROL. The table must cover every `g_profile` the linker reported, and
// "I transcribed them all" is the claim that just failed. Asserted at compile
// time so a dropped row cannot reach a build.
static_assert(sizeof(kRows) / sizeof(kRows[0]) == 20,
              "profile cluster is 20 (build/ww-excluded-link.log); a row was "
              "dropped or added without updating this control");

}  // namespace

// ---------------------------------------------------------------------------
// Public surface. Both are no-ops until the kill switch is on AND the indices
// are transcribed -- so the file is inert in the tree it lands in.
// ---------------------------------------------------------------------------

// Returns the number of rows still awaiting an index. NON-ZERO MEANS NOT READY:
// the cut-over is incomplete and the receiver's static table must remain
// authoritative. A caller that ignores this and registers anyway reintroduces
// the renumbering hazard this file exists to prevent.
int dWwProfileRegister_pendingRows() {
    int pending = 0;
    for (const WwProfileRow& row : kRows) {
        if (row.index == kIndexUnset) {
            ++pending;
        }
    }
    return pending;
}

bool dWwProfileRegister_isEnabled() {
    return s_enabled && dWwProfileRegister_pendingRows() == 0;
}

// ---------------------------------------------------------------------------
// MECHANISM FINDING (Engine, 2026-08-07) — "install" CANNOT BE A REGISTRATION.
//
// This function was specified as "register each row at its index". Reading the
// receiver rather than assuming it, there is nothing to register INTO on our
// build:
//
//   f_pc_profile.cpp:13  (#ifndef __MWERKS__ — the PC path, ours)
//     process_profile_definition DUSK_CONST* DUSK_CONST* DUSK_CONST
//         g_fpcPf_ProfileList_p = g_fpcPfLst_ProfileList;
//
//   f_pc_profile.cpp:21
//     process_profile_definition DUSK_CONST* fpcPf_Get(s16 i_profname) {
//         return g_fpcPf_ProfileList_p[index];
//     }
//
// `DUSK_CONST` is `const` (global.h:258), so on PC the list pointer is CONST and
// STATICALLY INITIALISED. The mutable, `ModuleProlog`-assigned pointer is the
// CONSOLE path (`__MWERKS__`), which we do not build. Swapping the pointer or
// writing into the array is not available: the objects are const and land in
// read-only data.
//
// THE VIABLE SHAPE IS TO HOOK THE LOOKUP, NOT MUTATE THE TABLE.
// `fpcPf_Get()` is the single point where an index becomes a profile. A hook
// there returns the plugin's profile for a WW index and delegates every other
// index to the original -- which is what the plugin ABI exists to do, and it
// preserves index identity by construction rather than by discipline. It is
// also revertible in the way step 19 requires: unhooking restores the receiver
// exactly, with no table left half-written.
//
// ADOPTED by the user 2026-08-07, after verifying the symbol is reachable in a
// SHIPPED build and not merely in ours:
//
//     fpcPf_Get           our fork exe: present (exported)   upstream exe: present
//     ?fpcPf_Get          our fork exe: present              upstream exe: present
//
// That check is the one that mattered. A hook design is only as good as the
// symbol it attaches to, and an inlined or stripped `fpcPf_Get` would have made
// this unbuildable on stock dusklight while working perfectly here.
//
// WHAT THIS DESIGN DOES NOT SOLVE, stated so it is not overread: the plugin
// attaching cleanly says nothing about host BEHAVIOUR. HR-1 (aurora LOAD_INDX)
// still applies -- the plugin will load and register fine on a stock build and
// still crash when WW content reaches the indexed-matrix path, because that bug
// is below the plugin boundary and cannot be hooked around from above.
//
// Also noted while reading, not fixed here: `fpcPf_Get` performs NO bounds
// check. An out-of-range index reads past the array. Not this file's business,
// but it is the reason index identity is load-bearing rather than merely tidy.
// ---------------------------------------------------------------------------
void dWwProfileRegister_install() {
    if (!dWwProfileRegister_isEnabled()) {
        return;  // inert: static table stays authoritative
    }
    // Deliberately empty. See MECHANISM FINDING above: the specified approach
    // (register into the table) is not implementable on PC, and the replacement
    // (hook fpcPf_Get) has not been ruled on. Reaching here is currently a
    // no-op by design rather than by omission.
}

#endif  // TARGET_PC
