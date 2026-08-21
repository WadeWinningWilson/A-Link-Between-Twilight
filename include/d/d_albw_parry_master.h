/**
 * d_albw_parry_master.h
 * Parry Master — fail chip HP, base-meter tax, reclaim queue (LoP-style).
 * Setting: game.parryMaster. Speedrun forces Off.
 */

#pragma once

#if TARGET_PC

class daAlink_c;

bool dParryMaster_isEnabled();

void dParryMaster_resetSession();
void dParryMaster_update();

// Normal failed shield block only (not guard-break shatter). atp = GetAtAtp().
void dParryMaster_onFailedBlock(daAlink_c* i_link, int i_atp);

// Perfect parry: reclaim up to 2 queue inputs.
void dParryMaster_onPerfectParry();

// Damaging hit landed on an enemy: reclaim 1 input.
void dParryMaster_onDealtDamage();

// Open HP loss (non-chip): wipe queue. Chip path marks itself so this no-ops.
void dParryMaster_onHpLoss(int i_pieces);

// Potion / heart heal: shrink FIFO by healed pieces.
void dParryMaster_onHeal(int i_pieces);

// Death / 0 HP.
void dParryMaster_clearQueue();

#endif  // TARGET_PC
