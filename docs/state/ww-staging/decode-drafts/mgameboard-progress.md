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

## NEXT

_execute (state dispatch over mState), set_2dposition, CreateInit,
set_mtx (0x304 bytes), CursorMove, MinigameMain, _draw, CreateHeap
(0x6B0, model/DList construction against m_bullet_table), then dtors/
sinit heal.
