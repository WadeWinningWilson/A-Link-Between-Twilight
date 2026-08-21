// ============================================
// NEW CODE — ALBW Port (Region Multipliers)
// Province/dungeon difficulty table. Region Damage is standalone;
// master + Health/Rupees axes share the same lookup.
// ============================================
#include "d/d_albw_region_mult.h"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_stage.h"
#include "dusk/settings.h"
#include <cmath>
#include <cstring>

namespace {

f32 tableMultForSaveTbl(int saveTbl) {
    switch (saveTbl) {
    case dStage_SaveTbl_ORDON:
    case dStage_SaveTbl_FARON:
        return 1.00f;
    case dStage_SaveTbl_PRISON:
        // Castle sewers / early twilight — treat with Lanayru hub tier.
        return 1.50f;
    case dStage_SaveTbl_ELDIN:
        return 1.25f;
    case dStage_SaveTbl_LANAYRU:
    case dStage_SaveTbl_CASTLE_TOWN:
    case dStage_SaveTbl_FISHING_POND:
        return 1.50f;
    case dStage_SaveTbl_FIELD:
        // Fallback when no F_SP121 room override matches.
        return 1.05f;
    case dStage_SaveTbl_GROVE:
        return 2.25f;
    case dStage_SaveTbl_SNOWPEAK:
        return 2.00f;
    case dStage_SaveTbl_DESERT:
        return 1.75f;
    case dStage_SaveTbl_LV1:  // Forest Temple
        return 1.05f;
    case dStage_SaveTbl_LV2:  // Goron Mines
        return 1.40f;
    case dStage_SaveTbl_LV3:  // Lakebed Temple
        return 1.65f;
    case dStage_SaveTbl_LV4:  // Arbiter's Grounds
        return 1.90f;
    case dStage_SaveTbl_LV5:  // Snowpeak Ruins
        return 2.15f;
    case dStage_SaveTbl_LV6:  // Temple of Time
        return 2.40f;
    case dStage_SaveTbl_LV7:  // City in the Sky
        return 2.65f;
    case dStage_SaveTbl_LV8:  // Palace of Twilight
        return 2.90f;
    case dStage_SaveTbl_LV9:  // Hyrule Castle
        return 3.15f;
    case dStage_SaveTbl_CAVE1:
    case dStage_SaveTbl_CAVE2:
    case dStage_SaveTbl_GROTTO:
        return 1.05f;
    default:
        return 1.00f;
    }
}

// Room-level overrides (warp-menu / dKy_F_SP121Check names). Return <0 if N/A.
f32 tableMultForStageRoom(const char* stage, int roomNo) {
    if (stage == NULL || stage[0] == '\0' || roomNo < 0) {
        return -1.0f;
    }

    // Hyrule Field — province pockets (tweaks later: rooms 2, 7, 14).
    if (strcmp(stage, "F_SP121") == 0) {
        switch (roomNo) {
        case 1:   // Faron Field / Kakariko Gorge Path South
        case 6:   // Faron Field
        case 15:  // Faron Field / Great Bridge of Hylia Path South
            return 1.05f;
        case 0:   // Eldin Field
        case 2:   // Faron Field / Kakariko Gorge Path North (Eldin twilight tag)
        case 3:   // Kakariko Gorge
        case 4:   // Eldin Field / Kakariko Gorge Path South
        case 5:   // Eldin Field / Kakariko Gorge Path North
        case 7:   // Outside Hidden Village
            return 1.25f;
        case 9:   // Lanayru Field / Outside Hidden Village Path
        case 10:  // Lanayru Field
        case 11:  // Lanayru Field / Great Bridge of Hylia Path North
        case 12:  // Lanayru Field / Great Bridge of Hylia Path South
        case 13:  // Great Bridge of Hylia
        case 14:  // Faron Field / Great Bridge of Hylia Path North (Lanayru tag)
            return 1.50f;
        default:
            return -1.0f;
        }
    }

    // Sacred Grove stage — Lost Woods early vs Grove / ToT Past.
    if (strcmp(stage, "F_SP117") == 0) {
        if (roomNo == 3) {
            return 2.25f;  // Lost Woods (aligned with Grove / ToT Past)
        }
        if (roomNo == 1 || roomNo == 2) {
            return 2.25f;  // Sacred Grove / Temple of Time (Past)
        }
        return -1.0f;
    }

    // Outside Castle Town — always Lanayru hub tier.
    if (strcmp(stage, "F_SP122") == 0) {
        return 1.50f;
    }

    return -1.0f;
}

// Mini-dungeons / misc stages whose SaveTbl is too coarse for the table.
f32 tableMultForStageName(const char* stage) {
    if (stage == NULL || stage[0] == '\0') {
        return -1.0f;
    }
    if (strcmp(stage, "D_SB00") == 0) {
        return 2.00f;  // Ice Cavern (Snowpeak)
    }
    if (strcmp(stage, "D_SB01") == 0) {
        return 2.65f;  // Cave of Ordeals
    }
    if (strcmp(stage, "D_SB02") == 0 || strcmp(stage, "D_SB04") == 0) {
        return 1.25f;  // Eldin caverns
    }
    if (strcmp(stage, "D_SB03") == 0) {
        return 1.50f;  // Lake Hylia cavern
    }
    if (strcmp(stage, "D_SB10") == 0) {
        return 1.00f;  // Faron Woods Cave
    }
    if (strcmp(stage, "F_SP102") == 0) {
        return 1.05f;  // King Bulblin 1 / title field
    }
    if (strcmp(stage, "F_SP123") == 0) {
        return 1.75f;  // King Bulblin 2 (desert-era)
    }
    if (strcmp(stage, "F_SP118") == 0 || strcmp(stage, "F_SP124") == 0 ||
        strcmp(stage, "F_SP125") == 0)
    {
        return 1.75f;  // Bulblin Camp / Gerudo Desert / Mirror Chamber
    }
    return -1.0f;
}

int currentRoomNo() {
    const int stay = dComIfGp_roomControl_getStayNo();
    if (stay >= 0) {
        return stay;
    }
    return dComIfGp_getStartStageRoomNo();
}

f32 resolveTableMult() {
    const char* stage = dComIfGp_getStartStageName();
    const f32 byRoom = tableMultForStageRoom(stage, currentRoomNo());
    if (byRoom > 0.0f) {
        return byRoom;
    }

    const f32 byName = tableMultForStageName(stage);
    if (byName > 0.0f) {
        return byName;
    }

    stage_stag_info_class* stag = dComIfGp_getStageStagInfo();
    if (stag == NULL) {
        return 1.00f;
    }
    return tableMultForSaveTbl(dStage_stagInfo_GetSaveTbl(stag));
}

u16 scaleAmountU16(u16 amount, f32 mult) {
    if (amount == 0 || mult <= 1.0f) {
        return amount;
    }
    const f32 scaled = static_cast<f32>(amount) * mult;
    int rounded = static_cast<int>(scaled + 0.5f);
    if (rounded < 1) {
        rounded = 1;
    }
    if (rounded > 0xFFFF) {
        rounded = 0xFFFF;
    }
    return static_cast<u16>(rounded);
}

}  // namespace

bool dAlbwRegionMult_isEnabled() {
    return dusk::getSettings().game.regionMult.getValue();
}

f32 dAlbwRegionMult_getTableMult() {
    return resolveTableMult();
}

f32 dAlbwRegionMult_getDamageMult() {
    // Standalone setting — does not require Region Multipliers master.
    if (!dusk::getSettings().game.regionDamage.getValue()) {
        return 1.0f;
    }
    return resolveTableMult();
}

f32 dAlbwRegionMult_getHealthMult() {
    if (!dAlbwRegionMult_isEnabled() ||
        !dusk::getSettings().game.regionMultHealth.getValue()) {
        return 1.0f;
    }
    return resolveTableMult();
}

f32 dAlbwRegionMult_getRupeeMult() {
    if (!dAlbwRegionMult_isEnabled() ||
        !dusk::getSettings().game.regionMultRupees.getValue()) {
        return 1.0f;
    }
    return resolveTableMult();
}

// Region Damage On → flat ×3 on enemy-death / fight-victory grants (independent of RM master).
f32 dAlbwRegionMult_getRegionDamageRupeeMult() {
    if (!dusk::getSettings().game.regionDamage.getValue()) {
        return 1.0f;
    }
    return 3.0f;
}

s16 dAlbwRegionMult_scaleHp(s16 hp) {
    if (hp <= 1) {
        return hp;
    }
    const f32 mult = dAlbwRegionMult_getHealthMult();
    if (mult <= 1.0f) {
        return hp;
    }
    const int scaled = static_cast<int>(static_cast<f32>(hp) * mult + 0.5f);
    if (scaled > 32767) {
        return 32767;
    }
    if (scaled < 1) {
        return 1;
    }
    return static_cast<s16>(scaled);
}

u16 dAlbwRegionMult_scaleRupees(u16 amount) {
    // finalGrant ≈ base × (RD ? 3 : 1) × (RM rupees ? table : 1)
    const f32 mult =
        dAlbwRegionMult_getRupeeMult() * dAlbwRegionMult_getRegionDamageRupeeMult();
    return scaleAmountU16(amount, mult);
}

#endif  // TARGET_PC
