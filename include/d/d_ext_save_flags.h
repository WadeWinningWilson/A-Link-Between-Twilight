#ifndef D_EXT_SAVE_FLAGS_H
#define D_EXT_SAVE_FLAGS_H

#include "global.h"

// ============================================================
// §303 — the donor event-flag namespace (pitfall E, the ceiling).
// Donor story actors call dComIfGs_is/on/offEventBit with DONOR bit
// indices; routed into TP's table those indices mean unrelated things
// (wrong variant/dialogue/progression). This block is the donor's own
// event-flag space, byte-for-byte the donor layout, living beside —
// never inside — TP's tables.
//
// Donor receipts (decomp, verbatim):
//   dSv_event_c { u8 mFlags[0x100]; }  (donor d_save.h:698, size-asserted)
//   onEventBit:  mFlags[no >> 8] |=  (no & 0xFF)   (d_save.cpp:1189)
//   offEventBit: mFlags[no >> 8] &= ~(no & 0xFF)   (d_save.cpp:1194)
//   isEventBit:  mFlags[no >> 8] &   (no & 0xFF)   (d_save.cpp:1199)
//   setEventReg / getEventReg: whole-masked-byte register ops
// i.e. high byte = byte index, LOW BYTE IS A MASK (0x2A20 = byte 0x2A,
// mask 0x20) — donor semantics, do not "fix" to bit numbers.
//
// №106 filename law: no "ww" in shipping paths — hence d_ext_save_flags.
// Routing for donor TUs lives in d_ext_save_flags_route.h (include LAST).
// ============================================================

class dExtSvFlags_event_c {
public:
    void init();
    void onEventBit(u16 i_no);
    void offEventBit(u16 i_no);
    BOOL isEventBit(u16 i_no);
    void setEventReg(u16 i_reg, u8 i_no);
    u8 getEventReg(u16 i_reg);

    u8* getPEventBit() { return mFlags; }

    /* 0x0 */ u8 mFlags[0x100];  // donor dSv_event_c size, verbatim
};

// Free-function surface matching the dComIfGs_* call shapes the donor
// sources use (so the route header is a pure symbol remap).
BOOL dExtWwSv_isEventBit(u16 i_no);
void dExtWwSv_onEventBit(u16 i_no);
void dExtWwSv_offEventBit(u16 i_no);
void dExtWwSv_setEventReg(u16 i_reg, u8 i_no);
u8 dExtWwSv_getEventReg(u16 i_reg);

// Lifecycle + persistence.
// reset(): new-game donor state (zero — donor init).
// store/restore(): serialize the block (magic+version+0x100 bytes) into the
// caller's buffer / from it. [PORT-INTEGRATION — History's wire, one line
// each]: call beside the quest-log card IO the way autosave does
// (g_mDoMemCd_control.save(...), src/dusk/autosave.cpp:70 pattern), or via
// a sidecar beside MemoryCard*.raw — either carries the donor semantics
// ("256 bytes live and die with the save slot").
void dExtWwSv_reset();
u32 dExtWwSv_storeSize();                    // bytes store() writes
void dExtWwSv_store(u8* o_buf);
BOOL dExtWwSv_restore(const u8* i_buf);      // FALSE = bad magic/version
                                             // (state left at reset())

// Raw block access for the slot-file (no header — the sidecar file carries
// one header for all three slots).
u32 dExtWwSv_rawSize();                      // 0x100, donor block size
void dExtWwSv_exportRaw(u8* o_buf);          // live block -> caller buffer
void dExtWwSv_importRaw(const u8* i_buf);    // caller buffer -> live block

// ============================================================
// §305 — sidecar persistence (wire ①). One donor block PER QUEST-LOG
// SLOT (0-2, same dataNum as dSv_info_c::memory_to_card/card_to_memory)
// in one sidecar file beside the card data:
//   <data>/<USA|EUR|JAP>/event_ext.sav
// Deliberately OUTSIDE the card image: mSaveBuffer is exactly
// QUEST_LOG_SIZE*3 with no slack, and mDoMemCdRWm_SetCheckSumGameData
// covers the in-slot bytes — riding slack would sit outside checksum
// coverage. The sidecar fails safe on its own 'WWEV' magic instead
// (bad/missing file -> reset() = donor new-game state, TP save untouched).
// Implementation: src/dusk/event_ext_store.cpp (dusk::io + dusk::data).
// ============================================================
BOOL dExtWwSvIo_restoreSlot(int i_dataNum);  // FALSE = no/bad sidecar
                                             // (live block left at reset())
void dExtWwSvIo_storeSlot(int i_dataNum);    // read-modify-write slot
void dExtWwSvIo_eraseSlot(int i_dataNum);    // file-select erase mirror
void dExtWwSvIo_copySlot(int i_from, int i_to);  // file-select copy mirror

#endif /* D_EXT_SAVE_FLAGS_H */
