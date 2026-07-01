/**
 * d_ww_itemmdl_pc.cpp — Aurora-safe material patch for WW itemmdl view models.
 */

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
#include "d/d_item_data.h"
#include "d/d_resorce.h"
#include "aurora/lib/gx/gx.hpp"
#include "dusk/settings.h"
#include "m_Do/m_Do_ext.h"
#include "res/Object/itemmdl.h"

#include <dolphin/gx/GXTev.h>
#include <dolphin/gx/GXGeometry.h>

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
static J3DModelData* s_wwBowDrawModelData = NULL;
static bool s_wwBowDrawScopeActive = false;

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

static bool isTevDumpMaterial(const char* name) {
    if (name == NULL) {
        return false;
    }

    return strcmp(name, "Vbow_v") == 0 || strcmp(name, "SC_Vbow_v") == 0;
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

static J3DModelData* loadVbowFromPrivateArcOnce() {
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

static void wwBowMatDrawPostDl(J3DMaterial* material) {
    if (!isVbowDrawMaterial(material)) {
        return;
    }

    applyTexGenFromMaterial(material);
    applyTevOrderFromMaterial(material);

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

void dWwItemmdl_beginBowDrawScope(J3DModel* model) {
    if (model == NULL || model->getModelData() == NULL) {
        return;
    }

    J3DModelData* model_data = model->getModelData();
    if (s_wwBowDrawScopeActive && s_wwBowDrawModelData == model_data) {
        return;
    }

    s_wwBowDrawScopeActive = true;
    s_wwBowDrawModelData = model_data;
    j3dSys.setMatDrawPostDlCallback(wwBowMatDrawPostDl);
}

void dWwItemmdl_clearBowDrawScope() {
    if (!s_wwBowDrawScopeActive) {
        return;
    }

    s_wwBowDrawScopeActive = false;
    j3dSys.setMatDrawPostDlCallback(nullptr);
    s_wwBowDrawModelData = NULL;
}

void dWwItemmdl_drawWwBowModel(J3DModel* model) {
    if (model == NULL) {
        return;
    }

    // 2N' bake-once + entry only here; MatPacket::draw runs later in painter (see draw scope).
    mDoExt_modelUpdateDL(model);
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

#endif
