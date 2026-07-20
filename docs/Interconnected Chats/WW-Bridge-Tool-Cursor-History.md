# WW Bridge Tool — Bridge (Cursor) ↔ Housing Security

**Interconnected Run** — offline `ww_bridge` toolchain (TTW model): player’s legal US WW copy → census + adapted arcs; distribute zero Nintendo bytes.

| | |
|---|---|
| **Protocol** | [INTERCONNECTED-RUNS.md](INTERCONNECTED-RUNS.md) |
| **Live state** | [state/ww-bridge-tool.md](../state/ww-bridge-tool.md) |
| **Run control** | [state/run-control.md](../state/run-control.md) |
| **Spec** | [ww-bridge-tool-interconnected.md](../ww-bridge-tool-interconnected.md) |
| **Roles** | **Cursor (Bridge)** = implement B0/B4 after dual sign-off · **Housing Security** = spec + acceptance + critique (does not build) |
| **Drive** | **Not in scope** |
| **Status** | **Bridge lane clear** — §16 accepted; GAP-3 closed; content/F-1/F-2 → user |

> **Lane discipline (user-decreed 2026-07-19):** two-party only — Housing Security ↔ Bridge. History owns the Outset/WW **content** lane. Content questions (e.g. G5 arcs) go back to the user for routing, not into this doc as History turns. **Doc is the bus** — no chat-to-chat between AIs.

---

## Run header

| Field | Value |
|-------|--------|
| **run_name** | WW Bridge Tool — Bridge ↔ Housing Security |
| **created** | 2026-07-19 |
| **live_state** | `docs/state/ww-bridge-tool.md` |
| **roles** | Cursor = Bridge implement · Housing Security = auditor |
| **drive_in_scope** | **no** |
| **status** | B4 tool done — awaiting Housing Security assess |

---

## User brief

### Goals

1. Critically review the bridge-tool spec — especially revised §4 and §9.
2. Ratify standalone project + `schema_version` handshake.
3. Rule receiver delivery (Mod + code vehicle ladder).
4. Ground design in real mod API (CLI-only offline tool).
5. After dual APPROVED: **B0 → B4** without disturbing Outset.

### Guidelines

- Dual sign-off before Phase D (unless user overrides).
- Housing Security does not build the tool it audits.
- B0 diffs against **copies** of live CSVs; never overwrite live until user OK.
- Leave `list_rarc` / `entry_size` / `parse_dzr` untouched in B0.
- **Sibling repo from commit one** — no in-tree staging under the skeleton (auditor + Bridge R7).
- Doc is the bus.

### Expectations

- Critique seriously.
- Commit / push only when user asks.
- Drive: not requested.

---

## Deliberation log

### Phase A — Bridge proposed plan (2026-07-19)

*(Full text retained earlier in session; summary: §4 APPROVE standalone + integer handshake; Q2 one Mod / phantom→optional dll; Q3 CLI-only no host API; B0→B4; G5 bounced as content.)*

User: **§4 + Q2 APPROVED.**

### Phase B — Housing Security critique of Bridge Phase A

**Source of record:** spec [§11](../ww-bridge-tool-interconnected.md) (2026-07-19).

**Verdict: RATIFY with four additions (A-1…A-4).**

Conceded to Bridge: Q3 drop adapter; Q2 MO2 ladder + phantom rung; Q1 integer + missing-meta = v0 legacy warn.

Also: M6 CLEAN on current exe; UTF-16 gate = defense not remediation; prefer sibling repo from commit one (no skeleton staging); G5 withdrawn from this lane.

| # | Addition | Meaning |
|---|----------|---------|
| **A-1** | Refused CSV → Mods **conflict badge** | DuskLog alone is developer-loud; player needs a visible refused-population badge on the mod row |
| **A-2** | Per-arc **source hash mandatory in B0** | Tool↔engine schema ≠ source↔tool; hash what extraction produced so “wrong Outset” is diagnosable |
| **A-3** | Byte-diff is permanent → lives in **B4 `verify`** | Not a one-shot acceptance only |
| **A-4** | Engine `kExtPopSchemaVersion` patch still under **M6** | Neutral names; not exempt from greplist |

### Phase C — Unified plan (Bridge, 2026-07-19)

Absorbs Phase A + §11 additions. **This is the execute plan.**

#### Architecture (locked)

- Tool: **standalone sibling repo** from first commit (name TBD with user — default `ww-bridge`).
- Offline surface: `argparse` only — `--ww-root`, `--mod-root`, stdout/stderr. No host-API adapter.
- Product: **one Mod** (folder) in Mods API; receiver code phantom-mod now, optional `plugin.dll` later.
- Handshake: `population/bridge_meta.ini` with `schema_version=<int>`; engine missing → warn + v0 legacy; mismatch → refuse CSV **and** surface A-1 badge.
- Ini/CSV = facts; DuskScript = behavior later (ceiling only this run).
- G5 / content semantics: **out of lane** — user routes to Outset/History.

#### Execute order: B0 → B4 (+ engine handshake in same slice notes)

**B0 — generalize census (sibling repo)**

1. Create sibling project; port proven `list_rarc` / `entry_size` / `parse_dzr` **verbatim** (no rewrite).
2. CLI: `ww_bridge census <arc> --out <csv> [--stage NAME] [--append]`.
3. Parameterize paths/stage; drop A_mori debug prints; re-run drop keys on `--stage`.
4. **A-2:** for each source arc, record content hash (e.g. SHA-256) into `bridge_meta.ini` (and/or per-arc lines). Mandatory, not optional.
5. Emit `schema_version=1` in `bridge_meta.ini` beside temp outputs.
6. Acceptance: regenerate both CSVs to **temp dir**; byte-identical vs live  
   `%AppData%\…\WW-Crew-Restoration\population\{outset,interior}_placements.csv`.  
   **Never write live.**
7. No in-tree prototype under `tools/ww_crew_restoration_skeleton/`.

**Engine handshake (this fork, after B0 proof — same slice)**

1. `kExtPopSchemaVersion` in population loader; missing meta = warn once + v0; mismatch = refuse that CSV + log.
2. **A-1:** refused / schema-mismatch population → conflict-badge (or equivalent visible mod-row signal) via existing `custom_assets` Phase 3 path.
3. **A-4:** name constants M6-clean; run `gate` after patch.

**B4 — verify + gate**

1. `verify`: arcs, population CSVs + stage rows, dialogue keys, no `J3D2bdl4`, no abs paths, skeleton↔live drift.
2. **A-3:** `verify` includes standing byte-diff (or digest) against golden/live reference CSVs when present — permanent regression guard (R1).
3. `gate <exe>`: greplist with UTF-16LE read as **hardening** (auditor confirmed current exe has zero wide hits — not a caught bug).
4. Read-only vs live + skeleton; no Outset content edits.

**Deferred (do not block B0)**

- B1 schema collapse, B2 SCLS, B3 installer command, B5 general bridge, GUI.
- R5 save compat / M5b folder-absent smoke — note only; Outset/containment lane.

| Role | Sign-off | Notes | When |
|------|----------|-------|------|
| Cursor (Bridge) | **APPROVED** | Unified plan absorbs A-1…A-4 + sibling-repo-from-commit-one | 2026-07-19 |
| Housing Security | **APPROVED*** | §11 “RATIFY with four additions” — *confirm if unified plan matches intent* | 2026-07-19 |

\*Auditor verdict predates this Phase C write-up; if anything in A-1…A-4 was mis-absorbed, REQUEST CHANGES in this doc.

**Implementation gate:** both APPROVED + **user go** (sibling repo name/location) before Phase D.

### Phase D — Execute notes

**Sibling home (user 2026-07-19):** `C:\Users\xxxxx\Documents\albt bridge`

**B0 landed:**
- Package `ww_bridge` at `C:\Users\xxxxx\Documents\albt bridge`.
- Commands: `census`, `regen-proof`, `diff-live`.
- Outset strict byte-MATCH verified by auditor (`md5 e0302104…`).

**Auditor §12 ruling (B0 acceptance) — absorbed:**
- Interior divergence is **not drift**: position identity 571/572; live CSV is **lossy** (rotation zeroed on seven stages). Tool recovers authored `ry`.
- **Do not** keep live interior as golden; **do not** Bridge-refresh live (content-lane / playtest).
- **Golden = regenerated set.** B4 `verify` diffs against that.
- Live refresh → Outset content lane + playtest.
- Corrections: mixed CRLF/LF claim **not reproduced** (auditor: uniform CRLF); understating as “drift” withdrawn — B0 success.
- **B0 ACCEPTED. Proceed to B4.**

**B4 tool side landed:**
- `seed-goldens` → `goldens/` (regenerated reference per §12).
- `verify` / `gate` negative-controlled by auditor (§13).
- No live writes.

**Auditor §13 — absorbed:**
- B4 **ACCEPTED** with GAP-1 (dialogue= check missing).
- **GAP-1 CLOSED:** `check_dialogue` — every `dialogue=` must hit a `[section]` in `dialogue/*.txt`; negative control fires. Live verify still CLEAN.
- **GAP-2 (minor):** skeleton drift now also WARNs same-name **size** divergence (actor_map/doors/etc.).
- Re-seed procedure documented in sibling `README.md` (arcs change → `seed-goldens` + run-doc note).
- Skeleton drift growing = content-lane flag (not Bridge to fix).
- A-1 / A-2(engine) / A-4 still open.

**Auditor §14 — absorbed:** GAP-1/2 CONFIRMED CLOSED. B4 done. Bridge clear for engine handshake. Skeleton half-built escalation → **user / Outset content lane** (not Bridge).

**Auditor §15 — absorbed (pre-handshake):**
- Decomp confirms angle @0x18 — §12 rotation ruling holds.
- Captured **`set_id` + `sx,sy,sz`** via new `parse_dzr_full` (`parse_dzr` left verbatim).
- **`schema_version=2`**, tool `0.2.0`, goldens re-seeded; `verify` CLEAN.
- Receiver may ignore new columns for now; archival capture done while window open.

**Engine handshake landed (2026-07-19):**
- `kExtPopSchemaVersion = 2` in `d_ext_npc_population.h`.
- Missing `bridge_meta.ini` / schema → **legacy v0 warn once**, population proceeds (live Outset OK).
- Mismatch → refuse that CSV + DuskLog + Mods **`[POP!]`** badge via `set_mod_status_note` (A-1).
- Build OK (`/O2`); dawn/pipeline caches wiped; `gate` CLEAN (A-4).
- Live folder still has no meta → legacy path until content-lane refresh + tool regen.

**A-1 / A-2(engine) / A-4:** implemented this slice. Tool-side A-2 (source hashes) already in `bridge_meta.ini`.

**Auditor §16 — ACCEPTED (handshake).** Legacy path runtime-confirmed (71 actors). Step 2 mismatch→`[POP!]` still optional playtest. Two fail-open edges noted.

**Auditor §17 — absorbed:**
- F-1/F-2/F-3 → out of lane / user (parked folders still overlay; rename≠disable; backup exe fails gate).
- **GAP-3 CLOSED:** `verify` WARNs parked-named siblings with `arcs/`/`files/` (caught `WW-Crew-Restoration.SKELETON_BAK`).
- Fail-open harden: negative `schema_version` / unreadable meta ⇒ **Mismatch** (not legacy).
- README: backup exes must `gate` before share.

**User ruling (2026-07-19) — F-1 / parked-folder engine fix NOT necessary:**
Noted for the record, but **do not prioritize** a `custom_assets` parked-name skip or “move folder for M5b” as Bridge/Outset work. Local parked siblings (`.SKELETON_BAK`, `.DISABLED`, etc.) are **gitignored AppData clutter**, not distributable content — they never ship with the skeleton or a player zip. GAP-3 WARN in `verify` is enough awareness. Real disable for smoke = Mods UI or not having the folder; no engine chase unless containment audit reopens it.

---

## ===== BEGIN HISTORY SECTION (content lane) — informational, posted at USER REQUEST 2026-07-19 =====

> **Why this is here:** this run is two-party (Bridge ↔ Housing Security) and History is normally
> out of lane. The user directed History to post a triage of this lane's three open items plus
> content-lane news that changes assumptions here. **No turn is claimed or transferred — see the
> END note.** Full detail lives in [state/cut-actors-demo-restore.md](../state/cut-actors-demo-restore.md) №75–№81.

### 1. Triage of this lane's open items (asked by the user: still valid, postpone, or defunct?)

| Item | Verdict | Reasoning |
|---|---|---|
| **§16 step 2 — `schema_version=99` ⇒ refusal + `[POP!]` badge** | **VALID NOW — neither blocked nor defunct** | Tests the schema handshake and the mod-row badge path only; **independent of stage/room hosting and of save slots**, so the interiors pivot cannot invalidate it. Sole scheduling note: it deliberately breaks population for one boot — don't run it mid-door-playtest. (Step 1 legacy path already confirmed in the wild, §16 UPDATE.) |
| **M5b folder-absent smoke** | **NOT defunct — RUN TWICE** | Still never validly performed (your F-2: rename ≠ disable; valid procedure = move the folder out / Mods-UI off). **Run now as a cheap baseline**, then **re-run after the stage pivot with expanded checks**. The expansion's shape depends on an open content-lane question: **if WW stage shells live in the game tree**, M5b must also verify vanilla boots/warps normally with foreign stage folders present; **if shells live mod-side**, it must verify the warp menu degrades gracefully when its target stages vanish. Either way the WW-absent surface grows, not shrinks. |
| **§12 live census refresh (recovered `ry` rotations)** | **ACCEPTED as CONTENT-LANE (History) debt — scheduled before stage-era authoring** | Your decomp validation stands: live CSVs carry **zeroed rotations = real data loss**, and gen recovers Nintendo's authored facings. This is very likely part of the "objects facing wrong / confused props" the user has been reporting in interiors. History will land it **before** authoring the new stage/room manifests so they are built on correct facings rather than redone. **Two cautions History owns:** (a) hand-appended live rows must survive the refresh — `Ojhous2R1` (Sturgeon's room, adapted from `Stage/Ojhous2/Room1.arc`) and the furniture arcs (Otana/Table/Plant/Opaper/Lamp); (b) your Result 2 — **two fields the CSVs never capture (`setID`, SCOB `scale`)** — History will assess mount relevance before adopting wholesale. |

### 2. Content-lane news that touches assumptions in this lane

- **Save architecture decision (user call, 2026-07-19): EXTENSION-FIRST.** Trigger: Outset is currently hosted in `F_SP115`, which declares `dStage_SaveTbl_LANAYRU` (4) — **any native save write from WW content today would land in Lanayru's chest/switch bits.** Unexercised so far (no WW chest/key/switch wired), but the exposure is live. Decision: **all WW progression goes into a Dusklight save-extension block from day one** (mirroring `dSv_memBit_c` layout so native chest/key/boss accessors work unchanged, with a versioned non-destructive migration). The **nine free vanilla save indices (5, 12, 13, 14, 15, 28, 29, 30, 31 — derived from the `dStage_SaveTbl` enum) are reserved as headroom for FORK-SIDE TP features**, not consumed by WW.
- **Interiors/stage pivot is proceeding** (interiors move out of the host stage's world space into their own stage(s), one room per interior, TP's `R_SP01` pattern). Consequence for this lane: **what "the live folder" contains may change, and stage shells will exist somewhere** (game tree vs mod-side is the open question above) — relevant to M5b scope and to the containment gate.
- **Covenant note relevant to your gate:** History **rejected `R_WW01`-style stage names** — stage folders live in `/res/Stage/` = the game tree, so WW tokens there would breach the receiver covenant. Preference is neutral names (or mod-side shells entirely). Flagging so the greplist gate expects neutral stage identifiers rather than WW ones.
- **Possible future ask (NOT now, no action requested): if WW progression tables become authored data** (chest/flag maps per stage), the bridge may eventually want to emit them alongside the census CSVs, to keep the "regenerate from a legal disc" property intact for progression as well as placement.

### 3. What History is NOT asking for

No changes to B0/B4, schema v2, the handshake, or GAP-3 — all accepted and unaffected. No response is required to this section; it exists so this lane isn't surprised by the pivot or by the save decision.

## ===== END HISTORY SECTION — turn is NOT held by History =====

> **Turn returns unchanged to Bridge ↔ Housing Security ↔ User.** History holds no turn in this
> lane; route any content-lane question (rotations, arcs, stage/room mapping, progression tables)
> **through the user**, per the standing lane discipline. Content-lane state of record:
> [state/cut-actors-demo-restore.md](../state/cut-actors-demo-restore.md).

## Turn board

| When | Whose turn | Action |
|------|------------|--------|
| 2026-07-19 | Housing Security | §16 handshake ACCEPTED; §17 F-1..F-3 + GAP-3 |
| 2026-07-19 | Bridge | GAP-3 + schema fail-open harden |
| 2026-07-19 | User | F-1 engine fix **declined** (gitignored local only) |
| optional | User | mismatch→`[POP!]` playtest (§16 step 2); skeleton reconcile when ready |
| 2026-07-19 | History *(informational — no turn held)* | Posted delimited **HISTORY SECTION** above at user request: `[POP!]` test **valid now** · M5b **run twice** (expanded after stage pivot) · §12 rotations **accepted as History debt** · save **EXTENSION-FIRST** (F_SP115 = LANAYRU exposure; 9 free indices reserved as fork headroom) · **neutral stage names** for the gate. **Turn unchanged: Bridge ↔ Housing Security ↔ User.** |

---

## Sign-off mirror

| Party | Status | When |
|-------|--------|------|
| Housing Security | §16 ALL ACCEPTED; §17 GAP-3 open→Bridge | 2026-07-19 |
| Bridge (Cursor) | Bridge lane clear (GAP-3 closed) | 2026-07-19 |
| User | content/F-1/F-2 routing | 2026-07-19 |
| History | **not in lane** | — |
