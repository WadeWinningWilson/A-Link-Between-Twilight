/**
 * @file d_a_npc_kdk.cpp
 *
 * WIP ExtNpc/BG socket path — restore after FPS_BISECT_A2.
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_npc_kdk.h"
#if TARGET_PC
#include "d/d_stub_watch.h"
#include "d/d_ext_npc_mount.h"
#endif

#if TARGET_PC
static int daNpc_Kdk_Create(void* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    fopAcM_ct(a, dExtNpcMount_c);
    char proc[32] = {};
    char src[96] = {};
    char head[64] = {};
    char joint[32] = {};
    // №44: id-keyed pending (BG pop / warp) beats global forceNext*.
    if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(a), proc, sizeof(proc), src, sizeof(src),
                                      head, sizeof(head), joint, sizeof(joint)) &&
        proc[0]) {
        if (src[0]) {
            dExtNpcMount_forceNextSpawnSrc(src);
        }
        if (head[0]) {
            dExtNpcMount_forceNextAttach(head, joint[0] ? joint : "head");
        }
        return dExtNpcMount_create(a, proc);
    }
    // №24 D1: BG warp pin; else resolve by socket_arg.
    if (dExtNpcMount_consumeForcedCreateProc(proc, sizeof(proc)) ||
        dExtNpcMount_resolveSocket("NPC_KDK", (int)(fopAcM_GetParam(a) & 0xFF), proc,
                                     sizeof(proc))) {
        return dExtNpcMount_create(a, proc);
    }
    return dStubWatch_refuseCreate((fopAc_ac_c*)i_this, "NPC_KDK");
}

static BOOL daNpc_Kdk_Delete(void* param_0) {
    return dExtNpcMount_delete((dExtNpcMount_c*)param_0);
}

static BOOL daNpc_Kdk_Execute(void* param_0) {
    return dExtNpcMount_execute((dExtNpcMount_c*)param_0);
}

static BOOL daNpc_Kdk_Draw(void* param_0) {
    return dExtNpcMount_draw((dExtNpcMount_c*)param_0);
}

static BOOL daNpc_Kdk_IsDelete(void* param_0) {
    return TRUE;
}

static DUSK_CONST actor_method_class daNpc_Kdk_MethodTable = {
    (process_method_func)daNpc_Kdk_Create,
    (process_method_func)daNpc_Kdk_Delete,
    (process_method_func)daNpc_Kdk_Execute,
    (process_method_func)daNpc_Kdk_IsDelete,
    (process_method_func)daNpc_Kdk_Draw,
};
#endif

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_NPC_KDK = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_NPC_KDK_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
#if TARGET_PC
    /* Size         */ sizeof(dExtNpcMount_c),
#else
    /* Size         */ sizeof(daNpcKdk_c),
#endif
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_NPC_KDK_e,
#if TARGET_PC
    /* Actor SubMtd */ &daNpc_Kdk_MethodTable,
#else
    /* Actor SubMtd */ NULL,
#endif
    // No CULL — large BG mounts must always draw when created.
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e | fopAcStts_UNK_0x4_e |
                           fopAcStts_UNK_0x2_e | fopAcStts_UNK_0x1_e,
#if TARGET_PC
    /* Group        */ fopAc_ENV_e,
#else
    /* Group        */ fopAc_NPC_e,
#endif
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#if TARGET_PC
// №57: dedicated Cave09 socket (№44 two-liner — one enum + one profile; one slot = one payload).
static int daExt_Bg10_Create(void* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    fopAcM_ct(a, dExtNpcMount_c);
    char proc[32] = {};
    char src[96] = {};
    char head[64] = {};
    char joint[32] = {};
    if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(a), proc, sizeof(proc), src, sizeof(src), head,
                                      sizeof(head), joint, sizeof(joint)) &&
        proc[0]) {
        if (src[0]) {
            dExtNpcMount_forceNextSpawnSrc(src);
        }
        if (head[0]) {
            dExtNpcMount_forceNextAttach(head, joint[0] ? joint : "head");
        }
        return dExtNpcMount_create(a, proc);
    }
    if (dExtNpcMount_consumeForcedCreateProc(proc, sizeof(proc)) && proc[0]) {
        return dExtNpcMount_create(a, proc);
    }
    return dExtNpcMount_create(a, "EXT_BG10");
}

static DUSK_CONST actor_method_class daExt_Bg10_MethodTable = {
    (process_method_func)daExt_Bg10_Create,
    (process_method_func)daNpc_Kdk_Delete,
    (process_method_func)daNpc_Kdk_Execute,
    (process_method_func)daNpc_Kdk_IsDelete,
    (process_method_func)daNpc_Kdk_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_EXT_BG10 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_EXT_BG10_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(dExtNpcMount_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_NPC_KDK_e,
    /* Actor SubMtd */ &daExt_Bg10_MethodTable,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e | fopAcStts_UNK_0x4_e |
                           fopAcStts_UNK_0x2_e | fopAcStts_UNK_0x1_e,
    /* Group        */ fopAc_ENV_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
#endif
