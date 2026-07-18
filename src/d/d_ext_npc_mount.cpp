/**
 * d_ext_npc_mount.cpp — Plan R generic external-NPC mount (L1 + Slice I lighting/blink).
 */
#include "d/d_ext_npc_mount.h"

#if TARGET_PC

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "SSystem/SComponent/c_math.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "d/d_drawlist.h"
#include "d/d_kankyo.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_mtx.h"

namespace {

namespace fs = std::filesystem;

std::unordered_map<std::string, dExtNpcManifest> s_providers;

// Arc name currently resLoading with skip_btp (empty = none).
char s_skipBtpArc[16];

// Match itemmdl BDL path: DoBdlMaterialCalc → readPatchedMaterial.
static constexpr u32 kExtNpcBdlFlags = 0x59020010u | 0x2000u;

void trimInPlace(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
        ++i;
    }
    if (i > 0) {
        s.erase(0, i);
    }
}

bool parseBoolVal(const std::string& val) {
    return val == "1" || val == "true" || val == "yes" || val == "on";
}

bool parseManifestFile(const fs::path& path, const char* modFolder, dExtNpcManifest* out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    out->cylRadius = 40.0f;
    out->cylHeight = 100.0f;
    out->scale = 1.0f;
    snprintf(out->modFolder, sizeof(out->modFolder), "%s", modFolder);

    std::string line;
    while (std::getline(in, line)) {
        trimInPlace(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trimInPlace(key);
        trimInPlace(val);
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
        }
        auto set = [&](char* dst, size_t n) { snprintf(dst, n, "%s", val.c_str()); };
        if (key == "proc") {
            set(out->proc, sizeof(out->proc));
        } else if (key == "arc") {
            set(out->arc, sizeof(out->arc));
        } else if (key == "model") {
            set(out->model, sizeof(out->model));
        } else if (key == "idle") {
            set(out->idle, sizeof(out->idle));
        } else if (key == "talk1") {
            set(out->talk1, sizeof(out->talk1));
        } else if (key == "talk2") {
            set(out->talk2, sizeof(out->talk2));
        } else if (key == "btp") {
            set(out->btp, sizeof(out->btp));
        } else if (key == "display_name") {
            set(out->displayName, sizeof(out->displayName));
        } else if (key == "neck_joint") {
            set(out->neckJoint, sizeof(out->neckJoint));
        } else if (key == "cyl_radius") {
            out->cylRadius = (f32)atof(val.c_str());
        } else if (key == "cyl_height") {
            out->cylHeight = (f32)atof(val.c_str());
        } else if (key == "scale") {
            out->scale = (f32)atof(val.c_str());
            if (out->scale <= 0.01f) {
                out->scale = 1.0f;
            }
        } else if (key == "skip_btp") {
            out->skipBtp = parseBoolVal(val);
        } else if (key == "source") {
            for (char& c : val) {
                if (c >= 'A' && c <= 'Z') {
                    c = (char)(c - 'A' + 'a');
                }
            }
            out->fromDvd = (val == "dvd" || val == "game" || val == "tp");
        }
    }
    out->valid = out->proc[0] && out->arc[0] && out->model[0] && out->idle[0];
    return out->valid;
}

bool arcFilePresent(const fs::path& modRoot, const char* arcName) {
    std::error_code ec;
    return fs::is_regular_file(modRoot / "arcs" / (std::string(arcName) + ".arc"), ec);
}

void stageLog(const char* stage, const char* detail) {
    DuskLog.info("[ExtNpcMount:D1] {} — {}", stage, detail != NULL ? detail : "");
}

// I1: apply the same light-mask pass BMDR gets in loaderBasicBmd so lit body
// channels actually receive kankyo lights (WW materials often arrive mask-zero).
void applyActorLightMask(J3DModelData* modelData) {
    if (modelData == NULL) {
        return;
    }
    for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL || material->getColorChan(0) == NULL) {
            continue;
        }
        u8 lightMask = material->getColorChan(0)->getLightMask();
        switch (g_env_light.light_mask_type) {
        case 1:
            lightMask &= 0x4;
            break;
        case 2:
            lightMask &= 0xC;
            break;
        case 3:
            lightMask &= 0xD;
            break;
        case 4:
            lightMask &= 0xF;
            break;
        case 5:
            lightMask &= 0x1F;
            break;
        case 6:
            lightMask &= 0x3F;
            break;
        case 7:
            lightMask &= 0x7F;
            break;
        default:
            break;
        }
        if (lightMask == 0) {
            lightMask = 0xFF;
        }
        material->getColorChan(0)->setLightMask(lightMask);
        material->change();
    }
}

J3DModelData* finishMountedModelData(J3DModelData* modelData) {
    stageLog("finish", "enter");
    if (modelData == NULL) {
        stageLog("finish", "FAIL modelData=NULL");
        return NULL;
    }
    const u16 matNum = modelData->getMaterialNum();
    char buf[96];
    snprintf(buf, sizeof(buf), "matNum=%u jointNum=%u", matNum, modelData->getJointNum());
    stageLog("finish", buf);
    if (matNum == 0) {
        stageLog("finish", "FAIL matNum=0");
        return NULL;
    }
    J3DMaterial* mat0 = modelData->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("finish", "FAIL material[0]=NULL");
        return NULL;
    }

    applyActorLightMask(modelData);

    for (u16 i = 0; i < matNum; i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL) {
            snprintf(buf, sizeof(buf), "FAIL material[%u]=NULL", i);
            stageLog("finish", buf);
            return NULL;
        }
        material->change();
        J3DMaterialAnm* materialAnm = JKR_NEW J3DMaterialAnm();
        if (materialAnm == NULL) {
            stageLog("finish", "FAIL J3DMaterialAnm alloc");
            return NULL;
        }
        material->setMaterialAnm(materialAnm);
    }
    if (modelData->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        stageLog("finish", "FAIL newSharedDisplayList");
        return NULL;
    }
    modelData->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    modelData->makeSharedDL();
    stageLog("finish", "ok");
    return modelData;
}

J3DModelData* resolveMountedModel(void* res) {
    if (res == NULL) {
        stageLog("resolve", "FAIL res=NULL");
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    char magicBuf[64];
    snprintf(magicBuf, sizeof(magicBuf), "magic1=%08x magic2=%08x", (u32)header->mMagic1,
             (u32)header->mMagic2);
    stageLog("resolve", magicBuf);

    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3')) {
        stageLog("resolve", "path=loadBinaryDisplayList");
        J3DModelData* loaded =
            J3DModelLoaderDataBase::loadBinaryDisplayList(res, kExtNpcBdlFlags);
        if (loaded == NULL) {
            stageLog("resolve", "FAIL loadBinaryDisplayList NULL");
            return NULL;
        }
        return finishMountedModelData(loaded);
    }
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2')) {
        stageLog("resolve", "path=load (bmd)");
        J3DModelData* loaded = (J3DModelData*)J3DModelLoaderDataBase::load(res, 0x59020010);
        if (loaded == NULL) {
            stageLog("resolve", "FAIL load(bmd) NULL");
            return NULL;
        }
        return finishMountedModelData(loaded);
    }
    stageLog("resolve", "path=preinstantiated J3DModelData*");
    J3DModelData* data = (J3DModelData*)res;
    if (data->getMaterialNum() == 0 || data->getMaterialNodePointer(0) == NULL) {
        stageLog("resolve", "FAIL preinstantiated model has no material[0]");
        return NULL;
    }
    applyActorLightMask(data);
    return data;
}

bool tryBindBtp(dExtNpcMount_c* a, J3DModelData* data) {
    a->mpBtp = NULL;
    a->mBtpBound = false;
    if (a->mManifest.skipBtp || a->mManifest.btp[0] == '\0') {
        stageLog("btp", "skipped (skip_btp or no btp=)");
        return true;
    }
    J3DAnmTexPattern* pat =
        (J3DAnmTexPattern*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.btp);
    if (pat == NULL) {
        DuskLog.warn("[ExtNpcMount] btp '{}' missing in arc '{}' — blink deferred",
                     a->mManifest.btp, a->mManifest.arc);
        stageLog("btp", "FAIL res NULL (cosmetic defer)");
        return true;  // non-fatal
    }
    a->mpBtp = JKR_NEW mDoExt_btpAnm();
    if (a->mpBtp == NULL) {
        stageLog("btp", "FAIL alloc");
        return true;
    }
    // Loop blink: attribute EMode_LOOP (2) like npc_ks relief path.
    if (a->mpBtp->init(data, pat, 1, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1) == 0) {
        DuskLog.warn("[ExtNpcMount] btp '{}' bind failed — blink deferred", a->mManifest.btp);
        stageLog("btp", "FAIL init (cosmetic defer)");
        a->mpBtp = NULL;
        return true;
    }
    a->mBtpBound = true;
    stageLog("btp", "bound ok");
    return true;
}

int useHeapInit(fopAc_ac_c* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    stageLog("heap", "getObjectRes model");
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.model);
    if (raw == NULL) {
        stageLog("heap", "FAIL model res NULL");
        return 0;
    }
    J3DModelData* data = resolveMountedModel(raw);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] model '{}' missing/unparseable in arc '{}'", a->mManifest.model,
                     a->mManifest.arc);
        return 0;
    }

    stageLog("heap", "getObjectRes idle BCK");
    J3DAnmTransform* anm =
        (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.idle);
    if (anm == NULL) {
        stageLog("heap", "FAIL idle anm NULL");
        return 0;
    }

    J3DMaterial* mat0 = data->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("heap", "FAIL pre-McaMorf material[0]=NULL");
        return 0;
    }

    stageLog("heap", "McaMorf ctor");
    a->mpMorf = JKR_NEW mDoExt_McaMorf(data, NULL, NULL, anm, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1,
                                       1, NULL, 0x80000, 0x11000084);
    if (a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        stageLog("heap", "FAIL McaMorf");
        a->mpMorf = NULL;
        return 0;
    }

    tryBindBtp(a, data);
    stageLog("heap", "ok — model bound");
    return 1;
}

}  // namespace

void dExtNpcMount_rescanProviders() {
    s_providers.clear();
    s_skipBtpArc[0] = '\0';
    std::error_code ec;
    const fs::path userRoot = dusk::ConfigPath / "model_replacements";
    if (!fs::is_directory(userRoot, ec)) {
        return;
    }
    for (auto it = fs::directory_iterator(userRoot, ec); it != fs::directory_iterator();
         it.increment(ec)) {
        if (ec || !it->is_directory(ec)) {
            continue;
        }
        const fs::path modRoot = it->path();
        const std::string modName = modRoot.filename().string();
        if (!dusk::custom_assets::is_folder_enabled(modName.c_str())) {
            continue;
        }
        const fs::path npcDir = modRoot / "npc";
        if (!fs::is_directory(npcDir, ec)) {
            continue;
        }
        for (auto nit = fs::directory_iterator(npcDir, ec); nit != fs::directory_iterator();
             nit.increment(ec)) {
            if (ec || !nit->is_regular_file(ec)) {
                continue;
            }
            if (nit->path().extension() != ".ini") {
                continue;
            }
            dExtNpcManifest man{};
            if (!parseManifestFile(nit->path(), modName.c_str(), &man)) {
                continue;
            }
            if (!man.fromDvd && !arcFilePresent(modRoot, man.arc)) {
                DuskLog.debug("[ExtNpcMount] '{}' manifest ok but arcs/{}.arc missing — socket idle",
                              man.proc, man.arc);
                continue;
            }
            s_providers[man.proc] = man;
            DuskLog.info("[ExtNpcMount] provider {} ← mod '{}' arc={} skip_btp={} btp={} scale={}",
                         man.proc, modName, man.arc, man.skipBtp ? 1 : 0,
                         man.btp[0] ? man.btp : "-", man.scale);
        }
    }
}

bool dExtNpcMount_hasPayload(const char* procName) {
    return procName != NULL && s_providers.find(procName) != s_providers.end();
}

bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out) {
    auto it = s_providers.find(procName != NULL ? procName : "");
    if (it == s_providers.end() || out == NULL) {
        return false;
    }
    *out = it->second;
    return true;
}

bool dExtNpcMount_shouldSkipBtp(const char* arcName) {
    return arcName != NULL && s_skipBtpArc[0] != '\0' && strcmp(arcName, s_skipBtpArc) == 0;
}

int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName) {
    if (i_this == NULL || !dExtNpcMount_lookup(procName, &i_this->mManifest)) {
        return cPhs_ERROR_e;
    }

    i_this->mpBtp = NULL;
    i_this->mBtpBound = false;

    if (i_this->mManifest.skipBtp) {
        snprintf(s_skipBtpArc, sizeof(s_skipBtpArc), "%s", i_this->mManifest.arc);
        stageLog("create", "skip_btp armed");
    } else {
        s_skipBtpArc[0] = '\0';
    }

    stageLog("create", "resLoad begin");
    int phase = dComIfG_resLoad(&i_this->mPhase, i_this->mManifest.arc);
    if (phase != cPhs_COMPLEATE_e) {
        if (phase == cPhs_ERROR_e) {
            stageLog("create", "FAIL resLoad ERROR");
            s_skipBtpArc[0] = '\0';
        }
        return phase;
    }
    s_skipBtpArc[0] = '\0';
    stageLog("create", "resLoad COMPLEATE → solid heap");

    if (!fopAcM_entrySolidHeap(i_this, useHeapInit, 0x50000)) {
        DuskLog.warn("[ExtNpcMount] heap fail for {}", procName);
        stageLog("create", "FAIL entrySolidHeap");
        return cPhs_ERROR_e;
    }

    const f32 s = i_this->mManifest.scale;
    i_this->scale.set(s, s, s);

    dKy_tevstr_init(&i_this->tevStr, fopAcM_GetRoomNo(i_this), 0xFF);
    i_this->tevStr.room_no = fopAcM_GetRoomNo(i_this);

    fopAcM_SetMtx(i_this, i_this->mpMorf->getModel()->getBaseTRMtx());
    fopAcM_SetMin(i_this, -80.0f * s, -20.0f * s, -80.0f * s);
    fopAcM_SetMax(i_this, 80.0f * s, 200.0f * s, 80.0f * s);
    i_this->attention_info.flags = 0;
    i_this->mGroundSnapped = false;

    if (fopAcM_gc_c::gndCheck(&i_this->current.pos)) {
        i_this->current.pos.y = fopAcM_gc_c::getGroundY();
        i_this->old.pos = i_this->current.pos;
        i_this->home.pos = i_this->current.pos;
        i_this->mGroundSnapped = true;
    }

    DuskLog.info("[ExtNpcMount] COMPLEATE {} arc={} model={} btp={} scale={} mod={}", procName,
                 i_this->mManifest.arc, i_this->mManifest.model,
                 i_this->mBtpBound ? i_this->mManifest.btp : "(none)", s,
                 i_this->mManifest.modFolder);
    stageLog("create", "COMPLEATE");
    return cPhs_COMPLEATE_e;
}

int dExtNpcMount_delete(dExtNpcMount_c* i_this) {
    if (i_this != NULL && i_this->mManifest.arc[0]) {
        dComIfG_resDelete(&i_this->mPhase, i_this->mManifest.arc);
    }
    return 1;
}

int dExtNpcMount_execute(dExtNpcMount_c* i_this) {
    if (i_this == NULL || i_this->mpMorf == NULL) {
        return 1;
    }
    if (!i_this->mGroundSnapped && fopAcM_gc_c::gndCheck(&i_this->current.pos)) {
        i_this->current.pos.y = fopAcM_gc_c::getGroundY();
        i_this->mGroundSnapped = true;
    }
    const f32 s = i_this->scale.x;
    mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y, i_this->current.pos.z);
    mDoMtx_stack_c::YrotM(i_this->shape_angle.y);
    mDoMtx_stack_c::scaleM(s, s, s);
    i_this->mpMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
    i_this->mpMorf->play(NULL, 0, 0);
    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->play();
    }
    i_this->mpMorf->modelCalc();
    return 1;
}

int dExtNpcMount_draw(dExtNpcMount_c* i_this) {
    if (i_this == NULL || i_this->mpMorf == NULL) {
        return 1;
    }

    J3DModel* model = i_this->mpMorf->getModel();

    // I1-REVISED: match d_demo_leftover_viewer::draw (proven on arbitrary external models).
    // Viewer never builds its own tevStr for lighting — it reuses Link's already-filled
    // tevStr (Material_use_fg==124) then MAJI → setList → entryDL. Our prior path called
    // settingTevStruct on a private tevStr then MAJI; №12 stayed black despite healthy MAT3.
    dKy_tevstr_c* lightTev = &i_this->tevStr;
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player != NULL) {
        lightTev = &player->tevStr;
    } else {
        // Fallback if player missing this frame: fill private tevStr like a stock NPC.
        g_env_light.settingTevStruct(0, &i_this->current.pos, &i_this->tevStr);
    }

    // Keep private tevStr warm for shadow / future attachments (init once at create).
    if (player != NULL) {
        g_env_light.settingTevStruct(0, &i_this->current.pos, &i_this->tevStr);
    }

    // Ensure material packets accept amb/light patches (SharedDL path).
    {
        J3DModelData* md = model->getModelData();
        for (u16 i = 0; i < md->getMaterialNum(); i++) {
            J3DMaterial* mat = md->getMaterialNodePointer(i);
            if (mat != NULL) {
                mat->change();
            }
        }
    }

    g_env_light.setLightTevColorType_MAJI(model, lightTev);

    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->entry(model->getModelData());
    }

    dComIfGd_setList();
    i_this->mpMorf->entryDL();

    // I3: simple ground shadow (no Acch — use up-normal).
    cXyz gndNrm(0.0f, 1.0f, 0.0f);
    dComIfGd_setSimpleShadow(&i_this->current.pos, i_this->current.pos.y, 50.0f * i_this->scale.x,
                             &gndNrm, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex());
    return 1;
}

#endif  // TARGET_PC
