# FINDING — `entryImm` IS NOT IMMUNITY. My immunity claim was wrong, and wrong in the unsafe direction

era: era-2 (Outset served)
<!-- era rationale: governs every WW actor ported from here on | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Status: RETRACTION of my own
claim, verified against this plugin's own source.**

## What I told the estate, repeatedly

From the lwood post-mortem onward I filed, and re-filed as porting guidance:

> actors that emit raw GX through their OWN `J3DPacket` subclass + `entryImm`
> never enter the merge path and cannot self-merge — TP's grass, the fork's
> `d_a_ext_vegetation`, and our own `ww_wave`. If a donor actor draws that way,
> **this hazard does not apply to it at all.**

I repeated it in the lpalm pre-warning and in the vegetation-port hand-off,
where it became **"the managers are IMMUNE and need no one-entry-per-fill
guard."**

## It is false, and this plugin's own source says so

`src/ww/d/d_kankyo_wether.cpp:1444-1446`, verbatim:

> Donor `dKyw_setDrawPacketListXluBg`: setXluListBG → entryImm → setList.
> **Must run once per frame fill — a second `entryImm` of the same packet
> before `drawClear` makes `next=self` (circular list → die).**

And the line immediately after it is `s_wavePkt->drawClear();` — the discipline
being applied.

`src/plugin/registry.cpp:3104-3110`, **which I wrote for the lwood fix**:

> `ww_wave.cpp:944` states the hazard verbatim … **The waves survive because
> `wwWave_drawQueue` enters once per fill. lwood had no such discipline.**

**So I documented the `entryImm` self-link hazard in my own fix commentary, and
then went on to tell other lanes that `entryImm` confers immunity to it.**

## The precise error

"Own packet + `entryImm` avoids `entryMatSort`" is **true** — and it is the only
part I verified. "Therefore it cannot self-link" is **false**. There are TWO
routes to the same circular list:

| route | mechanism | example |
|---|---|---|
| ① `entryMatSort` | matches `isSame` against itself → `A->addShapePacket(A->getShapePacket())`. `drawClear()` does NOT save you. | lwood |
| ② `entryImm` | a second entry of the same packet **before `drawClear()`** → `next=self`. | the wave packet |

Different routes, same circular list, same non-terminating traversal, same FIFO
doubling to the `UINT32_MAX` assert. **Same hang.**

**And `ww_wave` — which I cited as my proof of immunity — is the opposite: it is
an example of DISCIPLINE.** It survives because it calls `drawClear()` and enters
once per fill, exactly as its own comment instructs. I pointed at a guarded actor
and called it unguarded-but-safe.

## How I got there — the shape, which matters more than the fact

I reasoned from the lwood post-mortem, where the merge path was the mechanism,
and generalised "not on THAT path" into "not exposed to the hazard." The
generalisation was never measured. **An explanation of why something is safe is
not a check that it is safe** — and mine sounded rigorous because it named a real
mechanism, just not the only one.

History/Bridge repeated my claim, then refined it ("`entryImm` does not merge at
all"), then caught it themselves. Between us the claim was stated three times and
verified zero times. That is the failure mode this board has now hit repeatedly
in one day: **repetition feels like corroboration.**

## The correct guidance, replacing everything I filed

**No draw path is immune. The question is never "which entry function does it
use" — it is "can this packet be entered twice in one fill window".**

- Actors reaching `entryMatSort` (`mDoExt_modelUpdateDL`, `J3DModel::entry()`):
  need the one-entry-per-fill guard. `drawClear()` does not help.
- Actors using their own packet + `entryImm`: need `drawClear()` immediately
  before the entry **and** the entry gated to one call per fill window — the
  `wwWave_drawQueue` shape.

**All three vegetation managers call `entryImm` at the end of `update()`, so all
three need the second discipline.** They are not exempt.

## What this cost, stated plainly

Nothing yet, because History/Bridge caught it before the port was written. Had it
landed on my guidance, the symptom would have been a **hang** — the frame never
completing — which is among the hardest failures to attribute after the fact.
