// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_event_manager.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// ============================================================
// §423 A3 — WW EVENT MANAGER (semantic layer), donor-verbatim from
// WW DP src/d/d_event_manager.cpp (dtk address markers kept).
//
// THIS TU IS WHERE AUDIT №284's SHARPEST FINDING GOES HOME. §295 changed TP's
// getMyActIdx to return -1 on a no-match — globally — because ba1's custom cut
// needed WW's contract. TP's contract is `return 0` (unknown cut ⇒ index 0 =
// WAIT) and ~133 files index the result through an unchecked member-function
// array in release, so -1 was a wild jump on any vanilla cutscene whose cut
// name wasn't in that actor's list. Here the donor's -1 is simply WHAT THE WW
// MANAGER DOES, natively, and TP's function keeps its own contract untouched.
// At A5 the §295 gate in d_event_manager.cpp deletes with nothing lost.
//
// Same for cutEnd: the donor's is FOUR LINES with NO event-mode gate (:549) —
// which is the whole content of §320's `if (false && …)` global disable. The
// gate belongs to TP and is already restored (№285); the ungated form belongs
// here.
//
// SEAMS ([E1]-[E4] as A2; new ones numbered onward):
//   [E5] donor `staff->mAdvance` and `staff->mbHasAction` are RUNTIME SCRATCH,
//        which A2 proved TP places elsewhere in the same 0x50 record (donor
//        mAdvance @0x46 / mbHasAction @0x47 vs TP's mData[]). The WW manager
//        therefore reads JEvent1's scratch, not TP's bytes — the direct
//        consequence of A2's refined premise, and the reason getIsAddvance
//        could not simply delegate.
//   [E6] donor mCurActionIdx @0x3C IS file-meaningful and agrees with the
//        receiver's field_0x3c at the same offset — so THAT one is shared,
//        exactly per the A2 law (share what agrees, own what doesn't).
// ============================================================

#include "d/ext_evt/evt1_boundary.h"

#include "d/d_com_inf_game.h"
#include "d/d_event_data.h"
#include "d/d_event_manager.h"
#include "SSystem/SComponent/c_counter.h"  // §718 advance trace frame stamp
#include "dusk/logging.h"

#include <cstring>

namespace JEvent1 {

// A2's scratch tables (evt1_event_data.cpp).
u8& evt1_staffAdvance(int staffIdx);

// ============================================================
// [E8] A4 HANG ROOT — and why this is now DERIVED, not stored.
// The donor clears FIVE scratch fields in dEvDtStaff_c::advanceCut (:186):
// mTimer, mWipeDirection, mAdvance=1, mbHasAction=false, mCurActionIdx=-1.
// JEvent1's scratch was never cleared, because advances run through the
// RECEIVER's advanceCut, which touches only TP's fields — so mbHasAction
// stayed true forever, getMyActIdx returned its first cached index for the
// rest of the event, and the staff froze on cut 0. That is the A4 event-entry
// hang (log 225938: entry() GRANT then nothing).
// FIX: derive cache validity from the SHARED current-cut field instead of
// storing it. When the cut advances the cache is stale BY CONSTRUCTION, so no
// reset hook exists to be missed. The split-scratch lesson generalised: state
// that must stay in step with shared state should be DERIVED FROM IT, not
// mirrored beside it.
// ============================================================
// §727: sized for WW STAGE packs (sea staffNum=992), not TP's 64 — see the
// capacity banner in evt1_event_data.cpp. The old `& 63` masking aliased
// high staff indices onto each other and is gone with it.
static const int kEvt1MaxStaff = 2048;
static u8 s_hasAction[kEvt1MaxStaff];
static s16 s_actionCut[kEvt1MaxStaff];   // the cut index the cached action belongs to

void evt1_resetActionCache() {
    for (int i = 0; i < kEvt1MaxStaff; i++) {
        s_hasAction[i] = 0;
        s_actionCut[i] = -1;
    }
}

// [E8] donor advanceCut's scratch half, called by evt1_advanceCutLocal at the
// moment of advance so the WW stack matches the donor field for field.
void evt1_onStaffAdvance(int staffIdx) {
    if (staffIdx >= 0 && staffIdx < kEvt1MaxStaff) {
        s_hasAction[staffIdx] = 0;
        s_actionCut[staffIdx] = -1;
    }
    // §718: frame-stamped advance trace — instant per-frame advances here are
    // the [E5]/endCheck signature; sparse ones mean cuts genuinely completed.
    static int s_advTrace = 0;
    if (s_advTrace < 48) {
        s_advTrace++;
        DuskLog.info("[Evt1] §718 staff-advance: staff={} gFrm={}", staffIdx,
                     (int)g_Counter.mCounter0);
    }
}

static u8& hasAction(int staffIdx) {
    static u8 dummy = 0;
    if (staffIdx < 0 || staffIdx >= kEvt1MaxStaff) {
        dummy = 0;
        return dummy;
    }
    return s_hasAction[staffIdx];
}

// ============================================================
// [E7] getMyStaffId is NOT ported — it DELEGATES to the receiver's.
// The donor iterates the whole event pool looking for states PLAY/UNK3/CLOSE
// because WW's manager tracks state per event; TP tracks ONE current event id
// (mCurrentEvId) and its enum does not define the donor's values at all
// (receiver: START=1, END=2 only). Under this project's one-event-at-a-time
// invariant the two reach the SAME live event, and audit №284 found no fork in
// staff lookup — only in the action index, the advance flag and cutEnd.
// Inventing an enum mapping to "port" a function that does not diverge would
// be guessing at semantics, which is exactly what §375 taught us not to do.
// Share what agrees; own what does not.
// ============================================================
int evt1_getMyStaffId(const char* i_name, fopAc_ac_c* i_actor, int i_tagId) {
    return dComIfGp_getEventManager().getMyStaffId(i_name, i_actor, i_tagId);
}

/* 80074718-8007473C       .text getIsAddvance__16dEvent_manager_cFi */
BOOL evt1_getIsAddvance(int i_staffIdx) {
    if (i_staffIdx == -1) {
        return FALSE;
    }
    return evt1_staffAdvance(i_staffIdx) != 0;   // [E5] JEvent1 scratch
}

/* 8007473C-80074824       .text dEvmng_strcmp__FPCcPc */
// Donor's own comparison: prefix-tolerant (s2 may be longer), length-capped.
// NOT std::strcmp — the difference is load-bearing for nameType != 0 call sites.
static int evt1_strcmp(const char* s1, const char* s2) {
    const size_t len1 = std::strlen(s1);
    const size_t len2 = std::strlen(s2);
    if (len1 >= 100 || len2 >= 100) {
        return -1;   // [E4] donor JUT_ASSERT is a retail no-op
    }
    if (len1 > len2) {
        return 1;
    }
    for (size_t i = 0; i < len1; i++) {
        if (s1[i] != s2[i]) {
            return 1;
        }
    }
    return 0;
}

/* 80074824-80074964       .text getMyActIdx__16dEvent_manager_cFiPCPCciii */
// §295's home. The donor's no-match result is -1 and its OWN actors expect it.
int evt1_getMyActIdx(int i_staffIdx, DUSK_CONST char* DUSK_CONST* i_action, int i_actionNum,
                     BOOL i_force, int i_nameType) {
    if (i_staffIdx == -1) {
        return -1;
    }

    dEvDtBase_c& list = dComIfGp_getEventManager().getBase();
    // ========================================================================
    // §713 (WAVE-1 row 16): TEARDOWN GUARD at the DISPATCH — №283/№285 family.
    // After an event ends, the base's staff array is torn down, but an actor
    // still sitting in its demo action can re-query with a STALE staff idx.
    // Receipt (symbolicated, fault +0x218): sea outdoor knob00 (m2C6=7) after
    // DEFAULT_KNOB_DOOR_F_OPEN was killed with no stage change — demoProc2 →
    // getDemoAction → here, dead staff pointer. The donor's own no-match
    // contract is -1, and a dead event IS a no-match: its actors get "no
    // action" and wind down through their own default branches (knob00's
    // default = cutEnd, benign). Guard at the dispatch, not in each actor —
    // the №283 law: fix the lifecycle where the divergence lives.
    // ========================================================================
    if (list.getStaffP() == NULL || list.getHeaderP() == NULL || i_staffIdx < 0 ||
        i_staffIdx >= list.getStaffNum()) {
        static u32 s_teardownHits = 0;
        if (s_teardownHits++ == 0) {
            DuskLog.warn("[Evt1] §713 stale staff idx {} queried after event teardown — "
                         "returning donor no-match (-1); actor winds down",
                         i_staffIdx);
        }
        return -1;
    }
    dEvDtStaff_c* staff = list.getStaffP(i_staffIdx);

    // [E8] validity is derived: the cache is good only while the staff is
    // still on the cut it was built for.
    const int curCut = staff->getCurrentCut();
    if (hasAction(i_staffIdx) && i_staffIdx < kEvt1MaxStaff && s_actionCut[i_staffIdx] == (s16)curCut && !i_force) {
        return staff->field_0x3c;
    }

    // ========================================================================
    // §726 ROOT FIX (the 5-frame event death, §718 trace receipt): the
    // receiver's getMyNowCutName carries a TARGET_LITTLE_ENDIAN wrapper that
    // memcpy's FOUR bytes into an UNTERMINATED static buf and byte-reverses
    // them — correct for TP's loader-swapped packs with ≤4-char cut names,
    // CORRUPTING for WW packs whose names arrive in file order ('FIXE…' →
    // 'EXIF<junk>', 'PAUS…' → 'SUAP<junk>' — the §718 log, un-reversed). No
    // action can ever match a corrupted name, nothing holds the event, and it
    // dies the frame its start-checks pass. The donor's own getMyNowCutName
    // (800749A0) is cut->getName() DIRECT — ported verbatim; the receiver's
    // wrapper stays untouched for TP (scoping law №282/№283).
    // ========================================================================
    char* name = list.getCutStaffCurrentCutP(i_staffIdx)->getName();
    if (name == NULL) {
        return -1;
    }

    hasAction(i_staffIdx) = 1;
    if (i_staffIdx < kEvt1MaxStaff) {
        s_actionCut[i_staffIdx] = (s16)curCut;   // [E8]
    }
    for (s32 i = 0; i < i_actionNum; i++) {
        if (i_action[i] == NULL) {
            continue;   // [E4]
        }
        const bool hit = i_nameType != 0 ? (evt1_strcmp(i_action[i], name) == 0)
                                         : (std::strcmp(i_action[i], name) == 0);
        if (hit) {
            staff->field_0x3c = i;
            // ================================================================
            // §718 CUT TRACE (pass-12/H11 refinement): the 19:47 run proved the
            // door event STARTS (H2 printed, base live, staffNum=992 genuine —
            // donor header layout verified identical) and dies in 5 frames.
            // The remaining question is CUT-LEVEL: do names mismatch, or do
            // cuts advance instantly ([E5] scratch-split / endCheck reading
            // TP fields the WW dialect never writes)? First-48 both ways.
            // Strip with §717/§718.
            // ================================================================
            static int s_hitTrace = 0;
            if (s_hitTrace < 48) {
                s_hitTrace++;
                DuskLog.info("[Evt1] §718 cut-hit: staff={} cut={} name='{}' -> action {}",
                             i_staffIdx, curCut, name, (int)i);
            }
            return i;
        }
    }

    static int s_missTrace = 0;
    if (s_missTrace < 48) {
        s_missTrace++;
        DuskLog.info("[Evt1] §718 cut-MISS: staff={} cut={} name='{}' (actionNum={}) -> -1",
                     i_staffIdx, curCut, name, i_actionNum);
    }
    staff->field_0x3c = -1;
    return -1;   // donor contract — NOT the receiver's 0
}

/* 80074B48-80074B7C       .text cutEnd__16dEvent_manager_cFi */
// Donor cutEnd in full: no event-mode gate, no current-event check. §320's
// global `if (false && …)` was an attempt to make TP's function behave like
// this one; TP's gate is restored (№285) and the ungated form lives here.
void evt1_cutEnd(int i_staffIdx) {
    if (i_staffIdx == -1) {
        return;
    }
    dEvent_manager_c& mgr = dComIfGp_getEventManager();
    // ========================================================================
    // §714 (History, composing with §713's row-16 guard): the wind-down path
    // §713 CREATES routes here. getMyActIdx now returns -1 on a torn-down
    // event, and knob00's default branch on "no action" is cutEnd — but
    // getCutStaffCurrentCutP(i) is mStaffP[i].getCurrentCut() with NO null
    // check, the EXACT read that faulted: 0x218 = NULL + 6*sizeof(staff 0x50)
    // + mCurrentCut@0x38, i.e. staff 6's cut index off a NULL staff array.
    // Without this, §713's retest crashes at the same address one call later.
    // Donor rationale, same as [E4]: a dead event has no flag to set — the
    // donor can never reach cutEnd after teardown because its dispatch stops
    // its actors first; the port's lifetime gap makes the state reachable, so
    // the boundary absorbs it as a benign no-op.
    // ========================================================================
    dEvDtBase_c& base = mgr.getBase();
    if (base.getStaffP() == NULL || base.getHeaderP() == NULL || i_staffIdx < 0 ||
        i_staffIdx >= base.getStaffNum()) {
        static u32 s_teardownHits = 0;
        if (s_teardownHits++ == 0) {
            DuskLog.warn("[Evt1] §714 cutEnd({}) after event teardown — no-op "
                         "(dead event has no flag to set)",
                         i_staffIdx);
        }
        return;
    }
    dEvDtCut_c* cut = mgr.getBase().getCutStaffCurrentCutP(i_staffIdx);
    if (cut == NULL) {
        return;   // [E4] port-side null-safety; donor cannot reach NULL here
    }
    mgr.getFlags().flagSet(cut->getFlagId());
}

}  // namespace JEvent1
