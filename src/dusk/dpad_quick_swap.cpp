#include "dusk/dpad_quick_swap.h"

#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "d/d_albw_outfit.h"
#include "d/d_albw_outfit_debug.h"
#include "d/d_albw_rental.h"
#include "d/d_albw_wardrobe.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "d/d_menu_ring.h"
#include "d/d_meter2_info.h"
#include "d/dolzel.h" // IWYU pragma: keep
#include "aurora/lib/input.hpp"
#include "dusk/action_bindings.h"
#include "dusk/settings.h"
#include "f_op/f_op_overlap_mng.h"
#include "m_Do/m_Do_controller_pad.h"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>
#include <chrono>

namespace dusk {

#if TARGET_PC

void applyDpadQuickSwapPresetBinds() {
    if (!isDpadQuickSwapEnabled()) {
        return;
    }

    auto& binds = getSettings().actionBindings;
    auto presetIfUnset = [](ActionBindConfigVar& var, int value) {
        if (var.getValue() == PAD_NATIVE_BUTTON_INVALID) {
            var.setValue(value);
        }
    };

    presetIfUnset(binds.cycleSword[0], SDL_GAMEPAD_BUTTON_DPAD_UP);
    presetIfUnset(binds.cycleShield[0], SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

    // Down was quick transform at ship; migrate to outfit cycle (Down = CYCLE_OUTFIT, R+Y = transform).
    if (binds.cycleOutfit[0].getValue() == PAD_NATIVE_BUTTON_INVALID) {
        if (binds.quickTransform[0].getValue() == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            binds.cycleOutfit[0].setValue(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
            binds.quickTransform[0].setValue(PAD_NATIVE_BUTTON_INVALID);
        } else {
            binds.cycleOutfit[0].setValue(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        }
    }

    presetIfUnset(binds.toggleMinimap[0], SDL_SCANCODE_TAB);
    presetIfUnset(binds.openItemWheel[0], SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);

    // ============================================
    // Alpha cleanup (map button, take 2): the bind slots are PER-PORT
    // (player), one value per action — NOT multiple bindings — and the
    // stored int is interpreted per the port's active device (keyboard
    // scancode when a keyboard profile drives the port, SDL gamepad
    // button otherwise). So the old keyboard-M preset in port 0's slot
    // read as gamepad button 16 (a paddle) on controllers: the map was
    // dead on pad. Pick the preset by the device actually driving port 0
    // — TOUCHPAD click for controllers (no touchpad? rebind in Controller
    // Config; L3 recommended), M for keyboard — and migrate the old
    // keyboard-M value to TOUCHPAD when a controller is active (runs at
    // boot from m_Do_main, so existing configs heal on next launch).
    // ============================================
    const bool port0Controller = aurora::input::get_controller_for_player(0) != nullptr;
    if (port0Controller) {
        if (binds.openMapScreen[0].getValue() == SDL_SCANCODE_M) {
            binds.openMapScreen[0].setValue(SDL_GAMEPAD_BUTTON_TOUCHPAD);
        }
        presetIfUnset(binds.openMapScreen[0], SDL_GAMEPAD_BUTTON_TOUCHPAD);
    } else {
        presetIfUnset(binds.openMapScreen[0], SDL_SCANCODE_M);
    }
}

bool canUseDpadQuickSwap(u32 port) {
    if (!isDpadQuickSwapEnabled() || port != 0) {
        return false;
    }

    if (dALBWRental_isOpen()) {
        return false;
    }

    const int heapLock = dComIfGp_isHeapLockFlag();
    if (heapLock != 0 && heapLock != 5) {
        return false;
    }

    const u8 mapStatus = dMeter2Info_getMapStatus();
    if (mapStatus != 0 && mapStatus != 1) {
        return false;
    }

    if (dMeter2Info_isSub2DStatus(1)) {
        return false;
    }

    if (dComIfGp_isPauseFlag() || dComIfGp_getMesgStatus() != 0 || dComIfGp_isEnableNextStage() ||
        fopOvlpM_IsDoingReq())
    {
        return false;
    }

    // Live quick-equip wheel: D-pad Left/Right flip inventory pages instead.
    if (dMenu_Ring_c::isQuickEquipLiveWorld()) {
        return false;
    }

    return true;
}

static bool isOwnedSword(u8 itemNo) {
    if (!dComIfGs_isItemFirstBit(itemNo)) {
        return false;
    }
    if (isDpadQuickSwapEnabled() && dAlbwWardrobe_isStoredItemNo(itemNo)) {
        return false;
    }
    return true;
}

void cycleNextSword() {
    if (!canUseDpadQuickSwap(0)) {
        return;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr || player->checkWolf()) {
        return;
    }

    if (player->getSwordChangeWaitTimer() != 0) {
        return;
    }

    static constexpr u8 kSwordOrder[] = {
        dItemNo_WOOD_STICK_e,
        dItemNo_SWORD_e,
        dItemNo_MASTER_SWORD_e,
        dItemNo_LIGHT_SWORD_e,
    };

    const u8 current = dComIfGs_getSelectEquipSword();
    u8 next = current;

    static constexpr int kSwordCount = static_cast<int>(sizeof(kSwordOrder) / sizeof(kSwordOrder[0]));

    int start = 0;
    for (int i = 0; i < kSwordCount; ++i) {
        if (kSwordOrder[i] == current) {
            start = i + 1;
            break;
        }
    }

    for (int step = 0; step < kSwordCount; ++step) {
        const u8 candidate = kSwordOrder[(start + step) % kSwordCount];
        if (isOwnedSword(candidate) && candidate != current) {
            next = candidate;
            break;
        }
    }

    if (next == current || next == dItemNo_NONE_e) {
        return;
    }

    dMeter2Info_setSword(next, false);
    Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_SET_X, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
    dMeter2Info_set2DVibration();
}

void cycleNextShield() {
    if (!canUseDpadQuickSwap(0)) {
        return;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr || player->checkWolf()) {
        return;
    }

    if (player->getShieldChangeWaitTimer() != 0) {
        return;
    }

    const u8 current = dComIfGs_getSelectEquipShield();
    u8 next = dMeter2_getNextOwnedShield(current);
    if (isDpadQuickSwapEnabled()) {
        static constexpr u8 kShieldOrder[] = {
            (u8)dItemNo_WOOD_SHIELD_e,
            (u8)dItemNo_SHIELD_e,
            (u8)dItemNo_HYLIA_SHIELD_e,
        };
        int start = 0;
        for (int i = 0; i < 3; ++i) {
            if (kShieldOrder[i] == current) {
                start = i + 1;
                break;
            }
        }
        next = current;
        for (int step = 0; step < 3; ++step) {
            const u8 candidate = kShieldOrder[(start + step) % 3];
            if (candidate != current && dAlbwWardrobe_isActiveShield(candidate)) {
                next = candidate;
                break;
            }
        }
    }
    if (next == dItemNo_NONE_e || next == current) {
        return;
    }

    if (!dMeter2_equipOwnedShield(next)) {
        return;
    }

    Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_SET_X, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
    dMeter2Info_set2DVibration();
}

void cycleNextOutfit() {
    if (!canUseDpadQuickSwap(0)) {
        return;
    }

    daPy_py_c* player = daPy_getLinkPlayerActorClass();
    if (player == nullptr || player->checkWolf()) {
        return;
    }

    // Block the swap in slow/heavy "scripted movement" states (iron boots, depowered
    // Magic Armor, ...) where the clothes-change rebuild launches Link.  Play the parry
    // "not allowed" SFX instead of the switch jingle.  Sumo owns the predicate.
    if (dAlbwOutfit_isSwapBlockedState()) {
        Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_USE_CANCEL, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        dAlbwOutfit_debugLog("cycle blocked: heavy/slow movement state");
        return;
    }

    // Do not queue a new target mid-reload — leaving sumo (or any clothes change)
    // while FLG2/save disagree is the documented crash window (Quick-Sumo Work.md).
    if (dAlbwOutfit_isSwapInProgress()) {
        Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_USE_CANCEL, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        dAlbwOutfit_debugLog("cycle blocked: swap in progress");
        return;
    }

    const dAlbwOutfitKind current = dAlbwOutfit_getActive();
    const dAlbwOutfitKind next = dAlbwOutfit_getNextOwned(current);
    if (next == current) {
        dAlbwOutfit_debugLog("cycle no-op cur=%d next=%d clothTmr=%d", (int)current, (int)next,
                             player->getClothesChangeWaitTimer());
        return;
    }

    if (!dAlbwOutfit_equip(next)) {
        dAlbwOutfit_debugLog("cycle queued cur=%d next=%d", (int)current, (int)next);
        return;
    }

    dAlbwOutfit_debugLog("cycle ok cur=%d next=%d", (int)current, (int)next);

    Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_SET_X, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
    dMeter2Info_set2DVibration();
}

namespace {

constexpr auto kLayeredLeftDoubleTapWindow = std::chrono::milliseconds(300);

struct LayeredLeftState {
    int tapCount = 0;
    std::chrono::steady_clock::time_point firstTapTime{};
    bool midnaTrig = false;
};

static LayeredLeftState sLayeredLeft{};

static void resetLayeredLeftState() {
    sLayeredLeft = {};
}

static bool getCallMidnaLeftEdge(u32 port) {
    if (isActionBound(ActionBinds::CALL_MIDNA, port)) {
        return getActionBindTrig(ActionBinds::CALL_MIDNA, port);
    }

    return mDoCPd_c::getTrigLeft(port) != 0;
}

}  // namespace

bool isLayeredLeftDpadActive(u32 port) {
    return isDpadQuickSwapEnabled() && callMidnaReservesDpadLeft(port);
}

bool consumeLayeredLeftMidnaTrig(u32 port) {
    if (port != 0 || !sLayeredLeft.midnaTrig) {
        return false;
    }

    sLayeredLeft.midnaTrig = false;
    return true;
}

void tickLayeredLeftDpad(u32 port) {
    if (port != 0 || !isLayeredLeftDpadActive(port)) {
        resetLayeredLeftState();
        return;
    }

    if (!canUseDpadQuickSwap(port)) {
        resetLayeredLeftState();
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const bool leftEdge = getCallMidnaLeftEdge(port);

    if (leftEdge) {
        if (sLayeredLeft.tapCount == 0) {
            sLayeredLeft.tapCount = 1;
            sLayeredLeft.firstTapTime = now;
        } else if (sLayeredLeft.tapCount == 1 &&
                   now - sLayeredLeft.firstTapTime <= kLayeredLeftDoubleTapWindow) {
            sLayeredLeft.midnaTrig = true;
            sLayeredLeft.tapCount = 0;
            return;
        } else {
            sLayeredLeft.tapCount = 1;
            sLayeredLeft.firstTapTime = now;
        }
    }

    if (sLayeredLeft.tapCount == 1 &&
        now - sLayeredLeft.firstTapTime > kLayeredLeftDoubleTapWindow) {
        sLayeredLeft.tapCount = 0;
        if (daPy_py_c* player = daPy_getLinkPlayerActorClass()) {
            dynamic_cast<daAlink_c*>(player)->handleQuickTransform();
        }
    }
}

#endif

}
