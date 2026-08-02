# API surface — `d_a_esa.cpp` vs receiver

**35/40 external identifiers present in receiver = 87.5% coverage.** Missing list below is the direct-port work plan; kind decides the shim strategy (free → adapter function; method → class edit / wrapper).

## MISSING (5) — the shim stub sheet
| identifier | uses | kind | verbatim donor signature (cited) |
|---|---|---|---|
| `daSea_calcWave` | 2 | free | `f32 daSea_calcWave(f32, f32)` — d\actor\d_a_sea.h:125 |
| `ripple_scale` | 2 | UNKNOWN | not found in donor headers — [INFERENCE-NEEDED: locate decl (src-local static? macro?)] |
| `dComIfGp_particle_setShipTail` | 2 | free | `inline JPABaseEmitter* dComIfGp_particle_setShipTail(u16 particleID, const cXyz* pos, const csXyz* angle = NULL, const cXyz* scale = NULL, u8 alpha = 0xFF, dPa_levelEcallBack* pCallBack = NULL, s8 setupInfo = -1, const GXColor* pPrmColor = NULL, const GXColor* pEnvColor = NULL, const cXyz* pScale2D = NULL)` — d\d_com_inf_game.h:4085 |
| `daSea_ChkArea` | 1 | free | `bool daSea_ChkArea(f32, f32)` — d\actor\d_a_sea.h:124 |
| `dBgS_GetWaterHeight` | 1 | free | `f32 dBgS_GetWaterHeight(cXyz&)` — d\d_bg_s_func.h:11 |

## PRESENT (35) — top 40 by use
| identifier | uses |
|---|---|
| `cM_rndF` | 8 |
| `cM_rndFX` | 4 |
| `getEmitter` | 4 |
| `cMtx_YrotS` | 3 |
| `MtxPosition` | 3 |
| `dComIfG_Bgsp` | 2 |
| `setRate` | 2 |
| `fopAcM_delete` | 2 |
| `remove` | 2 |
| `fopAcM_GetParam` | 2 |
| `setLightTevColorType` | 1 |
| `mDoExt_modelUpdateDL` | 1 |
| `SetPos` | 1 |
| `GroundCross` | 1 |
| `LineCross` | 1 |
| `fopAcM_OffStatus` | 1 |
| `dComIfGp_evmng_startCheck` | 1 |
| `MtxTrans` | 1 |
| `cMtx_YrotM` | 1 |
| `cMtx_XrotM` | 1 |
| `cMtx_ZrotM` | 1 |
| `setBaseTRMtx` | 1 |
| `settingTevStruct` | 1 |
| `dComIfG_getObjectRes` | 1 |
| `mDoExt_J3DModel__create` | 1 |
| `daPy_getPlayerActorClass` | 1 |
| `fopAcM_ct` | 1 |
| `fopAcM_entrySolidHeap` | 1 |
| `fopAcM_CreateAppend` | 1 |
| `getLeftHandPos` | 1 |
| `fpcM_Create` | 1 |
| `scale` | 1 |
| `setBaseScale` | 1 |
| `fopAcM_SetMtx` | 1 |
| `getBaseTRMtx` | 1 |
