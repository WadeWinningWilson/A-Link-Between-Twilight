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
