// ============================================
// NEW CODE — ALBW Port
// Postman Cuckoo's Return service (dungeon entrance warp).
// ============================================
#include "d/d_albw_oocoo.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_item_data.h"
#include "d/d_com_inf_game.h"
#include "d/d_save.h"
#include "dusk/map_loader_definitions.h"
#include "dusk/truetest.hpp"
#include <cstring>

namespace {

// Priced for the post-death-economy era (rupee halving on death, ALBW
// inventory strip): 150 predated those systems and priced the service
// out of exactly the moments it exists for.
constexpr int kPrice = 15;

char sDeathDungeonStage[8] = {};
bool sDiedInDungeon        = false;
bool sChoseOrdonAfterDeath = false;
bool sUsedOocooThisDeath   = false;
bool sPendingEntranceWarp  = false;

bool hasMetOocooSrOnce() {
    // Sr. in inventory, Oocoo's note (TKS letter), Jr. after first warp, or dungeon warp flag.
    return dComIfGp_checkItemGet(dItemNo_DUNGEON_EXIT_e, 1)
        || dComIfGp_checkItemGet(dItemNo_DUNGEON_EXIT_2_e, 1)
        || dComIfGp_checkItemGet(dItemNo_LV7_DUNGEON_EXIT_e, 1)
        || dComIfGp_checkItemGet(dItemNo_DUNGEON_BACK_e, 1)
        || dComIfGp_checkItemGet(dItemNo_TKS_LETTER_e, 1)
        || dComIfGs_isDungeonItemWarp();
}

bool postmanRentalActive() {
    return dusk::truetest::isAlbwPostmanUnlocked();
}

// ============================================
// Entrance resolution (alpha cleanup — crash fix). The old warp hardcoded
// room 0 / point 0, but not every dungeon HAS a room 0 (Goron Mines D_MN04
// and Hyrule Castle D_MN09 start at room 1), and setNextStage does no
// validation — the load of the nonexistent room crashed the game. Resolve
// the entrance from the level-editor warp table instead: its first room
// entry + first point is the same baseline the warp menu lands on when a
// dungeon is selected. Unknown stage -> caller must not warp.
// ============================================
bool resolveDungeonEntrance(const char* i_stage, s8* o_roomNo, s16* o_point) {
    if (i_stage == NULL || i_stage[0] == '\0') {
        return false;
    }

    for (const auto& region : gameRegions) {
        for (const auto& map : region.maps) {
            if (map.mapFile == NULL || std::strcmp(map.mapFile, i_stage) != 0) {
                continue;
            }
            if (map.mapRooms.empty() || map.mapRooms[0].roomPoints.empty()) {
                return false;
            }
            *o_roomNo = (s8)map.mapRooms[0].roomNo;
            *o_point = map.mapRooms[0].roomPoints[0];
            return true;
        }
    }
    return false;
}

}  // namespace

void dALBWOocoo_onDeathWarpContext(const char* i_lastStageName, bool i_diedInDungeon) {
    sDiedInDungeon        = i_diedInDungeon;
    sChoseOrdonAfterDeath = false;
    sUsedOocooThisDeath   = false;
    sPendingEntranceWarp  = false;
    sDeathDungeonStage[0] = '\0';

    if (i_diedInDungeon && i_lastStageName != NULL) {
        int i = 0;
        for (; i < 7 && i_lastStageName[i] != '\0'; i++) {
            sDeathDungeonStage[i] = i_lastStageName[i];
        }
        sDeathDungeonStage[i] = '\0';
    }
}

void dALBWOocoo_onWarpChoice(int i_choice) {
    if (i_choice == 1 && sDiedInDungeon) {
        sChoseOrdonAfterDeath = true;
    }
}

bool dALBWOocoo_canShowInShop() {
    if (!postmanRentalActive()) {
        return false;
    }
    if (!hasMetOocooSrOnce()) {
        return false;
    }
    if (!sDiedInDungeon || !sChoseOrdonAfterDeath) {
        return false;
    }
    // ============================================
    // Never sell a warp we cannot land: if the death stage has no entry in
    // the warp table, hide the service row entirely instead of crashing or
    // refunding later.
    // ============================================
    s8 roomNo;
    s16 point;
    if (!resolveDungeonEntrance(sDeathDungeonStage, &roomNo, &point)) {
        return false;
    }
    return !sUsedOocooThisDeath;
}

bool dALBWOocoo_tryPurchase() {
    if (!dALBWOocoo_canShowInShop()) {
        return false;
    }
    if (sDeathDungeonStage[0] == '\0') {
        return false;
    }

    const u16 rupees = dComIfGs_getRupee();
    if (rupees < (u16)kPrice) {
        return false;
    }

    dComIfGs_setRupee(rupees - (u16)kPrice);
    sUsedOocooThisDeath  = true;
    sPendingEntranceWarp = true;
    return true;
}

void dALBWOocoo_executePendingWarp() {
    if (!sPendingEntranceWarp) {
        return;
    }
    sPendingEntranceWarp = false;

    if (sDeathDungeonStage[0] == '\0') {
        return;
    }

    // ============================================
    // Entrance spawn from the warp-table baseline (first room entry, first
    // point) — see resolveDungeonEntrance. setNextStage args are
    // (stage, point, room, layer); the old call passed a hardcoded room 0
    // that does not exist in D_MN04/D_MN09. If resolution fails here
    // (canShowInShop should have prevented the sale), refund and bail
    // rather than warp blind.
    // ============================================
    s8 roomNo;
    s16 point;
    if (!resolveDungeonEntrance(sDeathDungeonStage, &roomNo, &point)) {
        dComIfGs_setRupee(dComIfGs_getRupee() + (u16)kPrice);
        sUsedOocooThisDeath = false;
        return;
    }

    dComIfGp_setNextStage(sDeathDungeonStage, point, roomNo, -1);

    daAlink_c* link = daAlink_getAlinkActorClass();
    if (link != NULL) {
        link->procDungeonWarpSceneStartInit();
    }
}

const char* dALBWOocoo_getServiceName() {
    return "Oocoo's Return";
}

const char* dALBWOocoo_getServiceDesc() {
    return "A small friend flew all this way here to help! Let me feed him "
           "to restore his strength.";
}

int dALBWOocoo_getServicePrice() {
    return kPrice;
}

#endif  // TARGET_PC
