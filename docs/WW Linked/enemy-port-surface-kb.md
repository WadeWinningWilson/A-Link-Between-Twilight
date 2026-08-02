# API surface — `d_a_kb.cpp`, `d_a_kb.h` vs receiver

**172/182 external identifiers present in receiver = 94.5% coverage.** Missing list below is the direct-port work plan; kind decides the shim strategy (free → adapter function; method → class edit / wrapper).

## MISSING (10) — the shim stub sheet
| identifier | uses | kind | verbatim donor signature (cited) |
|---|---|---|---|
| `dComIfGp_particle_setToon` | 3 | free | `inline JPABaseEmitter* dComIfGp_particle_setToon(u16 particleID, const cXyz* pos, const csXyz* angle = NULL, const cXyz* scale = NULL, u8 alpha = 0xFF, dPa_levelEcallBack* pCallBack = NULL, s8 setupInfo = -1, const GXColor* pPrmColor = NULL, const GXColor* pEnvColor = NULL, const cXyz* pScale2D = NULL)` — d\d_com_inf_game.h:4058 |
| `dComIfGp_particle_setShipTail` | 2 | free | `inline JPABaseEmitter* dComIfGp_particle_setShipTail(u16 particleID, const cXyz* pos, const csXyz* angle = NULL, const cXyz* scale = NULL, u8 alpha = 0xFF, dPa_levelEcallBack* pCallBack = NULL, s8 setupInfo = -1, const GXColor* pPrmColor = NULL, const GXColor* pEnvColor = NULL, const cXyz* pScale2D = NULL)` — d\d_com_inf_game.h:4085 |
| `dComIfGs_getSelectEquip` | 2 | free (+3 overloads) | `inline u8 dComIfGs_getSelectEquip(int i_no)` — d\d_com_inf_game.h:948 |
| `OnAtHitBit` | 2 | method:cCcD_ObjHitInf | `void OnAtHitBit()` — SSystem\SComponent\c_cc_d.h:547 |
| `ClrAtSet` | 2 | method:cCcD_ObjHitInf | `void ClrAtSet()` — SSystem\SComponent\c_cc_d.h:569 |
| `getGrabMissActor` | 1 | method:daPy_py_c (+1 overloads) | `virtual fopAc_ac_c* getGrabMissActor()` — d\actor\d_a_player.h:620 |
| `onWindOff` | 1 | method:dPa_smokeEcallBack | `void onWindOff()` — d\d_particle.h:87 |
| `setTexNoAnimator` | 1 | method:J3DMaterialTable (+1 overloads) | `s32 setTexNoAnimator(J3DAnmTexPattern*, J3DTexNoAnm*)` — JSystem\J3DGraphAnimator\J3DMaterialAttach.h:32 |
| `kb_dig` | 1 | method:daTagKbItem_c | `void kb_dig(fopAc_ac_c*)` — d\actor\d_a_tag_kb_item.h:10 |
| `setAnmIndex` | 1 | method:J3DMatColorAnm (+4 overloads) | `void setAnmIndex(u16 index)` — JSystem\J3DGraphAnimator\J3DMaterialAnm.h:24 |

## PRESENT (172) — top 40 by use
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
| `GetAttributeCode` | 5 |
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
