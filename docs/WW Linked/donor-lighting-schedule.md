# Donor lighting schedule (dKyd_Schedule — the time→palette-slot law)

> Foundry §222. Verbatim from `d_kankyo_data.cpp:10-26` (struct
> `{mTimeEnd, mTimeBegin, mPalIdx0, mPalIdx1}` per `d_kankyo_data.h:11-16`).
> Units: donor daytime angle 0-360 (= 24 h → hour = t/15). PalIdx = SLOT into the
> active Colo/pselect `palette_id[8]` (which then picks the Pale palette —
> `donor-palettes-sea.md` holds the full chain). Blend between PalIdx0→PalIdx1
> across the window (change_rate from Colo).

## `l_time_attribute` (field/normal)

| t range | hours | slots (from→to) | reading |
|---|---|---|---|
| 0–90 | 00:00–06:00 | 5→5 | deep night |
| 90–105 | 06:00–07:00 | 5→0 | dawn blend |
| 105–120 | 07:00–08:00 | 0→1 | sunrise |
| 120–150 | 08:00–10:00 | 1→2 | morning |
| 150–270 | 10:00–18:00 | 2→2 | day |
| 270–285 | 18:00–19:00 | 2→3 | sunset in |
| 285–300 | 19:00–20:00 | 3→4 | sunset out |
| 300–315 | 20:00–21:00 | 4→5 | dusk |
| 315–360 | 21:00–24:00 | 5→5 | night |

## `l_time_attribute_boss`

0–45: 0→1 · 45–90: 1→2 · 90–180: 2→3 · 180–225: 3→4 · 225–270: 4→5 · 270–360: 5→0
(bosses cycle the six slots on their own clock).

## `l_time_attribute_menu`

0–60: 5→5 · 60–75: 5→0 · 75–90: 0→0 · 90–105: 0→1 · 105–150: 1→2 · 150–225: 2→2 ·
225–240: 2→3 · 240–255: 3→4 · 255–270: 4→5 · 270–360: 5→5.

**P14 status: the donor lighting law is now COMPLETE end-to-end** — EnvR (weather) →
Colo/pselect (time slots + change_rate) → this schedule (hour → slot pair + blend) →
Pale palettes → colors (+ Virt skies). The receiver differ needs one kankyo tap
(current palette indices + blend ratio + output colors) — Engine-queue, emitter-tap
pattern; then purple-black-class bugs become table lookups.
