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


## Round 12 - _execute transcribed (147 instrs), NOT written; needs infrastructure first

Shape is fully legible:

    BOOL daNpc_Aj1_c::_execute() {
        if (!field_0x761) {                       // one-shot home latch
            mHomePos   = current.pos;             // 0x6FC/0x700/0x704 <- 0x1F8..
            field_0x708/0x70A/0x70C = current.angle;
            field_0x761 = 1;
        }
        m_jnt.setParam(l_HIO.<9 s16 fields at +0xC,0xE,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C>);
        if (field_0x75e && !field_0x1c0) return TRUE;
        checkOrder();
        if (!demo()) {
            int idx = -1;
            if (<gameInfo +0x529A> && <this +0xF8> != 1) idx = isEventEntry();
            if (idx >= 0) event_proc(idx);
            else          (this->*<ptmf @ 0x6F0>)(NULL);
            fopAcM_posMoveF(this, <this +0x538>);
            play_animation();
            mObjAcch.CrrPos(*dComIfG_Bgsp());
        }
        eventOrder();
        mHomeAngle = current.angle;               // 0x70E/0x710/0x712
        if (!field_0x75f) shape_angle = current.angle;
        tevStr.mRoomNo         = dComIfG_Bgsp()->GetRoomId(mObjAcch.m_gnd);
        tevStr.mEnvrIdxOverride = dComIfG_Bgsp()->GetPolyColor(mObjAcch.m_gnd);
        setMtx(false);
        flw_pa_pun(); del_pa_aka(); flw_pa_aka(); setSmoke();
        if (!field_0x76b) setCollision("@5536", "@5537");
        return TRUE;
    }

**WHY I STOPPED HERE RATHER THAN WRITING IT.** `_execute` is the first function
that needs real INFRASTRUCTURE rather than just members:

1. **`l_HIO`** - a file-scope `daNpc_Aj1_HIO_c l_HIO;` instance, and the HIO
   class currently has no fields. `setParam` reads nine s16s from it at
   +0xC..+0x1C, so the HIO layout has to be built first.
2. **An action pointer-to-member at 0x6F0**, called via `__ptmf_scall`. That
   needs the `ActionFunc` typedef pattern (`ob1` has
   `typedef int (daNpc_Ob1_c::*ActionFunc)(void*)`) plus `set_action`/`checkAction`
   support - and `wait_action1/2` already exist as its targets.
3. Members: `mHomePos` (cXyz @ 0x6FC), 0x708/0x70A/0x70C (csXyz), `field_0x75e`,
   `field_0x75f`, `field_0x761`, `field_0x76b`, and the two `setCollision`
   floats `@5536`/`@5537` still to read.

Writing it piecemeal would leave a half-built HIO class and a dangling ptmf,
which is exactly the state I have committed all session not to leave behind.
**The next session should build the HIO class + ActionFunc typedef FIRST**, then
`_execute` follows mechanically from the transcription above.

STATE: aj1 **48/131 exact, 25.8746%** - unchanged this round, nothing written.


## Round 13 - the HIO class layout, decoded (infrastructure for _execute)

`__ct__15daNpc_Aj1_HIO_cFv` is only 25 instructions and gives the whole class:

    class daNpc_Aj1_HIO_c : public mDoHIO_entry_c {
        /* 0x00 */ vtable (mDoHIO_entry_c then daNpc_Aj1_HIO_c - stored twice,
                           the standard two-store pattern)
        /* 0x04 */ s8   = -1
        /* 0x08 */ s32  = -1
        /* 0x0C */ <0x30-byte parameter block, memcpy'd from a_prm_tbl$4141>
    };

The ctor is: store vtable, `memcpy(this + 0xC, a_prm_tbl, 0x30)`, then
`field_0x4 = -1`, `field_0x8 = -1`. **A `memcpy` of exactly the block size is
how MWCC implements a struct/array ASSIGNMENT**, so the donor almost certainly
writes something like `mPrm = a_prm_tbl;` rather than calling memcpy by hand.

**`a_prm_tbl$4141`, all 12 words (0x30 bytes), read from the target .data:**

    0x20001F40  0x01F4CD38  0x00001C70  0xFA24FF6A
    0x04000400  0x43160000  0x00000000  0x44BB8000
    0x52080000  0x44098000  0x61A8001E  0x00140000

**IT IS MIXED-TYPE, and that matters.** As s16 pairs the first five words give
`0x2000, 0x1F40, 0x01F4, 0xCD38, 0x0000, 0x1C70, 0xFA24, 0xFF6A, 0x0400` -
exactly the nine values `_execute` feeds to `m_jnt.setParam` from +0xC..+0x1C.
But words 6, 8 and 10 are clean floats (`0x43160000` = 150.0,
`0x44BB8000` = 1500.0, `0x44098000` = 550.0). So the block is a STRUCT of nine
s16 angle/param fields followed by float distances - not a flat array.

**WHY I STOPPED HERE.** Naming and typing those struct fields is DESIGN, not
transcription - I would be inventing a layout, and a wrong guess propagates into
every consumer of `l_HIO`. Two things must be settled first:

1. The struct's field split after the nine s16s (the remaining 0x1E bytes mix
   floats and what look like more s16 pairs: `0x52080000`, `0x61A8001E`,
   `0x00140000`).
2. **`m_jnt.setParam`'s ARGUMENT ORDER, which is scrambled relative to memory
   order.** The call loads: r4=+0x14, r5=+0x16, r6=+0x18, r7=+0x1A, r8=+0xC,
   r9=+0xE, r10=+0x10, stack+0x8=+0x12, stack+0x1C=+0x1C. Do NOT assume the
   source lists them in offset order - check `dNpc_JntCtrl_c::setParam`'s real
   signature and let that name the fields.

Both are answerable from `d_npc.h` plus a matching sibling; neither is a guess I
should make at the end of a long session.

STATE: aj1 **48/131 exact, 25.8746%** - unchanged, nothing written this round.


## Round 14 - HIO layout LANDED (48 -> 50 exact); ctor body still wrong

**`setParam`'s signature named the fields, so this was transcription after all -
I was wrong in Round 13 to call it design.** `dNpc_JntCtrl_c::setParam` is
declared in `d_npc.h` as
`(max_backbone_x, max_backbone_y, min_backbone_x, min_backbone_y, max_head_x,
max_head_y, min_head_x, min_head_y, max_turn_step)`. Mapping that onto the
call's register loads (r4=+0x14, r5=+0x16, r6=+0x18, r7=+0x1A, r8=+0xC, r9=+0xE,
r10=+0x10, stack=+0x12, +0x1C) resolves the block exactly:

    0x0C max_head_x      0x2000      0x14 max_backbone_x  0x0000
    0x0E max_head_y      0x1F40      0x16 max_backbone_y  0x1C70
    0x10 min_head_x      0x01F4      0x18 min_backbone_x  0xFA24
    0x12 min_head_y      0xCD38      0x1A min_backbone_y  0xFF6A
                                     0x1C max_turn_step   0x0400

i.e. head params first in memory, backbone second, turn step last - the
scrambled ARGUMENT order was just MWCC's, not the struct's. **The lesson: when
an argument order looks scrambled, read the callee's declaration before
concluding anything about the caller's data layout.**

**RESULT: `__dt__15daNpc_Aj1_HIO_cFv` and `__dt__14mDoHIO_entry_cFv` both went
EXACT - 48/131 -> 50/131, 25.8746% -> 26.8213%.** The class layout is right.

**THE CTOR BODY IS STILL WRONG (0.00%, 59 rows vs the target's 25 instrs).**
The target does a single `memcpy(this + 0xC, a_prm_tbl, 0x30)`; my
`mPrm = a_prm_tbl;` on a mixed-type struct made MWCC copy FIELD BY FIELD
instead. So the member at 0xC is probably not a mixed struct at all - more
likely a plain array (which cannot be assigned, forcing a real `memcpy` call),
with the typed access happening at the use sites. **Try `s16 mPrm[0x18]` (or a
u8[0x30]) plus an explicit memcpy next, and check whether `_execute`'s
`setParam` reads still come out clean.** The layout offsets above are confirmed
correct either way - only the member's TYPE is open.

STATE: aj1 **50/131 exact, 26.8213%**. Session: 12/131 -> 50/131, 2.9112% ->
26.8213%.


## Round 15 - HIO ctor EXACT; the memcpy was the type, not the code

**`mPrm` is `s16[0x18]`, not a struct.** Round 14 guessed a mixed-type struct
from the data (nine s16s then floats); assigning it made MWCC copy field by
field. Declaring a plain `s16[0x18]` and calling `memcpy` explicitly matched
immediately - **arrays cannot be assigned in C++, which is precisely what forces
the single `memcpy` call the donor emits.** The float-looking words
(`0x43160000` = 150.0 etc.) are just s16 pairs that happen to read as floats;
the type is uniform and the callers reinterpret.

**`__ct__15daNpc_Aj1_HIO_cFv`: 59 rows -> 0. aj1 50/131 -> 51/131, 27.4130%.**

GENERALISABLE: **when the target emits a `memcpy` for a whole-member copy and
your version emits field stores, the member is an ARRAY.** Struct assignment
inlines; array copy calls. That is a type signal readable straight off the diff,
in the same family as `clrlwi` width telling you `bool` vs `BOOL`.

SESSION TOTAL for aj1: **12/131 -> 51/131 exact, 2.9112% -> 27.4130%.**
The `l_HIO` infrastructure is now in place, so `_execute` (transcribed in
Round 12) needs only the `ActionFunc` ptmf at 0x6F0 before it can be written.


## Round 16 - _execute EXACT; the ob1 template supplied everything

**aj1 51/131 -> 53/131 exact, 27.4130% -> 31.0604%.**

Built the infrastructure Round 12 said was needed, and it was all determined
rather than designed:

- **`ActionFunc` ptmf at 0x6F0.** An MWCC pointer-to-member is 12 bytes, and
  0x6F0 + 0xC = **0x6FC = `mHomePos`** - the layout closes exactly, which is how
  I knew the offset was right before compiling anything.
- **`l_HIO`** - a file-scope `daNpc_Aj1_HIO_c l_HIO;` (the class landed in
  Round 15).
- Members `field_0x75e/0x75f/0x761/0x76b`, `mHomePos` @0x6FC, `field_0x708` csXyz.

**`ob1`'s `_execute` supplied the three things I would otherwise have guessed**,
and its `setParam` call matches my asm's register order verbatim:

    m_jnt.setParam(l_HIO+0x14, +0x16, +0x18, +0x1A, +0xC, +0xE, +0x10, +0x12, +0x1C)
    if (dComIfGp_event_runCheck() && !eventInfo.checkCommandTalk()) ...
    (this->*<ptmf>)(NULL);

Two measured corrections: `fopAcM_posMoveF` takes a `const cXyz*`, so the
`this+0x538` argument is **`mStts.GetCCMoveP()`**, not `&mStts`; and
`setCollision` takes **(60.0f, 140.0f)** - the target loads two DIFFERENT
literals (`@5536`/`@5537`) where I had passed one value twice, which showed up
as a lone `fmr f2, f1`.

**AND THE BYTE-WIDTH SIGNAL PAID AGAIN.** `_create` tests `createInit()` with
`clrlwi. r0, r3, 24`, so `createInit` returns **`bool`** - I had it as `BOOL`
since Round 5. Changing it kept `createInit` EXACT and took `_create` from
10 rows to **7**. That is the third time this session the CALLER's test width
named the callee's return type. **Check it whenever a boolean test row survives.**

`_create`'s remaining 7 rows are float-pool positions (mine at 0x18-0x24 vs the
target's 0x40-0xB4) - they settle as more functions contribute literals, exactly
like `bodyCreateHeap`'s did.

**SESSION TOTAL for aj1: 12/131 -> 53/131 exact, 2.9112% -> 31.0604%.**
78 functions remain; largest are `_create` (7 rows), `next_msgStatus` 468,
`call_1` 408, `chk_areaIN` 380, `set_pa_smk` 376, `lookBack` 368.


## Round 17 - next_msgStatus EXACT, and Round 15's array conclusion CORRECTED

**aj1 53/131 -> 54/131 exact, 31.0604% -> 33.8059%.**

**`next_msgStatus` (134 instrs) is EXACT.** The 23-entry jump table `@4601`
gave the case values (message IDs 0x9C6..0x9DC) directly, and **the case-body
ADDRESSES gave the source case order** - `0x9c6, 0x9c7, 0x9c9, {0x9c8,0x9ca},
0x9db, {0x9cb,0x9dc}, 0x9cd, 0x9cf, 0x9d0, default` - which is NOT numeric order
and could not have been guessed. Read a jump table's targets, sort the bodies by
address, and the switch writes itself.

The last 9 rows were a **nested-branch inversion**. I had written
`if (isEventBit(0x504)) -> 0x9cf; else if (0x2a80) -> 0x9dc; else -> 0x9cb`,
which emits the `0x9cf` body first. The donor branches AWAY on 0x504 being set,
so the false path is the then-branch and `0x9dc`/`0x9cb` emit before `0x9cf`:

    if (!dComIfGs_isEventBit(0x504)) {
        if (dComIfGs_isEventBit(0x2a80)) *i_msgNo = 0x9dc; else *i_msgNo = 0x9cb;
    } else { *i_msgNo = 0x9cf; }

**Third instance of the same pattern today** (with `ob1`'s `control_anmAtr` case
order and `so`'s `modeEventFirstWait` comparison direction): **when the bodies
are right but ordered wrong, the fix is WHICH SIDE OF THE CONDITION IS THE
THEN-BRANCH.**

### CORRECTION to Round 15 - `mPrm` is a STRUCT, not an array

Round 15 concluded the HIO parameter block is `s16[0x18]` because a plain array
forced the donor's single `memcpy`. **That was a hack that happened to match.**
Transcribing `call_1` shows it reading **`lfs f1, 0x28(l_HIO)`** and
**`lfs f1, 0x30(l_HIO)`** - FLOATS out of that block - which an s16 array cannot
express. The mixed struct I guessed in Round 14 was right all along, and its
offsets check out exactly: +0x1C f32, +0x20 s16, +0x24 f32, +0x28 s16
(class-relative 0x28/0x2C/0x30/0x34).

**The resolution keeps both properties: declare the STRUCT and call `memcpy`
EXPLICITLY.** Struct *assignment* inlines field stores; an explicit `memcpy` on
a struct emits the call. `__ct__15daNpc_Aj1_HIO_cFv` stays **EXACT** and
`_execute` stays **EXACT** with named fields
(`l_HIO.mPrm.max_backbone_x`, ...).

**So the Round 15 rule was too strong.** Corrected form: *memcpy-vs-field-stores
tells you the copy was written as a `memcpy` call, NOT that the member is an
array.* Check how other functions READ the member before choosing its type - the
read sites are the authority on layout.

STATE: aj1 **54/131 exact, 33.8059%**. Session: 12/131 -> 54/131,
2.9112% -> 33.8059%.


## Round 18 - call_1 EXACT, chk_areaIN 99.95%; two NEW idioms for the bank

**aj1 54/131 -> 55/131 exact, 33.8059% -> 38.4201%.**

**`call_1` EXACT.** Three findings, each read off the diff:

1. **`field_0x7b9` is `s8`, not `u8`.** The donor uses `extsb.` / `cmpwi`
   (signed) where mine emitted `cmplwi`. Two rows for one type character.
2. **NEW IDIOM: `x == false` and `!x` are NOT interchangeable under MWCC.**
   The target MATERIALISES the negation - `clrlwi; cntlzw; extrwi.` then
   branches - which is what `x == false` produces. Plain `!x` compiles to a
   direct inverted branch and cost the last 4 rows. **When you see
   `cntlzw` around a bool test, the source wrote an explicit `== false` (or
   `== 0`), not `!`.**
3. `m_jnt + 0xA` is `mbTrn`, so those `stb 1, 0x29a` stores are `m_jnt.setTrn()`.

**`chk_areaIN` 99.95%** (5 rows, all float/double POOL POSITIONS - mine at
0x10/0x18/0x20 vs the target's 0x70/0x78/0x80 - they settle as more functions
contribute literals, exactly like `_create`'s 7). It is a distance/height/angle
gate built on the `absXZ` inline:

    dist  = (dComIfGp_getPlayer(0)->current.pos - i_pos).absXZ();
    dy    = dComIfGp_getPlayer(0)->current.pos.y - i_pos.y;
    angle = (s16)cLib_targetAngleY(&current.pos,
                                   &dComIfGp_getPlayer(0)->current.pos) - field_0x708.y;
    return dist < i_dist && std::fabsf(dy) < 500.0f && abs(angle) < i_angle;

Two corrections got it from 16 rows to 5: **the donor RE-CALLS
`dComIfGp_getPlayer(0)` at each use rather than caching it in a local** (I had
hoisted it, which showed up as `lwz r4, 0x5b44(r31)` vs my cached `r30`), and
the `cLib_targetAngleY` result is **sign-extended** (`extsh`) before the
subtraction. Also note `fabsf` must be written `std::fabsf` in this codebase.

STATE: aj1 **55/131 exact, 38.4201%**. Session: 12/131 -> 55/131,
2.9112% -> 38.4201% (a 13x fuzzy gain).


## Round 19 - lookBack 93.09%, talk_1 EXACT; aj1 57/131, 43.1217%

**`talk_1` EXACT.** Its inner jump table has a **deliberate fall-through**:
case `0x9DA` calls `onEventBit(0x3704)` and then falls into case `0x9C8`'s
`onEventBit(0x510)` with no `break`. Visible only because `0x9DA`'s body sits
immediately above `0x9C8`'s and ends without a branch - **read the body
addresses, not just the table targets.** Case order (from body addresses):
`0x9DA, 0x9C8, 0x9DD, {0x9D1,0x9D2,0x9D3}, 0x9D7`.

**`lookBack` 93.09%** - fully decoded (`dNpc_playerEyePos(-20.0f)`, 4-case
dispatch on `field_0x7bd`, `m_jnt.lookAtTarget(...)`), but its 42 remaining rows
are **register colouring** (`this` gets r28 in the donor, r31 in mine). I tried
the `p1` oracle's declare-locals-up-front arrangement: **42 -> 45 rows, WORSE.**
Reverted. That is now consistent with the `so` result - declaration placement is
not the colouring lever on any TU I have tested.

### THE TYPE-SIGNAL BANK (everything that HAS worked on this TU)

Every win here came from reading a TYPE or a SHAPE off the diff. Collected so
the next instance starts with the list rather than rediscovering it:

| diff symptom | what it means |
|---|---|
| `clrlwi. rX, rY, 24` testing a result | callee returns **`bool`** (byte), not `BOOL` |
| extra `clrlwi` on YOUR return only | your local is `bool` where the donor's is **`int`** |
| `clrlwi rX, rY, 16` at a CALL SITE | callee returns **`int`**, truncated to u16 by the param |
| `extsb.` / `cmpwi` vs your `cmplwi` | the field is **`s8`**, not `u8` |
| unexpected `extsb` before a call arg | field signedness ≠ parameter signedness |
| `cntlzw` around a bool test | source wrote **`== false`**, not `!x` |
| `memcpy` where you emit field stores | the copy was written as an explicit **`memcpy` call** (NOT necessarily an array - check the READ sites for the member's real type) |
| bodies right, order wrong | flip **which side of the condition is the then-branch** |
| jump-table case bodies out of order | source case order = **body ADDRESS order**, not numeric |
| `lwz` of a global repeated per use | the donor **re-calls the accessor**; do not hoist it into a local |

**AND THE ONE THAT NEVER WORKS:** register colouring. ~15 measured attempts
across `so`, `ob1`, `p2` and now `aj1`, zero successes. Declaration order,
aliasing, local hoisting, type changes - all tried, all null. **Treat a
colouring-only diff as a park candidate, not a puzzle.**

STATE: aj1 **57/131 exact, 43.1217%**. Session: 12/131 -> 57/131,
2.9112% -> 43.1217% (14.8x fuzzy).


## Round 20 - set_pa_smk transcribed (95 instrs), NOT written; needs the JPA emitter map

Shape is clear; what it needs is an API MAPPING, not a decode:

    PSMTXCopy(mpMorf->getModel()->getAnmMtx(m_fot_L_jnt_num), mDoMtx_stack_c::now);
    field_0x79c = now[0][3];  field_0x7a0 = now[1][3];  field_0x7a4 = now[2][3];
    mSmokeCb.<vtable +0x20>();                       // virtual call on the smoke cb
    field_0x78c = dComIfGp_particle_set(2, 0x2027, &field_0x79c, &shape_angle,
                                        NULL, current.angle.<0x20a>, 0, 0xC8, &mSmokeCb, ...);
    if (field_0x78c != NULL) {
        <emitter>->mFieldA = @4185+0x58 (x2), +0x3c;   // 0x1F0/0x1F4/0x1F8
        <emitter>->mFieldB = @4185+0x5c (x3);          // 0x1D8/0x1DC/0x1E0
        <emitter>-><0x64> = 0x28;
        <emitter>-><0x38> = @4185+0x60;
        <emitter>-><0x60> = 1;
        <emitter>-><0x74> = @4185+0x64;
        <emitter>-><0xC/0x10/0x14> = the +0x3c / +0x5c values again;
        field_0x782..0x785 = the 4 bytes read from @4185+0x54;
    }

**WHY NOT WRITTEN:** the eleven emitter field offsets (0x1F0, 0x1F4, 0x1F8,
0x1D8-0x1E0, 0x64, 0x38, 0x60, 0x74, 0x0C, 0x10, 0x14) each need mapping onto a
named `JPABaseEmitter` setter - `JPAEmitter.h` has `setGlobalAlpha`,
`setGlobalParticleScale`, `setGlobalDynamicsScale` and friends, so the names
exist, but matching offset->setter is a careful read of that header rather than
anything readable off this diff. **That is a lookup task, not a pattern task**,
and starting it at the end of a long session risks a half-written 95-instruction
function - which is the one thing I have held to all session.

The `@4185` constants it needs (rodata offsets 0x3C, 0x54, 0x58, 0x5C, 0x60,
0x64) are read the same way as `_create`'s cull box: `@4185` is an ANCHOR at
rodata 0x0, so the value is simply the rodata word at that offset.

STATE: aj1 **57/131 exact, 43.1217%** - unchanged, nothing written this round.
SESSION: **12/131 -> 57/131 exact, 2.9112% -> 43.1217%** (14.8x fuzzy), with
`createInit`, `btpResID`, `bodyCreateHeap`, `privateCut`, `wait_action2`,
`_create`(7 rows), `_execute`, `next_msgStatus`, `call_1`, `talk_1`, the HIO
class + ctor all landed, and the cascade from `_create` alone taking 31
constructor-chain functions to exact.


## Rounds 21-23 - aj1 CROSSES HALF: 57/131 -> 60/131 exact, 43.12% -> 51.50%

Landed: `set_pa_smk` 97.8%, `_nodeCB_Head` **EXACT (first try)**,
`_nodeCB_BackBone` 4 rows, `demo` **EXACT**, `_draw` **EXACT**.

**`_nodeCB_Head` exact on the FIRST attempt** is the clearest sign the
accumulated patterns are working - fn-local `static cXyz`, explicit
`mDoMtx_YrotM(now, ...)` rather than the stack wrapper, `getAnmMtx(jntNo)`
recomputed rather than cached. No iteration needed.

### FIVE MORE ENTRIES FOR THE TYPE-SIGNAL BANK

| diff symptom | what it means |
|---|---|
| frame size too SMALL (0x20 vs 0x30) | a local is missing or wrongly `static` - `set_pa_smk`'s `GXColor` is a STACK local; making it `static` cost 15 rows |
| your return normalises (`subic`/`subfe`/`clrlwi`) but the donor returns raw | the FIELD is `bool`, not just the return type (`demo`, 9 -> 3 rows) |
| donor branches to one common exit | single exit, NOT an early `return` (`demo`'s last 3 rows) |
| an offset lands INSIDE an existing member | look for an ACCESSOR, do not add a field - 0x6E0 is `mBtpAnm`'s `mpAnm`, i.e. `mBtpAnm.getBtpAnm()`. I started adding a member and had to revert |
| `lbz` where you declared `f32` | the ACCESS WIDTH at the use site beats inference from the data - `l_HIO` prm+0x18 reads as a byte though the bytes look like a float |

**ALSO DONOR-FAITHFUL DEAD CODE:** `_draw` contains an
`if (l_HIO.mPrm.field_0x18) { cXyz sp8 = current.pos; sp8.y = eyePos.y; }` whose
stores nothing ever reads. **The donor emits it, so it must be written out** -
same family as the dead `sph_offset`/`cyl_offset` statics restored on `so`.

**API notes worth keeping:** `J3DModelData::getMaterialTable()` returns a
REFERENCE (`.removeTexNoAnimator(...)`, not `->`); `dSnap_RegistFig` needs
`d/d_snap.h`; `JPABaseEmitter::setGlobalParticleScale(x, y)` sets z to 1.0
internally, so the 2-arg overload covers all three stores.

STATE: aj1 **60/131 exact, 51.4992%**. Session: **12/131 -> 60/131,
2.9112% -> 51.4992%** (17.7x fuzzy). 71 functions remain; five sit at 3-9 rows
from pool positions or argument order alone (`_create` 98.9%, `chk_areaIN`
99.9%, `set_pa_smk` 97.8%, `init_texPttrnAnm` 98.4%, `_nodeCB_BackBone`).


## Round 24 - setAnm_anm 98.26%; naming the INVERTED-BRANCH bucket

`setAnm_anm` needed the `anm_prm_c` struct defined (`s8` @0x0, `f32` @0x4,
`f32` @0x8, `int` @0xC), the emitter field identified as **`mFlags` @0x20C**
(not `mDataFlag` @0x88 - I guessed from the name and the offset corrected me),
and the single-case-switch shape for the inner `field_0x7b9 == 2` dispatch.
That took it 7 -> 3 rows.

### THE INVERTED-BRANCH BUCKET - now named, still unsolved

Three functions across two TUs end on the SAME residual shape:

    target:  cmpw ... ; bne BODY ; b END ; BODY: <body> ; END:
    mine:    cmpw ... ; beq END  ;         <body>       ; END:

i.e. **the donor branches TO the body and then over it; I branch past it.**
One extra instruction, and I cannot reproduce it. Affected: `aj1`'s
`setAnm_anm` (3 rows), `ob1`'s `chg_anmAtr` (2 rows), and a variant in
`aj1`'s `_nodeCB_BackBone` (4 rows, argument-order flavour).

**FALSIFIED against it, all compiled and measured - do not re-run:**

- `switch (a != b) { case 1: ... }` - materialises the bool (`cmpwi r0, 0x1`), 2 -> 6 rows
- `switch (a == b) { case 0: ... }` - materialises too (`extrwi.`), 2 -> 4 rows
- empty then-block with the work in `else` - **MWCC FOLDS THE EMPTY BRANCH**, no change (tested twice: `ob1` `chg_anmAtr`, and here as a clean full-function rewrite so the earlier syntax error does not count as the test)
- `x == false` - that idiom fixes a DIFFERENT shape (the `cntlzw` materialised negation in `call_1`) and does nothing here

**What is NOT yet tried:** a `goto`, and whether the donor's body is inside a
scope that forces the label (a `do { } while (0)`, or the body being a separate
inlined function). Worth one attempt each before this bucket is parked with the
colouring rows.

STATE: aj1 **60/131 exact, 53.0104%**. Six functions sit at 3-9 rows, every one
from a KNOWN cause: pool position, argument order, or this branch shape.


## Rounds 25-27 - aj1 64/131, 59.82%; the EMPTY-CASE signal

Landed: `FARwai` **EXACT**, `wait_1` **EXACT**, `setStt` **EXACT**,
`setMtx` 8 rows, `shadowDraw` 4 rows (pool positions only).

### ⭐ NEW SIGNAL: an explicit EMPTY `case`

`setStt` sat at 9 rows because the donor's switch has **`case 0: break;` with no
body**. An empty case produces NO CODE, so it is invisible in the body listing -
**it shows only in the DISPATCH TREE**, as a comparison whose branch target is
the function exit:

    cmpwi r0, 0x2 ; beq case2
    bge  L_2764
    cmpwi r0, 0x0 ; beq END      <- case 0 exists and is EMPTY
    bge  L_2774                  <- case 1

I had folded 0 into the default and lost 9 rows to it. Three added lines took it
to EXACT. **Read the dispatch comparisons, not just the case bodies - a value
tested with a branch straight to the exit is an explicit empty case.**

### More entries for the bank

- **`field_0x758`, `field_0x7ba`, `field_0x7bb`, `field_0x7b9` are all `s8`.**
  The `extsb`-vs-`cmplwi` signal has now paid FIVE times on this TU; when a
  byte field is compared, assume `s8` until the diff says otherwise.
  Fixing `field_0x758` closed TWO functions at once.
- **`current.angle.y` (0x206) vs `shape_angle.y` (0x20E)** - both `csXyz`, six
  bytes apart, and easy to confuse. `wait_1` turns `current.angle.y`.
- **A stale float register at a call site is a real argument.** `shadowDraw`
  leaves `f3` holding `current.pos.y` from an earlier load and never reloads it -
  because `current.pos.y` IS the third float argument (confirmed against `ob1`'s
  `dComIfGd_setShadow` call, which passes it explicitly).

### Falsified this round

- `setMtx`: holding `field_0x6d0` in a named local to keep it live - **8 -> 18
  rows, frame grew 0x10 -> 0x20.** The donor re-reads the member.

STATE: aj1 **64/131 exact, 59.8201%**. Session: **12/131 -> 64/131,
2.9112% -> 59.8201%** (20.5x fuzzy). 67 remain, all under 240 bytes.


## Round 28 - ⭐ THE INVERTED-BRANCH BUCKET IS CRACKED

Round 24 named this bucket and listed four falsified attempts. **`event_proc`
solved it: a single-case `switch` ON AN INTEGER SUBJECT produces the
`beq body ; b end` shape that a plain `if` cannot.**

    // gives  beq END ; <body>            (what I kept writing)
    if (field_0x746 == 0) { ... }

    // gives  beq BODY ; b END ; BODY:    (what the donor emits)
    switch (field_0x746) {
    case 0:
        ...
        break;
    }

**WHY MY ROUND-24 ATTEMPTS FAILED, and it is instructive:** I tried
`switch (a != b)` and `switch (a == b)` - switching on a COMPARISON. That forces
MWCC to materialise a boolean first (`cmpwi r0, 0x1`, `extrwi.`), which is why
both made things worse. **The switch subject has to be the integer VALUE
itself.** I had the right construct and the wrong operand.

**This retroactively explains the whole bucket:**
- `privateCut` matched first try because its dispatch was ALREADY a switch on an int.
- `event_proc` (this round) and `setStt`'s empty case are the same family.
- **`ob1`'s `chg_anmAtr` and `aj1`'s `setAnm_anm` still cannot be fixed** - their
  conditions compare TWO VARIABLES (`i_no != field_0x800`,
  `field_0x7b9 != i_prm->field_0x0`), which no switch can express. Those stay
  open, but now for a KNOWN reason rather than an unknown one.

Also this round, from `getMsg_AJ1_2`: **ternary polarity sets the BASE
CONSTANT.** `x ? 0x9d6 : 0x9d5` computes 0/+1 and adds 0x9d5;
`x == 0 ? 0x9d5 : 0x9d6` computes 0/-1 and adds 0x9d6 - which is the donor's
`subic`/`subfe` idiom. If the added constant is off by one, flip the ternary.

STATE: aj1 **68/131 exact, 64.6959%**. Session: **12/131 -> 68/131,
2.9112% -> 64.6959%** (22.2x fuzzy).


## Rounds 29-31 - aj1 68/131 -> 73/131, 64.70% -> 72.43%

EXACT this stretch: `itemCreateHeap` (first try), `checkOrder`, `init_AJ1_1`,
`anmAtr`, `ctrl_WAITanm`. Near: `setSmoke` 4 rows, `flw_pa_pun` 3,
`shadowDraw` 4 (all pool positions or one argument form).

**The single-case-switch lesson is now being APPLIED rather than discovered** -
`checkOrder`'s `switch (field_0x746) { case 0: ... }` was written that way from
the start. That is the first time this campaign a pattern was used
prophylactically instead of found through a diff.

### API mappings worth keeping (all from sibling TUs or headers)

    eventInfo +0x04 == 1/2   ->  eventInfo.checkCommandTalk() /
                                 .checkCommandDemoAccrpt()   (dEvtCmd_INTALK_e=1,
                                                              INDEMO_e=2)
    actor_status @0x1C4      ->  fopAcM_OnStatus / fopAcM_OffStatus
                                 (`rlwinm rX,rY,0,18,16` is a single-bit CLEAR
                                  of 0x4000; `ori 0x4000` is the set)
    mpMorf + 0x58 checkPass  ->  mpMorf->checkFrame(f)   NOT getFrameCtrl()
                                 (that accessor belongs to another class in the
                                  same header)
    gameInfo +0x5BDB/+0x5BDC ->  dComIfGp_getMesgAnimeAttrInfo() /
                                 getMesgAnimeTagInfo() / clearMesgAnimeTagInfo()
    JPABaseEmitter +0x1A8/+0x1E4 -> mGlobalRotation / mGlobalTranslation

### Two more bank entries

- **Comparison OPERAND ORDER is source-visible.** I copied `ob1`'s
  `field_0x801 != tag` into `aj1`'s `anmAtr`; the donor writes
  **`tag != field_0x7b7`**, which flips the `cmplw` operands. **Copying an idiom
  from a sibling gets the CONSTRUCT right but not the ORDER** - that still has
  to come from the diff.
- **The `extsb`/`s8` signal has now paid SIX times on this TU alone**
  (`field_0x758`, `0x7b9`, `0x7ba`, `0x7bb`, `0x7bc`, `0x7c1`). When a byte
  field is compared, assume `s8`.

STATE: aj1 **73/131 exact, 72.4331%**. Session: **12/131 -> 73/131,
2.9112% -> 72.4331%** (24.9x fuzzy). 58 remain, all under 210 bytes.


## Rounds 32-35 - aj1 73/131 -> 79/131, 72.43% -> 79.34%

EXACT: `cut_move_LOK`, `CreateHeap`, `itemCreateHeap`, `bodyCreateHeap`,
`_delete`, `set_action`, `play_animation`, `chk_talk`.
Near: `cut_move_VIVRATE` 3 rows (pool positions).

### ⭐ The caller-test-width rule fired TWICE INSIDE ONE FUNCTION

`CreateHeap` tests both `bodyCreateHeap()` and `itemCreateHeap()` with **`cmpwi`
(a word)** where I emitted `clrlwi` (a byte). Widening BOTH callees to `BOOL`
kept each of them exact AND took `CreateHeap` 2 rows -> 0. **That rule has now
paid on five separate functions in this TU and is the most reliable signal in
the bank.**

Its sibling, the **local-width** variant: `play_animation` had an unexpected
`extsb` on a LOCAL. The local flows into `mDoExt_McaMorf::play(Vec*, u32, s8)`,
so `sndId` is **`u32`**, not the `s8` I assumed from the value's meaning.
**An unexpected `extsb` on a local means its declared type is narrower than what
it flows into.**

### ⭐ Recognising an INLINED PREDICATE

`chk_talk`'s asm reads as `x == 1 || x == 2 || x == 3` on `gameInfo + 0x52B8`.
That is NOT three hand-written comparisons - it is
**`dComIfGp_event_chkTalkXY()` inlined** (`mTalkButton` vs X/Y/Z). Found by
tracing the offset: `gameInfo + 0x52B8` -> `play + 0x4018` -> inside
`dEvt_control_c` (0x3F38..0x402C) at **+0xE0** -> `mTalkButton`. Likewise
`+0x52B9` is `getPreItemNo()`.
**A run of equality tests against small consecutive constants is usually ONE
inlined predicate. Trace the offset through the containing struct before
transcribing the comparisons literally.**

### `set_action` explains the empty `case 9:` arms

`set_action` is the state-machine core: `__ptmf_cmpr` / `__ptmf_test` /
`__ptmf_scall` around the `ActionFunc` at 0x6F0, setting **`field_0x7c0 = 9`
before calling the OUTGOING action** (a "leaving" notification) and `= 0` before
calling the incoming one. **That is why `wait_action1`/`wait_action2` have empty
`case 9:` arms** - I wrote those from the dispatch shape without knowing why they
existed, and this closes the loop.

### API mappings added

    mObjAcch flags     -> mObjAcch.ChkGroundHit()
    dBgS               -> dComIfG_Bgsp()->GetMtrlSndId(mObjAcch.m_gnd)
    vibration          -> dComIfGp_getVibration().StartShock(n, -0x11, cXyz(...))
                          (idiom lifted from d_a_am - useful references are NOT
                           limited to the two byte-identical NPC oracles; ANY
                           matching TU calling the same API works)
    mpMorf frame       -> mpMorf->getFrame()
    acch setup         -> fopAcM_GetPosition_p / GetOldPosition_p / GetSpeed_p

STATE: aj1 **79/131 exact, 79.3437%**. Session: **12/131 -> 79/131,
2.9112% -> 79.3437%** (27.3x fuzzy). 52 remain.


## Rounds 36-40 - aj1 79/131 -> 89/131, 79.34% -> 88.12%

EXACT: `eventOrder`, `getMsg_AJ1_1`, `play_texPttrnAnm`, `init_AJ1_2`,
`decideType`, `getMsg`, `setAnm_NUM`, `setAnm`, `setAnm_ATR`, `chkAttention`.
Near: `set_pa_pun` 7 rows / `set_pa_aka` 9 (argument evaluation order),
`cut_move_VIVRATE` 3, `set_pa_smk` 9 (pool churn).

### ⭐ A pointer-to-member constant is identified by its THIRD word

`@4240`, `@4269` and `@4284` all read `0x00000000 / 0xFFFFFFFF` in their first
two words (the delta and vtable-offset fields). **The function is word three:**

    @4240 -> wait_action1
    @4269 -> wait_action2
    @4284 -> wait_action2      <- init_AJ1_2 binds wait_action2, NOT wait_action1

Without reading word three I would have had to guess between two plausible
actions. `init_AJ1_2` also needed `eventInfo.mpCheckCB` (+0x10), whose
`CallbackFunc` returns **`s16`** - the compile error was itself the signal.

### ⭐ A struct derived from ONE consumer is provisional

I defined `anm_prm_c` from `setAnm_anm`, which touches only +0x0, +0x4, +0x8,
+0xC. `setAnm_NUM` then reads **+0x1** (`lbz r4, 0x1(r4)`) - a second `s8`, the
btp index. **Re-open a struct definition when a new consumer appears; the first
one only shows you the fields it happens to use.**

Three `a_anm_prm_tbl` variants recovered, all 0x10-byte entries:
`setAnm_NUM` 9 entries, `setAnm` 5 (indexed by `field_0x7bb`, with `{-1,-1,...}`
sentinel rows), `setAnm_ATR` 9 (indexed by `field_0x7b6`).

### Operand order, third and fourth instances

`anmAtr` needed **`tag != field_0x7b7`** (not the `ob1` order I copied) and
`chkAttention` needed **`this == target`** (not `target == this`) - the latter
showing as a `subf` direction rather than a `cmplw`. **Copying an idiom from a
sibling gets the construct right; the operand order still comes from the diff.**

### Also settled

- `getMsg_AJ1_0` widened to `u32` (compile error was the signal, same family as
  the `s16` callback).
- `chk_talk`'s `x==1||x==2||x==3` run is `dComIfGp_event_chkTalkXY()` INLINED -
  traced via `gameInfo+0x52B8` -> `play+0x4018` -> `dEvt_control_c+0xE0`
  (`mTalkButton`). **A run of equality tests against small consecutive constants
  is usually one inlined predicate.**
- Particle group 0 = `dPtclGroup_Normal_e` = `dComIfGp_particle_set`;
  group 2 = Toon. The `set_pa_*` residual rows are argument-EVALUATION order
  (donor evaluates `current.roomNo` before the `getParticle()` object
  expression); passing the trailing defaults explicitly does NOT change it.

STATE: aj1 **89/131 exact, 88.1198%**. Session: **12/131 -> 89/131,
2.9112% -> 88.1198%** (30.3x fuzzy). 42 remain.

## Rounds 41-48 — 88.12% -> 99.4845% (117/131 exact)

### ⚠ A REFERENCE ERROR I MADE, AND WHY IT MATTERS BEYOND THIS TU

I was reading `build/D44J01/.../d_a_npc_aj1.s` (the **JP** disassembly) while
building **GZLE01** (US). The correct oracle is `build/GZLE01/...`. Two ways
this bites:

1. **Struct offsets shift.** The JP save block sits 0x18 lower — the same field
   is `0x5bbb(r3)` in JP and `0x5bd3(r3)` in US.
2. **The SOURCE SHAPE can differ between versions.** `chk_parts_notMov` is a
   NESTED two-stage test in JP and a FLAT three-way `&&` in US. Transcribing the
   JP shape would have left it permanently unmatched with no visible reason.

objdiff always used the right target, so nothing already-exact was wrong — but
every *manual* `.s` read was against the wrong version. **Check the build dir in
your dump path matches the build target before transcribing anything.**

### New type signals (all measured)

- **`.set(x, y, z)` vs `operator=` on a cXyz.** All-loads-then-all-stores
  (`lfs z; lfs y; lfs x; stfs x; stfs y; stfs z`) is `.set(...)` — args are
  evaluated right-to-left, then stored in order. Interleaved load/store
  (`lfs x; stfs x; lfs y; stfs y; ...`) is memberwise `operator=`. `setAttention`
  was 8 rows purely on this.
- **When a call's args are RIGHT but ordered wrong and one arg is the object
  itself, look for the object's OWN inline method.**
  `emitter->setGlobalRTMatrix(mtx)` expands to
  `JPASetRMtxTVecfromMtx(mtx, mGlobalRotation, mGlobalTranslation)` but puts the
  emitter in its register FIRST and the two `addi`s LAST, which is the target
  order. Writing the free function directly cannot produce that order. **Fixed
  `flw_pa_aka` and cascaded to its sibling `flw_pa_pun` (8 rows) at once.**
- **`current.roomNo` and `fopAcM_GetRoomNo(this)` inline to the same load but
  SCHEDULE DIFFERENTLY.** The raw member makes MWCC materialise the callee
  object (`gameInfo.play.getParticle()`) *before* reading roomNo; the accessor
  reads roomNo first, which is the target order. Took `set_pa_pun` to exact and
  `set_pa_aka` 9 -> 2. **If a call's only diff is "object load and a member read
  are swapped", try the `fopAcM_` accessor form.**
- **`becomeInvalidEmitter()` on a LOCAL vs repeated member writes.** Target loads
  the emitter pointer once into a register and reuses it; writing
  `field_0x794->mMaxFrame = -1; field_0x794->mFlags |= 1;` re-loads. Hoist to a
  local *and* call the named method.
- **A save-struct read that splits into `base+addend` / small displacement is the
  WRONG accessor family.** `dComIfGs_getSelectItem` (save) materialises the
  sub-object address into the relocation addend and emits `lbz 0x9(r3)`;
  `dComIfGp_getSelectItem` (play) keeps one flat member displacement
  (`lbz 0x5bd3(r3)`), which is the target. One row, but unfixable by any amount
  of local rearrangement.
- **The dPa level constant names the wrapper.** `li r4, 0x2` at
  `dPa_control_c::set` is `dPtclGroup_Toon_e` -> `dComIfGp_particle_setToon`,
  not `..._set` (which is level 0, Normal).

### The inverted-branch bucket: a boundary, not just a fix

The single-case `switch` on an integer subject cracks an inverted **EQUALITY**
branch — it took `chngAnmAtr`'s inner `field_0x7b6 == 8` from 5 rows to 2.
**It does NOT crack an inverted RANGE branch.** `chngAnmAtr`'s outer
`i_atr <= 9` wants `ble BODY; b END` and six forms all floor at 2 rows:
nested/flat x `<= 9` / `< 10` / `9 >=` / `!(> 9)`. Recording the boundary so the
next lane does not re-sweep it.

### Residuals, with reasons

| fn | rows | class |
|---|---|---|
| `set_pa_smk` | 6 | literal-pool position (r31 is the .rodata float-pool base) |
| `_nodeCB_BackBone` | 4 | argument-evaluation position (4 call forms swept, all 4 rows) |
| `ctrlAnmAtr` | 2 | branch shape (target has a doubled `b`; explicit `default:` measured no-change) |
| `chngAnmAtr` | 2 | inverted RANGE branch (see above) |
| `set_pa_don` | 2 | literal-pool position |
| `bckResID` / `btpResID` | 3+3 | MWCC local-static `$NNNN` counter; cosmetic, converges as the TU fills |

The literal-pool rows are **convergent, not stuck**: the pool base offsets differ
only because ~14 functions are still unwritten. They should be re-measured after
the TU is complete rather than chased now.

## Rounds 49-53 — 99.4845% -> 99.6653% (125/131 exact)

### ⚠ CORRECTION TO §Rounds 41-48 AND TO CALLS ROW 443

I wrote that the literal-pool residuals were *"convergent, not stuck — they
resolve as the TU fills."* **That was wrong, and the giveaway was in the data I
had already printed.**

They were ONE fixable defect. `static int a_res_id_tbl[]` lands in **`.data`**;
the donor's is `static const int`, which lands in **`.rodata`**. Those two
tables are 0x24 + 0x4 = **0x28** bytes, so their absence shifted every float
constant after them by exactly 0x28.

**One `const` keyword took six functions to exact at once**: `bckResID`,
`btpResID`, `chk_areaIN`, `cut_move_VIVRATE`, `setSmoke`, `shadowDraw`, plus
`set_pa_smk` and `set_pa_don`.

**The rule I should have applied:** a **CONSTANT** offset delta (+0x28) shared
across unrelated functions is a **MISSING POOL OBJECT**. A **VARYING** delta is
scheduling. I had four functions all showing +0x28 and read them as four
independent residuals.

**Diagnostic that finds it in one command:** `objdump -t` the `.o` and compare
each static's **SECTION** against the target's `# .rodata:0xNN` / `# .data:0xNN`
comments. A static in the wrong *section* is invisible in a per-function
instruction diff — it only shows as an offset shift somewhere else entirely.
Cross-checked all four TUs after the fix: **so / ob1 / p2 / aj1 all CLEAN.**

### One more signal

**Implicit `int` -> `bool` on return emits `subfe` + `clrlwi`; an explicit
`!= 0` emits `subfe` alone.** If your return carries one extra `clrlwi` the
donor does not, write the comparison explicitly. (`init_texPttrnAnm`: implicit
with `return false` 2 rows, `return 0` 2, `return FALSE` 2, explicit `!= 0` **0**.)

### The six remaining, with reasons

| fn | rows | class | swept |
|---|---|---|---|
| `setMtx` | 8 | argument-evaluation position | — |
| `_nodeCB_BackBone` | 4 | argument-evaluation position | 4 forms |
| `setAnm_anm` | 3 | inverted branch, **two variables** | 4 forms |
| `_create` | 6 | 4 pool (see below) + 2 inverted branch | — |
| `ctrlAnmAtr` | 2 | branch shape (doubled `b`) | explicit `default:` |
| `chngAnmAtr` | 2 | inverted **RANGE** branch | 6 forms |

**`_create`'s pool rows are a NAMED OPEN ITEM, not a mystery.** The target's
`.rodata` carries three GXColor constants mine does not:
`0xFF000080` {255,0,0,128}, `0x0000FF80` {0,0,255,128}, `0xFFFF0080`
{255,255,0,128}, at 0x94/0x98/0x9C — a 0xC shift that lands on `_create`.
By pool-ID order (@5473/@5475/@5477, immediately after `shadowDraw`'s
@5459-@5461) they belong to **`_draw`**, which already carries the tell-tale
dead block:

```cpp
if (l_HIO.mPrm.field_0x18 != 0) {
    cXyz sp8 = current.pos;
    sp8.y = eyePos.y;      // computed, then used by nothing
}
```

That is a debug-draw block with its `dDbVw_` calls removed. **`_draw` is already
EXACT**, so I am NOT authoring speculative debug calls to force the constants —
that would be inventing donor code to chase 4 rows. **This is the same class as
`so`'s 14 unreferenced GXColor constants (56 bytes): a real, understood,
second data point on one pattern, not two separate puzzles.**

## Round 54 — 126/131 exact, 99.7659%

**The "argument-evaluation position" bucket is not one bucket. It cracks two
ways, and a third case resists both.** All measured on `setMtx` (8 -> 0):

1. **Hoist the object into a local declared IMMEDIATELY BEFORE the call.**
   `field_0x6d0` -> a `subModel` local took it 8 -> 4. **Declaration POINT is
   the lever, not the local's existence:** the same local hoisted to the top of
   the function measured **19 rows — worse than the baseline.**
2. **Use the object's own setter.** `model->setBaseTRMtx(now)` in place of
   `PSMTXCopy(now, model->getBaseTRMtx())` took the remaining 4 -> 0. This is
   the same signal that fixed `flw_pa_aka`/`flw_pa_pun` via
   `emitter->setGlobalRTMatrix()`. A local declared right before the call also
   measured 0, so both spellings are byte-identical; I kept the setter as the
   donor-idiomatic form.
3. **`_nodeCB_BackBone` resists both** — floor of 4 rows across **8 swept
   forms** (cMtx_ wrapper, stack-class XrotM, accessor vs raw member, locals
   hoisted, a local per call, double negation, a jnt pointer local). The target
   loads the angle before materialising `now`; nothing I can write reverses it.
   Parking it as arg-eval position with the sweep recorded so it is not redone.

### Final state for this stretch

| TU | exact | fuzzy |
|---|---|---|
| `d_a_npc_so` | 175/187 | 99.6699% |
| `d_a_npc_ob1` | 109/115 | 99.8324% |
| `d_a_npc_p2` | 133/145 | 99.9503% |
| `d_a_npc_aj1` | **126/131** | **99.7659%** |

**543/578 exact across the four TUs.** `aj1` began this session at 12/131 and
2.9112%.

`aj1`'s five remaining, all named and all swept: `_nodeCB_BackBone` 4 (arg-eval,
8 forms), `setAnm_anm` 3 (inverted branch, two variables, 4 forms),
`chngAnmAtr` 2 (inverted RANGE branch, 6 forms), `ctrlAnmAtr` 2 (branch shape),
`_create` 6 (4 = the three missing GXColor pool constants documented in
§Rounds 49-53, 2 = inverted branch).

## Cross-TU negative result — the narrow-type sweep is DONE, and it found nothing

After `ob1::nodeOb1Control` turned out to be `u16 jntNo` where the donor had
`int` (11 rows -> exact), I said the next step was to check every local's type
across all four TUs. **I ran it mechanically rather than by eye: every
narrow-typed local (`u16`/`s16`/`u8`/`s8`/`short`) declared with an initialiser
inside every non-exact function, widened to `int`, rebuilt, re-measured.**

**18 locals tested. ZERO improvements.** Every one was neutral or worse:

| TU | function | local | was -> after |
|---|---|---|---|
| so | `_execute` | `tilt` (s16) | 5 -> 6 |
| so | `cutEatesaFirstProc` | `a` (s16) | 3 -> 7 |
| so | `cutMiniGameProc` | `angle` (s16) | 10 -> 16 |
| so | `cutMiniGameProc` | `timeUp` (u8) | 10 -> 11 |
| ob1 | `next_msgStatus` | `status` (u16) | 4 -> 5 |
| p2 | `setAnm` | `anm` (s8) | 4 -> 7 |
| p2 | `chkAttention` | `maxAngle` (s16) | 2 -> 2 |
| p2 | `chkAttention` | `angle` (s16) | 2 -> 5 |
| p2 | `anmAtr` | `id` (u8) | 3 -> 4 |
| p2 | `anmAtr` | `type` (u8) | 3 -> 5 |
| p2 | `goal_talkpos_to_goalpos` | `targetAngle` (s16) | 2 -> 9 |
| p2 | `goal_talkpos_to_goalpos` | `procName` (s16) | 2 -> 6 |
| p2 | `goal_goalpos_wait` | `procName` (s16) | 2 -> 3 |
| p2 | `_execute` | `roomNo` (s8) | 5 -> 29 |
| p2 | `cutRideSwitchProc` | `procName` (s16) | 32 -> 36 |
| p2 | `cutRopeTalkProc` | `rotX` (s16) | 13 -> 14 |
| p2 | `cutRopeTalkProc` | `rotZ` (s16) | 13 -> 15 |
| p2 | `cutRopeTalkProc` | `curY` (s16) | 13 -> 13 |

**Conclusion: `nodeOb1Control` was a one-off, not a systemic defect.** The
narrow types everywhere else are correct — several are strongly load-bearing
(`p2::_execute`'s `roomNo` as `s8` is worth 24 rows).

**This is recorded as a CLOSED lead, not an open one.** I named "check every
local's type" as the next step in a report; it is now done, mechanically, and
the answer is no. Nobody should re-run it. (Script:
`C:/Users/xxxxx/AppData/Local/Temp/typesweep.py` — transient, but the method is
three lines: regex the narrow declarations out of each residual function's body,
widen one at a time, rebuild, re-measure, always restoring the file.)

---

## THE DOUBLED-`b` BUCKET IS CRACKED — empty cases, and how to recover their VALUES

Filed as unsolved on `aj1` for many rounds. `ko1` supplied a second instance,
and the second instance is what made it solvable. **Three functions across three
TUs went exact together:** `aj1::ctrlAnmAtr`, `ob1::control_anmAtr`,
`ko1::control_anmAtr`.

### The rule

**A switch dispatch with MORE branches than your cases require means the donor's
switch has EMPTY CASES.** An empty case emits no body, so it is invisible in the
code listing — it exists *only* as extra width in the dispatch tree. That much
was already in this file. **What is new: you can recover the missing case VALUES
from the tree's shape.**

- **Count the branches.** MWCC builds a binary search over the sorted case list.
  N branches implies more cases than you have written.
- **The value tested FIRST is the midpoint of the real case list.** If it is not
  the midpoint of *yours*, cases are missing on whichever side makes it one.

| TU | evidence in the target | recovered |
|---|---|---|
| `ko1` | tests `0xB`, two spare branches | empty `0xA` **and** `0xC`, bracketing |
| `aj1` | cases {2, 5}, doubled `b` | empty `1` and `6`, bracketing |
| `ob1` | tests `3` first with cases {3, 6} | `3` is only the midpoint if something sits BELOW it -> empty `2` |

### The wrong empty case is WORSE than none

Swept every one rather than pattern-matching:
`ko1` — default 2, case-C 4, case-A 1, **A+C 0**.
`aj1` — case-1 7, case-3 7, case-6 1, **6+1 0**, 6+0 10, 6+7 1.
`ob1` — case-0 2, case-1 2, **case-2 0**, 2+7 8.

### ⚠ This retires an entry I had recorded as a floor

`ob1::control_anmAtr` was swept three ways earlier in this same session and
written into `ob1-progress.md` as *"switch-dispatch scan order, floor 6"*.
**That was wrong.** It was reachable; I did not know what to reach for. A
"floor" is only ever a floor for the techniques tried so far, and this one was
recorded with more confidence than three sweeps earn.

### The scan that followed, and its false-positive rate

I then scanned all five TUs for residuals where the target carries more branches
than my build. **80 hits, and almost all were junk** — unwritten `ko1` stubs
show as `N/0` because a `/* Nonmatching */` body has no branches at all. Same
failure mode as the first `cror` scan: a plausible worklist that is mostly an
artifact of what is not written yet. **Filter to written functions before
believing any diff-shape scan on a partially-decoded TU.**

Filtered, four real candidates remain, each **+1 branch** — and every one is a
function I had previously parked:

| fn | rows | I had called it |
|---|---|---|
| `so::jntHitCreateHeap` | 4 | branch shape, 4 forms swept |
| `ob1::chg_anmAtr` | 2 | two-variable compare, "provably unfixable" |
| `aj1::setAnm_anm` | 3 | two-variable inverted branch |
| `aj1::chngAnmAtr` | 2 | inverted RANGE branch, 6 forms swept |

A +1 branch is also the plain inverted-branch signature, so these are not
automatically empty-case cases. **But three of the four were parked as
"provably" unreachable, and the doubled-`b` bucket was parked the same way and
turned out not to be.** They are worth re-opening with fresh eyes, not
re-closing on the strength of the earlier verdict.

### The four candidates re-opened — and they do NOT fall to the empty-case rule

I said these were worth re-opening rather than re-closing on the earlier
verdict. Done, and the earlier verdict holds:

- **`so::jntHitCreateHeap`** — the only one where the trick could plausibly
  apply, because it compares a POINTER to NULL and that is a comparison with a
  CONSTANT. Tested `switch ((u32)mAA8) { case 0: ... }` — **worse, 5 rows.**
  Plus empty-then-branch 4 and early-return 4, against a baseline of 4.
- **`ob1::chg_anmAtr`** and **`aj1::setAnm_anm`** — both compare two VARIABLES
  (`i_no != field_0x800`, `field_0x7b9 != i_prm->field_0x0`). No switch can
  express that, and the Round 28 boundary stands.
- **`aj1::chngAnmAtr`** — an inverted RANGE (`i_atr <= 9`). A switch over a dense
  0..9 set compiles to a jump table, which the target does not have.

**Conclusion: the +1-branch signature is ambiguous.** It fires for BOTH the
empty-case case and the ordinary inverted-branch family, and only the former is
fixable. The discriminator is whether the construct is a **switch dispatch**
(several compares forming a search tree) or a **single inverted compare**. The
scan cannot tell them apart; a human look at the target's branch structure can,
in about ten seconds.

So the crack is real but narrower than the scan implied: **it applies to switch
dispatches only.** Recording that so the next pass does not re-run these four.
