// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_ko1 (140 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_ko1\asm\d\actor\d_a_npc_ko1.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ J3DModelData *unk4;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0xC];                      /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x14 */ daNpc_Ko1_c *unk14;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
    /* 0x18 */ char pad18[0xC];                     /* maybe part of unk14[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x18 (receipt f_op_actor.h) */
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

typedef struct JUTAssertion {
    /* 0x000 */ char pad0[6];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x006 */ s16 unk6;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6 (receipt f_op_actor.h) */
    /* 0x008 */ s16 unk8;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x00A */ char padA[0x1E];                    /* maybe part of unk8[0x10]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA (receipt f_op_actor.h) */
    /* 0x028 */ u16 unk28;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x02A */ char pad2A[0x2A];                   /* maybe part of unk28[0x16]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2A (receipt f_op_actor.h) */
    /* 0x054 */ JUTNameTab *unk54;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x058 */ char pad58[0x16C];                  /* maybe part of unk54[0x5C]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ char pad1C8[0x30];                  /* maybe part of unk1C4[0xD]? */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[0x60];                  /* maybe part of unk200[0x19]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x264 */ f32 unk264;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x4 (receipt f_op_actor.h) */
    /* 0x268 */ char pad268[0x43C];                 /* maybe part of unk264[0x110]? */  /* = fopAc_ac_c::cXyz eyePos @0x260 +0x8 (receipt f_op_actor.h) */
    /* 0x6A4 */ s32 unk6A4;                         /* inferred */  /* [RESOLVED: alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands] */
    /* 0x6A8 */ char pad6A8[0xC];                   /* maybe part of unk6A4[4]? */  /* [RESOLVED: alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands] */
    /* 0x6B4 */ f32 unk6B4;                         /* inferred */  /* [RESOLVED: alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands] */
    /* 0x6B8 */ u16 unk6B8;                         /* inferred */  /* [RESOLVED: alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands] */
    /* 0x6BA */ char pad6BA[2];  /* [RESOLVED: alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands] */
    /* 0x6BC */ u8 m_manzai_stt;                          /* inferred */  /* [NAMED: manzai sync state -- chk_manzai_1 sets a partner to 1=requested (draft:1612), registers the group and sets own to 2=ready when all partners read 2 (draft:1602,1673); anmAtr treats ==2 && not-current-speaker as manzai-controlled (draft:1493); cut procs step 1->2 (draft:3252,3286)] */
} JUTAssertion;                                     /* size >= 0x6BD */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0x4 */

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

struct __vt__14mDoHIO_entry_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoHIO_entry_c *, s16);
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

struct __vt__15daNpc_Ko1_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Ko1_HIO_c *, s16);
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

struct __vt__20daNpc_Ko1_childHIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_Ko1_childHIO_c *, s16);
};                                                  /* size = 0xC */

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

struct _struct_a_anm_prm_tbl$4682_0x14 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

struct _struct_a_anm_prm_tbl$4689_0x14 {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

struct _struct_a_anm_prm_tbl$4763_0x14 {
    /* 0x00 */ char pad0[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ u8 unk1;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x02 */ char pad2[0x12];                     /* maybe part of unk1[0x13]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
};                                                  /* size = 0x14 */

struct _struct_l_HIO_0x60 {
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
    /* 0x2C */ s16 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x2E */ char pad2E[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2E (receipt f_op_actor.h) */
    /* 0x30 */ f32 unk30;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x34 */ f32 unk34;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x38 */ f32 unk38;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
    /* 0x3C */ f32 unk3C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3C (receipt f_op_actor.h) */
    /* 0x40 */ f32 unk40;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x40 (receipt f_op_actor.h) */
    /* 0x44 */ f32 unk44;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x44 (receipt f_op_actor.h) */
    /* 0x48 */ f32 unk48;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x48 (receipt f_op_actor.h) */
    /* 0x4C */ f32 unk4C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4C (receipt f_op_actor.h) */
    /* 0x50 */ f32 unk50;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ f32 unk54;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x58 */ f32 unk58;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x5C */ f32 unk5C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5C (receipt f_op_actor.h) */
};                                                  /* size = 0x60 */

typedef struct cBgS_PolyInfo {
    /* 0x0 */ u16 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cBgS_PolyInfo;                                    /* size >= 0x2 */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ f32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ f32 unk8;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0xC */

typedef struct dNpc_EventCut_c {
    /* 0x0 */ s8 *unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_EventCut_c;                                  /* size >= 0x4 */

typedef struct dNpc_PathRun_c {
    /* 0x0 */ dPath *unk0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dNpc_PathRun_c;                                   /* size >= 0x4 */

typedef struct dPath {
    /* 0x0 */ char pad0[5];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x5 */ u8 unk5;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
} dPath;                                            /* size >= 0x6 */

typedef struct daNpc_Ko1_HIO_c {
    /* 0x0 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
    /* 0x4 */ s8 unk4;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x5 */ char pad5[3];                         /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5 (receipt f_op_actor.h) */
    /* 0x8 */ s32 unk8;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0xC */ ? unkC;                               /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0xC */ char padC[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Ko1_HIO_c;                                  /* size >= 0xD */

typedef struct J3DAnmTexPattern {
    /* 0x0 */ char pad0[6];  /* J3DAnmBase head: vt @0x0, u8 mAttribute @0x4, u8 @0x5 (receipt donor J3DAnimation.h) */
    /* 0x6 */ s16 unk6;      /* = J3DAnmBase::mFrameMax @0x6 (receipt donor J3DAnimation.h; frame-max reads draft plyTexPttrnAnm + anmNum paths) */
} J3DAnmTexPattern;                                 /* size >= 0x8; full class is J3DAnmBase-derived */

typedef struct daNpc_Ko1_c {
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
    /* 0x220 */ char pad220[4];  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x224 */ f32 unk224;                         /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 +0x4 (receipt f_op_actor.h) */
    /* 0x228 */ char pad228[4];  /* = fopAc_ac_c::cXyz speed @0x220 +0x8 (receipt f_op_actor.h) */
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
    /* 0x330 */ mDoExt_McaMorf *mpMorf;             /* inferred */  /* [NAMED-CONVENTION: body mDoExt_McaMorf*; anmNum_toResID feeds it (setAnm_anm draft:1340); setMorf(0) in createInit (draft:1165); donor-wide mpMorf idiom] */
    /* 0x334 */ dBgS_Acch unk334;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x334 */ char pad334[0x28];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x35C */ u32 unk35C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x360 */ char pad360[0x68];                  /* maybe part of unk35C[0x1B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3C8 */ f32 unk3C8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3CC */ char pad3CC[0x50];                  /* maybe part of unk3C8[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ cBgS_PolyInfo unk41C;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41C */ char pad41C[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x41E */ u16 unk41E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x420 */ char pad420[0xD0];                  /* maybe part of unk41E[0x69]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F0 */ f32 unk4F0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F4 */ char pad4F4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4F8 */ u16 unk4F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4FA */ u16 unk4FA;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x4FC */ char pad4FC[0x3C];                  /* maybe part of unk4FA[0x1F]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ dCcD_Stts mStts;                   /* inferred */  /* [NAMED-CONVENTION: dCcD_Stts::Init(&this->0x538,..) (draft:1162); donor-wide mStts idiom] */
    /* 0x538 */ char pad538[0x3C];  /* [RESOLVED: interior of mStts (dCcD_Stts)] */
    /* 0x574 */ dCcD_Cyl mCyl;                    /* inferred */  /* [NAMED-CONVENTION: dCcD_Cyl::Set(&this->0x574, &dNpc_cyl_src) (draft:1164); donor-wide mCyl idiom] */
    /* 0x574 */ char pad574[0x44];  /* [RESOLVED: interior of mCyl (dCcD_Cyl)] */
    /* 0x5B8 */ dCcD_Stts *unk5B8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5BC */ char pad5BC[0xE8];                  /* maybe part of unk5B8[0x3B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A4 */ u32 unk6A4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6A8 */ char pad6A8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6AC */ s32 unk6AC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B0 */ void *unk6B0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B4 */ u32 unk6B4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B8 */ u16 unk6B8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6BA */ char pad6BA[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6BC */ u8 m_manzai_stt;                          /* inferred */  /* [NAMED: manzai sync state -- chk_manzai_1 sets a partner to 1=requested (draft:1612), registers the group and sets own to 2=ready when all partners read 2 (draft:1602,1673); anmAtr treats ==2 && not-current-speaker as manzai-controlled (draft:1493); cut procs step 1->2 (draft:3252,3286)] */
    /* 0x6BD */ char pad6BD[3];                     /* maybe part of m_manzai_stt[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C0 */ void *unk6C0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ request_of_phase_process_class unk6C4; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6C4 */ char pad6C4[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CC */ u8 m_hed_jnt_num;                          /* inferred */  /* [NAMED: retail string a_cut_tbl$5860 'head/m_hed_jnt_num >= 0'; 1st in joint-search seq (draft:4647); feeds head-morf mtx (draft:1211); km1 m_head_jnt_num @0x6CC] */
    /* 0x6CD */ u8 m_bbone_jnt_num;                          /* inferred */  /* [NAMED: retail string 'backbone/m_bbone_jnt_num >= 0'; 2nd in seq (draft:4655); km1 m_backbone_jnt_num @0x6CD] */
    /* 0x6CE */ u8 m_armR2_jnt_num;                          /* inferred */  /* [NAMED: retail string 'armR2/m_armR2_jnt_num >= 0'; 3rd in seq (draft:4663); armR2 mtx copy (draft:1219)] */
    /* 0x6CF */ u8 m_hed_2_jnt_num;                          /* inferred */  /* [NAMED: retail string 'head2/m_hed_2_jnt_num >= 0'; 4th in seq; nodeCallBack_Hed compare (draft:837)] */
    /* 0x6D0 */ u8 m_bln_loc_jnt_num;                          /* inferred */  /* [NAMED: retail string 'balloon_loc/m_bln_loc_jnt_num >= 0'; 5th in seq; nodeCallBack_Bln compare (draft:863)] */
    /* 0x6D1 */ u8 m_bln_jnt_num;                          /* inferred */  /* [NAMED: retail string 'ko_balloon/m_bln_jnt_num >= 0'; 6th in seq; feeds balloon-morf mtx (draft:2743)] */
    /* 0x6D2 */ char pad6D2[2];                     /* maybe part of m_bln_jnt_num[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ mDoMtx_stack_c unk6D4;              /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D4 */ char pad6D4[0x30];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x704 */ mDoExt_McaMorf *mpBalloonMorf;             /* inferred */  /* [NAMED-CONVENTION: balloon mDoExt_McaMorf*; balloon_anmNum_toResID feeds it (draft:1357); balloon-jnt mtx source (draft:2743)] */
    /* 0x708 */ void **unk708;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70C */ u32 unk70C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x710 */ mDoExt_McaMorf *mpHedMorf;             /* inferred */  /* [NAMED-CONVENTION: head mDoExt_McaMorf*; headAnmNum_toResID feeds it (draft:1341); head-jnt mtx target (draft:1211); split-morf idiom (bgn2/bmd mpHeadMorf); 'hed' spelling from this TU's retail strings] */
    /* 0x714 */ J3DAnmTexPattern *m_hed_tex_pttrn;  /* [NAMED: retail assert 'm_hed_tex_pttrn != 0' fires at this member's null-check in setBtp (draft:1289); type CORRECTED from m2c's JUTAssertion misbind per History/Bridge review 2026-08-17 -- J3DAnmTexPattern* (fed to mDoExt_btpAnm::init; frame-max read via J3DAnmBase::mFrameMax @0x6)] */
    /* 0x718 */ mDoExt_btpAnm mBtpAnm;               /* inferred */  /* [NAMED-ANALOGY: km1 mDoExt_btpAnm mBtpAnm = tex_pttrn+4 (0x6D8->0x6DC); ko1 0x714->0x718 same stride; in-TU mDoExt_btpAnm::init(&this->0x718) (draft:1296)] */
    /* 0x718 */ char pad718[8];  /* [RESOLVED: interior of mBtpAnm (mDoExt_btpAnm)] */
    /* 0x720 */ J3DAnmTexPattern *unk720;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x724 */ char pad724[8];                     /* maybe part of unk720[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72C */ u8 mBtpFrame;                          /* inferred */  /* [NAMED-ANALOGY: km1 mBtpFrame = mBtpAnm+0x14 (0x6F0); ko1 0x718+0x14=0x72C; role: frame counter vs J3DAnmTexPattern frame max (plyTexPttrnAnm draft:1313)] */
    /* 0x72D */ char pad72D[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x72E */ s16 unk72E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x730 */ ? unk730;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x730 */ char pad730[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x734 */ s32 unk734;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x738 */ s32 unk738;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x73C */ dNpc_PathRun_c mPathRun;              /* inferred */  /* [NAMED: dNpc_PathRun_c; dNpc_PathRun_c::setInf(&this->0x73C,..) in createInit (draft:1107); .unk0 is mPath per d_npc.h @0x00] */
    /* 0x73C */ char pad73C[5];  /* [RESOLVED: interior of mPathRun (mPath @+0, field_0x04) (receipt d_npc.h)] */
    /* 0x741 */ u8 unk741;                          /* inferred */  /* [RESOLVED: dNpc_PathRun_c::mIdx @+0x05 of mPathRun (receipt d_npc.h, size 0x08)] */
    /* 0x742 */ u8 unk742;                          /* inferred */  /* [RESOLVED: dNpc_PathRun_c::mbDir @+0x06 of mPathRun (receipt d_npc.h)] */
    /* 0x743 */ char pad743[1];  /* [RESOLVED: dNpc_PathRun_c::field_0x07, tail pad of mPathRun (receipt d_npc.h)] */
    /* 0x744 */ dPath *unk744;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x748 */ dNpc_EventCut_c mEventCut;             /* inferred */  /* [NAMED-ANALOGY: dNpc_EventCut_c; setActorInfo2 (draft:1116); km1 mEventCut, same type] */
    /* 0x748 */ char pad748[0x6C];  /* [RESOLVED: interior of mEventCut (dNpc_EventCut_c)] */
    /* 0x7B4 */ u32 m_partner_id0;                         /* inferred */  /* [NAMED: fpc_ProcID array base @0x7B4, stride 4, walked by chk_manzai_1 (draft:1595) and searchByID'd (draft:1619); entries registered into the manzai group block g_dComIfG+0x5C28] */
    /* 0x7B8 */ u32 m_partner_id1;                         /* inferred */  /* [NAMED: second entry of the 0x7B4 partner-ID array; searchByID'd for the 3-way manzai cases (draft:1635,1660)] */
    /* 0x7BC */ u8 m_partner_num;                          /* inferred */  /* [NAMED: loop bound over the 0x7B4 partner-ID array in chk_manzai_1 (draft:1593-1594)] */
    /* 0x7BD */ char pad7BD[3];                     /* maybe part of m_partner_num[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C0 */ u32 unk7C0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C4 */ f32 unk7C4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7C8 */ f32 unk7C8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7CC */ f32 unk7CC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D0 */ s16 unk7D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D2 */ s16 unk7D2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D4 */ s16 unk7D4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D6 */ s16 unk7D6;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7D8 */ s16 unk7D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7DA */ s16 unk7DA;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7DC */ f32 unk7DC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E0 */ f32 unk7E0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ f32 unk7E4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E8 */ f32 unk7E8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7EC */ f32 unk7EC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F0 */ f32 unk7F0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7F4 */ Vec m_tgt_pos;                         /* inferred */  /* [NAMED: the setter is literally set_tgtPos (draft:2148); target of cLib_targetAngleY from current pos (draft:2239,2272); displacement-to-it measured for arrival in ko_movPass (draft:2201,2234,2267). cXyz spanning 0x7F4-0x7FF] */
    /* 0x7F4 */ char pad7F4[4];  /* [RESOLVED: m_tgt_pos.x (cXyz interior; the Vec decl above is the same storage)] */
    /* 0x7F8 */ f32 unk7F8;                         /* inferred */  /* [RESOLVED: m_tgt_pos.y (cXyz interior)] */
    /* 0x7FC */ f32 unk7FC;                         /* inferred */  /* [RESOLVED: m_tgt_pos.z (cXyz interior)] */
    /* 0x800 */ char pad800[0xC];                   /* maybe part of unk7FC[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x80C */ f32 unk80C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x810 */ f32 unk810;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x814 */ f32 unk814;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x818 */ Vec unk818;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x818 */ char pad818[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x81C */ f32 unk81C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x820 */ f32 unk820;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x824 */ f32 unk824;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x828 */ f32 unk828;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x82C */ f32 m_mov_spd;                         /* inferred */  /* [NAMED: chase TARGET for fopAc speedF in ko_clcMovSpd -- cLib_chaseF(&speedF@0x254, this->0x82C, this->0x834) (draft:2254)] */
    /* 0x830 */ f32 m_swm_spd_y;                         /* inferred */  /* [NAMED: chase target for speed.y (fopAc 0x224) while swimming -- cLib_chaseF(&speed.y, this->0x830, HIO step) in ko_clcSwmSpd (draft:2275)] */
    /* 0x834 */ f32 m_mov_spd_step;                         /* inferred */  /* [NAMED: chase STEP of the same cLib_chaseF call (draft:2254)] */
    /* 0x838 */ char pad838[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x83C */ f32 unk83C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x840 */ s16 unk840;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x842 */ s16 unk842;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x844 */ s16 unk844;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x846 */ char pad846[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x848 */ s32 unk848;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84C */ char pad84C[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84E */ s16 unk84E;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x850 */ s16 unk850;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x852 */ s16 unk852;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x854 */ s16 unk854;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x856 */ s16 unk856;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x858 */ s16 unk858;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x85A */ char pad85A[4];                     /* maybe part of unk858[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x85E */ s8 unk85E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x85F */ s8 unk85F;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x860 */ u8 unk860;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x861 */ s8 unk861;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x862 */ char pad862[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x863 */ u8 unk863;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x864 */ u8 unk864;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x865 */ u8 unk865;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x866 */ char pad866[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x867 */ u8 unk867;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x868 */ u8 unk868;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x869 */ u8 unk869;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86A */ u8 unk86A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86B */ u8 unk86B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86C */ u8 unk86C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86D */ u8 unk86D;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86E */ u8 unk86E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86F */ char pad86F[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x870 */ s32 unk870;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x874 */ u8 unk874;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x875 */ u8 unk875;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x876 */ u8 unk876;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x877 */ u8 unk877;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x878 */ dPa_rippleEcallBack unk878;         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x878 */ char pad878[0x10];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x888 */ f32 unk888;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x88C */ u32 unk88C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x890 */ u32 unk890;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x894 */ u32 unk894;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x898 */ u8 unk898;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x899 */ u8 unk899;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89A */ u8 unk89A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89B */ u8 unk89B;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89C */ u8 mAnmAtr;                          /* inferred */  /* [NAMED: chg_anmAtr compare-then-store (draft:1451-1454), 0xFF none-sentinel (draft:1501), indexes a_anm_prm_tbl$4763 in setAnm_ATR (draft:1482-1484)] */
    /* 0x89D */ u8 mAnmTag;                          /* inferred */  /* [NAMED: chg_anmTag/control_anmTag gate on it (draft:1406,1414; 0xFF reset draft:1418), set from the message anime-tag channel (draft:1507-1508). NOTE: ko1's tag channel is LIVE, unlike Ym1/Yw1/Aj1 whose chngAnmTag is retail-empty (README 17-blr finding)] */
    /* 0x89E */ u8 unk89E;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89F */ u8 unk89F;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A0 */ char pad8A0[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A1 */ u8 unk8A1;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A2 */ u8 mOrderType;                          /* inferred */  /* [NAMED: eventOrder selector (draft:1524; cases 1/2 -> fopAcM_orderSpeakEvent), cleared by checkOrder on event ack (draft:1543) -- ba1 SS257 template rule hand-applied, same shape as So 0xB70] */
    /* 0x8A3 */ u8 unk8A3;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A4 */ u8 unk8A4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A5 */ u8 unk8A5;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A6 */ u8 unk8A6;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A7 */ u8 m_act_no;                          /* inferred */  /* [NAMED: retail token 'ActNo' in a_staff_tbl$4419; indexes the 9-entry staff tbl (draft:1116) and the 9-way init_HNA_0..4/init_BOU_0..3 switch (draft:1120)] */
    /* 0x8A8 */ u8 unk8A8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x8A9 */ u8 unk8A9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_Ko1_c;                                      /* size >= 0x8AA */

typedef struct daNpc_Ko1_c::anm_prm_c {
    /* 0x00 */ u8 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x01 */ char pad1[3];                        /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x04 */ f32 unk4;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s32 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
} daNpc_Ko1_c::anm_prm_c;                           /* size >= 0x10 */

typedef struct daNpc_Ko1_childHIO_c {
    /* 0x0 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
} daNpc_Ko1_childHIO_c;                             /* size >= 0x4 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x58 */ J3DFrameCtrl unk58;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x58 */ char pad58[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x64 */ f32 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x10 */ char pad10[0xC];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x10 (receipt f_op_actor.h) */
    /* 0x1C */ f32 unk1C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1C (receipt f_op_actor.h) */
    /* 0x20 */ char pad20[0xC];                     /* maybe part of unk1C[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x20 (receipt f_op_actor.h) */
    /* 0x2C */ f32 unk2C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x30 */

s32 ActionTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
u32 ChkCoHit__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
s32 ChkPresentEnd__16dEvent_manager_cFv(dEvent_manager_c *this); /* extern */
? CrrPos__9dBgS_AcchFR4dBgS(dBgS_Acch *this, dBgS *arg0); /* extern */
void *GetCoHitObj__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
u32 GetMtrlSndId__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
s8 GetPolyColor__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
s8 GetRoomId__4dBgSFR13cBgS_PolyInfo(dBgS *this, cBgS_PolyInfo *arg0); /* extern */
cXyz *GetTriPla__4cBgSCFii(cBgS *this, s32 arg0, s32 arg1); /* extern */
f32 GroundCross__4cBgSFP11cBgS_GndChk(cBgS *this, cBgS_GndChk *arg0); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
u8 LineCross__4cBgSFP11cBgS_LinChk(cBgS *this, cBgS_LinChk *arg0); /* extern */
s32 LockonTarget__12dAttention_cFl(dAttention_c *this, s32 arg0); /* extern */
u8 LockonTruth__12dAttention_cFv(dAttention_c *this); /* extern */
? OSPanic(s8 *, ?, s8 *);                           /* extern */
? PSMTXCopy(mDoMtx_stack_c *, mDoMtx_stack_c *);    /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, f32 *, f32 *, s32, s32, f32); /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
f32 PSVECSquareMag(f32 *);                          /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c(dBgS_LinChk *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
? __construct_array(? *, void *(*)(daNpc_Ko1_childHIO_c *), void *(*)(daNpc_Ko1_childHIO_c *, s16), ?, ?); /* extern */
void *__ct__11cBgS_GndChkFv(cBgS_GndChk *this);     /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
void *__dt__8cBgS_ChkFv(cBgS_Chk *this, s16 destroyFlag); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
s32 __ptmf_cmpr(s32);                               /* extern */
? __ptmf_scall(daNpc_Ko1_c *, s32);                 /* extern */
s32 __ptmf_test(s32);                               /* extern */
? __register_global_object(void *(*)(daNpc_Ko1_HIO_c *, s16), void *(*)(cXyz *, s16), void *); /* extern */
? addReal__21dDlst_shadowControl_cFUlP8J3DModel(dDlst_shadowControl_c *this, u32 arg0, J3DModel *arg1); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
? cLib_chaseAngleS__FPsss(s16 *arg0, s16 arg1, s16 arg2); /* extern */
? cLib_chaseF__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
s32 cLib_distanceAngleS__Fss(s16 arg0, s16 arg1);   /* extern */
s16 cLib_targetAngleY__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
s16 cM_atan2s__Fff(f32 arg0, f32 arg1);             /* extern */
f32 cM_rndF__Ff(f32 arg0);                          /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
s32 checkPass__12J3DFrameCtrlFf(J3DFrameCtrl *this, f32 arg0); /* extern */
u8 chkPointPass__14dNpc_PathRun_cF4cXyzb(dNpc_PathRun_c *this, cXyz arg0, u8 arg1); /* extern */
? ct__11cBgS_LinChkFv(cBgS_LinChk *this);           /* extern */
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
? end__19dPa_rippleEcallBackFv(dPa_rippleEcallBack *this); /* extern */
? entryDL__14mDoExt_McaMorfFP16J3DMaterialTable(mDoExt_McaMorf *this, J3DMaterialTable *arg0); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
? fopAcM_SearchByID__FUiPP10fopAc_ac_c(u32 arg0, fopAc_ac_c **arg1); /* extern */
? fopAcM_delete__FP10fopAc_ac_c(fopAc_ac_c *arg0);  /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
s32 fopAc_IsActor__FPv(void *arg0);                 /* extern */
s32 fopMsgM_messageSet__FUlP10fopAc_ac_c(u32 arg0, fopAc_ac_c *arg1); /* extern */
? fpcEx_Search__FPFPvPv_PvPv(void *(*arg0)(void *, void *), void *arg1); /* extern */
dDemo_actor_c *getActor__14dDemo_object_cFUc(dDemo_object_c *this, u8 arg0); /* extern */
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
? memcpy(void *, void *, ?);                        /* extern */
u8 nextIdxAuto__14dNpc_PathRun_cFv(dNpc_PathRun_c *this); /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? outprod__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);    /* extern */
u8 play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setCollision__12fopNpc_npc_cFff(fopNpc_npc_c *this, f32 arg0, f32 arg1); /* extern */
? setInfDrct__14dNpc_PathRun_cFP5dPath(dNpc_PathRun_c *this, dPath *arg0); /* extern */
? setInf__14dNpc_PathRun_cFUcScUc(dNpc_PathRun_c *this, u8 arg0, s8 arg1, u8 arg2); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setMorf__14mDoExt_McaMorfFf(mDoExt_McaMorf *this, f32 arg0); /* extern */
? setNearPathIndx__14dNpc_PathRun_cFP4cXyzf(dNpc_PathRun_c *this, cXyz *arg0, f32 arg1); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
u32 set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(dPa_control_c *this, u8 arg0, u16 arg1, cXyz *arg2, csXyz *arg3, cXyz *arg4, u8 arg5, dPa_levelEcallBack *arg6, s8 arg7, _GXColor *arg8, _GXColor *arg9, cXyz *arg10); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
? stopZelAnime__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
void *@100@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *@12@__dt__8dBgS_ChkFv(dBgS_Chk *this, s16 destroyFlag); /* static */
void *@20@__dt__11cBgS_GndChkFv(cBgS_GndChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11cBgS_LinChkFv(cBgS_LinChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@20@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *@64@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@76@__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *@88@__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
s32 CreateHeap__11daNpc_Ko1_cFv(daNpc_Ko1_c *this); /* static */
void *__ct__11daNpc_Ko1_cFv(daNpc_Ko1_c *this);     /* static */
void *__dt__11cBgS_GndChkFv(cBgS_GndChk *this, s16 destroyFlag); /* static */
void *__dt__11cBgS_LinChkFv(cBgS_LinChk *this, s16 destroyFlag); /* static */
void *__dt__11dBgS_GndChkFv(dBgS_GndChk *this, s16 destroyFlag); /* static */
void *__dt__11dBgS_LinChkFv(dBgS_LinChk *this, s16 destroyFlag); /* static */
void *__dt__13cBgS_PolyInfoFv(cBgS_PolyInfo *this, s16 destroyFlag); /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__15cBgS_GrpPassChkFv(cBgS_GrpPassChk *this, s16 destroyFlag); /* static */
void *__dt__15dBgS_GrpPassChkFv(dBgS_GrpPassChk *this, s16 destroyFlag); /* static */
void *__dt__15daNpc_Ko1_HIO_cFv(daNpc_Ko1_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__16cBgS_PolyPassChkFv(cBgS_PolyPassChk *this, s16 destroyFlag); /* static */
void *__dt__16dBgS_PolyPassChkFv(dBgS_PolyPassChk *this, s16 destroyFlag); /* static */
void *__dt__20daNpc_Ko1_childHIO_cFv(daNpc_Ko1_childHIO_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
void *__dt__8cM3dGLinFv(cM3dGLin *this, s16 destroyFlag); /* static */
void *__dt__8dBgS_ChkFv(dBgS_Chk *this, s16 destroyFlag); /* static */
void anmAtr__11daNpc_Ko1_cFUs(daNpc_Ko1_c *this, u16 arg0); /* static */
s8 bitCount__11daNpc_Ko1_cFUc(daNpc_Ko1_c *this, u8 arg0); /* static */
s16 cLib_calcTimer<s>__FPs(s16 *arg0);              /* static */
s16 cLib_getRndValue<i>__Fii(s32 arg0, s32 arg1);   /* static */
void daNpc_Ko1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
void daNpc_Ko1_Delete__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0); /* static */
void daNpc_Ko1_Draw__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0); /* static */
void daNpc_Ko1_Execute__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0); /* static */
s32 daNpc_Ko1_IsDelete__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0); /* static */
s32 getMsg__11daNpc_Ko1_cFv(daNpc_Ko1_c *this);     /* static */
s32 hana_action1__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 hana_action2__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 hana_action3__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 hana_action4__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 hana_action5__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 next_msgStatus__11daNpc_Ko1_cFPUl(daNpc_Ko1_c *this, u32 *arg0); /* static */
void nodeBlnControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void nodeHedControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void nodeKo1Control__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
void plyTexPttrnAnm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this); /* static */
s32 routeCheck__11daNpc_Ko1_cFfPs(daNpc_Ko1_c *this, f32 arg0, s16 *arg1); /* static */
JUTAssertion *searchByID__11daNpc_Ko1_cFUi(daNpc_Ko1_c *this, u32 arg0); /* static */
void setAnm_ATR__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0); /* static */
void setAttention__11daNpc_Ko1_cFb(daNpc_Ko1_c *this, u8 arg0); /* static */
void setMtx__11daNpc_Ko1_cFb(daNpc_Ko1_c *this, u8 arg0); /* static */
void setPrtcl_Hamon__11daNpc_Ko1_cFff(daNpc_Ko1_c *this, f32 arg0, f32 arg1); /* static */
s32 set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(daNpc_Ko1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0); /* static */
s32 wait_action1__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 wait_action2__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 wait_action3__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
s32 wait_action4__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0); /* static */
void *@4167(cXyz *this, s16 destroyFlag);           /* static */
extern dCcD_SrcCyl dNpc_cyl_src;
extern s32 g_Counter;
extern ? g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? j3dSys;
extern s32 jmaCosTable;
extern s32 jmaSinShift;
extern s32 jmaSinTable;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? a_prm_tbl$4154;                            /* unable to generate initializer: unknown type */
static ? @4306;                                     /* unable to generate initializer: unknown type */
static ? @4321;                                     /* unable to generate initializer: unknown type */
static ? @4333;                                     /* unable to generate initializer: unknown type */
static ? @4347;                                     /* unable to generate initializer: unknown type */
static ? @4360;                                     /* unable to generate initializer: unknown type */
static ? @4371;                                     /* unable to generate initializer: unknown type */
static ? @4383;                                     /* unable to generate initializer: unknown type */
static ? @4397;                                     /* unable to generate initializer: unknown type */
static ? @4410;                                     /* unable to generate initializer: unknown type */
static s8 *a_staff_tbl$4419[9] = {
    "Ko1",
    "Ko1",
    "Ko1",
    "Ko1",
    "Ko1",
    "Ko2\0Ko\0d_a_npc_ko1.cpp\0m_hed_tex_pttrn != 0\0Halt\00 != a_actor\0ActNo",
    "Ko2\0Ko\0d_a_npc_ko1.cpp\0m_hed_tex_pttrn != 0\0Halt\00 != a_actor\0ActNo",
    "Ko2\0Ko\0d_a_npc_ko1.cpp\0m_hed_tex_pttrn != 0\0Halt\00 != a_actor\0ActNo",
    "Ko2\0Ko\0d_a_npc_ko1.cpp\0m_hed_tex_pttrn != 0\0Halt\00 != a_actor\0ActNo",
};
static struct _struct_a_anm_prm_tbl$4677_0x14 a_anm_prm_tbl$4677[2]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4682_0x14 a_anm_prm_tbl$4682[0xE]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4689_0x14 a_anm_prm_tbl$4689[0x1E]; /* unable to generate initializer: non-zero padding */
static struct _struct_a_anm_prm_tbl$4763_0x14 a_anm_prm_tbl$4763[0xD]; /* unable to generate initializer: non-zero padding */
static s8 *a_cut_tbl$5860 = "ACTION\0a_partner != 0\0a_mdl_dat != 0\0head\0m_hed_jnt_num >= 0\0backbone\0m_bbone_jnt_num >= 0\0armR2\0m_armR2_jnt_num >= 0\0head2\0m_hed_2_jnt_num >= 0\0balloon_loc\0m_bln_loc_jnt_num >= 0\0ko_balloon\0m_bln_jnt_num >= 0";
static u32 a_size_tbl$6977[2] = { 0x272E0, 0x272E0 };
static s32 a_hed_bdl_resID_tbl$7263[2] = { 0x1C, 0x1D };
static s32 a_hed_bck_resID_tbl$7264[2] = { 0xA, 0x25 };
static ? @3569;
static struct _struct_l_HIO_0x60 l_HIO[2];
static void *l_check_inf[0x14];
static s32 l_check_wrk;
static s32 a_bck_resID_tbl$4577[0xE] = {
    0x16,
    0xF,
    0x18,
    0x15,
    0x14,
    0xD,
    0x17,
    0x10,
    0x11,
    0x12,
    0x13,
    0xE,
    0xC,
    0x26,
}; /* const */
static s32 a_bck_resID_tbl$4582[0xE] = { 0xA, 5, 0xB, 9, 8, 3, 0xA, 0xA, 0xA, 6, 7, 4, 2, 0xA }; /* const */
static s32 a_bck_resID_tbl$4589[0xE] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; /* const */
static daNpc_Ko1_c *a_btp_resID_tbl$4594[4] = {
    (daNpc_Ko1_c *)0x1F,
    (daNpc_Ko1_c *)0x22,
    (daNpc_Ko1_c *)0x20,
    (daNpc_Ko1_c *)0x21,
}; /* const */
static ? @6522;                                     /* unable to generate initializer: unknown type; const */
static u16 @7337 = 0;                               /* const */

/* daNpc_Ko1_childHIO_c::daNpc_Ko1_childHIO_c (void) */
void __ct__20daNpc_Ko1_childHIO_cFv(daNpc_Ko1_childHIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__20daNpc_Ko1_childHIO_c;
}

/* daNpc_Ko1_HIO_c::daNpc_Ko1_HIO_c (void) */
daNpc_Ko1_HIO_c *__ct__15daNpc_Ko1_HIO_cFv(daNpc_Ko1_HIO_c *this, ? arg_sp0) {
    s32 var_r28;
    s32 var_r30;
    s32 var_r31;

    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__15daNpc_Ko1_HIO_c;
    __construct_array(&this->unkC, __ct__20daNpc_Ko1_childHIO_cFv, __dt__20daNpc_Ko1_childHIO_cFv, 0x60, 2);
    var_r28 = 0;
    var_r31 = 0;
    var_r30 = 0;
    do {
        *(this + (var_r30 + 0x68)) = var_r28;
        memcpy(this + (var_r30 + 0x10), &a_prm_tbl$4154 + var_r31, 0x58);
        var_r28 += 1;
        var_r31 += 0x58;
        var_r30 += 0x60;
    } while (var_r28 < 2);
    this->unk4 = -1;
    this->unk8 = -1;
    return this;
}

/* searchActor_Ko_Hna (void *, void *) */
void *searchActor_Ko_Hna__FPvPv(void *arg0, void *arg1) {
    if (((s32) l_check_wrk < 0x14) && (fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0x141)) {
        l_check_inf[l_check_wrk] = arg0;
        l_check_wrk += 1;
    }
    return NULL;
}

/* searchActor_Ko_Bou (void *, void *) */
void *searchActor_Ko_Bou__FPvPv(void *arg0, void *arg1) {
    if (((s32) l_check_wrk < 0x14) && (fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0x142)) {
        l_check_inf[l_check_wrk] = arg0;
        l_check_wrk += 1;
    }
    return NULL;
}

/* searchActor_Ob (void *, void *) */
void *searchActor_Ob__FPvPv(void *arg0, void *arg1) {
    if (((s32) l_check_wrk < 0x14) && (fopAc_IsActor__FPv(arg0) != 0) && ((s16) arg0->unk8 == 0x14D)) {
        l_check_inf[l_check_wrk] = arg0;
        l_check_wrk += 1;
    }
    return NULL;
}

/* nodeCallBack_Hed (J3DNode *, int) */
s32 nodeCallBack_Hed__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ko1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            nodeHedControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ko1_c::nodeHedControl (J3DNode *, J3DModel *) */
void nodeHedControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1) {
    u16 temp_r31;

    temp_r31 = arg0->unk18;
    PSMTXCopy(arg1->unk8C + (temp_r31 * 0x30), &now__14mDoMtx_stack_c);
    if ((s32) temp_r31 == (s8) this->m_hed_2_jnt_num) {
        PSMTXCopy(&now__14mDoMtx_stack_c, &this->unk6D4);
    }
}

/* nodeCallBack_Bln (J3DNode *, int) */
s32 nodeCallBack_Bln__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ko1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            nodeBlnControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ko1_c::nodeBlnControl (J3DNode *, J3DModel *) */
void nodeBlnControl__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r30;
    u16 temp_r31;

    temp_r31 = arg0->unk18;
    temp_r30 = temp_r31 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r30, &now__14mDoMtx_stack_c);
    if ((s32) temp_r31 == (s8) this->m_bln_loc_jnt_num) {
        PSMTXCopy(&this->unk6D4, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
        PSMTXCopy(&this->unk6D4, arg1->unk8C + temp_r30);
    }
}

/* nodeCallBack_Ko1 (J3DNode *, int) */
s32 nodeCallBack_Ko1__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_Ko1_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            nodeKo1Control__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_Ko1_c::nodeKo1Control (J3DNode *, J3DModel *) */
void nodeKo1Control__11daNpc_Ko1_cFP7J3DNodeP8J3DModel(daNpc_Ko1_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    s32 temp_r28;
    u16 temp_r30;
    void *(*temp_r3)(daNpc_Ko1_HIO_c *, s16);

    if ((s8) @3569.unk184 == 0) {
        @3569.unk188 = 18.0f;
        temp_r3 = &@3569 + 0x188;
        temp_r3->unk4 = (f32) @4294.unk4;
        temp_r3->unk8 = (f32) @4294.unk8;
        __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x178);
        @3569.unk184 = 1U;
    }
    temp_r30 = arg0->unk18;
    temp_r28 = temp_r30 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r28, &now__14mDoMtx_stack_c);
    if ((s32) temp_r30 == (s8) this->m_hed_jnt_num) {
        mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk292);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, (s16) -this->unk290);
        PSMTXMultVec(&now__14mDoMtx_stack_c, &@3569 + 0x188, &this->unk7DC);
    }
    if ((s32) temp_r30 == (s8) this->m_bbone_jnt_num) {
        mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk296);
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk294);
    }
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r28);
}

/* daNpc_Ko1_c::init_HNA_0 (void) */
u8 init_HNA_0__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r31, 0x2A80U) != 0) && (isEventBit__11dSv_event_cFUs(temp_r31, 0xE20U) == 0)) {
        this->mpBalloonMorf = NULL;
        sp8 = @4306.unk0;
        spC = @4306.unk4;
        sp10 = @4306.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_HNA_1 (void) */
u8 init_HNA_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xE20U) != 0) {
        this->mpBalloonMorf = NULL;
        this->unk1C4 &= 0xFFFFFF7F;
        sp8 = @4321.unk0;
        spC = @4321.unk4;
        sp10 = @4321.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_HNA_2 (void) */
u8 init_HNA_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2A80U) == 0) {
        this->mpBalloonMorf = NULL;
        sp8 = @4333.unk0;
        spC = @4333.unk4;
        sp10 = @4333.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_HNA_3 (void) */
u8 init_HNA_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 0)) {
        this->unk1C4 &= 0xFFFFFF7F;
        this->unk1C4 |= 0x4000;
        this->mpBalloonMorf = NULL;
        sp8 = @4347.unk0;
        spC = @4347.unk4;
        sp10 = @4347.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_HNA_4 (void) */
u8 init_HNA_4__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 1)) {
        sp8 = @4360.unk0;
        spC = @4360.unk4;
        sp10 = @4360.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_BOU_0 (void) */
u8 init_BOU_0__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xE20U) == 0) {
        sp8 = @4371.unk0;
        spC = @4371.unk4;
        sp10 = @4371.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk1C4 &= 0xFFFFFF7F;
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_BOU_1 (void) */
u8 init_BOU_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xE20U) != 0) {
        sp8 = @4383.unk0;
        spC = @4383.unk4;
        sp10 = @4383.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk1C4 &= 0xFFFFFF7F;
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_BOU_2 (void) */
u8 init_BOU_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 0)) {
        this->unk1C4 &= 0xFFFFFF7F;
        this->unk1C4 |= 0x4000;
        sp8 = @4397.unk0;
        spC = @4397.unk4;
        sp10 = @4397.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk708 = NULL;
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::init_BOU_3 (void) */
u8 init_BOU_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;

    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x520U) != 0) && (dKy_daynight_check__Fv() == 1)) {
        sp8 = @4410.unk0;
        spC = @4410.unk4;
        sp10 = @4410.unk8;
        set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(this, &sp8, 0);
        this->unk708 = NULL;
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::createInit (void) */
u8 createInit__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 var_r31;
    u8 temp_r0;
    u8 temp_r0_2;
    u8 temp_r4;
    u8 var_r3;

    this->unk280 = 0xA;
    temp_r0 = this->unk8A6;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 0:                                         /* switch 1 */
        this->unk26D = 0xA9;
        this->unk26F = 0xA9;
        break;
    case 1:                                         /* switch 1 */
        this->unk26D = 0xA7;
        this->unk26F = 0xA9;
        break;
    }
    this->unk258 = -4.5f;
    this->unk7D0 = this->unk204;
    this->unk7D2 = this->unk206;
    this->unk7D4 = this->unk208;
    this->unk7C4 = this->unk1F8;
    this->unk7C8 = this->unk1FC;
    this->unk7CC = this->unk200;
    this->unk80C = this->unk7C4;
    this->unk810 = this->unk7C8;
    this->unk814 = this->unk7CC;
    this->unk7E8 = this->unk80C;
    this->unk7EC = this->unk810;
    this->unk7F0 = this->unk814;
    var_r31 = 0xFF;
    temp_r4 = (u8) ((u32) this->unkB0 >> 0x10U);
    if (temp_r4 != 0xFF) {
        setInf__14dNpc_PathRun_cFUcScUc(&this->mPathRun, temp_r4, (s8) this->unk20A, 1U);
        if ((dPath *) this->mPathRun.unk0 == NULL) {
            return 0U;
        }
        this->unk1C4 &= 0xFFFFFF7F;
        var_r31 = 0xF0;
        goto block_10;
    }
block_10:
    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->mEventCut, a_staff_tbl$4419[(s8) this->m_act_no], (fopNpc_npc_c *) this);
    this->unk89F = 0xE;
    temp_r0_2 = this->m_act_no;
    if ((u32) (s8) temp_r0_2 <= 8U) {
        switch ((s8) temp_r0_2) {                   /* switch 2 */
        case 0:                                     /* switch 2 */
            var_r3 = init_HNA_0__11daNpc_Ko1_cFv(this);
            break;
        case 1:                                     /* switch 2 */
            var_r3 = init_HNA_1__11daNpc_Ko1_cFv(this);
            var_r31 = 0xF0;
            break;
        case 2:                                     /* switch 2 */
            var_r3 = init_HNA_2__11daNpc_Ko1_cFv(this);
            var_r31 = 0xF0;
            break;
        case 3:                                     /* switch 2 */
            var_r3 = init_HNA_3__11daNpc_Ko1_cFv(this);
            break;
        case 4:                                     /* switch 2 */
            var_r3 = init_HNA_4__11daNpc_Ko1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            var_r3 = init_BOU_0__11daNpc_Ko1_cFv(this);
            break;
        case 6:                                     /* switch 2 */
            var_r3 = init_BOU_1__11daNpc_Ko1_cFv(this);
            var_r31 = 0xF0;
            break;
        case 7:                                     /* switch 2 */
            var_r3 = init_BOU_2__11daNpc_Ko1_cFv(this);
            break;
        case 8:                                     /* switch 2 */
            var_r3 = init_BOU_3__11daNpc_Ko1_cFv(this);
            break;
        }
    } else {
        var_r3 = 0;
    }
    if (var_r3 != 0) {
        this->unk7D6 = this->unk204;
        this->unk7D8 = this->unk206;
        this->unk7DA = this->unk208;
        this->unk20C = this->unk7D6;
        this->unk20E = this->unk7D8;
        this->unk210 = this->unk7DA;
        Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->mStts, var_r31, 0xFF, (fopAc_ac_c *) this);
        this->unk5B8 = &this->mStts;
        Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->mCyl, &dNpc_cyl_src);
        setMorf__14mDoExt_McaMorfFf(this->mpMorf, 0.0f);
        setMorf__14mDoExt_McaMorfFf(this->mpHedMorf, 0.0f);
        setMtx__11daNpc_Ko1_cFb(this, 1U);
        return 1U;
    }
    return 0U;
}

/* daNpc_Ko1_c::setMtx (bool) */
void setMtx__11daNpc_Ko1_cFb(daNpc_Ko1_c *this, u8 arg0, ? arg_sp0) {
    dBgS *temp_r31;
    mDoExt_McaMorf *temp_r3;
    mDoExt_McaMorf *temp_r4;
    u32 var_r28;
    void **temp_r3_2;
    void **temp_r4_2;

    if ((u8) this->unk877 == 0) {
        var_r28 = 0;
        plyTexPttrnAnm__11daNpc_Ko1_cFv(this);
        if (this->unk35C & 0x20) {
            var_r28 = GetMtrlSndId__4dBgSFR13cBgS_PolyInfo(&g_dComIfG_gameInfo + 0x12A0, &this->unk41C);
        }
        this->unk860 = play__14mDoExt_McaMorfFP3VecUlSc(this->mpMorf, &this->unk260, var_r28, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A));
        if (this->mpMorf->unk68 < this->unk824) {
            this->unk860 = 1;
        }
        this->unk824 = this->mpMorf->unk68;
        if (((s32) this->unk89F == 4) && (checkPass__12J3DFrameCtrlFf(&this->mpMorf->unk58, @4294.unk10) != 0)) {
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5817U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4294.unk14, @4294.unk14, @4294.unk18, @4294.unk18, 0U);
        }
        play__14mDoExt_McaMorfFP3VecUlSc(this->mpHedMorf, &this->unk260, 0U, 0);
        temp_r4 = this->mpBalloonMorf;
        if ((temp_r4 != NULL) && ((u8) this->unk865 < 2U)) {
            temp_r4->unk68 = (f32) (s16) this->mpMorf->unk68;
        }
        this->unk86C = ((u32) this->unk35C >> 5U) & 1;
        CrrPos__9dBgS_AcchFR4dBgS(&this->unk334, &g_dComIfG_gameInfo + 0x12A0);
    }
    temp_r31 = &g_dComIfG_gameInfo + 0x12A0;
    this->unk1B5 = GetRoomId__4dBgSFR13cBgS_PolyInfo(temp_r31, &this->unk41C);
    this->unk1B6 = GetPolyColor__4dBgSFR13cBgS_PolyInfo(temp_r31, &this->unk41C);
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk7D8);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->mpMorf->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->mpMorf);
    PSMTXCopy(this->mpMorf->unk50->unk8C + ((s8) this->m_hed_jnt_num * 0x30), &this->mpHedMorf->unk50->unk24);
    calc__14mDoExt_McaMorfFv(this->mpHedMorf);
    temp_r3 = this->mpBalloonMorf;
    if (temp_r3 != NULL) {
        calc__14mDoExt_McaMorfFv(temp_r3);
    }
    temp_r4_2 = this->unk708;
    if (temp_r4_2 != NULL) {
        PSMTXCopy(this->mpMorf->unk50->unk8C + ((s8) this->m_armR2_jnt_num * 0x30), (mDoMtx_stack_c *) (temp_r4_2 + 0x24));
        temp_r3_2 = this->unk708;
        (*temp_r3_2)->unk10(temp_r3_2);
    }
    setAttention__11daNpc_Ko1_cFb(this, arg0);
}

/* daNpc_Ko1_c::anmNum_toResID (int) */
s32 anmNum_toResID__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    return a_bck_resID_tbl$4577[arg0];
}

/* daNpc_Ko1_c::headAnmNum_toResID (int) */
s32 headAnmNum_toResID__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    if ((s32) this->unk8A6 == 1) {
        return 0x25;
    }
    return a_bck_resID_tbl$4582[arg0];
}

/* daNpc_Ko1_c::balloon_anmNum_toResID (int) */
s32 balloon_anmNum_toResID__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    return a_bck_resID_tbl$4589[arg0];
}

/* daNpc_Ko1_c::btpNum_toResID (int) */
daNpc_Ko1_c *btpNum_toResID__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    daNpc_Ko1_c *temp_r0;
    u8 temp_r0_2;
    u8 temp_r0_3;

    temp_r0 = a_btp_resID_tbl$4594[arg0];
    switch (temp_r0) {                              /* switch 1; irregular */
    case 31:                                        /* switch 1 */
        temp_r0_2 = this->unk8A6;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            return (daNpc_Ko1_c *)0x1F;
        case 1:                                     /* switch 2 */
            return (daNpc_Ko1_c *)0x23;
        default:                                    /* switch 2 */
        default:                                    /* switch 3 */
            return this;
        }
        break;
    case 32:                                        /* switch 1 */
        temp_r0_3 = this->unk8A6;
        switch ((s8) temp_r0_3) {                   /* switch 3; irregular */
        case 0:                                     /* switch 3 */
            return (daNpc_Ko1_c *)0x20;
        case 1:                                     /* switch 3 */
            return (daNpc_Ko1_c *)0x24;
        }
        break;
    default:                                        /* switch 1 */
        return temp_r0;
    }
}

/* daNpc_Ko1_c::setBtp (bool, int) */
s32 setBtp__11daNpc_Ko1_cFbi(daNpc_Ko1_c *this, s32 arg0, s32 arg1, ? arg_sp0) {
    s32 sp8;
    J3DModelData *temp_r29;
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r29 = this->mpHedMorf->unk50->unk4;
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *) btpNum_toResID__11daNpc_Ko1_cFi(this, arg1), (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    this->m_hed_tex_pttrn = temp_r3;
    if (this->m_hed_tex_pttrn == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x3AA, (s32) (temp_r4 + 0x1B), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0x3AA, temp_r3_2 + 0x30);
    }
    sp8 = 0;
    if (init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->mBtpAnm, temp_r29, (J3DAnmTexPattern *) this->m_hed_tex_pttrn, 1, 2, 1.0f, 0, -1, arg0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0) {
        this->mBtpFrame = 0;
        this->unk72E = 0;
        return 1;
    }
    return 0;
}

/* daNpc_Ko1_c::iniTexPttrnAnm (bool) */
void iniTexPttrnAnm__11daNpc_Ko1_cFb(daNpc_Ko1_c *this, s32 arg0) {
    setBtp__11daNpc_Ko1_cFbi(this, arg0, (s32) (s8) this->unk89E);
}

/* daNpc_Ko1_c::plyTexPttrnAnm (void) */
void plyTexPttrnAnm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s16 temp_r4;

    if ((((s8) this->unk89E != 0) || (cLib_calcTimer<s>__FPs(&this->unk72E) == 0)) && (this->mBtpFrame += 1, temp_r4 = this->m_hed_tex_pttrn->unk6, (((s32) this->mBtpFrame < temp_r4) == 0))) {
        if ((s8) this->unk89E != 0) {
            this->mBtpFrame = (u8) temp_r4;
            return;
        }
        this->unk72E = (s16) (30.0f + cM_rndF__Ff(60.0f));
        this->mBtpFrame = 0;
    }
}

/* daNpc_Ko1_c::setAnm_tex (char signed) */
void setAnm_tex__11daNpc_Ko1_cFSc(daNpc_Ko1_c *this, s8 arg0) {
    if ((s8) this->unk89E != arg0) {
        this->unk89E = (u8) arg0;
        iniTexPttrnAnm__11daNpc_Ko1_cFb(this, 1);
    }
}

/* daNpc_Ko1_c::setAnm_anm (daNpc_Ko1_c::anm_prm_c *) */
s32 setAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(daNpc_Ko1_c *this, daNpc_Ko1_c::anm_prm_c *arg0) {
    u8 temp_r5;

    temp_r5 = arg0->unk0;
    if ((s8) this->unk89F == (s8) temp_r5) {
        return 1;
    }
    this->unk89F = temp_r5;
    dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->mpMorf, arg0->unkC, arg0->unk4, arg0->unk8, anmNum_toResID__11daNpc_Ko1_cFi(this, (s32) (s8) this->unk89F), -1, "Ko1" + 8);
    dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->mpHedMorf, arg0->unkC, arg0->unk4, arg0->unk8, headAnmNum_toResID__11daNpc_Ko1_cFi(this, (s32) (s8) this->unk89F), -1, "Ko1" + 8);
    this->unk860 = 0;
    this->unk861 = 0;
    this->unk824 = (f32) 0;
    return 1;
}

/* daNpc_Ko1_c::set_balloonAnm_anm (daNpc_Ko1_c::anm_prm_c *) */
s32 set_balloonAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(daNpc_Ko1_c *this, daNpc_Ko1_c::anm_prm_c *arg0) {
    u8 temp_r5;

    temp_r5 = arg0->unk0;
    if ((s8) this->unk8A1 == (s8) temp_r5) {
        return 1;
    }
    this->unk8A1 = temp_r5;
    dNpc_setAnmIDRes__FP14mDoExt_McaMorfiffiiPCc(this->mpBalloonMorf, arg0->unkC, arg0->unk4, arg0->unk8, balloon_anmNum_toResID__11daNpc_Ko1_cFi(this, (s32) (s8) this->unk8A1), -1, "Ko1" + 8);
    this->unk85E = 0;
    this->unk85F = 0;
    this->unk828 = (f32) 0;
    return 1;
}

/* daNpc_Ko1_c::set_balloonAnm_NUM (int) */
void set_balloonAnm_NUM__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    set_balloonAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(this, (daNpc_Ko1_c::anm_prm_c *) &a_anm_prm_tbl$4677[arg0]);
}

/* daNpc_Ko1_c::setAnm_NUM (int, int) */
void setAnm_NUM__11daNpc_Ko1_cFii(daNpc_Ko1_c *this, s32 arg0, s32 arg1) {
    if (arg1 != 0) {
        setAnm_tex__11daNpc_Ko1_cFSc(this, (s8) a_anm_prm_tbl$4682[arg0].unk1);
    }
    setAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(this, (daNpc_Ko1_c::anm_prm_c *) &a_anm_prm_tbl$4682[arg0]);
}

/* daNpc_Ko1_c::setAnm (void) */
s32 setAnm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    struct _struct_a_anm_prm_tbl$4689_0x14 *temp_r4_2;
    u8 temp_r4;

    temp_r4 = a_anm_prm_tbl$4689[(s8) this->unk8A3].unk1;
    if ((s8) temp_r4 >= 0) {
        setAnm_tex__11daNpc_Ko1_cFSc(this, (s8) temp_r4);
    }
    temp_r4_2 = &a_anm_prm_tbl$4689[(s8) this->unk8A3];
    if ((s8) temp_r4_2->unk0 >= 0) {
        setAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(this, (daNpc_Ko1_c::anm_prm_c *) temp_r4_2);
    }
    return 1;
}

/* daNpc_Ko1_c::setPlaySpd (float) */
void setPlaySpd__11daNpc_Ko1_cFf(daNpc_Ko1_c *this, f32 arg0) {
    this->mpHedMorf->unk64 = arg0;
    this->mpMorf->unk64 = arg0;
}

/* daNpc_Ko1_c::chg_anmTag (void) */
void chg_anmTag__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    if ((s32) this->mAnmTag != 0xA) {
        return;
    }
    setAnm_NUM__11daNpc_Ko1_cFii(this, 4, 1);
}

/* daNpc_Ko1_c::control_anmTag (void) */
void control_anmTag__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    if ((s32) this->mAnmTag != 0xA) {
        return;
    }
    if ((s8) this->unk860 != 0) {
        this->mAnmTag = 0xFF;
        setAnm_ATR__11daNpc_Ko1_cFi(this, 1);
    }
}

/* daNpc_Ko1_c::chg_anmAtr (char unsigned) */
void chg_anmAtr__11daNpc_Ko1_cFUc(daNpc_Ko1_c *this, u8 arg0) {
    s32 temp_r5;

    temp_r5 = this->unk848;
    if ((u32) (temp_r5 + 0x10000) != -1U) {
        if (temp_r5 != 0xB0A) {
            if (temp_r5 < 0xB0A) {
                if (temp_r5 != 0xB03) {

                } else {
                    goto block_8;
                }
            } else if (temp_r5 < 0xB66) {
                if (temp_r5 < 0xB64) {

                } else {
                    goto block_8;
                }
            }
        } else {
block_8:
            this->unk8A5 = 1;
            this->unk876 = 0;
            this->unk29A = 1;
        }
    }
    if (arg0 < 0xDU) {
        if (arg0 == (u8) this->mAnmAtr) {
            return;
        }
        this->mAnmAtr = arg0;
        if ((u8) this->mAnmAtr == 0xB) {
            this->unk8A5 = 1;
            this->unk876 = 0;
            this->unk29A = 1;
        }
        setAnm_ATR__11daNpc_Ko1_cFi(this, 1);
    }
}

/* daNpc_Ko1_c::control_anmAtr (void) */
void control_anmAtr__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mAnmAtr;
    if ((s32) temp_r0 != 0xB) {
        if ((s32) temp_r0 < 0xB) {

        }
    } else if ((s8) this->unk860 != 0) {
        this->mAnmAtr = 0;
        setAnm_NUM__11daNpc_Ko1_cFii(this, 0, 1);
    }
}

/* daNpc_Ko1_c::setAnm_ATR (int) */
void setAnm_ATR__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    if (arg0 != 0) {
        setAnm_tex__11daNpc_Ko1_cFSc(this, (s8) a_anm_prm_tbl$4763[this->mAnmAtr].unk1);
    }
    setAnm_anm__11daNpc_Ko1_cFPQ211daNpc_Ko1_c9anm_prm_c(this, (daNpc_Ko1_c::anm_prm_c *) &a_anm_prm_tbl$4763[this->mAnmAtr]);
}

/* daNpc_Ko1_c::anmAtr (short unsigned) */
void anmAtr__11daNpc_Ko1_cFUs(daNpc_Ko1_c *this, u16 arg0) {
    u8 temp_r4;
    void *temp_r5;

    temp_r5 = &g_dComIfG_gameInfo + 0x5C20;
    if (((u8) this->m_manzai_stt == 2) && (this != (u32) (temp_r5 + (temp_r5->unk4 * 4))->unk4)) {
        control_anmTag__11daNpc_Ko1_cFv(&*this);
        control_anmAtr__11daNpc_Ko1_cFv(&*this);
        return;
    }
    switch ((s32) arg0) {                           /* irregular */
    case 6:
        if ((s8) this->unk8A9 == 0) {
            this->mAnmAtr = 0xFF;
            chg_anmAtr__11daNpc_Ko1_cFUc(&*this, g_dComIfG_gameInfo.unk5BDB);
            this->unk8A9 += 1;
        }
        temp_r4 = g_dComIfG_gameInfo.unk5BDC;
        g_dComIfG_gameInfo.unk5BDC = 0xFFU;
        if ((temp_r4 != 0xFF) && ((u8) this->mAnmTag != temp_r4)) {
            this->mAnmTag = temp_r4;
            chg_anmTag__11daNpc_Ko1_cFv(&*this);
        }
        break;
    case 14:
        this->unk8A9 = 0;
        break;
    }
    control_anmTag__11daNpc_Ko1_cFv(&*this);
    control_anmAtr__11daNpc_Ko1_cFv(&*this);
}

/* daNpc_Ko1_c::eventOrder (void) */
void eventOrder__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 temp_r0;

    temp_r0 = this->mOrderType;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 2)) {
        this->unkFA |= 1;
        if ((s32) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
        }
    }
}

/* daNpc_Ko1_c::checkOrder (void) */
void checkOrder__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 1:
        temp_r0_2 = this->mOrderType;
        if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 2)) {
            this->mOrderType = 0;
            this->unk875 = 1;
            return;
        }
    case 2:
        return;
    }
}

/* daNpc_Ko1_c::chk_talk (void) */
u8 chk_talk__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 var_r0;
    u8 var_r31;

    var_r31 = 0;
    var_r0 = 0;
    if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
        var_r0 = 1;
    }
    if (var_r0 != 0) {
        if (ChkPresentEnd__16dEvent_manager_cFv(&g_dComIfG_gameInfo + 0x52CC) != 0) {
            this->unk863 = g_dComIfG_gameInfo.unk52B9;
            var_r31 = 1;
        }
    } else {
        this->unk863 = 0xFF;
        var_r31 = 1;
    }
    return var_r31;
}

/* daNpc_Ko1_c::chk_manzai_1 (void) */
u8 chk_manzai_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    JUTAssertion *temp_r29;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r3_3;
    JUTAssertion *temp_r3_4;
    f32 var_r0;
    s32 var_r27;
    s32 var_r28;
    s32 var_r31;
    s8 *temp_r3_2;
    s8 *temp_r4;
    u8 temp_r0;
    u8 temp_r0_2;

    var_r28 = 0;
    var_r27 = 0;
    var_r31 = 0;
loop_10:
    temp_r0 = this->m_partner_num;
    if (var_r27 < (s32) temp_r0) {
        temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, *(this + (var_r31 + 0x7B4)));
        if (temp_r3 == NULL) {
            temp_r4 = "Ko1";
            showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x582, (s32) (temp_r4 + 0x35), M2C_ERROR(/* Read from unset register $r7 */));
            temp_r3_2 = "Ko1";
            OSPanic(temp_r3_2 + 0xB, 0x582, temp_r3_2 + 0x30);
        }
        if ((u8) temp_r3->m_manzai_stt == 2) {
            var_r28 += 1;
        } else {
            temp_r3->unk1C4 |= 0x4000;
            if (this != NULL) {
                var_r0 = this->unk4;
            } else {
                var_r0 = nanf;
            }
            temp_r3->unk6B4 = var_r0;
            temp_r3->m_manzai_stt = 1;
        }
        var_r27 += 1;
        var_r31 += 4;
        goto loop_10;
    }
    if ((u8) (var_r28 == temp_r0) != 0) {
        temp_r29 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id0);
        temp_r0_2 = this->m_act_no;
        switch ((s8) temp_r0_2) {                   /* irregular */
        case 1:
            g_dComIfG_gameInfo.unk5C28 = temp_r29;
            g_dComIfG_gameInfo.unk5C2C = this;
            g_dComIfG_gameInfo.unk5C30 = NULL;
            g_dComIfG_gameInfo.unk5C34 = 0;
            g_dComIfG_gameInfo.unk5C38 = 0;
            g_dComIfG_gameInfo.unk5C3C = 0;
            g_dComIfG_gameInfo.unk5C40 = 0;
            g_dComIfG_gameInfo.unk5C44 = 0;
            g_dComIfG_gameInfo.unk5C48 = 0;
            g_dComIfG_gameInfo.unk5C4C = 0;
            break;
        case 3:
            temp_r3_3 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id1);
            g_dComIfG_gameInfo.unk5C28 = temp_r29;
            g_dComIfG_gameInfo.unk5C2C = (daNpc_Ko1_c *) temp_r3_3;
            g_dComIfG_gameInfo.unk5C30 = (JUTAssertion *) this;
            g_dComIfG_gameInfo.unk5C34 = 0;
            g_dComIfG_gameInfo.unk5C38 = 0;
            g_dComIfG_gameInfo.unk5C3C = 0;
            g_dComIfG_gameInfo.unk5C40 = 0;
            g_dComIfG_gameInfo.unk5C44 = 0;
            g_dComIfG_gameInfo.unk5C48 = 0;
            g_dComIfG_gameInfo.unk5C4C = 0;
            break;
        case 6:
            g_dComIfG_gameInfo.unk5C28 = (JUTAssertion *) this;
            g_dComIfG_gameInfo.unk5C2C = (daNpc_Ko1_c *) temp_r29;
            g_dComIfG_gameInfo.unk5C30 = NULL;
            g_dComIfG_gameInfo.unk5C34 = 0;
            g_dComIfG_gameInfo.unk5C38 = 0;
            g_dComIfG_gameInfo.unk5C3C = 0;
            g_dComIfG_gameInfo.unk5C40 = 0;
            g_dComIfG_gameInfo.unk5C44 = 0;
            g_dComIfG_gameInfo.unk5C48 = 0;
            g_dComIfG_gameInfo.unk5C4C = 0;
            break;
        case 7:
            temp_r3_4 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id1);
            g_dComIfG_gameInfo.unk5C28 = (JUTAssertion *) this;
            g_dComIfG_gameInfo.unk5C2C = (daNpc_Ko1_c *) temp_r3_4;
            g_dComIfG_gameInfo.unk5C30 = temp_r29;
            g_dComIfG_gameInfo.unk5C34 = 0;
            g_dComIfG_gameInfo.unk5C38 = 0;
            g_dComIfG_gameInfo.unk5C3C = 0;
            g_dComIfG_gameInfo.unk5C40 = 0;
            g_dComIfG_gameInfo.unk5C44 = 0;
            g_dComIfG_gameInfo.unk5C48 = 0;
            g_dComIfG_gameInfo.unk5C4C = 0;
            break;
        }
        this->m_manzai_stt = 2;
    }
    return var_r28 == temp_r0;
}

/* daNpc_Ko1_c::chk_partsNotMove (void) */
u8 chk_partsNotMove__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 var_r5;

    var_r5 = 0;
    if (((s16) this->unk840 == (s16) this->unk292) && ((s16) this->unk842 == (s16) this->unk296) && ((s16) this->unk844 == (s16) this->unk206)) {
        var_r5 = 1;
    }
    return var_r5;
}

/* daNpc_Ko1_c::lookBack (void) */
void lookBack__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
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
    f32 temp_f1;
    f32 temp_f2;
    f32 temp_f3;
    f32 temp_f4;
    s16 var_r29;
    u8 temp_r0;
    u8 temp_r28;

    this->unk840 = this->unk292;
    this->unk842 = this->unk296;
    this->unk844 = this->unk206;
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
    var_r30 = NULL;
    var_r29 = this->unk206;
    temp_r28 = this->unk876;
    temp_r0 = this->unk8A5;
    switch ((s8) temp_r0) {                         /* irregular */
    case 1:
        dNpc_playerEyePos__Ff(-20.0f);
        sp2C = sp14;
        sp30 = sp18;
        sp34 = sp1C;
        var_r30 = &sp2C;
        sp20 = this->unk1F8;
        sp24 = this->unk1FC;
        sp28 = this->unk200;
        sp24 = this->unk264;
        break;
    case 2:
        sp2C = this->unk7E8;
        sp30 = this->unk7EC;
        sp34 = this->unk7F0;
        var_r30 = &sp2C;
        sp20 = temp_f4;
        sp24 = temp_f3;
        sp28 = temp_f2;
        sp24 = temp_f1;
        break;
    case 3:
        var_r29 = this->unk858;
        break;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk856, l_HIO[(s8) this->unk8A6].unk22, 4, 0x800);
    if ((u8) this->unk29A == 0) {
        this->unk856 = 0;
    }
    sp8 = sp20;
    spC = sp24;
    sp10 = sp28;
    lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk206, (cXyz *) var_r30, (cXyz) &sp8, var_r29, this->unk856, temp_r28);
}

/* daNpc_Ko1_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__11daNpc_Ko1_cFPUl(daNpc_Ko1_c *this, u32 *arg0, ? arg_sp0) {
    s32 var_r31;
    u32 temp_r3;

    var_r31 = 0xF;
    temp_r3 = *arg0;
    switch (temp_r3) {
    case 0xAF4:
        *arg0 = 0xB57;
        break;
    case 0xAF5:
        *arg0 = 0xB58;
        break;
    case 0xB59:
        *arg0 = 0xB5A;
        break;
    case 0xB5A:
        *arg0 = 0xB5B;
        break;
    case 0xB5B:
        *arg0 = 0xB5C;
        break;
    case 0xB5C:
        *arg0 = 0xB5D;
        break;
    case 0xB5D:
        *arg0 = 0xB64;
        break;
    case 0xB5E:
        *arg0 = 0xB5F;
        break;
    case 0xB5F:
        *arg0 = 0xB60;
        break;
    case 0xB60:
        *arg0 = 0xB61;
        break;
    case 0xB61:
        *arg0 = 0xB65;
        break;
    case 0xAF6:
        *arg0 = 0xAF7;
        break;
    case 0xAF7:
        *arg0 = 0xAF8;
        break;
    case 0xAF8:
        *arg0 = 0xAF9;
        break;
    case 0xAF9:
        *arg0 = 0xAFA;
        break;
    case 0xAFA:
        *arg0 = 0xAFB;
        break;
    case 0xAFB:
        if (bitCount__11daNpc_Ko1_cFUc(this, getEventReg__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0xBFFFU)) != 0) {
            *arg0 = 0xAFC;
        } else {
            *arg0 = 0xAFD;
        }
        break;
    case 0xAFC:
    case 0xAFD:
        *arg0 = 0xAFE;
        break;
    case 0xAFE:
        *arg0 = 0xAFF;
        break;
    case 0xB00:
    case 0xB07:
        *arg0 = 0xB01;
        break;
    case 0xB01:
        *arg0 = 0xB02;
        break;
    case 0xB02:
        *arg0 = 0xB03;
        break;
    case 0xB03:
        *arg0 = 0xB04;
        break;
    case 0xB04:
        *arg0 = 0xB05;
        break;
    case 0xB05:
        *arg0 = 0xB06;
        break;
    case 0xB08:
        *arg0 = 0xB09;
        break;
    case 0xB09:
        *arg0 = 0xB0A;
        break;
    case 0xB0A:
        *arg0 = 0xB0B;
        break;
    case 0xB0B:
        *arg0 = 0xB0C;
        break;
    case 0xB0C:
        *arg0 = 0xB0D;
        break;
    default:
        var_r31 = 0x10;
        break;
    }
    return var_r31;
}

/* daNpc_Ko1_c::getMsg_HNA_0 (void) */
s32 getMsg_HNA_0__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x220U) != 0) + 0xB55;
}

/* daNpc_Ko1_c::getMsg_HNA_1 (void) */
s32 getMsg_HNA_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x240U) != 0) + 0xAF4;
}

/* daNpc_Ko1_c::getMsg_HNA_2 (void) */
s32 getMsg_HNA_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3101U) != 0) + 0xB62;
}

/* daNpc_Ko1_c::getMsg_HNA_3 (void) */
s32 getMsg_HNA_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 var_r3;

    if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2C04U) != 0) {
        var_r3 = 0xB5A;
        if ((u8) this->unk86D != 0) {
            return 0xB5E;
        }
        /* Duplicate return node #4. Try simplifying control flow for better match */
        return var_r3;
    }
    var_r3 = 0xB59;
    return var_r3;
}

/* daNpc_Ko1_c::getMsg_BOU_0 (void) */
s32 getMsg_BOU_0__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    dSv_event_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x104U) != 0) {
        return 0xAF3;
    }
    return (isEventBit__11dSv_event_cFUs(temp_r31, 0x210U) != 0) + 0xAF1;
}

/* daNpc_Ko1_c::getMsg_BOU_1 (void) */
s32 getMsg_BOU_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    return (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x208U) != 0) + 0xAF4;
}

/* daNpc_Ko1_c::bitCount (char unsigned) */
void bitCount__11daNpc_Ko1_cFUc(daNpc_Ko1_c *this, u8 arg0) {
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

/* daNpc_Ko1_c::getMsg_BOU_2 (void) */
s32 getMsg_BOU_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    dSv_event_c *temp_r31;
    s32 var_r3;

    temp_r31 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3801U) != 0) {
        if (isEventBit__11dSv_event_cFUs(temp_r31, 0x3340U) != 0) {
            return 0xB08;
        }
        var_r3 = 0xB00;
        if (bitCount__11daNpc_Ko1_cFUc(this, getEventReg__11dSv_event_cFUs(temp_r31, 0xBFFFU)) >= 2) {
            return 0xB07;
        }
        /* Duplicate return node #6. Try simplifying control flow for better match */
        return var_r3;
    }
    var_r3 = 0xAF6;
    return var_r3;
}

/* daNpc_Ko1_c::getMsg (void) */
s32 getMsg__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 var_r5;
    u8 temp_r0;

    var_r5 = 0;
    temp_r0 = this->m_act_no;
    if ((u32) (s8) temp_r0 <= 8U) {
        switch ((s8) temp_r0) {
        case 0:
            var_r5 = getMsg_HNA_0__11daNpc_Ko1_cFv(this);
            break;
        case 1:
            var_r5 = getMsg_HNA_1__11daNpc_Ko1_cFv(this);
            break;
        case 2:
            var_r5 = getMsg_HNA_2__11daNpc_Ko1_cFv(this);
            break;
        case 3:
            var_r5 = getMsg_HNA_3__11daNpc_Ko1_cFv(this);
            break;
        case 5:
            var_r5 = getMsg_BOU_0__11daNpc_Ko1_cFv(this);
            break;
        case 6:
            var_r5 = getMsg_BOU_1__11daNpc_Ko1_cFv(this);
            break;
        case 7:
            var_r5 = getMsg_BOU_2__11daNpc_Ko1_cFv(this);
            break;
        }
    }
    return var_r5;
}

/* daNpc_Ko1_c::chkAttention (void) */
u8 chkAttention__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    dAttention_c *temp_r31;

    temp_r31 = &g_dComIfG_gameInfo + 0x5808;
    if (LockonTruth__12dAttention_cFv(temp_r31) != 0) {
        return this == LockonTarget__12dAttention_cFl(temp_r31, 0);
    }
    return this == ActionTarget__12dAttention_cFl(temp_r31, 0);
}

/* daNpc_Ko1_c::setAttention (bool) */
void setAttention__11daNpc_Ko1_cFb(daNpc_Ko1_c *this, u8 arg0) {
    this->unk274 = this->unk1F8;
    this->unk278 = this->unk1FC + l_HIO[(s8) this->unk8A6].unk24;
    this->unk27C = this->unk200;
    if (((s32) this->unk870 != 0) || (arg0 != 0)) {
        this->unk260.unk0 = this->unk7DC;
        this->unk264 = this->unk7E0;
        this->unk268 = this->unk7E4;
    }
}

/* daNpc_Ko1_c::searchByID (int unsigned) */
JUTAssertion *searchByID__11daNpc_Ko1_cFUi(daNpc_Ko1_c *this, u32 arg0) {
    fopAc_ac_c *sp8;

    sp8 = NULL;
    fopAcM_SearchByID__FUiPP10fopAc_ac_c(arg0, &sp8);
    return (JUTAssertion *) sp8;
}

/* daNpc_Ko1_c::partner_srch_sub (void * (*) (void *, void *) *) */
u32 partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(daNpc_Ko1_c *this, void *(*arg0)(void *, void *)) {
    s32 var_ctr;
    s32 var_r3;
    u32 var_r31;

    var_r31 = -1U;
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
            var_r31 = (*l_check_inf)->unk4;
        } else {
            var_r31 = -1U;
        }
    }
    return var_r31;
}

/* daNpc_Ko1_c::partner_srch (void) */
void partner_srch__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 temp_r0;

    if (((s32) this->unk8A8 == 1) && (temp_r0 = this->m_act_no, (((u32) (s8) temp_r0 > 8U) == 0))) {
        switch ((s8) temp_r0) {
        case 1:
            this->m_partner_id0 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ko_Bou__FPvPv);
            this->m_partner_num = 1;
            return;
        case 3:
            this->m_partner_id0 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ko_Bou__FPvPv);
            this->m_partner_id1 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ob__FPvPv);
            this->m_partner_num = 2;
            return;
        case 6:
            this->m_partner_id0 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ko_Hna__FPvPv);
            this->m_partner_num = 1;
            return;
        case 7:
            this->m_partner_id0 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ko_Hna__FPvPv);
            this->m_partner_id1 = partner_srch_sub__11daNpc_Ko1_cFPFPvPv_Pv(this, searchActor_Ob__FPvPv);
            this->m_partner_num = 2;
            break;
        }
    } else {
    case 0:
    case 2:
    case 4:
    case 5:
    case 8:
    }
}

/* daNpc_Ko1_c::check_landOn (void) */
s32 check_landOn__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
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
    void *temp_r30;

    temp_r30 = g_dComIfG_gameInfo.unk5B4C;
    sp24 = @4294.unkEC;
    sp28 = @4294.unkF0;
    sp2C = @4294.unkF4;
    __mi__4cXyzCFRC3Vec(&sp18, temp_r30 + 0x1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4294.unk8;
    sp14 = sp20;
    var_f1 = PSVECSquareMag(&spC);
    if (var_f1 > @4294.unk8) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    if ((temp_r30->unk1FC == sp28) && (var_f1 < @4294.unk108) && (temp_r30->unk2A4 & 0x40)) {
        onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x104U);
        return 1;
    }
    return 0;
}

/* daNpc_Ko1_c::ko_setPthPos (void) */
void ko_setPthPos__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp28;
    f32 sp24;
    f32 sp20;
    dNpc_PathRun_c sp14;
    dNpc_PathRun_c sp8;

    if ((dPath *) this->mPathRun.unk0 != NULL) {
        getPoint__14dNpc_PathRun_cFUc(&sp14, (u8) &this->mPathRun);
        this->unk1F8 = (bitwise f32) sp14;
        this->unk1FC = sp18;
        this->unk200 = sp1C;
        nextIdxAuto__14dNpc_PathRun_cFv(&this->mPathRun);
        getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->mPathRun);
        sp20 = (bitwise f32) sp8;
        sp24 = spC;
        sp28 = sp10;
        this->unk206 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp20);
    }
}

/* daNpc_Ko1_c::set_tgtPos (cXyz) */
void set_tgtPos__11daNpc_Ko1_cF4cXyz(daNpc_Ko1_c *this, ? arg0, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp8;
    f32 temp_f1;
    s16 temp_r5;

    sp14 = @4294.unk8;
    sp18 = @4294.unk8;
    sp1C = @4294.unk8;
    PSMTXTrans(&now__14mDoMtx_stack_c, *M2C_ERROR(/* Read from unset register $r5 */), M2C_ERROR(/* Read from unset register $r5 */)->unk4, M2C_ERROR(/* Read from unset register $r5 */)->unk8);
    mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, g_dComIfG_gameInfo.unk5B4C->unk206);
    temp_r5 = arg0->unk85A;
    sp14 = @4294.unk10C * *(jmaSinTable + (((s32) (u16) temp_r5 >> jmaSinShift) * 4));
    temp_f1 = *(jmaCosTable + (((s32) (u16) temp_r5 >> jmaSinShift) * 4));
    sp1C = @4294.unk110 * temp_f1;
    arg0->unk85A = (s16) (temp_r5 + 0x400);
    PSMTXMultVec(&now__14mDoMtx_stack_c, &sp14, &sp8, jmaSinTable, jmaSinShift, temp_f1);
    this->unk0 = sp8;
    this->unk4 = spC;
    this->unk8 = sp10;
}

/* daNpc_Ko1_c::ko_movPass (void) */
s32 ko_movPass__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    dPath *temp_r3;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s32 var_r30;

    var_r30 = 0;
    temp_r3 = this->mPathRun.unk0;
    if ((temp_r3 != NULL) && (temp_r3->unk5 & 1)) {
        sp24 = this->unk1F8;
        sp28 = this->unk1FC;
        sp2C = this->unk200;
        if (chkPointPass__14dNpc_PathRun_cF4cXyzb(&this->mPathRun, (cXyz) &sp24, (u8) (this->unk742 != 0)) != 0) {
            nextIdxAuto__14dNpc_PathRun_cFv(&this->mPathRun);
            var_r30 = 1;
        }
        return var_r30;
    }
    __mi__4cXyzCFRC3Vec(&sp18, &this->m_tgt_pos);
    spC = (bitwise f32) sp18;
    sp10 = @4294.unk8;
    sp14 = sp20;
    var_f1 = PSVECSquareMag(&spC);
    if (var_f1 > @4294.unk8) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
    if (var_f1 == this->unk83C) {
        var_r30 = 1;
        if (((dPath *) this->mPathRun.unk0 != NULL) && (nextIdxAuto__14dNpc_PathRun_cFv(&this->mPathRun) == 0)) {
            var_r30 = 2;
        }
    }
    return var_r30;
}

/* daNpc_Ko1_c::ko_clcMovSpd (void) */
void ko_clcMovSpd__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    s16 sp8;
    f32 temp_f31;
    s32 var_r3;
    u8 temp_r0;

    __mi__4cXyzCFRC3Vec(&sp18, &this->m_tgt_pos);
    spC = (bitwise f32) sp18;
    sp10 = 0.0f;
    sp14 = sp20;
    temp_f31 = PSVECSquareMag(&spC);
    sp8 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &this->m_tgt_pos);
    if (routeCheck__11daNpc_Ko1_cFfPs(this, temp_f31, &sp8) != 0) {
        temp_r0 = this->unk8A3;
        if (((s8) temp_r0 == 4) || ((s8) temp_r0 == 0xB) || ((s8) temp_r0 == 0x18)) {
            if (cLib_calcTimer<s>__FPs(&this->unk850) == 0) {
                this->unk864 ^= 1;
                this->unk850 = cLib_getRndValue<i>__Fii(8, 0x14);
            }
            var_r3 = 0x2000;
            if ((u8) this->unk864 != 0) {
                var_r3 = -0x2000;
            }
            sp8 += var_r3;
        }
        cLib_chaseAngleS__FPsss(&this->unk206, sp8, l_HIO[(s8) this->unk8A6].unk2C);
        cLib_chaseF__FPfff(&this->unk254, this->m_mov_spd, this->m_mov_spd_step);
    }
}

/* daNpc_Ko1_c::ko_clcSwmSpd (void) */
void ko_clcSwmSpd__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    s16 sp8;
    f32 temp_f31;

    __mi__4cXyzCFRC3Vec(&sp18, &this->m_tgt_pos);
    spC = (bitwise f32) sp18;
    sp10 = @4294.unk8;
    sp14 = sp20;
    temp_f31 = PSVECSquareMag(&spC);
    sp8 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &this->m_tgt_pos);
    if (routeCheck__11daNpc_Ko1_cFfPs(this, temp_f31, &sp8) != 0) {
        cLib_chaseAngleS__FPsss(&this->unk206, sp8, l_HIO[(s8) this->unk8A6].unk2C);
        cLib_chaseF__FPfff(&this->unk224, this->m_swm_spd_y, @4294.unk114);
        cLib_chaseF__FPfff(&this->unk254, @4294.unk8, @4294.unk118);
    }
}

/* daNpc_Ko1_c::ko_nMove (void) */
void ko_nMove__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 var_f1;
    s32 temp_r3_3;
    s32 temp_r3_4;
    struct _struct_l_HIO_0x60 *temp_r3;
    struct _struct_l_HIO_0x60 *temp_r3_2;
    u8 temp_r0;

    temp_r0 = this->unk898;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    default:                                        /* switch 1 */
        ko_clcMovSpd__11daNpc_Ko1_cFv(this);
        if ((s32) this->unk898 == 1) {
            temp_r3 = &l_HIO[(s8) this->unk8A6];
            temp_f1 = temp_r3->unk30;
            temp_f2 = this->unk254 * temp_f1;
            var_f1 = temp_r3->unk34 * temp_f1;
            if (temp_f2 > var_f1) {

            } else {
                var_f1 = temp_f2;
            }
        } else {
            temp_r3_2 = &l_HIO[(s8) this->unk8A6];
            temp_f1_2 = temp_r3_2->unk40;
            temp_f2_2 = this->unk254 * temp_f1_2;
            var_f1 = temp_r3_2->unk44 * temp_f1_2;
            if (temp_f2_2 > var_f1) {

            } else {
                var_f1 = temp_f2_2;
            }
        }
        if (var_f1 < @4294.unk11C) {
            var_f1 = @4294.unk11C;
        }
        setPlaySpd__11daNpc_Ko1_cFf(this, var_f1);
        temp_r3_3 = ko_movPass__11daNpc_Ko1_cFv(this);
        switch (temp_r3_3) {                        /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk86B = 1;
            break;
        case 2:                                     /* switch 2 */
            this->unk86B = 1;
            this->unk898 = 0;
            break;
        }
        break;
    case 3:                                         /* switch 1 */
        ko_clcSwmSpd__11daNpc_Ko1_cFv(this);
        temp_r3_4 = ko_movPass__11daNpc_Ko1_cFv(this);
        switch (temp_r3_4) {                        /* switch 3; irregular */
        case 1:                                     /* switch 3 */
            this->unk86B = 1;
            break;
        case 2:                                     /* switch 3 */
            this->unk86B = 1;
            this->unk898 = 0;
            break;
        }
        break;
    case 4:                                         /* switch 1 */
        cLib_chaseF__FPfff(&this->unk254, @4294.unk120, this->m_mov_spd_step);
        if (((u8) this->unk86C == 0) && (this->unk35C & 0x20)) {
            this->unk224 = @4294.unk8;
            this->unk254 = @4294.unk8;
            this->unk258 = @4294.unkC;
            this->unk898 = this->unk899;
            this->unk86B = 1;
        }
        break;
    }
    if ((u8) this->unk86B != 0) {
        this->unk869 = 1;
    }
}

/* daNpc_Ko1_c::chk_routeAngle (cXyz &, short *) */
void chk_routeAngle__11daNpc_Ko1_cFR4cXyzPs(daNpc_Ko1_c *this, cXyz *arg0, s16 *arg1, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    cXyz sp8;
    f32 temp_f1;
    s16 temp_r3;
    s16 var_r31;

    outprod__4cXyzCFRC3Vec(&sp8, &this->unk818);
    temp_f1 = (bitwise f32) sp8;
    sp14 = temp_f1;
    sp18 = spC;
    sp1C = sp10;
    temp_r3 = cM_atan2s__Fff(temp_f1, sp10);
    var_r31 = temp_r3;
    M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
    if (((this->unk81C == 0.999f) && (cLib_distanceAngleS__Fss(temp_r3, *arg1) > 0x4000)) || ((sp18 * (this->unk7F8 - this->unk1FC)) < 0.0f)) {
        var_r31 += 0x8000;
    }
    *arg1 = var_r31;
}

/* daNpc_Ko1_c::routeWallCheck (cXyz &, cXyz &, short *) */
void routeWallCheck__11daNpc_Ko1_cFR4cXyzR4cXyzPs(daNpc_Ko1_c *this, cXyz *arg0, cXyz *arg1, s16 *arg2, ? arg_sp0) {
    s32 *sp6C;                                      /* compiler-managed */
    s8 sp6A;
    s8 sp69;
    s8 sp68;
    s8 sp67;
    s8 sp66;
    s8 sp65;
    s8 sp64;
    struct __vt__16cBgS_PolyPassChk *vtable60;      /* compiler-managed */
    struct __vt__8cM3dGLin *vtable44;
    struct __vt__13cBgS_PolyInfo *vtable28;         /* compiler-managed */
    s32 sp24;
    s32 sp20;
    u16 sp1E;
    u16 sp1C;
    struct __vt__11cBgS_LinChk *vtable18;           /* compiler-managed */
    s8 sp14;
    struct __vt__16cBgS_PolyPassChk **spC;
    struct __vt__16cBgS_PolyPassChk **sp8;
    cBgS *temp_r22;
    cXyz *temp_r3;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    struct __vt__16cBgS_PolyPassChk **var_r0;

    vtable18 = &__vt__8cBgS_Chk;
    sp8 = NULL;
    spC = NULL;
    sp14 = 1;
    vtable28 = &__vt__13cBgS_PolyInfo;
    sp1C = 0xFFFF;
    sp1E = 0x100;
    sp20 = 0;
    sp24 = -1;
    vtable18 = &__vt__11cBgS_LinChk;
    vtable28 = &__vt__11cBgS_LinChk.unkC;
    vtable44 = &__vt__8cM3dGLin;
    ct__11cBgS_LinChkFv((cBgS_LinChk *) &sp8);
    vtable60 = &__vt__16cBgS_PolyPassChk;
    vtable60 = &__vt__16dBgS_PolyPassChk;
    sp64 = 0;
    sp65 = 0;
    sp66 = 0;
    sp67 = 0;
    sp68 = 0;
    sp69 = 0;
    sp6A = 0;
    vtable60.unkC = &__vt__15cBgS_GrpPassChk;
    vtable60.unkC = (struct __vt__15cBgS_GrpPassChk *) &__vt__15dBgS_GrpPassChk;
    vtable60.unk10 = 1;
    vtable60 = &__vt__8dBgS_Chk;
    sp6C = &__vt__8dBgS_Chk.unkC;
    vtable18 = &__vt__11dBgS_LinChk;
    vtable28 = &__vt__11dBgS_LinChk.unkC;
    vtable60 = &__vt__11dBgS_LinChk.unk18;
    sp6C = &__vt__11dBgS_LinChk.unk24;
    sp8 = &vtable60;
    var_r0 = &vtable60;
    if (&vtable60 != NULL) {
        var_r0 = &vtable60 + 0xC;
    }
    spC = var_r0;
    Set__11dBgS_LinChkFP4cXyzP4cXyzP10fopAc_ac_c((dBgS_LinChk *) &sp8, arg0, arg1, NULL);
    temp_r22 = &g_dComIfG_gameInfo + 0x12A0;
    if (LineCross__4cBgSFP11cBgS_LinChk(temp_r22, (cBgS_LinChk *) &sp8) != 0) {
        temp_r3 = GetTriPla__4cBgSCFii(temp_r22, (s32) sp1E, (s32) sp1C);
        if (temp_r3 != NULL) {
            chk_routeAngle__11daNpc_Ko1_cFR4cXyzPs(this, temp_r3, arg2);
        }
    }
    vtable18 = &__vt__11dBgS_LinChk;
    vtable28 = &__vt__11dBgS_LinChk.unkC;
    vtable60 = &__vt__11dBgS_LinChk.unk18;
    sp6C = &__vt__11dBgS_LinChk.unk24;
    if (&vtable60 != NULL) {
        vtable60 = &__vt__8dBgS_Chk;
        sp6C = &__vt__8dBgS_Chk.unkC;
        temp_cr0_eq = &sp6C == NULL;
        if (temp_cr0_eq == 0) {
            sp6C = &__vt__15dBgS_GrpPassChk;
            if (temp_cr0_eq == 0) {
                sp6C = &__vt__15cBgS_GrpPassChk;
            }
        }
        temp_cr0_eq_2 = &vtable60 == NULL;
        if (temp_cr0_eq_2 == 0) {
            vtable60 = &__vt__16dBgS_PolyPassChk;
            if (temp_cr0_eq_2 == 0) {
                vtable60 = &__vt__16cBgS_PolyPassChk;
            }
        }
    }
    vtable18 = &__vt__11cBgS_LinChk;
    vtable28 = &__vt__11cBgS_LinChk.unkC;
    vtable44 = &__vt__8cM3dGLin;
    vtable28 = &__vt__13cBgS_PolyInfo;
    __dt__8cBgS_ChkFv((cBgS_Chk *) &sp8, 0);
}

/* daNpc_Ko1_c::chk_ForwardGroundY (short) */
f32 chk_ForwardGroundY__11daNpc_Ko1_cFs(daNpc_Ko1_c *this, s16 arg0) {
    s32 *sp60;                                      /* compiler-managed */
    s8 sp5E;
    s8 sp5D;
    s8 sp5C;
    s8 sp5B;
    s8 sp5A;
    s8 sp59;
    s8 sp58;
    struct __vt__16cBgS_PolyPassChk *vtable54;      /* compiler-managed */
    s32 sp44;
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    s32 *sp34;                                      /* compiler-managed */
    struct __vt__11dBgS_GndChk *vtable24;           /* compiler-managed */
    struct __vt__16cBgS_PolyPassChk **sp18;
    cBgS_GndChk sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    cBgS *temp_r30;
    cXyz *temp_r3;
    f32 temp_f0;
    f32 temp_f31;
    f32 temp_f3;
    f32 temp_f4;
    s32 temp_cr0_eq;
    s32 temp_cr0_eq_2;
    s32 temp_r0;
    struct __vt__16cBgS_PolyPassChk **var_r0;

    temp_r30 = &g_dComIfG_gameInfo + 0x12A0;
    temp_r3 = GetTriPla__4cBgSCFii(temp_r30, (s32) this->unk4FA, (s32) this->unk4F8);
    if ((temp_r3 != NULL) && (cLib_distanceAngleS__Fss(arg0, cM_atan2s__Fff(temp_r3->unk0, temp_r3->unk8)) > 0x4000)) {
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
        sp18 = var_r0;
        sp44 &= 0xFFFFFFFD;
        temp_r0 = ((s32) (u16) arg0 >> jmaSinShift) * 4;
        temp_f4 = this->unk200 + (80.0f * *(jmaCosTable + temp_r0));
        temp_f3 = 80.0f + this->unk1FC;
        temp_f0 = this->unk1F8 + (80.0f * *(jmaSinTable + temp_r0));
        sp8 = temp_f0;
        spC = temp_f3;
        sp10 = temp_f4;
        sp38 = temp_f0;
        sp3C = temp_f3;
        sp40 = temp_f4;
        temp_f31 = GroundCross__4cBgSFP11cBgS_GndChk(temp_r30, &sp14);
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
        return temp_f31;
    }
    return -1e7f;
}

/* daNpc_Ko1_c::chk_wallJump (short) */
f32 chk_wallJump__11daNpc_Ko1_cFs(daNpc_Ko1_c *this, s16 arg0) {
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    var_f1 = chk_ForwardGroundY__11daNpc_Ko1_cFs(this, arg0);
    if ((@4294.unk8 < var_f1) && (var_f1 < @4294.unk12C)) {
        if (var_f1 > @4294.unk8) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            sp8 = (f32) ((f64) var_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = sp8;
        }
        return @4294.unk130 * var_f1;
    }
    return @4294.unk18;
}

/* daNpc_Ko1_c::routeCheck (float, short *) */
s32 routeCheck__11daNpc_Ko1_cFfPs(daNpc_Ko1_c *this, f32 arg0, s16 *arg1) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 temp_f3;
    f32 temp_f4;
    s32 temp_r0;

    if (((s32) this->unk898 != 3) && (this->unk35C & 0x10)) {
        chk_wallJump__11daNpc_Ko1_cFs(this, *arg1);
    }
    temp_f4 = this->unk200;
    temp_f3 = 80.0f + this->unk1FC;
    temp_f1 = this->unk1F8;
    sp14 = temp_f1;
    sp18 = temp_f3;
    sp1C = temp_f4;
    temp_r0 = ((s32) (u16) *arg1 >> jmaSinShift) * 4;
    sp8 = temp_f1 + (80.0f * *(jmaSinTable + temp_r0));
    spC = temp_f3;
    sp10 = temp_f4 + (80.0f * *(jmaCosTable + temp_r0));
    routeWallCheck__11daNpc_Ko1_cFR4cXyzR4cXyzPs(this, (cXyz *) &sp14, (cXyz *) &sp8, arg1);
    return 1;
}

/* daNpc_Ko1_c::chk_start_swim (void) */
u8 chk_start_swim__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    u8 var_r31;

    var_r31 = 0;
    if (this->unk35C & 0x1000) {
        var_r31 = ((u32) M2C_ERROR(/* unknown instruction: mfcr $r0 */) >> 0x1EU) & 1;
        if (var_r31 != 0) {
            if ((s32) this->unk8A3 != 7) {
                setPrtcl_Hamon__11daNpc_Ko1_cFff(this, @4294.unk14, @4294.unk8);
            }
        } else if ((s32) this->unk8A3 == 7) {
            setPrtcl_Hamon__11daNpc_Ko1_cFff(this, @4294.unk11C, @4294.unk14);
        }
    } else {
        end__19dPa_rippleEcallBackFv(&this->unk878);
        this->unk88C = 0U;
    }
    return var_r31;
}

/* daNpc_Ko1_c::get_crsActorID (void) */
u32 get_crsActorID__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    void *temp_r3;
    void *temp_r3_2;
    void *var_r3;

    if (ChkCoHit__12dCcD_GObjInfFv((dCcD_GObjInf *) &this->mCyl) != 0U) {
        temp_r3 = GetCoHitObj__12dCcD_GObjInfFv((dCcD_GObjInf *) &this->mCyl);
        if (temp_r3 != NULL) {
            temp_r3_2 = temp_r3->unk44;
            if (temp_r3_2 == NULL) {
                var_r3 = NULL;
            } else {
                var_r3 = temp_r3_2->unkC;
            }
            if (var_r3 != NULL) {
                return var_r3->unk4;
            }
            return -1U;
        }
    }
    return -1U;
}

/* daNpc_Ko1_c::chk_areaIn (float, cXyz) */
u32 chk_areaIn__11daNpc_Ko1_cFf4cXyz(daNpc_Ko1_c *this, f32 arg0, ? arg1) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    __mi__4cXyzCFRC3Vec(&sp18, g_dComIfG_gameInfo.unk5B4C + 0x1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4294.unk8;
    sp14 = sp20;
    temp_f1 = PSVECSquareMag(&spC);
    if (temp_f1 > @4294.unk8) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) temp_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
    }
    return (u32) M2C_ERROR(/* unknown instruction: mfcr $r0 */) >> 0x1FU;
}

/* daNpc_Ko1_c::setPrtcl_Hamon (float, float) */
void setPrtcl_Hamon__11daNpc_Ko1_cFff(daNpc_Ko1_c *this, f32 arg0, f32 arg1) {
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;

    sp18 = arg0;
    sp1C = arg0;
    sp20 = arg0;
    end__19dPa_rippleEcallBackFv(&this->unk878);
    sp8 = -1;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk88C = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 5U, 0x33U, (cXyz *) &this->unk1F8, NULL, (cXyz *) &sp18, 0xFFU, (dPa_levelEcallBack *) &this->unk878, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    if ((u32) this->unk88C != 0U) {
        this->unk888 = arg1;
    }
}

/* daNpc_Ko1_c::setPrtcl_HanaPachi (void) */
void setPrtcl_HanaPachi__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;

    PSMTXCopy(this->mpBalloonMorf->unk50->unk8C + ((s8) this->m_bln_jnt_num * 0x30), &now__14mDoMtx_stack_c);
    sp18 = now__14mDoMtx_stack_c.unk0[3];
    sp1C = now__14mDoMtx_stack_c.unk1C;
    sp20 = now__14mDoMtx_stack_c.unk2C;
    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk890 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x830CU, (cXyz *) &sp18, (csXyz *) &this->unk204, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    sp8 = (s32) this->unk20A;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    this->unk894 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x8313U, (cXyz *) &sp18, (csXyz *) &this->unk204, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
}

/* daNpc_Ko1_c::charDecide (int) */
u8 charDecide__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    s16 temp_r0;

    this->unk8A6 = -1U;
    this->m_act_no = -1U;
    temp_r0 = this->unk8;
    switch (temp_r0) {                              /* switch 1; irregular */
    case 0x141:                                     /* switch 1 */
        this->unk8A6 = 0;
        switch (arg0) {                             /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            this->m_act_no = 0;
            return 1U;
        case 1:                                     /* switch 2 */
            this->m_act_no = 1;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 2:                                     /* switch 2 */
            this->m_act_no = 2;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 3:                                     /* switch 2 */
            this->m_act_no = 3;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 4:                                     /* switch 2 */
            this->m_act_no = 4;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        default:                                    /* switch 2 */
            return 0U;
        }
        break;
    case 0x142:                                     /* switch 1 */
        this->unk8A6 = 1;
        switch (arg0) {                             /* switch 3; irregular */
        case 0:                                     /* switch 3 */
            this->m_act_no = 5;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 1:                                     /* switch 3 */
            this->m_act_no = 6;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 2:                                     /* switch 3 */
            this->m_act_no = 7;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        case 3:                                     /* switch 3 */
            this->m_act_no = 8;
            /* Duplicate return node #31. Try simplifying control flow for better match */
            return 1U;
        default:                                    /* switch 3 */
            return 0U;
        }
        break;
    default:                                        /* switch 1 */
        return 0U;
    }
}

/* daNpc_Ko1_c::event_actionInit (int) */
void event_actionInit__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    s32 *temp_r3;

    temp_r3 = getMySubstanceP__16dEvent_manager_cFiPCci(&g_dComIfG_gameInfo + 0x52CC, arg0, "Ko1" + 0x42, 3);
    if (temp_r3 != NULL) {
        this->unk89B = (u8) *temp_r3;
    }
}

/* daNpc_Ko1_c::event_action (void) */
u8 event_action__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    return 1U;
}

/* daNpc_Ko1_c::privateCut (int) */
void privateCut__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0, ? arg_sp0) {
    dEvent_manager_c *temp_r31;
    u8 var_r3;

    if (arg0 != -1) {
        temp_r31 = &g_dComIfG_gameInfo + 0x52CC;
        this->unk89A = getMyActIdx__16dEvent_manager_cFiPCPCciii(temp_r31, arg0, &a_cut_tbl$5860, 1, 1, 0);
        if ((s8) this->unk89A == -1) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
            return;
        }
        if (getIsAddvance__16dEvent_manager_cFi(temp_r31, arg0) != 0) {
            if ((s32) this->unk89A != 0) {

            } else {
                event_actionInit__11daNpc_Ko1_cFi(this, arg0);
            }
        }
        if ((s32) this->unk89A != 0) {
            var_r3 = 1;
        } else {
            var_r3 = event_action__11daNpc_Ko1_cFv(this);
        }
        if (var_r3 != 0) {
            cutEnd__16dEvent_manager_cFi(temp_r31, arg0);
        }
    }
}

/* daNpc_Ko1_c::endEvent (void) */
void endEvent__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
    this->mAnmAtr = 0xFF;
    this->mAnmTag = 0xFF;
}

/* daNpc_Ko1_c::isEventEntry (void) */
void isEventEntry__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(&g_dComIfG_gameInfo + 0x52CC, this->mEventCut.unk0, NULL, 0);
}

/* daNpc_Ko1_c::event_proc (int) */
void event_proc__11daNpc_Ko1_cFi(daNpc_Ko1_c *this, s32 arg0) {
    if (cutProc__15dNpc_EventCut_cFv(&this->mEventCut) == 0) {
        privateCut__11daNpc_Ko1_cFi(this, arg0);
    }
    lookBack__11daNpc_Ko1_cFv(this);
}

s32 set_action__11daNpc_Ko1_cFM11daNpc_Ko1_cFPCvPvPv_iPv(daNpc_Ko1_c *arg0, s32 *arg1, s32 arg2, ? arg_sp0) {
    if (__ptmf_cmpr(arg0 + 0x730) != 0) {
        if (__ptmf_test(arg0 + 0x730) != 0) {
            arg0->unk8A8 = 9;
            __ptmf_scall(arg0, arg2);
        }
        arg0->unk730 = (s32) arg1->unk0;
        arg0->unk734 = arg1->unk4;
        arg0->unk738 = arg1->unk8;
        arg0->unk8A8 = 0;
        __ptmf_scall(arg0, arg2);
    }
    return 1;
}

/* daNpc_Ko1_c::clrSpd (void) */
void clrSpd__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    this->m_mov_spd = 0.0f;
    this->m_mov_spd_step = 0.0f;
    this->unk224 = 0.0f;
    this->unk254 = 0.0f;
    this->unk258 = -4.5f;
}

/* daNpc_Ko1_c::setStt (char signed) */
void setStt__11daNpc_Ko1_cFSc(daNpc_Ko1_c *this, s8 arg0, ? arg_sp0) {
    dNpc_PathRun_c sp14;
    dNpc_PathRun_c sp8;
    JUTAssertion *temp_r28;
    dPath *temp_r0;
    dPath *temp_r0_2;
    dPath *temp_r4;
    dPath *temp_r4_2;
    s8 *temp_r3;
    s8 *temp_r4_3;
    u8 temp_r30;
    u8 temp_r5;

    temp_r28 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id0);
    temp_r30 = this->unk8A3;
    this->unk84E = 0;
    this->unk8A3 = (u8) arg0;
    temp_r5 = this->unk8A3;
    if ((u32) (s8) temp_r5 <= 0x1DU) {
        switch ((s8) temp_r5) {                     /* switch 1 */
        case 1:                                     /* switch 1 */
        case 2:                                     /* switch 1 */
        case 12:                                    /* switch 1 */
        case 22:                                    /* switch 1 */
        case 29:                                    /* switch 1 */
            switch ((s8) temp_r5) {                 /* switch 2; irregular */
            case 1:                                 /* switch 2 */
                this->unk852 = (s16) (@4294.unk138 + cM_rndF__Ff(@4294.unk138));
                break;
            case 2:                                 /* switch 2 */
                this->unk854 = (g_Counter & 3) + 1;
                break;
            }
            if ((s8) temp_r30 != 3) {
                this->unk8A5 = 3;
                this->unk858 = this->unk7D2;
                this->unk29A = 1;
                this->unk876 = 0;
            }
            this->mOrderType = 0;
            this->unk898 = 0;
            clrSpd__11daNpc_Ko1_cFv(this);
            break;
        case 3:                                     /* switch 1 */
            this->unk8A5 = 1;
            this->unk29A = 1;
            this->unk876 = 0;
            this->mOrderType = 0;
            this->unk898 = 0;
            clrSpd__11daNpc_Ko1_cFv(this);
            this->mAnmAtr = 0xFF;
            this->mAnmTag = 0xFF;
            this->unk8A4 = temp_r30;
            break;
        case 4:                                     /* switch 1 */
        case 11:                                    /* switch 1 */
            switch ((s8) temp_r5) {                 /* switch 3; irregular */
            case 4:                                 /* switch 3 */
                this->unk852 = (s16) (@4294.unk13C + cM_rndF__Ff(@4294.unk13C));
                break;
            case 11:                                /* switch 3 */
                temp_r4 = this->unk744;
                if (temp_r4 != NULL) {
                    setInfDrct__14dNpc_PathRun_cFP5dPath(&this->mPathRun, temp_r4);
                    setNearPathIndx__14dNpc_PathRun_cFP4cXyzf(&this->mPathRun, (cXyz *) &this->unk1F8, @4294.unk8);
                    this->unk744 = NULL;
                }
                break;
            }
            getPoint__14dNpc_PathRun_cFUc(&sp14, (u8) &this->mPathRun);
            this->m_tgt_pos.unk0 = (bitwise f32) sp14;
            this->unk7F8 = sp18;
            this->unk7FC = sp1C;
            this->unk8A5 = 0;
            this->unk876 = 1;
            this->mOrderType = 0;
            this->unk898 = 1;
            this->unk86B = 0;
block_36:
            this->unk258 = @4294.unkC;
            this->m_mov_spd = l_HIO[(s8) this->unk8A6].unk34;
            this->m_mov_spd_step = l_HIO[(s8) this->unk8A6].unk38;
            this->unk83C = l_HIO[(s8) this->unk8A6].unk3C;
            break;
        case 5:                                     /* switch 1 */
            this->unk8A5 = 0;
            this->unk876 = 0;
            this->mOrderType = 0;
            this->unk898 = 0;
            clrSpd__11daNpc_Ko1_cFv(this);
            break;
        case 6:                                     /* switch 1 */
        case 13:                                    /* switch 1 */
        case 23:                                    /* switch 1 */
            if ((s8) temp_r5 != 6) {

            } else {
                temp_r0 = this->mPathRun.unk0;
                if (temp_r0 != NULL) {
                    this->unk744 = temp_r0;
                    setInfDrct__14dNpc_PathRun_cFP5dPath(&this->mPathRun, NULL);
                }
            }
            this->m_tgt_pos.unk0 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
            this->unk7F8 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
            this->unk7FC = g_dComIfG_gameInfo.unk5B4C->unk200;
            this->unk8A5 = 1;
            this->unk876 = 1;
            this->mOrderType = 0;
            this->unk898 = 2;
            this->unk86B = 0;
block_40:
            this->unk258 = @4294.unkC;
            this->m_mov_spd = l_HIO[(s8) this->unk8A6].unk44;
            this->m_mov_spd_step = l_HIO[(s8) this->unk8A6].unk48;
            this->unk83C = l_HIO[(s8) this->unk8A6].unk4C;
            break;
        case 7:                                     /* switch 1 */
            temp_r0_2 = this->mPathRun.unk0;
            if (temp_r0_2 != NULL) {
                this->unk744 = temp_r0_2;
                setInfDrct__14dNpc_PathRun_cFP5dPath(&this->mPathRun, NULL);
            }
            this->m_tgt_pos.unk0 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
            this->unk7F8 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
            this->unk7FC = g_dComIfG_gameInfo.unk5B4C->unk200;
            this->unk8A5 = 1;
            this->unk876 = 1;
block_32:
            this->mOrderType = 0;
            this->unk898 = 3;
            this->unk86B = 0;
            this->m_swm_spd_y = @4294.unk140;
            this->unk224 = this->m_swm_spd_y;
            this->unk254 = @4294.unk8;
            this->m_mov_spd_step = @4294.unk8;
            this->m_mov_spd = @4294.unk8;
            this->unk258 = @4294.unk8;
            this->unk83C = l_HIO[(s8) this->unk8A6].unk50;
            break;
        case 8:                                     /* switch 1 */
            temp_r4_2 = this->unk744;
            if (temp_r4_2 != NULL) {
                setInfDrct__14dNpc_PathRun_cFP5dPath(&this->mPathRun, temp_r4_2);
                setNearPathIndx__14dNpc_PathRun_cFP4cXyzf(&this->mPathRun, (cXyz *) &this->unk1F8, @4294.unk8);
                this->unk744 = NULL;
            }
            getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->mPathRun);
            this->m_tgt_pos.unk0 = (bitwise f32) sp8;
            this->unk7F8 = spC;
            this->unk7FC = sp10;
            this->unk8A5 = 0;
            this->unk876 = 1;
            goto block_32;
        case 9:                                     /* switch 1 */
        case 16:                                    /* switch 1 */
        case 18:                                    /* switch 1 */
        case 25:                                    /* switch 1 */
            this->unk8A5 = 0;
            this->unk876 = 0;
            this->mOrderType = 0;
            this->unk899 = this->unk898;
            this->unk898 = 4;
            this->unk86C = 1;
            this->unk224 = @4294.unk10;
            this->unk254 = @4294.unk144;
            this->unk258 = @4294.unk148;
            this->m_mov_spd_step = @4294.unk120;
            break;
        case 10:                                    /* switch 1 */
        case 14:                                    /* switch 1 */
        case 26:                                    /* switch 1 */
            this->unk8A5 = 1;
            this->unk876 = 0;
            this->mOrderType = 0;
            this->unk898 = 0;
            clrSpd__11daNpc_Ko1_cFv(this);
            break;
        case 15:                                    /* switch 1 */
        case 24:                                    /* switch 1 */
            this->m_tgt_pos.unk0 = this->unk7C4;
            this->unk7F8 = this->unk7C8;
            this->unk7FC = this->unk7CC;
            this->unk8A5 = 0;
            this->unk876 = 1;
            this->mOrderType = 0;
            this->unk898 = 1;
            this->unk86B = 0;
            goto block_36;
        case 17:                                    /* switch 1 */
            if (temp_r28 == NULL) {
                temp_r4_3 = "Ko1";
                showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(&jtbl_6033), (u32) (temp_r4_3 + 0xB), (s8 *)0xA73, (s32) (temp_r4_3 + 0x4F), M2C_ERROR(/* Read from unset register $r7 */));
                temp_r3 = "Ko1";
                OSPanic(temp_r3 + 0xB, 0xA73, temp_r3 + 0x30);
            }
            this->m_tgt_pos.unk0 = temp_r28->unk1F8;
            this->unk7F8 = temp_r28->unk1FC;
            this->unk7FC = temp_r28->unk200;
            this->unk8A5 = 2;
            this->unk7E8 = this->m_tgt_pos.unk0;
            this->unk7EC = this->unk7F8;
            this->unk7F0 = this->unk7FC;
            this->unk7EC = temp_r28->unk264;
            this->unk876 = 1;
            this->mOrderType = 0;
            this->unk898 = 2;
            this->unk86B = 0;
            goto block_40;
        case 19:                                    /* switch 1 */
            if (temp_r28 != NULL) {
                this->unk8A5 = 2;
                this->unk7E8 = temp_r28->unk1F8;
                this->unk7EC = temp_r28->unk1FC;
                this->unk7F0 = temp_r28->unk200;
                this->unk7EC = temp_r28->unk264;
            } else {
                this->unk8A5 = 0;
            }
            this->unk876 = 0;
            this->mOrderType = 0;
            this->unk898 = 0;
            clrSpd__11daNpc_Ko1_cFv(this);
            break;
        case 20:                                    /* switch 1 */
            this->mAnmAtr = 0xFF;
            this->mAnmTag = 0xFF;
            this->unk8A4 = temp_r30;
            break;
        case 27:                                    /* switch 1 */
        case 28:                                    /* switch 1 */
            this->unk852 = (s16) (@4294.unk13C + cM_rndF__Ff(@4294.unk13C));
            this->unk854 = cLib_getRndValue<i>__Fii(3, 0xA);
            this->unk865 = 0;
            break;
        }
    }
    setAnm__11daNpc_Ko1_cFv(&*this);
}

/* daNpc_Ko1_c::wait_1 (void) */
s32 wait_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    if ((u8) this->unk875 != 0) {
        if (chk_talk__11daNpc_Ko1_cFv(this) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    this->mOrderType = 2;
    this->unk8A5 = 3;
    this->unk858 = this->unk7D2;
    if ((s32) this->unk89F == 6) {
        if ((s8) this->unk860 != 0) {
            cLib_calcTimer<s>__FPs(&this->unk854);
        }
        if (((s16) this->unk854 == 0) || ((u8) this->unk874 != 0)) {
            this->unk854 = (g_Counter & 3) + 1;
            this->unk852 = (s16) (90.0f + cM_rndF__Ff(90.0f));
            setAnm_NUM__11daNpc_Ko1_cFii(&*this, 0, 1);
        }
        return 1;
    }
    if ((u8) this->unk874 != 0) {
        this->unk84E = 0x3C;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk84E) != 0) {
        this->unk8A5 = 1;
        return 1;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk852) == 0) {
        setAnm_NUM__11daNpc_Ko1_cFii(&*this, 6, 1);
    }
    this->unk29A = 1;
    return 1;
}

/* daNpc_Ko1_c::wait_2 (void) */
s32 wait_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    if (((s8) this->unk860 != 0) && ((cLib_calcTimer<s>__FPs(&this->unk854), (((s16) this->unk854 == 0) != 0)) || ((u8) this->unk874 != 0) || ((u8) this->unk875 != 0))) {
        setStt__11daNpc_Ko1_cFSc(this, 1);
        return 1;
    }
    return 1;
}

/* daNpc_Ko1_c::wait_3 (void) */
s32 wait_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    if ((s8) this->unk860 != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 4);
    }
    return 1;
}

/* daNpc_Ko1_c::wait_4 (void) */
s32 wait_4__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk875 != 0) {
        if (chk_talk__11daNpc_Ko1_cFv(this) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    if (chk_start_swim__11daNpc_Ko1_cFv(this) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 7);
        return 1;
    }
    sp14 = this->unk1F8;
    sp18 = this->unk1FC;
    sp1C = this->unk200;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk5C, (cXyz) &sp14) == 0) != 0) {
        sp8 = this->unk80C;
        spC = this->unk810;
        sp10 = this->unk814;
        if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk64, (cXyz) &sp8) == 0) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 0xB);
            return 1;
        }
        setStt__11daNpc_Ko1_cFSc(this, 6);
        return 1;
    }
    this->mOrderType = 2;
    return 1;
}

/* daNpc_Ko1_c::wait_5 (char signed) */
s32 wait_5__11daNpc_Ko1_cFSc(daNpc_Ko1_c *this, s8 arg0) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->m_manzai_stt == 1) {
        this->m_manzai_stt = 2;
        setStt__11daNpc_Ko1_cFSc(this, 0x14);
        this->unk8A5 = 1;
        this->unk876 = 0;
        this->mOrderType = 0;
        this->unk898 = 0;
        clrSpd__11daNpc_Ko1_cFv(this);
        return 1;
    }
    sp8 = this->unk80C;
    spC = this->unk810;
    sp10 = this->unk814;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk54, (cXyz) &sp8) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, arg0);
    }
    return 1;
}

/* daNpc_Ko1_c::wait_6 (void) */
s32 wait_6__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk875 != 0) {
        if ((chk_talk__11daNpc_Ko1_cFv(this) != 0) && (chk_manzai_1__11daNpc_Ko1_cFv(this) != 0)) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    if ((u8) this->m_manzai_stt == 1) {
        this->m_manzai_stt = 2;
        setStt__11daNpc_Ko1_cFSc(this, 0x14);
        return 1;
    }
    sp14 = this->unk1F8;
    sp18 = this->unk1FC;
    sp1C = this->unk200;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk5C, (cXyz) &sp14) == 0) != 0) {
        sp8 = this->unk80C;
        spC = this->unk810;
        sp10 = this->unk814;
        if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk58, (cXyz) &sp8) == 0) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 0xF);
            return 1;
        }
        setStt__11daNpc_Ko1_cFSc(this, 0xD);
        return 1;
    }
    this->mOrderType = 2;
    return 1;
}

/* daNpc_Ko1_c::wait_7 (void) */
s32 wait_7__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    JUTAssertion *temp_r3;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id0);
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0xB6B, (s32) (temp_r4 + 0x4F), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0xB6B, temp_r3_2 + 0x30);
    }
    if ((u8) this->unk875 != 0) {
        if ((chk_talk__11daNpc_Ko1_cFv(this) != 0) && (chk_manzai_1__11daNpc_Ko1_cFv(this) != 0)) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    if ((u8) this->m_manzai_stt == 1) {
        this->m_manzai_stt = 2;
        setStt__11daNpc_Ko1_cFSc(this, 0x14);
        this->unk8A5 = 1;
        this->unk876 = 0;
        this->mOrderType = 0;
        this->unk898 = 0;
        clrSpd__11daNpc_Ko1_cFv(this);
        return 1;
    }
    sp24 = this->unk1F8;
    sp28 = this->unk1FC;
    sp2C = this->unk200;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, *(&l_HIO->unk5C + ((s8) this->unk8A6 * 0x60)), (cXyz) &sp24) != 0) {
        this->unk8A5 = 1;
    } else {
        this->unk8A5 = 2;
        this->unk7E8 = temp_r3->unk1F8;
        this->unk7EC = temp_r3->unk1FC;
        this->unk7F0 = temp_r3->unk200;
        this->unk7EC = temp_r3->unk264;
    }
    __mi__4cXyzCFRC3Vec(&sp18, (Vec *) &temp_r3->unk1F8);
    spC = (bitwise f32) sp18;
    sp10 = @4294.unk8;
    sp14 = sp20;
    var_f1 = PSVECSquareMag(&spC);
    if (var_f1 > @4294.unk8) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    if (var_f1 < *(&l_HIO->unk5C + ((s8) this->unk8A6 * 0x60))) {
        this->mOrderType = 2;
    } else {
        setStt__11daNpc_Ko1_cFSc(this, 0x11);
    }
    return 1;
}

/* daNpc_Ko1_c::wait_9 (void) */
s32 wait_9__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk875 != 0) {
        if (chk_talk__11daNpc_Ko1_cFv(this) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    sp14 = this->unk1F8;
    sp18 = this->unk1FC;
    sp1C = this->unk200;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk5C, (cXyz) &sp14) == 0) != 0) {
        sp8 = this->unk80C;
        spC = this->unk810;
        sp10 = this->unk814;
        if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk58, (cXyz) &sp8) == 0) != 0) {
            setStt__11daNpc_Ko1_cFSc(this, 0x18);
            return 1;
        }
        setStt__11daNpc_Ko1_cFSc(this, 0x17);
        return 1;
    }
    this->mOrderType = 2;
    return 1;
}

/* daNpc_Ko1_c::wait_a (void) */
s32 wait_a__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id0);
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0xBC3, (s32) (temp_r4 + 0x4F), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0xBC3, temp_r3_2 + 0x30);
    }
    if ((u8) this->unk875 != 0) {
        if ((chk_talk__11daNpc_Ko1_cFv(this) != 0) && (chk_manzai_1__11daNpc_Ko1_cFv(this) != 0)) {
            setStt__11daNpc_Ko1_cFSc(this, 3);
        }
        return 1;
    }
    if ((u8) this->m_manzai_stt == 1) {
        this->m_manzai_stt = 2;
        setStt__11daNpc_Ko1_cFSc(this, 0x14);
        return 1;
    }
    this->mOrderType = 2;
    if ((u8) this->unk874 != 0) {
        this->unk84E = 0x3C;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk84E) != 0) {
        this->unk8A5 = 1;
    } else {
        this->unk8A5 = 3;
        this->unk858 = this->unk7D2;
        this->unk29A = 1;
    }
    return 1;
}

/* daNpc_Ko1_c::walk_1 (void) */
s32 walk_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    dNpc_PathRun_c sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;

    this->unk86B = 0;
    getPoint__14dNpc_PathRun_cFUc(&sp14, (u8) &this->mPathRun);
    this->m_tgt_pos.unk0 = (bitwise f32) sp14;
    this->unk7F8 = sp18;
    this->unk7FC = sp1C;
    sp8 = this->unk80C;
    spC = this->unk810;
    sp10 = this->unk814;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk60, (cXyz) &sp8) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 6);
        return 1;
    }
    if (cLib_calcTimer<s>__FPs(&this->unk852) == 0) {
        setStt__11daNpc_Ko1_cFSc(this, 5);
    }
    return 1;
}

/* daNpc_Ko1_c::walk_2 (char signed, char signed) */
s32 walk_2__11daNpc_Ko1_cFScSc(daNpc_Ko1_c *this, s8 arg0, s8 arg1) {
    f32 sp10;
    f32 spC;
    f32 sp8;

    if ((u8) this->unk86B != 0) {
        setStt__11daNpc_Ko1_cFSc(this, arg0);
        return 1;
    }
    if ((u8) this->m_manzai_stt == 1) {
        setStt__11daNpc_Ko1_cFSc(this, 0xE);
        return 1;
    }
    sp8 = this->unk80C;
    spC = this->unk810;
    sp10 = this->unk814;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk54, (cXyz) &sp8) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, arg1);
        return 1;
    }
    this->m_tgt_pos.unk0 = this->unk7C4;
    this->unk7F8 = this->unk7C8;
    this->unk7FC = this->unk7CC;
    return 1;
}

/* daNpc_Ko1_c::walk_3 (void) */
s32 walk_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    dNpc_PathRun_c sp8;

    if ((u8) this->unk86B != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 4);
        return 1;
    }
    sp14 = this->unk80C;
    sp18 = this->unk810;
    sp1C = this->unk814;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk60, (cXyz) &sp14) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 6);
        return 1;
    }
    getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->mPathRun);
    this->m_tgt_pos.unk0 = (bitwise f32) sp8;
    this->unk7F8 = spC;
    this->unk7FC = sp10;
    if (chk_start_swim__11daNpc_Ko1_cFv(this) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 8);
    }
    return 1;
}

/* daNpc_Ko1_c::swim_1 (void) */
s32 swim_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;

    temp_f1 = this->unk4F0 - this->unk1FC;
    if (temp_f1 < @4294.unk14C) {
        this->m_swm_spd_y = @4294.unk140;
        if ((u8) this->unk86B == 0) {
            this->unk254 = @4294.unk150;
        }
    } else if (temp_f1 > @4294.unk154) {
        this->m_swm_spd_y = @4294.unk158;
    }
    if (chk_start_swim__11daNpc_Ko1_cFv(this) == 0) {
        setStt__11daNpc_Ko1_cFSc(this, 6);
        return 1;
    }
    sp8 = this->unk80C;
    spC = this->unk810;
    sp10 = this->unk814;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk64, (cXyz) &sp8) == 0) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 8);
        return 1;
    }
    this->unk86B = 0;
    this->m_tgt_pos.unk0 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
    this->unk7F8 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
    this->unk7FC = g_dComIfG_gameInfo.unk5B4C->unk200;
    return 1;
}

/* daNpc_Ko1_c::swim_2 (void) */
s32 swim_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    dNpc_PathRun_c sp8;
    f32 temp_f1;

    temp_f1 = this->unk4F0 - this->unk1FC;
    if (temp_f1 < @4294.unk14C) {
        this->m_swm_spd_y = @4294.unk140;
        if ((u8) this->unk86B == 0) {
            this->unk254 = @4294.unk150;
        }
    } else if (temp_f1 > @4294.unk154) {
        this->m_swm_spd_y = @4294.unk158;
    }
    if (chk_start_swim__11daNpc_Ko1_cFv(this) == 0) {
        setStt__11daNpc_Ko1_cFSc(this, 0xB);
        return 1;
    }
    sp14 = this->unk80C;
    sp18 = this->unk810;
    sp1C = this->unk814;
    if (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk60, (cXyz) &sp14) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 7);
        return 1;
    }
    this->unk86B = 0;
    getPoint__14dNpc_PathRun_cFUc(&sp8, (u8) &this->mPathRun);
    this->m_tgt_pos.unk0 = (bitwise f32) sp8;
    this->unk7F8 = spC;
    this->unk7FC = sp10;
    return 1;
}

/* daNpc_Ko1_c::attk_1 (void) */
s32 attk_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    daNpc_Ko1_c sp8;
    JUTAssertion *temp_r3;
    u32 temp_r4;

    this->unk86B = 0;
    this->unk7C0 = get_crsActorID__11daNpc_Ko1_cFv(this);
    temp_r4 = this->unk7C0;
    if ((u32) (temp_r4 + 0x10000) != -1U) {
        temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, temp_r4);
        if ((temp_r3 != NULL) && ((s16) temp_r3->unk8 == 0xA9)) {
            setStt__11daNpc_Ko1_cFSc(this, 9);
            return 1;
        }
    }
    sp20 = this->unk80C;
    sp24 = this->unk810;
    sp28 = this->unk814;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk64, (cXyz) &sp20) == 0) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 0xB);
        return 1;
    }
    sp14 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
    sp18 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
    sp1C = g_dComIfG_gameInfo.unk5B4C->unk200;
    set_tgtPos__11daNpc_Ko1_cF4cXyz(&sp8, (cXyz) this);
    this->m_tgt_pos.unk0 = (bitwise f32) sp8;
    this->unk7F8 = spC;
    this->unk7FC = sp10;
    if (chk_start_swim__11daNpc_Ko1_cFv(this) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, 7);
    }
    return 1;
}

/* daNpc_Ko1_c::attk_2 (char signed, char signed) */
s32 attk_2__11daNpc_Ko1_cFScSc(daNpc_Ko1_c *this, s8 arg0, s8 arg1, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    JUTAssertion *temp_r3;
    u32 temp_r4;

    this->unk86B = 0;
    if ((u8) this->m_manzai_stt == 1) {
        setStt__11daNpc_Ko1_cFSc(this, 0xE);
        return 1;
    }
    this->unk7C0 = get_crsActorID__11daNpc_Ko1_cFv(this);
    temp_r4 = this->unk7C0;
    if ((u32) (temp_r4 + 0x10000) != -1U) {
        temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, temp_r4);
        if ((temp_r3 != NULL) && ((s16) temp_r3->unk8 == 0xA9)) {
            setStt__11daNpc_Ko1_cFSc(this, arg0);
            return 1;
        }
    }
    sp8 = this->unk80C;
    spC = this->unk810;
    sp10 = this->unk814;
    if ((u8) (chk_areaIn__11daNpc_Ko1_cFf4cXyz(this, l_HIO[(s8) this->unk8A6].unk58, (cXyz) &sp8) == 0) != 0) {
        setStt__11daNpc_Ko1_cFSc(this, arg1);
        return 1;
    }
    this->m_tgt_pos.unk0 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
    this->unk7F8 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
    this->unk7FC = g_dComIfG_gameInfo.unk5B4C->unk200;
    return 1;
}

/* daNpc_Ko1_c::attk_3 (void) */
s32 attk_3__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    daNpc_Ko1_c sp8;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r3_3;
    s16 temp_r0;
    s8 *temp_r3_2;
    s8 *temp_r4;
    u32 temp_r4_2;

    temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, this->m_partner_id0);
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0xCD2, (s32) (temp_r4 + 0x4F), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0xCD2, temp_r3_2 + 0x30);
    }
    this->unk86B = 0;
    if ((u8) this->m_manzai_stt == 1) {
        setStt__11daNpc_Ko1_cFSc(this, 0x13);
        return 1;
    }
    this->unk7C0 = get_crsActorID__11daNpc_Ko1_cFv(this);
    temp_r4_2 = this->unk7C0;
    if ((u32) (temp_r4_2 + 0x10000) != -1U) {
        temp_r3_3 = searchByID__11daNpc_Ko1_cFUi(this, temp_r4_2);
        if ((temp_r3_3 != NULL) && ((temp_r0 = temp_r3_3->unk8, ((temp_r0 == 0x142) != 0)) || (temp_r0 == 0xA9))) {
            setStt__11daNpc_Ko1_cFSc(this, 0x12);
            return 1;
        }
    }
    sp14 = temp_r3->unk1F8;
    sp18 = temp_r3->unk1FC;
    sp1C = temp_r3->unk200;
    set_tgtPos__11daNpc_Ko1_cF4cXyz(&sp8, (cXyz) this);
    this->m_tgt_pos.unk0 = (bitwise f32) sp8;
    this->unk7F8 = spC;
    this->unk7FC = sp10;
    this->unk7E8 = this->m_tgt_pos.unk0;
    this->unk7EC = this->unk7F8;
    this->unk7F0 = this->unk7FC;
    this->unk7EC = temp_r3->unk264;
    return 1;
}

/* daNpc_Ko1_c::down_1 (char signed) */
s32 down_1__11daNpc_Ko1_cFSc(daNpc_Ko1_c *this, s8 arg0) {
    if ((u8) this->unk86B != 0) {
        setStt__11daNpc_Ko1_cFSc(this, arg0);
    }
    return 1;
}

/* daNpc_Ko1_c::talk_1 (void) */
u8 talk_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    dSv_event_c *temp_r29;
    u16 temp_r0;
    u32 temp_r0_2;
    u8 temp_r0_3;
    u8 temp_r31;
    void *temp_r3;
    void *temp_r3_2;

    temp_r31 = chk_partsNotMove__11daNpc_Ko1_cFv(this);
    temp_r3 = this->unk6B0;
    if (temp_r3 != NULL) {
        this->unk6B8 = temp_r3->unkF8;
    } else {
        this->unk6B8 = 0;
    }
    if (((u8) this->m_manzai_stt == 2) && ((u32) (this->unk6AC + 0x10000) == -1U)) {
        this->unk6A4 = this->unk6C0->unkC(this);
        this->unk6AC = fopMsgM_messageSet__FUlP10fopAc_ac_c(this->unk6A4, g_dComIfG_gameInfo.unk5C28);
        this->unk6B0 = NULL;
    } else {
        this->unk848 = -1;
        talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1);
    }
    temp_r3_2 = this->unk6B0;
    if (temp_r3_2 != NULL) {
        temp_r0 = temp_r3_2->unkF8;
        if ((s32) temp_r0 != 6) {
            switch ((s32) temp_r0) {                /* switch 1; irregular */
            case 2:                                 /* switch 1 */
                break;
            case 19:                                /* switch 1 */
                temp_r0_2 = this->unk6A4;
                switch ((s32) temp_r0_2) {          /* switch 2; irregular */
                case 0xAF1:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x210U);
                    break;
                case 0xAFF:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3801U);
                    break;
                case 0xB06:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3340U);
                    break;
                case 0xB55:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x220U);
                    break;
                case 0xB57:                         /* switch 2 */
                    temp_r29 = &g_dComIfG_gameInfo + 0x624;
                    onEventBit__11dSv_event_cFUs(temp_r29, 0x208U);
                    onEventBit__11dSv_event_cFUs(temp_r29, 0x240U);
                    break;
                case 0xB62:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3101U);
                    break;
                case 0xB64:                         /* switch 2 */
                    onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x2C04U);
                    this->unk86D = 1;
                    break;
                }
                this->unk863 = 0xFF;
                this->unk875 = 0;
                temp_r0_3 = this->m_act_no;
                if ((s8) temp_r0_3 != 3) {
                    if ((s8) temp_r0_3 < 3) {
                        if ((s8) temp_r0_3 != 1) {
                            goto block_43;
                        }
                        goto block_42;
                    }
                    if ((s8) temp_r0_3 < 8) {
                        if ((s8) temp_r0_3 < 6) {
                            goto block_43;
                        }
                        goto block_42;
                    }
block_43:
                    setStt__11daNpc_Ko1_cFSc(this, (s8) this->unk8A4);
                    this->unk84E = 0x3C;
                    endEvent__11daNpc_Ko1_cFv(this);
                } else {
block_42:
                    setStt__11daNpc_Ko1_cFSc(this, 0x15);
                }
                break;
            }
        }
    }
    return temp_r31;
}

/* daNpc_Ko1_c::talk_2 (void) */
s32 talk_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    JUTAssertion *temp_r3;
    s32 var_r28;
    s32 var_r29;
    s32 var_r31;
    s8 *temp_r3_2;
    s8 *temp_r4;
    u8 temp_r0;

    var_r29 = 0;
    var_r28 = 0;
    var_r31 = 0;
loop_7:
    temp_r0 = this->m_partner_num;
    if (var_r28 < (s32) temp_r0) {
        temp_r3 = searchByID__11daNpc_Ko1_cFUi(this, *(this + (var_r31 + 0x7B4)));
        if (temp_r3 == NULL) {
            temp_r4 = "Ko1";
            showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0xD59, (s32) (temp_r4 + 0x35), M2C_ERROR(/* Read from unset register $r7 */));
            temp_r3_2 = "Ko1";
            OSPanic(temp_r3_2 + 0xB, 0xD59, temp_r3_2 + 0x30);
        }
        if ((u8) temp_r3->m_manzai_stt != 0) {
            temp_r3->m_manzai_stt = 3;
        } else {
            var_r29 += 1;
        }
        var_r28 += 1;
        var_r31 += 4;
        goto loop_7;
    }
    if (var_r29 == (s32) temp_r0) {
        this->m_manzai_stt = 0;
        setStt__11daNpc_Ko1_cFSc(this, (s8) this->unk8A4);
        endEvent__11daNpc_Ko1_cFv(this);
    }
    return 1;
}

/* daNpc_Ko1_c::manzai (void) */
s32 manzai__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    JUTAssertion *temp_r4;
    u8 temp_r0;
    u8 temp_r0_2;
    void *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x5C20;
    temp_r0 = this->m_manzai_stt;
    switch ((s32) temp_r0) {                        /* irregular */
    case 2:
        temp_r4 = searchByID__11daNpc_Ko1_cFUi(this, this->unk6B4);
        if (this != (u32) (temp_r30 + (temp_r30->unk4 * 4))->unk4) {
            if ((u8) this->mAnmAtr != 0xFF) {
                temp_r0_2 = this->m_act_no;
                if (((s8) temp_r0_2 == 1) || ((s8) temp_r0_2 == 6)) {
                    this->unk8A5 = 1;
                    this->unk29A = 1;
                } else {
                    this->unk8A5 = 3;
                    this->unk858 = this->unk7D2;
                    this->unk29A = 1;
                    this->unk8A3 = this->unk8A4;
                    setAnm__11daNpc_Ko1_cFv(&*this);
                    this->unk8A3 = 0x14;
                }
                this->mAnmAtr = 0xFF;
            }
        } else {
            this->unk848 = temp_r4->unk6A4;
            this->unk6C0->unk10(this, temp_r4->unk6B8);
        }
        break;
    case 3:
        this->unk1C4 &= 0xFFFFBFFF;
        setStt__11daNpc_Ko1_cFSc(this, (s8) this->unk8A4);
        this->m_manzai_stt = 0;
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::neru_1 (void) */
s32 neru_1__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 spC;
    s32 sp8;

    sp8 = @6522.unk0;
    spC = @6522.unk4;
    if (cLib_calcTimer<s>__FPs(&this->unk852) == 0) {
        this->unk865 ^= 1;
        setAnm_NUM__11daNpc_Ko1_cFii(this, (&sp8)[this->unk865], 1);
        this->unk852 = (s16) (180.0f + cM_rndF__Ff(180.0f));
    }
    return 1;
}

/* daNpc_Ko1_c::neru_2 (void) */
s32 neru_2__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp10;
    s32 spC;
    s32 sp8;
    u8 temp_r0;

    sp8 = @4294.unk164;
    spC = @4294.unk168;
    sp10 = @4294.unk16C;
    temp_r0 = this->unk865;
    switch ((s32) temp_r0) {                        /* irregular */
    case 0:
        if (cLib_calcTimer<s>__FPs(&this->unk852) == 0) {
            if (cLib_calcTimer<s>__FPs(&this->unk854) == 0) {
                this->unk865 = 1;
                setAnm_NUM__11daNpc_Ko1_cFii(this, (&sp8)[this->unk865], 1);
                set_balloonAnm_NUM__11daNpc_Ko1_cFi(this, 1);
                this->unk854 = cLib_getRndValue<i>__Fii(3, 0xA);
            }
            this->unk852 = (s16) (@4294.unk13C + cM_rndF__Ff(@4294.unk13C));
        }
        break;
    case 1:
        if ((s8) this->unk860 != 0) {
            this->unk865 = 2;
            setAnm_NUM__11daNpc_Ko1_cFii(this, (&sp8)[this->unk865], 1);
        } else if (checkPass__12J3DFrameCtrlFf(&this->mpMorf->unk58, @4294.unk170) != 0) {
            setPrtcl_HanaPachi__11daNpc_Ko1_cFv(this);
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x592CU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4294.unk14, @4294.unk14, @4294.unk18, @4294.unk18, 0U);
        }
        break;
    case 2:
        if ((s8) this->unk860 != 0) {
            this->unk865 = 0;
            setAnm_NUM__11daNpc_Ko1_cFii(this, (&sp8)[this->unk865], 1);
            set_balloonAnm_NUM__11daNpc_Ko1_cFi(this, 0);
        }
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::hana_action1 (void *) */
s32 hana_action1__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        ko_setPthPos__11daNpc_Ko1_cFv(this);
        setStt__11daNpc_Ko1_cFSc(this, 4);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2 */
        case 4:                                     /* switch 2 */
            this->unk870 = walk_1__11daNpc_Ko1_cFv(this);
            break;
        case 11:                                    /* switch 2 */
            this->unk870 = walk_3__11daNpc_Ko1_cFv(this);
            break;
        case 5:                                     /* switch 2 */
            this->unk870 = wait_3__11daNpc_Ko1_cFv(this);
            break;
        case 10:                                    /* switch 2 */
            this->unk870 = wait_4__11daNpc_Ko1_cFv(this);
            break;
        case 6:                                     /* switch 2 */
            this->unk870 = attk_1__11daNpc_Ko1_cFv(this);
            break;
        case 7:                                     /* switch 2 */
            this->unk870 = swim_1__11daNpc_Ko1_cFv(this);
            break;
        case 8:                                     /* switch 2 */
            this->unk870 = swim_2__11daNpc_Ko1_cFv(this);
            break;
        case 9:                                     /* switch 2 */
            this->unk870 = down_1__11daNpc_Ko1_cFSc(this, 0xA);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::hana_action2 (void *) */
s32 hana_action2__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 0x13);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2 */
        case 19:                                    /* switch 2 */
            this->unk870 = wait_7__11daNpc_Ko1_cFv(this);
            break;
        case 17:                                    /* switch 2 */
            this->unk870 = attk_3__11daNpc_Ko1_cFv(this);
            break;
        case 18:                                    /* switch 2 */
            this->unk870 = down_1__11daNpc_Ko1_cFSc(this, 0x13);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        case 21:                                    /* switch 2 */
            this->unk870 = talk_2__11daNpc_Ko1_cFv(this);
            break;
        case 20:                                    /* switch 2 */
            this->unk870 = manzai__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::hana_action3 (void *) */
s32 hana_action3__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 0x16);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2 */
        case 22:                                    /* switch 2 */
            this->unk870 = wait_5__11daNpc_Ko1_cFSc(this, 0x17);
            break;
        case 23:                                    /* switch 2 */
            this->unk870 = attk_2__11daNpc_Ko1_cFScSc(this, 0x19, 0x18);
            break;
        case 24:                                    /* switch 2 */
            this->unk870 = walk_2__11daNpc_Ko1_cFScSc(this, 0x16, 0x17);
            break;
        case 26:                                    /* switch 2 */
            this->unk870 = wait_9__11daNpc_Ko1_cFv(this);
            break;
        case 25:                                    /* switch 2 */
            this->unk870 = down_1__11daNpc_Ko1_cFSc(this, 0x1A);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::hana_action4 (void *) */
s32 hana_action4__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 0x1D);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 29:                                    /* switch 2 */
            this->unk870 = wait_a__11daNpc_Ko1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        case 21:                                    /* switch 2 */
            this->unk870 = talk_2__11daNpc_Ko1_cFv(this);
            break;
        case 20:                                    /* switch 2 */
            this->unk870 = manzai__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::hana_action5 (void *) */
s32 hana_action5__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* irregular */
    case 9:
        break;
    case 0:
        setStt__11daNpc_Ko1_cFSc(this, 0x1C);
        this->unk8A8 += 1;
        break;
    default:
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        if ((s32) this->unk8A3 != 0x1C) {

        } else {
            this->unk870 = neru_2__11daNpc_Ko1_cFv(&*this);
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::wait_action1 (void *) */
s32 wait_action1__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 1);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x104U) == 0) {
            check_landOn__11daNpc_Ko1_cFv(this);
        }
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 1:                                     /* switch 2 */
            this->unk870 = wait_1__11daNpc_Ko1_cFv(this);
            break;
        case 2:                                     /* switch 2 */
            this->unk870 = wait_2__11daNpc_Ko1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::wait_action2 (void *) */
s32 wait_action2__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 0xC);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2 */
        case 12:                                    /* switch 2 */
            this->unk870 = wait_5__11daNpc_Ko1_cFSc(this, 0xD);
            break;
        case 13:                                    /* switch 2 */
            this->unk870 = attk_2__11daNpc_Ko1_cFScSc(this, 0x10, 0xF);
            break;
        case 15:                                    /* switch 2 */
            this->unk870 = walk_2__11daNpc_Ko1_cFScSc(this, 0xC, 0xD);
            break;
        case 14:                                    /* switch 2 */
            this->unk870 = wait_6__11daNpc_Ko1_cFv(this);
            break;
        case 16:                                    /* switch 2 */
            this->unk870 = down_1__11daNpc_Ko1_cFSc(this, 0xE);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        case 21:                                    /* switch 2 */
            this->unk870 = talk_2__11daNpc_Ko1_cFv(this);
            break;
        case 20:                                    /* switch 2 */
            this->unk870 = manzai__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::wait_action3 (void *) */
s32 wait_action3__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;
    u8 temp_r0_2;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 9:                                         /* switch 1 */
        break;
    case 0:                                         /* switch 1 */
        setStt__11daNpc_Ko1_cFSc(this, 0x1D);
        this->unk8A8 += 1;
        break;
    default:                                        /* switch 1 */
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        temp_r0_2 = this->unk8A3;
        switch ((s8) temp_r0_2) {                   /* switch 2; irregular */
        case 29:                                    /* switch 2 */
            this->unk870 = wait_a__11daNpc_Ko1_cFv(this);
            break;
        case 3:                                     /* switch 2 */
            this->unk870 = talk_1__11daNpc_Ko1_cFv(this);
            break;
        case 21:                                    /* switch 2 */
            this->unk870 = talk_2__11daNpc_Ko1_cFv(this);
            break;
        case 20:                                    /* switch 2 */
            this->unk870 = manzai__11daNpc_Ko1_cFv(this);
            break;
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::wait_action4 (void *) */
s32 wait_action4__11daNpc_Ko1_cFPv(daNpc_Ko1_c *this, void *arg0) {
    u8 temp_r0;

    temp_r0 = this->unk8A8;
    switch ((s8) temp_r0) {                         /* irregular */
    case 9:
        break;
    case 0:
        setStt__11daNpc_Ko1_cFSc(this, 0x1B);
        this->unk8A8 += 1;
        break;
    default:
        this->unk874 = chkAttention__11daNpc_Ko1_cFv(this);
        if ((s32) this->unk8A3 != 0x1B) {

        } else {
            this->unk870 = neru_1__11daNpc_Ko1_cFv(&*this);
        }
        lookBack__11daNpc_Ko1_cFv(this);
        break;
    }
    return 1;
}

/* daNpc_Ko1_c::demo (void) */
u8 demo__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    s32 sp8;
    JUTAssertion *temp_r3_2;
    dDemo_actor_c *temp_r3;
    s16 temp_r5;

    if ((u8) this->unk1C0 == 0) {
        if ((u8) this->unk877 != 0) {
            this->unk877 = 0;
        }
    } else {
        this->unk877 = 1;
        temp_r3 = getActor__14dDemo_object_cFUc(g_dComIfG_gameInfo.unk5AC8 + 0x20, this->unk1C0);
        if (this->m_hed_tex_pttrn != NULL) {
            this->mBtpFrame += 1;
            temp_r5 = this->m_hed_tex_pttrn->unk6;
            if ((s32) this->mBtpFrame >= temp_r5) {
                this->mBtpFrame = (u8) temp_r5;
            }
        }
        temp_r3_2 = getP_BtpData__13dDemo_actor_cFPCc(temp_r3, "Ko1" + 8);
        if (temp_r3_2 != NULL) {
            this->m_hed_tex_pttrn = temp_r3_2;
            sp8 = 0;
            if (init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->mBtpAnm, this->mpHedMorf->unk50->unk4, (J3DAnmTexPattern *) this->m_hed_tex_pttrn, 1, 2, 1.0f, 0, -1, 1, M2C_ERROR(/* Unable to find stack arg 0x0 in block */)) != 0) {
                this->unk89E = 4;
                this->mBtpFrame = 0;
            }
        }
        dDemo_setDemoData__FP10fopAc_ac_cUcP14mDoExt_McaMorfPCciPUsUlSc((fopAc_ac_c *) this, 0x6AU, this->mpMorf, "Ko1" + 8, 0, NULL, 0U, 0);
    }
    return this->unk877;
}

/* daNpc_Ko1_c::shadowDraw (void) */
void shadowDraw__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f3;
    u32 temp_r4;
    void **temp_r5;

    temp_f3 = this->unk1FC;
    sp8 = this->unk1F8;
    spC = @4294.unkF0 + temp_f3;
    sp10 = this->unk200;
    this->unk70C = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unk70C, 1, this->mpMorf->unk50, (cXyz *) &sp8, @4294.unk174, @4294.unk110, temp_f3, this->unk3C8, &this->unk41C, &this->unk10C, 0, @4294.unk14, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    temp_r4 = this->unk70C;
    if (temp_r4 != 0) {
        temp_r5 = this->unk708;
        if (temp_r5 != NULL) {
            addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, temp_r4, (J3DModel *) temp_r5);
        }
        addReal__21dDlst_shadowControl_cFUlP8J3DModel(&g_dComIfG_gameInfo + 0x5F6C, this->unk70C, this->mpHedMorf->unk50);
    }
}

/* daNpc_Ko1_c::_draw (void) */
s32 _draw__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r28;
    J3DModel *temp_r31;
    J3DModelData *temp_r30;
    f32 temp_f1;
    mDoExt_McaMorf *temp_r3;
    u8 temp_r0;
    u8 temp_r0_2;
    void **temp_r4;

    temp_r31 = this->mpHedMorf->unk50;
    temp_r30 = temp_r31->unk4;
    temp_r28 = this->mpMorf->unk50;
    if (((u8) this->unk867 != 0) || ((u8) this->unk86A != 0)) {
        return 1;
    }
    settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &this->unk1F8, &this->unk10C);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r28, &this->unk10C);
    temp_r0 = this->unk8A6;
    switch ((s8) temp_r0) {                         /* switch 1; irregular */
    case 0:                                         /* switch 1 */
        entryDL__14mDoExt_McaMorfFv(this->mpMorf);
        break;
    case 1:                                         /* switch 1 */
        entryDL__14mDoExt_McaMorfFP16J3DMaterialTable(this->mpMorf, getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)0x1E, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */)));
        break;
    }
    entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->mBtpAnm, temp_r30, (s16) this->mBtpFrame);
    entryDL__14mDoExt_McaMorfFv(this->mpHedMorf);
    removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r30->unk58, this->unk720);
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r31, &this->unk10C);
    temp_r3 = this->mpBalloonMorf;
    if (temp_r3 != NULL) {
        entryDL__14mDoExt_McaMorfFv(temp_r3);
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, this->mpBalloonMorf->unk50, &this->unk10C);
    }
    temp_r4 = this->unk708;
    if (temp_r4 != NULL) {
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, (J3DModel *) temp_r4, &this->unk10C);
        mDoExt_modelEntryDL__FP8J3DModel((J3DModel *) this->unk708);
    }
    shadowDraw__11daNpc_Ko1_cFv(this);
    temp_r0_2 = this->unk8A6;
    switch ((s8) temp_r0_2) {                       /* switch 2; irregular */
    case 0:                                         /* switch 2 */
        dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x52U, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
        break;
    case 1:                                         /* switch 2 */
        dSnap_RegistFig__FUcP10fopAc_ac_cfff(0x51U, (fopAc_ac_c *) this, 1.0f, 1.0f, 1.0f);
        break;
    }
    if ((u8) l_HIO[(s8) this->unk8A6].unk28 != 0) {
        sp8 = this->m_tgt_pos.unk0;
        spC = this->unk7F8;
        sp10 = this->unk7FC;
        temp_f1 = this->unk264;
        spC = temp_f1;
        if ((s8) this->m_act_no != 0) {
            sp8 = this->unk80C;
            spC = this->unk810;
            sp10 = this->unk814;
            spC = temp_f1;
        } else {
            sp8 = this->unk80C;
            spC = this->unk810;
            sp10 = this->unk814;
            spC = temp_f1;
        }
        sp8 = this->unk1F8;
        spC = this->unk1FC;
        sp10 = this->unk200;
        spC = temp_f1;
    }
    return 1;
}

/* daNpc_Ko1_c::_execute (void) */
s32 _execute__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    cXyz sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    s32 spC;
    s32 sp8;
    cXyz *temp_r3;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s32 var_r4;
    struct _struct_l_HIO_0x60 *temp_r10;

    if ((u8) this->unk86E == 0) {
        this->unk7C4 = this->unk1F8;
        this->unk7C8 = this->unk1FC;
        this->unk7CC = this->unk200;
        this->unk7D0 = this->unk204;
        this->unk7D2 = this->unk206;
        this->unk7D4 = this->unk208;
        this->unk86E = 1;
    }
    temp_r10 = &l_HIO[(s8) this->unk8A6];
    sp8 = (s32) temp_r10->unk16;
    spC = (s32) temp_r10->unk20;
    setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, temp_r10->unk18, temp_r10->unk1A, temp_r10->unk1C, temp_r10->unk1E, temp_r10->unk10, temp_r10->unk12, temp_r10->unk14, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    if (((u8) this->unk867 != 0) && ((u8) this->unk1C0 == 0)) {
        return 1;
    }
    this->unk869 = 0;
    this->unk867 = 0;
    partner_srch__11daNpc_Ko1_cFv(this);
    checkOrder__11daNpc_Ko1_cFv(this);
    if (demo__11daNpc_Ko1_cFv(this) == 0) {
        var_r4 = -1;
        if (((u8) g_dComIfG_gameInfo.unk529A != 0) && ((u16) this->unkF8 != 1)) {
            var_r4 = isEventEntry__11daNpc_Ko1_cFv(this);
        }
        if (var_r4 >= 0) {
            event_proc__11daNpc_Ko1_cFi(this, var_r4);
        } else {
            __ptmf_scall(this, 0);
        }
        if ((u8) this->unk869 == 0) {
            ko_nMove__11daNpc_Ko1_cFv(this);
            fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, (cXyz *) &this->mStts);
        }
        temp_r3 = GetTriPla__4cBgSCFii(&g_dComIfG_gameInfo + 0x12A0, (s32) this->unk41E, (s32) this->unk41C.unk0);
        if (temp_r3 != NULL) {
            this->unk818.unk0 = temp_r3->unk0;
            this->unk81C = temp_r3->unk4;
            this->unk820 = temp_r3->unk8;
        }
        if ((u8) this->unk868 == 0) {
            this->unk7D6 = this->unk204;
            this->unk7D8 = this->unk206;
            this->unk7DA = this->unk208;
            this->unk20C = this->unk7D6;
            this->unk20E = this->unk7D8;
            this->unk210 = this->unk7DA;
        }
        __mi__4cXyzCFRC3Vec(&sp20, (Vec *) &this->unk1F8);
        sp14 = (bitwise f32) sp20;
        sp18 = @4294.unk8;
        sp1C = sp28;
        var_f1 = PSVECSquareMag(&sp14);
        if (var_f1 > @4294.unk8) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = @4294.unkF8 * temp_f0 * (@4294.unk100 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = @4294.unkF8 * temp_f0_2 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            sp10 = (f32) ((f64) var_f1 * (@4294.unkF8 * temp_f0_3 * (@4294.unk100 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = sp10;
        }
        if (var_f1 > @4294.unk198) {
            fopAcM_delete__FP10fopAc_ac_c((fopAc_ac_c *) this);
            return 1;
        }
        goto block_22;
    }
block_22:
    eventOrder__11daNpc_Ko1_cFv(this);
    setMtx__11daNpc_Ko1_cFb(this, 0U);
    if ((u8) this->unk877 == 0) {
        setCollision__12fopNpc_npc_cFff((fopNpc_npc_c *) this, @4294.unkE0, @4294.unk10C);
    }
    return 1;
}

/* daNpc_Ko1_c::_delete (void) */
s32 _delete__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    mDoExt_McaMorf *temp_r3;
    mDoExt_McaMorf *temp_r3_2;
    mDoExt_McaMorf *temp_r3_3;

    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk6C4, "Ko1" + 8);
    if ((u32) this->unkF0 != 0U) {
        temp_r3 = this->mpMorf;
        if (temp_r3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3);
        }
        temp_r3_2 = this->mpHedMorf;
        if (temp_r3_2 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3_2);
        }
        temp_r3_3 = this->mpBalloonMorf;
        if (temp_r3_3 != NULL) {
            stopZelAnime__14mDoExt_McaMorfFv(temp_r3_3);
        }
    }
    end__19dPa_rippleEcallBackFv(&this->unk878);
    return 1;
}

/* CheckCreateHeap (fopAc_ac_c *) */
void CheckCreateHeap__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    CreateHeap__11daNpc_Ko1_cFv((daNpc_Ko1_c *) arg0);
}

/* daNpc_Ko1_c::_create (void) */
s32 _create__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    s32 temp_r3;
    s32 var_r3;

    if (!(this->unk1C8 & 8)) {
        if (this != NULL) {
            __ct__11daNpc_Ko1_cFv(this);
        }
        this->unk1C8 |= 8;
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk6C4, "Ko1" + 8);
    if (temp_r3 != 4) {
        return temp_r3;
    }
    if (charDecide__11daNpc_Ko1_cFi(this, (s32) (u8) this->unkB0) == 0) {
        return 5;
    }
    if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) CheckCreateHeap__FP10fopAc_ac_c, a_size_tbl$6977[(s8) this->unk8A6]) == 0) {
        return 5;
    }
    this->unk22C = &this->mpMorf->unk50->unk24;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, @4294.unk19C, @4294.unkE8, @4294.unk19C, @4294.unk1A0, @4294.unk1A4, @4294.unk1A0);
    var_r3 = 5;
    if (createInit__11daNpc_Ko1_cFv(this) != 0) {
        var_r3 = temp_r3;
    }
    return var_r3;
}

/* daNpc_Ko1_c::create_Anm (void) */
JUTAssertion *create_Anm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
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
    s8 *temp_r3_9;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    s8 *temp_r4_4;
    u8 temp_r3_4;
    u8 temp_r3_6;
    u8 temp_r3_8;
    void *temp_r28;

    temp_r28 = &g_dComIfG_gameInfo + 0x1BFC0;
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)0x1B, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x10A6, (s32) (temp_r4 + 0x5E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0x10A6, temp_r3_2 + 0x30);
    }
    var_r29 = __nw__FUl(0xB4U);
    if (var_r29 != NULL) {
        temp_r8 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)0x16, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x15021222;
        var_r29 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(var_r29, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r3, NULL, NULL, (s32) temp_r8, 1.0f, 2, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    this->mpMorf = var_r29;
    temp_r3_3 = this->mpMorf;
    if (temp_r3_3 == NULL) {
        return NULL;
    }
    if ((J3DModel *) temp_r3_3->unk50 == NULL) {
        this->mpMorf = NULL;
        return NULL;
    }
    temp_r3_4 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0x6D);
    this->m_hed_jnt_num = temp_r3_4;
    if ((s8) this->m_hed_jnt_num < 0) {
        temp_r4_2 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_4), (u32) (temp_r4_2 + 0xB), (s8 *)0x10BA, (s32) (temp_r4_2 + 0x72), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_5 = "Ko1";
        OSPanic(temp_r3_5 + 0xB, 0x10BA, temp_r3_5 + 0x30);
    }
    temp_r3_6 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0x85);
    this->m_bbone_jnt_num = temp_r3_6;
    if ((s8) this->m_bbone_jnt_num < 0) {
        temp_r4_3 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_6), (u32) (temp_r4_3 + 0xB), (s8 *)0x10BD, (s32) (temp_r4_3 + 0x8E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_7 = "Ko1";
        OSPanic(temp_r3_7 + 0xB, 0x10BD, temp_r3_7 + 0x30);
    }
    temp_r3_8 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0xA3);
    this->m_armR2_jnt_num = temp_r3_8;
    if ((s8) this->m_armR2_jnt_num < 0) {
        temp_r4_4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_8), (u32) (temp_r4_4 + 0xB), (s8 *)0x10C0, (s32) (temp_r4_4 + 0xA9), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_9 = "Ko1";
        OSPanic(temp_r3_9 + 0xB, 0x10C0, temp_r3_9 + 0x30);
    }
    return temp_r3;
}

/* daNpc_Ko1_c::create_hed_Anm (void) */
JUTAssertion *create_hed_Anm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r8;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *var_r31;
    s8 *temp_r3_2;
    s8 *temp_r3_5;
    s8 *temp_r4;
    s8 *temp_r4_2;
    u8 temp_r3_4;
    void *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x1BFC0;
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *) (u16) a_hed_bdl_resID_tbl$7263[(s8) this->unk8A6], (u16) temp_r30, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x10D8, (s32) (temp_r4 + 0x5E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0x10D8, temp_r3_2 + 0x30);
    }
    var_r31 = __nw__FUl(0xB4U);
    if (var_r31 != NULL) {
        temp_r8 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *) (u16) a_hed_bck_resID_tbl$7264[(s8) this->unk8A6], (u16) temp_r30, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x11020022;
        var_r31 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(var_r31, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r3, NULL, NULL, (s32) temp_r8, 1.0f, 2, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    this->mpHedMorf = var_r31;
    temp_r3_3 = this->mpHedMorf;
    if (temp_r3_3 == NULL) {
        return NULL;
    }
    if ((J3DModel *) temp_r3_3->unk50 == NULL) {
        this->mpHedMorf = NULL;
        return NULL;
    }
    if ((s8) this->unk8A6 == 0) {
        temp_r3_4 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0xBE);
        this->m_hed_2_jnt_num = temp_r3_4;
        if ((s8) this->m_hed_2_jnt_num < 0) {
            temp_r4_2 = "Ko1";
            showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_4), (u32) (temp_r4_2 + 0xB), (s8 *)0x10ED, (s32) (temp_r4_2 + 0xC4), M2C_ERROR(/* Read from unset register $r7 */));
            temp_r3_5 = "Ko1";
            OSPanic(temp_r3_5 + 0xB, 0x10ED, temp_r3_5 + 0x30);
        }
    }
    return temp_r3;
}

/* daNpc_Ko1_c::create_bln_Anm (void) */
JUTAssertion *create_bln_Anm__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
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
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)0x1A, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x10FD, (s32) (temp_r4 + 0x5E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0x10FD, temp_r3_2 + 0x30);
    }
    var_r29 = __nw__FUl(0xB4U);
    if (var_r29 != NULL) {
        temp_r8 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)1, (u16) temp_r28, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        sp8 = -1;
        spC = 1;
        sp10 = 0;
        sp14 = 0x80000;
        sp18 = 0x11000022;
        var_r29 = __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(var_r29, (J3DModelData *)1, (mDoExt_McaMorfCallBack1_c *) temp_r3, NULL, NULL, (s32) temp_r8, 1.0f, 2, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    this->mpBalloonMorf = var_r29;
    temp_r3_3 = this->mpBalloonMorf;
    if (temp_r3_3 == NULL) {
        return NULL;
    }
    if ((J3DModel *) temp_r3_3->unk50 == NULL) {
        this->mpBalloonMorf = NULL;
        return NULL;
    }
    temp_r3_4 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0xD9);
    this->m_bln_loc_jnt_num = temp_r3_4;
    if ((s8) this->m_bln_loc_jnt_num < 0) {
        temp_r4_2 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_4), (u32) (temp_r4_2 + 0xB), (s8 *)0x1111, (s32) (temp_r4_2 + 0xE5), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_5 = "Ko1";
        OSPanic(temp_r3_5 + 0xB, 0x1111, temp_r3_5 + 0x30);
    }
    temp_r3_6 = getIndex__10JUTNameTabCFPCc(temp_r3->unk54, "Ko1" + 0xFC);
    this->m_bln_jnt_num = temp_r3_6;
    if ((s8) this->m_bln_jnt_num < 0) {
        temp_r4_3 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) temp_r3_6), (u32) (temp_r4_3 + 0xB), (s8 *)0x1114, (s32) (temp_r4_3 + 0x107), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_7 = "Ko1";
        OSPanic(temp_r3_7 + 0xB, 0x1114, temp_r3_7 + 0x30);
    }
    return temp_r3;
}

/* daNpc_Ko1_c::create_itm_Mdl (void) */
u8 create_itm_Mdl__11daNpc_Ko1_cFv(daNpc_Ko1_c *this) {
    JUTAssertion *temp_r3;
    s8 *temp_r3_2;
    s8 *temp_r4;

    if ((s32) this->unk8A6 != 1) {
        return 1U;
    }
    temp_r3 = getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci((dRes_control_c *) ("Ko1" + 8), (s8 *)0x19, (u16) (&g_dComIfG_gameInfo + 0x1BFC0), (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "Ko1";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) (temp_r4 + 0xB), (s8 *)0x1126, (s32) (temp_r4 + 0x5E), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "Ko1";
        OSPanic(temp_r3_2 + 0xB, 0x1126, temp_r3_2 + 0x30);
    }
    this->unk708 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3, 0x80000U, 0x11000022U);
    if ((void **) this->unk708 == NULL) {
        return 0U;
    }
    return 1U;
}

/* daNpc_Ko1_c::CreateHeap (void) */
s32 CreateHeap__11daNpc_Ko1_cFv(daNpc_Ko1_c *this, ? arg_sp0) {
    u16 sp10;
    s32 sp8;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r3_2;
    u16 var_r5;
    u16 var_r5_2;
    u16 var_r5_3;

    temp_r3 = create_Anm__11daNpc_Ko1_cFv(this);
    if (temp_r3 == NULL) {
        return 0;
    }
    temp_r3_2 = create_hed_Anm__11daNpc_Ko1_cFv(this);
    if (temp_r3_2 == NULL) {
        this->mpMorf = NULL;
        return 0;
    }
    sp10 = @7337;
    this->unk89E = *(&sp10 + (s8) this->unk8A6);
    if (iniTexPttrnAnm__11daNpc_Ko1_cFb(this, 0) == 0) {
        this->mpMorf = NULL;
        this->mpHedMorf = NULL;
        return 0;
    }
    if (((s8) this->unk8A6 == 0) && (create_bln_Anm__11daNpc_Ko1_cFv(this) == NULL)) {
        this->mpMorf = NULL;
        this->mpHedMorf = NULL;
        return 0;
    }
    if (create_itm_Mdl__11daNpc_Ko1_cFv(this) != 0) {
        if ((s8) this->unk8A6 == 0) {
            var_r5 = 0;
loop_15:
            if (var_r5 < (u16) temp_r3_2->unk28) {
                if ((s32) var_r5 == (s8) this->m_bln_loc_jnt_num) {
                    (*(this->mpBalloonMorf->unk50->unk4->unk2C + ((var_r5 * 4) & 0x3FFFC)))->unk8 = nodeCallBack_Bln__FP7J3DNodei;
                }
                var_r5 += 1;
                goto loop_15;
            }
            this->mpBalloonMorf->unk50->unk14 = this;
            var_r5_2 = 0;
loop_20:
            if (var_r5_2 < (u16) temp_r3_2->unk28) {
                if ((s32) var_r5_2 == (s8) this->m_hed_2_jnt_num) {
                    (*(this->mpHedMorf->unk50->unk4->unk2C + ((var_r5_2 * 4) & 0x3FFFC)))->unk8 = nodeCallBack_Hed__FP7J3DNodei;
                }
                var_r5_2 += 1;
                goto loop_20;
            }
            this->mpHedMorf->unk50->unk14 = this;
        }
        var_r5_3 = 0;
loop_27:
        if (var_r5_3 < (u16) temp_r3->unk28) {
            if (((s32) var_r5_3 == (s8) this->m_hed_jnt_num) || ((s32) var_r5_3 == (s8) this->m_bbone_jnt_num)) {
                (*(this->mpMorf->unk50->unk4->unk2C + ((var_r5_3 * 4) & 0x3FFFC)))->unk8 = nodeCallBack_Ko1__FP7J3DNodei;
            }
            var_r5_3 += 1;
            goto loop_27;
        }
        this->mpMorf->unk50->unk14 = this;
        SetWall__12dBgS_AcchCirFff((dBgS_AcchCir *) &this->unk4F8, 30.0f, 30.0f);
        sp8 = 0;
        Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(&this->unk334, (cXyz *) &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, (dBgS_AcchCir *) &this->unk4F8, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
        return 1;
    }
    this->mpMorf = NULL;
    this->mpHedMorf = NULL;
    this->mpBalloonMorf = NULL;
    return 0;
}

/* daNpc_Ko1_Create (fopAc_ac_c *) */
void daNpc_Ko1_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _create__11daNpc_Ko1_cFv((daNpc_Ko1_c *) arg0);
}

/* daNpc_Ko1_Delete (daNpc_Ko1_c *) */
void daNpc_Ko1_Delete__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0) {
    _delete__11daNpc_Ko1_cFv(arg0);
}

/* daNpc_Ko1_Execute (daNpc_Ko1_c *) */
void daNpc_Ko1_Execute__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0) {
    _execute__11daNpc_Ko1_cFv(arg0);
}

/* daNpc_Ko1_Draw (daNpc_Ko1_c *) */
void daNpc_Ko1_Draw__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0) {
    _draw__11daNpc_Ko1_cFv(arg0);
}

/* daNpc_Ko1_IsDelete (daNpc_Ko1_c *) */
s32 daNpc_Ko1_IsDelete__FP11daNpc_Ko1_c(daNpc_Ko1_c *arg0) {
    return 1;
}

/* d_a_npc_ko1_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_ko1_cpp(void) {
    __ct__15daNpc_Ko1_HIO_cFv((daNpc_Ko1_HIO_c *) l_HIO);
    __register_global_object(__dt__15daNpc_Ko1_HIO_cFv, @4167);
}