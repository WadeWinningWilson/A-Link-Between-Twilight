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
