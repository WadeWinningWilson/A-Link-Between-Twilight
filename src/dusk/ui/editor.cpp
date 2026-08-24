#include "editor.hpp"

#include <RmlUi/Core.h>
#include <fmt/format.h>

#include "bool_button.hpp"
#include "button.hpp"
#include "d/actor/d_a_player.h"
#include "d/d_albw_potion.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_meter2_info.h"
#include "dusk/config.hpp"
#include "dusk/leveledit/enumerate.hpp"
#include "dusk/ui/input.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "dusk/map_loader_definitions.h"
#include "dusk/custom_assets.hpp"
#include "dusk/settings.h"
#include "dusk/truetest.hpp"
#include "d/d_ww_itemmdl_test.h"
#include "d/d_demo_leftover_viewer.h"
#include "d/d_cut_actor_spawn.h"
#include "d/d_ext_npc_mount.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_audio.h"
#include "Z2AudioLib/Z2SeqMgr.h"  // Wolf Howl tune preview (Z2BGM_* ids, Z2GetSeqMgr)
#include "number_button.hpp"
#include "pane.hpp"
#include "select_button.hpp"
#include "string_button.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace dusk::ui {

Rml::String stage_option_label(const MapEntry& map, bool showInternalNames) {
    return showInternalNames ? fmt::format("{} ({})", map.mapName, map.mapFile) : map.mapName;
}

Rml::String stage_label_for_file(const Rml::String& stageFile, bool showInternalNames) {
    for (const auto& region : gameRegions) {
        for (const auto& map : region.maps) {
            if (stageFile == map.mapFile) {
                return stage_option_label(map, showInternalNames);
            }
        }
    }
    return stageFile;
}

void populate_stage_picker(Pane& pane, std::function<Rml::String()> getStageFile,
    std::function<void(const char*)> setStageFile, bool showInternalNames) {
    pane.clear();
    for (const auto& region : gameRegions) {
        pane.add_section(region.regionName);
        for (const auto& map : region.maps) {
            pane.add_button({
                                .text = stage_option_label(map, showInternalNames),
                                .isSelected =
                                    [getStageFile, stageFile = map.mapFile] {
                                        return getStageFile() == stageFile;
                                    },
                            })
                .on_pressed([setStageFile, stageFile = map.mapFile] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    setStageFile(stageFile);
                });
        }
    }
}

namespace {

bool has_save_data() {
    return dComIfGs_getSaveData() != nullptr;
}

dSv_player_status_a_c* get_player_status() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getPlayerStatusA();
}

dSv_player_status_b_c* get_player_status_b() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getPlayerStatusB();
}

dSv_player_return_place_c* get_player_return_place() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getPlayerReturnPlace();
}

dSv_horse_place_c* get_horse_place() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getHorsePlace();
}

dSv_player_item_c* get_player_item() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getItem();
}

dSv_player_item_record_c* get_player_item_record() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getItemRecord();
}

dSv_player_item_max_c* get_player_item_max() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getItemMax();
}

dSv_fishing_info_c* get_player_fishing_info() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getFishingInfo();
}

dSv_MiniGame_c* get_minigame() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getMiniGame();
}

dSv_player_config_c* get_player_config() {
    if (!has_save_data()) {
        return nullptr;
    }
    return &dComIfGs_getSaveData()->getPlayer().getConfig();
}

template <size_t Size>
Rml::String fixed_string(const char (&value)[Size]) {
    size_t length = 0;
    while (length < Size && value[length] != '\0') {
        ++length;
    }
    return Rml::String(value, length);
}

template <size_t Size>
void set_fixed_string(char (&dest)[Size], const Rml::String& value) {
    std::memset(dest, 0, Size);
    std::memcpy(dest, value.data(), std::min(value.size(), Size - 1));
}

void skip_whitespace(const char*& cursor) {
    while (std::isspace(static_cast<unsigned char>(*cursor))) {
        ++cursor;
    }
}

bool parse_float_token(const char*& cursor, float& parsedValue) {
    skip_whitespace(cursor);
    char* end = nullptr;
    parsedValue = std::strtof(cursor, &end);
    if (end == cursor) {
        return false;
    }
    cursor = end;
    skip_whitespace(cursor);
    return true;
}

bool consume_comma(const char*& cursor) {
    skip_whitespace(cursor);
    if (*cursor != ',') {
        return false;
    }
    ++cursor;
    return true;
}

bool parse_vec3(const Rml::String& value, float& x, float& y, float& z) {
    const char* cursor = value.c_str();
    if (!parse_float_token(cursor, x) || !consume_comma(cursor) || !parse_float_token(cursor, y) ||
        !consume_comma(cursor) || !parse_float_token(cursor, z))
    {
        return false;
    }
    skip_whitespace(cursor);
    return *cursor == '\0';
}

Rml::String get_player_name() {
    if (!has_save_data()) {
        return "";
    }
    return dComIfGs_getPlayerName().buffer;
}

void set_player_name(Rml::String name) {
    dComIfGs_setPlayerName(name.c_str());
}

Rml::String get_horse_name() {
    if (!has_save_data()) {
        return "";
    }
    return dComIfGs_getHorseName().buffer;
}

void set_horse_name(Rml::String name) {
    dComIfGs_setHorseName(name.c_str());
}

enum ItemType {
    ITEMTYPE_DEFAULT_e,
    ITEMTYPE_EQUIP_e,
};

struct itemInfo {
    std::string m_name;
    u8 m_type = ITEMTYPE_DEFAULT_e;
};

std::map<int, itemInfo> itemMap = {
    {dItemNo_HEART_e, {"Heart"}},
    {dItemNo_GREEN_RUPEE_e, {"Green Rupee"}},
    {dItemNo_BLUE_RUPEE_e, {"Blue Rupee"}},
    {dItemNo_YELLOW_RUPEE_e, {"Yellow Rupee"}},
    {dItemNo_RED_RUPEE_e, {"Red Rupee"}},
    {dItemNo_PURPLE_RUPEE_e, {"Purple Rupee"}},
    {dItemNo_ORANGE_RUPEE_e, {"Orange Rupee"}},
    {dItemNo_SILVER_RUPEE_e, {"Silver Rupee"}},
    {dItemNo_S_MAGIC_e, {"Small Magic"}},
    {dItemNo_L_MAGIC_e, {"Large Magic"}},
    {dItemNo_BOMB_5_e, {"Bombs (5)"}},
    {dItemNo_BOMB_10_e, {"Bombs (10)"}},
    {dItemNo_BOMB_20_e, {"Bombs (20)"}},
    {dItemNo_BOMB_30_e, {"Bombs (30)"}},
    {dItemNo_ARROW_10_e, {"Arrows (10)"}},
    {dItemNo_ARROW_20_e, {"Arrows (20)"}},
    {dItemNo_ARROW_30_e, {"Arrows (30)"}},
    {dItemNo_ARROW_1_e, {"Arrows (1)"}},
    {dItemNo_PACHINKO_SHOT_e, {"Pumpkin Seeds"}},
    {dItemNo_NOENTRY_19_e, {"Reserved"}},
    {dItemNo_NOENTRY_20_e, {"Reserved"}},
    {dItemNo_NOENTRY_21_e, {"Reserved"}},
    {dItemNo_WATER_BOMB_5_e, {"Water Bombs (5)"}},
    {dItemNo_WATER_BOMB_10_e, {"Water Bombs (10)"}},
    {dItemNo_WATER_BOMB_20_e, {"Water Bombs (20)"}},
    {dItemNo_WATER_BOMB_30_e, {"Water Bombs (30)"}},
    {dItemNo_BOMB_INSECT_5_e, {"Bomblings (5)"}},
    {dItemNo_BOMB_INSECT_10_e, {"Bomblings (10)"}},
    {dItemNo_BOMB_INSECT_20_e, {"Bomblings (20)"}},
    {dItemNo_BOMB_INSECT_30_e, {"Bomblings (30)"}},
    {dItemNo_RECOVERY_FAILY_e, {"Fairy"}},
    {dItemNo_TRIPLE_HEART_e, {"Triple Hearts"}},
    {dItemNo_SMALL_KEY_e, {"Small Key"}},
    {dItemNo_KAKERA_HEART_e, {"Piece of Heart"}},
    {dItemNo_UTAWA_HEART_e, {"Heart Container"}},
    {dItemNo_MAP_e, {"Dungeon Map"}},
    {dItemNo_COMPUS_e, {"Compass"}},
    {dItemNo_DUNGEON_EXIT_e, {"Ooccoo Sr. (First Time)", ITEMTYPE_EQUIP_e}},
    {dItemNo_BOSS_KEY_e, {"Boss Key"}},
    {dItemNo_DUNGEON_BACK_e, {"Ooccoo Jr.", ITEMTYPE_EQUIP_e}},
    {dItemNo_SWORD_e, {"Ordon Sword"}},
    {dItemNo_MASTER_SWORD_e, {"Master Sword"}},
    // Enum names are misleading: WOOD_SHIELD (0x2A) is the Ordon house-quest shield,
    // SHIELD (0x2B) is the shop "Wooden" shield. Label by true identity, not enum name.
    {dItemNo_WOOD_SHIELD_e, {"Ordon Shield"}},
    {dItemNo_SHIELD_e, {"Wooden Shield"}},
    {dItemNo_HYLIA_SHIELD_e, {"Hylian Shield"}},
    {dItemNo_TKS_LETTER_e, {"Ooccoo's Note", ITEMTYPE_EQUIP_e}},
    {dItemNo_WEAR_CASUAL_e, {"Ordon Clothes"}},
    {dItemNo_WEAR_KOKIRI_e, {"Hero's Clothes"}},
    {dItemNo_ARMOR_e, {"Magic Armor"}},
    {dItemNo_WEAR_ZORA_e, {"Zora Armor"}},
    {dItemNo_MAGIC_LV1_e, {"Magic Level 1"}},
    {dItemNo_DUNGEON_EXIT_2_e, {"Ooccoo Sr.", ITEMTYPE_EQUIP_e}},
    {dItemNo_WALLET_LV1_e, {"Wallet"}},
    {dItemNo_WALLET_LV2_e, {"Big Wallet"}},
    {dItemNo_WALLET_LV3_e, {"Giant Wallet"}},
    {dItemNo_WALLET_LV4_e, {"Colossal Wallet"}},    // ALBW Port
    {dItemNo_DEITY_ARMOR_e, {"Deity Armor (flag)"}},  // ALBW Port
    {dItemNo_NOENTRY_57_e, {"Reserved"}},
    {dItemNo_NOENTRY_58_e, {"Reserved"}},
    {dItemNo_NOENTRY_59_e, {"Reserved"}},
    {dItemNo_NOENTRY_60_e, {"Reserved"}},
    {dItemNo_ZORAS_JEWEL_e, {"Coral Earring", ITEMTYPE_EQUIP_e}},
    {dItemNo_HAWK_EYE_e, {"Hawkeye", ITEMTYPE_EQUIP_e}},
    {dItemNo_WOOD_STICK_e, {"Wooden Sword"}},
    {dItemNo_BOOMERANG_e, {"Gale Boomerang", ITEMTYPE_EQUIP_e}},
    {dItemNo_SPINNER_e, {"Spinner", ITEMTYPE_EQUIP_e}},
    {dItemNo_IRONBALL_e, {"Ball and Chain", ITEMTYPE_EQUIP_e}},
    {dItemNo_BOW_e, {"Hero's Bow", ITEMTYPE_EQUIP_e}},
    {dItemNo_HOOKSHOT_e, {"Clawshot", ITEMTYPE_EQUIP_e}},
    {dItemNo_HVY_BOOTS_e, {"Iron Boots", ITEMTYPE_EQUIP_e}},
    {dItemNo_COPY_ROD_e, {"Dominion Rod", ITEMTYPE_EQUIP_e}},
    {dItemNo_W_HOOKSHOT_e, {"Double Clawshots", ITEMTYPE_EQUIP_e}},
    {dItemNo_KANTERA_e, {"Lantern", ITEMTYPE_EQUIP_e}},
    {dItemNo_LIGHT_SWORD_e, {"Light Sword"}},
    {dItemNo_FISHING_ROD_1_e, {"Fishing Rod", ITEMTYPE_EQUIP_e}},
    {dItemNo_PACHINKO_e, {"Slingshot", ITEMTYPE_EQUIP_e}},
    {dItemNo_COPY_ROD_2_e, {"Dominion Rod (Uncharged)"}},
    {dItemNo_NOENTRY_77_e, {"Reserved"}},
    {dItemNo_NOENTRY_78_e, {"Reserved"}},
    {dItemNo_BOMB_BAG_LV2_e, {"Giant Bomb Bag"}},
    {dItemNo_BOMB_BAG_LV1_e, {"Empty Bomb Bag", ITEMTYPE_EQUIP_e}},
    {dItemNo_BOMB_IN_BAG_e, {"Bomb Bag"}},
    {dItemNo_NOENTRY_82_e, {"Reserved"}},
    {dItemNo_LIGHT_ARROW_e, {"Light Arrow"}},
    {dItemNo_ARROW_LV1_e, {"Quiver"}},
    {dItemNo_ARROW_LV2_e, {"Big Quiver"}},
    {dItemNo_ARROW_LV3_e, {"Giant Quiver"}},
    {dItemNo_NOENTRY_87_e, {"Reserved"}},
    {dItemNo_LURE_ROD_e, {"Fishing Rod (Lure)"}},
    {dItemNo_BOMB_ARROW_e, {"Bomb Arrow"}},
    {dItemNo_HAWK_ARROW_e, {"Hawk Arrow"}},
    {dItemNo_BEE_ROD_e, {"Fishing Rod (Bee Larva)", ITEMTYPE_EQUIP_e}},
    {dItemNo_JEWEL_ROD_e, {"Fishing Rod (Earring)", ITEMTYPE_EQUIP_e}},
    {dItemNo_WORM_ROD_e, {"Fishing Rod (Worm)", ITEMTYPE_EQUIP_e}},
    {dItemNo_JEWEL_BEE_ROD_e, {"Fishing Rod (Earring + Bee Larva)", ITEMTYPE_EQUIP_e}},
    {dItemNo_JEWEL_WORM_ROD_e, {"Fishing Rod (Earring + Worm)", ITEMTYPE_EQUIP_e}},
    {dItemNo_EMPTY_BOTTLE_e, {"Empty Bottle", ITEMTYPE_EQUIP_e}},
    {dItemNo_RED_BOTTLE_e, {"Red Potion", ITEMTYPE_EQUIP_e}},
    {dItemNo_GREEN_BOTTLE_e, {"Green Potion", ITEMTYPE_EQUIP_e}},
    {dItemNo_BLUE_BOTTLE_e, {"Blue Potion", ITEMTYPE_EQUIP_e}},
    {dItemNo_MILK_BOTTLE_e, {"Milk Bottle", ITEMTYPE_EQUIP_e}},
    {dItemNo_HALF_MILK_BOTTLE_e, {"Half Milk Bottle", ITEMTYPE_EQUIP_e}},
    {dItemNo_OIL_BOTTLE_e, {"Lantern Oil", ITEMTYPE_EQUIP_e}},
    {dItemNo_WATER_BOTTLE_e, {"Water Bottle", ITEMTYPE_EQUIP_e}},
    {dItemNo_OIL_BOTTLE_2_e, {"Lantern Oil (Scooped)"}},
    {dItemNo_RED_BOTTLE_2_e, {"Red Potion (Scooped)"}},
    {dItemNo_UGLY_SOUP_e, {"Nasty Soup", ITEMTYPE_EQUIP_e}},
    {dItemNo_HOT_SPRING_e, {"Hotspring Water", ITEMTYPE_EQUIP_e}},
    {dItemNo_FAIRY_e, {"Fairy", ITEMTYPE_EQUIP_e}},
    {dItemNo_HOT_SPRING_2_e, {"Hotspring Water (Shop)"}},
    {dItemNo_OIL2_e, {"Lantern Refill (Scooped)"}},
    {dItemNo_OIL_e, {"Lantern Refill (Shop)"}},
    {dItemNo_NORMAL_BOMB_e, {"Bombs", ITEMTYPE_EQUIP_e}},
    {dItemNo_WATER_BOMB_e, {"Water Bombs", ITEMTYPE_EQUIP_e}},
    {dItemNo_POKE_BOMB_e, {"Bomblings", ITEMTYPE_EQUIP_e}},
    {dItemNo_FAIRY_DROP_e, {"Great Fairy's Tears", ITEMTYPE_EQUIP_e}},
    {dItemNo_WORM_e, {"Worm", ITEMTYPE_EQUIP_e}},
    {dItemNo_DROP_BOTTLE_e, {"Great Fairy Tears (Jovani)"}},
    {dItemNo_BEE_CHILD_e, {"Bee Larva", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_RARE_e, {"Rare Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_RED_e, {"Red Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_BLUE_e, {"Blue Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_GREEN_e, {"Green Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_YELLOW_e, {"Yellow Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_CHUCHU_PURPLE_e, {"Purple Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_LV1_SOUP_e, {"Simple Soup", ITEMTYPE_EQUIP_e}},
    {dItemNo_LV2_SOUP_e, {"Good Soup", ITEMTYPE_EQUIP_e}},
    {dItemNo_LV3_SOUP_e, {"Superb Soup", ITEMTYPE_EQUIP_e}},
    {dItemNo_LETTER_e, {"Renado's Letter", ITEMTYPE_EQUIP_e}},
    {dItemNo_BILL_e, {"Invoice", ITEMTYPE_EQUIP_e}},
    {dItemNo_WOOD_STATUE_e, {"Wooden Statue", ITEMTYPE_EQUIP_e}},
    {dItemNo_IRIAS_PENDANT_e, {"Ilia's Charm", ITEMTYPE_EQUIP_e}},
    {dItemNo_HORSE_FLUTE_e, {"Horse Call", ITEMTYPE_EQUIP_e}},
    {dItemNo_NOENTRY_133_e, {"Reserved"}},
    {dItemNo_NOENTRY_134_e, {"Reserved"}},
    {dItemNo_NOENTRY_135_e, {"Reserved"}},
    {dItemNo_NOENTRY_136_e, {"Reserved"}},
    {dItemNo_NOENTRY_137_e, {"Reserved"}},
    {dItemNo_NOENTRY_138_e, {"Reserved"}},
    {dItemNo_NOENTRY_139_e, {"Reserved"}},
    {dItemNo_NOENTRY_140_e, {"Reserved"}},
    {dItemNo_NOENTRY_141_e, {"Reserved"}},
    {dItemNo_NOENTRY_142_e, {"Reserved"}},
    {dItemNo_NOENTRY_143_e, {"Reserved"}},
    {dItemNo_RAFRELS_MEMO_e, {"Auru's Memo", ITEMTYPE_EQUIP_e}},
    {dItemNo_ASHS_SCRIBBLING_e, {"Ashei's Sketch", ITEMTYPE_EQUIP_e}},
    {dItemNo_NOENTRY_146_e, {"Reserved"}},
    {dItemNo_NOENTRY_147_e, {"Reserved"}},
    {dItemNo_NOENTRY_148_e, {"Reserved"}},
    {dItemNo_NOENTRY_149_e, {"Reserved"}},
    {dItemNo_NOENTRY_150_e, {"Reserved"}},
    {dItemNo_NOENTRY_151_e, {"Reserved"}},
    {dItemNo_NOENTRY_152_e, {"Reserved"}},
    {dItemNo_NOENTRY_153_e, {"Reserved"}},
    {dItemNo_NOENTRY_154_e, {"Reserved"}},
    {dItemNo_NOENTRY_155_e, {"Reserved"}},
    {dItemNo_CHUCHU_YELLOW2_e, {"Lantern Refill (Yellow Chu)"}},
    {dItemNo_OIL_BOTTLE3_e, {"Lantern Oil (Coro)"}},
    {dItemNo_SHOP_BEE_CHILD_e, {"Bee Larve (Shop)"}},
    {dItemNo_CHUCHU_BLACK_e, {"Black Chu Jelly", ITEMTYPE_EQUIP_e}},
    {dItemNo_LIGHT_DROP_e, {"Tear of Light"}},
    {dItemNo_DROP_CONTAINER_e, {"Vessel of Light (Faron)"}},
    {dItemNo_DROP_CONTAINER02_e, {"Vessel of Light (Eldin)"}},
    {dItemNo_DROP_CONTAINER03_e, {"Vessel of Light (Lanayru)"}},
    {dItemNo_FILLED_CONTAINER_e, {"Vessel of Light (Filled)"}},
    {dItemNo_MIRROR_PIECE_2_e, {"Mirror Shard (Snowpeak Ruins)"}},
    {dItemNo_MIRROR_PIECE_3_e, {"Mirror Shard (Temple of Time)"}},
    {dItemNo_MIRROR_PIECE_4_e, {"Mirror Shard (City in the Sky)"}},
    {dItemNo_NOENTRY_168_e, {"Reserved"}},
    {dItemNo_NOENTRY_169_e, {"Reserved"}},
    {dItemNo_NOENTRY_170_e, {"Reserved"}},
    {dItemNo_NOENTRY_171_e, {"Reserved"}},
    {dItemNo_NOENTRY_172_e, {"Reserved"}},
    {dItemNo_NOENTRY_173_e, {"Reserved"}},
    {dItemNo_NOENTRY_174_e, {"Reserved"}},
    {dItemNo_NOENTRY_175_e, {"Reserved"}},
    {dItemNo_SMELL_YELIA_POUCH_e, {"Scent of Ilia"}},
    {dItemNo_SMELL_PUMPKIN_e, {"Pumpkin Scent"}},
    {dItemNo_SMELL_POH_e, {"Poe Scent"}},
    {dItemNo_SMELL_FISH_e, {"Reekfish Scent"}},
    {dItemNo_SMELL_CHILDREN_e, {"Youth's Scent"}},
    {dItemNo_SMELL_MEDICINE_e, {"Medicine Scent"}},
    {dItemNo_NOENTRY_182_e, {"Reserved"}},
    {dItemNo_NOENTRY_183_e, {"Reserved"}},
    {dItemNo_NOENTRY_184_e, {"Reserved"}},
    {dItemNo_NOENTRY_185_e, {"Reserved"}},
    {dItemNo_NOENTRY_186_e, {"Reserved"}},
    {dItemNo_NOENTRY_187_e, {"Reserved"}},
    {dItemNo_NOENTRY_188_e, {"Reserved"}},
    {dItemNo_NOENTRY_189_e, {"Reserved"}},
    {dItemNo_NOENTRY_190_e, {"Reserved"}},
    {dItemNo_NOENTRY_191_e, {"Reserved"}},
    {dItemNo_M_BEETLE_e, {"Beetle (M)"}},
    {dItemNo_F_BEETLE_e, {"Beetle (F)"}},
    {dItemNo_M_BUTTERFLY_e, {"Butterfly (M)"}},
    {dItemNo_F_BUTTERFLY_e, {"Butterfly (F)"}},
    {dItemNo_M_STAG_BEETLE_e, {"Stag Beetle (M)"}},
    {dItemNo_F_STAG_BEETLE_e, {"Stag Beetle (F)"}},
    {dItemNo_M_GRASSHOPPER_e, {"Grasshopper (M)"}},
    {dItemNo_F_GRASSHOPPER_e, {"Grasshopper (F)"}},
    {dItemNo_M_NANAFUSHI_e, {"Phasmid (M)"}},
    {dItemNo_F_NANAFUSHI_e, {"Phasmid (F)"}},
    {dItemNo_M_DANGOMUSHI_e, {"Pill Bug (M)"}},
    {dItemNo_F_DANGOMUSHI_e, {"Pill Bug (F)"}},
    {dItemNo_M_MANTIS_e, {"Mantis (M)"}},
    {dItemNo_F_MANTIS_e, {"Mantis (F)"}},
    {dItemNo_M_LADYBUG_e, {"Ladybug (M)"}},
    {dItemNo_F_LADYBUG_e, {"Ladybug (F)"}},
    {dItemNo_M_SNAIL_e, {"Snail (M)"}},
    {dItemNo_F_SNAIL_e, {"Snail (F)"}},
    {dItemNo_M_DRAGONFLY_e, {"Dragonfly (M)"}},
    {dItemNo_F_DRAGONFLY_e, {"Dragonfly (F)"}},
    {dItemNo_M_ANT_e, {"Ant (M)"}},
    {dItemNo_F_ANT_e, {"Ant (F)"}},
    {dItemNo_M_MAYFLY_e, {"Mayfly (M)"}},
    {dItemNo_F_MAYFLY_e, {"Mayfly (F)"}},
    {dItemNo_NOENTRY_216_e, {"Reserved"}},
    {dItemNo_NOENTRY_217_e, {"Reserved"}},
    {dItemNo_NOENTRY_218_e, {"Reserved"}},
    {dItemNo_NOENTRY_219_e, {"Reserved"}},
    {dItemNo_NOENTRY_220_e, {"Reserved"}},
    {dItemNo_NOENTRY_221_e, {"Reserved"}},
    {dItemNo_NOENTRY_222_e, {"Reserved"}},
    {dItemNo_NOENTRY_223_e, {"Reserved"}},
    {dItemNo_POU_SPIRIT_e, {"Poe Soul"}},
    {dItemNo_NOENTRY_225_e, {"Reserved"}},
    {dItemNo_NOENTRY_226_e, {"Reserved"}},
    {dItemNo_NOENTRY_227_e, {"Reserved"}},
    {dItemNo_NOENTRY_228_e, {"Reserved"}},
    {dItemNo_NOENTRY_229_e, {"Reserved"}},
    {dItemNo_NOENTRY_230_e, {"Reserved"}},
    {dItemNo_NOENTRY_231_e, {"Reserved"}},
    {dItemNo_NOENTRY_232_e, {"Reserved"}},
    {dItemNo_ANCIENT_DOCUMENT_e, {"Ancient Sky Book", ITEMTYPE_EQUIP_e}},
    {dItemNo_AIR_LETTER_e, {"Ancient Sky Book (Partial)", ITEMTYPE_EQUIP_e}},
    {dItemNo_ANCIENT_DOCUMENT2_e, {"Ancient Sky Book (Filled)", ITEMTYPE_EQUIP_e}},
    {dItemNo_LV7_DUNGEON_EXIT_e, {"Ooccoo Sr. (City in the Sky)"}},
    {dItemNo_LINKS_SAVINGS_e, {"Purple Rupee (Link's Savings)"}},
    {dItemNo_SMALL_KEY2_e, {"Small Key (North Faron Gate)"}},
    {dItemNo_POU_FIRE1_e, {"Poe Fire 1"}},
    {dItemNo_POU_FIRE2_e, {"Poe Fire 2"}},
    {dItemNo_POU_FIRE3_e, {"Poe Fire 3"}},
    {dItemNo_POU_FIRE4_e, {"Poe Fire 4"}},
    {dItemNo_BOSSRIDER_KEY_e, {"Hyrule Field Keys"}},
    {dItemNo_TOMATO_PUREE_e, {"Ordon Pumpkin", ITEMTYPE_EQUIP_e}},
    {dItemNo_TASTE_e, {"Ordon Goat Cheese", ITEMTYPE_EQUIP_e}},
    {dItemNo_LV5_BOSS_KEY_e, {"Bedroom Key"}},
    {dItemNo_SURFBOARD_e, {"Surf Leaf"}},
    {dItemNo_KANTERA2_e, {"Lantern (Reclaimed)"}},
    {dItemNo_L2_KEY_PIECES1_e, {"Key Shard (1)"}},
    {dItemNo_L2_KEY_PIECES2_e, {"Key Shard (2)"}},
    {dItemNo_L2_KEY_PIECES3_e, {"Key Shard (3)"}},
    {dItemNo_KEY_OF_CARAVAN_e, {"Bulblin Camp Key"}},
    {dItemNo_LV2_BOSS_KEY_e, {"Goron Mines Boss Key"}},
    {dItemNo_KEY_OF_FILONE_e, {"South Faron Gate Key"}},
    {dItemNo_NONE_e, {"None"}},
};

Rml::String get_item_name(u8 id) {
    const auto it = itemMap.find(id);
    if (it == itemMap.end()) {
        return fmt::format("Item {}", id);
    }
    return it->second.m_name;
}

Rml::String item_label_for_slot(u8 slot) {
    if (slot == 0xFF) {
        return "None";
    }
    const auto id = dComIfGs_getSaveData()->getPlayer().getItem().mItems[slot];
    return fmt::format("Slot {0} ({1})", slot, get_item_name(id));
}

struct NamedIndexEntry {
    const char* name;
    u8 index;
};

struct NamedFlagEntry {
    const char* name;
    u16 flag;
};

struct BugSpeciesEntry {
    const char* name;
    u8 maleItem;
    u8 femaleItem;
    u16 maleTurnInFlag;
    u16 femaleTurnInFlag;
};

struct FishSpeciesEntry {
    const char* name;
    u8 index;
};

constexpr std::array<u8, 4> swordEntries = {
    dItemNo_SWORD_e,
    dItemNo_MASTER_SWORD_e,
    dItemNo_WOOD_STICK_e,
    dItemNo_LIGHT_SWORD_e,
};

constexpr std::array<u8, 3> shieldEntries = {
    dItemNo_SHIELD_e,
    dItemNo_WOOD_SHIELD_e,
    dItemNo_HYLIA_SHIELD_e,
};

constexpr std::array<u8, 5> smellEntries = {
    dItemNo_SMELL_CHILDREN_e,
    dItemNo_SMELL_YELIA_POUCH_e,
    dItemNo_SMELL_POH_e,
    dItemNo_SMELL_FISH_e,
    dItemNo_SMELL_MEDICINE_e,
};

constexpr std::array fusedShadowEntries = {
    NamedIndexEntry{"Forest Temple", 0},
    NamedIndexEntry{"Goron Mines", 1},
    NamedIndexEntry{"Lakebed Temple", 2},
};

constexpr std::array mirrorShardEntries = {
    NamedIndexEntry{"Snowpeak Ruins", 1},
    NamedIndexEntry{"Temple of Time", 2},
    NamedIndexEntry{"City in the Sky", 3},
};

constexpr std::array bugSpeciesEntries = {
    BugSpeciesEntry{"Ant", dItemNo_M_ANT_e, dItemNo_F_ANT_e, dSv_event_flag_c::F_0421,
        dSv_event_flag_c::F_0422},
    BugSpeciesEntry{"Dayfly", dItemNo_M_MAYFLY_e, dItemNo_F_MAYFLY_e, dSv_event_flag_c::F_0423,
        dSv_event_flag_c::F_0424},
    BugSpeciesEntry{"Beetle", dItemNo_M_BEETLE_e, dItemNo_F_BEETLE_e, dSv_event_flag_c::F_0401,
        dSv_event_flag_c::F_0402},
    BugSpeciesEntry{"Mantis", dItemNo_M_MANTIS_e, dItemNo_F_MANTIS_e, dSv_event_flag_c::F_0413,
        dSv_event_flag_c::F_0414},
    BugSpeciesEntry{"Stag Beetle", dItemNo_M_STAG_BEETLE_e, dItemNo_F_STAG_BEETLE_e,
        dSv_event_flag_c::F_0405, dSv_event_flag_c::F_0406},
    BugSpeciesEntry{"Pill Bug", dItemNo_M_DANGOMUSHI_e, dItemNo_F_DANGOMUSHI_e,
        dSv_event_flag_c::F_0411, dSv_event_flag_c::F_0412},
    BugSpeciesEntry{"Butterfly", dItemNo_M_BUTTERFLY_e, dItemNo_F_BUTTERFLY_e,
        dSv_event_flag_c::F_0403, dSv_event_flag_c::F_0404},
    BugSpeciesEntry{"Ladybug", dItemNo_M_LADYBUG_e, dItemNo_F_LADYBUG_e, dSv_event_flag_c::F_0415,
        dSv_event_flag_c::F_0416},
    BugSpeciesEntry{"Snail", dItemNo_M_SNAIL_e, dItemNo_F_SNAIL_e, dSv_event_flag_c::F_0417,
        dSv_event_flag_c::F_0418},
    BugSpeciesEntry{"Phasmid", dItemNo_M_NANAFUSHI_e, dItemNo_F_NANAFUSHI_e,
        dSv_event_flag_c::F_0409, dSv_event_flag_c::F_0410},
    BugSpeciesEntry{"Grasshopper", dItemNo_M_GRASSHOPPER_e, dItemNo_F_GRASSHOPPER_e,
        dSv_event_flag_c::F_0407, dSv_event_flag_c::F_0408},
    BugSpeciesEntry{"Dragonfly", dItemNo_M_DRAGONFLY_e, dItemNo_F_DRAGONFLY_e,
        dSv_event_flag_c::F_0419, dSv_event_flag_c::F_0420},
};

constexpr std::array<NamedFlagEntry, 7> hiddenSkillEntries = {
    NamedFlagEntry{"Ending Blow", dSv_event_flag_c::F_0339},
    NamedFlagEntry{"Shield Attack", dSv_event_flag_c::F_0338},
    NamedFlagEntry{"Back Slice", dSv_event_flag_c::F_0340},
    NamedFlagEntry{"Helm Splitter", dSv_event_flag_c::F_0341},
    NamedFlagEntry{"Mortal Draw", dSv_event_flag_c::F_0342},
    NamedFlagEntry{"Jump Strike", dSv_event_flag_c::F_0343},
    NamedFlagEntry{"Great Spin", dSv_event_flag_c::F_0344},
};

constexpr std::array<const char*, 16> letterSenders = {
    "Renado",
    "Ooccoo 1",
    "Ooccoo 2",
    "The Postman",
    "Kakariko Goods",
    "Barnes 1",
    "Barnes 2",
    "Barnes Bombs",
    "Malo Mart",
    "Telma",
    "Purlo",
    "From Jr.",
    "Princess Agitha",
    "Lanayru Tourism",
    "Shad",
    "Yeta",
};

constexpr std::array<FishSpeciesEntry, 6> fishSpeciesEntries = {
    FishSpeciesEntry{"Ordon Catfish", 3},
    FishSpeciesEntry{"Greengill", 5},
    FishSpeciesEntry{"Reekfish", 4},
    FishSpeciesEntry{"Hyrule Bass", 0},
    FishSpeciesEntry{"Hylian Pike", 2},
    FishSpeciesEntry{"Hylian Loach", 1},
};

constexpr std::array<const char*, 2> targetTypeNames = {
    "Hold",
    "Switch",
};

constexpr std::array<const char*, 3> soundModeNames = {
    "Mono",
    "Stereo",
    "Surround",
};

struct DefaultInventoryEntry {
    u8 slot;
    u8 item;
};

constexpr std::array<DefaultInventoryEntry, 22> defaultInventory = {
    DefaultInventoryEntry{SLOT_0, dItemNo_BOOMERANG_e},
    DefaultInventoryEntry{SLOT_1, dItemNo_KANTERA_e},
    DefaultInventoryEntry{SLOT_2, dItemNo_SPINNER_e},
    DefaultInventoryEntry{SLOT_3, dItemNo_HVY_BOOTS_e},
    DefaultInventoryEntry{SLOT_4, dItemNo_BOW_e},
    DefaultInventoryEntry{SLOT_5, dItemNo_HAWK_EYE_e},
    DefaultInventoryEntry{SLOT_6, dItemNo_IRONBALL_e},
    DefaultInventoryEntry{SLOT_8, dItemNo_COPY_ROD_e},
    DefaultInventoryEntry{SLOT_9, dItemNo_HOOKSHOT_e},
    DefaultInventoryEntry{SLOT_10, dItemNo_W_HOOKSHOT_e},
    DefaultInventoryEntry{SLOT_11, dItemNo_RED_BOTTLE_e},
    DefaultInventoryEntry{SLOT_12, dItemNo_EMPTY_BOTTLE_e},
    DefaultInventoryEntry{SLOT_13, dItemNo_EMPTY_BOTTLE_e},
    DefaultInventoryEntry{SLOT_14, dItemNo_EMPTY_BOTTLE_e},
    DefaultInventoryEntry{SLOT_15, dItemNo_NORMAL_BOMB_e},
    DefaultInventoryEntry{SLOT_16, dItemNo_WATER_BOMB_e},
    DefaultInventoryEntry{SLOT_17, dItemNo_POKE_BOMB_e},
    DefaultInventoryEntry{SLOT_18, dItemNo_DUNGEON_EXIT_e},
    DefaultInventoryEntry{SLOT_20, dItemNo_FISHING_ROD_1_e},
    DefaultInventoryEntry{SLOT_21, dItemNo_HORSE_FLUTE_e},
    DefaultInventoryEntry{SLOT_22, dItemNo_ANCIENT_DOCUMENT_e},
    DefaultInventoryEntry{SLOT_23, dItemNo_PACHINKO_e},
};

u8 get_slot_default(int slot) {
    for (const auto& entry : defaultInventory) {
        if (entry.slot == slot) {
            return entry.item;
        }
    }
    return dItemNo_NONE_e;
}

void set_item_first_bit(u8 itemNo, bool owned) {
    if (dMeter2_isShieldItem(itemNo)) {
        dMeter2_setShieldOwned(itemNo, owned);
        return;
    }

    if (owned) {
        dComIfGs_onItemFirstBit(itemNo);
    } else {
        dComIfGs_offItemFirstBit(itemNo);
    }
}

void toggle_item_first_bit(u8 itemNo) {
    set_item_first_bit(itemNo, !dComIfGs_isItemFirstBit(itemNo));
}

void set_event_bit(u16 flag, bool enabled) {
    if (enabled) {
        dComIfGs_onEventBit(flag);
    } else {
        dComIfGs_offEventBit(flag);
    }
}

void set_letter_get_flag(int index, bool received) {
    if (received) {
        if (dComIfGs_isLetterGetFlag(index)) {
            return;
        }
        dComIfGs_onLetterGetFlag(index);
        const u8 slot = dMeter2Info_getRecieveLetterNum() - 1;
        if (slot < 64) {
            dComIfGs_setGetNumber(slot, static_cast<u8>(index + 1));
        }
    } else {
        if (!dComIfGs_isLetterGetFlag(index)) {
            return;
        }
        auto& info = dComIfGs_getSaveData()->getPlayer().getLetterInfo();
        info.mLetterGetFlags[index >> 5] &= ~(1u << (index & 0x1F));
        for (int slot = 0; slot < 64; ++slot) {
            if (dComIfGs_getGetNumber(slot) != index + 1) {
                continue;
            }
            for (int nextSlot = slot; nextSlot < 63; ++nextSlot) {
                dComIfGs_setGetNumber(nextSlot, dComIfGs_getGetNumber(nextSlot + 1));
            }
            dComIfGs_setGetNumber(63, 0);
            break;
        }
    }
}

void set_max_life(int maxLife) {
    maxLife = std::clamp(maxLife, 15, 100);
    dComIfGs_setMaxLife(static_cast<u8>(maxLife));
    const u16 maxHealth = (dComIfGs_getMaxLife() / 5) * 4;
    if (dComIfGs_getLife() > maxHealth) {
        dComIfGs_setLife(maxHealth);
    }
}

Rml::String max_life_label() {
    const int maxLife = dComIfGs_getMaxLife();
    return fmt::format("{} hearts + {} pieces", maxLife / 5, maxLife % 5);
}

struct ToggleEntry {
    Rml::String text;
    std::function<bool()> isSelected;
    std::function<void(bool)> setSelected;
};

void populate_toggle_group(Pane& pane, const std::vector<ToggleEntry>& entries) {
    pane.clear();
    pane.add_section("Actions");
    pane.add_button("Select All").on_pressed([entries] {
        mDoAud_seStartMenu(kSoundItemChange);
        for (const auto& entry : entries) {
            entry.setSelected(true);
        }
    });
    pane.add_button("Select None").on_pressed([entries] {
        mDoAud_seStartMenu(kSoundItemChange);
        for (const auto& entry : entries) {
            entry.setSelected(false);
        }
    });

    pane.add_section("Items");
    for (const auto& entry : entries) {
        pane.add_button({
                            .text = entry.text,
                            .isSelected = entry.isSelected,
                        })
            .on_pressed([isSelected = entry.isSelected, setSelected = entry.setSelected] {
                mDoAud_seStartMenu(kSoundItemChange);
                setSelected(!isSelected());
            });
    }
}

template <size_t Size>
int count_item_first_bits(const std::array<u8, Size>& entries) {
    int count = 0;
    for (const auto item : entries) {
        if (dComIfGs_isItemFirstBit(item)) {
            ++count;
        }
    }
    return count;
}

template <size_t Size>
int count_event_bits(const std::array<NamedFlagEntry, Size>& entries) {
    int count = 0;
    for (const auto& entry : entries) {
        if (dComIfGs_isEventBit(entry.flag)) {
            ++count;
        }
    }
    return count;
}

template <size_t Size>
int count_collect_crystals(const std::array<NamedIndexEntry, Size>& entries) {
    int count = 0;
    for (const auto& entry : entries) {
        if (dComIfGs_isCollectCrystal(entry.index)) {
            ++count;
        }
    }
    return count;
}

template <size_t Size>
int count_collect_mirrors(const std::array<NamedIndexEntry, Size>& entries) {
    int count = 0;
    for (const auto& entry : entries) {
        if (dComIfGs_isCollectMirror(entry.index)) {
            ++count;
        }
    }
    return count;
}

Rml::String count_label(int count, int total) {
    return fmt::format("{} / {}", count, total);
}

int count_clothing() {
    int count = 0;
    if (dComIfGs_isItemFirstBit(dItemNo_WEAR_CASUAL_e)) {
        ++count;
    }
    if (dComIfGs_isCollectClothes(KOKIRI_CLOTHES_FLAG)) {
        ++count;
    }
    if (dComIfGs_isItemFirstBit(dItemNo_WEAR_ZORA_e)) {
        ++count;
    }
    if (dComIfGs_isItemFirstBit(dItemNo_ARMOR_e)) {
        ++count;
    }
    return count;
}

int count_letters() {
    int count = 0;
    for (int index = 0; index < letterSenders.size(); ++index) {
        if (dComIfGs_isLetterGetFlag(index)) {
            ++count;
        }
    }
    return count;
}

Rml::String bug_species_label(const BugSpeciesEntry& bug) {
    int owned = 0;
    int given = 0;
    if (dComIfGs_isItemFirstBit(bug.maleItem)) {
        ++owned;
    }
    if (dComIfGs_isItemFirstBit(bug.femaleItem)) {
        ++owned;
    }
    if (dComIfGs_isEventBit(bug.maleTurnInFlag)) {
        ++given;
    }
    if (dComIfGs_isEventBit(bug.femaleTurnInFlag)) {
        ++given;
    }
    return fmt::format("{} / 2 owned, {} / 2 given", owned, given);
}

Rml::String fish_species_label(const FishSpeciesEntry& fish) {
    return fmt::format(
        "{} caught, {} cm", dComIfGs_getFishNum(fish.index), dComIfGs_getFishSize(fish.index));
}

bool can_edit_item_first_bit(int itemId, const itemInfo& item) {
    return itemId < 254 && item.m_name != "Reserved";
}

void set_all_item_first_bits(bool owned) {
    for (const auto& [itemId, item] : itemMap) {
        if (!can_edit_item_first_bit(itemId, item)) {
            continue;
        }
        set_item_first_bit(static_cast<u8>(itemId), owned);
    }
}

void populate_item_slot_picker(Pane& pane, int slot) {
    pane.clear();
    pane.add_section("Actions");
    pane.add_button(fmt::format("Default ({})", get_item_name(get_slot_default(slot))))
        .on_pressed([slot] {
            mDoAud_seStartMenu(kSoundItemChange);
            const u8 itemId = get_slot_default(slot);
            dComIfGs_setItem(slot, itemId);
            if (itemId == dItemNo_NORMAL_BOMB_e || itemId == dItemNo_WATER_BOMB_e ||
                itemId == dItemNo_POKE_BOMB_e)
            {
                const int bagIdx = slot - SLOT_15;
                if (bagIdx >= 0 && bagIdx < 3 && dComIfGs_getBombNum(bagIdx) == 0) {
                    u8 maxNum = dComIfGs_getBombMax(itemId);
                    dComIfGs_setBombNum(bagIdx, maxNum > 0 ? maxNum : 30);
                }
            }
        });

    pane.add_section("Items");
    pane.add_button(
            {
                .text = "None",
                .isSelected = [slot] { return get_player_item()->mItems[slot] == dItemNo_NONE_e; },
            })
        .on_pressed([slot] {
            mDoAud_seStartMenu(kSoundItemChange);
            dComIfGs_setItem(slot, dItemNo_NONE_e);
        });
    for (const auto& [itemId, item] : itemMap) {
        if (item.m_type != ITEMTYPE_EQUIP_e) {
            continue;
        }
        pane
            .add_button({
                .text = item.m_name,
                .isSelected = [slot, itemId] { return get_player_item()->mItems[slot] == itemId; },
            })
            .on_pressed([slot, itemId] {
                mDoAud_seStartMenu(kSoundItemChange);
                const u8 id = static_cast<u8>(itemId);
                // Bomb types MUST live in SLOT_15/16/17 — ammo/HUD use (slot-15) as bag
                // index. Placing them elsewhere OOBs and crashes. Redirect to the
                // canonical bag slot; XY buttons should point at that bag slot.
                int bagSlot = -1;
                if (id == dItemNo_NORMAL_BOMB_e) {
                    bagSlot = SLOT_15;
                } else if (id == dItemNo_WATER_BOMB_e) {
                    bagSlot = SLOT_16;
                } else if (id == dItemNo_POKE_BOMB_e) {
                    bagSlot = SLOT_17;
                }

                if (bagSlot >= 0) {
                    dComIfGs_setItem(static_cast<u8>(bagSlot), id);
                    dComIfGs_onItemFirstBit(id);
                    const int bagIdx = bagSlot - SLOT_15;
                    u8 maxNum = dComIfGs_getBombMax(id);
                    if (maxNum == 0) {
                        maxNum = 30;
                    }
                    if (dComIfGs_getBombNum(bagIdx) == 0) {
                        dComIfGs_setBombNum(bagIdx, maxNum);
                    }
                    // If the user edited a non-bag slot, do not leave a bomb type there.
                    if (slot != bagSlot &&
                        (get_player_item()->mItems[slot] == dItemNo_NORMAL_BOMB_e ||
                         get_player_item()->mItems[slot] == dItemNo_WATER_BOMB_e ||
                         get_player_item()->mItems[slot] == dItemNo_POKE_BOMB_e))
                    {
                        dComIfGs_setItem(slot, dItemNo_NONE_e);
                    }
                } else {
                    dComIfGs_setItem(slot, id);
                }
            });
    }
}

void populate_item_flag_picker(Pane& pane) {
    pane.clear();
    pane.add_section("Actions");
    pane.add_button("Select All").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        set_all_item_first_bits(true);
    });
    pane.add_button("Clear None").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        set_all_item_first_bits(false);
    });

    pane.add_section("Items");
    for (const auto& [itemId, item] : itemMap) {
        if (!can_edit_item_first_bit(itemId, item)) {
            continue;
        }
        pane
            .add_button({
                .text = item.m_name,
                .isSelected = [itemId] { return dComIfGs_isItemFirstBit(static_cast<u8>(itemId)); },
            })
            .on_pressed([itemId] {
                mDoAud_seStartMenu(kSoundItemChange);
                toggle_item_first_bit(static_cast<u8>(itemId));
            });
    }
}

void populate_select_item_picker(Pane& pane, u8& selectItemData) {
    pane.clear();
    pane.add_button(
            {
                .text = "None",
                .isSelected = [&selectItemData] { return selectItemData == dItemNo_NONE_e; },
            })
        .on_pressed([&selectItemData] {
            mDoAud_seStartMenu(kSoundItemChange);
            selectItemData = dItemNo_NONE_e;
        });
    for (int i = 0; i < 24; i++) {
        pane.add_button({
                            .text = item_label_for_slot(i),
                            .isSelected = [i, &selectItemData] { return selectItemData == i; },
                        })
            .on_pressed([i, &selectItemData] {
                mDoAud_seStartMenu(kSoundItemChange);
                selectItemData = i;
            });
    }
}

void populate_select_clothes_picker(Pane& pane) {
    pane.clear();
    const auto addOption = [&pane](u8 id) {
        pane.add_button(
                {
                    .text = get_item_name(id),
                    .isSelected = [id] { return get_player_status()->mSelectEquip[0] == id; },
                })
            .on_pressed([id] {
                mDoAud_seStartMenu(kSoundItemChange);
                dMeter2Info_setCloth(id, false);
                daPy_getPlayerActorClass()->setClothesChange(0);
            });
    };
    addOption(dItemNo_WEAR_CASUAL_e);
    addOption(dItemNo_WEAR_KOKIRI_e);
    addOption(dItemNo_WEAR_ZORA_e);
    addOption(dItemNo_ARMOR_e);
}

template <size_t Size>
void populate_shield_equip_picker(Pane& pane, const std::array<u8, Size>& entries) {
    pane.clear();
    const auto addOption = [&pane](u8 id) {
        pane.add_button({
                            .text = get_item_name(id),
                            .isSelected = [id] { return dComIfGs_getSelectEquipShield() == id; },
                        })
            .on_pressed([id] {
                mDoAud_seStartMenu(kSoundItemChange);
                dMeter2_applyEquippedShield(id);
            });
    };
    addOption(dItemNo_NONE_e);
    for (const auto item : entries) {
        addOption(item);
    }
}

template <size_t Size>
void populate_select_equip_picker(Pane& pane, u8& equip, const std::array<u8, Size>& entries) {
    pane.clear();
    const auto addOption = [&pane, &equip](u8 id) {
        pane.add_button({
                            .text = get_item_name(id),
                            .isSelected = [id, &equip] { return equip == id; },
                        })
            .on_pressed([id, &equip] {
                mDoAud_seStartMenu(kSoundItemChange);
                equip = id;
            });
    };
    addOption(dItemNo_NONE_e);
    for (const auto item : entries) {
        addOption(item);
    }
}

// ============================================
// MODIFIED CODE — ALBW Port
// Added "Colossal" (size 3, 50000 rupees) to the wallet size picker.
// ============================================
static const std::array<Rml::String, 4> walletSizeNames = {
    "Normal",
    "Big",
    "Giant",
    "Colossal",  // ALBW Port — Cave of Ordeals reward
};
// ============================================
// MODIFIED CODE ENDS HERE
// ============================================

void populate_wallet_picker(Pane& pane) {
    pane.clear();
    for (int i = 0; i < walletSizeNames.size(); ++i) {
        pane.add_button({
                            .text = walletSizeNames[i],
                            .isSelected = [i] { return get_player_status()->getWalletSize() == i; },
                        })
            .on_pressed([i] {
                mDoAud_seStartMenu(kSoundItemChange);
                get_player_status()->setWalletSize(i);
            });
    }
}

static const std::array<Rml::String, 2> formNames = {
    "Human",
    "Wolf",
};

void populate_form_picker(Pane& pane) {
    pane.clear();
    for (int i = 0; i < formNames.size(); ++i) {
        pane.add_button(
                {
                    .text = formNames[i],
                    .isSelected = [i] { return get_player_status()->getTransformStatus() == i; },
                })
            .on_pressed([i] {
                mDoAud_seStartMenu(kSoundItemChange);
                get_player_status()->setTransformStatus(i);
            });
    }
}

void add_toggle_button(Pane& pane, ToggleEntry entry) {
    auto isSelected = std::move(entry.isSelected);
    auto setSelected = std::move(entry.setSelected);
    pane.add_button({
                        .text = entry.text,
                        .isSelected = isSelected,
                    })
        .on_pressed([isSelected, setSelected] {
            mDoAud_seStartMenu(kSoundItemChange);
            setSelected(!isSelected());
        });
}

template <size_t Size>
std::vector<ToggleEntry> item_toggle_entries(const std::array<u8, Size>& entries) {
    std::vector<ToggleEntry> toggles;
    toggles.reserve(entries.size());
    for (const auto item : entries) {
        toggles.push_back({
            .text = get_item_name(item),
            .isSelected = [item] { return dComIfGs_isItemFirstBit(item); },
            .setSelected = [item](bool selected) { set_item_first_bit(item, selected); },
        });
    }
    return toggles;
}

template <size_t Size>
std::vector<ToggleEntry> event_toggle_entries(const std::array<NamedFlagEntry, Size>& entries) {
    std::vector<ToggleEntry> toggles;
    toggles.reserve(entries.size());
    for (const auto& [name, flag] : entries) {
        toggles.push_back({
            .text = name,
            .isSelected = [flag] { return dComIfGs_isEventBit(flag); },
            .setSelected = [flag](bool selected) { set_event_bit(flag, selected); },
        });
    }
    return toggles;
}

template <size_t Size>
std::vector<ToggleEntry> collect_crystal_toggle_entries(
    const std::array<NamedIndexEntry, Size>& entries) {
    std::vector<ToggleEntry> toggles;
    toggles.reserve(entries.size());
    for (const auto& [name, index] : entries) {
        toggles.push_back({
            .text = name,
            .isSelected = [index] { return dComIfGs_isCollectCrystal(index); },
            .setSelected =
                [index](bool selected) {
                    if (selected) {
                        dComIfGs_onCollectCrystal(index);
                    } else {
                        dComIfGs_offCollectCrystal(index);
                    }
                },
        });
    }
    return toggles;
}

template <size_t Size>
std::vector<ToggleEntry> collect_mirror_toggle_entries(
    const std::array<NamedIndexEntry, Size>& entries) {
    std::vector<ToggleEntry> toggles;
    toggles.reserve(entries.size());
    for (const auto& [name, index] : entries) {
        toggles.push_back({
            .text = name,
            .isSelected = [index] { return dComIfGs_isCollectMirror(index); },
            .setSelected =
                [index](bool selected) {
                    if (selected) {
                        dComIfGs_onCollectMirror(index);
                    } else {
                        dComIfGs_offCollectMirror(index);
                    }
                },
        });
    }
    return toggles;
}

void populate_collect_clothes_picker(Pane& pane) {
    populate_toggle_group(pane,
        {
            ToggleEntry{
                .text = "Ordon Clothes",
                .isSelected = [] { return dComIfGs_isItemFirstBit(dItemNo_WEAR_CASUAL_e); },
                .setSelected =
                    [](bool selected) { set_item_first_bit(dItemNo_WEAR_CASUAL_e, selected); },
            },
            ToggleEntry{
                .text = "Hero's Clothes",
                .isSelected = [] { return dComIfGs_isCollectClothes(KOKIRI_CLOTHES_FLAG); },
                .setSelected =
                    [](bool selected) {
                        if (selected) {
                            dComIfGs_setCollectClothes(KOKIRI_CLOTHES_FLAG);
                        } else {
                            dComIfGs_offCollectClothes(KOKIRI_CLOTHES_FLAG);
                        }
                    },
            },
            ToggleEntry{
                .text = "Zora Armor",
                .isSelected = [] { return dComIfGs_isItemFirstBit(dItemNo_WEAR_ZORA_e); },
                .setSelected =
                    [](bool selected) { set_item_first_bit(dItemNo_WEAR_ZORA_e, selected); },
            },
            ToggleEntry{
                .text = "Magic Armor",
                .isSelected = [] { return dComIfGs_isItemFirstBit(dItemNo_ARMOR_e); },
                .setSelected = [](bool selected) { set_item_first_bit(dItemNo_ARMOR_e, selected); },
            },
        });
}

void populate_poe_souls_picker(Pane& pane) {
    pane.clear();
    pane.add_section("Actions");
    pane.add_button("All 60").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        dComIfGs_setPohSpiritNum(60);
    });
    pane.add_button("Clear").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        dComIfGs_setPohSpiritNum(0);
    });

    pane.add_section("Value");
    pane.add_child<NumberButton>(NumberButton::Props{
        .key = "Collected",
        .getValue = [] { return dComIfGs_getPohSpiritNum(); },
        .setValue =
            [](int value) { dComIfGs_setPohSpiritNum(static_cast<u8>(std::clamp(value, 0, 60))); },
        .max = 60,
    });
}

void populate_max_life_picker(Pane& pane) {
    pane.clear();
    pane.add_section("Actions");
    pane.add_button("3 Hearts").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        dComIfGs_setMaxLife(15);
        dComIfGs_setLife(12);
    });
    pane.add_button("20 Hearts").on_pressed([] {
        mDoAud_seStartMenu(kSoundItemChange);
        dComIfGs_setMaxLife(100);
        dComIfGs_setLife(80);
    });

    pane.add_section("Value");
    pane.add_child<NumberButton>(NumberButton::Props{
        .key = "Max Life",
        .getValue = [] { return dComIfGs_getMaxLife(); },
        .setValue = [](int value) { set_max_life(value); },
        .min = 15,
        .max = 100,
    });
}

void populate_bug_species_picker(Pane& pane, const BugSpeciesEntry& bug) {
    pane.clear();
    pane.add_section("Owned");
    add_toggle_button(
        pane, {
                  .text = fmt::format("Male {}", bug.name),
                  .isSelected = [item = bug.maleItem] { return dComIfGs_isItemFirstBit(item); },
                  .setSelected = [item = bug.maleItem](
                                     bool selected) { set_item_first_bit(item, selected); },
              });
    add_toggle_button(
        pane, {
                  .text = fmt::format("Female {}", bug.name),
                  .isSelected = [item = bug.femaleItem] { return dComIfGs_isItemFirstBit(item); },
                  .setSelected = [item = bug.femaleItem](
                                     bool selected) { set_item_first_bit(item, selected); },
              });

    pane.add_section("Given to Agitha");
    add_toggle_button(
        pane, {
                  .text = fmt::format("Male {}", bug.name),
                  .isSelected = [flag = bug.maleTurnInFlag] { return dComIfGs_isEventBit(flag); },
                  .setSelected = [flag = bug.maleTurnInFlag](
                                     bool selected) { set_event_bit(flag, selected); },
              });
    add_toggle_button(
        pane, {
                  .text = fmt::format("Female {}", bug.name),
                  .isSelected = [flag = bug.femaleTurnInFlag] { return dComIfGs_isEventBit(flag); },
                  .setSelected = [flag = bug.femaleTurnInFlag](
                                     bool selected) { set_event_bit(flag, selected); },
              });
}

void populate_letters_picker(Pane& pane) {
    std::vector<ToggleEntry> toggles;
    toggles.reserve(letterSenders.size());
    for (int index = 0; index < letterSenders.size(); ++index) {
        toggles.push_back({
            .text = letterSenders[index],
            .isSelected = [index] { return dComIfGs_isLetterGetFlag(index); },
            .setSelected = [index](bool selected) { set_letter_get_flag(index, selected); },
        });
    }
    populate_toggle_group(pane, toggles);
}

void populate_fish_species_picker(Pane& pane, const FishSpeciesEntry& fish) {
    pane.clear();
    pane.add_section(fish.name);
    pane.add_child<NumberButton>(NumberButton::Props{
        .key = "Caught",
        .getValue = [index = fish.index] { return dComIfGs_getFishNum(index); },
        .setValue =
            [index = fish.index](int value) {
                get_player_fishing_info()->mFishCount[index] =
                    static_cast<u16>(std::clamp(value, 0, 999));
            },
        .max = 999,
    });
    pane.add_child<NumberButton>(NumberButton::Props{
        .key = "Biggest",
        .getValue = [index = fish.index] { return dComIfGs_getFishSize(index); },
        .setValue =
            [index = fish.index](int value) {
                dComIfGs_setFishSize(index, static_cast<u8>(std::clamp(value, 0, 255)));
            },
        .max = 255,
    });
}

Rml::String target_type_label() {
    const auto type = get_player_config()->getAttentionType();
    if (type >= targetTypeNames.size()) {
        return fmt::format("Unknown ({})", type);
    }
    return targetTypeNames[type];
}

Rml::String sound_mode_label() {
    const auto mode = get_player_config()->getSound();
    if (mode >= soundModeNames.size()) {
        return fmt::format("Unknown ({})", mode);
    }
    return soundModeNames[mode];
}

void populate_target_type_picker(Pane& pane) {
    pane.clear();
    for (u8 type = 0; type < targetTypeNames.size(); ++type) {
        pane
            .add_button({
                .text = targetTypeNames[type],
                .isSelected = [type] { return get_player_config()->getAttentionType() == type; },
            })
            .on_pressed([type] {
                mDoAud_seStartMenu(kSoundItemChange);
                get_player_config()->setAttentionType(type);
            });
    }
}

void populate_sound_mode_picker(Pane& pane) {
    pane.clear();
    for (u8 mode = 0; mode < soundModeNames.size(); ++mode) {
        pane.add_button(
                {
                    .text = soundModeNames[mode],
                    .isSelected = [mode] { return get_player_config()->getSound() == mode; },
                })
            .on_pressed([mode] {
                mDoAud_seStartMenu(kSoundItemChange);
                get_player_config()->setSound(mode);
            });
    }
}

constexpr float kDaytimeUnitsPerHour = 15.0f;

float daytime_from_clock(int hour, int minute) {
    hour = std::clamp(hour, 0, 23);
    minute = std::clamp(minute, 0, 59);
    return (hour * kDaytimeUnitsPerHour) + (minute / 60.0f * kDaytimeUnitsPerHour);
}

void set_clock_time(int hour, int minute) {
    if (auto* statusB = get_player_status_b()) {
        statusB->setTime(daytime_from_clock(hour, minute));
    }
}

static constexpr const char* kAlbwUnfinishedDisclaimer =
    "<br/><br/><b>Not fully tested!</b> Turning on these settings may crash, softlock, "
    "or otherwise break your saves. Continue with caution.";

void editor_bool_option(Pane& leftPane, Pane& rightPane, ConfigVar<bool>& var,
                        const Rml::String& key, const Rml::String& helpText,
                        std::function<bool()> isDisabled = {}) {
    leftPane.register_control(
        leftPane.add_child<BoolButton>(BoolButton::Props{
            .key = key,
            .getValue = [&var] { return var.getValue(); },
            .setValue =
                [&var](bool value) {
                    var.setValue(value);
                    config::Save();
                },
            .isDisabled = std::move(isDisabled),
            .isModified = [&var] { return var.getValue() != var.getDefaultValue(); },
        }),
        rightPane,
        [helpText](Pane& pane) {
            pane.clear();
            pane.add_rml(helpText);
        });
}

// WW itemmdl viewer item list (label -> BDL index, from itemmdl.h). Drives the viewer dropdown.
struct WwItemmdlViewerItem {
    const char* label;
    int index;
};
static const WwItemmdlViewerItem kWwItemmdlViewerItems[] = {
    {"Bow", 0xF},           {"Skull Hammer", 0x12}, {"Bomb", 0xC},          {"Boomerang", 0xD},
    {"Iron Boots", 0xE},    {"Picto Box", 0x10},    {"Tingle Tuner", 0x11}, {"Sail", 0x13},
    {"Hookshot", 0x14},     {"Deku Leaf", 0x15},    {"Magic Armor", 0x16},  {"Grappling Hook", 0x17},
    {"Wind Waker", 0x18},   {"Telescope", 0x19},    {"Spoils Bag", 0x5},    {"Bait Bag", 0x6},
    {"Delivery Bag", 0x7},  {"Bottle (blue)", 0x8}, {"Bottle (green)", 0x9},{"Bottle (red)", 0xA},
    {"Bottle (special)", 0xB},
};
static Rml::String wwItemmdlViewerLabel() {
    const int idx = getSettings().game.wwItemmdlViewerBdlIndex.getValue();
    for (const auto& e : kWwItemmdlViewerItems) {
        if (e.index == idx) {
            return Rml::String(e.label);
        }
    }
    return Rml::String("Bow");
}

}  // namespace

// Bug gate 2 — Stage tab deferred refresh + RAII alive (dual APPROVED).
// Raw Pane*/Button* are only safe while alive==true (flipped false before
// mContentComponents destroy on hide/clear_content/dtor).
struct EditorWindow::StageTabState : std::enable_shared_from_this<EditorWindow::StageTabState> {
    // G2: one expanded name-group at a time; rows capped (show more raises limit).
    static constexpr size_t kPerGroupCap = 150;

    Pane* left = nullptr;
    Pane* right = nullptr;
    Button* flyCamBtn = nullptr;
    Button* selectModeBtn = nullptr;
    Button* pcHotkeysBtn = nullptr;
    bool needsRefresh = false;
    bool alive = true;
    std::string expandedName;  // empty = all groups collapsed
    size_t expandedLimit = kPerGroupCap;
    std::string scrollToName;  // gate 5: ScrollIntoView this header after rebuild

    void invalidate(const char* reason) noexcept {
        DuskLog.info("StageTab invalidate reason={} alive={} left={} right={} fly={} select={} pc={} needs={}",
                     reason, alive, static_cast<const void*>(left),
                     static_cast<const void*>(right), static_cast<const void*>(flyCamBtn),
                     static_cast<const void*>(selectModeBtn),
                     static_cast<const void*>(pcHotkeysBtn), needsRefresh);
        alive = false;
        needsRefresh = false;
        expandedName.clear();
        expandedLimit = kPerGroupCap;
        scrollToName.clear();
        left = nullptr;
        right = nullptr;
        flyCamBtn = nullptr;
        selectModeBtn = nullptr;
        pcHotkeysBtn = nullptr;
        dusk::leveledit::set_selection_detail_handler({});
    }

    static void fill_selection_detail(Pane* right, const dusk::leveledit::PlacedActor& sel) {
        right->clear();
        right->add_section(sel.name);
        right->add_rml(dusk::leveledit::format_placed_actor_detail_rml(sel));
    }

    static const char* pc_hotkeys_label() {
        return dusk::leveledit::session_pc_hotkeys_enabled()
                   ? "PC Hotkeys: On (keyboard + mouse with controller)"
                   : "PC Hotkeys: Off (UI blocks keyboard)";
    }

    static const char* select_mode_label() {
        return dusk::leveledit::session_select_mode_enabled()
                   ? "Select Mode: On (V or click to disable)"
                   : "Select Mode: Off (V or click to enable)";
    }

    static const char* fly_cam_label() {
        return dusk::leveledit::session_fly_cam_enabled() ? "Fly Cam: On (click to disable)"
                                                          : "Fly Cam: Off (click to enable)";
    }

    void refresh() {
        if (!alive || left == nullptr || right == nullptr) {
            DuskLog.info("StageRefresh skipped alive={} left={} right={}", alive,
                         static_cast<const void*>(left), static_cast<const void*>(right));
            return;
        }
        DuskLog.info("StageRefresh begin left={} right={}", static_cast<const void*>(left),
                     static_cast<const void*>(right));
        left->clear();
        right->clear();
        flyCamBtn = nullptr;
        selectModeBtn = nullptr;
        pcHotkeysBtn = nullptr;

        auto self = shared_from_this();
        dusk::leveledit::set_selection_detail_handler([self](const dusk::leveledit::PlacedActor& sel) {
            if (!self->alive || self->right == nullptr) {
                return;
            }
            fill_selection_detail(self->right, sel);
        });

        auto result = dusk::leveledit::enumerate_room_actors();
        const char* srcLabel =
            result.source == dusk::leveledit::EnumSource::Buffer ? "buffer" : "live fallback";
        int joinedCount = 0;
        int unspawnedCount = 0;
        int spawnPointCount = 0;
        for (const auto& a : result.actors) {
            if (a.isSpawnPoint) {
                ++spawnPointCount;
            } else if (a.unspawned) {
                ++unspawnedCount;
            } else {
                ++joinedCount;
            }
        }

        left->add_section(fmt::format(
            "{} · {} placed · {} joined · {} unspawned · {} PLYR · "
            "{} live-in-room · room {}",
            srcLabel, result.actors.size(), joinedCount, unspawnedCount, spawnPointCount,
            result.liveActorsInRoom, dComIfGp_roomControl_getStayNo()));
        left->add_rml(fmt::format(
            "Resource: <b>{}</b><br/>"
            "List = authored placements. Kill an enemy → Refresh → that row becomes "
            "<b>unspawned</b> (join is live).",
            result.resourceKey));

        // Option B: defer rebuild — never clear() from this callback.
        left->add_button("Refresh").on_pressed([self] {
            if (!self->alive) {
                return;
            }
            DuskLog.info("StageRefresh queued");
            // Collapse groups on full re-enum (bounded teardown).
            self->expandedName.clear();
            self->expandedLimit = kPerGroupCap;
            self->needsRefresh = true;
        });
        left->add_button("Clear selection").on_pressed([self] {
            dusk::leveledit::clear_selection();
            if (self->alive && self->right != nullptr) {
                self->right->clear();
            }
            DuskLog.info("StageSelection cleared");
        });

        // Option A: toggle latch + in-place label; no refresh().
        auto& flyBtn = left->add_button(fly_cam_label());
        flyCamBtn = &flyBtn;
        flyBtn.on_pressed([self] {
            if (!self->alive || self->flyCamBtn == nullptr) {
                return;
            }
            dusk::leveledit::enable_session_fly_cam(!dusk::leveledit::session_fly_cam_enabled());
            DuskLog.info("StageFlyCam toggle enabled={} btn={}",
                         dusk::leveledit::session_fly_cam_enabled(),
                         static_cast<const void*>(self->flyCamBtn));
            self->flyCamBtn->set_text(fly_cam_label());
        });

        auto& selectBtn = left->add_button(select_mode_label());
        selectModeBtn = &selectBtn;
        selectBtn.on_pressed([self] {
            if (!self->alive || self->selectModeBtn == nullptr) {
                return;
            }
            if (!dusk::leveledit::editor_fly_cam_active()) {
                DuskLog.info("StageSelectMode ignored — fly cam inactive");
                return;
            }
            dusk::leveledit::enable_session_select_mode(
                !dusk::leveledit::session_select_mode_enabled());
            DuskLog.info("StageSelectMode toggle enabled={} btn={}",
                         dusk::leveledit::session_select_mode_enabled(),
                         static_cast<const void*>(self->selectModeBtn));
            self->selectModeBtn->set_text(select_mode_label());
        });

        auto& pcBtn = left->add_button(pc_hotkeys_label());
        pcHotkeysBtn = &pcBtn;
        pcBtn.on_pressed([self] {
            if (!self->alive || self->pcHotkeysBtn == nullptr) {
                return;
            }
            dusk::leveledit::enable_session_pc_hotkeys(
                !dusk::leveledit::session_pc_hotkeys_enabled());
            DuskLog.info("StagePcHotkeys toggle enabled={} btn={}",
                         dusk::leveledit::session_pc_hotkeys_enabled(),
                         static_cast<const void*>(self->pcHotkeysBtn));
            self->pcHotkeysBtn->set_text(pc_hotkeys_label());
            dusk::ui::input::sync_input_block();
        });

        auto shared = std::make_shared<dusk::leveledit::EnumerateResult>(std::move(result));
        const size_t total = shared->actors.size();

        // G2: group by actor name (proc as secondary label). Collapsed headers only
        // until expand — expand/collapse queues deferred refresh (gate-3).
        std::map<std::string, std::vector<size_t>> groups;
        for (size_t i = 0; i < shared->actors.size(); ++i) {
            groups[shared->actors[i].name].push_back(i);
        }

        left->add_rml(fmt::format(
            "<i>{} name groups · expand one group (max {} rows + Show more). "
            "chunkTag / layer on each row.</i>",
            groups.size(), kPerGroupCap));

        size_t rowButtons = 0;
        Button* scrollTarget = nullptr;
        Button* firstRowUnderScroll = nullptr;
        for (const auto& [name, indices] : groups) {
            const bool expanded = (expandedName == name);
            s16 procSample = -1;
            if (!indices.empty() && indices.front() < shared->actors.size()) {
                procSample = shared->actors[indices.front()].procname;
            }
            const Rml::String header = fmt::format(
                "{} {} ({}) · proc {}", expanded ? "[-]" : "[+]", name, indices.size(), procSample);
            auto& headerBtn = left->add_button(header);
            if (!scrollToName.empty() && scrollToName == name) {
                scrollTarget = &headerBtn;
            }
            headerBtn.on_pressed([self, name] {
                if (!self->alive) {
                    return;
                }
                // Always scroll back to this header after rebuild (expand or collapse).
                self->scrollToName = name;
                if (self->expandedName == name) {
                    self->expandedName.clear();
                    self->expandedLimit = kPerGroupCap;
                } else {
                    self->expandedName = name;
                    self->expandedLimit = kPerGroupCap;
                }
                DuskLog.info("StageGroupExpand queued name={} open={}", name,
                             !self->expandedName.empty());
                self->needsRefresh = true;
            });

            if (!expanded) {
                continue;
            }

            const size_t shown = std::min(indices.size(), expandedLimit);
            for (size_t j = 0; j < shown; ++j) {
                const size_t i = indices[j];
                const auto& a = shared->actors[i];
                const char* status = a.isSpawnPoint ? " · spawn"
                                    : a.unspawned   ? " · unspawned"
                                                    : " · live";
                const Rml::String label = fmt::format("{} · #{} · L{}{}", a.chunkTag, a.setID,
                                                      a.layer, status);
                ++rowButtons;
                auto& rowBtn = left->add_button(label);
                if (scrollTarget != nullptr && firstRowUnderScroll == nullptr && j == 0 &&
                    scrollToName == name) {
                    firstRowUnderScroll = &rowBtn;
                }
                rowBtn.on_pressed([self, shared, i] {
                    if (!self->alive || self->right == nullptr) {
                        return;
                    }
                    if (i >= shared->actors.size()) {
                        return;
                    }
                    const auto& sel = shared->actors[i];
                    dusk::leveledit::set_selected_index(static_cast<int>(i));
                    dusk::leveledit::set_selection_snapshot(sel, true);
                });
            }
            if (indices.size() > shown) {
                left->add_button(fmt::format("Show more ({} remaining in {})",
                                             indices.size() - shown, name))
                    .on_pressed([self, name] {
                        if (!self->alive) {
                            return;
                        }
                        self->scrollToName = name;
                        self->expandedLimit += kPerGroupCap;
                        DuskLog.info("StageGroupShowMore queued limit={}", self->expandedLimit);
                        self->needsRefresh = true;
                    });
            }
        }

        // Gate 5 S1/S3: after rebuild, land on toggled header (+ first row if expanded).
        if (scrollTarget != nullptr && scrollTarget->root() != nullptr) {
            scrollTarget->root()->ScrollIntoView(Rml::ScrollIntoViewOptions{
                Rml::ScrollAlignment::Start,
                Rml::ScrollAlignment::Nearest,
                Rml::ScrollBehavior::Instant,
                Rml::ScrollParentage::Closest,
            });
            if (firstRowUnderScroll != nullptr && firstRowUnderScroll->root() != nullptr) {
                firstRowUnderScroll->root()->ScrollIntoView(Rml::ScrollIntoViewOptions{
                    Rml::ScrollAlignment::Nearest,
                    Rml::ScrollAlignment::Nearest,
                    Rml::ScrollBehavior::Instant,
                    Rml::ScrollParentage::Closest,
                });
            }
            DuskLog.info("StageScrollIntoView name={}", scrollToName.c_str());
        }
        scrollToName.clear();

        DuskLog.info("StageRefresh end actors={} groups={} expanded={} rowButtons={}", total,
                     groups.size(), expandedName.empty() ? "(none)" : expandedName.c_str(),
                     rowButtons);
    }

    void drain_deferred_refresh() {
        // No per-frame log — only breadcrumb when a deferred rebuild actually runs.
        if (!alive || !needsRefresh) {
            return;
        }
        DuskLog.info("StageTick drain alive={} left={} right={}", alive,
                     static_cast<const void*>(left), static_cast<const void*>(right));
        needsRefresh = false;
        if (left == nullptr || right == nullptr) {
            return;
        }
        refresh();
    }
};

void EditorWindow::teardown_stage_tab(const char* reason) noexcept {
    DuskLog.info("EditorStageTeardown reason={} hasTick={} hasState={}", reason,
                 static_cast<bool>(mStageTabTick), static_cast<bool>(mStageTabState));
    mStageTabTick = nullptr;
    if (mStageTabState) {
        mStageTabState->invalidate(reason);
        mStageTabState.reset();
    }
    // Keep selection for in-world highlight after Editor/Stage closes (1a/1b).
    // Drop raw live ptr only — process ID stays so draw can rebind (Gate 8d).
    dusk::leveledit::set_selection_detail_handler({});
    dusk::leveledit::detach_selection_live();
}

void EditorWindow::clear_content_now(const char* reason) noexcept {
    teardown_stage_tab(reason);
    const size_t n = mContentComponents.size();
    Window::clear_content();
    DuskLog.info("StageContentCleared reason={} components={}", reason, n);
}

void EditorWindow::drain_deferred_ui() {
    // Order: clear/rebuild first (leave Stage), then hide — all outside Rml dispatch.
    if (mPendingClearOnly) {
        mPendingClearOnly = false;
        clear_content_now("deferred_clear");
    }
    if (mPendingTabBuilder) {
        auto builder = std::move(*mPendingTabBuilder);
        mPendingTabBuilder.reset();
        clear_content_now("deferred_tab_replace");
        if (builder) {
            builder(mContentRoot);
        }
        DuskLog.info("StageContentRebuilt deferred");
    }
    if (mPendingHide) {
        const bool close = mPendingHideClose;
        mPendingHide = false;
        mPendingHideClose = false;
        DuskLog.info("EditorHide deferred close={} visible={}", close, visible());
        // Gate 4: clear Stage widget list BEFORE Window::hide / document teardown.
        clear_content_now(close ? "hide_clear_close" : "hide_clear");
        Window::hide(close);
    }
}

void EditorWindow::update() {
    drain_deferred_ui();
    if (mStageTabTick) {
        mStageTabTick();
    }
    Window::update();
}

void EditorWindow::hide(bool close) {
    // Defer hide so Transitionend / close click is not mid-destroy of Stage list.
    DuskLog.info("EditorHide queue close={} visible={}", close, visible());
    mPendingHide = true;
    mPendingHideClose = close;
}

void EditorWindow::clear_content() noexcept {
    // Called from tab switch path only via replace_content now; keep sync path
    // for any direct callers — still tear down Stage state immediately.
    clear_content_now("clear_content_sync");
}

void EditorWindow::replace_content(TabBuilder builder) {
    // Gate 3 H-B: never destroy hundreds of Stage buttons inside tab Click.
    DuskLog.info("EditorReplaceContent queue");
    mPendingClearOnly = false;
    mPendingTabBuilder = std::move(builder);
}

EditorWindow::~EditorWindow() {
    // Flush any pending work without going through Rml; then hard teardown.
    mPendingTabBuilder.reset();
    mPendingClearOnly = false;
    mPendingHide = false;
    teardown_stage_tab("dtor");
}

EditorWindow::EditorWindow() {
    add_tab("Player Status", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Player");
        leftPane.register_control(leftPane.add_child<StringButton>(StringButton::Props{
                                      .key = "Player Name",
                                      .getValue = get_player_name,
                                      .setValue = set_player_name,
                                      .maxLength = 16,
                                  }),
            rightPane, {});
        leftPane.register_control(leftPane.add_child<StringButton>(StringButton::Props{
                                      .key = "Horse Name",
                                      .getValue = get_horse_name,
                                      .setValue = set_horse_name,
                                      .maxLength = 16,
                                  }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Max Health",
                .getValue = [] { return get_player_status()->getMaxLife(); },
                .setValue = [](int value) { return get_player_status()->setMaxLife(value); },
                .max = UINT16_MAX,  // TODO: actual max
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Health",
                .getValue = [] { return get_player_status()->getLife(); },
                .setValue = [](int value) { return get_player_status()->setLife(value); },
                .max = UINT16_MAX,  // TODO: actual max
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Rupees",
                .getValue = [] { return get_player_status()->getRupee(); },
                .setValue = [](int value) { return get_player_status()->setRupee(value); },
                .max = get_player_status()->getRupeeMax(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Max Oil",
                .getValue = [] { return get_player_status()->getMaxOil(); },
                .setValue = [](int value) { return get_player_status()->setMaxOil(value); },
                .max = UINT16_MAX,  // TODO: actual max
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Oil",
                .getValue = [] { return get_player_status()->getOil(); },
                .setValue = [](int value) { return get_player_status()->setOil(value); },
                .max = UINT16_MAX,  // TODO: actual max
            }),
            rightPane, {});

        leftPane.add_section("Equipment");
        const auto genSelectItemComboBox = [&leftPane, &rightPane](
                                               const Rml::String& label, u8& selectItemData) {
            leftPane.register_control(
                leftPane.add_select_button({
                    .key = label,
                    .getValue = [&selectItemData] { return item_label_for_slot(selectItemData); },
                }),
                rightPane, [&selectItemData](Pane& pane) {
                    populate_select_item_picker(pane, selectItemData);
                });
        };
        genSelectItemComboBox("Equip X", get_player_status()->mSelectItem[0]);
        genSelectItemComboBox("Equip Y", get_player_status()->mSelectItem[1]);
        genSelectItemComboBox("Combo Equip X", get_player_status()->mMixItem[0]);
        genSelectItemComboBox("Combo Equip Y", get_player_status()->mMixItem[1]);

        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Clothes",
                .getValue = [] { return get_item_name(get_player_status()->mSelectEquip[0]); },
            }),
            rightPane, [](Pane& pane) { populate_select_clothes_picker(pane); });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Sword",
                .getValue = [] { return get_item_name(get_player_status()->mSelectEquip[1]); },
            }),
            rightPane, [](Pane& pane) {
                populate_select_equip_picker(
                    pane, get_player_status()->mSelectEquip[1], swordEntries);
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Shield",
                .getValue = [] { return get_item_name(get_player_status()->mSelectEquip[2]); },
            }),
            rightPane, [](Pane& pane) {
                populate_shield_equip_picker(pane, shieldEntries);
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Scent",
                .getValue = [] { return get_item_name(get_player_status()->mSelectEquip[3]); },
            }),
            rightPane, [](Pane& pane) {
                populate_select_equip_picker(
                    pane, get_player_status()->mSelectEquip[3], smellEntries);
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Wallet Size",
                .getValue = [] { return walletSizeNames[get_player_status()->getWalletSize()]; },
            }),
            rightPane, [](Pane& pane) { populate_wallet_picker(pane); });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Form",
                .getValue = [] { return formNames[get_player_status()->getTransformStatus()]; },
            }),
            rightPane, [](Pane& pane) { populate_form_picker(pane); });
        add_toggle_button(leftPane,
                          ToggleEntry{
                              .text = "End-Game Transform (Midna + Crystal)",
                              .isSelected =
                                  [] {
                                      return dComIfGs_isEventBit(dSv_event_flag_c::M_077) &&
                                             dComIfGs_isTransformLV(3) &&
                                             dComIfGs_isEventBit(dSv_event_flag_c::F_0250);
                                  },
                              .setSelected =
                                  [](bool selected) {
                                      set_event_bit(dSv_event_flag_c::M_077, selected);
                                      set_event_bit(dSv_event_flag_c::F_0250, selected);
                                      auto* statusB = get_player_status_b();
                                      for (int i = 0; i <= 3; ++i) {
                                          if (selected) {
                                              statusB->onTransformLV(i);
                                          } else {
                                              statusB->offTransformLV(i);
                                          }
                                      }
                                  },
                          });
        add_toggle_button(leftPane,
                          ToggleEntry{
                              .text = "Dominion Rod Restored (Shad)",
                              .isSelected =
                                  [] {
                                      return dComIfGs_isEventBit(dSv_event_flag_c::F_0302);
                                  },
                              .setSelected =
                                  [](bool selected) {
                                      set_event_bit(dSv_event_flag_c::F_0302, selected);
                                  },
                          });

        leftPane.add_section("World");
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Day",
                .getValue = [] { return get_player_status_b()->getDate(); },
                .setValue =
                    [](int value) { get_player_status_b()->setDate(static_cast<u16>(value)); },
                .max = UINT16_MAX,
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Hour",
                .getValue = [] { return dKy_getdaytime_hour(); },
                .setValue = [](int value) { set_clock_time(value, dKy_getdaytime_minute()); },
                .max = 23,
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Minute",
                .getValue = [] { return dKy_getdaytime_minute(); },
                .setValue = [](int value) { set_clock_time(dKy_getdaytime_hour(), value); },
                .max = 59,
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Transform Level",
                .getValue =
                    [] {
                        return std::popcount(static_cast<unsigned>(
                            get_player_status_b()->mTransformLevelFlag & 0xF));
                    },
                .setValue =
                    [](int value) {
                        get_player_status_b()->mTransformLevelFlag =
                            static_cast<u8>((1u << value) - 1u);
                    },
                .max = 4,
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Twilight Clear Level",
                .getValue =
                    [] {
                        return std::popcount(static_cast<unsigned>(
                            get_player_status_b()->mDarkClearLevelFlag & 0x7));
                    },
                .setValue =
                    [](int value) {
                        get_player_status_b()->mDarkClearLevelFlag =
                            static_cast<u8>((1u << value) - 1u);
                    },
                .max = 3,
            }),
            rightPane, {});
    });

    add_tab("Location", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Save Location");
        leftPane
            .register_control(leftPane.add_select_button({
                                  .key = "Stage",
                                  .getValue =
                                      [] {
                                          return stage_label_for_file(
                                              fixed_string(get_player_return_place()->mName));
                                      },
                              }),
                rightPane,
                [](Pane& pane) {
                    populate_stage_picker(
                        pane, [] { return fixed_string(get_player_return_place()->mName); },
                        [](const char* stageFile) {
                            set_fixed_string(
                                get_player_return_place()->mName, Rml::String(stageFile));
                        });
                })
            .set_disabled(true);
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Room",
                .getValue = [] { return get_player_return_place()->mRoomNo; },
                .setValue =
                    [](int value) { get_player_return_place()->mRoomNo = static_cast<s8>(value); },
                .min = std::numeric_limits<s8>::min(),
                .max = std::numeric_limits<s8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Spawn ID",
                .getValue = [] { return get_player_return_place()->mPlayerStatus; },
                .setValue =
                    [](int value) {
                        get_player_return_place()->mPlayerStatus = static_cast<u8>(value);
                    },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});

        leftPane.add_section("Horse Location");
        leftPane.register_control(leftPane.add_child<StringButton>(StringButton::Props{
                                      .key = "Horse Position",
                                      .getValue =
                                          [] {
                                              const auto* horsePlace = get_horse_place();
                                              return fmt::format("{}, {}, {}",
                                                  static_cast<float>(horsePlace->mPos.x),
                                                  static_cast<float>(horsePlace->mPos.y),
                                                  static_cast<float>(horsePlace->mPos.z));
                                          },
                                      .setValue =
                                          [](Rml::String value) {
                                              float x = 0.0f;
                                              float y = 0.0f;
                                              float z = 0.0f;
                                              if (parse_vec3(value, x, y, z)) {
                                                  auto* horsePlace = get_horse_place();
                                                  horsePlace->mPos.x = x;
                                                  horsePlace->mPos.y = y;
                                                  horsePlace->mPos.z = z;
                                              }
                                          },
                                  }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Horse Angle",
                .getValue = [] { return get_horse_place()->mAngleY; },
                .setValue = [](int value) { get_horse_place()->mAngleY = static_cast<s16>(value); },
                .min = std::numeric_limits<s16>::min(),
                .max = std::numeric_limits<s16>::max(),
            }),
            rightPane, {});
        leftPane
            .register_control(
                leftPane.add_select_button({
                    .key = "Horse Stage",
                    .getValue =
                        [] { return stage_label_for_file(fixed_string(get_horse_place()->mName)); },
                }),
                rightPane,
                [](Pane& pane) {
                    populate_stage_picker(
                        pane, [] { return fixed_string(get_horse_place()->mName); },
                        [](const char* stageFile) {
                            set_fixed_string(get_horse_place()->mName, Rml::String(stageFile));
                        });
                })
            .set_disabled(true);
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Horse Room",
                .getValue = [] { return get_horse_place()->mRoomNo; },
                .setValue = [](int value) { get_horse_place()->mRoomNo = static_cast<s8>(value); },
                .min = std::numeric_limits<s8>::min(),
                .max = std::numeric_limits<s8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Horse Spawn ID",
                .getValue = [] { return get_horse_place()->mSpawnId; },
                .setValue = [](int value) { get_horse_place()->mSpawnId = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
    });

    add_tab("Inventory", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Item Wheel");
        leftPane.register_control(leftPane.add_button("Default All").on_pressed([&rightPane] {
            mDoAud_seStartMenu(kSoundItemChange);
            for (int slot = 0; slot < 24; ++slot) {
                dComIfGs_setItem(slot, get_slot_default(slot));
            }
            // Bomb bag ammo is separate from the item IDs — fill if empty so bombs are usable.
            for (int bag = 0; bag < 3; ++bag) {
                if (dComIfGs_getBombNum(bag) == 0) {
                    const u8 bombItem = dComIfGs_getItem(static_cast<u8>(bag + SLOT_15), false);
                    u8 maxNum = dComIfGs_getBombMax(bombItem);
                    dComIfGs_setBombNum(bag, maxNum > 0 ? maxNum : 30);
                }
            }
#if TARGET_PC
            dAlbwPotion_applyDefaultInventorySlot11();
#endif
            rightPane.clear();
        }),
            rightPane, {});
        leftPane.register_control(leftPane.add_button("Clear All").on_pressed([&rightPane] {
            mDoAud_seStartMenu(kSoundItemChange);
            for (int slot = 0; slot < 24; ++slot) {
                dComIfGs_setItem(slot, dItemNo_NONE_e);
            }
            rightPane.clear();
        }),
            rightPane, {});
        for (int slot = 0; slot < 24; ++slot) {
            leftPane.register_control(
                leftPane.add_select_button({
                    .key = fmt::format("Slot {0:02d}", slot),
                    .getValue = [slot] { return get_item_name(get_player_item()->mItems[slot]); },
                }),
                rightPane, [slot](Pane& pane) { populate_item_slot_picker(pane, slot); });
        }

        leftPane.add_section("Amounts");
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Arrows Amount",
                .getValue = [] { return get_player_item_record()->mArrowNum; },
                .setValue =
                    [](int value) { get_player_item_record()->mArrowNum = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Slingshot Amount",
                .getValue = [] { return get_player_item_record()->mPachinkoNum; },
                .setValue =
                    [](int value) {
                        get_player_item_record()->mPachinkoNum = static_cast<u8>(value);
                    },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
        for (int bag = 0; bag < 3; ++bag) {
            leftPane.register_control(
                leftPane.add_child<NumberButton>(NumberButton::Props{
                    .key = fmt::format("Bomb Bag {} Amount", bag + 1),
                    .getValue = [bag] { return get_player_item_record()->mBombNum[bag]; },
                    .setValue =
                        [bag](int value) {
                            get_player_item_record()->mBombNum[bag] = static_cast<u8>(value);
                        },
                    .max = std::numeric_limits<u8>::max(),
                }),
                rightPane, {});
        }
        for (int bottle = 0; bottle < 4; ++bottle) {
            leftPane.register_control(
                leftPane.add_child<NumberButton>(NumberButton::Props{
                    .key = fmt::format("Bottle {} Amount", bottle + 1),
                    .getValue = [bottle] { return get_player_item_record()->mBottleNum[bottle]; },
                    .setValue =
                        [bottle](int value) {
                            get_player_item_record()->mBottleNum[bottle] = static_cast<u8>(value);
                        },
                    .max = std::numeric_limits<u8>::max(),
                }),
                rightPane, {});
        }

        leftPane.add_section("Capacities");
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Arrows Max",
                .getValue = [] { return get_player_item_max()->mItemMax[0]; },
                .setValue =
                    [](int value) { get_player_item_max()->mItemMax[0] = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Normal Bombs Max",
                .getValue = [] { return get_player_item_max()->mItemMax[1]; },
                .setValue =
                    [](int value) { get_player_item_max()->mItemMax[1] = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Water Bombs Max",
                .getValue = [] { return get_player_item_max()->mItemMax[2]; },
                .setValue =
                    [](int value) { get_player_item_max()->mItemMax[2] = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Bomblings Max",
                .getValue = [] { return get_player_item_max()->mItemMax[3]; },
                .setValue =
                    [](int value) { get_player_item_max()->mItemMax[3] = static_cast<u8>(value); },
                .max = std::numeric_limits<u8>::max(),
            }),
            rightPane, {});

        leftPane.add_section("Flags");
        leftPane.register_control(leftPane.add_select_button({
                                      .key = "Obtained Items",
                                      .getValue = [] { return "Edit"; },
                                  }),
            rightPane, [](Pane& pane) { populate_item_flag_picker(pane); });
    });
    add_tab("Collection", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Equipment");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Swords",
                .getValue =
                    [] {
                        return count_label(
                            count_item_first_bits(swordEntries), swordEntries.size());
                    },
            }),
            rightPane,
            [](Pane& pane) { populate_toggle_group(pane, item_toggle_entries(swordEntries)); });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Shields",
                .getValue =
                    [] {
                        return count_label(
                            count_item_first_bits(shieldEntries), shieldEntries.size());
                    },
            }),
            rightPane,
            [](Pane& pane) { populate_toggle_group(pane, item_toggle_entries(shieldEntries)); });
        leftPane.register_control(leftPane.add_select_button({
                                      .key = "Clothing",
                                      .getValue = [] { return count_label(count_clothing(), 4); },
                                  }),
            rightPane, [](Pane& pane) { populate_collect_clothes_picker(pane); });

        leftPane.add_section("Key Items");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Fused Shadows",
                .getValue =
                    [] {
                        return count_label(
                            count_collect_crystals(fusedShadowEntries), fusedShadowEntries.size());
                    },
            }),
            rightPane, [](Pane& pane) {
                populate_toggle_group(pane, collect_crystal_toggle_entries(fusedShadowEntries));
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Mirror Shards",
                .getValue =
                    [] {
                        return count_label(
                            count_collect_mirrors(mirrorShardEntries), mirrorShardEntries.size());
                    },
            }),
            rightPane, [](Pane& pane) {
                populate_toggle_group(pane, collect_mirror_toggle_entries(mirrorShardEntries));
            });

        leftPane.add_section("Health & Souls");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Poe Souls",
                .getValue = [] { return fmt::format("{} / 60", dComIfGs_getPohSpiritNum()); },
            }),
            rightPane, [](Pane& pane) { populate_poe_souls_picker(pane); });
        leftPane.register_control(leftPane.add_select_button({
                                      .key = "Max Life",
                                      .getValue = [] { return max_life_label(); },
                                  }),
            rightPane, [](Pane& pane) { populate_max_life_picker(pane); });

        leftPane.add_section("Golden Bugs");
        for (const auto& bug : bugSpeciesEntries) {
            leftPane.register_control(leftPane.add_select_button({
                                          .key = bug.name,
                                          .getValue = [bug] { return bug_species_label(bug); },
                                      }),
                rightPane, [bug](Pane& pane) { populate_bug_species_picker(pane, bug); });
        }

        leftPane.add_section("Skills");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Hidden Skills",
                .getValue =
                    [] {
                        return count_label(
                            count_event_bits(hiddenSkillEntries), hiddenSkillEntries.size());
                    },
            }),
            rightPane, [](Pane& pane) {
                populate_toggle_group(pane, event_toggle_entries(hiddenSkillEntries));
            });

        leftPane.add_section("Logs");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Postman Letters",
                .getValue = [] { return count_label(count_letters(), letterSenders.size()); },
            }),
            rightPane, [](Pane& pane) { populate_letters_picker(pane); });

        leftPane.add_section("Fishing Log");
        for (const auto& fish : fishSpeciesEntries) {
            leftPane.register_control(leftPane.add_select_button({
                                          .key = fish.name,
                                          .getValue = [fish] { return fish_species_label(fish); },
                                      }),
                rightPane, [fish](Pane& pane) { populate_fish_species_picker(pane, fish); });
        }
    });

    //add_tab("Flags", [this](Rml::Element* content) {
    //    // TODO
    //});

    add_tab("Minigame", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Records");
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "STAR Game Time (ms)",
                .getValue =
                    [] {
                        return static_cast<int>(std::min<u32>(
                            get_minigame()->getHookGameTime(), std::numeric_limits<int>::max()));
                    },
                .setValue =
                    [](int value) {
                        get_minigame()->setHookGameTime(static_cast<u32>(std::max(0, value)));
                    },
                .max = std::numeric_limits<int>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Snowboard Race Time (ms)",
                .getValue =
                    [] {
                        return static_cast<int>(std::min<u32>(
                            get_minigame()->getRaceGameTime(), std::numeric_limits<int>::max()));
                    },
                .setValue =
                    [](int value) {
                        get_minigame()->setRaceGameTime(static_cast<u32>(std::max(0, value)));
                    },
                .max = std::numeric_limits<int>::max(),
            }),
            rightPane, {});
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Fruit-Pop-Flight Score",
                .getValue =
                    [] {
                        return static_cast<int>(std::min<u32>(
                            get_minigame()->getBalloonScore(), std::numeric_limits<int>::max()));
                    },
                .setValue =
                    [](int value) {
                        get_minigame()->setBalloonScore(static_cast<u32>(std::max(0, value)));
                    },
                .max = std::numeric_limits<int>::max(),
            }),
            rightPane, {});
    });

    add_tab("Config", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_section("Options");
        leftPane.register_control(
            leftPane.add_child<BoolButton>(BoolButton::Props{
                .key = "Enable Vibration",
                .getValue = [] { return get_player_config()->getVibration() != 0; },
                .setValue = [](bool value) { get_player_config()->setVibration(value); },
            }),
            rightPane, {});
        leftPane.register_control(leftPane.add_select_button({
                                      .key = "Target Type",
                                      .getValue = [] { return target_type_label(); },
                                  }),
            rightPane, [](Pane& pane) { populate_target_type_picker(pane); });
        leftPane.register_control(leftPane.add_select_button({
                                      .key = "Sound",
                                      .getValue = [] { return sound_mode_label(); },
                                  }),
            rightPane, [](Pane& pane) { populate_sound_mode_picker(pane); });
    });

    add_tab("ALBW", [this](Rml::Element* content) {
        auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
        auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

        leftPane.add_text(
            "Experimental ALBW settings. Enable the editor from the main menu to access this "
            "tab.");
        editor_bool_option(leftPane, rightPane, getSettings().game.flurryRush, "FlurryTest",
            "Enables Flurry Rush (perfect-dodge slow-mo melee + Back Slice aerial-bow finisher). "
            "Requires Focused Arts." +
                Rml::String(kAlbwUnfinishedDisclaimer),
            [] { return !getSettings().game.focusedArts.getValue(); });
        editor_bool_option(leftPane, rightPane, getSettings().game.wolfArtsDevTest,
            "Wolf Arts Dev Test",
            "DEV/TEST: bypass the wolf-art shop unlocks AND the wolf-charge cost so the Wolf arts "
            "(howl / punch / giant) fire immediately. Needs Wolf Link Combat + quick-swap on; wolf "
            "form; D-pad Up = Howl. Keep OFF for normal play." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.wolfHowlVfxOverride,
            "Wolf Howl VFX: Apply Tuner",
            "OFF = the built-in default ring look (rotation on, no extra tilt, 1.0x). ON = apply the "
            "Tilt / Width / Height / Sweep Rate / Orbit / Period sliders below. Flip OFF to compare "
            "against the default, ON to tune." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Tilt (deg)",
                .getValue = [] { return getSettings().game.wolfHowlTiltDeg.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlTiltDeg.setValue(std::clamp(value, 0, 360));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlTiltDeg.getValue() !=
                               getSettings().game.wolfHowlTiltDeg.getDefaultValue();
                    },
                .min = 0,
                .max = 360,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Wolf Howl ring X-pitch in degrees (KAITENGIRIL). Full 0-360 sweep — KAITENGIRIL "
                    "sits at a natural angle, so sweep past 90/270 to find the value where it lies "
                    "completely horizontal. Live-tunable during a howl." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Roll (deg)",
                .getValue = [] { return getSettings().game.wolfHowlRollDeg.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlRollDeg.setValue(std::clamp(value, 0, 360));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlRollDeg.getValue() !=
                               getSettings().game.wolfHowlRollDeg.getDefaultValue();
                    },
                .min = 0,
                .max = 360,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Wolf Howl ring Z-roll in degrees (KAITENGIRIL). The second leveling axis — pair "
                    "with Tilt to cancel KAITENGIRIL's natural angle on both axes for a fully "
                    "horizontal ring. Full 0-360 sweep, live-tunable during a howl." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Width %",
                .getValue = [] { return getSettings().game.wolfHowlWidthPct.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlWidthPct.setValue(std::clamp(value, 10, 400));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlWidthPct.getValue() !=
                               getSettings().game.wolfHowlWidthPct.getDefaultValue();
                    },
                .min = 10,
                .max = 400,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Wolf Howl ring particle scale on X/Z, the horizontal spread (100 = 1.0x). "
                    "Live-tunable during a howl." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Height %",
                .getValue = [] { return getSettings().game.wolfHowlHeightPct.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlHeightPct.setValue(std::clamp(value, 10, 400));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlHeightPct.getValue() !=
                               getSettings().game.wolfHowlHeightPct.getDefaultValue();
                    },
                .min = 10,
                .max = 400,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Wolf Howl ring particle scale on Y, the vertical size (100 = 1.0x). Live-tunable "
                    "during a howl." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Sweep Rate",
                .getValue = [] { return getSettings().game.wolfHowlSweepRate.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlSweepRate.setValue(std::clamp(value, 0, 4000));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlSweepRate.getValue() !=
                               getSettings().game.wolfHowlSweepRate.getDefaultValue();
                    },
                .min = 0,
                .max = 4000,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "How fast the ring sweeps around the wolf (emit-yaw wind per frame). 0 = no "
                    "rotation; ~500 = one revolution every ~2 s; higher = faster. The wolf never "
                    "turns — only the effect." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Orbit",
                .getValue = [] { return getSettings().game.wolfHowlOrbit.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlOrbit.setValue(std::clamp(value, 0, 300));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlOrbit.getValue() !=
                               getSettings().game.wolfHowlOrbit.getDefaultValue();
                    },
                .min = 0,
                .max = 300,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "How far off-center the sweep swings (emission-point orbit radius). 0 = centered "
                    "on the wolf; higher = a wider circling ring. Helps the rotation read on the "
                    "near-symmetric spray." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Wolf Howl Period",
                .getValue = [] { return getSettings().game.wolfHowlPeriod.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wolfHowlPeriod.setValue(std::clamp(value, 1, 60));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wolfHowlPeriod.getValue() !=
                               getSettings().game.wolfHowlPeriod.getDefaultValue();
                    },
                .min = 1,
                .max = 60,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Re-emit cadence in frames — burst density. Lower = denser / smoother sweep "
                    "(1 = every frame); higher = sparser." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        // ============================================
        // NEW CODE — ALBW Port (Wolf Howl tune audition)
        // Preview the candidate howl tunes: select one to play it (cutting off the current), so we
        // can confirm which actually play (some Z2BGM_HOWL_* ids are silent) and which sound like
        // the Hero's-Shade / wolf duet.  HOWL / DUO (sing-with-the-wolf) / STONE variants.
        // ============================================
        static const struct { const char* name; u32 bgm; } kHowlTunes[] = {
            // The 6 DUETS (current move pool — all confirmed working).
            {"DUO LightPrld", Z2BGM_LIGHT_PRLD_DUO}, {"DUO SoulReq", Z2BGM_SOUL_REQ_DUO},
            {"DUO Healing", Z2BGM_HEALING_DUO},      {"DUO New01", Z2BGM_NEW_01_DUO},
            {"DUO New02", Z2BGM_NEW_02_DUO},         {"DUO New03", Z2BGM_NEW_03_DUO},
            // Working solo howls (the rest are silent; STONE variants intentionally omitted).
            {"HOWL Tobikusa", Z2BGM_HOWL_TOBIKUSA},  {"HOWL Umakusa", Z2BGM_HOWL_UMAKUSA},
            {"HOWL ZeldaSong", Z2BGM_HOWL_ZELDASONG},
        };
        static constexpr int kHowlTuneCount = (int)(sizeof(kHowlTunes) / sizeof(kHowlTunes[0]));
        static int s_howlPreviewIdx = 0;
        leftPane.register_control(
            leftPane.add_select_button({
                .key      = "Preview Howl Tune",
                .getValue = [] { return Rml::String(kHowlTunes[s_howlPreviewIdx].name); },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Preview Howl Tune (audition)");
                for (int i = 0; i < kHowlTuneCount; i++) {
                    pane.add_button({
                                        .text = kHowlTunes[i].name,
                                        .isSelected = [i] { return s_howlPreviewIdx == i; },
                                    })
                        .on_pressed([i] {
                            s_howlPreviewIdx = i;
                            Z2GetSeqMgr()->stopWolfHowlSong();  // cut off the current preview
                            mDoAud_subBgmStart(kHowlTunes[i].bgm);
                        });
                }
                pane.add_button({.text = "-- Stop --"}).on_pressed([] {
                    Z2GetSeqMgr()->stopWolfHowlSong();
                });
            });
        static constexpr std::array<const char*, 5> kFocusedArtsCheatModes = {
            "Off",
            "FA Cheat ON",
            "With Debug",
            "FA Cheat + Max Bank",
            "With Debug + Max Bank",
        };
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Focused Arts Cheat",
                .getValue =
                    [] {
                        const auto mode = getSettings().game.focusedArtsCheat.getValue();
                        const auto index = static_cast<size_t>(mode);
                        return kFocusedArtsCheatModes[index < kFocusedArtsCheatModes.size() ? index
                                                                                          : 0];
                    },
                .isDisabled =
                    [] {
                        return getSettings().game.speedrunMode ||
                               !getSettings().game.focusedArts.getValue();
                    },
                .isModified =
                    [] {
                        return getSettings().game.focusedArtsCheat.getValue() !=
                               getSettings().game.focusedArtsCheat.getDefaultValue();
                    },
            }),
            rightPane,
            [](Pane& pane) {
                for (int i = 0; i < static_cast<int>(kFocusedArtsCheatModes.size()); ++i) {
                    pane
                        .add_button({
                            .text = kFocusedArtsCheatModes[i],
                            .isSelected =
                                [i] {
                                    return getSettings().game.focusedArtsCheat.getValue() ==
                                           static_cast<FocusedArtsCheatMode>(i);
                                },
                        })
                        .on_pressed([i] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            getSettings().game.focusedArtsCheat.setValue(
                                static_cast<FocusedArtsCheatMode>(i));
                            config::Save();
                        });
                }
                pane.add_rml(
                    "<b>Off</b>: shop/save tiers only.<br/>"
                    "<b>FA Cheat ON</b>: effective tier 3 (max bank + finishers) for playtest.<br/>"
                    "<b>With Debug</b>: same as ON plus an in-game FA overlay (bank, fill, ALBW, "
                    "recent events) — no need to open the dev console.<br/>"
                    "<b>FA Cheat + Max Bank</b>: tier 3 cheat and bank starts full (3/3) on load.<br/>"
                    "<b>With Debug + Max Bank</b>: max bank cheat plus the debug overlay.<br/>"
                    "Requires <b>Settings → ALBW → Systems → Focused Arts</b>." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        static constexpr std::array<const char*, 3> kTrueAlbwModes = {
            "Off",
            "True ALBW",
            "TRUETEST",
        };
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "True ALBW Mode",
                .getValue =
                    [] {
                        const auto mode = getSettings().game.trueAlbwMode.getValue();
                        const auto index = static_cast<size_t>(mode);
                        return kTrueAlbwModes[index < kTrueAlbwModes.size() ? index : 0];
                    },
                .isDisabled = [] { return !dusk::truetest::canChangeGlobalTrueAlbwMode(); },
                .isModified =
                    [] {
                        return getSettings().game.trueAlbwMode.getValue() !=
                               getSettings().game.trueAlbwMode.getDefaultValue();
                    },
            }),
            rightPane,
            [](Pane& pane) {
                for (int i = 0; i < static_cast<int>(kTrueAlbwModes.size()); ++i) {
                    pane
                        .add_button({
                            .text = kTrueAlbwModes[i],
                            .isSelected =
                                [i] {
                                    return getSettings().game.trueAlbwMode.getValue() ==
                                           static_cast<TrueAlbwMode>(i);
                                },
                        })
                        .on_pressed([i] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            getSettings().game.trueAlbwMode.setValue(static_cast<TrueAlbwMode>(i));
                            config::Save();
                        });
                }
                pane.add_rml(
                    "<br/><b>Off</b>: vanilla reclaim-what-you-lost rental shop.<br/>"
                    "<b>True ALBW</b>: full rental catalog from the Postman (shop only).<br/>"
                    "<b>TRUETEST</b>: enables TRUETEST new-save prompt and per-save world "
                    "bootstrap. Change at file select or title — locked during field play." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.add_section("ALBW WIP");
        // Soulbound Red Potion → Settings → ALBW → Quality of Life.
        // Boss Refinement / Outfit Stats / Shade's Refuge / Realtime Potions /
        // Fists Only / Deku Leaf Glide → Settings → ALBW.
        editor_bool_option(leftPane, rightPane, getSettings().game.heroShadeSecretBoss,
            "Hero's Shade Secret Boss",
            "Post-game secret boss: after all Hidden Skills are learned, a final Hero's "
            "Wolf Shade appears and warps you to the arena for a real duel vs the Hero's "
            "Shade (full health bar + victory). Off disables the whole system." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.albwJuniorMailTest,
            "Junior Postman Mail Test",
            "Phase 0 onboarding mail in North Faron (F_SP108 room 6): spawns the deliver "
            "Postman and queues the strip-feature letter while ignoring story/delivered save "
            "gates. Leave off for normal F_0601 && !delivered gating." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.showLockonHpDebug,
            "Show Lock-on HP Debug",
            "While Z-targeting, shows the locked enemy's current HP, max HP, ALBW category, "
            "and true HP multiplier in a small on-screen overlay." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.showWardrobeRecoveryDebug,
            "Wardrobe Recovery Debug",
            "In-game overlay for Quick Swap resistance: recovery mult, penalty breakdown, "
            "active/stored wardrobe counts, equipped sword/shield/outfit, and passive recovery "
            "rates per 100ms (base vs taxed)." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.albwMagicArmorRentableDebug,
            "Magic Armor Rentable (Debug)",
            "Lists Magic Armor as rentable in the Postman shop before the vanilla purchase." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.showDarknutBashDebug,
            "Darknut Bash Debug Log",
            "Logs Darknut bash/guard-break state and shield bash-start charge snapshots to "
            "Documents/dusklight/albw_darknut_debug.txt (truncated once per session)." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(
            leftPane, rightPane, getSettings().game.hurricaneTest, "Hurricane test",
            "Stick hard left/right + tap sword for a sustained Great Spin prototype (~5 s, then "
            "~2 s tired lockout). Audio: Zant spin 1.05× + Gale tornado 0.85× @ 25% + spinner "
            "ride @ 35%. Particle layout uses Interface → ALBW Visuals → Hurricane Spin Visual." +
                Rml::String(kAlbwUnfinishedDisclaimer),
            []() -> bool { return getSettings().game.speedrunMode; });
        // ============================================
        // NEW CODE — ALBW Port (Demo Leftover Viewer + Cut Actors)
        // Demo lane: all unique BMD/BDL from retail Demo*.arc (365), category filter.
        // Cut-actor lane: E_ms / E_dt / stubs / titan presets via fopAcM_create.
        // ============================================
        leftPane.add_section("Demo Leftover Viewer");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Demo category",
                .getValue =
                    [] {
                        return Rml::String(dDemoLeftoverViewer::categoryName(
                            dDemoLeftoverViewer::categoryFilter()));
                    },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Demo category");
                static const int kCats[] = {
                    dDemoLeftoverViewer::CAT_GOLD,  dDemoLeftoverViewer::CAT_LINK,
                    dDemoLeftoverViewer::CAT_MIDNA, dDemoLeftoverViewer::CAT_WOLF,
                    dDemoLeftoverViewer::CAT_NPC,   dDemoLeftoverViewer::CAT_BOSS,
                    dDemoLeftoverViewer::CAT_PROP,  dDemoLeftoverViewer::CAT_ALL,
                };
                for (int cat : kCats) {
                    pane
                        .add_button({
                            .text = dDemoLeftoverViewer::categoryName(cat),
                            .isSelected =
                                [cat] {
                                    return dDemoLeftoverViewer::categoryFilter() == cat;
                                },
                        })
                        .on_pressed([cat] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            dDemoLeftoverViewer::setCategoryFilter(cat);
                        });
                }
                pane.add_rml(
                    "Filters the <b>365</b> unique Demo*.arc meshes. "
                    "<b>Gold leftovers</b> = original/high/henkei/demo00 crumbs. "
                    "Labels show [body]/[face]/[hand]/… — partials are often intentional "
                    "cutscene pieces." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Demo model",
                .getValue =
                    [] {
                        const auto* e =
                            dDemoLeftoverViewer::entry(dDemoLeftoverViewer::selectedIndex());
                        return Rml::String(e != nullptr ? e->label : "(none)");
                    },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Demo model");
                const int n = dDemoLeftoverViewer::filteredCount();
                // Cap list length so RmlUi stays responsive; prefer category filter.
                constexpr int kMaxButtons = 120;
                const int show = n < kMaxButtons ? n : kMaxButtons;
                for (int fi = 0; fi < show; ++fi) {
                    const int ci = dDemoLeftoverViewer::filteredCatalogIndex(fi);
                    const auto* e = dDemoLeftoverViewer::entry(ci);
                    if (e == nullptr) {
                        continue;
                    }
                    pane
                        .add_button({
                            .text = e->label,
                            .isSelected =
                                [ci] {
                                    return dDemoLeftoverViewer::selectedIndex() == ci;
                                },
                        })
                        .on_pressed([ci] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            dDemoLeftoverViewer::setSelectedIndex(ci);
                        });
                }
                if (n > kMaxButtons) {
                    pane.add_rml(fmt::format(
                        "<br/>Showing {0}/{1} — narrow the category filter to see the rest.",
                        kMaxButtons, n));
                }
                pane.add_rml(
                    "<br/>Spawn draws at Link's feet (no AI actor)." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_button("Spawn demo model at feet")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dDemoLeftoverViewer::requestSpawn();
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Load the selected Demo mesh ~1.5 m in front of Link. Status: " +
                    Rml::String(dDemoLeftoverViewer::status()) +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_button("Despawn demo model")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dDemoLeftoverViewer::requestDespawn();
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml("Free viewer model + Demo arc. Status: " +
                             Rml::String(dDemoLeftoverViewer::status()) +
                             Rml::String(kAlbwUnfinishedDisclaimer));
            });

        leftPane.add_section("Cut Actors");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Cut actor",
                .getValue =
                    [] {
                        const auto* e = dCutActorSpawn::entry(dCutActorSpawn::selectedIndex());
                        return Rml::String(e != nullptr ? e->label : "(none)");
                    },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Cut / stub actors");
                for (int i = 0; i < dCutActorSpawn::entryCount(); ++i) {
                    const auto* e = dCutActorSpawn::entry(i);
                    if (e == nullptr) {
                        continue;
                    }
                    pane
                        .add_button({
                            .text = e->label,
                            .isSelected =
                                [i] { return dCutActorSpawn::selectedIndex() == i; },
                        })
                        .on_pressed([i] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            dCutActorSpawn::setSelectedIndex(i);
                        });
                }
                const auto* cur = dCutActorSpawn::entry(dCutActorSpawn::selectedIndex());
                pane.add_rml(
                    Rml::String("<br/>") +
                    (cur != nullptr ? cur->note : "") +
                    "<br/><br/>Real enemies create live procs. "
                    "<b>STUB</b> entries are external-payload sockets — invisible without a mod." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_button("Spawn cut actor at feet")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dCutActorSpawn::requestSpawn();
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "fopAcM_create at Link's feet (same path as Actor Spawner). "
                    "Tracked for despawn: " +
                    Rml::String(fmt::format("{}", dCutActorSpawn::trackedCount())) +
                    ". Status: " + Rml::String(dCutActorSpawn::status()) +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_button("Despawn cut actors")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dCutActorSpawn::requestDespawn();
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Delete every cut actor this tool still tracks (up to 32). "
                    "Safe if they already died or left the room. Status: " +
                    Rml::String(dCutActorSpawn::status()) +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });

        // №27 N4: identity audition — cycle head / lock name (user is the WW expert).
        leftPane.register_control(
            leftPane.add_button("Cycle head (nearest ExtNpc)")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    fopAc_ac_c* player = dComIfGp_getPlayer(0);
                    if (player == NULL) {
                        return;
                    }
                    dExtNpcMount_cycleHeadNearest(player->current.pos, 800.0f);
                }),
            rightPane,
            [](Pane& pane) {
                fopAc_ac_c* player = dComIfGp_getPlayer(0);
                const char* name = "";
                if (player != NULL) {
                    name = dExtNpcMount_nearestDisplayName(player->current.pos, 800.0f);
                }
                pane.add_rml(
                    "№27 N4: cycle head attach on the nearest external NPC (body+head split). "
                    "Nearest: <b>" +
                    Rml::String(name != nullptr && name[0] ? name : "(none)") + "</b>." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_button("Lock identity = census key (nearest)")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    fopAc_ac_c* player = dComIfGp_getPlayer(0);
                    if (player == NULL) {
                        return;
                    }
                    // Placeholder lock: keeps current display_name into identity.ini.
                    // User renames via editing identity.ini after visual confirm, or
                    // replaces this button's name string once an input field exists.
                    const char* cur =
                        dExtNpcMount_nearestDisplayName(player->current.pos, 800.0f);
                    if (cur != NULL && cur[0]) {
                        dExtNpcMount_setDisplayNameNearest(player->current.pos, 800.0f, cur);
                    }
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Writes <code>population/identity.ini</code> for the nearest ExtNpc. "
                    "After you recognize someone, edit that file to the real name "
                    "(e.g. <code>display_name=RealName</code>) — Cursor must not invent names." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });

        // Wind Waker Item Viewer — get-item toggle + SC color tuning + item dropdown + replay.
        // The held/worn skin selector lives in its own "Wind Waker Skins" section below.
        leftPane.add_section("Wind Waker Item Viewer");
        editor_bool_option(leftPane, rightPane, getSettings().game.wwItemmdlGetItem,
            "WW itemmdl get-item",
            "Use retail itemmdl.arc vbow for Hero's Bow get-item spin (Phase 2 heap wiring). "
            "Off = vanilla O_gD_bow." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.wwItemmdlGetItem2DIsolate,
            "WW itemmdl 2D isolate",
            "Diagnostic: itemmdl arc at create, O_gD_bow mesh on heap (Phase 2 branch 2D). "
            "Requires WW itemmdl get-item ON. Log-only localize — not a shipping path." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.wwItemmdlBowScSuppress,
            "WW bow SC suppress (A/B)",
            "Hide SC_Vbow_v ink pass during get-item draw. Compare close-up cream tips vs "
            "cel outline. Requires WW itemmdl get-item ON." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        editor_bool_option(leftPane, rightPane, getSettings().game.wwItemmdlHeldBootsStyle,
            "WW held skin: boots-style light (A/B)",
            "Render the held WW skin exactly like the WW boots: ambient-only, no SC draw "
            "scope / authentic TEV replay. Matte + bloom-free; SC ink/spec parts may read "
            "flat or unrealized — that trade-off is the experiment." +
                Rml::String(kAlbwUnfinishedDisclaimer));
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "WW bow SC K0 cap",
                .getValue = [] {
                    return getSettings().game.wwItemmdlBowScK0Cap.getValue();
                },
                .setValue = [](int value) {
                    getSettings().game.wwItemmdlBowScK0Cap.setValue(std::clamp(value, 0, 255));
                    config::Save();
                },
                .isModified = [] {
                    return getSettings().game.wwItemmdlBowScK0Cap.getValue() !=
                           getSettings().game.wwItemmdlBowScK0Cap.getDefaultValue();
                },
                .min = 0,
                .max = 255,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Cosmetic SC cap tone: scales kColor[0] (K0) RGB toward matte silver. "
                    "150 = target pewter; 255 = baked authentic white. kColor[2] untouched "
                    "(OpaTexEdge alpha threshold). Requires WW itemmdl get-item ON." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "WW bow SC out ceiling",
                .getValue = [] {
                    return getSettings().game.wwItemmdlBowScOutputCeiling.getValue();
                },
                .setValue = [](int value) {
                    getSettings().game.wwItemmdlBowScOutputCeiling.setValue(
                        std::clamp(value, 0, 255));
                    config::Save();
                },
                .isModified = [] {
                    return getSettings().game.wwItemmdlBowScOutputCeiling.getValue() !=
                           getSettings().game.wwItemmdlBowScOutputCeiling.getDefaultValue();
                },
                .min = 0,
                .max = 255,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Cosmetic SC-pass output RGB ceiling (bloom threshold). 185 = default; "
                    "255 = off. Keeps K0-tuned caps, pulls string/nock below bloom. "
                    "Requires WW itemmdl get-item ON." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Viewer item",
                .getValue = [] { return wwItemmdlViewerLabel(); },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Wind Waker Item Viewer");
                const auto opt = [&pane](const char* label, int index) {
                    pane.add_button({
                                        .text = label,
                                        .isSelected =
                                            [index] {
                                                return getSettings().game.wwItemmdlViewerBdlIndex
                                                           .getValue() == index;
                                            },
                                    })
                        .on_pressed([index] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            getSettings().game.wwItemmdlViewerBdlIndex.setValue(index);
                            config::Save();
                        });
                };
                for (const auto& e : kWwItemmdlViewerItems) {
                    opt(e.label, e.index);
                }
            });
        leftPane.register_control(
            leftPane.add_button("Replay Get-Item Demo (viewer item)")
                .on_pressed([] {
                    mDoAud_seStartMenu(kSoundItemChange);
                    dWwItemmdl::requestBowGetItemDemoReplay();
                }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Dev replay: close the editor in the field to start the bow get-item spin. "
                    "Auto-ends after <b>6 seconds</b>. WW itemmdl toggle selects which arc "
                    "preloads (itemmdl vs O_gD_bow). Status: " +
                    Rml::String(dWwItemmdl::getBowGetItemDemoReplayStatus() != nullptr
                                    ? dWwItemmdl::getBowGetItemDemoReplayStatus()
                                    : "idle") +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
        leftPane.add_section("Wind Waker Skins");
        leftPane.register_control(
            leftPane.add_select_button({
                .key = "Held/worn skin",
                .getValue =
                    [] {
                        switch (getSettings().game.wwItemmdlHeldSkin.getValue()) {
                            case WwHeldSkinMode::Bow:       return Rml::String("Bow");
                            case WwHeldSkinMode::IronBoots: return Rml::String("Iron Boots");
                            case WwHeldSkinMode::Hookshot:  return Rml::String("Hookshot");
                            default:                        return Rml::String("Off");
                        }
                    },
            }),
            rightPane, [](Pane& pane) {
                pane.clear();
                pane.add_section("Wind Waker Skins");
                const auto opt = [&pane](const char* label, WwHeldSkinMode mode) {
                    pane.add_button({
                                        .text = label,
                                        .isSelected =
                                            [mode] {
                                                return getSettings().game.wwItemmdlHeldSkin
                                                           .getValue() == mode;
                                            },
                                    })
                        .on_pressed([mode] {
                            mDoAud_seStartMenu(kSoundItemChange);
                            getSettings().game.wwItemmdlHeldSkin.setValue(mode);
                            config::Save();
                        });
                };
                opt("Off", WwHeldSkinMode::Off);
                opt("Bow", WwHeldSkinMode::Bow);
                opt("Iron Boots", WwHeldSkinMode::IronBoots);
                opt("Hookshot", WwHeldSkinMode::Hookshot);
            });
        leftPane.register_control(
            leftPane.add_child<NumberButton>(NumberButton::Props{
                .key = "Held skin scale %",
                .getValue = [] { return getSettings().game.wwItemmdlHeldBowScalePct.getValue(); },
                .setValue =
                    [](int value) {
                        getSettings().game.wwItemmdlHeldBowScalePct.setValue(
                            std::clamp(value, 1, 1000));
                        config::Save();
                    },
                .isModified =
                    [] {
                        return getSettings().game.wwItemmdlHeldBowScalePct.getValue() !=
                               getSettings().game.wwItemmdlHeldBowScalePct.getDefaultValue();
                    },
                .min = 1,
                .max = 1000,
            }),
            rightPane,
            [](Pane& pane) {
                pane.add_rml(
                    "Scale % for the selected WW skin (100 = 1.0x), live-tunable (Bow / Hookshot). "
                    "Iron Boots is a worn re-rigged model driven by the vanilla foot rig (scale is "
                    "baked in the asset, not this slider); applies on the next clothes rebuild: "
                    "change outfit or reload the area after selecting it." +
                    Rml::String(kAlbwUnfinishedDisclaimer));
            });
    });

    // ========================================================================
    // Level Editor — Stage Inspector (1a). Gated on g_levelEditorSession.
    // Zero mutation: enumerate + list + detail only.
    // ========================================================================
    if (g_levelEditorSession) {
        add_tab("Stage", [this](Rml::Element* content) {
            // Tab switch / reopen: drop any prior Stage state before new panes.
            teardown_stage_tab("stage_tab_open");

            auto& leftPane = add_child<Pane>(content, Pane::Type::Controlled);
            auto& rightPane = add_child<Pane>(content, Pane::Type::Uncontrolled);

            auto state = std::make_shared<StageTabState>();
            state->left = &leftPane;
            state->right = &rightPane;
            mStageTabState = state;
            // weak_ptr: if Editor/Stage closes before the tick, lock fails.
            mStageTabTick = [weak = std::weak_ptr<StageTabState>(state)] {
                if (auto s = weak.lock()) {
                    s->drain_deferred_refresh();
                }
            };
            DuskLog.info("StageTab open left={} right={}", static_cast<const void*>(state->left),
                         static_cast<const void*>(state->right));
            state->refresh();
        });
    }
}

}  // namespace dusk::ui
