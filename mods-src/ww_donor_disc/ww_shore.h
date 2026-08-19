#pragma once

// Shore crash BTK (water taxonomy system 4) — Room44 model1 SC_01_mizu*.
// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_bg.cpp daBg_btkAnm_c entry/play (SC_01 → wave frame);
//            d/d_envse.cpp execute type-1 SOND (field_0xf8 0..99 → setWaveFrame)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Vanilla TP daBg plays BTK with EMode_NULL and no wave sync. Donor drives
// SC_01 materials from the global wave timer (100f loop). Plugin owns that
// timer (vanilla has no mWaveFrame) and hooks daBg_btkAnm entry/play.

void wwShore_reset();
void wwShore_tick();  // once per Counter0 on WW sea — donor envse type-1 advance
unsigned wwShore_waveFrame();

// daBg_btkAnm_c hooks (registry). entry POST; play PRE returns true → skip.
void wwShore_onBtkEntry(void* self, void* modelData);
bool wwShore_tryWavePlay(void* self);
