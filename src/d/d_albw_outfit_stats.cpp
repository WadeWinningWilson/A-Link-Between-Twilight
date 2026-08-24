// ============================================
// NEW CODE — ALBW Port (Outfit Stats)
// See include/d/d_albw_outfit_stats.h and docs/Outfit Stats.md.
// ============================================
#include "d/d_albw_outfit_stats.h"

#if TARGET_PC

#include "d/actor/d_a_player.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "dusk/settings.h"

namespace {

int sZoraGraceTimer = 0;
f32 sNonZoraOxygenDrainAccum = 0.0f;

f32 receivedMultForKind(dAlbwOutfitKind kind, bool zoraWaterBuff) {
    switch (kind) {
        case D_ALBW_OUTFIT_SUMO:
            return 3.5f;
        case D_ALBW_OUTFIT_ORDON:
            return 2.5f;
        case D_ALBW_OUTFIT_HEROS:
            return 1.5f;
        case D_ALBW_OUTFIT_ZORA:
            return zoraWaterBuff ? 1.5f : 2.5f;
        case D_ALBW_OUTFIT_MAGIC:
        case D_ALBW_OUTFIT_DEITY:
            return 1.0f;
        default:
            return 1.0f;
    }
}

}  // namespace

bool dAlbwOutfitStats_isEnabled() {
    return dusk::getSettings().game.outfitStats.getValue();
}

dAlbwOutfitKind dAlbwOutfitStats_getActiveOutfitKind() {
    return dAlbwOutfit_getActive();
}

f32 dAlbwOutfitStats_getReceivedDamageMult() {
    if (!dAlbwOutfitStats_isEnabled()) {
        return 1.0f;
    }

    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player != NULL && player->checkWolf()) {
        return 1.0f;
    }

    return receivedMultForKind(dAlbwOutfitStats_getActiveOutfitKind(),
                               dAlbwOutfitStats_isZoraWaterBuffActive());
}

bool dAlbwOutfitStats_isSumoOffensiveKitActive() {
    if (!dAlbwOutfitStats_isEnabled()) {
        return false;
    }

    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player != NULL && player->checkWolf()) {
        return false;
    }

    if (dAlbwOutfitStats_getActiveOutfitKind() != D_ALBW_OUTFIT_SUMO) {
        return false;
    }

    if (dComIfGs_getSelectEquipSword() != dItemNo_WOOD_STICK_e) {
        return false;
    }

    return dComIfGs_getSelectEquipShield() == dItemNo_NONE_e;
}

bool dAlbwOutfitStats_allowsWoodHiddenSkills() {
    return dAlbwOutfitStats_isSumoOffensiveKitActive();
}

u16 dAlbwOutfitStats_applyOutgoingDamageMult(u16 attackPower) {
    if (attackPower == 0 || !dAlbwOutfitStats_isSumoOffensiveKitActive()) {
        return attackPower;
    }

    const u32 scaled =
        static_cast<u32>(attackPower * kAlbwOutfitStatsSumoOffensiveDamageMult);
    if (scaled > 0xFFFFu) {
        return 0xFFFFu;
    }

    return static_cast<u16>(scaled);
}

f32 dAlbwOutfitStats_getSwimSpeedMult(const daAlink_c* link) {
    if (!dAlbwOutfitStats_isEnabled() || link == NULL || link->checkWolf()) {
        return 1.0f;
    }

    f32 mult = 1.05f;
    if (link->checkZoraWearAbility()) {
        mult *= 1.10f;
    }
    return mult;
}

f32 dAlbwOutfitStats_scaleSwimSpeed(const daAlink_c* link, f32 speed) {
    return speed * dAlbwOutfitStats_getSwimSpeedMult(link);
}

f32 dAlbwOutfitStats_getSubmergedHumanSwimSpeedMult(const daAlink_c* link) {
    if (dAlbwOutfitStats_isSubmergedHumanSwim(link)) {
        return kAlbwOutfitStatsSubmergedHumanSwimMult;
    }

    return 1.0f;
}

bool dAlbwOutfitStats_allowsSubmergedSwim(const daAlink_c* link) {
    if (!dAlbwOutfitStats_isEnabled() || link == NULL || link->checkWolf()) {
        return false;
    }

    return !link->checkBootsOrArmorHeavy();
}

bool dAlbwOutfitStats_isSubmergedHumanSwim(const daAlink_c* link) {
    if (!dAlbwOutfitStats_allowsSubmergedSwim(link) || link->checkZoraWearAbility()) {
        return false;
    }

    if (!link->checkModeFlg(daAlink_c::MODE_SWIMMING)) {
        return false;
    }

    if (!link->checkSwimUp() || link->getZoraSwim()) {
        return true;
    }

    // Dive init can still have FLG0_SWIM_UP set briefly; keep buoyancy/eject guards active.
    return link->mProcID == daAlink_c::PROC_SWIM_DIVE;
}

void dAlbwOutfitStats_resetOxygenDrainAccum() {
    sNonZoraOxygenDrainAccum = 0.0f;
}

void dAlbwOutfitStats_tickNonZoraOxygenDrain() {
    sNonZoraOxygenDrainAccum += kAlbwOutfitStatsNonZoraOxygenDrainMult;
    while (sNonZoraOxygenDrainAccum >= 1.0f) {
        dComIfGp_setOxygenCount(-1);
        sNonZoraOxygenDrainAccum -= 1.0f;
    }
}

bool dAlbwOutfitStats_hasActiveDiveIntent(const daAlink_c* link) {
    if (!dAlbwOutfitStats_isSubmergedHumanSwim(link)) {
        return false;
    }

    if (link->mProcID == daAlink_c::PROC_SWIM_DIVE) {
        return true;
    }

    const f32 depth = link->mWaterY - link->current.pos.y;

    // At / near the waterline, holding A is shore swim — not dive sustain.
    if (depth <= kAlbwOutfitStatsWaterlineSoftDepth) {
        return false;
    }

    // Clearly below the float-up band: stay submerged even if stroke briefly drops.
    if (depth > link->mpHIO->mSwim.m.mFloatUpHeight) {
        return true;
    }

    // Mid-depth: only while stroke / swim button is active.
    if (link->field_0x3000 != 0) {
        return true;
    }

    daAlink_c* mut = const_cast<daAlink_c*>(link);
    return mut->checkSwimButtonMove() || mut->checkZoraSwimMove();
}

bool dAlbwOutfitStats_isZoraWaterBuffActive() {
    if (!dAlbwOutfitStats_isEnabled()) {
        return false;
    }

    if (dAlbwOutfitStats_getActiveOutfitKind() != D_ALBW_OUTFIT_ZORA) {
        return false;
    }

    return sZoraGraceTimer > 0;
}

int dAlbwOutfitStats_getZoraGraceFramesRemaining() {
    if (!dAlbwOutfitStats_isEnabled()) {
        return 0;
    }

    if (dAlbwOutfitStats_getActiveOutfitKind() != D_ALBW_OUTFIT_ZORA) {
        return 0;
    }

    return sZoraGraceTimer;
}

void dAlbwOutfitStats_updateSwimState(daAlink_c* link) {
    if (!dAlbwOutfitStats_isEnabled() || link == NULL) {
        sZoraGraceTimer = 0;
        return;
    }

    if (dAlbwOutfitStats_getActiveOutfitKind() != D_ALBW_OUTFIT_ZORA) {
        sZoraGraceTimer = 0;
        return;
    }

    const bool submerged = link->checkModeFlg(daAlink_c::MODE_SWIMMING) &&
                           link->mWaterY > 5.0f + link->current.pos.y;

    if (submerged) {
        sZoraGraceTimer = kAlbwOutfitStatsZoraGraceFrames;
    } else if (sZoraGraceTimer > 0) {
        sZoraGraceTimer--;
    }
}

#endif  // TARGET_PC
