/**
 * d_albw_sword_atp.h
 * ALBW Master Quest — per-sword Atp shop upgrades (Phase D).
 */

#pragma once

#if TARGET_PC

static constexpr int kAlbwSwordAtpCount = 4;

enum AlbwSwordAtpId {
    ALBW_SWORD_ATP_WOOD   = 0,
    ALBW_SWORD_ATP_ORDON  = 1,
    ALBW_SWORD_ATP_MASTER = 2,
    ALBW_SWORD_ATP_LIGHT  = 3,
};

u8 dAlbwSwordAtp_getItemNo(int swordId);

bool dAlbwSwordAtp_isSwordPossessed(int swordId);
bool dAlbwSwordAtp_pageHasVisibleRows();

int  dAlbwSwordAtp_getBonus(int swordId);
int  dAlbwSwordAtp_getBonusForEquipped();

int  dAlbwSwordAtp_getShopStep(int swordId);
int  dAlbwSwordAtp_getShopPrice(int swordId);

bool dAlbwSwordAtp_canPurchase(int swordId);
bool dAlbwSwordAtp_tryPurchase(int swordId);

const char* dAlbwSwordAtp_getShopName(int swordId);
const char* dAlbwSwordAtp_getShopDesc(int swordId);

#endif
