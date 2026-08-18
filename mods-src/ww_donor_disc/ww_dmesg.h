#pragma once

// ============================================================================
// ww_dmesg.h — the DERIVED tier: WW's own game-tag dispatch.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_mesg.cpp `dMesg_tSequenceProcessor::do_tag` (207 lines) and
//            `::do_systemTagCode`; JMessage/processor.cpp for the base contract
// KIT-DONOR-REF: zeldaret/tww@2d094c26
// KIT-DONOR-STATUS: BEHAVIOURALLY SPECIFIED, BYTE-DIVERGENT
//
// 🔴 NEVER `byte-true: BYTE-TRUE`. Enforced twice — the Integrator's build gate
// and `row_store.py`'s filing check. See ww_jmessage.h for the §801 basis.
//
// ============================================================================
// WHAT THIS TIER IS FOR, AND WHY THE BASE LAYER ALONE RENDERS NOTHING
// ============================================================================
// The donor's dispatch is TWO-TIER. `WwJMessage::` (the base) handles only
// groups 0xFF/0xFE/0xFD/0xF6/0xF5 and system code 0x05. **Every WW GAME tag is
// dispatched HERE, by first refusal, before the base ever sees it.**
//
// Measured at `d_mesg.cpp:427`: the group is masked IN PLACE, not shifted —
//     int r30 = param_1 & 0xFF0000;
//     switch (r30) { case 0: case 0x10000: case 0x20000: case 0x30000: }
// so the four live game groups are 0, 1, 2, 3. Preserving the in-place mask
// matters: a shifted comparison would silently match nothing.
//
// ============================================================================
// ⚠ WHY "NOT IN dusklight_exports.def" IS NOT A BLOCKER HERE — I filed that as
// one and had to withdraw it, so the reasoning is recorded at the point of use
// ============================================================================
// Five of this tier's eight dependencies are **`inline`** (see
// `d_com_inf_game.h:1867`: `inline TEXT_SPAN dComIfGs_getPlayerName() { return
// g_dComIfG_gameInfo.info.getPlayer()... }`). **An inline function has NO
// linkable symbol, so its absence from an export table is evidence of INLINING,
// never of unreachability.** The plugin includes the header and compiles its
// own copy, which links against `?g_dComIfG_gameInfo@@3VdComIfG_inf_c@@A DATA`
// — exported. **The STATE is reachable although the ACCESSORS are not symbols.**
//
// The export table answers exactly one question: does a linkable symbol exist.
// It does NOT answer "is this reachable" in either direction — `getStartStageName`
// was exported and still unhookable because all eight consumers inlined it.
//
// DEPENDENCY SET (8 — do_tag is 207 lines, not the 40 that are obvious):
//   INLINE, via exported g_dComIfG_gameInfo:
//     dComIfGs_getPlayerName · dComIfGs_getPalLanguage · dComIfGs_getEventReg
//     dComIfGp_roomControl_getStayNo · dComIfGp_setMesgCameraTagInfo
//   EXPORTED NORMALLY:
//     dComIfGp_getReverb
//   BRIDGE-OWED STUBS (already owned by this lane, not new work):
//     dComIfGp_setMesgAnimeTagInfo · fopMsgM_passwordGet
// ============================================================================

#include "ww_jmessage.h"

namespace WwDMesg {

// ---------------------------------------------------------------------------
// THE FOUR GAME GROUPS, as the donor masks them (in place, NOT shifted).
// ---------------------------------------------------------------------------
enum GameGroup {
    kGroupFont   = 0x00000,  // player name, out-font digits, text positioning
    kGroupSound  = 0x10000,  // message SE
    kGroupCamera = 0x20000,  // camera tag -> dComIfGp_setMesgCameraTagInfo
    kGroupAnime  = 0x30000,  // anime tag  -> dComIfGp_setMesgAnimeTagInfo
};

// ---------------------------------------------------------------------------
// GROUP 0's CODE TABLE — the donor cases 0, 1..29 and 41.
//
// ✅ THIS IS WHAT RETIRES THE "52 UNCLASSIFIED PAIRS". The corpus census found
// 1,048 tag instances / 52 distinct pairs that §308 neither handles nor lists,
// and its top three by volume were group-0 codes 0x0001 (x310), 0x0002 (x312)
// and 0x0008 (x214) — ALL inside 0..29. They were never exotic: **§308
// hand-maps three families; the donor cases thirty.** Handling them here closes
// the gap by construction rather than by extending a hand-map.
// ---------------------------------------------------------------------------
const uint16_t kCodePlayerName = 0;    // -> dComIfGs_getPlayerName
const uint16_t kCodeOutFontLo  = 1;    // 1..24: out-font / control family
const uint16_t kCodeOutFontHi  = 24;
const uint16_t kCodeTextPosLo  = 25;   // 25..29: screen->getTextPosX
const uint16_t kCodeTextPosHi  = 29;
const uint16_t kCodeEventReg   = 41;   // -> dComIfGs_getEventReg

// ⚠ THE PAL POSSESSIVE BRANCH IS DATA, NOT A RULE, and it is exactly the kind
// of thing that gets "simplified" out of a port and never noticed. On non-JPN
// builds with `getPalLanguage() == 1`, SIX specific messages append `'` or `s`
// to the player's name depending on its last letter. The list is closed and
// verbatim from the donor — it cannot be derived, only copied.
const uint16_t kPalPossessiveMsgNo[6] = {
    0x33b, 0xc8b, 0x1d21, 0x31d7, 0x37dd, 0x37de,
};
bool isPalPossessiveMsg(uint16_t msgNo);

// ---------------------------------------------------------------------------
// First-refusal dispatch, mirroring the donor's contract exactly: return TRUE
// when this tier consumed the tag, FALSE to let the base layer have it.
//
// **RETURNING TRUE FOR A TAG YOU DID NOT ACTUALLY HANDLE IS THE SILENT-DROP
// THIS PORT EXISTS TO END** — it is how §308's unmapped families vanish with
// neither path erroring. When in doubt, return false and let the base warn.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// STATE COMES IN AS A PARAMETER — the plugin tier NEVER fetches it.
//
// **THIS IS THE DESIGN THAT REMOVES A WHOLE FAILURE CLASS RATHER THAN GUARDING
// AGAINST IT.** The obvious alternative was for the plugin to resolve the
// exported DATA symbol `g_dComIfG_gameInfo` and read the player name at a
// struct offset. That works, and it would make this port depend on the layout
// of `dComIfG_inf_c -> info -> player -> name` — **which is exactly the shape
// that produced the `mMsgGroup` crash: a field read at 0x28 out of a struct
// that is 0x20 long, on every WW stage load.**
//
// It is unnecessary. `dExtDmesg_paginate` — the ONE function of §308's
// twenty-two that this port touches — is RECEIVER code and **already calls
// `dComIfGs_getPlayerName()` at `d_ext_dmesg.cpp:426`, on stock, shipping.**
// The join point already holds the state. So it passes it DOWN, and this tier
// stays pure logic with no resolve, no offsets, and no layout dependency.
//
// Anything NULL/zero here means "the caller could not supply it" — an arm that
// needs a field it did not get returns FALSE and lets the base layer warn,
// rather than substituting a default that would render as plausible wrong text.
// ---------------------------------------------------------------------------
struct TagState {
    const char* playerName;              // dComIfGs_getPlayerName(), or nullptr
    int         palLanguage;             // dComIfGs_getPalLanguage(); 1 = the
                                         // possessive case. -1 = not supplied.
    uint16_t    nowMsgNo;                // current message, for kPalPossessive
    uint32_t  (*getEventReg)(uint16_t);  // dComIfGs_getEventReg, or nullptr
    // Group 3. Unblocked by History/Bridge paying the stub: what was
    // `getMesgAnimeTagInfo() { return 0xFF; }` beside an empty setter — a
    // CONSTANT getter, i.e. a channel that could never carry a value — is now
    // ported from the donor's own mechanism. Supplied by the caller like the
    // rest; nullptr still means "not available", never "send 0xFF".
    void      (*setAnimeTag)(uint8_t);
    // Group 2, same shape (dComIfGp_setMesgCameraTagInfo).
    void      (*setCameraTag)(uint8_t);
    // Group 1. §308 already resolves this effect on stock —
    // `mDoAud_messageSePlay(code, NULL, 0)` at d_ext_dmesg.cpp:374 — so the
    // channel is known-good and needs no new donor work. Note §308's own
    // caveat, which travels with the port rather than being fixed by it:
    // there is no typewriter to time the cue to, so it fires when the box
    // opens. That is a PRESENTATION limit owned by the host layer, not a
    // tag-interpretation bug for this tier to solve.
    void      (*playMessageSe)(uint16_t);
};

// Emitted text sink. The tier PRODUCES characters and never owns a buffer —
// §308 owns presentation (916 of its 1,042 lines) and that division is the
// whole point of the host/engine split.
typedef void (*EmitFn)(void* user, const char* text, uint32_t len);

bool doTag(uint32_t packedTag, const void* payload, uint32_t payloadLen,
           const TagState& state, EmitFn emit, void* user);

}  // namespace WwDMesg
