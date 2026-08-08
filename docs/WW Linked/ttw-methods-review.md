# TTW comparative methods review — Foundry (2026-08-01)

> **Lane: Foundry** (methods + instruments — this is doctrine/advisory, no content).
> **Assignment:** study Tale of Two Wastelands (TTW) — the closest existing analog to this
> project (FO3 ported into the FNV engine; two versions of the same engine, exactly our
> WW→TP shape) — and audit our porting methods against theirs.
> **Evidence tier:** `[web/document]` — GitHub repo/source reads + TTW's published docs.
> Never `[decomp law]`; every claim below cites its repo or page. Bus entry: §330.
>
> **Staleness caveat (load-bearing):** TTW's public GitHub org is their **2014–2017 era**
> (ttwinstaller 2.x, bsasharp, ESPSharp, the abandoned ttwlauncher rewrite). The modern
> 3.x installer (by zilav) is closed-source; its behavior is documented on their site/FAQ.
> Sources: github.com/TaleOfTwoWastelands (org), jzebedee/ttwinstaller releases,
> thebestoftimes.moddinglinked.com/faq.html, mod.pub/ttw/133/docs, geckwiki.com
> "TTW Mod Conversion Package and Guidelines".

---

## 1. Why TTW is the right comparator — and the one structural difference

TTW's problem was ours: move an entire donor game into a near-identical host engine,
legally (no donor bytes distributed), faithfully, and forever-regeneratable from the
user's own copies. Their answer, refined over ~14 years: **distribute transforms, not
content** — an installer + binary-diff databases + checksum manifests that rebuild
everything locally from user-owned files.

**The structural difference to keep in view:** Gamebryo actors are *data records* (ESM) —
TTW's port is overwhelmingly a data-conversion problem, so it could be ~95% mechanized.
Our actors are *compiled C++* — the donor `.cpp` adaptation (shims, compile-iterate) has
no TTW analog and will stay manual. Their nearest analog is instructive anyway: FO3
scripts could not be trusted after formID remap and had to be **recompiled one-by-one in
the GECK by hand** — and their mitigation pattern is the transferable part (see R6):
mechanize 95%, mandate the manual pass, then **ship a runtime check that the manual pass
actually happened**.

## 2. Scorecard — where current doctrine already matches or exceeds TTW

| Surface | TTW | Us | Verdict |
|---|---|---|---|
| Legal architecture | Ship diffs + checksums; regenerate from user's files | Covenant №31: zero donor bytes in repo/exe; user drops own extraction in `arcs/`; skeleton ships scripts only | **MATCH** — independently converged on the same architecture |
| Source checksums | MurmurHash `FileValidation` per file, multi-variant | `bridge_meta.ini` per-source-arc SHA-256 + tool/schema version | Match in kind; gaps in coverage (R2, R3) |
| Idempotence | curChk==newChk short-circuit → re-runs skip correct outputs | Audit-default + dry-run + `.pre-*-bak` + documented no-op re-runs | **MATCH** |
| Repair vs regenerate | Navmesh *repaired* not regenerated; LOD baseline shipped, regen downstream | §281 stage-data law: append offset-stable / edit in place, NEVER re-layout | **MATCH** — their scars confirm our law |
| Temporary-thing discipline | Hotfixes explicitly temporary, merged next release; satellite fix mods deprecated + absorbed (TTWFixes → core at 3.0) | Bridges/mounts carry native-target tickets; port-liberties ledger with reconciliation conditions; DN-9 | **MATCH** — confirms the owed-list doctrine |
| World bridging | Never merge worldspaces; new transitional content (Union Station train) + paired markers | Space purity; hosted WW spaces reached via warp/hub | **MATCH** in principle |
| Fix engine+tooling as part of the port | Team maintains xEdit, GECK Extender, the NVSE plugin layer | We ARE the source port + Foundry instrument estate | **MATCH** |
| Runtime parity instrumentation | **None.** QA = dedicated human testers + forum intake + reproduce-on-vanilla rules | Oracle stack, DuskTap/DuskLog, probe differ, state gate, golden traces, BTK baseline | **WE EXCEED** — this is the axis where our methods are genuinely ahead |
| One-command regeneration | `TTW Install.exe`: whole conversion end-to-end, progress, retry/abort, output size sanity | ~20 hand-run scripts in documented order | **THEY EXCEED** → R1 |
| Output verification | Every patched file verified against a known **target** hash | Source hashes + census goldens; converted-arc outputs mostly unverified byte-wise | **THEY EXCEED** → R2 |
| Conversion mapping data | One curated, versioned database (`TTWConversion.csv`) consumed by the conversion script | Rename dicts, `residmap.csv`, `actor_map.ini`, per-tool tables — scattered | **THEY EXCEED** → R5 |

## 3. Recommendations (prioritized; owner-lane routed; all await user ratification)

### R1 — HIGH: One-command pipeline runner (`ww_bridge convert-all`)
TTW's installer runs the entire donor→playable conversion as one sequential, resumable,
per-step-validated program. Ours is ~20 skeleton scripts run by hand in a documented
order — exactly the "repeated manual method" that Foundry P8 (instrument conversion)
exists to retire. Spec: a declarative recipe manifest (the abandoned ttwlauncher's
`Recipe` XML — versioned `from`/`to`, per-file copy/patch/rename/delete actions — is a
good shape) that regenerates the **whole mod folder from a fresh `arcs/` drop in one
command**, each step checksum-short-circuited (skip if output already correct, TTW's
idempotence trick). Payoff beyond convenience: the mod folder becomes a **derived
artifact** — today it is a hand-produced single copy in a local-only git repo with no
remote; if it is lost, unknown hours of hand-run conversion state go with it. With R1,
`arcs/` + repo = full regeneration. *Build: Bridge. Spec: Foundry. Audit: Housing.*

### R2 — HIGH: Output-target hash manifest (subsumes SL-1)
TTW verifies every transformed file against the **expected output** hash, not just the
source. We record source SHA-256s but do not pin converted-arc outputs. Proposal: at
acceptance time, record per-output-file hashes in `bridge_meta.ini` (or a sibling
`accepted_outputs.ini`); `ww_bridge verify` recomputes and diffs. This mechanically
catches the exact §113-STASH failure class — any lane that "repairs" the PAL0 stash
bytes changes the output hash and the gate names it — which subsumes the open SL-1
stash-assert ticket (keep the runtime assert too, per R6; the hash gate is the offline
half). Per №31-C: a missing manifest entry reports UNKNOWN, never CLEAN.
*Build: Bridge (SL-1 already routed there). Spec: Foundry.*

### R3 — HIGH: Donor-dump version roster (known-source manifest)
TTW's `.pat` dictionaries carry one diff **per known retail variant** of each source file
(GOTY/regional/patch), keyed by source checksum — they never transform an unrecognized
input. We hash the user's `arcs/` but have no roster declaring *which WW dump* those
hashes should be. Every downstream verdict (goldens, BTK baseline, golden traces from
the donor ISO) silently assumes one specific donor version. Proposal: a checked-in
expected-donor manifest (arc name → SHA-256 for the sanctioned dump, presumably NTSC-U);
`verify` flags any arc whose source hash is off-roster as UNKNOWN before conversion
runs. Cheap to build — the hashes are already computed. *Build: Bridge. Law (which dump
is sanctioned): user ruling, History records.*

### R4 — MEDIUM-HIGH: Runtime "manual-pass-happened" checks
TTW's best QA idea: converted plugins must be hand-re-saved in the GECK, and **TTW ships
runtime checks that this manual step occurred**. Generalize ours: every unavoidably
manual conversion step gets a load-time assert that its artifact is present and
well-formed — res-id restoration ran (arc entry ids are donor-numeric, `residmap.csv`
exists), PAL0 stash bytes present at palette load (SL-1's runtime half), manifest keys
resolve inside their RARC, veg blobs match `veg_manifest.ini` sizes. We have the static
half (actor contracts, `verify`, greplist); this is the boot-time half, reporting per
№31-C (UNKNOWN, never CLEAN, when a check can't run). *Spec: Foundry. Code: Engine
lands the hooks. Audit: Housing negative-controls (does a missing residmap actually
report?).*

### R5 — MEDIUM: Unified conversion database
TTW's whole record conversion is driven by one curated, versioned CSV database consumed
by one script — the mapping data *is* the product, maintained independently of the code.
Our WW→TP identity mappings are scattered: rename dictionaries in the port kit,
`residmap.csv` per arc, `actor_map.ini`, palette-table field maps inside
`convert_lighting.py`, particle-id facts in `jpc_crosscheck.py`. Proposal: consolidate
into one versioned conversion database (proc names, res ids, JPA ids, SE ids, palette
slot maps, chunk field maps) that every skeleton tool reads. One source of truth; diffs
of the database become reviewable methodology changes; the №-ledger can cite rows.
*Build: Bridge. Classification of rows: History (values, per LANES §0).*

### R6 — MEDIUM: Engine-envelope audio doctrine
TTW re-encodes all FO3 voice audio to 24 kHz mono **because FNV's runtime DSP (radio/
helmet/robot filters) distorts at the donor's native 44.1 kHz** — "plays" ≠ "plays
correctly under the host engine's DSP." Direct analog: our JAudio1 player feeds waves
into TP's JAudio2-era mixer via `registerShadowWave`. Before post-Outset audio scales
up, Foundry should audit the receiver mixer's envelope assumptions against WW wave
formats (sample rates, ADSR, pan law) *by measurement* — extend the note-on differ one
stage downstream toward rendered output, per the standing "verify at the stage the
player perceives" rule. *Spec + measurement: Foundry.*

### R7 — MEDIUM: Reviewable text projection of the mod folder
ESPSharp's role in TTW was **source control for binaries**: lossless plugin↔XML, canonical
ordering, unknown records preserved raw — so ESM edits diff/merge in git. We must NOT
round-trip (§281 re-layout law), but the *read-only half* is safe and valuable: extend
the existing census/goldens into a full canonical text projection of every mod-folder
artifact (arcs member tables + hashes, dzr/dzs chunk decodes, STB casts, palette
tables), regenerated by `verify` and git-tracked in the bridge repo. Any conversion
change becomes a reviewable text diff, and the projection doubles as the mod folder's
off-site record (pairs with R1's regenerability). Partly exists (goldens, fact sheets);
the delta is coverage + one-command regeneration. *Build: Bridge. Spec: Foundry.*

### R8 — LOW-MEDIUM (pre-release): Save-compatibility versioning policy
TTW's shipped rule: major.minor bump requires a new save; x.y.z is save-safe; hotfixes
temporary-by-definition and merged next release; bug intake requires reproduction on
unmodded TTW. Adopt the same three rules for the mod folder + exe pair before anything
ships beyond this machine. `d_ext_save_guard` gives the enforcement point.
*Law: user. Recording: Librarian.*

### R9 — LOW: System-reconciliation doctrine table
TTW wrote their reconciliation down as doctrine: **host systems win, donor content is
upgraded to them** (DT armor, ammo types, companion wheel applied to FO3 content), then
rebalanced as one game. Ours is the inverse inside donor spaces (donor = spec) with
receiver systems hosting — but it lives implicitly across the cookbook/DO-NOT/liberties.
One page, one table: for each system class (input, save, HUD, dialogue [DN-4 already
rules this], audio engine, collision, camera, events), which side wins and where the
seam is. Cheap; prevents relitigating settled seams. *Draft: Foundry. Ruling: user.*

### R10 — LOW: Version-ranged upgrade recipes; no big-bang rewrites
ttwlauncher's recipes were version-ranged (`from`/`to`, default `vanilla`) — planned
incremental upgrades instead of full reconversion. Our analog: `schema_version` bumps
currently mean re-adapt + deliberate golden re-seed; a recorded migration recipe per
bump would make upgrades auditable. **Attached caution from the same repo:** the 2017
clean-slate rewrite died at 14 commits while the "legacy" 2015 installer shipped for
years. Skeleton scripts get retired one instrument at a time (P8), never big-bang.

## 4. Two-way honesty — what TTW never had

TTW never built runtime parity instrumentation: no donor-side taps, no computed
MATCH/DRIFT verdicts, no golden traces — acceptance ran through human testers and a
forum, and their 3.2 release took 3.5 years partly for it. Our oracle stack (decomp law
→ measurement → noclip → video → user-as-tie-breaker) plus the differ estate is the
part of this project TTW would have envied. The gap they expose is not measurement —
it is **pipeline productization**: they made the conversion a *product* (one command,
target-verified, variant-tolerant, resumable) while ours is still a *procedure*. R1–R3
close that gap; the rest is doctrine hygiene.

## Addendum — V-series: TTW lessons aimed at PORTING VELOCITY (2026-08-06)

> R1–R10 above target pipeline productization and containment. On the user's follow-up
> ("what would speed up porting?") this addendum extracts the *velocity* lessons instead.
> Ranked by payoff-per-cost. Awaiting ratification. Cross-ref: roadmap in
> [ww-code-lineage-posture.md §8](ww-code-lineage-posture.md), which notes that **step 8
> (census) pays for throughput before step 19 (plugin) does.**

### The V-series at a glance

**This track runs PARALLEL to the Stage A–D roadmap — it is not sequenced into it.** Only V4
hangs off a roadmap step. Recommended order: **V1 → V2 → V3**, with V5 continuing as
already-queued work.

> **Rebuilt 2026-08-06** so the three-primary-source correction (§A1–§A3) and the §519 roadmap
> correction live in the columns rather than in prose beneath them.

| # | Owner | Step | Primary sources | Status / blocked by |
|---|---|---|---|---|
| V1 | **Foundry** builds · **History** first consumer | Cross-lineage accessor differ — **three-source**: donor header + receiver header + an independent implementation as **validator**, reporting **DISAGREEMENT rather than picking a winner**. Backlog: KFA1, SSP1, ETX1. | decomp *(both sides)* + **noclip** & **Winditor** *(validator)* | ready — nothing blocking |
| V2 | **User** ratifies · **all lanes** follow · **Engine** holds the kill switches | Intake rule: every playtest finding states whether it reproduces with the relevant gate off. Decides bug ownership before anyone reads code. | — *(process)* | ready |
| V3 | **Bridge** emits · **Foundry** specs | Environment fingerprint at the head of every log: our build-ID, WW-layer version, mod-folder state, `donor_roster` verdict — **plus dusklight build-ID, ABI version, and the 19b conformance verdict**, since symbol availability is a property of the *target build*. | decomp-side roster + receiver build | base ready; conformance field waits on **19b** |
| V4 | **Foundry** | **Donor-side** drift sentinel: on pin advance, diff ported donor functions and flag the changed ones. **Compose with `tools/vendoring/ww_rebaseline.py`, do not duplicate it** — that tool *merges*; V4 reports what needs **re-verification**. | decomp *(pinned ref)* | **UNBLOCKED** — roadmap 12/13 done (pin `1d57f046`, 0 behind) |
| V5 | **Foundry** | Crash-recipes-as-lint — the cookbook's [DIRECT-PORT CRASH RECIPES](../WW-Restoration-Cookbook-CANONICAL.md) as a kit scan over ported TUs. | project scars *(cookbook)* | **in flight** (§331 A1) |
| V6 | **User** rules · **Foundry** operates | Re-cost the parked reel/save-state farm — **temporal axis only**; noclip already answers static questions for free. Save-states primary, reels secondary. | **noclip** *(bounds the scope)* + DuskTap | ready — a re-cost, not an unpark |
| V7 | **Foundry** builds · **History** classifies · **Bridge** hosts in R5 | Import Winditor's `ActorDatabase.json` / `ActorResourceDatabase.json` with **per-field provenance tags**; `English Name` enters as **IVAN-governed leads at `? (unverified)`**, never as identity. Cross-check `Locations` against our DZR census. | **Winditor** *(our census stays authoritative)* | **BLOCKED on R5** *(corrected — was wrongly "ready")*; **and partly pre-existing**: `tools/foundry/winditor_oracle.py` (§394) already adapts the Winditor DBs as an independent oracle, so V7's remaining delta is the **R5 integration + IVAN-tagged `English Name`**, not the adapter |

**Source coverage, before and after.** As first written, V1 and V4 were decomp-only and V2/V3/V5
were source-agnostic — **noclip and Winditor appeared nowhere in the V-series.** They now carry
three roles: noclip and Winditor jointly **validate** V1 (independent implementations of the same
formats, guarding against our own transcription being the thing that is wrong); noclip **bounds**
V6 to the temporal axis; Winditor **supplies** V7.

**No V8 for noclip.** Its two contributions are already placed — format decoders (→ V1's third
source) and static scene answers (→ V6's scope bound). Its scene/resource knowledge does not
warrant a separate item: our DZR census is authoritative there, and noclip rosters are supersets
without story layers.

**Two drift sentinels, one pattern.** V4 watches the **donor** decomp advance; roadmap **19b**
watches the **receiver's** symbol table across dusklight builds. Same instrument shape, opposite
ends of the port — and **V3 is where both verdicts surface to a human.**

## Unified plan — R + V sequenced (2026-08-06)

> **Context for the ordering (user ruling, 2026-08-06):** final building/distribution pauses
> after step 19 until the port work is done. The goal is therefore **content throughput** —
> filling the mod folder, finishing islands and quests.
>
> **The organizing principle: R items are CAPACITY, V items are EFFICIENCY + SAFETY.** Build
> capacity first when capacity is the bottleneck — and it is (a 4-arc staging queue, one island,
> no quests finished). V1 prevents expensive bugs, but you cannot hit bug classes at volume if
> you never reach volume.

### Band 1 — DO NOW (zero-to-cheap; do not wait on the sequencing decision)

| # | Item | Owner | Work | Why here | Gate / status |
|---|---|---|---|---|---|
| 1 | **V2** intake rule | **User** ratifies · **all lanes** follow · **Engine** holds switches | Every finding states whether it repros with the gate off | **Zero build cost**; pays on the next bug report | ready |
| 2 | **V5** crash lint | **Foundry** | Add the cookbook's [DIRECT-PORT CRASH RECIPES](../WW-Restoration-Cookbook-CANONICAL.md) as laws to the existing `kit_laws.py` | Framework already landed (§423), so this is now **incremental** — best value-per-cost on either table | ready |

> **BAND 1 STATE, 2026-08-07 (read this before picking work):**
>
> **V2 — RATIFIED by the user.** In force. Every finding states whether it
> reproduces with the gate off. Its scope boundary is recorded above: it
> CANNOT test pure WW content, and needs a third answer, **N/A**, beside
> yes/no.
>
> **V5 — DELIVERED by Foundry, count UNDER REVIEW.** Shipped as 4 of 9
> lintable, with Recipe 1 refused by its own first run. Engine checked the two
> routed hits (§591): **both are false positives.** Recipe 5's TEX1 guard is 15
> lines above the call and predates the scan by five days; Recipe 2's
> `modelCalc()` is called in `_execute()` while `entryDL()` sits in `_draw()`,
> which is correct architecture, and the actor is runtime-dead besides. Both
> failed for ONE reason — the lint checked a narrower scope than the invariant
> lives in — which is the same defect Recipe 1 was correctly refused for.
> Pending Foundry's re-check the honest count is **2 of 9**.
>
> **Band 1 is therefore complete-pending-a-recount. Band 2 is next and it is
> BRIDGE's, not Foundry's.**

### Band 2 — CAPACITY (the throughput unlock; this is the "fill the mod folder" answer)

| # | Item | Owner | Work | Why here | Gate / status |
|---|---|---|---|---|---|
| 3 | **R1** convert-all | **Bridge** builds runner + non-stage modules · **Foundry**'s `space_kit` = stage module (§331 A2) | One declarative recipe; bulk-convert the full roster | **Deletes "stage the arc" from every future port** and moves work from arc scale to island scale. R2+R3 already verify its outputs, so the risky half is done | **VERIFY the R_DL02 pilot gate** (§364); run ends with an output-roster **re-pin** at a new user-verified stable point |
| 4 | **R5** conversion DB | **Bridge** hosts · **History** classifies rows | Consolidate codemod AUTO/REVIEW, `KNOWN_SIZE`, `STARTCODE_ALIAS`, `ww_dzb_roster`, both rosters | Absorption list is **compounding**; every port consults it; unblocks V7 | ready |

### Band 3 — CONDITIONAL (one judgment call)

| # | Item | Owner | Work | Why here | Gate / status |
|---|---|---|---|---|---|
| 5 | **V1** accessor differ | **Foundry** builds · **History** first consumer | Three-source bit-layout differ; reports **DISAGREEMENT**, never a winner | **GATE RESOLVED 2026-08-07 (§594) — SECOND BRANCH: slot behind R5.** The gate read *"if History's collision-attribute table is still open, do it now"*. It is NOT open: `dExtWw_repackDzbAttributes` is defined (`d_ext_npc_mount.cpp:3620`), declared, called from 6 sites across 3 actors; §334 built it 2026-08-01, §362 accepted it 2026-08-02, §363 closed its one amendment. This row previously stated the gate in the PRESENT TENSE and would have told a fresh instance to start V1 immediately. | **BLOCKED on R5** (Band 2, Bridge, not started). A trigger, not a queue position — V1 is Foundry's action WHEN Bridge lands R5, not before. |

### Band 4 — DEFERRED (none of these gate porting)

| Item | Owner | Status / why deferred |
|---|---|---|
| **R4** boot-time checks | Foundry specs · Engine lands | PARTIAL, un-censused — the census-then-systematize pass is cheap and can ride any engine-touch session |
| **R7** text projection | Bridge · Foundry | SEEDED — `space_kit` inventory JSON is the right shape; coverage + one-command regen missing; content-wall charter (§331 A3) stands |
| **V3** env fingerprint | Bridge · Foundry | **UNBLOCKED 2026-08-07** — 19b landed, so the conformance verdict now exists. Kept in this band by user ruling (order held), not by a blocker. Formerly: base ready, but its most valuable field (the **19b conformance verdict**) does not exist yet |
| **V4** donor drift sentinel | Foundry | UNBLOCKED (pin `1d57f046`, 0 behind) — but nothing is drifting yet. **Compose with `ww_rebaseline.py`**, which merges; V4 reports what needs re-verification |
| **V6** reel farm | User rules · Foundry operates | a **re-cost**, temporal axis only; noclip covers static questions free |
| **V7** Winditor → R5 | Foundry · History · Bridge | **BLOCKED on R5**; `winditor_oracle.py` (§394) already covers the adapter half — the delta is R5 integration + IVAN-tagged `English Name` |
| **R8** save-compat policy | User | pre-release timing; not urgent |
| **R9** doctrine table | Librarian | offered, not drafted |
| **R6** audio envelope | Foundry | HOLDS by its own terms until post-Outset audio scales |

### Band 5 — DONE (context, not work)

**R2** output hashes (§364, 2,057 files pinned) · **R3** dump roster (§332, 1,561 files, GZLE01) ·
**R10** doctrine adopted · census step 8 (all four axes + `leg_debt.py`) · roadmap step 10
(`banner_lint.py` + banner spec) · `kit_laws.py` lint framework · `winditor_oracle.py` adapter.

**R2+R3 together = TTW's foundation matched** (§364): sources pinned, outputs pinned, conversion
is a verified function. What remains on the R side is productization, not foundation.

### Stage D — COMPLETE (updated 2026-08-07; this section was stale)

**Step 19 is DONE: 129 of 129 receiver→WW call sites severed, and the receiver
BUILDS AND LINKS with the WW layer entirely absent** (exclusion build EXIT=0,
LNK1120 gone; normal 38,972,928 bytes vs excluded 38,759,936). 19a (import
classification, 15-symbol residual), 19b (symbol conformance across our fork and
dusklight main) and 19c (load-time gate) are all landed.

Step 11's link test, previously PROVEN unreachable before 19, now passes.

**Step 20 (distribution) remains paused by the user's ruling** — ruling (A)
PREBUILT PLUGIN stands and does not expire while content is built.

**Consequence for this plan: V3's deferral reason is void.** Band 4 defers V3
because "its most valuable field (the 19b conformance verdict) does not exist
yet." It exists — `docs/WW Linked/ww-19b-conformance.md`, per-symbol results
across both images. V3 is unblocked; whether it moves band is the user's call,
and as of 2026-08-07 the user has kept the order as-is.

**Does NOT mean the binary is WW-free.** 11 `mixed` TUs still carry legs — donor
lines inside receiver-owned files — which no file-level exclusion removes.

**Lanes with no V-series work:** Housing Security, HousingTemp, and Librarian are fully
loaded by roadmap Stage A (steps 3, 4, 5) — deliberately not given velocity items, since the
audit lanes must not be the ones racing.

### V1 — Cross-lineage accessor differ (HIGHEST VALUE) · *Foundry builds*

**The TTW lesson:** ESPSharp did not hand-write 150+ record parsers — they were **T4
template-generated from record definitions treated as data.** Mechanical layers get
generated, never transcribed.

**Our recurring bug class this kills.** Two independently-found, separately-debugged
failures were the *same* defect shape — a donor field read through receiver accessors with a
different bit layout:
- **§212** — JPA1→JPA2 `ESP1` flag word: WW alpha-enable `0x00000001` lands on TP
  scale-anim-enable ⇒ particle scaled to nothing ⇒ **invisible**. Its own recorded lesson:
  *"validating STRUCTURE is not validating SEMANTICS."* It explicitly leaves **KFA1, SSP1,
  ETX1 unverified and awaiting the same manual diff.**
- **§332** — dzb collision attribute: WW packs bits 16–20, TP reads 12–15/16–18 ⇒ every WW
  attribute with nonzero low bits reads as TP **sink-class** ⇒ Link sinks into furniture.

**The instrument:** given a donor header and a receiver header for the same block, emit the
field/bit-layout diff and a translation table stub, with both header citations inline. Turns
"hand-diff each block, discover the collision in a playtest" into a mechanical pre-flight.

**Why it is the top pick:** it is the cheapest instrument on the board, it retires the most
expensive bug class we have (semantic collisions that *pass* structural validation and stay
dormant until content triggers them — HousingTemp §3b), the backlog is already named
(KFA1/SSP1/ETX1), and it feeds History's live attribute-table work mechanically instead of
by hand.

### V2 — "Reproduce with the gate off" intake rule (CHEAPEST) · *process; Engine keeps switches alive*

**TTW lesson:** bug reports must reproduce on **unmodded TTW**, with formIDs included —
their single biggest triage-cost reducer.

**Ours:** every playtest finding states whether it reproduces with the relevant kill switch
off (`DUSK_EVT1_NATIVE=0`, `DUSK_WW_KNOB00_NATIVE=0`, …). One line in a report; it decides
*which lane owns the bug* before anyone reads code. Depends on kill switches staying alive —
already ruled for the event campaign through A5, and this is a second reason to hold that
line. Becomes free post-step-19 (load/don't-load the plugin).

**SCOPE BOUNDARY, found by running it 2026-08-07 — V2 CANNOT TEST PURE WW CONTENT.**

The rule compares a finding against the same game with the WW layer off. That
requires the thing to EXIST in both states. Receiver content does; WW content
does not — turn the layer off and Outset goes with it, so a WW rope bridge has
no counterpart to compare against. Not "hard to reach": nonexistent.

```
finding is on RECEIVER content   V2 applies      stars -> SETTLED as ours
finding is on WW content         V2 cannot        ropes -> untestable
```

Demonstrated the same day the rule was proposed, on the two open visual bugs.
Stars are TP's own draw code and the test settled ownership in one run, against
three sessions of reading the code and getting it wrong. Ropes are WW content
and the test cannot reach them at all.

**This does not weaken the rule** — it bounds it. Half of one day's findings were
decided by it. But "state whether it repros with the gate off" needs a third
answer alongside yes and no: **N/A — the finding is on content the gate removes.**
Without that, a WW-content bug either goes unreported under the rule or gets an
invented answer, and per №31-C an unanswerable check must say so rather than
guess.

What tests WW-content bugs instead: the donor itself (noclip, or the real game)
for "is this what it should look like", and the retained aurora instrumentation
for "what is the renderer actually being asked to draw".

### V3 — Environment fingerprint in every log · *Bridge emits · Foundry specs*

**TTW lesson:** a canonical load order existed so bug reports were *comparable*.

**Ours:** every DuskLog/crash log opens with build-ID, WW-layer version, mod-folder state,
and `donor_roster` verdict (R3, built §332). Makes every report self-describing and kills
"which build was that?" round-trips. Pairs with the existing fail-closed Build-ID↔PDB check
in symbolication — same discipline, wider net.

### V4 — Donor drift sentinel · *Foundry builds* · pairs with posture §6 ruling 2

**TTW lesson:** `PatchInfo` arrays keyed by **source checksum** — one diff per known donor
variant, so an unrecognized input is never transformed.

**Ours, applied to a *moving* donor:** zeldaret/tww is ~72% and climbing. Once we vendor by
pinned reference (posture §6 ruling 2), upstream **will** rewrite functions we transcribed.
The sentinel: on ref advance, diff the donor functions we have ported and flag the changed
ones. Without it, upstream drift becomes another stop-the-world audit — the exact failure
mode this whole document set exists to stop recurring.

### V5 — Crash-recipes-as-lint (ALREADY QUEUED — reaffirmed) · *Foundry*

Adopted at §331 A1 from the §330 analysis. Cookbook recipes 1–9 encoded as a kit scan over
ported TUs. Reaffirmed here as the cheapest velocity item already on the board: each recipe
is a bug that shipped and cost a playtest cycle to find.

### V6 — Revisit the parked reel/save-state farm · *user ruling; Foundry operates*

**The uncomfortable TTW data point:** they solved acceptance with **dedicated QA humans**
(named roles: Risewild, Callen, Kazopert, Hairylegs) — not instruments. We cannot hire, so
the instrument path is the *only* substitute for the human-oracle bottleneck
([Foundry.md](../Foundry.md)'s founding problem), and P1/P5 is currently **parked** for lane
cost.

Not a request to unpark. The recommendation is narrower: TTW's evidence suggests the
acceptance bottleneck justifies more structural investment than it has had, and the design
correction already on file (**save-states primary, reels secondary** — WW is a known
movie-desync offender) is the cheap version. Worth a re-cost, not a re-argument.

### Amendments — the three primary sources (2026-08-06, user correction)

> The user records that the workflow's three primary drivers are **noclip.website**, **the
> decomp**, and **Winditor**. The V-series as first written leaned on decomp + our own
> measurement and **did not account for noclip or Winditor at all** (V1/V4 are decomp-sourced;
> V2/V3/V5 are source-agnostic). Three consequences. Winditor was surveyed for this amendment
> at `D:\XXXXXXX\Winditor` — consulted, not assumed (§443 rule).

**§A1 — V1 becomes a THREE-source differ.** As first specified, V1 diffs a donor header against
a receiver header. Both are *our* transcriptions, and the entire bug class it targets **is
misread bits** — so a two-source differ can confidently emit a wrong table. noclip and Winditor
carry **independent implementations** of these same formats (noclip decodes both WW and TP;
Winditor ships `GameFormatReader`, `SuperBMD`, `JStudio`). Amendment: the differ takes a third
input — an independent implementation's constants — as **validator**, and reports DISAGREEMENT
rather than picking a winner. This is the oracle stack applied to the instrument itself, and it
is the difference between "the differ is fast" and "the differ can be trusted."

**§A2 — V6 narrows to temporal data.** noclip already answers *static* donor questions (what is
in this room, which arc holds this resource, what does it look like) without any playtime. It is
a renderer, not a simulation, so it **cannot** answer temporal questions — emitter density over
time, state timing, sequence behaviour. Amendment: the reel/save-state farm is re-costed **for
the temporal axis only.** Do not build reels to answer questions noclip answers for free. This
narrows V6's scope and *strengthens* its justification, since the remaining need is the part
nothing else covers.

**§A3 — V7 (NEW): mechanize third-party source consultation.** Winditor ships a curated
`ActorDatabase.json` (~404 KB) whose entries carry `Actor Name` (placement name) ↔
`ActorClassType` (donor class), `English Name`, `Archive Name` / `Main Model` /
`Secondary Models` / `Wait Animations`, and a per-actor `Locations` stage roster — plus a
separate `ActorResourceDatabase.json` (~150 KB). Four live consumers already exist: the
census→OBJNAME spawn path (name→class resolution), the multitype-actor registry (one placement
name, many behaviours), the port kit's resource-manifest pre-flight, and stage-roster
cross-checking.

Import it into the R5 conversion database **with per-field provenance tags**, and treat the
fields differently by evidence class:
- **Structural fields** (name↔class, archive/model/anim, locations) — mechanically useful,
  tagged `[winditor]`.
- **`English Name`** — a *community identity claim*. **The IVAN rule governs it: names and
  resemblance are never evidence.** These import as **leads at `? (unverified)`**, never as
  identities. Importing them untagged would launder community guesses into the database as
  fact — the precise failure IVAN exists to prevent.

Second benefit: §443's failure (*"Winditor-law claimed on work Winditor never saw"*) becomes
**structurally impossible** once the data is imported and tagged — you cannot cite a source the
database does not show you consulting.

**Redundancy check (the user's explicit question): nothing becomes redundant.**
- *Our DZR census vs Winditor `Locations`* — **ours stays authoritative.** Ours is data-derived
  with true per-story-layer rosters; noclip/Winditor rosters are supersets without story layers
  (the standing noclip caution). Winditor's list is a **cross-check whose disagreements are
  signal** — either our census missed a layer or their list is incomplete.
- *Our `decode_stb.py` vs Winditor's `JStudio`* — **ours stays.** Ours is transcribed from the
  **receiver's own** parse rules with `file:line` receipts, which is what a port needs; theirs
  is a donor-side reader. Cross-check, not replacement.
- *Our `adapt_bdl_arcs.py` vs `SuperBMD`* — different jobs. Ours retags BDL4→BMD3 **in place,
  preserving RARC member offsets**; SuperBMD is a full converter. Not interchangeable.

**Unchanged by this correction:** V2, V3, V4, V5 — process, logging, decomp-ref tracking, and
cookbook lint are all source-agnostic.

### Not recommended (considered, rejected)

- **Editor-tooling investment** (TTW maintained GECK Extender / xEdit as part of porting).
  Their bottleneck was *data authoring*; ours is *C++ actor porting*. The level editor and
  Winditor help placement review, not the critical path. Revisit if placement authoring ever
  becomes the constraint.
- **Third-party conversion enablement** (TTW published conversion guidelines so the community
  converted mods — a real force multiplier). Premature: it presupposes the stable plugin ABI
  from roadmap step 16. Name it as the post-step-17 stage, do not build toward it now.

## Weak spots in TTW's own code, noted so we don't inherit them

- `.pat` format had **no magic number and no version field** — compatibility hung on
  matching serializer code. (We already carry `schema_version`; keep it mandatory in any
  new artifact format, including R2/R5 outputs.)
- Parallelism was compile-time-`#if`-gated and off by default — bolted on, never trusted.
- Their main public "TTW" repo is an empty husk; the real 3.x pipeline went closed.
  Our bus/ledger/state-doc record is already stronger provenance — keep it that way.
