# WW spaces — per-island / per-dungeon work docs

**What this tree is:** one folder per space, holding a live inventory of what is *not yet done*
there. **These documents shrink.** A row exists only while it is outstanding or is a ratified
decision not to act; once something is mapped and verified it leaves the page.

They are **not** design docs — the pipeline lives in
[WW-Restoration-Cookbook-SUPERSEDED.md](../WW-Restoration-Cookbook-SUPERSEDED.md), the tool lane in
[ww-bridge-tool-interconnected.md](../ww-bridge-tool-interconnected.md), and the decision ledger in
[state/cut-actors-demo-restore.md](../../state/cut-actors-demo-restore.md).

## Layout

```
islands/
  <Space>/
    README.md          — the space's remaining work, sectioned (below)
    interiors/
      <Stage>.md       — one per interior stage, when it has content worth tracking
```

Dungeons use the same shape; a dungeon is just a space with more rooms.

## Standard sections

Kept uniform so the same question can be asked of any space:

1. **NPCs / identity** — who is placed, who is mapped, what is locked
2. **AI / behaviour** — inert vs minimal vs richer; what has decomp source
3. **Vegetation** — usually the largest block, often deliberately deferred
4. **Clusters worth porting** — several census names behind one decomp source
5. **Unverified families** — codes whose *purpose* is not yet established
6. **Quest / triggers** — narrative surface, separated from environmental triggers
7. **Props / structures** — unclassified one-offs
8. **Interiors** — per-stage index
9. **Data defects** — anything blocking the sections above
10. **Known-good reference** — snapshot + what must *not* be "restored"

Not every space needs all ten. Drop empty sections rather than padding them.

## Rules

- **Ground truth is the tool, not the doc.** Regenerate inventory with
  `python -m ww_bridge space-report <Space>` (ASK 10) → `<Space>/inventory.md`. A doc that disagrees
  with a fresh report is stale and the report wins. Do not hand-transcribe tool counts into README.
- **IVAN RULE.** Census codes are codes. Where a name or purpose is unverified, mark it
  `? (unverified)` and leave it — **never infer**. Identity comes from the user, semantics from the
  decomp.
- **Record deliberate absences as decisions, with their ledger reference** (e.g. grass off, №128).
  An unexplained gap and a ratified one must never look alike.
- **Counts are placements, not actors.** One decomp source may cover many placements — that ratio is
  the whole point of the "clusters" section.

## Spaces

| space | doc | status |
|---|---|---|
| **Outset Island** | [Outset/README.md](Outset/README.md) | exterior populated day-1; 192 unmapped; 9 interiors |
| Great Sea | — | not started; `sea` stage carries `SCLS:212`, `RTBL:50` (50 rooms) |
| Forest Haven / Dragon Roost / Windfall … | — | not started |

**Outset is the exemplar** (№63 one-exemplar rule). Its section shape is what later spaces copy —
and the Great Sea is the same assertion run fifty times, not a new problem.
