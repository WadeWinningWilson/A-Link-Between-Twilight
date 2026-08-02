# Multi-hypothesis logging — one probe, ~10 discriminating branches

The discipline that finally cracked the step-in-step problem, and whose *absence* stretched it out.

## The rule

When a step fails, **do not build one fix for one hypothesis.** Build **one probe** that instruments
~10 competing causes at once — log a discriminating value for every branch — so a single playtest tells
you which cause is real. Then fix precisely.

## Why (the receipt)

For several builds during the step-in-step work I guessed a single lever at a time: fix the input read,
then the suspend accounting, then the release. Each was a whole build + playtest that ruled out one
idea. The user's correction was blunt and correct:

> "I think we're not making as much forward progress because you're no longer abiding by the 10
> hypotheses rule when a step fails. Probes that abide by that rule would have advanced us much further."

That produced the **§194 probe**: one per-frame log line during a demo box, whose columns each rule a
cause in or out —

| Column | Discriminates |
|---|---|
| `susp` / `susp*` (change) | STB never suspends / we resume too fast / accounting went negative |
| `gap` (frame − frameNoMsg) | timeline advancing despite `susp>0` |
| `tA/tB` vs `hA/hB` | phantom trigger vs real held button |
| `p2A/p3A` | wrong pad index during a demo |
| `boxSt` | box auto-completes vs never renders |
| `active/pg/flow` | our flow state vs the box state |

One playtest with that probe killed three hypotheses (phantom trigger, pad remap, box-never-renders)
and confirmed the mechanism (the storyboard advanced despite `susp=1`; actor `NPC_LS` moved through 5
positions while `gap` climbed to 1092). Every build after that was targeted.

## How to run it

1. Enumerate ~10 plausible causes of the failure **before** writing code.
2. Add a **single** per-frame (or per-event) log line whose fields discriminate them; change **no**
   behavior (pure probe) unless a candidate fix can be cleanly A/B-flagged in the same line.
3. Keep a written **hypothesis ledger** — carry survivors, refill to ~10, record what each build
   KILLED/CONFIRMED with the discriminating value. (Ours: `docs/state/tale-step-in-step-hypotheses.md`.)
4. Throttle the log to the interesting window (a box being up, a state change) so it doesn't drown.

## Companion discipline

Pairs with [symbolication.md](symbolication.md) (resolve the crash before hypothesizing) and with the
project rule to **confirm visual fixes by screenshot, not metrics.** Together they replace
guess-and-rebuild with measure-then-fix.

*(Also saved as the memory `feedback_multi_hypothesis_logging`.)*
