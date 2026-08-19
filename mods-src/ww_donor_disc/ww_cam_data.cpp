// ============================================================================
// ww_cam_data.cpp — the donor camera style/type tables, generated + resident.
//
// KIT-LINEAGE: donor-port
// KIT-DONOR: d/d_cam_style.cpp MatchingFor
// KIT-DONOR: d/d_cam_type.cpp MatchingFor
// KIT-DONOR: d/d_cam_type2.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
//
// See ww_cam_data.h for the port rationale (compiled-in donor data, donor
// axis, FULL-selector ruling). The tables live in ww_cam_data.inc, GENERATED
// by tools/ww_crew_restoration_skeleton/gen_cam_data.py — regenerate, never
// hand-edit.
//
// GENERATION-TIME verification (hard failures in the tool): style-enum order
// == row order name for name; every algorithm mapped (sockets for the eight
// the receiver lacks); dropped params 28/29 proven to be LOCKON_FOVY_MIN/MAX.
// The install self-test below re-checks the RESIDENT bytes' framing — count,
// record size, spot 4CCs — so a stale or truncated .inc is caught in the log,
// not in a camera glitch three sessions later.
//
// Cross-validation receipt (2026-08-10): the generator independently
// reproduced the §670-era decode — Subject non-NONE slots {0:LN17 4:SS01
// 10:SX01 11:SY01 13:CC02 14:SN15 16:EN00}, CC02 flags 0x0C2, LN17 flags
// 0x042 — and surfaced CC01 (flags 0x082) as a third CRAWL-algorithm style.
// ============================================================================

#include "ww_cam_data.h"

#include <cstdio>
#include <cstring>

#include <mods/api.h>
#include <mods/svc/log.h>

extern const LogService* s_log;

#include "ww_cam_data.inc"

namespace {

// The receiver's camstyle.dat record: {s32be id; u16be alg; u16be flags;
// f32be params[28]} = 0x78. Framing constants, restated here so the self-test
// is independent of the generator's arithmetic.
constexpr int kStyleRecordSize = 4 + 2 + 2 + 28 * 4;
constexpr int kStyleHeaderSize = 8;

unsigned be32At(const unsigned char* p) {
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) | ((unsigned)p[2] << 8) | (unsigned)p[3];
}

}  // namespace

const void* dExtWwCam_styleDatBlob(int* o_byteSize) {
    if (o_byteSize != NULL) {
        *o_byteSize = (int)sizeof(kWwCamStyleBlobBE);
    }
    return kWwCamStyleBlobBE;
}

const dExtWwCamType* dExtWwCam_types(int* o_count) {
    if (o_count != NULL) {
        *o_count = kWwCamTypeCount;
    }
    return kWwCamTypes;
}

const void* dExtWwCam_typeShadowBlob(int* o_byteSize) {
    if (o_byteSize != NULL) {
        *o_byteSize = (int)sizeof(kWwCamTypeShadowBE);
    }
    return kWwCamTypeShadowBE;
}

int dExtWwCam_styleCount(void) {
    return kWwCamStyleCount;
}

const char* const* dExtWwCam_bgTypeNames(int* o_count) {
    if (o_count != NULL) {
        *o_count = kWwCamBgTypeCount;
    }
    return kWwCamBgTypeNames;
}

void dExtWwCam_installData(void) {
    // ------------------------------------------------------------------------
    // Resident-data self-test. Everything here was proven at generation time;
    // this catches the .inc going stale against this TU's expectations.
    // ------------------------------------------------------------------------
    bool ok = true;
    const int declaredCount = (int)be32At(kWwCamStyleBlobBE + 4);
    if (std::memcmp(kWwCamStyleBlobBE, "WWCS", 4) != 0 || declaredCount != kWwCamStyleCount ||
        (int)sizeof(kWwCamStyleBlobBE) != kStyleHeaderSize + kStyleRecordSize * kWwCamStyleCount)
    {
        if (s_log != nullptr) {
            s_log->write(mod_ctx, LOG_LEVEL_ERROR,
                         "[WwCamData] style blob framing BAD (magic/count/size) — regenerate "
                         "ww_cam_data.inc");
        }
        ok = false;
    }
    // Spot-check the CRAWL-family records the §670-era decode named: CC02
    // flags 0x0C2 / LN17 flags 0x042, both mapped onto extra-engine 20.
    struct Spot {
        int idx;
        const char* fourCC;
        unsigned flags;
    };
    static const Spot kSpots[] = {
        {kWwCamStyleIdx_CC02, "CC02", 0x0C2},
        {kWwCamStyleIdx_LN17, "LN17", 0x042},
    };
    for (int i = 0; ok && i < (int)(sizeof(kSpots) / sizeof(kSpots[0])); i++) {
        const unsigned char* rec =
            kWwCamStyleBlobBE + kStyleHeaderSize + kStyleRecordSize * kSpots[i].idx;
        const unsigned alg = ((unsigned)rec[4] << 8) | rec[5];
        const unsigned flags = ((unsigned)rec[6] << 8) | rec[7];
        if (std::memcmp(rec, kSpots[i].fourCC, 4) != 0 || alg != 20 || flags != kSpots[i].flags) {
            if (s_log != nullptr) {
                char buf[256];
                std::snprintf(buf, sizeof(buf),
                              "[WwCamData] spot-check %s FAILED (alg=%u flags=0x%x) — regenerate "
                              "ww_cam_data.inc",
                              kSpots[i].fourCC, alg, flags);
                s_log->write(mod_ctx, LOG_LEVEL_ERROR, buf);
            }
            ok = false;
        }
    }
    if (ok && s_log != nullptr) {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
                     "[WwCamData] resident: %d donor styles (%d bytes), %d donor-axis types, "
                     "%d BG type names. Self-test OK.",
                     kWwCamStyleCount, (int)sizeof(kWwCamStyleBlobBE), kWwCamTypeCount,
                     kWwCamBgTypeCount);
        s_log->write(mod_ctx, LOG_LEVEL_INFO, buf);
    }
}
