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

#include <cstring>

#include "d/d_ext_save_guard.h"  // dExtWwSave_isWwHostStage
#include "d/d_com_inf_game.h"    // dComIfGp_getStartStageName
#include "d/d_stage.h"           // dStage_dt_c_roomLoader
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
#endif  // DUSK_WW_ROOM_SEAM && DUSK_WW_ROOM_CHUNKS

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
        dStage_dt_c_roomLoader(i_data, i_stage, i_roomNo);
#if DUSK_WW_ROOM_CHUNKS
        wwRoom_translateScls(i_stage, i_roomNo);   // chunk 1 — SCLS (§604 stride hit)
        wwRoom_translateFili(i_stage, i_roomNo);   // chunk 2 — FILI (§607 semantic hit)
#endif
        return;
    }
#endif
    // ========================================================================
    // Mainline TP (and kill-switch-off): the receiver's own loader, untouched.
    // ========================================================================
    dStage_dt_c_roomLoader(i_data, i_stage, i_roomNo);
}
