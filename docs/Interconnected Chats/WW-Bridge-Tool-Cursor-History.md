## §62 Engine — remainder-carry (2026-07-21)

Trunc confirmed; fixed-point carry on `/1800`; clamps logged. Await user re-test.

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
| 2026-07-20 | Bridge | §26 ASK 1→3 implemented; reports for Housing Security |
| 2026-07-20 | Housing Security | §27 port policy RATIFIED; §28 ASK 4/5 posted |
| 2026-07-20 | Bridge | §28 ASK 4+5 shipped (`verify` deep RARC + `diag symbolize`) |
| 2026-07-20 | Housing Security | §29 ASK 1–5 **ALL ACCEPTED**; B2 unblocked |
| 2026-07-20 | Bridge | B2 `scls` shipped (0.4.0); ASK 5 exit=2 confirmed |
| 2026-07-20 | Housing Security | §30 ledger greenlit + ASK 4 triage |
| 2026-07-20 | Bridge | §30 `ledger` shipped (0.5.0); Outset work-order proven |
| 2026-07-20 | Housing Security | §31 B2+ledger **ACCEPTED**; §32/§33 ASK-4 BG; §34 ASK 6 |
| 2026-07-20 | Bridge | ASK 6 `quest-triggers` shipped (0.6.0); hybrid verdict |
| 2026-07-20 | Housing Security | §35 ASK 6 **ACCEPTED**; §36 EVNT table decompressed |
| 2026-07-20 | Bridge | §36 eventNo→names (`evnt` + quest-triggers, 0.7.0) |
| 2026-07-20 | Housing Security | §38 ASK 7+8 **GREENLIT** |
| 2026-07-20 | Bridge | ASK 7 `anim-ledger` + ASK 8 `beh-ledger` (0.8.0) |
| 2026-07-20 | Housing Security | §39 ASK 7/8 **ACCEPTED**; §40–45 specs; **ASK 9** posted |
| 2026-07-20 | Bridge | ASK 9 `day1-guard` + `identify` (0.9.0) |
| 2026-07-20 | Housing Security | §46 ASK 10 `space-report` posted |
| 2026-07-20 | Bridge | ASK 10 `space-report` → Outset/inventory.md (0.10.0) |
| 2026-07-20 | Housing Security | §47 ASK 11 `audio-map` posted |
| 2026-07-20 | Bridge | ASK 11 `audio-map` — split CONFIRMED; Outset sequenced (0.11.0) |
| 2026-07-20 | Housing Security | §48 ASK 11 **ACCEPTED**; recommend ASK 12 BMS→instruments |
| 2026-07-20 | User / Bridge | “go for it” → ASK 12 `seq-banks` (0.12.0) |
| 2026-07-20 | Housing Security | §49 ASK 12 **ACCEPTED**; §50 ASK 13 dialect-first |
| 2026-07-20 | Bridge | ASK 13.1 `bms-dialect` → **incompatible**; item 2 blocked (0.13.0) |
| 2026-07-20 | Housing Security | §51 (A) then §52 **(B) destination**; (A) throwaway |
| 2026-07-20 | Bridge | §52 (B) `bgm-package` → mod `audio/ww_jaudio1/` (0.14.0) |
| 2026-07-20 | Housing Security | §53 bank residency (engine); Bridge offline escape hatch |
| 2026-07-20 | Bridge | §53b `bank-map` → wave/program CSVs (0.15.0) |
| 2026-07-20 | Housing Security | §54 ASK 15 velocity hist + IBNK initVol |
| 2026-07-20 | Bridge | ASK 15 `vel-calib` (0.16.0) |
| 2026-07-20 | Housing Security | §55 ASK 16 tempo/timebase (exponent CLEARED) |
| 2026-07-20 | Bridge | ASK 16 `tempo-map` — TB=120 both BMS (0.17.0) |
| 2026-07-20 | Engine / HS | §56–58 tempo units; DAC hypothesis DEAD |
| 2026-07-21 | Housing Security | §59 ASK 17 independent BMS event decode |
| 2026-07-21 | Bridge | ASK 17 `seq-events` golden stream (0.18.0) |
| 2026-07-21 | Engine | ASK 17 event dump companion (`DUSK_EXTSEQ_EVENT_DUMP`) |
| 2026-07-21 | Engine | §60 loop-point wait = BMS song loop (dump policy only) |
| 2026-07-21 | Housing Security | gate false-CLEAN (npat=0) + greplist path |
| 2026-07-21 | Bridge | gate №31-C / §61b (0.19.0) |
| 2026-07-21 | Housing Security | §64 inst_volume zero-variance question |
| 2026-07-21 | Bridge | §64b ibnk-vol-check — literal 1.0f (0.20.0) |
| 2026-07-22 | Bridge | §C.1b seq-events set_param target+value (0.21.0) |
| 2026-07-22 | Housing Security | §70 cutscene audio scope; Bridge stream→.afc ask |
| 2026-07-22 | Bridge | §70c `stream-map` BSM 75/75 (0.22.0) |
| 2026-07-22 | Housing Security | §72 donor variant BGM; Bridge seq→bms unblock |
| 2026-07-22 | Bridge | §72b `seq-map` + extract `i_linkin` (0.23.0) |
| 2026-07-22 | Housing Security | §77/77b live-ab + wav-compare ask |
| 2026-07-22 | Bridge | §77e `live-ab` + `wav-compare` (0.24.0) |
| 2026-07-22 | Housing Security | §78 round-2 verdict; 3 Bridge defects |
| 2026-07-22 | Bridge | §78c zero-frame/tempo/--b-start (0.24.1) |
| 2026-07-22 | Engine | §88 CharVoice SE at demo message-open (package cues) |
| 2026-07-22 | Engine | §84 type-7 Fxline + SoundTable vol into ExtSeq/DuskDsp |
| 2026-07-22 | Bridge | §81b `aaf-control` type 6/7 + SoundTable BGM (0.25.0) |
| 2026-07-22 | Engine | §93 voice fxmix = room reverb (messageSePlay law) |
| 2026-07-22 | Bridge | §85c `voice-map` INF1→charVoice→CharVoice_0.aw (0.26.0) |
| 2026-07-22 | Bridge | §86b stage CharVoice_0.aw + ibnk_135/wsys_135; CLEAR (0.26.1) |
| 2026-07-22 | Bridge | §88b `voice-listen` CharVoice WAV pack (0.27.0) |
| 2026-07-23 | Bridge | §111 acceptance drives harness (0.33.0) |
| 2026-07-23 | Housing→Bridge | §110/§110b loss-protection BOTH (mod+receiver+bridge tool) |
| 2026-07-23 | Bridge | §109 predictions-as-code verdict engine (0.32.0) |
| 2026-07-23 | Bridge | §108 nledger-as-database schema/ingest/views (0.31.0) |
| 2026-07-22 | Bridge | §101c SC_01_mizu* TEV/blend/vtxα ref (0.30.2) |
| 2026-07-22 | Bridge | §99c model1=mizu/nami beach stack (0.30.1) |
| 2026-07-22 | Bridge | §97c waterline-inv + Always BTI 0x8B/0x8C (0.30.0) |
| 2026-07-22 | Bridge | §93b type-6=SeMgr concurrency ≠ getReverb (0.29.1) |
| 2026-07-22 | Bridge | §90b IsleLink payload + cue map (0.29.0) |
| 2026-07-22 | Bridge | §89c five-pack `CharVoice_0_wave094`…105 (0.28.1) |
| 2026-07-22 | Bridge | §89b voice-diag; Aryll=IsleLink not CharVoice (0.28.0) |
| 2026-07-21 | Engine | §60b re-dump MATCH + openTrack depth warn |
| 2026-07-21 | Engine | §62 tickOwned remainder-carry + clamp logs |
| next | User | re-test tempo; if drift remains → resolve/mix |
| content | User / content | §12 census refresh (PLYR+rot) — highest visible deploy |

---






## §93 / Engine voice fxmix = room reverb (2026-07-22)

Voice one-shots no longer use §81 music-scene send (~0.5). Now `messageSePlay` law:
`dComIfGp_getReverb(stayNo)/127` (0 when stayNo==0). Log: `[ExtSeq] §93 voice fxmix:
music_send=… room=… reverb_s8=… voice_send=…`. User re-listen for echo. Bus: §93.

## §88 / Engine CharVoice message-open (2026-07-22)

Wired package `voice/` cues at `dExtWw_handleDemoMessage`: SE 0x481F / wave(port&0xFF); 0x359 silent. User full-scene listen next. Bus: §88.

## §84 / Engine type-7 + SoundTable vol (2026-07-22)

Ported Bridge `control/` CSVs: type-7 → DuskDsp freeverb + AutoMixer send; SoundTable `vol_over_127` → ExtSeq master. Runtime chases paused. Bus: §84.

## §111 / acceptance drives (2026-07-23)

External launcher + `verdict.ini` log assertions. Scrubs `DUSK_DRIVE*`. No `src/` drive
code. Tool **0.33.0**.

```bat
python -m ww_bridge accept --ini verdicts\example_265.ini --launch
```

## §110 / §110b loss-protection (2026-07-23)

Housing: BOTH — mod local-only git `cb6ba2b` + snapshot; receiver local `6c5e9cd4e5` (no push).
Bridge handoff: renumbered bus §108→§110 (collision with nledger); init local-only git for
`albt bridge`; tip/bus follow-up commit. Push still gated on Ivan + gate CLEAN + user go.

## §109 / predictions-as-code verdict (2026-07-23)

Log-pattern engine. Per-build `verdict.ini` by shipping lane (Engine / History).
Tool **0.32.0**. Example: `verdicts/example_265.ini`.

```bat
python -m ww_bridge verdict --ini verdicts\example_265.ini
```

## §108 / №-ledger-as-database (2026-07-23)

Mechanical half of Housing order #1. Schema + JSONL ingest + generated views.
**246** unclassified drafts extracted; History classifies (`status` / supersedes / summary).
Tool **0.31.0**.

```bat
python -m ww_bridge nledger extract
python -m ww_bridge nledger build
```

See `albt bridge/ledger/SCHEMA.md` + `reports/nledger_unclassified.md`.

## §101c / SC_01_mizu* TEV/blend/vertex-alpha ref (2026-07-22)

Reference sheet for Engine system-4 fidelity. 7/8 XLU `BM_BLEND` SRCALPHA×INVSRCALPHA;
matSrc=VTX; RASA/COLOR0A0 on key stages; VTX1 Color0 present; Z-write off on XLU.
Tex slots corrected to MAT3 +0x84 (§99 CSV had texGen @+0x28). Tool **0.30.2**.

Artifacts: `reports/model1_mizu_mat_ref.md` + `model1_mizu_mat_ref.csv` +
`model1_mizu_tev_stages.csv`.

```bat
python -m ww_bridge model1-dump
```

## §99c / model1 beach water dump (2026-07-22)

`model1.bdl` = 8× `SC_01_mizu*` + `Txa_nami_*`/`umi_*`; BTK 16 tracks @100f. System-4
**SUPPORTED**. Tool **0.30.1**. Fidelity next, not a new port.

## §97c / waterline inventory + BTI index (2026-07-22)

Outset rooms: 0 water-named members; `model1.bdl`+`btk` present (unclaimed). Foam BTIs
**0x8B/0x8C** index-PASS. Tool **0.30.0**.

```bat
python -m ww_bridge waterline-inv
```

## §93b / type-6 ≠ getReverb (2026-07-22)

Type-6 `0x04` = SeMgr category concurrency (4 slots/cat). Voice reverb = stage RTBL
`field_0x1&0x7F`. Tool **0.29.1**. No hand-tune from type-6.

## §90b / IsleLink payload staged (2026-07-22)

`IsleLink_0.aw` (296544) + `ibnk_217`/`wsys_217` + cue map (25/28/26/27). Tool **0.29.0**.
Cues provisional pending ordering-listen. AppData mod only.

```bat
python -m ww_bridge voice-map --islelink-payload
```

## §89c / vanilla five-pack (2026-07-22)

`local_diag/charvoice_five/`: `CharVoice_0_wave094`…`097` + `_wave105` + CSV. Tool **0.28.1**. Fan labels never in filenames.

```bat
python -m ww_bridge voice-diag --five --open
```

## §89b / decoder + dayjo corr — Aryll in IsleLink (2026-07-22)

Decoder PASS; neutral `wave000`…`159` in `local_diag/`; dayjo Aryll 0/9 in CharVoice, **8/9 in IsleLink_0.aw** (Hoy1→025). Tool **0.28.0**. Bus: §89b. Files never repo/package.

```bat
python -m ww_bridge voice-diag
```

## §88b / CharVoice listen pack (2026-07-22)

Decoded waves 16–19, 104–107, and keyRgn 132/133 (prog 0x19) from staged CharVoice to PCM WAVs. Tool **0.27.0**. Bus: §88b.

```bat
python -m ww_bridge voice-listen --open
```

## §86b / CharVoice payload staged — Bridge CLEAR (2026-07-22)

Staged `CharVoice_0.aw` (1,057,984) + `ibnk_135.bin` + `wsys_135.bin` under mod `voice/`. Bank id is inner **135** (not AAF cid 1). Tool **0.26.1**. Bus: §86b. Bridge clear.

```bat
python -m ww_bridge voice-map --payload
```

## §85c / voice-map Aryll opening SE chain (2026-07-22)

INF1 `mInitialSound` for `0x357/358/050/359/35A` → DOL `charVoiceTable` → `JA_SE_CV_COMMON_PEOPLE` (`0x481F`) + port → WSYS/IBNK inner **135** → `CharVoice_0.aw`. Sounds 104–107 (0x359 silent); `0x050` anim=5. Tool **0.26.0**. Bus: §85c.

```bat
python -m ww_bridge voice-map --stage
```

## §81b / aaf-control type 6/7 + SoundTable BGM (2026-07-22)

Decoded the three §81 how-to-play tables. Type 7 Fxline: 2 scenes × 4 lines, all enabled; scenes differ on L2/L3. Type 6: mostly `0x04`. SoundTable BGM: 97/97 offsetNo↔JaiSeqs MATCH; Outset vol_u8=60. Tool **0.25.0**. Bus: §81b.

```bat
python -m ww_bridge aaf-control --stage
```

## §78c / wav-compare + live-ab defects (2026-07-22)

Three §78 defects: zero-frame WARN+exit3; loop-period tempo (~1.3% class); `--b-start`/`--a-start` + onset-density table. Tool **0.24.1**. Bus: §78c.

## §77e / live-ab + wav-compare Bridge implementation (2026-07-22)

§77 offline + §77b live analyzer. Shipped:

```bat
pip install -r requirements-audio.txt
python -m ww_bridge live-ab --list-devices
python -m ww_bridge live-ab --device-a N --device-b M --tag ilinkin
python -m ww_bridge wav-compare A.wav B.wav
```

- Dual WASAPI **device** loopback; live 8-band meters + DIVERGE; auto-WAV → `Videos\WW comparisons\round2\`
- Same session feeds §77 `wav-compare` (silence-align, tempo, band envelopes, onset deltas, verdict)
- Setup/privacy: `reports/live_ab_setup.md` (mode a routing; no screen capture)

Tool **0.24.0**. Bus: interconnected §77e.

## §72b / seq-map Bridge implementation (2026-07-22)

§72 unblock: BGM id→BMS (Lago first). JaiSeqs.arc is **alphabetical**, not id-ordered — rule is Lago id → filename → open member.

```bat
python -m ww_bridge seq-map --extract
```

| id | symbol | bms | vs base |
|----|--------|-----|---------|
| `0x38` | `ISLAND_LINK_0` | **`i_linkin.bms` 19552** | **DISTINCT** |
| `0x0E` | `ISLAND_LINK_2` | `i_link2.bms` 13088 | byte-identical to `i_link` |
| `0x55` | `ISLAND_LINK_3` | `i_link3.bms` 9920 | DISTINCT |
| `0x18` | `HOUSE_G` | `house_g.bms` 3584 | DISTINCT vs `house` |

Twin OK on all four extracts into `audio/ww_jaudio1/`. Tool **0.23.0**. Bus: §72b.

## §70c / stream-map Bridge implementation (2026-07-22)

§70 ask: stream-id→`.afc` (LagoLunatic first). Finding: Lago `BGM Sequences.txt` is **sequence-only**. Authority = `JaiInit.aaf` BSM (AAF type 5).

```bat
python -m ww_bridge stream-map
```

- **75/75** `JA_STRM_*` ↔ BSM names; all `.afc` on disk under `Audiores/Stream/`
- **`JA_STRM_PROLOGUE` → `e3title.afc`** (idx 0) — **not** `1tale.afc`
- **`JA_STRM_DEMO_01_01` → `1tale.afc`** (idx 36); `TITLE` → `title.afc`; `BPW_START` → `bp_start.afc`

Tool **0.22.0**. Bus: interconnected §70c Bridge response.

## §C.1b / seq-events set_param target Bridge implementation (2026-07-22)

Housing §C.1 Bridge ask: mirror target+value in golden stream. Shipped:

- **`set_param`**: `note_param`=target (`flag`), `velocity`=`data/32767.0` (6 dp)
- **i_link:** 1080/1140 targets are **0** (volume-dominated)
- Engine dump emit still open — format documented for mirror

Tool **0.21.0**. Bus: interconnected §C.1b Bridge response.

## §64b / ibnk-vol-check Bridge implementation (2026-07-21)

Housing §64: is `inst_volume` constant or a fallback? Bridge shipped two independent readers:

- **`python -m ww_bridge ibnk-vol-check`**
- BANK-table + magic-scan both see literal `3f800000` on every INST (36+7)
- **Reading (1) confirmed** — not a decode fallback; soft layers not explained by INST vol

Tool **0.20.0**. Bus: interconnected §64b Bridge response.

## §61b / gate №31-C Bridge implementation (2026-07-21)

Auditor: empty greplist → false CLEAN; silent greplist fallback. Bridge shipped:

- **`GATE INCONCLUSIVE`** + exit **3** when `npat == 0`
- Always print `greplist=<resolved path>` with `exe=`
- Narrowed `arc_contents.py` `_load_arc_index` except

Tool **0.19.0**. Bus: interconnected §61b Bridge response.

## §60b Engine — re-dump MATCH (2026-07-21)

`dusklight --extseq-dump …/ww_jaudio1` → engine CSVs **byte-identical** to Bridge (`i_link` 5215, `house` 1978). openTrack 8-level warn shipped. Drift (if any) = new target.

## §60 Engine response (2026-07-21)

Check 1/2 answered. **No live parser fix** — the wait is the BMS song-loop target.

- `i_link`: `@69 wait 12480; @72 jmp 69`
- `house`: `@42 wait 7680; @45 jmp 42`
- Dump had re-entered once (looked like “extra wait”); Bridge `pc_call_seen` never re-enters.
- Dump now stops on first backward jmp. Playback still follows forever (WW-correct).

Re-dump with `DUSK_EXTSEQ_EVENT_DUMP=1` to confirm parity.

## §59 / ASK 17 Engine dump companion (2026-07-21)

Housing ask: stop hypothesising; mechanical diff. Bridge golden already on disk. Engine shipped:

- Flag: **`DUSK_EXTSEQ_EVENT_DUMP=1`** (once at first `startOwned`)
- Offline walk via **`Ja1Parser`** (scratch tracks; no audio; no Bridge code)
- Writes `audio/ww_jaudio1/seq_events_engine_{i_link,house}.csv`
- Columns: `tick,track_id,event,note_param,velocity` (per-track local ticks; backward jmp once; loop cap 256)

**No playback / rate / velocity change.** Diff against Bridge `seq_events_*.csv`; disagreement = bug line.

## §59 / ASK 17 Bridge implementation (2026-07-21)

Housing §59: stop hypothesising; independent decoder for mechanical diff. Bridge shipped:

- **`python -m ww_bridge seq-events`** — fresh walker from §50b + WW Arglist/cmdNoteOn
- **No shared code** with `Ja1Parser` or `vel_calib`/`tempo_map`
- Columns: `tick, track_id, event, note_param, velocity`
- `i_link` 5215 events / 731 note_on · `house` 1978 / 314
- Artifacts: `seq_events_{i_link,house}.csv`

Tool **0.18.0**. Bus: interconnected §59 Bridge response.

## §55 / ASK 16 Bridge implementation (2026-07-20)

Housing §55: exponent cleared by measurement; extract BMS tempo/timebase. Bridge shipped:

- **`python -m ww_bridge tempo-map`**
- Defaults: tempo 120 / timebase **48**
- **Both** `i_link` and `house` set timebase **120** at tick 0; tempos 119 / 146; no later changes
- Hint: if engine kept default 48 vs specified 120 → ~2.5× too slow (matches fault class)

Tool **0.17.0**. Bus: interconnected §55 Bridge response.

## §54 / ASK 15 Bridge implementation (2026-07-20)

Housing §54 ASK 15: offline inputs for balance calibration. Bridge shipped:

- **`python -m ww_bridge vel-calib`** — per-track BMS velocity hist + IBNK initVol/vel-region CSV
- **i_link:** 731 scored · span 20–124 · **house:** 314 · span 47–115 (narrower — supports interior hypothesis as input fact)
- **initVol:** INST volumes all 1.0; resolved dips only on bank0 prog 9/24 (~0.80) — compounding hypothesis weak
- Does **not** infer WW curve (ASK constraint)

Tool **0.16.0**. Bus: interconnected §54 Bridge response.

## §53b / bank-map Bridge implementation (2026-07-20)

Housing §53: engine owns residency; Bridge optional if offline preprocess needed. User routed Bridge; shipped:

- **`python -m ww_bridge bank-map`** — parse staged `aaf_slices/` → `bank_waves.csv` + `bank_programs.csv`
- **vir 0:** 51 waves / 36 programs → `n_zelda_0.aw`
- **vir 2:** 8 waves / 7 programs → `n2i_link_0.aw`
- Engine still owns shadow register / `noteOnOsc` replace / №89 nest.

Tool **0.15.0**. Bus: interconnected §53b Bridge response.

## §52b / path (B) Bridge implementation (2026-07-20)

Housing §52: (B) destination packaging. Bridge shipped:

- **`python -m ww_bridge bgm-package`** — raw `i_link`/`house` BMS + `n_zelda`/`n2i_link` `.aw` + IBNK/WSYS slices + `manifest.ini`
- Stage: live mod `audio/ww_jaudio1/` (AppData; not repo). Inert until engine JA1 hook.
- (A) retarget probe **not** built; must not land in that folder.

Tool **0.14.0**. Bus: interconnected §52b Bridge response.

## §50b / ASK 13.1 Bridge implementation (2026-07-20)

Housing §50: dialect before packaging. Bridge compared WW `parseSeq` vs JA2 `parse` (dispatch, not greps).

- **Verdict: `incompatible`.** Waits/perf/bank encodings conflict; `0xA4` bank is NULL on JA2.
- Item 2 raw BMS packaging **blocked**. Recommend ASK: retarget (A) or dual parser (B).
- Artifact `reports/bms_dialect.md`. CLI: `python -m ww_bridge bms-dialect`.

Tool **0.13.0**. Bus: interconnected §50b Bridge response.

## §48b / ASK 12 Bridge implementation (2026-07-20)

Housing §48 recommended BMS instrument ownership before conversion. User greenlit. Bridge shipped:

- **`python -m ww_bridge seq-banks`** — BMS `writeRegParam` 0x20/0x21 → vir→IBNK→WSYS→`.aw`
- **`ISLAND_LINK`:** `i_link.bms` → banks 0+2 → **`n_zelda_0.aw` + `n2i_link_0.aw`**
- **`HOUSE`:** `house.bms` → bank 0 → **`n_zelda_0.aw`**
- ASK 11 `IsleLink`/`Ojhous` name-hints corrected: area SE, not BGM instruments.
- Artifact `reports/seq_banks.md`.

Tool **0.12.0**. Bus: interconnected §48b Bridge response.

## §47 / ASK 11 Bridge implementation (2026-07-20)

Housing §47: audio research before any conversion. Bridge shipped:

- **`python -m ww_bridge audio-map [--bgm …]`** — parse `JAZelAudio_BGM.h`; class via `JAISoundID_Type_*`.
- **Split CONFIRMED** (header comments + type masks + stream dispatch) — not count-correlation alone.
- Default: `ISLAND_LINK` + `HOUSE` both **`sequenced_portable`** (`IsleLink_0.aw` / `Ojhous_0.aw` + `JaiSeqs.arc`).
- Streams → `stream_needs_transcode` (`.afc` heuristic). No conversion; №28 B10 twin ban noted.
- Artifact `reports/audio_map.md`.

Tool **0.11.0**. Bus: interconnected §47b Bridge response.

## §46 / ASK 10 Bridge implementation (2026-07-20)

Housing §46: stop hand-transcribing island work-docs. Bridge shipped:

- **`python -m ww_bridge space-report Outset`** → `docs/WW Linked/islands/Outset/inventory.md`
- Pure join over day1-guard / beh / exceptions / coverage / ledger / quest-triggers / EVNT / live vs golden.
- README points at inventory; prose not overwritten. IVAN RULE on prefix families.

Tool **0.10.0**. Bus: interconnected §46 Bridge response.

## §44 / ASK 9 Bridge implementation (2026-07-20)

Housing §44: day-1 foundation guard. Bridge shipped:

- **`day1-guard <log>`** — census×layers×exceptions×spawn ledger; exit 2 on drift.
- Reviewed `goldens/day1_exceptions.ini` (kusax/swood). Sample log: **2 missing `bridge`**.
- **`identify <code>`** — §40 dossier join (no invented names).

Tool **0.9.0**. Bus: interconnected §44 Bridge response. Engine §41 / auditor §43 not Bridge.

## §38 Bridge implementation (2026-07-20)

Housing §38: ASK 7 animations + ASK 8 behaviour GREENLIT. Bridge shipped:

- **`anim-ledger`**: 34 Outset arcs; Kb **20 slots = 20 members** (gap closed via full BAS/BCK/BTP header).
- **`beh-ledger`**: inert=19 · minimal=24 · **7** `inert_but_portable` (item/bridge/lwood…).
- Report-only / suggest-never-fill. Tier 3 semantic mapping not attempted.

Tool **0.8.0**. Bus: interconnected §38 Bridge response.

## §36 Bridge implementation (2026-07-20)

Housing §36: EVNT table on disk (Yaz0 Stage.arc). Bridge shipped name resolve:

- **`python -m ww_bridge evnt --lookup 1 15 53`** → `STOLENSISTER` / `AJ_SPEAK` / `PUROLO_RETURN`
- `quest-triggers` auto-loads Ex WW `sea/Stage.arc`; success test **PASS** (3/3 named).
- No invented names. Artifact `reports/sea_evnt.csv` (57 entries).

Tool **0.7.0**. Bus: interconnected §36 Bridge response.

## §34 / ASK 6 Bridge implementation (2026-07-20)

Housing §34: decode Tag* quest triggers. Bridge shipped:

- **`python -m ww_bridge quest-triggers`** — Outset report-only.
- 25 placements / 9 names; all decomp sources via `d_stage.cpp` OBJNAME aliases.
- **Verdict: hybrid** — data selectors (eventNo / message / switches) + TagEv type code paths.
- Success test **PASS (partial):** msg `0x9c5`,`0x168b` · eventNo `1,15,53`. No EVNT in adapted Outset.arc.

Tool **0.6.0**. Bus: interconnected §34 Bridge response. §33 verify change **not built**.

## §30 Bridge implementation (2026-07-20)

Housing §30: socket ledger GREENLIT. Bridge shipped:

- **`python -m ww_bridge ledger`** — Outset JOIN census × actor_map × ASK3 × ASK4.
- Report-only / suggest-never-fill. Artifact `reports/outset_socket_ledger.csv`.
- Success test: **5** `unfilled_candidate` (SalvagE/Salvage/Salvag2/Akabe/HyoiKam) — better work order than skip-list restatement.
- Live `lwood` already `alwd.bdl` → ledger `filled_ok` (content ratified №118 off-bus).

Tool **0.5.0**. Bus: interconnected §30 Bridge response.

## §29 / B2 Bridge implementation (2026-07-20)

Housing §29: ASK 1–5 ACCEPTED; B2 unblocked. Bridge shipped:

- **`python -m ww_bridge scls <arc> --out exits.csv`** — SCLS → CSV (ASK 1 struct).
- Outset 10 exits + interior→`sea` room 44 returns proven vs №56.
- Does **not** overwrite live `doors.ini`.
- ASK 5 refuse exit code **2** confirmed (age 29≠30).

Tool **0.4.0**. Bus: interconnected §29 Bridge response.

## §28 Bridge implementation (2026-07-20)

| Ask | Where | Proof |
|-----|-------|--------|
| ASK 4 | `ww_bridge/arc_contents.py` → `verify` | Live mod: 3 real FAILs (LinkUG/Pjavdou `model1.bdl`, Lwood `lwood` vs `alwd`) |
| ASK 5 | sibling `diag/` (`python -m diag symbolize`) | Fail-closed on age 29≠30; matching smoke → `mDoExt_J3DModel__create` |

Full write-up: interconnected bus §28 Bridge response. Tool **0.3.0**.

## §26 Bridge implementation (2026-07-20)

Sibling tool (`C:\Users\xxxxx\Documents\albt bridge`), decomp `D:\XXXXXXX\WW DP`.

| Ask | CLI | Artifact |
|-----|-----|----------|
| ASK 1 | `python -m ww_bridge chunk-ref` | `reports/chunk_semantics.md` |
| ASK 3 | `python -m ww_bridge coverage --census goldens/outset_placements.csv` | `reports/actor_coverage.csv` |
| ASK 2 | `python -m ww_bridge decode-params --census … [--annotate-out …]` | `reports/params_decoder.md` |

**ASK 1:** Decomp-confirmed roles; corrected auditor guesses — **LBNK ≠ lights**, **RPPN ≠ NPCs**.
**ASK 3:** Outset golden `87` names → `31` present / `56` absent (file heuristic; not match%).
**ASK 2:** `11/87` names with scraped `GetParam` fields; annotate adds `params_decoded`.

Full write-up posted on the interconnected bus (§26 Bridge response).

## Sign-off mirror

| Party | Status | When |
|-------|--------|------|
| Housing Security | §52 (B) destination; awaiting engine | 2026-07-20 |
| Bridge (Cursor) | bgm-package 0.14.0; dialect 0.13.0 | 2026-07-20 |
| User | content/F-1/F-2 routing | 2026-07-19 |
| History | **not in lane** | — |
