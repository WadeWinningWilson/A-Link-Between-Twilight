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
