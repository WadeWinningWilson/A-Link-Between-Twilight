/**
 * f_pc_profile.cpp
 * Framework - Process Profile
 */

#include "f_pc/f_pc_profile.h"

#if TARGET_PC
#include "d/ext_plugin/ww_profile_register.h"
#endif


#ifndef __MWERKS__
// Forward declare the static list from f_pc_profile_lst.cpp
extern process_profile_definition DUSK_CONST* DUSK_CONST g_fpcPfLst_ProfileList[];
// On PC: Direct pointer to static array
process_profile_definition DUSK_CONST* DUSK_CONST* DUSK_CONST g_fpcPf_ProfileList_p = g_fpcPfLst_ProfileList;
#else
// On Console: Pointer initialized by REL module prolog
process_profile_definition** g_fpcPf_ProfileList_p;
#endif

process_profile_definition DUSK_CONST* fpcPf_Get(s16 i_profname) {
#if TARGET_PC
    // ========================================================================
    // Step 19 Phase 1 — WW profile ownership.
    //
    // The single point where an actor index becomes a profile, so it is the
    // single point the WW layer needs in order to own its own actors without
    // anything being renumbered. Returns NULL for every index it does not own,
    // which is all of them until the kill switch is turned on AND the shim's
    // self-check has agreed with this table.
    //
    // This is the IN-TREE form. In the plugin it becomes a hook on this same
    // function -- same shape, different attachment -- which is why the receiver
    // edit is one line rather than twenty table rows.
    // ========================================================================
    if (process_profile_definition DUSK_CONST* p = dWwProfileRegister_lookup(i_profname)) {
        return p;
    }
#endif
    int index = i_profname;
#if TARGET_PC
    // ========================================================================
    // §706 Ruling 3(iii) rider (WAVE-1 row 5-batch shape): BOUNDS GUARD.
    // Under per-row relinquishment a NULL table row is a VALID answer — "that
    // id is owned elsewhere now" — so an OUT-OF-RANGE index must be its own,
    // LOUD failure. Before this guard an OOB index was a wild read, and the
    // two conditions were confusable by construction.
    // ========================================================================
    if (index < 0 || index >= fpcNm_MAX_NUM) {
        OS_REPORT("fpcPf_Get: profile index %d OUT OF RANGE (max %d) — returning NULL; "
                  "this is a caller bug, not a relinquished row\n",
                  index, (int)fpcNm_MAX_NUM);
        return NULL;
    }
#endif
    return g_fpcPf_ProfileList_p[index];
}
