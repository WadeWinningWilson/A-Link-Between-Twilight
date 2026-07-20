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

### Sources (§1)

[The Fallout Wiki — Tale of Two Wastelands](https://fallout.wiki/wiki/Mod:Tale_of_Two_Wastelands) ·
[The Best of Times — Installing TTW](https://thebestoftimes.moddinglinked.com/ttw.html) ·
[The Best of Times — Introduction](https://thebestoftimes.moddinglinked.com/intro.html) ·
[DSOGaming — TTW merges Fallout 3 into New Vegas](https://www.dsogaming.com/news/tale-of-two-wastelands-is-a-mod-that-seamlessly-merges-fallout-3-and-its-dlcs-into-fallout-new-vegas/)
