// ============================================================================
// ww_cam_data.h — the donor camera style/type tables, WW-layer owned.
//
// KIT-LINEAGE: donor-port
// KIT-DONOR: d/d_cam_style.cpp MatchingFor
// KIT-DONOR: d/d_cam_type.cpp MatchingFor
// KIT-DONOR: d/d_cam_type2.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
//
// THE DONOR'S CAMERA DATA IS SOURCE, NOT A FILE. There is no camtype.dat on
// the WW disc (a TP-ism; §672) — the donor compiled types[]/styles[] into the
// DOL. So this data ports the way the CRAWL algorithm did: as WW-layer code,
// GENERATED from the donor's own TUs by gen_cam_data.py (the donor source is
// the spec; the generator is the consumption-boundary translation).
//
// DONOR AXIS, DELIBERATELY. Donor type rows keep their OWN 20-slot mode axis.
// The receiver's 11-mode axis is NOT positionally equivalent past slot 4
// (receiver mode 7=aim ↔ donor slot 10; receiver 9 ↔ donor 15; donor slots
// 12/13/14/17/18/19 — manual/crawl/telescope/hang/hang/shield — have no
// receiver mode at all). Remapping rows onto the receiver's axis would drop
// donor situations, so the donor's own mode selector must drive these rows —
// the FULL-port ruling (user, 2026-08-10). Until that selector lands, this
// data is RESIDENT and self-tested but selects nothing.
// ============================================================================
#ifndef WW_CAM_DATA_H
#define WW_CAM_DATA_H

#if TARGET_PC

// Donor-axis camera type row: name + one style index per donor mode slot
// (-1 = NONE). Mirrors the donor's dCamera__Type exactly.
struct dExtWwCamType {
    char name[24];
    short styles[20];
};

// The style table as a camstyle.dat-SHAPED big-endian blob ({u8[4]; s32be
// count; 0x78-byte records}), so the receiver's BE() style accessors can read
// it exactly like file data. Record alg fields hold RECEIVER engine indices
// (shared algorithms mapped by name; absent ones on extra-engine sockets).
const void* dExtWwCam_styleDatBlob(int* o_byteSize);

const dExtWwCamType* dExtWwCam_types(int* o_count);

// Receiver-shaped SHADOW type rows as a camtype.dat-shaped BE blob ({u8[4];
// s32be count; {char name[24]; s16be [2][11]} records}) — the compatibility
// view for receiver-side readers while the WW selector is authoritative.
const void* dExtWwCam_typeShadowBlob(int* o_byteSize);

// Style-record count in the style blob (donor style indices are [0, count)).
int dExtWwCam_styleCount(void);

// BG-attribute index -> camera type name (donor mvBGTypes).
const char* const* dExtWwCam_bgTypeNames(int* o_count);

// Install-time integrity self-test + log. Called from the layer's own
// dExtWwRoom_installHooks; asserts nothing, logs its verdict.
void dExtWwCam_installData(void);

#endif  // TARGET_PC

#endif  // WW_CAM_DATA_H
