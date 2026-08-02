#ifndef D_A_KNOB00_H
#define D_A_KNOB00_H

// ============================================================================
// §328 WW KNOB00 — NATIVE donor port vs §27 stand-in, compile-gated.
//
// The donor daKnob00_c (WW DP d_a_knob00, dDoor_info_c-based, ~fully verbatim)
// and the §27 port-wired stand-in (DoorK10 + ExtNpc warp backend, consumed by
// src/d/d_ext_npc_doors.cpp) share the SAME donor symbol names — daKnob00_c,
// g_profile_KNOB00, fpcNm_KNOB00_e (0x31C) — per the no-silent-rename rule.
// Exactly ONE branch may compile. Default = 0 (stand-in; zero behavior change).
//
// Flipping DUSK_WW_KNOB00_NATIVE to 1 compiles the donor-verbatim actor instead
// and REQUIRES the integration seams listed in the §328 report to be decided
// first (d_ext_npc_doors.cpp calls the stand-in-only setDoorKey()/doorKey()/
// spawnSrc() API and drives the warp on demo end — see report).
// ============================================================================
#ifndef DUSK_WW_KNOB00_NATIVE
// §329 (user order 2026-08-01): native ON — the donor daKnob00_c owns WW host
// doors; the §27 stand-in remains below as the gate-0 fallback/kill switch.
#define DUSK_WW_KNOB00_NATIVE 1
#endif

#if DUSK_WW_KNOB00_NATIVE

// ============================================================================
// §328 donor-verbatim header (D:/XXXXXXX/WW DP/include/d/actor/d_a_knob00.h).
// Port adaptations: cPhs_State -> cPhs_Step; include set follows the port's
// d_door.h (which carries the §328 adaptation banners).
// ============================================================================

#include "f_op/f_op_actor.h"
#include "d/d_bg_w.h"
#include "m_Do/m_Do_ext.h"
#include "d/d_door.h"

class daKnob00_c : public dDoor_info_c {
public:
    bool checkFlag(unsigned short bit) {
        return m312 & bit;
    }
    void nextAction() {
        mAction++;
    }
    void offFlag(unsigned short bit) {
        m312 &= ~bit;
    }
    void onFlag(unsigned short bit) {
        m312 |= bit;
    }
    void setAction(unsigned char action) {
        mAction = action;
    }

    BOOL CreateHeap();
    u8 getShapeType();
    void setEventPrm();
    u8 getType2();
    s32 chkPassward();
    BOOL msgDoor();
    void openInit(int);
    BOOL openProc(int);
    void openEnd();
    BOOL chkException();
    void calcMtx();
    BOOL CreateInit();
    cPhs_Step create();  // §328 cPhs_State -> cPhs_Step
    void setStart(float, float);
    void setAngle();
    BOOL adjustmentProc();
    BOOL demoProc();
    BOOL demoProc2();
    BOOL actionWait();
    BOOL actionDemo();
    BOOL actionTalk();
    BOOL actionTalkWait();
    BOOL actionPassward2();
    BOOL actionVilla();
    BOOL actionPassward();
    BOOL actionInit();
    BOOL actionFigure();
    BOOL actionDead();
    BOOL draw();
    inline BOOL execute();

    static const char M_arcname[];

public:
    /* 0x2D0 */ dDoor_msg_c m2D0;
    /* 0x2DC */ request_of_phase_process_class mPhase;
    /* 0x2E4 */ J3DModel* mpModel;
    /* 0x2E8 */ mDoExt_bckAnm mBckAnm;
    /* 0x2F8 */ J3DModel* mpModel2;
    /* 0x2FC */ s8 m_jnt;
    /* 0x2FD */ u8 m2FD[0x300 - 0x2FD];
    /* 0x300 */ dBgW* mpBgW;
    /* 0x304 */ dDoor_stop_c mStopBars;
    /* 0x310 */ u8 mAction;
    /* 0x311 */ u8 m311[0x312 - 0x311];
    /* 0x312 */ u16 m312;
    /* 0x314 */ s16 m314;
    /* 0x316 */ u8 m316;
    /* 0x317 */ u8 mDoorType;

    // ========================================================================
    // §329 PORT EXTENSION (metadata only — no donor behavior change): the §27
    // door NETWORK stamps every host door with a key/src so the port's warp
    // backend (dExtNpcDoors) can resolve cross-stage destinations. The stand-in
    // carried these; the native actor carries the SAME surface so
    // d_ext_npc_doors.cpp compiles unchanged against either gate branch. A
    // native DZR door with no stamp behaves donor-pure (empty key ⇒ no warp
    // hook — closeEndCom restart-room only, exactly the donor).
    // ========================================================================
    void setDoorKey(const char* key);
    const char* doorKey() const { return mDoorKey; }
    const char* spawnSrc() const { return mSpawnSrc; }

    /* port */ char mDoorKey[32];
    /* port */ char mSpawnSrc[96];
}; // size = 0x318 (donor; offsets are donor-relative comments only on this port)

#else  // DUSK_WW_KNOB00_NATIVE == 0 — §27 stand-in (unchanged below)

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

#endif  // DUSK_WW_KNOB00_NATIVE

#endif /* D_A_KNOB00_H */
