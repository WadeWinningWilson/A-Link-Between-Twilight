// ============================================================================
// ww_itemmdl_dispatch.cpp — receiver-side. Step 19 cluster 2.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// This TU is NOT part of the WW layer and is never excluded: it is what lets the
// receiver stand alone. It owns the 35 public `dWwItemmdl_*` names and forwards
// each to the WW layer if that layer registered itself.
//
// GENERATED BODY — regenerate rather than hand-edit. See the header.
// ============================================================================
#include "d/ext_plugin/ww_itemmdl_dispatch.h"

// Zero-initialised: every entry point unbound until the WW layer installs.
WwItemmdlApi g_wwItemmdlApi = {};

int dWwItemmdlDispatch_boundCount() {
    // Counted over the struct as raw pointers rather than field-by-field, so
    // adding a field cannot silently escape the count.
    void** p = (void**)&g_wwItemmdlApi;
    const int n = (int)(sizeof(WwItemmdlApi) / sizeof(void*));
    int bound = 0;
    for (int i = 0; i < n; ++i) {
        if (p[i] != 0) { ++bound; }
    }
    return bound;
}

void dWwItemmdl_applyBowMaterialAmbientOnly(J3DModel* model, dKy_tevstr_c* tevstr_p) {
    if (g_wwItemmdlApi.applyBowMaterialAmbientOnly) { g_wwItemmdlApi.applyBowMaterialAmbientOnly(model, tevstr_p); }
}
void dWwItemmdl_beginBowDrawScope(J3DModel* model, fpc_ProcID owner_id) {
    if (g_wwItemmdlApi.beginBowDrawScope) { g_wwItemmdlApi.beginBowDrawScope(model, owner_id); }
}
void dWwItemmdl_bracketLog(const char* message) {
    if (g_wwItemmdlApi.bracketLog) { g_wwItemmdlApi.bracketLog(message); }
}
void dWwItemmdl_clearBowDrawScope() {
    if (g_wwItemmdlApi.clearBowDrawScope) { g_wwItemmdlApi.clearBowDrawScope(); }
}
void dWwItemmdl_clearClothesBundleCache() {
    if (g_wwItemmdlApi.clearClothesBundleCache) { g_wwItemmdlApi.clearClothesBundleCache(); }
}
void dWwItemmdl_clearOutlineSuppress() {
    if (g_wwItemmdlApi.clearOutlineSuppress) { g_wwItemmdlApi.clearOutlineSuppress(); }
}
const char* dWwItemmdl_clothesBundleArcName() {
    return g_wwItemmdlApi.clothesBundleArcName ? g_wwItemmdlApi.clothesBundleArcName() : NULL;
}
bool dWwItemmdl_clothesBundleForItem(u8 item_no) {
    return g_wwItemmdlApi.clothesBundleForItem ? g_wwItemmdlApi.clothesBundleForItem(item_no) : false;
}
const char* dWwItemmdl_clothesBundleGetTextForMessage(u32 msg_id) {
    return g_wwItemmdlApi.clothesBundleGetTextForMessage ? g_wwItemmdlApi.clothesBundleGetTextForMessage(msg_id) : NULL;
}
bool dWwItemmdl_clothesBundleHandOffset(f32* out_x, f32* out_y, f32* out_z) {
    return g_wwItemmdlApi.clothesBundleHandOffset ? g_wwItemmdlApi.clothesBundleHandOffset(out_x, out_y, out_z) : false;
}
u8 dWwItemmdl_clothesBundleHostItem() {
    return g_wwItemmdlApi.clothesBundleHostItem ? g_wwItemmdlApi.clothesBundleHostItem() : 0;
}
u32 dWwItemmdl_clothesBundleIconCap(u8 item_no) {
    return g_wwItemmdlApi.clothesBundleIconCap ? g_wwItemmdlApi.clothesBundleIconCap(item_no) : 0;
}
f32 dWwItemmdl_clothesBundleMaxScale() {
    return g_wwItemmdlApi.clothesBundleMaxScale ? g_wwItemmdlApi.clothesBundleMaxScale() : 0;
}
const char* dWwItemmdl_clothesBundleModelName() {
    return g_wwItemmdlApi.clothesBundleModelName ? g_wwItemmdlApi.clothesBundleModelName() : NULL;
}
void dWwItemmdl_debugLog(const char* message) {
    if (g_wwItemmdlApi.debugLog) { g_wwItemmdlApi.debugLog(message); }
}
void dWwItemmdl_drawWwBowModel(J3DModel* model) {
    if (g_wwItemmdlApi.drawWwBowModel) { g_wwItemmdlApi.drawWwBowModel(model); }
}
J3DModelData* dWwItemmdl_getClothesBundleModelData() {
    return g_wwItemmdlApi.getClothesBundleModelData ? g_wwItemmdlApi.getClothesBundleModelData() : NULL;
}
J3DModelData* dWwItemmdl_getItemmdlModelData(u16 bdl_index) {
    return g_wwItemmdlApi.getItemmdlModelData ? g_wwItemmdlApi.getItemmdlModelData(bdl_index) : NULL;
}
J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name) {
    return g_wwItemmdlApi.getVbowModelData ? g_wwItemmdlApi.getVbowModelData(arc_name) : NULL;
}
bool dWwItemmdl_isClothesGetPresentation() {
    return g_wwItemmdlApi.isClothesGetPresentation ? g_wwItemmdlApi.isClothesGetPresentation() : false;
}
bool dWwItemmdl_isPhase2BracketBow(u8 item_no) {
    return g_wwItemmdlApi.isPhase2BracketBow ? g_wwItemmdlApi.isPhase2BracketBow(item_no) : false;
}
void dWwItemmdl_log2QPrimeAudit(const char* tag, J3DModelData* model_data, s32 room_no) {
    if (g_wwItemmdlApi.log2QPrimeAudit) { g_wwItemmdlApi.log2QPrimeAudit(tag, model_data, room_no); }
}
void dWwItemmdl_logHeap(const char* label) {
    if (g_wwItemmdlApi.logHeap) { g_wwItemmdlApi.logHeap(label); }
}
void dWwItemmdl_logShapeInventory(J3DModelData* model_data, const char* phase) {
    if (g_wwItemmdlApi.logShapeInventory) { g_wwItemmdlApi.logShapeInventory(model_data, phase); }
}
void dWwItemmdl_logTevOrderDump(J3DModelData* model_data, const char* phase, s32 room_no, bool log_gx_runtime) {
    if (g_wwItemmdlApi.logTevOrderDump) { g_wwItemmdlApi.logTevOrderDump(model_data, phase, room_no, log_gx_runtime); }
}
void dWwItemmdl_patchModelForPc(J3DModelData* model_data) {
    if (g_wwItemmdlApi.patchModelForPc) { g_wwItemmdlApi.patchModelForPc(model_data); }
}
bool dWwItemmdl_retainItemmdlArcOnDemoItemDelete() {
    return g_wwItemmdlApi.retainItemmdlArcOnDemoItemDelete ? g_wwItemmdlApi.retainItemmdlArcOnDemoItemDelete() : false;
}
void dWwItemmdl_setWwBowActorAmbient(dKy_tevstr_c* tevstr_p) {
    if (g_wwItemmdlApi.setWwBowActorAmbient) { g_wwItemmdlApi.setWwBowActorAmbient(tevstr_p); }
}
void dWwItemmdl_setWwBowGetItemBeamSuppress(bool suppress) {
    if (g_wwItemmdlApi.setWwBowGetItemBeamSuppress) { g_wwItemmdlApi.setWwBowGetItemBeamSuppress(suppress); }
}
bool dWwItemmdl_shouldSuppressGetItemBeamParticle(u16 particle_id) {
    return g_wwItemmdlApi.shouldSuppressGetItemBeamParticle ? g_wwItemmdlApi.shouldSuppressGetItemBeamParticle(particle_id) : false;
}
int dWwItemmdl_stepPrivateItemmdlArcLoad(request_of_phase_process_class* phase) {
    return g_wwItemmdlApi.stepPrivateItemmdlArcLoad ? g_wwItemmdlApi.stepPrivateItemmdlArcLoad(phase) : 0;
}
void dWwItemmdl_tickHeldBowArcMount() {
    if (g_wwItemmdlApi.tickHeldBowArcMount) { g_wwItemmdlApi.tickHeldBowArcMount(); }
}
bool dWwItemmdl_use2DIsolateHeap() {
    return g_wwItemmdlApi.use2DIsolateHeap ? g_wwItemmdlApi.use2DIsolateHeap() : false;
}
bool dWwItemmdl_usingCustomHeldModel(J3DModel* i_model) {
    return g_wwItemmdlApi.usingCustomHeldModel ? g_wwItemmdlApi.usingCustomHeldModel(i_model) : false;
}
bool dWwItemmdl_writeClothesBundleIconTimg(u8 item_no, void* out_buf, u32 out_cap) {
    return g_wwItemmdlApi.writeClothesBundleIconTimg ? g_wwItemmdlApi.writeClothesBundleIconTimg(item_no, out_buf, out_cap) : false;
}

// The namespaced trio. `getBowGetItemDemoReplayStatus` returning NULL is not an
// invented default -- d_ww_itemmdl_test.h documents "nullptr if none", and the
// caller in editor.cpp already null-checks it.
namespace dWwItemmdl {
void requestBowGetItemDemoReplay() {
    if (g_wwItemmdlApi.requestBowGetItemDemoReplay) { g_wwItemmdlApi.requestBowGetItemDemoReplay(); }
}
void tickBowGetItemDemoReplay() {
    if (g_wwItemmdlApi.tickBowGetItemDemoReplay) { g_wwItemmdlApi.tickBowGetItemDemoReplay(); }
}
const char* getBowGetItemDemoReplayStatus() {
    return g_wwItemmdlApi.getBowGetItemDemoReplayStatus
         ? g_wwItemmdlApi.getBowGetItemDemoReplayStatus() : NULL;
}
}  // namespace dWwItemmdl
