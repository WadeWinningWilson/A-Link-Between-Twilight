#ifndef D_EXT_SAVE_FLAGS_ROUTE_H
#define D_EXT_SAVE_FLAGS_ROUTE_H

// ============================================================
// §303 — the SYMBOL ROUTE for donor story TUs (include LAST).
//
// Donor source stays VERBATIM: it calls dComIfGs_is/on/offEventBit with
// donor bit indices. This header remaps those SYMBOLS to the donor
// event-flag block (d_ext_save_flags) — the LinkRM→R_DL01 alias pattern
// applied to symbols: it translates WHERE THE BITS LIVE, never what
// happens. TP code never includes this header; TP tables untouched.
//
// RULES:
//  * include as the LAST include of a donor story TU (.cpp) ONLY —
//    never from a header, never in TP TUs;
//  * a TU that includes this routes ALL its event-bit traffic to the
//    donor block — donor TUs must not mix TP-table event bits (they
//    never do; that is the point);
//  * the eventReg forms route too (donor register-style reads).
// ============================================================

#include "d/d_ext_save_flags.h"

#define dComIfGs_isEventBit(no) dExtWwSv_isEventBit(no)
#define dComIfGs_onEventBit(no) dExtWwSv_onEventBit(no)
#define dComIfGs_offEventBit(no) dExtWwSv_offEventBit(no)
#define dComIfGs_setEventReg(reg, no) dExtWwSv_setEventReg(reg, no)
#define dComIfGs_getEventReg(reg) dExtWwSv_getEventReg(reg)

#endif /* D_EXT_SAVE_FLAGS_ROUTE_H */
