/**
 * d_a_ext_plank_span.cpp
 *
 * №124: donor rope bridge — a PLANK-INSTANCING actor, not a static prop.
 *
 * =========================================================================
 * WHY THIS EXISTS
 * =========================================================================
 * `bridge` was first wired as an ordinary manifest prop. It rendered exactly
 * one small plank that z-fought with itself, because `obm_bridge.bdl` IS one
 * plank segment. The donor's `d_a_bridge.cpp` picks a model by type bits and
 * then loops `for (i < mBrCount)`, creating one J3DModel per plank along a
 * span, so a manifest — which spawns a single model — can never build a
 * bridge. That is the rule this actor exists to satisfy: a prop whose donor
 * actor procedurally instances geometry needs a port, not a manifest.
 *
 * =========================================================================
 * WHERE THE SPAN COMES FROM
 * =========================================================================
 * The donor derives the span from a room PATH: `pathId = (param >> 16) & 0xFF`,
 * `dPath_GetRoomPath(...)`, endpoints `points[0]`/`points[1]`, then
 *
 *     count = |end - start| / ((|d| > 1300 ? 3 : 0) + 47) * 1.5
 *
 * Our authored room carries no RPAT/RPPN, and the population spawner passes
 * its own socket arg rather than the donor param word, so neither the path nor
 * the path id survives to this actor. The endpoints are therefore authored as
 * DATA (`population/bridges.ini`, extracted verbatim from the donor room's
 * RPAT/RPPN) and each instance claims the span whose start is nearest its own
 * spawn position — deterministic for the two placements Outset actually has.
 * The plank count uses the donor's formula unchanged, so spacing matches.
 *
 * NOT PORTED (deliberate, declared): the rope simulation, per-plank cylinder
 * collision, sway and the chain variant (`obm_chain1.bdl`, type bit 1). This
 * builds the bridge's GEOMETRY. Walking on it still depends on the span's own
 * collision mesh being mounted separately — see the ledger entry.
 * =========================================================================
 */
// KIT-LINEAGE: native-port

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/d_kankyo_ww.h"  // §425 native lighting chain

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
#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_tev_str.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_bg_w_sv.h"        // §429 deformable span collision
#include "SSystem/SComponent/c_math.h"
#include "m_Do/m_Do_mtx.h"

void dExtWw_repackDzbAttributes(cBgD_t* bgd, const char* tag);  // §429f (§334)

namespace {

struct Span {
    cXyz start;
    cXyz end;
};

std::vector<Span> s_spans;
std::string s_plankArc;
std::string s_plankModel;
std::string s_chainModel;  // optional; reserved for chain variant (§45)
u8 s_spanType = 0;  // §429g donor typeBits (Room44.dzr receipt: 0x0A = aite|thickrope)
bool s_spansTried = false;

// §45: span endpoints + plank/arc names from data. No hardcoded model strings.
void loadSpans() {
    if (s_spansTried) {
        return;
    }
    s_spansTried = true;

    const std::filesystem::path root =
        std::filesystem::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::is_directory(root, ec)) {
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (!entry.is_directory()) {
            continue;
        }
        const std::filesystem::path ini = entry.path() / "population" / "bridges.ini";
        std::ifstream in(ini);
        if (!in) {
            continue;
        }
        Span cur{};
        bool haveStart = false;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            const size_t eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            while (!val.empty() && (val.back() == '\r' || val.back() == ' ')) {
                val.pop_back();
            }
            if (key == "plank_arc" || key == "arc") {
                s_plankArc = val;
                continue;
            }
            if (key == "plank_model") {
                s_plankModel = val;
                continue;
            }
            if (key == "chain_model") {
                s_chainModel = val;
                continue;
            }
            if (key == "type") {
                s_spanType = (u8)std::strtoul(val.c_str(), NULL, 0);
                continue;
            }
            float x = 0.0f, y = 0.0f, z = 0.0f;
            if (std::sscanf(val.c_str(), "%f,%f,%f", &x, &y, &z) != 3) {
                continue;
            }
            if (key == "start") {
                cur.start.set(x, y, z);
                haveStart = true;
            } else if (key == "end" && haveStart) {
                cur.end.set(x, y, z);
                s_spans.push_back(cur);
                haveStart = false;
            }
        }
        if (!s_spans.empty()) {
            DuskLog.info("[ExtSpan] №124 {} span(s) loaded from '{}' plank={}", s_spans.size(),
                         entry.path().filename().string(), s_plankModel);
            if (!s_chainModel.empty()) {
                DuskLog.info("[ExtSpan] §45 chain_model='{}' reserved (not wired)", s_chainModel);
            }
            return;
        }
    }
    DuskLog.warn("[ExtSpan] №124 no bridges.ini found — bridges disabled");
}

const int kMaxPlanks = 50;  // donor rejects >= 50

}  // namespace

// ============================================================================
// §429 DONOR SIM STATE -- WW d_a_bridge br_s subset (spec:
// docs/state/ww-bridge-donor-spec.md). NOT PORTED THIS PASS (declared, owed):
// cut/fire mechanics + sword cylinders, chain/aite variants, moblin/boar/bomb
// rider specials, snap (m0304/m0308), sounds/particles. Outset's bridges are
// plain rope type; riders = player weight 100/depth -31, default 50/-10.
// ============================================================================
struct SpanPlank {
    cXyz m3CC;                       // rope-chain sim node
    cXyz mPosition;                  // rendered pos = node + taper
    csXyz mRotation;                 // y=heading to next node, x=pitch, z=roll
    s16 mRotationYExtra;             // random 180 flip
    f32 m3F0;                        // span weight profile |sin(pi*i/(n-1))|
    f32 m3F4;                        // rider deflection target
    f32 m3F8;                        // current deflection
    f32 m3FC;                        // sag term -> -15
    s16 m400;                        // rider roll target
    s16 m402;                        // rider roll current
    u8 m406;                         // rider-active countdown
    u8 m408;                         // bit2 = rope post
    cXyz mScale;
    cXyz m0F8[3];                    // RIGHT anchors: [0] rope top [1] edge [2] edge-30
    cXyz m11C[3];                    // LEFT anchors
    // §429d receiver-proven 3Dline shape: ONE line per mat, width-variant
    // update (d_a_obj_rope_bridge:376/583) -- the donor's 4-line mat + the
    // per-vertex-size update are unexercised on the PC path and were the
    // 190519 FIFO crash. Cut-half lines dropped with the unported cut system.
    mDoExt_3DlineMat1_c mHanger[2];  // [0]=left [1]=right, 1 line x 5 pts each
    bool mLineInit;
};

class daExtSpan_c : public fopAc_ac_c {
public:
    int create();
    int execute();
    int draw();
    int deleteMe();

    J3DModel* mpPlanks[kMaxPlanks];
    int mCount;
    request_of_phase_process_class mPhase;
    bool mBuilt;
    u8* mpDzbCopy;  // §429b per-actor dzb (see create)
    int m02DD;      // §429g real planks (aite bit1: 15/11/7 -- the authored GAP)

    // §429 donor bridge_class subset
    SpanPlank mSim[kMaxPlanks];
    dBgWSv* mpBgW;
    mDoExt_3DlineMat1_c mRail[2];    // §429d main handrails, 1 line x 14 pts each
    bool mLineMatInit;
    cXyz mHomePos;
    cXyz mEndPos;
    s16 mHomeYaw;
    s16 m02EC, m02EE;                // phase accumulators
    f32 m02E0, m02E4;                // rider energy (vert / lateral)
    f32 m02F4, m02F8, m02FC;         // sway amplitudes
    s16 m0300;                       // wobble phase
    int m030C;                       // post count (handrail segments)
};

// donor d_a_bridge.cpp:22-26 statics
static cXyz* s_spanWindVec;
static s16 s_spanWy;
static f32 s_spanWp;
static const f32 ita_z_p[11] = {0.1f, 0.3f, 0.5f, 0.75f, 0.9f, 1.0f,
                                0.9f, 0.75f, 0.5f, 0.3f, 0.1f};

// ============================================================================
// §429 ride_call_back (donor :29-124, player + default riders only). Fired by
// dBgW when an actor stands on the span collision.
// ============================================================================
static void daExtSpan_rideCallback(dBgW* i_bgw, fopAc_ac_c* i_self, fopAc_ac_c* i_rider) {
    daExtSpan_c* span = (daExtSpan_c*)i_self;
    if (span == NULL || i_rider == NULL || span->mCount <= 0) {
        return;
    }
    // §433-P53: proves the ride callback fires while crossing (collision
    // continuity witness) -- silent stretches while walking = collision hole.
    {
        static u32 s_p53 = 0;
        if ((++s_p53 % 120) == 1) {
            DuskLog.info("[ExtSpan] 433-P53 ride: rider=({}, {}, {}) span@({}, {})",
                         i_rider->current.pos.x, i_rider->current.pos.y,
                         i_rider->current.pos.z, span->mHomePos.x, span->mHomePos.z);
        }
    }
    const f32 dx = i_rider->current.pos.x - span->mSim[0].mPosition.x;
    const f32 dz = i_rider->current.pos.z - span->mSim[0].mPosition.z;
    int idx = (int)(std::sqrt(dx * dx + dz * dz) / 76.5f + 0.5f);
    if (idx < 0) idx = 0;
    if (idx >= span->mCount) idx = span->mCount - 1;
    SpanPlank* pl = &span->mSim[idx];

    f32 weight, depth;
    if (i_rider == (fopAc_ac_c*)dComIfGp_getPlayer(0)) {
        weight = 100.0f;
        depth = -31.0f;
    } else {
        weight = 50.0f;
        depth = -10.0f;
    }
    // rider lateral offset in plank-local space -> roll target
    cXyz local;
    const f32 sy = cM_ssin((s16)-pl->mRotation.y);
    const f32 cy = cM_scos((s16)-pl->mRotation.y);
    const f32 rx = i_rider->current.pos.x - pl->mPosition.x;
    const f32 rz = i_rider->current.pos.z - pl->mPosition.z;
    local.x = cy * rx + sy * rz;
    pl->m400 = (s16)(-local.x * weight);
    pl->m3F4 = depth + span->m02FC * cM_ssin(span->m0300) * 0.03f * weight;
    pl->m406 = 2;
    i_rider->speed.y = -5.0f;  // glue

    // movement energy (donor :105-123, k=1.0 rope type)
    const f32 dmove = std::sqrt(i_rider->speed.abs2());
    f32 e = dmove * 0.3f;
    if (e > span->m02E0) span->m02E0 = e > 20.0f ? 20.0f : e;
    f32 lat = local.x < 0.0f ? -local.x : local.x;
    if (lat > 50.0f) lat = 50.0f;
    cLib_addCalc2(&span->m02E4, lat, 1.0f, 0.5f);
}

int daExtSpan_c::create() {
    if (!mBuilt) {
        fopAcM_ct(this, daExtSpan_c);
        mCount = 0;
        mBuilt = true;
        std::memset(mpPlanks, 0, sizeof(mpPlanks));

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
    }

    // §45: arc + plank model come from bridges.ini (refuse if missing — no
    // hardcoded donor names). Phase-load like the donor; acquireModelData only
    // wraps getObjectRes and cannot load the arc itself.
    loadSpans();
    if (s_plankArc.empty() || s_plankModel.empty()) {
        DuskLog.warn("[ExtSpan] §45 refuse — bridges.ini missing plank_arc/plank_model");
        return cPhs_COMPLEATE_e;
    }
    const int phase = dComIfG_resLoad(&mPhase, s_plankArc.c_str());
    if (phase != cPhs_COMPLEATE_e) {
        return phase;  // NEXT/LOADING — stay alive and retry, never fail
    }

    // №126: NEVER return cPhs_ERROR_e past this point. Fail INERT instead.
    if (s_spans.empty()) {
        return cPhs_COMPLEATE_e;
    }

    // Claim the span whose start is nearest this placement. The donor selects
    // by path id from its param word; that word does not reach us, and the two
    // Outset spans are far apart, so nearest-start is unambiguous here.
    const Span* best = NULL;
    f32 bestDist = 0.0f;
    for (const Span& s : s_spans) {
        const f32 dx = s.start.x - current.pos.x;
        const f32 dy = s.start.y - current.pos.y;
        const f32 dz = s.start.z - current.pos.z;
        const f32 d = dx * dx + dy * dy + dz * dz;
        if (best == NULL || d < bestDist) {
            best = &s;
            bestDist = d;
        }
    }

    const cXyz delta(best->end.x - best->start.x, best->end.y - best->start.y,
                     best->end.z - best->start.z);
    const f32 len = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (len < 1.0f) {
        return cPhs_COMPLEATE_e;
    }

    // Donor spacing formula, unchanged.
    const f32 pad = (len > 1300.0f) ? 3.0f : 0.0f;
    int count = (int)(len / ((pad + 47.0f) * 1.5f));
    if (count < 1) {
        count = 1;
    }
    if (count >= kMaxPlanks) {
        count = kMaxPlanks - 1;
    }
    // ========================================================================
    // §429g THE MISSING PLANKS ARE AUTHORED (user report + donor receipt):
    // Room44.dzr places both bridges with typeBits 0x0A -- bit1 = "aite"
    // broken-pair half bridge (donor daBridge_Create :1505-1511: draws only
    // 15/11/7 real planks; sim + ropes + rail continue across the gap; the
    // two halves meet mid-crossing). bit3 = txm_rope1 thick ropes (rope pass).
    // ========================================================================
    m02DD = count;
    if (s_spanType & 2) {
        if (count >= 16)      m02DD = 15;
        else if (count >= 12) m02DD = 11;
        else                  m02DD = 7;
    }

    J3DModelData* modelData =
        dExtNpcMount_acquireModelData(s_plankArc.c_str(), s_plankModel.c_str());
    if (modelData == NULL) {
        DuskLog.warn("[ExtSpan] №124 missing/unparseable {} in '{}' — inert", s_plankModel,
                     s_plankArc);
        return cPhs_COMPLEATE_e;
    }

    const f32 horiz = std::sqrt(delta.x * delta.x + delta.z * delta.z);
    const s16 yaw = cM_atan2s(delta.x, delta.z);
    const s16 pitch = -cM_atan2s(delta.y, horiz);

    for (int i = 0; i < count; ++i) {
        J3DModel* m = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
        if (m == NULL) {
            break;
        }
        // Planks sit at segment CENTRES, so the run starts and ends flush with
        // the anchor points instead of overhanging by half a plank.
        const f32 t = (i + 0.5f) / (f32)count;
        mDoMtx_stack_c::transS(best->start.x + delta.x * t, best->start.y + delta.y * t,
                               best->start.z + delta.z * t);
        mDoMtx_stack_c::YrotM(yaw);
        mDoMtx_stack_c::XrotM(pitch);
        m->setBaseTRMtx(mDoMtx_stack_c::get());
        mpPlanks[mCount++] = m;
    }

    if (mCount == 0) {
        return cPhs_COMPLEATE_e;
    }

    // ========================================================================
    // §429 DONOR SIM INIT (heap CB :1315-1444 + mode-2 :646-655): posts every
    // 4th plank (rope phase i==2 mod 4), donor scales, span profile, chain
    // nodes seeded on the straight span (sim converges in a few frames),
    // handrail + hanger 3Dlines (rope.bti = staged WwAlways 0x7E), and the
    // ONE deformable dBgWSv from the donor's own mbrdg.dzb (4 verts/plank).
    // ========================================================================
    mHomePos = best->start;
    mEndPos = best->end;
    mHomeYaw = yaw;
    m02EC = m02EE = m0300 = 0;
    m02E0 = m02E4 = 0.0f;
    m02F4 = m02F8 = m02FC = 0.0f;
    m030C = 0;
    ResTIMG* ropeImg = (ResTIMG*)dComIfG_getObjectRes("WwAlways", (int)0x7E);
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        const f32 t = (i + 0.5f) / (f32)mCount;
        pl->m3CC.set(best->start.x + delta.x * t, best->start.y + delta.y * t,
                     best->start.z + delta.z * t);
        pl->mPosition = pl->m3CC;
        pl->mRotation.set(pitch, yaw, 0);
        pl->mRotationYExtra = (cM_rndF(1.0f) < 0.5f) ? (s16)-0x8000 : (s16)0;
        pl->m3F0 = mCount > 1 ? std::fabs(std::sin(3.14159265f * i / (f32)(mCount - 1))) : 1.0f;
        pl->m3F4 = pl->m3F8 = 0.0f;
        pl->m3FC = -15.0f;
        pl->m400 = pl->m402 = 0;
        pl->m406 = 0;
        pl->m408 = 0;
        pl->mLineInit = false;
        pl->mScale.set(cM_rndF(0.1f) + 1.0f, cM_rndF(0.3f) + 1.0f, 1.5f);
        if (((i + 2) & 3) == 0) {
            pl->m408 = 7;
            pl->mScale.x = 1.05f;
            if (ropeImg != NULL && pl->mHanger[0].init(1, 5, ropeImg, 1) &&
                pl->mHanger[1].init(1, 5, ropeImg, 1)) {
                pl->mLineInit = true;
            }
        }
        if (mpPlanks[i] != NULL) {
            mpPlanks[i]->setBaseScale(pl->mScale);
        }
    }
    mLineMatInit = false;
    if (ropeImg != NULL && mRail[0].init(1, 14, ropeImg, 1) && mRail[1].init(1, 14, ropeImg, 1)) {
        mLineMatInit = true;
    }
    if (ropeImg == NULL) {
        DuskLog.warn("[ExtSpan] 429 rope.bti (WwAlways 0x7E) not resident -- ropes disabled");
    }

    mpBgW = NULL;
    // ========================================================================
    // §429b PER-ACTOR DZB COPY (crash 184254): TWO spans Set() and rewrote the
    // SAME shared raw arc buffer -- each frame one actor's vertex writes
    // clobbered the other's collision (and OffsetPtr relocation mutates the
    // shared buffer). Same law as sumo-BMT/§418b: never share a fixed-up
    // buffer. Each actor owns a pristine copy. Size receipt: mbrdg.dzb is
    // 9312 bytes in the donor arc (verified against D:\XXXXXXX and the staged
    // copy, 2026-08-04); header cross-check below refuses anything bigger.
    // ========================================================================
    cBgD_t* dzb = NULL;
    {
        void* raw = dComIfG_getObjectRes(s_plankArc.c_str(), "mbrdg.dzb");
        if (raw != NULL) {
            static const u32 kMbrdgSize = 9312;
            mpDzbCopy = new u8[kMbrdgSize];
            if (mpDzbCopy != NULL) {
                std::memcpy(mpDzbCopy, raw, kMbrdgSize);
                dzb = (cBgD_t*)mpDzbCopy;
                // §429f (user report: wrong surface type): the WW dzb carries
                // WW attCodes in bits TP reads as sink/void classes -- run the
                // §334 per-code translation, same as every staged room dzb.
                dExtWw_repackDzbAttributes(dzb, "mbrdg.dzb");
            }
        }
    }
    if (dzb != NULL) {
        mpBgW = new dBgWSv();
        if (mpBgW != NULL && mpBgW->Set(dzb, 0) == 0) {
            mpBgW->SetRideCallback(daExtSpan_rideCallback);
            mpBgW->CopyBackVtx();
            cBgD_Vtx_t* vtx = mpBgW->GetVtxTbl();
            for (int i = 0; i < mpBgW->GetVtxNum(); ++i) {
                vtx[i].x = mHomePos.x;
                vtx[i].y = mHomePos.y;
                vtx[i].z = mHomePos.z;
            }
            mpBgW->Move();
            if (dComIfG_Bgsp().Regist(mpBgW, this)) {
                DuskLog.warn("[ExtSpan] 429 bgW Regist FAILED -- span not walkable");
                delete mpBgW;
                mpBgW = NULL;
            } else {
                DuskLog.info("[ExtSpan] 429 deformable collision LIVE (mbrdg.dzb, {} verts)",
                             mpBgW->GetVtxNum());
            }
        } else {
            DuskLog.warn("[ExtSpan] 429 dBgWSv Set failed -- span not walkable");
            delete mpBgW;
            mpBgW = NULL;
        }
    } else {
        DuskLog.warn("[ExtSpan] 429 mbrdg.dzb missing from '{}' -- span not walkable",
                     s_plankArc);
    }

    // The span is ~1300 units and sits far from the actor origin; never let the
    // frustum cull it on the actor's own point.
    fopAcM_SetMin(this, -2000.0f, -2000.0f, -2000.0f);
    fopAcM_SetMax(this, 2000.0f, 2000.0f, 2000.0f);
    fopAcM_OffStatus(this, fopAcStts_CULL_e);
    dKy_tevstr_init(&tevStr, fopAcM_GetRoomNo(this), 0xFF);
    tevStr.room_no = fopAcM_GetRoomNo(this);

    DuskLog.info("[ExtSpan] №124 span len={:.1f} -> {} planks at ({:.0f},{:.0f},{:.0f})", len,
                 mCount, best->start.x, best->start.y, best->start.z);
    return cPhs_COMPLEATE_e;
}

// ============================================================================
// §429 forward pass -- donor control1 (:388-447): wind + traveling waves fold
// into each 75.0-unit segment step.
// ============================================================================
static void daExtSpan_control1(daExtSpan_c* sp) {
    const s16 stride = (sp->mCount > 10) ? (s16)4000 : (s16)8000;
    cXyz swayOfs;  // lateral sway, world
    {
        const f32 amp = sp->m02F8 * cM_scos(sp->m02EC);
        swayOfs.set(amp * cM_scos(sp->mHomeYaw), 0.0f, amp * -cM_ssin(sp->mHomeYaw));
    }
    cXyz lat(cM_scos(sp->mHomeYaw), 0.0f, -cM_ssin(sp->mHomeYaw));  // bridge-lateral unit
    cXyz windOfs;
    windOfs.set(s_spanWp * 5.0f * cM_ssin(s_spanWy), 0.0f, s_spanWp * 5.0f * cM_scos(s_spanWy));

    for (int i = 1; i < sp->mCount; ++i) {
        SpanPlank* pl = &sp->mSim[i];
        SpanPlank* pv = &sp->mSim[i - 1];
        const f32 tmp = pl->m3F8 * 0.5f + (pl->m3FC * pl->m3F0 * 0.5f + pl->m3CC.y);
        const f32 wave = sp->m02F4 * cM_ssin((s16)(sp->m02EC + i * stride)) * pl->m3F0;
        const f32 vwave = sp->m02FC * cM_ssin((s16)(sp->m02EE + i * (stride + 1000))) * pl->m3F0;
        const f32 x = (pl->m3CC.x - pv->m3CC.x) + wave * lat.x + swayOfs.x * pl->m3F0 + windOfs.x;
        const f32 y = vwave + (tmp - pv->m3CC.y);
        const f32 z = (pl->m3CC.z - pv->m3CC.z) + wave * lat.z + swayOfs.z * pl->m3F0 + windOfs.z;
        const s16 yaw = cM_atan2s(x, z);
        const s16 pit = (s16)-cM_atan2s(y, std::sqrt(x * x + z * z));
        const f32 ch = 75.0f * cM_scos(pit);
        pl->m3CC.set(pv->m3CC.x + ch * cM_ssin(yaw), pv->m3CC.y - 75.0f * cM_ssin(pit),
                     pv->m3CC.z + ch * cM_scos(yaw));
    }
}

// donor control2 (:449-482) -- BACKWARD pass; writes plank yaw/pitch.
static void daExtSpan_control2(daExtSpan_c* sp) {
    for (int i = sp->mCount - 2; i >= 0; --i) {
        SpanPlank* pl = &sp->mSim[i];
        SpanPlank* nx = &sp->mSim[i + 1];
        const f32 tmp = pl->m3F8 * 0.5f + (pl->m3FC * pl->m3F0 * 0.5f + pl->m3CC.y);
        const f32 x = pl->m3CC.x - nx->m3CC.x;
        const f32 y = tmp - nx->m3CC.y;
        const f32 z = pl->m3CC.z - nx->m3CC.z;
        const s16 yaw = cM_atan2s(x, z);
        const s16 pit = (s16)-cM_atan2s(y, std::sqrt(x * x + z * z));
        nx->mRotation.y = yaw;
        nx->mRotation.x = pit;
        const f32 ch = 75.0f * cM_scos(pit);
        pl->m3CC.set(nx->m3CC.x + ch * cM_ssin(yaw), nx->m3CC.y - 75.0f * cM_ssin(pit),
                     nx->m3CC.z + ch * cM_scos(yaw));
    }
}

// donor control3 (:485-492) -- plank 0 orientation from node0 -> node1.
static void daExtSpan_control3(daExtSpan_c* sp) {
    if (sp->mCount < 2) {
        return;
    }
    const cXyz d = sp->mSim[1].m3CC - sp->mSim[0].m3CC;
    sp->mSim[0].mRotation.y = cM_atan2s(d.x, d.z);
    sp->mSim[0].mRotation.x = (s16)-cM_atan2s(d.y, std::sqrt(d.x * d.x + d.z * d.z));
}

int daExtSpan_c::execute() {
    if (mCount == 0) {
        return 1;
    }
    // donor Execute :882-884 -- wind statics
    s_spanWindVec = dKyw_get_wind_vec();
    s_spanWy = cM_atan2s(s_spanWindVec->x, s_spanWindVec->z);
    s_spanWp = dKyw_get_wind_pow();

    // phases (donor :780-793 order simplified: amplitudes first, then step)
    m02FC = m02E0;
    m02F4 = m02E0;
    m02F8 = m02E4;
    m02EC += 0x578;
    m02EE += 3000;
    m0300 += 3000;

    // the relaxation (donor bridge_move case 3 :657-691)
    mSim[0].m3CC = mHomePos;
    daExtSpan_control1(this);
    mSim[mCount - 1].m3CC = mEndPos;
    daExtSpan_control2(this);
    daExtSpan_control3(this);
    {
        const cXyz resid = mHomePos - mSim[0].m3CC;
        for (int i = 0; i < mCount; ++i) {
            const f32 k = ((f32)(mCount - i) / (f32)mCount) * 0.75f;
            mSim[i].mPosition.set(mSim[i].m3CC.x + resid.x * k, mSim[i].m3CC.y + resid.y * k,
                                  mSim[i].m3CC.z + resid.z * k);
        }
    }

    // rider springs over +/-5 neighbors (donor :693-703) + decays (:765-773)
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        if (pl->m406 != 0) {
            for (int j = -5; j <= 5; ++j) {
                const int k = i + j;
                if (k < 0 || k >= mCount) continue;
                SpanPlank* nb = &mSim[k];
                cLib_addCalcAngleS2(&nb->m402, (s16)(pl->m400 * ita_z_p[5 + j] * nb->m3F0), 4,
                                    0x800);
                cLib_addCalc2(&nb->m3F8, pl->m3F4 * ita_z_p[5 + j], 1.0f, 10.0f);
            }
        }
    }
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        if (pl->m406 != 0) pl->m406--;
        pl->m400 = 0;
        pl->mRotation.z = pl->m402;
        cLib_addCalcAngleS2(&pl->m402, 0, 4, 0x400);
        cLib_addCalc2(&pl->m3FC, -15.0f, 1.0f, 5.0f);
        cLib_addCalc0(&pl->m3F8, 1.0f, 5.0f);
        pl->m3F4 = 0.0f;
    }
    // wind floor (donor :787-793)
    {
        const f32 t = (s_spanWp > 0.1f) ? 2.0f : 0.0f;
        cLib_addCalc2(&m02E0, t, 0.1f, 0.1f);
        cLib_addCalc2(&m02E4, t * 0.3f, 0.1f, 0.05f);
    }

    // matrices + anchors (donor :902-921)
    m030C = 0;
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        mDoMtx_stack_c::transS(pl->mPosition.x, pl->mPosition.y, pl->mPosition.z);
        mDoMtx_stack_c::YrotM(pl->mRotation.y);
        mDoMtx_stack_c::XrotM(pl->mRotation.x);
        mDoMtx_stack_c::ZrotM(pl->mRotation.z);
        // anchors BEFORE the cosmetic 180 flip (donor order)
        cXyz v, o;
        v.set(-pl->mScale.x * 99.0f, 0.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m11C[1] = o;
        v.set(-pl->mScale.x * 99.0f, -30.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m11C[2] = o;
        v.set(pl->mScale.x * 99.0f, 0.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m0F8[1] = o;
        v.set(pl->mScale.x * 99.0f, -30.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m0F8[2] = o;
        pl->m11C[0] = pl->m11C[1];
        pl->m11C[0].y += 200.0f;
        pl->m0F8[0] = pl->m0F8[1];
        pl->m0F8[0].y += 200.0f;
        if (pl->m408 & 4) {
            m030C++;
        }
        mDoMtx_stack_c::YrotM(pl->mRotationYExtra);
        if (mpPlanks[i] != NULL) {
            mpPlanks[i]->setBaseTRMtx(mDoMtx_stack_c::get());
        }
    }

    // collision vertex writer (donor :1170-1243, no snap mode)
    if (mpBgW != NULL) {
        mpBgW->CopyBackVtx();
        cBgD_Vtx_t* vtx = mpBgW->GetVtxTbl();
        const int nv = mpBgW->GetVtxNum();
        for (int i = 0; i < nv; ++i) {
            int idx = i >> 2;
            if (idx >= m02DD) idx = m02DD - 1;  // §429g collapse past the gap (donor :1231-1236)
            if (idx >= mCount) idx = mCount - 1;
            const SpanPlank* pl = &mSim[idx];
            const cXyz* src;
            switch (i & 3) {
            case 0: src = &pl->m11C[2]; break;
            case 1: src = &pl->m0F8[2]; break;
            case 2: src = &pl->m11C[1]; break;
            default: src = &pl->m0F8[1]; break;
            }
            cXyz w = *src;
            if (idx == 0 || idx == mCount - 1) {
                // end extension (donor :1212-1230): +/-50 local Z, -40 Y
                const f32 ext = (idx == 0) ? -50.0f : 50.0f;
                w.x += ext * cM_ssin(pl->mRotation.y);
                w.z += ext * cM_scos(pl->mRotation.y);
                if ((i & 3) < 2) w.y -= 40.0f;
            }
            vtx[i].x = w.x;
            vtx[i].y = w.y;
            vtx[i].z = w.z;
        }
        mpBgW->Move();
        // §433-P56/P58: collision footprint + gap accounting, once per span.
        static u32 s_p56 = 0;
        if ((++s_p56 % 1200) == 1) {
            f32 yMin = 1e9f, yMax = -1e9f, xMin = 1e9f, xMax = -1e9f;
            cBgD_Vtx_t* vt = mpBgW->GetVtxTbl();
            for (int i = 0; i < mpBgW->GetVtxNum(); ++i) {
                if (vt[i].y < yMin) yMin = vt[i].y;
                if (vt[i].y > yMax) yMax = vt[i].y;
                if (vt[i].x < xMin) xMin = vt[i].x;
                if (vt[i].x > xMax) xMax = vt[i].x;
            }
            DuskLog.info("[ExtSpan] 433-P56 verts y=[{}..{}] x=[{}..{}] 433-P58 count={} "
                         "real={} (gap planks {}..{})",
                         yMin, yMax, xMin, xMax, mCount, m02DD, m02DD, mCount - 1);
        }
        // §433-P54: player-over-span telemetry -- position + room stay while
        // near the crossing; correlates a void with room-tracking state.
        {
            static u32 s_p54 = 0;
            fopAc_ac_c* pl = (fopAc_ac_c*)dComIfGp_getPlayer(0);
            if (pl != NULL && (++s_p54 % 300) == 1) {
                const f32 dx = pl->current.pos.x - mSim[mCount / 2].mPosition.x;
                const f32 dz = pl->current.pos.z - mSim[mCount / 2].mPosition.z;
                if (dx * dx + dz * dz < 1400.0f * 1400.0f) {
                    DuskLog.info("[ExtSpan] 433-P54 player near span: pos=({}, {}, {}) "
                                 "stayNo={} speedY={}",
                                 pl->current.pos.x, pl->current.pos.y, pl->current.pos.z,
                                 dComIfGp_roomControl_getStayNo(), pl->speed.y);
                }
            }
        }
    }
    return 1;
}

int daExtSpan_c::deleteMe() {
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
        mpBgW = NULL;
    }
    if (mpDzbCopy != NULL) {
        delete[] mpDzbCopy;  // §429b
        mpDzbCopy = NULL;
    }
    return 1;
}

int daExtSpan_c::draw() {
    if (mCount == 0) {
        return 1;
    }
    // ========================================================================
    // §425 LIGHTING-CONTRACT FIX (Foundry kit_laws law-1 finding): this WW
    // port was lit through the TP path (hand-picked 0x40 + _MAJI) — the exact
    // black-actor class §405-§407 closed. Donor authors TEV_TYPE_BG0
    // (WW d_a_bridge.cpp:1244); native chain feeds C0/K0 the donor way.
    // ========================================================================
    dKyWw_settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    for (int i = 0; i < mCount; ++i) {
        if (i >= m02DD) {
            break;  // §429g authored gap: planks beyond the aite count do not draw
        }
        if (mpPlanks[i] == NULL) {
            continue;
        }
        mpPlanks[i]->calc();
        dKyWw_setLightTevColorType(mpPlanks[i], &tevStr);
        mDoExt_modelUpdateDL(mpPlanks[i]);
    }

    // ========================================================================
    // §429 ROPES (donor Draw :144-386): hanger pairs on each post (4 lines x
    // 5 pts, per-vertex width 3; cut-halves collapsed -- cut system owed) and
    // the two main handrails (14 segs, width 4, interior points = post rope
    // tops). Color {150,150,150,255}; xlu 3Dline sort packet.
    // ========================================================================
    // ========================================================================
    // §429e BISECT SWITCH: three warp crashes; runs 2 and 3 both fatal'd in
    // the GX FIFO immediately after '429c ropes LIVE', but the rope path now
    // matches proven users exactly, so this build isolates it empirically.
    // false = sim + collision only. If the warp SURVIVES, ropes are convicted
    // and get a dedicated debug pass; if it still crashes, the sim/collision
    // side is guilty and ropes are exonerated.
    // ========================================================================
    static const bool kExtSpanRopes = false;  // §434: OFF -- the 3Dline material
    // path is a PLATFORM bug (aurora vs static GC DLs), documented on the bus
    // with five crash receipts; owed to a dedicated aurora-lane session.
    if (!kExtSpanRopes) {
        return 1;
    }
    // §429c lazy rope init: bridges create BEFORE WwAlways is resident
    // (log 184254: "rope.bti not resident"). Retry here until it lands.
    if (!mLineMatInit) {
        ResTIMG* img = (ResTIMG*)dComIfG_getObjectRes("WwAlways", (int)0x7E);
        if (img != NULL) {
            if (mRail[0].init(1, 14, img, 1) && mRail[1].init(1, 14, img, 1)) {
                mLineMatInit = true;
            }
            for (int i = 0; i < mCount; ++i) {
                if ((mSim[i].m408 & 4) && !mSim[i].mLineInit &&
                    mSim[i].mHanger[0].init(1, 5, img, 1) &&
                    mSim[i].mHanger[1].init(1, 5, img, 1)) {
                    mSim[i].mLineInit = true;
                }
            }
            if (mLineMatInit) {
                DuskLog.info("[ExtSpan] 429c ropes LIVE (lazy init)");
            }
        }
    }
    GXColor ropeCol;
    ropeCol.r = ropeCol.g = ropeCol.b = 150;
    ropeCol.a = 255;
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        if (!(pl->m408 & 4) || !pl->mLineInit) {
            continue;
        }
        for (int side = 0; side < 2; ++side) {
            const cXyz& lo = side == 0 ? pl->m11C[1] : pl->m0F8[1];
            const cXyz& hi = side == 0 ? pl->m11C[0] : pl->m0F8[0];
            cXyz* pts = pl->mHanger[side].getPos(0);
            for (int k = 0; k < 5; ++k) {
                const f32 t = k / 4.0f;
                pts[k].set(lo.x + (hi.x - lo.x) * t, lo.y + (hi.y - lo.y) * t,
                           lo.z + (hi.z - lo.z) * t);
            }
            pl->mHanger[side].update(5, 3.0f, ropeCol, 0, &tevStr);
            dComIfGd_set3DlineMat(&pl->mHanger[side]);
        }
    }
    if (mLineMatInit && mCount > 0) {
        // endpoints local (-/+120, 350, -/+40) about home/end (donor :315-383)
        const int segs = m030C + 2 <= 14 ? m030C + 2 : 14;
        for (int side = 0; side < 2; ++side) {
            cXyz* pts = mRail[side].getPos(0);
            const f32 lx = side == 0 ? -120.0f : 120.0f;
            const f32 lz = side == 0 ? -40.0f : 40.0f;
            cXyz o;
            o.set(lx * cM_scos(mHomeYaw) + lz * cM_ssin(mHomeYaw), 350.0f,
                  -lx * cM_ssin(mHomeYaw) + lz * cM_scos(mHomeYaw));
            pts[0] = mHomePos + o;
            pts[segs - 1] = mEndPos + o;
            int seg = 1;
            for (int i = 0; i < mCount && seg < segs - 1; ++i) {
                if (mSim[i].m408 & 4) {
                    pts[seg++] = side == 0 ? mSim[i].m11C[0] : mSim[i].m0F8[0];
                }
            }
            for (; seg < segs - 1; ++seg) {
                pts[seg] = pts[segs - 1];
            }
            mRail[side].update(segs, 4.0f, ropeCol, 0, &tevStr);
            dComIfGd_set3DlineMat(&mRail[side]);
        }
    }
    return 1;
}

// ==========================================================================
// Profile
// ==========================================================================
static int daExtSpan_Create(fopAc_ac_c* i_this) {
    return static_cast<daExtSpan_c*>(i_this)->create();
}
static int daExtSpan_Delete(void* i_this) {
    return static_cast<daExtSpan_c*>(i_this)->deleteMe();
}
static int daExtSpan_Execute(void* i_this) {
    return static_cast<daExtSpan_c*>(i_this)->execute();
}
static int daExtSpan_Draw(void* i_this) {
    return static_cast<daExtSpan_c*>(i_this)->draw();
}
static int daExtSpan_IsDelete(void* i_this) {
    (void)i_this;
    return 1;
}

static DUSK_CONST actor_method_class l_daExtSpan_Method = {
    (process_method_func)daExtSpan_Create,  (process_method_func)daExtSpan_Delete,
    (process_method_func)daExtSpan_Execute, (process_method_func)daExtSpan_IsDelete,
    (process_method_func)daExtSpan_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_EXT_SPAN = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_EXT_SPAN_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daExtSpan_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daExtSpan_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#endif  // TARGET_PC
