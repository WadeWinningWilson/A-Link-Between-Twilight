#include "d/dolzel.h" // IWYU pragma: keep

#include "d/d_event.h"
#include "d/actor/d_a_alink.h"
#include "d/d_demo.h"
#include "d/d_msg_object.h"
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_graphic.h"
#include "d/actor/d_a_midna.h"
#include "d/actor/d_a_tag_mhint.h"
#include "d/actor/d_a_tag_mstop.h"
#include "d/d_event_debug.h"
#include "SSystem/SComponent/c_counter.h"
#include <cstring>

#include "dusk/string.hpp"
#if TARGET_PC
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_save_guard.h"
#include "d/ext_plugin/ww_stage_loader.h"  // WW EVNT full-name resolver
#include "dusk/logging.h"
#endif

#if TARGET_PC
// ============================================================================
// §717 H1 (WAVE-1 pass-9 item 2): the §713c above-the-fork guard's reporter.
// The guard's silent -1/0 is the prime suspect for the 19:13 five-frame event
// death (door never opened) — if BASE_NULL is selected or the staff is not yet
// built while an event is LIVE, every cut no-ops with nothing in the log.
// First-8 one-shot with the full discriminator tuple; out-of-line so the
// header stays include-light. Strip with the §717 probe set.
// ============================================================================
void dEvtFork_guardReport(int i_staffId) {
    static int s_hits = 0;
    if (s_hits >= 8) {
        return;
    }
    s_hits++;
    dEvent_manager_c& mgr = dComIfGp_getEventManager();
    dEvDtBase_c& base = mgr.getBase();
    const char* runEvt = mgr.getRunEventName();
    DuskLog.warn(
        "[Evt] §717 H1 fork-guard hit #{}: staffId={} evt1Active={} evType={} evId={} "
        "evtRun={} runEvt='{}' staffP={} headerP={} gFrm={}",
        s_hits, i_staffId, JEvent1::evt1_isActive() ? 1 : 0, mgr.probeEvType(),
        (int)mgr.probeEvId(), dComIfGp_getEvent()->runCheck() ? 1 : 0,
        runEvt != NULL ? runEvt : "(none)", base.getStaffP() != NULL ? 1 : 0,
        base.getHeaderP() != NULL ? 1 : 0, (int)g_Counter.mCounter0);
}
#endif

namespace {
static u8 event_debug_evnt() {
#if DEBUG
    dEvM_HIO_c& hio = dComIfGp_getEventManager().getEventHIO();
    return hio.m_eve_debug;
#else
    return false;
#endif
}

static void clear_tmpflag_for_message() {
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[11]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[12]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[13]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[14]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[15]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[51]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[52]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[53]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[54]);
    dComIfGs_offTmpBit(dSv_event_tmp_flag_c::tempBitLabels[55]);
}
};  // namespace

dEvt_control_c::dEvt_control_c() {
#if TARGET_PC
    // §353 (the §345a-without-§352b resolution): this ctor runs remove() — and
    // the play-info is GLOBAL, so a scene CREATE re-initializing it executes
    // this on the LIVE object: status(pre) is the running event's state, and
    // the remove() kills it. Tag distinctly from wrapper calls; status(pre)==1
    // here == the tale assassin's fingerprint. Strip with §336.
    DuskLog.info("[Evt] §353 dEvt CTOR — status(pre)={} gFrm={}", (int)mEventStatus,
                 (int)g_Counter.mCounter0);
#endif
    remove();
}

dEvt_order_c::dEvt_order_c() {}

s32 dEvt_control_c::orderOld(u16 type, u16 prio, u16 flag, u16 hind, void* requestActor,
                             void* targetActor, const void* eventName) {
    #if DEBUG
    if (event_debug_evnt()) {
        if (type == dEvt_type_OTHER_e && eventName != NULL) {
            // "requested event %s!!"
            OS_REPORT("è¦æ±‚ã‚¤ãƒ™ãƒ³ãƒˆ %s!!", eventName);
        }
        OS_REPORT("\n");
    }
    #endif

    return order(type, prio, flag, hind, requestActor, targetActor, (s16)dComIfGp_getEventManager().getEventIdx((char*)eventName, -1, -1), 0xFF);
}

s32 dEvt_control_c::order(u16 type, u16 prio, u16 flag, u16 hind, void* requestActor,
                          void* targetActor, s16 id, u8 mapToolId) {
#if TARGET_PC
    // §345c (10-hyp refill; H-submitter identity): every change-family (0xE00)
    // order submitted while an event RUNS, with the requester named. Timestamps
    // vs §305 box beats (H9 clothes beat) and §324/§311 lines (H10 message
    // machinery). Strip with §336.
    if ((flag & 0xE00) && mEventStatus != 0) {
        DuskLog.info("[Evt] §345c order SUBMIT mid-event: reqProc={:#x} flag={:#x} type={} "
                     "prio={} id={} mapTool={:#x} runEvt='{}' status={}",
                     (int)(requestActor != NULL ? fopAcM_GetName((fopAc_ac_c*)requestActor)
                                                : -1),
                     (unsigned)flag, (int)type, (int)prio, (int)id, (int)mapToolId,
                     dComIfGp_getEventManager().getRunEventName(), (int)mEventStatus);
    }
#endif
    if (!(flag & 0x400) && mapToolId != 0xFF) {
        int roomNo = dComIfGp_roomControl_getStayNo();
        dStage_MapEvent_dt_c* data = searchMapEventData(mapToolId, roomNo);

        if (data != NULL && data->switch_no != 0xFF) {
            if (dComIfGs_isSwitch(data->switch_no, roomNo)) {
                return 0;
            }
        }
    }

    #if DEBUG
    if (event_debug_evnt()) {
        OS_REPORT("%06d: event: type = %d !!", g_Counter.mCounter0, type);

        if (requestActor != NULL) {
            // "Request actor %s!!"
            OS_REPORT("è¦æ±‚è€… %s!!", dStage_getName(fopAcM_GetProfName(requestActor), -1));
        }

        if (targetActor != NULL) {
            // "Target actor %s!!"
            OS_REPORT("å¯¾è±¡è€… %s!!", dStage_getName(fopAcM_GetProfName(targetActor), -1));
        }

        // "Request id = %d prio = %d "
        OS_REPORT(" è¦æ±‚ id = %d prio = %d ", id, prio);
        OS_REPORT("\n");
    }
    #endif

    if (mNum >= ARRAY_SIZE(mOrder)) {
        // "%06d: event: order failed"
        OS_REPORT("\x1b[34m%06d: event: æ³¨æ–‡å¤±æ•—\n\x1b[m", g_Counter.mCounter0);
        return 0;
    }

    dEvt_order_c* new_order = &mOrder[mNum];
    new_order->mEventType = type;
    new_order->mPriority = prio;
    new_order->mFlag = flag;
    new_order->mpRequestActor = (fopAc_ac_c*)requestActor;
    new_order->mpTargetActor = (fopAc_ac_c*)targetActor;
    new_order->mEventId = id;
    new_order->mHindFlag = hind;
    new_order->mMapToolId = mapToolId;

    if (new_order->mPriority == 0) {
        new_order->mPriority = 1;
    }

    if (mNum == 0) {
        mOrderIdx = 0;
        new_order->mNextOrderIdx = -1;
    } else {
        dEvt_order_c* curOrder;
        int curOrderIdx = mOrderIdx;
        curOrder = &mOrder[curOrderIdx];

        if (new_order->mPriority < curOrder->mPriority) {
            mOrderIdx = mNum;
            new_order->mNextOrderIdx = curOrderIdx;
        } else {
            while (curOrder->mNextOrderIdx >= 0) {
                dEvt_order_c* next_order = &mOrder[curOrder->mNextOrderIdx];
                if (new_order->mPriority < next_order->mPriority) {
                    break;
                }

                int nextOrderIdx = curOrder->mNextOrderIdx;
                curOrder = &mOrder[nextOrderIdx];
            }

            new_order->mNextOrderIdx = curOrder->mNextOrderIdx;
            curOrder->mNextOrderIdx = mNum;
        }
    }

    mNum++;

    #if DEBUG
    if (event_debug_evnt()) {
        // "%06d: event: order success  mNum = %d"
        OS_REPORT("\x1B[34m%06d: event: æ³¨æ–‡æˆåŠŸã€€mNum = %d\n\x1B[m", g_Counter.mCounter0, mNum);
    }
    #endif

    return 1;
}

void dEvt_control_c::setParam(dEvt_order_c* order) {
    setPt1(order->mpRequestActor);
    setPt2(order->mpTargetActor);

    mEventId = order->mEventId;
    mHindFlag = order->mHindFlag;

    if (dComIfGp_getPlayer(0) != order->mpRequestActor) {
        setPtT(order->mpRequestActor);
        setPtI(order->mpRequestActor);
    } else {
        setPtT(order->mpTargetActor);
        setPtI(order->mpTargetActor);
    }

    mMapToolId = order->mMapToolId;
    field_0xea = 0xFF;
    field_0xec = 0xFF;

    int roomNo = dComIfGp_roomControl_getStayNo();
    dStage_MapEvent_dt_c* mapdata = searchMapEventData(mMapToolId, roomNo);
    mStageEventDt = mapdata;

    if (mapdata != NULL) {
        field_0xea = mapdata->data.maptool.field_0x16;

        if (mapdata->switch_no != 0xFF) {
            dComIfGs_onSwitch(mapdata->switch_no, roomNo);
        }

        if (order->mFlag & 0x300) {
            field_0xec = mapdata->field_0x5;
        }
    }

    if (!(order->mFlag & 0x200)) {
        mUnkEventId = order->mEventId;
    }

    mCullRate = 1.0f;
    mEventFlag = 0;
}

s32 dEvt_control_c::beforeFlagProc(dEvt_order_c* order) {
    fopAc_ac_c* actor = order->mpTargetActor;

    if ((order->mFlag & 4) && !actor->eventInfo.chkCondition(dEvtCnd_CANTALK_e)) {
        return 0;
    }

    return 1;
}

static char const* dummy1() {
    return "ï¼Ÿï¼Ÿï¼Ÿ";
}

void dEvt_control_c::afterFlagProc(dEvt_order_c* order) {
    if (order->mFlag & 2) {
        dComIfGp_getEventManager().issueStaff("ALL");
    }
}

int dEvt_control_c::commonCheck(dEvt_order_c* order, u16 condition, u16 command) {
    fopAc_ac_c* actor1 = order->mpRequestActor;
    fopAc_ac_c* actor2 = order->mpTargetActor;

    if ((actor1 != NULL && actor1->eventInfo.chkCondition(condition)) &&
        (actor2 != NULL && actor2->eventInfo.chkCondition(condition))) {
        actor1->eventInfo.setCommand(command);
        actor2->eventInfo.setCommand(command);
        setParam(order);
        return 1;
    }

    return 0;
}

int dEvt_control_c::talkCheck(dEvt_order_c* order) {
    DUSK_CONST char* eventname = "DEFAULT_TALK";
    fopAc_ac_c* actor = order->mpTargetActor;
    if ((fopAcM_GetName(actor) == fpcNm_Tag_Mhint_e && ((daTagMhint_c*)actor)->checkNoAttention()) ||
        (fopAcM_GetName(actor) == fpcNm_Tag_Mstop_e && ((daTagMstop_c*)actor)->checkNoAttention()) ||
        fopAcM_GetName(actor) == fpcNm_MIDNA_e)
    {
        daMidna_c* midna = (daMidna_c*)daPy_py_c::getMidnaActor();
        if (!daPy_py_c::checkNowWolf() || midna->checkNoDraw()) {
            eventname = "MHINT_TALK";
        }
    }

    if (commonCheck(order, dEvtCnd_CANTALK_e, dEvtCmd_INTALK_e)) {
        #if DEBUG
        if (event_debug_evnt()) {
            // "conversation  actor1 = %x  actor2 = %x"
            OS_REPORT("ä¼šè©±ã¸ actor1 = %x  actor2 = %x\n", order->mpRequestActor, order->mpTargetActor);
            // "conversation partner %s"
            OS_REPORT("ä¼šè©±ç›¸æ‰‹ %s\n", dStage_getName(fopAcM_GetProfName(actor), -1));
        }
        #endif

        mMode = dEvt_mode_TALK_e;

        if (mEventId == -1) {
            if (actor != NULL && actor->eventInfo.getEventName() != NULL) {
                mEventId = actor->eventInfo.getEventId();
            } else {
                mEventId = dComIfGp_getEventManager().getEventIdx(eventname, 0xFF, -1);
            }
        }

        if (!dComIfGp_getEventManager().order(mEventId)) {
            // "specified event %d not ordered!!"
            OS_REPORT("æŒ‡å®šã‚¤ãƒ™ãƒ³ãƒˆ %d ãŒç„¡ã„!!\n", mEventId);
            JUT_ASSERT(429, FALSE);
        }

        return 1;
    }

    return 0;
}

int dEvt_control_c::talkXyCheck(dEvt_order_c* order) {
    const char* eventname = "DEFAULT_TALK_XY";
    fopAc_ac_c* targetActor = order->mpTargetActor;

    int itemIndex;
    switch (order->mEventType) {
    case dEvt_type_SHOWITEM_X_e:
        itemIndex = SELECT_ITEM_X;
        mTalkXyType = 1;
        break;
    case dEvt_type_SHOWITEM_Y_e:
        mTalkXyType = 2;
        itemIndex = SELECT_ITEM_Y;
        break;
    #if PLATFORM_WII || PLATFORM_SHIELD
    case 8:
        mTalkXyType = 3;
        itemIndex = 2;
        break;
    case 9:
        mTalkXyType = 4;
        itemIndex = 3;
        break;
    #endif
    default:
        OS_REPORT_ERROR("event XY talk unknown type %d\n", order->mEventType);
        mTalkXyType = 0;
        return 0;
    }

    if (dComIfGp_getSelectItem(itemIndex) == dItemNo_NONE_e) {
        return 0;
    }

    if (targetActor == NULL || !targetActor->eventInfo.chkCondition(dEvtCnd_CANTALKITEM_e)) {
        return 0;
    }

    if (commonCheck(order, dEvtCnd_CANTALK_e, dEvtCmd_INTALK_e)) {
        #if DEBUG
        if (event_debug_evnt()) {
            // "conversation  actor1 = %x  actor2 = %x"
            OS_REPORT("ä¼šè©±ã¸ actor1 = %x  actor2 = %x\n", order->mpRequestActor, order->mpTargetActor);
            // "conversation partner"
            OS_REPORT("ä¼šè©±ç›¸æ‰‹ %s\n", dStage_getName(fopAcM_GetProfName(targetActor), -1));
        }
        #endif

        mMode = dEvt_mode_TALK_e;
        mPreItemNo = dComIfGp_getSelectItem(itemIndex);
        mEventId = dComIfGp_getEventManager().getEventIdx(eventname, 0xFF, -1);
        
        if (!dComIfGp_getEventManager().order(mEventId)) {
            // "specified event %d not ordered!!"
            OS_REPORT("æŒ‡å®šã‚¤ãƒ™ãƒ³ãƒˆ %d ãŒç„¡ã„!!\n", mEventId);
            JUT_ASSERT(512, FALSE);
        }
        return 1;
    }

    return 0;
}

int dEvt_control_c::catchCheck(dEvt_order_c* order) {
    fopAc_ac_c* actor2 = order->mpTargetActor;
    fopAc_ac_c* actor1 = order->mpRequestActor;

    if (actor1 == NULL || (actor2 != NULL && !actor2->eventInfo.chkCondition(dEvtCnd_40_e))) {
        return 0;
    }

    actor1->eventInfo.setCommand(dEvtCmd_INCATCH_e);
    if (actor2 != NULL) {
        actor2->eventInfo.setCommand(dEvtCmd_INCATCH_e);
    }

    setParam(order);
    mPreItemNo = dComIfGp_att_getCatchChgItem();
    mMode = dEvt_mode_DEMO_e;

    if (mEventId != -1) {
        if (!dComIfGp_getEventManager().order(mEventId)) {
            // "specified event %d not ordered!!"
            OS_REPORT("æŒ‡å®šã‚¤ãƒ™ãƒ³ãƒˆ %d ãŒç„¡ã„!!\n", mEventId);
            JUT_ASSERT(551, FALSE);
        }
    }

    return 1;
}

int dEvt_control_c::talkEnd() {
    fopAc_ac_c* actor = getPt1();
    if (actor != NULL) {
        actor->eventInfo.setCommand(dEvtCmd_NONE_e);
    }

    actor = getPt2();
    if (actor != NULL) {
        actor->eventInfo.setCommand(dEvtCmd_NONE_e);
    }

    if (mEventId != -1) {
        dComIfGp_getEventManager().endProc(mEventId, TRUE);
        mEventId = -1;
    }

    daItemBase_c* item = (daItemBase_c*)fopAcM_getItemEventPartner(NULL);
    if (item != NULL && fopAcM_GetName(item) == fpcNm_ITEM_e) {
        item->dead();
    }

    return 1;
}

int dEvt_control_c::demoCheck(dEvt_order_c* order) {
    fopAc_ac_c* actor1 = order->mpRequestActor;
    fopAc_ac_c* actor2 = order->mpTargetActor;
    int eventId = order->mEventId;

    if (actor2 == NULL) {
        JUT_ASSERT(608, FALSE);
        return 0;
    }

    if (!(order->mFlag & 0x10)) {
        if (actor1 != NULL && !actor1->eventInfo.chkCondition(dEvtCnd_CANDEMO_e)) {
            return 0;
        }

        if (actor2 != NULL && !actor2->eventInfo.chkCondition(dEvtCnd_CANDEMO_e)) {
            return 0;
        }
    }

    if (!beforeFlagProc(order)) {
        return 0;
    }

    if (actor1 != NULL) {
        dComIfGp_getEventManager().setObjectArchive(actor1->eventInfo.mArchiveName);
    }

    if (!dComIfGp_getEventManager().order(eventId)) {
        return 0;
    }

    if (actor1 != NULL) {
        actor1->eventInfo.setCommand(dEvtCmd_INDEMO_e);
    }

    if (actor2 != NULL) {
        actor2->eventInfo.setCommand(dEvtCmd_INDEMO_e);
    }

    mMode = dEvt_mode_DEMO_e;
    setParam(order);
    afterFlagProc(order);
    return 1;
}

int dEvt_control_c::demoEnd() {
    fopAc_ac_c* actor = getPt1();
    if (actor != NULL) {
        actor->eventInfo.setCommand(dEvtCmd_NONE_e);
    }

    actor = getPt2();
    if (actor != NULL) {
        actor->eventInfo.setCommand(dEvtCmd_NONE_e);
    }

    if (mEventId != -1) {
        dComIfGp_getEventManager().endProc(mEventId, TRUE);
        mEventId = -1;
    }

    return 1;
}

int dEvt_control_c::potentialCheck(dEvt_order_c* order) {
    fopAc_ac_c* actor1 = order->mpRequestActor;
    fopAc_ac_c* actor2 = order->mpTargetActor;
    const char* actorname;

    #if DEBUG
    if (actor1 == NULL || actor2 == NULL) {
        JUT_ASSERT(704, FALSE);
    }
    #endif

    if (!beforeFlagProc(order)) {
        return 0;
    }

    actor1->eventInfo.setCommand(dEvtCmd_INDEMO_e);
    mMode = dEvt_mode_DEMO_e;
    setParam(order);
    afterFlagProc(order);

    #if DEBUG
    if (event_debug_evnt()) {
        if (actor1 != NULL) {
            actorname = dStage_getName(fopAcM_GetProfName(actor1), -1);
        } else {
            actorname = "???";
        }

        // "%06d: event: potential demo start (%s) "
        OS_REPORT("\x1b[34m%06d: event: ä»®æƒ³ãƒ‡ãƒ¢é–‹å§‹ (%s) \n\x1b[m", g_Counter.mCounter0, actorname);
    }
    #endif
    return 1;
}

int dEvt_control_c::doorCheck(dEvt_order_c* order) {
    // "door demo check start!!"
    OS_REPORT("ãƒ‰ã‚¢ãƒ‡ãƒ¢ã®ãƒã‚§ãƒƒã‚¯é–‹å§‹!!\n");

    if (commonCheck(order, dEvtCnd_CANDOOR_e, dEvtCmd_INDOOR_e)) {
        #if DEBUG
        if (event_debug_evnt()) {
            // "door demo  actor1 = %x  actor2 = %x"
            OS_REPORT("ãƒ‰ã‚¢ãƒ‡ãƒ¢ã¸ actor1 = %x  actor2 = %x\n", order->mpRequestActor, order->mpTargetActor);
        }
        #endif

        mMode = dEvt_mode_DEMO_e;

        fopAc_ac_c* actor2 = getPt2();
        if (actor2 != NULL) {
            dComIfGp_getEventManager().setObjectArchive(actor2->eventInfo.mArchiveName);
        }

        if (mEventId == -1 && actor2 != NULL && actor2->eventInfo.getEventId() != -1) {
            mEventId = actor2->eventInfo.getEventId();
        }

        if (mEventId != -1 && dComIfGp_getEventManager().getEventData(mEventId) != NULL) {
            if (!dComIfGp_getEventManager().order(mEventId)) {
                JUT_ASSERT(773, FALSE);
            }
        } else {
#if TARGET_PC
            // №53: WW mount knobs have no stage event archive — fire pinned-BG warp backend.
            if (actor2 != NULL && dExtNpcDoors_tryNativeWarp(actor2)) {
                mEventId = -1;
                // №89: stage-change arms next stage; endProc is blocked while
                // isEnableNextStage, so a source DEMO event never finishes.
                // Abort door-event entry — destination owns the arrival demo.
                if (dComIfGp_isEnableNextStage()) {
                    mMode = dEvt_mode_WAIT_e;
                    fopAc_ac_c* a1 = getPt1();
                    if (a1 != NULL) {
                        a1->eventInfo.setCommand(dEvtCmd_NONE_e);
                    }
                    fopAc_ac_c* a2 = getPt2();
                    if (a2 != NULL) {
                        a2->eventInfo.setCommand(dEvtCmd_NONE_e);
                    }
                    DuskLog.info(
                        "[Doors] №89 doorCheck — stage-change warp; skip source DEMO event");
                    return 0;
                }
                reset();
                return 1;
            }
#endif
            mEventId = -1;
            reset();
        }

        if (order->mEventType == dEvt_type_TREASURE_e) {
            onEventFlag(4);
        } else {
            onEventFlag(0x40);
        }

        return 1;
    }

    return 0;
}

int dEvt_control_c::itemCheck(dEvt_order_c* order) {
    const char* eventname = "DEFAULT_GETITEM";

    if (commonCheck(order, dEvtCnd_CANGETITEM_e, dEvtCmd_INGETITEM_e)) {
        #if DEBUG
        if (event_debug_evnt()) {
            OS_REPORT("ã‚¢ã‚¤ãƒ†ãƒ ãƒ‡ãƒ¢ã¸ actor1 = %x  actor2 = %x\n", order->mpRequestActor, order->mpTargetActor);
        }
        #endif

        mMode = dEvt_mode_DEMO_e;
        mEventId = dComIfGp_getEventManager().getEventIdx(eventname, 0xFF, -1);
        if (!dComIfGp_getEventManager().order(mEventId)) {
            // "specified event %d not ordered!!"
            OS_REPORT("æŒ‡å®šã‚¤ãƒ™ãƒ³ãƒˆ %s ãŒç„¡ã„!!\n", eventname);
            JUT_ASSERT(817, FALSE);
        }
        return 1;
    }

    return 0;
}

int dEvt_control_c::endProc() {
    #if DEBUG
    if (event_debug_evnt()) {
        OS_REPORT("\x1b[34m%06d: event: ã‚¤ãƒ™ãƒ³ãƒˆãŒçµ‚äº†ã—ã¾ã—ãŸ mMode = %d \n\x1b[m", g_Counter.mCounter0, mMode);
    }
    #endif

    switch (mMode) {
    case dEvt_mode_TALK_e:
        talkEnd();
        break;
    case dEvt_mode_DEMO_e:
        demoEnd();
        break;
    case dEvt_mode_COMPULSORY_e:
        JUT_ASSERT(858, FALSE);
    }

    mMode = dEvt_mode_WAIT_e;
    field_0xea = 0xFF;
    mMapToolId = 0xFF;
    mStageEventDt = NULL;
    field_0xec = 0xFF;
    mTalkXyType = 0;
    mUnkEventId = 255;
    mPreItemNo = dItemNo_NONE_e;
    dComIfGp_getEventManager().setStartDemo(-2);
    return 1;
}

int dEvt_control_c::change() {
    dEvt_order_c* order;

    if (mNum != 0) {
        s8 curOrderIdx = mOrderIdx;

        do {
            order = &mOrder[curOrderIdx];

            if ((order->mFlag & 0xE00) && order->mpRequestActor == mChangeActor) {
#if TARGET_PC
                // §344 (the §341a killer): change() is the ONLY direct 1→0 path —
                // an event-CHANGE order granted mid-tale killed TALE_DEMO at
                // fnm=98 (log 22:28). Name the requester. Strip with §336.
                DuskLog.info("[Evt] §344 change() GRANT — reqProc={:#x} flag={:#x} evType={} "
                             "(mid-run runEvt='{}')",
                             (int)(order->mpRequestActor != NULL
                                       ? fopAcM_GetName(order->mpRequestActor) : -1),
                             (unsigned)order->mFlag, (int)order->mEventType,
                             dComIfGp_getEventManager().getRunEventName());
#endif
                mOrderIdx = curOrderIdx;
                mNum = 1;
                order->mNextOrderIdx = -1;
                return curOrderIdx;
            }

            curOrderIdx = order->mNextOrderIdx;
        } while (curOrderIdx >= 0);
    }

    return -1;
}

int dEvt_control_c::entry() {
    if (mNum != 0) {
        // "%06d: event: %d event requests!!"
        OS_REPORT("\x1b[34m%06d: event: %då€‹ã®ã‚¤ãƒ™ãƒ³ãƒˆè¦æ±‚!!\n\x1b[m", g_Counter.mCounter0, mNum);

        s8 orderIdx = mOrderIdx;
        mNum = 0;

        do {
            dEvt_order_c* order = &mOrder[orderIdx];
            orderIdx = order->mNextOrderIdx;

            switch (order->mEventType) {
            case dEvt_type_TALK_e:
                if (dComIfGs_getLife() != 0 && talkCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_OTHER_e:
                if (dComIfGs_getLife() != 0 && demoCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_DOOR_e:
            case dEvt_type_TREASURE_e:
                if (dComIfGs_getLife() != 0 && doorCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_COMPULSORY_e:
                mMode = dEvt_mode_COMPULSORY_e;

                #if DEBUG
                if (event_debug_evnt()) {
                    // "%06d: event: player initiated demo start"
                    OS_REPORT("\x1b[34m%06d: event: ãƒ—ãƒ¬ãƒ¼ãƒ¤ä¸»å°Žãƒ‡ãƒ¢é–‹å§‹\n\x1b[m", g_Counter.mCounter0);
                }
                #endif

                setParam(order);
                return 1;
            case dEvt_type_POTENTIAL_e:
                if (dComIfGs_getLife() != 0 && potentialCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_ITEM_e:
                if (dComIfGs_getLife() != 0 && itemCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_SHOWITEM_X_e:
            case dEvt_type_SHOWITEM_Y_e:
                if (dComIfGs_getLife() != 0 && talkXyCheck(order)) {
                    return 1;
                }
                break;
            case dEvt_type_CATCH_e:
                if (dComIfGs_getLife() != 0 && catchCheck(order)) {
                    return 1;
                }
                break;
            case 12:
                JUT_ASSERT(980, FALSE);
                break;
            case 13:
                break;
            }
        } while (orderIdx >= 0);

        #if DEBUG
        if (event_debug_evnt()) {
            // "%06d: event: check failed or change"
            OS_REPORT("\x1b[34m%06d: event: ãƒã‚§ãƒƒã‚¯å¤±æ•— or ãƒã‚§ãƒ³ã‚¸\n\x1b[m", g_Counter.mCounter0);
        }
        #endif
    }

    return 0;
}

void dEvt_control_c::reset() {
    if (field_0xec != 0xFF) {
        fopAc_ac_c* pt1 = getPt1();
        fopAc_ac_c* var_r27 = pt1;
        s16 eventIdx = dComIfGp_getEventManager().getEventIdx(NULL, field_0xec, -1);

        order(dEvt_type_OTHER_e, 3, 0x201, mHindFlag, pt1, getPt2(), eventIdx, field_0xec);
        mChangeActor = pt1;
#if TARGET_PC
        // §345b (H4/H8): reset() queues the RE-ENTRANCE change order (0x201) and
        // arms mChangeActor whenever the map-event carries a re-entry idx —
        // prime suspect for the mid-tale change grant. Strip with §336.
        DuskLog.info("[Evt] §345b reset(): re-entrance order 0x201 queued, fld_ec={:#x} "
                     "changeActor=proc:{:#x} evIdx={}",
                     (int)field_0xec,
                     (int)(pt1 != NULL ? fopAcM_GetName(pt1) : -1), (int)eventIdx);
#endif
    }

    onEventFlag(8);
}

void dEvt_control_c::reset(void* param_0) {
    if (field_0xec != 0xFF) {
        fopAc_ac_c* var_r29 = (fopAc_ac_c*)param_0;
        s16 eventIdx = dComIfGp_getEventManager().getEventIdx(var_r29, field_0xec);
        order(dEvt_type_OTHER_e, 3, 0x201, mHindFlag, param_0, getPt2(), eventIdx, field_0xec);
    }

    mChangeActor = param_0;
#if TARGET_PC
    // §345b (H4/H8) — the reset(void*) overload's setter.
    DuskLog.info("[Evt] §345b reset(actor): changeActor=proc:{:#x} fld_ec={:#x}",
                 (int)(param_0 != NULL ? fopAcM_GetName((fopAc_ac_c*)param_0) : -1),
                 (int)field_0xec);
#endif
    onEventFlag(8);
}

void dEvt_control_c::clearSkipSystem() {
    mSkipFunc = NULL;
    mSkipActorId = fpcM_ERROR_PROCESS_ID_e;
    mSkipParameter = 0;
    mSkipTimer = 0;

    if (!chkFlag2(3)) {
        mIsSkipFade = FALSE;
    }

    mSkipEventName[0] = 0;
}

int dEv_defaultSkipProc(void* actor, int parameter) {
    // "%06d: event: skip event!!!"
    OS_REPORT("\x1b[34m%06d: event: ã‚¹ã‚­ãƒƒãƒ— event!!!\n\x1b[m", g_Counter.mCounter0);
    dEvt_control_c* event = dComIfGp_getEvent();

    event->reset(actor);
    return 1;
}

int dEv_defaultSkipZev(void* actor, int parameter) {
    // "%06d: event: skip ZEV event!!!"
    OS_REPORT("\x1b[34m%06d: event: ã‚¹ã‚­ãƒƒãƒ— ZEV event!!!\n\x1b[m", g_Counter.mCounter0);
    dEvt_control_c* event = dComIfGp_getEvent();

    s16 eventID = -1;
    dStage_MapEvent_dt_c* data = event->getStageEventDt();

    bool var_r27 = true;
    if (data != NULL) {
        if (data->field_0x9 != 0xFF) {
            var_r27 = false;
        } else if (data->field_0x7 != 0xFF) {
            var_r27 = false;
        }
    }

    char eventName[28];
    char* skipName;
#if TARGET_PC
    // WW: these build a suffixed name and LOOK IT UP, so they need the donor's
    // FULL name — a 12-char truncation matches nothing in the event list.
    // Pointer identity (never prefix: FROM_HYRULE_1/_2 share 12 chars); NULL
    // leaves the receiver's stored name exactly as before.
    const char* wwFullEvt =
        (data != NULL) ? dExtWwEvt_getStageEventNameForRecord(data) : NULL;
#endif
    switch (parameter) {
    case 0:
#if TARGET_PC
        if (wwFullEvt != NULL) { SAFE_STRCPY(eventName, wwFullEvt); } else
#endif
        SAFE_STRCPY(eventName, data->data.event_name);
        SAFE_STRCAT(eventName, "$0");
        eventID = dComIfGp_getEventManager().getEventIdx(eventName, 0xFF, -1);
        OS_REPORT("%06d: event:   [%d] %s!\n", g_Counter.mCounter0, eventID, eventName);
        break;
    case 1:
        skipName = dComIfGp_getEvent()->getSkipEventName();
        eventID = dComIfGp_getEventManager().getEventIdx(skipName, 0xFF, -1);
        OS_REPORT("%06d: event:   [%d] %s!\n", g_Counter.mCounter0, eventID, dComIfGp_getEvent()->getSkipEventName());
        break;
    default:
        OS_REPORT("%06d: event: ERROR: illegal prameter!\n", g_Counter.mCounter0);
    }

    if (eventID != -1) {
        event->reset(actor);
        fopAcM_orderOtherEventId((fopAc_ac_c*)actor, eventID, event->getMapToolId(), -1, 3,
                                 0x400);
        return 2;
    } else if (var_r27) {
        event->reset(actor);
        return 1;
    } else {
        event->reset(actor);
        return 1;
    }
}

int dEv_defaultSkipStb(void* actor, int parameter) {
    OS_REPORT("\x1b[34m%06d: event: ã‚¹ã‚­ãƒƒãƒ— STB event!!!\n\x1b[m", g_Counter.mCounter0);
    dEvt_control_c* event = dComIfGp_getEvent();

    s16 eventID = -1;
    dStage_MapEvent_dt_c* data = event->getStageEventDt();

    bool var_r27 = true;
    if (data != NULL) {
        if (data->field_0x9 != 0xFF) {
            var_r27 = false;
        } else if (data->field_0x7 != 0xFF) {
            var_r27 = false;
        }
    }

    char eventName[28];
    char* skipName;
#if TARGET_PC
    // WW: these build a suffixed name and LOOK IT UP, so they need the donor's
    // FULL name — a 12-char truncation matches nothing in the event list.
    // Pointer identity (never prefix: FROM_HYRULE_1/_2 share 12 chars); NULL
    // leaves the receiver's stored name exactly as before.
    const char* wwFullEvt =
        (data != NULL) ? dExtWwEvt_getStageEventNameForRecord(data) : NULL;
#endif
    switch (parameter) {
    case 0:
#if TARGET_PC
        if (wwFullEvt != NULL) { SAFE_STRCPY(eventName, wwFullEvt); } else
#endif
        SAFE_STRCPY(eventName, data->data.event_name);
        SAFE_STRCAT(eventName, "$0");
        eventID = dComIfGp_getEventManager().getEventIdx(eventName, 0xFF, -1);
        OS_REPORT("%06d: event:   [%d] %s!\n", g_Counter.mCounter0, eventID, eventName);
        break;
    case 1:
        skipName = dComIfGp_getEvent()->getSkipEventName();
        eventID = dComIfGp_getEventManager().getEventIdx(skipName, 0xFF, -1);
        OS_REPORT("%06d: event:   [%d] %s!\n", g_Counter.mCounter0, eventID, dComIfGp_getEvent()->getSkipEventName());
        break;
    default:
        OS_REPORT("%06d: event: ERROR: illegal prameter!\n", g_Counter.mCounter0);
    }

    if (eventID != -1) {
        event->reset(actor);
        dDemo_c::end();
        fopAcM_orderOtherEventId((fopAc_ac_c*)actor, eventID, event->getMapToolId(), -1, 3,
                                 0x400);
        return 2;
    } else if (var_r27) {
        event->reset(actor);
        dDemo_c::end();
        return 1;
    } else {
        event->reset(actor);
        return 1;
    }
}

int dEv_noFinishSkipProc(void* actor, int parameter) {
    dComIfGp_getEvent()->offSkipFade();
    return 0;
}

char* dEvt_control_c::getSkipEventName() {
    return mSkipEventName;
}

void dEvt_control_c::setSkipProc(void* skipActor, dEvt_SkipCb skipCb, int skipParameter) {
    mSkipFunc = skipCb;
    mSkipActorId = getPId(skipActor);
    mSkipParameter = skipParameter;
}

void dEvt_control_c::setSkipZev(void* skipActor, DUSK_CONST char* eventName) {
    setSkipProc(skipActor, dEv_defaultSkipZev, 1);
    SAFE_STRCPY(mSkipEventName, eventName);
}

void dEvt_control_c::onSkipFade() {
    mIsSkipFade = TRUE;
}

void dEvt_control_c::offSkipFade() {
    if (mIsSkipFade) {
        mIsSkipFade = FALSE;
    }

    mIsSkipFade = FALSE;
}

bool dEvt_control_c::skipper() {
    bool doSkip = false;
    bool canSkip = false;

    offFlag2(8);

    if (mEventStatus == 1) {
        if (mSkipFunc != NULL) {
            canSkip = true;
        }

        bool is_trig_skipbtn = mDoCPd_c::getTrigStart(PAD_1);
        if (is_trig_skipbtn) {
            if (mSkipTimer > 0) {
                mSkipTimer = -1;

                if (canSkip && mIsSkipFade) {
                    mDoGph_gInf_c::fadeOut(0.1f);
                }
            } else if (mSkipTimer == 0) {
                mSkipTimer = 1;
            }
        }

        if (mSkipTimer > 0) {
            if (canSkip) {
                dComIfGp_setSButtonStatusForce(0x43, 1);
            } else {
                dComIfGp_setSButtonStatusForce(0x4D, 1);
            }

            if (mSkipTimer++ > 45) {
                mSkipTimer = 0;
            }
        } else if (mSkipTimer != 0) {
            if (canSkip && mIsSkipFade) {
                if (mSkipTimer-- < -20) {
                    doSkip = true;
                    mSkipTimer = 0;
                }
            } else {
                if (canSkip) {
                    doSkip = true;
                }
                mSkipTimer = 0;
            }
        }

        if (doSkip) {
            dMsgObject_onKillMessageFlag();

            fopAc_ac_c* skipActor = convPId(mSkipActorId);
            if (skipActor == NULL) {
                OS_REPORT("\x1b[31m%06d: event: Skip ordered actor DEAD!! (%d) \n\x1b[m", g_Counter.mCounter0, mSkipActorId);
                skipActor = dComIfGp_getPlayer(0);
            }

            int skipRet = mSkipFunc(skipActor, mSkipParameter);
            onFlag2(8);

            if (skipRet != 0) {
                mSkipFunc = NULL;

                if (skipRet == 2) {
                    onFlag2(1);
                } else {
                    onFlag2(2);
                }
            }
        }
    }

    return doSkip;
}

int dEvt_control_c::Step() {
    dEvent_manager_c* evtMng = &dComIfGp_getEventManager();

    #if DEBUG
    static int prevEvId = -99;
    static char* basename[] = {
        "NULL",
        "KEEP",
        "STAGE",
        "ROOM0",
        "ROOM1",
        "ROOM2",
        "ROOM3",
        "ROOM4",
        "ROOM5",
        "DEMO",
        "DEBUG",
    };

    evtMng->debugBfProc();
    #endif

    field_0xe7 = 0;
    field_0xe8 = mNum;

    skipper();

    if (chkEventFlag(8)) {
        if (mEventStatus == 1) {
            mEventStatus = 5;

            if (dMsgObject_getMsgObjectClass() != NULL) {
                dMsgObject_endFlowGroup();
            }
        }

        dEvDtEvent_c* eventDt = evtMng->getEventData(mEventId);
        if (eventDt != NULL) {
            eventDt->forceFinish();
        }

        offEventFlag(8);
    }

    if (mEventStatus != 0) {
        evtMng->Sequencer();
    }

    offEventFlag(0x300);

    if (mEventStatus == 2) {
        // "%06d: event: end (%d)"
        OS_REPORT("\x1b[36m%06d: event: çµ‚äº† (%d)\n\x1b[m", g_Counter.mCounter0, prevEvId);

        evtMng->cancelStaff("ALL");
        evtMng->setCameraPlay(0);
        mEventFlag = 0;
        onEventFlag(0x200);
        mEventStatus = 0;
#if TARGET_PC
        // §350b site B: the 2→0 close write. Frame-stamped (H2 sequencing).
        DuskLog.info("[Evt] §350b 2→0 (close) gFrm={} nextStage={}",
                     (int)g_Counter.mCounter0, dComIfGp_isEnableNextStage() ? 1 : 0);
#endif
    } else if (mEventStatus == 0) {
        mEventFlag = 0;
    }

#if TARGET_PC
    {
        // ====================================================================
        // §341a STATUS-TRANSITION PROBE (10-hypothesis, live-state §340): the
        // tale leaves status 1 mid-item-box — log EVERY transition with the
        // discriminators for H1 (finish flags → see §318/§287 last prints),
        // H2 (early §322 exit → nextStage), H3/H4 (demo mode + §341b/c
        // attribution), H5 (eventFlag 8), H7 (susp), H8 (msg kill),
        // H10 (what entry() dispatched next). Change-only; strip with §336.
        // ====================================================================
        static int s_prevStatus341 = -1;
        if ((int)mEventStatus != s_prevStatus341) {
            JStudio::stb::TControl* c341 = dDemo_c::getControl();
            fopAc_ac_c* pl350 = dComIfGp_getPlayer(0);
            DuskLog.info(
                "[Evt] §341a status {}→{} runEvt='{}' demoMode={} f={} fnm={} susp={} "
                "nextStage={} evFlag8={} kill={} gFrm={} fade={} linkSpd={:.1f}",
                s_prevStatus341, (int)mEventStatus,
                dComIfGp_getEventManager().getRunEventName(), (int)dDemo_c::getMode(),
                (int)dDemo_c::getFrame(), (int)dDemo_c::getFrameNoMsg(),
                c341 != NULL ? (int)c341->getSuspend() : -999,
                dComIfGp_isEnableNextStage() ? 1 : 0, chkEventFlag(8) ? 1 : 0,
                dMsgObject_getMsgObjectClass() != NULL ? 1 : 0,
                (int)g_Counter.mCounter0, mDoGph_gInf_c::isFade() ? 1 : 0,
                pl350 != NULL ? pl350->speedF : -1.0f);
            // ================================================================
            // §717 H4/H6 (WAVE-1 pass-9 item 2): №269-class cover on the
            // ABORT path. Every existing ClrWallNone lives on an ARRIVAL /
            // post-transition path — an event that ends WITHOUT a transition
            // (door never opened; player stays put) had no clear, so a
            // door-open proc that set FLAG_WALL_NONE and then died with the
            // event leaves walls/ladders dead while ground stays live (the
            // user's exact 19:13 signature). This observer sees EVERY status
            // transition, so →0 with no armed stage change is the one site
            // that covers all end paths. §161 form: flags only, no reprobe.
            // WW-scoped per the shared-path rule; the log line IS the probe —
            // if it never fires, H4 dies and the wall loss is BG-side (H7).
            // ================================================================
            {
                const char* sn717 = dComIfGp_getStartStageName();
                if ((int)mEventStatus == 0 && s_prevStatus341 != 0 &&
                    !dComIfGp_isEnableNextStage() && pl350 != NULL && sn717 != NULL &&
                    dExtWwSave_isWwHostStage(sn717))
                {
                    daAlink_c* link717 = (daAlink_c*)pl350;
                    const u32 before = link717->mLinkAcch.GetFlags();
                    if ((before & dBgS_Acch::FLAG_WALL_NONE) != 0) {
                        link717->mLinkAcch.ClrWallNone();
                        link717->mLinkAcch.OffLineCheckNone();
                        DuskLog.warn(
                            "[Evt] §717 №269-abort: event ended w/o transition, WALL_NONE "
                            "was STUCK (acchFlags {:#x} → {:#x}) — walls restored",
                            (unsigned)before, (unsigned)link717->mLinkAcch.GetFlags());
                    } else {
                        DuskLog.info("[Evt] §717 abort-check: event → 0, acchFlags {:#x} "
                                     "(WALL_NONE clear — H4 not this end)",
                                     (unsigned)before);
                    }
                }
            }
            s_prevStatus341 = (int)mEventStatus;
        }
        // §285 event-end probe: teardown only fires at mEventStatus==5 AND
        // !isEnableNextStage() (№89 — a pending stage-change blocks endProc). Log the
        // gate so we see (a) whether the tale reaches status 5 (staff all done) and
        // (b) whether a next-stage is wrongly armed, stranding it (the observed hang).
        static int s_p285 = 0;
        if (mEventStatus != 0 && (mEventStatus == 5 ? ((s_p285++ % 30) == 0)
                                                    : ((s_p285++ % 120) == 0))) {
            const char* re = evtMng->getRunEventName();
            DuskLog.info(
                "[Evt] §285 mEventStatus={} isEnableNextStage={} runEvt='{}' "
                "(status5 + !nextStage ⇒ endProc→teardown)",
                (int)mEventStatus, dComIfGp_isEnableNextStage() ? 1 : 0,
                re != NULL ? re : "(none)");
        }
    }
#endif
    if (mEventStatus == 5 && !dComIfGp_isEnableNextStage()) {
        #if DEBUG
        prevEvId = mEventId;
        #endif

        if (chkFlag2(2) || mSkipTimer < 0) {
            if (mIsSkipFade) {
                mDoGph_gInf_c::fadeOut(-0.05f);
            }
            offFlag2(2);
            clear_tmpflag_for_message();
        }

        endProc();

        if (!chkFlag2(3)) {
            mIsSkipFade = false;
            evtMng->setObjectArchive(NULL);
        }

        mEventStatus = 2;

        // "%06d: event: order info (%d)"
        OS_REPORT("\x1b[36m%06d: event: ã‚ªãƒ¼ãƒ€ãƒ¼å—ä»˜ (%d)\n\x1b[m", g_Counter.mCounter0, prevEvId);
    } else if (mEventStatus == 5 && (chkFlag2(2) || mSkipTimer < 0)) {
        clear_tmpflag_for_message();
    }

    if (change() != -1) {
        #if DEBUG
        if (event_debug_evnt()) {
            // "%06d: event: event change"
            OS_REPORT("\x1b[34m%06d: event: ã‚¤ãƒ™ãƒ³ãƒˆå¤‰æ›´\n\x1b[m", g_Counter.mCounter0);
        }
        #endif

        evtMng->cancelStaff("ALL");
        evtMng->setCameraPlay(0);
        onEventFlag(0x200);
        mEventStatus = 0;

        if (chkFlag2(1)) {
            offFlag2(1);
            onFlag2(2);
        }
    }

    mCompulsory = 0;

    int roomNo = dComIfGp_roomControl_getStayNo();
    if (mRoomNo != roomNo) {
        mRoomInfoSet = 0;
        mRoomNo = roomNo;
    }

    if (mEventStatus == 0 && entry()) {
#if TARGET_PC
        // §350c: first event grant after idle — W2's end marker (H8), pairs
        // with the playerInit CREATE line as W2's start.
        DuskLog.info("[Evt] §350c entry() GRANT gFrm={}", (int)g_Counter.mCounter0);
#endif
        if (dMsgObject_getMsgObjectClass() != NULL) {
            dMsgObject_setKillMessageFlag();
        }

        onEventFlag(0x100);

        dComIfGp_getVibration().StopQuake(31);
        daAlink_c* player = daAlink_getAlinkActorClass();

        if (!dCam_getBody()->Active() && player->checkFishingCastMode()) {
            dCam_getBody()->QuickStart();
        }

        mEventStatus = 1;
        clearSkipSystem();

        #if PLATFORM_SHIELD || PLATFORM_WII
        field_0x130 = 0;
        #endif

        #if DEBUG
        const char* eventname;
        if (mEventId == -1) {
            eventname = "POTENTIAL?";
        } else {
            dEvDtEvent_c* data = evtMng->getEventData(mEventId);
            if (data == NULL) {
                eventname = "UNKOWN";
            } else {
                eventname = data->getName();
            }
        }

        // "%06d: event: start (%d=%s[%d]) %s"
        OS_REPORT("\x1b[36m%06d: event: é–‹å§‹ (%d=%s[%d]) %s\n\x1b[m", g_Counter.mCounter0, mEventId,
                  mEventId == -1 ? "PROGRAM" : basename[mEventId >> 8], mEventId & 0xFF, eventname);
        prevEvId = -99;
        #endif
    }

    mNum = 0;
    mChangeActor = NULL;

    if (chkEventFlag(0x200)) {
       Z2GetAudioMgr()->setDemoName(NULL);
    }

    if (mEventStatus != 0) {
        evtMng->Experts();

        #if PLATFORM_WII || PLATFORM_SHIELD
        if (field_0x130) {
            mDoGph_gInf_c::onWideZoom();
        }
        #endif
        return 1;
    } else {
        if (!mRoomInfoSet) {
            char lastStageName[8];
            strncpy(lastStageName, dComIfGp_getStartStageName(), sizeof(lastStageName) - 1);
            lastStageName[7] = 0;

            dComIfGp_setLastPlayStageName(lastStageName);
            dComIfGs_onVisitedRoom(mRoomNo);
        }
        mRoomInfoSet = true;
    }

    return 0;
}

int dEvt_control_c::moveApproval(void* param_0) {
    fopAc_ac_c* actor = (fopAc_ac_c*)param_0;
    if (fopAcM_CheckStatus(actor, fopAcStts_NOPAUSE_e)) {
        return 2;
    }

    if (!runCheck()) {
        return 1;
    }

    switch (mMode) {
    case dEvt_mode_TALK_e:
        if (getPt1() == actor || getPt2() == actor) {
            return 2;
        }
        break;
    case dEvt_mode_DEMO_e:
        if (getPt1() == actor || getPt2() == actor) {
            return 2;
        }

        if (actor->demoActorID != 0) {
            return 2;
        }
        break;
    case dEvt_mode_COMPULSORY_e:
        if (getPt1() == actor) {
            return 2;
        }
        break;
    }

    if (fopAcM_CheckStatus(actor, fopAcStts_STAFF_PRIMARY_e)) {
        return 2;
    }

    if (fopAcM_CheckStatus(actor, fopAcStts_STAFF_EXTRA_e)) {
        return 1;
    }

    if (dComIfGp_checkPlayerStatus0(0, 0x20000000) ||
        dComIfGp_checkPlayerStatus1(0, 0x4002000)) {
        return 0;
    }

    if (mMode == dEvt_mode_TALK_e && fopAcM_CheckStatus(actor, fopAcStts_UNK_0x40_e)) {
        return 1;
    }

    if (dDemo_c::getMode() == 1) {
        return 1;
    }

    if ((getMode() == 3 || getMode() == dEvt_mode_TALK_e) && fopAcM_CheckStatus(actor, fopAcStts_BOSS_e)) {
        return 0;
    }

    if (fopAcM_CheckStatus(actor, fopAcStts_UNK_0x4000_e)) {
        return 1;
    }

    if (fopAcM_CheckStatus(actor, fopAcStts_CARRY_NOW_e)) {
        return 1;
    }

    return 0;
}

BOOL dEvt_control_c::compulsory(void* param_0, const char* eventName, u16 hind) {
    if (!isOrderOK() || mCompulsory) {
        return 0;
    }

    mCompulsory = 1;
    return orderOld(dEvt_type_COMPULSORY_e, 2, 0, hind, param_0, NULL, eventName);
}

void dEvt_control_c::remove() {
#if TARGET_PC
    // §345a (H2): remove() is a DIRECT status-0 writer outside Step — if the
    // tale dies here, the §341a 1→0 line pairs with THIS attribution.
    if (mEventStatus != 0) {
        DuskLog.info("[Evt] §345a dEvt remove() while status={} runEvt='{}' this={} gFrm={}",
                     (int)mEventStatus, dComIfGp_getEventManager().getRunEventName(),
                     (void*)this, (int)g_Counter.mCounter0);
    }
#endif
    mMode = dEvt_mode_WAIT_e;
    mEventStatus = 0;
    mNum = 0;
    mDebugStb = 0;
    field_0xe6 = 0;
    field_0xe7 = 0;
    mCullRate = 0.0f;
    field_0xea = 0xFF;
    mMapToolId = 0xFF;
    field_0xec = 0xFF;
    mStageEventDt = NULL;
    mUnkEventId = 255;
    mTalkXyType = 0;
    mPreItemNo = dItemNo_NONE_e;
    mEventFlag = 0;
    mFlag2 = 0;
    mChangeActor = NULL;
    clearSkipSystem();
    mCompulsory = 0;
    mRoomInfoSet = false;
    mRoomNo = -1;
}

dStage_MapEvent_dt_c* dEvt_control_c::getStageEventDt() {
    return mStageEventDt;
}

void dEvt_control_c::sceneChange(int exitId) {
    dStage_changeScene4Event(exitId, mRoomNo, -1, mRoomInfoSet, 0.0f, 0, 0, -1);
}

fpc_ProcID dEvt_control_c::getPId(void* actor) {
    if (actor == NULL) {
        return fpcM_ERROR_PROCESS_ID_e;
    } else {
        return fopAcM_GetID(actor);
    }
}

fopAc_ac_c* dEvt_control_c::convPId(fpc_ProcID id) {
    return fopAcM_SearchByID(id);
}

void* dEvt_control_c::getStbDemoData(char* resName) {
    void* res = NULL;
    char* demoArcName = dStage_roomControl_c::getDemoArcName();
    
    if (*demoArcName != 0) {
        res = dComIfG_getObjectRes(demoArcName, resName);
        if (res != NULL) {
            if (event_debug_evnt()) {
                OS_REPORT("%06d: event: stb data %s found in %s.arc\n", g_Counter.mCounter0, resName, demoArcName);
            }
            return res;
        }

        if (event_debug_evnt()) {
            OS_REPORT("%06d: event: stb data %s not found in %s.arc\n", g_Counter.mCounter0, resName, demoArcName);
        }
    } else {
        if (event_debug_evnt()) {
            OS_REPORT("%06d: event: stb data %s unkown archive name\n", g_Counter.mCounter0, resName);
        }
    }

    char* roomArcName = dComIfG_getRoomArcName(dComIfGp_roomControl_getStayNo());
    res = dComIfG_getStageRes(roomArcName, resName);
    if (res != NULL) {
        if (event_debug_evnt()) {
            OS_REPORT("%06d: event: stb data %s found in %s.arc\n", g_Counter.mCounter0, resName, roomArcName);
        }
        return res;
    }

    res = dComIfG_getStageRes(resName);
    if (res != NULL) {
        if (event_debug_evnt()) {
            OS_REPORT("%06d: event: stb %s data found in %s.arc\n", g_Counter.mCounter0, resName, "Stage");
        }
        return res;
    } else {
        OS_REPORT("\x1b[43;30m%06d: event: stb data %s NOT found\n\x1b[m", g_Counter.mCounter0, resName);
    }

    return NULL;
}

dEvt_info_c::dEvt_info_c() {
    setCommand(dEvtCmd_NONE_e);
    setCondition(dEvtCnd_CANDEMO_e);
    setEventId(-1);
    setMapToolId(0xFF);
    mArchiveName = NULL;
    mIndex = 0;
}

void dEvt_info_c::setEventName(DUSK_CONST char* name) {
    if (name == NULL) {
        mEventId = -1;
    } else {
        mEventId = (s16)dComIfGp_getEventManager().getEventIdx(name, 0xFF, -1);
    }
}

char* dEvt_info_c::getEventName() {
    if (mEventId == -1) {
        return NULL;
    }

    dEvDtEvent_c* data = dComIfGp_getEventManager().getEventData(mEventId);
    if (data == NULL) {
        return NULL;
    } else {
        return data->getName();
    }
}

void dEvt_info_c::beforeProc() {
    mCondition = dEvtCnd_CANDEMO_e;
}

dStage_MapEvent_dt_c* dEvt_control_c::searchMapEventData(u8 mapToolID) {
    s32 roomNo = dComIfGp_roomControl_getStayNo();
    return searchMapEventData(mapToolID, roomNo);
}

dStage_MapEvent_dt_c* dEvt_control_c::searchMapEventData(u8 mapToolID, s32 roomNo) {
    int i;
    if (mapToolID == 0xFF) {
        return NULL;
    }

    s32 var_r28 = roomNo;
    dStage_roomDt_c* room_dt = dComIfGp_roomControl_getStatusRoomDt(var_r28);
    dStage_MapEventInfo_c* roomInfo;
    if (room_dt != NULL) {
        roomInfo = room_dt->getMapEventInfo();

        if (roomInfo != NULL) {
            for (i = 0; i < roomInfo->num; i++) {
                if (mapToolID == roomInfo->m_entries[i].field_0x4) {
                    return &roomInfo->m_entries[i];
                }
            }
        }
    }

    roomInfo = dComIfGp_getStage()->getMapEventInfo();
    if (roomInfo != NULL) {
        for (i = 0; i < roomInfo->num; i++) {
            if (mapToolID == roomInfo->m_entries[i].field_0x4) {
                return &roomInfo->m_entries[i];
            }
        }
    }

    return NULL;
}

s16 dEvt_control_c::runningEventID(s16 eventID) {
    if (eventID == mUnkEventId) {
        return mEventId;
    } else {
        return eventID;
    }
}

void dEvt_control_c::setPt1(void* actor) {
    mPt1 = getPId(actor);
}

void dEvt_control_c::setPt2(void* actor) {
    mPt2 = getPId(actor);
}

void dEvt_control_c::setPtT(void* actor) {
    mPtT = getPId(actor);
}

void dEvt_control_c::setPtI(void* actor) {
    mPtI = getPId(actor);
}

void dEvt_control_c::setPtI_Id(fpc_ProcID id) {
    mPtI = id;
}

void dEvt_control_c::setPtD(void* actor) {
    mPtd = getPId(actor);
}

void dEvt_control_c::setGtItm(u8 itemNo) {
    mGtItm = itemNo;
}

static char const* dummy2() {
    return "GIVEMAN";
}
