// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §423 A1 — boundary gate body. See include/d/ext_evt/evt1_boundary.h for the
// campaign rationale (audit №284's fork findings) and the phase plan.
// ============================================================

#include "d/ext_evt/evt1_boundary.h"

#include "d/d_com_inf_game.h"
#include "d/d_ext_save_guard.h"

namespace JEvent1 {

bool evt1_isActive() {
#if DUSK_EVT1_NATIVE
    // A1 discriminator: WW-host context — identical to the one №285's scope
    // fixes already use, so A4's flip changes WHICH stack runs, never WHEN.
    // A4 replaces this with the running event's own provenance.
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && dExtWwSave_isWwHostStage(stage);
#else
    return false;  // kill switch: receiver stack serves everything (today's behavior)
#endif
}

}  // namespace JEvent1
