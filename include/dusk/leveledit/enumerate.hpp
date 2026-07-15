#pragma once

// ============================================================================
// Level Editor — placed-actor enumeration + selection highlight (Phase 1a).
// ============================================================================

#include "SSystem/SComponent/c_xyz.h"
#include "SSystem/SComponent/c_sxyz.h"
#include "global.h"

#include <functional>
#include <string>
#include <vector>

class fopAc_ac_c;

namespace dusk::leveledit {

enum class EnumSource : u8 {
    Buffer = 0,
    LiveFallback = 1,
};

struct PlacedActor {
    char chunkTag[5];  // ACTR / ACT0 / SCOB / … + NUL
    char name[9];      // 8-char stage name + NUL
    s16 procname;
    s8 argument;
    u32 params;
    cXyz pos;       // display: live current when joined, else authored
    cXyz spawnPos;  // authored placement (stable for re-join after UI close)
    csXyz angle;
    cXyz scale;
    u16 setID;
    s8 layer;  // -1 = base; 0..14 = layer variant
    s8 roomNo;
    bool unspawned;
    bool isSpawnPoint;  // PLYR only — not a live-joinable placement
    fopAc_ac_c* live;
};

struct EnumerateResult {
    EnumSource source = EnumSource::LiveFallback;
    char resourceKey[48]{};
    int liveActorsInRoom = 0;
    std::vector<PlacedActor> actors;
};

// i_systemUse: allow non-editor-session callers (e.g. the twilight-border
// fallback) to read authored placements; the editor UI passes false.
EnumerateResult enumerate_room_actors(int i_roomNo = -1, bool i_systemUse = false);

// Selection for in-world highlight (session only). -1 = none.
void set_selected_index(int index);
int get_selected_index();
void set_selection_snapshot(const PlacedActor& actor, bool valid);
void clear_selection();
// Drop live actor ptr only (keep placement snapshot for highlight after UI close).
void detach_selection_live();
bool has_selection();
const PlacedActor* get_selection();

// Stage right-pane sync when selection changes (1b world-pick or list row).
using SelectionDetailHandler = std::function<void(const PlacedActor&)>;
void set_selection_detail_handler(SelectionDetailHandler handler);
std::string format_placed_actor_detail_rml(const PlacedActor& sel);

// 1b — click-to-select in world (call from dScnPly_Execute).
void try_world_pick_on_click();

// Draw highlight for current selection. Early-outs when !g_levelEditorSession.
void draw_selection_highlight();

// Session fly-cam latch (not ConfigVar). Cleared when session ends / Play boot.
void enable_session_fly_cam(bool enable);
bool session_fly_cam_enabled();
bool editor_fly_cam_active();  // session latch || debugFlyCam setting

// Gate 7 — Select Mode: freeze fly-cam mouse-look so cursor can click-pick (1b).
void enable_session_select_mode(bool enable);
bool session_select_mode_enabled();
// PC keyboard/mouse hotkeys alongside controller while Editor Stage is open.
void enable_session_pc_hotkeys(bool enable);
bool session_pc_hotkeys_enabled();
bool editor_pc_hotkeys_active();
void tick_editor_session_input();

}  // namespace dusk::leveledit
