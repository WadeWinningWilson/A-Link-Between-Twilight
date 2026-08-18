// ============================================================================
// ww_room_loader.cpp — NATIVE ROOMS 3b, Phase 1: the room-load dispatch seam.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// Phase 1 carries no donor code. The REF above is the reference the Phase-2
// chunk readers will be ported against (d/d_stage.cpp dStage_dt_c_roomLoader,
// :2180) -- recorded now so Phase 2 does not have to re-find it.
//
// NOTE on the banner format: KIT-DONOR takes a BARE value. This shipped as
// `none (Phase 1 carries no donor code)` and banner_lint flagged it DISAGREES,
// because it exact-matches the value against objects in the donor's own
// configure.py and a parenthetical is not one. The qualifier was true and in
// the wrong field; prose belongs below the block, not inside a matched value.
//
// WHAT THIS IS, AND WHAT IT DELIBERATELY IS NOT
// Phase 1 of docs/WW Linked/native-rooms-3b-scope.md: a seam, not a loader.
// The receiver's ONE room-load call site (d_s_room.cpp phase_2) routes here;
// this function delegates 100% of every call to the receiver's own
// dStage_dt_c_roomLoader. It reads no chunks, translates no records, and
// carries no donor code. Landing it changes ONE observable thing: a log line
// on WW host stages, so a playtest can confirm the seam is actually in the
// path before Phase 2 puts weight on it.
//
// This is step 19 Phase 1's shape reused deliberately: land the seam INERT,
// prove it at runtime, and only then migrate behaviour through it — the
// cut-over order that let 129 symbols sever in five reviewable steps.
//
// WHY THE SEAM EXISTS AT ALL (instead of Phase 2 branching at the call site)
// Phase 2 ports the donor's chunk readers one at a time — SCLS first, then
// RCAM/FILI, then Pale/Virt/EnvR as a unit (§607: six of the shared chunks
// have DIFFERENT record sizes across the lineage, so a WW room.dzr fed to
// the receiver's readers yields silent data, not a crash). Each ported reader
// slots in HERE, per-chunk, WW-scoped — the receiver's call site never
// changes again after this patch, and mainline TP never sees any of it.
//
// KILL SWITCH: DUSK_WW_ROOM_SEAM (default 1). At 0 the seam compiles to a
// bare forwarding call with no predicate check and no log — byte-equivalent
// routing for exact-revert, same discipline as D_ALBW_SUMO_MENU_LEAVE_FIX.
//
// WW-SCOPING (standing rule, feedback_ww_shared_path_scoping): the predicate
// is dExtWwSave_isWwHostStage at the SOURCE of the divergence, not per-consumer
// guards downstream. On non-WW stages the WW branch is unreachable.
// ============================================================================
#include "d/ext_plugin/ww_room_loader.h"
#include "d/ext_plugin/ww_cam_crawl.h"  // §669 dExtWwCam_installCrawl
#include "d/ext_plugin/ww_cam_data.h"   // donor camera style/type tables (resident)
#include "d/ext_plugin/ww_cam_select.h" // donor camera selector (type/mode/style)

#include <cstdio>
#include <cstring>

#include "d/d_ext_save_guard.h"  // dExtWwSave_isWwHostStage
#include "SSystem/SComponent/c_math.h"            // cM_ssin/cM_scos (§678)
#include "JSystem/J3DGraphBase/J3DShape.h"        // shape culling bounds (§678)
#include "d/d_com_inf_game.h"    // dComIfGp_getStartStageName
#include "d/d_stage.h"           // dStage_dt_c_roomLoader
#include "d/d_resorce.h"         // dRes_info_c::setRes (§634 publish)
#include "JSystem/JKernel/JKRArchive.h"  // getResSize — the §858 pre-resolve's bound
#include "d/d_bg_w.h"            // cBgD_t
#include "d/d_ext_npc_mount.h"   // §334 repack, DN-3 consume-time resolver
#include "d/d_msg_object.h"      // §644 message-archive hook
#include "dusk/logging.h"

// ============================================================================
// Kill switch — 1: seam active (WW stages log their pass-through, proving the
// route at runtime). 0: bare forward, no predicate, no log — exact revert.
// ============================================================================
#ifndef DUSK_WW_ROOM_SEAM
#define DUSK_WW_ROOM_SEAM 1
#endif

// ============================================================================
// Phase 2 kill switch — 1: ported chunk readers run on WW host stages.
// 0: Phase 1 behaviour exactly (pass-through log only, delegation untouched).
// Chunks are independently revertible below this: each translator is one
// function with one call site in the WW branch — step 19's granularity.
// ============================================================================
#ifndef DUSK_WW_ROOM_CHUNKS
#define DUSK_WW_ROOM_CHUNKS 1
#endif

#if DUSK_WW_ROOM_SEAM && DUSK_WW_ROOM_CHUNKS
// ============================================================================
// PHASE 2, CHUNK 1: SCLS — the exit table. THE measured stride difference.
//
// V1 (§604, verified at both headers + Winditor):
//
//   DONOR (WW)  0xC                      RECEIVER (TP)  0xD
//   0x0 char mStage[8]                   0x0 char mStage[8]
//   0x8 u8   mStart                      0x8 u8   mStart
//   0x9 u8   mRoom                       0x9 s8   mRoom
//   0xA u8   mWipe        <-- HERE       0xA u8   field_0xa   (no donor field)
//   0xB u8   field_0xb                   0xB u8   field_0xb
//                                        0xC u8   mWipe       <-- MOVED
//
// Feeding donor bytes to receiver consumers misreads mWipe as the NEXT
// record's first mStage byte and drifts every later record by its index
// (§604). So the WW path re-reads the chunk at DONOR stride and materializes
// RECEIVER-layout records in a seam-owned pool, translated BY NAME — the §609
// port rule. The donor's parsing semantics execute; the receiver's consumers
// never learn anything happened.
//
// Donor record, transcribed from D:/XXXXXXX/WW DP/include/d/d_stage.h:80
// (stage_scls_info_class — the SAME NAME as the receiver's, which is exactly
// why this file gives it a ww_ name; §604's whole finding is that the shared
// name lies).
// ============================================================================
struct WwSclsRecord {
    /* 0x0 */ char mStage[8];
    /* 0x8 */ u8 mStart;
    /* 0x9 */ u8 mRoom;
    /* 0xA */ u8 mWipe;
    /* 0xB */ u8 field_0xb;
};  // donor Size: 0xC
STATIC_ASSERT(sizeof(WwSclsRecord) == 0xC);
STATIC_ASSERT(sizeof(stage_scls_info_class) == 0xD);

// ============================================================================
// Per-room translation pool. Static, not heap: the room loader runs BEFORE
// phase_2 sets the room's expand-heap current (d_s_room.cpp:629-634), so a
// heap allocation here would land on whichever heap happens to be current —
// the lifetime bug d_stage's own №86/№93 notes warn about. 64 slots mirrors
// dStage_roomControl_c's room table.
//
// dummy.m_entries is an OFFSET_PTR: SELF-RELATIVE (offset from its own
// address, dusk/offset_ptr.h) with bit 31 as the relocated flag. Keeping the
// records adjacent to the dummy in one struct makes the offset a constant 4
// and the encoding trivial — and safe, because both live in the same static
// object, so the ±1GB range can never be exceeded.
// ============================================================================
static const int kWwSclsMax = 32;
static const int kWwRoomSlots = 64;

struct WwSclsPool {
    stage_scls_info_dummy_class dummy;              // {BE num, OFFSET_PTR entries}
    stage_scls_info_class recs[kWwSclsMax];         // receiver-layout records
};
static WwSclsPool s_wwScls[kWwRoomSlots];

static void wwRoom_translateScls(dStage_dt_c* i_stage, int i_roomNo) {
    stage_scls_info_dummy_class* src = i_stage->getSclsInfo();
    if (src == NULL) {
        return;  // room carries no SCLS — nothing to translate, nothing to log
    }
    int num = src->num;
    const WwSclsRecord* in =
        (const WwSclsRecord*)(stage_scls_info_class*)src->m_entries;
    if (in == NULL || num <= 0) {
        return;
    }
    if (i_roomNo < 0 || i_roomNo >= kWwRoomSlots) {
        DuskLog.error("[WwRoomSeam] SCLS: roomNo {} outside pool — left at donor "
                      "stride, exits WILL misread. Widen kWwRoomSlots.", i_roomNo);
        return;
    }
    if (num > kWwSclsMax) {
        // No silent caps: dropping records loudly beats stride-garbage silently.
        DuskLog.error("[WwRoomSeam] SCLS: {} donor records exceed pool of {} — "
                      "NOT translated. Exits in this room will misread until "
                      "kWwSclsMax is raised.", num, kWwSclsMax);
        return;
    }

    WwSclsPool& pool = s_wwScls[i_roomNo];
    for (int i = 0; i < num; i++) {
        // ====================================================================
        // BY NAME, never by position (§609 port rule) — and receiver-only
        // fields get the receiver's own NEUTRAL encodings, read from its
        // accessors, not zeros:
        //
        //   field_0xa >>4        arrival timeH[3:0]   (getTimeH)
        //   field_0xb & 0xF      scene layer, 15=none (getSceneLayer; >=15 -> -1)
        //   field_0xb & 0x10     timeH bit 4          (31 = NO time change,
        //                                              d_stage.cpp:3259 "timeH < 31")
        //   field_0xb >> 5       wipe duration        (getWipeTime; 0 = default)
        //
        // WW has none of these concepts in SCLS — its field_0xb is padding,
        // so it is DROPPED, not copied: copying it would feed TP's layer and
        // wipe-time bits phantom values. A zero here would have been worse
        // still — timeH=0 forces every WW exit to MIDNIGHT.
        // ====================================================================
        stage_scls_info_class& out = pool.recs[i];
        memcpy(out.mStage, in[i].mStage, sizeof(out.mStage));
        out.mStart = in[i].mStart;
        out.mRoom = (s8)in[i].mRoom;
        out.field_0xa = 0xF0;  // timeH[3:0]=F
        out.field_0xb = 0x1F;  // layer=15 (none) | timeH bit4 (=> 31, no change)
        out.mWipe = in[i].mWipe;
        DuskLog.info("[WwRoomSeam] SCLS[{}] stage='{:.8s}' start={} room={} "
                     "wipe={} (donor 0xC -> receiver 0xD)",
                     i, in[i].mStage, in[i].mStart, in[i].mRoom, in[i].mWipe);
    }

    pool.dummy.num = num;
    // ------------------------------------------------------------------------
    // Self-relative encode, mirroring OffsetPtr::setBase's storage format:
    // recs sits at a fixed positive distance from m_entries inside this pool,
    // bit 31 marks it relocated so nothing tries to rebase it.
    // ------------------------------------------------------------------------
    s32 diff = (s32)((char*)pool.recs - (char*)&pool.dummy.m_entries);
    pool.dummy.m_entries.value.value = (s32)(diff | 0x8000'0000);

    i_stage->setSclsInfo(&pool.dummy);
    DuskLog.info("[WwRoomSeam] SCLS: {} record(s) re-read at donor stride for "
                 "room {} — receiver now consumes translated copies.", num, i_roomNo);
}
// ============================================================================
// PHASE 2, CHUNK 2: FILI — room properties. NOT a stride bug: a SEMANTIC one.
//
// V1 (§607): donor 0x8 {u32 mParam, f32 mSeaLevel} vs receiver 0x20. Shared
// offsets 0x0/0x4 — but the mParam BIT FIELDS are DISJOINT between the games:
//
//   donor  mParam    bit0 DarkOn · bit1 AgbCom · bit2 AgbHover ·
//                    bits21-28 ParticleNo · bit30 SongOk
//   recv   mParameters  bits3-5 MiniMap · bits7-14 GRASSLIGHT ·
//                    bits15-17 WindDir · bits18-19 WindLevel · bit29 EnemyAppear
//
// And the receiver's struct reads 0x18 bytes PAST the donor chunk:
// field_0x8/0xc, mDefaultCamera@0x1A, mBitSw@0x1B, mMsg@0x1C are adjacent-
// chunk garbage on every WW room TODAY (R00's FILI sits before PLYR data).
//
// Translation: mSeaLevel copies (same meaning, same offset). Every receiver-
// only field gets its receiver-neutral value, receipts:
//   mParameters   = 0     wind 0, grasslight 0, minimap 0, enemy-appear off
//   mDefaultCamera= 0     d_camera.cpp:508-516 — 0 is the pre-read default
//                         used when a room has no FILI at all
//   mBitSw        = 0xFF  TP switch convention: 0xFF = no switch
//   mMsg          = 0     d_msg_flow.cpp:657 — Midna msg-flow only; logged if
//                         a WW room ever hits that path
// Donor DarkOn/Agb/ParticleNo/SongOk have NO receiver slots — WW-layer
// consumers that need them read the DONOR record, which stays intact in the
// dzr. Nothing is lost; the two views are simply separated.
// ============================================================================
struct WwFiliRecord {
    /* 0x0 */ u32 mParam;
    /* 0x4 */ f32 mSeaLevel;
};  // donor Size: 0x8
STATIC_ASSERT(sizeof(dStage_FileList_dt_c) == 0x20);

static dStage_FileList_dt_c s_wwFili[kWwRoomSlots];

static void wwRoom_translateFili(dStage_dt_c* i_stage, int i_roomNo) {
    dStage_FileList_dt_c* src = i_stage->getFileListInfo();
    if (src == NULL || i_roomNo < 0 || i_roomNo >= kWwRoomSlots) {
        return;
    }
    // Donor bytes, read at donor layout (BE data — go through the receiver's
    // own BE fields for the two shared offsets, they coincide exactly).
    u32 donorParam = src->mParameters;   // BE-decoded donor mParam
    f32 sea = src->mSeaLevel;

    dStage_FileList_dt_c& out = s_wwFili[i_roomNo];
    out.mParameters = 0;
    out.mSeaLevel = sea;
    out.field_0x8 = 0.0f;
    out.field_0xc = 0.0f;
    memset(out.field_0x10, 0, sizeof(out.field_0x10));
    out.mDefaultCamera = 0;
    out.mBitSw = 0xFF;
    out.mMsg = 0;

    i_stage->setFileListInfo(&out);
    DuskLog.info("[WwRoomSeam] FILI room {}: sea={} donorParam=0x{:08X} "
                 "(dark={} particle={} song={}) -> receiver-neutral copy; "
                 "trailing 0x18 bytes no longer read past the donor chunk.",
                 i_roomNo, sea, donorParam, donorParam & 1,
                 (donorParam & 0x1FE00000) >> 21, (donorParam >> 30) & 1);
}
// ============================================================================
// PHASE 2, CHUNK 3: RCAM — room cameras. Donor 0x14, receiver 0x18 (§607) —
// receiver = donor + BE(u16) flags@0x14 (camera behavior bits 13-15,
// d_camera.cpp:174-183) + BE(u16)@0x16.
//
// THE MOUNT'S ARCS ARE MIXED-FORMAT — measured, not assumed (§612):
//   R44_00.arc RCAM  num=5, gap 120 = 5*0x18: RECEIVER format, all five
//                    records decode clean at 0x18 (Subject x3, FixdPos x2,
//                    arrows 0-4, flag words ZEROED). Baked by the host-stage
//                    tooling, not bake_room_chunks (RCAM is not in its
//                    KNOWN_SIZE) — already correct for TP's reader.
//   SCLS             donor format (bake_room_chunks KNOWN_SIZE 0xC, §610).
//
// So a blanket donor-stride re-read would CORRUPT receiver-format chunks.
// This translator DETECTS the format per chunk and translates only donor-
// stride data (donor-verbatim rooms, the 3b direction); receiver-format
// passes through untouched. Detection = every record's cam_type must be
// printable-ASCII-or-NUL at the candidate stride; genuine donor 0x14 data
// misaligns 0x18 decode into name fragments and vice versa (proven on R44:
// donor-stride decode yielded 'os'/'ixd' fragments of FixdPos).
// ============================================================================
struct WwRcamRecord {
    /* 0x00 */ char m_cam_type[16];
    /* 0x10 */ u8 m_arrow_idx;
    /* 0x11 */ u8 field_0x11;
    /* 0x12 */ u8 field_0x12;
    /* 0x13 */ u8 field_0x13;
};  // donor Size: 0x14
STATIC_ASSERT(sizeof(WwRcamRecord) == 0x14);
STATIC_ASSERT(sizeof(stage_camera2_data_class) == 0x18);

static const int kWwRcamMax = 16;

struct WwRcamPool {
    stage_camera_class dummy;
    stage_camera2_data_class recs[kWwRcamMax];
};
static WwRcamPool s_wwRcam[kWwRoomSlots];

// ============================================================================
// tale §749 (History): the ADJACENCY RULE — with count==1 both strides read the
// same clean name and the plausibility test passes BOTH ways (LinkRM Room0's
// single FixdPos took the receiver branch and fed RARO floats into the camera
// tail). The TRUE stride is arithmetic, not heuristic: the dzr's own chunk
// table gives (nextChunkDataOff - chunkDataOff) / count. Adjacency cannot lie
// about its own file; plausibility remains only the fallback for the last
// chunk in the file.
// DZR chunk table (BE): u32 count; then count x { char tag[4]; u32 num; u32 off }.
// ============================================================================
static u32 wwRoom_be32(const u8* p) {
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

static int wwRoom_chunkStrideByAdjacency(const u8* i_dzr, const u8* i_dataPtr, int i_num) {
    if (i_dzr == NULL || i_dataPtr == NULL || i_num <= 0 || i_dataPtr <= i_dzr) {
        return -1;
    }
    const u32 dataOff = (u32)(i_dataPtr - i_dzr);
    const u32 chunkCount = wwRoom_be32(i_dzr);
    if (chunkCount == 0 || chunkCount > 64) {
        return -1;  // implausible table — refuse to derive
    }
    u32 nextOff = 0xFFFFFFFFu;
    bool sawSelf = false;
    for (u32 c = 0; c < chunkCount; c++) {
        const u32 off = wwRoom_be32(i_dzr + 4 + c * 12 + 8);
        if (off == dataOff) {
            sawSelf = true;
        } else if (off > dataOff && off < nextOff) {
            nextOff = off;
        }
    }
    if (!sawSelf || nextOff == 0xFFFFFFFFu) {
        return -1;  // pointer not table-backed, or last data block in the file
    }
    if ((nextOff - dataOff) % (u32)i_num != 0) {
        return -1;
    }
    return (int)((nextOff - dataOff) / (u32)i_num);
}

static bool wwRoom_namesPlausible(const u8* base, int num, int stride) {
    for (int i = 0; i < num; i++) {
        const u8* nm = base + i * stride;
        for (int c = 0; c < 16; c++) {
            u8 b = nm[c];
            if (b != 0 && (b < 0x20 || b >= 0x7F)) {
                return false;
            }
        }
    }
    return true;
}

static void wwRoom_translateRcam(dStage_dt_c* i_stage, int i_roomNo, const u8* i_dzr) {
    stage_camera_class* src = i_stage->getCamera();
    if (src == NULL || i_roomNo < 0 || i_roomNo >= kWwRoomSlots) {
        return;
    }
    int num = src->num;
    const u8* raw = (const u8*)(stage_camera2_data_class*)src->m_entries;
    if (raw == NULL || num <= 0) {
        return;
    }

    // tale §749: adjacency FIRST — the file's own chunk table is authoritative
    // where derivable; the name test is only the last-chunk fallback.
    const int derived = wwRoom_chunkStrideByAdjacency(i_dzr, raw, num);
    if (derived == 0x18) {
        DuskLog.info("[WwRoomSeam] RCAM room {}: {} record(s) receiver-format "
                     "(adjacency-verified stride 0x18) — untouched.", i_roomNo, num);
        return;
    }
    if (derived != 0x14 && derived != -1) {
        DuskLog.error("[WwRoomSeam] RCAM room {}: adjacency derives stride {:#x} "
                      "(neither dialect) — falling back to name test.", i_roomNo, derived);
    }

    bool recvOk = wwRoom_namesPlausible(raw, num, 0x18);
    bool donorOk = wwRoom_namesPlausible(raw, num, 0x14);
    if (derived == 0x14) {
        // Adjacency says DONOR — overrides the name test's count==1 blind spot
        // (both strides read one clean name; the old receiver-wins tie fed
        // RARO's floats into the camera tail — History's §749 proof).
        recvOk = false;
        donorOk = true;
        DuskLog.info("[WwRoomSeam] RCAM room {}: adjacency-verified DONOR stride "
                     "0x14 ({} record(s)) — translating.", i_roomNo, num);
    }
    if (recvOk) {
        DuskLog.info("[WwRoomSeam] RCAM room {}: {} record(s) already receiver-"
                     "format — untouched (name test; adjacency unavailable).",
                     i_roomNo, num);
        return;
    }
    if (!donorOk) {
        DuskLog.error("[WwRoomSeam] RCAM room {}: {} record(s) implausible at "
                      "BOTH strides — left untouched, cameras may misread. "
                      "Dump the chunk.", i_roomNo, num);
        return;
    }
    if (num > kWwRcamMax) {
        DuskLog.error("[WwRoomSeam] RCAM room {}: {} donor records exceed pool "
                      "of {} — NOT translated. Raise kWwRcamMax.",
                      i_roomNo, num, kWwRcamMax);
        return;
    }

    const WwRcamRecord* in = (const WwRcamRecord*)raw;
    WwRcamPool& pool = s_wwRcam[i_roomNo];
    for (int i = 0; i < num; i++) {
        stage_camera2_data_class& out = pool.recs[i];
        memcpy(out.m_cam_type, in[i].m_cam_type, sizeof(out.m_cam_type));
        out.m_arrow_idx = in[i].m_arrow_idx;
        out.field_0x11 = in[i].field_0x11;
        out.field_0x12 = in[i].field_0x12;
        out.field_0x13 = in[i].field_0x13;
        out.field_0x14 = 0;  // camera flags: neutral, d_camera.cpp:174 reads bits 13-15
        out.field_0x16 = 0;
        DuskLog.info("[WwRoomSeam] RCAM[{}] type='{:.16s}' arrow={} "
                     "(donor 0x14 -> receiver 0x18, flags neutral)",
                     i, in[i].m_cam_type, in[i].m_arrow_idx);
    }
    pool.dummy.num = num;
    s32 diff = (s32)((char*)pool.recs - (char*)&pool.dummy.m_entries);
    pool.dummy.m_entries.value.value = (s32)(diff | 0x8000'0000);
    i_stage->setCamera(&pool.dummy);
    DuskLog.info("[WwRoomSeam] RCAM: {} donor-stride record(s) translated for "
                 "room {}.", num, i_roomNo);
}
// tale §749: shared entry — the STAGE loader translates its own RCAM through
// the same adjacency-first path (slot 63 reserved for the stage table; rooms
// 0-62 keep their own slots, and no WW stage has a room 63 in play).
void dExtWwRoom_translateRcamShared(dStage_dt_c* i_stage, int i_slot, const void* i_dzr) {
    wwRoom_translateRcam(i_stage, i_slot, (const u8*)i_dzr);
}

// ============================================================================
// PHASE 2, CHUNK 4: room.dzb ATTRIBUTES — §334's vocabulary, second consumer.
//
// §332 measured the mismatch byte-exactly: WW packs its material attCode in
// PolyInf1 bits 16-20, and the receiver reads that same word as att0 (12-15),
// att1 (16-18) and groundCode (19-23). Untranslated, WW attCode 1 reads as TP
// att1=1 — a SINK class — and the floor swallows the player.
//
// §334 already solves this and was only ever wired to the mount's OBJECT dzbs.
// A room arc's dzb is a SECOND CONSUMER of the same translation, not a second
// mechanism.
//
// WHY IT BELONGS HERE AND NOT IN daBg. It shipped once as a call inside
// d_a_bg.cpp, which made a receiver TU reference the WW layer — a leg, and the
// separability gate caught it. The dzb is consumed at BG create, which happens
// AFTER room load, so the seam can translate it in place and daBg's untouched
// code reads a dzb already speaking the receiver's vocabulary.
//
// Idempotent by construction (§334f), so a room reload re-entering here is safe.
// ============================================================================
static void wwRoom_repackDzb(int i_roomNo) {
    const char* arcName = dComIfG_getRoomArcName(i_roomNo);
    cBgD_t* dzb = (cBgD_t*)dComIfG_getStageRes(arcName, "room.dzb");
    if (dzb == NULL) {
        return;
    }
    dExtWw_repackDzbAttributes(dzb, "room.dzb");
}

// ============================================================================
// PHASE 2, CHUNK 6: OBJECT-ARC PRE-RESOLVE — the LBNK-shaped fix (§858 RULED,
// evidence tale §856; BUILD-QUEUE row "WW room OBJECT-ARC PRE-RESOLVE").
//
// THE WINDOW IT CLOSES, measured in §856 and not re-derived here: a pot's
// create completes in ~1 frame (profile compiled in, bank resident) while a
// shelf's create phases through its Otana arc resolve — ~160 log lines later.
// In that window the pot, authored at shelf-top Y, runs the donor's own
// airborne branch, falls, and breaks by the donor's own fall-height rule. On
// GC the window never opened: every create serialised through the DVD FIFO, so
// completion followed request order emergently. DN-10 step 1 has NO TARGET —
// the donor has no ordering SYSTEM to port; the ordering was a property of its
// disc. So this is step 2, and it borrows the RECEIVER's own idiom:
//
//   TP rooms pre-load their object banks from LBNK before actors create.
//   WW rooms carry no LBNK -> derive the same set from the room's OWN parsed
//   ACTR/TGOB records and kick the identical async residency load.
//
// SAME CALL THE NATIVE USES: dComIfG_setObjectRes(name, 0, NULL) — the exact
// residency start ensureDemoArcResident already uses for the §297 tale arc
// (d_ext_npc_mount.cpp:8357), which is itself this port's proven LBNK stand-in.
// Requesting early is the whole fix; nothing waits, nothing blocks, and a
// create that still phases behaves exactly as it does today.
//
// NAMES COME FROM THE DONOR'S OWN DATA: the ACTR record's 8-char name, mapped
// to its arc by the receiver's own dStage_searchName table (the same funnel
// dStage_actorCreate uses at :1851). A name the table does not know is SKIPPED
// LOUD (§858 gate term 2) — never a silent fall-through.
//
// GATE TERM 1 (§858): load-time is logged per room so a spike on a big room
// (sea,44) is visible before anyone ships it. GATE TERM 3: touches no actor.
// ============================================================================
// Walks the RAW DZR, not i_stage: a room's ACTR records are dispatched by the
// RELOAD table (d_stage.cpp roomReLoader, called later at d_s_room.cpp:379), so
// `getActor()` is not populated at seam time — reading it here would silently
// pre-resolve nothing. The file's own chunk table is authoritative and already
// parsed by offsetToPtr, which is why the other translators take `i_dzr` too.
// ⛔ REVERTED ONCE FOR AN ACCESS VIOLATION ON ROOM LOAD (Integrator, symbolicated
// to the record loop below). THE DEFECT WAS NOT A MISSING CAP — it was that
// ITERATION CAPS ARE NOT ADDRESS BOUNDS: `c < 128` and `r < 512` limit how many
// times the loop runs, while the ADDRESS it reads is `i_dzr + off + r*0x20`
// with `off`/`n` taken from the file. A large or garbage pair walks off the
// allocation on the FIRST iteration, and the caps never see it.
//
// So the function now takes the buffer LENGTH and bounds every read against it:
// the chunk table, each record, and the name copy. Without a length there is no
// bound to check — which is why the size parameter is the fix and not an extra
// guard on top of the old shape.
//
// AND THE SEQUENCE IS THE LESSON: this code had never executed before the night
// it shipped (its TU did not compile until a separate repair), so fixing a
// compile error is what let an unbounded walk reach the user. "It compiles" and
// "it has ever run" are different facts.
static void wwRoom_preResolveObjectArcs(const u8* i_dzr, u32 i_size, int i_roomNo) {
    // 4-byte count + at least one 12-byte chunk entry, or there is nothing to read.
    if (i_dzr == NULL || i_size < 16u) {
        return;
    }
    const u32 chunkN = wwRoom_be32(i_dzr);
    // Small dedupe: a room names the same arc dozens of times (pots, grass).
    // 64 distinct arcs is far past any WW room's real spread; overflow degrades
    // to "resolve it again", which is a wasted call and not a fault.
    char seen[64][12];
    int seenN = 0;
    int kicked = 0;
    int unknown = 0;
    int records = 0;
    // Placement chunks a WW ROOM carries, all at the donor's 0x20 actor stride
    // (History's vocabulary decode: ACTR/TGOB plus the twelve per-layer ACTn).
    // TGSC/SCOB/DOOR live at 0x24 and carry a scale tail — excluded here rather
    // than mis-strided; their arcs are a follow-up, not a guess.
    static const char* const kActorTags[] = {
        "ACTR", "TGOB", "ACT0", "ACT1", "ACT2", "ACT3", "ACT4", "ACT5",
        "ACT6", "ACT7", "ACT8", "ACT9", "ACTa", "ACTb",
    };
    for (u32 c = 0; c < chunkN && c < 128u; c++) {
        // BOUND THE CHUNK-TABLE READ ITSELF: chunkN comes from the file, so a
        // garbage count must not walk the table off the end.
        if (4u + (u64)c * 12u + 12u > (u64)i_size) {
            break;
        }
        const u8* ent = i_dzr + 4 + (size_t)c * 12;
        bool isActorTag = false;
        for (size_t t = 0; t < sizeof(kActorTags) / sizeof(kActorTags[0]); t++) {
            if (std::memcmp(ent, kActorTags[t], 4) == 0) { isActorTag = true; break; }
        }
        if (!isActorTag) {
            continue;
        }
        const u32 n = wwRoom_be32(ent + 4);
        const u32 off = wwRoom_be32(ent + 8);
        // THE WHOLE RECORD ARRAY MUST FIT, checked in 64-bit so the product
        // cannot wrap: a chunk whose (off, n) claims more than the buffer holds
        // is REFUSED ENTIRELY rather than partially walked — a partial walk is
        // how a plausible-looking prefix hides a bad tail.
        if ((u64)off + (u64)n * 0x20u > (u64)i_size) {
            DuskLog.warn("[WwRoomSeam] §858 pre-resolve: room {} chunk claims off={} n={} "
                         "(needs {} bytes) but the dzr is {} — chunk REFUSED, not walked.",
                         i_roomNo, off, n, (u64)off + (u64)n * 0x20u, i_size);
            continue;
        }
        for (u32 r = 0; r < n && r < 512u; r++) {
        const u8* rec = i_dzr + off + (size_t)r * 0x20;
        records++;
        char name[12];
        // The 8-byte name is inside the record, and the record is inside the
        // buffer by the check above — but assert the copy's own extent too,
        // because that is the read the fault address landed in.
        if ((u64)(rec - i_dzr) + 8u > (u64)i_size) {
            break;
        }
        std::memcpy(name, rec, 8);
        name[8] = '\0';
        for (int k = 7; k >= 0 && (name[k] == ' ' || name[k] == '\0'); k--) {
            name[k] = '\0';
        }
        if (name[0] == '\0') {
            continue;
        }
        bool dup = false;
        for (int s = 0; s < seenN; s++) {
            if (std::strncmp(seen[s], name, 11) == 0) { dup = true; break; }
        }
        if (dup) {
            continue;
        }
        if (seenN < 64) {
            std::strncpy(seen[seenN], name, 11);
            seen[seenN][11] = '\0';
            seenN++;
        }
        dStage_objectNameInf* inf = dStage_searchName(name);
        if (inf == NULL) {
            unknown++;
            DuskLog.warn("[WwRoomSeam] §858 pre-resolve: room {} names '{}' — NOT in "
                         "the object table, so no arc is pre-resolved for it (its "
                         "create will phase as before, or resolve to Nothing).",
                         i_roomNo, name);
            continue;
        }
        // The actor NAME is the arc name for the donor's own object arcs (the
        // §716 res/Object aliasing the mount already serves). setObjectRes is
        // idempotent-by-residency: an arc already mounted is a no-op.
        if (dComIfG_setObjectRes(name, 0, NULL)) {
            kicked++;
        }
        }
    }
    DuskLog.info("[WwRoomSeam] §858 object-arc pre-resolve: room {} — {} record(s), {} distinct "
                 "name(s), {} residency load(s) kicked, {} unknown. LBNK-shaped: requested "
                 "BEFORE actor creation so a phased create cannot open the §856 window.",
                 i_roomNo, records, seenN, kicked, unknown);
}

// ============================================================================
// PHASE 2, CHUNK 5: room MODELS — publishing what the node type prevented.
//
// A vanilla WW room arc files its models under RARC node type 'BDL '.
// dRes_info_c::setRes dispatches its per-resource fixup on that 4CC and has no
// 'BDL ' branch, so nothing parsed them and the slot still holds the RAW file.
// Reading that as a J3DModelData is the §619 crash — [modelData + 0xC8] was
// file bytes and +8 was the reported fault address, bit for bit.
//
// Adding a 'BDL ' branch would be mount-time BDL parsing, which DN-3 forbids.
// DN-3's own prescribed remedy is the consume-time cached resolver, so that is
// what runs here — and it parses from a PRISTINE BYTE COPY, leaving the donor
// arc buffer un-pointer-fixed. Under the zero-bake rule that matters twice
// over: the donor bytes stay identical on disk AND in memory.
//
// THEN IT PUBLISHES. The parsed model is written back into the archive's own
// res slot, so daBg — and every other consumer — reads a real J3DModelData
// from the ordinary getStageRes call it always made. The receiver learns
// nothing; no call site changes. That is the difference between this and the
// version that shipped as a branch inside daBg.
//
// Idempotent: after publishing, the slot no longer begins "J3D2".
// ============================================================================
// ============================================================================
// §678: WORLDIZE the published model's per-shape culling bounds.
//
// daBg_c::draw() culls each shape with viewMtx x the shape's LOCAL min/max —
// it never applies the MULT baseTRMtx that daBg's own create() set on the
// model. TP's own stitched rooms sit within a few thousand units of the
// origin, so testing the local box was invisibly wrong there; sea's rooms sit
// at +/-200k (stage.dzs MULT room 44: x=-200000 z=300000, measured off the
// disc), so the receiver was clipping PHANTOM ORIGIN BOXES 360k units from
// where the island renders. Whether the island drew was an accident of
// camera orientation — the long-standing "island disappears at angles/
// heights while collision holds" bug (§6): collision never flickered because
// BgW takes the MULT transform through its own registration.
//
// The fix transforms the PARSED model's shape bounds into world space once
// per mount cycle — derived runtime data; the donor bytes on disc are
// untouched (zero-bake). Conservative AABB over the four Y-rotated corners;
// Y itself is unchanged (MULT translates in XZ and rotates about Y only).
// Idempotent per mount: publish runs once per raw slot, and a re-mount
// re-parses fresh local bounds before this runs again.
// ============================================================================
static void wwRoom_worldizeShapeBounds(J3DModelData* i_model, int i_roomNo) {
    f32 tx;
    f32 tz;
    s16 angle;
    if (i_model == NULL || !dComIfGp_getMapTrans(i_roomNo, &tx, &tz, &angle)) {
        return;
    }
    const f32 s = cM_ssin(angle);
    const f32 c = cM_scos(angle);
    for (u16 j = 0; j < i_model->getShapeNum(); j++) {
        J3DShape* shape = i_model->getShapeNodePointer(j);
        Vec* mn = shape->getMin();
        Vec* mx = shape->getMax();
        const f32 xs[2] = {mn->x, mx->x};
        const f32 zs[2] = {mn->z, mx->z};
        f32 minX = 1e30f, minZ = 1e30f, maxX = -1e30f, maxZ = -1e30f;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                // mDoMtx Y-rotation convention: x' = c*x + s*z; z' = -s*x + c*z
                const f32 wx = c * xs[a] + s * zs[b] + tx;
                const f32 wz = -s * xs[a] + c * zs[b] + tz;
                if (wx < minX) minX = wx;
                if (wx > maxX) maxX = wx;
                if (wz < minZ) minZ = wz;
                if (wz > maxZ) maxZ = wz;
            }
        }
        mn->x = minX;
        mn->z = minZ;
        mx->x = maxX;
        mx->z = maxZ;
    }
    DuskLog.info("[WwRoomSeam] §678 room {} shape bounds worldized (MULT {} {} rot {})",
                 i_roomNo, tx, tz, angle);
}

static void wwRoom_publishModels(int i_roomNo) {
    static const char* const kModels[] = {
        "model.bdl",  "model1.bdl", "model2.bdl",
        "model3.bdl", "model4.bdl", "model5.bdl",
    };
    const char* arcName = dComIfG_getRoomArcName(i_roomNo);
    dRes_info_c* info = dComIfG_getStageResInfo(arcName);
    if (info == NULL) {
        return;
    }
    JKRArchive* archive = info->getArchive();
    if (archive == NULL) {
        return;
    }
    for (int i = 0; i < (int)(sizeof(kModels) / sizeof(kModels[0])); i++) {
        void* res = dComIfG_getStageRes(arcName, kModels[i]);
        // A parsed J3DModelData never begins "J3D2"; compared as bytes so
        // endianness is not a question.
        if (res == NULL || std::memcmp(res, "J3D2", 4) != 0) {
            continue;
        }
        J3DModelData* parsed = dExtNpcMount_acquireStageModelData(arcName, kModels[i]);
        if (parsed == NULL) {
            DuskLog.warn("[WwRoomSeam] model '{}/{}' raw and UNRESOLVED — the "
                         "consumer will still see a file buffer",
                         arcName, kModels[i]);
            continue;
        }
        JKRArchive::SDIFileEntry* entry = archive->findNameResource(kModels[i]);
        if (entry == NULL) {
            continue;
        }
        wwRoom_worldizeShapeBounds(parsed, i_roomNo);  // §678 — see above
        info->setRes((s32)entry->file_id, parsed);
        DuskLog.info("[WwRoomSeam] model '{}/{}' published parsed into the res "
                     "slot (node 'BDL ' has no receiver fixup branch)",
                     arcName, kModels[i]);
    }
}
// ============================================================================
// PHASE 2, CHUNK 6: ARC FILENAMES — vanilla names on disk.
//
// Wind Waker ships /res/Stage/sea/Room44.arc and Stage.arc; this engine asks
// for R44_00 and Stg_00. The DIRECTORY already matches for free — the caller
// builds "/res/Stage/<stage>/" from the stage name, so a vanilla stage id lands
// on the vanilla folder unaided. Only the two leaf names differ.
//
// Under zero-bake the asset is never renamed, so the LOOKUP moves. This shipped
// once as a branch inside dRes_info_c::set — a WW conditional in a shared
// engine path, exactly the surface the shared-path-scoping rule names. It now
// lives here and reaches the resource layer through a WW-AGNOSTIC hook.
//
// Only the FILENAME is aliased. The receiver's key (mArchiveName) is untouched,
// because every getStageRes(arcName, ...) call in the engine is written against
// it — that string is an identifier, not a file.
//
// Vanilla convention, confirmed against the disc: Room%d with NO zero pad
// (Room0, Room4, Room44) and Stage. Ojhous2 ships Room0/Room1/Stage.
// ============================================================================
static const char* wwRoom_aliasArcFileName(const char* i_arcName) {
    static char s_name[16];
    const char* stage = dComIfGp_getStartStageName();
    // §637: DECLARED stages only. The neutral R_DL*/F_DL* fork stages are
    // repackaged under the receiver's own filenames, so aliasing them asks for
    // a file that is not there and the stage mounts nothing.
    if (i_arcName == NULL || stage == NULL || !dExtWwSave_isDeclaredWwStage(stage)) {
        return NULL;  // no alias — the receiver's own name stands
    }
    if (i_arcName[0] == 'R' && i_arcName[1] >= '0' && i_arcName[1] <= '9' &&
        i_arcName[2] >= '0' && i_arcName[2] <= '9' && std::strcmp(i_arcName + 3, "_00") == 0)
    {
        const int roomNo = (i_arcName[1] - '0') * 10 + (i_arcName[2] - '0');
        std::snprintf(s_name, sizeof(s_name), "Room%d", roomNo);
        return s_name;
    }
    if (std::strcmp(i_arcName, "Stg_00") == 0) {
        return "Stage";
    }
    return NULL;
}

// ============================================================================
// PHASE 2, CHUNK 7: PLYR PARAMETERS — the arrival the receiver could not read.
//
// daAlink_c::getStartMode() reads PLYR parameter bits 12-16, and the arrival
// sequence — DEMO_UNK_14_e, setMoveAngle(current.angle.y), setTimer(35), i.e.
// the walk-in — runs for start_mode 0/1/2/3/5/13/14 and nothing else.
//
// A census of the receiver's OWN 1277 shipped PLYR entries finds 0-14 and
// nothing above 14, with 0 alone accounting for 782. WW's word puts values well
// outside that range there — R_DL02 gives 27 and 16, and Outset's 24 spawns
// give seven distinct values (16, 17, 18, 21, 25, 26, 29). So the branch never
// runs and the player is left standing exactly where the animation was supposed
// to START, which for a door arrival is the threshold.
//
// WHY IT MUST FIRE HERE AND NOT AFTER THE ROOM LOAD. playerInit runs INSIDE the
// delegated loader and queues the actor create; a post-delegation translator is
// already too late. That is why this is a hook in the receiver rather than
// another entry in the chunk list above — the timing is the requirement.
//
// §636 — DECODED. The field is not foreign, it is one bit WIDER on the receiver.
//
//   donor     daPy_lk_c::getStartModeFromParam(u32 p) { return (p >> 0xC) & 0xF; }
//             D:/XXXXXXX/WW DP/include/d/actor/d_a_player_main.h:1880, and
//             confirmed against zeldaret/tww main — identical, 4 bits.
//   receiver  daAlink_c::getStartMode()  { return (param >> 0xC) & 0x1F; }
//                                                                   ^^^^ 5 bits
//
// The receiver reads ONE EXTRA BIT — bit 16 — which on the donor belongs to a
// different field of the same word. Every specimen has it set, so every donor
// start mode arrived exactly 16 too large:
//
//   param        receiver &0x1F   donor &0xF   bit16   delta
//   0x00ffb000        27              11         1      16
//   0x01ff0001        16               0         1      16
//   0xffffa02c        26              10         1      16
//   0xffff102c        17               1         1      16
//   0xffff202c        18               2         1      16
//   0xffff502c        21               5         1      16
//   0xffff902c        25               9         1      16
//   0xffffd02c        29              13         1      16
//   (11 specimens across R_DL02 and Outset; delta 16 in every one)
//
// Masked to the donor's four bits the values are 0, 1, 2, 5, 9, 10, 11, 13 —
// ALL INSIDE the 0-14 range the receiver's own 1277 shipped PLYR entries use.
// That is the corroboration: a wrong mask produces out-of-range noise, and the
// donor's mask produces values the receiver already speaks.
//
// So this clears bit 16 and nothing else. It is no longer a stand-in — it is
// the donor's own decode, and it supersedes the earlier blanket "-> 0", which
// would have flattened all 24 of Outset's spawns into the same walk-in.
//
// STILL OPEN, and NOT assumed: whether donor mode N means what receiver mode N
// means. The FIELD is decoded; the VOCABULARY is not. Neither decomp defines a
// named enum for it (checked locally and against zeldaret/tww main), so the
// semantics have to come from behaviour. 0/1/2/5/13 land on the receiver's
// arrival branch; 9/10/11 do not, and those are the ones to watch.
// ============================================================================
static void wwRoom_translatePlyrParam(u32* io_parameters) {
    const char* stage = dComIfGp_getStartStageName();
    if (io_parameters == NULL || stage == NULL || !dExtWwSave_isWwHostStage(stage)) {
        return;
    }
    const u32 asReceiverReads = (*io_parameters >> 12) & 0x1F;
    const u32 donorStartMode = (*io_parameters >> 12) & 0xF;
    if (asReceiverReads == donorStartMode) {
        return;  // bit 16 clear — the receiver already reads the donor's value
    }
    *io_parameters &= ~(1u << 16);
    DuskLog.info("[WwRoomSeam] §636 PLYR start_mode: receiver read {} (5-bit "
                 "mask), donor means {} (4-bit) — bit 16 cleared. stage='{}'",
                 asReceiverReads, donorStartMode, stage);
}
#endif  // DUSK_WW_ROOM_SEAM && DUSK_WW_ROOM_CHUNKS

// ============================================================================
// §635: install the two hooks the room path needs but the seam cannot reach.
// Called once from the WW layer's mod scan. Both hooks are WW-AGNOSTIC on the
// receiver side and NULL until this runs, so mainline TP is untouched.
// ============================================================================
// ============================================================================
// §644: WIND WAKER'S MESSAGE SYSTEM, ported rather than translated.
//
// WHAT THE DONOR DOES (d_s_logo.cpp:882-889, d_s_menu.cpp:44):
//
//     onMemMount("/res/Msg/bmgres.arc")      ONE archive, the whole game
//     onMemMount("/res/Msg/bmgresh.arc")
//
// There is NO per-stage message group in Wind Waker. The receiver splits its
// messages into ten archives and picks one from STAG offset 0x28 — a field the
// donor's 0x20-byte STAG record does not contain, because the CONCEPT does not
// exist there. A translator would have to invent the value; the donor's own
// system needs none.
//
// IT IS A DROP-IN, which is the part worth checking rather than assuming:
//
//     WW  /res/Msg/bmgres.arc   ->  zel_00.bmg, color.bmc
//     receiver asks for             bmg_filename[0] == "zel_00.bmg"
//
// The member names already match, so group 0 resolves against the donor archive
// with nothing renamed and nothing repacked — the same zero-bake result the arc
// filename alias gets for stages.
//
// SCOPE: declared donor stages only (§637). A neutral R_DL*/F_DL* fork stage
// keeps the receiver's own message selection, because its content is TP's.
// ============================================================================
static bool wwRoom_resolveMsgArchive(char* o_path, int i_pathSize, int* io_group) {
    const char* stage = dComIfGp_getStartStageName();
    if (o_path == NULL || io_group == NULL || stage == NULL ||
        !dExtWwSave_isDeclaredWwStage(stage))
    {
        return false;
    }
    std::snprintf(o_path, (size_t)i_pathSize, "/res/Msg/bmgres.arc");
    // Not "group 0" as a safe default — the donor has ONE message set, and 0 is
    // the index whose filename (zel_00.bmg) is the one that archive contains.
    *io_group = 0;
    DuskLog.info("[WwRoomSeam] §644 donor message system: one archive '{}', no "
                 "per-stage group (stage='{}')",
                 o_path, stage);
    return true;
}

void dExtWwRoom_installHooks(void) {
#if DUSK_WW_ROOM_SEAM && DUSK_WW_ROOM_CHUNKS
    dRes_setArcFileNameHook(&wwRoom_aliasArcFileName);
    dStage_setPlyrParamHook(&wwRoom_translatePlyrParam);
    dMsg_setGroupArchiveHook(&wwRoom_resolveMsgArchive);
    DuskLog.info("[WwRoomSeam] §635 hooks installed: arc-filename alias, PLYR "
                 "parameter translation, donor message archive");
#endif
    // §669: the crawl algorithm installs from INSIDE the layer, not from the
    // receiver. d_ext_npc_mount.cpp is a receiver TU, so naming the camera
    // installer there would make it a leg; routing it through the boundary
    // symbol the receiver already names keeps the layer at one entry point.
    dExtWwCam_installCrawl();
    // Donor camera style/type tables: resident + self-tested.
    dExtWwCam_installData();
    // The donor SELECTOR — drives the tables above on WW host stages through
    // the receiver's NULL-default selection extension point (FULL ruling).
    dExtWwCam_installSelect();
}

void dExtWwRoom_loadRoomDzr(void* i_data, dStage_dt_c* i_stage, int i_roomNo) {
#if DUSK_WW_ROOM_SEAM
    const char* stage = dComIfGp_getStartStageName();
    if (stage != NULL && dExtWwSave_isWwHostStage(stage)) {
        // ====================================================================
        // WW HOST PATH — Phase 2: delegate, then re-read audited chunks at
        // DONOR stride. The receiver's loader parses everything first (the
        // 19 chunks whose layouts AGREE stay exactly its); each translator
        // below then replaces its own chunk's storage with receiver-layout
        // records read by the donor's semantics. One translator per chunk,
        // one call site each — independently revertible, step 19's grain.
        // ====================================================================
        DuskLog.info("[WwRoomSeam] 3b: stage='{}' room={} (delegate + ported chunks)",
                     stage, i_roomNo);
        // ====================================================================
        // §772 ARC-IDENTITY CONTROL (Foundry, routed): the LinkRM/Ojhous
        // aliasing was invisible because the seam printed the ARC but not the
        // STAGE at publish time — R00_00 is not unique across stages, and a
        // session cache keyed on the bare name served one stage's model over
        // another's collision. Four file-layer measurements read clean while
        // the render layer was wrong. This map names the aliasing the INSTANT
        // it happens: same arc name, different stage than last time -> WARN,
        // with both stages in the line. It asserts nothing about the cache —
        // it makes the condition the cache must survive VISIBLE.
        // ====================================================================
        {
            struct ArcStage { char arc[16]; char stg[16]; };
            static ArcStage s_seen[64];
            static int s_seenN = 0;
            const char* arcName = dComIfG_getRoomArcName(i_roomNo);
            int found = -1;
            for (int i = 0; i < s_seenN; i++) {
                if (strncmp(s_seen[i].arc, arcName, 15) == 0) { found = i; break; }
            }
            if (found >= 0) {
                if (strncmp(s_seen[found].stg, stage, 15) != 0) {
                    DuskLog.warn("[WwRoomSeam] §772 ARC-IDENTITY: '{}' previously "
                                 "served for stage='{}', now stage='{}' — any "
                                 "session cache keyed on the BARE arc name is now "
                                 "serving the WRONG stage's data.",
                                 arcName, s_seen[found].stg, stage);
                    strncpy(s_seen[found].stg, stage, 15);
                }
            } else if (s_seenN < 64) {
                strncpy(s_seen[s_seenN].arc, arcName, 15);
                strncpy(s_seen[s_seenN].stg, stage, 15);
                s_seen[s_seenN].arc[15] = s_seen[s_seenN].stg[15] = 0;
                ++s_seenN;
            }
        }
        dStage_dt_c_roomLoader(i_data, i_stage, i_roomNo);
#if DUSK_WW_ROOM_CHUNKS
        wwRoom_translateScls(i_stage, i_roomNo);   // chunk 1 — SCLS (§604 stride hit)
        wwRoom_translateFili(i_stage, i_roomNo);   // chunk 2 — FILI (§607 semantic hit)
        wwRoom_translateRcam(i_stage, i_roomNo,    // chunk 3 — RCAM (adjacency-
            (const u8*)i_data);                    // first, name-test fallback §749)
        wwRoom_repackDzb(i_roomNo);                // chunk 4 — dzb attrs (§334 2nd consumer)
        wwRoom_publishModels(i_roomNo);            // chunk 5 — 'BDL ' models via DN-3 resolver
        // ====================================================================
        // CHUNK 6: crashed the fork on 2026-08-17 (I shipped it), reverted, then
        // RE-ENABLED WITH THE LENGTH THE REVERT REQUIRED. The size comes from
        // the receiver's OWN accessor (the room archive's getResSize) rather
        // than from anything this file computes — if it cannot be obtained the
        // pass is SKIPPED, because an unbounded walk is exactly what was
        // reverted. Chunk 6 is a pre-warm optimisation, so skipping costs
        // nothing but the optimisation.
        {
            const char* preArc = dComIfG_getRoomArcName(i_roomNo);
            dRes_info_c* preInfo = (preArc != NULL)
                ? g_dComIfG_gameInfo.mResControl.getStageResInfo(preArc) : NULL;
            JKRArchive* preArch = (preInfo != NULL) ? preInfo->getArchive() : NULL;
            u32 preSize = 0;
            if (preArch != NULL && i_data != NULL) {
                preSize = preArch->getResSize(i_data);
            }
            if (preSize > 0u) {
                wwRoom_preResolveObjectArcs((const u8*)i_data, preSize, i_roomNo);
            } else {
                DuskLog.warn("[WwRoomSeam] §858 pre-resolve SKIPPED for room {}: the dzr's "
                             "size is unavailable (arc='{}'), and this pass will not walk "
                             "an unbounded buffer — that is the fault that reverted it.",
                             i_roomNo, preArc != NULL ? preArc : "?");
            }
        }
        // ====================================================================
        // HISTORY — RESOLVED. Kept because the sequence is the lesson.
        // ====================================================================
        // Symbolicated: wwRoom_preResolveObjectArcs <- dExtWwRoom_loadRoomDzr
        // <- d_s_room phase_2, EXCEPTION_ACCESS_VIOLATION reading a record
        // pointer. THE DEFECT: the record loop computed `i_dzr + off + r*0x20`
        // from chunk-table values with NO BOUNDS CHECK, and the function never
        // received the buffer LENGTH, so a garbage `off`/`n` walked straight
        // off the allocation. Chunks 1-5 predate it and were unaffected.
        // THE SEQUENCE: this code had never executed before the night it
        // shipped, because its TU did not compile until a separate repair.
        // "It compiles" and "it has ever run" are different facts, and fixing
        // a compile error is what let an unbounded walk reach the user.
        // NOW FIXED ABOVE and verified by History/Bridge: four bounds, every
        // comparison u64-widened FIRST (a u32 `off + n*0x20` WRAPS, and a
        // wrapped sum passes a check it should fail), size taken from the
        // archive's own authoritative record, and an unavailable size SKIPS
        // the pass rather than falling back to iteration caps — the failure
        // mode is "the optimisation does not run", never "the walk runs
        // blind". The dead re-enable stub that used to sit here is gone; the
        // live bounded call is the one above.
        // ====================================================================
#endif
        return;
    }
#endif
    // ========================================================================
    // Mainline TP (and kill-switch-off): the receiver's own loader, untouched.
    // ========================================================================
    dStage_dt_c_roomLoader(i_data, i_stage, i_roomNo);
}
