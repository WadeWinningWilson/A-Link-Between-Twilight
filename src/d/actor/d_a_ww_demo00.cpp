// ============================================================
// §271  d_a_ww_demo00.cpp — WW demo00 puppet actor (donor `daDemo00_c`).
//
// Direct port of the WW retail demo00 (Foundry kit
// docs/WW Linked/port-kits/ww_demo00), renamed daDemo00_* -> daWwDemo00_* so it
// COEXISTS with TP's own src/d/actor/d_a_demo00.cpp (same class/profile names).
// On a WW-hosted stage the storyboard's `d_act0..d_act31` cast is routed here by
// dDemo_system_c::JSGFindObject (§271 gate in d_demo.cpp); TP's cutscenes keep
// using TP's DEMO00. This reads the WW storyboard's shape/anim/btp data channel
// via the donor's own TParseData path, so it does NOT fall into TP demo00's
// movie-player "Movie Start Wait" case that livelocks the tale at frame 0.
//
// Covenant: donor-faithful. Signature/name gaps are adapted (rename/retype) or
// shimmed benign per the pig/ba1/zl1 doctrine; the model+anim binding and the
// render path are faithful. Every non-faithful stub is tagged [INFERENCE-NEEDED]
// with a BRIDGE-OWED note. VERSION_* conditionals are resolved to retail-US.
// ============================================================

#include "d/dolzel_rel.h" // IWYU pragma: keep
#include "d/actor/d_a_ww_demo00.h"
#include "JSystem/J3DGraphAnimator/J3DSkinDeform.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DTexture.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_com_inf_game.h"
#include "d/d_demo.h"
#include "d/d_kankyo.h"
#include "d/d_stage.h"
#include "d/d_ext_ww_actor_shims.h"   // §271 TEV_TYPE_ACTOR (mirrors ba1/zl1)
#include "d/d_ext_npc_mount.h"        // §271 DN-3: parse-at-consume demo BDL resolver
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_mtx.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/JGadget/pointer.h"
#include "JSystem/JGadget/define.h"
#include "string.h"
#if TARGET_PC
#include "dusk/logging.h"
#include "d/d_kankyo_ww.h"           // §404 WW lighting write-path (was the empty stub)
#endif

// ============================================================
// §271 direct-port shims — donor identifiers the port lacks.
// Same doctrine as d_ext_ww_actor_shims (pig/ba1/zl1): donor-faithful where the
// symbol exists, inert-benign where the subsystem is absent. All shims here sit
// on AUXILIARY channels (demo point-light, monotone screen fx, mask draw-lists,
// zel-sound lifecycle); the load-bearing model/anim/render path is unshimmed.
// ============================================================

// DEMO_PLIGHT — WW demo point-light record (dKydm demo-plight subsystem,
// d_kankyo_demo.h). NOT ported. [INFERENCE-NEEDED §271] The plight channel
// (prm r5==2) is inert; puppets render without the moving demo light.
// BRIDGE-OWED: port dKydm_demo_plight_* for cutscenes that add a demo point light.
struct DEMO_PLIGHT { u8 _pad[4]; };
static void dExtWwDemo00_plight_entry(DEMO_PLIGHT*, cXyz*, int, u8) {}
static void dExtWwDemo00_plight_execute(DEMO_PLIGHT*, cXyz*) {}
static void dExtWwDemo00_plight_delete(DEMO_PLIGHT*) {}

// setListMaskOff / setListP1 — WW draw-list buckets the port's drawlist lacks.
// [INFERENCE-NEEDED §271] Fall back to the default list so the model still enters
// the render pass (mask/P1 layering is a deferred cosmetic).
static void dExtWwDemo00_setListMaskOff() { dComIfGd_setList(); }
static void dExtWwDemo00_setListP1() { dComIfGd_setList(); }

// mDoExt_McaMorf::stopZelAnime — absent on the port's McaMorf (only the SO/2
// variants expose it; §246 dExtNpcBm1_stopZelAnime precedent). No-op: this
// McaMorf carries no zel-sound object here.
static void dExtWwDemo00_stopZelAnime(mDoExt_McaMorf*) {}

// mDoGph_gInf_c::setMonotoneRateSpeed — absent. [INFERENCE-NEEDED §271] No-op
// (monotone screen fx, prm channel 6; deferred cosmetic). BRIDGE-OWED.
static void dExtWwDemo00_setMonotoneRateSpeed(s16) {}

// dDemo_prm_c::getId() — the port exposes the JStudio data id as field_0x0 (set
// by dDemo_actor_c::JSGSetData). Same value WW's getId() returns.
static u32 dExtWwDemo00_getPrmId(dDemo_prm_c* p) { return p->field_0x0; }

// ============================================================
// §271 per-frame demo-light table (donor l_lightData). Inert while the plight
// shims are no-op, but kept verbatim so the (dead) plight path stays faithful.
// ============================================================
struct light_data_s {
    /* 0x00 */ bool useJoint;
    /* 0x01 */ u8 m01;
    /* 0x04 */ const char* jointName;
};
static light_data_s l_lightData[] = {
    {true,  1, "V_24_tri_joint"},
    {false, 1, NULL},
    {false, 1, NULL},
};

// ============================================================
// §271 resource-id / model bundle resets (donor daDemo00_resID_c / _model_c)
// ============================================================
void daWwDemo00_resID_c::reset() {
    mShapeID = -1;
    mBckID = -1;
    field_0x08 = -1;
    mBtpID = -1;
    mBtkID = -1;
    mBrkID = -1;
    field_0x18 = -1;
    field_0x1C = -1;
    mPlightID = -1;
    mShadowID = -1;
}

void daWwDemo00_model_c::reset() {
    mID.reset();
    mpModel = NULL;
    mpBtpAnm = NULL;
    mpBtkAnm = NULL;
    mpBrkAnm = NULL;
    if (mpPlight != NULL) {
        dExtWwDemo00_plight_delete(mpPlight);
        mpPlight = NULL;
    }
    mpShadow = NULL;
}

daWwDemo00_c::~daWwDemo00_c() {
    if (heap != NULL && mModel.mpMorf != NULL) {
        dExtWwDemo00_stopZelAnime(mModel.mpMorf);
    }
}

// ============================================================
// §271 create — modelless until the storyboard's ENABLE_SHAPE binds a shape
// ============================================================
BOOL daWwDemo00_c::create() {
    fopAcM_ct(this, daWwDemo00_c);
    dKy_tevstr_init(&tevStr, dComIfGp_roomControl_getStayNo(), 0xFF);
    setAction(&daWwDemo00_c::actStandby);
    mNextID.reset();
    field_0x29d = -1;
    return cPhs_COMPLEATE_e;
}

// ============================================================
// §271 base matrix
// ============================================================
void daWwDemo00_c::setBaseMtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::XYZrotM(shape_angle.x, shape_angle.y, shape_angle.z);
    mModel.mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    mModel.mpModel->setBaseScale(scale);
    mModel.mpModel->calc();
}

// ============================================================
// §271 shadow bounds
// ============================================================
void daWwDemo00_c::setShadowSize() {
    J3DModelData* modelData = mModel.mpModel->getModelData();

    cXyz min(100000000.0f, 100000000.0f, 100000000.0f);
    cXyz max(-100000000.0f, -100000000.0f, -100000000.0f);

    for (u16 i = 0; i < modelData->getJointNum(); i++) {
        J3DJoint* joint = modelData->getJointNodePointer(i);
        if (joint->getKind() == 0) {
            cXyz jntMin;
            cXyz jntMax;

            mDoMtx_multVec(mModel.mpModel->getAnmMtx(i), joint->getMin(), &jntMin);
            mDoMtx_multVec(mModel.mpModel->getAnmMtx(i), joint->getMax(), &jntMax);
            min.x = jntMin.x < min.x ? jntMin.x : min.x;
            min.y = jntMin.y < min.y ? jntMin.y : min.y;
            min.z = jntMin.z < min.z ? jntMin.z : min.z;
            max.x = jntMax.x > max.x ? jntMax.x : max.x;
            max.y = jntMax.y > max.y ? jntMax.y : max.y;
            max.z = jntMax.z > max.z ? jntMax.z : max.z;
        }
    }

    mModel.mpShadow->mPos.x = (max.x + min.x) * 0.5f;
    mModel.mpShadow->mPos.y = (max.y + min.y) * 0.5f;
    mModel.mpShadow->mPos.z = (max.z + min.z) * 0.5f;

    cXyz extents = max - min;
    mModel.mpShadow->mCasterSize = extents.abs() * 3.0f;
    mModel.mpShadow->mSimpleScale = extents.absXZ() * 0.25f;
}

// ============================================================
// §271 awa (framebuffer / cy_kankyo dummy texture) patch — donor verbatim
// ============================================================
static BOOL awaCheck(J3DModel* model) {
    J3DModelData* modelData = model->getModelData();
    J3DTexture* tex = modelData->getTexture();
    if (tex != NULL) {
        JUTNameTab* texName = modelData->getTextureName();
        if (texName != NULL) {
            for (u16 i = 0; i < tex->getNum(); i++) {
                const char* name = texName->getName(i);
                if (strcmp(name, "B_dummy") == 0 || strcmp(name, "cy_kankyo") == 0) {
                    J3DSkinDeform* deform = new J3DSkinDeform();
                    if (deform == NULL)
                        return FALSE;

                    if (model->setSkinDeform(deform, 1) != 0)
                        return FALSE;

                    if (strcmp(name, "B_dummy") == 0) {
                        tex->setResTIMG(i, *mDoGph_gInf_c::getFrameBufferTimg());
                        mDoExt_modelTexturePatch(modelData);
                    }
                }
            }
        }
    }
    return TRUE;
}

static BOOL createHeapCallBack(fopAc_ac_c* i_this) {
    return ((daWwDemo00_c*)i_this)->createHeap();
}

// ============================================================
// §271 createHeap — build model + anims from the WW demo arc
// ============================================================
BOOL daWwDemo00_c::createHeap() {
    if (mModel.mID.mShapeID != -1) {
#if TARGET_PC
        // §271 DN-3 (§266): demo-arc BDLs arrive RAW from getObjectIDRes; parse at
        // CONSUME time via ExtNpcMount's cached/pristine resolver (single-parse,
        // dropped with the demo arc — no UAF). Numeric id LOOKUPS still work; only
        // BDL MODELS need this. Anims below stay on plain getObjectIDRes.
        void* rawRes = dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(),
                                              (u16)mModel.mID.mShapeID);
        J3DModelData* modelData = dExtNpcMount_acquireDemoModel(
            dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mShapeID, rawRes);
#else
        J3DModelData* modelData = (J3DModelData*)dComIfG_getObjectIDRes(
            dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mShapeID);
#endif
        if (modelData == NULL) {
            return FALSE;
        }

        u32 r28 = 0x11000002;

        if (mModel.mID.mBtpID != -1) {
            mModel.mpBtpAnm = new mDoExt_btpAnm();
            if (mModel.mpBtpAnm == NULL) {
                return FALSE;
            }
            J3DAnmTexPattern* anm = (J3DAnmTexPattern*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mBtpID);
            if (anm == NULL) {
                return TRUE;
            }
            if (mModel.mpBtpAnm->init(modelData, anm, 1, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1) == 0) {
                return FALSE;
            } else {
                r28 |= 0x04020000;
            }
        }

        if (mModel.mID.mBtkID != -1) {
            mModel.mpBtkAnm = new mDoExt_btkAnm();
            if (mModel.mpBtkAnm == NULL) {
                return FALSE;
            }
            J3DAnmTextureSRTKey* anm = (J3DAnmTextureSRTKey*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mBtkID);
            if (anm == NULL) {
                return TRUE;
            }
            if (mModel.mpBtkAnm->init(modelData, anm, 1, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1) == 0) {
                return FALSE;
            } else if (mModel.mID.mBtkID & 0x10000000) {
                r28 |= 0x1200;
            } else {
                r28 |= 0x200;
            }
        }

        if (mModel.mID.mBrkID != -1) {
            mModel.mpBrkAnm = new mDoExt_brkAnm();
            if (mModel.mpBrkAnm == NULL) {
                return FALSE;
            }
            J3DAnmTevRegKey* anm = (J3DAnmTevRegKey*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mBrkID);
            if (anm == NULL) {
                return TRUE;
            }
            if (mModel.mpBrkAnm->init(modelData, anm, 1, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1) == 0) {
                return FALSE;
            }
        }

        if (mModel.mID.mBckID == -1) {
            mModel.mpMorf = NULL;
            mModel.mpModel = mDoExt_J3DModel__create(modelData, 0x80000, r28);
            if (mModel.mpModel == NULL) {
                return FALSE;
            }
        } else {
            J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mModel.mID.mBckID);
            if (anm == NULL) {
                return FALSE;
            }
            mModel.mpMorf = new mDoExt_McaMorf(
                modelData, NULL, NULL, anm, J3DFrameCtrl::EMode_NULL,
                1.0f, 0, -1, 1, NULL, 0x80000, r28);
            if (mModel.mpMorf == NULL || mModel.mpMorf->getModel() == NULL) {
                return FALSE;
            }
            mModel.mpModel = mModel.mpMorf->getModel();
            if (!awaCheck(mModel.mpModel)) {
                return FALSE;
            }
        }

        if (field_0x29c == 3) {
            mModel.mpInvisibleModel = new mDoExt_invisibleModel();
            if (mModel.mpInvisibleModel == NULL) {
                return FALSE;
            }
            if (!mModel.mpInvisibleModel->create(mModel.mpModel, 1)) {
                return FALSE;
            }
        } else {
            mModel.mpInvisibleModel = NULL;
        }

        if (mModel.mID.mShadowID != -1) {
            mModel.mpShadow = new daWwDemo00_shadow_c();
            if (mModel.mpShadow == NULL) {
                return FALSE;
            }
            mModel.mpModel->calc();
            setShadowSize();
        }

        mModel.mBgc = new daWwDemo00_bgc_c();
        if (mModel.mBgc == NULL) {
            return FALSE;
        }
        mModel.mBgc->mGndChk.OffWall();
    }

    if (mModel.mID.mPlightID != -1) {
        mModel.mpPlight = new DEMO_PLIGHT();
        if (mModel.mpPlight == NULL) {
            return FALSE;
        }
        light_data_s* light_data = &l_lightData[mModel.mID.mPlightID - 1];
        dExtWwDemo00_plight_entry(mModel.mpPlight, &current.pos, mModel.mID.mPlightID, light_data->m01);
    }

    return TRUE;
}

// ============================================================
// §271 standby — wait for a shape, then build the heap and drive
// ============================================================
BOOL daWwDemo00_c::actStandby(dDemo_actor_c* act) {
    if (mNextID.mShapeID != -1 || mNextID.mPlightID != -1) {
        mModel.mID = mNextID;
        if (fopAcM_entrySolidHeap(this, createHeapCallBack, 0x4000) != 0) {
            if (mModel.mpModel != NULL) {
                setBaseMtx();
                fopAcM_SetMtx(this, mModel.mpModel->getBaseTRMtx());
                act->setModel(mModel.mpModel);
                if (mModel.mpMorf != NULL)
                    act->setAnmFrameMax(mModel.mpMorf->getEndFrame());
            }

            setAction(&daWwDemo00_c::actPerformance);
        }
    }

    return TRUE;
}

// ============================================================
// §271 performance — apply the storyboard's per-frame anim ids + frame
// ============================================================
BOOL daWwDemo00_c::actPerformance(dDemo_actor_c* actor) {
    f32 fVar1;
    if (mModel.mID.mShapeID != mNextID.mShapeID || mModel.mID.mPlightID != mNextID.mPlightID) {
        mModel.reset();
        setAction(&daWwDemo00_c::actLeaving);
    } else if (mModel.mpModel != NULL) {
        if (mModel.mpMorf != NULL && mModel.mID.mBckID != mNextID.mBckID) {
            J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mNextID.mBckID);
            if (anm == NULL) {
                return TRUE;
            }

            fVar1 = 0.0f;
            if (actor->checkEnable(dDemo_actor_c::ENABLE_ANM_FRAME_e)) {
                fVar1 = actor->getAnmTransition();
            }
            mModel.mpMorf->setAnm(anm, -1, fVar1, 1.0f, 0.0f, -1.0f, NULL);
            mModel.mID.mBckID = mNextID.mBckID;
        }

        if (mModel.mID.mBtpID != mNextID.mBtpID) {
            J3DAnmTexPattern* anmTexPattern = (J3DAnmTexPattern*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mNextID.mBtpID);
            if (anmTexPattern == NULL) {
                return TRUE;
            }

            mModel.mpBtpAnm->init(mModel.mpModel->getModelData(), anmTexPattern, 1, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1);
            mModel.mID.mBtpID = mNextID.mBtpID;
        }

        if (mModel.mID.mBtkID != mNextID.mBtkID) {
            J3DAnmTextureSRTKey* key = (J3DAnmTextureSRTKey*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mNextID.mBtkID);
            if (key == NULL) {
                return TRUE;
            }

            int btkAttr = (mModel.mID.mBtkID & 0x10000000) != 0 ? J3DFrameCtrl::EMode_LOOP : J3DFrameCtrl::EMode_NULL;
            s16 btkStart = (mModel.mID.mBtkID & 0x10000000) != 0 ? (s16)mModel.mpBtkAnm->getFrame() : 0;
            mModel.mpBtkAnm->init(mModel.mpModel->getModelData(), key, 1, btkAttr, 1.0f, btkStart, -1);
            mModel.mID.mBtkID = mNextID.mBtkID;
        }

        if (mModel.mID.mBrkID != mNextID.mBrkID) {
            J3DAnmTevRegKey* anmTev = (J3DAnmTevRegKey*)dComIfG_getObjectIDRes(dStage_roomControl_c::getDemoArcName(), (u16)mNextID.mBrkID);
            if (anmTev == NULL) {
                return TRUE;
            }

            int brkAttr = (mModel.mID.mBrkID & 0x10000000) != 0 ? J3DFrameCtrl::EMode_LOOP : J3DFrameCtrl::EMode_NULL;
            s16 brkStart = (mModel.mID.mBrkID & 0x10000000) != 0 ? (s16)mModel.mpBrkAnm->getFrame() : 0;
            mModel.mpBrkAnm->init(mModel.mpModel->getModelData(), anmTev, 1, brkAttr, 1.0f, brkStart, -1);
            mModel.mID.mBrkID = mNextID.mBrkID;
        }

        // §271 4-arg donor call adapted to the port's 8-arg dDemo_setDemoData.
        dDemo_setDemoData(this, (u8)(dDemo_actor_c::ENABLE_TRANS_e | dDemo_actor_c::ENABLE_ROTATE_e | dDemo_actor_c::ENABLE_ANM_e), NULL, NULL, 0, NULL, 0, 0);
        if (mModel.mBgc != NULL) {
            cXyz sp70(current.pos.x, current.pos.y + 100.0f, current.pos.z);
            mModel.mBgc->mGndChk.SetPos(&sp70);
            mModel.mBgc->mGroundY = dComIfG_Bgsp().GroundCross(&mModel.mBgc->mGndChk);
            field_0x29e = 1;
        }

        setBaseMtx();

        if (actor->checkEnable(dDemo_actor_c::ENABLE_ANM_FRAME_e)) {
            fVar1 = actor->getAnmFrame();
            if (fVar1 > 1.0f) {
                fVar1 -= 1.0f;
                if (mModel.mpMorf != NULL) {
                    mModel.mpMorf->setFrame(fVar1);
                    mModel.mpMorf->play(
                        &current.pos,
                        mModel.mBgc != NULL && field_0x29e != 0 &&
                        fabsf(mModel.mBgc->mGroundY - current.pos.y) < 20.0f ?
                            dComIfG_Bgsp().GetMtrlSndId(mModel.mBgc->mGndChk) : 0,
                        dComIfGp_getReverb(dComIfGp_roomControl_getStayNo()));
                }

                if (mModel.mpBtpAnm != NULL) {
                    mModel.mpBtpAnm->setFrame(fVar1);
                    mModel.mpBtpAnm->play();
                }

                if (mModel.mpBtkAnm != NULL) {
                    if ((mModel.mID.mBtkID & 0x10000000) == 0) {
                        mModel.mpBtkAnm->setFrame(fVar1);
                    }
                    mModel.mpBtkAnm->play();
                }

                if (mModel.mpBrkAnm != NULL) {
                    if ((mModel.mID.mBrkID & 0x10000000) == 0) {
                        mModel.mpBrkAnm->setFrame(fVar1);
                    }
                    mModel.mpBrkAnm->play();
                }
            } else {
                if (mModel.mpMorf != NULL) {
                    mModel.mpMorf->setFrame(fVar1);
                }

                if (mModel.mpBtpAnm != NULL) {
                    mModel.mpBtpAnm->setFrame(fVar1);
                }

                if (mModel.mpBtkAnm != NULL) {
                    if ((mModel.mID.mBtkID & 0x10000000) == 0) {
                        mModel.mpBtkAnm->setFrame(fVar1);
                    } else {
                        mModel.mpBtkAnm->play();
                    }
                }

                if (mModel.mpBrkAnm != NULL) {
                    if ((mModel.mID.mBrkID & 0x10000000) == 0) {
                        mModel.mpBrkAnm->setFrame(fVar1);
                    } else {
                        mModel.mpBrkAnm->play();
                    }
                }
            }
        } else if (mModel.mpMorf != NULL) {
            mModel.mpMorf->play(&current.pos, 0, 0);
        } else if (mModel.mpBtpAnm != NULL) {
            mModel.mpBtpAnm->play();
        } else if (mModel.mpBtkAnm != NULL) {
            mModel.mpBtkAnm->play();
        } else if (mModel.mpBrkAnm != NULL) {
            mModel.mpBrkAnm->play();
        }

        if (actor->checkEnable(dDemo_actor_c::ENABLE_SCALE_e)) {
            scale = actor->getScale();
        }

        if (mModel.mpPlight != NULL) {
            light_data_s* light_data = &l_lightData[mModel.mID.mPlightID - 1];
            cXyz sp1C = current.pos;
            if (light_data->useJoint == true) {
                s32 jno = mModel.mpModel->getModelData()->getJointName()->getIndex(light_data->jointName);
                mDoMtx_multVecZero(mModel.mpModel->getAnmMtx(jno), &sp1C);
            }
            dExtWwDemo00_plight_execute(mModel.mpPlight, &sp1C);
        }
    } else if (mModel.mpPlight != NULL) {
        dDemo_setDemoData(this, (u8)dDemo_actor_c::ENABLE_TRANS_e, NULL, NULL, 0, NULL, 0, 0);
        dExtWwDemo00_plight_execute(mModel.mpPlight, &current.pos);
    }

    return TRUE;
}

// ============================================================
// §271 leaving — tear the heap down, back to standby
// ============================================================
BOOL daWwDemo00_c::actLeaving(dDemo_actor_c* act) {
    if (mModel.mpMorf != NULL) {
        dExtWwDemo00_stopZelAnime(mModel.mpMorf);
    }

    fopAcM_DeleteHeap(this);
    setAction(&daWwDemo00_c::actStandby);
    return TRUE;
}

// ============================================================
// §271 draw — render recipe #2/#13/#6: modelCalc() BEFORE btp/btk entry (resets
// the tex pattern) and BEFORE entryDL() (makes the base matrix real). Donor's
// `mpMorf->updateDL()` is absent on the port's McaMorf; it splits into
// modelCalc() + entryDL() (zl1/mount idiom). play()+entryDL() alone = invisible.
// ============================================================
BOOL daWwDemo00_c::draw() {
    if (mModel.mpModel != NULL) {
    // §406: WW feeder (donor type) — see d_kankyo_ww.h; TP never writes TevColor/TevKColor.
        dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
        dKyWw_setLightTevColorType(mModel.mpModel, &tevStr);

        // §271 modelCalc first (base matrix real + tex pattern reset).
        if (mModel.mpMorf != NULL) {
            mModel.mpMorf->modelCalc();
        }

        if (mModel.mpBtpAnm != NULL) {
            mModel.mpBtpAnm->entry(mModel.mpModel->getModelData());
        }
        if (mModel.mpBtkAnm != NULL) {
            mModel.mpBtkAnm->entry(mModel.mpModel->getModelData());
        }
        if (mModel.mpBrkAnm != NULL) {
            mModel.mpBrkAnm->entry(mModel.mpModel->getModelData());
        }

        if (mModel.mpModel->getSkinDeform() != NULL && strcmp(dComIfGp_getStartStageName(), "GTower") == 0) {
            dComIfGd_setListInvisisble();
        }

        if (field_0x29c == 2) {
            dExtWwDemo00_setListMaskOff();
        } else if (field_0x29c == 8) {
            dExtWwDemo00_setListP1();
        } else {
            dComIfGd_setList();
        }

        if (mModel.mpMorf != NULL) {
            mModel.mpMorf->entryDL();
        } else {
            mDoExt_modelUpdateDL(mModel.mpModel);
        }

        if (mModel.mpModel->getSkinDeform() != NULL || field_0x29c == 2 || field_0x29c == 8) {
            dComIfGd_setList();
        }

        // §271 [INFERENCE-NEEDED] demo ground-shadow submission. The port's
        // dComIfGd_setShadow / setSimpleShadow2 signatures diverge from the donor's
        // (extra angle/tex params); shadows are cosmetic and off the bind+render
        // acceptance path, so the submission is deferred. BRIDGE-OWED: wire the
        // port shadow API (dComIfGd_setShadow 13-arg / dDlst_shadowControl) so the
        // puppets cast the demo's soft ground shadow.

        if (mModel.mpBtpAnm != NULL) {
            mModel.mpBtpAnm->remove(mModel.mpModel->getModelData());
        }
        if (mModel.mpBtkAnm != NULL) {
            mModel.mpBtkAnm->remove(mModel.mpModel->getModelData());
        }
        if (mModel.mpBrkAnm != NULL) {
            mModel.mpBrkAnm->remove(mModel.mpModel->getModelData());
        }
    }
    return TRUE;
}

// ============================================================
// §271 execute — pull the storyboard's data channel, then run the action
// ============================================================
BOOL daWwDemo00_c::execute() {
    field_0x29e = 0;

    dDemo_actor_c* demo_actor = dDemo_c::getActor(demoActorID);
    if (demo_actor == NULL) {
        fopAcM_delete(this);
    } else {
        if (demo_actor->checkEnable(dDemo_actor_c::ENABLE_SHAPE_e)) {
            mNextID.mShapeID = demo_actor->getShapeId();
        }
        if (demo_actor->checkEnable(dDemo_actor_c::ENABLE_ANM_e)) {
            mNextID.mBckID = demo_actor->getAnmId();
        }
        if (demo_actor->checkEnable(dDemo_actor_c::ENABLE_UNK_e)) {
            u8 r29 = field_0x29c;
            field_0x29c = dExtWwDemo00_getPrmId(demo_actor->getPrm());
            if (field_0x29c == 4) {
                // §271 [INFERENCE-NEEDED] WW story event-bit channel. The donor
                // writes WW dSv_event_flag_c bits (UNK_2A80 ... COLORS_IN_HYRULE)
                // that do NOT exist in the port's TP-named save; per the WW
                // save-guard doctrine (№81) a donor storyboard must not write TP
                // save memory on a host stage. Parse-and-drop. BRIDGE-OWED: map WW
                // story bits into the port save if these cutscenes must persist.
            } else if (field_0x29c == 5) {
                // §271 [INFERENCE-NEEDED] WW item-get channel (dItemNo_MASTER_SWORD_*
                // / PEARL_* / DELIVERY_BAG absent in the port). Item awards during a
                // WW host cutscene are deferred. Parse-and-drop. BRIDGE-OWED.
            } else if (field_0x29c == 6) {
                dDemo_prm_data* data = demo_actor->getPrm()->getData();
                JStudio::stb::TParseData_fixed<33, TValueIterator_misaligned<s8> > spA4(data);
                if (!spA4.isEnd() && spA4.isValid()) {
                    TValueIterator_misaligned<s8> it = spA4.begin();
                    int argID = *it;
                    dExtWwDemo00_setMonotoneRateSpeed(argID);
                }
            } else if (field_0x29c == 7) {
                dDemo_prm_data* data = demo_actor->getPrm()->getData();
                JStudio::stb::TParseData_fixed<49, TValueIterator_raw<u8> > sp90(data);
                if (!sp90.isEnd() && sp90.isValid()) {
                    TValueIterator_raw<u8> it = sp90.begin();
                    int argID = *it;
                    if (argID < 100) {
                        dComIfGp_getVibration().StartShock(argID, 1, cXyz(0.0f, 1.0f, 0.0f));
                    } else if (argID != 0xFF) {
                        dComIfGp_getVibration().StartQuake(argID - 100, 1, cXyz(0.0f, 1.0f, 0.0f));
                    } else {
                        dComIfGp_getVibration().StopQuake(1);
                    }
                }
            } else if (field_0x29c == 9 || field_0x29c == 10) {
                dDemo_prm_data* data = demo_actor->getPrm()->getData();
                JStudio::stb::TParseData_fixed<33, TValueIterator_raw<s8> > sp7C(data);
                if (!sp7C.isEnd() && sp7C.isValid()) {
                    TValueIterator_raw<s8> it = sp7C.begin();
                    int r4 = *it;
                    if (field_0x29c != r29 || r4 != field_0x29d) {
                        field_0x29d = r4;
                        s8 r5 = 0;
                        ++it;
                        if (!(it == sp7C.end())) {
                            r5 = (int)*it;
                        }
                        if ((s8)r4 == 0) {
                            DuskLog.info("[WwDemo00] §355d ch{} fade OUT {}f", (int)field_0x29c,
                                         (int)r5);
                            mDoGph_gInf_c::startFadeOut(r5);
                        } else {
                            DuskLog.info("[WwDemo00] §355d ch{} fade IN {}f", (int)field_0x29c,
                                         (int)r5);
                            mDoGph_gInf_c::startFadeIn(r5);
                        }

                        JUtility::TColor& fade_color = field_0x29c == 9 ? (JUtility::TColor&)g_blackColor : (JUtility::TColor&)g_saftyWhiteColor;
                        mDoGph_gInf_c::setFadeColor(fade_color);
                    }
                }
            } else {
                // §271 load-bearing model/anim data channel (donor verbatim).
                dDemo_prm_data* data = demo_actor->getPrm()->getData();
                JStudio::stb::TParseData_fixed<51, TValueIterator_misaligned<u32> > sp68(data);
                if (!sp68.isEnd() && sp68.isValid()) {
                    int r5 = -1;
                    TValueIterator_misaligned<u32> it = sp68.begin();
                    while (!(it == sp68.end())) {
                        if (r5 < 0) {
                            r5 = *it;
                        } else {
                            if (r5 == 0) {
                                mNextID.mBtpID = *it;
                            } else if (r5 == 1) {
                                mNextID.mBtkID = *it;
                            } else if (r5 == 2) {
                                mNextID.mPlightID = *it;
                            } else if (r5 == 3) {
                                mNextID.field_0x1C = *it;
                            } else if (r5 == 4) {
                                mNextID.mBrkID = *it;
                            } else if (r5 == 5) {
                                mNextID.mShadowID = *it;
                            } else if (r5 == 6) {
                                mNextID.mBtkID = *it | 0x10000000;
                            } else if (r5 == 7) {
                                mNextID.mBrkID = *it | 0x10000000;
                            }
                            r5 = -1;
                        }
                        it++;
                    }
                }
            }
        }

        action(demo_actor);
    }
    return TRUE;
}

// ============================================================
// §271 actor method table + profile (donor g_profile_DEMO00, renamed WW_DEMO00).
// Registered at slot 0x32B (f_pc_name.h / f_pc_profile_lst .h+.cpp / files.cmake).
// ============================================================
static BOOL daWwDemo00_Draw(daWwDemo00_c* i_this) {
    return i_this->draw();
}

static BOOL daWwDemo00_Execute(daWwDemo00_c* i_this) {
    return i_this->execute();
}

static BOOL daWwDemo00_IsDelete(daWwDemo00_c* i_this) {
    return TRUE;
}

static BOOL daWwDemo00_Delete(daWwDemo00_c* i_this) {
    i_this->~daWwDemo00_c();
    return TRUE;
}

static cPhs_Step daWwDemo00_Create(fopAc_ac_c* i_ac) {
    daWwDemo00_c* i_this = (daWwDemo00_c*)i_ac;
    return i_this->create();
}

static DUSK_CONST actor_method_class l_daWwDemo00_Method = {
    (process_method_func)daWwDemo00_Create,
    (process_method_func)daWwDemo00_Delete,
    (process_method_func)daWwDemo00_Execute,
    (process_method_func)daWwDemo00_IsDelete,
    (process_method_func)daWwDemo00_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_WW_DEMO00 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_WW_DEMO00_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daWwDemo00_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_DEMO00_e,
    /* Actor SubMtd */ &l_daWwDemo00_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
