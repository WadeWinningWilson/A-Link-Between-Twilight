// ============================================================================
// ww_dmesg.cpp — derived-tier game-tag dispatch. See ww_dmesg.h for the §801
// DIVERGENT provenance, the in-place group mask, and why "absent from
// dusklight_exports.def" is not a reachability verdict for the inline deps.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_mesg.cpp dMesg_tSequenceProcessor::do_tag (NONMATCHING,
//            §801-admissible behavioural spec)
// KIT-DONOR-REF: zeldaret/tww@2d094c26
// KIT-DONOR-STATUS: BEHAVIOURALLY SPECIFIED, BYTE-DIVERGENT
//
// SCOPE OF THIS FILE AS IT STANDS: the group/code ROUTING is ported and the
// vocabulary is complete. The per-code EFFECTS are not yet wired — each arm
// that needs game state is marked and returns FALSE so the base layer still
// sees the tag. **That is deliberate: a `true` return means "consumed", and
// claiming to consume a tag whose effect is not implemented is precisely the
// silent drop this port exists to end.** Wiring the effects is the next
// increment and needs the inline accessors' headers included here.
// ============================================================================

#include "ww_dmesg.h"

namespace WwDMesg {

bool isPalPossessiveMsg(uint16_t msgNo) {
    for (int i = 0; i < 6; i++) {
        if (kPalPossessiveMsgNo[i] == msgNo) {
            return true;
        }
    }
    return false;
}

bool doTag(uint32_t packedTag, const void* payload, uint32_t payloadLen,
           const TagState& state, EmitFn emit, void* user) {
    (void)payload;
    (void)payloadLen;

    // IN-PLACE MASK, matching the donor exactly (`param_1 & 0xFF0000`). A
    // shifted compare would match nothing and every game tag would fall to the
    // base layer, which does not handle them — a total, silent failure.
    const uint32_t group = packedTag & 0xFF0000;

    switch (group) {
    case kGroupFont: {
        // The donor's inner switch is on the FULL packed tag, not the code —
        // which is equivalent here only because group 0 shifts to zero. Kept
        // in the donor's form so a future group does not inherit a wrong
        // assumption from this one.
        const uint16_t code = WwJMessage::tagCode(packedTag);
        if (code == kCodePlayerName) {
            // WIRED. The caller supplies the name; this tier never
            // fetches it. If it could not, we must NOT invent one —
            // returning false lets the base layer warn by group/code,
            // which is recoverable. Emitting a placeholder would render
            // as plausible wrong text and never be noticed.
            if (state.playerName == nullptr || emit == nullptr) {
                return false;
            }
            uint32_t n = 0;
            // NUL terminator. A heredoc once wrote a LITERAL 0x00 byte here
            // instead of the escape - the shell-escape hazard, in code.
            while (n < 64u && state.playerName[n] != 0) { n++; }
            emit(user, state.playerName, n);
            // PAL POSSESSIVE — donor-verbatim and NOT derivable: on
            // non-JPN with language 1, six specific messages append
            // an apostrophe or 's' by the name's last letter.
            if (state.palLanguage == 1 && isPalPossessiveMsg(state.nowMsgNo) && n > 0) {
                const char last = state.playerName[n - 1];
                const bool sibilant = (last == 's' || last == 'S' || last == 'z' ||
                                       last == 'Z' || last == 'x' || last == 'X');
                emit(user, sibilant ? "'" : "s", 1);
            }
            return true;
        }
        if (code >= kCodeOutFontLo && code <= kCodeOutFontHi) {
            return false;   // EFFECT NOT WIRED — out-font / control family
        }
        if (code >= kCodeTextPosLo && code <= kCodeTextPosHi) {
            return false;   // EFFECT NOT WIRED — needs screen->getTextPosX
        }
        if (code == kCodeEventReg) {
            // WIRED via the caller's accessor. No accessor = no claim.
            if (state.getEventReg == nullptr) { return false; }
            (void)state.getEventReg(code);
            return false;   // read proven reachable; EMISSION still owed
        }
        // A group-0 code the DONOR does not case either. Falling through to
        // the base is correct: its `unknown tag-value` warning names the group
        // and code, which is strictly better than swallowing it here.
        return false;
    }
    case kGroupSound:
        // WIRED. The tag's CODE is the SE id. §308 proves this effect is
        // reachable on stock today (mDoAud_messageSePlay at :374), so this is
        // a delegation, not a new capability.
        if (state.playMessageSe == nullptr) {
            return false;   // no channel — let the base name the tag rather
        }                   // than dropping a cue with nothing reported
        state.playMessageSe(WwJMessage::tagCode(packedTag));
        return true;
    case kGroupCamera:
        // WIRED. The tag's CODE is the camera id; the donor passes it straight
        // to the receiver's channel and keeps no state of its own here.
        if (state.setCameraTag == nullptr) {
            return false;   // no channel supplied — let the base warn, do not
        }                   // silently swallow a camera cue
        state.setCameraTag((uint8_t)(WwJMessage::tagCode(packedTag) & 0xFF));
        return true;
    case kGroupAnime:
        // WIRED — the Bridge stub is paid. Worth remembering WHAT it was:
        // `getMesgAnimeTagInfo()` returned a CONSTANT 0xFF beside an empty
        // setter, so the channel could never carry a value and every anime tag
        // was lost with nothing reporting it. Guarding on nullptr here keeps
        // that same failure LOUD rather than reintroducing a silent default.
        if (state.setAnimeTag == nullptr) {
            return false;
        }
        state.setAnimeTag((uint8_t)(WwJMessage::tagCode(packedTag) & 0xFF));
        return true;
    default:
        // Not a game group at all — 0xFF/0xFE/0xFD/0xF6/0xF5 belong to the
        // base layer by design. Never claim these.
        return false;
    }
}

}  // namespace WwDMesg
