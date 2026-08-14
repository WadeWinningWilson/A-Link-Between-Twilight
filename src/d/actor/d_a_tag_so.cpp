// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_tag_so.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW SO-TAG (TAG_SO — NpcSo's zone marker, 6 Outset placements). Donor
// WHOLE: a passive param-carrying tag (zone id, radius in 100-unit steps
// with the 0xFF→1600 default, fan flag); consumers (NpcSo when it ports —
// the §-roster's one real cast gap) search it by proc name. SEAM [S1]:
// the donor debugDraw (dLib_debugDrawFan behind an HIO toggle) is
// debug-only — dropped with the HIO per the ls1 posture.
// ============================================================

#include "d/actor/d_a_tag_so_port.h"

#include "f_op/f_op_actor_mng.h"
#include "d/d_ext_ww_actor_shims.h"   // fopAcStts_UNK40000_e

#define fpcDwPi_TAG_SO_e fpcDwPi_E_RD_e

bool daTag_So_c::_execute() {
    return TRUE;
}

bool daTag_So_c::_draw() {
    // [S1] donor debug fan (HIO-gated) dropped with the HIO.
    return TRUE;
}

void daTag_So_c::getArg() {
    // receiver GetParamBit takes the ACTOR (reads param itself) — the §877
    // returned-error API shape, now a code_dialect row.
    m290 = fopAcM_GetParamBit(this, 0, 8);
    s32 paramRadius = fopAcM_GetParamBit(this, 8, 8);
    m298 = fopAcM_GetParamBit(this, 16, 8);
    if (paramRadius == 0xff) {
        mRadius = 1600.0f;
    } else {
        mRadius = paramRadius * 100;
    }
}

cPhs_Step daTag_So_c::_create() {
    fopAcM_ct(this, daTag_So_c);
    getArg();
    return cPhs_COMPLEATE_e;
}

bool daTag_So_c::_delete() {
    return TRUE;
}

static cPhs_Step daTag_SoCreate(void* i_this) {
    return ((daTag_So_c*)i_this)->_create();
}

static BOOL daTag_SoDelete(void* i_this) {
    return ((daTag_So_c*)i_this)->_delete();
}

static BOOL daTag_SoExecute(void* i_this) {
    return ((daTag_So_c*)i_this)->_execute();
}

static BOOL daTag_SoDraw(void* i_this) {
    return ((daTag_So_c*)i_this)->_draw();
}

static BOOL daTag_SoIsDelete(void*) {
    return TRUE;
}

static actor_method_class daTag_SoMethodTable = {
    (process_method_func)daTag_SoCreate,
    (process_method_func)daTag_SoDelete,
    (process_method_func)daTag_SoExecute,
    (process_method_func)daTag_SoIsDelete,
    (process_method_func)daTag_SoDraw,
};

extern actor_process_profile_definition g_profile_TAG_SO;

actor_process_profile_definition g_profile_TAG_SO = {
    // donor g_profile_TAG_SO (d_a_tag_so.cpp:100).
    fpcLy_CURRENT_e,           // Layer ID
    2,                         // List ID (donor 0x0002)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_TAG_SO_e,            // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daTag_So_c),        // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_TAG_SO_e,          // Draw Prio
    &daTag_SoMethodTable,      // Actor SubMtd
    fopAcStts_UNK40000_e,      // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_4_e,         // Cull Type
};
