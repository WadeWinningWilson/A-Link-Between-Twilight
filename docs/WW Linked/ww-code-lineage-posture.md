# WW code lineage posture — the second law (DRAFT)

> **Lane: Foundry drafted · USER RULES · Housing Security enforces · HousingTemp audits.**
> **Status: DRAFT, NOT IN FORCE.** Contains decisions only the user can make (§6).
> Nothing in this document authorizes anything; it names a surface that currently has
> **no written law at all.**
>
> Drafted 2026-08-06 on user assignment, after the TTW comparative review (§330) and the
> WW-layer discovery — originally stated as "46 WW-owned TUs in `dusklight.exe`".
> **That figure is corrected in §1.1 below** (HousingTemp, step 5, bus §491): it counts
> WW-*named* files, not WW lineage, and the lineage surface is materially larger.

## 1. The finding this exists to answer

Covenant №31 governs donor **bytes**: zero WW assets in the repo or the exe, user supplies
their own extraction, transforms ship instead of content. It is intact and verified —
Tier 3 is empty, no arcs under git.

But the project has **two donor pipelines**, and only one has ever had a law:

| | **Data pipeline** | **Code pipeline** |
|---|---|---|
| Source | user's WW ISO → `arcs/` | zeldaret/tww decomp source |
| Transform | `ww_bridge` + skeleton scripts | human transcription + shims |
| Destination | mod folder (AppData) | **`dusklight.exe`** |
| Governed by | **Covenant №31** | **nothing written** |
| Measured by | M6 greplist, `verify`, Tier-3 check | **nothing** |

**The M6 greplist cannot see the code pipeline.** It scans the exe for WW identity
literals — `Outset`, `Aryll`, `Makar`. Transcribed donor logic contains none, so every
`[('Ivan', 1)]` PASS is a true statement about *identity strings* that has been read as a
statement about *lineage*. Per №31-C — *a check that cannot run must report UNKNOWN, never
CLEAN* — the gate has been overclaiming by omission.

**The error was never the mixture. It was the missing second law.** One covenant and one
gate were built for two pipelines that need different governance, so everyone reasonably
assumed coverage that never existed.

### 1.1 Sizing the code pipeline — corrected statement

> **Restated by HousingTemp, 2026-08-06, roadmap step 5 (bus §491).** Restated *from*: the
> original §1 finding "46 WW-owned TUs in `dusklight.exe`" (Housing/Foundry, 2026-08-06),
> as recorded in `docs/state/ww-layer-manifest.json` and cited in
> [ww-subsystem-census-spec.md](ww-subsystem-census-spec.md). The superseded wording is kept
> above and struck here rather than deleted, so the correction is auditable.

**Superseded:** *"46 WW-owned TUs compiled into `dusklight.exe`."*

**Corrected:** **47 TUs are *named* by WW convention. That is a floor on the WW layer, not a
measure of it.** The lineage surface is **~60 donor-derived TUs, plus ~8 receiver-owned TUs
carrying donor legs** — roughly 30% larger than the superseded figure.

**Derived from** (independent re-derivation, not a re-reading of the manifest):
- **Roster axis** — `files.cmake` parsed directly for compiled `.cpp` sources, then filtered
  by the naming convention. Result: 44 convention hits + 2 declared judgment calls
  (`d_albw_dialogue.cpp`, `ext_line/mdoext1_3dline.cpp`) = the manifest's 46, **plus
  `src/d/d_kankyo_ww.cpp`**, which the convention's `_ww_` *infix* test cannot match against
  a `_ww.cpp` *suffix*. It is 486 lines whose own header reads *"donor-verbatim … from WW DP
  src/d/d_kankyo.cpp:1763-1873"*, and its child `d_kankyo_ww_sky.cpp` was already listed.
  **⇒ 47.**
- **Lineage axis** — every compiled TU scanned for in-file donor-authority markers
  (`DIRECT PORT`, `body VERBATIM from the WW donor`, `WW DP …`, `KIT-LINEAGE: native-port`,
  donor decomp paths). 22 such TUs are neither manifest members nor convention-named. Of
  these, **13 are whole-file WW donor ports wearing TP filenames**: `d_a_npc_ba1`,
  `d_a_npc_bm1`, `d_a_npc_ls1`, `d_a_npc_zl1`, `d_a_lamp`, `d_a_kamome`, `d_a_esa`,
  `d_a_knob00`, `d_a_obj_mshokki`, `d_a_obj_otble`, `d_a_obj_toripost`, `d_a_spc_item01`,
  `d_a_demo00`. The remaining ~8 (`d_door`, `d_stage`, `d_particle`, `d_event_data`,
  `d_a_swhit0`, `f_op_msg_mng`, `f_pc_profile_lst`, `JASChannel`) are receiver-owned files
  carrying donor legs — the leg-migration debt class, counted separately and **not** folded in.

**The reasoning — why the original figure could not have been right, and why that is not a
lapse.** A filename convention measures what a file is *called*; lineage is what its body
*is*. The two coincide only for TUs created by this project (`d_ext_*`, `d_a_ext_*`). They
diverge exactly where the donor port is most faithful: a direct port of WW's `d_a_lamp.cpp`
is *correctly* named `d_a_lamp.cpp`, because donor-native naming is itself a standing project
rule. **So the naming convention is structurally blind to precisely the most donor-faithful
work in the tree, and gets blinder the better the porting gets.** The manifest never claimed
otherwise — its `basis` field says `"filename-convention (NOT lineage)"` in as many words.
The defect was never the measurement; it was that every prose citation of the number dropped
that qualifier, and "WW-owned TUs in `dusklight.exe`" reads as a lineage claim.

**That is the same error this section diagnoses, one level up.** §1 above establishes that
`[('Ivan', 1)] PASS` is a true statement about identity strings that has been read as a
statement about lineage. "46 WW-owned TUs" is a true statement about filenames that has been
read the same way. The pattern recurred inside the document that names it — which is the
argument for the standing rule below, not evidence against the analysis.

**Standing rule for this figure.** Cite it as *"N TUs **named** by WW convention — a floor"*,
never as *"N WW-owned TUs"*. The lineage count is the census's output (Axis D, roadmap step
8), not a filename's. Until the census runs, the code-pipeline size is **UNKNOWN with a floor
of 47**, per №31-C.

**Consequence for the rulings in §6.** They are the user's, and they are argued below against
the *scope* of the WW layer. They should be made against ~60 donor-derived TUs plus ~8
leg-carriers, not against 46. The rulings may well come out identically — the argument in §2
turns on lineage *class*, not on count — but the count must be the corrected one when it is
weighed.

## 2. Why the code pipeline cannot be TTW-shaped (the reasoning)

TTW's architecture — ship transforms, regenerate from user-owned files — is why §330 scored
our **asset** architecture as a MATCH. It cannot extend to code, for a structural reason:

**TTW's donor was data-only, and their host engine binary was the user's own.** They never
distributed an engine; their sole engine-side change was a 4GB LAA *patch applied to the
user's `FalloutNV.exe`*. Every byte of Bethesda's engine stayed on the user's disk.

We distribute a binary. `dusklight` **is** a source port of the TP decomp — every byte of
that exe is already Nintendo-derived decomp code. The moment the project became a source
port, its code half entered the **decomp-port lineage** (Ship of Harkinian, sm64pc class:
ship code, never assets), which is a different architecture with a different containment
mechanism. The WW layer did not move the project off the TTW path; **it extended a posture
the project has had since its first commit.**

**The consistency argument.** WW-decomp code in the exe is the same lineage class as the
TP-decomp code already filling it. If TP-decomp code in a distributed binary is acceptable
to this project, WW-decomp code is not a new *kind* of liability. If it is not acceptable,
the whole project fails, not the WW layer.

**The real asymmetry is scope, not lineage.** Public dusklight is a *Twilight Princess*
port — that is its charter and its public identity. A second property in the same binary
doubles the rightsholder-relevant surface and changes what the project publicly claims to
be. This is why fork-locality was the right instinct even though the lineage argument says
"same class," and why **the project's own charter is the binding constraint here, not the
legal one.** (Where the question is genuinely legal, it is the user's and counsel's, not
any lane's.)

## 3. The membrane rule (the one place the mixture actually leaks)

**Donor data must not cross into the code pipeline.** A donor table, display list, or byte
array compiled into a TU has moved from the pipeline that has a law into the one that does
not, and the greplist cannot see it.

Standing ruling (user, 2026-08-05, [HOUSINGTEMP-HANDOFF.md](../HOUSINGTEMP-HANDOFF.md)
§5.1): `l_toonMat1DL` — 165 bytes of donor display list, GX register state only — **stays,
under watch**, with three trip-wires: (a) the byte-gate hits on it, (b) **donor DL/data
bytes in the exe stop being countable on one hand**, (c) a parallel WW render lane makes
relocation free.

**Trip-wire (b) has never been measured at layer scale.** Census axis B
([ww-subsystem-census-spec.md](ww-subsystem-census-spec.md) §6) is the measurement.
Sanctioned remedy when found: the `extract_veg_assets.py` pattern — donor arrays leave the
TU for mod-folder blobs loaded at runtime.

## 4. Containment primitives, in ascending strength

**Why ranked:** the project has been relying on the weakest one and calling it containment.

1. **Fork-locality (current).** The WW layer never reaches public `main`. This is
   real — it is structurally the same answer TTW used for a decade, where their private
   BuildDB held all of Fallout 3 and that was fine because *distribution* is the act that
   matters. **The fork is our gold master.** One difference demands more discipline than
   they needed: TTW's gold master was a build database, while ours is a tree that produces
   a runnable exe someone could hand to a person.
2. **Parallel stacks over legs.** A subsystem in its own files and namespace behind one
   gate (`JEvent1::`, `MDoExt1`) is deletable by removing the file and its gate call. A
   *leg* — a WW branch interleaved inside a receiver-owned file (vrbox) — cannot be removed
   without editing TP. **Every leg is a liability; every parallel stack is an asset.** Three
   independent findings converged here: the reporting lane's separability result, the §5.1
   ruling naming the parallel-lane pattern as the long-term resolution, and DN-9's
   always-native direction.
3. **Build exclusion.** A shipped-build configuration that compiles the WW layer out,
   gated by the *generated* manifest. Converts "we won't push it" into "it cannot be built
   in." Cheap for stacks, blocked on legs — which is the migration debt.
4. **Plugin boundary (the destination).** The WW layer lives in a separate repo producing a
   separate artifact against a hook ABI. Leaving a WW file in dusklight's tree becomes
   **impossible rather than forbidden** — the R5 lesson (generated, not curated) applied at
   the architecture level. This retires the entire failure class that produced the stale
   Tier-1 list.

**What the plugin boundary does and does not do.** It moves the exposure **off dusklight
and onto this mod** — which is exactly how the modding ecosystem isolates risk, and exactly
the relationship TTW has with Obsidian, who carry no risk from TTW because TTW never
touched their tree. It does **not** make donor-derived code stop being donor-derived. A
distributed prebuilt plugin still contains what it contains.

## 5. Wholesale vs piecewise — the porting rule

Ratified reasoning from the census spec, recorded here as doctrine:

- **Wholesale-port a subsystem when its dependency graph closes** — dependencies terminate
  inside itself plus platform primitives, and it meets the rest of the game through a narrow
  interface. JAudio1 is the proof: ~22 TUs lifted as a unit, shipping. JStudio/JEvent1 and
  JPA are the same shape.
- **Port piecewise when the closure is dominated by systems the receiver already owns.**
  One NPC's closure reaches `dComIfG*`, `dStage`, `fopAcM`, `dBgS`, player, camera, message.
  Wholesale would duplicate the receiver instead of extending it; shimming to TP equivalents
  *is* the work, and the API-surface numbers (pig 92.3%) are the measurement of how cheap
  that is.
- **Hard floor:** no duplication of singleton runtime state. Two JAudio stacks coexist; two
  J3D or two kernels do not.
- **Velocity gate:** wholesale import is a velocity play, and velocity without measurement
  is precisely how ~60 donor-derived TUs accumulated unswept (§1.1). Each subsystem lands with its differ site
  armed and its manifest entry generated. **Import no faster than you can measure.**

## 6. OPEN — user rulings required (nothing below is decided)

1. ~~**Does any user run a build produced from this tree?**~~ **RULED 2026-08-06 (user):
   NO — nobody builds from this tree; it is not finished.**
   Consequence, stated so it is not re-litigated: the posture is **CONTAINED**. Build
   exclusion (step 11) is a roadmap item, **not** an emergency path. What remains on this
   branch is stale paperwork and an overclaiming gate — both already owned (§7.1, §7.2).
   This does **not** retire the containment work; it sets its urgency.
2. ~~**Vendoring by copy or by reference?**~~ **RULED 2026-08-06 (user): BY PINNED
   REFERENCE — the official online decomp.** The pin is `zeldaret/tww`, and the reference
   this tree is pinned to today is commit
   **`be8da688fcc755d77e2cdb7a69124297b01ff683`** (2026-07-27, "zl1 cleanup, inline
   cleanup"). Verified at ruling time: the local donor checkout
   (`D:/XXXXXXX/WW DP`) is a real clone of that remote, its working tree is CLEAN
   (0 modified files), and HEAD is contained in `origin/main` — so the pin is public and
   reproducible rather than a local snapshot nobody else can resolve.

   This closes the precondition E5 was blocked on. Mechanism work (actually vendoring by
   reference rather than copy) remains roadmap steps 12/13 and is a larger job than the
   banner backfill.

3. **Distribution form of the WW layer**, once the plugin boundary exists: prebuilt binary
   (lowest friction, exposure sits on this mod), source + user builds (donor-derived code
   arrives on the machine that builds it — the TTW architecture applied to code), or
   runtime derivation (only reachable for the interpreter-shaped subsystems; the ported
   actor classes cannot get there without recompilation).
4. ~~**Has trip-wire (b) tripped?**~~ **RULED 2026-08-06 (user): YES — (b) IS TRIPPED.**
   Measured by census axis B via the step-10 `KIT-DONOR-DATA` markers (E4):
   **1,509 declared donor bytes over 19 arrays** (1,174 lookup-table / 315 display-list /
   20 asset-like) — against the 1 array / 165 bytes that the 2026-08-05 ruling watched.
   Not countable on one hand under any reading.

   **Disposition (user): watched now, FULLY EXTRACTED at the appropriate time.** The
   donor arrays do not stay permanently; the sanctioned remedy (`extract_veg_assets.py`
   pattern — donor arrays leave the TU for mod-folder blobs loaded at runtime) is
   ACCEPTED and DEFERRED, not declined. Urgency is set by §6.1: nobody builds from this
   tree, so the posture is CONTAINED and this is scheduled work, not an emergency.

   **The plugin boundary does NOT retire this item** (user asked; §4 answers). The plugin
   moves the WW layer out of dusklight's exe — that is containment primitive 4, the
   destination — but §4's own closing paragraph is explicit: it "does not make
   donor-derived code stop being donor-derived. A distributed prebuilt plugin still
   contains what it contains." The plugin answers *"not in dusklight"*; the extraction
   answers *"not in any shipped binary"*. Only the second satisfies the standing
   constraint that dusklight ship no WW code or assets, so the two are sequential, not
   alternatives.

## 7. Corrections owed (detail + routing in the bus entry)

1. Manifest generated from `files.cmake`, replacing the 12-files-stale hand list — **Bridge**.
2. M6 gate reports its scope (`identity literals: CLEAN; code lineage: UNKNOWN`) — **Bridge**
   builds, **Housing Security** rules, **HousingTemp** negative-controls.
3. Tier-1 refresh in [NEVER-PUSH-STRIP-SET.md](../NEVER-PUSH-STRIP-SET.md) from the
   generated roster — **Housing Security** (owns the doc).
4. Donor-byte sweep vs the §5.1 trip-wire — **Foundry** builds census axis B, **user** rules.
5. §330 scorecard correction: the TTW legal-architecture MATCH is **asset-only** — **Foundry**.
6. Provenance banners on WW-layer TUs — **Foundry** specs, **Engine** lands.
7. **Reclassify the aurora fixes out of the WW layer.** The `LOAD_INDX` fixes (§449/§450)
   (see §8 step 14)
   are *platform bug fixes* discovered via WW content — indexed loads were broken for the
   entire life of the port. They are not WW code, they belong **upstream**, and filing them
   under WW containment both burdens this project and deprives dusklight of fixes it needs.
   They owe a mainline TP battery first (HOUSINGTEMP §5.2) — **Engine/Housing**, not Foundry.

## 8. The road — sequential steps with owners

**Critical path:** 2 → 6 → 7 → 8 → 15 → 16 → 19. Everything else is parallelizable around it.
**Startable today with no predecessor:** 1, 2, 4, 5, 12, 14.

### Stage A — TRUTH (nothing below is trustworthy until these land)

| # | Owner | Step | Blocked by |
|---|---|---|---|
| 1 | **USER** | ~~Rule: does any user run a build produced from this tree?~~ **DONE 2026-08-06 — ruled NO.** Posture is CONTAINED; build exclusion is a roadmap item, not an emergency. | — |
| 2 | **Bridge** | ~~Generate the WW-layer manifest from `files.cmake`.~~ **DONE** — `tools/ww_crew_restoration_skeleton/ww_layer_manifest.py`; the census CONSUMES it (HT-15) rather than re-implementing, so one parser / one denominator. | — |
| 3 | **Housing Security** | ~~Refresh Tier-1 in NEVER-PUSH-STRIP-SET from the generated roster.~~ **DONE 2026-08-06** — Tier-1 is now a GENERATED block (`--emit-tier1`, 47 sources) with a `--check` drift gate. Verified by Foundry. | 2 |
| 4 | **Bridge** builds · **Housing Security** rules · **HousingTemp** negative-controls | ~~Gate reports scope.~~ **DONE 2026-08-06** — `covenant_gate.py` states its scope and what it cannot detect. Verified by Foundry. | — |
| 5 | **HousingTemp** | ~~Independently verify the 46-TU finding (the reporting lane cannot audit itself).~~ **DONE 2026-08-06 (§491).** Verdict: CONFIRMED as a filename-convention count of compiled TUs; **REJECTED as a lineage count**. Roster corrected to 47 (`d_kankyo_ww.cpp` omitted); lineage floor ~60 + ~8 leg-carriers; finding restated in §1.1. Gate negative-control passes. | — |

### Stage B — MEASURE

| # | Owner | Step | Blocked by |
|---|---|---|---|
| 6 | **Foundry** | ~~Build the census instrument.~~ **DONE** — `tools/foundry/ww_census.py` + axes B/C/D/P/W; it produced the step-8 run. | 2 |
| 7 | **HousingTemp** | ~~Negative-control the census before any output is used as evidence (spec §9).~~ **DONE** — §494 "step 7 RE-RUN #2 — all six controls PASS. Step 7 DISCHARGED" (`ww-tale-dmesg-live-state.md:4461`); §500 confirms discharge across all five axes (:4884). Verified by Housing 2026-08-06. | 6 |
| 8 | **Foundry** | ~~Run it.~~ **DONE 2026-08-06 — all four artifacts.** verdicts (adopted, §1) · plugin-ABI surface (lower-bounded) · leg-debt list (**14 legs / 11 TUs / 447 lines**, floor — was 11/8/281; rose when step 10/E4–E5 landed `KIT-DONOR-HUNK` markers on three further TUs and one existing leg grew, itemised in adoption record §1c) · **donor-byte tally 1,509 bytes over 19 arrays** — the 4th needed step 10's E4 markers to be rulable, which is why it landed last. | 7 |
| 9 | **USER** | ~~Rule on trip-wire (b).~~ **DONE 2026-08-06 — ruled TRIPPED** (1,509 bytes / 19 arrays vs the 1 array / 165 bytes watched). Disposition: watched now, FULLY EXTRACTED at the appropriate time; the plugin boundary does NOT retire it (§6.4). | 8 |
| 10 | **Foundry** specs · **Engine** lands | ~~Provenance banners on WW-layer TUs.~~ **DONE 2026-08-06 — E1–E5 all landed.** 72/72 TUs bannered, 0 DISAGREES; 11 hunk markers, 19 donor-data markers, 72 pinned refs. Build green. | 2 |

### Stage C — CONTAIN (parallel with B once the numbers exist)

| # | Owner | Step | Blocked by |
|---|---|---|---|
| 11 | **Bridge** + **Engine** | ~~Build-exclusion configuration, gated by the generated manifest.~~ **BUILT 2026-08-07 (Housing covering Engine).** `option(DUSK_EXCLUDE_WW ... OFF)` in `CMakeLists.txt` after `set(DUSK_FILES ...)`, including the generated `cmake/ww_layer_exclude.cmake` and `list(REMOVE_ITEM ...)`. Carries a **`files.cmake` SHA256 drift guard that FATAL_ERRORs** rather than silently excluding a stale roster — a gate run against a roster that forgot a file says nothing about the file it forgot. Emits a PARTIAL warning when ON, because it is partial by definition. **ON-PATH VERIFIED 2026-08-07 (§572), all three branches exercised:** (1) `-DDUSK_EXCLUDE_WW=ON` configures clean and reports **`removed 48 of 48 listed TUs (1358 -> 1310 sources)`** — every listed TU was present and removed, no silent "listed but absent" cases; (2) **drift guard FIRES** — stamp corrupted → `CMake Error ... list is STALE`, exit 1, naming both hashes, and it refuses BEFORE excluding rather than under-excluding; (3) **missing-list guard FIRES** — file removed → `is missing. Generate it:`, exit 1. **LINK-TIME RESOLVED AS UNREACHABLE UNTIL 19 — proof, not conjecture (§577).** Three runs: 194 → 61 → 129 unresolved. The first drop removed self-inflicted noise (the roster was re-based twice: filename → declared lineage §574 → *what moves to the plugin* §576, after §573 measured the filename basis wrong in BOTH directions — 14 host-plumbing files swept in, 22 donor-lineage TUs missed). The RISE to 129 is not a regression: the bridges are **bidirectional**, so removing them exposes the receiver's own dependencies. **Therefore unresolved count is not a progress metric** — it measures how much of the WW layer's API the receiver touches, and excluding more reveals more. It converges only when CALL SITES become hooks. Step 11's link test **cannot pass before step 19, by construction**; re-running it has no information left to give. The 129 symbols across 86 TUs are the measured **host → plugin inbound boundary** (complement of 19a's 15 outbound). Step 11 is therefore **DONE to its limit**: mechanism, both refusal guards, and criterion all verified; the residue is step 19's work list. | 2 (scope from 8) |
| 12 | **USER** | ~~Rule: vendor donor sources by copy or by pinned upstream reference (§6.2).~~ **DONE 2026-08-06 — ruled BY PINNED REFERENCE**, pin = `zeldaret/tww@be8da688` (§6.2). | — |
| 13 | **Engine** + **Bridge** | ~~Vendoring rework to pinned reference.~~ **DONE 2026-08-06 (Housing covering Engine).** Mechanism C: `tools/vendoring/ww_rebaseline.py` (3-way merge against the pin; no patches stored — a diff carries donor text). First re-baseline EXECUTED: pin → `1d57f046`, **0 commits behind**. | 12 |
| 14 | **Engine** + **Housing Security** | ~~Aurora `LOAD_INDX`: mainline TP battery → reclassify → upstream.~~ **BATTERY DONE + RECLASSIFIED + PATCH PREPARED 2026-08-07** (§557). Fix verified field-for-field against the SDK's own `GDBase.h GDWriteXFIndx[ABCD]Cmd`; opcode is emitted by CORE J3D (`J3DShapeMtx::loadMtxIndx_PNGP`, `J3DSys`) through dusklight's OWN `libs/dolphin` writers → platform bug, not WW code. Clean probe-free patch at `docs/state/aurora-load-indx-upstream.patch`. **PUSH is the user's act — not done.** | — |

### Stage D — THE BOUNDARY — **PREMISE FALSIFIED 2026-08-06 (§519); rows below superseded by 19a–19c**

> **What happened.** Steps 15–17 assumed *we* must design and build the boundary. We do not own
> dusklight — so rather than assume, Foundry checked what it already exposes. **Dusklight ships
> the entire thing.** Verified independently for this rewrite: `sdk/include/mods/` —
> `MOD_ABI_VERSION 1u` (`api.h:26`), `struct_size` forward-compat with per-service major/minor
> negotiation, eleven service headers (camera, config, game, gfx, hook, host, log, overlay,
> resource, texture, ui), `hook.hpp`'s `DEFINE_HOOK` with host-resolved metadata records, and
> native module loading at `src/dusk/mods/loader/native_module.cpp`.
>
> **Consequences.** 15/16/17 are moot — **we design no ABI**, loading exists, and it was never
> ours to promote. 18's empty list now **costs nothing**: a hook-based mod relocates nothing, it
> attaches by address, so subsystem separability stopped gating the boundary (the census's
> wholesale/piecewise verdict remains valid and useful for judging *coupling cost*, but it no
> longer decides feasibility). 19 is superseded — there is nothing to migrate.
> **Step 20's "the ABI becomes a PUBLIC CONTRACT" is also void:** we bind to dusklight's symbol
> table, so *"works on any dusklight build"* is a **testable property, not a contract we own**.
> Surface narrowing drops from urgent to hygiene.
>
> **The long pole was mis-identified.** It is not plugin loading. It is the set of imports
> reachable by **neither** a published service **nor** a symbol — and nobody has measured it.
>
> **Root cause, recorded.** The original roadmap took *"native plugin loading is deferred"* from
> a lane report and never checked it against the tree. One unverified premise cascaded into three
> steps. **This is the third instance of the same defect in this document** — §1 (gate scope),
> §1.1 (filename vs lineage), and now this. The standing fix does not change: check the artifact,
> never the summary.

| # | Owner | Step | Blocked by |
|---|---|---|---|
| 15 | ~~**Foundry** specs · **Engine** builds~~ | ~~Hook ABI, derived from census axis W.~~ **MOOT — VERIFIED 2026-08-07 (§558).** Dusklight already ships it: `sdk/include/mods/api.h`, `MOD_ABI_VERSION 1u`, services + `resolve()`. Replaced by the BINDING PLAN ([spec](ww-plugin-binding-spec.md)). No ABI is designed by us. | 8 |
| 16 | ~~**Engine**~~ | ~~Native plugin loading. **The long pole.**~~ **MOOT — VERIFIED 2026-08-07 (§558).** `src/dusk/mods/loader/` exists and ships. **The long pole was MIS-IDENTIFIED**: it is not loading, it is the (c) set — and that measures **15 symbols / 197 sites**. | 15 |
| 17 | ~~**Engine** + **USER**~~ | ~~Promote the ABI upstream to dusklight.~~ **MOOT (§519).** It is already upstream because it was never ours; and `origin` IS dusklight — we cannot promote to a project we do not own. | 16 |
| 18 | **Engine** | ~~Relocate WHOLESALE-verdict subsystems into the plugin. Cheap; they are already namespaced + gated.~~ **WORK LIST IS EMPTY (§512)** — step 8 returned 8 PIECEWISE / 2 VETO-PENDING / **0 WHOLESALE**. Not blocked, not deferred: nothing is in it. Needs re-scoping or acceptance — USER + Bridge/Engine. | 16 |
| 19 | **Engine** | ~~Leg → hook migration, one at a time, each with its kill switch. Long tail.~~ **SUPERSEDED (§519)** — hook binding attaches by address; there is nothing to migrate. Replaced by 19a–19c. | — |
| **19a** | **Foundry** | Classify the ~1,150 WW imports into **service-reachable / symbol-reachable / neither**. **The "neither" set is the real long pole and is unmeasured.** Use the SDK's own record-kind taxonomy (`ModMetaExport` / `HookFn` / `HookMem` / `HookMemExt` / `HookName`) rather than inventing categories — the classification schema already exists. | live (Foundry's next block; needs nothing from the user) |
| **19b** | **Foundry** | Symbol-resolution conformance check: resolve the WW import list against a target build → **RESOLVED / MISSING / FOLDED / LOCAL-ONLY**; run against our fork, pure upstream, and every future release. **The diff between runs is what turns "works on any dusklight build" from assumption into evidence.** | 19a |
| **19c** | **Foundry** specs · mod-side lands | ~~Load-time import-manifest gate~~ **MOD-SIDE LANDED 2026-08-07 (§559/§560).** `d/ext_plugin/ww_import_gate.*`. C1/C2/C4/C6 **PASS on a real boot** (log 021244: C6 counted 2 missing + 1 folded, i.e. refusal is total). **C5 remains open — it needs an SDK record kind for resolve-then-CALL and dusklight is not ours to change.** | 19b |
| 20 | **USER** + counsel | ~~Distribution form.~~ **RULED 2026-08-06 — (A) PREBUILT PLUGIN**, shipped as the sole end product, loadable into **any** dusklight build. User supplies their own WW ISO/arcs to the mod folder for DATA (already how the tree works: `<mod folder>/arcs/*.arc`). **Consequence: the ABI becomes a PUBLIC CONTRACT, not an internal interface** — see step 15/17. | 18, 19 | *(the PUBLIC-CONTRACT consequence is void per the §519 note above; ruling (A) itself stands, and it is no longer blocked by 18/19)*

#### 8.1 The gap that survives — refusal exists, but only lazily

`hook_install` already returns `MOD_UNAVAILABLE` when a target fails to resolve
(`sdk/include/mods/hook.hpp:166-170`), so an unresolved hook does **not** crash. But that
refusal is **per-call-site and lazy**: a mod can load clean, run for an hour, and a hook that
was never installed simply means a behaviour silently never happens. The service layer
negotiates up front (`struct_size` / minor checks); **symbol bindings have no equivalent.**
That is the precise shape of *"symbol-resolution bindings carry no version negotiation at all."*

**19c closes it:** validate the entire import manifest at load and refuse to initialize on
MISSING/FOLDED, mirroring the posture the service layer already takes.

**Why FOLDED is a correctness gate, not an availability one.** ICF folds identical functions to
one address; a hook resolved by name then silently intercepts *every* function folded onto that
address, including unrelated ones. Nothing at runtime can detect this — only an offline check
against the build's symbol table and address map can. `ModMetaHookName`'s own header comment
names the exposed class exactly: *"targets that cannot be named in C++ (file-local statics,
private members)"* — the LOCAL-ONLY class, already enumerated by the SDK.

> **Step 7 — RESOLVED 2026-08-06, and the question was my error (Housing).**
> I reported "no closure record exists anywhere in the bus or the adoption
> record" and framed it as an open two-way question. The record existed the whole
> time, in a third document: `ww-tale-dmesg-live-state.md:4461` (§494, "Step 7
> DISCHARGED") and :4884 (§500). I searched two of the three places it could be
> and reported a gap as a finding. Kept rather than deleted because the failure
> mode is the point: **an audit of staleness run with a stale sweep** — the same
> narrow-filter defect this campaign has now logged in three lanes.


### Throughout

**History** — actor ports continue piecewise on existing kit doctrine, unblocked, with no
dependency on any step above. Piecewise for actors is the *correct* answer (§5), not a
constraint imposed by this roadmap.

## 9. The end-state workflow (what step 19 buys)

After step 19 the tree splits three ways: **`dusklight`** = pure TP receiver + hook ABI, no
WW · **`ww_layer`** (separate repo) = the WW subsystems, the ported actor classes, and their
registrations · **mod folder** = donor assets, converted from the user's own `arcs/`.

Four paths, in descending frequency. **Only the last one touches dusklight's tree** — that is
the whole point of the boundary.

| Path | Steps | Touches dusklight? | Rebuild |
|---|---|---|---|
| **Assets only** (new island's arcs, textures, audio) | user's `arcs/` → `convert-all` (R1) → mod folder; verified by `donor_roster` (R3) + output hashes (R2) | No | None |
| **New WW actor** (the common case) | decomp-status gate → `actor_kit` apply tier → TU lands in `ww_layer` → registration through the ABI's profile/objname hook → assets staged as above → state gate + differ | No | Plugin only |
| **New WW subsystem** (wholesale) | census axes C/D/P → WHOLESALE verdict → import as a unit into `ww_layer` behind its namespace → arm its differ site | No, *if* the hooks it needs exist | Plugin only |
| **Needs a hook the ABI lacks** | propose ABI extension → review → land in dusklight → promote upstream | **Yes** | Receiver + plugin |

### What materially improves

1. **You test against a stock receiver.** Today every WW change rebuilds a fork exe that
   nobody else has, so "works in my fork" is unfalsifiable. Post-19 the plugin loads into the
   *shipping* dusklight build — the thing users actually run.
2. **Iteration decouples.** WW work rebuilds a plugin, not a full C++ receiver link.
3. **The kill switch becomes structural** — don't load the plugin. No `#define`, no gate
   audit, no "is it really compiled out?"
4. **Containment stops depending on discipline.** The §1.1 finding is the proof: 13 whole-file
   WW donor ports wear TP filenames (`d_a_lamp`, `d_a_npc_ba1`, …) precisely *because*
   donor-native naming is a project rule, so no naming convention or hand list could ever
   have caught them. A **repo boundary catches them automatically** — the structural fix
   solves the measurement problem too.
5. **The covenant gate scopes correctly.** dusklight's gate has no WW to look for; the WW
   gate runs in the repo that owns WW.

### New hazards this introduces (honest)

> **Corrected 2026-08-06 (§519).** The first two hazards below were written against the false
> premise that we design the ABI. Service-level version skew is already handled by dusklight
> (`struct_size` + per-service major/minor), and "hook coverage gaps" was the wrong frame — the
> real exposure is *symbol* binding, which has no negotiation at all. Replaced with the measured
> set:

- **Build-dependent symbol availability.** A stripped release can drop a local that our fork
  exposes. Availability is a property of the *target build*, not of our code — which is why 19b
  runs against fork, pure upstream, and every future release, and why the diff is the evidence.
- **ICF folding.** Identical functions fold to one address; a name-resolved hook then silently
  intercepts unrelated folded functions. Undetectable at runtime; offline-only (see §8.1).
- **No negotiation on symbol bindings.** Refusal exists but is lazy and per-call-site (§8.1) —
  a mod loads clean and a behaviour silently never happens. Closed by 19c.
- **Debugging across the boundary.** Symbolication, crash `rva=` resolution, and the
  Build-ID↔PDB fail-closed check all need extending to cover a second binary. *(Unchanged —
  this one survived the correction.)*

### What does not change

DECOMP-FIRST · the oracle stack and every measurement instrument · `ww_bridge` asset
conversion · piecewise actor porting via `actor_kit` · the differ, state gate, and golden
traces. **The boundary changes where code lives and who carries the risk — not how the
project decides what is correct.**

### Velocity — what this does and does not buy (expectation gate)

**Do not sell this roadmap as a speed-up of porting.** It is a de-risking and *unblocking*
investment. ~~Steps 15–19 are substantial Engine work that produces zero new content.~~
**Corrected 2026-08-06 (§519): that cost estimate has collapsed.** 15/16/17 are moot (dusklight
ships the ABI and the loader), 18 is empty, 19 is superseded. The remaining Stage D work is
**19a–19c — three Foundry measurement/tooling items, not an Engine build-out.** The boundary is
far cheaper than this document originally claimed. What does *not* change is the conclusion
above: none of it makes porting an individual actor faster, and **step 8 still pays for
throughput before Stage D does.**

*Faster:* the debug loop (kill switch becomes load/don't-load instead of a `#define` +
rebuild — this is the underrated one, since "is the WW layer causing this?" currently costs a
full relink) · plugin rebuild vs receiver relink · no fork-specific bug hunts, since testing
runs against the shipping receiver · containment overhead disappears (no strip-set upkeep, no
covenant sweeps over the receiver tree).

*Unchanged:* **the cost of porting any individual actor.** Decomp availability, shim
authoring, compile-iterate rounds, measurement — none of it is touched by a repo boundary.

*The real second-order win:* **it removes the ceiling.** Today every WW addition grows an
unmeasured, hand-contained liability surface, so the project must periodically stop and audit
(this document is one such stop). Post-19, adding WW work does not accrue containment debt,
so porting proceeds continuously. Not "each port is faster" — **"you stop having to pause."**

*Where velocity actually comes from* (separable from this roadmap, already in flight):
`actor_kit`'s apply tier (§329) · crash-recipes-as-lint (§331 A1, queued) · the
prediction-vs-actual feedback loop · **census step 8**, which ends discovering
wholesale-vs-piecewise by *attempting* it · and upstream decomp coverage, which no instrument
here can change. **If the goal is throughput, step 8 pays before step 19 does.**

**Two hard gates:** do not cost or start Stage D before step 8 (the ABI's size is unknown
until the census runs). Do not scale porting further before Stage A completes.
