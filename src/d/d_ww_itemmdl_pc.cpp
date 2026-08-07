// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
/**
 * d_ww_itemmdl_pc.cpp — Aurora-safe material patch for WW itemmdl view models.
 */

// Step 19 cluster 2: renames this TU's definitions to dWwItemmdlImpl_*.
// MUST precede every other include so the header's declarations are
// renamed too and the two cannot disagree.
#include "d/ext_plugin/ww_itemmdl_impl_names.h"

#include "d/d_ww_itemmdl_pc.h"

#if TARGET_PC

#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DTexture.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/JKernel/JKRArchive.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "SSystem/SComponent/c_counter.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_item_data.h"
#include "d/d_kankyo.h"
#include "d/d_particle_name.h"
#include "d/d_resorce.h"
#include "f_op/f_op_actor_mng.h"
#include "aurora/lib/gx/gx.hpp"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "dusk/settings.h"
#include "m_Do/m_Do_ext.h"
#include "res/Object/itemmdl.h"

#include <dolphin/gx/GXTev.h>
#include <dolphin/gx/GXGeometry.h>
#include <dolphin/gx/GXPixel.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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

void dWwItemmdl_bracketLog(const char* message) {
    char buf[160];
    snprintf(buf, sizeof(buf), "bracket: %s", message);
    dWwItemmdl_debugLog(buf);
}

void dWwItemmdl_logHeap(const char* label) {
    JKRHeap* heap = JKRGetCurrentHeap();
    char message[128];
    snprintf(message, sizeof(message), "bracket heap [%s]: ptr=%p name=%s", label, heap,
             heap != NULL ? heap->getName() : "null");
    dWwItemmdl_debugLog(message);
}

bool dWwItemmdl_isPhase2BracketBow(u8 item_no) {
    return item_no == dItemNo_BOW_e && dusk::getSettings().game.wwItemmdlGetItem.getValue();
}

bool dWwItemmdl_use2DIsolateHeap() {
    return dusk::getSettings().game.wwItemmdlGetItem.getValue() &&
           dusk::getSettings().game.wwItemmdlGetItem2DIsolate.getValue();
}

namespace {

// Patched BDL load: real TevBlockPatched + DoBdlMaterialCalc (not locked-null materials).
static constexpr u32 kItemmdlBdlLoadFlags = 0x59020010u | 0x2000u;

static void logTexGenDump(J3DModelData* model_data, const char* phase);
static void logTevOrderDumpInternal(J3DModelData* model_data, const char* phase, s32 room_no,
                                    bool log_gx_runtime);

static J3DMaterial* findMaterialByName(J3DModelData* model_data, const char* target) {
    if (model_data == NULL || target == NULL) {
        return NULL;
    }

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        const char* name = names != NULL ? names->getName(i) : NULL;
        if (name != NULL && strcmp(name, target) == 0) {
            return model_data->getMaterialNodePointer(i);
        }
    }

    return NULL;
}

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

// 2R: private itemmdl mount (cap-wear pattern) — isolated from shared dComIfG object-res slot.
static constexpr int kPrivateItemmdlInfoNum = 1;
static const char* const kItemmdlArcName = "itemmdl";
static dRes_info_c s_privateItemmdlInfo[kPrivateItemmdlInfoNum];
static u8 s_privateItemmdlState = 0;  // 0 idle, 1 loading, 2 ready, 3 failed

// 2Q: parse vbow once onto a retained heap; reuse across spawns (never re-parse arc buffer).
static JKRSolidHeap* s_vbowRetainedHeap = NULL;
static J3DModelData* s_cachedVbow = NULL;
static bool s_vbowParseFailed = false;
static const void* s_parseRawPtr = NULL;
static constexpr u32 kVbowRetainedHeapSize = 0x100000u;
static J3DShape* s_suppressedOutlineShape = NULL;
#if TARGET_PC
// 4E: warmer/less yellow body + lower cap + dark SC ink (4D baseline polish).
static constexpr u8 kWwBowFixedAmbR = 105;
static constexpr u8 kWwBowFixedAmbG = 78;
static constexpr u8 kWwBowFixedAmbB = 48;
static constexpr u8 kWwBowBodyAmbCap = 80;
static constexpr u8 kWwBowInkAmbR = 58;
static constexpr u8 kWwBowInkAmbG = 48;
static constexpr u8 kWwBowInkAmbB = 42;
static bool s_wwBowGetItemBeamSuppress = false;

static bool wwBowSuppressScInkPassForDraw() {
    return dusk::getSettings().game.wwItemmdlBowScSuppress.getValue();
}

static u8 wwBowScK0CapRgb() {
    int cap = dusk::getSettings().game.wwItemmdlBowScK0Cap.getValue();
    if (cap < 0) {
        cap = 0;
    } else if (cap > 255) {
        cap = 255;
    }
    return static_cast<u8>(cap);
}

static u8 wwBowScOutputCeilingRgb() {
    int ceiling = dusk::getSettings().game.wwItemmdlBowScOutputCeiling.getValue();
    if (ceiling < 0) {
        ceiling = 0;
    } else if (ceiling > 255) {
        ceiling = 255;
    }
    return static_cast<u8>(ceiling);
}

static void applyScOutputCeilingTevStage(u8 n_tev, u8 ceiling_rgb) {
    if (ceiling_rgb >= 255 || n_tev >= 16) {
        return;
    }

    const GXTevStageID clamp_st = static_cast<GXTevStageID>(GX_TEVSTAGE0 + n_tev);

    GXColor gx_ceiling_k1;
    gx_ceiling_k1.r = ceiling_rgb;
    gx_ceiling_k1.g = ceiling_rgb;
    gx_ceiling_k1.b = ceiling_rgb;
    gx_ceiling_k1.a = 255;
    GXSetTevKColor(GX_KCOLOR1, gx_ceiling_k1);

    GXSetNumTevStages(n_tev + 1);
    GXSetTevOrder(clamp_st, GX_TEXCOORD0, GX_TEXMAP_NULL, GX_COLOR_NULL);
    // min(CPREV, K1): COMP_RGB8_GT selects KONST when CPREV > ceiling else CPREV.
    GXSetTevColorIn(clamp_st, GX_CC_CPREV, GX_CC_KONST, GX_CC_CPREV, GX_CC_KONST);
    GXSetTevColorOp(clamp_st, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevKColorSel(clamp_st, GX_TEV_KCSEL_K1);
    GXSetTevAlphaIn(clamp_st, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevAlphaOp(clamp_st, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevKAlphaSel(clamp_st, GX_TEV_KASEL_K0_A);
    GXSetTevSwapMode(clamp_st, GX_TEV_SWAP0, GX_TEV_SWAP0);

    static bool s_logged_ceiling = false;
    if (!s_logged_ceiling) {
        char line[160];
        snprintf(line, sizeof(line),
                 "4b sc-out-ceiling: clamp rgb ceiling=%u (cosmetic bloom threshold, K0=%u)",
                 ceiling_rgb, wwBowScK0CapRgb());
        dWwItemmdl_debugLog(line);
        s_logged_ceiling = true;
    }
}
#endif
static J3DModelData* s_wwBowDrawModelData = NULL;
static bool s_wwBowDrawScopeActive = false;
static fpc_ProcID s_wwBowDrawOwnerId = fpcM_ERROR_PROCESS_ID_e;
static s32 s_wwBowDrawRoomNo = -1;

static void clearBowDrawScopeInternal(const char* reason) {
    if (!s_wwBowDrawScopeActive) {
        return;
    }

    s_wwBowDrawScopeActive = false;
    j3dSys.setMatDrawPostDlCallback(nullptr);
    s_wwBowDrawModelData = NULL;
    s_wwBowDrawOwnerId = fpcM_ERROR_PROCESS_ID_e;
    s_wwBowDrawRoomNo = -1;

    if (reason != NULL) {
        char line[128];
        snprintf(line, sizeof(line), "4A: clearBowDrawScope (%s)", reason);
        dWwItemmdl_debugLog(line);
    }
}

static bool privateItemmdlArcReady() {
    return s_privateItemmdlState == 2;
}

static void* getPrivateObjectRes(const char* res_name) {
    return dRes_control_c::getRes(kItemmdlArcName, res_name, s_privateItemmdlInfo,
                                    kPrivateItemmdlInfoNum);
}

static void* getPrivateObjectResByIndex(s32 index) {
    return dRes_control_c::getRes(kItemmdlArcName, index, s_privateItemmdlInfo,
                                  kPrivateItemmdlInfoNum);
}

static dRes_info_c* getPrivateItemmdlResInfo() {
    return dRes_control_c::getResInfoLoaded(kItemmdlArcName, s_privateItemmdlInfo,
                                            kPrivateItemmdlInfoNum);
}

static bool ensurePrivateItemmdlRetainedHeap() {
    if (s_vbowRetainedHeap != NULL) {
        return true;
    }

    s_vbowRetainedHeap = mDoExt_createSolidHeapFromGame(kVbowRetainedHeapSize, 0x20);
    if (s_vbowRetainedHeap == NULL) {
        dWwItemmdl_debugLog("2R: retained heap alloc FAILED");
        return false;
    }

    JKRHEAP_NAMEF(s_vbowRetainedHeap, "WW itemmdl vbow");
    return true;
}

static bool stepPrivateItemmdlArcMount() {
    if (s_privateItemmdlState == 2) {
        return true;
    }
    if (s_privateItemmdlState == 3) {
        return false;
    }

    if (!ensurePrivateItemmdlRetainedHeap()) {
        s_privateItemmdlState = 3;
        return false;
    }

    if (s_privateItemmdlState == 0) {
        if (dRes_control_c::setRes(kItemmdlArcName, s_privateItemmdlInfo, kPrivateItemmdlInfoNum,
                                    "/res/Object/", 0, s_vbowRetainedHeap) == 0) {
            dWwItemmdl_debugLog("2R: private setRes FAILED");
            s_privateItemmdlState = 3;
            return false;
        }
        s_privateItemmdlState = 1;
        dWwItemmdl_debugLog("2R: private itemmdl mount started");
    }

    if (s_privateItemmdlState == 1) {
        const int sync =
            dRes_control_c::syncRes(kItemmdlArcName, s_privateItemmdlInfo, kPrivateItemmdlInfoNum);
        if (sync < 0) {
            dWwItemmdl_debugLog("2R: private syncRes ERROR");
            s_privateItemmdlState = 3;
            return false;
        }
        if (sync > 0) {
            return false;
        }

        s_privateItemmdlState = 2;
        dWwItemmdl_debugLog("2R: private itemmdl arc ready");
    }

    return s_privateItemmdlState == 2;
}

// 2O: cross-spawn arc buffer audit (log-only; confirms in-place double-fixup theory).
static u32 s_2oLoadSerial = 0;
static void* s_2oPrevResPtr = NULL;

static void log2OBlock0(const char* tag, u32 serial, const void* res) {
    if (res == NULL) {
        dWwItemmdl_debugLog("2O: res NULL");
        return;
    }

    const u8* bytes = static_cast<const u8*>(res);
    char hdr[128];
    snprintf(hdr, sizeof(hdr),
             "2O [%s] #%u hdr16=%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
             tag, serial, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6],
             bytes[7], bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14],
             bytes[15]);
    dWwItemmdl_debugLog(hdr);

    if (!isRawJ3dBinaryDisplayListFile(res) && !isRawJ3dModelFile(res)) {
        dWwItemmdl_debugLog("2O: not raw J3D2 bdl/bmd — skip block0");
        return;
    }

    const J3DModelFileData* file = static_cast<const J3DModelFileData*>(res);
    const J3DModelBlock* blk0 = &file->mBlocks[0];
    const u32 block_num = file->mBlockNum;
    const u32 blk0_type = blk0->mBlockType;
    const u32 blk0_size = blk0->mBlockSize;
    char type_chars[5];
    type_chars[0] = static_cast<char>((blk0_type >> 24) & 0xFF);
    type_chars[1] = static_cast<char>((blk0_type >> 16) & 0xFF);
    type_chars[2] = static_cast<char>((blk0_type >> 8) & 0xFF);
    type_chars[3] = static_cast<char>(blk0_type & 0xFF);
    type_chars[4] = '\0';

    const bool size_looks_resolved = blk0_size >= 0x10000u;
    char line[192];
    snprintf(line, sizeof(line),
             "2O [%s] #%u blockNum=%u blk0=%s size=%u size_looks_resolved=%d", tag, serial,
             block_num, type_chars, blk0_size, size_looks_resolved ? 1 : 0);
    dWwItemmdl_debugLog(line);
}

static void log2OResourceAudit(const char* via, void* res) {
    const u32 serial = ++s_2oLoadSerial;
    const bool same_as_prev = res != NULL && res == s_2oPrevResPtr;

    char intro[160];
    snprintf(intro, sizeof(intro), "2O res via=%s #%u ptr=%p same_as_prev=%d", via, serial, res,
             same_as_prev ? 1 : 0);
    dWwItemmdl_debugLog(intro);

    log2OBlock0("pre-parse", serial, res);
    s_2oPrevResPtr = res;
}

static void bakeLockedMaterialSharedDl(J3DModelData* model_data);

static J3DModelData* finishLoadedModelData(J3DModelData* model_data) {
    if (model_data == NULL || model_data->getMaterialNum() == 0) {
        return NULL;
    }

    logTexGenDump(model_data, "after loadBinaryDisplayList");
    logTevOrderDumpInternal(model_data, "after loadBinaryDisplayList", -1, false);

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
    bakeLockedMaterialSharedDl(model_data);
    logTexGenDump(model_data, "after makeSharedDL");
    logTevOrderDumpInternal(model_data, "after makeSharedDL", -1, false);
    return model_data;
}

static J3DModelData* loadFromResourcePointer(void* res, const char* via) {
    if (res == NULL) {
        dWwItemmdl_debugLog("2O res via=getObjectRes: NULL");
        return NULL;
    }

    log2OResourceAudit(via != NULL ? via : "unknown", res);

    if (isRawJ3dModelFile(res)) {
        dWwItemmdl_debugLog("2O loadBinaryDisplayList: skip (raw bmd)");
        return finishLoadedModelData(J3DModelLoaderDataBase::load(res, 0x59020010));
    }

    if (isRawJ3dBinaryDisplayListFile(res)) {
        char enter[96];
        snprintf(enter, sizeof(enter), "2O loadBinaryDisplayList: enter serial=%u",
                 s_2oLoadSerial);
        dWwItemmdl_debugLog(enter);

        J3DModelData* model_data =
            J3DModelLoaderDataBase::loadBinaryDisplayList(res, kItemmdlBdlLoadFlags);

        char exit_line[128];
        snprintf(exit_line, sizeof(exit_line),
                 "2O loadBinaryDisplayList: return serial=%u model=%p", s_2oLoadSerial,
                 model_data);
        dWwItemmdl_debugLog(exit_line);
        log2OBlock0("post-parse", s_2oLoadSerial, res);

        return finishLoadedModelData(model_data);
    }

    dWwItemmdl_debugLog("2O res: not raw J3D2 file (maybe instantiated J3DModelData*)");

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

static J3DModelData* tryNamedResource(const char* res_name, const char* log_label) {
    void* res = getPrivateObjectRes(res_name);
    char via[96];
    snprintf(via, sizeof(via), "privateRes(%s)", res_name);
    J3DModelData* model_data = loadFromResourcePointer(res, via);
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

static J3DModelData* scanItemmdlArchiveForVbow() {
    dRes_info_c* info = getPrivateItemmdlResInfo();
    if (info == NULL || info->getArchive() == NULL) {
        dWwItemmdl_debugLog("scan: private itemmdl archive not resident");
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

        char via[96];
        snprintf(via, sizeof(via), "scan(%s)", name);
        J3DModelData* model_data = loadFromResourcePointer(res, via);
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

static void logTexGenDump(J3DModelData* model_data, const char* phase) {
    if (model_data == NULL) {
        return;
    }

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 mat_idx = 0; mat_idx < model_data->getMaterialNum(); mat_idx++) {
        J3DMaterial* material = model_data->getMaterialNodePointer(mat_idx);
        if (material == NULL) {
            continue;
        }

        const char* mat_name = names != NULL ? names->getName(mat_idx) : "?";
        const u32 tex_gen_num = material->getTexGenNum();
        char header[160];
        snprintf(header, sizeof(header), "2I texgen [%s]: mat[%u]=%s nTexGen=%u", phase, mat_idx,
                 mat_name != NULL ? mat_name : "?", tex_gen_num);
        dWwItemmdl_debugLog(header);

        for (u32 tg = 0; tg < tex_gen_num && tg < 8; tg++) {
            J3DTexCoord* coord = material->getTexCoord(tg);
            if (coord == NULL) {
                char line[128];
                snprintf(line, sizeof(line), "2I texgen [%s]: mat[%u] tg[%u]=NULL", phase, mat_idx,
                         tg);
                dWwItemmdl_debugLog(line);
                continue;
            }

            const u8 src = coord->getTexGenSrc();
            const u8 type = coord->getTexGenType();
            char line[160];
            snprintf(line, sizeof(line),
                     "2I texgen [%s]: mat[%u] tg[%u] type=%u src=%u%s", phase, mat_idx, tg, type,
                     src, src >= 21 ? " PHANTOM" : "");
            dWwItemmdl_debugLog(line);
        }
    }
}

// Generalized: any named material of a scoped itemmdl model gets WW realization / ambient fill.
// (Was bow-only Vbow_v/SC_Vbow_v. Every caller iterates an itemmdl model's own materials, so
// accepting all of them is what makes the render core item-agnostic.)
static bool isTevDumpMaterial(const char* name) {
    return name != NULL;
}

static void logTevOrderMaterialDump(J3DModelData* model_data, const char* phase, s32 room_no) {
    if (model_data == NULL) {
        return;
    }

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 mat_idx = 0; mat_idx < model_data->getMaterialNum(); mat_idx++) {
        const char* mat_name = names != NULL ? names->getName(mat_idx) : NULL;
        if (!isTevDumpMaterial(mat_name)) {
            continue;
        }

        J3DMaterial* material = model_data->getMaterialNodePointer(mat_idx);
        if (material == NULL) {
            continue;
        }

        J3DTevBlock* tev_block = material->getTevBlock();
        if (tev_block == NULL) {
            char line[160];
            snprintf(line, sizeof(line), "2B tevorder [%s room=%d]: mat[%u]=%s tevBlock=NULL",
                     phase, room_no, mat_idx, mat_name != NULL ? mat_name : "?");
            dWwItemmdl_debugLog(line);
            continue;
        }

        const u8 tev_stage_num = tev_block->getTevStageNum();
        char header[192];
        snprintf(header, sizeof(header),
                 "2B tevorder [%s room=%d]: mat[%u]=%s nTev=%u", phase, room_no, mat_idx,
                 mat_name != NULL ? mat_name : "?", tev_stage_num);
        dWwItemmdl_debugLog(header);

        for (u32 st = 0; st < tev_stage_num && st < 16; st++) {
            J3DTevOrder* order = tev_block->getTevOrder(st);
            if (order == NULL) {
                char line[160];
                snprintf(line, sizeof(line),
                         "2B tevorder [%s room=%d]: mat[%u] st[%u] struct order=NULL", phase,
                         room_no, mat_idx, st);
                dWwItemmdl_debugLog(line);
                continue;
            }

            const u8 tex_map = order->getTexMap();
            const u8 tex_coord = order->mTexCoord;
            const u8 color_chan = order->mColorChan;
            char line[224];
            if (tex_map == 0xFF && tex_coord == 0xFF) {
                snprintf(line, sizeof(line),
                         "2B tevorder [%s room=%d]: mat[%u] st[%u] struct texMap=NULL texCoord=NULL "
                         "colorChan=%u",
                         phase, room_no, mat_idx, st, color_chan);
            } else {
                snprintf(line, sizeof(line),
                         "2B tevorder [%s room=%d]: mat[%u] st[%u] struct texMap=%u texCoord=%u "
                         "colorChan=%u",
                         phase, room_no, mat_idx, st, tex_map, tex_coord, color_chan);
            }
            dWwItemmdl_debugLog(line);
        }
    }
}

static void logTevOrderGxDump(const char* phase, s32 room_no) {
    using aurora::gx::g_gxState;

    const u8 num_tev = g_gxState.numTevStages;
    char header[128];
    snprintf(header, sizeof(header), "2B tevorder [%s room=%d]: gx numTev=%u", phase, room_no,
             num_tev);
    dWwItemmdl_debugLog(header);

    const u32 stage_limit = num_tev > 0 ? num_tev : 16u;
    for (u32 st = 0; st < stage_limit && st < 16; st++) {
        const aurora::gx::TevStage& stage = g_gxState.tevStages[st];
        const u8 tex_map = static_cast<u8>(stage.texMapId);
        const u8 tex_coord = static_cast<u8>(stage.texCoordId);
        const u8 color_chan = static_cast<u8>(stage.channelId);
        char line[224];
        if (tex_map == 0xFF && tex_coord == 0xFF) {
            snprintf(line, sizeof(line),
                     "2B tevorder [%s room=%d]: gx st[%u] texMapId=NULL texCoordId=NULL "
                     "channelId=%u",
                     phase, room_no, st, color_chan);
        } else {
            snprintf(line, sizeof(line),
                     "2B tevorder [%s room=%d]: gx st[%u] texMapId=%u texCoordId=%u channelId=%u",
                     phase, room_no, st, tex_map, tex_coord, color_chan);
        }
        dWwItemmdl_debugLog(line);
    }
}

static void logTevOrderDumpInternal(J3DModelData* model_data, const char* phase, s32 room_no,
                                    bool log_gx_runtime) {
    logTevOrderMaterialDump(model_data, phase, room_no);
    if (log_gx_runtime) {
        logTevOrderGxDump(phase, room_no);
    }
}

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

static void bakeLockedMaterialSharedDl(J3DModelData* model_data) {
    if (model_data == NULL) {
        return;
    }

    static J3DModelData* s_baked_model = NULL;
    if (s_baked_model == model_data) {
        return;
    }

    j3dSys.setTexture(model_data->getTexture());
    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        J3DMaterial* material = model_data->getMaterialNodePointer(i);
        if (material == NULL || material->getSharedDisplayListObj() == NULL) {
            continue;
        }

        // BDL locked materials override makeSharedDisplayList with a no-op; without this bake
        // callDL replays an empty DL → numTexGens stays 0 while TEV samples texcoord 2 (tcg src 21).
        material->J3DMaterial::makeSharedDisplayList();

        const char* name = names != NULL ? names->getName(i) : NULL;
        char message[128];
        snprintf(message, sizeof(message), "2N': baked locked shared DL mat[%u]=%s nTexGen=%u nTev=%u",
                 i, name != NULL ? name : "?", material->getTexGenNum(),
                 material->getTevStageNum());
        dWwItemmdl_debugLog(message);
    }

    s_baked_model = model_data;
}

// ============================================
// NEW CODE — ALBW Port (custom vbow BMD: Layer-B override)
// Set when the held/get-item vbow came from a loose rebuilt .bmd instead of the
// retail BDL. A rebuilt BMD has NO MDL3 locked display lists — its materials load
// through J3D's runtime path, which Aurora renders natively — so the draw-scope
// TEV replay (and its white-konst bloom) is unnecessary and the boots-style
// ambient-only draw is used instead (see daAlink_c held-skin branch).
// ============================================
static J3DModelData* s_customVbowModelData = NULL;

static J3DModelData* loadVbowFromPrivateArcOnce() {
    // Layer-B first: <mods>/itemmdl_15.bmd (vbow, index 0xF). GameHeap pin + material
    // guard live inside try_load. Retail vbow.bdl (locked-DL + scope) is the fallback.
    {
        J3DModelData* custom = dusk::custom_assets::try_load(
            kItemmdlArcName, static_cast<int>(dRes_INDEX_ITEMMDL_BDL_VBOW_e));
        if (custom != NULL) {
            s_customVbowModelData = custom;
            dWwItemmdl_debugLog("getVbowModelData: CUSTOM loose BMD active (no draw scope)");
            return custom;
        }
        s_customVbowModelData = NULL;
    }

    static const char* const kNames[] = {"vbow.bdl", "vbow", "vbow.bmd", "Vbow.bdl", "Vbow",
                                         "VBOW.bdl", NULL};
    for (int i = 0; kNames[i] != NULL; i++) {
        J3DModelData* model_data = tryNamedResource(kNames[i], "name");
        if (model_data != NULL) {
            dWwItemmdl_logHeap("after load (named)");
            return model_data;
        }
    }

    J3DModelData* model_data = loadFromResourcePointer(
        getPrivateObjectResByIndex(static_cast<s32>(dRes_INDEX_ITEMMDL_BDL_VBOW_e)),
        "privateRes(index vbow)");
    if (model_data != NULL) {
        dWwItemmdl_debugLog("getVbowModelData: loaded via private file index");
        return model_data;
    }

    model_data = scanItemmdlArchiveForVbow();
    if (model_data != NULL) {
        return model_data;
    }

    dWwItemmdl_debugLog(
        "getVbowModelData: failed — ensure retail itemmdl.arc is in res/Object/ (vbow.bdl)");
    return NULL;
}

}  // namespace

// ============================================
// NEW CODE — ALBW Port (custom vbow BMD: Layer-B override)
// True when i_model is backed by the loose rebuilt vbow BMD — the caller should
// use the boots-style ambient-only draw (no scope) for it.
// ============================================
bool dWwItemmdl_usingCustomHeldModel(J3DModel* i_model) {
    return s_customVbowModelData != NULL && i_model != NULL &&
           i_model->getModelData() == s_customVbowModelData;
}

int dWwItemmdl_stepPrivateItemmdlArcLoad(request_of_phase_process_class* phase) {
    if (!dusk::getSettings().game.wwItemmdlGetItem.getValue() ||
        dusk::getSettings().game.wwItemmdlGetItem2DIsolate.getValue()) {
        return -1;
    }

    if (stepPrivateItemmdlArcMount()) {
        if (phase != NULL) {
            phase->id = 2;
        }
        return cPhs_COMPLEATE_e;
    }

    if (s_privateItemmdlState == 3) {
        return cPhs_ERROR_e;
    }

    return cPhs_INIT_e;
}

void dWwItemmdl_log2QPrimeAudit(const char* tag, J3DModelData* model_data, s32 room_no) {
    const void* cached_raw = model_data != NULL ? model_data->getRawData() : NULL;
    void* shared_live = dComIfG_getObjectRes(kItemmdlArcName, "vbow.bdl");
    void* private_live = privateItemmdlArcReady() ? getPrivateObjectRes("vbow.bdl") : NULL;

    dRes_info_c* shared_info = dComIfG_getObjectResInfo(kItemmdlArcName);
    const bool shared_arc =
        shared_info != NULL && shared_info->getArchive() != NULL && shared_info->getCount() > 0;
    const bool private_arc = getPrivateItemmdlResInfo() != NULL;

    const int raw_ne_shared =
        cached_raw != NULL && shared_live != NULL && cached_raw != shared_live ? 1 : 0;
    const int raw_eq_private =
        cached_raw != NULL && private_live != NULL && cached_raw == private_live ? 1 : 0;
    const int parse_ptr_match =
        s_parseRawPtr != NULL && cached_raw != NULL && cached_raw == s_parseRawPtr ? 1 : 0;

    char message[320];
    snprintf(message, sizeof(message),
             "2Q' [%s] room=%d cached_mpRawData=%p parse_raw=%p shared_live=%p private_live=%p "
             "shared_arc=%d private_arc=%d raw_ne_shared=%d raw_eq_private=%d parse_match=%d",
             tag != NULL ? tag : "?", room_no, cached_raw, s_parseRawPtr, shared_live,
             private_live, shared_arc ? 1 : 0, private_arc ? 1 : 0, raw_ne_shared,
             raw_eq_private, parse_ptr_match);
    dWwItemmdl_debugLog(message);
}

J3DModelData* dWwItemmdl_getVbowModelData(const char* arc_name) {
    (void)arc_name;

    if (!privateItemmdlArcReady()) {
        dWwItemmdl_debugLog("2R: getVbow before private arc ready");
        return NULL;
    }

    if (s_cachedVbow != NULL) {
        if (s_cachedVbow->getMaterialNum() == 2 && s_cachedVbow->getJointNum() == 2) {
            dWwItemmdl_bracketLog("2Q: cache HIT");
            dWwItemmdl_log2QPrimeAudit("cache-hit", s_cachedVbow, -1);
            return s_cachedVbow;
        }

        dWwItemmdl_debugLog("2Q: cache invalid — clearing");
        s_cachedVbow = NULL;
        s_parseRawPtr = NULL;
    }

    if (s_vbowParseFailed) {
        dWwItemmdl_debugLog("2Q: prior parse failed — skip");
        return NULL;
    }

    dWwItemmdl_bracketLog("2Q: cache MISS — parse once (2R private arc)");
    dWwItemmdl_logHeap("before parse");

    JKRHeap* const prev_heap = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_vbowRetainedHeap);

    J3DModelData* model_data = loadVbowFromPrivateArcOnce();

    mDoExt_setCurrentHeap(prev_heap);

    if (model_data != NULL) {
        s_cachedVbow = model_data;
        s_parseRawPtr = model_data->getRawData();
        char message[160];
        snprintf(message, sizeof(message), "2Q: parse OK heap=%p model=%p mats=%u joints=%u raw=%p",
                 s_vbowRetainedHeap, model_data, model_data->getMaterialNum(),
                 model_data->getJointNum(), s_parseRawPtr);
        dWwItemmdl_debugLog(message);
        dWwItemmdl_log2QPrimeAudit("parse-ok", model_data, -1);
    } else {
        s_vbowParseFailed = true;
        dWwItemmdl_debugLog("2Q: parse FAILED");
    }

    return model_data;
}

// Generic itemmdl loader — any BDL index off the same private arc, cached per-index. The bow keeps
// its proven, battle-tested path (getVbowModelData); this is the entry point for every OTHER item.
static const int kItemmdlCacheMax = 16;
static u16 s_itemmdlCacheIdx[kItemmdlCacheMax];
static J3DModelData* s_itemmdlCacheModel[kItemmdlCacheMax];
static int s_itemmdlCacheCount = 0;

J3DModelData* dWwItemmdl_getItemmdlModelData(u16 bdl_index) {
    if (bdl_index == static_cast<u16>(dRes_INDEX_ITEMMDL_BDL_VBOW_e)) {
        return dWwItemmdl_getVbowModelData(kItemmdlArcName);  // proven bow path (2/2 mats+joints)
    }

    for (int i = 0; i < s_itemmdlCacheCount; i++) {
        if (s_itemmdlCacheIdx[i] == bdl_index) {
            return s_itemmdlCacheModel[i];
        }
    }

    if (!privateItemmdlArcReady()) {
        dWwItemmdl_debugLog("getItemmdlModelData: private arc not ready");
        return NULL;
    }

    JKRHeap* const prev_heap = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_vbowRetainedHeap);
    J3DModelData* model_data = loadFromResourcePointer(
        getPrivateObjectResByIndex(static_cast<s32>(bdl_index)), "itemmdl by index");
    mDoExt_setCurrentHeap(prev_heap);

    if (model_data != NULL && s_itemmdlCacheCount < kItemmdlCacheMax) {
        s_itemmdlCacheIdx[s_itemmdlCacheCount] = bdl_index;
        s_itemmdlCacheModel[s_itemmdlCacheCount] = model_data;
        s_itemmdlCacheCount++;
        char msg[96];
        snprintf(msg, sizeof(msg), "getItemmdlModelData: loaded index=%u model=%p mats=%u joints=%u",
                 bdl_index, model_data, model_data->getMaterialNum(), model_data->getJointNum());
        dWwItemmdl_debugLog(msg);
    }
    return model_data;
}

// Track B: while the held-bow skin toggle is on, drive the private itemmdl arc mount to
// completion (stepPrivateItemmdlArcMount is a per-frame stepper) so setBowModel() can pull
// vbow via getVbowModelData without requiring a prior get-item replay.
void dWwItemmdl_tickHeldBowArcMount() {
    if (dusk::getSettings().game.wwItemmdlHeldSkin.getValue() == dusk::WwHeldSkinMode::Off) {
        return;
    }
    // state 2 = ready, 3 = failed — nothing more to do either way.
    if (s_privateItemmdlState == 2 || s_privateItemmdlState == 3) {
        return;
    }
    stepPrivateItemmdlArcMount();
}

void dWwItemmdl_patchModelForPc(J3DModelData* model_data) {
    logModelSummary(model_data, "patchModel");
    static bool s_logged_patch_tevorder = false;
    if (!s_logged_patch_tevorder) {
        logTevOrderDumpInternal(model_data, "patchModel", -1, false);
        dWwItemmdl_logShapeInventory(model_data, "patchModel");
        s_logged_patch_tevorder = true;
    }
    dWwItemmdl_debugLog("patchModel: done");
}

void dWwItemmdl_logTevOrderDump(J3DModelData* model_data, const char* phase, s32 room_no,
                                  bool log_gx_runtime) {
    logTevOrderDumpInternal(model_data, phase, room_no, log_gx_runtime);
}

static void applyTevOrderFromMaterial(J3DMaterial* material) {
    if (material == NULL) {
        return;
    }

    J3DTevBlock* tev_block = material->getTevBlock();
    if (tev_block == NULL) {
        return;
    }

    const u8 n_tev = tev_block->getTevStageNum();
    if (n_tev == 0) {
        return;
    }

    for (u32 st = 0; st < n_tev && st < 16; st++) {
        J3DTevOrder* order = tev_block->getTevOrder(st);
        if (order == NULL) {
            continue;
        }

        const GXTexCoordID tex_coord = static_cast<GXTexCoordID>(order->mTexCoord);
        const GXTexMapID tex_map = static_cast<GXTexMapID>(order->getTexMap());
        GXSetTevOrder(static_cast<GXTevStageID>(GX_TEVSTAGE0 + st), tex_coord, tex_map,
                      GX_COLOR_NULL);
    }

    GXSetNumTevStages(n_tev);
}

static void applyTexGenFromMaterial(J3DMaterial* material) {
    if (material == NULL) {
        return;
    }

    const u32 tex_gen_num = material->getTexGenNum();
    if (tex_gen_num == 0) {
        return;
    }

    GXSetNumTexGens(tex_gen_num);
    for (u32 tg = 0; tg < tex_gen_num && tg < 8; tg++) {
        J3DTexCoord* coord = material->getTexCoord(tg);
        if (coord == NULL) {
            continue;
        }

        GXSetTexCoordGen(static_cast<GXTexCoordID>(GX_TEXCOORD0 + tg),
                         static_cast<GXTexGenType>(coord->getTexGenType()),
                         static_cast<GXTexGenSrc>(coord->getTexGenSrc()), coord->getTexGenMtx());
    }
}

// Decode baked TEV op (full combiner register — matches Aurora command_processor, not J2D byte).
static void decodeScTevOpFromCombinerBytes(u8 op_byte, u8 ab_byte, u8 cd_byte, GXTevOp* out_op,
                                           GXTevBias* out_bias, GXTevScale* out_scale) {
    const u32 reg =
        (static_cast<u32>(op_byte) << 16) | (static_cast<u32>(ab_byte) << 8) | cd_byte;
    const u32 bias_field = (reg >> 16) & 3;
    if (bias_field == 3) {
        const u32 hw_op = ((reg >> 18) & 1) | (((reg >> 20) & 3) << 1);
        *out_op = static_cast<GXTevOp>(hw_op + 8);
        *out_bias = GX_TB_ZERO;
        *out_scale = GX_CS_SCALE_1;
    } else {
        *out_op = static_cast<GXTevOp>((reg >> 18) & 1);
        *out_bias = static_cast<GXTevBias>(bias_field);
        *out_scale = static_cast<GXTevScale>((reg >> 20) & 3);
    }
}

static void decodeScTevColorOpFromStageBytes(const J3DTevStage& stage, GXTevOp* out_op,
                                             GXTevBias* out_bias, GXTevScale* out_scale) {
    decodeScTevOpFromCombinerBytes(stage.mTevColorOp, stage.mTevColorAB, stage.mTevColorCD, out_op,
                                   out_bias, out_scale);
}

static void decodeScTevAlphaOpFromStageBytes(const J3DTevStage& stage, GXTevOp* out_op,
                                             GXTevBias* out_bias, GXTevScale* out_scale) {
    decodeScTevOpFromCombinerBytes(stage.mTevAlphaOp, stage.mTevAlphaAB, stage.mTevSwapModeInfo,
                                   out_op, out_bias, out_scale);
}

struct WwBowJ3DTevStageView {
    explicit WwBowJ3DTevStageView(const J3DTevStage& stage) : mStage(stage) {}

    u8 getColorA() const { return static_cast<u8>((mStage.mTevColorAB >> 4) & 0x0F); }
    u8 getColorB() const { return static_cast<u8>(mStage.mTevColorAB & 0x0F); }
    u8 getColorC() const { return static_cast<u8>((mStage.mTevColorCD >> 4) & 0x0F); }
    u8 getColorD() const { return static_cast<u8>(mStage.mTevColorCD & 0x0F); }
    u8 getAlphaA() const { return static_cast<u8>((mStage.mTevAlphaAB >> 5) & 0x07); }
    u8 getAlphaB() const { return static_cast<u8>((mStage.mTevAlphaAB >> 2) & 0x07); }
    u8 getAlphaC() const {
        return static_cast<u8>(((mStage.mTevAlphaAB & 0x03) << 1) | ((mStage.mTevSwapModeInfo >> 7) & 0x01));
    }
    u8 getAlphaD() const { return static_cast<u8>((mStage.mTevSwapModeInfo >> 4) & 0x07); }
    u8 getCClamp() const { return static_cast<u8>((mStage.mTevColorOp >> 3) & 0x01); }
    u8 getCReg() const { return static_cast<u8>((mStage.mTevColorOp >> 6) & 0x03); }
    u8 getAClamp() const { return static_cast<u8>((mStage.mTevAlphaOp >> 3) & 0x01); }
    u8 getAReg() const { return static_cast<u8>((mStage.mTevAlphaOp >> 6) & 0x03); }
    u8 getRasSel() const { return static_cast<u8>(mStage.mTevSwapModeInfo & 0x03); }
    u8 getTexSel() const { return static_cast<u8>((mStage.mTevSwapModeInfo >> 2) & 0x03); }
    void getCOpBiasScale(GXTevOp* op, GXTevBias* bias, GXTevScale* scale) const {
        decodeScTevColorOpFromStageBytes(mStage, op, bias, scale);
    }
    void getAOpBiasScale(GXTevOp* op, GXTevBias* bias, GXTevScale* scale) const {
        decodeScTevAlphaOpFromStageBytes(mStage, op, bias, scale);
    }

    const J3DTevStage& mStage;
};

static bool isScVbowDrawMaterial(J3DMaterial* material) {
    if (material == NULL || s_wwBowDrawModelData == NULL) {
        return false;
    }

    JUTNameTab* names = s_wwBowDrawModelData->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < s_wwBowDrawModelData->getMaterialNum(); i++) {
        if (s_wwBowDrawModelData->getMaterialNodePointer(i) != material) {
            continue;
        }

        const char* name = names != NULL ? names->getName(i) : NULL;
        // WW cel convention: an "SC_"-prefixed material is the 2nd-pass ink/edge that needs the
        // full TEV/konst replay. Generalized from the bow's exact "SC_Vbow_v".
        return name != NULL && strncmp(name, "SC_", 3) == 0;
    }

    return false;
}

static void applyPeFromMaterial(J3DMaterial* material) {
    if (material == NULL) {
        return;
    }

    J3DBlend* blend = material->getBlend();
    if (blend != NULL) {
        GXSetBlendMode(blend->getBlendMode(), blend->getSrcFactor(), blend->getDstFactor(),
                       blend->getLogicOp());
    }

    J3DPEBlock* pe = material->getPEBlock();
    if (pe == NULL) {
        return;
    }

    J3DAlphaComp* alpha = pe->getAlphaComp();
    if (alpha != NULL) {
        GXSetAlphaCompare(static_cast<GXCompare>(alpha->getComp0()), alpha->getRef0(),
                          static_cast<GXAlphaOp>(alpha->getOp()),
                          static_cast<GXCompare>(alpha->getComp1()), alpha->getRef1());
    }
}

// 4a: replay baked SC TEV/PE into Aurora after callDL.
// 4b: cosmetic K0 RGB cap (st[0] white-konst ceiling → matte silver).
static void applyScAuthenticTevAndPeFromMaterial(J3DMaterial* material) {
    if (material == NULL) {
        return;
    }

    J3DTevBlock* tev_block = material->getTevBlock();
    if (tev_block == NULL) {
        return;
    }

    if (material->isDrawModeOpaTexEdge()) {
        j3dSys.setDrawModeOpaTexEdge();
    }

    applyPeFromMaterial(material);

    for (u32 reg = 0; reg < 3; reg++) {
        J3DGXColorS10* color = material->getTevColor(reg);
        if (color == NULL) {
            continue;
        }

        GXColorS10 gx_color;
        gx_color.r = color->r;
        gx_color.g = color->g;
        gx_color.b = color->b;
        gx_color.a = color->a;
        GXSetTevColorS10(reg != 2 ? static_cast<GXTevRegID>(GX_TEVREG0 + reg + 1) : GX_TEVPREV,
                         gx_color);
    }

    const u8 k0_cap = wwBowScK0CapRgb();
    static bool s_logged_k0 = false;
    for (u32 reg = 0; reg < 4; reg++) {
        J3DGXColor* kcolor = material->getTevKColor(reg);
        if (kcolor == NULL) {
            continue;
        }

        GXColor gx_kcolor;
        gx_kcolor.r = kcolor->r;
        gx_kcolor.g = kcolor->g;
        gx_kcolor.b = kcolor->b;
        gx_kcolor.a = kcolor->a;
        if (reg == 0) {
            gx_kcolor.r = k0_cap;
            gx_kcolor.g = k0_cap;
            gx_kcolor.b = k0_cap;
            if (!s_logged_k0) {
                char line[128];
                snprintf(line, sizeof(line),
                         "4b sc-k0: K0 rgb=%u,%u,%u (cosmetic cap, baked 255,255,255)",
                         k0_cap, k0_cap, k0_cap);
                dWwItemmdl_debugLog(line);
                s_logged_k0 = true;
            }
        }
        GXSetTevKColor(static_cast<GXTevKColorID>(GX_KCOLOR0 + reg), gx_kcolor);
    }

    for (u32 table = 0; table < 4; table++) {
        J3DTevSwapModeTable* swap_table = tev_block->getTevSwapModeTable(table);
        if (swap_table == NULL) {
            continue;
        }

        GXSetTevSwapModeTable(static_cast<GXTevSwapSel>(GX_TEV_SWAP0 + table),
                              static_cast<GXTevColorChan>(swap_table->getR()),
                              static_cast<GXTevColorChan>(swap_table->getG()),
                              static_cast<GXTevColorChan>(swap_table->getB()),
                              static_cast<GXTevColorChan>(swap_table->getA()));
    }

    const u8 n_tev = tev_block->getTevStageNum();
    GXSetNumTevStages(n_tev);
    for (u32 st = 0; st < n_tev && st < 16; st++) {
        J3DTevOrder* order = tev_block->getTevOrder(st);
        if (order != NULL) {
            const u8 ras = order->mColorChan;
            const GXChannelID channel =
                ras >= 255 ? GX_COLOR_NULL : static_cast<GXChannelID>(ras);
            GXSetTevOrder(static_cast<GXTevStageID>(GX_TEVSTAGE0 + st),
                          static_cast<GXTexCoordID>(order->mTexCoord),
                          static_cast<GXTexMapID>(order->getTexMap()), channel);
        }

        J3DTevStage* stage = tev_block->getTevStage(st);
        if (stage == NULL) {
            continue;
        }

        const WwBowJ3DTevStageView view(*stage);
        const GXTevStageID stage_id = static_cast<GXTevStageID>(GX_TEVSTAGE0 + st);
        GXSetTevColorIn(stage_id, static_cast<GXTevColorArg>(view.getColorA()),
                        static_cast<GXTevColorArg>(view.getColorB()),
                        static_cast<GXTevColorArg>(view.getColorC()),
                        static_cast<GXTevColorArg>(view.getColorD()));
        GXSetTevAlphaIn(stage_id, static_cast<GXTevAlphaArg>(view.getAlphaA()),
                        static_cast<GXTevAlphaArg>(view.getAlphaB()),
                        static_cast<GXTevAlphaArg>(view.getAlphaC()),
                        static_cast<GXTevAlphaArg>(view.getAlphaD()));

        GXTevOp color_op;
        GXTevBias color_bias;
        GXTevScale color_scale;
        view.getCOpBiasScale(&color_op, &color_bias, &color_scale);
        GXSetTevColorOp(stage_id, color_op, color_bias, color_scale, view.getCClamp(),
                        static_cast<GXTevRegID>(view.getCReg()));

        GXTevOp alpha_op;
        GXTevBias alpha_bias;
        GXTevScale alpha_scale;
        view.getAOpBiasScale(&alpha_op, &alpha_bias, &alpha_scale);
        GXSetTevAlphaOp(stage_id, alpha_op, alpha_bias, alpha_scale, view.getAClamp(),
                        static_cast<GXTevRegID>(view.getAReg()));

        const u8 ksel = tev_block->getTevKColorSel(st);
        const u8 kasel = tev_block->getTevKAlphaSel(st);
        GXSetTevKColorSel(stage_id,
                          ksel != 0xFF ? static_cast<GXTevKColorSel>(ksel) : GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(stage_id,
                          kasel != 0xFF ? static_cast<GXTevKAlphaSel>(kasel) : GX_TEV_KASEL_K0_A);
        GXSetTevSwapMode(stage_id, static_cast<GXTevSwapSel>(view.getRasSel()),
                         static_cast<GXTevSwapSel>(view.getTexSel()));
    }

    applyScOutputCeilingTevStage(n_tev, wwBowScOutputCeilingRgb());

    static bool s_logged_4a = false;
    if (!s_logged_4a) {
        dWwItemmdl_debugLog("4a sc-tev: replay authentic SC TEV/PE (GXSetTev*, full-reg op decode)");
        s_logged_4a = true;
    }
}

static bool isVbowDrawMaterial(J3DMaterial* material) {
    if (material == NULL || s_wwBowDrawModelData == NULL) {
        return false;
    }

    JUTNameTab* names = s_wwBowDrawModelData->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < s_wwBowDrawModelData->getMaterialNum(); i++) {
        if (s_wwBowDrawModelData->getMaterialNodePointer(i) != material) {
            continue;
        }

        const char* name = names != NULL ? names->getName(i) : NULL;
        return isTevDumpMaterial(name);
    }

    return false;
}

static bool wwBowDrawScopeStillValid() {
    if (!s_wwBowDrawScopeActive || s_wwBowDrawModelData == NULL) {
        return false;
    }

    if (s_wwBowDrawOwnerId != fpcM_ERROR_PROCESS_ID_e && !fpcM_IsExecuting(s_wwBowDrawOwnerId)) {
        clearBowDrawScopeInternal("owner not executing");
        return false;
    }

    return true;
}

static u8 tevStageColorA(const J3DTevStage& st) {
    return static_cast<u8>((st.mTevColorAB >> 4) & 0x0F);
}

static u8 tevStageColorB(const J3DTevStage& st) {
    return static_cast<u8>(st.mTevColorAB & 0x0F);
}

static u8 tevStageColorC(const J3DTevStage& st) {
    return static_cast<u8>((st.mTevColorCD >> 4) & 0x0F);
}

static u8 tevStageColorD(const J3DTevStage& st) {
    return static_cast<u8>(st.mTevColorCD & 0x0F);
}

static u8 tevStageAlphaA(const J3DTevStage& st) {
    return static_cast<u8>((st.mTevAlphaAB >> 5) & 0x07);
}

static u8 tevStageAlphaB(const J3DTevStage& st) {
    return static_cast<u8>((st.mTevAlphaAB >> 2) & 0x07);
}

static u8 tevStageAlphaC(const J3DTevStage& st) {
    return static_cast<u8>(((st.mTevAlphaAB & 0x03) << 1) | ((st.mTevSwapModeInfo >> 7) & 0x01));
}

static u8 tevStageAlphaD(const J3DTevStage& st) {
    return static_cast<u8>((st.mTevSwapModeInfo >> 4) & 0x07);
}

// 2J: log-only SC_Vbow_v blend + TEV konst dump (decides de-bloom fix; no render change).
static void logScVbowMaterialDump(J3DMaterial* material) {
    static bool s_logged = false;
    if (s_logged || material == NULL) {
        return;
    }
    s_logged = true;

    J3DBlend* blend = material->getBlend();
    if (blend != NULL) {
        char line[224];
        snprintf(line, sizeof(line),
                 "2J sc-dump blend: type=%u src=%u dst=%u logicOp=%u matMode=0x%x opaTexEdge=%d",
                 blend->mType, blend->mSrcFactor, blend->mDstFactor, blend->mOp,
                 material->getMaterialMode(), material->isDrawModeOpaTexEdge() ? 1 : 0);
        dWwItemmdl_debugLog(line);
    } else {
        dWwItemmdl_debugLog("2J sc-dump blend: NULL");
    }

    J3DTevBlock* tev_block = material->getTevBlock();
    if (tev_block == NULL) {
        dWwItemmdl_debugLog("2J sc-dump: tevBlock=NULL");
        return;
    }

    for (u32 reg = 0; reg < 3; reg++) {
        J3DGXColorS10* color = material->getTevColor(reg);
        if (color == NULL) {
            continue;
        }

        char line[160];
        snprintf(line, sizeof(line), "2J sc-dump tevReg[%u]: r=%d g=%d b=%d a=%d", reg, color->r,
                 color->g, color->b, color->a);
        dWwItemmdl_debugLog(line);
    }

    for (u32 reg = 0; reg < 4; reg++) {
        J3DGXColor* kcolor = material->getTevKColor(reg);
        if (kcolor == NULL) {
            continue;
        }

        const u8 ksel = tev_block->getTevKColorSel(reg);
        char line[192];
        snprintf(line, sizeof(line), "2J sc-dump kColor[%u]: r=%u g=%u b=%u a=%u kSel=%u", reg,
                 kcolor->r, kcolor->g, kcolor->b, kcolor->a, ksel);
        dWwItemmdl_debugLog(line);
    }

    const u8 n_tev = tev_block->getTevStageNum();
    for (u32 st = 0; st < n_tev && st < 16; st++) {
        J3DTevOrder* order = tev_block->getTevOrder(st);
        if (order != NULL) {
            char line[256];
            snprintf(line, sizeof(line),
                     "2J sc-dump st[%u] order: texMap=%u texCoord=%u rasColor=%u "
                     "(2B forces GX_COLOR_NULL)",
                     st, order->getTexMap(), order->mTexCoord, order->mColorChan);
            dWwItemmdl_debugLog(line);
        }

        J3DTevStage* stage = tev_block->getTevStage(st);
        if (stage == NULL) {
            continue;
        }

        char line[384];
        snprintf(line, sizeof(line),
                 "2J sc-dump st[%u] colorIn A=%u B=%u C=%u D=%u colorOp=0x%02x "
                 "alphaIn A=%u B=%u C=%u D=%u alphaOp=0x%02x swap=0x%02x",
                 st, tevStageColorA(*stage), tevStageColorB(*stage), tevStageColorC(*stage),
                 tevStageColorD(*stage), stage->mTevColorOp, tevStageAlphaA(*stage),
                 tevStageAlphaB(*stage), tevStageAlphaC(*stage), tevStageAlphaD(*stage),
                 stage->mTevAlphaOp, stage->mTevSwapModeInfo);
        dWwItemmdl_debugLog(line);
    }
}

static void wwBowMatDrawPostDl(J3DMaterial* material) {
    if (!wwBowDrawScopeStillValid()) {
        return;
    }

    if (!isVbowDrawMaterial(material)) {
        return;
    }

    applyTexGenFromMaterial(material);
    if (isScVbowDrawMaterial(material)) {
        applyScAuthenticTevAndPeFromMaterial(material);
    } else {
        applyTevOrderFromMaterial(material);
    }

    static bool s_logged_vbow = false;
    static bool s_logged_sc = false;
    JUTNameTab* names = s_wwBowDrawModelData->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < s_wwBowDrawModelData->getMaterialNum(); i++) {
        if (s_wwBowDrawModelData->getMaterialNodePointer(i) != material) {
            continue;
        }

        const char* name = names != NULL ? names->getName(i) : NULL;
        const bool is_sc = name != NULL && strcmp(name, "SC_Vbow_v") == 0;
        if ((is_sc && s_logged_sc) || (!is_sc && s_logged_vbow)) {
            break;
        }

        char line[160];
        snprintf(line, sizeof(line), "2B apply post-dl: mat=%s nTexGen=%u nTev=%u",
                 name != NULL ? name : "?", material->getTexGenNum(), material->getTevStageNum());
        dWwItemmdl_debugLog(line);
        if (is_sc) {
            logScVbowMaterialDump(material);
            s_logged_sc = true;
        } else {
            s_logged_vbow = true;
        }
        break;
    }
}

static void logShapeInventoryInternal(J3DModelData* model_data, const char* phase) {
    if (model_data == NULL) {
        return;
    }

    char header[128];
    snprintf(header, sizeof(header), "2S shape [%s]: count=%u mats=%u joints=%u", phase,
             model_data->getShapeNum(), model_data->getMaterialNum(), model_data->getJointNum());
    dWwItemmdl_debugLog(header);
}

void dWwItemmdl_logShapeInventory(J3DModelData* model_data, const char* phase) {
    logShapeInventoryInternal(model_data, phase);
}

void dWwItemmdl_applyTevOrderForDraw(J3DModelData* model_data) {
    applyTevOrderFromMaterial(findMaterialByName(model_data, "Vbow_v"));
}

void dWwItemmdl_applyTexGenForDraw(J3DModelData* model_data) {
    applyTexGenFromMaterial(findMaterialByName(model_data, "Vbow_v"));
}

void dWwItemmdl_prepareWwBowGxForDraw(J3DModelData* model_data) {
    J3DMaterial* body_material = findMaterialByName(model_data, "Vbow_v");
    applyTexGenFromMaterial(body_material);
    applyTevOrderFromMaterial(body_material);
}

void dWwItemmdl_setWwBowActorAmbient(dKy_tevstr_c* tevstr_p) {
    if (tevstr_p == NULL) {
        return;
    }

    tevstr_p->AmbCol.r = kWwBowFixedAmbR;
    tevstr_p->AmbCol.g = kWwBowFixedAmbG;
    tevstr_p->AmbCol.b = kWwBowFixedAmbB;
}

void dWwItemmdl_applyBowMaterialAmbientOnly(J3DModel* model, dKy_tevstr_c* tevstr_p) {
    if (model == NULL || tevstr_p == NULL) {
        return;
    }

    J3DModelData* model_data = model->getModelData();
    if (model_data == NULL) {
        return;
    }

    GXColor amb_col;
    amb_col.r = tevstr_p->AmbCol.r;
    amb_col.g = tevstr_p->AmbCol.g;
    amb_col.b = tevstr_p->AmbCol.b;
    amb_col.a = tevstr_p->AmbCol.a;

    JUTNameTab* names = model_data->getMaterialTable().getMaterialName();
    for (u16 i = 0; i < model_data->getMaterialNum(); i++) {
        const char* name = names != NULL ? names->getName(i) : NULL;
        if (!isTevDumpMaterial(name)) {
            continue;
        }

        GXColor mat_amb = amb_col;
        const bool is_sc = name != NULL && strcmp(name, "SC_Vbow_v") == 0;
        if (is_sc) {
            mat_amb.r = kWwBowInkAmbR;
            mat_amb.g = kWwBowInkAmbG;
            mat_amb.b = kWwBowInkAmbB;
        } else {
            if (mat_amb.r > kWwBowBodyAmbCap) {
                mat_amb.r = kWwBowBodyAmbCap;
            }
            if (mat_amb.g > kWwBowBodyAmbCap) {
                mat_amb.g = kWwBowBodyAmbCap;
            }
            if (mat_amb.b > kWwBowBodyAmbCap) {
                mat_amb.b = kWwBowBodyAmbCap;
            }
        }

        J3DMaterial* material = model_data->getMaterialNodePointer(i);
        if (material != NULL) {
            material->setAmbColor(0, (J3DGXColor*)&mat_amb);
        }
    }

    static bool s_logged_4d = false;
    if (!s_logged_4d) {
        char line[192];
        snprintf(line, sizeof(line),
                 "4E ambient-only: body=%u,%u,%u cap=%u ink=%u,%u,%u scSuppress=%d beams=%d",
                 amb_col.r, amb_col.g, amb_col.b, kWwBowBodyAmbCap, kWwBowInkAmbR, kWwBowInkAmbG,
                 kWwBowInkAmbB, wwBowSuppressScInkPassForDraw() ? 1 : 0,
                 s_wwBowGetItemBeamSuppress ? 1 : 0);
        dWwItemmdl_debugLog(line);
        s_logged_4d = true;
    }
}

void dWwItemmdl_beginBowDrawScope(J3DModel* model, fpc_ProcID owner_id) {
    if (model == NULL || model->getModelData() == NULL) {
        return;
    }

    J3DModelData* model_data = model->getModelData();
    if (s_wwBowDrawScopeActive && s_wwBowDrawModelData == model_data &&
        s_wwBowDrawOwnerId == owner_id) {
        return;
    }

    if (s_wwBowDrawScopeActive) {
        clearBowDrawScopeInternal("begin replaced");
    }

    s_wwBowDrawScopeActive = true;
    s_wwBowDrawModelData = model_data;
    s_wwBowDrawOwnerId = owner_id;
    s_wwBowDrawRoomNo = dComIfGp_roomControl_getStayNo();
    j3dSys.setMatDrawPostDlCallback(wwBowMatDrawPostDl);
}

void dWwItemmdl_clearBowDrawScope() {
    clearBowDrawScopeInternal("Delete");
}

void dWwItemmdl_notifyRoomChange(s32 room_no) {
    if (!s_wwBowDrawScopeActive) {
        return;
    }

    if (room_no < 0 || room_no != s_wwBowDrawRoomNo) {
        clearBowDrawScopeInternal("room change");
    }
}

void dWwItemmdl_setWwBowGetItemBeamSuppress(bool suppress) {
    s_wwBowGetItemBeamSuppress = suppress;
}

bool dWwItemmdl_shouldSuppressGetItemBeamParticle(u16 particle_id) {
    if (!s_wwBowGetItemBeamSuppress) {
        return false;
    }

    return particle_id == ID_IT_JN_GETITEM_FLASH_L00 || particle_id == ID_IT_JN_GETITEM_FLASH_S00 ||
           particle_id == ID_IT_JN_GETITEM_HALO00 || particle_id == ID_IT_JN_GETITEM_STAR00;
}

void dWwItemmdl_drawWwBowModel(J3DModel* model) {
    if (model == NULL) {
        return;
    }

    J3DModelData* model_data = model->getModelData();
    if (wwBowSuppressScInkPassForDraw()) {
        dWwItemmdl_suppressOutlineForDraw(model_data);
    }

    // 2N' bake-once + entry only here; MatPacket::draw runs later in painter (see draw scope).
    mDoExt_modelUpdateDL(model);

    if (wwBowSuppressScInkPassForDraw()) {
        dWwItemmdl_restoreOutlineAfterDraw(model_data);
    }
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
        dWwItemmdl_debugLog("2K: suppress SC_* edge pass for draw");
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

void dWwItemmdl_clearOutlineSuppress() {
    dWwItemmdl_restoreOutlineAfterDraw(NULL);
}

bool dWwItemmdl_retainItemmdlArcOnDemoItemDelete() {
    return dusk::getSettings().game.wwItemmdlGetItem.getValue();
}

namespace {

struct ClothesBundleCfg {
    bool loaded = false;
    bool ok = false;
    u8 hostItem = 0xFF;
    char arc[32]{};
    char model[48]{};
    J3DModelData* cached = nullptr;

    // Get-box kit (icon + text + present-demo anchor) — all from the same ini.
    std::string getText;
    std::filesystem::path iconPath;
    std::vector<u8> iconTimg;
    bool iconTried = false;
    bool hasHandOffset = false;
    f32 handOffsetX = 0.0f;
    f32 handOffsetY = 0.0f;
    f32 handOffsetZ = 0.0f;
    f32 maxScale = 0.0f;
    std::filesystem::path modRoot;
};

ClothesBundleCfg s_clothes{};

void trimCfg(std::string* s) {
    while (!s->empty() && (s->front() == ' ' || s->front() == '\t')) {
        s->erase(s->begin());
    }
    while (!s->empty() &&
           (s->back() == ' ' || s->back() == '\t' || s->back() == '\r' || s->back() == '\n')) {
        s->pop_back();
    }
}

std::string expandCfgEscapes(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '\\' && i + 1 < in.size()) {
            const char n = in[i + 1];
            if (n == 'n') {
                out.push_back('\n');
                ++i;
                continue;
            }
            if (n == 'r') {
                out.push_back('\r');
                ++i;
                continue;
            }
            if (n == '\\') {
                out.push_back('\\');
                ++i;
                continue;
            }
        }
        out.push_back(in[i]);
    }
    return out;
}

void applyClothesBundleKey(const std::string& key, const std::string& val, u8* host,
                           std::string* arc, std::string* model) {
    if (key == "host_item") {
        *host = static_cast<u8>(std::strtoul(val.c_str(), nullptr, 0));
    } else if (key == "arc") {
        *arc = val;
    } else if (key == "model") {
        *model = val;
    } else if (key == "get_text") {
        s_clothes.getText = expandCfgEscapes(val);
    } else if (key == "get_icon") {
        if (!val.empty() && !s_clothes.modRoot.empty()) {
            s_clothes.iconPath = s_clothes.modRoot / val;
        }
    } else if (key == "hand_offset_x") {
        s_clothes.handOffsetX = static_cast<f32>(std::strtof(val.c_str(), nullptr));
        s_clothes.hasHandOffset = true;
    } else if (key == "hand_offset_y") {
        s_clothes.handOffsetY = static_cast<f32>(std::strtof(val.c_str(), nullptr));
        s_clothes.hasHandOffset = true;
    } else if (key == "hand_offset_z") {
        s_clothes.handOffsetZ = static_cast<f32>(std::strtof(val.c_str(), nullptr));
        s_clothes.hasHandOffset = true;
    } else if (key == "max_scale") {
        s_clothes.maxScale = static_cast<f32>(std::strtof(val.c_str(), nullptr));
    }
}

bool looksLikeIniKeyLine(const std::string& trimmed) {
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
        return false;
    }
    const size_t eq = trimmed.find('=');
    if (eq == std::string::npos || eq == 0) {
        return false;
    }
    for (size_t i = 0; i < eq; ++i) {
        const char c = trimmed[i];
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

bool parseClothesBundleIniKeys(const std::filesystem::path& file, u8* host, std::string* arc,
                               std::string* model) {
    std::ifstream in(file);
    if (!in) {
        return false;
    }
    std::string line;
    std::string pendingKey;
    std::string pendingVal;
    auto flushPending = [&]() {
        if (pendingKey.empty()) {
            return;
        }
        // Trailing whitespace only — preserve internal newlines.
        while (!pendingVal.empty() &&
               (pendingVal.back() == ' ' || pendingVal.back() == '\t' || pendingVal.back() == '\r')) {
            pendingVal.pop_back();
        }
        applyClothesBundleKey(pendingKey, pendingVal, host, arc, model);
        pendingKey.clear();
        pendingVal.clear();
    };
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::string trimmed = line;
        trimCfg(&trimmed);

        if (!pendingKey.empty() && !looksLikeIniKeyLine(trimmed)) {
            if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
                // blank / comment ends a multi-line get_text block
                flushPending();
                continue;
            }
            if (!pendingVal.empty()) {
                pendingVal.push_back('\n');
            }
            pendingVal.append(trimmed);
            continue;
        }

        flushPending();
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }
        const size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trimmed.substr(0, eq);
        std::string val = trimmed.substr(eq + 1);
        trimCfg(&key);
        trimCfg(&val);
        if (key == "get_text") {
            pendingKey = key;
            pendingVal = val;
            continue;
        }
        applyClothesBundleKey(key, val, host, arc, model);
    }
    flushPending();
    return true;
}

bool tryLoadClothesBundleIni(const std::filesystem::path& file) {
    s_clothes.modRoot = file.parent_path().parent_path();  // .../<mod>/getitem/file → <mod>
    s_clothes.getText.clear();
    s_clothes.iconPath.clear();
    s_clothes.iconTimg.clear();
    s_clothes.iconTried = false;
    s_clothes.hasHandOffset = false;
    s_clothes.handOffsetX = s_clothes.handOffsetY = s_clothes.handOffsetZ = 0.0f;
    s_clothes.maxScale = 0.0f;

    u8 host = 0xFF;
    std::string arc;
    std::string model;
    // Primary kit file, then optional staged sibling overlays get_text/get_icon/offsets.
    if (!parseClothesBundleIniKeys(file, &host, &arc, &model)) {
        return false;
    }
    parseClothesBundleIniKeys(file.parent_path() / "clothes_bundle_text.ini", &host, &arc, &model);

    if (host == 0xFF || arc.empty() || model.empty() || arc.size() >= sizeof(s_clothes.arc) ||
        model.size() >= sizeof(s_clothes.model)) {
        return false;
    }
    s_clothes.hostItem = host;
    std::snprintf(s_clothes.arc, sizeof(s_clothes.arc), "%s", arc.c_str());
    std::snprintf(s_clothes.model, sizeof(s_clothes.model), "%s", model.c_str());
    s_clothes.ok = true;
    DuskLog.info(
        "[WwItemmdl] clothes bundle cfg host=0x{:02X} arc='{}' model='{}' text={} icon='{}' "
        "hand={} scale={}",
        static_cast<unsigned>(host), s_clothes.arc, s_clothes.model,
        s_clothes.getText.empty() ? "no" : "yes",
        s_clothes.iconPath.empty() ? "" : s_clothes.iconPath.filename().string(),
        s_clothes.hasHandOffset ? "yes" : "no", s_clothes.maxScale);
    return true;
}

void ensureClothesBundleCfg() {
    if (s_clothes.loaded) {
        return;
    }
    s_clothes.loaded = true;
    namespace fs = std::filesystem;
    const fs::path root = fs::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        return;
    }
    for (const auto& mod : fs::directory_iterator(root, ec)) {
        if (!mod.is_directory()) {
            continue;
        }
        const fs::path ini = mod.path() / "getitem" / "clothes_bundle.ini";
        if (tryLoadClothesBundleIni(ini)) {
            return;
        }
    }
}

bool ensureClothesBundleIconLoaded() {
    ensureClothesBundleCfg();
    if (!s_clothes.ok || s_clothes.iconPath.empty()) {
        return false;
    }
    if (s_clothes.iconTried) {
        return !s_clothes.iconTimg.empty();
    }
    s_clothes.iconTried = true;
    std::ifstream in(s_clothes.iconPath, std::ios::binary);
    if (!in) {
        DuskLog.warn("[WwItemmdl] clothes get_icon miss '{}'", s_clothes.iconPath.string());
        return false;
    }
    in.seekg(0, std::ios::end);
    const std::streamoff sz = in.tellg();
    in.seekg(0, std::ios::beg);
    if (sz <= 0x20 || sz > 0x10000) {
        DuskLog.warn("[WwItemmdl] clothes get_icon bad size {}", static_cast<long long>(sz));
        return false;
    }
    s_clothes.iconTimg.resize(static_cast<size_t>(sz));
    if (!in.read(reinterpret_cast<char*>(s_clothes.iconTimg.data()), sz)) {
        s_clothes.iconTimg.clear();
        return false;
    }
    DuskLog.info("[WwItemmdl] clothes get_icon ready {} bytes", static_cast<int>(sz));
    return true;
}

}  // namespace

bool dWwItemmdl_clothesBundleForItem(u8 item_no) {
    ensureClothesBundleCfg();
    return s_clothes.ok && item_no == s_clothes.hostItem;
}

const char* dWwItemmdl_clothesBundleArcName() {
    ensureClothesBundleCfg();
    return s_clothes.ok ? s_clothes.arc : nullptr;
}

const char* dWwItemmdl_clothesBundleModelName() {
    ensureClothesBundleCfg();
    return s_clothes.ok ? s_clothes.model : nullptr;
}

J3DModelData* dWwItemmdl_getClothesBundleModelData() {
    ensureClothesBundleCfg();
    if (!s_clothes.ok) {
        return nullptr;
    }
    if (s_clothes.cached != nullptr) {
        return s_clothes.cached;
    }
    // Mature path (Ivan / Outset cast): BDLM arcs leave raw J3D2 on RelWithDebInfo
    // (dRes has no BDLM loader). Never cast getObjectRes to ModelData — re-parse +
    // finish via the mount acquire (GameHeap pin, pristine copy, actor light mask).
    J3DModelData* md = dExtNpcMount_acquireModelData(s_clothes.arc, s_clothes.model);
    if (md == nullptr) {
        DuskLog.warn("[WwItemmdl] clothes bundle model miss arc='{}' model='{}'", s_clothes.arc,
                     s_clothes.model);
        return nullptr;
    }
    s_clothes.cached = md;
    // M6 gate: neutral token only. The provider name is a greplist term and a
    // shipped string literal is a covenant leak regardless of what it explains.
    dWwItemmdl_debugLog("clothes bundle: acquired via ExtNpcMount (provider create path)");
    DuskLog.info("[WwItemmdl] clothes bundle model ready arc='{}' model='{}' mats={} joints={}",
                 s_clothes.arc, s_clothes.model, md->getMaterialNum(), md->getJointNum());
    return md;
}

void dWwItemmdl_clearClothesBundleCache() {
    s_clothes.cached = nullptr;
}

static bool s_clothesGetPresentation = false;

const char* dWwItemmdl_clothesBundleGetTextForMessage(u32 msg_id) {
    ensureClothesBundleCfg();
    if (!s_clothes.ok || s_clothes.getText.empty()) {
        return nullptr;
    }
    // §186: demo presentation arm — BMG message_id at table index host+0x65 may
    // not equal host+0x65 (that mis-wire showed "magic armor / Malo Mart").
    if (s_clothesGetPresentation) {
        return s_clothes.getText.c_str();
    }
    if (msg_id != static_cast<u32>(s_clothes.hostItem) + 0x65u) {
        return nullptr;
    }
    return s_clothes.getText.c_str();
}

u8 dWwItemmdl_clothesBundleHostItem() {
    ensureClothesBundleCfg();
    return s_clothes.ok ? s_clothes.hostItem : static_cast<u8>(0xFF);
}

void dWwItemmdl_beginClothesGetPresentation() {
    ensureClothesBundleCfg();
    s_clothesGetPresentation = true;
}

void dWwItemmdl_endClothesGetPresentation() {
    s_clothesGetPresentation = false;
}

bool dWwItemmdl_isClothesGetPresentation() {
    return s_clothesGetPresentation;
}

u32 dWwItemmdl_clothesBundleIconCap(u8 item_no) {
    if (!dWwItemmdl_clothesBundleForItem(item_no) || !ensureClothesBundleIconLoaded()) {
        return 0;
    }
    return static_cast<u32>(s_clothes.iconTimg.size());
}

bool dWwItemmdl_writeClothesBundleIconTimg(u8 item_no, void* out_buf, u32 out_cap) {
    if (out_buf == nullptr || !dWwItemmdl_clothesBundleForItem(item_no) ||
        !ensureClothesBundleIconLoaded()) {
        return false;
    }
    if (s_clothes.iconTimg.size() > out_cap) {
        return false;
    }
    std::memcpy(out_buf, s_clothes.iconTimg.data(), s_clothes.iconTimg.size());
    return true;
}

bool dWwItemmdl_clothesBundleHandOffset(f32* out_x, f32* out_y, f32* out_z) {
    ensureClothesBundleCfg();
    if (!s_clothes.ok || !s_clothes.hasHandOffset || out_x == nullptr || out_y == nullptr ||
        out_z == nullptr) {
        return false;
    }
    *out_x = s_clothes.handOffsetX;
    *out_y = s_clothes.handOffsetY;
    *out_z = s_clothes.handOffsetZ;
    return true;
}

f32 dWwItemmdl_clothesBundleMaxScale() {
    ensureClothesBundleCfg();
    return s_clothes.ok ? s_clothes.maxScale : 0.0f;
}

#endif

// ============================================================================
// Step 19 cluster 2 — hand this layer's entry points to the receiver's
// dispatcher. Defined HERE, WW-side, on purpose: excluding the WW layer removes
// this function, the table stays all-NULL, and the receiver runs standalone.
// The absence is structural rather than a flag someone must remember to clear.
// ============================================================================
#include "d/ext_plugin/ww_itemmdl_dispatch.h"

void dWwItemmdlDispatch_install() {
    g_wwItemmdlApi.applyBowMaterialAmbientOnly = &dWwItemmdlImpl_applyBowMaterialAmbientOnly;
    g_wwItemmdlApi.beginBowDrawScope = &dWwItemmdlImpl_beginBowDrawScope;
    g_wwItemmdlApi.bracketLog = &dWwItemmdlImpl_bracketLog;
    g_wwItemmdlApi.clearBowDrawScope = &dWwItemmdlImpl_clearBowDrawScope;
    g_wwItemmdlApi.clearClothesBundleCache = &dWwItemmdlImpl_clearClothesBundleCache;
    g_wwItemmdlApi.clearOutlineSuppress = &dWwItemmdlImpl_clearOutlineSuppress;
    g_wwItemmdlApi.clothesBundleArcName = &dWwItemmdlImpl_clothesBundleArcName;
    g_wwItemmdlApi.clothesBundleForItem = &dWwItemmdlImpl_clothesBundleForItem;
    g_wwItemmdlApi.clothesBundleGetTextForMessage = &dWwItemmdlImpl_clothesBundleGetTextForMessage;
    g_wwItemmdlApi.clothesBundleHandOffset = &dWwItemmdlImpl_clothesBundleHandOffset;
    g_wwItemmdlApi.clothesBundleHostItem = &dWwItemmdlImpl_clothesBundleHostItem;
    g_wwItemmdlApi.clothesBundleIconCap = &dWwItemmdlImpl_clothesBundleIconCap;
    g_wwItemmdlApi.clothesBundleMaxScale = &dWwItemmdlImpl_clothesBundleMaxScale;
    g_wwItemmdlApi.clothesBundleModelName = &dWwItemmdlImpl_clothesBundleModelName;
    g_wwItemmdlApi.debugLog = &dWwItemmdlImpl_debugLog;
    g_wwItemmdlApi.drawWwBowModel = &dWwItemmdlImpl_drawWwBowModel;
    g_wwItemmdlApi.getClothesBundleModelData = &dWwItemmdlImpl_getClothesBundleModelData;
    g_wwItemmdlApi.getItemmdlModelData = &dWwItemmdlImpl_getItemmdlModelData;
    g_wwItemmdlApi.getVbowModelData = &dWwItemmdlImpl_getVbowModelData;
    g_wwItemmdlApi.isClothesGetPresentation = &dWwItemmdlImpl_isClothesGetPresentation;
    g_wwItemmdlApi.isPhase2BracketBow = &dWwItemmdlImpl_isPhase2BracketBow;
    g_wwItemmdlApi.log2QPrimeAudit = &dWwItemmdlImpl_log2QPrimeAudit;
    g_wwItemmdlApi.logHeap = &dWwItemmdlImpl_logHeap;
    g_wwItemmdlApi.logShapeInventory = &dWwItemmdlImpl_logShapeInventory;
    g_wwItemmdlApi.logTevOrderDump = &dWwItemmdlImpl_logTevOrderDump;
    g_wwItemmdlApi.patchModelForPc = &dWwItemmdlImpl_patchModelForPc;
    g_wwItemmdlApi.retainItemmdlArcOnDemoItemDelete = &dWwItemmdlImpl_retainItemmdlArcOnDemoItemDelete;
    g_wwItemmdlApi.setWwBowActorAmbient = &dWwItemmdlImpl_setWwBowActorAmbient;
    g_wwItemmdlApi.setWwBowGetItemBeamSuppress = &dWwItemmdlImpl_setWwBowGetItemBeamSuppress;
    g_wwItemmdlApi.shouldSuppressGetItemBeamParticle = &dWwItemmdlImpl_shouldSuppressGetItemBeamParticle;
    g_wwItemmdlApi.stepPrivateItemmdlArcLoad = &dWwItemmdlImpl_stepPrivateItemmdlArcLoad;
    g_wwItemmdlApi.tickHeldBowArcMount = &dWwItemmdlImpl_tickHeldBowArcMount;
    g_wwItemmdlApi.use2DIsolateHeap = &dWwItemmdlImpl_use2DIsolateHeap;
    g_wwItemmdlApi.usingCustomHeldModel = &dWwItemmdlImpl_usingCustomHeldModel;
    g_wwItemmdlApi.writeClothesBundleIconTimg = &dWwItemmdlImpl_writeClothesBundleIconTimg;
}

// SELF-INSTALLING, and deliberately not called by the receiver.
//
// Cluster 1 needed a boot-time call from m_Do_main, which added a new
// unresolved symbol when the WW layer is excluded. This does not: the WW layer
// binds itself during static initialisation, so the receiver contains no
// reference to it at all. That is closer to how a plugin actually behaves --
// it announces itself on load rather than waiting to be switched on -- and it
// means excluding the layer leaves NO dangling call site.
//
// Safe with respect to init order: g_wwItemmdlApi has static storage and is
// zero-initialised before any dynamic initialiser runs, so this can only ever
// overwrite NULLs.
namespace {
struct WwItemmdlAutoInstall {
    WwItemmdlAutoInstall() { dWwItemmdlDispatch_install(); }
};
const WwItemmdlAutoInstall s_wwItemmdlAutoInstall;
}  // namespace
