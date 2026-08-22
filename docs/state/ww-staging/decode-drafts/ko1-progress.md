# d_a_npc_ko1 — Joel & Zill (Outset). Opened 2026-08-18.

**5/203 (1.9240%) -> 46/203 (6.9208%).**

## Foundation (do this first on any fresh NPC TU)

1. **Base class.** The skeleton had `daNpc_Ko1_c : public fopAc_ac_c`. It is
   `fopNpc_npc_c` — evidence is **29 references to `0x330` (`mpMorf`)** in the
   target asm, which is the `fopNpc_npc_c` signature.
2. **Class size, from the profile.** `g_profile_NPC_KO1`'s Size field is
   **0x8AC**. Mine emitted 0x478 (= `fopAc_ac_c` + 0x1E8) before the fix.
   `fopNpc_npc_c` is 0x6C4, so ko1's own members span **0x6C4..0x8AC = 0x1E8**.
   **Check this in the emitted `.data`, not by inference** — my first attempt
   looked fine because the build had FAILED and ninja left a stale `.o`.
3. **Three virtuals must match the base to compile at all:**
   `next_msgStatus` -> `u16`, `getMsg` -> `u32`, `anmAtr(u16)`.
4. **Carve the member block from access widths** before writing bodies. Every
   load/store displacement in [0x6C4, 0x8AC) with its width, plus
   `extsb`-after-`lbz` for signedness — 118 offsets. Unknown regions stay as
   **sized byte arrays with TODOs** (0x6D4, 0x73C, 0x748, 0x84C), never guessed
   types: a wrong guess compiles and silently misplaces everything after it.

**The carve paid immediately:** `field_0x8a6` was pinned as `s8` from 43
sign-extended loads before I knew what it did; it turned out to be the KO1/KO2
discriminator that `_create` indexes `a_size_tbl` with and that `charDecide`
sets.

## `_create` first — 31 functions exact in one edit

`fopAcM_SetupActor(this, daNpc_Ko1_c)` expands the ctor chain and instantiates
the whole inherited tail at once: cXyz / cBgS_PolyInfo / dCcD_Cyl / cM3dGCyl /
cCcD_ShapeAttr / cM3dGAab / dBgS_ObjAcch destructors, every `cCcD_CylAttr` and
`cCcD_ShapeAttr` CrossAtTg/CrossCo/GetCoCP virtual, four `@NNN@` dtor thunks,
and the three `fopNpc_npc_c` message virtuals. **Same cascade as aj1. This is
why the create chain goes first on a fresh TU, not last.**

Constants read off the target: resource name **`"Ko"`** (`stringBase0+8` — `+0`
is `"Ko1"`, `+4` is `"Ko2"`), `a_size_tbl` = two entries of `0x272E0`, cull box
`(-50, -20, -50, 50, 120, 50)`, and the ob1-style merged tail
`return createInit() ? state : cPhs_ERROR_e;`.

## Exact so far

`_create`'s 31, plus `CheckCreateHeap`, `charDecide` (88 instructions, nested
switches, first try), `event_action`, `setPlaySpd`, `endEvent`, `clrSpd`,
`bitCount`, `iniTexPttrnAnm`.

## Signals this TU produced

- **`x >>= 1` and `x = x >> 1` differ for a `u8`.** The compound form fuses to
  one `extrwi`; the explicit form promotes to int, `srawi`s, and re-masks —
  three instructions, which is what the donor has. (`bitCount`: `>>=` 4 rows,
  explicit **0**, an `(s8)` cast 4.)
- **Empty-case, third confirmation.** `event_action`'s target is
  `lbz; cmpwi r0, 0x0; li r3, 1; blr` — a compare nothing uses. An empty
  `switch` emits nothing, a lone `default:` emits nothing, **`case 0: break;`
  emits exactly the dead compare.**

## Next, in order

1. **`create_Anm` / `create_hed_Anm` / `create_bln_Anm`** — all three build a
   `new mDoExt_McaMorf(...)`. **⚠ I did NOT finish `create_bln_Anm`: the
   register-to-parameter mapping does not resolve cleanly.** The ctor takes 12
   params (`modelData, cb1, cb2, anmTransform, loopMode, playSpeed, startFrame,
   endFrame, param_8, basAnm, modelFlag, differedDlistFlag`), which is 11
   int-class args — 7 in r4..r10 and 4 on the stack at 0x8..0x14. **The target
   writes FIVE stack slots (0x8, 0xc, 0x10, 0x14, 0x18) and puts `1` in r4
   where `modelData` should be.** Resolve that before writing it; do not guess
   the arg order. Known values: model res `0x1a`, anim res `0x1`, stack words
   `-1, 1, 0, 0x80000, 0x11000022`, `r9 = 2`, and a float constant `@4569`.
   Result stored to `field_0x704`; joint index via
   `JUTNameTab::getIndex(stringBase0+0xd9)` into `field_0x6d0`.
2. **`CreateHeap`** (180 instructions) once those leaves exist.
3. **`createInit`.**

`create_itm_Mdl` sits at 6 rows, **all `@stringBase0` displacements** — my
string pool is a fraction of the target's with ~120 functions unwritten. The
string pool is a reliable ORDER oracle, so these converge as the TU fills. Same
for the three `*_toResID` functions at 3 rows each on the MWCC `$NNNN`
local-static counter.

---

## Update — create chain COMPLETE. 46/203 exact, fuzzy 1.9240% -> 11.5403%.

### The `mDoExt_McaMorf` ctor mapping, resolved

**`mDoExt_McaMorf` has a VIRTUAL BASE**, so its constructor carries an implicit
most-derived flag as the first argument. `li r4, 0x1` is that flag, and **every
declared parameter sits one register later than the signature suggests** — which
is why `modelData` appears in `r5`, and why there are five stack slots for four
stack parameters.

**Ground truth came from a MATCHING TU (`d_a_am`)** whose call is
instruction-for-instruction the same shape, so its source gives the argument
order for free. Checking for a matching sibling took two commands and replaced
an open-ended reverse-engineering job. Same lesson as using `d_a_npc_p1`/`ba1`
as NPC idiom oracles — **look for a matching caller before doing register
archaeology.**

Written, bodies matching, residuals purely `@stringBase0`:

| fn | rows | model res | anim res | differedDlistFlag |
|---|---|---|---|---|
| `create_Anm` | 23 | 0x1B | 0x16 | **0x15021222** |
| `create_hed_Anm` | 12 | tbl {0x1C, 0x1D} | tbl {0x0A, 0x25} | **0x11020022** |
| `create_bln_Anm` | 18 | 0x1A | 0x01 | **0x11000022** |

**Each anim carries its OWN differedDlistFlag** — easy to assume they are
identical; they are not.

### Assert strings name members — six so far

`m_hed_jnt_num` ("head"), `m_bbone_jnt_num` ("backbone"), `m_armR2_jnt_num`
("armR2"), `m_bln_loc_jnt_num` ("balloon_loc"), `m_bln_jnt_num` ("ko_balloon"),
`m_hed_2_jnt_num` ("head2").

**This is the cheapest naming source in the campaign and it cannot be wrong** —
the donor's own assert text states the field name. It also CAUGHT one of my
carve guesses: I had put `m_bln_jnt_num` at 0x6CD; it is 0x6D1.

### NEXT: `CreateHeap` (180 instructions) — analysis done, not yet written

Call order and captures:
1. `create_Anm()` -> `r30` (its modelData). NULL -> return FALSE.
2. `create_hed_Anm()` -> `r31`. NULL -> `mpMorf = NULL`, return FALSE.
3. A **local `u8[2]` copied from a `@7337` u16 constant**, indexed by
   `field_0x8a6`, stored to `field_0x89e` — a non-static local array with an
   initialiser, which MWCC materialises from .rodata onto the stack.
4. `iniTexPttrnAnm(0)`. FALSE -> `mpMorf = NULL`, `field_0x710 = NULL`, return FALSE.
5. `if (field_0x8a6 == 0)` -> `create_bln_Anm()`; NULL -> `mpMorf = NULL`,
   `field_0x710 = NULL`, return FALSE.
6. `create_itm_Mdl()`; FALSE -> the shared failure tail.
7. **Two joint-callback loops.** Each walks `i` from 0 while
   `i < <modelData>->getJointNum()` (`lhz 0x28`), and when `i` equals the joint
   number, does
   `<morf>->getModel()->getModelData()->getJointNodePointer(i)->setCallBack(cb)`:
   - `field_0x704` (balloon morf) vs `m_bln_loc_jnt_num`, cb `nodeCallBack_Bln`
   - `field_0x710` (head morf) vs `m_hed_2_jnt_num`, cb `nodeCallBack_Hed`
   Then `field_0x704->getModel()->setUserArea((u32)this)`.
   **⚠ Both loops are bounded by `r31`'s joint count (the HEAD modelData) while
   indexing a different model.** That reads like a donor quirk rather than a
   transcription error on my part — verify against the asm before "fixing" it.
   The sibling idiom is `modelData->getJointNodePointer(n)->setCallBack(cb)`
   (see `so` lines 241-242, `aj1` 1453-1455).

---

## Update — 80/203 exact, 25.37%. Five layers complete.

**Done and byte-exact:** foundation, create chain (10), init dispatch (9),
animation (setAnm / setAnm_NUM / setAnm_ATR / setAnm_tex / chg_anmTag /
control_anmTag / control_anmAtr), messaging (all 7 getMsg_*), plus event_proc,
isEventEntry, chk_partsNotMove, checkOrder, eventOrder, down_1, wait_3,
searchByID, bitCount, clrSpd, endEvent, setPlaySpd, event_action.

**Residuals, all with named causes:** setAnm_anm and set_balloonAnm_anm 2 rows
each (one shared register-reuse idiom on an int->float convert, int-local
workaround measured WORSE); create_Anm 23 / create_bln_Anm 18 / create_hed_Anm
12 / create_itm_Mdl 6 / event_actionInit 1 — every one of those is
`@stringBase0` position and converges as the TU fills.

### ⚠ NEXT UNLOCK: the HIO, and it gates a whole family

`daNpc_Ko1_HIO_c` and `daNpc_Ko1_childHIO_c` are still **empty stubs** with no
members, and there is no `l_HIO` global yet. Established so far from
`setAttention`:

- `l_HIO` is indexed by **`field_0x8a6`** (the KO1/KO2 discriminator) with a
  stride of **0x60** — so `daNpc_Ko1_childHIO_c` is 0x60 bytes and the children
  array starts at `l_HIO + 0`.
- `+0x24` within a child is an `f32` (the attention-height offset).

Derive the rest from `__ct__20daNpc_Ko1_childHIO_cFv` and
`__ct__15daNpc_Ko1_HIO_cFv` the same way `aj1`'s HIO was done — the ctor writes
every default, so it is a complete field-by-field spec.

**This gates more than `setAttention`:** any function reading tuning parameters
goes through `l_HIO`, so it is worth doing before the `wait_*`/`walk_*`/`swim_*`
bulk rather than after.

### Also still TODO in the carve

`0x6D4` (anm sub-objects) and `0x84C` (s16/u16 block) remain sized byte arrays.
Every member needed so far has split cleanly out of one, and `sizeof` has held
at 0x8AC through every split.

---

## Session close — 88/203 exact, 27.3998% (opened at 5/203, 1.9240%)

**The HIO unlock paid: +6 exact from one header edit.** Layout derived entirely
from `__ct__15daNpc_Ko1_HIO_cFv`, which is a complete spec because the ctor
writes every default:

```
__construct_array(this+0xC, ctor, dtor, 0x60, 2)   -> children[2], stride 0x60
loop i<2: *(this + 0x68 + i*0x60) = i              -> child + 0x5C = int
          memcpy(this + 0x10 + i*0x60, tbl, 0x58)  -> child + 0x04, 0x58 prm
stb -1, 0x4(this); stw -1, 0x8(this)
```

`daNpc_Ko1_prm_c` then carved to 27 fields by walking every register that holds
`l_HIO` and mapping displacement N to prm offset N-0x10.

**⚠ The prm family is similar but NOT identical across TUs** — `aj1`'s is 0x30,
`ko1`'s is 0x58. Borrow the sibling's NAMES only after the offsets independently
agree; copying the struct would misplace every field. Same trap as `anm_prm_c`
(`ko1`'s is `aj1`'s plus one trailing `int`).

### Two return-shape signals, which point OPPOSITE ways

- **One `li r3, N` in the target where you emit two** -> fold the returns into a
  merged tail (`ob1::_create`: `return createInit() ? state : cPhs_ERROR_e;`).
- **Two `li r3, N` where you emit one** -> the donor wrote an explicit
  `return N;` INSIDE the branch; MWCC does not tail-merge them (`ko1::wait_2`,
  4 rows -> 0).

Hold them together or you will apply the wrong one.

### Where the next pass starts

`wait_1` (98 instructions) is the head of the state machine and needs more of the
`0x856` block carved — it reads `field_0x858` and `field_0x8a5`, and branches on
`field_0x89f`. The rest of `wait_*`/`walk_*`/`swim_*`/`attk_*`/`talk_*` follow the
same shapes.

**Carve TODOs left:** `0x6D4` (anm sub-objects), `0x856..0x85E`. Every member
needed so far has split cleanly out of a sized array and `sizeof` has held at
0x8AC through every split — the conservative carve has not cost a single retry.

---

## The state-function recipe — apply these UP FRONT, not after the diff

`ko1` 91/203 (30.2743%). `wait_6` (94 instructions) was the first written with
no correction pass, because by then the family's signals were known. Anyone
picking up `wait_5/7/9/a`, `walk_*`, `swim_*`, `attk_*`, `talk_*` should apply
all of these before the first build:

1. **Every branch carries its own explicit `return 1;`.** MWCC does NOT
   tail-merge them, so each shows as a separate `li r3, 0x1`. **Count the
   `li r3, 0x1` in the target and give exactly that many branches a return.**
   Cost me 4 rows on `wait_2` and 2 on `wait_4` before I generalised it.
2. **`cntlzw` + `extrwi` around a bool result means the source wrote
   `== false`,** not `!x`. The two are not interchangeable under MWCC.
3. **`l_HIO` displacement N maps to `l_HIO.children[field_0x8a6].mPrm.field_0x(N-0x10)`**
   — children at +0xC, mPrm at child +0x4. Confirmed from three independent
   consumers (`setAttention`, `wait_4`, `wait_6`).
4. The common opening is
   `if (field_0x875 != 0) { if (chk_talk() [&& chk_manzai_1()]) setStt(3); return 1; }`.
5. `field_0x6bc` (inherited from `fopNpc_npc_c`) == 1 -> set it 2 and `setStt(0x14)`.

**The general shape of this cost curve is worth knowing:** the first few
functions in a family take several correction rounds each; once the signals are
extracted the rest land first try. It is worth spending the rounds on the early
ones rather than skipping to the biggest.

### Next

`wait_7` (159 instructions) is the largest of the family and adds a
`searchByID(field_0x7b4)` + `JUT_ASSERT` prologue before the shared opening.


## setStt() - WRITTEN 2026-08-21, 395 rows -> 17 (all pool position)

**Status: done, LOGIC-EXACT.** Committed WWDP `8bb915d7`. The decode below is
kept verbatim because it was correct; what it did NOT cover is recorded at the
end of this section under RESOLVED WHEN WRITTEN. Original heading:
'structure DECODED, body NOT yet written'.

The last function in ko1 above 100 rows (395). `tools/foundry/jump_table.py`
recovered its whole shape in one pass, so whoever writes it does not re-derive
this.

PROLOGUE (from the asm, ahead of the table):
    fopAc_ac_c* a_partner = searchByID(field_0x7b4[0]);   // r28, used by 0x11 / 0x13
    s8 prev = field_0x8a3;                                // r30, saved BEFORE the store
    field_0x84e = 0;
    field_0x8a3 = i_stt;                                  // the parameter
    switch (field_0x8a3) { ...

TABLE (@6033, 30 entries, base 0, 14 distinct bodies). Cases below are in SOURCE
order - sorted by label ADDRESS, not by value - because MWCC emits case bodies
in the order they were written:

=== @6033: 30 entries, 14 distinct bodies, default=.L_00004550
--- cases in SOURCE order (sorted by label address) ---
  case 0x3:  -> NOT a simple store, read it: li r0, 0x1 | stb r0, 0x8a5(r29) | stb r0, 0x29a(r29)
  case 0x4 0xB:  -> NOT a simple store, read it: cmpwi r5, 0xb | beq .L_00004654 | bge .L_00004680
        ^ 2 values share this body: FALL-THROUGH case list
  case 0x5:  -> NOT a simple store, read it: stb r4, 0x8a5(r29) | stb r4, 0x876(r29) | stb r4, 0x8a2(r29)
  case 0x6 0xD 0x17:  -> NOT a simple store, read it: cmpwi r5, 0x6 | beq .L_000046F0 | b .L_0000470C
        ^ 3 values share this body: FALL-THROUGH case list
  case 0x7:  -> NOT a simple store, read it: lwz r0, 0x73c(r29) | cmplwi r0, 0x0 | beq .L_00004770
  case 0x8:  -> NOT a simple store, read it: lwz r4, 0x744(r29) | cmplwi r4, 0x0 | beq .L_000047D0
  case 0x9 0x10 0x12 0x19:  -> NOT a simple store, read it: li r0, 0x0 | stb r0, 0x8a5(r29) | stb r0, 0x876(r29)
        ^ 4 values share this body: FALL-THROUGH case list
  case 0xA 0xE 0x1A:  -> NOT a simple store, read it: li r0, 0x1 | stb r0, 0x8a5(r29) | li r0, 0x0
        ^ 3 values share this body: FALL-THROUGH case list
  case 0xF 0x18:  -> NOT a simple store, read it: lfs f0, 0x7c4(r29) | stfs f0, 0x7f4(r29) | lfs f0, 0x7c8(r29)
        ^ 2 values share this body: FALL-THROUGH case list
  case 0x11:  -> NOT a simple store, read it: cmplwi r28, 0x0 | bne .L_000049A4 | bl getSDevice__12JUTAssertionFv
  case 0x13:  -> NOT a simple store, read it: cmplwi r28, 0x0 | beq .L_00004A8C | li r0, 0x2
  case 0x14:  -> NOT a simple store, read it: li r0, 0xff | stb r0, 0x89c(r29) | stb r0, 0x89d(r29)
  case 0x1B 0x1C:  -> NOT a simple store, read it: lfs f1, 0x13c(r31) | bl cM_rndF__Ff | lfs f0, 0x13c(r31)
        ^ 2 values share this body: FALL-THROUGH case list
  case 0x0 0x15:  -> NOT a simple store, read it: mr r3, r29 | bl setAnm__11daNpc_Ko1_cFv | addi r11, r1, 0x40
        ^ 2 values share this body: FALL-THROUGH case list

CAUTION on the tool's `default=.L_00004550`: that is the MOST FREQUENT target,
which is how the tool picks a default, but its body immediately re-tests r5 for
2 and then 1. Values 0x1, 0x2, 0xC, 0x16, 0x1D all arrive there and are
discriminated afterwards. Read that block before assuming a bare `default:` -
the heuristic is documented as a heuristic for exactly this case.

Starting point: write the 14 bodies in the listed order, build, let objdiff place
the rest. The fall-through groups are already correct - read off shared labels,
not guessed.


### setStt RESOLVED WHEN WRITTEN (2026-08-21) - three things the decode missed

1. **THE SHARED TAILS ARE STRUCTURAL - `goto`, not a compiler tail-merge.**
   Three tails are reached both by fall-in from a later case and by a forward
   branch from an earlier one (`.L_00004908` from the 4/0xB group,
   `.L_000049FC` from the 6/0xD/0x17 group, `.L_00004808` from case 7).
   Writing the statements out in full at both sites measured **66
   DIFF_INSERT rows: MWCC does not cross-jump here**, so identical duplicated
   tails stay duplicated. Rewiring them as three labels (`set_prm_1/2/3`)
   reached by `goto` took the function from 98 rows to 17. `goto` has
   precedent in 43 files of this repo. **Generalisable: a forward branch into
   a later case's tail is a source-level `goto`, not an optimisation - test it
   by duplicating once and counting inserts.**

2. **EVERY `l_HIO` REFERENCE IS +0x10 FROM THE DISPLACEMENT.**
   `children[]` is at 0x0C in `daNpc_Ko1_HIO_c` and `mPrm` at 0x04 within a
   child, so `lfs 0x34(r3)` means `mPrm.field_0x24`. I took the mapping from a
   matching sibling line (`walk_1` uses `mPrm.field_0x50`) and read it as
   identity - the sibling is consistent with +0x10, it just did not disprove
   identity. Cost 7 rows. **Derive the offset from the header arithmetic, not
   from a sibling call site that happens to fit.**

3. **THE PLAYER IS INDEX 1.** `mPlayerInfo` has stride 8, so 0x5b44/0x5b4c/
   0x5b54 are `dComIfGp_getPlayer(0/1/2)`. The target loads 0x5b4c ->
   `getPlayer(1)`, which `d_a_npc_so_cut.inc` already uses ten times. I
   defaulted to 0 because so.cpp's nearby calls are 0 and 2.

**Remaining 17 rows are pool/stringBase displacement only** (`pool_position.py`
agrees). Literal ORDER is right - 90/180/-4.0/-3.0/-1.6 sit at a uniform
-0xD8 shift - and the two early literals that appear swapped (0.0f vs -4.5f)
are owned by earlier stubs: parse order @4296 (0.0f) before @4483 (-4.5f)
matches the target, so they close as the TU fills in. **Re-check at TU
completion; a survivor there is a real pool-ordering defect.**

**CAUTION on the instrument:** `pool_position.py` runs `objdiff report
generate -p .` and prints `0 LOGIC-EXACT, 0 REAL` from the wrong directory -
a clean-looking all-clear. Run it from `D:/XXXXXXX/WWDP`.


## Round: stubs batch (2026-08-21) — 4 functions written, partner family 5/5 BYTE-EXACT

ko1 **151 -> 156 exact of 203** (exact_delta: +5 by name, nothing lost).
WWDP `c99e44d2`. chk_areaIn and ko_clcSwmSpd are logic-exact (4 pool rows each).

Two rounds cost by not reading the sibling first — recording so the pattern
sticks:
1. **`fopAcM_GetID`'s NULL check IS the inner if/else in the asm.** I wrote the
   check explicitly and got a doubled branch pattern (2 beq on one compare, two
   `li -1` arms). bm1's `partner_srch_sub` — the donor sibling — calls the macro
   bare inside `if (l_check_wrk != 0)`. The asm's apparent nested-if was the
   macro expansion. **When an asm shape looks like source structure, check
   whether a macro the siblings use produces it.**
2. **4 sparse cases over 0..8 emit a COMPARE TREE; the target has a jump
   table.** Explicit empty cases (`case 0: case 2: case 4: case 5: case 8:
   break;`) densify the value set and flip MWCC to the table. The bounds check
   (`cmplwi 0x8 / bgt`) belongs to the table, not the source.
3. Enum-parser validation paid immediately: my sequential fpcNm parser was OFF
   BY ONE (validated against ob1's known KB filter, asm 0xDD vs parsed 0xDE).
   Filters are KO1/KO2/OB1, not JI1/KO1/BM5 as the raw parse claimed. **A
   plausible-looking enum name from an unvalidated parser would have built
   clean and cost a debug round.**

Remaining REAL residuals on ko1 (from pool_position, top of the list):
nodeKo1Control 81 · hana_action4 60 · wait_action3 60 · privateCut 58 ·
ko_setPthPos + smaller stubs. nodeKo1Control has ob1's nodeOb1Control as a
direct template (read it first).


## Stubs batch 2 (2026-08-21, same session): ko1 169 -> 177/203, fuzzy 98.42%

Byte-exact this batch: plyTexPttrnAnm (ob1 verbatim; field_0x72e u16->s16, the
cLib_calcTimer<s> instantiation is the tell) · btpNum_toResID (4-entry static
table; the no-return fall-off compiles to bgelr/bltlr — matches the TU's
"return value expected" warnings) · chk_talk (bm1 named the accessors; ko1 uses
the ret-variable form — the r31 slot in the asm discriminates it from aj1's
early-return form) · getMsg (9-entry table via explicit empty cases 4/8) ·
chkAttention (ob1 verbatim; **u8 return forced an extrwi truncation, target's
bare srwi proves BOOL — the truncation SHAPE at a return site names the
declared type**) · _delete · nodeHedControl + nodeBlnControl (the balloon
matrix relay through field_0x6d4, retyped 0x30-byte blob -> Mtx).

Logic-exact (pool rows only): chk_wallJump (returns f32, header retyped),
chk_start_swim (ChkWaterIn gate, m_wtr.GetHeight - GetGroundH > 62), shadowDraw.

PARKED, do not re-run: **chg_anmAtr 2 rows** — target's equal-early-out is an
unfolded bne-over-b; early-return, goto-to-end-label, do-while-break, and
else-return ALL fold to beq. **get_crsActorID 6 rows** — GetAc()'s
inline-ternary materialization; target keeps the full li-0/lwz merge + re-test,
ours short-circuits mStts==NULL to the -1 arm; if-return and ternary tails
measure identical.

REMAINING on ko1: set_action 42 (the PTMF store) · HIO ctor 34 · ko_nMove 28
(open item 2, bounded) · ko_movPass/wait_7 10 each (open item 3, by-value cXyz
slot — 4 falsified approaches recorded) · setAnm_anm/set_balloonAnm_anm 2 each.


## TU FULLY WRITTEN (2026-08-21): 178/203 exact, fuzzy 99.7225%

`chk_routeAngle` was the last stub: outprod + atan2s with a single shared
180-flip — **the asm's forward-branch INTO the other arm's flip is the `||`
tell** (if/else-if duplicates the block, 4 rows). set_action (ob1 form,
teardown state 9) and the HIO ctor (memcpy loop from a function-local static
prm table, values decoded from .obj bytes) landed the same batch.

**Residual map at completion — this is the whole remaining distance:**
- **18 pool-position functions** (instrument-classified). NOW LIVE per the
  recorded caution: the TU is complete, so these are a REAL POOL-ORDERING
  question. Known component: my `a_prm_tbl` emits BEFORE the @-literal run
  (every constant shifted +0xB0) while the target has it after; the HIO
  ctor's `$NNNN` (mine 1613, target 4154) says the target's internal-symbol
  counter is far higher — inline-instantiation count differs. Section-level
  layout comparison is the next instrument-shaped task.
- **7 REAL rows**: ko_nMove 30 (open item 2, bounded) · ko_movPass/wait_7 12
  each (open item 3, by-value cXyz slot, 4 approaches falsified) ·
  get_crsActorID 6 (GetAc materialization, parked) · chg_anmAtr 2 (unfolded
  branch, parked) · setAnm_anm/set_balloonAnm_anm 2 each (not yet examined).

exact_delta earned its keep AGAIN this batch: a flat 177 hid nodeKo1Control
and setMtx regressing to pool-shift while set_action and the ctor went green
— equal-lost/equal-gained, invisible in the total, named by the tool.


## Post-completion rounds (2026-08-21 late): section placement + ko_nMove

**THE SESSION'S BIGGEST SINGLE LEVER — CONSTNESS IS SECTION PLACEMENT** (WWDP
`b7d96139`): static const -> .rodata, static non-const -> .data. The donor
keeps the prm table MUTABLE and the bck resID tables CONST; mine were both
backwards. The const prm table sat at the FRONT of .rodata shifting 18
functions' pool displacements +0xB0; the non-const bck tables hid in .data
where the 0.09% data match shows nothing. **Diagnosis path: map the split
asm's .obj lines to their TRUE sections (bare `.rodata`/`.data` directives —
a `.section`-only grep MISSES them and misled me once), then diff against the
built .o's symtab.** Result: 180/203, pool residuals 16 fns with smaller
rows, .rodata and .data symbol order now mirror the target.

**ko_nMove 28 -> 23** (`69b621be`): the recorded open item was right — a
per-branch `f32 spdF = speedF;` forces the load before the l_HIO address.
FALSIFIED on the residue (do not re-run): prm reference local (29 — early
address materialization breaks chain CSE), named products + if (34), named
products + ternary (31). Residue = base-vs-index evaluation order + the
product's dest register (f1 vs f0, one extra fmr) — the volatile-FPR
analogue of the mirror tie-break. PARKED.

Remaining REAL: ko_nMove 23 · movPass/wait_7 10 each · get_crsActorID 6 ·
chg_anmAtr 2 · setAnm_anm/set_balloonAnm_anm 2 each. Everything else is
pool-position (16 fns) converging with TU state.


### setAnm_anm 2-row park (2026-08-21): four spellings falsified

Mine emits an extra `lbz 0x860 + extsb` reload before the int-to-float of the
tail (`field_0x824 = field_0x860` after zeroing 860/861); the target converts
the `li 0` register directly, no reload, no extsb. FALSIFIED: `field_0x824 =
0` (33 rows — kills the conversion machinery entirely, so the conversion is
structural), an s8 local for the zero (33), and both chained-assignment
orders (19 each). **The target's source keeps the stored value visibly live
in a way none of the natural spellings reproduce.** set_balloonAnm_anm shares
the shape — same park. These two plus chg_anmAtr's unfolded branch form a
family: 2-row shape puzzles where every obvious spelling is falsified;
worth one dedicated session with a transform-search harness, not more
hand probes.


### ko_movPass/wait_7 slot question: FIFTH falsification, formally parked

The 10-row diff is a clean two-slot swap: the by-value `current.pos` arg temp
(chkPointPass) wants the HIGH slot 0x24 and `flat` the LOW 0xc; ours invert.
Frames match. Added to the handoff's four falsified approaches: **block-scoping
the flat/delta cluster is NEUTRAL** — named locals hoist ahead of call temps
regardless of scope depth. The target allocates the CALL TEMP first, which no
source spelling tried so far reproduces.

**PARKED into the transform-search family** with chg_anmAtr (unfolded branch),
setAnm_anm/set_balloonAnm_anm (live-value conversion), get_crsActorID (GetAc
materialization), and ko_nMove's residue (FPR dest tie-break). Six functions,
~45 rows, every hand-probe ledgered. The next attack on any of them should be
the automated transform-search harness the p2 anchor already names as a live
path — one engine, six targets.


## Cross-TU data audit round (2026-08-22): 180 -> 183/203

**The assert-string lever extends to LOCALS.** The @stringBase0 pool was
2 bytes short vs target: donor's four model-data asserts read
`a_mdl_dat != 0`, mine `mdlData != 0` — JUT_ASSERT stringifies the local
VARIABLE NAME, so donor local names are recoverable from assert text and
MUST be adopted. One rename → pool byte-equal (content AND order) and
+3 exact (WWDP 518fbb14). kg1 had the sibling defect (literal "Kg" where
donor routes m_arcname; 55 -> 56/66).

**objdiff's blind spot, proven twice today:** function scores do not
police string-pool content or .data emission order. tag_ba1 was 17/17
code-100.00 and STILL missed the REL SHA until l_evn_tbl moved below the
HIO ctor (function-local a_prm_tbl emits at the ctor's DEFINITION
position — .data order follows definition order, not use). Three TUs
now SHA-verified byte-true and flipped MatchingFor("GZLE01"):
aygr / fallrock_tag / tag_ba1 (WWDP 762b5faa). The dead .data extras in
every REL build (PCH Vec aggregates + sqrtf statics) are dropped by
-strip_partial at link — benign, confirmed.

ko1's remaining distance: 6 harness parks (~45 rows, all falsified) +
the pool-position stragglers now down near the noise floor. The ko1
SHA flip should be attempted once the parks close.


### setAnm_anm park: falsifications SIX and SEVEN (2026-08-22)

Target tail re-read: `li r0,0; stb 860; stb 861;` then the SIGNED
int->float magic consumes THE SAME r0 — donor's compiler store-forwards
the known constant and folds the extsb of 0; mine reloads (lbz+extsb).
Two spellings the original four missed, both now FALSIFIED (measured
WORSE than the parked 96.77/95.83):
- copy-chain `860=0; 861=860; 824=861;` -> 95.0 / 93.5
- source-swap `860=0; 861=0; 824=861;` (adjacent-store forward) ->
  95.2 / 95.6
The xoris signed-magic also KILLS the u8-retype theory (a u8 source
would take the unsigned path). Whatever makes the donor forward is not
reachable from the assignment's shape — sweep-harness slot sets should
still include these tails for interaction coverage, but no more hand
probes on this pair.
