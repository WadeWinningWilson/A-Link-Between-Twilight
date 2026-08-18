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
