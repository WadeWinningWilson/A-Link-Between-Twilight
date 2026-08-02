# KNOWN BUGS — open defects with armed instrumentation

Living registry of confirmed-but-unfixed defects. Each entry carries: symptoms, everything the
probe campaigns established/eliminated, the **passive probes armed in the current build** (so any
future occurrence self-documents in the log — no dedicated hunt runs needed), suspected causes,
and the next diagnostic step. Keep entries updated when a probe fires; move to the bus/live-state
on fix.

---

## KB-1 — Intermittent mid-cutscene KILL (the "get-item break") — OPEN, armed

**First seen:** tale (Grandma) era, recurring across sessions. **Frequency:** intermittent —
most tale runs are clean; strikes roughly 1-in-3 to 1-in-5 runs, historically at the get-item
beat region.

### Symptoms (user-observed, multiple sessions)
- The tale cutscene dies mid-flight, most often at/near the **get-item (Hero's Clothes) beat**
  (storyboard fnm ≈ 98–144, between message boxes 540 and 3095).
- **Full control returns to the player** — Link can walk the entire room freely.
- The dead event's **camera and letterboxes stay frozen** on the scene (one session: fixed on a
  Hero's Clothes model suspended mid-air from the interrupted give).
- After the kill, subsequent interactions (door use, talking) work — the event system recovers;
  only the killed cutscene's presentation lingers until the next event/reload.
- Distinct from KB-2 (below): this is a REAL event kill with real control return.

### Established by the probe campaigns (§341–§353, 2026-08-01)
- The kill is a **direct `dEvt_control_c::remove()`** on the **global** event-control instance
  while `mEventStatus==1` and the STB is mid-flight (`demoMode=1`, e.g. fnm=106).
  Log fingerprint: `§345a dEvt remove() while status=1 runEvt='TALE_DEMO' this=<global> gFrm=N`
  followed by `§341a status 1→0`.
- **Eliminated with receipts:** early finish flags (H1), early §322 exit (nextStage=0 at kill),
  STB stall (§341b silent), demo end()/remove() (§341c silent at that frame), eventFlag 8,
  suspend desync, message-kill, event-CHANGE grant (§344 never fired), reset() re-entrance
  order (§345b), arbitration preemption (gate is donor-true, d_event.cpp:1111), the
  dEvt ctor path (§353 never fired at runtime), a phantom second instance (§345a `this=` is
  always the global).
- The **only compiled caller** of the remove wrapper is the scene-delete path
  (d_s_play.cpp:967, §352b-stamped) — yet kill-time §345a lines appear **without an adjacent
  §352b stamp**, while legitimate stage loads always show the §352b+§345a pair. The same
  stamp-less pattern also fires around some door uses (KNOB_START) without visible harm.

### Suspected causes (open, in likelihood order)
1. **§352b/§345a pairing-at-distance:** the scene-delete function's execution may interleave
   with other logging such that the stamp and the remove land far apart in the log (or a phase
   re-entry runs the tail of the delete without the head). Verification: the pairing counter —
   compare total §352b vs §345a counts per session; a deficit proves a stampless path exists.
2. **A late-executing queued scene request:** a transition armed earlier (e.g. during the
   START_TALE1 re-arm window) whose scene-switch executes after the tale started — the delete
   side of a create/delete overlap in the fpc scene manager, entering teardown without the
   normal head of the delete function.
3. An untracked direct call path to `dEvt_control_c::remove()` not visible to source grep
   (inline expansion / member call via reference alias).

### Passive probes ARMED in the current build (all change-only; strip only after the fix)
| Probe | Site | Fires on |
|-------|------|----------|
| §345a | `dEvt_control_c::remove()` | any remove with an event live — logs status, runEvt, `this`, gFrm |
| §352b | d_s_play scene delete (before the remove call) | every scene deletion, gFrm-stamped |
| §341a | event Step | every mEventStatus transition — runEvt, demo mode/frames, susp, nextStage, evFlag8, gFrm, fade, linkSpd |
| §353 | `dEvt_control_c` ctor | any runtime construction (status-pre + gFrm) |
| §344 | `change()` grant | any event-change grant (requester proc) |
| §345b/c | reset() / order() | re-entrance orders, mid-event change-family submissions |
| §347a/b/c | `dStage_nextStage_c::set` + ba1 + doors | every transition ARM (stage/point/evRun/runEvt/gFrm) |
| §350b/c | status writers / entry() | 5→2, 2→0 writes and first-grant frames |
| §336b | `dDemo_c::start` | every STB start (instance #, runEvt, Link pos) |

**When it next strikes:** the log alone contains the full case — find the `§345a … status=1`
line, read its gFrm, and check (a) nearest §352b before/after (pairing distance), (b) §347a
ARMs in the preceding seconds (late transition), (c) §341a context. That names the cause among
the three suspects with no further instrumentation.

---

## KB-2 — Cutscene-seam PRESENTATION gap (~31 frames, "momentary regain") — OPEN, mechanism named

**Symptoms:** at the end of the tale (and by mechanism, any exit-ful WW cutscene), a ~0.5s
window where the room is visible while the camera/HUD revert — *feels* like a control regain.
**Input is proven locked** (§352c: 31 frames, linkSpd=0.0, position pinned, under a
held-direction protocol).

**Mechanism (confirmed §357–§359):** the STB's authored channel-9 fade-out fires
(`§355d ch9 fade OUT 20f`) but runs its 20 frames and **fully clears** — the JUT fader reads
status None (`jutSt=0`) and the gInf fader rate 0.00 through the entire gap. Neither of the
port's two fade systems holds black across the STB-end → reload seam; the donor's fader/overlap
holds until the scene wipe engages.

**Fix (bounded, queued as History's first item of the Foundry era):** donor JUTFader
`advance()`/`control()` semantics read — restore the completed-FadeOut hold (draw persists until
the next fadeIn/wipe) natively. Not a Foundry blocker.

**Armed probes:** §352c gap sampler (per-frame speed/pos/fade/jutSt/rate), §355b/c/d (offFade
callers, gInf fade edges, demo00 channel-9 beats).

---

*Founded 2026-08-02 at the History→Foundry pivot. Context: docs/state/ww-tale-dmesg-live-state.md
§335–§359 (the full campaign), bus §321–§330 (the tale-era fixes these defects survived).*
