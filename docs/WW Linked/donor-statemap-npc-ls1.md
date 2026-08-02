# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ls1.cpp`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions
- L1562 `switch (mActionIndex)` → cases: `0`, `1`, `2`, `3`
- L1579 `switch (mActionIndex)` → cases: `0`, `1`, `2`, `3`

## Transition writes (mode-like assignments, line-cited)
- L1554 `mActionIndex = dComIfGp_evmng_getMyActIdx(i_staffIdx, a_cut_tbl, ARRAY_SSIZE(a_cut_tbl), TRUE, 0);`
- L2552 `mpLsHandModel = NULL;`
- L2555 `mpLsHandModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000022);`
- L2575 `mpTelescopeModel = NULL;`
- L2586 `mpTelescopeModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000022);`

(5 transition writes)

## setAction/setMode-style calls (line-cited, first 60)

## NPC idiom — set_action transitions (function-pointer states)
- L257 `set_action(&daNpc_Ls1_c::wait_action1, NULL);`
- L264 `set_action(&daNpc_Ls1_c::demo_action1, NULL);`
- L1663 `BOOL daNpc_Ls1_c::set_action(ProcFunc i_newProcFunc, void* i_argsP) {`

(3 set_action transitions)

## NPC idiom — action-method state roster
`demo_action1`, `set_action`, `wait_action1`
