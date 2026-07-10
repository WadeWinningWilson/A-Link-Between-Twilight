// ============================================================================
// Level Editor — 1b click-to-select in world (zero mutation).
// ============================================================================

#include "dusk/leveledit/enumerate.hpp"

#if TARGET_PC

#include "dusk/main.h"
#include "dusk/settings.h"
#include "dusk/menu_pointer.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_lib.h"

#include <aurora/lib/window.hpp>
#include <aurora/rmlui.hpp>
#include <imgui.h>
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_mouse.h>

namespace dusk::leveledit {
namespace {

constexpr f32 kPickRadiusPx = 28.0f;
constexpr f32 kPickRadiusSq = kPickRadiusPx * kPickRadiusPx;

bool s_prevLeftDown = false;

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

bool rml_hover_blocks_pick() {
    auto* context = aurora::rmlui::get_context();
    if (context == nullptr) {
        return false;
    }
    Rml::Element* hover = context->GetHoverElement();
    return hover != nullptr && element_is_interactive(hover);
}

bool pick_blocked_by_ui() {
    if (dusk::menu_pointer::active()) {
        return true;
    }
    if (ImGui::GetIO().WantCaptureMouse) {
        return true;
    }
    return rml_hover_blocks_pick();
}

f32 mirror_screen_x(f32 x) {
    if (!dusk::getSettings().game.enableMirrorMode.getValue()) {
        return x;
    }
    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 width = mDoGph_gInf_c::getWidthF();
    return minX + width - (x - minX);
}

bool project_live_pos(const cXyz& worldPos, Vec& outScreen) {
    Vec src;
    src.x = worldPos.x;
    src.y = worldPos.y;
    src.z = worldPos.z;
    mDoLib_project(&src, &outScreen);
    return true;
}

bool on_screen(const Vec& screen) {
    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 minY = mDoGph_gInf_c::getMinYF();
    const f32 maxX = minX + mDoGph_gInf_c::getWidthF();
    const f32 maxY = minY + mDoGph_gInf_c::getHeightF();
    return screen.x >= minX && screen.x <= maxX && screen.y >= minY && screen.y <= maxY;
}

}  // namespace

void try_world_pick_on_click() {
    if (!dusk::g_levelEditorSession) {
        return;
    }

    float windowX = 0.0f;
    float windowY = 0.0f;
    const Uint32 mouseButtons = SDL_GetMouseState(&windowX, &windowY);
    const bool leftDown = (mouseButtons & (1u << 0)) != 0;
    const bool leftReleased = s_prevLeftDown && !leftDown;
    s_prevLeftDown = leftDown;
    if (!leftReleased) {
        return;
    }

    if (pick_blocked_by_ui()) {
        return;
    }

    f32 clickX = 0.0f;
    f32 clickY = 0.0f;
    if (!dusk::menu_pointer::map_window_mouse_to_game_screen(windowX, windowY, clickX, clickY)) {
        return;
    }
    clickX = mirror_screen_x(clickX);

    const EnumerateResult result = enumerate_room_actors();
    int bestIdx = -1;
    f32 bestDepth = 0.0f;
    bool haveBest = false;

    for (size_t i = 0; i < result.actors.size(); ++i) {
        const PlacedActor& actor = result.actors[i];
        if (actor.unspawned || actor.isSpawnPoint || actor.live == nullptr) {
            continue;
        }

        Vec screen{};
        project_live_pos(actor.live->current.pos, screen);
        if (!on_screen(screen)) {
            continue;
        }

        const f32 dx = screen.x - clickX;
        const f32 dy = screen.y - clickY;
        const f32 distSq = dx * dx + dy * dy;
        if (distSq > kPickRadiusSq) {
            continue;
        }

        if (!haveBest || screen.z < bestDepth) {
            haveBest = true;
            bestDepth = screen.z;
            bestIdx = static_cast<int>(i);
        }
    }

    if (bestIdx < 0) {
        return;
    }

    const PlacedActor& picked = result.actors[static_cast<size_t>(bestIdx)];
    set_selected_index(bestIdx);
    set_selection_snapshot(picked, true);
}

}  // namespace dusk::leveledit

#else

namespace dusk::leveledit {

void try_world_pick_on_click() {}

}  // namespace dusk::leveledit

#endif
