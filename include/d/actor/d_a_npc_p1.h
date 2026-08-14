#ifndef D_A_NPC_P1_H
#define D_A_NPC_P1_H

// ============================================================
// WW PIRATES — GONZO, SENZA & NUDGE (daNpc_P1_c, arc P1) — donor header
// mirrored on the ported fopNpc framework (§239). Port: tale §858.
// Types: P1A Gonzo · P1B Senza · P1C Nudge (param bits 0x14/4).
// ============================================================

#include "f_op/f_op_actor.h"
#include "m_Do/m_Do_ext.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_npc.h"
#include "d/d_cc_d.h"

class daNpc_P1_c : public fopNpc_npc_c {
public:
    enum Type_e {
        TYPE_P1A_e,
        TYPE_P1B_e,
        TYPE_P1C_e,
    };

    enum ActionStatus_e {
        ACTION_STARTING_e = 0,
        ACTION_ONGOING_e = 1,
        ACTION_ENDING_e = 9,
    };

    typedef BOOL (daNpc_P1_c::*ActionFunc)(void*);

    void setAnimFromMsg();
    BOOL setAnm(int, f32);
    BOOL normalAction(void*);
    BOOL confuseAction(void*);
    BOOL talkAction(void*);
    BOOL p1c_speakAction(void*);
    BOOL speakAction(void*);
    BOOL explainAction(void*);
    u32 getNextMsgNo(int);
    BOOL playTexPatternAnm();
    void demo_end_init();
    BOOL demo_move();
    BOOL event_move();
    BOOL evn_setAnm_init(int);
    BOOL evn_talk_init(int);
    BOOL evn_talk();
    BOOL minigameExplainCut();
    BOOL privateCut();
    BOOL setAttentionPos(cXyz*);
    cPhs_Step _create();
    BOOL CreateHeap();
    BOOL _delete();
    fpc_ProcID getKajiID();
    BOOL kaji_anm();
    BOOL _execute();
    BOOL _draw();
    BOOL lookBack();

    void setAction(ActionFunc func, void* arg, int) {
        if (mActionFunc != func) {
            if (mActionFunc != NULL) {
                mActionStatus = ACTION_ENDING_e;
                (this->*mActionFunc)(arg);
            }
            mPrevAction = mActionFunc;
            mActionFunc = func;
            mActionStatus = ACTION_STARTING_e;
        }
    }
    bool checkAction(ActionFunc func) const { return mActionFunc == func; }

public:
    request_of_phase_process_class mPhs;
    J3DModel* mpHeadModel;
    J3DModel* mpDoraModel;
    J3DAnmTexPattern* mpTexture;
    mDoExt_btpAnm mBtp;
    dNpc_JntCtrl_c m_jnt;
    dNpc_HeadAnm_c mHeadAnm;
    dNpc_EventCut_c mEventCut6B0;
    dCcD_Stts mStts;
    dCcD_Cyl mCyl;
    ActionFunc mActionFunc;
    ActionFunc mPrevAction;
    fpc_ProcID mKajiId;
    u32 mShadowId;
    u32 mParam;
    u32 mCurrMesg;
    u32 mPrevMesg;
    int mActionStatus;
    int mAnmNum;
    int mType;
    s16 m_handR_jnt_num;
    s16 mBlinkTimer;
    s16 mBlinkFrame;
    s16 mKajiTimer;
    s16 mMaxLookVel;
    s16 m66C;
    s8 m65A;
    u8 m670;
    u8 m671;
    u8 mbAttentionFlag;
};

#endif /* D_A_NPC_P1_H */
