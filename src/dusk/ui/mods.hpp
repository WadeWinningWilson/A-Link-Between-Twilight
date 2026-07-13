#pragma once

#include "window.hpp"

namespace dusk::ui {

// ============================================
// NEW CODE — ALBW Port (Mods window — the load-order mod manager's own page)
// A top-level Dusklight menu entry (menu bar, after Editor), NOT an editor
// tab: end users manage mods without touching developer tooling. Left pane =
// the load order with grab-and-place reorder; right pane = per-mod detail
// panel (Enabled toggle, conflicts, core/variant info) — the future home of
// modder-supplied descriptions (readme) + logos and collection sub-pages.
// ============================================
class ModsWindow : public Window {
public:
    ModsWindow();
    // Safety net: closing the window while holding a grabbed mod must still
    // apply the drag's deferred order changes (one rescan).
    void hide(bool close) override;
};

}  // namespace dusk::ui
