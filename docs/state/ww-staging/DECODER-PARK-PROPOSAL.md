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

## CORRECTION - "placement is exhausted" was true for ROWS, not for SHA

Found while fixing a bug in my own instrument, and it changes what a park means
for `so` specifically.

**The bug:** `pool_align.py` parsed `objdump -s` with a regex matching only
8-hex-digit groups, so a PARTIAL trailing word (the last line of a section, e.g.
`3100`) was silently dropped. Every pool reading truncated its final entry.
Fixed and committed; re-verified that so/ob1/p2 string pools still align.

**What the fixed reading shows on `so`:** `.rodata` aligns **71/85 objects**
(my earlier "25/85" predated the `cutSwimProc` 7.0f fix and I never re-ran it -
that is on me). First divergence is at **0x1E4**, where the target carries
**14 four-byte constants that my object does not have at all**: an alternating
`00FF0080` / `FF000080` pattern - GXColor green/red at alpha 0x80.

**They are UNREFERENCED.** No function in the target `.s` loads them. They are
donor-emitted dead data - the same shape as the `sph_offset` / `cyl_offset_A/B`
statics I restored to `jntHitCreateHeap` earlier in the campaign, and almost
certainly the dead-stripped body of `debugDraw` (which in both builds is a stub
of unused locals).

**WHY THIS MATTERS, and it cuts against my own recommendation:**

- For the **row count / §2b Equivalent** question: irrelevant. Nothing
  references them, so no function's diff rows depend on them. My "remaining
  rows are all allocator-class" statement stands.
- For **SHA**: decisive. `.rodata` is not byte-identical while 56 bytes of
  donor constants are missing, so **`so` could reach 187/187 exact functions
  and still fail the SHA gate.** Any ruling that says "grind to SHA" must
  include reconstructing dead data that no code path reads.

I would rather surface that now than have a SHA gate fail later and look like a
surprise. It does not change the park recommendation - if anything it
strengthens it, because the residual work on `so` includes speculative
reconstruction of code the donor itself dead-stripped.

---

## ADDENDUM 2026-08-18 — a correction to my own evidence, and a narrowed claim

**I found a real lever, and it retires part of the "colouring never pays" claim
above. The recommendation for `so`/`ob1`/`p2` is UNCHANGED; the reasoning is
narrower and one headline example was wrong.**

### What changed

`aj1`'s `lookBack` was cited in this document (and in bus rows 439/440/442) as
the flagship colouring failure — *"the matching-sibling declare-locals-up-front
arrangement, measured 42 -> 45 rows, WORSE"*. **`lookBack` is now EXACT.** It was
never a colouring problem. It had three ordinary defects:

1. It read `shape_angle.y` (0x20E) where the donor reads `current.angle.y`
   (0x206) — a **wrong field**, at three sites.
2. `field_0x76a` was `u8` where the donor has `bool`; passing a `u8` to a `bool`
   parameter emits `subic/subfe/clrlwi` normalisation the donor does not have.
3. `field_0x7bd` was `u8` where the donor has `s8` (`extsb` on the subject).

Errors 1 and 3 were only visible once I read the **correct game version's**
disassembly (I had been reading the JP `.s` while building US — see
`decode-drafts/aj1-progress.md` §Rounds 41-48).

### The new lever

**Declaration position and initialisation position are SEPARATE levers.**
`cXyz dstPos;` declared early emits no code but claims the **stack slot**;
`dstPos.set(0,0,0)` placed later fixes the **store order**. Every one of my
earlier ~17 attempts moved declaration and initialisation *together*, which is
why each one traded one diff for another. Measured on `lookBack`:
declare-first-with-initialiser 16 rows, pointer-between 20, pointer-first 18,
**declare-then-set-later 0**.

### And the boundary — this is why the recommendation still stands

**The lever moves STACK SLOTS. It does not move REGISTER NAMES.** Tested
directly on `so`'s `_nodeControl` (3 forms: decl-then-set 5 rows, swapped decl
16, mixed 13 — baseline 5, i.e. **no improvement**).

`so`'s remaining residuals are pure register-*name* swaps: identical
instructions, identical offsets, identical stack layout, with r30/r31 or r29/r30
exchanged (`_createHeap` 12, `_nodeControl` 5, `modeNearSwim` 8). There is no
stack slot to move, so there is nothing for the lever to act on.

### Net effect on the ask

- **`so`, `ob1`, `p2`: recommendation unchanged** — park as §2b Equivalent. The
  evidence is now *stronger*, not weaker: I found a genuine lever and it
  provably does not apply to what is left in these three.
- **`aj1` is withdrawn from any park discussion.** It is at **118/131 exact,
  99.6350%** and still moving; it should not be parked.
- **The falsified-attempts list above should be read as ~16, not ~17**, and the
  `lookBack` line struck.

**Still owed:** the user's ruling (park / keep grinding / hand off), and
History's `dNpc_HIO_c` vtable scoping — the one item I would not park.

---

## ADDENDUM 3 — `so` moved. The park recommendation is now WITHDRAWN pending a re-audit.

**`so` went 99.6699% -> 99.7854%** on the first function I re-opened with the
new lever. `checkTgHit` was **12 rows** — the largest single entry in this
document's "argument evaluation position" row — and it is now **3**.

```cpp
cXyz* sePos;
mDoAud_monsSeStart(0x4991, sePos = &eyePos, fopAcM_GetID(this), 0, ...);
```

**MWCC materialises an argument expression in right-to-left ARG order. A local
initialised at its declaration is materialised at the DECLARATION** — here, 15
instructions early. Assigning *inside the argument list* puts it where the
donor has it. Swept: initialise-at-declaration 12 (baseline), no local at all
30, declare-after-the-call 30, reference local 12, **assign-in-arg-list 3**.

### Why this changes the ask

This document's central claim was that everything left in `so`/`ob1`/`p2` is a
*compiler allocation decision* my technique cannot touch. **That claim has now
failed twice in one session** — `aj1`'s `lookBack` (42 rows, three ordinary
defects) and `so`'s `checkTgHit` (12 -> 3, a source-level lever). Both were
filed here as evidence *for* the park.

**What still stands:** `so`'s `_createHeap` (12), `_nodeControl` (5) and
`modeNearSwim` (8) were re-checked directly and ARE pure register-*name* swaps —
identical instructions, offsets and stack layout — and the decl/init lever was
tested on `_nodeControl` and did nothing (3 forms). Those are real.

**What I am now saying, plainly:** I recommended a park on evidence that
included at least two functions I had mis-diagnosed. **I am withdrawing the
recommendation until `so`, `ob1` and `p2` have had the same re-audit `aj1` got**
— correct-version asm, and the four levers found this session (decl/init
separation, assign-in-arg-list, object's-own-setter, wrong-section statics).
Tracked as task #5.

**The one item unchanged throughout: History's `dNpc_HIO_c` vtable scoping.**
Known cause, known fix, shared header, 13 rows on `so`. Still the highest-value
item and still blocked on a lane decision rather than technique.
