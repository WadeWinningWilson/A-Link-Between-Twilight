#pragma once

// ============================================================================
// ww_jmessage.h — WW's OWN message processor, ported alongside TP's.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JMessage/processor.cpp + resource.cpp (NONMATCHING —
//            admissible as a BEHAVIOURAL SPEC per the §801 ruling of
//            2026-08-16, which completeness-tested both: 61 and 10 functions,
//            ZERO suspect stubs, on a test with a live negative control);
//            control.cpp + data.cpp (MATCHED) for layout and accessors
// KIT-DONOR-REF: zeldaret/tww@2d094c26
// KIT-DONOR-STATUS: BEHAVIOURALLY SPECIFIED, BYTE-DIVERGENT
//
// 🔴 THE PROVENANCE LABEL IS LOAD-BEARING, NOT DECORATION. Work derived from
// those two bodies MUST NEVER carry `byte-true: BYTE-TRUE`. It stays
// DIVERGENT, and that is now enforced in two places independently: the
// Integrator REFUSES a mislabelled BUILD-QUEUE row at the build gate, and
// `row_store.py` REFUSES `portable: NONMATCHING` together with
// `byte-true: BYTE-TRUE` at filing time. **"Behaviourally specified" and
// "byte-true" are different claims; collapsing them launders a NonMatching
// provenance into a matched one.**
//
// ============================================================================
// WHY A PARALLEL PORT AT ALL — THE RECEIVER ALREADY HAS A JMessage
// ============================================================================
// It does, and that is not a reason to skip this. The receiver's processor
// dispatches `0xF5-0xFF` and contains **ZERO `0x1A` handling** — measured:
// `grep -c 0x1a` across `libs/JSystem/{src,include}/JMessage/` returns 0.
// `0x1A` is WW's tag escape byte; TP escapes with `0x1B`. Today the gap is
// bridged by §308 (`d_ext_dmesg.cpp`), a TRANSCODER that rewrites WW tags into
// TP's convention and hand-maps THREE families — so **11.8% of the donor's
// 8,888 corpus tags (1,048 instances, 52 distinct group/code pairs) are
// neither handled nor listed**, and they emit nothing, silently.
//
// ============================================================================
// THE NAMESPACE IS `WwJMessage`, AND THE RENAME IS THE WHOLE COLLISION FIX
// ============================================================================
// Both implementations would otherwise be `JMessage::`. Measured: the receiver
// EXPORTS 90 `JMessage::` symbols, 27 of them `TProcessor`, so a partial port
// that kept the name would collide at link.
//
// It does not collide, because the rename is **closed under itself** — every
// definition in the donor set is either `JMessage::`-qualified or sits in an
// anonymous namespace (internal linkage). There is NO straggler at global
// scope, so relocating the namespace relocates 100% of the externally visible
// symbols. Verified by reading the donor set, not assumed.
//
// **THE SEAM LAW, and it holds in both directions (measured):** CALL receiver
// symbols freely; DEFINE none of theirs. The donor set's entire external
// surface is `JGadget` (39 exported), `JUTFont` (37) and `JUTResFont` (32) —
// all call-only, all already exported — plus `JUTDataBlockHeader`, which
// exports nothing because it is a two-`u32` POD: a layout, not a linkage
// entity.
//
// ============================================================================
// ⚠ SCOPE BOUNDARY — WHAT THIS LAYER DOES *NOT* DO, and it is the single
// easiest thing to get wrong about the port
// ============================================================================
// The donor's dispatch is **TWO-TIER** (row 709 trap 6). Read `do_tag_()` in
// the donor and the split is unambiguous — the BASE class handles only:
//     group 0xFF  system tags (with its own subclass-first inner tier)
//     group 0xFE  on_word
//     group 0xFD  on_message_limited
//     group 0xF6  select, limited
//     group 0xF5  select
//   + base system code 0x05 -> on_message
//
// **EVERY WW *GAME* TAG IS THE SUBCLASS'S JOB, NOT THIS LAYER'S.** Player name
// (group 0), message SE (group 1), camera (group 2), anime (group 3) are
// dispatched by the game's derived processor (`d_mesg.cpp`'s
// `dMesg_tSequenceProcessor`), which gets FIRST REFUSAL and hands the rest
// down. So porting JMessage alone yields the FRAMEWORK and the escape-byte
// handling — **it does not by itself render one WW line of dialogue.** The
// derived `dMesg` layer is a separate, dependent piece of work.
//
// Collapsing the two tiers is what swallows WW's own tags with neither path
// erroring — which is exactly the silent-drop this port exists to end, so
// reproducing it here would be self-defeating.
// ============================================================================

#include <cstdint>

namespace WwJMessage {

// ---------------------------------------------------------------------------
// TAG WIRE FORMAT — ported from the donor's `on_tag_()`, not inferred.
//
//     1A <size> <group> <codeHi> <codeLo> <payload...>
//
// `size` counts the WHOLE tag including the 0x1A and the size byte itself, so
// the payload is `size - 5` bytes at offset +5. The donor advances `current`
// by `size` BEFORE dispatching, which is why a malformed `size` must never be
// allowed to advance zero — that spins forever.
//
// The donor packs the tag as a 24-BIT value, `(group << 16) | code`, and then
// unpacks it again through `data::getTagGroup` / `getTagCode`. That round trip
// is preserved rather than short-circuited: the packed form is what a subclass
// `do_tag(u32 tag, ...)` receives, and flattening it here would change the
// signature the derived layer has to implement.
// ---------------------------------------------------------------------------
const uint8_t kEscape = 0x1A;   // WW's escape byte. TP's is 0x1B — see header.
const uint8_t kTagHeaderBytes = 5;

inline uint32_t packTag(uint8_t group, uint16_t code) {
    return ((uint32_t)group << 16) | (uint32_t)code;
}
inline uint8_t tagGroup(uint32_t tag) { return (uint8_t)((tag >> 16) & 0xFF); }
inline uint16_t tagCode(uint32_t tag) { return (uint16_t)(tag & 0xFFFF); }

// ⚠ THE THREE CODE PACKINGS ARE NOT INTERCHANGEABLE (row 709 trap 3). The tag
// group is masked to 8 BITS here; a message groupID is a full u16; and the
// engine's message address is `(groupID << 16) | index`. A value from one
// scheme fed to another resolves to a plausible wrong answer with no error.
// MID1 is measured ABSENT from both donor BMGs, so ID-addressing is not
// reachable in this corpus and only the first three schemes are live.

// ---------------------------------------------------------------------------
// A decoded tag, as the base layer sees it before dispatch. `payload` points
// INTO the message buffer and is never copied — same lifetime rule as the BMG
// reader: it must not outlive the archive it points into.
// ---------------------------------------------------------------------------
struct Tag {
    uint32_t       packed;      // (group << 16) | code, the donor's own form
    const uint8_t* payload;     // +5, or nullptr when size == 5
    uint8_t        payloadLen;  // size - 5
    uint8_t        size;        // the raw size byte, for advancing
};

// Decode one tag at `p`. Returns false — WITHOUT advancing — when `p` is not
// an escape or `size` is structurally impossible (< 5). A caller that gets
// false must advance by ONE byte, never by `size`, or a corrupt byte becomes
// an infinite loop. The donor's own guard is the same shape.
bool decodeTag(const uint8_t* p, uint32_t avail, Tag* out);

// Is this byte the lead of a multibyte character? DATA-DRIVEN, never a
// codepage assumption: the donor asks its `TResourceContainer` via
// `IsLeadByte()`, because the two donor BMGs BOTH declare `encoding == 1`
// while one is single-byte English and the other is 56.0% high
// bytes (NOT Japanese as I first read it — it is WW's HYLIAN-SCRIPT NEW
// GAME+ set; byte density proves non-ASCII, never WHICH non-ASCII). **The `encoding` field cannot select a decoder** —
// that is trap 4, and hardcoding a codepage here reintroduces it.
bool isLeadByte(uint8_t c);
void setLeadByteTable(bool (*fn)(uint8_t));

}  // namespace WwJMessage
