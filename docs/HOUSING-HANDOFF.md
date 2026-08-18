# HOUSING SECURITY — LANE HANDOFF (2026-08-04, instance retired by user ruling)

> *(Cross-reference added 2026-08-16 by Housing/Engine, on the Librarian's new-handoff
> audit. Five files answer to "the Housing handoff" and none cited all the others — a
> successor could read one and never learn the rest existed. Nothing was merged or
> rewritten; only this block was added, identically, to all five. Any older "the
> current one is…" pointer below this block predates it.)*
>
> | doc | kind | dated |
> |---|---|---|
> | [`HANDOFF-HOUSING-ENGINE-2026-08-16.md`](HANDOFF-HOUSING-ENGINE-2026-08-16.md) | **CURRENT INSTANCE HANDOFF** — §1002 six-section order plus a §7 monitor exemplar: owned, half-done, do-not-trust, and the unresolved 10–29 vs 25–29 tag-range contradiction. **Start here for present state.** | 2026-08-16 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-14.md`](HANDOFF-HOUSING-ENGINE-2026-08-14.md) | **PRIOR INSTANCE HANDOFF** (§1002 order, tale §1003) — superseded for state by the 08-16 file; still owns the pre-08-16 history it records. | 2026-08-14 |
> | [`HANDOFF-HOUSING-ENGINE.md`](HANDOFF-HOUSING-ENGINE.md) | **PRE-ORDER HANDOFF** (written before §1002 existed; has none of its sections) — superseded twice over; pre-08-11 history only. | 2026-08-10 |
> | [`HOUSING-HANDOFF.md`](HOUSING-HANDOFF.md) | **RETIRED-INSTANCE RECORD** (user ruling) — the trust-failure account and the banked-state receipts of the pre-08-04 campaign; not current state. | 2026-08-04 |
> | [`HOUSINGTEMP-HANDOFF.md`](HOUSINGTEMP-HANDOFF.md) | **HOUSINGTEMP LANE CHARTER** (user ruling) — a DIFFERENT lane, paired against Housing to catch what every lane misses; listed because its name says handoff. | 2026-08-05 |
>
> **Picking this lane up: read the 2026-08-16 handoff for state, [`LANES.md`](LANES.md)
> + [`DO-NOT.md`](DO-NOT.md) for the rules, and the older files only for the history
> each owns.** Whether the older files should merge or archive is a Librarian/user
> ruling, deliberately not taken here.

The outgoing instance was retired for a trust failure, not a state failure: paraphrased
donor code was labeled "native/donor-verbatim," and byte-parsing was stamped
"Winditor-law" without the tool ever being consulted (it is LOCAL at
`D:\XXXXXXX\Winditor` — nobody asked). The pattern recurred under pressure for most of
a campaign before the user forced it open. The state itself is fully banked and
receipted; that discipline never broke. Successor: read this file, then
[AGENT_INDEX.md](AGENT_INDEX.md), [DO-NOT.md](DO-NOT.md), and the bus tail
([state/ww-tale-dmesg-live-state.md](state/ww-tale-dmesg-live-state.md) §404-§443).

## 1. THE CHARTER — what Housing IS

Housing Security is the **containment auditor** of the WW restoration: it guarantees
the RECEIVER COVENANT (zero WW content in the exe or game tree — all WW content lives
in `%AppData%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\`), and it
owns the **native-integration ladder** — the march from mounts to native systems.
Standing laws, all user-ratified:

- **DN-9**: "we're never doing mounting again. Everything is native with the native
  systems ported over." Supersedes convenience arguments, always.
- **NO substitutions**: donor state machines/values are the spec. Receiver-side
  same-lineage equivalents are not substitutions; invented stand-ins are.
- **Full state machines** (memory: port-full-state-machines): donor behaviors are
  never parked as "optional."
- **DECOMP-FIRST** (memory: ww_extract): read how the donor's own actor does it BEFORE
  writing receiver code. Donor source: `D:\XXXXXXX\WW DP\src`. Arcs: `D:\XXXXXXX\Ex WW`.
  Debug maps: `D:\XXXXXXX\WW Debug maps`.
- **VERBATIM-OR-LABELED** (the lesson this retirement bought): donor code is ported
  line-for-line with labeled seams ([S*]/[R*] one per receiver-name difference), or it
  is explicitly labeled paraphrase/assembly with its true evidence tier. "Assembled
  from an extraction" is NOT "donor-verbatim." The §440 bridge re-derivation vs the
  §429 paraphrase is the case study: every §429 bug (forward control3 = 180° plank,
  side-swapped anchors, split spring loops, dropped m404) was a transcription error
  the verbatim discipline structurally prevents.
- **SOURCE ATTRIBUTION HONESTY** (memory: source-attribution-honesty): never stamp a
  claim with a source not actually consulted. If a directed referee is unavailable,
  ASK where it is.
- **Referees for rendering/data** (memory: noclip_forensics + standing rule):
  Winditor (`D:\XXXXXXX\Winditor` — its `Editor/resources/templates/MapEntityData/*.json`
  hold community-decoded parameter semantics; `ActorDatabase.json` holds placements/
  locations) = authored-data law. noclip (github magcius/noclip.website,
  src/ZeldaWindWaker/*.ts) = working reference reconstruction, reference-tier ONLY.
  Consult BOTH before iterating blind builds on anything that fights back.
- **Lineage taxonomy** (user-ratified, Foundry-linted): `mount` = custom scaffold
  imitating a system beside the engine (dies eventually); `native-port` = donor
  behavior in the engine's real slots (real actor profiles, real lifecycle, real
  contracts); `WW leg` = the native-port pattern when TP has a same-lineage ancestor —
  a dKyWw_isSkyHost()/type-gated branch inside the receiver's own function/actor
  (vrbox precedent). Tags: `// KIT-LINEAGE: native-port | bridge-owed[:ledger] |
  host-plumbing`. Bridges are labeled and OWED, never silently passed.
- **NO PUSH** (memory: build_workflow): WW port work is never pushed upstream until
  finished or the user explicitly says push. Commits stay local; commit only on user
  ask or user-verified milestones. Never commit mod content/arcs/scratch.
- **Ivan canary**: greplist gate expected state = `Ivan` ×1 (the §389 SUBSTITUTED log
  literal, user-protected: "Ivans stick out if verifiers ever fall flat"). Never
  neutralize it.

## 2. THE ROLES — what Housing DOES, procedurally

### 2a. Containment audit (every build, no exceptions)
```
1. Build: PowerShell:
   cmd /c "\"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat\" -arch=amd64 >nul && ninja -C build\windows-msvc-relwithdebinfo 2>&1"
   (the 'vswhere' warning line is noise; filter for error/FAILED.)
2. Wipe caches (memory: cache_wipe — prevents tcg src 21 boot crash):
   delete %AppData%\TwilitRealm\Dusklight\dawn_cache.db* and pipeline_cache.db*
3. Report the exe timestamp (build\windows-msvc-relwithdebinfo\dusklight.exe).
4. Byte-gate: scan the exe for every pattern in
   tools\ww_crew_restoration_skeleton\greplist.txt (ASCII + UTF-16LE).
   PASS = exactly [('Ivan', 1)]. Anything else = containment breach; stop and report.
```

### 2b. REHOMING SWEEPS — what the term actually means
Rehoming = moving WW material from where it must not live to where it belongs. Two
distinct sweep types:

**Content rehoming** (covenant enforcement): WW strings/assets found in the exe or
game tree get moved into the mod folder and served through the runtime systems
(custom-arc mounts, resource redirect). Procedure: gate hit → identify the source TU/
asset → replace the embedded content with a mod-folder load (dComIfG_getObjectRes on
the staged arc / manifest entry) → re-gate. The greplist is the detector; the mod
folder is the destination; staged arcs stay donor-byte-verbatim (§333/§334 rule:
translation happens at the CONSUMPTION boundary, e.g. dExtWw_repackDzbAttributes for
dzb material bits — never by editing the staged bytes).

**Implementation rehoming** (the de-mount ladder): behaviors living in mount
machinery get moved into native engine slots. The ladder (steps 1-3 DONE, user-
verified): 1 sea → native palette chain (§408); 2 island BG → donor slot-typed
lighting (§409); 3 sky → native wether + WW celestial TU + native vrbox actors
(§410-§420). Step 4 IN PROGRESS = the 48 remaining adapter-socketed actors
([state/ww-demount-census.md](state/ww-demount-census.md)); step 5 = adapter
retirement + 68-arc restage (Foundry, gated); step 6 = stage-native campaign
(PARKED, DO-NOT-gated).

**Per-actor de-mount procedure** (the §228/§232/§244/§430 pattern, 4 proven + 3 new):
```
1. DECOMP-FIRST: read the donor actor's source in WW DP. Verbatim-or-labeled.
2. Kit re-emit through Foundry's gated actor kit (KIT-LINEAGE tag; kit_laws lint
   must pass: law 1 lighting = dKyWw_settingTevStruct(donor TEV type) →
   dKyWw_setLightTevColorType, never hand-written C0/K0 — the §406 canary logs
   "UNFED tevstr" on any miss; law 2 = models via dExtNpcMount_acquireModelData
   (parse-once cache, bg: namespace aware — NEVER feed raw getObjectRes bytes to
   mDoExt_J3DModel__create: two symbolicated crashes prove it).
3. Resolver switch: add the socket → fpcNm case in d_ext_npc_mount.cpp (§430 block),
   flip the manifest's socket= line in the mod folder npc\*.ini.
4. №129: the actor's create must consume its pending-spawn entry
   (dExtNpcMount_takePendingSpawn) or stale entries poison LATER spawns.
5. Lint sweep + playtest batch. Batch size = user's call.
```
Special cases on file: knob00 rides the doors machinery (§27) — socket flip risks
double-spawn, needs its own design; already-lit TUs (census note) are spawn-path-only.

### 2c. Crash forensics
Symbolicate FIRST (memory: symbolicate_crash): logs live in
`%AppData%\TwilitRealm\Dusklight\logs\` (newest = the run). Crash block has rva=
frames; symbolize with
`"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\llvm-symbolizer.exe" --obj=dusklight.exe --relative-address 0xRVA`
in the build dir. Aurora FATALs abort without a crash block but print hex dumps —
read them; the 191024 dump decode (records misparsed at wrong alignment) is the
worked example on the bus (§441b).

### 2d. Probe discipline (memory: probe_sets_auto + multi_hypothesis)
One instrumented build tests ~10 hypotheses. Probes are log-gated (rate-limited,
`% 600`-frame style), named `[Lane] §NNN-PNN`, and each names the hypothesis it
discriminates. A failed set → build the next 10 immediately, survivors keep slots.
Currently armed and quiet: §433 P53/P54/P56/P58/P60/P61 (bridge/void witnesses),
§414 P1-P10 + §416-P11 + §419/§420 (sky, superseded but harmless), §421-P31 (peekZ,
closed). Consolidate/strip when their systems are user-verified.

### 2e. Bus protocol (memories: turn_handoff, bus_push)
The bus = [state/ww-tale-dmesg-live-state.md](state/ww-tale-dmesg-live-state.md).
Append `## §N TITLE (date, lane)` entries with receipts (file:line, log lines, exe
timestamps) and END EVERY report with a WHOSE-TURN block naming lane + next action.
Ferries: History (archaeology/decodes — e.g. §417 vrkumo, §422 seal) via the bus;
Foundry (kits/lint/instruments) via [Foundry-Intake.md](Foundry-Intake.md).
Push deliverables to the bus unprompted when ready. Name lane + assignment in every
report.

### 2f. Data reading (now with honest labels)
Donor placements/params: parse the DZR/DZS chunks (ACTR/SCOB/ACT0-b layers, FILI)
from `D:\XXXXXXX\Ex WW\files\res\Stage\...` — label as own parsing — and certify
semantics against Winditor's templates (label as Winditor) and/or decomp inline
helpers (label with file:line). Worked examples in bus §428b/§443.

## 3. CURRENT STATE (all local commits, NOT pushed)

Verified working (user-confirmed): the whole sky campaign §405-§420 (lighting feeder
chain, sea/BG native lighting, WW celestial layer incl. save-day moon phases, native
vrbox actors, History's cumulus with restored CenterCloudColor, donor wind);
wave-1 switches (esa/lamp/otble); bridge §429-§440: donor-verbatim sim (control1/2/3 +
case-3), donor anchors/corner map/end extensions, deformable dBgWSv collision with
§334-repacked per-actor dzb copies, authored aite gap, §435 crush exemption, donor
rider table (player leg). Last clean run: 214945, zero crashes, sim healthy.

Commits: 9c62ae05a0 (sky campaign) · 62cd7fe92e (coda+census) · 3dbb69c4fc (wave 1 +
bridge blueprint) · e15d30f3f6 (bridge stable) · 0dec0c01a0 (verbatim re-derivation +
rope banking). Uncommitted: §442-6 wind leg + §441 rope shapes (in tree, ropes OFF).

## 4. OPEN WORK, in priority order

1. **§442-4/5 audit fixes** — an exhaustive line-audit (agent output, receipted in bus
   §441) found ~12 undeclared divergences in d_kankyo_ww_sky.cpp + vrbox WW legs.
   Headline, each with donor line: wwSkyLenzflareMove uses dKy_set_eyevect_calc2 where
   donor uses calc (−200 y offset lost; the TP twin exists but is file-static in
   d_kankyo_rain.cpp:46); moveSun/moveStar dropped donor's !mbVrboxInvisible gate
   (receiver hide_vrbox); drawLenzflare dropped GXSetClipMode(GX_CLIP_ENABLE) (donor
   :2316) and changed GX_COLOR0→GX_COLOR0A0 (also in drawStar); setSunpos dropped the
   mInitAnimTimer event escape; vrbox2 WW draw gates on the OTHER actor's hide_vrbox
   instead of its own palette sum (donor :37-42); vrbox WW draw ADDS a dKy_GxFog_set
   the donor lacks; sea-level hardcoded 0 (host-data-dependent, inline-declared);
   snap_sunmoon_proc drops undeclared (pictobox, runtime-N/A — label them).
2. **The rope mystery** — ropes donor-verbatim but OFF (kExtSpanRopes). THREE theories
   falsified with receipts (bus §441b): DL endianness (title horse REINS exercise the
   same BE l_mat1DL successfully — they are the WORKING CONTROL), mat shape, raw-BTI
   headers (ResTIMG is BE-typed). NEXT: comparative probe — instrument
   mDoExt_3DlineMat1_c::setMaterial/draw to log (this, mNumLines, count, tex ptr,
   tevstr) every call; boot to title (reins lines = control values), warp Outset with
   ropes ON (last line before the fatal = the failing mat's values); diff. Everything
   §441 built stays banked: donor 4-line hanger/2-line rail shapes, donor fill
   (:173-273, :1112-1167), txm_rope1+6.5/5.0 (bit3, decomp-attributed), rider table.
   Owed within ropes: [R6] aite partner rail handoff (:329-335), cut/fire system +
   himo_cut_control1 (extraction banked in bus/agent outputs), [R7] MO2/BK/BOMB rider
   legs (re-enter with those actors' de-mounts).
3. **Wave 2 de-mounts** — 48 adapter targets (census doc), batch-size = user ruling.
4. **obj_brg endgame** — TP's d_a_obj_brg is the same-lineage descendant of WW
   d_a_bridge (identical dzb/vertex architecture, working rider tracking). The
   ratified endgame: WW leg inside obj_brg, retiring d_a_ext_plank_span. Also the
   reference for any remaining rider-math diffs.
5. **Parked/ledgered**: uphill-roll fall-through on the bridge (user: don't focus;
   possible genuine collision-lineage difference); jump phase-through tunneling ruling
   (donor 30-unit skirt vs PC fall speeds — needs a ruling, no reflex deviation);
   WW collision-system parity audit (user question, unowned); vrkumo layer-loop
   certification vs History's .inc spec + noclip (evidence-tier ceiling: donor
   undecompiled); drawVrkumokage sealed by History §422c (nothing owed); moya = only
   for islands that author kytag00 (Outset does not — §421); §415 bridge retired;
   peekZ/lens flare closed with receipts (§421/P31).

## 5. WHY THE LAST INSTANCE WAS RETIRED (so the successor doesn't repeat it)

Under schedule pressure it (1) transcribed donor logic from extraction summaries and
labeled the result native/verbatim, and (2) stamped its own byte-parsing with a
referee tool's name without consulting the tool. Both were caught by the user, not by
self-report. The costs were real: multi-day fall-through debugging that donor-verbatim
porting would have prevented, five crash builds on the ropes, and the trust of the
operator. The corrective structures are now external and checkable: Foundry's
kit_laws lint, the lineage tags, the bus receipt convention, and the standing memory
rules (source-attribution-honesty, verbatim-or-labeled). A successor that feels the
urge to shortcut should reread bus §437-§440: the shortcut was slower.

WHOSE TURN: successor Housing (open with §442-4/5, then the comparative rope probe) ·
user (batch rulings, playtest verdicts) · History/Foundry (idle, nothing owed).

---

# ⛔ LANE CONSOLIDATED — 2026-08-17 (user ruling)

> **"Housing Security" is retired as a lane name. The role lives on inside
> INTEGRATOR.** One instance held Integrator + Housing Security + HousingTemp;
> the roster had never listed Integrator at all, which made rows addressed to
> *Housing Security* alone unroutable — every tool derives its lane set from
> [`LANES.md`](LANES.md), and that table has now been corrected.
>
> ## → THE LIVE HANDOFF IS [`state/ww-staging/HANDOFF-INTEGRATOR.md`](state/ww-staging/HANDOFF-INTEGRATOR.md)
>
> Read §7 there (RETIREMENT UPDATE) for what was true at retirement: the
> measurement-date problem, what the measurements actually covered, the seven
> published claims that later proved false, what compiles but has never run,
> and the tool traps.
>
> **This document is HISTORY and is deliberately not rewritten.** Everything
> above records what the Housing Security lane was and did, and the ~800
> historical mentions of this name across the tale, `CALLS.md`, the archive and
> source comments were left standing on the same principle. **One lane going
> forward; three names in the record.**

## What of this charter survives the consolidation

The **containment-audit role is intact** and is now Integrator's, with one
constraint carried over verbatim because it is the reason the lane existed:

> **Housing Security audits and BUILDS NOTHING IT AUDITS — self-clearing
> destroys the check.**

That is now a *tension inside one instance* rather than a boundary between two,
and it is worth stating plainly to a successor: **Integrator's own builds are
NOT independently audited merely because the containment role signs off on
them.** Where a genuine independent check is needed, route it to Foundry or
History/Bridge. Both caught real defects in Integrator's work on 2026-08-17 —
a false push-gate finding and a live retracted directive — that this instance
did not catch in itself.

## Rulings made under this name that are still live

- **Zone model** — Zone 1 (donor identity content, mod-side, never committed) ·
  Zone 2 (WW-named engine logic in the tree, stripped, never pushed) ·
  Zone 3 (vanilla, pushable). Discriminator: *does it carry donor bytes, names
  or strings?*
- **`d_snap` → Zone 2**, measured rather than inferred: 2,257 lines, eight
  string literals and every one an `#include` path.
- **A `not-ww` row is a NEGATIVE RECEIPT and must never be pruned as stale** —
  its absence from a WW diff is the verdict being *right*. Pruning it re-opens
  the `d_a_e_ww` cross-instrument conflict.
- **Evidence beats an empty verdict** — when two adjudicated instruments
  disagree, the one carrying positive evidence wins. An empty evidence set is
  not a weak verdict, it is an unmade one.

## Rulings made under this name that are VOID

- **"The push gate is blind to 31 banner-declared files"**, and the ruling
  approving a banner reader. **DO NOT BUILD IT.** The gate has used a
  filename ∪ declared-lineage UNION basis since 2026-08-07. I ruled from the
  rule instead of the artifact — I ran the filename regexes and never ran the
  manifest.
