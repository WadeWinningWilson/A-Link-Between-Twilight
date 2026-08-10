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
#include "d/d_ext_npc_mount.h"
#include "d/d_kankyo_ww_sky.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_seq_space.h"
#include "d/ww_jpa.h"
#include "d/d_com_inf_game.h"
#include "f_pc/f_pc_profile.h"
// Included for its `extern "C"` linkage: without it this definition takes
// C++ mangling and does not match the declaration the receiver calls.
#include "d/ext_plugin/ww_profile_register.h"
#include "d/ext_plugin/ww_room_loader.h"
#include "d/ext_plugin/ww_stage_loader.h"
#include "d/d_stage.h"

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
// §659: d_a_vrbox.cpp is a RECEIVER TU and resolves its dome model through
// the DN-3 consume-time resolver, so with the WW layer excluded that symbol
// needs an answer. NULL is not a stub standing in for behaviour: it sends
// the caller down its own vrbox_sora.bmd path, which is exactly what a build
// with no donor content should do.
//
// It lives HERE rather than in ww_npcmount_dispatch.cpp because that TU owns
// the public names in BOTH configurations (runtime table) -- defining it
// there duplicated the WW layer's own definition and the normal build failed
// to link. This cluster compiles only when the layer is out.
J3DModelData* dExtNpcMount_acquireStageModelData(const char* arc, const char* modelName) {
    return NULL;
}
// ---------------------------------------------------------------------------
// §634: the WW room-load seam. ww_room_loader.cpp moves with the WW layer (its
// lineage is WW end to end — the seam and every chunk translator behind it), so
// the receiver's one call site in d_s_room.cpp needs an answer.
//
// This default is NOT a stub standing in for absent behaviour. Its body is
// EXACTLY the call d_s_room.cpp made before the seam existed, so with no WW
// layer the room-load path is byte-equivalent to the unported receiver — the
// same relationship ww_profile_register.cpp has with fpcPf_Get.
//
// The distinction is worth keeping sharp, because getting it wrong once already
// cost a round: defaulting a WW symbol that a RECEIVER TU calls hides a leg.
// Defaulting the ONE symbol that IS the layer boundary is the boundary working.
// ---------------------------------------------------------------------------
void dExtWwRoom_loadRoomDzr(void* i_data, dStage_dt_c* i_stage, int i_roomNo) {
    dStage_dt_c_roomLoader(i_data, i_stage, i_roomNo);
}
// §661: same relationship for the STAGE seam — the default body is exactly
// the call d_stage.cpp made before it existed.
void dExtWwStage_loadStageDzs(void* i_data, dStage_dt_c* i_stage) {
    dStage_dt_c_stageLoader(i_data, i_stage);
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

// ---------------------------------------------------------------------------
// ww_jpa — the WW particle-archive reader. Two entry points, both bool, and the
// receiver ALREADY handles both failing:
//   d_particle.cpp:1647  if (!s_wwArc.parse(...))          -> gives up cleanly
//   d_particle.cpp:1694  if (!ww_jpa::bindResource(...))   -> marks the entry
//                                                             failed and returns
// So "false" here is not a default invented to satisfy the linker -- it is the
// value the call sites were already written to expect when a WW archive cannot
// be read, which without the WW layer is always.
// ---------------------------------------------------------------------------
namespace ww_jpa {
bool Archive::parse(const u8*, u32) {
    return false;
}
bool bindResource(const Archive&, u16, class JPAResourceManager*, class JKRHeap*) {
    return false;
}
}  // namespace ww_jpa

// ---------------------------------------------------------------------------
// JEvent1 — the WW event stack. 6 symbols reaching 41 TUs, which sounds like
// the worst cluster and is actually the easiest, because the reach is ONE
// HEADER rather than 41 decisions.
//
// Every call lives in an inline wrapper in d_com_inf_game.h, and every wrapper
// is already gated:
//
//     inline void dComIfGp_evmng_cutEnd(int id) {
//         if (JEvent1::evt1_isActive()) { JEvent1::evt1_cutEnd(id); return; }
//         dComIfGp_getPEvtManager()->cutEnd(id);      <- TP's own manager
//     }
//
// So `evt1_isActive() == false` sends every wrapper down the TP path, which is
// precisely correct with no WW layer -- the receiver's event system handles
// everything, as it did before the port. The other five are then UNREACHABLE at
// runtime; they need definitions only so the link succeeds.
//
// getMyActIdx returns -1 rather than 0 deliberately. d_com_inf_game.h records
// that the WW manager's no-match contract is -1 while TP's is 0; this function
// is the WW one, so it keeps the WW contract even though the gate means it
// cannot be called. A default that contradicts its own documented contract is a
// trap for whoever removes the gate later.
// ---------------------------------------------------------------------------
namespace JEvent1 {
bool evt1_isActive(void) {
    return false;   // no WW event stack -> TP's manager owns every wrapper
}
bool evt1_advanceCutLocal(class dEvDtBase_c*, class dEvDtStaff_c*) {
    return false;
}
int evt1_getIsAddvance(int) {
    return 0;
}
int evt1_getMyActIdx(int, DUSK_CONST char* DUSK_CONST*, int, BOOL, int) {
    return -1;      // WW's no-match contract, not TP's 0
}
void evt1_cutEnd(int) {
}
void evt1_specialProc(class dEvDtStaff_c*) {
}
}  // namespace JEvent1

// dExtNpcWorld_bump — the ONE straggler whose caller is genuinely dusklight's
// (d_stage.cpp:3325, a file that exists in dusklight main). The others are
// called only from TUs that move to the plugin, so they leave with it. This one
// needs a default because the receiver keeps calling it: a no-op, since with no
// WW layer there is no NPC world to bump.
void dExtNpcWorld_bump(const char* /*reason*/) {
}

// dWwProfileRegister_lookup — fpcPf_Get calls this on every actor creation and
// f_pc_profile.cpp is receiver-side, so it needs an answer with no WW layer.
// NULL means "not ours", and fpcPf_Get then reads the receiver's own table --
// where the 20 WW indices are already NULL, so those actors simply do not
// create. That is the correct plugin behaviour: no plugin, no WW actors.
process_profile_definition DUSK_CONST* dWwProfileRegister_lookup(s16 /*index*/) {
    return NULL;
}

#endif  // DUSK_EXCLUDE_WW_ACTIVE
