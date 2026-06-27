/**

 * d_albw_flurry_rush.h — ALBW Flurry Rush (perfect-dodge slow-mo + Back Slice aerial bow).

 * Phase 1–2: state skeleton, sim scale, lock, ALBW suppress.

 * Design: docs/albw-flurry-rush-brief.md

 */



#pragma once



#if TARGET_PC



#include "f_op/f_op_actor.h"



enum dFlurryRushSwordProfile {

    dFlurryRushProfile_Wood = 0,

    dFlurryRushProfile_Ordon,

    dFlurryRushProfile_Master,

    dFlurryRushProfile_Light,

    dFlurryRushProfile_Unknown,

};



enum dFlurryRushMode {

    dFlurryRushMode_None = 0,

    dFlurryRushMode_Melee,

    dFlurryRushMode_AerialBow,

};



enum dFlurryRushEndReason {

    dFlurryRushEnd_Debug = 0,

    dFlurryRushEnd_StartGateExpired,

    dFlurryRushEnd_HitCap,

    dFlurryRushEnd_TargetLost,

    dFlurryRushEnd_Interrupt,

    dFlurryRushEnd_EquipChange,

};

enum dFlurryPerfectDodgeKind {

    dFlurryPerfectDodge_SideStep = 0,

    dFlurryPerfectDodge_BackJump,

};

enum dFlurryMeleeTelegraphAxis {

    dFlurryTelegraph_None = 0,

    dFlurryTelegraph_Vertical,    // sidestep

    dFlurryTelegraph_Horizontal,  // backflip

};



struct dFlurryRushProfile {

    int spendGate;

    int barCost;

    int maxHits;

};



bool dFlurryRush_isEnabled();

// Phase 5 — FA spend gate for perfect-dodge entry (wood always true).
bool dFlurryRush_canOfferPerfectDodgeSpend();

dFlurryRushProfile dFlurryRush_getProfile(dFlurryRushSwordProfile i_profile);

dFlurryRushSwordProfile dFlurryRush_getEquippedSwordProfile();



bool dFlurryRush_beginMelee(fopAc_ac_c* i_target);

bool dFlurryRush_beginAerialBow(fopAc_ac_c* i_target);

void dFlurryRush_update();

void dFlurryRush_end(dFlurryRushEndReason i_reason);

// Ends active rush when sword equip changes (D-pad quick swap, etc.).
void dFlurryRush_cancelOnSwordEquipChange();

// Phase 3 — perfect dodge vs Bokoblin melee telegraph.

bool dFlurryRush_tryPerfectDodge(dFlurryPerfectDodgeKind i_kind);

bool dFlurryRush_hasPendingPerfectDodge();

bool dFlurryRush_tryEnterProcFromPerfectDodge();

dFlurryPerfectDodgeKind dFlurryRush_getPendingDodgeKind();

dFlurryMeleeTelegraphAxis dFlurryRush_queryLockTargetTelegraph();

const char* dFlurryRush_getTelegraphLabel(dFlurryMeleeTelegraphAxis i_axis);

const char* dFlurryRush_getDodgeKindLabel(dFlurryPerfectDodgeKind i_kind);

const char* dFlurryRush_getLastDodgeAttemptText();

// Phase 4 — procFlurryRush chain gates (Link anim frames at 1.0x).

f32 dFlurryRush_getChainGateWidthFrames();

void dFlurryRush_onChainGateMissed();



bool dFlurryRush_isActive();

fopAc_ac_c* dFlurryRush_getTargetActor();

bool dFlurryRush_isTargetActor(fopAc_ac_c* i_actor);

dFlurryRushMode dFlurryRush_getMode();

f32 dFlurryRush_getTimeScale();

bool dFlurryRush_shouldSuppressAlbwSpend();



void dFlurryRush_onMeleeProcEntered();

// Arms the 2.0s start gate once Link finishes snap-to-target (attack window begins).
void dFlurryRush_onSnapToTargetComplete();

void dFlurryRush_onAttackStarted();

void dFlurryRush_onHitLanded();



// Debug / overlay (Focused Arts With Debug overlay).

bool dFlurryRush_debugBeginMeleeOnLockTarget();

float dFlurryRush_getStartGateRemainingSeconds();

int dFlurryRush_getHitCount();

bool dFlurryRush_hasStartedAttack();

const char* dFlurryRush_getLastEventText();

const char* dFlurryRush_getProfileLabel(dFlurryRushSwordProfile i_profile);

const char* dFlurryRush_getModeLabel(dFlurryRushMode i_mode);



#endif // TARGET_PC

