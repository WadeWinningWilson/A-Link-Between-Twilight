# d_a_mgameboard (Squid-Hunt board) decode campaign — anchor

Opened 2026-08-22 by the DECODER session that closed kg1 the same day.
Picked over the marginally smaller d_a_fallrock_tag because mgameboard is
kg1's DIRECT PARTNER — its public interface (reqStartGame / checkEndGame /
checkClearGame / getScore / set-clrGInfoDraw / mbForceEnd 0x63F) was
already proven from the caller side during the kg1 campaign.

## Baseline at open

- 2/23 exact (header-inline accessors), unit fuzzy 1.21.
- Target .o: build/GZLE01/d_a_mgameboard/obj/d/actor/d_a_mgameboard.o;
  scratch disasm D:\XXXXXXX\WW DP\mgb_tgt.s (untracked).
- Stub header already carved (someone did census): dSeaFightGame_info_c
  subsystem at 0x47C, STControl 0x58C, dDlst_2D* display lists.
- REPORT TRAP: objdiff has FOUR units matching "mgameboard"; the real one
  is `d_a_mgameboard/d/actor/d_a_mgameboard` — `framework/d/
  d_a_mgameboard_static` (the 6 accessors) reads 100 and lies about
  campaign state.

## Batch 1: 2 -> 8/23 exact (WWDP ddeb3910)

- CheckCreateHeap / execGameMain / execEndGame / daMgBoard_Delete 100;
  daMgBoard_Create 98.5 (stringBase offset pending TU fill),
  MiniGameInit instruction-exact.
- **m_arcname is a WRITABLE static char[9] in .data, strcpy'd with
  "Kaisen_e" inside Create** (not a const arcname like every NPC so far);
  m_bullet_num = 24 (.data u8); m_bullet_table u8[0x120] bss.
- NO daMgBoard ctor symbol: the implicit ctor inlines into Create's
  SetupActor, and the visible setWaitParm(15,15,0,0,0.9,0.5,0,0x2000)+
  init() is just STControl's DEFAULT ctor (d_lib.h) — no user ctor.
- execGameMain returns bool (header said void): game over when
  mSeaFightGame.mAliveShipNum == 0 || mBulletNum == 0 -> mbEndGame=1.
- MiniGameInit: chained assigns (mBoardPosX = mBoardPosY = 0 stores
  Y-then-X), seafight init(m_bullet_num, 3), finds Salvatore via
  fpcNm_NPC_KG1_e (0x16C) + fopAcIt_Judge, mNPCPos = npc pos,
  icon resets via dDlst_2DObject_c::onBeforeTex().
- **LEVER: 2D-vs-flat array tell** — [3][8] spelled as [i][j] hoists a
  row base (add rN,this,rowoff); the target's add-then-slwi flat index
  proves `mpBombIcons[24]` + `[i * 8 + j]` in source.
- CreateHeap retyped BOOL (CheckCreateHeap passes its return through).

## Batch 2 (written, build pending — header edits triggered world rebuilds)

- _execute 100 after hoisting the two register reads into locals
  (`u8 best/score` before the two mValue stores — target loads both
  before either store); state machine 0->1(wait mbStartGame)->2->3
  (execGameMain until mbEndGame, timer 30)->4 (timer, then END_S/END_F
  jingle by mAliveShipNum) with case-2-falls-into-3.
- CreateInit: cullMtx = board getBaseTRMtx, cull box -600/-300/-500..
  600/300/100, getEventIdx MINIGAME_START/END, setWaitParm(5,2,3,2,
  0.9,0.5,0,0x800), MiniGameInit.
- set_2dposition 91->pending: squid column x=523 y=115+47i; bombs
  ACCUMULATOR form (x=95 -=35/col, y=120 +=35/row, y reset per column —
  a multiply spelling `95-i*35` misses); title/score positions;
  **LEVER: setScoreAlpha's f32->u8 conversion happens INSIDE the class
  inline in the donor (runtime fctiwz on 80.5f) — MWCC folds a constant
  conversion at the CALL boundary but NOT through an inline param, so
  the donor's param type is f32** (WWDP header said u8 + "???", fixed).
- CursorMove: STControl trigger quad, s8 clamps 0..7 (mBoardPosX/Y
  retyped s8), CURSOR se on change with NULL pos.
- MinigameMain (bool, was void): checkSePlaying(START) early-out,
  attack(), RIGHT/WRONG/DESTROY jingles, StartShock(7,-33,cXyz(0,1,0)),
  icon flips mpSquidIcon[mDeadShipNum-1] / mpBombIcons[mScore-1]
  offBeforeTex; mShips base folds +8 (reads field_0x8/0xb/0xc/0xe of
  dSeaFightGame_ship_data; 0xe retyped s8).
- set_mtx: board+cursor+64-cell hit/miss placement over static
  cXyz m_cur_table[8][8] (bss 0x300; grid [y][x] for cursor, [i][j]
  transposed vs mGrid[j][i] in the cell loop), ship placement with
  ZrotM(0x4000 flat / -0x8000 vertical), mpShip4Model retyped [2].
- _draw: board always; gated by mbDraw: cursor + hit/miss loops under
  dComIfGd_setListMaskOff()/setList() buffer swaps (drawlist +0x28/2C =
  MaskOff pair, +0x1C/20 = normal pair — the j3dSys+0x48/4C store-pair
  tell), ships drawn only when game over (value-form bool via cntlzw),
  2D packets via dComIfGd_set2DOpa (minigame, numbers, 24 bombs,
  3 squids).
- CreateHeap: res 8 board / 9 cursor / 7 hit x20 / 10 miss x32 /
  4,5,6 ships x2 each (all mDoExt_J3DModel__create(0x80000,0x11000022),
  every slot null-checked -> return 0; asserts 307-411 "modelData !=
  0"); 2DNumber init(4,470,75,24,24,0)+(4,100,80,24,24,0); 2DObject
  news are vtbl-inline (implicit ctor), TIMG init pairs spelled
  (18,19)/(15,16)/(14,17) — **MWCC evaluates call args right-to-left,
  so the SECOND source arg's getRes lands first in the binary**.

## Batch 3: 17/23 exact, fuzzy 78.78 - CLOSED TO PARKS (WWDP ff1a25fe)

- Verified via DIRECT-mwcc scratch compiles while ninja world-rebuilds
  held the lock (header edits to d_2dnumber/d_seafightgame trigger
  repo-wide rebuilds - budget for that before touching shared headers).
- CursorMove/_execute/CreateInit/MiniGameInit/set_2dposition exact;
  set_mtx 99.8 / MinigameMain 99.9 / _draw 97.9 / CreateHeap 95.3
  (register-naming + pool residues).
- LEVERS: (a) **bitwise-| bool** - `if ((A == 0) | (B == 0)) over =
  true;` emits the cntlzw+or value form; spelling it || emits
  short-circuit branches (ji1 family, now proven on u8 fields);
  (b) **f32-local anti-fold** - `f32 alpha = 80.5f;` then pass defeats
  MWCC float const-prop (a LITERAL folds even through an inline f32
  param - my earlier inline-boundary theory was WRONG, corrected here);
  (c) hoisted declarations set saved-reg order (mark/ship/shipNum
  declared before the loops = kg1 split-decl lever generalized);
  (d) hoist repeated record reads into locals (ship sx/sy) when the
  target loads them once.
- **m_cur_table = full 64-entry brace initializer**, cXyz(x, y, 0) with
  x,y = -87.5 + 25*index (col fastest, y ascends by row) - recovered
  from the 5KB sinit (64 stack temps live simultaneously, per-element
  dtor registration).
- **PARK (novel harness shape): sinit copy-call.** Target CALLS
  __ct__4cXyzFRC4cXyz 64x (temp built inline from pool floats, then
  copy ctor CALLED into the element; the TU even emits the out-of-line
  copy ctor). Falsified: cXyz(f,f,f) elements elide under GC 1.0/1.1/
  1.2.5/1.2.5n/1.3.2 alike; removing the repo's #ifdef __MWERKS__
  explicit copy ctor changes nothing (0 calls); cXyz(cXyz(...)) gives
  128 calls (2/element). No spelling found that yields exactly one
  un-elided copy per element. Costs sinit (55.98) + the phantom
  __ct__4cXyzFRC4cXyz symbol; everything else in the TU unaffected.
- Dead-pool tell: target rodata carries 0.8/0.75/-0.75 with ZERO .text
  references (DCE'd donor code still occupying @-slots) - unreachable
  by any spelling; absorbed as pool residue.

## NEXT

Campaign closed to parks. Open items: (1) the sinit copy-call park
(batch 3) - blocks __sinit + the phantom __ct__4cXyzFRC4cXyz; (2)
register/pool residues on set_mtx/MinigameMain/_draw/CreateHeap
(95-99.9); (3) m_bullet_table u8[0x120] is declared but its writer was
never seen in this TU - likely consumed by d_seafightgame.cpp (check
when that TU opens). Next campaign: pick the next smallest-open TU
(d_a_fallrock_tag 7 fns was the runner-up).
