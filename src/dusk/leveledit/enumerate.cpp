// ============================================================================
// Level Editor — placed-actor enumeration + highlight (Phase 1a).
// ============================================================================

#include "d/dolzel.h"  // IWYU pragma: keep

#include "dusk/leveledit/enumerate.hpp"

#include "d/d_com_inf_game.h"
#include "d/d_debug_viewer.h"
#include "d/d_stage.h"
#include "dusk/main.h"
#include "dusk/settings.h"
#include "dusk/string.hpp"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>

namespace dusk::leveledit {
namespace {

STATIC_ASSERT(sizeof(fopAcM_prmBase_class) == 0x18);
STATIC_ASSERT(sizeof(stage_actor_data_class) == 0x20);
STATIC_ASSERT(sizeof(stage_tgsc_data_class) == 0x24);

constexpr u16 kNonUniqueSetID = 0xFFFF;
constexpr f32 kJoinPosEpsilon = 50.0f;  // cm — spawn vs live can drift slightly

int sSelectedIndex = -1;
bool sSelectionValid = false;
PlacedActor sSelection{};
bool sSessionFlyCam = false;
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

// Re-bind selection.live from the live actor list (safe after UI detach / despawn).
void refresh_selection_live() {
    if (!sSelectionValid || sSelection.isSpawnPoint) {
        return;
    }

    LiveBySetId bySetId;
    std::vector<LiveActorRef> all;
    const int roomNo = sSelection.roomNo;
    LiveBuildCtx mapCtx{&bySetId, &all, 0, roomNo};
    fopAcIt_Executor(build_live_cb, &mapCtx);

    sSelection.live =
        join_live(roomNo, sSelection.setID, sSelection.name, sSelection.spawnPos, bySetId, all);
    sSelection.unspawned = sSelection.live == nullptr;
    if (sSelection.live != nullptr) {
        sSelection.pos = sSelection.live->current.pos;
        sSelection.angle = sSelection.live->shape_angle;
    } else {
        sSelection.pos = sSelection.spawnPos;
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

EnumerateResult enumerate_room_actors(int i_roomNo) {
    EnumerateResult result{};

#if !TARGET_PC
    return result;
#else
    if (!g_levelEditorSession) {
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
        return;
    }
    if (sSelectionDetailHandler) {
        sSelectionDetailHandler(actor);
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
    sSelection = {};
}

void detach_selection_live() {
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
    if (!g_levelEditorSession || !sSelectionValid) {
        return;
    }

    // Gate 6: after UI detach (live==null), re-bind once; then follow current.pos
    // every frame like native collision debug boxes. spawnPos keeps join stable.
    if (!sSelection.isSpawnPoint && sSelection.live == nullptr) {
        refresh_selection_live();
    }

    cXyz pos = sSelection.spawnPos;
    if (sSelection.live != nullptr && !sSelection.unspawned && !sSelection.isSpawnPoint) {
        pos = sSelection.live->current.pos;
        sSelection.pos = pos;
    } else if (!sSelection.isSpawnPoint) {
        pos = sSelection.spawnPos;
        sSelection.pos = pos;
    } else {
        pos = sSelection.pos;
    }

    // Z-test OFF (clipZ=0) so fly-cam can see the marker through geometry.
    // Cube always Z-tests — keep it as a local solid cue; sphere/arrow pierce.
    // Future: size highlight to actor collision bounds.
    cXyz size(80.0f, 80.0f, 80.0f);
    csXyz angle(0, 0, 0);
    const GXColor boxColor = {0x40, 0xC0, 0xFF, 0xA0};
    dDbVw_drawCubeXlu(pos, size, angle, boxColor);

    const GXColor sphereColor = {0x40, 0xC0, 0xFF, 0xC0};
    dDbVw_drawSphereXlu(pos, 100.0f, sphereColor, /*clipZ=*/0);

    cXyz up = pos;
    up.y += 200.0f;
    const GXColor arrowColor = {0xFF, 0xE0, 0x40, 0xFF};
    dDbVw_drawArrowXlu(pos, up, arrowColor, /*clipZ=*/0, /*width=*/12);

    // On-screen confirm so playtest can tell the path is live (incl. fly cam).
    dDbVw_Report(20, 200, "SEL %s #%d %s", sSelection.name, sSelection.setID,
                 sSelection.isSpawnPoint ? "spawn"
                 : sSelection.unspawned  ? "unspawned"
                                         : "live");
#endif
}

void enable_session_fly_cam(bool enable) {
    sSessionFlyCam = enable;
}

bool session_fly_cam_enabled() {
    return g_levelEditorSession && sSessionFlyCam;
}

bool editor_fly_cam_active() {
    if (!g_levelEditorSession) {
        return false;
    }
    return sSessionFlyCam || dusk::getSettings().game.debugFlyCam.getValue();
}

}  // namespace dusk::leveledit
