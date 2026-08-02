# Donor params decode map

## `D:\XXXXXXX\WW DP\include\d\actor\d_a_obj_ikada.h`
- L26 `bool isBonbori() { return mType == 3 || mType == 1; }`
- L27 `bool isCrane() {return mType == 4; }`
- L28 `bool isFlag() { return mType == 0 || mType == 4; }`
- L29 `bool isWave() { return mType == 4 || mType == 3 || mType == 1; }`

(4 param-related lines)

## `D:\XXXXXXX\WW DP\src\d\actor\d_a_obj_ikada.cpp`
- L188 `if (mType == 1) {`
- L593 `if (mType == 4) {`
- L599 `if (mType == 4) {`
- L647 `if ((mType == 1) || (mType == 3)) {`
- L671 `if (mType != 4) {`
- L725 `if (mType == 4) {`
- L952 `if (mType == 1) {`
- L961 `} else if (mType == 3) {`
- L1002 `if (mType == 1) {`
- L1010 `} else if (mType == 3 && dVar4 >= l_HIO.m98) {`
- L1136 `if (mType == 4) {`
- L1140 `if (mType == 4) {`
- L1191 `if (mType == 1 || mType == 3) {`
- L1245 `if (mType == 4) {`
- L1276 `u32 param = fopAcM_GetParam(this);`
- L1279 `mType = fopAcM_GetParamBit(param, 0, 4);`
- L1280 `if (mType != 4) {`
- L1281 `m0294 = fopAcM_GetParamBit(param, 4, 6);`
- L1282 `m0298 = fopAcM_GetParamBit(param, 10, 8);`
- L1283 `m02A0 = fopAcM_GetParamBit(param, 18, 8);`
- L1284 `m029C = fopAcM_GetParamBit(prmX, 0, 8);`
- L1285 `mPathId = fopAcM_GetParamBit(prmX, 8, 8);`
- L1287 `mPathId = fopAcM_GetParamBit(param, 16, 8);`
- L1296 `if ((mType == 1) || (mType == 2) || (mType == 0)) {`
- L1300 `if (mPathId != 0xFF && (mType == 4 || mType == 1 || mType == 3)) {`
- L1302 `if (mType == 4) {`
- L1309 `if (mType == 1 || mType == 3) {`
- L1313 `if (mType == 4) {`
- L1326 `if (mType != 4) {`
- L1333 `if (mType == 1) {`
- L1337 `if (mType == 1) {`
- L1372 `mFlagPcId = fopAcM_create(fpcNm_MAJUU_FLAG_e, params[mType], &current.pos, tevStr.mRoomNo, &current.angle);`
- L1373 `mFlagOffset = flag_offset[mType];`
- L1374 `mFlagScale = flag_scale[mType];`
- L1380 `if (mType == 4) {`
- L1433 `J3DModelData* modelData = (J3DModelData*)dComIfG_getObjectRes(m_arc_name, bdl[mType]);`
- L1441 `if (mType == 4) {`
- L1452 `if (mType == 4) {`
- L1471 `cBgD_t* cbgd = (cBgD_t*)dComIfG_getObjectRes(m_arc_name, dzb[mType]);`
- L1476 `if (mType == 4) {`
- L1503 `if (!fopAcM_entrySolidHeap(this, createHeap_CB, m_heapsize[mType])) {`

(41 param-related lines)

> Field semantics beyond the donor's own identifiers are [INFERENCE-NEEDED]; verify against decomp usage before labeling subtypes in any census.
