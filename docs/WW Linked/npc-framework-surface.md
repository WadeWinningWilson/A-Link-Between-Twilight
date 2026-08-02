# API surface — `d_npc.cpp`, `d_npc.h` vs receiver

**35/54 external identifiers present in receiver = 64.8% coverage.** Missing list below is the direct-port work plan; kind decides the shim strategy (free → adapter function; method → class edit / wrapper).

## MISSING (19) — the shim stub sheet
| identifier | uses | kind | verbatim donor signature (cited) |
|---|---|---|---|
| `cLib_addCalcAngleL` | 5 | free | `s32 cLib_addCalcAngleL(s32* pValue, s32 target, s32 scale, s32 maxStep, s32 minStep)` — SSystem\SComponent\c_lib.h:24 |
| `setWeightAnmMtx` | 1 | method:J3DModel | `void setWeightAnmMtx(int idx, Mtx mtx)` — JSystem\J3DGraphAnimator\J3DModel.h:78 |
| `dComIfGs_isGetItemReserve` | 1 | free | `inline BOOL dComIfGs_isGetItemReserve(u8 i_no)` — d\d_com_inf_game.h:1168 |
| `dComIfGs_checkReserveItem` | 1 | free | `inline u8 dComIfGs_checkReserveItem(u8 i_itemNo)` — d\d_com_inf_game.h:936 |
| `cutProc` | 1 | method:dNpc_EventCut_c (+6 overloads) | `bool cutProc()` — d\d_npc.h:145 |
| `cutWaitStart` | 1 | method:dNpc_EventCut_c (+1 overloads) | `void cutWaitStart()` — d\d_npc.h:146 |
| `cutWaitProc` | 1 | method:dNpc_EventCut_c (+1 overloads) | `void cutWaitProc()` — d\d_npc.h:147 |
| `cutTurnToActorStart` | 1 | method:dNpc_EventCut_c | `void cutTurnToActorStart()` — d\d_npc.h:148 |
| `cutTurnToActorProc` | 1 | method:dNpc_EventCut_c | `void cutTurnToActorProc()` — d\d_npc.h:149 |
| `cutMoveToActorStart` | 1 | method:dNpc_EventCut_c | `void cutMoveToActorStart()` — d\d_npc.h:150 |
| `cutMoveToActorProc` | 1 | method:dNpc_EventCut_c | `void cutMoveToActorProc()` — d\d_npc.h:151 |
| `cutTurnToPosStart` | 1 | method:dNpc_EventCut_c | `void cutTurnToPosStart()` — d\d_npc.h:152 |
| `cutTurnToPosProc` | 1 | method:dNpc_EventCut_c | `void cutTurnToPosProc()` — d\d_npc.h:153 |
| `cutMoveToPosStart` | 1 | method:dNpc_EventCut_c | `void cutMoveToPosStart()` — d\d_npc.h:154 |
| `cutMoveToPosProc` | 1 | method:dNpc_EventCut_c | `void cutMoveToPosProc()` — d\d_npc.h:155 |
| `cutTalkMsgStart` | 1 | method:dNpc_EventCut_c | `void cutTalkMsgStart()` — d\d_npc.h:156 |
| `cutContinueTalkStart` | 1 | method:dNpc_EventCut_c | `void cutContinueTalkStart()` — d\d_npc.h:157 |
| `cutTalkMsgProc` | 1 | method:dNpc_EventCut_c | `void cutTalkMsgProc()` — d\d_npc.h:158 |
| `findActorCallBack` | 1 | method:dNpc_EventCut_c | `static fopAc_ac_c* findActorCallBack(fopAc_ac_c*, void*)` — d\d_npc.h:172 |

## PRESENT (35) — top 40 by use
| identifier | uses |
|---|---|
| `cLib_addCalcAngleS` | 8 |
| `dComIfG_getObjectRes` | 7 |
| `cM_atan2s` | 5 |
| `cLib_targetAngleY` | 5 |
| `setAnm` | 5 |
| `cM_ssin` | 3 |
| `dPath_ChkClose` | 2 |
| `dComIfG_getObjectIDRes` | 2 |
| `dComIfGs_isEventBit` | 2 |
| `fopMsgM_messageSet` | 2 |
| `cLib_maxLimit` | 2 |
| `cLib_minLimit` | 2 |
| `cLib_distanceAngleS` | 1 |
| `dPath_GetRoomPath` | 1 |
| `dPath_GetNextRoomPath` | 1 |
| `point` | 1 |
| `cM_scos` | 1 |
| `abs2XZ` | 1 |
| `getWEvlpMtxNum` | 1 |
| `getWeightAnmMtx` | 1 |
| `getJointNum` | 1 |
| `setAnmMtx` | 1 |
| `getAnmMtx` | 1 |
| `daPy_getPlayerActorClass` | 1 |
| `getHeadTopPos` | 1 |
| `MtxTrans` | 1 |
| `MtxPosition` | 1 |
| `SetC` | 1 |
| `SetR` | 1 |
| `SetH` | 1 |
| `dComIfG_Ccsp` | 1 |
| `fopMsgM_SearchByID` | 1 |
| `cLib_targetAngleX` | 1 |
| `absXZ` | 1 |
| `cM_deg2s` | 1 |
