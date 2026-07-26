# Waves — whitecap foam panes (system 2)

Per-effect doc for the kankyo wave packet (the offshore whitecap panes). Part of the `Water effects/`
set; taxonomy + shared traps live in the parent [../water-rendering.md](../water-rendering.md).
Siblings: [shore-crashing.md](shore-crashing.md) (system 4), great-sea (system 1, future).

## STATUS: ACCEPTED (user eye, 2026-07-25)
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

## Open / tuning
- Color final per user eye; no open defect. Perimeter radii user-tunable in `wave_calm.ini`.

## Resource landing (user posting, 2026-07-25)
> _Incoming wave resource — paste/link here when posted. Mark epistemic tier on arrival
> ([verified]/[video]/[recreation]) per the parent doc's source ladder._
