/**
 * d_a_itembase_static.cpp
 *
 */

#include "d/dolzel.h" // IWYU pragma: keep

#include "d/d_a_itembase_static.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/actor/d_a_itembase.h"
#include "d/d_item_data.h"
#if TARGET_PC
#include "d/d_ww_itemmdl_pc.h"
#include "dusk/logging.h"  // §66 probe
#endif

u8 daItemBase_c::getItemNo() {
    return m_itemNo;
}

static void dummy(dItem_data* data) {
    data->getR(0);
    data->getH(0);
}

void daItemBase_c::hide() {
#if TARGET_PC
    // §66 arm 6 — demo items only (field pickups flash-cycle show/hide).
    if (chkDraw() && fopAcM_GetName(this) == fpcNm_Demo_Item_e) {
        DuskLog.warn("[ItemBase] §66 hide: item={}", (int)getItemNo());
    }
#endif
    cLib_offBit<u8>(field_0x92b, 1);
}

void daItemBase_c::show() {
#if TARGET_PC
    if (!chkDraw() && fopAcM_GetName(this) == fpcNm_Demo_Item_e) {
        DuskLog.warn("[ItemBase] §66 show: item={}", (int)getItemNo());
    }
#endif
    cLib_onBit<u8>(field_0x92b, 1);
}

void daItemBase_c::changeDraw() {
    if (chkDraw()) {
        hide();
    } else {
        show();
    }
}


bool daItemBase_c::chkDraw() {
    return cLib_checkBit<u8>(u8(field_0x92b), 1);
}

void daItemBase_c::dead() {
    cLib_onBit<u8>(field_0x92b, 2);
}

bool daItemBase_c::chkDead() {
    return cLib_checkBit<u8>(u8(field_0x92b), 2);
}

int CheckItemCreateHeap(fopAc_ac_c* i_this) {
    daItemBase_c* a_this = static_cast<daItemBase_c*>(i_this);

    u8 item_no = a_this->getItemNo();
#if TARGET_PC
    // §66 — clothes kit: do NOT feed host WEAR_KOKIRI's F_gD_rupy table into
    // CreateItemHeap. create() already resLoad'd the kit arc; CreateItemHeap's
    // clothes branch resolves ModelData via ExtNpcMount acquire (finished
    // bmd3). Pass kit arc + cleared anm indices so the call site matches the
    // model that actually gets built.
    if (dWwItemmdl_clothesBundleForItem(item_no)) {
        const char* kit_arc = dWwItemmdl_clothesBundleArcName();
        const char* kit_model = dWwItemmdl_clothesBundleModelName();
        const char* arc = kit_arc != NULL ? kit_arc : dItem_data::getArcName(item_no);
        const int result =
            a_this->CreateItemHeap(arc, /*bmd*/ -1, /*btk*/ -1, /*bpk*/ -1, /*bck*/ -1,
                                   /*bxa*/ -1, /*brk*/ -1, /*btp*/ -1);
        DuskLog.warn("[ItemBase] §66 heap: item={} kit=1 arc='{}' model='{}' → {}", (int)item_no,
                     arc != NULL ? arc : "NULL", kit_model != NULL ? kit_model : "NULL",
                     result != 0 ? "OK" : "FAIL");
        return result;
    }
#endif
    const int result =
        a_this->CreateItemHeap(dItem_data::getArcName(item_no), dItem_data::getBmdName(item_no),
                               dItem_data::getBtkName(item_no), dItem_data::getBpkName(item_no),
                               dItem_data::getBckName(item_no), dItem_data::getBxaName(item_no),
                               dItem_data::getBrkName(item_no), dItem_data::getBtpName(item_no));
#if TARGET_PC
    {
        const char* arc = dItem_data::getArcName(item_no);
        DuskLog.warn("[ItemBase] §66 heap: item={} kit=0 arc='{}' bmd={} → {}", (int)item_no,
                     arc != NULL ? arc : "NULL", (int)dItem_data::getBmdName(item_no),
                     result != 0 ? "OK" : "FAIL");
    }
#endif
    return result;
}

int CheckFieldItemCreateHeap(fopAc_ac_c* i_this) {
    daItemBase_c* a_this = static_cast<daItemBase_c*>(i_this);

    u8 item_no = a_this->getItemNo();
    return a_this->CreateItemHeap(
        dItem_data::getFieldArc(item_no), dItem_data::getItemBmdName(item_no),
        dItem_data::getItemBtkName(item_no), dItem_data::getItemBpkName(item_no),
        dItem_data::getItemBckName(item_no), dItem_data::getItemBxaName(item_no),
        dItem_data::getItemBrkName(item_no), dItem_data::getItemBtpName(item_no));
}

const daItemBase_data daItemBase_c::m_data = {
    -4.5f,  // mGravity
    0.62f,  // mGroundReflect
    45.0f,  // mLaunchSpeed
    10.0f,  // mScalingTime
    6.0f,   // mSpeedH
    1,      // mFlashCycleTime
    240,    // mWaitTime
    60,     // mDisappearTime
    4000,   // mRotateXSpeed
    120,    // mRotateYSpeed
    -4.0f,  // mHeartFallSpeed
    3.5f,   // mHeartAmplitude
    1100,   // mHeartFallCycleTime
    3000,   // mHeartTilt
    23.0f,  // mGetDemoLaunchSpeed
    -6.0f,  // mGetDemoGravity
    13,     // mSimpleExistTime
    10,     // mNoGetTime
};
