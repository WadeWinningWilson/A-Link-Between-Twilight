# COVERAGE vs CEILING (coverage_join.py, V10-b — GENERATED)

Feed: dusklight-20260814-012809.log  — mode: SINGLE RUN (resolve is deduped per-run)
Weighting: PLACEMENTS (never actor count). Area: the planner's Outset stage set.

| metric | placements | share |
|---|---|---|
| EXERCISED (ran in real play) | 535 | **59.5%** |
| CEILING (portable today) | 788 | **87.7%** |
| total placed | 899 | 100% |

**THE BOTTLENECK IS US** — 28.1 points of headroom sit inside what is already portable. More porting effort converts directly.

## Levels (V10-a contract)

- LINKED: layer=ww_profile rows=33 pending=0 mismatch=0 handed_over(=relinquished, the GOOD state per §920)=33
- EXERCISED names: 13 (EXT_EP, EXT_VEG, KNOB00, LAMP, Lwood, NPC_BA1, OBJ_OTBLE, Obj_Mshokki, Obj_Shelf, SPC_ITEM01)
- UNRESOLVED worklist: EMPTY this run — the healthy case (empty, not absent)

## Honest scope

- EXERCISED counts what ONE run touched; a run that never entered a house cannot exercise its props. Coverage rises with playtest breadth, so read it per-run, not as a project total.
- CEILING moves only with upstream decomp. That is the number this whole metric exists to expose.
