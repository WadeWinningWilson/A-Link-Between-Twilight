# yw1 (Sue-Belle) decode campaign — anchor

Opened 2026-08-21, same session that closed ym1 to 121/124. yw1 is the ym1
sibling PLUS: hair physics (3-joint chain), the water POT (TSUBO) subsystem,
path walking (dNpc_PathRun_c), partner search vs BM1 (Grandma).

## Batches 1-3: 8 -> 24/119 exact

- Header rebuilt on the ym1 template: base fopAc_ac_c -> fopNpc_npc_c (ctor
  asm), all signatures pre-typed with the ym1 levers. Actor size 0x7CC
  (g_profile_NPC_YW1); members carved as decoded.
- HIO ctor EXACT via the SINGLE-ITERATION LOOP idiom (for i<1 over
  a_prm_tbl[1] — straight-line spelling misses by 2 register-choice rows).
  prm 0x30: ten s16 limits {0x2000,7800,-0x2000,-7800,0,8400,0,-8400,
  1600,1200} + 170.0f + u8[4] debug flags + 300.0f + s16{6,0x400} +
  0.95f/1.2f/0.5f.
- nodeCB trio: BackBone + Head are ym1 twins EXCEPT ZrotM takes the NEGATED
  accessor (-getHead_x / -getBackbone_x — lis-first expression arg). Hair is
  NEW: three-joint chain on m_hair1/2/3 (donor names from assert strings),
  YrotM(m786/m7A6/m7AE) + ZrotM sums (m784+(m780+m7B6) / m7A4+m7B8 /
  m7AC+m7BA — keep the parens, adds pair right-first). Both Head and Hair
  own a static cXyz a_eye_pos_off(18,20,0); Hair's is initialized but
  UNUSED (donor dead code). All three snapped once l_HIO + l_check_inf/wrk
  were declared at file top (164 bss bytes before the guard blocks).
- init_YW1_0-3 exact FIRST BUILD: pot spawn
  fopAcM_create(fpcNm_TSUBO_e, 0x7F063F, &current.pos, current.roomNo)
  guarded by mPathRun.isPath(); bits 0x520/0x1/0x2A20 + daynight;
  return m704 != fpcM_ERROR_PROCESS_ID_e. Action ptmfs: YW1_0/3 ->
  wait_action1, YW1_1/2 -> wait_action2.
- createInit exact: ob1 path idiom ((param>>16)&0xFF, setInf, OffStatus
  NOCULLEXEC, cc 0xff/0xf0), att distances 0xAB literal, 4-case tree on
  m7C6, m726 home csXyz, gravity -4.5f, NO kari cyl block (simpler than ym1).
  **NEW LEVER (proven, 1-row): `if (cond) { A } else { return false; }`
  places the fail block AFTER the then-body (beq to else); the equivalent
  early-return inlines it before A.** (Tried against ym1's chngAnmAtr fold
  park: does NOT crack it — 8th falsified shape there.)

## Batch 4: 24 -> 28/119 exact (successor session, 2026-08-22)

- play_animation EXACT: the aj1 spelling verbatim (NOT the ym1 twin — yw1
  adds the ground-material footstep feed): `if (mObjAcch.ChkGroundHit())
  sndId = GetMtrlSndId(mObjAcch.m_gnd);` then `m770 = mpMorf->play(&eyePos,
  sndId, dComIfGp_getReverb(current.roomNo))`; frame-wrap check vs m750.
- upLift EXACT (u8 return, NOT void as the template decl had it): the
  pot-on-head carry. searchByID(m704, &prm); m775 = prm == 1; if pot found:
  setCarryNow-if-not-carried, pot->shape_angle.y = atan2s of headMtx x
  l_hed_front, pot->current.pos = headMtx x (34,-4,0). `l_hed_front` is a
  ZERO cXyz file-static (bss 0x134, only upLift references it — donor
  dead-ish read, ported faithfully; declared between play_animation and
  upLift to land after the @4468-era statics).
- setMtx EXACT: ym1's shape MINUS the R-hand model block, PLUS setHairAngle()
  after mpMorf->calc() and upLift() before setAttention(). Carved
  mpHeadModel at 0x6D8.
- searchByID EXACT: the hi1 two-arg twin PLUS a null-guard on the out-param
  both at clear and at set (yw1 0x5C vs siblings' 0x54).

**TWO LEVERS MINTED (one meta-lesson):**
1. **==1-store clamp form discriminates the DESTINATION type.** Retail
   `srwi r0,r0,5` after subfic/cntlzw => the stored field is **bool**
   (m775 is bool, direct `m775 = prm == 1;`). The byte-clamped
   `rlwinm rN,rN,27,24,31` appears when the bool must exist as a **u8
   VALUE** — u8 local (knob00 `u8 bVar3 = (x==K)`), or bool operands of
   bitwise `&` (ji1 setAnm). 1-row tell, falsified 9 shapes before landing
   because I read the diff SIDES INVERTED — always print target/base
   LABELED, never trust a bare `<`/`>` memory.
2. **Locals-swap signature = reverse declaration order.** MWCC allocates
   same-scope local aggregates top-down: LATER-declared cXyz gets the
   LOWER stack slot. If every stack ref of two locals is pairwise swapped
   vs retail, flip their declaration order (`cXyz front;` before
   `cXyz ofs(34,-4,0);` put ofs at 0xC, front at 0x18).

Carves this batch: mpHeadModel 0x6D8 (J3DModel*) · m750 f32 (morf frame) ·
m770 u8 (play result) · m775 bool (pot-gone flag).

## Batches 5-6: 28 -> 40/119 exact (same successor session)

- resID pair + texPttrn family EXACT first build: bckResID plain 7-entry
  {0,3,4,2,1,5,9} (NO subtype switch, simpler than ym1); btpResID =
  single-entry {8} — THE ym1 LANDMARK TRAP, checked first as ordered;
  init_texPttrnAnm is ym1's with arc-name LITERAL "Yw" (not mArcName) and
  JUT_ASSERT line 0x28A; play_texPttrnAnm pure ym1 twin (m7BF/m6F0/m6F2 ↔
  m8AA/m6F4/m6F6). Carves: mBtpAnm 0x6DC, m6F0/m6F2, m7BF s8. Header carve
  side-snapped chngAnmTag/ctrlAnmTag/ctrlAnmAtr (pre-drafted empties).
- setAnm ladder closed: setAnm_NUM (7x{i,0,8.0,1.0,2} table), setAnm
  (m7C2-indexed, table w/ -1 rows at [0]/[2]), setAnm_ATR (m7BD-indexed),
  chngAnmAtr, anmAtr (ym1 twin, m7C8/m7BE). Carves m7C2/m7C8 s8.

**THE SESSION LEVER — OR-GUARD RETURN (cracks two shape families):**
`if (c1 || c2) { return; }` emits c1 as branch-if-true-to-end (FOLDED) and
c2 as the UNFOLDED pair `branch-if-false BODY; b END`. This is the ONLY
spelling that keeps the pair — empty-then/else, bare return-in-then,
explicit goto, && nesting ALL fold to a single inverted branch (12+
falsifications across ym1+yw1). Donor witness: hi1 setAnm_anm
(`if (temp < 0 || field == temp) return;`).
- yw1 setAnm_anm: VOID (not int) + OR-guard → exact.
- yw1 chngAnmAtr: `if (i_no == m7BD || i_no > 7) return;` → exact.
- **ym1 chngAnmAtr PARK CRACKED** (was 8 falsified shapes) → ym1 122/124.
- **ym1 setAnm_anm PARK CRACKED**: the "shared li r3,1 tail" was a mirage —
  retail loads NO return value anywhere; the function is VOID with the
  OR-guard, `int` + `return 1` was the template's invention → ym1 123/124.

**ym1 kari_1 park STANDS** (+3 falsifications this session: flat/dist
top-declaration arrangements all worsen to 11 rows; base = 8). Structure
understood precisely now: retail slots delta-temp@0x24 > argtemp@0x18 >
flat@0xC — the chk_areaIN by-value temp is allocated BETWEEN the delta
temp and the named local, which no tried declaration order produces. Not
the upLift reverse-decl lever (that one is same-kind locals only).

## Batches 7-8: 40 -> 49/119 exact (same successor session)

- createHeap family + privateCut EXACT: bodyCreateHeap ("Yw" res 6, morf
  flags 0x80000/0x11020022, head+backbone with the hi1-style `& 0xffff`
  index masks, asserts 2415/2429/2431 — plain retail line numbers, demo
  variants TBD at four-version gate time); headCreateHeap (res tbl {7} +
  texPttrn tbl {0} BOTH single-entry, indexed by NEW m7C5 s8; hair1/2/3
  joints + nodeCB_Hair on all three, asserts 2460/2477/2479/2481);
  CreateHeap (ym1 shape minus itemCreateHeap, SetWall(30,30) not (30,60)).
  POOL-SHIFT COROLLARY PAID AGAIN: body/headCreateHeap sat at 99.9x on a
  6-byte string shift — the missing "DUMMY" literal from UNWRITTEN
  privateCut; writing privateCut (ym1 twin on m7BC s8) snapped all four.
  Write the POOL NEIGHBORS first when a 99.9x cluster shares a stringBase.
- getMsg family EXACT: dispatcher = ym1's switch shape on m7C6; YW1_0/3
  share the m774/m775/pot-path head (m775 = the upLift pot-gone flag
  feeding msg 0x89F); adjacent-msg ternaries (`? n+1 : n`) compile to the
  branchless bool-add — spell them as ternaries. 1-row lever: a trailing
  `if (bit) return A; return c ? B : C;` pair whose A-load sits at the
  BOTTOM in retail = donor spelled `if (!bit) { return c ? B : C; }
  return A;` (negated wrap, tail hoisted).

## Batches 9-10: 49 -> 57/119 exact (same successor session)

- Talk/attention cluster ALL EXACT FIRST BUILD (pure ym1/hi1 twin
  vocabulary, zero falsifications): eventOrder/checkOrder (m7C1 s8 order
  slot, m77C talk-latch), chk_parts_notMov (m758/m75A/m75C vs
  m_jnt.mAngles + current.angle.y), setAttention (attnY =
  l_HIO.children[m7C5].mPrm.field_0x14 — the 170.0f; eye override from
  m72C when m760||param — NOTE yw1 indexes children[m7C5] directly, ym1
  used [mSubType - 1]), decideType (param-switch, NOT fopAcM_GetName like
  ym1 — m7C5=0 always, m7C6 = 0..3), chk_talk (m772 = PreItemNo),
  chkAttention (verbatim ym1), next_msgStatus (single case 0x8A3 →
  0x8A4/0x8A6/0x8A5 on 0x2A20 + daynight).
- Carves: m758/m75A/m75C s16 (anim-settle snapshot) · m760 int ·
  m77C u8 · m7C1 → s8.

## Carved so far
mPhs 0x6C4 · m_hed/bbone_jnt_num 0x6CC/D · m_hair1/2/3 0x6CE-D0 ·
m704 fpc_ProcID (pot) · mPathRun 0x70C (dNpc_PathRun_c, 8b) ·
m726 csXyz home angle · m72C cXyz eye target · m744 cXyz head translation ·
hair angles m780/m782/m784/m786, m7A4/m7A6, m7AC/m7AE, m7B6/m7B8/m7BA s16 ·
m773/m777 flags · m7BD u8 (anm ATR no) · m7C0 s8 (stt) · m7C6 s8 (type).

## NEXT (in rough dependency order)
play_animation / upLift / setMtx cluster; bck/btpResID + texPttrn family
(check single-entry-table trap FIRST — the ym1 landmark); anm cluster;
createHeap family (body/head; NO itemCreateHeap here — pot is a separate
actor); getMsg family; talk/attention; setStt (limit 7 on m7BD-compare);
set_pthPoint/setHairAngle/chngTsuboAnm/chk_brkTsubo/chk_bm1Odoroki (pot+
hair subsystems — read fresh, no ym1 twin); walk_1 (path); wait/turn/talk
ladder; demo/_draw/_execute/_create; partner family (searchByID/
partner_search/partner_search_sub — ba1+ko1 vocabulary).
