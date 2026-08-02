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
void decodeDLInto(const std::vector<u8>& dl, size_t limit, std::vector<Strip>& out) {
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
            st.px[v] = beF32(s_assets.pos, pi * 12 + 0);
            st.py[v] = beF32(s_assets.pos, pi * 12 + 4);
            st.pz[v] = beF32(s_assets.pos, pi * 12 + 8);
            const size_t co = ci * 4;
            st.cr[v] = co + 3 < s_assets.color.size() ? s_assets.color[co + 0] : 255;
            st.cg[v] = co + 3 < s_assets.color.size() ? s_assets.color[co + 1] : 255;
            st.cb[v] = co + 3 < s_assets.color.size() ? s_assets.color[co + 2] : 255;
            st.ca[v] = co + 3 < s_assets.color.size() ? s_assets.color[co + 3] : 255;
            st.ts[v] = beF32(s_assets.texCoord, ti * 8 + 0);
            st.tt[v] = beF32(s_assets.texCoord, ti * 8 + 4);
        }
        out.push_back(st);
        i += 3 + cnt * 3;
    }
}

void decodeBladeDL() {
    decodeDLInto(s_assets.bladeDL, 0xA0, s_strips);
    // №227: the stub geometry — donor draws it from frame-of-cut onward.
    decodeDLInto(s_assets.bladeCutDL, 0x80, s_cutStrips);
}

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
            GXInitTexObj(&s_texObj, s_assets.tex.data(), 64, 64, GX_TF_CMPR, GX_REPEAT,
                         GX_CLAMP, GX_FALSE);
            s_texReady = true;
            s_assets.ready = true;
            decodeBladeDL();
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
        checkCut();
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
};


int daExtVeg_c::create() {
    fopAcM_ct(this, daExtVeg_c);

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

    // kind 0 is grass. Trees/flowers ride the same census dispatcher in the
    // donor but need their own packets — not this actor's job.
    if (kind != 0) {
        return cPhs_COMPLEATE_e;  // trees/flowers ride the same dispatcher — inert here
    }

    const OffsetData& off =
        l_offsetData[type < (sizeof(l_offsetData) / sizeof(l_offsetData[0])) ? type : 0];

    mBladeCount = 0;
    mRoomNo = static_cast<s8>(fopAcM_GetRoomNo(this));
    std::memset(mCut, 0, sizeof(mCut));
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
        {
            dBgS_GndChk gndchk;
            cXyz probe(bp.x, bp.y + 50.0f, bp.z);
            gndchk.SetPos(&probe);
            const f32 gy = dComIfG_Bgsp().GroundCross(&gndchk);
            if (gy > -G_CM3D_F_INF) {
                bp.y = gy;
            }
        }
        mBlades[mBladeCount] = bp;
        ++mBladeCount;
    }

    dKy_tevstr_init(&tevStr, fopAcM_GetRoomNo(this), 0xFF);
    tevStr.room_no = fopAcM_GetRoomNo(this);

    // A clump is small; let it cull normally on its own point. CULLBOX_CUSTOM
    // with no box is a ZERO box (culled instantly), so give it a real one.
    fopAcM_SetMin(this, -300.0f, -300.0f, -300.0f);
    fopAcM_SetMax(this, 300.0f, 600.0f, 300.0f);
    return cPhs_COMPLEATE_e;
}

int daExtVeg_c::deleteMe() {
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
        mOwner->drawBlades();
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
        if (mCut[i]) {
            continue;  // already a stump; nothing left to hit
        }
        dCcMassS_HitInf hitInf;
        fopAc_ac_c* hitActor = NULL;
        cXyz p = mBlades[i];
        const u32 massFlags = dComIfG_Ccsp()->ChkMass(&p, &hitActor, &hitInf);
        // §61 r3: EVERY nonzero flags value logs — the %10 limit hid the true
        // hit count in the first live run (1 line could be 1..10 hits). Hits
        // are rare frames; unconditional is safe. Blade index makes multi-hit
        // sweeps countable per swing.
        if (massFlags != 0) {
            DuskLog.warn("[ExtVeg] §61 HIT actor={:x} blade={} flags=0x{:X} name={}",
                         (uintptr_t)this & 0xFFFF, i, massFlags,
                         hitActor != NULL ? (int)fopAcM_GetName(hitActor) : -1);
        }
        // bit 0 = AT (an attack reached this point). The receiver's grass
        // excludes carried objects the same way; without that a thrown pot
        // would mow the field.
        const bool atHit = (massFlags & 1) && hitActor != NULL &&
                           fopAcM_GetName(hitActor) != fpcNm_Obj_Carry_e;
        if (!atHit) {
            continue;
        }
        mCut[i] = true;
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
            dComIfGp_particle_set(0x03DA, &ppos, &cutTev, &s_cutRot, NULL, 255, NULL, -1,
                                  &k0col, &k0col, NULL);
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
    const f32 wind_pow = dKyw_get_wind_pow();
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
        mDoMtx_trans(mv, mBlades[i].x, mBlades[i].y, mBlades[i].z);
        mDoMtx_YrotM(mv, (s16)(i * 0xDCF));
        // ============================================================
        // §130 HUNT 1 — donor per-blade wind sway (d_grass.cpp:322-329):
        // windSpeed = wind_pow*1000+1000 (clamp 2000); mRotX = windSpeed
        // + windSpeed*cos(windSpeed*(timer + i*250)). The i*250 phase is
        // what ripples the field instead of moving it as one sheet.
        // ============================================================
        {
            f32 windSpeed = wind_pow * 1000.0f + 1000.0f;
            windSpeed = cLib_maxLimit(windSpeed, 2000.0f);
            const s16 sway = (s16)(windSpeed + windSpeed *
                cM_scos((s16)(windSpeed * (f32)(g_Counter.mTimer + i * 250))));
            mDoMtx_XrotM(mv, sway);
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
