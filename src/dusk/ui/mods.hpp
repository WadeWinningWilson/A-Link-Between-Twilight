#pragma once

#include "window.hpp"

#include "dusk/mod_loader.hpp"

#include <cstdint>
#include <vector>

namespace dusk::ui {

class Pane;

// ============================================
// NEW CODE — ALBW Port (Mods window — the load-order mod manager's own page)
// A top-level Dusklight menu entry (menu bar, after Editor), NOT an editor
// tab: end users manage mods without touching developer tooling. Left pane =
// the load order with grab-and-place reorder; right pane = per-mod detail
// panel (plain Enabled toggle or collection variant picker).
//
// The same page also hosts the .dusk package half (ModLoader): the two mod
// systems are independent (asset folders vs. loadable bundles), so they get
// their own sections in the one list instead of two menu entries.
// ============================================
class ModsWindow : public Window {
public:
    ModsWindow();
    // Safety net: closing the window while holding a grabbed mod must still
    // apply the drag's deferred order changes (one rescan).
    void hide(bool close) override;
    // Package state changes land asynchronously (enable/disable/reload are applied on
    // ModLoader::tick), so the tab rebuilds off a state snapshot.
    void update() override;

private:
    struct PackageSnapshot {
        mods::LoadedMod* mod = nullptr;
        bool active = false;
        bool loadFailed = false;
        bool enabled = false;
        bool suspended = false;
        uint32_t cacheGeneration = 0;
    };

    void build_packages(Pane& listPane, Pane& detailPane);
    void build_package_detail(Pane& pane, mods::LoadedMod& mod);
    void take_package_snapshot();
    bool package_snapshot_dirty();
    bool content_has_focus() const;

    std::vector<PackageSnapshot> mPackages;
    std::vector<Component*> mPackageRows;
    std::vector<mods::LoadedMod*> mPackageRowMods;
    // The package whose detail (and mod-provided panels) the right pane currently shows;
    // null whenever the pane belongs to the load-order half or Options instead.
    mods::LoadedMod* mSelectedPackage = nullptr;
};

}  // namespace dusk::ui
