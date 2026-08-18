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
