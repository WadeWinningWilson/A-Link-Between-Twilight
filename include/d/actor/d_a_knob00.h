#ifndef D_A_KNOB00_H
#define D_A_KNOB00_H

#if TARGET_PC

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_bg_w.h"
#include "m_Do/m_Do_ext.h"

// §27 first port (WW d_a_knob00 → Dusklight). Filename kept for decomp
// traceability. No WW d_door.h inherit — DoorK10 events + ExtNpc warp backend.

class daKnob00_c : public fopAc_ac_c {
public:
    enum {
        ACTION_WAIT = 0,
        ACTION_DEMO = 1,
    };

    int CreateHeap();
    void calcMtx();
    int create();
    int execute();
    int draw();
    int Delete();

    void setDoorKey(const char* key);
    const char* doorKey() const { return mDoorKey; }
    const char* spawnSrc() const { return mSpawnSrc; }

private:
    bool bindEvents();
    int frontCheck() const;
    void setEventPrm();
    int getDemoAction();
    void startOpenAnim(int side);
    bool openAnimDone();
    void demoProc();
    void initOpenDemo();

    /* 0x568 */ request_of_phase_process_class mPhase;
    /* 0x570 */ request_of_phase_process_class mEvPhase;
    /* 0x578 */ J3DModel* mpModel;   // controller (door.bdl)
    /* 0x57C */ J3DModel* mpModel2;  // visual (door_a..h)
    /* 0x580 */ mDoExt_bckAnm mBck;
    /* 0x590 */ dBgW* mpBgW;
    /* 0x594 */ Mtx mBgMtx;
    /* 0x5C4 */ s8 mJoint;
    /* 0x5C5 */ u8 mAction;
    /* 0x5C6 */ u8 mEvBound;
    /* 0x5C7 */ u8 mOpenStarted;
    /* 0x5C8 */ u8 mArcRetained;  // №73 ExtNpc model-cache retain for Knob.arc
    /* 0x5CA */ s16 mEvtFront;
    /* 0x5CA */ s16 mEvtBack;
    /* 0x5CC */ s16 mEvtOrdered;
    /* 0x5D0 */ int mStaffId;
    /* 0x5D4 */ char mDoorKey[32];
    /* 0x5F4 */ char mSpawnSrc[96];
};

#endif  // TARGET_PC

#endif /* D_A_KNOB00_H */
