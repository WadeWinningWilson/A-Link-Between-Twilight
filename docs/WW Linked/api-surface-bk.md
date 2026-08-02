# API surface — `d_a_bk.cpp` vs receiver

**187/218 external identifiers present in receiver = 85.8% coverage.** Missing list below is the direct-port work plan (shim/rename/implement per item).

## MISSING (31) — the work plan
| identifier | uses |
|---|---|
| `setMaterialTable` | 4 |
| `checkGrabWear` | 3 |
| `setBkControl` | 2 |
| `setBtAttackData` | 2 |
| `setBtNowFrame` | 2 |
| `enemy_fire_remove` | 2 |
| `JntHit_create` | 2 |
| `setRotAngleSpeed` | 1 |
| `moveStateInit` | 1 |
| `GetAttributeCode` | 1 |
| `dComIfGp_particle_setToon` | 1 |
| `iceEntryDL` | 1 |
| `dComIfGd_setListMaskOff` | 1 |
| `dSnap_RegistFig` | 1 |
| `MtxRotY` | 1 |
| `setChildId` | 1 |
| `setBtMaxDis` | 1 |
| `enemy_piyo_set` | 1 |
| `setVolumeSweep` | 1 |
| `getFindFlag` | 1 |
| `setFindFlag` | 1 |
| `enemy_ice` | 1 |
| `CPad_CHECK_TRIG_B` | 1 |
| `CPad_CHECK_HOLD_Y` | 1 |
| `damage_reaction` | 1 |
| `enemy_fire` | 1 |
| `dComIfG_resDeleteDemo` | 1 |
| `dComIfGp_CharTbl` | 1 |
| `GetNameIndex` | 1 |
| `fopAcM_adjustHeap` | 1 |
| `initBt` | 1 |

## PRESENT (187) — top 40 by use
| identifier | uses |
|---|---|
| `cM_rndF` | 50 |
| `MtxPosition` | 48 |
| `fopAcM_monsSeStart` | 36 |
| `cLib_addCalcAngleS2` | 33 |
| `cLib_addCalc2` | 27 |
| `dComIfG_getObjectRes` | 25 |
| `cMtx_YrotS` | 24 |
| `isStop` | 23 |
| `ChkGroundHit` | 21 |
| `getModel` | 20 |
| `cMtx_YrotM` | 20 |
| `cM_atan2s` | 16 |
| `dComIfG_Bgsp` | 14 |
| `dComIfGp_getPlayer` | 13 |
| `fopAcM_GetRoomNo` | 13 |
| `MtxTrans` | 13 |
| `getModelData` | 12 |
| `dComIfG_Ccsp` | 12 |
| `fopAcM_SearchByID` | 10 |
| `fopAcM_GetParam` | 10 |
| `MTXCopy` | 9 |
| `SetR` | 9 |
| `fopAcM_GetName` | 8 |
| `SetC` | 8 |
| `getAnmMtx` | 7 |
| `cLib_addCalc0` | 7 |
| `LineCross` | 6 |
| `cMtx_XrotM` | 6 |
| `getJointNodePointer` | 6 |
| `dComIfGs_isSwitch` | 6 |
| `SetPos` | 6 |
| `GroundCross` | 6 |
| `fopAc_IsActor` | 6 |
| `fpcM_Search` | 6 |
| `fopAcM_OffStatus` | 6 |
| `mDoExt_J3DModel__create` | 6 |
| `cM_rndFX` | 5 |
| `remove` | 5 |
| `dComIfGp_particle_set` | 5 |
| `cMtx_ZrotM` | 5 |
