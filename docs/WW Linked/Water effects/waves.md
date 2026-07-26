# Waves — whitecap foam panes (system 2)

Per-effect doc for the kankyo wave packet (the offshore whitecap panes). Part of the `Water effects/`
set; taxonomy + shared traps live in the parent [../water-rendering.md](../water-rendering.md).
Siblings: [shore-crashing.md](shore-crashing.md) (system 4), great-sea (system 1, future).

## STATUS: color ACCEPTED; MOTION discrepancy open (user eye, 2026-07-25)
Color is right (blue base + white tips). **But the panes only BOB vertically — missing the donor's
horizontal wind DRIFT + SKEW (§133).**
**Blue waves with white pointed tips are IN.** The §98 recipe + palette fix + §109 calm-perimeter +
§120 wave-color thread all landed; user confirms the panes render as sea-blue trapezoids with white
crests in the correct offshore perimeter band, dead at the immediate waterline, calm near shore.

## Mechanism (verified — see parent doc §2 row + bus §98/§109/§119)
- ~300 pulsing camera-facing trapezoid QUADS; sin-phase gated; sea-color TEV lerp through the
  usonami texture (`d_kankyo_wether/rain`, `drawWave` `d_kankyo_rain.cpp:3281`).
- Color = kankyo sea palette (`dKy_get_seacolor` amb/dif lerp); the grayscale usonami tex is the
  MIXING CONTROL, never drawn directly. White-triangle failures were palette (`BG1_K0` dropped),
  not texture — resolved.
- Placement: donor two-tier — flatInter island-proximity gradient (100k cell + 12800 ramp) +
  per-shore `mpWaveInfl` kill (donor ~5000/6000); hosted map `population/wave_calm.ini`
  (coast-traced infls, §109/§111).

## Open — MOTION (§133): bob-only, missing wind-driven horizontal drift
Donor pane motion (`wave_move`, `d_kankyo_rain.cpp:1416-1418` + drawWave skew) has TWO parts:
- **Vertical pulse (we HAVE):** `mCounter += mCounterSpeed; sin(mCounter)` → grow/shrink. Wind-
  INDEPENDENT, so it runs regardless → our "bob up/down."
- **Horizontal drift + skew (we're MISSING):** `mPos.x/z += windPowVec.x/z * mWaveSpeed * mSpeed * …`
  (sprites drift in wind dir) + `mSkewWidth = windPow * …` (top verts lean). BOTH wind-driven.
**SHARED-ROOT — CONFIRMED by code read (§134):** skew+drift code IS present (`6737`/`7029`); windPow≈0 because WW wind = `dComIfGs_getWindX/Y` (Wind-Waker state) which TP host lacks. Was hypothesis, now: if the host stage's kankyo wind is zero (neutral F_DL01,
no wind config found; we feed none), the drift+skew vanish → bob-only. The SAME zero-wind would also
kill grass sway (§130 HUNT 1, `dKyw_get_wind_pow`). **ONE probe settles it:** log
`dKyw_get_wind_pow()` on F_DL01 — zero → feed kankyo wind (fixes pane drift AND grass sway together);
non-zero → effects read wind, drift blocked elsewhere. Perimeter radii still user-tunable in
`wave_calm.ini`.

## Resource landing (user posting, 2026-07-25)
> _Incoming wave resource — paste/link here when posted. Mark epistemic tier on arrival
> ([verified]/[video]/[recreation]) per the parent doc's source ladder._
