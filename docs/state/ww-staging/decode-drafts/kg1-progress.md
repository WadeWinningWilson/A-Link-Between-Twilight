# kg1 (Orca) decode campaign — anchor

Opened 2026-08-22 by the successor DECODER session that closed yw1 to
parks (109/119, fuzzy 98.71) the same day. kg1 = **Salvatore** (Squid-Hunt
/ Sploosh-Kaboom minigame host, WINDFALL — the TU comment rules it).
**CORRECTION: my open-note called this Orca/Outset — wrong on both; the
Outset-first charter did NOT select this TU, smallest-open-TU did.**

## Baseline at open

- 2/66 exact, fuzzy 1.31 (template stubs only; 44 Nonmatching markers
  incl. ctor).
- Target .o: build/GZLE01/d_a_npc_kg1/obj/d/actor/d_a_npc_kg1.o
  (3,303 disasm lines).

## Census facts (read from the target ctor before any body)

- **Base class: fopNpc_npc_c** (ctor chain: fopAc_ac_c → vt fopNpc_npc_c
  → dBgS_Acch/ObjAcch → dBgS_AcchCir → cCcD_Stts/dCcD_GStts) — the
  ym1/yw1 family; the whole session lever bank applies.
- **Fuller collision set than yw1**: dCcD_GObjInf + cM3dGAab + cM3dGCyl
  vtables in the ctor — Orca hosts the spear-training minigame (hit
  counting), expect dCcD sphere/cyl attack shapes and hit callbacks.
- **HIO derives dNpc_HIO_c** (not mDoHIO_entry_c like yw1) —
  `__ct__15daNpc_Kg1_HIO_cFv` calls `__ct__10dNpc_HIO_cFv`. Check ba1/so
  for the dNpc_HIO_c template before writing it.

## Levers to apply up front (minted ym1/yw1, all proven this session)

1. HIO ctor single-iteration-loop idiom (if prm table present).
2. btp/bck resID single-entry-table trap — check table sizes in rodata
   symbols FIRST.
3. OR-guard return for any bne+b guard pair; else-return placement.
4. Case blocks emit in SOURCE order; empty cases steer the pivot.
5. Inlines carry branches (fpcM_GetID-class) — read the callee first.
6. bool vs u8 vs BOOL: srwi=bool field · rlwinm27,24,31=u8 value ·
   clrlwi join = bool chain end-to-end · no-normalize return = bool.
7. anmMtx pair-cache: local for adjacent multVec pairs, inline single.
8. Reverse-declaration stack slots for same-kind named locals.
9. Write pool NEIGHBORS when a 99.9x cluster shares stringBase/floats.

## Batch 1: 2 -> 6/66 exact

- Base-class correction applied (header claimed fopAc_ac_c; ctor asm
  rules fopNpc_npc_c — members at 0x6C4 confirm). Return-type clashes
  with the base virtuals (getMsg u32, next_msgStatus u16) fixed in the
  same pass — the stub header had them void.
- HIO ctor EXACT: daNpc_Kg1_HIO_c = vtbl + s8/-1 + s32/-1 + u8 +
  **dNpc_HIO_c mNpc[1]** (0x28 elem, __construct_array tell — an ARRAY
  member emits construct_array even at count 1) + 2 trailing u8. Values:
  head ±2500/±7000, backbone ±2000/±8000, turn 1000/2000, attnY 35.0f,
  attnAngle 0x4000, attnDist 400.0f. Dtor chain snapped with it (6/66).

## Batch 2: 6 -> 7/66 exact (fuzzy 12.2)

- nodeCallBack 95.8 (pool-only residue): one big static (no yw1-style
  _nodeCB split). Head branch: rot pair FIRST, then MID-BLOCK statics
  l_offsetAttPos(24,5,0)/l_offsetEyePos(24,-16,0) (donor names from bss),
  multVec to m734/m740 + attention position (+= l_HIO.mNpc[0].mAttnYOffset),
  the rot pair REPEATED between multVecs (donor spelling, ported as-is).
  Joint 8 attaches m6C4 (Salvatore's prop model) at transM(23.46, -22.26,
  -47.05) + XYZrotM(8011, -20224, 8011).
- Class statics m_camera_ctr/m_camera_eye (bss-named) defined; l_HIO
  declared after l_cyl_src. Actor ctor is the IMPLICIT default — do NOT
  write one; it snaps at _create's first new.
- LEVER (1-row): a guarded static whose init code sits AFTER other
  statements = the donor declared it MID-BLOCK; hoisting to block top
  reorders the guard ahead of those statements and misses.

## Batches 3-4: 7 -> 10/66 exact (fuzzy 25.3)

- lookBack 99.8 / chkAttention 97.2 / nodeCallBack 95.9 (pool+slot
  residues) + set_mtx/playTexPatternAnm/initTexPatternAnm(89) written.
- LEVERS: (1) kg1 reads the player via **dComIfGp_getPlayer(0)**
  (mPlayerInfo[0], gameInfo+0x5B44), NOT getLinkPlayer (getPlayerPtr(0),
  +0x5B4C) - 8-byte offset tell distinguishes them. (2) byval cXyz arg
  passed DIRECTLY (eyePos) - a named copy first doubles the stores.
  (3) named RODATA symbols are donor spellings: m_arcname[3]=Kg +
  m_camera_fovy=40.0f are CLASS statics, l_btp_ix_tbl={9,B,D,C} is a
  file static - an anonymous-pool table means NEITHER (const local).
- Parks: chkAttention slot-interleave (kari family), pools pending fill.

## Batch 5: fuzzy 25.3 -> 36.95 (10/66 exact + 4 fns at 99.7-100 pool-pending)

- CreateHeap 99.67: morf from res 5 with an ANM TRANSFORM arg (res 17,
  playMode 2 — unlike the ym1/yw1 NULL+(-1) form); joints head/backbone2
  via m_jnt.setHead/BackboneJntNum (asserts 547/552);
  initTexPatternAnm(3,false) gated + (0,true) ungated; prop model m6C4
  res 6; mBtpAnm.init(res 6, res 10, loop 2); u16 LOOP over
  getJointNum() attaching nodeCallBack to head/backbone2/joint-8;
  Acch.Set passes &current.angle + &shape_angle (yw1 passed NULLs).
- CreateInit 99.95 (returns cPhs_COMPLEATE_e): Stts/Cyl/SetStts,
  setCollision(60,150), gravity -9, setActorInfo2("Auc0"), the JOR HIO
  once-guard (mDoHIO_createChild w/ SJIS label, l_HIO.field_0x8
  refcount), evmng getEventIdx ×3 → m784/m786/m788 (MINIGAME_START/END,
  KAISEN_GETITEM), m74D=2 camera mode, head+backbone LOCKED
  (onHeadLock/onBackBoneLock), m734 = spawn pos.
- LEVERS: (a) a JOR-paged HIO = `: public JORReflexible` + explicit
  `virtual ~X() {}` (bmsw pattern) — dropping the dtor drops the WHOLE
  vtable and shifts every member by 4; (b) the assert EXPR string names
  the member (m_eye_tex_pattern was my m6F4); (c) SJIS HIO labels embed
  as UTF-8 in source, sjiswrap converts at compile.

## Batch 6: fuzzy 36.95 -> 48.65 (16/66 exact; WWDP c2de8d4a)

- eventOrder/checkOrder/clr_seq_flag/anmAtr/daNpc_Kg1_bcks_setAnm all 100;
  setAnm 97.2 pool-pending (instruction-identical, pool 16 bytes short
  until the TU fills).
- Event cluster: eventOrder = OR-guard (m732==2||==1) -> onCondition
  (CANTALK) + orderSpeakEvent on 2; else-if 3/4/5 ->
  fopAcM_orderOtherEventId(this, m784/m786/m788, 0xff, 0xffff, 0, 1) +
  onCondition(dEvtCnd_UNK2_e). checkOrder reads the command ONCE
  (`u16 command = eventInfo.getCommand()`) - a single lhz serving both
  compares rules OUT the yw1-style two-inline-calls spelling (each call
  would emit its own load); evmng start/end checks + m730 talk latch.
- anmAtr: `static const u8 anm_atr[9] = {1,2,4,5,6,7,8,9,10}` (named
  $-sym), getMesgAnimeAttrInfo/clearMesgAnimeAttrInfo pair, m74D dest.
- setAnm = the dLib idiom, kg1 keeps a PRIVATE clone
  daNpc_Kg1_bcks_setAnm = dLib_bcks_setAnm minus `force`, PLUS a
  negative-playSpeed rewind: `if (mPlaySpeed < 0) morf->setFrame(
  morf->getEndFrame())` (the fctiwz+extsh double-conversion = McaMorf
  setFrame's (s16) cast, getEndFrame's s16->f32 - both inline tells).
- LEVER (table storage trio, one function): `static const` local table
  -> NAMED $-sym, NO stack copy; plain `const` local table -> stack
  copy loop; the 12x dLib_anm_prm_c prm table non-static -> anonymous
  @-image + copy loop. The 8-byte rolling copy's iteration count (24)
  counts 8-byte PAIRS, not entries.
- TRAP REPAID: first symbol dump was `| head`-truncated and hid
  a_anm_bck_tbl$4952/a_anm_btp_tbl$4953 - cost one wrong spelling
  (const local) before the full dump ruled static. Donor name is
  a_anm_bck_tbl (no 's').
- Salvatore's anim map: bck res {0x11,0x1A,0x16,0x19,0x14,0x12,0x13,
  0x15,0x17,0x18}, btp map {0,0,0,0,1,0,1,1,2,3,0,0}, prm rows: idx 5
  = {4,3,NONE}, idx 10 = {9,next 8,NONE}, idx 11 = {4,3,speed -1.0}
  (reverse-play row); anm 9 at frame 31 -> mBtpAnm.setFrame(1).

## Batch 7: fuzzy 48.65 -> 53.85 (18/66 exact; WWDP 351e6434)

- kg1_talk_camera 100: `dComIfGp_getCamera(dComIfGp_getPlayerCameraID(0))`
  (bdk:2776 precedent), Stay/Set(m_camera_ctr, m_camera_eye,
  m_camera_fovy, 0)/Reset/SetTrimSize(1) guarded by m751 && camera —
  needed `#include "f_op/f_op_camera.h"` (camera_class body lives there,
  d_camera.h only forward-declares).
- wait_action_init 100: clr_seq_flag() + setAction(&wait_action).
  **LEVER: a 12-byte anonymous .data @-sym {0, -1, reloc->method} is a
  PTMF CONSTANT** — the member at 0x6E4 is the ActionFunc ptmf (called
  via __ptmf_scall in _execute), NOT a cXyz; .data relocs on the third
  word give the exact method. setAction spelled as the header inline.
- getMsg 93.86 PARKED (semantically complete): scoreboard ladder exact
  (event regs 0xBEFF best / 0xFF07 play-count clamp<3, onEventBit 0xE04,
  msgs 0x1D4D..0x1D64) except the FINAL m770 tail: donor emits a real
  branch diamond (cmplwi/bne/li/b/li) where our build folds {7501,7502}
  to addic+subfe. 10-variant probe bank (if/else, inverted, bool/s8/u16/
  int/ptr operand, switch, double-if, msg-var) ALL fold under GC 1.3.2
  -O3,s; target itself uses the fold idiom elsewhere (wait_action), so
  flags are right and the donor spelling is something none of the 10
  shapes reach. Harness-class micro-shape - same family as ym1 kari_1.

## Batch 8: fuzzy 53.85 -> 60.40 (WWDP 07318211)

- next_msgStatus 98.99 instruction-exact (pool/address residue): 19-case
  switch on *i_msgno-0x1D4D via .data jump table @4894; absent cases
  0x1D53/0x1D54 -> default ret 16; case 0x1D5C also rets 16 (m771=1);
  everything else rets 15.
- Decoded flow: intro ladder gated by dComIfGs_isEventBit(0xE04) +
  dLib_getIplDaysFromSaveTime()<4 + dComIfGs_isTmpBit/onTmpBit(0x101);
  pay block = mpCurrMsg->mSelectNum==0 && dComIfGs_getRupee()>=10 ->
  m74D=1 (anm attr), dComIfGp_setItemRupeeCount(-10), m751=1,
  m_jnt.onHeadLock(), player->setPlayerPosAndAngle(&cXyz(0,0,250),
  0x2000), msg 0x1D55; replay counter = getEventReg/setEventReg(0xFE07)
  clamp<3 selecting 0x1D60/61/62.
- LEVERS: (a) **split-declaration register allocation** - target had
  ret=r30/player=r31 with ret's `li 15` emitted FIRST; only spelling
  satisfying both: `daPy_py_c* player;` declared BEFORE `u16 ret = 15;`
  with the init as a separate statement after (decl order sets the reg,
  statement order sets emission). (b) **daPy_py_c vptr at +0x31C** -
  fopAc_ac_c is non-polymorphic, so daPy virtuals dispatch via
  lwz r12,0x31C(player); the (cXyz*,s16) slot = setPlayerPosAndAngle.
  (c) u32-returning dLib fn compared signed = donor `(int)` cast.
  (d) fopNpc 0x29B = m_jnt.mbHeadLock (0x290 + JntCtrl 0x0B) - a bare
  stb 1 there is onHeadLock() inlined.

## NEXT

Rebuild header on the fopNpc template (actor size from g_profile_NPC_KG1),
carve as decoded; then _create → createInit → init ladder → CreateHeap
family → anm cluster → action ladder, mirroring the yw1 order.
