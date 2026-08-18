// ============================================================================
// ww_jmessage.cpp — see ww_jmessage.h for provenance, the §801 DIVERGENT
// label, the namespace-collision argument and the two-tier scope boundary.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JMessage/processor.cpp (NONMATCHING, §801-admissible
//            behavioural spec); data.cpp accessors (MATCHED)
// KIT-DONOR-REF: zeldaret/tww@2d094c26
// KIT-DONOR-STATUS: BEHAVIOURALLY SPECIFIED, BYTE-DIVERGENT
// ============================================================================

#include "ww_jmessage.h"

namespace WwJMessage {
namespace {

// Default: EVERYTHING is single-byte. Deliberately the conservative choice —
// a wrong "yes" consumes a following byte that was real text and corrupts the
// rest of the line, while a wrong "no" merely renders one glyph wrong. The
// donor asks its TResourceContainer; until the caller supplies that predicate
// via setLeadByteTable(), this refuses to guess.
bool defaultLeadByte(uint8_t) { return false; }

bool (*s_leadByte)(uint8_t) = defaultLeadByte;

}  // namespace

bool decodeTag(const uint8_t* p, uint32_t avail, Tag* out) {
    if (p == nullptr || out == nullptr || avail < kTagHeaderBytes) {
        return false;
    }
    if (p[0] != kEscape) {
        return false;
    }
    const uint8_t size = p[1];
    // STRUCTURAL FLOOR, and it is the guard that keeps a corrupt byte from
    // becoming a hang: the donor advances `current` by `size` before it
    // dispatches, so a `size` under the 5-byte header would advance backwards
    // or nowhere. §308's own walk carries the same `< 5` test for the same
    // reason. A caller receiving false MUST advance by exactly one byte.
    if (size < kTagHeaderBytes || size > avail) {
        return false;
    }
    out->packed = packTag(p[2], (uint16_t)((p[3] << 8) | p[4]));
    out->payloadLen = (uint8_t)(size - kTagHeaderBytes);
    out->payload = (out->payloadLen > 0) ? (p + kTagHeaderBytes) : nullptr;
    out->size = size;
    return true;
}

bool isLeadByte(uint8_t c) { return s_leadByte(c); }

void setLeadByteTable(bool (*fn)(uint8_t)) {
    s_leadByte = (fn != nullptr) ? fn : defaultLeadByte;
}

}  // namespace WwJMessage
