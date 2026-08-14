// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_item_data.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW d_a_item port — step 2 foundation: the donor item resource table as a
// compiled TU (docs/ww-item-actor-port-plan.md; plan step 1 generated the
// rows, this TU makes them linkable). LANDS DARK: only the accessor is
// exported; no actor consumes it until step 2's daItem port. Regenerate the
// .inc with tools/ww_crew_restoration_skeleton/gen_item_data.py — never
// hand-edit (256-row count control lives in the generator).
// ============================================================

#include "d/ext_plugin/ww_item_data.h"

#include "ww_item_data.inc"

const WwItemResource* dWwItemData_getResource(u32 i_donorItemNo) {
    if (i_donorItemNo >= 0x100) {
        return 0;   // donor table is exactly 0x100 rows; OOB refuses (№31-C)
    }
    return &l_wwItemResource[i_donorItemNo];
}

const WwFieldItemRes* dWwItemData_getFieldRes(u32 i_donorItemNo) {
    if (i_donorItemNo >= 0x100) {
        return 0;
    }
    return &l_wwFieldItemRes[i_donorItemNo];
}

const WwItemInfo* dWwItemData_getInfo(u32 i_donorItemNo) {
    if (i_donorItemNo >= 0x100) {
        return 0;
    }
    return &l_wwItemInfo[i_donorItemNo];
}
