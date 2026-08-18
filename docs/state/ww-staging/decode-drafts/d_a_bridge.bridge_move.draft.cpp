// ====================================================
// PORT-GRADE DECOMP DRAFT — d_a_bridge (1 fns)
// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)
//           -> fopAc offset receipts (rel_decomp.py §252)
// asm: build\GZLE01\d_a_bridge\asm\d\actor\d_a_bridge.m2c.s
// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle
// stack (probe differ / state taps), per covenant.
// ====================================================

typedef struct JAIZelBasic {
    /* 0x0 */ JAIZelBasic *unk0;                    /* inferred */  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
} JAIZelBasic;                                      /* size >= 0x4 */

typedef struct br_s {
    /* 0x000 */ char pad0[0x3A0];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x3A0 */ s16 unk3A0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3A2 */ s16 unk3A2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3A4 */ char pad3A4[0x28];                  /* maybe part of unk3A2[0x15]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3CC */ ? unk3CC;                           /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3CC */ char pad3CC[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3D0 */ f32 unk3D0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3D4 */ f32 unk3D4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3D8 */ f32 unk3D8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3DC */ f32 unk3DC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3E0 */ f32 unk3E0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3E4 */ char pad3E4[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3E8 */ s16 unk3E8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3EA */ char pad3EA[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3EC */ f32 unk3EC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3F0 */ char pad3F0[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3F4 */ f32 unk3F4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3F8 */ f32 unk3F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x3FC */ f32 unk3FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x400 */ s16 unk400;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x402 */ s16 unk402;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x404 */ s16 unk404;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x406 */ u8 unk406;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x407 */ char pad407[1];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x408 */ u8 unk408;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} br_s;                                             /* size >= 0x409 */

typedef struct bridge_class {
    /* 0x000 */ char pad0[0x1C4];  /* = fopAc_ac_c::leafdraw_class base @0x0 (receipt f_op_actor.h) */
    /* 0x1C4 */ s32 unk1C4;                         /* inferred */  /* = fopAc_ac_c::u32 actor_status @0x1C4 (receipt f_op_actor.h) */
    /* 0x1C8 */ char pad1C8[8];                     /* maybe part of unk1C4[3]? */  /* = fopAc_ac_c::u32 actor_condition @0x1C8 (receipt f_op_actor.h) */
    /* 0x1D0 */ f32 unk1D0;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 (receipt f_op_actor.h) */
    /* 0x1D4 */ f32 unk1D4;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x4 (receipt f_op_actor.h) */
    /* 0x1D8 */ f32 unk1D8;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x8 (receipt f_op_actor.h) */
    /* 0x1DC */ char pad1DC[2];  /* = fopAc_ac_c::actor_place home @0x1D0 +0xC (receipt f_op_actor.h) */
    /* 0x1DE */ s16 unk1DE;                         /* inferred */  /* = fopAc_ac_c::actor_place home @0x1D0 +0xE (receipt f_op_actor.h) */
    /* 0x1E0 */ char pad1E0[0x18];                  /* maybe part of unk1DE[0xD]? */  /* = fopAc_ac_c::actor_place home @0x1D0 +0x10 (receipt f_op_actor.h) */
    /* 0x1F8 */ f32 unk1F8;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 (receipt f_op_actor.h) */
    /* 0x1FC */ f32 unk1FC;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x4 (receipt f_op_actor.h) */
    /* 0x200 */ f32 unk200;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x8 (receipt f_op_actor.h) */
    /* 0x204 */ s16 unk204;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xC (receipt f_op_actor.h) */
    /* 0x206 */ s16 unk206;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0xE (receipt f_op_actor.h) */
    /* 0x208 */ s16 unk208;                         /* inferred */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x10 (receipt f_op_actor.h) */
    /* 0x20A */ char pad20A[0x8E];                  /* maybe part of unk208[0x48]? */  /* = fopAc_ac_c::actor_place current @0x1F8 +0x12 (receipt f_op_actor.h) */
    /* 0x298 */ s16 unk298;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x29A */ char pad29A[0x3E];                  /* maybe part of unk298[0x20]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D8 */ u8 unk2D8;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2D9 */ s8 unk2D9;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2DA */ char pad2DA[2];                     /* maybe part of unk2D9[3]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2DC */ u8 unk2DC;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2DD */ char pad2DD[3];                     /* maybe part of unk2DC[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2E0 */ f32 unk2E0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2E4 */ f32 unk2E4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2E8 */ char pad2E8[4];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2EC */ s16 unk2EC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2EE */ char pad2EE[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F0 */ s16 unk2F0;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F2 */ s16 unk2F2;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F4 */ f32 unk2F4;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2F8 */ f32 unk2F8;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x2FC */ f32 unk2FC;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x300 */ s16 unk300;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x302 */ char pad302[2];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x304 */ s32 unk304;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x308 */ s32 unk308;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x30C */ char pad30C[6];                     /* maybe part of unk308[2]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x312 */ s16 unk312;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x314 */ f32 unk314;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x318 */ f32 unk318;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x31C */ f32 unk31C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x320 */ char pad320[0x1C];                  /* maybe part of unk31C[8]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x33C */ u8 unk33C;                          /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x33D */ char pad33D[3];                     /* maybe part of unk33C[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x340 */ br_s unk340;                        /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x340 */ char pad340[0x3CC];  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x70C */ f32 unk70C;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x710 */ f32 unk710;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x714 */ f32 unk714;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x718 */ char pad718[0xC];                   /* maybe part of unk714[4]? */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x724 */ s16 unk724;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x726 */ s16 unk726;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
    /* 0x728 */ s16 unk728;                         /* inferred */  /* [INFERENCE-NEEDED: past fopAc base — name via sibling template or debug map] */
} bridge_class;                                     /* size >= 0x72A */

? MtxPosition__FP4cXyzP4cXyz(cXyz *arg0, cXyz *arg1); /* extern */
? PSVECAdd(? *, cXyz *, ? *);                       /* extern */
? PSVECSubtract(void *, cXyz *, void *);            /* extern */
? StartShock__12dVibration_cFii4cXyz(dVibration_c *this, s32 arg0, s32 arg1, cXyz arg2); /* extern */
? __mi__4cXyzCFRC3Vec(cXyz *this, Vec *arg0);       /* extern */
? cLib_addCalc0__FPfff(f32 *arg0, f32 arg1, f32 arg2); /* extern */
? cLib_addCalc2__FPffff(f32 *arg0, f32 arg1, f32 arg2, f32 arg3); /* extern */
? cLib_addCalcAngleS2__FPssss(s16 *arg0, s16 arg1, s16 arg2, s16 arg3); /* extern */
f32 cM_rndFX__Ff(f32 arg0);                         /* extern */
s8 dComIfGp_getReverb__Fi(s32 arg0);                /* extern */
? mDoMtx_YrotS__FPA4_fs(f32 (*arg0)[4], s16 arg1);  /* extern */
? seStart__11JAIZelBasicFUlP3VecUlScffffUc(JAIZelBasic *this, u32 arg0, Vec *arg1, u32 arg2, s8 arg3, f32 arg4, f32 arg5, f32 arg6, f32 arg7, u8 arg8); /* extern */
f64 sin(f32, f32);                                  /* extern */
? control1__FP12bridge_classP4br_s(bridge_class *arg0, br_s *arg1); /* static */
? control2__FP12bridge_classP4br_s(bridge_class *arg0, br_s *arg1); /* static */
? control3__FP12bridge_classP4br_s(bridge_class *arg0, br_s *arg1); /* static */
? cut_control1__FP12bridge_classP4br_s(bridge_class *arg0, br_s *arg1); /* static */
? cut_control2__FP12bridge_classP4br_s(bridge_class *arg0, br_s *arg1); /* static */
? kikuzu_set__FP12bridge_classP4cXyz(bridge_class *arg0, cXyz *arg1); /* static */
extern f32 (*calc_mtx)[4];
extern ? g_dComIfG_gameInfo;
extern ? g_regHIO;
extern s32 jmaCosTable;
extern s32 jmaSinShift;
extern s32 jmaSinTable;
extern JAIZelBasic zel_basic__11JAIZelBasic;
static ? ita_z_p;                                   /* unable to generate initializer: unknown type */
static f32 *wp;

/* bridge_move (bridge_class *) */
void bridge_move__FP12bridge_class(bridge_class *arg0) {
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    cXyz sp2C;
    f32 sp28;
    f32 sp24;
    f32 sp20;
    cXyz sp14;
    f32 sp10;
    f32 spC;
    f32 sp8;
    ? *temp_r3_2;
    br_s *var_r18_3;
    br_s *var_r22;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f0_4;
    f32 temp_f27;
    f32 temp_f27_2;
    f32 temp_f2;
    f32 temp_f3;
    f32 var_f25;
    f32 var_f25_2;
    f32 var_f26;
    s16 temp_r0;
    s16 temp_r3_12;
    s16 temp_r6;
    s32 temp_r0_3;
    s32 temp_r26;
    s32 temp_r3_10;
    s32 temp_r3_11;
    s32 temp_r3_5;
    s32 temp_r5;
    s32 var_ctr;
    s32 var_r18;
    s32 var_r18_2;
    s32 var_r19;
    s32 var_r19_2;
    s32 var_r19_3;
    s32 var_r19_4;
    s32 var_r20;
    s32 var_r20_2;
    s32 var_r23;
    s32 var_r23_2;
    s32 var_r24;
    s32 var_r3;
    s32 var_r4;
    s32 var_r7;
    u8 temp_r0_2;
    u8 temp_r3_7;
    u8 temp_r3_9;
    u8 temp_r4;
    void *temp_r19;
    void *temp_r25;
    void *temp_r25_2;
    void *temp_r3;
    void *temp_r3_13;
    void *temp_r3_3;
    void *temp_r3_4;
    void *temp_r3_6;
    void *temp_r3_8;
    void *temp_r5_2;

    temp_r19 = g_dComIfG_gameInfo.unk5B44;
    var_r22 = &arg0->unk340;
    temp_r0 = arg0->unk298;
    switch (temp_r0) {                              /* irregular */
    case 0:
        arg0->unk2D9 = 0;
        arg0->unk298 = 2;
        arg0->unk1C4 &= 0xFFFFFEFF;
        /* fallthrough */
    case 2:
        var_r18 = 0;
        var_r19 = 0;
loop_15:
        temp_r4 = arg0->unk2DC;
        if (var_r18 < (s8) temp_r4) {
            if ((s32) (arg0->unk2D8 & 1) == 1) {
                *(arg0 + (var_r19 + 0x730)) = @4306.unk24;
            } else {
                *(arg0 + (var_r19 + 0x730)) = (f32) fabs((f32) sin(@4306.unkAC * ((f32) var_r18 / (f32) ((s8) temp_r4 - 1)), @4306.unkAC));
            }
            temp_r3 = arg0 + var_r19;
            temp_r3->unk6E5 = 3;
            temp_r3->unk6E4 = 3;
            var_r18 += 1;
            var_r19 += 0x41C;
            goto loop_15;
        }
        arg0->unk298 = 3;
        /* fallthrough */
    case 3:
        arg0->unk300 += 0xBB8;
        arg0->unk70C = arg0->unk1D0;
        arg0->unk710 = arg0->unk1D4;
        arg0->unk714 = arg0->unk1D8;
        if ((s32) (arg0->unk2D8 & 1) == 1) {
            mDoMtx_YrotS__FPA4_fs(calc_mtx, arg0->unk1DE);
            sp38 = @4306.unkB0 * (arg0->unk2F8 * *(jmaCosTable + (((s32) (u16) arg0->unk2EC >> jmaSinShift) * 4)));
            sp40 = 0.0f;
            sp3C = 0.0f;
            MtxPosition__FP4cXyzP4cXyz((cXyz *) &sp38, &sp2C);
            temp_r3_2 = &var_r22->unk3CC;
            PSVECAdd(temp_r3_2, &sp2C, temp_r3_2);
        }
        control1__FP12bridge_classP4br_s(arg0, var_r22);
        temp_r3_3 = var_r22 + (((s8) arg0->unk2DC * 0x41C) - 0x50);
        temp_r3_3->unk0 = (f32) arg0->unk314;
        temp_r3_3->unk4 = (f32) arg0->unk318;
        temp_r3_3->unk8 = (f32) arg0->unk31C;
        if ((s32) (arg0->unk2D8 & 1) == 1) {
            temp_r3_4 = var_r22 + (((s8) arg0->unk2DC * 0x41C) - 0x50);
            PSVECSubtract(temp_r3_4, &sp2C, temp_r3_4);
        }
        control2__FP12bridge_classP4br_s(arg0, var_r22);
        control3__FP12bridge_classP4br_s(arg0, var_r22);
        __mi__4cXyzCFRC3Vec(&sp14, (Vec *) &arg0->unk1D0);
        temp_f3 = (bitwise f32) sp14;
        sp20 = temp_f3;
        sp24 = sp18;
        sp28 = sp1C;
        arg0->unk1F8 = arg0->unk70C;
        arg0->unk1FC = arg0->unk710;
        arg0->unk200 = arg0->unk714;
        arg0->unk204 = arg0->unk724;
        arg0->unk206 = arg0->unk726;
        arg0->unk208 = arg0->unk728;
        var_r24 = 0;
        temp_f27 = @4306.unkB4;
loop_62:
        if (var_r24 < (s8) arg0->unk2DC) {
            var_r22->unk3D8 = var_r22->unk3CC;
            var_r22->unk3DC = var_r22->unk3D0;
            var_r22->unk3E0 = var_r22->unk3D4;
            temp_r0_2 = arg0->unk2DC;
            temp_f2 = temp_f27 * ((f32) ((s8) temp_r0_2 - var_r24) / (f32) (s8) temp_r0_2);
            var_r22->unk3D8 += (f32) temp_f3 * temp_f2;
            var_r22->unk3DC += (f32) sp18 * temp_f2;
            var_r22->unk3E0 += (f32) sp1C * temp_f2;
            if ((u8) var_r22->unk406 != 0) {
                var_r23 = -5;
                var_r19_2 = -0x148C;
                var_r20 = -0x14;
                do {
                    temp_r3_5 = var_r24 + var_r23;
                    if ((temp_r3_5 >= 0) && (temp_r3_5 < (s8) arg0->unk2DC)) {
                        temp_r3_6 = &ita_z_p + var_r20;
                        temp_r25 = var_r22 + var_r19_2;
                        cLib_addCalcAngleS2__FPssss(temp_r25 + 0x402, (s16) ((f32) var_r22->unk400 * temp_r3_6->unk14 * temp_r25->unk3F0), 4, 0x800);
                        cLib_addCalc2__FPffff(temp_r25 + 0x3F8, var_r22->unk3F4 * temp_r3_6->unk14, @4306.unk24, @4306.unk60);
                    }
                    var_r23 += 1;
                    var_r19_2 += 0x41C;
                    var_r20 += 4;
                } while (var_r23 <= 5);
            }
            temp_r3_7 = var_r22->unk408;
            if (temp_r3_7 & 4) {
                temp_r0_3 = temp_r3_7 & 3;
                if (temp_r0_3 != 3) {
                    var_f26 = 0.0f;
                    var_f25 = @4306.unkB8;
                    if (temp_r0_3 == 1) {
                        var_f26 = @4306.unkBC;
                        var_f25 = @4306.unkC0;
                    } else if (temp_r0_3 == 2) {
                        var_f26 = @4306.unkC4;
                        var_f25 = @4306.unkC0;
                    }
                    var_r23_2 = -5;
                    var_r20_2 = -0x148C;
                    var_r19_3 = -0x14;
loop_35:
                    temp_r26 = var_r24 + var_r23_2;
                    if ((temp_r26 >= 0) && (temp_r26 < (s8) arg0->unk2DC)) {
                        temp_r3_8 = &ita_z_p + var_r19_3;
                        temp_r25_2 = var_r22 + var_r20_2;
                        cLib_addCalcAngleS2__FPssss(temp_r25_2 + 0x404, (s16) (var_f26 * temp_r3_8->unk14 * temp_r25_2->unk3F0), 4, 0x800);
                        cLib_addCalc2__FPffff(temp_r25_2 + 0x3F8, var_f25 * temp_r3_8->unk14, @4306.unk24, @4306.unkC8);
                        if (!(var_r22->unk408 & 3) && !(arg0->unk2D8 & 4) && (var_r23_2 >= -2) && (var_r23_2 <= 2) && ((u8) temp_r25_2->unk406 != 0)) {
                            if ((u8) g_dComIfG_gameInfo.unk529A != 0) {
                                arg0->unk308 = 0;
                            } else {
                                arg0->unk308 += 2;
                            }
                            if ((s32) arg0->unk308 > 0x64) {
                                arg0->unk298 = 4;
                                arg0->unk304 = temp_r26;
                                if ((s8) arg0->unk33C != 0) {
                                    sp8 = 0.0f;
                                    spC = @4306.unk24;
                                    sp10 = 0.0f;
                                    StartShock__12dVibration_cFii4cXyz(&g_dComIfG_gameInfo + 0x59A0, g_regHIO.unk88 + 5, -0x21, (cXyz) &sp8);
                                }
                            } else {
                                goto block_48;
                            }
                        } else {
block_48:
                            if (temp_r25_2->unk3F4 < @4306.unkCC) {
                                arg0->unk298 = 4;
                                arg0->unk304 = temp_r26;
                            } else {
                                goto block_50;
                            }
                        }
                    } else {
block_50:
                        var_r23_2 += 1;
                        var_r20_2 += 0x41C;
                        var_r19_3 += 4;
                        if (var_r23_2 <= 5) {
                            goto loop_35;
                        }
                    }
                }
            }
            if ((var_r22->unk408 & 4) && ((temp_r6 = var_r22->unk3A0, ((temp_r6 == 0) == 0)) || ((s16) var_r22->unk3A2 != 0))) {
                var_r7 = -5;
                var_r3 = -0x148C;
                var_r4 = -0x14;
                var_ctr = 0xB;
                do {
                    temp_r5 = var_r24 + var_r7;
                    if ((temp_r5 >= 0) && (temp_r5 < (s8) arg0->unk2DC)) {
                        temp_r5_2 = var_r22 + var_r3;
                        temp_r5_2->unk404 = (s16) (temp_r5_2->unk404 + (s32) (@4306.unk34 * (f32) (temp_r6 | var_r22->unk3A2) * *(jmaSinTable + (((s32) ((arg0->unk300 * 4) & 0xFFFC) >> jmaSinShift) * 4)) * (&ita_z_p + var_r4)->unk14 * temp_r5_2->unk3F0));
                    }
                    var_r7 += 1;
                    var_r3 += 0x41C;
                    var_r4 += 4;
                    var_ctr -= 1;
                } while (var_ctr != 0);
            }
            temp_r3_9 = var_r22->unk406;
            if (temp_r3_9 != 0) {
                var_r22->unk406 = temp_r3_9 - 1;
            }
            var_r22->unk400 = 0;
            var_r22->unk3E8 = var_r22->unk402 + var_r22->unk404;
            cLib_addCalcAngleS2__FPssss(&var_r22->unk402, 0, 4, 0x400);
            cLib_addCalcAngleS2__FPssss(&var_r22->unk404, 0, 4, 0x400);
            cLib_addCalc2__FPffff(&var_r22->unk3FC, @4306.unkD0, @4306.unk24, @4306.unk98);
            cLib_addCalc0__FPfff(&var_r22->unk3F8, @4306.unk24, @4306.unk98);
            var_r24 += 1;
            var_r22 += 0x41C;
            goto loop_62;
        }
        temp_r3_10 = arg0->unk308;
        if (temp_r3_10 != 0) {
            arg0->unk308 = temp_r3_10 - 1;
        }
        arg0->unk2FC = arg0->unk2E0;
        arg0->unk2F4 = arg0->unk2E0;
        arg0->unk2F8 = arg0->unk2E4;
        arg0->unk2F2 = 0xBB8;
        arg0->unk2F0 = 0x578;
        if (*wp > @4306.unkD8) {
            var_f25_2 = @4306.unkD4;
        } else {
            var_f25_2 = 0.0f;
        }
        cLib_addCalc2__FPffff(&arg0->unk2E0, var_f25_2, @4306.unkD8, @4306.unkD8);
        cLib_addCalc2__FPffff(&arg0->unk2E4, @4306.unk58 * var_f25_2, @4306.unkD8, @4306.unkDC);
        return;
    case 4:
        var_r18_2 = 0;
loop_75:
        if (var_r18_2 < (s8) arg0->unk2DC) {
            var_r22->unk3FC = 0.0f;
            temp_r3_11 = arg0->unk304;
            if ((var_r18_2 == temp_r3_11) || (var_r18_2 == (s32) (temp_r3_11 - 1)) || (var_r18_2 == (s32) (temp_r3_11 + 1))) {
                temp_f0 = var_r22->unk3D8;
                sp38 = temp_f0;
                sp3C = var_r22->unk3DC;
                temp_f0_2 = var_r22->unk3E0;
                sp40 = temp_f0_2;
                sp38 = temp_f0 + cM_rndFX__Ff(@4306.unk44);
                sp40 = temp_f0_2 + cM_rndFX__Ff(@4306.unk44);
                kikuzu_set__FP12bridge_classP4cXyz(arg0, (cXyz *) &sp38);
                temp_f0_3 = var_r22->unk3D8;
                sp38 = temp_f0_3;
                sp3C = var_r22->unk3DC;
                temp_f0_4 = var_r22->unk3E0;
                sp40 = temp_f0_4;
                sp38 = temp_f0_3 + cM_rndFX__Ff(@4306.unk44);
                sp40 = temp_f0_4 + cM_rndFX__Ff(@4306.unk44);
                kikuzu_set__FP12bridge_classP4cXyz(arg0, (cXyz *) &sp38);
            }
            var_r18_2 += 1;
            var_r22 += 0x41C;
            goto loop_75;
        }
        arg0->unk298 = 5;
        arg0->unk312 = 0x32;
        seStart__11JAIZelBasicFUlP3VecUlScffffUc(zel_basic__11JAIZelBasic.unk0, 0x6933U, temp_r19 + 0x260, 0U, dComIfGp_getReverb__Fi((s32) (s8) temp_r19->unk20A), @4306.unk24, @4306.unk24, @4306.unk84, @4306.unk84, 0U);
        /* fallthrough */
    case 5:
        var_r18_3 = &arg0->unk340;
        temp_r3_12 = arg0->unk312;
        if (temp_r3_12 != 0) {
            arg0->unk312 = temp_r3_12 - 1;
        }
        arg0->unk300 += 0xFA0;
        arg0->unk70C = arg0->unk1D0;
        arg0->unk710 = arg0->unk1D4;
        arg0->unk714 = arg0->unk1D8;
        cut_control1__FP12bridge_classP4br_s(arg0, var_r18_3);
        temp_r3_13 = var_r18_3 + (((s8) arg0->unk2DC * 0x41C) - 0x50);
        temp_r3_13->unk0 = (f32) arg0->unk314;
        temp_r3_13->unk4 = (f32) arg0->unk318;
        temp_r3_13->unk8 = (f32) arg0->unk31C;
        cut_control2__FP12bridge_classP4br_s(arg0, var_r18_3);
        var_r19_4 = 0;
        temp_f27_2 = @4306.unkE4;
loop_81:
        if (var_r19_4 < (s8) arg0->unk2DC) {
            var_r18_3->unk3D8 = var_r18_3->unk3CC;
            var_r18_3->unk3DC = var_r18_3->unk3D0;
            var_r18_3->unk3E0 = var_r18_3->unk3D4;
            cLib_addCalc2__FPffff(&var_r18_3->unk3FC, @4306.unkE0, @4306.unk24, @4306.unk98);
            var_r18_3->unk3EC = temp_f27_2;
            var_r19_4 += 1;
            var_r18_3 += 0x41C;
            goto loop_81;
        }
        /* fallthrough */
        return;
    }
}