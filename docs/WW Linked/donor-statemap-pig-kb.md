# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_kb.cpp`, `D:\XXXXXXX\WW DP\include\d\actor\d_a_kb.h`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions
- L742 `switch (i_this->m420)` → cases: `0`, `1`, `2`, `3`, `4`, `5`, `6`
- L935 `switch (i_this->m420)` → cases: `0xA`, `0xB`, `0xC`, `0xD`
- L1308 `switch (i_this->m420)` → cases: `0x14`, `0x15`, `0x16`, `0x17`, `0x18`, `0x19`
- L1529 `switch (i_this->m420)` → cases: `0x1E`, `0x1F`, `0x20`, `0x21`, `0x22`, `0x23`, `0x24`
- L1812 `switch (i_this->m420)` → cases: `0x28`, `0x29`, `0x2A`, `0x2B`, `0x2C`, `0x2D`, `0x2E`, `0x2F`, `0x30`, `0x31`

## Transition writes (mode-like assignments, line-cited)
- L2383 `pModelData = (J3DModelData*)dComIfG_getObjectRes("Kb", dRes_INDEX_KB_BDL_PG_BIG_e);`
- L2386 `pModelData = (J3DModelData*)dComIfG_getObjectRes("Kb", dRes_INDEX_KB_BDL_PG_e);`

(2 transition writes)

## Helper-call transition writes (numeric targets, line-cited)

(0 helper-call writes)

## Anonymous state-field transitions (fields m420 — switched on ≥2×; [INFERENCE-NEEDED: donor field names pending decomp naming])
- L157 `i_this->m420 = 10;`
- L321 `i_this->m420 = 0x14;`
- L365 `i_this->m420 = 4;`
- L381 `i_this->m420 = 0x21;`
- L406 `i_this->m420 = 0x23;`
- L714 `i_this->m420 = 0x28;`
- L760 `i_this->m420 = 2;`
- L769 `i_this->m420 = 4;`
- L793 `i_this->m420 = 0;`
- L808 `i_this->m420 = 4;`
- L869 `i_this->m420 = 0;`
- L877 `i_this->m420 = 2;`
- L957 `i_this->m420 = 0xB;`
- L981 `i_this->m420 = 0xB;`
- L1061 `i_this->m420 = 0xD;`
- L1077 `i_this->m420 = 0xC;`
- L1148 `i_this->m420 = 0x14;`
- L1187 `i_this->m420 = 0xD;`
- L1229 `i_this->m420 = 6;`
- L1248 `i_this->m420 = 0x24;`
- L1273 `i_this->m420 = 0xD;`
- L1289 `i_this->m420 = 0xC;`
- L1399 `i_this->m420 = 0x18;`
- L1403 `i_this->m420 = 0x18;`
- L1472 `i_this->m420 = 2;`
- L1571 `i_this->m420 = 4;`
- L1611 `i_this->m420 = 0x23;`
- L1635 `i_this->m420 = 0x24;`
- L1662 `i_this->m420 = 0x22;`
- L1882 `i_this->m420 = 0x31;`
- L1940 `i_this->m420 = 0x31;`
- L1965 `i_this->m420 = 0x31;`
- L2123 `i_this->m420 = 2;`
- L2326 `i_this->m420 = 2;`

(34 anonymous-field transitions)

## setAction/setMode-style calls (line-cited, first 60)

## NPC idiom — set_action transitions (function-pointer states)

(0 set_action transitions)
