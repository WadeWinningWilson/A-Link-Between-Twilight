# WW Restoration Cookbook

The ground-up process manual for the restoration run. The ledger
([cut-actors-demo-restore.md](state/cut-actors-demo-restore.md)) is the
chronological record of *what happened*; this is the distilled *how to do it*.

Rebuilt 2026-07-20 (the prior copy was lost before it was ever committed —
if this file goes missing again, that is a build/commit gap, not a decision).

---

## §0 Doctrine — the rules that outrank convenience

**The receiver covenant.** The shipped binary and the vanilla game tree contain
zero donor content: no arcs, no bytes, no dialogue, no player-visible names.
Everything lives in `%AppData%/TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration/`.
Never commit donor arcs. Never share a build that has not passed the gate today.

**№31 Space-Purity Law (FINAL, no exceptions).** Donor spaces receive only donor
assets; receiver spaces only receiver assets. **A missing prop is always
preferable to a foreign one.** This law has real teeth — see §7, where it closed
an otherwise free path and forced a 2,000-line port.

**R6.** Any player-visible string describing donor content comes from DATA
(manifests, `.ini`, `.tsv`), never from source.

**The Ivan rule.** No invented identity labels. A label stays `? (unverified)`
until the user locks it, or until a decomp source names it. Decomp-sourced names
are *sourced*, not invented — cite the file.

**Suggest, never fill.** When a value is unknown, say so and stop. Do not
produce a plausible-looking guess. The `lwood` fix (§7) is the model: the
finding lane flagged a candidate without asserting it, and the value was then
*proved* from `d_a_lwood.cpp` before anything changed.

**Division of labor.** If it is a VALUE, History wrote it. If it is a BEHAVIOR,
the engine lane built it. If it is a NAME ON A FACE, the user said it.

---

## §1 Where things live

| what | where |
|---|---|
| donor disc extract | `D:/XXXXXXX/Ex WW` |
| donor decomp source | `D:/XXXXXXX/WW DP` |
| receiver disc extract | `D:/XXXXXXX/Ex TP` |
| curated arcs (shipped by the mod) | `<mod>/arcs/` |
| batch-adapted object library (577) | `<mod>/arcs_lib/` |
| stages we author | `<mod>/files/res/Stage/<STAGE>/` |
| manifests | `<mod>/npc/*.ini`, `<mod>/population/*.ini` |
| extracted asset packs | `<mod>/assets/` |
| tools | `tools/ww_crew_restoration_skeleton/` |

**The decomp is the fastest way to decode donor binary data.** Struct
definitions are reliable even where a stage is only partly decompiled. It turned
the lighting conversion from reverse-engineering into a field table, and it is
what settles identity questions without guessing.

---

## §2 Arc adaptation

`adapt_bdl_arcs.py` converts a donor arc for the receiver's loader:

- BDL4 → BMD3 retag, MDL3 section stripped (precompiled GX lists are GC-only)
- `normalize_litmask` — clamp enabled channels to `0x01` (slot 1 garbage blacks
  the channel)
- `normalize_tevregs` — promote the 50%-gray TEV placeholder to white
- `adapt_dzb` — clear the receiver's through-flag cluster (`~0x00FFC000`)

**Trap: `normalize_tevregs` is wrong for sky domes.** For `vr_*` models the TEV
register *is* the sky colour, not a lighting multiplier — whitening it paints
the dome pure white. `adapt_arc` now passes `skip_tevregs=True` for `vr_*`;
their colours come from `bake_wwsky_colors.py`. (№116)

**Trap: J3D data is pointer-fixed to its buffer.** Never re-parse a fixed
buffer. Never free an arc while parsed data is cached. Audit every
session-lived cache when adding a lane.

---

## §3 Stages, rooms, and the RTBL trap

Stage arcs are built by `build_fdl_host_stg.py` / `grow_rdl01_stg.py`.

> **RTBL stores ABSOLUTE offsets into the dzs.** Adding or removing *any* chunk
> resizes the chunk header and slides RTBL, silently invalidating every pointer
> inside it. The table still parses — it just points at the old layout.
>
> **Any pass that changes the chunk count MUST regenerate RTBL and call
> `assert_rtbl_pointers()`.** Chunk-count changes are never cosmetic. (№114 —
> this cost a crash that was initially misattributed to another lane's bug.)

Also: convert from a pristine backup, never from your own output, so re-runs are
idempotent.

Collision room ids come from the dzb's own `cBgD_Grp_t.m_room_id` — donor rooms
declare *their* numbering and must be re-tagged to ours (№105).

---

## §4 Lighting conversion

Four-table chain, same in both engines, different records:

| donor | receiver | note |
|---|---|---|
| `EnvR` 0x08 | `Env0` 0x41 | per-room → palette sets |
| `Colo` 0x0C | `Col0` 0x0C | **identical layout** |
| `Pale` 0x2C | `PAL0` 0x34 | +6 trailing fields |
| `Virt` 0x24 | `VRB0` 0x18 | reordered |

Run `convert_lighting.py`. Two things that are not obvious:

1. **`PAL0`'s six trailing fields have no donor source and are not
   neutral when zeroed.** Native outdoor stages set `unk_0x2f=200`,
   `bg_light_influence=100`, `cloud_shadow_density=45`, `bloom_tbl_id=4+band`.
   A zero bloom id selects a different bloom table — this was the "blown-out"
   report. Defaults are sampled from `F_SP00/102/103/108/121`.
2. **The env layer is chosen by `Elst`** (`m_layerTable[storyLayer]` → env
   digit), *not* the story layer. With no `Elst` the decoder pins layer 0, so
   inherited `Env8/Col8/PAL8/VRB8` are unreachable — but they arm the moment an
   `Elst` appears. Drop them.

`getVrboxInfo()` (the legacy `Virt` table) is set but never read outside
`d_stage.cpp` — dropping `Virt` is inert.

---

## §5 Actors and population

Census CSV × `actor_map.ini` × arc library. A name is *mapped* when
`actor_map.ini` has a section for it and a manifest exists.

**Verify every manifest against real arc members.** Boilerplate is the enemy:
`model2=model1.bdl` / `model3=model3.bdl` were pasted into all 11 `ext_bg*.ini`
regardless of contents, and `npc_lwood.ini` pointed at a `lwood.bdl` that never
existed in the arc. Both produced *silent* load failures with correctly-placed
actors. Audit with the arc member list, not by eye. (№115, №118)

**A hole is a legitimate deliverable.** `Akabe.arc` ships only `akabe.dzb` —
collision, no model. Wiring it would spawn an invisible no-op, so it is recorded
as a hole with the reason. Under №31 that is the correct outcome.

---

## §6 Doors and transitions

Stage change: `dComIfGp_setNextStage` + `fopScnM_ChangeReq`. Same-stage room
load: `fopScnM_CreateReq`. `dStage_playerInit` matches the spawn point against
`(u8)angle.z` of PLYR entries.

Cameras need `QuickStart()` (snap) on arrival, not `Start()` (interpolate) —
otherwise the camera flies in from the map edge.

**Warm-mount storms are gated on arrival at the island** (`EXT_BG0`), which is
why a fault can appear on *leaving* an interior but never on entering. When a
symptom is asymmetric, look for an arrival-gated code path. (№115)

---

## §7 Porting a donor system

Before costing a port, in this order:

1. **Is it a manifest bug?** `lwood` looked like part of a ~1,700-line wood-system
   port. It was a placed, already-mapped actor pointing at a filename that never
   existed — a one-line fix that restored 38 trees. Check the manifest first.
2. **Does the receiver already have it?** The receiver's `d_stage.cpp` carries
   native `OBJNAME` rows for `kusax1/7/21`, `flower`, `flwr7/17`, `pflower`,
   `pflwrx7` → `fpcNm_GRASS_e`, and ships `d_a_grass.cpp` + `d_grass.inc`. The
   architecture was inherited wholesale.
3. **…but check the ASSETS against №31.** This is where step 2 died: the
   receiver's grass is `M_kusa05_RGBATEX` / `M_Hijiki00TEX` (31×31); the donor's
   is `l_K_kusa_00TEX` (64×128) / `l_Txa_ob_kusa_aTEX`. Different art. Routing
   donor placements through the native actor would plant receiver grass on a
   donor island. **Shared architecture does not mean shared assets.**

**Where donor system assets live.** Grass/tree/flower are *not* actor overlays —
they are packet systems in the donor's main executable with static texture and
display-list arrays. `files/maps/framework.map` names each with an exact address
and size; `extract_veg_assets.py` resolves address → file offset through the
executable header and writes them mod-side (§8 forbids embedding them).

Extracted pack: 30 blobs, 21.7 KB, `<mod>/assets/veg/` + `veg_manifest.ini`.

| system | donor source | placements |
|---|---|---|
| grass (`kusax1/7/21`) | `d_grass.cpp` 472 | 112 |
| trees (`swood*`) | `d_tree.cpp` 687 | 21 |
| flowers (`flwr*`) | `d_flower.cpp` 585 | 10 |
| dispatcher | `d_a_grass.cpp` 218 | — |

Note the dispatcher is shared: all of these arrive through one profile, and the
param word carries `type` (bits 0-3), `kind` (4-5), `itemNo` (6+).

---

## §8 The gates

**Covenant gate.** See the standing header at the top of the ledger. Comments
are exempt (stripped by the compiler); nouns the two games share are not
breaches; and donor-origin data the receiver *itself ships* (e.g. `itemmdl`,
21/21 byte-identical) is receiver data. Strip comments, check the term against
receiver content, check whether the receiver already ships the asset — then file
with file:line and the literal quoted.

**Never embed donor asset bytes in source.** A port that does
`#include "assets/l_K_kusa_00TEX.h"` compiles donor art into the shipped binary.
Extract to the mod folder and load at runtime.

---

## §9 Symptom → cause

| symptom | first thing to check |
|---|---|
| crash during stage setup, before actors | RTBL pointers after a chunk-count change (§3) |
| sky pure white | `vr_*` TEV registers whitened by the adapter (§2) |
| "blown out" / wrong exposure outdoors | `PAL0` trailing fields zeroed, esp. `bloom_tbl_id` (§4) |
| interior lighting looks like another room's | stage inherited a template's tables; per-room `Env0` missing (§4) |
| actor placed correctly but invisible | manifest names a member the arc does not contain (§5) |
| fault only when *leaving* a space | arrival-gated path — warm-mount storm (§6) |
| hard freeze, no log line, no crash dump | blocking allocation retry after a heap fail (§6) |
| camera flies in from map edge | `Start()` used where `QuickStart()` is needed (§6) |
| room black / no floor | donor collision declaring donor room ids (§3) |
| model at half brightness | `litMask` not clamped, or TEV gray placeholder (§2) |

---

## §10 Working agreements

- End every report with an explicit **WHOSE TURN** block: who holds it, and the
  exact next action.
- Never declare a visual fix done from metrics. Show it, and ask the user to
  confirm. A structural pass is not a visual pass.
- When a diagnosis is disproved, say so plainly and record the correction in the
  ledger. Several entries here exist because a confident wrong answer was
  corrected — that record is the point.
- Commit shared work often.
