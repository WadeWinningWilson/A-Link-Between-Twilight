# NPC presence recipe — existence, position, presentation (three axes, all data-driven)

**Established:** ledger №243–№245 (Grandma position research + the postman visibility case).
**Consumers:** the population spawner (existence/position) + the mount actor (presentation).
**For Cursor:** this is the implementation contract for the placement/visibility engine work —
each axis names its data source, its receiver mirror, and its acceptance test.

**The law this encodes (user):** WW's own systems move and reveal NPCs as the story advances.
We mirror THOSE systems so a story change re-stages the whole island from data — hand-placed
positions and ad-hoc visibility toggles are the anti-pattern, forever.

---

## Axis 1 — EXISTENCE (room layers)

**WW mechanism:** a room's actor list is not one list — placements live in LAYER chunks
(`ACTR` + `ACT0`…`ACTb`), and the story-flag ladder selects the active layer per room.
Actors on inactive layers DO NOT EXIST — no process, no draw, no collision.

- The ladder is real shipped code: `d_com_inf_game.cpp:192-200` — for Outset:
  `0x0520 → layer|4`, `0x0E20 → layer|2`, `0x0101 → layer 9`, else base. (Full story-term
  mapping: `reference-ww-flags-triggers.md §№222`.)
- **Diagnosed symptom:** "actors floating in mid-sea" = layer-blind spawning — census rows pulled
  without layer filtering (or from a merged-layer dump).

**Receiver mirror:** census rows carry their LAYER; the spawner computes the active layer from the
mod's story flags (our equivalents of the ladder bits) and spawns ONLY that layer's rows.

**Acceptance:** flip a story flag → the island re-stages (actors appear/vanish per layer) with no
code change and no manifest edit.

## Axis 2 — POSITION / STATE (params variants)

**WW mechanism:** one NPC, several placement rows distinguished by `params` = its story-state
variants. Proven case — Grandma (`Ba1`) in `LinkRM/Room0.arc`:

| params | pos | state |
|---|---|---|
| 0 | (-225.0, 375.0, -55.0) | **second floor (loft)** — the birthday/clothes scene |
| 1 | (-308.1, 0, 379.1) | ground floor |
| 3 | (575.1, 0, -27.3) | ground floor |
| 4 | (347.1, 0, 129.5) | ground floor |

Which params spawns under which flags is per-actor logic (often Nonmatching in the decomp) —
**map by observation at wiring time, never by guess (IVAN).**

**Receiver mirror:** census keeps EVERY params row; the spawner selects the variant for the
current story state (same flag source as axis 1). A state advance = a different row spawns.

**Acceptance:** advancing the clothes-scene flag moves Grandma loft → ground floor purely from
data.

## Axis 3 — PRESENTATION (present-but-hidden)

**WW mechanism:** an actor can EXIST as a live process while being neither drawn, collidable, nor
targetable — until its scene readies. Proven case — the postman (`Bm1`): at the telescope beat,
Aryll's own actor finds him via `searchByID(mBm1ProcID)` and sets a go-flag (`m881 = 1`) — he is
ALREADY a process before the player ever "sees" him. His exact hide mechanism is inside
Nonmatching code, so **both candidate forms stay open until observed** (late secondary spawn vs
draw+collision suppression). The CONTRACT is what matters:

> **Hidden means: no draw + no collision + no attention/targeting.** Placed ≠ presented.

- **Diagnosed symptom:** the postman standing visible at his mailbox pre-telescope = axis 3
  missing (his placement is right; his presentation is early).

**Receiver mirror:** a `present_if` flag family on the mount (sibling of the existing
`spawn_if_flag` / `spawn_unless_flag`): when the gate is closed the actor exists (process alive,
so scene coordination like the telescope-look can flag it) but draws nothing, registers no
collision, offers no attention. Opening the flag flips all three at once.

**Acceptance:** the postman is absent-to-the-player until the telescope beat fires his flag, then
present — with no spawn/despawn churn in the log.

---

## How the axes compose (one flow)

```
story flags ──► active LAYER (axis 1) ──► which rows exist
                     │
                     └─► params selection (axis 2) ──► where/which state
                                  │
                                  └─► present_if gate (axis 3) ──► shown & touchable?
```

One flag advance can move an actor through all three: a new layer activates, a different params
row spawns, and a presentation gate opens — all from data.

## Related prior art

- The show/hide STATE GATE on held props (telescope scale gate; Grandma's `ba_cloth` bundle) is
  axis 3 in miniature at the attachment level — same contract, smaller scope.
- `[[feedback_port_full_state_machines]]`: presence states are donor state machines — port them
  whole, never park a state as "optional."
- Sibling recipes: [../interactions/](../interactions/README.md) (mass channel + future cc AT/TG).
