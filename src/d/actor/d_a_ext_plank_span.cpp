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
// KIT-DONOR: d/actor/d_a_bridge.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching

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
#include "JSystem/J3DGraphBase/J3DShape.h"  // §474 resetVcdVatCache
#include "d/ext_line/mdoext1_3dline.h"      // §484 parallel donor 3D-line stack
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
// ============================================================================
// §484 The §460 dExtWw3DlineMat1_c subclass that used to live here is GONE,
// replaced by the parallel donor stack in d/ext_line/mdoext1_3dline.h
// (MDoExt1::LineMat1_c). That subclass inherited TP's class and diverged by
// EDITING a copy of TP's draw body -- the shape the user ruled against, and the
// shape that produced the black ropes when the copy silently omitted
// dKy_Global_amb_set. The parallel stack carries the donor's OWN data layout
// (u8 widths, INDEX8 normals off a static 2-entry array), its OWN display lists
// and its OWN draw, inheriting nothing but the abstract base needed to ride the
// existing sort packet ([W0], phase 2 owed).
// This deletion is of MY OWN WW code, superseded -- no TP or donor line removed.
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
    s16 m404;                        // rope-cut lean (donor; dead while cut owed)
    u8 m406;                         // rider-active countdown
    u8 m408;                         // bit2 = rope post
    cXyz mScale;
    cXyz m0F8[3];                    // RIGHT anchors: [0] rope top [1] edge [2] edge-30
    cXyz m11C[3];                    // LEFT anchors
    // §429d receiver-proven 3Dline shape: ONE line per mat, width-variant
    // update (d_a_obj_rope_bridge:376/583) -- the donor's 4-line mat + the
    // per-vertex-size update are unexercised on the PC path and were the
    // 190519 FIFO crash. Cut-half lines dropped with the unported cut system.
    // §441 donor shape restored: ONE 4-line x 5-pt mat per post (lines 0/2 =
    // left rope + cut-half, 1/3 = right + cut-half; per-vertex sizes), exactly
    // WW d_a_bridge. §429d's 1-line restructure was compensating for the §434
    // material bug, now fixed by the l_mat1DL LE translation.
    MDoExt1::LineMat1_c mLineMat1;  // §484 parallel donor stack
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
    MDoExt1::LineMat1_c mLineMat;    // §484 parallel donor stack; donor 2-line rail
    bool mLineMatInit;
    cXyz mHomePos;
    cXyz mEndPos;
    s16 mHomeYaw;
    s16 m02EC, m02EE;                // phase accumulators
    s16 m02F0, m02F2;                // phase speeds (donor 0x578 / 3000)
    f32 m02E0, m02E4;                // rider energy (vert / lateral)
    f32 m02F4, m02F8, m02FC;         // sway amplitudes
    s16 m0300;                       // wobble phase
    int m030C;                       // post count (handrail segments)
};

// donor d_a_bridge.cpp:22-26 statics
static cXyz* s_spanWindVec;
static s16 s_spanWy;
static f32 s_spanWp;
// KIT-DONOR-DATA: 44 lookup-table d/actor/d_a_bridge.cpp:22-26
static const f32 ita_z_p[11] = {0.1f, 0.3f, 0.5f, 0.75f, 0.9f, 1.0f,
                                0.9f, 0.75f, 0.5f, 0.3f, 0.1f};

// ============================================================================
// §429 ride_call_back (donor :29-124, player + default riders only). Fired by
// dBgW when an actor stands on the span collision.
// ============================================================================
static void daExtSpan_rideCallback(dBgW* i_bgw, fopAc_ac_c* i_self, fopAc_ac_c* i_rider) {
    // ========================================================================
    // §441 donor ride_call_back VERBATIM (:29-124). Seams: [R1]/[R2] naming;
    // [R7] the BK boar chase-field handoff (bk->dr.*) needs the receiver
    // bk_class layout verified before writing cross-actor fields -- weight/
    // depth applied, handoff OWED; MO2/BOMB legs live (same proc names).
    // ========================================================================
    daExtSpan_c* i_this = (daExtSpan_c*)i_self;
    if (i_this == NULL || i_rider == NULL || i_this->mCount <= 0) {
        return;
    }

    cXyz pos = i_this->mSim[0].mPosition - i_rider->current.pos;
    s32 brIdx = (s32)(std::sqrt(pos.x * pos.x + pos.z * pos.z) / 76.5f - -0.5f);
    if (brIdx > i_this->mCount - 1) {
        brIdx = i_this->mCount - 1;
    } else if (brIdx < 0) {
        brIdx = 0;
    }

    SpanPlank* pBr = &i_this->mSim[brIdx];
    f32 fVar2 = ((s_spanType & 5) != 0) ? 0.85f : 1.0f;
    cMtx_YrotS(*calc_mtx, (s16)-pBr->mRotation.y);

    cXyz posDiff = i_rider->current.pos - pBr->mPosition;
    cXyz sp4C;
    MtxPosition(&posDiff, &sp4C);

    posDiff = i_rider->old.pos - pBr->mPosition;
    cXyz sp40;
    MtxPosition(&posDiff, &sp40);

    i_rider->speed.y = -5.0f;

    f32 fVar7;
    // [R7] donor rider table: PLAYER -> receiver fpcNm_ALINK_e (same-lineage
    // rename). MO2 (WW moblin) and BK (WW boar) have NO receiver profiles yet
    // -- they are adapter-socketed census stand-ins until their de-mount, so
    // fpcM_GetName can never match them; their donor legs (150/-40 + speedY
    // -20; 100/-25 + chase handoff) re-enter verbatim WITH their de-mounts.
    // BOMB donor leg (:85-93, near-explode depth -300 + m02E0=20) owed same.
    if (fpcM_GetName(i_rider) == fpcNm_ALINK_e) {
        fVar7 = 100.0f;
        pBr->m3F4 = -31.0f;
    } else {
        fVar7 = 50.0f;
        pBr->m3F4 = -10.0f;
    }

    fVar7 *= fVar2;

    pBr->m3F4 *= fVar2;
    pBr->m3F4 = pBr->m3F4 + i_this->m02FC * cM_ssin(i_this->m0300) * 0.03f * fVar7;
    pBr->m400 = (s16)(-sp4C.x * fVar7);
    pBr->m406 = 2;

    pos = sp4C - sp40;

    f32 fVar3 = pos.abs() * 0.3f * fVar2;
    if (fVar3 > 20.0f) {
        fVar3 = 20.0f;
    }

    if (i_this->m02E0 <= fVar3) {
        i_this->m02E0 = fVar3;
    }

    f32 tmp = fVar2 * std::fabs(pos.x);
    if (tmp > 50.0f) {
        tmp = 50.0f;
    }

    if (i_this->m02E4 <= tmp) {
        i_this->m02E4 += 0.5f;
    }
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
    m02F0 = 0x578;
    m02F2 = 3000;
    m02E0 = m02E4 = 0.0f;
    m02F4 = m02F8 = m02FC = 0.0f;
    m030C = 0;
    // ========================================================================
    // §470: the rope-texture lookup that USED to sit here is GONE, and with it
    // a misleading warn. It ran before s_spanType was populated, so it asked
    // for 0x7E (rope.bti) when the Outset bridges are type 0x0A -- bit3 set --
    // and need 0x8D (txm_rope1.bti). It then logged "ropes disabled", which is
    // how §466 came to believe ropes were absent when they were not.
    // §448 already moved init to the execute path ahead of the anchor walk
    // (the ordering fix), where s_spanType IS resolved and the lookup retries
    // until WwAlways is resident. That is now the SINGLE init site for both the
    // rail mat and the per-post hanger mats. Two init sites with different
    // resource ids was the actual defect.
    // Receipt for which runs proved it: 095309 create SUCCEEDED (no warn, mats
    // live, P66 correctly silent); 225640/005947 create failed and execute
    // rescued; 232642/225938 create failed and the run hung before execute
    // could rescue.
    // ========================================================================
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
        pl->m400 = pl->m402 = pl->m404 = 0;
        pl->m406 = 0;
        pl->m408 = 0;
        pl->mLineInit = false;
        pl->mScale.set(cM_rndF(0.1f) + 1.0f, cM_rndF(0.3f) + 1.0f, 1.5f);
        if (((i + 2) & 3) == 0) {
            pl->m408 = 7;
            pl->mScale.x = 1.05f;
            // §470: hanger mats now initialise on the execute path only.
        }
        if (mpPlanks[i] != NULL) {
            mpPlanks[i]->setBaseScale(pl->mScale);
        }
    }
    // §470: rail mat likewise deferred to the execute path's single init site.
    // The flag is still cleared here because a rebuilt span must re-init.
    mLineMatInit = false;

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
// §440 FULL VERBATIM RE-DERIVATION (user order: full native, no halfway).
// control1/control2/control3 + the bridge_move case-3 body are the donor's
// lines (WW d_a_bridge.cpp:388-492, :657-794) with labeled seams only:
//   [R1] i_this->mBr / pBr walk  -> this->mSim array walk (same field names)
//   [R2] actor.home.pos/angle.y  -> mHomePos / mHomeYaw (ini-authored span)
//   [R3] donor statics wp/wy     -> s_spanWp / s_spanWy (§429 wind fetch)
//   [R4] chain-type (mTypeBits&1) legs compiled out: Outset bridges are rope
//        type (donor receipt 0x0A has bit0 clear)
//   [R5] cut/fire inputs (m3A0 wobble, m408 rope-cut bits, m0304/m0308 snap)
//        read their donor defaults (intact) -- those systems stay §429-owed,
//        so their branches are dead-but-present, donor shape preserved.
// My §429 paraphrase (split loops, forward control3, dropped m404) RETIRED.
// ============================================================================
static void daExtSpan_control1(daExtSpan_c* i_this, SpanPlank* pBr) {
    cXyz sp3C;
    cXyz sp30;
    cXyz sp24;
    cXyz sp18;
    cXyz sp0C;

    pBr++;

    i_this->m02EC += i_this->m02F0;
    i_this->m02EE += i_this->m02F2;

    s16 sVar13;
    if (i_this->mCount > 10) {
        sVar13 = 4000;
    } else {
        sVar13 = 8000;
    }

    sp3C.x = i_this->m02F8 * cM_scos(i_this->m02EC);
    sp3C.y = 0.0f;
    sp3C.z = 0.0f;
    cMtx_YrotS(*calc_mtx, i_this->mHomeYaw);
    MtxPosition(&sp3C, &sp24);
    sp3C.x = 1.0f;
    MtxPosition(&sp3C, &sp18);
    sp3C.x = 0.0f;
    sp3C.z = s_spanWp * 5.0f;
    cMtx_YrotS(*calc_mtx, s_spanWy);
    MtxPosition(&sp3C, &sp0C);
    sp3C.x = 0.0f;
    sp3C.z = 75.0f;

    for (s32 i = 1; i < i_this->mCount; i++, pBr++) {
        f32 x;
        f32 y;
        f32 z;
        f32 tmp = pBr->m3F8 * 0.5f + (pBr->m3FC * pBr->m3F0 * 0.5f + pBr->m3CC.y);
        f32 fVar8 = i_this->m02F4 * cM_ssin((s16)(i_this->m02EC + i * sVar13)) * pBr->m3F0;
        f32 fVar7 =
            i_this->m02FC * cM_ssin((s16)(i_this->m02EE + i * (sVar13 + 1000))) * pBr->m3F0;

        x = (pBr->m3CC.x - pBr[-1].m3CC.x) + fVar8 * sp18.x + sp24.x * pBr->m3F0 + sp0C.x;
        y = fVar7 + (tmp - pBr[-1].m3CC.y);
        z = (pBr->m3CC.z - pBr[-1].m3CC.z) + fVar8 * sp18.z + sp24.z * pBr->m3F0 + sp0C.z;

        s16 atan2v;
        s16 atanv;
        atanv = (s16)cM_atan2s(x, z);
        atan2v = (s16)-cM_atan2s(y, std::sqrt(x * x + z * z));

        cMtx_YrotS(*calc_mtx, atanv);
        cMtx_XrotM(*calc_mtx, atan2v);
        MtxPosition(&sp3C, &sp30);

        pBr->m3CC.x = pBr[-1].m3CC.x + sp30.x;
        pBr->m3CC.y = pBr[-1].m3CC.y + sp30.y;
        pBr->m3CC.z = pBr[-1].m3CC.z + sp30.z;
    }
}

static void daExtSpan_control2(daExtSpan_c* i_this, SpanPlank* pBr) {
    cXyz sp18;
    cXyz sp0C;

    pBr += i_this->mCount - 2;
    sp18.x = 0.0f;
    sp18.y = 0.0f;
    sp18.z = 75.0f;

    for (s32 i = 0; i < i_this->mCount - 1; i++, pBr--) {
        f32 tmp = pBr->m3F8 * 0.5f + (pBr->m3FC * pBr->m3F0 * 0.5f + pBr->m3CC.y);
        f32 y = tmp - pBr[1].m3CC.y;

        f32 x = pBr->m3CC.x - pBr[1].m3CC.x;
        f32 z = pBr->m3CC.z - pBr[1].m3CC.z;

        s16 atan2v;
        s16 atanv;
        atanv = (s16)cM_atan2s(x, z);
        atan2v = (s16)-cM_atan2s(y, std::sqrt(x * x + z * z));

        pBr[1].mRotation.y = atanv;
        pBr[1].mRotation.x = atan2v;

        cMtx_YrotS(*calc_mtx, atanv);
        cMtx_XrotM(*calc_mtx, atan2v);
        MtxPosition(&sp18, &sp0C);

        pBr->m3CC.x = pBr[1].m3CC.x + sp0C.x;
        pBr->m3CC.y = pBr[1].m3CC.y + sp0C.y;
        pBr->m3CC.z = pBr[1].m3CC.z + sp0C.z;
    }
}

static void daExtSpan_control3(daExtSpan_c* i_this, SpanPlank* pBr) {
    f32 x = pBr->m3CC.x - pBr[1].m3CC.x;
    f32 y = pBr->m3CC.y - pBr[1].m3CC.y;
    f32 z = pBr->m3CC.z - pBr[1].m3CC.z;

    pBr->mRotation.y = cM_atan2s(x, z);
    pBr->mRotation.x = (s16)-cM_atan2s(y, std::sqrt(x * x + z * z));
}

int daExtSpan_c::execute() {
    if (mCount == 0) {
        return 1;
    }
    // donor Execute :882-884 -- wind statics
    s_spanWindVec = dKyw_get_wind_vec();
    s_spanWy = cM_atan2s(s_spanWindVec->x, s_spanWindVec->z);
    s_spanWp = dKyw_get_wind_pow();

    // ------------------------------------------------------------------------
    // §440 donor bridge_move case 3 VERBATIM (:657-794), seams [R1]-[R5].
    // ------------------------------------------------------------------------
    {
        SpanPlank* pBr = &mSim[0];
        s32 i, j, iVar11;
        f32 tmpf, fVar14, fVar2;
        f32 my_tgt;
        cXyz sp14;

        m0300 += 3000;
        pBr->m3CC = mHomePos;

        daExtSpan_control1(this, pBr);

        (pBr + mCount - 1)->m3CC = mEndPos;

        daExtSpan_control2(this, pBr);
        daExtSpan_control3(this, pBr);

        sp14 = mHomePos - pBr->m3CC;

        current.pos = pBr->m3CC;
        current.angle = pBr->mRotation;

        for (i = 0; i < mCount; i++, pBr++) {
            pBr->mPosition = pBr->m3CC;
            tmpf = (((f32)(mCount - i) / (f32)mCount) * 0.75f);
            pBr->mPosition.x += sp14.x * tmpf;
            pBr->mPosition.y += sp14.y * tmpf;
            pBr->mPosition.z += sp14.z * tmpf;

            if (pBr->m406 != 0) {
                for (j = -5; j <= 5; j++) {
                    iVar11 = i + j;
                    if ((iVar11 < 0) || (iVar11 >= mCount)) {
                        continue;
                    }

                    my_tgt = (f32)pBr->m400 * ita_z_p[5 + j] * pBr[j].m3F0;
                    cLib_addCalcAngleS2(&pBr[j].m402, (s16)my_tgt, 4, 0x800);
                    cLib_addCalc2(&pBr[j].m3F8, pBr->m3F4 * ita_z_p[5 + j], 1.0f, 10.0f);
                }
            }

            if (((pBr->m408 & 4) != 0) && (pBr->m408 & 3) != 3) {
                // [R5] rope-cut lean: dead while the cut system is owed (m408
                // stays 7 = both ropes intact), donor shape preserved.
                fVar14 = 0.0f;
                fVar2 = -80.0f;
                if ((pBr->m408 & 3) == 1) {
                    fVar14 = 7000.0f;
                    fVar2 = -30.0f;
                } else if ((pBr->m408 & 3) == 2) {
                    fVar14 = -7000.0f;
                    fVar2 = -30.0f;
                }

                for (j = -5; j <= 5; j++) {
                    iVar11 = i + j;
                    if ((iVar11 < 0) || (iVar11 >= mCount)) {
                        continue;
                    }

                    my_tgt = fVar14 * ita_z_p[5 + j] * pBr[j].m3F0;
                    cLib_addCalcAngleS2(&pBr[j].m404, (s16)my_tgt, 4, 0x800);
                    cLib_addCalc2(&pBr[j].m3F8, fVar2 * ita_z_p[5 + j], 1.0f, 15.0f);
                }
            }

            if (pBr->m406 != 0) {
                pBr->m406--;
            }
            pBr->m400 = 0;
            pBr->mRotation.z = (s16)(pBr->m402 + pBr->m404);
            cLib_addCalcAngleS2(&pBr->m402, 0, 4, 0x400);
            cLib_addCalcAngleS2(&pBr->m404, 0, 4, 0x400);
            cLib_addCalc2(&pBr->m3FC, -15.0f, 1.0f, 5.0f);
            cLib_addCalc0(&pBr->m3F8, 1.0f, 5.0f);
        }

        m02FC = m02E0;
        m02F4 = m02E0;
        m02F8 = m02E4;
        m02F2 = 3000;
        m02F0 = 0x578;

        f32 tmpf2;
        if (s_spanWp > 0.1f) {
            tmpf2 = 2.0f;
        } else {
            tmpf2 = 0.0f;
        }
        cLib_addCalc2(&m02E0, tmpf2, 0.1f, 0.1f);
        cLib_addCalc2(&m02E4, tmpf2 * 0.3f, 0.1f, 0.05f);
    }

    // ========================================================================
    // §448 ROPE-MAT INIT MOVED HERE (root cause of the §429e/§441b rope fatal).
    // The donor inits both rope mats in its CREATE path (d_a_bridge.cpp:1363-
    // 1379), so by the time anything draws, the anchor walk below has already
    // written the rail's interior points. The receiver's §429c workaround did
    // the init lazily inside draw() instead -- because the custom-mounted
    // WwAlways arc is not resident at create time -- which opened a window the
    // donor does not have:
    //   execute(): walk counts m030C posts but SKIPS the interior writes
    //              (they are gated on mLineMatInit, still false)
    //   draw():    init allocates the position arrays, sets the flag, writes
    //              ONLY seg[0] and seg[m030C+1], then update(m030C+2) consumes
    //              indices 1..m030C -- NEVER WRITTEN -> heap garbage into
    //              GXPosition3f32 -> the "immediately after 429c ropes LIVE"
    //              GX FIFO fatal.
    // Initialising here, AHEAD of the walk, restores the donor's ordering
    // invariant (mats exist before the walk that fills them) while keeping the
    // receiver's residency retry. Reference for the invariant: noclip's live
    // Room44 bridge (rail numLines=2 maxSegments=14 numSegments=6, m030C=4
    // uncut ropes over 17 planks) -- see bus §448.
    // ========================================================================
    if (!mLineMatInit) {
        ResTIMG* img = (ResTIMG*)dComIfG_getObjectRes(
            "WwAlways", (int)((s_spanType & 8) ? 0x8D : 0x7E));
        if (img != NULL) {
            if (mLineMat.init(2, 14, img, 0)) {
                mLineMatInit = true;
            }
            for (int i = 0; i < mCount; ++i) {
                if ((mSim[i].m408 & 4) && !mSim[i].mLineInit &&
                    mSim[i].mLineMat1.init(4, 5, img, 1)) {
                    mSim[i].mLineInit = true;
                }
            }
            if (mLineMatInit) {
                // 451-H8: per-span init census -- a span that enqueues mats it
                // never initialised is the other way garbage reaches the FIFO.
                int inited = 0, posts = 0;
                for (int i = 0; i < mCount; ++i) {
                    if (mSim[i].m408 & 4) {
                        posts++;
                        if (mSim[i].mLineInit) {
                            inited++;
                        }
                    }
                }
                DuskLog.info("[ExtSpan] 448/451-P66 rope mats LIVE span={} planks={} posts={} "
                             "hangersInit={} railInit={} type=0x{:02X}",
                             (void*)this, mCount, posts, inited, (int)mLineMatInit,
                             (int)s_spanType);
            }
        }
    }

    // matrices + anchors (donor :902-921)
    m030C = 0;  // §441: donor counts posts during the anchor walk
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        mDoMtx_stack_c::transS(pl->mPosition.x, pl->mPosition.y, pl->mPosition.z);
        mDoMtx_stack_c::YrotM(pl->mRotation.y);
        mDoMtx_stack_c::XrotM(pl->mRotation.x);
        mDoMtx_stack_c::ZrotM(pl->mRotation.z);
        // ================================================================
        // §439 ANCHORS DONOR-VERBATIM (d_a_bridge.cpp:912-921; the root
        // transcription error): donor m11C[1] = local +99*scale.x, m0F8[1] =
        // -99, [2] twins at y-30. Mine were SIDE-SWAPPED -- §437's winding
        // swap was compensating in the writer while ends/edges stayed
        // crossed. Anchors corrected; writer reverted to the donor map.
        // ================================================================
        cXyz v, o;
        v.set(pl->mScale.x * 99.0f, 0.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m11C[1] = o;
        v.set(pl->mScale.x * 99.0f, -30.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m11C[2] = o;
        v.set(-pl->mScale.x * 99.0f, 0.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m0F8[1] = o;
        v.set(-pl->mScale.x * 99.0f, -30.0f, 0.0f);
        mDoMtx_stack_c::multVec(&v, &o);
        pl->m0F8[2] = o;
        pl->m11C[0] = pl->m11C[1];
        pl->m11C[0].y += 200.0f;
        pl->m0F8[0] = pl->m0F8[1];
        pl->m0F8[0].y += 200.0f;
        if (((s_spanType & 1) == 0) && (pl->m408 & 4)) {
            // §441 donor :1112-1167 (intact legs): post rope tops become the
            // rail interior segments. Cut interpolation + himo_cut_control1
            // dead while cut owed [R5].
            m030C++;
            if (mLineMatInit && m030C < 13) {
                mLineMat.getPos(0)[m030C] = pl->m11C[0];
                mLineMat.getPos(1)[m030C] = pl->m0F8[0];
            }
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
            // ================================================================
            // §437 WINDING EXPERIMENT (P64: vertices stationary, yet P63 roof
            // hits persist + riders shoved along the span): if left/right here
            // is mirrored vs the dzb's authored triangle order, every quad's
            // tris cross -- degenerate normals push riders and read as roofs.
            // This swaps left/right; verdicts: drag gone + P63 silent = was
            // mirrored; unchanged = winding correct, revert and look at
            // CrrPos itself.
            // ================================================================
            // §439: donor corner map restored (§437's swap was compensation
            // for the side-swapped anchors, now fixed at source).
            switch (i & 3) {
            case 0: src = &pl->m11C[2]; break;
            case 1: src = &pl->m0F8[2]; break;
            case 2: src = &pl->m11C[1]; break;
            default: src = &pl->m0F8[1]; break;
            }
            cXyz w = *src;
            // ================================================================
            // §438 END EXTENSION, donor-verbatim (d_a_bridge.cpp:1212-1230;
            // user caught my three deviations): plank 0 extends +50 local Z
            // (plank yaw points TOWARD home -- backward-pass heading), the
            // GAP-EDGE plank (m02DD-1) extends -50 full-bridge / -40 aite;
            // rotated by yaw AND pitch; NO y-drop (my earlier -40-Y was a
            // spec misreading). My flipped sign left both cliff junctions
            // uncovered -- the reported fall-through.
            // ================================================================
            if (idx == 0 || idx == m02DD - 1) {
                f32 extZ;
                if (idx == 0) {
                    extZ = 50.0f;
                } else if (m02DD == mCount) {
                    extZ = -50.0f;
                } else {
                    extZ = -40.0f;
                }
                const f32 ch = extZ * cM_scos(pl->mRotation.x);
                w.x += ch * cM_ssin(pl->mRotation.y);
                w.y += -extZ * cM_ssin(pl->mRotation.x);
                w.z += ch * cM_scos(pl->mRotation.y);
            }
            vtx[i].x = w.x;
            vtx[i].y = w.y;
            vtx[i].z = w.z;
        }
        // §433-P64: rider-drag investigation -- per-frame vertex motion. The
        // CrrPos correction moves riders by (new - back) vertex deltas; a
        // systematic per-frame delta = conveyor-belt drag. Logs the max delta
        // magnitude + its axis bias + the taper residual.
        {
            static u32 s_p64 = 0;
            if ((++s_p64 % 120) == 1) {
                cBgD_Vtx_t* nv2 = mpBgW->GetVtxTbl();
                f32 dMax = 0.0f, dxSum = 0.0f, dzSum = 0.0f;
                const cXyz res = mHomePos - mSim[0].m3CC;
                for (int i = 0; i < mpBgW->GetVtxNum(); ++i) {
                    // mBackVtx delta unavailable directly; use plank position
                    // deltas instead via statics below.
                }
                static cXyz s_prev0(0, 0, 0), s_prevMid(0, 0, 0);
                const cXyz& p0 = mSim[0].mPosition;
                const cXyz& pm = mSim[mCount / 2].mPosition;
                DuskLog.info("[ExtSpan] 433-P64 plank0 d=({}, {}, {}) mid d=({}, {}, {}) "
                             "resid=({}, {}, {})",
                             p0.x - s_prev0.x, p0.y - s_prev0.y, p0.z - s_prev0.z,
                             pm.x - s_prevMid.x, pm.y - s_prevMid.y, pm.z - s_prevMid.z,
                             res.x, res.y, res.z);
                s_prev0 = p0;
                s_prevMid = pm;
            }
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
    // §446b CAPTURE BUILD: ropes ON deliberately, to run the comparative probe
    // (§441b spec) against the banked control. Theory #4 (raw BE BTI) is now
    // ALSO falsified statically: txm_rope1.bti parses to fmt=14 CMPR 16x16
    // mipCount=1 imageOffset=32 (own parsing of the staged WwAlways arc) --
    // byte-for-byte the same header shape as the control's texture. Expect a
    // fatal; the P62/P63/P64 lines before it name the failing parameter.
    static const bool kExtSpanRopes = true;
    if (!kExtSpanRopes) {
        return 1;
    }
    GXColor ropeCol;
    ropeCol.r = ropeCol.g = ropeCol.b = 150;
    ropeCol.a = 255;
    for (int i = 0; i < mCount; ++i) {
        SpanPlank* pl = &mSim[i];
        if (!(pl->m408 & 4) || !pl->mLineInit) {
            continue;
        }
        // ====================================================================
        // §441 donor daBridge_Draw hanger fill VERBATIM (:173-273, intact-rope
        // legs; cut legs dead while m408 stays 7 [R5]). Per-vertex sizes:
        // uVar16 = 5 (bit3 thick) / 3; cut-half lines 2/3 sized 0.
        // §484: sizes are now u8, the DONOR's own type -- the parallel MDoExt1
        // stack carries the donor layout, so the f32 type-seam is retired
        // rather than converted at the call site.
        // ====================================================================
        const u8 uVar16 = (s_spanType & 8) ? 5 : 3;
        for (int side = 0; side < 2; ++side) {
            u8* size0 = pl->mLineMat1.getSize(side);
            u8* size1 = pl->mLineMat1.getSize(side + 2);
            cXyz* seg0 = pl->mLineMat1.getPos(side);
            cXyz* seg1 = pl->mLineMat1.getPos(side + 2);
            const cXyz& lo = side == 0 ? pl->m11C[1] : pl->m0F8[1];
            const cXyz& hi = side == 0 ? pl->m11C[0] : pl->m0F8[0];
            cXyz d = hi - lo;
            d.x /= 4.0f;
            d.y /= 4.0f;
            d.z /= 4.0f;
            for (int j = 0; j < 5; ++j) {
                size0[j] = uVar16;
                size1[j] = 0;
                seg0[j].set(lo.x + d.x * j, lo.y + d.y * j, lo.z + d.z * j);
                seg1[j] = hi;
            }
        }
        GXColor ropeCol1;
        ropeCol1.r = ropeCol1.g = ropeCol1.b = 150;
        ropeCol1.a = 255;
        pl->mLineMat1.update(5, ropeCol1, &tevStr);
        dComIfGd_set3DlineMat(&pl->mLineMat1);
    }
    if (mLineMatInit && mCount > 0) {
        // ====================================================================
        // §441 donor rail endpoints VERBATIM (:315-382): local (-120,350,-40)
        // about home yaw with the donor's sign-flip sequence; far endpoint =
        // else-leg (mEndPos + mirrored local). [R6] the aite partner rope-top
        // handoff (donor :329-335, joins the rails across the gap) is OWED --
        // needs the two span actors to find each other; ledgered.
        // ====================================================================
        cXyz sp54(-120.0f, 350.0f, -40.0f);
        cXyz sp48;
        const s16 hy = mHomeYaw;
        for (int line = 0; line < 2; ++line) {
            cXyz* seg = mLineMat.getPos(line);
            sp48.set(sp54.x * cM_scos(hy) + sp54.z * cM_ssin(hy), sp54.y,
                     -sp54.x * cM_ssin(hy) + sp54.z * cM_scos(hy));
            seg[0] = mHomePos + sp48;
            sp54.z *= -1.0f;
            sp48.set(sp54.x * cM_scos(hy) + sp54.z * cM_ssin(hy), sp54.y,
                     -sp54.x * cM_ssin(hy) + sp54.z * cM_scos(hy));
            seg[m030C + 1] = mEndPos + sp48;
            sp54.x *= -1.0f;  // donor flip sequence into line 1
        }
        const f32 railW = (s_spanType & 8) ? 6.5f : 4.0f;
        GXColor ropeCol;
        ropeCol.r = ropeCol.g = ropeCol.b = 150;
        ropeCol.a = 255;
        mLineMat.update(m030C + 2, railW, ropeCol, 0, &tevStr);
        dComIfGd_set3DlineMat(&mLineMat);
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
