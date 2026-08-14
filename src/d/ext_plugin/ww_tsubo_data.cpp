// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_tsubo.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW tsubo port — data TU (§803; the WW_ITEM §740 shape). Includes the
// generated donor tables and exports two accessors. LANDS DARK: nothing
// consumes it until the actor TU (tsubo port step 2). Regenerate the .inc
// with tools/ww_crew_restoration_skeleton/gen_tsubo_data.py.
// ============================================================

#include "d/ext_plugin/ww_tsubo_data.h"

#include "ww_tsubo_data.inc"

const WwTsuboData* dWwTsuboData_get(u32 i_subtype) {
    if (i_subtype >= 16) {
        return 0;   // donor table is exactly 16 rows; OOB refuses (№31-C)
    }
    return &l_wwTsuboData[i_subtype];
}

const char* dWwTsuboData_getArcName(u32 i_subtype) {
    if (i_subtype >= 16) {
        return 0;
    }
    return l_wwTsuboArcname[i_subtype];
}

const WwTsuboSpecBoko* dWwTsuboData_getSpecBoko(int i_idx) {
    if (i_idx < 0 || i_idx >= 3) {
        return 0;   // donor table is exactly 3 rows; OOB refuses (№31-C)
    }
    return &l_wwTsuboSpecBoko[i_idx];
}

const WwTsuboAttrSpine* dWwTsuboData_getAttrSpine() {
    return &l_wwTsuboAttrSpine;
}
