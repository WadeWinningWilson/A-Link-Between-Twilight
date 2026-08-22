# Estate tail pass — fresh-eyes sweep over all sub-100 functions (2026-08-22)

One pass over every predecessor TU's residuals with the day's lever kit.
WWDP commits: 7624deba, 69605d23, 09c244ee (+ yw1/grid ledgers).

## Scoreboard moves

- yw1 109 -> 113/119 (sinit 100, turn_1 100, lookBack 100, talk_1 100,
  walk_1 91.9->99.0, chk_areaIN 99.98) — see yw1-progress.md
- aj1 127 -> 128/131 (_nodeCB_BackBone 89.5 -> 100)
- so 177 -> 178/187 (jntHitCreateHeap 95.7 -> 100)
- ko1 183/203, kg1 56/66 (string-pool fixes, earlier today)

## The lever kit that did it (cross-TU, apply BEFORE hand-diffing)

1. **INT-LOCAL for s16-returning calls** (cLib_targetAngleY, cM_atan2s
   family): donor captures into `int`, producing extsh AT ASSIGNMENT.
   Six confirmations today (grid, walk_1, turn_1, chk_areaIN, BackBone
   x2 — BackBone also needed the value in a NAMED local before the
   mDoMtx_*rotM call; accessor and stack-method forms still evaluate
   the matrix address first).
2. **Signedness at the CONSUMER names the type**: raw lbz into an Sc
   param = s8 member (u8 adds a conversion extsb); switch with extsb =
   s8; bare-srwi store of a compare = bool. (m7C3/m7C4/m776.)
3. **else-return placement**: `if (x) { use; } else { return FALSE; }
   return TRUE;` is its own shape — sinks the TRUE load below the else
   arm. Guard-return, diamond-fold (`return x != NULL`), and
   if/else-assign all emit distinctly (so::jntHitCreateHeap).
4. **sinit inline-ctor stores ARE the initializer**: a file-scope cXyz
   whose sinit stores non-zero values was CONSTRUCTED with them
   (yw1 l_hed_front = (0,1,0); a default-ctor decl loses it).

## Two real behavior divergences found by byte-matching

- yw1 walk_1's area gate was INVERTED (donor: `chk_areaIN(...) ==
  false`, walk while OUTSIDE the stop zone).
- yw1 l_hed_front silently zero instead of the +Y unit vector.

## New parks (falsified, do not re-run)

- so::checkTgHit 98.0 (3 rows): the `sePos = &eyePos` in-arg assignment
  is hoisted 5 rows above the donor position. Falsified: no variable
  (89.9 — the callee reg disappears and everything shifts), separate
  declaration+init statement (90.8 — the whole call block reorders).
  The in-arg spelling is the best known; the donor's addi sits at
  arg-2-eval position in a way none of the three spellings produce.
- ko1 setAnm_anm/set_balloonAnm_anm: falsifications 6+7 (copy-chain,
  source-swap) — see ko1-progress.md.

## Remaining sub-100 estate map (post-pass)

- setHairAngle (yw1, 94.7) — spring family, sweep-harness target
- ko1 six-park family (~45 rows) — sweep-harness targets
- so: _nodeControl/_createHeap 99.67, modeNearSwim/_execute 99.81,
  cutMiniGameProc 99.24, checkTgHit 98.0 — mixed pool/small-shape,
  next fresh-eyes candidates
- p2 family 98.5-99.9 (10 fns), ob1 (3), bridge (2), kamome-actor (2),
  ym1 kari_1 (1) — untouched this pass
