#ifndef D_A_NPC_JB1_H
#define D_A_NPC_JB1_H

// ============================================================
// WW JABUN (daNpc_Jb1_c) — donor include/d/actor/d_a_npc_jb1.h mirrored
// field-for-field on the ported fopNpc framework (§239). Port: tale §822.
// ============================================================

#include "f_op/f_op_actor.h"
#include "m_Do/m_Do_ext.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_npc.h"

class daNpc_Jb1_c : public fopNpc_npc_c {
public:
    struct anm_prm_c {
        s8 field_0x0;
        s8 field_0x1;
        f32 mMorf;
        f32 mPlaySpeed;
        int mLoopMode;
    };

    typedef BOOL (daNpc_Jb1_c::*ActionFunc)(void*);

    daNpc_Jb1_c();
    bool init_JB1_0();
    bool createInit();
    void setMtx();
    void playBrkAnm(J3DAnmTevRegKey*, s16*);
    int anmNum_toResID(int);
    BOOL setAnm_anm(anm_prm_c*);
    BOOL setAnm();
    void chg_anmTag();
    void control_anmTag();
    void chg_anmAtr(u8);
    void control_anmAtr();
    void setAnm_ATR();
    void anmAtr(u16);
    void eventOrder();
    void checkOrder();
    u16 next_msgStatus(u32*);
    u32 getMsg_JB1_0();
    u32 getMsg();
    bool chkAttention();
    void setAttention();
    bool charDecide(int);
    void event_actionInit(int);
    bool event_action();
    void privateCut(int);
    int isEventEntry();
    void event_proc(int);
    BOOL set_action(ActionFunc, void*);
    void setStt(s8);
    BOOL wait_1();
    BOOL wait_action1(void*);
    bool demo();
    BOOL _draw();
    BOOL _execute();
    BOOL _delete();
    cPhs_Step _create();   // port: cPhs_State -> cPhs_Step (ls1 dialect)
    J3DModelData* create_Anm();
    bool create_lgt();
    BOOL CreateHeap();

public:
    // donor 0x6C4-0x923 (padding fields dropped — the receiver mirrors
    // MEMBERS, not offsets; donor names kept).
    request_of_phase_process_class mPhs;
    s8 m_cse_jnt_num;
    cXyz field_0x6D0;
    LIGHT_INFLUENCE field_0x7F8;
    J3DAnmTevRegKey* m_lmp_brk;
    mDoExt_brkAnm mBrkAnm;
    s16 field_0x834;
    J3DModel* m_lgt_mdl;
    J3DAnmTevRegKey* m_lgt_brk;
    mDoExt_brkAnm mBrkAnm2;
    s16 field_0x858;
    ActionFunc mCurrActionFunc;
    dNpc_EventCut_c mEventCut;
    cXyz mCurrentPos;
    csXyz mCurrentRot;
    cXyz field_0x8F4;
    f32 mFrame;
    s8 field_0x908;
    bool field_0x909;
    bool field_0x90C;
    bool field_0x90D;
    bool field_0x90E;
    bool field_0x90F;
    BOOL field_0x910;
    bool mHasAttention;
    s8 field_0x915;
    bool field_0x917;
    s8 mActIdx;
    s8 mActNo;
    u8 field_0x91A;
    u8 field_0x91B;
    s8 field_0x91C;
    s8 field_0x91D;
    s8 field_0x91E;
    s8 field_0x920;
    s8 field_0x921;
    s8 field_0x922;
    s8 field_0x923;
};

#endif /* D_A_NPC_JB1_H */
