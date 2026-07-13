#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_midna.h"
#include "d/d_meter2.h"
#include "d/d_meter2_draw.h"
#include "d/d_meter2_info.h"

#if TARGET_PC
#include "dusk/action_bindings.h"
#include "dusk/settings.h"
#include "d/d_albw_wolf_stun.h"        // dAlbwWolfCombat_isEnabled, dAlbwWolfArts_isHowlUnlocked
#include "d/d_albw_wolf_charge_hud.h"  // dAlbwWolfChargeHud_notify / _notifyDeny
#include "m_Do/m_Do_audio.h"           // mDoAud_subBgmStart (Wolf Howl music)
#include "Z2AudioLib/Z2SeqMgr.h"       // Z2BGM_HOWL_* howl-tune IDs

namespace {
// Wolf Howl music — the pool is built at howl time from songs the player has FOUND, so the wolf's
// repertoire grows as you learn howl songs.  Each howl picks ONE at random (equal chance); all
// route through the fanfare path (auto-ducks + restores the field BGM).
//
// ALWAYS-available basics: the 3 solo howls have no per-song "learned" save flag (contextual
// field/stone howls), so they're always in the pool -> it is never empty.
const u32 s_wolfHowlSolo[] = {
    Z2BGM_HOWL_TOBIKUSA, Z2BGM_HOWL_UMAKUSA, Z2BGM_HOWL_ZELDASONG,
};
constexpr int kWolfHowlSoloCount = (int)(sizeof(s_wolfHowlSolo) / sizeof(s_wolfHowlSolo[0]));

// DUET-with-the-Golden-Wolf tunes: each is UNLOCKED once its "distant howling complete" event bit
// is set (F_0472..F_0477 = 470 + the tune's Golden-Wolf index 2..7; set the instant the duet ends).
struct WolfHowlDuo {
    u32 bgm;
    int eventFlag;  // dSv_event_flag_c::saveBitLabels[] index (== F_0xxx number)
};
const WolfHowlDuo s_wolfHowlDuos[] = {
    {Z2BGM_HEALING_DUO, 472},    // Golden Wolf 2
    {Z2BGM_SOUL_REQ_DUO, 473},   // Golden Wolf 3
    {Z2BGM_LIGHT_PRLD_DUO, 474}, // Golden Wolf 4
    {Z2BGM_NEW_01_DUO, 475},     // Golden Wolf 5
    {Z2BGM_NEW_02_DUO, 476},     // Golden Wolf 6
    {Z2BGM_NEW_03_DUO, 477},     // Golden Wolf 7
};
constexpr int kWolfHowlDuoCount = (int)(sizeof(s_wolfHowlDuos) / sizeof(s_wolfHowlDuos[0]));
}  // namespace
#endif

void daAlink_c::handleWolfHowl() {
    if (checkWolf()) {
        if (!dusk::getSettings().game.sunsSong) {
            return;
        }

        // Check to see if Link has the ability to transform.
        if (!dComIfGs_isEventBit(dSv_event_flag_c::M_077)) {
            return;
        }

        // Ensure there is a proper pointer to the mMeterClass and mpMeterDraw structs in
        // g_meter2_info.
        const auto meterClassPtr = g_meter2_info.getMeterClass();
        if (!meterClassPtr) {
            return;
        }

        const auto meterDrawPtr = meterClassPtr->getMeterDrawPtr();
        if (!meterDrawPtr) {
            return;
        }

        // Ensure that link is not in a cutscene.
        if (checkEventRun()) {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
            return;
        }

        mDoCPd_c::getCpadInfo(PAD_1).mPressedButtonFlags = 0;

        // Ensure that the Z Button is not dimmed
        if (meterDrawPtr->getButtonZAlpha() != 1.f) {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
            return;
        }

        bool canHowl = false;

        if (mLinkAcch.ChkGroundHit() && !checkModeFlg(MODE_PLAYER_FLY) && !checkMagneBootsOn()) {
            if (checkMidnaRide()) {
                if ((checkWolf() &&
                     (checkModeFlg(MODE_UNK_1000) || dComIfGp_checkPlayerStatus0(0, 0x10))) ||
                    (!checkWolf() &&
                     (checkEventRun() || getMidnaActor()->checkMetamorphoseEnable()) &&
                     (checkModeFlg(4) || dComIfGp_checkPlayerStatus0(0, 0x10))))
                {
                    canHowl = true;
                }
            }
        }

        if (!canHowl) {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
            return;
        }

        getWolfHowlMgrP()->setCorrectCurve(9);
        procWolfHowlDemoInit();
    }
}

// ============================================
// NEW CODE — ALBW Port (Wolf Art: Howl AOE — D-pad Up, increment 1: input + charge + trigger)
// Triggered from f_ap_game.cpp's D-pad dispatch when in wolf form under quick-swap mode (where
// D-pad Up's normal CYCLE_SWORD handler no-ops on wolf, so the direction is free).  Gates on
// Wolf Combat + the howl shop-unlock (bit 713) — both bypassed by game.wolfArtsDevTest — then
// spends 1 wolf charge and plays the standing howl.  The AOE collider, looped Great-Spin VFX,
// and howl music are added in later increments, driven by mWolfCombatHowlActive.
// ============================================
void daAlink_c::handleWolfHowlBurst() {
#if TARGET_PC
    if (!checkWolf() || !dAlbwWolfCombat_isEnabled() || !dusk::isDpadQuickSwapEnabled()) {
        return;
    }

    const bool devTest = dusk::getSettings().game.wolfArtsDevTest.getValue();

    // Unlock gate (dev toggle bypasses).
    if (!devTest && !dAlbwWolfArts_isHowlUnlocked()) {
        return;
    }

    // Not during a cutscene; must be grounded and not flying — it is a "howl in place".
    if (checkEventRun()) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }
    if (!mLinkAcch.ChkGroundHit() || checkModeFlg(MODE_PLAYER_FLY)) {
        return;
    }

    // Charge gate: need >= 1, spend 1.  The dev toggle skips both cost and requirement.
    if (!devTest) {
        if (mWolfChargeCount < 1) {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
            dAlbwWolfChargeHud_notifyDeny();
            return;
        }
        mWolfChargeCount--;
        dAlbwWolfChargeHud_notify();
    }

    // Consume the D-pad edge so it doesn't leak to the vanilla item-ring/menu layer.
    mDoCPd_c::getCpadInfo(PAD_1).mPressedButtonFlags = 0;

    // Trigger the howl.  mWolfHowlWantCombat is the one-shot "this is a combat howl" request that
    // procWolfHowlInit consumes into mWolfCombatHowlActive (drives the duet pose / AOE / music /
    // song-length timing).  The duration is now driven by the song itself (procWolfHowlCombat polls
    // isItemGetDemo), so there is no fixed timer — mWolfHowlFramesLeft is only the post-song buffer.
    mWolfHowlWantCombat = true;
    mWolfHowlEnding     = false;
    mWolfHowlElapsed    = 0;
    mWolfHowlFramesLeft = 60;  // 1 s post-song buffer (re-armed each singing frame)

    // Build the pool from FOUND songs (3 always-on solos + each learned DUO), then pick one at
    // RANDOM and start it once for this howl.  The fanfare path ducks other audio; procWolfHowl
    // stops it (stopWolfHowlSong) when the howl ends.  Started here (not in procWolfHowlInit) so the
    // re-loop does not restart the song.
    {
        u32 pool[kWolfHowlSoloCount + kWolfHowlDuoCount];
        int n = 0;
        for (int i = 0; i < kWolfHowlSoloCount; i++) {
            pool[n++] = s_wolfHowlSolo[i];
        }
        for (int i = 0; i < kWolfHowlDuoCount; i++) {
            if (dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[s_wolfHowlDuos[i].eventFlag])) {
                pool[n++] = s_wolfHowlDuos[i].bgm;  // duet learned -> add to the pool
            }
        }
        int idx = (int)cM_rndF((f32)n);
        if (idx < 0 || idx >= n) {
            idx = 0;
        }
        mDoAud_subBgmStart(pool[idx]);
    }

    // param is ignored for a combat howl (procWolfHowlInit's combat branch overrides the anim to
    // the WANM_HOWL_SUCCESS duet pose once it consumes mWolfHowlWantCombat).
    procWolfHowlInit(0);
#endif
}

void daAlink_c::handleQuickTransform() {
#if TARGET_PC
    if (!dusk::isDpadQuickSwapEnabled() && !dusk::getSettings().game.enableQuickTransform.getValue()) {
        return;
    }
#else
    if (!dusk::getSettings().game.enableQuickTransform.getValue()) {
        return;
    }
#endif

    // Check to see if Link has the ability to transform.
    if (!dComIfGs_isEventBit(dSv_event_flag_c::M_077)) {
        return;
    }

    // Ensure there is a proper pointer to the mMeterClass and mpMeterDraw structs in g_meter2_info.
    const auto meterClassPtr = g_meter2_info.getMeterClass();
    if (!meterClassPtr) {
        return;
    }

    const auto meterDrawPtr = meterClassPtr->getMeterDrawPtr();
    if (!meterDrawPtr) {
        return;
    }

    // Ensure that link is not in a cutscene.
    if (checkEventRun()) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

    mDoCPd_c::getCpadInfo(PAD_1).mPressedButtonFlags = 0;

    // Don't allow quick transform while in the STAR tent.
    if (checkStageName("R_SP161")) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

    // The game will crash if trying to quick transform while holding the Ball and Chain
    if (mEquipItem == dItemNo_IRONBALL_e) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

    // Use the game's default checks for if the player can currently transform
    if (!m_midnaActor->checkMetamorphoseEnableBase()) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

#if TARGET_PC
    // Z alpha tracks Midna-on-Z affordance. Extra Item Slot moves Call Midna to left
    // D-pad (Z = third item), so Z stays dim for Midna even when transform is allowed —
    // including layered left single-tap. Trust Midna metamorphose checks instead.
    const bool skipZDimGate = dusk::isExtraItemSlotEnabled();
#else
    const bool skipZDimGate = false;
#endif

    if (!skipZDimGate && meterDrawPtr->getButtonZAlpha() != 1.f) {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

    bool canTransform = false;

    if (mLinkAcch.ChkGroundHit() && !checkModeFlg(MODE_PLAYER_FLY) && !checkMagneBootsOn()) {
        if (checkMidnaRide()) {
#if TARGET_PC
            if ((checkField() || checkCastleTown()) && !checkStageName("R_SP161")) {
                if ((checkWolf() && (checkModeFlg(MODE_UNK_1000) ||
                                     dComIfGp_checkPlayerStatus0(0, 0x10))) ||
                    (!checkWolf() &&
                     (checkEventRun() || getMidnaActor()->checkMetamorphoseEnable()) &&
                     (checkModeFlg(MODE_CLIMB) || dComIfGp_checkPlayerStatus0(0, 0x10))))
                {
                    canTransform = true;
                }
                // Bidirectional in open field when Midna allows transform (matches Z intent).
                if (!canTransform && getMidnaActor()->checkMetamorphoseEnable()) {
                    canTransform = true;
                }
            }
#else
            if ((checkWolf() && (checkModeFlg(MODE_UNK_1000) ||
                                 dComIfGp_checkPlayerStatus0(0, 0x10))) ||
                (!checkWolf() &&
                 (checkEventRun() || getMidnaActor()->checkMetamorphoseEnable()) &&
                 (checkModeFlg(MODE_CLIMB) || dComIfGp_checkPlayerStatus0(0, 0x10))))
            {
                canTransform = true;
            }
#endif
        }
    }

    if (!canTransform)
    {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        return;
    }

    procCoMetamorphoseInit();
}

bool daAlink_c::checkAimContext() {
    switch (mProcID) {
    case PROC_SUBJECTIVITY:
    case PROC_SWIM_SUBJECTIVITY:
    case PROC_HORSE_SUBJECTIVITY:
    case PROC_CANOE_SUBJECTIVITY:
    case PROC_BOARD_SUBJECTIVITY:
    case PROC_WOLF_ROPE_SUBJECTIVITY:
    case PROC_BOW_SUBJECT:
    case PROC_BOOMERANG_SUBJECT:
    case PROC_COPY_ROD_SUBJECT:
    case PROC_HAWK_SUBJECT:
    case PROC_HOOKSHOT_SUBJECT:
    case PROC_SWIM_HOOKSHOT_SUBJECT:
    case PROC_HORSE_BOW_SUBJECT:
    case PROC_HORSE_BOOMERANG_SUBJECT:
    case PROC_HORSE_HOOKSHOT_SUBJECT:
    case PROC_CANOE_BOW_SUBJECT:
    case PROC_CANOE_BOOMERANG_SUBJECT:
    case PROC_CANOE_HOOKSHOT_SUBJECT:
    case PROC_HOOKSHOT_ROOF_WAIT:
    case PROC_HOOKSHOT_ROOF_SHOOT:
    case PROC_HOOKSHOT_WALL_WAIT:
    case PROC_HOOKSHOT_WALL_SHOOT:
        return true;
    case PROC_IRON_BALL_SUBJECT:
        return itemButton() && mItemVar0.field_0x3018 == 2;
    default:
        return false;
    }
}

bool daAlink_c::checkAimInputContext() {
    switch (mProcID) {
    case PROC_HOOKSHOT_ROOF_WAIT:
    case PROC_HOOKSHOT_WALL_WAIT:
        return false;
    default:
        return checkAimContext();
    }
}
