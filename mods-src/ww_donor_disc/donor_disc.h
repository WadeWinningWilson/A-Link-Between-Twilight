// ============================================================================
// donor_disc.h — the donor-disc layer's SERVICE-SHAPED interface (L2b).
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// ONE PLUGIN, BY RULING — but the disc reader is kept behind a service-shaped
// boundary (ServiceHeader-led function-pointer table, ModContext* first-args)
// so that if a reusable donor-disc service is ever wanted by another mod,
// exporting it is EXPORT_SERVICE_AS + a ModMetaExport declaration, not a
// refactor. Same discipline as L4, one layer down: shape the seam now so the
// later move is plumbing rather than surgery. (ttw-methods-review.md L2b.)
//
// The disc reader is the BOTTOM layer of the precedence chain — donor,
// immutable. It serves bytes exactly as they sit on the user's own disc
// image; it never edits, decompresses, or re-encodes anything (zero-bake).
// ============================================================================
#pragma once

#include <mods/api.h>

// Reserved now so a later export changes no consumer: same id scheme as the
// host services, versioned exactly like them.
#define WW_DONOR_DISC_SERVICE_ID "dev.twilitrealm.dusklight.ww-donor-disc"
#define WW_DONOR_DISC_SERVICE_MAJOR 1u
#define WW_DONOR_DISC_SERVICE_MINOR 0u

typedef enum DonorDiscVerdict {
    DONOR_DISC_UNATTACHED = 0,
    // R3 (§332) roster gate: sys/boot.bin + sys/fst.bin SHA-256 both match the
    // sanctioned GZLE01 dump roster. boot.bin pins the FST offset/size fields,
    // fst.bin pins every file's name/offset/length — together they pin the
    // whole disc layout.
    DONOR_DISC_ON_ROSTER = 1,
    // Attached image is readable GC media but NOT the sanctioned dump. The
    // reader refuses to serve in this state (wrong-disc gate).
    DONOR_DISC_OFF_ROSTER = 2,
} DonorDiscVerdict;

typedef struct DonorDiscService {
    ServiceHeader header;

    /*
     * Attach a plain GameCube disc image. On any refusal the service stays
     * detached and out_error carries the LEGIBLE reason (19c): unreadable
     * path, compressed/RVZ image, not GC media, wrong game id, OFF-ROSTER.
     * Never asserts — a missing or wrong disc is a normal handled condition.
     */
    ModResult (*attach)(ModContext* ctx, const char* image_path, ModError* out_error);
    void (*detach)(ModContext* ctx);
    DonorDiscVerdict (*verdict)(ModContext* ctx);

    /*
     * FST enumeration. Paths are disc-relative without a leading slash,
     * exactly as the FST names them ("res/Stage/sea/Room44.arc").
     * file_info's out_path stays valid until detach.
     */
    int32_t (*file_count)(ModContext* ctx);
    ModResult (*file_info)(
        ModContext* ctx, int32_t index, const char** out_path, uint32_t* out_size);
    /* Case-insensitive exact-path lookup. MOD_UNAVAILABLE when absent. */
    ModResult (*find)(ModContext* ctx, const char* path, int32_t* out_index);

    /* Read length bytes at offset within file index. Short reads only at EOF. */
    ModResult (*read)(ModContext* ctx, int32_t index, uint32_t offset, void* buffer,
        uint32_t length, uint32_t* out_read);
} DonorDiscService;

// The single in-binary instance (L2b: internal today, exportable later).
const DonorDiscService* wwDonorDisc_service(void);
