#ifndef D_A_NPC_BA1_H
#define D_A_NPC_BA1_H

// ============================================================
// §261 WW Link's Grandma (NPC_BA1) DIRECT PORT — header assembled from the
// Foundry port-kit (named draft bodies + §260 rodata_tables.h) cross-checked
// against the WW-DP donor skeleton (include/d/actor/d_a_npc_ba1.h) and the
// landed sibling cutscene NPC ports (ls1 §244 / bm1 §246 / zl1 §254).
//
// BASE-CLASS CORRECTION (History §261): the body-less WW-DP skeleton header
// declared `: public fopAc_ac_c`, but the full m2c bodies call
// talk__12fopNpc_npc_cFi / setCollision__12fopNpc_npc_cFff and the actor's
// vtable is {next_msgStatus,getMsg,anmAtr} == fopNpc_npc_c's. The port's
// fopNpc_npc_c is ALSO offset-exact with ba1's own draft layout
// (m_jnt@0x290, mpMorf@0x330, mObjAcch@0x334, mAcchCir@0x4F8, mStts@0x538,
// mCyl@0x574, mEventCut) — so ba1 derives from fopNpc_npc_c exactly like every
// sibling. All 0x290..0x6C4 cluster accesses in the draft resolve to base
// members; only ba1's own tail (0x6C4+) is declared here.
// ============================================================

#include "d/d_npc.h"
#include "f_op/f_op_actor.h"
#include "m_Do/m_Do_ext.h"            // mDoExt_btpAnm / mDoExt_McaMorf
#include "m_Do/m_Do_hostIO.h"         // mDoHIO_entry_c (HIO param table)

class J3DNode;
class J3DJoint;
class J3DModel;
class J3DAnmTexPattern;

class daNpc_Ba1_c : public fopNpc_npc_c {
public:
    typedef int (daNpc_Ba1_c::*ActionFunc)(void*);

    // §266 anm_prm_c — ENDIANNESS FIX. The §260 tables are Foundry's raw BIG-ENDIAN
    // u32 ground truth (donor PPC). The donor struct packs {s8 anmNum @0, s8 btpNum @1,
    // u16 pad} into the FIRST word, i.e. big-endian word0 0xAABBpppp → anmNum=AA, btpNum=BB.
    // On little-endian x64 that word lands in memory as [pp,pp,BB,AA], so the packed
    // s8 fields must sit at the HIGH offsets (2/3), NOT 0/1. The §261 struct read
    // anmNum@0/btpNum@1 → always got the low pad byte (0x00) → EVERY entry resolved to
    // anmNum 0 (wrong anim) + wrong btpNum (wrong face) → distorted render. The floats
    // (morf/speed) + loop survive because a u32→f32/int reinterpret is endian-consistent
    // on-machine; only the sub-word packed bytes needed reordering. Size stays 0x10, and
    // the raw arrays are UNCHANGED (still memcmp-match §260 — Foundry's fidelity bar).
    //   Verify: a_anm_prm_tbl_4490[1] word0 = 0x01000000 → anmNum 1 (was misread as 0);
    //           entry[9] word0 = 0x090B0000 → anmNum 9, btpNum 0x0B (was 0,0).
    struct anm_prm_c {
        /* 0x00 */ u16 pad00;   // donor pad (LE low bytes of word0)
        /* 0x02 */ s8 mBtpNum;  // donor byte1 → LE offset 2
        /* 0x03 */ s8 mAnmNum;  // donor byte0 → LE offset 3
        /* 0x04 */ f32 mMorf;
        /* 0x08 */ f32 mPlaySpeed;
        /* 0x0C */ int mLoopMode;
    };  // Size: 0x10

    void holdEventOn() { m7F4 = 1; }

    void nodeBa1Control(J3DJoint*, J3DModel*);  // §261 J3DNode*->J3DJoint* (port callback type)
    bool XyCheck_cB(int);
    s16 XyEvent_cB(int);
    bool init_BA1_0();
    bool init_BA1_1();
    bool init_BA1_2();
    bool init_BA1_3();
    bool init_BA1_4();
    bool createInit();
    void setMtx(bool);
    int anmNum_toResID(int);
    int btpNum_toResID(int);
    bool setBtp(bool, int);
    bool iniTexPttrnAnm(bool);
    void plyTexPttrnAnm();
    void setAnm_tex(s8);
    bool setAnm_anm(anm_prm_c*);
    void setAnm_NUM(int, int);
    bool setAnm();
    void chg_anmTag();
    void control_anmTag();
    void chg_anmAtr(u8);
    void control_anmAtr();
    void setAnm_ATR(int);
    void anmAtr(u16);
    void eventOrder();
    void checkOrder();
    bool chk_talk();
    bool chk_drct(float);
    bool chk_partsNotMove();
    void lookBack();
    u16 next_msgStatus(u32*);
    u32 getMsg_BA1_0();
    u32 getMsg_BA1_1();
    u32 getMsg_BA1_3();
    u32 getMsg_BA1_4();
    u32 getMsg();
    bool chkAttention();
    void setAttention(bool);
    fopAc_ac_c* searchByID(fpc_ProcID);
    bool partner_srch_sub(fpcLyIt_JudgeFunc);
    void partner_srch();
    bool check_useFairyArea();
    bool checkCommandTalk();
    bool charDecide(int);
    void eInit_SET_PLYER_GOL_();
    void eInit_PLYER_INI_POS_();
    void eInit_USE_FAIRY_END_();
    void eInit_MOV_POS_();
    void eInit_SET_PLYER_TRN_ANG_();
    void eInit_ACTOR_DRW_CONTROL_(int*, int*);
    void eInit_setEvTimer(int*);
    void eInit_CHK_FAIRY_(int*);
    f32 eInit_prmFloat(float*, float);
    void eInit_SET_EYE_OFF_(float*);
    void eInit_EYE_OFF_ZRO_(float*);
    void eInit_CHK_FAIRY_MOV_1(int*);
    void event_actionInit(int);
    BOOL eMove_MOV_POS_();
    BOOL eMove_CHK_FAIRY_();
    BOOL eMove_EYE_OFF_ZRO_();
    BOOL eMove_CHK_FAIRY_MOV_1();
    BOOL event_action();
    void cut_init_START_TALE1(int);
    BOOL cut_move_START_TALE1();
    void privateCut(int);
    void endEvent();
    int isEventEntry();
    void event_proc(int);
    bool set_action(ActionFunc, void*);
    void setStt(s8);
    BOOL wait_0();
    BOOL wait_1();
    BOOL talk_1();
    BOOL talk_2();
    BOOL wait_2();
    BOOL wait_3();
    BOOL ZZZwai();
    int wait_action1(void*);
    int wait_action2(void*);
    int demo_action1(void*);
    int wait_action3(void*);
    int wait_action4(void*);
    bool demo();
    void shadowDraw();
    BOOL _draw();
    BOOL _execute();
    BOOL _delete();
    cPhs_Step _create();  // §261 WW cPhs_State -> port cPhs_Step
    J3DModelData* create_Anm();
    bool create_itm_Mdl();
    BOOL CreateHeap();

public:
    // ============================================================
    // §261 ba1's own tail (past fopNpc_npc_c base end @0x6C4). Offsets are the
    // WW donor offsets from the port-kit draft; kept as receipts (behavioral
    // acceptance, not byte-match). 0x290..0x6C4 accesses use base members.
    // ============================================================
    /* 0x6C4 */ request_of_phase_process_class mPhs;
    /* 0x6CC */ s8 m_hed_jnt_num;
    /* 0x6CD */ s8 m_bbone_jnt_num;
    /* 0x6CE */ s8 m_footL_jnt_num;
    /* 0x6CF */ u8 m6CF[0x6D0 - 0x6CF];
    /* 0x6D0 */ J3DModel* mpItemModel;   // §261 naming-map mislabeled this "m_hed_jnt_num"; bodies show create_itm_Mdl stores a J3DModel* here
    /* 0x6D4 */ u32 mShadowId;
    /* 0x6D8 */ J3DAnmTexPattern* mpBtpRes;
    /* 0x6DC */ mDoExt_btpAnm mBtpAnm;
    /* 0x6F0 */ u8 mBtpFrame;
    /* 0x6F1 */ u8 m6F1[0x6F2 - 0x6F1];
    /* 0x6F2 */ s16 mBlinkTimer;
    /* 0x6F4 */ ActionFunc mCurrActionFunc;
    /* 0x700 */ u8 m700[0x774 - 0x700];  // §261 base mEventCut is used in place of the WW-layout dNpc_EventCut_c @0x708
    /* 0x774 */ fpc_ProcID mPartnerProcID;
    /* 0x778 */ cXyz mHomePos;
    /* 0x784 */ csXyz mHomeAngle;
    /* 0x78A */ csXyz mShapeAngleTmp;
    /* 0x790 */ cXyz mAttnBasePos;
    /* 0x79C */ cXyz mInitialPos;
    /* 0x7A8 */ u8 m7A8[0x7C0 - 0x7A8];
    /* 0x7C0 */ f32 mPrevMorfFrame;
    /* 0x7C4 */ f32 mAttnPosY;
    /* 0x7C8 */ f32 mEyeOffTarget;
    /* 0x7CC */ csXyz mLookAngle;
    /* 0x7D2 */ s16 mEventIdTable[6];
    /* 0x7DE */ s16 mEventIdx;
    /* 0x7E0 */ s16 mEvTimer;
    /* 0x7E2 */ s16 mSttTimer;
    /* 0x7E4 */ u8 m7E4[0x7E8 - 0x7E4];
    /* 0x7E8 */ s16 mHeadTurnVel;
    /* 0x7EA */ s16 mLookTimer;
    /* 0x7EC */ u8 m7EC[0x7EE - 0x7EC];
    /* 0x7EE */ u8 mMorfAnimStopped;
    /* 0x7EF */ s8 m7EF;
    /* 0x7F0 */ u8 m7F0_movDone;
    /* 0x7F1 */ u8 mPresentItemNo;
    /* 0x7F2 */ u8 mGetReg;
    /* 0x7F3 */ u8 m7F3;
    /* 0x7F4 */ u8 m7F4;          // holdEvent gate (holdEventOn)
    /* 0x7F5 */ u8 m7F5_shapeLock;
    /* 0x7F6 */ u8 m7F6_posMoveSkip;
    /* 0x7F7 */ u8 m7F7_drawSkip;
    /* 0x7F8 */ u8 m7F8;
    /* 0x7F9 */ u8 m7F9_fairyReady;
    /* 0x7FA */ u8 mMsgIdx;
    /* 0x7FB */ u8 m7FB;
    /* 0x7FC */ u8 m7FC_fairyEnd;
    /* 0x7FD */ u8 m7FD;
    /* 0x7FE */ u8 m7FE_inEventMove;
    /* 0x7FF */ u8 m7FF_stageChg;
    /* 0x800 */ u8 m800_initDone;
    /* 0x801 */ u8 m801[0x804 - 0x801];
    /* 0x804 */ fpc_ProcID mPartnerID;
    /* 0x808 */ u8 m808_attnFlag;
    /* 0x809 */ u8 mTalkAccepted;
    /* 0x80A */ u8 mAnmChangeFlag;
    /* 0x80B */ u8 m80B_inDemo;
    /* 0x80C */ u8 mCutIdx;
    /* 0x80D */ s8 mEvtActionNo;
    /* 0x80E */ u8 mAnmAtr;
    /* 0x80F */ u8 mMesgTag;
    /* 0x810 */ s8 mBtpNum;
    /* 0x811 */ s8 mAnmNum;
    /* 0x812 */ s8 mOrderType;
    /* 0x813 */ s8 mSttNum;
    /* 0x814 */ u8 mSttNumOld;
    /* 0x815 */ s8 mLookBackMode;
    /* 0x816 */ u8 mSizeIdx;
    /* 0x817 */ u8 mCharType;
    /* 0x818 */ u8 mPartnerSrchState;
    /* 0x819 */ u8 mMesgAtrOnce;
    /* 0x81A */ char mArcName[4];   // §261 "Ba" (donor arc; §260 stringBase0 +0x35)
};

class daNpc_Ba1_HIO_c : public mDoHIO_entry_c {
public:
    struct hio_prm_c {
        /* 0x00 */ s16 mMaxBackboneX;
        /* 0x02 */ s16 mMaxBackboneY;
        /* 0x04 */ s16 mMinBackboneX;
        /* 0x06 */ s16 mMinBackboneY;
        /* 0x08 */ s16 mMaxHeadX;
        /* 0x0A */ s16 mMaxHeadY;
        /* 0x0C */ s16 mMinHeadX;
        /* 0x0E */ s16 mMinHeadY;
        /* 0x10 */ s16 mMaxTurnStep;
        /* 0x12 */ s16 mCalcAngleTarget;
        /* 0x14 */ f32 mAttPosOffsetY;
        /* 0x18 */ f32 mUseFairyDist0;
        /* 0x1C */ f32 mUseFairyDist1;
        /* 0x20 */ f32 mPlayerEyePosOffsetY;
        /* 0x24 */ f32 mDebugDraw;
    };  // Size: 0x28

    daNpc_Ba1_HIO_c();
    virtual ~daNpc_Ba1_HIO_c() {}

    void genMessage(JORMContext* ctx) {}

public:
    /* 0x04 */ s8 m04;
    /* 0x08 */ s32 m08;
    /* 0x0C */ hio_prm_c mPrm;
};

#endif /* D_A_NPC_BA1_H */
