# Storyboard / STB structure

A WW cutscene is a **JStudio STB** (storyboard) — a set of parallel tracks the demo manager plays. The
tale/awake scenes we restored are STBs inside `Demo01.arc`.

## ① Tracks (what a demo STB carries)

- **Control track** (`mObject_control`) — sequence commands incl. the **`suspend`** opcode (**type 4**,
  `stb.cpp:186-190`). Sign-extended value (`stb.h:84-89`) → the STB can author its own `unsuspend`
  (negative type-4). This is what freezes the timeline for a dialogue box (see
  [demo-message-step-in-step.md](demo-message-step-in-step.md)).
- **JMSG (message) track** — fires message codes at authored frames via `op19/cmd66 type 0x859`. The
  adaptor calls `dMsgObject_setDemoMessage(code)` (`d_demo.cpp:114`); the port intercepts at `:113` with
  `dExtWw_handleDemoMessage`.
- **Camera track** — runs the demo camera; this is the long clock (≈1129 frames for tale) that the
  storyboard end should not fall far short of.
- **Actor tracks** — per-demo-actor transforms/anim, consumed by the read-back (see
  [actor-driving.md](actor-driving.md)).

## Frame counters (learn these — they mislead)

`dDemo_c::update` (`d_demo.cpp:1178-1183`):
```cpp
if (m_control->forward(1) != 0) {
    m_frame++;                                 // getFrame() — raw, ALWAYS advances (camera clock)
    if (m_control->getSuspend() <= 0) m_frameNoMsg++;   // getFrameNoMsg() — FROZEN while suspended
}
```
`m_frame` climbing while `m_frameNoMsg` is stuck is **normal** during a suspend — it is not evidence the
cutscene is playing through. Only object motion is frozen by the bail. Events time off
`getFrameNoMsg` (e.g. the awake stage-name toggles at `getFrameNoMsg` 200/350, `d_menu_window.cpp:789-793`).

## tale.stb layout (JMSG track, decoded with the jstudio tool)

```
tale.stb   @145 → msg 3095 (Hero's Clothes GET)   @467 → 544   @620 → 545
tale_2.stb @145 → msg 4410 (already-have variant)  @467 → 544   @620 → 545
```
`tale` and `tale_2` are **story-state variants, not a chain** — the donor plays ONE based on the
fuku-owned flag. Our early `next_event=TALE_DEMO2` chain played BOTH ("cutscene twice") and was removed.

## ② Traps

- **TRAP — truncation reads as a natural end.** The storyboard ending at frame ~179/855 while the camera
  track runs 1129+ is a **truncation**, not a finish (`d_menu_window.cpp:789-793` proves the awake
  `getFrameNoMsg` reaches ≥350). Root cause was the auto-advance timer racing the boxes; fixed by the
  step-in-step pivot.
- **TRAP — SCOB/STB scale byte convention.** WW SCOB scale bytes are `byte/10` (byte 10 → float 1.0).
  Mis-scaling `byte×100` produced a 10× geometry error in the tale trigger — see
  [../region-triggers/region-triggers.md](../region-triggers/region-triggers.md).

## Tooling

`tools/ww_crew_restoration_skeleton/jstudio_stb.py <arc> <name>` dumps an STB's tracks (control/JMSG/
camera) — how we read the message frames and confirmed the tale/tale_2 variants. noclip.website's WW
`d_demo.ts` is a useful reference for STB structure (but has no story layers — superset only).

## ③ Correct method

Let the STB drive: `forward(1)` every frame, the control track fires its own suspend/unsuspend, the JMSG
track fires message codes we route to the **native** message path. Do not gate `forward` externally
(that was the §195 mistake) and do not hand-author suspends (that was §196/§197).

## ④ Imperfect

STB **timing/wait semantics per message** are not yet honored (see the step-in-step doc §④). We also
have not re-authored WW's own page-break control codes (dropped at extraction), so multi-page splitting
is done by TP box rules.
