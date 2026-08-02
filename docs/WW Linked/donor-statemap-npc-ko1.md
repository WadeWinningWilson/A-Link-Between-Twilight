# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ko1.cpp`, `D:\XXXXXXX\WW DP\include\d\actor\d_a_npc_ko1.h`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions

## Transition writes (mode-like assignments, line-cited)

(0 transition writes)

## setAction/setMode-style calls (line-cited, first 60)

## NPC idiom — set_action transitions (function-pointer states)
- L445 `void daNpc_Ko1_c::set_action(int (daNpc_Ko1_c::*)(void*), void*) {`
- L837 `void set_action(int (daNpc_Ko1_c::*)(void*), void*);`

(2 set_action transitions)

## NPC idiom — action-method state roster
`event_action`, `event_actionInit`, `hana_action1`, `hana_action2`, `hana_action3`, `hana_action4`, `hana_action5`, `set_action`, `wait_action1`, `wait_action2`, `wait_action3`, `wait_action4`
