# DECODER HANDOFF BRIEF
(2026-08-17 — from the outgoing DECODER instance at context exhaustion.
The charter, DECODER-BRIEFING.md, is UNCHANGED and remains your contract.
Read it first; this file is the campaign-state supplement.)

## 1. Who you are, immediately
You are the DECODER lane of the dusklight estate. Read
`C:\Users\xxxxx\Documents\dusklight\docs\DECODER-BRIEFING.md` in full before
anything else. Operating loop (briefing 3b): a 30-second work-continuation
TIMER + an exit-on-event WATCHER, both as background tasks.
**CRITICAL TRAP**: the timer MUST be created with the Monitor tool
(persistent:true; command: while true; do sleep 30; echo "DECODER-TIMER tick
$(date -u +%H:%M:%SZ) — your queue here"; done). A plain background Bash loop
is SILENT (only notifies on exit) — this bit two instances. The watcher:
`python tools/foundry/decoder_watch.py --exit-on-event` run from
`C:\Users\xxxxx\Documents\dusklight`, re-armed after every delivery. Log every
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
- Repo: `D:\XXXXXXX\WWDP` (fork of zeldaret/tww). Git tip `1673d54c`, tree
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
  `D:\XXXXXXX\tools\objdiff-cli.exe diff -p . -u TU/d/actor/TU -o out.json MANGLED_NAME`
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
- Charter: C:\Users\xxxxx\Documents\dusklight\docs\DECODER-BRIEFING.md
- This brief: C:\Users\xxxxx\Documents\dusklight\docs\DECODER-HANDOFF.md
- Campaign anchors: C:\Users\xxxxx\Documents\dusklight\docs\state\ww-staging\decode-drafts\p2-progress.md and ob1-progress.md
- Monitor log: C:\Users\xxxxx\Documents\dusklight\tools\foundry\MONITOR-REGISTRY.md
- Watcher script: C:\Users\xxxxx\Documents\dusklight\tools\foundry\decoder_watch.py
- Repo: D:\XXXXXXX\WWDP — configure.py; tools/project.py (path patch);
  include/d/dolzel_rel_lit.pch and .h; include/weak_data.h;
  src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include/math.h
  (SQRTF_CONST_LITERALS branch); src/d/actor/d_a_npc_p2.cpp,
  d_a_npc_p2_cut.inc, include/d/actor/d_a_npc_p2.h;
  src/d/actor/d_a_npc_ob1.* skeleton
- Tools: D:\XXXXXXX\tools\objdiff-cli.exe; compilers at
  build/compilers/GC/1.3.2 (1.3.2 is CORRECT — 1.3.2r tested and ruled out)
- Retail RELs (Yaz0-compressed): D:\XXXXXXX\WWDP\orig\GZLE01\files\rels\
- Scratch experiments: C:\Users\xxxxx\AppData\Local\Temp\claude\pooltest\
  (previous session's scratchpad; disposable — everything of value is in
  the anchors and commits)

## 6. First actions on wake
1. Read DECODER-BRIEFING.md, then this file, then both campaign anchors.
2. Arm YOUR timer (Monitor tool!) and watcher; log both in
   MONITOR-REGISTRY.md.
3. cd D:\XXXXXXX\WWDP; git log --oneline -5; git status — confirm tip
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
`D:/XXXXXXX/WWDP`. Filed to Foundry; `watcher_census.py`'s crash-after-output is
still open too.
