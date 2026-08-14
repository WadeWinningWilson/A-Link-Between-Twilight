/**
 * d_ext_tree.cpp
 * The donor SWOOD packet — WW d_tree.cpp ported (Foundry §242 order, §366).
 *
 * ============================================================================
 * §366 — SWOOD (small cuttable wood), body donor-verbatim from WW d_tree.cpp,
 * spec locked by Housing §231/§243/§249 (d_tree NOT d_wood; 5 DLs +
 * l_modelStatus {0,1,2 / 3,1,4}; shadow pass; 13 blobs decomp-exact at
 * assets/veg/d_tree__*.bin). Port deltas, each bannered inline:
 *   #1 PHASE : donor update()'s view-matrix math runs INSIDE packet draw()
 *              (draw phase) — Housing's stale-view law (§45 lesson: an actor
 *              callback's j3dSys view is not current; both references enter
 *              via drawBuffer entryImm and concat in draw phase).
 *   #2 BLOBS : compiled-in donor headers -> runtime blobs (readBlob pattern);
 *              DL call sizes are the DONOR CALL sizes (32-byte multiples),
 *              not raw symbol sizes (Housing's discipline).
 *   #3 TEX   : matDL's baked GameCube texture pointer is meaningless here —
 *              GXInitTexObj once + GXLoadTexObj after the matDL (№135).
 *              swood 64x64 @2048B = CMPR (4bpp + 1-bit alpha, foliage
 *              silhouette); kage 32x32 @512B = I4 first (intensity shadow;
 *              alternative at 4bpp is CMPR if it reads as noise).
 *   #4 TEVSTR: donor reads dComIfGp_roomControl_getTevStr(room) — №141: a
 *              mounted-host room's table entry is unfilled (BLACK). Build our
 *              own tevstr via settingTevStruct at the room's first tree.
 *   #5 GX    : donor GF* wrappers -> GX* (aurora): GXSetArray(le=false) for
 *              big-endian arrays (d_map_path.cpp:489 precedent), GXLoadPosMtxImm,
 *              GXSetTevColor/S10. dKy_GfFog_tevstr_set exists natively.
 *   #6 SWORD : daPy getSwordTopPos/getCutAtFlg have no port accessor —
 *              [PORT-INTEGRATION REVIEW] fall direction approximates with the
 *              player's facing angle; exact sword-tip tracking is an owed
 *              refinement (affects which way a cut swood tips, nothing else).
 *   #7 SOUND : JA_SE_OBJ_CUT_TREE_DOWN / JA_SE_OBJ_TREE_SWING are not in the
 *              port SE map yet — audio-lane add; v1 is silent with a one-shot
 *              log (Housing's "real failure mode logs stay" doctrine).
 *   #8 CLIP  : donor uses mDoLib_clipper far-scaling; port path draws all
 *              registered swood (counts are tiny: 2-4 per room) — perf-only
 *              delta, bannered not hidden.
 * Traps honored (§243/§249): donor never calls MassClear from the tree path
 * (the PrepareMass/SetMassAttr bracket is per-frame in calc, mirroring the
 * donor's own calc); per-actor probes stay per-actor; spawn mechanism split
 * by PLANT (this module = swood only; kusa/flowers stay in ext_vegetation).
 * ============================================================================
 */
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_tree.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor("GZLJ01", "GZLE01", "GZLP01")

#include "d/d_ext_tree.h"

#include <filesystem>
#include <fstream>
#include <vector>

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_ww.h"  // §694 donor C0/K0 tevstr fill
#include "d/d_particle_name.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_overlap_mng.h"
#include "m_Do/m_Do_ext.h"
#include "SSystem/SComponent/c_counter.h"  // g_Counter
#include "m_Do/m_Do_mtx.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/d_ext_ww_actor_shims.h"
#include "dusk/logging.h"

extern aurora::Module DuskLog;

// §366 — WW d_cc_d.h AT bits the shims header does not already carry
// (VALUE-FAITHFUL donor numbering; lamp §327 local-constant pattern).
#ifndef AT_TYPE_BOMB
#define AT_TYPE_BOMB         (1 << 5)
#endif
#ifndef AT_TYPE_FIRE
#define AT_TYPE_FIRE         (1 << 9)
#endif
#ifndef AT_TYPE_NORMAL_ARROW
#define AT_TYPE_NORMAL_ARROW (1 << 14)
#endif
#ifndef AT_TYPE_HOOKSHOT
#define AT_TYPE_HOOKSHOT     (1 << 15)
#endif
#ifndef AT_TYPE_FIRE_ARROW
#define AT_TYPE_FIRE_ARROW   (1 << 18)
#endif
#ifndef AT_TYPE_ICE_ARROW
#define AT_TYPE_ICE_ARROW    (1 << 19)
#endif
#ifndef AT_TYPE_LIGHT_ARROW
#define AT_TYPE_LIGHT_ARROW  (1 << 20)
#endif

namespace {

// ---------------------------------------------------------------------------
// §366 #2 — blob pack. Names fixed by Housing's staging; donor CALL sizes.
// ---------------------------------------------------------------------------
struct TreeAssets {
    bool ready = false;
    std::vector<u8> pos, color, texCoord, matDL;
    std::vector<u8> dl[5];  // none, hapa, miki, cutt, cutu (l_modelDataStatus order)
    std::vector<u8> shadowMatDL, kageDL;
    std::vector<u8> tex, kageTex;
};
TreeAssets s_assets;
GXTexObj s_texObj;
GXTexObj s_kageTexObj;
bool s_texReady = false;

// KIT-DONOR-DATA: 20 lookup-table d/d_tree.cpp draw table
const u32 kDLBytes[5] = {0x20, 0x160, 0xC0, 0xA0, 0x60};  // donor draw table
const u32 kMatDLBytes = 0xA0;
const u32 kShadowMatDLBytes = 0x80;
const u32 kKageDLBytes = 0x20;

// donor d_tree.cpp:30-45 — small inline arrays, carried verbatim (s8/u8 data)
// KIT-DONOR-DATA: 12 asset-like d/d_tree.cpp:30-45
u8 s_shadowPos[][3] = {
    {0xE2, 0x00, 0xE2}, {0x1E, 0x00, 0xE2}, {0x1E, 0x00, 0x1E}, {0xE2, 0x00, 0x1E},
};
// KIT-DONOR-DATA: 8 asset-like d/d_tree.cpp:30-45
u8 s_shadowTexCoord[][2] = {
    {0x00, 0x00}, {0x01, 0x00}, {0x01, 0x01}, {0x00, 0x01},
};

bool readBlob(const std::filesystem::path& p, std::vector<u8>& out) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    const std::streamoff len = in.tellg();
    if (len <= 0) {
        return false;
    }
    in.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(len));
    in.read(reinterpret_cast<char*>(out.data()), len);
    return in.good() || in.eof();
}

bool loadAssets() {
    if (s_assets.ready) {
        return true;
    }
    namespace fs = std::filesystem;
    const char* appdata = getenv("APPDATA");
    if (appdata == NULL) {
        return false;
    }
    fs::path veg = fs::path(appdata) / "TwilitRealm" / "Dusklight" /
                   "model_replacements" / "WW-Crew-Restoration" / "assets" / "veg";
    static const char* kDlNames[5] = {
        "d_tree__l_Oba_swood_noneDL.bin", "d_tree__l_Oba_swood_a_hapaDL.bin",
        "d_tree__l_Oba_swood_a_mikiDL.bin", "d_tree__l_Oba_swood_a_cuttDL.bin",
        "d_tree__l_Oba_swood_a_cutuDL.bin"};
    bool ok = readBlob(veg / "d_tree__l_pos.bin", s_assets.pos) &&
              readBlob(veg / "d_tree__l_color.bin", s_assets.color) &&
              readBlob(veg / "d_tree__l_texCoord.bin", s_assets.texCoord) &&
              readBlob(veg / "d_tree__l_matDL.bin", s_assets.matDL) &&
              readBlob(veg / "d_tree__g_dTree_shadowMatDL.bin", s_assets.shadowMatDL) &&
              readBlob(veg / "d_tree__g_dTree_Oba_kage_32DL.bin", s_assets.kageDL) &&
              readBlob(veg / "d_tree__l_Txa_swood_aTEX.bin", s_assets.tex) &&
              readBlob(veg / "d_tree__l_Txa_kage_32TEX.bin", s_assets.kageTex);
    for (int i = 0; i < 5 && ok; i++) {
        ok = readBlob(veg / kDlNames[i], s_assets.dl[i]);
    }
    if (!ok) {
        // Housing §61 law: a refused pack must SAY so — silence is ambiguous.
        DuskLog.warn("[ExtTree] §366 blob pack refused (assets/veg/d_tree__*) — "
                     "swood stays inert");
        return false;
    }
    for (int i = 0; i < 5; i++) {
        if (s_assets.dl[i].size() < kDLBytes[i]) {
            DuskLog.warn("[ExtTree] §366 DL {} short ({} < {}) — refusing pack",
                         i, s_assets.dl[i].size(), kDLBytes[i]);
            return false;
        }
    }
    // §366 #3 — texture objects from the raw blobs (donor pointers are dead).
    GXInitTexObj(&s_texObj, s_assets.tex.data(), 64, 64, GX_TF_CMPR,
                 GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObj(&s_kageTexObj, s_assets.kageTex.data(), 32, 32, GX_TF_I4,
                 GX_CLAMP, GX_CLAMP, GX_FALSE);
    s_texReady = true;
    s_assets.ready = true;
    DuskLog.info("[ExtTree] §366 swood pack loaded (5 DLs + mat/shadow/kage + 2 TEX)");
    return true;
}

dExtTree_packet_c s_packet;
bool s_cutSoundFlag;  // donor l_CutSoundFlag
u32 s_execFrame = 0xFFFFFFFF;
u32 s_drawFrame = 0xFFFFFFFF;

f32 checkGroundY(dExtTree_data_c* i_data, cXyz& i_pos) {
    // donor d_tree.cpp:checkGroundY — verbatim (Bgsp reference idiom, §329 rule)
    dBgS_GndChk chk;
    i_pos.y += 50.0f;
    chk.SetPos(&i_pos);
    f32 y = dComIfG_Bgsp().GroundCross(&chk);
    i_pos.y -= 50.0f;

    cXyz norm;
    if (y <= -G_CM3D_F_INF) {
        y = i_pos.y;
        norm.set(0.0f, 1.0f, 0.0f);
    } else {
        cM3dGPla plane;
        if (dComIfG_Bgsp().GetTriPla(chk, &plane)) {
            norm = plane.mNormal;
        } else {
            norm.set(0.0f, 1.0f, 0.0f);
        }
    }

    f32 var_f4 = std::sqrtf(1.0f - (norm.x * norm.x));
    f32 var_f2;
    f32 var_f3;
    if (var_f4 != 0.0f) {
        var_f2 = norm.y * var_f4;
        var_f3 = -norm.z * var_f4;
    } else {
        var_f2 = 0.0f;
        var_f3 = 0.0f;
    }

    i_data->mShadowMtx[0][0] = var_f4;
    i_data->mShadowMtx[0][1] = norm.x;
    i_data->mShadowMtx[0][2] = 0.0f;
    i_data->mShadowMtx[0][3] = i_pos.x;

    i_data->mShadowMtx[1][0] = -norm.x * var_f2;
    i_data->mShadowMtx[1][1] = norm.y;
    i_data->mShadowMtx[1][2] = var_f3;
    i_data->mShadowMtx[1][3] = y + 1.0f;

    i_data->mShadowMtx[2][0] = norm.x * var_f3;
    i_data->mShadowMtx[2][1] = norm.z;
    i_data->mShadowMtx[2][2] = var_f2;
    i_data->mShadowMtx[2][3] = i_pos.z;

    return y;
}

}  // namespace

// ---------------------------------------------------------------------------
// dExtTree_data_c — donor dTree_data_c, method for method
// ---------------------------------------------------------------------------
dExtTree_data_c::dExtTree_data_c() {
    mState = 0x00;
    field_0x001 = 0;
    mAnimIdx = 0;
    field_0x003 = 0xFF;
    mpNext = NULL;
}

void dExtTree_data_c::WorkCo(fopAc_ac_c* i_actor, int) {
    // donor WorkCo (run-through sway; CO-driven, NOT distance-driven — §249 trap)
    if (field_0x001) {
        return;
    }
    if (mAnimIdx < 8) {
        dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);
        s32 newAnm = s_packet.newAnm(anm->field_0x02);
        if (newAnm < 0) {
            return;
        }
        mAnimIdx = (s8)newAnm;
    }

    cXyz vel;
    vel.x = mPos.x - i_actor->current.pos.x;
    vel.y = mPos.y - i_actor->current.pos.y;
    vel.z = mPos.z - i_actor->current.pos.z;
    vel.abs2XZ();

    dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);

    s16 angle = cM_atan2s(vel.x, vel.z);
    f32 cos = cM_scos(i_actor->current.angle.y - angle);
    // donor: onFrollCrashFlg(7) when rolling square into the trunk —
    // [PORT-INTEGRATION REVIEW §366 #6]: no port accessor; sway still runs.

    f32 var_f1 = cLib_maxLimit(std::abs(i_actor->speedF * 50.0f), 4000.0f);
    if (std::fabsf(anm->mAnimTimer) < std::fabsf(var_f1 * cos)) {
        anm->mAnimTimer = var_f1 * cos;
        anm->field_0x0a = angle;
    }
}

void dExtTree_data_c::WorkAt_NoCutAnim(fopAc_ac_c* i_actor, int, cCcD_Obj* i_hitObj) {
    // donor WorkAt_NoCutAnim (non-cut hits: wind/bomb/arrows — sway only)
    if (field_0x001) {
        return;
    }
    if (mAnimIdx < 8) {
        dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);
        s32 newAnm = s_packet.newAnm(anm->field_0x02);
        if (newAnm < 0) {
            return;
        }
        mAnimIdx = (s8)newAnm;
    }

    dCcD_GObjInf* hitObjInf = dCcD_GetGObjInf(i_hitObj);
    cXyz vel = *hitObjInf->GetAtVecP();
    f32 mag = vel.absXZ();

    if (cM3d_IsZero(mag)) {
        if (fopAcM_GetName(i_actor) == fpcNm_ALINK_e) {
            vel.x = mPos.x - i_actor->current.pos.x;
            vel.y = mPos.y - i_actor->current.pos.y;
            vel.z = mPos.z - i_actor->current.pos.z;
            mag = vel.absXZ();
        } else if (false) {  // [PORT-INTEGRATION §366: donor BOMB/Bomb2
            // proximity-push — TP bomb proc ids differ; sway-only until wired.
            vel.x = mPos.x - i_actor->current.pos.x;
            vel.y = mPos.y - i_actor->current.pos.y;
            vel.z = mPos.z - i_actor->current.pos.z;
            mag = 200.0f - vel.absXZ();
            if (mag < 0.0f) {
                mag = 0.0f;
            }
            VECNormalize(&vel, &vel);
            VECScale(&vel, &vel, mag);
        }
    }

    dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);

    s16 var_r30 = cM_atan2s(vel.x, vel.z);
    f32 anm_timer = mag * 50.0f > 4000.0f ? 4000.0f : mag * 50.0f;
    if (std::fabsf(anm->mAnimTimer) < std::fabsf(anm_timer)) {
        anm->mAnimTimer = anm_timer;
        anm->field_0x0a = var_r30;
    }
}

void dExtTree_data_c::WorkAt(fopAc_ac_c* i_actor, int i_roomNo, dCcMassS_HitInf* i_hitInf) {
    // donor WorkAt — CUT vs non-cut dispatch by AT type (full state machine)
    cCcD_Obj* hitObj = i_hitInf->GetAtHitObj();
    if (hitObj != NULL && (hitObj->ChkAtType(AT_TYPE_WIND) ||
        hitObj->ChkAtType(AT_TYPE_BOMB) ||
        hitObj->ChkAtType(AT_TYPE_FIRE) ||
        hitObj->ChkAtType(AT_TYPE_NORMAL_ARROW) ||
        hitObj->ChkAtType(AT_TYPE_FIRE_ARROW) ||
        hitObj->ChkAtType(AT_TYPE_ICE_ARROW) ||
        hitObj->ChkAtType(AT_TYPE_LIGHT_ARROW) ||
        hitObj->ChkAtType(AT_TYPE_HOOKSHOT)))
    {
        WorkAt_NoCutAnim(i_actor, i_roomNo, hitObj);
    } else {
        if (field_0x001) {
            return;
        }
        if (mAnimIdx < 8) {
            dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);
            s32 newAnm = s_packet.newAnm(anm->field_0x02);
            if (newAnm < 0) {
                return;
            }
            mAnimIdx = (s8)newAnm;
        }

        field_0x001 = 1;  // CUT
        dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);
        anm->field_0x0a = s_packet.getPlayerSwordMoveAngY();
        anm->mAnimTimer = 0.0f;
        anm->field_0x10 = -2.0f;
        anm->field_0x14 = 12.0f;

        Mtx m;
        MTXScale(m, 0.4f, 1.0f, 0.4f);
        MTXConcat(mShadowMtx, m, mShadowMtx);

        if (!s_cutSoundFlag) {
            s_cutSoundFlag = true;
            // §366 #7 — JA_SE_OBJ_CUT_TREE_DOWN not in port SE map yet.
            static bool s_seNote = false;
            if (!s_seNote) {
                s_seNote = true;
                DuskLog.info("[ExtTree] §366 #7 OWED: cut-down SE "
                             "(JA_SE_OBJ_CUT_TREE_DOWN) — audio-lane add");
            }
        }
        dComIfGp_getVibration().StartShock(4, -0x21, cXyz(0.0f, 1.0f, 0.0f));
    }
}

BOOL dExtTree_data_c::hitCheck(fopAc_ac_c*, int i_roomNo) {
    // donor hitCheck — ChkMass per swood per frame (no per-tree collision obj)
    fopAc_ac_c* actor = NULL;
    dCcMassS_HitInf hitInf;
    u32 ret = dComIfG_Ccsp()->ChkMass(&mPos, &actor, &hitInf);

    if (ret == 0 || actor == NULL) {
        return 0;
    }
    if (ret & 2)
        WorkCo(actor, i_roomNo);
    if (ret & 1)
        WorkAt(actor, i_roomNo, &hitInf);
    return 0;
}

void dExtTree_data_c::animation(int i_roomNo) {
    // donor animation — intact sway settle + cut fall sequence, verbatim
    dExtTree_anm_c* anm = s_packet.getAnm(mAnimIdx);

    if (!field_0x001) {
        f32 temp = anm->field_0x08 * -1.0f;
        anm->mAnimTimer += temp;
        anm->mAnimTimer *= 0.9f;
        s16 temp2 = (s16)anm->mAnimTimer;
        anm->field_0x08 += temp2;

        if (anm->field_0x08 * temp >= 0.0f) {
            f32 var_f0 = std::fabsf(anm->mAnimTimer);
            if (var_f0 < 40.0f) {
                anm->field_0x08 = 0;
                anm->mAnimTimer = 0.0f;

                int idx = (anm->field_0x02 >> 13) & 7;
                dExtTree_anm_c* anm2 = s_packet.getAnm(idx);
                if (cLib_chaseAngleS(&anm->field_0x04, anm2->field_0x04, 800)
                    && cLib_chaseAngleS(&anm->field_0x06, anm2->field_0x06, 800))
                {
                    s_packet.deleteAnm(mAnimIdx);
                    mAnimIdx = (s8)idx;
                }
            } else if (var_f0 > 400.0f) {
                cXyz pos(mPos.x, mPos.y + 203.0f, mPos.z);
                static GXColor s_white = {0xFF, 0xFF, 0xFF, 0xFF};
                dComIfGp_particle_setSimple(ID_IT_JN_O_HAPPA01, &pos, 0xFF,
                                            s_white, s_white, 0, 0.0f);
                // §366 #7 — TREE_SWING SE owed (audio lane); silent v1
            }
        }

        anm->field_0x04 = anm->field_0x08;
    } else {
        f32 var_f0 = anm->field_0x10 * cM_ssin(anm->field_0x0a);
        f32 var_f0_2 = anm->field_0x10 * cM_scos(anm->field_0x0a);

        anm->field_0x14 += -2.0f;
        if (anm->field_0x14 < -40.0f) {
            anm->field_0x14 = -40.0f;
        }

        anm->field_0x18 += var_f0;
        anm->field_0x20 += var_f0_2;
        anm->field_0x1c += anm->field_0x14;

        if (anm->field_0x1c < -5.0f) {
            anm->field_0x1c = -5.0f;
            anm->field_0x10 = 0.0f;
        }

        cLib_chaseF(&anm->mAnimTimer, 1500.0f, 40.0f);

        if (anm->field_0x04 != 13600) {
            if (cLib_chaseAngleS(&anm->field_0x04, 13600, (s16)anm->mAnimTimer)) {
                static cXyz l_particleOffset(0.0f, 150.0f, 0.0f);
                cXyz sp18;
                MTXMultVec(anm->field_0x24, (Vec*)&l_particleOffset, (Vec*)&sp18);
                // donor: toon smoke ELEMENTSMOKE00 + HAPPA02 leaves at impact.
                // setToon's port arity differs across lanes — the leaves burst
                // carries the read; smoke rides the same simple path (§209 law:
                // port the donor CALL as close as the receiver allows, never
                // invent an equivalent).
                static GXColor s_white2 = {0xFF, 0xFF, 0xFF, 0xFF};
                dComIfGp_particle_setSimple(ID_AK_JT_ELEMENTSMOKE00,
                                            &sp18, 0xAA, s_white2, s_white2, 0, 0.0f);
                sp18.y -= 50.0f;
                dComIfGp_particle_setSimple(ID_IT_JN_HAPPA02, &sp18, 0xFF,
                                            s_white2, s_white2, 0, 0.0f);
            }
        } else if (anm->field_0x10 == 0.0f) {
            cLib_onBit<u8>(mState, 8);
            MTXCopy(anm->field_0x24, field_0x070);
            field_0x070[0][3] = mPos.x + anm->field_0x18;
            field_0x070[1][3] = mPos.y + anm->field_0x1c;
            field_0x070[2][3] = mPos.z + anm->field_0x20;

            s_packet.deleteAnm(mAnimIdx);
            mAnimIdx = (s8)((anm->field_0x02 >> 13) & 7);
        }
    }
}

// ---------------------------------------------------------------------------
// room / packet — donor verbatim
// ---------------------------------------------------------------------------
void dExtTree_room_c::newData(dExtTree_data_c* i_data) {
    i_data->mpNext = mpData;
    mpData = i_data;
}

void dExtTree_room_c::deleteData() {
    while (mpData != NULL) {
        mpData->mState = 0x00;  // WW cLib_setBit = assignment
        mpData = mpData->mpNext;
    }
}

dExtTree_packet_c::dExtTree_packet_c() {
    mNextIdx = 0;
    mPlayerSwordMoveAngY = 0;
    dExtTree_data_c* data = mData;
    for (s32 i = 0; i < 64; i++, data++)
        data->mState = 0x00;  // WW cLib_setBit = assignment

    dExtTree_anm_c* anm = mAnm;
    for (s32 i = 0; i < 72; i++, anm++)
        anm->mState = 0;

    s16 angle = 0;
    for (s32 i = 0; i < 8; i++, angle += 0x2000)
        setAnm(i, angle);
}

void dExtTree_packet_c::calc(fopAc_ac_c* i_player, int i_roomNo) {
    // donor calc — ambient sway drive + player sword state + per-tree checks
    dExtTree_anm_c* anm = getAnm();
    for (s32 i = 0; i < 8; anm++, i++) {
        anm->field_0x06 = (s16)(anm->field_0x02 + 100.0f +
                                cM_scos((s16)(((u32)g_Counter.mTimer + (i * 250)) * 4000)) * 100.0f);
        anm->field_0x04 = (s16)(100.0f + cM_scos((s16)(((u32)g_Counter.mTimer + (i * 250)) * 1000)) * 100.0f);
    }

    dExtTree_data_c* data = mRoom[i_roomNo & 63].getData();
    if (data == NULL) {
        return;
    }

    // §366 #6 — sword-tip tracking approximated with facing (bannered above).
    mPlayerSwordMoveAngY = i_player->current.angle.y;

    s_cutSoundFlag = false;
    // donor: SetMassAttr(40, 200, 19, 1). №225's law says the RECEIVER's attack
    // geometry sizes the test volume — 200 already clears TP's swing planes
    // (the №225 issue was grass's 80), so the donor number stands as-is.
    dComIfG_Ccsp()->SetMassAttr(40.0f, 200.0f, 19, 1);

    while (data != NULL) {
        if (!cLib_checkBit<u8>(data->mState, 0x08))
            data->hitCheck(i_player, i_roomNo);
        if (data->mAnimIdx >= 8)
            data->animation(i_roomNo);
        data = data->mpNext;
    }
}

void dExtTree_packet_c::setData(dExtTree_data_c* i_data, int i_nextIdx, cXyz& i_pos,
                                u8 i_cut, int i_roomNo) {
    f32 y;
    if (fopOvlpM_IsPeek()) {
        y = checkGroundY(i_data, i_pos);
        i_data->mState = 0x02 | 0x04;  // WW cLib_setBit = assignment
    } else {
        y = i_pos.y;
        i_data->mState = 0x02 | 0x04 | 0x10;
    }

    i_data->field_0x001 = i_cut;
    i_data->field_0x003 = i_cut == 1 ? 0 : 0xFF;
    i_data->mAnimIdx = (s8)(u8)cM_rndF(7.0f);
    i_data->mPos.set(i_pos.x, y, i_pos.z);
    mRoom[i_roomNo & 63].newData(i_data);
    mNextIdx = (u16)i_nextIdx;
}

dExtTree_data_c* dExtTree_packet_c::newData(cXyz& i_pos, u8 i_cut, int i_roomNo) {
    if (i_roomNo < 0 || i_roomNo >= 64) {
        return NULL;  // donor JUT_ASSERT -> guard (§329 idiom)
    }

    dExtTree_data_c* data = &mData[mNextIdx];
    s32 i = mNextIdx;
    for (; i < 64; data++, i++) {
        if (!cLib_checkBit<u8>(data->mState, 2)) {
            setData(data, i, i_pos, i_cut, i_roomNo);
            return data;
        }
    }
    data = getData();
    for (i = 0; i < mNextIdx; data++, i++) {
        if (!cLib_checkBit<u8>(data->mState, 2)) {
            setData(data, i, i_pos, i_cut, i_roomNo);
            return data;
        }
    }
    return NULL;
}

s32 dExtTree_packet_c::newAnm(s16 i_baseAng) {
    dExtTree_anm_c* anm = &mAnm[8];
    for (s32 i = 8; i < 72; anm++, i++) {
        if (anm->mState == 0) {
            setAnm(i, i_baseAng);
            return i;
        }
    }
    return -1;
}

void dExtTree_packet_c::setAnm(int i_no, s16 i_baseAng) {
    dExtTree_anm_c* anm = getAnm(i_no);
    anm->mState = 1;
    anm->field_0x02 = i_baseAng;
    anm->field_0x06 = i_baseAng;
    anm->field_0x04 = 0;
    anm->field_0x08 = 0;
    anm->field_0x0a = 0;
    anm->mAnimTimer = 0;
    anm->field_0x10 = 0;
    anm->field_0x14 = 0;
    anm->field_0x18 = 0;
    anm->field_0x1c = 0;
    anm->field_0x20 = 0;
}

void dExtTree_packet_c::draw() {
    // ========================================================================
    // §366 #1 — donor update()'s matrix math runs here (view is current), then
    // donor draw()'s GX, both verbatim in content.
    // ========================================================================
    if (!s_assets.ready) {
        return;
    }

    dExtTree_anm_c* anm = getAnm();
    for (int i = 0; i < 72; i++, anm++) {
        mDoMtx_stack_c::YrotS(anm->field_0x0a);
        mDoMtx_stack_c::XrotM(anm->field_0x04);
        mDoMtx_stack_c::YrotM((s16)(anm->field_0x06 - anm->field_0x0a));
        mDoMtx_copy(mDoMtx_stack_c::get(), anm->field_0x24);

        mDoMtx_stack_c::YrotS(anm->field_0x0a);
        mDoMtx_stack_c::XrotM(anm->field_0x08);
        mDoMtx_stack_c::YrotM((s16)(anm->field_0x02 - anm->field_0x0a));
        mDoMtx_copy(mDoMtx_stack_c::get(), anm->field_0x54);
    }

    dExtTree_data_c* data = getData();
    anm = getAnm();
    s32 numPerFrame = 0;
    for (int i = 0; i < 64; i++, data++) {
        if (!cLib_checkBit<u8>(data->mState, 2)) {
            continue;
        }
        if (cLib_checkBit<u8>(data->mState, 0x10) && numPerFrame < 4) {
            data->mPos.y = checkGroundY(data, data->mPos);
            cLib_offBit<u8>(data->mState, 0x10);
            numPerFrame++;
        }
        // §366 #8 — donor far-clip test skipped (2-4 swood per room); always in.
        cLib_offBit<u8>(data->mState, 0x04);

        dExtTree_anm_c* a = &anm[data->mAnimIdx];
        if (cLib_checkBit<u8>(data->mState, 8)) {
            mDoMtx_concat(j3dSys.getViewMtx(), data->field_0x070, data->field_0x010);
            cLib_chaseUC(&data->field_0x003, 0, 8);
        } else {
            a->field_0x24[0][3] = data->mPos.x + a->field_0x18;
            a->field_0x24[1][3] = data->mPos.y + a->field_0x1c;
            a->field_0x24[2][3] = data->mPos.z + a->field_0x20;
            mDoMtx_concat(j3dSys.getViewMtx(), a->field_0x24, data->field_0x010);
        }

        a->field_0x54[0][3] = data->mPos.x;
        a->field_0x54[1][3] = data->mPos.y;
        a->field_0x54[2][3] = data->mPos.z;

        mDoMtx_concat(j3dSys.getViewMtx(), a->field_0x54, data->field_0x040);
        mDoMtx_concat(j3dSys.getViewMtx(), data->mShadowMtx, data->field_0x0d0);
    }

    // ---- donor dTree_packet_c::draw GX, adapted per #3/#4/#5 ----
    static GXVtxDescList l_shadowVtxDescList[] = {
        {GX_VA_POS, GX_INDEX8},
        {GX_VA_TEX0, GX_INDEX8},
        {GX_VA_NULL, GX_NONE},
    };
    static GXVtxAttrFmtList l_shadowVtxAttrFmtList[] = {
        {GX_VA_POS, GX_POS_XYZ, GX_S8, 0},
        {GX_VA_TEX0, GX_TEX_ST, GX_S8, 0},
        {GX_VA_NULL, GX_POS_XYZ, GX_S8, 0},
    };
    static GXVtxDescList l_vtxDescList[] = {
        {GX_VA_POS, GX_INDEX8},
        {GX_VA_CLR0, GX_INDEX8},
        {GX_VA_TEX0, GX_INDEX8},
        {GX_VA_NULL, GX_NONE},
    };
    static GXVtxAttrFmtList l_vtxAttrFmtList[] = {
        {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
        {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
        {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
        {GX_VA_NULL, GX_POS_XYZ, GX_S8, 0},
    };
    static GXColor l_shadowColor = {0x00, 0x00, 0x00, 0x64};
    static u8 l_modelStatus[2][3] = {{0, 1, 2}, {3, 1, 4}};

    j3dSys.reinitGX();
    GXSetNumIndStages(0);
    dKy_setLight_again();  // №142 — lit channel needs lights configured

    // ========================================================================
    // §477 ROOT-CAUSE FIX. GXSetVtxDescv only sets the attributes PRESENT in
    // its list; anything absent keeps whatever the previous drawer left. Worse,
    // aurora's implementation then RE-APPLIES __gx->nrmType whenever
    // __gx->hasNrms is still set (GXGeometry.cpp), and hasNrms is sticky.
    //
    // This actor's lists declare POS/CLR0/TEX0 as INDEX8 and NO normal at all
    // -- 3 bytes per vertex, matching its display lists. But when the WW rope
    // material has drawn first it leaves hasNrms=1 with nrmType=INDEX16, so the
    // absent NRM came back as INDEX16 and the descriptor silently became 5
    // bytes per vertex. The FIFO parser then over-ran every strip in a 352-byte
    // (0x160 = kDLBytes[1]) geometry list, read vertex data as opcodes, and
    // corrupted the stream -- the crash chased from §448 to §476.
    //
    // Clearing first makes this actor establish its OWN complete descriptor
    // instead of inheriting one, which is the standard GX idiom and what every
    // other drawer here already does.
    // ========================================================================
    GXClearVtxDesc();
    // shadow pass
    GXSetVtxDescv(l_shadowVtxDescList);
    GXSetVtxAttrFmtv(GX_VTXFMT0, l_shadowVtxAttrFmtList);
    GXSetArray(GX_VA_POS, s_shadowPos, sizeof(s_shadowPos), 3, false);
    GXSetArray(GX_VA_TEX0, s_shadowTexCoord, sizeof(s_shadowTexCoord), 2, false);
    GXCallDisplayList(s_assets.shadowMatDL.data(), kShadowMatDLBytes);
    GXLoadTexObj(&s_kageTexObj, GX_TEXMAP0);  // #3: DL's baked tex ptr is dead
    GXSetTevColor(GX_TEVREG0, l_shadowColor);

    for (s32 i = 0; i < 64; i++) {
        dExtTree_room_c* room = &mRoom[i];
        for (dExtTree_data_c* data = room->getData(); data != NULL; data = data->mpNext) {
            if (!cLib_checkBit<u8>(data->mState, 0x04)) {
                GXLoadPosMtxImm(data->field_0x0d0, GX_PNMTX0);
                GXCallDisplayList(s_assets.kageDL.data(), kKageDLBytes);
            }
        }
    }

    // model pass — §477: same reasoning as the shadow pass above; establish the
    // full descriptor rather than inheriting a stale NRM.
    GXClearVtxDesc();
    GXSetVtxDescv(l_vtxDescList);
    GXSetVtxAttrFmtv(GX_VTXFMT0, l_vtxAttrFmtList);
    GXSetArray(GX_VA_POS, s_assets.pos.data(), (u32)s_assets.pos.size(),
               sizeof(cXyz), false);
    GXSetArray(GX_VA_CLR0, s_assets.color.data(), (u32)s_assets.color.size(),
               sizeof(GXColor), false);
    GXSetArray(GX_VA_TEX0, s_assets.texCoord.data(), (u32)s_assets.texCoord.size(),
               8, false);
    GXCallDisplayList(s_assets.matDL.data(), kMatDLBytes);
    GXLoadTexObj(&s_texObj, GX_TEXMAP0);  // #3

    for (s32 i = 0; i < 64; i++) {
        dExtTree_room_c* room = &mRoom[i];
        dExtTree_data_c* first = room->getData();
        if (first == NULL) {
            continue;
        }
        // #4 (№141): own tevstr, not the mounted-host room table.
        static dKy_tevstr_c s_tevStr;
        // §694: donor register pair (see ext_vegetation §694) — the white-K0
        // idiom was the placeholder for the then-unfilled K0 pool. Donor swood
        // feeds REG0/REG1 from the room tevstr C0/K0 (WW d_wood.cpp:922-923).
        dKyWw_settingTevStruct(TEV_TYPE_BG0, &first->mPos, &s_tevStr);
        GXColorS10 c0;
        c0.r = s_tevStr.TevColor.r; c0.g = s_tevStr.TevColor.g;
        c0.b = s_tevStr.TevColor.b; c0.a = 255;
        GXColor k0 = s_tevStr.TevKColor;
        k0.a = 255;
        GXSetTevColorS10(GX_TEVREG0, c0);
        GXSetTevColor(GX_TEVREG1, k0);
        dKy_GxFog_tevstr_set(&s_tevStr);
        for (dExtTree_data_c* data = first; data != NULL; data = data->mpNext) {
            if (cLib_checkBit<u8>(data->mState, 0x04)) {
                continue;
            }
            u8* sel = l_modelStatus[data->field_0x001 ? 1 : 0];
            if (data->field_0x003 != 0) {
                GXColor sp = {0, 0, 0, 0};
                sp.a = data->field_0x003;
                GXSetTevColor(GX_TEVREG2, sp);
                GXLoadPosMtxImm(data->field_0x010, GX_PNMTX0);
                GXCallDisplayList(s_assets.dl[sel[0]].data(), kDLBytes[sel[0]]);
                GXCallDisplayList(s_assets.dl[sel[1]].data(), kDLBytes[sel[1]]);
            }
            GXLoadPosMtxImm(data->field_0x040, GX_PNMTX0);
            GXCallDisplayList(s_assets.dl[sel[2]].data(), kDLBytes[sel[2]]);
        }
    }
}

// ---------------------------------------------------------------------------
// dispatcher surface
// ---------------------------------------------------------------------------
dExtTree_data_c* dExtTree_newData(cXyz& i_pos, int i_roomNo) {
    if (!loadAssets()) {
        return NULL;
    }
    return s_packet.newData(i_pos, 0 /*intact*/, i_roomNo);
}

void dExtTree_execute(fopAc_ac_c* i_player, int i_roomNo) {
    const u32 frame = (u32)g_Counter.mTimer;
    if (frame == s_execFrame) {
        return;  // one calc per frame regardless of how many swood actors run
    }
    s_execFrame = frame;
    if (i_player != NULL) {
        s_packet.calc(i_player, i_roomNo);
    }
}

void dExtTree_entryDraw() {
    const u32 frame = (u32)g_Counter.mTimer;
    if (frame == s_drawFrame || !s_assets.ready) {
        return;
    }
    s_drawFrame = frame;
    // donor update() tail: XLU BG list entry (d_tree.cpp:8007A0xx)
    dComIfGd_setXluListBG();
    j3dSys.getDrawBuffer(1)->entryImm(&s_packet, 0);
    dComIfGd_setList();  // restore default lists (port inline set)
}

void dExtTree_deleteRoom(int i_roomNo) {
    if (i_roomNo >= 0 && i_roomNo < 64) {
        s_packet.deleteRoom(i_roomNo);
    }
}

bool dExtTree_assetsReady() {
    return s_assets.ready;
}
