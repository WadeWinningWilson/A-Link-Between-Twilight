// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASCalc.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 calc utilities — donor-verbatim port of WW retail
// JSystem/JAudio/JASCalc.cpp (D:/XXXXXXX/WW DP; dtk address markers
// kept). Phase A4.5 of the native JA1 campaign (bus §362/§363/§366).
// Carries the REAL table-driven sinfT/sinfDolby2 (quarter-wave sine +
// dolby column tables, 257 entries each, built at init exactly like the
// donor — the donor has no baked table, initSinfT computes it) plus the
// donor's aligned bulk copy/zero family (bcopy/bcopyfast/bzero/
// bzerofast/imixcopy) the bank parser and track init run through.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept.
// Donor include map: JASCalc.h -> d/ext_seq/ja1_boundary.h (Calc decls
// live in the §367 boundary header); JUTAssert.h -> the ja1_boundary.h
// JUT shims; math.h -> <cmath> + receiver global.h M_PI;
// JASSystemHeap.h/JKRSolidHeap.h (JASDram) -> dropped, see below.
// §370 named adaptations (established classes only):
//   - `new (JASDram, 0)` -> plain new (A2/A3 heap precedent).
//   - reinterpret_cast<u32>(ptr) alignment checks ->
//     reinterpret_cast<uintptr_t> (§368 x64-widening precedent; only
//     the low bits are tested, value-identical).
//   - DCZeroRange is the receiver's own PC realization
//     (src/dusk/extras.c:72 = memset 0) — the donor cache-zero contract
//     "zero the block" holds, call kept verbatim.
//   - M_PI comes from receiver global.h (f32 literal) vs the donor GC
//     math.h double — table generation differs below f32 precision,
//     flagged not fixed (no invented constants).
// This TU retires the A2-support Calc stub fence in ja1_boundary.h
// (the old inline bzero/bcopy contract loops and the donor-neutral
// sinfT/sinfDolby2 0.0f returns).
// ============================================================

#include "d/ext_seq/ja1_boundary.h"

#include <cmath>    // ===== §370: donor math.h (sin)
#include <cstdint>  // ===== §370: uintptr_t for the x64-widened alignment checks
#include "global.h"       // ===== §370: receiver M_PI (see header note)
#include "dolphin/os.h"   // ===== §370: DCZeroRange (receiver PC realization, extras.c)

namespace JAudio1 {

f32* Calc::JASC_SINTABLE;
f32* Calc::JASC_DOL2TABLE;

/* 8027A804-8027A9C8       .text initSinfT__Q28JASystem4CalcFv */
void Calc::initSinfT() {
    JASC_SINTABLE = new f32[257];  // ===== §370: donor `new (JASDram, 0)` (A2 heap precedent)
    JUT_ASSERT(49, JASC_SINTABLE != NULL);
    JASC_DOL2TABLE = new f32[257];  // ===== §370: donor `new (JASDram, 0)` (A2 heap precedent)
    JUT_ASSERT(51, JASC_DOL2TABLE != NULL);
    for (u32 i = 0; i < 257; i++) {
        JASC_SINTABLE[i] = sin((M_PI / 2) * i / 256.0f);
    }
    for (u32 i = 0; i < 257; i++) {
        JASC_DOL2TABLE[i] = sin((M_PI / 2) * (0.32612f + 0.34776f * i / 256.0f));
    }
}

/* 8027A9C8-8027A9F4       .text sinfT__Q28JASystem4CalcFf */
f32 Calc::sinfT(f32 param_1) {
    return JASC_SINTABLE[s32(param_1 * 256.0f)];
}

/* 8027A9F4-8027AA20       .text sinfDolby2__Q28JASystem4CalcFf */
f32 Calc::sinfDolby2(f32 param_1) {
    return JASC_DOL2TABLE[s32(param_1 * 256.0f)];
}

/* 8027AA20-8027AA50       .text imixcopy__Q28JASystem4CalcFPCsPCsPsl */
void Calc::imixcopy(const s16* s1, const s16* s2, s16* dst, s32 n) {
    for (; n > 0; n--) {
        *dst++ = *s1++;
        *dst++ = *s2++;
    }
}

/* 8027AA50-8027AB68       .text bcopyfast__Q28JASystem4CalcFPCUlPUlUl */
void Calc::bcopyfast(const u32* src, u32* dest, u32 size) {
    JUT_ASSERT(280, (reinterpret_cast<uintptr_t>(src) & 0x03) == 0);   // ===== §370: donor u32 — x64 widening
    JUT_ASSERT(281, (reinterpret_cast<uintptr_t>(dest) & 0x03) == 0);  // ===== §370: donor u32 — x64 widening
    JUT_ASSERT(282, (size & 0x0f) == 0);
    for (size /= 16; size; size--) {
        u32 val1 = *src++;
        u32 val2 = *src++;
        u32 val3 = *src++;
        u32 val4 = *src++;
        *dest++ = val1;
        *dest++ = val2;
        *dest++ = val3;
        *dest++ = val4;
    }
}

/* 8027AB68-8027AC68       .text bcopy__Q28JASystem4CalcFPCvPvUl */
void Calc::bcopy(const void* src, void* dest, u32 size) {
    u32 *usrc;
    u32 *udest;

    u8 *bsrc = (u8 *)src;
    u8 *bdest = (u8 *)dest;

    u8 endbitsSrc = (reinterpret_cast<uintptr_t>(bsrc) & 0x03);   // ===== §370: donor u32 — x64 widening
    u8 enbitsDst = (reinterpret_cast<uintptr_t>(bdest) & 0x03);   // ===== §370: donor u32 — x64 widening
    if ((endbitsSrc) == (enbitsDst) && (size & 0x0f) == 0) {
        bcopyfast((u32*)src, (u32*)dest, size);
    } else if ((endbitsSrc == enbitsDst) && (size >= 16)) {
        if (endbitsSrc != 0) {
            for (endbitsSrc = 4 - endbitsSrc; endbitsSrc != 0; endbitsSrc--) {
                *bdest++ = (u32)*bsrc++;
                size--;
            }
        }

        udest = (u32 *)bdest;
        usrc = (u32 *)bsrc;

        for (; size >= 4; size -= 4) {
            *udest++ = *usrc++;
        }

        if (size != 0) {
            bdest = (u8 *)udest;
            bsrc = (u8 *)usrc;

            for (; size != 0; size--) {
                *bdest++ = (u32)*bsrc++;
            }
        }
    } else {
        for (; size != 0; size--) {
            *bdest++ = (u32)*bsrc++;
        }
    }
}

/* 8027AC68-8027AD38       .text bzerofast__Q28JASystem4CalcFPvUl */
void Calc::bzerofast(void* dest, u32 size) {
    JUT_ASSERT(387, (reinterpret_cast<uintptr_t>(dest) & 0x03) == 0);  // ===== §370: donor u32 — x64 widening
    JUT_ASSERT(388, (size & 0x0f) == 0);
    u32* udest = (u32*)dest;
    for (size = size / 16; size != 0; size--) {
        *udest++ = 0;
        *udest++ = 0;
        *udest++ = 0;
        *udest++ = 0;
    }
}

/* 8027AD38-8027AE30       .text bzero__Q28JASystem4CalcFPvUl */
void Calc::bzero(void* dest, u32 size) {
    u32 *udest;
    u8 *bdest = (u8 *)dest;
    if ((size & 0x1f) == 0 && (reinterpret_cast<uintptr_t>(dest) & 0x1f) == 0) {  // ===== §370: donor u32 — x64 widening
        DCZeroRange(dest, size);  // ===== §370: receiver PC realization (memset 0) — donor contract kept
        return;
    }

    u8 alignedbitsDst = reinterpret_cast<uintptr_t>(bdest) & 0x3;  // ===== §370: donor u32 — x64 widening

    if ((size & 0xf) == 0 && alignedbitsDst == 0) {
        bzerofast(dest, size);
        return;
    }

    if (size >= 16) {
        if (alignedbitsDst != 0) {
            for (alignedbitsDst = 4 - alignedbitsDst; alignedbitsDst != 0; alignedbitsDst--) {
                *bdest++ = 0;
                size--;
            }
        }

        udest = (u32 *)bdest;
        for (; size >= 4; size -= 4) {
            *udest++ = 0;
        }

        if (size != 0) {
            bdest = (u8 *)udest;
            for (; size != 0; size--) {
                *bdest++ = 0;
            }
        }
    } else {
        for (; size != 0; size--) {
            *bdest++ = 0;
        }
    }
}

}  // namespace JAudio1
