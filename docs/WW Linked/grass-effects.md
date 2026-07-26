# WW grass effects (ALL islands) — sway + cut/run VFX color

**Island-generic** (believed universal until proven otherwise): grass in WW is one system
(`d_grass.cpp` `dGrass_data_c` — the DATA class, NOT the actor shell `d_a_grass.cpp`). The
mechanisms below are kankyo/room-driven, not per-island — but per-island values (wind power, room
tevstr) vary, so verify per space. Identification done (bus §130); **not yet built — parked behind
shore work.** Our current grass renders STATIC with BLACK cut/run VFX.

## 1. GRASS SWAY (blowing) — [verified] `d_grass.cpp:322-329`
Wind-driven per-blade tilt. Located in the grass DATA class (why §95b, reading only the actor shell
`d_a_grass.cpp`, missed it):
```c
windSpeed = dKyw_get_wind_pow() * 1000.0f + 1000.0f;   // kankyo wind power
windSpeed = cLib_maxLimit(windSpeed, 2000.0f);          // clamp
anm->mRotX = windSpeed + windSpeed * cM_scos(windSpeed * (g_Counter.mTimer + i*250));  // per-blade
```
- Each blade `i` gets `mRotX` (X-tilt) = a cosine of `windSpeed × time`, with per-blade phase
  `i*250` — that phase offset is what makes the field ripple rather than move as one.
- Wind source: `dKyw_get_wind_pow()` (the kankyo wind system — the receiver has wind).
- **Our port has no `mRotX` animation → static grass.** Fix = drive per-blade `mRotX` by this
  formula from `dKyw_get_wind_pow()`. Depends on kankyo wind being live on the host stage.
- Feasibility: MEDIUM — small extension of hot grass code once wind is confirmed feeding through.

## 2. CUT/RUN VFX COLOR (currently BLACK) — [verified] `d_grass.cpp:80,153`
The grass run-through + cut scatter particles take their color from the ROOM's tev-str K0:
```c
dKy_tevstr_c* tevStr = dComIfGp_roomControl_getTevStr(roomNo);
dComIfGp_particle_setSimple(getKusaRunPID()/getKusaKenPID(), &pos, 0xFF,
                            tevStr->mColorK0, tevStr->mColorK0, 1);
```
- **BLACK VFX = our room's `tevStr->mColorK0` is (0,0,0)/unset.** The particle is fine; the color
  SOURCE isn't populated.
- **SAME BUG FAMILY as shore water (§126/§127) + wave color (§112):** a color source (room tevstr K0
  / kankyo K0 / donor-authored) not reaching the material/particle. Shared fix pattern = the §47
  donor-look lane (the WW room's tevstr must carry the donor K0). Env particles broadly pull color
  from kankyo K0 (e.g. kamome `setGlobalPrmColor(mBG0_K0)`, `d_kankyo_rain.cpp:220`).
- Fix: ensure the WW room's `dKy_tevstr_c::mColorK0` is the donor value, not black. Likely one fix
  clears grass VFX AND is adjacent to the shore/wave color work.

## Status / routing
- Both LOCATED (§130); neither built. Parked behind Engine's shore-motion (§128).
- **History/Engine (when unparked):** sway = apply the mRotX formula; VFX = fix room tevstr K0.
- **Housing:** covenant-neutral (render/color fidelity, no purity surface). No gate.
Cross-refs: bus §95b/§130 · [water-rendering.md](water-rendering.md) (same color-source family) ·
[wind-effects.md](wind-effects.md) (sway depends on kankyo wind, shared with wind streaks).
