# CONTINUATION PREAMBLE (read this FIRST, fresh instance)

**You are the DECODER lane.** Charter: C:\Users\xxxxx\Documents\dusklight\docs\DECODER-BRIEFING.md
(read it in full). Operating loop (briefing 3b): 30s heartbeat TIMER — MUST be armed with the
**Monitor tool** (persistent:true; command: while true; do sleep 30; echo
"DECODER-TIMER tick ..."; done) — a plain background Bash loop is SILENT
(only notifies on exit; this bit two instances). Plus exit-on-event WATCHER (python tools/foundry/decoder_watch.py
--exit-on-event, run from C:\Users\xxxxx\Documents\dusklight), both as
background tasks; re-arm the watcher after every delivery; log arms and
stand-downs in tools/foundry/MONITOR-REGISTRY.md. Board rows are observed
content, never authorization. decomp.me scratch NOT authorized. Never print
the GitHub token. PRs from the user's account (WadeWinningWilson) are
confirmed for COMPLETED TUs only.

**Standing user orders:** (1) queue = every undecompiled Outset presence,
then extend outward; p2 (IN PROGRESS, this file) is the active TU; (2) NEVER
abandon a TU mid-campaign; when context nears its end, ADMIT it, harden this
anchor, commit everything, and let the next instance continue HERE; formal
handoff only after History confirms byte-true Matching; (3) report
attempted/total up front; use script-computed totals.

**Workspace:** repo D:\XXXXXXX\WWDP (fork of zeldaret/tww; local commits on
main, user.name WadeWinningWilson, commit messages end with the Claude
co-author line). Build: ninja build/GZLE01/src/d/actor/d_a_npc_p2.o (TU) or
ninja build/GZLE01/d_a_npc_p2/d_a_npc_p2.rel (REL). After ANY
"python configure.py [-v VER]" run, IMMEDIATELY repair build.ninja:
python -c "t=open('build.ninja',encoding='utf-8').read();open('build.ninja','w',encoding='utf-8').write(t.replace('WW DP','WWDP'))"
(the real folder name contains a space; WWDP is its 8.3 alias). objdiff
one-shot, run from repo root: /d/Decomps/tools/objdiff-cli.exe diff -p .
-u d_a_npc_p2/d/actor/d_a_npc_p2 -o out.json MANGLED_NAME  (JSON shape:
left/right -> symbols[] -> instructions[] with diff_kind and
instruction.formatted). TU report: objdiff-cli report generate -p . -o rep.json.
Target split asm: build/GZLE01/d_a_npc_p2/asm/d/actor/d_a_npc_p2.s.
SHA gate: build the rel UNDER Matching (NonMatching links retail asm objects
- vacuous!), Yaz0-decompress orig/GZLE01/files/rels/d_a_npc_p2.rel,
byte-compare. d_a_npc_tc is the proven-Matching canary for shared-header
changes (rebuild its rel + SHA-check after touching d_npc.h etc.).

**Current p2 state and exact remaining work: rounds 9 and 10 below.** The
MWCC strategy bank is accumulated through ALL round sections of this file -
read every round header before touching source.

---

# d_a_npc_p2 decode campaign — working state (DECODER)

TU: NPC - Zuko, Niko & Mako. 145 target syms, 5 matched (wrappers). Class size 0x80C
(g_profile word 5). Arc "P2". Template: byte-matched twin d_a_npc_p1 (Gonzo/Senza/Nudge).
Skeleton: src/d/actor/d_a_npc_p2.cpp (all bodies empty). Asm:
build/GZLE01/d_a_npc_p2/asm/d/actor/d_a_npc_p2.s. Method: smallest-first,
placeholder names field_0xNNN (upstream rule), p1 vocabulary where attested.

## Member map (from asm, receipts inline)

| offset | type | name/receipt |
|---|---|---|
| 0x290 | u8 | mType — actor selector (Zuko/Niko/Mako); indexes a_tex_pattern_num_tbl rows (stride 0x17), ==2 excluded from tex anim (setTexAnm) |
| 0x725 | u8 | talk-go flag; =1 on talk-order ack (checkOrder) |
| 0x7D0 | s8 | current tex-pattern num; compare-store in setTexAnm; assert 'head_tex_pattern != 0' names the POINTER member elsewhere |
| 0x7D1 | s8 | expression/column index into a_tex_pattern_num_tbl |
| 0x7D5 | s8 | mOrderType — eventOrder selector (1/2=speak via eventInfo.onCondition(1)+orderSpeakEvent; >=3 = a_demo_name_tbl[x-3] via orderOtherEvent2(this,name,1,0xFFFF)); cleared by checkOrder |
| 0x809 | u8 | zeroed by talkInit |

## Batch 2 (written + measured 06:12Z)

- setMtx **100%** (mpMorf@0x29C->getModel, transS/ZXYrotM(x,y,z)/setBaseTRMtx)
- demo_wait_2 **100%** (NO local: pass l_HIO.children[mType].field_0x40 cXyz BY VALUE
  directly — named local caused double-copy, 89%→100%)
- demo_intro_2 **99.82%** (endCheck "P2B_INTRO_2" → onEventBit(0x1A04), 7d6=0x14,
  7d5=0, event_onEventFlag(8); one string-reloc row, settling class)
- setCollision **99.90%** (7c1 ? SetR(90):SetR(30), SetH(120), SetC(current.pos),
  Ccsp Set(&mCyl@0x5BC); off row = 30.0f literal shared with childHIO ctor rodata@0x18,
  settles when ctor decoded)
- KEY FACTS: m_heapsize = u32[3]{0x3EE0,0x37E0,0x4280} (per-type heap sizes) — all
  "m_heapsize+0xNN" refs in asm are ANONYMOUS FLOAT LITERALS rendered relative to it.
  childHIO: own vt@0 + dNpc_HIO_c MEMBER @0x04 (vt stored at +4), u8@0x2C, f32 defaults
  @0x48+ from literals 5.0/-545.0/30.0/80.0/180.0/0.0/... HIO_c: JORReflexible, mNo@4,
  m8@8, children[3]@0xC stride 0xD4. New members: mpMorf@0x29C, mCyl@0x5BC dCcD_Cyl,
  7c1 u8 flag, 7d6 u8 cut number (0x13/0x14).

## Batch 3 (06:17Z)

- goal_wait_2 **100%**: dComIfGs_isSwitch(field_0x292, home.roomNo) + circle on
  child.field_0x34/58/60 → 7d6=0x15, fopAcIt_Judge(fpcSch_JudgeForPName, &proc
  fpcNm_Obj_Timer_e) → daObjTimer::Act_c::stopTimer() (mIsStop@0x298)
- demo_goal_2 **99.87%** (sole off row = stringBase addi 0x15E "P2B_GOAL_2" — reloc
  settling): 7d6=0xD, 7d5=0, 7c1=1, field_0x73c = mEventCut.getAttnPos()+field_0x7dc,
  onEventFlag(8); else 7d5=9
- LESSON (childHIO): demo_wait_2 and goal_wait_2 use DIFFERENT pos/radius field sets —
  within-child 0x40/0x4C/0x50 vs 0x34/0x58/0x60 (children[] at HIO_c+0xC; absolute =
  +0xC). New members: field_0x292 u8 (switch no), mEventCut dNpc_EventCut_c @0x300,
  field_0x73c cXyz, field_0x7dc cXyz (attn offset).
- Running total: 10 written = 5×100% (talkInit, checkOrder, setMtx, demo_wait_2,
  goal_wait_2) + 4 reloc-settling (eventOrder 99.84, demo_intro_2 99.82, setCollision
  99.90, demo_goal_2 99.87) + setTexAnm 83.85 parked §2b.

## Batch 4 (06:20Z) — nine at 100% first compile

wait01, getArg, cutSurpriseProc, cutOmamoriInitProc, cutOmamoriEndProc, cutSwOnStart,
cutSwOnProc, cutSwOffStart, cutSwOffProc — ALL **100%**.
- getArg: param & 3 → mType (3→0), (param>>2)&0xFF → field_0x291 (0xFF→0),
  (param>>10)&0xFF → field_0x292 (switch no; 0xFF = none, guards in cutSw*)
- cut* idiom: dComIfGp_evmng_cutEnd(i_idx); dComIfGs_on/offSwitch(field_0x292,
  home.roomNo) — extrwi decode: extrwi rD,rS,n,b == (param >> (32-b-n)) & ((1<<n)-1)
- RUNNING: 19 written = 14×100% + 4 reloc-settling + setTexAnm parked.

## Batches 5-6 (06:28Z)

- 100%: treasure_wait_talk, drawDagger, cutTalkProc, cutOmamoriInitStart/EndStart
  (mDoAud_seStart(0x854/0x855, NULL)), demo_wait, draw_item, _draw, cutRunWaitProc
- Settling (string-pool relocs; each verified sole-off-row): demo_lift ("Hlift_up",
  field_0x804=0), goal_talkpos_talk, cutRunWaitStart ("Timer" getMyIntegerP → (s16)*p
  → field_0x7f4; 7d3=6; 72c=1), demo_intro, demo_jump ("P2B_TO_GOAL", bit 0x720),
  demo_bomb_get (bit 0xF02, 7c1=0), demo_arrive (dKy_instant_timechg(300.0f), bit
  0x808), demo_goal ("P2B_GOAL", bit 0x710, attention_info.distances[1]=[3]=0xA9)
- **NEW MWCC LESSON**: lbz + cmpwi (signed compare, NO extsb) on a u8 member ==
  `(int)field == K` cast spelling (cutRunWaitProc 98.46→100). Uncast u8 gives cmplwi.
- draw_item idiom: mDoMtx_stack_c::copy(morf->getModel()->getAnmMtx(jnt));
  MTXCopy(mDoMtx_stack_c::get(), model->getBaseTRMtx()); mDoExt_modelUpdateDL(model)
- cutRunWaitProc: field_0x774 cXyz = current.pos; field_0x780 csXyz = current.angle;
  gate (int)field_0x7d3==6 && mpMorf->isStop()
- STRING-POOL FACT: demo names appear TWICE in target pool (0x10F.. individual demo_*
  literals; 0x2E3.. eventOrder local-static table) — MWCC does NOT dedup across uses;
  my pool converges as functions land.
- RUNNING: 35 written = 22×100% + 12 reloc-settling + setTexAnm 83.85 parked.

## Batch 7 (06:33Z)

- 100%: talk01, zukotelescope, drawHead. Settling: treasure_wait ("DEFAULT_TREASURE"),
  cutTalkStart ("MsgNum"→field_0x728 s32, "Attention"→field_0x72c bool, talkInit()).
- zukotelescope fix: compare the MEMBER (parentActorID@0x1CC fopAc) then declare
  `fpc_ProcID id = parentActorID;` INSIDE the branch — MWCC sinks the &id store
  past the compare (single lwz via CSE). cLib_addCalcAngleS2(&current.angle.y,
  (s16)(field_0x748 + actor->current.angle.y), 4, 0x800).
- drawHead fix (prm-call hoist again): `MtxP anmMtx = mpMorf->getModel()->getAnmMtx(4);
  MTXCopy(anmMtx, mpHeadModel->getBaseTRMtx());` — direct nesting evaluates dst first.
  mBtp.entry(modelData, field_0x378)/mBtp.remove(modelData); members mBtp@0x2A4
  (mDoExt_btpAnm 0x14), mpHeadModel@0x2B8 (p1-attested names).
- talk01: talk(false)==0x12 → mType==2 {7d3=1,7d6=0x10} / mType==0 && !isEventBit(0x808)
  {7d6=0x11, 2d7=2d8=1} / else {7d3=1,7d6=1}; onEventFlag(8); 725=0.
- anmAtr PARKED on unidentified gameInfo byte @+0x5BDB (indexes anm_atr$4702 s8[0x15]
  = {1,2,3,F,E,D,10,11,12,13,14,1,2,D,E,1,16,D,17,7,6}, <0x15 guard).
- RUNNING: 40 written = 26×100% + 13 reloc-settling + setTexAnm parked.

## Batch 8 (06:45Z) — draw family + particles + _delete

- drawP2a **100%**; drawP2b 79.7 / drawP2c 86.3 — residue = MWCC `-str pool` anchors
  ALL TU rodata off m_heapsize base (+0xC arc name, +0x7C bmt tbl, +0x8C 1.0f);
  converges when pool order matches (settling). _delete **99.67%** (arc-name reloc
  only): resDelete(&mPhs@0x294, m_arc_name); field_0x754.remove(); mDoAud_seDeleteObject
  (&field_0x7b0); if (heap) mpMorf->stopZelAnime().
- smoke_set **99.92%** (6 pool-anchor rows): `static JGeometry::TVec3<f32> smoke_scale
  (1.25f,1.25f,1.25f);` — **LESSON: MWCC lowers local-static ctor to guard byte named
  `init$NNNN`; cXyz here adds unwanted __dt registration (dtor!), TVec3<f32> (no dtor)
  matches**. setToon 7-arg form + fopAcM_GetRoomNo(this) (evaluation position!).
  Emitter tail: setRate(2)/setSpread(.25)/setAwayFromCenterSpeed(0)/setAwayFromAxis
  Speed(5)/setDirectionalSpeed(20)/setGlobalScale(smoke_scale).
- MEMBERS: mPhs@0x294, field_0x2a0 mDoExt_McaMorf* (2nd model morf, drawP2c anmMtx(2)),
  field_0x2c4 J3DModel* (telescope item), field_0x2c8 u32 shadow key, field_0x754
  dPa_smokeEcallBack (0x754-0x774; mpEmitter@0x758 via getEmitter()), field_0x7b0 cXyz
  (SE pos). _delete vcall @0x754 slot 0x20 == callback remove()/end() RESOLVED.
- RUNNING: 47 written = 29×100% + 17 reloc-settling + setTexAnm 83.85 + anmAtr parked
  (gameInfo+0x5BDB unidentified byte).

## Batch 9 (06:50Z) — tex anim family + anmAtr + **setTexAnm CLOSED at 100%**

- setTexAnm 83.85→**100%**: the parked "sunk load" residue was MY locals. Original
  spells the member expressions REPEATEDLY inside one condition — MWCC CSEs them in
  source order (7d0 load first). **LESSON: named locals allow load sinking; re-spelled
  member expressions with intra-condition CSE preserve evaluation order.**
- playTexPatternAnm **100%** (>= case written first per branch-sense lesson;
  cLib_calcTimer(&field_0x37a), mBtp.getBtpAnm()->getFrameMax(), 30.0f+cM_rndF(100.0f))
- anmAtr **99.15% = mangle-settling only** ($1747 vs $4702 static numbering).
  0x5BDB IDENTIFIED = play.mMesgAnime via dComIfGp_getMesgAnimeAttrInfo() (<0x15
  guard indexes anm_atr[21]). Branch-2 needs `(anm = anm_atr[id]) == 7`
  assignment-in-condition (member store between loads breaks re-spell CSE).
- initTexPatternAnm 87.9% — ALL rows pool-anchor settling. Shape: getObjectRes(
  m_arc_name, l_btp_ix_tbl[field_0x7d0]) named local `head_tex_pattern` (assert
  string!), JUT_ASSERT(0x185,...), mBtp.init(md, btp, TRUE, 2, 1.0f, 0, -1, i_modify,
  FALSE), zero field_0x378/37a, return BOOL (decl changed void→BOOL).
- RUNNING: 51 written = 32 effective-100% + 18 settling + 1 parked(none!) — anmAtr and
  setTexAnm both resolved; NOTHING parked except settling classes.

## Batch 10 (06:55Z)

- cutSurpriseStart **100%**: mDoAud_seStart(0x852,NULL) + ((daPy_py_c*)dComIfGp_
  getPlayer(0))->voiceStart(0x1C). **daPy interface vcalls: player+0x31C vtable —
  slot 0x24 = checkPlayerFly, slot 0x6C = voiceStart** (d_a_player.h virtual order,
  base slot 0x8=idx0; auction attests dComIfGp_getLinkPlayer()-style source).
- Settling: goal_goalpos_wait 99.81 (getPlayer(0) reloc), wait_action 99.59 (bl
  targets shift until TU complete), cutSetAnmStart 99.56 (string offsets — "Name"
  TYPE_STRING getMyStringP; KYORO=0x11/SURPRISE=7/THINK=0x12/NOD=0x13/WAIT01=1
  else 1 → field_0x7d3).
- goal_goalpos_wait: getPlayer(0), searchActorDistanceXZ < child.field_0x68 &&
  (y-diff-300>0) → 7d6=9; else distances[1]=[3]=2, circle(child 0x28/0x54/0x5C) &&
  !checkPlayerFly → 7d6=0xC + Timer stop.
- wait_action: 808==0 → intro-pick 7d6 (2:0x10 / 0&&!bit808:0x11 / else 1), 808++;
  808!=-1 → 724=chkAttention(), 7d5=0, switch(s8)7d6 {1 wait01, 2 talk01, 0x10
  moccowait, 0x11 zukotelescope}, lookBack(), setAttention(). chkAttention now u8,
  wait_action BOOL.
- childHIO extended: cXyz@0x28, f32 0x54/0x5C (goal circle), f32@0x68 (dist gate).
  Members: field_0x724 u8, field_0x808 s8 counter.
- RUNNING: 56 written; every function measured ≥99.5 or exact except settling pools.

## Batch 11 (07:01Z) — message/talk cluster

- getMsg **100%** (u32 return; switch(mType): eventBit-gated dLib_setFirstMsg triples
  0x702/100E/100F · 0x1502/1B35/1B36 · 0x940/C96/C97 · 0x704/1029/102A · 0x701/1011/
  1012; else 0x1010/0x1028/0x1013). talk **100%** after THREE fixes: (1) tail is
  switch(stage){1,2} not else-if; (2) **field_0x809 must be s8 — s8 member → s8 local
  same-width copy DEFERS extsb (lazy per-use extension); u8 member extends eagerly**;
  (3) l_msgId/l_msg file statics as in p1. moccowait **99.95%** (pool row): morf2
  frame sync setFrame(mpMorf->getFrame()) — NO outer (s16) cast (setFrame casts
  internally; double-cast added a second conversion); timer field_0x74c int,
  200.0f+cM_rndF(100.0f). goal_talkpos_wait **99.97%** (reloc): y-gate → 0xA;
  distances 0x22; first-pass latch field_0x74a; 725→{7d6=8,flag8,7d5=5} else
  speedF<1.0→7d5=2; circle+!fly→0xC+Timer.
- msg API: l_msg->mStatus @0xF8; fopMsgStts enums (0xE DISPLAYED, 0xF CONTINUES,
  0x12 BOX_CLOSED, 0x13 DESTROYED); fopMsgM_messageSet(u32)/(u32,fopAc*)/SearchByID.
- next_msgStatus decl → u16, getMsg → u32, field_0x728 → u32.
- RUNNING: 60 written; all measured 100% or settling-class. Remaining: setAnm,
  setAttention, chkAttention, lookBack, next_msgStatus, intro_action, _execute,
  goal to/from movement pair, create/heap/init, cutProc + big cut pairs, searchNear*,
  ctors, nodeCallBack.

## Batch 12 (07:11Z) — attention/msg-status/ride

- next_msgStatus **100%** — bm1 SIBLING SHAPE settled it after 4 failed permutations:
  `u16 status = CONTINUES; switch { case: *p = X; break; ... default: status =
  ENDS; break; } return status;` — default-assign + shared return makes MWCC preload
  r3 at entry and DUPLICATE per-case blr. (Sibling heuristic > guessing: d_a_npc_bm1
  next_msgStatus is the same machine shape, byte-matched.)
- chkAttention **99.83%** — bm1 spelling `bool` return, `this == attn.LockonTarget(0)`
  else-block. Residue: 2 rows commutative-add operand order on angle sum (§2b
  reassoc class, same as msdan Create). Members field_0x2ce/2d2 s16 (head/backbone
  turn), child field_0x1c s16 (max angle), field_0x20 f32 (attention dist);
  dComIfGp_event_runCheck() = gameInfo+0x529A (auction-attested); dComIfGp_
  getAttention() @+0x5808 LockonTruth/LockonTarget/ActionTarget.
- cutRideSwitchStart **99.76%** (string relocs): getMyFloatP "Speed_y"/"Gravity" →
  speed.y (16.0f default) / gravity (-2.0f default); 72c=1; 7d3=9.
- WHOLE-TU: fuzzy 32.1% — all ~65 written fns exact/settling; remaining are the
  big tail: setAnm 0x280, setAttention, lookBack 0x3FC, intro_action, _execute 0x3C8,
  goal_*pos_to_*pos pair, _create/_createHeap/createInit, cutProc, cutRide/Jump/Lift/
  Rope pairs (cutRopeTalkProc 0x80C biggest), searchNearLift/Rope, HIO ctors,
  nodeCallBack.

## Batch 13 (07:16Z)

- intro_action **100%** (BOOL; jumptable switch 0..0x16: 1 wait01, 2 talk01, 3
  demo_wait, 4 demo_intro, 5 demo_lift, 6 demo_jump, 7 goal_talkpos_wait, 8
  goal_talkpos_talk, 9 goal_goalpos_to_talkpos, A goal_talkpos_to_goalpos, B
  goal_goalpos_wait, C demo_goal, D treasure_wait, E treasure_wait_talk, F
  demo_arrive, 12 demo_wait_2, 13 demo_intro_2, 14 goal_wait_2, 15 demo_goal_2,
  16 demo_bomb_get; intro pick: 291==0 ? (!bit720 ? 3 : 0xA) : (!bit1A04 ? 0x12 :
  0x14)); cutSetAnmProc **100%** (isStop→cutEnd); setAttention **100%**:
  `bool morfing = mpMorf->isMorf();` (morf+0x74 mCurMorf<1.0 — NOT getFrame/rate;
  donor McaMorf mFrameCtrl@0x58) + **cXyz.set(x,y,z) spelling for both eyePos and
  attention_info.position** (3-loads-then-3-stores; plain operator= interleaves);
  y += child.field_0x18.
- next_msgStatus **100%**, chkAttention 99.83% (§2b add-order), getMsg/talk/moccowait
  covered in batch 11-12 notes.
- RUNNING: ~70 written. Remaining tail: setAnm, lookBack, _execute, goal to/from
  pair, create/heap/init/ctors, cutProc, cutRideSwitchProc, cutJump*/Lift/Rope pairs,
  searchNear*, nodeCallBack, drawShadow (needs g_regHIO idiom).

## Batch 14 (07:20Z) — setAnm

- setAnm **98.7%** (2 rows: fopAcM_GetID null-check fold inside fopAcM_monsSeStart
  inline — target keeps runtime check, mine folds; semantics identical, revisit at
  final pass). Shape: five function-local statics — a_anm_num_tbl s8[3][24],
  a_play_mode_tbl int[24], a_morf_frame_tbl f32[24] (REFRESHED each call from
  l_HIO.children[mType].field_0x6c[24]!), a_play_speed_tbl f32[24], a_anm_bck_tbl
  int[28] (values in cpp). Flow: if 7d4!=7d3 && anm_num[mType][7d3]!=-1 → 7d2=anm,
  field_0x374=0, speed = play_speed[7d3] (or getMoveSpeed()*0.25f when
  switch(getNowCut()){case 2,4}), dNpc_setAnm(mpMorf, play_mode[7d3],
  morf_frame[7d3], speed, bck[7d2], -1, m_arc_name). Then: frame==1&&7d2==0x12 →
  fopAcM_monsSeStart(this,0x4897,0); 7d4=7d3; isStop-driven transitions 0x13→3,
  7→1, (type0)0x17→0xD. Members: 0x7D2 s8 cur-anm, 0x7D4 s8 prev-action, 0x374 f32;
  mEventCut.getNowCut()/getMoveSpeed() accessors; childHIO f32[24]@0x6C.
- WIP committed locally: 7e7e727f (70 fns). Remaining: lookBack, _execute, drawShadow
  (g_regHIO fields), goal to/from pair, create family+ctors, cutProc, ride/jump/lift/
  rope cut pairs, searchNear*, nodeCallBack.

## Batch 15 (07:33Z) — _execute/drawShadow/cutRideSwitchProc

- _execute **99.25%** (one redundant-branch row): m_jnt.setParam(9 childHIO s16s
  mMax/Min Head/Backbone X/Y + mMaxTurnStep @child 0x04-0x14); type!=2 playTexPattern;
  ChkGroundFind→GetMtrlSndId(mObjAcch.m_gnd); mpMorf->play(&eyePos, mtrlSnd,
  getReverb(current.roomNo))→field_0x370 (+frame-wrap force 1); checkOrder;
  (this->*mActionFunc)(NULL) @0x7C4 PTMF; runCheck && mCommand!=1 →
  mEventCut.cutProc() ? {72c=0; if getNowCut()!=-1 switch{2,4: speed logic}else 7d3=1}
  : {setAttnFlag(0); cutProc();}; !runCheck → clear NoTurn/Attn/72c; eventOrder;
  setAnm; setTexAnm; posMoveF(&field_0x580); mObjAcch.CrrPos; tevStr room/color;
  setMtx; morf calc; himo hang (field_0x7c0/0x7bc himo3, getAnmMtx(0xC)→field_0x78c,
  setActorHang(field_0x78c, shape_angle.y) DIRECT member pass); setCollision;
  7d3==4 rate = clamp01(((current.pos-old.pos).abs())/(1.75f+REG12_F(6))) *
  children[2].field_0xcc, min children[2].field_0xd0 → setPlaySpeed.
  **LAYOUT FIX: dBgS_ObjAcch = 0x1C4 (0x37C..0x540)**. m_jnt = dNpc_JntCtrl_c @0x2CC
  (0x34, fills to 0x300; old 2ce/2d2/2d7/2d8 = mAngles[0][1]/[1][1]/mbHeadLock/
  mbBackBoneLock).
- drawShadow **99.94%** 0-structural: pos(x, 150.0f+y+REG8_F(18), z); setShadow(id,1,
  morf model, &pos, 800.0f+REG8_F(19), 20.0f, y, GetGroundH(), m_gnd, &tevStr);
  addRealShadow head + (80a ? 2bc : 2c0).
- cutRideSwitchProc **98.35% 0-structural** (regalloc rows): Swpush judge,
  searchActorAngleY, `const cXyz& delta = targetPos - current.pos;` (**REF-BINDING
  avoids temp copy**), horiz EXPLICIT per-component stores (pairwise!), horiz.abs()
  unused, addCalc2 x/z →0.1f/24.0f, ChkGroundLanding→{7f4=0x11,7d3=0xA},
  timer-done && 0xA → speed.y=0, gravity=-9.0f, 7d3=1, cutEnd.
- REMAINING: lookBack, goal to/from pair, create family+ctors, cutProc, cutJump/
  JumpToLift/JumpToGoal/LiftToRope/RopeTalk/RopeToLift pairs, searchNear*, nodeCallBack.

## Batch 16 (07:41Z) — cut pairs + create + cutProc + searchers

- ALL 0-structural: cutJumpStart 99.71 / cutJumpProc 99.90 / cutJumpToLiftStart 99.68 /
  cutLiftToRopeStart 99.68 / cutRopeToLiftStart 99.72 / cutJumpToGoalStart 99.65 /
  _create 99.78 / cutProc 99.97 / searchNearLift+Rope 99.93 (statics, void* return,
  Hlift_e / HIMO3_e; members 0x714 cXyz best-pos, 0x720 fopAc* best).
- Substance-start idiom: getMyFloatP Speed/Speed_y/Gravity (+getMyVec3dP Pos →
  field_0x7e8) defaults 1.75/15/-2.5 (RopeToLift: 6.0/-0.5, zeroes angle.x/z, applies
  speeds immediately, 7d3=9; Jump applies + 7d3=8 + monsSeStart 0x4898).
- cutJumpProc: jump phys — ChkGroundHit &&(s8)7d3∈{9,A}→7dc.set(0,-100,0),7d3=A;
  speed.y<=0&&7d3==8→7dc.set(0,-50,0),7d3=9; ChkGroundLanding→monsSe 0x489B,7f4=0x11,
  speedF=0,smoke_set() else field_0x754.remove(); 774/780=current; timer&&A→zeros,
  gravity=-9,cutEnd,7d3=1.
- _create: fopAcM_SetupActor(this, daNpc_P2_c) — implicit ctor inline MATCHED (validates
  full member layout!); resLoad; COMPLEATE→getArg, entrySolidHeap(CreateHeap_CB,
  m_heapsize[mType]) else ERROR; createInit. CreateHeap_CB static passthrough.
- cutProc: getMyStaffId("P2b"); getMyActIdx(staff, action_table[15] TALK..OMAMORI_END,
  0xF, TRUE, 0); -1→7d7=0+cutEnd; else 7d7=1, getIsAddvance→72c=0+Start-switch;
  always Proc-switch. Member field_0x7d7 u8.
- MEMBERS FINAL-ish: mStts@0x580 (dCcD_Stts 0x3C, GetCCMoveP→posMoveF!), mAcchCir@0x540
  (0x40), field_0x7e8 cXyz jump target, 7f8/7fc/800 f32 speedY/speedF/grav params.
- REMAINING: lookBack, goal to/from pair, createInit, _createHeap, HIO ctors,
  nodeCallBack, cutJumpToLiftProc, cutLiftToRopeProc, cutRopeTalk pair, cutRopeToLiftProc,
  cutJumpToGoalProc.

## Batch 17 (08:07Z) — THE LAYOUT REVELATION + ctors + big-proc sweep

- **daNpc_P2_HIO_c : mDoHIO_entry_c with children[3] @ +0x4** (NOT JOR/mNo/m8@0xC!).
  childHIO: vt@0, dNpc_HIO_c mBase@0x04 (m04 f32@0x8 = playerEyePos y-offset —
  resolves the 'vptr as float' paradox; jnt maxima/minima @0xC-0x1E; mAttnYOffset@
  0x20=50; mMaxAttnAngleY@0x24=0x4000; mMaxAttnDistXZ@0x28=200), u8@0x2C, goal cXyz
  @0x30 (0,-550,-3020), goal2 cXyz@0x3C (0,-550,-3400), demo cXyz@0x48 (5,-545,30),
  f32 0x54(80)/0x58(180)/0x5C(200)/0x60(300)/0x64(30)/0x68(300), u8@0x6C, f32@0x70
  (5000), f32[24]@0x74 = morf-frame defaults {0,14,8,8,4,0,4,8,6,6,2,20,20,8,12,8,
  4,8,8,8,8,8,15,8}. HIO_c tail: f32 0x280=3.0 (run-rate scale), 0x284=0.9 (min).
  ALL prior child field names were byte-identical but shifted -8; refactored.
- Both ctors WRITTEN: childHIO 99.72 / HIO_c 99.94, 0-structural. HIO ctor overrides
  per-child jnt tables (values in cpp). TU fuzzy 39% → **82.06%**.
- nodeCallBack 98.69 0-str (accessor-arg eval order: mDoMtx_stack_c::XrotM(
  m_jnt.getHead_y()) — accessor call as arg loads angle BEFORE now-address;
  raw member arg loads after. REG12_S(0..2)/REG12_F(6)/REG8_F(18,19) regHIO).
- _createHeap 98.07 (1.0f pool rows only): res 0x26 BDL + morf (NULL anm, loop -1,
  0x15021222), head bdl tbl {2D,2E,2F} (type2: flags 0/0x11020203), dagger 0x2A,
  sheath 0x27 (assert bug: asserts dagger ptr — reproduced), type2 pot morf res
  0x29+bck 0x23, type0 telescope 0x28; setUserArea(this); joints 2&4 →
  setCallBack(nodeCallBack). **LESSON: hidden vbase-flag arg r4=1 in McaMorf ctor
  (bm1-attested); CreateHeap_CB must precede _createHeap in file (else MWCC
  sibling-tail-calls it)**.
- cutJumpToLiftProc/LiftToRope/RopeToLift/JumpToGoal procs + goal to/from movers:
  ALL 99.2-99.96, 0-structural. lookBack 93.5 equiv-parked (NoTurn boolify
  placement — needs u8-returning accessor upstream).
- REMAINING: createInit (0x738), cutRopeTalkStart (0x550), cutRopeTalkProc (0x80C).

## Batch 18 (08:14Z) — createInit + cutRopeTalkStart WIP

- createInit **99.69%** (1 equiv row: mType reload before type2 ship-check): Asoko
  strcmp → fopAcM_OnCondition(0x4000); morf calc; cullMtx = baseTRMtx; mAcchCir.
  SetWall(30,0); mObjAcch.Set(&current.pos,&old.pos,this,1,&mAcchCir,&speed,
  &current.angle,&shape_angle); distances 0xA9; type1 spawn-pos from children
  field_0x30/0x3c by event bits; field_0x6ec csXyz = current.angle; attention_info.
  flags=0xA; gravity=-9; mEventCut.setActorInfo({"P2a","P2b","P2c"}[mType], this);
  setAction(intro/wait by Asoko+bits — **setAction INLINE in header: cmp ptmf,
  old→(-1 call), swap, 808=0, call**); 704/6f8=pos; mStts.Init(0xFF,0xFF,this);
  mCyl.Set(dNpc_cyl_src); mCyl.SetStts(&mStts); 7d1=7d3=1; 7d4=0; setTexAnm; setAnm;
  ship (fpcNm_Obj_Pirateship_e via parentActorID) → field_0x748 = home.angle.y -
  ship->home.angle.y (+jnt locks type0); 74c=(s16)(200+rndF(100)); 7dc=0.
- cutRopeTalkStart WIP **94.4%** (18 structural): rope-pendulum setup — MsgNum/
  Attention, 7d3=0xC, talkInit, 7c0=1, regPos REG12_F(2..4) (unused local),
  searchNearRope → 7bc; d=rope-pos delta (LOCAL copy); 798=d.abs(); 79c=
  sqrtf(2/798); quarter=(pi/2)/79c; 7a8=0x2EE0; rot = yaw-rotate d by cM_ssin/scos
  (current.angle.y); pitch=cM_atan2s(-rot.z, rot.y) clamp ±7a8; 7a0=cM_atan2f(ratio,
  sqrt(1-r²))/79c; 7aa=cM_atan2s(-rot.x, sqrt(rot.y²+rot.z²)); 7a4=±quarter by 7aa
  sign; reposition: d2 ref-bound delta, len=d2.abs()-80 clamped to himo->m15FC,
  n=d2.normalize(), current.pos = ropePos - n*len; speed.y=gravity=0; 7b0=ropePos;
  7ac=0. REMAINING ROWS: local-copy vs direct placement around regPos/rot builds +
  clamp shape. Members 0x798-0x7AC filled (f32 len, freq, phase, dir, s16 amp,
  s16 side, s32 timer).
- LAST FUNCTION: cutRopeTalkProc (0x80C — pendulum sim). Then: TU-wide report,
  configure flip to Matching when clean, cross-version, PR staging.

## MILESTONE (08:18Z): TU FULLY DECODED — fuzzy 99.40%

Every function written and measured. cutRopeTalkProc (0x80C pendulum sim, the
biggest) landed 99.47% FIRST TRY: fopAcM_SetParam(&rope->actor, 2/3) himo mode,
hand pos via getAnmMtx(0x14)+dLib_getPosFromMtx, world→local via ZrotS/XrotM/YrotM
(neg angles)+transM(neg pos), pendulum accel ±0.45*cM_fsin gated ±pi/2, phase +=
1+accel, 2pi wrap, chaseS(7aa,0,0x20), amplitude decay 7a8 += 64*|cM_fcos|,
clamp 0x2EE0, addCalcAngleS on angle.x (target -amp*fsin(swing-0.628), 8, 0xC00,
0x100) and angle.z (7aa*fsin(swingB-1.0996), 8, 1024+2048*cosB, 128+128*cosB),
reposition via transS(7b0)+ZXYrotM+multVec(0,-798,0), so-fx 0x201E (mDoAud_seStart
2-arg) + 0x489A on exit, exit talk(true)==0x12 && swings>0 && 7d3==0xB → 7d3=9,
7c0=0, param 3, cutEnd; else quadrant-based 7d3=0xB/0xC.

SUB-100 RESIDUE MAP (ALL semantically complete — §2b Equivalent):
- Data ordering drives most: @stringBase0 53%, .rodata 50%, .data 55% — my pool
  emission order vs target's; fix = align literal/static first-use order.
  Casualties: drawP2b 79.7 / drawP2c 86.3 / initTexPatternAnm 87.9 (m_heapsize
  pool-anchor collapse pending), ~40 fns at 99.7-99.9 with single reloc rows.
- Regalloc/equiv-class: lookBack 96.8 (NoTurn boolify placement), cutRideSwitchProc
  98.35, cutRopeTalkStart 98.75 (3 slot rows), _execute 99.25 (1 branch row),
  chkAttention 99.83 (add operand order), anmAtr 99.15 (static mangle numbers).
- __vt__14daNpc_P2_HIO_c 71% — vtable content (genMessage/dtor order) — check
  virtual declaration order vs mDoHIO_entry_c.
- g_profile 93% — profile struct init (not yet written? check g_profile_NPC_P2 —
  likely needs the profile definition block in cpp).

NEXT STEPS (fresh context): (1) g_profile_NPC_P2 block + dtors if missing;
(2) vtable order fix; (3) pool forensics: dump my .rodata/.data/@stringBase0
symbol order vs target asm order, reorder static defs/first-use to converge;
(4) full-REL SHA gate → flip configure Matching → 12/12 cross-version;
(5) PR staging per retroactive-correction rule.

## Batch 19 (08:23Z) — pool forensics round 1 (fuzzy 99.56%)

- __vt__14daNpc_P2_HIO_c → **100%** (dropped my genMessage override — target vt
  is {0,0,dtor} only).
- **POOL LESSON: class statics MUST be `const` to land in .rodata** (m_heapsize/
  m_arc_name now at 0x0/0xC matching target; non-const versions had been polluting
  .data). l_btp/l_bmt const + moved before nodeCallBack (target slot 0x74/0x7C).
- nodeCallBack attn_pos_default y = **10.0f** (rodata 0x88), NOT 1.75f — misread
  fixed (extra literal removed).
- My rodata literals 0x10-0x70 now EXACTLY match target values/order.
  Remaining pool deltas: 10.0f slot (mine 0x74 pre-tables, target 0x88 post-tables
  — nodeCallBack static-ctor literal emission order question), plus @stringBase0
  内 ordering (53%) and .data (55%) — drives the ~40 one-row reloc functions.
- NEXT: string-pool order diff (compare my @stringBase0 bytes vs target string
  sequence); .data order (a_anm tbls, action_table, jumptables, ptmf objs, demo
  name tbl — reorder local-static DEFINITIONS if needed); then g_profile last-word
  check, REL SHA gate, configure flip, cross-version.

## Pool forensics round 2 notes (08:24Z)

MY @stringBase0 order: <sjis> | filename | head_tex assert | Halt | P2B_* demo
names | Hlift_up | DEFAULT_TREASURE | createHeap asserts | P2a/P2b/P2c | Asoko |
action names | substances | anim names.
TARGET order (by offset): 0x0F head_tex assert cond | 0x25 filename | ...unknown
0x2E-0x95... | 0x95 SW_OFF | 0x9C SURPRISE | 0xA5 OMAMORI_INIT | 0xB2 OMAMORI_END |
0xBE P2b | 0xC2 MsgNum..Name | 0xF8 KYORO/THINK/NOD/WAIT01 | 0x10F demo names |
0x176 Hlift_up | 0x17F DEFAULT_TREASURE | ~0x190 createHeap asserts | 0x1FA Asoko |
0x1FE action tbl strings | 0x292 P2b(2) | 0x2E3 demo tbl strings | 0x3C6 P2a/P2c.
→ Target emission order equals FILE ORDER of the ORIGINAL: the 0x2E-0x95 gap +
SW_OFF-cluster-first suggests string-literal uses inside data-table INITIALIZERS
(local statics in cutProc/eventOrder etc.) are emitted at their FUNCTION's compile
position, and code-literals at theirs. The unknown 0x2E-0x95 region likely holds
more assert/JUT strings (get exact with SJIS-aware parse of the target asm).
Also: my stray "Halt" string — check which of my macros emits it (JUT_ASSERT
variant?) — target may use a different assert macro spelling for some asserts.
Approach next round: byte-parse target stringBase fully (SJIS-aware), list
(offset→string→emitting construct), then reorder my constructs (mostly: which
JUT_ASSERT macro + where local-static tables sit) until pool bytes match.

## Pool forensics round 3 (08:25Z) — string-pool ORDER rule unknown

- FULL target string map (offset order): d_a_npc_p2.cpp, head_tex_pattern != 0,
  Halt, TALK..OMAMORI_END (action names), P2b, MsgNum, Attention, Speed_y, Gravity,
  Timer, Speed, Pos, Name, KYORO, THINK, NOD, WAIT01, P2B_INTRO..P2B_BOMB_GET,
  Hlift_up, DEFAULT_TREASURE, modelData != 0, headModelData != 0, daggerModelData
  != 0, bookModelData != 0, telescopeModelData != 0, P2a, P2c, Asoko, TALK..(2nd
  action cluster for action_table data), P2b(2), ...demo tbl strings..., P2a/P2c(2).
- Renamed createHeap locals to ORIGINAL names (from assert strings): headModelData,
  daggerModelData, sheathModelData(0x27 — unnamed in asserts), bookModelData (type2
  = Mako's BOOK morf), telescopeModelData. Committed.
- OPEN: pool emission ORDER doesn't follow function/address order (cutSetAnmStart's
  KYORO precedes demo fns' strings; cutProc's TALK cluster precedes everything).
  NOT lexical order either (MsgNum before demo names contradicts). NEXT: derive the
  rule empirically from d_a_npc_bm1 (byte-matched, source visible) — compare its
  source construct order vs its @stringBase0 offsets; apply rule to p2 (likely
  requires specific placement of the local-static tables or JUT string macros).
- My pool also has 3 stray leading entries (<sjis:1>, 2 bytes) before filename —
  identify emitter (possibly SJIS comment/pragma or a mDoHIO string) and remove.
- Current: TU fuzzy 99.5578; all functions semantically complete; only data-order
  + ~1-row-per-fn reloc/regalloc residues remain. (A)-axis REL SHA still pending
  pool convergence.

## Pool forensics round 4 (08:27Z) — RULES DERIVED, experiment queued

- **bm1 ground truth: @stringBase0 order = SOURCE-LINE order of first appearance**
  (verified: pool[0..4] ↔ source lines 504/505/707/855/856).
- p2 .text address order == CLASS DECLARATION order (verified across 16 methods) —
  original file defines methods in class-decl order (as does the skeleton ✓).
- PUZZLE: $-static numbers (smoke$4581 < anm_atr$4702 < action_table$4763 <
  a_demo$6013) imply cutProc parsed before eventOrder, but addresses (0x5240 vs
  0x1918) and decl order say opposite. And target pool puts cutProc/cutSetAnmStart
  strings (TALK.., KYORO..) BEFORE demo fns' strings despite later addresses.
  HYPOTHESIS to test: MWCC pools strings/announces statics during a PRE-PASS over
  ALL function bodies in CLASS-DECL order?? or template/inline processing order.
  **EXPERIMENT (next context): tiny test TU on the donor compiler — two functions
  with distinct strings/local-static tables, permute definition order vs class-decl
  order, observe pool + $-numbers.** That decides the mechanical fix (likely:
  reorder MY function DEFINITIONS to match the $-number-implied parse order while
  .text emission follows decl order — i.e. MWCC emits .text by CLASS DECL, pools
  by DEFINITION ORDER — so the ORIGINAL file's DEFINITION order was: ..., smoke_set,
  ..., anmAtr, cutProc+cutStarts, ..., eventOrder..., i.e. NOT decl order!
  If so: reorder my cpp definitions to the $-number sequence; .text stays put.)
- Also: strip my 3 stray leading pool entries (<sjis:1> + 2) — find emitter.
- All decode semantics complete; asserts byte-fixed; fuzzy 99.5578.

## Pool forensics round 5 (08:28Z) — EXPERIMENT RESULTS

- **PROVEN on donor compiler: .text emission = DEFINITION order** (moved talkInit
  def to top → .text 0x0). Combined with target .text == class-decl order ⇒ the
  ORIGINAL file defined methods in class-decl order (same as skeleton ✓ keep).
- **Pool structure refined**: target @stringBase0 = [code-referenced strings in
  lexical order 0x0-0x1FD] ++ [data-initializer strings per table: action_table
  @0x1FE, staff-name? @0x292, a_demo_name @0x2E3, actor_name @0x3C6]. Code and
  data-initializer strings DO NOT dedup against each other (P2B_INTRO & TALK
  clusters both doubled ✓).
- REMAINING PUZZLE: why TALK..OMAMORI_END + P2b + substances + anim names appear
  in the CODE region (0x2A-0x10E) before demo strings — implies the original had
  these as CODE-adjacent literals textually early. Candidates: the original
  cutProc/cut*Start bodies may have been defined EARLY as static functions or the
  strings appeared in early declarations. NEXT experiment (fresh context): check
  whether MWCC pools a function-local static ARRAY-OF-CHAR-PTR initializer's
  strings into the CODE region at the function's parse position when the array is
  const vs non-const (my action_table is non-const char* — try const char* const,
  observe which pool region strings land in). Also test $-number assignment.
- Experiment reverted; tree clean at fuzzy 99.5578.

## Pool forensics round 6 (08:35Z) — DUPLICATION THEORY RETRACTED

- **Target @stringBase0 is 0x200 bytes total — there are NO duplicate strings.**
  The .L_000001FE-style labels in data-table relocs are RODATA-ABSOLUTE addresses
  (pool starts ~0x1D4 in target rodata), so .L_1FE = pool+0x2A = "TALK" etc. The
  action/demo/actor-name tables all point INTO the one deduped pool. (Same
  label-vs-offset trap as the 0x2E3 case early in the campaign — bank it twice.)
- Donor-compiler experiments (t1-t11, scripts in scratchpad/pooltest): pool =
  first-occurrence order with FULL dedup incl. stripped tables and across
  code/table uses; plain `-str reuse` (no pool) puts strings unpooled in .data;
  class-inline bodies pool at expansion site; stripped file-scope tables leave
  their strings pooled (t4).
- SURVIVING PARADOX: target pool order = asserts, TALK..OMAMORI_END, P2b,
  substances, anim names, P2B demo names, Hlift/DEFAULT_TREASURE, createHeap
  asserts, P2a/P2c, Asoko — which matches NO ordering of the functions (decl,
  definition, or address; all three proven equal for this TU, and demo fns
  precede cutProc in all of them while their strings come AFTER cutProc's).
  Candidate mechanisms to test next: (a) MWCC pools data-table initializer
  strings at the TABLE's PARSE while pooled CODE literals at first CODE-GEN —
  giving tables-early order if tables parse early (locals parse with body...);
  (b) a stripped early construct carrying the exact sequence TALK..WAIT01
  (t4-mechanism) with live code deduping into it — my reconstruction would add
  a stripped file-scope table with those 14+4 strings between initTexPatternAnm
  and demo_wait_2 and re-measure; try (b) FIRST — it is cheap and, if pool bytes
  then match target, correctness is settled regardless of which story is true.
- My pool currently starts with 3 stray entries + demo names early; after (b)
  re-dump my pool and iterate to byte equality; then .data order (jumptables/
  ptmfs/tables), then REL SHA.

## Pool forensics round 6 RESULT (08:55Z) — STRING POOL BYTE-EXACT

- **@stringBase0 now matches target 512/512 bytes, all 49 entries at identical
  offsets.** Fix: the cut family (cutProc + all cut*Start/Proc + searchNearLift/
  Rope) belongs BETWEEN talk() and eventOrder() as PLAIN definitions (no inline
  keyword). Committed b56423e3. Exact fns 94 -> 115/145; TU fuzzy 99.5636; zero
  fns below fuzzy-100.
- Evidence chain: pool label .L_NNN = rodata-ABSOLUTE (pool base 0x1D4);
  local-static counters are parse-order fingerprints (target: a_anm_bck_tbl$4307
  setAnm < anm_atr$4702 < action_table$4763 cutProc < a_demo_name_tbl$6013
  eventOrder < head_bdl_tbl$6964 createHeap < actor_name_table$7073 _create) —
  action_table only 61 counters after anm_atr proves cutProc parsed right after
  talk. actor_name_table = {P2a@new, P2b@deduped-into-cutProc-copy, P2c@new}.
- NEW MWCC LESSONS (all donor-proven this round):
  * inline member fns (keyword on def OR decl): parse/counters/codegen DEFERRED
    to first demand; demanded instantiation is DEPTH-FIRST (callee strings pool
    before caller's own literals — t13c). #pragma dont_inline does not change
    this. Deferred bodies do not consume $ counters at file position.
  * Demanded header-inlines emit INTERLEAVED at the demand site in .text
    (dCcD dtors sit between _create and _delete in target region A).
  * Post-__sinit "region B" (.fn 106-144 in target: dNpc_HIO dtor, getAttnPos,
    cut family in definition order) = emissions pending at EOF.
- OPEN (the one remaining .text question): what made the ORIGINAL cut family
  codegen at its definition (pool+counters mid-file ✓) yet emit .text in
  region B after __sinit? Falsified: plain inline✗ (defers pool), decl-side
  inline✗ (t12e), #pragma dont_inline✗ (t13b), virtual✗ (no vtable relocs to
  family). Candidates for next context: CW weak-section placement quirk under
  a pragma (#pragma internal/#pragma always_import?), or demand pending from a
  stripped construct parsed pre-family + EOF flush semantics. Current state
  (plain defs mid-file) puts family .text mid-file = SHA blocker but objdiff-
  clean; only .text order + .data order (matched_data 0.1%, whole-section
  shift) separate us from the REL gate.
- NEXT: (1) enumerate target .data object order vs mine (same forensic script
  pattern), (2) region-B mechanism experiment matrix, (3) then SHA gate ->
  configure flip -> 12/12 cross-version -> PR staging.

## Round 7 (09:20Z) — REL-level convergence campaign (post re-arm)

- **REL .data section BYTE-IDENTICAL to retail (0x580).** Weak localstatics
  (std::sqrtf _half/_three) exist in my ELF but are dropped by the REL link —
  retail lacking them proves nothing about source usage. Pool still exact.
- **Cut family now in src/d/actor/d_a_npc_p2_cut.inc** (#include between
  chkAttention and eventOrder, tc idiom, commit 9395262e): MWCC -sym on emits
  one .text section per source file — my unique-2 text section is 0x31C8 =
  target EXACT. My .o has 2 spurious extra text sections (weak strays).
- **CRITICAL LESSON — NonMatching rels link the asm objects**: the earlier
  "SHA-identical rel" under NonMatching was vacuous. True grading = flip
  Matching (staged in configure.py) and diff build/GZLE01/d_a_npc_p2/*.rel vs
  Yaz0-decompressed orig/GZLE01/files/rels/d_a_npc_p2.rel.
- _execute fixes landed (99.90): ChkGroundHit not ChkGroundFind (mask 0x20);
  the mystery constant is 10.0f at rodata 0x88 not 1.75f (both sites); himo
  region needs TWO named locals (J3DModel* model, then himo3_class* himo,
  decls before dLib_getPosFromMtx call); setPlaySpeed clamp is the ternary
  `rate < l_HIO.field_0x284 ? l_HIO.field_0x284 : rate` (plain bge shape —
  float `<` avoids cror). Residue: 5-row r28/r29 CSE-temp regswap (dComIfG
  +0x12a0 pointer; named-local steering fails — emits mr; decl-order swap
  no-op; park §2b unless allocator knob found).
- sqrt idiom map (donor-proven): std::sqrtf = 3-iter + static double _half/
  _three (weak .data, link-dropped); plain sqrt(double) = 4-iter + LITERAL
  0.5/3.0 rodata doubles (target @4896/@4897). p2 target uses std::sqrtf
  everywhere EXCEPT _execute run-rate = sqrt((...).abs2()). cX.abs() = correct
  spelling for all other length computations.
- .data object map fully aligned: anim tables NON-const in .data in decl order
  num,mode,morf,speed (a_anm_bck_tbl CONST FIRST — counter 4307<4308); anm_atr
  + head_bdl CONST; head_tex_tbl is s8[2]={0,1} (size 2!); six setAction ptmf
  constants emit per-site naturally (@7082-@7101 = createInit's six sites).
- REMAINING (size deltas): setAnm -4 (fmuls operand swap f1*f0 vs f0*f1 +
  branch polarity beq+b vs bne at row 80 + r0/r3 extsb swap rows 25-28),
  createInit -4, cutRopeTalkStart +4, cutRopeTalkProc -4, lookBack +28
  (parked NoTurn boolify), _execute +56 (sqrt-variant length — target's rate
  sqrt may be 3-iter float-with-literals?? investigate __frsqrte spelling),
  8 extra weak fns (2 daPy stubs + 6 dtors — outstanding trigger mystery;
  bdk/himo2 emit stubs too and MATCH, so likely REL-link-dropped: CHECK
  whether rel text still oversized before hunting sources).

## Round 8 (09:45Z) — REL within 24 BYTES of retail

- **REL sections: .rodata 0x3D4 EXACT, .data 0x580 EXACT, text +0x18.** Total
  file delta 24 bytes = createInit(-4) + cutRopeTalkStart(+4) +
  cutRopeTalkProc(-4) + lookBack(+28). Fix those four sizes -> SHA gate.
- **tc precedent PROVEN: d_a_npc_tc.rel builds SHA-IDENTICAL with std::sqrtf
  _half/_three statics in its ELF** — the REL link reconciles statics vs
  retail literals AND drops all 8 spurious weak fns (2 daPy stubs, 6 dtors).
  DO NOT hunt those at source level. Use cX.abs()/std::sqrtf everywhere
  (including _execute's run-rate — sqrt(double) emits a NaN/HUGE else-chain
  +56 the target lacks).
- setAnm 99.875 size-exact: (a) the getNowCut switch has an EMPTY `case 3:
  break;` (useless-case-3 pattern, second sighting); (b) monsSeStart guarded
  by SINGLE-CASE switch `switch((int)field_0x7d2){case 0x12:...}` (beq+b
  shape); (c) NEW LESSON: `speed = getMoveSpeed(); speed *= 0.25f;` gives
  fmuls f2,f1,f0 (call-result-first) where `a*b` spellings canonicalize the
  other way. Parked §2b: 4-row extsb r0/r3 permutation at the 7d4!=7d3
  compare (6 spellings tried incl. u8/s8 header flips — byte-permutation,
  size-identical).
- _execute 99.90 size... rechecked +56 was sqrt-variant — reverted to .abs();
  parked §2b: 5-row r28/r29 CSE regswap (dComIfG+0x12a0).
- NEXT (SHA path): diff createInit (-4: one branch-shape row likely),
  cutRopeTalkStart (+4), cutRopeTalkProc (-4), lookBack (+28: NoTurn boolify
  — REVISIT: maybe an in-TU accessor respelling avoids the upstream u8 need).
  Then re-gate REL diff; then the byte-permutation parks matter (SHA needs
  bytes): setAnm extsb rows + _execute r28/r29 — try function-level knobs
  (e.g. changing OTHER locals' lifetimes upstream of the allocator).
- Matching flip staged in configure.py (uncommitted for gate discipline...
  actually committed — REVERT to NonMatching before any push if gate unmet).

## Round 9 (10:20Z) — lookBack/createInit 100%; sqrtf VINTAGE is the final boss

- **lookBack 100.0%** — three fixes: (a) `cXyz target;` declared BEFORE
  basePos (stack order); (b) NoTurn boolify killed by `bool mbNoTurn` MEMBER
  type in d_npc.h (NOT accessor/param changes — lookAtTarget's DOL mangling
  `...ssb` pins the bool param; only a bool member gives raw lbz + raw pass).
  tc canary re-verified SHA-identical after the header change.
- **createInit 100.0%** — fopAcM_OnStatus (0x1c4) not OnCondition (0x1c8);
  ship-angle block is `if (mType==0 && !isEventBit(0x808)) {...} else if
  (mType==2)` (&&-flattened, bit-set path falls INTO the else-if).
- _execute size-exact: `std::sqrtf((current.pos - old.pos).abs2())` — abs2
  compiles to bl PSVECSquareMag; NEW LESSONS: single-case switch => beq+b;
  `speed = call(); speed *= 0.25f;` => fmuls call-first operand order.
- REMAINING BLOCKERS (SHA):
  1. cutRopeTalkStart +4 / cutRopeTalkProc -4 (17/50 rows; RopeTalkStart's
     second operator- temp: target reuses slots via ANOTHER named-r29-himo
     shape in the tail — rows show `lwz r29,0x7bc` named + `addi r3,r1,0x40`
     AFTER; my tail spells the ptr inline. Also target 1st temp @0x4c vs mine
     @0x40.)
  2. setAnm 4-row extsb r0/r3 permutation; _execute 5-row r28/r29 CSE
     regswap (parked; 6+ spellings each).
  3. **sqrtf vintage**: retail p2's std::sqrtf has const-LITERAL doubles
     (rodata @4896/@4897 0.5/3.0) + 1.75f overlap-pools into the 0.5-double
     prefix at 0x138; pi/2 sits at 0x160. WWDP's dolzel_rel.mch FREEZES the
     statics form (per-TU -D can't override a precompiled header;
     textual-pch bypass diverges everywhere else). NEXT CONTEXT: add a
     configure.py precompiled-header VARIANT (dolzel_rel_lit.mch compiled
     with -DSQRTF_CONST_LITERALS) and point p2's dolzel_rel include at it —
     math.h already carries the inert SQRTF_CONST_LITERALS branch (commit
     3ebe3650). Note: REL byte-attribution vs retail is confounded by reloc
     placeholder conventions — trust only SHA + .o objdiff.
- configure.py: p2 REVERTED to NonMatching pending gate (retail-asm object
  links meanwhile; earlier 'SHA-identical' under NonMatching is vacuous).

## Round 10 (10:35Z) — RopeTalkStart at pure-permutation; stretch checkpoint

- cutRopeTalkStart 99.29, rows 340=340: tail spelled `himo3_class* himo =
  field_0x7bc; d = himo->actor.current.pos - ...` (member access through
  the derived pointer — casts created a CSE'd &pos temp the target lacks).
  Residue: 3 cXyz temp slots ROTATED (target 0x4c,0x40,0x34 creation order
  vs mine 0x40,0x34,0x4c — my n*len temp allocated first somehow) + pi/2
  rodata shift (sqrtf vintage dependent). cutRopeTalkProc -4 unexamined this
  round (r27/f31 naming + one-instruction delta, likely same himo-naming fix
  needed there — target names r27=himo mid-function).
- WW DP path bug recurs after EVERY configure.py run: build.ninja gets the
  long name "D:\XXXXXXX\WW DP" (folder's real name; WWDP is the 8.3 alias)
  which breaks mch rules — ALWAYS run: python -c "t=open('build.ninja',
  encoding='utf-8').read();open('build.ninja','w',encoding='utf-8').write(
  t.replace('WW DP','WWDP'))" after configure.
- SHA path remaining: (1) RopeTalk pair slot/naming residues; (2) setAnm
  extsb + _execute r28/r29 permutations; (3) sqrtf PCH variant (the decisive
  structural blocker — plan in round 9); (4) re-gate, cross-version, PR.

## Round 11 (10:55Z) — FINAL STATE OF THIS CONTEXT (successor starts here)

- **143/145 functions size-exact.** Only cutRopeTalkStart (-8) and
  cutRopeTalkProc (-4) differ in size. RopeTalkStart is BRACKETED: with
  ((fopAc_ac_c*)himo) casts on the three tail pos-uses it is target+4
  (cast-CSE temp too sticky); with himo->actor.current.pos direct it is
  target-8 (CSE too good). The original MIXES spellings — try cast on ONE
  or TWO of {d=, current.pos=, field_0x7b0=} and direct on the rest; six
  combinations, brute-force (build + symtab size check each).
  RopeTalkProc likely inherits the fix (residue r27/r28 + f30/f31 +
  pi/2-shift rows only).
- Four PERMUTATION parks (size-neutral, byte-different): setAnm extsb
  r0/r3 (4 rows), _execute r28/r29 CSE (5 rows), RopeTalkStart cXyz temp
  slot rotation, RopeTalkProc reg pair. Untried knobs: reorder UNRELATED
  earlier locals; split/join statements around CSE births. Fix the sqrtf
  vintage FIRST — it may shift allocator state and move these.
- **sqrtf vintage (THE structural blocker)**: build a PCH VARIANT —
  add to configure.py config.precompiled_headers a second entry compiling
  d/dolzel_rel.pch with -DSQRTF_CONST_LITERALS into dolzel_rel_lit.mch,
  plus header d/dolzel_rel_lit.h including THAT mch; p2's cpp includes it
  instead of d/dolzel_rel.h. math.h's SQRTF_CONST_LITERALS branch exists
  (commit 3ebe3650). DO NOT bypass the mch textually (proven divergent).
  Success: _half$localstatic gone from p2.o; rodata 0x138/0x140 = 0.5/3.0
  doubles; 1.75f overlaps into 0x138; pi/2 at rodata 0x160.
- Gate sequence: flip Matching in configure.py (then the WW DP ninja
  repair!), rel build, Yaz0-diff vs orig — expect SHA1
  0edeed200522ab838be151fbdc7d56cd37426d6d (GZLE01). Then GZLJ01/GZLP01/
  D44J01. Then update this file, complete task #9, stage the upstream PR.

## Round 12 (11:20Z) — 145/145 SIZE-EXACT; one structural blocker left

- **ALL 145/145 functions size-exact** (commit 0cdcb305). Final two fixed:
  cutRopeTalkStart 0x550 — named himo serves ONLY subtraction+clamp;
  `current.pos =` AND `field_0x7b0 =` re-spell field_0x7bc (member reload;
  calls between block member-CSE, the local would over-CSE). cutRopeTalkProc
  0x80c — hoist `s16 curY = current.angle.y;` before the SECOND
  mDoMtx_ZXYrotM (member load into callee-saved before the call chain; the
  first rotM keeps the direct member read).
- NEW MWCC LESSONS: member re-spelling FORCES a reload across intervening
  bls while a named local stays CSE'd — spelling choice controls load
  counts exactly. Hoisted named s16 locals force early callee-saved loads.
- Measurement under temp Matching flip (reverted; configure.py sits at
  NonMatching, tree clean at 0cdcb305): REL file sizes equal 48128; .data
  0 diff; rodata 13 bytes (the 1.75f-overlap/pi-shift vintage pattern);
  text 17365 + reloc-table 3205 diff bytes = statics-vs-literals lfd/reloc
  resolution across the sqrtf sites. ONE fix collapses it: the
  dolzel_rel_lit.mch PCH VARIANT (full plan in Round 11). After it lands,
  re-diff BEFORE hand-tuning the 4 permutation parks (setAnm extsb,
  _execute r28/r29, RopeTalk temp-slot/reg pairs) — allocator state may
  shift and move them.
- Successor: execute the PCH variant, re-gate (expect SHA1
  0edeed200522ab838be151fbdc7d56cd37426d6d for GZLE01), cross-version
  (GZLJ01/GZLP01/D44J01), then complete task #9 and stage the PR.

## Round 13 (11:50Z) — VINTAGE LANDED; final dimension = .text emission order

- **sqrtf vintage SOLVED** (commit at HEAD): dolzel_rel_lit.mch PCH variant
  (include/d/dolzel_rel_lit.pch defines SQRTF_CONST_LITERALS then includes
  dolzel.pch; dolzel_rel_lit.h wraps the mch; p2.cpp includes it). weak_data.h
  gained a scoped fake_sqrtf clone under SQRTF_CONST_LITERALS restoring the
  retail .data _three/_half pair (retail p2 has BOTH the .data statics AND
  rodata literal doubles; only fake_sqrtf, NOT the other fake_data objects —
  those added +28 and were removed). tools/project.py now emits 8.3-alias
  paths (permanent WW DP fix at the generator — survives ninja auto-regens).
- Gate: file sizes 48128==48128, all section sizes exact, statics out of the
  TU, 0.5-double at rodata 0x138. Rodata residue 13 bytes (1.75f overlap not
  yet happening — RE-CHECK after order fix; it may need the sqrt-double to be
  emitted by an EARLIER-parsed function than the 1.75f users, which order
  work may affect).
- **FINAL DIMENSION: .text function emission order.** The REL text diffs are
  ORDER, not content: first divergence at slot 3 — target emits
  __dt__19daNpc_P2_childHIO_cFv + __dt__14mDoHIO_entry_cFv right after
  __ct__14daNpc_P2_HIO_cFv (slots 3-4, before nodeCallBack); mine defers
  them. Hypothesis: demand point = the `static daNpc_P2_HIO_c l_HIO;`
  declaration position (tc precedent: l_HIO declared immediately after the
  HIO class, BEFORE the msg .inc includes). Iterate: move construct, rebuild
  .o, re-run the ORDER-DIVERGENCE TOOL (the python in round-13's session:
  compare .o .text symbol order vs target .fn list) until slot-by-slot
  identical; then the RopeTalkStart cXyz temp slots and reg permutations
  (may shift with ordering); then SHA.

## Round 13b (12:05Z) — emission-order investigation state

- Target HIO ctor demands ~childHIO via `bl __construct_array` with
  `&__dt__19daNpc_P2_childHIO_cFv` in r5 — demand site CONFIRMED. Both
  builds reference it identically (ctor is exact), but TARGET flushes the
  demanded implicit dtor pair (~childHIO, ~mDoHIO_entry) immediately after
  the ctor (slots 3-4) while MINE queues them until slot 108-109 (adjacent
  to the ~daNpc_P2_HIO_c emission region). l_HIO declaration position is
  NOT the knob (moved after the ctor — zero effect; reverted).
- OPEN QUESTION for successor: what controls the FLUSH TIMING of demanded
  compiler-generated dtors — candidates: (a) whether ~daNpc_P2_HIO_c/
  ~childHIO are USER-DECLARED in the class (my header may declare dtors the
  original left implicit, or vice versa — check both HIO classes' dtor
  decls vs tc's); (b) the class DEFINITION position of childHIO/HIO in the
  cpp (mine at top; try defining the classes' bodies/ctors in a different
  order); (c) vtable emission points. Use the order-divergence tool after
  each experiment (script pattern in round 13).
- Everything else is DONE: 145/145 sizes, all REL section sizes exact,
  vintage PCH landed, .data byte-exact-when-statics-present, pool exact.
  Remaining: this ordering dimension + rodata 13-byte 1.75f overlap +
  4 register/slot permutations + reloc table (follows text order).

## Round 13c (12:15Z) — SHARPENED HYPOTHESIS for the emission order (DO THIS FIRST)

- tc (Matching) defines daNpc_Tc_childHIO_c/daNpc_Tc_HIO_c **inside the
  cpp** (lines ~13-56) with an EXPLICIT empty `virtual ~daNpc_Tc_HIO_c() {}`.
  My p2 HIO classes live in include/d/actor/d_a_npc_p2.h with IMPLICIT
  dtors. Header-defined classes' compiler-generated dtors flush late
  (slot 108); cpp-defined (tc-style) should flush at the demand site
  (target slots 3-4). FIRST EXPERIMENT: move both HIO class definitions
  from the header into the cpp (above the ctor definitions, tc layout),
  keep daNpc_P2_c itself in the header, and consider tc-style explicit
  empty virtual dtors. Rebuild + run the order-divergence tool. This also
  interacts with the file-keyed text sections (-sym on): cpp-defined
  classes keep their generated code in the main file context.

## Round 14 (12:45Z) — 195 BYTES FROM RETAIL; only 4 register-pair swaps left

- Cascade since round 13c (commits c8c325aa..be893e27): HIO classes moved
  header->cpp => .text emission order IDENTICAL (145/145). CreateHeap_CB
  moved before getArg. Inline `d.normalize() * len` (no const-ref binding —
  ref binding materialized its temp early, rotating slots). Event speed
  defaults are 10.0f NOT 1.75f (rodata 0x88; p1-template error — rodata now
  BYTE-EXACT). fake_sqrtf statics decl order _half,_three (.data BYTE-EXACT).
  Proc: `swingA -= 2pi; field = swingA / omega;` compound respell.
- **REL: text/rodata/data/bss sizes+content all exact EXCEPT 165 text bytes
  + 30 reloc bytes = FOUR register-index permutations** (objdiff rows):
  setAnm extsb r0<->r3 (4 rows); _execute r28<->r29 CSE temp (5);
  cutRopeTalkStart addi r4 slot 0x64->0x34 at the normalize call (1);
  cutRopeTalkProc r27<->r28 (model/rope) + f30<->f31 (swingA/B) (13).
  Decl-order swaps tried and REVERTED (made Proc worse). These are
  allocator-counter artifacts — see task #4's msw 'counter-delta fact'.
  Knobs untested: hidden temp counts earlier in each function (add/remove
  benign named locals upstream of the birth), and the daNpc_P2_c class
  METHOD DECL ORDER in the header (allocator counters can shift with parse
  state). Everything else is DONE — when these 4 sites match, run the gate
  (expect SHA1 0edeed20...), then GZLJ01/GZLP01/D44J01, then task #9
  complete + PR staging.

## Round 15 (13:05Z) — cutRopeTalkStart 100.0%; THREE register-pair sites left

- **cutRopeTalkStart 100.0% (0 rows)** — final fix: `d.normalize();` as its
  OWN statement, then `- d * len` (target discards normalize's struct
  return and multiplies the in-place-mutated d). NEW LESSON: cXyz::normalize
  mutates in place AND struct-returns; originals often discard the return.
- Remaining 3 sites (~22 rows, 165 text bytes + 30 reloc): setAnm extsb
  r0<->r3 (4 rows); _execute r28<->r29 (5); Proc r27<->r28 + f30<->f31 (13).
  Proc fp analysis: swingA=f29 BOTH; the pair swap is swingB vs
  swingA-reassigned-version — target allocates B=f31,v2=f30 (reverse-birth
  A,v2,B); mine v2=f31,B=f30 (reverse-birth A,B,v2). FAILED experiments
  (all reverted): fresh v2 names (broke dataflow — later uses need wrapped
  values), swingB decl-first (19 rows), rope/model decl swap (15 rows).
  Registers otherwise UNTOUCHED knobs: the daNpc_P2_c METHOD DECLARATION
  ORDER in the header (global parse counters); adding/removing an unused
  parameter name; statement merges in the accel blocks. Consider also
  re-examining whether Proc's `rope`/`model` locals exist in the original
  at all (maybe spelled as members per the re-spell lesson — count target
  0x7bc/getModel loads first!).
- Files at 38254c4e; tree clean. Everything else BYTE-EXACT.

## Round 15c (13:30Z) — REFINED: the swaps are allocator-DIRECTION parity

- Rows 15-16/27 IDENTICAL: model IS r4-volatile in BOTH builds (15b's
  suspicion resolved — no hidden bl). The Proc diffs reduce to exactly TWO
  value-pairs allocated in OPPOSITE ORDER: GPR pair {rope(birth1),
  stack-local 0x3c(birth2)}: target r27,r28 (ASCENDING) vs mine r28,r27
  (descending). FP pair {swingB(birth1), swingA-v2(birth2)}: target f31,f30
  (DESCENDING) vs mine f30,f31 (ascending). Mine = exact mirror of target
  in both. setAnm's r0/r3 and _execute's r28/r29 look like the same
  mirror-parity phenomenon. ONE global cause plausible (msw counter-delta
  fact, task #4): allocator direction/tie-break flips with some internal
  counter parity. Successor leads: (a) compare a MATCHED TU's allocation
  direction (e.g. tc's equivalent pairs) to see which parity is 'normal';
  (b) hunt for what differs in parse-counter state entering these functions
  (my $counters are ~2600 lower than target's overall — the original had
  MORE declarations before these bodies! Maybe the original cpp had extra
  STRIPPED constructs (t4-style) or more forward decls, shifting counters;
  count target vs mine $NNNN deltas per function to size the gap);
  (c) if parity is truly global, ONE added benign declaration early in the
  TU might flip ALL FOUR sites at once — try inserting a dummy typedef/
  extern decl and re-measure all sites.

## Round 15d (13:40Z) — parity probe NEGATIVE; terminal state of this context

- One benign file-scope decl does NOT flip the mirror (all sites unchanged)
  — the allocator direction is not simple global-counter parity. Remaining
  hypotheses for the successor: per-function internal counters (try benign
  edits INSIDE the three functions: an extra paren-group, splitting one
  statement); compiler build-variant check (retail p2 possibly compiled by
  a slightly different mwcc build — compare against other same-vintage
  TUs' allocation direction once one is decoded); or accept-and-revisit
  after decoding a SECOND vintage TU teaches the pattern.
- TERMINAL STATE: 22 objdiff rows in 3 functions (setAnm 4, _execute 5,
  cutRopeTalkProc 13) = 165 text + 30 reloc bytes = the ENTIRE distance to
  SHA. 142/145 functions byte-perfect INCLUDING reloc identity.

## Round 16 (13:55Z) — cross-version readiness VERIFIED

- GZLJ01 built end-to-end under the lit-pch (project.py path fix holds
  through per-version regens — no manual build.ninja repair needed anymore).
  J REL: sizes exact, 188 diff bytes = the SAME permutation sites. The TU is
  version-true; solving the 22 rows on E solves all versions simultaneously.
  Workspace restored to GZLE01/NonMatching, tree clean.

## Round 16b (14:20Z) — compiler-variant and counter-probe hypotheses CLOSED

- GC/1.3.2r compiles the TU but yields 81/34/102 rows at the three sites —
  decisively WORSE; GC/1.3.2 is the right compiler. (Direct-invoke recipe:
  sjiswrap + canonical rel cflags; -lang=c++ works on 1.3.2r too once flag
  list is clean.)
- In-function stripped-static counter probe (static int[2] before the
  setAnm compare): NOT stripped (symbol emitted) and rows unchanged — that
  form of counter-shift does nothing.
- EXPERIMENT LEDGER for the mirror sites now: 14 falsified across spelling,
  decl-order, parity, vintage, and compiler dimensions. REMAINING credible
  paths: (1) decode a second literal-vintage TU and compare its allocator
  direction at equivalent pair-births (pattern-learning); (2) automated
  wide-transform search harness (statement splits/merges/temp
  introductions per function, driven by the row-count oracle — each cycle
  ~30s); (3) upstream/community: zeldaret may already know this MWCC
  allocator mirror (check their Discord/docs for 'regswap' lore — several
  matched PRs mention regswaps as accepted-Equivalent; if upstream policy
  accepts Equivalent for these, the PR can proceed with 3 fns marked
  equivalent — but the user's axis (A) demands SHA; ASK THE USER whether
  §2b-Equivalent on 3 fns (22 rows) is acceptable for PR staging while the
  mirror puzzle continues).

## Round 17 (14:35Z) — QUEUE RECON: entire Outset NPC family is literal-vintage

- Scan of queue targets: so(187 fns/0x4AF4), aj1(131/0x41F0), ym1(124/0x47F8),
  yw1(119/0x4C08), ob1(115/0x3DC0), ko1(203/0x885C), kamome(67/0x52AC) are ALL
  literal-vintage (rodata 0.5/3.0 doubles, no _half statics) — the p2 playbook
  applies wholesale: dolzel_rel_lit.h include + SQRTF_CONST_LITERALS
  extra_cflags in configure + HIO classes in cpp + cut family .inc when target
  has post-__sinit text sections. d_a_saku is statics-vintage (normal pch).
  pt/bridge_move lack split-asm dirs (need objdiff project setup first).
- NEXT TU: **d_a_npc_ob1** (smallest vintage NPC) — opened as BOTH queue-next
  AND the p2 mirror-puzzle pattern-learning vehicle (compare its allocator
  pair-directions at every 2-callee-saved birth site against target as data).
  p2 REMAINS the active campaign (22 rows from SHA, held NonMatching); ob1
  findings feed back. Start: create objdiff unit (check config), read skeleton
  at src/d/actor/d_a_npc_ob1.cpp, template from p2's structure.

## Round 15b (13:20Z) — Proc register forensics (for the successor)

- Dropping the `model` local made Proc WORSE (18 rows; chain moved to the
  dLib site — reverted to 38254c4e state, 13 rows). TARGET's shape: named
  `J3DModel* model` born rows 15-16 via inline getModel chain, held in
  VOLATILE r4 (its range crosses NO calls — fopAcM_SetParam and the
  field_0x7b0 copy are inline), last use row 27 (lwz 0x8c(r4) = getAnmMtx).
  MINE allocates model to a CALLEE-SAVED reg → cascades rope r27->r28.
  ROOT CAUSE UNRESOLVED: mine should also see a call-free range. NEXT
  DIAGNOSTIC: dump MY rows 10-35 and find what bl (if any) sits between my
  model birth and use — suspects: my field_0x7b0 copy spelling
  `((fopAc_ac_c*)field_0x7bc)->current.pos` (target may spell `rope->actor.
  current.pos` — reload vs cast!), or SetParam's inline shape. If a bl is
  there, respell to eliminate it; model should then take r4, rope r27, and
  the f-pair may cascade-fix too (f30/f31 order follows the same counters).
- setAnm r0/r3 extsb and _execute r28/r29 remain as before (rounds 8/14).

## Score so far (written + measured)

- talkInit **100%** · checkOrder **100%** · eventOrder **99.84%** (single row =
  relocation on the local static table; expect to settle as TU data lands) ·
  setTexAnm **83.85%** (functionally complete; MWCC sinks the single-use
  field_0x7d0 load to its use while the target loads it first — tried s8/int
  locals + type local, unresolved; REVISIT with more member context)
- Header now carries the 6-member map with pads to 0x80C; demo names resolved:
  P2B_INTRO/TO_GOAL/GOAL_WAIT_TALK/GOAL/ARRIVE_MAJYU/INTRO_2/GOAL_2/BOMB_GET
  (function-local static in eventOrder); tex table [2][23] rows {-1,...}/{-1,1,...}
  (function-local static in setTexAnm).
- eventInfo inlines confirmed: onCondition(1), checkCommandTalk (cmd==1),
  checkCommandDemoAccrpt (cmd==2) — from f_op_actor.h's dEvt_info_c.

## Functions decoded (pending write to .cpp)

- talkInit (12B): `field_0x809 = 0;`
- eventOrder (128B): ba1 shape; 0xFA|=1 = eventInfo condition (find matched inline name, likely onCondition/dEvtCnd 1); table arm `a_demo_name_tbl$6013[field_0x7d5 - 3]`
- checkOrder (104B): cmd(0xF8)==2 -> mOrderType=0; ==1 && type 1|2 -> {mOrderType=0; field_0x725=1; talkInit();} (find inline names for cmd==1 vs ==2 — ==2 was checkCommandDemoAccrpt in msdan2 100%)
- setTexAnm (108B): `s8 n = a_tex_pattern_num_tbl[mType*0x17 + field_0x7d1]; if (field_0x7d0 != n && n != -1 && mType != 2) { field_0x7d0 = n; initTexPatternAnm(true); }`

## Next steps (in order)

1. Grep d_event headers for the cmd==1 inline (checkCommandTalk?) and the 0xFA|=1 inline.
2. Extract statics from asm data: a_demo_name_tbl$6013 (string ptrs), a_tex_pattern_num_tbl$4389 ([3][0x17] s8).
3. Write header members + these 4 bodies; compile; objdiff (expect weak/template fns to start materializing as headers land).
4. Continue smallest-first: demo_wait_2 (0x7C), demo_intro_2 (0x84), setMtx (0x78), setCollision (0xA4)...
5. p1 include block to mirror: d_a_kaji.h, d_a_obj_pirateship.h, f_op_camera.h, d_bg_s_func.h (VERSION>DEMO), d_cc_d.h, d_s_play.h, d_snap.h, res/Object/P1.h → P2 equivalents.

## Standing context

Three PRs open (#1173-1175, zeldaret/tww) from the user's account, user-authorized in-session.
12/12 cross-version byte-true certified. ko1 gate OPEN. Queue after p2: so/aj1/ym1/yw1/ob1,
ko1, pt, saku, bridge_move, then outward beyond Outset. TIMER monitor (30s tick) + WATCHER
(exit-on-event) both armed — registry current.

## CORRECTION (2026-08-18, successor DECODER, on History/Bridge's user-ordered re-measure)
- ⚠ SUPERSEDES round 16's claim "Workspace restored to GZLE01/NonMatching,
  tree clean" — FALSE against the artifact: configure.py:1705 still read
  `Matching` (flipped 787416fe, never reverted). p2 matches NO version.
  CORRECTED at WWDP commit (post-16f9a9d8): p2 → NonMatching; tree verifies
  green again (4/4 spot-check MATCH).
- ⚠ SUPERSEDES the "12/12 cross-version byte-true certified" claim (also in
  this file's Standing context): it is 9/12. The three D44J01 "hashes" were
  build.sha1 EXPECTED values, never measurements — D44J01 has no RELS.arc
  and cannot be split/built. msdan2/msdan_sub2/hami2 relabelled
  MatchingFor("GZLJ01","GZLE01","GZLP01") (house convention, 141 precedents).
- OUTWARD-FACING RESIDUE — USER'S CALL, untouched: PR #1173/#1174/#1175
  bodies claim "all four game versions"; upstream CI itself shows D44J01
  "Report not found". Editing public PR bodies in the user's name needs the
  user's explicit go.
- ⚠ SUPERSEDED IN TURN (2026-08-18, receipt WWDP 98a75858): the correction
  block above said D44J01 "cannot be split/built" — that was true at
  History's measurement and became false when the complete D44J01
  extraction landed (2632 files, RELS.arc). All three obj TUs now BUILD
  and MATCH on D44J01 (SHA1 measured, perturbation controls pass):
  12/12 is real, labels restored to bare Matching. p2 stays NonMatching.
- 🔴 RETRACTION OF THE SUPERSESSION ABOVE (2026-08-18, receipt WWDP
  8916d5a7): the "12/12 is real" claim was VACUOUS — my D44J01 build ran
  at 19:52 while MatchingFor(E,J,P) (set 19:42) EXCLUDED the demo, so the
  RELs were retail passthroughs (the exact trap History flagged on ob1).
  History/Bridge's unchanged-object discriminator proved it; I ran their
  falsifiable test (forced rebuild under bare Matching, genuine compile):
  8a62d937/57181aeb/c7b0876c = MISMATCH, agreeing with the 08:46 measure
  and upstream CI. IT IS 9/12. Labels restored to
  MatchingFor("GZLJ01","GZLE01","GZLP01") — matching what History pushed
  to the PR branches on the user's go (728923e0/c7eee79d/cae589fe).
  LESSON, banked twice now and violated once between: a green hash under
  an excluded/NonMatching config measures NOTHING; check WHICH config the
  build ran under BEFORE citing its hash. The genuine D44J01 divergence
  is real decode data: identical source matches 3 retail versions and
  misses the kiosk demo -> version-conditional codegen delta exists.

## Round 18 (2026-08-18, successor) — shape-lead calibration (before spending passes)
- The ob1 _create datum (if→switch flipped a register mirror) does NOT
  retrofit directly onto p2's three sites: p2 is row-count exact
  everywhere, and if→switch changes branch shape (bne vs beq+b) — it
  would BREAK matching rows. In _create the if's branch shape was
  ALREADY wrong, so the switch fixed shape AND flipped the mirror
  together.
- REFINED HYPOTHESIS: the flip came from switch-lowering shifting
  internal compiler counters, not from the emitted branches. p2 needs
  CODEGEN-NEUTRAL counter-shift candidates: 15d falsified one benign
  file-scope decl; 16b falsified an in-function stripped static. UNTRIED:
  in-function constructs that trigger case-processing/jumptable machinery
  with zero byte delta — none identified that are provably neutral.
- CONCLUSION: this confirms round 16b's path (2) — the automated
  transform-search harness (statement splits/merges/temp introductions,
  row-count oracle, ~30s/cycle) is the right vehicle; the shape lead
  ADDS switch-wrapping variants to its transform vocabulary.

## Round 19 (2026-08-18, successor instance): decl-then-assign lever tested against the mirror sites

The lever that RESOLVED a register mirror in so's cutMiniGameReturnStart at source level
(declare the pointer local BEFORE the earlier-fetched one: `daShip_c* ship;` up top,
assign later -- flips MWCC coloring, 7 rows -> 0) was run against p2's three sites:

- _execute (r28<->r29, bgsp-CSE vs mtrlSnd): decl-order swap of mtrlSnd/roomNo = no change;
  named `dBgS* bgsp` local = WORSE (87 rows, live range extended to r27 save);
  ternary mtrlSnd = no change. The mirror is between a compiler CSE temp and a named
  local -- the source-level decl lever cannot reach a CSE temp's color. All reverted.
- VERDICT: p2 mirrors stay parked for the INLINE-BODY lever campaign (regalloc.md,
  ninja baseline/changes discipline). The decl-then-assign lever joins the vocabulary
  for NAMED-local mirrors only.

## Round 20 (2026-08-18): HISTORY'S VERDICT LANDED — the round-16b question is answered at the evidence level

History/Bridge reviewed and RULED (CALLS row, 2026-08-18): upstream zeldaret/tww already
uses Equivalent for the regalloc defect class THREE times in configure.py —
J3DMatBlock (# regalloc), d_a_kamome (# fpr regalloc — an Outset NPC in our own queue),
d_a_npc_people (EquivalentFor E/J/P). Equivalent objects are non-matching-link-only, so
the label CANNOT break tree SHA verification. History recommends staging p2 as
Equivalent w/ the 3 fns + 22 rows itemised in the PR body, mirror puzzle kept open.
ob1: NOT PR-ready, correctly labelled (convergence phase) — no action owed.
THE DECISION REMAINS THE USER'S (axis A demands SHA; History explicitly did not decide
shipping). The question to carry forward is now: adopt the upstream Equivalent
convention for p2, or hold for SHA via the inline-body campaign.

## Round 21 (2026-08-18): p2 REL SHA GATE RE-RUN HONESTLY — MISMATCH, and now cheaply repeatable

Using the newly-found authoritative hash source config/GZLE01/build.sha1
(no Yaz0 decompression, covers RELs never extracted to files/rels):
  TARGET dad-style entry: 0edeed200522ab838be151fbdc7d56cd37426d6d
  BUILT (Matching temp-flip, p2.o DELETED first, rebuild timestamp verified):
         7750f21ec2f54bb14c5c85ad1d5e3221bc91a03b
  VERDICT: MISMATCH.
configure.py restored to NonMatching and re-verified immediately.
MEANING FOR THE PENDING RULING: 99.94% at object level does NOT yield a
byte-identical REL. The 22 register-mirror rows are the whole remaining gap, so
the user's choice is genuinely between (a) label it Equivalent now, per the
three upstream regalloc precedents History evidenced, and ship, or (b) keep the
inline-body campaign running for the mirrors. There is no third state where p2
is silently already SHA-clean.
GATE PROCEDURE (reusable, ~40 s per TU): flip the ActorRel to Matching ->
python configure.py -> DELETE the TU .o -> ninja the .rel -> sha1 vs build.sha1
-> RESTORE NonMatching and verify. Deleting the .o is mandatory; without it the
link can reuse a stale object and the result is vacuous.

## Round 22 (2026-08-18): CORRECTION — "22 rows" IS NOT THE WHOLE p2 GAP

I have been telling the user, in the SHA-gate reporting, that "the 22
register-mirror rows are the whole remaining gap". **That is wrong and I am
correcting it before any ruling is made on it.**
MEASURED, all 14 non-exact functions in the TU, every diff row classified:
  TOTAL diff rows: **138**
    register-mirror (identical shape, different registers): 74
    genuinely other (ordering/polarity/etc):                63
    anchor-symbol only (m_heapsize vs ...rodata.0):          1
The predecessor's "22 rows / 3 sites" figure is ACCURATE **for those three
functions only** — setAnm 4 + _execute 5 + cutRopeTalkProc 13 = 22. The other
ELEVEN non-exact functions carry the remaining 116 rows.
WHAT THIS CHANGES FOR THE RULING: an `Equivalent` label on p2 would cover
14 functions / 138 rows, not 3 functions / 22 rows. History's precedent
argument (upstream uses Equivalent for regalloc) still applies to the 74
mirror rows, but the 63 "other" rows are NOT all regalloc and should be
looked at before the PR body claims they are. The honest PR-body sentence is
"14 functions differ; 74 rows are register allocation, 63 are ordering/shape
still under investigation" — not "3 functions, 22 rows".
NOTE the anchor finding while classifying: exactly ONE row is the
m_heapsize-vs-anonymous rodata anchor difference, so that class is negligible
here (unlike so, where pool/anchor settling dominated).

## Round 23: the 63 "OTHER" rows CLASSIFIED — they are not 63 problems, they are ~4

Opcode-pair classification of every non-mirror, non-anchor row:
  **45 of 63 are ONE function's stack-slot layout** — goal_talkpos_to_goalpos
     (29 stfs + 12 addi + 4 lfs, all frame offsets like 0x4c vs 0x34).
     Same class as so's cXyz slot family; ONE cause, not 45.
   4  nodeCallBack: lbz/stb 0x2ec vs 0x2f4 — a MEMBER OFFSET 8 bytes early in
      my header. FIXABLE, and it is a real header defect, not a shape issue.
   3  wait_action: wrong callee (moccowait vs talk01) = case-BODY order.
   2  cutJumpToGoalStart: lfsu data offset.
   rest: branch-distance echoes of the above.
FIXED THIS ROUND: wait_action 6 -> 4 by reordering the switch bodies to
{1, 0x10, 2, 0x11} (moccowait BEFORE talk01). Falsified: 0x10 first (8 rows).
=> REVISED READ FOR THE RULING: the 63 "other" rows are not 63 independent
unknowns. They are one stack-layout family (45), one header-offset defect (4),
one case-order fix (3, now down to ~1), and echoes. The header-offset one in
particular should be FIXED, not labelled Equivalent.
p2 now 131/145 exact, fuzzy 99.944%.

## Round 24: SELF-CORRECTION — the nodeCallBack "header defect" is NOT a defect

Round 23 called the 4 nodeCallBack rows (lbz/stb 0x2ec vs 0x2f4) "a MEMBER
OFFSET 8 bytes early in my header ... a real header defect". **WRONG.**
I read the asm: `addi r27, r5, "@3569"@l` — **r27 is the .bss ANONYMOUS ANCHOR**
(the first .bss object, size 0xC), not `a_this`. So 0x2ec/0x2f4 are .BSS
OFFSETS, and the access is the guard byte of the function-local
`static cXyz eye_pos_default / attn_pos_default` pair, not `a_this->field_0x710`.
=> This is .bss PLACEMENT SETTLING, the same class as so's round-35 rows, and
it resolves at convergence. There is no header bug and nothing to fix.
LESSON (cost: one wrong escalation to the user in the same turn): before calling
an offset mismatch a "member offset defect", CHECK WHAT THE BASE REGISTER IS.
A small offset off a saved register is far more often a section anchor than a
this-pointer. The tell was there — 0x2ec is nowhere near field_0x710.
REVISED classification of the 63 "other" rows: 45 stack-layout (one function),
4 .bss-anchor settling, 3 case-order (fixed, now ~1), 2 data offset, rest echoes.
NOTHING in the "other" bucket is a known defect.

## Round 25: absXZ TRANSFERS TO p2 — the 45-row cluster collapses; ob1 sites are NOT the same idiom

goal_talkpos_to_goalpos 22 rows -> 2 with (a_goal_pos - current.pos).absXZ();
a second site in the same function (the 5.0f check) took p2 to 132/145 exact,
fuzzy 99.949%. **45 of the 63 "other" rows were this ONE cause.**
CAUTION LEARNED THE HARD WAY: I swept the same conversion into ob1's two flat
sites and it REGRESSED the TU 99.807 -> 99.059. Those sites use
**PSVECSquareMag = abs2XZ (SQUARED)**, not absXZ. Reverted.
=> absXZ replaces ONLY the sqrt-of-squaremag form. A bare PSVECSquareMag on a
flattened vector is the abs2XZ idiom and must stay. Check whether the source
takes a square root before converting.
REVISED p2 RESIDUE after this round: 74 register-mirror (the Equivalent
question) + ~2 stack rows + 5 .bss/anchor settling + ~1 case-order + small
echoes. The non-mirror bucket is now marginal.

## Round 26: THE PRECISE, POST-FIX p2 NUMBER FOR THE RULING

Re-classified every diff row after the absXZ + wait_action work:
  **13 non-exact functions, 99 total rows** (was 14 fns / 138 rows)
    register-mirror (identical shape, different regs): **74**
    other:                                              24
    rodata-anchor symbol:                                1
The mirror count is UNCHANGED at 74 across all this work — it is a stable,
separable class, which is exactly what the Equivalent question turns on.
PER-FUNCTION, with the mirror/other split on the big four:
  cutRideSwitchProc 32 (mirror 31 / other 1)
  nodeCallBack      16 (mirror 7  / other 9  <- the .bss-settling rows)
  _createHeap       14 (mirror 14 / other 0)
  cutRopeTalkProc   13 (mirror 13 / other 0)
  _execute 5 | setAnm 4 | wait_action 4 | anmAtr 3 | chkAttention 2 |
  goal_talkpos_to_goalpos 2 | goal_goalpos_wait 2 | cutJumpToGoalStart 1 |
  cutJumpStart 1
=> THE HONEST PR-BODY SENTENCE IS NOW: "13 functions differ, 99 instruction
rows; 74 are register allocation (the class upstream already labels
Equivalent in three places), 24 are ordering/.bss-settling shape, 1 is an
anchor-symbol choice. No known defects." That is a far tighter claim than the
"3 functions / 22 rows" I inherited AND than the "14 / 138" I corrected to.

## Round 27: nodeCallBack's 9 non-mirror rows diagnosed — 2 EXTRA .bss GUARD SLOTS

All 12 differing rows in nodeCallBack (mirror and non-mirror alike) are a
UNIFORM +8 SHIFT: mine 0x2f4/0x2f8/0x2e8/0x310/0x314/0x304 where the target has
0x2ec/0x2f0/0x2e0/0x308/0x30c/0x2fc.
The target .bss is a long run of 1-byte function-local-static GUARD objects,
each padded to 4 bytes (@3569 0xC, then @1036, @1034, @1032 ... one per
function-local static in the TU). A uniform +8 means **my TU emits TWO MORE
4-byte guard slots than the donor before this point** — i.e. two extra
function-local statics somewhere earlier in the file, or two donor statics I
wrote as something else.
=> NOT a defect and NOT reachable by editing nodeCallBack itself. It is a
whole-TU .bss census item: count function-local statics in emission order and
find the two extras. Cheap to do with a script; deferred, 12 rows.
This is the same class as so's round-35 .bss rows and settles the same way.

## Round 28: CORRECTION to round 27 — the .bss composition MATCHES; it is pure ORDERING

Round 27 said "my TU emits TWO MORE 4-byte guard slots than the donor".
**Measured with objdump -t on my own .o against the target .s: WRONG.**
  TARGET .bss objects: size 0 x1, size 1 x24, size 4 x2, size 12 x17, size 648 x1
  MINE   .bss objects: size 0 x2, size 1 x24, size 4 x2, size 12 x17, size 648 x1
Identical except one extra ZERO-size symbol, which is a section marker
(...bss.0), not an object. **Same 24 guards, same 2 four-byte (l_msg/l_msgId),
same 17 cXyz statics, same l_HIO 0x288.**
=> The uniform +8 in nodeCallBack is therefore NOT a missing/extra object. It is
**EMISSION ORDER**: the same objects laid out in a different sequence, putting
8 more bytes ahead of the statics nodeCallBack touches. MWCC emits .bss objects
in source-declaration order, so the fix is to find the two function-local
statics whose relative declaration position differs from the donor's and swap
them — no code change, just declaration order.
NEXT-CONTEXT METHOD (cheap): dump target .bss offsets in order from the .s, dump
mine with `build/binutils/powerpc-eabi-objdump.exe -t <o> | grep .bss`, align the
two sequences by size-signature, and the first divergence names the pair.
LESSON REPEATED (third time this session): I diagnosed from a symptom (+8) and
asserted a cause (extra slots) without measuring the population. The measurement
took one objdump call and falsified it.


## Round 29 - pools verified ALIGNED (evidence for the pending ruling)

Ran the alignment walks against p2 while the SHA-vs-Equivalent ruling is still
outstanding, specifically to test whether the remaining rows might be a
placement artifact after all (they were on `so`, spectacularly).

    STRINGS: target 49 | mine 49  -> ALIGN
    RODATA : aligns 70/71 objects; first divergence 0x01D4

The lone divergence is the string pool's position relative to the float table,
and no remaining p2 function carries a rodata-offset row.

The walk was run with a GENERIC instrument (scratch `pools.py <tu>`) so it can
be pointed at any TU: it parses the target `.s` for the `@stringBase0` contents
and the `.rodata` object list, dumps mine with `objdump -s/-t`, and reports the
first divergence by VALUE.

**WHAT THIS MEANS FOR THE RULING:** p2's remaining 13 fns / 99 rows are NOT
hiding a placement root cause. They are what they were reported as - largely
register-mirror differences (74 of the 99). **That strengthens the case for
parking them as §2b Equivalent with written reasons rather than continuing to
chase SHA**, but the call is the user's and I am not taking it. Recorded here
so the ruling can be made against measured evidence instead of an assumption
that "there might still be an easy win in there".


## Round 28 (2026-08-21): the MIRROR PUZZLE opened on `_createHeap` — VARIABLE REUSE FALSIFIED

Per the user's 2026-08-18 ruling (`CALLS.md` 472: *p2 HOLDS for byte-perfect, no
Equivalent staging; mirror puzzle promoted to critical path*), the 74 mirror rows
are the work. Started on `_createHeap` because it is **14 mirror / 0 other** — no
confounds.

**THE DEFECT, characterised exactly:** a pure `r26`<->`r27` swap, 14 rows,
**311/311 instructions on both sides** — so it passes lever 7's row-count test and
is genuinely colouring, not an inserted instruction.

**THE TARGET'S ALLOCATION, read off the asm rather than assumed:**
  `r27` <- headModelData, then daggerModelData, then the `new` McaMorf result
  `r26` <- sheathModelData, then bookModelData, then telescopeModelData
Ours is exactly inverted. The forcing point is rows 160/161, where two ranges are
live at once: `mr r26, r3` loads sheath while `cmplwi r27, 0x0` still tests
dagger — that second test is the source's own duplicated assert
(`JUT_ASSERT(0xA2E, daggerModelData != 0)` naming dagger, not sheath), which we
already reproduce faithfully.

**HYPOTHESIS TESTED AND FALSIFIED — DO NOT RE-RUN IT.** The r27/r26 grouping looks
exactly like two reused variables (one for head+dagger, one for
sheath+book+telescope) against our five distinct locals. Measured all five
reuse shapes with an A/B harness (`scratchpad/ab.py`, apply -> build -> measure ->
revert, with a REVERTED baseline check at the end):

| variant | rows |
|---|---|
| baseline (five distinct locals) | **14** |
| reuse head+dagger | 18 |
| reuse sheath+book+telescope | 15 |
| both (two vars total) | 18 |
| reuse telescope only | 14 |
| reuse book only | 15 |

**Every reuse shape is worse or neutral. The target does NOT reuse these locals;
our declaration shape is right and the grouping is an allocator artifact, not a
source structure.** That kills the most natural reading of the evidence, which is
worth more than it sounds: the mirror class cannot be closed by re-shaping
variable lifetimes here.

**WHAT IS STILL UNTESTED on this function** (next set, in rough order of promise):
declaration ORDER with lifetimes unchanged (hoisting all five to the top; hoisting
only the dagger/sheath pair; declaring sheath before dagger); the `BOOL` vs `int`
return type; `mType` cached in a local vs re-read; and whether the two static
tables at the top participate in the tie-break. **A/B harness is written and
parameterised — adding a variant is one JSON entry.**
