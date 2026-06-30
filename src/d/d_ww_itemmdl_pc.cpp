/**
 * d_ww_itemmdl_pc.cpp — Aurora-safe material patch for WW itemmdl view models.
 */

#include "d/d_ww_itemmdl_pc.h"

#if TARGET_PC

#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/JKernel/JKRArchive.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "SSystem/SComponent/c_counter.h"
#include "d/d_com_inf_game.h"
#include "d/d_resorce.h"
#include "res/Object/itemmdl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

void dWwItemmdl_debugLog(const char* message) {
    const char* user = getenv("USERPROFILE");
    char path[512];
    if (user != nullptr && user[0] != '\0') {
        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_ww_itemmdl_debug.txt", user);
    } else {
        strncpy(path, "albw_ww_itemmdl_debug.txt", sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    FILE* fp = fopen(path, "a");
    if (fp == nullptr) {
        return;
    }

    fprintf(fp, "frame=%d heap: %s\n", g_Counter.mCounter0, message);
    fclose(fp);
}

namespace {

// Patched BDL load: real TevBlockPatched + DoBdlMaterialCalc (not locked-null materials).
static constexpr u32 kItemmdlBdlLoadFlags = 0x59020010u | 0x2000u;

static bool isRawJ3dModelFile(const void* data) {
    if (data == NULL) {
        return false;
    }

    const J3DModelFileData* header = static_cast<const J3DModelFileData*>(data);
    return header->mMagic1 == 'J3D2' &&
           (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2');
}

static bool isRawJ3dBinaryDisplayListFile(const void* data) {
    if (data == NULL) {
        return false;
    }

    const J3DModelFileData* header = static_cast<const J3DModelFileData*>(data);
    return header->mMagic1 == 'J3D2' &&
           (header->mMagic2 == 'bdl3' || header->mMagic2 == 'bdl4');
}

static J3DModelData* finishLoadedModelData(J3DModelData* model_data) {
    if (model_data == NULL || model_data->getMaterialNum() == 0) {
        return NULL;
    }

    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        J3DMaterial* material = model_data->getMaterialNodePointer(i);
        material->change();

        J3DMaterialAnm* material_anm = JKR_NEW J3DMaterialAnm();
        if (material_anm == NULL) {
            return NULL;
        }

        material->setMaterialAnm(material_anm);
    }

    if (model_data->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        return NULL;
    }

    model_data->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    model_data->makeSharedDL();
    return model_data;
}

static J3DModelData* loadFromResourcePointer(void* res) {
    if (res == NULL) {
        return NULL;
    }

    if (isRawJ3dModelFile(res)) {
        return finishLoadedModelData(
            J3DModelLoaderDataBase::load(res, 0x59020010));
    }

    if (isRawJ3dBinaryDisplayListFile(res)) {
        return finishLoadedModelData(
            J3DModelLoaderDataBase::loadBinaryDisplayList(res, kItemmdlBdlLoadFlags));
    }

    // Aurora may return an already-instantiated J3DModelData* for BDL entries. That object is
    // archive-resident; do not attach it to demo-item heap or mutate it during draw.
    return NULL;
}

static int asciiLower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

static bool nameEqualsIgnoreCase(const char* name, const char* expected) {
    if (name == NULL || expected == NULL) {
        return false;
    }

    while (*expected != '\0') {
        if (asciiLower(static_cast<unsigned char>(*name)) !=
            asciiLower(static_cast<unsigned char>(*expected))) {
            return false;
        }
        name++;
        expected++;
    }

    if (*name == '\0') {
        return true;
    }

    return nameEqualsIgnoreCase(name, ".bmd") || nameEqualsIgnoreCase(name, ".bdl");
}

static const char* baseResourceName(const char* name) {
    if (name == NULL) {
        return NULL;
    }

    const char* slash = strrchr(name, '/');
    const char* backslash = strrchr(name, '\\');
    const char* base = name;
    if (slash != NULL && slash + 1 > base) {
        base = slash + 1;
    }
    if (backslash != NULL && backslash + 1 > base) {
        base = backslash + 1;
    }
    return base;
}

static bool isVbowResourceName(const char* name) {
    const char* base = baseResourceName(name);
    return nameEqualsIgnoreCase(base, "vbow");
}

static J3DModelData* tryNamedResource(const char* arc_name, const char* res_name, const char* log_label) {
    J3DModelData* model_data = loadFromResourcePointer(dComIfG_getObjectRes(arc_name, res_name));
    if (model_data == NULL) {
        return NULL;
    }

    char message[128];
    snprintf(message, sizeof(message), "getVbowModelData: loaded via %s (%s) mats=%u tex=%u", log_label,
             res_name, model_data->getMaterialNum(),
             model_data->getTexture() != NULL ? model_data->getTexture()->getNum() : 0u);
    dWwItemmdl_debugLog(message);
    return model_data;
}

static J3DModelData* scanItemmdlArchiveForVbow(const char* arc_name) {
    dRes_info_c* info = dComIfG_getObjectResInfo(arc_name);
    if (info == NULL || info->getArchive() == NULL) {
        dWwItemmdl_debugLog("scan: itemmdl archive not resident");
        return NULL;
    }

    JKRArchive* archive = info->getArchive();
    const u32 file_count = archive->countFile();
    char summary[64];
    snprintf(summary, sizeof(summary), "scan: itemmdl files=%u", file_count);
    dWwItemmdl_debugLog(summary);

    for (u32 i = 0; i < file_count; i++) {
        JKRArchive::SDIFileEntry* entry = archive->findIdxResource(i);
        if (entry == NULL) {
            continue;
        }

        const char* name =
            archive->mStringTable + (entry->type_flags_and_name_offset & 0xFFFFFF);
        if (!isVbowResourceName(name)) {
            continue;
        }

        void* res = info->getRes(static_cast<s32>(i));
        if (res == NULL) {
            res = archive->getIdxResource(i);
        }

        J3DModelData* model_data = loadFromResourcePointer(res);
        if (model_data == NULL) {
            char message[128];
            snprintf(message, sizeof(message), "scan: matched name %s but could not load BMD", name);
            dWwItemmdl_debugLog(message);
            continue;
        }

        char message[128];
        snprintf(message, sizeof(message), "scan: matched %s mats=%u tex=%u", name,
                 model_data->getMaterialNum(),
                 model_data->getTexture() != NULL ? model_data->getTexture()->getNum() : 0u);
        dWwItemmdl_debugLog(message);
        return model_data;
    }

    for (u32 i = 0; i < file_count && i < 8; i++) {
        JKRArchive::SDIFileEntry* entry = archive->findIdxResource(i);
        if (entry == NULL) {
            continue;
        }

        const char* name =
            archive->mStringTable + (entry->type_flags_and_name_offset & 0xFFFFFF);
        char message[96];
        snprintf(message, sizeof(message), "scan: file[%u]=%s id=%u", i, name, entry->file_id);
        dWwItemmdl_debugLog(message);
    }

    return NULL;
}

static bool isOutlineMaterial(const char* name) {
    if (name == NULL) {
        return false;
    }

    // WW itemmdl cel ink / shadow pass (e.g. SC_Vbow_v). Main body is Vbow_v.
    if (strncmp(name, "SC_", 3) == 0) {
        return true;
    }

    return false;
}

static J3DShape* s_suppressedOutlineShape = NULL;

static void logModelSummary(J3DModelData* model_data, const char* label) {
    if (model_data == NULL) {
        return;
    }

    char message[128];
    snprintf(message, sizeof(message), "%s: mats=%u joints=%u tex=%u", label,
             model_data->getMaterialNum(), model_data->getJointNum(),
             model_data->getTexture() != NULL ? model_data->getTexture()->getNum() : 0u);
    dWwItemmdl_debugLog(message);
}

static void patchModelInternal(J3DModelData* model_data) {
    if (model_data == NULL || model_data->getMaterialNum() == 0) {
        return;
    }

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();

    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        const char* name = names != NULL ? names->getName(i) : NULL;
        if (name != NULL) {
            char message[96];
            snprintf(message, sizeof(message), "patchModel: mat[%u]=%s edge=%d", i, name,
                     isOutlineMaterial(name) ? 1 : 0);
            dWwItemmdl_debugLog(message);
        }
    }
}

}  // namespace

J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name) {
    static J3DModelData* s_cached = NULL;
    if (s_cached != NULL) {
        return s_cached;
    }

    static const char* const kNames[] = {"vbow.bdl", "vbow", "vbow.bmd", "Vbow.bdl", "Vbow",
                                         "VBOW.bdl", NULL};
    for (int i = 0; kNames[i] != NULL; i++) {
        J3DModelData* model_data = tryNamedResource(arc_name, kNames[i], "name");
        if (model_data != NULL) {
            s_cached = model_data;
            return model_data;
        }
    }

    J3DModelData* model_data = loadFromResourcePointer(
        dComIfG_getObjectIDRes(arc_name, dRes_ID_ITEMMDL_BDL_VBOW_e));
    if (model_data != NULL) {
        dWwItemmdl_debugLog("getVbowModelData: loaded via resource ID");
        s_cached = model_data;
        return model_data;
    }

    model_data = loadFromResourcePointer(
        dComIfG_getObjectRes(arc_name, static_cast<s32>(dRes_INDEX_ITEMMDL_BDL_VBOW_e)));
    if (model_data != NULL) {
        dWwItemmdl_debugLog("getVbowModelData: loaded via file index");
        s_cached = model_data;
        return model_data;
    }

    model_data = scanItemmdlArchiveForVbow(arc_name);
    if (model_data != NULL) {
        s_cached = model_data;
        return model_data;
    }

    dWwItemmdl_debugLog(
        "getVbowModelData: failed — ensure retail itemmdl.arc is in res/Object/ (vbow.bdl)");
    return NULL;
}

void dWwItemmdl_patchModelForPc(J3DModelData* model_data) {
    logModelSummary(model_data, "patchModel");
    patchModelInternal(model_data);
    dWwItemmdl_debugLog("patchModel: done");
}

void dWwItemmdl_suppressOutlineForDraw(J3DModelData* model_data) {
    s_suppressedOutlineShape = NULL;
    if (model_data == NULL) {
        return;
    }

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        const char* name = names != NULL ? names->getName(i) : NULL;
        if (!isOutlineMaterial(name)) {
            continue;
        }

        J3DShape* shape = model_data->getMaterialNodePointer(i)->getShape();
        if (shape == NULL) {
            return;
        }

        shape->hide();
        s_suppressedOutlineShape = shape;
        return;
    }
}

void dWwItemmdl_restoreOutlineAfterDraw(J3DModelData* model_data) {
    (void)model_data;
    if (s_suppressedOutlineShape != NULL) {
        s_suppressedOutlineShape->show();
        s_suppressedOutlineShape = NULL;
    }
}

#endif
