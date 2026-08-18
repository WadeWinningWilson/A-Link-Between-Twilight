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
#include <new>

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

// ============================================================================
// Yaz0 (SZS) decompression — PORTED FROM THE DONOR, NOT AUTHORED HERE.
//
// KIT-DONOR: JSystem/JKernel/JKRDecomp.cpp :: JKRDecomp::decodeSZS
// KIT-DONOR-STATUS: MATCHED — `Object(Matching, "JSystem/JKernel/JKRDecomp.cpp")`
//                   at configure.py:994, so this is byte-for-byte the routine
//                   the retail game runs on these archives.
//
// DN-10 STEP 1: the donor has its own decoder, so it is PORTED rather than
// reconstructed. No decompressor was authored for this receiver. Specialised
// to the whole-buffer case: the donor's `dstSize` is a SKIP counter and its
// `srcSize` an emit COUNT; here we skip nothing and emit the full length the
// header declares at +4.
//
// PROVEN BEFORE WIRING, against the donor's own archives rather than a
// synthetic case: all 12 Yaz0 files in `res/Msg` decode to magic `RARC` at
// byte 0 with a length exactly equal to the header's declared size. A
// transcription error in an LZ inner loop desynchronises the stream within a
// few hundred bytes — twelve exact-length RARCs is not a result a broken port
// can produce.
//
// THE BOUNDS CHECKS ARE RECEIVER-SIDE, NOT DONOR BEHAVIOUR. Retail trusts its
// own disc; we are handed a buffer whose length we know, so overruns refuse
// instead of reading past the end. That is a consumption-boundary guard and
// changes no donor byte.
// ============================================================================
uint32_t yaz0DecompressedSize(const unsigned char* a) {
    return (uint32_t(a[4]) << 24) | (uint32_t(a[5]) << 16) |
           (uint32_t(a[6]) << 8) | uint32_t(a[7]);
}

bool yaz0Decode(const unsigned char* src, uint32_t srcSize,
                unsigned char* dst, uint32_t dstSize) {
    uint32_t si = 0x10;
    uint32_t di = 0;
    unsigned int bits = 0;
    int nbits = 0;

    while (di < dstSize) {
        if (nbits == 0) {
            if (si >= srcSize) return false;
            bits = src[si++];
            nbits = 8;
        }
        if (bits & 0x80) {
            if (si >= srcSize) return false;
            dst[di++] = src[si++];
        } else {
            if (si + 1 >= srcSize) return false;
            const unsigned int b0 = src[si];
            const unsigned int b1 = src[si + 1];
            si += 2;
            const uint32_t dist = ((b0 & 0x0Fu) << 8) | b1;
            uint32_t count = b0 >> 4;
            if (count == 0) {
                if (si >= srcSize) return false;
                count = uint32_t(src[si++]) + 0x12u;
            } else {
                count += 2;
            }
            if (dist + 1u > di) return false;          // copy source underflows
            uint32_t cs = di - dist - 1u;
            if (di + count > dstSize) return false;     // would overrun the output
            while (count-- != 0) dst[di++] = dst[cs++];
        }
        bits = (bits << 1) & 0xFFu;
        nbits--;
    }
    return di == dstSize;
}

// ============================================================================
// Decompressed-archive cache — ENTRIES ARE NEVER EVICTED, AND THAT IS THE
// DESIGN, NOT AN OVERSIGHT.
//
// `rarcFindMember` returns a NON-OWNING pointer INTO the archive, and
// `wwMessage_open` then caches views into it (`s_base`, `s_inf1`, `s_dat1`).
// Freeing or evicting a decompressed buffer while any of those are live turns
// every later read into a read of freed memory — the failure this file already
// documents twice (see `wwMessage_close`, and the colour-table note below it).
//
// So a decompressed archive is allocated once and kept for the module's
// lifetime. The population is bounded and small: 12 Yaz0 archives in the
// donor's `res/Msg`, ~1.05 MB if every one were opened at once. A FULL TABLE
// REFUSES LOUDLY RATHER THAN EVICTING, because eviction reintroduces exactly
// the dangling pointer this design exists to prevent.
// ============================================================================
// ----------------------------------------------------------------------------
// THE KEY IS CONTENT, NEVER AN ADDRESS. (Housing/Engine caught this on review
// of the first cut, which keyed on the caller's archive POINTER.)
//
// The compressed archive is CALLER-OWNED and this module does not control its
// lifetime, so a raw address is not an identity — and never-evict made that
// permanent rather than transient. It failed in BOTH directions:
//
//   · SILENT WRONG DATA — the source arc is freed and a DIFFERENT archive of
//     the same compressed size lands at the same address. Allocators reuse
//     same-size blocks as ordinary behaviour, so that is not exotic. Result is
//     a cache HIT returning the previous archive's text, with no fault and no
//     refusal to notice it by.
//   · CAP EXHAUSTION BY CHURN, which fires first in practice — the SAME
//     archive re-loaded at a NEW address is a MISS and burns a fresh slot.
//     Across stage transitions the table fills with duplicates of a handful of
//     archives, then refuses PERMANENTLY. The symptom is "messages worked,
//     then stopped after playing a while", which no short run surfaces.
//
// Hashing the compressed bytes makes a re-load at a new address a HIT, which
// removes the churn failure entirely and converts the correctness question
// from a lifetime one (unanswerable here) into a collision one (bounded and
// astronomically unlikely with a 64-bit hash plus both sizes).
// ----------------------------------------------------------------------------
uint64_t yaz0KeyOf(const unsigned char* a, uint32_t size) {
    uint64_t h = 1469598103934665603ull;            // FNV-1a offset basis
    for (uint32_t i = 0; i < size; ++i) {
        h ^= a[i];
        h *= 1099511628211ull;                      // FNV-1a prime
    }
    return h;
}

struct Yaz0CacheEntry {
    uint64_t key;       // content hash — NOT an address (see above)
    uint32_t srcSize;
    uint32_t outSize;
    unsigned char* buf;
    uint32_t len;
};

const int kYaz0CacheMax = 24;   // 2x the donor's res/Msg population
Yaz0CacheEntry s_yaz0Cache[kYaz0CacheMax];
int s_yaz0CacheCount = 0;

// ----------------------------------------------------------------------------
// THE TRANSIENT PATH — why not everything goes in the never-evict table.
// (Housing/Engine raised the exhaustion; the consumer lifetimes below are the
// measurement that decides it.)
//
// `rarcFindMember` is ONE GATE serving FOUR callers, and they do not have the
// same lifetime needs:
//
//   · MESSAGE opens (`wwMessage_open`, `wwMessageColor_open`) stash views —
//     `s_base`, `s_inf1`, `s_dat1` — that outlive the call. Those MUST be
//     cached and never evicted. Population: 12 archives, ~1.05 MB. Bounded.
//   · `wwMessage_rarcFind` callers CONSUME WITHIN THE CALL. Measured, not
//     assumed: `main.cpp:271` reads `stage.dzs`, extracts one int and calls
//     `wwRegistry_setStageType(stage, int)`; `main.cpp:309` reads `room.dzr`
//     and `memcpy`s names into a LOCAL `char nm[9]`. Neither lets a pointer
//     into the archive escape.
//
// Sending stage/room archives through the persistent table was a real defect:
// `res/Stage` holds 328 Yaz0 archives against 24 never-evicted slots, so a
// player crossing ~24 compressed stages fills it with LEGITIMATELY DISTINCT
// entries and then every later stage load fails with "cache full". Content
// keying cannot help — that failure is diversity, not duplication. And 24
// slots of multi-MB `Stage.arc` is tens of MB held for the session, not the
// ~1 MB the cap was justified against.
//
// So the transient path decompresses into ONE reusable scratch buffer.
//
// ⚠ CONTRACT: a pointer from the transient path is valid ONLY UNTIL THE NEXT
// `wwMessage_rarcFind` CALL. Both present callers satisfy that trivially. A
// future caller that wants to HOLD the bytes must copy them or use the
// persistent path — and that is exactly why this is spelled out here rather
// than left as a property someone has to rediscover.
// ----------------------------------------------------------------------------
unsigned char* s_yaz0Scratch = nullptr;
uint32_t s_yaz0ScratchCap = 0;
// What is CURRENTLY sitting in the scratch buffer, so a repeat request for the
// same archive does not decompress it again. MEASURED NEED, not speculation:
// the first boot with this path live logged 20 decodes for 10 archives — every
// one decompressed TWICE, including a 149,191 -> 773,920 byte archive. The
// stage/room sniffers ask the same arc for the same member more than once per
// load, and the transient path had no reuse whatsoever.
//
// Safe by the contract already stated above: a transient pointer is valid only
// until the NEXT wwMessage_rarcFind call, and returning the same buffer for the
// SAME archive cannot violate that — the bytes are identical either way.
uint64_t s_yaz0ScratchKey = 0;
uint32_t s_yaz0ScratchLen = 0;
bool s_yaz0ScratchValid = false;

const unsigned char* yaz0AcquireTransient(const unsigned char* a, uint32_t size,
                                          uint32_t declared, uint32_t* outLen) {
    const uint64_t key = yaz0KeyOf(a, size);
    if (s_yaz0ScratchValid && s_yaz0ScratchKey == key &&
        s_yaz0ScratchLen == declared && s_yaz0Scratch != nullptr) {
        mlogf(LOG_LEVEL_INFO,
            "[WwMsg] {\"ev\":\"yaz0_scratch_reuse\",\"src\":%u,\"out\":%u}",
            size, declared);
        *outLen = s_yaz0ScratchLen;
        return s_yaz0Scratch;
    }
    s_yaz0ScratchValid = false;   // contents are about to be replaced
    if (declared > s_yaz0ScratchCap) {
        delete[] s_yaz0Scratch;                 // safe: call-scoped by contract
        s_yaz0Scratch = new (std::nothrow) unsigned char[declared];
        s_yaz0ScratchCap = (s_yaz0Scratch != nullptr) ? declared : 0u;
    }
    if (s_yaz0Scratch == nullptr) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"scratch allocation "
            "failed\",\"want\":%u}", declared);
        return nullptr;
    }
    if (!yaz0Decode(a, size, s_yaz0Scratch, declared)) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"stream ended early or "
            "would overrun (transient)\",\"declared\":%u}", declared);
        return nullptr;
    }
    s_yaz0ScratchKey = key;
    s_yaz0ScratchLen = declared;
    s_yaz0ScratchValid = true;
    mlogf(LOG_LEVEL_INFO,
        "[WwMsg] {\"ev\":\"yaz0_decoded\",\"mode\":\"transient\",\"src\":%u,"
        "\"out\":%u}", size, declared);
    *outLen = declared;
    return s_yaz0Scratch;
}

const unsigned char* yaz0Acquire(const void* arc, uint32_t size, uint32_t* outLen) {
    const unsigned char* a = static_cast<const unsigned char*>(arc);

    if (size < 0x11u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"shorter than a Yaz0 "
            "header\",\"size\":%u}", size);
        return nullptr;
    }

    const uint32_t declared = yaz0DecompressedSize(a);
    const uint64_t key = yaz0KeyOf(a, size);
    for (int i = 0; i < s_yaz0CacheCount; ++i) {
        if (s_yaz0Cache[i].key == key && s_yaz0Cache[i].srcSize == size &&
            s_yaz0Cache[i].outSize == declared) {
            // THE HIT PATH LOGS. The first cut was silent here, so a wrong hit
            // would have produced wrong text with nothing in the log to see it
            // by. A cache that only reports its misses cannot be audited.
            mlogf(LOG_LEVEL_INFO,
                "[WwMsg] {\"ev\":\"yaz0_cache_hit\",\"src\":%u,\"out\":%u,"
                "\"slot\":%d}", size, s_yaz0Cache[i].len, i);
            *outLen = s_yaz0Cache[i].len;
            return s_yaz0Cache[i].buf;
        }
    }

    if (s_yaz0CacheCount >= kYaz0CacheMax) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"cache full; entries are "
            "never evicted because live readers hold views into them\","
            "\"cap\":%d}", kYaz0CacheMax);
        return nullptr;
    }

    const uint32_t outSize = declared;
    if (outSize == 0u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"header declares zero "
            "decompressed size\"}");
        return nullptr;
    }
    unsigned char* buf = new (std::nothrow) unsigned char[outSize];
    if (buf == nullptr) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"allocation failed\","
            "\"want\":%u}", outSize);
        return nullptr;
    }
    if (!yaz0Decode(a, size, buf, outSize)) {
        delete[] buf;   // safe: nothing has been handed out yet
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"stream ended early or "
            "would overrun; archive truncated or not Yaz0\",\"declared\":%u}",
            outSize);
        return nullptr;
    }

    s_yaz0Cache[s_yaz0CacheCount].key = key;
    s_yaz0Cache[s_yaz0CacheCount].srcSize = size;
    s_yaz0Cache[s_yaz0CacheCount].outSize = declared;
    s_yaz0Cache[s_yaz0CacheCount].buf = buf;
    s_yaz0Cache[s_yaz0CacheCount].len = outSize;
    s_yaz0CacheCount++;

    mlogf(LOG_LEVEL_INFO,
        "[WwMsg] {\"ev\":\"yaz0_decoded\",\"src\":%u,\"out\":%u,\"cached\":%d}",
        size, outSize, s_yaz0CacheCount);

    *outLen = outSize;
    return buf;
}

// `persistent` = the caller will hold views into the returned bytes after it
// returns, so a Yaz0 archive must go in the never-evict table. FALSE routes it
// through the reusable scratch buffer (see the contract above).
const unsigned char* rarcFindMember(const void* arc, uint32_t size,
                                    const char* member, uint32_t* outLen,
                                    bool persistent) {
    const unsigned char* a = static_cast<const unsigned char*>(arc);
    if (a == nullptr || member == nullptr || size < 0x40u) {
        mlogf(LOG_LEVEL_ERROR,
            "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"null or shorter than a "
            "RARC header\",\"size\":%u}", size);
        return nullptr;
    }
    // ========================================================================
    // Yaz0 arrives DECOMPRESSED now, and the premise that used to sit here was
    // measurably false. This block read:
    //
    //     "Yaz0-compressed; this reader handles uncompressed RARC only, and
    //      the donor message archives are measured uncompressed"
    //
    // THE DONOR MESSAGE ARCHIVES ARE NOT ALL UNCOMPRESSED. Counted on the
    // user's own disc: `res/Msg` holds 35 archives and 12 ARE Yaz0 — every one
    // a map-name resource (docmapres, gsmapres, heartmapres, htmmapres,
    // irmapres, moonmapres, submamapres, terrymapres, tnmapres, trmapres,
    // ygmapres, ysmapres). Disc-wide the census is 640 Yaz0 against 681 RARC,
    // so this refusal was turning away 48% of every archive on ANY path that
    // reached this reader, not just messages.
    //
    // The refusal was still the right call while no decoder existed — reading
    // Yaz0 as RARC reports a corrupt entry table and sends someone hunting the
    // disc. It is superseded, not reversed: we now decode with the donor's own
    // routine and carry on.
    // ========================================================================
    if (std::memcmp(a, "Yaz0", 4) == 0) {
        uint32_t inflated = 0;
        const unsigned char* buf = nullptr;
        if (persistent) {
            buf = yaz0Acquire(arc, size, &inflated);
        } else {
            if (size < 0x11u) {
                mlogf(LOG_LEVEL_ERROR,
                    "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"shorter than a "
                    "Yaz0 header (transient)\",\"size\":%u}", size);
                return nullptr;
            }
            const uint32_t declared = yaz0DecompressedSize(a);
            if (declared == 0u) {
                mlogf(LOG_LEVEL_ERROR,
                    "[WwMsg] {\"ev\":\"yaz0_refused\",\"why\":\"header declares "
                    "zero decompressed size (transient)\"}");
                return nullptr;
            }
            buf = yaz0AcquireTransient(a, size, declared, &inflated);
        }
        if (buf == nullptr) {
            return nullptr;   // the acquire path has already logged the reason
        }
        a = buf;
        size = inflated;
        if (size < 0x40u) {
            mlogf(LOG_LEVEL_ERROR,
                "[WwMsg] {\"ev\":\"arc_refused\",\"why\":\"decompressed shorter "
                "than a RARC header\",\"size\":%u}", size);
            return nullptr;
        }
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
    // TRANSIENT: both callers (main.cpp:271 stage.dzs, :309 room.dzr) consume
    // within the call - measured, not assumed. The returned pointer is valid
    // only until the next wwMessage_rarcFind call; copy if you need to hold it.
    return rarcFindMember(arc, size, member, outLen, /*persistent=*/false);
}

bool wwMessage_openFromArc(const void* arc, uint32_t size, const char* member) {
    uint32_t len = 0;
    // PERSISTENT: wwMessage_open stashes s_base/s_inf1/s_dat1 into these bytes.
    const unsigned char* p = rarcFindMember(arc, size, member, &len, /*persistent=*/true);
    return (p != nullptr) && wwMessage_open(p, len);
}

bool wwMessageColor_openFromArc(const void* arc, uint32_t size, const char* member) {
    uint32_t len = 0;
    // PERSISTENT: the colour table is read back after this call returns.
    const unsigned char* p = rarcFindMember(arc, size, member, &len, /*persistent=*/true);
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
