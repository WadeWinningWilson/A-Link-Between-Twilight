// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_com_inf_game.cpp:185-271 (dComIfG_play_c::getLayerNo)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW STORY-LAYER SELECTION (tale §888) — the donor's OWN getLayerNo, verbatim.
//
// ROOT this closes (§884/§889 render calls): every Ls1/Zl1/P1 placement in
// sea Room44 lives on a STORY LAYER (ACT0/2/8/9/A chunks — zero on Default),
// and the WW room path selected layers through TP's dComIfG_play_c::getLayerNo
// (a TP-story answer for a WW room). Aryll/Tetra/pirates therefore never
// SPAWNED — zero [WwProbe884] lines with the probes proven in the exe.
// Fresh-save Outset resolves to layer 0 by day / 1 by night here, which is
// exactly the ACT0 block carrying Ls1 x2 + Zl1 + P1a.
//
// §865 parallel-systems: this is WW's algorithm on WW-owned reads; TP's
// getLayerNo keeps serving TP stages untouched (seam is WW-host-scoped in
// d_stage.cpp). DIALECT (all established rows): retail branch taken
// (VERSION_SELECT 3rd arg; JPN-only kenroom form dropped); WW story bits ride
// dComIfGs_isEventBit as raw u16 (byte<<8|bit — the §246 WWEV idiom, values
// verbatim from donor d_save_event_flag.inc); day/night + night-stop bind the
// receiver's own time services (engine services per covenant).
// SEAMS: [Y1] dComIfGs_isSymbol is the existing FALSE-stub shim (fresh-save
// truthful; NOTE the shim's dSymbol_DIN_e=0 diverges from donor NAYRU=0/DIN=1
// — inert behind the stub, flagged for the shim owner). [Y2] getTriforceNum
// has NO receiver surface — dExtWw_getTriforceNum() returns 0 (owed WW-save
// accessor; only the Hyrule/Hyroom/kenroom branches read it, all undeclared
// stages today; 0 = "not all shards" = the donor fresh-save answer).
// ============================================================

#include "d/ext_plugin/ww_stage_loader.h"

#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_ext_ww_actor_shims.h"
#include <string.h>

// donor d_save_event_flag.inc values, verbatim (byte<<8|bit encoding).
#define WWEV_LAYER_UNK_0520 0x0520
#define WWEV_LAYER_UNK_0E20 0x0E20
#define WWEV_LAYER_UNK_0101 0x0101
#define WWEV_LAYER_UNK_2D01 0x2D01
#define WWEV_LAYER_UNK_1820 0x1820
#define WWEV_LAYER_MET_KORL 0x0F80
#define WWEV_LAYER_UNK_3280 0x3280
#define WWEV_LAYER_UNK_3B40 0x3B40
#define WWEV_LAYER_UNK_2C01 0x2C01
#define WWEV_LAYER_COLORS_IN_HYRULE 0x3802
#define WWEV_LAYER_UNK_3B02 0x3B02
#define WWEV_LAYER_UNK_4002 0x4002
// donor d_com_inf_game.h:1679 — NAYRU is 0 (the shim's DIN=0 diverges; [Y1]).
#define dSymbol_NAYRU_LAYER_e 0

// [Y2] owed WW-save accessor — no receiver Triforce surface; donor fresh-save
// answer. Retire with the WW save-shadow Triforce counter.
static inline int dExtWw_getTriforceNum() { return 0; }

// donor d_kankyo.cpp dKy_checkEventNightStop, verbatim through the shims:
// isEventBit(ENDLESS_NIGHT) && !isSymbol(NAYRU). WWEV_ENDLESS_NIGHT (0x0A02)
// is the §246 shim define; isSymbol is the [Y1] stub.
static BOOL dExtWw_checkEventNightStop() {
    if (dComIfGs_isEventBit(WWEV_ENDLESS_NIGHT) && !dComIfGs_isSymbol(dSymbol_NAYRU_LAYER_e)) {
        return TRUE;
    }
    return FALSE;
}

// ============================================================
// donor dComIfG_play_c::getLayerNo (d_com_inf_game.cpp:185), retail branch.
// ============================================================
int dExtWw_getLayerNo(int i_roomNo) {
    int stageLayer = dComIfGp_getStartStageLayer();
    if (stageLayer < 0) {
        int hour = dKy_getdaytime_hour();
        int layer = dExtWw_checkEventNightStop() ? 1 :
                    hour >= 6 && hour < 18 ? 0 : 1;

        if (strcmp(dComIfGp_getStartStageName(), "sea") == 0) {
            if (i_roomNo == dIsleRoom_OutsetIsland_e) {
                if (dComIfGs_isEventBit(WWEV_LAYER_UNK_0520)) {
                    return layer | 4;
                } else if (dComIfGs_isEventBit(WWEV_LAYER_UNK_0E20)) {
                    return layer | 2;
                } else if (dComIfGs_isEventBit(WWEV_LAYER_UNK_0101)) {
                    return 9;
                }
            } else if (i_roomNo == dIsleRoom_WindfallIsland_e) {
                if (dComIfGs_isEventBit(WWEV_LAYER_UNK_2D01)) {
                    return layer | 4;
                } else if (dExtWw_checkEventNightStop()) {
                    return layer | 2;
                }
            } else if (i_roomNo == dIsleRoom_ForsakenFortress_e) {
                return dComIfGs_isEventBit(WWEV_LAYER_UNK_1820) ? 3 : 1;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "A_mori") == 0) {
            if (dComIfGs_isEventBit(WWEV_LAYER_MET_KORL)) {
                return layer | 2;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "Asoko") == 0) {
            if (dComIfGs_isEventBit(WWEV_LAYER_UNK_0520)) {
                return layer | 2;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "Hyrule") == 0) {
            if (dExtWw_getTriforceNum() == 8) {
                return layer | 4;
            } else if (dComIfGs_isEventBit(WWEV_LAYER_UNK_3280)) {
                return layer | 2;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "Hyroom") == 0) {
            if (dExtWw_getTriforceNum() == 8 && !dComIfGs_isEventBit(WWEV_LAYER_UNK_2C01)) {
                return layer | 4;
            } else if (dComIfGs_isEventBit(WWEV_LAYER_UNK_3280)) {
                return layer | 2;
            } else if (dComIfGs_isEventBit(WWEV_LAYER_UNK_3B40)) {
                return layer | 6;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "kenroom") == 0) {
            // retail form (VERSION > JPN).
            if (dComIfGs_isEventBit(WWEV_LAYER_UNK_2C01) ||
                (dComIfGs_isEventBit(WWEV_LAYER_COLORS_IN_HYRULE) &&
                 !dComIfGs_isEventBit(WWEV_LAYER_UNK_3280)))
            {
                return layer | 6;
            } else if (dExtWw_getTriforceNum() == 8) {
                return layer | 4;
            } else if (dComIfGs_isEventBit(WWEV_LAYER_COLORS_IN_HYRULE)) {
                // retail VERSION_SELECT arm (USA/PAL = COLORS_IN_HYRULE).
                return layer | 2;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "M2tower") == 0) {
            if (dComIfGs_isEventBit(WWEV_LAYER_UNK_2D01)) {
                return layer | 2;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "GanonK") == 0) {
            if (!dComIfGs_isEventBit(WWEV_LAYER_UNK_3B02)) {
                return 8;
            }
        } else if (strcmp(dComIfGp_getStartStageName(), "GTower") == 0) {
            if (!dComIfGs_isEventBit(WWEV_LAYER_UNK_4002)) {
                return 8;
            }
        }

        return layer;
    }

    return stageLayer;
}
