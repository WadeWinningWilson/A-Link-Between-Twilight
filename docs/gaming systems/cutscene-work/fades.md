# Fades & letterboxing in cutscenes

> **Status: LIGHTLY EXPLORED.** Fades were not a blocker for the tale/awake work, so this doc records
> what we observed and where the donor mechanism lives, but it is **not** a deeply-verified area yet.
> Flagged honestly so no one treats it as settled.

## ① What the donor does (pointers, not yet fully traced)

- **Letterboxing** — the black bars are part of the demo/event presentation. We saw them engage
  correctly on both cutscenes ("the screen did start to have its letterboxing, so something wants to
  work"), which was an early sign the demo pipeline was firing before the message sync was fixed.
- **Fades** — WW uses `JUTFader` for screen fade-in/out; the port already exposes fader status
  (`fader->getStatus()` with `JUTFader::FadeOut`/`FadeIn`/`Wait`/`None`), used in the mount arrival/room
  logic in `src/d/d_ext_npc_mount.cpp`. Cutscene fades ride the same fader.
- **Scene-specific fades** — the user noted the two **Ordon** NPCs used **short fade-to-blacks between
  them** as part of the *missing-children* quest — explicitly **not the norm** for ordinary dialogue.
  This is a reminder that fade behavior is **per-event**, authored, not a global rule.

## ② Traps / cautions

- **Don't assume a fade means a scene boundary.** The Ordon fade-to-blacks were quest-specific; a fade
  is authored per event, not a universal "cutscene start/end" signal.
- **Letterbox present ≠ demo working.** Letterboxing engaged even while the message sync was completely
  broken. It indicates the demo *presentation* started, not that the storyboard is healthy.

## ③ Correct method (as far as we know)

Let the event/demo pipeline own fades and letterboxing (they already fire correctly); do not hand-roll
fade timing around the message system. If a specific cutscene's fade is wrong, trace it to that event's
authored fade, not to a global fader change.

## ④ Imperfect / open

- We have **not** verified cutscene fade **timing** against the donor frame-for-frame.
- The interaction between a suspended storyboard (message hold) and an in-progress fade is untested — if
  a fade is mid-transition when a box suspends the timeline, behavior is unverified.
- The tale/awake letterbox in/out timing has not been compared to WW.
