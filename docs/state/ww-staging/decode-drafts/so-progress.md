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
