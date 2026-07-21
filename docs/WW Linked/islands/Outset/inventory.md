# Outset — generated inventory (ASK 10)

> **GENERATED** by `python -m ww_bridge space-report Outset`. Do not hand-edit numbers.
> Prose analysis stays in `README.md` / `grass-port-analysis.md` (not overwritten).
> Ground truth is the tool; if this disagrees with a fresh report, the report wins.

_Generated from tool joins. Exceptions: `day1_exceptions.ini`._

## 0. Baseline (day1-guard)

**Log:** `C:\Users\xxxxx\AppData\Roaming\TwilitRealm\Dusklight\logs\dusklight-20260720-143311.log`

| metric | n |
|--------|--:|
| day-1 expected | 271 |
| spawned (parsed) | 80 |
| matched | 80 |
| missing (unratified) | 2 |
| displaced | 0 |
| exception_ok | 79 |
| unmapped census rows | 106 |

**Missing:** `bridge`×2


## 1. NPCs / identity

Mapped folk-shaped (head/idle/talk/dialogue, non-static): **10** codes — `Bm1` `Dk` `Ko1` `Ko2` `Ob1` `P1a` `P1b` `Ym1` `Ym2` `Yw1`

**Identities locked:** **0** — `identity.ini` empty; all `? (unverified)`

Unmapped, NPC-shaped codes `? (unverified)`: `DmKmm`×1, `DmKmm2`×1, `HyoiKam`×1, `ITat00`×3, `Ktarur`×1, `NpcSo`×1, `Pitfall`×1, `Puti`×4, `Salvag2`×4, `SalvagE`×10, `Salvage`×4, `SwSlvg`×1

## 2. AI / behaviour (ASK 8)

| tier1 | count |
|-------|------:|
| inert | 19 |
| minimal | 24 |
| richer | 0 |
| unmapped | 44 |

**Actionable:**

| name | × | tier1 | states | actionable |
|------|--:|-------|-------:|------------|
| `item` | 14 | inert | 14 | `inert_but_portable` |
| `bridge` | 11 | inert | 0 | `inert_but_portable` |
| `lwood` | 6 | inert | 0 | `inert_but_portable` |
| `P1a` | 5 | minimal | 8 | `minimal_vs_rich_decomp` |
| `Kanban` | 3 | inert | 0 | `inert_but_portable` |
| `Ajav` | 2 | inert | 0 | `inert_but_portable` |
| `Auzu` | 2 | inert | 0 | `inert_but_portable` |
| `P1b` | 1 | minimal | 8 | `minimal_vs_rich_decomp` |
| `Akabe` | 1 | inert | 0 | `inert_but_portable` |

## 3. Vegetation / ratified absences

Deliberate holes from `day1_exceptions.ini` (not missing):

| name | × | decision |
|------|--:|----------|
| `kusax1` | 46 | deliberate_unmapped (№128) |
| `kusax21` | 36 | deliberate_unmapped (№128) |
| `kusax7` | 30 | deliberate_unmapped (№128) |
| `swood5` | 9 | wood_system_unported (ASK8/P1) |
| `swood3` | 6 | wood_system_unported (ASK8/P1) |
| `swood` | 6 | wood_system_unported (ASK8/P1) |

## 4. Clusters (unmapped + shared decomp source)

| decomp source | names | ×placements |
|---------------|-------|------------:|
| `src/d/actor/d_a_salvage.cpp` | `SalvagE`×10, `Salvage`×4, `Salvag2`×4 | 18 |
| `src/d/actor/d_a_tag_so.cpp` | `TagSo`×5, `TagMSo`×1 | 6 |

Socket ledger `unfilled_candidate` (suggest-never-fill):

| name | × | candidate |
|------|--:|-----------|
| `SalvagE` | 10 | decomp:src/d/actor/d_a_salvage.cpp |
| `Salvage` | 4 | decomp:src/d/actor/d_a_salvage.cpp |
| `Salvag2` | 4 | decomp:src/d/actor/d_a_salvage.cpp |
| `HyoiKam` | 1 | decomp:src/d/actor/d_a_kamome.cpp |

## 5. Unverified families (prefix groups — purpose `? (unverified)`)

**IVAN RULE:** grouping by prefix is allowed; **naming the purpose is not.**

- **`flwr*`** ×8 placements — `flwr17`×5, `flwr7`×3 — `? (unverified)`

## 6. Quest / triggers (ASK 6)

| name | × | flow |
|------|--:|------|
| `TagKb` | 10 | data |
| `TagSo` | 5 | env |
| `TagMsg2` | 3 | data |
| `AttTagB` | 2 | env |
| `TagMSo` | 1 | env |
| `ky_tag1` | 1 | env |
| `TagEv` | 1 | hybrid |
| `TagIsl` | 1 | hybrid |
| `TagHt` | 1 | hybrid |

Narrative-ish (`data`/`hybrid`) placements: **16** (of 25 trigger rows).

## 7. Props / structures — unclassified (low count)

`woodb`×5, `agbF`×5, `woodbx`×4, `SW_C00`×4, `c_green`×4, `Puti`×4, `TagMsg2`×3, `ITat00`×3, `flower`×2, `AttTagB`×2, `c_red`×2, `NpcSo`×1, `AND_SW2`×1, `Pitfall`×1, `ky_tag1`×1, `TagEv`×1, `agbCSW`×1, `SwSlvg`×1, `agbA2`×1, `Com_A`×1, `agbA`×1, `HyoiKam`×1, `TagIsl`×1, `agbFA`×1, `TagHt`×1, `DmKmm`×1, `DmKmm2`×1, `kuro_s`×1, `Ktarur`×1 — purposes `? (unverified)`

## 8. Interiors (EVNT counts from Yaz0 Stage.arc)

Extract root: `D:\XXXXXXX\Ex WW`

| stage | EVNT |
|-------|-----:|
| `LinkRM` | 5 |
| `A_mori` | 6 |
| `Cave09` | 2 |
| `Ojhous2` | 2 |
| `Ojhous` | 1 |
| `Pjavdou` | 1 |
| `Omasao` | 0 |
| `Onobuta` | 0 |
| `LinkUG` | 0 |

## 9. Data defects (live vs golden)

| defect | live | golden |
|--------|-----:|-------:|
| interior `PLYR` rows | 4 | 28 |
| outset rows with nonzero rotation | 354/475 | 354/475 |

Both cured by the §12 census refresh (content lane) — not by this report.

---

_End of generated inventory._
