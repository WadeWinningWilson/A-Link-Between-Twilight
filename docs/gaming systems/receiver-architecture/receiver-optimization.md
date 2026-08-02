# TP receiver architecture (how WW content runs inside TP)

The "receiver" is the TP-side machinery that lets WW data run in Twilight Princess's engine. This is a
**deliberately different covenant scope** from everything else in this tree, and getting that
distinction right saved us from tracking non-issues as debt.

## The scope rule (Housing §67)

> **Only what the player PERCEIVES must be donor-faithful.** The receiver's *internal architecture* —
> parse timing, model resolvers, cache/pristine machinery, stage gating — is **the port's to design**
> and is **NOT** a liberty.

So: the WW cutscene must *look and behave* like WW (that's tracked, and imperfect, in the cutscene
docs). But *how* TP loads the demo arc, resolves the model, caches it, and injects the text is receiver
engineering — free to differ, as long as the perceived result matches. Do not log receiver mechanisms in
[../../WW Linked/port-liberties.md](../../WW%20Linked/port-liberties.md); do log any *perceived*
deviation they cause.

## The receiver pieces that matter here

- **WW host-stage gating.** `dExtWwSave_isWwHostStage(stage)` (and `dExtWw_isWwDemoStage`) gate the WW
  behavior to WW stages, so native TP demos/dialogue are untouched. Every WW hook in shared code
  (`d_demo.cpp`, `d_msg_object.cpp`) is guarded this way — that guard is what lets us edit shared
  systems without regressing vanilla TP.
- **Consume-time model resolvers.** `dExtNpcMount_acquireDemoModel` → `acquireBgModel` resolves + parses
  demo models **when consumed**, never at arc-mount (DN-3). The per-arc `purgeModelCacheForArc` is
  **erase-only** (the arc owns the buffer). See
  [../models-and-lighting/bdl-bmd-parse-timing.md](../models-and-lighting/bdl-bmd-parse-timing.md).
- **Code-text injection.** The demo/dialogue text bridge (`dMsgObject_setWord` + code-text entry 4900)
  is a receiver mechanism — it lets the native message machine display our **catalog** text without the
  lines existing in the BMG. See [../dialogue-boxes/dialogue-system.md](../dialogue-boxes/dialogue-system.md).
- **Folder-side flags.** Story/arming gates are **folder-side flag strings** (`dExtModFlags`), never raw
  WW `UNK_` bits written into TP save memory. The donor bit each maps to is provenance only.
- **Native-write refusal guard.** `dExtWwSave_refuseNativeWrite` protects TP save state from WW content
  writing where it shouldn't — part of keeping the receiver from corrupting the host.

## Optimization notes (what "optimization" means here)

- **Reuse the game's own machines instead of re-implementing them.** The §201 pivot's whole lesson: the
  native `dMsgObject` state machine already does wait + suspend + unsuspend + box-kind selection. Driving
  it (via `setWord`/`messageSetDemo`) is both more faithful *and* less code than the hand-rolled
  `dMsgFlow_c` poll it replaced. Prefer wiring into an existing native machine over building a parallel
  one.
- **Guard shared edits narrowly.** Changes to `d_demo.cpp`/`d_msg_object.cpp` are gated to WW stages or
  the armed presentation flag so they cannot affect vanilla TP. This is what makes editing the shared
  message/demo core safe.
- **Don't fight data-authored control flow.** The counter-spiral disaster (§196/§197) came from adding
  manual suspend/unsuspend on top of the STB's own data-authored commands. Let the data mechanism own
  what it owns; the receiver's job is to *feed* it (text, model, flags), not to duplicate its bookkeeping.

## ④ Imperfect / open

- The receiver is validated by playtest, not by a formal audit of every guard's coverage.
- `A_mori`-blocked arming (L-3) means one receiver gate (`ba.tale_window` auto-arm) is a stand-in for
  the donor's TagEv chain — see [../region-triggers/region-triggers.md](../region-triggers/region-triggers.md).
