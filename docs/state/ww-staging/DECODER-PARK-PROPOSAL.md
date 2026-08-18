# DECODER — park proposal for the WW NPC decode campaign (so / ob1 / p2)

**Status 2026-08-18.** Three TUs, all NonMatching, all at the same point:
placement is exhausted and every remaining row is allocator-class.

| TU | exact | fuzzy | remaining |
|----|-------|-------|-----------|
| `d_a_npc_so`  | 175/187 | 99.6699% | 12 fns / 69 rows |
| `d_a_npc_ob1` | 109/115 | 99.8324% |  6 fns / 36 rows |
| `d_a_npc_p2`  | 133/145 | 99.9503% | 12 fns / 99 rows |

## What was ruled out, by measurement

- **Placement.** All three have aligned string pools and aligned `.bss`;
  `.data`/`.rodata` differ only where it costs zero rows. The four placement
  root causes that took `so` from 139 to 175 exact have no siblings left.
  (Instrument: `tools/foundry/pool_align.py`.)
- **Build configuration.** `configure.py` carries **314 `ActorRel(Matching)`**
  entries built with the identical `cflags_rel` (`-inline noauto`, `-O3,s`,
  `-schedule off`, `-sdata 0/-sdata2 0`), byte-identical to retail. The flags
  are correct and must not be tuned per-actor.
- **Declaration style.** Measured false as a systemic cause: the two
  byte-identical NPC oracles carry 11 (`p1`) and 10 (`ba1`) mid-body
  declarations, and `ba1::CreateHeap` declares at point of use exactly as I do.

## What remains, bucketed

| Class | Where | Rows |
|---|---|---|
| Register colouring | `_createHeap` 12, `nodeOb1Control` 11, `modeNearSwim` 8, `_nodeControl` 5 | 36 |
| Literal-pool position | `cutMiniGameProc` 15, `_execute` 5, `cutEatesaFirstProc` 3, `createInit` 1, `_draw` 1 | 25 |
| Argument evaluation position | `checkTgHit` | 12 |
| Stack-slot order | `ob_movPass` | 11 |
| Vtable shape (**shared header**) | HIO ctor 9 + dtor 4 | 13 |
| Switch dispatch scan | `control_anmAtr` | 6 |
| Branch shape | `jntHitCreateHeap` 4, `chg_anmAtr` 2, `_create` 2 | 8 |
| p2 register-mirror | (as previously inventoried) | 74 |

## The recommendation

**Park `so`, `ob1` and p2's register-mirror rows as §2b Equivalent with written
reasons; do NOT keep grinding them.** Reasons, stated plainly:

1. Every remaining class is a *compiler allocation decision*, not a semantic or
   structural difference. The instruction sequences match; what differs is
   which register or stack slot the allocator chose.
2. My hit rate on this bucket is **zero across roughly a dozen attempts**, and
   the falsified list is now long enough to be informative on its own: inlined
   `&eyePos` (14→32), `BOOL ret = TRUE` hoist (no change), `const cXyz&`→value
   (11→22), named arg copy (11→22, frame grew), aliased `i_model` (no change),
   hoisted declarations (no change), two `chg_anmAtr` switch forms (2→6, 2→4),
   `switch(==) case 0` (2→4).
3. Continuing has an opportunity cost: the queue behind these three
   (`aj1/ym1/yw1/ko1/kamome`, `saku`, `pt`, `bridge_move`) is untouched, and
   early-campaign work on a fresh TU has historically paid far better than
   endgame row-chasing — `so` went 139→175 in one session on placement alone.

**THE ONE EXCEPTION I would not park:** the HIO ctor/dtor's 13 rows have a
*known cause and a known fix* — my `daNpc_So_HIO_c` overrides a virtual (the
destructor) that the donor's does not, so I emit a merged derived vtable at
`__vt__14daNpc_So_HIO_c + 0xc` where the target stores the base's own
`__vt__10dNpc_HIO_c` at +0x4. That is a change to the **shared `dNpc_HIO_c`
header**, which reaches every other npc TU. It needs History's scoping and
review, not a decoder acting alone — but it is real, understood, and worth
taking.

## What I am NOT claiming

This is not "these TUs are done". They are NonMatching and the REL SHAs do not
equal retail. §2b Equivalent is a *park with written reasons*, and the reasons
are above. If the estate's bar is SHA-or-nothing, the honest answer is that
these three need a different technique than any I have, and I would rather say
that than keep reporting motion without movement.

**Decision needed from the user** (History for the vtable item): park, or keep
grinding, or hand the allocator bucket to a different approach.

## Precedent, found after the proposal was written

`configure.py` already parks actor RELs for exactly this reason class:

    ActorRel(Equivalent,  "d_a_kamome"),   # fpr regalloc
    ...                                    # regalloc

**`d_a_kamome` sits at 65/67 exact / 99.9905% and is parked `Equivalent` with
the one-line reason "fpr regalloc".** That is the same situation as my three
TUs and the same reason class, in the same actor-REL family. The mechanism I
am asking to use is not novel here, and the bar for the written reason appears
to be a short, specific phrase naming the cause.

**THE COUNTERWEIGHT, stated so this is not read as a free pass: there are only
9 `Equivalent` entries in the entire repo**, and the other reasons cited are
"Nondeterministic compiler bug? Do not link". So Equivalent is used SPARINGLY
- it is not a dumping ground, and adding three more at once is a meaningful
fraction of the total. That argues for parking with per-TU reasons that name
the specific bucket (register colouring / literal-pool position / stack-slot
order), not a blanket "allocator differences" label, and it argues for taking
the HIO vtable fix rather than parking it, since that one has a real cause.

Also worth recording from the same survey - **the opportunity cost is now
measured, not asserted.** The untouched queue behind these three:

    d_a_npc_aj1   12/131 exact   2.9112%
    d_a_npc_ym1    8/124 exact   2.4940%
    d_a_npc_yw1    8/119 exact   2.2788%
    d_a_npc_ko1    5/203 exact   1.9240%
    d_a_saku       2/ 66 exact   1.4700%

Five TUs at 1.5-2.9%, i.e. essentially undecoded, against 69+36+99 rows of
allocator noise on three TUs already at 99.67-99.95%. (`d_a_npc_kamome` is
already `Matching` at 162/162 - nothing owed there.)
