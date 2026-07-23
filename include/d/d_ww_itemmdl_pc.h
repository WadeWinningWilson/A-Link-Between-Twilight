#ifndef D_WW_ITEMMDL_PC_H

#define D_WW_ITEMMDL_PC_H



class J3DModelData;
class J3DModel;
struct dKy_tevstr_c;

#include "f_pc/f_pc_base.h"

struct request_of_phase_process_class;



#if TARGET_PC

void dWwItemmdl_debugLog(const char* message);

void dWwItemmdl_bracketLog(const char* message);

void dWwItemmdl_logHeap(const char* label);

bool dWwItemmdl_isPhase2BracketBow(u8 item_no);

bool dWwItemmdl_use2DIsolateHeap();

// Load vbow from private itemmdl.arc mount (2R); parse once onto retained heap.

J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name);

// Generic: load any itemmdl BDL by index (bow reuses its proven path). Entry point for new items.
J3DModelData* dWwItemmdl_getItemmdlModelData(u16 bdl_index);

void dWwItemmdl_patchModelForPc(J3DModelData* model_data);

// 2R: mount itemmdl on private dRes_info (not shared dComIfG slot). Returns cPhs_* or -1 if N/A.

int dWwItemmdl_stepPrivateItemmdlArcLoad(request_of_phase_process_class* phase);

// 2Q': arc / mpRawData audit (cache hit + draw entry before modelUpdateDL).

void dWwItemmdl_log2QPrimeAudit(const char* tag, J3DModelData* model_data, s32 room_no);

// Fix B step 1: TEV-order dump (material struct + optional runtime GX) for Vbow_v / SC_Vbow_v.
void dWwItemmdl_logTevOrderDump(J3DModelData* model_data, const char* phase, s32 room_no,
                                bool log_gx_runtime);

// Fix B step 2: per-draw GX TEV bind from Vbow_v struct (locked DL does not realize TevOrder).
void dWwItemmdl_applyTevOrderForDraw(J3DModelData* model_data);

// Fix B + 2B″: per-material texgen+TEV after mat callDL (see begin/end draw scope).
void dWwItemmdl_prepareWwBowGxForDraw(J3DModelData* model_data);

// Persistent mat-post-DL hook for WW bow demo item (set at spawn, clear at Delete / room change).
void dWwItemmdl_beginBowDrawScope(J3DModel* model, fpc_ProcID owner_id);
void dWwItemmdl_clearBowDrawScope();
// True when the held/get-item vbow is the loose rebuilt BMD (no MDL3) — caller
// should draw it boots-style (ambient-only, no scope). NEW CODE — ALBW Port.
bool dWwItemmdl_usingCustomHeldModel(J3DModel* i_model);
void dWwItemmdl_notifyRoomChange(s32 room_no);

// Track B: drive the private itemmdl arc mount each play tick while the held-bow skin is on,
// so setBowModel()'s getVbowModelData succeeds without a get-item replay first.
void dWwItemmdl_tickHeldBowArcMount();

// 4B/4D: actor ambient on Vbow_v/SC_Vbow_v only — no MAJI view lights (preserves cel detail).
void dWwItemmdl_setWwBowActorAmbient(dKy_tevstr_c* tevstr_p);
void dWwItemmdl_applyBowMaterialAmbientOnly(J3DModel* model, dKy_tevstr_c* tevstr_p);

// 4E: skip DEFAULT_GETITEM demo beam particles (flash/halo/star) during WW bow evaluation.
void dWwItemmdl_setWwBowGetItemBeamSuppress(bool suppress);
bool dWwItemmdl_shouldSuppressGetItemBeamParticle(u16 particle_id);

// 2N: per-draw GX texgen bind from Vbow_v (body material; SC max bind mis-samples).
void dWwItemmdl_applyTexGenForDraw(J3DModelData* model_data);

// 2N: two-pass draw — per-material texgen+TEV (SC_Vbow_v and Vbow_v disagree on GX state).
void dWwItemmdl_drawWwBowModel(J3DModel* model);

// 2S: shape count summary only (per-shape detail AVs at patchModel — do not iterate shapes).
void dWwItemmdl_logShapeInventory(J3DModelData* model_data, const char* phase);

// Hide SC_* outline for one draw without mutating shared arc-resident model state.

void dWwItemmdl_suppressOutlineForDraw(J3DModelData* model_data);

void dWwItemmdl_restoreOutlineAfterDraw(J3DModelData* model_data);

// 2P: release static outline suppress on demo-item delete (cached model outlives one spawn).

void dWwItemmdl_clearOutlineSuppress();

// 2Q/2R: demo item must not DeleteBase("itemmdl") on shared slot when true.

bool dWwItemmdl_retainItemmdlArcOnDemoItemDelete();

/**
 * Clothes get-item prop (Grandma handover / step 6).
 * Config: model_replacements/<mod>/getitem/clothes_bundle.ini
 * (host_item / arc / model / get_text / get_icon / hand_offset_* / max_scale —
 * no WW name literals in the exe). Optional overlay: clothes_bundle_text.ini.
 */
bool dWwItemmdl_clothesBundleForItem(u8 item_no);
/** Arc stem from config once loaded; nullptr if inactive. */
const char* dWwItemmdl_clothesBundleArcName();
/** Model member name from config; nullptr if inactive. */
const char* dWwItemmdl_clothesBundleModelName();
/**
 * Resolve finished ModelData from the already-mounted overlay arc.
 * Call only after dComIfG_resLoad of clothesBundleArcName() has completed.
 * Uses ExtNpcMount acquire (Ivan/Outset create path) — not raw getObjectRes.
 */
J3DModelData* dWwItemmdl_getClothesBundleModelData();

/** Drop cached ModelData before DeleteBase(unmount) of the clothes arc. */
void dWwItemmdl_clearClothesBundleCache();

/**
 * Get-box kit: if msgId is the host-item get message (host+0x65) and config
 * authored get_text, return that string (newlines already expanded). Else null.
 */
const char* dWwItemmdl_clothesBundleGetTextForMessage(u32 msg_id);

/**
 * Bytes needed for the staged get_icon ResTIMG (0 if no icon / wrong item).
 * Get-box alloc must be at least this — direct RGB5A3 48×48 is > 0xC00.
 */
u32 dWwItemmdl_clothesBundleIconCap(u8 item_no);

/** Copy get_icon BTI into out_buf. False → caller keeps vanilla path. */
bool dWwItemmdl_writeClothesBundleIconTimg(u8 item_no, void* out_buf, u32 out_cap);

/**
 * Hand-anchor offset for present-demo (player-local). True when config
 * authored hand_offset_*; fills *out. False → keep l_player_offset.
 */
bool dWwItemmdl_clothesBundleHandOffset(f32* out_x, f32* out_y, f32* out_z);

/** Present-demo max scale from config; 0 = leave actor default (grow to 1). */
f32 dWwItemmdl_clothesBundleMaxScale();

#endif



#endif /* D_WW_ITEMMDL_PC_H */

