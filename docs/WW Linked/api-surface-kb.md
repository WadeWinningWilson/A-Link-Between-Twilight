# API surface — `d_a_kb.cpp` vs receiver

**169/183 external identifiers present in receiver = 92.3% coverage.** Missing list below is the direct-port work plan (shim/rename/implement per item).

## MISSING (14) — the work plan
| identifier | uses |
|---|---|
| `GetAttributeCode` | 5 |
| `dComIfGp_particle_setToon` | 3 |
| `dComIfGp_particle_setShipTail` | 2 |
| `dComIfGs_getSelectEquip` | 2 |
| `dSnap_RegistFig` | 2 |
| `OnAtHitBit` | 2 |
| `ClrAtSet` | 2 |
| `getGrabMissActor` | 1 |
| `onWindOff` | 1 |
| `setTexNoAnimator` | 1 |
| `setMaterialTable` | 1 |
| `kb_dig` | 1 |
| `fopAcM_getGroundAngle` | 1 |
| `setAnmIndex` | 1 |

## PRESENT (169) — top 40 by use
| identifier | uses |
|---|---|
| `fopAcM_monsSeStart` | 34 |
| `getEmitter` | 26 |
| `cLib_addCalcAngleS2` | 21 |
| `dComIfG_Bgsp` | 18 |
| `cM_rndF` | 18 |
| `GetGroundH` | 15 |
| `dComIfG_getObjectRes` | 12 |
| `fopAcM_GetRoomNo` | 12 |
| `getModel` | 11 |
| `cM_rndFX` | 11 |
| `MtxPosition` | 10 |
| `setRate` | 10 |
| `cMtx_YrotS` | 8 |
| `checkFrame` | 8 |
| `fopAcM_checkCarryNow` | 7 |
| `daPy_getPlayerActorClass` | 7 |
| `GetHeight` | 7 |
| `fopAcM_seStart` | 7 |
| `dComIfGp_particle_set` | 7 |
| `cLib_addCalc2` | 7 |
| `cM_atan2s` | 7 |
| `isStop` | 7 |
| `cLib_distanceAngleS` | 6 |
| `temp` | 6 |
| `ChkGroundHit` | 6 |
| `SetWeight` | 6 |
| `dComIfGp_getStartStageName` | 5 |
| `fopAcM_GetParam` | 5 |
| `LineCross` | 4 |
| `setall` | 4 |
| `setGlobalScale` | 4 |
| `getModelData` | 4 |
| `cM_rnd` | 4 |
| `fopAcM_searchPlayerAngleY` | 4 |
| `ChkWallHit` | 4 |
| `dComIfGp_getVibration` | 4 |
| `StartShock` | 4 |
| `cXyz` | 4 |
| `GetSpecialCode` | 4 |
| `cLib_addCalc0` | 4 |
