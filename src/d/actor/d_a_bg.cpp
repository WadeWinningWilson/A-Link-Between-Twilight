/**
 * @file d_a_bg.cpp
 * 
*/

// KIT-LINEAGE: mixed
// KIT-DONOR: per-hunk
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
#include "d/dolzel_rel.h" // IWYU pragma: keep

#include "d/actor/d_a_bg.h"
#include "d/d_com_inf_game.h"
#include "d/d_com_static.h"
#include "d/actor/d_a_grass.h"
#include "d/d_bg_w.h"
#include "d/d_bg_parts.h"
#include "m_Do/m_Do_lib.h"
#include "d/d_demo.h"
#if TARGET_PC
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_save_guard.h"
#include "d/d_bg_s_gnd_chk.h"                    // tale §757 post-Regist self-test
#include "SSystem/SComponent/c_counter.h"        // tale §757 frame stamp
#include "dusk/logging.h"
#endif
#include "JSystem/JKernel/JKRExpHeap.h"
#include "JSystem/JKernel/JKRSolidHeap.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include <cstring>
#include <cstdio>  // tale §898 [P2] stage-latch snprintf

const char* daBg_c::setArcName() {
    static char arcName[32];

    strncpy(arcName, dComIfG_getRoomArcName(fopAcM_GetParam(this)), sizeof(arcName));
    return arcName;
}

static int createMatAnm(J3DModelData* i_modelData, u16 i_materialID) {
    if (i_materialID != 0xFFFF) {
        J3DMaterial* material = i_modelData->getMaterialNodePointer(i_materialID);
        
        if (material->getMaterialAnm() == NULL) {
            J3DMaterialAnm* anm = JKR_NEW J3DMaterialAnm();
            if (anm == NULL) {
                return 0;
            }

            material->setMaterialAnm(anm);
        }
    }

    return 1;
}

#if PLATFORM_GCN
static u8 const lit_3756[12] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
#endif

int daBg_btkAnm_c::create(J3DModelData* i_modelData, J3DAnmTextureSRTKey* i_btk, int i_anmPlay) {
    mpBtk = JKR_NEW mDoExt_btkAnm();
    if (mpBtk == NULL) {
        return 0;
    }

    if (!mpBtk->init(i_modelData, i_btk, i_anmPlay, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1)) {
        return 0;
    }

    J3DAnmTextureSRTKey* btk = mpBtk->getBtkAnm();
    for (u16 i = 0; i < btk->getUpdateMaterialNum(); i++) {
        if (!createMatAnm(i_modelData, btk->getUpdateMaterialID(i))) {
            return 0;
        }
    }

    return 1;
}

void daBg_btkAnm_c::entry(J3DModelData* i_modelData) {
    mpBtk->entry(i_modelData, 0.0f);
    field_0x4 = 0;
}

void daBg_btkAnm_c::play() {
    mpBtk->play();
}

int daBg_brkAnm_c::create(J3DModelData* i_modelData, J3DAnmTevRegKey* i_brk, int i_anmPlay) {
    mpBrk = JKR_NEW mDoExt_brkAnm();

    if (mpBrk == NULL) {
        return 0;
    }

    if (!mpBrk->init(i_modelData, i_brk, i_anmPlay, J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1)) {
        return 0;
    }

    J3DAnmTevRegKey* brk = mpBrk->getBrkAnm();
    for (u16 i = 0; i < brk->getCRegUpdateMaterialNum(); i++) {
        if (!createMatAnm(i_modelData, brk->getCRegUpdateMaterialID(i))) {
            return 0;
        }
    }

    for (u16 i = 0; i < brk->getKRegUpdateMaterialNum(); i++) {
        if (!createMatAnm(i_modelData, brk->getKRegUpdateMaterialID(i))) {
            return 0;
        }
    }

    return 1;
}

void daBg_brkAnm_c::entry(J3DModelData* i_modelData) {
    mpBrk->entry(i_modelData, 0.0f);
    field_0x4 = 0;
}

void daBg_brkAnm_c::play() {
    mpBrk->play();
}

static int checkCreateHeap(fopAc_ac_c* i_this) {
    daBg_c* a_this = (daBg_c*)i_this;
    return a_this->createHeap();
}

int daBg_c::createHeap() {
    static char l_modelName[6][11] = {"model.bmd", "model1.bmd", "model2.bmd", "model3.bmd", "model4.bmd", "model5.bmd"};
    static char l_modelName2[6][11] = {"model.bdl", "model1.bdl", "model2.bdl", "model3.bdl", "model4.bdl", "model5.bdl"};
    static char l_btkName[6][11] = {"model.btk", "model1.btk", "model2.btk", "model3.btk", "model4.btk", "model5.btk"};
    static char l_brkName[6][11] = {"model.brk", "model1.brk", "model2.brk", "model3.brk", "model4.brk", "model5.brk"};

    const char* arcName = setArcName();
    int roomNo = fopAcM_GetParam(this);
    daBg_Part* bgPart = mBgParts;

    for (int i = 0; i < 6; i++) {
        J3DModelData* modelData = (J3DModelData*)dComIfG_getStageRes(arcName, l_modelName[i]);
        if (modelData == NULL) {
            modelData = (J3DModelData*)dComIfG_getStageRes(arcName, l_modelName2[i]);
        }

#if TARGET_PC
        // ====================================================================
        // tale §898 [P1] (CALLS row 68 chain): daBg's model FETCH receipt — the
        // step between the seam publish (measured clean for A_mori) and draw.
        // WW hosts only; model[0] NULL here = the room model never reached the
        // room actor and the invisible-room fault is THIS seam. Sight-only.
        // ====================================================================
        {
            const char* sn898 = dComIfGp_getStartStageName();
            if (sn898 != NULL && dExtWwSave_isWwHostStage(sn898)) {
                if (modelData != NULL) {
                    DuskLog.info("[daBg] §898-P1 room{} model[{}] fetched data={}", roomNo, i,
                                 (void*)modelData);
                } else if (i == 0) {
                    DuskLog.warn("[daBg] §898-P1 room{} model[0] NULL from arc '{}' — room "
                                 "model never reached daBg create",
                                 roomNo, arcName);
                }
            }
        }
#endif

        if (modelData != NULL) {
            mDoExt_setupStageTexture(modelData);
            u32 modelFlags = 0x11000084;

            for (u16 j = 0; j < modelData->getMaterialNum(); j++) {
                J3DMaterial* material = modelData->getMaterialNodePointer(j);
                material->setMaterialAnm(NULL);
            }

            J3DAnmTextureSRTKey* btk =
                (J3DAnmTextureSRTKey*)dComIfG_getStageRes(arcName, l_btkName[i]);
            if (btk != NULL) {
                bgPart->btk = JKR_NEW daBg_btkAnm_c();
                if (bgPart->btk == NULL) {
                    return 0;
                }

                if (!bgPart->btk->create(modelData, btk, TRUE)) {
                    return 0;
                }

                modelFlags |= 0x1200;
            } else {
                bgPart->btk = NULL;
            }
            bgPart->btk_speed = -1.0f;

            J3DAnmTevRegKey* brk = (J3DAnmTevRegKey*)dComIfG_getStageRes(arcName, l_brkName[i]);
            if (brk != NULL) {
                bgPart->brk = JKR_NEW daBg_brkAnm_c();
                if (bgPart->brk == NULL) {
                    return 0;
                }

                if (!bgPart->brk->create(modelData, brk, TRUE)) {
                    return 0;
                }
            } else {
                bgPart->brk = NULL;
            }

            for (u16 j = 0; j < modelData->getMaterialNum(); j++) {
                const char* name;
                JUTNameTab* nametab = modelData->getMaterialName();
                J3DMaterial* material = modelData->getMaterialNodePointer(j);

                name = nametab->getName(j);

                if (name[3] == 'M' && name[4] == 'A') {
                    if (!memcmp(&name[5], "00", 2)) {
                        modelFlags |= 0x1200;
                    } else if (!memcmp(&name[5], "01", 2)) {
                        modelFlags |= 0x20000000;
                    } else if (!memcmp(&name[5], "12", 2) || !memcmp(&name[5], "18", 2)) {
                        field_0x5f0 = 1;
                    }
                }
            }

            bgPart->model = mDoExt_J3DModel__create(modelData, 0, modelFlags);
            if (bgPart->model == NULL) {
                return 0;
            }

            bgPart->tevstr = JKR_NEW dKy_tevstr_c();
            if (bgPart->tevstr == NULL) {
                return 0;
            }

            dKy_tevstr_init(bgPart->tevstr, roomNo, 0xFF);
        }

        bgPart++;
    }

    // №257: WW room-lane mounts own collision (GLOBAL_e + setBgW). A stub
    // room.dzb here dual-registers at PRIORITY_0 and steals WallCorrect from
    // the mount ladder/ledge polys (Verdict 2: wallCode=4 wallHit=0).
#if TARGET_PC
    {
        const char* stage = dComIfGp_getStartStageName();
        if (dExtWwSave_isWwHostStage(stage) && dExtNpcMount_isRoomLaneRoom(roomNo)) {
            mpBgW = NULL;
            mpKCol = NULL;
            DuskLog.info("[daBg] №257 skip room{} collision — room-lane mount owns BgW", roomNo);
            return 1;
        }
    }
#endif

    cBgD_t* dzb = (cBgD_t*)dComIfG_getStageRes(arcName, "room.dzb");
    if (dzb != NULL) {
        mpKCol = NULL;
        mpBgW = JKR_NEW dBgW();
        if (mpBgW == NULL) {
            return 0;
        }

        if (mpBgW->Set(dzb, cBgW::GLOBAL_e, NULL)) {
            return 0;
        }

        dStage_roomControl_c::setBgW(roomNo, (dBgW_Base*)mpBgW);
        mpBgW->SetPriority(dBgW_Base::PRIORITY_0);
    } else {
        void* kcl = dComIfG_getStageRes(arcName, "room.kcl");
        void* plc = dComIfG_getStageRes(arcName, "room.plc");

        if (kcl != NULL && plc != NULL) {
            mpBgW = NULL;
            mpKCol = JKR_NEW dBgWKCol();
            if (mpKCol == NULL) {
                return 0;
            }

            mpKCol->create(kcl, plc);
            mpKCol->SetPriority(dBgW_Base::PRIORITY_0);
        } else {
            mpBgW = NULL;
            mpKCol = NULL;
        }
    }

    return 1;
}

#if DEBUG
static void dummy() {
    GXColor color = {0xC0, 0x00, 0x00, 0x00};
}
#endif

daBg_c::~daBg_c() {
    int roomNo = fopAcM_GetParam(this);

    dBgp_c* bgp = dStage_roomControl_c::getBgp(roomNo);
    if (bgp != NULL) {
        bgp->releaseBg();
    }

    if (heap != NULL && mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
        dStage_roomControl_c::setBgW(roomNo, NULL);
    }

    if (heap != NULL && mpKCol != NULL) {
        dComIfG_Bgsp().Release(mpKCol);
        dStage_roomControl_c::setBgW(roomNo, NULL);
    }

    daGrass_c::deleteRoomGrass(roomNo);
    daGrass_c::deleteRoomFlower(roomNo);
    daSus_c::reset(roomNo);
    dComIfGp_roomControl_offStatusFlag(roomNo, 0x10);
}

static int daBg_Draw(daBg_c* i_this) {
    return i_this->draw();
}

int daBg_c::draw() {
    dScnKy_env_light_c* kankyo = dKy_getEnvlight();

    int roomNo = fopAcM_GetParam(this);
    daBg_Part* bgPart = mBgParts;
    J3DModel* bg_model;

#if TARGET_PC
    // ========================================================================
    // tale §898 [P2] (CALLS row 68 chain): FIRST-DRAW dispatch receipt, one-shot
    // per (stage, room) on WW hosts. Present with models bound = the whole
    // pipeline ran and the fault is visual (cull/far/lighting); absent while
    // [P1] fetched = draw never dispatched. Latch resets on stage change so
    // repeated room numbers across stages stay distinct. Sight-only.
    // ========================================================================
    {
        static char s_drawStage[12] = {0};
        static u64 s_drawSeen = 0;
        const char* sn898 = dComIfGp_getStartStageName();
        if (sn898 != NULL && dExtWwSave_isWwHostStage(sn898) && roomNo >= 0 && roomNo < 64) {
            if (std::strncmp(s_drawStage, sn898, sizeof(s_drawStage) - 1) != 0) {
                std::snprintf(s_drawStage, sizeof(s_drawStage), "%s", sn898);
                s_drawSeen = 0;
            }
            if (!(s_drawSeen & (1ull << roomNo))) {
                s_drawSeen |= 1ull << roomNo;
                int nm = 0;
                for (int k = 0; k < 6; k++) {
                    if (mBgParts[k].model != NULL) {
                        nm++;
                    }
                }
                DuskLog.info("[daBg] §898-P2 room{} FIRST DRAW dispatch — models bound={}/6",
                             roomNo, nm);
            }
        }
    }
#endif

    u8 spA;
    u8 sp9;
    u8 sp8 = 0;
    int sp38 = 0;

    dDlst_window_c* sp34 = dComIfGp_getWindow(0);
    camera_process_class* sp30 = dComIfGp_getCamera(sp34->getCameraID());

    dComIfGd_setListBG();
    mDoLib_clipper::changeFar(1000000.0f);

    J3DModelData* modelData;
    for (int i = 0; i < 6; i++) {
        sp8 = 0;
        spA = 0;
        sp9 = 0;

        bg_model = bgPart->model;
        
        if (bg_model != NULL) {
            modelData = bg_model->getModelData();

            if (bgPart->btk != NULL) {
                bgPart->btk->entryFrame();
            }

            if (bgPart->brk != NULL) {
                if (field_0x5f0 == 9) {
                    bgPart->brk->entryFrame(bgPart->brk->getEndFrame());
                } else {
                    bgPart->brk->entryFrame();
                }
            }

            bg_model->calc();

            // ================================================================
            // §682: NO PER-SHAPE CULLING for donor rooms — donor granularity.
            //
            // The §679/§679b probes proved the per-shape clip verdicts here
            // are self-consistent (independent plane math agreed on every
            // specimen) yet the island still visibly draw/undraws — i.e. the
            // view matrix present at draw-list-BUILD time is not the view the
            // frame finally renders (interpolation/pass-ordering skew). TP
            // never sees it because its room shapes are small; WW's island-
            // sized shapes turn the skew into whole-terrain holes.
            //
            // The donor's own daBg does NOT per-shape-cull room models — it
            // clips whole models only (WW d_a_bg.cpp:277 `clip(model)`). So
            // on WW host stages every shape stays shown, exactly the donor's
            // granularity; the receiver's own stages keep the receiver's
            // per-shape behavior byte-for-byte.
            // ================================================================
            // KIT-DONOR-HUNK: d/actor/d_a_bg.cpp MatchingFor
            const char* wwStage = dComIfGp_getStartStageName();
            const bool wwHost = wwStage != NULL && dExtWwSave_isWwHostStage(wwStage);

            for (u16 j = 0; j < modelData->getShapeNum(); j++) {
                J3DShape* shape = modelData->getShapeNodePointer(j);

                if (!wwHost && mDoLib_clipper::clip(j3dSys.getViewMtx(), (Vec*)shape->getMin(),
                                                    (Vec*)shape->getMax())) {
                    shape->hide();
                } else {
                    shape->show();
                }
            }
            // KIT-DONOR-HUNK-END

            static int l_tevStrType[6] = {32, 33, 34, 35, 35, 32};
            g_env_light.settingTevStruct(l_tevStrType[i], NULL, bgPart->tevstr);
            g_env_light.setLightTevColorType_MAJI(bg_model, bgPart->tevstr);
            dKy_bg_MAxx_proc(bg_model);

            if (bg_model != NULL) {
                modelData = bg_model->getModelData();

                for (u16 j = 0; j < modelData->getMaterialNum(); j++) {
                    const char* name;
                    J3DMaterial* mat;
                    JUTNameTab* nametab;

                    mat = modelData->getMaterialNodePointer(j);
                    nametab = modelData->getMaterialName();
                    name = nametab->getName(j);

                    if (!memcmp(&name[3], "MA12", 4)) {
                        if (g_env_light.wether_pat1 == 6) {
                            field_0x5f0 = 0;
                        }
                    } else if (!memcmp(&name[3], "MA18", 4)) {
                        if (dDemo_c::getFrame() >= 1118) {
                            field_0x5f0 = 0;
                        }

                        /* Main Event - Get shadow crystal (can now transform) */
                        if (dComIfGs_isEventBit(dSv_event_flag_c::M_077)) {
                            field_0x5f0 = 9;
                        }
                    } else if (!memcmp(&name[3], "MA15", 4)) {
                        if (dComIfGs_BossLife_public_Get() != -1) {
                            field_0x5f1 = dComIfGs_BossLife_public_Get() + 1;
                        } else {
                            field_0x5f1 = 0;
                        }
                    } else if (!memcmp(&name[3], "MA09", 4)) {
                        bgPart->btk_speed =
                            1.0f - (1.0f - g_env_light.mWaterSurfaceShineRate) * 0.9f;
                    } else if (!memcmp(&name[3], "MA05", 4)) {
                        bgPart->tevstr->Material_id |= (u8)j;
                    }

                    if (!strcmp(dComIfGp_getStartStageName(), "F_SP127") ||
                        !strcmp(dComIfGp_getStartStageName(), "R_SP127"))
                    {
                        if (!memcmp(&name[3], "MA00_Enkei_Tree_Color", 21) ||
                            !memcmp(&name[3], "MA00_Gake", 9) || !memcmp(&name[3], "MA00_Kusa", 9))
                        {
                            J3DGXColorS10 colorS10;
                            J3DGXColor color;

                            GXColorS10 sp50;
                            switch (g_env_light.fishing_hole_season) {
                            case 2:
                                sp50.r = -3;
                                sp50.g = 0;
                                sp50.b = -4;
                                break;
                            case 3:
                                sp50.r = 0;
                                sp50.g = -10;
                                sp50.b = -13;
                                break;
                            case 4:
                                sp50.r = 18;
                                sp50.g = 17;
                                sp50.b = 25;
                                break;
                            default:
                                sp50.r = 0;
                                sp50.g = 0;
                                sp50.b = 0;
                                break;
                            }

#if DEBUG
                            sp50.a = 0xFF;
                            if (g_kankyoHIO.navy.fish_pond_colreg_adjust_ON) {
                                sp50 = g_kankyoHIO.navy.fish_pond_colreg_c0;
                            }
#endif

                            f32 var_f31 = bgPart->tevstr->AmbCol.r / 10.0f;
                            var_f31 *= var_f31;
                            if (var_f31 > 1.0f) {
                                var_f31 = 1.0f;
                            }
                            colorS10.r = sp50.r * var_f31;

                            var_f31 = bgPart->tevstr->AmbCol.g / 10.0f;
                            var_f31 *= var_f31;
                            if (var_f31 > 1.0f) {
                                var_f31 = 1.0f;
                            }
                            colorS10.g = sp50.g * var_f31;

                            var_f31 = bgPart->tevstr->AmbCol.b / 10.0f;
                            var_f31 *= var_f31;
                            if (var_f31 > 1.0f) {
                                var_f31 = 1.0f;
                            }
                            colorS10.b = sp50.b * var_f31;
                            colorS10.a = 255;
                            mat->setTevColor(0, &colorS10);

                            color.r = 0;
                            color.g = 0;
                            color.b = 0;
                            color.a = 255;
                            mat->setTevKColor(0, &color);
                        }
                    }
                }
            }

            mDoExt_modelEntryDL(bg_model);
            dComIfGd_setListBG();
        }

        bgPart++;
    }

    dComIfGd_setList();
    g_env_light.settingTevStruct(0x10, NULL, dComIfGp_roomControl_getTevStr(roomNo));

    dBgp_c* bgp = dStage_roomControl_c::getBgp(roomNo);
    if (bgp != NULL) {
        bgp->draw(this);
    }

    return 1;
}

int daBg_c::execute() {
    daBg_Part* bgPart = mBgParts;

    for (int i = 0; i < 6; i++) {
        if (bgPart->btk != NULL) {
            if (field_0x5f1 != 0) {
                f32 speed = (field_0x5f1 - 1) / 100.0f;
                bgPart->btk->playspeed(speed);
            }

            if (bgPart->btk_speed >= 0.0f) {
                bgPart->btk->playspeed(bgPart->btk_speed);
            }
            bgPart->btk->play();
        }

        if (bgPart->brk != NULL && field_0x5f0 == 0) {
            bgPart->brk->play();
        }

        bgPart++;
    }

    return 1;
}

static int daBg_Execute(daBg_c* i_this) {
    return i_this->execute();
}

int daBg_c::isDelete() {
    return 1;
}

static int daBg_IsDelete(daBg_c* i_this) {
    return i_this->isDelete();
}

static int daBg_Delete(daBg_c* i_this) {
    i_this->~daBg_c();
    return 1;
}

static int daBg_Create(fopAc_ac_c* i_this) {
    daBg_c* a_this = (daBg_c*)i_this;
    int rt = a_this->create();
    return rt;
}

int daBg_c::create() {
    int roomNo = fopAcM_GetParam(this);
    field_0x5f0 = 0;
    field_0x5f1 = 0;
    dBgp_c* bgp = dStage_roomControl_c::getBgp(roomNo);

    if (this->heap == NULL) {
        fopAcM_ct(this, daBg_c);

        home.roomNo = roomNo;
        current.roomNo = roomNo;

        JKRExpHeap* room_heap = dStage_roomControl_c::getMemoryBlock(roomNo);
        if (room_heap != NULL) {
            this->heap = JKRCreateSolidHeap(-1, room_heap, false);
            JUT_ASSERT(471, heap != NULL);
            JKRHEAP_NAME(heap, "d_a_bg Anms");

            JKRHeap* old = mDoExt_setCurrentHeap(this->heap);
            int rt = createHeap();
            JUT_ASSERT(476, rt == 1);

            mDoExt_setCurrentHeap(old);
            this->heap->adjustSize();
        } else {
            u32 size = 0x20040;
            size |= 0x80000000;
            if (!fopAcM_entrySolidHeap(this, checkCreateHeap, size)) {
                return cPhs_ERROR_e;
            }
        }

        daBg_Part* bgPart = mBgParts;
        for (int i = 0; i < 6; i++) {
            if (bgPart->model != NULL) {
                J3DModelData* modelData = bgPart->model->getModelData();

                if (bgPart->btk != NULL) {
                    bgPart->btk->entry(modelData);
                }

                if (bgPart->brk != NULL) {
                    bgPart->brk->entry(modelData);
                }
            }
            bgPart++;
        }

        J3DModelData* modelData;
        f32 transX;
        f32 transY;
        s16 angle;
        if (dComIfGp_getMapTrans(roomNo, &transX, &transY, &angle)) {
            daBg_Part* bgPart = mBgParts;
            J3DModel* model;
            for (int i = 0; i < 6; i++) {
                model = bgPart->model;

                if (model != NULL) {
                    mDoMtx_stack_c::transS(transX, 0.0f, transY);
                    mDoMtx_stack_c::YrotM(angle);
                    model->setBaseTRMtx(mDoMtx_stack_c::get());

                    modelData = model->getModelData();
                    for (u16 j = 0; j < modelData->getMaterialNum(); j++) {
                        JUTNameTab* nametab = modelData->getMaterialName();
                        const char* name = nametab->getName(j);

                        if (name[3] == 'M' && name[4] == 'A' && name[5] == '0' && name[6] == '8') {
                            J3DMaterial* material = modelData->getMaterialNodePointer(j);
                            J3DShape* shape = material->getShape();
                            shape->setTexMtxLoadType(0x2000);
                        }
                    }
                }
                bgPart++;
            }
        }

        if (mpBgW != NULL) {
            if (dComIfG_Bgsp().Regist(mpBgW, this)) {
                return cPhs_ERROR_e;
            }
#if TARGET_PC
            // ================================================================
            // tale §757 (the invisible floor, tale §755): the ground query at
            // LinkRM/0's spawn returned gndCode=-1 over four donor floor tris
            // at Y=0, and the log cannot say whether this Regist ever ran or
            // whether the registered bg answers queries at all. One-shot per
            // WW-host room: Regist receipt + a DIRECT GndChk down the spawn
            // column immediately after registration. If the self-test HITS,
            // the defect is timing (player spawned before this line) or
            // query-side flags; if it MISSES over present geometry, it is
            // registration-side. Instrument only — DN-10-clean.
            // ================================================================
            {
                const char* sn757 = dComIfGp_getStartStageName();
                if (sn757 != NULL && dExtWwSave_isWwHostStage(sn757)) {
                    dBgS_GndChk chk;
                    cXyz probe(-255.0f, 500.0f, 1125.0f);
                    fopAc_ac_c* pl757 = dComIfGp_getPlayer(0);
                    if (pl757 != NULL) {
                        probe.set(pl757->current.pos.x, pl757->current.pos.y + 500.0f,
                                  pl757->current.pos.z);
                    }
                    chk.SetPos(&probe);
                    const f32 hit = dComIfG_Bgsp().GroundCross(&chk);
                    DuskLog.warn("[daBg] §757 room{} BgW REGISTERED gFrm={} — post-Regist "
                                 "GndChk at ({:.0f},{:.0f},{:.0f}) -> {}",
                                 roomNo, (int)g_Counter.mCounter0, probe.x, probe.y, probe.z,
                                 hit <= -1e9f ? "MISS (-inf)" : "HIT");
                }
            }
#endif
        }

        if (mpKCol != NULL) {
            if (dComIfG_Bgsp().Regist(mpKCol, this)) {
                return cPhs_ERROR_e;
            }
        }

        dKy_tevstr_c* tevstr = dComIfGp_roomControl_getTevStr(roomNo);
        dKy_tevstr_init(tevstr, roomNo, 0xFF);

        if (bgp != NULL) {
            bgp->registBg(this);
        }
    }

    if (bgp != NULL) {
        if (!bgp->execute(false)) {
            return cPhs_INIT_e;
        }
    }

    dComIfGp_roomControl_onStatusFlag(roomNo, 0x10);
    OS_REPORT("<BG> room%d\n", roomNo);

#if TARGET_PC
    draw_interp_frame = true;
#endif

    return cPhs_COMPLEATE_e;
}

static DUSK_CONST actor_method_class l_daBg_Method = {
    (process_method_func)daBg_Create,
    (process_method_func)daBg_Delete,
    (process_method_func)daBg_Execute,
    (process_method_func)daBg_IsDelete,
    (process_method_func)daBg_Draw,
};

DUSK_PROFILE actor_process_profile_definition2 DUSK_CONST g_profile_BG = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_BG_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daBg_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_BG_e,
    /* Actor SubMtd */ &l_daBg_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_NOPAUSE_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
    /* Unknown      */ 0,
};
