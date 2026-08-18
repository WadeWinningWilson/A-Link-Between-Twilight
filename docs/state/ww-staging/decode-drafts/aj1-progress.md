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


## Round 3 - createInit fully transcribed; TWO BLOCKERS FOUND IN THE HEADER

Pulled the donor's `createInit` in full: **118 instructions**, and its shape is
completely legible. But it cannot be written yet, and the reason is upstream.

**BLOCKER 1 - the class has NO MEMBER FIELDS AT ALL.** `include/d/actor/d_a_npc_aj1.h`
is 2.7 KB of method declarations over an empty class body. `createInit` alone
touches at least these offsets, none of which exist yet:

    0x1b5 roomId     0x1b6 polyColor   0x258 f32 (from @4332)
    0x26d, 0x26f (u8 = 0xad)           0x280 (s32 = 0xa)
    0x2c4 dNpc_EventCut_c              0x330 mDoExt_McaMorf*
    0x334 dBgS_Acch                    0x41c cBgS_PolyInfo
    0x538 dCcD_Stts (+0x5b8 back-ptr)  0x574 dCcD_Cyl
    0x70e/0x710/0x712 s16 home angles  0x744 s16 eventIdx
    0x75b u8                           0x7b9 u8 = 9     0x7bf s8 type

**BLOCKER 2 - THE BASE CLASS IS WRONG, and the donor's own call proves it.**
The header says `class daNpc_Aj1_c : public fopAc_ac_c`. But `createInit` does:

    mr r5, r30                     ; r30 == this
    bl setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c

i.e. **`this` is passed where a `fopNpc_npc_c*` is required.** The class must
derive from `fopNpc_npc_c`, not `fopAc_ac_c`. That also explains the 0x26d /
0x26f / 0x280 cluster, which is npc-base territory rather than actor-base.
Every field offset I lay out against the wrong base would be wrong, so this is
strictly first.

**WHAT createInit DOES** (recorded now so the transcription is not lost):

    field_0x75b = (u8)(field_0xb0 >> 8);
    mEventIdx = dComIfGp_evmng_getEventIdx(l_evn_tbl[0], 0xff);
    mEventCut.setActorInfo2("Aj1", this);
    field_0x280 = 0xa;  field_0x26d = 0xad;  field_0x26f = 0xad;
    field_0x7b9 = 9;
    switch ((s8)field_0x7bf) {   // 0/1/2 dispatch, default -> return FALSE
        case 0: ok = init_AJ1_0(); break;
        case 1: ok = init_AJ1_1(); break;
        case 2: ok = init_AJ1_2(); break;
        default: ok = FALSE;
    }
    if (!ok) return FALSE;
    home.angle = shape_angle (x/y/z copied via 0x70e/0x710/0x712);
    current.angle = the same three;
    field_0x258 = @4332;
    mStts.Init(0xff, 0xff, this);  mCcCyl.Set(dNpc_cyl_src);
    play_animation();
    mAcch.CrrPos(dComIfG_Bgsp());
    field_0x1b5 = GetRoomId(mPolyInfo);  field_0x1b6 = GetPolyColor(mPolyInfo);
    mpMorf->setMorf(@4187);
    setMtx(true);
    return TRUE;

**ALSO: `init_AJ1_0/1/2` and `createInit` are declared `void` and must return a
BOOL** - the donor tests r3 with `clrlwi. r0, r3, 24` after each. Same shape as
`ob1`'s `init_OB1_0`.

NEXT: fix the base class to `fopNpc_npc_c`, lay out the members above against
that base, then write `createInit`. **No code written yet** - `aj1` remains at
12/131 exact, 2.9112%, and the header work is the gate.


## Round 4 - CORRECTION to Round 2, and the real string map

**Round 2 claimed "`createInit` owns `@stringBase0` offset 0". That was an
artifact of my own regex** - it matched the ANCHOR load
(`addi rN, rN, "@stringBase0"@l`) and not the string actually selected, which
comes from the FOLLOWING `addi rN, rN, 0x<offset>`. `createInit`'s real string
use is **+0x6 = "Aj1"**, not +0x0 = "angry".

Re-derived properly by reading the anchor load AND its follow-on addi:

    createInit        +0x6                       -> "Aj1"
    init_texPttrnAnm  +0xa, +0xd                 -> "Aj", assert file
    setAnm_anm        +0xa                       -> "Aj"
    _create/_delete/demo/itemCreateHeap  +0xa    -> "Aj"
    wait_action2      +0x62
    bodyCreateHeap    +0xa,+0xd,+0x79,+0x91,+0xaf,+0xca

    string offsets: angry@0  Aj1@6  Aj@0xA  d_a_npc_aj1.cpp@0xD
                    a_btp != 0@0x1D  Halt@0x28  AJ1_TLK@0x2D  INI_ANGRY@0x35

**NO FUNCTION REFERENCES +0x0 AT ALL.** So `"angry"` is not emitted by code -
it belongs to a **file-scope static table** declared above the first function.
Same for the `AJ1_TLK...INVITE` block at +0x2D onward: `createInit` loads
`l_evn_tbl` (its own symbol) and dereferences `l_evn_tbl[0]`, so that is a
`static char* l_evn_tbl[]` whose strings emit at the TABLE's declaration
point - which the pool places after `init_texPttrnAnm`'s asserts.

**THE ROUND 2 CONCLUSION SURVIVES, FOR A DIFFERENT REASON.** `createInit` is
still the correct first target - it is the first *function* in string order -
but not because it owns offset 0. The actual pool head is a static table, and
**the file needs that table declared before the first function or every string
offset downstream will be short by 6.**

REVISED WORK ORDER:
1. declare the file-scope static table owning `"angry"` (pool offset 0)
2. `createInit` (+0x6 "Aj1") - transcribed in full in Round 3, ready to write
3. `l_evn_tbl` static table (AJ1_TLK / INI_ANGRY / VIVRATE / JMP / SPPRISE /
   LOK / DAN / INVITE) declared after `init_texPttrnAnm`
4. `init_texPttrnAnm`, then `bodyCreateHeap` (6 string uses, the assert-heavy one)
5. re-run `tools/foundry/pool_align.py d_a_npc_aj1` before any per-function work

DONE THIS ROUND (committed): header now derives from `fopNpc_npc_c` - proven,
not guessed, by the donor passing `this` to
`setActorInfo2__15dNpc_EventCut_cFPcP12fopNpc_npc_c` AND by the base already
declaring `mStts` at 0x538 / `mCyl` at 0x574, the exact offsets `createInit`
uses. Members 0x6C0+ laid out; override return types aligned to the base
virtuals (`u16 next_msgStatus(u32*)`, `u32 getMsg()`, `void anmAtr(u16)`);
`init_AJ1_0/1/2` and `createInit` corrected to return BOOL. Compiles clean.
**Measured effect: NONE yet (12/131, 2.9112%) - bodies are still empty. This
is the gate, not the payoff.**


## Round 5 - FIRST FUNCTION LANDED: createInit is EXACT

**aj1 12/131 -> 13/131 exact, fuzzy 2.9112% -> 5.4911%** (the fuzzy nearly
doubled off one function because `createInit` is one of the larger ones and the
header fix corrected offsets used TU-wide).

Written: `static char* l_evn_tbl[] = {"angry"};` at file scope (the pool head,
proven in Round 4 to be a static table rather than a code reference) plus the
full `createInit` body from the Round 3 transcription.

**FOUR CORRECTIONS the oracle forced, each caught by measurement:**

1. **The derived class starts at 0x6C4, not 0x6C0.** My members landed 4 bytes
   high (mHomeAngle at 0x712 vs the target's 0x70E). `fopNpc_npc_c`'s layout
   comment reads `/* 0x6C0 */ /* vtable */` - the vtable POINTER occupies
   0x6C0..0x6C4, so derived data begins at 0x6C4.
2. **I had the angle copy backwards.** `current` is at 0x1F8 (so `current.angle`
   = 0x204) and `shape_angle` is at 0x20C. The donor reads 0x204 and writes
   0x20C, i.e. **`mHomeAngle = current.angle; shape_angle = mHomeAngle;`** - I
   had written it the other way round from the field names alone.
3. **`init_AJ1_0/1/2` return `bool`, not `BOOL`.** The last remaining row was
   `clrlwi. r0, r3, 24` (mine: `cmpwi r3, 0x0`) - clrlwi against 24 tests a
   BYTE, so the return type is byte-sized. Changing the three to `bool` took
   the function to zero rows. **`BOOL` vs `bool` is visible in the diff and is
   worth checking first whenever a boolean test row is the only survivor.**
4. `attention_info` at 0x26C explains the 0x26D/0x26F/0x280 cluster:
   `distances[1]`, `distances[3]`, and `flags` - and the donor writes **flags
   FIRST**, then the two distances.

Also confirmed while writing: `gravity` (0x258) = **-4.5f**, `mpMorf->setMorf(0.0f)`,
`mCyl.SetStts(&mStts)` comes BEFORE `mCyl.Set(dNpc_cyl_src)` (the reverse of
`so`'s order), and the room/colour pair is the `ob1` idiom verbatim:
`tevStr.mRoomNo = dComIfG_Bgsp()->GetRoomId(mObjAcch.m_gnd)` /
`tevStr.mEnvrIdxOverride = ...GetPolyColor(...)`.

**INSTRUMENT CAVEAT:** `pools.py` reports mine as `angry, Aj` where the source
plainly has `"Aj1"` - the reader truncates the final entry when the object size
lands short. **`createInit` being byte-EXACT is the ground truth; the reader is
wrong, not the code.** Worth fixing in `tools/foundry/pool_align.py` before
anyone trusts its tail entry.

NEXT: `init_texPttrnAnm` (+0xa "Aj", +0xd assert file) then `bodyCreateHeap`
(six string uses). Both are named in the string map in Round 4.
