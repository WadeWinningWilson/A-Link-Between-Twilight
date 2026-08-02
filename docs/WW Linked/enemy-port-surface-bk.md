# API surface — `d_a_bk.cpp`, `d_a_bk.h` vs receiver

**192/218 external identifiers present in receiver = 88.1% coverage.** Missing list below is the direct-port work plan; kind decides the shim strategy (free → adapter function; method → class edit / wrapper).

## MISSING (26) — the shim stub sheet
| identifier | uses | kind | verbatim donor signature (cited) |
|---|---|---|---|
| `checkGrabWear` | 3 | method:daPy_py_c | `BOOL checkGrabWear() const` — d\actor\d_a_player.h:596 |
| `setBkControl` | 2 | method:Act_c | `void setBkControl(bool control)` — d\actor\d_a_obj_search.h:22 |
| `setBtAttackData` | 2 | method:fopEn_enemy_c | `void setBtAttackData(f32 startFrame, f32 endFrame, f32 maxDis, u8 attackType)` — f_op\f_op_actor.h:342 |
| `setBtNowFrame` | 2 | method:fopEn_enemy_c | `void setBtNowFrame(f32 frame)` — f_op\f_op_actor.h:353 |
| `enemy_fire_remove` | 2 | free | `void enemy_fire_remove(enemyfire*)` — c\c_damagereaction.h:198 |
| `JntHit_create` | 2 | free | `JntHit_c* JntHit_create(J3DModel* model, __jnt_hit_data_c* jntHitData, s16 hitDataCount)` — d\d_jnt_hit.h:111 |
| `setRotAngleSpeed` | 1 | method:daBoko_c | `void setRotAngleSpeed(s16 speed)` — d\actor\d_a_boko.h:92 |
| `moveStateInit` | 1 | method:daBoko_c | `void moveStateInit(f32 speedForward, f32 speedY, s16 angleY)` — d\actor\d_a_boko.h:81 |
| `dComIfGp_particle_setToon` | 1 | free | `inline JPABaseEmitter* dComIfGp_particle_setToon(u16 particleID, const cXyz* pos, const csXyz* angle = NULL, const cXyz* scale = NULL, u8 alpha = 0xFF, dPa_levelEcallBack* pCallBack = NULL, s8 setupInfo = -1, const GXColor* pPrmColor = NULL, const GXColor* pEnvColor = NULL, const cXyz* pScale2D = NULL)` — d\d_com_inf_game.h:4058 |
| `iceEntryDL` | 1 | method:dMat_control_c (+1 overloads) | `static void iceEntryDL(J3DModel* model, s8 param_2, mDoExt_invisibleModel* invisModel)` — d\d_material.h:45 |
| `dComIfGd_setListMaskOff` | 1 | free | `inline void dComIfGd_setListMaskOff()` — d\d_com_inf_game.h:3809 |
| `MtxRotY` | 1 | free | `void MtxRotY(f32 rot, u8 concat)` — SSystem\SComponent\c_lib.h:99 |
| `setChildId` | 1 | method:Act_c | `void setChildId(fpc_ProcID id)` — d\actor\d_a_obj_search.h:23 |
| `setBtMaxDis` | 1 | method:fopEn_enemy_c (+1 overloads) | `setBtMaxDis(maxDis)` — f_op\f_op_actor.h:345 |
| `enemy_piyo_set` | 1 | free | `void enemy_piyo_set(fopAc_ac_c*)` — c\c_damagereaction.h:199 |
| `setVolumeSweep` | 1 | method:JPABaseEmitter | `void setVolumeSweep(f32 i_volSweep)` — JSystem\JParticle\JPAEmitter.h:268 |
| `enemy_ice` | 1 | free | `BOOL enemy_ice(enemyice*)` — c\c_damagereaction.h:196 |
| `CPad_CHECK_TRIG_B` | 1 | UNKNOWN | not found in donor headers — [INFERENCE-NEEDED: locate decl (src-local static? macro?)] |
| `CPad_CHECK_HOLD_Y` | 1 | UNKNOWN | not found in donor headers — [INFERENCE-NEEDED: locate decl (src-local static? macro?)] |
| `damage_reaction` | 1 | free | `int damage_reaction(damagereaction*)` — c\c_damagereaction.h:208 |
| `enemy_fire` | 1 | free | `void enemy_fire(enemyfire*)` — c\c_damagereaction.h:197 |
| `dComIfG_resDeleteDemo` | 1 | method:daBranch_c | `dComIfG_resDeleteDemo(&mPhase, m_arcname)` — d\actor\d_a_branch.h:18 |
| `dComIfGp_CharTbl` | 1 | free | `inline dADM_CharTbl* dComIfGp_CharTbl()` — d\d_com_inf_game.h:3065 |
| `GetNameIndex` | 1 | method:cDT | `int GetNameIndex(const char* pName, int start) const` — SSystem\SComponent\c_data_tbl.h:45 |
| `fopAcM_adjustHeap` | 1 | free | `void fopAcM_adjustHeap(fopAc_ac_c* i_this)` — f_op\f_op_actor_mng.h:710 |
| `initBt` | 1 | method:fopEn_enemy_c | `void initBt(f32 height, f32 radius)` — f_op\f_op_actor.h:333 |

## PRESENT (192) — top 40 by use
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
