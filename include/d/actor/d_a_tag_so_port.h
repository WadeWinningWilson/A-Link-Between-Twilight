#ifndef D_A_TAG_SO_PORT_H
#define D_A_TAG_SO_PORT_H

// ============================================================
// WW SO-TAG (daTag_So_c) — donor header mirrored (port §871 batch).
// NpcSo's zone marker: zone id + radius + fan flag from params; consumers
// search by proc name (NpcSo binds when it ports).
// ============================================================

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_phase.h"

class daTag_So_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    bool _delete();
    bool _execute();
    bool _draw();
    void getArg();

    u8 m290;       // zone id
    f32 mRadius;   // 100-unit steps; 0xFF param = 1600.0f
    u8 m298;       // fan flag
};

#endif /* D_A_TAG_SO_PORT_H */
