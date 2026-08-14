#ifndef D_EXT_PLUGIN_WW_TSUBO_DATA_H
#define D_EXT_PLUGIN_WW_TSUBO_DATA_H

// ============================================================
// WW tsubo port — data layer (§803; the WW_ITEM §740 shape).
// Structs mirror the DONOR's daTsubo::Act_c tables FIELD-FOR-FIELD
// (WW DP d_a_tsubo.h:55-157); rows are GENERATED donor-verbatim
// (ww_tsubo_data.inc via gen_tsubo_data.py — regenerate, never edit).
// ============================================================

#include "dolphin/types.h"

struct WwTsuboData {
    /* 0x00 */ f32 mGravity;
    /* 0x04 */ f32 m04;
    /* 0x08 */ f32 mAttnY;
    /* 0x0C */ f32 mModelScale;
    /* 0x10 */ u32 m10;
    /* 0x14 */ f32 m14;
    /* 0x18 */ f32 m18;
    /* 0x1C */ f32 m1C;
    /* 0x20 */ f32 m20;
    /* 0x24 */ f32 m24;
    /* 0x28 */ u16 m28;   // u16 mirrors: donor writes full bit patterns (0xF63C)
    /* 0x2A */ u16 m2A;
    /* 0x2C */ u16 m2C;
    /* 0x30 */ f32 m30;
    /* 0x34 */ f32 m34;
    /* 0x38 */ f32 m38;
    /* 0x3C */ f32 m3C;
    /* 0x40 */ f32 m40;
    /* 0x44 */ f32 m44;
    /* 0x48 */ f32 m48;
    /* 0x4C */ f32 m4C;
    /* 0x50 */ f32 m50;
    /* 0x54 */ u16 m54;
    /* 0x56 */ u16 m56;
    /* 0x58 */ f32 m58;
    /* 0x5C */ f32 m5C;
    /* 0x60 */ u8 m60;
    /* 0x61 */ u8 m61;
    /* 0x62 */ u8 m62;
    /* 0x63 */ u8 m63;
    /* 0x64 */ u8 m64;
    /* 0x65 */ u8 m65;
    /* 0x68 */ u32 mFlag;          // donor DataFlag_e set
    /* 0x6C */ u16 m6C;
    /* 0x6E */ u8 m6E;
    /* 0x6F */ u8 mAcchCirRad;
    /* 0x70 */ u8 m70;
    /* 0x71 */ u8 mAcchRoofHeight;
    /* 0x72 */ u8 mAttnDist;
    /* 0x74 */ struct { f32 x, y, z; } mParticleScale;   // donor Vec m74
    /* 0x80 */ u32 m80;
    /* 0x84 */ s32 mSoundID_Break;
    /* 0x88 */ s32 m88;
    /* 0x8C */ s32 mSoundID_FallLava;
    /* 0x90 */ s32 mSoundID_FallWater;
    /* 0x94 */ s32 mSoundID_Hit;
    /* 0x98 */ s16 mCullSphX_Move;
    /* 0x9A */ s16 mCullSphY_Move;
    /* 0x9C */ s16 mCullSphZ_Move;
    /* 0x9E */ s16 mCullSphR_Move;
    /* 0xA0 */ u8 mA0[4];             // donor pad, row-initialized {0,0,0,0}
    /* 0xA4 */ s16 mCullSphX_Draw;
    /* 0xA6 */ s16 mCullSphY_Draw;
    /* 0xA8 */ s16 mCullSphZ_Draw;
    /* 0xAA */ s16 mCullSphR_Draw;
    /* 0xAC */ u8 mAC[4];             // donor pad, row-initialized
    /* 0xB0 */ f32 mB0;
    /* 0xB4 */ u8 mB4[4];             // donor pad, row-initialized
    /* 0xB8 */ u32 mHeapSize;
    /* 0xBC */ f32 mBC;
    /* 0xC0 */ f32 mC0;
    /* 0xC4 */ f32 mC4;
    /* 0xC8 */ f32 mC8;
};

struct WwTsuboSpecBoko {
    s16 m00;
    s16 m02;
    f32 m04;
    f32 m08;
};

struct WwTsuboAttrSpine {
    f32 m00;
    f32 m04;
    f32 m08;
    f32 m0C;
    f32 m10;
    f32 m14;
    s16 m18;
    s16 m1A;
    s16 m1C;
    s16 m1E;
    f32 m20;
    f32 m24;
    f32 m28;
    f32 m2C;
    s16 m30;
};

// Accessors — donor subtype space only (0..15; the 12 l_objectName args map
// into it per WW d_stage.cpp:519-530). OOB refuses (№31-C).
const WwTsuboData* dWwTsuboData_get(u32 i_subtype);
const char* dWwTsuboData_getArcName(u32 i_subtype);
// donor M_data_spec_boko (3 rows) + M_attrSpine singletons for the actor TU.
const WwTsuboSpecBoko* dWwTsuboData_getSpecBoko(int i_idx);
const WwTsuboAttrSpine* dWwTsuboData_getAttrSpine();

#endif /* D_EXT_PLUGIN_WW_TSUBO_DATA_H */
