// ============================================
// NEW CODE — ALBW Port (Mods window — load-order mod manager UI)
// See mods.hpp. Interaction model:
//   - Left pane lists mods in PRIORITY order (top wins conflicts), one
//     POSITION-BOUND row per mod with per-frame labels (order_view — a cheap
//     setting-string parse, no disk I/O), so moves update in place without
//     rebuilding the pane from inside a button's own event handler.
//   - GRAB-AND-PLACE reorder: A/click grabs the focused mod (highlight + > <
//     markers); Up/Down then MOVES it one slot per press — the pane's normal
//     focus move runs after ours and lands on the adjacent row, which now
//     shows the grabbed mod, so the cursor rides along; A places it. Clicking
//     a DIFFERENT row while holding drops the held mod AT that row
//     (move_folder_to — any distance, one rescan).
//   - Right pane: per-mod panel with the Enabled toggle (this is why A is
//     free to grab) + conflict/core/variant info. Future: modder-supplied
//     readme descriptions + logos, collection sub-pages.
//   - "Allow Core Override" (D4) lives under Options.
// ============================================

#include "mods.hpp"

#include <aurora/lib/gfx/png_io.hpp>  // load_png_file (modinfo screenshot preview)
#include <aurora/rmlui.hpp>           // register_texture_provider (modshot://)
#include <fmt/format.h>

#include "Z2AudioLib/Z2SeMgr.h"
#include "bool_button.hpp"
#include "button.hpp"
#include "dusk/config.hpp"
#include "dusk/custom_assets.hpp"
#include "dusk/settings.h"
#include "m_Do/m_Do_audio.h"
#include "pane.hpp"
#include "ui.hpp"  // escape()

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dusk::ui {
namespace {

// A ControlledButton whose label is re-read every frame — rows are
// position-bound (row i shows the mod at load-order slot i), so a move changes
// which mod a slot shows and the per-frame text keeps the list correct.
class ModOrderRowButton : public ControlledButton {
public:
    ModOrderRowButton(Rml::Element* parent, ControlledButton::Props props,
        std::function<Rml::String()> getText)
        : ControlledButton(parent, std::move(props)), mGetText(std::move(getText)) {}

    void update() override {
        if (mGetText) {
            set_text(mGetText());
        }
        ControlledButton::update();
    }

private:
    std::function<Rml::String()> mGetText;
};

// Grab-and-place state: the load-order slot currently "held" (-1 = none).
// File-scope because the row lambdas outlive the build function; reset on
// every window build.
int s_grabbedSlot = -1;
// Deferred-apply flag: Up/Down steps during a grab persist ONLY the order
// (move_folder apply=false); the rescan happens ONCE at place/drop/close.
// A 10-step drag used to fire 10 scans + 10 overlay generations — the bump
// storm that let resident-model refreshes latch stale (Linkle/Link hybrid).
bool s_orderDirty = false;

void apply_pending_order() {
    if (s_orderDirty) {
        s_orderDirty = false;
        dusk::custom_assets::apply_order_changes();
    }
}

// ============================================
// modinfo.ini screenshot preview (modshot:// texture provider)
// One slot: only one mod panel is visible at a time. The decoded RGBA stays
// resident until another mod's screenshot replaces it, so the RuntimeTexture
// span handed to RmlUi can't dangle. ?rev= in the source string busts RmlUi's
// per-source texture cache when the slot changes. PNG only for now (aurora
// ships a PNG decoder; jpg/tga/bmp would need a new dependency).
// ============================================
struct ScreenshotSlot {
    std::string path;                  // absolute path currently decoded
    aurora::gfx::ConvertedTexture tex; // RGBA8
    uint64_t rev = 0;
};
ScreenshotSlot s_shot;

std::optional<aurora::rmlui::RuntimeTexture> modshot_provider(std::string_view) {
    if (s_shot.tex.data.empty() || s_shot.tex.width == 0 || s_shot.tex.height == 0) {
        return std::nullopt;
    }
    return aurora::rmlui::RuntimeTexture{
        .width = s_shot.tex.width,
        .height = s_shot.tex.height,
        .rgba8 = std::span(reinterpret_cast<const std::byte*>(s_shot.tex.data.data()),
                           s_shot.tex.data.size()),
        .premultipliedAlpha = false,
    };
}

void ensure_modshot_provider() {
    static bool registered = false;
    if (!registered) {
        aurora::rmlui::register_texture_provider("modshot", modshot_provider);
        registered = true;
    }
}

// Load (or reuse) the screenshot for the focused mod; returns the <img> RML
// or "" when there is no usable screenshot.
std::string screenshot_rml(const std::string& absPath) {
    if (absPath.empty()) {
        return {};
    }
    std::string lower = absPath;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower.size() < 4 || lower.compare(lower.size() - 4, 4, ".png") != 0) {
        return {};  // PNG only (see provider block comment)
    }
    if (s_shot.path != absPath) {
        auto png = aurora::gfx::png::load_png_file(absPath);
        if (!png || png->width == 0 || png->height == 0) {
            return {};
        }
        s_shot.path = absPath;
        s_shot.tex = std::move(*png);
        ++s_shot.rev;
    }
    // Fit to a fixed panel width, preserving aspect.
    const uint32_t w = 300;
    const uint32_t h = std::max<uint32_t>(1, s_shot.tex.height * w / s_shot.tex.width);
    return fmt::format("<img src=\"modshot://shot?rev={}\" width=\"{}\" height=\"{}\"/>",
                       s_shot.rev, w, h);
}

// RML-escape + convert newlines (modinfo descriptions) to <br/>.
std::string rml_multiline(const std::string& text) {
    std::string out = escape(text);
    std::string withBreaks;
    withBreaks.reserve(out.size());
    for (char c : out) {
        if (c == '\n') {
            withBreaks += "<br/>";
        } else {
            withBreaks.push_back(c);
        }
    }
    return withBreaks;
}

}  // namespace

void ModsWindow::hide(bool close) {
    apply_pending_order();  // drag interrupted by closing the window
    Window::hide(close);
}

ModsWindow::ModsWindow() {
    add_tab("Mods", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        s_grabbedSlot = -1;  // never carry a grab across window rebuilds
        apply_pending_order();  // defensive: stale dirty order from a prior build
        ensure_modshot_provider();

        leftPane.add_section("Load Order — top wins");
        const auto corePacks = dusk::custom_assets::list_core_packs();
        if (!corePacks.empty()) {
            leftPane.add_rml(fmt::format(
                "<b>{} core pack(s)</b> bundled — always on, {} user mods in conflicts.",
                corePacks.size(),
                getSettings().game.customModelsAllowCoreOverride.getValue() ? "overridable by"
                                                                            : "winning over"));
        }
        const auto folders = dusk::custom_assets::list_folders();  // priority order
        if (folders.empty()) {
            leftPane.add_rml(
                "No mods found. Drop a folder under <b>model_replacements/</b> (a full-mod "
                "<b>files/</b> tree, loose <i>&lt;arc&gt;_&lt;idx&gt;.bmd</i>, <b>icons/</b>, "
                "or <b>textures/</b>) and reopen this menu.");
        } else {
            const auto names = std::make_shared<const std::vector<std::string>>(folders);
            for (size_t i = 0; i < folders.size(); ++i) {
                auto& row = leftPane.add_child<ModOrderRowButton>(
                    ControlledButton::Props{
                        .text = fmt::format("{}. {}", i + 1, folders[i]),
                        .isSelected = [i] { return s_grabbedSlot == static_cast<int>(i); },
                    },
                    [names, i]() -> Rml::String {
                        const auto view = dusk::custom_assets::order_view(*names);
                        if (i >= view.size()) {
                            return Rml::String("-");
                        }
                        const auto c =
                            dusk::custom_assets::folder_conflicts(view[i].first.c_str());
                        std::string badge;
                        if (c.wins != 0 || c.losses != 0) {
                            badge = fmt::format("  [{}w/{}l{}]", c.wins, c.losses,
                                                c.overridesCore       ? " CORE!"
                                                : c.overriddenByCore  ? " core"
                                                                      : "");
                        }
                        const bool grabbed = s_grabbedSlot == static_cast<int>(i);
                        return fmt::format("{}{}. {} — {}{}{}", grabbed ? "> " : "", i + 1,
                                           dusk::custom_assets::display_name(
                                               view[i].first.c_str()),
                                           view[i].second ? "ON" : "OFF", badge,
                                           grabbed ? " <" : "");
                    });
                row.on_nav_command([names, i](Rml::Event&, NavCommand cmd) -> bool {
                    const int slot = static_cast<int>(i);
                    if (cmd == NavCommand::Confirm) {
                        const auto view = dusk::custom_assets::order_view(*names);
                        if (i >= view.size()) {
                            return true;
                        }
                        if (s_grabbedSlot < 0) {
                            s_grabbedSlot = slot;  // grab
                        } else if (s_grabbedSlot == slot) {
                            s_grabbedSlot = -1;    // place: apply the drag's moves once
                            apply_pending_order();
                        } else {
                            // Holding another mod: drop it AT this row (mouse path).
                            // move_folder_to rescans internally, which also covers
                            // any deferred Up/Down steps taken before the drop.
                            if (s_grabbedSlot < static_cast<int>(view.size()) &&
                                dusk::custom_assets::move_folder_to(
                                    view[s_grabbedSlot].first.c_str(), slot))
                            {
                                config::Save();
                                s_orderDirty = false;
                            } else {
                                apply_pending_order();
                            }
                            s_grabbedSlot = -1;
                        }
                        mDoAud_seStartMenu(kSoundItemChange);
                        return true;
                    }
                    if ((cmd == NavCommand::Up || cmd == NavCommand::Down) &&
                        s_grabbedSlot == slot)
                    {
                        const int delta = cmd == NavCommand::Up ? -1 : 1;
                        const auto view = dusk::custom_assets::order_view(*names);
                        const int target = slot + delta;
                        if (i >= view.size() || target < 0 ||
                            target >= static_cast<int>(view.size()))
                        {
                            return true;  // list edge: consume so the grab stays put
                        }
                        // Deferred apply: persist the order only; ONE rescan at place.
                        if (dusk::custom_assets::move_folder(view[i].first.c_str(), delta,
                                                             /*apply=*/false)) {
                            s_grabbedSlot = target;
                            s_orderDirty = true;
                            config::Save();
                            mDoAud_seStartMenu(kSoundItemChange);
                        }
                        // NOT handled: the pane's own Up/Down focus move runs next and
                        // lands on the adjacent row — which now shows the grabbed mod.
                        return false;
                    }
                    return false;
                });
                leftPane.register_control(row, rightPane, [names, i](Pane& pane) {
                    pane.clear();
                    const auto view = dusk::custom_assets::order_view(*names);
                    if (i >= view.size()) {
                        return;
                    }
                    const std::string name = view[i].first;
                    const auto meta = dusk::custom_assets::mod_info(name.c_str());
                    pane.add_section(meta.name.empty() ? name : meta.name);
                    pane.add_button({
                                        .text = "Enabled",
                                        .isSelected =
                                            [name] {
                                                return dusk::custom_assets::is_folder_enabled(
                                                    name.c_str());
                                            },
                                    })
                        .on_pressed([name] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            dusk::custom_assets::toggle_folder(name.c_str());
                            config::Save();
                        });

                    // modinfo.ini preview: screenshot, version/author byline,
                    // description. All optional; escape everything user-supplied.
                    const std::string shot = screenshot_rml(meta.screenshot);
                    if (!shot.empty()) {
                        pane.add_rml(shot);
                    }
                    std::string byline;
                    if (!meta.version.empty()) {
                        byline += escape(meta.version);
                    }
                    if (!meta.author.empty()) {
                        if (!byline.empty()) {
                            byline += " — ";
                        }
                        byline += "by " + escape(meta.author);
                    }
                    if (!byline.empty()) {
                        pane.add_rml("<i>" + byline + "</i>");
                    }
                    if (!meta.description.empty()) {
                        pane.add_rml(rml_multiline(meta.description));
                    }

                    const auto c = dusk::custom_assets::folder_conflicts(name.c_str());
                    std::string info;
                    if (name.find('/') != std::string::npos) {
                        info +=
                            "Part of a <b>collection</b> — its variants replace the same "
                            "assets, so enable only one at a time.<br/>";
                    }
                    if (c.wins != 0 || c.losses != 0) {
                        info += fmt::format("Conflicts: wins <b>{}</b> asset(s), loses "
                                            "<b>{}</b> to higher mods.<br/>",
                                            c.wins, c.losses);
                    }
                    if (c.overridesCore) {
                        info += "<b>Overrides bundled core content.</b><br/>";
                    }
                    if (c.overriddenByCore) {
                        info += "Some of its assets are overridden by core content.<br/>";
                    }
                    if (info.empty()) {
                        info = "No conflicts with other enabled mods.<br/>";
                    }
                    std::string tail =
                        "<br/>Changes apply the next time the asset loads (textures apply "
                        "instantly).";
                    if (meta.description.empty() && shot.empty()) {
                        tail += "<br/><br/><i>Mods can describe themselves here: add a "
                                "modinfo.ini (name, version, author, description, "
                                "screenshot) to the mod folder.</i>";
                    }
                    pane.add_rml(info + tail);
                });
            }
        }

        leftPane.add_section("Options");
        leftPane.register_control(
            leftPane.add_child<BoolButton>(BoolButton::Props{
                .key = "Allow Core Override",
                .getValue =
                    [] { return getSettings().game.customModelsAllowCoreOverride.getValue(); },
                .setValue =
                    [](bool value) {
                        getSettings().game.customModelsAllowCoreOverride.setValue(value);
                        // Core-vs-user sequence changed — re-resolve every site.
                        dusk::custom_assets::scan();
                        dusk::custom_assets::install_overlays();
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.customModelsAllowCoreOverride.getValue() !=
                               getSettings().game.customModelsAllowCoreOverride
                                   .getDefaultValue();
                    },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Allow Core Override");
                pane.add_rml(
                    "<b>Off (recommended):</b> the game's bundled core content (boss "
                    "models, Wind Waker gear, future first-party assets) always wins when a "
                    "custom mod replaces the same asset.<br/><br/>"
                    "<b>On:</b> your custom mods win instead and can restyle core content. "
                    "<b>Warning:</b> overriding core assets can break boss fights or other "
                    "features — turn this off again if something misbehaves.");
            });
    });
}

}  // namespace dusk::ui
