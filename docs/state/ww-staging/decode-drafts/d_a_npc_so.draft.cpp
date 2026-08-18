// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in naming-map-auto.md;
// remaining unkNNN are still inference-needed. Behavior identical to the full draft.
// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_npc_so (123 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_npc_so\asm\d\actor\d_a_npc_so.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DModel {
    /* 0x00 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ J3DModelData *unk4;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x08 */ char pad8[0xC];                      /* maybe part of unk4[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x14 */ daNpc_So_c *unk14;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x14 (receipt f_op_actor.h) */
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
    /* 0x00 */ JAIZelBasic *unk0;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x04 */ char pad4[0xBB];                     /* maybe part of unk0[0x2F]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0xBF */ s8 unkBF;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xBF (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0xC0 */

typedef struct JUTAssertion {
    /* 0x00 */ char pad0[0x2C];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x2C */ void *unk2C;                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
} JUTAssertion;                                     /* size >= 0x30 */

typedef struct Vec {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} Vec;                                              /* size >= 0x4 */

struct __vt__10dNpc_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(dNpc_HIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__14daNpc_So_HIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(daNpc_So_HIO_c *, s16);
};                                                  /* size = 0xC */

struct __vt__14mDoHIO_entry_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(mDoHIO_entry_c *, s16);
};                                                  /* size = 0xC */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct dEvt_control_c {
    /* 0x00 */ char pad0[0xCC];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0xCC */ s32 unkCC;                           /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 +0x8 (receipt f_op_actor.h) */
} dEvt_control_c;                                   /* size >= 0xD0 */

typedef struct dKy_tevstr_c {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dKy_tevstr_c;                                     /* size >= 0x4 */

typedef struct daNpc_So_HIO_c {
    /* 0x00 */ struct __vt__14mDoHIO_entry_c *vtable0; /* inferred */
    /* 0x04 */ struct __vt__10dNpc_HIO_c *vtable4;  /* inferred */
    /* 0x08 */ f32 unk8;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x0C */ s16 unkC;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
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
    /* 0x24 */ s16 unk24;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x24 (receipt f_op_actor.h) */
    /* 0x26 */ u8 unk26;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x26 (receipt f_op_actor.h) */
    /* 0x27 */ char pad27[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x27 (receipt f_op_actor.h) */
    /* 0x28 */ f32 unk28;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x28 (receipt f_op_actor.h) */
    /* 0x2C */ u8 unk2C;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x2D */ s8 unk2D;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2D (receipt f_op_actor.h) */
    /* 0x2E */ u8 unk2E;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2E (receipt f_op_actor.h) */
    /* 0x2F */ u8 unk2F;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2F (receipt f_op_actor.h) */
    /* 0x30 */ u8 unk30;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x31 */ u8 unk31;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x31 (receipt f_op_actor.h) */
    /* 0x32 */ char pad32[2];                       /* maybe part of unk31[3]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x32 (receipt f_op_actor.h) */
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
    /* 0x60 */ f32 unk60;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x60 (receipt f_op_actor.h) */
    /* 0x64 */ s16 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x66 */ s16 unk66;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x66 (receipt f_op_actor.h) */
    /* 0x68 */ s16 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
    /* 0x6A */ s16 unk6A;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6A (receipt f_op_actor.h) */
    /* 0x6C */ f32 unk6C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6C (receipt f_op_actor.h) */
    /* 0x70 */ f32 unk70;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x70 (receipt f_op_actor.h) */
    /* 0x74 */ f32 unk74;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x74 (receipt f_op_actor.h) */
    /* 0x78 */ f32 unk78;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x78 (receipt f_op_actor.h) */
    /* 0x7C */ s16 unk7C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x7C (receipt f_op_actor.h) */
    /* 0x7E */ char pad7E[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x7E (receipt f_op_actor.h) */
    /* 0x80 */ f32 unk80;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x80 (receipt f_op_actor.h) */
    /* 0x84 */ f32 unk84;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x84 (receipt f_op_actor.h) */
    /* 0x88 */ f32 unk88;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x88 (receipt f_op_actor.h) */
    /* 0x8C */ f32 unk8C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8C (receipt f_op_actor.h) */
    /* 0x90 */ f32 unk90;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x90 (receipt f_op_actor.h) */
    /* 0x94 */ u8 unk94;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x94 (receipt f_op_actor.h) */
    /* 0x95 */ char pad95[3];                       /* maybe part of unk94[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x95 (receipt f_op_actor.h) */
    /* 0x98 */ JntHit_HIO_c unk98;                  /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x98 (receipt f_op_actor.h) */
    /* 0x98 */ char pad98[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x98 (receipt f_op_actor.h) */
} daNpc_So_HIO_c;                                   /* size >= 0x99 */

typedef struct daNpc_So_c {
    /* 0x000 */ s8 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x001 */ char pad1[3];                       /* maybe part of unk0[4]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x004 */ u32 unk4;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x008 */ dCcD_SrcSph unk8;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x008 */ char pad8[0x40];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x048 */ f32 unk48;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x48 (receipt f_op_actor.h) */
    /* 0x04C */ f32 unk4C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4C (receipt f_op_actor.h) */
    /* 0x050 */ f32 unk50;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x054 */ f32 unk54;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x058 */ f32 unk58;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x58 (receipt f_op_actor.h) */
    /* 0x05C */ f32 unk5C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5C (receipt f_op_actor.h) */
    /* 0x060 */ f32 unk60;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x60 (receipt f_op_actor.h) */
    /* 0x064 */ f32 unk64;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x068 */ f32 unk68;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
    /* 0x06C */ f32 unk6C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x6C (receipt f_op_actor.h) */
    /* 0x070 */ f32 unk70;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x70 (receipt f_op_actor.h) */
    /* 0x074 */ f32 unk74;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x74 (receipt f_op_actor.h) */
    /* 0x078 */ f32 unk78;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x78 (receipt f_op_actor.h) */
    /* 0x07C */ f32 unk7C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x7C (receipt f_op_actor.h) */
    /* 0x080 */ f32 unk80;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x80 (receipt f_op_actor.h) */
    /* 0x084 */ f32 unk84;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x84 (receipt f_op_actor.h) */
    /* 0x088 */ f32 unk88;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x88 (receipt f_op_actor.h) */
    /* 0x08C */ f32 unk8C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8C (receipt f_op_actor.h) */
    /* 0x090 */ f32 unk90;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x90 (receipt f_op_actor.h) */
    /* 0x094 */ f32 unk94;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x94 (receipt f_op_actor.h) */
    /* 0x098 */ f32 unk98;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x98 (receipt f_op_actor.h) */
    /* 0x09C */ f32 unk9C;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x9C (receipt f_op_actor.h) */
    /* 0x0A0 */ f32 unkA0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA0 (receipt f_op_actor.h) */
    /* 0x0A4 */ f32 unkA4;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA4 (receipt f_op_actor.h) */
    /* 0x0A8 */ f32 unkA8;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA8 (receipt f_op_actor.h) */
    /* 0x0AC */ f32 unkAC;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xAC (receipt f_op_actor.h) */
    /* 0x0B0 */ f64 unkB0;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB0 (receipt f_op_actor.h) */
    /* 0x0B8 */ f64 unkB8;                          /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xB8 (receipt f_op_actor.h) */
    /* 0x0C0 */ f32 unkC0;                          /* inferred */  /* = fopAc_ac_c::int actor_type @0xC0 (receipt f_op_actor.h) */
    /* 0x0C4 */ f32 unkC4;                          /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 (receipt f_op_actor.h) */
    /* 0x0C8 */ f32 unkC8;                          /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 +0x4 (receipt f_op_actor.h) */
    /* 0x0CC */ f32 unkCC;                          /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 +0x8 (receipt f_op_actor.h) */
    /* 0x0D0 */ f32 unkD0;                          /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 +0xC (receipt f_op_actor.h) */
    /* 0x0D4 */ f32 unkD4;                          /* inferred */  /* = fopAc_ac_c::create_tag_class actor_tag @0xC4 +0x10 (receipt f_op_actor.h) */
    /* 0x0D8 */ f32 unkD8;                          /* inferred */  /* = fopAc_ac_c::create_tag_class draw_tag @0xD8 (receipt f_op_actor.h) */
    /* 0x0DC */ f32 unkDC;                          /* inferred */  /* = fopAc_ac_c::create_tag_class draw_tag @0xD8 +0x4 (receipt f_op_actor.h) */
    /* 0x0E0 */ char padE0[4];  /* = fopAc_ac_c::create_tag_class draw_tag @0xD8 +0x8 (receipt f_op_actor.h) */
    /* 0x0E4 */ f32 unkE4;                          /* inferred */  /* = fopAc_ac_c::create_tag_class draw_tag @0xD8 +0xC (receipt f_op_actor.h) */
    /* 0x0E8 */ f32 unkE8;                          /* inferred */  /* = fopAc_ac_c::create_tag_class draw_tag @0xD8 +0x10 (receipt f_op_actor.h) */
    /* 0x0EC */ f32 unkEC;                          /* inferred */  /* = fopAc_ac_c::actor_method_class* sub_method @0xEC (receipt f_op_actor.h) */
    /* 0x0F0 */ f32 unkF0;                          /* inferred */  /* = fopAc_ac_c::JKRSolidHeap* heap @0xF0 (receipt f_op_actor.h) */
    /* 0x0F4 */ f32 unkF4;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 (receipt f_op_actor.h) */
    /* 0x0F8 */ f32 unkF8;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x4 (receipt f_op_actor.h) */
    /* 0x0FC */ f32 unkFC;                          /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x8 (receipt f_op_actor.h) */
    /* 0x100 */ void (*unk100)(void *, s32);        /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0xC (receipt f_op_actor.h) */
    /* 0x104 */ void (*unk104)(void *, s32);        /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x10 (receipt f_op_actor.h) */
    /* 0x108 */ f32 unk108;                         /* inferred */  /* = fopAc_ac_c::dEvt_info_c eventInfo @0xF4 +0x14 (receipt f_op_actor.h) */
    /* 0x10C */ dKy_tevstr_c unk10C;                /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x10C */ char pad10C[4];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x110 */ f32 unk110;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x4 (receipt f_op_actor.h) */
    /* 0x114 */ f32 unk114;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x8 (receipt f_op_actor.h) */
    /* 0x118 */ f32 unk118;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0xC (receipt f_op_actor.h) */
    /* 0x11C */ f32 unk11C;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x10 (receipt f_op_actor.h) */
    /* 0x120 */ char pad120[4];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x14 (receipt f_op_actor.h) */
    /* 0x124 */ f32 unk124;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x18 (receipt f_op_actor.h) */
    /* 0x128 */ f32 unk128;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x1C (receipt f_op_actor.h) */
    /* 0x12C */ f32 unk12C;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x20 (receipt f_op_actor.h) */
    /* 0x130 */ f32 unk130;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x24 (receipt f_op_actor.h) */
    /* 0x134 */ f32 unk134;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x28 (receipt f_op_actor.h) */
    /* 0x138 */ f32 unk138;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x2C (receipt f_op_actor.h) */
    /* 0x13C */ f32 unk13C;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x30 (receipt f_op_actor.h) */
    /* 0x140 */ f32 unk140;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x34 (receipt f_op_actor.h) */
    /* 0x144 */ f32 unk144;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x38 (receipt f_op_actor.h) */
    /* 0x148 */ s32 unk148;                         /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x3C (receipt f_op_actor.h) */
    /* 0x14C */ char pad14C[0x10];                  /* maybe part of unk148[5]? */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x40 (receipt f_op_actor.h) */
    /* 0x15C */ dLib_anm_prm_c unk15C;              /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x50 (receipt f_op_actor.h) */
    /* 0x15C */ char pad15C[0x64];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x50 (receipt f_op_actor.h) */
    /* 0x1C0 */ f64 unk1C0;                         /* inferred */  /* = fopAc_ac_c::u8 demoActorID @0x1C0 (receipt f_op_actor.h) */
    /* 0x1C8 */ f32 unk1C8;                         /* inferred */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1CC */ f32 unk1CC;                         /* inferred */  /* = fopAc_ac_c::fpc_ProcID parentActorID @0x1CC (receipt f_op_actor.h) */
    /* 0x1D0 */ f32 unk1D0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 (receipt f_op_actor.h) */
    /* 0x1D4 */ f32 unk1D4;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x4 (receipt f_op_actor.h) */
    /* 0x1D8 */ f32 unk1D8;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x8 (receipt f_op_actor.h) */
    /* 0x1DC */ f32 unk1DC;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0xC (receipt f_op_actor.h) */
    /* 0x1E0 */ f32 unk1E0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x10 (receipt f_op_actor.h) */
    /* 0x1E4 */ cXyz unk1E4;                        /* inferred */  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1E4 */ char pad1E4[0x14];  /* = fopAc_ac_c::actor_place old @0x1E4 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[2];  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x206 */ s16 unk206;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xE (receipt f_op_actor.h) */
    /* 0x208 */ char pad208[2];  /* = fopAc_ac_c::actor_place current @0x1F8 +0x10 (receipt f_op_actor.h) */
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
    /* 0x220 */ char pad220[4];  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x224 */ f32 unk224;                         /* inferred */  /* = fopAc_ac_c::cXyz speed @0x220 +0x4 (receipt f_op_actor.h) */
    /* 0x228 */ char pad228[4];  /* = fopAc_ac_c::cXyz speed @0x220 +0x8 (receipt f_op_actor.h) */
    /* 0x22C */ mDoMtx_stack_c *unk22C;             /* inferred */  /* = fopAc_ac_c::MtxP cullMtx @0x22C (receipt f_op_actor.h) */
    /* 0x230 */ char pad230[0x18];                  /* maybe part of unk22C[7]? */  /* = fopAc_ac_c::fopAc_cullSizeSphere sphere @0x230 (receipt f_op_actor.h) */
    /* 0x248 */ f32 unk248;                         /* inferred */  /* = fopAc_ac_c::f32 cullSizeFar @0x248 (receipt f_op_actor.h) */
    /* 0x24C */ char pad24C[4];  /* = fopAc_ac_c::J3DModel* model @0x24C (receipt f_op_actor.h) */
    /* 0x250 */ u32 unk250;                         /* inferred */  /* = fopAc_ac_c::JntHit_c* jntHit @0x250 (receipt f_op_actor.h) */
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
    /* 0x298 */ u8 unk298;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x299 */ u8 unk299;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29A */ u8 unk29A;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29B */ char pad29B[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29C */ s8 unk29C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29D */ char pad29D[0x27];                  /* maybe part of unk29C[0x28]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ dNpc_EventCut_c unk2C4;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2C4 */ char pad2C4[0x54];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x318 */ f32 unk318;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x31C */ f32 unk31C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x320 */ f32 unk320;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x324 */ u8 unk324;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x325 */ char pad325[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x326 */ s16 unk326;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x328 */ char pad328[0x210];                 /* maybe part of unk326[0x109]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ dCcD_Stts unk538;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x538 */ char pad538[0x3C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ dCcD_Cyl unk574;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x574 */ char pad574[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5B8 */ dCcD_Stts *unk5B8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x5BC */ char pad5BC[0xF4];                  /* maybe part of unk5B8[0x3E]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B0 */ void *unk6B0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6B4 */ char pad6B4[0x18];                  /* maybe part of unk6B0[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6CC */ s32 mProcNo;                         /* inferred */  /* [NAMED: modeProc(Proc_e,i) case 0 stores the requested proc (draft:2140), case 1 dispatches the ptmf table by it *0x1C (draft:2144)] */
    /* 0x6D0 */ s16 unk6D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D2 */ s8 mBckIdx;                          /* inferred */  /* [NAMED: passed as pBckIdx (s8*) to dLib_bcks_setAnm (draft:1348; signature receipt d_lib.h)] */
    /* 0x6D3 */ u8 mPrmIdx;                          /* inferred */  /* [NAMED: setAnm arg store with 6=keep sentinel (draft:1336) AND passed as pPrmIdx to dLib_bcks_setAnm (draft:1348; d_lib.h)] */
    /* 0x6D4 */ s8 unk6D4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D5 */ char pad6D5[3];                     /* maybe part of unk6D4[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6D8 */ s32 unk6D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ dCcD_Stts unk6DC;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6DC */ char pad6DC[0x1C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ dCcD_GStts unk6F8;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x6F8 */ char pad6F8[0x20];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x718 */ dCcD_GObjInf unk718;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x718 */ char pad718[0x44];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x75C */ dCcD_Stts *unk75C;                  /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x760 */ char pad760[0x84];                  /* maybe part of unk75C[0x22]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ cXyz unk7E4;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x7E4 */ char pad7E4[0x4C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x830 */ cM3dGSph unk830;                    /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x830 */ char pad830[0x14];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x844 */ request_of_phase_process_class unk844; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x844 */ char pad844[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x84C */ mDoExt_McaMorf *mpMorf;             /* inferred */  /* [NAMED: the morf arg of dLib_bcks_setAnm (draft:1348; d_lib.h) -- So's body mDoExt_McaMorf*; donor-wide mpMorf idiom] */
    /* 0x850 */ J3DModel *unk850;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x854 */ mDoExt_btpAnm unk854;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x854 */ char pad854[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x858 */ void *unk858;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x85C */ J3DAnmTexPattern *unk85C;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x860 */ char pad860[8];                     /* maybe part of unk85C[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x868 */ s32 unk868;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86C */ s16 unk86C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x86E */ char pad86E[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x870 */ dBgS_ObjAcch mAcch;                /* inferred */  /* [NAMED: dBgS_ObjAcch& arg of dLib_getWaterY (draft:1461); donor-wide dBgS_ObjAcch member name mAcch (53 of 63 donor actor headers)] */
    /* 0x870 */ char pad870[0x28];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x898 */ s32 unk898;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x89C */ char pad89C[0x68];                  /* maybe part of unk898[0x1B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x904 */ f32 unk904;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x908 */ char pad908[0x50];                  /* maybe part of unk904[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x958 */ cBgS_PolyInfo unk958;               /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x958 */ char pad958[0xDC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA34 */ dBgS_AcchCir unkA34;                /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA34 */ char padA34[0x40];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA74 */ u32 unkA74;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA78 */ u8 unkA78;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA79 */ u8 unkA79;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA7A */ char padA7A[2];                     /* maybe part of unkA79[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA7C */ f32 unkA7C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA80 */ f32 unkA80;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA84 */ f32 unkA84;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA88 */ f32 unkA88;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA8C */ char padA8C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA90 */ s32 unkA90;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA94 */ char padA94[8];                     /* maybe part of unkA90[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA9C */ s32 unkA9C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAA0 */ char padAA0[8];                     /* maybe part of unkA9C[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAA8 */ u32 unkAA8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAAC */ ? unkAAC;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAAC */ char padAAC[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAB0 */ f32 unkAB0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAB4 */ f32 unkAB4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAB8 */ char padAB8[0x30];                  /* maybe part of unkAB4[0xD]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAE8 */ dPa_levelEcallBack unkAE8;          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAE8 */ char padAE8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAEC */ u32 unkAEC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAF0 */ char padAF0[8];                     /* maybe part of unkAEC[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAF8 */ f32 unkAF8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xAFC */ f32 unkAFC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB00 */ f32 unkB00;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB04 */ f32 unkB04;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB08 */ f32 unkB08;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB0C */ u8 unkB0C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB0D */ char padB0D[3];                     /* maybe part of unkB0C[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB10 */ f32 unkB10;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB14 */ f32 unkB14;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB18 */ f32 unkB18;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB1C */ cXyz unkB1C;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB1C */ char padB1C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB20 */ f32 unkB20;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB24 */ f32 unkB24;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB28 */ f32 unkB28;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB2C */ f32 unkB2C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB30 */ s16 unkB30;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB32 */ s16 unkB32;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB34 */ f32 unkB34;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB38 */ f32 unkB38;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB3C */ f32 unkB3C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB40 */ f32 unkB40;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB44 */ f32 unkB44;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB48 */ f32 unkB48;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB4C */ f32 unkB4C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB50 */ s16 unkB50;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB52 */ char padB52[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB54 */ cXyz unkB54;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB54 */ char padB54[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB58 */ f32 unkB58;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB5C */ f32 unkB5C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB60 */ cXyz unkB60;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB60 */ char padB60[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB64 */ f32 unkB64;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB68 */ f32 unkB68;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB6C */ s32 unkB6C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB70 */ u8 mOrderType;                          /* inferred */  /* [NAMED: eventOrder selector (draft:2153), cleared by checkOrder on event ack (draft:2192,2204) -- the ba1 SS257 template rule hand-applied; the auto regex missed it because So loads via temp_r5] */
    /* 0xB71 */ char padB71[3];                     /* maybe part of mOrderType[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB74 */ s32 unkB74;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB78 */ s32 unkB78;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB7C */ s32 unkB7C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB80 */ s32 unkB80;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB84 */ u8 unkB84;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB85 */ char padB85[3];                     /* maybe part of unkB84[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB88 */ f32 unkB88;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB8C */ f32 unkB8C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB90 */ f32 unkB90;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB94 */ f32 unkB94;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB98 */ f32 unkB98;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB9C */ s16 unkB9C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xB9E */ char padB9E[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA0 */ f32 unkBA0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA4 */ f32 unkBA4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBA8 */ f32 unkBA8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBAC */ s16 unkBAC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBAE */ u8 unkBAE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBAF */ char padBAF[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBB0 */ f32 unkBB0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBB4 */ f32 unkBB4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBB8 */ f32 unkBB8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBBC */ s32 unkBBC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBC0 */ f32 unkBC0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBC4 */ f32 unkBC4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBC8 */ f32 unkBC8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBCC */ f32 unkBCC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBD0 */ f32 unkBD0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBD4 */ f32 unkBD4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBD8 */ u8 unkBD8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBD9 */ u8 unkBD9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBDA */ u8 unkBDA;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBDB */ u8 unkBDB;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBDC */ s16 unkBDC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBDE */ u8 unkBDE;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBDF */ char padBDF[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xBE0 */ s32 unkBE0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daNpc_So_c;                                       /* size >= 0xBE4 */

typedef struct daShip_c {
    /* 0x000 */ char pad0[0x1F8];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[8];                     /* maybe part of unk200[3]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x20C */ s32 unk20C;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
    /* 0x210 */ u16 unk210;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x212 */ char pad212[0x86];                  /* maybe part of unk210[0x44]? */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x6 (receipt f_op_actor.h) */
    /* 0x298 */ s8 unk298;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daShip_c;                                         /* size >= 0x299 */

typedef struct fopAc_ac_c {
    /* 0x000 */ char pad0[8];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x008 */ s16 unk8;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x00A */ char padA[0x1EE];                   /* maybe part of unk8[0xF8]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xA (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[0xA];                   /* maybe part of unk200[3]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x20E */ s16 unk20E;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x2 (receipt f_op_actor.h) */
    /* 0x210 */ char pad210[0x80];                  /* maybe part of unk20E[0x41]? */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x290 */ u8 unk290;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x291 */ char pad291[3];                     /* maybe part of unk290[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x294 */ f32 unk294;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x298 */ u8 unk298;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} fopAc_ac_c;                                       /* size >= 0x299 */

typedef struct mDoExt_McaMorf {
    /* 0x00 */ char pad0[0x50];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x50 */ J3DModel *unk50;                     /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x50 (receipt f_op_actor.h) */
    /* 0x54 */ char pad54[9];                       /* maybe part of unk50[3]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x54 (receipt f_op_actor.h) */
    /* 0x5D */ u8 unk5D;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5D (receipt f_op_actor.h) */
    /* 0x5E */ char pad5E[2];                       /* maybe part of unk5D[3]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5E (receipt f_op_actor.h) */
    /* 0x60 */ s16 unk60;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x60 (receipt f_op_actor.h) */
    /* 0x62 */ char pad62[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x62 (receipt f_op_actor.h) */
    /* 0x64 */ f32 unk64;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x64 (receipt f_op_actor.h) */
    /* 0x68 */ f32 unk68;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x68 (receipt f_op_actor.h) */
} mDoExt_McaMorf;                                   /* size >= 0x6C */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x10 */

u32 ChkTgHit__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
? CrrPos__9dBgS_AcchFR4dBgS(dBgS_Acch *this, dBgS *arg0); /* extern */
void *GetTgHitObj__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
u32 JntHit_create__FP8J3DModelP16__jnt_hit_data_cs(J3DModel *arg0, __jnt_hit_data_c *arg1, s16 arg2); /* extern */
? Move__10dCcD_GSttsFv(dCcD_GStts *this);           /* extern */
? MtxPosition__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
JUTAssertion *OSPanic(s8 *, ?, s8 *);               /* extern */
? PSMTXCopy(mDoMtx_stack_c *, mDoMtx_stack_c *);    /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, f32 *, cXyz *, f32); /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32);      /* extern */
? PSVECAdd(f32 *, f32 *, f32 *);                    /* extern */
f32 PSVECSquareMag(cXyz *);                         /* extern */
? Reset__9dCamera_cF4cXyz4cXyz(dCamera_c *this, cXyz arg0, cXyz arg1); /* extern */
? SetC__8cM3dGSphFRC4cXyz(cM3dGSph *this, cXyz *arg0); /* extern */
? SetR__8cM3dGSphFf(cM3dGSph *this, f32 arg0);      /* extern */
? SetTrimSize__9dCamera_cFl(dCamera_c *this, s32 arg0); /* extern */
? SetTypeForce__9dCamera_cFPcP10fopAc_ac_c(dCamera_c *this, s8 *arg0, fopAc_ac_c *arg1); /* extern */
? SetWall__12dBgS_AcchCirFff(dBgS_AcchCir *this, f32 arg0, f32 arg1); /* extern */
? Set__4cCcSFP8cCcD_Obj(cCcS *this, cCcD_Obj *arg0); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? Set__8dCcD_SphFRC11dCcD_SrcSph(dCcD_Sph *this, dCcD_SrcSph *arg0); /* extern */
? Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz(dBgS_Acch *this, cXyz *arg0, cXyz *arg1, fopAc_ac_c *arg2, s32 arg3, dBgS_AcchCir *arg4, cXyz *arg5, csXyz *arg6, csXyz *arg7); /* extern */
? Set__9dCamera_cF4cXyz4cXyz(dCamera_c *this, cXyz arg0, cXyz arg1); /* extern */
? Start__9dCamera_cFv(dCamera_c *this);             /* extern */
? Stop__9dCamera_cFv(dCamera_c *this);              /* extern */
void *__ct__12JntHit_HIO_cFv(JntHit_HIO_c *this);   /* extern */
mDoExt_McaMorf *__ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_cP25mDoExt_McaMorfCallBack2_cP15J3DAnmTransformifiiiPvUlUl(mDoExt_McaMorf *this, J3DModelData *arg0, mDoExt_McaMorfCallBack1_c *arg1, mDoExt_McaMorfCallBack2_c *arg2, J3DAnmTransform *arg3, s32 arg4, f32 arg5, s32 arg6, s32 arg7, s32 arg8, void *arg9, u32 arg10, u32 arg11); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
? __ml__4cXyzCFf(cXyz *this, f32 arg0);             /* extern */
mDoExt_McaMorf *__nw__FUl(u32 arg0);                /* extern */
? __pl__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
? __ptmf_scall(s32, s32);                           /* extern */
? __register_global_object(void *(*)(daNpc_So_HIO_c *, s16), void *(*)(cXyz *, s16), void *); /* extern */
? cLib_addCalc2__FPffff(f32 *arg0, f32 arg1, f32 arg2, f32 arg3); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
? cLib_addCalcPos2__FP4cXyzRC4cXyzff(cXyz *arg0, cXyz *arg1, f32 arg2, f32 arg3); /* extern */
s32 cLib_chasePosXZ__FP4cXyzRC4cXyzf(cXyz *arg0, cXyz *arg1, f32 arg2); /* extern */
s32 cLib_distanceAngleS__Fss(s16 arg0, s16 arg1);   /* extern */
s16 cLib_targetAngleY__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
f32 cM_rndF__Ff(f32 arg0);                          /* extern */
? calc__14mDoExt_McaMorfFv(mDoExt_McaMorf *this);   /* extern */
? cutEnd__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
u8 cutProc__15dNpc_EventCut_cFv(dNpc_EventCut_c *this); /* extern */
dCamera_c *dCam_getBody__Fv();                      /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s32 dComIfG_resLoad__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
u32 dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(u32 arg0, s8 arg1, J3DModel *arg2, cXyz *arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, cBgS_PolyInfo *arg8, dKy_tevstr_c *arg9, s16 arg10, f32 arg11, _GXTexObj *arg12); /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
u8 dComIfGs_checkGetItem__FUc(u8 arg0);             /* extern */
s32 dComIfGs_isStageBossEnemy__Fi(s32 arg0);        /* extern */
? dLib_bcks_setAnm__FPCcP14mDoExt_McaMorfPScPScPScPCiPC14dLib_anm_prm_cb(s8 *arg0, mDoExt_McaMorf *arg1, s8 *arg2, s8 *arg3, s8 *arg4, s32 *arg5, dLib_anm_prm_c *arg6, s32 arg7); /* extern */
f32 dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch(cXyz *arg0, dBgS_ObjAcch *arg1); /* extern */
? dLib_setCirclePath__FP18dLib_circle_path_c(dLib_circle_path_c *arg0); /* extern */
? dNpc_playerEyePos__Ff(f32 arg0);                  /* extern */
? dSnap_RegistFig__FUcP10fopAc_ac_cRC3Vecsfff(u8 arg0, fopAc_ac_c *arg1, Vec *arg2, s16 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
s32 endCheckOld__16dEvent_manager_cFPCc(dEvent_manager_c *this, s8 *arg0); /* extern */
s32 endCheck__16dEvent_manager_cFs(dEvent_manager_c *this, s16 arg0); /* extern */
? end__19dPa_rippleEcallBackFv(dPa_rippleEcallBack *this); /* extern */
? entryDL__14mDoExt_McaMorfFv(mDoExt_McaMorf *this); /* extern */
? entry__13mDoExt_btpAnmFP12J3DModelDatas(mDoExt_btpAnm *this, J3DModelData *arg0, s16 arg1); /* extern */
daShip_c *fopAcIt_Judge__FPFPvPv_PvPv(void *(*arg0)(void *, void *), void *arg1); /* extern */
u8 fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl(fopAc_ac_c *arg0, s32 (*arg1)(fopAc_ac_c *), u32 arg2); /* extern */
? fopAcM_orderChangeEvent__FP10fopAc_ac_cPcUsUs(fopAc_ac_c *arg0, s8 *arg1, u16 arg2, u16 arg3); /* extern */
? fopAcM_orderOtherEvent2__FP10fopAc_ac_cPcUsUs(fopAc_ac_c *arg0, s8 *arg1, u16 arg2, u16 arg3); /* extern */
? fopAcM_orderSpeakEvent__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* extern */
? fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz(fopAc_ac_c *arg0, cXyz *arg1); /* extern */
f32 fopAcM_searchActorDistanceXZ__FP10fopAc_ac_cP10fopAc_ac_c(fopAc_ac_c *arg0, fopAc_ac_c *arg1); /* extern */
? fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff(fopAc_ac_c *arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6); /* extern */
? fopKyM_createWpillar__FPC4cXyzffi(cXyz *arg0, f32 arg1, f32 arg2, s32 arg3); /* extern */
void *fpcSch_JudgeForPName__FPvPv(void *arg0, void *arg1); /* extern */
s16 getEventIdx__16dEvent_manager_cFPCcUc(dEvent_manager_c *this, s8 *arg0, u8 arg1); /* extern */
s32 getIsAddvance__16dEvent_manager_cFi(dEvent_manager_c *this, s32 arg0); /* extern */
s32 getMyActIdx__16dEvent_manager_cFiPCPCciii(dEvent_manager_c *this, s32 arg0, s8 **arg1, s32 arg2, s32 arg3, s32 arg4); /* extern */
s32 getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(dEvent_manager_c *this, s8 *arg0, fopAc_ac_c *arg1, s32 arg2); /* extern */
f32 *getMySubstanceP__16dEvent_manager_cFiPCci(dEvent_manager_c *this, s32 arg0, s8 *arg1, s32 arg2); /* extern */
s32 getPId__14dEvt_control_cFPv(dEvt_control_c *this, void *arg0); /* extern */
JUTAssertion *getRes__14dRes_control_cFPCclP11dRes_info_ci(dRes_control_c *this, s8 *arg0, s32 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 getTriforceNum__20dSv_player_collect_cFv(dSv_player_collect_c *this); /* extern */
? initStartPos__8daShip_cFPC4cXyzs(daShip_c *this, cXyz *arg0, s16 arg1); /* extern */
JUTAssertion *init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(mDoExt_btpAnm *this, J3DModelData *arg0, J3DAnmTexPattern *arg1, s32 arg2, s32 arg3, f32 arg4, s16 arg5, s16 arg6, s32 arg7, s32 arg8); /* extern */
s32 isCollect__20dSv_player_collect_cFiUc(dSv_player_collect_c *this, s32 arg0, u8 arg1); /* extern */
s32 isEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
s32 isSaveArriveGrid__16dSv_player_map_cFi(dSv_player_map_c *this, s32 arg0); /* extern */
? lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb(dNpc_JntCtrl_c *this, s16 *arg0, cXyz *arg1, cXyz arg2, s16 arg3, s16 arg4, u8 arg5); /* extern */
J3DModel *mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_modelUpdateDL__FP8J3DModel(J3DModel *arg0); /* extern */
? mDoMtx_XrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_YrotS__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? mDoMtx_ZXYrotM__FPA4_fsss(f32 (*arg0)[4], s16 arg1, s16 arg2, s16 arg3); /* extern */
? mDoMtx_ZrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, u32 arg3, s8 arg4); /* extern */
? onEventBit__11dSv_event_cFUs(dSv_event_c *this, u16 arg0); /* extern */
? play__14mDoExt_McaMorfFP3VecUlSc(mDoExt_McaMorf *this, Vec *arg0, u32 arg1, s8 arg2); /* extern */
? removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(J3DMaterialTable *this, J3DAnmTexPattern *arg0); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(dNpc_EventCut_c *this, s8 *arg0, fopNpc_npc_c *arg1); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? setParam__14dNpc_JntCtrl_cFsssssssss(dNpc_JntCtrl_c *this, s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7, s16 arg8); /* extern */
void *set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(dPa_control_c *this, u8 arg0, u16 arg1, cXyz *arg2, csXyz *arg3, cXyz *arg4, u8 arg5, dPa_levelEcallBack *arg6, s8 arg7, _GXColor *arg8, _GXColor *arg9, cXyz *arg10); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
s32 strcmp(s8 *, s8 *);                             /* extern */
u16 talk__12fopNpc_npc_cFi(fopNpc_npc_c *this, s32 arg0); /* extern */
? transM__14mDoMtx_stack_cFfff(mDoMtx_stack_c *this, f32 arg0, f32 arg1, f32 arg2); /* extern */
s32 XyCheckCB__10daNpc_So_cFi(daNpc_So_c *this, s32 arg0); /* static */
s16 XyEventCB__10daNpc_So_cFi(daNpc_So_c *this, s32 arg0); /* static */
void *__ct__10daNpc_So_cFv(daNpc_So_c *this);       /* static */
void *__dt__10dNpc_HIO_cFv(dNpc_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__14daNpc_So_HIO_cFv(daNpc_So_HIO_c *this, s16 destroyFlag); /* static */
void *__dt__14mDoHIO_entry_cFv(mDoHIO_entry_c *this, s16 destroyFlag); /* static */
void *__dt__4cXyzFv(cXyz *this, s16 destroyFlag);   /* static */
s32 _createHeap__10daNpc_So_cFv(daNpc_So_c *this);  /* static */
void _nodeControl__10daNpc_So_cFP7J3DNodeP8J3DModel(daNpc_So_c *this, J3DNode *arg0, J3DModel *arg1); /* static */
fopAc_ac_c *_searchEsa__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0); /* static */
fopAc_ac_c *_searchMinigameTagSo__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0); /* static */
fopAc_ac_c *_searchTagSo__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0); /* static */
s32 cLib_calcTimer<i>__FPi(s32 *arg0);              /* static */
void cutAppearProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutAppearStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutDisappearProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutDisappearStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutDiveProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutDiveStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEatesaFirstProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEatesaFirstStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEatesaProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEatesaStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEffectProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEffectStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEquipProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutEquipStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutJumpMapopenProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutJumpMapopenStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutJumpProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutJumpStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameEndProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameEndStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGamePlTurnProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGamePlTurnStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGamePlUpProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGamePlUpStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameReturnProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameReturnStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameWaitProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameWaitStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameWarpProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutMiniGameWarpStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutPartnerShipProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutPartnerShipStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutProc__10daNpc_So_cFv(daNpc_So_c *this);     /* static */
void cutSetAnmProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutSetAnmStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutSwimProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutSwimStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutTurnProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutTurnStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutUnequipProc__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void cutUnequipStart__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void daNpc_SoCreate__FPv(void *arg0);               /* static */
u8 daNpc_SoDelete__FPv(void *arg0);                 /* static */
u8 daNpc_SoDraw__FPv(void *arg0);                   /* static */
u8 daNpc_SoExecute__FPv(void *arg0);                /* static */
s32 daNpc_SoIsDelete__FPv(void *arg0);              /* static */
? getAttnPos__15dNpc_EventCut_cFv(dNpc_EventCut_c *this); /* static */
s16 getMsg__10daNpc_So_cFv(daNpc_So_c *this);       /* static */
void initCam__10daNpc_So_cFv(daNpc_So_c *this);     /* static */
u8 jntHitCreateHeap__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeDebugInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeDebug__10daNpc_So_cFv(daNpc_So_c *this);   /* static */
void modeDisappearInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeDisappear__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventBowInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventBow__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventEsaInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventEsa__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirstEndInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirstEnd__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirstInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirstWaitInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirstWait__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventFirst__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventMapopenInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventMapopen__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventTriForceInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeEventTriForce__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeGetRupeeInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeGetRupee__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeHideInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeHide__10daNpc_So_cFv(daNpc_So_c *this);    /* static */
void modeJumpInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeJump__10daNpc_So_cFv(daNpc_So_c *this);    /* static */
void modeNearSwimInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeNearSwim__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(daNpc_So_c *this, daNpc_So_c::Proc_e arg0, s32 arg1); /* static */
void modeSwimInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeSwim__10daNpc_So_cFv(daNpc_So_c *this);    /* static */
void modeTalkInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeTalk__10daNpc_So_cFv(daNpc_So_c *this);    /* static */
void modeWaitInit__10daNpc_So_cFv(daNpc_So_c *this); /* static */
void modeWait__10daNpc_So_cFv(daNpc_So_c *this);    /* static */
void moveCam__10daNpc_So_cFv(daNpc_So_c *this);     /* static */
s32 next_msgStatus__10daNpc_So_cFPUl(daNpc_So_c *this, u32 *arg0); /* static */
void *@4259(cXyz *this, s16 destroyFlag);           /* static */
extern f32 (*calc_mtx)[4];
extern dCcD_SrcCyl dNpc_cyl_src;
extern ? g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? g_regHIO;
extern ? j3dSys;
extern s32 jmaCosTable;
extern s32 jmaSinShift;
extern s32 jmaSinTable;
extern J3DSys mCurrentMtx__6J3DSys;
extern dDlst_shadowControl_c mSimpleTexObj__21dDlst_shadowControl_c;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? @2100;                                     /* unable to generate initializer: unknown type */
static __jnt_hit_data_c search_data$4469;           /* unable to generate initializer: confusing struct layout */
static ? a_demo_name_tbl$6640;                      /* unable to generate initializer: unknown type */
static ? @3569;
static daNpc_So_HIO_c l_HIO;
static u8 init$4529;
static u8 init$6629;
static daNpc_So_c m_heapsize__10daNpc_So_c;         /* unable to generate initializer: confusing struct layout; const */

/* daNpc_So_HIO_c::daNpc_So_HIO_c (void) */
daNpc_So_HIO_c *__ct__14daNpc_So_HIO_cFv(daNpc_So_HIO_c *this) {
    this->vtable0 = &__vt__14mDoHIO_entry_c;
    this->vtable0 = (struct __vt__14mDoHIO_entry_c *) &__vt__14daNpc_So_HIO_c;
    this->vtable4 = &__vt__10dNpc_HIO_c;
    __ct__12JntHit_HIO_cFv(&this->unk98);
    this->unk2C = 0;
    this->unk2D = 0;
    this->unk2E = 0;
    this->unk2F = 0;
    this->unk30 = 0;
    this->unk31 = 0;
    this->unk34 = m_heapsize__10daNpc_So_c.unk48;
    this->unk38 = m_heapsize__10daNpc_So_c.unk4C;
    this->unk3C = m_heapsize__10daNpc_So_c.unk50;
    this->unk40 = m_heapsize__10daNpc_So_c.unk54;
    this->unk54 = m_heapsize__10daNpc_So_c.unk58;
    this->unk5C = m_heapsize__10daNpc_So_c.unk5C;
    this->unk44 = m_heapsize__10daNpc_So_c.unk60;
    this->unk48 = m_heapsize__10daNpc_So_c.unk54;
    this->unk4C = m_heapsize__10daNpc_So_c.unk64;
    this->unk50 = m_heapsize__10daNpc_So_c.unk68;
    this->unk64 = 0x1F40;
    this->unk66 = 0xFA0;
    this->unk68 = -0x1F40;
    this->unk6A = -0xFA0;
    this->unk6C = m_heapsize__10daNpc_So_c.unk6C;
    this->unk8 = m_heapsize__10daNpc_So_c.unk70;
    this->unkC = 0x1FFE;
    this->unk10 = 0x1000;
    this->unkE = 0x1000;
    this->unk12 = 0x2000;
    this->unk14 = -0xBB8;
    this->unk18 = -0x1000;
    this->unk16 = -0x1000;
    this->unk1A = -0x2000;
    this->unk1C = 0x250;
    this->unk1E = 0x150;
    this->unk20 = m_heapsize__10daNpc_So_c.unk74;
    this->unk24 = 0x7FFF;
    this->unk26 = 0;
    this->unk28 = m_heapsize__10daNpc_So_c.unk78;
    this->unk58 = m_heapsize__10daNpc_So_c.unk7C;
    this->unk60 = m_heapsize__10daNpc_So_c.unk80;
    this->unk70 = m_heapsize__10daNpc_So_c.unk84;
    this->unk74 = m_heapsize__10daNpc_So_c.unk88;
    this->unk78 = m_heapsize__10daNpc_So_c.unk50;
    this->unk7C = 5;
    this->unk80 = m_heapsize__10daNpc_So_c.unk8C;
    this->unk84 = m_heapsize__10daNpc_So_c.unk90;
    this->unk88 = m_heapsize__10daNpc_So_c.unk94;
    this->unk8C = m_heapsize__10daNpc_So_c.unk88;
    this->unk90 = m_heapsize__10daNpc_So_c.unk98;
    this->unk94 = 0;
    return this;
}

/* searchEsa_CB (void *, void *) */
void searchEsa_CB__FPvPv(void *arg0, void *arg1) {
    _searchEsa__10daNpc_So_cFP10fopAc_ac_c((daNpc_So_c *) arg1, (fopAc_ac_c *) arg0);
}

/* daNpc_So_c::_searchEsa (fopAc_ac_c *) */
fopAc_ac_c *_searchEsa__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0) {
    if (((s16) arg0->unk8 == 0xDE) && ((u8) arg0->unk298 == 0)) {
        return arg0;
    }
    return NULL;
}

/* nodeControl_CB (J3DNode *, int) */
s32 nodeControl_CB__FP7J3DNodei(J3DNode *arg0, s32 arg1) {
    daNpc_So_c *temp_r3;

    if (arg1 == 0) {
        temp_r3 = j3dSys.unk38->unk14;
        if (temp_r3 != NULL) {
            _nodeControl__10daNpc_So_cFP7J3DNodeP8J3DModel(temp_r3, arg0, j3dSys.unk38);
        }
    }
    return 1;
}

/* daNpc_So_c::_nodeControl (J3DNode *, J3DModel *) */
void _nodeControl__10daNpc_So_cFP7J3DNodeP8J3DModel(daNpc_So_c *this, J3DNode *arg0, J3DModel *arg1, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    s32 temp_r30;
    u16 temp_r28;

    temp_r28 = arg0->unk18;
    temp_r30 = temp_r28 * 0x30;
    PSMTXCopy(arg1->unk8C + temp_r30, &now__14mDoMtx_stack_c);
    if ((s32) temp_r28 == (s8) this->unk298) {
        sp14 = m_heapsize__10daNpc_So_c.unk9C;
        sp18 = m_heapsize__10daNpc_So_c.unk9C;
        sp1C = m_heapsize__10daNpc_So_c.unk9C;
        sp8 = m_heapsize__10daNpc_So_c.unkA0;
        spC = m_heapsize__10daNpc_So_c.unkA4;
        sp10 = m_heapsize__10daNpc_So_c.unk9C;
        PSMTXMultVec(&now__14mDoMtx_stack_c, &sp14, &this->unkB60, m_heapsize__10daNpc_So_c.unk9C);
        mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk292);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk290);
        PSMTXMultVec(&now__14mDoMtx_stack_c, &sp8, &this->unkB54);
    } else if ((s32) temp_r28 == (s8) this->unk299) {
        mDoMtx_XrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk296);
        mDoMtx_ZrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk294);
    }
    PSMTXCopy(&now__14mDoMtx_stack_c, (mDoMtx_stack_c *) &mCurrentMtx__6J3DSys);
    PSMTXCopy(&now__14mDoMtx_stack_c, arg1->unk8C + temp_r30);
}

/* searchTagSo_CB (void *, void *) */
void searchTagSo_CB__FPvPv(void *arg0, void *arg1) {
    _searchTagSo__10daNpc_So_cFP10fopAc_ac_c((daNpc_So_c *) arg1, (fopAc_ac_c *) arg0);
}

/* daNpc_So_c::_searchTagSo (fopAc_ac_c *) */
fopAc_ac_c *_searchTagSo__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0) {
    if (((s16) arg0->unk8 == 0x25) && ((u8) this->unkA79 == (u8) arg0->unk290) && ((u8) arg0->unk298 != 1)) {
        this->unkA7C = arg0->unk294;
        this->unkA80 = arg0->unk1F8;
        this->unkA84 = arg0->unk1FC;
        this->unkA88 = arg0->unk200;
        this->unk1F8 = this->unkA80;
        this->unk1FC = this->unkA84;
        this->unk200 = this->unkA88;
        return arg0;
    }
    return NULL;
}

/* searchMinigameTagSo_CB (void *, void *) */
void searchMinigameTagSo_CB__FPvPv(void *arg0, void *arg1) {
    _searchMinigameTagSo__10daNpc_So_cFP10fopAc_ac_c((daNpc_So_c *) arg1, (fopAc_ac_c *) arg0);
}

/* daNpc_So_c::_searchMinigameTagSo (fopAc_ac_c *) */
fopAc_ac_c *_searchMinigameTagSo__10daNpc_So_cFP10fopAc_ac_c(daNpc_So_c *this, fopAc_ac_c *arg0) {
    if (((s16) arg0->unk8 == 0x25) && ((u8) arg0->unk298 == 1)) {
        this->unkB90 = arg0->unk1F8;
        this->unkB94 = arg0->unk1FC;
        this->unkB98 = arg0->unk200;
        this->unkB9C = arg0->unk20E;
        this->unkBAE = 1;
        return arg0;
    }
    return NULL;
}

/* daNpc_So_XyCheckCB (void *, int) */
void daNpc_So_XyCheckCB__FPvi(void *arg0, s32 arg1) {
    XyCheckCB__10daNpc_So_cFi((daNpc_So_c *) arg0, arg1);
}

/* daNpc_So_c::XyCheckCB (int) */
s32 XyCheckCB__10daNpc_So_cFi(daNpc_So_c *this, s32 arg0) {
    if (fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchEsa_CB__FPvPv, this) != NULL) {
        return 0;
    }
    if ((s32) this->mPrmIdx != 2) {
        return 0;
    }
    if ((g_dComIfG_gameInfo.unk5CC8 & 0x10000) && ((u8) (&g_dComIfG_gameInfo + arg0)->unk5BD3 == 0x82)) {
        return 1;
    }
    return 0;
}

/* daNpc_So_XyEventCB (void *, int) */
void daNpc_So_XyEventCB__FPvi(void *arg0, s32 arg1) {
    XyEventCB__10daNpc_So_cFi((daNpc_So_c *) arg0, arg1);
}

/* daNpc_So_c::XyEventCB (int) */
s16 XyEventCB__10daNpc_So_cFi(daNpc_So_c *this, s32 arg0) {
    this->unkBDC = getEventIdx__16dEvent_manager_cFPCcUc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0", 0xFFU);
    return this->unkBDC;
}

/* createHeap_CB (fopAc_ac_c *) */
void createHeap_CB__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _createHeap__10daNpc_So_cFv((daNpc_So_c *) arg0);
}

/* daNpc_So_c::_createHeap (void) */
s32 _createHeap__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    J3DModel *temp_r3_5;
    JUTAssertion *temp_r3;
    JUTAssertion *temp_r3_10;
    JUTAssertion *temp_r3_6;
    JUTAssertion *var_r3;
    mDoExt_McaMorf *temp_r3_3;
    mDoExt_McaMorf *temp_r3_4;
    mDoExt_McaMorf *var_r0;
    s8 *temp_r3_11;
    s8 *temp_r3_2;
    s8 *temp_r3_7;
    s8 *temp_r3_8;
    s8 *temp_r3_9;
    s8 *temp_r4;
    s8 *temp_r4_2;
    s8 *temp_r4_3;
    s8 *temp_r4_4;
    s8 *temp_r4_5;
    void *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x1BFC0;
    temp_r3 = getRes__14dRes_control_cFPCclP11dRes_info_ci((dRes_control_c *) "So", (s8 *)0xC, (s32) temp_r30, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3 == NULL) {
        temp_r4 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) &temp_r4[0xA], (s8 *)0x1FD, (s32) &temp_r4[0x19], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        OSPanic(&temp_r3_2[0xA], 0x1FD, &temp_r3_2[0x28]);
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
    this->mpMorf = var_r0;
    temp_r3_4 = this->mpMorf;
    if ((temp_r3_4 == NULL) || (temp_r3_5 = temp_r3_4->unk50, ((temp_r3_5 == NULL) != 0))) {
        return 0;
    }
    temp_r3_5->unk14 = this;
    temp_r3_6 = getRes__14dRes_control_cFPCclP11dRes_info_ci((dRes_control_c *) "So", (s8 *)0x10, (s32) temp_r30, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3_6 == NULL) {
        temp_r4_2 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3_6), (u32) &temp_r4_2[0xA], (s8 *)0x210, (s32) &temp_r4_2[0x2D], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_7 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        OSPanic(&temp_r3_7[0xA], 0x210, &temp_r3_7[0x28]);
    }
    sp8 = 0;
    var_r3 = init__13mDoExt_btpAnmFP12J3DModelDataP16J3DAnmTexPatterniifssbi(&this->unk854, (J3DModelData *) temp_r3, (J3DAnmTexPattern *) temp_r3_6, 1, 0, 1.0f, 0, -1, 0, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    if (var_r3 == NULL) {
        return 0;
    }
    this->unk298 = 0xB;
    if ((s8) this->unk298 < 0) {
        temp_r4_3 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(var_r3), (u32) &temp_r4_3[0xA], (s8 *)0x215, (s32) &temp_r4_3[0x36], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_8 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        var_r3 = OSPanic(&temp_r3_8[0xA], 0x215, &temp_r3_8[0x28]);
    }
    this->unk299 = 1;
    if ((s8) this->unk299 < 0) {
        temp_r4_4 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(var_r3), (u32) &temp_r4_4[0xA], (s8 *)0x217, (s32) &temp_r4_4[0x51], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_9 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        OSPanic(&temp_r3_9[0xA], 0x217, &temp_r3_9[0x28]);
    }
    temp_r3->unk2C->unk2C->unk8 = nodeControl_CB__FP7J3DNodei;
    temp_r3->unk2C->unk4->unk8 = nodeControl_CB__FP7J3DNodei;
    temp_r3_10 = getRes__14dRes_control_cFPCclP11dRes_info_ci((dRes_control_c *) "So", (s8 *)0xD, (s32) temp_r30, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
    if (temp_r3_10 == NULL) {
        temp_r4_5 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3_10), (u32) &temp_r4_5[0xA], (s8 *)0x221, (s32) &temp_r4_5[0x19], M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_11 = "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0";
        OSPanic(&temp_r3_11[0xA], 0x221, &temp_r3_11[0x28]);
    }
    this->unk850 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) temp_r3_10, 0x80000U, 0x11000022U);
    if ((J3DModel *) this->unk850 == NULL) {
        return 0;
    }
    return jntHitCreateHeap__10daNpc_So_cFv(this) != 0;
}

/* daNpc_So_c::jntHitCreateHeap (void) */
u8 jntHitCreateHeap__10daNpc_So_cFv(daNpc_So_c *this) {
    u32 temp_r0;

    this->unkAA8 = JntHit_create__FP8J3DModelP16__jnt_hit_data_cs(this->mpMorf->unk50, &search_data$4469, 2);
    temp_r0 = this->unkAA8;
    if (temp_r0 != 0U) {
        this->unk250 = temp_r0;
        return 1U;
    }
    return 0U;
}

/* daNpc_So_c::checkTgHit (void) */
u8 checkTgHit__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    Vec *temp_r28;
    cXyz *temp_r29;
    s32 temp_r30;
    s8 temp_r8;
    u32 var_r6;
    void *temp_r3;

    temp_r30 = g_dComIfG_gameInfo.unk5B44;
    Move__10dCcD_GSttsFv(&this->unk6F8);
    if ((cLib_calcTimer<i>__FPi(&this->unk6D8) == 0) && (ChkTgHit__12dCcD_GObjInfFv(&this->unk718) != 0U)) {
        temp_r29 = &this->unk7E4;
        temp_r3 = GetTgHitObj__12dCcD_GObjInfFv(&this->unk718);
        this->unk6D8 = (s32) l_HIO.unk7C;
        if (temp_r3 == NULL) {
            return 0U;
        }
        if ((s32) temp_r3->unk10 != 0x4000) {

        } else {
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x2879U, &this->unk260, 0x20U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
        }
        temp_r8 = dComIfGp_getReverb__Fi((s32) (s8) this->unk20A);
        if (this != NULL) {
            var_r6 = this->unk4;
        } else {
            var_r6 = -1U;
        }
        temp_r28 = &this->unk260;
        monsSeStart__11JAIZelBasicFUlP3VecUlUlSc(zel_basic__11JAIZelBasic.unk0, 0x4991U, temp_r28, var_r6, 0U, temp_r8);
        sp8 = -1;
        spC = 0;
        sp10 = 0;
        sp14 = 0;
        set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x10U, temp_r29, NULL, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
        sp18 = m_heapsize__10daNpc_So_c.unk64;
        sp1C = m_heapsize__10daNpc_So_c.unk64;
        sp20 = m_heapsize__10daNpc_So_c.unk64;
        sp8 = -1;
        spC = 0;
        sp10 = 0;
        sp14 = 0;
        set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0xFU, temp_r29, temp_r30 + 0x20C, (cXyz *) &sp18, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x2828U, temp_r28, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
        return 1U;
    }
    return 0U;
}

/* daNpc_So_c::offsetZero (void) */
void offsetZero__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkB40 = m_heapsize__10daNpc_So_c.unkFC;
    this->unkB3C = m_heapsize__10daNpc_So_c.unkF8;
    this->unkB38 = m_heapsize__10daNpc_So_c.unkC8;
}

/* daNpc_So_c::offsetDive (void) */
void offsetDive__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkB40 = m_heapsize__10daNpc_So_c.unk130;
    this->unkB3C = m_heapsize__10daNpc_So_c.unk6C;
    this->unkB38 = m_heapsize__10daNpc_So_c.unk134;
}

/* daNpc_So_c::offsetSwim (void) */
void offsetSwim__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkB40 = m_heapsize__10daNpc_So_c.unk130;
    this->unkB3C = m_heapsize__10daNpc_So_c.unk6C;
    this->unkB38 = m_heapsize__10daNpc_So_c.unk138;
}

/* daNpc_So_c::offsetAppear (void) */
void offsetAppear__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkB40 = m_heapsize__10daNpc_So_c.unk13C;
    this->unkB3C = m_heapsize__10daNpc_So_c.unk140;
    this->unkB38 = m_heapsize__10daNpc_So_c.unk144;
}

/* daNpc_So_c::getMsg (void) */
s16 getMsg__10daNpc_So_cFv(daNpc_So_c *this) {
    s32 temp_r0;

    if ((u8) this->unkB0C != 0) {
        if (((u8) l_HIO.unk2E != 0) || (temp_r0 = this->unkB7C, ((temp_r0 < 0xA) == 0))) {
            if ((u8) this->unkBD9 != 0) {
                return 0x32E2;
            }
            return 0x32DD;
        }
        if (temp_r0 == 0) {
            return 0x32E1;
        }
        if (temp_r0 == 1) {
            return 0x32E0;
        }
        g_dComIfG_gameInfo.unk5BB8 = (s16) temp_r0;
        return 0x32DF;
    }
    if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x901U) == 0) && (strcmp(&g_dComIfG_gameInfo + 0x5134, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x163) == 0) && ((s32) this->unk20A == 0xD)) {
        if ((u8) this->unkBD8 != 0) {
            return this->unk6D0;
        }
        return 0x32CA;
    }
    if ((u8) this->unkBD8 != 0) {
        return this->unk6D0;
    }
    return 0x32D0;
}

/* daNpc_So_c::next_msgStatus (long unsigned *) */
s32 next_msgStatus__10daNpc_So_cFPUl(daNpc_So_c *this, u32 *arg0, ? arg_sp0) {
    s16 temp_r4;
    s32 var_r31;
    u32 temp_r0;

    var_r31 = 0xF;
    temp_r0 = *arg0;
    temp_r4 = this->unk6D0;
    if (temp_r0 == (u32) temp_r4) {
        if ((u8) this->unkBD8 != 0) {
            if ((isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x901U) == 0) && (strcmp(&g_dComIfG_gameInfo + 0x5134, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x163) == 0) && ((s32) this->unk20A == 0xD)) {
                *arg0 = 0x32CE;
            } else {
                *arg0 = 0x32D2;
            }
        } else {
            *arg0 = 0x32D6;
        }
        return 0xF;
    }
    if ((s32) temp_r0 != 0x32D4) {
        if ((s32) temp_r0 < 0x32D4) {
            switch ((s32) temp_r0) {                /* switch 1; irregular */
            case 0x32CA:                            /* switch 1 */
                *arg0 = 0x32CB;
                break;
            case 0x32CB:                            /* switch 1 */
                *arg0 = 0x32CC;
                break;
            case 0x32CC:                            /* switch 1 */
                *arg0 = 0x32CD;
                break;
            case 0x32CE:                            /* switch 1 */
                var_r31 = 0x10;
                modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 7);
                break;
            case 0x32D0:                            /* switch 1 */
                if ((isSaveArriveGrid__16dSv_player_map_cFi(&g_dComIfG_gameInfo + 0xC4, (s8) this->unk20A - 1) != 0) || ((u8) l_HIO.unk2F != 0)) {
                    *arg0 = 0x32D4;
                } else {
                    *arg0 = 0x32D1;
                }
                break;
            case 0x32CD:                            /* switch 1 */
            case 0x32D1:                            /* switch 1 */
                var_r31 = 0x10;
                modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 9);
                break;
            case 0x32D2:                            /* switch 1 */
                *arg0 = 0x32D3;
                break;
            case 0x633:                             /* switch 1 */
                if (getTriforceNum__20dSv_player_collect_cFv(&g_dComIfG_gameInfo + 0xB4) == 8) {
                    *arg0 = 0x635;
                } else {
                    *arg0 = 0x634;
                }
                break;
            case 0x32CF:                            /* switch 1 */
                var_r31 = 0x10;
                break;
            }
        } else {
            switch ((s32) temp_r0) {                /* switch 2; irregular */
            case 0x32D6:                            /* switch 2 */
                fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchMinigameTagSo_CB__FPvPv, this);
                if (((u8) l_HIO.unk30 != 0) || ((u8) this->unkBAE != 0)) {
                    if (((u8) g_dComIfG_gameInfo.unk48 != 0xFF) && ((u32) g_dComIfG_gameInfo.unk5CE0 == 0U)) {
                        if (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3A10U) == 0) {
                            *arg0 = 0x32D8;
                        } else {
                            *arg0 = 0x32DC;
                        }
                    } else {
                        *arg0 = 0x32D7;
                    }
                } else {
                    *arg0 = 0x32D7;
                }
                break;
            case 0x32D8:                            /* switch 2 */
                if ((u8) this->unk6B0->unkFA == 0) {
                    *arg0 = 0x32DA;
                } else {
                    *arg0 = 0x32D9;
                }
                break;
            case 0x32DC:                            /* switch 2 */
                if ((u8) this->unk6B0->unkFA == 0) {
                    *arg0 = 0x32DB;
                } else {
                    *arg0 = 0x32D9;
                }
                break;
            case 0x32DA:                            /* switch 2 */
                *arg0 = 0x32DB;
                break;
            case 0x32DB:                            /* switch 2 */
                var_r31 = 0x10;
                modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xA);
                break;
            case 0x32DD:                            /* switch 2 */
                *arg0 = 0x32DE;
                break;
            case 0x32DE:                            /* switch 2 */
                var_r31 = 0x10;
                modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xE);
                break;
            default:                                /* switch 2 */
                g_dComIfG_gameInfo.unk5B60 = (s32) (g_dComIfG_gameInfo.unk5B60 + (this->unkB7C * 0xA));
                *arg0 = 0x32E2;
                break;
            case 0x32E1:                            /* switch 2 */
                *arg0 = 0x32E2;
                break;
            case 0x32D9:                            /* switch 2 */
                *arg0 = 0x32D7;
                break;
            }
        }
    } else {
        *arg0 = (u32) temp_r4;
    }
    return var_r31;
}

/* daNpc_So_c::lookBack (void) */
void lookBack__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    dNpc_EventCut_c sp2C;
    f32 sp20;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f0;
    f32 temp_f1;
    f32 temp_f2;
    f32 temp_f3;
    s16 temp_r3;
    s16 var_r4;

    if ((u8) g_dComIfG_gameInfo.unk529A != 0) {
        if ((u8) this->unk324 != 0) {
            if ((s32) this->mPrmIdx == 4) {
                this->unk29A = 0;
            } else {
                this->unk29A = 1;
                getAttnPos__15dNpc_EventCut_cFv(&sp2C);
                this->unkB44 = (bitwise f32) sp2C;
                this->unkB48 = sp30;
                this->unkB4C = sp34;
            }
        } else {
            dNpc_playerEyePos__Ff(l_HIO.unk8);
            this->unkB44 = sp20;
            this->unkB48 = sp24;
            this->unkB4C = sp28;
        }
    } else {
        this->unk29A = 0;
        dNpc_playerEyePos__Ff(l_HIO.unk8);
        this->unkB44 = sp14;
        this->unkB48 = sp18;
        this->unkB4C = sp1C;
    }
    if ((u8) this->unk29A != 0) {
        var_r4 = l_HIO.unk1E;
        temp_r3 = this->unk326;
        if (temp_r3 != 0) {
            var_r4 = temp_r3;
        }
        cLib_addCalcAngleS2__FPssss(&this->unkB50, var_r4, 4, 0x800);
    } else {
        this->unkB50 = 0;
    }
    temp_f3 = this->unkB54.unk0;
    sp38 = temp_f3;
    temp_f2 = this->unkB58;
    sp3C = temp_f2;
    temp_f1 = this->unkB5C;
    sp40 = temp_f1;
    temp_f0 = temp_f2 + 200.0f;
    sp3C = temp_f0;
    sp8 = temp_f3;
    spC = temp_f0;
    sp10 = temp_f1;
    lookAtTarget__14dNpc_JntCtrl_cFPsP4cXyz4cXyzssb((dNpc_JntCtrl_c *) &this->unk290, &this->unk20E, (cXyz *) &this->unkB44, (cXyz) &sp8, this->unk20E, this->unkB50, this->unkBDA);
}

/* daNpc_So_c::setAttention (void) */
void setAttention__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 temp_f1;

    this->unk274 = this->unkB60.unk0;
    this->unk278 = this->unkB64;
    this->unk27C = this->unkB68;
    this->unk278 = this->unk278 + l_HIO.unk20;
    temp_f1 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk274, &this->mAcch);
    M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
    if (this->unk278 == temp_f1) {
        this->unk278 = temp_f1;
    }
    this->unk260.unk0 = this->unkB54.unk0;
    this->unk264 = this->unkB58;
    this->unk268 = this->unkB5C;
}

/* daNpc_So_c::setAnm (char signed, bool) */
void setAnm__10daNpc_So_cFScb(daNpc_So_c *this, s8 arg0, s32 arg1, ? arg_sp0) {
    mDoExt_McaMorf *temp_r3;
    u8 temp_r0;

    if (arg0 != 6) {
        this->mPrmIdx = (u8) arg0;
    }
    temp_r3 = this->mpMorf;
    M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
    if ((temp_r3->unk68 == ((f32) temp_r3->unk60 - m_heapsize__10daNpc_So_c.unkA8)) && (cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk50) < l_HIO.unk5C)) {
        temp_r0 = this->mPrmIdx;
        if ((s8) temp_r0 == 5) {
            this->mPrmIdx = 3;
        } else if ((s8) temp_r0 == 3) {
            this->mPrmIdx = 5;
        }
    }
    dLib_bcks_setAnm__FPCcP14mDoExt_McaMorfPScPScPScPCiPC14dLib_anm_prm_cb((s8 *) &m_heapsize__10daNpc_So_c.unk4, this->mpMorf, &this->mBckIdx, (s8 *) &this->mPrmIdx, &this->unk6D4, &m_heapsize__10daNpc_So_c.unk148, &m_heapsize__10daNpc_So_c.unk15C, arg1);
}

/* daNpc_So_c::setAnmSwimSpeed (void) */
void setAnmSwimSpeed__10daNpc_So_cFv(daNpc_So_c *this) {
    cXyz spC;
    f32 sp8;
    f32 temp_f1;
    f32 var_f0;
    f32 var_f1;
    f32 var_f1_2;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    if ((s32) this->mPrmIdx == 2) {
        __mi__4cXyzCFRC3Vec(&spC, (Vec *) &this->unk1F8);
        var_f1 = PSVECSquareMag(&spC);
        if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = sp8;
        }
        var_f1_2 = var_f1 / m_heapsize__10daNpc_So_c.unk6C;
        M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
        if (var_f1_2 == m_heapsize__10daNpc_So_c.unk9C) {
            var_f1_2 = m_heapsize__10daNpc_So_c.unk9C;
        } else {
            M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
            if (var_f1_2 == m_heapsize__10daNpc_So_c.unkA8) {
                var_f1_2 = m_heapsize__10daNpc_So_c.unkA8;
            }
        }
        temp_f1 = var_f1_2 * l_HIO.unk34;
        var_f0 = l_HIO.unk38;
        if (temp_f1 < var_f0) {

        } else {
            var_f0 = temp_f1;
        }
        this->mpMorf->unk64 = var_f0;
    }
}

/* daNpc_So_c::setMtx (void) */
void setMtx__10daNpc_So_cFv(daNpc_So_c *this) {
    J3DModel *temp_r31;

    temp_r31 = this->mpMorf->unk50;
    temp_r31->unk18 = this->unk214;
    temp_r31->unk1C = this->unk218;
    temp_r31->unk20 = this->unk21C;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8, this->unk1FC, this->unk200);
    mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20C, this->unk20E, this->unk210);
    transM__14mDoMtx_stack_cFfff(&@4341, 0.0f, this->unkB34, 0.0f);
    PSMTXCopy(&now__14mDoMtx_stack_c, &temp_r31->unk24);
}

/* daNpc_So_c::modeWaitInit (void) */
void modeWaitInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unk254 = 0.0f;
    this->unkAFC = 0.0f;
    offsetAppear__10daNpc_So_cFv(this);
}

/* daNpc_So_c::modeWait (void) */
void modeWait__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::modeHideInit (void) */
void modeHideInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkBDB = 1;
    offsetDive__10daNpc_So_cFv(this);
    this->unkA7C = 0.0f;
}

/* daNpc_So_c::modeHide (void) */
void modeHide__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unk1F8 = this->unkA80;
    this->unk1FC = this->unkA84;
    this->unk200 = this->unkA88;
}

/* daNpc_So_c::modeJumpInit (void) */
void modeJumpInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkAFC = this->unkB08 * (m_heapsize__10daNpc_So_c.unkC4 + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unkC4));
    this->unk254 = this->unkAFC;
    this->unk224 = (m_heapsize__10daNpc_So_c.unk5C * this->unkB08) + (m_heapsize__10daNpc_So_c.unk114 * this->unk254);
    if (this->unk224 > l_HIO.unk50) {
        this->unk224 = l_HIO.unk50;
    }
    this->unkB00 = this->unk224;
    this->unk20C = l_HIO.unk68;
    setAnm__10daNpc_So_cFScb(this, 4, 0);
    this->unk29C = 1;
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5938U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
}

/* daNpc_So_c::modeJump (void) */
void modeJump__10daNpc_So_cFv(daNpc_So_c *this) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    if (this->unk1FC < dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch)) {
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5939U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
        fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkCC * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
        __mi__4cXyzCFRC3Vec(&sp18, (Vec *) &this->unkA80);
        spC = (bitwise f32) sp18;
        sp10 = m_heapsize__10daNpc_So_c.unk9C;
        sp14 = sp20;
        var_f1 = PSVECSquareMag((cXyz *) &spC);
        if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = sp8;
        }
        if (var_f1 > this->unkA7C) {
            this->unk1F8 = this->unkA80;
            this->unk1FC = this->unkA84;
            this->unk200 = this->unkA88;
        }
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 3);
    }
}

/* daNpc_So_c::modeSwimInit (void) */
void modeSwimInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unk280 = 0x0200000A;
    this->unkA90 = (s32) (30.0f + cM_rndF__Ff(90.0f));
    setAnm__10daNpc_So_cFScb(this, 2, 0);
    this->unk29C = 1;
    offsetDive__10daNpc_So_cFv(this);
}

/* daNpc_So_c::modeSwim (void) */
void modeSwim__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    cXyz sp58;
    cXyz sp4C;
    cXyz sp40;
    cXyz sp34;
    cXyz sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 var_f1;
    f32 var_f31;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    void *temp_r29;

    this->unkBDB = 0;
    __mi__4cXyzCFRC3Vec(&sp58, g_dComIfG_gameInfo.unk5B44 + 0x1F8);
    sp1C = (bitwise f32) sp58;
    sp20 = m_heapsize__10daNpc_So_c.unk9C;
    sp24 = sp60;
    temp_f1 = PSVECSquareMag((cXyz *) &sp1C);
    if (temp_f1 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        spC = (f32) ((f64) temp_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f31 = spC;
    } else {
        var_f31 = temp_f1;
    }
    temp_r29 = g_dComIfG_gameInfo.unk5B54;
    if (temp_r29 != NULL) {
        cLib_addCalc2__FPffff(&this->unkB28, m_heapsize__10daNpc_So_c.unkF0, m_heapsize__10daNpc_So_c.unkFC, m_heapsize__10daNpc_So_c.unk6C);
        this->unkB2C = m_heapsize__10daNpc_So_c.unk74;
        this->unkB32 = 0x150;
        this->unkB10 = this->unkA80;
        this->unkB14 = this->unkA84;
        this->unkB18 = this->unkA88;
        this->unkB14 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unkB10, &this->mAcch);
        dLib_setCirclePath__FP18dLib_circle_path_c((dLib_circle_path_c *) &this->unkB10);
        this->unkB20 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch(&this->unkB1C, &this->mAcch);
        this->unkB20 += this->unkB34;
        __mi__4cXyzCFRC3Vec(&sp4C, (Vec *) &this->unk1F8);
        sp10 = (bitwise f32) sp4C;
        sp14 = m_heapsize__10daNpc_So_c.unk9C;
        sp18 = sp54;
        var_f1 = PSVECSquareMag((cXyz *) &sp10);
        if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0_4 = __frsqrte(var_f1);
            temp_f0_5 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_4 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_4 * temp_f0_4)));
            temp_f0_6 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_5 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_5 * temp_f0_5)));
            sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_6 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_6 * temp_f0_6)))));
            var_f1 = sp8;
        }
        if ((var_f1 > m_heapsize__10daNpc_So_c.unk68) || (temp_r29->unk254 > m_heapsize__10daNpc_So_c.unk6C)) {
            this->unkAFC = m_heapsize__10daNpc_So_c.unkF4;
            cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, &this->unkB1C), 8, 0x400);
            this->unkB04 = m_heapsize__10daNpc_So_c.unk9C;
        } else {
            cLib_addCalc2__FPffff(&this->unkB04, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unk1C8, m_heapsize__10daNpc_So_c.unkE8);
            __mi__4cXyzCFRC3Vec(&sp40, (Vec *) &this->unkB1C);
            sp64 = (bitwise f32) sp40;
            sp68 = sp44;
            sp6C = sp48;
            __ml__4cXyzCFf(&sp34, this->unkB04);
            __pl__4cXyzCFRC3Vec(&sp28, (Vec *) &this->unk1F8);
            this->unk1F8 = (bitwise f32) sp28;
            this->unk1FC = sp2C;
            this->unk200 = sp30;
            cLib_addCalcAngleS2__FPssss(&this->unk20E, (s16) (this->unkB30 + 0x8000), 4, 0x400);
        }
        if ((var_f31 < this->unkA7C) && (g_dComIfG_gameInfo.unk5CC8 & 0x10000)) {
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 4);
            return;
        }
        if (cLib_calcTimer<i>__FPi(&this->unkA90) == 0) {
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 2);
        }
    }
}

/* daNpc_So_c::modeNearSwimInit (void) */
void modeNearSwimInit__10daNpc_So_cFv(daNpc_So_c *this) {
    setAnm__10daNpc_So_cFScb(this, 2, 0);
    offsetDive__10daNpc_So_cFv(this);
    this->unk29C = 1;
}

/* daNpc_So_c::modeNearSwim (void) */
void modeNearSwim__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    cXyz sp58;
    cXyz sp4C;
    cXyz sp40;
    cXyz sp34;
    cXyz sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 var_f1;
    f32 var_f31;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    void *temp_r29;
    void *temp_r30;

    SetTypeForce__9dCamera_cFPcP10fopAc_ac_c(dCam_getBody__Fv(), "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x167, NULL);
    this->mOrderType = 2;
    temp_r29 = g_dComIfG_gameInfo.unk5B54;
    if (temp_r29 != NULL) {
        temp_r30 = g_dComIfG_gameInfo.unk5B44;
        __mi__4cXyzCFRC3Vec(&sp58, temp_r30 + 0x1F8);
        sp1C = (bitwise f32) sp58;
        sp20 = m_heapsize__10daNpc_So_c.unk9C;
        sp24 = sp60;
        temp_f1 = PSVECSquareMag((cXyz *) &sp1C);
        if (temp_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0 = __frsqrte(temp_f1);
            temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
            spC = (f32) ((f64) temp_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f31 = spC;
        } else {
            var_f31 = temp_f1;
        }
        cLib_addCalc2__FPffff(&this->unkB28, m_heapsize__10daNpc_So_c.unk78, m_heapsize__10daNpc_So_c.unkFC, m_heapsize__10daNpc_So_c.unk6C);
        this->unkB2C = m_heapsize__10daNpc_So_c.unk74;
        this->unkB32 = 0x100;
        this->unkB10 = temp_r30->unk1F8;
        this->unkB14 = temp_r30->unk1FC;
        this->unkB18 = temp_r30->unk200;
        this->unkB14 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unkB10, &this->mAcch);
        dLib_setCirclePath__FP18dLib_circle_path_c((dLib_circle_path_c *) &this->unkB10);
        this->unkB20 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch(&this->unkB1C, &this->mAcch);
        this->unkB20 += this->unkB34;
        __mi__4cXyzCFRC3Vec(&sp4C, (Vec *) &this->unk1F8);
        sp10 = (bitwise f32) sp4C;
        sp14 = m_heapsize__10daNpc_So_c.unk9C;
        sp18 = sp54;
        var_f1 = PSVECSquareMag((cXyz *) &sp10);
        if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0_4 = __frsqrte(var_f1);
            temp_f0_5 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_4 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_4 * temp_f0_4)));
            temp_f0_6 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_5 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_5 * temp_f0_5)));
            sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_6 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_6 * temp_f0_6)))));
            var_f1 = sp8;
        }
        if ((var_f1 > m_heapsize__10daNpc_So_c.unk68) || (temp_r29->unk254 > m_heapsize__10daNpc_So_c.unk6C)) {
            this->unkAFC = m_heapsize__10daNpc_So_c.unkF4;
            cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, &this->unkB1C), 8, 0x400);
            this->unkB04 = m_heapsize__10daNpc_So_c.unk9C;
            this->unk1FC += this->unkB34;
        } else {
            cLib_addCalc2__FPffff(&this->unkB04, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unk1C8, m_heapsize__10daNpc_So_c.unkE8);
            __mi__4cXyzCFRC3Vec(&sp40, (Vec *) &this->unkB1C);
            sp64 = (bitwise f32) sp40;
            sp68 = sp44;
            sp6C = sp48;
            __ml__4cXyzCFf(&sp34, this->unkB04);
            __pl__4cXyzCFRC3Vec(&sp28, (Vec *) &this->unk1F8);
            this->unk1F8 = (bitwise f32) sp28;
            this->unk1FC = sp2C;
            this->unk200 = sp30;
            cLib_addCalcAngleS2__FPssss(&this->unk20E, (s16) (this->unkB30 + 0x8000), 4, 0x400);
            this->unk1FC += this->unkB34;
        }
        M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
        if (var_f31 == this->unkA7C) {
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 3);
        }
    }
}

/* daNpc_So_c::modeEventFirstWaitInit (void) */
void modeEventFirstWaitInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA90 = 0x96;
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    offsetDive__10daNpc_So_cFv(this);
}

/* daNpc_So_c::modeEventFirstWait (void) */
void modeEventFirstWait__10daNpc_So_cFv(daNpc_So_c *this) {
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    if ((void *) g_dComIfG_gameInfo.unk5B54 != NULL) {
        this->unk1F8 = g_dComIfG_gameInfo.unk5B54->unk1F8;
        this->unk1FC = g_dComIfG_gameInfo.unk5B54->unk1FC;
        this->unk200 = g_dComIfG_gameInfo.unk5B54->unk200;
        __mi__4cXyzCFRC3Vec(&sp18, g_dComIfG_gameInfo.unk5B54 + 0x1F8);
        spC = (bitwise f32) sp18;
        sp10 = m_heapsize__10daNpc_So_c.unk9C;
        sp14 = sp20;
        var_f1 = PSVECSquareMag((cXyz *) &spC);
        if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
            temp_f0 = __frsqrte(var_f1);
            temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0 * temp_f0)));
            temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
            sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
            var_f1 = sp8;
        }
        M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
        if ((var_f1 == l_HIO.unk54) && (dComIfGs_checkGetItem__FUc(0x78U) != 0) && (dComIfGs_isStageBossEnemy__Fi(3) != 0) && (g_dComIfG_gameInfo.unk5CC8 & 0x10000)) {
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 6);
        }
    }
}

/* daNpc_So_c::modeEventFirstInit (void) */
void modeEventFirstInit__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetSwim__10daNpc_So_cFv(this);
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
}

/* daNpc_So_c::modeEventFirst (void) */
void modeEventFirst__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x172) != 0) {
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            this->mOrderType = 1;
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xB);
            this->unk280 = 0x0200000A;
        }
    } else {
        this->mOrderType = 3;
    }
}

/* daNpc_So_c::modeEventFirstEndInit (void) */
void modeEventFirstEndInit__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetAppear__10daNpc_So_cFv(this);
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
}

/* daNpc_So_c::modeEventFirstEnd (void) */
void modeEventFirstEnd__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x17E) != 0) {
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xC);
        }
    } else if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        this->mOrderType = 4;
    }
}

/* daNpc_So_c::modeEventEsaInit (void) */
void modeEventEsaInit__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetSwim__10daNpc_So_cFv(this);
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
}

/* daNpc_So_c::modeEventEsa (void) */
void modeEventEsa__10daNpc_So_cFv(daNpc_So_c *this) {
    if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
        cutProc__10daNpc_So_cFv(this);
    }
    if (endCheck__16dEvent_manager_cFs(&g_dComIfG_gameInfo + 0x52CC, this->unkBDC) != 0) {
        g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
        this->unkBDC = -1;
        this->mOrderType = 1;
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xB);
    }
}

/* daNpc_So_c::modeEventMapopenInit (void) */
void modeEventMapopenInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
}

/* daNpc_So_c::modeEventMapopen (void) */
void modeEventMapopen__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x18E) != 0) {
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            this->mOrderType = 1;
            this->unkBD8 = 1;
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xB);
        }
    } else if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        this->mOrderType = 5;
    }
}

/* daNpc_So_c::modeEventBowInit (void) */
void modeEventBowInit__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetSwim__10daNpc_So_cFv(this);
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
}

/* daNpc_So_c::modeEventBow (void) */
void modeEventBow__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    s32 temp_r30;

    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x199) != 0) {
            this->unkB0C = 1;
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            this->unkBDB = 0;
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3A10U);
            temp_r30 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
            sp14 = this->unkBCC;
            sp18 = this->unkBD0;
            sp1C = this->unkBD4;
            sp8 = this->unkBC0;
            spC = this->unkBC4;
            sp10 = this->unkBC8;
            Reset__9dCamera_cF4cXyz4cXyz(temp_r30 + 0x244, (cXyz) &sp14, (cXyz) &sp8);
            Start__9dCamera_cFv(temp_r30 + 0x244);
            this->mOrderType = 1;
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xB);
        }
    } else if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        this->mOrderType = 6;
    }
}

/* daNpc_So_c::modeTalkInit (void) */
void modeTalkInit__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetAppear__10daNpc_So_cFv(this);
    setAnm__10daNpc_So_cFScb(this, 3, 0);
    this->unk29C = 0;
}

/* daNpc_So_c::modeTalk (void) */
void modeTalk__10daNpc_So_cFv(daNpc_So_c *this) {
    if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xC);
    }
}

/* daNpc_So_c::modeDisappearInit (void) */
void modeDisappearInit__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    dSv_event_c *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x624;
    if (isEventBit__11dSv_event_cFUs(temp_r30, 0x901U) == 0) {
        onEventBit__11dSv_event_cFUs(temp_r30, 0x901U);
    }
    offsetDive__10daNpc_So_cFv(this);
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x593BU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
    fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkD0 * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
    this->unk29C = 0;
}

/* daNpc_So_c::modeDisappear (void) */
void modeDisappear__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    daShip_c *temp_r3;
    u8 temp_r29;

    if ((f32) fabs(this->unkB34 - this->unkB38) < m_heapsize__10daNpc_So_c.unk6C) {
        temp_r29 = this->unkA79;
        do {
loop_2:
            this->unkA79 = (u8) (s32) cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk1CC);
            if ((s32) this->unkA79 == (s32) temp_r29) {
                goto loop_2;
            }
            temp_r3 = fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchTagSo_CB__FPvPv, this);
        } while (temp_r3 == NULL);
        this->unk1F8 = temp_r3->unk1F8;
        this->unk1FC = temp_r3->unk1FC;
        this->unk200 = temp_r3->unk200;
        this->unkB74 = 0;
        this->unkB78 = 0;
        this->unkB7C = 0;
        this->unkB80 = 0;
        this->unkB84 = 0;
        this->unkB88 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkB8C = m_heapsize__10daNpc_So_c.unk9C;
        this->unkB90 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkB94 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkB98 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkB9C = 0;
        this->unkBA0 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkBA4 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkBA8 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkBAC = 0;
        this->unkB0C = 0;
        this->unkBD9 = 0;
        this->unkBD8 = 0;
        this->unkBDB = 1;
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 1);
        this->unk258 = m_heapsize__10daNpc_So_c.unk1D0;
    }
}

/* daNpc_So_c::modeDebugInit (void) */
void modeDebugInit__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f0;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f3;
    f32 temp_f4;
    s32 temp_r0;

    this->unk254 = 0.0f;
    this->unkAFC = 0.0f;
    setAnm__10daNpc_So_cFScb(this, 1, 0);
    temp_f1 = g_dComIfG_gameInfo.unk5B44->unk1F8;
    sp8 = temp_f1;
    temp_f4 = g_dComIfG_gameInfo.unk5B44->unk1FC;
    spC = temp_f4;
    temp_f3 = g_dComIfG_gameInfo.unk5B44->unk200;
    sp10 = temp_f3;
    temp_r0 = ((s32) (u16) g_dComIfG_gameInfo.unk5B44->unk20E >> jmaSinShift) * 4;
    temp_f1_2 = temp_f1 + (100.0f * *(jmaCosTable + temp_r0));
    sp8 = temp_f1_2;
    temp_f0 = temp_f3 + (100.0f * *(jmaSinTable + temp_r0));
    sp10 = temp_f0;
    this->unk1F8 = temp_f1_2;
    this->unk1FC = temp_f4;
    this->unk200 = temp_f0;
    offsetAppear__10daNpc_So_cFv(this);
}

/* daNpc_So_c::modeDebug (void) */
void modeDebug__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::modeGetRupeeInit (void) */
void modeGetRupeeInit__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::modeGetRupee (void) */
void modeGetRupee__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x1A0) != 0) {
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            this->mOrderType = 1;
            this->unkBD9 = 1;
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xB);
        }
    } else if (talk__12fopNpc_npc_cFi((fopNpc_npc_c *) this, 1) == 0x12) {
        if ((s16) g_regHIO.unk756 != 0) {
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
        }
        g_dComIfG_gameInfo.unk5B44->unk304 = 2;
        g_dComIfG_gameInfo.unk5B44->unk314 = 1;
        this->mOrderType = 7;
    }
}

/* daNpc_So_c::modeEventTriForceInit (void) */
void modeEventTriForceInit__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
    this->unk29C = 0;
    offsetAppear__10daNpc_So_cFv(this);
    setAnm__10daNpc_So_cFScb(this, 1, 0);
}

/* daNpc_So_c::modeEventTriForce (void) */
void modeEventTriForce__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u16) this->unkF8 == 2) {
        if (cutProc__15dNpc_EventCut_cFv(&this->unk2C4) == 0) {
            cutProc__10daNpc_So_cFv(this);
        }
        if (endCheckOld__16dEvent_manager_cFPCc(&g_dComIfG_gameInfo + 0x52CC, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x1AD) != 0) {
            this->unk280 = 0x0200000A;
            g_dComIfG_gameInfo.unk52C0 = (u16) (g_dComIfG_gameInfo.unk52C0 | 8);
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xC);
            onEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x3A20U);
        }
    } else {
        this->mOrderType = 8;
    }
}

/* daNpc_So_c::modeProc (daNpc_So_c::Proc_e, int) */
void modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(daNpc_So_c *this, ? arg0, s32 arg1) {
    void *temp_r6;

    if ((s8) init$6629 == 0) {
        @2100.unk668 = (s32) @2100.unk4E8;
        @2100.unk66C = (s32) @2100.unk4EC;
        @2100.unk670 = (s32) @2100.unk4F0;
        temp_r6 = &@2100 + 0x668;
        temp_r6->unkC = (s32) @2100.unk4F4;
        temp_r6->unk10 = (s32) @2100.unk4F8;
        temp_r6->unk14 = (s32) @2100.unk4FC;
        temp_r6->unk1C = (s32) @2100.unk500;
        temp_r6->unk20 = (s32) @2100.unk504;
        temp_r6->unk24 = (s32) @2100.unk508;
        temp_r6->unk28 = (s32) @2100.unk50C;
        temp_r6->unk2C = (s32) @2100.unk510;
        temp_r6->unk30 = (s32) @2100.unk514;
        temp_r6->unk38 = (s32) @2100.unk518;
        temp_r6->unk3C = (s32) @2100.unk51C;
        temp_r6->unk40 = (s32) @2100.unk520;
        temp_r6->unk44 = (s32) @2100.unk524;
        temp_r6->unk48 = (s32) @2100.unk528;
        temp_r6->unk4C = (s32) @2100.unk52C;
        temp_r6->unk54 = (s32) @2100.unk530;
        temp_r6->unk58 = (s32) @2100.unk534;
        temp_r6->unk5C = (s32) @2100.unk538;
        temp_r6->unk60 = (s32) @2100.unk53C;
        temp_r6->unk64 = (s32) @2100.unk540;
        temp_r6->unk68 = (s32) @2100.unk544;
        temp_r6->unk70 = (s32) @2100.unk548;
        temp_r6->unk74 = (s32) @2100.unk54C;
        temp_r6->unk78 = (s32) @2100.unk550;
        temp_r6->unk7C = (s32) @2100.unk554;
        temp_r6->unk80 = (s32) @2100.unk558;
        temp_r6->unk84 = (s32) @2100.unk55C;
        temp_r6->unk8C = (s32) @2100.unk560;
        temp_r6->unk90 = (s32) @2100.unk564;
        temp_r6->unk94 = (s32) @2100.unk568;
        temp_r6->unk98 = (s32) @2100.unk56C;
        temp_r6->unk9C = (s32) @2100.unk570;
        temp_r6->unkA0 = (s32) @2100.unk574;
        temp_r6->unkA8 = (s32) @2100.unk578;
        temp_r6->unkAC = (s32) @2100.unk57C;
        temp_r6->unkB0 = (s32) @2100.unk580;
        temp_r6->unkB4 = (s32) @2100.unk584;
        temp_r6->unkB8 = (s32) @2100.unk588;
        temp_r6->unkBC = (s32) @2100.unk58C;
        temp_r6->unkC4 = (s32) @2100.unk590;
        temp_r6->unkC8 = (s32) @2100.unk594;
        temp_r6->unkCC = (s32) @2100.unk598;
        temp_r6->unkD0 = (s32) @2100.unk59C;
        temp_r6->unkD4 = (s32) @2100.unk5A0;
        temp_r6->unkD8 = (s32) @2100.unk5A4;
        temp_r6->unkE0 = (s32) @2100.unk5A8;
        temp_r6->unkE4 = (s32) @2100.unk5AC;
        temp_r6->unkE8 = (s32) @2100.unk5B0;
        temp_r6->unkEC = (s32) @2100.unk5B4;
        temp_r6->unkF0 = (s32) @2100.unk5B8;
        temp_r6->unkF4 = (s32) @2100.unk5BC;
        temp_r6->unkFC = (s32) @2100.unk5C0;
        temp_r6->unk100 = (s32) @2100.unk5C4;
        temp_r6->unk104 = (s32) @2100.unk5C8;
        temp_r6->unk108 = (s32) @2100.unk5CC;
        temp_r6->unk10C = (s32) @2100.unk5D0;
        temp_r6->unk110 = (s32) @2100.unk5D4;
        temp_r6->unk118 = (s32) @2100.unk5D8;
        temp_r6->unk11C = (s32) @2100.unk5DC;
        temp_r6->unk120 = (s32) @2100.unk5E0;
        temp_r6->unk124 = (s32) @2100.unk5E4;
        temp_r6->unk128 = (s32) @2100.unk5E8;
        temp_r6->unk12C = (s32) @2100.unk5EC;
        temp_r6->unk134 = (s32) @2100.unk5F0;
        temp_r6->unk138 = (s32) @2100.unk5F4;
        temp_r6->unk13C = (s32) @2100.unk5F8;
        temp_r6->unk140 = (s32) @2100.unk5FC;
        temp_r6->unk144 = (s32) @2100.unk600;
        temp_r6->unk148 = (s32) @2100.unk604;
        temp_r6->unk150 = (s32) @2100.unk608;
        temp_r6->unk154 = (s32) @2100.unk60C;
        temp_r6->unk158 = (s32) @2100.unk610;
        temp_r6->unk15C = (s32) @2100.unk614;
        temp_r6->unk160 = (s32) @2100.unk618;
        temp_r6->unk164 = (s32) @2100.unk61C;
        temp_r6->unk16C = (s32) @2100.unk620;
        temp_r6->unk170 = (s32) @2100.unk624;
        temp_r6->unk174 = (s32) @2100.unk628;
        temp_r6->unk178 = (s32) @2100.unk62C;
        temp_r6->unk17C = (s32) @2100.unk630;
        temp_r6->unk180 = (s32) @2100.unk634;
        temp_r6->unk188 = (s32) @2100.unk638;
        temp_r6->unk18C = (s32) @2100.unk63C;
        temp_r6->unk190 = (s32) @2100.unk640;
        temp_r6->unk194 = (s32) @2100.unk644;
        temp_r6->unk198 = (s32) @2100.unk648;
        temp_r6->unk19C = (s32) @2100.unk64C;
        temp_r6->unk1A4 = (s32) @2100.unk650;
        temp_r6->unk1A8 = (s32) @2100.unk654;
        temp_r6->unk1AC = (s32) @2100.unk658;
        temp_r6->unk1B0 = (s32) @2100.unk65C;
        temp_r6->unk1B4 = (s32) @2100.unk660;
        temp_r6->unk1B8 = (s32) @2100.unk664;
        init$6629 = 1;
    }
    switch ((s32) arg0) {                           /* irregular */
    case 0:
        this->mProcNo = arg1;
        __ptmf_scall();
        return;
    case 1:
        __ptmf_scall(this->mProcNo * 0x1C);
        return;
    }
}

/* daNpc_So_c::eventOrder (void) */
void eventOrder__10daNpc_So_cFv(daNpc_So_c *this) {
    u8 temp_r5;

    temp_r5 = this->mOrderType;
    switch (temp_r5) {                              /* irregular */
    case 1:
    case 2:
        this->unkFA = (u16) (this->unkFA | 1);
        this->unkFA = (u16) (this->unkFA | 0x20);
        if ((u8) this->mOrderType == 1) {
            fopAcM_orderSpeakEvent__FP10fopAc_ac_c((fopAc_ac_c *) this);
            return;
        }
        return;
    case 5:
    case 4:
    case 6:
        fopAcM_orderChangeEvent__FP10fopAc_ac_cPcUsUs((fopAc_ac_c *) this, (&a_demo_name_tbl$6640 + ((temp_r5 * 4) & 0x3FC))->unk-C, 0U, 0xFFFFU);
        return;
    case 7:
        if ((s16) g_regHIO.unk756 == 0) {
            fopAcM_orderChangeEvent__FP10fopAc_ac_cPcUsUs((fopAc_ac_c *) this, (&a_demo_name_tbl$6640 + ((temp_r5 * 4) & 0x3FC))->unk-C, 0U, 0xFFFFU);
            this->unkFA = (u16) (this->unkFA | 8);
            return;
        }
        fopAcM_orderOtherEvent2__FP10fopAc_ac_cPcUsUs((fopAc_ac_c *) this, (&a_demo_name_tbl$6640 + ((temp_r5 * 4) & 0x3FC))->unk-C, 1U, 0xFFFFU);
        return;
    default:
        fopAcM_orderOtherEvent2__FP10fopAc_ac_cPcUsUs((fopAc_ac_c *) this, (&a_demo_name_tbl$6640 + ((temp_r5 * 4) & 0x3FC))->unk-C, 1U, 0xFFFFU);
        break;
    }
}

/* daNpc_So_c::checkOrder (void) */
void checkOrder__10daNpc_So_cFv(daNpc_So_c *this) {
    u16 temp_r0;
    u8 temp_r0_2;
    u8 var_r0;

    temp_r0 = this->unkF8;
    switch (temp_r0) {                              /* irregular */
    case 2:
        this->mOrderType = 0;
        return;
    case 1:
        temp_r0_2 = this->mOrderType;
        if ((temp_r0_2 == 1) || (temp_r0_2 == 2)) {
            var_r0 = 0;
            if (((u8) g_dComIfG_gameInfo.unk52B8 == 1) || ((u8) g_dComIfG_gameInfo.unk52B8 == 2) || ((u8) g_dComIfG_gameInfo.unk52B8 == 3)) {
                var_r0 = 1;
            }
            if (var_r0 != 0) {
                modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 8);
            }
            this->mOrderType = 0;
        }
        return;
    }
}

/* daNpc_So_c::setScale (void) */
void setScale__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f1;

    temp_f1 = fopAcM_searchActorDistanceXZ__FP10fopAc_ac_cP10fopAc_ac_c((fopAc_ac_c *) this, g_dComIfG_gameInfo.unk5B44);
    if (temp_f1 > l_HIO.unk48) {
        this->unkB08 = m_heapsize__10daNpc_So_c.unkA8 + ((temp_f1 - l_HIO.unk48) / ((m_heapsize__10daNpc_So_c.unk1D4 - l_HIO.unk48) / l_HIO.unk4C));
        if (this->unkB08 > l_HIO.unk4C) {
            this->unkB08 = l_HIO.unk4C;
        }
    } else {
        this->unkB08 = l_HIO.unk44;
    }
    if ((s32) this->mProcNo == 0xF) {
        this->unkB08 = m_heapsize__10daNpc_So_c.unkA8;
        temp_f0 = this->unkB08;
        this->unk214 = temp_f0;
        this->unk218 = temp_f0;
        this->unk21C = temp_f0;
    }
    temp_f0_2 = this->unkB08;
    sp8 = temp_f0_2;
    spC = temp_f0_2;
    sp10 = temp_f0_2;
    cLib_addCalcPos2__FP4cXyzRC4cXyzff((cXyz *) &this->unk214, (cXyz *) &sp8, m_heapsize__10daNpc_So_c.unkFC, m_heapsize__10daNpc_So_c.unk110);
}

/* daNpc_So_c::_execute (void) */
u8 _execute__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f1_3;
    f32 temp_f2;
    f32 temp_f3;
    f32 temp_f4;
    s16 var_r28;
    s32 temp_r0;
    void *(*temp_r3)(daNpc_So_HIO_c *, s16);
    void *temp_r10;

    temp_f1 = this->unk214;
    temp_f4 = m_heapsize__10daNpc_So_c.unk50 * temp_f1;
    temp_f1_2 = m_heapsize__10daNpc_So_c.unk1D8 * temp_f1;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, temp_f1_2, temp_f1_2, temp_f1_2, temp_f4, temp_f4, temp_f4);
    if (((u8) g_dComIfG_gameInfo.unk529A == 0) && (this->unk898 & 0x20)) {
        this->unk1FC = m_heapsize__10daNpc_So_c.unk9C;
        this->unk254 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
        this->unk224 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 1);
        return 1U;
    }
    temp_r10 = &@3569 + 0x58;
    sp8 = (s32) temp_r10->unk18;
    spC = (s32) temp_r10->unk1C;
    setParam__14dNpc_JntCtrl_cFsssssssss((dNpc_JntCtrl_c *) &this->unk290, temp_r10->unkE, temp_r10->unk12, temp_r10->unk16, temp_r10->unk1A, temp_r10->unkC, temp_r10->unk10, temp_r10->unk14, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */));
    this->unk206 = this->unk20E;
    if ((s32) this->mProcNo == 1) {
        if (m_heapsize__10daNpc_So_c.unk9C == this->unkA7C) {
            fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchTagSo_CB__FPvPv, this);
        } else {
            modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 3);
        }
    } else if (m_heapsize__10daNpc_So_c.unk9C == this->unkA7C) {
        fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchTagSo_CB__FPvPv, this);
    }
    if (cLib_calcTimer<i>__FPi(&this->unk868) == 0) {
        this->unk86C += 1;
        if ((f32) this->unk86C > (f32) this->unk858->unk8) {
            this->unk868 = (s32) (s16) (m_heapsize__10daNpc_So_c.unk50 + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk50));
            this->unk86C = 0;
        }
    }
    setScale__10daNpc_So_cFv(this);
    setAttention__10daNpc_So_cFv(this);
    cLib_addCalc2__FPffff(&this->unk254, this->unkAFC, m_heapsize__10daNpc_So_c.unk130, m_heapsize__10daNpc_So_c.unk114);
    cLib_addCalc2__FPffff(&this->unkB34, this->unkB38, this->unkB40, this->unkB3C);
    lookBack__10daNpc_So_cFv(this);
    checkOrder__10daNpc_So_cFv(this);
    modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 1, 0x10);
    eventOrder__10daNpc_So_cFv(this);
    var_r28 = 0;
    if (this->unk1FC < dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch)) {
        this->unk1FC = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch);
        if ((this->unkB34 > m_heapsize__10daNpc_So_c.unk9C) && ((u32) this->unkAEC == 0U)) {
            if ((s8) @3569.unk12C == 0) {
                @3569.unk130 = (f32) m_heapsize__10daNpc_So_c.unk1DC;
                temp_r3 = &@3569 + 0x130;
                temp_r3->unk4 = (f32) m_heapsize__10daNpc_So_c.unk1DC;
                temp_r3->unk8 = (f32) m_heapsize__10daNpc_So_c.unk1DC;
                __register_global_object(temp_r3, __dt__4cXyzFv, &@3569 + 0x120);
                @3569.unk12C = 1U;
            }
            sp8 = -1;
            spC = 0;
            sp10 = 0;
            sp14 = 0;
            set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 5U, 0x33U, (cXyz *) &this->unk1F8, NULL, &@3569 + 0x130, 0xFFU, &this->unkAE8, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
            if ((u32) this->unkAEC != 0U) {
                this->unkAF8 = m_heapsize__10daNpc_So_c.unk9C;
            }
        }
    } else {
        temp_f2 = this->unk224;
        temp_f1_3 = this->unkB00;
        temp_f3 = temp_f1_3 * m_heapsize__10daNpc_So_c.unk1E0;
        if (temp_f2 < -temp_f3) {
            if (temp_f2 < -(temp_f1_3 * m_heapsize__10daNpc_So_c.unk110)) {
                var_r28 = (&@3569 + 0x58)->unk64;
            } else {
                var_r28 = (&@3569 + 0x58)->unk66;
            }
        } else if (temp_f2 > temp_f3) {
            if (temp_f2 > (temp_f1_3 * m_heapsize__10daNpc_So_c.unk110)) {
                var_r28 = (&@3569 + 0x58)->unk68;
            } else {
                var_r28 = (&@3569 + 0x58)->unk6A;
            }
        } else {
            var_r28 = 0;
        }
        end__19dPa_rippleEcallBackFv((dPa_rippleEcallBack *) &this->unkAE8);
    }
    cLib_addCalcAngleS2__FPssss(&this->unk20C, var_r28, 4, 0x800);
    temp_r0 = this->mProcNo;
    if ((temp_r0 != 1) && (temp_r0 != 5) && ((u8) this->unkBDB == 0) && (cLib_calcTimer<i>__FPi(&this->unkBE0) == 0)) {
        fopAcM_posMoveF__FP10fopAc_ac_cPC4cXyz((fopAc_ac_c *) this, NULL);
        CrrPos__9dBgS_AcchFR4dBgS((dBgS_Acch *) &this->mAcch, &g_dComIfG_gameInfo + 0x12A0);
    }
    play__14mDoExt_McaMorfFP3VecUlSc(this->mpMorf, NULL, 0U, 0);
    calc__14mDoExt_McaMorfFv(this->mpMorf);
    setMtx__10daNpc_So_cFv(this);
    setAnm__10daNpc_So_cFScb(this, 6, 0);
    setAnmSwimSpeed__10daNpc_So_cFv(this);
    this->unk206 = this->unk20E;
    return 0U;
}

/* daNpc_So_c::debugDraw (void) */
void debugDraw__10daNpc_So_cFv(daNpc_So_c *this) {
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
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f1_3;
    f32 temp_f1_4;

    sp2C = this->unkA80;
    temp_f1 = this->unkA84;
    sp30 = temp_f1;
    sp34 = this->unkA88;
    sp30 = temp_f1 + 20.0f;
    sp20 = g_dComIfG_gameInfo.unk5B44->unk1F8;
    temp_f1_2 = g_dComIfG_gameInfo.unk5B44->unk1FC;
    sp24 = temp_f1_2;
    sp28 = g_dComIfG_gameInfo.unk5B44->unk200;
    temp_f1_3 = temp_f1_2 + 20.0f;
    sp24 = temp_f1_3;
    sp14 = this->unk1F8;
    sp18 = this->unk1FC;
    sp1C = this->unk200;
    sp18 = temp_f1_3;
    sp8 = this->unkAAC;
    temp_f1_4 = this->unkAB0;
    spC = temp_f1_4;
    sp10 = this->unkAB4;
    spC = temp_f1_4 + 20.0f;
}

/* daNpc_So_c::hudeDraw (void) */
void hudeDraw__10daNpc_So_cFv(daNpc_So_c *this) {
    setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, this->unk850, &this->unk10C);
    PSMTXCopy(this->mpMorf->unk50->unk8C + 0x210, &now__14mDoMtx_stack_c);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->unk850->unk24);
    mDoExt_modelUpdateDL__FP8J3DModel(this->unk850);
}

/* daNpc_So_c::_draw (void) */
u8 _draw__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    J3DModel *temp_r29;
    J3DModelData *temp_r28;
    f32 temp_f1;
    f32 temp_f3;

    if ((u8) l_HIO.unk26 != 0) {
        debugDraw__10daNpc_So_cFv(this);
    }
    if ((s32) this->mProcNo == 5) {
        return 1U;
    }
    if (((u8) l_HIO.unk31 == 0) && ((temp_r29 = this->mpMorf->unk50, temp_r28 = temp_r29->unk4, settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, (cXyz *) &this->unk1F8, &this->unk10C), setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r29, &this->unk10C), entry__13mDoExt_btpAnmFP12J3DModelDatas(&this->unk854, temp_r28, this->unk86C), entryDL__14mDoExt_McaMorfFv(this->mpMorf), removeTexNoAnimator__16J3DMaterialTableFP16J3DAnmTexPattern(&temp_r28->unk58, this->unk85C), (((u8) this->unkA78 == 0) == 0)) || ((u8) l_HIO.unk2C != 0))) {
        hudeDraw__10daNpc_So_cFv(this);
    }
    sp14 = this->unk1F8;
    temp_f1 = this->unk1FC;
    sp18 = temp_f1;
    sp1C = this->unk200;
    sp18 = temp_f1 + this->unkB34;
    dSnap_RegistFig__FUcP10fopAc_ac_cRC3Vecsfff(0x7DU, (fopAc_ac_c *) this, (Vec *) &sp14, this->unk20E, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8);
    temp_f3 = this->unk1FC;
    sp8 = this->unk1F8;
    spC = m_heapsize__10daNpc_So_c.unk68 + temp_f3;
    sp10 = this->unk200;
    this->unkA74 = dComIfGd_setShadow__FUlScP8J3DModelP4cXyzffffR13cBgS_PolyInfoP12dKy_tevstr_csfP9_GXTexObj(this->unkA74, 0, this->mpMorf->unk50, (cXyz *) &sp8, m_heapsize__10daNpc_So_c.unk210, m_heapsize__10daNpc_So_c.unkC0, temp_f3 + this->unkB34, this->unk904, &this->unk958, &this->unk10C, 0, m_heapsize__10daNpc_So_c.unkA8, (_GXTexObj *) &mSimpleTexObj__21dDlst_shadowControl_c);
    return 1U;
}

/* daNpc_So_c::createInit (void) */
void createInit__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    s32 sp8;
    dSv_event_c *temp_r30;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f4;

    this->unkBDA = 0;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk538, 0xFF, 0xFF, (fopAc_ac_c *) this);
    Set__8dCcD_CylFRC11dCcD_SrcCyl(&this->unk574, &dNpc_cyl_src);
    this->unk5B8 = &this->unk538;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk6DC, 0xFF, 0xFF, (fopAc_ac_c *) this);
    Set__8dCcD_SphFRC11dCcD_SrcSph((dCcD_Sph *) &this->unk718, &m_heapsize__10daNpc_So_c.unk8);
    this->unk75C = &this->unk6DC;
    this->unk1FC -= m_heapsize__10daNpc_So_c.unk214;
    setMtx__10daNpc_So_cFv(this);
    calc__14mDoExt_McaMorfFv(this->mpMorf);
    this->unkAAC = (f32) this->unk1F8;
    this->unkAB0 = this->unk1FC;
    this->unkAB4 = this->unk200;
    offsetZero__10daNpc_So_cFv(this);
    setAnm__10daNpc_So_cFScb(this, 1, 0);
    this->unkA79 = (u8) (s32) cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk1CC);
    temp_r30 = &g_dComIfG_gameInfo + 0x624;
    if ((isEventBit__11dSv_event_cFUs(temp_r30, 0x901U) == 0) && (strcmp(&g_dComIfG_gameInfo + 0x5134, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x163) == 0) && ((s32) this->unk20A == 0xD) && (dComIfGs_isStageBossEnemy__Fi(3) != 0)) {
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 5);
    } else if ((strcmp(&g_dComIfG_gameInfo + 0x5134, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x163) == 0) && ((s32) this->unk20A == 4) && (dComIfGs_isStageBossEnemy__Fi(7) != 0) && (isCollect__20dSv_player_collect_cFiUc(&g_dComIfG_gameInfo + 0xB4, 0, 3U) != 0) && (isEventBit__11dSv_event_cFUs(temp_r30, 0x3A20U) == 0)) {
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 0xF);
    } else {
        this->unk280 = 0x0200000A;
        modeProc__10daNpc_So_cFQ210daNpc_So_c6Proc_ei(this, (daNpc_So_c::Proc_e) 0, 1);
    }
    this->unkBE0 = 0x1E;
    SetWall__12dBgS_AcchCirFff(&this->unkA34, m_heapsize__10daNpc_So_c.unk5C, m_heapsize__10daNpc_So_c.unk5C);
    sp8 = 0;
    Set__9dBgS_AcchFP4cXyzP4cXyzP10fopAc_ac_ciP12dBgS_AcchCirP4cXyzP5csXyzP5csXyz((dBgS_Acch *) &this->mAcch, (cXyz *) &this->unk1F8, &this->unk1E4, (fopAc_ac_c *) this, 1, &this->unkA34, &this->unk220, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */));
    this->unk898 |= 4;
    this->unk898 |= 8;
    this->unk22C = &this->mpMorf->unk50->unk24;
    temp_f1 = this->unk214;
    temp_f4 = m_heapsize__10daNpc_So_c.unk50 * temp_f1;
    temp_f1_2 = m_heapsize__10daNpc_So_c.unk1D8 * temp_f1;
    fopAcM_setCullSizeBox__FP10fopAc_ac_cffffff((fopAc_ac_c *) this, temp_f1_2, temp_f1_2, temp_f1_2, temp_f4, temp_f4, temp_f4);
    this->unk248 = m_heapsize__10daNpc_So_c.unk6C;
    this->unk258 = m_heapsize__10daNpc_So_c.unk1D0;
    this->unk26D = 0x22;
    this->unk26F = 0x22;
    this->unk104 = daNpc_So_XyCheckCB__FPvi;
    this->unk100 = daNpc_So_XyEventCB__FPvi;
    setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c(&this->unk2C4, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x13F, (fopNpc_npc_c *) this);
}

/* daNpc_So_c::getArg (void) */
void getArg__10daNpc_So_cFv(daNpc_So_c *this) {
    s16 temp_r4;

    this->unk6D0 = this->unk1DC;
    temp_r4 = this->unk6D0;
    if (((u32) temp_r4 == -1U) || (temp_r4 == 0)) {
        this->unk6D0 = 1;
    }
}

/* daNpc_So_c::_create (void) */
s32 _create__10daNpc_So_cFv(daNpc_So_c *this) {
    s32 temp_r3;

    if (!((bitwise s32) this->unk1C8 & 8)) {
        if (this != NULL) {
            __ct__10daNpc_So_cFv(this);
        }
        this->unk1C8 = (bitwise f32) ((bitwise s32) this->unk1C8 | 8);
    }
    temp_r3 = dComIfG_resLoad__FP30request_of_phase_process_classPCc(&this->unk844, &"So"->unk0);
    if (temp_r3 == 4) {
        getArg__10daNpc_So_cFv(this);
        if ((strcmp(&g_dComIfG_gameInfo + 0x5134, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x163) == 0) && ((s32) this->unk20A == 0x1A) && (isEventBit__11dSv_event_cFUs(&g_dComIfG_gameInfo + 0x624, 0x1E40U) == 0)) {
            return 5;
        }
        if (dComIfGs_isStageBossEnemy__Fi(3) == 0) {
            return 5;
        }
        if (fopAcM_entrySolidHeap__FP10fopAc_ac_cPFP10fopAc_ac_c_iUl((fopAc_ac_c *) this, (s32 (*)(fopAc_ac_c *)) createHeap_CB__FP10fopAc_ac_c, 0x1C00U) == 0) {
            return 5;
        }
        createInit__10daNpc_So_cFv(this);
        goto block_14;
    }
block_14:
    return temp_r3;
}

/* daNpc_So_c::_delete (void) */
u8 _delete__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((u8) g_dComIfG_gameInfo.unk5CDA == 8) {
        g_dComIfG_gameInfo.unk5CDA = 0U;
        g_dComIfG_gameInfo.unk5CD8 = (u16) (g_dComIfG_gameInfo.unk5CD8 ^ 0x80);
        g_dComIfG_gameInfo.unk5CDE = 0;
    }
    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&this->unk844, &"So"->unk0);
    end__19dPa_rippleEcallBackFv((dPa_rippleEcallBack *) &this->unkAE8);
    return 1U;
}

/* daNpc_SoCreate (void *) */
void daNpc_SoCreate__FPv(void *arg0) {
    _create__10daNpc_So_cFv((daNpc_So_c *) arg0);
}

/* daNpc_SoDelete (void *) */
u8 daNpc_SoDelete__FPv(void *arg0) {
    return _delete__10daNpc_So_cFv((daNpc_So_c *) arg0);
}

/* daNpc_SoExecute (void *) */
u8 daNpc_SoExecute__FPv(void *arg0) {
    return _execute__10daNpc_So_cFv((daNpc_So_c *) arg0);
}

/* daNpc_SoDraw (void *) */
u8 daNpc_SoDraw__FPv(void *arg0) {
    return _draw__10daNpc_So_cFv((daNpc_So_c *) arg0);
}

/* daNpc_SoIsDelete (void *) */
s32 daNpc_SoIsDelete__FPv(void *arg0) {
    return 1;
}

/* d_a_npc_so_cpp::__sinit void (*) (void) */
void __sinit_d_a_npc_so_cpp(void) {
    __ct__14daNpc_So_HIO_cFv(&l_HIO);
    __register_global_object(__dt__14daNpc_So_HIO_cFv, @4259);
}

/* daNpc_So_c::cutProc (void) */
void cutProc__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    dEvent_manager_c *temp_r30;
    s32 temp_r3_2;
    void *temp_r3;

    temp_r30 = &g_dComIfG_gameInfo + 0x52CC;
    this->unkB6C = getMyStaffId__16dEvent_manager_cFPCcP10fopAc_ac_ci(temp_r30, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x13F, NULL, 0);
    if ((s32) this->unkB6C != -1) {
        if ((s8) init$4529 == 0) {
            @2100.unk2D0 = (s32) @2100.unkD8;
            @2100.unk2D4 = (s32) @2100.unkDC;
            @2100.unk2D8 = (s32) @2100.unkE0;
            temp_r3 = &@2100 + 0x2D0;
            temp_r3->unkC = (s32) @2100.unkE4;
            temp_r3->unk10 = (s32) @2100.unkE8;
            temp_r3->unk14 = (s32) @2100.unkEC;
            temp_r3->unk18 = (s32) @2100.unkF0;
            temp_r3->unk1C = (s32) @2100.unkF4;
            temp_r3->unk20 = (s32) @2100.unkF8;
            temp_r3->unk24 = (s32) @2100.unkFC;
            temp_r3->unk28 = (s32) @2100.unk100;
            temp_r3->unk2C = (s32) @2100.unk104;
            temp_r3->unk30 = (s32) @2100.unk108;
            temp_r3->unk34 = (s32) @2100.unk10C;
            temp_r3->unk38 = (s32) @2100.unk110;
            temp_r3->unk3C = (s32) @2100.unk114;
            temp_r3->unk40 = (s32) @2100.unk118;
            temp_r3->unk44 = (s32) @2100.unk11C;
            temp_r3->unk48 = (s32) @2100.unk120;
            temp_r3->unk4C = (s32) @2100.unk124;
            temp_r3->unk50 = (s32) @2100.unk128;
            temp_r3->unk54 = (s32) @2100.unk12C;
            temp_r3->unk58 = (s32) @2100.unk130;
            temp_r3->unk5C = (s32) @2100.unk134;
            temp_r3->unk60 = (s32) @2100.unk138;
            temp_r3->unk64 = (s32) @2100.unk13C;
            temp_r3->unk68 = (s32) @2100.unk140;
            temp_r3->unk6C = (s32) @2100.unk144;
            temp_r3->unk70 = (s32) @2100.unk148;
            temp_r3->unk74 = (s32) @2100.unk14C;
            temp_r3->unk78 = (s32) @2100.unk150;
            temp_r3->unk7C = (s32) @2100.unk154;
            temp_r3->unk80 = (s32) @2100.unk158;
            temp_r3->unk84 = (s32) @2100.unk15C;
            temp_r3->unk88 = (s32) @2100.unk160;
            temp_r3->unk8C = (s32) @2100.unk164;
            temp_r3->unk90 = (s32) @2100.unk168;
            temp_r3->unk94 = (s32) @2100.unk16C;
            temp_r3->unk98 = (s32) @2100.unk170;
            temp_r3->unk9C = (s32) @2100.unk174;
            temp_r3->unkA0 = (s32) @2100.unk178;
            temp_r3->unkA4 = (s32) @2100.unk17C;
            temp_r3->unkA8 = (s32) @2100.unk180;
            temp_r3->unkAC = (s32) @2100.unk184;
            temp_r3->unkB0 = (s32) @2100.unk188;
            temp_r3->unkB4 = (s32) @2100.unk18C;
            temp_r3->unkB8 = (s32) @2100.unk190;
            temp_r3->unkBC = (s32) @2100.unk194;
            temp_r3->unkC0 = (s32) @2100.unk198;
            temp_r3->unkC4 = (s32) @2100.unk19C;
            temp_r3->unkC8 = (s32) @2100.unk1A0;
            temp_r3->unkCC = (s32) @2100.unk1A4;
            temp_r3->unkD0 = (s32) @2100.unk1A8;
            temp_r3->unkD4 = (s32) @2100.unk1AC;
            temp_r3->unkD8 = (s32) @2100.unk1B0;
            temp_r3->unkDC = (s32) @2100.unk1B4;
            temp_r3->unkE0 = (s32) @2100.unk1B8;
            temp_r3->unkE4 = (s32) @2100.unk1BC;
            temp_r3->unkE8 = (s32) @2100.unk1C0;
            temp_r3->unkEC = (s32) @2100.unk1C4;
            temp_r3->unkF0 = (s32) @2100.unk1C8;
            temp_r3->unkF4 = (s32) @2100.unk1CC;
            temp_r3->unkF8 = (s32) @2100.unk1D0;
            temp_r3->unkFC = (s32) @2100.unk1D4;
            temp_r3->unk100 = (s32) @2100.unk1D8;
            temp_r3->unk104 = (s32) @2100.unk1DC;
            temp_r3->unk108 = (s32) @2100.unk1E0;
            temp_r3->unk10C = (s32) @2100.unk1E4;
            temp_r3->unk110 = (s32) @2100.unk1E8;
            temp_r3->unk114 = (s32) @2100.unk1EC;
            temp_r3->unk118 = (s32) @2100.unk1F0;
            temp_r3->unk11C = (s32) @2100.unk1F4;
            temp_r3->unk120 = (s32) @2100.unk1F8;
            temp_r3->unk124 = (s32) @2100.unk1FC;
            temp_r3->unk128 = (s32) @2100.unk200;
            temp_r3->unk12C = (s32) @2100.unk204;
            temp_r3->unk130 = (s32) @2100.unk208;
            temp_r3->unk134 = (s32) @2100.unk20C;
            temp_r3->unk138 = (s32) @2100.unk210;
            temp_r3->unk13C = (s32) @2100.unk214;
            temp_r3->unk140 = (s32) @2100.unk218;
            temp_r3->unk144 = (s32) @2100.unk21C;
            temp_r3->unk148 = (s32) @2100.unk220;
            temp_r3->unk14C = (s32) @2100.unk224;
            temp_r3->unk150 = (s32) @2100.unk228;
            temp_r3->unk154 = (s32) @2100.unk22C;
            temp_r3->unk158 = (s32) @2100.unk230;
            temp_r3->unk15C = (s32) @2100.unk234;
            temp_r3->unk160 = (s32) @2100.unk238;
            temp_r3->unk164 = (s32) @2100.unk23C;
            temp_r3->unk168 = (s32) @2100.unk240;
            temp_r3->unk16C = (s32) @2100.unk244;
            temp_r3->unk170 = (s32) @2100.unk248;
            temp_r3->unk174 = (s32) @2100.unk24C;
            temp_r3->unk178 = (s32) @2100.unk250;
            temp_r3->unk17C = (s32) @2100.unk254;
            temp_r3->unk180 = (s32) @2100.unk258;
            temp_r3->unk184 = (s32) @2100.unk25C;
            temp_r3->unk188 = (s32) @2100.unk260;
            temp_r3->unk18C = (s32) @2100.unk264;
            temp_r3->unk190 = (s32) @2100.unk268;
            temp_r3->unk194 = (s32) @2100.unk26C;
            temp_r3->unk198 = (s32) @2100.unk270;
            temp_r3->unk19C = (s32) @2100.unk274;
            temp_r3->unk1A0 = (s32) @2100.unk278;
            temp_r3->unk1A4 = (s32) @2100.unk27C;
            temp_r3->unk1A8 = (s32) @2100.unk280;
            temp_r3->unk1AC = (s32) @2100.unk284;
            temp_r3->unk1B0 = (s32) @2100.unk288;
            temp_r3->unk1B4 = (s32) @2100.unk28C;
            temp_r3->unk1B8 = (s32) @2100.unk290;
            temp_r3->unk1BC = (s32) @2100.unk294;
            temp_r3->unk1C0 = (s32) @2100.unk298;
            temp_r3->unk1C4 = (s32) @2100.unk29C;
            temp_r3->unk1C8 = (s32) @2100.unk2A0;
            temp_r3->unk1CC = (s32) @2100.unk2A4;
            temp_r3->unk1D0 = (s32) @2100.unk2A8;
            temp_r3->unk1D4 = (s32) @2100.unk2AC;
            temp_r3->unk1D8 = (s32) @2100.unk2B0;
            temp_r3->unk1DC = (s32) @2100.unk2B4;
            temp_r3->unk1E0 = (s32) @2100.unk2B8;
            temp_r3->unk1E4 = (s32) @2100.unk2BC;
            temp_r3->unk1E8 = (s32) @2100.unk2C0;
            temp_r3->unk1EC = (s32) @2100.unk2C4;
            temp_r3->unk1F0 = (s32) @2100.unk2C8;
            temp_r3->unk1F4 = (s32) @2100.unk2CC;
            init$4529 = 1;
        }
        temp_r3_2 = getMyActIdx__16dEvent_manager_cFiPCPCciii(temp_r30, this->unkB6C, &@2100 + 0x84, 0x15, 1, 0);
        if (temp_r3_2 == -1) {
            cutEnd__16dEvent_manager_cFi(temp_r30, this->unkB6C);
            return;
        }
        if (getIsAddvance__16dEvent_manager_cFi(temp_r30, this->unkB6C) != 0) {
            __ptmf_scall((s32) this);
        }
        __ptmf_scall((s32) this, temp_r3_2 * 0x18);
    }
}

/* daNpc_So_c::cutSwimStart (void) */
void cutSwimStart__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::cutSwimProc (void) */
void cutSwimProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp38;
    f32 sp34;
    f32 sp30;
    dNpc_EventCut_c sp24;
    cXyz sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 var_f31;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;

    getAttnPos__15dNpc_EventCut_cFv(&sp24);
    sp30 = (bitwise f32) sp24;
    sp34 = sp28;
    sp38 = sp2C;
    this->unkAFC = m_heapsize__10daNpc_So_c.unkAC + g_regHIO.unk6F4;
    __mi__4cXyzCFRC3Vec(&sp18, (Vec *) &this->unk1F8);
    spC = (bitwise f32) sp18;
    sp10 = m_heapsize__10daNpc_So_c.unk9C;
    sp14 = sp20;
    temp_f1 = PSVECSquareMag((cXyz *) &spC);
    if (temp_f1 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0 = __frsqrte(temp_f1);
        temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) temp_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f31 = sp8;
    } else {
        var_f31 = temp_f1;
    }
    cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp30), 8, 0x400);
    if (var_f31 < (m_heapsize__10daNpc_So_c.unk68 + g_regHIO.unk6E8)) {
        this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutJumpStart (void) */
void cutJumpStart__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    dEvent_manager_c *temp_r28;
    f32 *temp_r30;
    f32 *temp_r3;

    this->unkA9C = 0xF;
    this->unk1FC = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch);
    temp_r28 = &g_dComIfG_gameInfo + 0x52CC;
    temp_r30 = getMySubstanceP__16dEvent_manager_cFiPCci(temp_r28, this->unkB6C, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x145, 0);
    temp_r3 = getMySubstanceP__16dEvent_manager_cFiPCci(temp_r28, this->unkB6C, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x14D, 0);
    this->unk20C = l_HIO.unk68;
    if (temp_r30 != NULL) {
        this->unk224 = *temp_r30;
    } else {
        this->unk224 = m_heapsize__10daNpc_So_c.unkC0;
    }
    if (temp_r3 != NULL) {
        this->unkAFC = *temp_r3;
    } else {
        this->unkAFC = m_heapsize__10daNpc_So_c.unkC4;
    }
    offsetSwim__10daNpc_So_cFv(this);
    this->unk254 = this->unkAFC;
    setAnm__10daNpc_So_cFScb(this, 4, 0);
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5938U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
}

/* daNpc_So_c::cutJumpProc (void) */
void cutJumpProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 temp_f31;

    temp_f31 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch);
    if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
        M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
        if (this->unk1FC == temp_f31) {
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5939U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
            fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkCC * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            this->unk254 = this->unkAFC;
            cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
        }
    }
}

/* daNpc_So_c::cutAppearStart (void) */
void cutAppearStart__10daNpc_So_cFv(daNpc_So_c *this) {
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x593AU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), 1.0f, 1.0f, -1.0f, -1.0f, 0U);
    offsetAppear__10daNpc_So_cFv(this);
}

/* daNpc_So_c::cutAppearProc (void) */
void cutAppearProc__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((f32) fabs(this->unkB34 - this->unkB38) < 10.0f) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutDiveStart (void) */
void cutDiveStart__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetDive__10daNpc_So_cFv(this);
}

/* daNpc_So_c::cutDiveProc (void) */
void cutDiveProc__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((f32) fabs(this->unkB34 - this->unkB38) < 10.0f) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutDisappearStart (void) */
void cutDisappearStart__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetDive__10daNpc_So_cFv(this);
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x593BU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
    fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkD0 * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
}

/* daNpc_So_c::cutDisappearProc (void) */
void cutDisappearProc__10daNpc_So_cFv(daNpc_So_c *this) {
    if ((f32) fabs(this->unkB34 - this->unkB38) < 10.0f) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutSetAnmStart (void) */
void cutSetAnmStart__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 *temp_r3;

    temp_r3 = getMySubstanceP__16dEvent_manager_cFiPCci(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x154, 4);
    if (temp_r3 != NULL) {
        if (strcmp("SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x159) == 0) {
            this->mPrmIdx = 1;
            return;
        }
        if (strcmp((s8 *) temp_r3, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x15E) == 0) {
            this->mPrmIdx = 3;
            return;
        }
        if (strcmp((s8 *) temp_r3, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x75) == 0) {
            this->mPrmIdx = 4;
            return;
        }
        if (strcmp((s8 *) temp_r3, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x70) == 0) {
            this->mPrmIdx = 2;
        }
    } else {
        this->mPrmIdx = 1;
    }
}

/* daNpc_So_c::cutSetAnmProc (void) */
void cutSetAnmProc__10daNpc_So_cFv(daNpc_So_c *this) {
    dEvent_manager_c *temp_r31;
    mDoExt_McaMorf *temp_r4;
    u8 temp_r0;
    u8 var_r5;

    temp_r31 = &g_dComIfG_gameInfo + 0x52CC;
    getMySubstanceP__16dEvent_manager_cFiPCci(temp_r31, this->unkB6C, "SO_ESA_XY\0d_a_npc_so.cpp\0modelData != 0\0Halt\0btp != 0\0m_jnt.getHeadJntNum() >= 0\0m_jnt.getBackboneJntNum() >= 0" + 0x154, 4);
    temp_r0 = this->mPrmIdx;
    if (((s8) temp_r0 == 1) || ((s8) temp_r0 == 4) || ((s8) temp_r0 == 3) || ((s8) temp_r0 == 2)) {
        cutEnd__16dEvent_manager_cFi(temp_r31, this->unkB6C);
    }
    temp_r4 = this->mpMorf;
    var_r5 = 1;
    if (!(temp_r4->unk5D & 1) && (temp_r4->unk64 != 0.0f)) {
        var_r5 = 0;
    }
    if (var_r5 != 0) {
        cutEnd__16dEvent_manager_cFi(temp_r31, this->unkB6C);
    }
}

/* daNpc_So_c::cutEffectStart (void) */
void cutEffectStart__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;
    f32 temp_f1;
    void *temp_r3;

    sp24 = this->unk1F8;
    temp_f1 = this->unk1FC;
    sp28 = temp_f1;
    sp2C = this->unk200;
    sp28 = temp_f1 - m_heapsize__10daNpc_So_c.unkD4;
    sp18 = m_heapsize__10daNpc_So_c.unkA8;
    sp1C = m_heapsize__10daNpc_So_c.unkA8;
    sp20 = m_heapsize__10daNpc_So_c.unkA8;
    sp8 = -1;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    temp_r3 = set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x8152U, (cXyz *) &sp24, NULL, (cXyz *) &sp18, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    temp_r3->unk1F0 = (f32) m_heapsize__10daNpc_So_c.unkD8;
    temp_r3->unk1F4 = (f32) m_heapsize__10daNpc_So_c.unkDC;
    temp_r3->unk1F8 = (f32) m_heapsize__10daNpc_So_c.unkA8;
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x58BDU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
}

/* daNpc_So_c::cutEffectProc (void) */
void cutEffectProc__10daNpc_So_cFv(daNpc_So_c *this) {
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutEquipStart (void) */
void cutEquipStart__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::cutEquipProc (void) */
void cutEquipProc__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA78 = 1;
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutUnequipStart (void) */
void cutUnequipStart__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::cutUnequipProc (void) */
void cutUnequipProc__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA78 = 0;
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutEatesaStart (void) */
void cutEatesaStart__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetZero__10daNpc_So_cFv(this);
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
}

/* daNpc_So_c::cutEatesaProc (void) */
void cutEatesaProc__10daNpc_So_cFv(daNpc_So_c *this) {
    daShip_c *temp_r3;

    temp_r3 = fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchEsa_CB__FPvPv, this);
    if (temp_r3 != NULL) {
        temp_r3->unk298 = 1;
        return;
    }
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutEatesaFirstStart (void) */
void cutEatesaFirstStart__10daNpc_So_cFv(daNpc_So_c *this) {
    offsetZero__10daNpc_So_cFv(this);
    this->unkB28 = 650.0f + g_regHIO.unk4B8;
    this->unkB30 = g_dComIfG_gameInfo.unk5B54->unk20E;
    this->unkBDA = 1;
    this->unkAFC = 0.0f;
    this->unk254 = 0.0f;
}

/* daNpc_So_c::cutEatesaFirstProc (void) */
void cutEatesaFirstProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp70;
    f32 sp6C;
    f32 sp68;
    f32 sp64;
    f32 sp60;
    f32 sp5C;
    cXyz sp50;
    cXyz sp44;
    cXyz sp38;
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
    daShip_c *temp_r3;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f2;
    f32 temp_f3;
    f32 var_f1;
    f32 var_f1_2;
    f32 var_f31;
    f32 var_f31_2;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    f64 temp_f0_7;
    f64 temp_f0_8;
    f64 temp_f0_9;
    s32 temp_r0;

    sp68 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
    sp6C = g_dComIfG_gameInfo.unk5B4C->unk1FC;
    sp70 = g_dComIfG_gameInfo.unk5B4C->unk200;
    if ((void *) g_dComIfG_gameInfo.unk5B54 != NULL) {
        temp_f3 = g_dComIfG_gameInfo.unk5B54->unk1F8;
        sp68 = temp_f3;
        sp6C = g_dComIfG_gameInfo.unk5B54->unk1FC;
        temp_f2 = g_dComIfG_gameInfo.unk5B54->unk200;
        sp70 = temp_f2;
        temp_r0 = ((s32) (u16) ((g_dComIfG_gameInfo.unk5B54->unk20E + g_regHIO.unk750) - 0x4000) >> jmaSinShift) * 4;
        sp68 = temp_f3 + (m_heapsize__10daNpc_So_c.unkE4 * *(jmaSinTable + temp_r0));
        sp70 = temp_f2 + (m_heapsize__10daNpc_So_c.unkE4 * *(jmaCosTable + temp_r0));
    }
    __mi__4cXyzCFRC3Vec(&sp50, (Vec *) &this->unk1F8);
    sp2C = (bitwise f32) sp50;
    sp30 = m_heapsize__10daNpc_So_c.unk9C;
    sp34 = sp58;
    var_f1 = PSVECSquareMag((cXyz *) &sp2C);
    if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp10 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp10;
    }
    if (var_f1 < (m_heapsize__10daNpc_So_c.unk80 + g_regHIO.unk4B8)) {
        cLib_addCalc2__FPffff(&this->unkB28, m_heapsize__10daNpc_So_c.unk5C, m_heapsize__10daNpc_So_c.unkE8, m_heapsize__10daNpc_So_c.unkEC + g_regHIO.unk4B4);
    }
    this->unkB32 = 0x300;
    this->unkB10 = sp68;
    this->unkB14 = sp6C;
    this->unkB18 = sp70;
    this->unkB14 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unkB10, &this->mAcch);
    dLib_setCirclePath__FP18dLib_circle_path_c((dLib_circle_path_c *) &this->unkB10);
    this->unkB20 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch(&this->unkB1C, &this->mAcch);
    sp5C = this->unkB1C.unk0;
    temp_f1 = this->unkB20;
    sp60 = temp_f1;
    sp64 = this->unkB24;
    sp60 = temp_f1 - m_heapsize__10daNpc_So_c.unk50;
    __mi__4cXyzCFRC3Vec(&sp44, (Vec *) &this->unk1F8);
    sp20 = (bitwise f32) sp44;
    sp24 = m_heapsize__10daNpc_So_c.unk9C;
    sp28 = sp4C;
    temp_f1_2 = PSVECSquareMag((cXyz *) &sp20);
    if (temp_f1_2 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0_4 = __frsqrte(temp_f1_2);
        temp_f0_5 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_4 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_4 * temp_f0_4)));
        temp_f0_6 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_5 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_5 * temp_f0_5)));
        spC = (f32) ((f64) temp_f1_2 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_6 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_6 * temp_f0_6)))));
        var_f31 = spC;
    } else {
        var_f31 = temp_f1_2;
    }
    __mi__4cXyzCFRC3Vec(&sp38, (Vec *) &this->unk1F8);
    sp14 = (bitwise f32) sp38;
    sp18 = m_heapsize__10daNpc_So_c.unk9C;
    sp1C = sp40;
    var_f1_2 = PSVECSquareMag((cXyz *) &sp14);
    if (var_f1_2 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0_7 = __frsqrte(var_f1_2);
        temp_f0_8 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_7 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1_2 * (temp_f0_7 * temp_f0_7)));
        temp_f0_9 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_8 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1_2 * (temp_f0_8 * temp_f0_8)));
        sp8 = (f32) ((f64) var_f1_2 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_9 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1_2 * (temp_f0_9 * temp_f0_9)))));
        var_f1_2 = sp8;
    }
    this->unk324 = 0;
    this->unk318 = sp5C;
    this->unk31C = sp60;
    this->unk320 = sp64;
    this->unkB44 = sp5C;
    this->unkB48 = sp60;
    this->unkB4C = sp64;
    if (var_f1_2 < (m_heapsize__10daNpc_So_c.unk50 + g_regHIO.unk6E8)) {
        var_f31_2 = m_heapsize__10daNpc_So_c.unk64;
        M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
        if (var_f1_2 == m_heapsize__10daNpc_So_c.unk74) {
            var_f31_2 = m_heapsize__10daNpc_So_c.unk9C;
            temp_r3 = fopAcIt_Judge__FPFPvPv_PvPv((void *(*)(void *, void *)) searchEsa_CB__FPvPv, this);
            if (temp_r3 != NULL) {
                temp_r3->unk298 = 1;
            } else {
                cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
                this->unkBDA = 0;
                this->unkB04 = var_f31_2;
            }
        }
    } else if (var_f31 < (m_heapsize__10daNpc_So_c.unkF0 + g_regHIO.unk6E8)) {
        var_f31_2 = m_heapsize__10daNpc_So_c.unkF4;
    } else {
        var_f31_2 = m_heapsize__10daNpc_So_c.unkF8 + g_regHIO.unk6F4;
    }
    if (var_f31_2 != m_heapsize__10daNpc_So_c.unk9C) {
        cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz(&this->unkB54, (cXyz *) &sp5C), 3, 0x1200);
    }
    cLib_addCalcPos2__FP4cXyzRC4cXyzff((cXyz *) &this->unk1F8, (cXyz *) &sp5C, m_heapsize__10daNpc_So_c.unkFC, var_f31_2);
    this->unk1FC = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch) - m_heapsize__10daNpc_So_c.unk50;
}

/* daNpc_So_c::cutJumpMapopenStart (void) */
void cutJumpMapopenStart__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkBBC = 0;
    offsetZero__10daNpc_So_cFv(this);
    setAnm__10daNpc_So_cFScb(this, 2, 0);
    this->unkBDA = 1;
}

/* daNpc_So_c::cutJumpMapopenProc (void) */
void cutJumpMapopenProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f31;
    f32 temp_f31_2;
    s32 temp_r0;
    u16 temp_r0_2;
    u16 temp_r0_3;

    if ((void *) g_dComIfG_gameInfo.unk5B54 == NULL) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
        return;
    }
    temp_r0 = this->unkBBC;
    switch (temp_r0) {                              /* irregular */
    case 0:
        this->unkB34 = this->unkB38;
        this->unk258 = l_HIO.unk80;
        this->unk224 = l_HIO.unk84;
        this->unkB00 = this->unk224;
        this->unk254 = m_heapsize__10daNpc_So_c.unk9C;
        this->unkAFC = this->unk254;
        this->unk20C = l_HIO.unk68;
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x593CU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
        this->unkBBC += 1;
        return;
    case 1:
        temp_r0_2 = g_dComIfG_gameInfo.unk5B54->unk20E + 0x4000;
        this->unkBB0 = g_dComIfG_gameInfo.unk5B54->unk1F8;
        this->unkBB4 = g_dComIfG_gameInfo.unk5B54->unk1FC;
        this->unkBB8 = g_dComIfG_gameInfo.unk5B54->unk200;
        temp_f2 = -l_HIO.unk88;
        this->unkBB0 += temp_f2 * *(jmaSinTable + (((s32) temp_r0_2 >> jmaSinShift) * 4));
        this->unkBB8 += temp_f2 * *(jmaCosTable + (((s32) temp_r0_2 >> jmaSinShift) * 4));
        temp_f31 = this->unk1FC;
        if (this->unk224 < (f32) m_heapsize__10daNpc_So_c.unk100) {
            this->unk224 = m_heapsize__10daNpc_So_c.unkA8;
        }
        if (cLib_chasePosXZ__FP4cXyzRC4cXyzf((cXyz *) &this->unk1F8, (cXyz *) &this->unkBB0, l_HIO.unk90) != 0) {
            this->unk1FC = temp_f31;
            if ((u8) l_HIO.unk94 == 0) {
                g_dComIfG_gameInfo.unk5BF1 = 2;
            }
            this->unk20E += 0x8000;
            this->unkBBC += 1;
            return;
        }
        return;
    case 2:
        temp_r0_3 = g_dComIfG_gameInfo.unk5B54->unk20E + 0x4000;
        this->unkBB0 = g_dComIfG_gameInfo.unk5B54->unk1F8;
        this->unkBB4 = g_dComIfG_gameInfo.unk5B54->unk1FC;
        this->unkBB8 = g_dComIfG_gameInfo.unk5B54->unk200;
        temp_f2_2 = -l_HIO.unk8C;
        this->unkBB0 += temp_f2_2 * *(jmaSinTable + (((s32) temp_r0_3 >> jmaSinShift) * 4));
        this->unkBB8 += temp_f2_2 * *(jmaCosTable + (((s32) temp_r0_3 >> jmaSinShift) * 4));
        temp_f31_2 = this->unk1FC;
        if (cLib_chasePosXZ__FP4cXyzRC4cXyzf((cXyz *) &this->unk1F8, (cXyz *) &this->unkBB0, l_HIO.unk90) != 0) {
            this->unk1FC = temp_f31_2;
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x593BU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
            fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkCC * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            this->unk254 = m_heapsize__10daNpc_So_c.unk9C;
            this->unk224 = m_heapsize__10daNpc_So_c.unk9C;
            this->unkB00 = this->unk224;
            cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
            offsetDive__10daNpc_So_cFv(this);
            this->unkBDA = 0;
        }
        break;
    }
}

/* daNpc_So_c::cutMiniGameStart (void) */
void cutMiniGameStart__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    s32 temp_r30;
    void *temp_r29;

    this->unkB78 = 0;
    this->unkB80 = 0;
    this->unkB7C = 0;
    this->unkB74 = 0;
    offsetDive__10daNpc_So_cFv(this);
    g_dComIfG_gameInfo.unk5CDA = 8;
    g_dComIfG_gameInfo.unk5CD8 = (u16) (g_dComIfG_gameInfo.unk5CD8 | 0x80);
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x8F0U, NULL, 0U, 0, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
    temp_r29 = g_dComIfG_gameInfo.unk5B4C;
    temp_r30 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
    if ((s16) g_regHIO.unk506 == 0) {
        this->unkBCC = temp_r29->unk1F8;
        this->unkBD0 = temp_r29->unk1FC;
        this->unkBD4 = temp_r29->unk200;
        this->unkBCC += m_heapsize__10daNpc_So_c.unk50 * *(jmaSinTable + (((s32) (u16) temp_r29->unk20E >> jmaSinShift) * 4));
        this->unkBD4 += m_heapsize__10daNpc_So_c.unk50 * *(jmaCosTable + (((s32) (u16) temp_r29->unk20E >> jmaSinShift) * 4));
        this->unkBD0 += m_heapsize__10daNpc_So_c.unk74;
        this->unkBC0 = temp_r29->unk1F8;
        this->unkBC4 = temp_r29->unk1FC;
        this->unkBC8 = temp_r29->unk200;
        this->unkBC0 += m_heapsize__10daNpc_So_c.unk108 * *(jmaSinTable + (((s32) (u16) temp_r29->unk20E >> jmaSinShift) * 4));
        this->unkBC8 += m_heapsize__10daNpc_So_c.unk108 * *(jmaCosTable + (((s32) (u16) temp_r29->unk20E >> jmaSinShift) * 4));
        this->unkBC4 += m_heapsize__10daNpc_So_c.unk50;
        sp14 = this->unkBCC;
        sp18 = this->unkBD0;
        sp1C = this->unkBD4;
        sp8 = this->unkBC0;
        spC = this->unkBC4;
        sp10 = this->unkBC8;
        Reset__9dCamera_cF4cXyz4cXyz(temp_r30 + 0x244, (cXyz) &sp14, (cXyz) &sp8);
        Start__9dCamera_cFv(temp_r30 + 0x244);
    }
    temp_r29->unk304 = 3;
    temp_r29->unk30C = 0;
    temp_r29->unk314 = 0x44;
    this->unkBDB = 0;
    this->unkBDE = 0;
}

/* daNpc_So_c::cutMiniGameProc (void) */
void cutMiniGameProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 spB4;
    f32 spB0;
    f32 spAC;
    f32 spA8;
    f32 spA4;
    f32 spA0;
    f32 sp9C;
    f32 sp98;
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
    dNpc_EventCut_c sp4C;
    cXyz sp40;
    dNpc_EventCut_c sp34;
    cXyz sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f31;
    f32 var_f31;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    f64 temp_f0_4;
    f64 temp_f0_5;
    f64 temp_f0_6;
    s16 var_r29;
    s32 temp_r0;
    u8 var_r28;
    void *temp_r30;

    temp_r30 = g_dComIfG_gameInfo.unk5B44;
    var_r29 = temp_r30->unk20E;
    spAC = temp_r30->unk1F8;
    spB0 = temp_r30->unk1FC;
    spB4 = temp_r30->unk200;
    var_r28 = 0;
    if ((void *) g_dComIfG_gameInfo.unk5B54 != NULL) {
        var_r29 = (g_dComIfG_gameInfo.unk5B54->unk20E + g_regHIO.unk750) - 0x4000;
        spAC = g_dComIfG_gameInfo.unk5B54->unk1F8;
        spB0 = g_dComIfG_gameInfo.unk5B54->unk1FC;
        spB4 = g_dComIfG_gameInfo.unk5B54->unk200;
    }
    if (temp_r30->unk2A4 & 0x20000000) {
        this->unkB78 += 1;
    }
    if ((s32) this->unkB78 >= 0xA) {
        var_r28 = 1;
        if ((s32) this->unkB74 <= 6) {
            temp_r30->unk314 = 6;
            temp_r30->unk30C = 1;
            initCam__10daNpc_So_cFv(this);
            moveCam__10daNpc_So_cFv(this);
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x8F1U, NULL, 0U, 0, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
            g_dComIfG_gameInfo.unk5CDA = 0;
            g_dComIfG_gameInfo.unk5CD8 = (u16) (g_dComIfG_gameInfo.unk5CD8 ^ 0x80);
            g_dComIfG_gameInfo.unk5CDE = 0;
            this->unk254 = m_heapsize__10daNpc_So_c.unk9C;
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
            return;
        }
    }
    temp_r0 = this->unkB74;
    if ((u32) temp_r0 <= 7U) {
        switch (temp_r0) {
        case 0:
            setAnm__10daNpc_So_cFScb(this, 2, 0);
            this->unkA9C = 0x1E;
            this->unkB84 = 0;
            this->unkB74 += 1;
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            this->unk254 = this->unkAFC;
            offsetSwim__10daNpc_So_cFv(this);
            return;
        case 1:
            if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
                this->unkB74 += 1;
                return;
            }
            break;
        case 2:
            setAnm__10daNpc_So_cFScb(this, 2, 0);
            this->unkB8C = m_heapsize__10daNpc_So_c.unk10C.unk0 + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk78);
            this->unkB88 = (-m_heapsize__10daNpc_So_c.unk10C.unk0 * m_heapsize__10daNpc_So_c.unk110) + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk10C.unk0);
            spA0 = this->unkB88;
            spA4 = m_heapsize__10daNpc_So_c.unk9C;
            spA8 = this->unkB8C;
            sp94 = m_heapsize__10daNpc_So_c.unk9C;
            sp98 = m_heapsize__10daNpc_So_c.unk9C;
            sp9C = m_heapsize__10daNpc_So_c.unk9C;
            mDoMtx_YrotS__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, var_r29);
            PSMTXMultVec(&now__14mDoMtx_stack_c, &spA0, (cXyz *) &sp94);
            PSVECAdd(&sp94, &spAC, &sp94);
            this->unk318 = sp94;
            this->unk31C = sp98;
            this->unk320 = sp9C;
            this->unkB74 += 1;
            offsetSwim__10daNpc_So_cFv(this);
            return;
        case 3:
            getAttnPos__15dNpc_EventCut_cFv(&sp4C);
            sp88 = (bitwise f32) sp4C;
            sp8C = sp50;
            sp90 = sp54;
            this->unkAFC = m_heapsize__10daNpc_So_c.unkF4 + g_regHIO.unk6F4;
            __mi__4cXyzCFRC3Vec(&sp40, (Vec *) &this->unk1F8);
            sp1C = (bitwise f32) sp40;
            sp20 = m_heapsize__10daNpc_So_c.unk9C;
            sp24 = sp48;
            temp_f1 = PSVECSquareMag((cXyz *) &sp1C);
            if (temp_f1 > m_heapsize__10daNpc_So_c.unk9C) {
                temp_f0 = __frsqrte(temp_f1);
                temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0 * temp_f0)));
                temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_2 * temp_f0_2)));
                spC = (f32) ((f64) temp_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1 * (temp_f0_3 * temp_f0_3)))));
                var_f31 = spC;
            } else {
                var_f31 = temp_f1;
            }
            cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp88), 8, 0x400);
            if (var_f31 < (m_heapsize__10daNpc_So_c.unk68 + g_regHIO.unk6E8)) {
                this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
                this->unkB74 += 1;
                return;
            }
            break;
        case 4:
            this->unkB88 += (-m_heapsize__10daNpc_So_c.unk88 * m_heapsize__10daNpc_So_c.unk110) + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unk88);
            sp7C = this->unkB88;
            sp80 = m_heapsize__10daNpc_So_c.unk9C;
            sp84 = this->unkB8C;
            sp70 = m_heapsize__10daNpc_So_c.unk9C;
            sp74 = m_heapsize__10daNpc_So_c.unk9C;
            sp78 = m_heapsize__10daNpc_So_c.unk9C;
            mDoMtx_YrotS__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, var_r29);
            PSMTXMultVec(&now__14mDoMtx_stack_c, &sp7C, (cXyz *) &sp70);
            PSVECAdd(&sp70, &spAC, &sp70);
            this->unk318 = sp70;
            this->unk31C = sp74;
            this->unk320 = sp78;
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            this->unk254 = this->unkAFC;
            this->unkA9C = 0x1E;
            this->unkB74 += 1;
            offsetSwim__10daNpc_So_cFv(this);
            return;
        case 5:
            getAttnPos__15dNpc_EventCut_cFv(&sp34);
            sp64 = (bitwise f32) sp34;
            sp68 = sp38;
            sp6C = sp3C;
            this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
            __mi__4cXyzCFRC3Vec(&sp28, (Vec *) &this->unk1F8);
            sp10 = (bitwise f32) sp28;
            sp14 = m_heapsize__10daNpc_So_c.unk9C;
            sp18 = sp30;
            temp_f1_2 = PSVECSquareMag((cXyz *) &sp10);
            if (temp_f1_2 > m_heapsize__10daNpc_So_c.unk9C) {
                temp_f0_4 = __frsqrte(temp_f1_2);
                temp_f0_5 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_4 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_4 * temp_f0_4)));
                temp_f0_6 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_5 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_5 * temp_f0_5)));
                sp8 = (f32) ((f64) temp_f1_2 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_6 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) temp_f1_2 * (temp_f0_6 * temp_f0_6)))));
            }
            cLib_addCalcAngleS2__FPssss(&this->unk20E, cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp64), 8, 0x400);
            if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
                this->unkB74 += 1;
                return;
            }
            break;
        case 6:
            this->unk1FC = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch);
            this->unkAFC = this->unkB08 * (m_heapsize__10daNpc_So_c.unkC4 + cM_rndF__Ff(m_heapsize__10daNpc_So_c.unkC4));
            this->unk254 = this->unkAFC;
            this->unk224 = (m_heapsize__10daNpc_So_c.unk90 * this->unkB08) + (m_heapsize__10daNpc_So_c.unk114 * this->unk254);
            if (this->unk224 > l_HIO.unk50) {
                this->unk224 = l_HIO.unk50;
            }
            this->unk258 = m_heapsize__10daNpc_So_c.unk118;
            setAnm__10daNpc_So_cFScb(this, 4, 0);
            this->unk20C = l_HIO.unk68;
            offsetZero__10daNpc_So_cFv(this);
            this->unkB74 += 1;
            seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5938U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
            return;
        case 7:
            temp_f31 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unk1F8, &this->mAcch);
            if (((u8) this->unkB84 == 0) && ((s32) this->unkB7C < 0xA)) {
                if ((s32) this->unk6D8 == 0) {
                    M2C_ERROR(/* unknown instruction: cror eq, gt, eq */);
                    if (this->unk1FC == (m_heapsize__10daNpc_So_c.unk74 + temp_f31)) {
                        SetR__8cM3dGSphFf(&this->unk830, l_HIO.unk3C);
                        SetC__8cM3dGSphFRC4cXyz(&this->unk830, (cXyz *) &this->unk1F8);
                        Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo + 0x26A4, (cCcD_Obj *) &this->unk718);
                    } else {
                        sp58 = m_heapsize__10daNpc_So_c.unk9C;
                        sp5C = m_heapsize__10daNpc_So_c.unk11C;
                        sp60 = m_heapsize__10daNpc_So_c.unk9C;
                        SetR__8cM3dGSphFf(&this->unk830, m_heapsize__10daNpc_So_c.unk9C);
                        SetC__8cM3dGSphFRC4cXyz(&this->unk830, (cXyz *) &sp58);
                        Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo + 0x26A4, (cCcD_Obj *) &this->unk718);
                    }
                }
                if (checkTgHit__10daNpc_So_cFv(this) != 0) {
                    this->unkB7C += 1;
                    if ((s32) this->unkB7C >= 0xA) {
                        this->unkB7C = 0xA;
                    }
                }
            }
            M2C_ERROR(/* unknown instruction: cror eq, lt, eq */);
            if (this->unk1FC == temp_f31) {
                seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x5939U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
                fopKyM_createWpillar__FPC4cXyzffi((cXyz *) &this->unk1F8, m_heapsize__10daNpc_So_c.unkCC * this->unk214, m_heapsize__10daNpc_So_c.unkCC, 0);
                this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
                this->unk254 = this->unkAFC;
                this->unkB80 += 1;
                if (((s32) this->unkB80 >= 0xA) || (var_r28 != 0)) {
                    temp_r30->unk314 = 6;
                    temp_r30->unk30C = 1;
                    initCam__10daNpc_So_cFv(this);
                    moveCam__10daNpc_So_cFv(this);
                    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x8F1U, NULL, 0U, 0, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkA8, m_heapsize__10daNpc_So_c.unkC8, m_heapsize__10daNpc_So_c.unkC8, 0U);
                    g_dComIfG_gameInfo.unk5CDA = 0;
                    g_dComIfG_gameInfo.unk5CD8 = (u16) (g_dComIfG_gameInfo.unk5CD8 ^ 0x80);
                    g_dComIfG_gameInfo.unk5CDE = 0;
                    this->unk254 = m_heapsize__10daNpc_So_c.unk9C;
                    this->unkAFC = m_heapsize__10daNpc_So_c.unk9C;
                    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
                    return;
                }
                this->unkB74 = 0;
            }
            break;
        }
    }
}

/* daNpc_So_c::cutTurnStart (void) */
void cutTurnStart__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA9C = 0x1E;
}

/* daNpc_So_c::cutTurnProc (void) */
void cutTurnProc__10daNpc_So_cFv(daNpc_So_c *this) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    f32 temp_f2;
    f32 temp_f3;
    s16 temp_r3;
    s32 temp_r0;
    s32 temp_r30;

    if ((void *) g_dComIfG_gameInfo.unk5B54 == NULL) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
        return;
    }
    temp_f3 = g_dComIfG_gameInfo.unk5B54->unk1F8;
    sp8 = temp_f3;
    spC = g_dComIfG_gameInfo.unk5B54->unk1FC;
    temp_f2 = g_dComIfG_gameInfo.unk5B54->unk200;
    sp10 = temp_f2;
    temp_r0 = ((s32) (u16) (g_dComIfG_gameInfo.unk5B54->unk20E + 0x4000) >> jmaSinShift) * 4;
    sp8 = temp_f3 + (-300.0f * *(jmaSinTable + temp_r0));
    sp10 = temp_f2 + (-300.0f * *(jmaCosTable + temp_r0));
    temp_r3 = cLib_targetAngleY__FP4cXyzP4cXyz((cXyz *) &this->unk1F8, (cXyz *) &sp8);
    cLib_addCalcAngleS2__FPssss(&this->unk20E, temp_r3, 8, 0x400);
    temp_r30 = cLib_distanceAngleS__Fss(this->unk20E, temp_r3);
    if ((cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) || (temp_r30 <= 0x400)) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutMiniGameWarpStart (void) */
void cutMiniGameWarpStart__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    u16 spC;
    s32 sp8;
    daShip_c *temp_r29;
    s16 temp_r6;
    s32 temp_r30_2;
    u16 temp_r0;
    void *temp_r30;

    zel_basic__11JAIZelBasic.unk0->unkBF = 1;
    temp_r30 = g_dComIfG_gameInfo.unk5B4C;
    temp_r30->unk304 = 3;
    temp_r30->unk30C = 0;
    temp_r29 = g_dComIfG_gameInfo.unk5B54;
    this->unkB94 = dLib_getWaterY__FR4cXyzR12dBgS_ObjAcch((cXyz *) &this->unkB90, &this->mAcch);
    this->unkBA0 = temp_r29->unk1F8;
    this->unkBA4 = temp_r29->unk1FC;
    this->unkBA8 = temp_r29->unk200;
    this->unkBAC = temp_r29->unk20E;
    initStartPos__8daShip_cFPC4cXyzs(temp_r29, (cXyz *) &this->unkB90, (s16) (this->unkB9C + 0x4000));
    sp8 = temp_r29->unk20C;
    spC = temp_r29->unk210;
    temp_r6 = this->unkB9C;
    temp_r30->unk31C->unk84(temp_r30, &this->unkB90, (s16) (temp_r6 + g_regHIO.unk510), temp_r6);
    this->unkAFC = 0.0f;
    this->unk254 = this->unkAFC;
    temp_r0 = (temp_r29->unk20E + g_regHIO.unk750) - 0x4000;
    this->unk1F8 = this->unkB90;
    this->unk1FC = this->unkB94;
    this->unk200 = this->unkB98;
    this->unk1F8 += 300.0f * *(jmaSinTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unk200 += 300.0f * *(jmaCosTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unkBDB = 1;
    this->unkA9C = g_regHIO.unk746 + 0xA;
    temp_r30_2 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
    sp1C = this->unkBCC;
    sp20 = this->unkBD0;
    sp24 = this->unkBD4;
    sp10 = this->unkBC0;
    sp14 = this->unkBC4;
    sp18 = this->unkBC8;
    Reset__9dCamera_cF4cXyz4cXyz(temp_r30_2 + 0x244, (cXyz) &sp1C, (cXyz) &sp10);
    Start__9dCamera_cFv(temp_r30_2 + 0x244);
}

/* daNpc_So_c::cutMiniGameWarpProc (void) */
void cutMiniGameWarpProc__10daNpc_So_cFv(daNpc_So_c *this) {
    u16 temp_r0;

    g_dComIfG_gameInfo.unk5B4C->unk304 = 3;
    g_dComIfG_gameInfo.unk5B4C->unk30C = 0;
    g_dComIfG_gameInfo.unk5B4C->unk314 = 6;
    g_dComIfG_gameInfo.unk5B4C->unk30C = 1;
    temp_r0 = (g_dComIfG_gameInfo.unk5B54->unk20E + g_regHIO.unk750) - 0x4000;
    this->unk1F8 = g_dComIfG_gameInfo.unk5B54->unk1F8;
    this->unk1FC = g_dComIfG_gameInfo.unk5B54->unk1FC;
    this->unk200 = g_dComIfG_gameInfo.unk5B54->unk200;
    this->unk1F8 += 300.0f * *(jmaSinTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unk200 += 300.0f * *(jmaCosTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutMiniGameReturnStart (void) */
void cutMiniGameReturnStart__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    s16 sp8;
    daShip_c *temp_r30;
    s16 temp_r6;
    void *temp_r29;

    zel_basic__11JAIZelBasic.unk0->unkBF = 0;
    temp_r29 = g_dComIfG_gameInfo.unk5B4C;
    temp_r29->unk304 = 3;
    temp_r29->unk30C = 0;
    sp8 = 0xA7;
    temp_r30 = fopAcIt_Judge__FPFPvPv_PvPv(fpcSch_JudgeForPName__FPvPv, &sp8);
    temp_r6 = this->unkB9C;
    temp_r29->unk31C->unk84(temp_r29, &this->unkBA0, (s16) (temp_r6 + g_regHIO.unk510), temp_r6);
    initStartPos__8daShip_cFPC4cXyzs(temp_r30, (cXyz *) &this->unkBA0, this->unkBAC);
    this->unkAFC = 0.0f;
    this->unk254 = this->unkAFC;
    this->unkBDB = 1;
    setAnm__10daNpc_So_cFScb(this, 2, 0);
    offsetDive__10daNpc_So_cFv(this);
}

/* daNpc_So_c::cutMiniGameReturnProc (void) */
void cutMiniGameReturnProc__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp5C;
    f32 sp58;
    f32 sp54;
    f32 sp50;
    f32 sp4C;
    f32 sp48;
    cXyz sp3C;
    cXyz sp30;
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
    f32 var_f1;
    f64 temp_f0;
    f64 temp_f0_2;
    f64 temp_f0_3;
    s32 temp_r31;
    u16 temp_r0;
    void *temp_r29;

    temp_r0 = (g_dComIfG_gameInfo.unk5B54->unk20E + g_regHIO.unk750) - 0x4000;
    this->unk1F8 = g_dComIfG_gameInfo.unk5B54->unk1F8;
    this->unk1FC = g_dComIfG_gameInfo.unk5B54->unk1FC;
    this->unk200 = g_dComIfG_gameInfo.unk5B54->unk200;
    this->unk1F8 += m_heapsize__10daNpc_So_c.unk88 * *(jmaSinTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unk200 += m_heapsize__10daNpc_So_c.unk88 * *(jmaCosTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unk20C = 0;
    __mi__4cXyzCFRC3Vec(&sp3C, (Vec *) &this->unkBA0);
    spC = (bitwise f32) sp3C;
    sp10 = m_heapsize__10daNpc_So_c.unk9C;
    sp14 = sp44;
    var_f1 = PSVECSquareMag((cXyz *) &spC);
    if (var_f1 > m_heapsize__10daNpc_So_c.unk9C) {
        temp_f0 = __frsqrte(var_f1);
        temp_f0_2 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0 * temp_f0)));
        temp_f0_3 = m_heapsize__10daNpc_So_c.unkB0 * temp_f0_2 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_2 * temp_f0_2)));
        sp8 = (f32) ((f64) var_f1 * (m_heapsize__10daNpc_So_c.unkB0 * temp_f0_3 * (m_heapsize__10daNpc_So_c.unkB8 - ((f64) var_f1 * (temp_f0_3 * temp_f0_3)))));
        var_f1 = sp8;
    }
    if (var_f1 < m_heapsize__10daNpc_So_c.unk78) {
        temp_r29 = g_dComIfG_gameInfo.unk5B4C;
        temp_r29->unk304 = 3;
        temp_r29->unk30C = 0;
        temp_r29->unk314 = 6;
        temp_r29->unk30C = 1;
        sp54 = m_heapsize__10daNpc_So_c.unk9C;
        sp58 = m_heapsize__10daNpc_So_c.unk9C;
        sp5C = m_heapsize__10daNpc_So_c.unk9C;
        sp48 = m_heapsize__10daNpc_So_c.unk9C;
        sp4C = m_heapsize__10daNpc_So_c.unk9C;
        sp50 = m_heapsize__10daNpc_So_c.unk9C;
        mDoMtx_YrotS__FPA4_fs(calc_mtx, temp_r29->unk20E);
        sp54 = m_heapsize__10daNpc_So_c.unk124 + g_regHIO.unk1C;
        sp58 = m_heapsize__10daNpc_So_c.unk74 + g_regHIO.unk20;
        sp5C = m_heapsize__10daNpc_So_c.unk50 + g_regHIO.unk24;
        MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp54, (cXyz *) &sp48);
        __pl__4cXyzCFRC3Vec(&sp30, temp_r29 + 0x1F8);
        this->unkBC0 = (bitwise f32) sp30;
        this->unkBC4 = sp34;
        this->unkBC8 = sp38;
        this->unkBCC = temp_r29->unk1F8;
        this->unkBD0 = temp_r29->unk1FC;
        this->unkBD4 = temp_r29->unk200;
        this->unkBD0 += m_heapsize__10daNpc_So_c.unk128 + g_regHIO.unk28;
        temp_r31 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
        sp24 = this->unkBCC;
        sp28 = this->unkBD0;
        sp2C = this->unkBD4;
        sp18 = this->unkBC0;
        sp1C = this->unkBC4;
        sp20 = this->unkBC8;
        Reset__9dCamera_cF4cXyz4cXyz(temp_r31 + 0x244, (cXyz) &sp24, (cXyz) &sp18);
        Start__9dCamera_cFv(temp_r31 + 0x244);
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutPartnerShipStart (void) */
void cutPartnerShipStart__10daNpc_So_cFv(daNpc_So_c *this) {

}

/* daNpc_So_c::cutPartnerShipProc (void) */
void cutPartnerShipProc__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    dEvt_control_c *temp_r30;

    temp_r30 = &g_dComIfG_gameInfo + 0x51D8;
    temp_r30->unkCC = getPId__14dEvt_control_cFPv(temp_r30, g_dComIfG_gameInfo.unk5B54);
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutMiniGameWaitStart (void) */
void cutMiniGameWaitStart__10daNpc_So_cFv(daNpc_So_c *this) {
    s32 temp_r31;

    temp_r31 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
    if ((s16) g_regHIO.unk506 == 0) {
        Stop__9dCamera_cFv(temp_r31 + 0x244);
        SetTrimSize__9dCamera_cFl(temp_r31 + 0x244, 1);
    }
}

/* daNpc_So_c::cutMiniGameWaitProc (void) */
void cutMiniGameWaitProc__10daNpc_So_cFv(daNpc_So_c *this) {
    cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
}

/* daNpc_So_c::cutMiniGameEndStart (void) */
void cutMiniGameEndStart__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA9C = g_regHIO.unk744 + 0x2D;
    this->unkBDE = 0;
}

/* daNpc_So_c::cutMiniGameEndProc (void) */
void cutMiniGameEndProc__10daNpc_So_cFv(daNpc_So_c *this) {
    initCam__10daNpc_So_cFv(this);
    moveCam__10daNpc_So_cFv(this);
    if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutMiniGamePlTurnStart (void) */
void cutMiniGamePlTurnStart__10daNpc_So_cFv(daNpc_So_c *this) {
    this->unkA9C = g_regHIO.unk748 + 0xA;
    this->unkBDE = 0;
}

/* daNpc_So_c::cutMiniGamePlTurnProc (void) */
void cutMiniGamePlTurnProc__10daNpc_So_cFv(daNpc_So_c *this) {
    u16 temp_r0;

    g_dComIfG_gameInfo.unk5B4C->unk304 = 3;
    g_dComIfG_gameInfo.unk5B4C->unk30C = 0;
    g_dComIfG_gameInfo.unk5B4C->unk314 = 6;
    g_dComIfG_gameInfo.unk5B4C->unk30C = 1;
    temp_r0 = (g_dComIfG_gameInfo.unk5B54->unk20E + g_regHIO.unk750) - 0x4000;
    this->unk1F8 = g_dComIfG_gameInfo.unk5B54->unk1F8;
    this->unk1FC = g_dComIfG_gameInfo.unk5B54->unk1FC;
    this->unk200 = g_dComIfG_gameInfo.unk5B54->unk200;
    this->unk1F8 += 300.0f * *(jmaSinTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    this->unk200 += 300.0f * *(jmaCosTable + (((s32) temp_r0 >> jmaSinShift) * 4));
    if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::cutMiniGamePlUpStart (void) */
void cutMiniGamePlUpStart__10daNpc_So_cFv(daNpc_So_c *this) {
    s32 temp_r31;

    this->unkA9C = 0x1E;
    temp_r31 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
    Stop__9dCamera_cFv(temp_r31 + 0x244);
    SetTrimSize__9dCamera_cFl(temp_r31 + 0x244, 1);
}

/* daNpc_So_c::cutMiniGamePlUpProc (void) */
void cutMiniGamePlUpProc__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    cXyz sp20;
    f32 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    void *temp_r29;

    temp_r29 = g_dComIfG_gameInfo.unk5B4C;
    temp_r29->unk314 = 6;
    sp38 = m_heapsize__10daNpc_So_c.unk9C;
    sp3C = m_heapsize__10daNpc_So_c.unk9C;
    sp40 = m_heapsize__10daNpc_So_c.unk9C;
    sp2C = m_heapsize__10daNpc_So_c.unk9C;
    sp30 = m_heapsize__10daNpc_So_c.unk9C;
    sp34 = m_heapsize__10daNpc_So_c.unk9C;
    mDoMtx_YrotS__FPA4_fs(calc_mtx, temp_r29->unk20E);
    sp38 = m_heapsize__10daNpc_So_c.unk124 + g_regHIO.unk1C;
    sp3C = m_heapsize__10daNpc_So_c.unk74 + g_regHIO.unk20;
    sp40 = m_heapsize__10daNpc_So_c.unk50 + g_regHIO.unk24;
    MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp38, (cXyz *) &sp2C);
    __pl__4cXyzCFRC3Vec(&sp20, temp_r29 + 0x1F8);
    this->unkBC0 = (bitwise f32) sp20;
    this->unkBC4 = sp24;
    this->unkBC8 = sp28;
    this->unkBCC = temp_r29->unk1F8;
    this->unkBD0 = temp_r29->unk1FC;
    this->unkBD4 = temp_r29->unk200;
    this->unkBD0 += m_heapsize__10daNpc_So_c.unk128 + g_regHIO.unk28;
    sp14 = this->unkBCC;
    sp18 = this->unkBD0;
    sp1C = this->unkBD4;
    sp8 = this->unkBC0;
    spC = this->unkBC4;
    sp10 = this->unkBC8;
    Set__9dCamera_cF4cXyz4cXyz((&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10 + 0x244, (cXyz) &sp14, (cXyz) &sp8);
    if (cLib_calcTimer<i>__FPi(&this->unkA9C) == 0) {
        cutEnd__16dEvent_manager_cFi(&g_dComIfG_gameInfo + 0x52CC, this->unkB6C);
    }
}

/* daNpc_So_c::initCam (void) */
void initCam__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
    s32 temp_r29;

    if ((u8) this->unkBDE != 1) {
        this->unkBDE = 1;
        temp_r29 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
        if ((s16) g_regHIO.unk506 == 0) {
            Stop__9dCamera_cFv(temp_r29 + 0x244);
            SetTrimSize__9dCamera_cFl(temp_r29 + 0x244, 1);
            this->unkBCC = g_dComIfG_gameInfo.unk5B4C->unk1F8;
            this->unkBD0 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
            this->unkBD4 = g_dComIfG_gameInfo.unk5B4C->unk200;
            this->unkBCC += m_heapsize__10daNpc_So_c.unk50 * *(jmaSinTable + (((s32) (u16) g_dComIfG_gameInfo.unk5B4C->unk20E >> jmaSinShift) * 4));
            this->unkBD4 += m_heapsize__10daNpc_So_c.unk50 * *(jmaCosTable + (((s32) (u16) g_dComIfG_gameInfo.unk5B4C->unk20E >> jmaSinShift) * 4));
            this->unkBD0 += m_heapsize__10daNpc_So_c.unk74;
            this->unkBC0 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
            this->unkBC4 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
            this->unkBC8 = g_dComIfG_gameInfo.unk5B4C->unk200;
            this->unkBC0 += m_heapsize__10daNpc_So_c.unk108 * *(jmaSinTable + (((s32) (u16) g_dComIfG_gameInfo.unk5B4C->unk20E >> jmaSinShift) * 4));
            this->unkBC8 += m_heapsize__10daNpc_So_c.unk108 * *(jmaCosTable + (((s32) (u16) g_dComIfG_gameInfo.unk5B4C->unk20E >> jmaSinShift) * 4));
            this->unkBC4 += m_heapsize__10daNpc_So_c.unk50;
        }
    }
}

/* daNpc_So_c::moveCam (void) */
void moveCam__10daNpc_So_cFv(daNpc_So_c *this, ? arg_sp0) {
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
    f32 temp_f2;
    f32 temp_f3;
    f32 temp_f4;
    s16 temp_r0;
    s32 temp_r30;

    if ((s16) g_regHIO.unk506 == 0) {
        temp_r30 = (&g_dComIfG_gameInfo + ((s8) g_dComIfG_gameInfo.unk5B48 * 0x34))->unk5B10;
        temp_f4 = g_dComIfG_gameInfo.unk5B4C->unk1F8;
        sp20 = temp_f4;
        temp_f3 = g_dComIfG_gameInfo.unk5B4C->unk1FC;
        sp24 = temp_f3;
        temp_f2 = g_dComIfG_gameInfo.unk5B4C->unk200;
        sp28 = temp_f2;
        temp_r0 = g_dComIfG_gameInfo.unk5B4C->unk20E;
        sp20 = temp_f4 + (m_heapsize__10daNpc_So_c.unk68 * *(jmaSinTable + (((s32) (u16) temp_r0 >> jmaSinShift) * 4)));
        sp28 = temp_f2 + (m_heapsize__10daNpc_So_c.unk68 * *(jmaCosTable + (((s32) (u16) temp_r0 >> jmaSinShift) * 4)));
        sp24 = temp_f3 + m_heapsize__10daNpc_So_c.unk50;
        sp2C = temp_f4;
        sp30 = temp_f3;
        sp34 = temp_f2;
        sp2C = temp_f4 + (m_heapsize__10daNpc_So_c.unk12C * *(jmaSinTable + (((s32) (u16) temp_r0 >> jmaSinShift) * 4)));
        sp34 = temp_f2 + (m_heapsize__10daNpc_So_c.unk12C * *(jmaCosTable + (((s32) (u16) temp_r0 >> jmaSinShift) * 4)));
        sp30 = temp_f3 + m_heapsize__10daNpc_So_c.unk88;
        cLib_addCalcPos2__FP4cXyzRC4cXyzff((cXyz *) &this->unkBC0, (cXyz *) &sp2C, m_heapsize__10daNpc_So_c.unkE8, m_heapsize__10daNpc_So_c.unkC4);
        cLib_addCalcPos2__FP4cXyzRC4cXyzff((cXyz *) &this->unkBCC, (cXyz *) &sp20, m_heapsize__10daNpc_So_c.unkE8, m_heapsize__10daNpc_So_c.unkC4);
        sp14 = this->unkBCC;
        sp18 = this->unkBD0;
        sp1C = this->unkBD4;
        sp8 = this->unkBC0;
        spC = this->unkBC4;
        sp10 = this->unkBC8;
        Set__9dCamera_cF4cXyz4cXyz(temp_r30 + 0x244, (cXyz) &sp14, (cXyz) &sp8);
    }
}