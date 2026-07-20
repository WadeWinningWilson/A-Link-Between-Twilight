#include "warp.hpp"

#include "editor.hpp"
#include "pane.hpp"

#include "dusk/debug_warp.h"
#include "dusk/map_loader_definitions.h"
#include "fmt/format.h"

#if TARGET_PC
#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_ext_save_guard.h"
#include "m_Do/m_Do_graphic.h"
#endif

#include <string>

namespace dusk::ui {

// ============================================
// NEW CODE — ALBW Port (alpha cleanup: warp-menu safety)
// One-shot suppression of arrival story triggers for warp-menu
// transitions. See dusk/debug_warp.h.
// ============================================
static bool sDebugWarpStorySuppress = false;
static s8 sRoomLayerOverride = -1;
static bool sClearRoomLayerOverrideOnStageLoad = false;

void markDebugWarpStorySuppress() {
    sDebugWarpStorySuppress = true;
}

bool consumeDebugWarpStorySuppress() {
    const bool pending = sDebugWarpStorySuppress;
    sDebugWarpStorySuppress = false;
    return pending;
}

void setRoomLayerOverride(s8 layer) {
    sRoomLayerOverride = layer;
    // Keep the override through the stage load this warp is about to trigger.
    sClearRoomLayerOverrideOnStageLoad = false;
}

s8 getRoomLayerOverride() {
    return sRoomLayerOverride;
}

void onStageLoadRoomLayerOverride() {
    if (sClearRoomLayerOverrideOnStageLoad) {
        sRoomLayerOverride = -1;
    }
    sClearRoomLayerOverrideOnStageLoad = true;
}

namespace {

constexpr int kMinLayer = -1;
constexpr int kMaxLayer = 14;

struct WarpSelectionState {
    int regionIdx = 0;
    int mapIdx = 0;
    int roomIdx = 0;
    int pointIdx = 0;
    int layer = -1;
    bool showInternalNames = false;
};

WarpSelectionState& selection_state() {
    static WarpSelectionState state;
    return state;
}

const RegionEntry* selected_region(const WarpSelectionState& state) {
    if (state.regionIdx < 0 || state.regionIdx >= static_cast<int>(gameRegions.size())) {
        return nullptr;
    }
    return &gameRegions[state.regionIdx];
}

const MapEntry* selected_map(const WarpSelectionState& state) {
    const auto* region = selected_region(state);
    if (region == nullptr || state.mapIdx < 0 || state.mapIdx >= static_cast<int>(region->maps.size())) {
        return nullptr;
    }
    return &region->maps[state.mapIdx];
}

const RoomEntry* selected_room(const WarpSelectionState& state) {
    const auto* map = selected_map(state);
    if (map == nullptr || state.roomIdx < 0 || state.roomIdx >= static_cast<int>(map->mapRooms.size())) {
        return nullptr;
    }
    return &map->mapRooms[state.roomIdx];
}

const s16* selected_point(const WarpSelectionState& state) {
    const auto* room = selected_room(state);
    if (room == nullptr || state.pointIdx < 0 ||
        state.pointIdx >= static_cast<int>(room->roomPoints.size()))
    {
        return nullptr;
    }
    return &room->roomPoints[state.pointIdx];
}

void clamp_indices(WarpSelectionState& state) {
    if (gameRegions.empty()) {
        state.regionIdx = -1;
        state.mapIdx = -1;
        state.roomIdx = -1;
        state.pointIdx = -1;
        state.layer = std::clamp(state.layer, kMinLayer, kMaxLayer);
        return;
    }

    state.regionIdx = std::clamp(state.regionIdx, 0, static_cast<int>(gameRegions.size()) - 1);
    const auto& region = gameRegions[state.regionIdx];
    if (region.maps.empty()) {
        state.mapIdx = -1;
        state.roomIdx = -1;
        state.pointIdx = -1;
        state.layer = std::clamp(state.layer, kMinLayer, kMaxLayer);
        return;
    }

    state.mapIdx = std::clamp(state.mapIdx, 0, static_cast<int>(region.maps.size()) - 1);
    const auto& map = region.maps[state.mapIdx];
    if (map.mapRooms.empty()) {
        state.roomIdx = -1;
        state.pointIdx = -1;
        state.layer = std::clamp(state.layer, kMinLayer, kMaxLayer);
        return;
    }

    state.roomIdx = std::clamp(state.roomIdx, 0, static_cast<int>(map.mapRooms.size()) - 1);
    const auto& room = map.mapRooms[state.roomIdx];
    if (room.roomPoints.empty()) {
        state.pointIdx = -1;
    } else {
        state.pointIdx = std::clamp(state.pointIdx, 0, static_cast<int>(room.roomPoints.size()) - 1);
    }

    state.layer = std::clamp(state.layer, kMinLayer, kMaxLayer);
}

bool can_warp(const WarpSelectionState& state) {
    return selected_point(state) != nullptr;
}

void reset_selection(WarpSelectionState& state) {
    state.roomIdx = 0;
    state.pointIdx = 0;
    state.layer = kMinLayer;
    clamp_indices(state);
}

void populate_map_picker(Pane& pane, WarpSelectionState& state) {
    pane.clear();
    clamp_indices(state);
    if (state.regionIdx < 0 || state.regionIdx >= static_cast<int>(gameRegions.size())) {
        return;
    }

    pane.add_button({
                    .text = "Show Internal Names",
                    .isSelected = [&state] { return state.showInternalNames; },
                })
        .on_pressed([&pane, &state] {
            mDoAud_seStartMenu(kSoundItemChange);
            state.showInternalNames = !state.showInternalNames;
            populate_map_picker(pane, state);
        });

    pane.add_section("Maps");
    const auto& region = gameRegions[state.regionIdx];
    for (int i = 0; i < static_cast<int>(region.maps.size()); ++i) {
        pane.add_button({
                        .text = stage_option_label(region.maps[i], state.showInternalNames),
                        .isSelected = [i, &state] { return state.mapIdx == i; },
                    })
            .on_pressed([i, &state] {
                mDoAud_seStartMenu(kSoundItemChange);
                if (state.mapIdx != i) {
                    state.mapIdx = i;
                    reset_selection(state);
                }
            });
    }
}

}  // namespace

WarpWindow::WarpWindow() {
    add_tab("Warp", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);
        auto& state = selection_state();
        clamp_indices(state);

        leftPane.add_section("Destination");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Region",
                .getValue =
                    [&state] {
                        clamp_indices(state);
                        const auto* region = selected_region(state);
                        return region == nullptr ? Rml::String{"None"} :
                                                   Rml::String{region->regionName};
                    },
            }),
            rightPane, [&state](Pane& pane) {
                pane.clear();
                for (int i = 0; i < static_cast<int>(gameRegions.size()); ++i) {
                    pane.add_button({
                                    .text = gameRegions[i].regionName,
                                    .isSelected = [i, &state] { return state.regionIdx == i; },
                                })
                        .on_pressed([i, &state] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            if (state.regionIdx != i) {
                                state.regionIdx = i;
                                state.mapIdx = 0;
                                reset_selection(state);
                            }
                        });
                }
            });

        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Map",
                .getValue =
                    [&state] {
                        clamp_indices(state);
                        const auto* map = selected_map(state);
                        return map == nullptr ? Rml::String{"None"} :
                                                stage_option_label(*map, state.showInternalNames);
                    },
            }),
            rightPane, [&state](Pane& pane) { populate_map_picker(pane, state); });

        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Room",
                .getValue = [&state] {
                        clamp_indices(state);
                        const auto* room = selected_room(state);
                        return room == nullptr ? Rml::String{"None"} :
                                                 fmt::format("{}", room->roomNo);
                    },
                .isDisabled = [&state] {
                        clamp_indices(state);
                        const auto* map = selected_map(state);
                        return map == nullptr || map->mapRooms.size() <= 1;
                    },
            }),
            rightPane, [&state](Pane& pane) {
                pane.clear();
                clamp_indices(state);
                if (state.regionIdx < 0 || state.regionIdx >= static_cast<int>(gameRegions.size())) {
                    return;
                }
                const auto& region = gameRegions[state.regionIdx];
                if (state.mapIdx < 0 || state.mapIdx >= static_cast<int>(region.maps.size())) {
                    return;
                }
                const auto& map = region.maps[state.mapIdx];
                for (int i = 0; i < static_cast<int>(map.mapRooms.size()); ++i) {
                    pane.add_button({
                                    .text = fmt::format("{}", map.mapRooms[i].roomNo),
                                    .isSelected = [i, &state] { return state.roomIdx == i; },
                                })
                        .on_pressed([i, &state] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            if (state.roomIdx != i) {
                                state.roomIdx = i;
                                state.pointIdx = 0;
                                clamp_indices(state);
                            }
                        });
                }
            });

        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Point",
                .getValue = [&state] {
                        clamp_indices(state);
                        const auto* point = selected_point(state);
                        return point == nullptr ? Rml::String{"None"} : fmt::format("{}", *point);
                    },
                .isDisabled = [&state] {
                        clamp_indices(state);
                        const auto* room = selected_room(state);
                        return room == nullptr || room->roomPoints.size() <= 1;
                    },
            }),
            rightPane, [&state](Pane& pane) {
                pane.clear();
                clamp_indices(state);
                if (state.regionIdx < 0 || state.regionIdx >= static_cast<int>(gameRegions.size())) {
                    return;
                }
                const auto& region = gameRegions[state.regionIdx];
                if (state.mapIdx < 0 || state.mapIdx >= static_cast<int>(region.maps.size())) {
                    return;
                }
                const auto& map = region.maps[state.mapIdx];
                if (state.roomIdx < 0 || state.roomIdx >= static_cast<int>(map.mapRooms.size())) {
                    return;
                }
                const auto& room = map.mapRooms[state.roomIdx];
                for (int i = 0; i < static_cast<int>(room.roomPoints.size()); ++i) {
                    pane.add_button({
                                    .text = fmt::format("{}", room.roomPoints[i]),
                                    .isSelected = [i, &state] { return state.pointIdx == i; },
                                })
                        .on_pressed([i, &state] {
                            if (state.pointIdx != i) {
                                mDoAud_seStartMenu(kSoundItemChange);
                                state.pointIdx = i;
                                clamp_indices(state);
                            }
                        });
                }
            });

        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Room layer override",
                .getValue =
                    [&state] {
                        return state.layer < 0 ? std::string("Auto") : fmt::format("{}", state.layer);
                    },
            }),
            rightPane, [&state](Pane& pane) {
                pane.clear();
                for (int layer = kMinLayer; layer <= kMaxLayer; ++layer) {
                    pane.add_button({
                                    .text = layer < 0 ? std::string("Auto") : fmt::format("{}", layer),
                                    .isSelected = [layer, &state] { return state.layer == layer; },
                                })
                        .on_pressed([layer, &state] {
                            if (state.layer != layer) {
                                mDoAud_seStartMenu(kSoundItemChange);
                                state.layer = layer;
                            }
                        });
                }
            });

        leftPane.add_section("Action");
#if TARGET_PC
        // №99 R2: warp rows from mod manifests (warp_label=). Labels stay data-side —
        // never compile WW-facing strings into map_loader_definitions.h.
        {
            const int nProv = dExtNpcMount_providerCount();
            bool sectionOpened = false;
            for (int pi = 0; pi < nProv; ++pi) {
                dExtNpcManifest man{};
                if (!dExtNpcMount_providerAt(pi, &man) || !man.isBg || man.warpLabel[0] == '\0' ||
                    !man.hasHostPos || !man.hasAnchor || !man.hasSpawnRel ||
                    man.hostStage[0] == '\0') {
                    continue;
                }
                if (!sectionOpened) {
                    leftPane.add_section(man.modFolder[0] ? man.modFolder : "Mod Stages");
                    sectionOpened = true;
                }
                const std::string proc = man.proc;
                const std::string label = man.warpLabel;
                const std::string hostStage = man.hostStage;
                const int hostRoom = man.hostRoom;
                const int hostLayer = man.hostLayer;
                leftPane.register_control(
                    leftPane
                        .add_button({
                            .text = label,
                            .isDisabled =
                                [proc] { return !dExtNpcMount_hasPayload(proc.c_str()); },
                        })
                        .on_pressed([proc, hostStage, hostRoom, hostLayer] {
                            if (!dExtNpcMount_hasPayload(proc.c_str())) {
                                return;
                            }
                            mDoAud_seStartMenu(kSoundClick);
                            markDebugWarpStorySuppress();
                            setRoomLayerOverride(static_cast<s8>(hostLayer));
                            const char* cur = dComIfGp_getStartStageName();
                            const bool foreign =
                                cur == NULL || std::strcmp(cur, hostStage.c_str()) != 0;
                            if (foreign) {
                                // №84/№85: foreign host = native stage change only. Room-lane
                                // mount binds onRoomObjectsReady inside the new play scene.
                                // Clear door fade/demo so ChangeReq wipe can finish.
                                dExtNpcManifest man{};
                                if (dExtNpcMount_lookup(proc.c_str(), &man) && man.isBg &&
                                    man.hostRoom >= 0) {
                                    dExtNpcMount_registerRoomLane(proc.c_str(), man.hostRoom);
                                }
                                dExtNpcMount_cancelTransports();
                                dExtNpcMount_endDoorDemoLock();
                                mDoGph_gInf_c::offFade();
                                if (!dComIfGp_isEnableNextStage()) {
                                    // №90: WW hosts always use authored PLYR point 0 (never -1).
                                    const s8 point = 0;
                                    dComIfGp_setNextStage(hostStage.c_str(), point,
                                                          static_cast<s8>(hostRoom),
                                                          static_cast<s8>(hostLayer));
                                    dExtNpcDoors_armArrivalGuard(hostStage.c_str());
                                }
                            } else if (!dExtNpcMount_requestBgWarp(proc.c_str())) {
                                return;
                            }
                        }),
                    rightPane, [label, hostStage, hostRoom, proc](Pane& pane) {
                        pane.clear();
                        pane.add_text(fmt::format(
                            "{} — BG warp:\n"
                            "1) If host stage {} differs from current → native setNextStage "
                            "(play scene recreates; mount on room-ready).\n"
                            "2) Same host → pinned BG warp + relocate to spawn_rel for '{}'.\n"
                            "3) Needs the mod providing that payload enabled with arc + ini.",
                            label, hostStage, proc));
                    });
            }
        }
#endif
        leftPane.register_control(
            leftPane.add_button({
                        .text = "Warp",
                        .isDisabled = [&state] {
                            clamp_indices(state);
                            return !can_warp(state);
                        },
                    })
                .on_pressed([&state] {
                    clamp_indices(state);
                    if (!can_warp(state)) {
                        return;
                    }

                    mDoAud_seStartMenu(kSoundClick);
                    const auto& region = gameRegions[state.regionIdx];
                    const auto& map = region.maps[state.mapIdx];
                    const auto& room = map.mapRooms[state.roomIdx];
                    // ============================================
                    // Warp-menu safety (alpha cleanup):
                    // 1. Suppress arrival story triggers for this one
                    //    transition (Lanayru/Eldin DarkClearLV etc.) so a
                    //    debug warp can't silently mutate progression.
                    // 2. Reset the Layer selector to auto after every
                    //    warp — a stale forced layer (e.g. twilight 14)
                    //    used to persist and desync enemy layout from the
                    //    province-flag-driven atmosphere.
                    // True-ALBW note: both are transition-scoped; the
                    // TRUETEST bootstrap/twilight-bubble policies run in
                    // dusk::truetest::onStageLoad and are unaffected.
                    // ============================================
                    markDebugWarpStorySuppress();
                    setRoomLayerOverride(static_cast<s8>(state.layer));
                    s16 point = room.roomPoints[state.pointIdx];
                    // №90: WW host stages must never arm restart point (-1).
                    if (dExtWwSave_isWwHostStage(map.mapFile) && point < 0) {
                        point = 0;
                    }
                    dComIfGp_setNextStage(map.mapFile, point, room.roomNo, state.layer);
                    if (dExtWwSave_isWwHostStage(map.mapFile)) {
                        dExtNpcDoors_armArrivalGuard(map.mapFile);
                    }
                    // UI snaps back to Auto; override persists for this stage load only.
                    state.layer = kMinLayer;
                }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_text("Warp to the selected destination.");
            });
    });
}

}  // namespace dusk::ui
