#ifndef D_EXT_QUICK_EQUIP_H
#define D_EXT_QUICK_EQUIP_H

#if TARGET_PC

#include "dolphin/types.h"

// Session quick-equip socket registry (WW-agnostic).
// Pages × 24 sockets; callers claim by behavior kind. No WW names here.
// Ownership/persistence stays with the mod; this store is runtime-only.
// Draw packs occupied sockets only; capacity remains pages×24.

constexpr u8 dQe_kSlotsPerPage = 24;
constexpr u8 dQe_kMinPages = 1;
constexpr u8 dQe_kMaxPages = 4;
constexpr u8 dQe_kDefaultPages = 2;
constexpr u8 dQe_kBagCapacity = 8;
constexpr u8 dQe_kMaxBags = 8;

enum dQeKind : u8 {
    dQeKind_Empty = 0,
    dQeKind_InvSlot_Z = 1,    // tpInvSlot → SELECT_ITEM_DOWN
    dQeKind_SwordEquip = 2,   // iconItemNo → sword equip
    dQeKind_ShieldEquip = 3,  // iconItemNo → shield equip
    dQeKind_ZSelect = 4,      // tpInvSlot → SELECT_ITEM_DOWN (mod overflow)
    dQeKind_Custom = 5,       // reserved; no-op confirm in v1
    dQeKind_Bag = 6,          // root socket; A opens nested bag children
};

enum dQeFlags : u16 {
    dQeFlag_None = 0,
    dQeFlag_BuiltinSeed = 1 << 0,  // TP seed; cleared/reseeded on wheel open
    dQeFlag_ModClaim = 1 << 1,     // mod/session claim; cleared by rescan
};

struct dQeSocketDesc {
    u16 id;          // opaque handle; 0 = vacant
    dQeKind kind;
    u8 page;         // 0..pageCount-1
    u8 slot;         // 0..23
    u8 tpInvSlot;    // inventory slot for InvSlot_Z / ZSelect (0xFF if none)
    u8 iconItemNo;   // TP icon path; 0xFF = blank/empty draw
    u16 flags;
};

bool dQe_setPageCount(u8 pages);
u8 dQe_getPageCount();

bool dQe_claim(const dQeSocketDesc& desc);
bool dQe_clear(u8 page, u8 slot);
bool dQe_clearById(u16 id);
void dQe_clearByFlag(u16 flagMask);
void dQe_clearAll();

const dQeSocketDesc* dQe_peek(u8 page, u8 slot);

/** First free slot on page, or 0xFF if full. */
u8 dQe_findFreeSlot(u8 page);

/**
 * Clear builtin-seeded sockets and re-register TP tools on page 0 only
 * (vanilla item-wheel contents). Page 1+ are mod sockets — no TP sword/shield
 * seed. Leaves mod-claimed sockets untouched.
 */
void dQe_seedTpBuiltin();

/** Nested bag children (up to dQe_kBagCapacity per bag root id). */
bool dQe_claimBagChild(u16 bagId, u8 childSlot, const dQeSocketDesc& child);
bool dQe_clearBag(u16 bagId);
const dQeSocketDesc* dQe_peekBagChild(u16 bagId, u8 childSlot);
u8 dQe_countBagOccupied(u16 bagId);

/** Deep-link: claim/overwrite a ZSelect/InvSlot_Z socket for assign-to-Z. */
bool dQe_deepLinkAssignZ(u8 tpInvSlot, u8 iconItemNo, u16 opaqueId);

#endif  // TARGET_PC

#endif /* D_EXT_QUICK_EQUIP_H */
