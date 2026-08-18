// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_pt (23 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_pt\asm\d\actor\d_a_pt.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ J3DModelData *unk4;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0x1C];                     /* maybe part of unk4[8]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x24 */ f32 unk24[4];                        /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x34 */ char pad34[0x58];                    /* maybe part of unk24[6]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x8C */ s32 unk8C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8C (receipt f_op_actor.h) */
} J3DModel;                                         /* size >= 0x90 */

typedef struct JAIZelBasic {
    /* 0x0 */ JAIZelBasic *unk0;                    /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0x4 */

typedef struct JORReflexible {
    /* 0x0 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ u8 unk4;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x5 */ u8 unk5;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x6 */ u8 unk6;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
} JORReflexible;                                    /* size >= 0x7 */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ f32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ f32 unk8;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0xC */

struct __vt__10daPt_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daPt_HIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__11cBgS_GndChk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(cBgS_GndChk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(cBgS_GndChk *, s16);
};                                                  /* size = 0x18 */

struct __vt__11cBgS_LinChk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(cBgS_LinChk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(cBgS_LinChk *, s16);
};                                                  /* size = 0x18 */

struct __vt__11dBgS_GndChk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_GndChk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(dBgS_GndChk *, s16);
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ void *(*@64@__dt)(dBgS_GndChk *, s16);
    /* 0x24 */ s32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ s32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ void *(*@76@__dt)(dBgS_GndChk *, s16);
};                                                  /* size = 0x30 */

struct __vt__11dBgS_LinChk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_LinChk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(dBgS_LinChk *, s16);
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ void *(*@88@__dt)(dBgS_LinChk *, s16);
    /* 0x24 */ s32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ s32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ void *(*@100@__dt)(dBgS_LinChk *, s16);
};                                                  /* size = 0x30 */

struct __vt__13cBgS_PolyInfo {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cBgS_PolyInfo *, s16);
};                                                  /* size = 0xC */

struct __vt__13mDoExt_brkAnm {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoExt_brkAnm *, s16);
};                                                  /* size = 0xC */

struct __vt__13mDoExt_btpAnm {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoExt_btpAnm *, s16);
};                                                  /* size = 0xC */

struct __vt__14dBgS_ObjGndChk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_ObjGndChk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(dBgS_ObjGndChk *, s16);
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ void *(*@64@__dt)(dBgS_ObjGndChk *, s16);
    /* 0x24 */ s32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ s32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ void *(*@76@__dt)(dBgS_ObjGndChk *, s16);
};                                                  /* size = 0x30 */

struct __vt__14mDoExt_baseAnm {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoExt_baseAnm *, s16);
};                                                  /* size = 0xC */

struct __vt__15cBgS_GrpPassChk {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cBgS_GrpPassChk *, s16);
};                                                  /* size = 0xC */

struct __vt__15dBgS_GrpPassChk {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(dBgS_GrpPassChk *, s16);
};                                                  /* size = 0xC */

struct __vt__16cBgS_PolyPassChk {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cBgS_PolyPassChk *, s16);
};                                                  /* size = 0xC */

struct __vt__16dBgS_PolyPassChk {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(dBgS_PolyPassChk *, s16);
};                                                  /* size = 0xC */

struct __vt__18dBgS_ObjGndChk_Spl {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_ObjGndChk_Spl *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(dBgS_ObjGndChk_Spl *, s16);
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ void *(*@64@__dt)(dBgS_ObjGndChk_Spl *, s16);
    /* 0x24 */ s32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ s32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ void *(*@76@__dt)(dBgS_ObjGndChk_Spl *, s16);
};                                                  /* size = 0x30 */

struct __vt__8cM3dGLin {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cM3dGLin *, s16);
};                                                  /* size = 0xC */

struct __vt__8dBgS_Chk {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_Chk *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@12@__dt)(dBgS_Chk *, s16);
};                                                  /* size = 0x18 */

typedef struct cBgS_GndChk {
    /* 0x0 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ struct __vt__16cBgS_PolyPassChk **unk4; /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
} cBgS_GndChk;                                      /* size >= 0x8 */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct csXyz {
    /* 0x0 */ s16 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} csXyz;                                            /* size >= 0x2 */

typedef struct dPa_levelEcallBack {
    /* 0x0 */ void *unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dPa_levelEcallBack;                               /* size >= 0x4 */

typedef struct dRes_control_c {
    /* 0x0 */ s8 unk0;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x1 */ char pad1[2];                         /* maybe part of unk0[3]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x3 */ s8 unk3;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3 (receipt f_op_actor.h) */
    /* 0x4 */ char pad4[4];                         /* maybe part of unk3[5]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ s8 unk8;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
} dRes_control_c;                                   /* size >= 0x9 */

typedef struct dStage_roomControl_c {
    /* 0x0 */ u8 unk0;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dStage_roomControl_c;                             /* size >= 0x1 */

typedef struct dSv_info_c {
    /* 0x00000 */ char pad0[0x12A0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x012A0 */ cBgS unk12A0;                     /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x012A0 */ char pad12A0[0x1404];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x026A4 */ cCcS unk26A4;                     /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x026A4 */ char pad26A4[0x2A08];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x050AC */ cDT_NamePTbl unk50AC;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x050AC */ char pad50AC[0xA18];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC4 */ dPa_control_c *unk5AC4;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC8 */ char pad5AC8[0x48];               /* maybe part of unk5AC4[0x13]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B10 */ s32 unk5B10;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B14 */ char pad5B14[0x30];               /* maybe part of unk5B10[0xD]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B44 */ fopAc_ac_c *unk5B44;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B48 */ char pad5B48[0x16478];            /* maybe part of unk5B44[0x591F]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ ? unk1BFC0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ char pad1BFC0[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} dSv_info_c;                                       /* size >= 0x1BFC1 */

typedef struct daPt_HIO_c {
    /* 0x0 */ struct __vt__10daPt_HIO_c *vtable0;   /* inferred */
    /* 0x4 */ s8 unk4;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x5 */ s8 unk5;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x6 */ s8 unk6;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
} daPt_HIO_c;                                       /* size >= 0x7 */

typedef struct fopAc_ac_c {
    /* 0x000 */ char pad0[0xB0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x0B0 */ u32 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
    /* 0x0B4 */ char padB4[0x110];                  /* maybe part of unkB0[0x45]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB4 (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ s32 unk1C8;                         /* inferred */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1CC */ char pad1CC[0x18];                  /* maybe part of unk1C8[7]? */  /* = fopAc_ac_c::fpc_ProcID parentActorID @0x1CC (receipt f_op_actor.h) */
    /* 0x1E4 */ cXyz unk1E4;                        /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1E4 */ char pad1E4[0x14];  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1F8 */ Vec unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1F8 */ char pad1F8[0xC];  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x204 */ s16 unk204;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x206 */ char pad206[0x1A];                  /* maybe part of unk204[0xE]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xE (receipt f_op_actor.h) */
    /* 0x220 */ cXyz unk220;                        /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x220 */ char pad220[0xC];  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x22C */ f32 *unk22C;                        /* inferred */  /* = fopAc_ac_c::MtxP cullMtx @0x22C (receipt f_op_actor.h) */
    /* 0x230 */ char pad230[0x50];                  /* maybe part of unk22C[0x15]? */  /* = fopAc_ac_c::fopAc_cullSizeSphere sphere @0x230 (receipt f_op_actor.h) */
    /* 0x280 */ s32 unk280;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x14 (receipt f_op_actor.h) */
    /* 0x284 */ s8 unk284;                          /* inferred */  /* = fopAc_ac_c::s8 max_health @0x284 (receipt f_op_actor.h) */
    /* 0x285 */ s8 unk285;                          /* inferred */  /* = fopAc_ac_c::s8 health @0x285 (receipt f_op_actor.h) */
    /* 0x286 */ char pad286[2];                     /* maybe part of unk285[3]? */  /* = fopAc_ac_c::s8 health @0x285 +0x1 (receipt f_op_actor.h) */
    /* 0x288 */ s32 unk288;                         /* inferred */  /* = fopAc_ac_c::int itemTableIdx @0x288 (receipt f_op_actor.h) */
    /* 0x28C */ char pad28C[1];  /* = fopAc_ac_c::u8 stealItemBitNo @0x28C (receipt f_op_actor.h) */
    /* 0x28D */ s8 unk28D;                          /* inferred */  /* = fopAc_ac_c::s8 stealItemLeft @0x28D (receipt f_op_actor.h) */
    /* 0x28E */ char pad28E[2];                     /* maybe part of unk28D[3]? */  /* = fopAc_ac_c::s8 stealItemLeft @0x28D +0x1 (receipt f_op_actor.h) */
    /* 0x290 */ u8 unk290;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x291 */ char pad291[0x13];                  /* maybe part of unk290[0x14]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2A4 */ s32 unk2A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2A8 */ char pad2A8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2AC */ request_of_phase_process_class unk2AC; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2AC */ char pad2AC[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B4 */ s8 unk2B4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B5 */ u8 unk2B5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B6 */ s8 unk2B6;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B7 */ u8 unk2B7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B8 */ u8 unk2B8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B9 */ s8 unk2B9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2BA */ char pad2BA[6];                     /* maybe part of unk2B9[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C0 */ mDoExt_McaMorf *unk2C0;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ mDoExt_McaMorf *unk2C4;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C8 */ mDoExt_McaMorf *unk2C8;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2CC */ s32 unk2CC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D0 */ char pad2D0[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D2 */ s16 unk2D2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D4 */ char pad2D4[4];                     /* maybe part of unk2D2[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D8 */ s16 unk2D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2DA */ char pad2DA[0x42];                  /* maybe part of unk2D8[0x22]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x31C */ void *unk31C;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x320 */ char pad320[8];                     /* maybe part of unk31C[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x328 */ dBgS_AcchCir unk328;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x328 */ char pad328[0x40];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x368 */ dBgS_Acch unk368;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x368 */ char pad368[0x1C4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x52C */ dCcD_Stts unk52C;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x52C */ char pad52C[0x3C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x568 */ dCcD_Sph unk568;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x568 */ char pad568[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5AC */ dCcD_Stts *unk5AC;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5B0 */ char pad5B0[0xE4];                  /* maybe part of unk5AC[0x3A]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x694 */ dCcD_Sph unk694;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x694 */ char pad694[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ dCcD_Stts *unk6D8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ char pad6DC[0x108];                 /* maybe part of unk6D8[0x43]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ fopAc_ac_c *unk7E4;                 /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E8 */ char pad7E8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EC */ f32 unk7EC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F0 */ char pad7F0[0x190];                 /* maybe part of unk7EC[0x65]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x980 */ f32 unk980;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x984 */ f32 unk984;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x988 */ char pad988[8];                     /* maybe part of unk984[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x990 */ f32 unk990;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x994 */ char pad994[0x208];                 /* maybe part of unk990[0x83]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB9C */ fopAc_ac_c *unkB9C;                 /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA0 */ char padBA0[8];                     /* maybe part of unkB9C[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA8 */ mDoExt_McaMorf *unkBA8;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBAC */ char padBAC[0x218];                 /* maybe part of unkBA8[0x87]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xDC4 */ s8 unkDC4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} fopAc_ac_c;                                       /* size >= 0xDC5 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ struct __vt__14mDoExt_baseAnm *vtable0; /* inferred */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ char pad14[0x3C];                    /* maybe part of unk10[0x10]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[8];                       /* maybe part of unk50[3]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x5C */ s8 unk5C;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5C (receipt f_op_actor.h) */
    /* 0x5D */ u8 unk5D;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5D (receipt f_op_actor.h) */
    /* 0x5E */ char pad5E[6];                       /* maybe part of unk5D[7]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5E (receipt f_op_actor.h) */
    /* 0x64 */ f32 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoExt_brkAnm {
    /* 0x0 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ void *unk4;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
} mDoExt_brkAnm;                                    /* size >= 0x8 */

typedef struct mDoExt_btpAnm {
    /* 0x0 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ void *unk4;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
} mDoExt_btpAnm;                                    /* size >= 0x8 */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x10 */

typedef struct pt_class {
    /* 0x000 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x004 */ u32 unk4;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x008 */ char pad8[0xA8];                    /* maybe part of unk4[0x2B]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0B0 */ s32 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
    /* 0x0B4 */ char padB4[0x58];                   /* maybe part of unkB0[0x17]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB4 (receipt f_op_actor.h) */
    /* 0x10C */ dKy_tevstr_c unk10C;                /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x10C */ char pad10C[0xB0];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x1BC */ u16 unk1BC;                         /* inferred */  /* = fopAc_ac_c::u16 setID @0x1BC (receipt f_op_actor.h) */
    /* 0x1BE */ char pad1BE[6];                     /* maybe part of unk1BC[4]? */  /* = fopAc_ac_c::u8 group @0x1BE (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ char pad1C8[8];                     /* maybe part of unk1C4[3]? */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1D0 */ f32 unk1D0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 (receipt f_op_actor.h) */
    /* 0x1D4 */ f32 unk1D4;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x4 (receipt f_op_actor.h) */
    /* 0x1D8 */ f32 unk1D8;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x8 (receipt f_op_actor.h) */
    /* 0x1DC */ s16 unk1DC;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0xC (receipt f_op_actor.h) */
    /* 0x1DE */ s16 unk1DE;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0xE (receipt f_op_actor.h) */
    /* 0x1E0 */ s16 unk1E0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x10 (receipt f_op_actor.h) */
    /* 0x1E2 */ u8 unk1E2;                          /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x12 (receipt f_op_actor.h) */
    /* 0x1E3 */ char pad1E3[0x15];                  /* maybe part of unk1E2[0x16]? */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x13 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ s16 unk204;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x206 */ s16 unk206;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xE (receipt f_op_actor.h) */
    /* 0x208 */ s16 unk208;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x10 (receipt f_op_actor.h) */
    /* 0x20A */ u8 unk20A;                          /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x12 (receipt f_op_actor.h) */
    /* 0x20B */ char pad20B[1];  /* = fopAc_ac_c::actor_place current @0x1F8 +0x13 (receipt f_op_actor.h) */
    /* 0x20C */ csXyz unk20C;                       /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
    /* 0x20C */ char pad20C[2];  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
    /* 0x20E */ s16 unk20E;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x2 (receipt f_op_actor.h) */
    /* 0x210 */ s16 unk210;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x212 */ char pad212[2];  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x6 (receipt f_op_actor.h) */
    /* 0x214 */ f32 unk214;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 (receipt f_op_actor.h) */
    /* 0x218 */ f32 unk218;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 +0x4 (receipt f_op_actor.h) */
    /* 0x21C */ char pad21C[4];  /* = fopAc_ac_c::cXyz scale @0x214 +0x8 (receipt f_op_actor.h) */
    /* 0x220 */ f32 unk220;                         /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x224 */ f32 unk224;                         /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 +0x4 (receipt f_op_actor.h) */
    /* 0x228 */ f32 unk228;                         /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 +0x8 (receipt f_op_actor.h) */
    /* 0x22C */ char pad22C[0x28];                  /* maybe part of unk228[0xB]? */  /* = fopAc_ac_c::MtxP cullMtx @0x22C (receipt f_op_actor.h) */
    /* 0x254 */ f32 unk254;                         /* inferred */  /* = fopAc_ac_c::f32 speedF @0x254 (receipt f_op_actor.h) */
    /* 0x258 */ char pad258[8];                     /* maybe part of unk254[3]? */  /* = fopAc_ac_c::f32 gravity @0x258 (receipt f_op_actor.h) */
    /* 0x260 */ Vec unk260;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x260 */ char pad260[4];  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x264 */ f32 unk264;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x4 (receipt f_op_actor.h) */
    /* 0x268 */ f32 unk268;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x8 (receipt f_op_actor.h) */
    /* 0x26C */ char pad26C[8];                     /* maybe part of unk268[3]? */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C (receipt f_op_actor.h) */
    /* 0x274 */ f32 unk274;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x8 (receipt f_op_actor.h) */
    /* 0x278 */ f32 unk278;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0xC (receipt f_op_actor.h) */
    /* 0x27C */ f32 unk27C;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x10 (receipt f_op_actor.h) */
    /* 0x280 */ s32 unk280;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x14 (receipt f_op_actor.h) */
    /* 0x284 */ char pad284[1];  /* = fopAc_ac_c::s8 max_health @0x284 (receipt f_op_actor.h) */
    /* 0x285 */ u8 unk285;                          /* inferred */  /* = fopAc_ac_c::s8 health @0x285 (receipt f_op_actor.h) */
    /* 0x286 */ char pad286[0x26];                  /* maybe part of unk285[0x27]? */  /* = fopAc_ac_c::s8 health @0x285 +0x1 (receipt f_op_actor.h) */
    /* 0x2AC */ request_of_phase_process_class unk2AC; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2AC */ char pad2AC[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B4 */ u8 unk2B4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B5 */ u8 unk2B5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B6 */ u8 unk2B6;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B7 */ u8 unk2B7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B8 */ u8 unk2B8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2B9 */ u8 unk2B9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2BA */ u8 unk2BA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2BB */ char pad2BB[5];                     /* maybe part of unk2BA[6]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C0 */ mDoExt_McaMorf *unk2C0;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ mDoExt_btpAnm *unk2C4;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C8 */ mDoExt_brkAnm *unk2C8;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2CC */ s32 unk2CC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D0 */ s16 unk2D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D2 */ s16 unk2D2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D4 */ s16 unk2D4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D6 */ s16 unk2D6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D8 */ s16 unk2D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2DA */ char pad2DA[0xE];                   /* maybe part of unk2D8[8]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2E8 */ f32 unk2E8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2EC */ f32 unk2EC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F0 */ f32 unk2F0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F4 */ u32 unk2F4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F8 */ Vec *unk2F8;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2FC */ s16 unk2FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2FE */ char pad2FE[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x300 */ f32 unk300;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x304 */ f32 unk304;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x308 */ s16 unk308;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x30A */ s16 unk30A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x30C */ s16 unk30C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x30E */ s16 unk30E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x310 */ s16 unk310;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x312 */ char pad312[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x314 */ f32 unk314;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x318 */ f32 unk318;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x31C */ f32 unk31C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x320 */ f32 unk320;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x324 */ s16 unk324;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x326 */ u8 unk326;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x327 */ u8 unk327;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x328 */ char pad328[0x40];                  /* maybe part of unk327[0x41]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x368 */ dBgS_Acch unk368;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x368 */ char pad368[0x28];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x390 */ s32 unk390;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x394 */ char pad394[0x68];                  /* maybe part of unk390[0x1B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3FC */ f32 unk3FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x400 */ char pad400[0x50];                  /* maybe part of unk3FC[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x450 */ cBgS_PolyInfo unk450;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x450 */ char pad450[0xDC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x52C */ ? unk52C;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x52C */ char pad52C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x530 */ f32 unk530;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x534 */ f32 unk534;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ char pad538[0x10];                  /* maybe part of unk534[5]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x548 */ dCcD_GStts unk548;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x548 */ char pad548[0x20];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x568 */ dCcD_GObjInf unk568;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x568 */ char pad568[0xCC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x634 */ ? unk634;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x634 */ char pad634[0x4C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x680 */ cM3dGSph unk680;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x680 */ char pad680[0x14];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x694 */ dCcD_GObjInf unk694;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x694 */ char pad694[0x118];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AC */ cM3dGSph unk7AC;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AC */ char pad7AC[0x14];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C0 */ u8 unk7C0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C1 */ char pad7C1[3];                     /* maybe part of unk7C0[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C4 */ dPa_levelEcallBack unk7C4;          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C4 */ char pad7C4[0x20];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ enemyice unk7E4;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ char pad7E4[6];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EA */ s8 unk7EA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EB */ char pad7EB[0x3B1];                 /* maybe part of unk7EA[0x3B2]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB9C */ enemyfire unkB9C;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB9C */ char padB9C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA0 */ s16 unkBA0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA2 */ char padBA2[0x222];                 /* maybe part of unkBA0[0x112]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xDC4 */ u8 unkDC4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} pt_class;                                         /* size >= 0xDC5 */

u32 ChkAtHit__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
u32 ChkTgHit__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
? CrrPos__9dBgS_AcchFR4dBgS(dBgS_Acch *this, dBgS *arg0); /* extern */
void *GetAtHitObj__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
s32 GetIndex__12cDT_NamePTblCFPCci(cDT_NamePTbl *this, s8 *arg0, s32 arg1); /* extern */
void *GetTgHitObj__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
f32 GroundCross__4cBgSFP11cBgS_GndChk(cBgS *this, cBgS_GndChk *arg0); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
u8 LineCross__4cBgSFP11cBgS_LinChk(cBgS *this, cBgS_LinChk *arg0); /* extern */
? Move__10dCcD_GSttsFv(dCcD_GStts *this);           /* extern */
? MtxPosition__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
? MtxPull__Fv();                                    /* extern */
? MtxPush__Fv();                                    /* extern */
? MtxTrans__FfffUc(f32 arg0, f32 arg1, f32 arg2, u8 arg3); /* extern */
? PSMTXCopy(mDoMtx_stack_c *, f32 (*)[4]);          /* extern */
mDoMtx_stack_c *PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32); /* extern */
? PSVECAdd(cXyz *, f32 *, cXyz *);                  /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetC__8cM3dGSphFRC4cXyz(cM3dGSph *this, cXyz *arg0); /* extern */
? SetR__8cM3dGSphFf(cM3dGSph *this, f32 arg0);      /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c(dBgS_LinChk *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2); /* extern */
? Set__4cCcSFP8cCcD_Obj(cCcS *this, cCcD_Obj *arg0); /* extern */
? Set__8dCcD_SphFRC11dCcD_SrcSph(dCcD_Sph *this, dCcD_SrcSph *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
void *__ct__11cBgS_GndChkFv(cBgS_GndChk *this);     /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
void *__dt__8cBgS_ChkFv(cBgS_Chk *this, s16 destroyFlag); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
? __register_global_object(void *(*)(daPt_HIO_c *, s16), void *(*)(cXyz *, s16), void *, f32); /* extern */
? cLib_addCalc0__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
? cLib_addCalc2__FPffff(f32 *arg0, f32 arg1, f32 arg2, f32 arg3); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
s16 cM_atan2s__Fff(f32 arg0, f32 arg1);             /* extern */
f32 cM_rndFX__Ff(f32 arg0);                         /* extern */
f32 cM_rndF__Ff(f32 arg0);                          /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
? cc_at_check__FP10fopAc_ac_cP8CcAtInfo(fopAc_ac_c *arg0, CcAtInfo *arg1); /* extern */
u8 createChild__16mDoHIO_subRoot_cFPCcP13JORReflexible(mDoHIO_subRoot_c *this, s8 *arg0, JORReflexible *arg1); /* extern */
? ct__11cBgS_LinChkFv(cBgS_LinChk *this);           /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s32 dComIfG_resLoad__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
u32 dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(u32 arg0, s8 arg1, J3DModel *arg2, cXyz *arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, cBgS_PolyInfo *arg8, dKy_tevstr_c *arg9, s16 arg10, f32 arg11, _GXTexObj *arg12); /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cfff(u8 arg0, fopAc_ac_c *arg1, f32 arg2, f32 arg3, f32 arg4); /* extern */
u8 daSea_ChkArea__Fff(f32 arg0, f32 arg1);          /* extern */
f32 daSea_calcWave__Fff(f32 arg0, f32 arg1);        /* extern */
? deleteChild__16mDoHIO_subRoot_cFSc(mDoHIO_subRoot_c *this, s8 arg0); /* extern */
? enemy_fire__FP9enemyfire(enemyfire *arg0);        /* extern */
? enemy_fire_remove__FP9enemyfire(enemyfire *arg0); /* extern */
s32 enemy_ice__FP8enemyice(enemyice *arg0);         /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_brkAnmFP12J3DModelDataf(mDoExt_brkAnm *this, J3DModelData *arg0, f32 arg1); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
void *fopAcM_CreateAppend__Fv();                    /* extern */
? fopAcM_createDisappear__FP10fopAc_ac_cP4cXyzUcUcUc(fopAc_ac_c *arg0, cXyz *arg1, u8 arg2, u8 arg3, u8 arg4); /* extern */
? fopAcM_delete__FP10fopAc_ac_c(fopAc_ac_c *arg0);  /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
s16 fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c(fopAc_ac_c *arg0, fopAc_ac_c *arg1); /* extern */
f32 fopAcM_searchActorDistanceXZ__FP10fopAc_ac_cP10fopAc_ac_c(fopAc_ac_c *arg0, fopAc_ac_c *arg1); /* extern */
f32 fopAcM_searchActorDistance__FP10fopAc_ac_cP10fopAc_ac_c(fopAc_ac_c *arg0, fopAc_ac_c *arg1); /* extern */
s32 fopAc_IsActor__FPv(void *arg0);                 /* extern */
? fopKyM_createWpillar__FPC4cXyzffi(cXyz *arg0, f32 arg1, f32 arg2, s32 arg3); /* extern */
u32 fpcEx_Search__FPFPvPv_PvPv(void *(*arg0)(void *, void *), void *arg1); /* extern */
layer_class *fpcLy_CurrentLayer__Fv();              /* extern */
? fpcSCtRq_Request__FP11layer_classsPFPvPv_iPvPv(layer_class *arg0, s16 arg1, s32 (*arg2)(void *, void *), void *arg3, void *arg4); /* extern */
J3DAnmTransform *getRes__14dRes_control_cFPCclP11dRes_info_ci(dRes_control_c *this, s8 *arg0, s32 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
s32 init__13mDoExt_brkAnmFP12J3DModelDataP15J3DAnmTevRegKeyiifssbi(mDoExt_brkAnm *this, J3DModelData *arg0, J3DAnmTevRegKey *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotS__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
mDoMtx_stack_c *mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1); /* extern */
? monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, u32 arg3, s8 arg4); /* extern */
? onActor__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
? onSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
? play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? scaleM__14mDoMtx_stack_cFfff(mDoMtx_stack_c *this, f32 arg0, f32 arg1, f32 arg2); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setAnm__14mDoExt_McaMorfFP15J3DAnmTransformiffffPv(mDoExt_McaMorf *this, J3DAnmTransform *arg0, s32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, void *arg6); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
void *set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(dPa_control_c *this, u8 arg0, u16 arg1, cXyz *arg2, csXyz *arg3, cXyz *arg4, u8 arg5, dPa_levelEcallBack *arg6, s8 arg7, _GXColor *arg8, _GXColor *arg9, cXyz *arg10); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? transM__14mDoMtx_stack_cFfff(mDoMtx_stack_c *this, f32 arg0, f32 arg1, f32 arg2); /* extern */
void *@100@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *@12@__dt__8dBgS_ChkFv(dBgS_Chk *this, s16 destroyFlag); /* static */
void *@20@__dt__11cBgS_GndChkFv(cBgS_GndChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11cBgS_LinChkFv(cBgS_LinChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *@20@__dt__14dBgS_ObjGndChkFv(dBgS_ObjGndChk *this, s16 destroyFlag); /* static */
void *@20@__dt__18dBgS_ObjGndChk_SplFv(dBgS_ObjGndChk_Spl *this, s16 destroyFlag); /* static */
void *@64@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@64@__dt__14dBgS_ObjGndChkFv(dBgS_ObjGndChk *this, s16 destroyFlag); /* static */
void *@64@__dt__18dBgS_ObjGndChk_SplFv(dBgS_ObjGndChk_Spl *this, s16 destroyFlag); /* static */
void *@76@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@76@__dt__14dBgS_ObjGndChkFv(dBgS_ObjGndChk *this, s16 destroyFlag); /* static */
void *@76@__dt__18dBgS_ObjGndChk_SplFv(dBgS_ObjGndChk_Spl *this, s16 destroyFlag); /* static */
void *@88@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *__ct__8pt_classFv(pt_class *this);            /* static */
void *__dt__10daPt_HIO_cFv(daPt_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__11cBgS_GndChkFv(cBgS_GndChk *this, s16 destroyFlag); /* static */
void *__dt__11cBgS_LinChkFv(cBgS_LinChk *this, s16 destroyFlag); /* static */
void *__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *__dt__13cBgS_PolyInfoFv(cBgS_PolyInfo *this, s16 destroyFlag); /* static */
void *__dt__13mDoExt_brkAnmFv(mDoExt_brkAnm *this, s16 destroyFlag); /* static */
void *__dt__13mDoExt_btpAnmFv(mDoExt_btpAnm *this, s16 destroyFlag); /* static */
void *__dt__14dBgS_ObjGndChkFv(dBgS_ObjGndChk *this, s16 destroyFlag); /* static */
void *__dt__14mDoExt_baseAnmFv(mDoExt_baseAnm *this, s16 destroyFlag); /* static */
void *__dt__15cBgS_GrpPassChkFv(cBgS_GrpPassChk *this, s16 destroyFlag); /* static */
void *__dt__15dBgS_GrpPassChkFv(dBgS_GrpPassChk *this, s16 destroyFlag); /* static */
void *__dt__16cBgS_PolyPassChkFv(cBgS_PolyPassChk *this, s16 destroyFlag); /* static */
void *__dt__16dBgS_PolyPassChkFv(dBgS_PolyPassChk *this, s16 destroyFlag); /* static */
void *__dt__18dBgS_ObjGndChk_SplFv(dBgS_ObjGndChk_Spl *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void *__dt__8cM3dGLinFv(cM3dGLin *this, s16 destroyFlag); /* static */
void *__dt__8dBgS_ChkFv(dBgS_Chk *this, s16 destroyFlag); /* static */
s32 daPt_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0);  /* static */
s32 daPt_Delete__FP8pt_class(pt_class *arg0);       /* static */
s32 daPt_Draw__FP8pt_class(pt_class *arg0);         /* static */
s32 daPt_Execute__FP8pt_class(pt_class *arg0);      /* static */
s32 daPt_IsDelete__FP8pt_class(pt_class *arg0);     /* static */
extern f32 (*calc_mtx)[4];
extern dSv_info_c g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? g_regHIO;
extern ? mDoHIO_root;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern dStage_roomControl_c mStayNo__20dStage_roomControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? @2100;                                     /* unable to generate initializer: unknown type */
static ? @3569;
static u8 hio_set;
static JORReflexible l_HIO;

/* daPt_HIO_c::daPt_HIO_c (void) */
void __ct__10daPt_HIO_cFv(daPt_HIO_c *this) {
    this->vtable0 = &__vt__10daPt_HIO_c;
    this->unk4 = -1;
    this->unk5 = 0;
    this->unk6 = 0;
}

/* anm_init (pt_class *, int, float, char unsigned, float, int) */
void anm_init__FP8pt_classifUcfi(pt_class *arg0, s32 arg1, f32 arg2, u8 arg3, f32 arg4, s32 arg5) {
    J3DAnmTransform *temp_r29;

    if (arg5 >= 0) {
        temp_r29 = getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *) arg5, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        setAnm__14mDoExt_McaMorfFP15J3DAnmTransformiffffPv(arg0->unk2C0, getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *) arg1, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */)), (s32) arg3, arg2, arg4, 0.0f, -1.0f, temp_r29);
        return;
    }
    setAnm__14mDoExt_McaMorfFP15J3DAnmTransformiffffPv(arg0->unk2C0, getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *) arg1, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */)), (s32) arg3, arg2, arg4, 0.0f, -1.0f, NULL);
}

/* daPt_Draw (pt_class *) */
s32 daPt_Draw__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r29;
    f32 temp_f3;
    mDoExt_brkAnm *temp_r3_2;
    mDoExt_btpAnm *temp_r3;

    if ((s8) arg0->unk2B9 == 0) {
        temp_r29 = arg0->unk2C0->unk50;
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r29, &arg0->unk10C);
        temp_r3 = arg0->unk2C4;
        entry__13mDoExt_btpAnmFP12J3DModelDatas(temp_r3, temp_r29->unk4, (s16) temp_r3->unk4->unk10);
        temp_r3_2 = arg0->unk2C8;
        entry__13mDoExt_brkAnmFP12J3DModelDataf(temp_r3_2, temp_r29->unk4, temp_r3_2->unk4->unk10);
        entryDL__14mDoExt_McaMorfFv(arg0->unk2C0);
        if ((s8) arg0->unk326 != 0) {
            temp_f3 = arg0->unk1FC;
            sp8 = arg0->unk1F8;
            spC = @4155.unk8 + temp_f3;
            sp10 = arg0->unk200;
            arg0->unk2F4 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(arg0->unk2F4, 1, temp_r29, (cXyz *) &sp8, @4155.unkC, @4155.unk10, temp_f3, arg0->unk3FC, &arg0->unk450, &arg0->unk10C, 0, @4155.unk14, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
            arg0->unk326 = 0;
        }
    }
    dSnap_RegistFig__FUcP10fopAc_ac_cfff(0xABU, (fopAc_ac_c *) arg0, @4155.unk14, @4155.unk14, @4155.unk14);
    return 1;
}

/* smoke_set (pt_class *, char signed) */
void smoke_set__FP8pt_classSc(pt_class *arg0, s8 arg1, ? arg_sp0) {
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    void *temp_r3;

    if ((s8) arg0->unk7C0 == 0) {
        arg0->unk7C4.unk0->unk20(&arg0->unk7C4);
        sp8 = (s32) arg0->unk20A;
        spC = 0;
        sp10 = 0;
        sp14 = 0;
        temp_r3 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 2U, 0x2022U, (cXyz *) &arg0->unk1F8, &arg0->unk20C, NULL, 0xB9U, &arg0->unk7C4, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
        if (temp_r3 != NULL) {
            temp_r3->unk38 = (f32) @4155.unk18;
            temp_r3->unk5C = (f32) @4155.unk1C;
            temp_r3->unk1D8 = (f32) @4155.unk14;
            temp_r3->unk1DC = (f32) @4155.unk14;
            temp_r3->unk1E0 = (f32) @4155.unk14;
            temp_r3->unk1F0 = (f32) @4155.unk14;
            temp_r3->unk1F4 = (f32) @4155.unk14;
            temp_r3->unk1F8 = (f32) @4155.unk14;
            temp_r3->unk1F0 = (f32) @4155.unk20;
            temp_r3->unk1F4 = (f32) @4155.unk20;
            temp_r3->unk1F8 = (f32) @4155.unk20;
            arg0->unk7C0 = (u8) arg1;
        }
    }
}

/* damage_check (pt_class *) */
void damage_check__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    ? *sp38;
    void *sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    cXyz spC;
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    fopAc_ac_c *temp_r28;
    s32 temp_r4;
    s8 temp_r8;
    s8 temp_r8_2;
    s8 temp_r8_3;
    s8 temp_r8_4;
    u32 var_r6;
    u32 var_r6_2;
    u32 var_r6_3;
    u32 var_r6_4;
    void *temp_r3;
    void *temp_r3_2;
    void *var_r3;

    temp_r28 = g_dComIfG_gameInfo.unk5B44;
    if ((s16) arg0->unk30E == 0) {
        Move__10dCcD_GSttsFv(&arg0->unk548);
        if (ChkAtHit__12dCcD_GObjInfFv(&arg0->unk694) != 0U) {
            temp_r3 = GetAtHitObj__12dCcD_GObjInfFv(&arg0->unk694)->unk44;
            if (temp_r3 == NULL) {
                var_r3 = NULL;
            } else {
                var_r3 = temp_r3->unkC;
            }
            if ((var_r3 != NULL) && ((s16) var_r3->unk8 == 0xA9)) {
                if (temp_r28->unk31C->unk1C(temp_r28) != 0) {
                    arg0->unk324 = fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
                    arg0->unk320 = @4155.unk24;
                    goto block_10;
                }
                arg0->unk2D2 = 3;
                arg0->unk2D4 = 2;
                arg0->unk30E = 6;
                return;
            }
            goto block_10;
        }
block_10:
        if (temp_r28->unk2A4 & 0x20000) {
            __mi__4cXyzCFRC3Vec(&spC, &temp_r28->unk1F8);
            sp18 = (bitwise f32) spC;
            sp1C = sp10;
            sp20 = sp14;
            var_f1 = PSVECSquareMag(&sp18);
            if (var_f1 > 0.0f) {
                temp_f0 = __frsqrte(var_f1);
                temp_f0_2 = @4155.unk28 * temp_f0 * (@4155.unk30 - ((f64) var_f1 * (temp_f0 * temp_f0)));
                temp_f0_3 = @4155.unk28 * temp_f0_2 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
                sp8 = (f32) ((f64) var_f1 * (@4155.unk28 * temp_f0_3 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
                var_f1 = sp8;
            }
            if (var_f1 < @4155.unk38) {
                arg0->unk2D2 = 3;
                arg0->unk2D4 = 0;
                arg0->unk30A = (s16) (@4155.unk3C + cM_rndF__Ff(@4155.unk40));
                smoke_set__FP8pt_classSc(arg0, 3);
                arg0->unk30E = 0xA;
                arg0->unk224 = @4155.unk38;
                return;
            }
            goto block_15;
        }
block_15:
        if (ChkTgHit__12dCcD_GObjInfFv(&arg0->unk568) != 0U) {
            arg0->unk30E = 6;
            temp_r3_2 = GetTgHitObj__12dCcD_GObjInfFv(&arg0->unk568);
            sp24 = temp_r3_2;
            sp38 = &arg0->unk634;
            temp_r4 = temp_r3_2->unk10;
            if (temp_r4 & 0x100000) {
                arg0->unk7EA = 1;
                enemy_fire_remove__FP9enemyfire(&arg0->unkB9C);
                arg0->unk7C0 = 0;
                arg0->unk7C4.unk0->unk20(&arg0->unk7C4);
                return;
            }
            if (temp_r4 & 0x40200) {
                arg0->unkBA0 = 0x64;
                arg0->unk30E = 0x32;
            }
            if (sp24->unk10 & 0x200000) {
                arg0->unk324 = fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
                smoke_set__FP8pt_classSc(arg0, 5);
                arg0->unk320 = @4155.unk44 + cM_rndF__Ff(@4155.unk40);
                arg0->unk310 = (s16) cM_rndFX__Ff(@4155.unk48);
                arg0->unk2D2 = 3;
                arg0->unk2D4 = 0xA;
                smoke_set__FP8pt_classSc(arg0, 0xA);
                temp_r8 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
                if (arg0 != NULL) {
                    var_r6 = arg0->unk4;
                } else {
                    var_r6 = -1U;
                }
                monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48F9U, &arg0->unk260, var_r6, 0U, temp_r8);
                return;
            }
            cc_at_check__FP10fopAc_ac_cP8CcAtInfo((fopAc_ac_c *) arg0, (CcAtInfo *) &sp24);
            if (sp2E == 9) {
                if ((u8) temp_r28->unk290 == 0x11) {
                    arg0->unk2D2 = 5;
                    smoke_set__FP8pt_classSc(arg0, 0x1E);
                } else {
                    arg0->unk2D2 = 4;
                }
                arg0->unk2D4 = 0;
                temp_r8_2 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
                if (arg0 != NULL) {
                    var_r6_2 = arg0->unk4;
                } else {
                    var_r6_2 = -1U;
                }
                monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FAU, &arg0->unk260, var_r6_2, 0U, temp_r8_2);
                arg0->unk30E = 0x32;
                return;
            }
            arg0->unk324 = fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
            smoke_set__FP8pt_classSc(arg0, 0xA);
            if ((s8) arg0->unk285 <= 0) {
                arg0->unk30E = 0x32;
                temp_r8_3 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
                if (arg0 != NULL) {
                    var_r6_3 = arg0->unk4;
                } else {
                    var_r6_3 = -1U;
                }
                monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48F9U, &arg0->unk260, var_r6_3, 0U, temp_r8_3);
                arg0->unk30A = (s16) (@4155.unk4C + cM_rndF__Ff(@4155.unk4C));
                arg0->unk320 = @4155.unk50;
            } else {
                temp_r8_4 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
                if (arg0 != NULL) {
                    var_r6_4 = arg0->unk4;
                } else {
                    var_r6_4 = -1U;
                }
                monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FAU, &arg0->unk260, var_r6_4, 0U, temp_r8_4);
                arg0->unk320 = @4155.unk54;
            }
            arg0->unk2D2 = 2;
            arg0->unk2D4 = 1;
        }
    }
}

/* get_z_ang (pt_class *) */
s16 get_z_ang__FP8pt_class(pt_class *arg0) {
    s32 *sp90;                                      /* compiler-managed */
    s8 sp8E;
    s8 sp8D;
    s8 sp8C;
    s8 sp8B;
    s8 sp8A;
    s8 sp89;
    s8 sp88;
    struct __vt__16cBgS_PolyPassChk *vtable84;      /* compiler-managed */
    struct __vt__8cM3dGLin *vtable68;
    struct __vt__13cBgS_PolyInfo *vtable4C;         /* compiler-managed */
    s32 sp48;
    s32 sp44;
    s16 sp42;
    s16 sp40;
    struct __vt__11cBgS_LinChk *vtable3C;           /* compiler-managed */
    s8 sp38;
    struct __vt__16cBgS_PolyPassChk **sp30;
    struct __vt__16cBgS_PolyPassChk **sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    cXyz sp14;
    cXyz sp8;
    f32 temp_f28;
    f32 temp_f29;
    f32 temp_f31;
    s16 var_r22;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_cr0_eq_3;
    s32 temp_cr0_eq_4;
    s32 var_r21;
    struct __vt__16cBgS_PolyPassChk **var_r0;

    vtable3C = &__vt__8cBgS_Chk;
    sp2C = NULL;
    sp30 = NULL;
    sp38 = 1;
    vtable4C = &__vt__13cBgS_PolyInfo;
    sp40 = 0xFFFF;
    sp42 = 0x100;
    sp44 = 0;
    sp48 = -1;
    vtable3C = &__vt__11cBgS_LinChk;
    vtable4C = &__vt__11cBgS_LinChk.unkC;
    vtable68 = &__vt__8cM3dGLin;
    ct__11cBgS_LinChkFv((cBgS_LinChk *) &sp2C);
    vtable84 = &__vt__16cBgS_PolyPassChk;
    vtable84 = &__vt__16dBgS_PolyPassChk;
    sp88 = 0;
    sp89 = 0;
    sp8A = 0;
    sp8B = 0;
    sp8C = 0;
    sp8D = 0;
    sp8E = 0;
    vtable84.unkC = &__vt__15cBgS_GrpPassChk;
    vtable84.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable84.unk10 = 1;
    vtable84 = &__vt__8dBgS_Chk;
    sp90 = &__vt__8dBgS_Chk.unkC;
    vtable3C = &__vt__11dBgS_LinChk;
    vtable4C = &__vt__11dBgS_LinChk.unkC;
    vtable84 = &__vt__11dBgS_LinChk.unk18;
    sp90 = &__vt__11dBgS_LinChk.unk24;
    sp2C = &vtable84;
    var_r0 = &vtable84;
    if (&vtable84 != NULL) {
        var_r0 = &vtable84 + 0xC;
    }
    sp30 = var_r0;
    MtxTrans__FfffUc(arg0->unk1F8, arg0->unk1FC, arg0->unk200, 0U);
    mDoMtx_YrotM__FPA4_fs(calc_mtx, arg0->unk206);
    mDoMtx_XrotM__FPA4_fs(calc_mtx, arg0->unk204);
    var_r22 = 0;
    var_r21 = 0;
    temp_f28 = @4155.unk58;
    temp_f29 = @4155.unk5C;
    temp_f31 = @4155.unk60;
loop_3:
    MtxPush__Fv();
    mDoMtx_ZrotM__FPA4_fs(calc_mtx, var_r22);
    sp20 = temp_f28;
    sp24 = temp_f29;
    sp28 = 0.0f;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp20, &sp14);
    sp24 = temp_f31;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp20, &sp8);
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp2C, &sp14, &sp8, (fopAc_ac_c *) arg0);
    if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp2C) != 0) {
        sp20 = @4155.unk64;
        sp24 = @4155.unk5C;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp20, &sp14);
        sp24 = @4155.unk60;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp20, &sp8);
        Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp2C, &sp14, &sp8, (fopAc_ac_c *) arg0);
        if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp2C) != 0) {
            MtxPull__Fv();
            vtable3C = &__vt__11dBgS_LinChk;
            vtable4C = &__vt__11dBgS_LinChk.unkC;
            vtable84 = &__vt__11dBgS_LinChk.unk18;
            sp90 = &__vt__11dBgS_LinChk.unk24;
            if (&vtable84 != NULL) {
                vtable84 = &__vt__8dBgS_Chk;
                sp90 = &__vt__8dBgS_Chk.unkC;
                temp_cr0_eq = &sp90 == NULL;
                if (temp_cr0_eq == 0) {
                    sp90 = &__vt__15dBgS_GrpPassChk;
                    if (temp_cr0_eq == 0) {
                        sp90 = &__vt__15cBgS_GrpPassChk;
                    }
                }
                temp_cr0_eq_2 = &vtable84 == NULL;
                if (temp_cr0_eq_2 == 0) {
                    vtable84 = &__vt__16dBgS_PolyPassChk;
                    if (temp_cr0_eq_2 == 0) {
                        vtable84 = &__vt__16cBgS_PolyPassChk;
                    }
                }
            }
            vtable3C = &__vt__11cBgS_LinChk;
            vtable4C = &__vt__11cBgS_LinChk.unkC;
            vtable68 = &__vt__8cM3dGLin;
            vtable4C = &__vt__13cBgS_PolyInfo;
            __dt__8cBgS_ChkFv((cBgS_Chk *) &sp2C, 0);
            return var_r22;
        }
    }
    MtxPull__Fv();
    var_r21 += 1;
    var_r22 += 0x1000;
    if (var_r21 >= 0x10) {
        vtable3C = &__vt__11dBgS_LinChk;
        vtable4C = &__vt__11dBgS_LinChk.unkC;
        vtable84 = &__vt__11dBgS_LinChk.unk18;
        sp90 = &__vt__11dBgS_LinChk.unk24;
        if (&vtable84 != NULL) {
            vtable84 = &__vt__8dBgS_Chk;
            sp90 = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq_3 = &sp90 == NULL;
            if (temp_cr0_eq_3 == 0) {
                sp90 = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq_3 == 0) {
                    sp90 = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_4 = &vtable84 == NULL;
            if (temp_cr0_eq_4 == 0) {
                vtable84 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_4 == 0) {
                    vtable84 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable3C = &__vt__11cBgS_LinChk;
        vtable4C = &__vt__11cBgS_LinChk.unkC;
        vtable68 = &__vt__8cM3dGLin;
        vtable4C = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp2C, 0);
        return 0xDCF;
    }
    goto loop_3;
}

/* next_pos_set (pt_class *) */
s32 next_pos_set__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    s32 *sp9C;                                      /* compiler-managed */
    s8 sp9A;
    s8 sp99;
    s8 sp98;
    s8 sp97;
    s8 sp96;
    s8 sp95;
    s8 sp94;
    struct __vt__16cBgS_PolyPassChk *vtable90;      /* compiler-managed */
    struct __vt__8cM3dGLin *vtable74;
    ? sp5C;
    struct __vt__13cBgS_PolyInfo *vtable58;         /* compiler-managed */
    s32 sp54;
    s32 sp50;
    s16 sp4E;
    s16 sp4C;
    struct __vt__11cBgS_LinChk *vtable48;           /* compiler-managed */
    s8 sp44;
    struct __vt__16cBgS_PolyPassChk **sp3C;
    struct __vt__16cBgS_PolyPassChk **sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    cXyz sp20;
    cXyz sp14;
    cXyz sp8;
    f32 *temp_r3;
    f32 *temp_r3_2;
    f32 *temp_r3_3;
    s16 temp_r0;
    s16 var_r21;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_cr0_eq_3;
    s32 temp_cr0_eq_4;
    s32 temp_cr0_eq_5;
    s32 temp_cr0_eq_6;
    s32 temp_cr0_eq_7;
    s32 temp_cr0_eq_8;
    struct __vt__16cBgS_PolyPassChk **var_r0;
    u8 temp_r4;
    u8 temp_r4_2;

    vtable48 = &__vt__8cBgS_Chk;
    sp38 = NULL;
    sp3C = NULL;
    sp44 = 1;
    vtable58 = &__vt__13cBgS_PolyInfo;
    sp4C = 0xFFFF;
    sp4E = 0x100;
    sp50 = 0;
    sp54 = -1;
    vtable48 = &__vt__11cBgS_LinChk;
    vtable58 = &__vt__11cBgS_LinChk.unkC;
    vtable74 = &__vt__8cM3dGLin;
    ct__11cBgS_LinChkFv((cBgS_LinChk *) &sp38);
    vtable90 = &__vt__16cBgS_PolyPassChk;
    vtable90 = &__vt__16dBgS_PolyPassChk;
    sp94 = 0;
    sp95 = 0;
    sp96 = 0;
    sp97 = 0;
    sp98 = 0;
    sp99 = 0;
    sp9A = 0;
    vtable90.unkC = &__vt__15cBgS_GrpPassChk;
    vtable90.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable90.unk10 = 1;
    vtable90 = &__vt__8dBgS_Chk;
    sp9C = &__vt__8dBgS_Chk.unkC;
    vtable48 = &__vt__11dBgS_LinChk;
    vtable58 = &__vt__11dBgS_LinChk.unkC;
    vtable90 = &__vt__11dBgS_LinChk.unk18;
    sp9C = &__vt__11dBgS_LinChk.unk24;
    sp38 = &vtable90;
    var_r0 = &vtable90;
    if (&vtable90 != NULL) {
        var_r0 = &vtable90 + 0xC;
    }
    sp3C = var_r0;
    temp_r0 = arg0->unk204;
    if ((temp_r0 < 0x2000) && (temp_r0 > -0x2000)) {
        var_r21 = arg0->unk2FC;
        if (((s16) arg0->unk30A != 0) || ((temp_r4 = arg0->unk2B7, ((temp_r4 == 0xFF) == 0)) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) == 0)) || ((temp_r4_2 = arg0->unk2B8, ((temp_r4_2 == 0xFF) == 0)) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4_2, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) != 0))) {
            var_r21 += 0x8000;
        }
        mDoMtx_YrotS__FPA4_fs(calc_mtx, var_r21 + (s32) cM_rndFX__Ff(@4155.unk68));
        mDoMtx_XrotM__FPA4_fs(calc_mtx, arg0->unk204);
    } else {
        mDoMtx_YrotS__FPA4_fs(calc_mtx, (s16) (arg0->unk206 + (s32) cM_rndFX__Ff(@4155.unk68)));
        mDoMtx_XrotM__FPA4_fs(calc_mtx, arg0->unk204);
    }
    mDoMtx_ZrotM__FPA4_fs(calc_mtx, arg0->unk208);
    sp2C = 0.0f;
    sp30 = @4155.unk6C;
    sp34 = @4155.unk6C;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp14);
    PSVECAdd(&sp14, &arg0->unk1F8, &sp14);
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp38, (cXyz *) &arg0->unk1F8, &sp14, (fopAc_ac_c *) arg0);
    if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp38) != 0) {
        arg0->unk2E8 = sp68;
        arg0->unk2EC = sp6C;
        arg0->unk2F0 = sp70;
        sp30 = @4155.unk70;
        sp34 = @4155.unk70;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp8);
        temp_r3 = &arg0->unk2E8;
        PSVECAdd((cXyz *) temp_r3, &sp8.unk0, (cXyz *) temp_r3);
        vtable48 = &__vt__11dBgS_LinChk;
        vtable58 = &__vt__11dBgS_LinChk.unkC;
        vtable90 = &__vt__11dBgS_LinChk.unk18;
        sp9C = &__vt__11dBgS_LinChk.unk24;
        if (&vtable90 != NULL) {
            vtable90 = &__vt__8dBgS_Chk;
            sp9C = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq = &sp9C == NULL;
            if (temp_cr0_eq == 0) {
                sp9C = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq == 0) {
                    sp9C = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_2 = &vtable90 == NULL;
            if (temp_cr0_eq_2 == 0) {
                vtable90 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_2 == 0) {
                    vtable90 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable48 = &__vt__11cBgS_LinChk;
        vtable58 = &__vt__11cBgS_LinChk.unkC;
        if (&sp5C != NULL) {
            vtable74 = &__vt__8cM3dGLin;
        }
        vtable58 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
        return 1;
    }
    sp30 = @4155.unk74;
    sp34 = @4155.unk6C;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp20);
    PSVECAdd(&sp20, &arg0->unk1F8, &sp20);
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp38, &sp14, &sp20, (fopAc_ac_c *) arg0);
    if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp38) != 0) {
        arg0->unk2E8 = sp68;
        arg0->unk2EC = sp6C;
        arg0->unk2F0 = sp70;
        sp30 = @4155.unk5C;
        sp34 = 0.0f;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp8);
        temp_r3_2 = &arg0->unk2E8;
        PSVECAdd((cXyz *) temp_r3_2, &sp8.unk0, (cXyz *) temp_r3_2);
        vtable48 = &__vt__11dBgS_LinChk;
        vtable58 = &__vt__11dBgS_LinChk.unkC;
        vtable90 = &__vt__11dBgS_LinChk.unk18;
        sp9C = &__vt__11dBgS_LinChk.unk24;
        if (&vtable90 != NULL) {
            vtable90 = &__vt__8dBgS_Chk;
            sp9C = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq_3 = &sp9C == NULL;
            if (temp_cr0_eq_3 == 0) {
                sp9C = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq_3 == 0) {
                    sp9C = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_4 = &vtable90 == NULL;
            if (temp_cr0_eq_4 == 0) {
                vtable90 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_4 == 0) {
                    vtable90 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable48 = &__vt__11cBgS_LinChk;
        vtable58 = &__vt__11cBgS_LinChk.unkC;
        if (&sp5C != NULL) {
            vtable74 = &__vt__8cM3dGLin;
        }
        vtable58 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
        return 1;
    }
    sp30 = @4155.unk78;
    sp34 = @4155.unk78;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp14);
    PSVECAdd(&sp14, &arg0->unk1F8, &sp14);
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp38, &sp20, &sp14, (fopAc_ac_c *) arg0);
    if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp38) != 0) {
        arg0->unk2E8 = sp68;
        arg0->unk2EC = sp6C;
        arg0->unk2F0 = sp70;
        sp30 = 0.0f;
        sp34 = @4155.unk5C;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, &sp8);
        temp_r3_3 = &arg0->unk2E8;
        PSVECAdd((cXyz *) temp_r3_3, &sp8.unk0, (cXyz *) temp_r3_3);
        vtable48 = &__vt__11dBgS_LinChk;
        vtable58 = &__vt__11dBgS_LinChk.unkC;
        vtable90 = &__vt__11dBgS_LinChk.unk18;
        sp9C = &__vt__11dBgS_LinChk.unk24;
        if (&vtable90 != NULL) {
            vtable90 = &__vt__8dBgS_Chk;
            sp9C = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq_5 = &sp9C == NULL;
            if (temp_cr0_eq_5 == 0) {
                sp9C = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq_5 == 0) {
                    sp9C = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_6 = &vtable90 == NULL;
            if (temp_cr0_eq_6 == 0) {
                vtable90 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_6 == 0) {
                    vtable90 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable48 = &__vt__11cBgS_LinChk;
        vtable58 = &__vt__11cBgS_LinChk.unkC;
        if (&sp5C != NULL) {
            vtable74 = &__vt__8cM3dGLin;
        }
        vtable58 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
        return 1;
    }
    vtable48 = &__vt__11dBgS_LinChk;
    vtable58 = &__vt__11dBgS_LinChk.unkC;
    vtable90 = &__vt__11dBgS_LinChk.unk18;
    sp9C = &__vt__11dBgS_LinChk.unk24;
    if (&vtable90 != NULL) {
        vtable90 = &__vt__8dBgS_Chk;
        sp9C = &__vt__8dBgS_Chk.unkC;
        temp_cr0_eq_7 = &sp9C == NULL;
        if (temp_cr0_eq_7 == 0) {
            sp9C = &__vt__15dBgS_GrpPassChk;
            if (temp_cr0_eq_7 == 0) {
                sp9C = &__vt__15cBgS_GrpPassChk;
            }
        }
        temp_cr0_eq_8 = &vtable90 == NULL;
        if (temp_cr0_eq_8 == 0) {
            vtable90 = &__vt__16dBgS_PolyPassChk;
            if (temp_cr0_eq_8 == 0) {
                vtable90 = &__vt__16cBgS_PolyPassChk;
            }
        }
    }
    vtable48 = &__vt__11cBgS_LinChk;
    vtable58 = &__vt__11cBgS_LinChk.unkC;
    vtable74 = &__vt__8cM3dGLin;
    vtable58 = &__vt__13cBgS_PolyInfo;
    __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
    return 0;
}

/* pt_move (pt_class *) */
void pt_move__FP8pt_class(pt_class *arg0) {
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    cXyz sp20;
    cXyz sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 *temp_r3_2;
    f32 temp_f1;
    f32 var_f1;
    f32 var_f1_2;
    f32 var_f2;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    f64 temp_f0_7;
    f64 temp_f0_8;
    f64 temp_f0_9;
    s16 temp_r0;
    s16 temp_r0_2;
    s16 temp_r3;
    s16 temp_r3_3;
    s8 temp_r8;
    u32 var_r6;

    sp2C = 0.0f;
    sp30 = 0.0f;
    sp34 = @4155.unk3C;
    temp_r0 = arg0->unk2D4;
    switch (temp_r0) {                              /* irregular */
    case 0:
        if (next_pos_set__FP8pt_class(arg0) != 0) {
            arg0->unk2D4 = 1;
            __mi__4cXyzCFRC3Vec(&sp20, (Vec *) &arg0->unk2E8);
            sp38 = (bitwise f32) sp20;
            sp3C = sp24;
            sp40 = sp28;
            var_f1 = PSVECSquareMag(&sp38);
            if (var_f1 > 0.0f) {
                temp_f0 = __frsqrte(var_f1);
                temp_f0_2 = @4155.unk28 * temp_f0 * (@4155.unk30 - ((f64) var_f1 * (temp_f0 * temp_f0)));
                temp_f0_3 = @4155.unk28 * temp_f0_2 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
                spC = (f32) ((f64) var_f1 * (@4155.unk28 * temp_f0_3 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
                var_f1 = spC;
            }
            arg0->unk31C = @4155.unk7C * var_f1;
            if (arg0->unk31C > @4155.unk10) {
                arg0->unk31C = @4155.unk10;
            }
        default:
block_32:
            if (((arg0->unk1FC - arg0->unk2F8->unk4) > @4155.unk78) && (arg0->unk304 < @4155.unk88)) {
                arg0->unk2D2 = 2;
                arg0->unk2D4 = 0;
            }
            return;
        }
        arg0->unk2D2 = 2;
        arg0->unk2D4 = 0;
        return;
    case 1:
        __mi__4cXyzCFRC3Vec(&sp14, (Vec *) &arg0->unk2E8);
        temp_f1 = (bitwise f32) sp14;
        sp38 = temp_f1;
        sp3C = sp18;
        sp40 = sp1C;
        arg0->unk206 = cM_atan2s__Fff(temp_f1, sp1C);
        var_f2 = (sp38 * sp38) + (sp40 * sp40);
        if (var_f2 > 0.0f) {
            temp_f0_4 = __frsqrte(var_f2);
            temp_f0_5 = @4155.unk28 * temp_f0_4 * (@4155.unk30 - ((f64) var_f2 * (temp_f0_4 * temp_f0_4)));
            temp_f0_6 = @4155.unk28 * temp_f0_5 * (@4155.unk30 - ((f64) var_f2 * (temp_f0_5 * temp_f0_5)));
            sp10 = (f32) ((f64) var_f2 * (@4155.unk28 * temp_f0_6 * (@4155.unk30 - ((f64) var_f2 * (temp_f0_6 * temp_f0_6)))));
            var_f2 = sp10;
        }
        arg0->unk204 = -cM_atan2s__Fff(sp3C, var_f2);
        temp_r3 = get_z_ang__FP8pt_class(arg0);
        if (temp_r3 != 0xDCF) {
            arg0->unk208 = temp_r3;
        }
        var_f1_2 = PSVECSquareMag(&sp38);
        if (var_f1_2 > 0.0f) {
            temp_f0_7 = __frsqrte(var_f1_2);
            temp_f0_8 = @4155.unk28 * temp_f0_7 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_7 * temp_f0_7)));
            temp_f0_9 = @4155.unk28 * temp_f0_8 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_8 * temp_f0_8)));
            sp8 = (f32) ((f64) var_f1_2 * (@4155.unk28 * temp_f0_9 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_9 * temp_f0_9)))));
            var_f1_2 = sp8;
        }
        if (var_f1_2 < (@4155.unk20 * sp34)) {
            if ((cM_rndF__Ff(@4155.unk14) < @4155.unk80) && (temp_r0_2 = arg0->unk204, ((temp_r0_2 < 0x1000) != 0)) && (temp_r0_2 > -0x1000)) {
                arg0->unk2D2 = 3;
                arg0->unk2D4 = 0;
                smoke_set__FP8pt_classSc(arg0, 3);
            } else {
                arg0->unk2D4 = 2;
                arg0->unk308 = (s16) (@4155.unk5C + cM_rndF__Ff(@4155.unk84));
                anm_init__FP8pt_classifUcfi(arg0, 0xC, @4155.unk18, 2U, @4155.unk14, -1);
                seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592DU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            }
        } else {
            mDoMtx_YrotS__FPA4_fs(calc_mtx, arg0->unk206);
            mDoMtx_XrotM__FPA4_fs(calc_mtx, arg0->unk204);
            MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, (cXyz *) &arg0->unk220);
            temp_r3_2 = &arg0->unk1F8;
            PSVECAdd((cXyz *) temp_r3_2, &arg0->unk220, (cXyz *) temp_r3_2);
        }
        goto block_32;
    case 2:
        cLib_addCalc2__FPffff(&arg0->unk1F8, arg0->unk2E8, @4155.unk14, (f32) fabs(arg0->unk220));
        cLib_addCalc2__FPffff(&arg0->unk1FC, arg0->unk2EC, @4155.unk14, (f32) fabs(arg0->unk224));
        cLib_addCalc2__FPffff(&arg0->unk200, arg0->unk2F0, @4155.unk14, (f32) fabs(arg0->unk228));
        temp_r3_3 = get_z_ang__FP8pt_class(arg0);
        if (temp_r3_3 != 0xDCF) {
            arg0->unk208 = temp_r3_3;
        }
        if ((s16) arg0->unk308 == 0) {
            arg0->unk2D4 = 0;
            anm_init__FP8pt_classifUcfi(arg0, 9, @4155.unk18, 0U, @4155.unk14, -1);
            temp_r8 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6 = arg0->unk4;
            } else {
                var_r6 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48F8U, &arg0->unk260, var_r6, 0U, temp_r8);
        }
        goto block_32;
    }
}

/* view_check (pt_class *) */
s32 view_check__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    s32 *sp78;                                      /* compiler-managed */
    s8 sp76;
    s8 sp75;
    s8 sp74;
    s8 sp73;
    s8 sp72;
    s8 sp71;
    s8 sp70;
    struct __vt__16cBgS_PolyPassChk *vtable6C;      /* compiler-managed */
    struct __vt__8cM3dGLin *vtable50;
    struct __vt__13cBgS_PolyInfo *vtable34;         /* compiler-managed */
    s32 sp30;
    s32 sp2C;
    s16 sp2A;
    s16 sp28;
    struct __vt__11cBgS_LinChk *vtable24;           /* compiler-managed */
    s8 sp20;
    struct __vt__16cBgS_PolyPassChk **sp18;
    struct __vt__16cBgS_PolyPassChk **sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    Vec *temp_r3;
    f32 temp_f1;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_cr0_eq_3;
    s32 temp_cr0_eq_4;
    struct __vt__16cBgS_PolyPassChk **var_r0;

    temp_r3 = arg0->unk2F8;
    sp8 = temp_r3->unk0;
    temp_f1 = temp_r3->unk4;
    spC = temp_f1;
    sp10 = temp_r3->unk8;
    spC = temp_f1 + 100.0f;
    vtable24 = &__vt__8cBgS_Chk;
    sp14 = NULL;
    sp18 = NULL;
    sp20 = 1;
    vtable34 = &__vt__13cBgS_PolyInfo;
    sp28 = 0xFFFF;
    sp2A = 0x100;
    sp2C = 0;
    sp30 = -1;
    vtable24 = &__vt__11cBgS_LinChk;
    vtable34 = &__vt__11cBgS_LinChk.unkC;
    vtable50 = &__vt__8cM3dGLin;
    ct__11cBgS_LinChkFv((cBgS_LinChk *) &sp14);
    vtable6C = &__vt__16cBgS_PolyPassChk;
    vtable6C = &__vt__16dBgS_PolyPassChk;
    sp70 = 0;
    sp71 = 0;
    sp72 = 0;
    sp73 = 0;
    sp74 = 0;
    sp75 = 0;
    sp76 = 0;
    vtable6C.unkC = &__vt__15cBgS_GrpPassChk;
    vtable6C.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable6C.unk10 = 1;
    vtable6C = &__vt__8dBgS_Chk;
    sp78 = &__vt__8dBgS_Chk.unkC;
    vtable24 = &__vt__11dBgS_LinChk;
    vtable34 = &__vt__11dBgS_LinChk.unkC;
    vtable6C = &__vt__11dBgS_LinChk.unk18;
    sp78 = &__vt__11dBgS_LinChk.unk24;
    sp14 = &vtable6C;
    var_r0 = &vtable6C;
    if (&vtable6C != NULL) {
        var_r0 = &vtable6C + 0xC;
    }
    sp18 = var_r0;
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp14, (cXyz *) &arg0->unk260, (cXyz *) &sp8, (fopAc_ac_c *) arg0);
    if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp14) != 0) {
        vtable24 = &__vt__11dBgS_LinChk;
        vtable34 = &__vt__11dBgS_LinChk.unkC;
        vtable6C = &__vt__11dBgS_LinChk.unk18;
        sp78 = &__vt__11dBgS_LinChk.unk24;
        if (&vtable6C != NULL) {
            vtable6C = &__vt__8dBgS_Chk;
            sp78 = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq = &sp78 == NULL;
            if (temp_cr0_eq == 0) {
                sp78 = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq == 0) {
                    sp78 = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_2 = &vtable6C == NULL;
            if (temp_cr0_eq_2 == 0) {
                vtable6C = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_2 == 0) {
                    vtable6C = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable24 = &__vt__11cBgS_LinChk;
        vtable34 = &__vt__11cBgS_LinChk.unkC;
        vtable50 = &__vt__8cM3dGLin;
        vtable34 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp14, 0);
        return 1;
    }
    vtable24 = &__vt__11dBgS_LinChk;
    vtable34 = &__vt__11dBgS_LinChk.unkC;
    vtable6C = &__vt__11dBgS_LinChk.unk18;
    sp78 = &__vt__11dBgS_LinChk.unk24;
    if (&vtable6C != NULL) {
        vtable6C = &__vt__8dBgS_Chk;
        sp78 = &__vt__8dBgS_Chk.unkC;
        temp_cr0_eq_3 = &sp78 == NULL;
        if (temp_cr0_eq_3 == 0) {
            sp78 = &__vt__15dBgS_GrpPassChk;
            if (temp_cr0_eq_3 == 0) {
                sp78 = &__vt__15cBgS_GrpPassChk;
            }
        }
        temp_cr0_eq_4 = &vtable6C == NULL;
        if (temp_cr0_eq_4 == 0) {
            vtable6C = &__vt__16dBgS_PolyPassChk;
            if (temp_cr0_eq_4 == 0) {
                vtable6C = &__vt__16cBgS_PolyPassChk;
            }
        }
    }
    vtable24 = &__vt__11cBgS_LinChk;
    vtable34 = &__vt__11cBgS_LinChk.unkC;
    vtable50 = &__vt__8cM3dGLin;
    vtable34 = &__vt__13cBgS_PolyInfo;
    __dt__8cBgS_ChkFv((cBgS_Chk *) &sp14, 0);
    return 0;
}

/* pt_attack (pt_class *) */
void pt_attack__FP8pt_class(pt_class *arg0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    cXyz sp8;
    f32 *temp_r3_3;
    f32 temp_f1;
    f32 temp_f31;
    mDoExt_McaMorf *temp_r3_2;
    s16 temp_r0;
    s16 var_r4;
    s8 temp_r8;
    u32 temp_r3;
    u32 var_r6;
    u8 var_r4_2;

    var_r4 = arg0->unk2FC;
    temp_f31 = arg0->unk300;
    if ((s16) arg0->unk30A != 0) {
        var_r4 += 0x8000;
    }
    cLib_addCalcAngleS2__FPssss(&arg0->unk206, var_r4, 4, 0x800);
    cLib_addCalcAngleS2__FPssss(&arg0->unk204, 0, 2, 0x1000);
    cLib_addCalcAngleS2__FPssss(&arg0->unk208, 0, 2, 0x1000);
    temp_r3 = arg0->unk390 & 0x20;
    if ((temp_r3 != 0) && (arg0->unk314 < @4155.unk8C)) {
        arg0->unk2D2 = 3;
        if (cM_rndF__Ff(@4155.unk14) < @4155.unk1C) {
            arg0->unk2D4 = 1;
        } else {
            arg0->unk2D4 = 0;
        }
        smoke_set__FP8pt_classSc(arg0, 5);
        return;
    }
    temp_r0 = arg0->unk2D4;
    switch (temp_r0) {                              /* irregular */
    case 0:
        if ((s16) arg0->unk308 == 0) {
            arg0->unk2D4 = 1;
        } else {
            arg0->unk220 = 0.0f;
            arg0->unk224 = 0.0f;
            arg0->unk228 = 0.0f;
        }
        break;
    case 1:
        if (temp_r3 != 0) {
            if (cM_rndF__Ff(@4155.unk14) < @4155.unk80) {
                arg0->unk2D2 = 3;
                arg0->unk2D4 = 0;
                smoke_set__FP8pt_classSc(arg0, 3);
            } else {
                arg0->unk2D4 = 2;
                anm_init__FP8pt_classifUcfi(arg0, 7, @4155.unk84, 2U, @4155.unk14, -1);
                arg0->unk220 = 0.0f;
                arg0->unk224 = 0.0f;
                arg0->unk228 = 0.0f;
                arg0->unk254 = 0.0f;
                seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592DU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            }
        }
        break;
    case 2:
        cLib_addCalc2__FPffff(&arg0->unk254, @4155.unk3C, @4155.unk14, @4155.unk90);
        mDoMtx_YrotS__FPA4_fs(calc_mtx, arg0->unk206);
        sp14 = 0.0f;
        sp18 = 0.0f;
        sp1C = arg0->unk254;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp14, &sp8);
        arg0->unk220 = (bitwise f32) sp8;
        arg0->unk228 = sp10;
        if ((temp_f31 < @4155.unk94) && (view_check__FP8pt_class(arg0) == 0)) {
            arg0->unk30C = (arg0->unk2CC * 3) + 0xA;
            arg0->unk2D4 = 3;
            anm_init__FP8pt_classifUcfi(arg0, 0xC, @4155.unk18, 2U, @4155.unk14, -1);
        }
        break;
    case 3:
        if ((s16) arg0->unk30C == 1) {
            anm_init__FP8pt_classifUcfi(arg0, 6, @4155.unk84, 0U, @4155.unk14, -1);
            temp_r8 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6 = arg0->unk4;
            } else {
                var_r6 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48F7U, &arg0->unk260, var_r6, 0U, temp_r8);
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592EU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
        }
        arg0->unk220 *= @4155.unk98;
        arg0->unk228 *= @4155.unk98;
        if ((s16) arg0->unk30C == 0) {
            temp_f1 = arg0->unk2C0->unk68;
            M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
            if (temp_f1 == @4155.unk9C) {
                M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
                if (temp_f1 == @4155.unkA0) {
                    arg0->unk327 = 1;
                }
            }
        }
        temp_r3_2 = arg0->unk2C0;
        var_r4_2 = 1;
        if (!(temp_r3_2->unk5D & 1) && (temp_r3_2->unk64 != 0.0f)) {
            var_r4_2 = 0;
        }
        if (var_r4_2 != 0) {
            arg0->unk2D4 = 1;
        }
        break;
    }
    temp_r3_3 = &arg0->unk1F8;
    PSVECAdd((cXyz *) temp_r3_3, &arg0->unk220, (cXyz *) temp_r3_3);
    arg0->unk224 -= @4155.unkA4;
    if (arg0->unk224 < @4155.unkA8) {
        arg0->unk224 = @4155.unkA8;
    }
    arg0->unk326 = 1;
    if ((arg0->unk390 & 0x20) && (arg0->unk300 > @4155.unkAC)) {
        arg0->unk2D2 = 1;
        arg0->unk2D4 = 0;
    }
}

/* pt_wait (pt_class *) */
void pt_wait__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    s32 *sp9C;                                      /* compiler-managed */
    s8 sp9A;
    s8 sp99;
    s8 sp98;
    s8 sp97;
    s8 sp96;
    s8 sp95;
    s8 sp94;
    struct __vt__16cBgS_PolyPassChk *vtable90;      /* compiler-managed */
    struct __vt__8cM3dGLin *vtable74;
    struct __vt__13cBgS_PolyInfo *vtable58;         /* compiler-managed */
    s32 sp54;
    s32 sp50;
    s16 sp4E;
    s16 sp4C;
    struct __vt__11cBgS_LinChk *vtable48;           /* compiler-managed */
    s8 sp44;
    struct __vt__16cBgS_PolyPassChk **sp3C;
    struct __vt__16cBgS_PolyPassChk **sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    cXyz sp14;
    cXyz sp8;
    f32 temp_f1;
    f32 temp_f1_2;
    s16 temp_r21;
    s16 temp_r3;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_cr0_eq_3;
    s32 temp_cr0_eq_4;
    s32 temp_cr0_eq_5;
    s32 temp_cr0_eq_6;
    s32 temp_r25;
    s8 var_r23;
    struct __vt__16cBgS_PolyPassChk **var_r0;
    u8 temp_r4;
    u8 temp_r4_2;
    u8 temp_r4_3;

    temp_r25 = g_dComIfG_gameInfo.unk5B10;
    vtable48 = &__vt__8cBgS_Chk;
    sp38 = NULL;
    sp3C = NULL;
    sp44 = 1;
    vtable58 = &__vt__13cBgS_PolyInfo;
    sp4C = 0xFFFF;
    sp4E = 0x100;
    sp50 = 0;
    sp54 = -1;
    vtable48 = &__vt__11cBgS_LinChk;
    vtable58 = &__vt__11cBgS_LinChk.unkC;
    vtable74 = &__vt__8cM3dGLin;
    ct__11cBgS_LinChkFv((cBgS_LinChk *) &sp38);
    vtable90 = &__vt__16cBgS_PolyPassChk;
    vtable90 = &__vt__16dBgS_PolyPassChk;
    sp94 = 0;
    sp95 = 0;
    sp96 = 0;
    sp97 = 0;
    sp98 = 0;
    sp99 = 0;
    sp9A = 0;
    vtable90.unkC = &__vt__15cBgS_GrpPassChk;
    vtable90.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable90.unk10 = 1;
    vtable90 = &__vt__8dBgS_Chk;
    sp9C = &__vt__8dBgS_Chk.unkC;
    vtable48 = &__vt__11dBgS_LinChk;
    vtable58 = &__vt__11dBgS_LinChk.unkC;
    vtable90 = &__vt__11dBgS_LinChk.unk18;
    sp9C = &__vt__11dBgS_LinChk.unk24;
    sp38 = &vtable90;
    var_r0 = &vtable90;
    if (&vtable90 != NULL) {
        var_r0 = &vtable90 + 0xC;
    }
    sp3C = var_r0;
    var_r23 = 0;
    arg0->unk30E = 6;
    arg0->unk1C4 = arg0->unk1C4;
    arg0->unk280 = 0;
    temp_r3 = arg0->unk2D8;
    if (temp_r3 != 0) {
        arg0->unk2D8 = temp_r3 - 1;
        vtable48 = &__vt__11dBgS_LinChk;
        vtable58 = &__vt__11dBgS_LinChk.unkC;
        vtable90 = &__vt__11dBgS_LinChk.unk18;
        sp9C = &__vt__11dBgS_LinChk.unk24;
        if (&vtable90 != NULL) {
            vtable90 = &__vt__8dBgS_Chk;
            sp9C = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq = &sp9C == NULL;
            if (temp_cr0_eq == 0) {
                sp9C = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq == 0) {
                    sp9C = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_2 = &vtable90 == NULL;
            if (temp_cr0_eq_2 == 0) {
                vtable90 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_2 == 0) {
                    vtable90 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable48 = &__vt__11cBgS_LinChk;
        vtable58 = &__vt__11cBgS_LinChk.unkC;
        vtable74 = &__vt__8cM3dGLin;
        vtable58 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
        return;
    }
    if ((u8) l_HIO.unk6 != 0) {
        temp_r4 = arg0->unk2B7;
        if (temp_r4 != 0xFF) {
            onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0);
        }
    }
    temp_r4_2 = arg0->unk2B7;
    if (((temp_r4_2 != 0xFF) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4_2, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) == 0)) || ((temp_r4_3 = arg0->unk2B8, ((temp_r4_3 == 0xFF) == 0)) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4_3, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) != 0))) {
        vtable48 = &__vt__11dBgS_LinChk;
        vtable58 = &__vt__11dBgS_LinChk.unkC;
        vtable90 = &__vt__11dBgS_LinChk.unk18;
        sp9C = &__vt__11dBgS_LinChk.unk24;
        if (&vtable90 != NULL) {
            vtable90 = &__vt__8dBgS_Chk;
            sp9C = &__vt__8dBgS_Chk.unkC;
            temp_cr0_eq_3 = &sp9C == NULL;
            if (temp_cr0_eq_3 == 0) {
                sp9C = &__vt__15dBgS_GrpPassChk;
                if (temp_cr0_eq_3 == 0) {
                    sp9C = &__vt__15cBgS_GrpPassChk;
                }
            }
            temp_cr0_eq_4 = &vtable90 == NULL;
            if (temp_cr0_eq_4 == 0) {
                vtable90 = &__vt__16dBgS_PolyPassChk;
                if (temp_cr0_eq_4 == 0) {
                    vtable90 = &__vt__16cBgS_PolyPassChk;
                }
            }
        }
        vtable48 = &__vt__11cBgS_LinChk;
        vtable58 = &__vt__11cBgS_LinChk.unkC;
        vtable74 = &__vt__8cM3dGLin;
        vtable58 = &__vt__13cBgS_PolyInfo;
        __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
        return;
    }
    if (fopAcM_searchActorDistance__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44) < (100.0f * (f32) arg0->unk2B5)) {
        sp20 = arg0->unk1F8;
        temp_f1 = arg0->unk1FC;
        sp24 = temp_f1;
        sp28 = arg0->unk200;
        sp24 = temp_f1 + 100.0f;
        Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp38, temp_r25 + 0xD8, (cXyz *) &sp20, (fopAc_ac_c *) arg0);
        if (LineCross__4cBgSFP11cBgS_LinChk(&g_dComIfG_gameInfo.unk12A0, (cBgS_LinChk *) &sp38) != 0) {
            var_r23 = 1;
        } else {
            __mi__4cXyzCFRC3Vec(&sp14, temp_r25 + 0xE4);
            temp_f1_2 = (bitwise f32) sp14;
            sp2C = temp_f1_2;
            sp30 = sp18;
            sp34 = sp1C;
            temp_r21 = cM_atan2s__Fff(temp_f1_2, sp1C);
            __mi__4cXyzCFRC3Vec(&sp8, (Vec *) &sp20);
            sp2C = (bitwise f32) sp8;
            sp30 = spC;
            sp34 = sp10;
            mDoMtx_YrotS__FPA4_fs(calc_mtx, (s16) -temp_r21);
            MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp2C, (cXyz *) &sp20);
            if (sp28 < 0.0f) {
                var_r23 = 1;
            }
        }
        if (((s8) arg0->unk2B9 == 0) || (var_r23 != 0)) {
            arg0->unk1C4 |= 0x36;
            arg0->unk280 = 4;
            arg0->unk2D2 = 1;
            arg0->unk2D4 = 0;
            arg0->unk2B9 = 0;
        }
    }
    vtable48 = &__vt__11dBgS_LinChk;
    vtable58 = &__vt__11dBgS_LinChk.unkC;
    vtable90 = &__vt__11dBgS_LinChk.unk18;
    sp9C = &__vt__11dBgS_LinChk.unk24;
    if (&vtable90 != NULL) {
        vtable90 = &__vt__8dBgS_Chk;
        sp9C = &__vt__8dBgS_Chk.unkC;
        temp_cr0_eq_5 = &sp9C == NULL;
        if (temp_cr0_eq_5 == 0) {
            sp9C = &__vt__15dBgS_GrpPassChk;
            if (temp_cr0_eq_5 == 0) {
                sp9C = &__vt__15cBgS_GrpPassChk;
            }
        }
        temp_cr0_eq_6 = &vtable90 == NULL;
        if (temp_cr0_eq_6 == 0) {
            vtable90 = &__vt__16dBgS_PolyPassChk;
            if (temp_cr0_eq_6 == 0) {
                vtable90 = &__vt__16cBgS_PolyPassChk;
            }
        }
    }
    vtable48 = &__vt__11cBgS_LinChk;
    vtable58 = &__vt__11cBgS_LinChk.unkC;
    vtable74 = &__vt__8cM3dGLin;
    vtable58 = &__vt__13cBgS_PolyInfo;
    __dt__8cBgS_ChkFv((cBgS_Chk *) &sp38, 0);
}

/* pt_koke (pt_class *) */
void pt_koke__FP8pt_class(pt_class *arg0) {
    mDoExt_McaMorf *temp_r3;
    s16 temp_r0;
    s8 temp_r8;
    s8 temp_r8_2;
    s8 temp_r8_3;
    s8 temp_r8_4;
    u32 var_r6;
    u32 var_r6_2;
    u32 var_r6_3;
    u32 var_r6_4;
    u8 var_r4;

    cLib_addCalcAngleS2__FPssss(&arg0->unk204, 0, 2, 0x2000);
    cLib_addCalcAngleS2__FPssss(&arg0->unk208, 0, 2, 0x2000);
    temp_r0 = arg0->unk2D4;
    if ((u32) temp_r0 <= 0xCU) {
        switch (temp_r0) {
        case 0:
            arg0->unk2D4 = 4;
            anm_init__FP8pt_classifUcfi(arg0, 0xA, @4155.unk5C, 0U, @4155.unk14, -1);
            arg0->unk308 = 0;
            temp_r8 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6 = arg0->unk4;
            } else {
                var_r6 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FBU, &arg0->unk260, var_r6, 0U, temp_r8);
            break;
        case 1:
            arg0->unk2D4 = 5;
            anm_init__FP8pt_classifUcfi(arg0, 0xB, @4155.unk5C, 0U, @4155.unk14, -1);
            arg0->unk308 = 0;
            temp_r8_2 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6_2 = arg0->unk4;
            } else {
                var_r6_2 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FBU, &arg0->unk260, var_r6_2, 0U, temp_r8_2);
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592FU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            break;
        case 2:
            arg0->unk2D4 = 5;
            anm_init__FP8pt_classifUcfi(arg0, 8, @4155.unk5C, 2U, @4155.unk14, -1);
            arg0->unk308 = (s16) (@4155.unk3C + cM_rndF__Ff(@4155.unk3C));
            temp_r8_3 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6_3 = arg0->unk4;
            } else {
                var_r6_3 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FCU, &arg0->unk260, var_r6_3, 0U, temp_r8_3);
            break;
        case 4:
            if ((s32) arg0->unk2C0->unk68 == 6) {
                seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592FU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            }
            /* fallthrough */
        case 5:
            if ((s16) arg0->unk30A == 0) {
                temp_r3 = arg0->unk2C0;
                var_r4 = 1;
                if (!(temp_r3->unk5D & 1) && (temp_r3->unk64 != 0.0f)) {
                    var_r4 = 0;
                }
                if ((var_r4 != 0) || ((s16) arg0->unk308 == 1)) {
                    arg0->unk2D2 = 1;
                    arg0->unk2D4 = 0;
                    anm_init__FP8pt_classifUcfi(arg0, 9, @4155.unk18, 0U, @4155.unk14, -1);
                }
            }
            break;
        case 10:
            arg0->unk2D4 = 0xB;
            anm_init__FP8pt_classifUcfi(arg0, 0xA, @4155.unk5C, 0U, @4155.unk14, -1);
            temp_r8_4 = dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A);
            if (arg0 != NULL) {
                var_r6_4 = arg0->unk4;
            } else {
                var_r6_4 = -1U;
            }
            monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x48FBU, &arg0->unk260, var_r6_4, 0U, temp_r8_4);
            break;
        case 11:
            arg0->unk206 += arg0->unk310;
            if ((s32) arg0->unk2C0->unk68 == 6) {
                seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592FU, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            }
            if (arg0->unk320 < @4155.unkB8) {
                arg0->unk2D4 = 0xC;
                arg0->unk308 = (s16) cM_rndF__Ff(@4155.unk40);
            }
            break;
        case 12:
            if ((s16) arg0->unk308 == 0) {
                arg0->unk2D2 = 1;
                arg0->unk2D4 = 0;
                anm_init__FP8pt_classifUcfi(arg0, 9, @4155.unk18, 0U, @4155.unk14, -1);
            }
            break;
        }
    }
    arg0->unk326 = 1;
    arg0->unk1FC += arg0->unk224;
    arg0->unk224 -= @4155.unkA4;
}

/* pt_ples (pt_class *) */
s32 pt_ples__FP8pt_class(pt_class *arg0) {
    s16 temp_r0;

    arg0->unk30E = 5;
    arg0->unk208 = 0;
    arg0->unk204 = 0;
    temp_r0 = arg0->unk2D4;
    switch (temp_r0) {                              /* irregular */
    case 0:
        arg0->unk308 = 0x23;
        arg0->unk2D4 = 1;
        anm_init__FP8pt_classifUcfi(arg0, 0xC, @4155.unk14, 0U, @4155.unk84, -1);
        /* fallthrough */
    case 1:
        cLib_addCalc2__FPffff(&arg0->unk218, @4155.unk7C, @4155.unk14, @4155.unk1C);
        cLib_addCalc2__FPffff(&arg0->unk214, @4155.unkBC, @4155.unk98, @4155.unk1C);
        if ((s16) arg0->unk308 == 0) {
            return 1;
        }
    default:
        arg0->unk326 = 1;
        arg0->unk1FC += arg0->unk224;
        arg0->unk224 -= @4155.unkA4;
        return 0;
    }
}

/* pt_bat (pt_class *) */
s32 pt_bat__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 *temp_r3;
    s16 temp_r0;
    s16 temp_r29;
    s32 temp_r3_2;

    arg0->unk30E = 5;
    temp_r0 = arg0->unk2D4;
    switch (temp_r0) {                              /* irregular */
    case 0:
        arg0->unk308 = 0x96;
        arg0->unk2D4 = 1;
        anm_init__FP8pt_classifUcfi(arg0, 0xC, @4155.unk14, 0U, @4155.unk84, -1);
        temp_r29 = fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
        arg0->unk324 = (s32) cM_rndFX__Ff(@4155.unkC0) + (temp_r29 + 0x8000);
        mDoMtx_YrotS__FPA4_fs(calc_mtx, arg0->unk324);
        sp8 = 0.0f;
        spC = @4155.unk10 + cM_rndF__Ff(@4155.unk3C);
        sp10 = @4155.unk8;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp8, (cXyz *) &arg0->unk220);
        arg0->unk206 = (s16) cM_rndFX__Ff(@4155.unkC4);
        /* fallthrough */
    case 1:
        arg0->unk206 += 0x400;
        arg0->unk204 += 0x300;
        if ((s16) arg0->unk308 == 0) {
            return 1;
        }
    default:
        arg0->unk326 = 1;
        temp_r3 = &arg0->unk1F8;
        PSVECAdd((cXyz *) temp_r3, &arg0->unk220, (cXyz *) temp_r3);
        arg0->unk224 -= @4155.unk84;
        if (arg0->unk224 < @4155.unkA8) {
            arg0->unk224 = @4155.unkA8;
        }
        temp_r3_2 = arg0->unk390;
        if ((temp_r3_2 & 0x10) || ((arg0->unk224 < 0.0f) && (temp_r3_2 & 0x20))) {
            return 1;
        }
        return 0;
    }
}

/* water_check (pt_class *) */
s32 water_check__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    s32 sp64;
    s32 *sp60;                                      /* compiler-managed */
    s8 sp5E;
    s8 sp5D;
    s8 sp5C;
    s8 sp5B;
    s8 sp5A;
    s8 sp59;
    s8 sp58;
    struct __vt__16cBgS_PolyPassChk *vtable54;      /* compiler-managed */
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    s32 *sp34;                                      /* compiler-managed */
    struct __vt__11dBgS_GndChk *vtable24;           /* compiler-managed */
    cBgS_GndChk sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 temp_f1_2;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_cr0_eq_3;
    s32 temp_cr0_eq_4;
    s32 temp_cr0_eq_5;
    s32 temp_cr0_eq_6;
    s32 temp_r0;
    struct __vt__16cBgS_PolyPassChk **var_r0;

    __ct__11cBgS_GndChkFv(&sp14);
    vtable54 = &__vt__16cBgS_PolyPassChk;
    vtable54 = &__vt__16dBgS_PolyPassChk;
    sp58 = 0;
    sp59 = 0;
    sp5A = 0;
    sp5B = 0;
    sp5C = 0;
    sp5D = 0;
    sp5E = 0;
    vtable54.unkC = &__vt__15cBgS_GrpPassChk;
    vtable54.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable54.unk10 = 1;
    vtable54 = &__vt__8dBgS_Chk;
    sp60 = &__vt__8dBgS_Chk.unkC;
    vtable24 = &__vt__11dBgS_GndChk;
    sp34 = &__vt__11dBgS_GndChk.unkC;
    vtable54 = &__vt__11dBgS_GndChk.unk18;
    sp60 = &__vt__11dBgS_GndChk.unk24;
    sp14 = &vtable54;
    var_r0 = &vtable54;
    if (&vtable54 != NULL) {
        var_r0 = &vtable54 + 0xC;
    }
    sp14.unk4 = var_r0;
    vtable24 = &__vt__14dBgS_ObjGndChk;
    sp34 = &__vt__14dBgS_ObjGndChk.unkC;
    vtable54 = &__vt__14dBgS_ObjGndChk.unk18;
    sp60 = &__vt__14dBgS_ObjGndChk.unk24;
    sp58 = 1;
    vtable24 = &__vt__18dBgS_ObjGndChk_Spl;
    sp34 = &__vt__18dBgS_ObjGndChk_Spl.unkC;
    vtable54 = &__vt__18dBgS_ObjGndChk_Spl.unk18;
    sp60 = &__vt__18dBgS_ObjGndChk_Spl.unk24;
    temp_r0 = sp64 & 0xFFFFFFFE;
    sp64 = temp_r0;
    sp64 = temp_r0 | 0xE;
    sp38 = arg0->unk1F8;
    sp3C = arg0->unk1FC + @4155.unk88;
    sp40 = arg0->unk200;
    temp_f1 = GroundCross__4cBgSFP11cBgS_GndChk(&g_dComIfG_gameInfo.unk12A0, &sp14);
    if (@4155.unkC8 != temp_f1) {
        M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
        if (arg0->unk1FC == temp_f1) {
            vtable24 = &__vt__18dBgS_ObjGndChk_Spl;
            sp34 = &__vt__18dBgS_ObjGndChk_Spl.unkC;
            vtable54 = &__vt__18dBgS_ObjGndChk_Spl.unk18;
            sp60 = &__vt__18dBgS_ObjGndChk_Spl.unk24;
            vtable24 = &__vt__14dBgS_ObjGndChk;
            sp34 = &__vt__14dBgS_ObjGndChk.unkC;
            vtable54 = &__vt__14dBgS_ObjGndChk.unk18;
            sp60 = &__vt__14dBgS_ObjGndChk.unk24;
            vtable24 = &__vt__11dBgS_GndChk;
            sp34 = &__vt__11dBgS_GndChk.unkC;
            vtable54 = &__vt__11dBgS_GndChk.unk18;
            sp60 = &__vt__11dBgS_GndChk.unk24;
            if (&vtable54 != NULL) {
                vtable54 = &__vt__8dBgS_Chk;
                sp60 = &__vt__8dBgS_Chk.unkC;
                temp_cr0_eq = &sp60 == NULL;
                if (temp_cr0_eq == 0) {
                    sp60 = &__vt__15dBgS_GrpPassChk;
                    if (temp_cr0_eq == 0) {
                        sp60 = &__vt__15cBgS_GrpPassChk;
                    }
                }
                temp_cr0_eq_2 = &vtable54 == NULL;
                if (temp_cr0_eq_2 == 0) {
                    vtable54 = &__vt__16dBgS_PolyPassChk;
                    if (temp_cr0_eq_2 == 0) {
                        vtable54 = &__vt__16cBgS_PolyPassChk;
                    }
                }
            }
            vtable24 = &__vt__11cBgS_GndChk;
            sp34 = &__vt__11cBgS_GndChk.unkC;
            sp34 = &__vt__13cBgS_PolyInfo;
            __dt__8cBgS_ChkFv((cBgS_Chk *) &sp14, 0);
            return 1;
        }
    }
    if (daSea_ChkArea__Fff(arg0->unk1F8, arg0->unk200) != 0) {
        temp_f1_2 = daSea_calcWave__Fff(arg0->unk1F8, arg0->unk200) - @4155.unk40;
        M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
        if (arg0->unk1FC == temp_f1_2) {
            arg0->unk1FC = temp_f1_2;
            sp8 = arg0->unk1F8;
            spC = arg0->unk1FC;
            sp10 = arg0->unk200;
            spC = temp_f1_2;
            fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &sp8, @4155.unk14, @4155.unk14, 0);
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x6918U, &arg0->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) arg0->unk20A), @4155.unk14, @4155.unk14, @4155.unk4, @4155.unk4, 0U);
            vtable24 = &__vt__18dBgS_ObjGndChk_Spl;
            sp34 = &__vt__18dBgS_ObjGndChk_Spl.unkC;
            vtable54 = &__vt__18dBgS_ObjGndChk_Spl.unk18;
            sp60 = &__vt__18dBgS_ObjGndChk_Spl.unk24;
            vtable24 = &__vt__14dBgS_ObjGndChk;
            sp34 = &__vt__14dBgS_ObjGndChk.unkC;
            vtable54 = &__vt__14dBgS_ObjGndChk.unk18;
            sp60 = &__vt__14dBgS_ObjGndChk.unk24;
            vtable24 = &__vt__11dBgS_GndChk;
            sp34 = &__vt__11dBgS_GndChk.unkC;
            vtable54 = &__vt__11dBgS_GndChk.unk18;
            sp60 = &__vt__11dBgS_GndChk.unk24;
            if (&vtable54 != NULL) {
                vtable54 = &__vt__8dBgS_Chk;
                sp60 = &__vt__8dBgS_Chk.unkC;
                temp_cr0_eq_3 = &sp60 == NULL;
                if (temp_cr0_eq_3 == 0) {
                    sp60 = &__vt__15dBgS_GrpPassChk;
                    if (temp_cr0_eq_3 == 0) {
                        sp60 = &__vt__15cBgS_GrpPassChk;
                    }
                }
                temp_cr0_eq_4 = &vtable54 == NULL;
                if (temp_cr0_eq_4 == 0) {
                    vtable54 = &__vt__16dBgS_PolyPassChk;
                    if (temp_cr0_eq_4 == 0) {
                        vtable54 = &__vt__16cBgS_PolyPassChk;
                    }
                }
            }
            vtable24 = &__vt__11cBgS_GndChk;
            sp34 = &__vt__11cBgS_GndChk.unkC;
            sp34 = &__vt__13cBgS_PolyInfo;
            __dt__8cBgS_ChkFv((cBgS_Chk *) &sp14, 0);
            return 1;
        }
    }
    vtable24 = &__vt__18dBgS_ObjGndChk_Spl;
    sp34 = &__vt__18dBgS_ObjGndChk_Spl.unkC;
    vtable54 = &__vt__18dBgS_ObjGndChk_Spl.unk18;
    sp60 = &__vt__18dBgS_ObjGndChk_Spl.unk24;
    vtable24 = &__vt__14dBgS_ObjGndChk;
    sp34 = &__vt__14dBgS_ObjGndChk.unkC;
    vtable54 = &__vt__14dBgS_ObjGndChk.unk18;
    sp60 = &__vt__14dBgS_ObjGndChk.unk24;
    vtable24 = &__vt__11dBgS_GndChk;
    sp34 = &__vt__11dBgS_GndChk.unkC;
    vtable54 = &__vt__11dBgS_GndChk.unk18;
    sp60 = &__vt__11dBgS_GndChk.unk24;
    if (&vtable54 != NULL) {
        vtable54 = &__vt__8dBgS_Chk;
        sp60 = &__vt__8dBgS_Chk.unkC;
        temp_cr0_eq_5 = &sp60 == NULL;
        if (temp_cr0_eq_5 == 0) {
            sp60 = &__vt__15dBgS_GrpPassChk;
            if (temp_cr0_eq_5 == 0) {
                sp60 = &__vt__15cBgS_GrpPassChk;
            }
        }
        temp_cr0_eq_6 = &vtable54 == NULL;
        if (temp_cr0_eq_6 == 0) {
            vtable54 = &__vt__16dBgS_PolyPassChk;
            if (temp_cr0_eq_6 == 0) {
                vtable54 = &__vt__16cBgS_PolyPassChk;
            }
        }
    }
    vtable24 = &__vt__11cBgS_GndChk;
    sp34 = &__vt__11cBgS_GndChk.unkC;
    sp34 = &__vt__13cBgS_PolyInfo;
    __dt__8cBgS_ChkFv((cBgS_Chk *) &sp14, 0);
    return 0;
}

/* esa_s_sub (void *, void *) */
void *esa_s_sub__FPvPv(void *arg0, void *arg1) {
    if ((fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0xDE)) {
        return arg0;
    }
    return NULL;
}

/* action (pt_class *) */
void action__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    cXyz sp1C;
    cXyz sp10;
    f32 spC;
    f32 sp8;
    Vec *temp_r4;
    f32 *temp_r3_2;
    f32 temp_f1;
    f32 var_f1;
    f32 var_f1_2;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    s16 temp_r0;
    s32 var_r29;
    s32 var_r29_2;
    u32 temp_r3;
    u8 temp_r3_3;
    u8 temp_r4_2;

    temp_r3 = fpcEx_Search__FPFPvPv_PvPv(esa_s_sub__FPvPv, arg0);
    if (temp_r3 != 0U) {
        temp_r4 = temp_r3 + 0x1F8;
        arg0->unk2F8 = temp_r4;
        __mi__4cXyzCFRC3Vec(&sp10, temp_r4);
        sp28 = (bitwise f32) sp10;
        sp2C = sp14;
        sp30 = sp18;
        var_f1 = PSVECSquareMag(&sp28);
        if (var_f1 > 0.0f) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = @4155.unk28 * temp_f0 * (@4155.unk30 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = @4155.unk28 * temp_f0_2 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            spC = (f32) ((f64) var_f1 * (@4155.unk28 * temp_f0_3 * (@4155.unk30 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = spC;
        }
        arg0->unk300 = var_f1;
        sp2C = 0.0f;
        var_f1_2 = PSVECSquareMag(&sp28);
        if (var_f1_2 > 0.0f) {
            temp_f0_4 = __frsqrte(var_f1_2);
            temp_f0_5 = @4155.unk28 * temp_f0_4 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_4 * temp_f0_4)));
            temp_f0_6 = @4155.unk28 * temp_f0_5 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_5 * temp_f0_5)));
            sp8 = (f32) ((f64) var_f1_2 * (@4155.unk28 * temp_f0_6 * (@4155.unk30 - ((f64) var_f1_2 * (temp_f0_6 * temp_f0_6)))));
            var_f1_2 = sp8;
        }
        arg0->unk304 = var_f1_2;
        arg0->unk2FC = cM_atan2s__Fff(sp28, sp30);
    } else {
        arg0->unk2F8 = &g_dComIfG_gameInfo.unk5B44->unk1F8;
        arg0->unk300 = fopAcM_searchActorDistance__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
        arg0->unk304 = fopAcM_searchActorDistanceXZ__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
        arg0->unk2FC = fopAcM_searchActorAngleY__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) arg0, g_dComIfG_gameInfo.unk5B44);
    }
    var_r29 = 0;
    temp_r0 = arg0->unk2D2;
    switch (temp_r0) {                              /* irregular */
    case 0:
        pt_wait__FP8pt_class(arg0);
        break;
    case 1:
        pt_move__FP8pt_class(arg0);
        break;
    case 2:
        pt_attack__FP8pt_class(arg0);
        break;
    case 3:
        pt_koke__FP8pt_class(arg0);
        break;
    case 4:
        var_r29 = pt_ples__FP8pt_class(arg0);
        break;
    case 5:
        var_r29 = pt_bat__FP8pt_class(arg0);
        break;
    }
    damage_check__FP8pt_class(arg0);
    var_r29_2 = var_r29 + water_check__FP8pt_class(arg0);
    temp_f1 = arg0->unk320;
    if (temp_f1 > @4155.unkCC) {
        sp28 = 0.0f;
        sp2C = 0.0f;
        sp30 = -temp_f1;
        mDoMtx_YrotS__FPA4_fs(calc_mtx, arg0->unk324);
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp28, &sp1C);
        temp_r3_2 = &arg0->unk1F8;
        PSVECAdd((cXyz *) temp_r3_2, &sp1C.unk0, (cXyz *) temp_r3_2);
        cLib_addCalc0__FPfff(&arg0->unk320, @4155.unk14, @4155.unkA4);
        if (((s8) arg0->unk285 <= 0) && ((arg0->unk390 & 0x10) || (arg0->unk320 < @4155.unk7C))) {
            var_r29_2 += 1;
        }
        arg0->unk326 = 1;
    }
    cLib_addCalcAngleS2__FPssss(&arg0->unk20E, arg0->unk206, 4, 0x2000);
    cLib_addCalcAngleS2__FPssss(&arg0->unk20C.unk0, arg0->unk204, 4, 0x1000);
    cLib_addCalcAngleS2__FPssss(&arg0->unk210, arg0->unk208, 4, 0x1000);
    arg0->unk318 += arg0->unk31C;
    arg0->unk31C -= @4155.unk84;
    if (arg0->unk318 < 0.0f) {
        arg0->unk318 = 0.0f;
    }
    temp_r3_3 = arg0->unk7C0;
    if ((s8) temp_r3_3 != 0) {
        arg0->unk7C0 = temp_r3_3 - 1;
        if ((s8) arg0->unk7C0 == 0) {
            arg0->unk7C4.unk0->unk20(&arg0->unk7C4);
        }
    }
    if (var_r29_2 != 0) {
        fopAcM_createDisappear__FP10fopAc_ac_cP4cXyzUcUcUc((fopAc_ac_c *) arg0, (cXyz *) &arg0->unk260, 5U, 4U, 0xFFU);
        fopAcM_delete__FP10fopAc_ac_c((fopAc_ac_c *) arg0);
        onActor__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) arg0->unk1BC, (s32) (s8) arg0->unk1E2);
        if ((u8) arg0->unk2B4 == 0) {
            arg0->unk2BA = 1;
            return;
        }
        temp_r4_2 = arg0->unk2B8;
        if (temp_r4_2 != 0xFF) {
            onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4_2, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0);
        }
    }
}

/* daPt_Execute (pt_class *) */
s32 daPt_Execute__FP8pt_class(pt_class *arg0, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    cXyz sp8;
    J3DModel *temp_r30;
    f32 temp_f1;
    f32 temp_f2;
    mDoMtx_stack_c *temp_r3_2;
    s16 temp_r0;
    s16 temp_r3;
    s16 temp_r3_3;
    s16 temp_r4_2;
    s32 temp_r5;
    s32 var_ctr;
    s32 var_r3;
    u8 temp_r4;

    sp14 = 0.0f;
    sp18 = 0.0f;
    sp1C = 0.0f;
    if (enemy_ice__FP8enemyice(&arg0->unk7E4) != 0) {
        arg0->unk2C0->unk5C = 0;
        arg0->unk2C0->unk64 = @4155.unk18;
        play__14mDoExt_McaMorfFP3VecUlSc(arg0->unk2C0, &arg0->unk260, 0U, 0);
        PSMTXCopy(&now__14mDoMtx_stack_c, (f32 (*)[4]) arg0->unk2C0->unk50->unk24);
        calc__14mDoExt_McaMorfFv(arg0->unk2C0);
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &arg0->unk1F8, &arg0->unk10C);
    arg0->unk2D0 += 1;
    if (!(arg0->unk2D0 & 0x1F) && ((arg0->unk1FC - arg0->unk1D4) < @4155.unkD0)) {
        fopAcM_delete__FP10fopAc_ac_c((fopAc_ac_c *) arg0);
        onActor__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) arg0->unk1BC, (s32) (s8) arg0->unk1E2);
        if ((u8) arg0->unk2B4 == 0) {
            arg0->unk2BA = 1;
        } else {
            temp_r4 = arg0->unk2B8;
            if (temp_r4 != 0xFF) {
                onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0);
            }
        }
        return 1;
    }
    var_r3 = 0;
    var_ctr = 3;
    do {
        temp_r5 = var_r3 + 0x308;
        temp_r4_2 = *(arg0 + temp_r5);
        if (temp_r4_2 != 0) {
            *(arg0 + temp_r5) = temp_r4_2 - 1;
        }
        var_r3 += 2;
        var_ctr -= 1;
    } while (var_ctr != 0);
    temp_r3 = arg0->unk30E;
    if (temp_r3 != 0) {
        arg0->unk30E = temp_r3 - 1;
    }
    if ((u8) l_HIO.unk5 == 0) {
        action__FP8pt_class(arg0);
        if ((s8) arg0->unk2B9 != 0) {
            return 1;
        }
        if ((s8) arg0->unk326 != 0) {
            if (&arg0->unk52C != NULL) {
                arg0->unk1F8 += arg0->unk52C;
                arg0->unk1FC += arg0->unk530;
                arg0->unk200 += arg0->unk534;
            }
            arg0->unk314 = arg0->unk224;
            CrrPos__9dBgS_AcchFR4dBgS(&arg0->unk368, (dBgS *) &g_dComIfG_gameInfo.unk12A0);
        }
        temp_r3_2 = PSMTXTrans(&now__14mDoMtx_stack_c, arg0->unk1F8, arg0->unk1FC, arg0->unk200);
        temp_f2 = arg0->unk218;
        if (temp_f2 < @4155.unkD4) {
            temp_f1 = arg0->unk214;
            scaleM__14mDoMtx_stack_cFfff(temp_r3_2, temp_f1, temp_f2, temp_f1);
        }
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, arg0->unk20E);
        mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, arg0->unk20C.unk0);
        transM__14mDoMtx_stack_cFfff(mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, arg0->unk210), 0.0f, arg0->unk318, 0.0f);
        temp_r30 = arg0->unk2C0->unk50;
        PSMTXCopy(&now__14mDoMtx_stack_c, (f32 (*)[4]) temp_r30->unk24);
        temp_r3_3 = arg0->unk2D6;
        if (temp_r3_3 != 0) {
            arg0->unk2D6 = temp_r3_3 - 1;
            temp_r0 = arg0->unk2D6;
            if (temp_r0 <= 7) {
                arg0->unk2C4->unk4->unk10 = (f32) temp_r0;
            }
        } else {
            arg0->unk2D6 = (s16) (@4155.unk3C + cM_rndF__Ff(@4155.unk40));
        }
        play__14mDoExt_McaMorfFP3VecUlSc(arg0->unk2C0, &arg0->unk260, 0U, 0);
        arg0->unk2C8->unk4->unk10 = (f32) arg0->unk2CC;
        calc__14mDoExt_McaMorfFv(arg0->unk2C0);
        enemy_fire__FP9enemyfire(&arg0->unkB9C);
        PSMTXCopy(temp_r30->unk8C + 0x180, calc_mtx);
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp14, (cXyz *) &arg0->unk260);
        arg0->unk274 = arg0->unk260.unk0;
        arg0->unk278 = arg0->unk264;
        arg0->unk27C = arg0->unk268;
        arg0->unk278 += @4155.unk40;
        SetC__8cM3dGSphFRC4cXyz(&arg0->unk680, (cXyz *) &arg0->unk260);
        SetR__8cM3dGSphFf(&arg0->unk680, @4155.unk24);
        Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo.unk26A4, (cCcD_Obj *) &arg0->unk568);
        if ((s8) arg0->unk327 != 0) {
            PSMTXCopy(temp_r30->unk8C + 0x2D0, calc_mtx);
            sp14 = @4155.unk10;
            sp18 = 0.0f;
            sp1C = 0.0f;
            MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp14, &sp8);
            arg0->unk327 = 0;
        } else {
            sp8 = @4155.unkD8;
            spC = @4155.unkDC;
            sp10 = @4155.unkD8;
        }
        SetC__8cM3dGSphFRC4cXyz(&arg0->unk7AC, &sp8);
        Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo.unk26A4, (cCcD_Obj *) &arg0->unk694);
        goto block_32;
    }
block_32:
    return 1;
}

/* daPt_IsDelete (pt_class *) */
s32 daPt_IsDelete__FP8pt_class(pt_class *arg0) {
    return 1;
}

/* daPt_Delete (pt_class *) */
s32 daPt_Delete__FP8pt_class(pt_class *arg0) {
    u8 temp_r0;
    u8 temp_r4;
    u8 temp_r4_2;
    void *temp_r3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&arg0->unk2AC, &"Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93"->unk0);
    if ((u8) arg0->unkDC4 != 0) {
        hio_set = 0;
        deleteChild__16mDoHIO_subRoot_cFSc(&mDoHIO_root + 4, (s8) l_HIO.unk4);
    }
    arg0->unk7C4.unk0->unk20(&arg0->unk7C4);
    enemy_fire_remove__FP9enemyfire(&arg0->unkB9C);
    if (((s8) arg0->unk2BA != 0) && ((temp_r4 = arg0->unk2B7, ((temp_r4 == 0xFF) != 0)) || (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) arg0->unk20A) != 0)) && ((temp_r4_2 = arg0->unk2B8, ((temp_r4_2 == 0xFF) != 0)) || (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4_2, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) == 0))) {
        temp_r3 = fopAcM_CreateAppend__Fv();
        temp_r3->unk4 = (f32) arg0->unk1D0;
        temp_r3->unk8 = (f32) arg0->unk1D4;
        temp_r3->unkC = (f32) arg0->unk1D8;
        temp_r3->unk10 = (s16) arg0->unk1DC;
        temp_r3->unk12 = (s16) arg0->unk1DE;
        temp_r3->unk14 = (s16) arg0->unk1E0;
        temp_r0 = arg0->unk2B6;
        if (temp_r0 != 7) {
            temp_r3->unk10 = (s16) ((temp_r0 * 0x14) + 0x14);
        }
        temp_r3->unk0 = (s32) (arg0->unkB0 | 0x10);
        temp_r3->unk21 = (u8) arg0->unk20A;
        fpcSCtRq_Request__FP11layer_classsPFPvPv_iPvPv(fpcLy_CurrentLayer__Fv(), 0xF9, NULL, NULL, temp_r3);
    }
    return 1;
}

/* useHeapInit (fopAc_ac_c *) */
s32 useHeapInit__FP10fopAc_ac_c(fopAc_ac_c *arg0, ? arg_sp0) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    J3DAnmTransform *temp_r29;
    J3DAnmTransform *temp_r5;
    J3DAnmTransform *temp_r5_2;
    J3DAnmTransform *temp_r5_3;
    mDoExt_McaMorf *temp_r3;
    mDoExt_McaMorf *temp_r3_2;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *var_r31;

    var_r31 = __nw__FUl(0xB4U);
    if (var_r31 != NULL) {
        temp_r29 = getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *)0xC, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        temp_r5 = getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *)0xF, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0;
        sp18 = 0x11020203;
        var_r31 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(var_r31, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r5, NULL, NULL, (s32) temp_r29, 1.0f, 0, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    arg0->unk2C0 = var_r31;
    temp_r3 = arg0->unk2C0;
    if ((temp_r3 == NULL) || ((J3DModel *) temp_r3->unk50 == NULL)) {
        return 0;
    }
    temp_r3_2 = __nw__FUl(0x14U);
    if (temp_r3_2 != NULL) {
        temp_r3_2->vtable0 = &__vt__14mDoExt_baseAnm;
        temp_r3_2->unk4 = 0;
        temp_r3_2->vtable0 = (struct __vt__14mDoExt_baseAnm *) &__vt__13mDoExt_btpAnm;
        temp_r3_2->unkC = 0;
    }
    arg0->unk2C4 = temp_r3_2;
    if ((mDoExt_McaMorf *) arg0->unk2C4 == NULL) {
        return 0;
    }
    temp_r5_2 = getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *)0x15, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    sp8 = 0;
    if (init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi((mDoExt_btpAnm *) arg0->unk2C4, arg0->unk2C0->unk50->unk4, (J3DAnmTexPattern *) temp_r5_2, 1, 0, 1.0f, 0, -1, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) == 0) {
        return 0;
    }
    temp_r3_3 = __nw__FUl(0x18U);
    if (temp_r3_3 != NULL) {
        temp_r3_3->vtable0 = &__vt__14mDoExt_baseAnm;
        temp_r3_3->unk4 = 0;
        temp_r3_3->vtable0 = (struct __vt__14mDoExt_baseAnm *) &__vt__13mDoExt_brkAnm;
        temp_r3_3->unkC = 0;
        temp_r3_3->unk10 = 0;
    }
    arg0->unk2C8 = temp_r3_3;
    if ((mDoExt_McaMorf *) arg0->unk2C8 == NULL) {
        return 0;
    }
    temp_r5_3 = getRes__14dRes_control_cFPCclP11dRes_info_ci("Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93", (s8 *)0x12, (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    sp8 = 0;
    if (init__13mDoExt_brkAnmFP12J3DModelDataP15J3DAnmTevRegKeyiifssbi((mDoExt_brkAnm *) arg0->unk2C8, arg0->unk2C0->unk50->unk4, (J3DAnmTevRegKey *) temp_r5_3, 1, 0, 1.0f, 0, -1, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) == 0) {
        return 0;
    }
    arg0->unk2CC = (s32) cM_rndF__Ff(3.999f);
    return 1;
}

/* daPt_Create (fopAc_ac_c *) */
s32 daPt_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0, ? arg_sp0) {
    s32 sp8;
    s32 temp_r3;
    s32 var_ctr;
    s32 var_r3;
    s32 var_r7;
    u8 *var_r6;
    u8 temp_r4;

    if (!(arg0->unk1C8 & 8)) {
        if (arg0 != NULL) {
            __ct__8pt_classFv((pt_class *) arg0);
        }
        arg0->unk1C8 |= 8;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&arg0->unk2AC, &"Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93"->unk0);
    if (temp_r3 == 4) {
        arg0->unk2B4 = arg0->unkB0 & 0xF;
        arg0->unk2B6 = ((u32) arg0->unkB0 >> 5U) & 7;
        arg0->unk2B5 = (u8) ((u32) arg0->unkB0 >> 8U);
        if (arg0->unkB0 & 0x10) {
            arg0->unk2B9 = 1;
        }
        arg0->unk2D8 = arg0->unk204;
        arg0->unk204 = 0;
        arg0->unk2B8 = (u8) ((u32) arg0->unkB0 >> 0x10U);
        arg0->unk2B7 = (u8) ((u32) arg0->unkB0 >> 0x18U);
        temp_r4 = arg0->unk2B7;
        if ((temp_r4 != 0xFF) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) mStayNo__20dStage_roomControl_c.unk0) == 0)) {
            arg0->unk2B9 = 1;
        }
        arg0->unk288 = GetIndex__12cDT_NamePTblCFPCci(&g_dComIfG_gameInfo.unk50AC, &"Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93"->unk3, 0);
        if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(arg0, useHeapInit__FP10fopAc_ac_c, 0x4B000U) == 0) {
            return 5;
        }
        if ((u8) hio_set == 0) {
            arg0->unkDC4 = 1;
            hio_set = 1;
            l_HIO.unk4 = createChild__16mDoHIO_subRoot_cFPCcP13JORReflexible(&mDoHIO_root + 4, &"Pt\0Puti\0\x83v\x83`\x83u\x83\x8a\x83\x93"->unk8, &l_HIO);
        }
        arg0->unk1C4 |= 0x100;
        arg0->unk280 = 4;
        arg0->unk2D2 = 0;
        arg0->unk22C = arg0->unk2C0->unk50->unk24;
        sp8 = 0;
        Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&arg0->unk368, (cXyz *) &arg0->unk1F8, &arg0->unk1E4, arg0, 1, &arg0->unk328, &arg0->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
        SetWall__12dBgS_AcchCirFff(&arg0->unk328, @4155.unk10, @4155.unk10);
        arg0->unk285 = 2;
        arg0->unk284 = 2;
        Init__9dCcD_SttsFiiP10fopAc_ac_c(&arg0->unk52C, 0x64, 0, arg0);
        Set__8dCcD_SphFRC11dCcD_SrcSph(&arg0->unk568, &@2100 + 0x64);
        arg0->unk5AC = &arg0->unk52C;
        Set__8dCcD_SphFRC11dCcD_SrcSph(&arg0->unk694, &@2100 + 0xA4);
        arg0->unk6D8 = &arg0->unk52C;
        arg0->unk28D = 2;
        arg0->unk7E4 = arg0;
        arg0->unk984 = @4155.unk40 + g_regHIO.unk1C;
        arg0->unk980 = @4155.unk40 + g_regHIO.unk20;
        arg0->unk990 = @4155.unkEC + g_regHIO.unk2C;
        arg0->unk7EC = @4155.unk24 + g_regHIO.unk30;
        arg0->unkBA8 = arg0->unk2C0;
        arg0->unkB9C = arg0;
        var_r7 = 0;
        var_r3 = 0;
        var_r6 = &@2100 + 0xE4;
        var_ctr = 0xA;
        do {
            *(arg0 + (var_r7 + 0xBAC)) = *var_r6;
            *(arg0 + (var_r3 + 0xBB8)) = *(&@2100 + 0xF0 + var_r3);
            var_r7 += 1;
            var_r3 += 4;
            var_r6 += 1;
            var_ctr -= 1;
        } while (var_ctr != 0);
        daPt_Execute__FP8pt_class((pt_class *) arg0);
        goto block_17;
    }
block_17:
    return temp_r3;
}

/* d_a_pt_cpp::__sinit void (*) (void) */
void __sinit_d_a_pt_cpp(void) {
    f32 sp94;
    f32 sp90;
    f32 sp8C;
    f32 sp88;
    f32 sp84;
    f32 sp80;
    f32 sp7C;
    f32 sp78;
    f32 sp74;
    f32 sp70;
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    f32 sp60;
    f32 sp5C;
    f32 sp58;
    f32 sp54;
    f32 sp50;
    f32 sp4C;
    f32 sp48;
    f32 sp44;
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    void *(*temp_r3)(daPt_HIO_c *, s16);
    void *(*temp_r3_7)(daPt_HIO_c *, s16);
    void *temp_r3_10;
    void *temp_r3_11;
    void *temp_r3_12;
    void *temp_r3_2;
    void *temp_r3_3;
    void *temp_r3_4;
    void *temp_r3_5;
    void *temp_r3_6;
    void *temp_r3_8;
    void *temp_r3_9;

    __ct__10daPt_HIO_cFv(&@3569 + 0x58);
    __register_global_object(__dt__10daPt_HIO_cFv, &@3569 + 0x4C);
    sp8C = @4155.unk8;
    sp90 = 0.0f;
    sp94 = 0.0f;
    @3569.unkA8 = (f32) @4155.unk8;
    temp_r3 = &@3569 + 0xA8;
    temp_r3->unk4 = 0.0f;
    temp_r3->unk8 = 0.0f;
    __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x60, @4155.unk8);
    sp80 = @4155.unk8C;
    sp84 = 0.0f;
    sp88 = 0.0f;
    temp_r3_2 = &@3569 + 0xA8;
    temp_r3_2->unkC = (f32) @4155.unk8C;
    temp_r3_2->unk10 = 0.0f;
    temp_r3_2->unk14 = 0.0f;
    __register_global_object(temp_r3_2 + 0xC, __dt__4cXyzFv, &@3569 + 0x6C, @4155.unk8C);
    sp74 = 0.0f;
    sp78 = @4155.unk8;
    sp7C = 0.0f;
    temp_r3_3 = &@3569 + 0xA8;
    temp_r3_3->unk18 = 0.0f;
    temp_r3_3->unk1C = (f32) @4155.unk8;
    temp_r3_3->unk20 = 0.0f;
    __register_global_object(temp_r3_3 + 0x18, __dt__4cXyzFv, &@3569 + 0x78, 0.0f);
    sp68 = 0.0f;
    sp6C = @4155.unk8C;
    sp70 = 0.0f;
    temp_r3_4 = &@3569 + 0xA8;
    temp_r3_4->unk24 = 0.0f;
    temp_r3_4->unk28 = (f32) @4155.unk8C;
    temp_r3_4->unk2C = 0.0f;
    __register_global_object(temp_r3_4 + 0x24, __dt__4cXyzFv, &@3569 + 0x84, 0.0f);
    sp5C = 0.0f;
    sp60 = 0.0f;
    sp64 = @4155.unk8;
    temp_r3_5 = &@3569 + 0xA8;
    temp_r3_5->unk30 = 0.0f;
    temp_r3_5->unk34 = 0.0f;
    temp_r3_5->unk38 = (f32) @4155.unk8;
    __register_global_object(temp_r3_5 + 0x30, __dt__4cXyzFv, &@3569 + 0x90, 0.0f);
    sp50 = 0.0f;
    sp54 = 0.0f;
    sp58 = @4155.unk8C;
    temp_r3_6 = &@3569 + 0xA8;
    temp_r3_6->unk3C = 0.0f;
    temp_r3_6->unk40 = 0.0f;
    temp_r3_6->unk44 = (f32) @4155.unk8C;
    __register_global_object(temp_r3_6 + 0x3C, __dt__4cXyzFv, &@3569 + 0x9C, 0.0f);
    sp44 = @4155.unkF0;
    sp48 = 0.0f;
    sp4C = 0.0f;
    @3569.unk138 = (f32) @4155.unkF0;
    temp_r3_7 = &@3569 + 0x138;
    temp_r3_7->unk4 = 0.0f;
    temp_r3_7->unk8 = 0.0f;
    __register_global_object(temp_r3_7, __dt__4cXyzFv, &@3569 + 0xF0, @4155.unkF0);
    sp38 = @4155.unk18;
    sp3C = 0.0f;
    sp40 = 0.0f;
    temp_r3_8 = &@3569 + 0x138;
    temp_r3_8->unkC = (f32) @4155.unk18;
    temp_r3_8->unk10 = 0.0f;
    temp_r3_8->unk14 = 0.0f;
    __register_global_object(temp_r3_8 + 0xC, __dt__4cXyzFv, &@3569 + 0xFC, @4155.unk18);
    sp2C = 0.0f;
    sp30 = @4155.unkF0;
    sp34 = 0.0f;
    temp_r3_9 = &@3569 + 0x138;
    temp_r3_9->unk18 = 0.0f;
    temp_r3_9->unk1C = (f32) @4155.unkF0;
    temp_r3_9->unk20 = 0.0f;
    __register_global_object(temp_r3_9 + 0x18, __dt__4cXyzFv, &@3569 + 0x108, 0.0f);
    sp20 = 0.0f;
    sp24 = @4155.unk18;
    sp28 = 0.0f;
    temp_r3_10 = &@3569 + 0x138;
    temp_r3_10->unk24 = 0.0f;
    temp_r3_10->unk28 = (f32) @4155.unk18;
    temp_r3_10->unk2C = 0.0f;
    __register_global_object(temp_r3_10 + 0x24, __dt__4cXyzFv, &@3569 + 0x114, 0.0f);
    sp14 = 0.0f;
    sp18 = 0.0f;
    sp1C = @4155.unkF0;
    temp_r3_11 = &@3569 + 0x138;
    temp_r3_11->unk30 = 0.0f;
    temp_r3_11->unk34 = 0.0f;
    temp_r3_11->unk38 = (f32) @4155.unkF0;
    __register_global_object(temp_r3_11 + 0x30, __dt__4cXyzFv, &@3569 + 0x120, 0.0f);
    sp8 = 0.0f;
    spC = 0.0f;
    sp10 = @4155.unk18;
    temp_r3_12 = &@3569 + 0x138;
    temp_r3_12->unk3C = 0.0f;
    temp_r3_12->unk40 = 0.0f;
    temp_r3_12->unk44 = (f32) @4155.unk18;
    __register_global_object(temp_r3_12 + 0x3C, __dt__4cXyzFv, &@3569 + 0x12C, 0.0f);
}