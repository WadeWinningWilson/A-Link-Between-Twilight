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

## Weak spots in TTW's own code, noted so we don't inherit them

- `.pat` format had **no magic number and no version field** — compatibility hung on
  matching serializer code. (We already carry `schema_version`; keep it mandatory in any
  new artifact format, including R2/R5 outputs.)
- Parallelism was compile-time-`#if`-gated and off by default — bolted on, never trusted.
- Their main public "TTW" repo is an empty husk; the real 3.x pipeline went closed.
  Our bus/ledger/state-doc record is already stronger provenance — keep it that way.
