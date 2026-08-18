// ============================================================================
// ww_message.cpp — the BMG container reader. See ww_message.h for scope and
// the §801 reconstruction declaration.
//
// KIT-LINEAGE: native-port
// KIT-DONOR: JMessage/data.h + control.cpp (MATCHED) for layout; the user's
//            own disc bytes for every value that source could not settle
// KIT-DONOR-REF: zeldaret/tww@2d094c26
// KIT-DONOR-STATUS: layout MEASURED, behaviour NOT reconstructed here
//
// THE REF IS PINNED AT THE CURRENT COMMIT AND THAT IS LOAD-BEARING, not
// bookkeeping. This file's §801 declaration is a MEASUREMENT of decomp status,
// and a measurement without the commit it was taken at cannot be re-checked
// when upstream moves — which it did today (`1d57f046` -> `2d094c26`, mid-
// build). Verdicts were re-run at the new commit and held (3 MATCHED /
// 7 NONMATCHING; `JMessage/control.cpp` and `data.cpp`, the two this file
// actually builds on, are both still MATCHED). 109 other in-tree files still
// carry the SUPERSEDED `@1d57f046` ref — a ledger-wide sweep that is not this
// file's to make, but this file will not add a 110th.
// ============================================================================

#include "ww_message.h"

#include <mods/service.hpp>
#include <mods/svc/log.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

IMPORT_SERVICE(LogService, s_msgLog);

namespace {

void mlogf(LogLevel level, const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (s_msgLog != nullptr) {
        s_msgLog->write(mod_ctx, level, buf);
    }
}

// The donor is big-endian; the host is not. Every multi-byte field below goes
// through these — a raw struct overlay would read every value byte-swapped and
// the failure would look like corrupt data rather than a byte-order bug.
uint16_t be16(const unsigned char* p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}
uint32_t be32(const unsigned char* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

const unsigned char* s_base = nullptr;
uint32_t s_size = 0;
const unsigned char* s_inf1 = nullptr;   // INF1 block start
const unsigned char* s_dat1 = nullptr;   // DAT1 block start
uint32_t s_dat1Size = 0;
uint16_t s_count = 0;
uint16_t s_stride = 0;
uint16_t s_groupId = 0;
bool s_open = false;

// MEASURED, not a constant: the header size field at +0x08 counts 32-BYTE
// UNITS. `0x4E0B * 32 = 639328` matched the file exactly, and the same
// convention held on a second format (`color.bmc`, `34 * 32 = 1088`), so it is
// a family convention rather than a coincidence.
const uint32_t kSizeUnit = 32u;

}  // namespace

// ============================================================================
// RARC MEMBER LOCATOR. Offsets ported verbatim in behaviour from the estate's
// proven `ww_disc.py:126 rarc_list()`:
//     data_off = be32(+0x0C) + 0x20 · numEntries = be32(+0x28)
//     entryTbl = be32(+0x2C) + 0x20 · strTbl    = be32(+0x34) + 0x20
//     entry stride 0x14 · type e[4] · nameOff be16(e+6)
//     dataOff be32(e+8) · dataLen be32(e+12) · type 0x02 == directory
// ============================================================================
//
// EXTRACTED TO A SHARED HELPER 2026-08-16 so the colour table (`color.bmc`,
// the other member of the same archive) uses the SAME walk rather than a
// second copy of this arithmetic — the note above says re-deriving it would be
// a third implementation of something already right twice, and that applies
// inside this file too. EXTRACTION ONLY: every refusal, bound and log event is
// the code that was in `wwMessage_openFromArc` verbatim; the sole change is
// that the member bytes are RETURNED instead of going straight to the BMG
// parser, so a second consumer can exist. Nothing was relaxed.
// ============================================================================
namespace {

const unsigned char* rarcFindMember(const void* arc, uint32_t size,
                                    const char* member, uint32_t* outLen) {
    const unsigned char* a = static_cast<const unsigned char*>(arc);
    if (a == nullptr || member == nullptr || size < 0x40u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"null or shorter than a "
            "RARC header\",\"size\":%u}", size);
        return nullptr;
    }
    if (std::memcmp(a, "Yaz0", 4) == 0) {
        // Deliberate refusal — see the header. Reading Yaz0 as RARC would
        // report a corrupt entry table and send someone hunting the disc.
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"Yaz0-compressed; this "
            "reader handles uncompressed RARC only, and the donor message "
            "archives are measured uncompressed\"}");
        return nullptr;
    }
    if (std::memcmp(a, "RARC", 4) != 0) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"not RARC\",\"got\":\"%.4s\"}",
            (const char*)a);
        return nullptr;
    }

    const uint32_t dataOff  = be32(a + 0x0C) + 0x20u;
    const uint32_t numEnts  = be32(a + 0x28);
    const uint32_t entryTbl = be32(a + 0x2C) + 0x20u;
    const uint32_t strTbl   = be32(a + 0x34) + 0x20u;

    // Every derived offset is range-checked before use. A malformed archive
    // must produce a NAMED refusal, never a read past the buffer.
    if (entryTbl > size || strTbl > size || dataOff > size ||
        numEnts > 0xFFFFu ||
        entryTbl + (uint64_t)numEnts * 0x14u > size) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"header offsets out of "
            "range for the buffer\",\"entries\":%u,\"entryTbl\":%u,"
            "\"strTbl\":%u,\"dataOff\":%u,\"size\":%u}",
            numEnts, entryTbl, strTbl, dataOff, size);
        return nullptr;
    }

    for (uint32_t i = 0; i < numEnts; i++) {
        const unsigned char* e = a + entryTbl + (size_t)i * 0x14u;
        if (e[4] == 0x02) {
            continue;  // directory entry carries no file bytes
        }
        const uint32_t nameOff = be16(e + 6);
        const uint32_t dOff = be32(e + 8);
        const uint32_t dLen = be32(e + 12);
        if (strTbl + nameOff >= size) {
            continue;
        }
        const char* nm = reinterpret_cast<const char*>(a + strTbl + nameOff);
        // Bounded compare: the string table is not guaranteed terminated
        // before the buffer ends.
        const size_t room = size - (strTbl + nameOff);
        if (std::strncmp(nm, member, room) != 0) {
            continue;
        }
        if ((uint64_t)dataOff + dOff + dLen > size) {
            mlogf(LOG_LEVEL_ERROR,
                "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"member extends past "
                "the buffer\",\"member\":\"%s\",\"off\":%u,\"len\":%u}",
                member, dOff, dLen);
            return nullptr;
        }
        mlogf(LOG_LEVEL_INFO,
            "[WwMsg] {\"ev\":\"arc_member\",\"name\":\"%s\",\"len\":%u,"
            "\"of_entries\":%u}", member, dLen, numEnts);
        if (outLen != nullptr) { *outLen = dLen; }
        return a + dataOff + dOff;
    }

    mlogf(LOG_LEVEL_ERROR,
        "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"member not found\","
        "\"member\":\"%s\",\"entries\":%u}", member, numEnts);
    return nullptr;
}

}  // namespace

// External face of the internal locator. Declared in ww_message.h for the
// warp-menu parentage pass; see that header for why it lives in this file.
// DEFINED AT GLOBAL SCOPE DELIBERATELY - an externally-declared function
// defined inside the anonymous namespace compiles clean and fails at LINK,
// which is exactly the trap just fixed in registry.cpp.
const void* wwMessage_rarcFind(const void* arc, uint32_t size, const char* member,
                               uint32_t* outLen) {
    return rarcFindMember(arc, size, member, outLen);
}

bool wwMessage_openFromArc(const void* arc, uint32_t size, const char* member) {
    uint32_t len = 0;
    const unsigned char* p = rarcFindMember(arc, size, member, &len);
    return (p != nullptr) && wwMessage_open(p, len);
}

bool wwMessageColor_openFromArc(const void* arc, uint32_t size, const char* member) {
    uint32_t len = 0;
    const unsigned char* p = rarcFindMember(arc, size, member, &len);
    return (p != nullptr) && wwMessageColor_open(p, len);
}

bool wwMessage_isOpen() { return s_open; }
uint32_t wwMessage_count() { return s_open ? s_count : 0u; }

void wwMessage_close() {
    // Drop every pointer, not just the open flag. Leaving stale pointers
    // behind a false flag is how a "closed" reader still hands out text into
    // freed memory if some later path forgets to check.
    s_open = false;
    s_base = nullptr;
    s_inf1 = s_dat1 = nullptr;
    s_size = s_dat1Size = 0;
    s_count = s_stride = s_groupId = 0;
}

bool wwMessage_open(const void* bytes, uint32_t size) {
    s_open = false;
    s_base = nullptr;
    s_inf1 = s_dat1 = nullptr;
    s_count = s_stride = s_groupId = 0;
    s_dat1Size = 0;

    const unsigned char* p = static_cast<const unsigned char*>(bytes);
    if (p == nullptr || size < 0x20u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"null or shorter than the "
            "0x20 header\",\"size\":%u}", size);
        return false;
    }
    if (std::memcmp(p, "MESG", 4) != 0) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"signature is not MESG\","
            "\"got\":\"%.4s\"}", (const char*)p);
        return false;
    }

    // TRAP 2. Declared size is in 32-byte units. Read as bytes this comparison
    // rejects nearly every valid archive, which would read as "the disc is
    // wrong" rather than "the unit is wrong".
    const uint32_t declaredUnits = be32(p + 0x08);
    const uint32_t declaredBytes = declaredUnits * kSizeUnit;
    const uint32_t blockCount = be32(p + 0x0C);

    if (declaredBytes > size) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"declared size exceeds the "
            "buffer\",\"units\":%u,\"bytes\":%u,\"buffer\":%u}",
            declaredUnits, declaredBytes, size);
        return false;
    }

    // Walk the blocks. The walk CROSS-CHECKS ITSELF against the declared block
    // count — a walker that cannot disagree with its own header is not a check.
    const unsigned char* q = p + 0x20;
    const unsigned char* end = p + declaredBytes;
    uint32_t walked = 0;
    while (q + 8 <= end) {
        const uint32_t blockSize = be32(q + 4);
        if (blockSize < 8u || q + blockSize > end) {
            mlogf(LOG_LEVEL_ERROR,
                "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"block size out of "
                "range; walk stopped rather than ran off\",\"at\":%u,"
                "\"blockSize\":%u}", walked, blockSize);
            return false;
        }
        if (std::memcmp(q, "INF1", 4) == 0) {
            s_inf1 = q;
        } else if (std::memcmp(q, "DAT1", 4) == 0) {
            s_dat1 = q;
            s_dat1Size = blockSize;
        }
        walked++;
        q += blockSize;
    }
    if (walked != blockCount) {
        mlogf(LOG_LEVEL_WARN,
            "[WwMsg] {\"ev\":\"bmg_block_mismatch\",\"declared\":%u,\"walked\":%u,"
            "\"note\":\"header and walk disagree; reported rather than "
            "silently trusted\"}", blockCount, walked);
    }
    if (s_inf1 == nullptr || s_dat1 == nullptr) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"INF1 or DAT1 absent\","
            "\"inf1\":%d,\"dat1\":%d}", s_inf1 != nullptr, s_dat1 != nullptr);
        return false;
    }

    s_count = be16(s_inf1 + 0x08);
    s_stride = be16(s_inf1 + 0x0A);   // TRAP 3: stride is DATA, never a constant
    s_groupId = be16(s_inf1 + 0x0C);  // TRAP 5: this selects an ARCHIVE

    if (s_stride < 4u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmg_refused\",\"why\":\"entry stride too small to "
            "hold the DAT1 offset\",\"stride\":%u}", s_stride);
        return false;
    }

    s_base = p;
    s_size = size;
    s_open = true;
    mlogf(LOG_LEVEL_INFO,
        "[WwMsg] {\"ev\":\"bmg_open\",\"entries\":%u,\"stride\":\"0x%02X\","
        "\"groupID\":%u,\"dat1\":%u,\"blocks\":%u,"
        "\"note\":\"INF1 table read at 0x10 (the NONMATCHING struct says 0x14 "
        "and is wrong); size field decoded as 32-byte units\"}",
        s_count, s_stride, s_groupId, s_dat1Size, walked);
    return true;
}

bool wwMessage_get(uint32_t index, WwMessageEntry* out) {
    if (!s_open || out == nullptr) {
        return false;
    }
    if (index >= s_count) {
        // NOT clamped. An out-of-range index is a caller bug, and clamping
        // would hand back a real-looking message for a wrong request.
        return false;
    }
    // TRAP 1 + TRAP 3: table at 0x10, stride from the file.
    const unsigned char* e = s_inf1 + 0x10 + (size_t)index * s_stride;
    if (e + s_stride > s_inf1 + be32(s_inf1 + 4)) {
        return false;
    }

    std::memset(out, 0, sizeof(*out));
    out->dataOffset = be32(e + 0x00);
    if (out->dataOffset >= s_dat1Size) {
        mlogf(LOG_LEVEL_WARN,
            "[WwMsg] {\"ev\":\"entry_offset_out_of_range\",\"index\":%u,"
            "\"offset\":%u,\"dat1\":%u}", index, out->dataOffset, s_dat1Size);
        return false;
    }
    // DAT1 payload starts after its own 8-byte block header.
    out->text = reinterpret_cast<const char*>(s_dat1 + 8 + out->dataOffset);

    // ========================================================================
    // TRAP 7 — A WW MESSAGE IS NOT A C STRING, AND `text` ALONE CANNOT CARRY
    // ONE. MEASURED, not deduced: a corpus scan of the donor's own
    // `bmgres.arc/zel_00.bmg` (4,411 messages, 8,888 tags) shows tag PAYLOADS
    // CONTAIN 0x00 BYTES. The colour tag is literally `1A 06 FF 00 <idx>` —
    // its code high byte IS the NUL — and colour is 58.9% of every tag in the
    // corpus. So `strlen`/`%s`/`std::string(text)` stop at the FIRST colour
    // tag and silently discard the rest of the message.
    //
    // This is the same failure shape as the other six traps and it is why the
    // length is computed here rather than left to the caller: it fails
    // SILENTLY, producing a shorter but perfectly plausible message.
    //
    // Terminator rule (donor `processor.cpp:141` tag form
    // `1A <size> <group> <codeHi> <codeLo> <payload>`): skip `size` bytes at
    // every 0x1A; only a 0x00 found OUTSIDE a tag ends the message. The
    // `size < 5` guard mirrors §308's malformed guard — never advance 0, which
    // would spin here forever on a corrupt byte.
    // ========================================================================
    {
        const unsigned char* base = s_dat1 + 8;
        const unsigned char* p    = base + out->dataOffset;
        const unsigned char* lim  = base + (s_dat1Size > 8 ? s_dat1Size - 8 : 0);
        while (p < lim) {
            const unsigned char b = *p;
            if (b == 0x1A) {
                const unsigned char sz = (p + 1 < lim) ? p[1] : 0;
                p += (sz >= 5) ? sz : 1;
                continue;
            }
            if (b == 0x00) {
                break;
            }
            ++p;
        }
        out->textLen = static_cast<uint32_t>(p - (base + out->dataOffset));
    }

    // Presentation fields are only readable when the stride actually covers
    // them. Donor `JMSMesgEntry_c` is 0x18; a narrower stride is a different
    // record shape and reading past it would invent values.
    if (s_stride >= 0x18u) {
        out->nextMsgNo       = be16(e + 0x04);
        out->itemPrice       = (int16_t)be16(e + 0x06);
        out->textboxType     = e[0x08];
        out->drawType        = e[0x09];
        out->textboxPosition = e[0x0A];
        out->itemImage       = e[0x0B];
        out->textAlignment   = e[0x0C];
        out->initialSound    = e[0x0D];
        out->initialCamera   = e[0x0E];
        out->initialAnimation= e[0x0F];
    }
    return true;
}

// ============================================================================
// THE DONOR'S OWN COLOUR TABLE (`color.bmc`, `MGCLbmc1`). See ww_message.h for
// why this is a deliverable: `d_ext_dmesg.cpp:313` hardcodes a `kWwColor[9]`
// whose entries disagree with this file in 7 of 9 slots, and colour tags are
// 58.9% of the corpus. This reads the shipped data instead of approximating it.
// ============================================================================
namespace {

const unsigned char* s_clut = nullptr;   // -> palette base, NOT the file start
uint32_t s_clutHeaderWord = 0;           // the 4 bytes at block-body +0x00
bool s_clutOpen = false;

const uint32_t kClutEntries = 256u;      // by construction, not by observation

}  // namespace

bool wwMessageColor_open(const void* bytes, uint32_t size) {
    wwMessageColor_close();
    const unsigned char* b = static_cast<const unsigned char*>(bytes);
    if (b == nullptr || size < 0x2Cu + kClutEntries * 4u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmc_refused\",\"why\":\"null or too short for a "
            "256-entry CLT1\",\"size\":%u,\"need\":%u}",
            size, 0x2Cu + kClutEntries * 4u);
        return false;
    }
    if (std::memcmp(b, "MGCLbmc1", 8) != 0) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmc_refused\",\"why\":\"not MGCLbmc1\","
            "\"got\":\"%.8s\"}", (const char*)b);
        return false;
    }

    // SAME 32-BYTE-UNIT SIZE FIELD AS THE BMG (kSizeUnit). This file is the
    // second confirmation of that convention, so it is checked here rather
    // than assumed: 34 * 32 = 1088 exactly on the donor's own file.
    const uint32_t declared = be32(b + 0x08) * kSizeUnit;
    if (declared != size) {
        mlogf(LOG_LEVEL_WARN,
            "[WwMsg] {\"ev\":\"bmc_size_mismatch\",\"declared_units\":%u,"
            "\"declared_bytes\":%u,\"actual\":%u,\"note\":\"reported, not "
            "fatal - the palette is bounds-checked independently\"}",
            be32(b + 0x08), declared, size);
    }

    // Walk to CLT1 rather than assuming it is first. One block in the donor's
    // file, but a positional assumption is exactly the kind of thing that
    // fails silently if a second block ever appears.
    const uint32_t blocks = be32(b + 0x0C);
    uint32_t off = 0x20u;
    const unsigned char* clt = nullptr;
    uint32_t cltSize = 0;
    for (uint32_t i = 0; i < blocks && off + 8u <= size; i++) {
        const uint32_t bsz = be32(b + off + 4u);
        if (bsz == 0u || off + bsz > size) {
            break;              // refuse rather than spin or read past the end
        }
        if (std::memcmp(b + off, "CLT1", 4) == 0) {
            clt = b + off;
            cltSize = bsz;
            break;
        }
        off += bsz;
    }
    if (clt == nullptr) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmc_refused\",\"why\":\"no CLT1 block\","
            "\"blocks\":%u}", blocks);
        return false;
    }

    // Block body is at +0x08. The first 4 bytes there are an UNIDENTIFIED
    // field (observed 0x01000000) — recorded and reported, never interpreted
    // and never used as a gate. The palette follows it, so base = block+0x0C.
    s_clutHeaderWord = be32(clt + 0x08);
    const uint32_t need = 0x0Cu + kClutEntries * 4u;
    if (cltSize < need) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"bmc_refused\",\"why\":\"CLT1 too small for 256 "
            "RGBA8 entries\",\"block\":%u,\"need\":%u}", cltSize, need);
        return false;
    }
    s_clut = clt + 0x0Cu;
    s_clutOpen = true;

    mlogf(LOG_LEVEL_INFO,
        "[WwMsg] {\"ev\":\"bmc_open\",\"entries\":%u,\"distinct\":%u,"
        "\"header_word\":\"%08X\",\"idx0\":\"%08X\",\"idx1\":\"%08X\","
        "\"note\":\"donor palette; replaces the hardcoded kWwColor "
        "approximation that differs in 7 of 9 slots\"}",
        kClutEntries, wwMessageColor_distinctCount(), s_clutHeaderWord,
        wwMessageColor_get(0), wwMessageColor_get(1));
    return true;
}

uint32_t wwMessageColor_get(uint8_t index) {
    if (!s_clutOpen || s_clut == nullptr) {
        return 0u;   // NOT black - callers must check isOpen(); see the header
    }
    return be32(s_clut + (uint32_t)index * 4u);
}

uint32_t wwMessageColor_distinctCount() {
    if (!s_clutOpen) {
        return 0u;
    }
    uint32_t seen[16];
    uint32_t n = 0;
    for (uint32_t i = 0; i < kClutEntries; i++) {
        const uint32_t v = be32(s_clut + i * 4u);
        bool dup = false;
        for (uint32_t j = 0; j < n; j++) {
            if (seen[j] == v) { dup = true; break; }
        }
        if (!dup) {
            if (n >= 16u) {
                return n;   // saturated: report what we counted, never lie low
            }
            seen[n++] = v;
        }
    }
    return n;
}

bool wwMessageColor_isOpen() { return s_clutOpen; }

void wwMessageColor_close() {
    // Same lifetime contract as the BMG reader: this holds a POINTER INTO the
    // caller's bytes and copies nothing, so it must be closed before that
    // buffer dies or every colour lookup reads freed memory.
    s_clut = nullptr;
    s_clutHeaderWord = 0;
    s_clutOpen = false;
}
