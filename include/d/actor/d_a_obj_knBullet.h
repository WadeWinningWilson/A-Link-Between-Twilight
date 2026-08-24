#ifndef D_A_OBJ_KNBULLET_H
#define D_A_OBJ_KNBULLET_H

#include "f_op/f_op_actor_mng.h"
#include "d/d_cc_d.h"

/**
 * @ingroup actors-objects
 * @class daObjKnBullet_c
 * @brief Hero's Shade Energy Ball? (Knight Bullet)
 *
 * @details
 *
 */
class daObjKnBullet_c : public fopAc_ac_c {
public:
    int Create();
    int Execute();
    int Draw();
    int Delete();
    void setBaseMtx();
    void col_init();
    BOOL col_chk();
    void hitPrtclSet();

    u8 getActionMode() { return mActionMode; }
    void setActionMode(u8 i_action) { mActionMode = i_action; }
    // ALBW boss barrage: per-instance overrides so the fast boss balls don't alter
    // the vanilla Shield-Attack lesson balls (default 1.0 = unchanged).
    void setSpeedMul(f32 m) { mSpeedMul = m; }
    void setRadiusMul(f32 m) { mRadiusMul = m; }
    // The lesson ball has mAtp=0 (non-damaging tutorial). The boss ball sets a real
    // attack power here so it deals damage (same mechanism as the Kn sword).
    void setAtp(u8 a) { mCcSph.SetAtAtp(a); }

private:
    /* 0x568 */ Mtx mMtx;
    /* 0x598 */ dCcD_Stts mCcStts;
    /* 0x5D4 */ dCcD_Sph mCcSph;
    /* 0x70C */ u8 mActionMode;
    /* 0x70E */ s16 mTimer;
    /* 0x710 */ u32 mEmtIds[3];
    /* 0x71C */ f32 mSpeedMul;   // ALBW: per-instance move-speed multiplier
    /* 0x720 */ f32 mRadiusMul;  // ALBW: per-instance hitbox-radius multiplier
};

STATIC_ASSERT(sizeof(daObjKnBullet_c) == 0x724);

struct daObjKnBullet_Hio_Param_c {
    /* 0x0 */ f32 radius;
    /* 0x4 */ f32 move_speed;
    /* 0x8 */ s16 lifetime;
};

#endif /* D_A_OBJ_KNBULLET_H */
