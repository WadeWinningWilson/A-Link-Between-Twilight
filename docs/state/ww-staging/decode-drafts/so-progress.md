# d_a_npc_so decode campaign (queue-next after ob1) — KICKOFF

**Read p2-progress.md CONTINUATION PREAMBLE + ob1-progress.md rounds 3-15
FIRST** — they carry the charter, monitor discipline, toolchain recipes,
the complete MWCC lesson bank (~35 lessons), the transform-search harness
(WWDP tools/decoder_transform_search.py), and the inline-lever campaign
spec that both p2 and ob1's endgames await.

## Identity + shape (kickoff recon, 2026-08-18)
- daNpc_So_c = THE FISHMAN (Esa/bait event, offsetDive/Swim/Appear,
  modeEventMapopen = the sea-chart opener, Outset presence). 187 fns,
  77 local/weak, text 0x4AF4, literal-vintage (round-17 recon).
- Base fopNpc_npc_c — ctor pattern IDENTICAL to ob1/km1 (vt@0x6c0,
  jnt 0x29b/0x29c, EventCut 0x2cc/0x2d0/0x32c, ObjAcch/AcchCir/Stts/Cyl
  standard offsets). The ob1 header is the closest template; expect a
  second EventCut/PathRun-region layout — read the ctor tail for
  derived-member inits.
- Naming style DIFFERS from ob1: modeXxxInit/modeXxx pairs (~20 modes:
  Wait Hide Jump Swim NearSwim EventFirst{Wait,,End} EventEsa
  EventMapopen EventBow Talk Disappear Debug GetRupee EventTriForce...)
  + _CB-suffixed static callbacks (searchEsa_CB, nodeControl_CB,
  XyCheckCB/XyEventCB = fopMsg XY callbacks!, createHeap_CB).
- 3 text sections → post-__sinit region B EXISTS: expect demanded-late
  constructs (cut-family .inc treatment per p2 round 7 OR HIO-in-cpp
  placement per p2 round 14 — read the region-B .fn list first).
- ⚠ configure.py:1577 `ActorRel(NonMatching, "d_a_npc_so")` LACKS the
  vintage wiring — FIRST STEP: add extra_cflags=["-DSQRTF_CONST_LITERALS"]
  + switch the cpp to dolzel_rel_lit.h (round-17 recon says so IS
  literal-vintage; VERIFY via rodata 0.5/3.0 doubles before trusting).

## Kickoff checklist (ob1 playbook, proven this session)
1. Vintage wiring (above) + build the skeleton .o; objdiff baseline.
2. Ctor asm → derived member map; header rewrite on the ob1 pattern
   (fopNpc base + typed tail); tc canary after ANY shared-header edit.
3. Smallest-fns-first with per-fn objdiff; sibling-verbatim where a
   matched sibling has the same machine shape (km1/bm1 lessons: copy
   the SOURCE, not the idea).
4. Read caller test shapes for bool/BOOL/int returns (clrlwi. = bool,
   cmpwi = int) BEFORE guessing signatures — saved ~10 iterations on ob1.
5. Assert strings + res-enum headers (res/Object/So.h in assets/) give
   original names — use them from the start.
6. Pool/data order LAST (l_evn-style file statics position, GXColor
   debug trio at _draw tail, de-const .data tables) then REL gate under
   TEMP Matching flip (NEVER cite a NonMatching-config hash).

## CORRECTION (same day): vintage wiring REVERTED
- The kickoff's "literal-vintage (round-17 recon)" is WRONG for so:
  target has 0 .double literals and 0 _half/_three statics; the 14
  frsqrte chains are f32-constant normalize-family inlines, not sqrtf.
  Round-17's recon mislabeled so. Wire the vintage PCH ONLY if/when a
  genuine sqrtf site appears during decode (then re-check this).
- Lesson re-banked (3rd time this session): verify the signal yourself
  before acting on a recon row — and never let a commit message claim
  "verified" for a grep that returned nothing.

## Ctor member map (from __ct__10daNpc_So_cFv target asm, 2026-08-18)
- Class size 0xBE4 (g_profile word 5). Base fopNpc_npc_c to 0x6C4
  (standard subobjects verified: jnt 0x29b/c, EventCut 0x2cc/2d0/32c,
  ObjAcch@0x334, AcchCir@0x4F8, Stts@0x538(vt 0x550), Cyl@0x574,
  0x6ac=-1, 0x6b0=0).
- Derived: 0x6F4 dCcD_Stts #2 (GStts@0x6F8; ALSO stores r31 — a saved
  reg value into 0x6F8+? read the head for what r31 held) ·
  0x718 dCcD_Sph block (SPHERE collider — GObjInf@0x718, Aab@+0x110,
  ShapeAttr@+0x114, Sph@+0x128, dCcD_Sph vt@+0x3C; fish body) ·
  0x854 mDoExt_btpAnm (0x858/0x860 zeroed) · 0x870 dBgS_ObjAcch #2
  (vt@0x880, +0x14=r30, +0x20=r29 — saved values, read head) ·
  0xA34 dBgS_AcchCir #2 · 0xAE8 dPa_rippleEcallBack (JPACallBack →
  levelE → rippleE vt chain; 0xAEC=0 — water ripple particles) ·
  tail to 0xBE4 unknown (read member fns).
- NOTE: the ctor head saves r29/r30/r31 BEFORE the tail — they hold
  computed defaults stored into Stts#2/Acch#2 fields; decode the head
  block when writing the ctor (implicit-vs-explicit TBD: .fn weak?
  CHECK — if global+long, so has an EXPLICIT ctor unlike ob1).
- Ctor is WEAK = implicit/in-class (ob1 pattern) — the r29-r31 defaults come from member-object inline ctors, not user code. Kickoff package COMPLETE; begin decode rounds at checklist step 2 (header rewrite).
- Header restructured + virtual overrides fixed (u16/u32); BASELINE 13/187 exact, fuzzy 1.9049%. so is VERSION-CONDITIONAL (#if VERSION_DEMO in class tail) — MatchingFor rule applies at label time. Kickoff steps 1-2 DONE; decode rounds begin smallest-first.

## Round 1 PARTIAL (03:51Z, at context depth limit — shapes banked, bodies unwritten)
- Ten smallest dumped; decode-ready shapes:
  * modeDebug / modeGetRupeeInit / modeWait = EMPTY bodies `{}`.
  * modeEventMapopenInit: mAFC(f32)=0.0f; speedF=0.0f; m_jnt lock byte
    0x29C(=mbBackBoneLock)=0 — find/park the accessor spelling.
  * modeHide: `current.pos = mA80;` (cXyz @0xA80, per-component
    interleave = plain operator=).
  * offsetZero/Dive/Swim/Appear: three f32 stores to mB40/mB3C/mB38
    (DESCENDING store order = source order) from ANONYMOUS literals
    anchored off class-static m_heapsize (p2 anchor pattern; so's class
    statics: m_heapsize@0x0, m_arc_name@0x4, m_sph_src@0x8-0x48, then
    the literal pool). Values NOT yet extracted — the offset parser
    drifted on empty .obj entries; recompute with a size-aware parser
    (count .4byte/.float lines per object) before writing these.
  * getArg: m6D0(s16?)=<fopAc 0x1DC lha>; if ==0xFFFF (addis+cmplwi
    unsigned idiom) or ==0 then m6D0=1. fopAc 0x1DC field name TBD.
- KNOWN VALUES: @4341=0.0f (Mapopen), pool floats sighted: 100/30/10/
  300/1/-1/650/200/20/-300/90 at various slots.
- STOP-POINT NOTE: this instance reached its effective depth here after
  ~4 hours continuous; successor starts at these shapes with the fixed
  parser, then walks the size ladder exactly as ob1 rounds 3+ did.
- Round 2 micro: modeDebug/modeGetRupeeInit/modeWait/modeHide 100% (17/187). ⚠ -4 layout error before 0xA80 (mA80 needed pad end 0xA84): verify AcchCir2/ObjAcch2 real sizes before trusting comments in 0x870-0xAE8.
- modeEventMapopenInit 100% (18/187); tail shifted -0x10 empirically (mAFC@real 0xAFC ✓, mA80 unaffected ✓). Layout comments 0x870-0xB00 still unverified against subobject reality.
- offsetZero/Dive/Swim/Appear written (values 0.1/20/-1, 0.3/10/-150, 0.3/10/-40, 0.4/22/110): 3 pool-settling rows each, member placement VERIFIED (mB38-B40 land exactly). 22/187 effective. Offset-comment parser pattern banked (the .s carries '# .rodata:0xNN' per object — parse those, never arithmetic).
- Round 3: search/glue/getArg batch 100% (~27/187). getArg lesson: s16 == 0xFFFF spells addis+cmplwi directly. m6D0 = home.angle.x (angle-packed arg).
- Round 4: tag-search pair + CB glues 100% (23/187 exact + 4 settling). Members: mA79 u8 (tag id), mA7C f32 (radius), mA80 cXyz (tag pos), mB90/94/98 f32 + mB9C s16 + mBAE u8 (minigame tag snapshot). daTag_So_c attested. Real-vs-comment offsets in 0xA74-0xBE4 now oracle-verified at 6 points.
- Round 5: mode-init batch 7/7 first-compile (~30/187). mA90 int, mBDB u8.
- Round 6: 4 event-mode inits + XyEventCB 100% (~35/187). PARKED: XyCheckCB needs the gameInfo+0x5cc8 (bit 0x10000... rlwinm 15,15) and +0x5bd3[i] accessors — no matched-TU spelling found (gy also Nonmatching); hunt the dComIfGp XY/tact accessor family in d_com_inf_game.h by offset math (event mgr base 0x52cc region) next session. m6D3 u8 also needed (esa-state == 2 gate).
- Round 7: modeSwimInit 100%, modeJumpInit settling-only (~37/187). Members mB00/mB08 f32; HIO m50 f32 (jump speed cap) / m68 s16 (shape pitch); l_HIO static placed (position provisional).
- Round 8: jntHitCreateHeap parked 4 rows (block-order polarity family; != form got beq, block order flipped — 3 spellings tried). Members m84C/m850/mAA8 oracle-placed. _delete PARKED pending the 0x5cda/5cd8 accessor family (same hunt as XyCheckCB).
- Round 9: modeEventEsa 100%, Mapopen settling (~42/187). PROCESS RULE (2 silent edit-losses): every python patch MUST end with a read-back assert of the new text.
- Round 10: modeEventFirst settling (~44/187). CAMERA-FAMILY PARK consolidated: modeEventBow, modeGetRupee (else-arm writes camera->mCamera work-union +0xC0/+0xD0 via dComIfGp_getCamera(1)), initCam, moveCam, XyCheckCB (0x5cc8 flags), _delete (0x5cda region) — all need the dCamera work/trim accessor decode; single hunt unblocks six functions. Non-camera remainder: modeSwim/NearSwim/Jump/Hide bodies, cut family, create chain, _execute/_draw, setAnm/setMtx/lookBack/etc.

## Rounds 11-20 (2026-08-18, successor): layout probe, cut-family blitz, new levers

- OFFSET-PROBE TECHNIQUE (round 11): temp fn taking &member into a volatile sink;
  scan the .o for addi rN,r3,imm. Dumps the real layout in ONE build. Ended the
  +-4 pad whack-a-mole (single -4 in mA94..mB0C). USE THIS FIRST for any layout drift.
- modeDisappear/modeJumpInit: pool-only. int old = mA79 (u8 member) = lbz-no-extsb
  + signed cmpw (u8 local = cmplw; s8 local = extsb).
- CUT FAMILY: 30+ fns decoded this session, most first-try exact. Recipes:
  * cutEnd idiom = dComIfGp_evmng_cutEnd(mB6C); staff idx member mB6C.
  * getPlayer(1) = Link (daPy_py_c, mDemo at 0x304: setDemoType 0x304/setParam0
    0x30C/setDemoMode 0x314; vtbl at 0x31C, slot 0x84 = setPlayerPosAndAngle(cXyz*,s16)).
  * getPlayer(2) = THE SHIP (KoRL). daShip_c::initStartPos. NULL-check it.
  * camera idiom: dComIfGp_getCamera(dComIfGp_getPlayerCameraID(0))->mCamera
    .Stop()/.SetTrimSize(1)/.Set(a,b)/.Reset(a,b)/.Start() - by value cXyz args.
  * REG map: g_regHIO child stride 0x90 (floats +0x4, shorts +0x7C):
    0x1c=REG0_F(4) 0x4b4=REG8_F(10) 0x4b8=REG8_F(11) 0x506=REG8_S(1) 0x510=REG8_S(6)
    0x6e8=REG12_F(7) 0x6f4=REG12_F(10) 0x744=REG12_S(0) 0x746=REG12_S(1) 0x748=REG12_S(2)
    0x750=REG12_S(6).
  * WW-era mDoExt_McaMorf has J3DFrameCtrl at 0x58: lbz 0x5D bit0 + lfs 0x64
    vs 0.0 == the isStop() inline.
  * substances: dComIfGp_evmng_getMyFloatP / getMyStringP ("Speed_y","SpeedF","Name").
  * zel_basic direct field: JAIZelBasic::zel_basic->field_0x00bf (0=off,1=on).
  * play.mFmapOpen = 2 written DIRECT (accessors only do 0/1).
  * mEventCut at fopNpc+0x2C4; setAttnFlag(0) = stb +0x60; setAttnPos = cXyz +0x54.
- NEW LEVER (register mirrors, NAMED locals only): decl-then-assign. 'daShip_c* ship;'
  declared BEFORE an earlier-fetched pointer flips MWCC coloring (cutMiniGameReturnStart
  7 rows -> 0). FALSIFIED against CSE-temp mirrors (p2 _execute) - see p2 round 19.
- NEW LESSON (add/sub association): target add-then-subi = the constant sits in the
  MIDDLE of the source expression: 'shape_angle.y - 0x4000 + REG12_S(6)'. A trailing
  constant folds into the REG operand instead. Found by harness sweep (10 variants).
- fneg ordering: name the negated multiplier first (f32 v = -l_HIO.m88) when the
  target loads/negates before the accumulate target.
- MWCC stack-slot laws observed: ctor-at-decl pair -> first=HIGH slot; default-declare
  both then assign -> first=LOW (moveCam exact). Full-function top-decl C89 style
  matches some fns but NOT assignment-heavy ones (operator= from call results makes
  +0xC temps that ctor-init would elide... except when it doesn't - cutEatesaFirstProc
  resisted 3 spellings, PARKED at +0x20 frame; cutSwimProc parked ~21 rows same family).
- PARKED: cutSwimProc (~21 rows, slot layout), cutEatesaFirstProc (frame +0x20),
  modeDisappearInit region settling. All logic-complete.
- REMAINING NOT DECODED: cutProc, cutMiniGameStart, cutMiniGameProc,
  cutMiniGameReturnProc, plus non-cut: _createHeap tail, checkTgHit, getMsg,
  next_msgStatus, lookBack, setAttention, setAnm family, setMtx, mode* stragglers,
  _execute/_draw, createInit/_create/ctor/_delete, XyCheckCB/XyEventCB,
  debugDraw/hudeDraw, eventOrder/checkOrder, setScale, _nodeControl, jntHitCreateHeap.

## Round 21 (session close): 72/187 exact, fuzzy 38.36% (was 40/11.66% at session start)

- cutMiniGameWarpProc EXACT (PlTurnProc sibling verbatim).
- cutJumpMapopenProc pool-only (3-stage mBBC machine; f32 v = -HIO fneg-first lever).
- setAttention + debugDraw EXACT (attention_info.position=mB60 water-clamped, eyePos=mB54;
  debugDraw = NDEBUG-stripped dead stores). hudeDraw parked 5 rows (MTXCopy arg order).
- XyCheckCB + _delete EXACT. THE CAMERA-FAMILY PARK ACCESSOR HUNT IS CLOSED:
  0x5cc8 = dComIfGp_checkPlayerStatus0(0, 0x10000)
  0x5bd3 = dComIfGp_getSelectItem(i)   (mSelectItem BASE is 0x5bd3; 0x82 = bait)
  0x5cda/0x5cd8/0x5cde = play.getMiniGameType() / endMiniGame(8) inline (^=0x80 + result=0)
  0x4951(play) = mFmapOpen (=2 written direct).
  => modeEventBow / modeGetRupee else-arm remain writable with these + initCam/moveCam (both decoded).
- mPhs = request_of_phase_process_class at REAL 0x844 (header comment drift -4 in that
  region; m84C/m850 real offsets verified by asm). m_arc_name[] = "So".
- cutEatesaFirstProc PARKED at frame +0x20 (logic complete; 3 decl spellings falsified).
- REMAINING: cutProc, cutMiniGameStart/Proc, cutMiniGameReturnProc, checkTgHit, getMsg,
  next_msgStatus, lookBack, setAnm/setAnmSwimSpeed/setMtx, mode* stragglers (Bow,
  GetRupee, Jump, Swim, NearSwim, Wait, Talk, Debug, TriForce, FirstWait...), modeProc,
  eventOrder/checkOrder, setScale, _execute/_draw, createInit/getArg tail/_create/ctor,
  _createHeap tail, jntHitCreateHeap tail, _nodeControl, XyEventCB, HIO ctor.

## Round 22: modeEventBow/modeGetRupee string-pool-only; modeJump EXACT; sqrtf lesson

- std::sqrtf carries its OWN (x > 0.0f) guard inside the inline: NEVER wrap it in a
  source-level if - the wrap emits a duplicate ble (modeJump 1-row residue). Plain
  call matches. (Retro-applied to both parked fns; they shrank but stay layout-parked.)
- const cXyz& binding on operator- results avoids the +0xC temp copy (modeJump frame
  0x40->0x30 exact).
- modeEventBow/GetRupee: sibling-copy from modeEventMapopen + the resolved accessors;
  eventBit 0x3a10; talk(1)==0x12; player(0) mDemo TYPE_SYSTEM/DEMO_N_WAIT in GetRupee.

## Round 23 (final this context): setMtx/checkOrder/setScale/eventOrder + three lessons

- setMtx EXACT: setBaseScale + transS/ZXYrotM/transM(0,mB34,0) + MTXCopy(get(), baseTR).
- checkOrder EXACT: dComIfGp_event_chkTalkXY() == the 1/2/3 mTalkButton chain (0x52b8).
- setScale real-0: chained 'scale.z = scale.y = scale.x = mB08' loads the member ONCE
  (per-component re-reads = extra lfs rows). l_HIO m44/m48/m4C; int m6CC (==15 gate).
- eventOrder decode-complete: fn-local static char* tbl[6] (SO_1ST_MEET..TRIFORCE_CHECK,
  indexed [mB70-3]); RANGE-FOLD lesson: a cached u8 local lets MWCC fold ==5||==4||==6
  into (x-4)<=1 -- per-clause MEMBER RE-READS block the fold (CSE still lands one lbz).
  Switch spelling is WORSE here (jump table).
- modeJump EXACT + std::sqrtf INTERNAL-GUARD lesson (round 22) retro-applies everywhere.
- Next context: modeSwim/modeNearSwim/modeWait/modeTalk stragglers, lookBack, setAnm
  family, getMsg/next_msgStatus, checkTgHit, cutProc/cutMiniGame* bigs, create chain,
  _execute/_draw, then REL-link SHA gate (temp-flip Matching + tc canary + tools/
  decoder .bat hygiene).

## Round 24: eventOrder + setAnm decode-complete; m_heapsize defined

- eventOrder: fn-local static tbl; RANGE-FOLD blocked by per-clause member re-reads.
- setAnm: dLib_bcks_setAnm w/ fn-local static tables (bcks {9,6,7,5,8}; prm[6]
  {i,-1,0,8.0f,1.0f,EMode_LOOP} for i in 0,0,1,2,3,4); m84C->getFrame() >=
  getEndFrame()-1.0f gate; rnd 5<->3 swap vs l_HIO.m5C. m6D2/m6D3/m6D4 = the
  bckIdx/prmIdx/oldPrmIdx trio.
- m_heapsize = 0x1C00 (const u32 class static, rodata anchor at +0; m_arc_name at +4).
- Standing counts: 75/187 exact + eventOrder/setAnm/modeEventBow/modeGetRupee/
  cutJumpMapopenProc/WarpStart etc. at settling-rows-only. Fuzzy 44%+.

## Round 25: message/joint family + the missed-watcher defect

- setAnmSwimSpeed real-0 first try (p2 rate-clamp sibling verbatim; HIO m34/m38).
- lookBack EXACT: runCheck/getAttnFlag/trnChk/setTrn/clrTrn/getTurnSpeed chain;
  dNpc_playerEyePos(l_HIO.m08); lookAtTarget(&shape_angle.y,&mB44,pos+200y,
  shape_angle.y,mB50,mBDA). bool mBDA (u8->bool killed the subic/subfe/clrlwi
  normalization — the bank's ssb lesson, second confirmation).
- getMsg decode-complete: ids 0x32CA/D0/DD/DF/E0/E1/E2, m6D0 override via mBD8;
  setMessageCountNumber(mB7C); "sea" room 13 / eventBit 0x901 gate. CONDITION-ORDER
  LESSON: target emits the ||-form else-arm FIRST — inverting to && reorders blocks.
- eventOrder/setAnm: see round 24. _nodeControl decoded, parked 10 rows (now-address
  remat vs CSE — regalloc family). setMtx/checkOrder/setScale exact (round 23).
- WATCHER DEFECT (user-caught): #115's exit-on-event completion notice was missed
  among timer ticks — ~27 min blind, 11 rows moved (all yaz0/census close-outs, no
  DECODER ask; HISTORY checked off the so milestone row). Rule: every timer tick,
  also CHECK THE NEWEST WATCHER TASK OUTPUT for an unread EXITING line.
- Standing: exact count via report; remaining majors: cutProc, cutMiniGameStart/Proc,
  cutMiniGameReturnProc, next_msgStatus, checkTgHit, modeSwim/NearSwim/Wait/Talk/
  Debug/TriForce/FirstWait stragglers, modeProc, _execute/_draw, create chain, HIO ctor.

## Round 26 (tail): modeDebugInit EXACT, modeEventTriForce string-only,
## modeEventFirstWait parked 1 row (fcmpo cror gt-vs-lt polarity family).
## DebugInit lesson: chained mAFC=speedF=0.0f; COS-x/SIN-z axis swap.
## TriForce: attention_info.flags=0x200000A; eventBit 0x3a20; modeProc(0,0xc).
## FirstWait: ship-follow + checkGetItem(0x78)/isStageBossEnemy(3) gate; HIO m54.

## Round 27: modeSwim decoded/parked — THE cXyz SLOT-CLASS FAMILY IS NOW THE BLOCKER

modeSwim parked ~25 rows; same family as cutSwimProc/cutEatesaFirstProc. Measured
target layout has THREE allocation classes (top-down): [named cXyz vars] >
[operator-call sret temps, creation order] > [flat-vector objects + f32 frsp spills].
No source spelling tried yet produces class 3 for the flats: named-var (memberwise),
ctor-at-decl, ctor-temp-in-expression, top-decl all falsified. NEXT-CONTEXT OPENER:
harness campaign over flat spellings (setall(), Vec init, dLib helper?, VECSetPoint?,
static?) on cutSwimProc (smallest member of the family, 18 real rows) — one win
retro-applies to all four. Grep how OTHER matched WW actors build XZ-flattened
distance vectors — the donor idiom likely repo-wide (check d_a_kamome, d_a_tag_so,
dLib source itself).

## Round 28 — THE SLOT-CLASS FAMILY FALLS: cXyz::absXZ()

The round-27 blocker dissolved in one stroke: the donor spells XZ-flattened distance
as **(a - b).absXZ()** (c_xyz.h line 122). The inline itself constructs the flat temp
(cXyz tmp(x, 0, z)) — that IS the mystery class-3 stack allocation. Every hand-spelled
flat (named var, memberwise, ctor-temp) fought the inline. Found by grepping the
sibling actors for the idiom (the anchor round-27 instruction, executed same context).
- modeSwim 25 real -> 0. cutSwimProc 18 -> 0 (PARK DISSOLVED; target ctor is direct
  cXyz target(mEventCut.getAttnPos()) once the frame is right).
- cutEatesaFirstProc 74 -> 3 (one fcmpu operand-order + branch-length echoes) — park.
- RULE: any frsqrte + flat-vector site — reach for absXZ()/abs()/abs2XZ() FIRST.
- so at 77/187 exact, fuzzy 53.60% (session start 40/11.66%).

## Round 29 (context close): modeNearSwim decoded/parked; fuzzy 56.17%

- modeNearSwim: BoatBattle camera SetTypeForce, player-centered circle path
  (radius 400/wobble 50/0x100, y+=mB34 both arms), exit >= mA7C -> mode 3.
  Parked 8-row ship/player r29-r30 mirror; decl-then-assign FALSIFIED on this
  pair (3 orders) - use-pattern coloring, joins the p2/ob1 inline-body-campaign class.
- CONTEXT CLOSE STATE: 77/187 exact, fuzzy 56.17% (start 40/11.66%). absXZ() rule
  in force (round 28). Remaining majors: modeProc, next_msgStatus, checkTgHit,
  cutProc, cutMiniGameStart/Proc/ReturnProc, _execute, _draw, create chain,
  _createHeap tail, HIO ctor, XyEventCB, getArg tail, jntHitCreateHeap tail.
- Monitors at close: timer b15aguq92 alive; watcher #117 (b9d14osfs) armed 303-row
  baseline. Re-arm IMMEDIATELY on delivery; CHECK NEWEST WATCHER OUTPUT each tick.

## Round 30 (this context's true tail): modeProc structurally exact + checkTgHit parked

- modeProc: 16-entry fn-local static ptmf table {init ptmf, proc ptmf, int field_0x18}
  (the UNCOPIED third int pads stride 0x18->0x1C; MWCC lazy-copies only initialized
  ptmf members under an init$ guard). All 102 remaining rows = .data placement
  settling, ZERO shape mismatches. Table order: Wait Hide Jump Swim NearSwim
  EventFirstWait EventFirst EventFirstEnd EventEsa EventMapopen EventBow Talk
  Disappear Debug GetRupee EventTriForce.
- checkTgHit: BOOL (header said void); parked ~15 ordering rows; sePos hoist
  recovered the eyePos-addr reg cache (35->15). m6D8 cooldown int; l_HIO m7C.
- History verdicts round: p2 = upstream Equivalent precedent x3 (p2-progress round 20),
  ob1 = convergence/correct. Receipt row filed. USER still owns the p2 ruling.
- Watchers cycled to #122 (bnx4hf2ro). One &-launch near-miss self-caught.
- NEXT: next_msgStatus, cutProc, cutMiniGameStart/Proc/ReturnProc, _execute, _draw,
  create chain, HIO ctor, XyEventCB, then REL SHA gate (Matching temp-flip + tc canary).

## Round 31: next_msgStatus DECODE-COMPLETE — the case-order lesson proven at scale

244 rows -> 13 by REORDERING MY CASE BODIES to the target's emission order, then
-> 1 (string pool) after two content fixes. This is the ob1 wait_action1 lesson
(case BODIES emit in SOURCE order) proven on a 20-case switch: write the cases in
whatever order the target's bodies appear, NOT in numeric order.
- Target body order: 32CA 32CB 32CC 32CE 32D0 [32CD|32D1] 32D4 32D2 32D6 32D8 32DC
  32DA 32DB 32DD 32DE [32DF|32E0] 32E1 633 32D9 default.
- gameInfo+0x48 = save.player.mPlayerItem(0x3C) + 0xC = dComIfGs_getItem(0xC) != 0xFF
  (NOT getSelectEquip — that resolves to +0xE and reads a different base).
- Minigame gate: (HIO m30 || mBAE) && getItem(0xC)!=0xFF && getTimerPtr()==NULL,
  then eventBit 0x3a10 picks 0x32DC vs 0x32D8.
- Rupee payout: dComIfGp_setItemRupeeCount(mB7C * 10).
- so now 77/187 exact, FUZZY 63.36%.

## Round 32: cutProc structurally exact — the STATIC-GUARD-PLACEMENT lesson

cutProc = 21-action dispatcher (getMyStaffId("NpcSo") -> mB6C; getMyActIdx over
a_cut_name_tbl[21]; getIsAddvance gates Start; Proc always). ptmf stride 0x18
(2 ptmfs, NO pad — modeProc's 0x1C came from its uncopied third int).
LESSON (new, high value): MWCC emits a function-local static's LAZY-INIT GUARD
AT THE DECLARATION POINT, not at first use. Declaring the ptmf table at function
top emitted the entire guarded copy BEFORE getMyStaffId (42 shape mismatches on
an otherwise length-identical fn); moving the declaration inside the
'if (mB6C != -1)' block reproduced the target placement exactly -> 0 shape
mismatches, all remaining rows are .data offsets. WHEN A GUARDED COPY LANDS IN
THE WRONG PLACE, MOVE THE DECLARATION, not the use.
so: 77/187 exact, fuzzy 67.26%.

## Round 33: THE WHOLE CUT FAMILY IS DECODED — fuzzy 78.22%

cutMiniGameStart real-0 first try; cutMiniGameReturnProc real-0; cutMiniGameProc
(2260 bytes, the TU's largest) down to 12 real rows, length-identical at 565.
- cutMiniGameProc = 8-state jump table on mB74: 0 anim/timer, 1 wait, 2 pick a
  ship-relative random spot (fwd 1000+rnd400, lateral +-1000*0.5), 3 swim to it,
  4 re-pick lateral (+-300*0.5), 5 face+wait, 6 LAUNCH (speedF = mB08*(5+rnd5);
  speed.y = 18*mB08 + 4*speedF clamped to l_HIO.m50; gravity -0.8; anim 4),
  7 airborne (Sph collide window: r = l_HIO.m3C while above water+50, else the
  sphere is PARKED at y=30000 with r=0 — the donor's way of disabling a collider;
  checkTgHit -> mB7C hit count capped 10; splash -> mB80 round count; 10 rounds
  or arrow budget out -> endMiniGame + cutEnd).
- ARROW BUDGET: mB78 increments on daPyRFlg0_ARROW_SHOOT (player mResetFlg0 0x20000000).
- ATTN TARGET: writes to 0x318 are mEventCut.setAttnPos (EventCut+0x54), NOT mAAC.
- checkTgHit returns **bool**, not BOOL (call sites: clrlwi. vs cmpwi).
- REMAINING 12 real in cutMiniGameProc: 2x fneg-fold sites (target loads +1000/+300
  and negates at RUNTIME - my literal folds to -500/-150; source likely a named
  const or macro-with-parameter, PARKED), 2x fcmpo cror polarity, 1 dead-load.
- so: 77/187 exact, FUZZY 78.22% (session start 11.66%).

## Round 34: _execute FULLY MAPPED (spec below — decode is mechanical from here)

Not yet written to source (context ended); everything needed is here.
ORDER (291 rows, r30 = .bss ANON ANCHOR "@3569", l_HIO lives at .bss+0x58 —
that is why some refs symbolize as l_HIO@ha and others as anchor+0x58):
 1. fopAcM_setCullSizeBox(this, -100*scale.x, -100*scale.x, -100*scale.x,
    100*scale.x, 100*scale.x, 100*scale.x)  [rodata 0x1d8=-100, 0x50=100]
 2. if (!dComIfGp_event_runCheck() && mObjAcch2 flag 0x20 @0x898) {
      current.pos.y = speedF = mAFC = speed.y = mAFC = 0; modeProc(0,1); return 1; }
 3. m_jnt.setParam(9 s16 from l_HIO+0xC..0x1C — SPLIT the m0C pad into 9 s16)
 4. old.angle.y (0x206) = shape_angle.y
 5. if (m6CC == 1) { if (mA7C == 0) fopAcIt_Judge(searchTagSo_CB, this);
                     else modeProc(0,3); }
    else if (mA7C == 0) fopAcIt_Judge(searchTagSo_CB, this);
 6. BLINK: if (!cLib_calcTimer(&m868)) { m86C++;
      if ((f32)m86C > (f32)mBtpAnm.getAnm()->getFrameMax()  [ptr @0x858, s16 @+8])
        { m868 = (int)(100.0f + cM_rndF(100.0f)); m86C = 0; } }
    (the s16->f32 compares use the xoris/0x4330 int->float idiom BOTH sides)
 7. setScale(); setAttention();
 8. cLib_addCalc2(&speedF, mAFC, 0.3f, 4.0f)          [0x130=0.3, 0x114=4.0]
 9. cLib_addCalc2(&mB34, mB38, mB40, mB3C)
10. lookBack(); checkOrder(); modeProc(1, 0x10); eventOrder();
11. s16 tiltTarget = 0; f32 waterY = dLib_getWaterY(...);
    if (current.pos.y < waterY) {            // underwater
       current.pos.y = waterY;
       if (mB34 > 0.0f && mRippleCb.field@0xAEC == NULL) {
          static cXyz s_ripple_scale(0.8f, 0.8f, 0.8f);   // guard @0xC + obj @0x10
                                                          // of a .bss static pair
          dComIfGp_particle_set(0x33, &current.pos, NULL, &s_ripple_scale, 0xFF,
                                &mRippleCb);
          if (mRippleCb.field@0xAEC != NULL) mAF8 = 0.0f;
       }
    } else {                                  // airborne: pick tilt by speed.y
       f32 q = mB00 * 0.25f;                 [0x1e0=0.25]
       if (speed.y < -q)      tilt = (speed.y < -(mB00*0.5f)) ? HIO+0x64 : HIO+0x66;
       else if (speed.y > q)  tilt = (speed.y > mB00*0.5f)    ? HIO+0x68 : HIO+0x6A;
       else tilt = 0;
       mRippleCb.end();
    }
12. cLib_addCalcAngleS2(&shape_angle.x, tilt, 4, 0x800)
13. if (m6CC != 1 && m6CC != 5 && !mBDB && !cLib_calcTimer(&mBE0))
      { fopAcM_posMoveF(this, NULL); mAcchCir2/mObjAcch2.CrrPos(*dComIfG_Bgsp()); }
      (0x870 is mObjAcch2; CrrPos on dBgS_Acch)
14. m84C->play(NULL,0,0); m84C->calc(); setMtx(); setAnm(6,false);
    setAnmSwimSpeed(); old.angle.y = shape_angle.y; return 0;
NEW MEMBERS TO ADD: int m868 (blink timer), s16 m86C (blink count),
  mBE0 int (post-mode timer, in the mBDF pad), mAF8 f32, l_HIO 9xs16 @0xC-0x1C
  and 4xs16 @0x64/0x66/0x68/0x6A (m68 IS the third of those four).

### Round 34 ADDENDUM — a REGRESSION TRAP caught by measuring (77 -> 39 exact)

Adding the round-34 spec's header members in one batch DROPPED the TU from
77/187 to 39/187. Cause: **f32 mAF8 inserted at 0xAF8 sits INSIDE
dPa_rippleEcallBack mRippleCb (0xAE8, size 0x14 -> ends 0xAFC)** — the insert
GREW the class and pushed mAFC/everything after it +4, silently breaking ~38
already-exact functions. Reverted (git checkout the header) and re-verified
77/187 78.22%.
RULE: 0xAF8 is INSIDE mRippleCb — the 'mAF8' the asm stores to is a FIELD OF
THE CALLBACK OBJECT (mRippleCb.field_0x10), not a new member of daNpc_So_c.
Same caution for any offset that lands inside mObjAcch2 (0x870-0xA34),
mAcchCir2 (0xA34-0xA74), mSph (0x718-0x848), mB10 (0xB10-0xB34).
PROCESS: after ANY header batch, re-run the report and compare the exact count
BEFORE committing. A header edit is the only change class in this TU that can
regress dozens of functions at once.

## Round 35: _execute WRITTEN — 277 rows, 9 real; fuzzy 81.59%

Header members added ONLY into genuine pads, with a report gate run BEFORE the
commit (77 exact held). Corrections to the round-34 spec, found by measuring:
- flag @0x898 = mObjAcch2.**ChkGroundHit()** (bit 5 / 0x20), NOT ChkWaterHit.
- 0x206 = **current.angle.y** (current = pos 0x1F8 + angle 0x204 + roomNo 0x20A),
  NOT old.angle.y.
- 0x858 = mBtpAnm + 4 = **mFrameCtrl** (mDoExt_baseAnm: vtbl 0, mFrameCtrl 0x4);
  the s16 at +8 is J3DFrameCtrl::mEnd -> the call is **mBtpAnm.getEndFrame()**.
  (mBtpAnm stays at 0x854; m850 J3DModel is unchanged.)
- m868 is **s16-cast on store** ((s16)(100.0f + cM_rndF(100.0f))) though the
  member is int -- the extsh proves the cast.
- ripple = **dComIfGp_particle_setShipTail(0x33, ...)** -- group 5 is
  dPtclGroup_ShipTail_e (the SHIP-WAKE group), not the normal group.
  mRippleCb.getEmitter()/setRate(0.0f) are the 0xAEC/0xAF8 accesses.
- The tilt multiply must be INLINE in the compare (speed.y < -(mB00 * 0.25f));
  a precomputed local reorders the loads.
PARKED (9): 5 x .bss anchor offsets (settle at convergence) + 4 x fmuls operand
order on the tilt compares (both source orders produce f0,f1; target has f1,f0).

## Round 36: _draw real-0 — fuzzy 82.89%

l_HIO m26 (debugDraw gate) / m31 (skip-model) / m2C (force hudeDraw);
m6CC==5 returns early. Body: settingTevStruct(0,&current.pos,&tevStr) ->
setLightTevColorType(model,&tevStr) -> mBtpAnm.entry(modelData, m86C) ->
m84C->entryDL() -> modelData->getMaterialTable().removeTexNoAnimator(
mBtpAnm.getBtpAnm()) -> (mA78 || l_HIO.m2C) hudeDraw().
Then dSnap_RegistFig(0x7D, this, snapPos, shape_angle.y, 1,1,1) and
dComIfGd_setShadow(mA74, 0, m84C->getModel(), &shadowPos(y+150), 800.0f,
40.0f, current.pos.y + mB34, mObjAcch2.GetGroundH(), mObjAcch2.m_gnd,
&tevStr, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex()); mA74 = u32 shadow id.
LESSON: a cXyz built from another cXyz plus one tweak is COPY-THEN-ADJUST
(cXyz p(current.pos); p.y += x) — the 3-arg component ctor reorders the loads
(cost 11 rows until flipped). Same shape as tc/bm1/ba1 sibling actors.
Includes added to the main .cpp: d/d_snap.h, d/d_drawlist.h.

## Round 37: CREATE CHAIN LANDS — CONVERGENCE FIRES: 77 -> 133 exact, 94.55% fuzzy

createInit + _create written length-identical (residual = pool settling only).
Writing them **collapsed the string/rodata pool**, and 56 functions that had been
sitting at settling-rows-only snapped to EXACT in one build. This is the
convergence effect the anchors predicted — DO NOT chase pool rows per-function;
they resolve when the TU is complete.
createInit: dual collision (mStts/mCyl dNpc_cyl_src + mStts2/mSph m_sph_src),
pos.y -= 150, setMtx, morf calc, mAAC = current.pos, anim 1, mA79 = cM_rndF(4.9);
THREE spawn routes -- sea/room13/boss3/!bit0x901 -> mode 5; sea/room4/boss7/
isCollect(0,3)/!bit0x3a20 -> mode 0xF; else attention 0x200000A + mode 1.
Tail: mBE0=30, AcchCir SetWall(30,30), ObjAcch2.Set + SetWallNone + SetRoofNone,
cullMtx = baseTRMtx, cull box +-100*scale, cullSizeFar 10, gravity -2.5,
attention distances[1]/[3]=0x22, eventInfo.setXyCheckCB/setXyEventCB (they are
dEvt_info_c methods, NOT actor methods), mEventCut.setActorInfo2(NpcSo, this).
_create: fopAcM_ct_Retail; resLoad; room-26/!bit0x1e40 and !isStageBossEnemy(3)
both return cPhs_ERROR_e; entrySolidHeap(createHeap_CB, 0x1C00); createInit.
REMAINING (54, but only TWO are undecoded): _createHeap (0.51%) and the
daNpc_So_HIO_c ctor (0.91%). Everything else is >= 98% and settles.

## Round 38 (context close): _createHeap + HIO ctor — 133/187 exact, FUZZY 97.94%

_createHeap decoded, length-identical 197, parked at an 18-row r30/r31 mirror
(decl-then-assign falsified). Res indices 0xC model / 0x10 btp / 0xD hude;
McaMorf 0x80000/0x11020022; J3DModel__create 0x80000/0x11000022; joints head 11
+ backbone 1 both take nodeControl_CB; JUT_ASSERT lines 0x1FD 0x210 0x215 0x217
0x221 (the stringBase head strings confirm each).
HIO ctor field block written (110 rows, length-identical).

### THE LAST PIECE — daNpc_So_HIO_c IS MULTIPLE-INHERITANCE (write this first next time)

The ctor stores THREE vtable words: __vt__mDoHIO_entry_c then
__vt__daNpc_So_HIO_c at offset 0x0, and __vt__dNpc_HIO_c at offset 0x4. So:
    class daNpc_So_HIO_c : public mDoHIO_entry_c, public dNpc_HIO_c { ... };
with a JntHit_HIO_c member at 0x98 (its ctor is called on this+0x98).
CONSEQUENCE: the dNpc_HIO_c subobject starts at 0x4, so ITS m04 lands at 0x8 and
ITS mMaxHeadX/mMaxBackboneX/... chain lands at 0xC-0x1E — i.e. my flat m08/m0C..m1C
fields ARE dNpc_HIO_c's inherited members, which is exactly why _execute's
m_jnt.setParam(m0E,m12,m16,m1A,m0C,m10,m14,m18,m1C) reads them in that order.
So the fix is: declare the two bases + the JntHit_HIO_c m98 member and DELETE the
flat 0x00-0x1E fields (they come from the base). Own fields start at 0x20.
This is a header restructure with real regression risk — do it with the report
gate (must stay >= 133 exact) and revert immediately if it drops.

### STATE AT CONTEXT CLOSE
so: 133/187 exact, fuzzy 97.94% (session start 40 / 11.66%).
Every function in the TU is now DECODED. What remains is convergence work:
the HIO-ctor inheritance fix above, an 18-row _createHeap mirror, a 12-row
cutMiniGameProc residue (2 fneg-fold sites + 2 fcmpo polarity + 1 dead load),
9 rows in _execute (5 .bss settling + 4 fmuls order), 15 in checkTgHit
(arg-eval ordering), and small polarity/ordering rows elsewhere.
NEXT AFTER THAT: the REL SHA gate — temp-flip so to Matching in configure.py,
build the REL, compare against the Yaz0-decompressed retail REL, and re-verify
the tc canary (c5f975667b7d...) because d_npc.h was touched this session.

## Round 39: HIO INHERITANCE FIX LANDED — 137/187 exact, FUZZY 99.37%

The round-38 finding was right and the restructure RAISED the count (133 -> 137)
instead of regressing it. daNpc_So_HIO_c : public mDoHIO_entry_c, public
dNpc_HIO_c with JntHit_HIO_c m98; the dNpc_HIO_c subobject at +0x4 supplies
m04 / mMaxHeadX / mMaxBackboneX / mMaxHeadY / mMaxBackboneY / mMinHeadX /
mMinBackboneX / mMinHeadY / mMinBackboneY / mMaxTurnStep / mMaxHeadTurnVel /
mAttnYOffset / mMaxAttnAngleY / m22 / mMaxAttnDistXZ across 0x08-0x2C. Own
fields start at 0x2C. Header gained d/d_jnt_hit.h + m_Do/m_Do_hostIO.h.
EVERY FUNCTION IN THE TU IS DECODED. All 50 remaining are >= 98%: register
mirrors and instruction ordering only, no unknown logic.

## Round 40: tc CANARY RE-VERIFIED AFTER THE d_npc.h EDITS — MATCH

d_a_npc_tc REL sha1 = c5f975667b7d9536d3e72ac7d87536fa4a622851 = the recorded
retail value. tc is ActorRel(Matching) so the REL link is REAL, not a
retail-object passthrough. The shared-header work this session (fopNpc pad
typing) is provably harmless to shared consumers.
Object-level TU status this build: tc 100.00%, p2 99.94%, ob1 99.50%, so 99.37%.

## Round 41: residue work + a REVERT-HAZARD lesson

- checkTgHit 15 -> 12: the AtType test is a SINGLE-CASE SWITCH, not an if
  (if emits bne; the target beq+b pair is the switch shape — bank lesson,
  second confirmation). Falsified and reverted: inlining &eyePos at the
  monsSeStart call (12 -> 30; the sePos hoist IS correct).
- cutMiniGameProc 12 -> 11 after restoring bool checkTgHit. Falsified and
  reverted: inverting the collision-sphere if/else arms (12 -> 28).
- **REVERT HAZARD (cost one silent loss):** a "git checkout <file>" to undo a
  bad edit ALSO reverts every unrelated good fix made to that file in the same
  uncommitted window. The bool checkTgHit fix was lost that way after the mAF8
  rollback and only resurfaced because the oracle showed cmpwi where the target
  had clrlwi. RULE: before reverting a file, diff it and re-apply the keepers —
  or commit good fixes before attempting a risky one.
- Gate: 137/187 exact, fuzzy 99.40%.
  (This very entry was ALSO eaten by the shell on first write — backticks inside
  an unquoted heredoc EXECUTE and vanish. Re-read every artifact after writing.)

## Round 42: so REL SHA GATE RUN HONESTLY — MISMATCH (as expected at 99.4%)

Procedure followed exactly (the anti-vacuous discipline):
1. configure.py temp-flipped d_a_npc_so NonMatching -> Matching, reconfigured.
2. DELETED build/GZLE01/src/d/actor/d_a_npc_so.o and rebuilt so the link could
   not reuse a stale object (verified the .o timestamp advanced).
3. TARGET HASH SOURCE FOUND: config/GZLE01/build.sha1 is the authoritative
   per-REL hash list (there is NO d_a_npc_so.rel in orig/GZLE01/files/rels --
   only 235 rels are extracted and so is not among them; RELS.arc holds the rest).
   Target: dad717a5da27e7c168e18fb5af8bbd85b8c71200
   Built:  4d81f104ed1c1ad75166be4bb60f0b7f03f94445
   VERDICT: MISMATCH. Correct and expected -- 50 functions still carry ordering
   and register-mirror residues. NOT a regression, NOT a defect.
4. configure.py RESTORED to NonMatching and re-verified (leaving it Matching is
   the vacuous-match trap that produced this lane's 12/12 over-claim).
USE build.sha1 FOR EVERY FUTURE GATE -- no Yaz0 decompression needed, and it
covers RELs that were never extracted to files/rels.

## Round 43: _createHeap probe — bool jntHitCreateHeap FALSIFIED (net negative)

_createHeap's tail has a clrlwi r3,r3,24 before the subic/subfe BOOL
normalization, which reads as "callee returns bool". Flipping
jntHitCreateHeap to bool DID fix that row (_createHeap 18 -> 17) but broke
jntHitCreateHeap itself (0 -> 7 rows): net worse, fuzzy 99.40 -> 99.37.
REVERTED and re-verified at 137/187, 99.40%.
=> jntHitCreateHeap genuinely returns BOOL; the narrowing in _createHeap comes
from the CALL SITE's conversion, not the callee's type. Whatever spelling
produces it must narrow at the call (a u8/bool temp), without changing the
callee signature. Untried and cheap next time: assign to a local bool/u8 first,
or compare != 0 through a u8 cast.
STANDING: 137/187 exact, fuzzy 99.40%. _createHeap residue is 17 rows, of which
16 are the r30/r31 mirror (decl-then-assign already falsified) and 1 is this
narrowing.

## Round 44: the narrowing was at the CALL SITE — (u8) cast lands it

Round 43 predicted this and it holds: 'return (u8)jntHitCreateHeap() ? TRUE : FALSE'
gives _createHeap 18 -> 17 WITHOUT touching the callee signature (the callee
flip cost jntHitCreateHeap 7 rows; a 'bool ok =' temp cost 3 more at 20).
LESSON: when a caller shows clrlwi-before-normalize, narrow AT THE CALL with a
cast; do not change the callee's return type to chase it.
Gate: 137/187 exact, fuzzy 99.41%. _createHeap residue is now 16 mirror rows + 1.

## Round 45: cutEatesaFirstProc down to 3; the residue is now MWCC-CANONICAL shapes

cutEatesaFirstProc: 31 total / 3 real, and the 3 are one fcmpu operand order
plus its two branch-distance echoes. BOTH source orders (spd != 0.0f and
0.0f != spd) emit the SAME f0,f31 — MWCC canonicalizes the compare, so this row
is NOT reachable from operand order. Same conclusion as the p2 mirror class.
LOWEST-PERCENT SURVIVORS (the real work list, all decoded, all shape-only):
  hudeDraw 85.86 | _nodeControl 89.41 | checkTgHit 90.73 |
  jntHitCreateHeap 95.72 | setAnm 98.20
These five are where the remaining bytes are concentrated; the big functions are
all >= 98%. Next context should start there rather than re-walking the bigs.
STANDING: 137/187 exact, fuzzy 99.41%, tree clean, configure.py NonMatching.

## Round 46: TWO NEW ARG-ORDER LEVERS (hudeDraw EXACT, _nodeControl 13->8); 99.56%

The residue class is ARGUMENT EVALUATION ORDER and it IS reachable — two
distinct levers found, both verified by the gate:
 1. **Hoist the OTHER argument.** hudeDraw: MTXCopy(get(), m850->getBaseTRMtx())
    evaluated the member load first; the target wants "now" first. Hoisting
    get() into a local did NOT work (5 rows) and mDoMtx_copy did NOT work (5).
    Hoisting the MEMBER — "J3DModel* model = m850;" then
    MTXCopy(get(), model->getBaseTRMtx()) — gave 5 -> 0 EXACT. Simplifying an
    argument releases its claim on the early evaluation slot.
 2. **Drop the stack wrapper for the explicit form.** _nodeControl:
    mDoMtx_stack_c::YrotM(angle) evaluated the angle load before materializing
    "now"; mDoMtx_YrotM(mDoMtx_stack_c::now, angle) puts "now" first — 13 -> 8.
    Hoisting the angles into s16 locals was WORSE (16); converting the other
    get() calls to ::now was inert.
FALSIFIED on checkTgHit (both reverted, 12 -> 17 each): hoisting fopAcM_GetID
into a local, and hoisting dComIfGp_getReverb into an s8. Its 12 rows are the
zel_basic/seNum/sePos block ordering vs the GetID null-check, and neither
hoist direction moves it.
STANDING: 138/187 exact, fuzzy 99.56%. Work list now: checkTgHit (12, resistant),
jntHitCreateHeap, setAnm, _createHeap (17, 16 of them the r30/r31 mirror).

## Round 47: CROSS-CHECKS RUN ACROSS ALL TUs (History's request) — 4 pass, 1 UNVERIFIABLE

1. FULL GZLE01 BUILD: passes. No Matching TU broke from this session's edits.
   (Modules 73.41% fuzzy / TWW Game Code 75.19% / Core Engine 99.92%.)
2. MATCHING-LABEL AUDIT, the real regression test: cross-referenced every
   plain-Matching entry in configure.py against the objdiff report.
   **753 configured Matching TUs, ZERO below 100%.** The d_npc.h fopNpc pad
   typing is provably harmless project-wide, not just to the tc canary.
3. d_npc.h CONSUMER FAMILY: 59 NPC TUs, 27 at 100%; the rest are undecoded
   (1-3% — never worked on), not regressions.
4. d_a_npc_so is GZLE01-ONLY (single ActorRel entry) so its header/VERSION_DEMO
   guard edits have no cross-version exposure.
5. **UNVERIFIABLE, AND THIS ONE MATTERS: the workspace builds NOTHING but
   GZLE01.** `ninja -t targets all` lists **0** source objects under
   build/D44J01, build/GZLJ01 or build/GZLP01 — the directories exist but no
   compile edges are wired. CONSEQUENCE: any
   `MatchingFor("GZLJ01","GZLE01","GZLP01")` label in configure.py CANNOT be
   verified in this workspace as configured. Those labels (msdan2 / msdan_sub2 /
   hami2 from an earlier session) are asserted, not measured, here.
MEASUREMENT-ERROR NOTE: my first cross-version probe reported "errors: 1" for
all three versions. That was MY grep counting ninja's own "unknown target"
message, not a compile failure. Verify a target EXISTS before treating its
absence as a defect.

## Round 48: jntHitCreateHeap + setAnm residues characterised (both source-inexpressible)

jntHitCreateHeap (4 rows): the TRUE/FALSE emission blocks are ordered opposite
to the target. THREE spellings all give the same 4 rows — if/else on != NULL,
if/else on == NULL (flips beq->bne, no help), early-return, and init-FALSE-then-if.
The target emits beq -> [jntHit store] -> b, with li 0 BEFORE li 1, a block order
no source arrangement reproduced. PARKED.
setAnm (4 rows): 2 are pool literals and 2 are the m_arc_name reference —
the TARGET reaches it as "addi r3, r31, 0x4" (ANCHOR-RELATIVE off m_heapsize,
the rodata-anchor lesson) while mine emits a separate lis/addi symbol pair.
That is pool/anchor placement, expected to settle at full convergence, not a
source defect.
STANDING: 138/187 exact, fuzzy 99.56%, gate held, tree clean.
REMAINING RESIDUE INVENTORY (all decoded, all shape-only):
  _createHeap 17 (16 = r30/r31 mirror) | checkTgHit 12 (arg-block ordering) |
  cutMiniGameProc 11 (2 fneg-fold sites + polarity) | _execute 9 (5 bss + 4 fmuls) |
  _nodeControl 8 | jntHitCreateHeap 4 | setAnm 4 | cutEatesaFirstProc 3 |
  plus small pool tails.

## Round 49: .bss ALIGNMENT METHOD (proven on p2) applied to so — divergence located

METHOD (reusable, ~20 s per TU, proven by taking p2's nodeCallBack 16 rows -> 0):
  1. Parse the target .s for "# .bss:<off> | .. | size: <sz>" + the following .obj name.
  2. Dump mine: build/binutils/powerpc-eabi-objdump.exe -t <o> | grep .bss (drop size-0).
  3. Sort both by offset and walk them together; the first (off,size) mismatch names
     the pair to reorder. MWCC emits .bss in SOURCE-DECLARATION order, so the fix is
     moving a declaration, never changing code.
so RESULT: 24 objects both sides, identical count. First divergence at index 20:
  TARGET  0x011C init$4529 (1) | 0x011D init$6629 (1) | 0x0120 "@6744" (12)
  MINE    0x011C init$2731 (1) | 0x0120 "@2837" (12)  | 0x012C init$2838 (1)
=> The target packs the TWO ptmf-table guards (cutProc's cut_tbl and modeProc's
mode_tbl) ADJACENTLY at 0x11C/0x11D with no object between them; mine puts a
12-byte cXyz static in between. So in the donor the two tables are encountered
back-to-back, while in my file _execute's s_ripple_scale (and its guard) lands
between modeProc (main .cpp) and cutProc (in d_a_npc_so_cut.inc, included late).
CONSTRAINT: the .inc include position is pinned by TEXT order (the cut functions
must follow daNpc_SoIsDelete), so this cannot be fixed by moving the include.
The likely donor shape is that s_ripple_scale is NOT a function-local static in
_execute — worth re-testing as a file-scope static or a different construct.
Deferred: worth 1-2 rows, and the method is now recorded for the next context.

## Round 50: so's 627 rows CLASSIFIED — the gap is DATA PLACEMENT, not code

  pool/literal-offset rows : 317
  register-mirror          :  33
  other                    : 277
TOP FUNCTIONS: cutProc 134 | modeProc 102 | cutMiniGameProc 54 |
cutEatesaFirstProc 31 | modeNearSwim 28 | _execute 20 | modeSwim 19 |
_createHeap 17.
**cutProc (134) + modeProc (102) = 236 rows, more than a third of the total,
and BOTH were verified STRUCTURALLY EXACT earlier (shape-mismatch 0).** Their
rows are entirely .data offsets for the two ptmf dispatch tables — they resolve
when the TU's .data/.rodata pool converges, exactly like the string-pool rows
that snapped 56 functions to exact in round 37.
=> so is NOT 49 functions away from done. It is ~2 structural clusters plus
pool placement. The honest read: **d_a_npc_so is decode-complete and in
placement convergence**, which is a different (and much better) state than the
raw 138/187 number suggests.
ob1 .bss checked with the round-49 method: 25 objects both sides, NO
DIVERGENCE — already aligned, which is why it leads at 99.807%.

## Round 51: .data ALIGNMENT — 25 extra objects located; SQRTF_CONST_LITERALS FALSIFIED for so

Applied the round-49 alignment method to .data (note: the target .s writes a
bare `.data` directive, NOT `.section .data` — the round-49 regex must be
relaxed or it silently reports zero objects, which it did on the first run).
RESULT: **target 88 .data objects, mine 113 — 25 EXTRA on my side.**
First divergence at index 3:
  TARGET  0x0030 sph_offset$4466 (12) | 0x003C cyl_offset_A$4467 (24)
  MINE    0x0020 _three$localstatic4$sqrt (8) | 0x0028 _half$localstatic3$sqrtf (8)
So my build emits the MSL sqrt inline's `_half`/`_three` local statics into
.data, and is MISSING sph_offset / cyl_offset_A at that position.
HYPOTHESIS TESTED AND FALSIFIED: that this is the SQRTF_CONST_LITERALS vintage
flag (ob1 carries it: `extra_cflags=["-DSQRTF_CONST_LITERALS"]` plus
`#include "d/dolzel_rel_lit.h"`).
  - added the flag alone      -> localstatic count still 2, metrics identical
  - added flag + lit header   -> localstatic count still 2, metrics identical
Both REVERTED (dead config is worse than none). The earlier session's note that
"so does NOT need the vintage flag" stands, but for a NEW reason: so now HAS
sqrtf sites (I added absXZ) and the flag still does not move them.
NEXT LEAD (untested): the missing sph_offset / cyl_offset_A suggests
jntHitCreateHeap's static tables differ in SHAPE from the donor's — the donor
has a 12-byte sph_offset plus a 24-byte cyl_offset_A that my source does not
declare at all. Compare jntHitCreateHeap's static table set against the target
.data names before chasing the sqrt statics further.

## Round 52: CORRECTION — so's .data IS aligned; the "25 extra objects" was a METHOD ARTIFACT

Round 51 reported "target 88 .data objects, mine 113 — 25 EXTRA" and blamed the
MSL sqrt statics. **The target HAS those statics.** Reading the raw target .s at
0x1C-0x30:
    .obj "@1811" ... .endobj      (ends 0x1C)
    .4byte 0x00000000
    .4byte 0x40080000 / 0x00000000   <- double 3.0  == _three
    .4byte 0x3FE00000 / 0x00000000   <- double 0.5  == _half
    # .data:0x30 -> sph_offset$4466
They are emitted as RAW DATA with NO `.obj` wrapper, so a named-object walk does
not see them. Mine emits the same values at the same offsets (0x20/0x28) but AS
NAMED SYMBOLS (_three$localstatic4 / _half$localstatic3).
=> **so's .data is ALIGNED from 0x30 onward and the 0x20/0x28 contents match.**
The object-count gap is symbol GRANULARITY, not layout.
METHOD CAVEAT (add to the round-49/51 instrument): the target .s emits some data
WITHOUT .obj wrappers, so a named-object alignment walk can report FALSE
divergences. Always read the raw bytes around a reported divergence before
acting on it. Corollary: the SQRTF_CONST_LITERALS hypothesis was falsified twice
over — the flag did nothing AND there was nothing to fix.
STILL TRUE AND VALUABLE from round 51: sph_offset/cyl_offset_A really were
missing dead statics, and adding them took so 138 -> 139 exact / 99.585%.

## Round 53: THE DEAD-STATIC FIX CASCADED — 627 rows -> 392, modeProc EXACT

Adding the donor's two unreferenced statics (round 51) did far more than the
+1 exact function it showed at the time. Re-measured after the .data realigned:
  **modeProc: 102 rows -> 0 (EXACT)**
  **cutProc:  134 rows -> 1** (one string-pool row, addi 0x13f vs 0x1a9)
  TOTAL so diff rows: **627 -> 392**
Both ptmf dispatch tables were structurally exact all along; their 236 rows were
purely .data placement, and placing sph_offset/cyl_offset_A correctly shifted
the whole section into the donor's layout. This is the SAME cascade shape as
round 37 (the create chain collapsing the string pool and snapping 56 functions
to exact) — **fixing a placement root cause pays out across every consumer at
once, which is why per-function row-chasing is the wrong order of work.**
CURRENT TOP ROWS: cutMiniGameProc 54 | cutEatesaFirstProc 31 | modeNearSwim 28 |
_execute 20 | modeSwim 19 | _createHeap 17 | checkTgHit 17 | createInit 12.
so: 139/187 exact, fuzzy 99.585%.


## Rounds 54-58 - the placement cascade completes (139 -> 153 exact)

Four placement root causes fell in sequence. Every one was found by an
ALIGNMENT WALK (parse the target .s for the section's object list, dump mine
with objdump, walk both and name the FIRST divergence) rather than by reading
diff rows.

**Round 54 - .rodata head.** The target's .rodata STARTS with the class
statics (m_heapsize@0x0, m_arc_name@0x4, m_sph_src@0x8) and only then the
float pool at 0x48; mine started with floats. Moving the three definitions to
the file top took 392 -> 294 rows. Moving them ABOVE the HIO ctor as well then
matched the head byte-for-byte (99.622 -> 99.631). NOTE THE FAILED VARIANT:
inserting after `s.rfind('#include')` put them in the MIDDLE of the file,
because the cut .inc include is itself an #include near the bottom - that
experiment measured a regression that was never the hypothesis's fault.

**Round 55 - alignment walks must compare VALUES, not (offset,size).** My
first .rodata walk reported "aligned through idx 28" while the values had
already diverged at 0xAC. Sizes agree trivially when everything is a 4-byte
float. Compare the emitted words.

**Round 56 - the cut block compiles before getMsg.** The STRING pool put the
21 cut names (SWIM/JUMP/APPEAR/...) at index 7, immediately after the assert
strings and BEFORE "sea"/"BoatBattle"/the SO_* event names. Mine emitted them
at 16. Moving `#include "d_a_npc_so_cut.inc"` to just before getMsg: 139 ->
141 exact. The opposite move (include at end of file, which the target's .text
ORDER appeared to argue for) measured WORSE (138) - the target's tail
functions live in separate `.text unique` sections, so their listed order is
not source order. **The string pool is the reliable order oracle here; the .s
function listing is not.**

**Round 57 - the donor reuses `modelData`, and that was worth 12 functions.**
The target's string pool has "modelData != 0", "btp != 0" and both m_jnt
asserts but NO "hudeData != 0". The donor does not declare a second local for
the 0xD hude model - it REASSIGNS modelData, so the assert string dedups
against the one already in the pool. Making that one change: **141 -> 153
exact**, string pool aligned end to end, and it is also the root of
_createHeap's r30/r31 colouring swap (one fewer live local).
*Lesson: a string the target does NOT have is as much evidence as one it does.*

**Round 58 - the missing debug table.** Target had 53 strings, mine 42. The 11
absent ones (HIDE, NEAR_SWIM, EVENT_FIRST_WAIT, EVENT_FIRST, EVENT_FIRST_END,
EVENT_ESA, EVENT_MAPOPEN, EVENT_BOW, DEBUG, GETRUPEE, EVENT_TRIFORCE) are
exactly the members of a 16-entry mode-name table in modeProc order once the
five already present (WAIT/JUMP/SWIM/TALK/DISAPPEAR) dedup. Restored as a
fn-local static in debugDraw (which is otherwise a stub of dead locals).
String pool now **53/53**. Row count did not move, but the .rodata size is now
the donor's.

STATE: so **153/187 exact, fuzzy 99.633%**, 34 fns / 194 rows (was 48/294).
Cross-check after the round: p2 133/145 and ob1 108/115 both UNCHANGED - all
edits were so-local, no shared header touched.

TWO FALSIFIED HYPOTHESES, recorded so they are not retried:
- `checkTgHit`: inlining `&eyePos` to move its materialisation after the
  fopAcM_GetID inline made it 14 -> 32 rows. The donor DOES keep the named
  local (it saves r26-r31 via _savegpr_26; the inlined form only needs
  _savegpr_27). What differs is only WHERE r28 is assigned - still open.
- `jntHitCreateHeap`: hoisting `BOOL ret = TRUE` to the declaration changed
  the branch shape but stayed at 4 rows. The target emits then-block -> b ->
  else(li 0) -> b -> join(li 1), i.e. BOTH arms branch to a join that
  materialises TRUE. Not yet reproduced.

CURRENT TOP ROWS: cutMiniGameProc 31 | cutEatesaFirstProc 16 | modeNearSwim 15
| checkTgHit 14 | _createHeap 12 | _execute 11 | HIO ctor 9 | modeSwim 8.
cutMiniGameProc still has REAL missing code (three `fneg`/`fmuls` pairs the
target has and mine lacks) - that is the next substantive target, not a
placement artifact. _execute's remaining rows are float-pool positions
(divergence still at 0xAC, where the donor emits 7.0 + the two inline-sqrt
doubles that mine places at 0x150/0x158).
