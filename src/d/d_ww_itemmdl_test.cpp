/**
 * d_ww_itemmdl_test.cpp — dev replay for WW itemmdl bow get-item demo.
 */

#include "d/d_ww_itemmdl_test.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_demo_item.h"
#include "d/actor/d_a_player.h"
#include "d/d_a_itembase_static.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "d/d_ww_itemmdl_pc.h"
#include "dusk/settings.h"
#include "dusk/ui/ui.hpp"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include "SSystem/SComponent/c_counter.h"
#include "SSystem/SComponent/c_phase.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace dWwItemmdl {

enum ReplayPhase {
    REPLAY_NONE,
    REPLAY_PENDING,
    REPLAY_PRELOAD_ARC,
    REPLAY_WAIT_ITEM,
    REPLAY_START_DEMO,
    REPLAY_RUNNING,
};

static ReplayPhase s_phase = REPLAY_NONE;
static int s_waitFrames = 0;
static int s_runningFrames = 0;
static const char* s_replayStatus = nullptr;
static fpc_ProcID s_itemId = fpcM_ERROR_PROCESS_ID_e;
static request_of_phase_process_class s_arcPhase;
static char s_demoArcName[32] = {};

// 6 s @ 30 Hz logic frames (matches TP proc timing).
static constexpr int kReplayDurationFrames = 180;

static void debugLog(const char* message) {
    const char* user = getenv("USERPROFILE");
    char path[512];
    if (user != nullptr && user[0] != '\0') {
        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_ww_itemmdl_debug.txt", user);
    } else {
        strncpy(path, "albw_ww_itemmdl_debug.txt", sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    FILE* fp = fopen(path, "a");
    if (fp == nullptr) {
        return;
    }

    fprintf(fp, "frame=%d phase=%d: %s\n", g_Counter.mCounter0, static_cast<int>(s_phase), message);
    fclose(fp);
}

static void failReplay(const char* message) {
    debugLog(message);
    s_replayStatus = message;
    s_phase = REPLAY_NONE;
    s_waitFrames = 0;
    s_runningFrames = 0;
    s_itemId = fpcM_ERROR_PROCESS_ID_e;
    s_arcPhase.id = cPhs_INIT_e;
    s_demoArcName[0] = '\0';
}

static void finishReplay(const char* statusMessage, daAlink_c* link) {
    debugLog(statusMessage);
    if (s_itemId != fpcM_ERROR_PROCESS_ID_e && fpcM_IsExecuting(s_itemId)) {
        fopAcM_delete(s_itemId);
    }
    dComIfGp_event_setItemPartnerId(fpcM_ERROR_PROCESS_ID_e);
    if (link != NULL) {
        link->procWaitInit();
    }
    s_replayStatus = statusMessage;
    s_phase = REPLAY_NONE;
    s_waitFrames = 0;
    s_runningFrames = 0;
    s_itemId = fpcM_ERROR_PROCESS_ID_e;
    s_arcPhase.id = cPhs_INIT_e;
    s_demoArcName[0] = '\0';
}

static const char* getDemoArcNameForBow() {
#if TARGET_PC
    if (dusk::getSettings().game.wwItemmdlGetItem.getValue()) {
        return "itemmdl";
    }
#endif
    return dItem_data::getArcName(dItemNo_BOW_e);
}

static fopAc_ac_c* getDemoItemActor() {
    if (s_itemId == fpcM_ERROR_PROCESS_ID_e) {
        return NULL;
    }

    return fopAcM_SearchByID(s_itemId);
}

static bool demoItemReady() {
    if (s_itemId == fpcM_ERROR_PROCESS_ID_e || fpcM_IsCreating(s_itemId)) {
        return false;
    }

    daDitem_c* item = static_cast<daDitem_c*>(getDemoItemActor());
    if (item == NULL || !fpcM_IsExecuting(s_itemId)) {
        return false;
    }

    return item->mpModel != NULL;
}

static void startDirectGetItemDemo(daAlink_c* link) {
    fopAc_ac_c* item = getDemoItemActor();
    if (item == NULL) {
        failReplay("Get-item demo model disappeared before the presentation could start.");
        return;
    }

    debugLog("starting procCoGetItemInit (direct, no DEFAULT_GETITEM event)");
    dComIfGp_event_setItemPartnerId(s_itemId);
    link->changeDemoParam0(0);
    link->procCoGetItemInit();
    s_phase = REPLAY_RUNNING;
    s_waitFrames = 0;
    s_runningFrames = 0;
    debugLog("procCoGetItemInit returned (dev replay; auto-end 6s)");
}

void requestBowGetItemDemoReplay() {
    if (s_phase == REPLAY_RUNNING) {
        s_replayStatus = "Replay already in progress.";
        return;
    }

    if (s_phase != REPLAY_NONE) {
        s_replayStatus = "Replay queued — close the editor to start.";
        return;
    }

    s_phase = REPLAY_PENDING;
    s_waitFrames = 0;
    s_replayStatus = "Replay queued — close the editor to start the demo.";
    s_itemId = fpcM_ERROR_PROCESS_ID_e;
    s_arcPhase.id = cPhs_INIT_e;
    s_demoArcName[0] = '\0';
    debugLog("replay requested");
}

const char* getBowGetItemDemoReplayStatus() {
    return s_replayStatus;
}

void tickBowGetItemDemoReplay() {
    if (s_phase == REPLAY_NONE) {
        return;
    }

    s_waitFrames++;

    daAlink_c* link = static_cast<daAlink_c*>(daPy_getPlayerActorClass());
    if (link == nullptr) {
        if (s_waitFrames > 300) {
            failReplay("Enter the field with a loaded save first.");
        }
        return;
    }

    if (dusk::ui::any_document_visible()) {
        return;
    }

    if (s_phase == REPLAY_RUNNING) {
        s_runningFrames++;

        if (s_runningFrames >= kReplayDurationFrames) {
            finishReplay("Replay finished (6 s).", link);
            return;
        }

        if (getDemoItemActor() == NULL) {
            finishReplay("Replay finished.", link);
            return;
        }

        if (s_runningFrames > kReplayDurationFrames + 60) {
            failReplay("Get-item demo replay timed out.");
        }
        return;
    }

    if (s_phase == REPLAY_PENDING) {
        if (dComIfGp_event_runCheck() || link->checkPlayerDemoMode()) {
            if (s_waitFrames > 300) {
                failReplay("Could not start demo — finish cutscenes and stand still in the field.");
            }
            return;
        }

        if (s_waitFrames < 15) {
            return;
        }

        const char* arcName = getDemoArcNameForBow();
        strncpy(s_demoArcName, arcName, sizeof(s_demoArcName) - 1);
        s_demoArcName[sizeof(s_demoArcName) - 1] = '\0';
        s_arcPhase.id = cPhs_INIT_e;
        s_phase = REPLAY_PRELOAD_ARC;
        s_waitFrames = 0;
        debugLog("preloading demo item archive");
        return;
    }

    if (s_phase == REPLAY_PRELOAD_ARC) {
        const int arcState = dComIfG_resLoad(&s_arcPhase, s_demoArcName);
        if (arcState == cPhs_ERROR_e) {
            failReplay("Could not load the get-item demo archive (missing itemmdl.arc or O_gD_bow?).");
            return;
        }

        if (arcState != cPhs_COMPLEATE_e) {
            if (s_waitFrames > 180) {
                failReplay("Timed out preloading the get-item demo archive.");
            }
            return;
        }

        debugLog("spawning present-demo item");
        s_itemId = fopAcM_createItemForPresentDemo(&link->current.pos, dItemNo_BOW_e, 1, -1,
                                                   fopAcM_GetRoomNo(link), NULL, NULL);
        if (s_itemId == fpcM_ERROR_PROCESS_ID_e) {
            failReplay("Could not spawn the get-item demo model.");
            return;
        }

        s_phase = REPLAY_WAIT_ITEM;
        s_waitFrames = 0;
        return;
    }

    if (s_phase == REPLAY_WAIT_ITEM) {
        if (!demoItemReady()) {
            if (getDemoItemActor() == NULL && s_waitFrames > 30) {
                if (dusk::getSettings().game.wwItemmdlGetItem.getValue()) {
                    failReplay("Could not load vbow from itemmdl.arc — ensure retail itemmdl.arc is in "
                               "res/Object/ (or turn the WW toggle off to test vanilla).");
                } else {
                    failReplay("Get-item demo model failed to initialize.");
                }
            } else if (s_waitFrames > 180) {
                failReplay("Timed out waiting for the get-item demo model.");
            }
            return;
        }

        debugLog("demo item ready");
        startDirectGetItemDemo(link);
        return;
    }

    if (s_phase == REPLAY_START_DEMO) {
        if (!demoItemReady()) {
            failReplay("Get-item demo model disappeared before the presentation could start.");
            return;
        }

        startDirectGetItemDemo(link);
    }
}

}  // namespace dWwItemmdl

#endif
