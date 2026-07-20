/**
 * d_a_knob00.cpp — §27 first decomp port (WW regular knob door).
 *
 * Minimal Outset cut: two-model swing + DoorK10 DEFAULT_KNOB_* + ExtNpc warp.
 * Omitted (quest-only): password, villa, Mt figure, pirateship, msgDoor, WW event bits.
 * No res/Object/Knob.h — string resource names only (§27 D4).
 *
 * WW Knob BDLs are bmd3 — must go through dExtNpcMount_acquireModelData (same as
 * NPC_KNOB). Raw dComIfG_getObjectRes cast → J3DModelData* AVs (log 021350).
 */
#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include "d/actor/d_a_knob00.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_kankyo.h"
#include "d/d_meter2_info.h"
#include "dusk/logging.h"
#include "f_op/f_op_actor_mng.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "m_Do/m_Do_mtx.h"
#include <cstring>

namespace {

static const char* const kArc = "Knob";  // mod arcs/Knob.arc → Object/Knob.arc
static const char* const kEvArc = "DoorK10";

static int CheckCreateHeap(fopAc_ac_c* i_this) {
    return static_cast<daKnob00_c*>(i_this)->CreateHeap();
}

}  // namespace

void daKnob00_c::setDoorKey(const char* key) {
    if (key == NULL) {
        mDoorKey[0] = '\0';
        return;
    }
    std::snprintf(mDoorKey, sizeof(mDoorKey), "%s", key);
}

int daKnob00_c::CreateHeap() {
    // Controller (tiny joint host) + visible door_a — match npc_knob.ini.
    // Outset spawn params encode doorIndex in bits 8+, NOT WW shape type.
    J3DModelData* modelData = dExtNpcMount_acquireModelData(kArc, "door.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §27 missing/unparseable door.bdl in '{}'", kArc);
        return 0;
    }
    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (mpModel == NULL) {
        DuskLog.warn("[Knob00] §27 door.bdl model create failed");
        return 0;
    }
    mpModel->setBaseScale(scale);
    calcMtx();

    J3DAnmTransform* anm =
        (J3DAnmTransform*)dComIfG_getObjectRes(kArc, "dooropenadoor.bck");
    if (anm == NULL || mBck.init(anm, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, false) == 0) {
        DuskLog.warn("[Knob00] §27 dooropenadoor.bck init failed");
        return 0;
    }

    JUTNameTab* names = modelData->getJointName();
    mJoint = names != NULL ? names->getIndex("DoorDummy") : -1;
    if (mJoint < 0) {
        DuskLog.warn("[Knob00] §27 DoorDummy joint missing");
        return 0;
    }

    modelData = dExtNpcMount_acquireModelData(kArc, "door_a.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §27 visual door_a.bdl missing/unparseable");
        return 0;
    }
    mpModel2 = mDoExt_J3DModel__create(modelData, 0x80000, 0x11020002);
    if (mpModel2 == NULL) {
        DuskLog.warn("[Knob00] §27 door_a.bdl model create failed");
        return 0;
    }

    cBgD_t* bgd = (cBgD_t*)dComIfG_getObjectRes(kArc, "door.dzb");
    if (bgd == NULL) {
        DuskLog.warn("[Knob00] §27 door.dzb missing — continue without BG");
        mpBgW = NULL;
        return 1;
    }
    mpBgW = JKR_NEW dBgW();
    if (mpBgW == NULL) {
        return 0;
    }
    mBck.entry(mpModel->getModelData());
    mpModel->calc();
    calcMtx();
    if (mpBgW->Set(bgd, cBgW::MOVE_BG_e, &mpModel->getBaseTRMtx()) == 1) {
        DuskLog.warn("[Knob00] §27 door.dzb Set failed");
        return 0;
    }
    mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
    return 1;
}

void daKnob00_c::calcMtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    if (mpModel != NULL) {
        mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    }
    MTXCopy(mDoMtx_stack_c::get(), mBgMtx);
}

bool daKnob00_c::bindEvents() {
    if (mEvBound) {
        return mEvtFront >= 0 || mEvtBack >= 0;
    }
    int st = dComIfG_resLoad(&mEvPhase, kEvArc);
    if (st != cPhs_COMPLEATE_e) {
        return false;
    }
    eventInfo.setArchiveName(kEvArc);
    mEvtFront = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_KNOB_DOOR_F_OPEN", 0xff);
    mEvtBack = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_KNOB_DOOR_B_OPEN", 0xff);
    mEvBound = 1;
    mEvtOrdered = -1;
    mStaffId = -1;
    DuskLog.info("[Knob00] §27 DoorK10 bound F={} B={}", (int)mEvtFront, (int)mEvtBack);
    return mEvtFront >= 0 || mEvtBack >= 0;
}

int daKnob00_c::frontCheck() const {
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player == NULL) {
        return 0;
    }
    cXyz playerDist = player->current.pos - current.pos;
    mDoMtx_stack_c::YrotS(-current.angle.y);
    mDoMtx_stack_c::multVec(&playerDist, &playerDist);
    return playerDist.z > 0.0f ? 0 : 1;
}

void daKnob00_c::setEventPrm() {
    if (!bindEvents()) {
        return;
    }
    const int side = frontCheck();
    const s16 evt = (side == 0) ? mEvtFront : mEvtBack;
    if (evt < 0) {
        return;
    }
    mEvtOrdered = evt;
    eventInfo.setArchiveName(kEvArc);
    eventInfo.setEventId(evt);
    eventInfo.setMapToolId(0xff);
    eventInfo.onCondition(dEvtCnd_CANDOOR_e);
}

int daKnob00_c::getDemoAction() {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[16] = {
        "WAIT",           "SETSTART",      "SETANGLE",       "ADJUSTMENT",
        "OPEN_PUSH",      "OPEN_PULL",     "OPEN_PUSH2",     "OPEN_PULL2",
        "OPEN_PUSH_STOP", "OPEN_PULL_STOP","TALK",           "TALK_END",
        "SETSTART_PUSH",  "SETSTART_PULL", "DEMO_OPEN",      "DEMO_CLOSE",
    };
    return dComIfGp_evmng_getMyActIdx(mStaffId, action_table, 16, 0, 0);
}

void daKnob00_c::startOpenAnim(int side) {
    if (mOpenStarted || mpModel == NULL) {
        return;
    }
    const char* bck = (side == 0) ? "dooropenadoor.bck" : "dooropenbdoor.bck";
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(kArc, bck);
    if (anm == NULL) {
        anm = (J3DAnmTransform*)dComIfG_getObjectRes(kArc, "dooropenadoor.bck");
    }
    if (anm == NULL) {
        return;
    }
    mBck.init(anm, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, true);
    mOpenStarted = 1;
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
    }
    DuskLog.info("[Knob00] §27 OPEN anim '{}'", bck);
}

bool daKnob00_c::openAnimDone() {
    if (!mOpenStarted) {
        return false;
    }
    return mBck.play() != 0;
}

void daKnob00_c::initOpenDemo() {
    mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
    mOpenStarted = 0;
    mAction = ACTION_DEMO;
    DuskLog.info("[Knob00] §27 demo BEGIN staff={} evt={}", mStaffId, (int)mEvtOrdered);
}

void daKnob00_c::demoProc() {
    if (mStaffId == -1) {
        return;
    }
    const int demoAction = getDemoAction();
    const int side = frontCheck();
    if (dComIfGp_evmng_getIsAddvance(mStaffId) != 0) {
        switch (demoAction) {
        case 4:
        case 5:
        case 6:
        case 7:
            startOpenAnim(side);
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
        if (!mOpenStarted) {
            startOpenAnim(side);
            if (!mOpenStarted) {
                dComIfGp_evmng_cutEnd(mStaffId);
            }
            break;
        }
        if (openAnimDone()) {
            dComIfGp_evmng_cutEnd(mStaffId);
        }
        break;
    default:
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    }
}

int daKnob00_c::create() {
    fopAcM_ct(this, daKnob00_c);
    mpModel = NULL;
    mpModel2 = NULL;
    mpBgW = NULL;
    mJoint = -1;
    mAction = ACTION_WAIT;
    mEvBound = 0;
    mOpenStarted = 0;
    mArcRetained = 0;
    mEvtFront = -1;
    mEvtBack = -1;
    mEvtOrdered = -1;
    mStaffId = -1;
    mDoorKey[0] = '\0';
    mSpawnSrc[0] = '\0';

    char proc[32] = {};
    char src[96] = {};
    char head[64] = {};
    char joint[32] = {};
    if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(this), proc, sizeof(proc), src, sizeof(src),
                                      head, sizeof(head), joint, sizeof(joint))) {
        if (src[0]) {
            std::snprintf(mSpawnSrc, sizeof(mSpawnSrc), "%s", src);
            if (std::strncmp(src, "door:", 5) == 0) {
                setDoorKey(src + 5);
            }
        }
    }

    int phase = dComIfG_resLoad(&mPhase, kArc);
    if (phase == cPhs_ERROR_e) {
        DuskLog.warn("[Knob00] §27 arc '{}' load ERROR", kArc);
        return cPhs_ERROR_e;
    }
    if (phase != cPhs_COMPLEATE_e) {
        return phase;
    }
    // Match daKnob20: adjust flag + size (PC doubles the low 24 bits).
    if (!fopAcM_entrySolidHeap(this, CheckCreateHeap, 0x80003800)) {
        DuskLog.warn("[Knob00] §27 entrySolidHeap FAILED key='{}'", mDoorKey);
        return cPhs_ERROR_e;
    }
    dExtNpcMount_retainArc(kArc);
    mArcRetained = 1;
    if (mpBgW != NULL && dComIfG_Bgsp().Regist(mpBgW, this)) {
        DuskLog.warn("[Knob00] §27 Bgsp Regist failed");
    }
    attention_info.position = current.pos;
    attention_info.position.y += 150.0f;
    eyePos = attention_info.position;
    attention_info.flags = fopAc_AttnFlag_DOOR_e;
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    fopAcM_SetMin(this, -200.0f, 0.0f, -200.0f);
    fopAcM_SetMax(this, 200.0f, 300.0f, 200.0f);
    DuskLog.info("[Knob00] §27 COMPLEATE key='{}' src='{}'", mDoorKey, mSpawnSrc);
    return cPhs_COMPLEATE_e;
}

int daKnob00_c::execute() {
    bindEvents();
    calcMtx();
    if (mpBgW != NULL) {
        mpBgW->Move();
    }

    if (mAction == ACTION_DEMO) {
        dMeter2Info_onGameStatus(2);
        if (mEvtOrdered >= 0 && dComIfGp_evmng_endCheck(mEvtOrdered)) {
            mAction = ACTION_WAIT;
            mStaffId = -1;
            dComIfGp_event_reset();
            DuskLog.info("[Knob00] §27 demo END → warp key='{}'", mDoorKey);
            dExtNpcDoors_tryNativeWarp(this, /*openAlreadyDone=*/true);
            mEvtOrdered = -1;
            mOpenStarted = 0;
            if (mpBgW != NULL) {
                dComIfG_Bgsp().Regist(mpBgW, this);
            }
            return 1;
        }
        mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
        demoProc();
        return 1;
    }

    if (eventInfo.checkCommandDoor()) {
        if (mEvtOrdered < 0) {
            setEventPrm();
        }
        initOpenDemo();
        return 1;
    }

    // Arm Open prompt when Link is near (TP linear area ≈ №91).
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player != NULL) {
        const f32 dx = player->current.pos.x - current.pos.x;
        const f32 dz = player->current.pos.z - current.pos.z;
        if (dx * dx + dz * dz < 110.0f * 110.0f) {
            setEventPrm();
        }
    }
    return 1;
}

int daKnob00_c::draw() {
    if (mpModel == NULL) {
        return 1;
    }
    g_env_light.settingTevStruct(0x10, &current.pos, &tevStr);
    g_env_light.setLightTevColorType_MAJI(mpModel, &tevStr);
    mBck.entry(mpModel->getModelData());
    mDoExt_modelUpdateDL(mpModel);
    mBck.remove(mpModel->getModelData());
    if (mpModel2 != NULL && mJoint >= 0) {
        mpModel2->setBaseTRMtx(mpModel->getAnmMtx(mJoint));
        g_env_light.setLightTevColorType_MAJI(mpModel2, &tevStr);
        mDoExt_modelUpdateDL(mpModel2);
    }
    return 1;
}

int daKnob00_c::Delete() {
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
        mpBgW = NULL;
    }
    dComIfG_resDelete(&mPhase, kArc);
    if (mArcRetained) {
        dExtNpcMount_releaseArc(kArc);
        mArcRetained = 0;
    }
    if (mEvBound) {
        dComIfG_resDelete(&mEvPhase, kEvArc);
    }
    return 1;
}

static int daKnob00_Create(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->create();
}
static int daKnob00_Delete(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->Delete();
}
static int daKnob00_Execute(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->execute();
}
static int daKnob00_Draw(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->draw();
}
static int daKnob00_IsDelete(void* i_this) {
    (void)i_this;
    return 1;
}

static DUSK_CONST actor_method_class l_daKnob00_Method = {
    (process_method_func)daKnob00_Create,  (process_method_func)daKnob00_Delete,
    (process_method_func)daKnob00_Execute, (process_method_func)daKnob00_IsDelete,
    (process_method_func)daKnob00_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_KNOB00 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_KNOB00_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daKnob00_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daKnob00_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#endif  // TARGET_PC
