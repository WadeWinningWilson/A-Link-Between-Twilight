#ifndef D_A_VRBOX_H
#define D_A_VRBOX_H

#include "d/d_com_inf_game.h"

/**
 * @ingroup actors-unsorted
 * @class vrbox_class
 * @brief VR Box
 *
 * @details
 *
 */
class vrbox_class : public fopAc_ac_c {
public:
    /* 0x568 */ u8 unk_0x568[0x56C - 0x568];
    /* 0x56C */ J3DModel* mpSoraModel;
    /* 0x570 */ u8 unk_0x570[0x574 - 0x570];
    /* 0x574 */ u8 field_0x574;
#if TARGET_PC
    // ========================================================================
    // §418 WW NATIVE SKYDOME: donor d_a_vrbox owns vr_sky.bdl (WW
    // d_a_vrbox.cpp:163-195). PC-only append, same precedent as
    // dKy_tevstr_c::mWwColorK1 (§407).
    // ========================================================================
    J3DModel* mpWwSky;
#endif
};

#if !TARGET_PC
STATIC_ASSERT(sizeof(vrbox_class) == 0x578);
#endif

#endif /* D_A_VRBOX_H */
