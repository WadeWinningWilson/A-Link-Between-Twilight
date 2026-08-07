// ============================================================================
// ww_npcmount_dispatch.cpp — receiver-side. Step 19 cluster 3.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// NOT part of the WW layer and never excluded: this is what lets the receiver
// stand alone. GENERATED BODY -- regenerate rather than hand-edit.
// ============================================================================
#include "d/ext_plugin/ww_npcmount_dispatch.h"

WwNpcMountApi g_wwNpcMountApi = {};

int dWwNpcMountDispatch_boundCount() {
    void** p = (void**)&g_wwNpcMountApi;
    const int n = (int)(sizeof(WwNpcMountApi) / sizeof(void*));
    int bound = 0;
    for (int i = 0; i < n; ++i) {
        if (p[i] != 0) { ++bound; }
    }
    return bound;
}

J3DModelData* dExtNpcMount_acquireDemoModel(const char* arc, u16 id, void* res) {
    return g_wwNpcMountApi.fn_acquireDemoModel ? g_wwNpcMountApi.fn_acquireDemoModel(arc, id, res) : NULL;
}
J3DModelData* dExtNpcMount_acquireModelData(const char* arc, const char* modelName) {
    return g_wwNpcMountApi.fn_acquireModelData ? g_wwNpcMountApi.fn_acquireModelData(arc, modelName) : NULL;
}
void dExtNpcMount_cancelTransports() {
    if (g_wwNpcMountApi.fn_cancelTransports) { g_wwNpcMountApi.fn_cancelTransports(); }
}
bool dExtNpcMount_consumeForcedCreateProc(char* procOut, u32 procOutBytes) {
    return g_wwNpcMountApi.fn_consumeForcedCreateProc ? g_wwNpcMountApi.fn_consumeForcedCreateProc(procOut, procOutBytes) : false;
}
int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName) {
    return g_wwNpcMountApi.fn_create ? g_wwNpcMountApi.fn_create(i_this, procName) : 0;
}
bool dExtNpcMount_cycleHeadNearest(const cXyz& from, f32 maxDist) {
    return g_wwNpcMountApi.fn_cycleHeadNearest ? g_wwNpcMountApi.fn_cycleHeadNearest(from, maxDist) : false;
}
int dExtNpcMount_delete(dExtNpcMount_c* i_this) {
    return g_wwNpcMountApi.fn_delete ? g_wwNpcMountApi.fn_delete(i_this) : 0;
}
int dExtNpcMount_draw(dExtNpcMount_c* i_this) {
    return g_wwNpcMountApi.fn_draw ? g_wwNpcMountApi.fn_draw(i_this) : 0;
}
void dExtNpcMount_endDoorDemoLock() {
    if (g_wwNpcMountApi.fn_endDoorDemoLock) { g_wwNpcMountApi.fn_endDoorDemoLock(); }
}
int dExtNpcMount_execute(dExtNpcMount_c* i_this) {
    return g_wwNpcMountApi.fn_execute ? g_wwNpcMountApi.fn_execute(i_this) : 0;
}
void dExtNpcMount_forceNextAttach(const char* modelName, const char* jointName) {
    if (g_wwNpcMountApi.fn_forceNextAttach) { g_wwNpcMountApi.fn_forceNextAttach(modelName, jointName); }
}
void dExtNpcMount_forceNextSpawnSrc(const char* src) {
    if (g_wwNpcMountApi.fn_forceNextSpawnSrc) { g_wwNpcMountApi.fn_forceNextSpawnSrc(src); }
}
bool dExtNpcMount_hasPayload(const char* procName) {
    return g_wwNpcMountApi.fn_hasPayload ? g_wwNpcMountApi.fn_hasPayload(procName) : false;
}
bool dExtNpcMount_hasSocketPayload(const char* socketName) {
    return g_wwNpcMountApi.fn_hasSocketPayload ? g_wwNpcMountApi.fn_hasSocketPayload(socketName) : false;
}
bool dExtNpcMount_isRoomLaneProtected(int roomNo) {
    return g_wwNpcMountApi.fn_isRoomLaneProtected ? g_wwNpcMountApi.fn_isRoomLaneProtected(roomNo) : false;
}
bool dExtNpcMount_isRoomLaneRoom(int roomNo) {
    return g_wwNpcMountApi.fn_isRoomLaneRoom ? g_wwNpcMountApi.fn_isRoomLaneRoom(roomNo) : false;
}
bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out) {
    return g_wwNpcMountApi.fn_lookup ? g_wwNpcMountApi.fn_lookup(procName, out) : false;
}
const char* dExtNpcMount_nearestDisplayName(const cXyz& from, f32 maxDist) {
    return g_wwNpcMountApi.fn_nearestDisplayName ? g_wwNpcMountApi.fn_nearestDisplayName(from, maxDist) : NULL;
}
void dExtNpcMount_onRoomObjectsReady(const char* stageName, int roomNo) {
    if (g_wwNpcMountApi.fn_onRoomObjectsReady) { g_wwNpcMountApi.fn_onRoomObjectsReady(stageName, roomNo); }
}
void dExtNpcMount_onRoomUnload(const char* stageName, int roomNo) {
    if (g_wwNpcMountApi.fn_onRoomUnload) { g_wwNpcMountApi.fn_onRoomUnload(stageName, roomNo); }
}
void dExtNpcMount_onStageReady() {
    if (g_wwNpcMountApi.fn_onStageReady) { g_wwNpcMountApi.fn_onStageReady(); }
}
void dExtNpcMount_pollBgWarps() {
    if (g_wwNpcMountApi.fn_pollBgWarps) { g_wwNpcMountApi.fn_pollBgWarps(); }
}
void dExtNpcMount_pollCullProbe() {
    if (g_wwNpcMountApi.fn_pollCullProbe) { g_wwNpcMountApi.fn_pollCullProbe(); }
}
void dExtNpcMount_pollIdentifyProbe() {
    if (g_wwNpcMountApi.fn_pollIdentifyProbe) { g_wwNpcMountApi.fn_pollIdentifyProbe(); }
}
void dExtNpcMount_pollRegionTriggers() {
    if (g_wwNpcMountApi.fn_pollRegionTriggers) { g_wwNpcMountApi.fn_pollRegionTriggers(); }
}
bool dExtNpcMount_providerAt(int index, dExtNpcManifest* out) {
    return g_wwNpcMountApi.fn_providerAt ? g_wwNpcMountApi.fn_providerAt(index, out) : false;
}
int dExtNpcMount_providerCount() {
    return g_wwNpcMountApi.fn_providerCount ? g_wwNpcMountApi.fn_providerCount() : 0;
}
bool dExtNpcMount_queryActor(const fopAc_ac_c* actor, dExtNpcIdentifyInfo* out) {
    return g_wwNpcMountApi.fn_queryActor ? g_wwNpcMountApi.fn_queryActor(actor, out) : false;
}
void dExtNpcMount_registerRoomLane(const char* procName, int hostRoom) {
    if (g_wwNpcMountApi.fn_registerRoomLane) { g_wwNpcMountApi.fn_registerRoomLane(procName, hostRoom); }
}
bool dExtNpcMount_requestBgWarp(const char* procName) {
    return g_wwNpcMountApi.fn_requestBgWarp ? g_wwNpcMountApi.fn_requestBgWarp(procName) : false;
}
void dExtNpcMount_rescanProviders() {
    if (g_wwNpcMountApi.fn_rescanProviders) { g_wwNpcMountApi.fn_rescanProviders(); }
}
bool dExtNpcMount_resolveSocket(const char* socketName, int arg, char* procOut, u32 procOutBytes) {
    return g_wwNpcMountApi.fn_resolveSocket ? g_wwNpcMountApi.fn_resolveSocket(socketName, arg, procOut, procOutBytes) : false;
}
bool dExtNpcMount_setDisplayNameNearest(const cXyz& from, f32 maxDist, const char* name) {
    return g_wwNpcMountApi.fn_setDisplayNameNearest ? g_wwNpcMountApi.fn_setDisplayNameNearest(from, maxDist, name) : false;
}
bool dExtNpcMount_shouldSkipBtp(const char* arcName) {
    return g_wwNpcMountApi.fn_shouldSkipBtp ? g_wwNpcMountApi.fn_shouldSkipBtp(arcName) : false;
}
s16 dExtNpcMount_socketActorId(const char* socketName) {
    return g_wwNpcMountApi.fn_socketActorId ? g_wwNpcMountApi.fn_socketActorId(socketName) : 0;
}
bool dExtNpcMount_takePendingSpawn(fpc_ProcID id, char* procOut, u32 procBytes, char* srcOut, u32 srcBytes, char* headOut, u32 headBytes, char* jointOut, u32 jointBytes) {
    return g_wwNpcMountApi.fn_takePendingSpawn ? g_wwNpcMountApi.fn_takePendingSpawn(id, procOut, procBytes, srcOut, srcBytes, headOut, headBytes, jointOut, jointBytes) : false;
}
