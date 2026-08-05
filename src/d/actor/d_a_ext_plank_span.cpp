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

namespace {

struct Span {
    cXyz start;
    cXyz end;
};

std::vector<Span> s_spans;
std::string s_plankArc;
std::string s_plankModel;
std::string s_chainModel;  // optional; reserved for chain variant (§45)
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

class daExtSpan_c : public fopAc_ac_c {
public:
    int create();
    int execute() { return 1; }
    int draw();
    int deleteMe() { return 1; }

    J3DModel* mpPlanks[kMaxPlanks];
    int mCount;
    request_of_phase_process_class mPhase;
    bool mBuilt;
};

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
        if (mpPlanks[i] == NULL) {
            continue;
        }
        mpPlanks[i]->calc();
        dKyWw_setLightTevColorType(mpPlanks[i], &tevStr);
        mDoExt_modelUpdateDL(mpPlanks[i]);
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
