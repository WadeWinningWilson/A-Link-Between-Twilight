/**
 * d_albw_sword_atp.cpp
 * ALBW Master Quest — per-sword Atp shop (regs 106–113).
 */

#if TARGET_PC

#include "d/d_albw_sword_atp.h"
#include "d/d_albw_master_quest.h"
#include "d/d_com_inf_game.h"
#include "d/d_item_data.h"

#include <cstdio>

namespace {

static constexpr int kTier1Steps = 5;
static constexpr int kTier2Steps = 5;
static constexpr int kTier3Steps = 5;
static constexpr int kTier3EndStep = kTier1Steps + kTier2Steps + kTier3Steps;  // 15

static constexpr int kTier1Prices[kTier1Steps] = {100, 200, 300, 400, 500};

static constexpr int kTier2BugGate[kTier2Steps]  = {4, 6, 8, 10, 12};
static constexpr int kTier2Prices[kTier2Steps]   = {100, 200, 300, 400, 500};

static constexpr int kTier3SoulGate[kTier3Steps] = {5, 12, 20, 27, 35};
static constexpr int kTier3Prices[kTier3Steps]   = {500, 625, 750, 875, 1000};

static constexpr u8 kSwordItemNos[kAlbwSwordAtpCount] = {
    static_cast<u8>(dItemNo_WOOD_STICK_e),
    static_cast<u8>(dItemNo_SWORD_e),
    static_cast<u8>(dItemNo_MASTER_SWORD_e),
    static_cast<u8>(dItemNo_LIGHT_SWORD_e),
};

static constexpr const char* kSwordNames[kAlbwSwordAtpCount] = {
    "Wooden Sword",
    "Ordon Sword",
    "Master Sword",
    "Light Sword",
};

// Event regs 106–109 = Atp bonus u8 ×4; 110–113 = purchase step u8 ×4.
static constexpr u16 kBonusRegBase = static_cast<u16>(106 << 8) | 0xFF;
static constexpr u16 kStepRegBase  = static_cast<u16>(110 << 8) | 0xFF;

static char sDescBuf[kAlbwSwordAtpCount][256];

static u8 readReg(u16 reg) {
    return dComIfGs_getEventReg(reg);
}

static void writeReg(u16 reg, u8 value) {
    dComIfGs_setEventReg(reg, value);
}

static u16 bonusRegFor(int swordId) {
    return static_cast<u16>(kBonusRegBase + swordId);
}

static u16 stepRegFor(int swordId) {
    return static_cast<u16>(kStepRegBase + swordId);
}

static int countInsectFirstBits() {
    static const u8 kInsects[] = {
        dItemNo_M_BEETLE_e,      dItemNo_F_BEETLE_e,      dItemNo_M_BUTTERFLY_e, dItemNo_F_BUTTERFLY_e,
        dItemNo_M_STAG_BEETLE_e, dItemNo_F_STAG_BEETLE_e, dItemNo_M_GRASSHOPPER_e, dItemNo_F_GRASSHOPPER_e,
        dItemNo_M_NANAFUSHI_e,   dItemNo_F_NANAFUSHI_e,   dItemNo_M_DANGOMUSHI_e,  dItemNo_F_DANGOMUSHI_e,
        dItemNo_M_MANTIS_e,      dItemNo_F_MANTIS_e,      dItemNo_M_LADYBUG_e,   dItemNo_F_LADYBUG_e,
        dItemNo_M_SNAIL_e,       dItemNo_F_SNAIL_e,       dItemNo_M_DRAGONFLY_e, dItemNo_F_DRAGONFLY_e,
        dItemNo_M_ANT_e,         dItemNo_F_ANT_e,         dItemNo_M_MAYFLY_e,    dItemNo_F_MAYFLY_e,
    };

    int count = 0;
    for (u8 itemNo : kInsects) {
        if (dComIfGs_isItemFirstBit(itemNo)) {
            count++;
        }
    }
    return count;
}

static bool swordIdValid(int swordId) {
    return swordId >= 0 && swordId < kAlbwSwordAtpCount;
}

static int tierForStep(int step) {
    if (step < kTier1Steps) {
        return 1;
    }
    if (step < kTier1Steps + kTier2Steps) {
        return 2;
    }
    if (step < kTier3EndStep) {
        return 3;
    }
    return 4;
}

static bool meetsGateForStep(int step) {
    if (step < kTier1Steps) {
        return true;
    }
    if (step < kTier1Steps + kTier2Steps) {
        const int idx = step - kTier1Steps;
        return countInsectFirstBits() >= kTier2BugGate[idx];
    }
    if (step < kTier3EndStep) {
        const int idx = step - kTier1Steps - kTier2Steps;
        return dComIfGs_getPohSpiritNum() >= kTier3SoulGate[idx];
    }
    return dComIfGs_getClearCount() > 0;
}

static int gateCountForStep(int step) {
    if (step < kTier1Steps + kTier2Steps && step >= kTier1Steps) {
        return kTier2BugGate[step - kTier1Steps];
    }
    if (step < kTier3EndStep) {
        return kTier3SoulGate[step - kTier1Steps - kTier2Steps];
    }
    return 0;
}

}  // namespace

u8 dAlbwSwordAtp_getItemNo(int swordId) {
    if (!swordIdValid(swordId)) {
        return static_cast<u8>(dItemNo_NONE_e);
    }
    return kSwordItemNos[swordId];
}

bool dAlbwSwordAtp_isSwordPossessed(int swordId) {
    if (!swordIdValid(swordId)) {
        return false;
    }
    return dComIfGs_isItemFirstBit(kSwordItemNos[swordId]);
}

bool dAlbwSwordAtp_pageHasVisibleRows() {
    if (!dAlbwMQ_isEnabled()) {
        return false;
    }
    for (int i = 0; i < kAlbwSwordAtpCount; ++i) {
        if (dAlbwSwordAtp_isSwordPossessed(i)) {
            return true;
        }
    }
    return false;
}

int dAlbwSwordAtp_getBonus(int swordId) {
    if (!swordIdValid(swordId)) {
        return 0;
    }
    return readReg(bonusRegFor(swordId));
}

int dAlbwSwordAtp_getBonusForEquipped() {
    if (!dAlbwMQ_isEnabled()) {
        return 0;
    }
    const u8 sword = dComIfGs_getSelectEquipSword();
    for (int i = 0; i < kAlbwSwordAtpCount; ++i) {
        if (kSwordItemNos[i] == sword) {
            return dAlbwSwordAtp_getBonus(i);
        }
    }
    return 0;
}

int dAlbwSwordAtp_getShopStep(int swordId) {
    if (!swordIdValid(swordId)) {
        return 0;
    }
    return readReg(stepRegFor(swordId));
}

int dAlbwSwordAtp_getShopPrice(int swordId) {
    if (!swordIdValid(swordId)) {
        return 0;
    }
    const int step = dAlbwSwordAtp_getShopStep(swordId);
    if (step < kTier1Steps) {
        return kTier1Prices[step];
    }
    if (step < kTier1Steps + kTier2Steps) {
        return kTier2Prices[step - kTier1Steps];
    }
    if (step < kTier3EndStep) {
        return kTier3Prices[step - kTier1Steps - kTier2Steps];
    }
    return kTier1Prices[(step - kTier3EndStep) % kTier1Steps];
}

bool dAlbwSwordAtp_canPurchase(int swordId) {
    if (!dAlbwMQ_isEnabled() || !dAlbwSwordAtp_isSwordPossessed(swordId)) {
        return false;
    }
    const int step  = dAlbwSwordAtp_getShopStep(swordId);
    const int bonus = dAlbwSwordAtp_getBonus(swordId);
    if (bonus >= 255) {
        return false;
    }
    if (step >= kTier3EndStep && dComIfGs_getClearCount() == 0) {
        return false;
    }
    if (step >= kTier3EndStep && bonus >= 254) {
        return false;
    }
    return true;
}

bool dAlbwSwordAtp_tryPurchase(int swordId) {
    if (!dAlbwSwordAtp_canPurchase(swordId)) {
        return false;
    }
    const int step = dAlbwSwordAtp_getShopStep(swordId);
    if (!meetsGateForStep(step)) {
        return false;
    }
    const int gain = (step >= kTier3EndStep) ? 2 : 1;
    const int bonus = dAlbwSwordAtp_getBonus(swordId);
    if (bonus + gain > 255) {
        return false;
    }
    writeReg(bonusRegFor(swordId), static_cast<u8>(bonus + gain));
    if (step < 255) {
        writeReg(stepRegFor(swordId), static_cast<u8>(step + 1));
    }
    return true;
}

const char* dAlbwSwordAtp_getShopName(int swordId) {
    if (!swordIdValid(swordId)) {
        return "";
    }
    return kSwordNames[swordId];
}

const char* dAlbwSwordAtp_getShopDesc(int swordId) {
    if (!swordIdValid(swordId)) {
        return "";
    }

    char* buf = sDescBuf[swordId];
    const int step = dAlbwSwordAtp_getShopStep(swordId);

    if (!dAlbwSwordAtp_canPurchase(swordId)) {
        if (dAlbwSwordAtp_getBonus(swordId) >= 255) {
            std::snprintf(buf, sizeof(sDescBuf[swordId]), "%s", "Sold out.");
            return buf;
        }
        if (step >= kTier3EndStep && dComIfGs_getClearCount() == 0) {
            std::snprintf(buf, sizeof(sDescBuf[swordId]), "%s",
                          "Sharpening secrets await a second journey.");
            return buf;
        }
    }

    switch (tierForStep(step)) {
    case 1:
    case 4:
        std::snprintf(buf, sizeof(sDescBuf[swordId]), "%s",
                      " I hope you don't mind, but I've been using your house to sleep in every "
                      "now and then, and I found this whetstone. I can sharpen your sword to "
                      "repay your hospitality!");
        break;
    case 2: {
        const int n = gateCountForStep(step);
        std::snprintf(buf, sizeof(sDescBuf[swordId]),
                      " I'm going to let you in on a new tip from a master swordsman in castle "
                      "town, they say if you lather the pheremones of %d bugs, your sword will "
                      "get stronger",
                      n);
        break;
    }
    case 3: {
        const int n = gateCountForStep(step);
        std::snprintf(buf, sizeof(sDescBuf[swordId]),
                      "A gruff resistance member scolded me recently, saying slathering bugs on a "
                      "sword never works. The true secret is refining with %d souls, creepy magic "
                      "but true.",
                      n);
        break;
    }
    default:
        buf[0] = '\0';
        break;
    }
    return buf;
}

#endif
