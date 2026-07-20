/**
 * @file d_a_npc_henna0.cpp
 *
 * ExtNpc socket — HENNA0 multiplex (folk / knobs / props by socket_arg + pending bind).
 */

#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_npc_henna0.h"
#if TARGET_PC
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_stub_watch.h"
#include "dusk/logging.h"
#include "f_op/f_op_actor_mng.h"
#include <cstdio>
#include <cstring>
#endif

#if TARGET_PC
static int daNpc_Henna0_Create(void* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    fopAcM_ct(a, dExtNpcMount_c);
    const u32 params = fopAcM_GetParam(a);
    char proc[32] = {};
    char src[96] = {};
    char head[64] = {};
    char joint[32] = {};
    // №44: id-keyed pending beats global forceNext* (async create race → head theft).
    const bool pending = dExtNpcMount_takePendingSpawn(fopAcM_GetID(a), proc, sizeof(proc), src,
                                                       sizeof(src), head, sizeof(head), joint,
                                                       sizeof(joint));
    if (pending && proc[0]) {
        if (src[0]) {
            dExtNpcMount_forceNextSpawnSrc(src);
            // №51: stamp before create — forceNextSpawnSrc is cleared after the ledger line.
            std::snprintf(a->mSpawnSrc, sizeof(a->mSpawnSrc), "%s", src);
            if (std::strncmp(src, "door:", 5) == 0 && src[5]) {
                std::snprintf(a->mDoorKey, sizeof(a->mDoorKey), "%s", src + 5);
            }
        }
        if (head[0]) {
            dExtNpcMount_forceNextAttach(head, joint[0] ? joint : "head");
        }
        // №53-D: params mid-byte = doors.ini knob index (structural anti-scramble).
        if (std::strcmp(proc, "NPC_KNOB") == 0) {
            const int doorIdx = (int)((params >> 8) & 0xFF);
            if (doorIdx > 0) {
                dExtNpcDoors_stampKnobByIndex(a, doorIdx);
            }
        }
        DuskLog.info("[NPC_HENNA0] create params={:08x} → {} (pending)", params, proc);
        return dExtNpcMount_create(a, proc);
    }
    if (dExtNpcMount_consumeForcedCreateProc(proc, sizeof(proc)) ||
        dExtNpcMount_resolveSocket("NPC_HENNA0", (int)(params & 0xFF), proc, sizeof(proc))) {
        if (std::strcmp(proc, "NPC_KNOB") == 0) {
            const int doorIdx = (int)((params >> 8) & 0xFF);
            if (doorIdx > 0) {
                dExtNpcDoors_stampKnobByIndex(a, doorIdx);
            }
        }
        DuskLog.info("[NPC_HENNA0] create params={:08x} → {}", params, proc);
        return dExtNpcMount_create(a, proc);
    }
    // №38 E1: refuse = hole (never wildcard into Ganondorf / wrong folk).
    DuskLog.warn("[Spawn] src=socket:NPC_HENNA0 proc=REFUSED arg={} head=MISS:e1_no_exact",
                 (int)(params & 0xFF));
    return dStubWatch_refuseCreate(a, "NPC_HENNA0");
}

static BOOL daNpc_Henna0_Delete(void* param_0) {
    return dExtNpcMount_delete((dExtNpcMount_c*)param_0);
}

static BOOL daNpc_Henna0_Execute(void* param_0) {
    return dExtNpcMount_execute((dExtNpcMount_c*)param_0);
}

static BOOL daNpc_Henna0_Draw(void* param_0) {
    return dExtNpcMount_draw((dExtNpcMount_c*)param_0);
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
#if TARGET_PC
    /* Size         */ sizeof(dExtNpcMount_c),
#else
    /* Size         */ sizeof(d_a_npc_henna0),
#endif
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
