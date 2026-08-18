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
