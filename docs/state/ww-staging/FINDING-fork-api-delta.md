# FINDING - the fork-only API surface a TYPED plugin cannot see (2026-08-18)

era: era-independent
<!-- era rationale: Phase-5 scoping measurement | Housing/Engine, 2026-08-18 -->

## Why this exists

A typed (header-ful) plugin compiles against the **vanilla** headers, so it sees
**vanilla's** API surface. Every fork-added declaration is invisible to it. That cost
was found by accident: a Phase-5 probe TU called `dStage_roomControl_c::getBgW`,
which exists in the fork (`include/d/d_stage.h:1226`) and **not** on vanilla, and the
compiler refused.

**The trade this prices:** inlining blindness returns silence indistinguishable from
*"the code never ran"* - five hypotheses died on it in one night. API divergence fails
**at compile time**. Not a free trade, a strictly better one.

## Denominator and filter, stated as code

- **Population:** headers present in **BOTH** trees under `include/`, extensions
  `.h`/`.hpp`. Fork = `%USERPROFILE%/Documents/dusklight`, vanilla =
  `%USERPROFILE%/Documents/dusklight-main`.
- **Predicate:** `re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")` over each file with
  block and line comments stripped, minus a keyword NOISE set; per-file **name-set**
  difference `fork - vanilla`.
- **Numerator is a subset of the denominator** (both sides are the same shared list).

## The control is the point

A previous sweep for this returned **zero** fork-only names, and its author correctly
refused to publish the zero. So this tool **refuses to report unless it first re-finds
the seed case** (`d/d_stage.h::getBgW`) and exits nonzero if it cannot.

> **A sweep for divergence that cannot detect a KNOWN divergence is not evidence of
> convergence.**

## Limits, stated rather than discovered later

- The predicate is a **NAME-SET diff, not a parse**. It is a **scoping** instrument:
  it sizes the work. It does **not** prove a name is absent for the right reason, and
  it will include macro invocations that look like calls.
- It compares **declarations by name only** - a signature that changed while keeping
  its name is INVISIBLE here. That is a second divergence class this does not measure.
- Vanilla-only names (the reverse direction) are not reported.

## Re-derive

```
python tools/foundry/fork_api_delta.py            # full listing
python tools/foundry/fork_api_delta.py --selftest # control + counts only
```

## Result

```
CONTROL  d/d_stage.h::getBgW fork-only ... FOUND (sweep is live)
shared headers compared : 1139
headers with fork-only  : 45
fork-only names (total) : 324

Z2AudioLib/Z2SeqMgr.h  (1)
    isWolfHowlSong
d/actor/d_a_alink.h  (37)
    albwAbortStuckClothesChange
    checkDekuLeafGlide
    checkGuardSlipMode
    checkMetamorphoseProcActive
    checkRealtimeUse
    checkWwBowSkinActive
    dAlbwAlink_abortStuckClothesChange
    dAlbwAlink_invalidateClothesEpoch
    dAlbwAlink_nativeCapResolved
    dAlbwAlink_requestClothesRemount
    dAlbwAlink_resyncClothesEpoch
    endRealtimeUse
    flurryBeginSwing
    flurryCheckSwordHit
    flurryConsumeChainInput
    flurryEngageDistance
    flurryEnterWaitFirstSwing
    flurryExitToWait
    flurryIsWithinEngageRange
    flurryReserveChainInput
    flurryTryChainSwing
    flurryUpdateSnapToTarget
    handleWolfArmBurst
    handleWolfHowlBurst
    manualShieldAttackTrigger
    manualShieldBlocksSwordInput
    manualShieldButton
    procCutGsHurricane
    procCutGsHurricaneInit
    procCutGsHurricaneTired
    procCutGsHurricaneTiredInit
    procFlurryRush
    procFlurryRushInit
    procWolfHowlCombat
    setWolfHowlSpinEffect
    startRealtimeUse
    updateDekuLeafModel
d/actor/d_a_b_bh.h  (1)
    daB_BH_getPtr
d/actor/d_a_b_tn.h  (19)
    albwApplyPendingPhase1GuardBreak
    albwApplyPhase1BashGuardBreak
    albwApplyPhase2BashGuardBreak
    albwArmorBroken
    albwArmorRemaining
    albwArmorTotal
    albwBeginGuardOpenWindow
    albwDebugLogEvent
    albwFinishBashGuardBreakFromHit
    albwHandleParryCombatBashShieldHit
    albwInternalDamageMax
    albwInternalDamageTaken
    albwIsArmoredPhase
    albwIsPhase1AttackVulnerable
    albwIsPhase1BashTargetState
    albwIsTransitionPhase
    albwIsUnarmoredPhase
    albwTryApplyBashGuardBreakFromHit
    albwUsesInternalDamageMeter
d/actor/d_a_demo_item.h  (2)
    DrawBase
    wwBowDrawModel
d/actor/d_a_e_gi.h  (2)
    albwCryTimerActive
    daE_GI_isScreamOwner
d/actor/d_a_e_gm.h  (1)
    albwGetBossHitRemaining
d/actor/d_a_e_kk.h  (1)
    albwIsFreezeUnsafeState
d/actor/d_a_e_nz.h  (1)
    dE_NZ_isRatStuckOnPlayer
d/actor/d_a_e_oc.h  (4)
    executeConfuse
    getStunBridgeSound
    queryFlurryMeleeTelegraph
    refreshStunHurtColliders
d/actor/d_a_e_sh.h  (2)
    e_sh_refreshStunHurtColliders
    getStunBridgeSound
d/actor/d_a_movie_player.h  (1)
    MoviePlayerShutdown
d/actor/d_a_npc_kn.h  (1)
    heroShadeCombat
d/actor/d_a_obj_knBullet.h  (4)
    SetAtAtp
    setAtp
    setRadiusMul
    setSpeedMul
d/actor/d_a_obj_twGate.h  (1)
    daObjTwGate_albwProvinceForType
d/actor/d_a_tag_TWgate.h  (1)
    daTagTWGate_albwProvinceForType
d/d_attention.h  (1)
    tryStickCycleBattleLockon
d/d_bg_s_acch.h  (1)
    GetFlags
d/d_bg_w_base.h  (1)
    GetPriority
d/d_camera.h  (4)
    dCamera_engineTblCount
    dCamera_setExtraEngineHook
    dCamera_setSelectHook
    resetEditorFlyCamMouseLook
d/d_com_inf_game.h  (12)
    dComIfGp_setMesgStatus
    dEvtFork_guardReport
    dExtWwSave_refuseNativeWrite
    evt1_cutEnd
    evt1_getIsAddvance
    evt1_getMyActIdx
    evt1_isActive
    getBase
    getHeaderP
    getSettings
    getStaffP
    setMesgStatus
d/d_demo.h  (5)
    getP_BrkData
    getP_BtkData
    getP_BtpData
    getTexAnmFrame
    getTexAnmId
d/d_event_manager.h  (4)
    lateStageListInit
    probeEvId
    probeEvType
    probeSlot
d/d_file_select.h  (12)
    applyTrueTestNewSavePreset
    driveCanLoadSlot
    driveLoadSlot
    headerTxtSetRaw
    setTrueTestLabels
    startNewGameNameInput
    startTrueTestPrompt
    trueTestModeCancel
    trueTestModeClose
    trueTestModeCursorMove
    trueTestModeIn
    trueTestModeSelect
d/d_gameover.h  (2)
    warpChoice_init
    warpChoice_proc
d/d_item.h  (3)
    isAmmo
    item_func_WALLET_LV4
    item_getcheck_func_WALLET_LV4
d/d_kankyo.h  (4)
    dKy_GxFog_sea_set
    dKy_get_seacolor
    dKy_usonami_set
    dKy_wave_chan_init
d/d_kankyo_rain.h  (2)
    drawWave
    wave_move
d/d_kankyo_wether.h  (7)
    WAVE_EFF
    dKankyo_wave_Packet
    dKyw_getWaveInfl
    dKyw_wave_calm_onStage
    dKyw_wave_calm_update
    dKyw_ww_host_wind_onStage
    dKyw_ww_windline_delete
d/d_menu_collect.h  (1)
    clearSubWindowOpenCheck
d/d_menu_ring.h  (21)
    applyQuickEquipBagView
    applyQuickEquipPage
    clearQuickEquipItemTextures
    confirmQuickEquipHover
    drawQuickEquipPageCue
    forceQuickConfirmClose
    getHighlightedItem
    getQuickEquipSimScale
    getQuickRegistrySlot
    getQuickRingItem
    isQuickEquipLiveWorld
    isQuickEquipMode
    isQuickEquipPagesExclusive
    loadQuickEquipItemTextures
    peekPendingQuickEquip
    remapQuickEquipFaceSlots
    setPendingQuickEquip
    setQuickEquipLiveWorld
    tryQuickEquipBagOpen
    tryQuickEquipPageFlip
    usesQuickEquipPages
d/d_menu_skill.h  (7)
    applySkillPageState
    changePage
    isAlbwScrollPage
    setDetailPage
    setNameStringRaw
    setRowVisibility
    updatePageTitle
d/d_menu_window.h  (11)
    dMw_ext_status_create
    dMw_ext_status_delete
    dMw_getRingCursorItem
    ext_status_close_init
    ext_status_close_proc
    ext_status_move_init
    ext_status_move_proc
    ext_status_open_init
    ext_status_open_proc
    getRingCursorItem
    isMenuActive
d/d_meter2_draw.h  (24)
    applyMagicMeterLayoutTransient
    applyMagicMeterSlot
    changeTextureItemZ
    dLopHudOn
    drawButtonZItem
    drawFocusedArtsMeter
    drawFocusedArtsMeterKantera
    drawLopHealthBar
    drawShieldDurabilityBelowAlbw
    getKanteraRootOffset
    getRupeeAnchorCenter
    getRupeeDigitMetrics
    getRupeeHudAlphaRate
    getRupeeHudReferenceSize
    getShieldHudAnchorCenter
    getShowMidnaIcon
    hideButtonZItem
    isLopHealthBarActive
    isLopHudActive
    layoutMagicMeterSlot
    lopRestoreButtonRing
    setAlphaMagicAnimeMax
    setAlphaMagicAnimeMin
    setButtonIconZItemAlpha
d/d_meter2_info.h  (82)
    dMeter2_addALBWBaseFraction
    dMeter2_addALBWFraction
    dMeter2_applyEquippedShield
    dMeter2_canALBWArmorBlock
    dMeter2_canALBWArrow
    dMeter2_canALBWBackJump
    dMeter2_canALBWBomb
    dMeter2_canALBWBombArrow
    dMeter2_canALBWBombling
    dMeter2_canALBWBoom
    dMeter2_canALBWDekuLeaf
    dMeter2_canALBWDomRod
    dMeter2_canALBWDomRodConfuseThrow
    dMeter2_canALBWDoubleHookshot
    dMeter2_canALBWHiddenSkill
    dMeter2_canALBWHookshot
    dMeter2_canALBWIronball
    dMeter2_canALBWRollJump
    dMeter2_canALBWSidestep
    dMeter2_canALBWSling
    dMeter2_canALBWSpinner
    dMeter2_canALBWSword
    dMeter2_canAcquireShield
    dMeter2_commitALBWHiddenSkillIfPending
    dMeter2_countOwnedShields
    dMeter2_drainALBWToLockout
    dMeter2_equipOwnedShield
    dMeter2_fillALBWMeter
    dMeter2_getALBWLockoutRecoveryRate
    dMeter2_getALBWMaxValue
    dMeter2_getALBWMeterValue
    dMeter2_getALBWNormalRecoveryRate
    dMeter2_getNextOwnedShield
    dMeter2_grantRentalClothes
    dMeter2_grantRentalShield
    dMeter2_grantShieldOwnership
    dMeter2_isALBWArmorDepleted
    dMeter2_isALBWDepleted
    dMeter2_isALBWLocked
    dMeter2_isALBWMovementExhausted
    dMeter2_isALBWRentalEligible
    dMeter2_isCasualWearEligible
    dMeter2_isHerosWearEligible
    dMeter2_isInCombatEncounter
    dMeter2_isShieldItem
    dMeter2_isShieldRentalEligible
    dMeter2_isWolfForm
    dMeter2_isZoraWearEligible
    dMeter2_onALBWArmorHit
    dMeter2_onALBWArrow
    dMeter2_onALBWBackJump
    dMeter2_onALBWBomb
    dMeter2_onALBWBombArrow
    dMeter2_onALBWBoom
    dMeter2_onALBWDekuLeaf
    dMeter2_onALBWDekuLeafStart
    dMeter2_onALBWDomRod
    dMeter2_onALBWDoubleHookshot
    dMeter2_onALBWHiddenSkill
    dMeter2_onALBWHookshot
    dMeter2_onALBWIronball
    dMeter2_onALBWMeterShopPurchase
    dMeter2_onALBWRentalEligible
    dMeter2_onALBWRollJump
    dMeter2_onALBWSidestep
    dMeter2_onALBWSling
    dMeter2_onALBWSpinner
    dMeter2_onALBWSword
    dMeter2_onArmorAttackHit
    dMeter2_onArmorEncounterHit
    dMeter2_onMultiShieldUpgradePurchase
    dMeter2_onShieldDestroyedForRental
    dMeter2_playerHasAnyShield
    dMeter2_playerHasMultiShieldUpgrade
    dMeter2_playerOwnsRentalItem
    dMeter2_restoreALBWMeterToFull
    dMeter2_setALBWPlayerIdle
    dMeter2_setShieldOwned
    dMeter2_shieldIsOwned
    dMeter2_stripAllALBWInventoryOnDeath
    dMeter2_stripRentalItemOnDeath
    dMeter2_subALBWFraction
d/d_msg_object.h  (3)
    bool
    dMsg_resolveGroupArchive
    dMsg_setGroupArchiveHook
d/d_particle.h  (6)
    ensureTearSceneRes
    ensureWwCommonRes
    ensureWwWindlineRes
    getScenePrtclNo
    hasSceneParticleRes
    hasWwWindlineRes
d/d_resorce.h  (2)
    dRes_aliasArcFileName
    dRes_setArcFileNameHook
d/d_s_name.h  (1)
    getFileSelectForDrive
d/d_save.h  (10)
    getExtModFlags
    getReserve
    hasGanonGatePolicyV1
    hasTwilightPolicyV1
    isBootstrapApplied
    isTrueTest
    setBootstrapApplied
    setGanonGatePolicyV1
    setTrueTest
    setTwilightPolicyV1
d/d_stage.h  (6)
    bytes
    dStage_dt_c_stageLoader
    dStage_setPlyrParamHook
    getBgW
    static_assert
    void
f_op/f_op_actor_mng.h  (4)
    dAlbwLockout_queryRivalAimAngleX
    dAlbwLockout_queryRivalAimAngleY
    dAlbwLockout_queryRivalAimDistance
    dAlbwLockout_queryRivalAimDistanceXZ
f_op/f_op_msg_mng.h  (6)
    fopMsgM_checkForceSend
    fopMsgM_forceSendOff
    fopMsgM_forceSendOn
    fopMsgM_getScopeMode
    fopMsgM_releaseScopeMode
    fopMsgM_scopeMessageSet
m_Do/m_Do_audio.h  (2)
    getSettings
    getValue
m_Do/m_Do_controller_pad.h  (1)
    getSettings
```
