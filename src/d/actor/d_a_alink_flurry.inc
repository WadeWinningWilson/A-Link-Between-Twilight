/**
 * d_a_alink_flurry.inc
 * PC-only Flurry Rush melee proc (perfect-dodge slow-mo chain).
 */

#if TARGET_PC

#include "d/d_albw_flurry_rush.h"

static constexpr int kFlurryPhase_SnapToTarget = 0;
static constexpr int kFlurryPhase_WaitFirstSwing = 1;
static constexpr int kFlurryPhase_Swinging = 2;
static constexpr int kFlurryPhase_RecoveryGate = 3;

// ALBW-style lunge: fixed forward speed toward lock target until melee engage range.
static constexpr f32 kFlurrySnapSpeed = 85.0f;
static constexpr f32 kFlurrySnapGoalEpsilon = 8.0f;
static constexpr f32 kFlurrySnapEngagePadding = 50.0f;

static constexpr int kFlurryCutVertical = 0;
static constexpr int kFlurryCutLeft = 1;
static constexpr int kFlurryCutRight = 2;

static int flurryCutParamForSwing(int i_swingIndex) {
    switch (i_swingIndex % 3) {
    case kFlurryCutLeft:
        return CUT_NM_PARAM_LEFT;
    case kFlurryCutRight:
        return CUT_NM_PARAM_RIGHT;
    default:
        return CUT_NM_PARAM_VERTICAL;
    }
}

void daAlink_c::flurryReserveChainInput() {
    if (swordSwingTrigger() && !manualShieldBlocksSwordInput()) {
        onNoResetFlg2(FLG2_COMBO_RESERB);
    }
}

bool daAlink_c::flurryConsumeChainInput() {
    if (manualShieldBlocksSwordInput()) {
        return false;
    }

    if (swordSwingTrigger() || checkComboReserb()) {
        offNoResetFlg2(FLG2_COMBO_RESERB);
        return true;
    }

    return false;
}

void daAlink_c::flurryBeginSwing(int i_swingIndex) {
    static daAlink_cutParamTbl const cutParamTable[3] = {
        {
            ANM_CUT_NM_VERTICAL,
            ANM_CUT_RECOIL_A,
            CUT_TYPE_NM_VERTICAL,
            dCcD_SE_SWORD,
            0,
            0,
            6.0f,
        },
        {
            ANM_CUT_NM_LEFT,
            ANM_CUT_RECOIL_A,
            CUT_TYPE_NM_LEFT,
            dCcD_SE_SWORD,
            0,
            0,
            5.0f,
        },
        {
            ANM_CUT_NM_RIGHT,
            ANM_CUT_RECOIL_A,
            CUT_TYPE_NM_RIGHT,
            dCcD_SE_SWORD,
            0,
            0,
            5.0f,
        },
    };

    const int cutParam = flurryCutParamForSwing(i_swingIndex);
    const daAlink_cutParamTbl* cutParams = &cutParamTable[cutParam];

    setCutType(cutParams->m_cutType);
    field_0x3198 = cutParams->m_recoilAnmID;

    const daAlinkHIO_cutNormal_c1* cutData;
    if (cutParam == CUT_NM_PARAM_LEFT) {
        cutData = &mpHIO->mCut.mCutLeft.m;
    } else if (cutParam == CUT_NM_PARAM_RIGHT) {
        cutData = &mpHIO->mCut.mCutRight.m;
    } else {
        cutData = &mpHIO->mCut.mCutVertical.m;
    }

    field_0x3478 = cutData->mAttackStartFrame;
    field_0x347c = cutData->mAttackEndFrame;
    field_0x3484 = cutData->mCutAnm.mCancelFrame;
    // field_0x3480 = chain gate close (cancel + width) for this proc only.
    field_0x3480 = field_0x3484 + dFlurryRush_getChainGateWidthFrames();

    f32 morf = cutData->mCutAnm.mInterpolation;
    setSingleAnime(cutParams->m_anmID, cutData->mCutAnm.mSpeed, cutData->mCutAnm.mStartFrame,
                   cutData->mCutAnm.mEndFrame, morf);
    current.angle.y = shape_angle.y;
    mProcVar2.field_0x300c = shape_angle.y;

    setSwordAtParam(dCcG_At_Spl_UNK_0, 1, cutParams->m_atSe, 2, mpHIO->mCut.m.mSwordLength,
                    mpHIO->mCut.m.mSwordRadius);
    setSwordComboVoice();

    field_0x3588 = l_halfAtnWaitBaseAnime;
    field_0x2f99 = 4;
    mProcVar1.field_0x300a = kFlurryPhase_Swinging;
    mProcVar3.field_0x300e = 0;
    mProcVar4.field_0x3010 = 0;
    mProcVar5.mCutTurnChargeCheckTimer = 0;
    offNoResetFlg2(FLG2_COMBO_RESERB);

    dComIfGp_setPlayerStatus0(0, 0x8000);
    setCutWaterDropEffect();
    initCutAtnActorSearch();

    if (i_swingIndex == 0) {
        dFlurryRush_onAttackStarted();
    }
}

bool daAlink_c::flurryCheckSwordHit() {
    for (int i = 0; i < 3; i++) {
        if (mAtCps[i].ChkAtHit() && checkEnemyGroup(mAtCps[i].GetAtHitAc())) {
            return true;
        }
    }

    // Slow-mo desyncs enemy hurt spheres (0.1x anim) from Link sword (1.0x). For the
    // pinned rush target, accept a Z-lock range check during the attack window.
    fopAc_ac_c* target = dFlurryRush_getTargetActor();
    if (target == nullptr || !checkEnemyGroup(target)) {
        return false;
    }

    daPy_frameCtrl_c* frameCtrl = mUnderFrameCtrl;
    if (frameCtrl->getFrame() < field_0x3478 || frameCtrl->getFrame() >= field_0x347c + 2.0f) {
        return false;
    }

    const f32 reach = mpHIO->mCut.m.mSwordLength + mpHIO->mCut.m.mSwordRadius + 80.0f;
    return current.pos.abs2XZ(target->current.pos) <= reach * reach;
}

int daAlink_c::flurryExitToWait(int i_reason) {
    offNoResetFlg2(FLG2_COMBO_RESERB);
    if (dFlurryRush_isActive()) {
        dFlurryRush_end(static_cast<dFlurryRushEndReason>(i_reason));
    }
    return procWaitInit();
}

bool daAlink_c::flurryTryChainSwing(int i_nextSwingIndex) {
    if (!flurryConsumeChainInput()) {
        return false;
    }

    mProcVar5.field_0x3012 = i_nextSwingIndex;
    flurryBeginSwing(i_nextSwingIndex);
    return true;
}

static f32 flurryEngageDistanceFrom(const daAlinkHIO_c* i_hio) {
    return i_hio->mCut.m.mSwordLength + i_hio->mCut.m.mSwordRadius + kFlurrySnapEngagePadding;
}

f32 daAlink_c::flurryEngageDistance() const {
    return flurryEngageDistanceFrom(mpHIO);
}

bool daAlink_c::flurryIsWithinEngageRange(fopAc_ac_c* i_target) const {
    if (i_target == nullptr) {
        return false;
    }

    const f32 engageDist = flurryEngageDistance();
    return current.pos.abs2XZ(i_target->current.pos) <= engageDist * engageDist;
}

void daAlink_c::flurryEnterWaitFirstSwing() {
    mNormalSpeed = 0.0f;
    speedF = 0.0f;
    mProcVar1.field_0x300a = kFlurryPhase_WaitFirstSwing;
    setShapeAngleToAtnActor(0);
    setBlendMoveAnime(-1.0f);
    dFlurryRush_onSnapToTargetComplete();
}

bool daAlink_c::flurryUpdateSnapToTarget() {
    fopAc_ac_c* target = dFlurryRush_getTargetActor();
    if (target == nullptr) {
        return false;
    }

    if (flurryIsWithinEngageRange(target)) {
        flurryEnterWaitFirstSwing();
        return true;
    }

    const f32 engageDist = flurryEngageDistance();

    cXyz delta = current.pos - target->current.pos;
    delta.y = 0.0f;
    const f32 distToTarget = delta.abs();

    cXyz offset = delta;
    if (distToTarget > 0.001f) {
        offset.normalizeZP();
        offset = offset * engageDist;
    } else {
        offset = cXyz::Zero;
    }

    const cXyz goal = target->current.pos + offset;
    cXyz step = goal - current.pos;
    step.y = 0.0f;

    if (step.absXZ() <= kFlurrySnapGoalEpsilon) {
        flurryEnterWaitFirstSwing();
        return true;
    }

    const s16 angleY = step.atan2sX_Z();
    shape_angle.y = angleY;
    current.angle.y = angleY;
    mNormalSpeed = kFlurrySnapSpeed;
    mMaxSpeed = kFlurrySnapSpeed;
    setBlendMoveAnime(-1.0f);
    return true;
}

int daAlink_c::procFlurryRushInit() {
    if (!dFlurryRush_isEnabled() || !dFlurryRush_isActive() ||
        dFlurryRush_getMode() != dFlurryRushMode_Melee)
    {
        return 0;
    }

    commonProcInit(PROC_FLURRY_RUSH);
    dFlurryRush_onMeleeProcEntered();
    resetCombo(TRUE);
    offNoResetFlg2(FLG2_COMBO_RESERB);
    mNormalSpeed = 0.0f;
    mProcVar5.field_0x3012 = 0;
    mProcVar3.field_0x300e = 0;
    mProcVar4.field_0x3010 = 0;
    field_0x307e = 0;
    setBlendMoveAnime(-1.0f);
    onNoResetFlg1(FLG1_UNK_10000000);
    dComIfGp_setPlayerStatus0(0, 0x8000);

    fopAc_ac_c* target = dFlurryRush_getTargetActor();
    if (target != nullptr && flurryIsWithinEngageRange(target)) {
        flurryEnterWaitFirstSwing();
    } else {
        mProcVar1.field_0x300a = kFlurryPhase_SnapToTarget;
        setShapeAngleToAtnActor(0);
    }

    if (mProcVar1.field_0x300a == kFlurryPhase_WaitFirstSwing &&
        (flurryConsumeChainInput() || itemButtonCheck(BTN_B)))
    {
        flurryBeginSwing(0);
    }

    return 1;
}

int daAlink_c::procFlurryRush() {
    if (!dFlurryRush_isActive()) {
        return flurryExitToWait(dFlurryRushEnd_Interrupt);
    }

    if (checkGroundSpecialMode()) {
        return 1;
    }

    onNoResetFlg1(FLG1_UNK_10000000);
    onEndResetFlg0(ERFLG0_UNK_8000000);
    field_0x2f99 = 4;

    daPy_frameCtrl_c* frameCtrl = mUnderFrameCtrl;
    const int swingIndex = mProcVar5.field_0x3012;
    const int phase = mProcVar1.field_0x300a;

    if (phase == kFlurryPhase_SnapToTarget) {
        if (!flurryUpdateSnapToTarget()) {
            return flurryExitToWait(dFlurryRushEnd_TargetLost);
        }

        flurryReserveChainInput();
        if (mProcVar1.field_0x300a == kFlurryPhase_WaitFirstSwing &&
            (flurryConsumeChainInput() || itemButtonCheck(BTN_B)))
        {
            flurryBeginSwing(0);
        }
        return 1;
    }

    if (phase == kFlurryPhase_WaitFirstSwing) {
        // Move at Link speed during the opening window; checkNextAction would leave
        // PROC_FLURRY_RUSH and end the rush via commonProcInit.
        if (mTargetedActor != NULL || checkAttentionLock()) {
            setSpeedAndAngleAtn();
        } else {
            setSpeedAndAngleNormal();
        }
        setFrontWallType();
        if (checkAttentionState()) {
            if (mTargetedActor != NULL) {
                mMaxSpeed = mpHIO->mAtnMove.m.mMaxSpeed;
            } else {
                mMaxSpeed = mpHIO->mNoActAtnMove.m.mMaxSpeed;
            }
        } else {
            mMaxSpeed = mpHIO->mMove.m.mMaxSpeed;
        }
        if (!checkFrontWallTypeAction()) {
            setBlendMoveAnime(-1.0f);
        }

        flurryReserveChainInput();
        if (flurryConsumeChainInput() || itemButtonCheck(BTN_B)) {
            flurryBeginSwing(0);
        }
        return 1;
    }

    setShapeAngleToAtnActor(0);
    cLib_chaseF(&mNormalSpeed, 0.0f, mpHIO->mMove.m.mDeceleration);

    if (phase == kFlurryPhase_Swinging) {
        flurryReserveChainInput();
        checkCutAtnActorChange();

        if (setShapeAngleToAtnActor(mProcVar4.field_0x3010)) {
            mProcVar2.field_0x300c = shape_angle.y;
            if (cLib_distanceAngleS(getShapeAngleYAtnActor(), shape_angle.y) < 0x800) {
                mProcVar4.field_0x3010 = 1;
            }
        } else {
            cLib_addCalcAngleS(&shape_angle.y, mProcVar2.field_0x300c, 2, 0x2000, 0x800);
        }
        current.angle.y = shape_angle.y;

        if (frameCtrl->getFrame() >= field_0x3478 && frameCtrl->getFrame() < field_0x347c) {
            if (!checkNoResetFlg0(FLG0_CUT_AT_FLG)) {
                seStartSwordCut(Z2SE_AL_SWORD_SWING_S);
            }
            onResetFlg0(RFLG0_UNK_2);
        }

        if (flurryCheckSwordHit() && mProcVar3.field_0x300e == 0) {
            dFlurryRush_onHitLanded();
            mProcVar3.field_0x300e = 1;
            for (int i = 0; i < 3; i++) {
                mAtCps[i].ResetAtHit();
            }
        }

        // Chain gate opens at attack end (not cancel) so mash registers during recovery.
        if (frameCtrl->getFrame() >= field_0x347c) {
            mProcVar1.field_0x300a = kFlurryPhase_RecoveryGate;
            mProcVar4.field_0x3010 = 0;
            if (flurryTryChainSwing(swingIndex + 1)) {
                return 1;
            }
        }
        return 1;
    }

    // Recovery intersect gate: [attack end, cancel + width) at Link 1.0x anim pace.
    const f32 gateOpen = field_0x347c;
    const f32 gateClose = field_0x3480;
    const f32 frame = frameCtrl->getFrame();

    flurryReserveChainInput();

    if (frame >= gateClose) {
        dFlurryRush_onChainGateMissed();
        return flurryExitToWait(dFlurryRushEnd_Interrupt);
    }

    if (frame >= gateOpen && frame < gateClose) {
        flurryTryChainSwing(swingIndex + 1);
    }

    if (checkAnmEnd(frameCtrl)) {
        dFlurryRush_onChainGateMissed();
        return flurryExitToWait(dFlurryRushEnd_Interrupt);
    }

    return 1;
}

#endif
