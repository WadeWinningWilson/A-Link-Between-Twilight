#ifndef D_A_ESA_H
#define D_A_ESA_H

// §224 Pass 2 — direct source port of d_a_esa (All-Purpose Bait, 餌). Struct is
// verbatim from the donor; fields accessed by NAME so the layout is base-size
// agnostic (same JSystem/f_op lineage). mPhase added for the mod-folder arc load
// (donor read esa.bdl from the resident Link.arc; the port loads it from the
// extracted Esa.arc via the ext resLoad path — covenant: mod-folder bytes).

#include "f_op/f_op_actor.h"
#include "d/d_particle.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"

class esa_class : public fopAc_ac_c {
public:
    /* 0x290 */ u8 field_0x290[0x298 - 0x290];
    /* 0x298 */ u8 field_0x298;  // claim slot (0 = unclaimed; a pig writes its id)
    /* 0x29C */ f32 mGroundHeight;
    /* 0x2A0 */ s8 mActionState;
    /* 0x2A1 */ s8 mState;        // 0=falling, 1=ground-available, 2=water
    /* 0x2A2 */ u8 field_0x2A2[0x2A4 - 0x2A2];
    /* 0x2A4 */ u8 field_0x2A4[0x2B8 - 0x2A4];  // §224 was dPa_rippleEcallBack (ripple no-op'd)
    /* 0x2B8 */ u8 field_0x2B8;
    /* 0x2B9 */ u8 field_0x2B9;
    /* 0x2BA */ u8 field_0x2BA;
    /* 0x2BC */ J3DModel* mpModel;
    /* 0x2C0 */ s16 mTimer[2];
    request_of_phase_process_class mPhase;  // §224 mod-arc load
};

#endif /* D_A_ESA_H */
