// ============================================================================
// ww_room_loader.h — NATIVE ROOMS 3b, Phase 1: the room-load dispatch seam.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none (Phase 1 carries no donor code; Phase 2 will)
//
// One function. The receiver's single room-load call site
// (d_s_room.cpp phase_2) routes through it instead of calling
// dStage_dt_c_roomLoader directly. On a non-WW stage — or with the kill
// switch off — it IS dStage_dt_c_roomLoader, nothing more.
//
// Scope doc: docs/WW Linked/native-rooms-3b-scope.md (Phase 1).
// Audit that unblocked it: bus §607/§608 (Phase 0 discharged).
// ============================================================================
#ifndef D_EXT_PLUGIN_WW_ROOM_LOADER_H
#define D_EXT_PLUGIN_WW_ROOM_LOADER_H

struct dStage_dt_c;

// ============================================================================
// dExtWwRoom_loadRoomDzr — the seam.
//
//   i_data   room.dzr file image (offset-fixed by the loader it delegates to)
//   i_stage  the room's dStage_dt_c
//   i_roomNo room number (№86: pinned before PLYR decode)
//
// Phase 1 contract: BEHAVIOURALLY IDENTICAL to dStage_dt_c_roomLoader on every
// input. The only observable difference is one log line on WW host stages.
// ============================================================================
void dExtWwRoom_loadRoomDzr(void* i_data, dStage_dt_c* i_stage, int i_roomNo);

#endif
