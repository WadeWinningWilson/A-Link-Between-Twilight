// ============================================================================
// ww_stage_loader.h — NATIVE ROOMS 3b: the STAGE-load dispatch seam.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// The sibling of ww_room_loader.h, for the other loader. The receiver's single
// stage-load call site (d_stage.cpp:3167) routes through this instead of
// calling dStage_dt_c_stageLoader directly. On a stage carrying no donor
// chunks — or with the kill switch off — it IS dStage_dt_c_stageLoader.
//
// WHY A SECOND SEAM WAS NEEDED
// The room seam (§Phase 2) covers room.dzr only. The lighting chunks are STAGE
// chunks, and §660 measured why they never arrive: the two lineages route the
// SAME four handlers under DIFFERENT tag names —
//
//     receiver   Env0 / Col0 / PAL0 / VRB0
//     donor      EnvR / Colo / Pale / Virt
//
// so a donor stage.dzs decodes to nothing for three of the four and the room's
// tevstr is fed no colour at all. That is the black island.
//
// WHY NOT SIMPLY ADD THE DONOR TAGS TO THE RECEIVER'S TABLE
// Because two of the three records have different layouts, and one differs
// eight-fold (EnvR: donor 0x8, receiver 0x41). Accepting the tag would hand
// donor bytes to receiver readers — §607's "silent data, not a crash", which is
// worse than the black screen it appears to fix. So the chunks are read at
// DONOR stride and materialised as RECEIVER records, translated BY FIELD.
// ============================================================================
#ifndef D_EXT_PLUGIN_WW_STAGE_LOADER_H
#define D_EXT_PLUGIN_WW_STAGE_LOADER_H

struct dStage_dt_c;

// The seam. Behaviourally identical to dStage_dt_c_stageLoader on any stage
// that carries no donor lighting chunks.
void dExtWwStage_loadStageDzs(void* i_data, dStage_dt_c* i_stage);

#endif
