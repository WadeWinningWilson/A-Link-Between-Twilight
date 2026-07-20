/**
 * d_a_ww_grass.cpp
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

#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#include <filesystem>

#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_xyz.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "dolphin/gx/GXGeometry.h"
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

struct WwGrassAssets {
    std::vector<u8> pos;
    std::vector<u8> color;
    std::vector<u8> texCoord;
    std::vector<u8> matDL;
    std::vector<u8> bladeDL;
    std::vector<u8> bladeCutDL;
    std::vector<u8> tex;
    bool ready = false;
    bool tried = false;
};

WwGrassAssets s_assets;

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

// The pack lives under whichever mod folder shipped it; scan rather than hard-
// code a folder name, matching how the rest of the custom-asset lane resolves.
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
        if (!std::filesystem::is_directory(veg, ec)) {
            continue;
        }
        const bool ok =
            readBlob(veg / "d_grass__l_pos.bin", s_assets.pos) &&
            readBlob(veg / "d_grass__l_color.bin", s_assets.color) &&
            readBlob(veg / "d_grass__l_texCoord.bin", s_assets.texCoord) &&
            readBlob(veg / "d_grass__l_matDL.bin", s_assets.matDL) &&
            readBlob(veg / "d_grass__l_Oba_kusa_aDL.bin", s_assets.bladeDL) &&
            readBlob(veg / "d_grass__l_Oba_kusa_a_cutDL.bin", s_assets.bladeCutDL) &&
            readBlob(veg / "d_grass__l_Txa_ob_kusa_aTEX.bin", s_assets.tex);
        if (ok) {
            s_assets.ready = true;
            DuskLog.info(
                "[WwGrass] №122 asset pack loaded from '{}' (pos={} color={} tex={} bytes)",
                entry.path().filename().string(), s_assets.pos.size(), s_assets.color.size(),
                s_assets.tex.size());
            return true;
        }
        DuskLog.warn("[WwGrass] №122 incomplete pack under '{}' — grass disabled",
                     entry.path().filename().string());
    }
    return false;
}

// ==========================================================================
// Instance registry — every clump expands into N blades at Create time, the
// same way the donor's actor seeds its packet.
// ==========================================================================
struct Blade {
    Mtx mtx;
    s8 roomNo;
    bool live;
};

std::vector<Blade> s_blades;
fpc_ProcID s_drawOwner = fpcM_ERROR_PROCESS_ID_e;

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
class daWwGrass_c : public fopAc_ac_c {
public:
    int create();
    int execute() { return 1; }
    int draw();
    int deleteMe();

    u16 mFirstBlade;
    u16 mBladeCount;
};

int daWwGrass_c::create() {
    fopAcM_ct(this, daWwGrass_c);

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

    mFirstBlade = static_cast<u16>(s_blades.size());
    mBladeCount = off.num;

    const s8 roomNo = static_cast<s8>(fopAcM_GetRoomNo(this));
    for (u32 i = 0; i < off.num; ++i) {
        Blade b{};
        cXyz p;
        p.x = current.pos.x + off.pos[i].x;
        p.y = current.pos.y;
        p.z = current.pos.z + off.pos[i].z;
        mDoMtx_stack_c::transS(p.x, p.y, p.z);
        MTXCopy(mDoMtx_stack_c::get(), b.mtx);
        b.roomNo = roomNo;
        b.live = true;
        s_blades.push_back(b);
    }

    if (s_drawOwner == fpcM_ERROR_PROCESS_ID_e) {
        s_drawOwner = fopAcM_GetID(this);
    }
    return cPhs_COMPLEATE_e;
}

int daWwGrass_c::deleteMe() {
    for (u32 i = 0; i < mBladeCount && (mFirstBlade + i) < s_blades.size(); ++i) {
        s_blades[mFirstBlade + i].live = false;
    }
    if (s_drawOwner == fopAcM_GetID(this)) {
        s_drawOwner = fpcM_ERROR_PROCESS_ID_e;
        // Whole space is tearing down; drop the list so a re-entry rebuilds it.
        s_blades.clear();
    }
    return 1;
}

int daWwGrass_c::draw() {
    // One owner draws the entire list: GX setup is paid once per frame instead
    // of once per clump. Mirrors the donor's packet draw.
    if (s_drawOwner != fopAcM_GetID(this) || !s_assets.ready || s_blades.empty()) {
        return 1;
    }

    j3dSys.reinitGX();
    GXSetNumIndStages(0);
    GXClearVtxDesc();

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

    GXSetVtxDescv(l_vtxDescList);
    GXSetVtxAttrFmtv(GX_VTXFMT0, l_vtxAttrFmtList);
    GXSETARRAY(GX_VA_POS, s_assets.pos.data(), (u32)s_assets.pos.size(), sizeof(cXyz), true);
    GXSETARRAY(GX_VA_CLR0, s_assets.color.data(), (u32)s_assets.color.size(), sizeof(GXColor),
               true);
    GXSETARRAY(GX_VA_TEX0, s_assets.texCoord.data(), (u32)s_assets.texCoord.size(), 8, true);

    GXCallDisplayList(s_assets.matDL.data(), kMatDLSize);

    s8 lastRoom = -2;
    for (const Blade& b : s_blades) {
        if (!b.live) {
            continue;
        }
        // Room tev only needs re-binding when the room actually changes.
        if (b.roomNo != lastRoom) {
            dKy_tevstr_c* tevstr = dComIfGp_roomControl_getTevStr(b.roomNo);
            if (tevstr != NULL) {
                GXSetTevColorS10(GX_TEVREG0, tevstr->TevColor);
                GXSetTevColor(GX_TEVREG1, tevstr->TevKColor);
                dKy_GxFog_tevstr_set(tevstr);
            }
            lastRoom = b.roomNo;
        }
        GXLoadPosMtxImm(const_cast<f32(*)[4]>(b.mtx), GX_PNMTX0);
        GXCallDisplayList(s_assets.bladeDL.data(), kBladeDLSize);
    }

    // MANDATORY. This draw rewrites the vertex descriptors and binds its own
    // arrays; without restoring the cache every J3D model drawn AFTER it
    // inherits the wrong vertex format and renders as nothing. Omitting this
    // made the entire cast — people, trees, props, indoors and out — vanish
    // while still spawning normally (№127). BOTH engines' own grass packets
    // end their draw with exactly this call.
    J3DShape::resetVcdVatCache();
    return 1;
}

// ==========================================================================
// Profile
// ==========================================================================
static int daWwGrass_Create(fopAc_ac_c* i_this) {
    return static_cast<daWwGrass_c*>(i_this)->create();
}
static int daWwGrass_Delete(void* i_this) {
    return static_cast<daWwGrass_c*>(i_this)->deleteMe();
}
static int daWwGrass_Execute(void* i_this) {
    return static_cast<daWwGrass_c*>(i_this)->execute();
}
static int daWwGrass_Draw(void* i_this) {
    return static_cast<daWwGrass_c*>(i_this)->draw();
}
static int daWwGrass_IsDelete(void* i_this) {
    (void)i_this;
    return 1;
}

static DUSK_CONST actor_method_class l_daWwGrass_Method = {
    (process_method_func)daWwGrass_Create,  (process_method_func)daWwGrass_Delete,
    (process_method_func)daWwGrass_Execute, (process_method_func)daWwGrass_IsDelete,
    (process_method_func)daWwGrass_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_WWGRASS = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_WWGRASS_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daWwGrass_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daWwGrass_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#endif  // TARGET_PC
