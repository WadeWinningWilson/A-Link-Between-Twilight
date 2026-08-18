# d_a_npc_aj1 - decode campaign

## Round 1 - campaign opened, baseline measured

Opened `aj1` while `so`/`ob1`/`p2` are blocked on the park ruling (they are
documented, committed and paused on an explicit ask in chat - not abandoned).
`aj1` is next in the standing queue and is blocked by neither ruling.

**BASELINE, script-computed:** `d_a_npc_aj1` **12/131 exact, 2.9112% fuzzy.**
Source is 502 lines / 13.8 KB with **84 stub definitions and empty bodies.**

    STRINGS: target 24 | mine 0     (nothing emitted yet)
    RODATA : target 38 objects | mine 0

The 12 "exact" functions are trivia - the five `daNpc_Aj1_Create/Delete/
Execute/Draw/IsDelete` wrappers and small `cut_init_*` stubs that happen to be
empty in the donor too. **Effectively this TU is undecoded.**

LARGEST UNDONE (by target size):

    bodyCreateHeap   820   next_msgStatus  468   call_1        408
    _create          596   privateCut      448   chk_areaIN    380
    _execute         552   createInit      440

## The opening play, and why

**Write the CREATE CHAIN first: `createInit` -> `_create` -> `bodyCreateHeap`.**

This is not a guess - it is the `so` precedent. On `so`, writing the create
chain collapsed the string pool and **snapped 56 functions to exact in one
step** (77 -> 133), because the asserts and resource names in those functions
populate `@stringBase0` and every later function's string offsets depend on it.
With `aj1`'s string pool currently EMPTY (0 of 24), every string-referencing
function in the TU is guaranteed wrong until the create chain exists.

**Order of work:**
1. create chain (populates the string pool + `.rodata` head)
2. re-run `tools/foundry/pool_align.py d_a_npc_aj1` and fix placement BEFORE
   touching individual functions - placement paid ~36 exact functions on `so`
   and per-function row-chasing paid almost nothing
3. only then the per-function work, smallest-first

**Carry forward from `so`/`ob1`, already paid for:**
- The MWCC allocation bucket (register colouring, stack-slot order,
  literal-pool position) is NOT worth chasing per-function - see
  `DECODER-PARK-PROPOSAL.md` for ~12 measured failures.
- Do not tune cflags: 314 `ActorRel(Matching)` prove `cflags_rel` correct.
- `d_a_npc_p1` and `d_a_npc_ba1` are byte-identical NPC-family oracles - read
  them per-idiom, but do NOT draw blanket style rules from them (I did, and
  measured it false).


## Round 2 - the opening play is CONFIRMED by the donor's own layout

Before writing anything I checked whether create-chain-first was actually right
for THIS TU rather than assuming the `so` precedent transfers. It is:

**`createInit` owns `@stringBase0` offset 0.** The donor's string pool opens
with the strings that function emits, so every string-referencing function
downstream depends on `createInit` existing first. Writing anything else first
would bake in wrong offsets and have to be redone.

**The target's full string order IS the campaign roadmap** - it names the
compile order of the string-producing functions:

     0 angry                        <- createInit (pool head)
     1 Aj1                          <- arc names
     2 Aj
     3 d_a_npc_aj1.cpp              <- assert file, first assert site
     4 a_btp != 0                   <- init_texPttrnAnm
     5 Halt
     6 AJ1_TLK                      <- cut / event names
     7 INI_ANGRY
     8 VIVRATE   9 JMP   10 SPPRISE   11 LOK   12 DAN   13 INVITE
    14 Ojhous2                      <- stage-name check
    15 a_mdl_dat != 0               <- bodyCreateHeap starts here
    16 head    17 m_hed_jnt_num >= 0
    18 backbone 19 m_bbone_jnt_num >= 0
    20 handL   21 m_hnd_L_jnt_num >= 0
    22 footL   23 m_fot_L_jnt_num >= 0

**NOTE the correction to my own Round 1 plan:** I wrote "create chain first:
createInit -> _create -> bodyCreateHeap". The pool says `bodyCreateHeap`'s
strings land at 15-23, i.e. LATE - it is not part of the pool head at all.
The true order is **createInit, then the arc/anm accessors, then
init_texPttrnAnm, then the cut-name functions, and bodyCreateHeap only after
that.** `_create` does not appear in the string pool head either. So the
opening is `createInit` ALONE, then re-measure - not a three-function chain.

WORK ORDER, revised from measurement rather than precedent:
1. `createInit` (440 bytes) - owns pool offset 0, unblocks everything
2. re-run `tools/foundry/pool_align.py d_a_npc_aj1`; expect the string pool to
   go from 0/24 to a partial match, and fix any placement divergence THEN
3. follow the string order outward (arc accessors -> init_texPttrnAnm ->
   cut-name functions -> bodyCreateHeap)
4. per-function work smallest-first only after the pools stop moving

STATE: no code written yet. `aj1` untouched at 12/131 exact, 2.9112%.
