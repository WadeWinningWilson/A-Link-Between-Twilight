# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_bmsw.cpp`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions

## Transition writes (mode-like assignments, line-cited)
- L1303 `mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11020002);`

(1 transition writes)

## setAction/setMode-style calls (line-cited, first 60)
- L586 `setAction(&daNpc_Bmsw_c::wait_action, NULL);`
- L755 `setAction(&daNpc_Bmsw_c::shiwake_game_action, NULL);`
- L1006 `setAction(&daNpc_Bmsw_c::wait_action, NULL);`

## NPC idiom — set_action transitions (function-pointer states)

(0 set_action transitions)

## NPC idiom — action-method state roster
`shiwake_game_action`, `wait_action`
