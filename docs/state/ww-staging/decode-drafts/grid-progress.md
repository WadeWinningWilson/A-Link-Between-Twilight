# d_a_grid decode campaign — anchor

Opened 2026-08-22 (EIGHTH TU this session). **d_a_grid is the ship's
SAIL** — daHo_packet_c (帆 ho = sail) is a double-buffered 85-vertex
cloth renderer, ho_move is the 3.3KB cloth/wind simulation. This is a
FRAMEWORK unit (main DOL, not a REL): target object is
build/GZLE01/obj/d/actor/d_a_grid.o and it uses **R_PPC_EMB_SDA21
small-data relocations** — a different reloc regime from every REL TU
this session (float literals come from @sda anchors, r13/r2-relative).

## Baseline at open

- 3/20-ish exact (weak dtors), unit fuzzy 3.06, target disasm 3145
  lines (grid_tgt.s untracked in WW DP root).
- Function sizes: ho_move 0xD1C(!), packet draw 0x830, setNrmVtx 0x538,
  _create 0x514, _execute 0x1C4, _draw 0x23C, sinit 0x148.

## Census (from the small functions)

- daHo_packet_c layout: +0xB4 vtx[2][85] cXyz (bank stride 0x3FC=1020 =
  85*12); +0x8AC nrm bank; +0x10A4 back-nrm bank (setBackNrm: back[i] =
  zero - nrm[i] via PSVECSubtract into a zeroed dst); +0x189C s16 yrot
  (setNrmMtx = mDoMtx_YrotS(calc_mtx, angle) — calc_mtx is an SDA
  global); +0x18A2 u8 current-bank index.
- setNrmVtx(cXyz*, int col, int row): vertex index = col + row*7,
  rows 0..9 special-cased at 0 and 9 — grid topology 7 wide.
- daHo_HIO_c 0xA4 bytes, virtual dtor; sinit registers l_HIO +
  something at @-bss.
- daGrid_c: fopAc + blob to 0x2200 f32, 0x2216 s16 wind-rel angle +
  0x2218 u8 force flag (header already carves these — the
  force_calc_wind_rel_angle inline exists).

## Plan

Batch order: wrappers + _delete + setBackNrm/setNrmMtx first, then
setTopNrmVtx/setNrmVtx, _execute/_draw/_create, then the two monsters
(packet draw, ho_move) last with the yw1 setHairAngle experience
(spring/wind constants from the SDA pool). Expect SDA quirks: literal
pools are shared DOL-wide sda2, so pool-offset residues behave
differently than REL @-pools.

## Batch 1 (WWDP see log): 6+ matched

- Wrappers were already stub-correct; setNrmMtx = cMtx_YrotS(*calc_mtx,
  mAngleY) (calc_mtx = SDA Mtx* from c_lib.h, pirate_flag precedent);
  setBackNrm EXACT with the **roving-pointer loop lever** (target
  advances two pointers +12/iter; an indexed [i] spelling emits
  add-per-access — spell `for (...; i++, nrm++, back++)`).
- _delete instruction-exact (pool order pending _create): arcs are
  "Cloth" (mPhs 0x290) and "Ship" (mPhs2 0x298); the grid HIO teardown
  uses mNo-and-reset (no refcount): deleteChild + mNo = -1.
- daHo_HIO_c.mNo carved (s8 at 0x4).

## Batch 2 (WWDP faa40a64): 8/20 matched

- setTopNrmVtx 99.85 (pool), _draw 98.60 instruction-exact, _execute
  98.54 (one reg-rename park, kari family), setBackNrm/setNrmMtx 100.
- **LEVER: whole-struct tevStr assign.** The 60-line field-copy block
  in _draw is ONE line - `tevStr = l_ship->tevStr;` - because the
  IMPLICIT dKy_tevstr_c operator= calls J3DLightObj's USER operator=
  (which copies only mInfo, skipping field_0x34[64]) and word-copies
  the GXColorS10s; hand-spelled member assigns go bytewise/lha and
  miss. Proven precedent: d_a_sail (matched 100) - THE PIRATE SAIL IS
  THIS ACTOR'S SIBLING and covers most remaining shapes.
- _draw details: current.angle (not shape_angle) rotations + ship
  getSailAngle() Yrot, MtxScale(1,scale.y,1)*HIO scale, concat into
  packet mtx, entryZSort into isMonotone() ? XluListP1 : XluList with
  buf->mpZMtx = *calc_mtx.
- _execute: camera-distance alpha fade - (current.pos - eye).abs()
  AS A TEMP (naming the diff adds 12 bytes of frame), lerp between
  HIO alphaFar/alphaNear by dist/fadeDist, approach by ±5/frame,
  skip ho_move when scale.y < 0.01f (sail furled).
- Carves: daHo_packet_c head = J3DMatPacket + pad[0x80-sizeof] + Mtx
  mMtx(0x80) + dKy_tevstr_c* (0xB0); daGrid embeds the packet at
  0x2A0 (alpha at packet+0x18A3); daHo_HIO draw-scale xyz(0x24) +
  alphaFar/Near(0x30/31) + fadeDist(0x34); l_ship = daShip_c* GLOBAL
  DEFINED IN THIS TU (sbss).

## Batch 3 (WWDP 8fce0db2): 14/20 matched

- NEW 100s: _create-cluster heals — __sinit, __dt__10daHo_HIO_c,
  __dt__13daHo_packet_c, entry weak, _draw, setTopNrmVtx. _create 97.88,
  _delete 93.55 (pool-only), _execute 98.54 (batch-2 reg park stands).
- **STRUCTURAL: J3D packet layout is TP-stale in comments only.** WW
  J3DDrawPacket sizeof = 0x24, J3DMatPacket = 0x3C (DOL evidence:
  entryMatAnmSort reads mpMaterialAnm at 0x38, addShapePacket list head
  at 0x28, ctor inits 0x24/0x2C/0x30=-1/0x34/0x38). The repo header's
  `/* 0x28 */`-style comments and `// Size: 0x40` are TP numbers, but the
  COMPILED layout is already WW-correct — derive daHo_packet_c from
  J3DMatPacket directly; mShapePacket lands at 0x3C, mMtx at 0x80.
  daHo ctor = zero 18A2/189C/18A0/189E, alpha 255,
  setShapePacket(&mShapePacket) (0x28 = mpShapePacket, NOT init-shape).
- **LEVER (new): stack-slot decl order.** Named cXyz locals get frame
  slots in DECLARATION order (later decl = lower address). setTopNrmVtx
  needed `cXyz out;` declared BEFORE the cross var to swap two slots.
- **LEVER (new): cMtx_concat, not PSMTXConcat.** The m_Do_mtx.h INLINE
  WRAPPER evaluates its args as a call boundary — arg3 (packet mMtx addi)
  emitted FIRST, then j3dSys, then calc_mtx. Raw PSMTXConcat evaluates
  left-to-right and never matches. Precedents: d_a_bwdg, d_a_goal_flag,
  d_a_majuu_flag all spell `cMtx_concat(j3dSys.getViewMtx(), *calc_mtx,
  packet.getMtx())`.
- **LEVER (new): int reg-numbering via decl scope.** `int top;` declared
  BEFORE the for loop (assigned inside) makes top enter the allocator
  before i → top=r29, i=r28 as donor. Declared inside, order flips.
- HIO dtor sets mNo = -1 (donor teardown, not empty).
- _create decode: fopAcM_ct + param u8 (0x1B48) + resLoad Cloth/Ship +
  HIO createChild("船の帆" SJIS) + 85-vertex loop: band amp
  (rows 0-6→40, 7-13→70, 42-55→85, else 80), z-envelope
  amp*sin(minDist * 1.05f*(pi/2 / (span*0.5f))), column-detect 12-term
  || chain (i≡j mod 7 → top=j+56, default 6), y-envelope split at
  l_pos[top].y (below: 0-anchored 1.05 rate, amp 35/70/80 by top;
  above: 84-anchored 1.15 rate, amp 20), mSwingSize[i] =
  std::sqrtf(SQUARE+SQUARE). l_pos = static Vec[85] extracted bit-exact
  (12 rows x 7 cols + apex y=368.75).
- Carves: daGrid_c mParam 0x1B48, f32 mSwingSize[85] 0x1B54,
  cXyz mVtxSpd[85] 0x1CA8. daHo_HIO_c full 0xA4 (cloth-sim params
  0x3C-0xA0, values in ctor).
- PARK-until-closure families in _create/_delete: string pool ("Ship"
  first ref lives in packet::draw at +0x7ee) and @literal ordering
  (donor 0.5f/1.0f numbered in draw/ho_move before _create) — both heal
  when those two functions are written. Also .data @2100/@2080 (two
  (1,1,1) float triples) precede l_pos — emitted from draw/ho_move
  bodies.

## Batch 4 (WWDP b7a748b4): setNrmVtx 99.47 PARKED

Sail-sibling normal accumulator; grid variants: row-8/9 boom seam (no
smoothing across), row-11 apex vs vtx[84], y-flatten rows >7,
normalizeRS fallback +X, 900*cM_ssin(-800*(col+row)) Y-skew. 2-insn
park (in-source note): donor REMATERIALIZES (param_1+idx) at the col!=6
site while 1.3.2 promotes it for every containing spelling — 6 variants
falsified (plain/parens/register/named idx2/inline mul/flipped) — the
v2 operand-flip (idx+param_1+1) keeps the full register map at the cost
of 2 reordered adds.

## Batch 5 (WWDP 985f6114): packet draw 99.7 (now 100 in report)

Full GX pipeline: INDEX8 pos/nrm/tex over the double banks; Ship CI8
sail tex (RGB565 TLUT, dRes_INDEX_SHIP_BTI_NEW_HO1_e=23) + Cloth toon
TEXMAP1; 2/3-stage TEV keyed on tevStr K1.a; fade alpha =
mColorC0.a=mAlpha through TEVREG0, A0*TEXA in the l_HIO.field_0x05
branch; matDL 0x20 + DOUBLE-SIDED geometry (CULL_BACK l_DL pass, rebind
GX_VA_NRM to mBackNrm, CULL_FRONT same l_DL); J3DShape::sOldVcdVatCmd
reset. l_texCoord (cXy[85]) / l_DL (u8[0x233] ALIGN32, GLOBAL not
static) / l_matDL extracted bit-exact. **@2100/@2080 (two (1,1,1) Vec
aggregates) auto-emit via the PCH J3DJoint inlines — never author them.**

## Batch 6 (WWDP bf5fd9d0 + 5c07bb8d): ho_move 97.2, unit 98.80, 16/20

The 0xD1C cloth sim. First draft scored 92.1; levers to 97.2:
- **LICM shape**: the x/z wind-clamp expressions live INSIDE the loop
  (donor); hoist temps rank BELOW windPow in the f-map. Precomputing
  them outside gives them f22/f21 and wrecks the whole allocation.
- **f32 top-declaration order = f-reg order** (f31 down): swayX, swayZ,
  swingZ, swingX, stretch, leanSin, open, colF, rowF, windPow.
- loop-2 bank pointer = fresh short-lived local (vtx2), select-form
  row advance (`row = col < 6 ? row : row + 1`), ternary phase-speed
  cap, named single-use loads (fabsf temp, sp28.x/z copies).
- **MWCC FLOAT-PARSE TRAP (new lever, repo-wide)**: python round-trip
  extraction is IEEE-correct but MWCC parses some decimals 1 ULP LOW
  (0.0007f -> ...34 vs donor ...35; 0.0002f likewise). Spell the exact
  f32 decimal (0.00070000003f) — himo2's 0.060000002f is the precedent.
  ANY table extracted with the python formatter must be re-verified
  against MWCC output bytes, not python.
- _delete healed to 100 (string pool fixed by draw's "Ship" first-ref).
Decode map: wind-relative angle chase w/ force_calc override
(cLib_addCalcAngleS2 4/0x1000 vs 2/0x1400), mBankIdx ^= 1 double
buffer, m1B44 phase (2500+9000*0.8*pow, cap 10000), m2212 col-phase
(3000*cos +/-300), sag machine (getSailOn + m2208 latch + 15-frame
m1B4A burst, cLib_addCalc2 targets 1.414/1.0/1-0.65*m2200), 85-vertex
loop (colStep=10922*x_rate[row], sway pair vs mSwingSize envelope,
row-anchor y-blend l_pos[row*7].y, 120/5 amp fans, col>4 4.25 kicker,
fade=0.35+0.65*open, z -13.75 bias), boom-lean z-band pass
(fabsf(vtx[59].x) * (const + REG6_F(8..12)) on rows 6-10), 12x7
setNrmVtx sweep + apex + setBackNrm + 3x DCStoreRangeNoSync.

## PARKS at campaign hold (unit fuzzy 98.80, 16/20 exact)

- setNrmVtx 99.4: 2-insn remat shape (batch 4, falsified list in-source).
- ho_move 97.2: caller-save temp rotation flips after the first sqrtf
  (f1/f2 cascade ~85 value-identical rows), extsh order at the atan2s
  use (+relAngle r28/r26), vtx bank-pointer association (4 rows). All
  kari-class; candidates for the transform-search harness.
- _create 98.5: rate-constant f28/f29/f30 binding follows @slot order;
  donor's 0.5f slot is a PCH-HEADER token (@4130) so it sorts FIRST;
  my TU numbers 0.5f fresh in ho_move/_create. Same root as the
  .sdata2 slot ORDER + 0x64 pad delta (values all match). Park:
  header-token numbering isn't reachable from this TU's source.
- _execute 98.5: batch-2 reg swap park stands.
- daHo_HIO_c vtbl is weak here, global in donor (donor likely defines a
  genMessage out-of-line somewhere; no such function in the TU — check
  at Matching-flip time).


## Sweep-harness pilot (2026-08-22): ho_move rotation park HARDENED

`tools/foundry/decoder_sweep.py` (NEW INSTRUMENT, v0 of the
transform-search harness the park family names): given binary
source-spelling slots, it compiles every combination direct-mwcc
(bypassing ninja, cflags auto-read from build.ninja), scores one symbol
via objdiff-cli, restores source AND object, reports the top
combinations. 32 builds ≈ 4 min.

Pilot result on ho_move's caller-save temp-rotation park: the full
2^5 space over {windAngle-naming, x-temp, z-temp, t-ternary-arms,
lift-operand-order} NEVER beats the 97.03 baseline, shows ZERO
interaction effects, and confirms x-temp (+0.26) and t-arms (+0.14) as
the only live knobs — all already at their best setting. The rotation
seed is not reachable through this toggle family; the park stands with
mechanical falsification instead of hand-probe anecdotes.

Next harness targets: the six-TU park family (ko1 x6, ym1 chngAnmAtr,
kg1 wait_action/getMsg heads, mgameboard sinit, grid setNrmVtx/_execute)
— each needs its slots authored from the existing falsification notes,
then one sweep per function.
