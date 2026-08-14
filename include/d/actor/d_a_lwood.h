#ifndef D_A_LWOOD_H
#define D_A_LWOOD_H

// ============================================================
// WW NORMAL TREE (daLwood_c) — donor header mirrored (port §861 batch).
// ============================================================

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_phase.h"

class dBgW;

class daLwood_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    bool _delete();
    bool _execute();
    bool _draw();
    BOOL CreateHeap();
    void CreateInit();
    void set_mtx();
    void setMoveBGMtx();

    s16 getYureTimer() const { return mTimer; }
    f32 getYureScale() const { return mScale; }

    static const char m_arcname[6];

public:
    request_of_phase_process_class mPhs;
    J3DModel* mModel;
    dBgW* mpBgW;
    Mtx mtx;
    f32 mScale;
    s16 mTimer;
};

#endif /* D_A_LWOOD_H */
