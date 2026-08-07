# WW subsystem census — Foundry instrument spec (2026-08-06)

> **Lane: Foundry** (instrument spec; Foundry builds, HousingTemp negative-controls).
> **Status: DRAFT — not built.** Awaiting user ratification alongside
> [ww-code-lineage-posture.md](ww-code-lineage-posture.md). Bus: §428.
> **R-item state at drafting:** §330's R3 is **BUILT** (§332 — `donor_roster.py`,
> sanctioned dump GZLE01, 1,561 files, OFF-ROSTER reports UNKNOWN per №31-C); the §331
> audit endorsed §330 with three amendments — **A2 in particular binds this spec: one
> declarative recipe, `space_kit` as its stage-data module, never a second orchestrator.**
> **Origin:** user assignment 2026-08-06 — after the WW-layer discovery. *Originally stated
> as "46 WW-owned TUs compiled into `dusklight.exe`, 43 confirmed by naming convention in
> `files.cmake`"; **corrected by HousingTemp step 5 (bus §491) to 47 TUs **named** by WW
> convention — a floor, not a lineage count — against a lineage surface of ~60 donor-derived
> TUs plus ~8 receiver TUs carrying donor legs.** Reasoning and derivation:
> [ww-code-lineage-posture.md §1.1](ww-code-lineage-posture.md). Axis D below is what actually
> measures lineage; this spec's roster axis measures names.*
> On that corrected basis —
> decide *wholesale vs piece-by-piece* per subsystem and size the plugin boundary.

## 0. What this instrument answers

One census, five questions that are currently all guesswork:

1. **Which remaining WW subsystems can be ported wholesale, and which must stay piecewise?**
2. **How big is the plugin/hook ABI actually?** (Today: unknown, so the plugin
   architecture cannot be costed.)
3. **What is the leg-migration debt?** (Every WW branch interleaved in a receiver-owned
   file — the class that cannot relocate.)
4. **How many donor data bytes are compiled into the exe?** — closes the open watch item
   ruled 2026-08-05 ([HOUSINGTEMP-HANDOFF.md](../HOUSINGTEMP-HANDOFF.md) §5.1), whose
   trip-wire (b) is *"donor DL/data bytes stop being countable on one hand."*
5. **What is the true WW-layer roster?** — generated from the build, replacing the hand
   list that went 12 files stale in 12 days.

**Why one instrument and not five:** every one of these is a different projection of the
same underlying fact — the dependency and provenance graph of the WW layer. Building five
tools would give five rosters that drift apart. This is the R5 lesson (one curated
database, not scattered tables) applied to our own instruments.

## 1. Roster source — generated, never curated

**Input:** `files.cmake` + a provenance banner convention (§6), **not** a maintained list.

**Reasoning.** The Tier-1 never-push list in
[NEVER-PUSH-STRIP-SET.md](../NEVER-PUSH-STRIP-SET.md) says 34 files; the build has 47
WW-*named* ones, and ~60 donor-derived ones once lineage rather than filename is the test
(§491 — and note that *this* axis, the roster, can only ever see the 47; Axis D sees the rest).
It was authored 2026-07-25 and never maintained. A hand list guarding a boundary that does
not physically exist is not a lapse waiting to happen — it is the *predictable* end state
of every hand list, and the same failure R5 diagnoses in TTW's scattered mapping data. The
census must therefore derive its roster from the artifact that cannot lie: what the build
actually compiles. If the census and the build can disagree, the census is worthless.

**Negative control (HousingTemp):** add a WW-named TU to `files.cmake` without touching
any list; the census must report it on the next run. If it does not, the instrument fails.

## 2. Axis C — graph closure

**Measure:** for each subsystem, walk the outbound call/type edges and classify each as
(a) **internal** — terminates inside the subsystem; (b) **platform** — OS/DVD/math/
allocator primitives the receiver also provides; (c) **receiver-native** — a system the
receiver already owns (`dComIfG*`, `dStage`, `fopAcM`, `dBgS`, player, camera, message);
(d) **receiver-absent** — needs new receiver code. Report **the edge list, not only the
percentage.**

**Reasoning.** Closure is the cost driver for wholesale porting. A closed subsystem
imports as a unit behind one adapter; an open one demands per-call-site shimming, and the
shimming *is* where the manual compile rounds go (pig: 5 rounds, 101→0 errors). This is
also why the existing API-surface numbers work as doctrine — "WW enemies are 86–92%
receiver-native" is exactly a closure measurement, just at actor granularity. The census
runs the same closure walk (already implemented in `enemy_port_kit` v3/v4) one level up.

The percentage alone is misleading, hence the edge list: a subsystem at 95% closure whose
remaining 5% reaches into `dStage` is harder than one at 85% whose remainder is all
platform. **Shape beats count.**

**Guidance, not a gate:** closure ≥ ~90% with the remainder in class (b) is a wholesale
candidate. Class (c)-dominated remainders mean piecewise — the receiver already has those
systems, so wholesale would duplicate the receiver rather than extend it.

## 3. Axis D — decomp completeness

**Measure:** per subsystem, the count of donor TUs that are genuinely decompiled vs
stubbed/`NONMATCHING`, **plus a stub roster with a reachability note** (is the stub on a
path our content will actually hit?).

**Reasoning.** zeldaret/tww is ~72% decompiled. Wholesale import silently imports the
other 28% as stubs, and **stubs do not fail at compile time — they fail when reached.**
That is HousingTemp's dormant-landmine class (§3b), the most expensive failure mode we
have, because it stays invisible until content triggers it, by which point the surrounding
work is already accepted and the search space has grown. Two live precedents: the
`GXCallDisplayList(NULL, 0x80)` FIFO fatal that was dormant only because nothing
instantiated Mat2, and aurora's indexed-XF handler broken for the entire life of the port
until WW content emitted one.

The existing per-actor gate (`grep -c "Nonmatching"` < 20) is the right instrument at the
wrong granularity — it gates one file when the decision being made is about twenty.

**Why a roster and not a percentage:** a 95%-complete subsystem whose missing 5% is the
one function our content calls is strictly worse than an 80%-complete one whose gaps sit
in unused paths. The number cannot express that; the roster can.

## 4. Axis W — interface width

**Measure:** enumerate (i) the distinct entry points the receiver calls into the
subsystem, and (ii) the callbacks the subsystem requires back from the receiver. Record
call sites `file:line`, both sides of every gate branch.

**Reasoning.** **This axis is the plugin ABI.** The union of the interface edges of all
WHOLESALE-verdict subsystems *is* the hook interface the receiver must expose — which is
why the plugin architecture cannot be costed today and can be the moment this runs. Width
also predicts the stack/leg split: a narrow interface can live as a parallel stack behind
one gate (the `JEvent1::` / `MDoExt1` precedent), while a wide or interleaved one becomes
legs inside receiver files (the vrbox precedent).

**Receipt standard** (inherited from HousingTemp §3a, non-negotiable): line numbers, both
sides of the branch, named file — *"all four entry points gated by `dKyWw_isSkyHost()` in
`d_kankyo_wether.cpp` (:479/:655/:1895/:1906), TP path in the else legs."* **"It's gated"
is not a receipt**, and comments claiming a gate have been wrong before.

## 5. Axis P — platform-layer depth (hard veto, not a score)

**Measure:** does the subsystem own or duplicate singleton runtime state the receiver also
owns (device/kernel/allocator/render-state singletons)?

**Reasoning.** Wholesale porting has a floor. Two JAudio stacks coexist fine — JAudio1 and
JAudio2 are separate players over separate data, and that is already shipping. Two J3D
implementations, two kernels, or two allocators mean symbol collision, duplicated global
state, and two runtimes fighting over hardware the receiver must own singularly. Depth is
therefore a **veto**, not a tradeoff: any subsystem that duplicates singleton runtime
state is PIECEWISE regardless of how well it scores on C, D, and W.

## 6. Axis B — donor-data payload (the membrane check)

**Measure:** scan each WW-layer TU for static/const arrays of donor origin; classify each
as GX register state / lookup table / display list / asset-like; tally bytes; compare to
the §5.1 trip-wire.

**Reasoning.** This is the one place the two-pipeline model genuinely leaks. Donor *code*
is governed by distribution scope; donor *data* is governed by Covenant №31, and a table
compiled into a TU crosses from the pipeline that has a law into the one that does not.
The user has **already ruled** on this surface (2026-08-05: `l_toonMat1DL`, 165 bytes,
keep under watch), and that ruling ships a defined trip-wire — *donor DL/data bytes
countable on one hand*. Nobody has ever counted at layer scale. The census makes the count
a by-product of a run rather than a project.

**Precedent for the remedy, already sanctioned:** `extract_veg_assets.py` pulls donor
arrays out of the donor executable into mod-folder blobs loaded at runtime. Anything this
axis finds routes to that pattern; the ruling's own stated long-term resolution (the
parallel-lane pattern, relocating donor data into a WW stack wholesale) is the same move
at subsystem scale.

## 7. Outputs

Per subsystem, one row plus its evidence:

| Field | Content |
|---|---|
| Roster | TUs, from `files.cmake` (§1) |
| C | closure %, full edge list by class |
| D | decomp %, stub roster + reachability |
| W | entry points + callbacks, `file:line` both sides |
| P | singleton-duplication verdict |
| B | donor byte tally by class |
| **Verdict** | **WHOLESALE / PIECEWISE / VETO** |

Four derived artifacts — the reason the census is worth building:

1. **The wholesale/piecewise call** per remaining subsystem.
2. **The plugin ABI surface** = union of W-edges over WHOLESALE rows. Sizes the boundary.
3. **The leg-migration debt** = every WW branch inside a receiver-owned TU, each with the
   hook it would need to become. This is the Engine work item list.
4. **The donor-byte tally** vs the §5.1 trip-wire → either closes the watch item or trips
   it to the user.

## 8. Build notes

Reuse, do not rebuild: the closure walk and rename/shim machinery from
`tools/foundry/enemy_port_kit.py` (v3/v4) and `api_surface.py` — they already do this at
actor granularity; the census generalizes the granularity. `files.cmake` is the roster
oracle. Emit JSONL + a markdown table, per the existing probe/differ conventions, so the
output diffs across runs and can enter the goldens.

**Provenance banner convention (new, cheap, high value).** Each WW-layer TU carries a
machine-readable header: donor source path, upstream decomp commit, decomp status. This is
what lets §1 generate the roster instead of pattern-matching filenames, makes lineage
auditable per file rather than per belief, and is the precondition for vendoring by
reference (§9 of the posture doc). Filename conventions (`ja1_`, `evt1_`, `d_ext_`) work
today only because everyone has been disciplined; a banner is checkable.

## 9. Negative controls (HousingTemp — before any output is used as evidence)

Per HousingTemp §3c, an instrument is audited *before* it judges:

- **Roster:** planted TU appears (§1).
- **Falsifiability:** a subsystem with no donor bytes must be able to report **B = 0** —
  if the scanner cannot produce a zero, it is not an instrument.
- **Completeness of validity sets:** the stub detector enumerated against the donor tree,
  not from memory (the CP-register set missing `0xB0–0xBF` produced false positives that
  consumed an anomaly budget).
- **UNKNOWN ≠ CLEAN:** a subsystem the walk cannot resolve reports UNKNOWN. Per №31-C, a
  check that cannot run must never report clean.

## 10. Honest limits

- Static closure misses runtime/vtable dispatch — undercounts edges. Report as UNKNOWN
  where dispatch is indirect, never as internal.
- Decomp % is a proxy for risk, not a correctness measure. A matching decomp can still be
  wrong for our use.
- Interface width undercounts coupling through shared global state — the census sees calls,
  not `g_env_light`-style implicit contracts. The §113 PAL0 stash is the standing example
  of coupling no call-graph would show.
- The census scores *portability*, never *correctness*. Nothing here replaces the differ,
  the state gate, or a playtest.
