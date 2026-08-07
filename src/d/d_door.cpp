// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_door.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
/**
 * d_door.cpp — WW door base layer
 *
 * ============================================================================
 * §328 WW DOOR LAYER DIRECT PORT — body VERBATIM from the WW donor
 * (D:/XXXXXXX/WW DP/src/d/d_door.cpp, fully matched). VERSION blocks follow the
 * RETAIL (#else) side like every landed WW port (§327 lamp/mshokki precedent).
 * Direct-port crash recipes applied at the call sites only (banners inline):
 *   #1 DN-3  : MODELS via dExtNpcMount_acquireModelData(arc, member) —
 *              donor res ids resolved to member names by RARC parse of the donor
 *              arcs (D:/XXXXXXX/Ex WW/files/res/Object/) + the donor generated
 *              headers (WW DP/assets/GZLE01/res/Object/{Key,Hkyo}.h agree):
 *                Key.arc : BCK 4=vlocb.bck 5=vlocn.bck ; BDLM 8=vlocb.bdl 9=vlocn.bdl
 *                Hkyo.arc: BDLM 4=hkyo1.bdl ; BRK 7=hkyo1a 8=hkyo1b 9=hkyo1c (.brk)
 *   tevStr.mRoomNo -> tevStr.room_no ; dComIfGd_getView()->mLookat.mEye ->
 *              ->lookat.eye (port field names, identical semantics)
 *   eventInfo.setToolId -> setMapToolId ; evmng_getMyStaffId 1-arg -> 3-arg
 *   makeEventId: WW per-stage tool-id table (dComIfGp_evmng_getToolId) has no
 *              port equivalent — the HOST event_list carries the KNOB events;
 *              actor-form getEventIdx(this, name, 0xFF) (TP knob20 idiom)
 *   mDoExt_bckAnm::init donor (modelData, bck, ...) -> port 7-arg (no modelData);
 *              mDoExt_brkAnm::init donor 4/10-arg -> port 7-arg
 *   fopMsgM_messageSet(msgId, cXyz*) -> port 2-arg (msgId, 1000) — the port
 *              dropped the position overload (d_npc.cpp §239 precedent)
 *   fopMsgStts_* / mStatus -> fopMsg_MODE_* / mode (§245 mapping, f_op_msg.h)
 *   particle_set 7-arg -> port 10-arg; smoke callback BRIDGE-OWED (see site)
 *   JUT_ASSERT -> NULL-guard/log (port macro panics; donor RETAIL assert
 *              compiles out — guard is the faithful behavior)
 *   cPhs_State -> cPhs_Step ; *dComIfG_Bgsp() -> reference form
 * WW save event bits ride the DONOR event-flag block via the §303 route header
 * (LAST include) with value-faithful WWEV_* ids — never TP's table. WW TMP bits
 * and the WW baton (isTact) have no donor block yet: inert-faithful local stubs,
 * §328 BRIDGE-OWED (see banners below).
 * DN-1 audit: this layer registers NO BG and stamps NO room id on collision
 * owners — dDoor_* room fields are actor/tevStr identities resolved from the
 * door's own placement (getF/BRoomNo) or the player's stay room, donor-verbatim.
 * ============================================================================
 */

#include "d/dolzel.h" // IWYU pragma: keep
#include "d/d_door.h"
#include "d/d_com_inf_game.h"
#include "d/d_vibration.h"
#include "d/actor/d_a_player.h"
#include "SSystem/SComponent/c_angle.h"
#include "SSystem/SComponent/c_math.h"    // §328 cM_ssin/cM_scos
#include "SSystem/SComponent/c_lib.h"     // §328 cLib_chaseF
#include "f_op/f_op_actor_mng.h"          // §328 fopAcM_* helpers (toripost include set)
#include "f_op/f_op_msg_mng.h"            // §328 fopMsgM_messageSet/SearchByID
#include "f_pc/f_pc_manager.h"            // §328 fpcM_ERROR_PROCESS_ID_e
#include "f_op/f_op_view.h"               // §328 view_class (lookat.eye)
#include "m_Do/m_Do_ext.h"                // §328 mDoExt_J3DModel__create / modelUpdateDL
#include "m_Do/m_Do_mtx.h"                // §328 mDoMtx_stack_c
#include "d/d_kankyo.h"                   // §328 g_env_light
#include "d/d_particle_name.h"            // §328 ID_AK_JT_ELEMENTSMOKE00 (native port enum, unscoped)
#include "d/d_ext_npc_mount.h"            // §328 DN-3 parse-at-consume model resolver
#include "dusk/logging.h"                 // §328 retail-assert fallbacks log instead of panicking
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DSkinDeform.h"  // §328 boss-key chain deform
#include "JSystem/JParticle/JPAEmitter.h"            // §328 JPABaseEmitter setRate/setSpread
#include <cstdlib>                        // §328 abs()

// §303 SYMBOL ROUTE — MUST BE THE LAST INCLUDE. Remaps this TU's verbatim
// dComIfGs_*EventBit calls (all WW dSv_event indices) to the donor event-flag
// block (d_ext_save_flags) — never TP's table.
#include "d/d_ext_save_flags_route.h"
#include "d/d_kankyo_ww.h"           // §404 WW lighting write-path (was the empty stub)

// §328 ========================================================================
// WW-absent VALUE-FAITHFUL constants (donor numbering; toripost §253 pattern —
// kept LOCAL to this TU, not the shared shims header).
// ----------------------------------------------------------------------------
// WW JAZelAudio_SE.h ids — value-faithful (§327 precedent; the WW SE bank is not
// loaded, a later audio pass maps real Z2SE ids).
#define JA_SE_OBJ_BOSS_LOCK_OPEN   0x69AE
#define JA_SE_OBJ_DOOR_CHAIN_OPEN  0x6948
#define JA_SE_OBJ_CAGE_CLOSE       0x697C
#define JA_SE_OBJ_CAGE_OPEN        0x697D
#define JA_SE_OBJ_STN_DOOR_STL_BAR 0x6906
#define JA_SE_OBJ_RES_DOOR_BLINK   0x61D4
// WW dSv_event_flag_c scoped bits -> unscoped value-faithful WWEV_* (the §244
// pattern; value == donor name, verbatim from WW d_save_event_flag.inc). Routed
// to the DONOR flag block by the §303 route header above.
#define WWEV_UNK_1710 0x1710
#define WWEV_UNK_1704 0x1704
#define WWEV_UNK_1B01 0x1B01
#define WWEV_UNK_2602 0x2602
#define WWEV_UNK_2601 0x2601
// WW dSv_event_tmp_flag_c bits (d_save_event_tmp_flag.inc). >>> BRIDGE-OWED §328:
// the donor TEMP-flag block is NOT ported (d_ext_save_flags carries event bits
// only); the port's own dComIfGs_isTmpBit is TP's table and MUST NOT see WW
// indices. Inert-faithful stub below (reads FALSE = "no tmp event pending"). <<<
#define WWTMP_UNK_0108 0x0108
#define WWTMP_UNK_0110 0x0110
static BOOL dExtDoorWw_isTmpBit(u16 i_no) {
    (void)i_no;
    return FALSE;
}
// WW baton-song query (dComIfGs_isTact). >>> BRIDGE-OWED §328: the WW baton
// subsystem is absent; FALSE keeps the hkyo case-3 branch on its donor
// "song not played" side (setAnm(0)). Donor free-fn name kept. <<<
static BOOL dComIfGs_isTact(u8 i_no) {
    (void)i_no;
    return FALSE;
}
// WW ship-position stage helper (dStage_setShipPos). >>> BRIDGE-OWED §328: no
// ship subsystem in the port; knob00 always calls openInitCom(0) so the guarded
// call is dead — no-op keeps the source verbatim. Donor free-fn name kept. <<<
static void dStage_setShipPos(int i_shipId, int i_roomNo) {
    (void)i_shipId;
    (void)i_roomNo;
}
// WW AGB (GBA Tingle-link) map-send flags — the subsystem does not exist on TP.
// Inert no-ops keep the donor source verbatim.
static void dComIfGp_map_setAGBMapSendStopFlg() {}
// WW status bit fopAcStts_UNK1000_e == 1 << 12 == the port's named
// fopAcStts_STAFF_SHUTTER_e ("primary participant of an active SHUTTER_DOOR
// staff") — same bit, port name; donor spelling kept.
#define fopAcStts_UNK1000_e fopAcStts_STAFF_SHUTTER_e
// WW J3DErrType names (donor JSystem J3DSys.h) — the port's J3D layer kept the
// values (setSkinDeform returns s32) but dropped the enum names.
#define J3DErrType_Success     0
#define J3DErrType_OutOfMemory 4
// WW fopMsgStts_* names -> port fopMsg_MODE_* (§245 value-mapped enum,
// f_op_msg.h) — donor spellings kept so the msg proc stays verbatim.
#define fopMsgStts_MSG_TYPING_e    fopMsg_MODE_MSG_TYPING_e
#define fopMsgStts_MSG_DISPLAYED_e fopMsg_MODE_MSG_DISPLAYED_e
#define fopMsgStts_MSG_CONTINUES_e fopMsg_MODE_MSG_CONTINUE_e
#define fopMsgStts_MSG_ENDS_e      fopMsg_MODE_MSG_END_e
#define fopMsgStts_BOX_CLOSED_e    fopMsg_MODE_BOX_CLOSED_e
#define fopMsgStts_MSG_DESTROYED_e fopMsg_MODE_MSG_DESTROYED_e
// §328 ========================================================================

/* 8006B39C-8006B3A8       .text getSwbit__12dDoor_info_cFv */
u8 dDoor_info_c::getSwbit() {
    return fopAcM_GetParam(this) & 0xFF;
}

/* 8006B3A8-8006B3B4       .text getSwbit2__12dDoor_info_cFv */
u8 dDoor_info_c::getSwbit2() {
    return (fopAcM_GetParam(this) >> 0x14) & 0xFF;
}

/* 8006B3B4-8006B3C0       .text getType__12dDoor_info_cFv */
u8 dDoor_info_c::getType() {
    return (fopAcM_GetParam(this) >> 0x08) & 0xF;
}

/* 8006B3C0-8006B3EC       .text setType__12dDoor_info_cFUc */
void dDoor_info_c::setType(u8 type) {
    if (type >= 0x10)
        return;
    fopAcM_SetParam(this, fopAcM_GetParam(this) & ~(0xF << 0x08));
    fopAcM_SetParam(this, fopAcM_GetParam(this) | ((type << 0x08) & (0xFF << 0x08)));
}

/* 8006B3EC-8006B3F8       .text getEventNo__12dDoor_info_cFv */
u8 dDoor_info_c::getEventNo() {
    return (fopAcM_GetParam(this) >> 0x0C) & 0xFF;
}

/* 8006B3F8-8006B404       .text getFRoomNo__12dDoor_info_cFv */
u8 dDoor_info_c::getFRoomNo() {
    return home.angle.x & 0x3F;
}

/* 8006B404-8006B410       .text getBRoomNo__12dDoor_info_cFv */
u8 dDoor_info_c::getBRoomNo() {
    return (home.angle.x >> 0x06) & 0x3F;
}

/* 8006B410-8006B41C       .text getShipId__12dDoor_info_cFv */
u8 dDoor_info_c::getShipId() {
    return home.angle.z & 0x3F;
}

/* 8006B41C-8006B428       .text getArg1__12dDoor_info_cFv */
u8 dDoor_info_c::getArg1() {
    return (home.angle.z >> 0x08) & 0xFF;
}

/* 8006B428-8006B4C4       .text adjoinPlayer__12dDoor_info_cFv */
BOOL dDoor_info_c::adjoinPlayer() {
    int frontRoomNo = getFRoomNo();
    int backRoomNo = getBRoomNo();
    if (frontRoomNo == 0x3F || backRoomNo == 0x3F)
        return TRUE;
    if (dComIfGp_roomControl_checkRoomDisp(frontRoomNo) ||
        dComIfGp_roomControl_checkRoomDisp(backRoomNo))
    {
        return TRUE;
    }
    return FALSE;
}

/* 8006B4C4-8006B554       .text getViewRoomNo__12dDoor_info_cFv */
u8 dDoor_info_c::getViewRoomNo() {
    // §328 WW view->mLookat.mEye == port view->lookat.eye (same eye position).
    cXyz delta = dComIfGd_getView()->lookat.eye - current.pos;
    if (delta.inprodXZ(mAngleVec) < 0.0f)
        return getBRoomNo();
    else
        return getFRoomNo();
}

/* 8006B554-8006B5E4       .text frontCheckOld__12dDoor_info_cFv */
s32 dDoor_info_c::frontCheckOld() {
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    cSGlobe globe(player->current.pos - current.pos);
    cSAngle angle1;
    angle1 = (globe.U() - current.angle.y);
    s16 angle = angle1.Abs();
    // §328 RETAIL (#else) side of the donor VERSION_DEMO block kept.
    if (angle < 0x4000 && angle >= 0)
        return 0;
    else
        return 1;
}

/* 8006B65C-8006B6F4       .text frontCheck__12dDoor_info_cFv */
s32 dDoor_info_c::frontCheck() {
    s32 stayNo = dComIfGp_roomControl_getStayNo();
    if (getFRoomNo() == getBRoomNo())
        return frontCheckOld();

    return stayNo == getFRoomNo() ? 0 :
        stayNo == getBRoomNo() ? 1 : 2;
}

/* 8006B6F4-8006B824       .text drawCheck_local__12dDoor_info_cFv */
s32 dDoor_info_c::drawCheck_local() {
    if (!adjoinPlayer() && eventInfo.checkCommandDemoAccrpt() == FALSE && eventInfo.checkCommandDoor() ==  FALSE)
        return 0;

    // §328 WW tevStr.mRoomNo == port tevStr.room_no (dKy_tevstr_c field name).
    if (getFRoomNo() == 0x3F || getBRoomNo() == 0x3F)
        tevStr.room_no = dComIfGp_roomControl_getStayNo();
    else
        tevStr.room_no = getViewRoomNo();

    current.roomNo = tevStr.room_no;

    u8 front = frontCheck();
    if (front == 2)
        mRoomNo = -1;
    else
        mRoomNo = dComIfGp_roomControl_getStayNo();

    if (!dComIfGp_roomControl_checkRoomDisp(tevStr.room_no))
        return 1;

    if (getViewRoomNo() == 0x3F && (getFRoomNo() != 0x3F || getBRoomNo() != 0x3F))
        return 1;
    else
        return 2;
}

/* 8006B824-8006B8AC       .text drawCheck__12dDoor_info_cFi */
s32 dDoor_info_c::drawCheck(int mode) {
    s32 rt = drawCheck_local();
    if (rt != 0) {
        if (mode) {
            fopAcM_SetStatusMap(this, 0x0A);
        }  else {
            fopAcM_SetStatusMap(this, 0x09);
        }
    } else {
        fopAcM_OffStatus(this, fopAcStts_SHOWMAP_e);
    }
    return rt == 2;
}

/* 8006B8AC-8006B954       .text checkExecute__12dDoor_info_cFv */
s32 dDoor_info_c::checkExecute() {
    mFrontCheck = frontCheck();
    if (fopAcM_CheckStatus(this, fopAcStts_UNK1000_e))
        return 1;

    if (eventInfo.checkCommandDemoAccrpt() || eventInfo.checkCommandDoor())
        return 2;

    if (mRoomNo2 != dComIfGp_roomControl_getStayNo())
        return 0;

    if (mFrontCheck != 2 && adjoinPlayer())
        return 2;

    return 0;
}

/* 8006B954-8006BA30       .text startDemoProc__12dDoor_info_cFv */
void dDoor_info_c::startDemoProc() {
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    // §328 evmng_getMyStaffId 1-arg -> port 3-arg (§253 recipe).
    mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
    shape_angle.y = current.angle.y;
    // §328 donor JUT_ASSERT(DEMO_SELECT(271, 274), player) — retail assert
    // compiles out; the port macro panics -> NULL-guard.
    if (player == NULL) {
        return;
    }
    s16 delta = player->home.angle.y - home.angle.y;
    if (delta < 0)
        delta = -delta;
    if (delta < 0x1000 && delta > -1000)
        shape_angle.y += 0x7FFF;
}

/* 8006BA30-8006BB5C       .text makeEventId__12dDoor_info_cFi */
void dDoor_info_c::makeEventId(int spl) {
    static DUSK_CONSTEXPR char DUSK_CONST* table[] = {
        "DEFAULT_STOP_OPEN",
        "DEFAULT_STOP_OPEN",
        "DEFAULT_SHUTTER_DOOR_F",
        "DEFAULT_SHUTTER_DOOR_F",
        "DEFAULT_SHUTTER_DOOR_F_STOP",
        "DEFAULT_SHUTTER_DOOR_F_STOP",
        "DEFAULT_BS_SHUTTER_F",
        "DEFAULT_KNOB_DOOR_F_OPEN",
        "DEFAULT_KNOB_DOOR_B_OPEN",
        "DEFAULT_KNOB_TALK",
        "HKYO_DOOR",
        "SHUTTER_DROP_CARRY",
    };

    for (s32 i = 0; i < 12; i++) {
        // §328 WW dComIfGp_evmng_getToolId(eventNo, i) reads the stage's
        // event-pack tool-id table — no port equivalent. The HOST event_list
        // already carries the KNOB events; the port resolves by ACTOR-form
        // getEventIdx(this, name, 0xFF) (TP knob20 / §27 idiom, room-typed).
        mToolId[i] = 0xFF;
        mEventIdx[i] = dComIfGp_getEventManager().getEventIdx(this, table[i], mToolId[i]);
    }

    s16 r27;
    switch (spl) {
    case 1:
        r27 = 2;
        mEventIdx[r27] = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_SHUTTER_DOOR_10", mToolId[r27]);
        r27 = 3;
        mEventIdx[r27] = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_SHUTTER_DOOR_10", mToolId[r27]);
        break;
    case 2:
        r27 = 2;
        mEventIdx[r27] = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_SHUTTER_DOOR_12", mToolId[r27]);
        r27 = 3;
        mEventIdx[r27] = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_SHUTTER_DOOR_12", mToolId[r27]);
        break;
    }
}

/* 8006BB5C-8006BBB0       .text initProc__12dDoor_info_cFi */
void dDoor_info_c::initProc(int spl) {
    mAngleVec.set(cM_ssin(home.angle.y), 0.0f, cM_scos(home.angle.y));
    makeEventId(spl);
}

/* 8006BBB0-8006BC50       .text initOpenDemo__12dDoor_info_cFi */
void dDoor_info_c::initOpenDemo(int evt) {
    if (m2C6 != 9)
        dComIfGp_map_setAGBMapSendStopFlg();

    shape_angle.y = current.angle.y;
    if (mFrontCheck == 1)
        shape_angle.y += 0x7FFF;

    mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);  // §328 3-arg
    if (evt)
        dComIfGp_event_onEventFlag(2);
}

/* 8006BC50-8006BDBC       .text checkArea__12dDoor_info_cFfff */
BOOL dDoor_info_c::checkArea(f32 f1, f32 f2, f32 distXZSqMax) {
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    cXyz delta = player->current.pos - current.pos;
    f32 distXZSq = delta.abs2XZ();
    if (distXZSq > distXZSqMax)
        return FALSE;

    delta.normalize();
    f32 dot = delta.inprodXZ(mAngleVec);
    f32 d3 = distXZSq * dot * dot;
    if (d3 > f2)
        return FALSE;

    if (distXZSq - d3 > f1)
        return FALSE;

    s16 checkAngle = current.angle.y;
    if (mFrontCheck == 1)
        checkAngle += 0x7FFF;
    if (abs((s16)(checkAngle - player->current.angle.y)) < 0x5000)
        return FALSE;

    return TRUE;
}

/* 8006BDBC-8006BE94       .text openInitCom__12dDoor_info_cFi */
void dDoor_info_c::openInitCom(int ship) {
    if (mFrontCheck == 0) {
        mFromRoomNo = getFRoomNo();
        mToRoomNo = getBRoomNo();
    } else {
        mFromRoomNo = getBRoomNo();
        mToRoomNo = getFRoomNo();
    }

    if (mFromRoomNo != mToRoomNo && mFromRoomNo != 0x3F && mToRoomNo != 0x3F)
        dComIfGp_roomControl_offStatusFlag(mToRoomNo, 0x08);

    if (ship && valShipId())
        dStage_setShipPos(getShipId(), mToRoomNo);  // §328 BRIDGE-OWED no-op (see banner)
}

/* 8006BE94-8006BF74       .text openProcCom__12dDoor_info_cFv */
void dDoor_info_c::openProcCom() {
    daPy_py_c* player = daPy_getPlayerActorClass();
    s16 angle = shape_angle.y + 0x7FFF;
    cXyz target;
    cXyz playerPos = player->current.pos;
    target = current.pos;
    target.x += cM_ssin(angle) * -100.0f;
    target.z += cM_scos(angle) * -100.0f;
    playerPos.x = playerPos.x * 0.9f + target.x * 0.1f;
    playerPos.z = playerPos.z * 0.9f + target.z * 0.1f;
    // §328 port setPlayerPosAndAngle takes a trailing BOOL (knob20 idiom: 0).
    player->setPlayerPosAndAngle(&playerPos, player->current.angle.y, 0);
}

/* 8006BF74-8006C0A4       .text closeEndCom__12dDoor_info_cFv */
void dDoor_info_c::closeEndCom() {
    if (mFromRoomNo != mToRoomNo && mFromRoomNo != 0x3F && mToRoomNo != 0x3F)
        dComIfGp_roomControl_onStatusFlag(mFromRoomNo, 0x08);

    daPy_py_c* player = daPy_getPlayerActorClass();
    cXyz delta = player->current.pos - current.pos;
    f32 dot = delta.inprodXZ(mAngleVec);
    f32 rad = dot < 0.0f ? 180.0f : -180.0f;

    cXyz pos(current.pos.x - rad * mAngleVec.x, current.pos.y, current.pos.z - rad * mAngleVec.z);
    s8 roomNo = fopAcM_GetRoomNo(player);
    dComIfGs_setRestartRoom(
        pos,
        dot > 0.0f ? current.angle.y : (s16)(current.angle.y + 0x8000),
        roomNo
    );
}

/* 8006C0A4-8006C0EC       .text getDemoAction__12dDoor_info_cFv */
s32 dDoor_info_c::getDemoAction() {
    // §328 donor `static char*` -> const-qualified for the port compiler
    // (knob20 idiom); strings verbatim.
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[] = {
        "WAIT",
        "STOP_OPEN",
        "STOP_CLOSE",
        "OPEN",
        "CLOSE",
        "SMOKE",
        "SMOKE_END",
        "SETGOAL",
        "UNLOCK",
        "SETSTART",
        "SETANGLE",
        "ADJUSTMENT",
        "OPEN_PUSH",
        "OPEN_PULL",
        "OPEN_PUSH2",
        "OPEN_PULL2",
        "TALK",
        "SETSTART_PUSH",
        "SETSTART_PULL",
        "END_CHECK",
        "DROP_BF",
        "DROP_AF",
    };

    return dComIfGp_evmng_getMyActIdx(mStaffId, action_table, ARRAY_SIZE(action_table), FALSE, 0);
}

/* 8006C0EC-8006C1D8       .text setGoal__12dDoor_info_cFv */
void dDoor_info_c::setGoal() {
    daPy_py_c* player = daPy_getPlayerActorClass();
    cXyz playerPos = player->current.pos;
    cXyz target = current.pos;
    s16 angle = shape_angle.y + 0x7FFF;
    playerPos.x += cM_ssin(angle) * 350.0f;
    playerPos.z += cM_scos(angle) * 350.0f;
    target.x += cM_ssin(angle) * 250.0f;
    target.z += cM_scos(angle) * 250.0f;
    playerPos.x = playerPos.x * 0.8f + target.x * 0.2f;
    playerPos.z = playerPos.z * 0.8f + target.z * 0.2f;
    dComIfGp_evmng_setGoal(&playerPos);
}

/* 8006C1D8-8006C200       .text setPlayerAngle__12dDoor_info_cFi */
void dDoor_info_c::setPlayerAngle(BOOL flip) {
    s16 angle = shape_angle.y;
    daPy_py_c* link = (daPy_py_c*)dComIfGp_getLinkPlayer();
    if (flip) {
        angle += 0x7FFF;
    }
    link->changeDemoMoveAngle(angle);
}

/* 8006C200-8006C2BC       .text setPosAndAngle__12dDoor_info_cFP4cXyzs */
void dDoor_info_c::setPosAndAngle(cXyz* pPos, s16 angle) {
    if (eventInfo.checkCommandDemoAccrpt() || eventInfo.checkCommandDoor()) {
        return;
    }
    if (pPos) {
        attention_info.position = current.pos = *pPos;
        attention_info.position.y += 150.0f;
        eyePos = attention_info.position;
    }
    current.angle.y = angle;
    shape_angle.y = current.angle.y;
    mAngleVec.set(cM_ssin(current.angle.y), 0.0f, cM_scos(current.angle.y));
}

/* 8006C2BC-8006C388       .text smokeInit__13dDoor_smoke_cFP12dDoor_info_c */
void dDoor_smoke_c::smokeInit(dDoor_info_c* door) {
    mPos = door->current.pos;
    mRot.y = door->shape_angle.y;
    // §328 donor 7-arg particle_set (dPa_name::ID scoped; smoke callback) ->
    // port 10-arg overload, unscoped native id. >>> BRIDGE-OWED: the port's
    // callback slot is dPa_levelEcallBack*; the WW dPa_smokeEcallBack (wind
    // on/off behavior) is only a §225 no-op shim, so NULL is passed here and
    // mSmokeCb.remove() below is a no-op — real smoke wind/kill behavior lands
    // with the wave-emit particle pass. Dormant for knob00 (SMOKE actions are
    // shutter/door20 territory). <<<
    JPABaseEmitter* emtr = dComIfGp_particle_set((u16)ID_AK_JT_ELEMENTSMOKE00, &mPos, &mRot, NULL,
                                                 0xAA, NULL, fopAcM_GetRoomNo(door), NULL, NULL, NULL);
    m34 = 0;
    m35 = 0;
    if (emtr != NULL) {
        emtr->setRate(16.0f);
        emtr->setSpread(0.35f);
        JGeometry::TVec3<f32> scale(2.0f, 2.0f, 2.0f);
        emtr->setGlobalScale(scale);
    }
}

/* 8006C388-8006C41C       .text smokeProc__13dDoor_smoke_cFP12dDoor_info_c */
void dDoor_smoke_c::smokeProc(dDoor_info_c* door) {
    if (m35 != 0) {
        f32 wave = (m34 * 20) * ((m34 & 1) ? 1.0f : -1.0f);
        m34++;
        mPos.x += wave * door->mAngleVec.z;
        mPos.z += wave * door->mAngleVec.x;
    } else {
        m35 = 1;
    }
}

/* 8006C41C-8006C448       .text smokeEnd__13dDoor_smoke_cFv */
void dDoor_smoke_c::smokeEnd() {
    mSmokeCb.remove();  // §328 shim no-op (see smokeInit banner)
}

/* 8006C448-8006C478       .text keyResLoad__12dDoor_key2_cFv */
cPhs_Step dDoor_key2_c::keyResLoad() {  // §328 cPhs_State -> cPhs_Step
    return dComIfG_resLoad(&mPhs, "Key");
}

/* 8006C478-8006C4A8       .text keyResDelete__12dDoor_key2_cFv */
void dDoor_key2_c::keyResDelete() {
    dComIfG_resDelete(&mPhs, "Key");
}

/* 8006C4A8-8006C5E8       .text keyInit__12dDoor_key2_cFP12dDoor_info_c */
void dDoor_key2_c::keyInit(dDoor_info_c* door) {
    if (mpModel != NULL && mbEnabled && !door->mFrontCheck) {
        if ((int)door->getSwbit() < 0x80)
            dComIfGs_onSwitch(door->getSwbit(), -1);
        if (!mbIsBossDoor)
            dComIfGp_setItemKeyNumCount(-1);
        switch (mbIsBossDoor) {
        case 1: fopAcM_seStart(door, JA_SE_OBJ_BOSS_LOCK_OPEN, 0); break;
        default: fopAcM_seStart(door, JA_SE_OBJ_DOOR_CHAIN_OPEN, 0); break;
        }
        m20 = 1;
    } else {
        m20 = 0;
    }
}

/* 8006C5E8-8006C650       .text keyProc__12dDoor_key2_cFv */
BOOL dDoor_key2_c::keyProc() {
    if (m20) {
        if (mBckAnim.play()) {
            keyOff();
            m20 = 0;
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return TRUE;
    }
}

/* 8006C650-8006C764       .text keyCreate_Nkey__12dDoor_key2_cFv */
BOOL dDoor_key2_c::keyCreate_Nkey() {
    // §328 DN-3 (recipe #1): getObjectRes returns RAW bytes on this port — the
    // MODEL goes through acquireModelData (parse-at-consume).
    // Donor: dComIfG_getObjectRes("Key", dRes_INDEX_KEY_BDL_VLOCN_e /*=9*/);
    // res id 9 == "vlocn.bdl" (donor Key.h + donor-arc RARC parse agree).
    // Donor JUT_ASSERT(DEMO_SELECT(713,716), modelData) -> NULL-guard FALSE.
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Key", "vlocn.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Door] §328 keyCreate_Nkey: 'Key' member 'vlocn.bdl' not resolvable");
        return FALSE;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (mpModel == NULL)
        return FALSE;

    // §328 donor id dRes_INDEX_KEY_BCK_VLOCN_e == 5 == "vlocn.bck".
    J3DAnmTransform* bck = (J3DAnmTransform*)dComIfG_getObjectRes("Key", "vlocn.bck");
    // §328 donor init(modelData, bck, TRUE, EMode_NONE) -> port 7-arg (WW
    // default args 1.0f/0/-1 made explicit; no modelData on the port init).
    if (!mBckAnim.init(bck, TRUE, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, false))
        return FALSE;

    return TRUE;
}

/* 8006C764-8006C910       .text keyCreate_Bkey__12dDoor_key2_cFv */
BOOL dDoor_key2_c::keyCreate_Bkey() {
    // §328 DN-3: donor dRes_INDEX_KEY_BDL_VLOCB_e == 8 == "vlocb.bdl".
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Key", "vlocb.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Door] §328 keyCreate_Bkey: 'Key' member 'vlocb.bdl' not resolvable");
        return FALSE;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (mpModel == NULL)
        return FALSE;

    // §328 donor id dRes_INDEX_KEY_BCK_VLOCB_e == 4 == "vlocb.bck".
    J3DAnmTransform* bck = (J3DAnmTransform*)dComIfG_getObjectRes("Key", "vlocb.bck");
    if (!mBckAnim.init(bck, TRUE, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, false))
        return FALSE;

    J3DSkinDeform* deform = new J3DSkinDeform();
    if (deform == NULL)
        return FALSE;

    switch (mpModel->setSkinDeform(deform, 1)) {
    case J3DErrType_OutOfMemory:
        return FALSE;
    default:
        // §328 donor JUT_ASSERT(DEMO_SELECT(771,774), FALSE) — retail no-op;
        // log + fall through to the donor Success return.
        DuskLog.warn("[Door] §328 keyCreate_Bkey: unexpected setSkinDeform result");
    case J3DErrType_Success:
        return TRUE;
    }
}

/* 8006C910-8006C948       .text keyCreate__12dDoor_key2_cFi */
BOOL dDoor_key2_c::keyCreate(int type) {
    mbIsBossDoor = type;
    switch (type) {
    case 1: return keyCreate_Bkey();
    default: return keyCreate_Nkey();
    }
}

/* 8006C948-8006C954       .text keyOn__12dDoor_key2_cFv */
void dDoor_key2_c::keyOn() {
    mbEnabled = true;
}

/* 8006C954-8006C960       .text keyOff__12dDoor_key2_cFv */
void dDoor_key2_c::keyOff() {
    mbEnabled = false;
}

/* 8006C960-8006CA10       .text calcMtx__12dDoor_key2_cFP12dDoor_info_c */
void dDoor_key2_c::calcMtx(dDoor_info_c* door) {
    if (mbEnabled) {
        mDoMtx_stack_c::transS(door->current.pos);
        mDoMtx_stack_c::YrotM(door->current.angle.y);
        switch (mbIsBossDoor) {
        case 1: mDoMtx_stack_c::transM(0.0f, 230.0f, 20.0f); break;
        default: mDoMtx_stack_c::transM(0.0f, 150.0f, 20.0f); break;
        }
        mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    }
}

/* 8006CA10-8006CA78       .text draw__12dDoor_key2_cFP12dDoor_info_c */
void dDoor_key2_c::draw(dDoor_info_c* door) {
    J3DModelData* modelData = mpModel->getModelData();
    dKyWw_setLightTevColorType(mpModel, &door->tevStr);
    mBckAnim.entry(modelData);
    mDoExt_modelUpdateDL(mpModel);
}

/* 8006CA78-8006CB28       .text calcMtx__12dDoor_stop_cFP12dDoor_info_c */
void dDoor_stop_c::calcMtx(dDoor_info_c* door) {
    if (m8 == 0 || mpModel == NULL)
        return;
    mDoMtx_stack_c::transS(door->current.pos.x, door->current.pos.y + mOffsY, door->current.pos.z);
    mDoMtx_stack_c::YrotM(door->current.angle.y);
    if (mFrontCheck == 1)
        mDoMtx_stack_c::YrotM(0x7FFF);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

/* 8006CB28-8006CBFC       .text closeInit__12dDoor_stop_cFP12dDoor_info_c */
void dDoor_stop_c::closeInit(dDoor_info_c* door) {
    mOffsY = 300.0f;
    fopAcM_SetSpeedF(door, 0.0f);
    if (door->getArg1() == 17) {
        fopAcM_seStart(door, JA_SE_OBJ_CAGE_CLOSE, 0);
    } else {
        fopAcM_seStart(door, JA_SE_OBJ_STN_DOOR_STL_BAR, 0);
    }
    mB = 1;
}

/* 8006CBFC-8006CC80       .text closeProc__12dDoor_stop_cFP12dDoor_info_c */
s32 dDoor_stop_c::closeProc(dDoor_info_c* door) {
    if (!mB)
        return 1;

    cLib_chaseF(&door->speedF, 60.0f, 6.0f);
    if (cLib_chaseF(&mOffsY, 0.0f, door->speedF)) {
        mB = FALSE;
        return 2;
    } else {
        return 0;
    }
}

/* 8006CC80-8006CD50       .text openInit__12dDoor_stop_cFP12dDoor_info_c */
void dDoor_stop_c::openInit(dDoor_info_c* door) {
    mOffsY = 0.0f;
    fopAcM_SetSpeedF(door, 0.0f);
    if (door->getArg1() == 17) {
        fopAcM_seStart(door, JA_SE_OBJ_CAGE_OPEN, 0);
    } else {
        fopAcM_seStart(door, JA_SE_OBJ_STN_DOOR_STL_BAR, 0);
    }
    mB = 1;
}

/* 8006CD50-8006CDD8       .text openProc__12dDoor_stop_cFP12dDoor_info_c */
s32 dDoor_stop_c::openProc(dDoor_info_c* door) {
    if (!mB)
        return 1;

    cLib_chaseF(&door->speedF, 30.0f, 4.0f);
    if (cLib_chaseF(&mOffsY, 300.0f, door->speedF)) {
        mB = FALSE;
        m8 = FALSE;
        return 2;
    } else {
        return 0;
    }
}

/* 8006CDD8-8006CE8C       .text create__12dDoor_stop_cFv */
BOOL dDoor_stop_c::create() {
    // §328 DN-3 note: on TP host stages no "stop10" member exists, so both
    // lookups return NULL and the donor NULL-tolerant path is taken (bars
    // simply have no model). If a hosted stage ever ships a stop10, this site
    // must be routed through the parse-at-consume resolver before use.
    J3DModelData* modelData = (J3DModelData*)dComIfG_getStageRes("Stage", "stop10.bmd");
    if (modelData == NULL)
        modelData = (J3DModelData*)dComIfG_getStageRes("Stage", "stop10.bdl");

    if (modelData != NULL) {
        mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
        if (mpModel == NULL)
            return FALSE;
    }

    return TRUE;
}

/* 8006CE8C-8006CEA8       .text init__11dDoor_msg_cFs */
void dDoor_msg_c::init(s16 msg_id) {
    mMsgId = msg_id;
    mMsgPId = fpcM_ERROR_PROCESS_ID_e;
    m_msg = NULL;
    mState = 0;
}

/* 8006CEA8-8006D0DC       .text proc__11dDoor_msg_cFP4cXyz */
BOOL dDoor_msg_c::proc(cXyz* pos) {
    (void)pos;  // §328 the port dropped the position overload (see below)
    switch (mState) {
    case 0:
        // §328 donor fopMsgM_messageSet(mMsgId, pos) — the port has no
        // (u32, cXyz*) overload; 2-arg (msgIdx, 1000) is the port-wide idiom
        // (d_npc.cpp §239 / d_event_data.cpp). Box placement is handled by the
        // port's native message flow.
        mMsgPId = fopMsgM_messageSet(mMsgId, 1000);
        if (mMsgPId != fpcM_ERROR_PROCESS_ID_e)
            mState++;
        break;
    case 1:
        m_msg = fopMsgM_SearchByID(mMsgPId);
        if (m_msg != NULL)
            mState++;
        break;
    case 2:
        // §328 donor JUT_ASSERT(DEMO_SELECT(951,954), m_msg) — retail no-op ->
        // NULL-guard (stay in state; the search above owns progression).
        if (m_msg == NULL)
            break;
        // §328 WW m_msg->mStatus == port m_msg->mode (fopMsg_MODE_* §245 map).
        if (m_msg->mode == fopMsgStts_MSG_TYPING_e) {
            switch (mMsgId) {
            case 0x1BBD:
            case 0x06A8:
                dComIfGp_getVibration().StartShock(7, -0x21, cXyz(0.0f, 1.0f, 0.0f));
                break;
            }
            mState++;
        }
        break;
    case 3:
        if (m_msg == NULL)  // §328 retail-assert NULL-guard (as above)
            break;
        if (m_msg->mode == fopMsgStts_MSG_DISPLAYED_e) {
            switch (mMsgId) {
            case 0x1BBD:
            case 0x1BC0:
            case 0x1BC1:
            case 0x1BC2:
                mMsgId++;
                m_msg->mode = fopMsgStts_MSG_CONTINUES_e;
                fopMsgM_messageSet(mMsgId, 1000);  // §328 port 2-arg (see above)
                break;
            default:
                mState++;
                m_msg->mode = fopMsgStts_MSG_ENDS_e;
                break;
            }
        }
        break;
    }

    if (m_msg != NULL && m_msg->mode == fopMsgStts_BOX_CLOSED_e) {
        m_msg->mode = fopMsgStts_MSG_DESTROYED_e;
        return TRUE;
    } else {
        return FALSE;
    }
}

/* 8006D0DC-8006D11C       .text resLoad__12dDoor_hkyo_cFv */
cPhs_Step dDoor_hkyo_c::resLoad() {  // §328 cPhs_State -> cPhs_Step
    if (m11 == 0) {
        return cPhs_COMPLEATE_e;
    }
    return dComIfG_resLoad(&mPhs, "Hkyo");
}

/* 8006D11C-8006D154       .text resDelete__12dDoor_hkyo_cFv */
void dDoor_hkyo_c::resDelete() {
    if (m11 == 0) {
        return;
    }
    dComIfG_resDelete(&mPhs, "Hkyo");
}

/* 8006D154-8006D2D4       .text create__12dDoor_hkyo_cFv */
BOOL dDoor_hkyo_c::create() {
    if (m11 == 0)
        return TRUE;

    // §328 DN-3: donor dRes_INDEX_HKYO_BDL_HKYO1_e == 4 == "hkyo1.bdl".
    // Donor JUT_ASSERT(DEMO_SELECT(1049,1052), modelData) -> NULL-guard FALSE.
    J3DModelData* modelData = dExtNpcMount_acquireModelData("Hkyo", "hkyo1.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Door] §328 hkyo create: 'Hkyo' member 'hkyo1.bdl' not resolvable");
        return FALSE;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000202);
    if (mpModel == NULL)
        return FALSE;

    mpBrkAnm = new mDoExt_brkAnm();
    if (mpBrkAnm == NULL)
        return FALSE;

    // §328 donor id dRes_INDEX_HKYO_BRK_HKYO1B_e == 8 == "hkyo1b.brk".
    J3DAnmTevRegKey* brk = (J3DAnmTevRegKey*)dComIfG_getObjectRes("Hkyo", "hkyo1b.brk");
    // §328 donor init(modelData, brk, TRUE, EMode_LOOP) -> port 7-arg.
    if (!mpBrkAnm->init(modelData, brk, TRUE, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1))
        return FALSE;

    return TRUE;
}

/* 8006D2D4-8006D3A8       .text setAnm__12dDoor_hkyo_cFUc */
void dDoor_hkyo_c::setAnm(u8 idx) {
    if (mAnmIdx == idx)
        return;

    mAnmIdx = idx;
    if (idx == 0)
        return;

    // §328 donor file-index switch (HKYO1A/B/C = 7/8/9) -> member names.
    const char* fileName;
    switch (idx) {
    case 1: fileName = "hkyo1a.brk"; break;
    case 2: fileName = "hkyo1b.brk"; break;
    default: fileName = "hkyo1c.brk"; break;
    }

    J3DModelData* modelData = mpModel->getModelData();
    J3DAnmTevRegKey* brk = (J3DAnmTevRegKey*)dComIfG_getObjectRes("Hkyo", fileName);
    // §328 donor 10-arg init(..., -1, true, 0) -> port 7-arg (trailing
    // modify/sound args have no port slots on mDoExt_brkAnm).
    mpBrkAnm->init(modelData, brk, TRUE, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1);
}

/* 8006D3A8-8006D3B4       .text init__12dDoor_hkyo_cFv */
void dDoor_hkyo_c::init() {
    mAnmIdx = 0;
}

/* 8006D3B4-8006D464       .text calcMtx__12dDoor_hkyo_cFP12dDoor_info_cf */
void dDoor_hkyo_c::calcMtx(dDoor_info_c* door, f32 offsY) {
    if (m11 == 0 || mpModel == NULL)
        return;
    mDoMtx_stack_c::transS(door->current.pos);
    mDoMtx_stack_c::YrotM(door->current.angle.y);
    mDoMtx_stack_c::transM(0.0f, offsY, 0.0f);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

/* 8006D464-8006D500       .text draw__12dDoor_hkyo_cFP12dDoor_info_c */
void dDoor_hkyo_c::draw(dDoor_info_c* door) {
    if (m11 == 0 || mAnmIdx == 0 || mpModel == NULL)
        return;
    if (!chkFirst()) {
        J3DModelData* modelData = mpModel->getModelData();
        dKyWw_setLightTevColorType(mpModel, &door->tevStr);
        mpBrkAnm->entry(modelData);
        mDoExt_modelUpdateDL(mpModel);
    }
}

/* 8006D500-8006D71C       .text proc__12dDoor_hkyo_cFP12dDoor_info_c */
void dDoor_hkyo_c::proc(dDoor_info_c* door) {
    if (m11 == 0 || mpModel == NULL)
        return;

    switch (m11) {
    case 1:
        // §328 WW TMP bits -> BRIDGE-OWED stub (see file banner). Donor:
        // dComIfGs_isTmpBit(dSv_event_tmp_flag_c::UNK_0108 / UNK_0110).
        if (dExtDoorWw_isTmpBit(WWTMP_UNK_0108))
            setAnm(1);
        else if (dExtDoorWw_isTmpBit(WWTMP_UNK_0110))
            setAnm(2);
        else
            setAnm(0);
        break;
    case 4:
        if (dComIfGs_isEventBit(WWEV_UNK_1710))  // §328 donor dSv_event_flag_c::UNK_1710 (routed)
            setAnm(0);
        else
            setAnm(3);
        break;
    case 3:
        if (!dComIfGs_isTact(2) || dComIfGs_isEventBit(WWEV_UNK_1704))
            setAnm(0);
        else
            setAnm(3);
        break;
    case 2:
        if (!dComIfGs_isEventBit(WWEV_UNK_1704) || dComIfGs_isEventBit(WWEV_UNK_1B01))
            setAnm(0);
        else
            setAnm(3);
        break;
    default:
        setAnm(0);
        break;
    }

    if (mAnmIdx != 0) {
        mpBrkAnm->play();
        fopAcM_seStart(door, JA_SE_OBJ_RES_DOOR_BLINK, 0);
    }
}

/* 8006D71C-8006D784       .text chkFirst__12dDoor_hkyo_cFv */
BOOL dDoor_hkyo_c::chkFirst() {
    if (m11 != 1)
        return FALSE;

    switch (mAnmIdx) {
    case 1:
        if (!dComIfGs_isEventBit(WWEV_UNK_2602))  // §328 donor UNK_2602 (routed)
            return TRUE;
        break;
    }

    return FALSE;
}

/* 8006D784-8006D7E8       .text onFirst__12dDoor_hkyo_cFv */
void dDoor_hkyo_c::onFirst() {
    switch (mAnmIdx) {
    case 1: dComIfGs_onEventBit(WWEV_UNK_2602); break;  // §328 donor UNK_2602 (routed)
    case 2: dComIfGs_onEventBit(WWEV_UNK_2601); break;  // §328 donor UNK_2601 (routed)
    }
}

/* 8006D7E8-8006D800       .text chkStart__12dDoor_hkyo_cFv */
BOOL dDoor_hkyo_c::chkStart() {
    if (daPy_getPlayerActorClass()->getGrabUpEnd()) {
        return TRUE;
    } else {
        return FALSE;
    }
}
