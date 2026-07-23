# Outset Island — remaining work

**Purpose:** a live inventory of what is *not yet done* on Outset. **This document shrinks.** Every
row here is either work outstanding or a ratified decision not to do it; when something is mapped and
verified it leaves the page.

**Ground truth is the tool**, not this file. **Numbers live in the generated inventory** — regenerate
with:

```bat
python -m ww_bridge space-report Outset [--log path\to\dusklight-*.log]
```

→ [inventory.md](inventory.md) (ASK 10). Do not hand-transcribe counts into this README.

Prose / decisions stay here; if a number here disagrees with `inventory.md`, **the inventory wins**.

> **The 2 "missing" rows are `bridge` on `ACT0`.** Working hypothesis (user, 2026-07-20):
> WW rope bridges carry **suspender ropes** as well as planks, so these are likely the rope variant
> the plank-instancing actor does not yet handle — i.e. a scoped feature gap, not a regression.
> **SUSPECTED, NOT PROVEN.** Until confirmed against the donor's bridge actor they stay counted as
> missing; do not close them on the hypothesis alone.

> **IVAN RULE applies throughout.** Names and purposes below are census codes and observed counts.
> Where a code's meaning is unverified it is marked `? (unverified)` and **must not be guessed** —
> confirm against the decomp or a user identity pass first.

---

## 1. NPCs / identity

> **Per-character detail lives in [characters/](characters/README.md)** — one subfolder each (assets,
> cutscene beats, expression maps, open bugs). Aryll (`Ls1`) is the first; others get folders as work
> reaches them. Keep this section to the island-wide census/identity picture; character specifics go
> in the subfolders.

| status | detail |
|---|---|
| Mapped folk on the exterior | 16 codes — `Ah` `Aj1` `Ajav` `Bb` `Bm1` `Dk` `Ko1` `Ko2` `Ls1` `Ob1` `P1a` `P1b` `P2b` `Ym1` `Ym2` `Zl1` |
| **Identities locked** | **0** — `identity.ini` is empty; every `display_name` reads `? (unverified)` |
| Unmapped, NPC-shaped | `NpcSo` ×1 — `? (unverified)` |

**Blocking work:** the §41 Z-target probe (engine). Until it lands, identity cannot be established by
observation and **must not be established by inference.** User has visually identified two characters
(1st, 2nd in target order) but they cannot yet be bound to census codes.

---

## 2. AI / behaviour

From the ASK 8 behaviour ledger — **0 actors have richer-than-minimal behaviour.**

| tier | count |
|---|---|
| inert (`static=1`) | 19 |
| minimal (`idle` / `talk1` / `dialogue` only) | 24 |
| richer | **0** |

**Actionable (decomp source exists):**

| actor | × | decomp states | note |
|---|---:|---:|---|
| `item` | 14 | 14 | inert but portable |
| `P1a` / `P1b` | 6 | 8 | minimal vs an 8-state decomp actor |

Most other "inert" entries are props with 0 states — correctly inert, **not** work.

**Constraint:** the whole behaviour vocabulary is three manifest keys (`idle`, `talk1`, `dialogue`).
Richer behaviour needs either schema growth or a ported actor driving states itself.

---

## 3. Vegetation — **largest single block, deliberately off**

| name | × | status |
|---|---:|---|
| `kusax21` | 36 | **deliberate_unmapped (№128)** |
| `kusax7` | 29 | **deliberate_unmapped (№128)** |
| `kusax1` | 21 | **deliberate_unmapped (№128)** |
| `swood5` `swood3` `swood` | 19 | unmapped — donor wood system |
| `flwr17` `flwr7` `flower` | 10 | unmapped |

**115 placements — 60 % of everything unmapped.**

**Do not "fix" the grass.** `kusax*` is off because it was the confirmed cause of the vanishing cast
(№128); it stays off until its resource footprint is solved. This is ratified in
`goldens/day1_exceptions.ini`.

`swood*` is the donor wood system (content-lane P1). `lwood` is *already mapped* and is a separate
actor — do not conflate them.

---

## 4. Salvage — best port-per-effort ratio on the island

| name | × |
|---|---:|
| `SalvagE` | 10 |
| `Salvage` | 4 |
| `Salvag2` | 4 |
| `SwSlvg` | 1 |

**19 placements behind a single decomp source (`d_a_salvage.cpp`).** One port closes all of them —
the highest-leverage cluster the socket ledger found.

---

## 5. `agb*` family — purpose UNVERIFIED

`agbMARK` ×10 · `agbF` ×5 · `agbA` `agbA2` `agbB` `agbCSW` `agbFA` ×1 each — **20 placements.**

**Meaning not established.** Second-largest unmapped cluster; likely shares one system given the
common prefix. **First step is decomp identification, not mapping.**

---

## 6. Quest / triggers

Classified by ASK 6 — most are **not narrative**:

| name | × | ASK 6 class |
|---|---:|---|
| `TagKb` | 10 | dig-item gates — not narrative |
| `TagSo` / `TagMSo` | 6 | sonar volumes — env |
| `ky_tag1` | 1 | waves — env |
| `TagEv` / `TagIsl` / `TagHt` / `TagMsg2` | 4 | **the actual narrative surface** |

**Outset's exterior narrative surface is ~3–4 triggers, not 25.** `eventNo` indices `1`, `15`, `53`
resolve into `sea/stage.dzs`'s 57-entry `EVNT` table (decompressed at
`D:\XXXXXXX\Ex WW\_extracted\Stage_decompressed\`).

**Interior events add ~17** across the nine interior stages — see §8.

---

## 6b. `Akabe` — PROMOTED, **UNTESTED**

`Akabe.arc` promoted from `arcs_quarantine_full_object_dump/` → `arcs/` (2026-07-20, user-directed),
resolving the `npc_akabe.ini` FAIL that ASK 4 caught. `verify` now CLEAN.

**What is verified:**

- Arc is **collision-only** — one member, `akabe.dzb` (352 b), no model. Consistent with
  `npc_akabe.ini` declaring `proc=` / `arc=` and **no `model=` key**.
- **Adaptation confirmed applied** — `batch_adapt_log.txt`: *"akabe.dzb: 1 ti through-cluster
  cleared."* The DZB through-flag fix is in, so it will not render as pass-through geometry.
- Copy is md5-identical to the quarantine source; quarantine left intact.

> **⚠ NEEDS TESTING TO PROVE. Nothing here shows it WORKS.**
> `verify` only proves the arc the manifest names now exists. **Untested:** whether the collision
> actually lands in-world, whether `NPC_AKABE` does anything useful with a model-less arc, and
> whether the placement (×1 on Outset) sits where it should. **Do not treat CLEAN as working** —
> that distinction is the whole point of the guard.

**Confirm by:** entering Outset and testing the single `Akabe` placement for solidity. Until then it
stays listed as outstanding.

**Process note:** the quarantine dump is no longer inert clutter — it is now a **staging area work is
drawn from**. That strengthens the standing recommendation to relocate it out of
`model_replacements/` and formalise promotion, rather than leaving promotion as an ad-hoc copy.

---

## 7. Props / structures — unclassified

`woodb` ×5 · `woodbx` ×4 · `SW_C00` ×2 · `AND_SW2` `Pitfall` `Com_A` `HyoiKam` `ikada_h` ×1 each.

Low placement counts; likely one-off props and switches. **Purposes unverified.**

---

## 8. Interiors

Nine stages, now hosted as rooms in `R_DL01` (one room per interior, TP `R_SP01` pattern).

| stage | EVNT | per-interior doc |
|---|---:|---|
| `LinkRM` | 5 | `interiors/LinkRM.md` |
| `A_mori` | 6 | `interiors/A_mori.md` |
| `Cave09` | 2 | `interiors/Cave09.md` |
| `Ojhous2` | 2 | `interiors/Ojhous2.md` |
| `Ojhous` | 1 | `interiors/Ojhous.md` |
| `Pjavdou` | 1 | `interiors/Pjavdou.md` |
| `Omasao` `Onobuta` `LinkUG` | 0 | — |

**Known interior defect:** `LinkRM`'s census holds **no NPC actors at all** — 18 rows, all props and
triggers. Whatever populates Grandma's house is not the room census. **Unexplained; worth chasing.**

---

## 9. Data defects blocking everything above

Both fixed in the tool, **neither deployed** (§12 / §42):

1. **24 `PLYR` arrival spawn points missing** from the live census (live 4, golden 28). Door
   arrivals misplace as a result.
2. **Rotations zeroed across 7 stages** — every actor faces default-north.

**The §12 census refresh fixes both and is the highest-value deployable action on the board.**

---

## 10. Known-good reference

Mod folder snapshot: `Documents\dusklight-backups\modfolder-KNOWN-GOOD-20260720-122124\`
(1,362 files / 201,635,853 b, verified).

**Deliberately captured, do not "restore":** grass off (№128); `actor_map.ini` correct as-is — the
day-1 comparison proved all 10 identity-bearing folk map correctly, and rolling back to the skeleton
copy would destroy real authoring.
