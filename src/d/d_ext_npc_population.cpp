/**
 * d_ext_npc_population.cpp — Phase O2 folder-driven BG population.
 *
 * Reads modFolder/population/<csv> + actor_map.ini. Maps WW placement names to
 * ExtNpc procs already provided by the mod. Zero character knowledge in code.
 *
 * №27: N1 layer bleed control, N2 head_from_params attach from CSV params_hex.
 */
#include "d/d_ext_npc_population.h"

#if TARGET_PC

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "d/d_com_inf_game.h"
#include "d/d_ext_mod_flags.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_item_data.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_layer.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "f_pc/f_pc_node.h"

namespace fs = std::filesystem;

namespace {

std::unordered_set<std::string> s_spawnedBgProcs;
// №94: generation when each proc was marked spawned (mismatch ⇒ allow re-run).
std::unordered_map<std::string, u32> s_spawnedBgProcGen;

void markSpawned(const char* proc) {
    if (proc == NULL || proc[0] == '\0') {
        return;
    }
    s_spawnedBgProcs.insert(proc);
    s_spawnedBgProcGen[proc] = dExtNpcWorld_generation();
}

struct ActorMapEntry {
    char proc[32];
    int arg;
    char spawnIf[64];
    char spawnUnless[64];
    char headFromParams[32];  // N2: prefix → <prefix>0N.bdl from params low byte
    char headModel[64];       // N2: fixed attach (e.g. oba_head.bdl)
    char headJoint[32];
    bool unique;              // N1: first placement wins (folk); false for herds
    // №37: WW authored positions to skip (event-gated ACT0 alternates).
    cXyz disablePos[4];
    int disablePosCount;
};

// N1: chunk → required mod flag (empty = always allow when listed).
std::unordered_map<std::string, std::string> s_layerFlags;
bool s_layersLoaded = false;

void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' ||
                          s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
        ++i;
    }
    if (i > 0) {
        s.erase(0, i);
    }
}

void lowerInPlace(std::string& s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
}

bool looksLikeHeadOnlyName(const std::string& name) {
    // Never mount a standalone head/hair mesh as a body (yellow unlit "Arylls").
    std::string low = name;
    lowerInPlace(low);
    if (low.find("head") != std::string::npos) {
        return true;
    }
    if (low.rfind("hair", 0) == 0) {
        return true;
    }
    return false;
}

bool loadActorMap(const fs::path& path, std::unordered_map<std::string, ActorMapEntry>* out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    s_layerFlags.clear();
    s_layersLoaded = true;

    std::string line;
    std::string current;
    ActorMapEntry* cur = NULL;
    bool inLayers = false;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            const size_t end = line.find(']');
            if (end == std::string::npos) {
                continue;
            }
            current = line.substr(1, end - 1);
            trim(current);
            if (current == "layers") {
                inLayers = true;
                cur = NULL;
                continue;
            }
            inLayers = false;
            ActorMapEntry& e = (*out)[current];
            std::memset(&e, 0, sizeof(e));
            e.arg = -1;
            e.unique = true;  // folk default — herds set unique=0
            std::snprintf(e.headJoint, sizeof(e.headJoint), "%s", "head");
            cur = &e;
            continue;
        }
        if (inLayers) {
            const size_t eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            trim(key);
            trim(val);
            // Allow ACT0= (empty) meaning always; ACT2=q5.rescue meaning gated.
            s_layerFlags[key] = val;
            continue;
        }
        if (cur == NULL) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key);
        trim(val);
        lowerInPlace(key);
        if (key == "proc") {
            std::snprintf(cur->proc, sizeof(cur->proc), "%s", val.c_str());
        } else if (key == "arg") {
            cur->arg = std::atoi(val.c_str());
        } else if (key == "spawn_if_flag") {
            std::snprintf(cur->spawnIf, sizeof(cur->spawnIf), "%s", val.c_str());
        } else if (key == "spawn_unless_flag") {
            std::snprintf(cur->spawnUnless, sizeof(cur->spawnUnless), "%s", val.c_str());
        } else if (key == "head_from_params") {
            std::snprintf(cur->headFromParams, sizeof(cur->headFromParams), "%s", val.c_str());
        } else if (key == "head_model") {
            std::snprintf(cur->headModel, sizeof(cur->headModel), "%s", val.c_str());
        } else if (key == "head_joint") {
            std::snprintf(cur->headJoint, sizeof(cur->headJoint), "%s", val.c_str());
        } else if (key == "unique") {
            cur->unique = !(val == "0" || val == "false" || val == "no");
        } else if (key == "disable_pos" || key == "disabled_pos") {
            // №37: one or more WW world positions to refuse (event alternates).
            if (cur->disablePosCount < 4) {
                cXyz& p = cur->disablePos[cur->disablePosCount];
                if (std::sscanf(val.c_str(), "%f,%f,%f", &p.x, &p.y, &p.z) == 3) {
                    ++cur->disablePosCount;
                }
            }
        }
    }
    // Default day-1 layer table if author omitted [layers].
    if (s_layerFlags.empty()) {
        s_layerFlags["ACT0"] = "";
    }
    return !out->empty();
}

bool chunkAllowed(const std::string& chunk, const char* modFolder) {
    if (chunk == "ACTR" || chunk == "SCOB" || chunk == "SCO0") {
        return true;
    }
    auto it = s_layerFlags.find(chunk);
    if (it == s_layerFlags.end()) {
        // Unknown story layer — refuse (kills ACT2–ACTb bleed).
        return false;
    }
    if (it->second.empty()) {
        return true;
    }
    return modFolder != NULL && dExtModFlags_get(modFolder, it->second.c_str());
}

// N2: build head bdl from prefix + WW params low byte.
// Byte 0 → 01 (default); byte N → 0N. Returns false if no model string written.
bool buildHeadModelFromParams(const char* prefix, u32 wwParams, char* out, u32 outBytes) {
    if (prefix == NULL || prefix[0] == '\0' || out == NULL || outBytes < 8) {
        return false;
    }
    int idx = static_cast<int>(wwParams & 0xFF);
    if (idx <= 0 || idx > 99) {
        idx = 1;
    }
    // WW often uses 0xFF as "default" sentinel in the low byte.
    if ((wwParams & 0xFF) == 0xFF) {
        idx = 1;
    }
    std::snprintf(out, outBytes, "%s%02d.bdl", prefix, idx);
    return true;
}

// bridge_meta.ini schema_version — missing = v0 legacy (warn once), mismatch = refuse.
std::unordered_set<std::string> s_schemaLegacyWarned;

enum class SchemaCheck { Ok, LegacyV0, Mismatch };

SchemaCheck checkPopulationSchema(const fs::path& modRoot, int* outFound) {
    if (outFound != NULL) {
        *outFound = -1;
    }
    const fs::path metaPath = modRoot / "population" / "bridge_meta.ini";
    std::error_code ec;
    const bool exists = fs::exists(metaPath, ec) && !ec;
    if (!exists) {
        return SchemaCheck::LegacyV0;  // truly absent
    }
    std::ifstream in(metaPath);
    if (!in) {
        // Present but unreadable (perms/corrupt) — fail closed, not legacy open.
        return SchemaCheck::Mismatch;
    }
    std::string line;
    int found = -1;
    bool keySeen = false;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trim(key);
        trim(val);
        lowerInPlace(key);
        if (key == "schema_version") {
            keySeen = true;
            found = std::atoi(val.c_str());
            break;
        }
    }
    if (outFound != NULL) {
        *outFound = found;
    }
    if (!keySeen) {
        return SchemaCheck::LegacyV0;  // file present, key absent
    }
    // Key present: negative / non-matching ⇒ mismatch (not legacy fail-open).
    if (found < 0 || found != kExtPopSchemaVersion) {
        return SchemaCheck::Mismatch;
    }
    return SchemaCheck::Ok;
}

}  // namespace

void dExtNpcPopulation_clearForBg(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        return;
    }
    s_spawnedBgProcs.erase(procName);
    s_spawnedBgProcGen.erase(procName);
}

void dExtNpcPopulation_clearAll() {
    s_spawnedBgProcs.clear();
    s_spawnedBgProcGen.clear();
}

bool dExtNpcPopulation_isLatched(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        return false;
    }
    auto git = s_spawnedBgProcGen.find(procName);
    return s_spawnedBgProcs.count(procName) != 0 && git != s_spawnedBgProcGen.end() &&
           git->second == dExtNpcWorld_generation();
}

int dExtNpcPopulation_countLiveCensus() {
    struct Ctx {
        int n;
    } ctx{0};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            Ctx* c = (Ctx*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg) {
                return 0;
            }
            if (std::strncmp(m->mSpawnSrc, "census:", 7) == 0) {
                ++c->n;
            }
            return 0;
        },
        &ctx);
    return ctx.n;
}

void dExtNpcPopulation_spawnForBg(const dExtNpcManifest& bg) {
    if (!bg.isBg || bg.populationCsv[0] == '\0' || bg.modFolder[0] == '\0') {
        return;
    }
    const u32 gen = dExtNpcWorld_generation();
    auto git = s_spawnedBgProcGen.find(bg.proc);
    if (s_spawnedBgProcs.count(bg.proc) != 0 && git != s_spawnedBgProcGen.end() &&
        git->second == gen) {
        return;
    }
    // Stale latch from a prior world gen — allow re-run.
    s_spawnedBgProcs.erase(bg.proc);
    s_spawnedBgProcGen.erase(bg.proc);
    // Perf A/B: set DUSK_EXTNPC_SKIP_POP=1 to load island BG only (no census mounts).
    // Isolates Outset mesh cost from ~70+ day-1 actor execute/draw.
    {
        const char* skip = std::getenv("DUSK_EXTNPC_SKIP_POP");
        if (skip != NULL && skip[0] != '\0' && skip[0] != '0') {
            DuskLog.info("[ExtNpcPop] SKIP_POP — not spawning for '{}' (env DUSK_EXTNPC_SKIP_POP)",
                         bg.proc);
            markSpawned(bg.proc);
            return;
        }
    }

    const fs::path modRoot = dusk::ConfigPath / "model_replacements" / bg.modFolder;
    const fs::path csvPath = modRoot / "population" / bg.populationCsv;
    const fs::path mapPath = modRoot / "population" / "actor_map.ini";

    int foundSchema = -1;
    const SchemaCheck schema = checkPopulationSchema(modRoot, &foundSchema);
    if (schema == SchemaCheck::Mismatch) {
        DuskLog.warn(
            "[ExtNpcPop] schema_version mismatch for '{}': file={} engine={} — "
            "refusing population CSV '{}' (empty island prevented)",
            bg.modFolder, foundSchema, kExtPopSchemaVersion, bg.populationCsv);
        dusk::custom_assets::set_mod_status_note(
            bg.modFolder, "Population refused: schema_version mismatch (regenerate census)");
        markSpawned(bg.proc);
        return;
    }
    if (schema == SchemaCheck::LegacyV0) {
        if (s_schemaLegacyWarned.insert(bg.modFolder).second) {
            DuskLog.warn(
                "[ExtNpcPop] no bridge_meta.ini schema_version under '{}' — "
                "treating as legacy v0 (one-time warn)",
                bg.modFolder);
        }
        // Clear a prior mismatch badge if the player removed a bad meta file.
        dusk::custom_assets::set_mod_status_note(bg.modFolder, "");
    } else {
        dusk::custom_assets::set_mod_status_note(bg.modFolder, "");
    }

    std::unordered_map<std::string, ActorMapEntry> map;
    if (!loadActorMap(mapPath, &map)) {
        DuskLog.warn("[ExtNpcPop] no actor_map.ini under {}", bg.modFolder);
        markSpawned(bg.proc);
        return;
    }

    // N4: overlay locked identities (user-named) onto display names at spawn time is
    // handled by mount rescan; population only needs census→proc.

    std::ifstream in(csvPath);
    if (!in) {
        DuskLog.warn("[ExtNpcPop] population csv missing: {}", csvPath.string());
        markSpawned(bg.proc);
        return;
    }

    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;  // retry next poll
    }
    // №64: room-lane interiors spawn into the claimed host room (not Link's old stay).
    int roomNo = fopAcM_GetRoomNo(player);
    if (dExtNpcMount_isRoomLaneProc(bg.proc)) {
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(bg.proc);
        if (hostRoom >= 0) {
            roomNo = hostRoom;
        }
    }

    std::string line;
    std::getline(in, line);  // header
    trim(line);
    // №32 B3: interior CSV = stage,chunk,layer,name,... ; exterior = chunk,layer,name,...
    bool hasStageCol = false;
    {
        std::string h = line;
        for (char& c : h) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        hasStageCol = h.rfind("stage,", 0) == 0 || h.find("stage,chunk") != std::string::npos;
    }
    const char* stageFilter =
        bg.populationStage[0] ? bg.populationStage : (hasStageCol ? bg.arc : "");

    int spawned = 0;
    int skipped = 0;
    std::unordered_set<std::string> uniqueNames;
    // №131: clear entries owned by actors that no longer exist before spawning.
    // Vanilla drives story variation by re-running this pass per layer
    // (dComIfG_play_c::getLayerNo), so ids recycle constantly and a stale entry
    // would eventually be inherited by an unrelated actor.
    dExtNpcMount_sweepPendingById();
    dExtNpcMount_markPendingPass();
    layer_class* savedLayer = fpcLy_CurrentLayer();
    base_process_class* playScene = fpcM_SearchByName(fpcNm_PLAY_SCENE_e);
    if (playScene != NULL) {
        fpcLy_SetCurrentLayer(&((process_node_class*)playScene)->layer);
    }

    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::vector<std::string> cols;
        {
            std::string cur;
            for (char c : line) {
                if (c == ',') {
                    cols.push_back(cur);
                    cur.clear();
                } else {
                    cur.push_back(c);
                }
            }
            cols.push_back(cur);
        }
        const int base = hasStageCol ? 1 : 0;
        if (static_cast<int>(cols.size()) < base + 10) {
            continue;
        }
        if (hasStageCol) {
            if (stageFilter[0] == '\0' || cols[0] != stageFilter) {
                continue;
            }
        }
        const std::string& chunk = cols[base + 0];
        const std::string& name = cols[base + 2];
        if (!chunkAllowed(chunk, bg.modFolder)) {
            continue;
        }
        if (looksLikeHeadOnlyName(name)) {
            ++skipped;
            continue;
        }
        auto it = map.find(name);
        if (it == map.end() || it->second.proc[0] == '\0') {
            continue;
        }
        const ActorMapEntry& me = it->second;
        if (me.unique && uniqueNames.count(name) != 0) {
            ++skipped;
            continue;
        }
        if (me.spawnIf[0] && !dExtModFlags_get(bg.modFolder, me.spawnIf)) {
            ++skipped;
            continue;
        }
        if (me.spawnUnless[0] && dExtModFlags_get(bg.modFolder, me.spawnUnless)) {
            ++skipped;
            continue;
        }
        const u32 wwParams =
            static_cast<u32>(std::strtoul(cols[base + 3].c_str(), NULL, 16));
        const f32 wx = std::strtof(cols[base + 4].c_str(), NULL);
        const f32 wy = std::strtof(cols[base + 5].c_str(), NULL);
        const f32 wz = std::strtof(cols[base + 6].c_str(), NULL);
        const int ry = std::atoi(cols[base + 8].c_str());

        // №24 D3: CSV coords are WW authored-world; place at host + (ww − anchor).
        cXyz pos;
        pos.x = bg.hostPos.x + (wx - bg.anchor.x);
        pos.y = bg.hostPos.y + (wy - bg.anchor.y);
        pos.z = bg.hostPos.z + (wz - bg.anchor.z);

        // №25 F2: skip salvage/seafloor ACTRs (e.g. Zl1 at ww_y=-5000).
        if (pos.y < bg.hostPos.y - 2000.0f) {
            ++skipped;
            continue;
        }
        // №42: Nintendo sky-parks event actors high above the room (e.g. A_mori P1a y=34707).
        if (pos.y > bg.hostPos.y + 20000.0f) {
            DuskLog.info("[ExtNpcPop] skip sky-parked name={} ww=({:.0f},{:.0f},{:.0f}) hostY={:.0f}",
                         name, wx, wy, wz, bg.hostPos.y);
            ++skipped;
            continue;
        }

        csXyz angle;
        angle.set(0, static_cast<s16>(ry), 0);
        cXyz scale(1.0f, 1.0f, 1.0f);

        // №37: skip Nintendo's event-gated ACT0 alternate positions (same census name).
        if (me.disablePosCount > 0) {
            bool disabled = false;
            for (int di = 0; di < me.disablePosCount; ++di) {
                const f32 ddx = wx - me.disablePos[di].x;
                const f32 ddy = wy - me.disablePos[di].y;
                const f32 ddz = wz - me.disablePos[di].z;
                if (ddx * ddx + ddz * ddz < 80.0f * 80.0f && ddy > -120.0f && ddy < 120.0f) {
                    disabled = true;
                    break;
                }
            }
            if (disabled) {
                ++skipped;
                continue;
            }
        }

        // №36 C / №31 / №55: pickup-grant rows key on the MANIFEST FLAG (pickup_rupee=1),
        // never on a payload name — the engine stays name-blind; actor_map supplies the
        // proc, the manifest supplies the behavior.
        dExtNpcManifest pickupMan{};
        const bool isPickup = dExtNpcMount_hasPayload(me.proc) &&
                              dExtNpcMount_lookup(me.proc, &pickupMan) &&
                              pickupMan.pickupRupee;
        if (isPickup) {
            const int itemNo = static_cast<int>(wwParams & 0xFF);
            const bool okTier =
                (itemNo >= dItemNo_GREEN_RUPEE_e && itemNo <= dItemNo_ORANGE_RUPEE_e) ||
                itemNo == 0x0F;
            if (!okTier) {
                ++skipped;
                continue;
            }
            const s16 actorId = dExtNpcMount_socketActorId(pickupMan.socket[0] ? pickupMan.socket
                                                                               : pickupMan.proc);
            if (actorId < 0) {
                ++skipped;
                continue;
            }
            const u32 params = static_cast<u32>(itemNo) & 0xFF;
            char src[96];
            std::snprintf(src, sizeof(src), "census:item@(%.0f,%.0f,%.0f)", wx, wy, wz);
            // №45: push BEFORE create — Create may run before an id bind lands.
            dExtNpcMount_pushPendingSpawn(me.proc, src, NULL, NULL);
            const fpc_ProcID id =
                fopAcM_create(actorId, params, &pos, roomNo, &angle, &scale, -1);
            if (id != fpcM_ERROR_PROCESS_ID_e) {
                ++spawned;
                if (spawned <= 12) {
                    DuskLog.info(
                        "[ExtNpcPop] spawn #{} name=item proc={} itemNo={} "
                        "world=({:.1f},{:.1f},{:.1f})",
                        spawned, me.proc, itemNo, pos.x, pos.y, pos.z);
                }
            } else {
                char discard[8];
                dExtNpcMount_takePendingSpawn(fpcM_ERROR_PROCESS_ID_e, discard, sizeof(discard),
                                              NULL, 0, NULL, 0, NULL, 0);
                ++skipped;
            }
            continue;
        }

        if (!dExtNpcMount_hasPayload(me.proc)) {
            ++skipped;
            continue;
        }
        dExtNpcManifest man{};
        if (!dExtNpcMount_lookup(me.proc, &man)) {
            ++skipped;
            continue;
        }
        if (man.spawnIfFlag[0] && !dExtModFlags_get(man.modFolder, man.spawnIfFlag)) {
            ++skipped;
            continue;
        }
        if (man.spawnUnlessFlag[0] && dExtModFlags_get(man.modFolder, man.spawnUnlessFlag)) {
            ++skipped;
            continue;
        }

        const s16 actorId = dExtNpcMount_socketActorId(man.socket[0] ? man.socket : man.proc);
        if (actorId < 0) {
            ++skipped;
            continue;
        }
        u32 params = 0;
        if (me.arg >= 0) {
            params = static_cast<u32>(me.arg) & 0xFF;
        } else if (man.socketArg >= 0) {
            params = static_cast<u32>(man.socketArg) & 0xFF;
        }

        // N2 / №35 H1: pin head attach from WW params (or fixed head_model).
        char headModel[64] = {};
        if (me.headModel[0]) {
            std::snprintf(headModel, sizeof(headModel), "%s", me.headModel);
        } else if (me.headFromParams[0]) {
            buildHeadModelFromParams(me.headFromParams, wwParams, headModel, sizeof(headModel));
        }
        if (!headModel[0] && !man.isStatic && !man.isBg) {
            // №38 E3: folk body without a head pin is a miss (ledger will say MISS).
            DuskLog.warn("[ExtNpcPop] {} proc={} — no head_model/head_from_params", name,
                         me.proc);
        }

        char src[96];
        std::snprintf(src, sizeof(src), "census:%s@(%.0f,%.0f,%.0f)", name.c_str(), wx, wy, wz);
        // №45: FIFO push before create (id-bind after was too late when Create ran sync).
        // Head pin still passed for P1a/P1b same-arg collisions; registry pull covers the rest.
        const u32 pendingSeq =
            dExtNpcMount_pushPendingSpawn(me.proc, src, headModel[0] ? headModel : NULL,
                                          me.headJoint[0] ? me.headJoint : "head");
        const fpc_ProcID id =
            fopAcM_create(actorId, params, &pos, roomNo, &angle, &scale, -1);
        // №130: reclaim our own entry if the actor did not consume it. This makes
        // the whole leak class impossible instead of relying on every actor
        // author remembering — the failure mode (a stale entry served to the NEXT
        // actor) is silent, and its blast radius is the entire remaining cast.
        dExtNpcMount_reapPendingSpawn(pendingSeq, id);
        if (id != fpcM_ERROR_PROCESS_ID_e) {
            ++spawned;
            if (me.unique) {
                uniqueNames.insert(name);
            }
            if (spawned <= 12) {
                DuskLog.info(
                    "[ExtNpcPop] spawn #{} name={} proc={} chunk={} ww_params={:08x} head={} "
                    "world=({:.1f},{:.1f},{:.1f})",
                    spawned, name, me.proc, chunk, wwParams, headModel[0] ? headModel : "(none)",
                    pos.x, pos.y, pos.z);
            }
        } else {
            // №130: the reap above already dropped it — and by TOKEN, not by
            // position. The old front-pop could discard a DIFFERENT actor's
            // entry when anything else was queued.
            ++skipped;
        }
    }

    fpcLy_SetCurrentLayer(savedLayer);
    markSpawned(bg.proc);
    DuskLog.info(
        "[ExtNpcPop] BG '{}' spawned {} actors ({} skipped/unmapped) host=({:.0f},{:.0f},{:.0f}) "
        "anchor=({:.0f},{:.0f},{:.0f}) unique_folk={} gen={}",
        bg.proc, spawned, skipped, bg.hostPos.x, bg.hostPos.y, bg.hostPos.z, bg.anchor.x,
        bg.anchor.y, bg.anchor.z, (int)uniqueNames.size(), dExtNpcWorld_generation());
}

#endif  // TARGET_PC
