// ============================================================================
// Level Editor — placed-actor enumeration + highlight (Phase 1a).
// ============================================================================

#include "d/dolzel.h"  // IWYU pragma: keep

#include "dusk/leveledit/enumerate.hpp"
#include "dusk/leveledit/raycast.hpp"

#include "d/d_com_inf_game.h"
#include "d/d_debug_viewer.h"
#include "d/d_stage.h"
#include "d/d_camera.h"
#include "dusk/main.h"
#include "dusk/settings.h"
#include "dusk/string.hpp"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>

#if TARGET_PC
#include <aurora/rmlui.hpp>
#include <RmlUi/Core/Element.h>
#include <imgui.h>
#include <SDL3/SDL_keyboard.h>
#endif

namespace dusk::leveledit {
namespace {

#if TARGET_PC
bool rml_text_input_focused() {
    auto* context = aurora::rmlui::get_context();
    if (context == nullptr) {
        return false;
    }
    const Rml::Element* focus = context->GetFocusElement();
    if (focus == nullptr) {
        return false;
    }
    const Rml::String& tag = focus->GetTagName();
    return tag == "input" || tag == "textarea" || tag == "select";
}

bool scancode_pressed_edge(SDL_Scancode scancode) {
    static bool s_prevDown[SDL_SCANCODE_COUNT]{};
    if (scancode < 0 || scancode >= SDL_SCANCODE_COUNT) {
        return false;
    }
    int keyCount = 0;
    const bool* keys = SDL_GetKeyboardState(&keyCount);
    const bool down =
        keys != nullptr && scancode < keyCount && keys[scancode];
    const bool edge = down && !s_prevDown[scancode];
    s_prevDown[scancode] = down;
    return edge;
}
#endif

STATIC_ASSERT(sizeof(fopAcM_prmBase_class) == 0x18);
STATIC_ASSERT(sizeof(stage_actor_data_class) == 0x20);
STATIC_ASSERT(sizeof(stage_tgsc_data_class) == 0x24);

constexpr u16 kNonUniqueSetID = 0xFFFF;
constexpr f32 kJoinPosEpsilon = 50.0f;  // cm — spawn vs live can drift slightly

int sSelectedIndex = -1;
bool sSelectionValid = false;
PlacedActor sSelection{};
fpc_ProcID sSelectionProcId = fpcM_ERROR_PROCESS_ID_e;
bool sSessionFlyCam = false;
bool sSessionSelectMode = false;
bool sSessionPcHotkeys = true;
SelectionDetailHandler sSelectionDetailHandler;

struct LiveKey {
    s8 roomNo;
    u16 setID;
    bool operator==(const LiveKey& o) const {
        return roomNo == o.roomNo && setID == o.setID;
    }
};

struct LiveKeyHash {
    size_t operator()(const LiveKey& k) const {
        return (static_cast<size_t>(static_cast<u8>(k.roomNo)) << 16) ^ k.setID;
    }
};

using LiveBySetId = std::unordered_map<LiveKey, fopAc_ac_c*, LiveKeyHash>;

struct LiveActorRef {
    fopAc_ac_c* actor;
    char name[9];
    cXyz homePos;  // spawn — join matches placement vs home, not current
    s8 roomNo;
};

struct LiveBuildCtx {
    LiveBySetId* bySetId;
    std::vector<LiveActorRef>* all;
    int liveInStayRoom;
    int stayRoom;
};

int build_live_cb(void* i_actor, void* i_data) {
    auto* actor = static_cast<fopAc_ac_c*>(i_actor);
    auto* ctx = static_cast<LiveBuildCtx*>(i_data);
    if (actor == nullptr || ctx == nullptr) {
        return 0;
    }

    const s8 room = fopAcM_GetRoomNo(actor);
    if (room == ctx->stayRoom) {
        ctx->liveInStayRoom++;
    }

    LiveActorRef ref{};
    ref.actor = actor;
    ref.roomNo = room;
    ref.homePos = actor->home.pos;
    char nameBuf[16]{};
    fopAcM_getNameString(actor, nameBuf);
    std::memcpy(ref.name, nameBuf, 8);
    ref.name[8] = '\0';
    ctx->all->push_back(ref);

    if (actor->setID != kNonUniqueSetID) {
        ctx->bySetId->emplace(LiveKey{room, actor->setID}, actor);
    }
    return 0;
}

struct LiveCollectCtx {
    int roomNo;
    std::vector<PlacedActor>* out;
};

int collect_live_cb(void* i_actor, void* i_data) {
    auto* actor = static_cast<fopAc_ac_c*>(i_actor);
    auto* ctx = static_cast<LiveCollectCtx*>(i_data);
    if (actor == nullptr || ctx == nullptr || ctx->out == nullptr) {
        return 0;
    }
    if (ctx->roomNo >= 0 && fopAcM_GetRoomNo(actor) != ctx->roomNo) {
        return 0;
    }

    PlacedActor placed{};
    SAFE_STRCPY(placed.chunkTag, "LIVE");
    char nameBuf[16]{};
    fopAcM_getNameString(actor, nameBuf);
    std::memcpy(placed.name, nameBuf, 8);
    placed.name[8] = '\0';
    placed.procname = fopAcM_GetName(actor);
    placed.argument = actor->argument;
    placed.params = fopAcM_GetParam(actor);
    placed.pos = actor->current.pos;
    placed.spawnPos = actor->home.pos;
    placed.angle = actor->shape_angle;
    placed.scale = actor->scale;
    placed.setID = actor->setID;
    placed.layer = -1;
    placed.roomNo = fopAcM_GetRoomNo(actor);
    placed.unspawned = false;
    placed.isSpawnPoint = false;
    placed.live = actor;
    ctx->out->push_back(placed);
    return 0;
}

bool tag_eq(u32 tag, const char* four) {
    return tag == *reinterpret_cast<const u32*>(four);
}

s8 layer_from_tag_char(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<s8>(c - '0');
    }
    if (c >= 'A' && c <= 'E') {
        return static_cast<s8>(10 + (c - 'A'));
    }
    if (c >= 'a' && c <= 'e') {
        return static_cast<s8>(10 + (c - 'a'));
    }
    return -1;
}

bool is_actor_tag(u32 tag, s8* outLayer, bool* outHasScale, char outTag[5]) {
    char t[5]{};
    std::memcpy(t, &tag, 4);
    t[4] = '\0';
    std::memcpy(outTag, t, 5);

    if (tag_eq(tag, "ACTR") || tag_eq(tag, "PLYR")) {
        *outLayer = -1;
        *outHasScale = false;
        return true;
    }
    if (tag_eq(tag, "SCOB") || tag_eq(tag, "TGSC") || tag_eq(tag, "TGDR")) {
        *outLayer = -1;
        *outHasScale = true;
        return true;
    }
    if (std::memcmp(t, "ACT", 3) == 0) {
        *outLayer = layer_from_tag_char(t[3]);
        *outHasScale = false;
        return *outLayer >= 0;
    }
    if (std::memcmp(t, "SCO", 3) == 0 || std::memcmp(t, "TGS", 3) == 0) {
        *outLayer = layer_from_tag_char(t[3]);
        *outHasScale = true;
        return *outLayer >= 0;
    }
    return false;
}

f32 dist_sq(const cXyz& a, const cXyz& b) {
    const f32 dx = a.x - b.x;
    const f32 dy = a.y - b.y;
    const f32 dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

fopAc_ac_c* join_live(int roomNo, u16 setID, const char* name, const cXyz& pos,
                      const LiveBySetId& bySetId, const std::vector<LiveActorRef>& all) {
    if (setID != kNonUniqueSetID) {
        auto it = bySetId.find(LiveKey{static_cast<s8>(roomNo), setID});
        if (it != bySetId.end()) {
            return it->second;
        }
    }

    // setID 0xFFFF: match name + nearest home.pos (spawn), not current.pos —
    // wanderers stay joined. J4 belt: sole same-name in room ignores distance.
    fopAc_ac_c* best = nullptr;
    fopAc_ac_c* uniqueSameName = nullptr;
    int sameNameCount = 0;
    f32 bestDist = kJoinPosEpsilon * kJoinPosEpsilon;
    for (const auto& ref : all) {
        if (ref.roomNo != roomNo) {
            continue;
        }
        if (std::strncmp(ref.name, name, 8) != 0) {
            continue;
        }
        ++sameNameCount;
        uniqueSameName = ref.actor;
        const f32 d = dist_sq(ref.homePos, pos);
        if (d <= bestDist) {
            bestDist = d;
            best = ref.actor;
        }
    }
    if (best != nullptr) {
        return best;
    }
    if (sameNameCount == 1) {
        return uniqueSameName;
    }
    return nullptr;
}

void fill_common(PlacedActor* placed, const char* tagStr, s8 layer, int roomNo,
                 const fopAcM_prmBase_class& base, const LiveBySetId& bySetId,
                 const std::vector<LiveActorRef>& all) {
    SAFE_STRCPY(placed->chunkTag, tagStr);
    placed->params = static_cast<u32>(base.parameters);
    placed->spawnPos = static_cast<cXyz>(base.position);
    placed->pos = placed->spawnPos;
    placed->angle = static_cast<csXyz>(base.angle);
    placed->setID = static_cast<u16>(base.setID);
    placed->layer = layer;
    placed->roomNo = static_cast<s8>(roomNo);
    placed->isSpawnPoint = (std::strncmp(tagStr, "PLYR", 4) == 0);

    if (placed->isSpawnPoint) {
        placed->live = nullptr;
        placed->unspawned = false;
        return;
    }

    // Join against authored spawn (home), not wandering current.
    placed->live = join_live(roomNo, placed->setID, placed->name, placed->spawnPos, bySetId, all);
    placed->unspawned = placed->live == nullptr;
    if (placed->live != nullptr) {
        placed->pos = placed->live->current.pos;
        placed->angle = placed->live->shape_angle;
    }
}

// Re-bind selection.live (safe after Stage UI teardown / despawn).
// Prefer process ID — setID 0xFFFF joins are ambiguous and froze the highlight on spawnPos.
void refresh_selection_live() {
    if (!sSelectionValid || sSelection.isSpawnPoint) {
        return;
    }

    fopAc_ac_c* live = nullptr;

    if (sSelectionProcId != fpcM_ERROR_PROCESS_ID_e) {
        if (base_process_class* base = fpcM_SearchByID(sSelectionProcId)) {
            live = static_cast<fopAc_ac_c*>(base);
        }
    }

    if (live == nullptr) {
        if (std::strncmp(sSelection.name, "Link", 8) == 0) {
            live = dComIfGp_getPlayer(0);
        } else if (std::strncmp(sSelection.name, "Horse", 8) == 0) {
            live = reinterpret_cast<fopAc_ac_c*>(dComIfGp_getHorseActor());
        }
        if (live != nullptr) {
            sSelectionProcId = fopAcM_GetID(live);
        }
    }

    if (live == nullptr) {
        LiveBySetId bySetId;
        std::vector<LiveActorRef> all;
        const int roomNo = sSelection.roomNo;
        LiveBuildCtx mapCtx{&bySetId, &all, 0, roomNo};
        fopAcIt_Executor(build_live_cb, &mapCtx);
        // Prefer last known pos (current at pick time) over authored spawn for 0xFFFF joins.
        const cXyz hint =
            (sSelection.pos.x != 0.0f || sSelection.pos.y != 0.0f || sSelection.pos.z != 0.0f)
                ? sSelection.pos
                : sSelection.spawnPos;
        live = join_live(roomNo, sSelection.setID, sSelection.name, hint, bySetId, all);
        if (live != nullptr) {
            sSelectionProcId = fopAcM_GetID(live);
        }
    }

    sSelection.live = live;
    sSelection.unspawned = live == nullptr;
    if (live != nullptr) {
        sSelection.pos = live->current.pos;
        sSelection.angle = live->shape_angle;
    }
}

void append_from_buffer(void* buffer, int roomNo, const LiveBySetId& bySetId,
                        const std::vector<LiveActorRef>& all, std::vector<PlacedActor>* out) {
    if (buffer == nullptr || out == nullptr) {
        return;
    }

    auto* file = static_cast<dStage_fileHeader*>(buffer);
    dStage_nodeHeader* node = file->m_nodes;
    for (int i = 0; i < file->m_chunkCount; i++, node++) {
        s8 layer = -1;
        bool hasScale = false;
        char tagStr[5]{};
        if (!is_actor_tag(node->m_tag, &layer, &hasScale, tagStr)) {
            continue;
        }

        const int num = static_cast<int>(node->m_entryNum);
        if (num <= 0) {
            continue;
        }

        if (hasScale) {
            auto* entries = static_cast<stage_tgsc_data_class*>(node->m_offset);
            if (entries == nullptr) {
                continue;
            }
            for (int e = 0; e < num; e++) {
                auto& src = entries[e];
                PlacedActor placed{};
                std::memcpy(placed.name, src.name, 8);
                placed.name[8] = '\0';
                dStage_objectNameInf* info = dStage_searchName(placed.name);
                placed.procname = info != nullptr ? info->procname : static_cast<s16>(-1);
                placed.argument = info != nullptr ? info->argument : static_cast<s8>(0);
                placed.scale.set(src.scale.x / 10.0f, src.scale.y / 10.0f, src.scale.z / 10.0f);
                fill_common(&placed, tagStr, layer, roomNo, src.base, bySetId, all);
                out->push_back(placed);
            }
        } else {
            auto* entries = static_cast<stage_actor_data_class*>(node->m_offset);
            if (entries == nullptr) {
                continue;
            }
            for (int e = 0; e < num; e++) {
                auto& src = entries[e];
                PlacedActor placed{};
                std::memcpy(placed.name, src.name, 8);
                placed.name[8] = '\0';
                dStage_objectNameInf* info = dStage_searchName(placed.name);
                placed.procname = info != nullptr ? info->procname : static_cast<s16>(-1);
                placed.argument = info != nullptr ? info->argument : static_cast<s8>(0);
                placed.scale.set(1.0f, 1.0f, 1.0f);
                fill_common(&placed, tagStr, layer, roomNo, src.base, bySetId, all);
                out->push_back(placed);
            }
        }
    }
}

void* try_resolve_buffer(int roomNo, char (&keyOut)[48]) {
    char* arcName = dComIfG_getRoomArcName(roomNo);
    if (arcName != nullptr && arcName[0] != '\0') {
        void* dzr = dComIfG_getStageRes(arcName, "room.dzr");
        if (dzr != nullptr) {
            SAFE_SPRINTF(keyOut, "%s/room.dzr", arcName);
            return dzr;
        }
    }

    char dzsName[16]{};
    SAFE_SPRINTF(dzsName, "room%d.dzs", roomNo);
    void* dzs = dComIfG_getStageRes(dzsName);
    if (dzs != nullptr) {
        SAFE_STRCPY(keyOut, dzsName);
        return dzs;
    }

    keyOut[0] = '\0';
    return nullptr;
}

}  // namespace

EnumerateResult enumerate_room_actors(int i_roomNo, bool i_systemUse) {
    EnumerateResult result{};

#if !TARGET_PC
    return result;
#else
    if (!g_levelEditorSession && !i_systemUse) {
        return result;
    }

    int roomNo = i_roomNo;
    if (roomNo < 0) {
        roomNo = dComIfGp_roomControl_getStayNo();
    }

    LiveBySetId bySetId;
    std::vector<LiveActorRef> all;
    LiveBuildCtx mapCtx{&bySetId, &all, 0, roomNo};
    fopAcIt_Executor(build_live_cb, &mapCtx);
    result.liveActorsInRoom = mapCtx.liveInStayRoom;

    char key[48]{};
    void* buffer = try_resolve_buffer(roomNo, key);
    if (buffer != nullptr) {
        result.source = EnumSource::Buffer;
        SAFE_STRCPY(result.resourceKey, key);
        append_from_buffer(buffer, roomNo, bySetId, all, &result.actors);
        return result;
    }

    result.source = EnumSource::LiveFallback;
    SAFE_STRCPY(result.resourceKey, "(live fallback)");
    LiveCollectCtx ctx{roomNo, &result.actors};
    fopAcIt_Executor(collect_live_cb, &ctx);
    return result;
#endif
}

void set_selected_index(int index) {
    sSelectedIndex = index;
}

int get_selected_index() {
    return sSelectedIndex;
}

void set_selection_snapshot(const PlacedActor& actor, bool valid) {
    sSelection = actor;
    sSelectionValid = valid;
    if (!valid) {
        sSelectedIndex = -1;
        sSelectionProcId = fpcM_ERROR_PROCESS_ID_e;
        return;
    }
    if (sSelection.live != nullptr) {
        sSelectionProcId = fopAcM_GetID(sSelection.live);
        sSelection.pos = sSelection.live->current.pos;
        sSelection.angle = sSelection.live->shape_angle;
        sSelection.unspawned = false;
    } else {
        sSelectionProcId = fpcM_ERROR_PROCESS_ID_e;
    }
    if (sSelectionDetailHandler) {
        sSelectionDetailHandler(sSelection);
    }
}

void set_selection_detail_handler(SelectionDetailHandler handler) {
    sSelectionDetailHandler = std::move(handler);
}

std::string format_placed_actor_detail_rml(const PlacedActor& sel) {
    return fmt::format(
        "tag <b>{}</b><br/>"
        "proc <b>{}</b> arg <b>{}</b><br/>"
        "setID <b>{}</b> layer <b>{}</b> room <b>{}</b><br/>"
        "params <b>0x{:08X}</b><br/>"
        "pos <b>{:.1f}, {:.1f}, {:.1f}</b><br/>"
        "angle <b>{}, {}, {}</b><br/>"
        "scale <b>{:.2f}, {:.2f}, {:.2f}</b><br/>"
        "{}",
        sel.chunkTag, sel.procname, sel.argument, sel.setID, sel.layer, sel.roomNo, sel.params,
        sel.pos.x, sel.pos.y, sel.pos.z, sel.angle.x, sel.angle.y, sel.angle.z, sel.scale.x,
        sel.scale.y, sel.scale.z,
        sel.isSpawnPoint ? "<b>spawn point</b> (PLYR — not uniquely joinable)"
        : sel.unspawned ? "<b>unspawned</b> (no live match — dead/despawned/other layer)"
                        : "<b>live</b> (joined)");
}

void clear_selection() {
    sSelectionValid = false;
    sSelectedIndex = -1;
    sSelectionProcId = fpcM_ERROR_PROCESS_ID_e;
    sSelection = {};
}

void detach_selection_live() {
    // Keep sSelectionProcId — draw rebinds via fpcM_SearchByID so highlight follows
    // the picked actor after Stage/Editor teardown (Gate 8d).
    sSelection.live = nullptr;
}

bool has_selection() {
    return g_levelEditorSession && sSelectionValid;
}

const PlacedActor* get_selection() {
    return has_selection() ? &sSelection : nullptr;
}

void draw_selection_highlight() {
#if TARGET_PC
    // Must be called from Draw (after BeforeOfDraw). Execute-time queues are
    // deleted by dDbVw_deleteDrawPacketList before paint — never visible.
    // Selection persists after Editor/Stage UI closes until clear_selection().
    if (!g_levelEditorSession) {
        return;
    }

    if (session_select_mode_enabled()) {
        dDbVw_Report(20, 180, "SELECT MODE - V to exit");
    } else if (editor_pc_hotkeys_active()) {
        dDbVw_Report(20, 180, "PC hotkeys on - V = select mode");
    }

    if (!sSelectionValid) {
        return;
    }

    // Gate 8d: always rebind by process ID (Stage teardown nulls live every hide).
    if (!sSelection.isSpawnPoint) {
        refresh_selection_live();
    }

    cXyz pos = sSelection.pos;
    if (sSelection.live != nullptr && !sSelection.unspawned && !sSelection.isSpawnPoint) {
        pos = sSelection.live->current.pos;
        sSelection.pos = pos;
    } else if (sSelection.isSpawnPoint) {
        pos = sSelection.pos;
    }

    // Gate 11b (Path A): model-tight highlight — one translucent box per model
    // joint (hugs the actor) instead of the oversized cull box. Cyan = selected.
    // Marker/arrow keep it legible for actors that fall back to the cull box.
    constexpr GXColor kSelectMarker = {0xFF, 0xE0, 0x40, 0xFF};
    if (sSelection.live != nullptr && !sSelection.unspawned && !sSelection.isSpawnPoint) {
        draw_actor_volume_highlight(sSelection.live, 0x40, 0xC0, 0xFF, 0x60);
    }

    cXyz markerPos = pos;
    markerPos.y += 40.0f;
    dDbVw_drawSphereXlu(markerPos, 35.0f, kSelectMarker, /*clipZ=*/0);
    cXyz up = pos;
    up.y += 180.0f;
    dDbVw_drawArrowXlu(pos, up, kSelectMarker, /*clipZ=*/0, /*width=*/10);

    dDbVw_Report(20, 200, "SEL %s #%d %s", sSelection.name, sSelection.setID,
                 sSelection.isSpawnPoint ? "spawn"
                 : sSelection.unspawned  ? "unspawned"
                                         : "live");
    if (const char* idLine = get_pick_identify_hud_line()) {
        dDbVw_Report(20, 216, "%s", idLine);
    }
#endif
}

void enable_session_fly_cam(bool enable) {
    sSessionFlyCam = enable;
    if (!enable) {
        enable_session_select_mode(false);
    }
}

bool session_fly_cam_enabled() {
    return g_levelEditorSession && sSessionFlyCam;
}

bool editor_fly_cam_active() {
    // ============================================================
    // Fly cam has TWO independent enable sources:
    //   1. the standalone debug fly-cam ConfigVar (works ANYWHERE),
    //   2. the level-editor session latch (editor mode only).
    // Regression fix: the old early-out `if (!g_levelEditorSession)
    // return false;` gated the ConfigVar behind an active editor
    // session, so the general debug fly cam was dead outside the
    // editor. session_fly_cam_enabled() already carries the
    // g_levelEditorSession guard, so editor-only behaviour is intact.
    // ============================================================
    return session_fly_cam_enabled()
        || dusk::getSettings().game.debugFlyCam.getValue();
}

void enable_session_select_mode(bool enable) {
    if (!g_levelEditorSession) {
        sSessionSelectMode = false;
        return;
    }
    if (sSessionSelectMode == enable) {
        return;
    }
    sSessionSelectMode = enable;
    dCamera_c::resetEditorFlyCamMouseLook();
}

bool session_select_mode_enabled() {
    return g_levelEditorSession && sSessionSelectMode && editor_fly_cam_active();
}

void enable_session_pc_hotkeys(bool enable) {
    sSessionPcHotkeys = enable;
}

bool session_pc_hotkeys_enabled() {
    return g_levelEditorSession && sSessionPcHotkeys;
}

bool editor_pc_hotkeys_active() {
#if TARGET_PC
    if (!session_pc_hotkeys_enabled() || rml_text_input_focused()) {
        return false;
    }
    return true;
#else
    return false;
#endif
}

void tick_editor_session_input() {
#if TARGET_PC
    if (!g_levelEditorSession || !editor_fly_cam_active() || !editor_pc_hotkeys_active()) {
        return;
    }
    if (scancode_pressed_edge(SDL_SCANCODE_V)) {
        enable_session_select_mode(!sSessionSelectMode);
    }
#endif
}

}  // namespace dusk::leveledit
