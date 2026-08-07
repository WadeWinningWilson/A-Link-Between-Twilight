// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_event_data.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// ============================================================
// §423 A2 — WW EVENT DATA (semantic layer), donor-verbatim from
// WW DP src/d/d_event_data.cpp (dtk address markers kept).
//
// WHAT THIS TU IS. The parallel WW event stack's behavior half. It carries the
// donor's staff/cut/flag semantics — the exact surface where audit №284's fork
// findings live (§319's PACKAGE/PLAY teardown, §320's cutEnd gate, §318's cut
// advance) — so that TP's own d_event_data.cpp can go back to byte-vanilla at
// A5 instead of hosting WW conditionals forever.
//
// WHAT IT DELIBERATELY DOES *NOT* PORT: the FORMAT layer. §379a proved donor
// event data parses cleanly with the receiver's own structs (that is how WW's
// real door events were merged into the host stages), so dEvDtStaff_c /
// dEvDtCut_c / dEvDtEvent_c / dEvDtBase_c are SHARED, not duplicated. Porting
// them twice would fork a format that does not diverge and would guarantee the
// two copies drift. The campaign's premise, stated once more: the fork is
// SEMANTICS, never format.
//
// PORT SEAMS (each labeled at its site):
//   [E1] donor methods become JEvent1 free functions taking the receiver's
//        struct pointer — the port cannot add methods to a shared TP class
//        without touching it, which is exactly what A5 exists to undo. Same
//        adaptation JA1 made when JASystem:: flattened into JAudio1::.
//   [E2] donor `mWipeDirection` is a per-staff PACKAGE latch with no
//        unambiguous receiver field (TP's staff carries a differently-shaped
//        work buffer; §319 already flagged "the ambiguous port field_0x40").
//        JEvent1 keeps its own small per-staff latch table rather than
//        reinterpreting TP's bytes — faithful in behavior, safe in memory.
//   [E3] dEvDt_Next_Stage stays SHARED (the receiver's). It resolves a stage
//        name/spawn from staff properties — data mapping, not event semantics —
//        and the receiver's copy already carries the host-stage aliasing WW
//        content needs (§322). If A4 shows it diverging, it forks then.
//   [E4] donor JUT_ASSERTs are retail no-ops; the port logs instead of
//        panicking (the established WW-port rule).
//
// A2 SCOPE: the tale-critical spine — PACKAGE staff, wait procs, cut advance,
// flag/finish checks, startCheck. The remaining donor staff procs (LIGHT,
// MESSAGE, SOUND, CREATE, DIRECTOR, TIMEKEEPER) are A2b; nothing dispatches to
// this TU until A4, so partial coverage is inert rather than risky.
// ============================================================

#include "d/ext_evt/evt1_boundary.h"

#include "d/d_com_inf_game.h"
#include "d/d_demo.h"
#include "d/d_event_data.h"
#include "d/d_stage.h"
#include "dusk/logging.h"

// [E3] the receiver's stage resolver (data mapping, not event semantics) —
// exposed non-static for the parallel stack; see d_event_data.h.
int dEvDt_Next_Stage(int i_staffId, int i_wipe);

// A2 reconciliation: TP's dEvDtFlagCheck/Set are TU-local inlines over the
// SHARED save-backed flag store. Mirrored (not re-pointed) so the WW stack
// writes the SAME flags — §423 risk-ledger item (ii): a separate flag store
// would desync tale progress from the save.
static inline BOOL evt1_flagCheck(int i_flag) {
    return dComIfGp_getEventManager().getFlags().flagCheck(i_flag);
}
static inline void evt1_flagSet(int i_flag) {
    dComIfGp_getEventManager().getFlags().flagSet(i_flag);
}

namespace JEvent1 {

// [E8] manager-side scratch reset (evt1_event_manager.cpp).
void evt1_onStaffAdvance(int staffIdx);

// ============================================================
// [E2] PER-STAFF RUNTIME SCRATCH — and the layout receipt that decided it.
// Donor vs receiver dEvDtStaff_c (both 0x50 records, verified field by field):
//   0x20 mTagID | 0x24 staffIdx/mIndex | 0x28 m28/mFlagID | 0x2C type |
//   0x30 firstCut/mStartCut | 0x38 mCurCutIdx/mCurrentCut | 0x3C curAction
//        ^ the FILE-MEANINGFUL fields agree at every offset (why §379a's merge
//          works and why the format layer is genuinely shared), BUT:
//   0x40 donor s16 mWipeDirection  vs  receiver bool field_0x40 + bool 0x41
//   0x42 donor s16 mTimer          vs  receiver u8 mData[0x42..0x50]
//   0x46 donor u8  mAdvance        vs  receiver mData[4]
// SAME REGION, INCOMPATIBLE TYPING — it is per-game RUNTIME scratch, not file
// data (which is exactly why §319 called field_0x40 "ambiguous"; it is
// mWipeDirection's big-endian high half). CAMPAIGN PREMISE REFINED: "the format
// is shared" holds for file-meaningful fields and NOT for runtime scratch.
// JEvent1 therefore owns its scratch rather than aliasing TP's bytes — safe
// because one event runs at a time, and it keeps A5's revert clean.
// ============================================================
static u8 s_wipeDirection[64];
static s16 s_timer[64];
static u8 s_advance[64];

void evt1_resetStaffLatches() {
    for (int i = 0; i < 64; i++) {
        s_wipeDirection[i] = 0;
        s_timer[i] = 0;
        s_advance[i] = 0;
    }
}

static bool staffInRange(int i) { return i >= 0 && i < 64; }

static u8& wipeDirection(int staffIdx) {
    static u8 dummy = 0;
    if (!staffInRange(staffIdx)) { dummy = 0; return dummy; }
    return s_wipeDirection[staffIdx];
}

static s16& staffTimer(int staffIdx) {
    static s16 dummy = 0;
    if (!staffInRange(staffIdx)) { dummy = 0; return dummy; }
    return s_timer[staffIdx];
}

u8& evt1_staffAdvance(int staffIdx) {
    static u8 dummy = 0;
    if (!staffInRange(staffIdx)) { dummy = 0; return dummy; }
    return s_advance[staffIdx];
}

/* 80071AC4-80071B4C       .text finish_check__12dEvDtEvent_cFv */
// [E1] donor dEvDtEvent_c::finish_check
// A2 reconciliation: the donor body walks mFlagCheckFinish[3] with exactly the
// receiver's own semantics, and the receiver already exposes finishCheck() over
// the SAME file-meaningful field. Per [E1] (never duplicate a format-level
// helper that agrees) this delegates instead of re-porting.
BOOL evt1_finishCheck(dEvDtEvent_c* i_event) {
    return i_event->finishCheck() != 0;
}

/* 80071BB4-80071C18       .text specialProc_WaitStart__12dEvDtStaff_cFi */
void evt1_waitStart(dEvDtStaff_c* i_staff, int i_staffIdx) {
    int* pTimer = dComIfGp_evmng_getMyIntegerP(i_staffIdx, "Timer");
    staffTimer(i_staffIdx) = (s16)(pTimer != NULL ? *pTimer : 0);   // [E2] JEvent1 scratch
    wipeDirection(i_staffIdx) = 0;   // [E2] donor init-time zero
}

/* 80071C18-80071C5C       .text specialProc_WaitProc__12dEvDtStaff_cFi */
void evt1_waitProc(dEvDtStaff_c* i_staff, int i_staffIdx) {
    if (staffTimer(i_staffIdx) > 0) {
        staffTimer(i_staffIdx)--;
    } else {
        dComIfGp_evmng_cutEnd(i_staffIdx);
    }
}

/* 8007317C-800733CC       .text specialProcPackage__12dEvDtStaff_cFv */
// THE fork site. Audit №284 finding #2/#3: §319 injected THIS body into TP's
// own specialProcPackage, deleting TP's re-entrance branch and calling
// dEvDt_Next_Stage from PLAY (which vanilla never does). Here it lives in the
// WW stack where it belongs, and TP's copy can be vanilla again.
void evt1_specialProcPackage(dEvDtStaff_c* i_staff) {
    const int staffIdx = dComIfGp_evmng_getMyStaffId("PACKAGE", NULL, 0);
    if (staffIdx == -1) {
        DuskLog.warn("[Evt1] §423 PACKAGE staff missing");   // [E4]
        return;
    }

    static DUSK_CONSTEXPR char DUSK_CONST* action_table[] = {
        "WAIT",
        "PLAY",
        "PLAY2",
    };
    enum { ACT_WAIT, ACT_PLAY, ACT_PLAY2 };

    const int actIdx = dComIfGp_evmng_getMyActIdx(
        staffIdx, (DUSK_CONST char* DUSK_CONST*)action_table, 3, FALSE, 0);

    if (dComIfGp_evmng_getIsAddvance(staffIdx)) {
        switch (actIdx) {
        case ACT_WAIT:
            evt1_waitStart(i_staff, staffIdx);
            break;
        case ACT_PLAY:
        case ACT_PLAY2: {
            const char* filename = dComIfGp_evmng_getMyStringP(staffIdx, "FileName");
            cXyz* pOffsetPos = dComIfGp_evmng_getMyXyzP(staffIdx, "OffsetPos");
            f32* pOffsetAngY = dComIfGp_evmng_getMyFloatP(staffIdx, "OffsetAngY");
            const f32 offsetAngY = pOffsetAngY != NULL ? *pOffsetAngY : 0.0f;

            void* demo_data = NULL;
            const char* demo_arc = dStage_roomControl_c::getDemoArcName();
            if (demo_arc != NULL && demo_arc[0] != '\0') {
                demo_data = dComIfG_getObjectRes(demo_arc, filename);
            }
            if (demo_data == NULL) {
                demo_data = dComIfG_getStageRes("Stage", filename);
            }
            if (demo_data == NULL) {
                DuskLog.warn("[Evt1] §423 PACKAGE demo data '{}' not found",   // [E4]
                             filename != NULL ? filename : "(null)");
                break;
            }
            dDemo_c::start((const u8*)demo_data, pOffsetPos, offsetAngY);
            dComIfGp_event_setCullRate(10.0f);
            int* eventFlag = dComIfGp_evmng_getMyIntegerP(staffIdx, "EventFlag");
            if (eventFlag != NULL) {
                dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[*eventFlag]);
            }
            break;
        }
        }
    }

    switch (actIdx) {
    case ACT_WAIT:
        evt1_waitProc(i_staff, staffIdx);
        break;
    case ACT_PLAY:
        // Donor tail VERBATIM (WW d_event_data.cpp:812-824). The mWipeDirection
        // latch is what stops the per-frame re-fire that the receiver-side §306
        // had to emulate with !isEnableNextStage(); here it is the donor's own.
        if (wipeDirection(staffIdx) == 0) {
            if (dDemo_c::getMode() == 2) {
                if (dEvDt_Next_Stage(staffIdx, 5)) {   // [E3]
                    wipeDirection(staffIdx) = 1;
                } else {
                    dDemo_c::remove();
                }
            }
            if (dDemo_c::getMode() == 0) {
                dComIfGp_evmng_cutEnd(staffIdx);
            }
        }
        break;
    default:
        DuskLog.warn("[Evt1] §423 PACKAGE unknown act {}", actIdx);   // [E4]
        dComIfGp_evmng_cutEnd(staffIdx);
        break;
    }
}

/* 8007363C-8007364C       .text startCheck__10dEvDtCut_cFv */
s32 evt1_cutStartCheck(dEvDtCut_c* i_cut) {
    for (s32 i = 0; i < 3; i++) {   // donor ARRAY_SIZE(mStartFlag)
        const int flag_id = i_cut->getStartFlag(i);
        if (flag_id == -1) {
            return i == 0 ? -1 : 1;
        }
        if (!evt1_flagCheck(flag_id)) {
            return 0;
        }
    }
    return 1;
}

// ============================================================
// [E8] donor dEvDtStaff_c::advanceCut (:186) sets, beyond the shared cut index:
//   mTimer = 0 | mWipeDirection = 0 | mAdvance = 1 | mbHasAction = false
// The receiver's advanceCut writes only TP's own fields, so the WW stack must
// apply these itself — the omission is what hung A4.
// s_seen additionally seeds the FIRST sight of a staff (event start, which
// runs through TP's init and never reaches this path) with mAdvance = 1, so
// the staff's start branch fires exactly as the donor's does.
// ============================================================
static u8 s_seen[64];

static void evt1_onAdvance(dEvDtStaff_c* i_staff) {
    const int idx = (int)i_staff->mIndex;
    if (idx < 0 || idx >= 64) {
        return;
    }
    s_timer[idx] = 0;
    s_wipeDirection[idx] = 0;
    s_advance[idx] = 1;
    s_seen[idx] = 1;
    evt1_onStaffAdvance(idx);   // clears the manager-side action cache
}

/* 800736E4-800737DC       .text advanceCutLocal__11dEvDtBase_cFP12dEvDtStaff_c */
// Donor cut advance. Note what it does NOT have: the receiver's added
// per-staff bookkeeping. A cut advances when its successor's start flags are
// satisfied; the -1 case (successor has no start flags) additionally requires
// THIS cut's own flag to be set, and the 1 case sets it.
bool evt1_advanceCutLocal(dEvDtBase_c* i_base, dEvDtStaff_c* i_staff) {
    // [E8] first sight of this staff in the WW stack = event start; the donor
    // reaches the same state through its own init (mAdvance = 1).
    {
        const int idx0 = (int)i_staff->mIndex;
        if (idx0 >= 0 && idx0 < 64 && !s_seen[idx0]) {
            s_seen[idx0] = 1;
            s_advance[idx0] = 1;
        }
    }
    dEvDtCut_c* cutP = i_base->getCutP(i_staff->getCurrentCut());

    if (cutP->getNext() != -1) {
        dEvDtCut_c* nextP = i_base->getCutP(cutP->getNext());
        switch (evt1_cutStartCheck(nextP)) {
        case -1:
            if (evt1_flagCheck(cutP->getFlagId())) {
                i_staff->advanceCut(cutP->getNext());
                evt1_onAdvance(i_staff);   // [E8] donor advanceCut's scratch half
                return true;
            }
            break;
        case 1:
            evt1_flagSet(cutP->getFlagId());
            i_staff->advanceCut(cutP->getNext());
            evt1_onAdvance(i_staff);       // [E8]
            return true;
        }
    }

    // Donor advance-counter wind-down (dEvDtStaff_c::mAdvance).
    // [E2] donor staff->mAdvance wind-down, in JEvent1 scratch.
    u8& adv = evt1_staffAdvance((int)i_staff->mIndex);   // receiver field (no getter)
    adv = adv > 1 ? 1 : 0;
    return false;
}

/* 80073674-800736E4       .text advanceCut__11dEvDtBase_cFP12dEvDtEvent_c */
void evt1_advanceCut(dEvDtBase_c* i_base, dEvDtEvent_c* i_event) {
    for (s32 i = 0; i < i_event->getNStaff(); i++) {
        evt1_advanceCutLocal(i_base, i_base->getStaffP(i_event->getStaff(i)));
    }
}

// ============================================================
// §423 A2b — THE STAFF DISPATCHER, and why it is this short.
//
// A2b was scoped to "port the remaining donor staff procs (LIGHT, MESSAGE,
// SOUND, CREATE, DIRECTOR, TIMEKEEPER)" — ~605 donor lines. A census of the
// staff records in the ACTUAL merged WW event data (F_DL01 + R_DL01
// event_list.dat, the §379a/§423 stages) closed it instead:
//
//     CAMERA   type  2  x12      Link/Ba1  type 0 (DEFAULT)  x14
//     ALL      type  1  x5       PACKAGE   type 11           x4
//     SHUTTER_ type 10  x4
//
// NOT ONE WW event uses LIGHT(9), MESSAGE(7), SOUND(8), CREATE(12),
// DIRECTOR(6) or TIMEKEEPER(4). Porting them would be 605 lines of
// speculative reconstruction against subsystems (TP's message/sound/create)
// that the donor code does not even call. Of the types we DO use, only
// PACKAGE and ALL are staff-proc driven at all: CAMERA is consumed by the
// STB/demo system, DEFAULT staffs are read by the actors themselves (Link,
// ba1), and SHUTTER_ is driven by the ported knob00's own demoProc (§329).
// A2 therefore already covers our content completely.
//
// The default leg DELEGATES to the receiver's staff proc, so if WW content
// ever introduces one of the six, behavior stays exactly as today rather than
// dropping to nothing — and the census script makes that arrival detectable.
// Port on evidence, not on inventory. [E7] law, applied a second time.
// ============================================================
void evt1_specialProc(dEvDtStaff_c* i_staff) {
    switch (i_staff->getType()) {
    case dEvDtStaff_c::TYPE_PACKAGE:
        evt1_specialProcPackage(i_staff);
        break;
    case dEvDtStaff_c::TYPE_ALL:
        dComIfGp_evmng_cutEnd((int)i_staff->mIndex);   // donor: ALL_e ⇒ cutEnd
        break;
    default:
        // Unused-by-our-content staff types: the receiver's own proc runs.
        i_staff->specialProc();
        break;
    }
}

}  // namespace JEvent1
