// ============================================================================
// ww_stage_loader.cpp — NATIVE ROOMS 3b: the STAGE-load seam and its lighting
// translators. See the header for why a second seam exists.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WHAT IT DOES, AND THE ORDER THAT MATTERS
// It delegates to the receiver's own loader first, then walks the dzs image
// looking for the DONOR's chunk tags. It cannot use i_stage->getEnvrInfo() the
// way the room seam uses getSclsInfo(), because the receiver's handlers never
// ran for these chunks — the tag never matched. So the seam finds them itself
// and calls the setters directly.
//
// That is 3b's stated shape reached literally: donor bytes are read at donor
// stride by code that knows the donor's layout, and RECEIVER structures are
// written. Every consumer downstream reads what it always read.
//
// NO MOUNT, NO BAKE. Nothing is mounted and no staged byte is altered; the
// donor stage.dzs is read exactly as it ships and the translated records live
// in seam-owned static pools.
// ============================================================================
#include "d/ext_plugin/ww_stage_loader.h"

#include <cstring>

#include "d/d_com_inf_game.h"
#include "d/d_stage.h"
#include "dusk/logging.h"

#ifndef DUSK_WW_STAGE_SEAM
#define DUSK_WW_STAGE_SEAM 1
#endif

#if DUSK_WW_STAGE_SEAM

namespace {

// ============================================================================
// The dzs container, same shape the receiver's own decoder walks: a u32 count
// followed by 12-byte entries of {tag[4], num u32, offset u32}. Offsets are
// file-relative and are resolved by dStage_dt_c_offsetToPtr BEFORE the loader
// runs, so by the time the seam sees them the receiver has already turned them
// into pointers. The seam therefore re-reads the RAW header itself rather than
// trusting a field it did not set.
// ============================================================================
struct WwDzsEntry {
    char tag[4];
    u8 num[4];
    u8 offset[4];
};

u32 be32(const u8* p) {
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

const u8* findChunk(const void* i_data, const char* i_tag, int* o_num) {
    if (i_data == NULL) {
        return NULL;
    }
    const u8* base = (const u8*)i_data;
    const u32 count = be32(base);
    for (u32 i = 0; i < count && i < 64; i++) {
        const WwDzsEntry* e = (const WwDzsEntry*)(base + 4 + i * 12);
        if (std::memcmp(e->tag, i_tag, 4) == 0) {
            *o_num = (int)be32(e->num);
            return base + be32(e->offset);
        }
    }
    return NULL;
}

// ============================================================================
// DONOR RECORDS, transcribed from D:/XXXXXXX/WW DP/include/d/d_stage.h.
// Given ww_ names deliberately: two of these share a NAME with a receiver
// struct of a different size, and §604's whole lesson is that the shared name
// lies.
// ============================================================================
struct WwEnvrRecord {      // donor stage_envr_info_class
    u8 pselect_id[8];
};                          // donor Size: 0x8   (receiver's is 0x41)

struct WwPaleRecord {      // donor stage_palet_info_class
    /* 0x00 */ u8 mActor_C0[3];
    /* 0x03 */ u8 mActor_K0[3];
    /* 0x06 */ u8 mBG0_C0[3];
    /* 0x09 */ u8 mBG0_K0[3];
    /* 0x0C */ u8 mBG1_C0[3];
    /* 0x0F */ u8 mBG1_K0[3];
    /* 0x12 */ u8 mBG2_C0[3];
    /* 0x15 */ u8 mBG2_K0[3];
    /* 0x18 */ u8 mBG3_C0[3];
    /* 0x1B */ u8 mBG3_K0[3];
    /* 0x1E */ u8 mFog[3];
    /* 0x21 */ u8 mVirtIdx;
    /* 0x22 */ u8 pad[2];
    /* 0x24 */ u8 mFogStartZ[4];
    /* 0x28 */ u8 mFogEndZ[4];
};                          // donor Size: 0x2C  (receiver's is 0x34)

const int kMaxEnvr = 64;
const int kMaxPale = 64;

stage_envr_info_class s_envrPool[kMaxEnvr];
stage_palette_info_class s_palePool[kMaxPale];

void copyRGB(color_RGB_class* o_dst, const u8* i_src) {
    o_dst->r = i_src[0];
    o_dst->g = i_src[1];
    o_dst->b = i_src[2];
}

// ============================================================================
// CHUNK A — EnvR. Same single field, different array length.
//
//     donor     u8 pselect_id[8]
//     receiver  u8 pselect_id[65]
//
// So this is a WIDENING, not a re-layout. The donor's eight entries copy
// straight in.
//
// STATED LIMIT — the tail is a CLAMP, not a translation. What indexes this
// table is not decoded on either side; the receiver simply has 65 slots where
// the donor has 8. Leaving 8..64 as zeroes would silently select palette 0 for
// any index above 7, so the donor's LAST entry is replicated instead: a
// reader that runs off the donor's range gets the donor's final state rather
// than a fabricated one. If a donor stage ever proves to index past 7, this is
// the line to revisit, and it says so.
// ============================================================================
int translateEnvr(const void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = findChunk(i_dzs, "EnvR", &num);
    if (raw == NULL || num <= 0) {
        return 0;
    }
    if (num > kMaxEnvr) {
        DuskLog.error("[WwStageSeam] EnvR: {} records exceeds the pool of {} — "
                      "clamped, later entries will read stale",
                      num, kMaxEnvr);
        num = kMaxEnvr;
    }
    for (int i = 0; i < num; i++) {
        const WwEnvrRecord* in = (const WwEnvrRecord*)(raw + i * sizeof(WwEnvrRecord));
        stage_envr_info_class& out = s_envrPool[i];
        for (int k = 0; k < 8; k++) {
            out.pselect_id[k] = in->pselect_id[k];
        }
        for (int k = 8; k < 65; k++) {
            out.pselect_id[k] = in->pselect_id[7];  // clamp, see the banner
        }
    }
    i_stage->setEnvrInfo(s_envrPool);
    return num;
}

// ============================================================================
// CHUNK B — Colo. MEASURED IDENTICAL, and that is worth stating rather than
// assuming: donor {u8 palette_id[8]; f32 change_rate;} at 0xC, receiver
// {u8 palette_id[8]; BE(f32) change_rate;} at 0xC. Same fields, same offsets,
// same size — the receiver's BE wrapper is only how it reads the same
// big-endian bytes the donor wrote.
//
// So there is NOTHING to translate. The chunk is simply pointed at, which is
// exactly what the receiver's own handler would have done had the tag matched.
// ============================================================================
int translateColo(const void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = findChunk(i_dzs, "Colo", &num);
    if (raw == NULL || num <= 0) {
        return 0;
    }
    i_stage->setPselectInfo((stage_pselect_info_class*)raw);
    return num;
}

// ============================================================================
// CHUNK C — Pale. The real translation: same role, different vocabulary.
//
//   donor 0x2C                        receiver 0x34
//   0x00 mActor_C0    ---------->     0x00 actor_amb_col
//   0x06 mBG0_C0      ---------->     0x03 bg_amb_col[0]
//   0x0C mBG1_C0      ---------->     0x06 bg_amb_col[1]
//   0x12 mBG2_C0      ---------->     0x09 bg_amb_col[2]
//   0x18 mBG3_C0      ---------->     0x0C bg_amb_col[3]
//   0x1E mFog         ---------->     0x21 fog_col
//   0x21 mVirtIdx     ---------->     0x2C vrboxcol_id
//   0x24 mFogStartZ   ---------->     0x24 fog_start_z    (same offset)
//   0x28 mFogEndZ     ---------->     0x28 fog_end_z      (same offset)
//
// NOT DELIVERED, and named rather than dropped silently: the donor's five K0
// colours (mActor_K0, mBG0_K0..mBG3_K0) are TEV KONSTANT register colours, and
// the receiver's palette has no field for them — its plight_col[6] is point
// lights, a different concept. Their donor consumer is the WW tevstr setup
// (settingTevStruct_*), which §407 ported the point-light half of. Until that
// side lands there is nowhere correct to put them, and inventing a mapping onto
// plight_col would be exactly the fabrication this port refuses.
//
// The receiver-only tail (bg_light_influence, cloud_shadow_density,
// bloom_tbl_id, BG1..3_amb_alpha) has no donor source at all, so it is zeroed:
// a donor stage genuinely has no opinion on those.
// ============================================================================
int translatePale(const void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = findChunk(i_dzs, "Pale", &num);
    if (raw == NULL || num <= 0) {
        return 0;
    }
    if (num > kMaxPale) {
        DuskLog.error("[WwStageSeam] Pale: {} records exceeds the pool of {} — "
                      "clamped, later palettes will read stale",
                      num, kMaxPale);
        num = kMaxPale;
    }
    for (int i = 0; i < num; i++) {
        const WwPaleRecord* in = (const WwPaleRecord*)(raw + i * sizeof(WwPaleRecord));
        stage_palette_info_class& out = s_palePool[i];
        std::memset(&out, 0, sizeof(out));

        copyRGB(&out.actor_amb_col, in->mActor_C0);
        copyRGB(&out.bg_amb_col[0], in->mBG0_C0);
        copyRGB(&out.bg_amb_col[1], in->mBG1_C0);
        copyRGB(&out.bg_amb_col[2], in->mBG2_C0);
        copyRGB(&out.bg_amb_col[3], in->mBG3_C0);
        copyRGB(&out.fog_col, in->mFog);
        out.vrboxcol_id = in->mVirtIdx;
        // Both fog planes sit at the SAME offset in both records and are both
        // big-endian floats, so the bytes move across verbatim.
        std::memcpy(&out.fog_start_z, in->mFogStartZ, 4);
        std::memcpy(&out.fog_end_z, in->mFogEndZ, 4);
    }
    i_stage->setPaletteInfo(s_palePool);
    return num;
}

}  // namespace
#endif  // DUSK_WW_STAGE_SEAM

void dExtWwStage_loadStageDzs(void* i_data, dStage_dt_c* i_stage) {
    dStage_dt_c_stageLoader(i_data, i_stage);

#if DUSK_WW_STAGE_SEAM
    // The predicate is the DATA, for the same reason as §657b and §658: a stage
    // either carries donor lighting chunks or it does not, and asking it is
    // more reliable than any name or flag. A receiver stage has no "EnvR",
    // "Colo" or "Pale", so all three translators return 0 and this is inert.
    const int envr = translateEnvr(i_data, i_stage);
    const int colo = translateColo(i_data, i_stage);
    const int pale = translatePale(i_data, i_stage);

    if (envr != 0 || colo != 0 || pale != 0) {
        DuskLog.info("[WwStageSeam] §661 donor lighting translated: EnvR={} "
                     "(8->65 widened) Colo={} (identical, pointed at) Pale={} "
                     "(field-mapped; K0 colours NOT delivered — no receiver field)",
                     envr, colo, pale);
    }
#endif
}
