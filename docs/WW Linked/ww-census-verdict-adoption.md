# WW census — verdict ADOPTION record and implementation path

> **Lane: Foundry (record) · USER adopts · HousingTemp audits · decode work routes out.**
> **Status: ADOPTED PROVISIONALLY**, pending HousingTemp's audit of step 8's verdict
> logic. Nothing here authorises Stage D work; it authorises *planning against*
> these verdicts with their incompleteness attached.
>
> User direction, 2026-08-06: *"adopt each but substantially note where they are
> incomplete… implementation should follow a clearly documented implementation
> path that is reviewable and more importantly updatable with future
> findings/decoding… if it is possible to decode such matters to unveil it, such
> work should be done in another lane to fully decompile those (though we'd
> ultimately take the greater decomp's ruling per the updatable provision)."*

## 0. How to read this document

Each verdict below is stated with three things, and **a verdict without all three
is not adopted**:

1. **INPUTS** — the axis figures it rests on.
2. **INCOMPLETE BECAUSE** — what the census could not see. Not a disclaimer: the
   specific blind spot, in the direction it biases.
3. **REVISIT TRIGGER** — the concrete finding that would change this verdict.

The trigger is what makes this record *updatable* rather than a snapshot. A
verdict whose trigger fires is not "wrong"; it is due for re-derivation, and the
trigger says who can fire it.

**Standing supersession rule (user, 2026-08-06):** where a verdict rests on the
donor decomp's current state, **the upstream decomp's eventual ruling wins over
ours.** Our figures are the best available today, not a claim about the finished
work.

## 1. The verdicts, as adopted

Run: step 8, all five axes, roster 73 TUs over 1523 build sources. Verdict rules
are spec §2/§5, applied mechanically; the tool never self-clears and never
self-vetoes.

| # | Subsystem | Verdict | Closure | recv% | **GATE** | total surface | D cov/dec | Stubs |
|---|---|---|---|---|---|---|---|---|
| V1 | JPA (WW particle) | PIECEWISE | 84.8% | 14.3 | **UNKNOWN** | 15–18 | 100 / 50 | 2 |
| V2 | JAudio1 / sequence | PIECEWISE | 73.7–74.9% | 21.2 | **14–160** | 115–261 | 90.9 / 89.6 | 5 |
| V3 | JEvent1 | PIECEWISE | 26.8% | 69.6 | 2–3 | 41–42 | 66.7 / 100 | 0 |
| V4 | MDoExt1 (3D line) | PIECEWISE | 2.9% | 26.5 | 0–4 | 9–13 | 100 / 0 | 1 |
| V5 | JSystem legs | PIECEWISE | 18.5–35.4% | 74.1 | **UNKNOWN** | 40–55 | 100 / 100 | 0 |
| ~~V6~~ | ~~WW direct-port actors~~ | ~~PIECEWISE~~ | ~~43.9–45.5%~~ | ~~53.9~~ | ~~1–101~~ | ~~577–677~~ | ~~81.2 / 64.3~~ | ~~5~~ |
| V6 | WW direct-port actors | PIECEWISE | 48.2–49.9% | 49.5 | 1–103 | 465–567 | 100 / 84.6 | 2 |
| V7 | WW engine legs (`_ww`) | PIECEWISE | 34.3–37.3% | 47.7 | 48–54 | 214–220 | 50 / 80 | 1 |
| ~~V8~~ | ~~receiver engine TUs w/ legs~~ | ~~PIECEWISE~~ | ~~38.5–40.5%~~ | ~~53.8~~ | ~~123–240~~ | ~~678–795~~ | ~~90 / 66.7~~ | ~~3~~ |
| V8 | receiver engine TUs w/ legs | PIECEWISE | 38.4–40.1% | 55.7 | 122–230 | 736–844 | 91.7 / 72.7 | 3 |
| V9 | WW actors (ext) | **VETO-PENDING** | 19.9–26.7% | 68.5 | **UNKNOWN** | 189–213 | 75 / 77.8 | 2 |
| V10 | WW host systems (`d_ext_`) | **VETO-PENDING** | 38.2–42.4% | 55.4 | 100–153 | 626–679 | 70 / 66.7 | 10 |

> **§502 Finding A — GATE and SURFACE are different questions, and only GATE
> answers this one.** The earlier single `width` column summed entry points +
> callbacks + ambiguous-external. Arithmetic: JPA 15 = 0+15, 18 = 15+3; JAudio1
> 115 = 14+101, 261 = 115+146. **JPA has ZERO visible entry points** — nothing
> enters it through a symbol the axis can see — so under L1 that is an
> *undercount*, not a narrow gate. "Width 15–18" read as a narrow ABI and it is
> not one; **JPA's gate is UNKNOWN.** The tool no longer emits a conflated
> number, and `gate_width` reports UNKNOWN (never 0) where there is no visible
> entry point AND indirect dispatch is present, because "no gate" and "a gate
> this axis cannot see" are different claims.
>
> **Do not cite total surface as a gate-width comparison in any Stage D
> costing.** That is the one place this presentation could carry a wrong
> conclusion into an expensive decision.

**No subsystem qualifies WHOLESALE.** That is the adopted result.


## 1b. Provenance — what moved since §500, and why (P-protocol applied to itself)

**§502 Finding B, accepted:** the record's own update protocol says *a verdict
that moves without an identified cause is a defect, not a finding* — and §501
carried moved figures with no cause stated. Closed here; this table is now a
required part of every re-adoption.

| Verdict | §500 (audited) | §501 | Δ | Cause |
|---|---|---|---|---|
| V1 JPA | 85.6% · 14–17 | 84.8% · 15–18 | −0.8pp · +1 | HT-23 |
| V2 JAudio1 | 75.1% · 112–130 | 74.9% · 115–261 | −0.2pp · +131 upper | HT-23 |
| others | — | see §1 | small | HT-23 |
| V6 direct-port actors | 43.9–45.5% · 577–677 | 48.2–49.9% · 465–567 | +4.3pp · −112 surface | **E3 re-partition** |
| V8 receiver TUs w/ legs | 38.5–40.5% · 678–795 | 38.4–40.1% · 736–844 | −0.1pp · +58 surface | **E3 re-partition** (mirror side) |

**Cause, stated precisely (update protocol step 1).** Not a measurement change —
a MEMBERSHIP change. The step-10 lineage tags reached 100% of the roster, so the
V6/V8 split now follows the declared `KIT-LINEAGE` tag instead of the path
heuristic. Exactly **5 of 25 TUs (20%) were misfiled**, and in BOTH directions:

| TU | was | now | declared lineage |
|---|---|---|---|
| `d_a_demo00.cpp` | V6 | V8 | `mixed` |
| `d_a_swhit0.cpp` | V6 | V8 | `mixed` |
| `d_a_vrbox.cpp` | V6 | V8 | `mixed` |
| `d_a_vrbox2.cpp` | V6 | V8 | `mixed` |
| `d_door.cpp` | V8 | V6 | `native-port` |

The four that left V6 are TP actors carrying donor hunks — counted as whole-file
donor ports for no reason but living in `src/d/actor/`. `d_door.cpp` is the
converse: a genuine verbatim donor port (§328) filed as a receiver TU because it
sits in `src/d/`. **L8's stated bias was that the partition may be WRONG rather
than imprecise. It was wrong, both ways.** V6 rising 4.3pp is not V6 improving;
it is four leg-carriers leaving a group they never belonged to.


**Single cause: HT-23's indexing fix.** Typedefs, forward declarations and
braced-typedef forms became resolvable, so edges that previously fell to
`unresolved` were reclassified — which moves closure slightly — and more names
became *attributable-but-ambiguous*, which is the whole of JAudio1's
+131 upper-bound jump (`ambiguous_called_externally` = 146). **Not a numbers
defect; a provenance one.** The figures were right; the record failed to say why
they changed.

### V1 — JPA, the marginal case that must not be rounded

**INPUTS.** Closure 84.8% (zero-width range: every edge resolved), remainder only
14.3% receiver-native, TOTAL SURFACE 15–18 (gate: see below), P clear on both falsifiable zeros.

**INCOMPLETE BECAUSE.** Axis D reports **50% decomp over 2 donor objects, 2
stubs** — half its donor surface is NonMatching upstream. Spec §3's
dormant-stub class: stubs do not fail at compile time, they fail when reached.
One TU shows indirect dispatch, so its edge set is undercounted (spec §10).

**GATE (§502).** **UNKNOWN, not narrow.** Zero entry points resolve and one TU uses indirect dispatch, so D-2 is the only route that turns this into a number. Until it lands, JPA's gate is unmeasured — and a wholesale case cannot rest on an unmeasured gate.

**REVISIT TRIGGER.** Either (a) the upstream decomp lands JPA's two NonMatching
objects — the supersession rule then applies and D's risk note is retired; or
(b) a user ruling that 84.8% clears the ~90% guidance given the remainder's
shape. **This is the closest candidate in the table and the tool deliberately did
not round it up.**

### V2 — JAudio1, and the contradiction worth naming

**INPUTS.** Closure 73.7–74.9%, remainder 21.2% receiver-native, TOTAL SURFACE 115–261 (gate: see below).

**INCOMPLETE BECAUSE.** **This subsystem already shipped wholesale.** A census
that calls PIECEWISE what is demonstrably running wholesale is either applying
the wrong threshold to this shape, or is blind to the reason it worked — most
likely that it lives as a parallel stack behind one gate, which spec §4 names as
the narrow-interface pattern and which closure percentage does not capture.
12 of 22 TUs show indirect dispatch. D: 5 stubs (`JASBankMgr`, `JASBNKParser`,
`JASChannelMgr`, `JASDSPChannel`, `JAIBasic`), all NonMatching upstream, under
shipping code.

**GATE (§502) — the trigger is softer than the evidence already in hand.** JAudio1's gate is **14 unambiguous entry points over 22 TUs**. That *is* spec §4's narrow-interface pattern — a parallel stack behind one gate — and it was sitting in the census output while V2 asked for evidence of it. The hypothesis is now partly confirmed by our own measurement, not merely plausible.

**REVISIT TRIGGER.** Any lane demonstrating *why* JAudio1 shipped despite this
score. That reason is a correction to the verdict rule, not to JAudio1 — and it
would likely move V1 too.

### V3–V8 — the class-(c)-dominated group

**INPUTS.** Closure 2.9–45.5%; remainders 26.5–74.1% receiver-native.

**INCOMPLETE BECAUSE.** For V6/V8 the grouping is `[provisional]`: the split
between whole-file donor ports and receiver TUs carrying legs is **by path**,
because nothing checkable separates them until lineage tags land (21% coverage
today). V4's 2.9% rests on 37 edges in a single TU — a percentage that thin is
dominated by boundary code, and spec §2's own guidance is *shape beats count*.

**REVISIT TRIGGER — FIRED 2026-08-06 (E3).** Lineage-tag coverage reached the
roster (step 10 E2: 21% → 100%). It DID re-partition V6/V8 and both verdicts are
re-derived above. Both remain PIECEWISE — the verdict class did not move, only
the membership and the figures. V4's thin-percentage caveat is untouched.

### V9/V10 — VETO-PENDING, and why they are not vetoed

**INPUTS.** Axis P surfaced one platform-declared singleton candidate each, both
`GXTexObj`: `s_texObj` at `d_a_ext_vegetation.cpp:146` and `d_ext_tree.cpp:107`,
type declared under a platform root (`extern/aurora/include/dolphin/gx/GXStruct.h`).

**INCOMPLETE BECAUSE.** Spec §5 makes duplicated singleton runtime state a hard
veto — but *whether a given platform type must be owned singularly* is the ruling
§5 reserves for a human. **Evidence against a veto, recorded by HousingTemp
(§500):** both sites sit under `namespace {` (:86 and :94), so they are
internal-linkage per-TU texture objects, not a shared device singleton, and the
repeated name is no duplicate-symbol hazard. On the JAudio precedent — parallel
players over separate data, sanctioned — these would not veto.

**REVISIT TRIGGER.** A user ruling under spec §5. Until then the verdict is
PENDING in both directions: the tool will not clear it and will not veto it.

### P1 audit — run of 2026-08-06 (E3 re-derivation)

**Auditor: Housing Security. This is a SELF-AUDIT and is weaker than the P1 this
record specifies.** §3 assigns P1 to HousingTemp precisely so the lane that
derived a figure is not the lane that clears it; here Engine re-partitioned and
Housing Security audited, both held by the same instance, on the user's
instruction to work both ends while HousingTemp is absent. The figures below may
be cited WITH that caveat attached. An independent HousingTemp pass remains
worth doing and is not superseded by this one.

**Checks performed.**

1. *Did the tool self-clear or self-veto?* No. Both subsystems stay PIECEWISE;
   only membership and figures moved. No verdict CLASS changed in this run.
2. *Is every movement caused?* Yes — one cause (the re-partition), with the
   5 moved TUs named individually and their declared tags shown. Update-protocol
   step 1 satisfied.
3. *Is the run reproducible?* Yes — `docs/state/ww-census-step8.jsonl`, and the
   partition is re-derivable from the tags in the tree.
4. *Is the new partition itself checkable?* Yes, and this is the point of the
   change: it now rests on a declaration in each TU that `banner_lint.py`
   validates against the donor's own `configure.py`, rather than on a path.

**Finding A — Axis P mislabels a non-duplication (Foundry to fix; no verdict
impact).** V8 reports `DUPLICATED: 1` for `dEvM_HIO_c`
(`include/d/d_event_debug.h`, instantiated once as `l_HIO` in
`src/d/d_event_manager.cpp`) with `receiver_site_count: 0`. The axis defines
DUPLICATED as "instantiated both inside and outside the subsystem" — with zero
outside sites this is not duplication at all. The `else` branch at
`census_axis_p.py:193` fires whenever a receiver-declared type is used ONLY by
the subsystem. No verdict moves (only VETO-CANDIDATE drives VETO-PENDING), but
DUPLICATED findings are surfaced for a spec §5 human ruling, so this could put a
non-question in front of the user. **Not fixed here on purpose:** an auditor who
patches the instrument under audit invalidates the audit. Routed to Foundry.

> **RESOLVED (Foundry). The scale was 7 of 8, not 1 of 8.**
>
> The diagnosis was exact — `census_axis_p.py:193` is reachable **only when
> `theirs` is empty**, because the preceding `elif theirs` consumes every
> non-empty case. So it fired for any receiver-declared type instantiated
> *solely* by the subsystem: one file-scope instance in the whole build, and it
> is ours. Measured across the roster:
>
> | | count | types |
> |---|---|---|
> | real duplications | **1** | `dMsgFlow_c` |
> | mislabelled | **7** | `DSPBuffer`, `dRes_info_c`, `dDlst_blo_c`, `TColor`, `Ja1Track`, `Ja1Parser`, `dEvM_HIO_c` |
>
> **The spec §5 ruling queue drops from 8 items to 1.** The finding said this
> "could put a non-question in front of the user"; it was putting seven. On a
> veto axis that is also how a real one gets skimmed past — the cost is not only
> the wasted rulings but the attention they spend.
>
> **Fix:** new class **`SOLE-INSTANCE`** — kept and reported rather than
> discarded, because a subsystem holding file-scope state of a *receiver* type is
> genuinely relevant to a plugin boundary (that type must exist across it). It is
> simply not duplication, and it does not enter the ruling queue.
>
> **A Foundry claim this corrects.** JAudio1's `DSPBuffer` was described in
> conversation as "the sanctioned parallel-stack case spec §5 names by example."
> Nothing else instantiates `DSPBuffer`; it was never a duplication and so was
> never that example. The claim did not reach a document, but it was wrong when
> made.
>
> **Only VETO-CANDIDATE drives a verdict, so no verdict moved** — the finding's
> own assessment holds. What changed is the ruling queue, which is the part a
> human reads.
>
> **Ruling queue as it now stands (spec §5):**
> * `dMsgFlow_c` — `s_mountFlow` @ `d_ext_npc_mount.cpp` vs `sAlbwMailDeliverFlow`
>   @ `d_a_npc_post.cpp` (1 receiver site) — the one genuine duplication;
> * the two `GXTexObj` VETO-PENDING candidates (V9/V10), unchanged.
>
> **Owed:** HousingTemp to control the SOLE-INSTANCE split before the P-axis
> ruling queue is cited again — the axis output changed shape, and it changed
> because of a Foundry patch to an instrument HousingTemp audits.

**Finding B — an E2 defect the build caught, and a silent one behind it.** E2's
banner prepend displaced the UTF-8 BOM in three TUs (`d_stage.cpp`,
`d_event_manager.cpp`, `ww_jpa_bind.cpp`), putting `ï»¿` in front of
`#include` and breaking compilation. Fixed by restoring the BOM to offset 0.
Behind it sat a quieter one: `banner_lint` and `census_axis_c` anchored their tag
regexes with `^//`, so a leading BOM made a tagged TU read as UNTAGGED. That
would have silently dropped `d_stage.cpp` back to the `[provisional: no tag]`
path rule — reintroducing exactly the L8 bias this step retires, while the
coverage figure still read 100%. Both readers now use `utf-8-sig`. **The build
failure is what surfaced this; the tag readers reported clean throughout.**

**Finding C — read the +4.3pp correctly.** V6's closure rise is NOT V6
improving. Four leg-carrying TUs left a group they never belonged to. Any Stage D
costing that cites "direct-port actors got cheaper" from this delta is reading it
backwards.

**Audit result: figures may be cited, with the self-audit caveat above.**

**P3 — ADOPTED by the user, 2026-08-06**, with the self-audit caveat
explicitly carried. The re-derived V6/V8 rows in §1 are the record's current
state and may be cited. HousingTemp's independent P1 remains open and is not
discharged by this adoption; if it contradicts anything above, this row is
re-derived rather than defended.

## 1c. Step 8 artifact 3 — the leg-migration debt list

Spec §7's third derived artifact: *"every WW branch inside a receiver-owned TU,
each with the hook it would need to become. This is the Engine work item list."*
Tool: `tools/foundry/leg_debt.py`.

**No new marker was introduced.** Engine's `KIT-DONOR-HUNK` (F3, landed in E3)
already marks donor-derived lines inside an otherwise receiver-owned file — that
*is* a leg. A `KIT-LEG` marker would have been a second roster drifting from the
first, which is census spec §1's own failure mode and the reason the banner spec
extended §426 rather than inventing a format. The artifact reads what Engine
declared.

**Measured: 14 legs across 11 TUs, 447 donor-derived lines.**

| TU | legs | lines | donor object |
|---|---|---|---|
| `d_kankyo_ww.cpp` | 1 | 109 | `d/d_kankyo.cpp` |
| `f_op_msg_mng.cpp` | 1 | 72 | `f_op/f_op_msg_mng.cpp` |
| `d_a_vrbox.cpp` | 3 | 69 | `d/actor/d_a_vrbox.cpp` |
| `d_a_vrbox2.cpp` | 2 | 63 | `d/actor/d_a_vrbox2.cpp` |
| `d_stage.cpp` | 1 | 47 | `d/d_stage.cpp` |
| `d_ext_dmesg.cpp` | 1 | 29 | `d/d_mesg.cpp` |
| `d_a_demo00.cpp` | 1 | 27 | `d/d_resorce.cpp` |
| `d_ext_save_flags.cpp` | 1 | 25 | `d/d_save.cpp` |
| `d_a_swhit0.cpp` | 1 | 3 | `d/actor/d_a_swhit0.cpp` |
| `d_particle.cpp` | 1 | 2 | `d/d_grass.cpp` |
| `d_demo.cpp` | 1 | 1 | `d/d_demo.cpp` |

> **This supersedes the 11 / 8 / 281 figure recorded above, and the cause is
> identified rather than assumed** — a verdict that moves without one is a
> defect, so the movement is itemised: **Engine landed `KIT-DONOR-HUNK` markers
> on three further TUs** during E4/E5 (`d_kankyo_ww` +109, `d_ext_dmesg` +29,
> `d_ext_save_flags` +25 = +163), **and one existing leg grew** —
> `f_op_msg_mng` 69 → 72 lines. 281 − 69 + 72 + 163 = **447**, exact. The roster
> is unchanged at 72 TUs and no tool behaviour changed; this is the artifact
> doing its job, since a FLOOR on declared debt is supposed to rise as Engine
> declares more.

**Scope split: 9 file-scope / 5 in-function** — cite the tool's own
`SCOPE SPLIT` line, never a count made alongside it.

> **§508 corrections (HousingTemp). Three discrepancies; the headline —
> 11 legs / 8 TUs / 281 lines — verified exact, and the per-TU table matched to
> the line.**
>
> **D-1 — the split was stated three ways.** Prose said 8/3, the tool printed
> 7/4, ground truth was 6/5 (now 9/5 after the E4/E5 markers above). Two errors stacked: a stale hand-count in this
> record on top of the tool's D-2 defect. The record now quotes the tool, and
> the tool prints the instruction to do so.
>
> **D-2 — `d_demo.cpp:498` was reported FILE SCOPE and is not.** It sits two
> levels inside `dDemo_setDemoData()`, in the `isWwHostStage` branch. Cause: that
> signature WRAPS — the `{` lands on :458 while the name is on :457 — so a test
> requiring both on one line never opened the frame, and everything inside read
> as file scope. It is the only wrapped signature among the 11 sites, which is
> why exactly one leg was hit. By this section's own argument that is the
> consequential one: it routed a control-flow call-out inside a runtime-gated
> branch to a data-table fix.
>
> **The spot-check that missed it, and why.** Foundry named the exact risk — *"a
> forward parse that silently fails would produce exactly this"* — and then
> verified `d_a_vrbox.cpp:117`, a member of the majority FILE-SCOPE class. **A
> frame-dropping parser fails by producing false file scope, so a file-scope
> sample returns the right answer either way and discriminates nothing.** The
> control for this failure mode is a case that must come back IN-FUNCTION —
> the same principle as the census self-tests: prove the instrument can report
> the *other* value. That control now exists and passes
> (`d_demo.cpp:498 -> dDemo_setDemoData() @:457`).
>
> **D-3 — UNMEASURED conflated two states** and was described as empty while
> printing 17. `KIT-DONOR: none` is a **checked claim** — HousingTemp inspected
> those line by line at E2 — whereas `per-hunk` with zero markers is genuinely
> unmeasured. Filing a checked claim as unknown debt inverts the point of having
> looked. Now split: **UNMEASURED: none**, and **CHECKED CLEAN: 22** reported
> separately. (22, not 17: the old predicate matched a narrower set, so the two
> counts are not comparable.)
>
> **Minor, no impact:** the marked row at `d_stage.cpp:628` is `OBJNAME("Ls1", …)`;
> `OBJNAME("Link", …)` sits at :620, above the marker.

**Relation to the verdicts:** none. This artifact changes no verdict — it is the
work-item list that follows from them, and it is the input Stage D step 19 (leg →
hook migration, one at a time, each with its kill switch) consumes.

## 1d. Axis B — the §507 counting defects, and what fixing them found

**INPUTS:** `tools/foundry/ww_census.py::axis_b`, run over the roster union.
**INCOMPLETE BECAUSE:** payload ≠ donor origin; see the limit at the end.
**REVISIT TRIGGER:** any new array-shaped declaration form landing in a roster
TU, or E4's declared set changing.

§507 established that Axis B's raw 8,436 was **not a bound in either
direction** — it over-counted junk and silently dropped real payload. Both are
now fixed at the declaration, which is the only thing §507 permits to decide
after indentation and `const` were each falsified as splits.

| | before | after |
|---|---|---|
| declarations matched | 122 | 123 |
| excluded — no payload declared | 0 | 71 |
| excluded — computed at runtime | 0 | 1 |
| payload arrays | 122 | 51 |
| **payload bytes** | **8,436** | **2,182** |
| payload of unknown size | 29 (as 0 bytes) | 0 |

**The over-count fix.** Scratch buffers are declared `char src[96] = {}` — an
**empty initialiser**. An array with no elements written down carries no
payload, by definition rather than by heuristic; `{}` is a zero-fill request to
the compiler, not data from anywhere. Extended once, to C's `= {0}` idiom
(`static char s_lastPlayerStage[32] = {0}`), under an exact rule — extent > 1
with a single literal-zero leaf. `history[10] = {1000000, 0, …}` keeps payload
status because its leaves are real values.

**The under-count fix.** An unsized `[]` was sized as **0**, and unsized is the
normal shape of a ported donor table. The extent is recoverable *exactly* by
counting initialiser leaves, so reporting 0 was not caution — it was a wrong
number wearing caution's clothes. `a_anm_prm_tbl$4490/4497/4553` now report
176 + 128 + 336 = **640 bytes**, matching §507's figure.

**A third defect, found while fixing those two and disclosed as mine.** Only the
*first* extent was read, so every multi-dimensional array lost its trailing
dimensions — `a_att_dis_TBL[11][2]` scored 11 bytes instead of 22.

**A fourth, which no one had named: the scan never skipped comments.**
`d_a_obj_mshokki.cpp:152` keeps the donor's `bdl_idx[]` inside a comment to
document what the port replaced, and it was being tallied as a live array.
Commented-out code has been inflating this axis since its first landing.

**How that one was found is the point.** It surfaced because a rule I wrote in
the previous pass was **too broad** — flagging any leaf containing `(` as
runtime-computed swept up `dPa_RM(ID_ZI_S_DASHSAND_A)`, a macro over a constant,
and sent three real `d_particle` tables to UNKNOWN. Chasing that false positive
is what exposed the comment bug. **A too-broad rule sends real data to UNKNOWN;
a too-narrow one sends it to zero — and only the first gets looked at.** That is
№31-C's reasoning arriving from the other side, and it is why the residual
`tag` at `d_ext_npc_mount.cpp:10768` is reported as COMPUTED rather than counted
or dropped.

### The control — two independent instruments on one number

E4 declared **1,509 donor bytes over 19 arrays**, each with a hand-counted byte
figure. Axis B derives the same quantity by parse. Every declared array must
appear in the payload set *and* agree on its bytes.

> **19 agree exactly, 0 disagree, 0 missing — all 1,509 declared bytes.**

This is the control the leg-scope layer lacked in §508: a hole in the regex
surfaces as MISSING, bad arithmetic as DISAGREE, and **neither failure mode can
present as silence**. It is locked into `--selftest-b` alongside the existing
zero and `l_toonMat1DL` controls. An independent corroboration falls out of it:
Axis B's display-list class totals **315 bytes**, exactly E4's display-list line.

### One number NOT reconciled — flagged, not papered over

§507 counted **93** scratch buffers; this exclusion finds **71**. The *byte*
magnitudes agree closely (7,484 excluded here against their ~7.8k estimate), and
the discrepancy touches no figure that goes in front of a ruling, since payload
bytes are what the trip-wire is defined on. But a count that moves without an
identified cause is a defect, not a finding — **HousingTemp should confirm which
93 declarations they counted**, since their list was not published and I cannot
reconstruct it from the summary.

**LIMIT — payload is not provenance.** PAYLOAD means elements were written down
at the declaration. It does **not** mean donor origin, which only the §8
provenance banner establishes. 2,182 is a **superset** of the 1,509 declared
donor bytes — it includes receiver-authored tables such as `d_a_swhit0`'s
`l_color`, which §507 verified is TP's own. **2,182 must never be read as the
§5.1 trip-wire figure; 1,509 is that figure.**

## 1e. HousingTemp HT-24 / HT-25 / HT-26 / HT-27 — resolved

**INPUTS:** `census_axis_p.py`, `ww_census.py`, `banner_lint.py`, `leg_debt.py`.
**INCOMPLETE BECAUSE:** the ownership rule below is measured on today's directory
layout; a new subsystem sharing an existing `ext_*` directory would silently
un-exclusive it. **REVISIT TRIGGER:** any roster TU landing in a directory that
another subsystem already occupies.

### HT-25 + HT-26 — fixed by one mechanism, because they are one defect

Both classes **asserted "receiver" without ever testing it**. SOLE-INSTANCE said
*"receiver-declared type"*; DUPLICATED said *"receiver instantiates this type
too"*. Neither resolved who owns the other side. So ownership is now resolved
once and reported.

**Why it was not cosmetic** (HousingTemp's argument, which is the correct one):
the class was kept because *"that type must exist across the plugin boundary"* —
and that is precisely **wrong** for a type JAudio1 declares. JAudio1's own types
travel *with* JAudio1 across the boundary and owe the receiver side nothing.

| type | declared in | before | after |
|---|---|---|---|
| `DSPBuffer` | `include/d/ext_seq/ja1_dsp_boundary.h` | SOLE-INSTANCE | **OWN-STATE** |
| `Ja1Track` | `include/d/ext_seq/…` | SOLE-INSTANCE | **SOLE-INSTANCE-WW** |
| `Ja1Parser` | `include/d/ext_seq/ja1_parser.h` | SOLE-INSTANCE | **SOLE-INSTANCE-WW** |
| `dRes_info_c` | `include/d/d_resorce.h` | SOLE-INSTANCE | unchanged ✔ |
| `dDlst_blo_c` | `include/d/d_drawlist.h` | SOLE-INSTANCE | unchanged ✔ |
| `TColor` | `libs/JSystem/…/TColor.h` | SOLE-INSTANCE | unchanged ✔ |
| `dEvM_HIO_c` | receiver | SOLE-INSTANCE | unchanged ✔ |

`SOLE-INSTANCE-WW` is a separate class rather than a silent merge into OWN-STATE,
because `Ja1Track`/`Ja1Parser` are declared by JAudio1 but **instantiated in a
different subsystem** (`d_ext_seq_space.cpp`, WW host systems). That is a real
cross-subsystem coupling and deleting it from view would hide it.

**The ownership rule, and why it may use directories at all.** Axis C v7
established that directory membership is not subsystem membership — the error
that produced 13,195 entry points for a 4-file subsystem. The include-mirror
claim is therefore gated on **exclusivity**: a source directory's `include/`
mirror belongs to a subsystem only if *every build source in that directory*
belongs to that subsystem. Measured:

    src/d/ext_seq  -> 1 owner (JAudio1)            mirror claimed
    src/d/ext_evt  -> 1 owner (JEvent1)            mirror claimed
    src/d/ext_line -> 1 owner (MDoExt1)            mirror claimed
    src/d          -> 6 owners (incl. non-roster)  REFUSED
    src/d/actor    -> 4 owners (incl. non-roster)  REFUSED

**The refusals are the load-bearing half.** They keep `dRes_info_c` and
`dDlst_blo_c` correctly receiver-side. A wrongly-claimed type would *vanish* from
the ruling queue, and emptying a queue on a tool's say-so is what §506 already
cost once.

**Count discrepancy, stated not smoothed.** HousingTemp tabulated **6**
SOLE-INSTANCE entries and predicted 6 → 3; this run shows **7 → 4**. The
difference is one entry, `dEvM_HIO_c` (`d_event_manager.cpp:l_HIO`, receiver-
declared, genuine), absent from their table. The **substance is identical** —
the same three JAudio1-owned types leave the receiver-relevant list.

### HT-27 — right, latent as filed, and not a one-character fix

Latency **confirmed rather than accepted**: under `r["declared"]` the roster is
72 TUs, identical to the 72 under `== "native-port"`, symmetric difference empty.
The tree declares three lineage values — `native-port` (40), `host-plumbing`
(15), `bridge-owed` (4) — so a `host-plumbing` TU that was neither named nor
evidenced would have been silently unlinted: declared provenance, never checked.

**But the predicate was copy-pasted at five sites across four tools.** Editing
five copies leaves five things that can drift — the exact defect HT-15 removed
when the census and the manifest disagreed about the denominator. It is now
**one function, `ww_census.on_roster()` / `roster_union()`**, called by
`banner_lint` and `leg_debt`; the character is fixed once, in it.

### HT-24 — the count's basis is now stated

"123 declarations" was *arrays with an initialiser*, not *arrays in the roster* —
the pattern requires `= {`. The report now reads **"arrays WITH AN INITIALISER:
123"** alongside **"array declarations, no initialiser: 149"**. Byte-wise this is
inert (an uninitialised array has no payload either way), but HousingTemp's
reason for raising it is the sharp one: 122 → 123 is a near-identical count
across a large population change, the same masking shape as the evidenced-59
coincidence in §493 where one gain hid one loss. *My no-initialiser count is 149
against their ~125 estimate; the two patterns differ and neither figure feeds a
ruling.*

**Verified after all four:** `banner_lint` 72/72 bannered, 0 DISAGREES, exit 0;
`--selftest-b` all three controls PASS (19/19 cross-instrument).

## 1f. RULING — the most-vexing-parse DECL question: DO NOT CHANGE THE REGEX

**INPUTS:** `census_axis_c.DECL`, three discriminator designs measured over
`src/` + `include/`. **INCOMPLETE BECAUSE:** a per-SITE rule was not attempted;
only per-NAME. **REVISIT TRIGGER:** a per-site discriminator that flags zero real
functions on a control set.

HousingTemp's diagnosis is correct: `cXyz sp54(player->current.pos);`
(`d_a_albw_shade_boss_wolf.cpp:1411`, `d_a_albw_shade_watcher.cpp:1438`,
`d_a_ext_plank_span.cpp:1143`) is a most-vexing parse, and `DECL` indexes `sp54`
as a declaration. **The diagnosis is right and the fix is still refused.**

**Three discriminators, each measured:**

| rule | phantoms | verdict |
|---|---|---|
| expression markers anywhere in the match | 1,284 | **contaminated** — `AddCircle`, `AppendChild`, `AddFontDefault`; matched from CALL sites, where `->` is the receiver |
| expression markers inside the ARGUMENTS only | 534 | **still contaminated** — `CreateProcessW`, `DVDCancel`, `CARDSetLoadType`; real functions *called* with member-access arguments |
| type-like PREFIX **and** expression arguments | 225 | **best, ~99% clean** (`acStack_158`, `cStack_130`, `at_vec`, `boomerangPos`) **but still flags `LoadLibraryExW` and `aurora_imgui_add_texture`** |

**The decisive fact: the best rule does not fix the named case.** `sp54` has
**11** DECL matches of which only **6** are phantom-shaped, so any per-NAME rule
("drop names matched only this way") leaves `sp54` indexed. Removing it requires
per-SITE suppression — a structural change to how the index is built, not a
regex tweak.

So the candidate fix pays the full cost and does not buy the thing it was for.

**Cost, at the corrected radius (HousingTemp's §536 correction, accepted):
C, W and P re-derive. Axis D does not** — it has 0 `DECL` references and its
authority is the donor's `configure.py` (§498). I had quoted C/W/D/P, one axis
too wide, in the direction of my own conclusion.

**Error direction is what settles it.** Removing declarations from the index
**shrinks** the measured import surface — the flattering direction, and the same
one-directional mistake this lane has already made twice: §512 understated the
ABI by 2.5×, §520 invented a 772-symbol long pole that was 29. A rule that
silently deletes `LoadLibraryExW` would repeat it a third time, across three
axes, to remove four annotated entries from a fifteen-entry manifest.

**The consequence is already handled correctly** by HousingTemp's annotation:
the unresolvable entries are marked UNKNOWN rather than deleted — 19b's MISSING
category, applied as `UNKNOWN ≠ CLEAN`. Four annotated entries cost far less
than a silent deletion across C/W/P.

**RULING: keep `DECL` as it is; keep annotation.** Revisit only on a per-site
discriminator with a clean control set.

## 2. Incompleteness that applies to EVERY verdict

Stated once, applies to all ten. Each carries its bias direction, because a
limit without a direction cannot be reasoned about.

| # | Limit | Bias |
|---|---|---|
| L1 | **Indirect dispatch** — vtables/method tables are invisible to static analysis (16/16 direct-port actor TUs, 12/22 JAudio1). Spec §10. | Edge sets **undercounted**; width is a **lower bound** |
| L2 | **Member calls** — `obj->foo(` resolves to a method on a type, not a free symbol | Interface surface **undercounted** |
| L3 | **Implicit coupling** — the census sees calls, not shared global state (`g_env_light`, the §113 PAL0 stash) | Coupling **undercounted** |
| L4 | **`unresolved` is an upper bound** on spec §2 class (d), not equal to it — it holds stdlib symbols the index cannot resolve | Argues **toward PIECEWISE** |
| L5 | **Axis P sees only resolvable types**; its UNKNOWN bucket is enums + primitives + std templates. **PROVEN, not hypothetical (§502 Finding C):** HT-23 made `GXTexObj` resolvable and two VETO candidates fired immediately — a limit with a demonstrated instance | Veto surface **understated** |
| L6 | **Axis D perishes on re-base** — its content is "what is stubbed today" | Time-limited, not directional |
| L7 | **Axis D coverage < 100%** for 7 of 10 subsystems; percentages state both denominators | Risk **understated** where coverage is low |
| ~~L8~~ | ~~**Groupings V6/V8 are `[provisional]`**~~ — **RETIRED at E3**: the split is now the declared `KIT-LINEAGE` tag at 100% roster coverage, not a path heuristic. A TU with no tag still falls back to the path rule and is labelled `[provisional: no tag]`, so any regression is visible rather than silent. | ~~Partition may be wrong~~ → residual bias only for untagged TUs (currently none) |

| **L9** | **THE VERDICTS DESCRIBE OUR PORT, NOT THE DONOR.** Every axis measures the coupling of *code as we transcribed it*, which embeds the shimming choices made during transcription. It does **not** measure the donor subsystem's intrinsic separability. | Says nothing either way about the donor; **only about our port** |

> **L9 in full, because it is the one most likely to harden into doctrine.**
> Zero WHOLESALE across ten subsystems **falsified a standing expectation** that
> JAudio1 and JEvent1 would prove to be closed graphs — the census earned its
> keep by killing that assumption rather than confirming it. But the honest
> statement of the result is **"as ported, they are coupled"**, never *"WW
> subsystems are inseparable."* The second claim is about Wind Waker; the census
> never looked at Wind Waker's own build, only at ours.
>
> **Nothing currently turns on the distinction** — relocation is moot under the
> step-20 (A) ruling, since a hook-based plugin does not relocate anything
> (binding spec §6). That is precisely why it is recorded **now**: a limit noted
> while it is inert is a limit; the same limit noticed after someone has cited
> the verdict as evidence about the donor is a retraction.
>
> Raised by another lane's advisory, 2026-08-06, and adopted without amendment.

## 3. Implementation path — reviewable and updatable

**Principle.** No verdict is implemented directly. Each one becomes a work item
only through this path, and every stage is re-runnable, so a trigger firing
re-derives rather than invalidates.

| Stage | What | Owner | Re-runnable? |
|---|---|---|---|
| **P0** | `ww_census.py --full` regenerates every figure from the build | Foundry | Yes — the whole point |
| **P1** | HousingTemp audits the run *before* any figure is cited | HousingTemp | Yes, per run |
| **P2** | This record is updated: verdict, inputs, incompleteness, trigger | Foundry | Yes |
| **P3** | User adopts / re-adopts, or fires a trigger | USER | Yes |
| **P4** | Adopted verdicts become work items with their incompleteness carried into the item | owning lane | — |

**Update protocol (the "updatable provision").** When a trigger fires:
1. Re-run P0 and record what moved **and by how much** — a verdict that moves
   without an identified cause is a defect, not a finding.
2. Re-state the verdict here with the new inputs; keep the superseded row struck
   rather than deleted, so the correction is auditable. (§1.1 of the posture doc
   is the precedent, and the reason.)
3. Re-enter at P1. **A re-derived verdict is not adopted until re-audited.**

**Supersession.** Where upstream decomp completion contradicts an Axis D figure,
**upstream wins** and this record follows it. Our decomp figures are a snapshot
of the donor's current state and were never a claim about the finished work.

## 4. What is DECODABLE — routes to another lane

The census reports UNKNOWN where it cannot resolve. Some of that is not a tool
limit at all: it is undone decoding, and decoding is a different lane's work.
**Foundry builds instruments; it does not decompile.**

| # | Unknown | Decodable? | Route |
|---|---|---|---|
| D-1 | **86 NonMatching donor objects**, incl. JAudio1's five and JPA's two | **Yes — this is literally decomp work** | Decode lane / upstream zeldaret. **Upstream's ruling supersedes ours.** |
| D-2 | **Indirect dispatch (L1)** — vtables and `actor_method_class` tables | **Yes** — method tables are static data and can be decoded to resolve dispatch targets | Decode lane. Would convert the largest structural undercount into measured edges |
| D-3 | **Axis P UNKNOWN: project enums** (`OwnState`, `HandoffKind`, `ReplayPhase`) | **No — Foundry fix.** Enums are scalars by the same reasoning that keeps `fpc_ProcID` out of TYPE_NAMES | Foundry, non-blocking |
| D-4 | **Axis P UNKNOWN: std templates** (`vector<…>`, `unique_ptr<…>`) | **No** — outside the tree; nested templates also mis-parse (`vector<u8>>`) | Foundry, low value; **do not filter by "looks like std"** |
| D-5 | **Axis D unmapped TUs** (7 of 10 subsystems < 100% coverage) | **Partly** — a declared donor-source banner resolves mapping without guessing | Foundry (step 10 banner) |
| D-6 | **`[provisional]` groupings** | **No** — needs declared lineage, not decoding | Foundry (step 10 banner) |

**D-1 and D-2 are the two that would materially move verdicts.** D-2 in
particular attacks L1, the largest stated bias in this record, and it is
tractable: method tables are static data in the binary and in the donor source.

## 5. What this record does NOT authorise

- No Stage D costing. Width is a lower bound (L1–L4); an ABI costed from it is a
  floor.
- No wholesale port. Nothing qualified.
- No veto. V9/V10 are PENDING in both directions.
- No citation of a figure that has not passed P1 for the run it came from.
