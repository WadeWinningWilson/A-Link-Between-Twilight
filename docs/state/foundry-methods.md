# Foundry — live state

**Status:** LANE LIVE (2026-07-28). Charter posted ([Foundry.md](../Foundry.md)); DuskTap
ownership transferred Housing→Foundry. Sprint 1 open. **F1 BLOCKED ON A REAL BUG:** the
user's first reels came back header-only (0 input samples). Diagnosis so far (§201): WW
polls the pad at 120 Hz from ~2 s after boot (probe-verified), user's procedure and configs
exonerated, DuskTap patch exonerated (BreakPoints/PowerPC only), no documented upstream
regression; `hmm.dtm` shows exactly ONE poll recorded then silence → recording-start kills
or bypasses SI-poll capture in current master. FIVE DuskProbe logs are now BUILT INTO our
Dolphin (SI attach / poll cadence+movie state / frame counters / BeginRecording / first
RecordInput). GUI automation could not trigger recording headlessly (4 attempts; menu
accelerator + hotkey routes both failed silently) — one 60-s USER mission with the
instrumented build will name the failing branch.
**UPDATE 2026-07-29 (§202): RECORDING BUG FIXED.** User's attempt left probe data → root
cause = unreported Dolphin-master regression (BeginRecordingInput's static-header config
layer sets all SI ports to NONE because SaveToDTM never fills the controllers byte). Fixed
in our tree (`DuskFix` in Movie.cpp: mirror live roster into header), rebuilt, mechanism
validated via playback probes (header roster → port attach type 6 → polls). **F7 DONE:**
`tools/foundry/dzr_census.py` works — Outset Room44 + sea Stage fact sheets in
`docs/WW Linked/outset-*-census.md` incl. TRUE per-story-layer ACTR rosters.
**UPDATE 2026-07-29 (user ruling): DOLPHIN REEL WORK PARKED.** Too much lane time spent.
Final status of the parked branch: recording fix VERIFIED capturing (60-s run, 7k+ inputs,
attract demo reached); remaining defect = auto-export lambda needs Core::CPUThreadGuard
(patch written in MainWindow.cpp, UNBUILT). Manual DuskTap capture sessions (the proven
path) remain the donor-runtime instrument; reels/P5 resume only on user ask.
**UPDATE 2026-07-29 (§204): P6 SWEEP DONE** — full-game fact-sheet library at
`docs/WW Linked/fact-sheets/` (156 stages, 705 arcs, 11,826+1,283 ACTR placements, md+JSON;
3 shift_jis failures on dev stages queued). vexp verdict: crash fixed for good (128 MB
stack) but exclude_patterns is dead in daemon 2.3.1 and the FREE plan caps at 2,000
nodes/1 repo/20 calls-day → 3% coverage is the vendor ceiling; excludes left configured
for a future fixed daemon.
**Next (Dolphin-free):** P2 schema v0 + differ over the EXISTING capture corpus · F4
oracle-stack doctrine doc · shift_jis decode fallback in dzr tools · History cross-check
of per-layer censuses.
**Mission (60 s):** launch the DuskTap Dolphin → select WW in the list (do NOT boot) →
Movie → Start Recording Input → press buttons/stick from the very start, play ~60 s →
Movie → Export Recording (anywhere) → quit Dolphin normally → tell Foundry. The log does
the rest.
**Do-not:** no content work in this lane (no donor diffing for fixes, no restoration code);
taps observe, never modify donor behavior (§189 READ-ONLY policy); no self-clearing audits —
Housing negative-controls every instrument before it becomes a verdict source.

---

## Sprint 1

| # | Item | Owner | State |
|---|------|-------|-------|
| F1 | Capture-reel SOP + determinism checklist + 2-min validation mission | Foundry (SOP done) → user (run it) | SOP BELOW; awaiting run |
| F2 | Probe-event schema v0 + converter for existing DuskTap logs + first differ; pilot = windline 0x31 donor rate profile (§196) vs receiver | Foundry; needs Engine DuskLog hook (spec below) | **P2 v0 SHIPPED (§206)**: schema+converter (`dusktap_to_jsonl.py`, reproduces §196 censuses exactly) + general differ (`probe_differ.py` census/seq/profile; census CALIBRATED — independent-boot MATCH at 3%; UNKNOWN-on-empty) + **windline donor profile computed** (`docs/WW Linked/windline-donor-profile.md`: plateau ≈1.7/s, peak 2.1/s); doc `docs/gaming systems/debugging-methods/p2-mechanized-parity.md`; BGM differ (§P2-audio) = specialized exemplar. **Receiver tap LANDED §207** (user un-gated): `dPa_emitterTapLog` in d_particle.cpp, both call sites, `DUSK_EMITTER_TAP=1` toggle, built + caches wiped; verdict closes on the user's ~5-min WW-Outset-exterior port session. **VERDICT DELIVERED §209: W-LINE density DRIFT(74%)** — port 0.45/s flat vs donor plateau 1.77/s varying; same run caught 0x89D7 TP-grass STILL LIVE (V-d unapplied, d_a_ext_vegetation.cpp:738, 558×) + donor ambients 0x24/0x2022/0x429 missing + TP-id triage list; 3 ferries queued (Engine ×2, History ×1). P2 loop CLOSED |
| F3 | Golden-candidate #1: tale-scene donor trace (§191, 13,052 lines) → becomes golden when the receiver tale is accepted | Foundry (bookkeeping) | SEEDED |
| F4 | Oracle-stack doctrine → durable doc in `gaming systems/debugging-methods/` | Foundry | **SHIPPED §208**: `oracle-stack.md` (stack, rules, case receipts) |
| F5 | Headless harvest harness: DolphinNoGUI + .dtm playback + auto-exit at movie end (check stock flags before patching) | Foundry | OPEN |
| F6 | External-resource intake ([Foundry-Intake.md](../Foundry-Intake.md)): T1 ADOPTED + pilot PASSED 8/8 (`tools/foundry/jpc_crosscheck.py` reproduced §192 facts); Python 3.12 venv `D:\XXXXXXX\foundry-py312` (gclib is 3.14-incompatible — do not use under 3.14); wwrando @ `D:\XXXXXXX\wwrando`; decomp pulled to be8da68 | Foundry | T1 DONE; T2 evals open |
| F7 | DZR census pilot (wwlib dzx.py on Outset rooms) — P6's first fact-sheet ingredient; cross-check vs History census + next spawn-tap capture | Foundry | OPEN (next) |
| F8 | **P9-P13 game-systems parity program** ([Foundry-P9-P13.md](../Foundry-P9-P13.md), §210-§211): covenant laws bound (donor-verbatim · inference-flag · naming-agnostic). **P9 first verdict SHIPPED** (`outset-spawn-verdict-20260729.md`: 16/37/5/36 rows, layered rows correctly deferred) · **P11 SHIPPED** (`donor-transition-graph.md` 1,955 edges; receiver check awaits socket→donor-stage map — History/Engine) · **P10 SHIPPED+VALIDATED §212** (`layer_oracle.py`: donor law verbatim+cited, real-.gci checksum-validated 6/6 slots; P9×P10 join resolved the 36 flagged rows → 16 new MATCHes, Tag*/Salvage liberty clusters, 4 real cast gaps, layer-divergence candidates; ba.*↔UNK_* mapping = History) · **P12+P13 SETTLED §213** (`path_maps.py`: 1,821 paths/13,441 waypoints game-wide + dPath tap spec, RPAT→PATH conversion = Bridge Q; `state_map.py` + Bokoblin worked example: 24-action dispatcher, 232 cited transitions). **PROGRAM COMPLETE — all five domains instrumented**. **§214: multi-type identity fixed** (355/672 names; (name,params) keys, auto-flagging, `param_map.py` + Ikada example; Engine: +params on census line) · **P14 DONOR LAW COMPLETE §215** (EnvR/Colo transcribed from d_stage.h; full chain 52 env → 10 pselect → 57 Pale in `donor-palettes-sea.md`; dKyd_Schedule transcription pending) · **BTK fast-track DESIGNED §215** (`btk-native-fast-track.md`: sibling-BTK auto-binder on mDoExt_btkAnm — model1.btk already verified active §128; DUSK_BTK_TAP harness; Maya-mode [INFERENCE]; J3DUltra/noclip staged T3) **§219: DUSK_BTK_TAP LANDED+BUILT** (both sites: scene1/model1 + §218 socket; calcTransform SRT vocabulary = Bridge CSV-diffable; baseline mission = 1-min shore stand w/ DUSK_BTK_TAP=4) · **P13b NPC AI chartered** (WW NPC idiom = set_action function-pointer states; state_map extracts both idioms; Grandma+ko1 worked examples; NPC path binding per-actor [INFERENCE]) · Pig receipt: blocked-on-bait = named prerequisite **§220: BTK BASELINE = MATCH BY MEASUREMENT** (50,656 samples; 2-key/const components EXACT 0.0000; 12 flags = evaluator Hermite artifact, refinement queued) · **pig↔bait contract SPEC'D** (`donor-pig-bait-contract.md`: d_a_esa decompiled, fpcM_Search criteria cited, receiver primitive verified — pig now implementable) · NPC cast statemaps generated (thin-shell class → header-inline pass queued) **§221: HERMITE VERDICT CLEAN MATCH** (donor JMAHermiteInterpolation verbatim + real TTK1 tangents via gclib; ±0.005/±8-s16 tolerance; all 12 artifacts retired — scene1 donor-EXACT; harness permanent: `btk_baseline.py --btk <arc>`) · **NPC INDEX game-wide** (`donor-npc-index.md`: 59 NPCs — 10 RICH/31 MOD/18 THIN; Outset sorted first; state_map multi-file .cpp+.h) **§222: P14 donor law CLOSED** (dKyd_Schedule transcribed — full chain weather→time→slot→Pale; receiver differ waits on one kankyo tap) · **P13c doctrine: WW enemies 86-92% receiver-native by measured API surface** (`api_surface.py`; kb 92.3/bk 85.8/cc 86.7; direct-port default; ~6 shared shims amortize roster-wide) **§223 gap assessment** (`port-gap-assessment.md`): ship-wave = §209 ambience solved (ikada:327 SHIPWAVE00 0x37 family, all in staged common.jpc → Engine ferry) · `cc_map.py` shipped (donor dCcD_Src blocks verbatim; DN-1-safe law split) · shadows mostly present (I3 simple; tier = liberty) **§224 History's 3 requests DELIVERED+VALIDATED**: api_surface v2 (free/method:Class kinds + verbatim cited signatures — the shim strategy reads off the sheet) · state-tap chain (`include/dusk/state_tap.hpp` → converter → `state_gate.py`; synthetic validation incl. per-actor discrimination) · BONUS third idiom: anonymous state fields (kb m420 — pig's real machine: 5 banks, 34 transitions) **§230 ENEMY PORT KIT** (`enemy_port_kit.py`: one command → surface/statemap/ccmap/parammap + res-manifest pre-flight + WW→TP rename dictionary (accumulating, §229-seeded) + AUTO-GENERATED shims_skeleton.h in History's Pass-1/Pass-2 architecture + §229-recipe README; **Bokoblin kit generated** at `port-kits/bk/` — 10 free/13 adapters/2 unknown; cxxheaderparser staged T2 on first-garbled-stub trigger). **§231 refined from render/crash laws**: pattern audit (bk L5013 setUserArea((u32)) pre-caught — the pig's AV class), res-routing law column (DN-3 per res type), README render triple + 64-bit law; holding on pig playtest + duskStateTap slice → then §224 gate. **§232 KIT DOGFOOD (Bokoblin, real standalone compile): analysis tier AUTHORIZED; drop-in tier WITHDRAWN pending v3** — 3 measured gaps: dependency closure (10 donor-only headers incl. base-class d_a_boko.h), presence≠equivalence (receiver c_damagereaction.h = stub; donor's holds damagereaction/enemyfire/enemyice = the state core), type-blind skeleton; round-1 errors 112 ≈ pig 101. Shared-shim economics: donor c_damagereaction types = one header, whole combat tier **§239 v4 piece 1: ISLAND ROSTERS** (island_roster.py: placements × l_objectName × g_profile files; Outset 89/0-unmapped + Windfall 63; kit step -1 = island check — the near-miss guard) · **§238 pig E2E COMPLETE + NPC gate closed** (duskStateTapS + state_gate --npc validated; CARRY/WATER laws absorbed; island ruling: RICH NPCs direct-port now, shells stay socketed, kit v4 = NPC integration manifest before campaign) · **§236 BMT CENSUS** (pig-shade gap class mechanized: `bmt_census.py` + inline in kits; 6/21 material-only; bk+mo2 exposures pre-caught; ko02 mirror case) · **§235 kit actor-agnostic** (Grandma 83.2%; NPC res idiom added; string-state gate = open NPC gap) · **§234 MOBLIN GENERALIZATION TEST: v3 PREDICTIVE on a cold second enemy** (3 closure-driven compile rounds; every residual error class pre-named by an artifact; meter 86→0 via stub pattern, damagereaction staging resolved the state core; shared-shim set = 3-enemy evidence; kit AUTHORIZED analysis+prediction tier) · **§233 KIT v3 SHIPPED+VALIDATED**: closure.md (recursive include walk — all 10 §232 hand-staged headers auto-found incl. base class), symbol equivalence (c_damagereaction acceptance case EXACT: damagereaction/enemyfire/enemyice; + JntHit_c/CcAtInfo real finds), typed skeleton (auto forward-decls + [TYPE-GAP]); README step 0. Drop-in claim re-earnable on first real v3 port | Foundry | kankyo tap → palette differ standing; §224 gate on pig taps |

**⚠ P1 DESIGN CORRECTION (intake intelligence):** WW is a known Dolphin movie-desync
offender (no published WW TAS exists; TAS community uses special builds). Reels are
validated-per-reel, never assumed; **save states are the primary re-harvest asset**
(build-pinned = fully reliable); short from-state movie segments > long boot-anchored
movies. Fallback if F1 fails on our build: Lobsterzelda's lua-support Dolphin (intake T2).

## F1 — Capture-reel SOP (P1; effective immediately)

**Goal:** every vanilla session becomes a permanent, re-harvestable asset. Play once, on
record; replay forever with whatever taps future questions need.

**Determinism checklist (record AND replay must match):**
- Local DuskTap Dolphin build only (save states + movies are build-pinned; ours is pinned).
- Dual Core **OFF** (thread races desync movies) · JIT recompiler · same DSP engine both
  sides (HLE) · cheats/panic handlers identical (off).
- Start recording **from boot** (Movie → Start Recording Input before launching GZLE01), or
  from a save state saved *with* the movie. Keep a copy of the memory card `.raw` beside the
  reel — save-data divergence is the classic silent desync.
- On session end: Movie → Export Recording.

**Library convention:** `D:\Dolpheen Plz\captures\reels\<area>-<beat>-<YYYYMMDD>.dtm` +
paired save states + memcard copy; one line per reel in a `reels-index.md` beside them
(route description, story flags at start, taps that were live).

**Validation mission — exact steps (user does 1-8; Foundry does the replay+diff):**
1. Back up the memory card FIRST: copy `<Local Dolphin>\build\Binaries\User\GC\USA\Card A.raw`
   to `D:\Dolpheen Plz\captures\reels\f1-premovie-CardA.raw` (replay must start from the
   identical save; this is the classic silent desync).
2. Launch the DuskTap `Dolphin.exe`. Config → General → **Enable Dual Core OFF**. Debug UI +
   Write-to-File logging on as usual (§190).
3. Do NOT start the game. Select GZLE01 in the game list, then **Movie → Start Recording
   Input** — this boots the game with recording anchored at boot.
4. Play ~2 minutes on Outset: walk grass, talk to one NPC, one door in/out. Variety > length.
5. **Movie → Export Recording...** → save as
   `D:\Dolpheen Plz\captures\reels\outset-f1-validation-20260728.dtm`. Then stop emulation.
6. Copy `User\Logs\dolphin.log` → `D:\Dolpheen Plz\captures\reels\f1-record.log`.
7. (Any session, standing habit:) save states at each beat regardless — states are the
   primary asset per the P1 correction above.
8. Tell Foundry the files exist. Foundry then: restores the memcard copy, runs
   `Dolphin.exe -m <reel.dtm> -b` headless-ish, harvests the replay log, strips timestamps,
   diffs. Identical tap streams ⇒ reel pipeline GREEN. Divergent ⇒ P1 pivots to
   save-state-first + Lobsterzelda-build evaluation (intake T2) — the program survives
   either verdict.

## F2 — DuskLog hook spec (FERRY → Engine, when convenient; not urgent)

For the P2 windline pilot, the receiver needs ONE choke-point log line mirroring the donor
tap: at the receiver's emitter-creation path (our `createSimpleEmitterID`-equivalent /
`dPa_wwWindlineResRM` resolve site), emit
`DuskLog emitter t=<frame> id=<resid hex> pos=<x,y,z>` behind a settings toggle (default
off). That single line + the §196 donor census (windline 0x0031 ×500 rate profile) = the
first computed vanilla-law verdict. Schema v0 (JSONL: `{t, site, name, args, derefs}`) and
the log→JSONL converters are Foundry-built — Engine only lands the one line.

## §P2-audio — BGM note-fidelity differ (SHIPPED 2026-07-28)

The first realized P2 differ, over the **existing** BGM-note capture corpus (no new Dolphin run
needed to build/validate the instrument). Answers: *does the port's ExtSeq player perform the same
NOTES as WW, note-for-note?* — orthogonal to the timing/balance axes in
[../WW Linked/ext-seq-audio-findings.md](../WW%20Linked/ext-seq-audio-findings.md) (§B/§C).

**Donor side (already captured):** `docs/WW Linked/dolphin-captures-bgm-notes-20260728.txt`
(8,044 notes) — DuskTap at `TTrack::noteOn @80281258`, whose args are
`JASSeqParser.cpp:915 track->noteOn(noteid, note, r25, time, r22)`. Line shape
`note=<voice> p=(<key>,<vel>,<gate>)`; `note=`=noteid (voice slot), `p`=(key, velocity, gate/time),
`gate=0xffff` on the sustained voice.

**Receiver side (the one Engine hook, per F2 spec, SHIPPED):** `Ja1Track::noteOn`
(`src/d/ext_seq/ja1_track.cpp`) emits the **identical shape** at function entry — the port's
`cmdNoteOn` already mirrors the donor call `track->noteOn(noteid, note, r25, time, 0)`
(`ja1_parser.cpp:291`), so the same 4-tuple is in hand; the ignored `gate` arg was un-ignored to log
it. Toggle `DUSK_EXTSEQ_NOTE_TAP=1` (default OFF, HIGH VOLUME — capture sessions only). Tag
`[ExtSeq] §P2 noteTap`.

**Differ (the P2 verdict tool):** `tools/extseq_note_differ.py <donor> <portlog> [--donor-start S --donor-end E]`.
difflib sequence alignment on the `(voice,key,vel,gate)` tuple stream (absolute timestamps don't
align across runs; timing is a separate axis). Reports **strict %** (voice,key,vel,gate identical) +
**pitch %** (voice,key identical — right notes, dynamics aside) and mismatch classes:
`dyn-drift` (same note, vel/gate differ) · `wrong-note` · `donor-only` (port never played) ·
`port-only` (extra). Donor corpus is multi-song (~11 min) → window it to the played song with
`--donor-start/--donor-end`.

**Validated** (synthetic port from a donor window): identical→100%; a dropped note→`donor-only`
(both %s fall); a changed velocity→`dyn-drift` with pitch **still 100%** (the note-vs-dynamics split
works). **Awaiting:** a live port capture (`DUSK_EXTSEQ_NOTE_TAP=1` on a known song — Outset
`i_link`/`i_linkin`) → run the differ vs the matching donor window for the first computed note-fidelity
verdict. Ties to the §E variant caveat: capture the same arrangement the donor window holds.

### §P2-audio UPDATE — folded into the general JSONL pipeline (2026-07-28)

Foundry's `dusktap_to_jsonl.py` + `probe_differ.py` are now the pipeline; the standalone
`tools/extseq_note_differ.py` is **superseded** (kept as a convenience only). Two receiver-side parsers
were added to `dusktap_to_jsonl.py` so port logs convert to the **same schema** as the donor DuskTap:
- `[ExtSeq] §P2 noteTap note=<voice> p=(<key>,<vel>,<gate>)` → `site=bgm_note_on`,
  `key=["note", voice, pitch]` — **byte-identical to the donor `@80281258` key** (verified: port
  `note=0 p=(0018,…)` and donor `BP 80281258 (… 00000000 00000018 …)` both →
  `["note","00000000","00000018"]`, velocity `0000005f` preserved aside, exactly as the donor lambda does).
- `[Spawn] src=census:<name>@(x,y,z) proc=<proc>` → `site=stage_placement`, `key=["name", name]`.

**Verdict now runs through the general differ:** `probe_differ census|seq --site bgm_note_on donor.jsonl port.jsonl`.

### §P2-placement — computed layer verdict + the ONE Engine ferry (WHOSE TURN: History→Engine)

Ran `probe_differ census --site stage_placement` (donor banked JSONL, 1,608 placements — vs the
2026-07-28 port log, 148). **Result confirms §⑨ of the layers doc computationally:** the port reproduces
**30 core WW-crew placement names** (all villagers + Kamome/Oyashi/Pig/kani/koisi/kotubo/lwood/Lamp/
Kanban/Tpost/Throck/Akabe/Ekao/items/pflower). Every port-absent donor name is explained — non-content
(`LOD*`/`Tag*`/`KNOB*`), gated (`Dk`), interior-scope (`MKoppu`/`MOsara`/`MPot`/`SPitem`, `Cb1`/`Md1`),
held grass-flowers/objects, or the known `NpcSo`. **No surprise content gap.**

**Two caveats the run exposed (each a small, well-specced Engine ferry — History could take either):**
1. **Placement mirror is incomplete → the ONE real ferry.** `[Spawn] src=census` (148, 29 procs) omits
   **grass** (`NPC_EXTVEG`) and other bulk paths; `[ExtNpcPop] spawn #N` has the *perfect* schema
   (`name proc chunk=<layer> ww_params world=(x,y,z)`) **but is capped at #26**. For a complete placement
   census, add **one uncapped, toggle-gated** placement-emit at the universal ext-spawn choke point
   (`d_ext_npc_population.cpp` spawn loop) — reuse the `[ExtNpcPop] spawn` schema verbatim, drop the cap,
   gate behind `DUSK_EXTSEQ_PLACE_TAP=1`. Then the census covers every spawn (incl. grass/bridge) and the
   donor-vs-port placement verdict is apples-to-apples. `dusktap_to_jsonl.py` already has the parser shape;
   only the emit line changes.
2. **`probe_differ census` rate-verdict is wrong for one-shot events.** It models events/second (fine for
   emitters/SEs/notes); placements are one-shot and receiver events carry `t_s=0`, so the `A/s`,`B/s` +
   `DRIFT%` columns are noise for `stage_placement`. Needs a **presence/count** verdict mode for one-shot
   sites (Foundry differ tweak). Counts are the real signal today; read the table's `A n`/`B n`, ignore
   the rate/verdict for placements.

**WHOSE TURN → Engine (or History): ferry #1** — the uncapped toggle-gated placement-emit. Small,
schema-already-specced (copy `[ExtNpcPop] spawn`), one line at the spawn choke point. Unblocks the
complete layer-placement verdict.

**UPDATE — ferry #1 LANDED (History, §211).** `dExtNpcPop_placeTapLog` in `d_ext_npc_population.cpp`
(both create sites), uncapped, `DUSK_PLACE_TAP=1`, emits `[DuskLog] §P2 placement t=<ms> name=<n>
proc=<p> chunk=<layer> pos=(x,y,z)`; carries `t=ms` so the one-shot rate artifact (caveat 2) is also
gone. `dusktap_to_jsonl.py` parses it (`DL_PLACE_RE` → `stage_placement`). Built. Placement census is
now complete (grass included) on any `DUSK_PLACE_TAP=1` capture.

### §P2-placement — TWO History follow-ups ASSESSED + DEFERRED (2026-07-28, user-directed)

Both scoped now, implementation deferred (focus moved to the grass flower/tree port). They are the last
two pieces that turn placement/transition parity from a *special-capture* verdict into an *every-session*
one.

1. **PORT-ONLY spawn triage** (History analysis, not code). With the complete place-tap, the census now
   has a real `port-only` bucket (names the port spawns that the donor census lacks). Each needs
   adjudication: **faithful** (donor also spawns it, just not in the compared window — widen the donor
   window / check the full 1,608 set) vs **spurious** (a port over-spawn — a real bug). Deliverable = a
   per-name verdict table on the next `DUSK_PLACE_TAP=1` capture, feeding P9's row ledger
   (`outset-spawn-verdict-*.md`). No new code; a differ run + decomp/census cross-check. **Deferred.**

2. **socket→stage (socket_arg → identity) mapping table** — *the high-leverage one.* The port
   multiplexes ~40 static-prop identities through ONE socket proc (`NPC_HENNA0`) keyed by `socket_arg`
   (Otana=50 … Lamp=54 … Ikada=62). In an **ordinary** runtime spawn log (`fopAcM_create` / native
   `[Spawn]`), every one of them reads as `NPC_HENNA0` — indistinguishable. A table
   `NPC_HENNA0:<socket_arg> → <manifest proc / donor identity>` (built by scanning the `npc/*.ini`
   `socket`+`socket_arg` fields) **de-multiplexes any normal play log** into real identities. **That is
   what unlocks transition verdicts from every ordinary session forever** — no `DUSK_PLACE_TAP=1`
   required, and it's exactly the blocker P11's receiver check `awaits socket→donor-stage map` (F8).
   Deliverable = the generated table (a Foundry tool reading the manifests) + a `dusktap_to_jsonl.py`
   resolve step that applies it. Small, data-driven, no engine change. **Deferred.**

## Instrument inventory (transferred from Housing, §190–§197)

- **DuskTap**: patched local Dolphin (`D:\Dolpheen Plz\Local Dolphin\build\Binaries\
  Dolphin.exe`), 3 contained edits, portable, READ-ONLY taps via `User/Config/DuskTap.ini`.
- **Tap roster (10):** setSimple · JStudio actor/camera/message do_paragraph ·
  paragraph_reserved · fopAcM_create ×2 (char* overload: 0 hits, prune candidate) ·
  createSimpleEmitterID · seStart (HIGH VOLUME — capture sessions only) ·
  dStage placement @80041628 · TTrack::noteOn @80281258 (BGM notes; HIGH VOLUME, armed for
  the music session then comment out).
- **Captures:** `D:\Dolpheen Plz\captures\` (backed-up logs) + extracted traces in
  `docs/WW Linked/dolphin-captures-*.txt` (tale / storybook / awake-Aryll).
- **DolphinNoGUI:** built, unharnessed (F5).
