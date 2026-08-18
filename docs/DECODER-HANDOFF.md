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
  rules otherwise (see section 4: a decision is PENDING with the user).

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

## 4. Campaign state
### d_a_npc_p2 (task #9) — 22 instruction rows from SHA
Anchor: docs/state/ww-staging/decode-drafts/p2-progress.md (in dusklight) —
its CONTINUATION PREAMBLE + rounds 1-17 are REQUIRED READING (the entire MWCC
lesson bank: pool = parse order with rodata-absolute labels; counters as
source archaeology; -sym on file-keyed text sections => cut family in a
_cut.inc; demanded-inline emission rules; member re-spell vs named-local
load-count control; the sqrt idiom map; useless-case-3 switches; compound
*= and -= operand-order effects; and more).
- DONE: string pool, rodata, .data byte-exact; ALL 145 function sizes exact;
  .text emission order identical; vintage PCH landed; cross-version verified
  (GZLJ01 shows only the same residue). 142/145 functions byte-perfect
  including relocs; cutRopeTalkStart (the 0x550 giant) is 100%.
- REMAINING: three functions with mirror-order register-pair allocations —
  setAnm (4 rows, extsb r0/r3), _execute (5 rows, r28/r29 CSE temp),
  cutRopeTalkProc (13 rows, r27/r28 + f30/f31). A 14-experiment
  falsification ledger is in the anchor — do NOT redo those (spelling
  sweeps, decl orders, parity probes, stripped-static probe, GC/1.3.2r
  compiler: all dead). Live paths: (1) allocator pattern-learning from ob1;
  (2) an automated transform-search harness driven by the row-count oracle;
  (3) USER DECISION PENDING: accept 2b-Equivalent on these 3 fns for PR
  staging vs hold for SHA. Ask once if unanswered.
- p2 is held NonMatching in configure.py until the gate passes.

### d_a_npc_ob1 (task #12) — opened, structure mapped
Anchor: docs/state/ww-staging/decode-drafts/ob1-progress.md (kickoff
checklist + rounds 1-2). 115 fns, text 0x3DC0, literal-vintage, ONE weak
text section (the fopNpc next_msgStatus/getMsg/anmAtr trio) — NO cut-family
.inc needed. Vintage wiring already committed (configure + dolzel_rel_lit
include). Round 2 holds the full ctor member map (offsets 0x29b..0x770).
Skeleton: src/d/actor/d_a_npc_ob1.cpp (422 lines, 69 stubs, 5/115 exact)
plus a bare header. NEXT STEP: populate the header member layout from the
ctor map, then decode smallest-fns-first (bitCount, clrSpd, setStt, resID
helpers) with per-fn objdiff. SECONDARY MISSION: at every site where a pair
of callee-saved registers is assigned, record the target direction vs
yours — that data may crack the p2 mirror puzzle.

### Completed earlier this campaign (Matching, SHA-clean)
d_a_obj_msdan2 (9/9), d_a_obj_msdan (8/9, one 2b park), d_a_obj_msdan_sub2
(18/18), d_a_obj_hami2 (25/25). Cross-version 9/9 byte-true E/J/P; D44J01
pending kiosk extraction. Upstream PRs #1173/#1174/#1175 (zeldaret/tww) may
have CI/maintainer feedback — check them; retroactive-correction rule armed.

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
  Equivalent-vs-SHA STILL PENDING (asked in chat 2026-08-18).
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
