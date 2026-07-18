#include "pane.hpp"

#include "Z2AudioLib/Z2SeMgr.h"
#include "m_Do/m_Do_audio.h"
#include "ui.hpp"

#include <algorithm>

namespace dusk::ui {
namespace {

Rml::Element* createRoot(Rml::Element* parent) {
    auto* doc = parent->GetOwnerDocument();
    auto elem = doc->CreateElement("pane");
    return parent->AppendChild(std::move(elem));
}

}  // namespace

Pane::Pane(Rml::Element* parent, Type type) : FluentComponent(createRoot(parent)), mType(type) {
    listen(Rml::EventId::Keydown, [this](Rml::Event& event) {
        const auto cmd = map_nav_event(event);

        // If navigating to the next pane, select the focused item
        if (mType == Type::Controlled && cmd == NavCommand::Right) {
            auto* target = event.GetTargetElement();
            int focusedChild = -1;
            for (size_t i = 0; i < mChildren.size(); ++i) {
                if (mChildren[i]->contains(target)) {
                    focusedChild = i;
                    break;
                }
            }
            if (focusedChild == -1) {
                return;
            }
            set_selected_item(focusedChild);
            return;
        }

        int direction = 0;
        if (cmd == NavCommand::Down) {
            direction = 1;
        } else if (cmd == NavCommand::Up) {
            direction = -1;
        } else {
            return;
        }
        auto* target = event.GetTargetElement();
        int focusedChild = -1;
        for (size_t i = 0; i < mChildren.size(); ++i) {
            if (mChildren[i]->contains(target)) {
                focusedChild = i;
                break;
            }
        }
        if (focusedChild == -1) {
            return;
        }
        int i = focusedChild + direction;
        while (i >= 0 && i < mChildren.size()) {
            if (mChildren[i]->focus()) {
                mDoAud_seStartMenu(kSoundItemFocus);
                event.StopPropagation();
                break;
            }
            i += direction;
        }
    });

    if (type == Type::Uncontrolled) {
        // Wheel + pad scroll: detail panes are mostly static text/images with few
        // focusables, so Up/Down would otherwise dead-end (Window eats Down) or jump
        // to the tab bar without moving the scroll offset.
        listen(Rml::EventId::Mousescroll, [this](Rml::Event& event) {
            const float delta_y = event.GetParameter<float>("delta_y", 0.0f);
            if (delta_y != 0.0f) {
                scroll_by(-delta_y * 32.0f);
                event.StopPropagation();
            }
        }, true);

        listen(Rml::EventId::Keydown, [this](Rml::Event& event) {
            const auto cmd = map_nav_event(event);
            if (cmd != NavCommand::Up && cmd != NavCommand::Down) {
                return;
            }
            auto* target = event.GetTargetElement();
            if (!contains(target)) {
                return;
            }

            const int direction = cmd == NavCommand::Down ? 1 : -1;
            int focusedChild = -1;
            for (size_t i = 0; i < mChildren.size(); ++i) {
                if (mChildren[i]->contains(target)) {
                    focusedChild = static_cast<int>(i);
                    break;
                }
            }

            int i = focusedChild + direction;
            while (i >= 0 && i < static_cast<int>(mChildren.size())) {
                if (mChildren[i]->focus()) {
                    mDoAud_seStartMenu(kSoundItemFocus);
                    event.StopPropagation();
                    return;
                }
                i += direction;
            }

            scroll_by(static_cast<float>(direction) * 48.0f);
            mDoAud_seStartMenu(kSoundItemFocus);
            event.StopPropagation();
        }, true);
    }

    if (type == Type::Controlled) {
        // For controlled panes, handle SelectButton Submit events for item selection
        listen(Rml::EventId::Submit, [this](Rml::Event& event) {
            int childIndex = -1;
            for (int i = 0; i < mChildren.size(); ++i) {
                if (event.GetTargetElement() == mChildren[i]->root()) {
                    childIndex = i;
                }
            }
            // If item already selected, deselect
            if (childIndex >= 0 && childIndex < mChildren.size() &&
                mChildren[childIndex]->selected())
            {
                childIndex = -1;
            }
            set_selected_item(childIndex);
            // If the selection was handled locally, don't allow it to bubble up to window
            if (event.GetParameter("handled", false)) {
                event.StopPropagation();
            }
        });
    }
}

void Pane::update() {
    finalize();
    Component::update();
}

void Pane::set_selected_item(int index) {
    if (mType == Type::Uncontrolled) {
        return;
    }
    for (int i = 0; i < mChildren.size(); ++i) {
        mChildren[i]->set_selected(i == index);
    }
}

Component& Pane::register_control(
    Component& component, Pane& nextPane, std::function<void(Pane&)> callback) {
    component.listen(component.root(), Rml::EventId::Mouseover,
        [this, &component, &nextPane, callback](Rml::Event&) {
            if (component.disabled()) {
                return;
            }
            bool anySelected = false;
            for (const auto& child : mChildren) {
                if (child->selected()) {
                    anySelected = true;
                    break;
                }
            }
            if (!anySelected) {
                nextPane.clear();
                if (callback) {
                    callback(nextPane);
                }
            }
        });
    component.listen(component.root(), Rml::EventId::Focus,
        [this, &component, &nextPane, callback = std::move(callback)](Rml::Event&) {
            if (component.disabled()) {
                return;
            }
            nextPane.clear();

            // If an item is already selected, deselect
            for (const auto& child : mChildren) {
                if (child->selected()) {
                    set_selected_item(-1);
                    break;
                }
            }

            if (callback) {
                callback(nextPane);
            }
        });
    return component;
}

bool Pane::focus() {
    // Focus the first selected child
    for (const auto& child : mChildren) {
        if (child->selected() && child->focus()) {
            return true;
        }
    }
    // Otherwise, focus the first focusable child
    for (const auto& child : mChildren) {
        if (child->focus()) {
            return true;
        }
    }
    return false;
}

Rml::Element* Pane::add_section(const Rml::String& text) {
    auto* elem = append(mRoot, "div");
    elem->SetClass("section-heading", true);
    elem->SetInnerRML(escape(text));
    return elem;
}

Rml::Element* Pane::add_text(const Rml::String& text) {
    auto* elem = append(mRoot, "div");
    elem->SetInnerRML(escape(text));
    return elem;
}

Rml::Element* Pane::add_rml(const Rml::String& rml) {
    auto* elem = append(mRoot, "div");
    elem->SetInnerRML(rml);
    return elem;
}

void Pane::finalize() {
    if (finalized) {
        return;
    }
    finalized = true;

    // Append spacer element to the bottom. RmlUi does not properly handle
    // padding-bottom or margin-bottom on a scrollable flex container, so
    // we need to create a fake spacer with an actual layout height to get
    // padding at the bottom of a scrollable container.
    append(mRoot, "spacer");
}

void Pane::reset_scroll() {
    if (mRoot != nullptr) {
        mRoot->SetScrollTop(0.0f);
    }
}

void Pane::scroll_by(float delta_pixels) {
    if (mRoot == nullptr || delta_pixels == 0.0f) {
        return;
    }
    const float next = std::max(0.0f, mRoot->GetScrollTop() + delta_pixels);
    mRoot->SetScrollTop(next);
}

void Pane::clear() {
    clear_children();
    finalized = false;
    reset_scroll();
}

}  // namespace dusk::ui
