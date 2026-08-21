# ym1-progress — d_a_npc_ym1 campaign anchor

## Kickoff (2026-08-21, fifth-instance session)

Baseline: **8/124 exact, fuzzy 2.49%** (exact_delta snapshot written at open).
81 `/* Nonmatching */` stubs. Queue position: after ko1 (fully written,
180/203); before yw1 (8/119, likely sibling).

**FIRST STRUCTURAL FACT, and it changes the template strategy:
`daNpc_Ym1_c : public fopAc_ac_c` — NOT fopNpc_npc_c.** The ob1/ko1/so/aj1
playbook (fopNpc member map at 0x290-0x6BC, m_jnt, mEventCut, PathRun idioms)
does NOT carry over structurally. yw1 is the likely 1:1 sibling (same base,
near-identical size 119 vs 124 — the ob1<->ko1 relationship again). RULE FROM
ko1: read the sibling's SOURCE before decoding any asm; four functions were
re-derived from scratch that ob1 already had.

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
