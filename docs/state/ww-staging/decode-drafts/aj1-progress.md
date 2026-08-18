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


## Round 6 - two more functions; aj1 12/131 -> 14/131, fuzzy 2.9112% -> 7.1231%

    createInit         100.00%  EXACT
    btpResID           100.00%  EXACT
    init_texPttrnAnm    98.43%  (2 rows)

**`btpResID`** is a five-instruction table lookup:
`static int a_res_id_tbl[] = {7,8,0,2,5,3,6,1,4}; return a_res_id_tbl[i_no];`
NOTE: its diff initially showed 3 rows that were ONLY the local-static's
generated suffix (`a_res_id_tbl$4419` vs my `$1654`). **objdiff normalises those
for the fuzzy score - it reports 100.00% - so do not chase guard-ID suffixes.**
They are a function of how many statics precede in compile order and settle by
themselves as the TU fills in.

**`init_texPttrnAnm`** went 16 -> 2 rows on two typing corrections, both read
off the instruction stream rather than guessed:

- `btpResID` returns **`int`, not `u16`**. The target does
  `bl btpResID; mr r0, r3; clrlwi r4, r0, 16` - it ZERO-EXTENDS the result at
  the call site, which only happens if the callee returned something wider than
  the `u16` that `dComIfG_getObjectIDRes` wants. A `u16` return would need no
  `clrlwi`.
- `init_texPttrnAnm` returns **`BOOL`, not `bool`** - my byte-sized return added
  a `clrlwi r3, r0, 24` the target does not have.

Fixing those two also dissolved a five-row register-allocation difference
(r28/r29/r30/r31 assignment) **without touching it directly** - worth noting,
because I spent a lot of the `so`/`ob1` endgame trying to attack colouring
head-on. Here the colouring was a SYMPTOM of wrong types, not a cause.

**STILL OPEN on `init_texPttrnAnm` (2 rows):** the target ends
`subic r0, r3, 0x1; subfe r3, r0, r3` - a full-word 0/1 normalisation with NO
byte narrowing. Three combinations tried and measured:

    bool return + plain return init(...)   -> subfe into r0 + clrlwi  (2 rows)
    BOOL return + plain return init(...)   -> no subic/subfe at all   (2 rows)
    BOOL return + return init(...) != 0    -> subfe into r0 + clrlwi  (2 rows)

In C++ `x != 0` yields `bool`, so the narrowing keeps reappearing. The donor
gets an int-width 0/1 some other way. Kept the `!= 0` form as the semantically
correct one. **Do not re-run those three.**

Members added this round: `mBtpAnm` (mDoExt_btpAnm, 0x14 bytes) at **0x6D8**,
`field_0x6ec` u8, `field_0x6ee` s16, `field_0x7b8` s8 - the same
mBtpAnm/byte/halfword shape `so` has at 0x854/0x868/0x86C.

NEXT: `bodyCreateHeap` (820 bytes, six string uses - the largest undone).


## Round 7 - bodyCreateHeap fully transcribed (215 instrs), NOT written, one open question

Pulled all 215 instructions. The shape is unambiguous and the semantics are
recorded below so the write is mechanical next session. **I did not write it**
- one detail does not reconcile and I would rather leave a clean transcription
than a half-correct 215-instruction function.

**SEMANTICS:**

    a_mdl_dat = (J3DModelData*)dComIfG_getObjectIDRes("Aj", 0xA);
    JUT_ASSERT(0x8BE, a_mdl_dat != 0);                  // "a_mdl_dat != 0" @+0x6A
    mpMorf = new mDoExt_McaMorf(...);                   // operator new(0xB4)
    if (mpMorf == NULL) return false;
    if (mpMorf->getModel() == NULL) { mpMorf = NULL; return false; }
    if (!init_texPttrnAnm(0, false)) { mpMorf = NULL; return false; }
    m_hed_jnt_num   = a_mdl_dat->getJointName()->getIndex("head");     // +0x79
    JUT_ASSERT(0x8D3, m_hed_jnt_num >= 0);                             // +0x7E
    m_bbone_jnt_num = ...getIndex("backbone");                         // +0x91
    JUT_ASSERT(0x8D5, ...);                                            // +0x9A
    m_hnd_L_jnt_num = ...getIndex("handL");                            // +0xAF
    JUT_ASSERT(0x8D7, ...);                                            // +0xB5
    m_fot_L_jnt_num = ...getIndex("footL");                            // +0xCA
    JUT_ASSERT(0x8D9, ...);
    ...getJointNodePointer(m_hed_jnt_num)->setCallBack(nodeCB_Head);
    ...getJointNodePointer(m_bbone_jnt_num)->setCallBack(nodeCB_BackBone);
    mpMorf->getModel()->setUserArea((u32)this);
    return true;

Joint-number fields are **0x6CC / 0x6CD / 0x6CE / 0x6CF** (four s8, consecutive)
and must be carved out of the current `field_0x6c4[0x6D8 - 0x6C4]` padding.
`lwz r3, 0x54(r31)` is `a_mdl_dat->getJointName()`.

**THE OPEN QUESTION - do not paper over it.** The McaMorf construction reads:

    li r3, 0xb4 ; bl __nw__FUl ; mr. r0, r3 ; beq <skip>
    ... five stack args at 0x8..0x18: -1, 1, 0, 0x80000, 0x11020022 ...
    li r4, 0x1 ; mr r5, r31 ; li r6,0 ; li r7,0 ; li r8,0 ; li r9,-1
    lfs f1, "@4434"  (= 1.0f) ; li r10, 0
    bl __ct__14mDoExt_McaMorfFP12J3DModelDataP25mDoExt_McaMorfCallBack1_c...

With `this` in r3 (the freshly new'd pointer, never reloaded), **r4 = 1 and
r5 = a_mdl_dat**. But the mangled name says parameter 1 is `J3DModelData*`,
which would put `a_mdl_dat` in r4, not r5. Either I am misreading the register
convention here or the ctor signature in our headers differs from the donor's.
**Resolve that BEFORE writing the call** - `so`'s `_createHeap` has a
byte-EXACT 12-argument `new mDoExt_McaMorf(modelData, NULL, NULL, NULL,
J3DFrameCtrl::EMode_NULL, 1.0f, 0, -1, 1, NULL, 0x80000, 0x11020022)` and is
the oracle to diff against.

STATE: aj1 **14/131 exact, 7.1231%** (session start 12/131, 2.9112%).
createInit and btpResID EXACT; init_texPttrnAnm 98.43% (2 rows, three return-type
combinations already falsified - see Round 6).


## Round 8 - the "open question" was my own misreading; bodyCreateHeap written

**RESOLVED, and the oracle did it in one command.** Round 7 flagged that the
McaMorf construction passes `r4 = 1, r5 = a_mdl_dat` while the mangled name says
parameter 1 is `J3DModelData*`. I dumped `so`'s **byte-EXACT** `_createHeap`
call to the same constructor: **identical register pattern** - `li r4, 0x1`,
`mr r5, r31`(modelData), `li r6/r7/r8, 0`, `li r9, -1`, `lfs f1` = 1.0, `li r10, 0`,
same five stack stores. So there was never a discrepancy - my reading of the
register convention was simply wrong, and the source form is settled
empirically:

    new mDoExt_McaMorf(a_mdl_dat, NULL, NULL, NULL, J3DFrameCtrl::EMode_NULL,
                       1.0f, 0, -1, 1, NULL, 0x80000, 0x11020022)

**This is what the byte-identical siblings are FOR** - not style rules (which I
over-read in the `so` campaign and had to retract), but settling a specific
question about a specific construct. Cost: one command. I had been about to
leave the function unwritten over it.

**RESULT: `bodyCreateHeap` 99.66%, aj1 fuzzy 7.1231% -> 11.9351%.**

All 10 residual rows are **string-pool OFFSETS** and nothing else:

    T addi r4, r4, 0x91   |  M addi r4, r4, 0x54     ("backbone")
    T addi r6, r6, 0x9a   |  M addi r6, r6, 0x5d
    T addi r4, r4, 0xaf   |  M addi r4, r4, 0x72     ("handL")
    ...

The structure is exact; my offsets are uniformly ~0x3D low because the strings
that belong BETWEEN them do not exist yet - the eight cut names
(`AJ1_TLK`..`INVITE`, +0x2D..) which live in `privateCut`'s `a_cut_tbl` local
static, and `"Ojhous2"`. **These 10 rows will close by themselves when those
functions are written - do not chase them.** Same placement principle that
drove the whole `so` campaign, and it confirms the Round 4 work order.

Members added: `m_hed_jnt_num` / `m_bbone_jnt_num` / `m_hnd_L_jnt_num` /
`m_fot_L_jnt_num` at **0x6CC-0x6CF** (four consecutive s8).

SESSION TOTAL for aj1: **12/131 -> 14/131 exact, 2.9112% -> 11.9351% fuzzy**,
with `createInit` + `btpResID` byte-exact, `bodyCreateHeap` 99.66% and
`init_texPttrnAnm` 98.43% - the last two blocked only on strings/typing already
diagnosed.

NEXT: `privateCut` (its `a_cut_tbl` static owns the eight cut names at +0x2D)
- writing it should close most of `bodyCreateHeap`'s 10 rows as a side effect.


## Round 9 - the predicted cascade fired: two functions exact from one write

**aj1 14/131 -> 17/131 exact, 11.9351% -> 16.1645%.**

    privateCut       100.00%  EXACT (first try)
    wait_action2     100.00%  EXACT
    bodyCreateHeap   100.00%  EXACT  <- closed by wait_action2, not by editing it

**`privateCut` was exact on the FIRST attempt** because the shape transferred
directly from `ob1`: the action-result is a `switch` with a `default: ret = true;`
arm, not a ternary - the same correction that made `ob1`'s `privateCut` exact
earlier this session. Two 8-way jump tables (init + move), `a_cut_tbl` as a
function-local static owning the eight cut names, `cut_move_*` returning `bool`.

**THE CASCADE, exactly as Round 8 predicted.** After `privateCut`,
`bodyCreateHeap`'s residual offsets went from ~0x3D low to **exactly 8 low** -
and 8 bytes is precisely `"Ojhous2 "`. The Round 4 map named `wait_action2` as
its only user. Writing `wait_action2` closed all 9 rows **without touching
`bodyCreateHeap` at all**, taking it to EXACT. One more datum for the principle
that has driven this entire session: **fix the placement root cause and every
consumer settles at once; chasing the consumer's rows is the wrong order.**

`init_texPttrnAnm` also improved on the way: `bodyCreateHeap` tests its result
with `clrlwi. r0, r3, 24` - a BYTE - so it returns **`bool`**, not the `BOOL` I
had settled on in Round 6. That fixed one row here at no cost there (it stays at
2 rows either way). **The caller's test width is the reliable evidence for a
callee's return type** - better than reasoning about the callee alone, which is
where I went wrong twice.

MEMBERS this round: `field_0x764` (int), `field_0x768` (u8) - **note these sit
inside the 0x75C block, NOT the 0x6F0 one**; I mis-seated them first and got
`illegal constant expression` from a negative array size, which is the cheap
way that mistake announces itself. Also `field_0x7ba`, `field_0x7bb`,
`field_0x7c0`, and `m_hed/bbone/hnd_L/fot_L_jnt_num` at 0x6CC-0x6CF.

SESSION TOTAL for aj1: **12/131 -> 17/131 exact, 2.9112% -> 16.1645%.**
Five functions written, four of them byte-exact.

NEXT: `_create` (596 bytes) and `_execute` (552) are the largest remaining;
`call_1` (408) and `chk_areaIN` (380) after that.


## Round 10 - _create transcribed (155 instrs), NOT written; exact data still needed

Shape is settled - it is the standard actor `_create`, and `ob1`'s (2 rows off
exact) is the template:

    cPhs_State daNpc_Aj1_c::_create() {
        fopAcM_SetupActor(this, daNpc_Aj1_c);          // the inlined ctor chain,
                                                        // instrs 0..104
        cPhs_State state = dComIfG_resLoad(&mPhs, "Aj");   // "Aj" = pool +0xA
        switch (state) {
        case cPhs_COMPLEATE_e: {
            static u32 a_siz_tbl[] = { ... };              // a_siz_tbl$5553
            if (!decideType(fopAcM_GetParam(this) & 0xFF)) return cPhs_ERROR_e;
            if (!fopAcM_entrySolidHeap(this, CheckCreateHeap,
                                       a_siz_tbl[field_0x7be])) return cPhs_ERROR_e;
            cullMtx = mpMorf->getModel()->getBaseTRMtx();
            fopAcM_setCullSizeBox(this, /* six floats, see below */);
            if (!createInit()) return cPhs_ERROR_e;
        }
        }
        return state;
    }

The first 104 instructions are the `fopAcM_SetupActor` expansion (fopAc_ac_c
ctor, the `__vt__12fopNpc_npc_c` store at 0x6C0 - **which independently confirms
the Round 5 finding that the derived class starts at 0x6C4** - then dBgS_Acch,
dBgS_AcchCir, cCcD_Stts, dCcD_GStts, dCcD_GObjInf, dCcD_Cyl ctors). Nothing to
decide there; the macro emits it.

**STILL NEEDED BEFORE WRITING (do not guess these):**
1. `a_siz_tbl$5553` contents - the solid-heap sizes, indexed by `field_0x7be`.
   Read them out of the target `.data` the same way `a_res_id_tbl` was read.
2. The six `fopAcM_setCullSizeBox` floats. They load off the `"@4185"` anchor at
   **+0xA8, +0x40, (f3 = f1 again), +0xAC, +0xB0, +0xB4** - note the THIRD arg
   is `fmr f3, f1`, i.e. the same value as the first, exactly like `ob1`'s
   `(-60, -20, -60, 60, 170, 60)` shape where x and z share a literal.
3. Members: `mPhs` (for `dComIfG_resLoad`) and `field_0x7be` (s8, the size-table
   index). `decideType` must return **bool** - tested with `clrlwi. r0, r3, 24`.

STATE: aj1 **17/131 exact, 16.1645%** - unchanged this round, nothing written.


## Round 11 - _create fires the biggest cascade of the campaign

**aj1 17/131 -> 48/131 exact, 16.1645% -> 25.8746%. THIRTY-ONE functions went
exact from writing ONE function.**

`_create` is where `fopAcM_SetupActor(this, daNpc_Aj1_c)` instantiates the whole
constructor chain - `fopAc_ac_c`, `dBgS_Acch`, `dBgS_ObjAcch`, `dBgS_AcchCir`,
`cCcD_Stts`, `dCcD_GStts`, `dCcD_GObjInf`, `cCcD_ShapeAttr`, `cM3dGAab`,
`cM3dGCyl`, `cCcD_CylAttr`, `dCcD_Cyl` and their dtors/thunks. **Every one of
those is emitted into this TU and all of them snapped to byte-exact at once.**
Same class of cascade as `so`'s create chain (77 -> 133) earlier in the campaign,
and it is the third time this session that writing the *instantiating* function
paid out far more than the function itself is worth.

Data resolved before writing (none of it guessed):

    a_siz_tbl$5553  = { 0 }                       read from target .data
    cull box        = (-60, -20, -60, 80, 260, 100)
                      via the "@4185" ANCHOR at rodata 0x0 plus the addends
                      +0xA8, +0x40, (f3 = f1 again), +0xAC, +0xB0, +0xB4
    mPhs            @ 0x6C4  <- the FIRST derived member, which is why the
                                derived class starts there (Round 5's 0x6C4
                                finding, independently confirmed a third way)
    field_0x7be     @ 0x7BE  (s8, indexes a_siz_tbl)
    decideType      returns bool  (tested with clrlwi. r0, r3, 24)

**Then `mSmokeCb`.** `_create` sat at 93.52% until I noticed the ctor prologue
calls `__ct__18dPa_smokeEcallBackFUc` on `this + 0x76C` - a member I had never
declared. `dPa_smokeEcallBack` is 0x20 bytes (per `d_a_npc_ji1.h`'s 0x2E0/0x300
pair). Declaring it took `_create` to **97.48%**. The existence of a
`set_pa_smk` function (376 bytes) in the undone list corroborated it.

**HEADER MISTAKE WORTH RECORDING:** while adding `field_0x7be` I deleted the
`field_0x7b8` line and produced overlapping declarations of `field_0x7b9`. Both
faults announced themselves immediately (`undefined identifier`, then a visibly
duplicated offset in the layout). **When editing a padded layout by string
replacement, re-read the 0x7B0-0x7C0 window afterwards** - this is the fourth
anchor/arithmetic slip of the session and they are all the same shape.

SESSION TOTAL for aj1: **12/131 -> 48/131 exact, 2.9112% -> 25.8746%.**
83 functions remain; largest are `_execute` 552, `next_msgStatus` 468,
`call_1` 408, `chk_areaIN` 380, `set_pa_smk` 376, `lookBack` 368, `talk_1` 336.
