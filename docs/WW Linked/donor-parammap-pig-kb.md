# Donor params decode map

## `D:\XXXXXXX\WW DP\include\d\actor\d_a_kb.h`

(0 param-related lines)

## `D:\XXXXXXX\WW DP\src\d\actor\d_a_kb.cpp`
- L2475 `i_this->mShapeType = (fopAcM_GetParam(i_this) >> 0) & 0xF;`
- L2476 `i_this->m401 = (fopAcM_GetParam(i_this) >> 4) & 0xF;`
- L2477 `i_this->mbCanBeBigPig = (fopAcM_GetParam(i_this) >> 8);`
- L2478 `i_this->m41D = (fopAcM_GetParam(i_this) >> 0x10);`
- L2534 `fopAcM_SetParam(i_this, fopAcM_GetParam(i_this) + 8);`

(5 param-related lines)

> Field semantics beyond the donor's own identifiers are [INFERENCE-NEEDED]; verify against decomp usage before labeling subtypes in any census.
