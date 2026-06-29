#ifndef D_WW_ITEMMDL_PC_H
#define D_WW_ITEMMDL_PC_H

#if TARGET_PC
void dWwItemmdl_debugLog(const char* message);
// Load vbow from itemmdl.arc (handles raw BMD / BDL files the arc loader skips).
J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name);
void dWwItemmdl_patchModelForPc(J3DModelData* model_data);
// Hide SC_* outline for one draw without mutating shared arc-resident model state.
void dWwItemmdl_suppressOutlineForDraw(J3DModelData* model_data);
void dWwItemmdl_restoreOutlineAfterDraw(J3DModelData* model_data);
#endif

#endif /* D_WW_ITEMMDL_PC_H */
