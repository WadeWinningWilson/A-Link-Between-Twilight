#ifndef D_EXT_PLUGIN_WW_ITEM_DATA_H
#define D_EXT_PLUGIN_WW_ITEM_DATA_H

// ============================================================
// WW d_a_item port — step 2 foundation (docs/ww-item-actor-port-plan.md).
// The DONOR's dItem_data_item_resource, field-for-field (WW DP
// d_item_data.h:271-284, struct size 0x24 there; pointers widen on PC — the
// LAYOUT is donor law, the packing is the platform's). Data rows are
// GENERATED (ww_item_data.inc via gen_item_data.py): donor arc names, donor
// resource indices, donor heap sizes — trap #1/#2/#6 handled at generation,
// not at consumption.
// mItemMesgNum is carried VERBATIM and known-dead (L4 items measurement:
// zero donor call sites; name-string band 0x191-0x1F0) — kept because the
// row is donor data, commented so nobody maps through it.
// ============================================================

#include "dolphin/types.h"

struct WwItemResource {
    const char* mArcname;   // donor arc (e.g. "Always", "Vlupy")
    const char* mTexture;   // inventory icon in itemicon.arc
    s16 mBmdIdx;
    s16 mSrtIdx;            // BTK
    s16 mSrtIdx2;           // BTK
    s16 mTevIdx;            // BRK
    s16 mTevIdx2;           // BRK
    s16 mBckIdx;
    s8 mTevFrm;             // hold BRK on this frame if >= 0
    s16 mItemMesgNum;       // donor-dead field (see banner) — verbatim
    u32 mUnused;
    u32 mUnknown;
    u16 mHeapSize;
};

// Donor dItem_data_field_item_res (d_item_data.h field struct) — the FIELD
// item actor's model table (d_a_item reads THIS, not item_resource; the two
// tables serve different consumers in the donor and both port).
struct WwFieldItemRes {
    const char* mArc;
    s16 mBmdIdx;
    const char* mBmdMember;   // derived member name (the receiver resolver's key)
    s16 mSrtIdx;              // BTK
    const char* mSrtMember;
    s16 mSrtIdx2;             // BTK
    s16 mTevIdx;              // BRK
    const char* mTevMember;
    s16 mTevIdx2;             // BRK
    s16 mBckIdx;
    const char* mBckMember;
    s8 mTevFrm;               // donor-unused in this table (item_resource's wins)
    u32 mUnknown;
    u16 mHeapSize;
};

// Donor dItem_data_item_info — dimensions + flags (getH/getR/getShadowSize/chkFlag).
struct WwItemInfo {
    u8 mShadowSize;
    u8 mCollisionH;
    u8 mCollisionR;
    u8 mFlag;
};

// Row accessors. Donor id space ONLY (R5 item.id_space: never a TP id).
const WwItemResource* dWwItemData_getResource(u32 i_donorItemNo);
const WwFieldItemRes* dWwItemData_getFieldRes(u32 i_donorItemNo);
const WwItemInfo* dWwItemData_getInfo(u32 i_donorItemNo);

#endif /* D_EXT_PLUGIN_WW_ITEM_DATA_H */
