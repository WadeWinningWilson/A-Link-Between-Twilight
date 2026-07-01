#ifndef D_WW_ITEMMDL_PC_H

#define D_WW_ITEMMDL_PC_H



class J3DModelData;

struct request_of_phase_process_class;



#if TARGET_PC

void dWwItemmdl_debugLog(const char* message);

void dWwItemmdl_bracketLog(const char* message);

void dWwItemmdl_logHeap(const char* label);

bool dWwItemmdl_isPhase2BracketBow(u8 item_no);

bool dWwItemmdl_use2DIsolateHeap();

// Load vbow from private itemmdl.arc mount (2R); parse once onto retained heap.

J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name);

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

// Hide SC_* outline for one draw without mutating shared arc-resident model state.

void dWwItemmdl_suppressOutlineForDraw(J3DModelData* model_data);

void dWwItemmdl_restoreOutlineAfterDraw(J3DModelData* model_data);

// 2P: release static outline suppress on demo-item delete (cached model outlives one spawn).

void dWwItemmdl_clearOutlineSuppress();

// 2Q/2R: demo item must not DeleteBase("itemmdl") on shared slot when true.

bool dWwItemmdl_retainItemmdlArcOnDemoItemDelete();

#endif



#endif /* D_WW_ITEMMDL_PC_H */

