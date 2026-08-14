#ifndef D_A_OBJ_PLANT_H
#define D_A_OBJ_PLANT_H

// ============================================================
// WW POTTED PLANT (daObjPlant_c) — donor header mirrored (port §837).
// ============================================================

#include "f_op/f_op_actor.h"
#include "d/d_cc_d.h"
#include "SSystem/SComponent/c_phase.h"

class daObjPlant_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    BOOL _delete();
    BOOL _draw();
    BOOL _execute();

    BOOL CreateHeap();
    void CreateInit();
    void set_mtx();

public:
    request_of_phase_process_class mPhase;
    J3DModel* mpModel;
    dCcD_Stts mStts;
    dCcD_Cyl mCyl;
    s16 field_0x408;   // current sway yaw
    s16 field_0x40A;   // sway phase
    s16 mHitTimer;
    s16 field_0x40E;   // hit-direction yaw
    u8 field_0x410;    // swaying flag
};

#endif /* D_A_OBJ_PLANT_H */
