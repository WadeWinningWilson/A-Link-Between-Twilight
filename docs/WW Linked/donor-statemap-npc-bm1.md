# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_bm1.cpp`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions
- L2937 `switch (mActionIndex)` → cases: `0`, `1`
- L2946 `switch (mActionIndex)` → cases: `0`, `1`

## Transition writes (mode-like assignments, line-cited)
- L2932 `mActionIndex = dComIfGp_evmng_getMyActIdx(arg0, a_cut_tbl, ARRAY_SIZE(a_cut_tbl), 1, 0);`
- L3015 `mCurrActionFunc = i_action;`
- L4264 `mpBinderModel = NULL;`
- L4265 `mpBagModel = NULL;`
- L4266 `mpKnifeModel = NULL;`
- L4267 `mpStickModel = NULL;`
- L4271 `mpBinderModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000002);`
- L4279 `mpBagModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000002);`
- L4287 `mpKnifeModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000002);`
- L4295 `mpStickModel = mDoExt_J3DModel__create(a_mdl_dat, 0x80000, 0x11000002);`

(10 transition writes)

## setAction/setMode-style calls (line-cited, first 60)

## NPC idiom — set_action transitions (function-pointer states)
- L548 `set_action(&daNpc_Bm1_c::demo_action1, NULL);`
- L558 `set_action(&daNpc_Bm1_c::wait_action1, NULL);`
- L566 `set_action(&daNpc_Bm1_c::wait_action3, NULL);`
- L578 `set_action(&daNpc_Bm1_c::wait_action1, NULL);`
- L590 `set_action(&daNpc_Bm1_c::wait_action9, NULL);`
- L599 `set_action(&daNpc_Bm1_c::wait_action6, NULL);`
- L610 `set_action(&daNpc_Bm1_c::wait_action8, NULL);`
- L616 `set_action(&daNpc_Bm1_c::wait_action2, NULL);`
- L626 `set_action(&daNpc_Bm1_c::wait_action4, NULL);`
- L638 `set_action(&daNpc_Bm1_c::wait_action5, NULL);`
- L647 `set_action(&daNpc_Bm1_c::wait_actionA, NULL);`
- L670 `set_action(&daNpc_Bm1_c::wait_action7, NULL);`
- L3009 `BOOL daNpc_Bm1_c::set_action(ActionFunc i_action, void* i_param_2) {`

(13 set_action transitions)

## NPC idiom — action-method state roster
`demo_action1`, `event_action`, `event_actionInit`, `set_action`, `wait_action1`, `wait_action2`, `wait_action3`, `wait_action4`, `wait_action5`, `wait_action6`, `wait_action7`, `wait_action8`, `wait_action9`, `wait_actionA`
