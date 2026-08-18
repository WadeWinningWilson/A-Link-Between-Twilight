// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in naming-map-auto.md;
// remaining unkNNN are still inference-needed. Behavior identical to the full draft.
// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_yw1 (83 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_yw1\asm\d\actor\d_a_npc_yw1.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DAnmTexPattern {
    /* 0x0 */ char pad0[6];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x6 */ s16 unk6;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
} J3DAnmTexPattern;                                 /* size >= 0x8 */

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ void *unk4;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0xC];                      /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x14 */ daNpc_Yw1_c *unk14;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x18 */ f32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ f32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ f32 unk20;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x24 */ mDoMtx_stack_c unk24;                /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x24 */ char pad24[0x68];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x8C */ s32 unk8C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8C (receipt f_op_actor.h) */
} J3DModel;                                         /* size >= 0x90 */

typedef struct J3DModelData {
    /* 0x00 */ char pad0[0x58];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x58 */ J3DMaterialTable unk58;              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x58 */ char pad58[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
} J3DModelData;                                     /* size >= 0x59 */

typedef struct J3DNode {
    /* 0x00 */ char pad0[0x18];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x18 */ u16 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
} J3DNode;                                          /* size >= 0x1A */

typedef struct JUTAssertion {
    /* 0x00 */ char pad0[0x54];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x54 */ JUTNameTab *unk54;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
} JUTAssertion;                                     /* size >= 0x58 */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0x4 */

struct __vt__11daNpc_Yw1_c {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ s32 (*next_msgStatus)(daNpc_Yw1_c *, u32 *);
    /* 0x0C */ s32 (*getMsg)(daNpc_Yw1_c *);
    /* 0x10 */ void (*anmAtr)(daNpc_Yw1_c *, u16);
};                                                  /* size = 0x14 */

struct __vt__12dBgS_ObjAcch {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dBgS_ObjAcch *, s16);
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ void *(*@20@__dt)(dBgS_ObjAcch *, s16);
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ void *(*@32@__dt)(dBgS_ObjAcch *, s16);
};                                                  /* size = 0x24 */

struct __vt__12fopNpc_npc_c {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ ? (*next_msgStatus)(fopNpc_npc_c *, u32 *);
    /* 0x0C */ ? (*getMsg)(fopNpc_npc_c *);
    /* 0x10 */ ? (*anmAtr)(fopNpc_npc_c *, u16);
};                                                  /* size = 0x14 */

struct __vt__13mDoExt_btpAnm {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoExt_btpAnm *, s16);
};                                                  /* size = 0xC */

struct __vt__14cCcD_ShapeAttr {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(cCcD_ShapeAttr *, s16);
    /* 0x0C */ ? (*CrossAtTg)(cCcD_ShapeAttr *, cCcD_ShapeAttr *, cXyz *);
    /* 0x10 */ s32 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x14 */ s32 unk14;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x18 */ s32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ s32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ s32 unk20;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x24 */ s32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ ? (*CrossCo)(cCcD_ShapeAttr *, cCcD_ShapeAttr *, f32 *);
    /* 0x2C */ s32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ s32 unk30;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x34 */ s32 unk34;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x38 */ s32 unk38;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
    /* 0x3C */ s32 unk3C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3C (receipt f_op_actor.h) */
    /* 0x40 */ s32 unk40;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x40 (receipt f_op_actor.h) */
    /* 0x44 */ ? (*GetCoCP)(cCcD_ShapeAttr *);
    /* 0x48 */ ? (*GetCoCP)(cCcD_ShapeAttr *);
    /* 0x4C */ s32 unk4C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4C (receipt f_op_actor.h) */
    /* 0x50 */ s32 unk50;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
};                                                  /* size = 0x54 */

struct __vt__14mDoExt_baseAnm {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoExt_baseAnm *, s16);
};                                                  /* size = 0xC */

struct __vt__14mDoHIO_entry_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoHIO_entry_c *, s16);
};                                                  /* size = 0xC */

struct __vt__15daNpc_Yw1_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Yw1_HIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__20daNpc_Yw1_childHIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Yw1_childHIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__8cM3dGAab {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cM3dGAab *, s16);
};                                                  /* size = 0xC */

struct __vt__8cM3dGCyl {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(cM3dGCyl *, s16);
};                                                  /* size = 0xC */

struct __vt__8dCcD_Cyl {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ void *(*__dt)(dCcD_Cyl *, s16);
    /* 0x0C */ ? (*GetGObjInf)(cCcD_GObjInf *);
    /* 0x10 */ ? (*GetGObjInf)(dCcD_GObjInf *);
    /* 0x14 */ ? (*GetShapeAttr)(cCcD_Obj *);
    /* 0x18 */ ? (*GetShapeAttr)(dCcD_Cyl *);
    /* 0x1C */ ? (*ClrAtHit)(dCcD_GObjInf *);
    /* 0x20 */ ? (*ClrTgHit)(dCcD_GObjInf *);
    /* 0x24 */ ? (*ClrCoHit)(dCcD_GObjInf *);
    /* 0x28 */ s32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ s32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ void *(*@248@__dt)(dCcD_Cyl *, s16);
    /* 0x34 */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_ShapeAttr *, cXyz *);
    /* 0x38 */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_PntAttr *, cXyz *);
    /* 0x3C */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_CpsAttr *, cXyz *);
    /* 0x40 */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_TriAttr *, cXyz *);
    /* 0x44 */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_AabAttr *, cXyz *);
    /* 0x48 */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_CylAttr *, cXyz *);
    /* 0x4C */ ? (*CrossAtTg)(cCcD_CylAttr *, cCcD_SphAttr *, cXyz *);
    /* 0x50 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_ShapeAttr *, f32 *);
    /* 0x54 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_PntAttr *, f32 *);
    /* 0x58 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_CpsAttr *, f32 *);
    /* 0x5C */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_TriAttr *, f32 *);
    /* 0x60 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_AabAttr *, f32 *);
    /* 0x64 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_CylAttr *, f32 *);
    /* 0x68 */ ? (*CrossCo)(cCcD_CylAttr *, cCcD_SphAttr *, f32 *);
    /* 0x6C */ ? (*GetCoCP)(cCcD_CylAttr *);
    /* 0x70 */ ? (*GetCoCP)(cCcD_CylAttr *);
    /* 0x74 */ ? (*CalcAabBox)(cCcD_CylAttr *);
    /* 0x78 */ ? (*GetNVec)(cCcD_CylAttr *, cXyz *, cXyz *);
    /* 0x7C */ s32 unk7C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x7C (receipt f_op_actor.h) */
    /* 0x80 */ s32 unk80;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x80 (receipt f_op_actor.h) */
    /* 0x84 */ void *(*@280@__dt)(dCcD_Cyl *, s16);
};                                                  /* size = 0x88 */

struct _struct_a_anm_prm_tbl$4595_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4602_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4640_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_l_HIO_0x38 {
    /* 0x00 */ char pad0[0x10];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x10 */ s16 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x12 */ s16 unk12;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x12 (receipt f_op_actor.h) */
    /* 0x14 */ s16 unk14;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x16 */ s16 unk16;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x16 (receipt f_op_actor.h) */
    /* 0x18 */ s16 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1A */ s16 unk1A;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1A (receipt f_op_actor.h) */
    /* 0x1C */ s16 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x1E */ s16 unk1E;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1E (receipt f_op_actor.h) */
    /* 0x20 */ s16 unk20;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x22 */ s16 unk22;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x22 (receipt f_op_actor.h) */
    /* 0x24 */ f32 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ u8 unk28;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x29 */ char pad29[3];                       /* maybe part of unk28[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x29 (receipt f_op_actor.h) */
    /* 0x2C */ f32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ s16 unk30;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x32 */ s16 unk32;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x32 (receipt f_op_actor.h) */
    /* 0x34 */ f32 unk34;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
};                                                  /* size = 0x38 */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct dCcD_GStts {
    /* 0x0 */ void *unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dCcD_GStts;                                       /* size >= 0x4 */

typedef struct dNpc_EventCut_c {
    /* 0x0 */ s8 *unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_EventCut_c;                                  /* size >= 0x4 */

typedef struct daNpc_Yw1_HIO_c {
    /* 0x00 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
    /* 0x04 */ s8 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x05 */ char pad5[3];                        /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x08 */ s32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ ? unkC;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x0C */ char padC[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ ? unk10;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x10 */ char pad10[0x30];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x40 */ s32 unk40;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x40 (receipt f_op_actor.h) */
} daNpc_Yw1_HIO_c;                                  /* size >= 0x44 */

typedef struct daNpc_Yw1_c {
    /* 0x000 */ char pad0[0xB0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x0B0 */ u32 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
    /* 0x0B4 */ char padB4[0x3C];                   /* maybe part of unkB0[0x10]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB4 (receipt f_op_actor.h) */
    /* 0x0F0 */ u32 unkF0;                          /* inferred */  /* = fopAc_ac_c::JKRSolidHeap* heap @0xF0 (receipt f_op_actor.h) */
    /* 0x0F4 */ char padF4[4];  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 (receipt f_op_actor.h) */
    /* 0x0F8 */ u16 unkF8;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x4 (receipt f_op_actor.h) */
    /* 0x0FA */ u16 unkFA;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x6 (receipt f_op_actor.h) */
    /* 0x0FC */ char padFC[0x10];                   /* maybe part of unkFA[9]? */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x8 (receipt f_op_actor.h) */
    /* 0x10C */ dKy_tevstr_c unk10C;                /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x10C */ char pad10C[0xA9];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x1B5 */ s8 unk1B5;                          /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0xA9 (receipt f_op_actor.h) */
    /* 0x1B6 */ s8 unk1B6;                          /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0xAA (receipt f_op_actor.h) */
    /* 0x1B7 */ char pad1B7[9];                     /* maybe part of unk1B6[0xA]? */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0xAB (receipt f_op_actor.h) */
    /* 0x1C0 */ u8 unk1C0;                          /* inferred */  /* = fopAc_ac_c::u8 demoActorID @0x1C0 (receipt f_op_actor.h) */
    /* 0x1C1 */ char pad1C1[3];                     /* maybe part of unk1C0[4]? */  /* = fopAc_ac_c::s8 argument @0x1C1 (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ s32 unk1C8;                         /* inferred */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1CC */ char pad1CC[0x18];                  /* maybe part of unk1C8[7]? */  /* = fopAc_ac_c::fpc_ProcID parentActorID @0x1CC (receipt f_op_actor.h) */
    /* 0x1E4 */ cXyz unk1E4;                        /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1E4 */ char pad1E4[0x14];  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1F8 */ cXyz unk1F8;                        /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1F8 */ char pad1F8[4];  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ s16 unk204;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x206 */ s16 unk206;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xE (receipt f_op_actor.h) */
    /* 0x208 */ s16 unk208;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x10 (receipt f_op_actor.h) */
    /* 0x20A */ u8 unk20A;                          /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x12 (receipt f_op_actor.h) */
    /* 0x20B */ char pad20B[1];  /* = fopAc_ac_c::actor_place current @0x1F8 +0x13 (receipt f_op_actor.h) */
    /* 0x20C */ s16 unk20C;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
    /* 0x20E */ s16 unk20E;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x2 (receipt f_op_actor.h) */
    /* 0x210 */ s16 unk210;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x212 */ char pad212[2];  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x6 (receipt f_op_actor.h) */
    /* 0x214 */ f32 unk214;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 (receipt f_op_actor.h) */
    /* 0x218 */ f32 unk218;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 +0x4 (receipt f_op_actor.h) */
    /* 0x21C */ f32 unk21C;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 +0x8 (receipt f_op_actor.h) */
    /* 0x220 */ cXyz unk220;                        /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x220 */ char pad220[0xC];  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x22C */ mDoMtx_stack_c *unk22C;             /* inferred */  /* = fopAc_ac_c::MtxP cullMtx @0x22C (receipt f_op_actor.h) */
    /* 0x230 */ char pad230[0x24];                  /* maybe part of unk22C[0xA]? */  /* = fopAc_ac_c::fopAc_cullSizeSphere sphere @0x230 (receipt f_op_actor.h) */
    /* 0x254 */ f32 unk254;                         /* inferred */  /* = fopAc_ac_c::f32 speedF @0x254 (receipt f_op_actor.h) */
    /* 0x258 */ f32 unk258;                         /* inferred */  /* = fopAc_ac_c::f32 gravity @0x258 (receipt f_op_actor.h) */
    /* 0x25C */ char pad25C[4];  /* = fopAc_ac_c::f32 maxFallSpeed @0x25C (receipt f_op_actor.h) */
    /* 0x260 */ Vec unk260;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x260 */ char pad260[4];  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x264 */ f32 unk264;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x4 (receipt f_op_actor.h) */
    /* 0x268 */ f32 unk268;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x8 (receipt f_op_actor.h) */
    /* 0x26C */ char pad26C[1];  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C (receipt f_op_actor.h) */
    /* 0x26D */ s8 unk26D;                          /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x1 (receipt f_op_actor.h) */
    /* 0x26E */ char pad26E[1];  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x2 (receipt f_op_actor.h) */
    /* 0x26F */ s8 unk26F;                          /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x3 (receipt f_op_actor.h) */
    /* 0x270 */ char pad270[4];                     /* maybe part of unk26F[5]? */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x4 (receipt f_op_actor.h) */
    /* 0x274 */ f32 unk274;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x8 (receipt f_op_actor.h) */
    /* 0x278 */ f32 unk278;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0xC (receipt f_op_actor.h) */
    /* 0x27C */ f32 unk27C;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x10 (receipt f_op_actor.h) */
    /* 0x280 */ s32 unk280;                         /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x14 (receipt f_op_actor.h) */
    /* 0x284 */ char pad284[0xC];                   /* maybe part of unk280[4]? */  /* = fopAc_ac_c::s8 max_health @0x284 (receipt f_op_actor.h) */
    /* 0x290 */ s16 unk290;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x292 */ s16 unk292;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x294 */ s16 unk294;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x296 */ s16 unk296;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x298 */ char pad298[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29A */ u8 unk29A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29B */ s8 unk29B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29C */ s8 unk29C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29D */ char pad29D[0x27];                  /* maybe part of unk29C[0x28]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ dNpc_EventCut_c unk2C4;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ char pad2C4[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2CC */ s32 unk2CC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D0 */ s32 unk2D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D4 */ char pad2D4[0x58];                  /* maybe part of unk2D0[0x17]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x32C */ s32 unk32C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x330 */ mDoExt_McaMorf *unk330;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x334 */ dBgS_Acch unk334;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x334 */ char pad334[0x10];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x344 */ struct __vt__12dBgS_ObjAcch *vtable344; /* inferred */
    /* 0x348 */ s32 *unk348;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x34C */ s8 unk34C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x34D */ char pad34D[7];                     /* maybe part of unk34C[8]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x354 */ s32 *unk354;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x358 */ char pad358[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x35C */ s32 unk35C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x360 */ char pad360[0x68];                  /* maybe part of unk35C[0x1B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3C8 */ f32 unk3C8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3CC */ char pad3CC[0x50];                  /* maybe part of unk3C8[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ cBgS_PolyInfo unk41C;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ char pad41C[0xDC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F8 */ dBgS_AcchCir unk4F8;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F8 */ char pad4F8[0x40];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ dCcD_Stts unk538;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ char pad538[0x18];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x550 */ ? *unk550;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x554 */ dCcD_GStts unk554;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x554 */ char pad554[0x20];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ dCcD_Cyl unk574;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ char pad574[0x3C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5B0 */ struct __vt__8dCcD_Cyl *vtable5B0;  /* inferred */
    /* 0x5B4 */ char pad5B4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5B8 */ dCcD_Stts *unk5B8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5BC */ char pad5BC[0xC8];                  /* maybe part of unk5B8[0x33]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x684 */ struct __vt__8cM3dGAab *vtable684;  /* inferred */
    /* 0x688 */ struct __vt__14cCcD_ShapeAttr *vtable688; /* inferred */
    /* 0x68C */ char pad68C[0x14];                  /* maybe part of vtable688[6]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A0 */ struct __vt__8cM3dGCyl *vtable6A0;  /* inferred */
    /* 0x6A4 */ s32 unk6A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A8 */ char pad6A8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6AC */ s32 unk6AC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B0 */ void *unk6B0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B4 */ char pad6B4[0xC];                   /* maybe part of unk6B0[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C0 */ struct __vt__12fopNpc_npc_c *vtable6C0; /* inferred */
    /* 0x6C4 */ request_of_phase_process_class unk6C4; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ char pad6C4[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CC */ u8 unk6CC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CD */ u8 unk6CD;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CE */ u8 unk6CE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CF */ u8 unk6CF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D0 */ u8 unk6D0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D1 */ char pad6D1[3];                     /* maybe part of unk6D0[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ u32 unk6D4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ void **unk6D8;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ mDoExt_btpAnm unk6DC;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ char pad6DC[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E0 */ s32 unk6E0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E4 */ J3DAnmTexPattern *unk6E4;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E8 */ s32 unk6E8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6EC */ char pad6EC[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F0 */ u8 unk6F0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F1 */ char pad6F1[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F2 */ s16 unk6F2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F4 */ ? unk6F4;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F4 */ char pad6F4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ s32 unk6F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6FC */ s32 unk6FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x700 */ u32 unk700;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x704 */ u32 unk704;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x708 */ u32 unk708;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70C */ u32 unk70C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x710 */ char pad710[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x711 */ u8 unk711;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x712 */ u8 unk712;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x713 */ char pad713[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x714 */ f32 unk714;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x718 */ f32 unk718;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x71C */ f32 unk71C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x720 */ s16 unk720;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x722 */ s16 unk722;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x724 */ s16 unk724;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x726 */ s16 unk726;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x728 */ s16 unk728;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72A */ s16 unk72A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72C */ Vec unk72C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72C */ char pad72C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x730 */ f32 unk730;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x734 */ f32 unk734;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x738 */ f32 unk738;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x73C */ f32 unk73C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x740 */ f32 unk740;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x744 */ f32 unk744;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x748 */ f32 unk748;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x74C */ f32 unk74C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x750 */ f32 unk750;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x754 */ char pad754[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x758 */ s16 unk758;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75A */ s16 unk75A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75C */ s16 unk75C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75E */ char pad75E[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x760 */ s32 unk760;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x764 */ char pad764[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x766 */ s16 unk766;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x768 */ char pad768[4];                     /* maybe part of unk766[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76C */ s16 unk76C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76E */ s16 unk76E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x770 */ s8 unk770;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x771 */ s8 unk771;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x772 */ u8 unk772;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x773 */ u8 unk773;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x774 */ u8 unk774;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x775 */ u8 unk775;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x776 */ u8 unk776;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x777 */ u8 unk777;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x778 */ u8 unk778;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x779 */ u8 unk779;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77A */ u8 unk77A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77B */ u8 unk77B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77C */ u8 unk77C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77D */ u8 unk77D;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77E */ u8 unk77E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77F */ char pad77F[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x780 */ s16 unk780;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x782 */ s16 unk782;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x784 */ s16 unk784;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x786 */ s16 unk786;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x788 */ char pad788[0xC];                   /* maybe part of unk786[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x794 */ f32 unk794;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x798 */ f32 unk798;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x79C */ f32 unk79C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A0 */ s16 unk7A0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A2 */ s16 unk7A2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A4 */ s16 unk7A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A6 */ s16 unk7A6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A8 */ s16 unk7A8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AA */ s16 unk7AA;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AC */ s16 unk7AC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AE */ s16 unk7AE;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B0 */ s16 unk7B0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B2 */ s16 unk7B2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B4 */ s16 unk7B4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B6 */ s16 unk7B6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B8 */ s16 unk7B8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BA */ s16 unk7BA;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BC */ u8 unk7BC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BD */ u8 mAnmAtr;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BE */ u8 unk7BE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BF */ s8 unk7BF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C0 */ u8 unk7C0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C1 */ u8 mOrderType;                          /* inferred */  /* [NAMED-BY-RULE: eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-yw1.md)] */
    /* 0x7C2 */ u8 mSttNum;                          /* inferred */  /* [NAMED-BY-RULE: setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-yw1.md)] */
    /* 0x7C3 */ u8 unk7C3;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C4 */ u8 unk7C4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C5 */ u8 unk7C5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C6 */ u8 unk7C6;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C7 */ u8 unk7C7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C8 */ u8 mSttTimer;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_Yw1_c;                                      /* size >= 0x7C9 */

typedef struct daNpc_Yw1_c::anm_prm_c {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ char pad1[3];                        /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Yw1_c::anm_prm_c;                           /* size >= 0x10 */

typedef struct daNpc_Yw1_childHIO_c {
    /* 0x0 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
} daNpc_Yw1_childHIO_c;                             /* size >= 0x4 */

typedef struct fopAc_ac_c {
    /* 0x000 */ char pad0[0x1C4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ char pad1C8[0x30];                  /* maybe part of unk1C4[0xD]? */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1F8 */ Vec unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1F8 */ char pad1F8[4];  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[0xA];                   /* maybe part of unk200[3]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x20E */ s16 unk20E;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x2 (receipt f_op_actor.h) */
    /* 0x210 */ char pad210[0x3C];                  /* maybe part of unk20E[0x1F]? */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x24C */ J3DModel *unk24C;                   /* inferred */  /* = fopAc_ac_c::J3DModel* model @0x24C (receipt f_op_actor.h) */
    /* 0x250 */ char pad250[0x14];                  /* maybe part of unk24C[6]? */  /* = fopAc_ac_c::JntHit_c* jntHit @0x250 (receipt f_op_actor.h) */
    /* 0x264 */ f32 unk264;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x4 (receipt f_op_actor.h) */
    /* 0x268 */ char pad268[0x618];                 /* maybe part of unk264[0x187]? */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x8 (receipt f_op_actor.h) */
    /* 0x880 */ u8 unk880;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} fopAc_ac_c;                                       /* size >= 0x881 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[0x10];                    /* maybe part of unk50[5]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x64 */ f32 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoExt_btpAnm {
    /* 0x0 */ struct __vt__14mDoExt_baseAnm *vtable0; /* inferred */
} mDoExt_btpAnm;                                    /* size >= 0x4 */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x10 */ char pad10[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x1C */ f32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ char pad20[0xC];                     /* maybe part of unk1C[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x2C */ f32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x30 */

s32 ActionTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
? CalcAabBox__12cCcD_CylAttrFv(cCcD_CylAttr *this); /* extern */
s32 ChkPresentEnd__16dEvent_manager_cFv(dEvent_manager_c *this); /* extern */
? ClrAtHit__12dCcD_GObjInfFv(dCcD_GObjInf *this);   /* extern */
? ClrCoHit__12dCcD_GObjInfFv(dCcD_GObjInf *this);   /* extern */
? ClrTgHit__12dCcD_GObjInfFv(dCcD_GObjInf *this);   /* extern */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_CpsAttrP4cXyz(cCcD_CylAttr *this, cCcD_CpsAttr *arg0, cXyz *arg1); /* extern */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_CylAttrP4cXyz(cCcD_CylAttr *this, cCcD_CylAttr *arg0, cXyz *arg1); /* extern */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_SphAttrP4cXyz(cCcD_CylAttr *this, cCcD_SphAttr *arg0, cXyz *arg1); /* extern */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_TriAttrP4cXyz(cCcD_CylAttr *this, cCcD_TriAttr *arg0, cXyz *arg1); /* extern */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_CpsAttrPf(cCcD_CylAttr *this, cCcD_CpsAttr *arg0, f32 *arg1); /* extern */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_CylAttrPf(cCcD_CylAttr *this, cCcD_CylAttr *arg0, f32 *arg1); /* extern */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_SphAttrPf(cCcD_CylAttr *this, cCcD_SphAttr *arg0, f32 *arg1); /* extern */
? CrrPos__9dBgS_AcchFR4dBgS(dBgS_Acch *this, dBgS *arg0); /* extern */
? GetGObjInf__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
u32 GetMtrlSndId__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
? GetNVec__12cCcD_CylAttrCFRC4cXyzP4cXyz(cCcD_CylAttr *this, cXyz *arg0, cXyz *arg1); /* extern */
s8 GetPolyColor__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
s8 GetRoomId__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
s32 LockonTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
u8 LockonTruth__12dAttention_cFv(dAttention_c *this); /* extern */
? OSPanic(s8 *, ?, s8 *);                           /* extern */
? PSMTXCopy(mDoMtx_stack_c *, mDoMtx_stack_c *);    /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, f32 *, Vec *);     /* extern */
? PSMTXMultVecSR(s32, ? *, f32 *);                  /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
? PSVECAdd(f32 *, cXyz *, f32 *);                   /* extern */
f32 PSVECSquareDistance(f32 *, f32 *, f32);         /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
? __construct_array(? *, void *(*)(daNpc_Yw1_childHIO_c *), void *(*)(daNpc_Yw1_childHIO_c *, s16), ?, ?); /* extern */
void *__ct__10dCcD_GSttsFv(dCcD_GStts *this);       /* extern */
void *__ct__10fopAc_ac_cFv(fopAc_ac_c *this);       /* extern */
void *__ct__12dBgS_AcchCirFv(dBgS_AcchCir *this);   /* extern */
void *__ct__12dCcD_GObjInfFv(dCcD_GObjInf *this);   /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
void *__ct__9dBgS_AcchFv(dBgS_Acch *this);          /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
? __ml__4cXyzCFf(cXyz *this, f32 arg0);             /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
s32 __ptmf_cmpr(s32);                               /* extern */
? __ptmf_scall(daNpc_Yw1_c *, s32);                 /* extern */
s32 __ptmf_test(s32);                               /* extern */
? __register_global_object(void *(*)(daNpc_Yw1_HIO_c *, s16), void *(*)(cXyz *, s16), void *, f32); /* extern */
s32 abs(s16);                                       /* extern */
? addReal__21dDlst_shadowControl_cFUlP8J3DModel(dDlst_shadowControl_c *this, u32 arg0, J3DModel *arg1); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
? cLib_addCalcAngleS__FPsssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4); /* extern */
? cLib_chaseAngleS__FPsss(s16 *arg0, s16 arg1, s16 arg2); /* extern */
? cLib_chaseF__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
s16 cLib_targetAngleY__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
s16 cM_atan2s__Fff(f32 arg0, f32 arg1);             /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
u8 chkPointPass__14dNpc_PathRun_cF4cXyzb(dNpc_PathRun_c *this, cXyz arg0, u8 arg1); /* extern */
? cutEnd__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 cutProc__15dNpc_EventCut_cFv(dNpc_EventCut_c *this); /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s32 dComIfG_resLoad__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
u32 dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(u32 arg0, s8 arg1, J3DModel *arg2, cXyz *arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, cBgS_PolyInfo *arg8, dKy_tevstr_c *arg9, s16 arg10, f32 arg11, _GXTexObj *arg12); /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
? dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc(fopAc_ac_c *arg0, u8 arg1, mDoExt_McaMorf *arg2, s8 *arg3, s32 arg4, u16 *arg5, u32 arg6, s8 arg7); /* extern */
s32 dKy_daynight_check__Fv();                       /* extern */
? dKyw_get_AllWind_vec__FP4cXyzP4cXyzPf(cXyz *arg0, cXyz *arg1, f32 *arg2); /* extern */
? dNpc_playerEyePos__Ff(f32 arg0);                  /* extern */
? dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(mDoExt_McaMorf *arg0, s32 arg1, f32 arg2, f32 arg3, s32 arg4, s32 arg5, s8 *arg6); /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cfff(u8 arg0, fopAc_ac_c *arg1, f32 arg2, f32 arg3, f32 arg4); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
s32 fopAcM_SearchByID__FUiPP10fopAc_ac_c(u32 arg0, fopAc_ac_c **arg1); /* extern */
u32 fopAcM_create__FsUlP4cXyziP5csXyzP4cXyzScPFPv_i(s16 arg0, u32 arg1, cXyz *arg2, s32 arg3, csXyz *arg4, cXyz *arg5, s8 arg6, s32 (*arg7)(void *)); /* extern */
? fopAcM_delete__FP10fopAc_ac_c(fopAc_ac_c *arg0);  /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
? fopAcM_setCarryNow__FP10fopAc_ac_ci(fopAc_ac_c *arg0, s32 arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
s32 fopAc_IsActor__FPv(void *arg0);                 /* extern */
? fpcEx_Search__FPFPvPv_PvPv(void *(*arg0)(void *, void *), void *arg1); /* extern */
dDemo_actor_c *getActor__14dDemo_object_cFUc(dDemo_object_c *this, u8 arg0); /* extern */
JUTAssertion *getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(dRes_control_c *this, s8 *arg0, u16 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
u8 getIndex__10JUTNameTabCFPCc(JUTNameTab *this, s8 *arg0); /* extern */
? getIsAddvance__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 getMyActIdx__16dEvent_manager_cFiPCPCciii(dEvent_manager_c *this, s32 arg0, s8 **arg1, s32 arg2, s32 arg3, s32 arg4); /* extern */
? getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(dEvent_manager_c *this, s8 *arg0, fopAc_ac_c *arg1, s32 arg2); /* extern */
J3DAnmTexPattern *getP_BtpData__13dDemo_actor_cFPCc(dDemo_actor_c *this, s8 *arg0); /* extern */
? getPoint__14dNpc_PathRun_cFUc(dNpc_PathRun_c *this, u8 arg0); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb(dNpc_JntCtrl_c *this, s16 *arg0, cXyz *arg1, cXyz arg2, s16 arg3, s16 arg4, u8 arg5); /* extern */
void **mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_modelEntryDL__FP8J3DModel(J3DModel *arg0); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_ZXYrotM__FPA4_fsss(f32 (*arg0)[4], s16 arg1, s16 arg2, s16 arg3); /* extern */
? mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? memcpy(? *, ? *, ?);                              /* extern */
u8 nextIdx__14dNpc_PathRun_cFv(dNpc_PathRun_c *this); /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
s8 play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setCollision__12fopNpc_npc_cFff(fopNpc_npc_c *this, f32 arg0, f32 arg1); /* extern */
? setInf__14dNpc_PathRun_cFUcScUc(dNpc_PathRun_c *this, u8 arg0, s8 arg1, u8 arg2); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setMorf__14mDoExt_McaMorfFf(mDoExt_McaMorf *this, f32 arg0); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
? stopZelAnime__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
void *@20@__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
void *@248@__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void *@280@__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void *@32@__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
s32 CreateHeap__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_AabAttrP4cXyz(cCcD_CylAttr *this, cCcD_AabAttr *arg0, cXyz *arg1); /* static */
? CrossAtTg__12cCcD_CylAttrCFRC12cCcD_PntAttrP4cXyz(cCcD_CylAttr *this, cCcD_PntAttr *arg0, cXyz *arg1); /* static */
? CrossAtTg__12cCcD_CylAttrCFRC14cCcD_ShapeAttrP4cXyz(cCcD_CylAttr *this, cCcD_ShapeAttr *arg0, cXyz *arg1); /* static */
? CrossAtTg__14cCcD_ShapeAttrCFRC14cCcD_ShapeAttrP4cXyz(cCcD_ShapeAttr *this, cCcD_ShapeAttr *arg0, cXyz *arg1); /* static */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_AabAttrPf(cCcD_CylAttr *this, cCcD_AabAttr *arg0, f32 *arg1); /* static */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_PntAttrPf(cCcD_CylAttr *this, cCcD_PntAttr *arg0, f32 *arg1); /* static */
? CrossCo__12cCcD_CylAttrCFRC12cCcD_TriAttrPf(cCcD_CylAttr *this, cCcD_TriAttr *arg0, f32 *arg1); /* static */
? CrossCo__12cCcD_CylAttrCFRC14cCcD_ShapeAttrPf(cCcD_CylAttr *this, cCcD_ShapeAttr *arg0, f32 *arg1); /* static */
? CrossCo__14cCcD_ShapeAttrCFRC14cCcD_ShapeAttrPf(cCcD_ShapeAttr *this, cCcD_ShapeAttr *arg0, f32 *arg1); /* static */
? GetCoCP__12cCcD_CylAttrCFv(cCcD_CylAttr *this);   /* static */
? GetCoCP__12cCcD_CylAttrFv(cCcD_CylAttr *this);    /* static */
? GetCoCP__14cCcD_ShapeAttrCFv(cCcD_ShapeAttr *this); /* static */
? GetCoCP__14cCcD_ShapeAttrFv(cCcD_ShapeAttr *this); /* static */
? GetGObjInf__12cCcD_GObjInfCFv(cCcD_GObjInf *this); /* static */
? GetShapeAttr__8cCcD_ObjCFv(cCcD_Obj *this);       /* static */
? GetShapeAttr__8dCcD_CylFv(dCcD_Cyl *this);        /* static */
void *__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
void *__dt__13mDoExt_btpAnmFv(mDoExt_btpAnm *this, s16 destroyFlag); /* static */
void *__dt__14cCcD_ShapeAttrFv(cCcD_ShapeAttr *this, s16 destroyFlag); /* static */
void *__dt__14mDoExt_baseAnmFv(mDoExt_baseAnm *this, s16 destroyFlag); /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__15daNpc_Yw1_HIO_cFv(daNpc_Yw1_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__20daNpc_Yw1_childHIO_cFv(daNpc_Yw1_childHIO_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void *__dt__8cM3dGAabFv(cM3dGAab *this, s16 destroyFlag); /* static */
void *__dt__8cM3dGCylFv(cM3dGCyl *this, s16 destroyFlag); /* static */
void *__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void _nodeCB_BackBone__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void _nodeCB_Hair__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void _nodeCB_Head__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void anmAtr__11daNpc_Yw1_cFUs(daNpc_Yw1_c *this, u16 arg0); /* static */
? anmAtr__12fopNpc_npc_cFUs(fopNpc_npc_c *this, u16 arg0); /* static */
s16 cLib_calcTimer<s>__FPs(s16 *arg0);              /* static */
s16 cLib_getRndValue<i>__Fii(s32 arg0, s32 arg1);   /* static */
void chngTsuboAnm__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
void daNpc_Yw1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
void daNpc_Yw1_Delete__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0); /* static */
void daNpc_Yw1_Draw__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0); /* static */
void daNpc_Yw1_Execute__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0); /* static */
s32 daNpc_Yw1_IsDelete__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0); /* static */
s32 getMsg__11daNpc_Yw1_cFv(daNpc_Yw1_c *this);     /* static */
? getMsg__12fopNpc_npc_cFv(fopNpc_npc_c *this);     /* static */
s32 next_msgStatus__11daNpc_Yw1_cFPUl(daNpc_Yw1_c *this, u32 *arg0); /* static */
? next_msgStatus__12fopNpc_npc_cFPUl(fopNpc_npc_c *this, u32 *arg0); /* static */
void play_animation__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
void play_texPttrnAnm__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
fopAc_ac_c *searchByID__11daNpc_Yw1_cFUiPi(daNpc_Yw1_c *this, u32 arg0, s32 *arg1); /* static */
void setAnm_ATR__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
void setAttention__11daNpc_Yw1_cFb(daNpc_Yw1_c *this, u8 arg0); /* static */
void setHairAngle__11daNpc_Yw1_cFv(daNpc_Yw1_c *this); /* static */
void setMtx__11daNpc_Yw1_cFb(daNpc_Yw1_c *this, u8 arg0); /* static */
s32 set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(daNpc_Yw1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0); /* static */
void set_pthPoint__11daNpc_Yw1_cFUc(daNpc_Yw1_c *this, u8 arg0); /* static */
s32 wait_action1__11daNpc_Yw1_cFPv(daNpc_Yw1_c *this, void *arg0); /* static */
s32 wait_action2__11daNpc_Yw1_cFPv(daNpc_Yw1_c *this, void *arg0); /* static */
extern dCcD_SrcCyl dNpc_cyl_src;
extern ? g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? j3dSys;
extern s32 jmaCosTable;
extern s32 jmaSinShift;
extern s32 jmaSinTable;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
static ? a_prm_tbl$4183;                            /* unable to generate initializer: unknown type */
static ? @4343;                                     /* unable to generate initializer: unknown type */
static ? @4362;                                     /* unable to generate initializer: unknown type */
static ? @4382;                                     /* unable to generate initializer: unknown type */
static ? @4402;                                     /* unable to generate initializer: unknown type */
static struct _struct_a_anm_prm_tbl$4595_0x10 a_anm_prm_tbl$4595[7]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4602_0x10 a_anm_prm_tbl$4602[7]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4640_0x10 a_anm_prm_tbl$4640[7]; /* unable to generate initializer: non-zero padding */
static s8 *a_cut_tbl$4940 = "DUMMY\0a_mdl_dat != 0\0head\0m_hed_jnt_num >= 0\0backbone\0m_bbone_jnt_num >= 0\0hair1\0m_hair1 >= 0\0hair2\0m_hair2 >= 0\0hair3\0m_hair3 >= 0";
static u32 a_siz_tbl$5772[1] = { 0 };
static s32 a_hed_bdl_resID_tbl$6058[1] = { 7 };
static ? a_tex_pttrn_num_tbl$6061;                  /* unable to generate initializer: unknown type */
static ? @3569;
static struct _struct_l_HIO_0x38 l_HIO[1];
static void *l_check_inf[0x14];
static s32 l_check_wrk;
static ? l_hed_front;
static s32 a_res_id_tbl$4544[7] = { 0, 3, 4, 2, 1, 5, 9 }; /* const */
static s32 a_res_id_tbl$4549[1] = { 8 };            /* const */

/* daNpc_Yw1_childHIO_c::daNpc_Yw1_childHIO_c (void) */
void __ct__20daNpc_Yw1_childHIO_cFv(daNpc_Yw1_childHIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__20daNpc_Yw1_childHIO_c;
}

/* daNpc_Yw1_HIO_c::daNpc_Yw1_HIO_c (void) */
daNpc_Yw1_HIO_c *__ct__15daNpc_Yw1_HIO_cFv(daNpc_Yw1_HIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__15daNpc_Yw1_HIO_c;
    __construct_array(&this->unkC, __ct__20daNpc_Yw1_childHIO_cFv, __dt__20daNpc_Yw1_childHIO_cFv, 0x38, 1);
    this->unk40 = 0;
    memcpy(&this->unk10, &a_prm_tbl$4183, 0x30);
    this->unk4 = -1;
    this->unk8 = -1;
    return this;
}

/* nodeCB_Hair (J3DNode *, int) */
s32 nodeCB_Hair__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Yw1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_Hair__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Yw1_c::_nodeCB_Hair (J3DNode *, J3DModel *) */
void _nodeCB_Hair__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r31;
    u16 temp_r28;
    void *(*temp_r3)(daNpc_Yw1_HIO_c *, s16);

    if ((s8) @3569.unkFC == 0) {
        @3569.unk100 = 18.0f;
        temp_r3 = &@3569 + 0x100;
        temp_r3->unk4 = (f32) @4246.unk4;
        temp_r3->unk8 = (f32) @4246.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0xF0);
        @3569.unkFC = 1U;
    }
    temp_r28 = arg0->unk18;
    temp_r31 = temp_r28 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r31, &now__14mDoMtx_stack_c);
    if ((s8) this->unk6CE == (s32) temp_r28) {
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk786);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) (this->unk784 + (this->unk780 + this->unk7B6)));
    } else if ((s8) this->unk6CF == (s32) temp_r28) {
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk7A6);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) (this->unk7A4 + this->unk7B8));
    } else if ((s8) this->unk6D0 == (s32) temp_r28) {
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk7AE);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) (this->unk7AC + this->unk7BA));
    }
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r31);
}

/* nodeCB_Head (J3DNode *, int) */
s32 nodeCB_Head__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Yw1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_Head__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Yw1_c::_nodeCB_Head (J3DNode *, J3DModel *) */
void _nodeCB_Head__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r30;
    void *(*temp_r3)(daNpc_Yw1_HIO_c *, s16);

    if ((s8) @3569.unk118 == 0) {
        @3569.unk11C = 18.0f;
        temp_r3 = &@3569 + 0x11C;
        temp_r3->unk4 = (f32) @4246.unk4;
        temp_r3->unk8 = (f32) @4246.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x10C);
        @3569.unk118 = 1U;
    }
    temp_r30 = arg0->unk18 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r30, &now__14mDoMtx_stack_c);
    this->unk744 = now__14mDoMtx_stack_c.unk0[3];
    this->unk748 = now__14mDoMtx_stack_c.unk1C;
    this->unk74C = now__14mDoMtx_stack_c.unk2C;
    mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk292);
    mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk290);
    PSMTXMultVec(&now__14mDoMtx_stack_c, &@3569 + 0x11C, &this->unk72C);
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r30);
}

/* nodeCB_BackBone (J3DNode *, int) */
s32 nodeCB_BackBone__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Yw1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_BackBone__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Yw1_c::_nodeCB_BackBone (J3DNode *, J3DModel *) */
void _nodeCB_BackBone__11daNpc_Yw1_cFP7J3DNodeP8J3DModel(daNpc_Yw1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r31;

    temp_r31 = arg0->unk18 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r31, &now__14mDoMtx_stack_c);
    mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk296);
    mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk294);
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r31);
}

/* CheckCreateHeap (fopAc_ac_c *) */
void CheckCreateHeap__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    CreateHeap__11daNpc_Yw1_cFv((daNpc_Yw1_c *) arg0);
}

/* searchActor_Bm1 (void *, void *) */
void *searchActor_Bm1__FPvPv(void *arg0, void *arg1) {
    if (((s32) l_check_wrk < 0x14) && (fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0x148)) {
        l_check_inf[l_check_wrk] = arg0;
        l_check_wrk += 1;
    }
    return NULL;
}

/* daNpc_Yw1_c::init_YW1_0 (void) */
u8 init_YW1_0__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) == 0) && (isEventBit__11dSv_event_cFUs(temp_r31, 1U) == 0)) {
        if ((u32) this->unk70C == 0U) {
            return 0U;
        }
        this->unk773 = 1;
        this->unk704 = fopAcM_create__FsUlP4cXyziP5csXyzP4cXyzScPFPv_i(0x1CB, 0x7F063FU, &this->unk1F8, (s32) (s8) this->unk20A, NULL, NULL, -1, NULL);
        sp8 = @4343.unk0;
        spC = @4343.unk4;
        sp10 = @4343.unk8;
        set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(this, &sp8, 0);
        set_pthPoint__11daNpc_Yw1_cFUc(this, 0U);
        return this->unk704 != -1U;
    }
    return 0U;
}

/* daNpc_Yw1_c::init_YW1_1 (void) */
u8 init_YW1_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) == 0) {
        if (isEventBit__11dSv_event_cFUs(temp_r31, 1U) == 0) {
            this->unk1C4 &= 0xFFFFFFC0;
        }
        sp8 = @4362.unk0;
        spC = @4362.unk4;
        sp10 = @4362.unk8;
        set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk777 = 1;
        return 1U;
    }
    return 0U;
}

/* daNpc_Yw1_c::init_YW1_2 (void) */
u8 init_YW1_2__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) != 0) && ((dKy_daynight_check__Fv() == 1) || (isEventBit__11dSv_event_cFUs(temp_r31, 0x2A20U) == 0))) {
        sp8 = @4382.unk0;
        spC = @4382.unk4;
        sp10 = @4382.unk8;
        set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Yw1_c::init_YW1_3 (void) */
u8 init_YW1_3__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) != 0) && (dKy_daynight_check__Fv() == 0)) {
        if ((u32) this->unk70C == 0U) {
            return 0U;
        }
        if (isEventBit__11dSv_event_cFUs(temp_r31, 0x2A20U) != 0) {
            this->unk773 = 1;
            this->unk704 = fopAcM_create__FsUlP4cXyziP5csXyzP4cXyzScPFPv_i(0x1CB, 0x7F063FU, &this->unk1F8, (s32) (s8) this->unk20A, NULL, NULL, -1, NULL);
            sp8 = @4402.unk0;
            spC = @4402.unk4;
            sp10 = @4402.unk8;
            set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(this, &sp8, 0);
            set_pthPoint__11daNpc_Yw1_cFUc(this, 0U);
            return this->unk704 != -1U;
        }
        goto block_6;
    }
block_6:
    return 0U;
}

/* daNpc_Yw1_c::createInit (void) */
u8 createInit__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    dBgS *temp_r31;
    s32 var_r31;
    u8 temp_r0;
    u8 temp_r4;
    u8 var_r3;

    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->unk2C4, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt", (fopNpc_npc_c *) this);
    this->unk280 = 0xA;
    this->unk26D = 0xAB;
    this->unk26F = 0xAB;
    var_r31 = 0xFF;
    temp_r4 = (u8) ((u32) this->unkB0 >> 0x10U);
    if (temp_r4 != 0xFF) {
        setInf__14dNpc_PathRun_cFUcScUc((dNpc_PathRun_c *) &this->unk70C, temp_r4, (s8) this->unk20A, 1U);
        if ((u32) this->unk70C != 0U) {
            this->unk1C4 &= 0xFFFFFF7F;
            var_r31 = 0xF0;
            goto block_4;
        }
        return 0U;
    }
block_4:
    this->unk7C0 = 7;
    temp_r0 = this->unk7C6;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r3 = init_YW1_0__11daNpc_Yw1_cFv(this);
        break;
    case 1:
        var_r3 = init_YW1_1__11daNpc_Yw1_cFv(this);
        break;
    case 2:
        var_r3 = init_YW1_2__11daNpc_Yw1_cFv(this);
        break;
    case 3:
        var_r3 = init_YW1_3__11daNpc_Yw1_cFv(this);
        break;
    default:
        var_r3 = 0;
        break;
    }
    if (var_r3 == 0) {
        return 0U;
    }
    this->unk726 = this->unk204;
    this->unk728 = this->unk206;
    this->unk72A = this->unk208;
    this->unk20C = this->unk726;
    this->unk20E = this->unk728;
    this->unk210 = this->unk72A;
    this->unk258 = -4.5f;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk538, var_r31, 0xFF, (fopAc_ac_c *) this);
    this->unk5B8 = &this->unk538;
    Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk574, &dNpc_cyl_src);
    play_animation__11daNpc_Yw1_cFv(this);
    temp_r31 = &g_dComIfG_gameInfo + 0x12A0;
    CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, temp_r31);
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r31, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r31, &this->unk41C);
    setMorf__14mDoExt_McaMorfFf(this->unk330, 0.0f);
    setMtx__11daNpc_Yw1_cFb(this, 1U);
    return 1U;
}

/* daNpc_Yw1_c::play_animation (void) */
void play_animation__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u32 var_r31;

    var_r31 = 0;
    play_texPttrnAnm__11daNpc_Yw1_cFv(this);
    if (this->unk35C & 0x20) {
        var_r31 = GetMtrlSndId__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo + 0x12A0, &this->unk41C);
    }
    this->unk770 = play__14mDoExt_McaMorfFP3VecUlSc(this->unk330, &this->unk260, var_r31, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A));
    if (this->unk330->unk68 < this->unk750) {
        this->unk770 = 1;
    }
    this->unk750 = this->unk330->unk68;
}

/* daNpc_Yw1_c::upLift (void) */
u8 upLift__11daNpc_Yw1_cFv(daNpc_Yw1_c *this, ? arg_sp0) {
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    s32 sp8;
    fopAc_ac_c *temp_r31;
    fopAc_ac_c *temp_r3;
    s32 temp_cr0_eq;

    temp_r3 = searchByID__11daNpc_Yw1_cFUiPi(this, this->unk704, &sp8);
    temp_r31 = temp_r3;
    temp_cr0_eq = temp_r31 == NULL;
    this->unk775 = sp8 == 1;
    if (temp_cr0_eq == 0) {
        if (!(temp_r31->unk1C4 & 0x2000)) {
            fopAcM_setCarryNow__FP10fopAc_ac_ci(temp_r3, 0);
        }
        spC = @4246.unk10;
        sp10 = @4246.unk14;
        sp14 = @4246.unk8;
        PSMTXMultVecSR(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), &l_hed_front, &sp18);
        temp_r31->unk20E = cM_atan2s__Fff(sp18, sp20);
        PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), &now__14mDoMtx_stack_c);
        PSMTXMultVec(&now__14mDoMtx_stack_c, &spC, &temp_r31->unk1F8);
    }
    return this->unk775;
}

/* daNpc_Yw1_c::setMtx (bool) */
void setMtx__11daNpc_Yw1_cFb(daNpc_Yw1_c *this, u8 arg0) {
    J3DModel *temp_r3;
    void **temp_r3_2;

    temp_r3 = this->unk330->unk50;
    temp_r3->unk18 = this->unk214;
    temp_r3->unk1C = this->unk218;
    temp_r3->unk20 = this->unk21C;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8.unk0, this->unk1FC, this->unk200);
    mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk726, this->unk728, this->unk72A);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->unk330->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->unk330);
    setHairAngle__11daNpc_Yw1_cFv(this);
    PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), (mDoMtx_stack_c *) (this->unk6D8 + 0x24));
    temp_r3_2 = this->unk6D8;
    (*temp_r3_2)->unk10(temp_r3_2);
    upLift__11daNpc_Yw1_cFv(this);
    setAttention__11daNpc_Yw1_cFb(this, arg0);
}

/* daNpc_Yw1_c::bckResID (int) */
s32 bckResID__11daNpc_Yw1_cFi(daNpc_Yw1_c *this, s32 arg0) {
    return a_res_id_tbl$4544[arg0];
}

/* daNpc_Yw1_c::btpResID (int) */
s32 btpResID__11daNpc_Yw1_cFi(daNpc_Yw1_c *this, s32 arg0) {
    return a_res_id_tbl$4549[arg0];
}

/* daNpc_Yw1_c::init_texPttrnAnm (char signed, bool) */
u8 init_texPttrnAnm__11daNpc_Yw1_cFScb(daNpc_Yw1_c *this, s8 arg0, s32 arg1, ? arg_sp0) {
    s32 sp8;
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;
    void **temp_r29;

    temp_r29 = this->unk6D8;
    if (arg0 < 0) {
        return 0U;
    }
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4], (s8 *) btpResID__11daNpc_Yw1_cFi(this, (s32) arg0), (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) &temp_r4[7], (s8 *)0x28A, (s32) &temp_r4[0x17], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_2[7], 0x28A, &temp_r3_2[0x22]);
    }
    this->unk7BF = arg0;
    this->unk6F0 = 0;
    this->unk6F2 = 0;
    sp8 = 0;
    return init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6DC, temp_r29->unk4, (J3DAnmTexPattern *) temp_r3, 1, 0, 1.0f, 0, -1, arg1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0;
}

/* daNpc_Yw1_c::play_texPttrnAnm (void) */
void play_texPttrnAnm__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s16 temp_r3;

    if ((((s8) (u8) this->unk7BF != 0) || (cLib_calcTimer<s>__FPs(&this->unk6F2) == 0)) && (this->unk6F0 += 1, temp_r3 = this->unk6E4->unk6, (((s32) this->unk6F0 < temp_r3) == 0))) {
        if ((s8) (u8) this->unk7BF != 0) {
            this->unk6F0 = (u8) temp_r3;
            return;
        }
        this->unk6F2 = cLib_getRndValue<i>__Fii(0x3C, 0x5A);
        this->unk6F0 = 0;
    }
}

/* daNpc_Yw1_c::setAnm_anm (daNpc_Yw1_c::anm_prm_c *) */
void setAnm_anm__11daNpc_Yw1_cFPQ211daNpc_Yw1_c9anm_prm_c(daNpc_Yw1_c *this, daNpc_Yw1_c::anm_prm_c *arg0) {
    u8 temp_r4;

    temp_r4 = arg0->unk0;
    if ((s8) temp_r4 >= 0) {
        if ((s8) this->unk7C0 == (s8) temp_r4) {
            return;
        }
        dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->unk330, arg0->unkC, arg0->unk4, arg0->unk8, bckResID__11daNpc_Yw1_cFi(this, (s32) (s8) temp_r4), -1, &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4]);
        this->unk7C0 = arg0->unk0;
        this->unk770 = 0;
        this->unk771 = 0;
        this->unk750 = 0.0f;
    }
}

/* daNpc_Yw1_c::setAnm_NUM (int, int) */
void setAnm_NUM__11daNpc_Yw1_cFii(daNpc_Yw1_c *this, s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        init_texPttrnAnm__11daNpc_Yw1_cFScb(this, (s8) a_anm_prm_tbl$4595[arg0].unk1, 1);
    }
    setAnm_anm__11daNpc_Yw1_cFPQ211daNpc_Yw1_c9anm_prm_c(this, (daNpc_Yw1_c::anm_prm_c *) &a_anm_prm_tbl$4595[arg0]);
}

/* daNpc_Yw1_c::setAnm (void) */
void setAnm__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    init_texPttrnAnm__11daNpc_Yw1_cFScb(this, (s8) a_anm_prm_tbl$4602[(s8) this->mSttNum].unk1, 1);
    setAnm_anm__11daNpc_Yw1_cFPQ211daNpc_Yw1_c9anm_prm_c(this, (daNpc_Yw1_c::anm_prm_c *) &a_anm_prm_tbl$4602[(s8) this->mSttNum]);
}

/* daNpc_Yw1_c::chngAnmTag (void) */
void chngAnmTag__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {

}

/* daNpc_Yw1_c::ctrlAnmTag (void) */
void ctrlAnmTag__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {

}

/* daNpc_Yw1_c::chngAnmAtr (char unsigned) */
void chngAnmAtr__11daNpc_Yw1_cFUc(daNpc_Yw1_c *this, u8 arg0) {
    if (arg0 != (u8) this->mAnmAtr) {
        if (arg0 > 7U) {
            return;
        }
        this->mAnmAtr = arg0;
        setAnm_ATR__11daNpc_Yw1_cFv(this);
    }
}

/* daNpc_Yw1_c::ctrlAnmAtr (void) */
void ctrlAnmAtr__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {

}

/* daNpc_Yw1_c::setAnm_ATR (void) */
void setAnm_ATR__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    init_texPttrnAnm__11daNpc_Yw1_cFScb(this, (s8) a_anm_prm_tbl$4640[this->mAnmAtr].unk1, 1);
    setAnm_anm__11daNpc_Yw1_cFPQ211daNpc_Yw1_c9anm_prm_c(this, (daNpc_Yw1_c::anm_prm_c *) &a_anm_prm_tbl$4640[this->mAnmAtr]);
}

/* daNpc_Yw1_c::anmAtr (short unsigned) */
void anmAtr__11daNpc_Yw1_cFUs(daNpc_Yw1_c *this, u16 arg0) {
    u8 temp_r4;

    switch ((s32) arg0) {                           /* irregular */
    case 6:
        if ((s8) this->mSttTimer == 0) {
            chngAnmAtr__11daNpc_Yw1_cFUc(this, g_dComIfG_gameInfo.unk5BDB);
            this->mSttTimer += 1;
        }
        temp_r4 = g_dComIfG_gameInfo.unk5BDC;
        if ((temp_r4 != 0xFF) && (temp_r4 != (u8) this->unk7BE)) {
            g_dComIfG_gameInfo.unk5BDC = 0xFFU;
            this->unk7BE = temp_r4;
            chngAnmTag__11daNpc_Yw1_cFv(this);
        }
        break;
    case 14:
        this->mSttTimer = 0;
        break;
    }
    ctrlAnmAtr__11daNpc_Yw1_cFv(this);
    ctrlAnmTag__11daNpc_Yw1_cFv(this);
}

/* daNpc_Yw1_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__11daNpc_Yw1_cFPUl(daNpc_Yw1_c *this, u32 *arg0) {
    s32 var_r31;

    var_r31 = 0xF;
    if ((s32) *arg0 != 0x8A3) {
        var_r31 = 0x10;
    } else if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2A20U) == 0) {
        *arg0 = 0x8A4;
    } else if (dKy_daynight_check__Fv() == 0) {
        *arg0 = 0x8A6;
    } else {
        *arg0 = 0x8A5;
    }
    return var_r31;
}

/* daNpc_Yw1_c::getMsg_YW1_0 (void) */
s32 getMsg_YW1_0__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    dSv_event_c *temp_r31;
    s32 var_r3;

    if ((u8) this->unk774 != 0) {
        return 0x8A0;
    }
    if ((u8) this->unk775 != 0) {
        return 0x89F;
    }
    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 1U) != 0) {
        return (isEventBit__11dSv_event_cFUs(temp_r31, 0x140U) != 0) + 0x89B;
    }
    if (((u32) this->unk70C != 0U) && ((u8) this->unk776 == 1)) {
        return (this->unk712 == 1) + 0x8A1;
    }
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x180U) == 0) {
        var_r3 = 0x899;
        if (isEventBit__11dSv_event_cFUs(temp_r31, 0x2A80U) != 0) {
            return 0x8AA;
        }
        /* Duplicate return node #13. Try simplifying control flow for better match */
        return var_r3;
    }
    var_r3 = 0x89A;
    return var_r3;
}

/* daNpc_Yw1_c::getMsg_YW1_1 (void) */
s32 getMsg_YW1_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0xE20U) != 0) {
        return (isEventBit__11dSv_event_cFUs(temp_r31, 0x120U) != 0) + 0x89D;
    }
    return (isEventBit__11dSv_event_cFUs(temp_r31, 0x140U) != 0) + 0x89B;
}

/* daNpc_Yw1_c::getMsg_YW1_2 (void) */
s32 getMsg_YW1_2__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3A40U) == 0) {
        return 0x8A3;
    }
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x2A20U) == 0) {
        return 0x8A7;
    }
    return (dKy_daynight_check__Fv() == 0) + 0x8A8;
}

/* daNpc_Yw1_c::getMsg_YW1_3 (void) */
s32 getMsg_YW1_3__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    if ((u8) this->unk774 != 0) {
        return 0x8A0;
    }
    if ((u8) this->unk775 != 0) {
        return 0x89F;
    }
    if (((u32) this->unk70C != 0U) && ((u8) this->unk776 == 1)) {
        return (this->unk712 == 1) + 0x8A1;
    }
    return getMsg_YW1_2__11daNpc_Yw1_cFv(this);
}

/* daNpc_Yw1_c::getMsg (void) */
s32 getMsg__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 var_r4;
    u8 temp_r0;

    var_r4 = 0;
    temp_r0 = this->unk7C6;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r4 = getMsg_YW1_0__11daNpc_Yw1_cFv(this);
        break;
    case 1:
        var_r4 = getMsg_YW1_1__11daNpc_Yw1_cFv(this);
        break;
    case 2:
        var_r4 = getMsg_YW1_2__11daNpc_Yw1_cFv(this);
        break;
    case 3:
        var_r4 = getMsg_YW1_3__11daNpc_Yw1_cFv(this);
        break;
    }
    return var_r4;
}

/* daNpc_Yw1_c::eventOrder (void) */
void eventOrder__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 2)) {
        this->unkFA |= 1;
        if ((s32) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
        }
    }
}

/* daNpc_Yw1_c::checkOrder (void) */
void checkOrder__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 1:
        temp_r0_2 = this->mOrderType;
        if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 2)) {
            this->mOrderType = 0;
            this->unk77C = 1;
            return;
        }
    case 2:
        return;
    }
}

/* daNpc_Yw1_c::chk_talk (void) */
u8 chk_talk__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u8 var_r0;

    var_r0 = 0;
    if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
        var_r0 = 1;
    }
    if (var_r0 != 0) {
        if (ChkPresentEnd__16dEvent_manager_cFv(&g_dComIfG_gameInfo + 0x52CC) != 0) {
            this->unk772 = g_dComIfG_gameInfo.unk52B9;
            return 1U;
        }
        return 0U;
    }
    this->unk772 = 0xFF;
    return 1U;
}

/* daNpc_Yw1_c::chk_parts_notMov (void) */
u8 chk_parts_notMov__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u8 var_r5;

    var_r5 = 0;
    if (((s16) this->unk75A == (s16) this->unk292) && ((s16) this->unk75C == (s16) this->unk296) && ((s16) this->unk758 == (s16) this->unk206)) {
        var_r5 = 1;
    }
    return var_r5;
}

/* daNpc_Yw1_c::searchByID (int unsigned, int *) */
fopAc_ac_c *searchByID__11daNpc_Yw1_cFUiPi(daNpc_Yw1_c *this, u32 arg0, s32 *arg1) {
    fopAc_ac_c *sp8;

    sp8 = NULL;
    if (arg1 != NULL) {
        *arg1 = 0;
    }
    if ((fopAcM_SearchByID__FUiPP10fopAc_ac_c(arg0, &sp8) == 0) && (arg1 != NULL)) {
        *arg1 = 1;
    }
    return sp8;
}

/* daNpc_Yw1_c::partner_search_sub (void * (*) (void *, void *) *) */
u8 partner_search_sub__11daNpc_Yw1_cFPFPvPv_Pv(daNpc_Yw1_c *this, void *(*arg0)(void *, void *)) {
    s32 var_ctr;
    s32 var_r3;
    u32 var_r0;
    u8 var_r31;

    var_r31 = 0;
    this->unk700 = -1U;
    l_check_wrk = 0;
    var_r3 = 0;
    var_ctr = 0x14;
    do {
        *(l_check_inf + var_r3) = 0;
        var_r3 += 4;
        var_ctr -= 1;
    } while (var_ctr != 0);
    fpcEx_Search__FPFPvPv_PvPv(arg0, this);
    if ((s32) l_check_wrk != 0) {
        if ((void *) *l_check_inf != NULL) {
            var_r0 = (*l_check_inf)->unk4;
        } else {
            var_r0 = -1U;
        }
        this->unk700 = var_r0;
        var_r31 = 1;
    }
    return var_r31;
}

/* daNpc_Yw1_c::partner_search (void) */
void partner_search__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u8 var_r3;

    if ((s32) this->unk7C7 == 1) {
        if ((s32) this->unk7C6 != 0) {
            var_r3 = 1;
        } else {
            var_r3 = partner_search_sub__11daNpc_Yw1_cFPFPvPv_Pv(this, searchActor_Bm1__FPvPv);
        }
        if (var_r3 != 0) {
            this->unk7C7 += 1;
        }
    }
}

/* daNpc_Yw1_c::lookBack (void) */
void lookBack__11daNpc_Yw1_cFv(daNpc_Yw1_c *this, ? arg_sp0) {
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 *var_r30;
    fopAc_ac_c *temp_r3;
    s16 var_r29;
    u8 temp_r0;
    u8 temp_r28;

    this->unk75A = this->unk292;
    this->unk75C = this->unk296;
    this->unk758 = this->unk206;
    sp20 = this->unk1F8.unk0;
    sp24 = this->unk1FC;
    sp28 = this->unk200;
    sp24 = this->unk264;
    sp2C = 0.0f;
    sp30 = 0.0f;
    sp34 = 0.0f;
    var_r30 = NULL;
    var_r29 = this->unk206;
    temp_r28 = this->unk77D;
    temp_r0 = this->unk7C4;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        dNpc_playerEyePos__Ff(-20.0f);
        sp2C = sp14;
        sp30 = sp18;
        sp34 = sp1C;
        var_r30 = &sp2C;
        break;
    case 2:
        sp2C = this->unk738;
        sp30 = this->unk73C;
        sp34 = this->unk740;
        var_r30 = &sp2C;
        break;
    case 3:
        var_r29 = this->unk76E;
        break;
    case 4:
        temp_r3 = searchByID__11daNpc_Yw1_cFUiPi(this, this->unk708, NULL);
        if (temp_r3 != NULL) {
            this->unk738 = temp_r3->unk1F8.unk0;
            this->unk73C = temp_r3->unk1FC;
            this->unk740 = temp_r3->unk200;
            this->unk73C = temp_r3->unk264;
            sp2C = this->unk738;
            sp30 = this->unk73C;
            sp34 = this->unk740;
            var_r30 = &sp2C;
        }
        break;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk76C, l_HIO[(s8) this->unk7C5].unk22, 4, 0x800);
    if ((u8) this->unk29A == 0) {
        this->unk76C = 0;
    }
    sp8 = sp20;
    spC = sp24;
    sp10 = sp28;
    lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk206, (cXyz *) var_r30, (cXyz) &sp8, var_r29, this->unk76C, temp_r28);
}

/* daNpc_Yw1_c::chkAttention (void) */
u8 chkAttention__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    dAttention_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x5808;
    if (LockonTruth__12dAttention_cFv(temp_r31) != 0) {
        return this == LockonTarget__12dAttention_cFl(temp_r31, 0);
    }
    return this == ActionTarget__12dAttention_cFl(temp_r31, 0);
}

/* daNpc_Yw1_c::setAttention (bool) */
void setAttention__11daNpc_Yw1_cFb(daNpc_Yw1_c *this, u8 arg0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f0;
    f32 temp_f1;
    f32 temp_f2;

    temp_f2 = this->unk1F8.unk0;
    sp8 = temp_f2;
    temp_f1 = this->unk1FC;
    spC = temp_f1;
    temp_f0 = this->unk200;
    sp10 = temp_f0;
    this->unk274 = temp_f2;
    this->unk278 = temp_f1 + l_HIO[(s8) this->unk7C5].unk24;
    this->unk27C = temp_f0;
    if (((s32) this->unk760 != 0) || (arg0 != 0)) {
        this->unk260.unk0 = this->unk72C.unk0;
        this->unk264 = this->unk730;
        this->unk268 = this->unk734;
    }
}

/* daNpc_Yw1_c::decideType (int) */
u8 decideType__11daNpc_Yw1_cFi(daNpc_Yw1_c *this, s32 arg0) {
    u8 var_r4;

    this->unk7C5 = 0;
    this->unk7C6 = -1U;
    switch (arg0) {                                 /* irregular */
    case 0:
        this->unk7C6 = 0;
block_12:
        var_r4 = 0;
        if (((s8) this->unk7C5 != -1) && ((s8) this->unk7C6 != -1)) {
            var_r4 = 1;
        }
        return var_r4;
    case 1:
        this->unk7C6 = 1;
        goto block_12;
    case 2:
        this->unk7C6 = 2;
        goto block_12;
    case 3:
        this->unk7C6 = 3;
        goto block_12;
    default:
        return 0U;
    }
}

/* daNpc_Yw1_c::privateCut (int) */
void privateCut__11daNpc_Yw1_cFi(daNpc_Yw1_c *this, s32 arg0, ? arg_sp0) {
    dEvent_manager_c *temp_r31;

    if (arg0 != -1) {
        temp_r31 = &g_dComIfG_gameInfo + 0x52CC;
        this->unk7BC = getMyActIdx__16dEvent_manager_cFiPCPCciii(temp_r31, arg0, &a_cut_tbl$4940, 1, 1, 0);
        if ((s8) this->unk7BC == -1) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
            return;
        }
        getIsAddvance__16dEvent_manager_cFi(temp_r31, arg0);
        cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
    }
}

/* daNpc_Yw1_c::endEvent (void) */
void endEvent__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
    this->mAnmAtr = 0xFF;
    this->unk7BE = 0xFF;
}

/* daNpc_Yw1_c::isEventEntry (void) */
void isEventEntry__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(&g_dComIfG_gameInfo + 0x52CC, this->unk2C4.unk0, NULL, 0);
}

/* daNpc_Yw1_c::event_proc (int) */
void event_proc__11daNpc_Yw1_cFi(daNpc_Yw1_c *this, s32 arg0) {
    if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
        privateCut__11daNpc_Yw1_cFi(this, arg0);
    }
    lookBack__11daNpc_Yw1_cFv(this);
}

s32 set_action__11daNpc_Yw1_cFM11daNpc_Yw1_cFPCvPvPv_iPv(daNpc_Yw1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0) {
    if (__ptmf_cmpr(arg0 + 0x6F4) != 0) {
        if (__ptmf_test(arg0 + 0x6F4) != 0) {
            arg0->unk7C7 = 9;
            __ptmf_scall(arg0, arg2);
        }
        arg0->unk6F4 = (s32) arg1->unk0;
        arg0->unk6F8 = arg1->unk4;
        arg0->unk6FC = arg1->unk8;
        arg0->unk7C7 = 0;
        __ptmf_scall(arg0, arg2);
    }
    return 1;
}

/* daNpc_Yw1_c::setStt (char signed) */
void setStt__11daNpc_Yw1_cFSc(daNpc_Yw1_c *this, s8 arg0) {
    u8 temp_r0;

    this->mSttNum = (u8) arg0;
    temp_r0 = this->mSttNum;
    switch ((s8) temp_r0) {                         /* irregular */
    case 4:
    case 6:
        break;
    case 2:
        this->mAnmAtr = 0xFF;
        this->unk7BE = 0xFF;
        this->mSttTimer = 0;
        this->unk7C3 = this->mSttNum;
        break;
    case 5:
        this->unk77D = 1;
        break;
    }
    setAnm__11daNpc_Yw1_cFv(this);
    chngTsuboAnm__11daNpc_Yw1_cFv(this);
}

/* daNpc_Yw1_c::chngTsuboAnm (void) */
void chngTsuboAnm__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    u8 temp_r0;
    u8 temp_r0_2;

    if ((u8) this->unk773 != 0) {
        if ((u8) this->unk775 == 0) {
            temp_r0 = this->unk7C0;
            switch ((s8) temp_r0) {                 /* switch 1; irregular */
            case 4:                                 /* switch 1 */
                setAnm_NUM__11daNpc_Yw1_cFii(this, 3, 1);
                return;
            case 5:                                 /* switch 1 */
                setAnm_NUM__11daNpc_Yw1_cFii(this, 1, 1);
                return;
            }
        } else {
            temp_r0_2 = this->unk7C0;
            switch ((s8) temp_r0_2) {               /* switch 2; irregular */
            case 2:                                 /* switch 2 */
                break;
            case 1:                                 /* switch 2 */
                setAnm_NUM__11daNpc_Yw1_cFii(this, 5, 1);
                break;
            case 3:                                 /* switch 2 */
                setAnm_NUM__11daNpc_Yw1_cFii(this, 4, 1);
                break;
            }
            this->unk773 = 0;
        }
    }
}

/* daNpc_Yw1_c::chk_areaIN (float, float, short, cXyz) */
u8 chk_areaIN__11daNpc_Yw1_cFffs4cXyz(daNpc_Yw1_c *this, f32 arg0, f32 arg1, s16 arg2, ? arg3) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 temp_f28;
    f32 var_f29;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s16 temp_r3;

    __mi__4cXyzCFRC3Vec(&sp18, g_dComIfG_gameInfo.unk5B4C + 0x1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4246.unk8;
    sp14 = sp20;
    temp_f1 = PSVECSquareMag(&spC);
    if (temp_f1 > @4246.unk8) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = @4246.unk48 * temp_f0 * (@4246.unk50 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4246.unk48 * temp_f0_2 * (@4246.unk50 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) temp_f1 * (@4246.unk48 * temp_f0_3 * (@4246.unk50 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f29 = sp8;
    } else {
        var_f29 = temp_f1;
    }
    temp_f28 = g_dComIfG_gameInfo.unk5B4C->unk1FC - arg3->unk4;
    temp_r3 = cLib_targetAngleY__FP4cXyzP4cXyz(&this->unk1F8, g_dComIfG_gameInfo.unk5B4C + 0x1F8) - this->unk206;
    if ((var_f29 < arg0) && ((f32) fabs(temp_f28) < arg1) && (abs(temp_r3) < arg2)) {
        return 1U;
    }
    return 0U;
}

/* daNpc_Yw1_c::set_pthPoint (char unsigned) */
void set_pthPoint__11daNpc_Yw1_cFUc(daNpc_Yw1_c *this, u8 arg0) {
    f32 sp28;
    f32 sp24;
    f32 sp20;
    dNpc_PathRun_c sp14;
    dNpc_PathRun_c sp8;

    if ((u32) this->unk70C != 0U) {
        this->unk711 = arg0;
        getPoint__14dNpc_PathRun_cFUc(&sp14, (u8) &this->unk70C);
        this->unk1F8.unk0 = (bitwise f32) sp14;
        this->unk1FC = sp18;
        this->unk200 = sp1C;
        if (nextIdx__14dNpc_PathRun_cFv((dNpc_PathRun_c *) &this->unk70C) != 0) {
            getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk70C);
            sp20 = (bitwise f32) sp8;
            sp24 = spC;
            sp28 = sp10;
            this->unk206 = cLib_targetAngleY__FP4cXyzP4cXyz(&this->unk1F8, (cXyz *) &sp20);
        }
    }
}

/* daNpc_Yw1_c::setHairAngle (void) */
void setHairAngle__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    f32 spA0;
    f32 sp94;
    Vec sp88;
    Vec sp7C;
    f32 sp78;
    f32 sp74;
    f32 sp70;
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    f32 sp60;
    f32 sp5C;
    f32 sp58;
    cXyz sp4C;
    cXyz sp40;
    cXyz sp34;
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
    f32 temp_f0_4;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f1_3;
    f32 temp_f28;
    f32 temp_f29;
    f32 temp_f30;
    f32 temp_f31;
    f32 var_f0;
    f32 var_f0_2;
    f32 var_f1;
    f32 var_f2;
    f32 var_f2_2;
    f64 temp_f0;
    f64 temp_f0_10;
    f64 temp_f0_11;
    f64 temp_f0_12;
    f64 temp_f0_13;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_5;
    f64 temp_f0_6;
    f64 temp_f0_7;
    f64 temp_f0_8;
    f64 temp_f0_9;
    mDoMtx_stack_c *temp_r25;
    s16 temp_r25_2;
    s16 temp_r25_3;
    s16 temp_r25_4;
    s16 temp_r26;
    s16 temp_r26_2;
    s16 temp_r27;
    s16 temp_r28;
    s16 temp_r28_2;
    s16 temp_r9;
    s16 var_r27;
    s16 var_r28;
    s16 var_r4;
    s16 var_r4_2;
    s32 temp_r0;
    void *temp_r3;

    temp_r25 = this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30);
    PSMTXMultVec(temp_r25, &@4246 + 0x64, &sp88);
    PSMTXMultVec(temp_r25, &@4246 + 0x58, &sp7C);
    __mi__4cXyzCFRC3Vec(&sp40, &sp7C);
    temp_f1 = (bitwise f32) sp40;
    sp70 = temp_f1;
    sp74 = sp44;
    sp78 = sp48;
    temp_r0 = ((s32) cM_atan2s__Fff(temp_f1, sp48) >> jmaSinShift) * 4;
    temp_f31 = *(jmaSinTable + temp_r0);
    temp_f30 = *(jmaCosTable + temp_r0);
    temp_r3 = this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30);
    sp64 = temp_r3->unkC;
    sp68 = temp_r3->unk1C;
    sp6C = temp_r3->unk2C;
    dKyw_get_AllWind_vec__FP4cXyzP4cXyzPf((cXyz *) &sp64, &sp4C, &sp18);
    temp_f29 = sp18 * sp18;
    PSMTXMultVecSR(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), &@4246 + 0x70, &spA0);
    PSMTXMultVecSR(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), &@4246 + 0x7C, &sp94);
    temp_r25_2 = this->unk780;
    temp_r26 = this->unk782;
    if (sp98 < @4246.unk8) {
        sp28 = spA0;
        sp2C = @4246.unk8;
        sp30 = spA8;
        temp_f1_2 = PSVECSquareMag(&sp28);
        if (temp_f1_2 > @4246.unk8) {
            temp_f0 = __frsqrte(temp_f1_2);
            temp_f0_2 = @4246.unk48 * temp_f0 * (@4246.unk50 - ((f64) temp_f1_2 * (temp_f0 * temp_f0)));
            temp_f0_3 = @4246.unk48 * temp_f0_2 * (@4246.unk50 - ((f64) temp_f1_2 * (temp_f0_2 * temp_f0_2)));
            temp_f0_4 = (f32) ((f64) temp_f1_2 * (@4246.unk48 * temp_f0_3 * (@4246.unk50 - ((f64) temp_f1_2 * (temp_f0_3 * temp_f0_3)))));
            sp10 = temp_f0_4;
            var_f0 = temp_f0_4;
        } else {
            var_f0 = temp_f1_2;
        }
        this->unk780 = cM_atan2s__Fff(spA4, -var_f0);
        this->unk782 = cM_atan2s__Fff(spA0, spA8) + 0x8000;
    } else {
        sp1C = spA0;
        sp20 = @4246.unk8;
        sp24 = spA8;
        temp_f1_3 = PSVECSquareMag(&sp1C);
        if (temp_f1_3 > @4246.unk8) {
            temp_f0_5 = __frsqrte(temp_f1_3);
            temp_f0_6 = @4246.unk48 * temp_f0_5 * (@4246.unk50 - ((f64) temp_f1_3 * (temp_f0_5 * temp_f0_5)));
            temp_f0_7 = @4246.unk48 * temp_f0_6 * (@4246.unk50 - ((f64) temp_f1_3 * (temp_f0_6 * temp_f0_6)));
            spC = (f32) ((f64) temp_f1_3 * (@4246.unk48 * temp_f0_7 * (@4246.unk50 - ((f64) temp_f1_3 * (temp_f0_7 * temp_f0_7)))));
            var_f2 = spC;
        } else {
            var_f2 = temp_f1_3;
        }
        this->unk780 = cM_atan2s__Fff(spA4, var_f2);
        this->unk782 = cM_atan2s__Fff(spA0, spA8);
    }
    if ((f32) fabs(spA4) > @4246.unk88) {
        this->unk782 = temp_r26;
    }
    var_r27 = (s16) ((s16) (this->unk780 - temp_r25_2) / 2);
    var_r28 = (s16) ((s16) (this->unk782 - temp_r26) / 2);
    if (var_r27 > 0x200) {
        var_r27 = 0x200;
    } else if (var_r27 < -0x200) {
        var_r27 = -0x200;
    }
    if (var_r28 > 0x800) {
        var_r28 = 0x800;
    } else if (var_r28 < -0x800) {
        var_r28 = -0x800;
    }
    sp58 = this->unk794 - sp64;
    sp5C = (this->unk798 - sp68) - @4246.unk8C;
    sp60 = this->unk79C - sp6C;
    __ml__4cXyzCFf(&sp34, temp_f29);
    PSVECAdd(&sp58, &sp34, &sp58);
    if ((f32) fabs(sp58) < @4246.unk90) {
        sp58 = @4246.unk8;
    }
    if ((f32) fabs(sp60) < @4246.unk90) {
        sp60 = @4246.unk8;
    }
    temp_r26_2 = this->unk784;
    temp_r25_3 = this->unk786;
    temp_f28 = (sp60 * temp_f30) + (sp58 * temp_f31);
    var_r4 = cM_atan2s__Fff(-temp_f28, -sp5C);
    if ((s16) this->unk784 < 0) {
        var_r4 = 0;
    } else if ((var_r4 < 0) && (var_r4 > -0x7800)) {
        var_r4 = 0;
    } else if ((var_r4 > 0x7800) || (var_r4 <= -0x7800)) {
        var_r4 = 0x7800;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk784, var_r4, 5, 0x400);
    this->unk784 = this->unk7A0 + (var_r27 + this->unk784);
    var_f2_2 = (temp_f28 * temp_f28) + (sp5C * sp5C);
    if (var_f2_2 > @4246.unk8) {
        temp_f0_8 = __frsqrte(var_f2_2);
        temp_f0_9 = @4246.unk48 * temp_f0_8 * (@4246.unk50 - ((f64) var_f2_2 * (temp_f0_8 * temp_f0_8)));
        temp_f0_10 = @4246.unk48 * temp_f0_9 * (@4246.unk50 - ((f64) var_f2_2 * (temp_f0_9 * temp_f0_9)));
        sp14 = (f32) ((f64) var_f2_2 * (@4246.unk48 * temp_f0_10 * (@4246.unk50 - ((f64) var_f2_2 * (temp_f0_10 * temp_f0_10)))));
        var_f2_2 = sp14;
    }
    var_r4_2 = cM_atan2s__Fff(-((sp58 * temp_f30) - (sp60 * temp_f31)), var_f2_2);
    if (var_r4_2 > 0x3800) {
        var_r4_2 = 0x3800;
    } else if (var_r4_2 < -0x3800) {
        var_r4_2 = -0x3800;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk786, var_r4_2, 5, 0x400);
    this->unk786 += this->unk7A2 - var_r28;
    this->unk7A0 = (s16) (@4246.unk94 * (f32) (s16) (this->unk784 - temp_r26_2));
    this->unk7A2 = (s16) (@4246.unk94 * (f32) (s16) (this->unk786 - temp_r25_3));
    this->unk7A4 -= (s16) (this->unk784 - temp_r26_2);
    this->unk7A6 -= (s16) (this->unk786 - temp_r25_3);
    temp_r28 = this->unk7A4;
    temp_r27 = this->unk7A6;
    cLib_addCalcAngleS2__FPssss(&this->unk7A4, 0, 5, 0x400);
    cLib_addCalcAngleS2__FPssss(&this->unk7A6, 0, 5, 0x400);
    this->unk7A4 = this->unk7A8 + (var_r27 + this->unk7A4);
    this->unk7A6 += this->unk7AA - var_r28;
    this->unk7A8 = (s16) (@4246.unk94 * (f32) (s16) (this->unk7A4 - temp_r28));
    this->unk7AA = (s16) (@4246.unk94 * (f32) (s16) (this->unk7A6 - temp_r27));
    this->unk7AC -= (s16) (this->unk7A4 - temp_r28);
    this->unk7AE -= (s16) (this->unk7A6 - temp_r27);
    temp_r25_4 = this->unk7AC;
    temp_r28_2 = this->unk7AE;
    cLib_addCalcAngleS2__FPssss(&this->unk7AC, 0, 5, 0x400);
    cLib_addCalcAngleS2__FPssss(&this->unk7AE, 0, 5, 0x400);
    this->unk7AC = this->unk7B0 + (var_r27 + this->unk7AC);
    this->unk7AE += this->unk7B2 - var_r28;
    this->unk7B0 = (s16) (@4246.unk94 * (f32) (s16) (this->unk7AC - temp_r25_4));
    this->unk7B2 = (s16) (@4246.unk94 * (f32) (s16) (this->unk7AE - temp_r28_2));
    var_f1 = PSVECSquareDistance(&this->unk794, &sp64, @4246.unk94);
    if (var_f1 > @4246.unk8) {
        temp_f0_11 = __frsqrte(var_f1);
        temp_f0_12 = @4246.unk48 * temp_f0_11 * (@4246.unk50 - ((f64) var_f1 * (temp_f0_11 * temp_f0_11)));
        temp_f0_13 = @4246.unk48 * temp_f0_12 * (@4246.unk50 - ((f64) var_f1 * (temp_f0_12 * temp_f0_12)));
        sp8 = (f32) ((f64) var_f1 * (@4246.unk48 * temp_f0_13 * (@4246.unk50 - ((f64) var_f1 * (temp_f0_13 * temp_f0_13)))));
        var_f1 = sp8;
    }
    var_f0_2 = (temp_f29 + (@4246.unk98 * var_f1)) * @4246.unk9C;
    if (var_f0_2 > @4246.unk38) {
        var_f0_2 = @4246.unk38;
    }
    temp_r9 = (s16) (@4246.unkA0 + (@4246.unkA4 * var_f0_2));
    this->unk7B4 += temp_r9;
    this->unk7B6 = (s16) (@4246.unkA8 * var_f0_2 * *(jmaCosTable + (((s32) (u16) this->unk7B4 >> jmaSinShift) * 4)));
    this->unk7B8 = (s16) (@4246.unkB0 * var_f0_2 * *(jmaCosTable + (((s32) (u16) (s32) ((f32) this->unk7B4 - (@4246.unkAC * (f32) temp_r9)) >> jmaSinShift) * 4)));
    this->unk7BA = (s16) (@4246.unkB8 * var_f0_2 * *(jmaCosTable + (((s32) (u16) (s32) ((f32) this->unk7B4 - (@4246.unkB4 * (f32) temp_r9)) >> jmaSinShift) * 4)));
    this->unk794 = sp64;
    this->unk798 = sp68;
    this->unk79C = sp6C;
}

/* daNpc_Yw1_c::chk_brkTsubo (void) */
u8 chk_brkTsubo__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    if ((u8) this->unk775 != 0) {
        setStt__11daNpc_Yw1_cFSc(this, 4);
        this->unk254 = 0.0f;
        this->unk7C4 = 0;
        this->unk77D = 0;
    }
    return this->unk775;
}

/* daNpc_Yw1_c::chk_bm1Odoroki (void) */
u8 chk_bm1Odoroki__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    fopAc_ac_c *temp_r3;

    temp_r3 = searchByID__11daNpc_Yw1_cFUiPi(this, this->unk700, NULL);
    if (temp_r3 != NULL) {
        return temp_r3->unk880;
    }
    return 0U;
}

/* daNpc_Yw1_c::wait_1 (void) */
s32 wait_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if (chk_brkTsubo__11daNpc_Yw1_cFv(this) != 0) {
        return 1;
    }
    if ((u8) this->unk77C != 0) {
        if (chk_talk__11daNpc_Yw1_cFv(this) != 0) {
            setStt__11daNpc_Yw1_cFSc(this, 2);
            this->unk7C4 = 1;
            this->unk77D = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    if (chk_bm1Odoroki__11daNpc_Yw1_cFv(this) != 0) {
        this->unk7C4 = 4;
        this->unk708 = this->unk700;
        return 1;
    }
    this->mOrderType = 2;
    chngTsuboAnm__11daNpc_Yw1_cFv(this);
    sp8 = this->unk1F8.unk0;
    spC = this->unk1FC;
    sp10 = this->unk200;
    if ((u8) (chk_areaIN__11daNpc_Yw1_cFffs4cXyz(this, 100.0f + l_HIO[(s8) this->unk7C5].unk2C, 100.0f, 0x4400, (cXyz) &sp8) == 0) != 0) {
        if (cLib_calcTimer<s>__FPs(&this->unk766) == 0) {
            setStt__11daNpc_Yw1_cFSc(this, 3);
            this->unk7C4 = 0;
            this->unk77D = 1;
            return 1;
        }
        goto block_12;
    }
    this->unk766 = cLib_getRndValue<i>__Fii(0xA, 0x14);
block_12:
    this->unk7C4 = 1;
    return 1;
}

/* daNpc_Yw1_c::wait_2 (void) */
s32 wait_2__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    if ((u8) this->unk77C != 0) {
        if (chk_talk__11daNpc_Yw1_cFv(this) != 0) {
            setStt__11daNpc_Yw1_cFSc(this, 2);
            this->unk7C4 = 1;
            this->unk77D = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    if (chk_bm1Odoroki__11daNpc_Yw1_cFv(this) != 0) {
        this->unk7C4 = 4;
        this->unk708 = this->unk700;
        this->unk77D = 0;
        this->unk29A = 1;
        return 1;
    }
    this->mOrderType = 2;
    this->unk77D = 1;
    cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk722, 4, 0x800, 0x80);
    if ((u8) this->unk77B != 0) {
        this->unk766 = cLib_getRndValue<i>__Fii(0xA, 0x14);
    }
    if (cLib_calcTimer<s>__FPs(&this->unk766) != 0) {
        this->unk7C4 = 1;
        return 1;
    }
    this->unk7C4 = 0;
    return 1;
}

/* daNpc_Yw1_c::wait_3 (void) */
s32 wait_3__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    if ((u8) this->unk77C != 0) {
        if (chk_talk__11daNpc_Yw1_cFv(this) != 0) {
            setStt__11daNpc_Yw1_cFSc(this, 2);
            this->unk7C4 = 1;
            this->unk77D = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk77D = 1;
    cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk722, 4, 0x800, 0x80);
    if ((u8) this->unk77B != 0) {
        this->unk766 = cLib_getRndValue<i>__Fii(0xA, 0x14);
    }
    if (cLib_calcTimer<s>__FPs(&this->unk766) != 0) {
        this->unk7C4 = 1;
        return 1;
    }
    this->unk7C4 = 0;
    return 1;
}

/* daNpc_Yw1_c::walk_1 (void) */
s32 walk_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    dNpc_PathRun_c sp8;
    f32 temp_f1;
    f32 var_f0;
    f32 var_f31;
    struct _struct_l_HIO_0x38 *temp_r6;

    if (chk_brkTsubo__11daNpc_Yw1_cFv(this) != 0) {
        return 1;
    }
    if ((u8) this->unk776 == 0) {
        sp20 = this->unk1F8.unk0;
        sp24 = this->unk1FC;
        sp28 = this->unk200;
        if (chkPointPass__14dNpc_PathRun_cF4cXyzb((dNpc_PathRun_c *) &this->unk70C, (cXyz) &sp20, (u8) (this->unk712 != 0)) != 0) {
            this->unk776 = nextIdx__14dNpc_PathRun_cFv((dNpc_PathRun_c *) &this->unk70C) == 0;
        }
    }
    if (((u8) this->unk776 == 0) && (sp14 = this->unk1F8.unk0, sp18 = this->unk1FC, sp1C = this->unk200, (((u8) (chk_areaIN__11daNpc_Yw1_cFffs4cXyz(this, l_HIO[(s8) this->unk7C5].unk2C, @4246.unkC8, 0x4000, (cXyz) &sp14) == 0) == 0) == 0)) && ((u8) this->unk77C == 0) && (chk_bm1Odoroki__11daNpc_Yw1_cFv(this) == 0)) {
        getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk70C);
        sp2C = (bitwise f32) sp8;
        sp30 = spC;
        sp34 = sp10;
        temp_r6 = &l_HIO[(s8) this->unk7C5];
        cLib_addCalcAngleS__FPsssss(&this->unk206, cLib_targetAngleY__FP4cXyzP4cXyz(&this->unk1F8, (cXyz *) &sp2C), temp_r6->unk30, temp_r6->unk32, 0);
        var_f31 = l_HIO[(s8) this->unk7C5].unk38;
    } else {
        var_f31 = @4246.unk8;
    }
    cLib_chaseF__FPfff(&this->unk254, var_f31, l_HIO[(s8) this->unk7C5].unk3C);
    temp_f1 = this->unk254 * l_HIO[(s8) this->unk7C5].unk34;
    var_f0 = @4246.unkCC;
    if (temp_f1 < var_f0) {

    } else {
        var_f0 = temp_f1;
    }
    this->unk330->unk64 = var_f0;
    if (((s32) var_f31 == 0) && ((s32) this->unk254 == 0)) {
        this->unk254 = @4246.unk8;
        if ((u8) this->unk77C != 0) {
            if (chk_talk__11daNpc_Yw1_cFv(this) != 0) {
                setStt__11daNpc_Yw1_cFSc(this, 2);
                this->unk7C4 = 1;
                this->unk77D = 0;
                this->unk29A = 1;
            }
            return 1;
        }
        if ((u8) this->unk776 != 0) {
            setStt__11daNpc_Yw1_cFSc(this, 5);
            this->unk7C4 = 0;
            return 1;
        }
        setStt__11daNpc_Yw1_cFSc(this, 1);
        this->unk7C4 = 1;
        this->unk77D = 0;
        this->unk29A = 1;
        return 1;
    }
    this->mOrderType = 2;
    this->unk7C4 = 0;
    return 1;
}

/* daNpc_Yw1_c::turn_1 (void) */
s32 turn_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    dNpc_PathRun_c sp8;
    s16 temp_r3;

    if (chk_brkTsubo__11daNpc_Yw1_cFv(this) != 0) {
        return 1;
    }
    if ((u8) this->unk77C != 0) {
        if (chk_talk__11daNpc_Yw1_cFv(this) != 0) {
            setStt__11daNpc_Yw1_cFSc(this, 2);
        }
        return 1;
    }
    this->mOrderType = 2;
    getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk70C);
    sp14 = (bitwise f32) sp8;
    sp18 = spC;
    sp1C = sp10;
    temp_r3 = cLib_targetAngleY__FP4cXyzP4cXyz(&this->unk1F8, (cXyz *) &sp14);
    cLib_chaseAngleS__FPsss(&this->unk206, temp_r3, l_HIO[(s8) this->unk7C5].unk32);
    if ((s16) this->unk206 == temp_r3) {
        setStt__11daNpc_Yw1_cFSc(this, 3);
        this->unk7C4 = 0;
        this->unk712 ^= 1;
        this->unk776 = 0;
        return 1;
    }
    return 1;
}

/* daNpc_Yw1_c::talk_1 (void) */
u8 talk_1__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 temp_r3_2;
    u16 temp_r0;
    u8 temp_r31;
    void *temp_r3;

    temp_r31 = chk_parts_notMov__11daNpc_Yw1_cFv(this);
    talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1);
    temp_r3 = this->unk6B0;
    if (temp_r3 == NULL) {
        return temp_r31;
    }
    temp_r0 = temp_r3->unkF8;
    switch ((s32) temp_r0) {                        /* switch 1; irregular */
    case 6:                                         /* switch 1 */
    case 2:                                         /* switch 1 */
        break;
    case 19:                                        /* switch 1 */
        temp_r3_2 = this->unk6A4;
        switch (temp_r3_2) {                        /* switch 2 */
        case 0x899:                                 /* switch 2 */
        case 0x8AA:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x180U);
            break;
        case 0x89B:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x140U);
            break;
        case 0x89D:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x120U);
            break;
        case 0x89F:                                 /* switch 2 */
            g_dComIfG_gameInfo.unk5B60 = (s32) (g_dComIfG_gameInfo.unk5B60 - 0xA);
            this->unk774 = 1;
            break;
        case 0x8A4:                                 /* switch 2 */
        case 0x8A5:                                 /* switch 2 */
        case 0x8A6:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3A40U);
            break;
        }
        this->unk772 = 0xFF;
        this->unk77C = 0;
        setStt__11daNpc_Yw1_cFSc(this, (s8) this->unk7C3);
        this->unk766 = cLib_getRndValue<i>__Fii(0xA, 0x14);
        endEvent__11daNpc_Yw1_cFv(this);
        break;
    }
    return temp_r31;
}

/* daNpc_Yw1_c::wait_action1 (void *) */
s32 wait_action1__11daNpc_Yw1_cFPv(daNpc_Yw1_c *this, void *arg0) {
    s32 sp8;
    fopAc_ac_c *temp_r3;
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk7C7;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Yw1_cFSc(this, 3);
        this->unk7C7 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk77B = chkAttention__11daNpc_Yw1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk760 = wait_1__11daNpc_Yw1_cFv(this);
            break;
        case 4:                                     /* switch 2 */
            this->unk760 = wait_2__11daNpc_Yw1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk760 = walk_1__11daNpc_Yw1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk760 = turn_1__11daNpc_Yw1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk760 = talk_1__11daNpc_Yw1_cFv(this);
            break;
        }
        lookBack__11daNpc_Yw1_cFv(this);
        break;
    }
    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 1U) != 0) && ((s8) this->unk7C6 == 0)) {
        temp_r3 = searchByID__11daNpc_Yw1_cFUiPi(this, this->unk704, &sp8);
        if ((sp8 == 0) && (temp_r3 != NULL)) {
            fopAcM_delete__FP10fopAc_ac_c(temp_r3);
        }
        fopAcM_delete__FP10fopAc_ac_c((fopAc_ac_c *) this);
    }
    return 1;
}

/* daNpc_Yw1_c::wait_action2 (void *) */
s32 wait_action2__11daNpc_Yw1_cFPv(daNpc_Yw1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk7C7;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Yw1_cFSc(this, 6);
        this->unk7C7 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk77B = chkAttention__11daNpc_Yw1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 6:                                     /* switch 2 */
            this->unk760 = wait_3__11daNpc_Yw1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk760 = talk_1__11daNpc_Yw1_cFv(this);
            break;
        }
        lookBack__11daNpc_Yw1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Yw1_c::demo (void) */
u8 demo__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp8;
    J3DAnmTexPattern *temp_r3_2;
    dDemo_actor_c *temp_r3;
    s16 temp_r4;

    if ((u8) this->unk1C0 == 0) {
        if ((u8) this->unk77E != 0) {
            this->unk77E = 0;
        }
    } else {
        if ((u8) this->unk77E == 0) {
            this->unk77E = 1;
            this->unk778 = 0;
            this->unk292 = 0;
            this->unk290 = 0;
            this->unk296 = 0;
            this->unk294 = 0;
        }
        temp_r3 = getActor__14dDemo_object_cFUc(g_dComIfG_gameInfo.unk5AC8 + 0x20, this->unk1C0);
        if ((J3DAnmTexPattern *) this->unk6E4 != NULL) {
            this->unk6F0 += 1;
            temp_r4 = this->unk6E4->unk6;
            if ((s32) this->unk6F0 >= temp_r4) {
                this->unk6F0 = (u8) temp_r4;
            }
        }
        temp_r3_2 = getP_BtpData__13dDemo_actor_cFPCc(temp_r3, &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4]);
        if (temp_r3_2 != NULL) {
            sp8 = 0;
            init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6DC, this->unk6D8->unk4, temp_r3_2, 1, 0, 1.0f, 0, -1, 1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
            this->unk7BF = 1;
            this->unk6F0 = 0;
        }
        dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc((fopAc_ac_c *) this, 0x6AU, this->unk330, &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4], 0, NULL, 0U, 0);
    }
    return this->unk77E;
}

/* daNpc_Yw1_c::shadowDraw (void) */
void shadowDraw__11daNpc_Yw1_cFv(daNpc_Yw1_c *this, ? arg_sp0) {
    f32 sp14;
    f32 sp10;
    f32 spC;
    s32 sp8;
    J3DModel *temp_r5;
    f32 temp_f3;
    fopAc_ac_c *temp_r31;
    u32 temp_r4;

    temp_r31 = searchByID__11daNpc_Yw1_cFUiPi(this, this->unk704, &sp8);
    temp_f3 = this->unk1FC;
    spC = this->unk1F8.unk0;
    sp10 = @4246.unkD0 + temp_f3;
    sp14 = this->unk200;
    this->unk6D4 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unk6D4, 1, this->unk330->unk50, (cXyz *) &spC, @4246.unkD4, @4246.unkD8, temp_f3, this->unk3C8, &this->unk41C, &this->unk10C, 0, @4246.unk38, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    temp_r4 = this->unk6D4;
    if (temp_r4 != 0) {
        if ((sp8 == 0) && (temp_r31 != NULL)) {
            temp_r5 = temp_r31->unk24C;
            if (temp_r5 != NULL) {
                addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, temp_r4, temp_r5);
            }
        }
        addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, this->unk6D4, (J3DModel *) this->unk6D8);
    }
}

/* daNpc_Yw1_c::_draw (void) */
s32 _draw__11daNpc_Yw1_cFv(daNpc_Yw1_c *this, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r29;
    J3DModelData *temp_r30;
    void **temp_r28;

    temp_r28 = this->unk6D8;
    temp_r30 = temp_r28->unk4;
    temp_r29 = this->unk330->unk50;
    if (((u8) this->unk777 != 0) || ((u8) this->unk779 != 0)) {
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, &this->unk1F8, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r29, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r28, &this->unk10C);
    entryDL__14mDoExt_McaMorfFv(this->unk330);
    entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->unk6DC, temp_r30, (s16) this->unk6F0);
    mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) temp_r28);
    removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r30->unk58, this->unk6E4);
    shadowDraw__11daNpc_Yw1_cFv(this);
    dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x4BU, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
    if ((u8) l_HIO[(s8) this->unk7C5].unk28 != 0) {
        sp8 = this->unk1F8.unk0;
        spC = this->unk1FC;
        sp10 = this->unk200;
        spC = this->unk264;
    }
    return 1;
}

/* daNpc_Yw1_c::_execute (void) */
s32 _execute__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 spC;
    s32 sp8;
    dBgS *temp_r30;
    s32 var_r4;
    struct _struct_l_HIO_0x38 *temp_r10;

    if ((u8) this->unk77A == 0) {
        this->unk714 = this->unk1F8.unk0;
        this->unk718 = this->unk1FC;
        this->unk71C = this->unk200;
        this->unk720 = this->unk204;
        this->unk722 = this->unk206;
        this->unk724 = this->unk208;
        this->unk77A = 1;
    }
    temp_r10 = &l_HIO[(s8) this->unk7C5];
    sp8 = (s32) temp_r10->unk16;
    spC = (s32) temp_r10->unk20;
    setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, temp_r10->unk18, temp_r10->unk1A, temp_r10->unk1C, temp_r10->unk1E, temp_r10->unk10, temp_r10->unk12, temp_r10->unk14, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    if (((u8) this->unk777 != 0) && ((u8) this->unk1C0 == 0)) {
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 1U) != 0) {
            if ((s32) this->unk7C6 == 1) {
                this->unk1C4 = (this->unk1C4 & 0xFFFFFFC0) | 0x28;
            }
            this->unk777 = 0;
            goto block_9;
        }
        return 1;
    }
block_9:
    partner_search__11daNpc_Yw1_cFv(this);
    checkOrder__11daNpc_Yw1_cFv(this);
    if (demo__11daNpc_Yw1_cFv(this) == 0) {
        var_r4 = -1;
        if (((u8) g_dComIfG_gameInfo.unk529A != 0) && ((u16) this->unkF8 != 1)) {
            var_r4 = isEventEntry__11daNpc_Yw1_cFv(this);
        }
        if (var_r4 >= 0) {
            event_proc__11daNpc_Yw1_cFi(this, var_r4);
        } else {
            __ptmf_scall(this, 0);
        }
        fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, (cXyz *) &this->unk538);
        play_animation__11daNpc_Yw1_cFv(this);
        CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo + 0x12A0);
    }
    eventOrder__11daNpc_Yw1_cFv(this);
    this->unk726 = this->unk204;
    this->unk728 = this->unk206;
    this->unk72A = this->unk208;
    if ((u8) this->unk778 == 0) {
        this->unk20C = this->unk204;
        this->unk20E = this->unk206;
        this->unk210 = this->unk208;
    }
    temp_r30 = &g_dComIfG_gameInfo + 0x12A0;
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r30, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r30, &this->unk41C);
    setMtx__11daNpc_Yw1_cFb(this, 0U);
    setCollision__12fopNpc_npc_cFff((fopNpc_npc_c *) this, 30.0f, 150.0f);
    return 1;
}

/* daNpc_Yw1_c::_delete (void) */
s32 _delete__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    mDoExt_McaMorf *temp_r3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk6C4, &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4]);
    if ((u32) this->unkF0 != 0U) {
        temp_r3 = this->unk330;
        if (temp_r3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3);
        }
    }
    return 1;
}

/* daNpc_Yw1_c::_create (void) */
s32 _create__11daNpc_Yw1_cFv(daNpc_Yw1_c *this, ? arg_sp0) {
    s32 temp_r3;
    s32 var_r3;

    if (!(this->unk1C8 & 8)) {
        if (this != NULL) {
            __ct__10fopAc_ac_cFv((fopAc_ac_c *) this);
            this->vtable6C0 = &__vt__12fopNpc_npc_c;
            this->unk29C = 0;
            this->unk29B = 0;
            this->unk2CC = 0;
            this->unk2D0 = 0;
            this->unk32C = 0;
            __ct__9dBgS_AcchFv(&this->unk334);
            this->vtable344 = &__vt__12dBgS_ObjAcch;
            this->unk348 = &__vt__12dBgS_ObjAcch.unkC;
            this->unk354 = &__vt__12dBgS_ObjAcch.unk18;
            this->unk34C = 1;
            __ct__12dBgS_AcchCirFv(&this->unk4F8);
            this->unk550 = &__vt__9cCcD_Stts;
            __ct__10dCcD_GSttsFv(&this->unk554);
            this->unk550 = &__vt__9dCcD_Stts;
            this->unk554.unk0 = &__vt__9dCcD_Stts + 0x20;
            __ct__12dCcD_GObjInfFv((dCcD_GObjInf *) &this->unk574);
            this->vtable688 = &__vt__14cCcD_ShapeAttr;
            this->vtable684 = &__vt__8cM3dGAab;
            this->vtable6A0 = &__vt__8cM3dGCyl;
            this->vtable688 = &__vt__12cCcD_CylAttr;
            this->vtable6A0 = (struct __vt__8cM3dGCyl *) (&__vt__12cCcD_CylAttr + 0x54);
            this->vtable5B0 = &__vt__8dCcD_Cyl;
            this->vtable688 = (struct __vt__14cCcD_ShapeAttr *) &__vt__8dCcD_Cyl.unk28;
            this->vtable6A0 = (struct __vt__8cM3dGCyl *) &__vt__8dCcD_Cyl.unk7C;
            this->unk6AC = -1;
            this->unk6B0 = NULL;
            this->vtable6C0 = (struct __vt__12fopNpc_npc_c *) &__vt__11daNpc_Yw1_c;
            this->unk6DC.vtable0 = &__vt__14mDoExt_baseAnm;
            this->unk6E0 = 0;
            this->unk6DC.vtable0 = (struct __vt__14mDoExt_baseAnm *) &__vt__13mDoExt_btpAnm;
            this->unk6E8 = 0;
        }
        this->unk1C8 |= 8;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk6C4, &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4]);
    if (temp_r3 != 4) {
        return temp_r3;
    }
    if (decideType__11daNpc_Yw1_cFi(this, (s32) (u8) this->unkB0) == 0) {
        return 5;
    }
    if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) CheckCreateHeap__FP10fopAc_ac_c, a_siz_tbl$5772[(s8) this->unk7C5]) == 0) {
        return 5;
    }
    this->unk22C = &this->unk330->unk50->unk24;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, @4246.unkEC, @4246.unk40, @4246.unkEC, @4246.unkD8, @4246.unkF0, @4246.unkD8);
    var_r3 = 5;
    if (createInit__11daNpc_Yw1_cFv(this) != 0) {
        var_r3 = temp_r3;
    }
    return var_r3;
}

/* daNpc_Yw1_c::bodyCreateHeap (void) */
s32 bodyCreateHeap__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JUTAssertion *temp_r3;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *temp_r3_4;
    mDoExt_McaMorf *var_r0;
    s8 *temp_r3_2;
    s8 *temp_r3_6;
    s8 *temp_r3_8;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    u8 temp_r3_5;
    u8 temp_r3_7;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4], (s8 *)6, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) &temp_r4[7], (s8 *)0x96F, (s32) (temp_r4 + 0x2D), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_2[7], 0x96F, &temp_r3_2[0x22]);
    }
    temp_r3_3 = __nw__FUl(0xB4U);
    var_r0 = temp_r3_3;
    if (var_r0 != NULL) {
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x11020022;
        var_r0 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(temp_r3_3, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r3, NULL, NULL, 0, 1.0f, -1, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    this->unk330 = var_r0;
    temp_r3_4 = this->unk330;
    if (temp_r3_4 == NULL) {
        return 0;
    }
    if ((J3DModel *) temp_r3_4->unk50 == NULL) {
        this->unk330 = NULL;
        return 0;
    }
    temp_r3_5 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt" + 0x3C);
    this->unk6CC = temp_r3_5;
    if ((s8) this->unk6CC < 0) {
        temp_r4_2 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_5), (u32) &temp_r4_2[7], (s8 *)0x97D, (s32) (temp_r4_2 + 0x41), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_6 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_6[7], 0x97D, &temp_r3_6[0x22]);
    }
    temp_r3_7 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt" + 0x54);
    this->unk6CD = temp_r3_7;
    if ((s8) this->unk6CD < 0) {
        temp_r4_3 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_7), (u32) &temp_r4_3[7], (s8 *)0x97F, (s32) (temp_r4_3 + 0x5D), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_8 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_8[7], 0x97F, &temp_r3_8[0x22]);
    }
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CC * 4) & 0x3FFFC)))->unk8 = nodeCB_Head__FP7J3DNodei;
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CD * 4) & 0x3FFFC)))->unk8 = nodeCB_BackBone__FP7J3DNodei;
    this->unk330->unk50->unk14 = this;
    return 1;
}

/* daNpc_Yw1_c::headCreateHeap (void) */
s32 headCreateHeap__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r3_4;
    s8 *temp_r3_6;
    s8 *temp_r3_8;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    s8 *temp_r4_4;
    u8 temp_r3_3;
    u8 temp_r3_5;
    u8 temp_r3_7;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) &"Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt"[4], (s8 *) (u16) a_hed_bdl_resID_tbl$6058[(s8) this->unk7C5], (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) &temp_r4[7], (s8 *)0x99C, (s32) (temp_r4 + 0x2D), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_2[7], 0x99C, &temp_r3_2[0x22]);
    }
    this->unk6D8 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x15020022U);
    if ((void **) this->unk6D8 == NULL) {
        return 0;
    }
    if (init_texPttrnAnm__11daNpc_Yw1_cFScb(this, (s8) *(&a_tex_pttrn_num_tbl$6061 + (s8) this->unk7C5), 0) == 0) {
        return 0;
    }
    temp_r3_3 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt" + 0x72);
    this->unk6CE = temp_r3_3;
    if ((s8) this->unk6CE < 0) {
        temp_r4_2 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_3), (u32) &temp_r4_2[7], (s8 *)0x9AD, (s32) (temp_r4_2 + 0x78), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_4 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_4[7], 0x9AD, &temp_r3_4[0x22]);
    }
    temp_r3_5 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt" + 0x85);
    this->unk6CF = temp_r3_5;
    if ((s8) this->unk6CF < 0) {
        temp_r4_3 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_5), (u32) &temp_r4_3[7], (s8 *)0x9AF, (s32) (temp_r4_3 + 0x8B), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_6 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_6[7], 0x9AF, &temp_r3_6[0x22]);
    }
    temp_r3_7 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt" + 0x98);
    this->unk6D0 = temp_r3_7;
    if ((s8) this->unk6D0 < 0) {
        temp_r4_4 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_7), (u32) &temp_r4_4[7], (s8 *)0x9B1, (s32) (temp_r4_4 + 0x9E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_8 = "Yw1\0Yw\0d_a_npc_yw1.cpp\0a_btp != 0\0Halt";
        OSPanic(&temp_r3_8[7], 0x9B1, &temp_r3_8[0x22]);
    }
    (*(this->unk6D8->unk4->unk2C + (((s8) this->unk6CE * 4) & 0x3FFFC)))->unk8 = nodeCB_Hair__FP7J3DNodei;
    (*(this->unk6D8->unk4->unk2C + (((s8) this->unk6CF * 4) & 0x3FFFC)))->unk8 = nodeCB_Hair__FP7J3DNodei;
    (*(this->unk6D8->unk4->unk2C + (((s8) this->unk6D0 * 4) & 0x3FFFC)))->unk8 = nodeCB_Hair__FP7J3DNodei;
    this->unk6D8->unk14 = this;
    return 1;
}

/* daNpc_Yw1_c::CreateHeap (void) */
s32 CreateHeap__11daNpc_Yw1_cFv(daNpc_Yw1_c *this) {
    s32 sp8;

    if (bodyCreateHeap__11daNpc_Yw1_cFv(this) == 0) {
        return 0;
    }
    if (headCreateHeap__11daNpc_Yw1_cFv(this) == 0) {
        this->unk330 = NULL;
        return 0;
    }
    SetWall__12dBgS_AcchCirFff(&this->unk4F8, 30.0f, 30.0f);
    sp8 = 0;
    Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&this->unk334, &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, &this->unk4F8, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    return 1;
}

/* daNpc_Yw1_Create (fopAc_ac_c *) */
void daNpc_Yw1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _create__11daNpc_Yw1_cFv((daNpc_Yw1_c *) arg0);
}

/* daNpc_Yw1_Delete (daNpc_Yw1_c *) */
void daNpc_Yw1_Delete__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0) {
    _delete__11daNpc_Yw1_cFv(arg0);
}

/* daNpc_Yw1_Execute (daNpc_Yw1_c *) */
void daNpc_Yw1_Execute__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0) {
    _execute__11daNpc_Yw1_cFv(arg0);
}

/* daNpc_Yw1_Draw (daNpc_Yw1_c *) */
void daNpc_Yw1_Draw__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0) {
    _draw__11daNpc_Yw1_cFv(arg0);
}

/* daNpc_Yw1_IsDelete (daNpc_Yw1_c *) */
s32 daNpc_Yw1_IsDelete__FP11daNpc_Yw1_c(daNpc_Yw1_c *arg0) {
    return 1;
}

/* d_a_npc_yw1_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_yw1_cpp(void) {
    void *(*temp_r3)(daNpc_Yw1_HIO_c *, s16);

    __ct__15daNpc_Yw1_HIO_cFv(&@3569 + 0x58);
    __register_global_object(__dt__15daNpc_Yw1_HIO_cFv, &@3569 + 0x4C);
    @3569.unk134 = 0.0f;
    temp_r3 = &@3569 + 0x134;
    temp_r3->unk4 = 1.0f;
    temp_r3->unk8 = 0.0f;
    __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x128, 0.0f);
}