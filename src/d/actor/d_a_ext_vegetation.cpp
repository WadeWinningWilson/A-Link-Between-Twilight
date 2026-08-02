/**
 * d_a_ext_vegetation.cpp
 *
 * №122: donor grass for donor spaces (`kusax1` / `kusax7` / `kusax21`).
 *
 * =========================================================================
 * WHY A SEPARATE ACTOR INSTEAD OF THE NATIVE GRASS
 * =========================================================================
 * The native build already spawns these census names — `d_stage.cpp` maps
 * `kusax1/7/21` to `fpcNm_GRASS_e` and ships a full `dGrass_packet_c`. The
 * architecture was inherited, so routing donor placements there would "work".
 *
 * It is still wrong. The ART differs: the native grass draws
 * `M_kusa05_RGBATEX` / `M_Hijiki00TEX` (31x31), the donor's draws
 * `l_Txa_ob_kusa_aTEX` (64x64) off a different vertex set. Sending donor
 * placements through the native actor would plant receiver grass on a donor
 * island, which the space-purity law forbids outright. Shared architecture is
 * not shared assets.
 *
 * =========================================================================
 * WHERE THE ASSETS COME FROM
 * =========================================================================
 * The donor's grass is not an overlay — it is a packet system compiled into
 * the donor's main executable with static texture / display-list / vertex
 * arrays. Those bytes may never enter this repo, so `extract_veg_assets.py`
 * pulls them (address -> file offset via the executable header, sizes from the
 * shipped symbol map) into the mod folder, and this actor loads them at
 * runtime. Nothing donor-authored is compiled in.
 *
 * Draw mirrors the donor's own packet draw: set the vertex descriptors once,
 * bind the three arrays, call the material DL, then per instance load a
 * position matrix and call the blade DL. One instance does the drawing for the
 * whole list so the GX setup is paid once per frame, not once per clump.
 *
 * Blobs are raw big-endian executable bytes. The native grass stores its own
 * arrays the same way (`static u8 l_pos[960]` of big-endian floats) and passes
 * `le=true` to GXSETARRAY, so the identical flag applies here.
 * =========================================================================
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <filesystem>

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_xyz.h"
#include "SSystem/SComponent/c_counter.h"
#include "d/actor/d_a_player.h"  // №220: horse check for direct-get drops
#include "d/d_bg_s_gnd_chk.h"
#include "d/d_cc_s.h"
#include "d/d_particle.h"  // №220: cut-scatter particle + light callback
#include "Z2AudioLib/Z2SeMgr.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "m_Do/m_Do_lib.h"
#include "m_Do/m_Do_mtx.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXVert.h"
#include "d/d_cc_d.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_kankyo_tev_str.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"

// ==========================================================================
// Asset pack — loaded once, shared by every clump.
// ==========================================================================
namespace {

struct ExtVegAssets {
    std::vector<u8> pos;
    std::vector<u8> color;
    std::vector<u8> texCoord;
    std::vector<u8> matDL;
    std::vector<u8> bladeDL;
    std::vector<u8> bladeCutDL;
    std::vector<u8> tex;
    // ========================================================================
    // №220 — cut rewards: which global item table a cut blade rolls on.
    // ========================================================================
    // The receiver's item tables are GLOBAL (loaded once at boot from
    // res/ItemTable/item_table.bin, d_s_logo), so any table number is valid on
    // the host stage. Default 150 = the receiver's health-scaled mercy family
    // (NONE x8 / GREEN x5 / HEART x2 / BLUE x1, shifting toward hearts as the
    // player's HP drops — fopAcM_getItemNoFromTableNo bumps 150→151..154).
    // That family exists in the receiver precisely for grass/pot drops, which
    // makes it the receiver-native analogue of the donor's per-clump grass
    // tables. Overridable per pack with `item_table=` (-1 disables drops).
    //
    // ========================================================================
    // №225 — DROPS DEFAULT OFF (covenant №31). The first live cut spawned a
    // TP-model rupee inside the WW space — a foreign prop, and №31 says a
    // MISSING prop is preferable to a foreign one. Drops stay disabled until
    // the WW-visual drop path is wired (the mod's own rupee design: vlupy.bdl
    // visual + TP wallet grant — see the ledger's `rupees` row). `item_table=`
    // in the pack re-enables deliberately; the default must not violate law.
    // ========================================================================
    int itemTable = -1;
    bool ready = false;
    bool tried = false;
};

ExtVegAssets s_assets;


// §45/№147: one cut sound per FRAME across every clump. A single sword swing
// crosses several blades, and without this each one would start the sample.
// The donor guards the same way with its l_CutSoundFlag.
bool s_cutSoundThisFrame = false;

// §215 post-cut emission watch (strip with the other probes).
int s_emitWatchFrames = 0;
int s_emitWatchBase = 0;

// №135: the blade texture must be bound EXPLICITLY. The material DL carries a
// texture pointer, but it is a GameCube address baked into donor data and means
// nothing here — relying on it left every blade untextured, i.e. WHITE, which
// is what shipped and read on screen as pale angular shapes on the ground.
// The native grass does the same thing the same way (GXInitTexObj once,
// GXLoadTexObj before drawing); this mirrors it.
//
// Format: the blob is 2048 bytes for a 64x64 texture = 4 bits/texel, and the
// blade needs alpha for its silhouette, which makes it CMPR (4bpp, 1-bit
// alpha). The native equivalent is RGB5A3 only because its texture is 32x32
// (32*32*2 = 2048) — same byte count, different dimensions, so the size alone
// does not decide it. If blades render as a grey/─noise mess rather than
// foliage, the alternative at 4bpp is GX_TF_I4.
GXTexObj s_texObj;
bool s_texReady = false;

// №138: the donor blade is replayed as a display list against INDEXED arrays.
// Every piece of that was verified correct in isolation — the DL parses as 9
// GX_TRIANGLESTRIPs of 4 verts at vtxfmt 0, three INDEX8 attributes per vertex,
// every index inside its array, arrays big-endian and sane, stride 12/4/8. The
// composite still drew stretched black geometry, so the fault lies in replaying
// donor display lists through this port's GX layer — the one part that cannot
// be inspected from either side.
//
// So stop replaying it. The DL is a fixed, tiny, fully-parsed structure: decode
// it ONCE into explicit vertices and emit them directly. Same geometry, same
// triangle order, no indexed-array binding and no donor bytecode in the hot
// path — and every value becomes inspectable from our own code.
struct Strip {
    int count;
    f32 px[8], py[8], pz[8];
    u8 cr[8], cg[8], cb[8], ca[8];
    f32 ts[8], tt[8];
};
std::vector<Strip> s_strips;
// ============================================================================
// №227 — the donor's CUT display list (`l_Oba_kusa_a_cutDL`), decoded to
// strips exactly like the full blade. The donor draws every grass unit from
// one of TWO DLs — full blade while standing, the stub after a cut (draw:
// `mAnimIdx >= 0 ? mpDL : mpDLCut`, d_grass.cpp) — and the stub persists
// until reload. The extractor shipped this DL in the pack from day one
// (`blade_cut_dl=`); it was loaded but never decoded or drawn.
// ============================================================================
std::vector<Strip> s_cutStrips;

f32 beF32(const std::vector<u8>& v, size_t off) {
    if (off + 4 > v.size()) {
        return 0.0f;
    }
    const u32 u = ((u32)v[off] << 24) | ((u32)v[off + 1] << 16) | ((u32)v[off + 2] << 8) |
                  (u32)v[off + 3];
    f32 f;
    std::memcpy(&f, &u, 4);
    return f;
}

// №227: shared for the full blade AND the cut stub — both DLs index the SAME
// pos/color/texCoord arrays (the donor binds one array set for both draws).
void decodeDLInto(const std::vector<u8>& dl, size_t limit, const std::vector<u8>& pos,
                  const std::vector<u8>& color, const std::vector<u8>& texCoord,
                  std::vector<Strip>& out) {
    out.clear();
    size_t i = 0;
    while (i + 3 <= dl.size() && i < limit) {
        const u8 op = dl[i];
        if (op == 0x00) {
            ++i;
            continue;
        }
        if ((op & 0xF8) != 0x98) {  // only GX_TRIANGLESTRIP appears in this DL
            ++i;
            continue;
        }
        const int cnt = (dl[i + 1] << 8) | dl[i + 2];
        if (cnt <= 0 || cnt > 8 || i + 3 + cnt * 3 > dl.size()) {
            break;
        }
        Strip st{};
        st.count = cnt;
        for (int v = 0; v < cnt; ++v) {
            const u8 pi = dl[i + 3 + v * 3 + 0];
            const u8 ci = dl[i + 3 + v * 3 + 1];
            const u8 ti = dl[i + 3 + v * 3 + 2];
            st.px[v] = beF32(pos, pi * 12 + 0);
            st.py[v] = beF32(pos, pi * 12 + 4);
            st.pz[v] = beF32(pos, pi * 12 + 8);
            const size_t co = ci * 4;
            st.cr[v] = co + 3 < color.size() ? color[co + 0] : 255;
            st.cg[v] = co + 3 < color.size() ? color[co + 1] : 255;
            st.cb[v] = co + 3 < color.size() ? color[co + 2] : 255;
            st.ca[v] = co + 3 < color.size() ? color[co + 3] : 255;
            st.ts[v] = beF32(texCoord, ti * 8 + 0);
            st.tt[v] = beF32(texCoord, ti * 8 + 4);
        }
        out.push_back(st);
        i += 3 + cnt * 3;
    }
}

void decodeBladeDL() {
    decodeDLInto(s_assets.bladeDL, 0xA0, s_assets.pos, s_assets.color,
                 s_assets.texCoord, s_strips);
    // №227: the stub geometry — donor draws it from frame-of-cut onward.
    decodeDLInto(s_assets.bladeCutDL, 0x80, s_assets.pos, s_assets.color,
                 s_assets.texCoord, s_cutStrips);
}

// ==========================================================================
// FLOWERS (donor d_flower.cpp) -- the second native vegetation kind.
// ==========================================================================
// Ported after the JPA particle port landed (bus 241): flowers were the
// remaining MOUNTED vegetation, drawn as discrete NPC_YAFLW J3D models and
// currently invisible (liberty L-6). This replaces the mount with the donor's
// own raw-GX packet geometry, exactly as grass was done.
//
// The donor draws flowers in TWO TIERS with separate arrays, material DL and
// texture, selected by the census kind (d_a_grass.cpp:176-181, then
// d_flower.cpp setData:496 and draw:318-386):
//
//   census kind 2 -> flowerType 1 -> bit 0x20 CLEAR -> tier WHITE
//                    l_pos / l_color / l_texCoord, l_matDL,
//                    l_OhanaDL (0x100) / l_Ohana_gutDL (0xA0),
//                    l_Txo_ob_flower_white_64x64TEX
//   census kind 3 -> flowerType 2 -> bit 0x20 SET   -> tier PINK
//                    l_pos2 / l_color2 / l_texCoord2, l_matDL2,
//                    l_Ohana_highDL (0x120) / l_Ohana_high_gutDL (0x80),
//                    l_Txo_ob_flower_pink_64x64TEX
//
// (The kind -> flowerType indirection is easy to read backwards: setData tests
// `param_3 == 2`, which is flowerTYPE 2, i.e. census kind THREE = pink. L-6's
// note is right; the intermediate step is what makes it look inverted.)
//
// The third tier (l_QbsfwDL / bessou) is stage "sea" room 0x21 only and has no
// Outset placement, so it is not wired -- its blobs are staged but unused.
// ==========================================================================
struct FlowerTier {
    std::vector<u8> pos;
    std::vector<u8> color;
    std::vector<u8> texCoord;
    std::vector<u8> matDL;
    std::vector<u8> dl;
    std::vector<u8> cutDL;
    std::vector<u8> tex;
    std::vector<Strip> strips;
    std::vector<Strip> cutStrips;
    GXTexObj texObj;
    bool texReady = false;
    u32 dlSize = 0;
    u32 cutDLSize = 0;
};

// Index by tier id: 0 = white (kind 2), 1 = pink (kind 3).
FlowerTier s_flower[2];
bool s_flowerReady = false;

// Donor call sizes, verbatim from d_flower.cpp::draw -- NOT the raw symbol
// sizes, which are unaligned (266/176/290/140) and would fail
// GXCallDisplayList's 32-byte requirement. Same discipline as the blade DLs.
const u32 kFlowerMatDLSize = 0xA0;

// The donor calls these display lists with 32-byte-aligned sizes, NOT the raw
// symbol sizes from the map (0xA7/0x8C/0xA8). GXCallDisplayList requires the
// alignment, so the donor's own runtime constants are used verbatim.
const u32 kMatDLSize = 0xA0;
const u32 kBladeDLSize = 0xA0;
const u32 kBladeCutDLSize = 0x80;

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

// §45: blob filenames come from assets/veg/veg_manifest.ini [pack] — never
// hardcoded. Missing key ⇒ refuse (no silent fallback).
bool readPackKeys(const std::filesystem::path& mani, std::string& pos, std::string& color,
                  std::string& texCoord, std::string& matDl, std::string& bladeDl,
                  std::string& bladeCutDl, std::string& bladeTex) {
    std::ifstream in(mani);
    if (!in) {
        return false;
    }
    bool inPack = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            inPack = (line.find("[pack]") != std::string::npos);
            continue;
        }
        if (!inPack) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        while (!val.empty() && (val.back() == '\r' || val.back() == ' ')) {
            val.pop_back();
        }
        if (key == "pos") {
            pos = val;
        } else if (key == "color") {
            color = val;
        } else if (key == "tex_coord") {
            texCoord = val;
        } else if (key == "mat_dl") {
            matDl = val;
        } else if (key == "blade_dl") {
            bladeDl = val;
        } else if (key == "blade_cut_dl") {
            bladeCutDl = val;
        } else if (key == "blade_tex") {
            bladeTex = val;
        } else if (key == "item_table") {
            // №220: cut-reward table override (-1 disables drops)
            s_assets.itemTable = atoi(val.c_str());
        }
    }
    return !pos.empty() && !color.empty() && !texCoord.empty() && !matDl.empty() &&
           !bladeDl.empty() && !bladeCutDl.empty() && !bladeTex.empty();
}

// ==========================================================================
// Flower pack -- same 45 discipline as the blade pack: every blob is named in
// the manifest, nothing is hardcoded, and an incomplete pack REFUSES (flowers
// stay inert) rather than half-loading.
// ==========================================================================
bool readFlowerKeys(const std::filesystem::path& mani, std::string* out) {
    // Order matches kFlowerKeys below.
    std::ifstream in(mani);
    if (!in) {
        return false;
    }
    static const char* const kFlowerKeys[14] = {
        "w_pos",    "w_color", "w_tex_coord", "w_mat_dl", "w_dl", "w_cut_dl", "w_tex",
        "p_pos",    "p_color", "p_tex_coord", "p_mat_dl", "p_dl", "p_cut_dl", "p_tex",
    };
    std::string line;
    bool inSection = false;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            inSection = (line == "[flower_pack]");
            continue;
        }
        if (!inSection) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        for (int k = 0; k < 14; ++k) {
            if (key == kFlowerKeys[k]) {
                out[k] = val;
            }
        }
    }
    for (int k = 0; k < 14; ++k) {
        if (out[k].empty()) {
            return false;
        }
    }
    return true;
}

bool loadFlowerPack(const std::filesystem::path& veg, const std::filesystem::path& mani) {
    std::string k[14];
    if (!readFlowerKeys(mani, k)) {
        DuskLog.warn("[ExtVeg] flower pack: [flower_pack] incomplete -- flowers inert");
        return false;
    }
    for (int t = 0; t < 2; ++t) {
        FlowerTier& f = s_flower[t];
        const int b = t * 7;
        if (!(readBlob(veg / k[b + 0], f.pos) && readBlob(veg / k[b + 1], f.color) &&
              readBlob(veg / k[b + 2], f.texCoord) && readBlob(veg / k[b + 3], f.matDL) &&
              readBlob(veg / k[b + 4], f.dl) && readBlob(veg / k[b + 5], f.cutDL) &&
              readBlob(veg / k[b + 6], f.tex))) {
            DuskLog.warn("[ExtVeg] flower pack: tier {} blob missing -- flowers inert", t);
            return false;
        }
        // Donor call sizes (d_flower.cpp::draw / setData): white 0x100/0xA0,
        // pink 0x120/0x80.
        f.dlSize = (t == 0) ? 0x100 : 0x120;
        f.cutDLSize = (t == 0) ? 0xA0 : 0x80;
        decodeDLInto(f.dl, f.dlSize, f.pos, f.color, f.texCoord, f.strips);
        decodeDLInto(f.cutDL, f.cutDLSize, f.pos, f.color, f.texCoord, f.cutStrips);
        // Both flower textures are 64x64 at 2048 bytes = CMPR, same as the
        // blade texture. 135 applies here too: the material DL's texture
        // pointer is a baked GameCube address and must not be trusted.
        // ====================================================================
        // WRAP MODES COME FROM THE DONOR'S OWN MATERIAL DL, NOT FROM A GUESS.
        //
        // Both flower material DLs write TX_SETMODE0_I0 with wrap_s = wrap_t =
        // GX_MIRROR (decoded straight out of d_flower__l_matDL*.bin). That is
        // load-bearing here, because the flower UVs run WELL outside [0,1]:
        // white u[0.065,2.004] v[-1.927,0.551], pink u[0,2.917] v[-1.917,0.917].
        // The first attempt reused the blade call's (GX_REPEAT, GX_CLAMP); with
        // T clamped, every negative-v vertex collapses onto one texture row, so
        // each polygon drew as a flat slab of whatever colour sat on that row --
        // the big green triangles, with the petal silhouette gone because the
        // alpha cutout lives in the part of the atlas that was never sampled.
        //
        // (Filters: the donor also asks for mag=GX_LINEAR, min=NEAR_MIP_NEAR.
        // There is exactly one mip level in the blob, so GXInitTexObj's
        // no-mipmap default is equivalent for min; mag already matches.)
        // ====================================================================
        GXInitTexObj(&f.texObj, f.tex.data(), 64, 64, GX_TF_CMPR, GX_MIRROR, GX_MIRROR,
                     GX_FALSE);
        f.texReady = true;
    }
    DuskLog.info("[ExtVeg] flower pack loaded: white strips={}/{} pink strips={}/{}",
                 s_flower[0].strips.size(), s_flower[0].cutStrips.size(),
                 s_flower[1].strips.size(), s_flower[1].cutStrips.size());
    return true;
}

bool loadAssets() {
    if (s_assets.tried) {
        return s_assets.ready;
    }
    s_assets.tried = true;

    const std::filesystem::path root =
        std::filesystem::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::is_directory(root, ec)) {
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (!entry.is_directory()) {
            continue;
        }
        const std::filesystem::path veg = entry.path() / "assets" / "veg";
        const std::filesystem::path mani = veg / "veg_manifest.ini";
        if (!std::filesystem::is_directory(veg, ec)) {
            continue;
        }
        std::string pos, color, texCoord, matDl, bladeDl, bladeCutDl, bladeTex;
        if (!readPackKeys(mani, pos, color, texCoord, matDl, bladeDl, bladeCutDl, bladeTex)) {
            DuskLog.warn("[ExtVeg] §45 refuse — '{}' missing [pack] keys",
                         entry.path().filename().string());
            continue;
        }
        const bool ok = readBlob(veg / pos, s_assets.pos) &&
                        readBlob(veg / color, s_assets.color) &&
                        readBlob(veg / texCoord, s_assets.texCoord) &&
                        readBlob(veg / matDl, s_assets.matDL) &&
                        readBlob(veg / bladeDl, s_assets.bladeDL) &&
                        readBlob(veg / bladeCutDl, s_assets.bladeCutDL) &&
                        readBlob(veg / bladeTex, s_assets.tex);
        if (ok) {
            // Same correction as the flower pack: d_grass__l_matDL.bin writes
            // TX_SETMODE0_I0 with wrap_s = wrap_t = GX_MIRROR. The blade UVs are
            // u[0,2] v[0,1], so only S is affected -- but MIRROR reverses the
            // second half of the sweep where REPEAT tiles it, which is a real
            // difference on the blade, and the donor's value is the spec.
            GXInitTexObj(&s_texObj, s_assets.tex.data(), 64, 64, GX_TF_CMPR, GX_MIRROR,
                         GX_MIRROR, GX_FALSE);
            s_texReady = true;
            s_assets.ready = true;
            decodeBladeDL();
            s_flowerReady = loadFlowerPack(veg, mani);
            DuskLog.info(
                "[ExtVeg] №122 asset pack loaded from '{}' (pos={} color={} tex={} bytes)",
                entry.path().filename().string(), s_assets.pos.size(), s_assets.color.size(),
                s_assets.tex.size());
            return true;
        }
        DuskLog.warn("[ExtVeg] №122 incomplete pack under '{}' — grass disabled",
                     entry.path().filename().string());
    }
    return false;
}

// ==========================================================================
// Instance registry — every clump expands into N blades at Create time, the
// same way the donor's actor seeds its packet.
// ==========================================================================
// №136: NO shared blade list. The first cut kept one global vector plus a
// "draw owner" instance that rendered everyone's blades. That coupled every
// clump's lifetime to one actor's: the list was only cleared when the OWNER
// died, so each population pass appended another ~86 clumps that the draw loop
// then walked EVERY FRAME — a per-frame loop and a memory footprint that both
// grew with every room re-entry. It also meant one culled actor could blank all
// the grass at once.
//
// Each actor now owns and draws only its own blades. GX setup is paid per clump
// instead of once, which at this population is affordable (the native packet
// only centralises because it handles thousands), and in exchange the whole
// class of shared-state lifetime bugs disappears.
const int kMaxBlades = 21;  // largest donor clump layout

// Donor clump layouts, transcribed from its own actor. Index = param type.
const csXyz l_setType0[] = {
    csXyz(0, 0, 0),    csXyz(3, 0, -50),  csXyz(-2, 0, 50),  csXyz(50, 0, 27),
    csXyz(52, 0, -25), csXyz(-50, 0, 22), csXyz(-50, 0, -29),
};
const csXyz l_setType1[] = {
    csXyz(-18, 0, 76),   csXyz(-15, 0, 26),  csXyz(133, 0, 0),   csXyz(80, 0, 23),
    csXyz(86, 0, -83),   csXyz(33, 0, -56),  csXyz(83, 0, -27),  csXyz(-120, 0, -26),
    csXyz(-18, 0, -74),  csXyz(-20, 0, -21), csXyz(-73, 0, 1),   csXyz(-67, 0, -102),
    csXyz(-21, 0, 126),  csXyz(-120, 0, -78), csXyz(-70, 0, -49), csXyz(32, 0, 103),
    csXyz(34, 0, 51),    csXyz(-72, 0, 98),  csXyz(-68, 0, 47),  csXyz(33, 0, -5),
    csXyz(135, 0, -53),
};
const csXyz l_setType2[] = {
    csXyz(-75, 0, -50), csXyz(75, 0, -25), csXyz(14, 0, 106),
};

// ==========================================================================
// THE ANIM-SLOT POOL -- donor dGrass_packet_c::mGrassAnm[104] and
// dFlower_packet_c::mAnm[72], ported rather than approximated.
// ==========================================================================
// This is the donor's real sway/lean state machine, and it is LOAD-BEARING for
// more than the sway: the run-through VFX rate limit IS this pool. The donor
// never uses a timer for grass -- `WorkCo` fires the puff only on the frame a
// blade transitions from "no push slot" to "has push slot", so one approach
// yields one puff, and a player standing still in the grass gets none. A
// frame-count cooldown reproduces the look and not the rule; this is the rule.
//
// Layout (both donors identical in shape):
//   slots 0..7    AMBIENT. rotY = i * 0x2000 fixed; rotX driven per frame from
//                 wind (grass) or a sine (flowers). Every plant starts on one,
//                 picked as cM_rndF(7.0f) -- d_grass.cpp:428, d_flower.cpp:501.
//   slots 8..N    PUSH. Allocated by newAnm when the player closes, released
//                 when the plant has eased all the way back.
//
// The return path is the clever part and is transcribed exactly
// (d_grass.cpp:174-190). A push slot's rotY holds the bearing to the player;
// its TOP THREE BITS therefore also name an ambient slot. So the donor recovers
// the plant's home slot from the lean direction itself -- `(rotY >> 13) & 7` --
// and eases rotX back toward that slot's current wind value before chasing rotY
// to `rotY & 0xE000`. When the chase lands, the push slot is freed and the
// plant adopts the ambient slot encoded in its own final angle. No "original
// slot" is ever stored.
//
// The sizes are the donor's own (96 push slots for grass, 64 for flowers): a
// full pool is a real state the donor handles by declining to lean, and
// shrinking it would change behaviour under crowding.
//
// This IS shared state across clumps, which No.136 warned about -- but No.136's
// problem was a shared BLADE LIST whose lifetime was tied to one actor's, so it
// grew every room re-entry. This is a fixed-size array of transient slots with
// explicit alloc/free and no per-actor ownership, which is exactly the donor's
// own arrangement.
// ==========================================================================
struct VegAnm {
    u8 state;  // 0 = free, 1 = idle/returning, 2 = held by the player this frame
    s16 rotY;
    s16 rotX;
};

const int kGrassAmbient = 8;
const int kGrassAnmNum = 104;  // donor mGrassAnm[104]
const int kFlowerAnmNum = 72;  // donor mAnm[72]

VegAnm s_grassAnm[kGrassAnmNum];
VegAnm s_flowerAnm[kFlowerAnmNum];
bool s_anmInit = false;
u32 s_anmFrame = 0xFFFFFFFF;

const u8 kAnmNone = 0xFF;  // donor mAnimIdx = -1 (cut plant: no anim at all)

void vegAnmInit() {
    if (s_anmInit) {
        return;
    }
    s_anmInit = true;
    // Donor ctors: setAnm(i, angle) with angle stepping 0x2000, for i in 0..7.
    for (int i = 0; i < kGrassAmbient; ++i) {
        s_grassAnm[i].state = 1;
        s_grassAnm[i].rotY = (s16)(i * 0x2000);
        s_grassAnm[i].rotX = 0;
        s_flowerAnm[i].state = 1;
        s_flowerAnm[i].rotY = (s16)(i * 0x2000);
        s_flowerAnm[i].rotX = 0;
    }
}

// ==========================================================================
// 244 PROBE -- why does the run-through puff fire less than expected?
// ==========================================================================
// Accumulated per frame and flushed from vegAnmTickAmbient (which already
// detects the frame boundary), so ONE line covers every clump instead of one
// line per actor. Reads as a funnel: examined -> CO -> in range -> eligible
// (still on an ambient slot) -> fast enough -> spawned. Whichever step drops to
// zero is the gate. `gFree`/`fFree` catch pool exhaustion, which is the
// hypothesis this was built for and is invisible from the funnel alone.
// Set DUSK_VEG_PROBE=0 to silence.
// ==========================================================================
struct VegProbe {
    // [0] = grass, [1] = flower. The first cut of this funnel instrumented only
    // the grass path, so a question about FLOWERS was answered with grass data.
    int examined[2], co[2], at[2], inRange[2], eligible[2], fast[2], spawned[2];
    int returned[2], poolFull[2];
    // Who is actually in the plant, and how fast. 244 showed the speedF > 16
    // gate rejecting 77% of eligible encounters, which is only meaningful if
    // the actor being measured is the player -- the donor drives WorkCo from
    // whatever has CO, and a static mass object has speedF 0.
    f32 spdMax[2];      // fastest hitActor seen at an eligible encounter
    int nonPlayer[2];   // eligible encounters whose hitActor was NOT the player
    int lastName[2];    // fopAcM_GetName of the most recent rejected actor
};
VegProbe s_probe;

int vegAnmFree(const VegAnm* pool, int poolNum) {
    int n = 0;
    for (int i = kGrassAmbient; i < poolNum; ++i) {
        if (pool[i].state == 0) {
            ++n;
        }
    }
    return n;
}

// ==========================================================================
// SLOT RELEASE ON TEARDOWN -- required by OUR ownership model, not the donor's.
// ==========================================================================
// The donor's dGrass_room_c::deleteData frees the plant DATA and leaves any
// anim slot those plants held; the packet ctor resets the whole pool on stage
// load, so its leak is bounded by stage lifetime and rarely reached.
//
// Here every clump is its own actor, created and destroyed by the population
// spawner and by culling, many times per visit. A blade that is still leaning
// when its clump goes away would strand its push slot forever -- and once the
// 96 push slots are gone, NO plant anywhere can take one, which means no lean
// and (because the puff fires on slot acquisition) no run-through VFX at all.
// Progressive, silent, and exactly the "appears less often" symptom.
//
// Releasing on teardown restores the donor's invariant -- a slot is held only
// by a live plant -- rather than changing the rule.
// ==========================================================================
void vegAnmReleaseAll(VegAnm* pool, int poolNum, u8* idx, int num) {
    for (int i = 0; i < num; ++i) {
        if (idx[i] != kAnmNone && idx[i] >= kGrassAmbient && idx[i] < poolNum) {
            pool[idx[i]].state = 0;
        }
        idx[i] = kAnmNone;
    }
}

// Donor dGrass_packet_c::calc:322-337 and dFlower_packet_c::calc:394-398.
// Runs ONCE per frame for the whole world, not once per clump -- the ambient
// slots are shared, so driving them per actor would advance them N times.
void vegAnmTickAmbient() {
    vegAnmInit();
    if (s_anmFrame == (u32)g_Counter.mTimer) {
        return;
    }
    s_anmFrame = (u32)g_Counter.mTimer;

    {
        static const bool s_probeOn = [] {
            const char* v = std::getenv("DUSK_VEG_PROBE");
            return v == NULL || v[0] != '0';
        }();
        if (s_probeOn) {
            static const char* const kName[2] = {"grass ", "flower"};
            for (int k = 0; k < 2; ++k) {
                if (s_probe.examined[k] == 0 ||
                    (s_probe.co[k] == 0 && s_probe.returned[k] == 0)) {
                    continue;
                }
                DuskLog.warn("[ExtVeg] 246 {} exam={} co={} at={} rng={} elig={} fast={} "
                             "spawn={} ret={} poolFull={} free={}/{} spdMax={:.1f} "
                             "nonPlayer={} lastName={}",
                             kName[k], s_probe.examined[k], s_probe.co[k], s_probe.at[k],
                             s_probe.inRange[k], s_probe.eligible[k], s_probe.fast[k],
                             s_probe.spawned[k], s_probe.returned[k], s_probe.poolFull[k],
                             k == 0 ? vegAnmFree(s_grassAnm, kGrassAnmNum)
                                    : vegAnmFree(s_flowerAnm, kFlowerAnmNum),
                             (k == 0 ? kGrassAnmNum : kFlowerAnmNum) - kGrassAmbient,
                             s_probe.spdMax[k], s_probe.nonPlayer[k], s_probe.lastName[k]);
            }
        }
        s_probe = VegProbe();
    }

    f32 windSpeed = dKyw_get_wind_pow() * 1000.0f + 1000.0f;
    windSpeed = cLib_maxLimit(windSpeed, 2000.0f);
    for (int i = 0; i < kGrassAmbient; ++i) {
        s_grassAnm[i].rotX =
            (s16)(windSpeed +
                  windSpeed * cM_scos((s16)(windSpeed * (f32)(g_Counter.mTimer + i * 250))));
        s_flowerAnm[i].rotX =
            (s16)(cM_scos((s16)((g_Counter.mTimer + i * 0xFA) * 1000.0f)) * 1000.0f + 1000.0f);
    }
}

// Donor newAnm: first free PUSH slot, or -1 when the pool is full.
u8 vegAnmNew(VegAnm* pool, int poolNum) {
    for (int i = kGrassAmbient; i < poolNum; ++i) {
        if (pool[i].state == 0) {
            pool[i].state = 1;
            pool[i].rotY = 0;
            pool[i].rotX = 0;
            return (u8)i;
        }
    }
    return kAnmNone;
}

// ==========================================================================
// 246 -- GRASS SPAWNS THROUGH THE SIMPLE EMITTER, AS THE DONOR DOES.
// ==========================================================================
// The donor splits its vegetation spawns by PLANT, not by event:
//   grass  run d_grass.cpp:80 / cut :154  -> dComIfGp_particle_setSimple
//   flower run d_flower.cpp:91 / cut :218 -> dComIfGp_particle_set
// So the flower path was already right; only GRASS was substituted.
//
// The two are not interchangeable. particle_set mints a NEW emitter per call
// that then lives out its own maxFrame/lifeTime. setSimple never spawns:
// dPa_simpleEcallBack holds ONE persistent emitter per id, built with
// setMaxFrame(0) + stopCreateParticle() so it never self-emits, and each call
// merely APPENDS a position record. Once per frame executeAfter takes
// getCurrentCreateNumber() from the resource's own rate and creates that many
// particles at EVERY recorded position in one batch, clipping records past 200
// units. N blades triggering in a frame = one batched burst of N x rate. The
// receiver's executeAfter is line-for-line the donor's.
//
// Why this was abandoned before, and why it is right to retry. 228 recorded
// that the simple path "registers successfully but draws NOTHING". That verdict
// was reached in a build where the WW resource could not draw AT ALL -- 241's
// missing tex-coord-matrix table meant every WW particle rendered as heap
// garbage or nothing. The simple path was convicted on evidence that indicted
// the whole lane.
//
// Registration order is load-bearing (221): newSimple builds its emitter
// immediately, so the WW bank must be resident first.
//
// There is deliberately NO particle_set fallback. A fallback would hide a
// registration failure behind a working-looking effect, which is how 228
// reached the wrong conclusion. If this fails it fails loudly.
// ==========================================================================
bool extVegEnsureSimple(u16 i_resID) {
    struct SimpleReg {
        u16 id;
        u8 state;  // 0 = untried, 1 = registered, 2 = gave up
        u8 tries;
    };
    static SimpleReg s_reg[] = {{0x03DA, 0, 0}, {0x03DB, 0, 0}};
    SimpleReg* e = NULL;
    for (int i = 0; i < (int)(sizeof(s_reg) / sizeof(s_reg[0])); ++i) {
        if (s_reg[i].id == i_resID) {
            e = &s_reg[i];
        }
    }
    if (e == NULL || e->state == 2) {
        return false;
    }
    dPa_control_c* pa = g_dComIfG_gameInfo.play.getParticle();
    if (pa == NULL) {
        return false;
    }
    if (e->state != 1) {
        // 221(b): do not latch failure on the first miss -- the bank may still
        // be loading. Bounded retries, then give up loudly.
        const bool resOk = pa->ensureWwCommonRes(i_resID);
        u32 handle = 0;
        if (resOk && pa->newSimple(i_resID, 0, &handle) && pa->getSimple(i_resID) != NULL) {
            e->state = 1;
            DuskLog.info("[ExtVeg] 246 simple emitter registered for {:#06x}", (unsigned)i_resID);
        } else if (++e->tries >= 8) {
            e->state = 2;
            DuskLog.error("[ExtVeg] 246 newSimple({:#06x}) FAILED res={} -- grass VFX silent. "
                          "This is a registration bug, not a design choice.",
                          (unsigned)i_resID, resOk ? 1 : 0);
            return false;
        } else {
            return false;
        }
    }
    return pa->getSimple(i_resID) != NULL;
}

// ==========================================================================
// THE SPAWN COLOUR MODULATOR -- one helper, because every WW vegetation VFX
// needs it and sourcing it wrongly fails SILENTLY (black particles).
// ==========================================================================
// 208 established the mechanism: dPa_control_c::set's no-userWork branch writes
// the prm/env arguments straight into the emitter's GLOBAL colours, and the
// final particle colour is resource x global (COLOR_MULTI). So this argument is
// a MULTIPLIER over the donor's authored colour, not a decorative tint -- feed
// it something dark and the VFX goes black however good the resource is.
//
// The donor feeds it the room tevstr's mColorK0 (d_grass.cpp:80/153,
// d_flower.cpp:91/218). On a MOUNTED host stage the room colour table is never
// filled, so the live WW value comes from the No.113 stash instead:
// dungeonlight_col[1] = BG0_K0, the same slot W-LINE-c reads at
// d_kankyo_wether.cpp:1559.
//
// It is NOT AmbCol. AmbCol is the ambient and runs near-black (36,24,59 at
// hour 11). 182 found that once already for the grass cut scatter; the flower
// run-through VFX rediscovered it by drawing black, because that spawn was
// written against AmbCol instead of reusing this. Hence the helper.
void extVegSpawnTev(const cXyz& i_pos, dKy_tevstr_c* o_tev, GXColor* o_k0) {
    cXyz pos = i_pos;
    g_env_light.settingTevStruct(0x40, &pos, o_tev);
    const GXColorS10& bg0K0 = g_env_light.dungeonlight_col[1];
    auto clamp8 = [](s16 v) -> u8 {
        if (v < 0) {
            return 0;
        }
        if (v > 255) {
            return 255;
        }
        return (u8)v;
    };
    o_k0->r = clamp8(bg0K0.r);
    o_k0->g = clamp8(bg0K0.g);
    o_k0->b = clamp8(bg0K0.b);
    o_k0->a = 255;
    // No.143: settingTevStruct leaves C0/K0 at zero on non-J3D paths, so fill
    // them explicitly -- the donor reads them off a populated room tevstr.
    o_tev->TevColor.r = o_k0->r;
    o_tev->TevColor.g = o_k0->g;
    o_tev->TevColor.b = o_k0->b;
    o_tev->TevColor.a = 255;
    o_tev->TevKColor.r = o_k0->r;
    o_tev->TevKColor.g = o_k0->g;
    o_tev->TevKColor.b = o_k0->b;
    o_tev->TevKColor.a = 255;
}

struct OffsetData {
    u8 num;
    const csXyz* pos;
};

const OffsetData l_offsetData[] = {
    {1, l_setType0},  {7, l_setType0},  {21, l_setType1}, {3, l_setType2},
    {7, l_setType0},  {17, l_setType1}, {7, l_setType0},  {5, l_setType0},
};

}  // namespace

// ==========================================================================
// Actor
// ==========================================================================
class daExtVeg_c;

// §45/№140: draw through a J3D DRAW-BUFFER PACKET, not inline in the actor's
// Draw callback. This was the whole bug behind "grass is invisible".
//
// Everything else had already been verified correct, individually: vertex data
// (big-endian, blade-scale), colours (no blacks), texcoords, stride, the
// decoded display list (9 strips x 4 verts, indices in range), the texture
// bind, and the model-view concat. The collision probe then proved the actors
// sit at EXACTLY their census coordinates and can be walked into.
//
// What was wrong was WHEN the draw ran. An actor Draw callback executes outside
// the J3D draw phase, where j3dSys.getViewMtx() is not the current view.
// Concatenating against that stale (identity) view placed every blade at world
// coordinates interpreted as CAMERA-RELATIVE - which is exactly why the shapes
// appeared symmetrically around the player instead of on the hillside, and why
// they tracked every geometry edit while never moving to where they belonged.
//
// Both available references do it this way: the donor's own grass packet ends
// with getDrawBuffer(0)->entryImm(this, 0), and the in-tree raw-GX precedent
// d_a_obj_flag3.cpp registers its packet the same way before using the very
// same concat(getViewMtx(), ...) call that failed here.
class ExtVegPacket_c : public J3DPacket {
public:
    ExtVegPacket_c() : mOwner(NULL) {}
    virtual void draw();
    daExtVeg_c* mOwner;
};

class daExtVeg_c : public fopAc_ac_c {
public:
    int create();
    int execute() {
        if (isFlower()) {
            checkFlowers();
        } else {
            checkCut();
        }
        return 1;
    }
    void checkCut();
    int draw();
    int deleteMe();

    cXyz mBlades[kMaxBlades];  // WORLD positions; model-view is built per frame
    int mBladeCount;
    s8 mRoomNo;
    // §45/№147: per-blade cut state. A cut blade draws the STUMP display list
    // (l_Oba_kusa_a_cutDL, already in the extracted pack) instead of the full
    // blade — the donor expresses the same thing by setting its anim index to
    // -1 and selecting mpDLCut in draw.
    bool mCut[kMaxBlades];
    ExtVegPacket_c mPacket;
    void drawBlades();  // real GX work; runs from the packet, in the draw phase

    // ========================================================================
    // Flowers. mKind is the census kind straight from the actor parameter:
    // 0 = grass (blades), 2 = white flower, 3 = pink flower. The donor runs all
    // three through one census dispatcher too (d_a_grass.cpp), so this actor
    // keeping all three is donor-shaped rather than a convenience.
    // ========================================================================
    u8 mKind;
    u8 mTier;         // 0 = white, 1 = pink; only meaningful when mKind != 0
    u8 mAnmIdx[kMaxBlades];    // donor field_0x01: which ambient sway slot
    u8 mRunCool[kMaxBlades];   // donor field_0x03: run-through VFX cooldown
    bool isFlower() const { return mKind == 2 || mKind == 3; }
    // Donor d_flower.cpp: bit 0x20 clear (white) -> ID_IT_JN_FLOWER_W 0x03DE,
    // set (pink) -> ID_IT_JN_FLOWER_P 0x03DD. Both the cut VFX (WorkAt:207-212)
    // and the run-through VFX (WorkCo:80-88) use the same id per tier.
    u16 flowerParticleId() const { return mTier == 0 ? 0x03DE : 0x03DD; }
    void drawFlowers();
    void checkFlowers();
};


int daExtVeg_c::create() {
    fopAcM_ct(this, daExtVeg_c);

    // execute() and the packet draw both dispatch on mKind, and every early
    // return below leaves create() before the parameter is read. Seed the
    // dispatch state FIRST so an inert actor is inert as grass-with-no-blades
    // rather than as whatever the allocation happened to contain.
    mKind = 0;
    mTier = 0;
    mBladeCount = 0;

    // ========================================================================
    // №229 — grass-cut scatter (0x89D7) is a SCENE particle our host stage's
    // own Pscene bank doesn't carry (§62: emitter=0, tevstr fine). The mod
    // already owns the answer: the death-orb work loads Pscene011.jpc as a
    // SUPPLEMENTAL archive (slot 2) with a GENERAL fallback in
    // dPa_tearResFallbackRM — and the offline bank scan confirms Pscene011
    // contains 0x89D6+0x89D7. Kick the (idempotent, async) load at create so
    // the resources are resident by the first cut.
    // ========================================================================
    if (dPa_control_c* pa = g_dComIfG_gameInfo.play.getParticle()) {
        pa->ensureTearSceneRes();
    }

    // №129: CONSUME this actor's pending-spawn entry. The population spawner
    // pushes one BEFORE fopAcM_create for every census row, and every other
    // census-spawnable actor (henna0/kdk/mk/p2/knob00) takes it during create.
    // An actor that does not consume leaves a STALE entry, and every LATER
    // actor then pulls the wrong head/identity — bodies swapped while dialogue
    // (manifest-driven, not FIFO-driven) stays correct. That is the reported
    // identity swap. Take and discard: this actor has no head to pin.
    {
        char proc[32] = {};
        char src[96] = {};
        char head[64] = {};
        char joint[32] = {};
        dExtNpcMount_takePendingSpawn(fopAcM_GetID(this), proc, sizeof(proc), src, sizeof(src),
                                      head, sizeof(head), joint, sizeof(joint));
    }


    // NEVER return cPhs_ERROR_e from create. fopAcM_create has already handed
    // the population spawner a valid id by this point, so the spawner will not
    // discard its pending-spawn FIFO entry — and an unconsumed entry shifts
    // every LATER actor's head/identity by one (№126: this is exactly how the
    // bridge swapped the cast's identities). Fail INERT instead.
    if (!loadAssets()) {
        return cPhs_COMPLEATE_e;
    }

    const u32 params = fopAcM_GetParam(this);
    const u32 type = params & 0x0F;
    const u32 kind = (params >> 4) & 0x03;

    // ========================================================================
    // Kind dispatch. 0 = grass (blades). 2/3 = flowers, now NATIVE (241/L-6):
    // they used to fall through here as inert and were mounted as NPC_YAFLW
    // J3D models instead, which is what "invisible flowers" was. Trees still
    // ride out -- they are the swood packet, a separate order.
    // ========================================================================
    mKind = (u8)kind;
    mTier = (kind == 3) ? 1 : 0;  // kind 3 -> flowerType 2 -> pink; kind 2 -> white
    if (kind == 1) {
        return cPhs_COMPLEATE_e;  // trees ride the same dispatcher -- inert here
    }
    if (isFlower() && !s_flowerReady) {
        return cPhs_COMPLEATE_e;  // pack refused; stay inert rather than half-draw
    }

    const OffsetData& off =
        l_offsetData[type < (sizeof(l_offsetData) / sizeof(l_offsetData[0])) ? type : 0];

    mBladeCount = 0;
    mRoomNo = static_cast<s8>(fopAcM_GetRoomNo(this));
    std::memset(mCut, 0, sizeof(mCut));
    int probeMiss = 0;
    f32 probeMaxDy = 0.0f;
    for (u32 i = 0; i < off.num && mBladeCount < kMaxBlades; ++i) {
        // Store the WORLD position only. The matrix cannot be baked here: it has
        // to include the view matrix, which changes every frame.
        // §45/№146: ground-sample EACH blade the way the donor does — probe
        // down from +50 and take ANY hit, with no distance clamp.
        //
        // №143 used dExtNpcMount_localGroundSnap for this, which was the wrong
        // tool: its own docstring says it probes from refY+50 down and accepts
        // only within ±250 of refY — a window tuned for LINK'S SPAWN, to reject
        // roofs and sea floor. A blade sitting up to ~135 units out from its
        // clump can easily have ground outside that window on a slope, so the
        // snap was REJECTED and the blade kept the clump's y, leaving it in the
        // air. That made floaters MORE common than having no snap at all.
        //
        // The donor's checkGroundY (d_grass.cpp) has no clamp: probe, use the
        // hit if there is one, keep y only when there is none.
        cXyz bp(current.pos.x + off.pos[i].x, current.pos.y,
                current.pos.z + off.pos[i].z);
        bool gndHit = false;
        {
            dBgS_GndChk gndchk;
            cXyz probe(bp.x, bp.y + 50.0f, bp.z);
            gndchk.SetPos(&probe);
            const f32 gy = dComIfG_Bgsp().GroundCross(&gndchk);
            if (gy > -G_CM3D_F_INF) {
                bp.y = gy;
                gndHit = true;
            }
        }
        // 247: a blade whose ground probe MISSED keeps the clump's y. It still
        // draws there, but its mass test cylinder -- 40x120 for grass, 40x50 for
        // flowers -- then sits at the wrong height, and ChkMass never sees the
        // player: a patch that is visible, polled every frame, and completely
        // inert. Flowers have less than half the vertical tolerance, which is
        // why they fail where grass survives. Count misses and the worst y
        // correction so the bad clumps can be found by position.
        if (!gndHit) {
            ++probeMiss;
        }
        {
            const f32 dy = bp.y - current.pos.y;
            const f32 ady = dy < 0.0f ? -dy : dy;
            if (ady > probeMaxDy) {
                probeMaxDy = ady;
            }
        }
        mBlades[mBladeCount] = bp;
        // Donor setData: `field_0x01 = cM_rndF(7.0f)` -- one of the eight shared
        // ambient sway slots, so a clump does not move as one sheet.
        // Donor: mAnimIdx = cM_rndF(7.0f) -- one of the 8 ambient slots
        // (d_grass.cpp:428, d_flower.cpp:501).
        mAnmIdx[mBladeCount] = (u8)cM_rndF(7.0f);
        mRunCool[mBladeCount] = 0;
        ++mBladeCount;
    }

    DuskLog.info("[ExtVeg] 247 clump kind={} type={} blades={} gndMiss={} maxDy={:.0f} "
                 "pos=({:.0f},{:.0f},{:.0f})",
                 (int)mKind, (int)type, mBladeCount, probeMiss, probeMaxDy, current.pos.x,
                 current.pos.y, current.pos.z);

    dKy_tevstr_init(&tevStr, fopAcM_GetRoomNo(this), 0xFF);
    tevStr.room_no = fopAcM_GetRoomNo(this);

    // A clump is small; let it cull normally on its own point. CULLBOX_CUSTOM
    // with no box is a ZERO box (culled instantly), so give it a real one.
    fopAcM_SetMin(this, -300.0f, -300.0f, -300.0f);
    fopAcM_SetMax(this, 300.0f, 600.0f, 300.0f);
    return cPhs_COMPLEATE_e;
}

int daExtVeg_c::deleteMe() {
    // 244: release this clump's push slots -- see vegAnmReleaseAll.
    vegAnmReleaseAll(isFlower() ? s_flowerAnm : s_grassAnm,
                     isFlower() ? kFlowerAnmNum : kGrassAnmNum, mAnmIdx, mBladeCount);
    return 1;  // №136: owns nothing global — nothing to unwind.
}

int daExtVeg_c::draw() {
    if (!s_assets.ready || mBladeCount == 0) {
        return 1;
    }
    // Defer the real work to the draw phase; see ExtVegPacket_c above.
    mPacket.mOwner = this;
    j3dSys.getDrawBuffer(0)->entryImm(&mPacket, 0);
    return 1;
}

void ExtVegPacket_c::draw() {
    if (mOwner != NULL) {
        if (mOwner->isFlower()) {
            mOwner->drawFlowers();
        } else {
            mOwner->drawBlades();
        }
    }
}

// §45/№147: FIRST REAL INTERACTION between the player and donor world content.
//
// The donor does NOT give each blade a collision object. It asks the mass
// collision system whether anything is hitting that point, per blade, per frame
// (d_grass.cpp hitCheck): ChkMass returns bit 1 for an ATTACK hit and bit 2 for
// a touch. The receiver's own grass uses the identical call (d_grass.inc ~403),
// so this is the same mechanism on both sides — no new collision registration,
// and it costs one query per uncut blade.
void daExtVeg_c::checkCut() {
    // §61 H4 — the early-return must LOG, or its silence is ambiguous with
    // "player wasn't near grass" (the LAW: no-line must be a diagnostic
    // outcome). One heartbeat per ~10s per actor is enough to prove the gate.
    if (!s_assets.ready || mBladeCount == 0) {
        static u32 s_gateN = 0;
        if ((s_gateN++ % 600) == 0) {
            DuskLog.warn("[ExtVeg] §61 GATE actor={:x} ready={} blades={} — checkCut inert",
                         (uintptr_t)this & 0xFFFF, s_assets.ready ? 1 : 0, mBladeCount);
        }
        return;
    }

    // §45/№148: the mass system must be PREPARED and given a test volume before
    // any query, and cleared afterwards. The native grass actor brackets exactly
    // this way (d_a_grass.cpp execute: PrepareMass -> checks -> MassClear), and
    // sets the blade's extent with SetMassAttr before its loop. **Our island
    // never spawns that actor**, so none of it was happening: ChkMass was being
    // asked about an unprepared system with no extent, and therefore never
    // reported a hit. That is why swinging did nothing.
    //
    // Radius/height are the donor's own values for its grass (40 x 80); the
    // receiver uses 40 x 120 for its taller variety. We draw donor blades, so
    // the donor's numbers are the right ones.
    //
    // The bracket is per clump rather than hoisted to a single owner: there is
    // no ordering guarantee between clumps, and a shared owner is exactly the
    // coupling that caused the No.136 blade-list bug. If profiling ever shows
    // this costing, hoist it then — not on speculation.
    // §229: §215/§216 post-cut watch REMOVED (see the cut site).
#if 0
    // §215: report the global particle count for a few frames after any cut.
    // A rise over the pre-spawn baseline proves the emitter EMITS (⇒ the fault
    // is draw-side); a flat count proves it never emits (⇒ dynamics/volume).
    if (s_emitWatchFrames > 0) {
        s_emitWatchFrames--;
        // §216: emitterNum added. particleNum flat proved zero emission; this
        // says whether the emitter is even REGISTERED and calc'd:
        //   emitterNum rises then falls → emitter lived, emitted nothing
        //   emitterNum never rises      → create() didn't register it at all
        DuskLog.warn("[ExtVeg] §215/§216 postCut f={} particleNum={} (base {}) emitterNum={}",
                     s_emitWatchFrames, dComIfGp_particle_getParticleNum(), s_emitWatchBase,
                     dComIfGp_particle_getEmitterNum());
    }
#endif

    vegAnmTickAmbient();
    dComIfG_Ccsp()->PrepareMass();
    // ========================================================================
    // №225 — 40x120, the RECEIVER's own grass test volume (d_grass.inc:1226),
    // replacing the donor's 40x80. №148 argued "we draw donor blades, so the
    // donor's numbers are right" — but the test volume's job is to catch the
    // RECEIVER's attack shapes (TP Link's swing planes sit higher than WW's),
    // and the receiver sized its own grass at 120 for exactly that geometry.
    // First live results fit: only the low, wide great-spin sphere connected;
    // slash capsules likely passed over the 80-tall volume. Same lesson as
    // №224: the receiver is the spec for interaction plumbing.
    // ========================================================================
    dComIfG_Ccsp()->SetMassAttr(40.0f, 120.0f, 11, 0);

    // ========================================================================
    // §61 — 10-hypothesis cut-detection probe (LAW). User: grass not cuttable.
    // ========================================================================
    // One line answers: H2 massN (0 = feed never seen), H3 per-actor wipe (an
    // earlier actor logging massN>0 while we log 0 in the same second), H4/H8
    // nearest-blade distance + dY vs the player, H10 count across held-swing
    // frames. H5/H6/H7 log at the hit site below. H1/H9 correlate with the
    // ALINK-side §61 line (setSwordAtCollision).
    {
        static u32 s_n = 0;
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            f32 best = 1.0e9f;
            f32 bestDY = 0.0f;
            for (int i = 0; i < mBladeCount; ++i) {
                const f32 dx = mBlades[i].x - player->current.pos.x;
                const f32 dz = mBlades[i].z - player->current.pos.z;
                const f32 d2 = dx * dx + dz * dz;
                if (d2 < best) {
                    best = d2;
                    bestDY = player->current.pos.y - mBlades[i].y;
                }
            }
            if (best < 500.0f * 500.0f && (s_n++ % 30) == 0) {
                DuskLog.warn("[ExtVeg] §61 poll actor={:x} blades={} massN=0 nearD={:.0f} dY={:.0f} ccsp={:x}",
                             (uintptr_t)this & 0xFFFF, mBladeCount,
                             std::sqrt(best), bestDY,
                             (uintptr_t)dComIfG_Ccsp() & 0xFFFF);
            }
        }
    }

    for (int i = 0; i < mBladeCount; ++i) {
        if (mRunCool[i] != 0) {
            --mRunCool[i];
        }
        // ====================================================================
        // hitCheck -- donor d_grass.cpp:167-197, structure and all.
        //
        // ONE ChkMass per blade drives everything, and its BITS choose the
        // branch. bit 1 (0x2) = CO, something is standing in the blade -> lean
        // + run-through puff. bit 0 (0x1) = AT, an attack reached it -> cut.
        // Neither -> ease back home.
        //
        // The first cut of this used a raw XZ distance to the player instead of
        // the CO bit. That is what made walking into grass JUDDER: a distance
        // test flickers frame to frame as the player moves, so a blade
        // alternated between "lean toward player" and "ease home" every frame.
        // The mass system is a real volume overlap and does not chatter. It
        // also means anything with CO -- not only the player -- parts the
        // grass, which is the donor's behaviour and was lost by testing the
        // player's position directly.
        //
        // A CUT blade is skipped entirely: the donor's calc only calls
        // hitCheck when `mAnimIdx >= 0`, and WorkAt set it to -1.
        // ====================================================================
        if (mAnmIdx[i] == kAnmNone) {
            continue;
        }
        dCcMassS_HitInf hitInf;
        fopAc_ac_c* hitActor = NULL;
        cXyz p = mBlades[i];
        const u32 massFlags = dComIfG_Ccsp()->ChkMass(&p, &hitActor, &hitInf);
        ++s_probe.examined[0];
        if (massFlags & 2) {
            ++s_probe.co[0];
        }
        if (massFlags & 1) {
            ++s_probe.at[0];
        }
        // AT exclusion: the donor names TWO actors (fpcNm_TSUBO_e, fpcNm_STONE_e)
        // because WW has a pot actor and a stone actor. The receiver unifies both
        // under fpcNm_Obj_Carry_e -- its own grass excludes exactly that
        // (d_grass.inc:404). Same set, one name. Do not "restore" the donor's
        // two identifiers: they do not exist here.
        const bool atHit = (massFlags & 1) && hitActor != NULL &&
                           fopAcM_GetName(hitActor) != fpcNm_Obj_Carry_e;

        if ((massFlags & 2) == 0 && !atHit) {
            // --- untouched: ease rotX home, then chase rotY, then free.
            if (mAnmIdx[i] >= kGrassAmbient) {
                VegAnm& a = s_grassAnm[mAnmIdx[i]];
                const s16 targetY = (s16)(a.rotY & 0xE000);
                const u8 origIdx = (u8)((a.rotY >> 13) & 7);
                if (a.state == 2) {
                    fopAcM_seStart(this, JA_SE_FT_ADD_GRASS, 0);
                    a.state = 1;
                }
                if (!cLib_addCalcAngleS(&a.rotX, s_grassAnm[origIdx].rotX, 16, 4000, 100)) {
                    if (cLib_chaseAngleS(&a.rotY, targetY, 800)) {
                        a.state = 0;  // donor deleteAnm
                        mAnmIdx[i] = (u8)((a.rotY >> 13) & 7);
                        ++s_probe.returned[0];
                    }
                }
            }
            continue;
        }

        // --- WorkCo (donor d_grass.cpp:62-95): something is in the blade.
        if ((massFlags & 2) != 0 && hitActor != NULL) {
            const f32 rdx = mBlades[i].x - hitActor->current.pos.x;
            const f32 rdz = mBlades[i].z - hitActor->current.pos.z;
            const f32 rd2 = rdx * rdx + rdz * rdz;
            if (rd2 <= 1600.0f) {  // donor: distSq > 1600 returns without touching the anim
                ++s_probe.inRange[0];
                ++s_probe.inRange[0];
                ++s_probe.eligible[0];
                if (hitActor->speedF > s_probe.spdMax[0]) {
                    s_probe.spdMax[0] = hitActor->speedF;
                }
                if (hitActor != dComIfGp_getPlayer(0)) {
                    ++s_probe.nonPlayer[0];
                }
                if (!(hitActor->speedF > 16.0f)) {
                    s_probe.lastName[0] = (int)fopAcM_GetName(hitActor);
                }
                if (mAnmIdx[i] < kGrassAmbient) {
                    // The puff fires HERE -- on the frame the blade takes a
                    // push slot -- which is the donor's only rate limit.
                    if (hitActor->speedF > 16.0f) {
                        ++s_probe.fast[0];
                        ++s_probe.spawned[0];
                        cXyz rpos(mBlades[i].x, mBlades[i].y + 20.0f, mBlades[i].z);
                        dKy_tevstr_c runTev;
                        GXColor runK0;
                        extVegSpawnTev(rpos, &runTev, &runK0);
                        // Donor: setSimple(KusaRunPID, &pos, 0xFF, K0, K0, 1).
                        if (extVegEnsureSimple(0x03DB)) {
                            dComIfGp_particle_setSimple(0x03DB, &rpos, 0xFF, runK0, runK0, 1,
                                                        0.0f);
                        }
                        // Donor also calls setBatta here (a grasshopper leaves
                        // the blade). Foundry's order; not ported.
                    }
                    const u8 idx = vegAnmNew(s_grassAnm, kGrassAnmNum);
                    if (idx != kAnmNone) {
                        mAnmIdx[i] = idx;
                    } else {
                        ++s_probe.poolFull[0];
                    }
                }
                if (mAnmIdx[i] >= kGrassAmbient) {
                    VegAnm& a = s_grassAnm[mAnmIdx[i]];
                    a.rotY = cM_atan2s(rdx, rdz);
                    a.rotX = cM_atan2s(40.0f - std::sqrt(rd2), 40.0f);
                    a.state = 2;
                }
            }
        }

        if (!atHit) {
            continue;
        }
        mCut[i] = true;
        // Donor WorkAt:146-150: a cut blade releases its push slot and drops
        // to mAnimIdx = -1, i.e. no sway at all -- the stub does not wave.
        if (mAnmIdx[i] != kAnmNone && mAnmIdx[i] >= kGrassAmbient) {
            s_grassAnm[mAnmIdx[i]].state = 0;
        }
        mAnmIdx[i] = kAnmNone;
        // ====================================================================
        // №220 — the donor's full cut consequence, receiver APIs throughout.
        // ====================================================================
        // Donor (d_grass.cpp WorkAt, cut branch): scatter particle at +25y
        // coloured by the room tevstr, an item roll from the clump's table,
        // then the frame-guarded cut sound. Receiver (d_grass.inc WorkAt) does
        // the same three beats with its own APIs — particle 0x89D7 (its grass
        // scatter), room tevstr + light callback, createItemFromTable with
        // direct-get while on horseback. This is that recipe, verbatim.
        {
            cXyz ppos(mBlades[i].x, mBlades[i].y + 25.0f, mBlades[i].z);
            // ============================================================
            // §182 Ferry V-b: AmbCol was near-black ambient — particle drew it
            // faithfully. Donor scatter colors from room tevstr mColorK0
            // (daylight). Live BG0_K0 = №113 stash dungeonlight_col[1]
            // (same slot W-LINE-c reads at d_kankyo_wether.cpp:1559).
            // Keep settingTevStruct for light info; C0/K0 filled explicitly
            // (settingTevStruct leaves them zero on non-J3D paths — №143).
            // ============================================================
            dKy_tevstr_c cutTev;
            g_env_light.settingTevStruct(0x40, &ppos, &cutTev);
            const GXColorS10& bg0K0 = g_env_light.dungeonlight_col[1];
            auto clamp8 = [](s16 v) -> u8 {
                if (v < 0) {
                    return 0;
                }
                if (v > 255) {
                    return 255;
                }
                return (u8)v;
            };
            const u8 kr = clamp8(bg0K0.r);
            const u8 kg = clamp8(bg0K0.g);
            const u8 kb = clamp8(bg0K0.b);
            cutTev.TevColor.r = kr;
            cutTev.TevColor.g = kg;
            cutTev.TevColor.b = kb;
            cutTev.TevColor.a = 255;
            cutTev.TevKColor.r = kr;
            cutTev.TevKColor.g = kg;
            cutTev.TevKColor.b = kb;
            cutTev.TevKColor.a = 255;
            static csXyz s_cutRot(0, 0, 0);
            // №227: the receiver's own grass passes an envcolor (its per-clump
            // tint, r/g from m_addCol); we have no clump tint so neutral zero —
            // but the ARG is part of the call shape (d_grass.inc:390) and was
            // NULL before. Also capture the emitter handle + inputs once per
            // cut burst: the first live cuts showed NO visible particle, and
            // emitter==NULL vs tevstr==NULL vs bogus room reads off this line.
            // ================================================================
            // §208 ROOT: this argument is the GLOBAL PRM MULTIPLIER, not a
            // decorative "envcolor". dPa_control_c::set's no-flag branch does
            // `setGlobalPrmColor(param_9)` / `setGlobalEnvColor(param_10)`
            // (d_particle.cpp:2397-2404), and the emitter's final colour is
            // resource × global (JPABaseShape.cpp:24-27, COLOR_MULTI). Passing
            // {0,0,0,0} multiplied the donor's authored green by ZERO — BLACK.
            // №227 added it as "part of the call shape"; it was load-bearing.
            //
            // Donor semantics (d_grass.cpp:153): setSimple(pid, pos, 0xFF,
            // tevStr->mColorK0, tevStr->mColorK0, 1) — room K0 as BOTH prm and
            // env modulator. Pass exactly that; the green comes from the
            // emitter's own registers, K0 tints it by time of day.
            // ================================================================
            // ================================================================
            // §211 — the receiver's setSimple is NOT the donor's setSimple.
            //
            // §209 switched to `setSimple` for donor parity. In THIS port that
            // call is registration-gated: `dPa_control_c::setSimple` looks the
            // id up in the pre-registered `dPa_simpleEcallBack` table
            // (`getSimple`, d_particle.cpp:2472-2494) and returns 0 with a
            // JUT_WARN if absent. 0x03DA is a WW-supplemental id that was never
            // put in that table, so NOTHING SPAWNED — the VFX vanished
            // entirely (and the warning never reached the log, so it looked
            // silent). Same name, different function: donor `setSimple` is a
            // general spawn, receiver `setSimple` is a curated fast path.
            //
            // So: back to `particle_set` — the general spawn that DOES route
            // through the WW supplemental resolver (§201) — but with the level
            // callback dropped to NULL. `getLight8EcallBack()` recolours the
            // emitter from the room LIGHT, and this stage's ambient is
            // (36,24,59) dark violet — the purple the user saw (§209's
            // diagnosis, now testable for the first time). K0 still rides in
            // as the prm/env global modulator (§208), which is the donor's
            // colour semantics; the green comes from the emitter's own
            // authored registers.
            // ================================================================
            GXColor k0col;
            k0col.r = kr;
            k0col.g = kg;
            k0col.b = kb;
            k0col.a = 255;
            // ================================================================
            // §241 — FINAL: no level callback. The ported resource draws itself.
            //
            // The donor spawns its cut scatter with no level callback at all
            // (d_grass.cpp:153), and now that the WW resource binds natively that
            // is exactly what works here: JPAResource::draw -> pBsp->setGX ->
            // drawP applies the donor's OWN TEV, blend and z-mode straight from
            // BSP1, in the emitter's authored colours, with room K0 riding in as
            // the global modulator through the call arguments below.
            //
            // Every hand-written presentation we tried before the port is gone,
            // and the reasons are recorded so they are not re-attempted:
            //   * light8 callback (§218/§219) — visible, but its second pass
            //     re-draws through the scene lights, which is the purple and the
            //     time-of-day drift.
            //   * a hand-rolled UNLIT pass (§230/§231) — forced a 1-stage TEV and
            //     cleared the vertex descriptors, which the draw path cannot
            //     survive.
            //   * the registration-gated simple path (§220/§228) — registers but
            //     silently draws nothing on this receiver.
            // The real fault was never presentation: it was BSP1's missing
            // tex-coord-matrix table (§241, ww_jpa_bind.cpp).
            // ================================================================
            // Donor: setSimple(KusaKenPID, &pos, 0xFF, K0, K0, 1) -- d_grass.cpp:154.
            if (extVegEnsureSimple(0x03DA)) {
                dComIfGp_particle_setSimple(0x03DA, &ppos, 0xFF, k0col, k0col, 1, 0.0f);
            }
        }
        if (s_assets.itemTable >= 0) {
            const bool directGet = daPy_getPlayerActorClass() != NULL &&
                                   daPy_getPlayerActorClass()->checkHorseRide();
            fopAcM_createItemFromTable(&mBlades[i], s_assets.itemTable, -1, mRoomNo, NULL, 0,
                                       NULL, NULL, NULL, directGet);
        }
        // One cut sound per frame across every clump, not one per blade — a
        // single sword swing crosses several blades and would otherwise stack
        // the sample on itself. The donor guards this with l_CutSoundFlag.
        if (!s_cutSoundThisFrame) {
            s_cutSoundThisFrame = true;
            mDoAud_seStart(JA_SE_LK_CUT_GRASS, &mBlades[i], 0,
                           dComIfGp_getReverb(mRoomNo));
        }
    }

    // ========================================================================
    // №226 — NO per-actor MassClear: it wipes the GLOBAL mass list and starves
    // every veg actor that executes after this one. №148 kept the bracket per
    // clump; that's harmless for Prepare (idempotent rebuild) but Clear is
    // global state destruction — with the №224 list move it made EXACTLY ONE
    // patch cuttable per session (the first actor in execute order saw Link's
    // shapes, cut its blades, then cleared the list for everyone behind it —
    // the user's "only one patch was ever cut"). The native manager may Clear
    // because it is the SOLE poller of its stage; we are many. The engine
    // already clears unconditionally at frame end (dCcS draw), so simply not
    // clearing here is correct on every stage we exist on.
    // ========================================================================
}

void daExtVeg_c::drawBlades() {
    // §210 probe RETIRED: it proved the blade draw path healthy (15,746 hits,
    // texReady=1, strips=9) — the "not rendering" report was the cut VFX, not
    // the blades. Removed rather than throttled; it answered its question.
    j3dSys.reinitGX();
    GXSetNumIndStages(0);
    // §45/№142: configure the GX lights before anything is emitted. The native
    // grass draw does exactly this, in exactly this position (d_grass.inc, right
    // after GXSetNumIndStages and before GXClearVtxDesc) — and mine did not.
    //
    // This is very likely why the blades were BLACK while their alpha silhouette
    // stayed perfect: the material DL enables a LIT colour channel, and with no
    // GX lights configured the rasterised colour is black. A TEV stage consuming
    // RASC then yields black no matter what vertex colours we emit or what we
    // load into the colour registers — which is consistent with grass staying
    // black across two entirely different colour feeds (room table, own tevStr).
    dKy_setLight_again();
    GXClearVtxDesc();

    // Direct (non-indexed) vertex format — we supply values, not indices.
    static GXVtxDescList l_vtxDescList[] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_CLR0, GX_DIRECT},
        {GX_VA_TEX0, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    static GXVtxAttrFmtList l_vtxAttrFmtList[] = {
        {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
        {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
        {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
        {GX_VA_NULL, GX_POS_XYZ, GX_S8, 0},
    };
    GXSetVtxDescv(l_vtxDescList);
    GXSetVtxAttrFmtv(GX_VTXFMT0, l_vtxAttrFmtList);

    GXCallDisplayList(s_assets.matDL.data(), kMatDLSize);
    if (s_texReady) {
        GXLoadTexObj(&s_texObj, GX_TEXMAP0);
    }

    // §45/№141: populate OUR OWN tevStr instead of reading the room colour table.
    //
    // The donor reads dComIfGp_roomControl_getTevStr(roomNo), which is right in
    // the donor because its rooms are real rooms with populated colour entries.
    // Here Outset is an ACTOR MOUNTED IN A HOST STAGE, so the table entry for the
    // host room was never filled: TevColor came back BLACK, and since the
    // material's TEV stage multiplies through that register, every blade drew
    // black while its alpha silhouette stayed perfectly correct.
    //
    // The island BG in this same file's lane lights correctly precisely because
    // it does NOT use the room table — it calls settingTevStruct on its own
    // tevStr (d_ext_npc_mount.cpp ~5784). Same fix here.
    //
    // This is also why the sky is wrong: both feed from the environment state,
    // which is not populated on a mounted host stage. The housing lane called
    // that ("it's one problem, not two") and they were right.
    g_env_light.settingTevStruct(0x40, &current.pos, &tevStr);

    // §45/№143: colour from AmbCol, measured — not guessed.
    //
    // The donor feeds GX_TEVREG0/1 from its room tevstr's mColorC0/mColorK0. The
    // receiver's dKy_tevstr_c has the same-named TevColor/TevKColor, which made
    // them the obvious analogue — but a one-shot probe showed them arriving as
    // (0,0,0,0) after settingTevStruct, because they are populated by
    // setLightTevColorType_MAJI, which pushes into a J3D MODEL's materials. On a
    // raw-GX path nothing ever fills them, so both earlier colour feeds resolved
    // to black. Same probe showed AmbCol correctly populated — (36,24,59), which
    // is bg0 of PAL0[2] from the converted palettes, i.e. the real environment
    // colour for this room and time of day.
    //
    // So AmbCol is the populated analogue here, and using it keeps grass tracking
    // day/night the way the donor intends.
    GXColorS10 c0;
    c0.r = tevStr.AmbCol.r;
    c0.g = tevStr.AmbCol.g;
    c0.b = tevStr.AmbCol.b;
    c0.a = 255;
    GXColor k0;
    k0.r = 255; k0.g = 255; k0.b = 255; k0.a = 255;
    GXSetTevColorS10(GX_TEVREG0, c0);
    GXSetTevColor(GX_TEVREG1, k0);
    dKy_GxFog_tevstr_set(&tevStr);

    Mtx mv;
    for (int i = 0; i < mBladeCount; ++i) {
        // ====================================================================
        // №227 — cut blades draw the donor's STUB, not nothing. №225's vanish
        // was wrong against the donor (user's Dolphin side-by-side is the
        // spec: a cut leaves a little stub + particle). The donor's draw picks
        // per unit: full DL while standing, `l_Oba_kusa_a_cutDL` after a cut,
        // persisting until reload. Same arrays, different strips.
        // ====================================================================
        if (mDoLib_clipper::clip(j3dSys.getViewMtx(), mBlades[i], 260.0f)) {
            continue;
        }
        // ====================================================================
        // Donor matrix build (d_grass.cpp update:370-406). The blade's ANIM
        // SLOT supplies both angles -- YrotS(rotY) XrotM(rotX) YrotM(-rotY) --
        // and the world position is written into the translation column after.
        //
        // This replaces an inline per-blade wind sway plus an invented
        // `i * 0xDCF` yaw. The donor has no such yaw: the eight ambient slots
        // ARE the variety, each with its own fixed rotY and its own phase of
        // the wind sine (rotX, driven in vegAnmTickAmbient). Computing the sway
        // here also meant a blade could not lean away from the player, because
        // there was nowhere for a per-blade angle to live.
        //
        // mAnmIdx == kAnmNone is a CUT blade: the donor leaves it at -1 and the
        // stub sits still, so build the matrix with no rotation at all.
        // ====================================================================
        if (mAnmIdx[i] == kAnmNone) {
            mDoMtx_trans(mv, mBlades[i].x, mBlades[i].y, mBlades[i].z);
        } else {
            const VegAnm& a = s_grassAnm[mAnmIdx[i]];
            mDoMtx_stack_c::YrotS(a.rotY);
            mDoMtx_stack_c::XrotM(a.rotX);
            mDoMtx_stack_c::YrotM(-a.rotY);
            mDoMtx_copy(mDoMtx_stack_c::get(), mv);
            mv[0][3] = mBlades[i].x;
            mv[1][3] = mBlades[i].y;
            mv[2][3] = mBlades[i].z;
        }
        mDoMtx_concat(j3dSys.getViewMtx(), mv, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
        for (const Strip& st : mCut[i] ? s_cutStrips : s_strips) {
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)st.count);
            for (int v = 0; v < st.count; ++v) {
                GXPosition3f32(st.px[v], st.py[v], st.pz[v]);
                GXColor4u8(st.cr[v], st.cg[v], st.cb[v], st.ca[v]);
                GXTexCoord2f32(st.ts[v], st.tt[v]);
            }
            GXEnd();
        }
    }

    // MANDATORY. This draw rewrites the vertex descriptors and binds its own
    // arrays; without restoring the cache every J3D model drawn AFTER it
    // inherits the wrong vertex format and renders as nothing. Omitting this
    // made the entire cast — people, trees, props, indoors and out — vanish
    // while still spawning normally (№127). BOTH engines' own grass packets
    // end their draw with exactly this call.
    J3DShape::resetVcdVatCache();
    // Release the one-cut-sound-per-frame latch (see checkCut). Cleared in draw
    // rather than execute because every clump executes before any clump draws,
    // so clearing in execute would let each clump sound separately.
    s_cutSoundThisFrame = false;
}

// ==========================================================================
// FLOWER DRAW -- donor d_flower.cpp::update (matrix) + ::draw (GX).
// ==========================================================================
// The donor builds each flower's model matrix from a SHARED sway matrix:
//
//   update():  YrotS(angY); XrotM(angX); YrotM(-angY)      [8 ambient slots]
//              then per flower: mtx[*][3] = world pos; concat(view, mtx)
//   calc():    angX = scos((timer + i*0xFA) * 1000.0f) * 1000.0f + 1000.0f
//   ctor:      slot i gets angY = i * 0x2000
//
// The donor POOLS those matrices (8 ambient + 64 player-push slots) because one
// global packet serves up to 200 flowers and it will not build 200 matrices.
// This actor owns its own handful, so the same values are computed per flower
// instead of pooled: identical output, no allocator, and no shared-lifetime
// coupling of the kind No.136 already cost this lane once. The player-push
// branch (WorkCo) folds in the same way.
// ==========================================================================
void daExtVeg_c::drawFlowers() {
    const FlowerTier& f = s_flower[mTier];
    j3dSys.reinitGX();
    GXSetNumIndStages(0);
    dKy_setLight_again();
    GXClearVtxDesc();

    static GXVtxDescList l_fVtxDescList[] = {
        {GX_VA_POS, GX_DIRECT},
        {GX_VA_CLR0, GX_DIRECT},
        {GX_VA_TEX0, GX_DIRECT},
        {GX_VA_NULL, GX_NONE},
    };
    static GXVtxAttrFmtList l_fVtxAttrFmtList[] = {
        {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
        {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
        {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
        {GX_VA_NULL, GX_POS_XYZ, GX_S8, 0},
    };
    GXSetVtxDescv(l_fVtxDescList);
    GXSetVtxAttrFmtv(GX_VTXFMT0, l_fVtxAttrFmtList);

    GXCallDisplayList(const_cast<u8*>(f.matDL.data()), kFlowerMatDLSize);
    if (f.texReady) {
        GXLoadTexObj(const_cast<GXTexObj*>(&f.texObj), GX_TEXMAP0);
    }

    // No.141/No.143, unchanged from the blade path: a mounted host stage never
    // fills the room colour table, so feed our own tevStr and use AmbCol, which
    // the probe showed IS populated. The donor reads its room tevstr C0/K0 here.
    g_env_light.settingTevStruct(0x40, &current.pos, &tevStr);
    GXColorS10 c0;
    c0.r = tevStr.AmbCol.r;
    c0.g = tevStr.AmbCol.g;
    c0.b = tevStr.AmbCol.b;
    c0.a = 255;
    GXColor k0;
    k0.r = 255; k0.g = 255; k0.b = 255; k0.a = 255;
    GXSetTevColorS10(GX_TEVREG0, c0);
    GXSetTevColor(GX_TEVREG1, k0);
    dKy_GxFog_tevstr_set(&tevStr);

    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    Mtx mv;
    for (int i = 0; i < mBladeCount; ++i) {
        // Donor clips at +260y with a 260 radius (update:466).
        cXyz clipAt(mBlades[i].x, mBlades[i].y + 260.0f, mBlades[i].z);
        if (mDoLib_clipper::clip(j3dSys.getViewMtx(), clipAt, 260.0f)) {
            continue;
        }

        // ====================================================================
        // Donor matrix build (d_flower.cpp update:443-478) -- identical in
        // shape to the grass one, reading this flower's anim slot. The player
        // lean is NOT computed here: it is written into the slot by
        // checkFlowers (donor WorkCo), so a flower that has taken a push slot
        // is already leaning by the time the draw reads it.
        // ====================================================================
        const VegAnm& a = s_flowerAnm[mAnmIdx[i] == kAnmNone ? 0 : mAnmIdx[i]];
        const s16 angY = (mAnmIdx[i] == kAnmNone) ? 0 : a.rotY;
        const s16 angX = (mAnmIdx[i] == kAnmNone) ? 0 : a.rotX;
        mDoMtx_stack_c::YrotS(angY);
        mDoMtx_stack_c::XrotM(angX);
        mDoMtx_stack_c::YrotM(-angY);
        mDoMtx_copy(mDoMtx_stack_c::get(), mv);
        mv[0][3] = mBlades[i].x;
        mv[1][3] = mBlades[i].y;
        mv[2][3] = mBlades[i].z;
        mDoMtx_concat(j3dSys.getViewMtx(), mv, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);

        // Cut flowers draw the donor's `gut` list, exactly as blades draw the
        // stub (No.227) -- the donor picks on bit 0x8 in draw:348.
        const std::vector<Strip>& strips = mCut[i] ? f.cutStrips : f.strips;
        for (size_t sIdx = 0; sIdx < strips.size(); ++sIdx) {
            const Strip& st = strips[sIdx];
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)st.count);
            for (int v = 0; v < st.count; ++v) {
                GXPosition3f32(st.px[v], st.py[v], st.pz[v]);
                GXColor4u8(st.cr[v], st.cg[v], st.cb[v], st.ca[v]);
                GXTexCoord2f32(st.ts[v], st.tt[v]);
            }
            GXEnd();
        }
    }

    // MANDATORY -- see drawBlades. Without it every J3D model drawn after this
    // inherits our vertex format and renders as nothing (No.127).
    J3DShape::resetVcdVatCache();
    s_cutSoundThisFrame = false;
}

// ==========================================================================
// FLOWER EXECUTE -- donor hitCheck/WorkAt (cut) + WorkCo (run-through VFX).
// ==========================================================================
void daExtVeg_c::checkFlowers() {
    if (!s_flowerReady || mBladeCount == 0) {
        return;
    }

    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    vegAnmTickAmbient();

    // Donor calc:417 -- flowers use a 30 x 50 mass volume, NOT grass's 40 x 120.
    dComIfG_Ccsp()->PrepareMass();
    dComIfG_Ccsp()->SetMassAttr(30.0f, 50.0f, 0xB, 0x2);

    for (int i = 0; i < mBladeCount; ++i) {
        if (mRunCool[i] != 0) {
            --mRunCool[i];  // donor cLib_calcTimer on field_0x03
        }

        // ------------------------------------------------------------------
        // hitCheck -- donor d_flower.cpp:231-259. One ChkMass, its bits pick
        // the branch, exactly as grass does. The first cut of this used a raw
        // distance to the player, which is what made walking into vegetation
        // judder (see the grass path for the full note).
        //
        // Two things differ from grass, both the donor's:
        //   * WorkAt is gated on NOT already being gut -- a cut flower cannot
        //     be cut again, but it IS still hitChecked and still leans.
        //   * flowers use a 30-unit gate (900) and lean by atan2s(30-d, 40).
        // ------------------------------------------------------------------
        dCcMassS_HitInf hitInf;
        fopAc_ac_c* hitActor = NULL;
        cXyz pt = mBlades[i];
        const u32 massFlags = dComIfG_Ccsp()->ChkMass(&pt, &hitActor, &hitInf);
        ++s_probe.examined[1];
        if (massFlags & 2) {
            ++s_probe.co[1];
        }
        if (massFlags & 1) {
            ++s_probe.at[1];
        }
        const bool atHit = (massFlags & 1) && hitActor != NULL &&
                           fopAcM_GetName(hitActor) != fpcNm_Obj_Carry_e;

        if ((massFlags & 2) == 0 && !atHit) {
            if (mAnmIdx[i] != kAnmNone && mAnmIdx[i] >= kGrassAmbient) {
                VegAnm& a = s_flowerAnm[mAnmIdx[i]];
                const s16 targetY = (s16)(a.rotY & 0xE000);
                const u8 origIdx = (u8)((a.rotY >> 13) & 7);
                if (!cLib_addCalcAngleS(&a.rotX, s_flowerAnm[origIdx].rotX, 16, 4000, 100)) {
                    if (cLib_chaseAngleS(&a.rotY, targetY, 800)) {
                        a.state = 0;
                        mAnmIdx[i] = (u8)((a.rotY >> 13) & 7);
                        ++s_probe.returned[1];
                    }
                }
            }
            continue;
        }

        // --- WorkCo (donor d_flower.cpp:59-118).
        if ((massFlags & 2) != 0 && hitActor != NULL && mAnmIdx[i] != kAnmNone) {
            const f32 dx = mBlades[i].x - hitActor->current.pos.x;
            const f32 dz = mBlades[i].z - hitActor->current.pos.z;
            const f32 d2 = dx * dx + dz * dz;
            if (d2 <= 900.0f) {
                ++s_probe.inRange[1];
                if (mAnmIdx[i] < kGrassAmbient) {
                    ++s_probe.eligible[1];
                    if (hitActor->speedF > s_probe.spdMax[1]) {
                        s_probe.spdMax[1] = hitActor->speedF;
                    }
                    if (hitActor != dComIfGp_getPlayer(0)) {
                        ++s_probe.nonPlayer[1];
                    }
                    if (!(hitActor->speedF > 16.0f)) {
                        s_probe.lastName[1] = (int)fopAcM_GetName(hitActor);
                    }
                    // Flowers carry BOTH gates: the slot transition AND a real
                    // field_0x03 = 0x10 cooldown. Grass has only the slot.
                    if (mRunCool[i] == 0 && !mCut[i] && hitActor->speedF > 16.0f) {
                        ++s_probe.fast[1];
                        cXyz ppos(mBlades[i].x, mBlades[i].y + 20.0f, mBlades[i].z);
                        dKy_tevstr_c spawnTev;
                        GXColor kc;
                        extVegSpawnTev(ppos, &spawnTev, &kc);
                        static csXyz s_runRot(0, 0, 0);
                        JPABaseEmitter* em = dComIfGp_particle_set(
                            flowerParticleId(), &ppos, &spawnTev, &s_runRot, NULL, 255, NULL, -1,
                            &kc, &kc, NULL);
                        if (em != NULL) {
                            em->setRate(1.0f);
                            mRunCool[i] = 0x10;
                            ++s_probe.spawned[1];
                        }
                    }
                    const u8 idx = vegAnmNew(s_flowerAnm, kFlowerAnmNum);
                    if (idx != kAnmNone) {
                        mAnmIdx[i] = idx;
                    } else {
                        ++s_probe.poolFull[1];
                    }
                }
                if (mAnmIdx[i] >= kGrassAmbient) {
                    VegAnm& a = s_flowerAnm[mAnmIdx[i]];
                    a.rotY = cM_atan2s(dx, dz);
                    a.rotX = cM_atan2s(30.0f - std::sqrt(d2), 40.0f);
                    // Donor flower WorkCo sets ONLY the two angles; it never
                    // writes state 2 the way grass does (d_flower.cpp:113-118
                    // vs d_grass.cpp:93-95). The state-2 write here was mine.
                }
            }
        }

        // Donor gates WorkAt on the flower not already being gut.
        if (!atHit || mCut[i]) {
            continue;
        }

        // ------------------------------------------------------------------
        // CUT (donor WorkAt:189-227).
        // ------------------------------------------------------------------
        mCut[i] = true;  // donor onBit(field_0x00, 0x8) -- the `gut` state
        // Donor WorkAt:195-197 frees the push slot on cut. Note it does NOT
        // clear the index the way grass does -- a cut flower keeps swaying,
        // and the gut geometry is what changes. Matching that.
        if (mAnmIdx[i] != kAnmNone && mAnmIdx[i] >= kGrassAmbient) {
            s_flowerAnm[mAnmIdx[i]].state = 0;
            mAnmIdx[i] = (u8)((s_flowerAnm[mAnmIdx[i]].rotY >> 13) & 7);
        }

        // Donor WorkAt spawns the cut VFX at the flower's own position (no y
        // offset, unlike grass's +25) and passes room K0 as both prm and env.
        {
            cXyz ppos = mBlades[i];
            dKy_tevstr_c spawnTev;
            GXColor kc;
            extVegSpawnTev(ppos, &spawnTev, &kc);
            static csXyz s_flowerCutRot(0, 0, 0);
            dComIfGp_particle_set(flowerParticleId(), &ppos, &spawnTev, &s_flowerCutRot, NULL,
                                  255, NULL, -1, &kc, &kc, NULL);
        }

        if (s_assets.itemTable >= 0) {
            const bool directGet = daPy_getPlayerActorClass() != NULL &&
                                   daPy_getPlayerActorClass()->checkHorseRide();
            fopAcM_createItemFromTable(&mBlades[i], s_assets.itemTable, -1, mRoomNo, NULL, 0,
                                       NULL, NULL, NULL, directGet);
        }

        // The donor uses the GRASS cut sound for flowers too (WorkAt:225).
        if (!s_cutSoundThisFrame) {
            s_cutSoundThisFrame = true;
            fopAcM_seStart(this, JA_SE_LK_CUT_GRASS, 0);
        }
    }
    // ========================================================================
    // 248 -- NO MassClear. The donor never calls it: neither
    // dGrass_packet_c::calc nor dFlower_packet_c::calc clears the mass list,
    // they only SetMassAttr and poll. This call was mine, and 226 had already
    // written down why it must not exist here -- Clear is GLOBAL state
    // destruction, and the receiver runs MANY vegetation actors where the
    // donor runs one packet.
    //
    // What it did: Link registers his mass shapes once per frame. The first
    // flower clump to execute polled them and then wiped the list, so every
    // vegetation actor BEHIND it in execute order saw massFlags = 0 forever --
    // no CO (no lean, no run-through puff) and no AT (uncuttable). Visible,
    // polled every frame, completely inert, and positional only by accident
    // of execute order, which is why a dead patch could sit next to a working
    // one. `exam` still counted those blades because it increments before the
    // flags are read, so the census arithmetic looked perfect throughout.
    //
    // The engine clears unconditionally at frame end (dCcS draw), so simply
    // not clearing here is correct on every stage we exist on.
    // ========================================================================
}

// ==========================================================================
// Profile
// ==========================================================================
static int daExtVeg_Create(fopAc_ac_c* i_this) {
    return static_cast<daExtVeg_c*>(i_this)->create();
}
static int daExtVeg_Delete(void* i_this) {
    return static_cast<daExtVeg_c*>(i_this)->deleteMe();
}
static int daExtVeg_Execute(void* i_this) {
    return static_cast<daExtVeg_c*>(i_this)->execute();
}
static int daExtVeg_Draw(void* i_this) {
    return static_cast<daExtVeg_c*>(i_this)->draw();
}
static int daExtVeg_IsDelete(void* i_this) {
    (void)i_this;
    return 1;
}

static DUSK_CONST actor_method_class l_daExtVeg_Method = {
    (process_method_func)daExtVeg_Create,  (process_method_func)daExtVeg_Delete,
    (process_method_func)daExtVeg_Execute, (process_method_func)daExtVeg_IsDelete,
    (process_method_func)daExtVeg_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_EXT_VEG = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    // ========================================================================
    // №224 — List ID 7 → 11: poll the mass system at the RECEIVER's slot.
    // ========================================================================
    // 7 was the DONOR's grass-manager placement (WW d_a_grass.cpp, player at
    // list 5) imported verbatim. But the receiver's own grass poller
    // (g_profile_GRASS) sits at 11 — after EVERY mass feeder (Alink 5, arrow/
    // boomerang/spinner/horse) — and §61 proved a list-7 poll here reads an
    // empty mass list 247/247 while native list-11 grass cuts fine. The
    // receiver's frame-phase contract is the spec for WHEN the list is
    // visible; 11 is its answer. Mirror of №173's flags lesson, inverted.
    /* List ID      */ 11,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_EXT_VEG_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daExtVeg_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daExtVeg_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#endif  // TARGET_PC
