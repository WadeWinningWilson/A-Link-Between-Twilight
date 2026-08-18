// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in naming-map-auto.md;
// remaining unkNNN are still inference-needed. Behavior identical to the full draft.
// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_ob1 (79 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_ob1\asm\d\actor\d_a_npc_ob1.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ void *unk4;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0xC];                      /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x14 */ daNpc_Ob1_c *unk14;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x18 */ char pad18[0xC];                     /* maybe part of unk14[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
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
    /* 0x00 */ char pad0[6];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x06 */ s16 unk6;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0x20];                     /* maybe part of unk6[0x11]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x28 */ u16 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2A */ char pad2A[0x2A];                    /* maybe part of unk28[0x16]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2A (receipt f_op_actor.h) */
    /* 0x54 */ JUTNameTab *unk54;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
} JUTAssertion;                                     /* size >= 0x58 */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0x4 */

struct __vt__14mDoHIO_entry_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoHIO_entry_c *, s16);
};                                                  /* size = 0xC */

struct __vt__15daNpc_Ob1_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Ob1_HIO_c *, s16);
};                                                  /* size = 0xC */

struct _struct_a_anm_prm_tbl$4448_0x14 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

struct _struct_a_anm_prm_tbl$4455_0x14 {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

struct _struct_a_anm_prm_tbl$4520_0x14 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

typedef struct dNpc_EventCut_c {
    /* 0x0 */ s8 *unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_EventCut_c;                                  /* size >= 0x4 */

typedef struct dNpc_PathRun_c {
    /* 0x0 */ void *unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_PathRun_c;                                   /* size >= 0x4 */

typedef struct daNpc_Ob1_HIO_c {
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
    /* 0x24 */ char pad24[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x28 */ s16 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2A */ char pad2A[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2A (receipt f_op_actor.h) */
    /* 0x2C */ f32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ f32 unk30;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x34 */ f32 unk34;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x38 */ f32 unk38;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
} daNpc_Ob1_HIO_c;                                  /* size >= 0x3C */

typedef struct daNpc_Ob1_c {
    /* 0x000 */ f32 unk0;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x004 */ f32 unk4;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x008 */ f32 unk8;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x00C */ char padC[0xA4];                    /* maybe part of unk8[0x2A]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
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
    /* 0x212 */ char pad212[0xE];                   /* maybe part of unk210[8]? */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x6 (receipt f_op_actor.h) */
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
    /* 0x29B */ char pad29B[0x95];                  /* maybe part of unk29A[0x96]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x330 */ mDoExt_McaMorf *mpBtpRes;             /* inferred */  /* [NAMED-BY-RULE: btp resource (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x334 */ dBgS_Acch unk334;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x334 */ char pad334[0x94];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3C8 */ f32 unk3C8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3CC */ char pad3CC[0x50];                  /* maybe part of unk3C8[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ cBgS_PolyInfo unk41C;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ char pad41C[0xDC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F8 */ dBgS_AcchCir unk4F8;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F8 */ char pad4F8[0x40];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ dCcD_Stts unk538;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ char pad538[0x3C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ dCcD_Cyl unk574;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ char pad574[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5B8 */ dCcD_Stts *unk5B8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5BC */ char pad5BC[0xE8];                  /* maybe part of unk5B8[0x3B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A4 */ u32 unk6A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A8 */ char pad6A8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6AC */ s32 unk6AC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B0 */ void *unk6B0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B4 */ u32 unk6B4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B8 */ char pad6B8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6BC */ u8 unk6BC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6BD */ char pad6BD[3];                     /* maybe part of unk6BC[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C0 */ void *unk6C0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ request_of_phase_process_class unk6C4; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ char pad6C4[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CC */ u8 unk6CC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CD */ u8 unk6CD;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CE */ char pad6CE[2];                     /* maybe part of unk6CD[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D0 */ u32 unk6D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ void **unk6D4;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ JUTAssertion *unk6D8;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ mDoExt_btpAnm unk6DC;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ char pad6DC[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E4 */ J3DAnmTexPattern *unk6E4;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E8 */ char pad6E8[8];                     /* maybe part of unk6E4[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F0 */ u8 mBtpFrame;                          /* inferred */  /* [NAMED-BY-RULE: btp frame (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x6F1 */ char pad6F1[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F2 */ s16 mBlinkTimer;                         /* inferred */  /* [NAMED-BY-RULE: blink timer (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x6F4 */ ? unk6F4;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F4 */ char pad6F4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ s32 unk6F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6FC */ s32 unk6FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x700 */ dNpc_PathRun_c unk700;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x700 */ char pad700[5];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x705 */ u8 unk705;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x706 */ u8 unk706;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x707 */ char pad707[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x708 */ dNpc_EventCut_c unk708;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x708 */ char pad708[0x6C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x774 */ u32 unk774;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x778 */ f32 unk778;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x77C */ f32 unk77C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x780 */ f32 unk780;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x784 */ s16 unk784;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x786 */ s16 unk786;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x788 */ s16 unk788;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x78A */ s16 unk78A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x78C */ s16 unk78C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x78E */ s16 unk78E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x790 */ ? unk790;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x790 */ char pad790[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x794 */ f32 unk794;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x798 */ f32 unk798;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x79C */ f32 unk79C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A0 */ f32 unk7A0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A4 */ f32 unk7A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A8 */ Vec unk7A8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7A8 */ char pad7A8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7AC */ f32 unk7AC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B0 */ f32 unk7B0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B4 */ f32 unk7B4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7B8 */ f32 unk7B8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7BC */ f32 unk7BC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C0 */ f32 unk7C0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C4 */ s16 unk7C4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C6 */ s16 unk7C6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C8 */ s16 unk7C8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7CA */ char pad7CA[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7CC */ s32 unk7CC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D0 */ s16 mEventIdTable;                         /* inferred */  /* [NAMED-BY-RULE: event id table (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x7D2 */ s16 mEventIdx;                         /* inferred */  /* [NAMED-BY-RULE: event index (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x7D4 */ char pad7D4[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D6 */ s16 mSttTimer;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D8 */ char pad7D8[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7DA */ s16 unk7DA;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7DC */ char pad7DC[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7DE */ s16 unk7DE;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E0 */ s16 unk7E0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E2 */ u16 unk7E2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ u8 unk7E4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E5 */ u8 unk7E5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E6 */ char pad7E6[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E7 */ u8 unk7E7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E8 */ u8 unk7E8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E9 */ u8 unk7E9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EA */ u8 unk7EA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EB */ u8 unk7EB;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EC */ u8 unk7EC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7ED */ u8 unk7ED;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EE */ u8 unk7EE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EF */ u8 unk7EF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F0 */ u8 unk7F0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F1 */ u8 unk7F1;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F2 */ u8 unk7F2;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F3 */ u8 unk7F3;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F4 */ s32 unk7F4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F8 */ u8 unk7F8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F9 */ u8 unk7F9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FA */ u8 unk7FA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FB */ u8 unk7FB;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FC */ u8 unk7FC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FD */ char pad7FD[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FE */ u8 unk7FE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7FF */ u8 unk7FF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x800 */ u8 mAnmAtr;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x801 */ u8 unk801;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x802 */ u8 mBtpNum;                          /* inferred */  /* [NAMED-BY-RULE: setAnm_tex store (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x803 */ u8 unk803;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x804 */ u8 mOrderType;                          /* inferred */  /* [NAMED-BY-RULE: eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x805 */ u8 mSttNum;                          /* inferred */  /* [NAMED-BY-RULE: setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md)] */
    /* 0x806 */ u8 unk806;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x807 */ u8 unk807;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x808 */ u8 unk808;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x809 */ u8 unk809;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x80A */ u8 unk80A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x80B */ u8 unk80B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_Ob1_c;                                      /* size >= 0x80C */

typedef struct daNpc_Ob1_c::anm_prm_c {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ char pad1[3];                        /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Ob1_c::anm_prm_c;                           /* size >= 0x10 */

typedef struct fopAc_ac_c {
    /* 0x000 */ char pad0[0x6A4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x6A4 */ s32 unk6A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A8 */ char pad6A8[0x10];                  /* maybe part of unk6A4[5]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B8 */ u16 unk6B8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} fopAc_ac_c;                                       /* size >= 0x6BA */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[0x10];                    /* maybe part of unk50[5]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x64 */ f32 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x10 */

s32 ActionTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
s32 ChkPresentEnd__16dEvent_manager_cFv(dEvent_manager_c *this); /* extern */
? CrrPos__9dBgS_AcchFR4dBgS(dBgS_Acch *this, dBgS *arg0); /* extern */
s8 GetPolyColor__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
s8 GetRoomId__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
s32 LockonTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
u8 LockonTruth__12dAttention_cFv(dAttention_c *this); /* extern */
? OSPanic(s8 *, ?, s8 *);                           /* extern */
? PSMTXCopy(mDoMtx_stack_c *, mDoMtx_stack_c *);    /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, void *, ? *);      /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
s32 __ptmf_cmpr(s32);                               /* extern */
? __ptmf_scall(daNpc_Ob1_c *, s32);                 /* extern */
s32 __ptmf_test(s32);                               /* extern */
? __register_global_object(void *(*)(daNpc_Ob1_HIO_c *, s16), void *(*)(cXyz *, s16), void *); /* extern */
? addReal__21dDlst_shadowControl_cFUlP8J3DModel(dDlst_shadowControl_c *this, u32 arg0, J3DModel *arg1); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
? cLib_chaseAngleS__FPsss(s16 *arg0, s16 arg1, s16 arg2); /* extern */
? cLib_chaseF__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
s16 cLib_targetAngleY__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
f32 cM_rndF__Ff(f32 arg0);                          /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
u8 chkPointPass__14dNpc_PathRun_cF4cXyzb(dNpc_PathRun_c *this, cXyz arg0, u8 arg1); /* extern */
? cutEnd__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 cutProc__15dNpc_EventCut_cFv(dNpc_EventCut_c *this); /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s32 dComIfG_resLoad__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
u32 dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(u32 arg0, s8 arg1, J3DModel *arg2, cXyz *arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, cBgS_PolyInfo *arg8, dKy_tevstr_c *arg9, s16 arg10, f32 arg11, _GXTexObj *arg12); /* extern */
? dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc(fopAc_ac_c *arg0, u8 arg1, mDoExt_McaMorf *arg2, s8 *arg3, s32 arg4, u16 *arg5, u32 arg6, s8 arg7); /* extern */
s32 dKy_daynight_check__Fv();                       /* extern */
? dNpc_playerEyePos__Ff(f32 arg0);                  /* extern */
? dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(mDoExt_McaMorf *arg0, s32 arg1, f32 arg2, f32 arg3, s32 arg4, s32 arg5, s8 *arg6); /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cfff(u8 arg0, fopAc_ac_c *arg1, f32 arg2, f32 arg3, f32 arg4); /* extern */
s32 endCheck__16dEvent_manager_cFs(dEvent_manager_c *this, s16 arg0); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
? fopAcM_SearchByID__FUiPP10fopAc_ac_c(u32 arg0, fopAc_ac_c **arg1); /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderOtherEventId__FP10fopAc_ac_csUcUsUsUs(fopAc_ac_c *arg0, s16 arg1, u8 arg2, u16 arg3, u16 arg4, u16 arg5); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
s32 fopAc_IsActor__FPv(void *arg0);                 /* extern */
s32 fopMsgM_messageSet__FUlP10fopAc_ac_c(u32 arg0, fopAc_ac_c *arg1); /* extern */
? fpcEx_Search__FPFPvPv_PvPv(void *(*arg0)(void *, void *), void *arg1); /* extern */
dDemo_actor_c *getActor__14dDemo_object_cFUc(dDemo_object_c *this, u8 arg0); /* extern */
s16 getEventIdx__16dEvent_manager_cFPCcUc(dEvent_manager_c *this, s8 *arg0, u8 arg1); /* extern */
u8 getEventReg__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
JUTAssertion *getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(dRes_control_c *this, s8 *arg0, u16 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
u8 getIndex__10JUTNameTabCFPCc(JUTNameTab *this, s8 *arg0); /* extern */
s32 getIsAddvance__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 getMyActIdx__16dEvent_manager_cFiPCPCciii(dEvent_manager_c *this, s32 arg0, s8 **arg1, s32 arg2, s32 arg3, s32 arg4); /* extern */
? getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(dEvent_manager_c *this, s8 *arg0, fopAc_ac_c *arg1, s32 arg2); /* extern */
s32 *getMySubstanceP__16dEvent_manager_cFiPCci(dEvent_manager_c *this, s32 arg0, s8 *arg1, s32 arg2); /* extern */
JUTAssertion *getP_BtpData__13dDemo_actor_cFPCc(dDemo_actor_c *this, s8 *arg0); /* extern */
? getPoint__14dNpc_PathRun_cFUc(dNpc_PathRun_c *this, u8 arg0); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb(dNpc_JntCtrl_c *this, s16 *arg0, cXyz *arg1, cXyz arg2, s16 arg3, s16 arg4, u8 arg5); /* extern */
void **mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_modelEntryDL__FP8J3DModel(J3DModel *arg0); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
u8 maxPoint__14dNpc_PathRun_cFv(dNpc_PathRun_c *this); /* extern */
? memcpy(? *, ? *, ?);                              /* extern */
u8 nextIdxAuto__14dNpc_PathRun_cFv(dNpc_PathRun_c *this); /* extern */
void *nextPath__14dNpc_PathRun_cFSc(dNpc_PathRun_c *this, s8 arg0); /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
u8 play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setCollision__12fopNpc_npc_cFff(fopNpc_npc_c *this, f32 arg0, f32 arg1); /* extern */
? setEventReg__11dSv_event_cFUsUc(dSv_event_c *this, u16 arg0, u8 arg1); /* extern */
? setInf__14dNpc_PathRun_cFUcScUc(dNpc_PathRun_c *this, u8 arg0, s8 arg1, u8 arg2); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setMorf__14mDoExt_McaMorfFf(mDoExt_McaMorf *this, f32 arg0); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
s32 startCheck__16dEvent_manager_cFs(dEvent_manager_c *this, s16 arg0); /* extern */
? stopZelAnime__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
s32 CreateHeap__11daNpc_Ob1_cFv(daNpc_Ob1_c *this); /* static */
void *__ct__11daNpc_Ob1_cFv(daNpc_Ob1_c *this);     /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__15daNpc_Ob1_HIO_cFv(daNpc_Ob1_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void anmAtr__11daNpc_Ob1_cFUs(daNpc_Ob1_c *this, u16 arg0); /* static */
s16 cLib_calcTimer<s>__FPs(s16 *arg0);              /* static */
s16 cLib_getRndValue<i>__Fii(s32 arg0, s32 arg1);   /* static */
void daNpc_Ob1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
void daNpc_Ob1_Delete__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0); /* static */
void daNpc_Ob1_Draw__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0); /* static */
void daNpc_Ob1_Execute__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0); /* static */
s32 daNpc_Ob1_IsDelete__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0); /* static */
s32 getMsg__11daNpc_Ob1_cFv(daNpc_Ob1_c *this);     /* static */
s32 next_msgStatus__11daNpc_Ob1_cFPUl(daNpc_Ob1_c *this, u32 *arg0); /* static */
void nodeOb1Control__11daNpc_Ob1_cFP7J3DNodeP8J3DModel(daNpc_Ob1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void plyTexPttrnAnm__11daNpc_Ob1_cFv(daNpc_Ob1_c *this); /* static */
void setAnm_ATR__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0); /* static */
void setAttention__11daNpc_Ob1_cFb(daNpc_Ob1_c *this, u8 arg0); /* static */
void setMtx__11daNpc_Ob1_cFb(daNpc_Ob1_c *this, u8 arg0); /* static */
s32 set_action__11daNpc_Ob1_cFM11daNpc_Ob1_cFPCvPvPv_iPv(daNpc_Ob1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0); /* static */
s32 wait_action1__11daNpc_Ob1_cFPv(daNpc_Ob1_c *this, void *arg0); /* static */
s32 wait_action2__11daNpc_Ob1_cFPv(daNpc_Ob1_c *this, void *arg0); /* static */
void *@4156(cXyz *this, s16 destroyFlag);           /* static */
extern dCcD_SrcCyl dNpc_cyl_src;
extern ? g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? j3dSys;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
static ? a_prm_tbl$4151;                            /* unable to generate initializer: unknown type */
static s8 *l_evn_tbl = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
static ? @4249;                                     /* unable to generate initializer: unknown type */
static ? @4263;                                     /* unable to generate initializer: unknown type */
static ? @4276;                                     /* unable to generate initializer: unknown type */
static struct _struct_a_anm_prm_tbl$4448_0x14 a_anm_prm_tbl$4448[8]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4455_0x14 a_anm_prm_tbl$4455[7]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4520_0x14 a_anm_prm_tbl$4520[7]; /* unable to generate initializer: non-zero padding */
static s8 *a_cut_tbl$5037 = "ACTION\0a_mdl_dat != 0\0head\0m_hed_jnt_num >= 0\0backbone\0m_bbone_jnt_num >= 0";
static u32 a_size_tbl$5467[1] = { 0x272E0 };
static s32 a_hed_mdl_resID_tbl$5728[1] = { 6 };
static ? @3569;
static daNpc_Ob1_HIO_c l_HIO;
static u8 l_check_flg;
static s32 a_bck_resID_tbl$4383[8] = { 3, 0, 1, 4, 2, 8, 9, 0xA }; /* const */
static s32 a_btp_resID_tbl$4388[2] = { 7, 7 };      /* const */

/* daNpc_Ob1_HIO_c::daNpc_Ob1_HIO_c (void) */
daNpc_Ob1_HIO_c *__ct__15daNpc_Ob1_HIO_cFv(daNpc_Ob1_HIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__15daNpc_Ob1_HIO_c;
    memcpy(&this->unkC, &a_prm_tbl$4151, 0x30);
    this->unk4 = -1;
    this->unk8 = -1;
    return this;
}

/* searchActor_Kb (void *, void *) */
void *searchActor_Kb__FPvPv(void *arg0, void *arg1) {
    u8 temp_r3;

    if (((s32) @3569.unkE4 < 0x14) && (fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0xDD)) {
        temp_r3 = arg0->unk405;
        if (temp_r3 != 0) {
            @3569.unkE8 = (u8) (@3569.unkE8 | temp_r3);
        }
        *(&@3569 + 0x94 + (@3569.unkE4 * 4)) = arg0;
        @3569.unkE4 = (s32) (@3569.unkE4 + 1);
    }
    return NULL;
}

/* nodeCallBack_Ob1 (J3DNode *, int) */
s32 nodeCallBack_Ob1__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ob1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            nodeOb1Control__11daNpc_Ob1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ob1_c::nodeOb1Control (J3DNode *, J3DModel *) */
void nodeOb1Control__11daNpc_Ob1_cFP7J3DNodeP8J3DModel(daNpc_Ob1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r28;
    u16 temp_r30;
    void *(*temp_r3)(daNpc_Ob1_HIO_c *, s16);

    if ((s8) @3569.unkF8 == 0) {
        @3569.unkFC = 20.0f;
        temp_r3 = &@3569 + 0xFC;
        temp_r3->unk4 = (f32) @4239.unk4;
        temp_r3->unk8 = (f32) @4239.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0xEC);
        @3569.unkF8 = 1U;
    }
    temp_r30 = arg0->unk18;
    temp_r28 = temp_r30 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r28, &now__14mDoMtx_stack_c);
    if ((s32) temp_r30 == (s8) this->unk6CC) {
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk292);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk290);
        PSMTXMultVec(&now__14mDoMtx_stack_c, &@3569 + 0xFC, &this->unk790);
    }
    if ((s32) temp_r30 == (s8) this->unk6CD) {
        mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk296);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk294);
    }
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r28);
}

/* daNpc_Ob1_c::init_OB1_0 (void) */
u8 init_OB1_0__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) == 0) {
        sp8 = @4249.unk0;
        spC = @4249.unk4;
        sp10 = @4249.unk8;
        set_action__11daNpc_Ob1_cFM11daNpc_Ob1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ob1_c::init_OB1_1 (void) */
u8 init_OB1_1__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 0)) {
        this->unk1C4 &= 0xFFFFFF7F;
        sp8 = @4263.unk0;
        spC = @4263.unk4;
        sp10 = @4263.unk8;
        set_action__11daNpc_Ob1_cFM11daNpc_Ob1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ob1_c::init_OB1_2 (void) */
u8 init_OB1_2__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 1)) {
        sp8 = @4276.unk0;
        spC = @4276.unk4;
        sp10 = @4276.unk8;
        set_action__11daNpc_Ob1_cFM11daNpc_Ob1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ob1_c::createInit (void) */
u8 createInit__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 var_r31;
    u8 temp_r0;
    u8 temp_r4;
    u8 var_r3;

    this->mEventIdTable = getEventIdx__16dEvent_manager_cFPCcUc(&g_dComIfG_gameInfo + 0x52CC, l_evn_tbl, 0xFFU);
    this->unk280 = 0xA;
    this->unk26D = 0xAC;
    this->unk26F = 0xAC;
    this->unk258 = -4.5f;
    this->unk784 = this->unk204;
    this->unk786 = this->unk206;
    this->unk788 = this->unk208;
    this->unk778 = this->unk1F8;
    this->unk77C = this->unk1FC;
    this->unk780 = this->unk200;
    this->unk79C = this->unk778;
    this->unk7A0 = this->unk77C;
    this->unk7A4 = this->unk780;
    var_r31 = 0xFF;
    temp_r4 = (u8) ((u32) this->unkB0 >> 0x10U);
    if (temp_r4 != 0xFF) {
        setInf__14dNpc_PathRun_cFUcScUc(&this->unk700, temp_r4, (s8) this->unk20A, 1U);
        if ((void *) this->unk700.unk0 == NULL) {
            return 0U;
        }
        this->unk1C4 &= 0xFFFFFF7F;
        var_r31 = 0xD9;
        goto block_4;
    }
block_4:
    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->unk708, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xA, (fopNpc_npc_c *) this);
    this->unk803 = 8;
    temp_r0 = this->unk809;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r3 = init_OB1_0__11daNpc_Ob1_cFv(this);
        break;
    case 1:
        var_r3 = init_OB1_1__11daNpc_Ob1_cFv(this);
        break;
    case 2:
        var_r3 = init_OB1_2__11daNpc_Ob1_cFv(this);
        break;
    default:
        var_r3 = 0;
        break;
    }
    if (var_r3 != 0) {
        this->unk78A = this->unk204;
        this->unk78C = this->unk206;
        this->unk78E = this->unk208;
        this->unk20C = this->unk78A;
        this->unk20E = this->unk78C;
        this->unk210 = this->unk78E;
        Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk538, var_r31, 0xFF, (fopAc_ac_c *) this);
        this->unk5B8 = &this->unk538;
        Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk574, &dNpc_cyl_src);
        setMorf__14mDoExt_McaMorfFf(this->mpBtpRes, 0.0f);
        setMtx__11daNpc_Ob1_cFb(this, 1U);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ob1_c::setMtx (bool) */
void setMtx__11daNpc_Ob1_cFb(daNpc_Ob1_c *this, u8 arg0, ? arg_sp0) {
    dBgS *temp_r29;
    void **temp_r3;

    if ((u8) this->unk7FB == 0) {
        plyTexPttrnAnm__11daNpc_Ob1_cFv(this);
        this->unk7E4 = play__14mDoExt_McaMorfFP3VecUlSc(this->mpBtpRes, &this->unk260, 0U, 0);
        if (this->mpBtpRes->unk68 < this->unk7B4) {
            this->unk7E4 = 1;
        }
        this->unk7B4 = this->mpBtpRes->unk68;
        CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo + 0x12A0);
    }
    temp_r29 = &g_dComIfG_gameInfo + 0x12A0;
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r29, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r29, &this->unk41C);
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk78C);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->mpBtpRes->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->mpBtpRes);
    PSMTXCopy(this->mpBtpRes->unk50->unk8C + ((s8) this->unk6CC * 0x30), (mDoMtx_stack_c *) (this->unk6D4 + 0x24));
    temp_r3 = this->unk6D4;
    (*temp_r3)->unk10(temp_r3);
    setAttention__11daNpc_Ob1_cFb(this, arg0);
}

/* daNpc_Ob1_c::anmNum_toResID (int) */
s32 anmNum_toResID__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    return a_bck_resID_tbl$4383[arg0];
}

/* daNpc_Ob1_c::btpNum_toResID (int) */
s32 btpNum_toResID__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    return a_btp_resID_tbl$4388[arg0];
}

/* daNpc_Ob1_c::setBtp (bool, int) */
s32 setBtp__11daNpc_Ob1_cFbi(daNpc_Ob1_c *this, s32 arg0, s32 arg1, ? arg_sp0) {
    s32 sp8;
    J3DModelData *temp_r29;
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r29 = this->unk6D4->unk4;
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE), (s8 *) btpNum_toResID__11daNpc_Ob1_cFi(this, arg1), (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    this->unk6D8 = temp_r3;
    if ((JUTAssertion *) this->unk6D8 == NULL) {
        temp_r4 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0x11), (s8 *)0x1FF, (s32) (temp_r4 + 0x21), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        OSPanic(temp_r3_2 + 0x11, 0x1FF, temp_r3_2 + 0x36);
    }
    sp8 = 0;
    if (init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6DC, temp_r29, (J3DAnmTexPattern *) this->unk6D8, 1, 2, 1.0f, 0, -1, arg0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0) {
        this->mBtpFrame = 0;
        this->mBlinkTimer = 0;
        return 1;
    }
    return 0;
}

/* daNpc_Ob1_c::iniTexPttrnAnm (bool) */
void iniTexPttrnAnm__11daNpc_Ob1_cFb(daNpc_Ob1_c *this, s32 arg0) {
    setBtp__11daNpc_Ob1_cFbi(this, arg0, (s32) (s8) this->mBtpNum);
}

/* daNpc_Ob1_c::plyTexPttrnAnm (void) */
void plyTexPttrnAnm__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s16 temp_r4;

    if ((((s8) this->mBtpNum != 0) || (cLib_calcTimer<s>__FPs(&this->mBlinkTimer) == 0)) && (this->mBtpFrame += 1, temp_r4 = this->unk6D8->unk6, (((s32) this->mBtpFrame < temp_r4) == 0))) {
        if ((s8) this->mBtpNum != 0) {
            this->mBtpFrame = (u8) temp_r4;
            return;
        }
        this->mBlinkTimer = (s16) (30.0f + cM_rndF__Ff(60.0f));
        this->mBtpFrame = 0;
    }
}

/* daNpc_Ob1_c::setAnm_tex (char signed) */
void setAnm_tex__11daNpc_Ob1_cFSc(daNpc_Ob1_c *this, s8 arg0) {
    if ((s8) this->mBtpNum != arg0) {
        this->mBtpNum = (u8) arg0;
        iniTexPttrnAnm__11daNpc_Ob1_cFb(this, 1);
    }
}

/* daNpc_Ob1_c::setAnm_anm (daNpc_Ob1_c::anm_prm_c *) */
s32 setAnm_anm__11daNpc_Ob1_cFPQ211daNpc_Ob1_c9anm_prm_c(daNpc_Ob1_c *this, daNpc_Ob1_c::anm_prm_c *arg0) {
    u8 temp_r5;

    temp_r5 = arg0->unk0;
    if ((s8) this->unk803 == (s8) temp_r5) {
        return 1;
    }
    this->unk803 = temp_r5;
    dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->mpBtpRes, arg0->unkC, arg0->unk4, arg0->unk8, anmNum_toResID__11daNpc_Ob1_cFi(this, (s32) (s8) this->unk803), -1, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE);
    this->unk7E4 = 0;
    this->unk7E5 = 0;
    this->unk7B4 = (f32) 0;
    return 1;
}

/* daNpc_Ob1_c::setAnm_NUM (int, int) */
void setAnm_NUM__11daNpc_Ob1_cFii(daNpc_Ob1_c *this, s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        setAnm_tex__11daNpc_Ob1_cFSc(this, (s8) a_anm_prm_tbl$4448[arg0].unk1);
    }
    setAnm_anm__11daNpc_Ob1_cFPQ211daNpc_Ob1_c9anm_prm_c(this, (daNpc_Ob1_c::anm_prm_c *) &a_anm_prm_tbl$4448[arg0]);
}

/* daNpc_Ob1_c::setAnm (void) */
s32 setAnm__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    struct _struct_a_anm_prm_tbl$4455_0x14 *temp_r4_2;
    u8 temp_r4;

    temp_r4 = a_anm_prm_tbl$4455[(s8) this->mSttNum].unk1;
    if ((s8) temp_r4 >= 0) {
        setAnm_tex__11daNpc_Ob1_cFSc(this, (s8) temp_r4);
    }
    temp_r4_2 = &a_anm_prm_tbl$4455[(s8) this->mSttNum];
    if ((s8) temp_r4_2->unk0 >= 0) {
        setAnm_anm__11daNpc_Ob1_cFPQ211daNpc_Ob1_c9anm_prm_c(this, (daNpc_Ob1_c::anm_prm_c *) temp_r4_2);
    }
    return 1;
}

/* daNpc_Ob1_c::chg_anmTag (void) */
void chg_anmTag__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {

}

/* daNpc_Ob1_c::control_anmTag (void) */
void control_anmTag__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {

}

/* daNpc_Ob1_c::chg_anmAtr (char unsigned) */
void chg_anmAtr__11daNpc_Ob1_cFUc(daNpc_Ob1_c *this, u8 arg0) {
    s32 temp_r5;

    temp_r5 = this->unk7CC;
    if ((u32) (temp_r5 + 0x10000) != -1U) {
        switch (temp_r5) {                          /* irregular */
        case 0xAFE:
            break;
        default:
        case 0xB09:
        case 0xB61:
        case 0xB5D:
        case 0xB02:
            this->unk807 = 1;
            this->unk7FA = 0;
            this->unk29A = 1;
            break;
        }
    }
    if (arg0 < 7U) {
        if (arg0 == (u8) this->mAnmAtr) {
            return;
        }
        this->mAnmAtr = arg0;
        setAnm_ATR__11daNpc_Ob1_cFi(this, 1);
    }
}

/* daNpc_Ob1_c::control_anmAtr (void) */
void control_anmAtr__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mAnmAtr;
    switch ((s32) temp_r0) {                        /* irregular */
    case 3:
        if ((s8) this->unk7E4 != 0) {
            setAnm_NUM__11daNpc_Ob1_cFii(this, 4, 1);
            this->mAnmAtr = 7;
            return;
        }
        return;
    case 6:
        if ((s8) this->unk7E4 != 0) {
            setAnm_NUM__11daNpc_Ob1_cFii(this, 7, 1);
            this->mAnmAtr = 7;
        }
        break;
    }
}

/* daNpc_Ob1_c::setAnm_ATR (int) */
void setAnm_ATR__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    if (arg0 != 0) {
        setAnm_tex__11daNpc_Ob1_cFSc(this, (s8) a_anm_prm_tbl$4520[this->mAnmAtr].unk1);
    }
    setAnm_anm__11daNpc_Ob1_cFPQ211daNpc_Ob1_c9anm_prm_c(this, (daNpc_Ob1_c::anm_prm_c *) &a_anm_prm_tbl$4520[this->mAnmAtr]);
}

/* daNpc_Ob1_c::anmAtr (short unsigned) */
void anmAtr__11daNpc_Ob1_cFUs(daNpc_Ob1_c *this, u16 arg0) {
    u8 temp_r4;
    void *temp_r5;

    temp_r5 = &g_dComIfG_gameInfo + 0x5C20;
    if (((u8) this->unk6BC == 2) && (this != (u32) (temp_r5 + (temp_r5->unk4 * 4))->unk4)) {
        control_anmTag__11daNpc_Ob1_cFv(this);
        control_anmAtr__11daNpc_Ob1_cFv(this);
        return;
    }
    switch ((s32) arg0) {                           /* irregular */
    case 6:
        if ((s8) this->unk80B == 0) {
            this->mAnmAtr = 0xFF;
            chg_anmAtr__11daNpc_Ob1_cFUc(this, g_dComIfG_gameInfo.unk5BDB);
            this->unk80B += 1;
        }
        temp_r4 = g_dComIfG_gameInfo.unk5BDC;
        g_dComIfG_gameInfo.unk5BDC = 0xFFU;
        if ((temp_r4 != 0xFF) && ((u8) this->unk801 != temp_r4)) {
            this->unk801 = temp_r4;
            chg_anmTag__11daNpc_Ob1_cFv(this);
        }
        break;
    case 14:
        this->unk80B = 0;
        break;
    }
    control_anmTag__11daNpc_Ob1_cFv(this);
    control_anmAtr__11daNpc_Ob1_cFv(this);
}

/* daNpc_Ob1_c::eventOrder (void) */
void eventOrder__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 2)) {
        this->unkFA |= 1;
        if ((s32) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
        }
    } else if ((s8) temp_r0 >= 3) {
        this->mEventIdx = (s8) temp_r0 - 3;
        fopAcM_orderOtherEventId__FP10fopAc_ac_csUcUsUsUs((fopAc_ac_c *) this, (this + (this->mEventIdx * 2))->mEventIdTable, 0xFFU, 0xFFFFU, 0U, 1U);
    }
}

/* daNpc_Ob1_c::checkOrder (void) */
void checkOrder__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 2:
        if (startCheck__16dEvent_manager_cFs(&g_dComIfG_gameInfo + 0x52CC, (this + (this->mEventIdx * 2))->mEventIdTable) != 0) {
            this->mOrderType = 0;
            return;
        }
        return;
    case 1:
        temp_r0_2 = this->mOrderType;
        if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 2)) {
            this->mOrderType = 0;
            this->unk7F9 = 1;
        }
        break;
    }
}

/* daNpc_Ob1_c::chk_talk (void) */
u8 chk_talk__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    u8 var_r0;
    u8 var_r31;

    var_r31 = 0;
    var_r0 = 0;
    if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
        var_r0 = 1;
    }
    if (var_r0 != 0) {
        if (ChkPresentEnd__16dEvent_manager_cFv(&g_dComIfG_gameInfo + 0x52CC) != 0) {
            this->unk7EB = g_dComIfG_gameInfo.unk52B9;
            var_r31 = 1;
        }
    } else {
        this->unk7EB = 0xFF;
        var_r31 = 1;
    }
    return var_r31;
}

/* daNpc_Ob1_c::chk_partsNotMove (void) */
u8 chk_partsNotMove__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    u8 var_r5;

    var_r5 = 0;
    if (((s16) this->unk7C4 == (s16) this->unk292) && ((s16) this->unk7C6 == (s16) this->unk296) && ((s16) this->unk7C8 == (s16) this->unk206)) {
        var_r5 = 1;
    }
    return var_r5;
}

/* daNpc_Ob1_c::lookBack (void) */
void lookBack__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
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
    f32 temp_f1;
    f32 temp_f2;
    f32 temp_f3;
    f32 temp_f4;
    s16 var_r30;
    u8 temp_r0;
    u8 temp_r29;

    this->unk7C4 = this->unk292;
    this->unk7C6 = this->unk296;
    this->unk7C8 = this->unk206;
    temp_f4 = this->unk1F8;
    sp20 = temp_f4;
    temp_f3 = this->unk1FC;
    sp24 = temp_f3;
    temp_f2 = this->unk200;
    sp28 = temp_f2;
    temp_f1 = this->unk264;
    sp24 = temp_f1;
    sp2C = 0.0f;
    sp30 = 0.0f;
    sp34 = 0.0f;
    var_r31 = NULL;
    var_r30 = this->unk206;
    temp_r29 = this->unk7FA;
    temp_r0 = this->unk807;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        dNpc_playerEyePos__Ff(-20.0f);
        sp2C = sp14;
        sp30 = sp18;
        sp34 = sp1C;
        var_r31 = &sp2C;
        sp20 = this->unk1F8;
        sp24 = this->unk1FC;
        sp28 = this->unk200;
        sp24 = this->unk264;
        break;
    case 2:
        sp2C = this->unk79C;
        sp30 = this->unk7A0;
        sp34 = this->unk7A4;
        var_r31 = &sp2C;
        sp20 = temp_f4;
        sp24 = temp_f3;
        sp28 = temp_f2;
        sp24 = temp_f1;
        break;
    case 3:
        var_r30 = this->unk7E0;
        break;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk7DE, l_HIO.unk1E, 4, 0x800);
    if ((u8) this->unk29A == 0) {
        this->unk7DE = 0;
    }
    sp8 = sp20;
    spC = sp24;
    sp10 = sp28;
    lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk206, (cXyz *) var_r31, (cXyz) &sp8, var_r30, this->unk7DE, temp_r29);
}

/* daNpc_Ob1_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__11daNpc_Ob1_cFPUl(daNpc_Ob1_c *this, u32 *arg0, ? arg_sp0) {
    dSv_event_c *temp_r29;
    s32 var_r31;
    u32 temp_r3;

    var_r31 = 0xF;
    temp_r3 = *arg0;
    switch (temp_r3) {
    case 0xA8D:
        *arg0 = 0xA8E;
        break;
    case 0xA90:
        *arg0 = 0xA91;
        break;
    case 0xA93:
        *arg0 = 0xA94;
        break;
    case 0xA97:
        *arg0 = 0xA98;
        break;
    case 0xA92:
    case 0xA96:
    case 0xA9B:
    case 0xA9D:
    case 0xAA0:
    case 0xAA2:
    case 0xAA3:
    case 0xAA4:
    case 0xAA5:
    case 0xAA6:
        temp_r29 = &g_dComIfG_gameInfo + 0x624;
        if ((isEventBit__11dSv_event_cFUs(temp_r29, 0xE20U) != 0) && (isEventBit__11dSv_event_cFUs(temp_r29, 0x308U) == 0)) {
            *arg0 = 0xAA7;
        } else {
        default:
            var_r31 = 0x10;
        }
        break;
    case 0xAA8:
        if (dKy_daynight_check__Fv() == 0) {
            *arg0 = 0xAA9;
        } else {
            *arg0 = 0xAAA;
        }
        break;
    case 0xAAD:
    case 0xAAE:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x304U) != 0) {
            *arg0 = 0xA8F;
        } else {
            *arg0 = 0xA8D;
        }
        break;
    }
    return var_r31;
}

/* daNpc_Ob1_c::getMsg_OB1_0 (void) */
s32 getMsg_OB1_0__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    dSv_event_c *temp_r30;
    s32 var_r31;
    s32 var_r3;
    u8 temp_r0;
    u8 temp_r0_2;
    u8 temp_r0_3;
    u8 temp_r0_4;
    u8 temp_r0_5;
    u8 temp_r3;

    var_r31 = 0;
    if (((s8) this->unk7E8 != 0) || ((s8) this->unk7E7 != 0)) {
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x302U) != 0) {
            temp_r3 = this->unk7E9;
            if ((s8) temp_r3 != 0) {
                if ((s32) this->unk7E7 == 2) {
                    if ((s8) this->unk7EA != 0) {
                        var_r31 = 0xAA2;
                        this->unk7EA = 0;
                    } else {
                        var_r31 = 0xAA1;
                        this->unk7EA = 1;
                    }
                } else {
                    switch ((s8) temp_r3) {         /* switch 1; irregular */
                    case 1:                         /* switch 1 */
                        if ((s32) this->unk7EA == 1) {
                            var_r31 = 0xA9D;
                            this->unk7EA = 0;
                        } else {
                            var_r31 = 0xA9C;
                            this->unk7EA = 1;
                        }
                        break;
                    case 2:                         /* switch 1 */
                        temp_r0 = this->unk7EA;
                        if ((s8) temp_r0 == 2) {
                            var_r31 = 0xA9F;
                            this->unk7EA = 1;
                        } else if ((s8) temp_r0 == 1) {
                            var_r31 = 0xAA0;
                            this->unk7EA = 0;
                        } else {
                            var_r31 = 0xA9E;
                            this->unk7EA = 2;
                        }
                        break;
                    }
                }
            } else {
                temp_r0_2 = this->unk7E8;
                if ((s8) this->unk7E7 > (s8) temp_r0_2) {
                    var_r31 = 0xAA3;
                } else {
                    switch ((s8) temp_r0_2) {       /* switch 2; irregular */
                    case 1:                         /* switch 2 */
                        var_r31 = 0xAA4;
                        break;
                    case 2:                         /* switch 2 */
                        var_r31 = 0xAA5;
                        break;
                    default:                        /* switch 2 */
                        var_r31 = 0xAA6;
                        break;
                    }
                }
            }
        } else {
            temp_r0_3 = this->unk7E9;
            switch ((s8) temp_r0_3) {               /* switch 3; irregular */
            case 1:                                 /* switch 3 */
                if ((s32) this->unk7EA == 1) {
                    var_r31 = 0xA92;
                    this->unk7EA = 0;
                } else {
                    var_r31 = 0xA90;
                    this->unk7EA = 1;
                }
                break;
            case 2:                                 /* switch 3 */
                temp_r0_4 = this->unk7EA;
                if ((s8) temp_r0_4 == 2) {
                    var_r31 = 0xA95;
                    this->unk7EA = 1;
                } else if ((s8) temp_r0_4 == 1) {
                    var_r31 = 0xA96;
                    this->unk7EA = 0;
                } else {
                    var_r31 = 0xA93;
                    this->unk7EA = 2;
                }
                break;
            case 3:                                 /* switch 3 */
                temp_r0_5 = this->unk7EA;
                switch ((s8) temp_r0_5) {           /* switch 4; irregular */
                case 3:                             /* switch 4 */
                    var_r31 = 0xA99;
                    this->unk7EA = 2;
                    break;
                case 2:                             /* switch 4 */
                    var_r31 = 0xA9A;
                    this->unk7EA = 1;
                    break;
                case 1:                             /* switch 4 */
                    var_r31 = 0xA9B;
                    this->unk7EA = 0;
                    break;
                default:                            /* switch 4 */
                    var_r31 = 0xA97;
                    this->unk7EA = 3;
                    break;
                }
                break;
            }
            this->unk7EC = 1;
        }
        return var_r31;
    }
    temp_r30 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r30, 0x2A80U) != 0) {
        if (isEventBit__11dSv_event_cFUs(temp_r30, 0x2C40U) == 0) {
            onEventBit__11dSv_event_cFUs(temp_r30, 0x2C40U);
            return 0xAAD;
        }
        goto block_57;
    }
    if (isEventBit__11dSv_event_cFUs(temp_r30, 0x2C80U) == 0) {
        onEventBit__11dSv_event_cFUs(temp_r30, 0x2C80U);
        return 0xAAE;
    }
block_57:
    var_r3 = 0xA8D;
    if (isEventBit__11dSv_event_cFUs(temp_r30, 0x304U) != 0) {
        var_r3 = 0xA8F;
    }
    return var_r3;
}

/* daNpc_Ob1_c::getMsg_OB1_1 (void) */
s32 getMsg_OB1_1__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    dSv_event_c *temp_r31;
    s32 temp_r0;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x2C20U) == 0) {
        return 0xAA8;
    }
    temp_r0 = 0 - getEventReg__11dSv_event_cFUs(temp_r31, 0xB6FFU);
    return ((temp_r0 - temp_r0) - !M2C_CARRY) + 0xAAC;
}

/* daNpc_Ob1_c::getMsg_OB1_2 (void) */
void getMsg_OB1_2__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    getMsg_OB1_1__11daNpc_Ob1_cFv(this);
}

/* daNpc_Ob1_c::getMsg (void) */
s32 getMsg__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 var_r4;
    u8 temp_r0;

    var_r4 = 0;
    temp_r0 = this->unk809;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r4 = getMsg_OB1_0__11daNpc_Ob1_cFv(this);
        break;
    case 1:
        var_r4 = getMsg_OB1_1__11daNpc_Ob1_cFv(this);
        break;
    case 2:
        var_r4 = getMsg_OB1_2__11daNpc_Ob1_cFv(this);
        break;
    }
    return var_r4;
}

/* daNpc_Ob1_c::chkAttention (void) */
u8 chkAttention__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    dAttention_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x5808;
    if (LockonTruth__12dAttention_cFv(temp_r31) != 0) {
        return this == LockonTarget__12dAttention_cFl(temp_r31, 0);
    }
    return this == ActionTarget__12dAttention_cFl(temp_r31, 0);
}

/* daNpc_Ob1_c::setAttention (bool) */
void setAttention__11daNpc_Ob1_cFb(daNpc_Ob1_c *this, u8 arg0) {
    this->unk274 = this->unk1F8;
    this->unk278 = this->unk1FC + l_HIO.unk20;
    this->unk27C = this->unk200;
    if (((s32) this->unk7F4 != 0) || (arg0 != 0)) {
        this->unk260.unk0 = this->unk790;
        this->unk264 = this->unk794;
        this->unk268 = this->unk798;
    }
}

/* daNpc_Ob1_c::searchByID (int unsigned) */
fopAc_ac_c *searchByID__11daNpc_Ob1_cFUi(daNpc_Ob1_c *this, u32 arg0) {
    fopAc_ac_c *sp8;

    sp8 = NULL;
    fopAcM_SearchByID__FUiPP10fopAc_ac_c(arg0, &sp8);
    return sp8;
}

/* daNpc_Ob1_c::partner_srch (void) */
void partner_srch__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {

}

/* daNpc_Ob1_c::bitCount (char unsigned) */
void bitCount__11daNpc_Ob1_cFUc(daNpc_Ob1_c *this, u8 arg0) {
    s32 var_ctr;
    u8 var_r4;

    var_r4 = arg0;
    var_ctr = 8;
    do {
        if (var_r4 & 1) {

        }
        var_r4 = (u8) ((s32) var_r4 >> 1);
        var_ctr -= 1;
    } while (var_ctr != 0);
}

/* daNpc_Ob1_c::set_pigCnt (void) */
void set_pigCnt__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    dSv_event_c *temp_r29;
    s32 var_ctr;
    s32 var_r3;

    temp_r29 = &g_dComIfG_gameInfo + 0x624;
    this->unk7E7 = bitCount__11daNpc_Ob1_cFUc(this, getEventReg__11dSv_event_cFUs(temp_r29, 0xB6FFU));
    @3569.unkE4 = 0;
    @3569.unkE8 = 0U;
    var_r3 = 0;
    var_ctr = 0x14;
    do {
        *(&@3569 + 0x94 + var_r3) = 0;
        var_r3 += 4;
        var_ctr -= 1;
    } while (var_ctr != 0);
    fpcEx_Search__FPFPvPv_PvPv(searchActor_Kb__FPvPv, this);
    this->unk7E8 = bitCount__11daNpc_Ob1_cFUc(this, @3569.unkE8);
    this->unk7E9 = bitCount__11daNpc_Ob1_cFUc(this, (u8) (@3569.unkE8 & ~getEventReg__11dSv_event_cFUs(temp_r29, 0xB6FFU)));
    this->unk7EA = 0;
}

/* daNpc_Ob1_c::ob_setPthPos (void) */
void ob_setPthPos__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    f32 sp28;
    f32 sp24;
    f32 sp20;
    dNpc_PathRun_c sp14;
    dNpc_PathRun_c sp8;

    if ((void *) this->unk700.unk0 != NULL) {
        getPoint__14dNpc_PathRun_cFUc(&sp14, (u8) &this->unk700);
        this->unk1F8 = (bitwise f32) sp14;
        this->unk1FC = sp18;
        this->unk200 = sp1C;
        nextIdxAuto__14dNpc_PathRun_cFv(&this->unk700);
        getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk700);
        sp20 = (bitwise f32) sp8;
        sp24 = spC;
        sp28 = sp10;
        this->unk206 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp20);
    }
}

/* daNpc_Ob1_c::get_attPos (void) */
void get_attPos__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    u8 temp_r0;
    u8 var_r3;
    void *temp_r30;
    void *temp_r3;
    void *temp_r3_2;

    temp_r30 = M2C_ERROR(/* Read from unset register $r4 */);
    temp_r3 = nextPath__14dNpc_PathRun_cFSc(temp_r30 + 0x700, (s8) M2C_ERROR(/* Read from unset register $r4 */)->unk20A);
    if (temp_r3 != NULL) {
        temp_r0 = temp_r30->unk705;
        var_r3 = temp_r0;
        if (temp_r0 == 0) {
            var_r3 = maxPoint__14dNpc_PathRun_cFv(temp_r30 + 0x700);
        }
        temp_r3_2 = temp_r3->unk8 + (((var_r3 - 1) * 0x10) & 0xFF0);
        sp8 = temp_r3_2->unk4;
        spC = temp_r3_2->unk8;
        sp10 = temp_r3_2->unkC;
    }
    this->unk0 = sp8;
    this->unk4 = spC;
    this->unk8 = sp10;
}

/* daNpc_Ob1_c::ob_movPass (void) */
s32 ob_movPass__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s32 var_r30;
    void *temp_r3;

    var_r30 = 0;
    temp_r3 = this->unk700.unk0;
    if ((temp_r3 != NULL) && (temp_r3->unk5 & 1)) {
        sp24 = this->unk1F8;
        sp28 = this->unk1FC;
        sp2C = this->unk200;
        if (chkPointPass__14dNpc_PathRun_cF4cXyzb(&this->unk700, (cXyz) &sp24, (u8) (this->unk706 != 0)) != 0) {
            nextIdxAuto__14dNpc_PathRun_cFv(&this->unk700);
            var_r30 = 1;
        }
        return var_r30;
    }
    __mi__4cXyzCFRC3Vec(&sp18, &this->unk7A8);
    spC = (bitwise f32) sp18;
    sp10 = @4239.unk8;
    sp14 = sp20;
    var_f1 = PSVECSquareMag(&spC);
    if (var_f1 > @4239.unk8) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = @4239.unk50 * temp_f0 * (@4239.unk58 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4239.unk50 * temp_f0_2 * (@4239.unk58 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (@4239.unk50 * temp_f0_3 * (@4239.unk58 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
    if (var_f1 == this->unk7C0) {
        var_r30 = 1;
        if (((void *) this->unk700.unk0 != NULL) && (nextIdxAuto__14dNpc_PathRun_cFv(&this->unk700) == 0)) {
            var_r30 = 2;
        }
    }
    return var_r30;
}

/* daNpc_Ob1_c::ob_clcMovSpd (void) */
void ob_clcMovSpd__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    cXyz sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    __mi__4cXyzCFRC3Vec(&sp14, &this->unk7A8);
    sp8 = (bitwise f32) sp14;
    spC = 0.0f;
    sp10 = sp1C;
    PSVECSquareMag(&sp8);
    cLib_chaseAngleS__FPsss(&this->unk206, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &this->unk7A8), l_HIO.unk28);
    cLib_chaseF__FPfff(&this->unk254, this->unk7B8, this->unk7BC);
}

/* daNpc_Ob1_c::ob_nMove (void) */
void ob_nMove__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    f32 var_f31;
    s32 temp_r3;

    var_f31 = saved_reg_f31;
    if ((s32) this->unk7FC != 1) {

    } else {
        ob_clcMovSpd__11daNpc_Ob1_cFv(this);
        if ((s32) this->unk7FC == 1) {
            var_f31 = this->unk254 * l_HIO.unk2C;
        }
        if (var_f31 < 0.5f) {
            var_f31 = 0.5f;
        }
        this->mpBtpRes->unk64 = var_f31;
        temp_r3 = ob_movPass__11daNpc_Ob1_cFv(this);
        switch (temp_r3) {                          /* irregular */
        case 1:
            this->unk7F2 = 1;
            break;
        case 2:
            this->unk7F2 = 1;
            this->unk7FC = 0;
            break;
        }
    }
    if ((u8) this->unk7F2 != 0) {
        this->unk7F0 = 1;
    }
}

/* daNpc_Ob1_c::charDecide (int) */
u8 charDecide__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    this->unk808 = 0;
    this->unk809 = -1U;
    switch (arg0) {                                 /* irregular */
    case 1:
        this->unk809 = 1;
        break;
    case 2:
        this->unk809 = 2;
        break;
    default:
        this->unk809 = 0;
        break;
    }
    return 1U;
}

/* daNpc_Ob1_c::event_actionInit (int) */
void event_actionInit__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    s32 *temp_r3;

    temp_r3 = getMySubstanceP__16dEvent_manager_cFiPCci(&g_dComIfG_gameInfo + 0x52CC, arg0, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0x3B, 3);
    if (temp_r3 != NULL) {
        this->unk7FF = (u8) *temp_r3;
    }
}

/* daNpc_Ob1_c::event_action (void) */
u8 event_action__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    return 1U;
}

/* daNpc_Ob1_c::privateCut (int) */
void privateCut__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0, ? arg_sp0) {
    dEvent_manager_c *temp_r31;
    u8 var_r3;

    if (arg0 != -1) {
        temp_r31 = &g_dComIfG_gameInfo + 0x52CC;
        this->unk7FE = getMyActIdx__16dEvent_manager_cFiPCPCciii(temp_r31, arg0, &a_cut_tbl$5037, 1, 1, 0);
        if ((s8) this->unk7FE == -1) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
            return;
        }
        if (getIsAddvance__16dEvent_manager_cFi(temp_r31, arg0) != 0) {
            if ((s32) this->unk7FE != 0) {

            } else {
                event_actionInit__11daNpc_Ob1_cFi(this, arg0);
            }
        }
        if ((s32) this->unk7FE != 0) {
            var_r3 = 1;
        } else {
            var_r3 = event_action__11daNpc_Ob1_cFv(this);
        }
        if (var_r3 != 0) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
        }
    }
}

/* daNpc_Ob1_c::endEvent (void) */
void endEvent__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
    this->mAnmAtr = 0xFF;
}

/* daNpc_Ob1_c::isEventEntry (void) */
void isEventEntry__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(&g_dComIfG_gameInfo + 0x52CC, this->unk708.unk0, NULL, 0);
}

/* daNpc_Ob1_c::event_proc (int) */
void event_proc__11daNpc_Ob1_cFi(daNpc_Ob1_c *this, s32 arg0) {
    if (endCheck__16dEvent_manager_cFs(&g_dComIfG_gameInfo + 0x52CC, (this + (this->mEventIdx * 2))->mEventIdTable) != 0) {
        this->mOrderType = 1;
        endEvent__11daNpc_Ob1_cFv(this);
        return;
    }
    if (cutProc__15dNpc_EventCut_cFv(&this->unk708) == 0) {
        privateCut__11daNpc_Ob1_cFi(this, arg0);
    }
    lookBack__11daNpc_Ob1_cFv(this);
}

s32 set_action__11daNpc_Ob1_cFM11daNpc_Ob1_cFPCvPvPv_iPv(daNpc_Ob1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0) {
    if (__ptmf_cmpr(arg0 + 0x6F4) != 0) {
        if (__ptmf_test(arg0 + 0x6F4) != 0) {
            arg0->unk80A = 0;
            __ptmf_scall(arg0, arg2);
        }
        arg0->unk6F4 = (s32) arg1->unk0;
        arg0->unk6F8 = arg1->unk4;
        arg0->unk6FC = arg1->unk8;
        arg0->unk80A = 0;
        __ptmf_scall(arg0, arg2);
    }
    return 1;
}

/* daNpc_Ob1_c::clrSpd (void) */
void clrSpd__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    this->unk7B8 = 0.0f;
    this->unk7BC = 0.0f;
    this->unk254 = 0.0f;
    this->unk258 = -4.5f;
}

/* daNpc_Ob1_c::setStt (char signed) */
void setStt__11daNpc_Ob1_cFSc(daNpc_Ob1_c *this, s8 arg0, ? arg_sp0) {
    daNpc_Ob1_c sp14;
    dNpc_PathRun_c sp8;
    u8 temp_r0;
    u8 temp_r0_2;
    u8 temp_r30;

    searchByID__11daNpc_Ob1_cFUi(this, this->unk774);
    temp_r30 = this->mSttNum;
    this->mSttTimer = 0;
    this->mSttNum = (u8) arg0;
    temp_r0 = this->mSttNum;
    if ((u32) (s8) temp_r0 <= 6U) {
        switch ((s8) temp_r0) {                     /* switch 1 */
        case 4:                                     /* switch 1 */
            this->unk7DA = cLib_getRndValue<i>__Fii(0x5A, 0xB4);
            /* fallthrough */
        case 1:                                     /* switch 1 */
        case 5:                                     /* switch 1 */
            if ((s8) temp_r30 != 2) {
                temp_r0_2 = this->mSttNum;
                switch ((s8) temp_r0_2) {           /* switch 2; irregular */
                case 1:                             /* switch 2 */
                    this->unk807 = 3;
                    this->unk7E0 = this->unk786;
                    break;
                case 4:                             /* switch 2 */
                    this->unk807 = 2;
                    get_attPos__11daNpc_Ob1_cFv(&sp14);
                    this->unk79C = (bitwise f32) sp14;
                    this->unk7A0 = sp18;
                    this->unk7A4 = sp1C;
                    break;
                case 5:                             /* switch 2 */
                    this->unk807 = 1;
                    break;
                }
                this->unk29A = 1;
                this->unk7FA = 0;
            }
            this->mOrderType = 0;
            this->unk7FC = 0;
            clrSpd__11daNpc_Ob1_cFv(this);
            goto block_20;
        case 2:                                     /* switch 1 */
            this->unk807 = 1;
            this->unk29A = 1;
            this->unk7FA = 0;
            this->mOrderType = 0;
            this->unk7FC = 0;
            clrSpd__11daNpc_Ob1_cFv(this);
            this->mAnmAtr = 0xFF;
            if ((s8) temp_r30 != 5) {
                this->unk806 = temp_r30;
            }
            set_pigCnt__11daNpc_Ob1_cFv(this);
            goto block_20;
        case 3:                                     /* switch 1 */
            getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk700);
            this->unk7A8.unk0 = (bitwise f32) sp8;
            this->unk7AC = spC;
            this->unk7B0 = sp10;
            this->unk807 = 0;
            this->unk7FA = 1;
            this->mOrderType = 0;
            this->unk7FC = 1;
            this->unk7F2 = 0;
            this->unk258 = -4.5f;
            this->unk7B8 = l_HIO.unk30;
            this->unk7BC = l_HIO.unk34;
            this->unk7C0 = l_HIO.unk38;
            goto block_20;
        case 6:                                     /* switch 1 */
            this->mAnmAtr = 0xFF;
            this->unk806 = temp_r30;
            return;
        }
    } else {
    case 0:                                         /* switch 1 */
block_20:
        setAnm__11daNpc_Ob1_cFv(this);
    }
}

/* daNpc_Ob1_c::wait_1 (void) */
s32 wait_1__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    if ((u8) this->unk7F9 != 0) {
        if (chk_talk__11daNpc_Ob1_cFv(this) != 0) {
            setStt__11daNpc_Ob1_cFSc(this, 2);
        }
        return 1;
    }
    if ((u8) this->unk6BC == 1) {
        this->unk6BC = 2;
        setStt__11daNpc_Ob1_cFSc(this, 6);
        this->unk807 = 3;
        this->unk7E0 = this->unk786;
        return 1;
    }
    this->mOrderType = 2;
    if ((u8) this->unk7F8 != 0) {
        this->mSttTimer = 0x3C;
    }
    if (cLib_calcTimer<s>__FPs(&this->mSttTimer) != 0) {
        this->unk807 = 1;
    } else {
        this->unk807 = 3;
        this->unk7E0 = this->unk786;
        this->unk29A = 1;
    }
    return 1;
}

/* daNpc_Ob1_c::wait_2 (void) */
s32 wait_2__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    daNpc_Ob1_c sp8;

    if ((u8) this->unk7F9 != 0) {
        if (chk_talk__11daNpc_Ob1_cFv(this) != 0) {
            setStt__11daNpc_Ob1_cFSc(this, 2);
        }
        return 1;
    }
    this->mOrderType = 2;
    if ((u8) this->unk7F8 != 0) {
        this->mSttTimer = 0x3C;
    }
    if (cLib_calcTimer<s>__FPs(&this->mSttTimer) != 0) {
        this->unk807 = 1;
        goto block_11;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk7DA) == 0) {
        setStt__11daNpc_Ob1_cFSc(this, 3);
        return 1;
    }
    this->unk807 = 2;
    get_attPos__11daNpc_Ob1_cFv(&sp8);
    this->unk79C = (bitwise f32) sp8;
    this->unk7A0 = spC;
    this->unk7A4 = sp10;
    this->unk29A = 1;
block_11:
    return 1;
}

/* daNpc_Ob1_c::wait_3 (void) */
s32 wait_3__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    if ((u8) this->unk7F9 != 0) {
        if (chk_talk__11daNpc_Ob1_cFv(this) != 0) {
            setStt__11daNpc_Ob1_cFSc(this, 2);
        }
        return 1;
    }
    if ((s8) this->unk7E4 != 0) {
        this->unk7E5 += 1;
        if ((s8) this->unk7E5 > 3) {
            setStt__11daNpc_Ob1_cFSc(this, (s8) this->unk806);
            return 1;
        }
    }
    this->mOrderType = 2;
    this->unk807 = 1;
    this->unk29A = 1;
    return 1;
}

/* daNpc_Ob1_c::walk_1 (void) */
s32 walk_1__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    dNpc_PathRun_c sp8;

    if ((u8) this->unk7F9 != 0) {
        setAnm_NUM__11daNpc_Ob1_cFii(this, 0, 1);
        this->unk254 = 0.0f;
        if (chk_talk__11daNpc_Ob1_cFv(this) != 0) {
            setStt__11daNpc_Ob1_cFSc(this, 2);
        }
        return 1;
    }
    this->mOrderType = 2;
    if ((u8) this->unk7F8 != 0) {
        this->mSttTimer = 0x14;
    }
    if (cLib_calcTimer<s>__FPs(&this->mSttTimer) != 0) {
        this->unk807 = 1;
    } else {
        this->unk807 = 0;
    }
    if ((u8) this->unk7F2 != 0) {
        if ((u8) this->unk705 == 0) {
            nextIdxAuto__14dNpc_PathRun_cFv(&this->unk700);
        }
        setStt__11daNpc_Ob1_cFSc(this, 4);
        return 1;
    }
    this->unk7F2 = 0;
    getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->unk700);
    this->unk7A8.unk0 = (bitwise f32) sp8;
    this->unk7AC = spC;
    this->unk7B0 = sp10;
    return 1;
}

/* daNpc_Ob1_c::talk_1 (void) */
u8 talk_1__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    dSv_event_c *temp_r29;
    u16 temp_r0_2;
    u32 temp_r0_4;
    u8 temp_r0;
    u8 temp_r0_3;
    u8 temp_r31;
    void *temp_r3;
    void *temp_r3_2;

    temp_r31 = chk_partsNotMove__11daNpc_Ob1_cFv(this);
    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 >= 3)) {
        return 1U;
    }
    temp_r3 = this->unk6B0;
    if (temp_r3 != NULL) {
        this->unk7E2 = temp_r3->unkF8;
    } else {
        this->unk7E2 = 0;
    }
    if (((u8) this->unk7ED != 0) && ((u32) (this->unk6AC + 0x10000) == -1U)) {
        this->unk6A4 = this->unk6C0->unkC(this);
        this->unk6AC = fopMsgM_messageSet__FUlP10fopAc_ac_c(this->unk6A4, g_dComIfG_gameInfo.unk5C28);
        this->unk6B0 = NULL;
    } else {
        this->unk7CC = -1;
        talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1);
    }
    temp_r3_2 = this->unk6B0;
    if ((temp_r3_2 != NULL) && (temp_r0_2 = temp_r3_2->unkF8, (((s32) temp_r0_2 == 6) == 0))) {
        switch ((s32) temp_r0_2) {                  /* switch 1; irregular */
        case 19:                                    /* switch 1 */
            if ((s8) this->unk7EA != 0) {
                this->mOrderType = 3;
                setAnm_NUM__11daNpc_Ob1_cFii(this, 0, 1);
                endEvent__11daNpc_Ob1_cFv(this);
                return temp_r31;
            }
            temp_r29 = &g_dComIfG_gameInfo + 0x624;
            temp_r0_3 = l_check_flg | getEventReg__11dSv_event_cFUs(temp_r29, 0xB6FFU);
            l_check_flg = temp_r0_3;
            setEventReg__11dSv_event_cFUsUc(temp_r29, 0xB6FFU, temp_r0_3);
            temp_r0_4 = this->unk6A4;
            switch ((s32) temp_r0_4) {              /* switch 2; irregular */
            case 0xA8E:                             /* switch 2 */
                onEventBit__11dSv_event_cFUs(temp_r29, 0x304U);
                break;
            case 0xAA7:                             /* switch 2 */
                onEventBit__11dSv_event_cFUs(temp_r29, 0x308U);
                break;
            default:                                /* switch 2 */
                onEventBit__11dSv_event_cFUs(temp_r29, 0x2C20U);
                break;
            }
            if ((u8) this->unk7EC != 0) {
                onEventBit__11dSv_event_cFUs(temp_r29, 0x302U);
            }
            this->unk7EB = 0xFF;
            this->unk7F9 = 0;
            if ((s32) this->unk809 != 0) {
                setStt__11daNpc_Ob1_cFSc(this, (s8) this->unk806);
                this->mSttTimer = 0x3C;
            } else {
                setStt__11daNpc_Ob1_cFSc(this, 5);
            }
            endEvent__11daNpc_Ob1_cFv(this);
            goto block_36;
        }
    } else {
    case 2:                                         /* switch 1 */
block_36:
        return temp_r31;
    }
}

/* daNpc_Ob1_c::manzai (void) */
s32 manzai__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    fopAc_ac_c *temp_r4;
    u8 temp_r0;
    void *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x5C20;
    temp_r0 = this->unk6BC;
    switch ((s32) temp_r0) {                        /* irregular */
    case 2:
        temp_r4 = searchByID__11daNpc_Ob1_cFUi(this, this->unk6B4);
        if (this != (u32) (temp_r30 + (temp_r30->unk4 * 4))->unk4) {
            if ((u8) this->mAnmAtr != 0xFF) {
                this->unk807 = 3;
                this->unk7E0 = this->unk786;
                this->unk29A = 1;
                this->mSttNum = this->unk806;
                setAnm__11daNpc_Ob1_cFv(this);
                this->mSttNum = 6;
                this->mAnmAtr = 0xFF;
            }
        } else {
            this->unk7CC = temp_r4->unk6A4;
            this->unk6C0->unk10(this, temp_r4->unk6B8);
        }
        break;
    case 3:
        this->unk1C4 &= 0xFFFFBFFF;
        setStt__11daNpc_Ob1_cFSc(this, (s8) this->unk806);
        this->unk6BC = 0;
        break;
    }
    return 1;
}

/* daNpc_Ob1_c::wait_action1 (void *) */
s32 wait_action1__11daNpc_Ob1_cFPv(daNpc_Ob1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk80A;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        ob_setPthPos__11daNpc_Ob1_cFv(this);
        setStt__11daNpc_Ob1_cFSc(this, 3);
        this->unk80A += 1;
        break;
    default:                                        /* switch 1 */
        this->unk7F8 = chkAttention__11daNpc_Ob1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 4:                                     /* switch 2 */
            this->unk7F4 = wait_2__11daNpc_Ob1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk7F4 = wait_3__11daNpc_Ob1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk7F4 = walk_1__11daNpc_Ob1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk7F4 = talk_1__11daNpc_Ob1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ob1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ob1_c::wait_action2 (void *) */
s32 wait_action2__11daNpc_Ob1_cFPv(daNpc_Ob1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk80A;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ob1_cFSc(this, 1);
        this->unk80A += 1;
        break;
    default:                                        /* switch 1 */
        this->unk7F8 = chkAttention__11daNpc_Ob1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk7F4 = wait_1__11daNpc_Ob1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk7F4 = talk_1__11daNpc_Ob1_cFv(this);
            break;
        case 6:                                     /* switch 2 */
            this->unk7F4 = manzai__11daNpc_Ob1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ob1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ob1_c::demo (void) */
u8 demo__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 sp8;
    JUTAssertion *temp_r3_2;
    dDemo_actor_c *temp_r3;
    s16 temp_r5;

    if ((u8) this->unk1C0 == 0) {
        if ((u8) this->unk7FB != 0) {
            this->unk7FB = 0;
        }
    } else {
        this->unk7FB = 1;
        temp_r3 = getActor__14dDemo_object_cFUc(g_dComIfG_gameInfo.unk5AC8 + 0x20, this->unk1C0);
        if ((JUTAssertion *) this->unk6D8 != NULL) {
            this->mBtpFrame += 1;
            temp_r5 = this->unk6D8->unk6;
            if ((s32) this->mBtpFrame >= temp_r5) {
                this->mBtpFrame = (u8) temp_r5;
            }
        }
        temp_r3_2 = getP_BtpData__13dDemo_actor_cFPCc(temp_r3, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE);
        if (temp_r3_2 != NULL) {
            this->unk6D8 = temp_r3_2;
            sp8 = 0;
            if (init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6DC, this->unk6D4->unk4, (J3DAnmTexPattern *) this->unk6D8, 1, 2, 1.0f, 0, -1, 1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0) {
                this->mBtpNum = 2;
                this->mBtpFrame = 0;
            }
        }
        dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc((fopAc_ac_c *) this, 0x6AU, this->mpBtpRes, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE, 0, NULL, 0U, 0);
    }
    return this->unk7FB;
}

/* daNpc_Ob1_c::shadowDraw (void) */
void shadowDraw__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f3;
    u32 temp_r4;

    temp_f3 = this->unk1FC;
    sp8 = this->unk1F8;
    spC = @4239.unk64 + temp_f3;
    sp10 = this->unk200;
    this->unk6D0 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unk6D0, 1, this->mpBtpRes->unk50, (cXyz *) &sp8, @4239.unk68, @4239.unk6C, temp_f3, this->unk3C8, &this->unk41C, &this->unk10C, 0, @4239.unk38, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    temp_r4 = this->unk6D0;
    if (temp_r4 != 0) {
        addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, temp_r4, (J3DModel *) this->unk6D4);
    }
}

/* daNpc_Ob1_c::_draw (void) */
s32 _draw__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    J3DModel *temp_r30;
    J3DModelData *temp_r31;
    void **temp_r29;

    temp_r29 = this->unk6D4;
    temp_r31 = temp_r29->unk4;
    temp_r30 = this->mpBtpRes->unk50;
    if (((u8) this->unk7EE != 0) || ((u8) this->unk7F1 != 0)) {
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &this->unk1F8, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r30, &this->unk10C);
    entryDL__14mDoExt_McaMorfFv(this->mpBtpRes);
    entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->unk6DC, temp_r31, (s16) this->mBtpFrame);
    mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) temp_r29);
    removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r31->unk58, this->unk6E4);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r29, &this->unk10C);
    shadowDraw__11daNpc_Ob1_cFv(this);
    dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x4FU, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
    return 1;
}

/* daNpc_Ob1_c::_execute (void) */
s32 _execute__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    s32 spC;
    s32 sp8;
    s32 var_r4;

    if ((u8) this->unk7F3 == 0) {
        this->unk778 = this->unk1F8;
        this->unk77C = this->unk1FC;
        this->unk780 = this->unk200;
        this->unk784 = this->unk204;
        this->unk786 = this->unk206;
        this->unk788 = this->unk208;
        this->unk7F3 = 1;
    }
    sp8 = (s32) l_HIO.unk12;
    spC = (s32) l_HIO.unk1C;
    setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, l_HIO.unk14, l_HIO.unk16, l_HIO.unk18, l_HIO.unk1A, l_HIO.unkC, l_HIO.unkE, l_HIO.unk10, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    if (((u8) this->unk7EE != 0) && ((u8) this->unk1C0 == 0)) {
        return 1;
    }
    this->unk7F0 = 0;
    this->unk7EE = 0;
    partner_srch__11daNpc_Ob1_cFv(this);
    checkOrder__11daNpc_Ob1_cFv(this);
    if (demo__11daNpc_Ob1_cFv(this) == 0) {
        var_r4 = -1;
        if (((u8) g_dComIfG_gameInfo.unk529A != 0) && ((u16) this->unkF8 != 1)) {
            var_r4 = isEventEntry__11daNpc_Ob1_cFv(this);
        }
        if (var_r4 >= 0) {
            event_proc__11daNpc_Ob1_cFi(this, var_r4);
        } else {
            __ptmf_scall(this, 0);
        }
        if ((u8) this->unk7F0 == 0) {
            ob_nMove__11daNpc_Ob1_cFv(this);
            fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, (cXyz *) &this->unk538);
        }
        if ((u8) this->unk7EF == 0) {
            this->unk78A = this->unk204;
            this->unk78C = this->unk206;
            this->unk78E = this->unk208;
            this->unk20C = this->unk78A;
            this->unk20E = this->unk78C;
            this->unk210 = this->unk78E;
        }
    }
    eventOrder__11daNpc_Ob1_cFv(this);
    setMtx__11daNpc_Ob1_cFb(this, 0U);
    if ((u8) this->unk7FB == 0) {
        setCollision__12fopNpc_npc_cFff((fopNpc_npc_c *) this, 80.0f, 160.0f);
    }
    return 1;
}

/* daNpc_Ob1_c::_delete (void) */
s32 _delete__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    mDoExt_McaMorf *temp_r3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk6C4, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE);
    if ((u32) this->unkF0 != 0U) {
        temp_r3 = this->mpBtpRes;
        if (temp_r3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3);
        }
    }
    return 1;
}

/* CheckCreateHeap (fopAc_ac_c *) */
void CheckCreateHeap__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    CreateHeap__11daNpc_Ob1_cFv((daNpc_Ob1_c *) arg0);
}

/* daNpc_Ob1_c::_create (void) */
s32 _create__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    s32 temp_r3;
    s32 var_r3;

    if (!(this->unk1C8 & 8)) {
        if (this != NULL) {
            __ct__11daNpc_Ob1_cFv(this);
        }
        this->unk1C8 |= 8;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk6C4, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE);
    if (temp_r3 != 4) {
        return temp_r3;
    }
    if (charDecide__11daNpc_Ob1_cFi(this, (s32) (u8) this->unkB0) == 0) {
        return 5;
    }
    if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) CheckCreateHeap__FP10fopAc_ac_c, a_size_tbl$5467[(s8) this->unk808]) == 0) {
        return 5;
    }
    this->unk22C = &this->mpBtpRes->unk50->unk24;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, @4239.unk84, @4239.unk4, @4239.unk84, @4239.unk40, @4239.unk88, @4239.unk40);
    var_r3 = 5;
    if (createInit__11daNpc_Ob1_cFv(this) != 0) {
        var_r3 = temp_r3;
    }
    return var_r3;
}

/* daNpc_Ob1_c::create_Anm (void) */
JUTAssertion *create_Anm__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r8;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *var_r29;
    s8 *temp_r3_2;
    s8 *temp_r3_5;
    s8 *temp_r3_7;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    u8 temp_r3_4;
    u8 temp_r3_6;
    void *temp_r28;

    temp_r28 = &g_dComIfG_gameInfo + 0x1BFC0;
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE), (s8 *)5, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0x11), (s8 *)0x977, (s32) (temp_r4 + 0x48), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        OSPanic(temp_r3_2 + 0x11, 0x977, temp_r3_2 + 0x36);
    }
    var_r29 = __nw__FUl(0xB4U);
    if (var_r29 != NULL) {
        temp_r8 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE), (s8 *)3, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x11020022;
        var_r29 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(var_r29, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r3, NULL, NULL, (s32) temp_r8, 1.0f, 2, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    this->mpBtpRes = var_r29;
    temp_r3_3 = this->mpBtpRes;
    if (temp_r3_3 == NULL) {
        return NULL;
    }
    if ((J3DModel *) temp_r3_3->unk50 == NULL) {
        this->mpBtpRes = NULL;
        return NULL;
    }
    temp_r3_4 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0x57);
    this->unk6CC = temp_r3_4;
    if ((s8) this->unk6CC < 0) {
        temp_r4_2 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_4), (u32) (temp_r4_2 + 0x11), (s8 *)0x98B, (s32) (temp_r4_2 + 0x5C), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_5 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        OSPanic(temp_r3_5 + 0x11, 0x98B, temp_r3_5 + 0x36);
    }
    temp_r3_6 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0x6F);
    this->unk6CD = temp_r3_6;
    if ((s8) this->unk6CD < 0) {
        temp_r4_3 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_6), (u32) (temp_r4_3 + 0x11), (s8 *)0x98E, (s32) (temp_r4_3 + 0x78), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_7 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        OSPanic(temp_r3_7 + 0x11, 0x98E, temp_r3_7 + 0x36);
    }
    return temp_r3;
}

/* daNpc_Ob1_c::create_hed_Mdl (void) */
JUTAssertion *create_hed_Mdl__11daNpc_Ob1_cFv(daNpc_Ob1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo" + 0xE), (s8 *) (u16) a_hed_mdl_resID_tbl$5728[(s8) this->unk808], (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0x11), (s8 *)0x9A1, (s32) (temp_r4 + 0x48), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Get_Rupee\0Ob1\0Ob\0d_a_npc_ob1.cpp\0m_hed_tex_pttrn != 0\0Halt\0ActNo";
        OSPanic(temp_r3_2 + 0x11, 0x9A1, temp_r3_2 + 0x36);
    }
    this->unk6D4 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x11020022U);
    return temp_r3;
}

/* daNpc_Ob1_c::CreateHeap (void) */
s32 CreateHeap__11daNpc_Ob1_cFv(daNpc_Ob1_c *this, ? arg_sp0) {
    u8 sp10;
    s32 sp8;
    JUTAssertion *temp_r3;
    u16 var_r5;

    temp_r3 = create_Anm__11daNpc_Ob1_cFv(this);
    if (temp_r3 == NULL) {
        return 0;
    }
    if (create_hed_Mdl__11daNpc_Ob1_cFv(this) == NULL) {
        this->mpBtpRes = NULL;
        return 0;
    }
    sp10 = @4239.unk8C;
    this->mBtpNum = @4239.unk8C;
    if (iniTexPttrnAnm__11daNpc_Ob1_cFb(this, 0) == 0) {
        this->mpBtpRes = NULL;
        return 0;
    }
    var_r5 = 0;
loop_11:
    if (var_r5 < (u16) temp_r3->unk28) {
        if (((s32) var_r5 == (s8) this->unk6CC) || ((s32) var_r5 == (s8) this->unk6CD)) {
            (*(this->mpBtpRes->unk50->unk4->unk2C + ((var_r5 * 4) & 0x3FFFC)))->unk8 = nodeCallBack_Ob1__FP7J3DNodei;
        }
        var_r5 += 1;
        goto loop_11;
    }
    this->mpBtpRes->unk50->unk14 = this;
    SetWall__12dBgS_AcchCirFff(&this->unk4F8, @4239.unk3C, @4239.unk7C);
    sp8 = 0;
    Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&this->unk334, (cXyz *) &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, &this->unk4F8, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    return 1;
}

/* daNpc_Ob1_Create (fopAc_ac_c *) */
void daNpc_Ob1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _create__11daNpc_Ob1_cFv((daNpc_Ob1_c *) arg0);
}

/* daNpc_Ob1_Delete (daNpc_Ob1_c *) */
void daNpc_Ob1_Delete__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0) {
    _delete__11daNpc_Ob1_cFv(arg0);
}

/* daNpc_Ob1_Execute (daNpc_Ob1_c *) */
void daNpc_Ob1_Execute__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0) {
    _execute__11daNpc_Ob1_cFv(arg0);
}

/* daNpc_Ob1_Draw (daNpc_Ob1_c *) */
void daNpc_Ob1_Draw__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0) {
    _draw__11daNpc_Ob1_cFv(arg0);
}

/* daNpc_Ob1_IsDelete (daNpc_Ob1_c *) */
s32 daNpc_Ob1_IsDelete__FP11daNpc_Ob1_c(daNpc_Ob1_c *arg0) {
    return 1;
}

/* d_a_npc_ob1_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_ob1_cpp(void) {
    __ct__15daNpc_Ob1_HIO_cFv(&l_HIO);
    __register_global_object(__dt__15daNpc_Ob1_HIO_cFv, @4156);
}