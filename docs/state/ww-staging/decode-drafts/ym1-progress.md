# ym1-progress — d_a_npc_ym1 campaign anchor

## Kickoff (2026-08-21, fifth-instance session)

Baseline: **8/124 exact, fuzzy 2.49%** (exact_delta snapshot written at open).
81 `/* Nonmatching */` stubs. Queue position: after ko1 (fully written,
180/203); before yw1 (8/119, likely sibling).

**FIRST STRUCTURAL FACT — CORRECTED WITHIN THE HOUR, against the asm:
the skeleton header says `: public fopAc_ac_c` and THE HEADER IS WRONG.**
The target's `__ct__11daNpc_Ym1_cFv` (weak) stores `__vt__12fopNpc_npc_c` at
0x6C0 and constructs the complete fopNpc member set at ko1's exact offsets
(m_jnt fields at 0x29B/0x29C, mObjAcch at 0x334, mAcchCir at 0x4F8, mStts at
0x550, mCyl at 0x574, mCurrMsgBsPcId = -1 at 0x6AC). **ym1 IS
fopNpc_npc_c-derived; the ob1/ko1 playbook carries in full.** My kickoff
filing repeated the skeleton header as ground truth — the same
unverified-premise failure the estate has on record; the asm is the ground
truth. **FIRST WORK ITEM: fix the header's base class**, which makes the
0x290-0x6BC region free (inherited) and leaves only ym1's own members past
0x6C4 to map. yw1 (8/119, near-identical size) is the likely 1:1 sibling —
read its source AND check its header for the same wrong base before decoding.

Biggest stubs (pool_position census at open): bodyCreateHeap 192 ·
_execute 174 · createInit 172 · next_msgStatus 172 · talk_1 145 · _draw 128 ·
wait_2 121 · wait_1 109 · set_collision_sp 102 · kari_1/wait_action3 91 ·
lookBack 90 · NBTwai 86 · (68 more).

Levers in force from the ko1 campaign (see ko1-progress + handoff UPDATE 10):
constness-is-section-placement · truncation-shape-names-the-type · explicit
empty cases densify jump tables · label-address order = source order ·
macro-owned NULL checks (fopAcM_GetID) · the || tell (forward branch into the
other arm's tail) · exact_delta before EVERY commit, pool_position from the
DECOMP ROOT only.

Next actions: ctor member map from `__ct` asm -> header layout confirmation,
then smallest-fns-first with per-fn objdiff.


## Batches 1-5 (same session as kickoff): 8 -> 33/124, every written function byte-exact

25 functions written, zero parked except chngAnmAtr (2 rows, the
unfolded-branch family's second witness - ble-over-b resists 4 shapes).
Levers re-confirmed in the new TU: ret-variable vs bool-normalize (x2),
truncation-shape/lever-6 both directions (chk_BlackPig bool retype flips its
CALLER), isEventBit ternary = subic/subfe + base, the && empty-branch beqlr
tell, ko1's set_action form verbatim (9/0 teardown states identical).

NEXT: the anm cluster as one unit - anm_prm_c struct (0x10 stride: s8 id,
u8 btpNo at +1, three f32 at 4/8/C presumed), the a_anm_prm_tbl statics
($4537 setAnm on m8AD, $4575 setAnm_ATR on m8A8 - .data, NON-const per the
section-placement lever), then setAnm_anm/init_texPttrnAnm/play_texPttrnAnm
and the wait/talk action ladder above it.


## Ladder batches 6-8 (same session): 33 -> 47/124

Byte-exact: setMtx (ob1 spellings + hoisted-MtxP lever, first build),
nodeCB_BackBone, decideType (logic-exact, 1 string row - "Halt" owner
pending), NBTwai; logic-exact: set_cutGrass (checkFrame(34) + hand-L joint
particle + seStart), SITwai (dNpc_chkAttn by-value player pos). The anm
cluster closed earlier at three setters exact via ONE retype.

Field-signedness tells cashed this stretch: m8AF s8, m898 s8 (extsb at
reads), anm_prm_c.field_0x1 s8 (bare lbz at the Sc argument). The donor's
own member names recovered from the assert-string table (m_hed_jnt_num
family, a_btp).

STANDING ORDER integrated: four-version verification acknowledged on the
board; this lane runs the Matching flip + gate itself at each TU close.


## Batches 9-10: getMsg family complete + kari/area pair — 50/124

All seven getMsg fns byte-exact (dispatcher = 6 dense cases under the
jump-table threshold; YM2_2's last row was lever 4 verbatim). chk_areaIN
is a HEIGHT VARIANT of ko1's chk_areaIn (adds fabsf(dy) < 300) — assuming
the sibling without reading cost one round, again. kari_1 written with the
dead-sqrt idiom; its 12 rows are the by-value-arg stack-slot family
(flat-before-delta measures worse, so current order is right — same class
as ko1's movPass/wait_7, PARKED with them).

Remaining ym1 stubs (larger half): talk_1 145 · wait_1/2 109/121 ·
wait_action1-4 · createInit 172 · bodyCreateHeap 192 · head/itemCreateHeap ·
CreateHeap · _create 74 · next_msgStatus 172 · _execute 174 · _draw 128 ·
lookBack 90 · turn_1 · eventOrder done · privateCut · endEvent done ·
init_YM* family · chngAnmTag/ctrlAnmTag/ctrlAnmAtr · shadowDraw · demo.


## Batch 11 (2026-08-21, post-compaction resume): createInit closed — 98/124

Byte-exact: createInit (WWDP b3432a6c), _nodeCB_BackBone; decideType
snapped exact as a side effect of the init_YM* retype. _nodeCB_Head is
LOGIC-EXACT, parked on bss-position (the static a_eye_pos_off guard block
@3569 sits 84 bytes earlier in my .bss than retail — earlier .bss content
not yet emitted; converges with the TU).

createInit decode facts:
- staff jump table @4408: cases 0-5 -> init_YM1_0/1, init_YM2_0/1/2/3;
  cases 6 AND 7 share one label -> init_YMx_error; default (>7) ret=0.
- a_att_dis_TBL[3][2] u8 .data = {{0,0},{0xAA,0xAA},{0xAA,0xAA}} (the
  earlier "6 bytes of 0xAA" read was WRONG — first pair is zeros).
- a_staff_tbl char*[8] .data = Ym1,Ym1,Ym2,Ym2,Ym2,Ym2,Ym1,Ym2.
- kari mass-cyl block (mStaff==0): m704.SetStts/Set, offset cXyz(0,0,80)
  rotated by current.angle.y, SetC/SetR(50)/SetH(30),
  dComIfG_Ccsp()->SetMass(&m704, 3). Pool: gravity -4.5f (same as ob1).
- TYPE SIGNAL cashed: single clrlwi. at the switch JOIN (no per-case
  truncation) => init_YM* return bool, ret is bool. int-returning callees
  + u8/bool ret both add a per-case clrlwi and miss by 7 rows.

NEW LEVER (proven on _nodeCB_BackBone, 4-row swing):
mDoMtx_stack_c::XrotM(m_jnt.getBackbone_y()) — the WRAPPER form
pre-evaluates the inline-accessor argument (lha lands BEFORE the lis/addi
of &now). The direct mDoMtx_XrotM(mDoMtx_stack_c::now, accessor()) form
evaluates left-to-right and misses. Note the asymmetry: with a PLAIN FIELD
argument (createInit's YrotM(current.angle.y)) the wrapper emits &now
first — the pre-evaluation only fires for inline-call arguments.

Header corrections: 0x704 cyl renamed m704 (was shadowing
fopNpc_npc_c::mCyl at 0x574 — first block of createInit uses the BASE
mStts/mCyl, kari block uses the derived m704); m84C/m870 carved to cXyz.

Remaining (~26 non-exact): bodyCreateHeap 192 · headCreateHeap ·
itemCreateHeap · CreateHeap · next_msgStatus 172 · _execute 174 ·
_draw 128 · demo 78 · privateCut · setStt · anmAtr · chk_talk ·
chkAttention · setAttention · set_collision_sp · plus the pool/bss parks
(init_texPttrnAnm, setAnm_anm, chngAnmAtr, kari_1, _nodeCB_Head,
set_cutGrass, chk_areaIN, wait_2, SITwai, shadowDraw, _create, near-100s).
NEXT: the createHeap family as one cluster (share arc/model vocabulary),
then next_msgStatus/_execute/_draw, then the talk/attention cluster.


## Batches 12-17 (2026-08-21, same session): 98 -> 121/124 — ALL functions written

Byte-exact this stretch: CreateHeap, init_texPttrnAnm (the 98.36 park closed:
`return mBtpAnm.init(...) != 0;` — explicit !=0 in a bool fn emits subfe
alone; implicit int->bool adds a defensive clrlwi), privateCut, body/head/
itemCreateHeap (snapped by privateCut's "DUMMY" emission), setStt, anmAtr,
chk_talk, chkAttention, setAttention, chk_nbt_attn (held), next_msgStatus,
demo, _execute, _draw, _create, and the SIX pool-shift victims
(chk_areaIN, kari_1->99.9, set_collision_sp, set_cutGrass, shadowDraw,
wait_2, SITwai) + _nodeCB_Head.

THE BIG POOL FIND: btpResID's a_res_id_tbl is a SINGLE-entry {0xE} static
(donor indexes out of bounds by design). My 13-entry copy inserted 48 pool
bytes and shifted every later constant — one fix snapped six functions.
Corollary: a systematic pool shift across many 99.9x functions has ONE
upstream cause; find it instead of parking each function.

Orphan reproduction levers (both proven):
- 3 orphaned pool words FF000080/0000FF80/FFFF0080 = stripped debug-draw
  GXColors; daiocta's unreferenced-GXColor-local idiom emits them POOL-ONLY
  (no code rows) inside _draw's field_0x18 debug block.
- ba1's partner-search scratch (fopAc_ac_c* l_check_inf[20] + int
  l_check_wrk) exists ORPHANED in ym1 .bss; declaring them unreferenced
  after l_HIO restores the 84-byte bss layout and snapped _nodeCB_Head.

Retypes cashed by signal: init_YM* -> bool (single join clrlwi), m8B3 s8
(extsb.), m8A6 bool (plain-lbz return), m8A7 s8, a_tex_pttrn_num_tbl s8
(bare lbzx at Sc arg), chk_nbt_attn BOOL->bool + bool local,
body/head/itemCreateHeap void->BOOL (caller cmpwi), demo void->bool.
Carves: mHomePos cXyz + mHomeAngle csXyz (mRotYTarget was mHomeAngle.y);
prm field_0x18 f32 -> 4 u8 debug flags (lbz in _draw, no lfs anywhere).
Other levers: set_collision_sp's shared `off` local at function top
(per-case cXyz locals cost 16 frame bytes — MWCC does not overlap block
locals) + `f32 h, r;` declaration order (h=f31, r=f30);
setAttention's pos is COPY-INIT (interleaved one-reg) not ctor (batched).

REMAINING 3 (harness-class, ledgered):
- kari_1 99.9: named cXyz local vs by-value arg temp SLOT SWAP (target has
  the arg temp ABOVE the named local; flat-first/last both falsified).
- setAnm_anm 95.98: mine duplicates `li r3,1` at two return sites; target
  shares one tail.
- chngAnmAtr 93.12: target's `ble body; b end` unfolded pair. SEVEN source
  shapes falsified (&&, nested if, early return, empty-then/else, literal
  gotos) — the optimizer folds them all to bgt. Do NOT re-run these.
TU fuzzy 99.93. Matching flip + four-version gate DEFERRED until these 3
close (harness or yw1-sibling insight). NEXT: yw1 (8/119), expected ~1:1.
