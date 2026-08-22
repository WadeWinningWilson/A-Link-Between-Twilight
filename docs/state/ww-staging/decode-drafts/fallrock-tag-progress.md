# d_a_fallrock_tag decode campaign — FULLY MATCHED

Opened and closed 2026-08-22 in one sitting (same DECODER session as
kg1/mgameboard). **7/7 exact, unit fuzzy 100.00 — the session's first
fully matched TU** (WWDP b59501e2). The falling-rock scheduler tag:
watches the stage STAG schbit schedule and periodically spawns FallRock
actors (proc 0x1A9) at randomized offsets/scales/angles with the
JA_SE_ATM_RAKUBAN rumble.

## Decode facts

- Statics: m_div_num = 6.0f; m_data = {250.0f, 0.3f, 0.8f, -70.0f,
  -7.0f, 90, 90, 3} (spread scale, scale min/max, two unused f32s +
  s16, timer offset 0x16, rate 0x18). getData() returns &m_data
  (retyped from void).
- Execute: t = (int)((u8)dComIfGp_getStageStagInfo()->
  mStageTypeAndSchbit / m_div_num) * 30 — **the anonymous virtual call
  at gameInfo+0x5150 is mStageData (dStage_stageDt_c); play base is
  0x12A0 (dBgS is play's FIRST member — reconciles dComIfG_Bgsp() =
  gameInfo+0x12A0 with mEvtManager 0x52CC = play+0x402C)**.
- Members: mSchBit u8 0x29E (low byte of params), int m298.
- Create: SetupActor + cDyl_LinkASync(fpcNm_FallRock_e) — a
  dynamic-link (cDyl) actor pair: the TAG links the ROCK's module.
  fopAcM_offDraw(this) — the inline that wraps fopDwTg_DrawQTo.

## Levers minted (all confirmed by the 100.00)

1. **Dtor owns the teardown**: Delete = `i_this->~daFallRockTag_c();`
   where the CLASS dtor is `~daFallRockTag_c() { cDyl_Unlink(
   fpcNm_FallRock_e); }` in-class. The mr./beq null guard belongs to
   the PSEUDO-DTOR CALL; the base-dtor call inside the derived dtor is
   unguarded (base-subobject call). Spelling unlink+explicit-dtor in
   Delete's body instead produces a SECOND guard that nothing removes
   (falsified: ->~T(), (*p).~T(), ~base(), empty in-class dtor).
2. **return-in-case**: `switch (state) { case COMPLEATE: work; return
   COMPLEATE; } return state;` keeps `state` DEAD across the work calls
   so it lives in r3 (passthrough return); `break; return state;` makes
   it live-across-calls = an extra saved register.
3. **u8-local flips and.-operand order**: `u8 bit = call(); if (bit &
   field)` puts the call result in rA (and. r0,r3,r0); both direct
   spellings of `call() & field` canonicalize the other way.
4. Reverse-declaration slots again (scl declared before offs before
   ang so offs lands at the lower slot), cache getData()->field in an
   f32 local when the target reuses it across a call, chained
   `scl.x = scl.y = scl.z = v` stores z,y,x.

## Data pointers for the FallRock actor TU (when it opens)

fpcNm_FallRock_e = 0x1A9; the rock receives param via fopAcM_create
with angle/scale from this tag; spawn Y range -70..-7? (m_data fields
0xC/0x10 unused by the tag — likely consumed by the rock reading
getData() cross-TU? watch for m_data references from d_a_fallrock).
