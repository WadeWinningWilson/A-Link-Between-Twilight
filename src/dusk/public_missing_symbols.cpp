// Link stubs for WW call sites that are not compiled on the public tree.
#include "d/d_ext_save_guard.h"
#include "d/d_ext_seq_space.h"
#include "dusk/leveledit/enumerate.hpp"
#include "d/d_camera.h"
#include "d/d_ext_npc_mount.h"
#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_xyz.h"

class J3DModelData;

#if TARGET_PC

void dExtNpcMount_rescanProviders() {}
void dExtNpcMount_pollBgWarps() {}
void dExtNpcMount_pollRegionTriggers() {}
void dExtNpcMount_pollIdentifyProbe() {}
void dExtNpcMount_pollCullProbe() {}
void dExtNpcMount_onStageReady() {}
bool dExtNpcMount_cycleHeadNearest(const cXyz&, f32) { return false; }
bool dExtNpcMount_setDisplayNameNearest(const cXyz&, f32, const char*) { return false; }
const char* dExtNpcMount_nearestDisplayName(const cXyz&, f32) { return NULL; }
bool dExtNpcMount_hasSocketPayload(const char*) { return false; }
s16 dExtNpcMount_socketActorId(const char*) { return -1; }
int dExtNpcMount_providerCount() { return 0; }
bool dExtNpcMount_providerAt(int, dExtNpcManifest*) { return false; }
bool dExtNpcMount_queryActor(const fopAc_ac_c*, dExtNpcIdentifyInfo*) { return false; }
J3DModelData* dExtNpcMount_acquireModelData(const char*, const char*) { return NULL; }
J3DModelData* dExtNpcMount_acquireModelDataByIndex(const char*, int) { return NULL; }
J3DModelData* dExtNpcMount_acquireStageModelData(const char*, const char*) { return NULL; }
J3DModelData* dExtNpcMount_acquireDemoModel(const char*, u16, void* res) {
    return static_cast<J3DModelData*>(res);
}
void dExtNpcMount_retainArc(const char*) {}
void dExtNpcMount_releaseArc(const char*) {}
bool dExtNpcMount_shouldSkipBtp(const char*) { return false; }

void dEvtFork_guardReport(int) {}

namespace JEvent1 {
bool evt1_isActive() { return false; }
int evt1_getMyActIdx(int, char const* const*, int, BOOL, int) { return 0; }
BOOL evt1_getIsAddvance(int) { return FALSE; }
void evt1_cutEnd(int) {}
}

void dExtSeqSpace_poll() {}
bool dExtSeqSpace_shouldSuppressJa2Bgm() { return false; }
void dExtSeqSpace_requestHandoffToField(u32) {}
int dExtSeqSpace_cliDumpEvents(const char*) { return 1; }

bool dExtWwSave_isWwHostStage(const char*) { return false; }
void dExtWwSave_registerWwStage(const char*) {}
bool dExtWwSave_isDeclaredWwStage(const char*) { return false; }
bool dExtWw_applyPlayerDonorLook(J3DModel*, dKy_tevstr_c*) { return false; }
bool dExtWw_handleDemoMessage(u32) { return false; }
bool dExtWw_injectTalkText(fopAc_ac_c*, u32) { return false; }
bool dExtWw_isWwTalkProc(s16) { return false; }
bool dExtWw_injectTalkChain(u32) { return false; }
bool dExtWw_isWwDemoStage() { return false; }
void dExtWw_pollDemoMessage() {}
void dExtWw_drawDemoMessage() {}
bool dExtWw_openingOwnsCamera() { return false; }
bool dExtWw_openingPauseArrivalGuard() { return false; }
bool dExtWw_isTaleRunEvent(const char*) { return false; }
bool dExtWw_deferArrivalCameraSnap() { return false; }
bool dExtWwSave_isWwContentActive() { return false; }
bool dExtWwSave_refuseNativeWrite(const char*, int, int) { return false; }

void dCamera_c::resetEditorFlyCamMouseLook() {}

int fpcNdRq_DebugHead(int*, int*, int*) { return 0; }
int fopOvlpM_DebugState(int*, int*, int*, int*, int*, int*) { return 0; }

#endif
