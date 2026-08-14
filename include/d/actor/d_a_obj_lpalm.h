#ifndef D_A_OBJ_LPALM_H
#define D_A_OBJ_LPALM_H

// ============================================================
// WW PALM TREE (daObjLpalm_c) — donor header mirrored (port §863 batch).
// ============================================================

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_phase.h"

class dBgW;

class daObjLpalm_c : public fopAc_ac_c {
public:
    struct Attr_c {
        u8 flag0;   // execute gate
        u8 flag1;   // draw gate
    };

    cPhs_Step _create();
    bool _delete();
    bool _execute();
    bool _draw();
    BOOL CreateHeap();
    void CreateInit();

    static const Attr_c& attr() { return M_attr; }

    static const char M_arcname[7];
    static const Attr_c M_attr;

public:
    request_of_phase_process_class mPhs;
    J3DModel* mModel;
    dBgW* mpBgW;
    Quaternion mBaseQuat;
    Quaternion mBaseQuatTarget;
    Quaternion mAnmMtxQuat[2];
    s16 mAnimDir[2];
    s16 mAnimWave[2];
};

#endif /* D_A_OBJ_LPALM_H */
