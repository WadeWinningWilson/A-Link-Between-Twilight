// ============================================================================
// ww_misc_dispatch.cpp — step 19 cluster 4 (the tail): 23 entry points across
// FOUR WW implementation TUs (npc_mount, kankyo_ww_sky, npc_doors, seq_space).
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WHY THIS CLUSTER USES A DIFFERENT MECHANISM TO CLUSTERS 2 AND 3
//
// Those use a runtime dispatch table: the WW impl's symbols are renamed via a
// generated macro header, and the receiver-side dispatcher owns the public
// names. That works when the impl TU can be renamed cleanly. Here it cannot,
// and the reason is worth recording rather than rediscovering:
//
//   - the rename header must precede EVERY include so the header declarations
//     rename too;
//   - but a declaration needs its parameter TYPES (`dExtNpcMount_c`, knob
//     structs), which are not declared yet at that point;
//   - and relying on the ORIGINAL header's declaration surviving the macro
//     fails on include order -- calls renamed, declaration not, "identifier
//     not found" thousands of lines before the definition.
//
// Three orderings were tried and all three failed on the same knot. So this
// cluster takes the simpler mechanism: these defaults compile ONLY when the WW
// layer is excluded, and the WW impls own the names otherwise. No renames, no
// ordering hazard, and the two configurations cannot both define a symbol.
//
// WHAT IT COSTS, stated plainly: this is COMPILE-time selection, not runtime
// dispatch. A plugin overriding these would hook the 23 functions individually
// rather than filling one table. That is a real difference and the runtime form
// is better -- clusters 2 and 3 already demonstrate it. This one is not worth
// the fragility, and a mechanism that cannot be built is worth less than a
// plainer one that can.
// ============================================================================
#include "d/d_ext_save_guard.h"
#include "d/d_kankyo_ww_sky.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_seq_space.h"

#if defined(DUSK_EXCLUDE_WW_ACTIVE)

// Defaults follow the return types, as in every other cluster: void does
// nothing, bool is false ("not a WW case"), scalars 0. None fabricates a value
// the receiver could mistake for real data.

bool dExtWw_applyPlayerDonorLook(J3DModel* model, dKy_tevstr_c* tevStr) {
    return false;
}
void dExtWw_drawDemoMessage() {
}
bool dExtWw_handleDemoMessage(u32 donorMsgId) {
    return false;
}
bool dExtWw_injectTalkChain(u32 i_msgIdx) {
    return false;
}
bool dExtWw_injectTalkText(fopAc_ac_c* i_talkActor, u32 i_msgIdx) {
    return false;
}
bool dExtWw_isTaleRunEvent(const char* runEvt) {
    return false;
}
bool dExtWw_openingPauseArrivalGuard() {
    return false;
}
void dExtWw_pollDemoMessage() {
}
bool dExtWwSave_isWwHostStage(const char* stageName) {
    return false;
}
bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit) {
    return false;
}
void dKyWwSky_drawStar() {
}
void dKyWwSky_drawSun() {
}
void dKyWwSky_moveStar() {
}
void dKyWwSky_moveSun() {
}
void dKyWwSky_reset() {
}
void dExtNpcDoors_armArrivalGuard(const char* stage) {
}
void dExtNpcDoors_poll() {
}
void dExtNpcDoors_stampKnobByIndex(dExtNpcMount_c* knob, int doorIndex1Based) {
}
bool dExtNpcDoors_tryNativeWarp(fopAc_ac_c* doorActor, bool openAlreadyDone) {
    return false;
}
int dExtSeqSpace_cliDumpEvents(const char* packageRoot) {
    return 0;
}
void dExtSeqSpace_poll() {
}
bool dExtSeqSpace_shouldSuppressJa2Bgm() {
    return false;
}
bool dExtWw_j3dForceFullMat3() {
    return false;
}

#endif  // DUSK_EXCLUDE_WW_ACTIVE
