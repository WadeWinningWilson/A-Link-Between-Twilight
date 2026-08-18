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

// ============================================================================
// §888 WW STORY-LAYER SELECTION (ww_layer_select.cpp) — donor
// dComIfG_play_c::getLayerNo verbatim on WW-owned reads. Consumed by the
// d_stage layer loaders on WW host stages ONLY; TP stages keep TP's own.
// ============================================================================
int dExtWw_getLayerNo(int i_roomNo);

// ============================================================================
// §901 WW ARRIVAL-EVENT NAMES (ww_event_names.cpp) — donor
// dEvent_exception_c::getEventName's special-index table, verbatim. NULL means
// "not a WW arrival id" (see the TU's contract note). Consumed by
// dEvent_exception_c::getEventName on WW host stages ONLY.
// ============================================================================
const char* dExtWwEvt_getArrivalEventName(int i_eventInfoIdx);

// ============================================================================
// WW STAGE-EVENT FULL NAMES — the 15→13 narrowing, answered the same way.
//
// The donor's EVNT name is 15 bytes; the receiver's `event_name` arm holds
// 12+NUL, and 11 of sea's 57 names exceed it — including `departure_DEMO`
// (Outset room 44, critical path) and, worst, `FROM_HYRULE_1`/`FROM_HYRULE_2`,
// which TRUNCATE TO THE SAME STRING: two distinct events collapsing into one
// name is worse than a shortened one.
//
// DN-10 order of resort: the receiver's 0x1C record is TP's on-disc layout and
// TP data uses it, so it is not widened; donor names are not shortened, which
// would be baking. Instead the full name is SERVED from the seam, keyed by the
// record index the receiver already passes — the `dMsg_resolveGroupArchive`
// shape this port already runs, at a site that is already WW-gated.
//
// publish: called by the EVNT translator with the donor's own 15-byte names.
// get:     NULL means "not a WW stage event" — the caller falls through to the
//          receiver's own record, exactly as the arrival hook does.
// ============================================================================
void dExtWwEvt_publishStageEventNames(const char* i_names, int i_num, int i_stride);
const char* dExtWwEvt_getStageEventName(int i_eventInfoIdx);

// The COMPARISON path's resolver. `getEventIdx` takes a record it already holds
// a POINTER to and looks its name up in the event list — where the names are
// full. Feeding it the truncated stored name fails to match. This maps the
// record BACK to its index by pointer identity (never by prefix: `FROM_HYRULE_1`
// and `_2` share their first 12 chars, so a prefix match would silently pick
// one) and returns the donor's full name, or NULL if the record is not one of
// ours — the same fall-through contract as the other two.
const char* dExtWwEvt_getStageEventNameForRecord(const void* i_record);

#endif
