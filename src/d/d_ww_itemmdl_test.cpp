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
#include "d/d_stage.h"
#include "d/d_ww_itemmdl_pc.h"
#include "dusk/settings.h"
#include "dusk/ui/ui.hpp"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_scene_mng.h"
#include "f_pc/f_pc_create_iter.h"
#include "f_pc/f_pc_create_req.h"
#include "f_pc/f_pc_node.h"
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
static int s_preloadStep = 0;
static const char* s_replayStatus = nullptr;
static fpc_ProcID s_itemId = fpcM_ERROR_PROCESS_ID_e;
static request_of_phase_process_class s_arcPhase;
static char s_demoArcName[32] = {};
static int s_idleStableFrames = 0;
static bool s_logged2VDefer = false;
static s8 s_trackedRoomNo = -1;

// 6 s @ 30 Hz logic frames (matches TP proc timing).
static constexpr int kReplayDurationFrames = 180;
// 2V: require this many consecutive idle frames before spawning demo item.
static constexpr int k2VIdleStableFrames = 30;
static constexpr int k2VDeferTimeoutFrames = 900;

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
    s_preloadStep = 0;
    s_itemId = fpcM_ERROR_PROCESS_ID_e;
    s_arcPhase.id = cPhs_INIT_e;
    s_demoArcName[0] = '\0';
    s_idleStableFrames = 0;
    s_logged2VDefer = false;
    s_trackedRoomNo = -1;
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
    s_preloadStep = 0;
    s_itemId = fpcM_ERROR_PROCESS_ID_e;
    s_arcPhase.id = cPhs_INIT_e;
    s_demoArcName[0] = '\0';
    s_idleStableFrames = 0;
    s_logged2VDefer = false;
    s_trackedRoomNo = -1;
}

static void cancelReplayOnRoomChange(const char* message, daAlink_c* link) {
    if (link != NULL) {
        dWwItemmdl_notifyRoomChange(fopAcM_GetRoomNo(link));
    }

    if (s_phase == REPLAY_NONE) {
        return;
    }

    if (s_phase == REPLAY_RUNNING && link != NULL) {
        finishReplay(message, link);
    } else {
        failReplay(message);
    }
}

static bool sceneHasPendingCreates(scene_class* scene) {
    return scene != NULL && fpcNd_IsCreatingFromUnder(scene) != NULL;
}

static void* pendingCreateOnLayerCallback(void* i_createTag, void* i_layerId) {
    create_request* createReq = (create_request*)((create_tag*)i_createTag)->base.mpTagData;
    if (createReq->layer->layer_id == *(fpc_ProcID*)i_layerId) {
        return createReq;
    }

    return NULL;
}

static bool layerHasPendingCreateRequests(fpc_ProcID layerId) {
    if (layerId == fpcM_ERROR_PROCESS_ID_e) {
        return false;
    }

    return fpcCtIt_Judge((fpcCtIt_JudgeFunc)pendingCreateOnLayerCallback, &layerId) != NULL;
}

static const char* getRoomLoadIdleBlockReason(daAlink_c* link) {
    scene_class* playScene = fopScnM_SearchByID(dStage_roomControl_c::getProcID());
    if (playScene == NULL) {
        return "no play scene";
    }

    if (sceneHasPendingCreates(playScene)) {
        return "play scene pending creates";
    }

    if (layerHasPendingCreateRequests(fopScnM_LayerID(playScene))) {
        return "play layer create queue";
    }

    if (dStage_roomControl_c::getRoomReadId() >= 0) {
        return "room read in progress";
    }

    s8 roomNo = fopAcM_GetRoomNo(link);
    if (roomNo >= 0) {
        if (dComIfGp_roomControl_checkStatusFlag(roomNo, 0x02 | 0x04)) {
            return "room status loading";
        }

        scene_class* roomScene = fopScnM_SearchByID(dStage_roomControl_c::getStatusProcID(roomNo));
        if (roomScene != NULL) {
            if (sceneHasPendingCreates(roomScene)) {
                return "room scene pending creates";
            }

            if (layerHasPendingCreateRequests(fopScnM_LayerID(roomScene))) {
                return "room layer create queue";
            }
        }
    }

    return NULL;
}

static const char* getPreloadArcNameForStep(int step) {
#if TARGET_PC
    if (!dusk::getSettings().game.wwItemmdlGetItem.getValue()) {
        return dItem_data::getArcName(dItemNo_BOW_e);
    }
    if (dWwItemmdl_use2DIsolateHeap()) {
        return step == 0 ? "itemmdl" : dItem_data::getArcName(dItemNo_BOW_e);
    }
    return "itemmdl";
#else
    return dItem_data::getArcName(dItemNo_BOW_e);
#endif
}

static const char* getDemoArcNameForBow() {
    return getPreloadArcNameForStep(0);
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
    s_preloadStep = 0;
    s_idleStableFrames = 0;
    s_logged2VDefer = false;
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

    const s8 roomNo = fopAcM_GetRoomNo(link);
    if (s_trackedRoomNo >= 0 && roomNo != s_trackedRoomNo) {
        cancelReplayOnRoomChange("Cancelled — room changed.", link);
    }
    s_trackedRoomNo = roomNo;

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

        const char* idleBlockReason = getRoomLoadIdleBlockReason(link);
        if (idleBlockReason != NULL) {
            s_idleStableFrames = 0;
            if (!s_logged2VDefer) {
                char message[128];
                snprintf(message, sizeof(message), "2V: defer — %s", idleBlockReason);
                debugLog(message);
                s_logged2VDefer = true;
            }

            if (s_waitFrames > k2VDeferTimeoutFrames) {
                failReplay("Timed out waiting for room load to finish — stand still and retry.");
            }
            return;
        }

        s_idleStableFrames++;
        if (s_idleStableFrames < k2VIdleStableFrames) {
            return;
        }

        s_logged2VDefer = false;
        debugLog("2V: room load idle — preloading demo item archive");

        const char* arcName = getPreloadArcNameForStep(s_preloadStep);
        strncpy(s_demoArcName, arcName, sizeof(s_demoArcName) - 1);
        s_demoArcName[sizeof(s_demoArcName) - 1] = '\0';
        s_arcPhase.id = cPhs_INIT_e;
        s_phase = REPLAY_PRELOAD_ARC;
        s_waitFrames = 0;
        if (dWwItemmdl_use2DIsolateHeap()) {
            debugLog("preloading demo archives (2D: itemmdl then O_gD_bow)");
        }
        return;
    }

    if (s_phase == REPLAY_PRELOAD_ARC) {
        int arcState;
#if TARGET_PC
        if (dusk::getSettings().game.wwItemmdlGetItem.getValue() &&
            !dWwItemmdl_use2DIsolateHeap() && s_preloadStep == 0) {
            arcState = dWwItemmdl_stepPrivateItemmdlArcLoad(&s_arcPhase);
        } else {
            arcState = dComIfG_resLoad(&s_arcPhase, s_demoArcName);
        }
#else
        arcState = dComIfG_resLoad(&s_arcPhase, s_demoArcName);
#endif
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

        if (dWwItemmdl_use2DIsolateHeap() && s_preloadStep == 0) {
            s_preloadStep = 1;
            const char* bowArc = getPreloadArcNameForStep(1);
            strncpy(s_demoArcName, bowArc, sizeof(s_demoArcName) - 1);
            s_demoArcName[sizeof(s_demoArcName) - 1] = '\0';
            s_arcPhase.id = cPhs_INIT_e;
            s_waitFrames = 0;
            debugLog("2D isolate: itemmdl ready, preloading O_gD_bow");
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
