# API surface — `d_a_cc.cpp` vs receiver

**158/180 external identifiers present in receiver = 87.8% coverage.** Missing list below is the direct-port work plan; kind decides the shim strategy (free → adapter function; method → class edit / wrapper).

## MISSING (22) — the shim stub sheet
| identifier | uses | kind | verbatim donor signature (cited) |
|---|---|---|---|
| `ClrAtSet` | 8 | method:cCcD_ObjHitInf | `void ClrAtSet()` — SSystem\SComponent\c_cc_d.h:569 |
| `dComIfGp_CharTbl` | 5 | free | `inline dADM_CharTbl* dComIfGp_CharTbl()` — d\d_com_inf_game.h:3065 |
| `GetNameIndex` | 5 | method:cDT | `int GetNameIndex(const char* pName, int start) const` — SSystem\SComponent\c_data_tbl.h:45 |
| `setBtNowFrame` | 4 | method:fopEn_enemy_c | `void setBtNowFrame(f32 frame)` — f_op\f_op_actor.h:353 |
| `OnAtHitBit` | 3 | method:cCcD_ObjHitInf | `void OnAtHitBit()` — SSystem\SComponent\c_cc_d.h:547 |
| `fopAcM_setGbaName` | 3 | free | `void fopAcM_setGbaName(fopAc_ac_c* i_this, u8 itemNo, u8 gbaName0, u8 gbaName1)` — f_op\f_op_actor_mng.h:636 |
| `enemy_fire` | 2 | free | `void enemy_fire(enemyfire*)` — c\c_damagereaction.h:197 |
| `dComIfGp_getDetect` | 2 | free | `inline dDetect_c& dComIfGp_getDetect()` — d\d_com_inf_game.h:2409 |
| `chk_light` | 2 | method:dDetect_c (+5 overloads) | `bool chk_light(const cXyz*) const` — d\d_detect.h:32 |
| `enemy_fire_remove` | 2 | free | `void enemy_fire_remove(enemyfire*)` — c\c_damagereaction.h:198 |
| `iceUpdateDL` | 1 | method:dMat_control_c (+1 overloads) | `static void iceUpdateDL(J3DModel* model, s8 param_2, mDoExt_invisibleModel* invisModel)` — d\d_material.h:51 |
| `dComIfGd_setSimpleShadow2` | 1 | free | `int dComIfGd_setSimpleShadow2(cXyz* i_pos, f32 groundY, f32 scaleXZ, cBgS_PolyInfo& i_floorPoly, s16 i_angle = 0, f32 scaleZ = 1.0f, GXTexObj* i_tex = dDlst_shadowControl_c::getSimpleTex())` — d\d_com_inf_game.h:3740 |
| `chk_state` | 1 | method:daBomb_c | `bool chk_state(State_e) const` — d\actor\d_a_bomb.h:159 |
| `chk_explode` | 1 | method:Act_c | `bool chk_explode()` — d\actor\d_a_bomb2.h:88 |
| `dComIfGp_particle_setShipTail` | 1 | free | `inline JPABaseEmitter* dComIfGp_particle_setShipTail(u16 particleID, const cXyz* pos, const csXyz* angle = NULL, const cXyz* scale = NULL, u8 alpha = 0xFF, dPa_levelEcallBack* pCallBack = NULL, s8 setupInfo = -1, const GXColor* pPrmColor = NULL, const GXColor* pEnvColor = NULL, const cXyz* pScale2D = NULL)` — d\d_com_inf_game.h:4085 |
| `checkHammerQuake` | 1 | method:daPy_py_c | `u32 checkHammerQuake() const` — d\actor\d_a_player.h:587 |
| `ClrCoSet` | 1 | method:cCcD_ObjHitInf | `void ClrCoSet()` — SSystem\SComponent\c_cc_d.h:571 |
| `enemy_piyo_set` | 1 | free | `void enemy_piyo_set(fopAc_ac_c*)` — c\c_damagereaction.h:199 |
| `enemy_ice` | 1 | free | `BOOL enemy_ice(enemyice*)` — c\c_damagereaction.h:196 |
| `dComIfG_resDeleteDemo` | 1 | method:daBranch_c | `dComIfG_resDeleteDemo(&mPhase, m_arcname)` — d\actor\d_a_branch.h:18 |
| `initBt` | 1 | method:fopEn_enemy_c | `void initBt(f32 height, f32 radius)` — f_op\f_op_actor.h:333 |
| `setBtAttackData` | 1 | method:fopEn_enemy_c | `void setBtAttackData(f32 startFrame, f32 endFrame, f32 maxDis, u8 attackType)` — f_op\f_op_actor.h:342 |

## PRESENT (158) — top 40 by use
| identifier | uses |
|---|---|
| `fopAcM_seStart` | 47 |
| `getModel` | 32 |
| `getModelData` | 20 |
| `getCutType` | 19 |
| `dComIfG_getObjectRes` | 17 |
| `remove` | 16 |
| `dComIfGp_particle_set` | 15 |
| `setFrame` | 10 |
| `fopAcM_monsSeStart` | 10 |
| `OnTgSetBit` | 10 |
| `isStop` | 10 |
| `fopAcM_createDisappear` | 9 |
| `OffAtSPrmBit` | 8 |
| `SetTgType` | 8 |
| `OnCoSetBit` | 8 |
| `OffCoSetBit` | 8 |
| `getEmitter` | 8 |
| `getAnmMtx` | 7 |
| `fopAcM_delete` | 7 |
| `cM_rndF` | 7 |
| `cLib_addCalc0` | 7 |
| `entry` | 6 |
| `dComIfG_Bgsp` | 6 |
| `fopAcM_onActor` | 6 |
| `becomeInvalidEmitter` | 6 |
| `dComIfGp_getPlayer` | 6 |
| `SetWeight` | 6 |
| `fopAcM_searchPlayerAngleY` | 6 |
| `OffTgSetBit` | 6 |
| `ClrTgHit` | 6 |
| `checkFrame` | 6 |
| `ChkGroundHit` | 6 |
| `cLib_addCalc2` | 6 |
| `init` | 6 |
| `setBaseTRMtx` | 5 |
| `fopAcM_searchPlayerDistance` | 5 |
| `fopAcM_OnStatus` | 5 |
| `fopAcM_checkCarryNow` | 5 |
| `MTXCopy` | 4 |
| `setAnm` | 4 |
