# Decoder — scout of the queue behind so/ob1/p2/aj1

**Measured 2026-08-18, script-computed from `objdiff report`.** Nothing in this
file has been edited by me except where noted; this is reconnaissance so the
next pass starts informed instead of re-measuring.

| TU | exact | fuzzy | configure.py |
|---|---|---|---|
| `d_a_npc_kamome` | 162/162 | **100.0000%** | (already Matching) |
| `d_a_lbridge` | 24/24 | **100.0000%** | Matching |
| `d_a_kamome` | 65/67 | 99.9905% | `Equivalent` — *fpr regalloc* |
| `d_a_bridge` | **43/45** | 99.9467% | `NonMatching` |
| `d_a_saku` | 2/66 | 1.4700% | `NonMatching` |
| `d_a_npc_ko1` | 5/203 | 1.9240% | `NonMatching` |
| `d_a_npc_ym1` | 8/124 | 2.4940% | `NonMatching` |
| `d_a_npc_yw1` | 8/119 | 2.2788% | `NonMatching` |
| `d_a_pt` | 1/107 | 0.4051% | `NonMatching` |

## `d_a_bridge` — 2 functions short of a whole TU, and both resist

The cheapest-looking win on the board, so I swept it first. **It did not move,
and I changed nothing.**

Both residuals are *pure register-name swaps* — identical instructions,
identical offsets, identical stack layout:

- `daBridge_Execute` **22 rows**: target `r17` = the `idx` (`srawi r20, 2`),
  `r18` = the `&i_this->mBr[idx]` pointer; mine has them exchanged, and every
  other row follows from that one choice.
- `bridge_move` **25 rows**: r24/r25, r23/r24, r25/r26 exchanged across three
  loop variables.

**Swept on `daBridge_Execute`, all 22 rows, no movement:** `sw` declared before
`idx`; `idx = i >> 2` as one statement instead of `idx = i; idx >>= 2;`;
`other_i` declared first; the inner shadowed `pBr` hoisted to the top as a
separate uninitialised local; the shadow removed entirely; an `idx2` copy inside
the block (that one made it **worse**, 38).

Per today's `nodeOb1Control` lesson I checked the types first — `idx`, `sw`,
`other_i` are all already `int`, and the pointer is API-typed. **There is no
free type to correct here**, which is what distinguishes this from the `u16
jntNo` case.

## Where the real work is

`ko1` (203 fns), `ym1` (124), `yw1` (119), `pt` (107), `saku` (66) are all
**under 2.5%** — five untouched TUs, 619 functions. `aj1` went 2.9112% ->
99.7659% in one session, so early-campaign work on a fresh TU is worth far more
per hour than the last two rows of `d_a_bridge`.

**Recommended order: `ko1` first** (largest, and it is an NPC so the
`fopAcM_SetupActor` create-chain trick that snapped 31 functions exact at once
on `aj1` applies directly).
