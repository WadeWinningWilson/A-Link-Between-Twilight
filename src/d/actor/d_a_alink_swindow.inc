/**
 * d_a_alink_swindow.inc
 * Pause Menu Player Display
 */

#include "d/actor/d_a_alink.h"
#include "JSystem/JKernel/JKRExpHeap.h"
#include "JSystem/J3DGraphLoader/J3DAnmLoader.h"

static int daAlink_modelCallBack(J3DJoint* i_joint, int param_1);
static int daAlink_headModelCallBack(J3DJoint* i_joint, int param_1);
static int daAlink_wolfModelCallBack(J3DJoint* i_joint, int param_1);

void daAlink_c::setArcName(BOOL i_isWolf) {
    if (i_isWolf) {
        mArcName = l_wArcName;
    } else if (checkCasualWearFlg()) {
        mArcName = l_bArcName;
    } else if (checkZoraWearFlg()) {
        mArcName = l_zArcName;
    } else if (checkMagicArmorWearFlg()) {
        mArcName = l_mArcName;
    } else {
        mArcName = l_kArcName;
    }
}

void daAlink_c::setShieldArcName() {
#if TARGET_PC
    const u8 equip = dComIfGs_getSelectEquipShield();
    // Always derive the body arc from equipped shield — required for save-editor multi-shield
    // cycling where several first bits may be set at once. See docs/shield-ordon-quest-assets.md.
    switch (equip) {
    case dItemNo_WOOD_SHIELD_e:
        mShieldArcName = l_cWShdArcName;
        return;
    case dItemNo_SHIELD_e:
        mShieldArcName = l_sWShdArcName;
        return;
    case dItemNo_HYLIA_SHIELD_e:
        mShieldArcName = l_hyShdArcName;
        return;
    default:
        break;
    }

    // No shield equipped — preview/init fallback when equip is unset.
    if (dComIfGs_isItemFirstBit(dItemNo_HYLIA_SHIELD_e)) {
        mShieldArcName = l_hyShdArcName;
    } else if (dComIfGs_isItemFirstBit(dItemNo_WOOD_SHIELD_e)) {
        mShieldArcName = l_cWShdArcName;
    } else if (dComIfGs_isItemFirstBit(dItemNo_SHIELD_e)) {
        mShieldArcName = l_sWShdArcName;
    } else {
        mShieldArcName = l_cWShdArcName;
    }
#else
    if (checkCarvingWoodShieldEquip() || !checkShieldGet()) {
        mShieldArcName = l_cWShdArcName;
    } else if (checkShopWoodShieldEquip()) {
        mShieldArcName = l_sWShdArcName;
    } else {
        mShieldArcName = l_hyShdArcName;
    }
#endif
}

void daAlink_c::setOriginalHeap(JKRExpHeap** i_ppheap, u32 i_size) {
    if (*i_ppheap == NULL) {
        u32 var_r29 = 0x90;
        u32 var_r28 = 0x10;
        u32 size = ROUND(i_size, 16);
#if TARGET_PC
        size *= 20; // Increase Link's heap size to prevent mods from crashing with higher-quality models.
#endif
        JKRHeap* parent = mDoExt_getGameHeap();

        JKRExpHeap* heap = JKRExpHeap::create(size + (var_r29 + var_r28), parent, true);
        *i_ppheap = heap;
    }
}

void daAlink_c::setClothesChange(int param_0) {
    if (!checkWolf()) {
#if TARGET_PC
        // Ignore re-entrant requests while a reload is in flight — resetting the
        // timer mid-sequence corrupts loadModelDVD() and crashes under mash/warp.
        if (mClothesChangeWaitTimer != 0) {
            return;
        }
#endif
        mClothesChangeWaitTimer = 4;
#if TARGET_PC
        // Each new change re-preps the build-then-swap; never inherit a stuck swap flag.
        s_albwSwapActive = false;
#endif

        if (param_0 != 0) {
            onNoResetFlg2(FLG2_UNK_200000);
        }
    }
}

void daAlink_c::setShieldChange() {
    if (mShieldChangeWaitTimer != 0) {
        return;
    }

    // Model is cleared during reload; drop guard so collision/guard code does not use a null shield mtx.
    offNoResetFlg2(FLG2_UNK_8000000);
    mShieldChangeWaitTimer = 4;
}

int daAlink_c::loadModelDVD() {
    if (mClothesChangeWaitTimer != 0) {
        mClothesChangeWaitTimer--;

        if (mClothesChangeWaitTimer == 2) {
            mEyeHL1.remove();
            mEyeHL2.remove();
            mpWlMidnaModel = NULL;
            mpWlMidnaMaskModel = NULL;
            mpWlMidnaHandModel = NULL;
            mpWlMidnaHairModel = NULL;

            if (!checkNoResetFlg2(FLG2_UNK_280000)) {
                const bool isMeta =
                    (mProcID == PROC_METAMORPHOSE || mProcID == PROC_METAMORPHOSE_ONLY);
#if TARGET_PC
                // BUILD-THEN-SWAP prep (normal clothes change, alt heap available): keep the
                // OLD models fully valid -- do NOT free here.  Just remember the old arc (to
                // free after the swap) and point mArcName at the new arc.  Runs once; the
                // timer 1->2 retry re-enters at ==1, never here.  See d_a_alink.cpp.
                if (!isMeta && s_albwArcHeapB != NULL) {
                    if (!s_albwSwapActive) {
                        s_albwSwapOldArc = mArcName;
                        setArcName(checkWolf());   // non-meta -> the new clothes arc
                        cPhs_Reset(&s_albwPhaseReqB);
                        s_albwSwapActive = true;
                    }
                } else
#endif
                {
                    // In-place free-then-load: wolf metamorphose, or fallback (no alt heap).
#if TARGET_PC && D_ALBW_ARC_LIFECYCLE_DEBUG
                    DuskLog.debug("ALBW-LIFE loadModelDVD FREEALL mArcName={} arc={} heap={} procID={}",
                                  (mArcName ? mArcName : "(null)"), albwArcArchive(mArcName),
                                  (void*)mpArcHeap, mProcID);
#endif
                    if (!dComIfG_resDelete(&mPhaseReq, mArcName)) {
#if TARGET_PC
                        // resDelete no-ops if load in-progress; force-unregister before freeAll
                        dComIfG_deleteObjectResMain(mArcName);
#endif
                    }
                    cPhs_Reset(&mPhaseReq);
                    mpArcHeap->freeAll();
#if TARGET_PC
                    ++s_albwArcEpoch;  // in-place free: prior-epoch models are now stale
#endif
                    if (isMeta) {
                        setArcName(!checkWolf());
                    } else {
                        setArcName(checkWolf());
                    }
                }
            }
        } else if (mClothesChangeWaitTimer == 1) {
            if (checkNoResetFlg2(FLG2_UNK_280000)) {
                mClothesChangeWaitTimer = 0;
                changeLink(1);
            } else {
                const bool isMeta =
                    (mProcID == PROC_METAMORPHOSE || mProcID == PROC_METAMORPHOSE_ONLY);
#if TARGET_PC
                if (s_albwSwapActive) {
                    // BUILD-THEN-SWAP: load the new arc into the alt heap, build from it, then
                    // free the old heap and swap.  The old models stay valid until the swap,
                    // so draw/calc/shadow never see freed memory -- no freed-window crash.
                    if (mArcName == s_albwSwapOldArc) {
                        // Same arc re-equipped: still resident in mpArcHeap -> rebuild in place.
                        mClothesChangeWaitTimer = 0;
                        s_albwSwapActive = false;
                        changeLink(1);
                        return 1;
                    }
                    if (dComIfG_resLoad(&s_albwPhaseReqB, mArcName, s_albwArcHeapB) ==
                        cPhs_COMPLEATE_e) {
                        mClothesChangeWaitTimer = 0;
                        changeLink(1);  // builds the new models from the alt heap; repoints refs
#if D_ALBW_ARC_LIFECYCLE_DEBUG
                        DuskLog.debug("ALBW-LIFE SWAP newArc={} oldArc={} -> free old heap={}",
                                      (mArcName ? mArcName : "(null)"),
                                      (s_albwSwapOldArc ? s_albwSwapOldArc : "(null)"),
                                      (void*)mpArcHeap);
#endif
                        // Live ptrs now reference the alt heap, so the old heap is free to drop.
                        if (!dComIfG_resDelete(&mPhaseReq, s_albwSwapOldArc)) {
                            dComIfG_deleteObjectResMain(s_albwSwapOldArc);
                        }
                        cPhs_Reset(&mPhaseReq);
                        mpArcHeap->freeAll();
                        // Swap heaps + phase requests so mpArcHeap / mPhaseReq track the live arc.
                        {
                            JKRExpHeap* l_th = mpArcHeap;
                            mpArcHeap = s_albwArcHeapB;
                            s_albwArcHeapB = l_th;
                        }
                        {
                            request_of_phase_process_class l_tr = mPhaseReq;
                            mPhaseReq = s_albwPhaseReqB;
                            s_albwPhaseReqB = l_tr;
                        }
                        s_albwSwapActive = false;
                        return 1;
                    }
                    mClothesChangeWaitTimer = 2;  // alt arc not loaded yet; retry
                } else
#endif
                {
                    // In-place load: wolf metamorphose, or fallback (no alt heap).
                    if (dComIfG_resLoad(&mPhaseReq, mArcName, mpArcHeap) == cPhs_COMPLEATE_e) {
                        mClothesChangeWaitTimer = 0;
                        if (isMeta) {
                            if (checkWolf()) {
                                changeLink(0);
#if TARGET_PC
                                dAlbwOutfit_onMetamorphoseToHuman(this);
#endif
                            } else {
                                changeWolf();
                            }
                        } else {
                            changeLink(1);
                        }
                        return 1;
                    }
                    mClothesChangeWaitTimer = 2;
                }
            }
        }
    } else {
        return 1;
    }

    return 0;
}

void daAlink_c::setShieldModel() {
    JKRHeap* heap = mAnmHeap4.setAnimeHeap();
    J3DModelData* modelData = (J3DModelData*)dComIfG_getObjectRes(mShieldArcName, 3);
    if (modelData == NULL) {
        mShieldModel = NULL;
        mDoExt_setCurrentHeap(heap);
        return;
    }

    mShieldModel = initModel(modelData, 0);

    mDoExt_setCurrentHeap(heap);
}

int daAlink_c::loadShieldModelDVD() {
    if (mShieldChangeWaitTimer != 0) {
        mShieldChangeWaitTimer--;

        if (mShieldChangeWaitTimer == 2) {
            mShieldModel = NULL;
            if (!dComIfG_resDelete(&mShieldPhaseReq, mShieldArcName)) {
#if TARGET_PC
                // resDelete no-ops when phase id != 2; force-unregister before freeAll
                dComIfG_deleteObjectResMain(mShieldArcName);
#endif
            }
            cPhs_Reset(&mShieldPhaseReq);
            if (mpShieldArcHeap != NULL) {
                mpShieldArcHeap->freeAll();
            }
            setShieldArcName();
        } else if (mShieldChangeWaitTimer == 1) {
            if (mpShieldArcHeap == NULL) {
                mShieldChangeWaitTimer = 0;
                return 1;
            }

            int phase_state = dComIfG_resLoad(&mShieldPhaseReq, mShieldArcName, mpShieldArcHeap);
            if (phase_state == cPhs_COMPLEATE_e) {
                mShieldChangeWaitTimer = 0;
                setShieldModel();
                if (mShieldModel == NULL) {
                    OS_REPORT("loadShieldModelDVD: missing model for arc %s equip %u\n", mShieldArcName,
                              dComIfGs_getSelectEquipShield());
                }
            } else {
                mShieldChangeWaitTimer = 2;
            }
        }
    } else {
        return 1;
    }

    return 0;
}

void daAlink_c::changeModelDataDirect(int param_0) {
    u16 i;
    J3DModelData* modelData;

    field_0x064C = mpLinkModel->getModelData();
    field_0x06c0 = mpLinkFaceModel->getModelData();
    
    if (!checkNoResetFlg2(FLG2_STATUS_WINDOW_DRAW)) {
        field_0x064C->getJointNodePointer(0)->setMtxCalc(field_0x1f20);
        field_0x064C->getJointNodePointer(1)->setMtxCalc(field_0x1f24);
        field_0x064C->getJointNodePointer(16)->setMtxCalc(field_0x1f20);

        for (i = 0; i < 35; i++) {
            field_0x064C->getJointNodePointer(i)->setCallBack(daAlink_modelCallBack);
        }

        modelData = mpLinkHatModel->getModelData();
        for (i = 1; i < modelData->getJointNum(); i++) {
            modelData->getJointNodePointer(i)->setCallBack(daAlink_headModelCallBack);
        }

        if (param_0) {
            if (mpFaceBtp != NULL) {
                field_0x06c0->entryTexNoAnimator(mpFaceBtp);
            }

            if (mpFaceBtk != NULL) {
                field_0x06c0->entryTexMtxAnimator(mpFaceBtk);
            }
        }
    } else {
        field_0x064C->getJointNodePointer(0)->setMtxCalc(NULL);
        field_0x064C->getJointNodePointer(1)->setMtxCalc(NULL);
        field_0x064C->getJointNodePointer(16)->setMtxCalc(NULL);
        field_0x06c0->getJointNodePointer(0)->setMtxCalc(NULL);

        for (i = 0; i < 35; i++) {
            field_0x064C->getJointNodePointer(i)->setCallBack(NULL);
        }

        modelData = mpLinkHatModel->getModelData();
        for (i = 1; i < modelData->getJointNum(); i++) {
            modelData->getJointNodePointer(i)->setCallBack(NULL);
        }

        if (!checkNoResetFlg2(daPy_FLG2(FLG2_UNK_100000 | FLG2_UNK_80000))) {
            for (i = 6; i < 8; i++) {
                modelData->getJointNodePointer(i)->setCallBack(daAlink_headModelCallBack);
            }
        }
    }
}

void daAlink_c::changeModelDataDirectWolf(int param_0) {
    u16 i;

    field_0x064C = mpLinkModel->getModelData();
    field_0x06c0 = mpLinkModel->getModelData();
    
    if (!checkNoResetFlg2(FLG2_STATUS_WINDOW_DRAW)) {
        field_0x064C->getJointNodePointer(0)->setMtxCalc(field_0x1f20);
        field_0x064C->getJointNodePointer(3)->setMtxCalc(field_0x1f24);
        field_0x064C->getJointNodePointer(15)->setMtxCalc(field_0x1f20);

        for (i = 0; i < 40; i++) {
            field_0x064C->getJointNodePointer(i)->setCallBack(daAlink_wolfModelCallBack);
        }

        if (param_0) {
            if (mpFaceBtp != NULL) {
                field_0x06c0->entryTexNoAnimator(mpFaceBtp);
            }

            if (mpFaceBtk != NULL) {
                field_0x06c0->entryTexMtxAnimator(mpFaceBtk);
            }
        }
    } else {
        field_0x064C->getJointNodePointer(0)->setMtxCalc(NULL);
        field_0x064C->getJointNodePointer(3)->setMtxCalc(NULL);
        field_0x064C->getJointNodePointer(15)->setMtxCalc(NULL);

        for (i = 0; i < 40; i++) {
            field_0x064C->getJointNodePointer(i)->setCallBack(NULL);
        }
    }
}

void daAlink_c::initStatusWindow() {
    onNoResetFlg2(FLG2_STATUS_WINDOW_DRAW);
    
    void* tmpBuffer = JKR_NEW_ARRAY_ARGS(void*, 0x500, 0x20);
    JUT_ASSERT(394, tmpBuffer);

    u16 bckResIdx, btpResIdx, btkResIdx;
    if (checkWolf()) {
        bckResIdx = dRes_ID_ALANM_BCK_WL_WAITA_e;
        btpResIdx = dRes_ID_ALANM_BTP_WL_FA_e;
        btkResIdx = dRes_ID_ALANM_BTK_WL_FA_e;
        changeModelDataDirectWolf(0);
    } else {
        bckResIdx = dRes_ID_ALANM_BCK_WAITS_e;
        btpResIdx = dRes_ID_ALANM_BTP_FA_e;
        btkResIdx = dRes_ID_ALANM_BTK_FA_e;

        if (checkZoraWearAbility() && !checkZoraWearMaskDraw()) {
            field_0x06f0->hide();
        }

        changeModelDataDirect(0);
    }

    JKRReadIdxResource(tmpBuffer, 0x1400, bckResIdx, dComIfGp_getAnmArchive());
    m_sWindowBck = JKR_NEW mDoExt_bckAnm();
    JUT_ASSERT(428, m_sWindowBck);

    if (!m_sWindowBck->init((J3DAnmTransform*)J3DAnmLoaderDataBase::load(tmpBuffer), TRUE, 2, 1.0f, 0, -1, false)) {
        JUT_ASSERT(433, FALSE);
    }

    tmpBuffer = JKR_NEW_ARRAY_ARGS(void*, 0x100, 0x20);
    JUT_ASSERT(437, tmpBuffer);

    JKRReadIdxResource(tmpBuffer, 0x400, btpResIdx, dComIfGp_getAnmArchive());
    // this should call J3DAnmLoaderDataBase::load(const void*) but it breaks retail
    J3DAnmTexPattern* btp = (J3DAnmTexPattern*)J3DAnmLoaderDataBase::load(tmpBuffer, J3DLOADER_UNK_FLAG0);
    btp->setFrame(0.0f);
    btp->searchUpdateMaterialID(field_0x06c0);
    field_0x06c0->entryTexNoAnimator(btp);

    tmpBuffer = JKR_NEW_ARRAY_ARGS(void*, 0x100, 0x20);
    JUT_ASSERT(449, tmpBuffer);

    JKRReadIdxResource(tmpBuffer, 0x400, btkResIdx, dComIfGp_getAnmArchive());
    // this should call J3DAnmLoaderDataBase::load(const void*) but it breaks retail
    J3DAnmTextureSRTKey* btk = (J3DAnmTextureSRTKey*)J3DAnmLoaderDataBase::load(tmpBuffer, J3DLOADER_UNK_FLAG0);
    btk->setFrame(0.0f);
    btk->searchUpdateMaterialID(field_0x06c0);
    field_0x06c0->entryTexMtxAnimator(btk);
}

void daAlink_c::statusWindowExecute(const cXyz* i_pos, s16 i_angle) {
    if (loadModelDVD()) {
        if (mSwordChangeWaitTimer != 0) {
            mSwordChangeWaitTimer--;
        }

        loadShieldModelDVD();
        setSelectEquipItem(TRUE);
        
        if (mSwordChangeWaitTimer >= 30) {
            mSwordChangeWaitTimer = 0;
        }

        mDoMtx_stack_c::transS(*i_pos);
        mDoMtx_stack_c::YrotM(i_angle);
        mpLinkModel->setBaseTRMtx(mDoMtx_stack_c::get());

        m_sWindowBck->play();
        m_sWindowBck->entry(field_0x064C);
        mpLinkModel->calc();

        if (checkWolf()) {
            setWolfItemMatrix();
        } else {
            setItemMatrix(1);
        }
    }
}

void daAlink_c::statusWindowDraw() {
    static const GXColorS10 waterColor = {0, 0, 0, 0};

    if (mClothesChangeWaitTimer == 0) {
        g_env_light.settingTevStruct(12, &current.pos, &tevStr);
        initTevCustomColor();

        if (!checkWolf()) {
            setWaterDropColor((J3DGXColorS10*)&waterColor);
            setDrawHand();
        }

        basicModelDraw(mpLinkModel);

        if (!checkWolf()) {
            basicModelDraw(mpLinkHandModel);
            basicModelDraw(mpLinkHatModel);
            basicModelDraw(mpLinkFaceModel);

            if (checkEquipHeavyBoots()) {
                for (int i = 0; i < 2; i++) {
                    basicModelDraw(mpLinkBootModels[i]);
                }
            }
        }

        if (checkSwordDraw()) {
            basicModelDraw(mSwordModel);
            if (!checkWoodSwordEquip()) {
                basicModelDraw(mSheathModel);
            }
        }

        if (checkShieldDraw()) {
            basicModelDraw(mShieldModel);
        }
    }
}

void daAlink_c::resetStatusWindow() {
    offNoResetFlg2(FLG2_STATUS_WINDOW_DRAW);

    if (checkWolf()) {
        changeModelDataDirectWolf(1);
    } else {
        changeModelDataDirect(1);
    }

    setMatrix();
    mpLinkModel->calc();
    onNoResetFlg2(FLG2_STATUS_WINDOW_DRAW);
    setSelectEquipItem(FALSE);

    if (checkWolf()) {
        setWolfItemMatrix();
    } else {
        setItemMatrix(0);
    }

    offNoResetFlg2(FLG2_STATUS_WINDOW_DRAW);
    m_sWindowBck = NULL;
}
