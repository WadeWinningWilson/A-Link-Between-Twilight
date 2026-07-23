#ifndef D_EXT_STATUS_H
#define D_EXT_STATUS_H

#if TARGET_PC

#include "dolphin/types.h"

// Ext Status registry — WW-agnostic Collect sibling (Tools / Quest / Atlas).
// Labels/icons come from callers; no WW place/item name literals here.

constexpr u8 dExtStatus_kMaxRows = 48;
constexpr u8 dExtStatus_kTabCount = 3;

enum dExtStatusTab : u8 {
    dExtStatusTab_Tools = 0,
    dExtStatusTab_Quest = 1,
    dExtStatusTab_Atlas = 2,
};

enum dExtStatusKind : u8 {
    dExtStatusKind_Empty = 0,
    dExtStatusKind_Passive = 1,   // quest strip / songs / pearls
    dExtStatusKind_Usable = 2,   // tools browse; may deep-link to dQe_
    dExtStatusKind_Bag = 3,       // bag root browse
    dExtStatusKind_Chart = 4,     // atlas list row
    dExtStatusKind_Mark = 5,      // atlas mark
};

enum dExtStatusFlags : u16 {
    dExtStatusFlag_None = 0,
    dExtStatusFlag_DebugSeed = 1 << 0,
    dExtStatusFlag_ModClaim = 1 << 1,
};

struct dExtStatusRow {
    u16 id;               // opaque; 0 = vacant
    dExtStatusTab tab;
    dExtStatusKind kind;
    u8 iconItemNo;        // TP icon path or 0xFF
    u8 tpInvSlot;         // optional vehicle; 0xFF none
    u16 flags;
    char label[32];       // caller-supplied display (may be empty)
};

bool dExtStatus_claim(const dExtStatusRow& row);
bool dExtStatus_clearById(u16 id);
void dExtStatus_clearByFlag(u16 flagMask);
void dExtStatus_clearAll();

u8 dExtStatus_countTab(dExtStatusTab tab);
const dExtStatusRow* dExtStatus_peekTab(dExtStatusTab tab, u8 index);

/** Debug stubs for empty-shell playtest (neutral labels). */
void dExtStatus_seedDebug();

/**
 * Tools-tab deep-link: if row is Usable with tpInvSlot, assign via dQe_.
 * Returns true if assign ran.
 */
bool dExtStatus_tryDeepLinkZ(u16 rowId);

/** Rescan enabled mod folders for ext_inv/claims.ini (session claims). */
void dExtInv_rescanClaims();

#endif  // TARGET_PC

#endif /* D_EXT_STATUS_H */
