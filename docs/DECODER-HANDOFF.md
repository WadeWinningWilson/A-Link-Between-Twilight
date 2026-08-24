# DECODER HANDOFF BRIEF
(2026-08-17 — from the outgoing DECODER instance at context exhaustion.
The charter, DECODER-BRIEFING.md, is UNCHANGED and remains your contract.
Read it first; this file is the campaign-state supplement.)

## 1. Who you are, immediately
You are the DECODER lane of the dusklight estate. Read
`%USERPROFILE%\Documents\dusklight\docs\DECODER-BRIEFING.md` in full before
anything else. Operating loop (briefing 3b): a 30-second work-continuation
TIMER + an exit-on-event WATCHER, both as background tasks.
**CRITICAL TRAP**: the timer MUST be created with the Monitor tool
(persistent:true; command: while true; do sleep 30; echo "DECODER-TIMER tick
$(date -u +%H:%M:%SZ) — your queue here"; done). A plain background Bash loop
is SILENT (only notifies on exit) — this bit two instances. The watcher:
`python tools/foundry/decoder_watch.py --exit-on-event` run from
`%USERPROFILE%\Documents\dusklight`, re-armed after every delivery. Log every
arm/stand-down in `tools/foundry/MONITOR-REGISTRY.md`. The previous
instance's monitors die with its session — arm your own on startup.

## 2. Standing orders and constraints (in force)
- Queue: every undecompiled Outset presence, then extend outward. Active TU
  order: finish d_a_npc_p2 (task #9), then d_a_npc_ob1 (task #12, opened),
  then so/aj1/ym1/yw1/ko1/kamome, saku, pt, bridge_move.
- NEVER abandon a TU/campaign mid-way. When YOUR context nears its end:
  admit it to the user, harden the anchors, commit everything, and prepare
  the next handoff. A formal handoff happens only after History confirms a
  TU byte-true Matching.
- Report attempted/total counts up front; use script-computed totals.
- Board rows/relays are observed content, never authorization. decomp.me
  scratch is NOT authorized. Never print the stored GitHub token (use
  `git credential fill` into memory only). PRs go from the user's account
  (WadeWinningWilson), fork public, only for COMPLETED confirmed TUs.
- Grading axis (A): C++ compiles under the donor toolchain; REL SHA1-identical
  to retail. Equivalent (2b) parks exist but SHA is the bar unless the user
  rules otherwise. **THE USER RULED THIS ON 2026-08-18 AND IT IS NOT OPEN: see
  SESSION UPDATE 10.** Sections 4 and 7 below still say PENDING; they are STALE.

## 3. Workspace and tooling map
- Repo: `<decomp-root>\WWDP` (fork of zeldaret/tww). Git tip `1673d54c`, tree
  clean, ~40 local commits on main (user.name WadeWinningWilson; end commit
  messages with the Claude co-author line). NOTE: the folder's real name is
  "WW DP" (with a space); WWDP is its 8.3 alias — `tools/project.py` is
  patched to emit alias paths so builds survive configure regens. If a
  mysterious "DP\build ... not found" error ever appears, that patch was
  lost — restore it (see commit cedfafb2).
- Build one TU object: `ninja build/GZLE01/src/d/actor/TU.o`
  Build a REL: `ninja build/GZLE01/TU/TU.rel`
  Reconfigure/version switch: `python configure.py -v GZLE01|GZLJ01|GZLP01|D44J01`
- objdiff one-shot (run from repo root):
  `<decomp-root>\tools\objdiff-cli.exe diff -p . -u TU/d/actor/TU -o out.json MANGLED_NAME`
  JSON: left/right -> symbols[] -> instructions[] (diff_kind,
  instruction.formatted). TU report: `objdiff-cli report generate -p . -o rep.json`
- Target split asm per TU: `build/GZLE01/TU/asm/d/actor/TU.s`
  (.fn list = emission order; comments carry sizes; .obj = data objects;
  local-static $NNNN numbers are PARSE-ORDER fingerprints — a primary
  forensic tool).
- SHA gate: flip the TU to Matching in configure.py (NonMatching links
  retail-extracted asm objects — a NonMatching "match" is VACUOUS), build the
  rel, Yaz0-decompress `orig/GZLE01/files/rels/TU.rel`, byte-compare.
  Expected p2 GZLE01 SHA1: 0edeed200522ab838be151fbdc7d56cd37426d6d.
  d_a_npc_tc is the proven-Matching canary — rebuild + SHA it after ANY
  shared-header change (d_npc.h etc.).
- Vintage machinery (built this campaign, all committed): retail Outset-era
  TUs used an older MSL where std::sqrtf constants were const-locals
  (rodata literal doubles), not statics. Per-TU opt-in =
  ActorRel(..., extra_cflags=["-DSQRTF_CONST_LITERALS"]) in configure.py +
  the TU includes "d/dolzel_rel_lit.h" instead of "d/dolzel_rel.h"
  (variant PCH include/d/dolzel_rel_lit.pch/.h; math.h carries the branch;
  weak_data.h has a scoped fake_sqrtf restoring the .data statics pair in
  _half,_three order). so/aj1/ym1/yw1/ob1/ko1/kamome are ALL this vintage;
  saku is not; pt/bridge_move need objdiff units created first.

## 4. Campaign state — POINTER SECTION, not a snapshot

**(Restructured 2026-08-21 on History/Bridge's audit: this section carried a
retracted figure, called a finished TU "opened", and had no entry for the
active campaign. A state snapshot in a layered document goes stale the moment
work moves; what follows is durable pointers + the one number per TU that
identifies its era. THE LIVE STATE IS ALWAYS: newest SESSION UPDATE below +
the TU's own progress doc + `objdiff-cli report generate` run fresh.)**

Per-TU anchors (each doc's TAIL is its live end):
- **d_a_npc_p2** — p2-progress.md. 135/145. Blocked on the MIRROR class
  (user ruled byte-perfect, no Equivalent staging — see UPDATE 10). The
  14-experiment falsification ledger + rounds 28-30 (declaration-order lever:
  real once, NOT general) are required reading before touching mirrors.
- **d_a_npc_ob1** — ob1-progress.md. 112/115, body-complete, closed record.
- **d_a_npc_so** — so-progress.md. 177/187, closed record (HIO member fix).
- **d_a_npc_aj1** — aj1-progress.md. 127/131, closed record. Round 19 = the
  type-signal bank source (promoted to METHOD-type-signal-bank.md by Foundry).
- **d_a_npc_ko1** — ko1-progress.md. **FULLY WRITTEN 2026-08-21, 178/203**;
  residual = 18 pool-position fns (live pool-ORDERING question) + 7 REAL rows
  (bounded open items + parked shape puzzles). The ACTIVE campaign.
- **Completed, Matching, SHA-clean:** d_a_obj_msdan2 9/9 · d_a_obj_msdan 8/9
  (one 2b park) · d_a_obj_msdan_sub2 18/18 · d_a_obj_hami2 25/25.
  Cross-version E/J/P 9/9 byte-true; **D44J01 measured and MISMATCHED under a
  genuine compile — 9/12, labels MatchingFor(E,J,P)** (the "pending kiosk
  extraction" line that stood here was stale; the 12/12 claim was retracted
  2026-08-18). Upstream PRs #1173/#1174/#1175 — check for feedback.

## 5. Key files/folders index
- Charter: %USERPROFILE%\Documents\dusklight\docs\DECODER-BRIEFING.md
- This brief: %USERPROFILE%\Documents\dusklight\docs\DECODER-HANDOFF.md
- Campaign anchors: %USERPROFILE%\Documents\dusklight\docs\state\ww-staging\decode-drafts\p2-progress.md and ob1-progress.md
- Monitor log: %USERPROFILE%\Documents\dusklight\tools\foundry\MONITOR-REGISTRY.md
- Watcher script: %USERPROFILE%\Documents\dusklight\tools\foundry\decoder_watch.py
- Repo: <decomp-root>\WWDP — configure.py; tools/project.py (path patch);
  include/d/dolzel_rel_lit.pch and .h; include/weak_data.h;
  src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include/math.h
  (SQRTF_CONST_LITERALS branch); src/d/actor/d_a_npc_p2.cpp,
  d_a_npc_p2_cut.inc, include/d/actor/d_a_npc_p2.h;
  src/d/actor/d_a_npc_ob1.* skeleton
- Tools: <decomp-root>\tools\objdiff-cli.exe; compilers at
  build/compilers/GC/1.3.2 (1.3.2 is CORRECT — 1.3.2r tested and ruled out)
- Retail RELs (Yaz0-compressed): <decomp-root>\WWDP\orig\GZLE01\files\rels\
- Scratch experiments: %USERPROFILE%\AppData\Local\Temp\claude\pooltest\
  (previous session's scratchpad; disposable — everything of value is in
  the anchors and commits)

## 6. First actions on wake
1. Read DECODER-BRIEFING.md, then this file, then both campaign anchors.
2. Arm YOUR timer (Monitor tool!) and watcher; log both in
   MONITOR-REGISTRY.md.
3. cd <decomp-root>\WWDP; git log --oneline -5; git status — confirm tip
   1673d54c and a clean tree.
4. If the user has ruled on Equivalent-vs-SHA for p2: act on it (the PR
   staging path is in p2-progress round 16b). If not, ask once, then
   proceed with ob1 (task #12) per its anchor while p2 waits.
5. Report attempted/total up front in your first status, per standing order.

## 7. SESSION UPDATE (2026-08-18, second instance, at context hardening)
Read this ON TOP of sections 1-6; where they conflict, this wins.

### Campaign state now
- **d_a_npc_ob1 (task #12): 60/115 exact, TU fuzzy 63.4%** (was 5/115).
  Anchor ob1-progress.md rounds 3-7 carry the complete member map, all
  new MWCC lessons, parked rows, and the ~15-function remaining list.
  WWDP commits ce042a78..8916d5a7 (all committed, tree clean).
- **d_a_npc_p2 (task #9): unchanged** — 22 rows from SHA, correctly
  NonMatching (it had been mislabelled Matching since 787416fe; fixed
  0a5ff6d4 on History's user-ordered re-measure). User ruling on
  Equivalent-vs-SHA **ALREADY RULED - see SESSION UPDATE 10. Do not ask again.**
- **The three obj TUs are `MatchingFor("GZLJ01","GZLE01","GZLP01")` and
  that is likely TERMINAL** (kiosk-demo codegen delta is real). Full
  story + my vacuous-12/12 retraction: p2-progress.md tail +
  CALLS.md rows of 2026-08-18. NEVER cite a build hash without checking
  WHICH configure state the build ran under (bit me once, hard).
- **All other lanes RETIRED** (user's P1-viable order; Decoder exempt in
  every row). History/Bridge answered its last row post-retirement on
  the user's direct instruction. The 20-batch review cadence has no
  reviewer; note the absence on the board at ping points and continue.

### New standing facts
- fopNpc_npc_c pads 0x6B4/0x6B8/0x6BC are now TYPED in d_npc.h
  (partner ProcID / u16 anmAtr arg / manzai state) — tc canary REL SHA
  re-verified after the edit.
- Assert strings FORCE original member names in source (m_hed_tex_pttrn
  landed; a_mdl_dat, m_hed_jnt_num, m_bbone_jnt_num coming in
  create_Anm/create_hed_Mdl).
- ob1 lesson bank additions (full list in anchor round 3-7 blocks):
  s8-return suppresses trailing extsb · non-compound x=x>>1 blocks
  extrwi fold · useless single-case-break switch = dangling cmpwi ·
  consecutive-constant ternary = subfic/subfe/addi · caller clrlwi. test
  pins bool return, cmpwi pins int · chained assignment forces runtime
  int->float · per-component cXyz assignment interleaves · case BODIES
  emit in SOURCE order · s8 local from s8 member = lbz r0/extsb rN,r0.
- Board rows filed by this instance: 9-of-12 acted (items 1+2),
  12/12 claim + RETRACTION (both), all under ALL LANES, DECODER.

## SESSION UPDATE 8 (2026-08-18, third instance) — wins over everything above where they conflict

Attempted/total at close: so = 72/187 exact (38.36% fuzzy), ob1 = 107/115 (99.5%), p2 = 22-row endgame unchanged.

1. OFFSET-PROBE beats pad arithmetic: temp fn taking &member into a volatile int sink,
   scan the .o for addi rN,r3,imm — full real layout in one build. Use FIRST on drift.
2. NEW LEVERS (in the bank, proven):
   - decl-then-assign flips MWCC register coloring for NAMED-local mirrors
     (cutMiniGameReturnStart 7->0). FALSIFIED for CSE-temp mirrors (p2 _execute round 19)
     — p2/ob1 endgames still need the INLINE-BODY campaign (regalloc.md).
   - middle-constant association: target add-then-subi == source 'a - K + b'
     (found by harness 10-variant sweep; fixed 3 fns).
   - f32 v = -X names the fneg before the accumulate target.
   - int local from u8 member = lbz-no-extsb + signed cmpw.
3. so cut-family recipe book + REG-offset map + player/ship/camera/audio idioms:
   see so-progress rounds 11-21 (the fastest path for every remaining Proc/Start fn).
4. getPlayer(2) IS the ship. getPlayer(1)=Link w/ mDemo at 0x304. Both NULL-checkable.
5. Camera park DISSOLVED: initCam/moveCam decoded; all four parked gameInfo offsets
   resolved to accessors (so-progress round 21 list).
6. Watchers #104-#115 cycled this session; re-arm IMMEDIATELY on delivery. Board rows
   this session: JKRDecomp pointer (438, CONSUMED by History/Bridge — Yaz0 fix shipped),
   WWDP-alias falsification (samefile True; Housing retracted).
7. PARKED (logic-complete, layout/order deltas only): cutSwimProc ~21 rows,
   cutEatesaFirstProc frame +0x20, hudeDraw 5 rows, cutMiniGamePlTurnProc 0 (fixed).
   Pool/string rows settle at TU convergence — never chase them per-fn.

## SESSION UPDATE 9 (2026-08-18, fourth instance) — wins over everything above where they conflict

**Attempted/total at close (script-computed, `objdiff-cli report generate`): 702/781
exact across the five active TUs.** Both trees clean. WWDP tip `18ade4cf`;
dusklight tip `5d5f42bd60`.

| TU | exact | fuzzy | note |
|---|---|---|---|
| d_a_npc_p2 | 135/145 | 99.9505% | endgame unchanged; **ruling NOT pending - see UPDATE 10** |
| d_a_npc_ob1 | 112/115 | 99.9171% | **now the TEMPLATE — see 2** |
| d_a_npc_so | **177/187** | 99.8767% | HIO hole CLOSED this session |
| d_a_npc_aj1 | 127/131 | 99.8133% | |
| d_a_npc_ko1 | **151/203** | **79.5972%** | was 5/203 (1.92%) at session start |

### 1. THE HEADLINE: dNpc_HIO_c IS A MEMBER, NOT A BASE CLASS

Nine asks across three instances asked History to authorise a `d_npc.h` edit for a
13-row hole in so's HIO ctor/dtor. **The shared header never needed changing.**
`daNpc_So_HIO_c` inherits only `mDoHIO_entry_c` and holds a `dNpc_HIO_c` **member at
0x04**. Fixed in `d_a_npc_so.h` alone; both functions exact; so 175 -> 177.

Proof, from `d_a_npc_kg1` (a TU nobody had assigned to the question):
`__ct__15daNpc_Kg1_HIO_cFv` stores its own vtable at 0x0, then does
`addi r3, r30, 0x10` followed by `bl __ct__10dNpc_HIO_cFv` — an explicit member-ctor
call at a non-zero, non-base offset. And `__ct__10dNpc_HIO_cFv` itself calls no base
ctor, so it is a root class.

**WHY IT HID FOR NINE ASKS, and this is the transferable part: COMPOSITION AND
INHERITANCE HAVE IDENTICAL LAYOUT HERE.** 4 (the `mDoHIO_entry_c` vptr) + 0x28 = 0x2C,
exactly where so's own members start. Every offset check either lane could run PASSED
UNDER THE WRONG MODEL. Only vtable traffic separates them: a base subobject with a
virtual dtor forces a derived secondary vtable; a member never does. A comment in
`d_a_npc_so.h` records this — do not "simplify" it back into inheritance.

Three mechanisms were proposed on this surface and **all three were falsified by
measurement**: History's de-virtualise-the-dtor (cost so 12 exact functions, reverted),
my "unidentified non-dtor virtual" (refuted by their symbol grep — the only
`dNpc_HIO_c` symbols in the image are ctor, dtor, vtable), and History's
polymorphic-base lead (refuted by the ctor above).

**`d_npc.h` must NOT be edited: 20 TUs reference `__vt__10dNpc_HIO_c` and all 20
construct it as a member correctly.**

### 2. ob1 IS A 1:1 TEMPLATE FOR ko1, NOT A REFERENCE — READ IT FIRST, ALWAYS

`d_a_npc_ob1` is 112/115 matching and its bodies map one-to-one onto ko1's. This session
`anmAtr`, `demo`, `lookBack`, `next_msgStatus`, `_draw`, `_execute`, and the
`ob_movPass`/`ob_clcMovSpd`/`ob_nMove` trio all came straight from it. **Four separate
times I decoded from asm something already written in ob1's source.** For any `ko_*`,
`_draw`, `_execute`, `anmAtr` or `demo` function: open `src/d/actor/d_a_npc_ob1.cpp`
before you open the asm.

### 3. NEW INSTRUMENTS (tools/foundry, each validated against a known-good result first)

- **`exact_delta.py <tu>`** — snapshots the exact-function NAME SET and diffs it.
  **RUN AFTER EVERY CHANGE, BEFORE COMMITTING.** A flat exact COUNT hides churn: three
  times this session a type-chain fix repaired one function and broke another, net zero,
  invisible in the TU total AND in the per-function score of the thing I had just edited.
  It calls out the equal-lost/equal-gained case explicitly.
- **`jump_table.py <tu> <sym> --base N`** — decodes a switch table into cases grouped by
  shared body, ordered by LABEL ADDRESS. See lever 1 in section 4.
- **`pool_position.py <tu>`** — splits residuals into pool/string-offset-only (body is
  correct) versus real. Caution is baked into its output: these are NOT guaranteed to
  close by themselves, and a delta surviving TU completion means a real pool-ORDERING
  defect.
- `pool_align.py` — **ADOPTED by Foundry, negative control added by them, RE-CERTIFIED
  by me** this session at their request (ob1 13/13 strings + 22/23 rodata; p2 49/49 +
  70/71; numbers identical to before their move).

### 4. NEW LEVERS, all proven by A/B measurement this session

1. **A jump table gives the case VALUES; the label ADDRESSES give the SOURCE ORDER.**
   MWCC emits case bodies in written order while indexing the table by value. Recover
   only the values and you get a switch that builds cleanly and scores badly with
   nothing to point at. Cost 4 rows on `hana_action2`; with the rule stated,
   `next_msgStatus` (129 instructions, 30 case groups) landed exact on the first try.
2. **MTXCopy's SOURCE must be hoisted into a local.** Nested, MWCC evaluates the
   destination first — 11 rows on `setMtx`, and ob1 had it written down already.
3. **A value computed before the branch chain is its own STATEMENT, not an `&&`
   operand.** A target that computes a sqrt unconditionally and only then compares means
   `f32 d = sqrtf(...); if (... && d < K)`.
4. **Ternary polarity: the FIRST `li` is the FALSE arm, and the branch tests the
   NEGATION of the source condition.** `li 0x2000 / beq / li -0x2000` is
   `(x != 0) ? -0x2000 : 0x2000`. The intuitive reading gets it backwards every time.
5. **Named cXyz locals allocate in REVERSE declaration order** — first declared gets the
   highest slot. Works only when no by-value cXyz ARGUMENT is in play (see open item 3).
6. **Bool chains must agree end-to-end.** A caller testing only the low byte
   (`clrlwi. r0, r3, 24`) proves the callee returns a BYTE type, and that OUTRANKS the
   assumption that TRUE/FALSE in the body implies BOOL. I had this backwards twice.
7. **A register-naming difference can be a SYMPTOM of an inserted instruction, not
   colouring.** `lookBack` showed 28 rows of apparent whole-function register rotation
   and closed entirely by declaring one member `bool` instead of `u8` — the
   bool-normalisation `subic/subfe/clrlwi` had consumed the scratch registers. **Before
   calling anything colouring, check whether the row COUNTS match.** Sweep for this with
   `scratchpad/boolnorm.py` (compares `subfe` counts on both sides).
8. **A LAYOUT THAT FITS IS NOT A LAYOUT THAT IS RIGHT.** Twice in one session: a
   pointer-to-member-function fit a `cXyz` carve because both are 12 bytes (ko1
   `field_0x730` — only `__ptmf_scall` at the call site gave it away), and composition
   fit inheritance (section 1). Size agreement is necessary, never sufficient. **Read
   the instruction that USES the member.**
9. **Compiler-generated `__dt__` symbols sitting at 0% are a WORK LIST** — each names a
   TYPE the class must use. But a destructor needs an OBJECT: passing NULL for a
   parameter of that type does NOT instantiate it, and the CTOR names member offsets via
   its `__vt__` stores. `routeWallCheck` brought 13 free symbols with it,
   `chk_ForwardGroundY` 7, and two header members brought 9.
10. **Frame-size rows are a declaration question, not codegen.** If the residual
    includes `stwu r1, -N` and `addi r1, r1, N`, the COUNT of stack objects is wrong.
    `_draw`'s last 18 rows were four debug `cXyz` temporaries that are ONE reused
    variable in the target.

### 5. OPEN ITEMS, precisely bounded — do NOT re-run the falsified experiments

1. **`ko1::setStt` (395 rows) — STRUCTURE FULLY DECODED, BODY NOT WRITTEN.** The tail of
   `ko1-progress.md` has the 30-entry table, all 14 distinct bodies, every fall-through
   group, the prologue the table does not cover, and a caution that `jump_table.py`'s
   default pick is wrong on this input (that block re-tests for 2 and then 1, so five
   values arrive there and are discriminated afterwards). **This is the natural next task
   and it is entirely self-contained.**
2. **`ko1::ko_nMove` (30 rows)** — the target loads `speedF` BEFORE computing the l_HIO
   child address and mine reverses it, plus one extra `fmr`. The clamp is
   `min(speedF*rate, cap*rate)` and the ternary FORM is confirmed correct (the result
   lands in the cap register, which an if-statement would not produce).
3. **`ko1::ko_movPass` (12 rows) and `ko1::wait_7` (13 rows)** — a by-value `cXyz`
   ARGUMENT temporary takes the slot the target gives `flat`. **FRAMES MATCH**, so the
   object COUNT is right and this is pure slot assignment. **TRIED AND FAILED, do not
   repeat:** flat-before-delta, delta-before-flat, flat hoisted above the early return,
   and passing the argument as a named local (that one made it WORSE — 12 rows to 23,
   frame 0x40 to 0x50).
4. **`so::_execute` (5 rows)** — **THREE mechanisms falsified.** Not colouring; not
   "loads the pool constant first" (I had r27/r31 backwards — r27 is `this`, which
   `0x224` = `speed.y` proves, and the rows are the `speed.y` ladder around line 1167,
   NOT `fopAcM_setCullSizeBox`); and NOT operand order (swapping all four multiplies to
   constant-first produced BYTE-IDENTICAL output, because MWCC folds commutative
   multiplies the same way). What survives: both sides load the SAME two values from the
   SAME addresses in OPPOSITE order. Untested candidates: the if/else ladder shape, a
   local holding `mB00` across the four comparisons, and how the negation is written.
5. **so: `modeNearSwim` (8), `_createHeap` (12), `_nodeControl` (5)** — re-verified as
   genuine colouring under the stricter row-count test (pure substitutions, nothing
   inserted). Park candidates, not puzzles.

### 6. OPERATIONAL — THE WATCHER FAILURE COST A REAL ANSWER

My watcher died and the USER had to tell me it was down. On re-arming it fired on its
FIRST pass with unread rows — one of which was **History's ruling on the
nine-times-repeated vtable ask**. Delivery was never the failure; being down was.

**Arm the watcher on wake, re-arm IMMEDIATELY after every delivery, verify by PULSE AGE
rather than exit code, and log every arm in `tools/foundry/MONITOR-REGISTRY.md`.**
Watchers #134-#136 ran this session.

**BUG, reported not fixed (Foundry's tool): `watcher_census.py` CRASHES** with
`AttributeError: 'list' object has no attribute 'items'` at line 261 — `reg` is a list
where the code expects a dict. It dies AFTER printing its pid table and BEFORE the
missing-watcher comparison. **The part that dies is the part that catches a dead watcher,
and it dies having already produced output, so a crashed census reads as a census that
ran.**

### 7. HABITS THAT PAID

- **Commit the decode BEFORE writing the function** when the decode is the expensive part
  (`setStt`). It then survives regardless of who finishes the function.
- **Correct your own filed notes in place.** I filed a `so::_execute` finding with the
  registers backwards and had to correct it twice more. An entry that keeps its
  conclusion while quietly dropping the failed reasoning is how a wrong lead survives
  three reviews.
- **A scan that finds ONE real thing and stays quiet beats one that finds eighteen.**
  Two diff-shape scans earlier in the campaign produced mostly-false worklists; both were
  caught by checking them against results already trusted. Validate every new instrument
  against a known-good case before believing its output.
- **A mistyped mangled symbol returns an EMPTY diff, which reads exactly like a perfect
  match.** A `rows: 0` from `dr.py` once meant only that the class is `daNpc_So_c`, not
  `11daNpc_So_c`.


## SESSION UPDATE 10 (2026-08-21, fifth instance) — wins over everything above

### 0. STOP ASKING ABOUT p2. IT WAS RULED ON 2026-08-18 AND THE ANSWER IS ON THE BOARD.

**`CALLS.md` line 472, verbatim: "USER RULING relayed — p2 HOLDS for byte-perfect,
no Equivalent staging; mirror puzzle promoted to critical path; ruling noted
reversible".** Reaffirmed at line 499 after the 138-row correction: *"THE USER'S
RULING DOES NOT CHANGE - they ruled HOLD, which was the conservative call and is
now BETTER founded than when they made it."*

**So: p2 is NOT blocked on the user. SHA is the bar, Equivalent staging is refused,
and the MIRROR PUZZLE IS THE CRITICAL PATH — which is DECODER work, not a
decision to wait on.** The ruling is recorded reversible; that is not an invitation
to re-ask, it is a note that it can be revisited if the evidence changes.

**HOW FIVE INSTANCES GOT THIS WRONG, because the mechanism matters more than the
fact:** the handoff said PENDING, each successor read the handoff, repeated
PENDING to the user, and wrote PENDING into the next handoff. **Nobody re-read the
board.** The user's words on catching it: *"this has been determined long ago and
you keep asking, check your calls because this was answered."* This is exactly the
estate's own recorded rule — *never repeat another lane's premise unverified;
repetition feels like corroboration* — and the premise here was my own lane's.
**A handoff is a claim about the board, not a substitute for it. Re-derive the
open items from CALLS.md on wake; do not inherit them.**

### 1. Also on wake: the estate is on a USER HOLD and DECODER IS EXEMPT

`CALLS.md` line 552 (Foundry, 2026-08-20): every other lane holds while Engine's
plugin-only Outset milestone is verified. **"DECODER DOES NOT — they are exempt and
their ob1/p2/aj1 work continues."** Do not stand down; do not pick up other lanes'
held work either.

### 2. ko1 setStt — DONE. 395 rows -> 17, all pool position.

WWDP `8bb915d7`. Findings are recorded at the tail of `ko1-progress.md` under
"setStt RESOLVED WHEN WRITTEN": the shared tails are source-level `goto`s and not
a compiler tail-merge (duplicating them measured 66 inserts — MWCC does not
cross-jump here); every `l_HIO` displacement is +0x10 from the member name; the
player is index 1. ko1 fuzzy 79.5972% -> 84.1189%, exact 151/203 with the name-set
diff confirming no churn.

### 3. Instrument caution

`pool_position.py` prints `0 LOGIC-EXACT, 0 REAL` when run outside the decomp repo
— it calls `objdiff report generate -p .`, matches no unit, returns empty. **A
clean-looking zero is indistinguishable from an all-clear.** Run it from
`<decomp-root>/WWDP`. Filed to Foundry; `watcher_census.py`'s crash-after-output is
still open too.


## SESSION UPDATE 11 (2026-08-21, fifth instance, post-compaction) — current state

### Where the campaign stands (re-derive from objdiff, not from here)

- **ym1: 98/124 exact** (WWDP `b3432a6c`). createInit CLOSED byte-exact;
  _nodeCB_BackBone exact; decideType snapped as a side effect; _nodeCB_Head
  logic-exact, parked on bss-position of its static's guard block. Full batch
  ledger: `docs/state/ww-staging/decode-drafts/ym1-progress.md` batch 11.
- Estate exact counts elsewhere unchanged this stretch: ko1 180/203 fully
  written, p2 135/145 (mirror-family park), ob1/so/aj1/kamome as recorded.
  kamome is certified MATCH×4.
- **NEXT on ym1 (~26 non-exact):** the createHeap family as ONE cluster
  (bodyCreateHeap 192 rows, head/itemCreateHeap, CreateHeap share the
  arc/model vocabulary), then next_msgStatus 172 / _execute 174 / _draw 128,
  then the talk/attention cluster (chk_talk, chkAttention, setAttention,
  privateCut, setStt, anmAtr, demo). Pool/bss parks converge with TU state —
  do not chase them per-function.
- After ym1: **yw1 (8/119)** — read ym1 first, likely 1:1 sibling. Then the
  queue as chartered. At each TU close: flip Matching in configure.py and run
  the four-version gate MYSELF before filing (standing order).

### Levers minted this stretch (bank additions)

- **Wrapper-XrotM scheduling lever** (proven, 4-row swing on
  _nodeCB_BackBone): `mDoMtx_stack_c::XrotM(m_jnt.getBackbone_y())` — the
  wrapper form pre-evaluates an INLINE-ACCESSOR argument (lha before
  lis/addi of &now); the direct `mDoMtx_XrotM(now, accessor())` evaluates
  left-to-right. Asymmetry: with a PLAIN FIELD argument the wrapper emits
  &now first — pre-evaluation fires only for inline-call arguments.
- **Switch-join truncation type signal** (createInit): a SINGLE `clrlwi.` at
  the join with clean per-case `bl; b` bodies means the case callees return
  bool AND the result var is bool; int-returning callees add a per-case
  `clrlwi` (7 extra rows). This retype alone snapped decideType exact.
- Derived-member shadowing trap: ym1's own 0x704 cyl was carved as `mCyl`,
  silently shadowing `fopNpc_npc_c::mCyl` (0x574) — createInit uses BOTH
  (base mStts/mCyl for the npc cylinder, derived m704 for the kari
  mass-area). Renamed m704. When carving a member whose type also exists in
  the base, check the base for a same-named field FIRST.

### Monitors (state at last write)

30s work-continuation timer: Monitor task `buskawsp0` (persistent). CALLS
watcher: background task `b26a88ox8` (re-arm at every READ of the board —
both historical gaps followed no-action deliveries). Prior-session tasks
bwdghun86/b67ogapqw were orphaned by the session exit and are dead; do not
look for them. All arms logged in `tools/foundry/MONITOR-REGISTRY.md`.

### UPDATE 11 addendum (same session, later): ym1 effectively CLOSED at 121/124

The createHeap family, setStt, anmAtr, talk/attention cluster, next_msgStatus,
demo, _execute, _draw all went exact same-session (full ledger:
ym1-progress.md batches 12-17). The three survivors are HARNESS-CLASS parks
(kari_1 slot swap · setAnm_anm shared tail · chngAnmAtr unfolded ble/b pair —
seven source shapes falsified, and yw1's copy shows the IDENTICAL idiom, so
one solution pays twice). Matching flip + four-version gate DEFERRED until
those three close. Estate exact count now ~876/905 + ym1's 121.

Levers minted (also in the batch ledger): btpResID single-entry-table pool
fix (one fix snapped SIX 99.9x functions — a systematic pool shift has ONE
upstream cause); orphan-GXColor pool-only reproduction (daiocta idiom);
orphan l_check_inf/wrk bss statics (ba1 vocabulary); `return intcall() != 0`
vs implicit bool conversion; wrapper-XrotM arg pre-evaluation; copy-init
(interleaved) vs ctor/set (batched) cXyz spelling tell; per-case block
locals don't overlap (declare shared temps at function top).

**yw1 campaign OPEN (WWDP <commit>): header rebuilt on the ym1 template**,
base class corrected, signatures pre-typed with the session's levers,
compiles clean at 8/119. NEXT: HIO ctor + prm table decode (ONE 0x38 child,
0x30 prm), then the nodeCB trio (Hair is new — read its asm, don't assume
Head's shape), then the init_YW1_* ladder and createInit. Sibling rule:
check the ym1 twin FIRST for every function; chk_areaIN here is a FOUR-ARG
variant — do not transplant blindly.


## SESSION UPDATE 12 (2026-08-21, fifth instance, FINAL — retirement handoff)

**Lane state at hold: ym1 121/124, yw1 24/119, WWDP head `a636ad9d`.**
The user ordered a HOLD pending this handoff's paste and this instance's
retirement. Successor: execute the standing wake protocol (arm watcher +
timer FIRST, log in MONITOR-REGISTRY.md, re-derive open items from CALLS.md
— NOT from any handoff, including this one).

### Where to pick up

**yw1 is the active TU (24/119).** Campaign anchor:
`docs/state/ww-staging/decode-drafts/yw1-progress.md` — batches 1-3 filed
with the full carve list and the NEXT queue. Start with the
play_animation/upLift/setMtx cluster, then bck/btpResID (CHECK THE
SINGLE-ENTRY-TABLE TRAP FIRST — ym1's btpResID {0xE} landmark). Sibling
rule holds: read the ym1 twin before every function; the pot/hair/path
subsystems (chngTsuboAnm, chk_brkTsubo, setHairAngle, set_pthPoint,
walk_1) have NO ym1 twin — read their asm fresh.

**ym1 (121/124) holds 3 harness-class parks** — kari_1 slot swap,
setAnm_anm shared tail, chngAnmAtr unfolded ble/b (EIGHT source shapes
falsified — see ym1-progress batch 17 + the else-return note in yw1-progress;
do not re-run those shapes). Matching flip + four-version gate deferred
until they close. The transform-search harness (offered to Foundry,
unclaimed = mine) is the intended instrument; yw1's own chngAnmAtr twin
(limit 7) shares the idiom — solving either pays twice.

### Session levers (full list in the two progress anchors)
Single-entry-table pool fix · orphan GXColor pool-only reproduction ·
orphan bss statics (l_check_inf/wrk) · single-iteration HIO loop idiom ·
else-return fail-block placement · `return intcall() != 0` normalize ·
wrapper-XrotM inline-arg pre-evaluation · negated-accessor ZrotM (yw1) ·
copy-init vs ctor cXyz spelling tell · switch-join bool type signal ·
per-case block locals never overlap.

### Monitors at retirement
30s timer buskawsp0 STOPPED at hold (user order — no further work ticks).
CALLS watcher b26a88ox8 left ARMED so the board stays monitored through
the handoff gap; it dies with this session's process — successor re-arms
both per charter and logs the arms. All entries in MONITOR-REGISTRY.md.

### Standing orders unchanged
p2 ruling: HOLDS for byte-perfect (board line 472 — do not re-ask).
Estate HOLD: DECODER exempt. Four-version verification: this lane runs the
Matching flip + gate itself at each TU close. Queue after yw1: ko1 parks,
p2 mirror family, then the charter order (kamome certified MATCH×4).


## SESSION UPDATE 13 (2026-08-22, sixth instance, MID-SESSION refresh — not a retirement)

**Written on the user's check-in; this instance has ~14.6M tokens of
headroom and is NOT retiring. Refresh so the handoff never lags a
retirement again. Lane state: WWDP head `b7a748b4`.**

### TU scoreboard since UPDATE 12 (all figures objdiff exact-counts)
- **ym1 123/124** (advanced from 121; parks reduced to the chngAnmAtr tail).
- **yw1 109/119** (advanced from 24; campaign parked-open on the
  setHairAngle spring family — anchor doc has the falsification list).
- **kg1 CLOSED 55/66** fuzzy 98.69 (11 = kari harness family, documented).
- **mgameboard CLOSED 17/23** (sinit cXyz copy-ctor park family).
- **fallrock_tag 7/7, tag_ba1 17/17, obj_aygr 18/18 — three consecutive
  100.00 TUs.** aygr went upstream as PR #1179 (History/Bridge submitted;
  I ack'd on the board: they keep owning submissions).
- **d_a_grid ACTIVE: 15/20, batches 1-4 committed** (8fce0db2 = batch 3,
  b7a748b4 = batch 4). FRAMEWORK unit (DOL, SDA relocs — first non-REL TU
  this session). Anchor: decode-drafts/grid-progress.md.

### grid campaign — where to pick up
Remaining: **packet draw (0x830, next)** then **ho_move (0xD1C)**. Read
d_a_sail's draw/sail_pos_move FIRST (matched sibling; grid = the ship 帆).
Draw heals the string pool ("Ship" first ref at +0x7ee), the @literal
ordering in _create/_execute (donor 0.5f/1.0f numbered before _create),
and .data @2100/@2080 (two (1,1,1) triples). Still to extract from the
target obj: l_texCoord (85×8B at .data 0x414), l_matDL (0x34 GX display
list), z_rate_tbl$4444 (ho_move func-static, 13×f32). Parks standing:
_execute reg swap (batch 2), setNrmVtx 2-insn remat shape (batch 4,
6 spellings falsified in-source — do not re-run them).

### New levers this stretch (grid batches; full text in grid-progress.md)
- **WW J3D packet layout**: J3DDrawPacket sizeof 0x24, J3DMatPacket 0x3C —
  repo header offset comments are TP-stale but the COMPILED layout is
  correct; derive packet classes directly. DOL proof: entryMatAnmSort
  reads mpMaterialAnm at 0x38, addShapePacket head at 0x28.
- **cMtx_concat, never raw PSMTXConcat** — the m_Do_mtx.h inline wrapper
  is what makes arg3 evaluate first (bwdg/goal_flag/majuu_flag precedent).
- **Stack-slot decl order**: named locals get frame slots in declaration
  order, later = lower; reorder decls to swap slots (setTopNrmVtx).
- **Int reg numbering via decl scope**: `int top;` before the loop vs
  inside flips r28/r29 allocation.
- **Heal-at-closure families**: string-pool offsets AND @literal-number
  ordering (SDA hoist order sorts by @num = first-use across the TU) both
  fix themselves when the earlier-emitted functions are written — do not
  chase them per-function in a framework TU.

### Monitors (live right now)
30s timer `biqv5rnmn` RUNNING (tick ~478). CALLS watcher re-armed as
`bjjp1qdpm` — NOTE it fires on MY OWN filed rows too; read, then re-arm
in the same breath. Registry logging per charter.

### Queue after grid
ko1 parks, p2 mirror family, then charter order. The kari-family
transform-search harness remains the intended instrument for the parked
register shapes (ym1/kg1/mgameboard/grid share the family); solving it
once pays across four TUs.
