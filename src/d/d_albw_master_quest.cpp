/**
 * d_albw_master_quest.cpp
 * ALBW Master Quest — shop tiers, save-backed purchase counts, heart grant scaling.
 */

#if TARGET_PC

#include "d/d_albw_master_quest.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"
#include "d/d_meter2_info.h"
#include "dusk/settings.h"

namespace {

// Full byte of dSv_event_c::mEvent[] — indices 100–102 (confirmed unused by vanilla).
// 104 = post-soft-cap bonus quarter-pips (103 = Focused Arts; 105 = potion capacity).
static constexpr u16 kHeartShopTierReg      = static_cast<u16>(100 << 8) | 0xFF;
static constexpr u16 kMeterShopTierReg      = static_cast<u16>(101 << 8) | 0xFF;
static constexpr u16 kBonusHalfHeartsReg    = static_cast<u16>(102 << 8) | 0xFF;
static constexpr u16 kBonusQuarterHeartsReg = static_cast<u16>(104 << 8) | 0xFF;

// Soft-cap shop still uses half-heart grants up to this many array tiers.
// Past that: +333 price steps and quarter-heart grants (Phase C2).
static constexpr int kAlbwMQMaxLifeQuarters = 80;  // 20 hearts — piece/container soft stop only

static constexpr int kHeartShopPrices[kAlbwMQHeartShopTiers] = {
    225, 250, 275, 325, 375, 425, 500, 575, 675, 800,
    1000, 2100, 3200, 4000, 5000, 8000, 9999,
};

static constexpr int kMeterShopPrices[kAlbwMQMeterShopTiers] = {
    100, 200, 300, 400, 500, 600, 700, 800, 900, 1000,
    1200, 1400, 1600, 1800, 2000, 2250, 2500, 2750, 3000, 3033,
    3333, 3333, 3333,
};

static u8 readTierReg(u16 reg) {
    return dComIfGs_getEventReg(reg);
}

static void writeTierReg(u16 reg, u8 tier) {
    dComIfGs_setEventReg(reg, tier);
}

static u8 readBonusHalfHearts() {
    return readTierReg(kBonusHalfHeartsReg);
}

static void writeBonusHalfHearts(u8 count) {
    writeTierReg(kBonusHalfHeartsReg, count);
}

static u8 readBonusQuarterHearts() {
    return readTierReg(kBonusQuarterHeartsReg);
}

static void writeBonusQuarterHearts(u8 count) {
    writeTierReg(kBonusQuarterHeartsReg, count);
}

// Half a heart = +2 quarter-pips via bonus half-heart counter (save maxLife untouched).
static void grantHalfHeartMaxCapacity() {
    const u8 halves = readBonusHalfHearts();
    if (halves >= 255) {
        return;
    }
    writeBonusHalfHearts(static_cast<u8>(halves + 1));
    dComIfGp_setItemLifeCount(2.0f, 0);
}

// Quarter heart = +1 quarter-pip (Phase C2 past soft-cap).
static void grantQuarterHeartMaxCapacity() {
    const u8 quarters = readBonusQuarterHearts();
    if (quarters >= 255) {
        return;
    }
    writeBonusQuarterHearts(static_cast<u8>(quarters + 1));
    dComIfGp_setItemLifeCount(1.0f, 0);
}

static bool heartShopPastSoftCap(int tier) {
    return tier >= kAlbwMQHeartShopTiers;
}

}  // namespace

int dAlbwMQ_getBonusMaxLifeQuarters() {
    if (!dAlbwMQ_isEnabled()) {
        return 0;
    }
    return static_cast<int>(readBonusHalfHearts()) * 2 + static_cast<int>(readBonusQuarterHearts());
}

u16 dAlbwMQ_getDisplayMaxLifeInternal() {
    const u16 gauge = dComIfGs_getMaxLifeGauge();
    return static_cast<u16>(((gauge + 3) / 4) * 5);
}

bool dAlbwMQ_isEnabled() {
    return dusk::getSettings().game.masterQuest.getValue();
}

int dAlbwMQ_getHeartShopTier() {
    const int tier = readTierReg(kHeartShopTierReg);
    if (tier < 0) {
        return 0;
    }
    // u8 reg — allow full 0..255 for unlimited post-soft-cap buys (Phase C2).
    return tier;
}

int dAlbwMQ_getMeterShopTier() {
    const int tier = readTierReg(kMeterShopTierReg);
    if (tier < 0) {
        return 0;
    }
    if (tier > kAlbwMQMeterShopTiers) {
        return kAlbwMQMeterShopTiers;
    }
    return tier;
}

int dAlbwMQ_getHeartShopPrice(int tier) {
    if (tier < 0) {
        return 0;
    }
    if (tier < kAlbwMQHeartShopTiers) {
        return kHeartShopPrices[tier];
    }
    // First buy past the 9999 soft-cap: n=1 → 10332; then +333 each.
    return 9999 + 333 * (tier - (kAlbwMQHeartShopTiers - 1));
}

int dAlbwMQ_getMeterShopPrice(int tier) {
    if (tier < 0 || tier >= kAlbwMQMeterShopTiers) {
        return 0;
    }
    return kMeterShopPrices[tier];
}

bool dAlbwMQ_canPurchaseHeartShop() {
    if (!dAlbwMQ_isEnabled()) {
        return false;
    }
    // Phase C2: same Upgrades row stays purchasable past soft-cap / past 20♥.
    // Soft stop only when bonus counters are saturated (u8).
    const int tier = dAlbwMQ_getHeartShopTier();
    if (heartShopPastSoftCap(tier)) {
        return readBonusQuarterHearts() < 255;
    }
    return readBonusHalfHearts() < 255;
}

bool dAlbwMQ_canPurchaseMeterShop() {
    if (!dAlbwMQ_isEnabled()) {
        return false;
    }
    return dAlbwMQ_getMeterShopTier() < kAlbwMQMeterShopTiers;
}

bool dAlbwMQ_tryPurchaseHeartShop() {
    if (!dAlbwMQ_canPurchaseHeartShop()) {
        return false;
    }
    const int tier = dAlbwMQ_getHeartShopTier();
    if (heartShopPastSoftCap(tier)) {
        grantQuarterHeartMaxCapacity();
    } else {
        grantHalfHeartMaxCapacity();
    }
    if (tier < 255) {
        writeTierReg(kHeartShopTierReg, static_cast<u8>(tier + 1));
    }
    return true;
}

bool dAlbwMQ_tryPurchaseMeterShop() {
    if (!dAlbwMQ_canPurchaseMeterShop()) {
        return false;
    }
    writeTierReg(kMeterShopTierReg, static_cast<u8>(dAlbwMQ_getMeterShopTier() + 1));
    dMeter2_onALBWMeterShopPurchase();
    return true;
}

void dAlbwMQ_applyPieceHeartGrant() {
    if (!dAlbwMQ_isEnabled() || dComIfGs_getMaxLifeGauge() + 2 > kAlbwMQMaxLifeQuarters) {
        return;
    }
    // Fifth piece in a cycle (maxLife % 5 == 4 before grant): half a heart for the set.
    if ((dComIfGs_getMaxLife() % 5) == 4) {
        grantHalfHeartMaxCapacity();
    }
}

void dAlbwMQ_applyContainerHeartGrant() {
    if (!dAlbwMQ_isEnabled() || dComIfGs_getMaxLifeGauge() + 2 > kAlbwMQMaxLifeQuarters) {
        return;
    }
    grantHalfHeartMaxCapacity();
}

const char* dAlbwMQ_getHeartShopName() {
    return "Heart Upgrade";
}

const char* dAlbwMQ_getMeterShopName() {
    return "Stamina Upgrade";
}

const char* dAlbwMQ_getHeartShopDesc() {
    if (!dAlbwMQ_canPurchaseHeartShop()) {
        return "Sold out.";
    }
    if (heartShopPastSoftCap(dAlbwMQ_getHeartShopTier())) {
        return "Permanently increases your maximum health by a quarter heart. "
               "These don't grow on trees you know!";
    }
    return "Permanently increases your maximum health by half a heart.";
}

const char* dAlbwMQ_getMeterShopDesc() {
    if (!dAlbwMQ_canPurchaseMeterShop()) {
        return "Sold out.";
    }
    return "Permanently expands your ALBW stamina meter.";
}

#endif
