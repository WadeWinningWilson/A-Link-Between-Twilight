// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in naming-map-auto.md;
// remaining unkNNN are still inference-needed. Behavior identical to the full draft.
// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_ym1 (87 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_ym1\asm\d\actor\d_a_npc_ym1.m2c.s
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
    /* 0x14 */ daNpc_Ym1_c *unk14;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
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

typedef struct JAIZelBasic {
    /* 0x0 */ JAIZelBasic *unk0;                    /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0x4 */

typedef struct JUTAssertion {
    /* 0x00 */ char pad0[0x54];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
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

struct __vt__15daNpc_Ym1_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Ym1_HIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__20daNpc_Ym1_childHIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Ym1_childHIO_c *, s16);
};                                                  /* size = 0xC */

struct _struct_a_anm_prm_tbl$4530_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4537_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_a_anm_prm_tbl$4575_0x10 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0xE];                      /* maybe part of unk1[0xF]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x10 */

struct _struct_l_HIO_0x2C {
    /* 0x00 */ f32 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0x24];                     /* maybe part of unk4[0xA]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
};                                                  /* size = 0x2C */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct dNpc_EventCut_c {
    /* 0x0 */ s8 *unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_EventCut_c;                                  /* size >= 0x4 */

typedef struct dRes_control_c {
    /* 0x0 */ s8 unk0;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dRes_control_c;                                   /* size >= 0x1 */

typedef struct daNpc_Ym1_HIO_c {
    /* 0x0 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
    /* 0x4 */ s8 unk4;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x5 */ char pad5[3];                         /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x8 */ s32 unk8;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0xC */ ? unkC;                               /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0xC */ char padC[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Ym1_HIO_c;                                  /* size >= 0xD */

typedef struct daNpc_Ym1_c {
    /* 0x000 */ char pad0[8];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x008 */ s16 unk8;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x00A */ char padA[0xA6];                    /* maybe part of unk8[0x54]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA (receipt f_op_actor.h) */
    /* 0x0B0 */ s32 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
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
    /* 0x1C1 */ char pad1C1[7];                     /* maybe part of unk1C0[8]? */  /* = fopAc_ac_c::s8 argument @0x1C1 (receipt f_op_actor.h) */
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
    /* 0x26D */ u8 unk26D;                          /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x1 (receipt f_op_actor.h) */
    /* 0x26E */ char pad26E[1];  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x2 (receipt f_op_actor.h) */
    /* 0x26F */ u8 unk26F;                          /* inferred */  /* = fopAc_ac_c::actor_attention_types attention_info @0x26C +0x3 (receipt f_op_actor.h) */
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
    /* 0x29A */ s8 unk29A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29B */ char pad29B[0x29];                  /* maybe part of unk29A[0x2A]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ dNpc_EventCut_c unk2C4;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ char pad2C4[0x6C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x330 */ mDoExt_McaMorf *unk330;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
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
    /* 0x5BC */ char pad5BC[0xD0];                  /* maybe part of unk5B8[0x35]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x68C */ cM3dGCyl unk68C;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x68C */ char pad68C[0x18];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A4 */ s32 unk6A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A8 */ char pad6A8[8];                     /* maybe part of unk6A4[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B0 */ void *unk6B0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B4 */ char pad6B4[0x10];                  /* maybe part of unk6B0[5]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ request_of_phase_process_class unk6C4; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ char pad6C4[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CC */ u8 unk6CC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CD */ u8 unk6CD;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CE */ u8 unk6CE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CF */ u8 unk6CF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D0 */ void **unk6D0;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ dRes_control_c unk6D4;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ char pad6D4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ u32 unk6D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ void **unk6DC;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E0 */ mDoExt_btpAnm unk6E0;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E0 */ char pad6E0[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6E8 */ J3DAnmTexPattern *unk6E8;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6EC */ char pad6EC[8];                     /* maybe part of unk6E8[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F4 */ u8 unk6F4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F5 */ char pad6F5[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F6 */ s16 unk6F6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ ? unk6F8;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ char pad6F8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6FC */ s32 unk6FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x700 */ s32 unk700;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x704 */ dCcD_Cyl unk704;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x704 */ char pad704[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x748 */ dCcD_Stts *unk748;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x74C */ char pad74C[0xD0];                  /* maybe part of unk748[0x35]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x81C */ cM3dGCyl unk81C;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x81C */ char pad81C[0x18];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x834 */ f32 unk834;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x838 */ f32 unk838;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x83C */ f32 unk83C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x840 */ s16 unk840;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x842 */ s16 unk842;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x844 */ s16 unk844;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x846 */ s16 unk846;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x848 */ s16 unk848;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84A */ s16 unk84A;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84C */ cXyz unk84C;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84C */ char pad84C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x850 */ f32 unk850;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x854 */ f32 unk854;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x858 */ f32 unk858;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x85C */ f32 unk85C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x860 */ f32 unk860;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x864 */ f32 unk864;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x868 */ f32 unk868;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86C */ f32 unk86C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x870 */ f32 unk870;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x874 */ f32 unk874;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x878 */ f32 unk878;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x87C */ f32 unk87C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x880 */ s16 unk880;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x882 */ s16 unk882;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x884 */ s16 unk884;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x886 */ char pad886[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x888 */ s32 unk888;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x88C */ char pad88C[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x88E */ s16 unk88E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x890 */ s16 unk890;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x892 */ char pad892[4];                     /* maybe part of unk890[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x896 */ s16 unk896;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x898 */ u8 unk898;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x899 */ s8 unk899;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89A */ u8 unk89A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89B */ u8 unk89B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89C */ u8 unk89C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89D */ u8 unk89D;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89E */ u8 unk89E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89F */ u8 unk89F;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A0 */ u8 unk8A0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A1 */ u8 unk8A1;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A2 */ u8 unk8A2;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A3 */ u8 unk8A3;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A4 */ u8 unk8A4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A5 */ u8 unk8A5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A6 */ u8 unk8A6;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A7 */ u8 unk8A7;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A8 */ u8 mAnmAtr;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A9 */ u8 unk8A9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8AA */ s8 unk8AA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8AB */ u8 unk8AB;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8AC */ u8 mOrderType;                          /* inferred */  /* [NAMED-BY-RULE: eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md)] */
    /* 0x8AD */ u8 mSttNum;                          /* inferred */  /* [NAMED-BY-RULE: setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md)] */
    /* 0x8AE */ u8 mSttNumOld;                          /* inferred */  /* [NAMED-BY-RULE: stt old (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md)] */
    /* 0x8AF */ u8 unk8AF;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8B0 */ u8 unk8B0;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8B1 */ u8 unk8B1;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8B2 */ u8 unk8B2;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8B3 */ u8 unk8B3;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_Ym1_c;                                      /* size >= 0x8B4 */

typedef struct daNpc_Ym1_c::anm_prm_c {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ char pad1[3];                        /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Ym1_c::anm_prm_c;                           /* size >= 0x10 */

typedef struct daNpc_Ym1_childHIO_c {
    /* 0x0 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
} daNpc_Ym1_childHIO_c;                             /* size >= 0x4 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x58 */ J3DFrameCtrl unk58;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x58 */ char pad58[0x10];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ char pad0[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x0C */ f32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10 */ char pad10[0xC];                     /* maybe part of unkC[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x1C */ f32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ char pad20[0xC];                     /* maybe part of unk1C[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x2C */ f32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x30 */

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
? PSMTXMultVec(mDoMtx_stack_c *, f32 *, cXyz *);    /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetAreaChk__12dCcMassS_MngFP8cCcD_ObjUcPFP10fopAc_ac_cP4cXyzUl_v(dCcMassS_Mng *this, cCcD_Obj *arg0, u8 arg1, void (*arg2)(fopAc_ac_c *, cXyz *, u32)); /* extern */
? SetC__8cM3dGCylFRC4cXyz(cM3dGCyl *this, cXyz *arg0); /* extern */
? SetH__8cM3dGCylFf(cM3dGCyl *this, f32 arg0);      /* extern */
? SetR__8cM3dGCylFf(cM3dGCyl *this, f32 arg0);      /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__12dCcMassS_MngFP8cCcD_ObjUc(dCcMassS_Mng *this, cCcD_Obj *arg0, u8 arg1); /* extern */
? Set__4cCcSFP8cCcD_Obj(cCcS *this, cCcD_Obj *arg0); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
? __construct_array(? *, void *(*)(daNpc_Ym1_childHIO_c *), void *(*)(daNpc_Ym1_childHIO_c *, s16), ?, ?); /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
s32 __ptmf_cmpr(s32);                               /* extern */
? __ptmf_scall(daNpc_Ym1_c *, s32);                 /* extern */
s32 __ptmf_test(s32);                               /* extern */
? __register_global_object(void *(*)(daNpc_Ym1_HIO_c *, s16), void *(*)(cXyz *, s16), void *); /* extern */
? addReal__21dDlst_shadowControl_cFUlP8J3DModel(dDlst_shadowControl_c *this, u32 arg0, J3DModel *arg1); /* extern */
? cLib_addCalcAngleS__FPsssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4); /* extern */
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
u8 dNpc_chkAttn__FP10fopAc_ac_c4cXyzfffb(fopAc_ac_c *arg0, cXyz arg1, f32 arg2, f32 arg3, f32 arg4, s32 arg5); /* extern */
? dNpc_playerEyePos__Ff(f32 arg0);                  /* extern */
? dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(mDoExt_McaMorf *arg0, s32 arg1, f32 arg2, f32 arg3, s32 arg4, s32 arg5, s8 *arg6); /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cRC3Vecsfff(u8 arg0, fopAc_ac_c *arg1, Vec *arg2, s16 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cfff(u8 arg0, fopAc_ac_c *arg1, f32 arg2, f32 arg3, f32 arg4); /* extern */
? entryDL__14mDoExt_McaMorfFP16J3DMaterialTable(mDoExt_McaMorf *this, J3DMaterialTable *arg0); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
dDemo_actor_c *getActor__14dDemo_object_cFUc(dDemo_object_c *this, u8 arg0); /* extern */
u8 getEventReg__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
JUTAssertion *getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(dRes_control_c *this, s8 *arg0, u16 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
u8 getIndex__10JUTNameTabCFPCc(JUTNameTab *this, s8 *arg0); /* extern */
? getIsAddvance__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 getMyActIdx__16dEvent_manager_cFiPCPCciii(dEvent_manager_c *this, s32 arg0, s8 **arg1, s32 arg2, s32 arg3, s32 arg4); /* extern */
? getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(dEvent_manager_c *this, s8 *arg0, fopAc_ac_c *arg1, s32 arg2); /* extern */
J3DAnmTexPattern *getP_BtpData__13dDemo_actor_cFPCc(dDemo_actor_c *this, s8 *arg0); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? lookAtTarget_2__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb(dNpc_JntCtrl_c *this, s16 *arg0, cXyz *arg1, cXyz arg2, s16 arg3, s16 arg4, u8 arg5); /* extern */
void **mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_modelEntryDL__FP8J3DModel(J3DModel *arg0); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_ZXYrotM__FPA4_fsss(f32 (*arg0)[4], s16 arg1, s16 arg2, s16 arg3); /* extern */
? mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? memcpy(void *, void *, ?);                        /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
u8 play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setMorf__14mDoExt_McaMorfFf(mDoExt_McaMorf *this, f32 arg0); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
? setSimple__13dPa_control_cFUsPC4cXyzUcRC8_GXColorRC8_GXColori(dPa_control_c *this, u16 arg0, cXyz *arg1, u8 arg2, _GXColor *arg3, _GXColor *arg4, s32 arg5); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
? stopZelAnime__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? strcpy(dRes_control_c *, s8 *);                   /* extern */
? talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
s32 CreateHeap__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void *__ct__11daNpc_Ym1_cFv(daNpc_Ym1_c *this);     /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__15daNpc_Ym1_HIO_cFv(daNpc_Ym1_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__20daNpc_Ym1_childHIO_cFv(daNpc_Ym1_childHIO_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void _nodeCB_BackBone__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(daNpc_Ym1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void _nodeCB_Head__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(daNpc_Ym1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void anmAtr__11daNpc_Ym1_cFUs(daNpc_Ym1_c *this, u16 arg0); /* static */
s16 cLib_calcTimer<s>__FPs(s16 *arg0);              /* static */
s16 cLib_getRndValue<i>__Fii(s32 arg0, s32 arg1);   /* static */
u8 chk_BlackPig__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void daNpc_Ym1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
void daNpc_Ym1_Delete__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0); /* static */
void daNpc_Ym1_Draw__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0); /* static */
void daNpc_Ym1_Execute__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0); /* static */
s32 daNpc_Ym1_IsDelete__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0); /* static */
s32 demo_action1__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0); /* static */
s32 getMsg__11daNpc_Ym1_cFv(daNpc_Ym1_c *this);     /* static */
s32 next_msgStatus__11daNpc_Ym1_cFPUl(daNpc_Ym1_c *this, u32 *arg0); /* static */
void play_animation__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void play_texPttrnAnm__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void setAnm_ATR__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void setAttention__11daNpc_Ym1_cFb(daNpc_Ym1_c *this, u8 arg0); /* static */
void setKariFlg__11daNpc_Ym1_cFv(daNpc_Ym1_c *this); /* static */
void setMtx__11daNpc_Ym1_cFb(daNpc_Ym1_c *this, u8 arg0); /* static */
s32 set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(daNpc_Ym1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0); /* static */
s32 wait_action1__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0); /* static */
s32 wait_action2__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0); /* static */
s32 wait_action3__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0); /* static */
s32 wait_action4__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0); /* static */
void *@4164(cXyz *this, s16 destroyFlag);           /* static */
extern dCcD_SrcCyl dNpc_cyl_src;
extern ? g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern _GXColor g_whiteColor;
extern ? j3dSys;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? a_prm_tbl$4151;                            /* unable to generate initializer: unknown type */
static ? @4257;                                     /* unable to generate initializer: unknown type */
static ? @4269;                                     /* unable to generate initializer: unknown type */
static ? @4282;                                     /* unable to generate initializer: unknown type */
static ? @4299;                                     /* unable to generate initializer: unknown type */
static ? @4315;                                     /* unable to generate initializer: unknown type */
static ? @4328;                                     /* unable to generate initializer: unknown type */
static ? @4337;                                     /* unable to generate initializer: unknown type */
static s16 a_att_dis_TBL$4345[4] = { 0, 0xAAAA, 0xAAAA, 0 };
static s8 *a_staff_tbl$4346[8] = {
    "Ym1",
    "Ym1",
    "Ym2\0d_a_npc_ym1.cpp\0a_btp != 0\0Halt\0Ym",
    "Ym2\0d_a_npc_ym1.cpp\0a_btp != 0\0Halt\0Ym",
    "Ym2\0d_a_npc_ym1.cpp\0a_btp != 0\0Halt\0Ym",
    "Ym2\0d_a_npc_ym1.cpp\0a_btp != 0\0Halt\0Ym",
    "Ym1",
    "Ym2\0d_a_npc_ym1.cpp\0a_btp != 0\0Halt\0Ym",
};
static struct _struct_a_anm_prm_tbl$4530_0x10 a_anm_prm_tbl$4530[0xD]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4537_0x10 a_anm_prm_tbl$4537[0xC]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4575_0x10 a_anm_prm_tbl$4575[0xD]; /* unable to generate initializer: non-zero padding */
static s8 *a_cut_tbl$4964 = "DUMMY\0a_mdl_dat != 0\0head\0m_hed_jnt_num >= 0\0backbone\0m_bbone_jnt_num >= 0\0handL\0m_hnd_L_jnt_num >= 0\0handR\0m_hnd_R_jnt_num >= 0";
static u32 a_siz_tbl$5560[3] = { 0, 0, 0 };
static s32 a_hed_bdl_resID_tbl$5867[3] = { 0, 0xB, 0xC };
static ? a_tex_pttrn_num_tbl$5870;                  /* unable to generate initializer: unknown type */
static ? @3569;
static struct _struct_l_HIO_0x2C l_HIO[2];
static s32 a_res_id_tbl$4471[0xD] = { 0, 1, 2, 5, 3, 8, 6, 7, 4, 0x13, 0x10, 0x11, 0x12 }; /* const */
static s32 a_res_id_tbl$4476[1] = { 0xE };          /* const */

/* daNpc_Ym1_childHIO_c::daNpc_Ym1_childHIO_c (void) */
void __ct__20daNpc_Ym1_childHIO_cFv(daNpc_Ym1_childHIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__20daNpc_Ym1_childHIO_c;
}

/* daNpc_Ym1_HIO_c::daNpc_Ym1_HIO_c (void) */
daNpc_Ym1_HIO_c *__ct__15daNpc_Ym1_HIO_cFv(daNpc_Ym1_HIO_c *this, ? arg_sp0) {
    s32 var_r28;
    s32 var_r30;
    s32 var_r31;

    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__15daNpc_Ym1_HIO_c;
    __construct_array(&this->unkC, __ct__20daNpc_Ym1_childHIO_cFv, __dt__20daNpc_Ym1_childHIO_cFv, 0x2C, 2);
    var_r28 = 0;
    var_r31 = 0;
    var_r30 = 0;
    do {
        *(this + (var_r30 + 0x34)) = var_r28;
        memcpy(this + (var_r30 + 0x10), &a_prm_tbl$4151 + var_r31, 0x24);
        var_r28 += 1;
        var_r31 += 0x24;
        var_r30 += 0x2C;
    } while (var_r28 < 2);
    this->unk4 = -1;
    this->unk8 = -1;
    return this;
}

/* area_check (fopAc_ac_c *, cXyz *, long unsigned) */
void area_check__FP10fopAc_ac_cP4cXyzUl(fopAc_ac_c *arg0, cXyz *arg1, u32 arg2) {
    if (arg2 == 0) {
        setKariFlg__11daNpc_Ym1_cFv((daNpc_Ym1_c *) arg0);
    }
}

/* daNpc_Ym1_c::setKariFlg (void) */
void setKariFlg__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    this->unk89B = 1;
}

/* nodeCB_Head (J3DNode *, int) */
s32 nodeCB_Head__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ym1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_Head__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ym1_c::_nodeCB_Head (J3DNode *, J3DModel *) */
void _nodeCB_Head__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(daNpc_Ym1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r30;
    void *(*temp_r3)(daNpc_Ym1_HIO_c *, s16);

    if ((s8) @3569.unk11C == 0) {
        @3569.unk120 = 26.0f;
        temp_r3 = &@3569 + 0x120;
        temp_r3->unk4 = (f32) @4213.unk4;
        temp_r3->unk8 = (f32) @4213.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x110);
        @3569.unk11C = 1U;
    }
    temp_r30 = arg0->unk18 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r30, &now__14mDoMtx_stack_c);
    this->unk870 = now__14mDoMtx_stack_c.unkC;
    this->unk874 = now__14mDoMtx_stack_c.unk1C;
    this->unk878 = now__14mDoMtx_stack_c.unk2C;
    mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk292);
    mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk290);
    PSMTXMultVec(&now__14mDoMtx_stack_c, &@3569 + 0x120, &this->unk84C);
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r30);
}

/* nodeCB_BackBone (J3DNode *, int) */
s32 nodeCB_BackBone__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ym1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeCB_BackBone__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ym1_c::_nodeCB_BackBone (J3DNode *, J3DModel *) */
void _nodeCB_BackBone__11daNpc_Ym1_cFP7J3DNodeP8J3DModel(daNpc_Ym1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
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
    CreateHeap__11daNpc_Ym1_cFv((daNpc_Ym1_c *) arg0);
}

/* daNpc_Ym1_c::init_YM1_0 (void) */
u8 init_YM1_0__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) == 0) {
        sp8 = @4257.unk0;
        spC = @4257.unk4;
        sp10 = @4257.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YM1_1 (void) */
u8 init_YM1_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) {
        sp8 = @4269.unk0;
        spC = @4269.unk4;
        sp10 = @4269.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YM2_0 (void) */
u8 init_YM2_0__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) == 0) && (isEventBit__11dSv_event_cFUs(temp_r31, 0xE20U) == 0)) {
        sp8 = @4282.unk0;
        spC = @4282.unk4;
        sp10 = @4282.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YM2_1 (void) */
u8 init_YM2_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x520U) == 0) && (isEventBit__11dSv_event_cFUs(temp_r31, 0xE20U) != 0)) {
        sp8 = @4299.unk0;
        spC = @4299.unk4;
        sp10 = @4299.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YM2_2 (void) */
u8 init_YM2_2__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 0)) {
        sp8 = @4315.unk0;
        spC = @4315.unk4;
        sp10 = @4315.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YM2_3 (void) */
u8 init_YM2_3__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 1)) {
        sp8 = @4328.unk0;
        spC = @4328.unk4;
        sp10 = @4328.unk8;
        set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::init_YMx_error (void) */
u8 init_YMx_error__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    sp8 = @4337.unk0;
    spC = @4337.unk4;
    sp10 = @4337.unk8;
    set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv((daNpc_Ym1_c *) &sp8, NULL);
    return 1U;
}

/* daNpc_Ym1_c::createInit (void) */
u8 createInit__11daNpc_Ym1_cFv(daNpc_Ym1_c *this, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    cXyz sp8;
    dBgS *temp_r29;
    u8 temp_r0;
    u8 var_r3;

    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->unk2C4, a_staff_tbl$4346[(s8) this->unk8B1], (fopNpc_npc_c *) this);
    this->unk280 = 0xA;
    this->unk26D = a_att_dis_TBL$4345[(s8) this->unk8B0];
    this->unk26F = a_att_dis_TBL$4345[(s8) this->unk8B0].unk1;
    this->unk8AB = 0xD;
    temp_r0 = this->unk8B1;
    if ((u32) (s8) temp_r0 <= 7U) {
        switch ((s8) temp_r0) {
        case 0:
            var_r3 = init_YM1_0__11daNpc_Ym1_cFv(this);
            break;
        case 1:
            var_r3 = init_YM1_1__11daNpc_Ym1_cFv(this);
            break;
        case 2:
            var_r3 = init_YM2_0__11daNpc_Ym1_cFv(this);
            break;
        case 3:
            var_r3 = init_YM2_1__11daNpc_Ym1_cFv(this);
            break;
        case 4:
            var_r3 = init_YM2_2__11daNpc_Ym1_cFv(this);
            break;
        case 5:
            var_r3 = init_YM2_3__11daNpc_Ym1_cFv(this);
            break;
        case 6:
        case 7:
            var_r3 = init_YMx_error__11daNpc_Ym1_cFv(this);
            break;
        }
    } else {
        var_r3 = 0;
    }
    if (var_r3 == 0) {
        return 0U;
    }
    this->unk846 = this->unk204;
    this->unk848 = this->unk206;
    this->unk84A = this->unk208;
    this->unk20C = this->unk846;
    this->unk20E = this->unk848;
    this->unk210 = this->unk84A;
    this->unk258 = @4213.unkC;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk538, 0xFF, 0xFF, (fopAc_ac_c *) this);
    this->unk5B8 = &this->unk538;
    Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk574, &dNpc_cyl_src);
    if ((s8) this->unk8B1 == 0) {
        sp14 = @4213.unk8;
        sp18 = @4213.unk8;
        sp1C = @4213.unk10;
        this->unk748 = &this->unk538;
        Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk704, &dNpc_cyl_src);
        PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk206);
        PSMTXMultVec(&now__14mDoMtx_stack_c, &sp14, &sp8);
        SetC__8cM3dGCylFRC4cXyz(&this->unk81C, &sp8);
        SetR__8cM3dGCylFf(&this->unk81C, @4213.unk14);
        SetH__8cM3dGCylFf(&this->unk81C, @4213.unk18);
        Set__12dCcMassS_MngFP8cCcD_ObjUc(&g_dComIfG_gameInfo + 0x4EF8, (cCcD_Obj *) &this->unk704, 3U);
    }
    play_animation__11daNpc_Ym1_cFv(this);
    if ((s32) this->unk8B1 != 1) {
        CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo + 0x12A0);
    }
    temp_r29 = &g_dComIfG_gameInfo + 0x12A0;
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r29, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r29, &this->unk41C);
    setMorf__14mDoExt_McaMorfFf(this->unk330, @4213.unk8);
    setMtx__11daNpc_Ym1_cFb(this, 1U);
    return 1U;
}

/* daNpc_Ym1_c::play_animation (void) */
void play_animation__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    play_texPttrnAnm__11daNpc_Ym1_cFv(this);
    this->unk898 = play__14mDoExt_McaMorfFP3VecUlSc(this->unk330, &this->unk260, 0U, 0);
    if (this->unk330->unk68 < this->unk87C) {
        this->unk898 = 1;
    }
    this->unk87C = this->unk330->unk68;
}

/* daNpc_Ym1_c::setMtx (bool) */
void setMtx__11daNpc_Ym1_cFb(daNpc_Ym1_c *this, u8 arg0) {
    J3DModel *temp_r3;
    void **temp_r3_2;
    void **temp_r3_3;
    void **temp_r4;

    temp_r3 = this->unk330->unk50;
    temp_r3->unk18 = this->unk214;
    temp_r3->unk1C = this->unk218;
    temp_r3->unk20 = this->unk21C;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk846, this->unk848, this->unk84A);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->unk330->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->unk330);
    PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CC * 0x30), (mDoMtx_stack_c *) (this->unk6DC + 0x24));
    temp_r3_2 = this->unk6DC;
    (*temp_r3_2)->unk10(temp_r3_2);
    temp_r4 = this->unk6D0;
    if (temp_r4 != NULL) {
        PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CF * 0x30), (mDoMtx_stack_c *) (temp_r4 + 0x24));
        temp_r3_3 = this->unk6D0;
        (*temp_r3_3)->unk10(temp_r3_3);
    }
    setAttention__11daNpc_Ym1_cFb(this, arg0);
}

/* daNpc_Ym1_c::bckResID (int) */
s32 bckResID__11daNpc_Ym1_cFi(daNpc_Ym1_c *this, s32 arg0) {
    return a_res_id_tbl$4471[arg0];
}

/* daNpc_Ym1_c::btpResID (int) */
s32 btpResID__11daNpc_Ym1_cFi(daNpc_Ym1_c *this, s32 arg0) {
    s32 temp_r4;
    u8 temp_r0;

    temp_r4 = a_res_id_tbl$4476[arg0];
    if (temp_r4 == 0xE) {
        temp_r0 = this->unk8B0;
        switch ((s8) temp_r0) {                     /* irregular */
        case 1:
            return 0xE;
        case 2:
            return 0xF;
        }
    } else {
    default:
        return temp_r4;
    }
}

/* daNpc_Ym1_c::init_texPttrnAnm (char signed, bool) */
u8 init_texPttrnAnm__11daNpc_Ym1_cFScb(daNpc_Ym1_c *this, s8 arg0, s32 arg1, ? arg_sp0) {
    s32 sp8;
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;
    void **temp_r31;

    temp_r31 = this->unk6DC;
    if (arg0 < 0) {
        return 0U;
    }
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(&this->unk6D4, (s8 *) btpResID__11daNpc_Ym1_cFi(this, (s32) arg0), (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 8), (s8 *)0x270, (s32) (temp_r4 + 0x18), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ym1";
        OSPanic(temp_r3_2 + 8, 0x270, temp_r3_2 + 0x23);
    }
    this->unk8AA = arg0;
    this->unk6F4 = 0;
    this->unk6F6 = 0;
    sp8 = 0;
    return init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6E0, temp_r31->unk4, (J3DAnmTexPattern *) temp_r3, 1, 0, 1.0f, 0, -1, arg1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0;
}

/* daNpc_Ym1_c::play_texPttrnAnm (void) */
void play_texPttrnAnm__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s16 temp_r3;

    if ((((s8) (u8) this->unk8AA != 0) || (cLib_calcTimer<s>__FPs(&this->unk6F6) == 0)) && (this->unk6F4 += 1, temp_r3 = this->unk6E8->unk6, (((s32) this->unk6F4 < temp_r3) == 0))) {
        if ((s8) (u8) this->unk8AA != 0) {
            this->unk6F4 = (u8) temp_r3;
            return;
        }
        this->unk6F6 = cLib_getRndValue<i>__Fii(0x3C, 0x5A);
        this->unk6F4 = 0;
    }
}

/* daNpc_Ym1_c::setAnm_anm (daNpc_Ym1_c::anm_prm_c *) */
void setAnm_anm__11daNpc_Ym1_cFPQ211daNpc_Ym1_c9anm_prm_c(daNpc_Ym1_c *this, daNpc_Ym1_c::anm_prm_c *arg0) {
    u8 temp_r4;

    temp_r4 = arg0->unk0;
    if ((s8) temp_r4 >= 0) {
        if ((s8) this->unk8AB == (s8) temp_r4) {
            return;
        }
        dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->unk330, arg0->unkC, arg0->unk4, arg0->unk8, bckResID__11daNpc_Ym1_cFi(this, (s32) (s8) temp_r4), -1, &this->unk6D4.unk0);
        this->unk8AB = arg0->unk0;
        this->unk898 = 0;
        this->unk899 = 0;
        this->unk87C = 0.0f;
        if ((s32) this->unk8AB == 8) {
            setMorf__14mDoExt_McaMorfFf(this->unk330, l_HIO[(s8) this->unk8B0].unk4);
        }
    }
}

/* daNpc_Ym1_c::setAnm_NUM (int, int) */
void setAnm_NUM__11daNpc_Ym1_cFii(daNpc_Ym1_c *this, s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        init_texPttrnAnm__11daNpc_Ym1_cFScb(this, (s8) a_anm_prm_tbl$4530[arg0].unk1, 1);
    }
    setAnm_anm__11daNpc_Ym1_cFPQ211daNpc_Ym1_c9anm_prm_c(this, (daNpc_Ym1_c::anm_prm_c *) &a_anm_prm_tbl$4530[arg0]);
}

/* daNpc_Ym1_c::setAnm (void) */
void setAnm__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    init_texPttrnAnm__11daNpc_Ym1_cFScb(this, (s8) a_anm_prm_tbl$4537[(s8) this->mSttNum].unk1, 1);
    setAnm_anm__11daNpc_Ym1_cFPQ211daNpc_Ym1_c9anm_prm_c(this, (daNpc_Ym1_c::anm_prm_c *) &a_anm_prm_tbl$4537[(s8) this->mSttNum]);
}

/* daNpc_Ym1_c::chngAnmTag (void) */
void chngAnmTag__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {

}

/* daNpc_Ym1_c::ctrlAnmTag (void) */
void ctrlAnmTag__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {

}

/* daNpc_Ym1_c::chngAnmAtr (char unsigned) */
void chngAnmAtr__11daNpc_Ym1_cFUc(daNpc_Ym1_c *this, u8 arg0) {
    if (arg0 != (u8) this->mAnmAtr) {
        if (arg0 > 0xDU) {
            return;
        }
        this->mAnmAtr = arg0;
        setAnm_ATR__11daNpc_Ym1_cFv(this);
    }
}

/* daNpc_Ym1_c::ctrlAnmAtr (void) */
void ctrlAnmAtr__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {

}

/* daNpc_Ym1_c::setAnm_ATR (void) */
void setAnm_ATR__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    init_texPttrnAnm__11daNpc_Ym1_cFScb(this, (s8) a_anm_prm_tbl$4575[this->mAnmAtr].unk1, 1);
    setAnm_anm__11daNpc_Ym1_cFPQ211daNpc_Ym1_c9anm_prm_c(this, (daNpc_Ym1_c::anm_prm_c *) &a_anm_prm_tbl$4575[this->mAnmAtr]);
}

/* daNpc_Ym1_c::anmAtr (short unsigned) */
void anmAtr__11daNpc_Ym1_cFUs(daNpc_Ym1_c *this, u16 arg0) {
    u8 temp_r4;

    switch ((s32) arg0) {                           /* irregular */
    case 6:
        if ((s8) this->unk8B3 == 0) {
            chngAnmAtr__11daNpc_Ym1_cFUc(this, g_dComIfG_gameInfo.unk5BDB);
            this->unk8B3 += 1;
        }
        temp_r4 = g_dComIfG_gameInfo.unk5BDC;
        if ((temp_r4 != 0xFF) && (temp_r4 != (u8) this->unk8A9)) {
            g_dComIfG_gameInfo.unk5BDC = 0xFFU;
            this->unk8A9 = temp_r4;
            chngAnmTag__11daNpc_Ym1_cFv(this);
        }
        break;
    case 14:
        this->unk8B3 = 0;
        break;
    }
    ctrlAnmAtr__11daNpc_Ym1_cFv(this);
    ctrlAnmTag__11daNpc_Ym1_cFv(this);
}

/* daNpc_Ym1_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__11daNpc_Ym1_cFPUl(daNpc_Ym1_c *this, u32 *arg0, ? arg_sp0) {
    dSv_event_c *temp_r29;
    s32 var_r31;
    u32 temp_r0;

    var_r31 = 0xF;
    temp_r0 = *arg0;
    switch ((s32) temp_r0) {                        /* irregular */
    case 0x8FD:
        *arg0 = 0x8FE;
        break;
    case 0x8FF:
        *arg0 = 0x900;
        break;
    case 0x901:
        *arg0 = 0x902;
        break;
    case 0x90A:
        *arg0 = 0x90B;
        break;
    case 0x904:
        *arg0 = 0x905;
        break;
    case 0x905:
        *arg0 = 0x906;
        break;
    case 0x907:
        *arg0 = 0x908;
        break;
    case 0x908:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3004U) != 0) {
            var_r31 = 0x10;
        } else {
            *arg0 = 0x909;
        }
        break;
    case 0xA2F:
        if (dKy_daynight_check__Fv() == 1) {
            *arg0 = 0xA30;
        } else {
            temp_r29 = &g_dComIfG_gameInfo + 0x624;
            if (getEventReg__11dSv_event_cFUs(temp_r29, 0xBFFFU) != 0) {
                if (isEventBit__11dSv_event_cFUs(temp_r29, 0x3402U) != 0) {
                    *arg0 = 0xA31;
                } else {
                    *arg0 = 0xA33;
                }
            } else if (isEventBit__11dSv_event_cFUs(temp_r29, 0x3402U) != 0) {
                *arg0 = 0xA37;
            } else {
                *arg0 = 0xA35;
            }
        }
        break;
    case 0xA31:
        *arg0 = 0xA32;
        break;
    case 0xA33:
        *arg0 = 0xA34;
        break;
    case 0xA35:
        *arg0 = 0xA36;
        break;
    case 0xA37:
        *arg0 = 0xA38;
        break;
    case 0xA3B:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3402U) != 0) {
            *arg0 = 0xA3C;
        } else {
            *arg0 = 0xA3D;
        }
        break;
    case 0xA3E:
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3402U) != 0) {
            *arg0 = 0xA40;
        } else {
            *arg0 = 0xA3F;
        }
        break;
    case 0xA41:
        *arg0 = 0xA42;
        break;
    default:
        var_r31 = 0x10;
        break;
    }
    return var_r31;
}

/* daNpc_Ym1_c::getMsg_YM1_0 (void) */
s32 getMsg_YM1_0__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    dSv_event_c *temp_r31;
    s32 var_r3;

    if ((u8) g_dComIfG_gameInfo.unkB4 != 0) {
        var_r3 = 0x901;
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x10U) != 0) {
            return 0x903;
        }
        /* Duplicate return node #7. Try simplifying control flow for better match */
        return var_r3;
    }
    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x20U) != 0) {
        return 0x8FF;
    }
    var_r3 = 0x90A;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x2A80U) != 0) {
        var_r3 = 0x8FD;
    }
    return var_r3;
}

/* daNpc_Ym1_c::getMsg_YM1_1 (void) */
s32 getMsg_YM1_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 var_r3;

    var_r3 = 0x904;
    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2904U) != 0) {
        var_r3 = 0x907;
    }
    return var_r3;
}

/* daNpc_Ym1_c::getMsg_YM2_0 (void) */
s32 getMsg_YM2_0__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    if (chk_BlackPig__11daNpc_Ym1_cFv(this) != 0) {
        return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 8U) != 0) + 0xA2B;
    }
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x80U) != 0) + 0xA29;
}

/* daNpc_Ym1_c::getMsg_YM2_1 (void) */
s32 getMsg_YM2_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xB01U) != 0) + 0xA2D;
}

/* daNpc_Ym1_c::getMsg_YM2_2 (void) */
s32 getMsg_YM2_2__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    dSv_event_c *temp_r31;
    s32 var_r3;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3140U) == 0) {
        return 0xA2F;
    }
    if (dKy_daynight_check__Fv() == 1) {
        return (isEventBit__11dSv_event_cFUs(temp_r31, 0x3402U) != 0) + 0xA39;
    }
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3580U) == 0) {
        return 0xA3B;
    }
    var_r3 = 0xA41;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3540U) == 0) {
        var_r3 = 0xA3E;
    }
    return var_r3;
}

/* daNpc_Ym1_c::getMsg_YM2_3 (void) */
void getMsg_YM2_3__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    getMsg_YM2_2__11daNpc_Ym1_cFv(this);
}

/* daNpc_Ym1_c::getMsg (void) */
s32 getMsg__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 var_r4;
    u8 temp_r0;

    var_r4 = 0;
    temp_r0 = this->unk8B1;
    switch ((s8) temp_r0) {                         /* irregular */
    case 0:
        var_r4 = getMsg_YM1_0__11daNpc_Ym1_cFv(this);
        break;
    case 1:
        var_r4 = getMsg_YM1_1__11daNpc_Ym1_cFv(this);
        break;
    case 2:
        var_r4 = getMsg_YM2_0__11daNpc_Ym1_cFv(this);
        break;
    case 3:
        var_r4 = getMsg_YM2_1__11daNpc_Ym1_cFv(this);
        break;
    case 4:
        var_r4 = getMsg_YM2_2__11daNpc_Ym1_cFv(this);
        break;
    case 5:
        var_r4 = getMsg_YM2_3__11daNpc_Ym1_cFv(this);
        break;
    }
    return var_r4;
}

/* daNpc_Ym1_c::eventOrder (void) */
void eventOrder__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 2)) {
        this->unkFA |= 1;
        if ((s32) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
        }
    }
}

/* daNpc_Ym1_c::checkOrder (void) */
void checkOrder__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 1:
        temp_r0_2 = this->mOrderType;
        if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 2)) {
            this->mOrderType = 0;
            this->unk8A3 = 1;
            return;
        }
    case 2:
        return;
    }
}

/* daNpc_Ym1_c::set_collision_sp (void) */
void set_collision_sp__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp1C;
    f32 sp18;
    cXyz sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f30;
    f32 var_f31;
    u8 temp_r0;

    if ((u8) this->unk8A3 == 0) {
        temp_r0 = this->unk8AB;
        if ((s8) temp_r0 < 5) {
            if ((s8) temp_r0 != 1) {
                goto block_8;
            }
            PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
            mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk206);
            sp8 = @4213.unk8;
            spC = @4213.unk8;
            sp10 = @4213.unk58;
            var_f30 = @4213.unk5C;
            var_f31 = @4213.unk60;
            PSMTXMultVec(&now__14mDoMtx_stack_c, &sp8, &sp14);
        } else if ((s8) temp_r0 < 8) {
            PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
            mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk206);
            sp8 = @4213.unk8;
            spC = @4213.unk8;
            sp10 = @4213.unk5C;
            var_f30 = @4213.unk10;
            var_f31 = @4213.unk64;
            PSMTXMultVec(&now__14mDoMtx_stack_c, &sp8, &sp14);
        } else {
block_8:
            sp14 = this->unk1F8;
            sp18 = this->unk1FC;
            sp1C = this->unk200;
            var_f30 = @4213.unk5C;
            var_f31 = @4213.unk68;
        }
        SetC__8cM3dGCylFRC4cXyz(&this->unk68C, &sp14);
        SetR__8cM3dGCylFf(&this->unk68C, var_f30);
        SetH__8cM3dGCylFf(&this->unk68C, var_f31);
        Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo + 0x26A4, (cCcD_Obj *) &this->unk574);
    }
}

/* daNpc_Ym1_c::set_cutGrass (void) */
void set_cutGrass__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if (((u8) this->unk8A6 == 0) && ((s32) this->unk8AB == 1) && (checkPass__12J3DFrameCtrlFf(&this->unk330->unk58, @4213.unk6C) != 0)) {
        PSMTXCopy(this->unk330->unk50->unk8C + ((s8) this->unk6CE * 0x30), &now__14mDoMtx_stack_c);
        sp8 = now__14mDoMtx_stack_c.unkC;
        spC = now__14mDoMtx_stack_c.unk1C;
        sp10 = now__14mDoMtx_stack_c.unk2C;
        setSimple__13dPa_control_cFUsPC4cXyzUcRC8_GXColorRC8_GXColori(g_dComIfG_gameInfo.unk5AC4, 0x3DAU, (cXyz *) &sp8, 0xFFU, &g_whiteColor, &g_whiteColor, 0);
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5812U, (Vec *) &this->unk1F8, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4213.unk54, @4213.unk54, @4213.unk70, @4213.unk70, 0U);
    }
}

/* daNpc_Ym1_c::chk_BlackPig (void) */
u8 chk_BlackPig__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    return (getEventReg__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xBFFFU) >> 2U) & 1;
}

/* daNpc_Ym1_c::chk_nbt_attn (void) */
u8 chk_nbt_attn__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    u8 temp_r0;
    u8 var_r4;

    var_r4 = 0;
    temp_r0 = this->unk8AB;
    if (((s8) temp_r0 == 5) || ((s8) temp_r0 == 6) || ((s8) temp_r0 == 7)) {
        var_r4 = 1;
    }
    return var_r4;
}

/* daNpc_Ym1_c::chk_talk (void) */
u8 chk_talk__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    u8 var_r0;

    var_r0 = 0;
    if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
        var_r0 = 1;
    }
    if (var_r0 != 0) {
        if (ChkPresentEnd__16dEvent_manager_cFv(&g_dComIfG_gameInfo + 0x52CC) != 0) {
            this->unk89A = g_dComIfG_gameInfo.unk52B9;
            return 1U;
        }
        return 0U;
    }
    this->unk89A = 0xFF;
    return 1U;
}

/* daNpc_Ym1_c::chk_parts_notMov (void) */
u8 chk_parts_notMov__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    u8 var_r5;

    var_r5 = 0;
    if (((s16) this->unk882 == (s16) this->unk292) && ((s16) this->unk884 == (s16) this->unk296) && ((s16) this->unk880 == (s16) this->unk206)) {
        var_r5 = 1;
    }
    return var_r5;
}

/* daNpc_Ym1_c::lookBack (void) */
void lookBack__11daNpc_Ym1_cFv(daNpc_Ym1_c *this, ? arg_sp0) {
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
    f32 *var_r5;
    s16 var_r31;
    u8 temp_r0;
    u8 temp_r30;

    this->unk882 = this->unk292;
    this->unk884 = this->unk296;
    this->unk880 = this->unk206;
    sp20 = this->unk1F8;
    sp24 = this->unk1FC;
    sp28 = this->unk200;
    sp24 = this->unk264;
    sp2C = 0.0f;
    sp30 = 0.0f;
    sp34 = 0.0f;
    var_r5 = NULL;
    var_r31 = this->unk206;
    temp_r30 = this->unk8A4;
    temp_r0 = this->unk8AF;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        dNpc_playerEyePos__Ff(-20.0f);
        this->unk858 = sp14;
        this->unk85C = sp18;
        this->unk860 = sp1C;
        sp2C = this->unk858;
        sp30 = this->unk85C;
        sp34 = this->unk860;
        var_r5 = &sp2C;
        break;
    case 2:
        sp2C = this->unk858;
        sp30 = this->unk85C;
        sp34 = this->unk860;
        var_r5 = &sp2C;
        break;
    case 3:
        var_r31 = this->unk896;
        break;
    }
    sp8 = sp20;
    spC = sp24;
    sp10 = sp28;
    lookAtTarget_2__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk206, (cXyz *) var_r5, (cXyz) &sp8, var_r31, l_HIO[(s8) this->unk8B0].unk-A, temp_r30);
}

/* daNpc_Ym1_c::chkAttention (void) */
u8 chkAttention__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    dAttention_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x5808;
    if (LockonTruth__12dAttention_cFv(temp_r31) != 0) {
        return this == LockonTarget__12dAttention_cFl(temp_r31, 0);
    }
    return this == ActionTarget__12dAttention_cFl(temp_r31, 0);
}

/* daNpc_Ym1_c::setAttention (bool) */
void setAttention__11daNpc_Ym1_cFb(daNpc_Ym1_c *this, u8 arg0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f31;

    var_f31 = l_HIO[(s8) this->unk8B0].unk-8;
    sp8 = this->unk1F8;
    spC = this->unk1FC;
    sp10 = this->unk200;
    if (chk_nbt_attn__11daNpc_Ym1_cFv(this) != 0) {
        if ((s32) this->unk8AB == 6) {
            var_f31 = this->unk874 - 210.0f;
        } else {
            var_f31 = 116.0f;
        }
        sp8 = this->unk864;
        spC = this->unk868;
        sp10 = this->unk86C;
    }
    this->unk274 = sp8;
    this->unk278 = spC + var_f31;
    this->unk27C = sp10;
    if (((s32) this->unk888 != 0) || (arg0 != 0)) {
        this->unk260.unk0 = this->unk84C.unk0;
        this->unk264 = this->unk850;
        this->unk268 = this->unk854;
    }
}

/* daNpc_Ym1_c::decideType (int) */
u8 decideType__11daNpc_Ym1_cFi(daNpc_Ym1_c *this, s32 arg0) {
    s16 temp_r0;
    u8 var_r3;

    if ((s8) this->unk8B0 > 0) {
        return 1U;
    }
    this->unk8B0 = -1U;
    this->unk8B1 = -1U;
    temp_r0 = this->unk8;
    switch (temp_r0) {                              /* switch 1; irregular */
    case 0x13D:                                     /* switch 1 */
        this->unk8B0 = 1;
        switch (arg0) {                             /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            this->unk8B1 = 0;
            break;
        case 1:                                     /* switch 2 */
            this->unk8B1 = 1;
            break;
        }
        break;
    case 0x13E:                                     /* switch 1 */
        this->unk8B0 = 2;
        switch (arg0) {                             /* switch 3; irregular */
        case 0:                                     /* switch 3 */
            this->unk8B1 = 2;
            break;
        case 1:                                     /* switch 3 */
            this->unk8B1 = 3;
            break;
        case 2:                                     /* switch 3 */
            this->unk8B1 = 4;
            break;
        case 3:                                     /* switch 3 */
            this->unk8B1 = 5;
            break;
        }
        break;
    }
    strcpy(&this->unk6D4, "Ym1" + 0x28);
    var_r3 = 0;
    if (((s8) this->unk8B0 != -1) && ((s8) this->unk8B1 != -1)) {
        var_r3 = 1;
    }
    return var_r3;
}

/* daNpc_Ym1_c::privateCut (int) */
void privateCut__11daNpc_Ym1_cFi(daNpc_Ym1_c *this, s32 arg0, ? arg_sp0) {
    dEvent_manager_c *temp_r31;

    if (arg0 != -1) {
        temp_r31 = &g_dComIfG_gameInfo + 0x52CC;
        this->unk8A7 = getMyActIdx__16dEvent_manager_cFiPCPCciii(temp_r31, arg0, &a_cut_tbl$4964, 1, 1, 0);
        if ((s8) this->unk8A7 == -1) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
            return;
        }
        getIsAddvance__16dEvent_manager_cFi(temp_r31, arg0);
        cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
    }
}

/* daNpc_Ym1_c::endEvent (void) */
void endEvent__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
    this->mAnmAtr = 0xFF;
    this->unk8A9 = 0xFF;
}

/* daNpc_Ym1_c::isEventEntry (void) */
void isEventEntry__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(&g_dComIfG_gameInfo + 0x52CC, this->unk2C4.unk0, NULL, 0);
}

/* daNpc_Ym1_c::event_proc (int) */
void event_proc__11daNpc_Ym1_cFi(daNpc_Ym1_c *this, s32 arg0) {
    if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
        privateCut__11daNpc_Ym1_cFi(this, arg0);
    }
}

s32 set_action__11daNpc_Ym1_cFM11daNpc_Ym1_cFPCvPvPv_iPv(daNpc_Ym1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0) {
    if (__ptmf_cmpr(arg0 + 0x6F8) != 0) {
        if (__ptmf_test(arg0 + 0x6F8) != 0) {
            arg0->unk8B2 = 9;
            __ptmf_scall(arg0, arg2);
        }
        arg0->unk6F8 = (s32) arg1->unk0;
        arg0->unk6FC = arg1->unk4;
        arg0->unk700 = arg1->unk8;
        arg0->unk8B2 = 0;
        __ptmf_scall(arg0, arg2);
    }
    return 1;
}

/* daNpc_Ym1_c::setStt (char signed) */
void setStt__11daNpc_Ym1_cFSc(daNpc_Ym1_c *this, s8 arg0) {
    u8 temp_r0;
    u8 temp_r5;

    temp_r5 = this->mSttNum;
    this->mSttNum = (u8) arg0;
    temp_r0 = this->mSttNum;
    if ((u32) (s8) temp_r0 <= 0xBU) {
        switch ((s8) temp_r0) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 8:
        case 10:
        case 11:
            this->mOrderType = 0;
            break;
        case 5:
        case 7:
        case 9:
            this->mOrderType = 0;
            this->mAnmAtr = 0xFF;
            this->unk8A9 = 0xFF;
            this->unk8B3 = 0;
            this->mSttNumOld = temp_r5;
            break;
        case 6:
            this->mOrderType = 0;
            this->unk890 = cLib_getRndValue<i>__Fii(0x5A, 0xB4);
            break;
        }
    }
    setAnm__11daNpc_Ym1_cFv(this);
}

/* daNpc_Ym1_c::chk_areaIN (float, cXyz) */
u8 chk_areaIN__11daNpc_Ym1_cFf4cXyz(daNpc_Ym1_c *this, f32 arg0, ? arg1) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    __mi__4cXyzCFRC3Vec(&sp18, g_dComIfG_gameInfo.unk5B4C + 0x1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4213.unk8;
    sp14 = sp20;
    var_f1 = PSVECSquareMag(&spC);
    if (var_f1 > @4213.unk8) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = @4213.unk80 * temp_f0 * (@4213.unk88 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4213.unk80 * temp_f0_2 * (@4213.unk88 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (@4213.unk80 * temp_f0_3 * (@4213.unk88 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    if ((var_f1 < arg0) && ((f32) fabs(g_dComIfG_gameInfo.unk5B4C->unk1FC - arg1->unk4) < @4213.unk90)) {
        return 1U;
    }
    return 0U;
}

/* daNpc_Ym1_c::kari_1 (void) */
s32 kari_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    cXyz sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    __mi__4cXyzCFRC3Vec(&sp24, g_dComIfG_gameInfo.unk5B4C + 0x1F8);
    spC = (bitwise f32) sp24;
    sp10 = @4213.unk8;
    sp14 = sp2C;
    temp_f1 = PSVECSquareMag(&spC);
    if (temp_f1 > @4213.unk8) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = @4213.unk80 * temp_f0 * (@4213.unk88 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4213.unk80 * temp_f0_2 * (@4213.unk88 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) temp_f1 * (@4213.unk80 * temp_f0_3 * (@4213.unk88 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
    }
    if (cLib_calcTimer<s>__FPs(&this->unk890) != 0) {
        return 1;
    }
    sp18 = this->unk1F8;
    sp1C = this->unk1FC;
    sp20 = this->unk200;
    if ((chk_areaIN__11daNpc_Ym1_cFf4cXyz(this, l_HIO[(s8) this->unk8B0].unk0, (cXyz) &sp18) != 0) || ((u8) this->unk89C == 0)) {
        setStt__11daNpc_Ym1_cFSc(this, 2);
        this->unk8AF = 0;
        this->unk8A4 = 1;
        setAnm_NUM__11daNpc_Ym1_cFii(this, 2, 1);
    }
    return 1;
}

/* daNpc_Ym1_c::wait_1 (void) */
s32 wait_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((s32) this->unk8AB == 2) {
        if ((s8) this->unk898 != 0) {
            setAnm_NUM__11daNpc_Ym1_cFii(this, 3, 1);
            this->unk8AF = 0;
            this->unk8A4 = 1;
            this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
        }
        return 1;
    }
    if ((u8) this->unk8A5 != 0) {
        cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk842, 4, l_HIO[(s8) this->unk8B0].unk-A, 0x80);
    }
    if ((u8) this->unk8A3 != 0) {
        if (chk_talk__11daNpc_Ym1_cFv(this) != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 5);
            this->unk8AF = 1;
            this->unk8A4 = 0;
            this->unk8A5 = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk8AF = 1;
    this->unk8A4 = 1;
    if ((u8) this->unk8A2 != 0) {
        this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
    }
    sp8 = this->unk1F8;
    spC = this->unk1FC;
    sp10 = this->unk200;
    if ((u8) (chk_areaIN__11daNpc_Ym1_cFf4cXyz(this, 50.0f + l_HIO[(s8) this->unk8B0].unk0, (cXyz) &sp8) == 0) != 0) {
        if (cLib_calcTimer<s>__FPs(&this->unk88E) == 0) {
            setStt__11daNpc_Ym1_cFSc(this, 3);
            this->unk8AF = 0;
            this->unk8A4 = 1;
        }
        return 1;
    }
    return 1;
}

/* daNpc_Ym1_c::wait_2 (void) */
s32 wait_2__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f30;
    f32 var_f31;
    s16 var_r3;
    u8 temp_r3;

    var_r3 = 0;
    if ((u8) this->unk8A5 != 0) {
        cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk842, 4, l_HIO[(s8) this->unk8B0].unk-A, 0x80);
        var_r3 = this->unk842 - this->unk206;
    }
    if ((u8) this->unk8A3 != 0) {
        if (chk_talk__11daNpc_Ym1_cFv(this) != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 5);
            this->unk8AF = 1;
            this->unk8A4 = 0;
            this->unk8A5 = 0;
            this->unk29A = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk8AF = 0;
    this->unk8A4 = 1;
    if (var_r3 == 0) {
        temp_r3 = this->unk8B0;
        if ((s32) temp_r3 == 1) {
            var_f31 = @4213.unk94;
        } else {
            var_f31 = @4213.unk98;
        }
        if ((s8) temp_r3 == 1) {
            var_f30 = @4213.unk9C;
        } else {
            var_f30 = @4213.unkA0;
        }
        if ((u8) this->unk8A2 != 0) {
            this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
        }
        if (cLib_calcTimer<s>__FPs(&this->unk88E) != 0) {
            this->unk8AF = 1;
        }
        sp8 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
        spC = g_dComIfG_gameInfo.unk5B4C->unk1FC;
        sp10 = g_dComIfG_gameInfo.unk5B4C->unk200;
        if (dNpc_chkAttn__FP10fopAc_ac_c4cXyzfffb((fopAc_ac_c *) this, (cXyz) &sp8, var_f30, @4213.unk14, var_f31, (s8) this->unk8AF == 1) != 0) {
            return 1;
        }
        this->unk8AF = 0;
        this->unk8A5 = 1;
        goto block_20;
    }
block_20:
    return 1;
}

/* daNpc_Ym1_c::talk_1 (void) */
u8 talk_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 temp_r0_2;
    u16 temp_r0;
    u8 temp_r31;
    void *temp_r3;

    temp_r31 = chk_parts_notMov__11daNpc_Ym1_cFv(this);
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
        temp_r0_2 = this->unk6A4;
        switch (temp_r0_2) {                        /* switch 2; irregular */
        case 0x902:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x10U);
            break;
        case 0x906:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2904U);
            break;
        case 0xA29:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x80U);
            break;
        case 0xA2B:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 8U);
            break;
        case 0xA2D:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xB01U);
            break;
        case 0xA40:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3540U);
            break;
        case 0x90B:                                 /* switch 2 */
        case 0x8FE:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x20U);
            break;
        case 0xA30:                                 /* switch 2 */
        case 0xA38:                                 /* switch 2 */
        case 0xA34:                                 /* switch 2 */
        case 0xA32:                                 /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3140U);
            break;
        default:                                    /* switch 2 */
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3580U);
            break;
        }
        this->unk89A = 0xFF;
        this->unk8A3 = 0;
        setStt__11daNpc_Ym1_cFSc(this, (s8) this->mSttNumOld);
        this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
        endEvent__11daNpc_Ym1_cFv(this);
        break;
    }
    return temp_r31;
}

/* daNpc_Ym1_c::turn_1 (void) */
s32 turn_1__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    cLib_addCalcAngleS__FPsssss(&this->unk206, this->unk842, 4, l_HIO[(s8) this->unk8B0].unk-A, 0x80);
    if ((s16) (this->unk842 - this->unk206) == 0) {
        if ((u8) this->unk89C != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 1);
            setMorf__14mDoExt_McaMorfFf(this->unk330, 10.0f);
            this->unk8AF = 0;
            this->unk8A4 = 1;
            this->unk890 = cLib_getRndValue<i>__Fii(0x1E, 0x3C);
        } else {
            setStt__11daNpc_Ym1_cFSc(this, 4);
            this->unk8AF = 0;
            this->unk8A4 = 1;
        }
    }
    return 1;
}

/* daNpc_Ym1_c::NBTwai (void) */
s32 NBTwai__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    if ((u8) this->unk8A3 != 0) {
        if (chk_talk__11daNpc_Ym1_cFv(this) != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 7);
            this->unk8AF = 0;
            this->unk8A4 = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk8AF = 0;
    this->unk8A4 = 1;
    if (chk_BlackPig__11daNpc_Ym1_cFv(this) != 0) {
        setStt__11daNpc_Ym1_cFSc(this, 8);
        this->unk8AF = 0;
        this->unk8A4 = 1;
        this->unk8A5 = 1;
        return 1;
    }
    if ((s32) this->unk8AB == 6) {
        if (((s8) this->unk898 != 0) || ((u8) this->unk8A2 != 0)) {
            setAnm_NUM__11daNpc_Ym1_cFii(this, 5, 1);
            this->unk890 = cLib_getRndValue<i>__Fii(0x5A, 0xB4);
        }
        return 1;
    }
    if ((u8) this->unk8A2 != 0) {
        this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
    }
    if (cLib_calcTimer<s>__FPs(&this->unk88E) != 0) {
        this->unk8AF = 1;
        return 1;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk890) == 0) {
        setAnm_NUM__11daNpc_Ym1_cFii(this, 6, 1);
    }
    return 1;
}

/* daNpc_Ym1_c::SITwai (void) */
s32 SITwai__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk8A3 != 0) {
        if (chk_talk__11daNpc_Ym1_cFv(this) != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 5);
            this->unk8AF = 1;
            this->unk8A4 = 1;
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk8AF = 0;
    this->unk8A4 = 1;
    if ((u8) this->unk8A2 != 0) {
        this->unk88E = cLib_getRndValue<i>__Fii(0xF, 0x1E);
    }
    if (cLib_calcTimer<s>__FPs(&this->unk88E) != 0) {
        this->unk8AF = 1;
    }
    sp8 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
    spC = g_dComIfG_gameInfo.unk5B4C->unk1FC;
    sp10 = g_dComIfG_gameInfo.unk5B4C->unk200;
    if (dNpc_chkAttn__FP10fopAc_ac_c4cXyzfffb((fopAc_ac_c *) this, (cXyz) &sp8, @4213.unk9C, @4213.unk14, @4213.unk94, (s8) this->unk8AF == 1) != 0) {
        return 1;
    }
    this->unk8AF = 0;
    return 1;
}

/* daNpc_Ym1_c::wait_action1 (void *) */
s32 wait_action1__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8B2;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ym1_cFSc(this, 1);
        this->unk8A5 = 1;
        this->unk8B2 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk8A2 = chkAttention__11daNpc_Ym1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk888 = kari_1__11daNpc_Ym1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk888 = wait_1__11daNpc_Ym1_cFv(this);
            break;
        case 4:                                     /* switch 2 */
            this->unk888 = wait_2__11daNpc_Ym1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk888 = turn_1__11daNpc_Ym1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk888 = talk_1__11daNpc_Ym1_cFv(this);
            break;
        }
        break;
    }
    return 1;
}

/* daNpc_Ym1_c::wait_action2 (void *) */
s32 wait_action2__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8B2;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        if ((s32) this->unk8B1 == 5) {
            setStt__11daNpc_Ym1_cFSc(this, 0xA);
            this->unk8A5 = 1;
            this->unk8B2 += 1;
        } else {
            setStt__11daNpc_Ym1_cFSc(this, 8);
            this->unk8A5 = 1;
            this->unk8B2 += 1;
        }
        break;
    default:                                        /* switch 1 */
        this->unk8A2 = chkAttention__11daNpc_Ym1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 8:                                     /* switch 2 */
            this->unk888 = wait_2__11daNpc_Ym1_cFv(this);
            break;
        case 10:                                    /* switch 2 */
            this->unk888 = wait_2__11daNpc_Ym1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk888 = talk_1__11daNpc_Ym1_cFv(this);
            break;
        }
        break;
    }
    return 1;
}

/* daNpc_Ym1_c::wait_action3 (void *) */
s32 wait_action3__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    u8 temp_r0;
    u8 temp_r0_2;

    sp8 = 0.0f;
    spC = 0.0f;
    sp10 = 110.0f;
    temp_r0 = this->unk8B2;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        if (chk_BlackPig__11daNpc_Ym1_cFv(this) != 0) {
            setStt__11daNpc_Ym1_cFSc(this, 8);
            this->unk8A5 = 1;
            this->unk8B2 += 1;
        } else {
            setStt__11daNpc_Ym1_cFSc(this, 6);
            PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
            mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk848);
            PSMTXMultVec(&now__14mDoMtx_stack_c, &sp8, (cXyz *) &this->unk864);
            this->unk8B2 += 1;
        }
        break;
    default:                                        /* switch 1 */
        this->unk8A2 = chkAttention__11daNpc_Ym1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 6:                                     /* switch 2 */
            this->unk888 = NBTwai__11daNpc_Ym1_cFv(this);
            break;
        case 7:                                     /* switch 2 */
            this->unk888 = talk_1__11daNpc_Ym1_cFv(this);
            break;
        case 8:                                     /* switch 2 */
            this->unk888 = wait_2__11daNpc_Ym1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk888 = talk_1__11daNpc_Ym1_cFv(this);
            break;
        }
        break;
    }
    return 1;
}

/* daNpc_Ym1_c::wait_action4 (void *) */
s32 wait_action4__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8B2;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ym1_cFSc(this, 0xB);
        this->unk8B2 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk8A2 = chkAttention__11daNpc_Ym1_cFv(this);
        temp_r0_2 = this->mSttNum;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 11:                                    /* switch 2 */
            this->unk888 = SITwai__11daNpc_Ym1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk888 = talk_1__11daNpc_Ym1_cFv(this);
            break;
        }
        break;
    }
    return 1;
}

/* daNpc_Ym1_c::demo_action1 (void *) */
s32 demo_action1__11daNpc_Ym1_cFPv(daNpc_Ym1_c *this, void *arg0) {
    u8 temp_r4;

    temp_r4 = this->unk8B2;
    switch ((s8) temp_r4) {                         /* irregular */
    case 9:
        break;
    case 0:
        this->unk8B2 = temp_r4 + 1;
        break;
    default:
        this->unk8A2 = chkAttention__11daNpc_Ym1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ym1_c::demo (void) */
u8 demo__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp8;
    J3DAnmTexPattern *temp_r3_2;
    dDemo_actor_c *temp_r3;
    s16 temp_r4;

    if ((u8) this->unk1C0 == 0) {
        if ((u8) this->unk8A6 != 0) {
            this->unk8A6 = 0;
        }
    } else {
        if ((u8) this->unk8A6 == 0) {
            this->unk8A6 = 1;
            this->unk89F = 0;
            this->unk292 = 0;
            this->unk290 = 0;
            this->unk296 = 0;
            this->unk294 = 0;
        }
        temp_r3 = getActor__14dDemo_object_cFUc(g_dComIfG_gameInfo.unk5AC8 + 0x20, this->unk1C0);
        if ((J3DAnmTexPattern *) this->unk6E8 != NULL) {
            this->unk6F4 += 1;
            temp_r4 = this->unk6E8->unk6;
            if ((s32) this->unk6F4 >= temp_r4) {
                this->unk6F4 = (u8) temp_r4;
            }
        }
        temp_r3_2 = getP_BtpData__13dDemo_actor_cFPCc(temp_r3, &this->unk6D4.unk0);
        if (temp_r3_2 != NULL) {
            sp8 = 0;
            init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk6E0, this->unk6DC->unk4, temp_r3_2, 1, 0, 1.0f, 0, -1, 1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
            this->unk8AA = 1;
            this->unk6F4 = 0;
        }
        dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc((fopAc_ac_c *) this, 0x6AU, this->unk330, &this->unk6D4.unk0, 0, NULL, 0U, 0);
    }
    return this->unk8A6;
}

/* daNpc_Ym1_c::shadowDraw (void) */
void shadowDraw__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f3;
    u32 temp_r4;
    void **temp_r5;

    temp_f3 = this->unk1FC;
    sp8 = this->unk1F8;
    spC = @4213.unk64 + temp_f3;
    sp10 = this->unk200;
    this->unk6D8 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unk6D8, 1, this->unk330->unk50, (cXyz *) &sp8, @4213.unkAC, @4213.unkB0, temp_f3, this->unk3C8, &this->unk41C, &this->unk10C, 0, @4213.unk54, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    temp_r4 = this->unk6D8;
    if (temp_r4 != 0) {
        temp_r5 = this->unk6D0;
        if (temp_r5 != NULL) {
            addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, temp_r4, (J3DModel *) temp_r5);
        }
        addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, this->unk6D8, (J3DModel *) this->unk6DC);
    }
}

/* daNpc_Ym1_c::_draw (void) */
s32 _draw__11daNpc_Ym1_cFv(daNpc_Ym1_c *this, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r28;
    J3DModelData *temp_r31;
    u8 temp_r0;
    u8 temp_r0_2;
    void **temp_r30;
    void **temp_r4;

    temp_r30 = this->unk6DC;
    temp_r31 = temp_r30->unk4;
    temp_r28 = this->unk330->unk50;
    if (((u8) this->unk89E != 0) || ((u8) this->unk8A0 != 0)) {
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &this->unk1F8, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r28, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r30, &this->unk10C);
    temp_r0 = this->unk8B0;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 1:                                         /* switch 1 */
        entryDL__14mDoExt_McaMorfFv(this->unk330);
        break;
    case 2:                                         /* switch 1 */
        entryDL__14mDoExt_McaMorfFP16J3DMaterialTable(this->unk330, getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(&this->unk6D4, (s8 *)0xD, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */)));
        break;
    }
    entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->unk6E0, temp_r31, (s16) this->unk6F4);
    mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) temp_r30);
    removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r31->unk58, this->unk6E8);
    temp_r4 = this->unk6D0;
    if (temp_r4 != NULL) {
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r4, &this->unk10C);
        mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) this->unk6D0);
    }
    shadowDraw__11daNpc_Ym1_cFv(this);
    temp_r0_2 = this->unk8B0;
    switch ((s8) temp_r0_2) {                       /* switch 2; irregular */
    case 1:                                         /* switch 2 */
        dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x50U, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
        break;
    case 2:                                         /* switch 2 */
        dSnap_RegistFig__FUcP10fopAc_ac_cRC3Vecsfff(0x4EU, (fopAc_ac_c *) this, &this->unk260, this->unk20E, 1.0f, 1.0f, 1.0f);
        break;
    }
    if ((u8) l_HIO[(s8) this->unk8B0].unk-4 != 0) {
        sp8 = this->unk1F8;
        spC = this->unk1FC;
        sp10 = this->unk200;
        spC = this->unk264;
    }
    this->unk89C = this->unk89B;
    this->unk89B = 0;
    return 1;
}

/* daNpc_Ym1_c::_execute (void) */
s32 _execute__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 spC;
    s32 sp8;
    dBgS *temp_r30;
    s32 var_r4;
    struct _struct_l_HIO_0x2C *temp_r10;

    if ((u8) this->unk8A1 == 0) {
        this->unk834 = this->unk1F8;
        this->unk838 = this->unk1FC;
        this->unk83C = this->unk200;
        this->unk840 = this->unk204;
        this->unk842 = this->unk206;
        this->unk844 = this->unk208;
        this->unk8A1 = 1;
    }
    if (chk_nbt_attn__11daNpc_Ym1_cFv(this) != 0) {
        sp8 = -0x38E0;
        spC = (s32) l_HIO[(s8) this->unk8B0].unk-C;
        setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, 0, 0, 0, 0, 0x2000, 0x38E0, -0x2000, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    } else {
        temp_r10 = &l_HIO[(s8) this->unk8B0];
        sp8 = (s32) temp_r10->unk-16;
        spC = (s32) temp_r10->unk-C;
        setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, temp_r10->unk-14, temp_r10->unk-12, temp_r10->unk-10, temp_r10->unk-E, temp_r10->unk-1C, temp_r10->unk-1A, temp_r10->unk-18, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    }
    if (((u8) this->unk89E != 0) && ((u8) this->unk1C0 == 0)) {
        return 1;
    }
    checkOrder__11daNpc_Ym1_cFv(this);
    if (demo__11daNpc_Ym1_cFv(this) == 0) {
        var_r4 = -1;
        if (((u8) g_dComIfG_gameInfo.unk529A != 0) && ((u16) this->unkF8 != 1)) {
            var_r4 = isEventEntry__11daNpc_Ym1_cFv(this);
        }
        if (var_r4 >= 0) {
            event_proc__11daNpc_Ym1_cFi(this, var_r4);
        } else {
            __ptmf_scall(this, 0);
        }
        lookBack__11daNpc_Ym1_cFv(this);
        if ((s32) this->unk8B1 != 1) {
            fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, (cXyz *) &this->unk538);
            CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo + 0x12A0);
        }
        play_animation__11daNpc_Ym1_cFv(this);
    } else {
        this->unk89E = 0;
    }
    eventOrder__11daNpc_Ym1_cFv(this);
    this->unk846 = this->unk204;
    this->unk848 = this->unk206;
    this->unk84A = this->unk208;
    if ((u8) this->unk89F == 0) {
        this->unk20C = this->unk204;
        this->unk20E = this->unk206;
        this->unk210 = this->unk208;
    }
    temp_r30 = &g_dComIfG_gameInfo + 0x12A0;
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r30, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r30, &this->unk41C);
    setMtx__11daNpc_Ym1_cFb(this, 0U);
    set_cutGrass__11daNpc_Ym1_cFv(this);
    if ((s8) this->unk8B1 == 0) {
        SetAreaChk__12dCcMassS_MngFP8cCcD_ObjUcPFP10fopAc_ac_cP4cXyzUl_v(&g_dComIfG_gameInfo + 0x4EF8, (cCcD_Obj *) &this->unk704, 3U, area_check__FP10fopAc_ac_cP4cXyzUl);
    }
    if ((u8) this->unk8A6 == 0) {
        set_collision_sp__11daNpc_Ym1_cFv(this);
    }
    return 1;
}

/* daNpc_Ym1_c::_delete (void) */
s32 _delete__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    mDoExt_McaMorf *temp_r3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk6C4, &this->unk6D4.unk0);
    if ((u32) this->unkF0 != 0U) {
        temp_r3 = this->unk330;
        if (temp_r3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3);
        }
    }
    return 1;
}

/* daNpc_Ym1_c::_create (void) */
s32 _create__11daNpc_Ym1_cFv(daNpc_Ym1_c *this, ? arg_sp0) {
    s32 temp_r3;
    s32 var_r3;

    if (!(this->unk1C8 & 8)) {
        if (this != NULL) {
            __ct__11daNpc_Ym1_cFv(this);
        }
        this->unk1C8 |= 8;
    }
    if (decideType__11daNpc_Ym1_cFi(this, (s32) (u8) this->unkB0) == 0) {
        return 5;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk6C4, &this->unk6D4.unk0);
    this->unk89D = temp_r3 == 4;
    if ((u8) this->unk89D == 0) {
        return temp_r3;
    }
    if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) CheckCreateHeap__FP10fopAc_ac_c, a_siz_tbl$5560[(s8) this->unk8B0]) == 0) {
        return 5;
    }
    this->unk22C = &this->unk330->unk50->unk24;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, @4213.unkC0, @4213.unk74, @4213.unkC4, @4213.unk10, @4213.unk68, @4213.unkC8);
    var_r3 = 5;
    if (createInit__11daNpc_Ym1_cFv(this) != 0) {
        var_r3 = temp_r3;
    }
    return var_r3;
}

/* daNpc_Ym1_c::bodyCreateHeap (void) */
s32 bodyCreateHeap__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
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

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(&this->unk6D4, (s8 *)0xA, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 8), (s8 *)0x971, (s32) (temp_r4 + 0x31), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ym1";
        OSPanic(temp_r3_2 + 8, 0x971, temp_r3_2 + 0x23);
    }
    temp_r3_3 = __nw__FUl(0xB4U);
    var_r0 = temp_r3_3;
    if (var_r0 != NULL) {
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x15021222;
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
    temp_r3_5 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ym1" + 0x40);
    this->unk6CC = temp_r3_5;
    if ((s8) this->unk6CC < 0) {
        temp_r4_2 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_5), (u32) (temp_r4_2 + 8), (s8 *)0x97F, (s32) (temp_r4_2 + 0x45), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_6 = "Ym1";
        OSPanic(temp_r3_6 + 8, 0x97F, temp_r3_6 + 0x23);
    }
    temp_r3_7 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ym1" + 0x58);
    this->unk6CD = temp_r3_7;
    if ((s8) this->unk6CD < 0) {
        temp_r4_3 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_7), (u32) (temp_r4_3 + 8), (s8 *)0x981, (s32) (temp_r4_3 + 0x61), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_8 = "Ym1";
        OSPanic(temp_r3_8 + 8, 0x981, temp_r3_8 + 0x23);
    }
    temp_r3_9 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ym1" + 0x76);
    this->unk6CE = temp_r3_9;
    if ((s8) this->unk6CE < 0) {
        temp_r4_4 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_9), (u32) (temp_r4_4 + 8), (s8 *)0x983, (s32) (temp_r4_4 + 0x7C), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_10 = "Ym1";
        OSPanic(temp_r3_10 + 8, 0x983, temp_r3_10 + 0x23);
    }
    temp_r3_11 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ym1" + 0x91);
    this->unk6CF = temp_r3_11;
    if ((s8) this->unk6CF < 0) {
        temp_r4_5 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_11), (u32) (temp_r4_5 + 8), (s8 *)0x985, (s32) (temp_r4_5 + 0x97), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_12 = "Ym1";
        OSPanic(temp_r3_12 + 8, 0x985, temp_r3_12 + 0x23);
    }
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CC * 4) & 0x3FFFC)))->unk8 = nodeCB_Head__FP7J3DNodei;
    (*(this->unk330->unk50->unk4->unk2C + (((s8) this->unk6CD * 4) & 0x3FFFC)))->unk8 = nodeCB_BackBone__FP7J3DNodei;
    this->unk330->unk50->unk14 = this;
    return 1;
}

/* daNpc_Ym1_c::headCreateHeap (void) */
s32 headCreateHeap__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(&this->unk6D4, (s8 *) (u16) a_hed_bdl_resID_tbl$5867[(s8) this->unk8B0], (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ym1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 8), (s8 *)0x9A4, (s32) (temp_r4 + 0x31), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ym1";
        OSPanic(temp_r3_2 + 8, 0x9A4, temp_r3_2 + 0x23);
    }
    this->unk6DC = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x15020022U);
    if ((void **) this->unk6DC == NULL) {
        return 0;
    }
    return init_texPttrnAnm__11daNpc_Ym1_cFScb(this, (s8) *(&a_tex_pttrn_num_tbl$5870 + (s8) this->unk8B0), 0) != 0;
}

/* daNpc_Ym1_c::itemCreateHeap (void) */
s32 itemCreateHeap__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    if ((s8) this->unk8B1 == 0) {
        temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci(&this->unk6D4, (s8 *)9, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        if (temp_r3 == NULL) {
            temp_r4 = "Ym1";
            showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 8), (s8 *)0x9C6, (s32) (temp_r4 + 0x31), M2C_ERROR(/* Read from unset register $r7 */));
            temp_r3_2 = "Ym1";
            OSPanic(temp_r3_2 + 8, 0x9C6, temp_r3_2 + 0x23);
        }
        this->unk6D0 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x11000022U);
        if ((void **) this->unk6D0 == NULL) {
            return 0;
        }
        goto block_6;
    }
    this->unk6D0 = NULL;
block_6:
    return 1;
}

/* daNpc_Ym1_c::CreateHeap (void) */
s32 CreateHeap__11daNpc_Ym1_cFv(daNpc_Ym1_c *this) {
    s32 sp8;

    if (bodyCreateHeap__11daNpc_Ym1_cFv(this) == 0) {
        return 0;
    }
    if (headCreateHeap__11daNpc_Ym1_cFv(this) == 0) {
        this->unk330 = NULL;
        return 0;
    }
    if (itemCreateHeap__11daNpc_Ym1_cFv(this) == 0) {
        this->unk330 = NULL;
        return 0;
    }
    SetWall__12dBgS_AcchCirFff(&this->unk4F8, 30.0f, 60.0f);
    sp8 = 0;
    Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&this->unk334, (cXyz *) &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, &this->unk4F8, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    return 1;
}

/* daNpc_Ym1_Create (fopAc_ac_c *) */
void daNpc_Ym1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _create__11daNpc_Ym1_cFv((daNpc_Ym1_c *) arg0);
}

/* daNpc_Ym1_Delete (daNpc_Ym1_c *) */
void daNpc_Ym1_Delete__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0) {
    _delete__11daNpc_Ym1_cFv(arg0);
}

/* daNpc_Ym1_Execute (daNpc_Ym1_c *) */
void daNpc_Ym1_Execute__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0) {
    _execute__11daNpc_Ym1_cFv(arg0);
}

/* daNpc_Ym1_Draw (daNpc_Ym1_c *) */
void daNpc_Ym1_Draw__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0) {
    _draw__11daNpc_Ym1_cFv(arg0);
}

/* daNpc_Ym1_IsDelete (daNpc_Ym1_c *) */
s32 daNpc_Ym1_IsDelete__FP11daNpc_Ym1_c(daNpc_Ym1_c *arg0) {
    return 1;
}

/* d_a_npc_ym1_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_ym1_cpp(void) {
    __ct__15daNpc_Ym1_HIO_cFv((daNpc_Ym1_HIO_c *) l_HIO);
    __register_global_object(__dt__15daNpc_Ym1_HIO_cFv, @4164);
}