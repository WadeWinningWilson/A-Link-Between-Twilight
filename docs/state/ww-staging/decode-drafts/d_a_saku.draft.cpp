// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_saku (29 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_saku\asm\d\actor\d_a_saku.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct J3DMaterial {
    /* 0x00 */ char pad0[0x2C];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x2C */ void **unk2C;                        /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2C (receipt f_op_actor.h) */
    /* 0x30 */ char pad30[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x34 */ void **unk34;                        /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
} J3DMaterial;                                      /* size >= 0x38 */

typedef struct J3DModel {
    /* 0x0 */ char pad0[4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ J3DModelData *unk4;                   /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
} J3DModel;                                         /* size >= 0x8 */

typedef struct J3DModelData {
    /* 0x00 */ char pad0[0x5C];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x5C */ u16 unk5C;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5C (receipt f_op_actor.h) */
    /* 0x5E */ char pad5E[2];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x5E (receipt f_op_actor.h) */
    /* 0x60 */ s32 unk60;                           /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x60 (receipt f_op_actor.h) */
} J3DModelData;                                     /* size >= 0x64 */

typedef struct JAIZelBasic {
    /* 0x0 */ JAIZelBasic *unk0;                    /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0x4 */

typedef struct JUTAssertion {
    /* 0x00 */ char pad0[0x30];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x30 */ u8 unk30;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x30 (receipt f_op_actor.h) */
    /* 0x31 */ u8 unk31;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x31 (receipt f_op_actor.h) */
    /* 0x32 */ u8 unk32;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x32 (receipt f_op_actor.h) */
    /* 0x33 */ u8 unk33;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x33 (receipt f_op_actor.h) */
    /* 0x34 */ u8 unk34;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x34 (receipt f_op_actor.h) */
    /* 0x35 */ u8 unk35;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x35 (receipt f_op_actor.h) */
    /* 0x36 */ u8 unk36;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x36 (receipt f_op_actor.h) */
    /* 0x37 */ char pad37[1];  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x37 (receipt f_op_actor.h) */
    /* 0x38 */ u8 unk38;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x38 (receipt f_op_actor.h) */
    /* 0x39 */ u8 unk39;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x39 (receipt f_op_actor.h) */
    /* 0x3A */ u8 unk3A;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3A (receipt f_op_actor.h) */
} JUTAssertion;                                     /* size >= 0x3B */

struct __vt__9sakuHIO_c {
    /* 0x0 */ s32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x4 */ s32 unk4;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x8 */ void *(*__dt)(sakuHIO_c *, s16);
};                                                  /* size = 0xC */

typedef struct cXyz {
    /* 0x0 */ f32 unk0;                             /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} cXyz;                                             /* size >= 0x4 */

typedef struct dRes_control_c {
    /* 0x0 */ s8 unk0;                              /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} dRes_control_c;                                   /* size >= 0x1 */

typedef struct dSv_info_c {
    /* 0x00000 */ char pad0[0x12A0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x012A0 */ cBgS unk12A0;                     /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x012A0 */ char pad12A0[0x1404];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x026A4 */ cCcS unk26A4;                     /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x026A4 */ char pad26A4[0x32FC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x059A0 */ dVibration_c unk59A0;             /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x059A0 */ char pad59A0[0x124];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC4 */ dPa_control_c *unk5AC4;           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05AC8 */ char pad5AC8[0x268];              /* maybe part of unk5AC4[0x9B]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05D30 */ s32 unk5D30;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05D34 */ s32 unk5D34;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05D38 */ s32 unk5D38;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05D3C */ s32 unk5D3C;                      /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x05D40 */ char pad5D40[0x16280];            /* maybe part of unk5D3C[0x58A1]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ ? unk1BFC0;                       /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x1BFC0 */ char pad1BFC0[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} dSv_info_c;                                       /* size >= 0x1BFC1 */

typedef struct daSaku_c {
    /* 0x000 */ u8 unk0;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x001 */ u8 unk1;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x1 (receipt f_op_actor.h) */
    /* 0x002 */ u8 unk2;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x2 (receipt f_op_actor.h) */
    /* 0x003 */ u8 unk3;                            /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x3 (receipt f_op_actor.h) */
    /* 0x004 */ dRes_control_c *unk4;               /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x4 (receipt f_op_actor.h) */
    /* 0x008 */ dRes_control_c *unk8;               /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0x8 (receipt f_op_actor.h) */
    /* 0x00C */ char padC[0x100];                   /* maybe part of unk8[0x41]? */  /* = fopAc_ac_c::leafdraw_class base @0x0 +0xC (receipt f_op_actor.h) */
    /* 0x10C */ dKy_tevstr_c unk10C;                /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x10C */ char pad10C[0x88];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C (receipt f_op_actor.h) */
    /* 0x194 */ ? unk194;                           /* inferred */  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x88 (receipt f_op_actor.h) */
    /* 0x194 */ char pad194[0x4E];  /* = fopAc_ac_c::dKy_tevstr_c tevStr @0x10C +0x88 (receipt f_op_actor.h) */
    /* 0x1E2 */ u8 unk1E2;                          /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x12 (receipt f_op_actor.h) */
    /* 0x1E3 */ char pad1E3[0x15];                  /* maybe part of unk1E2[0x16]? */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x13 (receipt f_op_actor.h) */
    /* 0x1F8 */ cXyz unk1F8;                        /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1F8 */ char pad1F8[4];  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ csXyz unk204;                       /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x204 */ char pad204[6];  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x20A */ u8 unk20A;                          /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x12 (receipt f_op_actor.h) */
    /* 0x20B */ char pad20B[1];  /* = fopAc_ac_c::actor_place current @0x1F8 +0x13 (receipt f_op_actor.h) */
    /* 0x20C */ s16 unk20C;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C (receipt f_op_actor.h) */
    /* 0x20E */ s16 unk20E;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x2 (receipt f_op_actor.h) */
    /* 0x210 */ s16 unk210;                         /* inferred */  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x4 (receipt f_op_actor.h) */
    /* 0x212 */ char pad212[2];  /* = fopAc_ac_c::csXyz shape_angle @0x20C +0x6 (receipt f_op_actor.h) */
    /* 0x214 */ f32 unk214;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 (receipt f_op_actor.h) */
    /* 0x218 */ f32 unk218;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 +0x4 (receipt f_op_actor.h) */
    /* 0x21C */ f32 unk21C;                         /* inferred */  /* = fopAc_ac_c::cXyz scale @0x214 +0x8 (receipt f_op_actor.h) */
    /* 0x220 */ char pad220[0xC];                   /* maybe part of unk21C[4]? */  /* = fopAc_ac_c::cXyz speed @0x220 (receipt f_op_actor.h) */
    /* 0x22C */ s32 unk22C;                         /* inferred */  /* = fopAc_ac_c::MtxP cullMtx @0x22C (receipt f_op_actor.h) */
    /* 0x230 */ char pad230[0x30];                  /* maybe part of unk22C[0xD]? */  /* = fopAc_ac_c::fopAc_cullSizeSphere sphere @0x230 (receipt f_op_actor.h) */
    /* 0x260 */ Vec unk260;                         /* inferred */  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x260 */ char pad260[0x70];  /* = fopAc_ac_c::cXyz eyePos @0x260 (receipt f_op_actor.h) */
    /* 0x2D0 */ dCcD_Stts unk2D0;                   /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D0 */ char pad2D0[0x75C];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA2C */ f32 unkA2C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA30 */ f32 unkA30;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA34 */ f32 unkA34;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA38 */ f32 unkA38;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA3C */ f32 unkA3C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA40 */ f32 unkA40;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA44 */ f32 unkA44;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA48 */ f32 unkA48;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA4C */ f32 unkA4C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA50 */ f32 unkA50;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA54 */ f32 unkA54;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA58 */ f32 unkA58;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA5C */ f32 unkA5C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA60 */ f32 unkA60;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA64 */ f32 unkA64;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA68 */ f32 unkA68;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA6C */ f32 unkA6C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA70 */ f32 unkA70;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xA74 */ char padA74[0x390];                 /* maybe part of unkA70[0xE5]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE04 */ request_of_phase_process_class unkE04; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE04 */ char padE04[8];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE0C */ request_of_phase_process_class unkE0C; /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE0C */ char padE0C[0x18];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE24 */ s32 unkE24;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE28 */ u32 unkE28;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE2C */ char padE2C[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE30 */ u32 unkE30;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE34 */ char padE34[0x18];                  /* maybe part of unkE30[7]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE4C */ ? unkE4C;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE4C */ char padE4C[0x30];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE7C */ ? unkE7C;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xE7C */ char padE7C[0x40];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEBC */ s32 unkEBC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEC0 */ s32 unkEC0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEC4 */ char padEC4[0x1C];                  /* maybe part of unkEC0[8]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEE0 */ s32 unkEE0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEE4 */ s32 unkEE4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEE8 */ s32 unkEE8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEEC */ s32 unkEEC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF0 */ char padEF0[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF2 */ u8 unkEF2;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF3 */ char padEF3[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF4 */ u8 unkEF4;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF5 */ char padEF5[3];                     /* maybe part of unkEF4[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEF8 */ s32 unkEF8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xEFC */ s32 unkEFC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xF00 */ s32 unkF00;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0xF04 */ s32 unkF04;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} daSaku_c;                                         /* size >= 0xF08 */

typedef struct mDoMtx_stack_c {
    /* 0x00 */ f32 unk0[4];                         /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} mDoMtx_stack_c;                                   /* size >= 0x10 */

u32 ChkTgHit__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
void *GetTgHitObj__12dCcD_GObjInfFv(dCcD_GObjInf *this); /* extern */
? Init__9dCcD_SttsFiiP10fopAc_ac_c(dCcD_Stts *this, s32 arg0, s32 arg1, fopAc_ac_c *arg2); /* extern */
? Move__4dBgWFv(dBgW *this);                        /* extern */
? OSPanic(? *, ?, void *);                          /* extern */
? PSMTXCopy(mDoMtx_stack_c *, ? *);                 /* extern */
? PSMTXMultVec(mDoMtx_stack_c *, void *, void *);   /* extern */
? PSMTXTrans(mDoMtx_stack_c *, f32, f32, f32, f32); /* extern */
u8 Regist__4dBgSFP4cBgWP10fopAc_ac_c(dBgS *this, cBgW *arg0, fopAc_ac_c *arg1); /* extern */
? Release__4cBgSFP4cBgW(cBgS *this, cBgW *arg0);    /* extern */
? SetC__8cM3dGCylFRC4cXyz(cM3dGCyl *this, cXyz *arg0); /* extern */
u8 Set__4cBgWFP6cBgD_tUlPA3_A4_f(cBgW *this, cBgD_t *arg0, u32 arg1, f32 (*arg2)[3][4]); /* extern */
? Set__4cCcSFP8cCcD_Obj(cCcS *this, cCcD_Obj *arg0); /* extern */
? Set__8dCcD_CylFRC11dCcD_SrcCyl(dCcD_Cyl *this, dCcD_SrcCyl *arg0); /* extern */
? StartShock__12dVibration_cFii4cXyz(dVibration_c *this, s32 arg0, s32 arg1, cXyz arg2); /* extern */
cBgW *__ct__4dBgWFv(dBgW *this);                    /* extern */
dBgW *__nw__FUl(u32 arg0);                          /* extern */
? __register_global_object(? *, void *(*)(sakuHIO_c *, s16), ? *); /* extern */
? cLib_chaseF__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
s32 cLib_chaseUC__FPUcUcUc(u8 *arg0, u8 arg1, u8 arg2); /* extern */
? dComIfG_resDelete__FP30request_of_phase_process_classPCc(request_of_phase_process_class *arg0, s8 *arg1); /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
? deleteChild__16mDoHIO_subRoot_cFSc(mDoHIO_subRoot_c *this, s8 arg0); /* extern */
? freeAll__7JKRHeapFv(JKRHeap *this);               /* extern */
cBgD_t *getRes__14dRes_control_cFPCclP11dRes_info_ci(dRes_control_c *this, s8 *arg0, s32 arg1, dRes_info_c *arg2, s32 arg3); /* extern */
JUTAssertion *getSDevice__12JUTAssertionFv(JUTAssertion *this); /* extern */
s32 isSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
u32 mDoExt_J3DModel__create__FP12J3DModelDataUlUl(J3DModelData *arg0, u32 arg1, u32 arg2); /* extern */
? mDoExt_destroySolidHeap__FP12JKRSolidHeap(JKRSolidHeap *arg0); /* extern */
? mDoExt_modelUpdateDL__FP8J3DModel(J3DModel *arg0); /* extern */
JKRHeap *mDoExt_setCurrentHeap__FP7JKRHeap(JKRHeap *arg0); /* extern */
mDoMtx_stack_c *mDoMtx_YrotM__FPA4_fs(f32 (*arg0)[4], s16 arg1); /* extern */
? mDoMtx_ZXYrotM__FPA4_fsss(f32 (*arg0)[4], s16 arg1, s16 arg2, s16 arg3); /* extern */
? onSwitch__10dSv_info_cFii(dSv_info_c *this, s32 arg0, s32 arg1); /* extern */
? scaleM__14mDoMtx_stack_cFfff(mDoMtx_stack_c *this, f32 arg0, f32 arg1, f32 arg2); /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
? setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(dScnKy_env_light_c *this, J3DModel *arg0, dKy_tevstr_c *arg1); /* extern */
? set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(dPa_control_c *this, u8 arg0, u16 arg1, cXyz *arg2, csXyz *arg3, cXyz *arg4, u8 arg5, dPa_levelEcallBack *arg6, s8 arg7, _GXColor *arg8, _GXColor *arg9, cXyz *arg10); /* extern */
? settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(dScnKy_env_light_c *this, s32 arg0, cXyz *arg1, dKy_tevstr_c *arg2); /* extern */
? showAssert__12JUTAssertionFUlPCciPCc(JUTAssertion *this, u32 arg0, s8 *arg1, s32 arg2, s8 *arg3); /* extern */
s32 CreateHeap__8daSaku_cFii(daSaku_c *this, s32 arg0, s32 arg1); /* static */
s32 GetDzbId__8daSaku_cFi(daSaku_c *this, s32 arg0); /* static */
s32 MoveBGResist__8daSaku_cFii(daSaku_c *this, s32 arg0, s32 arg1); /* static */
void *__dt__9sakuHIO_cFv(sakuHIO_c *this, s16 destroyFlag); /* static */
? _daSaku_create__8daSaku_cFv(daSaku_c *this);      /* static */
s32 broken__8daSaku_cFi(daSaku_c *this, s32 arg0);  /* static */
s32 burn__8daSaku_cFv(daSaku_c *this);              /* static */
void changeXluMaterialAlpha__FP11J3DMaterialUcb(J3DMaterial *arg0, u8 arg1, u8 arg2); /* static */
void daSaku_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0); /* static */
s32 daSaku_Delete__FP8daSaku_c(daSaku_c *arg0);     /* static */
s32 daSaku_Draw__FP8daSaku_c(daSaku_c *arg0);       /* static */
s32 daSaku_Execute__FP8daSaku_c(daSaku_c *arg0);    /* static */
s32 daSaku_IsDelete__FP8daSaku_c(daSaku_c *arg0);   /* static */
s32 loadModel__8daSaku_cFiii(daSaku_c *this, s32 arg0, s32 arg1, s32 arg2); /* static */
s32 loadMoveBG__8daSaku_cFiii(daSaku_c *this, s32 arg0, s32 arg1, s32 arg2); /* static */
s32 matAlphaAnim__FP12J3DModelDataUcb(J3DModelData *arg0, u8 arg1, u8 arg2); /* static */
void setCol__8daSaku_cFv(daSaku_c *this);           /* static */
s32 setEffBreak__8daSaku_cFi(daSaku_c *this, s32 arg0); /* static */
s32 setEffFire__8daSaku_cFi(daSaku_c *this, s32 arg0); /* static */
void setMtx__8daSaku_cFv(daSaku_c *this);           /* static */
extern dSv_info_c g_dComIfG_gameInfo;
extern dScnKy_env_light_c g_env_light;
extern ? j3dSys;
extern ? mDoHIO_root;
extern mDoMtx_stack_c now__14mDoMtx_stack_c;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static JUTAssertion @2100;                          /* unable to generate initializer: non-zero padding */
static daSaku_c m_arcname__8daSaku_c;               /* unable to generate initializer: confusing struct layout */
static daSaku_c m_smoke_alpha__8daSaku_c;           /* unable to generate initializer: confusing struct layout */
static daSaku_c m_alpha_start_time__8daSaku_c;      /* unable to generate initializer: confusing struct layout */
static daSaku_c m_saku_alpha_out_time__8daSaku_c;   /* unable to generate initializer: confusing struct layout */
static daSaku_c m_fade_time__8daSaku_c;             /* unable to generate initializer: confusing struct layout */
static daSaku_c dust_color__8daSaku_c;              /* unable to generate initializer: confusing struct layout */
static daSaku_c m_cyl_src__8daSaku_c;               /* unable to generate initializer: confusing struct layout */
static ? @4006;
static ? l_sakuHIO;
static ? @4323;                                     /* unable to generate initializer: unknown type; const */
static ? @4325;                                     /* unable to generate initializer: unknown type; const */
static ? @4384;                                     /* unable to generate initializer: unknown type; const */
static ? @4402;                                     /* unable to generate initializer: unknown type; const */
static JUTAssertion @4403;                          /* unable to generate initializer: non-zero padding; const */
static daSaku_c m_max_particle_timer__8daSaku_c;    /* unable to generate initializer: confusing struct layout; const */

/* daSaku_c::CreateInit (void) */
void CreateInit__8daSaku_cFv(daSaku_c *this) {
    s32 var_ctr;
    s32 var_ctr_2;
    s32 var_r30;
    s32 var_r3;
    s32 var_r3_2;
    s32 var_r4;
    void *temp_r11;
    void *temp_r12;
    void *temp_r6;

    var_r30 = 0;
    var_r3 = 0;
    var_r4 = 0;
    var_ctr = 2;
    do {
        temp_r11 = this + var_r4;
        temp_r11->unkEBC = 0;
        temp_r11->unkEAC = 0;
        temp_r12 = this + var_r3;
        temp_r12->unkEDC = 0xFF;
        temp_r12->unkEDD = 0;
        temp_r11->unkEE0 = -1;
        *(this + (var_r30 + 0xEF0)) = 2;
        var_r30 += 1;
        var_r3 += 2;
        var_r4 += 4;
        var_ctr -= 1;
    } while (var_ctr != 0);
    this->unkEF4 = 0;
    this->unkEEC = 0;
    this->unk22C = this->unkE24 + 0x24;
    Init__9dCcD_SttsFiiP10fopAc_ac_c(&this->unk2D0, 0xFF, 0xFF, (fopAc_ac_c *) this);
    setCol__8daSaku_cFv(this);
    setMtx__8daSaku_cFv(this);
    var_r3_2 = 0;
    var_ctr_2 = 2;
    do {
        temp_r6 = this + var_r3_2;
        temp_r6->unk2A6 = (u8) dust_color__8daSaku_c.unk0;
        temp_r6->unk2A7 = (u8) dust_color__8daSaku_c.unk1;
        temp_r6->unk2A8 = (u8) dust_color__8daSaku_c.unk2;
        temp_r6->unk2A9 = (u8) dust_color__8daSaku_c.unk3;
        temp_r6->unk2A1 = 1;
        var_r3_2 += 0x20;
        var_ctr_2 -= 1;
    } while (var_ctr_2 != 0);
}

/* daSaku_c::saku_draw_sub (int) */
s32 saku_draw_sub__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    s32 temp_r29;
    s32 temp_r30;
    u8 temp_r5;
    u8 var_r28;
    void *temp_r26;
    void *temp_r30_2;
    void *temp_r31;

    var_r28 = 1;
    temp_r29 = arg0 * 2;
    temp_r26 = this + temp_r29;
    temp_r5 = temp_r26->unkEDC;
    if (temp_r5 < (u8) l_sakuHIO.unk16) {
        var_r28 = 0;
    }
    temp_r30 = arg0 * 8;
    temp_r31 = this + temp_r30;
    if (((u32) temp_r31->unkE14 != 0U) && ((J3DModel *) temp_r31->unkE24 != NULL) && (temp_r5 != 0)) {
        settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, &this->unk1F8, &this->unk10C);
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r31->unkE24, &this->unk10C);
        matAlphaAnim__FP12J3DModelDataUcb(temp_r31->unkE24->unk4, temp_r26->unkEDC, var_r28);
        j3dSys.unk48 = (s32) g_dComIfG_gameInfo.unk5D30;
        j3dSys.unk4C = (s32) g_dComIfG_gameInfo.unk5D34;
        mDoExt_modelUpdateDL__FP8J3DModel(temp_r31->unkE24);
        j3dSys.unk48 = (s32) g_dComIfG_gameInfo.unk5D38;
        j3dSys.unk4C = (s32) g_dComIfG_gameInfo.unk5D3C;
        matAlphaAnim__FP12J3DModelDataUcb(temp_r31->unkE24->unk4, 0xFFU, 1U);
    }
    temp_r30_2 = this + temp_r30;
    if (((u32) temp_r30_2->unkE18 != 0U) && ((J3DModel *) temp_r30_2->unkE28 != NULL) && ((u8) (this + temp_r29)->unkEDD != 0)) {
        settingTevStruct__18dScnKy_env_light_cFiP4cXyzP12dKy_tevstr_c(&g_env_light, 0, &this->unk1F8, &this->unk10C);
        setLightTevColorType__18dScnKy_env_light_cFP8J3DModelP12dKy_tevstr_c(&g_env_light, temp_r30_2->unkE28, &this->unk10C);
        matAlphaAnim__FP12J3DModelDataUcb(temp_r30_2->unkE28->unk4, 0xFFU, var_r28 == 0);
        mDoExt_modelUpdateDL__FP8J3DModel(temp_r30_2->unkE28);
        matAlphaAnim__FP12J3DModelDataUcb(temp_r30_2->unkE28->unk4, 0xFFU, 1U);
    }
    return 1;
}

/* daSaku_c::mode_break_none (int) */
s32 mode_break_none__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    f32 sp10;
    f32 spC;
    f32 sp8;
    dCcD_GObjInf *temp_r31;
    s32 temp_r4;
    s32 temp_r4_2;
    s32 temp_r4_3;
    s32 var_r23;
    s32 var_r26;
    u32 var_r27;
    u32 var_r28;
    u8 temp_r0;
    u8 var_r3;
    u8 var_r3_2;
    u8 var_r3_3;
    void *temp_r3;

    var_r28 = 0;
    var_r27 = 0;
    var_r26 = 0;
    var_r23 = 0;
loop_1:
    temp_r31 = this + (arg0 * 0x390) + (var_r23 + 0x30C);
    if ((ChkTgHit__12dCcD_GObjInfFv(temp_r31) != 0U) && (temp_r3 = GetTgHitObj__12dCcD_GObjInfFv(temp_r31), ((temp_r3 == NULL) == 0))) {
        temp_r0 = this->unkEF2;
        if (temp_r0 == 0) {
            var_r3 = 0;
            temp_r4 = temp_r3->unk10;
            if ((temp_r4 & 2) || (temp_r4 & 8) || (temp_r4 & 0x20) || (temp_r4 & 0x400) || (temp_r4 & 0x800) || (temp_r4 & 0x04000000) || (temp_r4 & 0x10000000) || (temp_r4 & 0x10000)) {
                var_r3 = 1;
            }
            var_r27 |= var_r3;
        } else if (temp_r0 == 1) {
            var_r3_2 = 0;
            temp_r4_2 = temp_r3->unk10;
            if ((temp_r4_2 & 0x400) || (temp_r4_2 & 0x20) || (temp_r4_2 & 0x800) || (temp_r4_2 & 0x04000000)) {
                var_r3_2 = 1;
            }
            var_r27 |= var_r3_2;
        }
        if (var_r27 != 0) {
            sp8 = 0.0f;
            spC = 1.0f;
            sp10 = 0.0f;
            StartShock__12dVibration_cFii4cXyz(&g_dComIfG_gameInfo.unk59A0, 4, -0x21, (cXyz) &sp8);
        }
        var_r3_3 = 0;
        temp_r4_3 = temp_r3->unk10;
        if ((temp_r4_3 & 0x200) || (temp_r4_3 & 0x20000) || (temp_r4_3 & 0x40000)) {
            var_r3_3 = 1;
        }
        var_r28 |= var_r3_3;
        if (var_r28 == 0) {
            goto block_28;
        }
    } else {
block_28:
        var_r26 += 1;
        var_r23 += 0x130;
        if (var_r26 < 3) {
            goto loop_1;
        }
    }
    if (var_r28 != 0) {
        return burn__8daSaku_cFv(this);
    }
    if (var_r27 != 0) {
        if ((arg0 == 1) && ((s32) this->unkEF8 == 1)) {
            broken__8daSaku_cFi(this, 0);
        }
        return broken__8daSaku_cFi(this, arg0);
    }
    return 1;
}

/* daSaku_c::mode_break_fire (int) */
s32 mode_break_fire__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    s32 temp_cr0_eq;
    u8 temp_r3;
    void *temp_r29;
    void *temp_r29_2;
    void *temp_r4;

    if ((s32) this->unkEBC > (s32) m_saku_alpha_out_time__8daSaku_c) {
        temp_r29 = this + (arg0 * 2);
        cLib_chaseUC__FPUcUcUc(temp_r29 + 0xEDD, 0xFFU, (u8) l_sakuHIO.unk10);
        if (cLib_chaseUC__FPUcUcUc(temp_r29 + 0xEDC, 0U, (u8) l_sakuHIO.unk10) != 0) {
            temp_r29_2 = this + (arg0 * 8);
            temp_cr0_eq = (JKRSolidHeap *) temp_r29_2->unkE14 == NULL;
            if ((temp_cr0_eq == 0) && (temp_cr0_eq == 0)) {
                temp_r4 = this + arg0;
                temp_r3 = temp_r4->unkEF0;
                if (temp_r3 != 0) {
                    temp_r4->unkEF0 = (u8) (temp_r3 - 1);
                    if ((u8) temp_r4->unkEF0 == 0) {
                        mDoExt_destroySolidHeap__FP12JKRSolidHeap(temp_r29_2->unkE14);
                        temp_r29_2->unkE14 = NULL;
                        temp_r29_2->unkE24 = 0;
                    }
                }
            }
        }
    }
    return 1;
}

/* daSaku_c::mode_break_throw_obj (int) */
s32 mode_break_throw_obj__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    s32 temp_r29;
    s32 temp_r31_2;
    s8 temp_r0;
    u8 temp_r3;
    void **temp_r3_2;
    void *temp_r30;
    void *temp_r31;
    void *temp_r4;
    void *temp_r4_2;

    temp_r31 = this + (arg0 * 8);
    if (((JKRSolidHeap *) temp_r31->unkE14 != NULL) && ((u32) temp_r31->unkE18 != 0U)) {
        temp_r4 = this + arg0;
        temp_r3 = temp_r4->unkEF0;
        if (temp_r3 != 0) {
            temp_r4->unkEF0 = (u8) (temp_r3 - 1);
            if ((u8) temp_r4->unkEF0 == 0) {
                mDoExt_destroySolidHeap__FP12JKRSolidHeap(temp_r31->unkE14);
                temp_r31->unkE14 = NULL;
                temp_r31->unkE24 = 0;
            }
        }
    }
    temp_r31_2 = arg0 * 4;
    if ((s32) (this + temp_r31_2)->unkEBC >= (s32) m_alpha_start_time__8daSaku_c) {
        temp_r29 = arg0 << 5;
        temp_r30 = this + temp_r29;
        if ((void *) temp_r30->unk294 != NULL) {
            cLib_chaseF__FPfff(this + (temp_r31_2 + 0xEB4), 0.0f, (f32) l_sakuHIO.unk12 / (255.0f * (f32) m_fade_time__8daSaku_c));
            temp_r4_2 = this + temp_r31_2;
            temp_r4_2->unkEB4 = (f32) fabs(temp_r4_2->unkEB4);
            temp_r0 = (s8) (255.0f * temp_r4_2->unkEB4);
            temp_r30->unk294->unk1FF = temp_r0;
            if ((u8) temp_r0 == 0) {
                temp_r3_2 = this + (temp_r29 + 0x290);
                (*temp_r3_2)->unk20(temp_r3_2);
                (this + temp_r31_2)->unkEAC = 0;
            }
        }
    }
    return 1;
}

/* daSaku_c::RecreateHeap (int, int) */
s32 RecreateHeap__8daSaku_cFii(daSaku_c *this, s32 arg0, s32 arg1, ? arg_sp0) {
    ? *temp_r3_2;
    ? *temp_r4;
    JKRHeap *temp_r31_2;
    JUTAssertion *temp_r3;
    void *temp_r31;

    temp_r3 = this + (arg1 * 8);
    temp_r31 = temp_r3 + (arg0 * 4);
    if ((JKRHeap *) temp_r31->unkE14 == NULL) {
        temp_r4 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(temp_r3), (u32) temp_r4, (s8 *)0x365, (s32) (temp_r4 + 0xD), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3_2 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        OSPanic(temp_r3_2, 0x365, temp_r3_2 + 0x2B);
    }
    freeAll__7JKRHeapFv(temp_r31->unkE14);
    temp_r31_2 = mDoExt_setCurrentHeap__FP7JKRHeap(temp_r31->unkE14);
    CreateHeap__8daSaku_cFii(this, 1, arg1);
    mDoExt_setCurrentHeap__FP7JKRHeap(temp_r31_2);
    return 1;
}

/* daSaku_c::CreateHeap (int, int) */
s32 CreateHeap__8daSaku_cFii(daSaku_c *this, s32 arg0, s32 arg1, ? arg_sp0) {
    s32 temp_r0;
    s32 temp_r30;
    s32 var_r4;

    var_r4 = arg0;
    temp_r30 = var_r4;
    temp_r0 = (this + (arg1 * 4))->unkEF8;
    switch (temp_r0) {                              /* irregular */
    case 1:
        var_r4 = 0;
        break;
    case 3:
        var_r4 = 2;
        break;
    case 2:
        var_r4 = 1;
        break;
    }
    if (loadModel__8daSaku_cFiii(this, var_r4, temp_r30, arg1) == 0) {
        return 0;
    }
    return loadMoveBG__8daSaku_cFiii(this, GetDzbId__8daSaku_cFi(this, arg1), temp_r30, arg1) != 0;
}

/* daSaku_c::GetDzbId (int) */
s32 GetDzbId__8daSaku_cFi(daSaku_c *this, s32 arg0) {
    s32 temp_r3;

    temp_r3 = (this + (arg0 * 4))->unkEF8;
    if ((arg0 == 1) || ((s32) this->unkEFC == 0)) {
        if (temp_r3 == 1) {
            return 0;
        }
        return 1;
    }
    if ((temp_r3 == 3) || (temp_r3 == 2)) {
        return 3;
    }
    if (isSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, this->unkF04, (s32) (s8) this->unk1E2) != 0) {
        return 4;
    }
    return 2;
}

/* daSaku_c::CreateDummyHeap (int) */
s32 CreateDummyHeap__8daSaku_cFi(daSaku_c *this, s32 arg0) {
    s32 temp_r31;
    s32 var_r4;
    u8 temp_r0;

    var_r4 = arg0;
    temp_r31 = var_r4;
    temp_r0 = this->unkEF2;
    if (temp_r0 == 0) {
        var_r4 = 0;
    } else if (temp_r0 == 1) {
        var_r4 = 1;
    }
    if (loadModel__8daSaku_cFiii(this, var_r4, 1, temp_r31) == 0) {
        return 0;
    }
    return loadMoveBG__8daSaku_cFiii(this, 1, 1, temp_r31) != 0;
}

/* daSaku_c::loadMoveBG (int, int, int) */
s32 loadMoveBG__8daSaku_cFiii(daSaku_c *this, s32 arg0, s32 arg1, s32 arg2, ? arg_sp0) {
    ? sp4;
    dBgW *temp_r3;
    dBgW *var_r4;
    void *temp_r28;
    void *temp_r4;

    temp_r4 = &@4384 - 4;
    M2C_STRUCT_COPY((s32) &sp4 + 4, (s32) temp_r4 + 4, 0x10);
    (&sp4 + 0x10)->unk4 = (s32) (temp_r4 + 0x10)->unk4;
    temp_r3 = __nw__FUl(0xBCU);
    var_r4 = temp_r3;
    if (var_r4 != NULL) {
        var_r4 = __ct__4dBgWFv(temp_r3);
    }
    temp_r28 = this + (arg2 * 8) + (arg1 * 4);
    temp_r28->unkE34 = var_r4;
    if ((dBgW *) temp_r28->unkE34 != NULL) {
        return Set__4cBgWFP6cBgD_tUlPA3_A4_f((cBgW *) temp_r28->unkE34, getRes__14dRes_control_cFPCclP11dRes_info_ci(m_arcname__8daSaku_c, (&sp8[0])[arg0], (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */)), 1U, this + ((arg2 * 0x30) + 0xE4C)) != 1;
    }
    return 0;
}

/* daSaku_c::loadModel (int, int, int) */
s32 loadModel__8daSaku_cFiii(daSaku_c *this, s32 arg0, s32 arg1, s32 arg2, ? arg_sp0) {
    ? sp1C;
    ? sp4;
    ? *temp_r3;
    ? *temp_r4;
    JUTAssertion *var_r31;
    JUTAssertion *var_r3;
    s32 var_r4;
    u8 temp_r0;
    void *temp_r4_2;

    var_r31 = saved_reg_r31;
    var_r4 = arg0;
    M2C_STRUCT_COPY((s32) &sp1C + 4, (s32) (&@4402 - 4) + 4, 0x18);
    var_r3 = &@4403;
    M2C_STRUCT_COPY((s32) &sp4 + 4, (s32) (&@4403 - 4) + 4, 0x18);
    if (arg2 == 1) {
        var_r4 += 3;
    }
    temp_r0 = this->unkEF2;
    if (temp_r0 == 0) {
        var_r3 = getRes__14dRes_control_cFPCclP11dRes_info_ci(m_arcname__8daSaku_c.unk4, (&sp20[0])[var_r4], (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        var_r31 = var_r3;
    } else if (temp_r0 == 1) {
        var_r3 = getRes__14dRes_control_cFPCclP11dRes_info_ci(m_arcname__8daSaku_c.unk8, (&sp8[0])[var_r4], (s32) &g_dComIfG_gameInfo.unk1BFC0, (dRes_info_c *)0x40, M2C_ERROR(/* Read from unset register $r7 */));
        var_r31 = var_r3;
    }
    if (var_r31 == NULL) {
        temp_r4 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(var_r3), (u32) temp_r4, (s8 *)0x43D, (s32) (temp_r4 + 0x30), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        OSPanic(temp_r3, 0x43D, temp_r3 + 0x2B);
    }
    temp_r4_2 = this + (arg2 * 8) + (arg1 * 4);
    temp_r4_2->unkE24 = mDoExt_J3DModel__create__FP12J3DModelDataUlUl((J3DModelData *) var_r31, 0U, 0x11020203U);
    if ((u32) temp_r4_2->unkE24 == 0U) {
        return 0;
    }
    return 1;
}

/* daSaku_c::burn (void) */
s32 burn__8daSaku_cFv(daSaku_c *this) {
    u32 temp_r3;
    u32 temp_r3_2;

    if ((u8) this->unkEF4 == 0) {
        if ((s32) this->unkEF8 == 1) {
            this->unkEF8 = 2;
            RecreateHeap__8daSaku_cFii(this, 1, 0);
            this->unkEE0 = 0x32;
        }
        if ((s32) this->unkEFC == 1) {
            this->unkEFC = 2;
            RecreateHeap__8daSaku_cFii(this, 1, 1);
            this->unkEE4 = 0x32;
        }
        temp_r3 = this->unkE28;
        if (temp_r3 != 0U) {
            this->unk22C = temp_r3 + 0x24;
        } else {
            temp_r3_2 = this->unkE30;
            if (temp_r3_2 != 0U) {
                this->unk22C = temp_r3_2 + 0x24;
            }
        }
        setEffFire__8daSaku_cFi(this, 0);
        this->unkEEC = 0x5A;
        onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, this->unkF00, (s32) (s8) this->unk1E2);
        if ((s32) this->unkEFC != 0) {
            onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, this->unkF04, (s32) (s8) this->unk1E2);
        }
        this->unkEF4 = 1;
    }
    return 1;
}

/* daSaku_c::broken (int) */
s32 broken__8daSaku_cFi(daSaku_c *this, s32 arg0) {
    void *temp_r3;
    void *temp_r3_2;

    setEffBreak__8daSaku_cFi(this, arg0);
    temp_r3 = this + (arg0 * 4);
    temp_r3->unkEF8 = 3;
    temp_r3->unkEE0 = 0;
    if (arg0 == 0) {
        onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, this->unkF00, (s32) (s8) this->unk1E2);
    } else {
        onSwitch__10dSv_info_cFii(&g_dComIfG_gameInfo, this->unkF04, (s32) (s8) this->unk1E2);
    }
    RecreateHeap__8daSaku_cFii(this, 1, arg0);
    if (arg0 == 0) {
        this->unk22C = (this + (arg0 * 8))->unkE28 + 0x24;
    }
    temp_r3_2 = this + (arg0 * 2);
    temp_r3_2->unkEDC = 0;
    temp_r3_2->unkEDD = 0xFF;
    return 1;
}

/* daSaku_c::changeCollision (int) */
s32 changeCollision__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    s32 temp_r0;
    void *temp_r31;

    temp_r31 = this + (arg0 * 4);
    if ((s32) temp_r31->unkEF8 == 0) {
        return 0;
    }
    temp_r0 = temp_r31->unkEE0;
    if (temp_r0 >= 0) {
        if (temp_r0 == 0) {
            Release__4cBgSFP4cBgW(&g_dComIfG_gameInfo.unk12A0, temp_r31->unkE44);
            MoveBGResist__8daSaku_cFii(this, 1, arg0);
        }
        temp_r31->unkEE0 = (s32) (temp_r31->unkEE0 - 1);
    }
    return 1;
}

/* daSaku_c::setMtx (void) */
void setMtx__8daSaku_cFv(daSaku_c *this, ? arg_sp0) {
    s32 var_r28;
    s32 var_r29;
    s32 var_r30;
    s32 var_r30_2;
    void *temp_r28;
    void *temp_r29;

    var_r29 = 0;
    var_r30 = 0;
    do {
        temp_r28 = *(this + (var_r30 + 0xE24));
        if (temp_r28 != NULL) {
            temp_r28->unk18 = (f32) this->unk214;
            temp_r28->unk1C = (f32) this->unk218;
            temp_r28->unk20 = (f32) this->unk21C;
            PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8.unk0, this->unk1FC, this->unk200);
            mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20C, this->unk20E, this->unk210);
            PSMTXCopy(&now__14mDoMtx_stack_c, temp_r28 + 0x24);
        }
        var_r29 += 1;
        var_r30 += 4;
    } while (var_r29 < 2);
    if ((s32) this->unkEFC != 0) {
        var_r28 = 0;
        var_r30_2 = 0;
        do {
            temp_r29 = *(this + (var_r30_2 + 0xE2C));
            if (temp_r29 != NULL) {
                temp_r29->unk18 = (f32) this->unk214;
                temp_r29->unk1C = (f32) this->unk218;
                temp_r29->unk20 = (f32) this->unk21C;
                PSMTXTrans(&now__14mDoMtx_stack_c, (bitwise f32) &@4549, this->unk1F8.unk0, 200.0f + this->unk1FC, this->unk200);
                mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20C, this->unk20E, this->unk210);
                PSMTXCopy(&now__14mDoMtx_stack_c, temp_r29 + 0x24);
            }
            var_r28 += 1;
            var_r30_2 += 4;
        } while (var_r28 < 2);
    }
}

/* daSaku_c::setMoveBGMtx (void) */
void setMoveBGMtx__8daSaku_cFv(daSaku_c *this) {
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8.unk0, this->unk1FC, this->unk200);
    scaleM__14mDoMtx_stack_cFfff(mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20E), this->unk214, this->unk218, this->unk21C);
    PSMTXCopy(&now__14mDoMtx_stack_c, &this->unkE4C);
    if ((s32) this->unkEFC != 0) {
        PSMTXTrans(&now__14mDoMtx_stack_c, (bitwise f32) &@4549, this->unk1F8.unk0, 200.0f + this->unk1FC, this->unk200);
        scaleM__14mDoMtx_stack_cFfff(mDoMtx_YrotM__FPA4_fs((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20E), this->unk214, this->unk218, this->unk21C);
        PSMTXCopy(&now__14mDoMtx_stack_c, &this->unkE7C);
    }
}

/* daSaku_c::checkCol (void) */
void checkCol__8daSaku_cFv(daSaku_c *this, ? arg_sp0) {
    s32 temp_r0;
    s32 var_r26;
    s32 var_r26_2;
    s32 var_r26_3;
    s32 var_r29;
    s32 var_r29_2;
    s32 var_r29_3;
    s32 var_r30;
    s32 var_r30_2;
    s32 var_r30_3;
    void *temp_r27;
    void *temp_r27_2;
    void *temp_r27_3;

    if ((s32) this->unkEEC != 0) {
        var_r26 = 0;
        var_r29 = 0;
        var_r30 = 0;
        do {
            temp_r27 = this + var_r30;
            SetC__8cM3dGCylFRC4cXyz(temp_r27 + 0xB8C, this + (var_r29 + 0xA2C));
            Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo.unk26A4, temp_r27 + 0xA74);
            var_r26 += 1;
            var_r29 += 0xC;
            var_r30 += 0x130;
        } while (var_r26 < 3);
    }
    if ((s32) this->unkEF8 == 1) {
        var_r26_2 = 0;
        var_r29_2 = 0;
        var_r30_2 = 0;
        do {
            temp_r27_2 = this + var_r30_2;
            SetC__8cM3dGCylFRC4cXyz(temp_r27_2 + 0x424, this + (var_r29_2 + 0xA2C));
            Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo.unk26A4, temp_r27_2 + 0x30C);
            var_r26_2 += 1;
            var_r29_2 += 0xC;
            var_r30_2 += 0x130;
        } while (var_r26_2 < 3);
    }
    temp_r0 = this->unkEFC;
    if ((temp_r0 != 0) && (temp_r0 == 1)) {
        var_r26_3 = 0;
        var_r30_3 = 0;
        var_r29_3 = 0;
        do {
            temp_r27_3 = this + var_r29_3;
            SetC__8cM3dGCylFRC4cXyz(temp_r27_3 + 0x7B4, this + (var_r30_3 + 0xA50));
            Set__4cCcSFP8cCcD_Obj(&g_dComIfG_gameInfo.unk26A4, temp_r27_3 + 0x69C);
            var_r26_3 += 1;
            var_r30_3 += 0xC;
            var_r29_3 += 0x130;
        } while (var_r26_3 < 3);
    }
}

/* daSaku_c::setCol (void) */
void setCol__8daSaku_cFv(daSaku_c *this, ? arg_sp0) {
    s32 var_r23;
    s32 var_r23_2;
    s32 var_r29;
    s32 var_r29_2;
    s32 var_r30;
    s32 var_r30_2;
    void *temp_r24;
    void *temp_r24_2;
    void *temp_r4;
    void *temp_r4_2;

    this->unkA2C = 0.0f;
    this->unkA30 = @4267.unk68;
    this->unkA34 = 0.0f;
    this->unkA38 = @4267.unk6C;
    this->unkA3C = @4267.unk68;
    this->unkA40 = 0.0f;
    this->unkA44 = @4267.unk70;
    this->unkA48 = @4267.unk68;
    this->unkA4C = 0.0f;
    PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8.unk0, this->unk1FC, this->unk200);
    mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20C, this->unk20E, this->unk210);
    var_r23 = 0;
    var_r29 = 0;
    var_r30 = 0;
    do {
        temp_r4 = this + (var_r30 + 0xA2C);
        PSMTXMultVec(&now__14mDoMtx_stack_c, temp_r4, temp_r4);
        temp_r24 = this + var_r29;
        Set__8dCcD_CylFRC11dCcD_SrcCyl(temp_r24 + 0x30C, (dCcD_SrcCyl *) &m_cyl_src__8daSaku_c);
        temp_r24->unk350 = &this->unk2D0;
        var_r23 += 1;
        var_r29 += 0x130;
        var_r30 += 0xC;
    } while (var_r23 < 3);
    if ((s32) this->unkEFC != 0) {
        this->unkA50 = 0.0f;
        this->unkA54 = @4267.unk74;
        this->unkA58 = 0.0f;
        this->unkA5C = @4267.unk6C;
        this->unkA60 = @4267.unk74;
        this->unkA64 = 0.0f;
        this->unkA68 = @4267.unk70;
        this->unkA6C = @4267.unk74;
        this->unkA70 = 0.0f;
        PSMTXTrans(&now__14mDoMtx_stack_c, this->unk1F8.unk0, this->unk1FC, this->unk200);
        mDoMtx_ZXYrotM__FPA4_fsss((f32 (*)[4]) &now__14mDoMtx_stack_c, this->unk20C, this->unk20E, this->unk210);
        var_r23_2 = 0;
        var_r30_2 = 0;
        var_r29_2 = 0;
        do {
            temp_r4_2 = this + (var_r29_2 + 0xA50);
            PSMTXMultVec(&now__14mDoMtx_stack_c, temp_r4_2, temp_r4_2);
            temp_r24_2 = this + var_r30_2;
            Set__8dCcD_CylFRC11dCcD_SrcCyl(temp_r24_2 + 0x69C, (dCcD_SrcCyl *) &m_cyl_src__8daSaku_c);
            temp_r24_2->unk6E0 = &this->unk2D0;
            var_r23_2 += 1;
            var_r30_2 += 0x130;
            var_r29_2 += 0xC;
        } while (var_r23_2 < 3);
    }
}

/* daSaku_c::MoveBGResist (int, int) */
s32 MoveBGResist__8daSaku_cFii(daSaku_c *this, s32 arg0, s32 arg1, ? arg_sp0) {
    void *temp_r31;
    void *temp_r4;

    temp_r31 = this + (arg1 * 8) + (arg0 * 4);
    if (Regist__4dBgSFP4cBgWP10fopAc_ac_c((dBgS *) &g_dComIfG_gameInfo.unk12A0, temp_r31->unkE34, (fopAc_ac_c *) this) != 0) {
        return 0;
    }
    temp_r4 = this + (arg1 * 4);
    temp_r4->unkE44 = (cBgW *) temp_r31->unkE34;
    Move__4dBgWFv((dBgW *) temp_r4->unkE44);
    return 1;
}

/* daSaku_c::setEffFire (int) */
s32 setEffFire__8daSaku_cFi(daSaku_c *this, s32 arg0) {
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    s32 sp10;
    s32 spC;
    s32 sp8;

    sp18 = this->unk1F8.unk0;
    sp1C = this->unk1FC;
    sp20 = this->unk200;
    sp8 = -1;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x45CU, (cXyz *) &sp18, &this->unk204, NULL, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    sp8 = -1;
    spC = 0;
    sp10 = 0;
    sp14 = 0;
    set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x245EU, (cXyz *) &sp18, &this->unk204, NULL, m_smoke_alpha__8daSaku_c.unk0, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    this->unkEC0 = 1;
    this->unkEBC = 1;
    seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x6924U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), 1.0f, 1.0f, -1.0f, -1.0f, 0U);
    return 1;
}

/* daSaku_c::setEffBreak (int) */
s32 setEffBreak__8daSaku_cFi(daSaku_c *this, s32 arg0, ? arg_sp0) {
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    f32 sp1C;
    f32 sp18;
    s32 sp14;
    ? *sp10;
    ? *spC;
    s32 sp8;
    ? *temp_r0;
    f32 temp_f1;
    f32 temp_f1_2;
    s32 temp_r28;
    s32 temp_r29;
    s32 temp_r6;
    u8 temp_r0_2;
    void *temp_r30;
    void *temp_r3;
    void *temp_r3_2;
    void *temp_r3_3;
    void *temp_r3_4;
    void *temp_r3_5;
    void *temp_r3_6;
    void *temp_r4;

    sp30 = this->unk1F8.unk0;
    temp_f1 = this->unk1FC;
    sp34 = temp_f1;
    sp38 = this->unk200;
    temp_f1_2 = temp_f1 + @4267.unk70;
    sp34 = temp_f1_2;
    if (arg0 == 1) {
        sp34 = temp_f1_2 + @4267.unk64;
    }
    if ((u8) l_sakuHIO.unkF != 0) {
        sp8 = -1;
        temp_r0 = &this->unk194;
        spC = temp_r0;
        sp10 = temp_r0;
        sp14 = 0;
        set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 0U, 0x45DU, (cXyz *) &sp30, &this->unk204, (cXyz *) &this->unk214, 0xFFU, NULL, M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    }
    temp_r29 = arg0 * 4;
    temp_r30 = this + temp_r29;
    temp_r30->unkEB4 = (f32) ((f32) l_sakuHIO.unk12 / @4267.unk8);
    dust_color__8daSaku_c.unk0 = l_sakuHIO.unk13;
    dust_color__8daSaku_c.unk1 = l_sakuHIO.unk14;
    dust_color__8daSaku_c.unk2 = l_sakuHIO.unk15;
    temp_r6 = arg0 * 0xC;
    temp_r3 = this + temp_r6;
    temp_r3->unkEC4 = sp30;
    temp_r3->unkEC8 = sp34;
    temp_r3->unkECC = sp38;
    sp8 = (s32) this->unk20A;
    spC = NULL;
    sp10 = NULL;
    sp14 = 0;
    temp_r28 = arg0 << 5;
    set__13dPa_control_cFUcUsPC4cXyzPC5csXyzPC4cXyzUcP18dPa_levelEcallBackScPC8_GXColorPC8_GXColorPC4cXyz(g_dComIfG_gameInfo.unk5AC4, 2U, 0x2027U, this + (temp_r6 + 0xEC4), &this->unk204, NULL, l_sakuHIO.unk12, this + (temp_r28 + 0x290), M2C_ERROR(/* Unable to find stack arg 0x0 in block */), M2C_ERROR(/* Unable to find stack arg 0x4 in block */), M2C_ERROR(/* Unable to find stack arg 0x8 in block */), M2C_ERROR(/* Unable to find stack arg 0xc in block */));
    temp_r4 = this + temp_r28;
    temp_r3_2 = temp_r4->unk294;
    if (temp_r3_2 != NULL) {
        temp_r3_2->unk1FF = (s8) (@4267.unk8 * temp_r30->unkEB4);
        temp_r3_3 = temp_r4->unk294;
        temp_r3_3->unk20C = (s32) (temp_r3_3->unk20C | 0x40);
        sp24 = @4267.unk7C;
        sp28 = @4267.unk7C;
        sp2C = @4267.unk7C;
        sp18 = @4267.unk4;
        sp1C = @4267.unk80;
        sp20 = @4267.unk84;
        temp_r3_4 = temp_r4->unk294;
        temp_r3_4->unk1F0 = (f32) @4267.unk88;
        temp_r3_4->unk1F4 = (f32) @4267.unk88;
        temp_r3_4->unk1F8 = (f32) @4267.unk4;
        temp_r3_5 = temp_r4->unk294;
        temp_r3_5->unk1D8 = (f32) @4267.unk7C;
        temp_r3_5->unk1DC = (f32) @4267.unk7C;
        temp_r3_5->unk1E0 = (f32) @4267.unk7C;
        temp_r3_6 = temp_r4->unk294;
        temp_r3_6->unkC = (f32) @4267.unk4;
        temp_r3_6->unk10 = (f32) @4267.unk80;
        temp_r3_6->unk14 = (f32) @4267.unk84;
        temp_r4->unk294->unk38 = (f32) @4267.unk8C;
        temp_r4->unk294->unk60 = 1;
    }
    temp_r0_2 = this->unkEF2;
    if (temp_r0_2 == 0) {
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x6847U, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4267.unk4, @4267.unk4, @4267.unk78, @4267.unk78, 0U);
    } else if (temp_r0_2 == 1) {
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x693FU, &this->unk260, 0U, dComIfGp_getReverb__Fi((s32) (s8) this->unk20A), @4267.unk4, @4267.unk4, @4267.unk78, @4267.unk78, 0U);
    }
    (this + temp_r29)->unkEBC = 1;
    return 1;
}

/* matAlphaAnim (J3DModelData *, char unsigned, bool) */
s32 matAlphaAnim__FP12J3DModelDataUcb(J3DModelData *arg0, u8 arg1, u8 arg2, ? arg_sp0) {
    ? *temp_r3;
    ? *temp_r4;
    u16 var_r31;

    if (arg0 == NULL) {
        temp_r4 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv((JUTAssertion *) arg0), (u32) temp_r4, (s8 *)0x5D1, (s32) (temp_r4 + 0x30), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        OSPanic(temp_r3, 0x5D1, temp_r3 + 0x2B);
    }
    var_r31 = 0;
loop_4:
    if (var_r31 < (u16) arg0->unk5C) {
        changeXluMaterialAlpha__FP11J3DMaterialUcb(*(arg0->unk60 + ((var_r31 * 4) & 0x3FFFC)), arg1, arg2);
        var_r31 += 1;
        goto loop_4;
    }
    return 1;
}

/* changeXluMaterialAlpha (J3DMaterial *, char unsigned, bool) */
void changeXluMaterialAlpha__FP11J3DMaterialUcb(J3DMaterial *arg0, u8 arg1, u8 arg2, ? arg_sp0) {
    ? *temp_r3;
    ? *temp_r4;
    void **temp_r30;
    void **temp_r3_2;
    void *temp_r3_3;

    if (arg0 == NULL) {
        temp_r4 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        showAssert__12JUTAssertionFUlPCciPCc(getSDevice__12JUTAssertionFv(&@2100), (u32) temp_r4, (s8 *)0x5FF, (s32) (temp_r4 + 0x3F), M2C_ERROR(/* Read from unset register $r7 */));
        temp_r3 = "d_a_saku.cpp\0m_heap[saku_id][heap_id] != 0\0Halt\0modelData != 0\0i_material != 0";
        OSPanic(temp_r3, 0x5FF, temp_r3 + 0x2B);
    }
    temp_r30 = arg0->unk34;
    temp_r3_2 = arg0->unk2C;
    (*temp_r3_2)->unk74(temp_r3_2, 3)->unk3 = arg1;
    temp_r3_3 = (*temp_r30)->unk44(temp_r30);
    temp_r3_3->unk0 = (u8) @2100.unk30;
    temp_r3_3->unk1 = (u8) @2100.unk31;
    temp_r3_3->unk2 = (u8) @2100.unk32;
    temp_r3_3->unk3 = (u8) @2100.unk33;
    if (arg2 != 0) {
        *(*temp_r30)->unk50(temp_r30) = ((@2100.unk39 * 2) & 0x1FE) + ((@2100.unk38 * 0x10) + @2100.unk3A);
        return;
    }
    *(*temp_r30)->unk50(temp_r30) = ((@2100.unk35 * 2) & 0x1FE) + ((@2100.unk34 * 0x10) + @2100.unk36);
}

/* daSaku_Create (fopAc_ac_c *) */
void daSaku_Create__FP10fopAc_ac_c(fopAc_ac_c *arg0) {
    _daSaku_create__8daSaku_cFv((daSaku_c *) arg0);
}

/* daSaku_Delete (daSaku_c *) */
s32 daSaku_Delete__FP8daSaku_c(daSaku_c *arg0, ? arg_sp0) {
    JKRSolidHeap *temp_r3_2;
    s32 var_r25;
    s32 var_r25_2;
    s32 var_r25_3;
    s32 var_r26;
    s32 var_r29;
    s32 var_r29_2;
    s32 var_r29_3;
    s32 var_r30;
    void **temp_r3;
    void *temp_r23;
    void *temp_r4;

    if ((s8) l_sakuHIO.unk4 >= 0) {
        deleteChild__16mDoHIO_subRoot_cFSc(&mDoHIO_root + 4, (s8) l_sakuHIO.unk4);
        l_sakuHIO.unk4 = -1U;
    }
    var_r25 = 0;
    var_r29 = 0;
    do {
        temp_r3 = arg0 + (var_r29 + 0x290);
        (*temp_r3)->unk20(temp_r3);
        var_r25 += 1;
        var_r29 += 0x20;
    } while (var_r25 < 2);
    var_r25_2 = 0;
    var_r29_2 = 0;
    do {
        temp_r4 = arg0 + var_r29_2;
        if ((s32) temp_r4->unkEF8 != 0) {
            Release__4cBgSFP4cBgW(&g_dComIfG_gameInfo.unk12A0, temp_r4->unkE44);
        }
        var_r25_2 += 1;
        var_r29_2 += 4;
    } while (var_r25_2 < 2);
    var_r25_3 = 0;
    var_r30 = 0;
    do {
        var_r26 = 0;
        var_r29_3 = 0;
loop_10:
        temp_r23 = arg0 + var_r30 + var_r29_3;
        temp_r3_2 = temp_r23->unkE14;
        if (temp_r3_2 != NULL) {
            mDoExt_destroySolidHeap__FP12JKRSolidHeap(temp_r3_2);
            temp_r23->unkE14 = NULL;
            temp_r23->unkE24 = 0;
        }
        var_r26 += 1;
        var_r29_3 += 4;
        if (var_r26 < 2) {
            goto loop_10;
        }
        var_r25_3 += 1;
        var_r30 += 8;
    } while (var_r25_3 < 2);
    dComIfG_resDelete__FP30request_of_phase_process_classPCc(&arg0->unkE0C, m_arcname__8daSaku_c.unk0);
    if ((u8) arg0->unkEF2 == 0) {
        dComIfG_resDelete__FP30request_of_phase_process_classPCc(&arg0->unkE04, &m_arcname__8daSaku_c.unk4->unk0);
    } else {
        dComIfG_resDelete__FP30request_of_phase_process_classPCc(&arg0->unkE04, &m_arcname__8daSaku_c.unk8->unk0);
    }
    return 1;
}

/* daSaku_IsDelete (daSaku_c *) */
s32 daSaku_IsDelete__FP8daSaku_c(daSaku_c *arg0) {
    return 1;
}

/* daSaku_Draw (daSaku_c *) */
s32 daSaku_Draw__FP8daSaku_c(daSaku_c *arg0) {
    saku_draw_sub__8daSaku_cFi(arg0, 0);
    if ((s32) arg0->unkEFC != 0) {
        saku_draw_sub__8daSaku_cFi(arg0, 1);
    }
    return 1;
}

/* daSaku_Execute (daSaku_c *) */
s32 daSaku_Execute__FP8daSaku_c(daSaku_c *arg0, ? arg_sp0) {
    s32 temp_r0;
    s32 temp_r3;
    s32 temp_r5;
    s32 temp_r6;
    s32 var_ctr;
    s32 var_r29;
    s32 var_r30;
    s32 var_r30_2;
    s32 var_r4;

    var_r4 = 0;
    var_ctr = 2;
    do {
        temp_r6 = var_r4 + 0xEBC;
        temp_r5 = *(arg0 + temp_r6);
        if ((temp_r5 != 0) && (temp_r5 < (s32) m_max_particle_timer__8daSaku_c)) {
            *(arg0 + temp_r6) = temp_r5 + 1;
        }
        var_r4 += 4;
        var_ctr -= 1;
    } while (var_ctr != 0);
    temp_r3 = arg0->unkEEC;
    if (temp_r3 != 0) {
        arg0->unkEEC = temp_r3 - 1;
    }
    arg0->unkEE8 += 1;
    var_r29 = 0;
    var_r30 = 0;
    do {
        temp_r0 = *(arg0 + (var_r30 + 0xEF8));
        switch (temp_r0) {                          /* irregular */
        case 0:
            break;
        case 1:
            mode_break_none__8daSaku_cFi(arg0, var_r29);
            break;
        case 3:
            mode_break_throw_obj__8daSaku_cFi(arg0, var_r29);
            break;
        case 2:
            mode_break_fire__8daSaku_cFi(arg0, var_r29);
            break;
        }
        var_r29 += 1;
        var_r30 += 4;
    } while (var_r29 < 2);
    var_r30_2 = 0;
    do {
        changeCollision__8daSaku_cFi(arg0, var_r30_2);
        var_r30_2 += 1;
    } while (var_r30_2 < 2);
    setMtx__8daSaku_cFv(arg0);
    checkCol__8daSaku_cFv(arg0);
    return 1;
}

/* d_a_saku_cpp::__sinit void (*) (void) */
void __sinit_d_a_saku_cpp(void) {
    l_sakuHIO.unk0 = &__vt__9sakuHIO_c;
    l_sakuHIO.unk4 = -1;
    l_sakuHIO.unk6 = 0x46;
    l_sakuHIO.unk8 = 0x46;
    l_sakuHIO.unkA = 0x41;
    l_sakuHIO.unkC = 7;
    l_sakuHIO.unkE = 1;
    l_sakuHIO.unkF = 1;
    l_sakuHIO.unk10 = 5;
    l_sakuHIO.unk12 = 0xB4;
    l_sakuHIO.unk13 = 0x69;
    l_sakuHIO.unk14 = 0x5B;
    l_sakuHIO.unk15 = 0x30;
    l_sakuHIO.unk16 = 0x64;
    __register_global_object(&l_sakuHIO, __dt__9sakuHIO_cFv, &@4006);
}