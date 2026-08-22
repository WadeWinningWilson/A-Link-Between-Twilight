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
