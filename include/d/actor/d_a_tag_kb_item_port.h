#ifndef D_A_TAG_KB_ITEM_PORT_H
#define D_A_TAG_KB_ITEM_PORT_H

// ============================================================
// WW PIG DIG-ITEM TAG (daTagKbItemPort_c) — donor header mirrored (port
// §864 batch). Named *_port to coexist with the §225 shim STUB class until
// the land step retires the stub + the 0xFFFE sentinel (then the pig's
// kb_dig call site re-types to this class in the same change).
// ============================================================

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_phase.h"

class daTagKbItemPort_c : public fopAc_ac_c {
public:
    void kb_dig(fopAc_ac_c*);
    void dig_main();
    bool _delete();
    void CreateInit();
    cPhs_Step _create();
    bool _execute();
    bool _draw();

    u8 field_0x298;    // dug latch
    u8 field_0x299;    // dig-pending flag
    s32 field_0x29c;   // item save bit
    u8 field_0x2a0;    // item no (0xFF = none)
    u8 field_0x2a1;    // pig home-angle payload (0xFF = none)
    s32 field_0x2a4;   // switch (0xFF = none)
    fopAc_ac_c* mpActor;
};

#endif /* D_A_TAG_KB_ITEM_PORT_H */
