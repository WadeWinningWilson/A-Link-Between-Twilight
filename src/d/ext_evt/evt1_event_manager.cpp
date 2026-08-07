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
static u8 s_hasAction[64];
static s16 s_actionCut[64];   // the cut index the cached action belongs to

void evt1_resetActionCache() {
    for (int i = 0; i < 64; i++) {
        s_hasAction[i] = 0;
        s_actionCut[i] = -1;
    }
}

// [E8] donor advanceCut's scratch half, called by evt1_advanceCutLocal at the
// moment of advance so the WW stack matches the donor field for field.
void evt1_onStaffAdvance(int staffIdx) {
    if (staffIdx >= 0 && staffIdx < 64) {
        s_hasAction[staffIdx] = 0;
        s_actionCut[staffIdx] = -1;
    }
}

static u8& hasAction(int staffIdx) {
    static u8 dummy = 0;
    if (staffIdx < 0 || staffIdx >= 64) {
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
    dEvDtStaff_c* staff = list.getStaffP(i_staffIdx);

    // [E8] validity is derived: the cache is good only while the staff is
    // still on the cut it was built for.
    const int curCut = staff->getCurrentCut();
    if (hasAction(i_staffIdx) && s_actionCut[i_staffIdx & 63] == (s16)curCut && !i_force) {
        return staff->field_0x3c;
    }

    char* name = dComIfGp_getEventManager().getMyNowCutName(i_staffIdx);
    if (name == NULL) {
        return -1;
    }

    hasAction(i_staffIdx) = 1;
    s_actionCut[i_staffIdx & 63] = (s16)curCut;   // [E8]
    for (s32 i = 0; i < i_actionNum; i++) {
        if (i_action[i] == NULL) {
            continue;   // [E4]
        }
        const bool hit = i_nameType != 0 ? (evt1_strcmp(i_action[i], name) == 0)
                                         : (std::strcmp(i_action[i], name) == 0);
        if (hit) {
            staff->field_0x3c = i;
            return i;
        }
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
    dEvDtCut_c* cut = mgr.getBase().getCutStaffCurrentCutP(i_staffIdx);
    if (cut == NULL) {
        return;   // [E4] port-side null-safety; donor cannot reach NULL here
    }
    mgr.getFlags().flagSet(cut->getFlagId());
}

}  // namespace JEvent1
