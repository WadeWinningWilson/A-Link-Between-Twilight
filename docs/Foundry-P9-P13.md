# Foundry P9–P13 — the game-systems parity program

> Foundry, 2026-07-29 (bus §210; user-ordered evolution beyond the delivered/parked
> P1–P8). Five domains: spawning, layers, transitions, AI pathing, enemy combat AI.
> Method sourced from the projects that already solved donor-faithful porting at scale,
> then grounded in our instrument estate (P2 differ, fact-sheet library, decomp, taps).

## Sourced disciplines (what the analog projects teach)

- **Tale of Two Wastelands** (FO3→FNV, the closest living analog): ships **zero donor
  bytes** — an installer *converts* a clean donor install into receiver-engine records
  as a **pure, re-runnable transformation**. Clean inputs → deterministic output.
  **COVENANT CORRECTION (user ruling, §211) — we take TTW's conversion discipline but
  NOT its mechanics-upgrade doctrine:** here, ALL logic derives from the DONOR to the
  letter; the TP engine is the RECEIVER/execution substrate only. Donor state machines,
  values, and conditions are the spec verbatim — never "improved" into receiver idiom.
- **DevilutionX** (Diablo reimpl): **save-game compatibility as the parity oracle** —
  vanilla saves load in the port and vice versa; behavior verified at the save-bit
  level. We hold the same asset: donor `.gci` saves + the decomp's `d_save.h` layout.
- **OpenRCT2**: **replay + checksum CI** — every PR runs recorded replays and fails on
  entity-checksum divergence; deterministic RNG state is the desync sentinel; one
  pinned platform (their x64 lesson). This is P3/P7's proven production blueprint.
- **Skywind/Skyblivion** (Morrowind/Oblivion→Skyrim): the **dependency-chain
  doctrine** — quests need moving NPCs, movement needs AI packages, packages need
  navmeshes, navmeshes need meshes. Porting order is a dependency graph, not a
  preference.

**The chain, ours:** spawn → layer → transition → pathing → combat AI. Verify in that
order per island; a broken lower rung invalidates verdicts above it.

## P9 — Spawn-parity harness (spawning logic)

**Have:** donor static rosters (156-stage fact-sheet library, per-layer), donor runtime
spawn tap (`stage_placement`, 1,427 events banked), receiver `[Spawn] src=census`
events (282 banked; converter parses all three into one schema).
**Build:** `spawn_differ` — per-room/per-layer roster verdict: fact-sheet (what should
exist) vs receiver spawn log (what did), with donor runtime tap as the tie-breaker for
WHEN-conditions. Then the WHEN table: extract per-actor spawn gates (switches, flags,
day/night) from decomp `dStage`/actor createInit into fact-sheet sidecars.
**First mission:** Outset exterior spawn verdict from ALREADY-BANKED logs — zero new
capture.

## P10 — Layer-logic oracle (layer logic)

**Have:** true per-layer rosters (fact sheets); decomp layer-selection machinery
(d_stage.cpp/d_s_room.cpp, save inputs per `d_save.h`); donor `.gci` saves on hand;
receiver flag store (`dExtModFlags`).
**Build (DevilutionX pattern):** a pure-Python **layer oracle** — decomp's
layer-selection decision transcribed once: (story-flag state) → expected layer per
stage. Feed it donor `.gci` saves (gclib parses the container; `d_save.h` gives the
layout) → expected-layer table → diff against the port's chosen layer (one DuskLog
line at the port's layer pick, same pattern as the emitter tap).
**Payoff:** story-layer correctness becomes computable per save, pre-empting the
"invisible resets" fear for every future island.

## P11 — Transition graph (transition logic)

**Have:** every SCLS exit table in the game already parsed (fact-sheet JSONs: 10
Outset-room exits, 212 sea-stage, all 156 stages); receiver transitions ALREADY
LOGGED (`[Doors] enter … transport=stage host=… room=… point=…` — seen in the §208
session log).
**Build:** `transition_graph.py` — donor exit graph artifact (stage/room/point →
dest, per layer) + differ that replays receiver `[Doors]` logs against the graph:
every port transition either matches a donor SCLS edge or gets named (port liberty or
bug). Ordinary play sessions become transition-verdict data with no new instrument.
**First mission:** run every banked receiver log through it.

## P12 — Path parity (AI pathing) — SETTLED §213

**Delivered:** sweep now captures RPAT/RPPN fields; `tools/foundry/path_maps.py`
groups waypoints under path headers (offset/0x10 arithmetic per wwlib; mismatched
groupings self-flag [INFERENCE-NEEDED]). Artifacts: `donor-path-summary.md` —
**1,821 paths / 13,441 waypoints game-wide** — and per-stage tables on demand
(`donor-paths-sea.md`: 1,051 paths shipped).
**Receiver socket (vexp-located):** TP's `dPath` system (d_path), consumed across
actor code — the port-side tap is one DuskLog line at path-follow init (actor, path
index; emitter-tap pattern). **[INFERENCE-NEEDED]:** whether the bridge converts
donor RPAT/RPPN into receiver PATH/PPNT when authoring STGs — Bridge/History confirm
before the tap means anything. Runtime trajectory diff stays the later rung.
**Dependency note:** meaningless until P9/P10 pass for the room (Skywind chain).

## P13 — Combat-AI state parity (enemy AI)

**Doctrine (TTW + port-full-state-machines law):** the donor's decompiled action state
machine IS the spec (`d_a_e_*.cpp`, 72% and climbing); receiver executes it on TP's
combat framework; parity = same states, same transition triggers (ranges, timers, HP
thresholds — VALUES from decomp), not frame-identical animation.
**Build:** action-state tap pair — donor: DuskTap on the enemy family's mode-write
choke (per-family address from the 372 debug maps); receiver: DuskLog in the ported
actor's mode switch (emitter-tap pattern). P2 `seq` mode diffs the state streams per
encounter scenario (aggro → attack → flinch → death). Scenario windows are the seq
mode's home turf (its documented constraint, §206).
**SETTLED §213 — extractor + worked example delivered:** `tools/foundry/state_map.py`
mechanically extracts any donor actor's state machine (enums, mode-switch
dispatchers with case labels, every transition write — all verbatim quotes with
line numbers; semantics stay [INFERENCE-NEEDED] by design). Worked example:
`donor-statemap-bk.md` — the Bokoblin: **master action dispatcher (d_a_bk.cpp:3384,
24 action states), 24 mode sub-machines, 232 line-cited transition writes.** That
artifact IS the restoration spec skeleton and simultaneously the tap vocabulary
(the values both taps log and the differ compares).
**First target:** whichever enemy family History restores first; run `state_map.py`
on its donor source, restore from the artifact, verify with the tap pair.
**P13 pipeline receipt (Pig, §219):** History assessed WW Pig AI via this pipeline →
correctly identified as NOT-YET-IMPLEMENTABLE (donor bait subsystem absent in the
receiver) — the pipeline's job includes saying "blocked on missing subsystem"; the
bait dependency is now a named prerequisite, not a surprise.

## P13b — regular NPC AI + pathing (chartered §219)

**The finding that shapes it:** WW NPCs do NOT use the enemy mMode-switch idiom.
Their states are **member-function pointers** installed via
`set_action(&Class::method, arg)` (`d_a_npc_ko1.h:93`), with rosters like
`wait_action1..4` / `hana_action1..5` / `event_action`. `state_map.py` now extracts
BOTH idioms (mode-switch + set_action graph + method roster). Worked examples:
`donor-statemap-npc-bm1.md` (Grandma — 13 set_action transitions + 10 mode writes)
and `donor-statemap-npc-ko1.md` (kid — full 12-method roster).
**Pathing:** NPC↔path binding is per-actor (params byte or hardcoded targets — ko1
walks WITHOUT engine paths; grep-verified). Method: `param_map.py` per NPC + the
P12 path tables; [INFERENCE-NEEDED] until each actor's decomp names its path use.
**Talk/event layer:** NPC behavior is event-framework-coupled (`event_action`,
EVNT); verdicts on talk states ride the existing JStudio/event traces, not a new
instrument.
**Receiver note (covenant):** the port's crew NPCs run on the ext-actor socket
(population/mount) — donor set_action graphs are the SPEC for their behavior
scripts; the state tap mirrors log the CURRENT action-method name (string), which
the seq differ compares directly against donor rosters.

## Covenant laws binding P9–P13 (user ruling, bus §211)

1. **Donor-verbatim law.** Every oracle/harness (P9 WHEN-tables, P10 layer oracle, P13
   state machines) transcribes the donor's decompiled code **to the letter** — same
   conditions, same constants, same order. No receiver-idiom substitutions.
2. **Inference-flag law.** Where donor behavior is vague (undecompiled branch, unclear
   constant, ambiguous flag), the item is **FLAGGED `[INFERENCE-NEEDED]` and NOT
   implemented**. Flags accumulate in each artifact; a flagged row can never carry a
   MATCH verdict. Vagueness is surfaced, never papered over.
3. **Naming-agnosticism law.** Repo-side code and artifacts stay donor-agnostic in
   naming (legal wall): generic sockets/harnesses in the repo; donor-named inhabitants
   and donor-derived data live in the mod folder / capture archives, per the covenant
   (M6 greplist governs anything pushed).

## P13c — enemy fast-implementation doctrine (§222; the transmutable route)

**The finding that changes the economics:** WW enemy actors are **86-92%
receiver-native by measured API surface** (`api_surface.py`: pig/kb 92.3% missing 14 ·
Bokoblin/bk 85.8% missing 31 over ~5,000 lines · ChuChu/cc 86.7% missing 24) — same
engine lineage, so **direct source port of the donor actor is the DEFAULT route**;
5,000 lines is mechanical when 86% of its API already exists. Chassis-transplant
(receiver enemy chassis + donor decision tables) is the fallback for low-coverage
actors only.

**The recipe (per enemy, repeatable):**
1. `api_surface.py <donor.cpp> <donor.h> --receiver <root>` → coverage + MISSING
   list (= the literal work plan).
2. Check the missing list against the **shared-shim library** — the lists CLUSTER
   into recurring WW support subsystems: `setBt*` (battle coordinator that meters
   simultaneous attackers), `enemy_fire`, `piyo` (dizzy stars), `dSnap_RegistFig`
   (figurine snap — already partially handled in the ext mount, vexp),
   `ice*DL`, `JntHit`. **Each shim is built ONCE as an agnostic ext socket and
   amortizes across the whole enemy roster** — that is the transmutability.
3. Port the donor file with WW→TP API renames + the shims; covenant: donor logic
   verbatim, shims are plumbing not behavior.
4. Verify per encounter scenario: state tap pair + P2 seq differ vs the
   `state_map.py` skeleton (aggro → attack → flinch → death).
5. Priority order falls out of the index + shim overlap: enemies sharing
   already-built shims come nearly free after the first.

**Artifacts:** `enemy-port-surface-{kb,bk,cc}.md` (the first three work plans);
run the scan on any candidate before scoping its port.

## P14 — Lighting parity (chartered §214)

**Donor side (delivered):** the sweep captures Pale (full actor/bg C0+K0 color sets)
and Virt (sky sets); `tools/foundry/palette_maps.py` emits per-stage color-law
artifacts (`donor-palettes-sea.md`: 57 palettes + 37 sky sets shipped). Colors are
numbers → straight P2-differ fodder; the §191 grass K0 measurement was this system's
first live receipt (dungeonlight K0 = a Pale-family color). **[INFERENCE-NEEDED]:**
EnvR/Colo selector tables (which palette WHEN — weather × time-of-day) are unparsed;
transcribe the decomp's dStage EnvR/pselect structs (donor-verbatim) to close them.
**Donor runtime tap:** kankyo palette-blend poll (§189 reach list) for the
interpolated in-game truth on top of the authored tables.
**Receiver side (vexp-located):** `d_kankyo.cpp` (680 dependents) +
`d_kankyo_wether` + the existing WW sky-recipe/purple-black work
(`docs/state/lighting-purple-black-research.md`, `docs/WW Linked/islands/Outset/
sky-recipe.md`) — the parity harness plugs into an ACTIVE lane investigation:
palette-table diff (authored) + blend tap diff (runtime) will decide purple-black
class bugs by measurement.

## Cross-cutting: multi-type identity (bus §214, the Ikada correction)

One donor NAME can host many entities — subtype selected from placement params
(`daObj_Ikada`: mType picks Beedle ship / submarine / raft / salvage; the donor's
own header names them isCrane/isFlag/isBonbori). **Registry:
`docs/WW Linked/multitype-actor-registry.md` — 355 of 672 placed names carry
multiple distinct params.** Consequences, encoded in the tools: census identity is
(name, params) donor-side; name-level MATCHes on multi-type names auto-flag
[MULTI-TYPE, INFERENCE-NEEDED]; `tools/foundry/param_map.py` extracts any actor's
params decode verbatim (worked example `donor-parammap-ikada.md`). Receiver census
line needs params added to bind subtypes — Engine-queue item (one line, emitter-tap
precedent). The same discipline covers systems/props/body-parts/face-pane selectors:
if params (or a donor field) selects it, the selector value is part of identity.

## Standing notes

- **vexp**: first call for every "where is the receiver counterpart" question (it
  found the §208 heads gate and the layer machinery in one call each). Budget-aware.
- **OpenRCT2's platform lesson**: all future golden/checksum artifacts pin to our
  RelWithDebInfo x64 build — never cross-compare configs.
- Instruments before verdicts, Housing audits before trust, UNKNOWN ≠ MATCH —
  unchanged (oracle-stack doctrine governs all five).
