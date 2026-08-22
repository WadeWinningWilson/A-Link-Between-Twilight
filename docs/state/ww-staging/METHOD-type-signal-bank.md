# METHOD — the type-signal bank: reading source constructs off an asm diff

era: era-independent
<!-- era rationale: decomp-decode doctrine, lane-independent | promoted by Foundry 2026-08-21 from Decoder's rows 440/441; source of record aj1-progress.md §Round 19 -->

**Provenance:** authored by **Decoder** across the `so` / `ob1` / `p2` / `aj1`
campaigns (rows 440–441; worked examples in
`decode-drafts/aj1-progress.md` §Round 19). Promoted to a standalone doc by
Foundry because reusable method was stranded inside a per-TU progress file —
the same shape as Engine's trace method before the user rescued it.
**Decoder owns the content; report corrections to them. Numbers below are
their measurements, not estimates.** Receipts: `aj1` went 12/131 → 68/131
exact (2.91% → 64.70%) in one session on these signals alone.

## The bank — ten diff symptoms that name the source construct

| symptom in the diff | what the source actually says |
|---|---|
| `clrlwi. rX,rY,24` testing a result | the callee returns **`bool`** (byte), not `BOOL` |
| an extra `clrlwi` on YOUR return only | your local is `bool` where the donor's is **`int`** (a bool re-narrows on return) |
| `clrlwi rX,rY,16` at a CALL SITE | the callee returns **`int`**, truncated by a u16 parameter |
| `extsb.`/`cmpwi` where you emit `cmplwi` | the field is **`s8`**, not `u8` — **paid five times on one TU; one field fix closed two functions at once** |
| `extsb` at a CALL-ARGUMENT position | **sharpened 2026-08-21 (Decoder):** this is a MISMATCH ARTIFACT, not by itself proof of the field's signedness. A **bare `lbz`** feeding a signed-char parameter proves the source FIELD is `s8`. **`lbz` followed by `extsb`** at that argument means the field is `u8` and the PARAMETER is signed — the extend is the u8→s8 conversion, not evidence about the field. Proven three-for-three: retyping one `anm_prm_c` field `u8`→`s8` flipped `setAnm`/`setAnm_ATR`/`setAnm_NUM` byte-exact at once (`ym1`, WWDP `8d131e4a`). |
| `cntlzw` around a bool test | the source wrote **`== false`**, NOT `!x` — not interchangeable under MWCC (`!x` = direct inverted branch; `== false` materialises the negation) |
| `memcpy` where you emit field stores | the copy was an explicit `memcpy` CALL — **but this does NOT mean the member is an array** (Decoder concluded that once, wrongly; check the READ sites — a later `lfs` out of the block proved a mixed struct) |
| bodies right but ordered wrong | flip **which side of the condition is the then-branch** (three instances in one session: `so` `modeEventFirstWait`, `ob1` `control_anmAtr`, `aj1` `next_msgStatus`) |
| jump-table cases in the wrong order | source case order is **body ADDRESS order**, not numeric — and a body ending WITHOUT a branch is a deliberate `case` fall-through (`aj1` `talk_1`) |
| a global re-loaded at each use | the donor **re-calls the accessor** — do not hoist into a local (cost 11 rows in `chk_areaIN` by caching `dComIfGp_getPlayer(0)`) |

## The single-case switch crack (row 441)

Residual shape: donor emits `beq BODY ; b END ; BODY:` where a plain `if`
emits `beq END ; <body>` — one extra instruction. **Fix:**
`switch (field) { case 0: …; break; }` on **the integer VALUE itself**.
Four earlier attempts failed because they switched on a **comparison**
(`switch (a != b)`), which forces MWCC to materialise a boolean and makes it
worse. Genuinely inapplicable where the condition compares two variables
(`i_no != field_0x800`) — `ob1` `chg_anmAtr` and `aj1` `setAnm_anm` stay open
for that known reason.

## Smaller signals, all measured

- Ternary polarity sets the BASE CONSTANT: `x ? A : A-1` computes 0/+1 and
  adds A−1; `x == 0 ? A-1 : A` computes 0/−1 and adds A. Off-by-one in the
  added constant → flip the ternary.
- An explicit empty `case` shows only in the dispatch tree, never the body.
- A stale float register at a call site is a REAL argument (`shadowDraw`
  never reloads `f3` because `current.pos.y` IS the third float arg).
- Frame size too small → a missing or wrongly-`static` local.

## The one that NEVER pays — treat as a park signal

**Register colouring: ~17 measured attempts across four TUs, ZERO
successes.** Declaration order, local aliasing, hoisting, up-front locals
(measured 42 → 45 rows, WORSE), holding a member in a named local
(`setMtx` 8 → 18 rows, frame grew) — all null. **A colouring-only diff is a
park candidate, not a puzzle.**
