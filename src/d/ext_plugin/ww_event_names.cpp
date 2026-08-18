// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_event_manager.cpp:53-92 (dEvent_exception_c::getEventName)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW ARRIVAL-EVENT NAME TABLE (tale §901) — the donor's own special-index
// table, verbatim.
//
// ROOT this closes (rows §899/§900): donor and receiver use the SAME index
// space for arrival events (donor `idx > 0xc8 && idx < 0xd6` -> table[idx-0xc9];
// receiver `200 < idx < 215` -> table[idx-201]) with DIFFERENT TABLES — WW's
// 13 entries open with FIVE *_COMEBACK rows, TP's 14 with one. 12 of 13 WW ids
// therefore resolved to the WRONG TP event. The ids are not stage data: the
// PLAYER maps PLYR `start_mode` ((param>>12)&0xF) to them (donor
// d_a_player_main.cpp:12302-12321), and that PLYR field is already translated
// donor-correct by the §636 seam — so correct modes arrived and TP's table
// mis-answered them. Measured Outset consequences (§900, disc-read): every
// interior door arrival (mode 0xB) asked for KNOB_START_B and got
// CRAWLOUT_START; sea r44 spawns 1/2/3/4/11 (0xA) asked KNOB_START and got
// FALL_START; the Great-Fairy fall (0xF) asked FALL_START and got
// PORTALWARP_START — an event whose staff nothing on Outset can man, so the
// event never advances and the player is stuck inside it.
//
// §865 parallel-systems + the shared-path law: this table answers ONLY on WW
// host stages (the seam in d_event_manager.cpp gates on
// dExtWwSave_isWwHostStage); TP's 14-entry table keeps answering for TP
// stages, untouched — the rule that caught №282/№283.
//
// DIALECT/SEAMS: [E1] donor `dStageType_BOSS_e` == receiver `ST_BOSS_ROOM`
// (both 3, same STType bitfield extract) — the 207 special case ports WITH the
// table, per the call's constraint (b). [E2] donor returns NULL where the
// receiver returns "(!NO REGIST!)"; the receiver's own consumer
// (dEvent_manager_c::getEventIdx:889) returns -1 on a NULL name, so the donor
// return is carried verbatim and is safe here — constraint (c). [E3] the
// receiver's TP-side 213->214 rewrite (SelectWarpPt 3) is NOT ported: WW 213 is
// FALL_START with its own meaning, and this function returns before that code.
// ============================================================

#include "d/ext_plugin/ww_stage_loader.h"

#include <cstring>

#include "d/d_com_inf_game.h"
#include "d/d_stage.h"

// ============================================================
// donor soecial_table (d_event_manager.cpp:54), 13 entries, ids 201..213.
// ============================================================
static const char* const l_wwArrivalEventNames[13] = {
    "NORMAL_COMEBACK",     // 201
    "MAGMA_COMEBACK",      // 202
    "TORNADO_COMEBACK",    // 203
    "TIMEWARP_COMEBACK",   // 204
    "SHIP_COMEBACK",       // 205
    "DEFAULT_START",       // 206
    "SHUTTER_START",       // 207 (boss stages -> BS_SHUTTER_START, below)
    "SHUTTER_START_STOP",  // 208
    "BS_SHUTTER_START",    // 209
    "KNOB_START",          // 210
    "KNOB_START_B",        // 211
    "FMASTER_START",       // 212
    "FALL_START",          // 213
};

// CONTRACT (one meaning for NULL): returns the donor name for a WW arrival id,
// or NULL when the id is OUTSIDE the donor's special-index space — the caller
// then uses the receiver's own map-event lookup (the same data path in
// receiver record shape). The donor's `idx == -1 -> NULL` case is handled by
// the seam BEFORE this call, so it never reaches here and NULL stays
// unambiguous.
const char* dExtWwEvt_getArrivalEventName(int i_eventInfoIdx) {
    // donor 207 special case, ported WITH the table (call constraint (b)).
    if (i_eventInfoIdx == 207) {
        stage_stag_info_class* stage_info = dComIfGp_getStageStagInfo();
        if (stage_info != NULL && dStage_stagInfo_GetSTType(stage_info) == ST_BOSS_ROOM) {
            return "BS_SHUTTER_START";  // [E1]
        }
    }

    // donor index test verbatim: idx > 0xc8 && idx < 0xd6 -> table[idx - 0xc9].
    if (i_eventInfoIdx > 0xc8 && i_eventInfoIdx < 0xd6) {
        return l_wwArrivalEventNames[i_eventInfoIdx - 0xc9];
    }

    return NULL;
}

// ============================================================
// WW STAGE-EVENT FULL NAMES — the 15→13 narrowing, answered at the boundary.
//
// The EVNT translator can only store 12 chars + NUL in the receiver's
// `event_name` arm, and 11 of sea's 57 donor names are longer. Two of them,
// `FROM_HYRULE_1` and `FROM_HYRULE_2`, truncate to the SAME string — so the
// loss is not merely cosmetic: two distinct events become one name, and a
// lookup cannot tell them apart.
//
// The receiver's own struct is not widened (0x1C is TP's on-disc layout, and
// TP data uses it) and the donor's names are not shortened (that is baking).
// Instead the seam SERVES the full name, keyed by the record index the
// receiver already has in hand — the same shape as the arrival table above and
// as `dMsg_resolveGroupArchive`.
//
// The names are the DONOR'S OWN BYTES, copied at publish time into a
// seam-owned pool. The translator's pool is static and stage-lived, but the
// stage.dzs buffer behind it is not this TU's to reason about, so the strings
// are copied rather than pointed at — the J3D pointer-fix law's habit applied
// to a much smaller thing.
//
// SCOPE, stated so nobody mistakes this for the whole fix: it answers the
// NAME-RETURNING path. A consumer that compares an incoming name against the
// receiver's stored (truncated) record — getEventIdx's own string compare —
// still sees 12 chars. That side is named on the task; this half is what
// getName needs and it lands independently.
// ============================================================
namespace {
const int kMaxWwStageEvents = 96;
const int kWwEventNameCap = 16;  // 15 donor bytes + NUL
char l_wwStageEventNames[kMaxWwStageEvents][kWwEventNameCap];
int l_wwStageEventCount = 0;
}  // namespace

void dExtWwEvt_publishStageEventNames(const char* i_names, int i_num, int i_stride) {
    l_wwStageEventCount = 0;
    if (i_names == NULL || i_num <= 0 || i_stride <= 0) {
        return;  // a stage with no EVNT publishes nothing; get() then returns NULL
    }
    const int n = (i_num < kMaxWwStageEvents) ? i_num : kMaxWwStageEvents;
    for (int i = 0; i < n; i++) {
        const char* src = i_names + (size_t)i * (size_t)i_stride;
        std::memcpy(l_wwStageEventNames[i], src, kWwEventNameCap - 1);
        l_wwStageEventNames[i][kWwEventNameCap - 1] = '\0';
    }
    l_wwStageEventCount = n;
}

const char* dExtWwEvt_getStageEventName(int i_eventInfoIdx) {
    if (i_eventInfoIdx < 0 || i_eventInfoIdx >= l_wwStageEventCount) {
        return NULL;  // not a published WW stage event — caller falls through
    }
    const char* name = l_wwStageEventNames[i_eventInfoIdx];
    return (name[0] != '\0') ? name : NULL;
}
