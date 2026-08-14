// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_npc_p1.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW PIRATES — GONZO, SENZA & NUDGE (NPC_P1, arc P1; placements P1a/P1b/
// P1c) — donor WHOLE (tale §858; §830 planner filler, user's "keep the port
// queue coming"). One shared body + per-type HEAD models with btp blinks +
// per-type BMT body recolor + the Outset layer-0xA gong stick (Senza) +
// the full action set (normal/confuse/talk/speak/p1c_speak/explain) with
// the donor's story-bit message trees + §239 head/backbone joint control.
//
// DIALECT: every substitution here is a code_dialect ROW (R5, §854) — this
// TU is the first port authored FROM the table.
// SEAMS:
//   [N1] models via the DN-3 acquirer (body/heads/dora by donor res NAME;
//        anm/btp/bmt types are mount-parsed and fetch by ID — §816 law).
//   [N2] the draw-time BMT body swap (receiver J3D lacks setMaterialTable —
//        §227 pig receipt) becomes the §229 parse-time route:
//        acquireModelDataBmt bakes the B/C body tint into each type's own
//        cached modeldata; the donor's texture save/restore dance is
//        subsumed. Same visual, one receipt.
//   [N4] kaji + pirateship are UNPORTED: sentinel proc names never match,
//        getKajiID stays ERROR, the wheel-anim sync is inert-faithful
//        (ship context — no Outset placement reaches it).
//   [N6] HIO child dropped (ls1 posture; the donor's own param table ships
//        verbatim); REG10_F debug reg reads as 0; snap figs through the
//        no-op shim with the DONOR ids (d_snap.h:123-125 = 0x73/74/75).
//   [N7] talk flow: §239 message dialect (mode / fopMsg_MODE_* /
//        messageSet(no,this,1000)); mSelectNum via the bm1 shim.
//   [N8] donor onCondition(dEvtMode_TALK_e) → dEvtCnd_CANTALK_e (the
//        family's uniform receiver condition; same "player may talk" bit).
//   [N10] dComIfGp_checkMesgSendButton has no receiver surface → false
//        (minigame-explain advance rides BOX_CLOSED only; ship context).
// ============================================================

#include "d/actor/d_a_npc_p1.h"

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_ww.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_ww_actor_shims.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_msg_mng.h"
#include "f_op/f_op_camera_mng.h"   // §877: camera_process_class full type (the ls1 §244 fix)
#include "d/d_demo.h"                  // §879: dDemo_actor_c + ENABLE_* (ls1:53 precedent — receiver demo system is real)
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "res/Object/P1.h"
#include "dusk/logging.h"
// ============================================================
// §885 PROBE (STRIP BEFORE PUSH — registered in NEVER-PUSH-STRIP-SET):
// run 134919 correlation — P1's §880 draw rewrite is FIRST-EXERCISED in the
// same runs where zl1/ls1 broke; entry/exit markers detect a draw-state
// corruption neighbor. Kill switch = WW_PROBE_884 0.
// ============================================================
#define WW_PROBE_884 1

#define fpcDwPi_NPC_P1_e fpcDwPi_E_RD_e
// [N4] unported cross-actors — sentinels that never match a live proc name.
#define fpcNm_Obj_Pirateship_e 0xFFF8
#define fpcNm_Kaji_e 0xFFF7
// [N6] donor d_snap.h:123-125 (RegistFig is the no-op shim; values donor-true).
#define DSNAP_TYPE_NPC_P1A 0x73
#define DSNAP_TYPE_NPC_P1B 0x74
#define DSNAP_TYPE_NPC_P1C 0x75
// donor SE (audio pass owed, [T5] posture)
#define JA_SE_OBJ_PIRATE_WHEEL 0
// WW story bits this TU reads/writes (raw byte<<8|bit, donor d_save.h — the
// §246 WWEV pattern; 0x0310 already in the shims).
#define WWEV_UNK_0910 0x0910
#define WWEV_UNK_0880 0x0880
#define WWEV_UNK_0840 0x0840
#define WWEV_UNK_0808 0x0808
#define WWEV_UNK_0820 0x0820   // §877 correction: NOT in the shims — my §861 claim was wrong
// donor d_event.h:32 — event-order flag, verbatim value.
#define dEvtFlag_NOPARTNER_e 0x01
// [N10] no receiver surface — the explain cut advances on BOX_CLOSED alone.
static inline bool dNpcP1_checkMesgSendButton() { return false; }

// ============================================================
// [N6] donor HIO parameter block — tuning child dropped; the donor's own
// per-type table ships verbatim (ctor values, d_a_npc_p1.cpp:91-140).
// ============================================================
struct daNpc_P1_childPrm_c {
    f32 mAttnYPosOffset;
    s16 mMaxHeadX;
    s16 mMinHeadX;
    s16 mMaxBackboneX;
    s16 mMinBackboneX;
    s16 mMaxHeadY;
    s16 mMinHeadY;
    s16 mMaxBackboneY;
    s16 mMinBackboneY;
    s16 mMaxTurnStep;
    s16 mLookBackTargetY;
    f32 mMaxTalkDist;
    f32 mMorfBackup;
};
static const daNpc_P1_childPrm_c l_prm[3] = {
    // P1A Gonzo (u16 bit-images cast per the cSAngle dialect row)
    {50.0f, 0x9C4, (s16)0xF63C, 2000, (s16)0xF830, 5000, (s16)0xEC78, 8000, (s16)0xE0C0,
     1000, 0x708, 300.0f, 8.0f},
    // P1B Senza
    {45.0f, 0x9C4, (s16)0xF63C, 2000, (s16)0xF830, 10000, (s16)0xD8F0, 8000, (s16)0xE0C0,
     1000, 0x708, 250.0f, 8.0f},
    // P1C Nudge
    {45.0f, 0x9C4, (s16)0xF63C, 2000, (s16)0xF830, 5000, (s16)0xEC78, 8000, (s16)0xE0C0,
     1000, 0x708, 250.0f, 7.0f},
};

// donor l_cyl_src semantics in the receiver initializer shape: At none;
// Tg = ALL attacks, Set|IsEnemy (0x03); Co Set|IsPlayer|VsGrpAll (0x75);
// GObjTg no-hit-mark (0x4). NPC-standard target/push cylinder.
static const dCcD_SrcCyl l_p1CylSrc = {
    {
        {0x0, {{0, 0, 0}, {0xFFFFFFFF, 0x03}, 0x75}},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x4},
        {0x0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            40.0f,
            160.0f,
        }
    }
};

static fpc_ProcID l_msgId;
static msg_class* l_msg;

void daNpc_P1_c::setAnimFromMsg() {
    if ((mAnmNum == 10 || mAnmNum == 11) && !mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
        return;
    }

    if (mAnmNum == 8 && mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
        if (--m66C <= 0) {
            setAnm(4, 15.0f);
        }
    } else if (mAnmNum == 12 && mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
        if (--m66C <= 0) {
            setAnm(9, 8.0f);
        }
    }
    int iVar1;
    switch (dComIfGp_getMesgAnimeAttrInfo()) {
        case 0:
            iVar1 = (mType != TYPE_P1C_e) ? 0 : 4;
            break;
        case 1: iVar1 = 1; break;
        case 2: iVar1 = 2; break;
        case 3: iVar1 = 3; break;
        case 4:
            iVar1 = (mType != TYPE_P1C_e) ? 9 : 4;
            break;
        case 5: iVar1 = 10; break;
        case 6: iVar1 = 0xb; break;
        case 7: iVar1 = 0xc; m66C = 4; break;
        case 8: iVar1 = 0xd; break;
        case 9: iVar1 = 0xe; break;
        case 10: iVar1 = 0xf; break;
        case 0xb: iVar1 = 0x10; break;
        case 0xc: iVar1 = 4; break;
        case 0xd: iVar1 = 5; break;
        case 0xe: iVar1 = 6; break;
        case 0xf: iVar1 = 7; break;
        case 0x10: iVar1 = 8; m66C = 6; break;
        default:
            return;
    }
    dComIfGp_setMesgAnimeAttrInfo(0xFF);
    setAnm(iVar1, -1.0);
}

BOOL daNpc_P1_c::setAnm(int i_anm, f32 i_morf) {
    if (mAnmNum == i_anm) {
        return false;
    }

    mAnmNum = i_anm;

    if (i_morf < 0.0f) {
        i_morf = l_prm[mType].mMorfBackup;
    }

    // donor per-case res fetch collapsed to a table (same ids, same modes —
    // [N1]: BCK members are mount-parsed, ID fetch is sound).
    static const struct { int res; int mode; } a_anm_tbl[17] = {
        {dRes_INDEX_P1_BCK_WAIT_e,     J3DFrameCtrl::EMode_LOOP},   // 0
        {dRes_INDEX_P1_BCK_TALK_e,     J3DFrameCtrl::EMode_LOOP},   // 1
        {dRes_INDEX_P1_BCK_TALK02_e,   J3DFrameCtrl::EMode_LOOP},   // 2
        {dRes_INDEX_P1_BCK_TALK03_e,   J3DFrameCtrl::EMode_LOOP},   // 3
        {dRes_INDEX_P1_BCK_C_WAIT_e,   J3DFrameCtrl::EMode_LOOP},   // 4
        {dRes_INDEX_P1_BCK_C_TALK01_e, J3DFrameCtrl::EMode_LOOP},   // 5
        {dRes_INDEX_P1_BCK_C_TALK02_e, J3DFrameCtrl::EMode_NONE},   // 6
        {dRes_INDEX_P1_BCK_C_STOP_e,   J3DFrameCtrl::EMode_LOOP},   // 7
        {dRes_INDEX_P1_BCK_C_TALK01_e, J3DFrameCtrl::EMode_LOOP},   // 8
        {dRes_INDEX_P1_BCK_WAIT02_e,   J3DFrameCtrl::EMode_LOOP},   // 9
        {dRes_INDEX_P1_BCK_OMOKJ_e,    J3DFrameCtrl::EMode_NONE},   // 10
        {dRes_INDEX_P1_BCK_TORIKJ_e,   J3DFrameCtrl::EMode_NONE},   // 11
        {dRes_INDEX_P1_BCK_ANGRY_e,    J3DFrameCtrl::EMode_LOOP},   // 12
        {dRes_INDEX_P1_BCK_TALK04_e,   J3DFrameCtrl::EMode_LOOP},   // 13
        {dRes_INDEX_P1_BCK_CHECK01_e,  J3DFrameCtrl::EMode_LOOP},   // 14
        {dRes_INDEX_P1_BCK_CHECK02_e,  J3DFrameCtrl::EMode_LOOP},   // 15
        {dRes_INDEX_P1_BCK_LOOK_e,     J3DFrameCtrl::EMode_LOOP},   // 16
    };
    if (i_anm < 0 || i_anm > 16) {
        return false;
    }
    J3DAnmTransform* anm =
        (J3DAnmTransform*)dComIfG_getObjectRes("P1", a_anm_tbl[i_anm].res);
    mpMorf->setAnm(anm, a_anm_tbl[i_anm].mode, i_morf, 1.0f, 0.0f, -1.0f, NULL);
    return TRUE;
}

BOOL daNpc_P1_c::normalAction(void*) {
    cXyz posdiff = (dComIfGp_getPlayer(0)->current.pos - current.pos);
    f32 fVar4 = posdiff.absXZ();
    if (mActionStatus == ACTION_STARTING_e) {
        if (mType == TYPE_P1A_e) {
            if (m671 == 1) {
                if (mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
                    setAnm(9, -1.0);
                } else {
                    return FALSE;
                }
            } else {
                setAnm(0, -1.0);
            }
        } else if (mType == TYPE_P1B_e) {
            if (mParam == 2) {
                setAnm(0xE, -1.0);
            } else if (mParam == 3) {
                setAnm(0x10, -1.0);
            } else {
                setAnm(0, -1.0);
            }
        } else {
            setAnm(4, -1.0);
        }
        mActionStatus += 1;
    } else if (mActionStatus != ACTION_ENDING_e) {
        if (eventInfo.checkCommandTalk()) {
            setAction(&daNpc_P1_c::talkAction, NULL, 0);
        }
        if (fVar4 < l_prm[mType].mMaxTalkDist) {
            eventInfo.onCondition(dEvtCnd_CANTALK_e);   // [N8] donor dEvtMode_TALK_e
        }
        if (mType == TYPE_P1C_e) {
            cLib_addCalcAngleS2(&current.angle.y, home.angle.y, 8, 0x400);
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::confuseAction(void*) {
    if (mActionStatus == ACTION_STARTING_e) {
        if (mType == TYPE_P1A_e) {
            setAnm(1, -1.0);
        } else {
            setAnm(2, -1.0);
        }
        mpMorf->setPlaySpeed(2.0f);
        mActionStatus += 1;
    } else if (mpMorf->checkFrame(mpMorf->getEndFrame() - 2.0f)) {
        m66C += 1;
        if (m66C > 3) {
            if (mAnmNum == 1) {
                setAnm(2, -1.0f);
            } else {
                setAnm(1, -1.0f);
            }
            mpMorf->setPlaySpeed(2.0f);
            m66C = 0;
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::talkAction(void*) {
    if (mActionStatus == ACTION_STARTING_e) {
        if (mAnmNum == 10 || mAnmNum == 11) {
            return 0;
        }
        mPrevMesg = mCurrMesg;
        if (mType == TYPE_P1A_e) {
            if (mParam == 1) {
                if (dComIfGs_isEventBit(WWEV_UNK_0910)) {
                    mCurrMesg = 0xC95;
                } else {
                    dComIfGs_onEventBit(WWEV_UNK_0910);
                    mCurrMesg = 0xC94;
                }
            } else if (mParam == 3) {
                mCurrMesg = 0x100A;
            } else {
                if (!dComIfGs_isEventBit(WWEV_UNK_0880)) {
                    mCurrMesg = 0x1007;
                    dComIfGs_onEventBit(WWEV_UNK_0880);
                } else {
                    mCurrMesg = 0x1009;
                }
            }
        } else if (mType == TYPE_P1B_e) {
            if (mParam == 1) {
                mCurrMesg = 0xFA1;
            } else if (mParam == 3) {
                mCurrMesg = 0x100D;
            } else {
                if (!dComIfGs_isEventBit(WWEV_UNK_0840)) {
                    mCurrMesg = 0x100B;
                    dComIfGs_onEventBit(WWEV_UNK_0840);
                } else if (dComIfGs_getClearCount() == 0) {
                    mCurrMesg = 0x100C;
                } else {
                    mCurrMesg = 0x1034;
                }
            }
            if (mParam == 2) {
                m_jnt.offHeadLock();
            }
        } else {
            if (dComIfGs_isEventBit(WWEV_UNK_0808)) {
                mCurrMesg = 0x1017;
            } else {
                if (!dComIfGs_isEventBit(WWEV_UNK_0820)) {
                    mCurrMesg = 0x1014;
                    dComIfGs_onEventBit(WWEV_UNK_0820);
                } else {
                    mCurrMesg = 0x1015;
                }
            }
        }
        mActionStatus++;
        l_msgId = fpcM_ERROR_PROCESS_ID_e;
    } else if (mActionStatus != ACTION_ENDING_e) {
        if (l_msgId == fpcM_ERROR_PROCESS_ID_e) {
            l_msgId = fopMsgM_messageSet(mCurrMesg, this, 1000);   // [N7]
        } else {
            if (mType == TYPE_P1B_e) {
                if (mParam != 2 && mParam != 3) {
                    m_jnt.setTrn();
                }
            } else if (mType == TYPE_P1A_e) {
                if (mParam != 2) {
                    m_jnt.setTrn();
                }
            } else {
                m_jnt.setTrn();
            }
            setAnimFromMsg();
            if (mActionStatus == ACTION_ONGOING_e) {
                l_msg = fopMsgM_SearchByID(l_msgId);
                if (l_msg != NULL) {
                    mActionStatus++;
                }
            } else if (l_msg->mode == fopMsg_MODE_MSG_DISPLAYED_e) {   // [N7]
                if (getNextMsgNo(1) != (u32)-1) {
                    l_msg->mode = fopMsg_MODE_MSG_CONTINUE_e;
                    fopMsgM_messageSet(mCurrMesg, 1000);
                } else {
                    l_msg->mode = fopMsg_MODE_MSG_END_e;
                }
            } else if (l_msg->mode == fopMsg_MODE_BOX_CLOSED_e) {
                if (mParam == 2) {
                    m_jnt.onHeadLock();
                }

                l_msg->mode = fopMsg_MODE_MSG_DESTROYED_e;
                setAction(mPrevAction, NULL, 0);
                dComIfGp_event_reset();
            }
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::p1c_speakAction(void*) {
    if (mActionStatus == ACTION_STARTING_e) {
        if (!eventInfo.checkCommandTalk()) {
            cXyz temp1 = (dComIfGp_getPlayer(0)->current.pos - home.pos);
            if (temp1.absXZ() < 200.0f) {
                fopAcM_orderSpeakEvent(this, 0, 0);
                eventInfo.onCondition(dEvtCnd_CANTALK_e);
            }
            return FALSE;
        }
        mCurrMesg = 0x1014;
        mPrevMesg = 0;
        mActionStatus += 1;
        l_msg = NULL;
        l_msgId = fpcM_ERROR_PROCESS_ID_e;
    } else if (mActionStatus != ACTION_ENDING_e) {
        if (l_msgId == fpcM_ERROR_PROCESS_ID_e) {
            l_msgId = fopMsgM_messageSet(mCurrMesg, this, 1000);
        } else {
            setAnimFromMsg();
            if (l_msg == NULL) {
                l_msg = fopMsgM_SearchByID(l_msgId);
            } else if (l_msg->mode == fopMsg_MODE_MSG_DISPLAYED_e) {
                if (getNextMsgNo(1) != (u32)-1) {
                    l_msg->mode = fopMsg_MODE_MSG_CONTINUE_e;
                    fopMsgM_messageSet(mCurrMesg, 1000);
                } else {
                    l_msg->mode = fopMsg_MODE_MSG_END_e;
                }
            } else if (l_msg->mode == fopMsg_MODE_BOX_CLOSED_e) {
                setAnm(4, 8.0f);
                cLib_addCalcPos2(&current.pos, home.pos, 0.75f, 5.0f);
                cXyz posdiff = home.pos - current.pos;
                f32 dist_xz = posdiff.absXZ();
                if (dist_xz < 1.0f) {
                    dComIfGs_onEventBit(WWEV_UNK_0820);
                    mCyl.SetR(100.0f);
                    l_msg->mode = fopMsg_MODE_MSG_DESTROYED_e;
                    setAction(&daNpc_P1_c::normalAction, NULL, 0);
                    dComIfGp_event_reset();
                }
            }
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::speakAction(void*) {
    if (mActionStatus == ACTION_STARTING_e) {
        if (mPrevMesg == 0xFAE) {
            // §879: dCamera_c::SkipSmoother is a WW camera-smoother latch the
            // receiver camera does not have (kikuzu-class owed surface) — the
            // §253-pattern shim carries the call; WW-camera lane (CRAWL
            // precedent, ww_cam_crawl.cpp) owns the smoother port.
            dExtWwCam_SkipSmoother();
        }
        if (!eventInfo.checkCommandTalk()) {
            cXyz temp1 = (dComIfGp_getPlayer(0)->current.pos - current.pos);
            if (temp1.absXZ() < 400.0f) {
                fopAcM_orderSpeakEvent(this, 0, 0);
                eventInfo.onCondition(dEvtCnd_CANTALK_e);
            }
            return FALSE;
        }
        if (mPrevMesg == 0xFAE) {
            mPrevMesg = mCurrMesg;
            mCurrMesg = 0xFA5;
        } else if (dComIfGp_getStartStagePoint() == 2) {
            mPrevMesg = mCurrMesg;
            mCurrMesg = 0xFAA;
        } else {
            mPrevMesg = mCurrMesg;
            mCurrMesg = 0xFA1;
        }
        mActionStatus += 1;
        l_msgId = fpcM_ERROR_PROCESS_ID_e;
    } else if (mActionStatus != ACTION_ENDING_e) {
        if (l_msgId == fpcM_ERROR_PROCESS_ID_e) {
            l_msgId = fopMsgM_messageSet(mCurrMesg, this, 1000);
        } else {
            m_jnt.setTrn();
            setAnimFromMsg();
            if (mActionStatus == ACTION_ONGOING_e) {
                if (mCurrMesg == 0xFA5) {
                    // §879: dCamera_c::SkipSmoother is a WW camera-smoother latch the
            // receiver camera does not have (kikuzu-class owed surface) — the
            // §253-pattern shim carries the call; WW-camera lane (CRAWL
            // precedent, ww_cam_crawl.cpp) owns the smoother port.
            dExtWwCam_SkipSmoother();
                }
                l_msg = fopMsgM_SearchByID(l_msgId);
                if (l_msg != NULL) {
                    mActionStatus += 1;
                }
            } else if (l_msg->mode == fopMsg_MODE_MSG_DISPLAYED_e) {
                if (getNextMsgNo(1) != (u32)-1) {
                    l_msg->mode = fopMsg_MODE_MSG_CONTINUE_e;
                    fopMsgM_messageSet(mCurrMesg, 1000);
                } else {
                    l_msg->mode = fopMsg_MODE_MSG_END_e;
                }
            } else if (l_msg->mode == fopMsg_MODE_BOX_CLOSED_e) {
                l_msg->mode = fopMsg_MODE_MSG_DESTROYED_e;
                setAction(&daNpc_P1_c::normalAction, NULL, 0);
                if (mPrevMesg == 0xFA4) {
                    dComIfGp_event_reset();
                    fopAcM_orderOtherEvent(this, "sea_exp_cam", 0, dEvtFlag_NOPARTNER_e, 0);   // §877: receiver name-form (5-arg)
                    eventInfo.onCondition(dEvtCnd_CANDEMO_e);   // donor dEvtCnd_UNK2_e (bit 0x2)
                    setAction(&daNpc_P1_c::explainAction, NULL, 0);
                } else {
                    dComIfGp_setNextStage("Ocean", 1, fopAcM_GetRoomNo(this), -1);   // §879: receiver 4-arg form, layer -1 = donor default
                }
            }
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::explainAction(void*) {
    if (mActionStatus == ACTION_STARTING_e) {
        mActionStatus += 1;
    } else if (mActionStatus == ACTION_ENDING_e && !eventInfo.checkCommandTalk()) {
        fopAcM_orderSpeakEvent(this, 0, 0);
        eventInfo.onCondition(dEvtCnd_CANTALK_e);
        return FALSE;
    }
    return TRUE;
}

u32 daNpc_P1_c::getNextMsgNo(int i_param_1) {
    u32 o_retval;
    switch (mCurrMesg) {
        case 0xC94:
            o_retval = 0xC99;
            break;
        case 0xFA1: case 0xFA2: case 0xFA5: case 0xFA6: case 0xFA7:
        case 0xFAA: case 0xFAB: case 0x1015: case 0x1007:
            o_retval = mCurrMesg + 1;
            break;
        case 0x1009:
            o_retval = 0x102E;
            break;
        case 0x1016:
            o_retval = 0x1033;
            break;
        case 0xFAE:
            o_retval = 0xFA5;
            break;
        case 0xFA8: case 0xFAC:
            o_retval = 0xFA2;
            break;
        case 0xFA3: {
            // [N7] donor l_msg->mSelectNum — the bm1 shim reads the port stub.
            s16 sel = dExtNpcBm1_msgSelectNum(l_msg);
            if (sel == 0) {
                o_retval = 0xFA4;
            } else if (sel == 1) {
                o_retval = 0xFB0;
            } else {
                o_retval = (u32)-1;
            }
            break;
        }
        case 0xFA4: case 0xFA9:
        default:
            o_retval = (u32)-1;
            break;
    }
    if (i_param_1 == 1) {
        mPrevMesg = mCurrMesg;
        mCurrMesg = o_retval;
    }
    return o_retval;
}

BOOL daNpc_P1_c::playTexPatternAnm() {
    if (!cLib_calcTimer(&mBlinkTimer)) {
        mBlinkFrame += 1;
        if (mBlinkFrame >= mpTexture->getFrameMax()) {
            mBlinkFrame -= mpTexture->getFrameMax();
            s16 rng = cM_rndF(100.0f);
            mBlinkTimer = rng + 0x1E;
        }
    }
    return TRUE;
}

void daNpc_P1_c::demo_end_init() {
    if (mParam == 0) {
        mAnmNum = -1;
        ActionFunc local_48 = &daNpc_P1_c::confuseAction;
        if (mActionFunc != NULL) {
            mActionStatus = ACTION_ENDING_e;
            (this->*mActionFunc)(0);
        }
        mPrevAction = mActionFunc;
        mActionFunc = local_48;
        mActionStatus = ACTION_STARTING_e;
        (this->*mActionFunc)(NULL);
        // [N4] donor repositions onto the pirate ship — unported; the search
        // never matches, the reposition never runs (ship context).
        fopAc_ac_c* actor = NULL;
        fopAcM_SearchByName(fpcNm_Obj_Pirateship_e, &actor);
        if (actor) {
            current.pos = actor->current.pos;
            current.pos.y += 700.0f;
            current.pos.x += cM_ssin(actor->current.angle.y) * 950.0f;
            current.pos.z += cM_scos(actor->current.angle.y) * 950.0f;
            current.pos.x += cM_scos(actor->current.angle.y) * 150.0f;
            current.pos.z -= cM_ssin(actor->current.angle.y) * 150.0f;
            s16 sVar2 = actor->current.angle.y - 0x4000;
            actor->current.angle.y = sVar2;
            current.angle.y = sVar2;
        }
    }
    m_jnt.offHeadLock();
    m_jnt.offBackBoneLock();
    m670 = 0;
}

BOOL daNpc_P1_c::demo_move() {
    dDemo_actor_c* actor = dComIfGp_demo_getActor(demoActorID);
    if (!actor) {
        if (m670 == 1) {
            demo_end_init();
        }
        return FALSE;
    }
    m_jnt.onHeadLock();
    m_jnt.onBackBoneLock();
    m670 = 1;
    J3DAnmTexPattern* btp = actor->getP_BtpData("P1");   // §251 native demo-btp path
    if (btp) {
        mBtp.init(mpHeadModel->getModelData(), btp, TRUE, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1);
        mBlinkFrame = 0;
    }
    if (mBtp.getBtpAnm()) {
        u8 uVar1 = mBtp.getBtpAnm()->getFrameMax();
        mBlinkFrame += 1;
        if (mBlinkFrame >= uVar1) {
            mBlinkFrame = uVar1;
        }
    }
    dDemo_setDemoData(this,
                      dDemo_actor_c::ENABLE_TRANS_e | dDemo_actor_c::ENABLE_ROTATE_e |
                          dDemo_actor_c::ENABLE_ANM_e | dDemo_actor_c::ENABLE_ANM_FRAME_e,
                      mpMorf, "P1", 0, NULL, 0, 0);   // port 8-arg
    return TRUE;
}

BOOL daNpc_P1_c::event_move() {
    if (mEventCut6B0.cutProc()) {
        mbAttentionFlag = mEventCut6B0.getAttnFlag();
        if (mEventCut6B0.getAttnFlag() == 0) {
            mEventCut6B0.setAttnFlag(mbAttentionFlag);
        }
        return TRUE;
    } else {
        return privateCut();
    }
}

BOOL daNpc_P1_c::evn_setAnm_init(int i_staff_id) {
    int* idx_p = dComIfGp_evmng_getMyIntegerP(i_staff_id, "idx");
    if (idx_p != NULL) {
        setAnm(*idx_p, -1.0f);
    } else if (mType == TYPE_P1C_e) {
        setAnm(4, -1.0f);
    } else {
        setAnm(0, -1.0f);
    }
    return TRUE;
}

BOOL daNpc_P1_c::evn_talk_init(int i_staff_id) {
    int* mesg_no_p = dComIfGp_evmng_getMyIntegerP(i_staff_id, "MsgNo");
    l_msgId = fpcM_ERROR_PROCESS_ID_e;
    l_msg = NULL;
    if (mesg_no_p != NULL) {
        mCurrMesg = *mesg_no_p;
    } else {
        mCurrMesg = 0;
    }
    return TRUE;
}

BOOL daNpc_P1_c::evn_talk() {
    if (l_msgId == fpcM_ERROR_PROCESS_ID_e) {
        l_msgId = fopMsgM_messageSet(mCurrMesg, this, 1000);
    } else if (l_msg == NULL) {
        l_msg = fopMsgM_SearchByID(l_msgId);
    } else if (l_msg->mode == fopMsg_MODE_MSG_DISPLAYED_e) {
        if (getNextMsgNo(true) != (u32)-1) {
            l_msg->mode = fopMsg_MODE_MSG_CONTINUE_e;
            fopMsgM_messageSet(mCurrMesg, 1000);
        } else {
            l_msg->mode = fopMsg_MODE_MSG_END_e;
        }
    } else if (l_msg->mode == fopMsg_MODE_BOX_CLOSED_e) {
        l_msg->mode = fopMsg_MODE_MSG_DESTROYED_e;
        l_msg = NULL;
        l_msgId = fpcM_ERROR_PROCESS_ID_e;
        return TRUE;
    }
    return FALSE;
}

BOOL daNpc_P1_c::minigameExplainCut() {
    static char* ActionNames[] = {(char*)"4013_msg", (char*)"4014_msg"};
    int staffId = dComIfGp_evmng_getMyStaffId(mEventCut6B0.getActorName(), this, 0);
    int actIdx = dComIfGp_evmng_getMyActIdx(staffId, ActionNames, 2, TRUE, 0);

    if (staffId == -1) {
        mbAttentionFlag = 0;
        return 0;
    }
    if (m65A == 0) {
        l_msgId = fpcM_ERROR_PROCESS_ID_e;
        dComIfGp_onCameraAttentionStatus(0, 4);
        mPrevMesg = mCurrMesg;
        mCurrMesg = 0xFAD;
        m65A++;
    } else if (m65A != -1) {
        if (l_msgId == fpcM_ERROR_PROCESS_ID_e) {
            l_msgId = fopMsgM_messageSet(mCurrMesg, this, 1000);
        } else {
            if (m65A == 1) {
                l_msg = fopMsgM_SearchByID(l_msgId);
                if (l_msg != NULL) {
                    m65A++;
                }
            } else {
                if (l_msg->mode == fopMsg_MODE_MSG_DISPLAYED_e) {
                    if (mCurrMesg == 0xFAD) {
                        l_msg->mode = fopMsg_MODE_MSG_CONTINUE_e;
                        mPrevMesg = mCurrMesg;
                        mCurrMesg = 0xFAE;
                        fopMsgM_messageSet(mCurrMesg, 1000);
                    } else {
                        mPrevMesg = mCurrMesg;
                        mCurrMesg = (u32)-1;
                        l_msg->mode = fopMsg_MODE_MSG_END_e;
                    }
                } else if (l_msg->mode == fopMsg_MODE_BOX_CLOSED_e) {
                    if (actIdx == 1) {
                        dComIfGp_evmng_cutEnd(staffId);
                    }
                    l_msg->mode = fopMsg_MODE_MSG_DESTROYED_e;
                    dComIfGp_offCameraAttentionStatus(0, 4);
                } else if (dNpcP1_checkMesgSendButton() && actIdx == 0) {   // [N10]
                    dComIfGp_evmng_cutEnd(staffId);
                }
            }
            if (checkAction(&daNpc_P1_c::explainAction)) {
                if (dComIfGp_evmng_endCheck((char*)"sea_exp_cam")) {
                    setAction(&daNpc_P1_c::speakAction, NULL, 0);
                    dComIfGp_event_reset();
                    // §879: dCamera_c::SkipSmoother is a WW camera-smoother latch the
            // receiver camera does not have (kikuzu-class owed surface) — the
            // §253-pattern shim carries the call; WW-camera lane (CRAWL
            // precedent, ww_cam_crawl.cpp) owns the smoother port.
            dExtWwCam_SkipSmoother();
                    fopAcM_orderSpeakEvent(this, 0, 0);
                    eventInfo.onCondition(dEvtCnd_CANTALK_e);
                }
            }
        }
    }
    return TRUE;
}

BOOL daNpc_P1_c::privateCut() {
    static char* cut_name_tbl[] = {(char*)"4013_msg", (char*)"4014_msg", (char*)"SETANM",
                                   (char*)"HEADSWING", (char*)"TALKMSG"};
    int staffIdx = dComIfGp_evmng_getMyStaffId(mEventCut6B0.getActorName(), this, 0);
    if (staffIdx == -1) {
        return FALSE;
    }
    int actIdx = dComIfGp_evmng_getMyActIdx(staffIdx, cut_name_tbl, 5, TRUE, 0);
    if (actIdx == -1) {
        dComIfGp_evmng_cutEnd(staffIdx);
    } else {
        int iVar4 = 0;
        if (dComIfGp_evmng_getIsAddvance(staffIdx)) {
            switch (actIdx) {
                case 2:
                    evn_setAnm_init(staffIdx);
                    break;
                case 3:
                    mHeadAnm.swing_vertical_init(2, 0x1000, 0x800, 1);
                    break;
                case 4:
                    evn_talk_init(staffIdx);
            }
        }
        switch (actIdx) {
            case 0:
            case 1:
                minigameExplainCut();
                break;
            case 4:
                iVar4 = evn_talk();
                setAnimFromMsg();
                break;
            default:
                iVar4 = 1;
                break;
        }
        if (iVar4 != 0) {
            dComIfGp_evmng_cutEnd(staffIdx);
        }
    }
    return TRUE;
}

// receiver joint-callback signature (code_dialect row; npc_tk idiom).
static int daNpc_P1_nodeCallBack1(J3DJoint* joint, int i_param_2) {
    if (i_param_2 == J3DNodeCBCalcTiming_In) {
        J3DModel* model = j3dSys.getModel();
        daNpc_P1_c* i_this = (daNpc_P1_c*)model->getUserArea();
        s32 jntNo = joint->getJntNo();
        if (i_this) {
            mDoMtx_stack_c::copy(model->getAnmMtx(jntNo));
            if (jntNo == i_this->m_jnt.getHeadJntNum()) {
                cXyz local_28(0.0f, 0.0f, 0.0f);   // donor REG10_F debug offsets -> 0 [N6]
                mDoMtx_stack_c::YrotM(
                    (s16)(-i_this->m_jnt.getHead_y() - i_this->mHeadAnm.field_0x02));
                mDoMtx_stack_c::ZrotM(
                    (s16)(-i_this->m_jnt.getHead_x() - i_this->mHeadAnm.field_0x00));
                mDoMtx_stack_c::multVec(&local_28, &i_this->eyePos);
            }
            if (jntNo == i_this->m_jnt.getBackboneJntNum()) {
                mDoMtx_stack_c::XrotM(i_this->m_jnt.getBackbone_y());
                mDoMtx_stack_c::ZrotM(i_this->m_jnt.getBackbone_x());
            }

            model->setAnmMtx(jntNo, mDoMtx_stack_c::get());
            cMtx_copy(mDoMtx_stack_c::get(), J3DSys::mCurrentMtx);
        }
    }
    return 1;
}

static BOOL daNpc_P1_CheckCreateHeap(fopAc_ac_c* i_this) {
    return ((daNpc_P1_c*)i_this)->CreateHeap();
}

BOOL daNpc_P1_c::setAttentionPos(cXyz* i_param_1) {
    i_param_1->set(eyePos.x, eyePos.y, eyePos.z);
    i_param_1->y += l_prm[mType].mAttnYPosOffset;
    return TRUE;
}

cPhs_Step daNpc_P1_c::_create() {
    fopAcM_ct(this, daNpc_P1_c);   // [J3] donor ct_Retail/ct_Demo pair
    cPhs_Step state = dComIfG_resLoad(&mPhs, "P1");
    if (state == cPhs_COMPLEATE_e) {
        u32 param_bit = fopAcM_GetParamBit(this, 0x14, 4);   // §877 actor-form
        mParam = fopAcM_GetParam(this);
        if (param_bit == 0) {
            mType = TYPE_P1A_e;
            if (mParam == 2) {
                m671 = 1;
                m_jnt.onBackBoneLock();
                mKajiId = getKajiID();   // [N4] ERROR (ship unported)
            } else {
                m671 = 0;
            }
            mEventCut6B0.setActorInfo((char*)"P1a", this);
        } else if (param_bit == 1) {
            mType = TYPE_P1B_e;
            m671 = 0;
            mEventCut6B0.setActorInfo((char*)"P1b", this);
        } else {
            mType = TYPE_P1C_e;
            m671 = 0;
            mEventCut6B0.setActorInfo((char*)"P1c", this);
        }
        mEventCut6B0.setJntCtrlPtr(&m_jnt);
        u32 max_heap_size;
        switch (mType) {
            case TYPE_P1A_e:
                max_heap_size = 0x25C0;
                break;
            case TYPE_P1B_e:
                if (strcmp(dComIfGp_getStartStageName(), "sea") == 0 &&
                    fopAcM_GetRoomNo(this) == dIsleRoom_OutsetIsland_e &&
                    dComIfGp_getStartStageLayer() == 0xA) {
                    max_heap_size = 0x25C0;
                } else {
                    max_heap_size = 0x2120;
                }
                break;
            default:
                max_heap_size = 0x2120;
                break;
        }
        if (!fopAcM_entrySolidHeap(this, daNpc_P1_CheckCreateHeap, max_heap_size)) {
            return cPhs_ERROR_e;
        }
        fopAcM_SetMtx(this, mpMorf->getModel()->getBaseTRMtx());
        gravity = -9.0f;
        attention_info.flags = fopAc_Attn_LOCKON_TALK_e | fopAc_Attn_ACTION_SPEAK_e;
        // [N6] HIO child dropped.
        mStts.Init(0xFF, 0xFF, this);
        mCyl.Set(l_p1CylSrc);
        mCyl.SetStts(&mStts);
        mAnmNum = -1;
        m66C = 0;
        m670 = 0;
        mKajiTimer = 300;
        mActionFunc = NULL;
        mPrevAction = NULL;
        mShadowId = 0;
        mbAttentionFlag = 0;
        m65A = 0;
        mBlinkTimer = 0;
        mBlinkFrame = 0;
        mMaxLookVel = 0;
        mCurrMesg = 0;
        mPrevMesg = 0;

        if (mType == TYPE_P1B_e) {
            attention_info.distances[1] = 0xAA;
            attention_info.distances[3] = 0xAA;
            u8 type = mParam;
            if (mParam == 1) {
                if (dComIfGp_getStartStagePoint() == 0 || dComIfGp_getStartStagePoint() == 2) {
                    setAction(&daNpc_P1_c::speakAction, NULL, 0);
                }
            } else if (type == 0) {
                setAction(&daNpc_P1_c::confuseAction, NULL, 0);
            } else {
                if (type == 2) {
                    m_jnt.onBackBoneLock();
                    m_jnt.onHeadLock();
                    mCyl.SetR(90.0f);
                } else if (type == 3) {
                    m_jnt.onBackBoneLock();
                    m_jnt.onHeadLock();
                }
                setAction(&daNpc_P1_c::normalAction, NULL, 0);
            }
        } else if (mType == TYPE_P1C_e) {
            attention_info.distances[1] = 0xAA;
            attention_info.distances[3] = 0xAA;
            if (!dComIfGs_isEventBit(WWEV_UNK_0820) && !dComIfGs_isEventBit(WWEV_UNK_0808)) {
                current.pos.x -= cM_scos(current.angle.y) * 40.0f;
                current.pos.z += cM_ssin(current.angle.y) * 40.0f;
                setAnm(4, -1.0f);
                setAction(&daNpc_P1_c::p1c_speakAction, NULL, 0);
            } else {
                setAction(&daNpc_P1_c::normalAction, NULL, 0);
            }
            mCyl.SetR(100.0f);
        } else {
            attention_info.distances[1] = 0xAB;
            attention_info.distances[3] = 0xAB;
            if (m671 != 0) {
                setAnm(9, 0.0f);
                mCyl.SetR(90.0f);   // donor REG10_F(5)+90 -> 90 [N6]
            }
            setAction(&daNpc_P1_c::normalAction, NULL, 0);
        }
        if (mActionFunc == NULL) {
            setAction(&daNpc_P1_c::normalAction, NULL, 0);
        }
        if (mType == TYPE_P1B_e && mParam == 1 && dComIfGp_getStartStagePoint() == 1) {
            return cPhs_ERROR_e;
        }
    }
    return state;
}

BOOL daNpc_P1_c::CreateHeap() {
    // [N1] body via the DN-3 acquirer; [N2] B/C bodies take the §229
    // parse-time BMT route (receiver J3D lacks the draw-time swap — §227).
    J3DModelData* model_data_p;
    if (mType == TYPE_P1B_e) {
        model_data_p = dExtNpcMount_acquireModelDataBmt("P1", "p1.bdl", "p1b_body.bmt");
    } else if (mType == TYPE_P1C_e) {
        model_data_p = dExtNpcMount_acquireModelDataBmt("P1", "p1.bdl", "p1c_body.bmt");
    } else {
        model_data_p = dExtNpcMount_acquireModelData("P1", "p1.bdl");
    }
    if (model_data_p == NULL) {
        DuskLog.warn("[NpcP1] p1.bdl unresolvable (type {})", (int)mType);
        return FALSE;
    }
    mpMorf = new mDoExt_McaMorf(
        model_data_p, NULL, NULL,
        (J3DAnmTransformKey*)dComIfG_getObjectRes("P1", dRes_INDEX_P1_BCK_WAIT_e),
        J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1, 1, NULL, 0x80000, 0x11020002);
    if (!mpMorf || !mpMorf->getModel()) {
        return FALSE;
    }
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
    JUTNameTab* jnt_names = model_data_p->getJointTree().getJointName();
    m_jnt.setHeadJntNum(jnt_names->getIndex("head"));
    m_jnt.setBackboneJntNum(jnt_names->getIndex("backbone"));
    m_handR_jnt_num = jnt_names->getIndex("handR");
    if (m_jnt.getHeadJntNum() < 0 || m_jnt.getBackboneJntNum() < 0 || m_handR_jnt_num < 0) {
        DuskLog.warn("[NpcP1] joint lookup failed");
        return FALSE;
    }
    const char* head_name;
    int btp_idx;
    if (mType == TYPE_P1A_e) {
        head_name = "p1a_head.bdl";
        btp_idx = dRes_INDEX_P1_BTP_MABA_A_e;
    } else if (mType == TYPE_P1B_e) {
        head_name = "p1b_head.bdl";
        btp_idx = dRes_INDEX_P1_BTP_MABA_B_e;
    } else {
        head_name = "p1c_head.bdl";
        btp_idx = dRes_INDEX_P1_BTP_MABA_C_e;
    }
    J3DModelData* head_model_data_p = dExtNpcMount_acquireModelData("P1", head_name);
    if (head_model_data_p == NULL) {
        DuskLog.warn("[NpcP1] head '{}' unresolvable", head_name);
        return FALSE;
    }
    mpTexture = (J3DAnmTexPattern*)dComIfG_getObjectRes("P1", btp_idx);
    J3DAnmTexPattern* dummy_tex_all_p =
        (J3DAnmTexPattern*)dComIfG_getObjectRes("P1", dRes_INDEX_P1_BTP_P1A_DUMMY_TEX_ALL_e);
    mpHeadModel = mDoExt_J3DModel__create(head_model_data_p, 0x80000, 0x11020002);
    if (mpTexture == NULL || mpHeadModel == NULL) {
        return FALSE;
    }
    if (!mBtp.init(head_model_data_p, dummy_tex_all_p, TRUE, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1)) {
        return FALSE;
    }
    mBtp.init(head_model_data_p, mpTexture, TRUE, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1);
    if (mType == TYPE_P1B_e && strcmp(dComIfGp_getStartStageName(), "sea") == 0 &&
        fopAcM_GetRoomNo(this) == dIsleRoom_OutsetIsland_e &&
        dComIfGp_getStartStageLayer() == 0xA) {
        // the Outset pirate-visit gong stick (layer 0xA) — [N1] acquirer.
        J3DModelData* dora = dExtNpcMount_acquireModelData("P1", "dora_stick.bdl");
        if (dora == NULL) {
            return FALSE;
        }
        mpDoraModel = mDoExt_J3DModel__create(dora, 0x80000, 0x11000002);
        if (mpDoraModel == NULL) {
            return FALSE;
        }
    } else {
        mpDoraModel = NULL;
    }
    for (u16 i = 0; i < model_data_p->getJointNum(); i += 1) {
        if ((i == m_jnt.getHeadJntNum()) || (i == m_jnt.getBackboneJntNum())) {
            model_data_p->getJointNodePointer(i)->setCallBack(daNpc_P1_nodeCallBack1);
        }
    }
    mpMorf->getModel()->setUserArea((uintptr_t)this);
    mAcchCir.SetWall(30.0f, 0.0f);
    mObjAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1, &mAcchCir,
                 fopAcM_GetSpeed_p(this), fopAcM_GetAngle_p(this), fopAcM_GetShapeAngle_p(this));
    if (mType == TYPE_P1B_e && mParam == 3) {
        mObjAcch.SetWallNone();
    }
    return TRUE;
}

BOOL daNpc_P1_c::_delete() {
    dComIfG_resDelete(&mPhs, "P1");
    if (heap != NULL && mpMorf) {
        dExtNpcBm1_stopZelAnime(mpMorf);
    }
    // [N6] HIO child dropped.
    return TRUE;
}

fpc_ProcID daNpc_P1_c::getKajiID() {
    // [N4] pirateship unported — the parent search cannot match; ERROR stands
    // (donor guarded the same way; the wheel sync below is id-gated).
    return fpcM_ERROR_PROCESS_ID_e;
}

BOOL daNpc_P1_c::kaji_anm() {
    if (m671 != 0) {
        if (mAnmNum == 0xA || mAnmNum == 0xB) {
            if (mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
                setAnm(9, -1.0f);
                mKajiTimer = 300;
            } else if (mpMorf->checkFrame(30.0f)) {
                fopAcM_seStart(this, JA_SE_OBJ_PIRATE_WHEEL, 0);
            }
        } else if (mKajiTimer-- < 0) {
            if (mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
                if (!checkAction(&daNpc_P1_c::talkAction)) {
                    if (cM_rndF(1.0f) > 0.5f) {
                        setAnm(0xA, -1.0f);
                    } else {
                        setAnm(0xB, -1.0f);
                    }
                }
            }
        }
        // [N4] donor syncs daKaji_c::setAnm here — kaji unported, id is ERROR.
        return TRUE;
    }
    if (mType == TYPE_P1B_e && mParam == 2) {
        if (!checkAction(&daNpc_P1_c::talkAction)) {
            if (mAnmNum == 0xE && mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f) &&
                --m66C <= 0) {
                setAnm(0xF, -1.0f);
                if (cM_rndF(1.0f) > 0.5f) {
                    m66C = 1;
                } else {
                    m66C = 2;
                }
            } else if (mAnmNum == 0xF) {
                if (mpMorf->checkFrame(mpMorf->getEndFrame() - 1.0f)) {
                    if (--m66C <= 0) {
                        setAnm(0xE, -1.0f);
                        m66C = 4;
                    }
                }
            }
        }
    }
    return FALSE;
}

BOOL daNpc_P1_c::_execute() {
    mHeadAnm.move();
    if (!demo_move()) {
        playTexPatternAnm();
        s8 room_no = fopAcM_GetRoomNo(this);
        int mtrlSndId = mObjAcch.ChkGroundHit() ? dComIfG_Bgsp().GetMtrlSndId(mObjAcch.m_gnd) : 0;
        int reverb = dComIfGp_getReverb(room_no);
        mpMorf->play(&current.pos, mtrlSndId, reverb);

        if (dComIfGp_event_getMode() == 0 || eventInfo.checkCommandTalk()) {
            if (mActionFunc != NULL) {
                (this->*mActionFunc)(NULL);
            }
            m65A = 0;
            mbAttentionFlag = 0;
        } else {
            event_move();
        }
    }
    kaji_anm();
    lookBack();
    setAttentionPos(&attention_info.position);
    if (m670 == 0) {
        fopAcM_posMoveF(this, mStts.GetCCMoveP());
        mObjAcch.CrrPos(dComIfG_Bgsp());
    }
    tevStr.room_no = dComIfG_Bgsp().GetRoomId(mObjAcch.m_gnd);
    tevStr.YukaCol = dComIfG_Bgsp().GetPolyColor(mObjAcch.m_gnd);
    J3DModel* model = mpMorf->getModel();
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    model->setBaseTRMtx(mDoMtx_stack_c::get());
    mCyl.SetC(current.pos);
    dComIfG_Ccsp()->Set(&mCyl);
    return TRUE;
}

BOOL daNpc_P1_c::_draw() {
    static u8 l_snap_idx_tbl[] = {DSNAP_TYPE_NPC_P1A, DSNAP_TYPE_NPC_P1B, DSNAP_TYPE_NPC_P1C};
    if (mType == TYPE_P1A_e && !dComIfGs_isEventBit(WWEV_UNK_0310)) {
        return TRUE;
    }
    J3DModel* pJVar7 = mpMorf->getModel();
    J3DModelData* head_model_data = mpHeadModel->getModelData();
    // [J4] WW-host lighting through the §406 feeders.
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(pJVar7, &tevStr);
    dKyWw_setLightTevColorType(mpHeadModel, &tevStr);
    // [N2] the donor's draw-time BMT swap is subsumed by the parse-time bake;
    // one draw path serves all three types.
    // §880: donor updateDL() split per the §244 recipe #2 (plain McaMorf has
    // no updateDL receiver-side) — modelCalc → anim entries AFTER the calc
    // (the material calc resets the pattern; the recurring Aryll/Grandma
    // blank-face order bug) → setList → entryDL. Head model gets the same
    // split so its btp survives its own calc.
#if WW_PROBE_884
    // H13 cross-actor draw-state: if zl1/ls1 break only when P1 draws, the
    // §880 head-calc-in-draw is the suspect. One line each ~2s per pirate.
    {
        static int s_pp = 0;
        if ((s_pp++ % 120) == 0) {
            DuskLog.info("[WwProbe884] p1 draw-enter type={} pos=({:.0f},{:.0f},{:.0f})",
                         (int)mType, current.pos.x, current.pos.y, current.pos.z);
        }
    }
#endif
    mpMorf->modelCalc();
    mpHeadModel->setBaseTRMtx(pJVar7->getAnmMtx(m_jnt.getHeadJntNum()));
    mpHeadModel->calc();
    mBtp.entry(head_model_data, mBlinkFrame);
    dComIfGd_setList();
    mpMorf->entryDL();
    mDoExt_modelEntryDL(mpHeadModel);
    if (mpDoraModel) {
        dKyWw_setLightTevColorType(mpDoraModel, &tevStr);
        mpDoraModel->setBaseTRMtx(pJVar7->getAnmMtx(m_handR_jnt_num));
        mDoExt_modelUpdateDL(mpDoraModel);
    }
    cXyz local_28(current.pos.x, current.pos.y + 130.0f, current.pos.z);
    // §880: receiver setShadow 13-arg form — 3 trailing shadow-tex params,
    // standard actor values (ls1:2347 / d_a_kb / alink precedent; §823 tex law).
    mShadowId = dComIfGd_setShadow(mShadowId, true, mpMorf->getModel(), &local_28, 800.0f, 20.0f,
                                   current.pos.y, mObjAcch.GetGroundH(), mObjAcch.m_gnd, &tevStr,
                                   0, 1.0f, dDlst_shadowControl_c::getSimpleTex());
    dSnap_RegistFig(l_snap_idx_tbl[mType], this, 1.0f, 1.0f, 1.0f);
    return TRUE;
}

BOOL daNpc_P1_c::lookBack() {
    u32 o_retval = false;
    cXyz posdiff = (dComIfGp_getPlayer(0)->current.pos - current.pos);
    cXyz dstPos;
    cXyz* dstPos_p;
    f32 fVar4 = posdiff.absXZ();
    bool look_at_target = true;

    if (m671 == 1) {
        if (mAnmNum == 9 && checkAction(&daNpc_P1_c::talkAction)) {
            dstPos = dNpc_playerEyePos(0.0f);
            dstPos_p = &dstPos;
        } else {
            dstPos_p = NULL;
        }
    } else if (mbAttentionFlag != 0) {
        dstPos = mEventCut6B0.getAttnPos();
        dstPos_p = &dstPos;
    } else if (fVar4 < l_prm[mType].mMaxTalkDist || checkAction(&daNpc_P1_c::talkAction) ||
               checkAction(&daNpc_P1_c::explainAction) ||
               checkAction(&daNpc_P1_c::speakAction) ||
               checkAction(&daNpc_P1_c::p1c_speakAction)) {
        dstPos = dNpc_playerEyePos(0.0f);
        dstPos_p = &dstPos;
    } else {
        dstPos_p = NULL;
    }
    m_jnt.setParam(l_prm[mType].mMaxBackboneX, l_prm[mType].mMaxBackboneY,
                   l_prm[mType].mMinBackboneX, l_prm[mType].mMinBackboneY,
                   l_prm[mType].mMaxHeadX, l_prm[mType].mMaxHeadY, l_prm[mType].mMinHeadX,
                   l_prm[mType].mMinHeadY, l_prm[mType].mMaxTurnStep);
    if (m_jnt.trnChk()) {
        cLib_addCalcAngleS2(&mMaxLookVel, l_prm[mType].mLookBackTargetY, 4, 0x800);
        look_at_target = false;
        o_retval = true;
    } else {
        mMaxLookVel = 0;
    }
    cXyz srcpos(current.pos.x, current.pos.y + 190.0f, current.pos.z);
    m_jnt.lookAtTarget(&current.angle.y, dstPos_p, srcpos, current.angle.y, mMaxLookVel,
                       look_at_target);
    return o_retval;
}

static BOOL daNpc_P1_Execute(void* i_this) {
    ((daNpc_P1_c*)i_this)->_execute();
    return TRUE;
}

static BOOL daNpc_P1_IsDelete(void*) {
    return TRUE;
}

static BOOL daNpc_P1_Delete(void* i_this) {
    return ((daNpc_P1_c*)i_this)->_delete();
}

static cPhs_Step daNpc_P1_Create(void* i_this) {
    return ((daNpc_P1_c*)i_this)->_create();
}

static BOOL daNpc_P1_Draw(void* i_this) {
    return ((daNpc_P1_c*)i_this)->_draw();
}

static actor_method_class l_daNpc_P1_Method = {
    (process_method_func)daNpc_P1_Create,
    (process_method_func)daNpc_P1_Delete,
    (process_method_func)daNpc_P1_Execute,
    (process_method_func)daNpc_P1_IsDelete,
    (process_method_func)daNpc_P1_Draw,
};

extern actor_process_profile_definition g_profile_NPC_P1;

actor_process_profile_definition g_profile_NPC_P1 = {
    // donor g_profile_NPC_P1 (d_a_npc_p1.cpp:1433).
    fpcLy_CURRENT_e,           // Layer ID
    7,                         // List ID (donor 0x0007)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_NPC_P1_e,            // Proc Name
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daNpc_P1_c),        // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_NPC_P1_e,          // Draw Prio
    &l_daNpc_P1_Method,        // Actor SubMtd
    0x07 | fopAcStts_SHOWMAP_e | fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e |
        fopAcStts_UNK40000_e,  // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_0_e,         // Cull Type
};
