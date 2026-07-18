// ============================================
// NEW CODE — ALBW Port (Mods window — load-order mod manager UI)
// See mods.hpp. Interaction model:
//   - Left pane lists mods in PRIORITY order (top wins conflicts), one
//     POSITION-BOUND row per mod or collapsed collection (variants live in
//     the right pane), with per-frame labels (order_view — a cheap setting-
//     string parse, no disk I/O), so moves update in place without rebuilding
//     the pane from inside a button's own event handler.
//   - GRAB-AND-PLACE reorder: A/click grabs the focused mod (highlight + > <
//     markers); Up/Down then MOVES it one slot per press — the pane's normal
//     focus move runs after ours and lands on the adjacent row, which now
//     shows the grabbed mod, so the cursor rides along; A places it. Clicking
//     a DIFFERENT row while holding drops the held mod AT that row
//     (move_mod_group_to — any distance, one rescan).
//   - Right pane: per-mod panel with Enabled toggle (plain mods) or a variant
//     picker (collections) + conflict/core info — modder readme + logos.
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
#include <unordered_map>
#include <vector>

namespace dusk::ui {
namespace {

struct ModUiGroup {
    std::string key;
    bool is_collection = false;
    std::vector<std::string> members;
};

std::vector<ModUiGroup> build_ui_groups(const std::vector<std::string>& folders) {
    std::vector<ModUiGroup> groups;
    std::unordered_map<std::string, size_t> index;
    for (const std::string& path : folders) {
        const std::string key = dusk::custom_assets::mod_group_key(path.c_str());
        const bool is_variant = dusk::custom_assets::mod_is_collection_variant(path.c_str());
        auto it = index.find(key);
        if (it == index.end()) {
            index.emplace(key, groups.size());
            groups.push_back({key, is_variant, {path}});
        } else {
            groups[it->second].members.push_back(path);
            groups[it->second].is_collection =
                groups[it->second].is_collection || is_variant;
        }
    }
    return groups;
}

using ModUiGroupMap = std::unordered_map<std::string, ModUiGroup>;

ModUiGroupMap build_ui_group_map(const std::vector<ModUiGroup>& groups) {
    ModUiGroupMap out;
    for (const ModUiGroup& group : groups) {
        out.emplace(group.key, group);
    }
    return out;
}

std::vector<std::string> group_keys_in_priority_order(
    const std::vector<std::string>& folders) {
    std::vector<std::string> keys;
    for (const auto& entry : dusk::custom_assets::order_view(folders)) {
        const std::string key = dusk::custom_assets::mod_group_key(entry.first.c_str());
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            keys.push_back(key);
        }
    }
    return keys;
}

const ModUiGroup* group_at_slot(const std::vector<std::string>& folders,
                                 const ModUiGroupMap& groups_by_key, size_t slot) {
    const std::vector<std::string> keys = group_keys_in_priority_order(folders);
    if (slot >= keys.size()) {
        return nullptr;
    }
    const auto it = groups_by_key.find(keys[slot]);
    if (it == groups_by_key.end()) {
        return nullptr;
    }
    return &it->second;
}

std::string group_row_title(const ModUiGroup& group) {
    if (group.members.empty()) {
        return group.key;
    }
    if (!group.is_collection) {
        return dusk::custom_assets::display_name(group.members[0].c_str());
    }
    const std::string display = dusk::custom_assets::display_name(group.members[0].c_str());
    const auto colon = display.find(": ");
    if (colon != std::string::npos) {
        return display.substr(0, colon);
    }
    return group.key;
}

bool group_any_enabled(const ModUiGroup& group) {
    for (const std::string& member : group.members) {
        if (dusk::custom_assets::is_folder_enabled(member.c_str())) {
            return true;
        }
    }
    return false;
}

std::string group_active_variant_name(const ModUiGroup& group) {
    for (const std::string& member : group.members) {
        if (dusk::custom_assets::is_folder_enabled(member.c_str())) {
            return dusk::custom_assets::display_name(member.c_str());
        }
    }
    return {};
}

dusk::custom_assets::FolderConflicts group_conflicts(const ModUiGroup& group) {
    dusk::custom_assets::FolderConflicts total;
    for (const std::string& member : group.members) {
        if (!dusk::custom_assets::is_folder_enabled(member.c_str())) {
            continue;
        }
        const auto c = dusk::custom_assets::folder_conflicts(member.c_str());
        total.wins += c.wins;
        total.losses += c.losses;
        total.overridesCore = total.overridesCore || c.overridesCore;
        total.overriddenByCore = total.overriddenByCore || c.overriddenByCore;
    }
    return total;
}

const std::string* group_panel_folder(const ModUiGroup& group) {
    if (group.members.empty()) {
        return nullptr;
    }
    for (const std::string& member : group.members) {
        if (dusk::custom_assets::is_folder_enabled(member.c_str())) {
            return &member;
        }
    }
    return &group.members[0];
}

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

void populate_mod_detail_panel(Pane& pane, const ModUiGroup& group) {
    const std::string* panel_folder = group_panel_folder(group);
    if (panel_folder == nullptr) {
        return;
    }

    const auto meta = dusk::custom_assets::mod_info(panel_folder->c_str());
    pane.add_section(meta.name.empty() ? group_row_title(group) : meta.name);

    if (group.is_collection) {
        pane.add_rml(fmt::format(
            "<b>Collection</b> — {} variant(s). Pick one to enable; siblings disable "
            "automatically.<br/>",
            group.members.size()));
        pane.add_section("Variants");
        for (const std::string& variant : group.members) {
            const std::string label = dusk::custom_assets::display_name(variant.c_str());
            pane.add_button({
                                  .text = label,
                                  .isSelected =
                                      [variant] {
                                          return dusk::custom_assets::is_folder_enabled(
                                              variant.c_str());
                                      },
                              })
                .on_pressed([variant] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dusk::custom_assets::select_collection_variant(variant.c_str());
                    config::Save();
                });
        }
    } else {
        const std::string& name = group.members[0];
        pane.add_button({
                            .text = "Enabled",
                            .isSelected =
                                [name] {
                                    return dusk::custom_assets::is_folder_enabled(name.c_str());
                                },
                        })
            .on_pressed([name] {
                mDoAud_seStartMenu(kSoundItemChange);
                dusk::custom_assets::toggle_folder(name.c_str());
                config::Save();
            });
    }

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

    const auto c = group_conflicts(group);
    std::string info;
    if (c.wins != 0 || c.losses != 0) {
        info += fmt::format("Conflicts: wins <b>{}</b> asset(s), loses <b>{}</b> to higher "
                            "mods.<br/>",
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
        "<br/>Changes apply the next time the asset loads (textures apply instantly).";
    if (meta.description.empty() && shot.empty()) {
        tail += "<br/><br/><i>Mods can describe themselves here: add a modinfo.ini (name, "
                "version, author, description, screenshot) to the mod folder.</i>";
    }
    pane.add_rml(info + tail);
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
        const auto folders =
            std::make_shared<const std::vector<std::string>>(dusk::custom_assets::list_folders());
        if (folders->empty()) {
            leftPane.add_rml(
                "No mods found. Drop a folder under <b>model_replacements/</b> (a full-mod "
                "<b>files/</b> tree, loose <i>&lt;arc&gt;_&lt;idx&gt;.bmd</i>, <b>icons/</b>, "
                "or <b>textures/</b>) and reopen this menu.");
        } else {
            const auto groups_by_key = std::make_shared<const ModUiGroupMap>(
                build_ui_group_map(build_ui_groups(*folders)));
            const size_t group_count = group_keys_in_priority_order(*folders).size();
            for (size_t i = 0; i < group_count; ++i) {
                auto& row = leftPane.add_child<ModOrderRowButton>(
                    ControlledButton::Props{
                        .text = fmt::format("{}. mod group", i + 1),
                        .isSelected = [i] { return s_grabbedSlot == static_cast<int>(i); },
                    },
                    [folders, groups_by_key, i]() -> Rml::String {
                        const ModUiGroup* group = group_at_slot(*folders, *groups_by_key, i);
                        if (group == nullptr) {
                            return Rml::String("-");
                        }
                        const auto c = group_conflicts(*group);
                        std::string badge;
                        if (c.wins != 0 || c.losses != 0) {
                            badge = fmt::format("  [{}w/{}l{}]", c.wins, c.losses,
                                                c.overridesCore       ? " CORE!"
                                                : c.overriddenByCore  ? " core"
                                                                      : "");
                        }
                        const bool grabbed = s_grabbedSlot == static_cast<int>(i);
                        std::string title = group_row_title(*group);
                        if (group->is_collection && group->members.size() > 1) {
                            title += fmt::format("  ({} variants)", group->members.size());
                        }
                        std::string status = group_any_enabled(*group) ? "ON" : "OFF";
                        if (group->is_collection) {
                            const std::string active = group_active_variant_name(*group);
                            if (!active.empty()) {
                                status += " — " + active;
                            }
                        }
                        return fmt::format("{}{}. {} — {}{}{}", grabbed ? "> " : "", i + 1,
                                           title, status, badge, grabbed ? " <" : "");
                    });
                row.on_nav_command([folders, groups_by_key, i](Rml::Event&, NavCommand cmd) -> bool {
                    const int slot = static_cast<int>(i);
                    const std::vector<std::string> keys =
                        group_keys_in_priority_order(*folders);
                    if (cmd == NavCommand::Confirm) {
                        if (i >= keys.size()) {
                            return true;
                        }
                        if (s_grabbedSlot < 0) {
                            s_grabbedSlot = slot;
                        } else if (s_grabbedSlot == slot) {
                            s_grabbedSlot = -1;
                            apply_pending_order();
                        } else {
                            if (s_grabbedSlot >= static_cast<int>(keys.size())) {
                                apply_pending_order();
                            } else if (dusk::custom_assets::move_mod_group_to(
                                           keys[s_grabbedSlot].c_str(), slot)) {
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
                        const int target = slot + delta;
                        if (i >= keys.size() || target < 0 ||
                            target >= static_cast<int>(keys.size()))
                        {
                            return true;
                        }
                        const std::string& group_key = keys[i];
                        if (dusk::custom_assets::move_mod_group(group_key.c_str(), delta,
                                                              /*apply=*/false)) {
                            s_grabbedSlot = target;
                            s_orderDirty = true;
                            config::Save();
                            mDoAud_seStartMenu(kSoundItemChange);
                        }
                        return false;
                    }
                    return false;
                });
                leftPane.register_control(row, rightPane, [folders, groups_by_key, i](Pane& pane) {
                    pane.clear();
                    const ModUiGroup* group = group_at_slot(*folders, *groups_by_key, i);
                    if (group == nullptr) {
                        return;
                    }
                    populate_mod_detail_panel(pane, *group);
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
