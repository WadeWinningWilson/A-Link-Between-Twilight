# Grandma tale — DuskTap golden trace, run 1 (donor ground truth, live)

**Lane: Foundry.** Captured 2026-08-01 from the DONOR US retail ISO in the user's local
Dolphin build via `tools/foundry/dusktap_dmesg.py` (§316; symbol-exact probes, read-only,
120 Hz). Raw CSV: `ww-arc-staging/dusktap_tale_run1.csv` (72 transition rows, entry →
nine boxes → teardown → control return → walk out the door). This document is the
oracle History diffs the port against.

## 1. CORRECTIONS the live data forced

**Font size is 23, not 25.** Every box in the run shows `mNowFontSize = 23`. Root cause
of my §311 error: `DEMO_SELECT(DEMO, RETAIL)` selects the SECOND argument on retail
(global.h:67), so `field_0x70 = DEMO_SELECT(0x19, 0x17)` → retail **0x17 = 23** (the
kiosk demo is the 25). History's `kWwFontSize = 25.0f` is +2pt — a strong candidate for
the user's "formatting sometimes off." Observed alongside: `charSpace = 0`,
`boxWidth = 486` (constant all run).

**The suspend counter goes NEGATIVE by design.** Box 544: the player dismissed it fast —
box close fired `unsuspend(1)` BEFORE the control track reached 544's authored
`suspend(1)` → counter observed at **−1** (frame 1559), then the authored suspend
executed and cancelled it back to 0 — **no hold, no deadlock**. The storyboard clock
(`mFrameNoMsg` 533→569) never paused. PORT REQUIREMENT: the suspend cache is a signed
counter and `isSuspended()` is `> 0` — any clamp-at-zero or assert on negative breaks
fast readers (early-dismiss double-hold or deadlock).

## 2. The two clocks — §304's diagnosis confirmed live

`mFrameNoMsg` is the STORYBOARD clock: each box's message op landed at exactly the §304
static-decode frames — 3095@~145, 541@198, 542@247, 547@272, 543@315, 544@467, 545@620.
`mFrame` is wall-clock (kept counting through every hold: e.g. 539's hold spans mFrame
30→161 while mFrameNoMsg sits at 29-30). Any port log keyed on the wall counter reads
"198" for reasons unrelated to storyboard position — as diagnosed.

## 3. Box-by-box (order, holds, pagination)

Order observed = §304's timeline exactly: **539 → 540 → 3095 → 541 → 542 → 547 → 543 →
544 → 545.** One suspend(1)/release cycle per box (544's cancelled-negative cycle
included). Page turns happen WHILE suspended — box-internal pagination never touches the
control counter; only the final close releases.

| box | line-count sequence (per page, as measured live) |
|---|---|
| 539 | 4 → 2 (two pages) |
| 540 | 2 (one page) |
| 3095 (item-get, d09 box) | 3 (one page) |
| 541 | 4 → 4 (new lines) → 2 (three pages) |
| 542 | 4 (one page) |
| 547 | 4 → 3 (two pages) |
| 543 | 3 (one page) |
| 544 | 1 (one page) |
| 545 | 4 → 4 → 4 → 1 (four pages, final single line) |

(`mLineLength[4]` per page is in the CSV — 243.0 entries are unused/full-sentinel slots;
use the CSV values verbatim as the wrap oracle when diffing the port's pagination.)

## 4. Lifecycle — entry, in-place talk, teardown, control return

* **Entry**: door event (evt 27, evt_flag 16) → Link climbs to the platform → tale event
  order (evt 73) with Link standing at **(−289, 375, 83)** — the talk spot — then a
  ~0.5 s window where the demo manager pointer is INVALID (the stage reload of the
  donor two-step), then the STB starts and teleports Link to the storyboard start
  (−341, 375, 250) — tale.stb frame-0 authored position, byte-exact.
* **The continuity trick**: the STB's END transform (−289, 375, 83)/0x8000 (§315) is the
  SAME SPOT Link stood to start the talk. The donor "returns" Link by ending the
  storyboard where he began — no warp anywhere.
* **Teardown — CORRECTED per §322/§323 (this section's first reading was an instrument
  blind spot):** 545 releases (mFrame 2030) → `demo_mode` 1→2 → **a SAME-STAGE RELOAD IS
  the teardown** (archetype A: `dEvDt_Next_Stage` fires unconditionally — the donor has no
  same-stage no-op). The demo-manager-invalid window at 004.7–005.1 was the reload; **"evt
  49" was `DEFAULT_START` — the reload's fingerprint**, not a "post-tale handoff." The
  original conclusion "no respawn, no reload at exit" was WRONG: a same-stage reload behind
  an already-black screen, landing on a spawn placed AT the STB end transform, is invisible
  to a transform probe. The landing spawn — (−290, 375, 85) @0x8000, event byte 0xff — IS
  the donor's continuity trick; the arrival wipe-in is what undoes the STB's authored fade.
  §315's acceptance number stands (Link ends at the talk spot), but the MECHANISM is
  reload-with-continuity-spawn, not in-place return. Method lesson: cross-check event ids
  against the stage event table before trusting a negative (bus §323.5.2).
* After control return the player walked downstairs and out (evt 25 at the door,
  ground-floor coords) — normal play, no residue.

## 5. Port-diff checklist (what History tests against this oracle)

1. Font 23 / charSpace 0 / boxWidth 486 in every box (swap kWwFontSize).
2. Signed suspend counter; early dismiss must produce the −1→0 cancel, never a clamp.
3. Storyboard-clock message frames per §304; wall-clock never used for sequencing.
4. Page-turn-while-suspended; single release per box at final close.
5. Teardown: mode-2 end phase, then control return AT the STB end transform, no respawn.
6. Item-get 3095 shows the d09 box mid-tale with 3 lines and the same hold contract.
