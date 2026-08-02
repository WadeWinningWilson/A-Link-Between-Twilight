# Donor state map — `D:\XXXXXXX\WW DP\src\d\actor\d_a_esa.cpp`

> Mechanical extraction (Foundry P13). Every row is a verbatim quote with
> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name
> them only from decomp evidence, never invention.

## Enums (candidate state vocabularies)

## switch() dispatchers over mode-like expressions
- L93 `switch (i_this->mActionState)` → cases: `0`, `1`, `2`, `3`, `4`

## Transition writes (mode-like assignments, line-cited)
- L104 `i_this->mActionState = 1;`
- L109 `i_this->mActionState = 3;`
- L135 `i_this->mActionState = 2;`
- L166 `i_this->mActionState = 4;`
- L171 `i_this->mActionState = 4;`
- L231 `i_this->mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);`

(6 transition writes)

## setAction/setMode-style calls (line-cited, first 60)

## NPC idiom — set_action transitions (function-pointer states)

(0 set_action transitions)
