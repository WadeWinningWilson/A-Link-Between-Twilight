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
