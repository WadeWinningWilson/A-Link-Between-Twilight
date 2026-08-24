/**
 * d_albw_parry_master.cpp
 * Parry Master — fail chip, base-meter tax, FIFO reclaim + LoP grace/melt.
 */

#include "d/d_albw_parry_master.h"

#if TARGET_PC

#include "d/actor/d_a_alink.h"
#include "d/d_albw_region_mult.h"
#include "d/d_com_inf_game.h"
#include "d/d_meter2_info.h"
#include "dusk/settings.h"

#include <chrono>

namespace {

constexpr f32 kChipFraction = 0.15f;
constexpr f32 kMeterPerAtp = 545.0f;  // 5% of sOilBaseMax (10900) per ATP
constexpr auto kGraceDur = std::chrono::seconds(3);
constexpr int kQueueCap = 32;

struct ChipEntry {
    u16 pieces;
};

ChipEntry sQueue[kQueueCap];
int sQueueHead = 0;
int sQueueCount = 0;

f32 sLivePool = 0.0f;
f32 sPoolAtMeltStart = 0.0f;
std::chrono::steady_clock::time_point sGraceEnd{};
std::chrono::steady_clock::time_point sMeltStart{};
bool sArmed = false;
bool sMelting = false;
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

int sumFifo() {
    int sum = 0;
    for (int i = 0; i < sQueueCount; ++i) {
        sum += sQueue[(sQueueHead + i) % kQueueCap].pieces;
    }
    return sum;
}

void clearQueueInternal() {
    sQueueHead = 0;
    sQueueCount = 0;
    sLivePool = 0.0f;
    sPoolAtMeltStart = 0.0f;
    sArmed = false;
    sMelting = false;
}

void refreshGrace(const std::chrono::steady_clock::time_point& now) {
    sGraceEnd = now + kGraceDur;
    sMelting = false;
}

// Trim oldest FIFO entries so sumFifo() <= floor(sLivePool).
void syncFifoToLivePool() {
    int target = static_cast<int>(sLivePool);
    if (target < 0) {
        target = 0;
    }
    while (sQueueCount > 0 && sumFifo() > target) {
        ChipEntry& e = sQueue[sQueueHead];
        const int excess = sumFifo() - target;
        if (e.pieces <= excess) {
            sQueueHead = (sQueueHead + 1) % kQueueCap;
            sQueueCount--;
        } else {
            e.pieces = static_cast<u16>(e.pieces - excess);
            break;
        }
    }
    if (sQueueCount == 0 || sLivePool <= 0.0f) {
        clearQueueInternal();
    }
}

void pushChip(u16 pieces, const std::chrono::steady_clock::time_point& now) {
    if (pieces == 0) {
        return;
    }
    if (sQueueCount >= kQueueCap) {
        const u16 dropped = sQueue[sQueueHead].pieces;
        sQueueHead = (sQueueHead + 1) % kQueueCap;
        sQueueCount--;
        sLivePool -= static_cast<f32>(dropped);
        if (sLivePool < 0.0f) {
            sLivePool = 0.0f;
        }
    }
    const int idx = (sQueueHead + sQueueCount) % kQueueCap;
    sQueue[idx].pieces = pieces;
    sQueueCount++;
    sLivePool += static_cast<f32>(pieces);
    sArmed = true;
    refreshGrace(now);
}

u16 popOne() {
    if (sQueueCount <= 0) {
        return 0;
    }
    const u16 pieces = sQueue[sQueueHead].pieces;
    sQueueHead = (sQueueHead + 1) % kQueueCap;
    sQueueCount--;
    sLivePool -= static_cast<f32>(pieces);
    if (sLivePool < 0.0f) {
        sLivePool = 0.0f;
    }
    if (sMelting) {
        sPoolAtMeltStart = sLivePool;
        sMeltStart = std::chrono::steady_clock::now();
    }
    if (sQueueCount == 0 || sLivePool <= 0.0f) {
        clearQueueInternal();
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

int dParryMaster_getRecoverablePieces() {
    if (!dParryMaster_isEnabled() || !sArmed) {
        return 0;
    }
    int pieces = static_cast<int>(sLivePool + 0.0001f);
    if (pieces < 0) {
        return 0;
    }
    return pieces;
}

void dParryMaster_update() {
    if (!dParryMaster_isEnabled() || !sArmed) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!sMelting) {
        if (now >= sGraceEnd) {
            sMelting = true;
            sMeltStart = sGraceEnd;
            sPoolAtMeltStart = sLivePool;
        }
        return;
    }

    const f32 meltSec =
        std::chrono::duration<f32>(now - sMeltStart).count();
    if (meltSec >= 6.0f || sPoolAtMeltStart <= 0.0f) {
        clearQueueInternal();
        return;
    }

    sLivePool = sPoolAtMeltStart * (1.0f - meltSec / 6.0f);
    if (sLivePool <= 0.0f) {
        clearQueueInternal();
        return;
    }
    syncFifoToLivePool();
}

void dParryMaster_onFailedBlock(daAlink_c* i_link, int i_atp) {
    if (!dParryMaster_isEnabled() || i_link == NULL || i_atp <= 0) {
        return;
    }
    if (!i_link->checkShieldGet()) {
        return;
    }

    // COVER: chip + meter tax from region-scaled enemy Atp (same as open hit).
    dAlbwRegionMult_DamageScaleScope regionDmg;
    const f32 magnified = static_cast<f32>(i_atp) * i_link->damageMagnification(FALSE, 0);
    const int chip = tenthsRound(magnified * kChipFraction);

    const int meterDrain = static_cast<int>(magnified * kMeterPerAtp + 0.5f);
    if (meterDrain > 0) {
        dMeter2_drainALBWAmount(meterDrain);
    }

    if (chip <= 0) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    sApplyingChip = true;
    dComIfGp_setItemLifeCount(static_cast<f32>(-chip), 0);
    i_link->onResetFlg1(daPy_py_c::RFLG1_DAMAGE_IMPACT);
    pushChip(static_cast<u16>(chip), now);
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
            sLivePool -= static_cast<f32>(entry.pieces);
            sQueueHead = (sQueueHead + 1) % kQueueCap;
            sQueueCount--;
        } else {
            entry.pieces = static_cast<u16>(entry.pieces - remaining);
            sLivePool -= static_cast<f32>(remaining);
            remaining = 0;
        }
    }
    if (sLivePool < 0.0f) {
        sLivePool = 0.0f;
    }
    if (sMelting) {
        sPoolAtMeltStart = sLivePool;
        sMeltStart = std::chrono::steady_clock::now();
    }
    if (sQueueCount == 0 || sLivePool <= 0.0f) {
        clearQueueInternal();
    }
}

#endif  // TARGET_PC
