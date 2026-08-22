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

## NEXT

setNrmVtx (0x538), then packet draw (0x830, heals string pool + @2100/
@2080), then ho_move (0xD1C, heals @literal order; z_rate_tbl$4444
func-static 13xf32, l_texCoord 85x8B, l_matDL 0x34 GX display list
still to extract). Consult d_a_sail's equivalents FIRST for each.
