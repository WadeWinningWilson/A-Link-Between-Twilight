// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in naming-map-auto.md;
// remaining unkNNN are still inference-needed. Behavior identical to the full draft.
// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_aj1 (95 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_aj1\asm\d\actor\d_a_npc_aj1.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DAnmTexPattern {
    /* 0x0 */ char pad0[6];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x6 */ s16 unk6;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
} J3DAnmTexPattern;                                 /* size >= 0x8 */

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ J3DModelData *unk4;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0xC];                      /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x14 */ daNpc_Aj1_c *unk14;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x18 */ f32 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1C */ f32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ f32 unk20;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x24 */ mDoMtx_stack_c unk24;                /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x24 */ char pad24[0x68];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x8C */ s32 unk8C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8C (receipt f_op_actor.h) */
} J3DModel;                                         /* size >= 0x90 */

typedef struct J3DModelData {
    /* 0x00 */ char pad0[0x2C];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x2C */ s32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ char pad30[0x28];                    /* maybe part of unk2C[0xB]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x58 */ J3DMaterialTable unk58;              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x58 */ char pad58[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
} J3DModelData;                                     /* size >= 0x59 */

typedef struct J3DNode {
    /* 0x00 */ char pad0[0x18];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x18 */ u16 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
} J3DNode;                                          /* size >= 0x1A */

typedef struct JAIZelBasic {
    /* 0x0 */ JAIZelBasic *unk0;                    /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0x4 */

typedef struct JPABaseEmitter {
    /* 0x000 */ char pad0[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x00C */ f32 unkC;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x010 */ f32 unk10;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x014 */ f32 unk14;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x018 */ char pad18[0x20];                   /* maybe part of unk14[9]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x038 */ f32 unk38;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
    /* 0x03C */ char pad3C[0x24];                   /* maybe part of unk38[0xA]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3C (receipt f_op_actor.h) */
    /* 0x060 */ s32 unk60;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x60 (receipt f_op_actor.h) */
    /* 0x064 */ s16 unk64;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x066 */ char pad66[0xE];                    /* maybe part of unk64[8]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x66 (receipt f_op_actor.h) */
    /* 0x074 */ f32 unk74;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x74 (receipt f_op_actor.h) */
    /* 0x078 */ char pad78[0x10C];                  /* maybe part of unk74[0x44]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x78 (receipt f_op_actor.h) */
    /* 0x184 */ s32 unk184;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x78 (receipt f_op_actor.h) */
    /* 0x188 */ char pad188[8];                     /* maybe part of unk184[3]? */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x7C (receipt f_op_actor.h) */
    /* 0x190 */ s32 unk190;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x84 (receipt f_op_actor.h) */
    /* 0x194 */ char pad194[0x14];                  /* maybe part of unk190[6]? */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x88 (receipt f_op_actor.h) */
    /* 0x1A8 */ f32 unk1A8[4];                      /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x9C (receipt f_op_actor.h) */
    /* 0x1B8 */ char pad1B8[0x20];                  /* maybe part of unk1A8[3]? */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0xAC (receipt f_op_actor.h) */
    /* 0x1D8 */ f32 unk1D8;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x8 (receipt f_op_actor.h) */
    /* 0x1DC */ f32 unk1DC;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0xC (receipt f_op_actor.h) */
    /* 0x1E0 */ f32 unk1E0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x10 (receipt f_op_actor.h) */
    /* 0x1E4 */ JGeometry::TVec3<float> unk1E4;     /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1E4 */ char pad1E4[0xC];  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1F0 */ f32 unk1F0;                         /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 +0xC (receipt f_op_actor.h) */
    /* 0x1F4 */ f32 unk1F4;                         /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 +0x10 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ char pad1FC[0x10];                  /* maybe part of unk1F8[5]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x20C */ s32 unk20C;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
} JPABaseEmitter;                                   /* size >= 0x210 */

typedef struct JUTAssertion {
    /* 0x00 */ char pad0[0x54];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x54 */ JUTNameTab *unk54;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
} JUTAssertion;                                     /* size >= 0x58 */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0x4 */

struct __vt__11daNpc_Aj1_c {
    /* 0x00 */ s32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ s32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ s32 (*next_msgStatus)(daNpc_Aj1_c *, u32 *);
    /* 0x0C */ s32 (*getMsg)(daNpc_Aj1_c *);
    /* 0x10 */ void (*anmAtr)(daNpc_Aj1_c *, u16);
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

struct __vt__15daNpc_Aj1_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Aj1_HIO_c *, s16);
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

struct _struct_a_anm_prm_tbl$4477_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4484_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4535_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct dBgS {
    /* 0x0000 */ char pad0[0x402C];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x402C */ dEvent_manager_c unk402C;          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x402C */ char pad402C[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} dBgS;                                             /* size >= 0x402D */

typedef struct dCcD_GStts {
    /* 0x0 */ void *unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dCcD_GStts;                                       /* size >= 0x4 */

typedef struct dNpc_EventCut_c {
    /* 0x0 */ s8 *unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_EventCut_c;                                  /* size >= 0x4 */

typedef struct dPa_levelEcallBack {
    /* 0x0 */ void *unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dPa_levelEcallBack;                               /* size >= 0x4 */

typedef struct dSv_info_c {
    /* 0x00000 */ char pad0[0x624];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x00624 */ dSv_event_c unk624;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x00624 */ char pad624[0xC7C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x012A0 */ dBgS unk12A0;                     /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x012A0 */ char pad12A0[0x3E94];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05134 */ ? unk5134;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05134 */ char pad5134[0x166];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x0529A */ u8 unk529A;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x0529B */ char pad529B[0x1D];               /* maybe part of unk529A[0x1E]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052B8 */ u8 unk52B8;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052B9 */ u8 unk52B9;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052BA */ char pad52BA[6];                  /* maybe part of unk52B9[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052C0 */ u16 unk52C0;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052C2 */ char pad52C2[0xA];                /* maybe part of unk52C0[6]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052CC */ dEvent_manager_c unk52CC;         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x052CC */ char pad52CC[0x53C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05808 */ dAttention_c unk5808;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05808 */ char pad5808[0x198];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x059A0 */ dVibration_c unk59A0;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x059A0 */ char pad59A0[0x84];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05A24 */ dDetect_c unk5A24;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05A24 */ char pad5A24[0xA0];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC4 */ dPa_control_c *unk5AC4;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC8 */ s32 unk5AC8;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05ACC */ char pad5ACC[0x78];               /* maybe part of unk5AC8[0x1F]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B44 */ void *unk5B44;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05B48 */ char pad5B48[0x93];               /* maybe part of unk5B44[0x25]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05BDB */ u8 unk5BDB;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05BDC */ u8 unk5BDC;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05BDD */ char pad5BDD[0x38F];              /* maybe part of unk5BDC[0x390]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05F6C */ dDlst_shadowControl_c unk5F6C;    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05F6C */ char pad5F6C[0x16054];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ ? unk1BFC0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ char pad1BFC0[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} dSv_info_c;                                       /* size >= 0x1BFC1 */

typedef struct daNpc_Aj1_HIO_c {
    /* 0x00 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
    /* 0x04 */ s8 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x05 */ char pad5[3];                        /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x08 */ s32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ ? unkC;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x0C */ char padC[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x0E */ s16 unkE;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xE (receipt f_op_actor.h) */
    /* 0x10 */ s16 unk10;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x12 */ s16 unk12;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x12 (receipt f_op_actor.h) */
    /* 0x14 */ s16 unk14;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x16 */ s16 unk16;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x16 (receipt f_op_actor.h) */
    /* 0x18 */ s16 unk18;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
    /* 0x1A */ s16 unk1A;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1A (receipt f_op_actor.h) */
    /* 0x1C */ s16 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x1E */ s16 unk1E;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1E (receipt f_op_actor.h) */
    /* 0x20 */ f32 unk20;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x24 */ u8 unk24;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x25 */ char pad25[3];                       /* maybe part of unk24[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x25 (receipt f_op_actor.h) */
    /* 0x28 */ f32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ s16 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x2E */ char pad2E[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2E (receipt f_op_actor.h) */
    /* 0x30 */ f32 unk30;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x34 */ s16 unk34;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x36 */ s16 unk36;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x36 (receipt f_op_actor.h) */
    /* 0x38 */ s16 unk38;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
} daNpc_Aj1_HIO_c;                                  /* size >= 0x3A */

typedef struct daNpc_Aj1_c {
    /* 0x000 */ char pad0[0xB0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x0B0 */ u32 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
    /* 0x0B4 */ char padB4[0x3C];                   /* maybe part of unkB0[0x10]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB4 (receipt f_op_actor.h) */
    /* 0x0F0 */ u32 unkF0;                          /* inferred */  /* = fopAc_ac_c::JKRSolidHeap* heap @0xF0 (receipt f_op_actor.h) */
    /* 0x0F4 */ char padF4[4];  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 (receipt f_op_actor.h) */
    /* 0x0F8 */ u16 unkF8;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x4 (receipt f_op_actor.h) */
    /* 0x0FA */ u16 unkFA;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x6 (receipt f_op_actor.h) */
    /* 0x0FC */ char padFC[8];                      /* maybe part of unkFA[5]? */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x8 (receipt f_op_actor.h) */
    /* 0x104 */ void (*unk104)(void *, s32);        /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x10 (receipt f_op_actor.h) */
    /* 0x108 */ char pad108[4];  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x14 (receipt f_op_actor.h) */
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
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
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
    /* 0x230 */ char pad230[0x28];                  /* maybe part of unk22C[0xB]? */  /* = fopAc_ac_c::fopAc_cullSizeSphere sphere @0x230 (receipt f_op_actor.h) */
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
    /* 0x6D0 */ void **unk6D0;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ u32 unk6D4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ mDoExt_btpAnm unk6D8;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ char pad6D8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ s32 unk6DC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E0 */ J3DAnmTexPattern *unk6E0;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E4 */ s32 unk6E4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E8 */ char pad6E8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6EC */ u8 unk6EC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6ED */ char pad6ED[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6EE */ s16 unk6EE;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F0 */ ? unk6F0;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F0 */ char pad6F0[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F4 */ s32 unk6F4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ s32 unk6F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6FC */ f32 unk6FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x700 */ f32 unk700;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x704 */ f32 unk704;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x708 */ s16 unk708;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70A */ s16 unk70A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70C */ s16 unk70C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70E */ s16 unk70E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x710 */ s16 unk710;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x712 */ s16 unk712;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x714 */ cXyz unk714;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x714 */ char pad714[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x718 */ f32 unk718;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x71C */ f32 unk71C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x720 */ f32 unk720;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x724 */ f32 unk724;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x728 */ f32 unk728;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72C */ f32 unk72C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x730 */ f32 unk730;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x734 */ f32 unk734;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x738 */ s16 unk738;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x73A */ s16 unk73A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x73C */ s16 unk73C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x73E */ char pad73E[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x740 */ f32 unk740;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x744 */ s16 mEventIdTable;                         /* inferred */  /* [NAMED-BY-RULE: event id table (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md)] */
    /* 0x746 */ s16 mEventIdx;                         /* inferred */  /* [NAMED-BY-RULE: event index (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md)] */
    /* 0x748 */ s16 unk748;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x74A */ s16 unk74A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x74C */ s16 unk74C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x74E */ s16 unk74E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x750 */ s16 unk750;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x752 */ char pad752[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x754 */ s16 unk754;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x756 */ s16 unk756;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x758 */ u8 unk758;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x759 */ s8 unk759;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75A */ u8 unk75A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75B */ u8 unk75B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75C */ u8 unk75C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75D */ char pad75D[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75E */ u8 unk75E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75F */ u8 unk75F;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x760 */ u8 unk760;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x761 */ u8 unk761;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x762 */ char pad762[2];                     /* maybe part of unk761[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x764 */ s32 unk764;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x768 */ u8 unk768;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x769 */ u8 unk769;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76A */ u8 unk76A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76B */ u8 unk76B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76C */ dPa_levelEcallBack unk76C;          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x76C */ char pad76C[0x16];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x782 */ u8 unk782;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x783 */ u8 unk783;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x784 */ u8 unk784;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x785 */ u8 unk785;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x786 */ char pad786[6];                     /* maybe part of unk785[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x78C */ JPABaseEmitter *unk78C;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x790 */ JPABaseEmitter *unk790;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x794 */ JPABaseEmitter *unk794;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x798 */ JPABaseEmitter *unk798;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x79C */ f32 unk79C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A0 */ f32 unk7A0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A4 */ f32 unk7A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A8 */ cXyz unk7A8;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A8 */ char pad7A8[0xC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B4 */ u8 unk7B4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B5 */ u8 unk7B5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B6 */ u8 mAnmAtr;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B7 */ u8 unk7B7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B8 */ s8 unk7B8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B9 */ u8 unk7B9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BA */ u8 mOrderType;                          /* inferred */  /* [NAMED-BY-RULE: eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md)] */
    /* 0x7BB */ u8 mSttNum;                          /* inferred */  /* [NAMED-BY-RULE: setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md)] */
    /* 0x7BC */ u8 mSttNumOld;                          /* inferred */  /* [NAMED-BY-RULE: stt old (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md)] */
    /* 0x7BD */ u8 unk7BD;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BE */ u8 unk7BE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BF */ u8 unk7BF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C0 */ u8 unk7C0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C1 */ u8 unk7C1;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_Aj1_c;                                      /* size >= 0x7C2 */

typedef struct daNpc_Aj1_c::anm_prm_c {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ char pad1[3];                        /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Aj1_c::anm_prm_c;                           /* size >= 0x10 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x58 */ J3DFrameCtrl unk58;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x58 */ char pad58[0x10];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoExt_btpAnm {
    /* 0x0 */ struct __vt__14mDoExt_baseAnm *vtable0; /* inferred */
} mDoExt_btpAnm;                                    /* size >= 0x4 */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ char pad0[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x0C */ f32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ char pad10[0xC];                     /* maybe part of unkC[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
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
? JPASetRMtxTVecfromMtx__FPA4_fPA4_fRQ29JGeometry8TVec3<f>(f32 (*arg0)[4], f32 (*arg1)[4], JGeometry::TVec3<float> *arg2); /* extern */
s32 LockonTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
u8 LockonTruth__12dAttention_cFv(dAttention_c *this); /* extern */
? OSPanic(s8 *, ?, s8 *);                           /* extern */
? PSMTXCopy(mDoMtx_stack_c *, mDoMtx_stack_c *);    /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, f32 *, cXyz *);    /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
? StartShock__12dVibration_cFii4cXyz(dVibration_c *this, s32 arg0, s32 arg1, cXyz arg2); /* extern */
void *__ct__10dCcD_GSttsFv(dCcD_GStts *this);       /* extern */
void *__ct__10fopAc_ac_cFv(fopAc_ac_c *this);       /* extern */
void *__ct__12dBgS_AcchCirFv(dBgS_AcchCir *this);   /* extern */
void *__ct__12dCcD_GObjInfFv(dCcD_GObjInf *this);   /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
void *__ct__18dPa_smokeEcallBackFUc(dPa_smokeEcallBack *this, u8 arg0); /* extern */
void *__ct__9dBgS_AcchFv(dBgS_Acch *this);          /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
s32 __ptmf_cmpr(s32);                               /* extern */
? __ptmf_scall(daNpc_Aj1_c *, s32);                 /* extern */
s32 __ptmf_test(s32);                               /* extern */
? __register_global_object(void *(*)(daNpc_Aj1_HIO_c *, s16), void *(*)(cXyz *, s16), void *); /* extern */
s32 abs(s16);                                       /* extern */
? addReal__21dDlst_shadowControl_cFUlP8J3DModel(dDlst_shadowControl_c *this, u32 arg0, J3DModel *arg1); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
? cLib_addCalcAngleS__FPsssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4); /* extern */
s16 cLib_targetAngleY__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
s32 checkPass__12J3DFrameCtrlFf(J3DFrameCtrl *this, f32 arg0); /* extern */
? cutEnd__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 cutProc__15dNpc_EventCut_cFv(dNpc_EventCut_c *this); /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s32 dComIfG_resLoad__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
u32 dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(u32 arg0, s8 arg1, J3DModel *arg2, cXyz *arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, cBgS_PolyInfo *arg8, dKy_tevstr_c *arg9, s16 arg10, f32 arg11, _GXTexObj *arg12); /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
? dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc(fopAc_ac_c *arg0, u8 arg1, mDoExt_McaMorf *arg2, s8 *arg3, s32 arg4, u16 *arg5, u32 arg6, s8 arg7); /* extern */
s32 dKy_daynight_check__Fv();                       /* extern */
? dNpc_playerEyePos__Ff(f32 arg0);                  /* extern */
? dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(mDoExt_McaMorf *arg0, s32 arg1, f32 arg2, f32 arg3, s32 arg4, s32 arg5, s8 *arg6); /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cfff(u8 arg0, fopAc_ac_c *arg1, f32 arg2, f32 arg3, f32 arg4); /* extern */
s32 endCheck__16dEvent_manager_cFs(dEvent_manager_c *this, s16 arg0); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderOtherEventId__FP10fopAc_ac_csUcUsUsUs(fopAc_ac_c *arg0, s16 arg1, u8 arg2, u16 arg3, u16 arg4, u16 arg5); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
dDemo_actor_c *getActor__14dDemo_object_cFUc(dDemo_object_c *this, u8 arg0); /* extern */
s16 getEventIdx__16dEvent_manager_cFPCcUc(dEvent_manager_c *this, s8 *arg0, u8 arg1); /* extern */
JUTAssertion *getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(dRes_control_c *this, s8 *arg0, u16 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
u8 getIndex__10JUTNameTabCFPCc(JUTNameTab *this, s8 *arg0); /* extern */
s32 getIsAddvance__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 getMyActIdx__16dEvent_manager_cFiPCPCciii(dEvent_manager_c *this, s32 arg0, s8 **arg1, s32 arg2, s32 arg3, s32 arg4); /* extern */
? getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(dEvent_manager_c *this, s8 *arg0, fopAc_ac_c *arg1, s32 arg2); /* extern */
J3DAnmTexPattern *getP_BtpData__13dDemo_actor_cFPCc(dDemo_actor_c *this, s8 *arg0); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
s32 isSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
? lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb(dNpc_JntCtrl_c *this, s16 *arg0, cXyz *arg1, cXyz arg2, s16 arg3, s16 arg4, u8 arg5); /* extern */
void **mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_modelEntryDL__FP8J3DModel(J3DModel *arg0); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_ZXYrotM__FPA4_fsss(f32 (*arg0)[4], s16 arg1, s16 arg2, s16 arg3); /* extern */
? mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? memcpy(? *, ? *, ?);                              /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? onSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
u8 play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setCollision__12fopNpc_npc_cFff(fopNpc_npc_c *this, f32 arg0, f32 arg1); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setMorf__14mDoExt_McaMorfFf(mDoExt_McaMorf *this, f32 arg0); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
JPABaseEmitter *set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(dPa_control_c *this, u8 arg0, u16 arg1, cXyz *arg2, csXyz *arg3, cXyz *arg4, u8 arg5, dPa_levelEcallBack *arg6, s8 arg7, _GXColor *arg8, _GXColor *arg9, cXyz *arg10); /* extern */
? set_quake__9dDetect_cFPC4cXyz(dDetect_c *this, cXyz *arg0); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
s32 startCheck__16dEvent_manager_cFs(dEvent_manager_c *this, s16 arg0); /* extern */
? stopZelAnime__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
s32 strcmp(? *, s8 *);                              /* extern */
u16 talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
void *@20@__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
void *@248@__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void *@280@__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void *@32@__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
s32 CreateHeap__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
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
u8 _XyCheckCB__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0); /* static */
void *__dt__12dBgS_ObjAcchFv(dBgS_ObjAcch *this, s16 destroyFlag); /* static */
void *__dt__13mDoExt_btpAnmFv(mDoExt_btpAnm *this, s16 destroyFlag); /* static */
void *__dt__14cCcD_ShapeAttrFv(cCcD_ShapeAttr *this, s16 destroyFlag); /* static */
void *__dt__14mDoExt_baseAnmFv(mDoExt_baseAnm *this, s16 destroyFlag); /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__15daNpc_Aj1_HIO_cFv(daNpc_Aj1_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void *__dt__8cM3dGAabFv(cM3dGAab *this, s16 destroyFlag); /* static */
void *__dt__8cM3dGCylFv(cM3dGCyl *this, s16 destroyFlag); /* static */
void *__dt__8dCcD_CylFv(dCcD_Cyl *this, s16 destroyFlag); /* static */
void _nodeCB_BackBone__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(daNpc_Aj1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void _nodeCB_Head__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(daNpc_Aj1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void anmAtr__11daNpc_Aj1_cFUs(daNpc_Aj1_c *this, u16 arg0); /* static */
? anmAtr__12fopNpc_npc_cFUs(fopNpc_npc_c *this, u16 arg0); /* static */
u8 cLib_calcTimer<Uc>__FPUc(u8 *arg0);              /* static */
s16 cLib_calcTimer<s>__FPs(s16 *arg0);              /* static */
s16 cLib_getRndValue<i>__Fii(s32 arg0, s32 arg1);   /* static */
u8 chk_parts_notMov__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void daNpc_Aj1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
void daNpc_Aj1_Delete__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0); /* static */
void daNpc_Aj1_Draw__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0); /* static */
void daNpc_Aj1_Execute__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0); /* static */
s32 daNpc_Aj1_IsDelete__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0); /* static */
void del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(daNpc_Aj1_c *this, JPABaseEmitter **arg0); /* static */
s32 getMsg__11daNpc_Aj1_cFv(daNpc_Aj1_c *this);     /* static */
? getMsg__12fopNpc_npc_cFv(fopNpc_npc_c *this);     /* static */
s32 next_msgStatus__11daNpc_Aj1_cFPUl(daNpc_Aj1_c *this, u32 *arg0); /* static */
? next_msgStatus__12fopNpc_npc_cFPUl(fopNpc_npc_c *this, u32 *arg0); /* static */
void play_animation__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void play_texPttrnAnm__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void setAnm_ATR__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void setAnm_NUM__11daNpc_Aj1_cFii(daNpc_Aj1_c *this, s32 arg0, s32 arg1); /* static */
void setAttention__11daNpc_Aj1_cFb(daNpc_Aj1_c *this, u8 arg0); /* static */
void setMtx__11daNpc_Aj1_cFb(daNpc_Aj1_c *this, u8 arg0); /* static */
s32 set_action__11daNpc_Aj1_cFM11daNpc_Aj1_cFPCvPvPv_iPv(daNpc_Aj1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0); /* static */
void set_pa_aka__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void set_pa_don__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
void set_pa_pun__11daNpc_Aj1_cFv(daNpc_Aj1_c *this); /* static */
s32 wait_action1__11daNpc_Aj1_cFPv(daNpc_Aj1_c *this, void *arg0); /* static */
s32 wait_action2__11daNpc_Aj1_cFPv(daNpc_Aj1_c *this, void *arg0); /* static */
void *@4146(cXyz *this, s16 destroyFlag);           /* static */
extern dCcD_SrcCyl dNpc_cyl_src;
extern dSv_info_c g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? j3dSys;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? a_prm_tbl$4141;                            /* unable to generate initializer: unknown type */
static s8 *l_evn_tbl = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
static ? @4240;                                     /* unable to generate initializer: unknown type */
static ? @4269;                                     /* unable to generate initializer: unknown type */
static ? @4284;                                     /* unable to generate initializer: unknown type */
static struct _struct_a_anm_prm_tbl$4477_0x10 a_anm_prm_tbl$4477[9]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4484_0x10 a_anm_prm_tbl$4484[5]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4535_0x10 a_anm_prm_tbl$4535[9]; /* unable to generate initializer: non-zero padding */
static s8 *a_cut_tbl$4900[8] = {
    "AJ1_TLK",
    "INI_ANGRY",
    "VIVRATE",
    "JMP",
    "SPPRISE",
    "LOK",
    "DAN",
    "INVITE\0Ojhous2\0a_mdl_dat != 0\0head\0m_hed_jnt_num >= 0\0backbone\0m_bbone_jnt_num >= 0\0handL\0m_hnd_L_jnt_num >= 0\0footL\0m_fot_L_jnt_num >= 0",
};
static u32 a_siz_tbl$5553[1] = { 0 };
static ? @3569;
static daNpc_Aj1_HIO_c l_HIO;
static s32 a_res_id_tbl$4414[9] = { 7, 8, 0, 2, 5, 3, 6, 1, 4 }; /* const */
static s32 a_res_id_tbl$4419[1] = { 0xB };          /* const */

/* daNpc_Aj1_HIO_c::daNpc_Aj1_HIO_c (void) */
daNpc_Aj1_HIO_c *__ct__15daNpc_Aj1_HIO_cFv(daNpc_Aj1_HIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__15daNpc_Aj1_HIO_c;
    memcpy(&this->unkC, &a_prm_tbl$4141, 0x30);
    this->unk4 = -1;
    this->unk8 = -1;
    return this;
}

/* nodeCB_Head (J3DNode *, int) */
s32 nodeCB_Head__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Aj1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_Head__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Aj1_c::_nodeCB_Head (J3DNode *, J3DModel *) */
void _nodeCB_Head__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(daNpc_Aj1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r30;
    void *(*temp_r3)(daNpc_Aj1_HIO_c *, s16);

    if ((s8) @3569.unkA0 == 0) {
        @3569.unkA4 = 24.0f;
        temp_r3 = &@3569 + 0xA4;
        temp_r3->unk4 = (f32) @4185.unk4;
        temp_r3->unk8 = (f32) @4185.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x94);
        @3569.unkA0 = 1U;
    }
    temp_r30 = arg0->unk18 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r30, &now__14mDoMtx_stack_c);
    this->unk72C = now__14mDoMtx_stack_c.unkC;
    this->unk730 = now__14mDoMtx_stack_c.unk1C;
    this->unk734 = now__14mDoMtx_stack_c.unk2C;
    mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk292);
    mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk290);
    PSMTXMultVec(&now__14mDoMtx_stack_c, &@3569 + 0xA4, &this->unk714);
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r30);
}

/* nodeCB_BackBone (J3DNode *, int) */
s32 nodeCB_BackBone__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Aj1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_BackBone__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Aj1_c::_nodeCB_BackBone (J3DNode *, J3DModel *) */
void _nodeCB_BackBone__11daNpc_Aj1_cFP7J3DNodeP8J3DModel(daNpc_Aj1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r31;

    temp_r31 = arg0->unk18 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r31, &now__14mDoMtx_stack_c);
    mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk296);
    mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk294);
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r31);
}

/* CheckCreateHeap (fopAc_ac_c *) */
void CheckCreateHeap__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    CreateHeap__11daNpc_Aj1_cFv((daNpc_Aj1_c *) arg0);
}

/* daNpc_Aj1_XyCheck_CB (void *, int) */
void daNpc_Aj1_XyCheck_CB__FPvi(void *arg0, s32 arg1) {
    _XyCheckCB__11daNpc_Aj1_cFi((daNpc_Aj1_c *) arg0, arg1);
}

/* daNpc_Aj1_c::_XyCheckCB (int) */
u8 _XyCheckCB__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0) {
    return (u8) ((&g_dComIfG_gameInfo + arg0)->unk5BD3 == 0x48);
}

/* daNpc_Aj1_c::init_AJ1_0 (void) */
u8 init_AJ1_0__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    u8 temp_r4;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0xE20U) == 0) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x502U) == 0)) {
        this->unk26D = 0x1A;
        sp8 = @4240.unk0;
        spC = @4240.unk4;
        sp10 = @4240.unk8;
        set_action__11daNpc_Aj1_cFM11daNpc_Aj1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    temp_r4 = this->unk75B;
    if ((temp_r4 != 0xFF) && (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) temp_r4, (s32) (s8) this->unk20A) == 0)) {
        onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, (s32) this->unk75B, (s32) (s8) this->unk20A);
    }
    return 0U;
}

/* daNpc_Aj1_c::init_AJ1_1 (void) */
u8 init_AJ1_1__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x520U) == 0) {
        onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x502U);
        sp8 = @4269.unk0;
        spC = @4269.unk4;
        sp10 = @4269.unk8;
        set_action__11daNpc_Aj1_cFM11daNpc_Aj1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk1C4 &= 0xFFFFFF7F;
        this->unk1C4 |= 0x4000;
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::init_AJ1_2 (void) */
u8 init_AJ1_2__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x520U) != 0) {
        this->unk104 = daNpc_Aj1_XyCheck_CB__FPvi;
        sp8 = @4284.unk0;
        spC = @4284.unk4;
        sp10 = @4284.unk8;
        set_action__11daNpc_Aj1_cFM11daNpc_Aj1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::createInit (void) */
u8 createInit__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 temp_r0;
    u8 var_r3;

    this->unk75B = (u8) ((u32) this->unkB0 >> 8U);
    this->mEventIdTable = getEventIdx__16dEvent_manager_cFPCcUc(&g_dComIfG_gameInfo.unk12A0.unk402C, l_evn_tbl, 0xFFU);
    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->unk2C4, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 6, (fopNpc_npc_c *) this);
    this->unk280 = 0xA;
    this->unk26D = 0xAD;
    this->unk26F = 0xAD;
    this->unk7B9 = 9;
    temp_r0 = this->unk7BF;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r3 = init_AJ1_0__11daNpc_Aj1_cFv(this);
        break;
    case 1:
        var_r3 = init_AJ1_1__11daNpc_Aj1_cFv(this);
        break;
    case 2:
        var_r3 = init_AJ1_2__11daNpc_Aj1_cFv(this);
        break;
    default:
        var_r3 = 0;
        break;
    }
    if (var_r3 == 0) {
        return 0U;
    }
    this->unk70E = this->unk204;
    this->unk710 = this->unk206;
    this->unk712 = this->unk208;
    this->unk20C = this->unk70E;
    this->unk20E = this->unk710;
    this->unk210 = this->unk712;
    this->unk258 = -4.5f;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk538, 0xFF, 0xFF, (fopAc_ac_c *) this);
    this->unk5B8 = &this->unk538;
    Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk574, &dNpc_cyl_src);
    play_animation__11daNpc_Aj1_cFv(this);
    CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo.unk12A0);
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo.unk12A0, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo.unk12A0, &this->unk41C);
    setMorf__14mDoExt_McaMorfFf(this->unk330, 0.0f);
    setMtx__11daNpc_Aj1_cFb(this, 1U);
    return 1U;
}

/* daNpc_Aj1_c::play_animation (void) */
void play_animation__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u32 var_r31;

    var_r31 = 0;
    play_texPttrnAnm__11daNpc_Aj1_cFv(this);
    if (this->unk35C & 0x20) {
        var_r31 = GetMtrlSndId__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo.unk12A0, &this->unk41C);
    }
    this->unk758 = play__14mDoExt_McaMorfFP3VecUlSc(this->unk330, &this->unk260, var_r31, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A));
    if (this->unk330->unk68 < this->unk740) {
        this->unk758 = 1;
    }
    this->unk740 = this->unk330->unk68;
}

/* daNpc_Aj1_c::ctrl_WAITanm (void) */
void ctrl_WAITanm__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 temp_r0;

    temp_r0 = this->unk7B9;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        if ((cLib_calcTimer<s>__FPs(&this->unk748) == 0) && ((s8) this->unk758 != 0) && (chk_parts_notMov__11daNpc_Aj1_cFv(this) != 0)) {
            setAnm_NUM__11daNpc_Aj1_cFii(this, 1, 1);
            return;
        }
        return;
    case 1:
        if ((s8) this->unk758 != 0) {
            setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
            this->unk748 = cLib_getRndValue<i>__Fii(0x5A, 0xB4);
        }
        break;
    }
}

/* daNpc_Aj1_c::ctrl_TIREanm (void) */
void ctrl_TIREanm__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if ((s32) this->unk7B9 == 6) {
        if (((s8) this->unk758 != 0) && (cLib_calcTimer<Uc>__FPUc(&this->unk75C) == 0)) {
            setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
            setMorf__14mDoExt_McaMorfFf(this->unk330, 20.0f);
        }
    } else {
        this->unk75C = 0;
    }
}

/* daNpc_Aj1_c::setMtx (bool) */
void setMtx__11daNpc_Aj1_cFb(daNpc_Aj1_c *this, u8 arg0) {
    J3DModel *temp_r3;
    void **temp_r3_2;
    void **temp_r4;

    temp_r3 = this->unk330->unk50;
    temp_r3->unk18 = this->unk214;
    temp_r3->unk1C = this->unk218;
    temp_r3->unk20 = this->unk21C;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk70E, this->unk710, this->unk712);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->unk330->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->unk330);
    temp_r4 = this->unk6D0;
    if (temp_r4 != NULL) {
        PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CE * 0x30), (mDoMtx_stack_c *) (temp_r4 + 0x24));
        temp_r3_2 = this->unk6D0;
        (*temp_r3_2)->unk10(temp_r3_2);
    }
    setAttention__11daNpc_Aj1_cFb(this, arg0);
}

/* daNpc_Aj1_c::bckResID (int) */
s32 bckResID__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0) {
    return a_res_id_tbl$4414[arg0];
}

/* daNpc_Aj1_c::btpResID (int) */
s32 btpResID__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0) {
    return a_res_id_tbl$4419[arg0];
}

/* daNpc_Aj1_c::init_texPttrnAnm (char signed, bool) */
u8 init_texPttrnAnm__11daNpc_Aj1_cFScb(daNpc_Aj1_c *this, s8 arg0, s32 arg1, ? arg_sp0) {
    s32 sp8;
    J3DModel *temp_r28;
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r28 = this->unk330->unk50;
    if (arg0 < 0) {
        return 0U;
    }
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA), (s8 *) btpResID__11daNpc_Aj1_cFi(this, (s32) arg0), (u16) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xD), (s8 *)0x213, (s32) (temp_r4 + 0x1D), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_2 + 0xD, 0x213, temp_r3_2 + 0x28);
    }
    this->unk7B8 = arg0;
    this->unk6EC = 0;
    this->unk6EE = 0;
    sp8 = 0;
    return init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6D8, temp_r28->unk4, (J3DAnmTexPattern *) temp_r3, 1, 0, 1.0f, 0, -1, arg1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0;
}

/* daNpc_Aj1_c::play_texPttrnAnm (void) */
void play_texPttrnAnm__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s16 temp_r3;

    if ((((s8) (u8) this->unk7B8 != 0) || (cLib_calcTimer<s>__FPs(&this->unk6EE) == 0)) && (this->unk6EC += 1, temp_r3 = this->unk6E0->unk6, (((s32) this->unk6EC < temp_r3) == 0))) {
        if ((s8) (u8) this->unk7B8 != 0) {
            this->unk6EC = (u8) temp_r3;
            return;
        }
        this->unk6EE = cLib_getRndValue<i>__Fii(0x3C, 0x5A);
        this->unk6EC = 0;
    }
}

/* daNpc_Aj1_c::setAnm_anm (daNpc_Aj1_c::anm_prm_c *) */
void setAnm_anm__11daNpc_Aj1_cFPQ211daNpc_Aj1_c9anm_prm_c(daNpc_Aj1_c *this, daNpc_Aj1_c::anm_prm_c *arg0) {
    JPABaseEmitter *temp_r3;
    u8 temp_r4;

    temp_r4 = arg0->unk0;
    if ((s8) temp_r4 >= 0) {
        if ((s8) this->unk7B9 == (s8) temp_r4) {
            return;
        }
        dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->unk330, arg0->unkC, arg0->unk4, arg0->unk8, bckResID__11daNpc_Aj1_cFi(this, (s32) (s8) temp_r4), -1, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA);
        this->unk7B9 = arg0->unk0;
        this->unk758 = 0;
        this->unk759 = 0;
        this->unk740 = 0.0f;
        if ((s32) this->unk7B9 != 2) {
            temp_r3 = this->unk794;
            if (temp_r3 != NULL) {
                temp_r3->unk20C |= 1;
                this->unk7B4 = 1;
            }
            del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(this, &this->unk790);
            del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(this, &this->unk798);
        } else {
            set_pa_pun__11daNpc_Aj1_cFv(this);
            set_pa_aka__11daNpc_Aj1_cFv(this);
            set_pa_don__11daNpc_Aj1_cFv(this);
        }
    }
}

/* daNpc_Aj1_c::setAnm_NUM (int, int) */
void setAnm_NUM__11daNpc_Aj1_cFii(daNpc_Aj1_c *this, s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        init_texPttrnAnm__11daNpc_Aj1_cFScb(this, (s8) a_anm_prm_tbl$4477[arg0].unk1, 1);
    }
    setAnm_anm__11daNpc_Aj1_cFPQ211daNpc_Aj1_c9anm_prm_c(this, (daNpc_Aj1_c::anm_prm_c *) &a_anm_prm_tbl$4477[arg0]);
}

/* daNpc_Aj1_c::setAnm (void) */
void setAnm__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    init_texPttrnAnm__11daNpc_Aj1_cFScb(this, (s8) a_anm_prm_tbl$4484[(s8) this->mSttNum].unk1, 1);
    setAnm_anm__11daNpc_Aj1_cFPQ211daNpc_Aj1_c9anm_prm_c(this, (daNpc_Aj1_c::anm_prm_c *) &a_anm_prm_tbl$4484[(s8) this->mSttNum]);
}

/* daNpc_Aj1_c::chngAnmTag (void) */
void chngAnmTag__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::ctrlAnmTag (void) */
void ctrlAnmTag__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::chngAnmAtr (char unsigned) */
void chngAnmAtr__11daNpc_Aj1_cFUc(daNpc_Aj1_c *this, u8 arg0) {
    if (arg0 != (u8) this->mAnmAtr) {
        if (arg0 > 9U) {
            return;
        }
        this->mAnmAtr = arg0;
        setAnm_ATR__11daNpc_Aj1_cFv(this);
        if ((s32) this->mAnmAtr != 8) {
            return;
        }
        this->unk75C = 3;
    }
}

/* daNpc_Aj1_c::ctrlAnmAtr (void) */
void ctrlAnmAtr__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mAnmAtr;
    switch ((s32) temp_r0) {                        /* irregular */
    case 2:
        ctrl_WAITanm__11daNpc_Aj1_cFv(this);
        break;
    case 5:
        if ((s8) this->unk758 != 0) {
            this->mAnmAtr = 0;
            setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
        }
        break;
    }
    ctrl_TIREanm__11daNpc_Aj1_cFv(this);
}

/* daNpc_Aj1_c::setAnm_ATR (void) */
void setAnm_ATR__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    init_texPttrnAnm__11daNpc_Aj1_cFScb(this, (s8) a_anm_prm_tbl$4535[this->mAnmAtr].unk1, 1);
    setAnm_anm__11daNpc_Aj1_cFPQ211daNpc_Aj1_c9anm_prm_c(this, (daNpc_Aj1_c::anm_prm_c *) &a_anm_prm_tbl$4535[this->mAnmAtr]);
}

/* daNpc_Aj1_c::anmAtr (short unsigned) */
void anmAtr__11daNpc_Aj1_cFUs(daNpc_Aj1_c *this, u16 arg0) {
    u8 temp_r4;

    switch ((s32) arg0) {                           /* irregular */
    case 6:
        if ((s8) this->unk7C1 == 0) {
            chngAnmAtr__11daNpc_Aj1_cFUc(this, g_dComIfG_gameInfo.unk5BDB);
            this->unk7C1 += 1;
        }
        temp_r4 = g_dComIfG_gameInfo.unk5BDC;
        if ((temp_r4 != 0xFF) && (temp_r4 != (u8) this->unk7B7)) {
            g_dComIfG_gameInfo.unk5BDC = 0xFF;
            this->unk7B7 = temp_r4;
            chngAnmTag__11daNpc_Aj1_cFv(this);
        }
        break;
    case 14:
        this->unk7C1 = 0;
        break;
    }
    ctrlAnmAtr__11daNpc_Aj1_cFv(this);
    ctrlAnmTag__11daNpc_Aj1_cFv(this);
}

/* daNpc_Aj1_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__11daNpc_Aj1_cFPUl(daNpc_Aj1_c *this, u32 *arg0, ? arg_sp0) {
    s32 var_r31;
    u32 temp_r3;

    var_r31 = 0xF;
    temp_r3 = *arg0;
    switch (temp_r3) {
    case 0x9C6:
        *arg0 = 0x9C7;
        break;
    case 0x9C7:
        *arg0 = 0x9C8;
        break;
    case 0x9C9:
        *arg0 = 0x9CA;
        break;
    case 0x9C8:
    case 0x9CA:
        if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 1U) != 0) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3704U) == 0)) {
            *arg0 = 0x9DA;
        } else {
            var_r31 = 0x10;
        }
        break;
    case 0x9DB:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x504U) == 0) {
            if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x2A80U) != 0) {
                *arg0 = 0x9DC;
            } else {
                *arg0 = 0x9CB;
            }
        } else {
            *arg0 = 0x9CF;
        }
        break;
    case 0x9CB:
    case 0x9DC:
        *arg0 = 0x9CC;
        break;
    case 0x9CD:
        *arg0 = 0x9CE;
        break;
    case 0x9CF:
        if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 1U) != 0) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3704U) == 0)) {
            *arg0 = 0x9DD;
        } else {
            var_r31 = 0x10;
        }
        break;
    case 0x9D0:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x2A20U) == 0) {
            *arg0 = 0x9D1;
        } else if (dKy_daynight_check__Fv() == 0) {
            *arg0 = 0x9D2;
        } else {
            *arg0 = 0x9D3;
        }
        break;
    default:
        var_r31 = 0x10;
        break;
    }
    return var_r31;
}

/* daNpc_Aj1_c::getMsg_AJ1_0 (void) */
s32 getMsg_AJ1_0__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 var_r3;

    var_r3 = 0x9C6;
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x510U) != 0) {
        var_r3 = 0x9C9;
    }
    return var_r3;
}

/* daNpc_Aj1_c::getMsg_AJ1_1 (void) */
s32 getMsg_AJ1_1__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 var_r3;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0xE20U) != 0) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3702U) == 0)) {
        onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3702U);
        return 0x9DB;
    }
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x504U) != 0) {
        return 0x9CF;
    }
    var_r3 = 0x9CB;
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x2A80U) != 0) {
        var_r3 = 0x9DC;
    }
    return var_r3;
}

/* daNpc_Aj1_c::getMsg_AJ1_2 (void) */
s32 getMsg_AJ1_2__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 temp_r0;
    s32 temp_r0_2;

    if ((u8) this->unk75A == 0x48) {
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3701U) == 0) {
            return 0x9D7;
        }
        temp_r0_2 = 0 - isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0xB20U);
        return ((temp_r0_2 - temp_r0_2) - !M2C_CARRY) + 0x9D9;
    }
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3708U) == 0) {
        return 0x9D0;
    }
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x2A20U) == 0) {
        return 0x9D4;
    }
    temp_r0 = dKy_daynight_check__Fv() - 1;
    return ((temp_r0 - temp_r0) - !M2C_CARRY) + 0x9D6;
}

/* daNpc_Aj1_c::getMsg (void) */
s32 getMsg__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 var_r4;
    u8 temp_r0;

    var_r4 = 0;
    temp_r0 = this->unk7BF;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r4 = getMsg_AJ1_0__11daNpc_Aj1_cFv(this);
        break;
    case 1:
        var_r4 = getMsg_AJ1_1__11daNpc_Aj1_cFv(this);
        break;
    case 2:
        var_r4 = getMsg_AJ1_2__11daNpc_Aj1_cFv(this);
        break;
    }
    return var_r4;
}

/* daNpc_Aj1_c::eventOrder (void) */
void eventOrder__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 2)) {
        this->unkFA |= 1;
        if ((s32) this->unk7BF == 2) {
            this->unkFA |= 0x20;
        }
        if ((s32) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
        }
    } else if ((s8) temp_r0 >= 3) {
        this->mEventIdx = (s8) temp_r0 - 3;
        fopAcM_orderOtherEventId__FP10fopAc_ac_csUcUsUsUs((fopAc_ac_c *) this, (this + (this->mEventIdx * 2))->mEventIdTable, 0xFFU, 0xFFFFU, 0U, 1U);
    }
}

/* daNpc_Aj1_c::checkOrder (void) */
void checkOrder__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 2:
        if (startCheck__16dEvent_manager_cFs(&g_dComIfG_gameInfo.unk52CC, (this + (this->mEventIdx * 2))->mEventIdTable) != 0) {
            if ((s16) this->mEventIdx != 0) {

            } else {
                this->unk1C4 &= 0xFFFFBFFF;
            }
            this->mOrderType = 0;
            return;
        }
        return;
    case 1:
        temp_r0_2 = this->mOrderType;
        if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 2)) {
            this->mOrderType = 0;
            this->unk769 = 1;
        }
        break;
    }
}

/* daNpc_Aj1_c::chk_talk (void) */
u8 chk_talk__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 var_r0;

    var_r0 = 0;
    if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
        var_r0 = 1;
    }
    if (var_r0 != 0) {
        if (ChkPresentEnd__16dEvent_manager_cFv(&g_dComIfG_gameInfo.unk52CC) != 0) {
            this->unk75A = g_dComIfG_gameInfo.unk52B9;
            return 1U;
        }
        return 0U;
    }
    this->unk75A = 0xFF;
    return 1U;
}

/* daNpc_Aj1_c::chk_parts_notMov (void) */
u8 chk_parts_notMov__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 var_r5;

    var_r5 = 0;
    if (((s16) this->unk73A == (s16) this->unk292) && ((s16) this->unk73C == (s16) this->unk296) && ((s16) this->unk738 == (s16) this->unk206)) {
        var_r5 = 1;
    }
    return var_r5;
}

/* daNpc_Aj1_c::lookBack (void) */
void lookBack__11daNpc_Aj1_cFv(daNpc_Aj1_c *this, ? arg_sp0) {
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
    f32 *var_r31;
    s16 var_r30;
    u8 temp_r0;
    u8 temp_r29;

    this->unk73A = this->unk292;
    this->unk73C = this->unk296;
    this->unk738 = this->unk206;
    sp20 = this->unk1F8;
    sp24 = this->unk1FC;
    sp28 = this->unk200;
    sp24 = this->unk264;
    sp2C = 0.0f;
    sp30 = 0.0f;
    sp34 = 0.0f;
    var_r31 = NULL;
    var_r30 = this->unk206;
    temp_r29 = this->unk76A;
    temp_r0 = this->unk7BD;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        dNpc_playerEyePos__Ff(-20.0f);
        sp2C = sp14;
        sp30 = sp18;
        sp34 = sp1C;
        var_r31 = &sp2C;
        break;
    case 2:
        sp2C = this->unk720;
        sp30 = this->unk724;
        sp34 = this->unk728;
        var_r31 = &sp2C;
        break;
    case 3:
        var_r30 = this->unk756;
        break;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk754, l_HIO.unk1E, 4, 0x800);
    if ((u8) this->unk29A == 0) {
        this->unk754 = 0;
    }
    sp8 = sp20;
    spC = sp24;
    sp10 = sp28;
    lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk206, (cXyz *) var_r31, (cXyz) &sp8, var_r30, this->unk754, temp_r29);
}

/* daNpc_Aj1_c::chkAttention (void) */
u8 chkAttention__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if (LockonTruth__12dAttention_cFv(&g_dComIfG_gameInfo.unk5808) != 0) {
        return this == LockonTarget__12dAttention_cFl(&g_dComIfG_gameInfo.unk5808, 0);
    }
    return this == ActionTarget__12dAttention_cFl(&g_dComIfG_gameInfo.unk5808, 0);
}

/* daNpc_Aj1_c::setAttention (bool) */
void setAttention__11daNpc_Aj1_cFb(daNpc_Aj1_c *this, u8 arg0) {
    this->unk274 = this->unk1F8;
    this->unk278 = this->unk1FC + l_HIO.unk20;
    this->unk27C = this->unk200;
    if (((s32) this->unk764 != 0) || (arg0 != 0)) {
        this->unk260.unk0 = this->unk714.unk0;
        this->unk264 = this->unk718;
        this->unk268 = this->unk71C;
    }
}

/* daNpc_Aj1_c::decideType (int) */
u8 decideType__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0) {
    u8 var_r4;

    this->unk7BE = 0;
    this->unk7BF = -1U;
    switch (arg0) {                                 /* irregular */
    case 0:
        this->unk7BF = 0;
        break;
    case 1:
        this->unk7BF = 1;
        break;
    case 2:
        this->unk7BF = 2;
        break;
    }
    var_r4 = 0;
    if (((s8) this->unk7BE != -1) && ((s8) this->unk7BF != -1)) {
        var_r4 = 1;
    }
    return var_r4;
}

/* daNpc_Aj1_c::cut_init_AJ1_TLK (void) */
void cut_init_AJ1_TLK__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    this->mAnmAtr = 0xFF;
    this->unk7B7 = 0xFF;
    this->unk7C1 = 0;
}

/* daNpc_Aj1_c::cut_move_AJ1_TLK (void) */
u8 cut_move_AJ1_TLK__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        this->mAnmAtr = 0xFF;
        this->unk7B7 = 0xFF;
        this->unk7C1 = 0;
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_INI_ANGRY (void) */
void cut_init_INI_ANGRY__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
    setMorf__14mDoExt_McaMorfFf(this->unk330, 8.0f);
}

/* daNpc_Aj1_c::cut_move_INI_ANGRY (void) */
u8 cut_move_INI_ANGRY__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    return 1U;
}

/* daNpc_Aj1_c::cut_init_VIVRATE (void) */
void cut_init_VIVRATE__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::cut_move_VIVRATE (void) */
u8 cut_move_VIVRATE__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((s8) this->unk758 != 0) {
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x58A2U, NULL, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4185.unk3C, @4185.unk3C, @4185.unk48, @4185.unk48, 0U);
        sp8 = @4185.unk8;
        spC = @4185.unk3C;
        sp10 = @4185.unk8;
        StartShock__12dVibration_cFii4cXyz(&g_dComIfG_gameInfo.unk59A0, 5, -0x11, (cXyz) &sp8);
        setAnm_NUM__11daNpc_Aj1_cFii(this, 3, 1);
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_JMP (void) */
void cut_init_JMP__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::cut_move_JMP (void) */
u8 cut_move_JMP__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if ((s8) this->unk758 != 0) {
        set_quake__9dDetect_cFPC4cXyz(&g_dComIfG_gameInfo.unk5A24, NULL);
        this->unk74C = 0x14;
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_SPPRISE (void) */
void cut_init_SPPRISE__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::cut_move_SPPRISE (void) */
u8 cut_move_SPPRISE__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if (cLib_calcTimer<s>__FPs(&this->unk74C) == 0) {
        setAnm_NUM__11daNpc_Aj1_cFii(this, 4, 1);
        this->unk74C = l_HIO.unk36;
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_LOK (void) */
void cut_init_LOK__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::cut_move_LOK (void) */
u8 cut_move_LOK__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if (((s8) this->unk758 != 0) && (cLib_calcTimer<s>__FPs(&this->unk74C) == 0)) {
        setAnm_NUM__11daNpc_Aj1_cFii(this, 5, 1);
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x4894U, (Vec *) &this->unk1F8, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), 1.0f, 1.0f, -1.0f, -1.0f, 0U);
        this->unk74C = l_HIO.unk38;
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_DAN (void) */
void cut_init_DAN__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {

}

/* daNpc_Aj1_c::cut_move_DAN (void) */
u8 cut_move_DAN__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if (((s8) this->unk758 != 0) && (cLib_calcTimer<s>__FPs(&this->unk74C) == 0)) {
        setAnm_NUM__11daNpc_Aj1_cFii(this, 2, 1);
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::cut_init_INVIT (void) */
void cut_init_INVIT__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x4893U, (Vec *) &this->unk1F8, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), 1.0f, 1.0f, -1.0f, -1.0f, 0U);
}

/* daNpc_Aj1_c::cut_move_INVIT (void) */
u8 cut_move_INVIT__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    return 1U;
}

/* daNpc_Aj1_c::privateCut (int) */
void privateCut__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0, ? arg_sp0) {
    u8 temp_r0;
    u8 temp_r0_2;
    u8 var_r3;

    if (arg0 != -1) {
        this->unk7B5 = getMyActIdx__16dEvent_manager_cFiPCPCciii(&g_dComIfG_gameInfo.unk52CC, arg0, a_cut_tbl$4900, 8, 1, 0);
        if ((s8) this->unk7B5 == -1) {
            cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo.unk52CC, arg0);
            return;
        }
        if (getIsAddvance__16dEvent_manager_cFi(&g_dComIfG_gameInfo.unk52CC, arg0) != 0) {
            temp_r0 = this->unk7B5;
            if ((u32) (s8) temp_r0 <= 7U) {
                switch ((s8) temp_r0) {             /* switch 1 */
                case 0:                             /* switch 1 */
                    cut_init_AJ1_TLK__11daNpc_Aj1_cFv(this);
                    break;
                case 1:                             /* switch 1 */
                    cut_init_INI_ANGRY__11daNpc_Aj1_cFv(this);
                    break;
                case 2:                             /* switch 1 */
                    cut_init_VIVRATE__11daNpc_Aj1_cFv(this);
                    break;
                case 3:                             /* switch 1 */
                    cut_init_JMP__11daNpc_Aj1_cFv(this);
                    break;
                case 4:                             /* switch 1 */
                    cut_init_SPPRISE__11daNpc_Aj1_cFv(this);
                    break;
                case 5:                             /* switch 1 */
                    cut_init_LOK__11daNpc_Aj1_cFv(this);
                    break;
                case 6:                             /* switch 1 */
                    cut_init_DAN__11daNpc_Aj1_cFv(this);
                    break;
                case 7:                             /* switch 1 */
                    cut_init_INVIT__11daNpc_Aj1_cFv(this);
                    break;
                }
            }
        }
        temp_r0_2 = this->unk7B5;
        if ((u32) (s8) temp_r0_2 <= 7U) {
            switch ((s8) temp_r0_2) {               /* switch 2 */
            case 0:                                 /* switch 2 */
                var_r3 = cut_move_AJ1_TLK__11daNpc_Aj1_cFv(this);
                break;
            case 1:                                 /* switch 2 */
                var_r3 = cut_move_INI_ANGRY__11daNpc_Aj1_cFv(this);
                break;
            case 2:                                 /* switch 2 */
                var_r3 = cut_move_VIVRATE__11daNpc_Aj1_cFv(this);
                break;
            case 3:                                 /* switch 2 */
                var_r3 = cut_move_JMP__11daNpc_Aj1_cFv(this);
                break;
            case 4:                                 /* switch 2 */
                var_r3 = cut_move_SPPRISE__11daNpc_Aj1_cFv(this);
                break;
            case 5:                                 /* switch 2 */
                var_r3 = cut_move_LOK__11daNpc_Aj1_cFv(this);
                break;
            case 6:                                 /* switch 2 */
                var_r3 = cut_move_DAN__11daNpc_Aj1_cFv(this);
                break;
            case 7:                                 /* switch 2 */
                var_r3 = cut_move_INVIT__11daNpc_Aj1_cFv(this);
                break;
            }
        } else {
            var_r3 = 1;
        }
        if (var_r3 != 0) {
            cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo.unk52CC, arg0);
        }
    }
}

/* daNpc_Aj1_c::endEvent (void) */
void endEvent__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    g_dComIfG_gameInfo.unk52C0 |= 8;
    this->mAnmAtr = 0xFF;
    this->unk7B7 = 0xFF;
}

/* daNpc_Aj1_c::isEventEntry (void) */
void isEventEntry__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(&g_dComIfG_gameInfo.unk52CC, this->unk2C4.unk0, NULL, 0);
}

/* daNpc_Aj1_c::event_proc (int) */
void event_proc__11daNpc_Aj1_cFi(daNpc_Aj1_c *this, s32 arg0) {
    if (endCheck__16dEvent_manager_cFs(&g_dComIfG_gameInfo.unk52CC, (this + (this->mEventIdx * 2))->mEventIdTable) != 0) {
        if ((s16) this->mEventIdx != 0) {

        } else {
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x508U);
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x504U);
        }
        endEvent__11daNpc_Aj1_cFv(this);
        return;
    }
    if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
        privateCut__11daNpc_Aj1_cFi(this, arg0);
    }
    lookBack__11daNpc_Aj1_cFv(this);
}

/* daNpc_Aj1_c::set_pa_pun (void) */
void set_pa_pun__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;

    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk790 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x8113U, (cXyz *) &this->unk1F8, NULL, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    if ((JPABaseEmitter *) this->unk790 != NULL) {
        this->unk74A = 0;
    }
}

/* daNpc_Aj1_c::set_pa_aka (void) */
void set_pa_aka__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JPABaseEmitter *temp_r3;

    temp_r3 = this->unk794;
    if (temp_r3 != NULL) {
        temp_r3->unk60 = -1;
        temp_r3->unk20C |= 1;
    }
    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk794 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x811FU, (cXyz *) &this->unk1F8, NULL, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
}

/* daNpc_Aj1_c::set_pa_don (void) */
void set_pa_don__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;

    sp18 = @4185.unk4C;
    sp1C = @4185.unk8;
    sp20 = @4185.unk50;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk206);
    PSMTXMultVec(&now__14mDoMtx_stack_c, &sp18, &this->unk7A8);
    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk798 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x8114U, &this->unk7A8, NULL, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
}

/* daNpc_Aj1_c::set_pa_smk (void) */
void set_pa_smk__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JPABaseEmitter *temp_r3;
    JPABaseEmitter *temp_r3_2;
    JPABaseEmitter *temp_r3_3;

    sp18 = @4185.unk54;
    PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CF * 0x30), &now__14mDoMtx_stack_c);
    this->unk79C = now__14mDoMtx_stack_c.unkC;
    this->unk7A0 = now__14mDoMtx_stack_c.unk1C;
    this->unk7A4 = now__14mDoMtx_stack_c.unk2C;
    this->unk76C.unk0->unk20(&this->unk76C);
    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk78C = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 2U, 0x2027U, (cXyz *) &this->unk79C, (csXyz *) &this->unk204, NULL, 0xC8U, &this->unk76C, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    temp_r3 = this->unk78C;
    if (temp_r3 != NULL) {
        temp_r3->unk1F0 = @4185.unk58;
        temp_r3->unk1F4 = @4185.unk58;
        temp_r3->unk1F8 = @4185.unk3C;
        temp_r3_2 = this->unk78C;
        temp_r3_2->unk1D8 = @4185.unk5C;
        temp_r3_2->unk1DC = @4185.unk5C;
        temp_r3_2->unk1E0 = @4185.unk5C;
        this->unk78C->unk64 = 0x28;
        this->unk78C->unk38 = @4185.unk60;
        this->unk78C->unk60 = 1;
        this->unk78C->unk74 = @4185.unk64;
        temp_r3_3 = this->unk78C;
        temp_r3_3->unkC = @4185.unk3C;
        temp_r3_3->unk10 = @4185.unk5C;
        temp_r3_3->unk14 = @4185.unk3C;
        this->unk782 = (u8) sp18;
        this->unk783 = unksp19;
        this->unk784 = unksp1A;
        this->unk785 = unksp1B;
    }
}

/* daNpc_Aj1_c::flw_pa_pun (void) */
void flw_pa_pun__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    JPABaseEmitter *temp_r5;

    temp_r5 = this->unk790;
    if (temp_r5 != NULL) {
        JPASetRMtxTVecfromMtx__FPA4_fPA4_fRQ29JGeometry8TVec3<f>(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), (f32 (*)[4]) temp_r5->unk1A8, &temp_r5->unk1E4);
        if (cLib_calcTimer<s>__FPs(&this->unk74A) == 0) {
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x58A3U, (Vec *) &this->unk1F8, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), 1.0f, 1.0f, -1.0f, -1.0f, 0U);
            this->unk74A = 5;
        }
    }
}

/* daNpc_Aj1_c::flw_pa_aka (void) */
void flw_pa_aka__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    JPABaseEmitter *temp_r5;

    temp_r5 = this->unk794;
    if (temp_r5 != NULL) {
        JPASetRMtxTVecfromMtx__FPA4_fPA4_fRQ29JGeometry8TVec3<f>(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), (f32 (*)[4]) temp_r5->unk1A8, &temp_r5->unk1E4);
    }
}

/* daNpc_Aj1_c::del_pa_aka (void) */
void del_pa_aka__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    JPABaseEmitter *temp_r5;

    if ((u8) this->unk7B4 != 0) {
        temp_r5 = this->unk794;
        if (temp_r5 != NULL) {
            if ((temp_r5->unk184 + temp_r5->unk190) == 0) {
                temp_r5->unk60 = -1;
                temp_r5->unk20C |= 1;
                this->unk7B4 = 0;
                this->unk794 = NULL;
            }
        } else {
            this->unk7B4 = 0;
        }
    }
}

/* daNpc_Aj1_c::del_pa (JPABaseEmitter * *) */
void del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(daNpc_Aj1_c *this, JPABaseEmitter **arg0) {
    JPABaseEmitter *temp_r3;

    temp_r3 = *arg0;
    if (temp_r3 != NULL) {
        temp_r3->unk60 = -1;
        temp_r3->unk20C |= 1;
        *arg0 = NULL;
    }
}

/* daNpc_Aj1_c::setSmoke (void) */
void setSmoke__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    if ((s32) this->unk7B9 == 2) {
        if ((checkPass__12J3DFrameCtrlFf(&this->unk330->unk58, @4185.unk8) != 0) || (checkPass__12J3DFrameCtrlFf(&this->unk330->unk58, @4185.unk68) != 0)) {
            set_pa_smk__11daNpc_Aj1_cFv(this);
        }
        if ((checkPass__12J3DFrameCtrlFf(&this->unk330->unk58, @4185.unk8) != 0) || (checkPass__12J3DFrameCtrlFf(&this->unk330->unk58, @4185.unk6C) != 0)) {
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x58A4U, (Vec *) &this->unk1F8, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4185.unk3C, @4185.unk3C, @4185.unk48, @4185.unk48, 0U);
        }
    }
}

s32 set_action__11daNpc_Aj1_cFM11daNpc_Aj1_cFPCvPvPv_iPv(daNpc_Aj1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0) {
    if (__ptmf_cmpr(arg0 + 0x6F0) != 0) {
        if (__ptmf_test(arg0 + 0x6F0) != 0) {
            arg0->unk7C0 = 9;
            __ptmf_scall(arg0, arg2);
        }
        arg0->unk6F0 = (s32) arg1->unk0;
        arg0->unk6F4 = arg1->unk4;
        arg0->unk6F8 = arg1->unk8;
        arg0->unk7C0 = 0;
        __ptmf_scall(arg0, arg2);
    }
    return 1;
}

/* daNpc_Aj1_c::setStt (char signed) */
void setStt__11daNpc_Aj1_cFSc(daNpc_Aj1_c *this, s8 arg0) {
    u8 temp_r0;
    u8 temp_r0_2;
    u8 temp_r5;

    temp_r5 = this->mSttNum;
    this->mSttNum = (u8) arg0;
    temp_r0 = this->mSttNum;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        this->mOrderType = 0;
        this->unk750 = cLib_getRndValue<i>__Fii(0x5A, 0x78);
    default:
block_14:
        setAnm__11daNpc_Aj1_cFv(this);
        return;
    case 2:
        this->mOrderType = 0;
        goto block_14;
    case 3:
        this->mOrderType = 0;
        this->mAnmAtr = 0xFF;
        this->unk7B7 = 0xFF;
        this->unk7C1 = 0;
        this->mSttNumOld = temp_r5;
        goto block_14;
    case 4:
        this->mOrderType = 0;
        this->unk748 = cLib_getRndValue<i>__Fii(0x5A, 0xB4);
        temp_r0_2 = this->unk7B9;
        if ((s8) temp_r0_2 != 1) {
            if ((s8) temp_r0_2 == 6) {
                return;
            }
            goto block_14;
        }
        break;
    }
}

/* daNpc_Aj1_c::chk_areaIN (float, short, cXyz) */
u8 chk_areaIN__11daNpc_Aj1_cFfs4cXyz(daNpc_Aj1_c *this, f32 arg0, s16 arg1, ? arg2) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 temp_f29;
    f32 var_f30;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s16 temp_r3;

    __mi__4cXyzCFRC3Vec(&sp18, g_dComIfG_gameInfo.unk5B44 + 0x1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4185.unk8;
    sp14 = sp20;
    temp_f1 = PSVECSquareMag(&spC);
    if (temp_f1 > @4185.unk8) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = @4185.unk70 * temp_f0 * (@4185.unk78 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4185.unk70 * temp_f0_2 * (@4185.unk78 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) temp_f1 * (@4185.unk70 * temp_f0_3 * (@4185.unk78 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f30 = sp8;
    } else {
        var_f30 = temp_f1;
    }
    temp_f29 = g_dComIfG_gameInfo.unk5B44->unk1FC - arg2->unk4;
    temp_r3 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, g_dComIfG_gameInfo.unk5B44 + 0x1F8) - this->unk70A;
    if ((var_f30 < arg0) && ((f32) fabs(temp_f29) < @4185.unk80) && (abs(temp_r3) < arg1)) {
        return 1U;
    }
    return 0U;
}

/* daNpc_Aj1_c::FARwai (void) */
s32 FARwai__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    sp8 = this->unk1F8;
    spC = this->unk1FC;
    sp10 = this->unk200;
    if (chk_areaIN__11daNpc_Aj1_cFfs4cXyz(this, l_HIO.unk28, l_HIO.unk2C, (cXyz) &sp8) != 0) {
        setStt__11daNpc_Aj1_cFSc(this, 2);
        this->unk7BD = 1;
        this->unk76A = 0;
        this->unk29A = 1;
        return 1;
    }
    if ((s32) this->unk7B9 == 8) {
        if ((s8) this->unk758 != 0) {
            setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
            setMorf__14mDoExt_McaMorfFf(this->unk330, 8.0f);
        }
        return 1;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk750) == 0) {
        setAnm_NUM__11daNpc_Aj1_cFii(this, 8, 1);
        this->unk750 = cLib_getRndValue<i>__Fii(0x5A, 0x78);
    }
    return 1;
}

/* daNpc_Aj1_c::call_1 (void) */
s32 call_1__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk769 != 0) {
        if (chk_talk__11daNpc_Aj1_cFv(this) != 0) {
            setStt__11daNpc_Aj1_cFSc(this, 3);
            this->unk7BD = 1;
            this->unk76A = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    sp14 = this->unk1F8;
    sp18 = this->unk1FC;
    sp1C = this->unk200;
    if ((u8) (chk_areaIN__11daNpc_Aj1_cFfs4cXyz(this, l_HIO.unk28, l_HIO.unk2C, (cXyz) &sp14) == 0) != 0) {
        setStt__11daNpc_Aj1_cFSc(this, 1);
        setMorf__14mDoExt_McaMorfFf(this->unk330, 15.0f);
        this->unk7BD = 3;
        this->unk756 = this->unk70A;
        this->unk76A = 0;
        this->unk29A = 1;
        return 1;
    }
    this->unk29A = 1;
    sp8 = this->unk1F8;
    spC = this->unk1FC;
    sp10 = this->unk200;
    if (chk_areaIN__11daNpc_Aj1_cFfs4cXyz(this, l_HIO.unk30, l_HIO.unk34, (cXyz) &sp8) != 0) {
        this->mOrderType = 2;
        if ((u8) this->unk768 != 0) {
            if ((s8) this->unk7B9 != 0) {
                setAnm_NUM__11daNpc_Aj1_cFii(this, 0, 1);
                setMorf__14mDoExt_McaMorfFf(this->unk330, 15.0f);
            }
            return 1;
        }
        if ((s32) this->unk7B9 != 7) {
            setAnm_NUM__11daNpc_Aj1_cFii(this, 7, 1);
        }
        goto block_13;
    }
block_13:
    return 1;
}

/* daNpc_Aj1_c::wait_1 (void) */
s32 wait_1__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    u8 temp_r0;

    cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk70A, 4, 0x400, 0);
    ctrl_WAITanm__11daNpc_Aj1_cFv(this);
    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 >= 3)) {
        return 1;
    }
    ctrl_TIREanm__11daNpc_Aj1_cFv(this);
    if ((u8) this->unk769 != 0) {
        if (chk_talk__11daNpc_Aj1_cFv(this) != 0) {
            setStt__11daNpc_Aj1_cFSc(this, 3);
            this->unk7BD = 1;
            this->unk76A = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    if (((s32) this->unk7B9 != 1) && ((u8) this->unk768 != 0)) {
        this->unk7BD = 1;
        return 1;
    }
    this->unk7BD = 3;
    this->unk756 = this->unk70A;
    return 1;
}

/* daNpc_Aj1_c::talk_1 (void) */
u8 talk_1__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 temp_r3_2;
    u16 temp_r0;
    u8 temp_r31;
    void *temp_r3;

    temp_r31 = chk_parts_notMov__11daNpc_Aj1_cFv(this);
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
        case 0x9DA:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3704U);
            /* fallthrough */
        case 0x9C8:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x510U);
            break;
        case 0x9DD:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3704U);
            break;
        case 0x9D1:                                 /* switch 2 */
        case 0x9D2:                                 /* switch 2 */
        case 0x9D3:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3708U);
            break;
        case 0x9D7:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x3701U);
            break;
        }
        this->unk75A = 0xFF;
        this->unk769 = 0;
        setStt__11daNpc_Aj1_cFSc(this, (s8) this->mSttNumOld);
        this->unk74E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
        endEvent__11daNpc_Aj1_cFv(this);
        break;
    }
    return temp_r31;
}

/* daNpc_Aj1_c::wait_action1 (void *) */
s32 wait_action1__11daNpc_Aj1_cFPv(daNpc_Aj1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk7C0;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Aj1_cFSc(this, 1);
        this->unk7C0 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk768 = chkAttention__11daNpc_Aj1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk764 = FARwai__11daNpc_Aj1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk764 = call_1__11daNpc_Aj1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk764 = talk_1__11daNpc_Aj1_cFv(this);
            break;
        }
        lookBack__11daNpc_Aj1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Aj1_c::wait_action2 (void *) */
s32 wait_action2__11daNpc_Aj1_cFPv(daNpc_Aj1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk7C0;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Aj1_cFSc(this, 4);
        if ((strcmp(&g_dComIfG_gameInfo.unk5134, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0x62) == 0) && ((s32) this->unk7BF == 1) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo.unk624, 0x508U) == 0)) {
            this->mOrderType = 3;
        }
        this->unk7C0 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk768 = chkAttention__11daNpc_Aj1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 4:                                     /* switch 2 */
            this->unk764 = wait_1__11daNpc_Aj1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk764 = talk_1__11daNpc_Aj1_cFv(this);
            break;
        }
        lookBack__11daNpc_Aj1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Aj1_c::demo (void) */
u8 demo__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp8;
    J3DAnmTexPattern *temp_r3_2;
    dDemo_actor_c *temp_r3;
    s16 temp_r4;

    if ((u8) this->unk1C0 == 0) {
        if ((u8) this->unk76B != 0) {
            this->unk76B = 0;
        }
    } else {
        if ((u8) this->unk76B == 0) {
            this->unk76B = 1;
            this->unk75F = 0;
            this->unk292 = 0;
            this->unk290 = 0;
            this->unk296 = 0;
            this->unk294 = 0;
        }
        temp_r3 = getActor__14dDemo_object_cFUc(g_dComIfG_gameInfo.unk5AC8 + 0x20, this->unk1C0);
        if ((J3DAnmTexPattern *) this->unk6E0 != NULL) {
            this->unk6EC += 1;
            temp_r4 = this->unk6E0->unk6;
            if ((s32) this->unk6EC >= temp_r4) {
                this->unk6EC = (u8) temp_r4;
            }
        }
        temp_r3_2 = getP_BtpData__13dDemo_actor_cFPCc(temp_r3, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA);
        if (temp_r3_2 != NULL) {
            sp8 = 0;
            init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6D8, this->unk330->unk50->unk4, temp_r3_2, 1, 0, 1.0f, 0, -1, 1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
            this->unk7B8 = 1;
            this->unk6EC = 0;
        }
        dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc((fopAc_ac_c *) this, 0x6AU, this->unk330, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA, 0, NULL, 0U, 0);
    }
    return this->unk76B;
}

/* daNpc_Aj1_c::shadowDraw (void) */
void shadowDraw__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f3;
    u32 temp_r4;
    void **temp_r5;

    temp_f3 = this->unk1FC;
    sp8 = this->unk1F8;
    spC = @4185.unk88 + temp_f3;
    sp10 = this->unk200;
    this->unk6D4 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unk6D4, 1, this->unk330->unk50, (cXyz *) &sp8, @4185.unk8C, @4185.unk90, temp_f3, this->unk3C8, &this->unk41C, &this->unk10C, 0, @4185.unk3C, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    temp_r4 = this->unk6D4;
    if (temp_r4 != 0) {
        temp_r5 = this->unk6D0;
        if (temp_r5 != NULL) {
            addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo.unk5F6C, temp_r4, (J3DModel *) temp_r5);
        }
    }
}

/* daNpc_Aj1_c::_draw (void) */
s32 _draw__11daNpc_Aj1_cFv(daNpc_Aj1_c *this, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r31;
    J3DModelData *temp_r30;
    void **temp_r4;

    temp_r31 = this->unk330->unk50;
    temp_r30 = temp_r31->unk4;
    if (((u8) this->unk75E != 0) || ((u8) this->unk760 != 0)) {
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &this->unk1F8, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r31, &this->unk10C);
    entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->unk6D8, temp_r30, (s16) this->unk6EC);
    entryDL__14mDoExt_McaMorfFv(this->unk330);
    removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r30->unk58, this->unk6E0);
    temp_r4 = this->unk6D0;
    if (temp_r4 != NULL) {
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r4, &this->unk10C);
        mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) this->unk6D0);
    }
    shadowDraw__11daNpc_Aj1_cFv(this);
    if ((u8) l_HIO.unk24 != 0) {
        sp8 = this->unk1F8;
        spC = this->unk1FC;
        sp10 = this->unk200;
        spC = this->unk264;
    }
    dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x4CU, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
    return 1;
}

/* daNpc_Aj1_c::_execute (void) */
s32 _execute__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 spC;
    s32 sp8;
    s32 var_r4;

    if ((u8) this->unk761 == 0) {
        this->unk6FC = this->unk1F8;
        this->unk700 = this->unk1FC;
        this->unk704 = this->unk200;
        this->unk708 = this->unk204;
        this->unk70A = this->unk206;
        this->unk70C = this->unk208;
        this->unk761 = 1;
    }
    sp8 = (s32) l_HIO.unk12;
    spC = (s32) l_HIO.unk1C;
    setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, l_HIO.unk14, l_HIO.unk16, l_HIO.unk18, l_HIO.unk1A, l_HIO.unkC, l_HIO.unkE, l_HIO.unk10, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    if (((u8) this->unk75E != 0) && ((u8) this->unk1C0 == 0)) {
        return 1;
    }
    checkOrder__11daNpc_Aj1_cFv(this);
    if (demo__11daNpc_Aj1_cFv(this) == 0) {
        var_r4 = -1;
        if (((u8) g_dComIfG_gameInfo.unk529A != 0) && ((u16) this->unkF8 != 1)) {
            var_r4 = isEventEntry__11daNpc_Aj1_cFv(this);
        }
        if (var_r4 >= 0) {
            event_proc__11daNpc_Aj1_cFi(this, var_r4);
        } else {
            __ptmf_scall(this, 0);
        }
        fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, (cXyz *) &this->unk538);
        play_animation__11daNpc_Aj1_cFv(this);
        CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo.unk12A0);
    }
    eventOrder__11daNpc_Aj1_cFv(this);
    this->unk70E = this->unk204;
    this->unk710 = this->unk206;
    this->unk712 = this->unk208;
    if ((u8) this->unk75F == 0) {
        this->unk20C = this->unk204;
        this->unk20E = this->unk206;
        this->unk210 = this->unk208;
    }
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo.unk12A0, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo.unk12A0, &this->unk41C);
    setMtx__11daNpc_Aj1_cFb(this, 0U);
    flw_pa_pun__11daNpc_Aj1_cFv(this);
    del_pa_aka__11daNpc_Aj1_cFv(this);
    flw_pa_aka__11daNpc_Aj1_cFv(this);
    setSmoke__11daNpc_Aj1_cFv(this);
    if ((u8) this->unk76B == 0) {
        setCollision__12fopNpc_npc_cFff((fopNpc_npc_c *) this, 60.0f, 140.0f);
    }
    return 1;
}

/* daNpc_Aj1_c::_delete (void) */
s32 _delete__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    mDoExt_McaMorf *temp_r3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk6C4, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA);
    if ((u32) this->unkF0 != 0U) {
        temp_r3 = this->unk330;
        if (temp_r3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3);
        }
    }
    del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(this, &this->unk790);
    del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(this, &this->unk794);
    del_pa__11daNpc_Aj1_cFPP14JPABaseEmitter(this, &this->unk798);
    this->unk76C.unk0->unk20(&this->unk76C);
    return 1;
}

/* daNpc_Aj1_c::_create (void) */
s32 _create__11daNpc_Aj1_cFv(daNpc_Aj1_c *this, ? arg_sp0) {
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
            this->vtable6C0 = (struct __vt__12fopNpc_npc_c *) &__vt__11daNpc_Aj1_c;
            this->unk6D8.vtable0 = &__vt__14mDoExt_baseAnm;
            this->unk6DC = 0;
            this->unk6D8.vtable0 = (struct __vt__14mDoExt_baseAnm *) &__vt__13mDoExt_btpAnm;
            this->unk6E4 = 0;
            __ct__18dPa_smokeEcallBackFUc((dPa_smokeEcallBack *) &this->unk76C, 1U);
        }
        this->unk1C8 |= 8;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk6C4, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA);
    if (temp_r3 != 4) {
        return temp_r3;
    }
    if (decideType__11daNpc_Aj1_cFi(this, (s32) (u8) this->unkB0) == 0) {
        return 5;
    }
    if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) CheckCreateHeap__FP10fopAc_ac_c, a_siz_tbl$5553[(s8) this->unk7BE]) == 0) {
        return 5;
    }
    this->unk22C = &this->unk330->unk50->unk24;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, @4185.unkA8, @4185.unk40, @4185.unkA8, @4185.unkAC, @4185.unkB0, @4185.unkB4);
    var_r3 = 5;
    if (createInit__11daNpc_Aj1_cFv(this) != 0) {
        var_r3 = temp_r3;
    }
    return var_r3;
}

/* daNpc_Aj1_c::bodyCreateHeap (void) */
s32 bodyCreateHeap__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JUTAssertion *temp_r3;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *temp_r3_4;
    mDoExt_McaMorf *var_r0;
    s8 *temp_r3_10;
    s8 *temp_r3_12;
    s8 *temp_r3_2;
    s8 *temp_r3_6;
    s8 *temp_r3_8;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    s8 *temp_r4_4;
    s8 *temp_r4_5;
    u8 temp_r3_11;
    u8 temp_r3_5;
    u8 temp_r3_7;
    u8 temp_r3_9;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA), (s8 *)0xA, (u16) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xD), (s8 *)0x8BE, (s32) (temp_r4 + 0x6A), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_2 + 0xD, 0x8BE, temp_r3_2 + 0x28);
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
    if (init_texPttrnAnm__11daNpc_Aj1_cFScb(this, 0, 0) == 0) {
        this->unk330 = NULL;
        return 0;
    }
    temp_r3_5 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0x79);
    this->unk6CC = temp_r3_5;
    if ((s8) this->unk6CC < 0) {
        temp_r4_2 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_5), (u32) (temp_r4_2 + 0xD), (s8 *)0x8D3, (s32) (temp_r4_2 + 0x7E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_6 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_6 + 0xD, 0x8D3, temp_r3_6 + 0x28);
    }
    temp_r3_7 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0x91);
    this->unk6CD = temp_r3_7;
    if ((s8) this->unk6CD < 0) {
        temp_r4_3 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_7), (u32) (temp_r4_3 + 0xD), (s8 *)0x8D5, (s32) (temp_r4_3 + 0x9A), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_8 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_8 + 0xD, 0x8D5, temp_r3_8 + 0x28);
    }
    temp_r3_9 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xAF);
    this->unk6CE = temp_r3_9;
    if ((s8) this->unk6CE < 0) {
        temp_r4_4 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_9), (u32) (temp_r4_4 + 0xD), (s8 *)0x8D7, (s32) (temp_r4_4 + 0xB5), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_10 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_10 + 0xD, 0x8D7, temp_r3_10 + 0x28);
    }
    temp_r3_11 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xCA);
    this->unk6CF = temp_r3_11;
    if ((s8) this->unk6CF < 0) {
        temp_r4_5 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_11), (u32) (temp_r4_5 + 0xD), (s8 *)0x8D9, (s32) (temp_r4_5 + 0xD0), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_12 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_12 + 0xD, 0x8D9, temp_r3_12 + 0x28);
    }
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CC * 4) & 0x3FFFC)))->unk8 = nodeCB_Head__FP7J3DNodei;
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CD * 4) & 0x3FFFC)))->unk8 = nodeCB_BackBone__FP7J3DNodei;
    this->unk330->unk50->unk14 = this;
    return 1;
}

/* daNpc_Aj1_c::itemCreateHeap (void) */
s32 itemCreateHeap__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt" + 0xA), (s8 *)9, (u16) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xD), (s8 *)0x8F3, (s32) (temp_r4 + 0x6A), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "angry\0Aj1\0Aj\0d_a_npc_aj1.cpp\0a_btp != 0\0Halt";
        OSPanic(temp_r3_2 + 0xD, 0x8F3, temp_r3_2 + 0x28);
    }
    this->unk6D0 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x11000022U);
    if ((void **) this->unk6D0 == NULL) {
        return 0;
    }
    return 1;
}

/* daNpc_Aj1_c::CreateHeap (void) */
s32 CreateHeap__11daNpc_Aj1_cFv(daNpc_Aj1_c *this) {
    s32 sp8;

    if (bodyCreateHeap__11daNpc_Aj1_cFv(this) == 0) {
        return 0;
    }
    if (itemCreateHeap__11daNpc_Aj1_cFv(this) == 0) {
        this->unk330 = NULL;
        return 0;
    }
    SetWall__12dBgS_AcchCirFff(&this->unk4F8, 30.0f, 60.0f);
    sp8 = 0;
    Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&this->unk334, (cXyz *) &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, &this->unk4F8, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    return 1;
}

/* daNpc_Aj1_Create (fopAc_ac_c *) */
void daNpc_Aj1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _create__11daNpc_Aj1_cFv((daNpc_Aj1_c *) arg0);
}

/* daNpc_Aj1_Delete (daNpc_Aj1_c *) */
void daNpc_Aj1_Delete__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0) {
    _delete__11daNpc_Aj1_cFv(arg0);
}

/* daNpc_Aj1_Execute (daNpc_Aj1_c *) */
void daNpc_Aj1_Execute__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0) {
    _execute__11daNpc_Aj1_cFv(arg0);
}

/* daNpc_Aj1_Draw (daNpc_Aj1_c *) */
void daNpc_Aj1_Draw__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0) {
    _draw__11daNpc_Aj1_cFv(arg0);
}

/* daNpc_Aj1_IsDelete (daNpc_Aj1_c *) */
s32 daNpc_Aj1_IsDelete__FP11daNpc_Aj1_c(daNpc_Aj1_c *arg0) {
    return 1;
}

/* d_a_npc_aj1_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_aj1_cpp(void) {
    __ct__15daNpc_Aj1_HIO_cFv(&l_HIO);
    __register_global_object(__dt__15daNpc_Aj1_HIO_cFv, @4146);
}