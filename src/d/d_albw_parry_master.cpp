/**
 * d_albw_parry_master.cpp
 * Parry Master — fail chip, base-meter tax, FIFO reclaim queue.
 */

#include "d/d_albw_parry_master.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_meter2_info.h"
#include "dusk/settings.h"

#include <chrono>

namespace {

constexpr f32 kChipFraction = 0.15f;
constexpr f32 kMeterPerAtp = 545.0f;  // 5% of sOilBaseMax (10900) per ATP
constexpr auto kQueueTtl = std::chrono::seconds(6);
constexpr int kQueueCap = 32;

struct ChipEntry {
    u16 pieces;
};

ChipEntry sQueue[kQueueCap];
int sQueueHead = 0;
int sQueueCount = 0;
std::chrono::steady_clock::time_point sQueueExpire{};
bool sQueueArmed = false;
bool sApplyingChip = false;
bool sApplyingReclaim = false;

int tenthsRound(f32 value) {
    int whole = static_cast<int>(value);
    if (static_cast<int>(value * 10.0f) % 10 != 0) {
        whole++;
    }
    if (whole < 0) {
        return 0;
    }
    return whole;
}

void clearQueueInternal() {
    sQueueHead = 0;
    sQueueCount = 0;
    sQueueArmed = false;
}

void pushChip(u16 pieces) {
    if (pieces == 0) {
        return;
    }
    if (sQueueCount >= kQueueCap) {
        sQueueHead = (sQueueHead + 1) % kQueueCap;
        sQueueCount--;
    }
    const int idx = (sQueueHead + sQueueCount) % kQueueCap;
    sQueue[idx].pieces = pieces;
    sQueueCount++;
    sQueueExpire = std::chrono::steady_clock::now() + kQueueTtl;
    sQueueArmed = true;
}

u16 popOne() {
    if (sQueueCount <= 0) {
        return 0;
    }
    const u16 pieces = sQueue[sQueueHead].pieces;
    sQueueHead = (sQueueHead + 1) % kQueueCap;
    sQueueCount--;
    if (sQueueCount == 0) {
        sQueueArmed = false;
    } else {
        sQueueExpire = std::chrono::steady_clock::now() + kQueueTtl;
    }
    return pieces;
}

void reclaimInputs(int count) {
    sApplyingReclaim = true;
    for (int i = 0; i < count; ++i) {
        const u16 pieces = popOne();
        if (pieces == 0) {
            break;
        }
        dComIfGp_setItemLifeCount(static_cast<f32>(pieces), 0);
    }
    sApplyingReclaim = false;
}

}  // namespace

bool dParryMaster_isEnabled() {
    if (!dusk::getSettings().game.parryMaster.getValue()) {
        return false;
    }
    // Economy requires Shield Parry perfect/fail classification.
    return dusk::getSettings().game.shieldParryCombat.getValue();
}

void dParryMaster_resetSession() {
    clearQueueInternal();
    sApplyingChip = false;
    sApplyingReclaim = false;
}

void dParryMaster_clearQueue() {
    clearQueueInternal();
}

void dParryMaster_update() {
    if (!dParryMaster_isEnabled() || !sQueueArmed) {
        return;
    }
    if (std::chrono::steady_clock::now() >= sQueueExpire) {
        clearQueueInternal();
    }
}

void dParryMaster_onFailedBlock(daAlink_c* i_link, int i_atp) {
    if (!dParryMaster_isEnabled() || i_link == NULL || i_atp <= 0) {
        return;
    }
    if (!i_link->checkShieldGet()) {
        return;
    }

    // Post-scale damage, then ×0.15, then tenths (same rule as setDamagePoint).
    const f32 magnified = static_cast<f32>(i_atp) * i_link->damageMagnification(FALSE, 0);
    const int chip = tenthsRound(magnified * kChipFraction);

    // Meter tax: round(effAtp × 545) pinned to base capacity fractions.
    const int meterDrain = static_cast<int>(magnified * kMeterPerAtp + 0.5f);
    if (meterDrain > 0) {
        dMeter2_drainALBWAmount(meterDrain);
    }

    if (chip <= 0) {
        return;
    }

    sApplyingChip = true;
    dComIfGp_setItemLifeCount(static_cast<f32>(-chip), 0);
    i_link->onResetFlg1(daPy_py_c::RFLG1_DAMAGE_IMPACT);
    pushChip(static_cast<u16>(chip));
    sApplyingChip = false;
}

void dParryMaster_onPerfectParry() {
    if (!dParryMaster_isEnabled()) {
        return;
    }
    reclaimInputs(2);
}

void dParryMaster_onDealtDamage() {
    if (!dParryMaster_isEnabled()) {
        return;
    }
    reclaimInputs(1);
}

void dParryMaster_onHpLoss(int i_pieces) {
    if (!dParryMaster_isEnabled() || i_pieces <= 0 || sApplyingChip) {
        return;
    }
    clearQueueInternal();
}

void dParryMaster_onHeal(int i_pieces) {
    if (!dParryMaster_isEnabled() || i_pieces <= 0 || sQueueCount <= 0 || sApplyingReclaim) {
        return;
    }

    int remaining = i_pieces;
    while (remaining > 0 && sQueueCount > 0) {
        ChipEntry& entry = sQueue[sQueueHead];
        if (entry.pieces <= remaining) {
            remaining -= entry.pieces;
            popOne();
        } else {
            entry.pieces = static_cast<u16>(entry.pieces - remaining);
            remaining = 0;
            sQueueExpire = std::chrono::steady_clock::now() + kQueueTtl;
        }
    }
}

#endif  // TARGET_PC
