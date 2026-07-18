/**
 * d_cut_actor_spawn.cpp — ALBW editor cut/stub actor spawn presets.
 *
 * Slice 1 (Cut Actors / Demo Restore run): event-scoped create-phase logging,
 * TestCube parked outside DEBUG, A5 display-label demotions. No param "fixes".
 */
#include "d/d_cut_actor_spawn.h"

#if TARGET_PC

#include <stdio.h>

#include "SSystem/SComponent/c_math.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/actor/d_a_e_dt.h"
#include "d/d_ext_npc_mount.h"
#include "dusk/logging.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"

namespace dCutActorSpawn {
namespace {

static const Entry kEntries[] = {
    // Best restore candidates (full code + assets on retail)
    // Labels: A5 day-one demotions / evidence aliases (display-only).
    {"E_ms — rat family (shipped) — CONFIRMED", fpcNm_E_MS_e, 0, 0xFFFF0000, true,
     "Shipped rat (playtest №2). params=0xFFFF0000 kill+enable sentinels."},
    {"E_dt — Deku Toad (shipped midboss)? (probable)", fpcNm_E_DT_e, 0, 0, true,
     "Opening demo teleports to world (0,4000,0). Cut spawn forces ACT_WAIT + feet pos."},
    {"E_S1 — Shadow Beast (shipped; loads E_S2)", fpcNm_E_S1_e, 0, 0xFFFFFFFF, true,
     "Retail E_S2 mesh. params=0xFFFFFFFF (solo/search/no-path/no-kill) — params=0 freezes/never aggros."},
    {"E_s1 — gen-2 proto (live AI)", fpcNm_E_S1_e, 0x0E51, 0xFFFFFFFF, true,
     "angle.x=0x0E51 → E_s1.arc + mapAnm; params=0xFFFFFFFF. Scale: WREG_F(29) on gen-2. Hang/shout degraded."},
    {"E_IS — Armos Titan (unused) — CONFIRMED", fpcNm_E_IS_e, 0, 0, true,
     "Visual confirm playtest №2. Snap to ground — air spawn can trap/death."},
    {"B_GO — Goron Golem boss", fpcNm_B_GO_e, 0, 0, true,
     "Composite: ~31 B_GOS children. Despawn cascades createChild lineage."},
    {"E_GS — ghost gatekeeper (Wolf Sense) — CONFIRMED", fpcNm_E_GS_e, 0, 0, true,
     "Playtest №5: visible under Wolf Sense. Opacity fades to 0 without it. D_MN09 R03/R09/R12."},
    {"E_OC — Bokoblin (club)", fpcNm_E_OC_e, 0, 0, true, "Shipping club bokoblin."},
    {"E_OC2 — shipped orc retexture + heavy cleaver — CONFIRMED", fpcNm_E_OC_e, 0x0100, 0, true,
     "39 retail placements (ToT/Ordeals/Field). rot.x byte → resLoad(E_OC2). Folk Moblin label dead."},
    {"Obj_Lv6bemos — ToT Beamos unused", fpcNm_Obj_Lv6bemos_e, 0, 0, true,
     "Cut Temple of Time beamos variant."},
    {"Obj_Lv6bemos2 — ToT Beamos shipped", fpcNm_Obj_Lv6bemos2_e, 0, 0, true,
     "Reference shipping ToT beamos."},
    {"Obj_Bemos — Field Beamos", fpcNm_Obj_Bemos_e, 0, 0, true, "Standard movable beamos."},
    {"Obj_TestCube — DEBUG-only (parked)", fpcNm_Obj_TestCube_e, 0, 0, true,
     "Methods are #if DEBUG only — RelWithDebInfo profile has NULL SubMtd (crash). "
     "Spawn refused outside DEBUG. Assets K_cube00/01 exist for later restore."},
    // Stub RELs — PARKED (playtest №1: NPC_MK procSize=1 → ACCESS_VIOLATION)
    {"NPC_MK — Ivan socket (Mk.arc; Plan R)", fpcNm_NPC_MK_e, 0, 0, true,
     "Socket: needs WW-Crew-Restoration arcs/Mk.arc. Absent → StubWatch ERROR. Present → L1 idle mount."},
    {"NPC_P2 — Medli socket (Plan R)", fpcNm_NPC_P2_e, 0, 0, true,
     "Socket: needs WW-Crew-Restoration arcs/P2.arc. Absent → StubWatch ERROR. Present → L1 idle mount."},
    {"NPC_KDK — cut NPC STUB (parked)", fpcNm_NPC_KDK_e, 0, 0, true,
     "PARKED: StubWatch-safe ERROR. Jailer experiment — retail R_SP107/R03 only."},
    {"NPC_HENNA0 — cut NPC STUB (parked)", fpcNm_NPC_HENNA0_e, 0, 0, true,
     "PARKED: StubWatch-safe ERROR (no assets)."},
};

static constexpr int kCount = (int)(sizeof(kEntries) / sizeof(kEntries[0]));
static constexpr int kTrackMax = 32;
static constexpr int kPhaseTimeoutFrames = 90;

static int s_selected = 0;
static char s_statusBuf[256] = "Pick a cut actor and Spawn at feet.";
static const char* s_status = s_statusBuf;
static fpc_ProcID s_tracked[kTrackMax];
static int s_trackedCount = 0;

struct PendingObserve {
    bool active;
    fpc_ProcID id;
    s16 actorId;
    u32 params;
    const char* label;
    int framesLeft;
    cXyz spawnPos;
};

static PendingObserve s_pending = {};

void setStatus(const char* msg) {
    if (msg == NULL) {
        s_statusBuf[0] = '\0';
    } else {
        snprintf(s_statusBuf, sizeof(s_statusBuf), "%s", msg);
    }
    s_status = s_statusBuf;
}

void trackId(fpc_ProcID id) {
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        return;
    }
    for (int i = 0; i < s_trackedCount; ++i) {
        if (s_tracked[i] == id) {
            return;
        }
    }
    if (s_trackedCount < kTrackMax) {
        s_tracked[s_trackedCount++] = id;
        return;
    }
    // Drop oldest when full.
    for (int i = 1; i < kTrackMax; ++i) {
        s_tracked[i - 1] = s_tracked[i];
    }
    s_tracked[kTrackMax - 1] = id;
}

bool isParkedStub(s16 actorId) {
    // MK/P2 unparked when Plan R payload present; KDK/HENNA0 stay refuse-spawn.
    if (actorId == fpcNm_NPC_MK_e) {
        return !dExtNpcMount_hasPayload("NPC_MK");
    }
    if (actorId == fpcNm_NPC_P2_e) {
        return !dExtNpcMount_hasPayload("NPC_P2");
    }
    return actorId == fpcNm_NPC_KDK_e || actorId == fpcNm_NPC_HENNA0_e;
}

int deleteAllOtama(void* actor, void* data) {
    fopAc_ac_c* ac = (fopAc_ac_c*)actor;
    int* deleted = (int*)data;
    if (ac != NULL && fopAcM_GetName(ac) == fpcNm_E_OT_e) {
        fopAcM_delete(ac);
        if (deleted != NULL) {
            ++(*deleted);
        }
    }
    return 0;
}

void finishPending(const char* phaseTag) {
    DuskLog.debug("[CutActorSpawn] phase={} id={:08x} actor={} params={:08x} label={}", phaseTag,
                  (u32)s_pending.id, (int)s_pending.actorId, s_pending.params,
                  s_pending.label != NULL ? s_pending.label : "?");
    if (phaseTag[0] == 'C' && s_pending.actorId == fpcNm_E_GS_e) {
        fopAc_ac_c* ac = fopAcM_SearchByID(s_pending.id);
        if (ac != NULL) {
            f32* opacity = (f32*)((u8*)ac + 0x5D8);
            DuskLog.debug("[CutActorSpawn] E_GS draw-probe opacity={} (Wolf Sense to sustain)",
                          *opacity);
        }
    }
    // E_dt opening demo hard-teleports to (0,4000,0) + spawns 20 otama — skip for cut tool.
    if (phaseTag[0] == 'C' && s_pending.actorId == fpcNm_E_DT_e) {
        daE_DT_c* dt = (daE_DT_c*)fopAcM_SearchByID(s_pending.id);
        if (dt != NULL) {
            int otamaDeleted = 0;
            fopAcIt_Executor(deleteAllOtama, &otamaDeleted);
            dt->setActionMode(0, 0);  // ACT_WAIT
            cXyz feet = s_pending.spawnPos;
            if (fopAcM_gc_c::gndCheck(&feet)) {
                feet.y = fopAcM_gc_c::getGroundY();
            }
            dt->current.pos = feet;
            dt->old.pos = feet;
            dt->home.pos = feet;
            dt->speed.y = 0.0f;
            dt->speedF = 0.0f;
            dt->shape_angle.x = 0;
            dt->gravity = -5.0f;
            DuskLog.debug("[CutActorSpawn] E_dt forced ACT_WAIT + ground snap y={}; deleted {} otama",
                          feet.y, otamaDeleted);
        }
    }
    char line[256];
    snprintf(line, sizeof(line), "Phase %s — id=%08x params=%08x (session log).", phaseTag,
             (u32)s_pending.id, s_pending.params);
    setStatus(line);
    s_pending.active = false;
}

struct ChildDeleteCtx {
    fpc_ProcID parents[kTrackMax];
    int parentCount;
    int deleted;
};

bool isTrackedParent(const ChildDeleteCtx* ctx, fpc_ProcID parentId) {
    for (int i = 0; i < ctx->parentCount; ++i) {
        if (ctx->parents[i] == parentId) {
            return true;
        }
    }
    return false;
}

int deleteChildrenOfTracked(void* actor, void* data) {
    fopAc_ac_c* ac = (fopAc_ac_c*)actor;
    ChildDeleteCtx* ctx = (ChildDeleteCtx*)data;
    if (ac == NULL || ctx == NULL) {
        return 0;
    }
    if (isTrackedParent(ctx, ac->parentActorID)) {
        fopAcM_delete(ac);
        ++ctx->deleted;
    }
    return 0;
}

}  // namespace

int entryCount() {
    return kCount;
}

const Entry* entry(int index) {
    if (index < 0 || index >= kCount) {
        return NULL;
    }
    return &kEntries[index];
}

int selectedIndex() {
    return s_selected;
}

void setSelectedIndex(int index) {
    if (index >= 0 && index < kCount) {
        s_selected = index;
    }
}

bool requestSpawn() {
    daAlink_c* player = (daAlink_c*)daPy_getPlayerActorClass();
    if (player == NULL) {
        setStatus("Failed — enter the field with a loaded save first.");
        return false;
    }

    const Entry& e = kEntries[s_selected];

#if !DEBUG
    if (e.actorId == fpcNm_Obj_TestCube_e) {
        setStatus("TestCube parked: Actor SubMtd is NULL outside DEBUG (would crash).");
        DuskLog.debug("[CutActorSpawn] refuse TestCube — non-DEBUG NULL methods");
        return false;
    }
#endif
    if (isParkedStub(e.actorId)) {
        setStatus("Stub parked: Size=0x1 or NULL SubMtd (Makar crash class) — spawn refused.");
        DuskLog.debug("[CutActorSpawn] refuse stub actor={} label={}", (int)e.actorId, e.label);
        return false;
    }

    cXyz pos = player->current.pos;
    pos.x += cM_ssin(player->shape_angle.y) * 150.0f;
    pos.z += cM_scos(player->shape_angle.y) * 150.0f;
    if (e.snapToGround && fopAcM_gc_c::gndCheck(&pos)) {
        pos.y = fopAcM_gc_c::mGroundY;
    }

    csXyz angle;
    angle.set(e.angleX, player->shape_angle.y, 0);
    cXyz scale(1.0f, 1.0f, 1.0f);
    const int roomNo = fopAcM_GetRoomNo(player);

    layer_class* savedLayer = fpcLy_CurrentLayer();
    base_process_class* playScene = fpcM_SearchByName(fpcNm_PLAY_SCENE_e);
    if (playScene != NULL) {
        fpcLy_SetCurrentLayer(&((process_node_class*)playScene)->layer);
    }

    const fpc_ProcID result =
        fopAcM_create(e.actorId, e.params, &pos, roomNo, &angle, &scale, -1);

    fpcLy_SetCurrentLayer(savedLayer);

    DuskLog.debug("[CutActorSpawn] create factory id={:08x} actor={} params={:08x} label={}",
                  (u32)result, (int)e.actorId, e.params, e.label);

    if (result == fpcM_ERROR_PROCESS_ID_e) {
        setStatus("Spawn failed (fopAcM_create returned ERROR).");
        return false;
    }

    trackId(result);
    s_pending.active = true;
    s_pending.id = result;
    s_pending.actorId = e.actorId;
    s_pending.params = e.params;
    s_pending.label = e.label;
    s_pending.framesLeft = kPhaseTimeoutFrames;
    s_pending.spawnPos = pos;
    setStatus("Spawned — waiting create-phase (COMPLEATE/ERROR)…");
    return true;
}

void requestDespawn() {
    int deleted = 0;
    int missing = 0;

    // Plan W2a: if Link's hang-bite keep points at a tracked cut actor, clear it
    // before delete — otherwise field_0x281c goes stale and every future latch fails.
    {
        daAlink_c* link = (daAlink_c*)daPy_getPlayerActorClass();
        if (link != NULL) {
            fopAc_ac_c* keep = link->field_0x281c.getActor();
            if (keep != NULL) {
                const fpc_ProcID keepId = fopAcM_GetID(keep);
                for (int i = 0; i < s_trackedCount; ++i) {
                    if (s_tracked[i] == keepId) {
                        link->resetWolfEnemyBiteAll();
                        DuskLog.debug("[CutActorSpawn] resetWolfEnemyBiteAll (latched tracked id={:08x})",
                                      (u32)keepId);
                        break;
                    }
                }
            }
        }
    }

    // Cascade createChild lineage (B_GO → B_GOS) before deleting parents.
    ChildDeleteCtx childCtx = {};
    childCtx.parentCount = 0;
    for (int i = 0; i < s_trackedCount && childCtx.parentCount < kTrackMax; ++i) {
        if (s_tracked[i] != fpcM_ERROR_PROCESS_ID_e) {
            childCtx.parents[childCtx.parentCount++] = s_tracked[i];
        }
    }
    if (childCtx.parentCount > 0) {
        fopAcIt_Executor(deleteChildrenOfTracked, &childCtx);
        deleted += childCtx.deleted;
        DuskLog.debug("[CutActorSpawn] despawn cascaded {} children", childCtx.deleted);
    }

    for (int i = 0; i < s_trackedCount; ++i) {
        const fpc_ProcID id = s_tracked[i];
        if (id == fpcM_ERROR_PROCESS_ID_e) {
            continue;
        }
        if (fpcM_SearchByID(id) != NULL || fpcM_IsCreating(id)) {
            fopAcM_delete(id);
            ++deleted;
        } else {
            ++missing;
        }
    }
    s_trackedCount = 0;
    s_pending.active = false;

    if (deleted == 0 && missing == 0) {
        setStatus("Nothing tracked to despawn.");
    } else if (missing == 0) {
        setStatus("Despawned tracked cut actors (+ children).");
    } else {
        setStatus("Despawned tracked actors (some were already gone).");
    }
}

void tick() {
    if (!s_pending.active) {
        return;
    }

    if (fpcM_IsCreating(s_pending.id)) {
        if (--s_pending.framesLeft <= 0) {
            finishPending("TIMEOUT");
        }
        return;
    }

    if (fpcM_SearchByID(s_pending.id) != NULL) {
        finishPending("COMPLEATE");
        return;
    }

    // Create finished and process is gone → create returned cPhs_ERROR and deleted.
    finishPending("ERROR");
}

int trackedCount() {
    return s_trackedCount;
}

const char* status() {
    return s_status != NULL ? s_status : "";
}

}  // namespace dCutActorSpawn

#endif  // TARGET_PC
