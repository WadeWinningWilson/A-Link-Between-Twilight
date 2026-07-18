// ============================================================================
// Level Editor — 1b click-to-select in world (zero mutation).
// Gate 8e: screen capsule hit + depth-scaled radius + Select Mode hover preview.
// ============================================================================

#include "dusk/leveledit/enumerate.hpp"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_debug_viewer.h"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "dusk/menu_pointer.h"
#include "dusk/settings.h"
#include "dusk/string.hpp"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
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

// Gate 8e: body capsule + larger base; hover shows what a click would grab.
constexpr f32 kPickRadiusDefaultPx = 120.0f;
constexpr f32 kPickRadiusLargePx = 220.0f;
constexpr f32 kHoverRadiusScale = 1.35f;
constexpr int kSelectHeartbeatFrames = 120;

bool s_prevLeftDown = false;
int s_selectHeartbeat = 0;

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

// Screen-space capsule from feet → attention/eyes. Radius grows with on-screen height.
ActorScreenPick measure_actor_pick(fopAc_ac_c* live, const char* name, f32 cursorX, f32 cursorY) {
    ActorScreenPick out{};
    if (live == nullptr) {
        return out;
    }

    const cXyz feet = live->current.pos;
    cXyz head = live->eyePos;
    // Fallback head if eye == feet (some actors).
    if (std::fabs(head.x - feet.x) < 1.0f && std::fabs(head.y - feet.y) < 1.0f &&
        std::fabs(head.z - feet.z) < 1.0f) {
        head.y += is_large_pick_name(name) ? 180.0f : 120.0f;
    }
    // Prefer attention lock point when it's meaningfully above feet.
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
    // ~35% of body screen height as extra slop so torso/legs are easy clicks.
    out.radiusPx = base + 0.35f * capsuleH;
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
    const char* name = "(none)";
    f32 distSq = FLT_MAX;
    f32 depth = 0.0f;
    f32 radiusPx = kPickRadiusDefaultPx;
    cXyz anchor{};
    bool valid = false;
};

void consider_live(PickCandidate& nearest, PickCandidate& best, fopAc_ac_c* live, const char* name,
                   const PlacedActor* placed, f32 cursorX, f32 cursorY, f32 radiusScale) {
    const ActorScreenPick m = measure_actor_pick(live, name, cursorX, cursorY);
    if (!m.valid) {
        return;
    }

    if (m.distSq < nearest.distSq) {
        nearest.distSq = m.distSq;
        nearest.name = name;
        nearest.live = live;
        nearest.placed = placed;
        nearest.depth = m.depth;
        nearest.radiusPx = m.radiusPx;
        nearest.anchor = m.worldAnchor;
        nearest.valid = true;
        if (placed == nullptr) {
            fill_live_snapshot(nearest.snapshot, live, name);
        }
    }

    const f32 limit = m.radiusPx * radiusScale;
    if (m.distSq > limit * limit) {
        return;
    }

    if (!best.valid || m.depth < best.depth) {
        best.valid = true;
        best.distSq = m.distSq;
        best.depth = m.depth;
        best.name = name;
        best.live = live;
        best.placed = placed;
        best.radiusPx = m.radiusPx;
        best.anchor = m.worldAnchor;
        if (placed == nullptr) {
            fill_live_snapshot(best.snapshot, live, name);
        }
    }
}

bool map_cursor_game(f32& outX, f32& outY) {
    float windowX = 0.0f;
    float windowY = 0.0f;
    SDL_GetMouseState(&windowX, &windowY);
    if (!dusk::menu_pointer::map_window_mouse_to_game_screen(windowX, windowY, outX, outY)) {
        return false;
    }
    outX = mirror_screen_x(outX);
    return true;
}

void gather_at_cursor(f32 cursorX, f32 cursorY, f32 radiusScale, PickCandidate& nearest,
                      PickCandidate& best, int* liveOnScreenOut, EnumerateResult* resultOut) {
    EnumerateResult result = enumerate_room_actors();
    int liveOnScreen = 0;

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
        consider_live(nearest, best, actor.live, actor.name, &actor, cursorX, cursorY, radiusScale);
    }

    if (fopAc_ac_c* player = dComIfGp_getPlayer(0)) {
        consider_live(nearest, best, player, "Link", nullptr, cursorX, cursorY, radiusScale);
    }
    if (fopAc_ac_c* horse = reinterpret_cast<fopAc_ac_c*>(dComIfGp_getHorseActor())) {
        consider_live(nearest, best, horse, "Horse", nullptr, cursorX, cursorY, radiusScale);
    }

    if (liveOnScreenOut != nullptr) {
        *liveOnScreenOut = liveOnScreen;
    }
    if (resultOut != nullptr) {
        *resultOut = std::move(result);
    }
}

void clear_hover() {
    sHover = {};
}

}  // namespace

void tick_world_pick_hover() {
    if (!dusk::g_levelEditorSession || !session_select_mode_enabled()) {
        clear_hover();
        return;
    }
    if (ui_block_reason() != nullptr) {
        clear_hover();
        return;
    }

    f32 cursorX = 0.0f;
    f32 cursorY = 0.0f;
    if (!map_cursor_game(cursorX, cursorY)) {
        clear_hover();
        return;
    }

    PickCandidate nearest{};
    PickCandidate best{};
    gather_at_cursor(cursorX, cursorY, kHoverRadiusScale, nearest, best, nullptr, nullptr);

    // Prefer in-radius best; else show nearest ghost only if reasonably close (2× radius).
    PickCandidate* show = nullptr;
    if (best.valid) {
        show = &best;
    } else if (nearest.valid && nearest.distSq <= (nearest.radiusPx * 2.0f) * (nearest.radiusPx * 2.0f)) {
        show = &nearest;
    }

    if (show == nullptr || show->live == nullptr) {
        clear_hover();
        return;
    }

    sHover.valid = true;
    sHover.live = show->live;
    sHover.procId = fopAcM_GetID(show->live);
    SAFE_STRCPY(sHover.name, show->name);
    sHover.pos = show->anchor;
}

void draw_pick_hover() {
    if (!dusk::g_levelEditorSession || !session_select_mode_enabled() || !sHover.valid) {
        return;
    }

    // Rebind if Stage churn invalidated the pointer.
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

    cXyz pos(sHover.live->current.pos.x,
             0.5f * (sHover.live->current.pos.y + sHover.live->eyePos.y),
             sHover.live->current.pos.z);

    const GXColor ring = {0xFF, 0xC0, 0x40, 0x90};
    dDbVw_drawSphereXlu(pos, 70.0f, ring, /*clipZ=*/0);
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
                "relative={} win=({:.0f},{:.0f})",
                pcHotkeys, mouseButtons, leftDown, wantMouse, relative, windowX, windowY);
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

    f32 clickX = 0.0f;
    f32 clickY = 0.0f;
    if (!dusk::menu_pointer::map_window_mouse_to_game_screen(windowX, windowY, clickX, clickY)) {
        DuskLog.info("Pick skip map selectMode={} pcHotkeys={} win=({:.0f},{:.0f})", selectMode,
                     pcHotkeys, windowX, windowY);
        return;
    }
    clickX = mirror_screen_x(clickX);

    PickCandidate nearest{};
    PickCandidate best{};
    int liveOnScreen = 0;
    EnumerateResult result{};
    gather_at_cursor(clickX, clickY, 1.0f, nearest, best, &liveOnScreen, &result);

    if (!best.valid) {
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

    const PlacedActor& picked = best.placed != nullptr ? *best.placed : best.snapshot;
    if (best.placed != nullptr) {
        int idx = -1;
        for (size_t i = 0; i < result.actors.size(); ++i) {
            if (&result.actors[i] == best.placed) {
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
        picked.name, picked.setID, best.depth, best.distSq, best.radiusPx, selectMode, pcHotkeys,
        edge);
    set_selection_snapshot(picked, true);
}

}  // namespace dusk::leveledit

#else

namespace dusk::leveledit {

void try_world_pick_on_click() {}
void tick_world_pick_hover() {}
void draw_pick_hover() {}

}  // namespace dusk::leveledit

#endif
