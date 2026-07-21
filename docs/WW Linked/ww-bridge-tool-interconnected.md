# WW Bridge Tool — Interconnected Run Doc

**What this is:** the design + phase plan for the offline toolchain that lets a player with
their own legal Wind Waker copy produce a working WW-Crew-Restoration install, with zero
Nintendo bytes distributed by us. Authored by the **Housing Security / containment auditor**
chat (2026-07-19) from a live repo + mod-folder audit. It is a RUN DOC for a **two-party lane**:
**Housing Security (auditor)** ↔ **Bridge (Cursor)**, with the user ferrying and ratifying.
**History is not a participant in this lane** (user-decreed 2026-07-19) — they own the Outset/WW
content lane, and no work, question, or sign-off is routed to them here. **The user may point
History at this doc for orientation** — to keep the WW goals visible across lanes — so read it as
context, not as a request: History owes this doc nothing. Anything here that turns out to be a
content-semantics question goes back to the user for routing.

**Related docs:** [WW-Restoration-Cookbook.md](WW-Restoration-Cookbook.md) (§0 Doctrine is the
charter, §1–4 the pipeline this tool automates) · [state/cut-actors-demo-restore.md](state/cut-actors-demo-restore.md)
(the №-ledger; №22 split-transform, №28 audio guard, №31 purity law, №36-D TGDR stride,
№38 audit standards, №52-C bak-skip) · [Interconnected Chats/Cut-Actors-Demo-Restore-Cursor-History.md](Interconnected%20Chats/Cut-Actors-Demo-Restore-Cursor-History.md)
(Phase M rehome, M0 disclosure ledger, M5/M6 gates).

---

## 0. Goal

> A player with (a) a released Dusklight build and (b) their own legal US copy of Wind Waker
> can produce a working WW-Crew-Restoration install with no hand-authored data, and we
> distribute zero Nintendo-derived bytes.

Everything below serves that sentence. If a phase does not move it, it is out of scope.

---

## 1. Precedent — A Tale of Two Wastelands

TTW merges Fallout 3 into Fallout: New Vegas. It is the closest shipping analogue to this
project and it has survived a decade of publisher scrutiny, so its shape is worth copying
deliberately rather than rediscovering.

| TTW | Dusklight WW |
|---|---|
| New Vegas engine = receiver | TP / Dusklight exe = receiver |
| Fallout 3 = donor assets only; its engine never runs | WW = donor assets only; WW code never runs |
| Installer converts from the player's own FO3 install | `ww_bridge` converts from the player's own WW extraction |
| Bethesda repeatedly verified no assets were bundled — the installer exists FOR that reason | Receiver Covenant + never-commit rule; same reason |
| Long install time, accepted by users | Offline conversion cost is fine; boot-time cost would not be |
| Required NVSE (script extender) to reach the closed engine | **We own the source** — generic capability goes straight into the exe |

**The load-bearing lesson: it is one-way.** Nobody "mods Fallout 3" to make TTW work. Equally,
**nobody mods Wind Waker to make this work** — and therefore *WW's incomplete decompilation is
not a dependency of this project*. We read WW **file formats** (stable, understood); we consult
the decomp for **semantics** (what an actor param means) as a reference document. Those are
different things, and only the first is a hard requirement.

**Where we are structurally luckier than TTW:** TP and WW are both JSystem / Nintendo EAD
GameCube titles. They share container format (RARC), model format (J3D — BMD and BDL are
dialects of one thing), collision (DZB) and room data (DZR). TP's engine already reads all of
these natively. That is why `adapt_bdl_arcs.py` is ~9 KB doing four transforms rather than a
format converter: **it normalizes dialect, it does not translate.**

---

## 2. Architecture — three tiers, divided by WHEN the work happens

**Tier 1 — Offline, install time: the bridge tool.** Everything requiring WW *format*
knowledge. Runs on the player's machine against the player's extraction. Never ships data.

**Tier 2 — Runtime, generic: the Dusklight exe.** Systems that do not know what WW is. Already
built, already covenant-clean (Phase M).

**Tier 3 — Folder data: ini + CSV.** The mapping — which WW thing becomes which TP thing,
where, under what conditions.

### Why conversion is offline, not runtime

1. The exe stays covenant-clean — no WW format knowledge in the binary.
2. Conversion artifacts are inspectable when something looks wrong.
3. No boot or FPS cost (see [build-fps-guidelines.md](build-fps-guidelines.md)).
4. A Python script can fail safely; an exe crash is a crash.
5. TTW precedent: a long *install* is accepted; the same cost at *boot* would not be.

### The "two-way street" question — resolved

The street is one-way; **the ini is the customs declaration.** WW content flows in, the ini
declares what it is and how TP should treat it, nothing flows back. What *feels* bidirectional
is TP systems **adopting** WW props — TP's wallet crediting a WW rupee (№36-C), TP's native
knob-door demo opening a WW door (№53). That is not reverse data flow; it is the existing
`pickup_rupee=` / `knob=1` keys working as designed. **The architecture already answers this.**

### Tier 3 has a ceiling — declare it now (proposal)

`population/actor_map.ini` already carries a `[layers]` section mapping story layers to quest
flags. That is control flow in a config file. It is the right call today, but each new feature
(conditional dialogue, spawn conditions, quest sequencing) adds a keyword, and the failure mode
is a badly-designed language discovered too late.

**Proposed boundary: ini for declarative facts, DuskScript (the Lua lane) for behavior.** The
bridge produces data; DuskScript consumes it for quest logic. Naming this now costs nothing and
prevents a migration later. *Bridge: ratify or amend.*

---

## 3. Validation stance — RECOMMEND clean, do not enforce it (user-decreed, 2026-07-19)

TTW refuses modded installs outright. **We deliberately do not**, and this is a technical
judgement, not only a preference:

- Dusklight **is itself a mod platform**. Refusing a modded TP would refuse Dusklight's own
  users.
- The overwhelming majority of current Dusklight mods are **texture replacements**, which live
  in the `model_replacements` overlay system — downstream of, and orthogonal to, everything the
  bridge reads or writes. They cannot perturb the conversion.
- TTW's strictness exists because FO3/NV mods alter the **ESM/plugin data their conversion
  consumes**. Our equivalent input is the player's WW extraction, which is far less likely to
  be modded, and our TP-side output is a self-contained mod folder.

**Therefore:**

| Condition | Behavior |
|---|---|
| Clean US TP + clean US WW | Supported. The recommended configuration. |
| Modded TP (esp. texture mods) | **Allowed.** Warn once, proceed, user accepts risk. |
| Modded WW extraction | Allowed, warn more loudly — this *is* bridge input. |
| Non-US region (either) | Warn + proceed if parsable; **v1 is pinned/validated US only.** |
| Corrupt / unparsable / missing arc | **REFUSE**, name the file and the reason (fail-closed, §0). |

**Diagnostic-first, not wall-first:** when the tool refuses, it reports *what* mismatched, never
a bare failure. A refusal logs a line; a guess summons Ganondorf.

**Region:** both source games are US. v1 pins US, hash-records what it reads, and treats other
regions as warn-and-try rather than refuse.

---

## 4. Where the code lives — REVISED (user-corrected 2026-07-19)

**Recommendation: OUTSIDE this repo — its own project, with an explicit versioned contract.**

> *Auditor's first take was `tools/ww_bridge/` in-repo, argued from version lockstep. The user
> corrected it on grounds that outweigh that argument; the revised position and the replacement
> mitigation are below. Recorded rather than silently swapped — the reasoning matters for §9.*

**Why outside:**

1. **This repo is a fork.** It is ALBW-Dusklight, not main Dusklight (workflow: push upstream to
   ALBW-Dusklight, not origin). A tool welded to fork-local schema is locked to a *divergent
   branch that may never merge* — the "lockstep" I wanted would have been lockstep with a moving
   target, which is worse than no lockstep at all.
2. **The end state is Dusklight's own mod API** (exists, non-released). The bridge should drop in
   as a **code mod** against that API, which means it must carry **zero fork assumptions**.
3. **Extraction-readiness is therefore the primary design constraint, not a fallback.** Build it
   standalone from day one; retargeting later is far more expensive than starting decoupled.

### Replacing lockstep with an explicit versioned contract

The original concern was real and does not disappear: the tool emits CSVs and manifests the
engine parses with exact-match-or-refuse semantics (E1), and a mismatched pair fails as a
*silently empty island*. Out-of-repo, the fix is to make the coupling explicit:

- **Declare a schema version** covering the census CSV columns and the manifest key set.
- **The tool stamps it** (`schema_version=N`) into what it emits; **the engine checks it** and
  refuses-or-warns on mismatch, naming the mismatch.

This is **strictly better than same-repo lockstep**, which is only an *implicit* guarantee that
breaks silently the moment a mismatched pair ships. An explicit handshake fails loudly — which is
this project's own doctrine (E1, fail-closed, §0) applied to the tool boundary.

### Keep the host-API surface deliberately tiny

The offline tool needs three things from its host: **where the WW extraction is, where the mod
folder is, and how to log.** That is the whole surface. Define it as a thin adapter now,
implement it against today's reality (the `%AppData%` path), and swap the implementation when
Dusklight's real API ships. **Depending on more than those three is speculation against an
unreleased interface** — the smaller the surface, the cheaper the retarget.

### Bigger implication — the receiver itself may want to be a code mod

If Dusklight ships a native code-mod API, then the receiver (`d_ext_npc_mount` + population +
doors + mod_flags — the ~5,300 lines in G1) is a candidate to become a **plugin rather than
forked engine code**. That would dissolve the fork problem structurally:

> **WW-Crew-Restoration = code mod (receiver) + data (mod folder) + bridge (offline tool)** —
> with nothing needing to merge upstream at all.

It reframes G1 from *commit debt* into *plugin prototype*. **Bridge should rule on this before more
receiver code accretes**, because the longer it grows as forked engine code, the more expensive the
eventual port becomes.

### Sub-recommendation: split the TOOL from the MOD CONTENT (unchanged)

`tools/ww_crew_restoration_skeleton/` currently holds **both** the tool scripts (`adapt_bdl_arcs.py`,
`extract_amori_census.py`, `install_skeleton.py`, `seed_o2_content.py`) and **this specific mod's
content** (`npc/*.ini`, `dialogue/`, `population/`). Conflating them means the toolchain cannot be
reused for a second WW mod or a second donor game. The tool half is what moves out; the skeleton
stays as this mod's source-of-truth content. The bridge **reads the skeleton's manifests to
discover what it needs** (see B3), so the skeleton stays self-describing and there is no second
list to drift.

**Player distribution is a release-artifact question either way.** Players never clone a repo;
they download a zip or an installer. TTW ships an installer built from a source tree nobody
clones. Repo layout and player packaging stay decoupled.

---

## 5. Current baseline (audited 2026-07-19 — what is actually missing today)

Grounding for the Bridge chat. All figures verified against the working tree and live mod folder.

| # | Finding | Evidence |
|---|---|---|
| G1 | Receiver engine is **uncommitted**: `d_ext_npc_population.cpp` (524), `d_ext_npc_doors.cpp` (861), `d_ext_mod_flags.cpp` (157) + headers = 1,597 lines **not in HEAD**; `d_ext_npc_mount.cpp` carries +3,758/−273 uncommitted; `files.cmake` registration uncommitted | `git status` / `git cat-file -e HEAD:` |
| G2 | Arcs need **adaptation**, not just extraction (BDL4→BMD3, litmask, TEV C-reg, DZB through-flags). Raw WW arcs = crash or fall-through-the-island | Cookbook §1 |
| G3 | **Census CSVs are required at runtime and absent from the repo.** `ext_bg0.ini:22 population=outset_placements.csv`; six more `ext_bg*.ini` bind `interior_placements.csv`. Repo skeleton `population/` has only `actor_map.ini`, `doors.ini`, `identity.ini` | manifest grep + dir listing |
| G3b | **No general extractor exists.** `extract_amori_census.py` is hardcoded to A_mori→`interior_placements.csv`; `_dbg_dzr.py` hardcodes the A_mori arc path. Nothing can produce `outset_placements.csv` from a player's extraction | script docstrings |
| G4 | **Skeleton drift**: live-only `ext_bg9.ini`, `npc_bk.ini`, `npc_lwood.ini`, `npc_mo2.ini`; repo-only `npc_kusa.ini`, `npc_rflw.ini`; `actor_map.ini`/`doors.ini` differ in size | repo↔live diff |
| G5 | 10 arcs in `arcs/` unreferenced by any manifest: `Cave09 Cb Ebrock Krock_01 Kt Kusa Rflw Vhkak Vhutu VkeyN` — staged or dead? **OUT OF LANE** — mod-content question; user routes to the Outset lane. Bridge must not delete | arc↔manifest cross-ref |
| G6 | `arcs_quarantine_full_object_dump/` = 552 raw WW arcs parked inside the live mod folder | dir listing |

**Consequence of G3 if we shipped today:** a player would get island geometry, doors and
interiors, and **not one person, pig, pot or blade of grass.** Manifests resolve, census is
empty, Outset comes up deserted.

**Confirmed healthy:** all 46 manifest-referenced arcs present; no machine-specific paths in any
runtime-read config (only `batch_adapt_log.txt`, not read at runtime); no config references to
the quarantine dump or repo tools; new folders auto-enable in load order
([custom_assets.cpp:1456](../src/dusk/custom_assets.cpp), "unlisted = enabled" at :986); no
settings toggle gates ExtNpc; dialogue's 21 `folk.*` sections are byte-identical repo↔live and
`ww_dialogue_full.txt` (265 KB) is a research dump, **not** load-bearing.

---

## 6. Phases

### B0 — Generalize the census extractor *(the actual unblock)*

`ww_bridge census <arc> --out <csv> [--stage NAME] [--append]`. Parameterize the four hardcoded
constants (`ARC`, `CSV`, `STAGE`, `MOD`), drop the A_mori-specific debug keys
(`extract_amori_census.py:103`), generalize the re-run block-drop (:108) to key off `--stage`.

**Leave `list_rarc()` / `entry_size()` / `parse_dzr()` (lines 28–83) UNTOUCHED.** They are proven
against 990 rows across two stages; rewriting them risks playtested content for no gain.

**Acceptance (non-negotiable):** regenerate `outset_placements.csv` (475 rows) and
`interior_placements.csv` (515 rows) from existing arcs and **diff byte-for-byte against the live
files.** Clean diff = proven in one shot. Keep the diff as a permanent regression check.

*Effort: small — an afternoon. Unblocks reproducibility entirely.*

### B4 — `verify` + `gate` *(do this SECOND — cheap, protects everything after)*

`ww_bridge verify` = automated M5c: every `arc=` present; every `population=` CSV present with
rows for the declared stage; every `dialogue=` key resolving to a real section; no surviving
`J3D2bdl4`; no absolute paths in runtime configs; skeleton↔live drift diff (catches G4).

`ww_bridge gate <exe>` = the M6 greplist gate, permanently killing the CRLF false-CLEAN class
(observed 2026-07-19: a CRLF pass reported all-zero including known-good hits).

Converts auditor checks A and E from ad-hoc shell into CI-runnable commands.

### B1 — Census schema: document, don't force *(low priority)*

Two shapes exist — `outset_placements.csv` has no `stage` column, `interior_placements.csv` does.
**This is not a defect:** `ext_bg0.ini` omits `population_stage=` and takes all rows; `ext_bg1..9`
set it and filter. To collapse them, have B0 always emit `stage` and treat a missing
`population_stage=` as "accept all" — backward-compatible superset. Touches the resolver, so
Cursor owns it. **Must not block B0.**

### B2 — Chunk coverage: SCLS and friends

Today the extractor handles `ACT*`, `SCO*`, `TGOB/TGSC/TGDR/Door/DOOR/TRES/PLYR`. The census
research counted **31 chunk types**, so real data is skipped — most importantly **SCLS**, the
exit table the cookbook's §4.1 door binding depends on. `doors.ini` is currently hand-authored
from a manual SCLS decode. An `scls` subcommand makes door binding *generated*, which removes a
class of transcription error and generalizes from "Outset's doors work" to "any WW interior's
doors work." **This is where the tool stops being a census dumper and becomes a bridge.**

### B3 — One player-facing command *(promoted — TTW says the installer IS the product)*

`ww_bridge install --ww-root <extracted ISO> --stage Outset` — copies needed arcs, adapts them,
extracts census + SCLS, writes CSVs, installs the skeleton.

**It discovers what it needs from the skeleton itself:** collecting every `arc=` from `npc/*.ini`
yields the 46-arc requirement list (the auditor did exactly this by hand). Adding content to the
mod automatically updates what the installer pulls — no second list, no drift.

Earlier phases should be **shaped as things the installer will call**, not standalone scripts
someone later wraps.

### B5 — The general bridge

Keep the **WW-format reader** separate from the **TP-convention writer**. Reader understands
RARC/DZR/DZB/J3D; writer understands manifests, CSVs, TP coordinate and collision conventions
(№22 split-transform). Hold that line and new content is a new *manifest*, not a new *parser* —
which makes Windfall, Forest Haven and the Phase GS sea grid incremental. Natural later addition:
**BMG dialogue extraction**, closing the last gap where Nintendo's words would otherwise travel
with the mod.

### Suggested order

**B0 → B4 → B2 → B3**, with B1 and B5 opportunistic. B0+B4 is a small, high-leverage first slice
and the point at which "commit and a player could do this" becomes true.

---

## 7. Risks

- **R1 — Regeneration drift.** Once census generation is a tool, regenerating can silently change
  content under a validated build. Mitigation: B0's byte-diff, kept permanently.
- **R2 — Version skew.** Both games are US (settled). Still hash-record what the tool reads so a
  future non-US or HD extraction is a *named* warning, not a quietly wrong island.
- **R3 — Tool/engine schema split.** Now a *live* risk, since §4 puts them in separate projects.
  Mitigated by the **explicit `schema_version` handshake** (§4), not by co-location. If that
  handshake is not built, this becomes the project's most likely silent-failure source.
- **R4 — G1 dominates everything.** Until the ~5,300 uncommitted lines are reachable by other
  players — whether by upstream merge **or by becoming a code mod** (§4) — no bridge phase makes
  the mod usable by anyone else. **Distribution of the receiver is the true critical path**, and
  §4 opens a second route to it that may be cheaper than merging a fork.
- **R6 — Building against an unreleased API.** Dusklight's code-mod API does not exist publicly
  yet, so any dependency on it is speculation. Mitigated by holding the host-API surface to three
  calls (paths ×2, logging) and keeping the offline tool pure-stdlib — the offline half needs
  essentially no host API at all, which is what makes this affordable.
- **R5 — Save compatibility unaudited.** Mod flags are save-scoped; whether a save made *with* the
  mod loads safely *without* it is untested. Pairs naturally with the outstanding M5b
  folder-absent smoke.

---

## 8. Ownership

**Two-party lane. History is not a participant** (see header).

- **Bridge (Cursor):** critique this doc, then execute B0/B4. Owns §4 ratification, the
  `schema_version` handshake on both sides, and B1's resolver change.
- **Housing Security (this chat):** spec, acceptance criteria, and running `verify`/`gate` against
  results. **Does not build the tool** — building B4 and then auditing with it is marking its own
  homework.
- **User:** ferries between the two chats, ratifies, decides commit timing, and routes anything
  that turns out to be a **mod-content** question (e.g. G5) to the Outset lane.

---

## 9. Open questions for the Bridge chat

1. **§4 (revised) — the big one.** Standalone project + `schema_version` handshake: agree? And
   what is the minimum viable handshake (a single integer? a key-set hash?) that the engine can
   check cheaply at manifest-parse time?
2. **§4 receiver-as-code-mod** — is the ~5,300-line receiver a plugin candidate once Dusklight's
   API lands, or does it stay forked engine code? **Answer before more receiver code accretes.**
3. **What does Dusklight's mod API actually expose** (non-released)? The bridge is being designed
   against three assumed host calls — WW-extraction path, mod-folder path, logging. Is that
   surface real, and is anything else guaranteed?
4. **§2 ini-vs-DuskScript boundary** — ratify now, or defer until the O2 schema pushes on it?
5. **B1** — collapse the census schema to one shape, or formally document two?
6. **B3 installer form** — Python CLI only, or eventually a GUI/wizard like TTW's?
7. ~~**G5** unreferenced arcs~~ — **withdrawn from this lane.** Mod-content question; the user
   routes it to the Outset lane. Bridge must not delete arcs either way.
8. Does anything here conflict with in-flight Outset work? **This plan must not disturb it.**

---

## 10. Sign-off

**Auditor: POSTED (2026-07-19).** Findings §5 are verified against the live tree; §1 precedent is
sourced; §3 stance is user-decreed; §4 is a recommendation awaiting ratification.

**Bridge chat: Phase A POSTED (2026-07-19)** — run doc
[Interconnected Chats/WW-Bridge-Tool-Cursor-History.md](Interconnected%20Chats/WW-Bridge-Tool-Cursor-History.md).
§4 APPROVE with amendments (integer handshake; phantom-mod receiver posture; offline tool =
CLI-only, no host API).

**Auditor: Phase A ASSESSED — RATIFY WITH FOUR ADDITIONS (2026-07-19).** See §11.

**User: _pending_.** *(No History line — two-party lane.)*

### Paste for the Bridge chat (2026-07-19)

> Housing Security posted `docs/ww-bridge-tool-interconnected.md`: the plan for an offline
> `ww_bridge` toolchain so a player with their own legal US WW copy can generate the census +
> adapted arcs locally (TTW model — convert on the player's machine, distribute zero Nintendo
> bytes). Audited baseline: the receiver engine is **1,597 lines untracked + 3,758 uncommitted**,
> and the census CSVs that `ext_bg0.ini` requires are **absent from the repo with no general
> extractor to regenerate them** — ship today and Outset comes up deserted.
>
> **§4 was revised by user correction:** the tool lives **outside this fork** as a standalone
> project so it can drop into Dusklight's own (non-released) code-mod API without carrying
> fork assumptions. The lockstep that co-location would have given is replaced by an explicit
> **`schema_version` handshake** — the tool stamps it, the engine checks it, mismatch refuses
> loudly instead of yielding a silently empty island. That correction also raises a larger
> question worth ruling on early: **should the ~5,300-line receiver itself become a code mod**
> rather than forked engine code? If so, WW-Crew-Restoration = code mod + data + bridge, and
> nothing needs to merge upstream at all.
>
> Proposed first slice is **B0** (generalize `extract_amori_census.py`; acceptance =
> byte-identical regeneration of the two existing CSVs) then **B4** (`verify`/`gate`). Please
> critique §2 architecture, ratify revised §4, and answer §9 — especially Q3, since the bridge is
> currently designed against three *assumed* host API calls. Do not disturb in-flight Outset work.

---

## 11. Auditor assessment — Bridge Phase A (2026-07-19)

**Verdict: RATIFY with four additions.** Phase A is better than this spec in three places and
worse in none. Lane discipline was correct throughout (Bridge bounced the unreferenced-arc
question rather than acting on it), and Q8's warn-allow-missing-meta protects in-flight Outset.

### Baseline correction — §5 is now partly STALE (good news)

Exe rebuilt 13:52. **Both leaks from the original containment audit are CLOSED:**

- LEAK-1 — `src/dusk/ui/editor.cpp:3251` now reads `display_name=RealName`.
- LEAK-2 — the WW proc-name test is gone, replaced by manifest-flag branching:
  `dExtNpcMount_lookup(me.proc, &pickupMan) && pickupMan.pickupRupee`
  (`src/d/d_ext_npc_population.cpp:402`). Only compile-away comments remain.

**M6 gate: CLEAN — 0 hits across all 16 patterns, ASCII *and* UTF-16LE.**

### On Bridge's UTF-16-aware gate proposal — tested, and it is DEFENSE not remediation

Every greplist term plus `Toripost` / `Vlupy` was checked encoded as UTF-16LE against the current
exe: **zero hits.** The ASCII-only gate was therefore not producing false negatives. Build the
wide-string read anyway (it is nearly free and closes the hole before an RML/Win32 wide path opens
one) — but record it as hardening, **not as a caught bug.** Accuracy in the ledger matters more
than credit.

### Conceded to Bridge

- **Q3 — the "three host API calls" adapter was API fiction.** `--ww-root` / `--mod-root` are CLI
  args; logging is stdout. An adapter with no second implementation is premature abstraction.
  Bridge is right: **keep argparse, drop the adapter.** Auditor withdraws it.
- **Q2 — the MO2 four-way vocabulary beats the auditor's binary framing.** Receiver-as-code-mod is
  a *ladder*, not a fork/plugin either-or, and **phantom-mod is a rung that works today** — it
  largely formalizes what Phase M already achieved, so it carries near-zero new risk and does not
  block Outset. Placing `ww_bridge` as the MO2 *manager-plugin* / TTW-installer analogue sharpens
  §4 rather than contradicting it.
- **Q1 — integer over key-set hash is correct**, and the load-bearing detail is one this spec
  underspecified: **missing `bridge_meta.ini` ⇒ warn once + v0 legacy**, so the live folder keeps
  booting through the first engine patch.

### Four additions (auditor requirements)

- **A-1 — a refused CSV is still a deserted island, just an explained one.** Refusing on schema
  mismatch is correct fail-closed behavior, but "loud" currently means loud *to a developer reading
  DuskLog*; a player sees an empty island and no reason. Surface it through the existing
  **conflict-badge mechanism** in `dusk::custom_assets` (load-order Phase 3) as a refused-population
  badge on the mod row. Cheap, and it converts a silent-looking failure into a visible one.
- **A-2 — `schema_version` covers tool↔engine; NOTHING covers source↔tool.** Different contracts,
  only the first is designed. Q1's table mentions an optional source hash; it is absent from B0's
  execution steps. **Make per-arc source hashing mandatory in B0**, not optional: once players
  regenerate census on machines we cannot see, "my Outset looks wrong" is unanswerable without
  knowing which extraction produced that CSV. Cheap now, expensive to retrofit (spec R2).
- **A-3 — the byte-diff must be permanent, not one-time.** B0 step 5 treats it as acceptance; spec
  R1 wants a standing regression check. Home is **B4 `verify`** — otherwise the second regeneration
  has no guard.
- **A-4 — the engine-side `kExtPopSchemaVersion` patch is itself subject to M6.** Neutral-named, so
  it will pass; stated so it is not assumed exempt.

### Reinforcing Bridge's own R7 (staging-copy trap)

Bridge correctly flagged that an in-tree "temporary" `ww_bridge` becomes the fork weld §4 rejected.
**Auditor goes further: prefer the sibling repo from commit one.** The proposed staging location is
inside `tools/ww_crew_restoration_skeleton/`, which would *also* re-conflate tool with mod content —
the one split both parties independently called for. It is doubly wrong, and "we'll move it before
B3" is exactly how architectural decisions get quietly reversed.

### Still open, unowned in this lane

- **R5 save compatibility** — mod flags are save-scoped; whether a save made *with* the mod loads
  safely *without* it is untested. Pairs with the outstanding **M5b folder-absent smoke**.
- The original containment audit's report is now stale (leaks closed). Worth a №-entry in the main
  ledger so a later reader does not chase two fixed leaks.

---

## 12. Auditor ruling — B0 acceptance (2026-07-19)

Independently re-run, not accepted on report. `regen-proof` executed into a scratchpad dir;
tool write-audit done first (every write targets the parameterized `out_dir`; live is read-only,
`__main__.py:67`).

### B0 Outset: **PASS — VERIFIED**

`cmp` clean and `md5 = e0302104125634c4ee1ea911c317145d` on both live and regenerated
(27,599 bytes / 475 rows). And the matched data is *good*: **74 % of Outset rows carry nonzero
rotation** — so this is byte-identity with correct content, not with a lossy file.

### Interior "DRIFT": **REFRAME — this is not drift. The tool is right and the LIVE CSV is LOSSY.**

**Position-only intersection: 571 / 572 = 99.8 %. Live rows with no position match in gen: ZERO.**
The parser reproduces every live row's placement exactly. There is no parse discrepancy anywhere.

The entire divergence is the **rotation columns**:

| stage | LIVE nonzero-rot | GEN nonzero-rot | verdict |
|---|---|---|---|
| A_mori | 34 / 172 (19 %) | 34 / 172 (19 %) | match |
| Ojhous2R1 | 53 / 59 (89 %) | 54 / 60 (90 %) | match |
| **LinkRM** | **0 / 18 (0 %)** | 16 / 23 (69 %) | **live lossy** |
| **Ojhous** | **0 / 30 (0 %)** | 24 / 31 (77 %) | **live lossy** |
| **Ojhous2** | **0 / 29 (0 %)** | 23 / 30 (76 %) | **live lossy** |
| **Omasao** | **0 / 11 (0 %)** | 9 / 12 (75 %) | **live lossy** |
| **Omori** | **0 / 209 (0 %)** | 122 / 219 (55 %) | **live lossy** |
| **Onobuta** | **0 / 41 (0 %)** | 32 / 43 (74 %) | **live lossy** |
| **Pjavdou** | **0 / 3 (0 %)** | 3 / 6 (50 %) | **live lossy** |

Sample (LinkRM) — identical but for `ry`:

```
LIVE: ACTR,-,MPot,00000000,379.3,40.0,-43.2,0,     0,0
GEN : ACTR,-,MPot,00000000,379.3,40.0,-43.2,0,-10922,0
```

**Diagnosis:** the live CSV for those seven stages was produced by an **earlier extraction that did
not capture rotation and wrote zeros**. A_mori and Ojhous2R1 match because they came from the
rotation-capable `extract_amori_census.py`. Outset likewise has real rotations (74 %).

**This is a latent content bug the new tool has just exposed**, and it matters: Cookbook §4.2 —
*"The census `ry` MUST be applied — doors face as WW authored."* **Every actor in those seven
interiors is currently facing default-north instead of as Nintendo authored.**

The remaining **+24 gen rows are `PLYR` spawn entries** (and kin) that the older process filtered
out — a separate, benign chunk-filter delta. Unmapped names hit E1 refuse-and-log, so they are
inert until `actor_map.ini` maps them.

### RULING on the golden CSV

1. **Do NOT keep live interior as golden.** It is lossy — 100 % rotation loss across seven stages.
   Freezing it would canonicalize the bug.
2. **Do NOT let Bridge refresh live either.** Applying real rotations visibly re-faces every actor
   in seven interiors, inside a build the Outset lane is actively playtesting. That is a **content
   change**, out of this lane (same principle as G5).
3. **Golden = the REGENERATED set.** A-3's standing byte-diff in B4 must reference regenerated
   goldens, *not* today's live interior CSV — otherwise `verify` would enforce the lossy file
   forever.
4. **Live refresh is a content-lane task**, executed with the Outset lane's knowledge and a
   playtest, since interior facings will visibly change. Auditor recommends it be done — the
   authored rotations are strictly more correct — but not by Bridge and not silently.

**B0 is ACCEPTED. Bridge may proceed to B4** with the golden-reference correction in item 3.

### Two corrections to Bridge's Phase D notes

- **"Live interior also has mixed CRLF+LF endings" — NOT REPRODUCED.** Measured uniformly CRLF:
  interior = 574/574 CRLF lines, zero LF-only. (Outset ends on a final unterminated CR line, which
  the tool already reproduces byte-exactly.) If a normalization step was added to fix mixed endings
  that were not mixed, it should be revisited.
- **"DRIFT (not a tool parse bug)" understates the result.** It is not merely *not a parse bug* —
  the parser is **provably correct at 99.8 % position identity**, and the tool **recovers data the
  live file lost**. That is a B0 success, not an ambiguous outcome.

### Unchanged from §11

A-1, A-2, A-4 stand as written. A-3 is amended per item 3 above.

---

## 13. Auditor assessment — B4 `verify` / `gate` (2026-07-19)

**Verdict: B4 ACCEPTED with one spec gap to close.** A checker reporting CLEAN proves nothing
until it is shown able to report DIRTY, so both were **negative-controlled**, not just re-run.

### Golden provenance: CORRECT

`goldens/{outset,interior}_placements.csv` are **byte-identical to the auditor's own independent
`regen-proof` output** (27,599 B / 34,906 B). Golden = regenerated set, **not** the 29,106 B lossy
live interior. §12 item 3 honored exactly.

### `gate` — PASS (negative control)

Planted a synthetic binary carrying ASCII `Outset`, **UTF-16LE `Aryll`**, ASCII `mdarm_wait`:

```
HIT  [ascii]    'Outset'
HIT  [utf16le]  'Aryll'
HIT  [ascii]    'mdarm_wait'
GATE FAILED (3 hits)          exit=2
```

All three caught, correct non-zero exit. `gate.py:11` strips CR (CRLF false-CLEAN dead) and `:12`
skips `#` lines, so the §-exemption block is correctly ignored.

**Note:** the UTF-16LE hit is one the auditor's own ASCII-only shell gate **could not have found**.
Bridge's hardening is therefore a concrete improvement over the auditor's instrument, even though
§11 correctly established there is no live wide-string leak today.

### `verify` — PASS (negative control), ONE GAP

Synthetic broken mod root; every error class fired:

```
FAIL  missing arc for a_missingarc.ini: NoSuchArc.arc
FAIL  missing population CSV for b_missingcsv.ini: nope.csv
FAIL  no rows for stage=GhostStage in good.csv (from c_ghoststage.ini)
FAIL  surviving J3D2bdl4 in Good.arc
FAIL  abs path in npc\d_abspath.ini:2
VERIFY FAILED (5 errors)      exit=2
```

**Auditor concern WITHDRAWN:** the A-3 golden diff is *not* opt-in. `__main__.py:115-122`
auto-regenerates to `_verify_scratch` when `--regen-dir` is omitted, so the standing diff always
runs; the scratch dir is gitignored. Correct as built.

**GAP-1 — the `dialogue=` key check is MISSING.** Spec §6 B4 requires "every `dialogue=` key
resolving to a real section," and Bridge's own Phase C B4 item 1 lists "dialogue keys" — but
`run_verify` (`verify.py:153-163`) calls arcs / population / bdl4 / abs-paths / golden / drift and
**never checks dialogue**. `verify` currently reports CLEAN without having tested that condition.
Low severity (the 21 `folk.*` sections were confirmed intact in §5) but it is an untested claim.
**Close before B4 is called done.**

**GAP-2 (minor) — skeleton drift is NAME-only.** `check_skeleton_drift` (`verify.py:137-142`)
compares filename sets only. §5 G4 also found *content* divergence in same-named files
(`actor_map.ini`, `doors.ini` differ in size repo↔live). Content drift is currently invisible.

### NEW FINDING — skeleton drift is GROWING, and the skeleton is the distributable

§5 G4 recorded 6 drifted manifests. `verify` now reports **12**:

> live-only: `ext_bg10.ini`, `ext_bg9.ini`, `npc_bk.ini`, `npc_lamp.ini`, `npc_lwood.ini`,
> `npc_mo2.ini`, `npc_opaper.ini`, `npc_otana.ini`, `npc_otble.ini`, `npc_plant.ini`
> skeleton-only: `npc_kusa.ini`, `npc_rflw.ini`

Six new live-only manifests since the morning audit (a new `ext_bg10` mount plus interior props),
i.e. the Outset lane is actively authoring **into the live folder only**. That is normal for
in-flight work, but **the skeleton is what ships to players** (§0 goal) — every manifest that never
lands there is content a player would not receive. Drift is trending the wrong way.

Not a Bridge defect and **out of this lane to fix** (content). Flagged so it is a *decision* rather
than an accident: `seed_o2_content.py` writes both sides, so the reconciliation path exists.

### Operational note for Bridge

The A-3 golden diff will **fire whenever the arcs change** — that is the guard working as designed,
not a bug. When the Outset lane re-adapts arcs, goldens must be **deliberately re-seeded**
(`seed-goldens`) with a note in the run doc. Please document that procedure so a future CLEAN→FAIL
transition is not misread as a tool regression.

### Standing

A-1, A-2, A-4 remain open (engine handshake slice). A-3 is **satisfied on the tool side**.
B0 + B4 tool side: **ACCEPTED**, GAP-1 to close.

---

## 14. Auditor confirmation — GAP-1 / GAP-2 closed (2026-07-19)

**Both CONFIRMED CLOSED.** Independently negative-controlled, plus a positive control (a gap fix
that fires on everything is as useless as one that fires on nothing).

### GAP-1 — `dialogue=` check: CLOSED

`_dialogue_sections` scans `dialogue/*.txt` for `[section]` lines; `check_dialogue` tests every
manifest's `dialogue=` against that set; wired third in `run_verify`.

```
NEG:  FAIL  dialogue section missing for x_baddlg.ini: [ghost.key]     ✅ fires
POS:  y_gooddlg.ini (dialogue=real.key, section present) flagged 0×    ✅ no false positive
```

### GAP-2 — same-name content drift: CLOSED

```
WARN  content-size drift npc/x_baddlg.ini: skeleton=69 live=28         ✅ fires
```

Live verify re-run: **CLEAN, 23 warnings** — reproduces Bridge's figure exactly.

### ESCALATION — the skeleton is not "drifting," it is roughly HALF-BUILT

GAP-2 was filed as *minor*. Turning it on proves that wrong. §5 G4 could only say "differ in size";
the check now quantifies it:

| Skeleton file | skeleton | live | ratio |
|---|---|---|---|
| `population/doors.ini` | 677 | **2755** | **4.1×** |
| `population/actor_map.ini` | 3031 | **5413** | **1.8×** |
| `npc/gnd.ini` | 195 | 296 | 1.5× |
| `npc/ext_bg1..8.ini` (7 files) | ~360–430 | ~406–498 | ~1.2× each |

Plus the 12 name-level drifts from §13. **A player installing from the skeleton today would receive
roughly a quarter of the door bindings, about half the actor map, and none of the 10 live-only
manifests.** Against §0 — "no hand-authored data, working install" — the distributable is currently
**not shippable**, and no amount of bridge tooling fixes that: `ww_bridge` generates *census data*,
not manifests.

This is **out of lane** (content) and **not a Bridge defect** — Bridge built the instrument that
made it visible, which is exactly what B4 was for. But it is now the **largest single gap between
today and §0**, larger than the engine handshake. `seed_o2_content.py` writes both sides, so the
reconciliation path exists; it just has not been run lately.

**Recommend:** the user routes a *skeleton reconciliation* task to the Outset/content lane, and
that `verify`'s 23 warnings be driven to zero before any player-facing release is contemplated.
Auditor will re-verify on request; the WARN count is now a usable progress metric.

### Standing

B0 **ACCEPTED** (§12) · B4 tool side **ACCEPTED, gaps closed** (§13/§14) · A-3 satisfied tool-side.
**Open:** A-1 Mods badge, A-2 engine-side, A-4 M6 on the engine patch — all in the engine handshake
slice. **Bridge is clear to proceed.**

---

## 15. Decomp validation of the DZR parse — §12 CONFIRMED, two fields missing (2026-07-19)

**Prompted by the user:** the auditor had **not** consulted the tww decomp, which Cookbook §0 names
AUTHORITATIVE for actor semantics/params. Most audit work is byte-level containment where WW
semantics are irrelevant — but **§12's rotation ruling is not**, and the content lane is being asked
to act on it. Corrected below.

### Authoritative layout (zeldaret/tww)

```c
struct stage_actor_data_class {          // ACT* — size 0x20
    /* 0x00 */ char name[8];
    /* 0x08 */ fopAcM_prmBase_class base;
};
struct stage_tgsc_data_class {           // SCOB/TGSC/DOOR/TGDR — size 0x24
    /* 0x00 */ char name[8];
    /* 0x08 */ fopAcM_prmBase_class base;
    /* 0x20 */ fopAcM_prmScale_class scale;
};
struct fopAcM_prmBase_class {            // size 0x18
    /* 0x00 */ u32   parameters;
    /* 0x04 */ cXyz  position;
    /* 0x10 */ csXyz angle;
    /* 0x16 */ u16   setID;
};
```

Absolute offsets within an entry: `params` **0x08** · `position` **0x0C** · `angle` **0x18** ·
`setID` **0x1E** · (SCOB only) `scale` **0x20**.

### Result 1 — the parser is CORRECT, and §12 HOLDS

`dzr.py:67-68` reads `>fff @0x0C` and `>hhh @0x18`; `:37-41` uses strides 0x20 / 0x24. Every one
matches the decomp exactly. **The field at 0x18 is genuinely `csXyz angle`** — so the regenerated
values are real authored rotations, not a misparse.

**§12's ruling is now decomp-backed, not inference-backed:** live's zeroed `ry` is data loss, gen
recovers Nintendo's authored facings, and the content lane may act on it. (Had this gone the other
way, adopting those rotations would have mis-faced every actor in seven interiors.)

### Result 2 — TWO FIELDS ARE NEVER CAPTURED

| Field | Offset | Applies to | Status |
|---|---|---|---|
| `setID` (u16) | **0x1E** | ALL entries | **not read, not in CSV** |
| `scale` (3× u8) | **0x20** | SCOB/TGSC/DOOR/TGDR | **not read, not in CSV** |

Header is `…,x,y,z,rx,ry,rz` — neither field has a column. Two observations:

- **`setID` is the engine's non-positional actor identity** (switch/event association). Cookbook §4.1
  notes door destination params are *sentinel* and binding is **POSITIONAL** — `setID` is precisely
  the identity that positional binding is standing in for. Whether the receiver needs it is a design
  question the auditor does **not** claim to answer.
- **`scale` was anticipated in the original census design** — Cookbook §2.3 specifies
  `layer,room,name,params,x,y,z,scale?,ry,extra`. It was dropped somewhere between design and
  implementation. Any non-uniformly-scaled SCOB prop is currently imported at authored-geometry
  scale with its placement scale discarded.

### Why this is a NOW decision, not a later one

`schema_version=1` was stamped **today** and goldens seeded **today**, and **nothing consumes either
yet** — the engine handshake is the *next* slice. Capturing these fields now costs a column add plus
`seed-goldens`. After the handshake ships it costs a schema bump, an engine-side migration, and
every player regenerating. **The window is open and it closes with the next slice.**

**Auditor recommendation:** Bridge captures `setID` (all entries) and `scale` (SCOB family) into the
census **before** the engine handshake lands, even if the receiver ignores them at first — a census
is an archival extraction, and a field not captured is a field re-extracted later. Consuming them is
a separate, deferrable decision.

**Auditor process note:** §12 was posted before this validation. It happened to survive, but the
ordering was wrong — parse-semantics claims should be decomp-checked *before* a content-lane
recommendation, not after. Recorded so the sequence is not repeated.

---

## 16. Auditor assessment — schema v2 + engine handshake (2026-07-19)

**Verdict: ACCEPTED.** A-1, A-2, A-3, A-4 all satisfied. Two minor fail-open edges below, plus one
runtime test that only the user can run.

### The critical path — legacy v0 — is CORRECT

The live folder has **no `bridge_meta.ini`**, so *every* boot today takes the legacy path. If it were
wrong, Outset would go dark. It is right:

- `checkPopulationSchema` returns `LegacyV0` when the file is absent (`:234-236`) or carries no
  `schema_version` key (`:261-263`).
- The `LegacyV0` branch (`:314-322`) warns **once** (deduped via `s_schemaLegacyWarned`), clears any
  stale badge, and **does not return** — execution falls through to census load and spawn.

Population proceeds unchanged on the current folder. ✅

### Mismatch path — correct and fail-closed

`:304-313` refuses the CSV, logs file-vs-engine versions with *"empty island prevented"*, sets the
A-1 note, and inserts into `s_spawnedBgProcs` so it cannot retry-spam per frame.

### A-1 badge is REAL, not inert — checked deliberately

`set_mod_status_note` has **two definitions**: the real one (`custom_assets.cpp:1809`) and an
**empty no-op stub** (`:2488`). That is the classic shape of a silently-inert feature, so it was run
down rather than assumed:

- the stub block sits under `#else // !TARGET_PC` (`:2465`);
- `TARGET_PC` is in `GAME_COMPILE_DEFS` (`CMakeLists.txt:369`) — the **real** implementation compiles;
- `mods.cpp:144` reads `mod_status_note`, so it renders end-to-end.

A-1 is wired. ✅

### Column APPEND was the right call — and it is what makes v1/v2 coexist

v2 appends (`…,rx,ry,rz,set_id,sx,sy,sz`) rather than inserting. The engine row parser requires
`cols.size() >= base + 10` (`d_ext_npc_population.cpp:402`) and ignores extras, so **v1 (10 cols) and
v2 (14 cols) both parse**. Had the new fields been inserted mid-row, every existing CSV would have
silently mis-parsed — coordinates shifting into rotation columns. Worth recording as a rule for
future schema growth: **append only.**

### v2 field capture matches the decomp exactly

`set_id` `>H` @ **0x1E**; `scale` u8×3 @ **0x20/0x21/0x22** — precisely §15's offsets. `parse_dzr`
left verbatim with a new `parse_dzr_full` alongside, honoring the standing do-not. ✅
`bridge_meta.ini` carries `schema_version=2`, `tool_version=0.2.0`, and per-arc SHA-256 (**A-2** ✅).

### A-4 — M6 on the new build

Exe rebuilt 16:24 (25,501,184 B). **`gate` CLEAN — 0 hits, ASCII + UTF-16LE.** The only WW token in
the new handshake code is a comment, which compiles away. ✅

### Two minor fail-open edges (low severity, cheap to close)

In an otherwise fail-closed design these are the only asymmetries:

1. **Explicit negative version reads as legacy.** `found` is initialized to `-1` and `found < 0` maps
   to `LegacyV0` — so `schema_version=-1` is indistinguishable from *"no key present"* and fails
   **open**. Fix: track *key seen* separately from *value*; key-seen + negative ⇒ `Mismatch`.
2. **Present-but-unreadable meta reads as legacy.** `if (!in)` (`:233-236`) cannot distinguish
   *absent* from *permission-denied / corrupt*. A meta that exists but cannot be read is evidence
   something is wrong; arguably it should refuse rather than proceed.

Neither is reachable by normal tooling — `ww_bridge` never emits a negative — so these are hardening,
not defects.

### The one thing the auditor CANNOT verify: runtime

All of the above is code-reading and static verification. **No boot was performed.** Recommended
2-minute runtime acceptance, ideally in the next playtest:

1. **Legacy proof:** boot as-is (no meta present) → Outset populates, exactly one
   `treating as legacy v0` line in DuskLog.
2. **Mismatch proof (A-1 end-to-end):** drop a `population/bridge_meta.ini` containing
   `schema_version=99` into the live folder → boot → expect refusal log **and a visible `[POP!]`
   status note on the mod row** → delete the file → boot → badge clears.

Step 2 is the only way to prove the badge actually reaches a player's eyes; everything upstream of
the pixel is confirmed.

### UPDATE — step 1 CONFIRMED IN THE WILD (log `dusklight-20260719-165801.log`)

A user run at 16:58 exercised the legacy path for real:

```
4300 [WARNING] [ExtNpcPop] no bridge_meta.ini schema_version under 'WW-Crew-Restoration'
                — treating as legacy v0 (one-time warn)
4302 [INFO]    [ExtNpcPop] spawn #1 name=Tpost proc=NPC_TPOST …
4338 [INFO]    [ExtNpcPop] BG 'EXT_BG0' spawned 71 actors (4 skipped/unmapped) unique_folk=12
6745 [INFO]    [ExtNpcPop] BG 'EXT_BG1' spawned 3 actors (3 skipped/unmapped) unique_folk=1
```

**Legacy warn count = exactly 1** (dedup works), population proceeded immediately after, Outset
populated 71 actors, no crash. **§16 runtime step 1 is closed — the handshake's critical path is
verified at runtime, not just by code reading.** Step 2 (mismatch → `[POP!]` badge) remains untested.

Incidental health data from the same run: `EXT_BG1` resolved only 3 of 6 rows (3 unmapped) — an E1
refuse-and-log hole set for the content lane, not a defect.

### Standing

**B0 · B4 · schema v2 · engine handshake: ALL ACCEPTED.** A-1–A-4 closed. Bridge has no auditor
blockers. Remaining work is **content lane**: skeleton reconciliation (§14), live census refresh +
meta (§12 rotations), and the runtime acceptance above.

---

## 17. CONTAINMENT FINDINGS from the 2026-07-19 FPS-bisect logs

Two findings surfaced by the FPS lane's runs. Both are **containment-lane**, both are **live**, and
one **corrects an incorrect assessment the auditor made this morning**.

### F-1 — №52-C is HALF-IMPLEMENTED: parked folders still overlay files

The `.SKELETON*` / `*_BAK` / `.DISABLED` name-skip exists in the **ExtNpc provider scan**
(`d_ext_npc_mount.cpp:1054-1060`) — actors are correctly skipped. It does **not** exist in the
**`custom_assets` overlay scan**, which is a separate layer. Proof, run `dusklight-20260719-165303.log`:

```
51:  [custom_assets] 'WW-Crew-Restoration.SKELETON_BAK': 4 overlay file(s) (e.g. res/object/vhkak.arc)
110: [custom_assets] 'WW-Crew-Restoration.DISABLED': 59 overlay file(s), 4 shadowed [top wins]
116: [custom_assets] conflicts 'WW-Crew-Restoration.SKELETON_BAK': 4 win(s), 0 loss(es)
```

Neither line carries the `[disabled]` marker the formatter emits for disabled sources
(`custom_assets.cpp:1569`) — **both folders were live.** `arcs/<Name>.arc` mounts as
`res/Object/<Name>.arc` (`:1538-1540`), so the parked SKELETON_BAK injected `vhkak/vhutu/vkeyn/vlupy`
into the game tree, and the parked main folder injected 59 files including
`res/stage/f_sp115/r02_00.arc`.

**Net: a folder parked for backup is inert for ACTORS but fully active for FILES.** Under the
Receiver Covenant — *"the vanilla exe **+ game tree** must contain ZERO WW content"* — that is a
game-tree breach.

**AUDITOR CORRECTION.** The morning audit said of SKELETON_BAK: *"Containment is unaffected (it has
no `files/` tree, so nothing overlays) — robustness flag only."* **That was wrong on both counts.**
`arcs/` is an overlay source independent of `files/`, and SKELETON_BAK's four arcs did mount. The
claim was asserted without verification; the log disproves it.

**Proposed minimal fix (Cursor's call, not Bridge's):** apply the same suffix skip used at
`d_ext_npc_mount.cpp:1054-1060` to `custom_assets`' source enumeration — ideally as *force-disabled*
rather than *not-listed*, so a parked folder is inert in **both** layers. This also matches the
obvious user expectation that renaming a folder `_BAK` parks it.

**USER RULING (2026-07-19):** **not necessary to chase.** Parked siblings live under AppData
`model_replacements/` and are gitignored local clutter — they are not the skeleton and do not ship
to players. Bridge GAP-3 `verify` WARN is enough awareness. Do not block the lane or Outset work on
an engine parked-name skip unless a later containment audit reopens it.

### F-2 — "Rename to `.DISABLED`" DOES NOT DISABLE — so M5b has never actually run

`WW-Crew-Restoration.DISABLED` is not a disabled mod; it is a **new mod whose name happens to end in
`.DISABLED`**. Being unlisted in the order setting, it defaults to **enabled**
(`custom_assets.cpp:986`, "unlisted = enabled"). Renaming therefore *creates* an enabled mod rather
than parking the old one — which is precisely what lines 51/110 above show.

**Consequence: M5b (folder-absent smoke) remains OPEN and has never been validly performed.** Every
prior "mod renamed off" run still had WW content overlaid. The valid procedure is to **move the
folder out of `model_replacements/` entirely**, or disable it through the Mods UI — *not* rename in
place.

### F-3 — the preserved backup exe FAILS the M6 gate (pre-Phase-M binary)

`build/fps-good-backup-20260719-0827/dusklight.exe` (08:27, 32.5 MB):

```
HIT [ascii] 'Ivan'   HIT [ascii] 'Makar'   HIT [ascii] 'Medli'   → GATE FAILED (3 hits)
```

String context identifies it as a **pre-Phase-M build carrying the complete M0 leak set**:
`NPC_MK … Ivan socket (Mk.arc; Plan R)` (CA-01..04) · `Stub parked: … (Makar crash class) … spawn
refused` (CA-06) · `display_name=Makar.neck_joint=head…` and `[mk.greet].Makar:` (**AS-01..11 — the
embedded manifest templates *with dialogue text*, M0's "big leak"**) · `STUB entries (Makar/Medli/…)
are roster shells` (ED-01). It also contains **zero `[ExtNpcPop]`** — no population system at all.

**Severity: contained but real.** `build/` is gitignored (`.gitignore:28`), so it cannot reach a
commit. It is a **distribution/provenance** risk, not a commit risk: running it locally for
forensics breaches nothing, but it must never be shared as "a good build."

**Standing rule proposed:** *binaries outlive source fixes.* Phase M cleaned the source on 07-18;
a pre-M binary survived into 07-19 and was proposed for active use. **Any preserved or backup exe
must pass `ww_bridge gate` before it is shared with anyone**, and ideally the M6 gate becomes part
of the share/push checklist rather than only the build checklist. This is the first time the gate
has caught a complete leak set in the wild — it earned its keep.

### What this lane can action — GAP-3 for Bridge

F-1 and F-2 are engine/content matters **out of this lane** (user routes them). But they expose a
real hole in **Bridge's own tool**, which is squarely ours:

**GAP-3 — `verify` is blind to parked sibling folders.** It validates one `--mod-root` and knows
nothing about neighbours. A `verify` CLEAN today says nothing about a `.SKELETON_BAK` or `.DISABLED`
sibling silently overlaying WW arcs into the game tree — exactly the F-1 breach. Since `verify`
exists to answer *"is this install contained?"*, that is a false-CLEAN class.

**Proposed check:** scan `model_replacements/` for siblings whose names match the parked patterns
(`.SKELETON*`, `*_BAK`, `*.DISABLED`) and that contain an `arcs/` or `files/` tree — **WARN** each
one as a live overlay source masquerading as parked. Cheap, read-only, no engine dependency, and it
would have caught F-1 without a game boot.

**Secondary:** `README` should state that any **preserved/backup exe must pass `gate` before it is
shared** (F-3's lesson: binaries outlive source fixes).

### Status

| # | Finding | Owner | State |
|---|---|---|---|
| F-1 | Parked folders still overlay files (№52-C half-done) | user ruled **not necessary** (gitignored local) | **DECLINED** |
| F-2 | Rename ≠ disable; M5b never validly run | noted; same ruling — don't chase for this lane | parked |
| F-3 | Backup exe fails M6 (`build/` gitignored ⇒ contained) | user — do-not-share | contained |
| **GAP-3** | **`verify` blind to parked sibling folders** | **Bridge** | **OPEN — this lane** |

---

## 18. Auditor confirmation — GAP-3 + fail-open harden (2026-07-19). **LANE CLEAR.**

**GAP-3 CLOSED.** `verify` now fires on the real case:

```
WARN  parked-named sibling still overlay-capable: WW-Crew-Restoration.SKELETON_BAK (arcs/)
      -- rename is not disable; move out of model_replacements/ or Mods-UI off
```

Live verify **CLEAN (24 warnings)**. The F-1 condition is now detectable by tool, without a game
boot — which is exactly the awareness level the user's ruling calls for.

**Fail-open edges CLOSED — both, as specified in §16:**

| §16 edge | Fix |
|---|---|
| explicit negative version read as legacy | `keySeen` flag separates *key absent* from *bad value*; `found < 0 \|\| found != kExtPopSchemaVersion ⇒ Mismatch` |
| present-but-unreadable meta read as legacy | `if (!in) return Mismatch` |

Critically, **truly-absent meta still returns `LegacyV0`** — the live folder keeps booting.

**F-1 engine fix: user-DECLINED, and the auditor concurs.** Parked siblings are gitignored AppData
clutter that never ships in the skeleton or a player zip; they are not distributable content. The
covenant governs what we distribute, and nothing here reaches that. GAP-3's WARN is sufficient.
**Auditor withdraws the proposed `custom_assets` skip and will not reopen it** unless a parked
folder is found in a *shipping* path.

### Lane status: NO OPEN AUDITOR ITEMS FOR BRIDGE

B0 · B4 · schema v2 · engine handshake · GAP-1 · GAP-2 · GAP-3 · fail-open harden — **all ACCEPTED**.
A-1 – A-4 closed. Bridge has nothing outstanding from this lane.

**Remaining items are the user's to route, not Bridge's:**

| Item | Owner | Note |
|---|---|---|
| M5b folder-absent smoke (valid procedure: move folder out / Mods-UI off) | user | §17 F-2 |
| §16 step 2 — mismatch ⇒ `[POP!]` badge playtest | user | optional; only unproven link in A-1 |
| Skeleton reconciliation (24 `verify` warnings; `actor_map.ini` now 3031→6057) | content lane | §14 — largest remaining gap to §0 |
| Live census refresh w/ recovered rotations | content lane | §12 |
| Do-not-share: `build/fps-good-backup-20260719-0827/` fails M6 | user | §17 F-3; contained (gitignored) |

`verify`'s warning count is the progress metric for row 3: **24 → 0 means shippable.**

---

## 19. Auditor note — gate coverage gap (2026-07-19)

Two items from the informational content-lane post touch **this lane's instruments**. Recorded for
Bridge; no turn is transferred and nothing is routed out of lane.

### 19.1 Neither `gate` nor `verify` inspects the GAME TREE

The Receiver Covenant covers **"the vanilla exe *and game tree*."** Current coverage:

| instrument | scans | does NOT scan |
|---|---|---|
| `gate <exe>` | the built binary | anything on disk |
| `verify --mod-root` | `arcs/ npc/ population/ dialogue/` of one mod | the game tree; sibling mods (until GAP-3, names only) |

**Nothing checks `/res/Stage/` — or any game-tree path — for WW-named identifiers.** Today that is
harmless: no WW-named stage folders exist. It stops being harmless the moment stage shells land in
the game tree, because a WW token in a *path* is a covenant breach the exe gate cannot see (paths
are not necessarily strings in the binary).

**Not proposing work now** — there is nothing to scan yet. Logging it so that when stage shells
appear, the gate is extended rather than assumed sufficient. Likely shape: `gate --tree <path>`
matching greplist patterns against **file and directory names**, not just binary contents.

### 19.2 The "wholesale refresh could lose hand-appended rows" concern — already measured

§12's measurement bears directly on this and de-risks it: **position-only intersection was 571/572,
and live rows with NO position match in gen was ZERO.** Every live row had a regenerated
counterpart — including `Ojhous2R1`, which the tool regenerates (59 live / 60 gen) because its arc
is present in `arcs/`. So a refresh loses nothing positionally; it *adds* rotations and the v2
fields.

**Caveat that matters:** that measurement was taken at ~16:00 and the live folder has changed since
(`actor_map.ini` 5413 → 6057 bytes). The claim is therefore **"was true at measurement time,"** not
a standing guarantee. **Re-run `diff-live` immediately before any refresh** — that is precisely what
it exists for, and it converts this from an assumption into a check.

---

## 20. §16 step 2 — `[POP!]` test RESULT (2026-07-19)

Ran with `population/bridge_meta.ini` = `schema_version=99`; artifact removed after. Control boot
confirmed clean restore ("everything back in order"), which makes the causal reading below sound
rather than coincidental with the in-flight interiors pivot.

### PASS — A-1 works

Refusal fired with the correct message (`file=99 engine=2 — refusing population CSV
'outset_placements.csv'`), island emptied, **no crash**. Badge path exercised end-to-end. **§16 step
2 is closed; A-1 is proven at runtime.**

### DEFECT 1 — the refusal fires **743×**, not once. *Auditor correction.*

§16 credited the refusal path with being spam-proof: *"inserts into `s_spawnedBgProcs` so it cannot
retry-spam per frame."* **That was wrong.** 743 identical warnings in one session — something calls
`dExtNpcPopulation_clearForBg` repeatedly and wipes the latch, so the guard does not hold. The claim
was inferred from reading the insert without testing it; the test disproves it.

### DEFECT 2 — refusal blast radius exceeds what the badge says

**Confirmed by control:** with the bad meta present, door→interior warps stop working; with it
removed, they work again. Symptom is precise — the door opens, the knob demo plays, the warp is
*requested*, and nothing follows:

```
[Doors] №91 post-cutEnd OPEN → prop='linkrm'
[ExtNpcMount] №91 knob demo BEGIN proc='NPC_KNOB' staff=3 evt=513
[ExtNpcMount] №91 knob demo END (cutEnd owned) → warp proc='NPC_KNOB'
   ← no "BG warp 'EXT_BG1' → host …" line; loop repeats
```

A working warp logs `BG warp 'EXT_BG0' → host F_SP115 r0 then relocate`; the interior equivalent
never appears. So refusal halts between *warp requested* and *BG warp started*.

**Why this matters to A-1:** the note reads *"Population refused: schema_version mismatch (regenerate
census)."* A player also loses **door transport between spaces** and is told nothing about it — they
would reasonably read that as a broken mod rather than one stale file.

**UNTESTED, flagged not chased — possible softlock.** This test ran with Link on the exterior. If a
save has Link **inside an interior** when a schema mismatch occurs, the exit door uses the same warp
mechanism that just failed. Whether the №34-P4 / G-guard fallback rescues that case is **unknown**;
the auditor did not test it because doing so risks a save state. Recording it as an open question,
not a work item.

### Routing

Both defects sit in engine code and are **out of this lane to fix** — recorded here as results of a
test this lane owned, for the user to route. No Bridge action requested; B0/B4/schema v2/handshake
remain accepted.

---

## 21. Status correction — M5b DEFERRED; §20 DEFECT 2 has a shelf life (2026-07-19)

**M5b is DEFERRED, not open.** Content-lane №92-B (state doc, 23:15) rules that the interiors pivot
**changed M5b's scope**, so running it now would test a state about to be abandoned. Correct
sequence: run it **after** the interiors rollout, expanded to also verify (a) vanilla boots/warps
cleanly with our stage folders present, and (b) the warp menu degrades gracefully when target stages
vanish. §17 F-2 is likewise **parked** by user ruling.

**Supersedes** any earlier auditor statement that M5b was the outstanding item in this lane.
**This lane now has ZERO open auditor items.** Saves remain backed up + hash-verified
(`Documents/dusklight-backups/saves-20260719-175656`) for whenever M5b is scheduled.

### Caveat on §20 DEFECT 2 (refusal breaks door→interior warps)

The A/B was sound — bad meta present ⇒ warps dead; removed ⇒ restored — but it was measured against
a **door/warp lane that is actively being replaced** (№90 residuals: population + exit-knob still
gated on the retired room-lane path; №91 moves door demos onto TP's native `daKnob20_c` event
pattern). The finding is therefore **true of today's lane, with uncertain longevity**.

Recommendation: **do not spend engine effort on DEFECT 2 as stated.** Re-run the `[POP!]` test after
the №90/№91 rework and see whether the blast radius survives. If it does, it is a durable property of
the refusal path worth addressing; if not, it was an artifact of the lane being retired. **DEFECT 1
(743× refusal spam) is independent of the door lane and does not carry this caveat.**

---

## 22. Containment re-audit (2026-07-19, post-interiors-pivot). **M6 GATE FAILS.**

Fresh sweep at user request. Gate was CLEAN at the §16 build; it is **not clean now**.

### L-1 — GATE FAIL: `"Outset Interiors"` compiled into the exe

`include/dusk/map_loader_definitions.h:607` — **tracked and modified, so it WOULD be committed:**

```cpp
// №81: mod-side WW interiors host (Layer A overlay). History expands rooms/points.
RegionEntry("Outset Interiors", {
    MapEntry("Link's House (interior)", "R_DL01", { {0, {0}} }),
}),
```

Confirmed in the binary beside the other map labels (`…R_DL01…Link's House (interior).Outset
Interiors…`). `gate` → **FAILED (1 hit)**.

**Root cause is structural, and it was predicted.** Content-lane №90 warned in advance:
*"`map_loader_definitions.h` is a compile-time table, so every new WW interior would … put WW-side
labels in the binary. Better: have the warp menu ENUMERATE WW stages from the mod manifests …
Covenant-clean (no WW naming compiled in)."* The compile-time entry was added anyway — presumably to
make the warp reachable for testing. **Every future WW interior added this way is another gate
failure**, so the label fix is a patch and the manifest enumeration is the actual remedy.

Credit where due: **`R_DL01` is correctly neutral** — the neutral-stage-name guidance held.

### L-2 — GATE MISS: `"Link's House (interior)"` is a WW label the greplist cannot see

Same table, line 608. It reaches the exe and the gate **does not catch it**, because `Link's House`
is *also* a legitimate TP location (`map_loader_definitions.h:109`, `R_SP01`, Ordon). Adding it to
greplist.txt would fire forever on TP's own entry — the **Ganondorf problem** again.

**This is a real limit of a flat string list**, and worth stating plainly: the gate proves the
*absence* of unambiguous WW tokens; it cannot prove the absence of WW *labels that TP also uses*.
Options: (a) neutralize the label (`"Interiors"`), (b) remove it with L-1's manifest enumeration, or
(c) accept a documented blind spot. **(b) fixes both at once** and is the standing recommendation.

### L-3 — convention drift, NOT a breach: `include/d/d_ext_ww_save_guard.h`

A `ww` token in a shipping-source **filename**. Tested for the `__FILE__`/assert leak vector:
`d_ext_ww_save_guard`, `ww_save_guard`, `d_ext_ww` all = **0 hits, ASCII and UTF-16.** The filename
does not reach the binary, so **no covenant breach**. It is inconsistent with the Phase-M neutral
naming used by its siblings (`d_ext_npc_population.h`, `EXT_BG0`, `NPC_TPOST`). Cosmetic; flagged
only so the convention is a decision rather than drift.

### L-4 — inert, no action: comment-class `"Arylls"`

`src/d/d_ext_npc_population.cpp:93`, inside a comment. Comments compile away — the gate reports
`Aryll` = 0. This is the item the content lane flagged as awaiting a ruling; **from the covenant's
standpoint it is already compliant.**

### Housekeeping — strays that would be committed

`tmp_r00_names.txt` (repo root, arc-listing debug dump) · `tools/_symcrash.out.txt` ·
`tools/_symcrash.ps1` · `tools/_sym2.ps1` · `tools/_fps_ab3.bat` · `tools/_factory_recover.bat` ·
`tools/demo_cut_content/err.txt` · `.cursor/`. No WW content in them; ordinary commit hygiene.

### Answer to the standing question: **would committing commit the mod folder? NO.**

`%AppData%\…\model_replacements\WW-Crew-Restoration\` is **outside the working tree** — git cannot
see it, so no commit can include it. Verified: of 75 pending entries, **zero** are `.arc/.bdl/.bmd/
.dzb/.dzr`. The repo-side skeleton (`tools/ww_crew_restoration_skeleton/`) *is* tracked and would
commit, but it holds config text only — its `arcs/` contains just `README.txt`. **The never-commit
rule is holding.**

### Summary

| # | Finding | Reaches exe? | Would commit? | Severity |
|---|---|---|---|---|
| L-1 | `"Outset Interiors"` label | **YES — gate FAIL** | **YES** | **fix before push** |
| L-2 | `"Link's House (interior)"` label | **YES — gate blind** | **YES** | fix with L-1 |
| L-3 | `d_ext_ww_save_guard.h` filename | no (0 hits) | yes | cosmetic |
| L-4 | `"Arylls"` in a comment | no | yes | none |
| — | mod folder / WW arcs | no | **NO** | clean |

**L-1 and L-2 are one fix.** Out of this lane to implement — recorded for the user to route.

---

## 23. Containment re-pass on the crashing build (2026-07-19, exe 23:33)

Re-audit after the interiors slice + crash. **Damage is UNCHANGED — nothing new leaked.**

| Check | Result |
|---|---|
| M6 `gate` | **FAIL — 1 hit, `Outset`** (identical to §22 L-1; not yet fixed) |
| `verify` (mod folder) | **CLEAN**, 24 warnings (same parked-sibling + skeleton-drift set) |
| New WW string literals in `src/`+`include/` | **none** beyond the known L-1 |
| WW arcs / mod folder in commit scope | **none** — rule still holding |

**The compile-time map table did not grow.** Still one region + one entry:

```cpp
RegionEntry("Outset Interiors", {                    // L-1 — gate FAIL
    MapEntry("Link's House (interior)", "R_DL01", {  // L-2 — gate blind
```

So the interiors work stayed **data-side**, which is the right instinct and is why the leak did not
multiply.

### FORWARD RISK — the table is about to grow 6×

Content-lane №92-A maps `R_DL01` to **six rooms** (0 LinkRM · 1 Ojhous2 · 2 Ojhous2R1 · 3 Omasao ·
4 Onobuta · 5 LinkUG). Only room 0 is listed today. If each room lands as a labeled `MapEntry` in
this compile-time table, the natural labels are WW residents' houses — i.e. **up to six more WW
labels compiled into the exe**, most of them (like `Link's House`) invisible to a flat greplist.

**This is exactly the multiplication §22 L-1 warned about, now with a number on it.** The manifest
enumeration proposed in content-lane №90 stops it at one; a per-label patch does not. **Cheapest
moment to decide is before the other five rooms are wired.**

---

## 24. M5b — PREPARED, NOT RUN (2026-07-19)

Staged at user request; **nothing has been moved.** Execute only on explicit go.

### Pre-flight: DONE

- **Saves backed up + hash-verified**, and re-checked at prep time: live `gczelda2` md5
  `55bf3325…` == backup — **still current**, no re-take needed.
  (`Documents/dusklight-backups/saves-20260719-175656`, outside AppData and outside the repo.)
- Reminder: the three save slots live in **one** card file, so restore is all-or-nothing.

### Scope — exactly TWO folders move

Enumerated every mod folder by overlay payload; only these inject WW-Crew content:

| folder | payload | why it moves |
|---|---|---|
| `WW-Crew-Restoration` | `files/`=19, `arcs/`=62 | the mod itself |
| `WW-Crew-Restoration.SKELETON_BAK` | `arcs/`=4 | **§17 F-1** — proven to overlay `vhkak/vhutu/vkeyn/vlupy` independently |

**Deliberately NOT moved:** `Wind Waker Deku Leaf`, `Wind Waker Skins` — both `files=0 arcs=0`
(Layer-B only, no overlay) and both **exempt** per the greplist ruling (they render TP's own leftover
data). Scope is the Receiver Covenant for WW-Crew-Restoration, not "all mods off." A stricter
variant disabling every mod is available if wanted — say so before the run, not after.

### The moves — **MOVE, never rename** (F-2: rename ≠ disable)

```
%AppData%\…\model_replacements\WW-Crew-Restoration          →  <holding dir outside model_replacements>
%AppData%\…\model_replacements\WW-Crew-Restoration.SKELETON_BAK  →  same holding dir
```

Holding dir must be **outside `model_replacements/`** entirely.

### Baseline checks (the original M5b)

In the new log, ALL of these must be **absent**:

```
[custom_assets] 'WW-Crew-Restoration…'      ← any overlay claim
[custom_assets] R2 arc-mount                 ← any WW arc mounted
[ExtNpcPop] / [ExtNpcMount] / [ExtNpcDoors]  ← any WW system active
```

Plus: game boots, plays vanilla TP for a few minutes, **no crash**, and **Lake Hylia (`F_SP115`)
loads TP's real `R02_00.arc`** — the mod currently overrides that room, so its restoration is a
positive vanilla-recovery signal.

### EXPANDED check (why M5b was deferred until now) — **warp-menu degradation**

№92-B made the expanded shape conditional on where stage shells live. **Determined: MOD-SIDE.** All
six rooms + `STG_00.arc` are at `files/res/Stage/R_DL01/`. So the required check is *"warp menu
degrades gracefully when its target stages vanish"*:

1. Open the warp menu with the mod removed.
2. **"Outset Interiors → Link's House (interior)" will still be listed** — that entry is
   **compile-time** (`map_loader_definitions.h:607`) and cannot vanish with the mod.
3. Select it. `R_DL01` stage data no longer exists.
4. **PASS** = refuses / no-ops / returns to menu with a message. **FAIL** = crash, hang, black void,
   or Link left controllable in a stageless state.

**This also gives §22 L-1 a second, non-covenant argument.** A compile-time warp entry pointing at
mod-side data that can disappear is a *robustness* defect as well as a leak. Manifest enumeration
(content-lane №90) fixes both; neutralizing the label fixes only the leak. If step 4 fails, that is
the strongest case yet for enumeration.

### Restore + confirm

Move both folders back → boot → expect `EXT_BG0` ≈ **71 actors**, doors working, warp menu
functional. Restoration failure is itself a finding.

### Acceptance

| # | Criterion | Pass |
|---|---|---|
| 1 | Zero WW overlay / arc-mount / ExtNpc lines | vanilla receiver confirmed |
| 2 | No crash across a few minutes of vanilla play | — |
| 3 | `F_SP115` R02 restored to TP's own | override cleanly reversible |
| 4 | **Warp entry to vanished `R_DL01` degrades safely** | **the new, pivot-era check** |
| 5 | Full restore reproduces 71 actors + working doors | non-destructive |

### Standing instruction

**Do not save during the run** — no known risk, but the backup rolls back all three slots at once.
Auditor executes the moves on explicit go; user drives the boots; auditor analyses the logs.

---

## 25. M5b — **PASS. CLOSED.** (2026-07-20)

First valid execution in the project's history. Phase M closed 2026-07-18 with M5b listed
"outstanding"; every subsequent attempt was invalid because **rename ≠ disable** (§17 F-2). This run
used the correct procedure — both folders **moved out** of `model_replacements/` — and both were
restored byte-exact afterwards (715/104,429,138 and 67/116,431, matching pre-move).

| # | Criterion | Result |
|---|---|---|
| 1 | Zero WW overlay / arc-mount / ExtNpc-population lines | **PASS** — `WW-Crew-Restoration` 0 · `R2 arc-mount` 0 · `ExtNpcPop` 0 · `ExtNpcDoors` 0 · `SKELETON_BAK` 0 |
| 2 | No crash across vanilla play | **PASS** — 0 crash blocks |
| 3 | Vanilla receiver behaviour | **PASS** — sole `ExtNpcMount` line is `head registry entries=0`: the generic system present, knowing nothing about WW, and saying so |
| 4 | Warp menu degrades when target stages vanish | **PASS — structurally**, see below |
| 5 | Full restore reproduces the mod | **PASS** — user confirmed |

**The Receiver Covenant is verified end-to-end for the first time.** With the mod removed, the game
is pure vanilla TP: no WW bytes, no WW names, no dangling references, no crash.

### Criterion 4 — better than "graceful": it is structural

Auditor predicted the compile-time entry would still be listed and would need to fail safely.
**It was not listed at all.** `src/dusk/ui/warp.cpp:359`:

```cpp
// №99 R2: warp rows from mod manifests (warp_label=). Labels stay data-side —
// never compile WW-facing strings into map_loader_definitions.h.
```

Warp rows are built by **iterating providers**, so with no providers there are no rows. A row cannot
outlive its data because the row *is* the data. **The manifest enumeration this doc has been
recommending since §22 is already implemented** (№99 R2).

### Consequence — §22 L-1 DOWNGRADED, and one auditor argument WITHDRAWN

- **L-1 is not a redesign.** `map_loader_definitions.h:607`'s `RegionEntry("Outset Interiors", …)`
  duplicates a covenant-clean path that already exists. The probable fix is **deleting the entry**,
  not rebuilding anything. It remains the **sole gate failure** — the string is in the exe whether or
  not a UI reads it — but the cost estimate drops from "phase" to "one line." *Whether the map-loader
  screen still needs it is a Cursor question; do not delete on the auditor's read alone.*
- **WITHDRAWN:** the §24 robustness argument ("a compile-time entry pointing at mod-side data that
  can vanish is also a robustness defect"). It was premised on rows outliving their data, which
  №99 R2 makes impossible. The covenant argument stands; the robustness one does not.

### Standing containment status

| Item | State | Owner |
|---|---|---|
| **M5b folder-absent smoke** | ✅ **CLOSED — PASS** | — |
| L-1 `"Outset Interiors"` (gate FAIL) | open — probable one-line delete | engine, user routes |
| L-2 `"Link's House (interior)"` (gate blind) | open — same fix as L-1 | engine, user routes |
| §20 DEFECT 1 — refusal fires 743× | open | engine, user routes |
| §20 DEFECT 2 — refusal breaks door warps | open, **shelf-life caveat §21** | re-test after №90/№91 |
| Skeleton reconciliation (24 `verify` warns) | open | content lane |
| Live census rotation refresh (§12) | open | content lane |

**Auditor charter check B is now satisfied.** A (greplist gate) is failing on L-1 only; C, D, E are
clean.

---

## 26. NEW RESOURCE — WW decomp local. Three asks for Bridge. (2026-07-20)

### The resource

`zeldaret/tww` cloned to **`D:\XXXXXXX\WW DP`** (user-directed) — 81 MB, 8,092 files, shallow
(`--depth 1`; deepen on request). HEAD `61a3a0a`, **dated 2026-07-19** — actively maintained.
Targets **`GZLE01` (USA rev 0)**, matching the project's extract and the §3 US pin.

Contents: **455 `d_a_*.cpp` actor sources** of 914 total, plus the shared framework — `src/d` (575),
`src/JSystem` (214), `src/f_pc` (31), `src/f_op` (22). Same vocabulary as this fork
(`fopAcM_*`, `fopAc_ac_c`, `dComIfG_*`, `mDoExt_*`, `cLib_*`), which is what makes a port a
**source-adaptation** job rather than a reimplementation.

### Why this matters to the tool

§15 established that the tool captures fields **faithfully but blindly** — `params_hex` is preserved
bytes with unknown meaning. The decomp is the missing semantics layer, now on local disk instead of
behind a web fetch.

### ASK 1 — validate skipped-chunk semantics locally *(do first; gates the rest)*

§26-adjacent survey of Outset's DZR found these chunk types **present and unread**:

| chunk | count | suspected role |
|---|---|---|
| `RPPN` / `RPAT` | 296 / 40 | NPC + camera routes |
| `SCLS` | 10 | stage exits (B2) |
| `LBNK` / `FILI` | 12 / 1 | light banks / room light index |
| `RCAM` / `RARO` | 5 / 5 | camera setups |
| `SOND` | 2 | positional sound emitters |
| `SHIP` | 2 | ship moorings (Phase GS) |

**Those roles are auditor inference from naming — NOT confirmed.** §15's lesson applies directly:
rotation semantics were asserted before checking and happened to survive; that was luck, not method.
Confirm each struct against the decomp the same way `fopAcM_prmBase_class` settled `angle @0x18`
before any extraction work is planned on them. **This gates ASK 3 and B2.**

### ASK 2 — a params decoder *(highest-value NEW capability)*

Cross-reference census `params_hex` against the decomp's param accessors for that actor and emit a
documented column. Converts 475 Outset rows from "position + mystery integer" into
**documented placements**, and directly serves the doctrine that behavior is never guessed.
Degrades cleanly: actors without decomp source simply keep the raw hex.

### ASK 3 — an actor-coverage report

Per census name, report whether decomp source exists — so the content lane can answer *port vs
substitute vs script* with data instead of guesswork. Auditor spot check of the Outset cast:

- **source present:** `npc_md` (~6.3k lines) · `kb` pigs (~2.6k) · `npc_mk` (~1.8k) ·
  `kamome` (~1.6k) · `knob00` (~0.9k) · `npc_ah` · `npc_p1` · `npc_p2`
- **absent:** `npc_ls` `npc_ob` `npc_ym` `npc_zl` `npc_ko` `npc_bm` `npc_aj` `npc_cc` `npc_dk`
  `npc_ji` `npc_bk` · `vlupy` · `tpost`

Roughly a third covered, including the two structurally important ones — **`knob00`** (the door
mechanism content-lane №91 is already mirroring) and **`kb`** (catchable pigs). The gaps skew toward
**named residents**, i.e. exactly the bespoke behaviour least substitutable by a TP actor.

**CAVEAT — the auditor checked file EXISTENCE, not match state.** A present file may be partly
stubbed. The report must read actual match status; **nobody should plan around an actor on the
strength of the spot check above.**

### NOT an ask for Bridge — auditor item, recorded so the lane sees it

**A containment policy for ported decomp code is needed BEFORE the first port.** Measured: port
candidates carry **13–43 unique string literals each** (`knob00` 13 · `kamome` 13 · `kb` 16 ·
`npc_mk` 43). The M6 gate matches a **fixed greplist** — a novel resource name, event id or debug
string arriving inside a ported actor would pass **silently**.

M6's standing rule is that every feature passes the gates **at introduction**. Phase M is the
counter-example: leaks built first, audited after, whole phase spent cleaning up. Three decisions
needed — filename policy (L-3 says filenames don't reach the exe, so likely permissive), literal
policy (neutralize / manifest-drive / accept case-by-case), and whether the greplist grows per port
— plus a pre-port audit step. **Auditor owns this; it gates the first port, not Bridge's tool work.**

### Sequencing

**ASK 1 → ASK 3 → B2**, with ASK 2 parallel. The auditor's policy item runs independently and blocks
neither.

---

## §26 Bridge response — ASK 1 / 2 / 3 landed (2026-07-20)

Tool root: `C:\Users\xxxxx\Documents\albt bridge`. Decomp: `D:\XXXXXXX\WW DP` (`--decomp-root` / `WW_DECOMP`).

### ASK 1 — DONE (gates B2)

`python -m ww_bridge chunk-ref` → `reports/chunk_semantics.md`

Headers verified present (`d_stage.h` / `d_path.h` / `d_stage.cpp`). Confirmed roles:

| tag | confirmed role | notes vs auditor table |
|-----|----------------|------------------------|
| `SCLS` | stage exit list (`stage_scls_info_class`, 0x0C) | B2 target — confirmed |
| `RPPN` | path **points** (`dPnt` cloud) | **not NPCs** |
| `RPAT` | path graphs (`dPath`) | NPC/camera **routes** via paths — directionally OK |
| `LBNK` | layer bank `u8` table | **not light banks** |
| `FILI` | room file-list / env flags + sea level | not “room light index” alone |
| `RCAM` / `RARO` | room camera types / arrow aim points | confirmed |
| `SOND` | positional sound emitters | confirmed |
| `SHIP` | KoRL mooring pose | Phase GS — confirmed |

No extraction code yet — semantics only, as asked.

### ASK 3 — DONE

`python -m ww_bridge coverage --census goldens/outset_placements.csv` → `reports/actor_coverage.csv`

Outset golden unique names: **87 → present_cleanish 31 / absent 56** (0 `NONMATCHING`/`asm` markers in hit sources).

Caveats honored:

- Status is **file-level heuristic** (size + marker grep), **not** objdiff match%.
- Census codes (`Ls1`, `Pig`) ≠ auditor spot-check strings (`npc_ls`, pigs as `kb`). Aliases: `Pig`→`kb`, `P1a`/`P1b`→`npc_p1`, etc.
- Several “absent” named residents in the auditor’s `npc_*` list **are present** under folk codes (`Ls1`→`npc_ls1`, `Ob1`→`npc_ob1`, …).

### ASK 2 — DONE (parallel)

`python -m ww_bridge decode-params --census goldens/outset_placements.csv` → `reports/params_decoder.md`  
Optional: `--annotate-out reports/outset_placements_decoded.csv` (adds `params_decoded`).

Scrapes `fopAcM_GetParam(...)` / `>>` / `&` / `GetParamBit` from matched sources (args include `this` / `a_this` / `&i_this->actor`). **11/87** unique outset names have at least one field; others keep raw `params_hex` (clean degrade).

### Next (Bridge)

- Housing Security accept / amend ASK 1 table → unlocks **B2 SCLS extractor** design.
- No port work; auditor still owns pre-port containment policy.

---

## 27. PORTING POLICY for decomp-sourced actors (auditor, 2026-07-20)

Auditor's own §26 item, delivered. **Ratify before the first port.**

### CORRECTION to §26 — the risk was overstated

§26 claimed port candidates carry "13–43 unique string literals each." **That number was wrong.**
It counted `#include` paths, which are preprocessor directives and never reach the binary.
Re-measured, excluding includes:

| actor | runtime literals |
|---|---|
| `d_a_kamome` | **2** |
| `d_a_knob00` | **3** |
| `d_a_kb` | **3** |
| `d_a_npc_mk` | 35 |
| `d_a_npc_md` | **161** |

**The risk is not uniform — it scales with actor class.** Props and simple creatures are near-zero;
NPCs carrying dialogue and event logic are where review is actually needed. A blanket policy would
have been wrong in both directions.

### What the literals actually are

`knob00`'s three are a joint name (`DoorDummy` — which this project *already* uses per Cookbook §4.4),
an actor short-code, and a lowercase model word. `kb`'s three are an actor code, an assert
expression, and a generic English word. **None of these are WW identity** — no person, place, or
dialogue.

That is the distinction the policy should turn on, and it is the same one already in force: bare
`Ganondorf` is exempt as a TP principal, `"OUTSET"` was not because it is a recognizable place.
**Recognizability is the test, not WW-origin.**

### D1 — Filenames: **KEEP the decomp's names** (e.g. `d_a_knob00.cpp`)

L-3 verified filenames do not reach the exe (`d_ext_ww_save_guard` = 0 hits, ASCII + UTF-16).
Keeping upstream names preserves **traceability to a live repo** — tww shipped a commit the day
before this clone, so ported actors will want upstream fixes pulled. That is worth more than
cosmetic neutrality. Phase-M's neutral-naming rule (A1) governs **runtime tokens**
(`EXT_BG0`, `NPC_TPOST`), not source filenames; this does not weaken it.

### D2 — Literals: **neutralize only recognizable WW identity**

- **Leave alone:** joint names, actor short-codes, assert expressions, generic words — technical,
  non-identifying, and invisible as WW content to any player.
- **Neutralize or manifest-drive:** anything naming a WW **person, place, or dialogue line**.
- Applied to `knob00`: **nothing to change — it ports clean as-is.**

### D3 — Greplist: **grows per port, but only with identity strings a port introduces**

Extend `greplist.txt` when D2 finds identity material, not for every port. Most prop/creature ports
will add nothing.

### The pre-port audit (auditor-owned, mechanical, ~2 minutes)

1. **Extract** runtime literals: strip `#include` lines, collect unique quoted strings.
2. **Classify** each as *technical* or *identity* (D2).
3. **Resolve** identity items — neutralize, drive from manifest, or ratify an exemption; extend the
   greplist.
4. **Also check the actor's generated resource header** (`res/Object/<Arc>.h`-style includes) — a
   second vector where arc/resource names could enter the binary. Unverified as of writing;
   **confirm on the first port.**
5. **Port**, rebuild, run `gate` → **must be CLEAN**. A failing gate blocks the port, not the push.

### Verdict for the likely first port

**`knob00` is clear to port under this policy** — 3 runtime literals, none identity-bearing, and it
is the mechanism content-lane №91 is already mirroring. It doubles as the **one-exemplar test** of
this policy (Cookbook №63): run the audit end-to-end on it, confirm the gate stays clean, then
generalize. `npc_md` at 161 literals is the opposite end and should not be the first attempt.

**Status: RATIFIED (user, 2026-07-20). In force.** Auditor executes steps 1–4 on request per port.

### The port ledger — make each port shrink the unknowns permanently

**User principle (2026-07-20):** *"The more confirmed ports we do, the more the list of unknown
variables shrinks, even by trial and error it informs us."*

That only compounds if findings are **recorded rather than re-discovered**, so every port appends a
row here:

| # | actor | runtime literals | identity found | neutralized | resource-header vector | gate after | notes |
|---|---|---|---|---|---|---|---|
| 1 | `d_a_knob00` | 3 (`DoorDummy`, `Mt`, `knob`) | none | omitted `Mt` (quest figure only; never in source) | **closed — not used** (string res names only; no `res/Object/Knob.h`) | greplist **0** (identity list); technical `DoorDummy`/`knob` present as expected | Minimal port: two-model swing + DoorK10 + ExtNpc warp. Outset doors spawn `fpcNm_KNOB00_e`; fallback ExtNpc `NPC_KNOB`. Password/villa/pirateship omitted. |

The bounded columns are the point. The unknown set **is finite** — 455 actor files, a fixed set of
DZR chunk types, a small number of literal categories — so each port is a sample that constrains the
space for every port after it. Specific questions this ledger is expected to close early:

- **Is `res/Object/<Arc>.h` a real leak vector?** (§27 step 4 — unverified; the first port answers it
  once, for all ports.)
- **Do WW framework calls have clean TP analogues, or is per-actor adaptation heavy?** Unknown until
  a port is attempted; `knob00` is the cheapest possible probe.
- **Does literal count actually predict porting difficulty?** The 2 → 161 spread suggests yes;
  two or three rows will confirm or kill that heuristic.

This is the same method that has already paid out three times in this lane: B0 turned an assumption
into a byte-diff, §15 turned inferred field semantics into decomp-confirmed ones, and §27 turned an
inflated literal estimate into a measurement. **Ports are the next instance — trial and error is
legitimate method here precisely because the unknown set is enumerable.**

---

## 28. Two diagnostic asks for Bridge — turn crashes into named causes (2026-07-20)

User ask: *"if we run into an error — something not loaded where it should be, Dusklight crashing
because WW needs X in Y — can the tool say exactly what it needs? Turn crashes into beefed
diagnostic logs?"* **Yes, via two mechanisms.** Both are offline, neither touches engine code —
which makes them safe to build while the engine lane is mid-pivot.

**Housing:** the **tool** implements, this **doc** holds the spec. Same split as B0–B4.

### ASK 4 — deep arc-content validation *(extend `verify`; do this first)*

**Gap:** `verify.py:33-44` `check_arcs` tests only that `arcs/<Name>.arc` **exists as a file**. It
never opens it. Every resource a manifest names *inside* the arc is therefore unvalidated:

`model=` · `model2=` · `model3=` · `collision=` · `companion_model=` · `companion_idle=` ·
`door_open_bck=` · `head_model=` · `attach_model=` · plus joint keys `attach_joint=` /
`neck_joint=` / `head_joint=`

If a name is wrong, or a file was dropped during adaptation, the engine takes a NULL at spawn — the
symptom is a crash or a silently invisible actor, discovered by playtest. **Offline, the tool can
name the exact missing resource before launch.**

**Why it is cheap:** `dzr.py`'s `list_rarc()` already enumerates RARC contents — that is how B0
locates `room.dzr`. Pointing it at manifest references is an extension of proven code, not new
parsing. Joint validation needs J3D parsing and is a fair second step; **resource-name validation
alone closes most of the class.**

**Output shape:** `FAIL  <manifest>: arc 'Ko' has no 'ko_wait01.bck' (referenced by companion_idle=)`

### ASK 5 — crash-log symbolizer

**Everything needed already exists.** Crash blocks carry `Build-ID`, `module_base`, and per-frame
`rva`; the PDB sits beside the exe (**180 MB, timestamp-matched**). Last night's crash was 24 frames
of bare `rva=` — auditor had to infer the cause from surrounding log lines. Symbolized, it would
have been a named stack in seconds.

**Required behaviour:** resolve `rva` → function + line per frame; **verify `Build-ID` against the
PDB and REFUSE on mismatch** rather than emitting confidently wrong symbols (fail-closed, same
posture as the `schema_version` handshake — a stale-PDB stack is worse than none).

**Immediate test case available:** a fresh crash block exists in
`logs/dusklight-20260720-021350.log`.

**SCOPING FLAG — this one is arguably NOT `ww_bridge`.** It serves the whole project; nothing about
it is WW-specific. Dropping it into `ww_bridge` repeats the tool/content conflation §4 warned about.
Suggest a sibling command or a `diag` namespace kept clearly separate from the WW conversion
surface. **Bridge's call — flagged so the tool does not accrete unrelated features by default.**

### What neither can do — stated so they are not oversold

- **Ownership/lifetime bugs** (the 2026-07-19 use-after-free: caches purged while live references
  remained). Nothing about the static data was wrong, so no offline validation catches it.
  Symbolization would have **named the frames** — valuable — but not predicted it.
- **Semantic "WW needs X because Y."** That lives in the decomp, not the data. The tool can say a
  file is missing; it cannot say an actor expects a joint WW's code assumed.

### Priority

**ASK 4 before ASK 5** — cheaper, extends proven code, and *prevents* rather than explains. ASK 5
has the higher per-incident payoff and would likely earn out fast at the current crash rate around
the interiors work.

Against the existing queue: **ASK 1 (chunk semantics) still goes first** — it gates ASK 3 and B2.
ASK 4 slots naturally beside it, since both are `verify`-adjacent.

---

## §28 Bridge response — ASK 4 / ASK 5 landed (2026-07-20)

Tool `0.3.0`. Sibling packages under `C:\Users\xxxxx\Documents\albt bridge\`.

### ASK 4 — DONE (extends `verify`)

`check_arc_contents` opens each `arcs/<Name>.arc` via existing `list_rarc`, then FAILs when a
manifest (or `population/actor_map.ini`) names a missing member:

`model=` · `model2=` · `model3=` · `collision=` · `companion_model=` · `companion_idle=` ·
`door_open_bck=` · `head_model=` · `attach_model=` (+ idle/talk/btp/brk/btk commonly crashy) ·
actor_map `head_model` / `companion` / `body_bmt` resolved through the proc→arc map.

Output shape matches the ask:
`FAIL  ext_bg2.ini: arc 'LinkUG' has no 'model1.bdl' (referenced by model2=)`

**Live mod first run (content debt, not tool bug):** 3 FAILs —

| manifest | arc | missing | actual members (sample) |
|---|---|---|---|
| `ext_bg2.ini` | LinkUG | `model1.bdl` (model2=) | `model.bdl`, `model3.bdl`, `room.dzb` |
| `ext_bg8.ini` | Pjavdou | `model1.bdl` (model2=) | `model.bdl`, `model3.bdl`, … |
| `npc_lwood.ini` | Lwood | `lwood.bdl` (model=) | `alwd.bdl`, `alwd.dzb` |

Joint-name validation deferred (needs J3D), as scoped.

### ASK 5 — DONE (sibling `diag`, not WW conversion surface)

```bat
python -m diag symbolize path\to\dusklight-*.log
rem thin redirect also: python -m ww_bridge symbolize …
```

- Parses last `DUSKLIGHT CRASHED` block; reads PE CodeView RSDS Build-ID from the Module exe.
- **Fail-closed:** log Build-ID ≠ exe ⇒ refuse (no wrong symbols). Proven on
  `dusklight-20260720-021350.log` (log age **29**, current exe age **30**).
- Matching Build-ID → `llvm-symbolizer --relative-address` → function + file:line.
  Smoke on current exe: `rva=0xbcc63` → `mDoExt_J3DModel__create` @ `m_Do_ext.cpp:3822`.

### Next

- Housing Security accept ASK 4/5.
- Content lane (or user) fix the 3 FAIL manifests / arcs when ready — tool is doing its job.
- B2 SCLS still gated on ASK 1 accept (already posted).

---

## 29. Auditor acceptance — ASK 1–5 (2026-07-20). **ALL ACCEPTED.**

Each claim re-run independently, not taken on report. **Bridge corrected the auditor twice; both
corrections are confirmed and adopted below.**

### ASK 4 — ACCEPTED. The 3 FAILs are REAL.

Independently opened each arc and listed members:

| manifest | arc | members | claimed-missing | verdict |
|---|---|---|---|---|
| `ext_bg2.ini` | `LinkUG` | `model.bdl, model3.bdl, room.dzb, room.dzr` | `model1.bdl` | **ABSENT — confirmed** |
| `ext_bg8.ini` | `Pjavdou` | `model.bdl, model.btk, model3.bdl, room.dzb, room.dzr` | `model1.bdl` | **ABSENT — confirmed** |
| `npc_lwood.ini` | `Lwood` | `alwd.bdl, alwd.dzb` | `lwood.bdl` | **ABSENT — confirmed** |

Three genuine live-mod defects, found offline, on first run. **This is the ask paying for itself
immediately** — `Lwood` in particular is a manifest pointing at a name the arc has never contained.
**Content-lane fix; the tool is correct.**

### ASK 5 — ACCEPTED. Fail-closed verified by negative control.

```
Build-ID mismatch → refuse symbolize (fail-closed)
  log: …-29     exe: …-30
```

Refuses, names both IDs, states the remedy. Exactly the required posture. Sibling `diag/` placement
honors the §28 scoping flag — the WW conversion surface stays clean.

*Minor, non-blocking:* a refusal exits **0**. Fine for interactive use; if `diag` is ever scripted,
a non-zero exit would make refusals detectable. Bridge's call.

### ASK 1 — ACCEPTED, **and it corrects the auditor.**

Confirmed independently against the decomp:

- **`LBNK` is NOT light banks.** `d_stage.cpp:1966` `dStage_lbnkInfoInit` → `dStage_Lbnk_c` — a
  **layer bank** table. The auditor's §26 guess was wrong.
- **`RPPN` is NOT "NPC routes".** `d_path.h` → `dPnt` / `dPath_GetPnt` — path **points**; `RPAT` is
  the graph. Directionally close, materially imprecise.

§26 explicitly labelled these "auditor inference from naming — NOT confirmed" and asked for exactly
this check. **The method worked.** ASK 1's table supersedes §26's; **B2 is unblocked.**

### ASK 3 — ACCEPTED, **and the auditor's spot check was materially WRONG.**

§26 reported most named residents absent. Re-checked with suffix variants — **8 of 13 exist**:

| §26 said absent | actually present |
|---|---|
| `npc_ls` `npc_ob` `npc_ym` `npc_zl` `npc_ko` `npc_aj` `npc_ji` | `…_ls1` (2434 ln) `…_ob1` `…_ym1` `…_zl1` (3075 ln) `…_ko1` `…_aj1` `…_ji1` |
| `npc_bm` | `…_bm1`, `…_bmcon1`, `…_bms1`, `…_bmsw` (**four**) |
| `npc_cc` `npc_dk` `npc_bk` `vlupy` `tpost` | genuinely absent |

**The §26 conclusion — "gaps skew toward named residents, exactly the bespoke behaviour least
substitutable" — is REVERSED.** The named residents are largely covered. Coverage is materially
better than this doc claimed, which improves the port-vs-substitute picture. **Route to the content
lane: more of Outset's cast is portable than §26 implied.**

### Auditor methodology note — two measurement errors in two days

Both had the same root cause: **an unvalidated query treated as a result.**

1. §26/§27 literal counts — the grep included `#include` lines; real counts were 2–161, not 13–43.
2. §26 coverage — searched exact `d_a_<name>.cpp`, missing every suffixed folk code.

In both cases the *reporting* was careful (caveats attached, "existence not match state") while the
*query itself* went unchecked. **Correction going forward: validate the search before trusting the
finding** — for coverage questions specifically, enumerate candidate name shapes rather than
assuming one. Recorded because both errors would have propagated into planning had Bridge not
re-derived them.

### Status

**ASK 1–5 all ACCEPTED. Bridge's lane is clear.** B2 SCLS unblocked by ASK 1 accept.

Open for others: the 3 ASK-4 FAILs (content lane) · corrected coverage → content-lane planning ·
L-1 gate failure still the sole covenant item.

---

## §29 Bridge response — B2 SCLS landed + ASK 5 exit note (2026-07-20)

Tool **0.4.0**. ASK 1–5 accept absorbed; B2 shipped.

### ASK 5 exit code

`python -m diag symbolize` already returns **2** on Build-ID refuse / parse error (1 on missing
log). Re-confirmed against `dusklight-20260720-021350.log` (29≠30). No change needed beyond the
explicit comment in `__main__` for scriptability.

### B2 — `scls` command

```bat
python -m ww_bridge scls path\to\Outset.arc --out exits.csv [--stage Outset] [--append]
```

Parses `room.dzr` **SCLS** via ASK 1 struct (`mStage[8]`, `mStart`, `mRoom`, `mWipe`, pad @0x0C).
Does **not** rewrite `parse_dzr` / overwrite live `doors.ini` — emits CSV for content binding.

Columns: `source_stage,index,dest_stage,spawn_id,dest_room,wipe,pad` (omit `source_stage` without
`--stage`).

**Proof (live Outset.arc, matches №56 return graph):**

| index | dest | spawn | room | wipe |
|------:|------|------:|-----:|-----:|
| 0 | LinkRM | 1 | 0 | 10 |
| 1 | Ojhous | 0 | 0 | 10 |
| 2 | Ojhous2 | 0 | **1** | 10 |
| 3 | Omasao | 0 | 0 | 10 |
| 4–5 | Onobuta | 0 / 1 | 0 | 10 / 8 |
| 6 | A_mori | 0 | 0 | 0 |
| 7 | Pjavdou | 0 | 0 | 0 |
| 8 | Cave09 | 0 | 0 | 0 |
| 9 | LinkUG | 1 | 0 | 0 |

Interior returns dump to `reports/interior_scls.csv` — all `sea` room **44** with Nintendo spawn ids
1/3/4/5/7/8/11 (LinkRM/Orca+Sturgeon/Onobuta×2/Mesa/forest/UG).

Artifacts: `albt bridge/reports/outset_scls.csv`, `interior_scls.csv`.

### Next

- Housing Security accept B2.
- Content lane: wire generated SCLS → doors when ready (tool stays generative, not silent overwrite).
- B3 installer when queued.

---

## 30. Bridge next steps + ASK 4 triage (2026-07-20)

### Bridge queue

**1. SOCKET LEDGER — GREENLIT (user, preemptively, 2026-07-20).**

Concept (user): *a socket X accepts fillers {W, Y, Z}; it is currently filled with one of them, or
unfilled.* Applies to actors, environments, sounds, quests. **ASK 4 is already this model in
degenerate form** — `|acceptable set| = 1`, no tiers. The ledger generalizes it.

**It is a JOIN of artifacts that already exist — no new extraction:**

| existing output | supplies |
|---|---|
| census CSV | socket **inventory** (each placement row) |
| `population/actor_map.ini` | current **fill state** |
| ASK 3 `actor_coverage.csv` | **candidate availability** (decomp source present?) |
| ASK 4 arc contents | resource-level **fill / mismatch** |

**Scope for v1 (one-exemplar rule, №63):** actors + arcs, **Outset only**, **report-only**. No
manifest grammar for alternatives yet. Row shape: `socket id · wants · offered · state · candidates
· forbidden-set`.

**HARD CONSTRAINT — SUGGEST, NEVER FILL.** The tool proposes candidates; a human ratifies. A wrong
filler is worse than a hole (№31), and auto-filling would repeat the "invented identity" failure the
IVAN RULE exists to prevent. Same posture as the schema handshake: refuse/report, never guess.

**Success test:** does the *"unfilled but candidate available"* column produce a better work order
than intuition? If yes it earns a real acceptable-set grammar. If it merely restates the skip-list,
it cost an afternoon.

**2. B2 — SCLS extractor. UNBLOCKED** by §29's ASK 1 accept, on decomp-confirmed layout
(`stage_scls_info_class`, 0x0C). Value **rose** with the pivot: `doors.ini` is still hand-transcribed
and `R_DL01` now has six rooms.

**Order:** Bridge's call. Ledger is cheaper and may reshape prioritization; B2 is the longer-standing
spec. Neither blocks the other.

**Still deferred:** **B3 installer** — gate unmoved, receiver still ~7,000 lines uncommitted; an
installer for a mod nobody can run builds against shifting assumptions. **B1** low value. Minor:
`diag` refusal exits 0 — non-zero would make it script-detectable (Bridge's call).

### ASK 4 triage — routing for the 3 FAILs (content lane)

All three independently confirmed in §29. **Each needs a human decision, not an auto-fix:**

| # | manifest | wants | arc actually has | likely reading |
|---|---|---|---|---|
| 1 | `npc_lwood.ini` `model=` | `lwood.bdl` | `alwd.bdl`, `alwd.dzb` | **manifest filename wrong** — `alwd.bdl` is the sole `.bdl` |
| 2 | `ext_bg2.ini` `model2=` | `model1.bdl` | `model.bdl`, `model3.bdl`, `room.dzb/dzr` | LinkUG has no `model1` — stray `model2=` key, or adaptation dropped it |
| 3 | `ext_bg8.ini` `model2=` | `model1.bdl` | `model.bdl`, `model.btk`, `model3.bdl`, … | same shape as #2 (Pjavdou) |

**FINDING — #1 probably explains missing trees on Outset, and it is NOT the P1 wood system.**

`npc_lwood.ini` declares `arc=Lwood`, `model=lwood.bdl`, `static=1`, `display_name=Tree`. The arc
contains **no `lwood.bdl`**. Yet the log shows lwood actors **spawning normally**
(`[ExtNpcPop] spawn #2..#5 name=lwood proc=NPC_LWOOD`) — i.e. **placed, then unable to load a model.**

Content-lane P1 attributes tree holes to WW's wood system needing a port (~1,700 lines, `d_wood` +
`d_tree`). That diagnosis is right for **`swood`/`swood3`**, which are *generated* and have no arc.
**`lwood` is a different case** — a placed actor, already mapped, pointing at a filename that has
never existed in its arc. If `alwd.bdl` is the intended model, this is a **one-line manifest fix that
restores trees immediately**, independent of the port.

**Not asserting the fix** — `alwd.bdl` is a *candidate*, and the manifest may be pointing at a
genuinely different asset that is actually missing. That is precisely the suggest-never-fill rule,
and it is the ledger's first real test case.

**Ordering implication for the user:** verify #1 before committing to P1's scale. Some visible tree
holes may be an afternoon, not a week.

---

## §30 Bridge response — socket ledger landed (2026-07-20)

Tool **0.5.0**. §30 item 1 shipped; B2 already on bus (§29).

### Command

```bat
python -m ww_bridge ledger [--census goldens\outset_placements.csv] [--out reports\outset_socket_ledger.csv]
```

JOIN (no new extraction): Outset census names × `actor_map.ini` × ASK 3 coverage × ASK 4 arc members.
**Report-only.** Does not write manifests / actor_map. Every unfilled / mismatch row carries
`suggest-never-fill` posture in notes.

Row columns: `socket_id · census_count · wants · offered · state · candidates · forbidden_set · notes`.

| state | meaning |
|-------|---------|
| `filled_ok` | mapped + named resources present in arc |
| `filled_mismatch` | mapped but ASK-4-style missing member (candidates list arc `.bdl`/`.dzb`) |
| `unfilled_candidate` | no map section; decomp source available |
| `unfilled_no_candidate` | no map; no decomp hit |

### Success test (Outset golden, live mod)

`87` sockets → `filled_ok=39` · `filled_mismatch=0` · **`unfilled_candidate=5`** · `unfilled_no_candidate=43`.

**Work-order column (unfilled + candidate available):**

| socket | ×placements | candidate |
|--------|------------:|-----------|
| SalvagE | 10 | `d_a_salvage.cpp` |
| Salvage | 4 | `d_a_salvage.cpp` |
| Salvag2 | 4 | `d_a_salvage.cpp` |
| Akabe | 1 | `d_a_obj_akabe.cpp` |
| HyoiKam | 1 | `d_a_kamome.cpp` |

Verdict vs success test: **yes — better than intuition.** Clusters salvage family (18 placements)
as one port surface; leaves 43 absent-decomp holes off the immediate queue. Earns keeping the
report; acceptable-set grammar still deferred until a human wants alternatives per socket.

### ASK 4 #1 (lwood) vs ledger

Live `npc_lwood.ini` already has `model=alwd.bdl` / `collision=alwd.dzb` (content comment №118).
Ledger therefore classifies `lwood` as **`filled_ok`**, not mismatch — the suggest-never-fill test
case was ratified off-bus. Remaining ASK-4 FAILs (LinkUG / Pjavdou `model1.bdl`) are **interior /
ext_bg**, outside Outset v1 scope; they will appear when ledger scope widens.

Artifact: `albt bridge/reports/outset_socket_ledger.csv`.

### Next

- Housing Security accept ledger + B2 (if not already).
- Content: ratify any `unfilled_candidate` before mapping; do not auto-fill from candidates.
- B3 installer still deferred.

---

## 31. Auditor acceptance — B2 + socket ledger (2026-07-20). **BOTH ACCEPTED.**

Both re-run independently.

### B2 SCLS — ACCEPTED

Independent run on live `Outset.arc` → **10 exits**, matching the `SCLS count=10` from the auditor's
own §26 chunk survey. Destinations resolve to the known interior set.

**Corroboration worth recording:** `index 2 → Ojhous2, dest_room=1`. The content lane authored
`Ojhous2R1` (Sturgeon's room) by hand as a separate stage. **Nintendo's own exit table independently
confirms the room-1 split** — hand-authoring and extracted data agree, which is the first time those
two paths have cross-validated.

Tool stays generative (emits CSV; does not overwrite `doors.ini`), per the standing posture.

### Socket ledger — ACCEPTED

Independent run reproduces Bridge's figures **exactly**:

| state | count |
|---|---|
| total sockets | **87** |
| `filled_ok` | **39** |
| `unfilled_candidate` | **5** |
| `unfilled_no_candidate` | **43** |

**Success test passed.** The work-order column surfaces something intuition would not: the
**salvage family clusters** — `SalvagE` ×10, `Salvage` ×4, `Salvag2` ×4 → **18 placements behind a
single decomp source** (`d_a_salvage.cpp`). One port, 18 holes closed. That ranking is not visible
from the census, the coverage report, or the skip-list individually — **it only appears in the
join**, which is precisely the argument the ledger was greenlit on. (20 placements across all 5
candidate sockets.)

Equally valuable is the negative space: **43 sockets have no decomp candidate**, so they stay off the
port queue and become substitute/script/hole decisions instead. The ledger separates "can be ported"
from "must be designed" — a distinction the project has been making by hand.

### The `lwood` loop — suggest-never-fill validated end-to-end

Worth recording as a process result, not just a fix:

1. **ASK 4** detected the mismatch mechanically (arc has no `lwood.bdl`).
2. **Auditor (§30)** named `alwd.bdl` as a *candidate* and explicitly refused to assert it.
3. **Content lane** verified against the decomp — `d_a_lwood.cpp` loads
   `dRes_INDEX_LWOOD_BDL_ALWD_e` — and ratified, recording the reasoning in-file (№118).
4. Manifest fixed 02:48; ledger now classifies `lwood` as `filled_ok`.

**Detected by tool, proposed by auditor, ratified by the lane that owns the knowledge, verified
against the decomp rather than guessed** — the whole chain, inside an hour, with no auto-fill. This
is the model working exactly as designed and is the strongest evidence yet that suggest-never-fill is
the right constraint.

### AUDITOR CORRECTION — third measurement error, same root cause

§29 reported `diag` refusal "exits **0**". **Wrong — it exits 2.** The check piped into `tail`, so
`$?` captured `tail`'s status rather than python's. Bridge was correct; the finding was an artifact
of the measurement.

That is the **third** instance in two days of the same failure mode — §26/§27 literal counts (grep
included `#include` lines), §26 coverage (search missed suffixed names), and now this. In each case
the *conclusion* was carefully hedged while the *measurement* went unvalidated.

**Standing correction, now explicit:** before reporting a measurement, validate the measurement
itself — check the pipeline captures what it claims (exit codes past pipes), that the query shape
matches the data's shape, and that the filter excludes what it should. **A hedge on a bad number is
not caution.** All three were caught by Bridge re-deriving independently, which is a concrete
argument for keeping the two-lane cross-check rather than consolidating.

### Status

**B0 · B4 · schema v2 · handshake · GAP-1/2/3 · ASK 1–5 · B2 · socket ledger — ALL ACCEPTED.**
Bridge's lane is clear; no auditor items outstanding.

Open elsewhere: **L-1** gate failure (engine — sole covenant item) · ASK-4 FAILs #2/#3
(LinkUG / Pjavdou `model1.bdl`, content lane) · 5 `unfilled_candidate` sockets awaiting content-lane
ratification before any mapping · census refresh §12 · skeleton reconciliation.

---

## 32. L-1 CLOSED (auditor was stale) + ASK-4 #2/#3 RESOLVED against the decomp (2026-07-20)

### L-1 / L-2 — CLOSED. **M6 GATE CLEAN.**

`"Outset Interiors"` is **absent from `map_loader_definitions.h`**, and the live gate on the 02:20
exe returns **CLEAN (0 hits, ASCII + UTF-16)**. L-1 and L-2 are both closed; the compile-time entry
was removed.

**AUDITOR ERROR — §31 listed L-1 as the "sole open covenant item." It was already fixed.** Reported
from carried state instead of re-running the gate. User flagged it. This is the **fourth** instance
in two days of asserting without re-measuring (literal counts · coverage search · `diag` exit code ·
now stale gate state). Same standing correction applies: **re-measure before reporting, every time —
including facts that were true an hour ago.**

> **MILESTONE: the Receiver Covenant is fully satisfied for the first time.**
> Charter **A** (M6 greplist gate) — CLEAN · **B** (M5b folder-absent) — PASS (§25) ·
> **C** (commit hygiene) · **D** (audio twin guard) · **E** (mod-folder integrity) — all clean.

### ASK-4 #2 / #3 — RESOLVED. **Neither is a missing asset.**

Both manifests declare `model2=model1.bdl`; neither arc contains it:

| manifest | arc | arc actually holds |
|---|---|---|
| `ext_bg2.ini` | LinkUG | `model.bdl`, `model3.bdl`, `room.dzb`, `room.dzr` |
| `ext_bg8.ini` | Pjavdou | `model.bdl`, `model.btk`, `model3.bdl`, `room.dzb`, `room.dzr` |

**Answered from the decomp, not guessed** (the `lwood`/№118 method). `src/d/actor/d_a_bg.cpp` carries
parallel name tables — `model` / `model1` / `model2` / `model3` across `.bmd`, `.bdl`, `.btk`, `.brk`
— consumed by `for (int i = 0; i < 4; …)` with `if (modelData == NULL) continue;`.

**WW's own BG loader PROBES four optional slots and skips the absent ones.** A room with two models
is not broken — it simply has fewer table hits. Outset has `model`/`model1`/`model3`; LinkUG and
Pjavdou legitimately have no `model1.bdl`. **Nothing was dropped in adaptation.**

**The real defect is our manifest convention**, which hardcodes `model2=model1.bdl` as though all
slots always exist — a template copied uniformly across `ext_bg*`.

**Two fixes, and the second is structurally correct:**

- **(a) Per-room:** delete `model2=` from `ext_bg2.ini` and `ext_bg8.ini`. Fixes these two; every
  future room re-litigates the same bookkeeping.
- **(b) Preferred — make the receiver probe like WW does:** try the four slots, use what exists, skip
  what doesn't. Manifests stop naming model slots entirely (or keep them as overrides). This mirrors
  the donor engine's own design, which is exactly the №91 precedent (TP's `daKnob20_c` already
  implemented WW's door mechanism — nothing needed inventing). Immune for all future rooms.

**Knock-on for Bridge (small):** under (b), `model2=` naming a missing member stops being an error,
so ASK 4 must distinguish **required** keys from **optional-probe** slots — or the keys disappear
from manifests and the question is moot. No action until the receiver decision lands; flagged so the
check isn't left asserting a rule that changed.

**Routing:** (a) is content-lane and immediate. (b) is engine-lane and is the durable answer. The
auditor recommends (b) but does not own it.

---

## 33. Recommendation (b) — probe-based BG model slots. Bridge action inside. (2026-07-20)

### The recommendation (engine-lane; auditor recommends, does not own)

**Make the receiver probe the four BG model slots instead of naming them in manifests.**

Decomp-verified basis (§32): `src/d/actor/d_a_bg.cpp` holds parallel name tables —
`model` / `model1` / `model2` / `model3` across `.bmd`, `.bdl`, `.btk`, `.brk` — consumed by
`for (int i = 0; i < 4; …)` with `if (modelData == NULL) continue;`. **WW's own loader treats every
slot as optional and skips absences.**

Our `ext_bg*.ini` convention hardcodes `model2=model1.bdl` as though all slots always exist. That is
a template assumption WW itself never makes, and it is why LinkUG and Pjavdou "fail" while being
perfectly valid two-model rooms.

**Why (b) over per-room deletion:** it mirrors the donor engine's actual design, which is the №91
precedent — TP's `daKnob20_c` already implemented WW's door mechanism, so nothing needed inventing.
Same shape here: the correct behaviour already exists upstream and should be adopted rather than
worked around. Every future room becomes immune instead of re-litigating slot bookkeeping.

Manifests then either drop model-slot keys entirely, or keep them as **explicit overrides** for the
rare case where a room needs a non-default binding.

### BRIDGE ACTION — `verify` must stop treating optional slots as errors

Under (b), `model2=` naming an absent member is **legitimate**, not a defect. ASK 4 currently FAILs
on it. Required change:

- Classify manifest resource keys as **required** vs **optional-probe**.
- BG model/btk/brk slots (`model`, `model2`, `model3` and their anim siblings) → **optional**: absent
  member ⇒ **WARN or silent**, never FAIL.
- Genuinely required bindings (`arc=`, `collision=`, actor `model=`) keep FAIL semantics.

**Interim posture until the engine decision lands:** downgrade the two BG `model2=` cases to WARN so
`verify` stops reporting a rule that is changing, without losing visibility. **Do not delete the
check** — if (a) is chosen instead, FAIL remains correct.

**Sequencing:** this is *reactive* to an engine decision that has not been made. Bridge should not
build it until the user rules between (a) and (b). Recorded now so the dependency is visible rather
than discovered later.

---

## 34. ASK 6 for Bridge — decode the quest trigger network (2026-07-20)

### The finding this rests on

**The quest trigger network is already inside the census we hold.** Counted in
`outset_placements.csv` + `interior_placements.csv`:

`TagEv` ×8 · `TagKb` ×7 · `AttTag` ×5 · `ky_tag0/1` ×5 · `TagMsg2` ×3 · `AttTagB` ×2

These are Nintendo's own event/message trigger placements, **with params**, extracted months ago and
never decoded. No new extraction is required to start.

Decomp support verified present: `d_a_tag_event.cpp` (17 KB), `d_a_tag_evsw`, `d_a_tag_kb_item`,
`d_a_tag_attention`, `d_a_tag_hint`, `d_a_tag_island` + siblings; plus `d_event.cpp` (784),
**`d_event_data.cpp` (943)**, `d_event_manager.cpp` (854).

### THE ASK — decode `Tag*` params, report-only

Same pattern as ASK 2's actor param decoder, pointed at the trigger actors:

- **Input:** `Tag*` / `Att*` / `ky_tag*` census rows (already present).
- **Reference:** the matching `d_a_tag_*.cpp` param accessors + `d_event_data` structures.
- **Output:** a report — which params decode, what each trigger references (event id? message id?
  flag? switch?), and what the resulting graph looks like for one island.
- **Scope:** Outset only. **Report-only, suggest-never-fill**, consistent with the ledger.
- **Not in scope:** building quest tooling, DuskScript design, any engine change.

### The question it exists to answer

**Is WW's event flow DATA-INTERPRETED or CODE-DRIVEN?**

`d_event_data.cpp` sitting beside an event *manager*, with trigger actors carrying params in room
data, is the signature of an engine that **walks event records** rather than hardcoding flow. If that
holds, quest flow ports the way TTW ported Fallout 3 — **as data** — which is a materially better
position than the actor side.

**This is explicitly NOT yet established.** §1 of this doc asserts WW has no ESM-equivalent and that
behaviour must be ported as source. **That claim may be wrong for events specifically**, and this ask
is how we find out. Recorded as the open question, not the conclusion.

### Why it is high-leverage

Cheapest probe on the board — no extraction, no engine dependency, one island — and it gates a much
larger decision: **whether DuskScript authors quests or interprets Nintendo's.** Those are different
products. Answering before designing follows the pattern that has paid out repeatedly here (B0
measured before trusting; §15 checked before asserting; the ledger joined before designing).

### Design note for whatever follows

**Quests are a graph, not a set.** Actor sockets are independent; quest beats carry order and
dependency, so "unfilled" propagates — a beat cannot fill while its prerequisite is unfilled. Any
future quest ledger needs edges, not just rows. Flagged now so the first design does not assume the
flat shape that worked for actors.

**Success test:** does Outset's trigger network resolve into readable beats? If yes, the quest graph
is recoverable from data already on disk. If the params stay opaque, quests are code-driven and
DuskScript is an authoring tool — also a useful answer, arrived at cheaply.

---

## §34 Bridge response — ASK 6 quest-trigger decode landed (2026-07-20)

Tool **0.6.0**. §31 accept of B2 + ledger absorbed. §33 ASK-4 optional-probe **not built** — waits
on user (a)/(b) engine call.

### Command

```bat
python -m ww_bridge quest-triggers [--census goldens\outset_placements.csv]
```

Artifacts: `reports/outset_quest_triggers.md` · `reports/outset_quest_triggers.csv`.

### Inventory (Outset golden)

**25** placements · **9** names — all decomp sources resolved via stage `OBJNAME` aliases
(`d_stage.cpp`).

| name | × | flow | role |
|------|--:|------|------|
| TagKb | 10 | data | dig-item gates (not narrative) |
| TagSo / TagMSo | 5+1 | env | sonar volumes |
| TagMsg2 | 3 | data | message + EVNT index |
| AttTagB | 2 | env | attention |
| TagEv / TagIsl / TagHt | 1 each | hybrid | event order / island / hint |
| ky_tag1 | 1 | env | waves |

### Verdict — DATA vs CODE

**Hybrid, with a clean split** (decomp-confirmed, not guessed):

- **DATA selection / gating:** `eventNo` (stage EVNT index), `message_id` (`home.angle.x`),
  `event_bit` (`home.angle.z`), room switches in params. `d_event_data` walks staff/cut once ordered.
- **CODE demos:** `daTag_Event_c::demoInitProc` / `demoProc` switch on `getType()` with hardcoded
  partners / BGM / named events (`SUPERELF`, `BEAST_GATE2`, …).
- **Not quest beats:** TagKb, TagSo*, AttTag*, ky_tag*.

§1's "no ESM-equivalent" claim is **too strong for events**: selection is data; a minority of TagEv
types remain code. Implication: DuskScript should **interpret** Nintendo's selectors first, and only
author replacements where type-switched demos have no data record.

### Success test

- Distinct message ids: **2** → `0x9c5`, `0x168b`
- Distinct eventNo indices: **3** → `1`, `15`, `53`
- **PASS (partial).** Selectors readable from census already on disk. Named beats need stage **EVNT**
  + message tables — adapted `Outset.arc` room.dzr has **no EVNT chunk**, so names do not resolve
  from the live mod alone.

### Next

- Housing Security accept / amend ASK 6 verdict.
- Optional follow-up: EVNT extract when a stage dump with the chunk is available (not inventing names).
- §33 ASK-4 optional-probe still gated on user (a)/(b).

---

## 35. Auditor acceptance — ASK 6. **ACCEPTED**, plus the EVNT follow-up LOCATED (2026-07-20)

### ASK 6 — ACCEPTED

Independent run reproduces Bridge exactly: **25 triggers · 9 names** (`TagKb`×10, `TagSo`×5,
`TagMsg2`×3, `AttTagB`×2, `TagMSo`/`ky_tag1`/`TagEv`/`TagIsl`/`TagHt`×1).

**The hybrid verdict is accepted, and it corrects this doc.** §1 asserted WW has "no ESM-equivalent"
and that behaviour must be ported as source. **Too strong for events.** Selection *is* data —
`eventNo` (stage EVNT index), `message_id`, `event_bit`, room switches — while a minority of `TagEv`
types run code-side demos. §34 flagged this as the open question; it is now answered, and the answer
is *partly* the TTW situation rather than none of it.

**Design consequence, adopted:** DuskScript should **interpret Nintendo's selectors first**, and
author replacements only where a type-switched demo has no data record. That is a materially
different brief from "authoring tool," and it was worth the probe to learn cheaply.

Also valuable is Bridge's **negative** classification: `TagKb`, `TagSo*`, `AttTag*`, `ky_tag*` are
**not narrative beats** (dig gates, sonar volumes, attention, waves). That removes 22 of 25
placements from the quest surface — the real narrative trigger count for Outset is **~3**, not 25.
Scope shrinks dramatically once classified.

### The EVNT follow-up — RESOLVED to a location

Bridge left this open ("EVNT extract when a stage dump with the chunk is available"). Auditor closed
it in two steps:

**1. The adaptation is CLEAN — this is NOT a second rotation-loss.** Compared original
`Ex WW/files/res/Stage/sea/Room44.arc` against the adapted `arcs/Outset.arc`:

```
ORIGINAL sea/Room44 : 31 chunk types
ADAPTED  Outset.arc : 31 chunk types
chunks in original but NOT adapted : none
EVNT in ORIGINAL : False      EVNT in ADAPTED : False
```

**Nothing was dropped.** `EVNT` genuinely does not exist in Outset's *room* data. The pipeline is
exonerated — worth stating explicitly given §12's precedent, where a comparable gap *was* data loss.

**2. It lives at STAGE level, and it is compressed.** `sea/Stage.arc` (149,191 b) and
`A_mori/Stage.arc` (41,935 b) both carry magic **`Yaz0`** — which is why a raw RARC scan skips them.
Room DZRs hold per-room data; the stage DZS holds stage-wide tables including `EVNT`.

**Concrete follow-up for Bridge:** Yaz0-decompress `Stage.arc` and read the stage DZS. The project
already owns Yaz0 handling — `adapt_bdl_arcs.py` is documented Yaz0-aware — so this is reuse, not new
capability. Named beats should resolve from there; the `eventNo` indices already decoded
(**1, 15, 53**) become lookups into that table.

**Constraint unchanged:** if names do not resolve, they stay unresolved. **Do not invent beat
names** — the IVAN RULE applies to events exactly as it does to people.

### Status

ASK 6 **ACCEPTED**. Bridge's open follow-up is now a located, scoped task rather than a blocked one.
§33 optional-probe remains correctly gated on the user's (a)/(b) engine call — Bridge was right not
to build it.

---

## 36. EVNT TABLE DECOMPRESSED AND LOCATED — ASK 6 follow-up unblocked (2026-07-20)

§35 traced the missing `EVNT` chunk to Yaz0-compressed stage arcs. **Now decompressed and confirmed.**

### Artifacts (user-directed)

```
D:\XXXXXXX\Ex WW\_extracted\Stage_decompressed\sea\Stage.arc      149,191 → 773,920 b
D:\XXXXXXX\Ex WW\_extracted\Stage_decompressed\A_mori\Stage.arc    41,935 → 207,808 b
```

Both verified `RARC` after decompression. Written under the existing `_extracted/` convention so the
**pristine extract tree is untouched**.

**Method note:** the tool at `D:\Dolpheen Plz\Toolz` is `GameCube File Tools.exe` — a **GUI** app
that will not run headless. Used the project's own `yaz0_dec()`
(`tools/ww_crew_restoration_skeleton/adapt_bdl_arcs.py:24`) instead — already proven across 577 arcs,
and avoids introducing a second Yaz0 implementation whose output would need separate validation.

### EVNT CONFIRMED PRESENT

| stage | chunk types | EVNT entries |
|---|---:|---:|
| `sea/stage.dzs` | 15 | **57** |
| `A_mori/stage.dzs` | 14 | **6** |

Full `sea` chunk set: `ACTR:4 Colo:10 EVNT:57 EnvR:52 MULT:50 Pale:57 RARO:1 RCAM:1 RPAT:4 RPPN:40
RTBL:50 SCLS:212 SCOB:50 STAG:1 Virt:37`

### THE UNBLOCK for Bridge

**The `eventNo` indices ASK 6 already decoded — `1`, `15`, `53` — are indices into
`sea/stage.dzs`'s 57-entry EVNT table.** That table is now on disk, decompressed. Named beats should
resolve directly; no further extraction is needed to attempt it.

**Standing constraint (IVAN RULE, applied to events):** if a name does not resolve, it stays
unresolved. **Do not invent beat names.**

### Three incidental findings worth routing

1. **`SCLS:212` at STAGE level.** B2 extracted **10** room-level exits from Outset. The stage table
   holds **212** — the sea-wide exit graph across every island. That is a far larger door/warp
   surface than current tooling has seen, and it is the natural B2 follow-on when islands beyond
   Outset come into scope.
2. **`RTBL:50` + `MULT:50`** — room table and multi-room offsets, 50 entries matching sea's room
   count. This is the authored data behind room hosting, which the interiors pivot has been
   **hand-authoring**. Potentially relevant to the content lane; routed via user, not asserted.
3. **`A_mori` carries only 6 EVNT entries** — the forest's narrative surface is genuinely small,
   consistent with ASK 6's finding that Outset's real narrative trigger count is ~3, not 25.

### Status

ASK 6's follow-up is **no longer blocked** — located, decompressed, verified. Bridge's remaining
queue: resolve `eventNo` → beat names from the EVNT table, and the §33 optional-probe change which
stays correctly gated on the user's (a)/(b) engine ruling.

---

## §36 Bridge response — eventNo → EVNT names resolved (2026-07-20)

Tool **0.7.0**. Auditor §36 unblock consumed; names resolved. **No invented beats.**

### Commands

```bat
python -m ww_bridge evnt --lookup 1 15 53
python -m ww_bridge quest-triggers
```

Reads Yaz0 `sea/Stage.arc` directly (same `yaz0_dec` as `adapt_bdl_arcs.py`); also works on the
auditor's decompressed copy under `_extracted/Stage_decompressed/`. Default:
`D:\XXXXXXX\Ex WW\files\res\Stage\sea\Stage.arc` (`WW_EXTRACT` / `--stage-arc`).

### Proof — ASK 6 indices → Nintendo names

| eventNo | EVNT name | used by |
|--------:|-----------|---------|
| 1 | `STOLENSISTER` | TagEv ×1 |
| 15 | `AJ_SPEAK` | TagMsg2 ×3 |
| 53 | `PUROLO_RETURN` | TagIsl ×1 |

`sea` EVNT = **57** entries (matches auditor). Artifact: `reports/sea_evnt.csv`. Updated
`outset_quest_triggers.md` success test → **PASS** (3/3 named).

### Notes on §36 incidentals (ack, not claimed as shipped)

1. Stage `SCLS:212` — noted as natural B2 expansion beyond Outset room exits; not extracted this turn.
2. `RTBL`/`MULT` — content-lane routing, out of Bridge.
3. `A_mori` EVNT×6 — consistent with thin forest narrative surface.

### Next

- Housing Security accept name resolve.
- Optional: message ids `0x9c5` / `0x168b` → BMG text (separate ask).
- §33 ASK-4 optional-probe still gated on user (a)/(b).

---

## 37. Quest surface is a SLICE, not the whole · Animations ARE socket-shaped — ASK 7 (2026-07-20)

### Q1: was ASK 6 "all of Outset's quest"? **NO — it is one slice.**

Measured across every Outset-relevant stage (Yaz0-decompressed where needed):

| stage | EVNT | | stage | EVNT |
|---|---:|---|---|---:|
| `sea` | **57** | | `Cave09` | 2 |
| `A_mori` | 6 | | `Ojhous` | 1 |
| `LinkRM` | 5 | | `Pjavdou` | 1 |
| `Ojhous2` | 2 | | `Omasao` / `Onobuta` / `LinkUG` | 0 |

**Total: 74.** And `sea`'s 57 is **not Outset-only** — `sea` hosts ~50 rooms (`RTBL:50`), so those 57
cover *every island*. Outset's exterior share is the `eventNo` set its triggers reference (`1, 15,
53`).

**What ASK 6 covered:** exterior trigger placements in one room.
**What it did not:** the 9 interior stages' own EVNT tables (17 entries), the story-layer beat
structure (`ACT0`–`ACTb`, already in census and flag-mapped in `actor_map.ini`), and any beat
advanced by **dialogue** rather than a trigger volume.

**Practical read:** the exterior narrative surface really is ~3 beats, as ASK 6 found. The *full*
Outset quest is that plus ~17 interior events plus the layer structure. **Still small and bounded —
which is good news — but "done" would be wrong.**

### Q2: animations — **YES, and the mapping already exists, per-arc, for USA**

Animations are **not** placement data (no census equivalent), so the socket shape differs — but it
fits cleanly, and the decomp supplies the binding layer outright.

**Discovery:** `D:\XXXXXXX\WW DP\assets\GZLE01\res\Object\<Arc>.h` — **577 generated per-arc headers**
for **GZLE01 (USA)**, matching this project's §3 region pin. Each maps semantic resource slots to
indices, e.g. for `Kb`: `dRes_INDEX_KB_BCK_DAMAGE1_e`, `…_EAT1_e`, `…_NAKU1_e` (9 BCK slots).

**Why this matters — the earlier measurement was misleading.** Naive grep for anim filenames in
actor source found `Md` 99/100 but `Kb`/`Kamome`/`Knob`/`Mk` **0**. That is not absence: most actors
address animations by **resource index enum**, not string literal. `Md` is the outlier (which also
explains its 161 runtime literals in §27). **The binding is there for every arc — it is just
indexed, not named inline.**

**The socket form:**

| | |
|---|---|
| socket | a named slot, e.g. `dRes_INDEX_KB_BCK_EAT1_e` |
| filler | the BCK/BRK/BTK/BTP member inside the arc |
| state | slot present in arc · orphaned (in arc, unaddressed) · missing (addressed, absent) |
| join | arc contents (ASK 4's `list_rarc`) × res header × actor code |

Sample gap already visible: `Kb.arc` holds **13** anim members; the header declares **9** BCK slots.
That delta is exactly what the ledger would explain.

### ASK 7 (proposed) — animation coverage ledger

Same join pattern as the socket ledger, report-only, Outset arcs only. Output per arc: declared
slots · present · orphaned · missing · which our manifests currently bind.

**Value:** manifests today bind a handful of anims by hand (`companion_idle=`, `door_open_bck=`).
This shows, per actor, **the full authored animation vocabulary** — so idle/talk/damage/eat states
stop being guessed or omitted. Directly serves the doctrine that behaviour is never invented.

**§27 STEP 4 — RESOLVED, and it is NOT a leak vector.** Those headers hold **enum identifiers**
(compile-time), not runtime strings. They do not reach the binary. The open question from the
porting policy is closed: **generated resource headers are safe.**

### Routing

ASK 7 needs user greenlight. Q1's interior-EVNT gap is content-lane scope information, not a Bridge
task. **Not asserting either as work** — recorded so the quest surface is not mistaken for complete.

---

## 38. ASK 7 (animations) + ASK 8 (NPC behaviour) — BOTH GREENLIT by user (2026-07-20)

Same join pattern as the socket ledger. **Report-only, suggest-never-fill, Outset scope.**

### ASK 7 — animation coverage ledger

**Binding layer already exists:** `D:\XXXXXXX\WW DP\assets\GZLE01\res\Object\<Arc>.h` — **577 per-arc
headers**, USA build, matching the §3 region pin. Each declares named slots
(`dRes_INDEX_KB_BCK_EAT1_e`, `…_DAMAGE1_e`, `…_NAKU1_e`).

**Join:** arc members (ASK 4's `list_rarc`) × res-index header × actor source × our manifest bindings.

| socket | a declared slot, e.g. `dRes_INDEX_KB_BCK_EAT1_e` |
|---|---|
| filler | the `.bck` / `.brk` / `.btk` / `.btp` member in the arc |
| states | `present` · `orphaned` (in arc, no slot) · `missing` (slot, no member) · `bound` (our manifest uses it) |

**Known gap to explain first:** `Kb.arc` holds **13** anim members; its header declares **9** BCK
slots.

**Do not repeat the naive grep.** Counting anim filenames in actor source gives `Md` 99/100 and
`Kb`/`Kamome`/`Knob`/`Mk` **0** — a false negative. Most actors address animations by **index enum**,
not string literal (`Md` is the outlier; also why it carries 161 runtime literals, §27). **Resolve
through the header, not by name-grep.**

**Value:** manifests currently bind a handful of anims by hand. This exposes each actor's **full
authored animation vocabulary**, so idle/talk/damage/eat stop being guessed or omitted.

### ASK 8 — NPC behaviour coverage ledger

**Key correction to the existing socket ledger:** `filled_ok` currently means *mapped + resources
present*. **It does NOT mean the actor behaves.** A `static=1` mount is `filled_ok` and completely
inert. The ledger conflates *renders* with *behaves*, and ASK 8 is the fix.

**Measured baseline (live manifests, 68 total):**

| | count |
|---|---:|
| `static=1` — **inert mounts, zero behaviour** | **27** |
| non-static (some behaviour declared) | 41 |
| entire behaviour vocabulary in use | `idle` ×30 · `dialogue` ×27 · `talk1` ×24 · `door_open_bck` ×1 · `companion_idle` ×1 |

**Three keys deep.** WW's actors carry far richer state machines than idle/talk/dialogue.

**Tiering — build 1 and 2; do NOT attempt 3 yet:**

1. **Inventory (cheap).** Per census actor: `inert` / `minimal` (idle+talk only) / `none`, joined
   against ASK 3 decomp availability → **the actionable column is "inert but portable."**
2. **State count (medium).** From decomp source, how many action/proc states does the actor
   implement? Turns "has AI" into a number and sizes each port.
3. **Semantic state mapping (hard — OUT OF SCOPE).** Mapping a WW state to a TP equivalent is design
   judgement, not extraction. Flagged so tier 2's number is not mistaken for a port plan.

**Value:** answers *"which actors are standing still that could be alive, and how big is each?"* —
directly feeding the ordering problem, the same way the salvage cluster (18 placements, one source)
fell out of the socket ledger.

### Shared constraints

- **Report-only. Suggest-never-fill.** No manifest writes.
- **Outset scope** for v1, per the one-exemplar rule (№63).
- **Do not invent** slots, states, or names that do not resolve — IVAN RULE applies to animation
  slots and behaviour states exactly as to people.
- Both extend existing joins; neither needs new extraction.

### Also resolved here

**§27 step 4 CLOSED — generated resource headers are NOT a leak vector.** They hold **enum
identifiers** (compile-time), not runtime strings; they never reach the binary. The porting policy
now has no unverified vectors.

---

## §38 Bridge response — ASK 7 + ASK 8 landed (2026-07-20)

Tool **0.8.0**. Both greenlit asks shipped. Report-only / suggest-never-fill / Outset scope.

### ASK 7 — `anim-ledger`

```bat
python -m ww_bridge anim-ledger
```

JOIN: `assets/GZLE01/res/Object/<Arc>.h` slots × live arc anim members × manifest anim keys.
**Resolved through headers, not actor-source name-grep.**

| figure | value |
|--------|------:|
| Outset-scoped arcs | **34** |
| res headers found | **32** |
| arcs with missing slots | **0** |
| arcs with orphans | **2** (`WwAlways`, `WwDalways` — custom, no GZLE01 header) |

**Kb spotlight (auditor sample gap):** header **20** anim slots (BCK×9 + BAS×7 + BTP×4) · arc
**20** members · present=20 · missing=0 · orphaned=0 · bound=`npc_kb.idle=wait1.bck`.

The "13 vs 9 BCK" reading counted BCK-only slots against BCK+BTP members and ignored BAS. Full
header join **closes the gap** — nothing missing, nothing orphaned.

Artifacts: `reports/outset_anim_ledger.md` · `.csv`.

### ASK 8 — `beh-ledger`

```bat
python -m ww_bridge beh-ledger
```

Corrects socket-ledger `filled_ok` (= renders) vs behaves. Tier 3 semantic WW→TP mapping **not
attempted**.

| tier1 | count |
|-------|------:|
| inert (`static=1`) | **19** |
| minimal (idle/talk/dialogue) | **24** |
| richer | 0 |
| unmapped | **44** |

**Actionable column (NPC only; Tag* excluded → ASK 6):**

| actionable | count |
|------------|------:|
| `inert_but_portable` | **7** |
| `minimal_vs_rich_decomp` | **2** |

Top work-order by placements: `item`×14 · `bridge`×11 · `lwood`×6 · `P1a`×5 (minimal vs 8-state
decomp) · `Kanban`×3 · …

Artifacts: `reports/outset_behaviour_ledger.md` · `.csv`.

### Next

- Housing Security accept ASK 7/8.
- Content ratifies any bind from anim orphans / inert_but_portable — tool does not auto-fill.
- §33 ASK-4 optional-probe still gated on user (a)/(b).

---

## 39. ASK 7/8 ACCEPTED — and the DEPLOYMENT GAP, stated plainly (2026-07-20)

### ASK 7 / ASK 8 — ACCEPTED

**ASK 7 result is "nothing is wrong," and that is a real finding.** 34 Outset-scoped arcs, 32 headers,
**0 missing slots**, orphans only in `WwAlways`/`WwDalways` (custom arcs with no GZLE01 header —
expected). **Animations are healthy and are ruled out as a source of problems.** Negative results
have value; this one closes a whole category.

**FIFTH auditor measurement error.** §37 flagged "`Kb`: 13 anim members vs 9 BCK slots." Wrong — I
compared **BCK-only slots** against **BCK+BTP members** and ignored **BAS** entirely. Full header
join: 20 slots, 20 members, present=20, missing=0, orphaned=0. Same failure mode as the other four —
**the query's shape did not match the data's shape.**

**ASK 8 accepted.** Tier1: 19 inert · 24 minimal · **0 richer** · 44 unmapped. Actionable:
**7 `inert_but_portable`**, 2 `minimal_vs_rich_decomp`. Work-order by placements: `item`×14 ·
`bridge`×11 · `lwood`×6 · `P1a`×5 (minimal vs an 8-state decomp actor) · `Kanban`×3. Tier 3 correctly
not attempted.

### THE DEPLOYMENT GAP — what any of this changes in-game today: **almost nothing**

User asked directly. Verified against the live folder:

| tool output | deployed? |
|---|---|
| `bridge_meta.ini` (schema v2 handshake) | **ABSENT** — engine still takes the legacy v0 path every boot |
| v2 census columns (`set_id`, scale) | **NO** — live CSVs are still **11 columns**, v1 shape |
| §12 recovered rotations | **NO** — LinkRM still **0 of 18** rows with rotation |
| socket / anim / behaviour ledgers | reports only — **never read by the game** |
| ASK 6 quest triggers, B2 SCLS exits | reports only |

**Exactly one change from this lane has reached the running game:** `npc_lwood.ini`
`model=lwood.bdl` → `alwd.bdl` (content lane, 02:48). 24 lwood actors spawn in the newest log with no
load failure — **trees plausibly render now, but that needs eyes, not a log.**

**This is by design, not a defect.** Suggest-never-fill means the tool never writes what the game
reads, and the auditor argued for that constraint. But the distinction must stay explicit:
**ASK 1–8 built an accurate map of what to do. Almost none of it has been done.**

### What would actually produce visible change, ranked by cost

1. **§12 census refresh** — deploys recovered rotations (**7 stages currently facing default-north**),
   moves live to v2, and activates the handshake. Open since §12. **Single highest-value action on
   the board.** Content lane.
2. **7 `inert_but_portable` actors** (ASK 8) — statues that could move.
3. **5 `unfilled_candidate` sockets** (§30) — salvage family alone is **18 placements behind one
   decomp source**.
4. Animations — **nothing to do**; ASK 7 says clean.

---

## 40. IDENTIFY-BY-Z-TARGET — the IVAN-RULE-safe identity probe (2026-07-20)

**User proposal:** Z-target a character in game; the log names which census actor it is. **This is the
correct way to resolve identity** — the user identifies the face, the engine reports the code, nobody
invents a mapping. It is what §27/IVAN require and it unblocks the `P1a`/Aryll question that §39
correctly refused to guess at.

### Nearly all of it already exists

| piece | status |
|---|---|
| ExtNpc actors are Z-targetable | **exists** — `d_ext_npc_mount.cpp:5108` sets `fopAc_AttnFlag_TALK_e \| SPEAK_e` |
| get the currently locked actor | **pattern proven** — `attn->LockonTarget(0)` (`d_albw_flurry_rush.cpp:497`) |
| resolve an actor → identity | **exists** — `dExtNpcMount_queryActor(actor, dExtNpcIdentifyInfo*)`; already used by the level-editor highlight/pick path (`leveledit/pick.cpp:445, 591`) |
| **census code retained on the mount** | **exists** — `m->mSpawnSrc` holds `census:<Name>@(x,y,z)`; read back at `d_ext_npc_population.cpp:334` |

**The only gap:** `dExtNpcIdentifyInfo` (`d_ext_npc_mount.h:227`) carries `proc`, `displayName`,
`modFolder`, `headModel`, `headJoint`, `socketArg`, `headVariant` — but **no census field**, and
nothing logs on lock-on.

### ENGINE HALF — not Bridge's, recorded for user routing

Two small changes:

1. Add `char censusName[32]` (or `spawnSrc`) to `dExtNpcIdentifyInfo`, populated from the mount's
   existing `mSpawnSrc`. **The data is already there — this only exposes it.**
2. On Z-target lock of a mount actor, log once per target change:
   `[ExtNpcId] Z-target census=<P1a> proc=<NPC_P1> arg=<41> head=<p1a_head.bdl> display=<? (unverified)>`

Then: aim at the character, read one line, and the census code is established **by observation**.
Feed it to `identity.ini` via the existing Lock-identity button, which already writes through
`dExtNpcMount_setDisplayNameNearest`.

**Why lock-on beats "nearest within 800u"** (the current editor path): in a populated space nearest
is ambiguous and silently picks the wrong actor. **Z-target is an explicit, unambiguous selection by
the user** — which is exactly the property an identity probe needs.

### BRIDGE HALF — `identify` dossier

Once a census code is known, join everything the ledgers already hold:

```bat
python -m ww_bridge identify <census_code>
```

Output per code: census placements + params · `actor_map` proc/arg/head · decomp source & state
estimate (ASK 3/8) · anim slots declared vs bound (ASK 7) · socket state (§30) · current
`display_name` and whether identity is **locked or `? (unverified)`**.

**Value:** turns "who is this?" into a single dossier at the moment the user has just pointed at
them — the natural companion to the engine's one log line, and pure join over existing reports.

### Standing constraint

The engine reports a **code**; the user supplies the **name**. `identity.ini` is currently **empty**
(only a commented example, which happens to use Aryll as illustration — that example is **not a
fact** and must not be treated as one). Nothing is locked until the user locks it.

---

## 41. BUILD SPEC — Z-target identity probe (2026-07-20)

Everything needed already exists (§40). This is the **complete** change list. **Engine-lane work,
routed via user — not Bridge's.**

### Engine — three small pieces

**1. Expose the census code that is already stored.**
`dExtNpcIdentifyInfo` (`include/d/d_ext_npc_mount.h:227`) gains:

```cpp
char censusName[32]{};   // from mSpawnSrc, "census:<Name>@(x,y,z)" -> "<Name>"
```

`dExtNpcMount_queryActor()` populates it by parsing the mount's existing `mSpawnSrc` — strip the
`census:` prefix, cut at `@`. **No new state; the field is already retained** (read back today at
`d_ext_npc_population.cpp:334`).

*Edge case to handle:* not every mount is census-spawned — door lanes use `door:<name>`. If the
prefix is not `census:`, copy the raw `mSpawnSrc` and let the log show it. **Do not blank it** — an
unexpected source is information.

**2. A poll, beside the existing ones.**
`d_s_play.cpp:790` already runs `dExtNpcMount_pollBgWarps(); dExtNpcDoors_poll();`. Add
`dExtNpcMount_pollIdentifyProbe();` there.

```
attn = dComIfGp_getAttention();            // pattern proven: d_albw_flurry_rush.cpp:492
target = attn->LockonTarget(0);
if (target != s_lastTarget) {              // fire on CHANGE only
    s_lastTarget = target;
    if (dExtNpcMount_isMountActor(target) && dExtNpcMount_queryActor(target, &info))
        log one line, ++counter;
}
```

**FPS note:** one pointer compare per frame; the log line fires **only on target change**, never
per-frame. Negligible against `build-fps-guidelines`.

**3. The log line — numbered, because the workflow is ordinal.**

```
[ExtNpcId] #1 census=P1a proc=NPC_P1 arg=41 head=p1a_head.bdl display=? (unverified)
```

The running `#N` is what makes the user's method work: **Z-target characters in a deliberate order,
note on paper who was who, then map `#N` → census code afterwards.** Without the counter the log is
a set; with it, it is a sequence.

### Why `censusName` and not just proc+arg

`P1a` and `P1b` **share** `proc=NPC_P1`. Proc+arg alone cannot separate them; the census code can.
This is the reason piece 1 exists rather than reading the existing fields.

### User workflow once built

1. Boot, walk Outset.
2. Z-target each character in a deliberate order; note on paper who each one is.
3. Quit, read the `[ExtNpcId]` lines, map `#N` → census code.
4. Lock names via the editor's existing Lock-identity button
   (`dExtNpcMount_setDisplayNameNearest`) or by hand into `identity.ini`.

**The engine reports the code; the user supplies the name.** IVAN RULE satisfied by construction —
identity comes from the user pointing at someone, never from inference.

### Bridge — optional follow-on, not required

`ww_bridge identify <census_code>` (§40): dossier joining census placements/params · `actor_map`
proc/arg/head · decomp source + state estimate (ASK 3/8) · anim slots declared vs bound (ASK 7) ·
socket state (§30) · lock status. **Useful, but the probe works without it** — build the engine half
first, identify the cast, then decide whether the dossier earns its keep.

### Auditor note

Nothing here touches containment. Census codes are already in logs (spawn ledger prints
`src=census:<Name>@…` today), so this exposes **no new class of string** and the M6 gate is
unaffected. Display names remain data-side in `identity.ini`, never compiled — R6 preserved.

---

## 42. USER BUG REPORT CONFIRMED — 24 spawn points missing from the live census (2026-07-20)

**User report:** *"the others have had their spawn points and placements broken somehow."* **Confirmed,
measured, and root-caused.**

### The measurement

| | live CSV | regenerated golden |
|---|---:|---:|
| `PLYR` spawn rows (all interiors) | **4** | **28** |

**24 spawn points are absent from the live census.** By stage (golden): `Omori` 10 · `LinkRM` 5 ·
`A_mori` 4 · `Pjavdou` 3 · `Onobuta` 2 · `Ojhous`/`Ojhous2`/`Ojhous2R1`/`Omasao` 1 each.

`LinkRM` alone is missing all five — every `PLYR Link` arrival point, including
`(-255.0, 0.0, 1125.0)` which sits exactly on the room's `KNOB00` door.

### Root cause — already known, never applied

This is **not new damage**. §12 recorded it: *"the +24 gen rows are `PLYR` spawn entries that the
older process filtered out."* The old extraction never captured `PLYR`; the tool does. **The count
matches exactly — 24 then, 24 now.**

So the live census carries **two** defects, both fixed by the same unapplied action:

1. **24 missing `PLYR` arrival spawns** (this report)
2. **Zeroed rotations across 7 stages** (§12 — `LinkRM` still 0/18)

**Both are cured by the §12 census refresh**, open since §12 and still not deployed (§39 confirmed
live is v1/11-column with rotations at 0). The user is now *seeing* in game what the tool measured
hours ago.

**This materially raises §12's priority.** It was previously "recovered rotations"; it is now
**"recovered rotations + the arrival spawn points the door system needs."** Broken arrival placement
is a gameplay-visible defect, not a polish item.

### Identity — LinkRM has NO NPCs in census

Checked directly: `LinkRM`'s 18 live rows are **entirely props and triggers** — `KNOB00`, `swood`×2,
`Lamp`, `Ba1`×5, `MPot`, `MOsara`, `MKoppu`×2, `SPitem`, `TagEv`×2, `bonbori`, `ky_tag1`. **No NPC
actor is placed in Grandma's house by the room census.**

So the two characters the user Z-targeted are **not** LinkRM census rows. Outset's exterior carries
16 mapped folk codes:

`Ah`×4 · `Aj1`×2 · `Ajav`×2 · `Bb`×1 · `Bm1`×2 · `Dk`×3 · `Ko1`×4 · `Ko2`×3 · `Ls1`×3 · `Ob1`×3 ·
`P1a`×5 · `P1b`×1 · `P2b`×1 · `Ym1`×3 · `Ym2`×5 · `Zl1`×5

**No identity is asserted here.** Every one of these still reads `? (unverified)`, `identity.ini` is
empty, and §39 already refused the `P1a`→Aryll guess. The ordered Z-target log (§41) is what closes
this — **and it is not built yet**, so the user's "1st Grandma, 2nd Aryll" cannot be bound to codes
without it.

**Open question for the user (not inferable):** were those two targeted **inside** Grandma's house or
**outside** on Outset? If inside, they are being spawned by something other than the room census —
which is itself a finding worth chasing.

### Routing

- **§12 census refresh → content lane. Now the highest-value action on the board**, with a
  user-visible symptom attached.
- **§41 Z-target probe → engine lane.** Until it exists, identity stays unresolved by design.

---

## 43. L-5 — WW MARKERS IN THE BINARY, and the M6 GATE MISSED THEM (2026-07-20)

User called this correctly: `d_a_ww_grass` / `d_a_ww_bridge` reaching the exe **is** a leak. Auditor
under-called it. Measured, it is three separate problems.

### A — WW-marked strings in the shipped binary (covenant)

| string | count |
|---|---:|
| `…\src\d\actor\d_a_ww_bridge.cpp` (full absolute path) | 1 |
| `…\src\d\actor\d_a_ww_grass.cpp` (full absolute path) | 1 |
| `WwBridge` (incl. `[WwBridge]` log tag ×4) | 5 |
| `WwGrass` (incl. `[WwGrass]` log tag ×2) | 3 |

~13 WW-marked strings. Source paths arrive via `__FILE__` in the logging macros; the tags are plain
runtime literals.

### B — 724 absolute source paths embedded build-wide

Not WW-specific, but it is the **mechanism** carrying A. Every file using a `__FILE__`-bearing macro
embeds its full path.

### C — the developer's username appears **725 times** in the binary

`C:\Users\xxxxx\…` throughout. **This is a distribution-hygiene defect independent of the covenant** —
a shipped binary should not disclose the author's home directory layout. Raised here because it has
never been flagged and it ships with every build.

### THE GATE FAILURE — auditor's instrument, auditor's fault

**`gate` reported CLEAN while 13 WW-marked strings sat in the binary.** The greplist is a list of
**character and place names**; it has never checked for **`ww` as a marker** in paths, symbols or log
tags. Every §22/§31/§32 "GATE CLEAN" was true *of the greplist* and **incomplete as a covenant
statement**.

This is the third demonstrated limit of the instrument (UTF-16 was defence-only; `Link's House` is a
shared-name blind spot; now marker-class). **Correction — the gate needs two check classes, not one:**

1. **Forbidden names** (existing greplist) — characters, places, dialogue.
2. **Forbidden markers (NEW)** — regex over binary strings for `ww_`, `Ww`, `_WW_` appearing in
   **source paths, symbol names and log tags**, with the §22 exemptions (TP-native leftovers, WW
   Skins lane) still honoured.

Auditor owns implementing class 2 into the gate. **Until it exists, "GATE CLEAN" should be read as
"no forbidden names," not "covenant clean."**

### Fixes (engine-lane; user routes)

- **Immediate:** rename `d_a_ww_grass.cpp` / `d_a_ww_bridge.cpp` to neutral (`d_a_ext_grass`,
  `d_a_ext_bridge` — matching `EXT_BG0` / `NPC_TPOST` convention) and neutralise the `[WwBridge]` /
  `[WwGrass]` log tags. Kills all 13.
- **Structural (fixes B and C at once):** strip or rewrite source paths at build time
  (`/d1trimfile:` on MSVC, `-ffile-prefix-map` on clang). One build-config change removes 724 paths
  **and** 725 username occurrences.

---

## 44. ASK 9 — DAY-1 FOUNDATION GUARD (world-state golden test)

**User requirement:** *actors stay where they should be at every stage, layer, and progression point;
vanilla is the guide, not AI judgement; Outset day 1 is the foundation layer.*

### Why this is achievable now

**The golden census IS vanilla ground truth** — mechanically derived from Nintendo's arcs and
byte-identically reproducible (B0). No human or model judgement is in it. Three assertion links
exist; **only two are guarded**:

| link | guarded? |
|---|---|
| arc → census | **yes** — A-3 golden diff / `diff-live` |
| census → live data | **partly** — `verify` |
| **live data → what actually spawns** | **NO — this is the gap** |

Today's incident is the proof: the log reported 161 actors spawned while the island was empty of
people. **Nothing compared expectation against reality.**

### THE ASK

Parse a run log's spawn ledger — `[Spawn] src=census:<Name>@(x,y,z) proc=…`, already emitted for
every actor — and diff it against the census filtered by layer gating. Report **missing · extra ·
displaced**, per name.

**Progression dimension for free:** `actor_map.ini [layers]` already maps `ACT0`–`ACTb` → quest
flags, so the expected set is a **function of flag state**. **Day-1 = the `ACT0` + always-on slice** —
the smallest complete assertion, and every later beat is the same query with different flags.

### CRITICAL DESIGN CONSTRAINT — record intent, not just presence

Grass is deliberately unmapped (`kusax1/7/21`, №128 — the confirmed cause of the vanishing cast, and
it **stays off**). A naive comparator reports three missing names every run and gets ignored inside a
week.

**The baseline must carry a ratified exceptions registry:** each deliberate absence signed off with
its reason and ledger reference, so intent reads as intent and only *unratified* drift is a
regression. **Without this the tool cries wolf and the signal is lost.**

Consequence: the baseline is a **reviewed artifact**, not an auto-capture. Someone ratifies "this is
day-1 Outset" once; drift from it is mechanical thereafter.

### Scope

v1: **Outset day-1 only**, report-only, suggest-never-fill. No engine change — the spawn ledger
already emits everything required.

### Extension path — the Great Sea

The same query generalises with no new machinery: swap the census, swap the layer/flag state. Stage
`sea` already yields **`SCLS:212`** (the sea-wide exit graph) and **`RTBL:50`/`MULT:50`** (50 rooms).
**Day-1 Outset is the exemplar; the sea is the same assertion run 50 times.** Getting the exceptions
registry and the layer-filter right *once*, at island scale, is what makes that tractable.

---

## §44 Bridge response — ASK 9 day1-guard + identify (2026-07-20)

Tool **0.9.0**. ASK 7/8 accept (§39) absorbed. ASK 9 shipped; §40 `identify` dossier also landed
(cheap join). **Engine Z-target probe (§41) is not Bridge's.**

### ASK 9 — `day1-guard`

```bat
python -m ww_bridge day1-guard path\to\dusklight-*.log
```

JOIN: golden Outset census × `[layers]` day-1 filter × `actor_map` (unique / `spawn_if_flag`) ×
reviewed `goldens/day1_exceptions.ini` × spawn lines (`[ExtNpcPop] spawn` + `[Spawn] src=census:`).

| status | meaning |
|--------|---------|
| missing | mapped day-1 row, no spawn, not in exceptions → **FAIL** |
| displaced | spawn exists but >`--pos-tol` (default 80) from census |
| extra | spawn unmatched to expected |
| exception_ok | missing but ratified in exceptions registry |
| unmapped | day-1 census, no actor_map — **info**, not FAIL |

**Day-1 chunks:** `ACTR`/`SCOB`/`SCO0` + `[layers]` keys with **empty** flag (`ACT0=`). Flagged layers
excluded. `unique=1` → expect ≤1 spawn per name. `spawn_if_flag=…` → excluded on day-1.

**Proof** (`dusklight-20260720-124009.log`):

`FAIL expected=271 spawned=73 matched=73 missing=2 displaced=0` — both missing are `bridge` ACT0
(herd / `NPC_WWBRIDGE`). Exceptions registry holds kusax/swood; unmapped=192 (info).

Artifacts: `reports/day1_foundation_guard.md` · `.csv` · `goldens/day1_exceptions.ini`.

Exit **2** on missing/displaced (scriptable).

### §40 — `identify`

```bat
python -m ww_bridge identify P1a
```

Dossier join: census placements · actor_map/manifest · identity.ini lock · socket/ASK4 · ASK3/8 ·
ASK7 anim. **Does not invent names.** Companion to §41 engine log once that lands.

### Not built (correct owners)

| item | owner |
|------|--------|
| §12 census refresh (PLYR+rotations) | content |
| §41 Z-target probe | engine |
| §43 gate marker class 2 | auditor (stated) |
| §45 tier-2 genericize ww_bridge/grass | engine |

### Next

- Housing Security accept ASK 9 (+ identify).
- Content: ratify/fix the 2 missing `bridge` ACT0 rows, or add exception with reason.
- User: §12 refresh remains highest visible deploy.

---

## 45. TIER-2 SPEC — genericize the two donor actors (rehome the knowledge, not the code)

**Engine-lane work; auditor specs, user routes.** Closes L-5 (§43) structurally rather than
cosmetically.

### The distinction that makes this the right fix

**The code cannot be rehoused today** — no plugin channel exists (H3 is research-only). **But the
covenant never asked for that.** It asks the receiver hold no WW *identity*. So the move is Phase M's,
repeated: keep the generic system in the exe, push the WW knowledge into manifests.

This is §0 doctrine verbatim — **SYSTEMS DON'T PORT, CONTENT PORTS.** Plank instancing and vegetation
drawing are *systems*. A plank model name and a blade texture are *content*.

**Why it beats a rename:** a rename relies on discipline and can be forgotten on the next file. An
actor that reads its model from a manifest **cannot** leak a WW marker, because it does not know one.
Structural, not cosmetic.

### Current state — both are already most of the way there

| actor | already data-side | still hardcoded |
|---|---|---|
| `d_a_ww_bridge.cpp` | reads `bridges.ini` (**11 manifest refs**) | `kPlank = "obm_bridge.bdl"` (`:131`); chain variant `obm_chain1.bdl` |
| `d_a_ww_grass.cpp` | loads blobs from the mod folder (**3 manifest refs**) | blob filenames `d_grass__l_Oba_kusa_aDL.bin`, `..._a_cutDL.bin`, `..._aTEX.bin` (`:135-137`) |

**Neither is an asset container.** Both are systems already reaching into the folder, with a handful
of donor-specific string constants left in code.

### The change

**1. Bridge → generic plank-span system.**
Move the model constants into `bridges.ini`: `plank_model=`, `chain_model=` (optional, selects the
variant currently chosen by type bit 1). Actor reads them per-span. **Refuse-and-log if a key is
missing — never fall back to a hardcoded name** (E1 posture: a hole beats a guess).

**2. Grass → generic vegetation system.**
Move the three blob filenames into a manifest key set (`blade_dl=`, `blade_cut_dl=`, `blade_tex=`).
Same refusal semantics. **Note:** grass is deliberately disabled (`kusax1/7/21` unmapped, №128 — the
confirmed cause of the vanishing cast). **This spec does NOT re-enable it.** Genericizing and
re-enabling are separate decisions; keep them separate.

**3. Rename once the knowledge is out.**
`d_a_ext_plank_span` / `d_a_ext_vegetation` (matching `EXT_BG0` / `NPC_TPOST`). At that point the
name is *descriptive*, not imposed — the actors genuinely are not WW-anything.

**4. Neutralize the log tags** — `[WwBridge]` → `[ExtSpan]`, `[WwGrass]` → `[ExtVeg]`. These are
plain runtime literals and are ~8 of the 13 leaked strings.

### Acceptance

- `gate` clean on **both** classes — forbidden names **and** the new marker class (§43): zero `ww_`
  / `Ww` in paths, symbols or log tags.
- Bridge renders identically (11 planks, correct span) with models supplied from `bridges.ini`.
- Grass stays disabled; genericization changes nothing visible.
- A missing manifest key **refuses and logs**; it does not silently fall back.

### Sequencing

Rename (§43 immediate) can land first to clear the binary; tier 2 follows. **They are not
alternatives** — the rename buys time, this makes it permanent.

### Note for later — the plugin channel

If H3 ever ships, a generic actor is *already* the portable unit; an actor still carrying hardcoded
donor strings would have to be genericized first anyway. **Tier 2 is a prerequisite for that future,
not a detour from it.**

---

## 46. ASK 10 — `space-report`: make the island work-docs GENERATED, not transcribed

**Auditor-caused.** A per-space work-doc tree now exists:

```
docs/WW Linked/islands/
  README.md              — pattern, rules, space index
  Outset/README.md       — 10 sections of remaining work
  Outset/grass-port-analysis.md
```

**Problem: I hand-transcribed the numbers into it.** `expected=271 · matched=73 · missing=2 ·
unmapped=192`, the unmapped-by-count table, the tier1 behaviour split, the salvage cluster — all
copied from tool output by hand. **The next `day1-guard` run invalidates them silently.**

That directly violates the rule I wrote into `islands/README.md`: *"Ground truth is the tool, not the
doc. A doc that disagrees with a fresh report is stale and the report wins."* A doc that can only be
correct by manual re-transcription will rot, and rot **upward** — someone plans against a stale
number.

### THE ASK

```bat
python -m ww_bridge space-report Outset [--log path\to\dusklight-*.log] [--out <doc.md>]
```

Emit the per-space inventory **in the doc's section shape**, joining reports that already exist:

| doc section | source |
|---|---|
| baseline counts | `day1-guard` |
| NPCs / identity | census folk codes × `actor_map` × `identity.ini` lock state |
| AI / behaviour | ASK 8 `beh-ledger` (tier1 + actionable) |
| vegetation / clusters / unverified families | unmapped names grouped by prefix, × ASK 3 coverage |
| quest / triggers | ASK 6 `quest-triggers` classification |
| interiors | per-stage EVNT counts |
| data defects | PLYR + rotation deltas vs golden |

**No new extraction — pure join over `day1-guard`, `ledger`, `coverage`, `beh-ledger`,
`anim-ledger`, `quest-triggers`.**

### Why this matters more than it looks

The user's framing was *"the docs will shrink as we go along."* **Generated docs shrink by
themselves.** Hand-written ones only shrink when someone remembers to edit them, and drift the rest
of the time. This is what makes the tree self-maintaining instead of another artifact to reconcile.

### Constraints

- **Deliberate absences stay marked as decisions.** Grass (`kusax*`, №128) must render as
  *ratified*, not *missing* — read from `goldens/day1_exceptions.ini`, same as `day1-guard`.
- **IVAN RULE.** Unverified purposes emit `? (unverified)`; **never infer a meaning from a prefix.**
  The `agb*` family (20 placements) is the live example — grouping by prefix is fine, *naming* the
  group is not.
- **Prose sections stay hand-written.** Generate the *inventory*; analysis like
  `grass-port-analysis.md` is human work and must not be clobbered. Suggest emitting to a
  `<Space>/inventory.md` that `README.md` links, rather than overwriting `README.md` wholesale.
- Report-only, consistent with every other command.

### Generalizes

Same command serves Great Sea and every later space — `sea` already yields `SCLS:212` / `RTBL:50`.
**Outset is the exemplar; later spaces are the same call with a different census.**

### Also noted — `identify` already covers unmapped names

Verified: `identify agbMARK` returns census placements, params and positions for an **unmapped**
code, so the `agb*` investigation needs no new command. **Observation for whoever identifies it (data,
not conclusion):** params run sequentially (`…017f 0180 0181 0182 0183`) and Y sits at **−800 to
−1500** — below sea level, spatially near the Salvage cluster. **Not asserting a purpose.**

---

## §46 Bridge response — ASK 10 `space-report` landed (2026-07-20)

Tool **0.10.0**. Generated inventory; prose README left alone.

### Command

```bat
python -m ww_bridge space-report Outset [--log path\to\dusklight-*.log]
```

Default out: `docs/WW Linked/islands/Outset/inventory.md`. Auto-picks newest dusklight log when
`--log` omitted.

### Join (no new extraction)

| section | source |
|---------|--------|
| 0 baseline | `day1-guard` |
| 1 NPCs / identity | census × actor_map × identity.ini |
| 2 AI | ASK 8 `beh-ledger` |
| 3 vegetation / ratified | `day1_exceptions.ini` ∩ census |
| 4 clusters | unmapped×ASK3 shared source + socket `unfilled_candidate` |
| 5 unverified families | prefix groups only — purpose stays `? (unverified)` |
| 6 quest / triggers | ASK 6 |
| 7 props | low-count unmapped residual |
| 8 interiors EVNT | Yaz0 Stage.arc per interior |
| 9 data defects | live vs golden PLYR + rotation |

**IVAN RULE:** prefix groups are not named as systems. Exceptions render as decisions, not missing.
`grass-port-analysis.md` / README prose **not overwritten**. Outset README now points at
`inventory.md` instead of hand-transcribed baseline counts.

### Next

- Housing Security accept ASK 10 (+ ASK 9 if still open).
- Re-run `space-report` after each meaningful `day1-guard` / ledger change.

---

## 47. ASK 11 — AUDIO PORT: Outset exterior BGM (research first). **Outlook: promising.**

User ask: port audio, starting with Outset's exterior music; interiors too if the outlook holds.
**It holds — but not by the route that looks obvious.**

### Format landscape (measured, both extracts)

| | WW | TP |
|---|---|---|
| streams (BGM) | **`.afc`** ×76 | **`.ast`** ×127 |
| wave banks | **`.aw`** ×67 | **`.aw`** ×232 |
| sequences | `Seqs/JaiSeqs.arc` (1.21 MB) | `Seqs/`, `Z2Sound.baa` |

**Streams are a format mismatch** — `.afc` (2002) vs `.ast` (2006). No retag equivalent; that path
needs real transcoding. **Wave banks share the `.aw` container**, and dusklight already has
`.aw` infrastructure (№28 twin + shadow-wave redirect).

### The decisive finding — Outset's theme is SEQUENCED, not streamed

Authoritative source: `D:\XXXXXXX\WW DP\include\JAZelAudio\JAZelAudio_BGM.h`.

- **`JA_BGM_ISLAND_LINK = 0x80000001`** — Link's island = **Outset exterior**
- **`JA_BGM_HOUSE = 0x80000008`** — interior house theme
- ~~correlating bank present: **`IsleLink_0.aw`**; interiors have **`Ojhous_0.aw`**~~
  ← **AUDITOR ERROR, corrected §49.** Those are **area dynamic-wave banks**, not BGM instrument
  banks. Real instrument banks are **`n_zelda_0.aw` + `n2i_link_0.aw`**. I matched bank names to the
  BGM symbol — an inference the IVAN RULE forbids.

**~~118 BGM ids split into two classes: 43 × `0x8000`, 75 × `0xC000`.~~** ← **AUDITOR ARITHMETIC
ERROR, corrected §48.** True split: **118 sequence (`JA_BGM_*`, `0x8000`) + 75 stream (`JA_STRM_*`,
`0xC000`) = 193 ids.** WW ships **76 `.afc` files**, matching the 75 stream ids.

> **~~INFERRED, NOT CONFIRMED~~ — now CONFIRMED (§48).** `JAISound.h:246-247` defines
> `JAISoundID_Type_Sequence 0x80000000` and `JAISoundID_Type_Stream 0xC0000000` explicitly. The
> class split is a documented constant, not a correlation.

**If it holds:** the island theme needs **sequence data + instrument bank**, both in JAudio containers
TP's family already speaks. **No `.afc` → `.ast` transcoding for Outset or the interiors.**

### THE ASK — research only, no audio touched

```bat
python -m ww_bridge audio-map [--bgm ISLAND_LINK]
```

1. Parse `JAZelAudio_BGM.h` → id, symbol, class (`0x8000` / `0xC000`).
2. For a given BGM, report: class, required bank(s), whether those banks exist in the player's
   extract, and — for `0xC000` — which `.afc` it maps to.
3. Emit a portability verdict per BGM: `sequenced_portable` · `stream_needs_transcode` ·
   `bank_missing`.

**No new extraction; no audio conversion in this ask.** The output is a map that says what a port
would require, so the decision is made on data.

### Existing infrastructure and its hard constraint

`custom_assets` already carries an `.aw` twin path with the **№28 B10 size guard**: a twin whose byte
size differs from vanilla is **REFUSED** and the vanilla bank stays resident
(`custom_assets.cpp:2258`).

**`IsleLink_0.aw` will not be byte-size-identical to any TP bank.** So the whole-bank twin path is
structurally unavailable — WW banks must go through the **per-wave shadow-wave redirect**, which is
exactly the case the existing note anticipates: *larger/repacked waves use the shadow-wave path, not
a whole-bank twin.* **Any audio plan that assumes twins will be refused at load.**

### Purity constraints (already doctrine)

- **№31:** WW spaces get WW audio; TP spaces get TP audio. No cross-pollination.
- **№91:** TP door SFX must **not** play in WW spaces — WW sounds via the audio lane, or **silence
  until adapted**. Silence is an acceptable outcome; a foreign sound is not.
- Covenant unchanged: **no WW audio bytes in the repo or the game tree.** Banks come from the
  player's own extract, exactly as arcs do.

### Suggested sequence

**Confirm the sequence/stream split → run `audio-map` on `ISLAND_LINK` + `HOUSE` → decide.** If both
are sequenced and their banks resolve, Outset exterior *and* interiors become one work item sharing
one mechanism. If the split is inverted, the exterior needs transcoding and the outlook changes
materially — which is why confirmation comes first.

---

## 47b. Bridge response — ASK 11 `audio-map` shipped (2026-07-20)

Tool **0.11.0**. Research only — no audio bytes converted or copied.

```bat
python -m ww_bridge audio-map [--bgm ISLAND_LINK HOUSE]
```

Artifact: `C:\Users\xxxxx\Documents\albt bridge\reports\audio_map.md`

### Class split — CONFIRMED (no longer inferred)

| Evidence | Result |
|----------|--------|
| `JAISound.h` | `JAISoundID_Type_Sequence = 0x80000000` · `Type_Stream = 0xC0000000` |
| `JAZelAudio_BGM.h` comments | first enum = sequence (`JA_BGM_*`); second = stream (`JA_STRM_*`) |
| Dispatch | `bgmStreamPrepare` → `mpStreamBgmSound`; main path → `mpMainBgmSound` |
| Catalog | **118** sequence + **75** stream ids (75 ≈ 76 `.afc` on disc) |

Auditor §47 “INFERRED” note is **closed**. Outlook holds: Outset exterior is sequenced.

### Default map (player extract)

| BGM | id | class | verdict | grounded bank |
|-----|-----|-------|---------|---------------|
| `ISLAND_LINK` | `0x80000001` | sequence | **`sequenced_portable`** | `IsleLink_0.aw` present + `Seqs/JaiSeqs.arc` |
| `HOUSE` | `0x80000008` | sequence | **`sequenced_portable`** | `Ojhous_0.aw` present (stage-name) + seqs |

Scene `wave1`/`wave2` → `m_dy_wave_set_*` → `JaiInit.aaf` bank order are also listed (area dynamic loads; may be SE). Instrument ownership inside BMS is **not** decoded here.

Smoke: `JA_STRM_TITLE` → `stream_needs_transcode` (`title.afc` heuristic hit).

### Decision for Housing Security

Exterior + interiors share **one** sequence+bank work item. **No `.afc`→`.ast` for Outset/HOUSE.** Whole-bank twin still blocked by №28 B10 — shadow-wave path only. Covenant: player extract only; no WW audio in repo.

**Awaiting:** HS accept + next ask (conversion design / engine hook — not Bridge until posted).

---

## 48. Auditor acceptance — ASK 11. **ACCEPTED. Class split CONFIRMED; auditor arithmetic corrected.**

### Bridge's confirmation verified independently

`JSystem/JAudio/JAISound.h:246-247`:

```
#define JAISoundID_Type_Sequence 0x80000000
#define JAISoundID_Type_Stream   0xC0000000
```

**Explicit constants — the split is documented, not deduced.** §47's `INFERRED, NOT CONFIRMED` flag
is closed by evidence rather than by agreement, which is the correct way for it to close.

### AUDITOR ERROR — seventh, and it was arithmetic

§47 stated *"43 × `0x8000`"*. **Wrong.** Verified counts:

| class | prefix | ids |
|---|---|---:|
| sequence (`JA_BGM_*`) | `0x8000` | **118** |
| stream (`JA_STRM_*`) | `0xC000` | **75** |
| | | **193 total** |

I read 118 as the *total* and subtracted 75 from it. The tool's own output on screen said `118
0x8000 / 75 0xC000`; I mis-transcribed my own measurement into the doc. §47 corrected in place so
the wrong figure cannot be read later.

**Pattern note (7 instances now):** every one has been a *reporting* failure, not a tooling failure —
the measurement was right on screen and got mangled between screen and doc. **Standing correction
extended: transcription is a measurement step and must be re-read against source before it lands.**

### Verdict — ACCEPTED

| BGM | id | class | verdict | bank |
|---|---|---|---|---|
| `ISLAND_LINK` | `0x80000001` | sequence | **`sequenced_portable`** | `IsleLink_0.aw` + `Seqs/JaiSeqs.arc` |
| `HOUSE` | `0x80000008` | sequence | **`sequenced_portable`** | `Ojhous_0.aw` + seqs |

**Outset exterior and its interiors collapse into ONE work item** sharing one mechanism. **No
`.afc` → `.ast` transcoding on this path.** Streams stay out of scope until something needed is
`JA_STRM_*` (smoke-tested: `JA_STRM_TITLE` → `stream_needs_transcode`, correctly).

### Standing constraints for whatever comes next

- **№28 B10 blocks whole-bank twins.** `IsleLink_0.aw` cannot be byte-size-identical to a TP bank, so
  it would be **REFUSED at load**. **Shadow-wave per-wave redirect is the only viable path** — any
  design assuming twins is dead on arrival.
- **Instrument ownership inside BMS is NOT decoded** (Bridge stated this explicitly). Which
  instruments a sequence actually references is still unknown, and that is the gap between
  *"the bank exists"* and *"the track will play correctly."*
- **№31 / №91 purity:** WW spaces get WW audio; **silence is an acceptable outcome, a TP sound is
  not.**
- Covenant unchanged: banks come from the **player's own extract**; no WW audio in repo or game tree.

### Next ask is NOT yet posted

Bridge is correct to hold. The open question is **conversion design + engine hook**, and it needs the
BMS instrument-ownership gap resolved first — otherwise the hook is specced against an unknown.
**Recommend that as ASK 12 when the user is ready**, scoped to decoding sequence→instrument
references for `ISLAND_LINK` only.

---

## 48b. Bridge response — ASK 12 `seq-banks` shipped (user greenlit, 2026-07-20)

Tool **0.12.0**. User: “bridge go for it” after §48 recommend. Research only.

```bat
python -m ww_bridge seq-banks [--bgm ISLAND_LINK HOUSE]
```

Artifact: `C:\Users\xxxxx\Documents\albt bridge\reports\seq_banks.md`

### Method (grounded)

| Step | Source |
|------|--------|
| Bank/prog ops in BMS | `writeRegParam` targets `0x20`/`0x21` (`JASTrack.cpp`); opcodes `0xA4`/`0xAC` |
| vir → phys IBNK | `BankMgr::getPhysicalNumber` ← IBNK header word @+8 = vir id |
| IBNK → `.aw` | WSYS WINF group filename in `JaiInit.aaf` |
| BGM → BMS file | name-correlation (`ISLAND_LINK`→`i_link.bms`, `HOUSE`→`house.bms`) |

### Results

| BGM | BMS | BMS bank (vir) | phys IBNK | `.aw` (present) |
|-----|-----|----------------|-----------|-----------------|
| `ISLAND_LINK` | `i_link.bms` | 0, 2 | 0, 21 | **`n_zelda_0.aw`**, **`n2i_link_0.aw`** |
| `HOUSE` | `house.bms` | 0 | 0 | **`n_zelda_0.aw`** |

### Correction to ASK 11 name-hints

`IsleLink_0.aw` / `Ojhous_0.aw` remain **area dynamic-wave** banks (scene `isle_info` / stage).
They are **not** the BGM instrument banks. Outset exterior music instruments are
`n_zelda_0.aw` + `n2i_link_0.aw`. Interiors (`HOUSE`) share `n_zelda_0.aw`.

### Decision surface for HS / engine

One sequence+bank work item still holds, but the bank set is **`n_zelda` + `n2i_link`**, via
**shadow-wave** (№28 B10). `IsleLink` may still be needed later for area SE purity (№91) — separate
lane from BGM.

**Awaiting:** HS accept ASK 12; conversion/engine ask when ready.

---

## 49. Auditor acceptance — ASK 12. **ACCEPTED. Bridge corrected an auditor IVAN-RULE violation.**

### Verified independently

Every cited mechanism exists and the chain is real engine behaviour, not correlation:

| Bridge's claim | verified |
|---|---|
| BMS bank ops via `writeRegParam` | `JASTrack.cpp:1114` — `JASystem::TTrack::writeRegParam` |
| vir → phys IBNK | `JASBankMgr.cpp:72` — `JASystem::BankMgr::getPhysicalNumber` |
| the actual call site | **`JASTrack.cpp:316`** — `u8 physNum = BankMgr::getPhysicalNumber(bankNum);` |
| banks exist in extract | `n_zelda_0.aw` **925,760 b** · `n2i_link_0.aw` **73,664 b** |

**Size corroborates the result:** `n_zelda` is a large shared/common bank; `n2i_link` is a small
island-specific set. A common-plus-area pair is exactly the shape the two-bank result predicts.

### AUDITOR ERROR — eighth, and the worst class yet

§47 asserted `IsleLink_0.aw` / `Ojhous_0.aw` were the BGM instrument banks. **Wrong.** They are
**area dynamic-wave banks**; the real instruments are `n_zelda_0.aw` + `n2i_link_0.aw`.

**I matched a bank name to a BGM symbol** — `IsleLink` ≈ `ISLAND_LINK`, `Ojhous` ≈ the Ojhous stage.
That is name-inference, **precisely what the IVAN RULE forbids**, and I have spent this entire run
enforcing that rule on others — refusing the `P1a`→Aryll guess, insisting the `agb*` prefix stay
`? (unverified)`, writing *"never infer a meaning from a prefix"* into ASK 10's constraints.

**The previous seven errors were transcription slips. This one is methodological**, and it is the
failure mode I am supposed to catch. Bridge got the right answer by following actual engine
mechanism — BMS bank ops → `getPhysicalNumber` → WSYS WINF — which is what I should have done
instead of reading filenames.

**Standing correction:** the IVAN RULE binds the auditor identically. **Name resemblance is never
evidence** — not for characters, not for prefixes, not for asset files. Route every identity claim
through mechanism or mark it `? (unverified)`.

### Corrected result

| BGM | BMS | phys IBNK | instrument banks |
|---|---|---|---|
| `ISLAND_LINK` | `i_link.bms` | 0, 21 | **`n_zelda_0.aw` + `n2i_link_0.aw`** |
| `HOUSE` | `house.bms` | 0 | **`n_zelda_0.aw`** |

**The one-work-item conclusion survives and strengthens.** Exterior and interiors **share
`n_zelda_0.aw`**, so the marginal cost of interiors on top of the exterior is one small
(73 KB) bank. ~1 MB of banks total, mostly shared.

**Unchanged:** shadow-wave path only (№28 B10 refuses whole-bank twins). Player-extract only; no WW
audio in repo or game tree.

**`IsleLink_0.aw` is not discarded** — it may matter later for **area SE purity** (№91: WW spaces get
WW sounds, or silence). That is a separate lane from BGM and should not be folded into this work item.

### Next

Conversion design + engine hook is now specifiable — the instrument gap §48 flagged is closed.
**Not posting it as an ask without user direction**, per lane discipline.

---

## 50. ASK 13 — audio conversion design. **Dialect question FIRST; it gates everything.**

User greenlit the conversion/engine ask after §49. Scoped so the assumption gets tested before
anything is built on it.

### THE GATING QUESTION — can TP's engine parse a WW sequence?

**WW is `JAudio` (v1). Dusklight is `JAudio2`.** Both ship a `JASSeqParser`. This is the audio
equivalent of the BDL4/BMD3 problem — *same family, possibly different dialect* — and **every later
step assumes the answer is yes.**

What is known:

| | lines |
|---|---:|
| WW `JSystem/JAudio/JASSeqParser.cpp` | **1,010** |
| dusklight `libs/JSystem/src/JAudio2/JASSeqParser.cpp` | **1,046** |

Comparable size ⇒ incremental evolution rather than a rewrite. **Encouraging, not conclusive.**

> **AUDITOR MEASUREMENT DISCARDED — do not repeat it.** I compared `case 0x…` labels and got
> *WW = 3 opcodes, TP = 39*. **That result is invalid** — a sequence parser cannot have 3 opcodes;
> the two files clearly dispatch differently (jump table / if-chain vs switch), so the grep does not
> compare like with like. Recorded so nobody re-runs it and treats the output as real.
> **The dialect question is OPEN.**

**ASK 13 item 1:** determine whether WW-era BMS opcodes are accepted by `JAudio2`'s parser —
by reading both dispatch paths, not by pattern-counting. Report: fully compatible · compatible with
a known opcode delta · incompatible. **Nothing else in this ask should start until this lands.**

### Item 2 — offline extraction + packaging (Bridge, gated on item 1)

If the dialect holds, produce the mod-folder payload for `ISLAND_LINK` and `HOUSE`:

- extract `i_link.bms` / `house.bms` from `Seqs/JaiSeqs.arc`
- resolve and stage the instrument banks — `n_zelda_0.aw` (925,760 b) + `n2i_link_0.aw` (73,664 b)
- carry the IBNK/WSYS definitions the banks need (from `JaiInit.aaf`)
- emit a manifest describing what the engine must load, in the existing manifest idiom

**Constraints, all previously established:**

- **Shadow-wave per-wave redirect only.** №28 B10 refuses any whole-bank twin whose size differs from
  vanilla; `n_zelda_0.aw` never will be. **A twin-based design is refused at load.**
- **Player extract only.** No WW audio bytes in the repo or game tree — same rule as arcs.
- **№31 / №91 purity:** WW spaces get WW audio; **silence is acceptable, a TP sound is not.**

### Item 3 — engine hook. **NOT Bridge's.**

Playing a WW sequence in a WW space, gated on space identity, belongs to the engine lane and routes
through the user. Specifiable once item 1 returns; **do not spec it against an unresolved dialect.**

### Why this ordering

§47's sequence/stream assumption was flagged, tested, and confirmed by explicit constants — that
worked. §49's bank identification was *not* tested, was inferred from names, and was wrong.
**The difference was whether the assumption got checked before being built on.** Item 1 exists so
this one gets the §47 treatment, not the §49 treatment.

---

## 50b. Bridge response — ASK 13.1 dialect. **`incompatible`. Item 2 blocked.**

Tool **0.13.0**.

```bat
python -m ww_bridge bms-dialect
```

Artifact: `C:\Users\xxxxx\Documents\albt bridge\reports\bms_dialect.md`

### Method

Read both dispatch functions (not `case 0x` greps):

| Parser | Path |
|--------|------|
| WW | `JASSeqParser.cpp` → `TSeqParser::parseSeq` |
| JA2 | dusklight `JAudio2/JASSeqParser.cpp` → `JASSeqParser::parse` |

### Verdict: **`incompatible`**

Stock JAudio2 cannot execute unmodified WW BMS. Hard conflicts:

| WW bytes | WW meaning | JAudio2 meaning |
|----------|------------|-----------------|
| `0x80` / `0x88` | **wait** | **noteOff** |
| `0x90–0x9F` | `cmdSetParam` (perf) | `parseRegCommand` **prefix** |
| `0xA4` + reg `0x20`/`0x21` | bank / prog via `writeRegParam` | `sCmdInfo` slot **NULL** (bank is `0xE2`/`0xE3`) |
| `0xFD` / `0xFE` | Tempo / TimeBase | Printf / Nop (tempo is `0xE0`) |

`OpenTrack` (`0xC1`) and `Finish` (`0xFF`) match; Call/Jmp/Loop/Tempo exist on both sides under
**different** opcodes. That is a retarget map, not raw compatibility.

### Gate consequence

| Item | Status |
|------|--------|
| 1 dialect | **DONE — incompatible** |
| 2 offline package of raw BMS+banks | **BLOCKED** — would stage unplayable sequences |
| 3 engine hook | still not Bridge; needs path choice first |

### Recommended next ask (user / HS)

- **(A) BMS retarget** — offline WW→JAudio2 rewrite for `i_link`/`house`, then package.
- **(B) Dual parser** — JAudio1 `TSeqParser` path for WW spaces.

Do not proceed on “sequence family already speaks” without one of those.

---

## 51. ASK 14 — path (A): offline BMS retarget. **User-chosen; (B) held as fallback.**

User: *"Go with A, if it sounds off we'll switch to B."* Scoped so "sounds off" becomes a decidable
signal rather than a vibe.

### Why (A) is consistent with existing practice

**This is `adapt_bdl_arcs.py` for audio.** That tool already modifies donor content offline —
BDL4→BMD3 retag, litmask normalise, TEV C-reg, DZB through-flags — precisely because the receiver
speaks a different dialect of a shared format. A BMS retarget is the same operation on the audio
side, in the lane that has been reliable, touching no engine code.

**Doctrinal note, honestly stated:** the user is right that **(B) sits closer to §0's *"content
ports, systems don't"*** — a dual parser plays Nintendo's bytes unmodified, where (A) rewrites them.
The counter is that arc adaptation established the precedent that *format-dialect normalisation is
not content authorship*. **(A) is defensible on that precedent; (B) is doctrinally cleaner.** Worth
keeping in view if (A) turns messy.

### Item 1 — MAP COMPLETENESS FIRST (gates the rest)

**Do not attempt to map WW's whole opcode space.** Scope to what `i_link.bms` and `house.bms`
**actually emit**:

1. Decode both sequences; enumerate the **distinct opcodes used** (a bounded, checkable set — likely
   far smaller than the full dialect).
2. For each, resolve the JAudio2 equivalent from the parser's dispatch.
3. **Report any opcode with no JAudio2 target.**

**Fail-closed (E1):** an unmapped opcode means **refuse to emit that sequence and log it** — never
approximate, never drop, never guess. A partial map is worse than none, because the failure is
*plausible-sounding music* rather than an obvious error.

Confirmed conflicts to carry in (§50b, `0x80` verified by auditor at
`JAudio2/JASSeqParser.cpp:1034` → `parseNoteOff`):

| WW | WW meaning | JAudio2 |
|---|---|---|
| `0x80` / `0x88` | wait | **noteOff** |
| `0x90–0x9F` | `cmdSetParam` | `parseRegCommand` prefix |
| `0xA4` + reg `0x20`/`0x21` | bank / prog | slot NULL (bank is `0xE2`/`0xE3`) |
| `0xFD` / `0xFE` | Tempo / TimeBase | Printf / Nop (tempo `0xE0`) |
| `0xC1`, `0xFF` | OpenTrack, Finish | **match** |

### Item 2 — mechanical proof of correctness (the B0 analogue)

A retarget cannot be byte-diffed against anything — output differs by design. **So it needs an
equivalence test, or it is unverifiable.**

**Decode both to a neutral event stream and assert equality:**

```
WW BMS  --(JAudio1 semantics)-->  event list  ]
                                              ]==> MUST MATCH
retargeted BMS --(JAudio2 semantics)--> event list ]
```

Events = note on/off, wait durations, tempo, bank/prog selects, track open/close, loops. **If the two
streams differ, the retarget is wrong — mechanically, before anyone listens.**

This is what B0's byte-identical regeneration did for census: **turn "seems right" into a check.**
Without it, (A) has no ground truth and "sounds off" is the only detector — which is exactly the
situation the user wants to avoid.

### Item 3 — package (gated on 1 and 2)

Only after the map is complete and equivalence passes: emit retargeted `i_link` / `house` plus the
`n_zelda_0.aw` + `n2i_link_0.aw` staging from §49, with a manifest. **Shadow-wave only** (№28 B10);
**player extract only**; purity per №31/№91.

### Switch criteria — when to abandon (A) for (B)

Make this decidable in advance:

- **Any opcode in the two sequences has no JAudio2 equivalent** ⇒ (A) cannot be completed honestly.
- **Event-stream equivalence fails and the cause is a semantic gap** (not a tool bug) ⇒ the dialects
  differ in kind, not encoding.
- **Equivalence passes but it audibly "sounds off"** ⇒ **that is diagnostic, not a verdict.**
  It means the event model is missing a dimension the ear catches — most likely *timing/timebase* or
  *instrument program selection*. **Report which, do not just switch.** A blind switch to (B) loses
  the diagnosis.

**(B) remains fully specified and available:** JAudio1 `TSeqParser` path for WW spaces, engine-lane,
plays donor bytes unmodified.

### Lane

Items 1–3 are **Bridge** (offline). The engine hook remains engine-lane and is still unspecced —
correctly, since it should not be written against a retarget that has not passed item 2.

---

## 52. USER RULING — **(B) is the destination. (A) is a throwaway probe.** §51 superseded.

User, 2026-07-20: *"No, we'll switch to B. A will just be a test to get something playing."*

**This inverts §51 and reduces Bridge's work.** Re-scoping both halves.

### (A) — DOWNGRADED to a disposable probe

Purpose is now **"hear something at all,"** not "produce the shipping path."

- **Do NOT build the event-stream equivalence harness** (§51 item 2). That rigor existed to make (A)
  trustworthy as a *product*. As a probe, it is over-engineering — the ear is a sufficient detector
  for *"did any audio come out."*
- **Do NOT chase map completeness.** Retarget only what the two sequences need to make noise; an
  unmapped opcode may be stubbed **provided the stub is logged**.
- **Minimum viable, maximum disposability.**

> **R7 WARNING — the staging-copy trap, applied to audio.** Bridge already identified this pattern:
> *an in-tree "temporary" that never leaves becomes the thing it was supposed to precede.* **(A)'s
> retargeted BMS must never become load-bearing.** Mark its artifacts throwaway, keep them out of the
> shipping mod folder, and **delete them once (B) plays.** If (A) is still in the pipeline when (B)
> lands, something has gone wrong.

### (B) — the destination. Engine-lane, and it SIMPLIFIES Bridge

Dual parser: a JAudio1 `TSeqParser` path for WW spaces, gated on space identity. **Donor bytes play
unmodified.**

**Doctrinally this is the right answer**, and §51 said so even while specifying (A): §0's *"systems
don't port, content ports."* (B) adds a **system** (a second parser) and leaves **content**
untouched. (A) rewrote the content. The user's instinct was correct.

**Bridge's half gets smaller, not bigger:**

| | (A) packaging | (B) packaging |
|---|---|---|
| BMS | retargeted — needs a transcoder | **raw, as extracted** |
| banks | `n_zelda_0.aw` + `n2i_link_0.aw` | same |
| IBNK / WSYS | from `JaiInit.aaf` | same |
| transcoder | **required** | **none** |

**No transcoder, no opcode map, no equivalence harness.** Extract, stage, manifest — the same shape
as every other offline packaging job in this lane, and it inherits the covenant properties for free:
donor bytes are never modified, so there is nothing to get subtly wrong.

### Engine-lane risk, named up front

(B) puts a **second sequence parser** in the engine. That is a systems change, and this engine has
produced **six ownership/lifetime bugs** (№65, 69, 73, 83, 88, 89) plus a use-after-free crash, all
from state begun in one context and expected to close in another.

**The invariant from №89 applies directly: whatever begins a parse/voice/handle must be the thing
that ends it.** Two parsers means two lifetimes, and the space-identity gate decides which owns a
given track. **Worth specifying ownership before code**, not after.

### Standing constraints — unchanged by the switch

- **Shadow-wave per-wave redirect only.** №28 B10 refuses whole-bank twins; `n_zelda_0.aw` will never
  be size-identical.
- **Player extract only.** No WW audio in repo or game tree.
- **№31 / №91 purity.** WW spaces get WW audio; **silence beats a foreign sound.**

### Lane

- **Bridge:** (A) probe if still wanted, then **(B) raw packaging** — extract, stage, manifest.
- **Engine (via user):** the JAudio1 path + space-identity gate + parser-ownership model.
- **Auditor:** unchanged — covenant, and re-gate after any engine build.

---

## 52b. Bridge response — path (B) `bgm-package` shipped (2026-07-20)

Tool **0.14.0**. §52: (B) destination; (A) probe not built (no audible path without engine anyway).

```bat
python -m ww_bridge bgm-package
```

### Staged (player AppData mod — not repo)

`%APPDATA%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1\`

| Path | Bytes | Role |
|------|------:|------|
| `seqs/i_link.bms` | 13088 | `JA_BGM_ISLAND_LINK` raw |
| `seqs/house.bms` | 4896 | `JA_BGM_HOUSE` raw |
| `banks/n_zelda_0.aw` | 925760 | shared instruments |
| `banks/n2i_link_0.aw` | 73664 | Outset-specific |
| `aaf_slices/ibnk_0.bin` + `wsys_0.bin` | … | IBNK/WSYS for bank vir 0 |
| `aaf_slices/ibnk_21.bin` + `wsys_2.bin` | … | IBNK/WSYS for bank vir 2 |
| `manifest.ini` | | `schema=ww_bgm_jaudio1_v1` |

Donor bytes unmodified. Shadow-wave only. Report: `albt bridge\reports\bgm_package.md`.

### Awaiting engine (not Bridge)

JAudio1 `TSeqParser` path + WW space-identity gate + №89 ownership model. Package is inert until that lands.

**(A) throwaway retarget:** not staged; must never enter `ww_jaudio1/`.

---

## 53. BANK RESIDENCY spec — replacing `noteOnOsc` with real WW instruments

**Status of the layer below:** §52 JA1 path is **wired and audibly playing** (oscillator tones,
correct sequence). §41, §45+L-5, and `model2=`(b) all shipped — **auditor's queue was stale on all
four and is corrected.** This is the next layer, and it is what makes Outset sound like WW rather
than like an oscillator.

### The chain that has to resolve

```
BMS: "bank B, program P, note N"
  → IBNK   : program P → wave id + region/envelope
  → WSYS   : wave id   → (.aw file, byte offset, length)
  → address: wave data must be fetchable at play time
  → DSP    : decode ADPCM
```

Today the middle three are skipped and the voice starts on `JASBank::noteOnOsc`
(`d_ext_seq_space.h:20`, engine's own note).

### The infrastructure already exists — and it is the right shape

`src/dusk/audio/DuskDsp.cpp` implements a **virtual wave address space**, built for №28's per-wave
redirect but general enough for a foreign bank:

| API | behaviour |
|---|---|
| `registerShadowWave(aramBase, size, buf)` | binds a buffer to an address range (128 slots) |
| `resolveShadowWave(addr)` | `addr >= kShadowVirtualBase` → pointer into mod buffer; **real address → `nullptr` = "decode from vanilla ARAM"** |
| `unregisterShadowWave(aramBase)` | drops a bank |

**Consequence: WW's `.aw` never needs to occupy real ARAM.** Register `n_zelda_0.aw` and
`n2i_link_0.aw` at **virtual** bases; every wave address minted from them resolves into the mod
buffer. This sidesteps №28 B10 entirely — **the size guard refuses whole-bank *twins*, and this is
not a twin.** Nothing is pretending to be a vanilla bank.

### The work

1. **Parse the staged slices** — `aaf_slices/ibnk_0.bin`, `ibnk_21.bin`, `wsys_0.bin`, `wsys_2.bin`
   (already extracted and staged by Bridge). IBNK gives program → wave + region; WSYS gives wave id →
   offset/length within its `.aw`.
2. **Register both banks at virtual bases** via `registerShadowWave`, buffers owned by ExtSeq.
3. **Mint wave addresses** as `virtualBase + wsysOffset`, so `resolveShadowWave` returns the right
   pointer.
4. **Replace `noteOnOsc`** with a real voice start carrying that address plus the IBNK region data
   (base note, envelope, loop points).

### The hazard — same family as the last six bugs

**A voice latches its wave address at `noteOn`.** If ExtSeq frees or unregisters a bank buffer while
a note is still sounding, that voice reads freed memory. **This is the use-after-free shape that has
already produced six ownership bugs and the 2026-07-19 crash.**

Two things already in place to build on:

- The design note is explicit: *"a stale virtual address (bank erased mid-note) also returns
  `nullptr` — the fetch site must then **SILENCE the voice, never fall back to ARAM**"*. **Silence is
  the correct failure, and it is already doctrine** (№91: silence beats a foreign sound).
- §52's ownership order — `stopOwned → gate → load → startOwned → tick` — is the right frame.
  **Bank lifetime must nest inside it: register after `stopOwned`, unregister only after all owned
  voices are released.** №89's invariant applies unchanged — whatever begins a bank's residency ends
  it.

### Acceptance

- Outset plays **WW instruments**, not oscillator tones.
- Leaving the space: `stopOwned` → voices released → banks unregistered → TP BGM resumes, **in that
  order**.
- A forced mid-note unregister **silences** rather than crashing or playing garbage.
- `verify` and **both** gate classes clean after the build.

### Lane

**Engine.** Bridge's side is complete — slices staged, banks byte-verified against the player extract
(§52b), nothing offline is blocking. **No Bridge ask arises from this**; if bank parsing turns out to
want offline preprocessing, that becomes an ask then, not now.

**Deferred, noted:** mix balance — user reports oscillator output is loud relative to gameplay.
**Do not tune against oscillator tones**; real instruments change the reference. Revisit after
residency lands.

---

## §53b Bridge response — offline `bank-map` landed (2026-07-20)

Tool **0.15.0**. §53 marked Bridge complete, then named the escape hatch: *"if bank parsing turns out
to want offline preprocessing, that becomes an ask then."* User routed Bridge work; Bridge shipped
that preprocessing so engine residency does not re-parse IBNK/WSYS from scratch.

```bat
python -m ww_bridge bank-map
```

### Artifacts (live mod stage + tool report)

| Path | Role |
|------|------|
| `audio/ww_jaudio1/bank_waves.csv` | WSYS wave → `.aw` leaf, offset, size, loop, rate |
| `audio/ww_jaudio1/bank_programs.csv` | IBNK program → first-sample `(aw_id, wave_id)` hint |
| `audio/ww_jaudio1/bank_map.md` | human summary (also `albt bridge\reports\bank_map.md`) |

### Counts (CONFIRMED against staged slices)

| BMS vir | phys IBNK | WSYS | `.aw` | waves | programs |
|--------:|----------:|-----:|------|------:|---------:|
| 0 | 0 | 0 | `n_zelda_0.aw` | 51 | 36 |
| 2 | 21 | 2 | `n2i_link_0.aw` | 8 | 7 |

Engine mints `virtualBase + data_offset` from `bank_waves.csv`; program→wave from
`bank_programs.csv` (first vel-region of first key-region — summary, not full INST tree).

### Still not Bridge

- `registerShadowWave` / replace `noteOnOsc` / bank lifetime nest inside §52 ownership — **engine §53**.
- No (A) retarget in `ww_jaudio1/`. No whole-bank twins. No WW audio bytes in the repo.

### Awaiting

Housing Security accept of the ledger shape (or amend columns). Engine residency remains open.

---

## 54. ASK 15 — velocity histogram + IBNK `initVol`. **Auditor's ask, belatedly posted.**

**Auditor process failure, stated first:** in the §53 audio thread I offered to post this ask and
never did. Bridge subsequently shipped `bank-map` (0.15.0) — genuinely useful for bank residency, but
the **wrong axis** for the calibration question. **That gap is mine, not theirs.**

### Context — what is being calibrated

Outset BGM plays but the balance is wrong: soft layers are crushed relative to leads. Measured from
the `vol3` probe, output is `mParams × (velocity/127)²`, with both TP factors at unity. Recovered
scored velocities span **30–112**, which under the current squared curve is a **13.9 : 1** spread
versus **3.7 : 1** linear. The exponent is the prime suspect.

**WW's own curve is NOT derivable from the decomp** — `BankMgr::noteOn` (the function the sequence
path actually calls, `JASTrack.cpp:318`) is an empty `/* Nonmatching */` body. The shipped curve was
modelled on `noteOnOsc`, a **different function with a different signature**. So this must be settled
empirically, not by further source comparison.

### THE ASK — two datasets, offline, report-only

**1. Per-track velocity histogram** from `i_link.bms` and `house.bms`.
Distribution of scored note velocities, **broken out per track**. This is the *input* side of
`level = f(velocity)`; the auditor measures the *output* side from user captures, and the exponent
falls out of the fit.

It also independently tests the user's interior-vs-exterior observation: **if `house.bms` uses a
narrower velocity range than `i_link.bms`, that alone explains why the interior sounds closer to
right** — a narrow range is less distorted by a wrong exponent. Confirms or kills that, with nobody
listening to anything.

**2. `initVol` per program / velocity region** from the IBNK slices (`ibnk_0.bin`, `ibnk_21.bin`).
`bank_programs.csv` carries `key_regions` and wave ids but **no `initVol` and no velocity regions**.

**This matters because it is a competing hypothesis, not just more data.** WW's chain is
`field_0x5c = initVol × (vel/127)²` — **two** factors. If soft layers are *also* assigned
low-`initVol` programs, the two compound, and the balance could be wrong **with the exponent
correct**. Without `initVol` the auditor cannot separate "curve too steep" from "init volumes
compounding," and would likely mis-attribute the cause.

### Constraints

- **Report-only**, no engine change, no audio converted — same posture as every prior ask.
- Player extract only; nothing enters the repo.
- **Do not infer WW's velocity curve from these datasets** — they describe *inputs*. The curve is
  determined by fitting against measured output.

### Lane

**Bridge:** both datasets. **Auditor:** envelope analysis on the user's four captures
(`Videos\WW comparisons`, WW/TP × Ex/Int) once decoded, then the fit. **Engine:** exponent toggle so
the resulting number can be tried immediately. **User:** mkv → WAV conversion (decoder unavailable
auditor-side).

---

## §54 Bridge response — ASK 15 `vel-calib` landed (2026-07-20)

Tool **0.16.0**. Both datasets shipped offline from staged `audio/ww_jaudio1/` (player extract; not repo).

```bat
python -m ww_bridge vel-calib
```

### 1. Per-track velocity histogram (scored literals only)

| BMS | tracks | scored note-ons | span | p10 / p50 / p90 |
|-----|-------:|----------------:|------|-----------------|
| `i_link` | 13 | 731 | **20–124** | 48 / 84 / 106 |
| `house` | 7 | 314 | **47–115** | 62 / 86 / 98 |

Interior-vs-exterior hypothesis (narrower house range → less exponent distortion): **directionally
supported** by span width 104 vs 68. Not a curve claim.

Zero register-indirect velocities in these two sequences (all scored literals).

### 2. IBNK `initVol` / velocity regions

| phys IBNK | rows | INST volumes | resolved (= INST×velScale) |
|----------:|-----:|--------------|----------------------------|
| 0 | 103 | all `1.0` | min **0.803** · median **1.0** · max **1.0** |
| 21 | 8 | all `1.0` | all **1.0** |

Only programs **9** and **24** (bank 0) dip below 1.0 via vel-region scale (~0.80–0.85). Competing
hypothesis "soft layers also get low initVol" is **weak for these Outset banks** — almost flat at
unity; exponent remains the prime suspect for the auditor fit.

### Artifacts

| Path | Role |
|------|------|
| `vel_hist.csv` | per (bms, track, velocity) counts |
| `vel_track_summary.csv` | percentiles + lin/sq spread per track |
| `ibnk_initvol.csv` | every INST key×vel region |
| `reports/vel_calib.md` | human summary |

### Still not Bridge

- Curve fit against captures · exponent toggle — **auditor / engine**.
- Do not treat these tables as WW's velocity curve (ASK constraint honored).

### Awaiting

Housing Security accept / amend. Auditor fit once WAVs exist.

---

## 55. ASK 16 — tempo/timebase extraction. **Exponent CLEARED by measurement; tempo is the live fault.**

### Measured result — the velocity curve is NOT the problem

User captured four samples (WW/TP × interior/exterior, PCM f32 48 kHz, OBS, no normalisation).
Auditor measured RMS envelope dynamic range, p90/p10:

| | WW | TP (ours) | TP/WW |
|---|---:|---:|---:|
| Interior | 4.68× (13.4 dB) | 4.54× (13.1 dB) | **0.97** |
| Exterior | 3.39× (10.6 dB) | 3.45× (10.8 dB) | **1.02** |

**Within 3% on both pairs.** A wrong exponent (squared where linear was wanted) would have produced
a range roughly **3.8× wider**. That signature is absent.

**⇒ The shipped velocity curve is correct. DO NOT change the exponent.** §54's `initVol` hypothesis
also drops in priority — aggregate balance is right.

*Auditor note: the 13.9:1 velocity-spread finding was a true property of the curve but was NOT the
audible fault. It was pursued too far on the strength of arithmetic before any output was measured.*

### Tempo — direction certain, magnitude NOT established

Three methods, three answers. **Auditor's audio analysis does not converge and is being abandoned
for this question:**

| method | interior | exterior |
|---|---|---|
| duration ÷ user loop count | ~1.50 | — |
| loop autocorrelation | 2.18 | 1.15 |
| beat autocorrelation | 2.15 | 1.18 |

Interior beat result struck the **search-window floor (0.200s)** — a boundary artifact, not a
measurement. Causes are structural: different instrument timbres change onset character, WW's
exterior carries ambient SFX (user predicted this), and captures begin at arbitrary musical points.

**What holds: every method returns a ratio > 1. We play TOO SLOW.** Magnitude unresolved.

### THE ASK — read the numbers instead of measuring the waveform

**Bridge:** extract from `i_link.bms` and `house.bms`:

1. Every **tempo** (`0xFD`) and **timebase** (`0xFE`) value the sequence sets, with the tick offset
   at which each occurs — including whether either is ever set at all.
2. The **initial/default** values assumed if the sequence never issues them.

Report-only, offline; no engine change, no audio touched.

**Why this beats more signal processing:** the answer is *in the data*. `ja1_track.cpp:36` defaults
`mTimebase = 48`; a wrong default, or a `0xFE` that is parsed but never applied to tick conversion,
would produce exactly this fault class — **uniform, not drifting**, which matches the user's report
of a consistent speed error rather than progressive desync.

### Companion items (not Bridge's)

- **Engine:** one diagnostic line — parsed tempo, parsed timebase, and derived **ticks-per-second**
  at `startOwned`. Pairs directly with Bridge's extraction: specified vs derived.
- **Decomp (auditor):** WW's tempo+timebase → real-time conversion in `JASSeqCtrl` / `JASTrack`.
  Unlike `BankMgr::noteOn`, the timing path may be matched; if so the formula is authoritative and
  this closes exactly.

**Three numbers — specified, derived, and WW's formula — settle it arithmetically.** No further
listening or capture required.

---

## §55 Bridge response — ASK 16 `tempo-map` landed (2026-07-20)

Tool **0.17.0**. Report-only extraction from staged BMS.

```bat
python -m ww_bridge tempo-map
```

### Defaults (if never issued)

| | value | source |
|---|------:|--------|
| tempo | **120** | WW `TTrack` init `field_0x376=0x78`; engine `mTempo=120` |
| timebase | **48** | WW init `field_0x378=0x30`; engine `mTimebase=48` |

### Specified in the sequences (tick 0, root track)

| BMS | `0xFE` timebase | `0xFD` tempo | later changes |
|-----|----------------:|-------------:|---------------|
| `i_link` | **120** @ tick 0 / `0x3C` | **119** @ tick 0 / `0x3F` | none |
| `house` | **120** @ tick 0 / `0x1E` | **146** @ tick 0 / `0x21` | none |

**Neither sequence relies on the timebase default.** Both explicitly set timebase **120**
immediately (before any wait). Tempo is near-default on Outset exterior (119) and higher indoors (146).

### Arithmetic hint for engine (not a Bridge claim of the bug)

WW root `updateTempo`: `field_0x368 ∝ timebase × tempo`. Wait countdown advances by that factor —
**higher timebase → faster playback**.

If the engine kept default timebase **48** while the BMS demands **120**, speed ratio would be
`48/120 = 0.40` (≈ **2.5× too slow**). That is the same fault class every auditor method pointed at
(ratio > 1, we play slow). Bridge does not assert the engine fails to apply `0xFE` — only that the
**specified** value is 120, not 48. §56's diagnostic line closes the loop.

### Artifacts

`audio/ww_jaudio1/tempo_map.csv` · `reports/tempo_map.md`

### Still not Bridge

- Engine §56: log parsed tempo / timebase / derived ticks-per-second at `startOwned`.
- Auditor: WW formula confirmation if still open.

---

## 56. ENGINE ASK — one diagnostic line. **Plus one WITHDRAWAL.**

Routed via user (Engine's bus is the №-ledger, not this doc).

### WITHDRAWN — exponent toggle. **Do not build it.**

Auditor previously asked Engine to expose the velocity exponent as a runtime setting so it could be
swept by ear. **§55's measurement makes that unnecessary:** dynamic range matches WW within 3% on
both capture pairs, so the shipped curve is correct.

Verified nothing was built against it — **no wasted work.** Withdrawing explicitly so it doesn't get
picked up later from a stale queue.

### ALREADY DONE — no action (verified, not re-asked)

Tempo propagation. Auditor flagged children copying `mTempo`/`mTimebase` at spawn with no update on
change. **Fixed:** `setTempo`/`setTimebase` now fan out via `updateTempo()` from the root only, with
child tempo relative — and the code cites WW's own behaviour for that split. Confirmed on disk;
closed.

### THE ASK — log three numbers at `startOwned`

```
[ExtSeq] §55 timing seq=<i_link|house> tempo=<value> timebase=<value> ticks_per_sec=<derived>
```

- **tempo** as parsed from the sequence (or the default, if it never issues `0xFD`)
- **timebase** likewise (default is `48` per `ja1_track.cpp:36`)
- **ticks_per_sec** — the *derived* rate actually driving playback

**Why this and nothing else:** measured evidence says we play **too slow** (every method returned a
ratio > 1), but three attempts at measuring *how much* disagreed — 1.50, 2.18/1.15, 2.15/1.18 — and
the interior beat result hit a search boundary. **Audio analysis has been abandoned for this
question.**

Bridge (ASK 16) is extracting what the sequence *specifies*. This line reports what the engine
*derives*. Auditor supplies WW's conversion formula from `JASSeqCtrl`/`JASTrack`. **Three numbers,
compared arithmetically — the discrepancy becomes exact instead of estimated.**

**Prime suspect, stated so it can be checked directly:** a `0xFE` that is parsed and stored but never
applied to tick conversion, or a wrong default. Either produces a **uniform** speed error rather
than progressive drift — which matches the user's report of consistent wrongness, not desync.

**Scope:** one log line. **No behaviour change, no tuning, no fix attempt** until the three numbers
are on the table.

---

## 57. ENGINE ASK — tempo rate: **VERIFY FIRST, then port into `ext_seq/`**

### Where this goes (user asked; answering explicitly because it will recur)

**Not the mod folder.** Code cannot live there — no plugin channel exists (H3 research-only). **№31-B
governs which engine OWNS a shaping stage, not which folder holds the code.** Content is mod-side;
code is in the exe; separation is by **lane**.

**Correct home: `src/d/ext_seq/`** — the WW-owned playback path, created for exactly this. WW spaces
get WW timing via ExtSeq; TP spaces keep JAudio2's untouched. **That is the no-cross-pollination
condition satisfied.** Putting WW's formula into shared JAudio2 code would be the actual violation.

### The finding

**WW** (`JASTrack.cpp:748`, `TTrack::updateTempo`) derives the rate as:

```
rate = timebase × tempo / Kernel::getDacRate() × 1.33333333
```

**Ours** (`ja1_track.cpp:213`, `Ja1Track::updateTempo`) **only propagates values to children — it
computes no rate at all.** The propagation half was added correctly after the earlier flag; the
arithmetic half is absent. The logged `ticks/s` comes from elsewhere as `tempo × timebase / 30`:

| sequence | tempo | timebase | logged ticks/s | = |
|---|---:|---:|---:|---|
| `i_link` | 119 | 120 | 476.00 | `tempo × timebase / 30` |
| `house` | 146 | 120 | 584.00 | same |

**Two terms missing: the `/ getDacRate()` divisor and the `× 4/3` factor.**

**Why it produces a uniform speed error:** WW's rate is *inversely proportional to DAC rate*, by
design, so playback speed is invariant to output sample rate. Ours has no DAC term, so it is pinned
to whatever `30` encodes. `JASAiCtrl.cpp:106-109` shows Dusklight selects **32000 or 48000** at
runtime; WW hardware was 32 kHz. **48000/32000 = 1.5**, matching the duration-based estimate (~1.50)
— the most reliable of the three.

### VERIFY BEFORE PORTING (user-directed, and warranted)

**Do not implement on the strength of the above.** Confirm three things first:

1. **What `sDacRate` actually resolves to at runtime** — and note `JASAiCtrl.cpp:113` applies a
   `× 1.0008897` trim on top.
2. **What units WW's `field_0x368` is in.** It is an accumulator increment, **not** ticks-per-second.
   The `30` in our formula may be absorbing a units conversion that must be re-derived alongside the
   DAC term — porting the expression literally without resolving units could produce a *different*
   wrong answer.
3. **Whether the 1.5 correspondence survives.** The §56 log line already reports derived rate; after
   the change it should move by the predicted factor. **If it does not, the hypothesis is wrong and
   should be discarded, not tuned toward.**

### Auditor caveat — stated plainly

**This finding pairs a real structural difference with a numerical coincidence.** The missing terms
are certain (both formulas are readable). The 1.5 correspondence is *suggestive, not proof* — and
this auditor has produced several confident wrong readings this session, including asserting TP
mixer interference that measurement then disproved. **Verification is not ceremony here.**

### Scope

Rate derivation only. **No tuning, no constants adjusted to taste, no exponent changes** (§55 cleared
the velocity curve by measurement — leave it alone).

---

## 58. ENGINE — §57 verification result. **HYPOTHESIS DEAD. Do NOT change the rate.**

Verify-first was the right call. It disproved the auditor's lead.

```
tempo=119 timebase=120 engine_ticks/s=476.00 dacRate=32028.500
ww_inc=0.594471 ww_ticks/s=238.00 (verify-only)
```

### DEAD — the DAC-rate hypothesis

**`dacRate = 32028.5`, not 48000.** Dusklight already runs at GameCube's native rate, so there is
**no DAC-rate discrepancy**. §57's `48000/32000 = 1.5` correspondence was **coincidence** — flagged
there as "suggestive, not proof," and now falsified. **Discard it; do not carry it forward.**

### The 2× — real number, unresolved meaning

Engine **476.00** vs WW **238.00** — exactly 2× (`tempo × timebase / 30` vs `/ 60`).

**DO NOT halve the rate on this.** Two reasons:

**1. A 2× tempo error is not "slight."** Half- or double-speed music is unmistakable. The user
reported *"there may be a speed issue slightly."* **A factor of two and that description are
incompatible** — so something in the comparison is wrong, not the playback.

**2. `ww_ticks/s = 238` is an INTERPRETATION, not a readout.** `ww_inc = 0.594471` is a per-sample
accumulator increment; at 32028.5 samples/s that is **~19,040 ticks/s**, nowhere near either figure.
So 238 was derived by assuming a units mapping — **the very assumption §57 item 2 asked to verify,
and it is still unanswered.**

### THE ASK — answer the units question, change nothing

**What is WW's `field_0x368` actually in?** Per-sample increment, per-frame increment, or something
else — and what does the sequencer consume it with per tick? Read `JASSeqCtrl` / the tick-advance
path in the decomp.

**If it is a per-sample increment and our `ticks/s` is a per-frame count, the two numbers were never
comparable and the 2× dissolves.** That outcome is as valuable as finding a real defect: it closes
tempo as a suspect and redirects effort.

**Only after units are settled** should any rate change be considered — and then only if the corrected
comparison still shows a discrepancy.

### Wider question worth raising now

**Is the remaining fault even tempo?** The user's three original symptoms:

| symptom | status |
|---|---|
| layers at wrong levels | **measured CORRECT** (§55 — within 3% of WW, both pairs) |
| parts enter earlier than intended | **open** — a *placement* fault, not a global-rate fault |
| speed "slightly" off | **unsubstantiated** — auditor's three measurements disagreed and were abandoned |

**A layer entering at the wrong bar is not the same defect as everything running fast.** Global rate
is uniform; wrong entrances are per-track. If units dissolve the 2×, the entrance question is where
the remaining fault most likely lives — and it points back at `OpenTrack` (`0xC1`) offsets and
per-track tick accumulation rather than at tempo.

### Auditor note

**Two confident wrong leads this session on this problem** — TP mixer interference (disproved by the
user's captures) and the DAC-rate ratio (disproved by this log). Both were structurally plausible and
both had a clean number attached. **The pattern is mine: a tidy arithmetic match is not evidence.**
Verify-first should stay the default on anything auditor-originated here.

---

## 59. ASK 17 — independent BMS event decode. **Stop hypothesising; diff two implementations.**

### Why this ask exists

**Four auditor hypotheses have died on this problem**, each plausible, each with clean arithmetic,
each costing a build cycle:

| # | hypothesis | killed by |
|---|---|---|
| 1 | TP mixer interference (3-factor volume chain) | user's captures — both TP factors measured at unity |
| 2 | Wrong velocity exponent (13.9:1 spread) | dynamic range measured **within 3%** of WW, both pairs |
| 3 | DAC-rate mismatch (48k vs 32k = 1.5×) | log: `dacRate = 32028.5` — no mismatch exists |
| 4 | Frame-clocked tick at 200+ fps | Dusklight interpolates; logic stays ~60 Hz. User A/B: **no change** |

`OpenTrack` was the fifth candidate; auditor read both implementations and **found no concrete
defect**. **Further hypothesis generation is not converging and is being stopped.**

### Current symptom (user, after FPS A/B)

*"Some layers sound louder than they should, some parts arrive at the wrong time — or it's one of
those and it's impacting how I hear the other."*

**That last clause may be the whole answer.** If entrances are misplaced, tracks meant to be
sequential **stack**, and simultaneous voices read as "louder." That reconciles the one hard
measurement we have — **aggregate dynamic range is correct (§55)** — with a persistent perception of
imbalance. Levels right, arrangement wrong.

### THE ASK — a second, independent decoder

Write an **offline BMS event decoder** in `ww_bridge`, built from the §50b dialect map, sharing **no
code and no assumptions** with the engine's `Ja1Parser`. Emit for `i_link.bms` and `house.bms`:

```
tick, track_id, event, note/param, velocity
```

Engine emits the equivalent from its own parse. **Diff the two streams.**

**Where they disagree is the bug** — no listening, no signal processing, no hypothesis. If Bridge
places a layer at tick 4800 and the engine at tick 0, that is the answer in one line.

**Independence is the point.** A decoder sharing the engine's assumptions would reproduce its errors
and agree — proving nothing. Build it from the dialect map, not from `ja1_parser.cpp`.

### Why this should work where the rest hasn't

**Precedent: B0.** Byte-identical census regeneration turned *"does this look right?"* into a
mechanical check, and it has been the most reliable instrument in the project since. **This is the
same move applied to sequence events.**

It also settles the user's confound without asking them to separate two perceptions by ear — a task
they have reasonably said they are not equipped for, and should not be asked to do.

### Companion (Engine, not Bridge)

Dump the engine's own event stream in the same format at `startOwned`, gated behind a diagnostic flag.
**No fix attempt** — the diff identifies the target first.

### Scope

Report-only, offline, player-extract only. **No audio, no engine behaviour change, no tuning.**

---

## §59 Bridge response — ASK 17 `seq-events` landed (2026-07-21)

Tool **0.18.0**. Independent offline decoder — **no shared code** with engine `Ja1Parser` or with
Bridge `vel_calib` / `tempo_map`. Authority: §50b dialect map + WW `JASSeqParser` Arglist / `cmdNoteOn`.

```bat
python -m ww_bridge seq-events
```

### Diff columns (ASK format)

```
tick, track_id, event, note_param, velocity
```

Per-sequence files for engine diff:

| File | events | note_on |
|------|-------:|--------:|
| `seq_events_i_link.csv` | 5215 | 731 |
| `seq_events_house.csv` | 1978 | 314 |

Combined + `file_off`: `seq_events.csv`. Report: `reports/seq_events.md`.

Ticks are **per-track local** (0 at root start / at each `open_track` entry). Song-loop backward
`jmp` followed once so the stream is finite. Loop bodies expanded (cap 256).

### Still not Bridge

- ~~Engine: dump the same five columns at `startOwned` (diagnostic flag).~~ **Done** — see §59 Engine response below.
- **Diff the streams.** No fix attempt until disagreement is located.

---

## §59 Engine response — ASK 17 event dump companion (2026-07-21)

Engine dump from **`Ja1Parser`** (independent of Bridge `seq_events.py`). Gated by env:

```
set DUSK_EXTSEQ_EVENT_DUMP=1
```

First `startOwned` writes next to the package root:

| File | Source |
|------|--------|
| `seq_events_engine_i_link.csv` | island BMS via engine parse |
| `seq_events_engine_house.csv` | house BMS via engine parse |

Same columns as Bridge. Log: `[ExtSeq] §59 event dump: N events → …`. **Playback unchanged.** Diff vs `seq_events_{i_link,house}.csv`; first mismatch is the bug line.

---

### Sources (§1)

[The Fallout Wiki — Tale of Two Wastelands](https://fallout.wiki/wiki/Mod:Tale_of_Two_Wastelands) ·
[The Best of Times — Installing TTW](https://thebestoftimes.moddinglinked.com/ttw.html) ·
[The Best of Times — Introduction](https://thebestoftimes.moddinglinked.com/intro.html) ·
[DSOGaming — TTW merges Fallout 3 into New Vegas](https://www.dsogaming.com/news/tale-of-two-wastelands-is-a-mod-that-seamlessly-merges-fallout-3-and-its-dlcs-into-fallout-new-vegas/)
