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

## NEXT

_create (0x514, heals sinit + string pool), setNrmVtx (0x538), then
packet draw (0x830) and ho_move (0xD1C) - consult d_a_sail's
equivalents FIRST for each.
