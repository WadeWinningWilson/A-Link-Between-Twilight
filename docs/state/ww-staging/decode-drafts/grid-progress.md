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

## NEXT

Batch 2: setTopNrmVtx (0x10C), _execute (0x1C4), _draw (0x23C); then
setNrmVtx (0x538), _create (0x514); the monsters (packet draw 0x830,
ho_move 0xD1C) last.
