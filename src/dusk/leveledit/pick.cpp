// ============================================================================
// Level Editor — 1b click-to-select in world (zero mutation).
// Gate 10: min-distSq winner + hysteresis + tighter radius (not min-depth).
// ============================================================================

#include "dusk/leveledit/enumerate.hpp"
#include "dusk/leveledit/raycast.hpp"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_debug_viewer.h"
#include "d/d_ext_npc_mount.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "dusk/menu_pointer.h"
#include "dusk/settings.h"
#include "dusk/string.hpp"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_lib.h"

#include <aurora/lib/window.hpp>
#include <aurora/rmlui.hpp>
#include <imgui.h>
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_mouse.h>

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

namespace dusk::leveledit {
namespace {

// Gate 10: tighter screen radii; winner = nearest capsule, not frontmost-in-disk.
constexpr f32 kPickRadiusDefaultPx = 50.0f;
constexpr f32 kPickRadiusLargePx = 100.0f;
constexpr f32 kHoverRadiusScale = 1.15f;
constexpr f32 kCapsuleHeightSlop = 0.15f;
constexpr f32 kDepthTieMarginPx = 15.0f;
constexpr f32 kHysteresisCloserFrac = 0.80f;  // challenger must be ≤80% of sticky dist (≥20% closer)
constexpr int kSelectHeartbeatFrames = 120;
constexpr int kBreadcrumbFrames = 45;
constexpr f32 kCursorSourceDeltaPx = 50.0f;

// Gate 11a: camera→cursor ray vs actor cull-volume is authoritative (pinpoint,
// depth-ordered — front-most volume under the cursor wins). The screen-space
// point+radius path below is kept only as a fallback when the ray hits nothing
// (cursor over empty space / volumeless actor). Flip to false to A/B the old
// screen-only behaviour.
constexpr bool kUseRayPick = true;

bool s_prevLeftDown = false;
int s_selectHeartbeat = 0;
int s_breadcrumbFrames = 0;
bool sPreferImGuiCursor = false;

f32 sPickDotX = 0.0f;
f32 sPickDotY = 0.0f;
bool sPickDotValid = false;

struct HoverState {
    bool valid = false;
    fopAc_ac_c* live = nullptr;
    fpc_ProcID procId = fpcM_ERROR_PROCESS_ID_e;
    char name[9]{};
    cXyz pos{};
};

HoverState sHover{};

bool element_is_interactive(const Rml::Element* element) {
    for (const Rml::Element* el = element; el != nullptr; el = el->GetParentNode()) {
        const auto& values = el->GetComputedValues();
        if (values.pointer_events() == Rml::Style::PointerEvents::None) {
            continue;
        }
        const Rml::String& tag = el->GetTagName();
        if (tag == "button" || tag == "input" || tag == "select" || tag == "textarea" || tag == "a" ||
            tag == "tab" || tag == "option") {
            return true;
        }
    }
    return false;
}

const char* ui_block_reason() {
    if (dusk::menu_pointer::active()) {
        return "menu_pointer";
    }
    if (ImGui::GetIO().WantCaptureMouse) {
        return "WantCaptureMouse";
    }
    auto* context = aurora::rmlui::get_context();
    if (context != nullptr) {
        Rml::Element* hover = context->GetHoverElement();
        if (hover != nullptr && element_is_interactive(hover)) {
            return "rml_interactive";
        }
    }
    return nullptr;
}

f32 mirror_screen_x(f32 x) {
    if (!dusk::getSettings().game.enableMirrorMode.getValue()) {
        return x;
    }
    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 width = mDoGph_gInf_c::getWidthF();
    return minX + width - (x - minX);
}

void align_project_to_ginf(Vec& screen) {
    view_port_class* vp = dComIfGd_getViewport();
    if (vp == nullptr || vp->height <= 1.0f) {
        return;
    }
    if (vp->y_orig == 0.0f) {
        const f32 t = screen.y / vp->height;
        screen.y = mDoGph_gInf_c::getMinYF() + t * mDoGph_gInf_c::getHeightF();
    }
}

void project_world(const cXyz& worldPos, Vec& outScreen) {
    Vec src;
    src.x = worldPos.x;
    src.y = worldPos.y;
    src.z = worldPos.z;
    mDoLib_project(&src, &outScreen);
    align_project_to_ginf(outScreen);
}

bool on_screen(const Vec& screen) {
    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 minY = mDoGph_gInf_c::getMinYF();
    const f32 maxX = minX + mDoGph_gInf_c::getWidthF();
    const f32 maxY = minY + mDoGph_gInf_c::getHeightF();
    constexpr f32 kMargin = 120.0f;
    return screen.x >= minX - kMargin && screen.x <= maxX + kMargin &&
           screen.y >= minY - kMargin && screen.y <= maxY + kMargin;
}

bool is_large_pick_name(const char* name) {
    if (name == nullptr) {
        return false;
    }
    return std::strncmp(name, "Link", 8) == 0 || std::strncmp(name, "Horse", 8) == 0 ||
           std::strncmp(name, "Obj_Uma", 8) == 0 || std::strncmp(name, "ALINK", 8) == 0;
}

f32 dist_sq_point_to_segment_2d(f32 px, f32 py, f32 ax, f32 ay, f32 bx, f32 by) {
    const f32 abx = bx - ax;
    const f32 aby = by - ay;
    const f32 apx = px - ax;
    const f32 apy = py - ay;
    const f32 abLenSq = abx * abx + aby * aby;
    f32 t = 0.0f;
    if (abLenSq > 1.0e-4f) {
        t = (apx * abx + apy * aby) / abLenSq;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
    }
    const f32 cx = ax + t * abx;
    const f32 cy = ay + t * aby;
    const f32 dx = px - cx;
    const f32 dy = py - cy;
    return dx * dx + dy * dy;
}

struct ActorScreenPick {
    f32 distSq = FLT_MAX;
    f32 radiusPx = kPickRadiusDefaultPx;
    f32 depth = 0.0f;
    cXyz worldAnchor{};
    bool valid = false;
};

ActorScreenPick measure_actor_pick(fopAc_ac_c* live, const char* name, f32 cursorX, f32 cursorY) {
    ActorScreenPick out{};
    if (live == nullptr) {
        return out;
    }

    const cXyz feet = live->current.pos;
    cXyz head = live->eyePos;
    if (std::fabs(head.x - feet.x) < 1.0f && std::fabs(head.y - feet.y) < 1.0f &&
        std::fabs(head.z - feet.z) < 1.0f) {
        head.y += is_large_pick_name(name) ? 180.0f : 120.0f;
    }
    const cXyz& attn = live->attention_info.position;
    if (attn.y > feet.y + 20.0f) {
        head = attn;
    }

    Vec feetS{};
    Vec headS{};
    project_world(feet, feetS);
    project_world(head, headS);
    if (!on_screen(feetS) && !on_screen(headS)) {
        return out;
    }

    const f32 capsuleH =
        std::sqrt((headS.x - feetS.x) * (headS.x - feetS.x) + (headS.y - feetS.y) * (headS.y - feetS.y));
    const f32 base = is_large_pick_name(name) ? kPickRadiusLargePx : kPickRadiusDefaultPx;
    out.radiusPx = base + kCapsuleHeightSlop * capsuleH;
    out.distSq = dist_sq_point_to_segment_2d(cursorX, cursorY, feetS.x, feetS.y, headS.x, headS.y);
    out.depth = 0.5f * (feetS.z + headS.z);
    out.worldAnchor = cXyz(0.5f * (feet.x + head.x), 0.5f * (feet.y + head.y), 0.5f * (feet.z + head.z));
    out.valid = true;
    return out;
}

void fill_live_snapshot(PlacedActor& out, fopAc_ac_c* live, const char* label) {
    out = {};
    SAFE_STRCPY(out.chunkTag, "LIVE");
    SAFE_STRCPY(out.name, label);
    out.procname = fopAcM_GetName(live);
    out.argument = live->argument;
    out.params = 0;
    out.pos = live->current.pos;
    out.spawnPos = live->home.pos;
    out.angle = live->shape_angle;
    out.scale = live->scale;
    out.setID = live->setID;
    out.layer = -1;
    out.roomNo = live->current.roomNo;
    out.unspawned = false;
    out.isSpawnPoint = false;
    out.live = live;
}

struct PickCandidate {
    const PlacedActor* placed = nullptr;
    PlacedActor snapshot{};
    fopAc_ac_c* live = nullptr;
    // Owned copy — never alias sHover.name or EnumerateResult storage (SAFE_STRCPY / UAF).
    char name[9]{"(none)"};
    f32 distSq = FLT_MAX;
    f32 depth = 0.0f;
    f32 radiusPx = kPickRadiusDefaultPx;
    cXyz anchor{};
    bool valid = false;
};

void fill_candidate(PickCandidate& dest, const ActorScreenPick& m, fopAc_ac_c* live, const char* name,
                    const PlacedActor* placed) {
    dest.valid = true;
    dest.distSq = m.distSq;
    dest.depth = m.depth;
    dest.radiusPx = m.radiusPx;
    dest.anchor = m.worldAnchor;
    if (name != nullptr) {
        SAFE_STRCPY(dest.name, name);
    } else {
        SAFE_STRCPY(dest.name, "(none)");
    }
    dest.live = live;
    dest.placed = placed;
    if (placed == nullptr) {
        fill_live_snapshot(dest.snapshot, live, dest.name);
    }
}

// ============================================================================
// Gate 11a — ray pick. Front-most cull-volume the camera→cursor ray enters.
// ============================================================================
struct RayHit {
    fopAc_ac_c* live = nullptr;
    const PlacedActor* placed = nullptr;
    char name[9]{"(none)"};
    f32 t = FLT_MAX;
    cXyz hit{};
    bool valid = false;
};

void consider_ray(RayHit& best, fopAc_ac_c* live, const char* name, const PlacedActor* placed,
                  const cXyz& origin, const cXyz& dir) {
    if (live == nullptr) {
        return;
    }
    f32 t = FLT_MAX;
    if (!ray_hits_actor(live, origin, dir, t)) {
        return;
    }
    if (t >= best.t) {
        return;
    }
    best.t = t;
    best.live = live;
    best.placed = placed;
    SAFE_STRCPY(best.name, name != nullptr ? name : "(none)");
    best.hit.set(origin.x + dir.x * t, origin.y + dir.y * t, origin.z + dir.z * t);
    best.valid = true;
}

// Promote a ray hit into the shared PickCandidate the hover/click paths consume.
void winner_from_ray(PickCandidate& winner, const RayHit& ray) {
    winner = {};
    winner.valid = true;
    winner.live = ray.live;
    winner.placed = ray.placed;
    SAFE_STRCPY(winner.name, ray.name);
    winner.anchor = ray.hit;
    winner.depth = ray.t;
    winner.distSq = 0.0f;
    winner.radiusPx = kPickRadiusDefaultPx;
    if (ray.placed == nullptr) {
        fill_live_snapshot(winner.snapshot, ray.live, winner.name);
    }
}

// Gate 10: min screen-dist among in-radius; depth only within ~15px screen margin.
bool beats_in_radius(const ActorScreenPick& cand, const PickCandidate& cur) {
    if (!cur.valid) {
        return true;
    }
    const f32 da = std::sqrt(cand.distSq);
    const f32 db = std::sqrt(cur.distSq);
    if (std::fabs(da - db) <= kDepthTieMarginPx) {
        return cand.depth < cur.depth;
    }
    return cand.distSq < cur.distSq;
}

void consider_live(PickCandidate& nearest, PickCandidate& winner, PickCandidate& runnerUp,
                   fopAc_ac_c* live, const char* name, const PlacedActor* placed, f32 cursorX,
                   f32 cursorY, f32 radiusScale) {
    const ActorScreenPick m = measure_actor_pick(live, name, cursorX, cursorY);
    if (!m.valid) {
        return;
    }

    if (m.distSq < nearest.distSq) {
        fill_candidate(nearest, m, live, name, placed);
    }

    const f32 limit = m.radiusPx * radiusScale;
    if (m.distSq > limit * limit) {
        return;
    }

    if (beats_in_radius(m, winner)) {
        runnerUp = winner;
        fill_candidate(winner, m, live, name, placed);
    } else if (beats_in_radius(m, runnerUp)) {
        fill_candidate(runnerUp, m, live, name, placed);
    }
}

bool map_window_xy_to_game(f32 windowX, f32 windowY, f32& outX, f32& outY) {
    if (!dusk::menu_pointer::map_window_mouse_to_game_screen(windowX, windowY, outX, outY)) {
        return false;
    }
    outX = mirror_screen_x(outX);
    return true;
}

bool map_cursor_game(f32& outX, f32& outY, f32* outWinX, f32* outWinY) {
    float sdlX = 0.0f;
    float sdlY = 0.0f;
    SDL_GetMouseState(&sdlX, &sdlY);

    float srcX = sdlX;
    float srcY = sdlY;
    const ImGuiIO& io = ImGui::GetIO();
    if (sPreferImGuiCursor && io.MousePos.x >= 0.0f && io.MousePos.y >= 0.0f) {
        srcX = io.MousePos.x;
        srcY = io.MousePos.y;
    }

    if (outWinX != nullptr) {
        *outWinX = srcX;
    }
    if (outWinY != nullptr) {
        *outWinY = srcY;
    }
    return map_window_xy_to_game(srcX, srcY, outX, outY);
}

void gather_at_cursor(f32 cursorX, f32 cursorY, f32 radiusScale, PickCandidate& nearest,
                      PickCandidate& winner, PickCandidate& runnerUp, int* liveOnScreenOut,
                      EnumerateResult* resultOut, RayHit* rayOut) {
    EnumerateResult result = enumerate_room_actors();
    int liveOnScreen = 0;

    // Gate 11a: one camera→cursor ray, reused across every actor this frame.
    RayHit rayBest{};
    cXyz rayOrigin{};
    cXyz rayDir{};
    const bool rayValid =
        kUseRayPick && rayOut != nullptr && build_pick_ray(cursorX, cursorY, rayOrigin, rayDir);

    for (size_t i = 0; i < result.actors.size(); ++i) {
        const PlacedActor& actor = result.actors[i];
        if (actor.unspawned || actor.isSpawnPoint || actor.live == nullptr) {
            continue;
        }
        Vec probe{};
        project_world(actor.live->current.pos, probe);
        if (on_screen(probe)) {
            ++liveOnScreen;
        }
        consider_live(nearest, winner, runnerUp, actor.live, actor.name, &actor, cursorX, cursorY,
                      radiusScale);
        if (rayValid) {
            consider_ray(rayBest, actor.live, actor.name, &actor, rayOrigin, rayDir);
        }
    }

    // №44: ExtNpc mounts use setID 0xFFFF — not joined into DZR buffer rows. Still pickable.
    struct ExtPickCtx {
        f32 cursorX;
        f32 cursorY;
        f32 radiusScale;
        bool rayValid;
        cXyz* rayOrigin;
        cXyz* rayDir;
        PickCandidate* nearest;
        PickCandidate* winner;
        PickCandidate* runnerUp;
        RayHit* rayBest;
        int* liveOnScreen;
    };
    ExtPickCtx extCtx{cursorX, cursorY, radiusScale, rayValid, &rayOrigin, &rayDir, &nearest,
                      &winner, &runnerUp, &rayBest, &liveOnScreen};
    fopAcIt_Executor(
        [](void* i_actor, void* i_data) -> int {
            auto* actor = static_cast<fopAc_ac_c*>(i_actor);
            auto* ctx = static_cast<ExtPickCtx*>(i_data);
            if (actor == nullptr || ctx == nullptr) {
                return 0;
            }
            dExtNpcIdentifyInfo mount{};
            if (!dExtNpcMount_queryActor(actor, &mount) || !mount.valid) {
                return 0;
            }
            // Skip BGs for click-identify (huge cull volumes steal every click).
            if (mount.proc[0] && std::strncmp(mount.proc, "EXT_BG", 6) == 0) {
                return 0;
            }
            const char* label =
                mount.displayName[0] ? mount.displayName : (mount.proc[0] ? mount.proc : "ExtNpc");
            Vec probe{};
            project_world(actor->current.pos, probe);
            if (on_screen(probe)) {
                ++(*ctx->liveOnScreen);
            }
            consider_live(*ctx->nearest, *ctx->winner, *ctx->runnerUp, actor, label, nullptr,
                          ctx->cursorX, ctx->cursorY, ctx->radiusScale);
            if (ctx->rayValid) {
                consider_ray(*ctx->rayBest, actor, label, nullptr, *ctx->rayOrigin, *ctx->rayDir);
            }
            return 0;
        },
        &extCtx);

    if (fopAc_ac_c* player = dComIfGp_getPlayer(0)) {
        consider_live(nearest, winner, runnerUp, player, "Link", nullptr, cursorX, cursorY,
                      radiusScale);
        if (rayValid) {
            consider_ray(rayBest, player, "Link", nullptr, rayOrigin, rayDir);
        }
    }
    if (fopAc_ac_c* horse = reinterpret_cast<fopAc_ac_c*>(dComIfGp_getHorseActor())) {
        consider_live(nearest, winner, runnerUp, horse, "Horse", nullptr, cursorX, cursorY,
                      radiusScale);
        if (rayValid) {
            consider_ray(rayBest, horse, "Horse", nullptr, rayOrigin, rayDir);
        }
    }

    if (liveOnScreenOut != nullptr) {
        *liveOnScreenOut = liveOnScreen;
    }
    if (resultOut != nullptr) {
        *resultOut = std::move(result);
    }
    if (rayOut != nullptr) {
        *rayOut = rayBest;
    }
}

void clear_hover() {
    sHover = {};
}

// Hysteresis after winner: keep sticky unless challenger is ≥20% closer, or sticky left radius/died.
void apply_hover_hysteresis(PickCandidate& winner, f32 cursorX, f32 cursorY, f32 radiusScale) {
    if (!winner.valid || !sHover.valid || sHover.procId == fpcM_ERROR_PROCESS_ID_e) {
        return;
    }
    if (winner.live != nullptr && fopAcM_GetID(winner.live) == sHover.procId) {
        return;
    }

    base_process_class* base = fpcM_SearchByID(sHover.procId);
    if (base == nullptr) {
        return;  // sticky dead → take new winner
    }
    fopAc_ac_c* stickyLive = static_cast<fopAc_ac_c*>(base);
    const ActorScreenPick stickyM = measure_actor_pick(stickyLive, sHover.name, cursorX, cursorY);
    if (!stickyM.valid) {
        return;
    }
    const f32 stickyLimit = stickyM.radiusPx * radiusScale;
    if (stickyM.distSq > stickyLimit * stickyLimit) {
        return;  // sticky left radius → take new winner
    }
    // Keep sticky unless winner is clearly closer (≥20%).
    // Pass a stack copy of the name — never sHover.name (SAFE_STRCPY same-buffer FATAL).
    if (winner.distSq > stickyM.distSq * kHysteresisCloserFrac) {
        char stickyName[9]{};
        SAFE_STRCPY(stickyName, sHover.name);
        fill_candidate(winner, stickyM, stickyLive, stickyName, nullptr);
    }
}

void maybe_breadcrumb(f32 sdlWinX, f32 sdlWinY, f32 cursorX, f32 cursorY, bool relative,
                      const PickCandidate& winner, const PickCandidate& runnerUp,
                      const PickCandidate& nearest) {
    if (++s_breadcrumbFrames < kBreadcrumbFrames) {
        return;
    }
    s_breadcrumbFrames = 0;

    f32 imguiGameX = 0.0f;
    f32 imguiGameY = 0.0f;
    const ImGuiIO& io = ImGui::GetIO();
    const bool imguiMapped =
        io.MousePos.x >= 0.0f && io.MousePos.y >= 0.0f &&
        map_window_xy_to_game(io.MousePos.x, io.MousePos.y, imguiGameX, imguiGameY);

    f32 sdlGameX = 0.0f;
    f32 sdlGameY = 0.0f;
    const bool sdlMapped = map_window_xy_to_game(sdlWinX, sdlWinY, sdlGameX, sdlGameY);

    f32 sourceDelta = -1.0f;
    if (imguiMapped && sdlMapped) {
        const f32 dx = sdlGameX - imguiGameX;
        const f32 dy = sdlGameY - imguiGameY;
        sourceDelta = std::sqrt(dx * dx + dy * dy);
        if (!sPreferImGuiCursor && (relative || sourceDelta > kCursorSourceDeltaPx)) {
            sPreferImGuiCursor = true;
            DuskLog.info(
                "Pick cursor-source → ImGui (relative={} delta={:.1f})", relative, sourceDelta);
        }
    }

    const char* rule = !winner.valid             ? "none"
                       : (nearest.valid && nearest.live == winner.live) ? "nearest-dist"
                                                                        : "in-radius-distSq";
    // If old depth-winner would differ, note it for classify (algorithm path).
    const bool depthWouldDiffer =
        winner.valid && runnerUp.valid && runnerUp.depth < winner.depth &&
        runnerUp.distSq > winner.distSq;

    DuskLog.info(
        "Pick crumb win=({:.0f},{:.0f}) map=({:.1f},{:.1f}) imguiMap=({:.1f},{:.1f}) "
        "delta={:.1f} relative={} preferImGui={} rule={} depthClash={} "
        "w1={} d1={:.1f} z1={:.3f} r1={:.0f} w2={} d2={:.1f} z2={:.3f}",
        sdlWinX, sdlWinY, cursorX, cursorY, imguiMapped ? imguiGameX : -1.0f,
        imguiMapped ? imguiGameY : -1.0f, sourceDelta, relative, sPreferImGuiCursor, rule,
        depthWouldDiffer, winner.valid ? winner.name : "(none)",
        winner.valid ? winner.distSq : -1.0f, winner.valid ? winner.depth : -1.0f,
        winner.valid ? winner.radiusPx : 0.0f, runnerUp.valid ? runnerUp.name : "(none)",
        runnerUp.valid ? runnerUp.distSq : -1.0f, runnerUp.valid ? runnerUp.depth : -1.0f);
}

// Gate 11c: click-only identify journal (TP buffer row + optional mod mount enrich).
int sPickIdentifyCount = 0;
char sPickIdentifyHud[160]{};

void log_pick_identify(int clickN, const PlacedActor& picked, fopAc_ac_c* live, bool usedRay) {
    const fpc_ProcID procId =
        live != nullptr ? fopAcM_GetID(live) : fpcM_ERROR_PROCESS_ID_e;
    const u32 liveParams = live != nullptr ? fopAcM_GetParam(live) : 0;

    dExtNpcIdentifyInfo mount{};
    if (live != nullptr) {
        dExtNpcMount_queryActor(live, &mount);
    }

    DuskLog.info(
        "Pick identify #{} procId=0x{:x} tag={} name={} proc={} arg={} params=0x{:08x} "
        "liveParams=0x{:08x} setID={} layer={} room={} pos=({:.1f},{:.1f},{:.1f}) "
        "home=({:.1f},{:.1f},{:.1f}) spawn=({:.1f},{:.1f},{:.1f}) ray={} join={} "
        "mount={} disp={} head={}@{} mod={}",
        clickN, static_cast<unsigned>(procId), picked.chunkTag, picked.name, picked.procname,
        picked.argument, picked.params, liveParams, picked.setID, picked.layer, picked.roomNo,
        picked.pos.x, picked.pos.y, picked.pos.z, live != nullptr ? live->home.pos.x : 0.0f,
        live != nullptr ? live->home.pos.y : 0.0f, live != nullptr ? live->home.pos.z : 0.0f,
        picked.spawnPos.x, picked.spawnPos.y, picked.spawnPos.z, usedRay ? 1 : 0,
        picked.unspawned ? "unspawned" : picked.isSpawnPoint ? "spawn" : "live",
        mount.valid ? mount.proc : "-", mount.valid ? mount.displayName : "-",
        mount.valid && mount.headModel[0] ? mount.headModel : "-",
        mount.valid && mount.headJoint[0] ? mount.headJoint : "-",
        mount.valid ? mount.modFolder : "-");

    if (mount.valid && mount.headModel[0]) {
        std::snprintf(sPickIdentifyHud, sizeof(sPickIdentifyHud),
                      "ID #%d %s proc=%s head=%s@%s", clickN, picked.name, mount.proc,
                      mount.headModel, mount.headJoint);
    } else {
        std::snprintf(sPickIdentifyHud, sizeof(sPickIdentifyHud),
                      "ID #%d %s proc=%d arg=%d setID=%u", clickN, picked.name, picked.procname,
                      picked.argument, static_cast<unsigned>(picked.setID));
    }
}

}  // namespace

int get_pick_identify_count() {
    return sPickIdentifyCount;
}

const char* get_pick_identify_hud_line() {
    return sPickIdentifyHud[0] ? sPickIdentifyHud : nullptr;
}

void tick_world_pick_hover() {
    sPickDotValid = false;
    if (!dusk::g_levelEditorSession || !session_select_mode_enabled()) {
        clear_hover();
        return;
    }
    if (ui_block_reason() != nullptr) {
        clear_hover();
        return;
    }

    float sdlWinX = 0.0f;
    float sdlWinY = 0.0f;
    SDL_GetMouseState(&sdlWinX, &sdlWinY);

    f32 cursorX = 0.0f;
    f32 cursorY = 0.0f;
    if (!map_cursor_game(cursorX, cursorY, nullptr, nullptr)) {
        clear_hover();
        return;
    }

    sPickDotX = cursorX;
    sPickDotY = cursorY;
    sPickDotValid = true;

    PickCandidate nearest{};
    PickCandidate winner{};
    PickCandidate runnerUp{};
    RayHit rayBest{};
    gather_at_cursor(cursorX, cursorY, kHoverRadiusScale, nearest, winner, runnerUp, nullptr,
                     nullptr, &rayBest);

    SDL_Window* const sdlWindow = aurora::window::get_sdl_window();
    const bool relative = sdlWindow != nullptr && SDL_GetWindowRelativeMouseMode(sdlWindow);
    maybe_breadcrumb(sdlWinX, sdlWinY, cursorX, cursorY, relative, winner, runnerUp, nearest);

    if (rayBest.valid) {
        // Ray hit is authoritative & stable frame-to-frame — no hysteresis needed.
        winner_from_ray(winner, rayBest);
    } else {
        apply_hover_hysteresis(winner, cursorX, cursorY, kHoverRadiusScale);
    }

    if (!winner.valid || winner.live == nullptr) {
        clear_hover();
        return;
    }

    sHover.valid = true;
    sHover.live = winner.live;
    sHover.procId = fopAcM_GetID(winner.live);
    SAFE_STRCPY(sHover.name, winner.name);
    sHover.pos = winner.anchor;
}

void draw_pick_hover() {
    if (!dusk::g_levelEditorSession || !session_select_mode_enabled()) {
        return;
    }

    // Gate 10: sanity pick-dot at mapped cursor (screen report under physical aim).
    if (sPickDotValid) {
        dDbVw_Report(static_cast<int>(sPickDotX), static_cast<int>(sPickDotY), "+");
        dDbVw_Report(20, 240, "PICK (%.0f,%.0f)%s", sPickDotX, sPickDotY,
                     sPreferImGuiCursor ? " imgui" : " sdl");
    }

    if (!sHover.valid) {
        return;
    }

    if (sHover.procId != fpcM_ERROR_PROCESS_ID_e) {
        if (base_process_class* base = fpcM_SearchByID(sHover.procId)) {
            sHover.live = static_cast<fopAc_ac_c*>(base);
        } else {
            clear_hover();
            return;
        }
    }
    if (sHover.live == nullptr) {
        return;
    }

    if (const PlacedActor* sel = get_selection()) {
        const bool sameLive = sel->live != nullptr && sel->live == sHover.live;
        const bool sameProc =
            sel->live != nullptr && sHover.procId != fpcM_ERROR_PROCESS_ID_e &&
            sHover.procId == fopAcM_GetID(sel->live);
        if (sameLive || sameProc) {
            dDbVw_Report(20, 220, "HOVER %s  (selected)", sHover.name);
            return;
        }
    }

    // Gate 11b (Path A): model-tight hover highlight (per-joint boxes) — amber.
    constexpr GXColor kHoverRing = {0xFF, 0xE0, 0x80, 0xA0};
    draw_actor_volume_highlight(sHover.live, 0xFF, 0xC0, 0x40, 0x50);

    cXyz ringPos(sHover.live->current.pos.x,
                 0.5f * (sHover.live->current.pos.y + sHover.live->eyePos.y),
                 sHover.live->current.pos.z);
    dDbVw_drawSphereXlu(ringPos, 40.0f, kHoverRing, /*clipZ=*/0);
    dDbVw_Report(20, 220, "HOVER %s  (click)", sHover.name);
}

void try_world_pick_on_click() {
    if (!dusk::g_levelEditorSession) {
        return;
    }

    float windowX = 0.0f;
    float windowY = 0.0f;
    const std::uint32_t mouseButtons = SDL_GetMouseState(&windowX, &windowY);
    const bool leftDown = (mouseButtons & SDL_BUTTON_LMASK) != 0;
    const bool sdlReleased = s_prevLeftDown && !leftDown;
    const bool imguiReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    const bool leftPressed = !s_prevLeftDown && leftDown;
    s_prevLeftDown = leftDown;

    const bool selectMode = session_select_mode_enabled();
    const bool pcHotkeys = session_pc_hotkeys_enabled();
    const bool wantMouse = ImGui::GetIO().WantCaptureMouse;
    SDL_Window* const sdlWindow = aurora::window::get_sdl_window();
    const bool relative = sdlWindow != nullptr && SDL_GetWindowRelativeMouseMode(sdlWindow);

    if (selectMode) {
        if (++s_selectHeartbeat >= kSelectHeartbeatFrames) {
            s_selectHeartbeat = 0;
            DuskLog.info(
                "Pick heart selectMode=1 pcHotkeys={} buttons=0x{:x} sdlDown={} wantMouse={} "
                "relative={} preferImGui={} win=({:.0f},{:.0f})",
                pcHotkeys, mouseButtons, leftDown, wantMouse, relative, sPreferImGuiCursor, windowX,
                windowY);
        }
    } else {
        s_selectHeartbeat = 0;
    }

    if (leftPressed) {
        DuskLog.info(
            "Pick down buttons=0x{:x} selectMode={} pcHotkeys={} wantMouse={} relative={} "
            "win=({:.0f},{:.0f})",
            mouseButtons, selectMode, pcHotkeys, wantMouse, relative, windowX, windowY);
    }

    if (imguiReleased && !sdlReleased) {
        DuskLog.info(
            "Pick edge imgui-only buttons=0x{:x} selectMode={} pcHotkeys={} wantMouse={} "
            "relative={} win=({:.0f},{:.0f})",
            mouseButtons, selectMode, pcHotkeys, wantMouse, relative, windowX, windowY);
    }

    if (!sdlReleased && !imguiReleased) {
        return;
    }

    const char* edge = sdlReleased && imguiReleased ? "both" : sdlReleased ? "sdl" : "imgui";
    DuskLog.info(
        "Pick release edge={} buttons=0x{:x} selectMode={} pcHotkeys={} wantMouse={} "
        "relative={} win=({:.0f},{:.0f})",
        edge, mouseButtons, selectMode, pcHotkeys, wantMouse, relative, windowX, windowY);

    if (const char* reason = ui_block_reason()) {
        DuskLog.info("Pick skip ui reason={} selectMode={} pcHotkeys={} win=({:.0f},{:.0f})", reason,
                     selectMode, pcHotkeys, windowX, windowY);
        return;
    }

    // Same cursor source + winner rule as hover.
    f32 clickX = 0.0f;
    f32 clickY = 0.0f;
    if (!map_cursor_game(clickX, clickY, nullptr, nullptr)) {
        DuskLog.info("Pick skip map selectMode={} pcHotkeys={} win=({:.0f},{:.0f})", selectMode,
                     pcHotkeys, windowX, windowY);
        return;
    }

    PickCandidate nearest{};
    PickCandidate winner{};
    PickCandidate runnerUp{};
    RayHit rayBest{};
    int liveOnScreen = 0;
    EnumerateResult result{};
    gather_at_cursor(clickX, clickY, 1.0f, nearest, winner, runnerUp, &liveOnScreen, &result,
                     &rayBest);

    // Gate 11a: ray hit wins outright (pinpoint, depth-ordered). Screen path is
    // the empty-space fallback below.
    if (rayBest.valid) {
        winner_from_ray(winner, rayBest);
    }

    if (!winner.valid) {
        DuskLog.info(
            "Pick miss click=({:.1f},{:.1f}) radiusDefault={:.0f} liveOnScreen={} nearest={} "
            "setID={} distSq={:.1f} nearR={:.0f} selectMode={} pcHotkeys={} actors={}",
            clickX, clickY, kPickRadiusDefaultPx, liveOnScreen,
            nearest.valid ? nearest.name : "(none)",
            nearest.valid && nearest.live != nullptr ? nearest.live->setID : -1,
            nearest.valid ? nearest.distSq : -1.0f, nearest.valid ? nearest.radiusPx : 0.0f,
            selectMode, pcHotkeys, result.actors.size());
        return;
    }

    const PlacedActor& picked = winner.placed != nullptr ? *winner.placed : winner.snapshot;
    if (winner.placed != nullptr) {
        int idx = -1;
        for (size_t i = 0; i < result.actors.size(); ++i) {
            if (&result.actors[i] == winner.placed) {
                idx = static_cast<int>(i);
                break;
            }
        }
        set_selected_index(idx);
    } else {
        set_selected_index(-1);
    }

    DuskLog.info(
        "Pick hit name={} setID={} depth={:.4f} distSq={:.1f} radius={:.0f} selectMode={} "
        "pcHotkeys={} edge={}",
        picked.name, picked.setID, winner.depth, winner.distSq, winner.radiusPx, selectMode,
        pcHotkeys, edge);
    set_selection_snapshot(picked, true);
    ++sPickIdentifyCount;
    log_pick_identify(sPickIdentifyCount, picked, winner.live, rayBest.valid);
}

}  // namespace dusk::leveledit

#else

namespace dusk::leveledit {

void try_world_pick_on_click() {}
void tick_world_pick_hover() {}
void draw_pick_hover() {}
int get_pick_identify_count() {
    return 0;
}
const char* get_pick_identify_hud_line() {
    return nullptr;
}

}  // namespace dusk::leveledit

#endif
