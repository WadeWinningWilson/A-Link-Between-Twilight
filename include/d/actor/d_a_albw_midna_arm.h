#ifndef D_A_ALBW_MIDNA_ARM_H
#define D_A_ALBW_MIDNA_ARM_H

// ============================================
// NEW CODE — ALBW Port — "Midna's Grasp" (Wolf Art 2, D-pad Right)
// d_a_albw_midna_arm.h
// An auto-attacking helper actor: for ~15 seconds Midna's hair-hand repeatedly
// stretches out to the current Z-lock target, strikes it as a SWORD hit, and
// retracts.  Runs fully in parallel with wolf Link (its own actor + its own AT
// collider — never touches Link's proc or Link's mAtCyl), so the wolf keeps
// fighting.  Because the collider's owner is NOT ALINK, its hits get the generic
// mHitType and can never feed the wolf charge counter.
// Spawned by daAlink_c::handleWolfArmBurst(); at most one alive at a time.
// ============================================

#if TARGET_PC

#include "f_op/f_op_actor_mng.h"
#include "d/d_cc_d.h"

class daAlbwMidnaArm_c : public fopAc_ac_c {
public:
    enum State {
        STATE_IDLE_e,     // no target: hair hovers in the READY stance above Midna, waiting
        STATE_STRETCH_e,  // hair extending toward the target (striking: 3x reach)
        STATE_HIT_e,      // strike window: AT collider armed at the target (striking)
        STATE_RETRACT_e,  // hair gliding back (guided, normal scale)
        STATE_PAUSE_e,    // brief rest between punches (READY hover)
    };

    int create();
    int Delete();
    int Execute();
    int Draw();

    // True while an arm actor is alive (one-at-a-time gate for the trigger).
    static bool isAlive();

private:
    fopAc_ac_c* getLockTarget();  // current Z-lock enemy, validated (NULL if none)
    void        enterState(State i_state);

    dCcD_Stts mStts;   // collider status — Init'd with THIS actor as owner (not ALINK)
    dCcD_Cyl  mAtCyl;  // the sword-typed strike collider, placed at the target

    State mState;
    int   mStateTimer;   // frames left in the current state
    int   mLifeFrames;   // total frames left before the art expires
    cXyz  mReachFrom;    // reach interpolation start (Midna/wolf position)
    cXyz  mReachTarget;  // strike point captured at stretch start
};

#endif  // TARGET_PC
#endif  // D_A_ALBW_MIDNA_ARM_H
