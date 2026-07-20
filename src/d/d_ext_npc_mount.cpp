/**
 * d_ext_npc_mount.cpp — Plan R generic external-NPC mount (L1 + Slice I lighting/blink).
 */
#include "d/d_ext_npc_mount.h"

#if TARGET_PC

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>

#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAttach.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DTransform.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/JMath/JMath.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_m3d.h"
#include "SSystem/SComponent/c_math.h"
#include <cmath>
#include "d/d_com_inf_game.h"
#include "d/d_stage.h"
#include "d/d_albw_dialogue.h"
#include "d/d_ext_mod_flags.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_population.h"
#include "d/d_ext_save_guard.h"
#include "d/d_item.h"
#include "d/d_item_data.h"
#include "d/d_bg_s_gnd_chk.h"
#include "d/d_drawlist.h"
#include "d/d_kankyo.h"
#include "d/d_meter2_info.h"
#include "d/d_s_play.h"
#include "global.h"
#include "d/actor/d_a_alink.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include <vector>
#include "JSystem/JKernel/JKRHeap.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_lib.h"
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_mtx.h"

// Live-tune NPC cel ambient offsets (additive on manifest amb_*).
// WREG_F(30)=R, WREG_F(31)=G, WREG_F(32)=B — e.g. -20 to pull bloom down.

namespace {

namespace fs = std::filesystem;

std::unordered_map<std::string, dExtNpcManifest> s_providers;
std::vector<std::string> s_providerOrder;  // stable UI order

// ModelData cache (GameHeap-pinned). Key = "arc/model" or "bg:arc/model".
// Actors hold their own J3DModel / McaMorf over shared cached ModelData — never
// store actor-solid-heap pointers in dRes slots (№18 / Layer-B lifetime).
// №73: NOT session-immortal — J3D is pointer-fixed into the archive buffer, so
// cache entries for an arc MUST be purged when that arc's last live mount
// resDeletes (room-lane unload). Shared arcs (Knob) use a live refcount.
std::unordered_map<std::string, J3DModelData*> s_modelDataCache;
// №50-C crash fix: J3D load pointer-fixes the dRes buffer in place. A second load of the
// same member (plain ko.bdl for Zill, then ko.bdl+ko02.bmt for Joel) AVs. Keep a pristine
// copy of each J3D2 blob before the first load and re-parse BMT variants from that.
std::unordered_map<std::string, std::vector<u8>> s_pristineJ3dRaw;
// №73: mounts that have reached COMPLEATE and still own a live resLoad of `arc`.
std::unordered_map<std::string, int> s_arcLiveCount;
// №73 sweep ("arcs/mounts are permanent" assumptions invalidated by room-lane):
// - s_modelDataCache / s_pristineJ3dRaw — FIXED (retain/release + purge on last).
// - s_bgMountIds — already cleared on room unload / releaseRoomLaneMount.
// - warm interiors — intentionally permanent for PINNED doors (refcount keeps cache).
// - pending-spawn FIFO — №64 already drain-on-bind; leave alone.
// - door exit flags — №68 clearExitKnobForProc on unload.

u32 readBeU32(const void* p) {
    const u8* b = static_cast<const u8*>(p);
    return (u32(b[0]) << 24) | (u32(b[1]) << 16) | (u32(b[2]) << 8) | u32(b[3]);
}

void ensurePristineJ3dRaw(const char* arc, const char* modelName, void* res) {
    if (arc == NULL || modelName == NULL || res == NULL) {
        return;
    }
    const std::string key = std::string(arc) + "/" + modelName;
    if (s_pristineJ3dRaw.find(key) != s_pristineJ3dRaw.end()) {
        return;
    }
    const u8* bytes = static_cast<const u8*>(res);
    if (bytes[0] != 'J' || bytes[1] != '3' || bytes[2] != 'D' || bytes[3] != '2') {
        return;
    }
    const u32 size = readBeU32(bytes + 8);
    const u32 blocks = readBeU32(bytes + 12);
    if (size < 0x20 || size > 64u * 1024u * 1024u || blocks == 0 || blocks > 256) {
        return;
    }
    std::vector<u8> copy(size);
    std::memcpy(copy.data(), res, size);
    s_pristineJ3dRaw.emplace(key, std::move(copy));
}
bool s_bgWarpPending = false;
char s_bgWarpProc[32] = {};
char s_lastBgProc[32] = {};
// №90: COMPLEATE on WW host may precede player — drain when player exists.
char s_interiorBootstrapProc[32] = {};
// №94: increments on play-scene recreate / restart; spawn latches key off this.
u32 s_worldGeneration = 1;
char s_worldGenStage[12] = {};
bool s_bgSpawnOverrideValid = false;
cXyz s_bgSpawnOverride;
bool s_bgSpawnFacingValid = false;
s16 s_bgSpawnFacing = 0;
std::unordered_map<std::string, fpc_ProcID> s_bgMountIds;
// №62 Phase D: proc → host room (same-stage stream). Empty = pinned-only.
std::unordered_map<std::string, int> s_roomLaneRooms;
std::unordered_map<int, std::string> s_roomLaneProcByRoom;
// Rooms currently claimed by an active room-lane enter (protected from RTBL kill).
bool s_roomLaneClaimed[0x40] = {};
// Prevent phase_3+phase_4 double objectSetCheck from minting two BG mounts.
bool s_roomLaneMountCreating[0x40] = {};
// №68: set before room teardown; poll/draw/attention skip until cleared.
bool s_roomLaneUnloading[0x40] = {};
char s_roomLaneUnloadingProc[32] = {};
// №58-B: after island COMPLEATE, cold-create same-stage interior BGs (first press = warm).
// №115: abort on heap fail; foreign host_stage / warm=0 skipped; one create per N frames.
bool s_warmInteriors = false;
int s_warmProviderIndex = 0;
int s_warmCooldown = 0;
static constexpr int kWarmCooldownFrames = 30;  // ~0.5s @60 — one provider per N frames
// entrySolidHeap size 0x120000 is doubled on TARGET_PC; keep margin for models/cache.
static constexpr s32 kWarmBgHeapNeed = 0x280000;
bool s_doorDemoLocked = false;

// One native window is shared by all external mounts.  It is deliberately never
// actor-owned: actors can despawn while the UI draw list is still processing.
#if TARGET_PC_NATIVE_UI
dALBWDialogue_c* s_mountDialogue = NULL;
#endif
dExtNpcMount_c* s_mountDialogueOwner = NULL;

const dCcD_SrcCyl s_mountCylSrc = {
    {{0x0, {{0x0, 0x0, 0x0}, {0, 0}, 0x79}},
     {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0},
     {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0},
     {0x0}},
    {{{0.0f, 0.0f, 0.0f}, 0.0f, 0.0f}},
};

// №27 N3: refuse cross-rig anims (e.g. kohead01_wait on ko.bdl).
// Model stem must be a prefix of the anim stem, and must not be a strict
// shorter prefix of a longer sibling (ko vs kohead).
bool animMatchesModel(const char* modelName, const char* animName) {
    if (modelName == NULL || animName == NULL || modelName[0] == '\0' || animName[0] == '\0') {
        return false;
    }
    auto stem = [](const char* s, char* out, size_t n) {
        size_t i = 0;
        while (s[i] && s[i] != '.' && i + 1 < n) {
            char c = s[i];
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
            out[i++] = c;
        }
        out[i] = '\0';
    };
    char mStem[64];
    char aStem[64];
    stem(modelName, mStem, sizeof(mStem));
    stem(animName, aStem, sizeof(aStem));
    const size_t mLen = std::strlen(mStem);
    if (mLen == 0 || std::strlen(aStem) < mLen) {
        return false;
    }
    for (size_t i = 0; i < mLen; ++i) {
        if (aStem[i] != mStem[i]) {
            return false;
        }
    }
    // Allow exact stem, stem_, or stem followed by digit — reject stem+letter that
    // starts a longer family (ko + head… → kohead).
    const char next = aStem[mLen];
    if (next == '\0' || next == '_' || (next >= '0' && next <= '9')) {
        return true;
    }
    return false;
}

// №47-A: WW body idles are often short names (`wait.bck`, `talk.bck`) — not model-prefixed.
// Keep N3's cross-rig refuse (kohead* on ko.bdl) but allow non-prefixed same-arc anims.
bool animAllowedOnBody(const char* modelName, const char* animName) {
    if (modelName == NULL || animName == NULL || modelName[0] == '\0' || animName[0] == '\0') {
        return false;
    }
    if (animMatchesModel(modelName, animName)) {
        return true;
    }
    auto stem = [](const char* s, char* out, size_t n) {
        size_t i = 0;
        while (s[i] && s[i] != '.' && i + 1 < n) {
            char c = s[i];
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
            out[i++] = c;
        }
        out[i] = '\0';
    };
    char mStem[64];
    char aStem[64];
    stem(modelName, mStem, sizeof(mStem));
    stem(animName, aStem, sizeof(aStem));
    const size_t mLen = std::strlen(mStem);
    if (mLen == 0 || aStem[0] == '\0') {
        return false;
    }
    // Anim stem begins with model stem but failed the digit/_ boundary ⇒ sibling family.
    if (std::strlen(aStem) >= mLen) {
        bool prefix = true;
        for (size_t i = 0; i < mLen; ++i) {
            if (aStem[i] != mStem[i]) {
                prefix = false;
                break;
            }
        }
        if (prefix) {
            return false;
        }
    }
    return true;  // e.g. ym.bdl + wait.bck
}

void setMountAnimation(dExtNpcMount_c* a, const char* name, u8 mode) {
    if (a == NULL || a->mpMorf == NULL || name == NULL || name[0] == '\0') {
        return;
    }
    // №37: door-open BCK may not match door.bdl name prefix — allow static/door mounts.
    // №47-A: WW short idle/talk names allowed; cross-rig still refused.
    if (!a->mManifest.isStatic && !a->mManifest.doorAttention &&
        !animAllowedOnBody(a->mManifest.model, name)) {
        DuskLog.warn("[ExtNpcMount] N3 skip anim '{}' for model '{}' (cross-rig)", name,
                     a->mManifest.model);
        return;
    }
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, name);
    if (anm != NULL) {
        a->mpMorf->setAnm(anm, mode, 3.0f, 1.0f, 0.0f, -1.0f, NULL);
    }
}

// №91: shared DoorK10 load (TP event archive for DEFAULT_KNOB_DOOR_*).
request_of_phase_process_class s_doorK10Phase;
int s_doorK10PhaseState = cPhs_INIT_e;  // COMPLEATE / ERROR once settled

bool pollDoorK10Ready() {
    if (s_doorK10PhaseState == cPhs_COMPLEATE_e) {
        return true;
    }
    if (s_doorK10PhaseState == cPhs_ERROR_e) {
        return false;
    }
    s_doorK10PhaseState = dComIfG_resLoad(&s_doorK10Phase, "DoorK10");
    if (s_doorK10PhaseState == cPhs_COMPLEATE_e) {
        DuskLog.info("[ExtNpcMount] №91 DoorK10 event archive ready");
        return true;
    }
    if (s_doorK10PhaseState == cPhs_ERROR_e) {
        DuskLog.warn("[ExtNpcMount] №91 DoorK10 load failed — falling back to №53 immediate warp");
        return false;
    }
    return false;
}

bool bindKnobDoorEvents(dExtNpcMount_c* a) {
    if (a == NULL || !a->mManifest.doorAttention) {
        return false;
    }
    if (a->mKnobEvBound) {
        return a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0;
    }
    if (!pollDoorK10Ready()) {
        return false;
    }
    a->eventInfo.setArchiveName("DoorK10");
    a->mKnobEvtFront =
        dComIfGp_getEventManager().getEventIdx(a, "DEFAULT_KNOB_DOOR_F_OPEN", 0xff);
    a->mKnobEvtBack =
        dComIfGp_getEventManager().getEventIdx(a, "DEFAULT_KNOB_DOOR_B_OPEN", 0xff);
    a->mKnobEvBound = 1;
    a->mKnobEvtOrdered = -1;
    a->mKnobStaffId = -1;
    a->mKnobDoorAction = 0;
    a->mKnobOpenStarted = 0;
    DuskLog.info("[ExtNpcMount] №91 knob events bound proc='{}' F={} B={}", a->mManifest.proc,
                 (int)a->mKnobEvtFront, (int)a->mKnobEvtBack);
    return a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0;
}

// daKnob20_c::frontCheck — 0 = front (F_OPEN), 1 = back (B_OPEN).
int knobFrontCheck(dExtNpcMount_c* a) {
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player == NULL || a == NULL) {
        return 0;
    }
    cXyz playerDist = player->current.pos - a->current.pos;
    mDoMtx_stack_c::YrotS(-a->current.angle.y);
    mDoMtx_stack_c::multVec(&playerDist, &playerDist);
    return playerDist.z > 0.0f ? 0 : 1;
}

void knobDoorSetEventPrm(dExtNpcMount_c* a) {
    if (a == NULL || !bindKnobDoorEvents(a)) {
        return;
    }
    const int side = knobFrontCheck(a);
    const s16 evt = (side == 0) ? a->mKnobEvtFront : a->mKnobEvtBack;
    if (evt < 0) {
        return;
    }
    a->mKnobEvtOrdered = evt;
    a->eventInfo.setArchiveName("DoorK10");
    a->eventInfo.setEventId(evt);
    a->eventInfo.setMapToolId(0xff);
    a->eventInfo.onCondition(dEvtCnd_CANDOOR_e);
}

int knobDoorGetDemoAction(dExtNpcMount_c* a) {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[16] = {
        "WAIT",
        "SETSTART",
        "SETANGLE",
        "ADJUSTMENT",
        "OPEN_PUSH",
        "OPEN_PULL",
        "OPEN_PUSH2",
        "OPEN_PULL2",
        "OPEN_PUSH_STOP",
        "OPEN_PULL_STOP",
        "TALK",
        "TALK_END",
        "SETSTART_PUSH",
        "SETSTART_PULL",
        "DEMO_OPEN",
        "DEMO_CLOSE",
    };
    return dComIfGp_evmng_getMyActIdx(a->mKnobStaffId, action_table, 16, 0, 0);
}

void knobDoorStartOpenAnim(dExtNpcMount_c* a) {
    if (a == NULL || a->mKnobOpenStarted || a->mpMorf == NULL) {
        return;
    }
    const char* bck = a->mManifest.doorOpenBck[0] ? a->mManifest.doorOpenBck : NULL;
    if (bck == NULL) {
        return;  // caller cutEnds when start leaves mKnobOpenStarted clear
    }
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, bck);
    if (anm == NULL) {
        DuskLog.warn("[ExtNpcMount] №97 door open BCK '{}' missing in arc '{}'", bck,
                     a->mManifest.arc);
        return;
    }
    a->mpMorf->setAnm(anm, J3DFrameCtrl::EMode_NONE, 0.0f, 1.0f, 0.0f, -1.0f, NULL);
    a->mKnobOpenStarted = 1;
    DuskLog.info("[ExtNpcMount] №97 door OPEN anim '{}' proc='{}'", bck, a->mManifest.proc);
    // №31: never play TP Z2SE_OBJ_KNOB_DOOR_* in WW spaces.
}

bool knobDoorOpenAnimDone(dExtNpcMount_c* a) {
    if (a == NULL || a->mpMorf == NULL) {
        return true;
    }
    // №97: never treat "not started yet" as done — that cutEnd'd OPEN before the BCK.
    if (!a->mKnobOpenStarted) {
        return false;
    }
    return a->mpMorf->isStop() != 0;
}

// №91/№97: door owns SHUTTER_DOOR staff cuts (WW door_open_bck on open actions).
void knobDoorDemoProc(dExtNpcMount_c* a) {
    if (a == NULL || a->mKnobStaffId == -1) {
        return;
    }
    const int demoAction = knobDoorGetDemoAction(a);
    if (dComIfGp_evmng_getIsAddvance(a->mKnobStaffId) != 0) {
        switch (demoAction) {
        case 4:  // OPEN_PUSH
        case 5:  // OPEN_PULL
        case 6:  // OPEN_PUSH2
        case 7:  // OPEN_PULL2
            knobDoorStartOpenAnim(a);
            break;
        default:
            break;
        }
    }
    switch (demoAction) {
    case 4:
    case 5:
    case 6:
    case 7:
        // Belt-and-suspenders: start even if advance was missed this frame.
        if (!a->mKnobOpenStarted) {
            knobDoorStartOpenAnim(a);
            if (!a->mKnobOpenStarted) {
                // No door_open_bck in manifest — don't stall the cut.
                dComIfGp_evmng_cutEnd(a->mKnobStaffId);
            }
            break;
        }
        if (knobDoorOpenAnimDone(a)) {
            dComIfGp_evmng_cutEnd(a->mKnobStaffId);
        }
        break;
    default:
        // SETSTART / ADJUSTMENT / WAIT / … — Link side is event-driven; door cuts through.
        dComIfGp_evmng_cutEnd(a->mKnobStaffId);
        break;
    }
}

void knobDoorInitOpenDemo(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
    a->mKnobStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
    a->mKnobOpenStarted = 0;
    a->mKnobDoorAction = 1;
    DuskLog.info("[ExtNpcMount] №91 knob demo BEGIN proc='{}' staff={} evt={}", a->mManifest.proc,
                 a->mKnobStaffId, (int)a->mKnobEvtOrdered);
}

// Returns true when the door finished its owned event and fired the warp.
bool tickKnobDoorEvent(dExtNpcMount_c* a) {
    if (a == NULL || !a->mManifest.doorAttention) {
        return false;
    }
    // Keep trying to bind while DoorK10 loads; until then №53 doorCheck fallback remains.
    bindKnobDoorEvents(a);

    if (a->mKnobDoorAction == 1) {
        dMeter2Info_onGameStatus(2);
        if (a->mKnobEvtOrdered >= 0 && dComIfGp_evmng_endCheck(a->mKnobEvtOrdered)) {
            a->mKnobDoorAction = 0;
            a->mKnobStaffId = -1;
            dComIfGp_event_reset();
            DuskLog.info("[ExtNpcMount] №91 knob demo END (cutEnd owned) → warp proc='{}'",
                         a->mManifest.proc);
            dExtNpcDoors_tryNativeWarp(a, /*openAlreadyDone=*/true);
            a->mKnobEvtOrdered = -1;
            a->mKnobOpenStarted = 0;
            return true;
        }
        a->mKnobStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
        knobDoorDemoProc(a);
        return true;
    }

    if (a->eventInfo.checkCommandDoor()) {
        if (a->mKnobEvtOrdered < 0) {
            // Event ordered without our bind (shouldn't happen) — let №53 path handle.
            return false;
        }
        knobDoorInitOpenDemo(a);
        knobDoorDemoProc(a);
        return true;
    }

    if (!dComIfGp_event_runCheck()) {
        if (a->mKnobEvBound && (a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0)) {
            knobDoorSetEventPrm(a);
        } else {
            // DoorK10 not ready / idxs missing — keep №53 CANDOOR so doorCheck warps.
            fopAc_ac_c* player = dComIfGp_getPlayer(0);
            if (player != NULL && (player->current.pos - a->current.pos).absXZ() < 300.0f) {
                a->eventInfo.onCondition(dEvtCnd_CANDOOR_e);
            }
        }
    }
    return a->mKnobEvBound != 0 && (a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0);
}

// №36 C: WW Vlupy tev frame (d_item_data mTevFrm) — item 1→0 … 6→5; silver 0x0F→6.
f32 vlupyColorFrame(u8 itemNo) {
    if (itemNo >= 1 && itemNo <= 6) {
        return (f32)(itemNo - 1);
    }
    if (itemNo == 0x0F) {
        return 6.0f;
    }
    return 0.0f;
}

// WW rupee item id → TP wallet item (silver differs: WW 0x0F → TP 0x07).
u8 tpRupeeGrantId(u8 wwItemNo) {
    if (wwItemNo >= dItemNo_GREEN_RUPEE_e && wwItemNo <= dItemNo_ORANGE_RUPEE_e) {
        return wwItemNo;
    }
    if (wwItemNo == 0x0F) {
        return (u8)dItemNo_SILVER_RUPEE_e;
    }
    return 0;
}

// №49 v4: REPLACE inside joint callback (param=0) BEFORE children recurse and
// BEFORE calcWeightEnvelopeMtx — Nintendo nodeCB pattern. Also patches
// j3dSys.mCurrentMtx so descendants + envelope see the new world mtx.
int extNpcSlaveJointCB(J3DJoint* joint, int timing) {
    if (timing != 0 || joint == NULL) {
        return 1;
    }
    J3DModel* model = j3dSys.getModel();
    if (model == NULL) {
        return 1;
    }
    dExtNpcMount_c* a = (dExtNpcMount_c*)model->getUserArea();
    if (a == NULL || a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        return 1;
    }
    J3DModel* body = a->mpMorf->getModel();
    const u16 jnt = joint->getJntNo();
    MtxP src = NULL;

    if (a->mpCompanion != NULL && model == a->mpCompanion->getModel()) {
        if (a->mSlavePairCount > 0) {
            for (int i = 0; i < a->mSlavePairCount; ++i) {
                if (a->mSlaveCompJnt[i] == (s16)jnt) {
                    src = body->getAnmMtx(a->mSlaveBodyJnt[i]);
                    break;
                }
            }
        } else if (jnt == 0) {
            s16 hostJnt = a->mNeckJnt;
            if (hostJnt < 0 && body->getModelData() != NULL) {
                JUTNameTab* names = body->getModelData()->getJointTree().getJointName();
                hostJnt = names != NULL ? (s16)names->getIndex("head") : (s16)-1;
            }
            if (hostJnt >= 0) {
                src = body->getAnmMtx(hostJnt);
            }
        }
    } else {
        for (int i = 0; i < a->mAttachCount; ++i) {
            if (a->mpAttach[i] != model || jnt != 0 || a->mAttachJnt[i] < 0 ||
                a->mAttachSlave[i] == 0) {
                continue;
            }
            J3DModel* host = body;
            if (a->mAttachOnCompanion[i] && a->mpCompanion != NULL &&
                a->mpCompanion->getModel() != NULL) {
                host = a->mpCompanion->getModel();
            }
            src = host->getAnmMtx(a->mAttachJnt[i]);
            break;
        }
    }

    if (src == NULL) {
        return 1;
    }
    // №50-A v5 PARENT-COMPOSE: world = body_target × companion local bind.
    // Identity-root heads unchanged; Zill's authored −90/−90 bind is preserved.
    Mtx local;
    const J3DTransformInfo& ti = joint->getTransformInfo();
    J3DGetTranslateRotateMtx(ti, local);
    if (ti.mScale.x != 1.0f || ti.mScale.y != 1.0f || ti.mScale.z != 1.0f) {
        JMAMTXApplyScale(local, local, ti.mScale.x, ti.mScale.y, ti.mScale.z);
    }
    Mtx world;
    MTXConcat(src, local, world);
    model->setAnmMtx(jnt, world);
    MTXCopy(world, j3dSys.mCurrentMtx);
    return 1;
}

void installSlaveJointCallbacks(J3DModel* model, dExtNpcMount_c* owner) {
    if (model == NULL || owner == NULL) {
        return;
    }
    model->setUserArea((uintptr_t)owner);
    J3DModelData* data = model->getModelData();
    if (data == NULL) {
        return;
    }
    for (u16 i = 0; i < data->getJointNum(); ++i) {
        J3DJoint* j = data->getJointNodePointer(i);
        if (j != NULL) {
            j->setCallBack(extNpcSlaveJointCB);
        }
    }
}

void resolveSlaveMapPairs(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
    a->mSlavePairCount = 0;
    if (a->mSlaveMap[0] == '\0' || a->mpCompanion == NULL || a->mpCompanion->getModel() == NULL ||
        a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        return;
    }
    J3DModel* body = a->mpMorf->getModel();
    J3DModel* slave = a->mpCompanion->getModel();
    J3DModelData* hostData = body->getModelData();
    J3DModelData* slaveData = slave->getModelData();
    if (hostData == NULL || slaveData == NULL) {
        return;
    }
    JUTNameTab* hostNames = hostData->getJointTree().getJointName();
    JUTNameTab* slaveNames = slaveData->getJointTree().getJointName();
    if (hostNames == NULL || slaveNames == NULL) {
        return;
    }
    const char* p = a->mSlaveMap;
    while (*p && a->mSlavePairCount < kExtNpcMaxSlavePairs) {
        while (*p == ' ' || *p == ',') {
            ++p;
        }
        if (*p == '\0') {
            break;
        }
        char compName[32] = {};
        char bodyName[32] = {};
        int ci = 0;
        while (*p && *p != ':' && *p != ',' && ci + 1 < (int)sizeof(compName)) {
            if (*p != ' ') {
                compName[ci++] = *p;
            }
            ++p;
        }
        compName[ci] = '\0';
        if (*p != ':') {
            break;
        }
        ++p;
        int bi = 0;
        while (*p && *p != ',' && bi + 1 < (int)sizeof(bodyName)) {
            if (*p != ' ') {
                bodyName[bi++] = *p;
            }
            ++p;
        }
        bodyName[bi] = '\0';
        if (compName[0] == '\0' || bodyName[0] == '\0') {
            continue;
        }
        const s32 cIdx = slaveNames->getIndex(compName);
        const s32 hIdx = hostNames->getIndex(bodyName);
        if (cIdx < 0 || cIdx >= slaveData->getJointNum() || hIdx < 0 ||
            hIdx >= hostData->getJointNum()) {
            DuskLog.warn("[ExtNpcMount] slave_map miss '{}'→'{}' (c={} h={})", compName, bodyName,
                         (int)cIdx, (int)hIdx);
            continue;
        }
        const u8 slot = a->mSlavePairCount++;
        a->mSlaveCompJnt[slot] = (s16)cIdx;
        a->mSlaveBodyJnt[slot] = (s16)hIdx;
    }
}

// №25 F2: snap to ground only when probe is within 500 of authored home Y.
// Rejects seafloor under the island while Outset dzb is still settling.
bool tryGroundSnapSanity(dExtNpcMount_c* a) {
    if (a == NULL || a->mIsBg || a->mGroundSnapped) {
        return a != NULL && a->mGroundSnapped;
    }
    cXyz probe = a->current.pos;
    probe.y = a->home.pos.y + 200.0f;
    if (!fopAcM_gc_c::gndCheck(&probe)) {
        if (++a->mGroundSnapTries > 90) {
            a->current.pos.y = a->home.pos.y;
            a->old.pos = a->current.pos;
            a->mGroundSnapped = true;
        }
        return a->mGroundSnapped;
    }
    const f32 gy = fopAcM_gc_c::getGroundY();
    if (gy < a->home.pos.y - 500.0f) {
        if (++a->mGroundSnapTries > 90) {
            // Island never reported — keep authored (never seat in salvage zone).
            a->current.pos.y = a->home.pos.y;
            a->old.pos = a->current.pos;
            a->mGroundSnapped = true;
            DuskLog.warn("[ExtNpcMount] ground snap rejected seafloor y={} for {} (keep {})",
                         gy, a->mManifest.proc, a->home.pos.y);
        }
        return a->mGroundSnapped;
    }
    a->current.pos.y = gy;
    a->old.pos = a->current.pos;
    a->mGroundSnapped = true;
    return true;
}

void closeMountDialogue(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
#if TARGET_PC_NATIVE_UI
    if (s_mountDialogue != NULL) {
        s_mountDialogue->hide();
    }
#endif
    if (a->mTalkEventActive) {
        dComIfGp_event_reset();
    }
    a->mTalking = false;
    a->mTalkEventActive = false;
    a->mTalkFrames = 0;
    a->mCloseCooldown = 12;
    if (s_mountDialogueOwner == a) {
        s_mountDialogueOwner = NULL;
    }
    setMountAnimation(a, a->mManifest.idle, J3DFrameCtrl::EMode_LOOP);
}

struct DialogueSectionParsed {
    std::string text;
    std::string ifFlag;
    std::string unlessFlag;
    std::string setFlag;
    std::string clearFlag;
    std::string next;
    std::string elseSection;
    std::string action;  // e.g. grant:62 or grant:0x3E
};

void applyDialogueAction(const dExtNpcMount_c* a, const std::string& action) {
    if (a == NULL || action.empty()) {
        return;
    }
    if (action.rfind("grant:", 0) == 0) {
        const char* spec = action.c_str() + 6;
        char* end = NULL;
        unsigned long id = std::strtoul(spec, &end, 0);
        if (end == spec || id > 255) {
            DuskLog.warn("[ExtNpcMount] bad grant action '{}'", action);
            return;
        }
        execItemGet(static_cast<u8>(id));
        DuskLog.info("[ExtNpcMount] grant item {} via '{}'", (int)id, a->mManifest.proc);
        return;
    }
    if (action.rfind("clear_flag:", 0) == 0) {
        dExtModFlags_set(a->mManifest.modFolder, action.c_str() + 11, false);
        return;
    }
    if (action.rfind("set_flag:", 0) == 0) {
        dExtModFlags_set(a->mManifest.modFolder, action.c_str() + 9, true);
        return;
    }
    DuskLog.warn("[ExtNpcMount] unknown dialogue action '{}'", action);
}

bool isDialogueDirective(const std::string& line, const char* key, std::string* valueOut) {
    const size_t keyLen = std::strlen(key);
    if (line.size() <= keyLen || line.compare(0, keyLen, key) != 0 || line[keyLen] != '=') {
        return false;
    }
    *valueOut = line.substr(keyLen + 1);
    return true;
}

bool loadDialogueSectionRaw(const dExtNpcMount_c* a, const char* sectionKey,
                            DialogueSectionParsed* out) {
    if (a == NULL || out == NULL || sectionKey == NULL || sectionKey[0] == '\0') {
        return false;
    }
    const fs::path dir = dusk::ConfigPath / "model_replacements" / a->mManifest.modFolder / "dialogue";
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        return false;
    }
    const std::string wanted = std::string("[") + sectionKey + "]";
    const auto trim = [](std::string& line) {
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' ||
                line.back() == '\t')) {
            line.pop_back();
        }
        const size_t first = line.find_first_not_of(" \t");
        line.erase(0, first == std::string::npos ? line.size() : first);
    };
    for (fs::directory_iterator it(dir, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec) || it->path().extension() != ".txt") {
            continue;
        }
        std::ifstream in(it->path());
        std::string line;
        bool inSection = false;
        DialogueSectionParsed parsed;
        bool found = false;
        while (std::getline(in, line)) {
            trim(line);
            if (!inSection) {
                inSection = line == wanted;
                continue;
            }
            if (!line.empty() && line[0] == '[') {
                break;
            }
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            found = true;
            std::string dirVal;
            if (isDialogueDirective(line, "if_flag", &dirVal)) {
                parsed.ifFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "unless_flag", &dirVal)) {
                parsed.unlessFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "set_flag", &dirVal)) {
                parsed.setFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "clear_flag", &dirVal)) {
                parsed.clearFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "next", &dirVal)) {
                parsed.next = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "else", &dirVal)) {
                parsed.elseSection = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "action", &dirVal)) {
                parsed.action = dirVal;
                continue;
            }
            // №27 N5: ww_ref=N pulls authentic BMG line from population/ww_dialogue_full.txt.
            if (isDialogueDirective(line, "ww_ref", &dirVal)) {
                const int idx = atoi(dirVal.c_str());
                const fs::path catalog = dusk::ConfigPath / "model_replacements" /
                                        a->mManifest.modFolder / "population" /
                                        "ww_dialogue_full.txt";
                std::ifstream cat(catalog);
                std::string catLine;
                char wanted[32];
                std::snprintf(wanted, sizeof(wanted), "[%d]", idx);
                while (std::getline(cat, catLine)) {
                    trim(catLine);
                    if (catLine.compare(0, std::strlen(wanted), wanted) != 0) {
                        continue;
                    }
                    std::string body = catLine.substr(std::strlen(wanted));
                    while (!body.empty() && (body[0] == ' ' || body[0] == '\t')) {
                        body.erase(0, 1);
                    }
                    // №32 B6 / №29 C3: honor literal "\n", real newlines, and legacy " / ".
                    std::string flat;
                    for (size_t i = 0; i < body.size(); ++i) {
                        if (body[i] == '\\' && i + 1 < body.size() &&
                            (body[i + 1] == 'n' || body[i + 1] == 'N')) {
                            flat += '\n';
                            ++i;
                            continue;
                        }
                        if (body[i] == '/' && (i == 0 || body[i - 1] == ' ') &&
                            i + 1 < body.size() && body[i + 1] == ' ') {
                            flat += '\n';
                            ++i;  // skip space after /
                            continue;
                        }
                        flat += body[i];
                    }
                    if (!parsed.text.empty()) {
                        parsed.text += '\n';
                    }
                    // Keep Speaker: prefix if the section already started one; else raw.
                    parsed.text += flat;
                    break;
                }
                continue;
            }
            if (!parsed.text.empty()) {
                parsed.text += '\n';
            }
            for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '\\' && i + 1 < line.size() && line[i + 1] == 'n') {
                    parsed.text += '\n';
                    ++i;
                } else {
                    parsed.text += line[i];
                }
            }
        }
        if (found) {
            *out = std::move(parsed);
            return true;
        }
    }
    return false;
}

bool sectionConditionsOk(const dExtNpcMount_c* a, const DialogueSectionParsed& sec) {
    const char* mod = a->mManifest.modFolder;
    if (!sec.ifFlag.empty() && !dExtModFlags_get(mod, sec.ifFlag.c_str())) {
        return false;
    }
    if (!sec.unlessFlag.empty() && dExtModFlags_get(mod, sec.unlessFlag.c_str())) {
        return false;
    }
    return true;
}

bool beginMountDialogue(dExtNpcMount_c* a, bool fromEvent) {
    if (a == NULL || a->mCloseCooldown > 0 || s_mountDialogueOwner != NULL) {
        return false;
    }
    const auto refuse = [&](const char* reason, bool playTalk) {
        DuskLog.warn("[ExtNpcMount] talk refused: {}", reason);
        if (playTalk) {
            setMountAnimation(a, a->mManifest.talk1, J3DFrameCtrl::EMode_LOOP);
        }
        if (fromEvent) {
            dComIfGp_event_reset();
        }
        a->mDialogueNext[0] = '\0';
        return false;
    };

    if (a->mDialogueSection[0] == '\0') {
        std::snprintf(a->mDialogueSection, sizeof(a->mDialogueSection), "%s",
                      a->mManifest.dialogueKey);
    }

    char key[64];
    std::snprintf(key, sizeof(key), "%s", a->mDialogueSection);
    DialogueSectionParsed sec;
    bool resolved = false;
    for (int depth = 0; depth < 8; ++depth) {
        DialogueSectionParsed loaded;
        if (!loadDialogueSectionRaw(a, key, &loaded)) {
            return refuse("dialogue section missing or empty", true);
        }
        if (!sectionConditionsOk(a, loaded)) {
            if (!loaded.elseSection.empty()) {
                std::snprintf(key, sizeof(key), "%s", loaded.elseSection.c_str());
                continue;
            }
            return refuse("dialogue gated by flag", true);
        }
        if (!loaded.setFlag.empty()) {
            dExtModFlags_set(a->mManifest.modFolder, loaded.setFlag.c_str(), true);
        }
        if (!loaded.clearFlag.empty()) {
            dExtModFlags_set(a->mManifest.modFolder, loaded.clearFlag.c_str(), false);
        }
        if (!loaded.action.empty()) {
            applyDialogueAction(a, loaded.action);
        }
        // Empty body + next= ⇒ silent hop (flag-only / router sections).
        if (loaded.text.empty() && !loaded.next.empty()) {
            std::snprintf(key, sizeof(key), "%s", loaded.next.c_str());
            continue;
        }
        sec = std::move(loaded);
        resolved = true;
        break;
    }
    if (!resolved || sec.text.empty()) {
        return refuse("dialogue section missing or empty", true);
    }

    if (dComIfGp_getMsgCommonArchive() == NULL) {
        return refuse("message archive unavailable", true);
    }
#if TARGET_PC_NATIVE_UI
    if (s_mountDialogue == NULL) {
        s_mountDialogue = JKR_NEW dALBWDialogue_c();
    }
    if (s_mountDialogue == NULL) {
        return refuse("dialogue allocation failed", false);
    }
    s_mountDialogue->tryCreate();
    if (!s_mountDialogue->isReady() || s_mountDialogue->isVisible()) {
        return refuse("dialogue window unavailable", false);
    }
    s_mountDialogue->showWithText(sec.text.c_str());
    if (!sec.next.empty()) {
        std::snprintf(a->mDialogueNext, sizeof(a->mDialogueNext), "%s", sec.next.c_str());
    } else {
        a->mDialogueNext[0] = '\0';
    }
    std::snprintf(a->mDialogueSection, sizeof(a->mDialogueSection), "%s", key);
    a->mTalking = true;
    a->mTalkEventActive = fromEvent;
    a->mTalkFrames = 0;
    s_mountDialogueOwner = a;
    setMountAnimation(a, a->mManifest.talk1, J3DFrameCtrl::EMode_LOOP);
    return true;
#else
    return refuse("native dialogue disabled", false);
#endif
}

// Match itemmdl BDL path: DoBdlMaterialCalc → readPatchedMaterial.
static constexpr u32 kExtNpcBdlFlags = 0x59020010u | 0x2000u;

void trimInPlace(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
        ++i;
    }
    if (i > 0) {
        s.erase(0, i);
    }
}

bool parseBoolVal(const std::string& val) {
    return val == "1" || val == "true" || val == "yes" || val == "on";
}

bool parseVec3(const std::string& value, cXyz* out) {
    return out != NULL &&
           sscanf(value.c_str(), "%f,%f,%f", &out->x, &out->y, &out->z) == 3;
}

bool parseManifestFile(const fs::path& path, const char* modFolder, dExtNpcManifest* out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    out->cylRadius = 40.0f;
    out->cylHeight = 100.0f;
    out->scale = 1.0f;
    out->socketArg = -1;
    out->hostRoom = 0;
    out->hostLayer = -1;
    out->colorFrame = -1.0f;  // №36 C: <0 ⇒ derive from create params / item id
    out->allowWarm = true;    // №115: warm=0 opts out of №58-B storm
    // Neutral gray ambient default (manifest may override).
    out->ambR = 90;
    out->ambG = 90;
    out->ambB = 90;
    snprintf(out->modFolder, sizeof(out->modFolder), "%s", modFolder);

    dExtNpcSubtype* subtype = NULL;
    std::string line;
    while (std::getline(in, line)) {
        trimInPlace(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            subtype = NULL;
            int index = -1;
            if (sscanf(line.c_str(), "[subtype.%d]", &index) == 1 &&
                index >= 0 && index < kExtNpcMaxSubtype) {
                subtype = &out->subtypes[index];
                subtype->valid = true;
                if (index + 1 > out->subtypeCount) {
                    out->subtypeCount = index + 1;
                }
            }
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trimInPlace(key);
        trimInPlace(val);
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
        }
        auto set = [&](char* dst, size_t n) { snprintf(dst, n, "%s", val.c_str()); };
        if (subtype != NULL) {
            if (key == "arg") {
                subtype->arg = atoi(val.c_str());
            } else if (key == "idle") {
                set(subtype->idle, sizeof(subtype->idle));
            } else if (key == "attach_model") {
                set(subtype->attachModel, sizeof(subtype->attachModel));
            } else if (key == "attach_joint") {
                set(subtype->attachJoint, sizeof(subtype->attachJoint));
            } else if (key == "display_name") {
                set(subtype->displayName, sizeof(subtype->displayName));
            }
        } else if (key == "proc") {
            set(out->proc, sizeof(out->proc));
        } else if (key == "socket") {
            set(out->socket, sizeof(out->socket));
        } else if (key == "socket_arg") {
            out->socketArg = atoi(val.c_str());
        } else if (key == "warp_label") {
            set(out->warpLabel, sizeof(out->warpLabel));
        } else if (key == "host_stage") {
            set(out->hostStage, sizeof(out->hostStage));
        } else if (key == "host_room") {
            out->hostRoom = atoi(val.c_str());
        } else if (key == "host_layer") {
            out->hostLayer = atoi(val.c_str());
        } else if (key == "host_pos") {
            if (parseVec3(val, &out->hostPos)) {
                out->hasHostPos = true;
            }
        } else if (key == "arc") {
            set(out->arc, sizeof(out->arc));
        } else if (key == "model") {
            set(out->model, sizeof(out->model));
        } else if (key == "model2") {
            set(out->model2, sizeof(out->model2));
        } else if (key == "model3") {
            set(out->model3, sizeof(out->model3));
        } else if (key == "model2_btk") {
            set(out->model2Btk, sizeof(out->model2Btk));
        } else if (key == "collision") {
            set(out->collision, sizeof(out->collision));
        } else if (key == "type") {
            out->isBg = val == "bg";
        } else if (key == "model_space") {
            out->modelSpaceLocal = (val == "local");
        } else if (key == "anchor") {
            if (parseVec3(val, &out->anchor)) {
                out->hasAnchor = true;
            }
        } else if (key == "spawn_rel") {
            if (parseVec3(val, &out->spawnRel)) {
                out->hasSpawnRel = true;
            }
        } else if (key == "spawn_ry" || key == "spawn_angle") {
            out->spawnRy = (s16)atoi(val.c_str());
            out->hasSpawnRy = true;
        } else if (key == "return_pos") {
            if (parseVec3(val, &out->returnPos)) {
                out->hasReturnPos = true;
            }
        } else if (key == "return_ry") {
            out->returnRy = (s16)atoi(val.c_str());
            out->hasReturnRy = true;
        } else if (key == "exit_door_rel") {
            if (parseVec3(val, &out->exitDoorRel)) {
                out->hasExitDoorRel = true;
            }
        } else if (key == "idle") {
            set(out->idle, sizeof(out->idle));
        } else if (key == "talk1") {
            set(out->talk1, sizeof(out->talk1));
        } else if (key == "talk2") {
            set(out->talk2, sizeof(out->talk2));
        } else if (key == "btp") {
            set(out->btp, sizeof(out->btp));
        } else if (key == "display_name") {
            set(out->displayName, sizeof(out->displayName));
        } else if (key == "neck_joint") {
            set(out->neckJoint, sizeof(out->neckJoint));
        } else if (key == "dialogue") {
            set(out->dialogueKey, sizeof(out->dialogueKey));
        } else if (key == "attach_model" || key == "attach_model2") {
            const int index = key == "attach_model2" ? 1 : 0;
            set(out->attach[index].model, sizeof(out->attach[index].model));
            if (index + 1 > out->attachCount) {
                out->attachCount = index + 1;
            }
        } else if (key == "attach_joint" || key == "attach_joint2") {
            const int index = key == "attach_joint2" ? 1 : 0;
            set(out->attach[index].joint, sizeof(out->attach[index].joint));
            if (index + 1 > out->attachCount) {
                out->attachCount = index + 1;
            }
        } else if (key == "companion_model") {
            set(out->companionModel, sizeof(out->companionModel));
        } else if (key == "companion_idle") {
            set(out->companionIdle, sizeof(out->companionIdle));
        } else if (key == "companion_mode") {
            // №36 A: joint_slave | synced (default)
            for (char& c : val) {
                if (c >= 'A' && c <= 'Z') {
                    c = (char)(c - 'A' + 'a');
                }
            }
            out->companionMode = (val == "joint_slave" || val == "slave" || val == "1") ? 1 : 0;
        } else if (key == "brk") {
            set(out->brk, sizeof(out->brk));
        } else if (key == "btk") {
            set(out->btk, sizeof(out->btk));
        } else if (key == "color_frame") {
            out->colorFrame = (f32)atof(val.c_str());
        } else if (key == "pickup_rupee") {
            out->pickupRupee = parseBoolVal(val);
        } else if (key == "door_open_bck") {
            set(out->doorOpenBck, sizeof(out->doorOpenBck));
        } else if (key == "cyl_radius") {
            out->cylRadius = (f32)atof(val.c_str());
        } else if (key == "cyl_height") {
            out->cylHeight = (f32)atof(val.c_str());
        } else if (key == "scale") {
            out->scale = (f32)atof(val.c_str());
            if (out->scale <= 0.01f) {
                out->scale = 1.0f;
            }
        } else if (key == "skip_btp") {
            out->skipBtp = parseBoolVal(val);
        } else if (key == "amb" || key == "ambient") {
            // amb=RRGGBB | #RRGGBB | R,G,B
            std::string v = val;
            if (!v.empty() && v[0] == '#') {
                v.erase(0, 1);
            }
            unsigned r = 90, g = 90, b = 90;
            if (v.find(',') != std::string::npos) {
                sscanf(v.c_str(), "%u,%u,%u", &r, &g, &b);
            } else if (v.size() >= 6) {
                sscanf(v.c_str(), "%02x%02x%02x", &r, &g, &b);
            }
            out->ambR = (u8)(r > 255 ? 255 : r);
            out->ambG = (u8)(g > 255 ? 255 : g);
            out->ambB = (u8)(b > 255 ? 255 : b);
        } else if (key == "amb_r") {
            out->ambR = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "amb_g") {
            out->ambG = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "amb_b") {
            out->ambB = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "source") {
            for (char& c : val) {
                if (c >= 'A' && c <= 'Z') {
                    c = (char)(c - 'A' + 'a');
                }
            }
            out->fromDvd = (val == "dvd" || val == "game" || val == "tp");
        } else if (key == "population") {
            set(out->populationCsv, sizeof(out->populationCsv));
        } else if (key == "population_stage") {
            set(out->populationStage, sizeof(out->populationStage));
        } else if (key == "spawn_if_flag") {
            set(out->spawnIfFlag, sizeof(out->spawnIfFlag));
        } else if (key == "spawn_unless_flag") {
            set(out->spawnUnlessFlag, sizeof(out->spawnUnlessFlag));
        } else if (key == "carryable") {
            out->carryable = parseBoolVal(val);
        } else if (key == "static") {
            out->isStatic = parseBoolVal(val);
        } else if (key == "codegen") {
            out->isCodeGeom = parseBoolVal(val);
        } else if (key == "door") {
            out->doorAttention = parseBoolVal(val);
        } else if (key == "door_visual") {
            set(out->doorVisual, sizeof(out->doorVisual));
        } else if (key == "body_bmt") {
            set(out->bodyBmt, sizeof(out->bodyBmt));
        } else if (key == "warm") {
            // №115: warm=0 / false — never №58-B cold-create this BG from the island.
            out->allowWarm = parseBoolVal(val);
        }
    }
    // №50-E: door controller (door.bdl) needs a visible variant; Nintendo default = door_a.
    if (out->doorAttention && out->doorVisual[0] == '\0') {
        snprintf(out->doorVisual, sizeof(out->doorVisual), "%s", "door_a.bdl");
    }
    // Phase M: no character dialogue defaults in code — require dialogue= or G1 refuse.
    if (out->socket[0] == '\0' && out->proc[0]) {
        snprintf(out->socket, sizeof(out->socket), "%s", out->proc);
    }
    // №32: static props/doors may omit idle (NULL anm). BG needs collision=.
    // №117: collision-only props (Akabe) — static + collision, model optional.
    if (out->isCodeGeom) {
        // №126: code-driven geometry (ported vegetation lane). No arc, no model
        // — the actor builds its own draw from an extracted asset pack, so the
        // only thing that must be present is the proc the spawner routes to.
        out->valid = out->proc[0] != '\0';
    } else if (out->isBg) {
        out->valid = out->proc[0] && out->arc[0] && out->collision[0] != '\0';
    } else {
        out->valid = out->proc[0] && out->arc[0] &&
                     (out->model[0] || (out->isStatic && out->collision[0])) &&
                     (out->idle[0] || out->isStatic);
    }
    return out->valid;
}

bool arcNameEqualsIgnoreCase(const std::string& stem, const char* arcName) {
    if (arcName == NULL || stem.size() != std::strlen(arcName)) {
        return false;
    }
    for (size_t i = 0; i < stem.size(); ++i) {
        char a = stem[i];
        char b = arcName[i];
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<char>(b - 'A' + 'a');
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

bool arcFilePresentInDir(const fs::path& arcsDir, const char* arcName) {
    std::error_code ec;
    if (!fs::is_directory(arcsDir, ec)) {
        return false;
    }
    const fs::path lower = arcsDir / (std::string(arcName) + ".arc");
    if (fs::is_regular_file(lower, ec)) {
        return true;
    }
    const fs::path upper = arcsDir / (std::string(arcName) + ".ARC");
    if (fs::is_regular_file(upper, ec)) {
        return true;
    }
    for (auto it = fs::directory_iterator(arcsDir, ec); it != fs::directory_iterator();
         it.increment(ec)) {
        if (ec || !it->is_regular_file(ec)) {
            continue;
        }
        const auto ext = it->path().extension().string();
        if (ext != ".arc" && ext != ".ARC") {
            continue;
        }
        if (arcNameEqualsIgnoreCase(it->path().stem().string(), arcName)) {
            return true;
        }
    }
    return false;
}

bool arcFilePresent(const fs::path& modRoot, const char* arcName) {
    if (arcName == NULL || arcName[0] == '\0') {
        return false;
    }
    // №52-C / №110: arcs/ first, then arcs_lib/ library fallback (never dump the lib).
    return arcFilePresentInDir(modRoot / "arcs", arcName) ||
           arcFilePresentInDir(modRoot / "arcs_lib", arcName);
}

bool shouldSkipModFolder(const std::string& modName) {
    // Bak / disabled twins must not scan as live providers (noisy "arcs missing" spam).
    if (modName.find(".SKELETON") != std::string::npos) {
        return true;
    }
    if (modName.size() >= 4 && modName.compare(modName.size() - 4, 4, "_BAK") == 0) {
        return true;
    }
    if (modName.size() >= 9 && modName.compare(modName.size() - 9, 9, ".DISABLED") == 0) {
        return true;
    }
    return false;
}

void stageLog(const char* stage, const char* detail) {
    DuskLog.info("[ExtNpcMount:D1] {} — {}", stage, detail != NULL ? detail : "");
}

// I1 (boots/leaf recipe — History №12c): WW lit channels ship litMask 0x03 (slots 0+1).
// TP actor/itemmdl host path only reliably fills slot 0; slot-1 garbage → black body
// (eyes stay bright because unlit). Working WW-boots overlay Kmdl_13.bmd uses 0x01.
// Clamp ENABLED channels to 0x01; leave unlit channels alone. Do NOT force 0xFF.
void applyActorLightMask(J3DModelData* modelData) {
    if (modelData == NULL) {
        return;
    }
    for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL || material->getColorChan(0) == NULL) {
            continue;
        }
        J3DColorChan* chan = material->getColorChan(0);
        if (chan->getEnable() == 0) {
            continue;  // unlit (eyes) — leave pristine
        }
        const u8 lightMask = chan->getLightMask();
        if (lightMask != 0x01) {
            chan->setLightMask(0x01);
            material->change();
        }
    }
}

// Load BDL/BMD into a fresh J3DModelData* without finish (shared-DL / light-mask).
J3DModelData* loadMountedModelDataOnly(void* res) {
    if (res == NULL) {
        stageLog("resolve", "FAIL res=NULL");
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    char magicBuf[64];
    snprintf(magicBuf, sizeof(magicBuf), "magic1=%08x magic2=%08x", (u32)header->mMagic1,
             (u32)header->mMagic2);
    stageLog("resolve", magicBuf);

    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3')) {
        stageLog("resolve", "path=loadBinaryDisplayList");
        return J3DModelLoaderDataBase::loadBinaryDisplayList(res, kExtNpcBdlFlags);
    }
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2')) {
        stageLog("resolve", "path=load (bmd)");
        return (J3DModelData*)J3DModelLoaderDataBase::load(res, 0x59020010);
    }
    stageLog("resolve", "path=preinstantiated J3DModelData*");
    J3DModelData* data = (J3DModelData*)res;
    if (data->getMaterialNum() == 0 || data->getMaterialNodePointer(0) == NULL) {
        stageLog("resolve", "FAIL preinstantiated model has no material[0]");
        return NULL;
    }
    return data;
}

J3DMaterialTable* loadMountedBmtTable(void* raw) {
    if (raw == NULL) {
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)raw;
    if (header->mMagic1 != 'J3D2') {
        return NULL;
    }
    if (header->mMagic2 == 'bmt3') {
        J3DModelLoader_v26 loader;
        return loader.loadMaterialTable(raw);
    }
    if (header->mMagic2 == 'bmt2') {
        J3DModelLoader_v21 loader;
        return loader.loadMaterialTable(raw);
    }
    return NULL;
}

// №50-C: apply body_bmt before finish. ko02.bmt is TEX1-only (shirt textures);
// other BMTs may also carry MAT3 overrides.
bool applyBodyBmtToModelData(J3DModelData* data, const char* arc, const char* bmtName) {
    if (data == NULL || arc == NULL || bmtName == NULL || bmtName[0] == '\0') {
        return false;
    }
    void* raw = dComIfG_getObjectRes(arc, bmtName);
    if (raw == NULL) {
        DuskLog.warn("[ExtNpcMount] body_bmt '{}' missing in arc '{}'", bmtName, arc);
        return false;
    }
    J3DMaterialTable* bmt = loadMountedBmtTable(raw);
    if (bmt == NULL) {
        DuskLog.warn("[ExtNpcMount] body_bmt '{}' unparseable", bmtName);
        return false;
    }
    u16 matCopied = 0;
    if (bmt->getMaterialNum() > 0) {
        JUTNameTab* srcNames = bmt->getMaterialName();
        JUTNameTab* dstNames = data->getMaterialName();
        for (u16 i = 0; i < bmt->getMaterialNum(); ++i) {
            J3DMaterial* src = bmt->getMaterialNodePointer(i);
            if (src == NULL) {
                continue;
            }
            s32 dstIdx = -1;
            if (srcNames != NULL && dstNames != NULL) {
                const char* n = srcNames->getName(i);
                if (n != NULL && n[0]) {
                    dstIdx = dstNames->getIndex(n);
                }
            }
            if (dstIdx < 0 && i < data->getMaterialNum()) {
                dstIdx = (s32)i;
            }
            if (dstIdx < 0 || dstIdx >= (s32)data->getMaterialNum()) {
                continue;
            }
            J3DMaterial* dst = data->getMaterialNodePointer((u16)dstIdx);
            if (dst == NULL) {
                continue;
            }
            dst->copy(src);
            ++matCopied;
        }
    }
    bool texReplaced = false;
    if (bmt->getTexture() != NULL) {
        data->getMaterialTable().replaceTextures(bmt->getTexture(), bmt->getTextureName());
        texReplaced = true;
    }
    DuskLog.info("[ExtNpcMount] body_bmt '{}' applied mats={} tex={}", bmtName, matCopied,
                 texReplaced ? "yes" : "no");
    return matCopied > 0 || texReplaced;
}

J3DModelData* finishMountedModelData(J3DModelData* modelData) {
    stageLog("finish", "enter");
    if (modelData == NULL) {
        stageLog("finish", "FAIL modelData=NULL");
        return NULL;
    }
    const u16 matNum = modelData->getMaterialNum();
    char buf[96];
    snprintf(buf, sizeof(buf), "matNum=%u jointNum=%u", matNum, modelData->getJointNum());
    stageLog("finish", buf);
    if (matNum == 0) {
        stageLog("finish", "FAIL matNum=0");
        return NULL;
    }
    J3DMaterial* mat0 = modelData->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("finish", "FAIL material[0]=NULL");
        return NULL;
    }

    applyActorLightMask(modelData);

    for (u16 i = 0; i < matNum; i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL) {
            snprintf(buf, sizeof(buf), "FAIL material[%u]=NULL", i);
            stageLog("finish", buf);
            return NULL;
        }
        material->change();
        J3DMaterialAnm* materialAnm = JKR_NEW J3DMaterialAnm();
        if (materialAnm == NULL) {
            stageLog("finish", "FAIL J3DMaterialAnm alloc");
            return NULL;
        }
        material->setMaterialAnm(materialAnm);
    }
    if (modelData->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        stageLog("finish", "FAIL newSharedDisplayList");
        return NULL;
    }
    modelData->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    modelData->makeSharedDL();
    stageLog("finish", "ok");
    return modelData;
}

J3DModelData* resolveMountedModelUncached(void* res) {
    J3DModelData* loaded = loadMountedModelDataOnly(res);
    if (loaded == NULL) {
        return NULL;
    }
    // Preinstantiated live ModelData: light-mask only (already finished elsewhere).
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    if (header != NULL && header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3' || header->mMagic2 == 'bmd3' ||
         header->mMagic2 == 'bmd2')) {
        return finishMountedModelData(loaded);
    }
    applyActorLightMask(loaded);
    return loaded;
}

// №26 F3: keys published during the current create — evicted on abort.
std::vector<std::string> s_createPublishedKeys;
bool s_createPublishTrack = false;

void beginCreateCacheTrack() {
    s_createPublishedKeys.clear();
    s_createPublishTrack = true;
}

void abortCreateCacheTrack() {
    if (!s_createPublishTrack) {
        return;
    }
    for (const std::string& key : s_createPublishedKeys) {
        auto it = s_modelDataCache.find(key);
        if (it != s_modelDataCache.end()) {
            DuskLog.warn("[ExtNpcMount] F3 cache eviction on abort: {}", key);
            s_modelDataCache.erase(it);
        }
    }
    s_createPublishedKeys.clear();
    s_createPublishTrack = false;
}

void commitCreateCacheTrack() {
    s_createPublishedKeys.clear();
    s_createPublishTrack = false;
}

// №73: keys for arc "LinkRM" are "LinkRM/…" or "bg:LinkRM/…".
static bool cacheKeyBelongsToArc(const std::string& key, const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return false;
    }
    const std::string bgPrefix = std::string("bg:") + arc + "/";
    const std::string npcPrefix = std::string(arc) + "/";
    return key.rfind(bgPrefix, 0) == 0 || key.rfind(npcPrefix, 0) == 0;
}

static void purgeModelCacheForArc(const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    int nModel = 0;
    for (auto it = s_modelDataCache.begin(); it != s_modelDataCache.end();) {
        if (cacheKeyBelongsToArc(it->first, arc)) {
            // Erase only — do not delete ModelData after/while archive dies (pointer-fixed
            // into the buffer; destructor would UAF). Matches F3 abort eviction.
            it = s_modelDataCache.erase(it);
            ++nModel;
        } else {
            ++it;
        }
    }
    int nRaw = 0;
    const std::string rawPrefix = std::string(arc) + "/";
    for (auto it = s_pristineJ3dRaw.begin(); it != s_pristineJ3dRaw.end();) {
        if (it->first.rfind(rawPrefix, 0) == 0) {
            it = s_pristineJ3dRaw.erase(it);
            ++nRaw;
        } else {
            ++it;
        }
    }
    DuskLog.info("[ExtNpcMount] №73 purged model cache for arc '{}' (models={} pristine={})",
                 arc, nModel, nRaw);
}

static void retainArcModels(const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    const int n = ++s_arcLiveCount[arc];
    DuskLog.info("[ExtNpcMount] №73 retain arc '{}' live={}", arc, n);
}

// №100: true if any live mount (except `dying`) still references this arc.
static bool liveMountRefsArc(const char* arc, const fopAc_ac_c* dying) {
    if (arc == NULL || arc[0] == '\0') {
        return false;
    }
    struct Ctx {
        const char* arc;
        const fopAc_ac_c* dying;
        bool hit;
    } ctx{arc, dying, false};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            Ctx* c = (Ctx*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL || ac == c->dying) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mManifest.arc[0] && std::strcmp(m->mManifest.arc, c->arc) == 0) {
                c->hit = true;
            }
            return 0;
        },
        &ctx);
    return ctx.hit;
}

// Call BEFORE dComIfG_resDelete. Shared arcs (Knob) only purge at last release.
// №100: `reason` names the caller; refuse purge while another live mount still refs the arc.
static void releaseArcModels(const char* arc, const char* reason, fopAc_ac_c* dying = NULL) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    const char* why = reason != NULL ? reason : "?";
    auto it = s_arcLiveCount.find(arc);
    if (it == s_arcLiveCount.end()) {
        if (liveMountRefsArc(arc, dying)) {
            DuskLog.warn(
                "[ExtNpcMount] №100 refuse purge arc '{}' — no retain but live mount refs "
                "(caller={})",
                arc, why);
            return;
        }
        DuskLog.info("[ExtNpcMount] №100 release arc '{}' (no retain) caller={} → purge", arc, why);
        purgeModelCacheForArc(arc);
        return;
    }
    --it->second;
    if (it->second > 0) {
        DuskLog.info("[ExtNpcMount] №73 release arc '{}' live={} (keep cache — shared) caller={}",
                     arc, it->second, why);
        return;
    }
    if (liveMountRefsArc(arc, dying)) {
        it->second = 1;  // restore — mount still drawing this arc
        DuskLog.warn(
            "[ExtNpcMount] №100 refuse purge arc '{}' — live mount still refs (caller={})", arc,
            why);
        return;
    }
    s_arcLiveCount.erase(it);
    DuskLog.info("[ExtNpcMount] №100 release arc '{}' live=0 caller={} → purge", arc, why);
    purgeModelCacheForArc(arc);
}

// GameHeap-pin + arc-scoped cache. Never mutates dRes slots; safe for N concurrent mounts.
// №26 F3: publish-on-success only; track keys for abort eviction.
// №50-C: optional body_bmt is part of the cache key so ko.bdl+ko02.bmt never poisons plain ko.bdl.
J3DModelData* acquireMountedModel(const char* arc, const char* modelName, void* res,
                                  const char* bodyBmt = NULL) {
    if (arc == NULL || modelName == NULL) {
        return NULL;
    }
    // Stash before any load — J3D parser pointer-fixes the dRes buffer in place.
    ensurePristineJ3dRaw(arc, modelName, res);

    std::string key = std::string(arc) + "/" + modelName;
    const bool wantBmt = bodyBmt != NULL && bodyBmt[0] != '\0';
    if (wantBmt) {
        key += "+";
        key += bodyBmt;
    }
    auto it = s_modelDataCache.find(key);
    if (it != s_modelDataCache.end()) {
        stageLog("resolve", "path=session-cache hit");
        return it->second;
    }

    JKRHeap* gameHeap = (JKRHeap*)mDoExt_getGameHeap();
    JKRHeap* prevHeap = gameHeap != NULL ? mDoExt_setCurrentHeap(gameHeap) : NULL;
    J3DModelData* data = NULL;
    if (wantBmt) {
        const std::string plainKey = std::string(arc) + "/" + modelName;
        auto pit = s_pristineJ3dRaw.find(plainKey);
        void* loadSrc = NULL;
        if (pit != s_pristineJ3dRaw.end()) {
            loadSrc = pit->second.data();
            stageLog("resolve", "path=bmt-load-from-pristine-copy");
        } else {
            // No pristine copy (already pointer-fixed before stash) — skip shirt, don't AV.
            DuskLog.warn(
                "[ExtNpcMount] body_bmt '{}' skipped — no pristine raw for {}/{} (would AV on "
                "re-parse)",
                bodyBmt, arc, modelName);
        }
        if (loadSrc != NULL) {
            data = loadMountedModelDataOnly(loadSrc);
            if (data != NULL) {
                applyBodyBmtToModelData(data, arc, bodyBmt);
                data = finishMountedModelData(data);
            }
        }
        if (data == NULL) {
            // Prefer already-finished plain body (never re-parse a pointer-fixed dRes buffer).
            const auto plainIt = s_modelDataCache.find(plainKey);
            if (plainIt != s_modelDataCache.end()) {
                data = plainIt->second;
                stageLog("resolve", "path=bmt-fallback-plain-cache");
            } else if (loadSrc != NULL) {
                data = resolveMountedModelUncached(loadSrc);
                stageLog("resolve", "path=bmt-fallback-plain-from-pristine");
            } else {
                data = resolveMountedModelUncached(res);
                stageLog("resolve", "path=bmt-fallback-plain-from-res");
            }
        }
    } else {
        data = resolveMountedModelUncached(res);
    }
    if (prevHeap != NULL) {
        mDoExt_setCurrentHeap(prevHeap);
    }
    if (data == NULL) {
        return NULL;
    }
    // Publish only after full resolve/finish succeeded.
    s_modelDataCache[key] = data;
    if (s_createPublishTrack) {
        s_createPublishedKeys.push_back(key);
    }
    DuskLog.info("[ExtNpcMount] model-data cache + {} heap={} (publish-on-success)", key,
                 gameHeap != NULL ? "GameHeap" : "?");
    return data;
}

// BG/room meshes: load raw J3D2 if dRes left the file unparsed (Outset BDLM path),
// but NEVER applyActorLightMask — that kills unlit/vertex-colored WW room mats.
J3DModelData* finishBgModelData(J3DModelData* modelData) {
    if (modelData == NULL || modelData->getMaterialNum() == 0 ||
        modelData->getMaterialNodePointer(0) == NULL) {
        return NULL;
    }
    for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL) {
            return NULL;
        }
        material->change();
        J3DMaterialAnm* materialAnm = JKR_NEW J3DMaterialAnm();
        if (materialAnm == NULL) {
            return NULL;
        }
        material->setMaterialAnm(materialAnm);
    }
    if (modelData->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        return NULL;
    }
    modelData->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    modelData->makeSharedDL();
    return modelData;
}

J3DModelData* resolveBgModelUncached(void* res) {
    if (res == NULL) {
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3')) {
        J3DModelData* loaded =
            J3DModelLoaderDataBase::loadBinaryDisplayList(res, kExtNpcBdlFlags);
        return finishBgModelData(loaded);
    }
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2')) {
        J3DModelData* loaded = (J3DModelData*)J3DModelLoaderDataBase::load(res, 0x59020010);
        return finishBgModelData(loaded);
    }
    // Already a live J3DModelData* from dRes — use as-is (no NPC light-mask).
    J3DModelData* data = (J3DModelData*)res;
    if (data->getMaterialNum() == 0 || data->getMaterialNodePointer(0) == NULL) {
        return NULL;
    }
    return data;
}

J3DModelData* acquireBgModel(const char* arc, const char* modelName, void* res) {
    if (arc == NULL || modelName == NULL) {
        return NULL;
    }
    const std::string key = std::string("bg:") + arc + "/" + modelName;
    auto it = s_modelDataCache.find(key);
    if (it != s_modelDataCache.end()) {
        return it->second;
    }
    JKRHeap* gameHeap = (JKRHeap*)mDoExt_getGameHeap();
    JKRHeap* prevHeap = gameHeap != NULL ? mDoExt_setCurrentHeap(gameHeap) : NULL;
    J3DModelData* data = resolveBgModelUncached(res);
    if (prevHeap != NULL) {
        mDoExt_setCurrentHeap(prevHeap);
    }
    if (data == NULL) {
        return NULL;
    }
    s_modelDataCache[key] = data;
    if (s_createPublishTrack) {
        s_createPublishedKeys.push_back(key);
    }
    DuskLog.info("[ExtNpcMount] BG model-data cache + {} (publish-on-success)", key);
    return data;
}

// №108: WW sea-stage sky (mod arcs/WwSky.arc → Object/WwSky.arc). Camera-follow
// like WW daVrbox/daVrbox2. TP stub vrbox stays hidden (N6).
static constexpr int kWwSkyModelCount = 4;
static const char* const kWwSkyModelNames[kWwSkyModelCount] = {
    "vr_sky.bdl",         // VRBOX dome
    "vr_uso_umi.bdl",     // false-sea horizon
    "vr_kasumi_mae.bdl",  // haze
    "vr_back_cloud.bdl",  // clouds (+100 Y in WW)
};
static request_of_phase_process_class s_wwSkyPhase;
static int s_wwSkyPhaseState = cPhs_INIT_e;
static J3DModel* s_wwSkyModels[kWwSkyModelCount] = {};
static bool s_wwSkyReady = false;
static int s_wwSkyUsers = 0;

static bool mountWantsWwSky(const dExtNpcMount_c* a) {
    if (a == NULL || !a->mIsBg || a->mManifest.proc[0] == '\0') {
        return false;
    }
    // Outdoor F_DL* hosts only (Outset / forest). Interiors keep hide_vrbox alone.
    if (std::strcmp(a->mManifest.proc, "EXT_BG0") != 0 &&
        std::strcmp(a->mManifest.proc, "EXT_BG9") != 0) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && dExtWwSave_isWwHostStage(stage) && stage[0] == 'F';
}

static void wwSkyRelease(const char* reason) {
    for (int i = 0; i < kWwSkyModelCount; ++i) {
        s_wwSkyModels[i] = NULL;
    }
    s_wwSkyReady = false;
    if (s_wwSkyPhaseState == cPhs_COMPLEATE_e) {
        purgeModelCacheForArc("WwSky");
        dComIfG_resDelete(&s_wwSkyPhase, "WwSky");
    }
    s_wwSkyPhase = {};
    s_wwSkyPhaseState = cPhs_INIT_e;
    DuskLog.info("[ExtNpcMount] №108 WwSky release ({})", reason != NULL ? reason : "?");
}

static bool wwSkyEnsure() {
    if (s_wwSkyReady) {
        return true;
    }
    s_wwSkyPhaseState = dComIfG_resLoad(&s_wwSkyPhase, "WwSky");
    if (s_wwSkyPhaseState != cPhs_COMPLEATE_e) {
        return false;
    }
    for (int i = 0; i < kWwSkyModelCount; ++i) {
        void* raw = dComIfG_getObjectRes("WwSky", kWwSkyModelNames[i]);
        J3DModelData* data = acquireBgModel("WwSky", kWwSkyModelNames[i], raw);
        if (data == NULL) {
            DuskLog.warn("[ExtNpcMount] №108 WwSky missing '{}'", kWwSkyModelNames[i]);
            wwSkyRelease("model-fail");
            return false;
        }
        // Same create flags as WW/TP vrbox (sky list / fog-friendly).
        s_wwSkyModels[i] = mDoExt_J3DModel__create(data, 0x80000, 0x11020202);
        if (s_wwSkyModels[i] == NULL) {
            DuskLog.warn("[ExtNpcMount] №108 WwSky create failed '{}'", kWwSkyModelNames[i]);
            wwSkyRelease("create-fail");
            return false;
        }
    }
    s_wwSkyReady = true;
    DuskLog.info("[ExtNpcMount] №108 WwSky ready (4 models)");
    return true;
}

static void wwSkyRetain() {
    if (s_wwSkyUsers++ == 0) {
        wwSkyEnsure();
    }
}

static void wwSkyReleaseUser() {
    if (s_wwSkyUsers <= 0) {
        return;
    }
    if (--s_wwSkyUsers == 0) {
        wwSkyRelease("last-user");
    }
}

// №121 Ask 1 / №116: WwSky is not TP's mpSoraModel — daVrbox_color_set never
// touches these. Drive TEV reg0 per frame like daVrbox_color_set (setCullMode +
// change + setTevColor), and call model->calc() before updateDL (island BG loop
// already does; prior №116 missed calc so material state never pushed).
// Bake stopgap RETRACTED — runtime path only. №113 VRB0 already feeds g_env_light.
static void wwSkyApplyTev(J3DModel* model, const GXColorS10& mat0, s16 mat0Alpha,
                          bool hasMat1Inner) {
    if (model == NULL) {
        return;
    }
    model->calc();
    J3DModelData* data = model->getModelData();
    if (data == NULL) {
        return;
    }
    J3DGXColorS10 color;
    J3DMaterial* m0 = data->getMaterialNodePointer(0);
    if (m0 != NULL) {
        m0->setCullMode(0);
        m0->change();
        color.r = mat0.r;
        color.g = mat0.g;
        color.b = mat0.b;
        color.a = mat0Alpha;
        m0->setTevColor(0, &color);
    }
    // Mirror daVrbox_color_set: material 1 = kasumi_inner when the dome has two mats.
    if (hasMat1Inner) {
        J3DMaterial* m1 = data->getMaterialNodePointer(1);
        if (m1 != NULL) {
            m1->setCullMode(0);
            m1->change();
            color.r = g_env_light.vrbox_kasumi_inner_col.r;
            color.g = g_env_light.vrbox_kasumi_inner_col.g;
            color.b = g_env_light.vrbox_kasumi_inner_col.b;
            color.a = g_env_light.vrbox_kasumi_inner_col.a;
            m1->setTevColor(0, &color);
        }
    }
}

static void wwSkyDraw() {
    if (!s_wwSkyReady || dComIfGd_getView() == NULL) {
        return;
    }
    f32 yOrigin = 0.0f;
    const s8 stay = dComIfGp_roomControl_getStayNo();
    if (stay >= 0) {
        dStage_roomDt_c* roomDt = dComIfGp_roomControl_getStatusRoomDt(stay);
        if (roomDt != NULL) {
            dStage_FileList_dt_c* fili = roomDt->getFileListInfo();
            if (fili != NULL) {
                yOrigin = dStage_FileList_dt_SeaLevel(fili);
            }
        }
    }
    const f32 yOff = (dComIfGd_getInvViewMtx()[1][3] - yOrigin) * 0.09f;
    const f32 cx = dComIfGd_getInvViewMtx()[0][3];
    const f32 cy = dComIfGd_getInvViewMtx()[1][3] - yOff;
    const f32 cz = dComIfGd_getInvViewMtx()[2][3];
    mDoMtx_stack_c::transS(cx, cy, cz);
    MtxP base = mDoMtx_stack_c::get();

    dComIfGd_setListSky();
    // WW order: sky → uso_umi → kasumi → back_cloud (+100 Y).
    // Colours: sky+uso ← vrbox_sky_col; kasumi ← vrbox_kasumi_inner_col; cloud ← kumo_top.
    if (s_wwSkyModels[0] != NULL) {
        s_wwSkyModels[0]->setBaseTRMtx(base);
        // №125 DIAGNOSTIC (one-shot, ~5s cadence): the converted VRB0 resolves
        // to sky (80,120,255) at daytime 225, but the dome renders pale/warm —
        // the tone of a DIFFERENT band. Print what the runtime actually chose so
        // the next playtest settles data-vs-application instead of inference.
        // Remove once the sky is confirmed.
        {
            static int s_skyDiagTick = 0;
            if ((s_skyDiagTick++ % 300) == 0) {
                DuskLog.info(
                    "[WwSky] №125 daytime={:.1f} sky=({},{},{}) kasumiIn=({},{},{}) "
                    "kumoTop=({},{},{}) hide_vrbox={}",
                    g_env_light.getDaytime(), g_env_light.vrbox_sky_col.r,
                    g_env_light.vrbox_sky_col.g, g_env_light.vrbox_sky_col.b,
                    g_env_light.vrbox_kasumi_inner_col.r, g_env_light.vrbox_kasumi_inner_col.g,
                    g_env_light.vrbox_kasumi_inner_col.b, g_env_light.vrbox_kumo_top_col.r,
                    g_env_light.vrbox_kumo_top_col.g, g_env_light.vrbox_kumo_top_col.b,
                    g_env_light.hide_vrbox ? 1 : 0);
            }
        }
        wwSkyApplyTev(s_wwSkyModels[0], g_env_light.vrbox_sky_col, 255, true);
        mDoExt_modelUpdateDL(s_wwSkyModels[0]);
    }
    if (s_wwSkyModels[1] != NULL) {
        s_wwSkyModels[1]->setBaseTRMtx(base);
        wwSkyApplyTev(s_wwSkyModels[1], g_env_light.vrbox_sky_col, 255, false);
        mDoExt_modelUpdateDL(s_wwSkyModels[1]);
    }
    if (s_wwSkyModels[2] != NULL) {
        s_wwSkyModels[2]->setBaseTRMtx(base);
        wwSkyApplyTev(s_wwSkyModels[2], g_env_light.vrbox_kasumi_inner_col,
                      g_env_light.vrbox_kasumi_inner_col.a, false);
        mDoExt_modelUpdateDL(s_wwSkyModels[2]);
    }
    if (s_wwSkyModels[3] != NULL) {
        mDoMtx_stack_c::transS(cx, cy + 100.0f, cz);
        s_wwSkyModels[3]->setBaseTRMtx(mDoMtx_stack_c::get());
        wwSkyApplyTev(s_wwSkyModels[3], g_env_light.vrbox_kumo_top_col,
                      g_env_light.vrbox_kumo_top_col.a, false);
        mDoExt_modelUpdateDL(s_wwSkyModels[3]);
    }
    dComIfGd_setList();
}

bool tryBindBtp(dExtNpcMount_c* a, J3DModelData* data) {
    a->mpBtp = NULL;
    a->mBtpBound = false;
    if (a->mManifest.skipBtp || a->mManifest.btp[0] == '\0') {
        stageLog("btp", "skipped (skip_btp or no btp=)");
        return true;
    }
    J3DAnmTexPattern* pat =
        (J3DAnmTexPattern*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.btp);
    if (pat == NULL) {
        DuskLog.warn("[ExtNpcMount] btp '{}' missing in arc '{}' — blink deferred",
                     a->mManifest.btp, a->mManifest.arc);
        stageLog("btp", "FAIL res NULL (cosmetic defer)");
        return true;  // non-fatal
    }
    a->mpBtp = JKR_NEW mDoExt_btpAnm();
    if (a->mpBtp == NULL) {
        stageLog("btp", "FAIL alloc");
        return true;
    }
    // Loop blink: attribute EMode_LOOP (2) like npc_ks relief path.
    if (a->mpBtp->init(data, pat, 1, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1) == 0) {
        DuskLog.warn("[ExtNpcMount] btp '{}' bind failed — blink deferred", a->mManifest.btp);
        stageLog("btp", "FAIL init (cosmetic defer)");
        a->mpBtp = NULL;
        return true;
    }
    a->mBtpBound = true;
    stageLog("btp", "bound ok");
    return true;
}

void applyModelAmbient(J3DModel* model, const GXColor& amb) {
    if (model == NULL || model->getModelData() == NULL) {
        return;
    }
    J3DModelData* data = model->getModelData();
    for (u16 i = 0; i < data->getMaterialNum(); ++i) {
        J3DMaterial* mat = data->getMaterialNodePointer(i);
        if (mat != NULL) {
            mat->change();
            mat->setAmbColor(0, (J3DGXColor*)&amb);
        }
    }
}

void logJointNames(J3DModelData* data) {
    JUTNameTab* names = data != NULL ? data->getJointTree().getJointName() : NULL;
    if (names == NULL) {
        DuskLog.warn("[ExtNpcMount] body has no joint-name table");
        return;
    }
    for (u16 i = 0; i < data->getJointNum(); ++i) {
        DuskLog.warn("[ExtNpcMount] body joint[{}]={}", i, names->getName(i));
    }
}

bool addAttachment(dExtNpcMount_c* a, J3DModelData* bodyData, const dExtNpcAttachSpec& spec) {
    if (spec.model[0] == '\0' || a->mAttachCount >= kExtNpcMaxAttach) {
        return true;
    }
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, spec.model);
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, spec.model, raw);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' missing/unparseable in arc '{}'", spec.model,
                     a->mManifest.arc);
        return true;
    }

    // Prefer companion joint table when present.
    J3DModelData* jointHost = bodyData;
    bool onCompanion = false;
    if (a->mpCompanion != NULL && a->mpCompanion->getModel() != NULL) {
        J3DModelData* companionData = a->mpCompanion->getModel()->getModelData();
        JUTNameTab* cNames = companionData != NULL ? companionData->getJointTree().getJointName() : NULL;
        const char* joint = spec.joint[0] ? spec.joint : "head";
        if (cNames != NULL && cNames->getIndex(joint) >= 0) {
            jointHost = companionData;
            onCompanion = true;
        }
    }

    JUTNameTab* names = jointHost->getJointTree().getJointName();
    const char* joint = spec.joint[0] ? spec.joint : "head";
    const s32 jointIndex = names != NULL ? names->getIndex(joint) : -1;
    if (jointIndex < 0 || jointIndex >= jointHost->getJointNum()) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' skipped: joint '{}' not found", spec.model, joint);
        logJointNames(jointHost);
        if (jointHost != bodyData) {
            logJointNames(bodyData);
        }
        return true;
    }
    J3DModel* model = mDoExt_J3DModel__create(data, 0, 1);
    if (model == NULL) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' model allocation failed", spec.model);
        return true;
    }
    const int slot = a->mAttachCount++;
    a->mpAttach[slot] = model;
    a->mAttachJnt[slot] = (s16)jointIndex;
    a->mAttachOnCompanion[slot] = onCompanion ? 1 : 0;
    a->mAttachSlave[slot] = 1;
    // №49/№50: base = host BASE; parent-compose via joint callback during calc (envelope-safe).
    J3DModel* hostModel =
        onCompanion ? a->mpCompanion->getModel() : a->mpMorf->getModel();
    model->setBaseTRMtx(hostModel->getBaseTRMtx());
    installSlaveJointCallbacks(model, a);
    DuskLog.info("[ExtNpcMount] attachment '{}' joint_slave → {} ({}) on {}", spec.model, joint,
                 jointIndex, onCompanion ? "companion" : "body");
    return true;
}

// №50-E: Nintendo knob = door.bdl controller (BCK + DoorDummy) + visible door_a..h at DoorDummy.
bool addDoorVisual(dExtNpcMount_c* a, J3DModelData* bodyData) {
    if (a == NULL || bodyData == NULL || !a->mManifest.doorAttention ||
        a->mManifest.doorVisual[0] == '\0' || a->mAttachCount >= kExtNpcMaxAttach) {
        return true;
    }
    JUTNameTab* names = bodyData->getJointTree().getJointName();
    const s32 jointIndex = names != NULL ? names->getIndex("DoorDummy") : -1;
    if (jointIndex < 0 || jointIndex >= bodyData->getJointNum()) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' skipped: DoorDummy joint missing",
                     a->mManifest.doorVisual);
        logJointNames(bodyData);
        return true;
    }
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.doorVisual);
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, a->mManifest.doorVisual, raw);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' missing/unparseable in arc '{}'",
                     a->mManifest.doorVisual, a->mManifest.arc);
        return true;
    }
    J3DModel* model = mDoExt_J3DModel__create(data, 0, 1);
    if (model == NULL) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' model allocation failed",
                     a->mManifest.doorVisual);
        return true;
    }
    const int slot = a->mAttachCount++;
    a->mpAttach[slot] = model;
    a->mAttachJnt[slot] = (s16)jointIndex;
    a->mAttachOnCompanion[slot] = 0;
    a->mAttachSlave[slot] = 0;  // base@DoorDummy only — no joint CB
    if (a->mpMorf != NULL && a->mpMorf->getModel() != NULL) {
        model->setBaseTRMtx(a->mpMorf->getModel()->getAnmMtx((u16)jointIndex));
    }
    DuskLog.info("[ExtNpcMount] door_visual '{}' @ DoorDummy ({})", a->mManifest.doorVisual,
                 jointIndex);
    return true;
}

int useHeapInit(fopAc_ac_c* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;

    // №117: collision-only static prop (Akabe.arc = dzb, no bdl). Invisible wall.
    if (a->mManifest.model[0] == '\0' && a->mManifest.isStatic && a->mManifest.collision[0]) {
        stageLog("heap", "collision-only (no model)");
        void* dzb = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.collision);
        if (dzb == NULL) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only '{}' missing in '{}'",
                         a->mManifest.collision, a->mManifest.arc);
            return 0;
        }
        a->mpBgW = JKR_NEW dBgW();
        if (a->mpBgW == NULL) {
            return 0;
        }
        MTXIdentity(a->mBgMtx);
        mDoMtx_stack_c::transS(a->current.pos.x, a->current.pos.y, a->current.pos.z);
        mDoMtx_stack_c::YrotM(a->current.angle.y);
        MTXCopy(mDoMtx_stack_c::get(), a->mBgMtx);
        if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::MOVE_BG_e, &a->mBgMtx) == 1) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only Set failed '{}'",
                         a->mManifest.collision);
            return 0;
        }
        a->mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
        a->mBgReady = true;
        a->mpMorf = NULL;
        stageLog("heap", "ok — collision-only");
        return 1;
    }

    stageLog("heap", "getObjectRes model");
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.model);
    if (raw == NULL) {
        stageLog("heap", "FAIL model res NULL");
        return 0;
    }
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, a->mManifest.model, raw,
                                             a->mManifest.bodyBmt[0] ? a->mManifest.bodyBmt : NULL);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] model '{}' missing/unparseable in arc '{}'", a->mManifest.model,
                     a->mManifest.arc);
        return 0;
    }

    // №47-A: bind idle on every non-static mount (WW short names allowed).
    stageLog("heap", "getObjectRes idle BCK");
    J3DAnmTransform* anm = NULL;
    if (a->mManifest.idle[0] &&
        (a->mManifest.isStatic || a->mManifest.doorAttention ||
         animAllowedOnBody(a->mManifest.model, a->mManifest.idle))) {
        anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.idle);
        if (anm == NULL) {
            DuskLog.warn("[ExtNpcMount] idle '{}' missing — static pose", a->mManifest.idle);
        }
    } else if (a->mManifest.idle[0]) {
        DuskLog.warn("[ExtNpcMount] N3 refuse idle '{}' for model '{}' — static pose",
                     a->mManifest.idle, a->mManifest.model);
    }

    J3DMaterial* mat0 = data->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("heap", "FAIL pre-McaMorf material[0]=NULL");
        return 0;
    }

    stageLog("heap", "McaMorf ctor");
    a->mpMorf = JKR_NEW mDoExt_McaMorf(data, NULL, NULL, anm, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1,
                                       1, NULL, 0x80000, 0x11000084);
    if (a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        stageLog("heap", "FAIL McaMorf");
        a->mpMorf = NULL;
        return 0;
    }

    a->mpCompanion = NULL;
    if (a->mManifest.companionModel[0]) {
        void* cRaw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.companionModel);
        J3DModelData* cData =
            acquireMountedModel(a->mManifest.arc, a->mManifest.companionModel, cRaw);
        // №36 A / №49: joint_slave (or slave_map) — no hand BCK. Synced mode still wants idle.
        J3DAnmTransform* cAnm = NULL;
        const bool jointSlave = a->mManifest.companionMode == 1 || a->mSlaveMap[0] != '\0';
        if (jointSlave) {
            a->mManifest.companionMode = 1;
        }
        if (!jointSlave) {
            const char* cIdle = a->mManifest.companionIdle[0] ? a->mManifest.companionIdle :
                                                               a->mManifest.idle;
            if (cIdle[0]) {
                cAnm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, cIdle);
            }
        }
        if (cData != NULL && (jointSlave || cAnm != NULL)) {
            a->mpCompanion =
                JKR_NEW mDoExt_McaMorf(cData, NULL, NULL, cAnm, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1,
                                       1, NULL, 0x80000, 0x11000084);
            if (a->mpCompanion == NULL || a->mpCompanion->getModel() == NULL) {
                DuskLog.warn("[ExtNpcMount] companion '{}' McaMorf failed — continuing without",
                             a->mManifest.companionModel);
                a->mpCompanion = NULL;
            } else {
                if (jointSlave) {
                    installSlaveJointCallbacks(a->mpCompanion->getModel(), a);
                    resolveSlaveMapPairs(a);
                }
                DuskLog.info("[ExtNpcMount] companion '{}' mode={} pairs={}",
                             a->mManifest.companionModel, jointSlave ? "joint_slave" : "synced",
                             (int)a->mSlavePairCount);
            }
        } else {
            DuskLog.warn("[ExtNpcMount] companion '{}' missing (or idle for synced) — skip",
                         a->mManifest.companionModel);
        }
    }

    // №36 C: optional brk/btk color (Vlupy rupees).
    a->mpBrk = NULL;
    a->mpColorBtk = NULL;
    if (a->mpMorf != NULL && a->mpMorf->getModel() != NULL) {
        J3DModelData* bodyData = a->mpMorf->getModel()->getModelData();
        if (a->mManifest.brk[0] && bodyData != NULL) {
            J3DAnmTevRegKey* brk =
                (J3DAnmTevRegKey*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.brk);
            if (brk != NULL) {
                a->mpBrk = JKR_NEW mDoExt_brkAnm();
                if (a->mpBrk != NULL &&
                    a->mpBrk->init(bodyData, brk, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1)) {
                    f32 frame = a->mManifest.colorFrame;
                    if (frame < 0.0f) {
                        frame = vlupyColorFrame(a->mPickupItemNo);
                    }
                    a->mpBrk->setFrame(frame);
                } else {
                    a->mpBrk = NULL;
                }
            }
        }
        if (a->mManifest.btk[0] && bodyData != NULL) {
            J3DAnmTextureSRTKey* btk =
                (J3DAnmTextureSRTKey*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.btk);
            if (btk != NULL) {
                a->mpColorBtk = JKR_NEW mDoExt_btkAnm();
                if (a->mpColorBtk != NULL &&
                    a->mpColorBtk->init(bodyData, btk, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1)) {
                    f32 frame = a->mManifest.colorFrame;
                    if (frame < 0.0f) {
                        frame = vlupyColorFrame(a->mPickupItemNo);
                    }
                    a->mpColorBtk->setFrame(frame);
                } else {
                    a->mpColorBtk = NULL;
                }
            }
        }
    }

    tryBindBtp(a, data);
    a->mAttachCount = 0;
    for (int i = 0; i < kExtNpcMaxAttach; ++i) {
        a->mpAttach[i] = NULL;
        a->mAttachJnt[i] = -1;
        a->mAttachOnCompanion[i] = 0;
        a->mAttachSlave[i] = 0;
        addAttachment(a, data, a->mManifest.attach[i]);
    }
    addDoorVisual(a, data);
    stageLog("heap", "ok — model bound");
    return 1;
}

void updateBgTransform(dExtNpcMount_c* a);

int useBgHeapInit(fopAc_ac_c* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    // Outset's models stay raw J3D2 in dRes (BDLM) — must loadBinaryDisplayList /
    // load before touching ModelData. Do NOT use the NPC light-mask finish.
    const char* names[] = {a->mManifest.model, a->mManifest.model2, a->mManifest.model3};
    for (int i = 0; i < 3; ++i) {
        a->mpBgModels[i] = NULL;
        if (names[i][0] == '\0') {
            continue;
        }
        void* raw = dComIfG_getObjectRes(a->mManifest.arc, names[i]);
        J3DModelData* data = acquireBgModel(a->mManifest.arc, names[i], raw);
        if (data == NULL) {
            DuskLog.warn("[ExtNpcMount] BG model '{}' resolve failed in '{}'", names[i],
                         a->mManifest.arc);
            return 0;
        }
        a->mpBgModels[i] = mDoExt_J3DModel__create(data, 0x80000, 0x11000084);
        if (a->mpBgModels[i] == NULL) {
            DuskLog.warn("[ExtNpcMount] BG model '{}' J3DModel__create failed", names[i]);
            return 0;
        }
        DuskLog.info("[ExtNpcMount] BG model[{}] '{}' mats={} joints={} shapes={}", i, names[i],
                     data->getMaterialNum(), data->getJointNum(), data->getShapeNum());
    }
    a->mpBgBtk = NULL;
    if (a->mManifest.model2Btk[0] && a->mpBgModels[1] != NULL) {
        J3DAnmTextureSRTKey* btk = (J3DAnmTextureSRTKey*)dComIfG_getObjectRes(
            a->mManifest.arc, a->mManifest.model2Btk);
        a->mpBgBtk = JKR_NEW mDoExt_btkAnm();
        if (btk == NULL || a->mpBgBtk == NULL ||
            !a->mpBgBtk->init(a->mpBgModels[1]->getModelData(), btk, TRUE,
                               J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1)) {
            DuskLog.warn("[ExtNpcMount] BG btk '{}' unavailable", a->mManifest.model2Btk);
            a->mpBgBtk = NULL;
        } else {
            DuskLog.info("[ExtNpcMount] BG btk '{}' bound", a->mManifest.model2Btk);
        }
    }
    // brakeeff / Obj_Fmobj pattern: Set against IDENTITY, then copy the real
    // host−anchor mtx and Move() AFTER Regist. GlobalVtx-at-Set with the final
    // translate left the dzb stranded at Great Sea (№20 hologram).
    MTXIdentity(a->mBgMtx);
    void* dzb = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.collision);
    if (dzb == NULL) {
        DuskLog.warn("[ExtNpcMount] BG collision '{}' missing in arc '{}'", a->mManifest.collision,
                     a->mManifest.arc);
        return 0;
    }
    a->mpBgW = JKR_NEW dBgW();
    if (a->mpBgW == NULL) {
        DuskLog.warn("[ExtNpcMount] BG dBgW alloc failed for '{}'", a->mManifest.collision);
        return 0;
    }
    // №98/№107: identity dzb transform (host − anchor == 0) = world collision
    // (GLOBAL_e), matching TP d_a_bg. MOVE_BG_e makes the space a "moving object"
    // ⇒ climb/ledge break. Outset identity is host=anchor=cell (not 0,0,0).
    a->mBgGlobal = false;
    {
        const char* stage = dComIfGp_getStartStageName();
        const bool onWwHost = stage != NULL && dExtWwSave_isWwHostStage(stage);
        const bool identity =
            a->mManifest.hasHostPos && a->mManifest.hasAnchor &&
            cM3d_IsZero(a->mManifest.hostPos.x - a->mManifest.anchor.x) &&
            cM3d_IsZero(a->mManifest.hostPos.y - a->mManifest.anchor.y) &&
            cM3d_IsZero(a->mManifest.hostPos.z - a->mManifest.anchor.z);
        if (onWwHost && identity) {
            if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::GLOBAL_e, NULL) == 1) {
                DuskLog.warn("[ExtNpcMount] BG dBgW::Set GLOBAL failed for '{}'",
                             a->mManifest.collision);
                return 0;
            }
            a->mBgGlobal = true;
            DuskLog.info("[ExtNpcMount] №107 BG GLOBAL_e (identity) proc='{}' arc='{}'",
                         a->mManifest.proc, a->mManifest.arc);
            return 1;
        }
    }
    if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::MOVE_BG_e, &a->mBgMtx) == 1) {
        DuskLog.warn("[ExtNpcMount] BG dBgW::Set failed for '{}'", a->mManifest.collision);
        return 0;
    }
    a->mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
    return 1;
}

void updateBgTransform(dExtNpcMount_c* a) {
    // DZB (world-baked): always host − anchor.
    mDoMtx_stack_c::transS(a->current.pos.x - a->mManifest.anchor.x,
                           a->current.pos.y - a->mManifest.anchor.y,
                           a->current.pos.z - a->mManifest.anchor.z);
    MTXCopy(mDoMtx_stack_c::get(), a->mBgMtx);

    // Models: №22 — WW sea-room meshes are cell-LOCAL; one shared mtx cannot align
    // them with a world-baked dzb. Local ⇒ translate(host) only.
    if (a->mManifest.modelSpaceLocal) {
        mDoMtx_stack_c::transS(a->current.pos.x, a->current.pos.y, a->current.pos.z);
    }
    MtxP modelBase = mDoMtx_stack_c::get();
    for (int i = 0; i < 3; ++i) {
        if (a->mpBgModels[i] != NULL) {
            a->mpBgModels[i]->setBaseTRMtx(modelBase);
            a->mpBgModels[i]->calc();  // without this, meshes stay at world origin
        }
    }
}

// №45/№48: static head/companion pins from actor_map, keyed by "PROC:arg".
struct HeadRegistryEntry {
    char headModel[64]{};
    char headJoint[32]{};
    char headFromParams[32]{};  // prefix → resolve at create from params low byte
    char slaveMap[160]{};       // №48 companion_slave_map
    char companionModel[64]{};  // №49: e.g. bmarm.bdl alongside head_model
    char bodyBmt[64]{};         // №50-C: e.g. ko02.bmt
};
std::unordered_map<std::string, HeadRegistryEntry> s_headRegistry;

void loadHeadRegistryForMod(const fs::path& modRoot) {
    const fs::path mapPath = modRoot / "population" / "actor_map.ini";
    std::ifstream in(mapPath);
    if (!in) {
        return;
    }
    std::string line;
    std::string section;
    HeadRegistryEntry cur{};
    char proc[32] = {};
    int arg = -1;
    bool haveSection = false;
    auto commit = [&]() {
        if (!haveSection || proc[0] == '\0') {
            return;
        }
        if (cur.headModel[0] == '\0' && cur.headFromParams[0] == '\0' && cur.slaveMap[0] == '\0' &&
            cur.companionModel[0] == '\0' && cur.bodyBmt[0] == '\0') {
            return;
        }
        const int useArg = arg >= 0 ? arg : 0;
        const std::string key = std::string(proc) + ":" + std::to_string(useArg);
        auto existing = s_headRegistry.find(key);
        if (existing != s_headRegistry.end()) {
            // Merge slave_map / companion / body_bmt into a prior head row (same proc:arg).
            if (existing->second.slaveMap[0] == '\0' && cur.slaveMap[0]) {
                snprintf(existing->second.slaveMap, sizeof(existing->second.slaveMap), "%s",
                         cur.slaveMap);
            }
            if (existing->second.companionModel[0] == '\0' && cur.companionModel[0]) {
                snprintf(existing->second.companionModel, sizeof(existing->second.companionModel),
                         "%s", cur.companionModel);
            }
            if (existing->second.bodyBmt[0] == '\0' && cur.bodyBmt[0]) {
                snprintf(existing->second.bodyBmt, sizeof(existing->second.bodyBmt), "%s",
                         cur.bodyBmt);
            }
            return;
        }
        if (cur.headJoint[0] == '\0' && (cur.headModel[0] || cur.headFromParams[0])) {
            snprintf(cur.headJoint, sizeof(cur.headJoint), "%s", "head");
        }
        s_headRegistry[key] = cur;
        DuskLog.debug(
            "[ExtNpcMount] head registry {} → {} / companion={} / slave_map={}", key,
            cur.headModel[0] ? cur.headModel : "-",
            cur.companionModel[0] ? cur.companionModel : "-",
            cur.slaveMap[0] ? cur.slaveMap : "-");
    };
    while (std::getline(in, line)) {
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            commit();
            const size_t end = line.find(']');
            section = end == std::string::npos ? "" : line.substr(1, end - 1);
            cur = HeadRegistryEntry{};
            proc[0] = '\0';
            arg = -1;
            haveSection = section != "layers";
            continue;
        }
        if (!haveSection) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        while (!key.empty() && key.back() == ' ') {
            key.pop_back();
        }
        while (!val.empty() && val[0] == ' ') {
            val.erase(0, 1);
        }
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        if (key == "proc") {
            snprintf(proc, sizeof(proc), "%s", val.c_str());
        } else if (key == "arg") {
            arg = std::atoi(val.c_str());
        } else if (key == "head_model") {
            snprintf(cur.headModel, sizeof(cur.headModel), "%s", val.c_str());
        } else if (key == "head_joint") {
            snprintf(cur.headJoint, sizeof(cur.headJoint), "%s", val.c_str());
        } else if (key == "head_from_params") {
            snprintf(cur.headFromParams, sizeof(cur.headFromParams), "%s", val.c_str());
        } else if (key == "companion_slave_map") {
            snprintf(cur.slaveMap, sizeof(cur.slaveMap), "%s", val.c_str());
        } else if (key == "companion") {
            snprintf(cur.companionModel, sizeof(cur.companionModel), "%s", val.c_str());
        } else if (key == "body_bmt") {
            snprintf(cur.bodyBmt, sizeof(cur.bodyBmt), "%s", val.c_str());
        }
    }
    commit();
}

bool pullHeadFromRegistry(const char* procName, int registryArg, u32 wwParams, char* headOut,
                          u32 headBytes, char* jointOut, u32 jointBytes) {
    if (procName == NULL || procName[0] == '\0' || headOut == NULL || headBytes == 0) {
        return false;
    }
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end()) {
        return false;
    }
    const HeadRegistryEntry& e = it->second;
    if (e.headFromParams[0]) {
        int idx = static_cast<int>(wwParams & 0xFF);
        if (idx <= 0 || idx > 99 || idx == 0xFF) {
            idx = 1;
        }
        snprintf(headOut, headBytes, "%s%02d.bdl", e.headFromParams, idx);
    } else if (e.headModel[0]) {
        snprintf(headOut, headBytes, "%s", e.headModel);
    } else {
        return false;
    }
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", e.headJoint[0] ? e.headJoint : "head");
    }
    return true;
}

bool pullSlaveMapFromRegistry(const char* procName, int registryArg, char* mapOut, u32 mapBytes) {
    if (procName == NULL || procName[0] == '\0' || mapOut == NULL || mapBytes == 0) {
        return false;
    }
    mapOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.slaveMap[0] == '\0') {
        return false;
    }
    snprintf(mapOut, mapBytes, "%s", it->second.slaveMap);
    return true;
}

bool pullCompanionFromRegistry(const char* procName, int registryArg, char* companionOut,
                               u32 companionBytes) {
    if (procName == NULL || procName[0] == '\0' || companionOut == NULL || companionBytes == 0) {
        return false;
    }
    companionOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.companionModel[0] == '\0') {
        return false;
    }
    snprintf(companionOut, companionBytes, "%s", it->second.companionModel);
    return true;
}

bool pullBodyBmtFromRegistry(const char* procName, int registryArg, char* bmtOut, u32 bmtBytes) {
    if (procName == NULL || procName[0] == '\0' || bmtOut == NULL || bmtBytes == 0) {
        return false;
    }
    bmtOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.bodyBmt[0] == '\0') {
        return false;
    }
    snprintf(bmtOut, bmtBytes, "%s", it->second.bodyBmt);
    return true;
}

}  // namespace

J3DModelData* dExtNpcMount_acquireModelData(const char* arc, const char* modelName) {
    if (arc == NULL || arc[0] == '\0' || modelName == NULL || modelName[0] == '\0') {
        return NULL;
    }
    void* res = dComIfG_getObjectRes(arc, modelName);
    if (res == NULL) {
        return NULL;
    }
    return acquireMountedModel(arc, modelName, res);
}

void dExtNpcMount_retainArc(const char* arc) {
    retainArcModels(arc);
}

void dExtNpcMount_releaseArc(const char* arc) {
    releaseArcModels(arc, "knob00-release");
}

void dExtNpcMount_rescanProviders() {
    s_providers.clear();
    s_providerOrder.clear();
    s_headRegistry.clear();
    // №73: do NOT treat s_modelDataCache as immortal. Room-lane mounts resDelete their
    // arcs; cache entries are purged on last releaseArcModels (see retain/release pair).
    // rescan leaves the cache alone — live mounts still own their arcs.
    std::error_code ec;
    const fs::path userRoot = dusk::ConfigPath / "model_replacements";
    if (!fs::is_directory(userRoot, ec)) {
        return;
    }
    // A2: socket+arg first-claim wins (directory walk order; top mod list is separate).
    std::unordered_map<std::string, std::string> claimedSocketArg;  // "SOCKET:arg" → proc
    for (auto it = fs::directory_iterator(userRoot, ec); it != fs::directory_iterator();
         it.increment(ec)) {
        if (ec || !it->is_directory(ec)) {
            continue;
        }
        const fs::path modRoot = it->path();
        const std::string modName = modRoot.filename().string();
        if (shouldSkipModFolder(modName)) {
            continue;
        }
        if (!dusk::custom_assets::is_folder_enabled(modName.c_str())) {
            continue;
        }
        const fs::path npcDir = modRoot / "npc";
        if (!fs::is_directory(npcDir, ec)) {
            continue;
        }
        for (auto nit = fs::directory_iterator(npcDir, ec); nit != fs::directory_iterator();
             nit.increment(ec)) {
            if (ec || !nit->is_regular_file(ec)) {
                continue;
            }
            if (nit->path().extension() != ".ini") {
                continue;
            }
            dExtNpcManifest man{};
            if (!parseManifestFile(nit->path(), modName.c_str(), &man)) {
                continue;
            }
            // №122: a manifest may legitimately name NO arc. Ported systems
            // (the vegetation lane) carry their geometry in an extracted asset
            // pack and create their own profile rather than mounting a model,
            // so there is nothing to look for under arcs/. Only enforce arc
            // presence when the manifest actually declares one.
            if (man.arc[0] != '\0' && !man.fromDvd && !arcFilePresent(modRoot, man.arc)) {
                DuskLog.debug(
                    "[ExtNpcMount] '{}' manifest ok but arc missing under {} — socket idle",
                    man.proc, (modRoot / "arcs" / (std::string(man.arc) + ".arc")).string());
                continue;
            }
            const char* sock = man.socket[0] ? man.socket : man.proc;
            const int sarg = man.socketArg >= 0 ? man.socketArg : 0;
            const std::string claimKey = std::string(sock) + ":" + std::to_string(sarg);
            auto claimed = claimedSocketArg.find(claimKey);
            if (claimed != claimedSocketArg.end() && claimed->second != man.proc) {
                DuskLog.warn(
                    "[ExtNpcMount] socket claim lost: {} arg={} kept proc={} rejected '{}' (mod {})",
                    sock, sarg, claimed->second, man.proc, modName);
                continue;
            }
            claimedSocketArg[claimKey] = man.proc;
            s_providers[man.proc] = man;
            s_providerOrder.push_back(man.proc);
            DuskLog.info("[ExtNpcMount] provider {} ← mod '{}' arc={} skip_btp={} btp={} scale={}",
                         man.proc, modName, man.arc, man.skipBtp ? 1 : 0,
                         man.btp[0] ? man.btp : "-", man.scale);
        }

        // №27 N4: overlay user-locked identities from population/identity.ini.
        const fs::path idPath = modRoot / "population" / "identity.ini";
        std::ifstream idIn(idPath);
        if (idIn) {
            std::string line;
            std::string section;
            while (std::getline(idIn, line)) {
                while (!line.empty() &&
                       (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                    line.pop_back();
                }
                if (line.empty() || line[0] == '#' || line[0] == ';') {
                    continue;
                }
                if (line[0] == '[') {
                    const size_t end = line.find(']');
                    section = end == std::string::npos ? "" : line.substr(1, end - 1);
                    continue;
                }
                const size_t eq = line.find('=');
                if (eq == std::string::npos || section.empty()) {
                    continue;
                }
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                while (!key.empty() && key.back() == ' ') {
                    key.pop_back();
                }
                while (!val.empty() && val[0] == ' ') {
                    val.erase(0, 1);
                }
                if (key != "display_name" && key != "proc") {
                    continue;
                }
                // Section may be census name (Ls1) or proc (NPC_LS).
                std::string procKey = section;
                if (key == "proc") {
                    procKey = val;
                    continue;
                }
                // Prefer matching by proc; also try census→proc via common NPC_ prefix guess.
                auto pit = s_providers.find(procKey);
                if (pit == s_providers.end()) {
                    // Try NPC_<SECTION> uppercased stem.
                    std::string guess = "NPC_";
                    for (char c : section) {
                        if (c >= 'a' && c <= 'z') {
                            guess += (char)(c - 'a' + 'A');
                        } else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                            guess += c;
                        }
                    }
                    // Strip trailing digits for Ls1 → NPC_LS
                    while (!guess.empty() && guess.back() >= '0' && guess.back() <= '9') {
                        guess.pop_back();
                    }
                    pit = s_providers.find(guess);
                }
                if (pit != s_providers.end() && !val.empty()) {
                    snprintf(pit->second.displayName, sizeof(pit->second.displayName), "%s",
                             val.c_str());
                    DuskLog.info("[ExtNpcMount] N4 identity lock {} → '{}'", pit->first, val);
                }
            }
        }

        // №45: actor_map → (proc,arg) head pull registry (no transient handoff).
        loadHeadRegistryForMod(modRoot);
    }
    DuskLog.info("[ExtNpcMount] head registry entries={}", s_headRegistry.size());
}

bool dExtNpcMount_hasPayload(const char* procName) {
    return procName != NULL && s_providers.find(procName) != s_providers.end();
}

bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out) {
    auto it = s_providers.find(procName != NULL ? procName : "");
    if (it == s_providers.end() || out == NULL) {
        return false;
    }
    *out = it->second;
    return true;
}

int dExtNpcMount_providerCount() {
    return (int)s_providerOrder.size();
}

bool dExtNpcMount_providerAt(int index, dExtNpcManifest* out) {
    if (out == NULL || index < 0 || index >= (int)s_providerOrder.size()) {
        return false;
    }
    return dExtNpcMount_lookup(s_providerOrder[index].c_str(), out);
}

s16 dExtNpcMount_socketActorId(const char* socketName) {
    if (socketName == NULL) {
        return -1;
    }
    if (strcmp(socketName, "NPC_MK") == 0) {
        return fpcNm_NPC_MK_e;
    }
    if (strcmp(socketName, "NPC_P2") == 0) {
        return fpcNm_NPC_P2_e;
    }
    if (strcmp(socketName, "NPC_HENNA0") == 0) {
        return fpcNm_NPC_HENNA0_e;
    }
    if (strcmp(socketName, "NPC_KDK") == 0) {
        return fpcNm_NPC_KDK_e;
    }
    if (strcmp(socketName, "EXT_BG10") == 0) {
        return fpcNm_EXT_BG10_e;
    }
    if (strcmp(socketName, "WWGRASS") == 0) {
        return fpcNm_WWGRASS_e;
    }
    if (strcmp(socketName, "WWBRIDGE") == 0) {
        return fpcNm_WWBRIDGE_e;
    }
    return -1;
}

static const char* providerSocket(const dExtNpcManifest& man) {
    return man.socket[0] ? man.socket : man.proc;
}

bool dExtNpcMount_hasSocketPayload(const char* socketName) {
    if (socketName == NULL || socketName[0] == '\0') {
        return false;
    }
    for (const auto& kv : s_providers) {
        if (strcmp(providerSocket(kv.second), socketName) == 0) {
            return true;
        }
    }
    return false;
}

// №38 E1 RESOLVER LAW: exact socket + exact arg, or REFUSE. No wildcard / first-match /
// default payload — a miss is a hole + log, never an imposter (Ganondorf = HENNA0 arg 1).
bool dExtNpcMount_resolveSocket(const char* socketName, int arg, char* procOut, u32 procOutBytes) {
    if (socketName == NULL || procOut == NULL || procOutBytes == 0) {
        return false;
    }
    procOut[0] = '\0';

    for (const auto& kv : s_providers) {
        const dExtNpcManifest& man = kv.second;
        if (strcmp(providerSocket(man), socketName) != 0) {
            continue;
        }
        if (man.socketArg == arg) {
            snprintf(procOut, procOutBytes, "%s", man.proc);
            return true;
        }
    }
    for (const auto& kv : s_providers) {
        const dExtNpcManifest& man = kv.second;
        if (strcmp(providerSocket(man), socketName) != 0) {
            continue;
        }
        for (int i = 0; i < man.subtypeCount; ++i) {
            if (man.subtypes[i].valid && man.subtypes[i].arg == arg) {
                snprintf(procOut, procOutBytes, "%s", man.proc);
                return true;
            }
        }
    }
    DuskLog.warn("[ExtNpcMount] E1 REFUSED socket='{}' arg={} — no exact payload (hole, not fallback)",
                 socketName, arg);
    return false;
}

static char s_forcedCreateProc[32] = {};
static bool s_forcedCreateProcSet = false;
static char s_forcedAttachModel[64] = {};
static char s_forcedAttachJoint[32] = {};
static bool s_forcedAttachSet = false;
static char s_forcedSpawnSrc[96] = {};
static bool s_forcedSpawnSrcSet = false;

struct PendingSpawn {
    char proc[32]{};
    char src[96]{};
    char headModel[64]{};
    char headJoint[32]{};
    // №130: identity token. The spawner reclaims its own entry by this value
    // after create, so an actor that forgets to consume cannot leave a stale
    // entry for the NEXT actor to pull. Order-based reaping was not safe —
    // popping the front can discard somebody else's entry.
    u32 seq{};
};
static u32 s_pendingSeq = 0;
static std::deque<u32> s_passWatermark;
static std::unordered_map<u32, PendingSpawn> s_pendingById;
static std::deque<PendingSpawn> s_pendingFifo;

static void fillPending(PendingSpawn* p, const char* procName, const char* src,
                        const char* headModel, const char* headJoint) {
    *p = PendingSpawn{};
    if (procName != NULL && procName[0]) {
        snprintf(p->proc, sizeof(p->proc), "%s", procName);
    }
    if (src != NULL && src[0]) {
        snprintf(p->src, sizeof(p->src), "%s", src);
    }
    if (headModel != NULL && headModel[0]) {
        snprintf(p->headModel, sizeof(p->headModel), "%s", headModel);
        snprintf(p->headJoint, sizeof(p->headJoint), "%s",
                 headJoint != NULL && headJoint[0] ? headJoint : "head");
    }
}

u32 dExtNpcMount_pushPendingSpawn(const char* procName, const char* src, const char* headModel,
                                  const char* headJoint) {
    PendingSpawn p{};
    fillPending(&p, procName, src, headModel, headJoint);
    p.seq = ++s_pendingSeq;
    s_pendingFifo.push_back(p);
    return p.seq;
}

// №130 STRUCTURAL GUARD. Every census spawn pushes an entry that the created
// actor is supposed to consume in its create. Historically that was a rule each
// actor author had to remember, and forgetting it was silent and catastrophic:
// the stale entry is pulled by the NEXT actor, so every later actor wears an
// earlier row's head. It has bitten twice (№64 orphan-entry, №129 ported
// actors) and the blast radius scales with cast size — one leak on a
// fully-populated Great Sea would shift hundreds of actors.
//
// So the queue is no longer trust-based. The spawner reclaims its own entry by
// token after create; anything unconsumed is dropped THERE, before it can be
// mis-served. Actors that consume correctly are unaffected (their entry is
// already gone). Actors that forget now degrade to "no head pinned" instead of
// corrupting the whole cast — a local, visible defect instead of a global,
// invisible one.
// №131 OVERRIDE. If the №130 guard itself turns out to be wrong, it must be
// switchable WITHOUT a rebuild — an escape hatch that needs a compile is not an
// escape hatch. Read mod-side: population/engine_overrides.ini, key
// `pending_spawn_guard` (default 1 = guard on; 0 = vanilla FIFO order only).
static int s_pendingGuard = -1;  // -1 = not yet read
bool dExtNpcMount_pendingGuardEnabled() {
    if (s_pendingGuard >= 0) {
        return s_pendingGuard != 0;
    }
    s_pendingGuard = 1;
    const fs::path root = fs::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (fs::is_directory(root, ec)) {
        for (const auto& entry : fs::directory_iterator(root, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::ifstream in(entry.path() / "population" / "engine_overrides.ini");
            if (!in) {
                continue;
            }
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0] == '#' || line[0] == ';') {
                    continue;
                }
                const size_t eq = line.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                while (!key.empty() && (key.back() == ' ' || key.back() == 0x0D)) key.pop_back();
                while (!val.empty() && (val.back() == ' ' || val.back() == 0x0D)) val.pop_back();
                if (key == "pending_spawn_guard" && (val == "0" || val == "off" || val == "false")) {
                    s_pendingGuard = 0;
                }
            }
        }
    }
    if (s_pendingGuard == 0) {
        DuskLog.warn(
            "[ExtNpcMount] №131 OVERRIDE ACTIVE — pending-spawn guard DISABLED via "
            "population/engine_overrides.ini. Head/identity binding falls back to vanilla "
            "FIFO ORDER, which is order-dependent: any actor that does not consume its entry "
            "will shift every actor created after it.");
        DuskLog.warn(
            "[ExtNpcMount] №131 BEFORE RELYING ON THIS BUILD: (1) snapshot the current engine "
            "+ mod-folder state so it can be restored, (2) re-verify actor identities AND "
            "placements on every affected space and story layer — a mis-bind is silent and "
            "keeps correct dialogue, so it will NOT announce itself.");
    }
    return s_pendingGuard != 0;
}

// №131: entries are keyed by actor id, and process ids are RECYCLED. An entry
// whose actor died without consuming would otherwise sit here until some later
// actor is handed the same id and pulls a dead row's head — the same corruption
// by a slower route, and story/layer changes (vanilla re-spawns the cast per
// dComIfG_play_c::getLayerNo) make repeats routine. Sweep before every pass.
void dExtNpcMount_sweepPendingById() {
    // №133: this used to ask vanilla whether each actor was still alive
    // (fopAcM_SearchByID / fpcM_IsCreating). That was the wrong instinct: it put
    // our bookkeeping inside vanilla's actor lifecycle, where it kept colliding
    // with meanings that are vanilla's to define — "not found" also means "still
    // creating", culling is a status bit and not a death, room unload is a real
    // death, and story layers re-spawn the whole cast. Every one of those had to
    // be re-taught to the guard, and each re-teaching was a chance to get
    // vanilla's semantics wrong.
    //
    // The guard has no business knowing any of that. It only needs to bound its
    // OWN side table, so it does that on its OWN clock: an entry that has
    // survived two full population passes was never going to be claimed. No
    // vanilla state is inspected, so vanilla's lifecycle — cull, drop, recall,
    // layer change, save/restore — proceeds exactly as it always did and simply
    // cannot be fought by this code.
    if (s_passWatermark.size() < 2) {
        return;
    }
    const u32 cutoff = s_passWatermark.front();
    for (auto it = s_pendingById.begin(); it != s_pendingById.end();) {
        it = (it->second.seq != 0 && it->second.seq < cutoff) ? s_pendingById.erase(it)
                                                             : std::next(it);
    }
}

// Ring of the last two pass-start token values. Purely our own clock.
void dExtNpcMount_markPendingPass() {
    s_passWatermark.push_back(s_pendingSeq);
    while (s_passWatermark.size() > 2) {
        s_passWatermark.pop_front();
    }
}

bool dExtNpcMount_reapPendingSpawn(u32 seq, fpc_ProcID id) {
    if (!dExtNpcMount_pendingGuardEnabled()) {
        return false;  // №131: vanilla FIFO order, by explicit override
    }
    if (seq == 0) {
        return false;
    }
    for (auto it = s_pendingFifo.begin(); it != s_pendingFifo.end(); ++it) {
        if (it->seq != seq) {
            continue;
        }
        // Still queued, so this actor's create has NOT consumed it yet.
        //
        // It matters enormously which way we resolve that. fopAcM_create may run
        // create synchronously OR defer it (that is why the push happens first —
        // see №45). Simply erasing here would be correct for a sync actor that
        // forgot, and CATASTROPHIC for a phase-based actor that was going to
        // consume on a later frame: we would delete the entry out from under it.
        //
        // So don't erase — RE-KEY. Move it out of the order-sensitive queue and
        // into the id map, where the owning actor can still claim it by its own
        // id whenever its create actually runs. Order-dependence, which is the
        // root of this entire bug class, disappears: a late or forgetful actor
        // can no longer be served somebody else's entry, and can no longer
        // cause somebody else to be served its own.
        PendingSpawn p = *it;
        char procName[32];
        snprintf(procName, sizeof(procName), "%s", p.proc);
        s_pendingFifo.erase(it);
        if (id != fpcM_ERROR_PROCESS_ID_e) {
            s_pendingById[static_cast<u32>(id)] = p;
        }
        return true;
    }
    return false;
}

void dExtNpcMount_bindPendingSpawn(fpc_ProcID id, const char* procName, const char* src,
                                   const char* headModel, const char* headJoint) {
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        return;
    }
    fillPending(&s_pendingById[static_cast<u32>(id)], procName, src, headModel, headJoint);
    // №64: createBgMountAtHost push+bind left an orphan FIFO entry; the next HENNA0
    // create (population) then stole it (EXT_BG1 at the door / Ba1←Lamp). Drain one
    // matching FIFO slot now that the id-map owns the bind.
    if (procName != NULL && procName[0] != '\0') {
        for (auto it = s_pendingFifo.begin(); it != s_pendingFifo.end(); ++it) {
            if (std::strcmp(it->proc, procName) == 0) {
                s_pendingFifo.erase(it);
                break;
            }
        }
    }
}

bool dExtNpcMount_takePendingSpawn(fpc_ProcID id, char* procOut, u32 procBytes, char* srcOut,
                                   u32 srcBytes, char* headOut, u32 headBytes, char* jointOut,
                                   u32 jointBytes) {
    PendingSpawn p{};
    bool have = false;
    if (id != fpcM_ERROR_PROCESS_ID_e) {
        auto it = s_pendingById.find(static_cast<u32>(id));
        if (it != s_pendingById.end()) {
            p = it->second;
            s_pendingById.erase(it);
            have = true;
        }
    }
    if (!have && !s_pendingFifo.empty()) {
        p = s_pendingFifo.front();
        s_pendingFifo.pop_front();
        have = true;
    }
    if (!have) {
        return false;
    }
    if (procOut != NULL && procBytes > 0) {
        snprintf(procOut, procBytes, "%s", p.proc);
    }
    if (srcOut != NULL && srcBytes > 0) {
        snprintf(srcOut, srcBytes, "%s", p.src);
    }
    if (headOut != NULL && headBytes > 0) {
        snprintf(headOut, headBytes, "%s", p.headModel);
    }
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", p.headJoint);
    }
    return true;
}

void dExtNpcMount_forceNextSpawnSrc(const char* src) {
    if (src == NULL || src[0] == '\0') {
        s_forcedSpawnSrcSet = false;
        s_forcedSpawnSrc[0] = '\0';
        return;
    }
    snprintf(s_forcedSpawnSrc, sizeof(s_forcedSpawnSrc), "%s", src);
    s_forcedSpawnSrcSet = true;
}

void dExtNpcMount_forceNextCreateProc(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        s_forcedCreateProcSet = false;
        s_forcedCreateProc[0] = '\0';
        return;
    }
    snprintf(s_forcedCreateProc, sizeof(s_forcedCreateProc), "%s", procName);
    s_forcedCreateProcSet = true;
}

bool dExtNpcMount_consumeForcedCreateProc(char* procOut, u32 procOutBytes) {
    if (!s_forcedCreateProcSet || procOut == NULL || procOutBytes == 0) {
        return false;
    }
    snprintf(procOut, procOutBytes, "%s", s_forcedCreateProc);
    s_forcedCreateProcSet = false;
    s_forcedCreateProc[0] = '\0';
    return true;
}

void dExtNpcMount_forceNextAttach(const char* modelName, const char* jointName) {
    if (modelName == NULL || modelName[0] == '\0') {
        s_forcedAttachSet = false;
        s_forcedAttachModel[0] = '\0';
        s_forcedAttachJoint[0] = '\0';
        return;
    }
    snprintf(s_forcedAttachModel, sizeof(s_forcedAttachModel), "%s", modelName);
    snprintf(s_forcedAttachJoint, sizeof(s_forcedAttachJoint), "%s",
             jointName != NULL && jointName[0] ? jointName : "head");
    s_forcedAttachSet = true;
}

bool dExtNpcMount_consumeForcedAttach(char* modelOut, u32 modelBytes, char* jointOut,
                                      u32 jointBytes) {
    if (!s_forcedAttachSet || modelOut == NULL || modelBytes == 0) {
        return false;
    }
    snprintf(modelOut, modelBytes, "%s", s_forcedAttachModel);
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", s_forcedAttachJoint);
    }
    s_forcedAttachSet = false;
    s_forcedAttachModel[0] = '\0';
    s_forcedAttachJoint[0] = '\0';
    return true;
}

bool dExtNpcMount_shouldSkipBtp(const char* arcName) {
    // Per-manifest, not a create-time global (№18a — interleaved creates stomped s_skipBtpArc).
    if (arcName == NULL || arcName[0] == '\0') {
        return false;
    }
    for (const auto& kv : s_providers) {
        if (kv.second.skipBtp && strcmp(kv.second.arc, arcName) == 0) {
            return true;
        }
    }
    return false;
}

// Manifest-driven BG warp latch (Phase M). Host stage/pos/spawn from ini.
enum {
    kBgWarpIdle = 0,
    kBgWarpWaitPlayer = 1,
    kBgWarpWaitIsland = 2,
    kBgWarpWaitBg = 3,
    kBgWarpHold = 4,
};
static int s_bgWarpPhase = kBgWarpIdle;
static fpc_ProcID s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
static cXyz s_bgHoldPos;
static cXyz s_bgHostPos;
static int s_bgHoldFrames = 0;
static bool s_bgFadeWatchdogFired = false;
static int s_bgSettleFrames = 0;
// №34 P4: frames waiting for BG ready; abort → fail-safe outdoor spawn.
static int s_bgReadyWaitFrames = 0;
static bool s_bgFailSafeValid = false;
static cXyz s_bgFailSafeSpawn;
// №58-B: abort only on true failure. Soft 180f mash-timeout retired while resLoad lives.
static constexpr int kBgReadyHardTimeout = 900;  // ~15s — genuine stuck
static constexpr int kBgReadyGoneTimeout = 45;   // actor vanished after create

// №83: last createBgMountAtHost failure reason (for room0 create FAILED lines).
static char s_bgCreateFailReason[96] = {};

static fpc_ProcID createBgMountAtHost(const dExtNpcManifest& man, const char* src,
                                      int forceRoomNo = -1) {
    s_bgCreateFailReason[0] = '\0';
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (!man.isBg) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "not_bg");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    if (!man.hasHostPos) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "no_host_pos");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    // Door/warm paths need the player for roomNo. Room-lane (forceRoomNo>=0) must
    // NOT — on stage enter the room can be ready before Link exists.
    if (forceRoomNo < 0 && player == NULL) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "no_player");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    const char* socket = providerSocket(man);
    const s16 actorId = dExtNpcMount_socketActorId(socket);
    if (actorId < 0) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "bad_socket='%s'",
                      socket != NULL ? socket : "?");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    const char* spawnSrc = (src != NULL && src[0]) ? src : "bg";
    csXyz angle;
    angle.set(0, 0, 0);
    cXyz scale(1.0f, 1.0f, 1.0f);
    const int roomNo = forceRoomNo >= 0 ? forceRoomNo : fopAcM_GetRoomNo(player);
    layer_class* savedLayer = fpcLy_CurrentLayer();
    base_process_class* playScene = fpcM_SearchByName(fpcNm_PLAY_SCENE_e);
    if (playScene != NULL) {
        fpcLy_SetCurrentLayer(&((process_node_class*)playScene)->layer);
    } else if (forceRoomNo >= 0) {
        // №85: room-ready runs under the new play scene's layer during create;
        // SearchByName can miss mid-phase. Stay on current layer (do not defer).
    }
    const u32 params = man.socketArg >= 0 ? (u32)man.socketArg : 0;
    const u32 pendingSeq = dExtNpcMount_pushPendingSpawn(man.proc, spawnSrc, NULL, NULL);
    const fpc_ProcID id =
        fopAcM_create(actorId, params, &man.hostPos, roomNo, &angle, &scale, -1);
    fpcLy_SetCurrentLayer(savedLayer);
    dExtNpcMount_reapPendingSpawn(pendingSeq, id);  // №130
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason),
                      "fopAcM_create ERROR actorId=%d room=%d layer=-1 params=%08x", (int)actorId,
                      roomNo, params);
        return fpcM_ERROR_PROCESS_ID_e;
    }
    dExtNpcMount_bindPendingSpawn(id, man.proc, spawnSrc, NULL, NULL);
    return id;
}

void dExtNpcMount_registerRoomLane(const char* procName, int hostRoom) {
    if (procName == NULL || procName[0] == '\0' || hostRoom < 0 || hostRoom >= 0x40) {
        return;
    }
    s_roomLaneRooms[procName] = hostRoom;
    s_roomLaneProcByRoom[hostRoom] = procName;
    DuskLog.info("[ExtNpcMount] №62 room-lane register '{}' → room {}", procName, hostRoom);
}

bool dExtNpcMount_isRoomLaneProc(const char* procName) {
    return procName != NULL && s_roomLaneRooms.count(procName) != 0;
}

int dExtNpcMount_roomLaneHostRoom(const char* procName) {
    if (procName == NULL) {
        return -1;
    }
    auto it = s_roomLaneRooms.find(procName);
    return it != s_roomLaneRooms.end() ? it->second : -1;
}

bool dExtNpcMount_isRoomLaneProtected(int roomNo) {
    return roomNo > 0 && roomNo < 0x40 && s_roomLaneClaimed[roomNo];
}

bool dExtNpcMount_isRoomLaneUnloading(int roomNo) {
    return roomNo > 0 && roomNo < 0x40 && s_roomLaneUnloading[roomNo];
}

static bool roomLaneMountIsUnloading(const dExtNpcMount_c* m) {
    if (m == NULL) {
        return false;
    }
    if (s_roomLaneUnloadingProc[0] != '\0' && m->mManifest.proc[0] != '\0' &&
        std::strcmp(m->mManifest.proc, s_roomLaneUnloadingProc) == 0) {
        return true;
    }
    return dExtNpcMount_isRoomLaneUnloading(fopAcM_GetRoomNo(m));
}

// №68/№69: soft-forget door/pop handles + latch. Does NOT Release collision yet —
// Link must leave the interior floor first (№69).
static void forgetRoomLaneSoftHandles(const char* procName, int hostRoom) {
    if (procName == NULL || procName[0] == '\0') {
        return;
    }
    if (hostRoom > 0 && hostRoom < 0x40) {
        s_roomLaneUnloading[hostRoom] = true;
    }
    std::snprintf(s_roomLaneUnloadingProc, sizeof(s_roomLaneUnloadingProc), "%s", procName);
    DuskLog.info("[ExtNpcMount] №69 forget soft handles '{}' room{}", procName, hostRoom);

    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            const char* proc = (const char*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (!m->mManifest.doorAttention) {
                return 0;
            }
            const int r = fopAcM_GetRoomNo(m);
            const bool keyed =
                m->mDoorKey[0] &&
                (std::strstr(m->mDoorKey, "exit:") != NULL || std::strcmp(m->mDoorKey, proc) == 0);
            if (dExtNpcMount_isRoomLaneUnloading(r) || keyed) {
                m->attention_info.flags = 0;
                m->mManifest.doorAttention = false;
            }
            return 0;
        },
        (void*)procName);

    dExtNpcDoors_clearExitKnobForProc(procName);
    dExtNpcPopulation_clearForBg(procName);
}

// №69: Release interior dBgW only AFTER Link is off it, then delete the mount.
static void releaseRoomLaneMount(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        return;
    }
    auto mid = s_bgMountIds.find(procName);
    if (mid == s_bgMountIds.end()) {
        return;
    }
    if (s_bgIslandId == mid->second) {
        s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    }
    fopAc_ac_c* existing = fopAcM_SearchByID(mid->second);
    if (existing != NULL) {
        dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
        // Explicit Release before delete — never while the player stands on it.
        if (mount->mpBgW != NULL) {
            dComIfG_Bgsp().Release(mount->mpBgW);
            mount->mpBgW = NULL;
            DuskLog.info("[ExtNpcMount] №69 Released interior BgW '{}'", procName);
        }
        fopAcM_delete(existing);
    }
    s_bgMountIds.erase(mid);
    DuskLog.info("[ExtNpcMount] №69 forgot mount '{}'", procName);
}

static void forceLinkGroundReprobe(fopAc_ac_c* player) {
    if (player == NULL) {
        return;
    }
    daAlink_c* link = (daAlink_c*)player;
    link->mLinkAcch.ClrGroundHit();
    link->mLinkAcch.CrrPos(dComIfG_Bgsp());
}

static void clearRoomLaneUnloading(int hostRoom) {
    if (hostRoom > 0 && hostRoom < 0x40) {
        s_roomLaneUnloading[hostRoom] = false;
    }
    s_roomLaneUnloadingProc[0] = '\0';
}

static void activateWwHostRoom(int roomNo, const char* reason);

bool dExtNpcMount_ensureRoomLaneLoaded(int hostRoom) {
    if (hostRoom < 0 || hostRoom >= 0x40) {
        return false;
    }
    // №81: LinkRM on R_DL01 room 0 — native stage load owns room 0; no keep0+claim.
    if (hostRoom == 0) {
        return dComIfGp_roomControl_checkStatusFlag(0, 0x01) ||
               dComIfGp_roomControl_checkStatusFlag(0, 0x02);
    }
    s_roomLaneClaimed[hostRoom] = true;
    dStage_roomControl_c::setNextStayNo(hostRoom);
    // Mid-delete (0x04) — wait; another loadRoom would early-out and leave a void.
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04)) {
        return false;
    }
    // Already live or still creating — do NOT re-call loadRoom (recreate loop → crash).
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01)) {
        activateWwHostRoom(hostRoom, "ensure-loaded");
        return true;
    }
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02)) {
        return true;
    }
    // Keep exterior (room 0) + claim the interior slot. BG bit (0x80) required for create.
    u8 rooms[2];
    rooms[0] = 0x80;
    rooms[1] = static_cast<u8>(0x80 | (hostRoom & 0x3f));
    const int rt = dComIfGp_roomControl_loadRoom(2, rooms, true);
    DuskLog.info("[ExtNpcMount] №62 loadRoom keep0+claim{} rt={}", hostRoom, rt);
    return rt != 0 || dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) ||
           dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02);
}

bool dExtNpcMount_ensureRoomLaneUnloaded(int hostRoom) {
    if (hostRoom <= 0 || hostRoom >= 0x40) {
        return false;
    }
    // Drop claim first so loadRoom may schedule the kill (protect would skip it).
    s_roomLaneClaimed[hostRoom] = false;
    u8 rooms[1];
    rooms[0] = 0x80;  // exterior only
    dStage_roomControl_c::setNextStayNo(0);
    dStage_roomControl_c::setStayNo(0);
    // Fully gone (no live / creating / deleting).
    if (!dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
        !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02) &&
        !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04)) {
        return true;
    }
    // Already deleting — just wait.
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04) ||
        dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x08)) {
        return false;
    }
    const int rt = dComIfGp_roomControl_loadRoom(1, rooms, true);
    DuskLog.info("[ExtNpcMount] №62 unload room{} keep0 rt={}", hostRoom, rt);
    // rt==0 with kill flags set is NORMAL (schedule-then-wait). Done only when gone.
    return !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
           !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02) &&
           !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04);
}

// №83: defer room-lane mount until createBgMountAtHost can succeed (play scene /
// layer ready). Cleared on success or room unload.
static char s_roomMountPendingProc[32] = {};
static int s_roomMountPendingRoom = -1;
static int s_roomMountPendingTries = 0;

static void clearRoomMountPending() {
    s_roomMountPendingProc[0] = '\0';
    s_roomMountPendingRoom = -1;
    s_roomMountPendingTries = 0;
}

static bool tryCreateRoomLaneMount(const char* procName, int roomNo, const char* stageName) {
    if (procName == NULL || procName[0] == '\0' || roomNo < 0 || roomNo >= 0x40) {
        return false;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(procName, &man) || !man.isBg || !man.hasHostPos) {
        DuskLog.warn("[ExtNpcMount] №83 room{} skip '{}' — lookup/bg/hostPos fail stage='{}'",
                     roomNo, procName, stageName != NULL ? stageName : "?");
        return false;
    }
    if (man.hostStage[0] != '\0' && stageName != NULL &&
        std::strcmp(stageName, man.hostStage) != 0) {
        return false;
    }
    auto mid = s_bgMountIds.find(procName);
    if (mid != s_bgMountIds.end()) {
        if (fopAcM_SearchByID(mid->second) != NULL) {
            clearRoomMountPending();
            return true;
        }
        s_bgMountIds.erase(mid);
    }
    if (s_roomLaneMountCreating[roomNo]) {
        return false;
    }
    s_roomLaneMountCreating[roomNo] = true;
    const s16 actorId = dExtNpcMount_socketActorId(providerSocket(man));
    const fpc_ProcID id = createBgMountAtHost(man, "room", roomNo);
    s_roomLaneMountCreating[roomNo] = false;
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        DuskLog.warn(
            "[ExtNpcMount] №83 room{} create FAILED '{}' reason={} actorId={} roomNo={} "
            "host=({},{},{}) stage='{}' playScene={}",
            roomNo, procName, s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?", (int)actorId,
            roomNo, man.hostPos.x, man.hostPos.y, man.hostPos.z, stageName != NULL ? stageName : "?",
            fpcM_SearchByName(fpcNm_PLAY_SCENE_e) != NULL ? 1 : 0);
        return false;
    }
    s_bgMountIds[procName] = id;
    clearRoomMountPending();
    DuskLog.info("[ExtNpcMount] №83 room{} mount '{}' id={:08x} stage='{}'", roomNo, procName,
                 (u32)id, stageName != NULL ? stageName : "?");
    return true;
}

// №83: when arriving on a WW host stage, re-bind room-lane procs whose manifests
// declare this host (doors.ini boot register may have been for the prior stage).
static void syncRoomLaneForCurrentStage() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || stage[0] == '\0') {
        return;
    }
    const int n = dExtNpcMount_providerCount();
    for (int i = 0; i < n; ++i) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_providerAt(i, &man) || !man.isBg) {
            continue;
        }
        if (man.hostStage[0] == '\0' || std::strcmp(man.hostStage, stage) != 0) {
            continue;
        }
        if (man.hostRoom < 0 || man.hostRoom >= 0x40) {
            continue;
        }
        // Only re-register rooms already claimed as room-lane (doors.ini) OR
        // host_room explicitly set on a WW host stage (R_DL*).
        if (dExtNpcMount_isRoomLaneProc(man.proc) || dExtWwSave_isWwHostStage(stage)) {
            dExtNpcMount_registerRoomLane(man.proc, man.hostRoom);
        }
    }
}

void dExtNpcMount_onRoomObjectsReady(const char* stageName, int roomNo) {
    if (stageName == NULL || roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    // №83: WW host stages re-assert lane bindings as rooms come up.
    if (dExtWwSave_isWwHostStage(stageName)) {
        syncRoomLaneForCurrentStage();
    }
    auto it = s_roomLaneProcByRoom.find(roomNo);
    if (it == s_roomLaneProcByRoom.end()) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(it->second.c_str(), &man) || !man.isBg || !man.hasHostPos) {
        return;
    }
    if (man.hostStage[0] != '\0' && std::strcmp(stageName, man.hostStage) != 0) {
        return;
    }
    if (tryCreateRoomLaneMount(it->second.c_str(), roomNo, stageName)) {
        return;
    }
    // Defer — pollBgWarps retries until play scene / create succeeds.
    std::snprintf(s_roomMountPendingProc, sizeof(s_roomMountPendingProc), "%s", it->second.c_str());
    s_roomMountPendingRoom = roomNo;
    s_roomMountPendingTries = 0;
    DuskLog.info("[ExtNpcMount] №83 room{} mount '{}' PENDING stage='{}' reason={}", roomNo,
                 s_roomMountPendingProc, stageName, s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
}

static void pollPendingRoomLaneMount() {
    if (s_roomMountPendingProc[0] == '\0' || s_roomMountPendingRoom < 0) {
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL) {
        return;
    }
    if (!dComIfGp_roomControl_checkStatusFlag(s_roomMountPendingRoom, 0x01) &&
        !dComIfGp_roomControl_checkStatusFlag(s_roomMountPendingRoom, 0x02)) {
        return;
    }
    ++s_roomMountPendingTries;
    if (tryCreateRoomLaneMount(s_roomMountPendingProc, s_roomMountPendingRoom, stage)) {
        return;
    }
    if (s_roomMountPendingTries >= 300) {
        DuskLog.warn(
            "[ExtNpcMount] №83 room{} mount '{}' GAVE UP after {} tries lastReason={}",
            s_roomMountPendingRoom, s_roomMountPendingProc, s_roomMountPendingTries,
            s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
        clearRoomMountPending();
    }
}

void dExtNpcMount_onRoomUnload(const char* stageName, int roomNo) {
    (void)stageName;
    if (roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    if (s_roomMountPendingRoom == roomNo) {
        clearRoomMountPending();
    }
    s_roomLaneMountCreating[roomNo] = false;
    auto it = s_roomLaneProcByRoom.find(roomNo);
    if (it == s_roomLaneProcByRoom.end()) {
        return;
    }
    // №69: soft handles + mount Release should already be done (place-first exit).
    dExtNpcPopulation_clearForBg(it->second.c_str());
    dExtNpcDoors_clearExitKnobForProc(it->second.c_str());
    auto mid = s_bgMountIds.find(it->second);
    if (mid == s_bgMountIds.end()) {
        DuskLog.info("[ExtNpcMount] №69 room{} unload (mount already released) '{}'", roomNo,
                     it->second);
        return;
    }
    // Fallback path (non-exit unload): still Release before delete.
    if (s_bgIslandId == mid->second) {
        s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    }
    fopAc_ac_c* existing = fopAcM_SearchByID(mid->second);
    if (existing != NULL) {
        dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
        if (mount->mpBgW != NULL) {
            dComIfG_Bgsp().Release(mount->mpBgW);
            mount->mpBgW = NULL;
        }
        fopAcM_delete(existing);
    }
    s_bgMountIds.erase(mid);
    DuskLog.info("[ExtNpcMount] №62 room{} unload drop '{}'", roomNo, it->second);
}

static void abortWarmInteriors(const char* reason) {
    if (!s_warmInteriors) {
        return;
    }
    s_warmInteriors = false;
    s_warmCooldown = 0;
    DuskLog.warn("[ExtNpcMount] №115 warm storm ABORT — {} (idx={})",
                 reason != NULL ? reason : "?", s_warmProviderIndex);
}

static bool warmHeapHasHeadroom() {
    JKRHeap* heap = (JKRHeap*)mDoExt_getGameHeap();
    if (heap == NULL) {
        return false;
    }
    return heap->getTotalFreeSize() >= kWarmBgHeapNeed;
}

static void pollWarmInteriors() {
    if (!s_warmInteriors || s_bgWarpPhase != kBgWarpIdle) {
        return;
    }
    if (s_warmCooldown > 0) {
        --s_warmCooldown;
        return;
    }
    // №115: abort the storm when the heap cannot host another BG solid heap.
    if (!warmHeapHasHeadroom()) {
        abortWarmInteriors("free-heap below headroom");
        return;
    }

    dExtNpcManifest island{};
    const bool haveIsland = dExtNpcMount_lookup("EXT_BG0", &island) && island.hostStage[0] != '\0';

    const int n = dExtNpcMount_providerCount();
    while (s_warmProviderIndex < n) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_providerAt(s_warmProviderIndex++, &man) || !man.isBg ||
            man.proc[0] == '\0') {
            continue;
        }
        if (std::strcmp(man.proc, "EXT_BG0") == 0) {
            continue;
        }
        // Shelved Omori — skip warm (no door traffic).
        if (std::strcmp(man.proc, "EXT_BG7") == 0) {
            continue;
        }
        // №62: room-lane mounts bind to room load — do not eternal-warm.
        if (s_roomLaneRooms.count(man.proc) != 0) {
            continue;
        }
        // №115: data opt-out.
        if (!man.allowWarm) {
            continue;
        }
        // №115: foreign-stage BGs (forest/cave/fountain own stages) must not warm on island.
        if (haveIsland && man.hostStage[0] != '\0' &&
            std::strcmp(man.hostStage, island.hostStage) != 0) {
            continue;
        }
        auto it = s_bgMountIds.find(man.proc);
        if (it != s_bgMountIds.end()) {
            fopAc_ac_c* existing = fopAcM_SearchByID(it->second);
            if (existing != NULL) {
                continue;  // already created (ready or still loading)
            }
            s_bgMountIds.erase(it);
        }
        const fpc_ProcID id = createBgMountAtHost(man, "warm");
        if (id == fpcM_ERROR_PROCESS_ID_e) {
            DuskLog.warn("[ExtNpcMount] №58-B warm create FAILED '{}' reason='{}'", man.proc,
                         s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
            // №115: first hard fail ends the storm (do not walk into heap exhaustion).
            abortWarmInteriors("create FAILED");
            return;
        }
        s_bgMountIds[man.proc] = id;
        s_warmCooldown = kWarmCooldownFrames;
        DuskLog.info("[ExtNpcMount] №58-B warm create '{}' id={:08x}", man.proc, (u32)id);
        return;
    }
    s_warmInteriors = false;
    DuskLog.info("[ExtNpcMount] №58-B warm interiors done ({} providers scanned)", n);
}

static bool beginBgWarp(const char* procName, bool hasSpawnOverride, const cXyz& spawnWorld) {
    dExtNpcManifest man{};
    if (procName == NULL || !dExtNpcMount_lookup(procName, &man) || !man.isBg ||
        !man.hasHostPos || !man.hasAnchor || !man.hasSpawnRel || man.hostStage[0] == '\0') {
        DuskLog.warn(
            "[ExtNpcMount] BG warp refused — need type=bg + host_stage + host_pos + "
            "anchor + spawn_rel");
        return false;
    }
    s_bgSpawnOverrideValid = hasSpawnOverride;
    if (hasSpawnOverride) {
        s_bgSpawnOverride = spawnWorld;
    } else {
        s_bgSpawnFacingValid = false;
    }
    s_bgWarpPending = true;
    s_bgWarpPhase = kBgWarpWaitPlayer;
    s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    s_bgHoldFrames = 0;
    s_bgSettleFrames = 0;
    s_bgReadyWaitFrames = 0;
    s_bgFadeWatchdogFired = false;
    s_bgHostPos = man.hostPos;
    snprintf(s_bgWarpProc, sizeof(s_bgWarpProc), "%s", procName);
    DuskLog.info("[ExtNpcMount] BG warp '{}' → host {} r{} then relocate{}", procName,
                 man.hostStage, man.hostRoom, hasSpawnOverride ? " (spawn override)" : "");
    return true;
}

bool dExtNpcMount_requestBgWarp(const char* procName) {
    s_bgFailSafeValid = false;
    return beginBgWarp(procName, false, cXyz());
}

bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld) {
    s_bgFailSafeValid = false;
    s_bgSpawnFacingValid = false;
    return beginBgWarp(procName, true, spawnWorld);
}

bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld, s16 facing) {
    s_bgFailSafeValid = false;
    s_bgSpawnFacingValid = true;
    s_bgSpawnFacing = facing;
    return beginBgWarp(procName, true, spawnWorld);
}

bool dExtNpcMount_requestBgWarpGuarded(const char* procName, const cXyz& failSafeSpawn) {
    s_bgFailSafeValid = true;
    s_bgFailSafeSpawn = failSafeSpawn;
    return beginBgWarp(procName, false, cXyz());
}

const char* dExtNpcMount_lastBgProc() {
    return s_lastBgProc;
}

static void placeLinkAt(fopAc_ac_c* player, const cXyz& spawn);
static bool beginBgWarp(const char* procName, bool hasSpawnOverride, const cXyz& spawnWorld);

// №65/№66 — room-lane transport: cover → change room → place → lift (never beginBgWarp).
enum {
    kRoomTxIdle = 0,
    kRoomTxEnterCover = 1,  // №66-A: wait until screen is black before room work
    kRoomTxEnterWait = 2,   // load + mount + room draw ready
    kRoomTxEnterHold = 3,
    kRoomTxExitCover = 4,   // №66-A: black before unload/place
    kRoomTxExitUnload = 5,  // №66-B: wait until room fully gone
    kRoomTxExitHold = 6,
};
static int s_roomTxPhase = kRoomTxIdle;
static char s_roomTxProc[32] = {};
static cXyz s_roomTxFailSafe;
static bool s_roomTxFailSafeValid = false;
static cXyz s_roomTxReturn;
static s16 s_roomTxFacing = 0;
static bool s_roomTxHasFacing = false;
static cXyz s_roomTxHoldPos;
static int s_roomTxHoldFrames = 0;
static int s_roomTxWaitFrames = 0;
static bool s_roomTxFadeWatch = false;
static bool s_roomTxForgot = false;

static void roomTxForceCover() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader == NULL) {
        return;
    }
    if (fader->getStatus() != JUTFader::FadeOut && fader->getStatus() != JUTFader::Wait) {
        mDoGph_gInf_c::startFadeOut(8);
    }
}

static bool roomTxScreenBlack() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader == NULL) {
        return true;
    }
    // Wait = solid black. FadeOut alone is mid-cover — not enough to hide the place.
    return fader->getStatus() == JUTFader::Wait;
}

static void roomTxStartFadeIn() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader != NULL &&
        (fader->getStatus() == JUTFader::FadeOut || fader->getStatus() == JUTFader::Wait)) {
        fader->setStatus(JUTFader::None, 0);
    }
    mDoGph_gInf_c::startFadeIn(15);
}

static void roomTxAssignPlayerRoom(fopAc_ac_c* player, int roomNo) {
    if (player == NULL || roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    fopAcM_SetRoomNo(player, (s8)roomNo);
    player->tevStr.room_no = (s8)roomNo;
    // Layer move only when the room scene exists (else assert in setRoomLayer).
    if (dComIfGp_roomControl_checkStatusFlag(roomNo, 0x01)) {
        fopAcM_setRoomLayer(player, roomNo);
    }
}

static bool roomTxDrawable(int hostRoom) {
    return hostRoom > 0 && dComIfGp_roomControl_checkRoomDisp(hostRoom) != FALSE;
}

// №104: WW host rooms have no retail daBg (68-byte FILI+PLYR skeleton), so flag 0x10
// never gets set — rooms 1–5 stay invisible/non-solid. Mirror daBg_Create's onStatusFlag.
static void activateWwHostRoom(int roomNo, const char* reason) {
    if (roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    dComIfGp_roomControl_offStatusFlag(roomNo, 0x08);  // clear hide
    dComIfGp_roomControl_onStatusFlag(roomNo, 0x10);   // drawable/solid (daBg)
    dStage_roomControl_c::setStayNo(roomNo);           // mDraw + stay
    DuskLog.info("[ExtNpcMount] №104 activate room{} ({}) disp={}", roomNo,
                 reason != NULL ? reason : "?",
                 dComIfGp_roomControl_checkRoomDisp(roomNo) != FALSE ? 1 : 0);
}

bool dExtNpcMount_roomLaneTransportBusy() {
    return s_roomTxPhase != kRoomTxIdle;
}

bool dExtNpcMount_bgWarpBusy() {
    return s_bgWarpPhase != kBgWarpIdle || s_roomTxPhase != kRoomTxIdle;
}

bool dExtNpcMount_requestRoomLaneEnter(const char* procName, const cXyz& failSafeSpawn) {
    dExtNpcManifest man{};
    if (procName == NULL || !dExtNpcMount_lookup(procName, &man) || !man.isBg ||
        !man.hasHostPos || !man.hasSpawnRel) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — incomplete manifest '{}'",
                     procName ? procName : "(null)");
        return false;
    }
    const int hostRoom = dExtNpcMount_roomLaneHostRoom(procName);
    if (hostRoom < 0) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — '{}' not registered", procName);
        return false;
    }
    if (s_roomTxPhase != kRoomTxIdle || s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — transport busy");
        return false;
    }
    std::snprintf(s_roomTxProc, sizeof(s_roomTxProc), "%s", procName);
    s_roomTxFailSafe = failSafeSpawn;
    s_roomTxFailSafeValid = true;
    s_roomTxWaitFrames = 0;
    s_roomTxFadeWatch = false;
    roomTxForceCover();
    s_roomTxPhase = kRoomTxEnterCover;
    DuskLog.info("[ExtNpcMount] №66 room-lane ENTER cover→load '{}' room={} (no BgWarp)",
                 procName, hostRoom);
    return true;
}

bool dExtNpcMount_requestRoomLaneExit(const char* procName, const cXyz& returnWorld, s16 facing,
                                       bool hasFacing) {
    if (procName == NULL || procName[0] == '\0') {
        return false;
    }
    if (s_roomTxPhase != kRoomTxIdle || s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane exit refused — transport busy");
        return false;
    }
    std::snprintf(s_roomTxProc, sizeof(s_roomTxProc), "%s", procName);
    s_roomTxReturn = returnWorld;
    s_roomTxFacing = facing;
    s_roomTxHasFacing = hasFacing;
    s_roomTxFailSafeValid = false;
    s_roomTxWaitFrames = 0;
    s_roomTxFadeWatch = false;
    s_roomTxForgot = false;
    roomTxForceCover();
    s_roomTxPhase = kRoomTxExitCover;
    DuskLog.info("[ExtNpcMount] №66 room-lane EXIT cover→unload '{}' (no BgWarp)", procName);
    return true;
}

static void pollRoomLaneTransport() {
    if (s_roomTxPhase == kRoomTxIdle) {
        return;
    }

    // №66-A enter: stay black before touching room memory.
    if (s_roomTxPhase == kRoomTxEnterCover) {
        roomTxForceCover();
        ++s_roomTxWaitFrames;
        if (!roomTxScreenBlack() && s_roomTxWaitFrames < 45) {
            return;
        }
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        if (hostRoom >= 0) {
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
        }
        s_roomTxWaitFrames = 0;
        s_roomTxPhase = kRoomTxEnterWait;
        DuskLog.info("[ExtNpcMount] №66 room-lane ENTER load '{}'", s_roomTxProc);
        return;
    }

    if (s_roomTxPhase == kRoomTxEnterWait) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_lookup(s_roomTxProc, &man) || !man.isBg) {
            s_roomTxPhase = kRoomTxIdle;
            dExtNpcMount_endDoorDemoLock();
            return;
        }
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        if (hostRoom >= 0) {
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
        }
        roomTxForceCover();
        fpc_ProcID mid = fpcM_ERROR_PROCESS_ID_e;
        auto it = s_bgMountIds.find(s_roomTxProc);
        if (it != s_bgMountIds.end()) {
            mid = it->second;
        }
        fopAc_ac_c* island = mid != fpcM_ERROR_PROCESS_ID_e ? fopAcM_SearchByID(mid) : NULL;
        dExtNpcMount_c* mount = island != NULL ? (dExtNpcMount_c*)island : NULL;
        const bool mountReady =
            mount != NULL && mount->mIsBg && mount->mBgReady && mount->mpBgW != NULL;
        // №66-B: wait for room draw bit (daBg sets 0x10) — mount alone is not enough.
        const bool roomReady = hostRoom < 0 || roomTxDrawable(hostRoom);
        if (!mountReady || !roomReady) {
            ++s_roomTxWaitFrames;
            if (s_roomTxFailSafeValid && s_roomTxWaitFrames >= kBgReadyHardTimeout) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=room_tx_timeout after {}f "
                    "(mount={} roomDisp={}) — fail-safe",
                    s_roomTxProc, s_roomTxWaitFrames, mountReady ? 1 : 0, roomReady ? 1 : 0);
                const cXyz back = s_roomTxFailSafe;
                s_roomTxPhase = kRoomTxIdle;
                s_roomTxFailSafeValid = false;
                dExtNpcMount_endDoorDemoLock();
                if (hostRoom >= 0) {
                    dExtNpcMount_ensureRoomLaneUnloaded(hostRoom);
                }
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            return;
        }

        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }
        updateBgTransform(mount);
        if (mount->mpBgW != NULL && !mount->mBgGlobal) {
            mount->mpBgW->Move();
        }
        cXyz spawn = man.hostPos + man.spawnRel;
        const f32 refY = spawn.y;
        if (!dExtNpcMount_localGroundSnap(&spawn, refY)) {
            spawn.y = refY + 50.0f;
        }
        placeLinkAt(player, spawn);
        if (hostRoom >= 0) {
            roomTxAssignPlayerRoom(player, hostRoom);
            activateWwHostRoom(hostRoom, "room-lane-enter");
        }
        if (man.hasSpawnRy) {
            player->current.angle.y = man.spawnRy;
            player->shape_angle.y = man.spawnRy;
        }
        s_roomTxHoldPos = spawn;
        s_roomTxHoldFrames = 120;
        s_bgIslandId = mid;
        roomTxStartFadeIn();
        dExtNpcPopulation_spawnForBg(man);
        DuskLog.info(
            "[ExtNpcMount] №66 Link placed room-lane '{}' at spawn_rel → ({:.0f},{:.0f},{:.0f}) "
            "roomDisp={}",
            s_roomTxProc, spawn.x, spawn.y, spawn.z, roomReady ? 1 : 0);
        s_roomTxPhase = kRoomTxEnterHold;
        return;
    }

    // №66-A exit: black, then unload (symmetric wait), then place.
    if (s_roomTxPhase == kRoomTxExitCover) {
        roomTxForceCover();
        ++s_roomTxWaitFrames;
        if (!roomTxScreenBlack() && s_roomTxWaitFrames < 45) {
            return;
        }
        s_roomTxWaitFrames = 0;
        s_roomTxPhase = kRoomTxExitUnload;
        return;
    }

    if (s_roomTxPhase == kRoomTxExitUnload) {
        roomTxForceCover();
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        // №69: place Link on island FIRST → Release interior BgW → THEN free room.
        // Never Release a dBgW the player is standing on.
        if (!s_roomTxForgot) {
            fopAc_ac_c* player = dComIfGp_getPlayer(0);
            if (player == NULL) {
                s_roomTxPhase = kRoomTxIdle;
                dExtNpcMount_endDoorDemoLock();
                return;
            }
            forgetRoomLaneSoftHandles(s_roomTxProc, hostRoom);

            cXyz spawn = s_roomTxReturn;
            dExtNpcMount_localGroundSnap(&spawn, s_roomTxReturn.y);
            placeLinkAt(player, spawn);
            roomTxAssignPlayerRoom(player, 0);
            dStage_roomControl_c::setStayNo(0);
            if (s_roomTxHasFacing) {
                player->current.angle.y = s_roomTxFacing;
                player->shape_angle.y = s_roomTxFacing;
            }
            forceLinkGroundReprobe(player);
            s_roomTxHoldPos = spawn;
            DuskLog.info(
                "[ExtNpcMount] №69 room-lane EXIT place-first '{}' → porch "
                "({:.0f},{:.0f},{:.0f})",
                s_roomTxProc, spawn.x, spawn.y, spawn.z);

            releaseRoomLaneMount(s_roomTxProc);
            s_roomTxForgot = true;
            s_roomTxWaitFrames = 0;
        }
        bool gone = true;
        if (hostRoom >= 0) {
            gone = dExtNpcMount_ensureRoomLaneUnloaded(hostRoom);
        }
        ++s_roomTxWaitFrames;
        if (!gone && s_roomTxWaitFrames < 180) {
            return;
        }
        if (!gone) {
            DuskLog.warn(
                "[ExtNpcMount] №69 room-lane EXIT unload slow room{} after {}f — continue",
                hostRoom, s_roomTxWaitFrames);
        }
        clearRoomLaneUnloading(hostRoom);
        s_roomTxForgot = false;
        s_roomTxHoldFrames = 45;
        roomTxStartFadeIn();
        std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "EXT_BG0");
        DuskLog.info("[ExtNpcMount] №69 room-lane EXIT complete — active EXT_BG0");
        s_roomTxPhase = kRoomTxExitHold;
        return;
    }

    if (s_roomTxPhase == kRoomTxEnterHold || s_roomTxPhase == kRoomTxExitHold) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - s_roomTxHoldPos.x;
            const f32 dz = player->current.pos.z - s_roomTxHoldPos.z;
            if ((dx * dx + dz * dz) > (5000.0f * 5000.0f)) {
                placeLinkAt(player, s_roomTxHoldPos);
            } else {
                player->speed.x = 0.0f;
                player->speed.z = 0.0f;
            }
        }
        if (s_roomTxPhase == kRoomTxEnterHold) {
            fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
            if (island != NULL) {
                dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
                if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                    updateBgTransform(mount);
                    mount->mpBgW->Move();
                }
            }
        }
        if (!s_roomTxFadeWatch && s_roomTxHoldFrames <= 90) {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL &&
                (fader->getStatus() == JUTFader::None ||
                 fader->getStatus() == JUTFader::FadeOut)) {
                fader->setStatus(JUTFader::None, 0);
                if (mDoGph_gInf_c::startFadeIn(10)) {
                    s_roomTxFadeWatch = true;
                }
            } else if (fader != NULL && (fader->getStatus() == JUTFader::FadeIn ||
                                         fader->getStatus() == JUTFader::Wait)) {
                s_roomTxFadeWatch = true;
            }
        }
        if (--s_roomTxHoldFrames <= 0) {
            if (s_roomTxPhase == kRoomTxEnterHold) {
                std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_roomTxProc);
            } else {
                std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "EXT_BG0");
            }
            DuskLog.info("[ExtNpcMount] №66 room-lane transport complete — active '{}'",
                         s_lastBgProc);
            s_roomTxPhase = kRoomTxIdle;
            dExtNpcMount_endDoorDemoLock();
        }
    }
}

// №84: after a native stage change back to the exterior, remount EXT_BG0 etc.
static char s_stageExitRemountProc[32] = {};
static cXyz s_stageExitRemountSpawn;
static s16 s_stageExitRemountFacing = 0;
static bool s_stageExitRemountHasFacing = false;
static bool s_stageExitRemountArmed = false;

void dExtNpcMount_cancelTransports() {
    if (s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.info("[ExtNpcMount] №84 cancel BG transport phase={}", s_bgWarpPhase);
        s_bgWarpPhase = kBgWarpIdle;
        s_bgWarpPending = false;
        s_bgFailSafeValid = false;
        s_bgReadyWaitFrames = 0;
    }
    if (s_roomTxPhase != kRoomTxIdle) {
        DuskLog.info("[ExtNpcMount] №84 cancel room-lane transport phase={}", s_roomTxPhase);
        s_roomTxPhase = kRoomTxIdle;
        s_roomTxProc[0] = '\0';
        s_roomTxFailSafeValid = false;
        s_roomTxWaitFrames = 0;
    }
    clearRoomMountPending();
}

void dExtNpcMount_armStageExitRemount(const char* procName, const cXyz& spawnWorld, s16 facing,
                                       bool hasFacing) {
    if (procName == NULL || procName[0] == '\0') {
        s_stageExitRemountArmed = false;
        s_stageExitRemountProc[0] = '\0';
        return;
    }
    std::snprintf(s_stageExitRemountProc, sizeof(s_stageExitRemountProc), "%s", procName);
    s_stageExitRemountSpawn = spawnWorld;
    s_stageExitRemountFacing = facing;
    s_stageExitRemountHasFacing = hasFacing;
    s_stageExitRemountArmed = true;
    DuskLog.info("[ExtNpcMount] №84 arm stage-exit remount '{}' spawn=({:.1f},{:.1f},{:.1f})",
                 s_stageExitRemountProc, spawnWorld.x, spawnWorld.y, spawnWorld.z);
}

static void tryStageExitRemount() {
    if (!s_stageExitRemountArmed || s_stageExitRemountProc[0] == '\0') {
        return;
    }
    if (fpcM_SearchByName(fpcNm_PLAY_SCENE_e) == NULL || dComIfGp_getPlayer(0) == NULL) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_stageExitRemountProc, &man) || !man.isBg) {
        s_stageExitRemountArmed = false;
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (man.hostStage[0] != '\0' &&
        (stage == NULL || std::strcmp(stage, man.hostStage) != 0)) {
        return;  // not on the exterior host yet
    }
    s_stageExitRemountArmed = false;
    if (s_stageExitRemountHasFacing) {
        s_bgSpawnFacingValid = true;
        s_bgSpawnFacing = s_stageExitRemountFacing;
    } else {
        s_bgSpawnFacingValid = false;
    }
    const bool ok = beginBgWarp(s_stageExitRemountProc, true, s_stageExitRemountSpawn);
    DuskLog.info("[ExtNpcMount] №84 stage-exit remount '{}' ok={} stage='{}'",
                 s_stageExitRemountProc, ok ? 1 : 0, stage != NULL ? stage : "?");
}

u32 dExtNpcWorld_generation() {
    return s_worldGeneration;
}

void dExtNpcWorld_bump(const char* reason) {
    ++s_worldGeneration;
    if (s_worldGeneration == 0) {
        s_worldGeneration = 1;  // skip 0
    }
    dExtNpcDoors_clearSpawnLatches();
    dExtNpcPopulation_clearAll();
    s_interiorBootstrapProc[0] = '\0';
    DuskLog.info("[ExtNpcWorld] №94 bump gen={} ({})", s_worldGeneration,
                 reason != NULL ? reason : "?");
}

void dExtNpcMount_onStageReady() {
    // №94: every play-scene Create tears down actors — bump so spawn latches re-arm.
    const char* stage = dComIfGp_getStartStageName();
    char reason[48];
    if (stage != NULL && s_worldGenStage[0] != '\0' &&
        std::strcmp(stage, s_worldGenStage) == 0) {
        std::snprintf(reason, sizeof(reason), "recreate '%s'", stage);
    } else {
        std::snprintf(reason, sizeof(reason), "enter '%s'", stage != NULL ? stage : "?");
    }
    dExtNpcWorld_bump(reason);
    if (stage != NULL) {
        std::snprintf(s_worldGenStage, sizeof(s_worldGenStage), "%s", stage);
    } else {
        s_worldGenStage[0] = '\0';
    }
    // №83: re-bind room-lane procs for the stage we just entered (R_DL01 etc.).
    syncRoomLaneForCurrentStage();
    // №84: remount exterior after native stage change; release door demo lock.
    tryStageExitRemount();
    dExtNpcMount_endDoorDemoLock();
    // №90: warp (and any WW-host entry without a door arm) gets a lane-agnostic G-guard.
    if (dExtWwSave_isWwHostStage(stage)) {
        dExtNpcDoors_armArrivalGuard(stage);
    }
}

static void placeLinkAt(fopAc_ac_c* player, const cXyz& spawn) {
    player->current.pos = spawn;
    player->old.pos = spawn;
    player->home.pos = spawn;
    player->speed.x = 0.0f;
    player->speed.y = 0.0f;
    player->speed.z = 0.0f;
    player->speedF = 0.0f;
}

// №54-4: probe from refY+50 DOWN; accept only within ±250 of refY (rejects roofs/sea floor).
bool dExtNpcMount_localGroundSnap(cXyz* pos, f32 refY) {
    if (pos == NULL) {
        return false;
    }
    cXyz probe(pos->x, refY + 50.0f, pos->z);
    if (!fopAcM_gc_c::gndCheck(&probe)) {
        DuskLog.warn("[Doors] local probe miss at ({:.1f},{:.1f},{:.1f}) refY={:.1f}", probe.x,
                     probe.y, probe.z, refY);
        pos->y = refY + 50.0f;
        return false;
    }
    const f32 gy = fopAcM_gc_c::getGroundY() + 50.0f;
    if (gy > refY + 250.0f || gy < refY - 250.0f) {
        DuskLog.warn(
            "[Doors] local probe y={:.1f} outside ±250 of refY={:.1f} — keep authored", gy, refY);
        pos->y = refY + 50.0f;
        return false;
    }
    pos->y = gy;
    DuskLog.info("[ExtNpcMount] BG ground hit y={} probe=({:.1f},{:.1f},{:.1f}) local", pos->y,
                 probe.x, probe.y, probe.z);
    return true;
}

// №21 probes — classify draw vs dBgW vs cBgS routing (kept for BG warps).
static void logBgWarpProbes(dExtNpcMount_c* mount, const cXyz& hostVillage) {
    if (mount == NULL || mount->mpBgW == NULL) {
        DuskLog.warn("[ExtNpcMount:BG] probe aborted — mount/bgw null");
        return;
    }
    dBgW* bgw = mount->mpBgW;
    // №22: local models use cell-local village; world-baked path used absolute GS vtx.
    const cXyz probeVtx = mount->mManifest.modelSpaceLocal ?
                              cXyz(-1651.0f, 163.0f, 11989.0f) :
                              cXyz(-201651.0f, 163.0f, 311989.0f);
    const f32 expectX = hostVillage.x;
    const f32 expectY = hostVillage.y - 20.0f;  // spawn_rel has +20 clearance
    const f32 expectZ = hostVillage.z;

    // P0 / P0b — draw pipeline
    if (mount->mpBgModels[0] != NULL) {
        MtxP base = mount->mpBgModels[0]->getBaseTRMtx();
        cXyz drawn;
        cMtx_multVec(base, &probeVtx, &drawn);
        DuskLog.info(
            "[ExtNpcMount:P0] village vtx via model[0] baseTRMtx → ({}, {}, {}) "
            "expect≈({},{},{}) model_space={}",
            drawn.x, drawn.y, drawn.z, expectX, expectY, expectZ,
            mount->mManifest.modelSpaceLocal ? "local" : "world");
        DuskLog.info("[ExtNpcMount:P0b] model[0] baseTRMtx T=({}, {}, {})", base[0][3], base[1][3],
                     base[2][3]);
    } else {
        DuskLog.warn("[ExtNpcMount:P0] model[0] NULL");
    }
    DuskLog.info(
        "[ExtNpcMount:P0b] mBgMtx T=({}, {}, {}) pm_base={} same_ptr={} lock={} nocalc={} flags={:02x} "
        "moveCtr={} model_space={}",
        mount->mBgMtx[0][3], mount->mBgMtx[1][3], mount->mBgMtx[2][3], (void*)bgw->pm_base,
        bgw->pm_base == &mount->mBgMtx, bgw->ChkLock(), (int)bgw->ChkNoCalcVtx(), bgw->mFlags,
        bgw->mMoveCounter, mount->mManifest.modelSpaceLocal ? "local" : "world");

    // P1 — collision tree AABB location
    if (bgw->pm_grp != NULL) {
        const u16 root = bgw->m_rootGrpIdx;
        const cM3dGAab& aab = bgw->pm_grp[root].m_aab;
        DuskLog.info(
            "[ExtNpcMount:P1] rootGrp={} aab min=({}, {}, {}) max=({}, {}, {}) "
            "(host≈-60k/-90k, GS≈-200k/+315k)",
            root, aab.GetMinX(), aab.GetMinY(), aab.GetMinZ(), aab.GetMaxX(), aab.GetMaxY(),
            aab.GetMaxZ());
    } else {
        DuskLog.warn("[ExtNpcMount:P1] pm_grp NULL");
    }

    // P2 — direct dBgW GroundCross (bypass cBgS list)
    cXyz probe(hostVillage.x, hostVillage.y + 2000.0f, hostVillage.z);
    dBgS_ObjGndChk chk;
    chk.SetPos(&probe);
    chk.SetNowY(-G_CM3D_F_INF);
    const bool p2 = bgw->GroundCross(&chk);
    DuskLog.info("[ExtNpcMount:P2] direct GroundCross hit={} y={} probe=({}, {}, {})", p2,
                 chk.GetNowY(), probe.x, probe.y, probe.z);

    // P3 — same with PolyPassChk cleared (ti through-filter test)
    dBgS_ObjGndChk chk3;
    chk3.SetPos(&probe);
    chk3.SetNowY(-G_CM3D_F_INF);
    chk3.SetPolyPassChk(NULL);
    const bool p3 = bgw->GroundCross(&chk3);
    DuskLog.info("[ExtNpcMount:P3] GroundCross PolyPassChk=NULL hit={} y={}", p3, chk3.GetNowY());

    if (!p3 && bgw->pm_blk != NULL && bgw->pm_bgd != NULL) {
        int withGnd = 0;
        int emptyGnd = 0;
        const int bnum = bgw->pm_bgd->m_b_num;
        for (int i = 0; i < bnum; ++i) {
            if (bgw->pm_blk[i].m_gnd_idx != 0xFFFF) {
                ++withGnd;
            } else {
                ++emptyGnd;
            }
        }
        DuskLog.warn(
            "[ExtNpcMount:P3miss] blocks={} with_gnd={} empty_gnd={} vtx_num={} tri_num={} "
            "vtx_tbl={}",
            bnum, withGnd, emptyGnd, bgw->GetVtxNum(), (int)bgw->pm_bgd->m_t_num,
            (void*)bgw->GetVtxTbl());
    }
}

static void tryInteriorBootstrap() {
    if (s_interiorBootstrapProc[0] == '\0') {
        return;
    }
    if (dComIfGp_getPlayer(0) == NULL) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_interiorBootstrapProc, &man) || !man.isBg) {
        s_interiorBootstrapProc[0] = '\0';
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (man.hostStage[0] != '\0' &&
        (stage == NULL || std::strcmp(stage, man.hostStage) != 0)) {
        return;  // not on host yet
    }
    std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_interiorBootstrapProc);
    // №104: activate host room before pop (rooms != 0 have no daBg to set 0x10).
    if (man.hostRoom >= 0 && man.hostRoom < 0x40 &&
        (dExtWwSave_isWwHostStage(stage) || dExtNpcMount_isRoomLaneProc(s_interiorBootstrapProc))) {
        activateWwHostRoom(man.hostRoom, "bootstrap");
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            roomTxAssignPlayerRoom(player, man.hostRoom);
        }
    }
    dExtNpcPopulation_spawnForBg(man);
    if (std::strcmp(s_interiorBootstrapProc, "EXT_BG0") == 0) {
        dExtNpcDoors_spawnKnobs(man);
        DuskLog.info(
            "[ExtNpcMount] №104 exterior bootstrap '{}' stage='{}' — population + outdoor knobs",
            s_interiorBootstrapProc, stage != NULL ? stage : "?");
    } else {
        dExtNpcDoors_onInteriorBgReady(s_interiorBootstrapProc);
        DuskLog.info(
            "[ExtNpcMount] №90 interior bootstrap '{}' stage='{}' — population + exit knob",
            s_interiorBootstrapProc, stage != NULL ? stage : "?");
    }
    s_interiorBootstrapProc[0] = '\0';
}

// №94/№104: empty world after soft reload / exterior re-entry ⇒ re-run + log.
static void trySpawnSelfHeal() {
    if (dComIfGp_getPlayer(0) == NULL || s_lastBgProc[0] == '\0') {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_lastBgProc, &man) || !man.isBg) {
        return;
    }
    if (std::strcmp(s_lastBgProc, "EXT_BG0") == 0) {
        const int want = dExtNpcDoors_wantOutdoorKnobCount();
        const int live = dExtNpcDoors_countLiveOutdoorKnobs();
        if (want > 0 && live == 0) {
            DuskLog.warn(
                "[ExtNpcMount] №94 self-heal doors — '{}' want={} live=0 latched={} → respawn",
                s_lastBgProc, want, dExtNpcDoors_knobsLatched() ? 1 : 0);
            dExtNpcDoors_clearSpawnLatches();
            dExtNpcDoors_spawnKnobs(man);
        }
    }
    if (man.populationCsv[0] == '\0') {
        return;
    }
    if (dExtNpcPopulation_countLiveCensus() != 0) {
        return;
    }
    // №104: post-bump exterior often never latched (spawn skipped) — still re-run.
    if (!dExtNpcPopulation_isLatched(s_lastBgProc)) {
        DuskLog.warn(
            "[ExtNpcMount] №104 self-heal census — unlatched '{}' live=0 → spawn (gen mismatch)",
            s_lastBgProc);
        dExtNpcPopulation_spawnForBg(man);
        return;
    }
    DuskLog.warn(
        "[ExtNpcMount] №94 self-heal census — COMPLEATE '{}' live=0 → re-run population",
        s_lastBgProc);
    dExtNpcPopulation_clearForBg(s_lastBgProc);
    dExtNpcPopulation_spawnForBg(man);
}

void dExtNpcMount_pollBgWarps() {
    // №83: drain deferred room-lane creates (room ready before play-scene/layer).
    pollPendingRoomLaneMount();
    // №84: exterior remount may arm before player exists in onStageReady.
    tryStageExitRemount();
    // №90: population + exit knob for WW-host BG mounts (COMPLEATE may precede player).
    tryInteriorBootstrap();
    // №94: empty-world net — latched but no live actors after soft reload.
    trySpawnSelfHeal();
    if (s_roomTxPhase != kRoomTxIdle) {
        pollRoomLaneTransport();
        return;
    }
    if (s_bgWarpPhase == kBgWarpIdle) {
        pollWarmInteriors();
        return;
    }

    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_bgWarpProc, &man) || !man.isBg || !man.hasHostPos) {
        DuskLog.warn("[ExtNpcMount] BG warp aborted — payload '{}' missing/incomplete", s_bgWarpProc);
        s_bgWarpPhase = kBgWarpIdle;
        s_bgWarpPending = false;
        dExtNpcMount_endDoorDemoLock();
        return;
    }
    s_bgHostPos = man.hostPos;

    if (s_bgWarpPhase == kBgWarpWaitPlayer) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }
        const char* stage = dComIfGp_getStartStageName();
        if (stage == NULL || strcmp(stage, man.hostStage) != 0) {
            return;
        }

        // Reuse an existing BG mount for this proc (door house↔island).
        // №25 F1: refuse reuse unless mManifest.proc still matches.
        // №58-B: if warm/cold create is still loading, WAIT — do not erase+recreate (mash).
        {
            auto it = s_bgMountIds.find(s_bgWarpProc);
            if (it != s_bgMountIds.end()) {
                fopAc_ac_c* existing = fopAcM_SearchByID(it->second);
                if (existing != NULL) {
                    dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
                    if (mount->mIsBg && strcmp(mount->mManifest.proc, s_bgWarpProc) == 0) {
                        s_bgIslandId = it->second;
                        if (mount->mBgReady && mount->mpBgW != NULL) {
                            s_bgSettleFrames = 4;
                            s_bgWarpPhase = kBgWarpWaitBg;
                            DuskLog.info("[ExtNpcMount] BG reuse '{}' id={:08x}", s_bgWarpProc,
                                         (u32)s_bgIslandId);
                        } else {
                            s_bgWarpPhase = kBgWarpWaitIsland;
                            DuskLog.info(
                                "[ExtNpcMount] BG wait-load '{}' id={:08x} (warm/cold in flight)",
                                s_bgWarpProc, (u32)s_bgIslandId);
                        }
                        return;
                    }
                    DuskLog.warn(
                        "[ExtNpcMount] BG reuse rejected '{}' — cached id={:08x} proc='{}' "
                        "ready={}",
                        s_bgWarpProc, (u32)it->second, mount->mManifest.proc,
                        mount->mIsBg && mount->mBgReady);
                }
                s_bgMountIds.erase(it);
            }
        }

        // №62: room-lane mounts are created by d_s_room — wait, do not self-create.
        if (s_roomLaneRooms.count(s_bgWarpProc) != 0) {
            const int hostRoom = s_roomLaneRooms[s_bgWarpProc];
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
            // Room already live but mount missing (e.g. prior kill) — rebind once.
            if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
                s_bgMountIds.count(s_bgWarpProc) == 0) {
                dExtNpcMount_onRoomObjectsReady(dComIfGp_getStartStageName(), hostRoom);
            }
            ++s_bgReadyWaitFrames;
            if (s_bgFailSafeValid && s_bgReadyWaitFrames >= kBgReadyHardTimeout) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=room_lane_timeout after {}f — return to "
                    "fail-safe",
                    s_bgWarpProc, s_bgReadyWaitFrames);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            if ((s_bgReadyWaitFrames % 30) == 1) {
                DuskLog.info("[ExtNpcMount] №62 wait room-lane mount '{}' room={} f={}",
                             s_bgWarpProc, hostRoom, s_bgReadyWaitFrames);
            }
            return;
        }

        s_bgIslandId = createBgMountAtHost(man, "door");
        if (s_bgIslandId == fpcM_ERROR_PROCESS_ID_e) {
            DuskLog.warn("[ExtNpcMount] BG island fopAcM_create FAILED for '{}'", s_bgWarpProc);
            s_bgWarpPhase = kBgWarpIdle;
            s_bgWarpPending = false;
            dExtNpcMount_endDoorDemoLock();
            return;
        }
        s_bgMountIds[s_bgWarpProc] = s_bgIslandId;
        DuskLog.info(
            "[ExtNpcMount] BG create '{}' id={:08x} host=({}, {}, {}) spawn_rel=({}, {}, {})",
            s_bgWarpProc, (u32)s_bgIslandId, s_bgHostPos.x, s_bgHostPos.y, s_bgHostPos.z,
            man.spawnRel.x, man.spawnRel.y, man.spawnRel.z);
        s_bgWarpPhase = kBgWarpWaitIsland;
        return;
    }

    if (s_bgWarpPhase == kBgWarpWaitIsland) {
        fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
        dExtNpcMount_c* mount =
            island != NULL ? (dExtNpcMount_c*)island : NULL;
        const bool ready =
            mount != NULL && mount->mIsBg && mount->mBgReady && mount->mpBgW != NULL;
        if (!ready) {
            ++s_bgReadyWaitFrames;
            // №58-B: hold black fade while resLoad is honestly in-flight.
            {
                JUTFader* fader = JFWDisplay::getManager()->getFader();
                if (fader != NULL && fader->getStatus() != JUTFader::FadeOut &&
                    fader->getStatus() != JUTFader::Wait) {
                    mDoGph_gInf_c::startFadeOut(5);
                }
            }
            const bool gone = island == NULL && s_bgReadyWaitFrames >= kBgReadyGoneTimeout;
            const bool hardStuck =
                s_bgFailSafeValid && s_bgReadyWaitFrames >= kBgReadyHardTimeout;
            if (gone || hardStuck) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason={} after {}f — return to fail-safe",
                    s_bgWarpProc, gone ? "create_gone" : "no_compleate", s_bgReadyWaitFrames);
                mDoGph_gInf_c::startFadeOut(10);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
                s_bgReadyWaitFrames = 0;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            return;
        }
        // Keep fail-safe armed until ground probe in WaitBg (№37).
        s_bgReadyWaitFrames = 0;
        s_bgSettleFrames = 2;
        s_bgWarpPhase = kBgWarpWaitBg;
        return;
    }

    if (s_bgWarpPhase == kBgWarpWaitBg) {
        fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
        if (island != NULL) {
            dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
            if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                updateBgTransform(mount);
                mount->mpBgW->Move();
            }
        }
        if (--s_bgSettleFrames > 0) {
            return;
        }

        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }

        cXyz spawn =
            s_bgSpawnOverrideValid ? s_bgSpawnOverride : (s_bgHostPos + man.spawnRel);
        if (island != NULL && !s_bgSpawnOverrideValid) {
            logBgWarpProbes((dExtNpcMount_c*)island, spawn);
        }

        // №54-4: local probe (refY+50), never sky+2000 — sky hits roofs on island returns.
        const f32 refY = spawn.y;
        if (dExtNpcMount_localGroundSnap(&spawn, refY)) {
            s_bgFailSafeValid = false;
        } else {
            DuskLog.warn("[Doors] enter {} → local ground miss/reject at ({:.1f},{:.1f},{:.1f})",
                         s_bgWarpProc, spawn.x, spawn.y, spawn.z);
            if (s_bgFailSafeValid && !s_bgSpawnOverrideValid) {
                // Interior enter failed — abort to outdoor fail-safe (already porch-snapped).
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=no_ground — return to fail-safe",
                    s_bgWarpProc);
                mDoGph_gInf_c::startFadeOut(10);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            DuskLog.warn("[ExtNpcMount] BG NO ground — placing at authored y (unguarded)");
            spawn.y = refY + 50.0f;
        }

        placeLinkAt(player, spawn);
        // №54-5: Link faces INTO the room (Nintendo PLYR spawn_ry) on interior enter.
        // №56: exit/return override may carry return_ry facing.
        if (s_bgSpawnOverrideValid && s_bgSpawnFacingValid) {
            player->current.angle.y = s_bgSpawnFacing;
            player->shape_angle.y = s_bgSpawnFacing;
        } else if (!s_bgSpawnOverrideValid && man.hasSpawnRy) {
            player->current.angle.y = man.spawnRy;
            player->shape_angle.y = man.spawnRy;
        }
        s_bgHoldPos = spawn;
        s_bgHoldFrames = s_bgSpawnOverrideValid ? 45 : 120;
        s_bgSpawnOverrideValid = false;
        s_bgSpawnFacingValid = false;
        DuskLog.info("[ExtNpcMount] Link placed on BG spawn ({}, {}, {})", spawn.x, spawn.y,
                     spawn.z);
        // №52-A: door lane fades out on A-press; menu lane already fades in — match it here.
        // JUTFader::startFadeIn only accepts status None (post-FadeOut black). If still mid
        // FadeOut, snap to None first so fade-in can start.
        {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL && fader->getStatus() == JUTFader::FadeOut) {
                fader->setStatus(JUTFader::None, 0);
            }
            if (!mDoGph_gInf_c::startFadeIn(15)) {
                DuskLog.warn("[Doors] fade-in refused after Link placed ({}) — retry next hold",
                             s_bgWarpProc);
            } else {
                DuskLog.info("[Doors] fade-in after Link placed ({})", s_bgWarpProc);
            }
        }
        // №25 F2: populate once BG is ready + Link seated (not at create-time of the BG).
        dExtNpcPopulation_spawnForBg(man);
        // №27 N6: door-knob visual props at TGDR positions (island only).
        if (std::strcmp(s_bgWarpProc, "EXT_BG0") == 0) {
            dExtNpcDoors_spawnKnobs(man);
        }
        s_bgFadeWatchdogFired = false;
        s_bgWarpPhase = kBgWarpHold;
        return;
    }

    if (s_bgWarpPhase == kBgWarpHold) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - s_bgHoldPos.x;
            const f32 dz = player->current.pos.z - s_bgHoldPos.z;
            if ((dx * dx + dz * dz) > (5000.0f * 5000.0f)) {
                DuskLog.warn("[ExtNpcMount] Link drifted off BG — re-anchoring");
                placeLinkAt(player, s_bgHoldPos);
            } else {
                player->speed.x = 0.0f;
                player->speed.z = 0.0f;
                if (player->speed.y < -50.0f) {
                    player->speed.y = -50.0f;
                }
            }
        }
        // Keep island MOVE_BG ticking while actors retry ground-snap.
        {
            fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
            if (island != NULL) {
                dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
                if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                    updateBgTransform(mount);
                    mount->mpBgW->Move();
                }
            }
        }
        // №53: one-shot stuck-fade watchdog (was spamming every Hold frame).
        if (!s_bgFadeWatchdogFired && s_bgHoldFrames <= 90) {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL &&
                (fader->getStatus() == JUTFader::None ||
                 fader->getStatus() == JUTFader::FadeOut)) {
                fader->setStatus(JUTFader::None, 0);
                if (mDoGph_gInf_c::startFadeIn(10)) {
                    s_bgFadeWatchdogFired = true;
                    DuskLog.warn("[Doors] stuck-fade watchdog — force fade-in ({})", s_bgWarpProc);
                }
            } else if (fader != NULL && fader->getStatus() == JUTFader::FadeIn) {
                s_bgFadeWatchdogFired = true;  // fade-in already running
            } else if (fader != NULL && fader->getStatus() == JUTFader::Wait) {
                s_bgFadeWatchdogFired = true;  // clear screen
            }
        }
        if (--s_bgHoldFrames <= 0) {
            std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_bgWarpProc);
            s_bgWarpPhase = kBgWarpIdle;
            s_bgWarpPending = false;
            dExtNpcMount_endDoorDemoLock();
            DuskLog.info("[ExtNpcMount] BG hold complete — active '{}'", s_lastBgProc);
            // №58-B: warm interiors once the island is live (first press = reuse, no mash).
            if (std::strcmp(s_lastBgProc, "EXT_BG0") == 0 && !s_warmInteriors) {
                s_warmInteriors = true;
                s_warmProviderIndex = 0;
                s_warmCooldown = 0;
                DuskLog.info("[ExtNpcMount] №58-B warm interiors start");
            }
        }
    }
}

void dExtNpcMount_beginDoorDemoLock() {
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    daAlink_c* link = (daAlink_c*)player;
    link->changeOriginalDemo();
    // №53-A: native door-open proc through the fade (param0=0 → left swing).
    link->changeDemoMode(daPy_demo_c::DEMO_DOOR_OPEN_e, 0, 0, 0);
    player->speedF = 0.0f;
    player->speed.x = 0.0f;
    player->speed.y = 0.0f;
    player->speed.z = 0.0f;
    s_doorDemoLocked = true;
}

void dExtNpcMount_endDoorDemoLock() {
    if (!s_doorDemoLocked) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player != NULL) {
        ((daPy_py_c*)player)->cancelOriginalDemo();
    }
    s_doorDemoLocked = false;
}

void dExtNpcMount_forceEndDoorEvent(const char* reason) {
    dExtNpcMount_endDoorDemoLock();
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player != NULL) {
        ((daPy_py_c*)player)->cancelOriginalDemo();
    }
    dEvt_control_c* ev = dComIfGp_getEvent();
    if (ev == NULL) {
        return;
    }
    const bool active = ev->runCheck() || ev->getMode() != dEvt_mode_WAIT_e;
    if (!active) {
        return;
    }
    dComIfGp_getEventManager().cancelStaff("ALL");
    dComIfGp_getEventManager().setCameraPlay(0);
    ev->remove();
    DuskLog.info("[ExtNpcMount] №89 force-end event ({}) — control released",
                 reason != NULL ? reason : "?");
}

int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName) {
    if (i_this == NULL || procName == NULL || procName[0] == '\0') {
        return cPhs_ERROR_e;
    }

    // №52: create is re-entrant across resLoad. One-time init + ledger once; later phases
    // only continue the phase machine (wiping pointers/keys mid-load left prop='?' ).
    const bool firstPhase = !i_this->mManifest.valid;
    if (firstPhase) {
        if (!dExtNpcMount_lookup(procName, &i_this->mManifest)) {
            return cPhs_ERROR_e;
        }

        // №24 D2: BG payloads require explicit anchor= (never invent 0).
        if (i_this->mManifest.isBg && !i_this->mManifest.hasAnchor) {
            DuskLog.warn("[ExtNpcMount] create '{}' refused — BG missing anchor=", procName);
            return cPhs_ERROR_e;
        }

        // R-O2e: folder-side spawn gates (missing flag = false).
        if (i_this->mManifest.spawnIfFlag[0] &&
            !dExtModFlags_get(i_this->mManifest.modFolder, i_this->mManifest.spawnIfFlag)) {
            DuskLog.debug("[ExtNpcMount] create '{}' refused — spawn_if_flag '{}'", procName,
                          i_this->mManifest.spawnIfFlag);
            return cPhs_ERROR_e;
        }
        if (i_this->mManifest.spawnUnlessFlag[0] &&
            dExtModFlags_get(i_this->mManifest.modFolder, i_this->mManifest.spawnUnlessFlag)) {
            DuskLog.debug("[ExtNpcMount] create '{}' refused — spawn_unless_flag '{}'", procName,
                          i_this->mManifest.spawnUnlessFlag);
            return cPhs_ERROR_e;
        }

        i_this->mpBtp = NULL;
        i_this->mpCompanion = NULL;
        i_this->mpBrk = NULL;
        i_this->mpColorBtk = NULL;
        i_this->mpBgBtk = NULL;
        i_this->mpBgW = NULL;
        i_this->mIsBg = i_this->mManifest.isBg;
        i_this->mBgReady = false;
        // №36 C: WW item id in params low byte (Vlupy color + TP grant mapping).
        i_this->mPickupItemNo =
            i_this->mManifest.pickupRupee ? (u8)(fopAcM_GetParam(i_this) & 0xFF) : 0;
        for (int i = 0; i < 3; ++i) {
            i_this->mpBgModels[i] = NULL;
        }
        i_this->mBtpBound = false;
        i_this->mAttachCount = 0;
        i_this->mNeckJnt = -1;
        i_this->mLookYaw = 0;
        i_this->mLookPitch = 0;
        i_this->mTalking = false;
        i_this->mCcReady = false;
        i_this->mTalkEventActive = false;
        i_this->mTalkFrames = 0;
        i_this->mCloseCooldown = 0;
        i_this->mOrbitPhase = 0;
        i_this->mHeadVariant = 0;
        i_this->mSlaveMap[0] = '\0';
        i_this->mSlavePairCount = 0;
        // №52-B: do NOT clear mDoorKey/mSpawnSrc — stub Create stamps them from pending
        // before the first create phase.
        std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                      i_this->mManifest.dialogueKey);
        i_this->mDialogueNext[0] = '\0';
        for (int i = 0; i < kExtNpcMaxAttach; ++i) {
            i_this->mpAttach[i] = NULL;
            i_this->mAttachJnt[i] = -1;
            i_this->mAttachOnCompanion[i] = 0;
            i_this->mAttachSlave[i] = 0;
        }

        const int arg = fopAcM_GetParam(i_this) & 0xFF;
        for (int i = 0; i < i_this->mManifest.subtypeCount; ++i) {
            const dExtNpcSubtype& subtype = i_this->mManifest.subtypes[i];
            if (!subtype.valid || subtype.arg != arg) {
                continue;
            }
            if (subtype.idle[0]) {
                snprintf(i_this->mManifest.idle, sizeof(i_this->mManifest.idle), "%s", subtype.idle);
            }
            if (subtype.displayName[0]) {
                snprintf(i_this->mManifest.displayName, sizeof(i_this->mManifest.displayName), "%s",
                         subtype.displayName);
            }
            if (subtype.attachModel[0] && i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                const int slot = i_this->mManifest.attachCount++;
                snprintf(i_this->mManifest.attach[slot].model,
                         sizeof(i_this->mManifest.attach[slot].model), "%s", subtype.attachModel);
                snprintf(i_this->mManifest.attach[slot].joint,
                         sizeof(i_this->mManifest.attach[slot].joint), "%s",
                         subtype.attachJoint[0] ? subtype.attachJoint : "head");
            }
            DuskLog.info("[ExtNpcMount] {} subtype arg={} selected", procName, arg);
            break;
        }

        // №27 N2 / №44: optional pending/forced head (P1a vs P1b same arg, or FIFO pin).
        {
            char headModel[64];
            char headJoint[32];
            if (dExtNpcMount_consumeForcedAttach(headModel, sizeof(headModel), headJoint,
                                                 sizeof(headJoint))) {
                if (i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                    const int slot = i_this->mManifest.attachCount++;
                    snprintf(i_this->mManifest.attach[slot].model,
                             sizeof(i_this->mManifest.attach[slot].model), "%s", headModel);
                    snprintf(i_this->mManifest.attach[slot].joint,
                             sizeof(i_this->mManifest.attach[slot].joint), "%s", headJoint);
                    const char* dot = std::strstr(headModel, ".bdl");
                    if (dot != NULL && dot >= headModel + 2 &&
                        dot[-2] >= '0' && dot[-2] <= '9' && dot[-1] >= '0' && dot[-1] <= '9') {
                        i_this->mHeadVariant = (u8)((dot[-2] - '0') * 10 + (dot[-1] - '0'));
                    } else {
                        i_this->mHeadVariant = 1;
                    }
                    DuskLog.info("[ExtNpcMount] {} pending/forced attach '{}' @ '{}'", procName,
                                 headModel, headJoint);
                }
            }
        }
        // №45/№48/№49 PULL: heads + companion + slave_map from actor_map (before heap).
        {
            const int regArg =
                i_this->mManifest.socketArg >= 0 ? i_this->mManifest.socketArg : arg;
            pullSlaveMapFromRegistry(procName, regArg, i_this->mSlaveMap, sizeof(i_this->mSlaveMap));
            if (i_this->mSlaveMap[0]) {
                DuskLog.info("[ExtNpcMount] {} slave_map={}", procName, i_this->mSlaveMap);
            }
            // №49: row may carry BOTH head_model and companion (Quill bmhead + bmarm).
            if (i_this->mManifest.companionModel[0] == '\0') {
                char companion[64] = {};
                if (pullCompanionFromRegistry(procName, regArg, companion, sizeof(companion))) {
                    snprintf(i_this->mManifest.companionModel,
                             sizeof(i_this->mManifest.companionModel), "%s", companion);
                    i_this->mManifest.companionMode = 1;
                    DuskLog.info("[ExtNpcMount] {} registry pull companion '{}'", procName,
                                 companion);
                }
            } else if (i_this->mSlaveMap[0]) {
                i_this->mManifest.companionMode = 1;
            }
            if (i_this->mManifest.bodyBmt[0] == '\0') {
                char bodyBmt[64] = {};
                if (pullBodyBmtFromRegistry(procName, regArg, bodyBmt, sizeof(bodyBmt))) {
                    snprintf(i_this->mManifest.bodyBmt, sizeof(i_this->mManifest.bodyBmt), "%s",
                             bodyBmt);
                    DuskLog.info("[ExtNpcMount] {} registry pull body_bmt '{}'", procName, bodyBmt);
                }
            }
            if (i_this->mManifest.attachCount == 0 && !i_this->mManifest.isBg) {
                char headModel[64] = {};
                char headJoint[32] = {};
                if (pullHeadFromRegistry(procName, regArg, fopAcM_GetParam(i_this), headModel,
                                         sizeof(headModel), headJoint, sizeof(headJoint))) {
                    if (i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                        const int slot = i_this->mManifest.attachCount++;
                        snprintf(i_this->mManifest.attach[slot].model,
                                 sizeof(i_this->mManifest.attach[slot].model), "%s", headModel);
                        snprintf(i_this->mManifest.attach[slot].joint,
                                 sizeof(i_this->mManifest.attach[slot].joint), "%s",
                                 headJoint[0] ? headJoint : "head");
                        DuskLog.info("[ExtNpcMount] {} registry pull head '{}' @ '{}'", procName,
                                     headModel, headJoint[0] ? headJoint : "head");
                    }
                }
            }
        }

        // №38 E2 SPAWN LEDGER — after head pin so head= is authoritative.
        // №51: persist src on the actor (forceNextSpawnSrc is one-shot; door props need it).
        {
            if (s_forcedSpawnSrcSet && s_forcedSpawnSrc[0]) {
                if (i_this->mSpawnSrc[0] == '\0') {
                    std::snprintf(i_this->mSpawnSrc, sizeof(i_this->mSpawnSrc), "%s",
                                  s_forcedSpawnSrc);
                }
                if (i_this->mDoorKey[0] == '\0' && std::strncmp(s_forcedSpawnSrc, "door:", 5) == 0 &&
                    s_forcedSpawnSrc[5]) {
                    std::snprintf(i_this->mDoorKey, sizeof(i_this->mDoorKey), "%s",
                                  s_forcedSpawnSrc + 5);
                }
            }
            const char* src =
                i_this->mSpawnSrc[0] ? i_this->mSpawnSrc :
                (s_forcedSpawnSrcSet && s_forcedSpawnSrc[0] ? s_forcedSpawnSrc : "unknown");
            s_forcedSpawnSrcSet = false;
            s_forcedSpawnSrc[0] = '\0';
            char headBuf[96];
            const char* head = "(none)";
            if (i_this->mManifest.attachCount > 0 && i_this->mManifest.attach[0].model[0]) {
                std::snprintf(headBuf, sizeof(headBuf), "%s@%s", i_this->mManifest.attach[0].model,
                              i_this->mManifest.attach[0].joint[0] ?
                                  i_this->mManifest.attach[0].joint :
                                  "head");
                head = headBuf;
            } else if (i_this->mManifest.companionModel[0] && !i_this->mManifest.isBg &&
                       !i_this->mManifest.isStatic) {
                // Aryll hands etc. — companion is the attach proof, not a head miss.
                std::snprintf(headBuf, sizeof(headBuf), "companion:%s",
                              i_this->mManifest.companionModel);
                head = headBuf;
            } else if (i_this->mManifest.attachCount == 0 && !i_this->mManifest.isBg &&
                       !i_this->mManifest.isStatic) {
                head = "MISS:no_head_attach";
            }
            DuskLog.info("[Spawn] src={} proc={} arg={} mod={} head={}", src, procName,
                         i_this->mManifest.socketArg, i_this->mManifest.modFolder, head);
        }

        stageLog("create", "resLoad begin");
    } else if (i_this->mManifest.proc[0] == '\0') {
        // Re-entry without a stamped manifest — recover lookup once.
        if (!dExtNpcMount_lookup(procName, &i_this->mManifest)) {
            return cPhs_ERROR_e;
        }
    }

    // Already heap-bound (should not re-enter Create after COMPLEATE).
    // №117: collision-only statics set mBgReady without mpMorf / without mIsBg.
    if (i_this->mpMorf != NULL || i_this->mBgReady) {
        return cPhs_COMPLEATE_e;
    }

    int phase = dComIfG_resLoad(&i_this->mPhase, i_this->mManifest.arc);
    if (phase != cPhs_COMPLEATE_e) {
        if (phase == cPhs_ERROR_e) {
            DuskLog.info("[ExtNpcMount:D1] create — FAIL resLoad ERROR ({})", procName);
            dExtNpcMount_forceNextAttach(NULL, NULL);
        }
        return phase;
    }
    stageLog("create", "resLoad COMPLEATE → solid heap");

    beginCreateCacheTrack();
    if (i_this->mIsBg) {
        if (!fopAcM_entrySolidHeap(i_this, useBgHeapInit, 0x120000)) {
            DuskLog.warn("[ExtNpcMount] BG heap fail for {}", procName);
            // №115: warm-storm hang signature — first heap fail must stop further warms.
            if (std::strcmp(i_this->mSpawnSrc, "warm") == 0) {
                abortWarmInteriors("BG heap fail");
            }
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        // Regist BEFORE first Move (History Phase O / №20). Matrix was identity at Set;
        // apply host−anchor now so Move() deltas the vtx table into diorama space.
        if (dComIfG_Bgsp().Regist(i_this->mpBgW, i_this)) {
            DuskLog.warn("[ExtNpcMount] BG Regist failed for {}", procName);
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        updateBgTransform(i_this);
        if (!i_this->mBgGlobal) {
            i_this->mpBgW->Move();
        }
        i_this->mBgReady = true;
        if (i_this->mpBgModels[0] != NULL) {
            fopAcM_SetMtx(i_this, i_this->mpBgModels[0]->getBaseTRMtx());
        } else {
            // №117: model-less BG — bind actor mtx to collision transform.
            fopAcM_SetMtx(i_this, i_this->mBgMtx);
        }
        // Outset's authored span is ~100k units. Never frustum-cull the actor away.
        fopAcM_SetMin(i_this, -120000.0f, -20000.0f, -120000.0f);
        fopAcM_SetMax(i_this, 120000.0f, 20000.0f, 120000.0f);
        fopAcM_OffStatus(i_this, fopAcStts_CULL_e);
        dKy_tevstr_init(&i_this->tevStr, fopAcM_GetRoomNo(i_this), 0xFF);
        i_this->tevStr.room_no = fopAcM_GetRoomNo(i_this);
        // №27 N6 / №108: hide TP stub vrbox; outdoor F_DL* mounts WwSky instead.
        if (strcmp(procName, "EXT_BG0") == 0 || strcmp(procName, "EXT_BG9") == 0) {
            g_env_light.hide_vrbox = true;
            DuskLog.info("[ExtNpcMount] N6 hide_vrbox for '{}'", procName);
        }
        if (mountWantsWwSky(i_this)) {
            wwSkyRetain();
        }
        commitCreateCacheTrack();
        retainArcModels(i_this->mManifest.arc);
        DuskLog.info(
            "[ExtNpcMount] BG COMPLEATE {} — dzbT=({}, {}, {}) model_space={} host=({}, {}, {}) "
            "anchor=({}, {}, {})",
            procName, i_this->current.pos.x - i_this->mManifest.anchor.x,
            i_this->current.pos.y - i_this->mManifest.anchor.y,
            i_this->current.pos.z - i_this->mManifest.anchor.z,
            i_this->mManifest.modelSpaceLocal ? "local" : "world", i_this->current.pos.x,
            i_this->current.pos.y, i_this->current.pos.z, i_this->mManifest.anchor.x,
            i_this->mManifest.anchor.y, i_this->mManifest.anchor.z);
        // №90/№104: population (+ knobs) off EVERY BG COMPLEATE on its host — including
        // EXT_BG0 (exterior re-entry was skipping census; gen bump alone is not enough).
        // Deferred until player exists via poll.
        {
            const char* stage = dComIfGp_getStartStageName();
            const bool onHost = i_this->mManifest.hostStage[0] == '\0' ||
                                (stage != NULL &&
                                 std::strcmp(stage, i_this->mManifest.hostStage) == 0);
            const bool isExterior = std::strcmp(procName, "EXT_BG0") == 0;
            if (onHost && (isExterior || dExtWwSave_isWwHostStage(stage) ||
                           dExtNpcMount_isRoomLaneProc(procName))) {
                // №104: activate room immediately (daBg never runs on these shells).
                if (!isExterior && i_this->mManifest.hostRoom >= 0 &&
                    i_this->mManifest.hostRoom < 0x40) {
                    activateWwHostRoom(i_this->mManifest.hostRoom, "bg-compleate");
                }
                std::snprintf(s_interiorBootstrapProc, sizeof(s_interiorBootstrapProc), "%s",
                              procName);
                tryInteriorBootstrap();
            }
        }
        return cPhs_COMPLEATE_e;
    }

    if (!fopAcM_entrySolidHeap(i_this, useHeapInit, 0x80000)) {
        DuskLog.warn("[ExtNpcMount] heap fail for {}", procName);
        stageLog("create", "FAIL entrySolidHeap");
        abortCreateCacheTrack();
        return cPhs_ERROR_e;
    }

    const f32 s = i_this->mManifest.scale;
    i_this->scale.set(s, s, s);

    dKy_tevstr_init(&i_this->tevStr, fopAcM_GetRoomNo(i_this), 0xFF);
    i_this->tevStr.room_no = fopAcM_GetRoomNo(i_this);

    // №117: collision-only static — no model / morf; regist dzb and finish.
    if (i_this->mpMorf == NULL && i_this->mpBgW != NULL && i_this->mBgReady) {
        if (dComIfG_Bgsp().Regist(i_this->mpBgW, i_this)) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only Regist failed for {}", procName);
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        i_this->mpBgW->Move();
        fopAcM_SetMtx(i_this, i_this->mBgMtx);
        fopAcM_SetMin(i_this, -200.0f * s, -20.0f * s, -200.0f * s);
        fopAcM_SetMax(i_this, 200.0f * s, 400.0f * s, 200.0f * s);
        commitCreateCacheTrack();
        retainArcModels(i_this->mManifest.arc);
        DuskLog.info("[ExtNpcMount] №117 COMPLEATE collision-only {} arc={} dzb={}", procName,
                     i_this->mManifest.arc, i_this->mManifest.collision);
        return cPhs_COMPLEATE_e;
    }

    fopAcM_SetMtx(i_this, i_this->mpMorf->getModel()->getBaseTRMtx());
    fopAcM_SetMin(i_this, -80.0f * s, -20.0f * s, -80.0f * s);
    fopAcM_SetMax(i_this, 80.0f * s, 200.0f * s, 80.0f * s);
    // №32 B7: carryable = PICKUP only (never SPEAK). B1: door=1 → DOOR attention.
    if (i_this->mManifest.carryable) {
        i_this->attention_info.flags = fopAc_AttnFlag_CARRY_e;
        fopAcM_OnCarryType(i_this, fopAcM_CARRY_LIGHT);
    } else if (i_this->mManifest.doorAttention) {
        i_this->attention_info.flags = fopAc_AttnFlag_DOOR_e;
        // №91: resolve DoorK10 events when ready (async load may complete later in execute).
        i_this->mKnobEvtFront = -1;
        i_this->mKnobEvtBack = -1;
        i_this->mKnobEvtOrdered = -1;
        i_this->mKnobStaffId = -1;
        i_this->mKnobDoorAction = 0;
        i_this->mKnobOpenStarted = 0;
        i_this->mKnobEvBound = 0;
        bindKnobDoorEvents(i_this);
    } else if (i_this->mManifest.isStatic && i_this->mManifest.dialogueKey[0] == '\0') {
        i_this->attention_info.flags = 0;
    } else {
        i_this->attention_info.flags = fopAc_AttnFlag_TALK_e | fopAc_AttnFlag_SPEAK_e;
    }
    // Attention distances are table indices (daNpcT_getDistTableIdx formula), not world units.
    auto distIdx = [](int dist, int angle) -> u8 {
        return (u8)(dist + angle * 0x14 + 0x5e);
    };
    i_this->attention_info.distances[fopAc_attn_LOCK_e] = distIdx(5, 6);
    i_this->attention_info.distances[fopAc_attn_TALK_e] = distIdx(5, 6);
    i_this->attention_info.distances[fopAc_attn_SPEAK_e] = distIdx(3, 6);
    if (i_this->mManifest.carryable) {
        i_this->attention_info.distances[fopAc_attn_CARRY_e] = distIdx(3, 6);
    }
    if (i_this->mManifest.doorAttention) {
        i_this->attention_info.distances[fopAc_attn_DOOR_e] = distIdx(3, 6);
    }
    i_this->attention_info.position = i_this->current.pos;
    i_this->attention_info.position.y += i_this->mManifest.cylHeight * s * 0.85f;
    i_this->mCcStts.Init(0xFF, 0, i_this);
    i_this->mCyl.Set(s_mountCylSrc);
    i_this->mCyl.SetStts(&i_this->mCcStts);
    i_this->mCyl.SetR(i_this->mManifest.cylRadius * s);
    i_this->mCyl.SetH(i_this->mManifest.cylHeight * s);
    i_this->mCcReady = true;
    JUTNameTab* jointNames = i_this->mpMorf->getModel()->getModelData()->getJointTree().getJointName();
    if (jointNames != NULL && i_this->mManifest.neckJoint[0]) {
        const s32 index = jointNames->getIndex(i_this->mManifest.neckJoint);
        if (index >= 0 && index < i_this->mpMorf->getModel()->getModelData()->getJointNum()) {
            i_this->mNeckJnt = (s16)index;
        }
    }
    // №25 F2: authored home Y is the placement floor; never seat below it into seafloor.
    i_this->home.pos = i_this->current.pos;
    i_this->mGroundSnapped = false;
    i_this->mGroundSnapTries = 0;
    tryGroundSnapSanity(i_this);

    // N6: seagulls start orbiting after ground settle (phase ticks in execute).
    if (std::strcmp(i_this->mManifest.arc, "Kamome") == 0 ||
        std::strcmp(procName, "NPC_KAMOME") == 0) {
        i_this->mOrbitPhase = 1;
    }

    // №47-A: force idle start on every lane (heap may have bound NULL if N3 once refused).
    if (!i_this->mManifest.isStatic && i_this->mManifest.idle[0]) {
        setMountAnimation(i_this, i_this->mManifest.idle, J3DFrameCtrl::EMode_LOOP);
    }

    commitCreateCacheTrack();
    retainArcModels(i_this->mManifest.arc);
    DuskLog.info("[ExtNpcMount] COMPLEATE {} arc={} model={} btp={} scale={} mod={}", procName,
                 i_this->mManifest.arc, i_this->mManifest.model,
                 i_this->mBtpBound ? i_this->mManifest.btp : "(none)", s,
                 i_this->mManifest.modFolder);
    stageLog("create", "COMPLEATE");
    return cPhs_COMPLEATE_e;
}

int dExtNpcMount_delete(dExtNpcMount_c* i_this) {
    if (i_this != NULL && i_this->mIsBg) {
        if (mountWantsWwSky(i_this) ||
            (i_this->mManifest.proc[0] &&
             (std::strcmp(i_this->mManifest.proc, "EXT_BG0") == 0 ||
              std::strcmp(i_this->mManifest.proc, "EXT_BG9") == 0))) {
            // Stage may already have changed; always drop a user if we retained on COMPLEATE.
            wwSkyReleaseUser();
        }
        // №100: drop collision + draw refs before releasing the arc cache.
        if (i_this->mpBgW != NULL) {
            dComIfG_Bgsp().Release(i_this->mpBgW);
            i_this->mpBgW = NULL;
        }
        for (int i = 0; i < 3; ++i) {
            i_this->mpBgModels[i] = NULL;
        }
        i_this->mBgReady = false;
        if (i_this->mManifest.arc[0]) {
            // №73/№100: purge cached J3D only when no other live mount refs this arc.
            releaseArcModels(i_this->mManifest.arc, "delete-bg", i_this);
            dComIfG_resDelete(&i_this->mPhase, i_this->mManifest.arc);
        }
        return 1;
    }
    // G4: the draw-list dialogue must be hidden before the owner actor goes away.
    if (i_this != NULL && (i_this->mTalking || s_mountDialogueOwner == i_this)) {
        closeMountDialogue(i_this);
    }
    if (i_this != NULL && i_this->mManifest.arc[0]) {
        releaseArcModels(i_this->mManifest.arc, "delete-npc", i_this);
        dComIfG_resDelete(&i_this->mPhase, i_this->mManifest.arc);
    }
    return 1;
}

int dExtNpcMount_execute(dExtNpcMount_c* i_this) {
    if (i_this == NULL) {
        return 1;
    }
    // №68: room teardown in flight — do not touch matrices / attention / bgw.
    if (roomLaneMountIsUnloading(i_this)) {
        return 1;
    }
    if (i_this->mIsBg) {
        if (mountWantsWwSky(i_this) || s_wwSkyUsers > 0) {
            wwSkyEnsure();
        }
        updateBgTransform(i_this);
        if (i_this->mpBgBtk != NULL) {
            i_this->mpBgBtk->play();
        }
        // №98: GLOBAL_e world collision has no move mtx — do not Move().
        if (i_this->mpBgW != NULL && !i_this->mBgGlobal) {
            i_this->mpBgW->Move();
        }
        return 1;
    }
    // №117: collision-only static prop — keep dzb aligned with actor pose.
    if (i_this->mpMorf == NULL) {
        if (i_this->mpBgW != NULL && i_this->mBgReady) {
            mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y,
                                   i_this->current.pos.z);
            mDoMtx_stack_c::YrotM(i_this->current.angle.y);
            MTXCopy(mDoMtx_stack_c::get(), i_this->mBgMtx);
            i_this->mpBgW->Move();
        }
        return 1;
    }
    tryGroundSnapSanity(i_this);

    // №27 N6: simple seagull orbit around authored home (no AI arc — folder-side motion).
    if (i_this->mOrbitPhase > 0 && !i_this->mTalking) {
        ++i_this->mOrbitPhase;
        const s16 ang = (s16)(i_this->mOrbitPhase * 180);
        const s16 bob = (s16)(i_this->mOrbitPhase * 310);
        const f32 radius = 220.0f;
        i_this->current.pos.x = i_this->home.pos.x + radius * cM_scos(ang);
        i_this->current.pos.z = i_this->home.pos.z + radius * cM_ssin(ang);
        i_this->current.pos.y = i_this->home.pos.y + 40.0f * cM_ssin(bob);
        i_this->shape_angle.y = (s16)(ang + 0x4000);
        i_this->current.angle.y = i_this->shape_angle.y;
    }

    const f32 s = i_this->scale.x;
    i_this->attention_info.position = i_this->current.pos;
    i_this->attention_info.position.y += i_this->mManifest.cylHeight * s * 0.85f;
    if (i_this->mCcReady) {
        i_this->mCyl.SetC(i_this->current.pos);
        i_this->mCyl.SetR(i_this->mManifest.cylRadius * s);
        i_this->mCyl.SetH(i_this->mManifest.cylHeight * s);
        i_this->mCyl.ClrCoHit();
        dComIfG_Ccsp()->Set(&i_this->mCyl);
    }

    if (i_this->mCloseCooldown > 0) {
        --i_this->mCloseCooldown;
    }
    if (i_this->mTalking) {
        ++i_this->mTalkFrames;
        // №33: keep native talk-event lock alive while the window is up (postman pattern).
        if (i_this->mTalkEventActive) {
            dMeter2Info_onGameStatus(2);
        }
#if TARGET_PC_NATIVE_UI
        if (s_mountDialogue == NULL || !s_mountDialogue->isVisible() ||
            s_mountDialogue->checkDismiss() || i_this->mTalkFrames >= 1800) {
            char pendingNext[64];
            std::snprintf(pendingNext, sizeof(pendingNext), "%s", i_this->mDialogueNext);
            const bool wasEvent = i_this->mTalkEventActive;
            closeMountDialogue(i_this);
            if (pendingNext[0] != '\0') {
                std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                              pendingNext);
                i_this->mDialogueNext[0] = '\0';
                i_this->mCloseCooldown = 0;
                beginMountDialogue(i_this, wasEvent);
            } else {
                // Next talk starts at the manifest entry key (else=/gates re-evaluate).
                std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                              i_this->mManifest.dialogueKey);
            }
        }
#else
        closeMountDialogue(i_this);
#endif
    } else if (i_this->mCloseCooldown == 0) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        // №91: doorAttention mounts order DEFAULT_KNOB_DOOR_* and own cutEnd, then warp.
        // Until DoorK10 binds, №53 CANDOOR → immediate tryNativeWarp remains as fallback.
        if (i_this->mManifest.doorAttention) {
            if (player != NULL &&
                (player->current.pos - i_this->current.pos).absXZ() < 300.0f) {
                i_this->attention_info.flags = fopAc_AttnFlag_DOOR_e;
                i_this->attention_info.position = i_this->current.pos;
                i_this->attention_info.position.y += 80.0f;
            }
            tickKnobDoorEvent(i_this);
        } else {
            // №33: wrap mount dialogue in a real TP speak event (postman pattern).
            // Prefer checkCommandTalk; TrigA only orders the speak event (no orphan window).
            i_this->eventInfo.onCondition(dEvtCnd_CANTALK_e);
            if (i_this->eventInfo.checkCommandTalk()) {
                beginMountDialogue(i_this, /*fromEvent=*/true);
            } else if (player != NULL &&
                       (player->current.pos - i_this->current.pos).absXZ() < 160.0f &&
                       mDoCPd_c::getTrigA(PAD_1) != 0 &&
                       (i_this->attention_info.flags & fopAc_AttnFlag_SPEAK_e) != 0) {
                fopAcM_orderSpeakEvent(i_this, 0, 0);
            }
        }
    }

    mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y, i_this->current.pos.z);
    mDoMtx_stack_c::YrotM(i_this->shape_angle.y);
    mDoMtx_stack_c::scaleM(s, s, s);
    MtxP base = mDoMtx_stack_c::get();
    i_this->mpMorf->getModel()->setBaseTRMtx(base);
    // №36 C: pickup rupee — near player ⇒ grant TP wallet item + delete WW visual.
    if (i_this->mManifest.pickupRupee) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - i_this->current.pos.x;
            const f32 dy = player->current.pos.y - i_this->current.pos.y;
            const f32 dz = player->current.pos.z - i_this->current.pos.z;
            if (dx * dx + dz * dz < 100.0f * 100.0f && dy > -80.0f && dy < 160.0f) {
                const u8 grant = tpRupeeGrantId(i_this->mPickupItemNo);
                if (grant != 0) {
                    execItemGet(grant);
                    DuskLog.info("[ExtNpcMount] pickup grant ww={} → TP item {} (wallet)",
                                 (int)i_this->mPickupItemNo, (int)grant);
                }
                fopAcM_delete(i_this);
                return 1;
            }
        }
    }

    i_this->mpMorf->play(NULL, 0, 0);
    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->play();
    }
    i_this->mpMorf->modelCalc();
    if (i_this->mNeckJnt >= 0) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            cXyz from;
            mDoMtx_stack_c::copy(i_this->mpMorf->getModel()->getAnmMtx(i_this->mNeckJnt));
            mDoMtx_stack_c::multVecZero(&from);
            cXyz to = player->current.pos - from;
            s16 yaw = (s16)(to.atan2sX_Z() - i_this->shape_angle.y);
            s16 pitch = (s16)-to.atan2sY_XZ();
            if (yaw > 0x2000) yaw = 0x2000;
            if (yaw < -0x2000) yaw = -0x2000;
            if (pitch > 0x1000) pitch = 0x1000;
            if (pitch < -0x1000) pitch = -0x1000;
            cLib_addCalcAngleS(&i_this->mLookYaw, yaw, 8, 0x400, 0x20);
            cLib_addCalcAngleS(&i_this->mLookPitch, pitch, 8, 0x200, 0x20);
        } else {
            cLib_addCalcAngleS(&i_this->mLookYaw, 0, 8, 0x400, 0x20);
            cLib_addCalcAngleS(&i_this->mLookPitch, 0, 8, 0x200, 0x20);
        }
        mDoMtx_stack_c::copy(i_this->mpMorf->getModel()->getAnmMtx(i_this->mNeckJnt));
        mDoMtx_stack_c::YrotM(i_this->mLookYaw);
        mDoMtx_stack_c::XrotM(i_this->mLookPitch);
        i_this->mpMorf->getModel()->setAnmMtx(i_this->mNeckJnt, mDoMtx_stack_c::get());
    }
    if (i_this->mpCompanion != NULL && i_this->mpCompanion->getModel() != NULL) {
        i_this->mpCompanion->getModel()->setBaseTRMtx(base);
        if (i_this->mManifest.companionMode == 1) {
            // №49: joint callbacks inside modelCalc do REPLACE before envelope.
            i_this->mpCompanion->modelCalc();
        } else {
            i_this->mpCompanion->play(NULL, 0, 0);
            i_this->mpCompanion->modelCalc();
        }
    }
    J3DModel* body = i_this->mpMorf->getModel();
    J3DModel* companion =
        i_this->mpCompanion != NULL ? i_this->mpCompanion->getModel() : NULL;
    // №49/№50: slave attach → parent-compose CBs; door visual → base@DoorDummy only.
    for (int i = 0; i < i_this->mAttachCount; ++i) {
        if (i_this->mpAttach[i] == NULL) {
            continue;
        }
        J3DModel* host =
            (i_this->mAttachOnCompanion[i] && companion != NULL) ? companion : body;
        if (host == NULL) {
            continue;
        }
        if (i_this->mAttachSlave[i]) {
            i_this->mpAttach[i]->setBaseTRMtx(host->getBaseTRMtx());
        } else if (i_this->mAttachJnt[i] >= 0) {
            i_this->mpAttach[i]->setBaseTRMtx(host->getAnmMtx((u16)i_this->mAttachJnt[i]));
        } else {
            i_this->mpAttach[i]->setBaseTRMtx(host->getBaseTRMtx());
        }
        i_this->mpAttach[i]->calc();
    }
    return 1;
}

static u8 clampAmbChannel(int base, f32 wregOffset) {
    int v = base + (int)wregOffset;
    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return (u8)v;
}

int dExtNpcMount_draw(dExtNpcMount_c* i_this) {
    if (i_this == NULL) {
        return 1;
    }
    // №68: skip draw while room handles are being torn down.
    if (roomLaneMountIsUnloading(i_this)) {
        return 1;
    }
    if (i_this->mIsBg) {
        // №108: sky before island geometry (camera-follow; sky draw list).
        if (std::strcmp(i_this->mManifest.proc, "EXT_BG0") == 0 ||
            std::strcmp(i_this->mManifest.proc, "EXT_BG9") == 0) {
            wwSkyDraw();
        }
        // Obj_Fmobj-style draw on the normal actor list. setListBG from an NPC draw
        // priority is too late — the BG list was already flushed (invisible island).
        // No per-draw logging — Outset BG is already the FPS hot path.
        mDoLib_clipper::changeFar(1000000.0f);
        g_env_light.settingTevStruct(0x40, &i_this->current.pos, &i_this->tevStr);
        for (int i = 0; i < 3; ++i) {
            if (i_this->mpBgModels[i] == NULL) {
                continue;
            }
            J3DModel* model = i_this->mpBgModels[i];
            model->calc();
            J3DModelData* data = model->getModelData();
            if (data != NULL) {
                for (u16 s = 0; s < data->getShapeNum(); ++s) {
                    J3DShape* shape = data->getShapeNodePointer(s);
                    if (shape != NULL) {
                        shape->show();
                    }
                }
            }
            g_env_light.setLightTevColorType_MAJI(model, &i_this->tevStr);
            if (i == 1 && i_this->mpBgBtk != NULL && data != NULL) {
                i_this->mpBgBtk->entry(data);
            }
            mDoExt_modelUpdateDL(model);
            if (i == 1 && i_this->mpBgBtk != NULL && data != NULL) {
                i_this->mpBgBtk->remove(data);
            }
        }
        mDoLib_clipper::resetFar();
        return 1;
    }
    if (i_this->mpMorf == NULL) {
        return 1;
    }

    J3DModel* model = i_this->mpMorf->getModel();

    // Boots/leaf recipe (no MAJI): settingTevStruct(0) → NPC ambient → entryDL.
    // Ambient is mount-owned (manifest amb= + WREG_F(30..32)), NOT bow warm tint —
    // that 105/78/48 cast was the №14 orange bloom on skin/cloth.
    g_env_light.settingTevStruct(0, &i_this->current.pos, &i_this->tevStr);

    GXColor amb_col;
    amb_col.r = clampAmbChannel(i_this->mManifest.ambR, WREG_F(30));
    amb_col.g = clampAmbChannel(i_this->mManifest.ambG, WREG_F(31));
    amb_col.b = clampAmbChannel(i_this->mManifest.ambB, WREG_F(32));
    amb_col.a = 255;
    i_this->tevStr.AmbCol.r = amb_col.r;
    i_this->tevStr.AmbCol.g = amb_col.g;
    i_this->tevStr.AmbCol.b = amb_col.b;
    i_this->tevStr.AmbCol.a = amb_col.a;

    applyModelAmbient(model, amb_col);

    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->entry(model->getModelData());
    }
    if (i_this->mpBrk != NULL) {
        i_this->mpBrk->entry(model->getModelData());
    }
    if (i_this->mpColorBtk != NULL) {
        i_this->mpColorBtk->entry(model->getModelData());
    }

    dComIfGd_setList();
    // №50-E: Nintendo draws only the visual door mesh — skip the controller (white triangle).
    bool drawController = true;
    if (i_this->mManifest.doorAttention) {
        for (int i = 0; i < i_this->mAttachCount; ++i) {
            if (i_this->mpAttach[i] != NULL && i_this->mAttachSlave[i] == 0) {
                drawController = false;
                break;
            }
        }
    }
    if (drawController) {
        i_this->mpMorf->entryDL();
    }
    if (i_this->mpCompanion != NULL && i_this->mpCompanion->getModel() != NULL) {
        applyModelAmbient(i_this->mpCompanion->getModel(), amb_col);
        i_this->mpCompanion->entryDL();
    }
    for (int i = 0; i < i_this->mAttachCount; ++i) {
        if (i_this->mpAttach[i] != NULL) {
            applyModelAmbient(i_this->mpAttach[i], amb_col);
            i_this->mpAttach[i]->entry();
        }
    }

    // I3: simple ground shadow (no Acch — use up-normal).
    cXyz gndNrm(0.0f, 1.0f, 0.0f);
    dComIfGd_setSimpleShadow(&i_this->current.pos, i_this->current.pos.y, 50.0f * i_this->scale.x,
                             &gndNrm, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex());
#if TARGET_PC_NATIVE_UI
    if (s_mountDialogue != NULL && s_mountDialogue->isVisible()) {
        s_mountDialogue->registerDraw();
    }
#endif
    return 1;
}

// №27 N4 helpers — operate on the nearest live ExtNpc mount to `from`.
namespace {

dExtNpcMount_c* findNearestMount(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* best = NULL;
    f32 bestD2 = maxDist * maxDist;
    // Walk tracked providers' live actors via name search is awkward; scan room players' peers
    // via fopAcIt is heavier. Use a simple iterative search of known stub IDs.
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        dExtNpcMount_c* best;
    } ctx{&from, bestD2, NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || m->mpMorf == NULL) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dz = ac->current.pos.z - c->from->z;
            const f32 d2 = dx * dx + dz * dz;
            if (d2 < c->bestD2) {
                c->bestD2 = d2;
                c->best = m;
            }
            return 0;
        },
        &ctx);
    return ctx.best;
}

bool rewriteIdentityIni(const char* modFolder, const char* proc, const char* displayName) {
    if (modFolder == NULL || proc == NULL || displayName == NULL) {
        return false;
    }
    const fs::path path =
        dusk::ConfigPath / "model_replacements" / modFolder / "population" / "identity.ini";
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    // Read existing, replace/append [proc] display_name=.
    std::string existing;
    {
        std::ifstream in(path);
        if (in) {
            existing.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }
    }
    const std::string header = std::string("[") + proc + "]";
    std::string out;
    bool replaced = false;
    std::istringstream iss(existing);
    std::string line;
    bool inSection = false;
    while (std::getline(iss, line)) {
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n')) {
            trimmed.pop_back();
        }
        if (!trimmed.empty() && trimmed[0] == '[') {
            if (inSection && !replaced) {
                out += "display_name=";
                out += displayName;
                out += "\n";
                replaced = true;
            }
            inSection = (trimmed == header);
            out += trimmed;
            out += "\n";
            continue;
        }
        if (inSection) {
            size_t eq = trimmed.find('=');
            std::string key = eq == std::string::npos ? trimmed : trimmed.substr(0, eq);
            while (!key.empty() && key.back() == ' ') {
                key.pop_back();
            }
            if (key == "display_name") {
                out += "display_name=";
                out += displayName;
                out += "\n";
                replaced = true;
                continue;
            }
        }
        out += trimmed;
        out += "\n";
    }
    if (inSection && !replaced) {
        out += "display_name=";
        out += displayName;
        out += "\n";
        replaced = true;
    }
    if (!replaced) {
        out += "\n";
        out += header;
        out += "\ndisplay_name=";
        out += displayName;
        out += "\n";
    }
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs) {
        return false;
    }
    ofs << out;
    return true;
}

}  // namespace

dExtNpcMount_c* dExtNpcMount_nearestDoorAttention(const cXyz& from, f32 maxDist) {
    // 0x7FFF = no facing preference (pure nearest).
    return dExtNpcMount_facedDoorAttention(from, (s16)0x7FFF, maxDist, NULL);
}

dExtNpcMount_c* dExtNpcMount_facedDoorAttention(const cXyz& from, s16 facingYaw, f32 maxDist,
                                                   f32* outDistXZ) {
    struct Ctx {
        const cXyz* from;
        s16 facingYaw;
        bool preferFaced;
        f32 maxD2;
        f32 bestFacedD2;
        f32 bestAnyD2;
        dExtNpcMount_c* bestFaced;
        dExtNpcMount_c* bestAny;
    };
    const bool preferFaced = (facingYaw != (s16)0x7FFF);
    Ctx ctx{&from,
            facingYaw,
            preferFaced,
            maxDist * maxDist,
            maxDist * maxDist,
            maxDist * maxDist,
            NULL,
            NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || m->mpMorf == NULL || !m->mManifest.doorAttention) {
                return 0;
            }
            if (roomLaneMountIsUnloading(m)) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dy = ac->current.pos.y - c->from->y;
            const f32 dz = ac->current.pos.z - c->from->z;
            if (dy > 250.0f || dy < -150.0f) {
                return 0;
            }
            const f32 d2 = dx * dx + dz * dz;
            if (d2 > c->maxD2) {
                return 0;
            }
            if (d2 < c->bestAnyD2) {
                c->bestAnyD2 = d2;
                c->bestAny = m;
            }
            if (!c->preferFaced) {
                return 0;
            }
            cXyz to(dx, 0.0f, dz);
            if (to.abs() < 1.0f) {
                c->bestFacedD2 = d2;
                c->bestFaced = m;
                return 0;
            }
            const s16 yawTo = to.atan2sX_Z();
            if (cLib_distanceAngleS(yawTo, c->facingYaw) <= 0x4000 && d2 < c->bestFacedD2) {
                c->bestFacedD2 = d2;
                c->bestFaced = m;
            }
            return 0;
        },
        &ctx);
    dExtNpcMount_c* best =
        (ctx.preferFaced && ctx.bestFaced != NULL) ? ctx.bestFaced : ctx.bestAny;
    if (outDistXZ != NULL) {
        if (best == NULL) {
            *outDistXZ = -1.0f;
        } else if (best == ctx.bestFaced) {
            *outDistXZ = std::sqrt(ctx.bestFacedD2);
        } else {
            *outDistXZ = std::sqrt(ctx.bestAnyD2);
        }
    }
    return best;
}

bool dExtNpcMount_playAnimNearest(const cXyz& from, f32 maxDist, const char* bckName) {
    if (bckName == NULL || bckName[0] == '\0') {
        return false;
    }
    dExtNpcMount_c* best = NULL;
    f32 bestD2 = maxDist * maxDist;
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        dExtNpcMount_c* best;
        bool preferDoor;
    } ctx{&from, bestD2, NULL, true};
    // Prefer doorAttention mounts (Knob); fall back to any nearby mount.
    for (int pass = 0; pass < 2; ++pass) {
        ctx.preferDoor = (pass == 0);
        ctx.best = NULL;
        ctx.bestD2 = bestD2;
        fopAcIt_Executor(
            [](void* actor, void* data) -> int {
                fopAc_ac_c* ac = (fopAc_ac_c*)actor;
                Ctx* c = (Ctx*)data;
                if (ac == NULL) {
                    return 0;
                }
                const s16 name = fopAcM_GetName(ac);
                if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e &&
                    name != fpcNm_NPC_P2_e && name != fpcNm_NPC_KDK_e) {
                    return 0;
                }
                dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
                if (m->mIsBg || m->mpMorf == NULL) {
                    return 0;
                }
                if (c->preferDoor && !m->mManifest.doorAttention) {
                    return 0;
                }
                const f32 dx = ac->current.pos.x - c->from->x;
                const f32 dz = ac->current.pos.z - c->from->z;
                const f32 d2 = dx * dx + dz * dz;
                if (d2 < c->bestD2) {
                    c->bestD2 = d2;
                    c->best = m;
                }
                return 0;
            },
            &ctx);
        if (ctx.best != NULL) {
            best = ctx.best;
            break;
        }
    }
    if (best == NULL) {
        return false;
    }
    const char* anim = bckName;
    if (best->mManifest.doorOpenBck[0]) {
        anim = best->mManifest.doorOpenBck;
    }
    // №55: callers may pass NULL and rely on the manifest; no anim resolved = no-op.
    if (anim == NULL || anim[0] == '\0') {
        return false;
    }
    setMountAnimation(best, anim, J3DFrameCtrl::EMode_NONE);
    DuskLog.info("[ExtNpcMount] playAnimNearest '{}' on '{}' ({})", anim, best->mManifest.proc,
                 best->mManifest.model);
    return true;
}

bool dExtNpcMount_cycleHeadNearest(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL) {
        return false;
    }
    // Cycle 1→2→3→1 among attach slot 0 if it looks like a numbered head.
    if (m->mManifest.attachCount <= 0 || m->mManifest.attach[0].model[0] == '\0') {
        DuskLog.warn("[ExtNpcMount] N4 cycle: nearest '{}' has no head attach", m->mManifest.proc);
        return false;
    }
    char model[64];
    std::snprintf(model, sizeof(model), "%s", m->mManifest.attach[0].model);
    char* dot = std::strstr(model, ".bdl");
    if (dot == NULL || dot < model + 2) {
        return false;
    }
    int idx = (dot[-2] - '0') * 10 + (dot[-1] - '0');
    if (idx < 1) {
        idx = 1;
    }
    idx += 1;
    if (idx > 8) {
        idx = 1;
    }
    // Rewrite digits in place.
    char prefix[64];
    const size_t preLen = (size_t)(dot - model - 2);
    if (preLen >= sizeof(prefix)) {
        return false;
    }
    std::memcpy(prefix, model, preLen);
    prefix[preLen] = '\0';
    char nextModel[64];
    std::snprintf(nextModel, sizeof(nextModel), "%s%02d.bdl", prefix, idx);
    // Probe arc for the next head; if missing, wrap to 01.
    if (dComIfG_getObjectRes(m->mManifest.arc, nextModel) == NULL) {
        idx = 1;
        std::snprintf(nextModel, sizeof(nextModel), "%s%02d.bdl", prefix, idx);
        if (dComIfG_getObjectRes(m->mManifest.arc, nextModel) == NULL) {
            return false;
        }
    }
    std::snprintf(m->mManifest.attach[0].model, sizeof(m->mManifest.attach[0].model), "%s",
                  nextModel);
    m->mHeadVariant = (u8)idx;
    // Rebuild attach slot 0 model on the actor solid heap path is hard mid-frame;
    // force a soft rebind via acquireMountedModel + replace J3DModel.
    void* raw = dComIfG_getObjectRes(m->mManifest.arc, nextModel);
    J3DModelData* data = acquireMountedModel(m->mManifest.arc, nextModel, raw);
    if (data == NULL) {
        return false;
    }
    J3DModel* modelObj = mDoExt_J3DModel__create(data, 0, 1);
    if (modelObj == NULL) {
        return false;
    }
    m->mpAttach[0] = modelObj;
    DuskLog.info("[ExtNpcMount] N4 cycle head '{}' → {} (var {})", m->mManifest.proc, nextModel,
                 idx);
    return true;
}

bool dExtNpcMount_setDisplayNameNearest(const cXyz& from, f32 maxDist, const char* name) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL || name == NULL || name[0] == '\0') {
        return false;
    }
    std::snprintf(m->mManifest.displayName, sizeof(m->mManifest.displayName), "%s", name);
    auto it = s_providers.find(m->mManifest.proc);
    if (it != s_providers.end()) {
        std::snprintf(it->second.displayName, sizeof(it->second.displayName), "%s", name);
    }
    const bool ok =
        rewriteIdentityIni(m->mManifest.modFolder, m->mManifest.proc, name);
    DuskLog.info("[ExtNpcMount] N4 lock identity '{}' → '{}' ({})", m->mManifest.proc, name,
                 ok ? "identity.ini ok" : "identity.ini FAIL");
    return ok;
}

const char* dExtNpcMount_nearestDisplayName(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL) {
        return "";
    }
    return m->mManifest.displayName[0] ? m->mManifest.displayName : m->mManifest.proc;
}

bool dExtNpcMount_isMountActor(const fopAc_ac_c* actor) {
    if (actor == NULL) {
        return false;
    }
    const s16 nm = fopAcM_GetName(const_cast<fopAc_ac_c*>(actor));
    if (nm != fpcNm_NPC_HENNA0_e && nm != fpcNm_NPC_MK_e && nm != fpcNm_NPC_P2_e &&
        nm != fpcNm_NPC_KDK_e) {
        return false;
    }
    const dExtNpcMount_c* m = static_cast<const dExtNpcMount_c*>(actor);
    return m->mIsBg || m->mpMorf != NULL || m->mManifest.valid;
}

// §41: expose census code already retained on the mount (no new state).
static void fillCensusNameFromSpawnSrc(const char* spawnSrc, char* out, size_t n) {
    if (out == NULL || n == 0) {
        return;
    }
    out[0] = '\0';
    if (spawnSrc == NULL || spawnSrc[0] == '\0') {
        return;
    }
    if (std::strncmp(spawnSrc, "census:", 7) == 0) {
        const char* start = spawnSrc + 7;
        const char* at = std::strchr(start, '@');
        size_t len = at != NULL ? static_cast<size_t>(at - start) : std::strlen(start);
        if (len >= n) {
            len = n - 1;
        }
        if (len > 0) {
            std::memcpy(out, start, len);
        }
        out[len] = '\0';
        return;
    }
    // Door / warm / other lanes — keep the raw source; an unexpected src is information.
    std::snprintf(out, n, "%s", spawnSrc);
}

bool dExtNpcMount_queryActor(const fopAc_ac_c* actor, dExtNpcIdentifyInfo* out) {
    if (out == NULL) {
        return false;
    }
    *out = {};
    if (!dExtNpcMount_isMountActor(actor)) {
        return false;
    }
    const dExtNpcMount_c* m = static_cast<const dExtNpcMount_c*>(actor);
    const dExtNpcManifest& man = m->mManifest;
    out->valid = true;
    std::snprintf(out->proc, sizeof(out->proc), "%s", man.proc[0] ? man.proc : "?");
    std::snprintf(out->displayName, sizeof(out->displayName), "%s",
                  man.displayName[0] ? man.displayName : "-");
    std::snprintf(out->modFolder, sizeof(out->modFolder), "%s",
                  man.modFolder[0] ? man.modFolder : "-");
    fillCensusNameFromSpawnSrc(m->mSpawnSrc, out->censusName, sizeof(out->censusName));
    out->socketArg = man.socketArg;
    out->headVariant = m->mHeadVariant;
    if (man.attachCount > 0 && man.attach[0].model[0]) {
        std::snprintf(out->headModel, sizeof(out->headModel), "%s", man.attach[0].model);
        std::snprintf(out->headJoint, sizeof(out->headJoint), "%s",
                      man.attach[0].joint[0] ? man.attach[0].joint : "head");
    }
    return true;
}

// §41: Z-target identity probe — one pointer compare/frame; log only on target change.
void dExtNpcMount_pollIdentifyProbe() {
    static fopAc_ac_c* s_lastLockTarget = NULL;
    static int s_identifySeq = 0;

    dAttention_c* attn = dComIfGp_getAttention();
    fopAc_ac_c* target = attn != NULL ? attn->LockonTarget(0) : NULL;
    if (target == s_lastLockTarget) {
        return;
    }
    s_lastLockTarget = target;
    if (target == NULL || !dExtNpcMount_isMountActor(target)) {
        return;
    }
    dExtNpcIdentifyInfo info{};
    if (!dExtNpcMount_queryActor(target, &info)) {
        return;
    }
    ++s_identifySeq;
    DuskLog.info("[ExtNpcId] #{} census={} proc={} arg={} head={} display={}", s_identifySeq,
                 info.censusName[0] ? info.censusName : "-", info.proc, info.socketArg,
                 info.headModel[0] ? info.headModel : "-",
                 info.displayName[0] ? info.displayName : "-");
}

// --- №81 EXTENSION-FIRST: native save write refuse ---------------------------------

bool dExtWwSave_isWwHostStage(const char* stageName) {
    // Neutral fork prefixes under /res/Stage/: R_DL* (interiors) + F_DL* (fields).
    if (stageName == NULL || stageName[1] != '_' || stageName[2] != 'D' || stageName[3] != 'L') {
        return false;
    }
    return stageName[0] == 'R' || stageName[0] == 'F';
}

bool dExtWwSave_isWwContentActive() {
    if (s_lastBgProc[0] != '\0' && std::strncmp(s_lastBgProc, "EXT_", 4) == 0) {
        return true;
    }
    if (!s_roomLaneRooms.empty()) {
        return true;
    }
    for (const auto& kv : s_bgMountIds) {
        if (kv.first.rfind("EXT_", 0) == 0) {
            return true;
        }
    }
    return false;
}

bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit) {
    if (!dExtWwSave_isWwContentActive()) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    // On a WW host stage: still refuse until the extension router lands (no vanilla
    // slot is the WW progression home under №81). Placeholder STAG values must not
    // accumulate real chest/switch/key bits.
    if (dExtWwSave_isWwHostStage(stage)) {
        DuskLog.warn(
            "[WwSave] REFUSED {} stageNo={} bit={} host='{}' bg='{}' — WW host waits on "
            "extension store (№81)",
            api != NULL ? api : "?", stageNo, bit, stage != NULL ? stage : "?", s_lastBgProc);
        return true;
    }
    // Vanilla host (F_SP115=LANAYRU today): refuse — WW must not touch vanilla memBit.
    DuskLog.warn(
        "[WwSave] REFUSED {} stageNo={} bit={} host='{}' bg='{}' — WW on vanilla host (№81)",
        api != NULL ? api : "?", stageNo, bit, stage != NULL ? stage : "?", s_lastBgProc);
    return true;
}

#endif  // TARGET_PC
