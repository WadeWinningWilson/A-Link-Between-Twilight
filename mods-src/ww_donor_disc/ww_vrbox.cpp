// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_vrbox.cpp MatchingFor (daVrbox_Draw / color_set /
//            Create, vr_sky.bdl) and d_a_vrbox2.cpp Draw / color_set /
//            solidHeapCB (vr_back_cloud / vr_kasumi_mae / vr_uso_umi).
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Compiled against vanilla headers. Spawn is the donor dStage_Create check
// (vr_sky.bdl present) with the proc name translated to our above-enum index.
// vrbox2 wind UV scroll is donor daVrbox2_color_set (per-material speeds).
// MISC-stage wind special-cases omitted (sea is not MISC). Dome UpdateDL is
// the donor call, deferred to sky-opa flush (drawOpaDrawList) so it does not
// share the PC presentation walk with daBg. Clouds stay packet-free until
// the dome is stable.

#include "ww_vrbox.h"

#include "ww_kankyo.h"
#include "ww_kankyo_wind.h"
#include "ww_sky.h"

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_rain.h"
#include "f_op/f_op_camera_mng.h"
#include "SSystem/SComponent/c_m3d.h"
#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphBase/J3DMatBlock.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DStruct.h"
#include "JSystem/J3DGraphBase/J3DTexture.h"
#include "JSystem/J3DGraphBase/J3DVertex.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "f_op/f_op_view.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "m_Do/m_Do_mtx.h"
#include <dolphin/gx.h>

#include <mods/api.h>
#include <mods/svc/hook.h>
#include <mods/svc/log.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

extern const LogService* s_log;

namespace {

const unsigned int kActorBaseSize = 0x668;  // same measured sizeof(fopAc_ac_c)

struct VrboxMembers {
    J3DModel* model;
};

struct Vrbox2Members {
    J3DModel* backCloud;
    J3DModel* kasumiMae;
    J3DModel* usoUmi;
};

VrboxMembers* vrboxM(void* self) {
    return reinterpret_cast<VrboxMembers*>(static_cast<char*>(self) + kActorBaseSize);
}

Vrbox2Members* vrbox2M(void* self) {
    return reinterpret_cast<Vrbox2Members*>(static_cast<char*>(self) + kActorBaseSize);
}

typedef void* (*FnGetStageRes)(const char*);
typedef void* (*FnModelCreate)(void*, unsigned int, unsigned int);
typedef bool (*FnEntrySolidHeap)(void*, int (*)(void*), unsigned int);
typedef void (*FnModelUpdateDL)(void*);
typedef void (*FnModelEntryDL)(void*);
typedef void (*FnMtxTransS)(float, float, float);
typedef void (*FnMtxTransM)(float, float, float);
typedef float* (*FnMtxGet)();
typedef void* (*FnLyCurrent)();
typedef unsigned int (*FnSCtRq)(void*, short, int (*)(void*, void*), void*, void*);
typedef void* (*FnLoadBDL)(const void*, unsigned int);
typedef void (*FnSetupStageTex)(void*);
typedef int (*FnCondition)(void*, unsigned int);
typedef void (*FnOSPanic)(const char*, int, const char*, ...);

FnGetStageRes s_fnGetStageRes = nullptr;
FnModelCreate s_fnModelCreate = nullptr;
FnEntrySolidHeap s_fnEntrySolidHeap = nullptr;
FnModelUpdateDL s_fnModelUpdateDL = nullptr;
FnModelEntryDL s_fnModelEntryDL = nullptr;
FnMtxTransS s_fnMtxTransS = nullptr;
FnMtxTransM s_fnMtxTransM = nullptr;
FnMtxGet s_fnMtxGet = nullptr;
FnLyCurrent s_fnLyCurrent = nullptr;
FnSCtRq s_fnSCtRq = nullptr;
FnLoadBDL s_fnLoadBDL = nullptr;
FnSetupStageTex s_fnSetupStageTex = nullptr;
FnCondition s_fnCheckCondition = nullptr;
FnCondition s_fnOnCondition = nullptr;
FnOSPanic s_fnOSPanic = nullptr;

const unsigned int kFopAcCnd_INIT = 0x08;
const unsigned int kOffLayerTagLayer = 80;  // measured, same as registry.cpp

// Receiver PC translation of the donor heap sizes (DN-10 step 2 at the
// allocator). fopAcM_entrySolidHeap on this host already does size*=2;
// 0x80000000 is HeapAdjustUnk, which vanilla daVrbox2_Create uses
// (0x80004340) for the same actor class. Donor GC 0x21A0 after *2 is half
// of that budget.
const unsigned int kVrboxHeap = 0x80000C60u;
const unsigned int kVrbox2Heap = 0x80004340u;

int s_spawned = 0;

void logv(LogLevel level, const char* fmt, ...) {
    if (s_log == nullptr) {
        return;
    }
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, level, buf);
}

// daBg's consume-boundary for stage BDLs whose TEX1 imageOffset is 0
// (external .bti in the same archive). Donor GC J3D load from the stage
// archive already has those images; the PC loader does not. Missing bind
// → TEV samples a header as texels → black even with correct K0.
void bindStageTextures(J3DModelData* data, const char* tag) {
    if (data == nullptr) {
        return;
    }
    J3DTexture* texture_p = data->getTexture();
    JUTNameTab* tex_name_p = data->getTextureName();
    if (texture_p == nullptr) {
        logv(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"vrbox_tex\",\"tag\":\"%s\",\"n\":0}",
             tag != nullptr ? tag : "?");
        return;
    }
    int embedded = 0;
    int bound = 0;
    int missing = 0;
    const u16 n = texture_p->getNum();
    for (u16 i = 0; i < n; i++) {
        ResTIMG* img = texture_p->getResTIMG(i);
        const s32 off = (img != nullptr) ? static_cast<s32>(img->imageOffset) : -1;
        const char* nm = (tex_name_p != nullptr) ? tex_name_p->getName(i) : "?";
        if (off != 0) {
            embedded++;
            logv(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"vrbox_tex\",\"tag\":\"%s\",\"i\":%u,"
                 "\"name\":\"%.40s\",\"off\":%d,\"embedded\":1,\"w\":%d,\"h\":%d}",
                 tag != nullptr ? tag : "?", (unsigned)i, nm != nullptr ? nm : "?",
                 (int)off, img != nullptr ? (int)img->width : -1,
                 img != nullptr ? (int)img->height : -1);
            continue;
        }
        char res_name[64];
        std::snprintf(res_name, sizeof(res_name), "%s.bti", nm != nullptr ? nm : "");
        void* timg = (s_fnGetStageRes != nullptr) ? s_fnGetStageRes(res_name) : nullptr;
        if (timg != nullptr) {
            bound++;
            logv(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"vrbox_tex\",\"tag\":\"%s\",\"i\":%u,"
                 "\"name\":\"%.40s\",\"off\":0,\"bti\":\"%s\",\"bound\":1}",
                 tag != nullptr ? tag : "?", (unsigned)i, nm != nullptr ? nm : "?",
                 res_name);
        } else {
            missing++;
            logv(LOG_LEVEL_ERROR,
                 "[WwRegistry] {\"ev\":\"vrbox_tex\",\"tag\":\"%s\",\"i\":%u,"
                 "\"name\":\"%.40s\",\"off\":0,\"bti\":\"%s\",\"bound\":0}",
                 tag != nullptr ? tag : "?", (unsigned)i, nm != nullptr ? nm : "?",
                 res_name);
        }
    }
    // Prefer the receiver helper (bind + shared-DL texno patch). It asserts
    // on a missing .bti, so only call it when every external slot resolved.
    if (missing == 0 && bound > 0 && s_fnSetupStageTex != nullptr) {
        s_fnSetupStageTex(data);
    } else if (missing != 0) {
        for (u16 i = 0; i < n; i++) {
            ResTIMG* img = texture_p->getResTIMG(i);
            if (img == nullptr || static_cast<s32>(img->imageOffset) != 0) {
                continue;
            }
            const char* nm = (tex_name_p != nullptr) ? tex_name_p->getName(i) : "?";
            char res_name[64];
            std::snprintf(res_name, sizeof(res_name), "%s.bti", nm != nullptr ? nm : "");
            void* timg = (s_fnGetStageRes != nullptr) ? s_fnGetStageRes(res_name) : nullptr;
            if (timg != nullptr) {
                texture_p->setResTIMG(i, *static_cast<const ResTIMG*>(timg));
            }
        }
    }
    logv(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"vrbox_tex_sum\",\"tag\":\"%s\",\"n\":%u,"
         "\"embedded\":%d,\"bound\":%d,\"missing\":%d}",
         tag != nullptr ? tag : "?", (unsigned)n, embedded, bound, missing);
}

void* loadStageBdl(const char* name) {
    if (s_fnGetStageRes == nullptr) {
        return nullptr;
    }
    void* raw = s_fnGetStageRes(name);
    if (raw == nullptr) {
        return nullptr;
    }
    // Already a parsed J3DModelData* if the getRes consume arm ran.
    if (*static_cast<const unsigned int*>(raw) != 0x3244334Au) {
        bindStageTextures(static_cast<J3DModelData*>(raw), name);
        return raw;
    }
    if (s_fnLoadBDL == nullptr) {
        return nullptr;
    }
    // Same flags + finish as registry BG parse (kWwBdlLoadFlags). 0x59020030
    // was a leftover that skipped newSharedDisplayList/makeSharedDL, so
    // EntryDL rebuilt DLs every frame and poisoned daBg (210402).
    const unsigned int kWwBdlLoadFlags = 0x59020010u;
    void* parsed = s_fnLoadBDL(raw, kWwBdlLoadFlags);
    wwFinishParsedModel(&parsed, name);
    if (parsed != nullptr) {
        bindStageTextures(static_cast<J3DModelData*>(parsed), name);
    }
    return parsed;
}

// Donor fopAcM_ct expansion (same body as registry WwTagSo_ct). INIT is
// what fopAc_Create's debug assert checks and what later condition tests
// treat as "constructed."
void actorCt(void* self) {
    void** layerp = reinterpret_cast<void**>(static_cast<char*>(self) + kOffLayerTagLayer);
    if (*layerp == nullptr && s_fnOSPanic != nullptr) {
        s_fnOSPanic(__FILE__, __LINE__, "UH OH");
    }
    if (s_fnCheckCondition != nullptr && !s_fnCheckCondition(self, kFopAcCnd_INIT)) {
        if (s_fnOnCondition != nullptr) {
            s_fnOnCondition(self, kFopAcCnd_INIT);
        }
    }
    if (*layerp == nullptr && s_fnOSPanic != nullptr) {
        s_fnOSPanic(__FILE__, __LINE__, "Oh come on");
    }
}

Mtx s_followMtx;

// Donor writes transS(invView) even when view is null (GC camera always
// exists). On PC the first draw can race the view — skip the whole sky
// draw rather than deref a null inv-view mtx (that crash is silent: vrbox
// is prio 4/7, before daBg, but a poisoned mtx after a "successful" draw
// kills daBg on the same frame).
//
// Donor daVrbox_Draw: transS(invView) with 0.09 FILI parallax. A zero
// invView translation (PC race before the view exists) parks the dome at
// the origin — a lid, only visible looking up. Fall back to lookat.eye.
// Tale §598: Outset FILI mSeaLevel is 0. A host-magnitude sea (order 10^5)
// turns the 0.09 term into a 30k lid. Ignore an out-of-range sea.
int followCam(float yOrigin) {
    view_class* v = dComIfGd_getView();
    MtxP inv = dComIfGd_getInvViewMtx();
    f32 x = 0.0f, y = 0.0f, z = 0.0f;
    if (inv != nullptr) {
        x = inv[0][3];
        y = inv[1][3];
        z = inv[2][3];
    }
    if (x == 0.0f && y == 0.0f && z == 0.0f && v != nullptr) {
        x = v->lookat.eye.x;
        y = v->lookat.eye.y;
        z = v->lookat.eye.z;
    }
    if (x == 0.0f && y == 0.0f && z == 0.0f) {
        return 0;
    }
    f32 sea = yOrigin;
    // Tale §598: Outset seaLevel is 0. NaN fails both comparisons — force 0.
    if (!(sea == sea) || sea > 10000.0f || sea < -10000.0f) {
        sea = 0.0f;
    }
    const f32 y_offset = (y - sea) * 0.09f;
    const f32 ty = y - y_offset;
    MTXTrans(s_followMtx, x, ty, z);
    if (s_fnMtxTransS != nullptr) {
        s_fnMtxTransS(x, ty, z);
    }
    return 1;
}

void setModelTR(J3DModel* model) {
    if (model == nullptr) {
        return;
    }
    model->setBaseTRMtx(s_followMtx);
}

void logSkyGeom(J3DModelData* data, const char* tag) {
    if (data == nullptr) {
        return;
    }
    J3DVertexData& vd = data->getVertexData();
    const u32 n = vd.getVtxNum();
    const int ty = vd.getVtxPosType();
    const u8 frac = vd.getVtxPosFrac();
    f32 jx = 0.0f, jy = 0.0f, jz = 0.0f;
    f32 jsx = 1.0f, jsy = 1.0f, jsz = 1.0f;
    if (data->getJointNum() > 0) {
        J3DJoint* joint = data->getJointNodePointer(0);
        if (joint != nullptr) {
            J3DTransformInfo& tr = joint->getTransformInfo();
            jx = tr.mTranslate.x;
            jy = tr.mTranslate.y;
            jz = tr.mTranslate.z;
            jsx = tr.mScale.x;
            jsy = tr.mScale.y;
            jsz = tr.mScale.z;
        }
    }
    f32 mn[3] = {1.0e12f, 1.0e12f, 1.0e12f};
    f32 mx[3] = {-1.0e12f, -1.0e12f, -1.0e12f};
    int scanned = 0;
    if (ty == GX_F32 && vd.getVtxPosArray() != nullptr && n > 0) {
        u32 stride = 12;
#if TARGET_PC
        stride = vd.getVtxArrStride(GX_VA_POS);
        if (stride < 12) {
            stride = 12;
        }
#endif
        const u8* base = static_cast<const u8*>(vd.getVtxPosArray());
        u32 count = n;
        if (count > 2048) {
            count = 2048;
        }
        for (u32 i = 0; i < count; i++) {
            const f32* p = reinterpret_cast<const f32*>(base + i * stride);
            for (int k = 0; k < 3; k++) {
                if (p[k] < mn[k]) {
                    mn[k] = p[k];
                }
                if (p[k] > mx[k]) {
                    mx[k] = p[k];
                }
            }
        }
        scanned = (int)count;
    }
    logv(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"vrbox_geom\",\"tag\":\"%s\",\"vtx\":%u,\"posType\":%d,"
         "\"frac\":%u,\"jointT\":[%.1f,%.1f,%.1f],\"jointS\":[%.3f,%.3f,%.3f],"
         "\"scanned\":%d,\"min\":[%.1f,%.1f,%.1f],\"max\":[%.1f,%.1f,%.1f]}",
         tag != nullptr ? tag : "?", n, ty, (unsigned)frac, jx, jy, jz, jsx, jsy, jsz,
         scanned, mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]);
}

// 205848: dome EntryDL (sky 21,35,33) lived through load, then the first
// WW play frame died after dabg_light on daBg 2 — same window as 204450.
// The field snap omitted PC vtx counts, mat/shape packets, model draw mtx,
// NBT, and J3DSys statics. Full copy is the consume-boundary restore.
struct J3dSysSnap {
    J3DSys sys;
    Mtx currentMtx;
    Vec currentS;
    Vec parentS;
    J3DTexCoordScaleInfo texScale[8];
};

void j3dSnap(J3dSysSnap* s) {
    std::memcpy(&s->sys, &j3dSys, sizeof(J3DSys));
    std::memcpy(s->currentMtx, J3DSys::mCurrentMtx, sizeof(Mtx));
    s->currentS = J3DSys::mCurrentS;
    s->parentS = J3DSys::mParentS;
    std::memcpy(s->texScale, J3DSys::sTexCoordScaleTable, sizeof(s->texScale));
}

void j3dRestore(const J3dSysSnap& s) {
    std::memcpy(&j3dSys, &s.sys, sizeof(J3DSys));
    std::memcpy(J3DSys::mCurrentMtx, s.currentMtx, sizeof(Mtx));
    J3DSys::mCurrentS = s.currentS;
    J3DSys::mParentS = s.parentS;
    std::memcpy(J3DSys::sTexCoordScaleTable, s.texScale, sizeof(s.texScale));
}

void setMatK0(J3DMaterial* mat, unsigned char r, unsigned char g, unsigned char b,
              unsigned char a) {
    if (mat == nullptr || mat->getTevBlock() == nullptr) {
        return;
    }
    if (mat->getColorBlock() != nullptr) {
        mat->setCullMode(0);
    }
    if (mat->getFog() != nullptr) {
        // TP setLight just installed a frustum-scale fog. WW sky BDLs load
        // that onto GX at packet draw; NONE is the donor GC default for this
        // mesh (daVrbox_Draw does not call dKy_GxFog_set).
        mat->getFog()->mType = GX_FOG_NONE;
    }
    mat->change();
    J3DGXColor color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    mat->setTevKColor(0, &color);
}

// Donor daVrbox2_color_set (d_a_vrbox2.cpp:87,171): Z compare off, Z write
// off. Sky flush draws vr_sky first (writes depth); without this the cloud
// bands Z-fail against the dome and never show, even looking around.
J3DZModeInfo const kVrbox2Z = {GX_FALSE, GX_LEQUAL, GX_FALSE};

void applyVrbox2Z(J3DMaterial* mat) {
    if (mat == nullptr || mat->getZMode() == nullptr) {
        return;
    }
    mat->getZMode()->setZModeInfo(kVrbox2Z);
    mat->change();
}

void setMatK0Vr2(J3DMaterial* mat, unsigned char r, unsigned char g, unsigned char b,
                 unsigned char a) {
    applyVrbox2Z(mat);
    setMatK0(mat, r, g, b, a);
}

void texScrollCheck(f32& v) {
    while (v < 0.0f) {
        v += 1.0f;
    }
    while (v > 1.0f) {
        v -= 1.0f;
    }
}

// Donor daVrbox2_ww_texscroll (d_a_vrbox2.cpp §418): wind on vr_back_cloud UVs.
void texScroll(J3DMaterial* mat, int idx, f32 speed) {
    if (mat == nullptr) {
        return;
    }
    J3DTexMtx* tm = mat->getTexMtx((u32)idx);
    if (tm != nullptr) {
        f32& tx = tm->getTexMtxInfo().mSRT.mTranslationX;
        tx += speed;
        texScrollCheck(tx);
    }
}

f32 cloudScrollSpeed() {
    camera_class* camera = (camera_class*)dComIfGp_getCamera(0);
    cXyz* windVec = dKyWw_get_wind_vec();
    const f32 windPow = dKyWw_get_wind_pow();
    if (camera == nullptr || windVec == nullptr) {
        return 0.0f;
    }
    cXyz eyeXZ = camera->view.lookat.eye;
    cXyz centerXZ = camera->view.lookat.center;
    eyeXZ.y = 0.0f;
    centerXZ.y = 0.0f;
    cXyz lookDirXZ;
    dKyr_get_vectle_calc(&eyeXZ, &centerXZ, &lookDirXZ);
    return cM3d_VectorProduct2d(0.0f, 0.0f, -windVec->x, -windVec->z, lookDirXZ.x, lookDirXZ.z) *
           0.0005f * windPow;
}

void scrollBackCloud(J3DModelData* modelData) {
    if (modelData == nullptr) {
        return;
    }
    const f32 speed = cloudScrollSpeed();
    static const f32 kWwCloudSpeed[3][2] = {{1.0f, 1.0f}, {0.8f, 0.8f}, {0.6f, 1.6f}};
    static int s_scrollLogs = 0;
    s_scrollLogs++;
    if (s_scrollLogs <= 6 || (s_scrollLogs % 300) == 0) {
        logv(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"cloud_scroll\",\"n\":%d,\"speed\":%.6f,\"pow\":%.3f}",
             s_scrollLogs, speed, dKyWw_get_wind_pow());
    }
    for (int mi = 0; mi < 3 && mi < (int)modelData->getMaterialNum(); mi++) {
        J3DMaterial* mat = modelData->getMaterialNodePointer((u16)mi);
        if (mat == nullptr) {
            continue;
        }
        mat->change();
        texScroll(mat, 0, speed * kWwCloudSpeed[mi][0]);
        texScroll(mat, 1, speed * kWwCloudSpeed[mi][1]);
    }
}

// Actor-walk EntryDL/UpdateDL of vr_sky.bdl kills the first WW daBg after
// dabg_light (204450/205848/210402/210914). Donor daVrbox_Draw uses
// mDoExt_modelUpdateDL during the actor walk; on PC that walk is
// presentation (fpcM_Draw) and shares j3dSys with daBg. Keep the actor
// walk packet-free. Same donor UpdateDL runs at sky-opa flush, after daBg.
const int kSubmitVrboxPackets = 0;
const int kSubmitVrbox2Packets = 0;

J3DModel* s_domeModel = nullptr;
J3DModel* s_usoModel = nullptr;
J3DModel* s_kasumiModel = nullptr;
J3DModel* s_cloudModel = nullptr;
void* s_skyOpaBuf = nullptr;
int s_flushReady = 0;
int s_flushing = 0;

void disarmSkyFlush() {
    s_domeModel = nullptr;
    s_usoModel = nullptr;
    s_kasumiModel = nullptr;
    s_cloudModel = nullptr;
    s_flushReady = 0;
}

void armSkyFlush(J3DModel* model) {
    s_domeModel = model;
    s_flushReady = 1;
    if (s_skyOpaBuf != nullptr) {
        return;
    }
    dComIfGd_setListSky();
    s_skyOpaBuf = j3dSys.getDrawBuffer(0);
    dComIfGd_setList();
    logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_skybuf\",\"buf\":\"%p\"}", s_skyOpaBuf);
}

void armVrbox2Flush(J3DModel* uso, J3DModel* kasumi, J3DModel* cloud) {
    s_usoModel = uso;
    s_kasumiModel = kasumi;
    s_cloudModel = cloud;
}

void updateOne(J3DModel* model) {
    if (model == nullptr || s_fnModelUpdateDL == nullptr) {
        return;
    }
    J3DModelData* data = model->getModelData();
    if (data != nullptr) {
        const u16 n = data->getShapeNum();
        for (u16 i = 0; i < n; i++) {
            J3DShape* shape = data->getShapeNodePointer(i);
            if (shape != nullptr) {
                shape->show();
            }
        }
    }
    s_fnModelUpdateDL(model);
}

void skyFlush(void* drawBuf) {
    if (s_flushReady == 0 || s_flushing != 0 || s_domeModel == nullptr) {
        return;
    }
    if (s_skyOpaBuf == nullptr || drawBuf != s_skyOpaBuf) {
        return;
    }
    s_flushReady = 0;
    s_flushing = 1;

    // Painter draws sky, then GXSetClipMode(ENABLE) for BG — same order as
    // donor m_Do_graphic. Clip is still ENABLE from the previous frame, and
    // the TP camera far plane is not the WW STAG far. Disable for this list
    // and leave it disabled through drawXluListSky (celestial/vrkumo).
    GXSetClipMode(GX_CLIP_DISABLE);
    GXColor fogOff = {0, 0, 0, 0};
    GXSetFog(GX_FOG_NONE, 0.0f, 1.0f, 1.0f, 1.0f, fogOff);

    // STAG mFar → camera → view.far before dome/cumulus (Housing [D3] chain).
    dKyWw_applyDiscStagFarToCamera();

    view_class* v = dComIfGd_getView();
    const f32 sea = dKyWw_filiSeaLevel();
    if (!followCam(sea)) {
        s_flushing = 0;
        return;
    }
    // Donor daVrbox / daVrbox2: transS, submit sky+uso+kasumi, THEN transM
    // +100 for back-cloud. Doing transM before any UpdateDL left the stack
    // at a stale daBg mtx (~30k Y) and the dome rode that as a lid.
    dComIfGd_setListSky();
    setModelTR(s_domeModel);
    updateOne(s_domeModel);
    setModelTR(s_usoModel);
    updateOne(s_usoModel);
    setModelTR(s_kasumiModel);
    updateOne(s_kasumiModel);
    if (s_fnMtxTransM != nullptr) {
        s_fnMtxTransM(0.0f, 100.0f, 0.0f);
        if (s_fnMtxGet != nullptr) {
            float* src = s_fnMtxGet();
            if (src != nullptr) {
                MTXCopy(reinterpret_cast<f32(*)[4]>(src), s_followMtx);
            }
        }
    }
    setModelTR(s_cloudModel);
    updateOne(s_cloudModel);
    wwSky_drawInto(drawBuf);
    J3DPacket* opaPkt = j3dSys.getDrawBuffer(0) != nullptr
                            ? j3dSys.getDrawBuffer(0)->mpBuffer[0]
                            : nullptr;
    J3DPacket* xluPkt = j3dSys.getDrawBuffer(1) != nullptr
                            ? j3dSys.getDrawBuffer(1)->mpBuffer[0]
                            : nullptr;
    dComIfGd_setList();
    GXSetClipMode(GX_CLIP_DISABLE);
    s_flushing = 0;

    static int s_flushes = 0;
    s_flushes++;
    if (s_flushes <= 8 || (s_flushes % 300) == 0) {
        const f32 farz = (v != nullptr) ? v->far_ : -1.0f;
        MtxP inv = dComIfGd_getInvViewMtx();
        MtxP tr = (s_domeModel != nullptr) ? s_domeModel->getBaseTRMtx() : nullptr;
        int opaMode = -1;
        if (s_domeModel != nullptr && s_domeModel->getModelData() != nullptr) {
            J3DMaterial* m0 = s_domeModel->getModelData()->getMaterialNodePointer(0);
            J3DMaterial* m1 = s_domeModel->getModelData()->getMaterialNodePointer(1);
            const int a = (m0 != nullptr) ? (int)m0->isDrawModeOpaTexEdge() : -1;
            const int b = (m1 != nullptr) ? (int)m1->isDrawModeOpaTexEdge() : -1;
            opaMode = (a << 4) | (b & 0xF);
        }
        logv(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"vrbox_flush\",\"n\":%d,\"step\":\"ok\","
             "\"buf\":\"%p\",\"model\":\"%p\",\"opa\":\"%p\",\"xlu\":\"%p\","
             "\"vrbox2\":%d,\"far\":%.1f,\"sea\":%.1f,\"inv\":[%.1f,%.1f,%.1f],"
             "\"eye\":[%.1f,%.1f,%.1f],\"tr\":[%.1f,%.1f,%.1f],\"matOpa\":%d}",
             s_flushes, drawBuf, s_domeModel, opaPkt, xluPkt,
             (s_usoModel != nullptr || s_kasumiModel != nullptr || s_cloudModel != nullptr) ? 1
                                                                                           : 0,
             farz, sea, inv != nullptr ? inv[0][3] : 0.0f, inv != nullptr ? inv[1][3] : 0.0f,
             inv != nullptr ? inv[2][3] : 0.0f,
             v != nullptr ? v->lookat.eye.x : 0.0f, v != nullptr ? v->lookat.eye.y : 0.0f,
             v != nullptr ? v->lookat.eye.z : 0.0f, tr != nullptr ? tr[0][3] : 0.0f,
             tr != nullptr ? tr[1][3] : 0.0f, tr != nullptr ? tr[2][3] : 0.0f, opaMode);
    }
}

void drawModel(J3DModel* model) {
    if (model == nullptr) {
        return;
    }
    setModelTR(model);
    if (s_fnModelEntryDL != nullptr) {
        s_fnModelEntryDL(model);
    } else if (s_fnModelUpdateDL != nullptr) {
        s_fnModelUpdateDL(model);
    }
}

}  // namespace

int wwVrbox_bind(const HookService* hook) {
    if (hook == nullptr) {
        return 0;
    }
    struct Bind {
        const char* sym;
        void** slot;
    };
    const Bind kBinds[] = {
        {"?dComIfG_getStageRes@@YAPEAXPEBD@Z", reinterpret_cast<void**>(&s_fnGetStageRes)},
        {"?mDoExt_J3DModel__create@@YAPEAVJ3DModel@@PEAVJ3DModelData@@II@Z",
         reinterpret_cast<void**>(&s_fnModelCreate)},
        {"?fopAcM_entrySolidHeap@@YA_NPEAVfopAc_ac_c@@P6AH0@ZI@Z",
         reinterpret_cast<void**>(&s_fnEntrySolidHeap)},
        {"?mDoExt_modelUpdateDL@@YAXPEAVJ3DModel@@@Z",
         reinterpret_cast<void**>(&s_fnModelUpdateDL)},
        {"?mDoExt_modelEntryDL@@YAXPEAVJ3DModel@@@Z",
         reinterpret_cast<void**>(&s_fnModelEntryDL)},
        {"?transS@mDoMtx_stack_c@@SAXMMM@Z", reinterpret_cast<void**>(&s_fnMtxTransS)},
        {"?transM@mDoMtx_stack_c@@SAXMMM@Z", reinterpret_cast<void**>(&s_fnMtxTransM)},
        {"?get@mDoMtx_stack_c@@SAPEAY03MXZ", reinterpret_cast<void**>(&s_fnMtxGet)},
        {"?fpcLy_CurrentLayer@@YAPEAUlayer_class@@XZ",
         reinterpret_cast<void**>(&s_fnLyCurrent)},
        {"?fpcSCtRq_Request@@YAIPEAUlayer_class@@FP6AHPEAX1@Z11@Z",
         reinterpret_cast<void**>(&s_fnSCtRq)},
        {"?loadBinaryDisplayList@J3DModelLoaderDataBase@@SAPEAVJ3DModelData@@PEBXI@Z",
         reinterpret_cast<void**>(&s_fnLoadBDL)},
        {"?mDoExt_setupStageTexture@@YAXPEAVJ3DModelData@@@Z",
         reinterpret_cast<void**>(&s_fnSetupStageTex)},
        {"fopAcM_CheckCondition", reinterpret_cast<void**>(&s_fnCheckCondition)},
        {"fopAcM_OnCondition", reinterpret_cast<void**>(&s_fnOnCondition)},
        {"OSPanic", reinterpret_cast<void**>(&s_fnOSPanic)},
    };
    int bound = 0;
    for (const Bind& b : kBinds) {
        void* a = nullptr;
        if (hook->resolve(mod_ctx, b.sym, &a, nullptr) == MOD_OK && a != nullptr) {
            *b.slot = a;
            bound++;
        } else {
            logv(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"vrbox_bind_miss\",\"sym\":\"%.80s\"}",
                 b.sym);
        }
    }
    logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_bind\",\"bound\":%d,\"of\":%d}", bound,
         (int)(sizeof(kBinds) / sizeof(kBinds[0])));
    return bound;
}

void wwVrbox_onSkyOpaDraw(void* drawBuf) {
    skyFlush(drawBuf);
}

void wwVrbox_resetSpawn() {
    s_spawned = 0;
    disarmSkyFlush();
}

void wwVrbox_trySpawn(short vrboxIndex, short vrbox2Index) {
    if (s_spawned != 0) {
        return;
    }
    if (s_fnGetStageRes == nullptr || s_fnSCtRq == nullptr || s_fnLyCurrent == nullptr) {
        logv(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"vrbox_spawn\",\"ok\":0,\"why\":\"bind\"}");
        return;
    }
    if (s_fnGetStageRes("vr_sky.bdl") == nullptr) {
        logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_spawn\",\"ok\":0,\"why\":\"no vr_sky.bdl\"}");
        return;
    }
    void* layer = s_fnLyCurrent();
    const unsigned int id1 = s_fnSCtRq(layer, vrboxIndex, nullptr, nullptr, nullptr);
    const unsigned int id2 = s_fnSCtRq(layer, vrbox2Index, nullptr, nullptr, nullptr);
    s_spawned = 1;
    logv(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"vrbox_spawn\",\"ok\":1,\"vrbox\":%u,\"vrbox2\":%u,"
         "\"idx\":[%d,%d]}",
         id1, id2, (int)vrboxIndex, (int)vrbox2Index);
}

int WwVrbox_solidHeapCB(void* self) {
    VrboxMembers* m = vrboxM(self);
    void* data = loadStageBdl("vr_sky.bdl");
    if (data == nullptr || s_fnModelCreate == nullptr) {
        return 0;
    }
    m->model = static_cast<J3DModel*>(s_fnModelCreate(data, 0x80000u, 0x11020202u));
    if (m->model != nullptr) {
        Vec one = {1.0f, 1.0f, 1.0f};
        m->model->setBaseScale(one);
        logSkyGeom(static_cast<J3DModelData*>(data), "vr_sky.bdl");
    }
    return (m->model != nullptr) ? 1 : 0;
}

int WwVrbox_create(void* self) {
    actorCt(self);
    std::memset(vrboxM(self), 0, sizeof(VrboxMembers));
    if (s_fnEntrySolidHeap == nullptr ||
        !s_fnEntrySolidHeap(self, WwVrbox_solidHeapCB, kVrboxHeap)) {
        logv(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"vrbox_created\",\"ok\":0}");
        return 5;
    }
    // Do NOT dComIfGp_onStatus(1). Donor Create sets it so WW kankyo
    // draws vrkumo/sun. On vanilla TP that same bit opens TP
    // dKyw_wether (sun + vrkumo), which reads roomRead vrboxswitch as
    // TP layout and loads cloudtx/F_moon from the stage. Boot 223539
    // died after this call, still in sea create, before vrbox_draw.
    logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_created\",\"ok\":1,\"model\":\"%p\"}",
         vrboxM(self)->model);
    return 4;
}

int WwVrbox_delete(void* self) {
    if (s_domeModel == vrboxM(self)->model) {
        disarmSkyFlush();
    }
    return 1;
}
int WwVrbox_execute(void*) {
    return 1;
}
int WwVrbox_isDelete(void*) {
    return 1;
}

int WwVrbox_draw(void* self) {
    unsigned char sky[4], kasumi[4], kumo[4], uso[4];
    int inv = 1;
    wwKankyo_vrboxGet(sky, kasumi, kumo, uso, &inv);

    J3DModel* model = vrboxM(self)->model;
    if (model == nullptr) {
        return 1;
    }
    J3DModelData* modelData = model->getModelData();
    if (modelData == nullptr) {
        return 1;
    }

    if (inv != 0 ||
        sky[0] + sky[1] + sky[2] + kasumi[0] + kasumi[1] + kasumi[2] + kumo[0] + kumo[1] +
                kumo[2] ==
            0) {
        return 1;
    }

    static int s_draws = 0;
    s_draws++;
    if (s_draws <= 6 || (s_draws % 300) == 0) {
        logv(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"vrbox_draw\",\"n\":%d,\"step\":\"color\","
             "\"sky\":[%u,%u,%u],\"kasumi\":[%u,%u,%u]}",
             s_draws, sky[0], sky[1], sky[2], kasumi[0], kasumi[1], kasumi[2]);
    }

    setMatK0(modelData->getMaterialNodePointer(0), kasumi[0], kasumi[1], kasumi[2], 0xFF);
    setMatK0(modelData->getMaterialNodePointer(1), sky[0], sky[1], sky[2], 0xFF);

    if (!followCam(dKyWw_filiSeaLevel())) {
        if (s_draws <= 6) {
            logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_draw\",\"step\":\"no_view\"}");
        }
        return 1;
    }
    setModelTR(model);
    if (kSubmitVrboxPackets == 0) {
        armSkyFlush(model);
        if (s_draws <= 12 || (s_draws % 300) == 0) {
            logv(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"vrbox_draw\",\"n\":%d,\"step\":\"nopkt\"}", s_draws);
        }
        return 1;
    }
    J3dSysSnap snap;
    j3dSnap(&snap);
    dComIfGd_setListSky();
    drawModel(model);
    dComIfGd_setList();
    j3dRestore(snap);
    if (s_draws <= 6) {
        logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox_draw\",\"step\":\"ok\"}");
    }
    return 1;
}

int WwVrbox2_solidHeapCB(void* self) {
    Vrbox2Members* m = vrbox2M(self);
    m->backCloud = m->kasumiMae = m->usoUmi = nullptr;
    if (s_fnModelCreate == nullptr) {
        return 0;
    }
    void* data = loadStageBdl("vr_back_cloud.bdl");
    if (data != nullptr) {
        m->backCloud = static_cast<J3DModel*>(s_fnModelCreate(data, 0x80000u, 0x11020202u));
    }
    data = loadStageBdl("vr_kasumi_mae.bdl");
    if (data != nullptr) {
        m->kasumiMae = static_cast<J3DModel*>(s_fnModelCreate(data, 0x80000u, 0x11020202u));
    }
    data = loadStageBdl("vr_uso_umi.bdl");
    if (data != nullptr) {
        m->usoUmi = static_cast<J3DModel*>(s_fnModelCreate(data, 0x80000u, 0x11020202u));
    }
    return (m->backCloud != nullptr && m->kasumiMae != nullptr && m->usoUmi != nullptr) ? 1 : 0;
}

int WwVrbox2_create(void* self) {
    actorCt(self);
    std::memset(vrbox2M(self), 0, sizeof(Vrbox2Members));
    if (s_fnEntrySolidHeap == nullptr ||
        !s_fnEntrySolidHeap(self, WwVrbox2_solidHeapCB, kVrbox2Heap)) {
        logv(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"vrbox2_created\",\"ok\":0}");
        return 5;
    }
    logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox2_created\",\"ok\":1}");
    return 4;
}

int WwVrbox2_delete(void* self) {
    Vrbox2Members* m = vrbox2M(self);
    if (s_usoModel == m->usoUmi || s_kasumiModel == m->kasumiMae || s_cloudModel == m->backCloud) {
        s_usoModel = nullptr;
        s_kasumiModel = nullptr;
        s_cloudModel = nullptr;
    }
    return 1;
}
int WwVrbox2_execute(void*) {
    return 1;
}
int WwVrbox2_isDelete(void*) {
    return 1;
}

int WwVrbox2_draw(void* self) {
    unsigned char sky[4], kasumi[4], kumo[4], uso[4];
    int inv = 1;
    wwKankyo_vrboxGet(sky, kasumi, kumo, uso, &inv);
    if (inv != 0 ||
        sky[0] + sky[1] + sky[2] + kasumi[0] + kasumi[1] + kasumi[2] + kumo[0] + kumo[1] +
                kumo[2] ==
            0) {
        return 1;
    }

    static int s_draws = 0;
    s_draws++;
    if (s_draws <= 6 || (s_draws % 300) == 0) {
        logv(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"vrbox2_draw\",\"n\":%d,\"step\":\"color\"}", s_draws);
    }

    if (!followCam(dKyWw_filiSeaLevel())) {
        if (s_draws <= 6) {
            logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox2_draw\",\"step\":\"no_view\"}");
        }
        return 1;
    }

    Vrbox2Members* m = vrbox2M(self);
    if (m->usoUmi != nullptr && m->usoUmi->getModelData() != nullptr) {
        setMatK0Vr2(m->usoUmi->getModelData()->getMaterialNodePointer(0), uso[0], uso[1], uso[2],
                    0xFF);
        setModelTR(m->usoUmi);
    }
    if (m->kasumiMae != nullptr && m->kasumiMae->getModelData() != nullptr) {
        J3DMaterial* mat = m->kasumiMae->getModelData()->getMaterialNodePointer(0);
        if (mat != nullptr && mat->getTevBlock() != nullptr) {
            applyVrbox2Z(mat);
            mat->change();
            J3DGXColorS10 c0;
            c0.r = kasumi[0];
            c0.g = kasumi[1];
            c0.b = kasumi[2];
            c0.a = 255;
            J3DGXColor ka;
            ka.r = kumo[3];
            ka.g = 0;
            ka.b = 0;
            ka.a = 0;
            mat->setTevColor(0, &c0);
            mat->setTevKColor(0, &ka);
            if (mat->getFog() != nullptr) {
                mat->getFog()->mType = GX_FOG_NONE;
            }
        }
        setModelTR(m->kasumiMae);
    }
    if (m->backCloud != nullptr && m->backCloud->getModelData() != nullptr) {
        J3DModelData* modelData = m->backCloud->getModelData();
        for (int i = 0; i < 3 && i < (int)modelData->getMaterialNum(); i++) {
            setMatK0Vr2(modelData->getMaterialNodePointer((u16)i), kumo[0], kumo[1], kumo[2], 0xFF);
        }
        scrollBackCloud(modelData);
        if (s_fnMtxTransM != nullptr) {
            s_fnMtxTransM(0.0f, 100.0f, 0.0f);
        }
        setModelTR(m->backCloud);
    }

    if (kSubmitVrbox2Packets == 0) {
        armVrbox2Flush(m->usoUmi, m->kasumiMae, m->backCloud);
        if (s_draws <= 12 || (s_draws % 300) == 0) {
            logv(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"vrbox2_draw\",\"n\":%d,\"step\":\"nopkt\"}", s_draws);
        }
        return 1;
    }

    J3dSysSnap snap;
    j3dSnap(&snap);
    dComIfGd_setListSky();

    if (m->usoUmi != nullptr && m->usoUmi->getModelData() != nullptr) {
        setMatK0Vr2(m->usoUmi->getModelData()->getMaterialNodePointer(0), uso[0], uso[1], uso[2],
                    0xFF);
        drawModel(m->usoUmi);
    }
    if (m->kasumiMae != nullptr && m->kasumiMae->getModelData() != nullptr) {
        J3DMaterial* mat = m->kasumiMae->getModelData()->getMaterialNodePointer(0);
        if (mat != nullptr && mat->getTevBlock() != nullptr) {
            applyVrbox2Z(mat);
            mat->change();
            J3DGXColorS10 c0;
            c0.r = kasumi[0];
            c0.g = kasumi[1];
            c0.b = kasumi[2];
            c0.a = 255;
            J3DGXColor ka;
            ka.r = kumo[3];  // donor: cloud ALPHA on kasumi K0.r
            ka.g = 0;
            ka.b = 0;
            ka.a = 0;
            mat->setTevColor(0, &c0);
            mat->setTevKColor(0, &ka);
        }
        drawModel(m->kasumiMae);
    }
    if (m->backCloud != nullptr && m->backCloud->getModelData() != nullptr) {
        J3DModelData* modelData = m->backCloud->getModelData();
        for (int i = 0; i < 3 && i < (int)modelData->getMaterialNum(); i++) {
            setMatK0Vr2(modelData->getMaterialNodePointer((u16)i), kumo[0], kumo[1], kumo[2], 0xFF);
        }
        if (s_fnMtxTransM != nullptr) {
            s_fnMtxTransM(0.0f, 100.0f, 0.0f);
        }
        drawModel(m->backCloud);
    }

    dComIfGd_setList();
    j3dRestore(snap);
    if (s_draws <= 6) {
        logv(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"vrbox2_draw\",\"step\":\"ok\"}");
    }
    return 1;
}
