# d_a_npc_ob1 decode campaign (task #12)

**Read `p2-progress.md` FIRST** — its CONTINUATION PREAMBLE carries the DECODER
charter, monitor discipline, toolchain recipes, and the complete MWCC lesson
bank (rounds 1-17). ob1 is queue-next AND the pattern-learning vehicle for
p2's 22-row mirror-allocation endgame (task #9): at every site where two
callee-saved registers get assigned to a value pair, RECORD target direction.

## State
- Skeleton: src/d/actor/d_a_npc_ob1.cpp (422 lines, 69 stubs), header 96
  lines. 5/115 exact, fuzzy 2.70%. Target: 115 fns, text 0x3DC0,
  literal-vintage (no _half statics).
- Target split asm: build/GZLE01/d_a_npc_ob1/asm/d/actor/d_a_npc_ob1.s

## Kickoff checklist (p2 playbook order)
1. configure.py: ActorRel entry gets extra_cflags=["-DSQRTF_CONST_LITERALS"];
   cpp includes "d/dolzel_rel_lit.h" instead of dolzel_rel.h.
2. Read target .fn list — note any post-__sinit text sections (cut family ->
   own _cut.inc positioned per local-static $counters).
3. Recover member layout + fn bodies smallest-first; objdiff one-shot per fn.
   Use p1/p2 sibling templates but VERIFY every constant against target
   (p1-template traps cost p2 two rounds: 1.75f-vs-10.0f, ChkGroundFind).
4. Class defs (HIO etc.) in the CPP; local statics const-ness per target
   section; sqrt spellings: .abs()/std::sqrtf everywhere unless target shows
   the NaN chain (plain sqrt) — vintage PCH makes constants literal.
5. Sizes-exact sweep -> .text order tool -> REL gate (Yaz0 diff vs
   orig/GZLE01/files/rels/d_a_npc_ob1.rel).

## Rounds

### Round 1 (15:00Z) — structure mapped
- 115 fns; ONE extra text section holding fns 112-114 (next_msgStatus/getMsg/
  anmAtr of fopNpc_npc_c — weak base-class copies post-__sinit). NO cut
  family — no _cut.inc needed. Region A order = fns 0-107 (definition order
  target): HIO ctor, mDoHIO dtor, searchActor_Kb, nodeCallBack_Ob1,
  nodeOb1Control, cXyz dtor(demanded), init_OB1_{0,1,2}, createInit, setMtx,
  anm/btp resID helpers, setBtp, iniTexPttrnAnm, plyTexPttrnAnm, setAnm_tex/
  _anm(anm_prm_c*)/_NUM/setAnm, chg/control_anmTag, chg/control_anmAtr,
  setAnm_ATR, anmAtr, eventOrder, checkOrder, chk_talk, chk_partsNotMove,
  lookBack, next_msgStatus, getMsg_OB1_{0,1,2}, getMsg, chkAttention,
  setAttention(b), searchByID(Ui), partner_srch, bitCount(Uc), set_pigCnt,
  ob_setPthPos, get_attPos, ob_movPass, ob_clcMovSpd, ob_nMove, charDecide,
  event_actionInit, event_action, privateCut, endEvent, isEventEntry,
  event_proc, set_action(PTMF!), clrSpd, setStt(Sc), wait_1..3, walk_1,
  talk_1, manzai, wait_action1/2, demo, shadowDraw, _draw, _execute, _delete,
  CheckCreateHeap, _create, __ct daNpc_Ob1_c(!an explicit ctor), [demanded
  dtor wave], create_Anm, create_hed_Mdl, CreateHeap, static wrappers,
  [cCcD weak wave], HIO dtor, cLib_calcTimer<s>, cLib_getRndValue<i>, __sinit,
  thunks, then the weak fopNpc trio.
- Notable: set_action takes a PTMF (mangled M11daNpc_Ob1_cFPCvPvPv_iPv);
  pig-counting helpers (bitCount/set_pigCnt) — this is Abe/pig-chase logic;
  path movement (ob_setPthPos/ob_movPass = dPath).
- NEXT: member layout from target asm offsets (start with __ct daNpc_Ob1_c
  fn 72 — explicit ctor gives field init map!), then smallest-fn-first decode.


### Round 2 (15:10Z) — ctor member map (from __ct__11daNpc_Ob1_cFv, 89 instr)
- daNpc_Ob1_c : fopNpc_npc_c (vptr at 0x6c0; fopAc ctor + fopNpc vt store).
- Zeroed u8: 0x29b, 0x29c. Zeroed ptrs: 0x2cc, 0x2d0, 0x32c.
- 0x334 dBgS_ObjAcch (to 0x4f8), 0x4f8 dBgS_AcchCir (to 0x550),
  0x550 dCcD_Stts (+GStts 0x554), 0x574 dCcD_Cyl block (GObjInf@0x574,
  ShapeAttr vt@+0x114, Aab@+0x110, CylAttr/Cyl vts; cyl obj base 0x574).
- 0x6ac s32 = -1; 0x6b0 = 0; 0x6c0 vptr(daNpc_Ob1_c final);
  0x6dc mDoExt_btpAnm (0x6e0, 0x6e8 zeroed); 0x710, 0x714, 0x770 ptrs = 0.
- NEXT: cross-check against the skeleton header (96 lines — likely already
  partial); fill gaps; then decode smallest fns first (bitCount, clrSpd,
  setStt, resID helpers) with per-fn objdiff.

### Round 3 (02:07Z, successor instance) — 23/115 exact, TU fuzzy 7.00% (commit ce042a78)
- Header REWRITTEN: base is fopNpc_npc_c (NOT bare fopAc). Derived layout:
  mBtpAnm@0x6DC (mDoExt_btpAnm), mPathRun@0x700 (dNpc_PathRun_c, 8 bytes),
  mEventCut@0x708 (SECOND dNpc_EventCut_c, shadows base — km1 precedent at
  0x700; its ctor zeroes +8/+C/+68 = the 0x710/714/770 stores), tail members
  0x774..0x80C from setStt/small-fn asm. Round-2's "0x550 dCcD_Stts" was the
  base's OWN subobject (km1 ctor stores vt at 0x550 too — header comment
  offsets fine; vt sits 0x18 into dCcD_Stts).
- HIO class defined IN CPP (p2 r14 lesson): mDoHIO_entry_c base, s8 0x4=-1,
  int 0x8=-1, 0x30-byte prm blob @0xC memcpy'd from static const struct
  {s16[10] jnt limits, 170.0f, 6, u8 4, 0.5f, 3.0f, 0.1f, 40.0f} — ctor
  100% FIRST COMPILE.
- 100% now: HIO ctor+dtor, mDoHIO_entry dtor, anmNum_toResID {3,0,1,4,2,8,
  9,10}, btpNum_toResID {7,7}, iniTexPttrnAnm, clrSpd (0x7B8/0x7BC/speedF=0,
  gravity=-4.5f), bitCount, setAnm_tex, chg/control_anmTag, partner_srch,
  event_action, endEvent (onEventFlag(8); 0x800=0xff), getMsg_OB1_2,
  searchByID (fopAcM_SearchByID out-param), isEventEntry
  (evmng_getMyStaffId(mEventCut.getActorName())), CheckCreateHeap + 5 glue.
- NEW MWCC LESSONS: (a) s8 RETURN TYPE suppresses the trailing extsb an
  int-return adds after an s8 local; (b) NON-COMPOUND `x = x >> 1` on u8
  emits clrlwi+srawi where compound `>>=` folds to extrwi; (c) the dangling
  lbz+cmpwi+blr shape == single-case-break SWITCH (km1 source proof;
  3rd useless-switch sighting).
- PAIR-BIRTH LOG (p2 secondary mission): setStt asm births r31=this,
  r29=param(Sc), r30=old 0x805 — savegpr_29 ASCENDING assignment order
  (this→r31 first). No 2-value CSE pair sites yet in decoded set.
- NEXT: charDecide(0x50), nodeCallBack_Ob1(0x4C), chk_partsNotMove(0x40),
  setAttention(0x58), event_actionInit(0x58), _delete(0x5C), setAnm_ATR
  (0x68), setAnm_NUM(0x6C), getMsg_OB1_1(0x6C), getMsg(0x70), then setStt
  (fully analyzed above) once get_attPos/set_pigCnt land.

### Rounds 4-5 (02:29Z) — 41/115 exact, TU fuzzy 24.14% (commits 13da80a5, 61614115, 602936aa)
- 100% now also: setAnm_NUM ($4448 8-row tbl), getMsg (0/1/2 switch),
  getMsg_OB1_1, set_pigCnt, searchActor_Kb (kb_class m405 bit-or, cap 20),
  get_attPos (mPathRun.nextPath(current.roomNo), maxPoint when idx 0),
  chk_talk (bool!), manzai, wait_2, wait_3, nodeCallBack_Ob1, setAnm_ATR,
  setAttention, charDecide, chk_partsNotMove, event_actionInit* /_delete*
  (*=1 pool-settling row each, effectively done).
- NEW MWCC LESSONS this stretch:
  * ternary with CONSECUTIVE constants `c ? 0xAAB : 0xAAC` = branchless
    subfic/subfe/addi (0xAAC + -(cond)); the `0xAAC - (x!=0)` spelling
    gives a DIFFERENT 0/1-materialize shape.
  * a CALLER testing a call result with clrlwi. pins callee return = bool;
    cmpwi = int/BOOL (wait_2/3 -> chk_talk bool).
  * u32 == -1 check compiles to addis+cmplwi 0xffff (unsigned form);
    int would cmpwi -1 directly (chg_anmAtr field_0x7CC u32).
  * .bss has an ANCHOR object like rodata's m_heapsize: "@3569" (0xC anon,
    early counter — likely a header-inline default-arg temp); all TU .bss
    symbols addressed relative to it in target asm. Settles with layout.
- d_npc.h RETYPE (shared header, offsets unchanged, tc canary REL SHA
  re-verified c5f975667b MATCH): fopNpc 0x6B4 fpc_ProcID field_0x6b4
  (manzai partner ID), 0x6B8 u16 field_0x6b8 (arg to virtual anmAtr),
  0x6BC u8 field_0x6bc (manzai state 2/3).
- manzai idiom: dComIfG_MesgCamInfo_c* caminfo = dComIfGp_getMesgCameraInfo();
  `this != caminfo->mActor[caminfo->mBasicID - 1]` (bm1 anmAtr attests);
  gameInfo+0x5C20 = MesgCamInfo, NOT attention.
- PARKED §2b (2 rows each, semantically proven): chg_anmAtr tail
  (target bne-to-body + b-to-end, mine beq-to-end; km1 && spelling adopted;
  !(==) and early-return falsified), control_anmAtr (2-case switch tree
  root: target roots at 3, mine at 6; case-swap made it worse (6 rows),
  empty case 0 matched root but added 2 rows — falsified both).
- Ob1 = ROSE (Abe's wife, pig lady). Msg ids 0xAA8/0xAAB/0xAAC; eventBit
  0x2C20; eventReg 0xB6FF = pig-collection bitmask (bitCount'd into
  0x7E7 total/0x7E8 present/0x7E9 alive-not-collected).
- BOARD NOTE: History/Bridge, Foundry, Integrator (+Housing Sec/Temp fold),
  all lanes RETIRED per user's P1-viable order — DECODER explicitly exempt
  in every row. The briefing's 20-batch review cadence pinged HISTORY;
  reviewer is gone. At next TU-complete ping, note absence on the board and
  continue (3b.2 relocate discipline).
- NEXT: wait_1, walk_1, talk_1, demo, checkOrder, eventOrder, setStt
  (analyzed in full — write after), setBtp, plyTexPttrnAnm, setAnm_anm,
  setAnm ($4455 tbl), anmAtr, lookBack, next_msgStatus, then the big tail
  (createInit/_create/_createHeap/create_Anm/create_hed_Mdl, _draw,
  _execute, init_OB1_*, nodeOb1Control, ob_* movement, event_proc,
  privateCut, set_action PTMF, getMsg_OB1_0, chkAttention, lookBack,
  setMtx, shadowDraw, searchActor partner).

### Round 6 (02:40Z) — 47/115 exact, TU fuzzy ~35% (commits 9582338c, b9e381c0, +setStt)
- 100%: wait_1, walk_1, talk_1, checkOrder, eventOrder, setStt (7-case
  jumptable, case 4 falls into 1/5, case 6 early-returns past setAnm).
  demo() at 2 pool-settling rows ("Ob" offset) — effectively done.
- demo() = dDemo binding: dComIfGp_demo_get()->getObject()->getActor(
  demoActorID); getP_BtpData("Ob"); mBtpAnm.init(mdl, btp, TRUE, 2, 1.0f,
  0, -1, true, 0); dDemo_setDemoData(this, 0x6a, mpMorf, "Ob", 0,0,0,0);
  returns field_0x7FB u8.
- NEW LESSONS: s8 local from s8 member = lbz r0/extsb rN,r0 vs int local
  lbz rN/extsb rN,rN (eventOrder); re-spelled inline accessor under CSE
  re-extends per use (extsh) where a named s16 local folds it (demo);
  eventOrder mixed local/member: `s8 action` local for the tree, member
  re-spell `(int)field==1` inside.
- checkOrder: eventInfo.checkCommandDemoAccrpt/Talk order; demo-id array
  = s16 field_0x7D0[1] indexed by s16 field_0x7D2 (only [0] reachable;
  eventOrder stores idx = action-3). orderOtherEventId(this, id, 0xff,
  0xffff, 0, 1).
- Header adds: 0x6D4 J3DModel*, 0x6D8 J3DAnmTexPattern*, 0x6F0 u8 frame,
  0x7D0 s16[1]/0x7D2 s16/0x7D4 s16, 0x7E2 u16 (msg status), 0x7EC u8,
  0x7ED u8, 0x7EA s8, 0x804 s8; l_check_flg file static (pig-bit accum,
  talk_1). Msg: talk_1 case 0x13 sets eventBits 0x304/0x308/0x2c20/0x302
  by msg id {0xa8e, 0xaa7, 0xaa9-0xaac}.
- REMAINING (68 fns): wait_action1/2, set_action (PTMF), event_proc,
  privateCut, chkAttention, lookBack, next_msgStatus, setBtp,
  plyTexPttrnAnm, setAnm_anm, setAnm ($4455 tbl), anmAtr, getMsg_OB1_0,
  init_OB1_0/1/2, createInit, _create, CreateHeap, create_Anm,
  create_hed_Mdl, _draw, _execute, setMtx, shadowDraw, nodeOb1Control,
  ob_setPthPos/movPass/clcMovSpd/nMove, isDelete/weak dtors (auto).

### Round 7 (03:00Z) — 60/115 exact, TU fuzzy 63.4% (commits through 8916d5a7)
- 100% added: setStt, wait_action1/2 (case bodies follow SOURCE order —
  original wrote 4,5,3,2), set_action (km1 verbatim, 0/0 stores),
  event_proc, privateCut*, init_OB1_0/1/2 (ptmf constants @4249/@4263/
  @4276 = wait_action1/2/2), ob_setPthPos, ob_clcMovSpd (per-component
  cXyz assignment interleaves; const-ref binding kills operator- temp
  copy), ob_nMove, setBtp* (m_hed_tex_pttrn NAME FORCED by assert string;
  u16 resID local), plyTexPttrnAnm, setAnm_anm* (CHAINED ASSIGNMENT
  a=b=c=0 forces runtime int->float xoris/0x4330 idiom), next_msgStatus
  (34-entry jumptable 0xa8d..0xaae), getMsg_OB1_0 (pig-count dialog tree;
  direct member compares CSE extension across > and switch).
  (* = 1-2 pool-settling rows only.)
- PARKED §2b (polarity family, ~2 rows each): chg_anmAtr tail,
  control_anmAtr tree-root, privateCut beq+b site.
- REMAINING REAL FNS (~15): nodeOb1Control, createInit, setMtx, setAnm
  ($4455 tbl w/ sentinels), anmAtr, lookBack, chkAttention, shadowDraw,
  _draw, _execute, _create (materializes implicit __ct + weak dtor wave),
  create_Anm (0x214, assert names a_mdl_dat/m_hed_jnt_num/m_bbone_jnt_num),
  create_hed_Mdl, CreateHeap. Weak dtor/cCcD wave + fopNpc trio
  materialize when _create/_draw land.
- FIELD TYPES pinned this round: 7E7/7E8/7E9 s8 (pig counts), 7EA s8,
  7ED u8, 803 s8, 804 s8, 80A s8, 7FE u8 ((s8) cast at -1 check),
  7B4 f32, 7F0/7F1 u8, 6F2 s16, 6F4 ActionFunc, 7D0 s16[1]/7D2/7D4 s16.
- 🔴 12/12 RETRACTION logged (see p2-progress.md): D44J01 genuinely
  mismatches the three obj TUs; labels MatchingFor(E,J,P) — terminal
  unless the demo delta is decoded. Vacuous-hash lesson re-banked:
  CHECK WHICH CONFIG A BUILD RAN UNDER BEFORE CITING ITS HASH.

### Round 8 (03:09Z) — 64/115 exact, FINAL ROUND OF THIS CONTEXT
- 100% added: setAnm, chkAttention (km1-VERBATIM: bool return + named
  dAttention_c& local + if/else — direct-call spelling masks == with
  extrwi; u8/BOOL returns also mask), setMtx (setBaseTRMtx METHOD for
  the stack->model copy + drawHead anmMtx-hoist for the head-model copy;
  m_hed_jnt_num@0x6CC s8 assert-attested name, field_0x78C s16 yaw,
  tevStr.mRoomNo/mEnvrIdxOverride p2 idiom), shadowDraw (settling-only:
  150/800/40 constants, GetGroundH, m_gnd, addRealShadow to 6D4).
- Rodata map fully computed (offset->object table in this round's
  session): sqrt doubles 0.5/3.0 @0x50/0x58 (vintage ✓), 150/800/40 @
  0x64-0x6C, 80/160 @0x7C/0x80 (unclaimed — lookBack candidates).
- REMAINING REAL FNS (10): lookBack (0x1A0, partially dumped — starts by
  saving jnt angles into 7C4/7C6/7C8 then eyePos-y copy, 807 switch,
  @4241 zeros into sp2C-34; 80/160 floats likely its dist gates),
  anmAtr (0x108, km1 template + MesgAnime idiom), _draw (0xF4),
  _execute (0x1E4), nodeOb1Control (0x150), createInit (0x208),
  _create (0x120, materializes implicit ctor + weak wave), CreateHeap
  (0x14C), create_Anm (0x214, asserts name a_mdl_dat/m_hed_jnt_num/
  m_bbone_jnt_num — NAMES FORCED), create_hed_Mdl (0xCC).
- l_evn_tbl file-scope static EXISTS in rodata (0x150, 4 bytes) — an
  event-name ptr table, likely used by createInit/eventOrder region;
  find its consumer before writing createInit.

### Round 9 (03:24Z) — 🎉 TU FULLY DECODED: fuzzy 99.4678%, 106/115 exact
- ALL functions written. Landed this round: setAnm, chkAttention (km1
  verbatim), setMtx (setBaseTRMtx + anmMtx-hoist), shadowDraw, lookBack
  (bm1 verbatim; bool field_0x7FA pins ssb param), anmAtr, _draw (first
  compile), _execute (FIRST COMPILE — core loop), nodeOb1Control,
  create_Anm (vbase-flag ctor, "head"/"backbone" getIndex, assert-forced
  m_hed_jnt_num/m_bbone_jnt_num), create_hed_Mdl (a_mdl_dat, tbl {6}),
  CreateHeap (a_hed_tex_tbl rodata-templated local, joint-callback loop,
  SetWall(30,80), 8-arg Set), createInit (FIRST COMPILE — l_evn_tbl
  "Get_Rupee" event, path from (param>>16)&0xFF, cc 0xff/0xd9,
  setActorInfo2("Ob1")), _create (switch-on-state, size 0x272E0,
  cullSizeBox ±60/-20/170).
- NEW LESSONS: caller clrlwi. pins bool returns EVERYWHERE (charDecide,
  createInit, init_OB1_*, setBtp, iniTexPttrnAnm, chk_talk, event_action
  all bool); switch(state){case COMPLEATE:} = beq+b; l_evn_tbl file-static
  initializer strings pool FIRST (pre-function parse) — explains
  "Get_Rupee" at pool offset 0.
- PAIR-BIRTH LOG: _create's this/anchor mirror FLIPPED to match when the
  if became switch — control-flow shape feeds the allocator direction!
  (Strongest mirror-puzzle lead yet — p2's setAnm/_execute/Proc sites
  might flip with an equivalent-shape change.)
- REMAINING sub-100 (9 fns): ALL pool-settling rows + the 4 §2b parks
  (chg_anmAtr 2, control_anmAtr 2, privateCut 2, nodeOb1Control mirror).
  NEXT PHASE (p2 playbook rounds 19+): pool forensics (my pool has extras/
  order deltas), .data/.bss order, .text emission order tool, then the
  REL SHA gate (flip Matching temporarily to measure), cross-version.

### Round 10 (03:25Z) — order-forensics opener (phase data for the REL gate)
- Section match: .text 99.47 / .rodata 95.68 / .data 89.58 / .bss 98.04 /
  .ctors 100.
- .bss CONVERGED: my layout matches TARGET ABSOLUTE offsets — anon anchor
  @0xC pile (17 header-inline anon objects 0xC-0x58), l_HIO@0x58 (0x3C),
  l_kb_actor@0x94, l_kb_count@0xE4, l_kb_bit@0xE8, nodeOb1Control static
  cXyz region @0xEC(dtor-obj)/0xF8(guard)/0xFC(value), l_check_flg@0x108.
  Residue likely 1-2 rows.
- .data order (mine): 3 anon 12-byte objects, _three/_half fake_sqrtf
  statics (NOTE: p2 target order was _half,_three — mine reversed; but
  REL link DROPS these per tc precedent, so only .o-level), l_evn_tbl,
  3 more anon, a_anm_prm_tbl x3 (0x58/0xF8/0x184), @2023 (0x88 — ptmf
  consts?), a_cut_tbl@0x298, method tbl, g_profile, vtables. Compare vs
  target .s .data .obj sequence next (p2 round-6 script pattern).
- .rodata: object sequence mostly parallel to target (a_prm_tbl FIRST in
  mine vs @4239 anchor first in target — my a_prm_tbl at 0x0 vs target
  0x120! The HIO-ctor local static emits EARLY in mine. Target's rodata
  starts with the float anons. Likely fix: parse order of the prm blob —
  investigate whether target's a_prm_tbl$4151 counter (4151 > 4239-ish
  region... actually 4151 < 4239) — hmm counter says a_prm_tbl parsed
  FIRST yet emits at 0x120. MWCC rodata emission may order floats first?
  Experiment matrix for next rounds.)
- NEXT: (1) target .data/.rodata .obj order lists; (2) reorder statics/
  first-use to converge; (3) temp-flip Matching, REL build, byte-diff;
  (4) the 4 parks re-examined under the control-flow-shape lead (the
  _create if->switch flip); (5) SHA gate + cross-version + PR staging.

### Round 11 (03:30Z) — REL-gate forensics state (commit f543c2a2)
- .bss 100% (names recovered from target .bss list: l_check_inf[20]/
  l_check_wrk/l_check_flg — my l_kb_bit and talk_1's accumulator were
  ONE object; static cXyz = a_eye_pos_off). .data 94.01 (a_prm_tbl/
  a_size_tbl/a_hed_mdl_resID_tbl de-const'd into .data; target .data
  object order == mine from l_evn_tbl@0x30 onward; residue = my 3 anon
  12-byte header statics + _three/_half at 0x0-0x30 where target has
  a_prm_tbl — likely REL-reconciled per tc precedent, VERIFY not assume).
- .text emission order IDENTICAL through slot 74; mine has 13 EXTRA weak
  fns interleaved from slot 75 (__dt__12fopNpc_npc_c, __dt__12cCcD_CylAttr
  + 11 more; mine 126 fns vs target 113). p2 precedent: extras get
  REL-link-dropped — but ob1's first gate run MISMATCHED (c51ba74a vs
  a1f385ef, sizes 23032 vs ?), so compare the KEPT set in the linked REL
  next: objdump/dtk the built .rel section sizes vs retail Yaz0-decomp,
  then attribute (extra-weak survival? text order of kept? data anons?).
- p2 SHAPE-LEAD calibrated (p2-progress round 18): no direct retrofit —
  feeds the transform-search harness vocabulary instead.

### Round 12 (03:37Z) — REL convergence: rodata 100%, data clean; SHA distance = 4 sites (commit fd111756)
- .rodata 100% BYTE-EXACT (0x11D): the missing 0xC was the debug GXColor
  trio {FF0000,0000FF,00FF00 alpha 0x80} — bare `(GXColor){...};`
  expression statements at the END of _draw (bm1/km1 sibling-attested
  idiom, stripped dDbVw leftovers).
- .data REL-clean: HIO blob initializer bug fixed (s16 split had eaten
  the 0.5f slot — 0x400, 0, then the four floats); l_evn_tbl moved to
  AFTER the HIO ctor (a_prm_tbl emits first; Get_Rupee still pools
  first). Leading .o statics (3 anons + _three/_half) confirmed
  REL-LINK-DROPPED. Sizes equal 23040.
- REL DISTANCE FULLY ATTRIBUTED (5681 bytes): the 3 branch-polarity
  parks (chg_anmAtr, control_anmAtr, privateCut) + nodeOb1Control's
  r29/r30 mirror + their local displacement cascades + reloc-table
  echoes. Nothing else.
- 🔑 KM1 CONTROL EXPERIMENT: km1's byte-matched chngAnmAtr emits MY
  beq-skip shape from the IDENTICAL && spelling — so ob1's target bne+b
  needs a structurally DIFFERENT spelling (nested-if / early-return /
  statement-after-if variants), NOT counter-parity. Polarity parks are
  decodable in principle. Experiment matrix for next context: (a)
  `if (i_no < 7) { if (i_no != field) { body } }` nested; (b) body with
  trailing statement; (c) early-return forms; same matrix for
  privateCut's beq+b site and control_anmAtr's tree-root.
- Status: task #12 at the SAME endgame class as p2 (#9) — 4 small
  allocator/polarity sites from SHA, everything else byte-true.
- (a) nested-if FALSIFIED for chg_anmAtr (2 rows unchanged) — matrix continues: trailing-statement, early-return, no-code-then variants; harness is the systematic vehicle.

### Round 13 (03:40Z) — HARNESS BUILT + first falsification campaign
- tools/decoder_transform_search.py (WWDP) — variant-spec brute-forcer,
  row-count oracle, ~8s/cycle, auto-restore. Spec format documented in
  the file header.
- chg_anmAtr polarity site: 18 variants falsified (nested-if, early
  returns, goto, else-empty, ;-then, || and && comma-expression
  short-circuits, do-while-break, switch-on-bool). LESSON: branch
  POLARITY is not a local-spelling lever — MWCC canonicalizes the sense
  (cousin of the p2 reassociation lesson). Park confirmed §2b.
- NEXT VECTOR (for both TUs' endgames): CROSS-FUNCTION shape variants —
  the _create if->switch flip changed ANOTHER site's allocation, so pair
  each park with shape changes in OTHER functions via multi-site specs.
  p2's spec should pair setAnm/_execute/Proc sites with shape variants
  in their NEIGHBOR functions.
- Cross-fn probe 1 (control_anmAtr shapes × 3 park symbols): NO coupling — chg/privateCut unmoved; control baseline 6 rows (tree-root), if-chain variant ties, default-case worse. Next pairings: setAnm_ATR/anmAtr-side variants; then p2's neighbor-spec.
- Round 14: km1 include block adopted (+Ob.h res enums for PR naming pass); counter gap NOT include-driven at this level (+9 only, PCH covers); cross-probe 2 (setAnm_ATR shapes x 5 syms): fully canonicalized, zero movement. Endgame total: 21 rows across 4 sites (chg 2 / control 6 / privateCut 2 / nodeOb1Control 11).
- nodeOb1Control in-function variants: decl-split/ptr-local tie at 11, static-after-jntNo worse (19) — pair-birth order robust to local spelling; most of the 11 rows are anchor-name display + the r29/r30 encoding bytes. Family confirmed systemic; all in-function levers exhausted (25+ variants across 4 sites).

### Round 15 (03:44Z) — upstream regalloc.md consulted (round 16b path 3 CLOSED)
- Cheatsheet levers vs our ledger: decl-order ✗tried · base-ptr local
  ✗tried (p2) · C-cast vs static_cast ~partially · temps ✗tried ·
  **INLINES — "the inline itself may be implemented wrong; modify the
  inline body" — UNTRIED for the current sites** · **const on inline
  PRIMITIVE params (invisible in mangling) — UNTRIED systematically.**
- THE INLINE-LEVER CAMPAIGN (next context, both TUs): candidate inlines
  at the mirror sites — nodeOb1Control: mDoMtx_stack_c::copy/get,
  J3DJoint::getJntNo, getAnmMtx, JntCtrl accessors; p2 setAnm:
  dNpc_setAnm edge, getMoveSpeed/getNowCut; p2 _execute: the dComIfG
  +0x12a0 accessor. DISCIPLINE: shared headers — ninja baseline on
  main first, experiment, ninja changes for regressions, tc canary SHA
  after any d_npc.h/m_Do touch.
- With this, ALL of round 16b's three paths are progressed: (1) ob1
  pattern-learning DONE (pair-birth log + the _create flip datum),
  (2) harness BUILT+campaigned, (3) upstream lore CONSULTED (the
  inline lever is their answer for persistent regswaps).
- Call-site inline spellings (hoist/static_cast/const-local/redundant-cast): all tie at 11 (redundant cast worse 27). ~34 variants falsified across the mirror/polarity family this session. In-function + call-site space EXHAUSTED; the shared-header inline-body campaign (regalloc.md's lever, baseline/changes discipline) is the next context's opening move.

## Round 16 (2026-08-18, so-campaign spillover): next_msgStatus 16 -> 4; TU 99.50 -> 99.80%

The case-BODY-ORDER lesson proven at scale on so transfers verbatim to ob1:
moving case 0xa93 / 0xa97 ABOVE the ten-case eventBit group reproduced the
target emission (16 rows -> 4) in ONE build.
FALSIFIED: restructuring the ten-case group to fall through into default
(4 -> 33). The group's POSITION in the case list dominates the shape; do not
move it to chase the shared else/default block.
Remaining 4 rows: the group's else-arm emits its own status block where the
target shares default's. ob1 now 108/115 exact, fuzzy 99.80%.

## Round 17: chg_anmAtr + control_anmAtr + _create probes — TU 99.81%

- chg_anmAtr: compound "if (i_no < 7 && i_no != field_0x800)" -> NESTED ifs
  matches the target's beq (not bne) shape.
- control_anmAtr: case 6 before case 3 measures marginally better (equal row
  count, fuzzy 99.80 -> 99.81). NOTE: MWCC builds the COMPARE CHAIN from
  case-label source order INDEPENDENTLY of where the bodies land, so the two
  orderings only trade which instructions align — this is not a free win like
  the next_msgStatus body reorder was.
- _create (2 rows): the final "if (!createInit()) return cPhs_ERROR_e;" emits
  bne+b where the target has a single beq. THREE spellings all give 2 rows:
  !createInit()+return, createInit()==0+return, createInit()+break-then-return.
  Assign-and-fall-through (state = cPhs_ERROR_e; break;) was WORSE (4). PARKED.
STANDING (all three of my TUs, same build):
  so  138/187 exact 99.56% | ob1 108/115 exact 99.81% | p2 131/145 exact 99.94%


## Rounds 18-19 - branch-shape bucket (108 -> 109/115, 99.807 -> 99.832%)

Came back to ob1 after the `so` placement work, with the branch-polarity
lesson sharpened. ob1 stood at 7 fns / 38 rows; three of those functions
differed by exactly 2 rows and ALL THREE were the same shape - target
`beq`+`b` (branch to body, branch over it) where I had a single inverted
branch, or the reverse.

**privateCut - EXACT.** The action-result test was written as a ternary,
`(int)field_0x7FE == 0 ? event_action() : true`. The donor uses a **switch
with a default**:

    bool ret;
    switch ((int)field_0x7FE) {
    case 0:  ret = event_action(); break;
    default: ret = true;
    }
    if (ret) { dComIfGp_evmng_cutEnd(i_staffId); }

The tell was in the SAME function: its other dispatch (the `getIsAddvance`
switch) already matched byte-for-byte with the `beq`+`b` shape, so the shape
was demonstrably available in this TU - I just had the second site written as
a ternary. **When one site in a function already matches with shape X and
another differs by exactly that shape, the answer is usually X.**

**chg_anmAtr - still 2 rows, TWO forms tried and falsified.** Target is
`cmplw r5,r0; bne <body>; b <end>` where I emit a single `beq <end>`.
Tried `switch (i_no != field_0x800) { case 1: }` - MWCC materialised the
boolean (`cmpwi r0, 0x1`), 2 -> 6 rows. Tried an empty then-block with the
work in the `else` - MWCC folded the empty branch away, back to the original
2. Reverted to the plain `if`. **Do not re-run either of those.** The compare
is variable-vs-variable, so a switch cannot express it directly; the shape
must come from somewhere else and I have not found it.

**control_anmAtr - bodies now exact, 6 rows are dispatch only.** The two case
BODIES were in the wrong source order: the target emits `li r4, 0x4` (case 3)
before `li r4, 0x7` (case 6), so case 3 must come first in source. Swapped.
Rows 13-29 are now identical. What remains is purely the switch DISPATCH scan
direction - target tests 3 then 6 (ascending), mine tests 6 then 3
(descending) - and that is NOT controlled by source case order: I measured the
identical dispatch before and after the swap. Same case set, same types.
**HONEST NOTE: the swap left the exact count flat and moved fuzzy by -0.002%
(99.8345 -> 99.8324), i.e. it did not pay off on the fuzzy axis.** I kept it
anyway because the body-emission order is a structural fact about the donor
and is required for an eventual SHA match; the fuzzy dip is noise-level and
the wrong thing to optimise against. Flagging it rather than hiding it.

**_create - 2 rows, hypothesis formed, NOT yet applied.** Target materialises
`li r3, 0x5` (cPhs_ERROR_e) UNCONDITIONALLY and then branches; mine branches
first and sets 5 on one path. That is the shape of a result local:
`int ret = cPhs_ERROR_e; if (createInit()) { ret = state; } return ret;`
rather than my early `return cPhs_ERROR_e;`. Not applied because the site sits
inside the phase switch and the restructure is more invasive than 2 rows
justifies right now.

STATE: ob1 **109/115 exact, 99.8324%**, 6 fns / 36 rows.
REMAINING: ob_movPass 11, nodeOb1Control 11, control_anmAtr 6 (dispatch scan
direction), next_msgStatus 4, chg_anmAtr 2, _create 2.
Cross-check: so 175/187 and p2 133/145 UNCHANGED across these commits.


## Round 20 - ob_movPass diagnosed (not fixed), and I removed my own dead flag

**`ob_movPass` (11 rows) is STACK-SLOT ORDER, and the frame size already
matches.** Three cXyz slots exist on both sides at 0xC / 0x18 / 0x24 and both
frames are 0x40 - only the assignment permutes. In use order the target takes
0x24 -> 0x18 -> 0xC (descending, i.e. by declaration order), mine takes
0xC -> 0x24 -> 0x18. My named locals `delta` and `flat` correctly hold the top
two slots; the odd one out is the unnamed temporary for `mPathRun.getDir()`,
which the donor appears to hold in a NAMED local declared before them.

TWO ATTEMPTS, both recorded so they are not repeated blind:
- `const cXyz& delta` -> `cXyz delta` (value): **11 -> 22 rows and the frame
  grew to 0x50.** The const-reference binding is CORRECT - the matching 0x40
  frame is the evidence. Do not "simplify" it.
- Naming the getDir() result (`cXyz dir = mPathRun.getDir();`): **DID NOT
  COMPILE.** Untested, therefore NOT falsified - it is still the live
  hypothesis, and it needs the real `getDir`/`chkPointPass` signatures checked
  first. **CAUTION FOR MY OWN FUTURE SELF: the failed build left a stale .o and
  the differ happily reported the PREVIOUS experiment's 22 rows as if they were
  this one's.** I only caught it because the number was suspiciously identical.
  Always read the compiler's exit status, not just the row count.

**`nodeOb1Control` (11 rows) is register colouring** - a straight r29/r30 swap
against a .bss anchor, same bucket as `so`'s `_createHeap` and `_nodeControl`.
Nothing tried yet.

**REMOVED MY OWN SCAFFOLDING: `-DSQRTF_CONST_LITERALS`.** I added it to ob1's
`ActorRel` in the campaign-kickoff commit (1673d54c) as "vintage PCH wiring",
and `MSL_Common/Include/math.h` really does gate `sqrtf`'s form on it, so I had
been assuming it was load-bearing for `ob_movPass`'s `std::sqrtf`. **Measured:
it is INERT.** With and without, ob1 is 109/115 exact at 99.8324% - identical
to four decimals, and `ob_movPass` stays at 11 rows either way. Dropped from
configure.py. Workspace-wide re-report confirms no other unit moved (only
configure.py changed, 1 line). **An assumption I carried for a whole campaign
cost one command to falsify.**

STATE unchanged by this round: ob1 **109/115 exact, 99.8324%**, 6 fns / 36 rows.


## Round 21 - NEGATIVE RESULT: ob1's pools are already aligned

After the `so` campaign, where four placement root causes were worth ~36 exact
functions, I ran the same alignment walks against ob1 expecting the same well.
**It is dry, and that is the useful finding.**

    STRINGS: target 13 | mine 13  -> ALIGN
    RODATA : aligns 22/23 objects; first divergence 0x0090

The single rodata divergence is only the position of the string pool relative
to one data table (target has a data object at 0x90, mine starts the strings
there) - and crucially **none of ob1's six remaining functions has a
rodata-offset row in its diff.** So this misplacement is costing ZERO rows.

The walk was run with a GENERIC instrument (scratch `pools.py <tu>`) so it can
be pointed at any TU: it parses the target `.s` for the `@stringBase0` contents
and the `.rodata` object list, dumps mine with `objdump -s/-t`, and reports the
first divergence by VALUE.

**CONCLUSION: ob1 has no placement lever left.** Its remaining 36 rows are the
hard bucket - stack-slot allocation order (ob_movPass 11), register colouring
(nodeOb1Control 11), switch dispatch scan direction (control_anmAtr 6), plus
next_msgStatus 4, chg_anmAtr 2, _create 2. Do not go looking for another
include-position sweep here; there is nothing movable to sweep.
