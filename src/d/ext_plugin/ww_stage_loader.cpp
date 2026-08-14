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
#include "d/d_ext_save_guard.h"  // §692 declared-stage predicate
#include "d/d_ext_npc_mount.h"   // tale §773 stage-scoped cache eviction
#include "d/ext_plugin/ww_room_loader.h"  // tale §749 shared RCAM translator
#include "dusk/logging.h"

#ifndef DUSK_WW_STAGE_SEAM
#define DUSK_WW_STAGE_SEAM 1
#endif

#if DUSK_WW_STAGE_SEAM

namespace {

// ============================================================================
// §662 CRASH FIX — I named this hazard in the banner above and then walked
// into it. The first version did `base + be32(entry.offset)`, treating the
// chunk offset as FILE-RELATIVE. It is not: dStage_dt_c_offsetToPtr runs
// BEFORE the loader and rewrites every chunk offset into a resolved pointer
// in place. So the arithmetic produced garbage, and because the seam runs on
// EVERY stage load it faulted on the first TP stage at boot -- an instant
// crash, not an Outset one.
//
// Writing "the receiver has already turned them into pointers" in a comment
// and then adding the base anyway is the exact failure this file exists to
// avoid. So it now uses the receiver's OWN types -- dStage_fileHeader and
// dStage_nodeHeader -- and reads m_offset through the same OFFSET_PTR the
// receiver's handlers read. No hand arithmetic on a field someone else owns.
//
// m_tag is deliberately kept unswapped by the receiver so a 4-char literal
// compares directly; this matches the same way.
// ============================================================================
// §663: MATCH THE WAY THE RECEIVER MATCHES, byte for byte.
//
// dStage_dt_c_decode compares  (int)node->m_tag == *(int*)identifier  — it
// reinterprets the 4-char STRING as an int in NATIVE order, so both sides read
// the same bytes the same way and the comparison holds without any swap.
//
// My first version composed a BIG-ENDIAN value instead ((tag[0]<<24)|...),
// which is the mathematically tidy thing and never matched anything. The seam
// found no EnvR, no Colo and no Pale on a stage that carries 52, 10 and 57 of
// them, and reported nothing at all — a silent miss, not a crash.
//
// The lesson is the same one m_offset taught two commits ago: when reading a
// field the receiver owns, do it the receiver's way rather than the correct-
// looking way.
u32 tagOf(const char* i_tag) {
    return *(const u32*)i_tag;
}

const void* findChunk(void* i_data, const char* i_tag, int* o_num) {
    *o_num = 0;
    if (i_data == NULL) {
        return NULL;
    }
    dStage_fileHeader* file = (dStage_fileHeader*)i_data;
    const int count = file->m_chunkCount;
    if (count <= 0 || count > 64) {
        return NULL;  // not a dzs we understand; leave it entirely alone
    }
    const u32 want = tagOf(i_tag);
    dStage_nodeHeader* node = file->m_nodes;
    for (int i = 0; i < count; i++, node++) {
        if (node->m_tag == want) {
            *o_num = node->m_entryNum;
            return (const void*)node->m_offset;
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
int translateEnvr(void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = (const u8*)findChunk(i_dzs, "EnvR", &num);
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
int translateColo(void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = (const u8*)findChunk(i_dzs, "Colo", &num);
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
// §694 K0 DELIVERED — the earlier "no receiver field" claim here was STALE:
// plight_col[6] IS the sanctioned WW K0 pool, by the repo's own convention —
// convert_lighting.py:188-191 has baked exactly this six-slot order since
// №113, and DO-NOT.md DN-2 protects slot [2] (BG1_K0, dKy_get_seacolor's
// dif) by name. The runtime seam simply never executed the same mapping, so
// seam-served stages fed ZERO K0 into the live-blend chain
// (d_kankyo.cpp:2418-2434 → dungeonlight_col → dKyWw_settingTevStruct →
// setTevKColor(0)) — black KColor0 on every WW model, silent because the
// §406 canary requires C0 AND K0 both zero. Slot order per the converter:
// [0]=Actor_K0 [1..4]=BG0..3_K0 [5]=Actor_K0 again.
//
// The receiver-only tail (bg_light_influence, cloud_shadow_density,
// bloom_tbl_id, BG1..3_amb_alpha) has no donor source at all, so it is zeroed:
// a donor stage genuinely has no opinion on those.
// ============================================================================
int translatePale(void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = (const u8*)findChunk(i_dzs, "Pale", &num);
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
        // §694: the K0 pool (see banner) — convert_lighting.py:191 at runtime.
        copyRGB(&out.plight_col[0], in->mActor_K0);
        copyRGB(&out.plight_col[1], in->mBG0_K0);
        copyRGB(&out.plight_col[2], in->mBG1_K0);
        copyRGB(&out.plight_col[3], in->mBG2_K0);
        copyRGB(&out.plight_col[4], in->mBG3_K0);
        copyRGB(&out.plight_col[5], in->mActor_K0);
        out.vrboxcol_id = in->mVirtIdx;
        // Both fog planes sit at the SAME offset in both records and are both
        // big-endian floats, so the bytes move across verbatim.
        std::memcpy(&out.fog_start_z, in->mFogStartZ, 4);
        std::memcpy(&out.fog_end_z, in->mFogEndZ, 4);
    }
    i_stage->setPaletteInfo(s_palePool);
    return num;
}

// ============================================================================
// §690 VIRT (VRB0) — the missing skybox-member translation, hit at runtime.
//
// The donor's Virt record is 0x24 bytes ({u32 pad[4]; GXColor kumo;
// GXColor kumoCenter; RGB sky; RGB usoUmi; RGB kasumiMae; pad[3]} — WW
// d_stage.h:42-52); the receiver's stage_vrboxcol_info_class is 0x15 with a
// DIFFERENT field order. The staged-era Stage.arc carried an offline-converted
// VRB0 (№144), so nothing noticed that the seam never translated Virt — until
// the disc-served vanilla stage.dzs bound the donor layout raw and the sky
// engine read record boundaries into FLOAT data (the §689 receipt: a "color"
// channel counting 0..20 with a constant 0x3F exponent byte in green — IEEE
// bytes as RGB). §690's overlay-gate remainder had already named "the skybox
// members"; this is that item, forced by measurement.
//
// Field mapping per the §410/§417b receipts (same rows the sky consumers and
// the §687 donor engine read): donor sky->sky_col, kumo.rgb->kumo_top_col,
// kumoCenter.rgb->kumo_bottom_col + kumo_shadow_col.rgb (the receiver keeps
// shadow as its TP-only third slot; donor has two kumo colors), kumo.a->
// kumo_shadow_col.a (the alpha the engine reads), usoUmi->kasumi_outer_col,
// kasumiMae->kasumi_inner_col; receiver-only alphas 255.
// ============================================================================
struct WwVirtRecord {  // donor stage_vrbox_info_class, size 0x24
    u8 mPad[0x10];
    u8 mKumo[4];        // GXColor
    u8 mKumoCenter[4];  // GXColor
    u8 mSky[3];         // RGB
    u8 mUsoUmi[3];      // RGB
    u8 mKasumiMae[3];   // RGB
    u8 mTail[3];
};
static_assert(sizeof(WwVirtRecord) == 0x24, "donor Virt record must stay 0x24");

constexpr int kMaxVirt = 64;
stage_vrboxcol_info_class s_virtPool[kMaxVirt];

int translateVirt(void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = (const u8*)findChunk(i_dzs, "Virt", &num);
    if (raw == NULL || num <= 0) {
        return 0;
    }
    if (num > kMaxVirt) {
        DuskLog.error("[WwStageSeam] Virt: {} records exceeds the pool of {} — clamped",
                      num, kMaxVirt);
        num = kMaxVirt;
    }
    for (int i = 0; i < num; i++) {
        const WwVirtRecord* in = (const WwVirtRecord*)(raw + i * sizeof(WwVirtRecord));
        stage_vrboxcol_info_class& out = s_virtPool[i];
        std::memset(&out, 0, sizeof(out));
        out.sky_col.r = in->mSky[0];
        out.sky_col.g = in->mSky[1];
        out.sky_col.b = in->mSky[2];
        out.kumo_top_col.r = in->mKumo[0];
        out.kumo_top_col.g = in->mKumo[1];
        out.kumo_top_col.b = in->mKumo[2];
        out.kumo_bottom_col.r = in->mKumoCenter[0];
        out.kumo_bottom_col.g = in->mKumoCenter[1];
        out.kumo_bottom_col.b = in->mKumoCenter[2];
        out.kumo_shadow_col.r = in->mKumoCenter[0];
        out.kumo_shadow_col.g = in->mKumoCenter[1];
        out.kumo_shadow_col.b = in->mKumoCenter[2];
        out.kumo_shadow_col.a = in->mKumo[3];
        out.kasumi_outer_col.r = in->mUsoUmi[0];
        out.kasumi_outer_col.g = in->mUsoUmi[1];
        out.kasumi_outer_col.b = in->mUsoUmi[2];
        out.kasumi_outer_col.a = 255;
        out.kasumi_inner_col.r = in->mKasumiMae[0];
        out.kasumi_inner_col.g = in->mKasumiMae[1];
        out.kasumi_inner_col.b = in->mKasumiMae[2];
        out.kasumi_inner_col.a = 255;
    }
    i_stage->setVrboxcolInfo(s_virtPool);
    return num;
}

// ============================================================================
// §692 STAG — TRANSLATE, per the mechanical derivation (bus §686): every
// record-size divergence (donor 0x20 -> receiver 0x3C) is settled by a
// seam-owned translated record, FILI's exact analogue. Kills the live wound
// (GetParticleNo reading to +0x3A past the record on EVERY stage load) and
// neutralizes the five in-range foreign-meaning readers (EscapeWarp ->
// dStage_changeScene the worst) that persist structurally could not.
//
// User rulings applied (defaults accepted): donor STType 4-7 have no receiver
// case -> receiver sees 0 (OUTDOORS-equivalent) and the DONOR value is kept
// layer-side (nothing is dropped — dExtWwStage_donorStType()); EscapeWarp -> 0
// (the donor bits there are schbit-enable, semantically unrelated).
//
// TIMING (measured, bus §686): dStage_stagInfoInit consumed GetSaveTbl from
// the RAW donor record before this seam runs — safe, because WW save-table
// ids are 0-15 and the receiver's 5-bit read equals the donor's 7-bit read
// for every value the donor ships. Every dangerous reader (particle rows,
// stage title, dmap floats, escape warp) runs at phase_2 or later — after
// this swap.
//
// Every receiver-only value below is DERIVED from the receiver's own
// documented sentinels, not invented: particle rows 0xFF -> the receiver's
// own 255-sentinel routes to the in-range donor-correct GetParticleNo
// overload (d_s_play.cpp:1554-1556); title 0 -> JUT_WARN branch; dmap floats
// 0.0f -> the receiver's own 10800 default; msgGroup 0 -> §644's receipt
// (zel_00.bmg IS index 0; the hook stays, now redundant but not wrong).
// ============================================================================
stage_stag_info_class s_stagPool;
u32 s_donorStType = 0;

int translateStag(void* i_dzs, dStage_dt_c* i_stage) {
    int num = 0;
    const u8* raw = (const u8*)findChunk(i_dzs, "STAG", &num);
    if (raw == NULL || num <= 0) {
        return 0;
    }
    const auto be16 = [](const u8* p) { return (u16)((p[0] << 8) | p[1]); };
    const auto be32 = [](const u8* p) {
        return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
    };
    std::memset(&s_stagPool, 0, sizeof(s_stagPool));
    // near/far: BE floats, byte-verbatim.
    std::memcpy(&s_stagPool, raw, 8);
    s_stagPool.mCameraType = raw[0x08];
    // 0x09: ChkKeyDisp bit0 + saveTbl bits1-5 (donor uses 1-7; WW ids fit);
    // receiver-only WolfDashType bits6-7 neutral.
    s_stagPool.field_0x09 = raw[0x09] & 0x3F;
    // 0x0A: UpButton = donor bits0-1 (donor bit2 is its own field, dropped);
    // ParticleNo bits3-10 verbatim; receiver-only ArchiveHeap/MiniMap -> 0.
    s_stagPool.field_0x0a = (u16)(be16(raw + 0x0A) & 0x07FB);
    // 0x0C: SchSec+TimeH verbatim; STType per the vocabulary ruling;
    // receiver-only Arg0 bits20-27 -> 0.
    const u32 d0c = be32(raw + 0x0C);
    s_donorStType = (d0c >> 16) & 7;
    const u32 stType = s_donorStType <= 3 ? s_donorStType : 0;
    s_stagPool.field_0x0c = (d0c & 0x0000FFFFu) | (stType << 16);
    // 0x10: CullPoint verbatim; donor schbit halves (16-31, where the
    // receiver's EscapeWarp lives) -> 0 per the ruling.
    s_stagPool.field_0x10 = be32(raw + 0x10) & 0x0000FFFFu;
    std::memset(s_stagPool.field_0x14, 0xFF, sizeof(s_stagPool.field_0x14));
    s_stagPool.mGapLevel = 0;
    s_stagPool.mRangeUp = 0;
    s_stagPool.mRangeDown = 0;
    s_stagPool.field_0x20 = 0.0f;
    s_stagPool.field_0x24 = 0.0f;
    s_stagPool.mMsgGroup = 0;
    s_stagPool.mStageTitleNo = 0;
    std::memset(s_stagPool.mParticleNo, 0xFF, sizeof(s_stagPool.mParticleNo));
    i_stage->setStagInfo(&s_stagPool);
    DuskLog.info("[WwStageSeam] §692 STAG translated (0x20->0x3C): donor STType {} -> "
                 "receiver {} (donor value retained), saveTbl {}, cull {}",
                 s_donorStType, stType, (raw[0x09] >> 1) & 0x7F,
                 be32(raw + 0x10) & 0xFFFF);
    return 1;
}

}  // namespace

// Nothing-is-dropped accessor: the donor's own stage-type vocabulary (SEA=7
// for Outset's host) for future donor consumers.
u32 dExtWwStage_donorStType() {
#if DUSK_WW_STAGE_SEAM
    return s_donorStType;
#else
    return 0;
#endif
}
#endif  // DUSK_WW_STAGE_SEAM

void dExtWwStage_loadStageDzs(void* i_data, dStage_dt_c* i_stage) {
#if DUSK_WW_STAGE_SEAM
    // tale §773: a NEW stage's resources are being installed — the previous
    // stage's parsed-model cache entries (positional room-arc keys) die HERE,
    // matching getStageRes's own lifecycle. Before the translators, so no
    // consumer of this stage can ever see the last stage's parse.
    dExtNpcMount_dropStageScopedModels("stage.dzs load");
#endif
    dStage_dt_c_stageLoader(i_data, i_stage);

#if DUSK_WW_STAGE_SEAM
    // The predicate is the DATA, for the same reason as §657b and §658: a stage
    // either carries donor lighting chunks or it does not, and asking it is
    // more reliable than any name or flag. A receiver stage has no "EnvR",
    // "Colo" or "Pale", so all three translators return 0 and this is inert.
    const int envr = translateEnvr(i_data, i_stage);
    const int colo = translateColo(i_data, i_stage);
    const int pale = translatePale(i_data, i_stage);
    const int virt = translateVirt(i_data, i_stage);
    // §692: STAG exists in BOTH dialects, so the predicate is the STAGE, not
    // the data: declared donor stages come off the disc with the 0x20 donor
    // record; neutral R_DL*/F_DL* stages carry rebuilt receiver-shape STAGs
    // and must not be touched.
    const char* stagStage = dComIfGp_getStartStageName();
    const int stag = (stagStage != NULL && dExtWwSave_isDeclaredWwStage(stagStage))
                         ? translateStag(i_data, i_stage)
                         : 0;
    (void)stag;
    // tale §749: the STAGE table's RCAM was never translated — donor 0x14
    // records read at receiver 0x18 on every WW stage (LinkRM Stage.arc
    // carries RCAMx1, the count==1 blind-spot case exactly). Same adjacency-
    // first translator as the room path, reserved stage slot.
    if (stagStage != NULL && dExtWwSave_isDeclaredWwStage(stagStage)) {
        dExtWwRoom_translateRcamShared(i_stage, kWwRcamStageSlot, i_data);
    }

    if (envr != 0 || colo != 0 || pale != 0 || virt != 0) {
        DuskLog.info("[WwStageSeam] §661 donor lighting translated: EnvR={} "
                     "(8->65 widened) Colo={} (identical, pointed at) Pale={} "
                     "(field-mapped; §694 K0 -> plight_col pool) "
                     "Virt={} (§690 skybox rows 0x24->0x15)",
                     envr, colo, pale, virt);
    }
#endif
}
