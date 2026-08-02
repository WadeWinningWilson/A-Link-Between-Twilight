#ifndef D_EXT_SCOPE_MSG_H
#define D_EXT_SCOPE_MSG_H

// ============================================================================
// §245  Scope-message subsystem (WW restoration)
// ----------------------------------------------------------------------------
// Wind Waker drives its "telescope scope" text box through a small amount of
// global game state that the TP-based port never had:
//
//   * mScopeMesgStatus  (WW d_com_inf_game.h @ 0x492B) — the scope box's own
//                        fopMsgStts_* state, distinct from the normal mesg
//                        status field.
//   * mScopeType        (WW d_com_inf_game.h @ 0x4945) — which scope prompt.
//   * daPyStts0_TELESCOPE_LOOK_e — the player-status bit set while Link is
//                        looking through the telescope item.
//
// The port's g_dComIfG_gameInfo struct is a fixed hardware-layout mirror of the
// TP build, so we must NOT grow it or repurpose its unknown `field_0xNNN`
// bytes (they may hold live TP state). The scope status is singleton global
// game state, so we back it with function-local statics here — one shared
// instance across every translation unit, header-only, zero struct edits.
//
// NOTE: WW re-zeroes mScopeMesgStatus on new-game / load. These statics default
// to 0 at boot, which is correct until the player telescope ITEM lands (that WW
// player-main state is a separate, deferred port — until it exists, the
// TELESCOPE_LOOK bit is never set, so every scope path below stays dormant but
// faithful). Re-zero-on-load is a follow-up once the telescope item is wired.
// ============================================================================

#include "dolphin/types.h"

// --- player status bit: WW daPyStts0_TELESCOPE_LOOK_e (value verbatim so
//     dComIfGp_checkPlayerStatus0() tests the same raw bit). ------------------
#define daPyStts0_TELESCOPE_LOOK_e 0x00200000

// --- scope box message status (WW dComIfGp_get/setScopeMesgStatus) -----------
inline u8& dExtScope_mesgStatusRef() {
    static u8 s_scopeMesgStatus = 0;
    return s_scopeMesgStatus;
}
inline u8 dComIfGp_getScopeMesgStatus() { return dExtScope_mesgStatusRef(); }
inline void dComIfGp_setScopeMesgStatus(u8 status) { dExtScope_mesgStatusRef() = status; }

// --- scope prompt type (WW dComIfGp_get/setScopeType) ------------------------
inline u8& dExtScope_typeRef() {
    static u8 s_scopeType = 0;
    return s_scopeType;
}
inline u8 dComIfGp_getScopeType() { return dExtScope_typeRef(); }
inline void dComIfGp_setScopeType(u8 type) { dExtScope_typeRef() = type; }

#endif /* D_EXT_SCOPE_MSG_H */
