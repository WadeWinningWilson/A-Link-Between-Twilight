/**
 * @file d_a_npc_henna0.cpp
 *
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_npc_henna0.h"
#if TARGET_PC
#include "d/d_stub_watch.h"
#endif

#if TARGET_PC
static int daNpc_Henna0_Create(void* i_this) {
    return dStubWatch_refuseCreate((fopAc_ac_c*)i_this, "NPC_HENNA0");
}

static BOOL daNpc_Henna0_Delete(void* param_0) {
    return TRUE;
}

static BOOL daNpc_Henna0_Execute(void* param_0) {
    return TRUE;
}

static BOOL daNpc_Henna0_Draw(void* param_0) {
    return TRUE;
}

static BOOL daNpc_Henna0_IsDelete(void* param_0) {
    return TRUE;
}

static DUSK_CONST actor_method_class daNpc_Henna0_MethodTable = {
    (process_method_func)daNpc_Henna0_Create,
    (process_method_func)daNpc_Henna0_Delete,
    (process_method_func)daNpc_Henna0_Execute,
    (process_method_func)daNpc_Henna0_IsDelete,
    (process_method_func)daNpc_Henna0_Draw,
};
#endif

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_NPC_HENNA0 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_NPC_HENNA0_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(d_a_npc_henna0),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_NPC_HENNA0_e,
#if TARGET_PC
    /* Actor SubMtd */ &daNpc_Henna0_MethodTable,
#else
    /* Actor SubMtd */ NULL,
#endif
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e | fopAcStts_CULL_e,
    /* Group        */ fopAc_NPC_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
