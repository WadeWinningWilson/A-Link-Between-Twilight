#pragma once

#include "window.hpp"

#include <functional>
#include <memory>
#include <optional>

struct MapEntry;

namespace dusk::ui {

class Pane;

Rml::String stage_option_label(const MapEntry& map, bool showInternalNames = false);
Rml::String stage_label_for_file(const Rml::String& stageFile, bool showInternalNames = false);
void populate_stage_picker(Pane& pane, std::function<Rml::String()> getStageFile,
                           std::function<void(const char*)> setStageFile,
                           bool showInternalNames = false);

class EditorWindow : public Window {
public:
    EditorWindow();
    ~EditorWindow() override;

    void update() override;
    void hide(bool close) override;

protected:
    void clear_content() noexcept override;
    void replace_content(TabBuilder builder) override;

private:
    // Bug gate 2: tear down Stage deferred tick + invalidate raw pane ptrs
    // BEFORE content components are destroyed (close / tab switch).
    void teardown_stage_tab(const char* reason) noexcept;
    // Bug gate 3: actually destroy widgets (must run outside Rml Click/tab dispatch).
    void clear_content_now(const char* reason) noexcept;
    void drain_deferred_ui();

    struct StageTabState;
    std::shared_ptr<StageTabState> mStageTabState;
    std::function<void()> mStageTabTick;

    // Deferred tab replace / clear (leave-Stage any path — gate 3 H-B).
    std::optional<TabBuilder> mPendingTabBuilder;
    bool mPendingClearOnly = false;
    bool mPendingHide = false;
    bool mPendingHideClose = false;
};

}  // namespace dusk::ui
