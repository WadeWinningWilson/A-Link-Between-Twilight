#ifndef D_EXT_WW_ACTOR_SHIMS_H
#define D_EXT_WW_ACTOR_SHIMS_H

// ============================================================
// §223 Pass 1 — direct-port shims for the WW enemy/NPC ports (P13c doctrine).
// The api_surface pass proved the missing donor identifiers are truly absent.
// This file supplies the ones the PIG (d_a_kb) needs so the Pass-2 source
// drop-in links. Split by kind:
//   · FREE FUNCTIONS keep the DONOR name (defined here) → source stays verbatim.
//   · CLASS-METHOD gaps use `dExtKb_` ADAPTER free-functions → the Pass-2 source
//     adapts `x->method(...)` to `dExtKb_method(x, ...)` (keeps libs/JSystem
//     untouched — lower risk than editing engine classes).
// Cosmetic gaps (particle FX, figurine gallery) are no-op first pass; the pig
// loop does not depend on them. Bokoblin's 31 (combat tier) land with its own
// port.
// ============================================================

#include "SSystem/SComponent/c_xyz.h"
#include "dolphin/types.h"
#include "f_op/f_op_actor.h"  // §225 fopAc_ac_c full type (daTagKbItem_c stub derives from it)

class J3DModelData;
class J3DMaterialTable;

// --- esa (bait) shims: sea/water not ported → bait always lands on GROUND -----
// (donor esa water-landing = mState 2, which the pig ignores — it only accepts
//  mState 1 ground bait, so stubbing water is faithful in effect).
bool daSea_ChkArea(f32 x, f32 z);       // in a sea area? → false (no sea system)
f32 daSea_calcWave(f32 x, f32 z);       // wave height → very low (no wave)
f32 dBgS_GetWaterHeight(cXyz& pos);     // water y → -inf (no water under bait)

// --- FREE-FUNCTION shims (donor names; verbatim source use) ----------------
// Ground-normal angle under an actor. First pass: flat (no slope tilt) — the
// pig visibly walks level; slope-lean is a Pass-2 refinement.
BOOL fopAcM_getGroundAngle(fopAc_ac_c* actor, csXyz* p_angle);
// Nintendo-Gallery figurine registration — not restored → no-op.
void dSnap_RegistFig(u8 type, fopAc_ac_c* actor, f32 sx, f32 sy, f32 sz);

// --- CLASS-METHOD adapters (dExtKb_ prefix; call-site adapted in Pass 2) ----
// J3DModelData::setMaterialTable — pig size/skin bmt swap.
void dExtKb_setMaterialTable(J3DModelData* md, J3DMaterialTable* bmt);
// dBgS::GetAttributeCode — ground material code under the pig. §227 the port's
// dBgS has no per-material code; returns a constant (matches no dBgS_Attr_*
// sentinel) so the pig's dig-material switch is inert (dead-but-faithful).
class cBgS_PolyInfo;
s32 dExtKb_GetAttributeCode(const cBgS_PolyInfo& gnd);
// cc attack-collision bit set/clear (only when the pig is hittable). No-op FP.
// §227 dCcD_Sph::OnAtHitBit is absent in the port -> adapter takes the sphere.
class dCcD_Sph;
void dExtKb_OnAtHitBit(dCcD_Sph* sph);
void dExtKb_ClrAtSet(fopAc_ac_c* actor);
// player grab/throw target — carry mechanic. Returns NULL (no grab) first pass.
fopAc_ac_c* dExtKb_getGrabMissActor();

// ============================================================
// §225 Pass 2 — d_a_kb (PIG) direct-port shims. The pig source is dropped in
// verbatim; these supply the donor identifiers the port lacks. All are no-op or
// sentinel: the pig's LOGIC (search/walk/eat/dig state machine) is faithful; the
// gaps are cosmetic FX (deferred to the wave-emit pass) or a secondary food
// source not yet placed.
// ============================================================
class JPABaseEmitter;

// --- WW isle-room ids (donor d_save.h). The pig's big-pig lottery gates on the
// WW "sea" stage + these rooms; in the port getStartStageName()!="sea" so the
// branch is dead, but it must still COMPILE. Values verbatim from donor.
enum {
    dIsleRoom_ForsakenFortress_e = 1,   // donor d_save.h:307 (§888 layer port)
    dIsleRoom_WindfallIsland_e = 11,
    dIsleRoom_OutsetIsland_e   = 44,
};

// --- §877: the §225 pig-tag shims (fpcNm_TAG_KB_ITEM_e 0xFFFE sentinel +
// daTagKbItem_c stub) are RETIRED — the real TAG_KB_ITEM tag is ported
// (d_a_tag_kb_item.cpp, enum row 0x33B); the pig's search_get_item now finds
// live tags and gains its dug-item food source, the intended donor behavior.

// --- particle FX no-ops (deferred to the wave-emit pass) --------------------
// The pig's water-ripple (setShipTail/HAMON00) and toon dust/smoke
// (setToon/ELEMENTSMOKE·PIGDIGSMOKE) have no port emitter yet. These stand-ins
// preserve the struct member layout + the exact method surface the pig calls
// (getEmitter()==NULL kills every config chain), and the set* shims no-op.
class dPa_rippleEcallBack {
public:
    dPa_rippleEcallBack() { mpBaseEmitter = NULL; mPos = NULL; mFlags = 0; mRate = 0.0f; }
    JPABaseEmitter* getEmitter() { return NULL; }
    void end() {}
    void remove() {}
    void setRate(f32 rate) { mRate = rate; }
    /* 0x04 */ JPABaseEmitter* mpBaseEmitter;
    /* 0x08 */ const cXyz* mPos;
    /* 0x0C */ u32 mFlags;
    /* 0x10 */ f32 mRate;
};

class dPa_smokeEcallBack {
public:
    dPa_smokeEcallBack() { mpBaseEmitter = NULL; }
    JPABaseEmitter* getEmitter() { return NULL; }
    void end() {}
    void remove() {}
    void onWindOff() {}
    void offWindOff() {}
    /* 0x04 */ JPABaseEmitter* mpBaseEmitter;
    /* 0x08 */ u8 field_pad[0x20 - 0x08];
};

// Donor particle emitters the port lacks — no-op (return NULL emitter). Signatures
// mirror the donor call sites exactly (ripple 6-arg, toon 7-arg incl. roomNo).
JPABaseEmitter* dComIfGp_particle_setShipTail(u16 id, const cXyz* pos, const csXyz* angle,
                                              const cXyz* scale, u8 alpha, dPa_rippleEcallBack* cb);
JPABaseEmitter* dComIfGp_particle_setToon(u16 id, const cXyz* pos, const csXyz* angle,
                                          const cXyz* scale, u8 alpha, dPa_smokeEcallBack* cb, int roomNo);

// ============================================================
// §226 Pass 2 — d_a_kb (PIG) donor identifiers the port lacks (compile-only).
// Values are verbatim from the WW donor so the logic is byte-faithful; every
// gap here is cosmetic FX, deferred combat, or a WW-only stage branch that is
// DEAD in the port (getStartStageName() != "sea") yet must still compile.
// ============================================================
class J3DAnmTexPattern;
class J3DTexNoAnm;
class cCcD_Obj;

// --- misc donor constants (donor names kept; source stays verbatim) ---------
// Env-light TEV struct type. WW TEV_TYPE_ACTOR == 0 (d_kankyo.h); the port's own
// McaMorf actors (e.g. d_a_L7demo_dr) pass 0 for an actor draw. Same value.
#define TEV_TYPE_ACTOR 0
// Nintendo-Gallery figurine type id (WW d_snap.h). dSnap_RegistFig is a no-op,
// so the value is inert — kept at the donor value for faithfulness.
#define DSNAP_TYPE_KB 0x54
// Player "crawl/hold" status bit. WW daPyStts0_CRAWL_e == 0x08000000, identical
// to the raw bit the port's alink checks via dComIfGp_checkPlayerStatus0.
#define daPyStts0_CRAWL_e 0x08000000
// J3D node-callback "calc-in" timing. Port idiom (d_a_npc_tk JointCallBack)
// checks `== 0`; WW's named constant is 0.
#define J3DNodeCBCalcTiming_In 0

// --- pig SFX (deferred audio) -----------------------------------------------
// The WW pig sound bank is not loaded; passing 0 is a silent no-op through
// fopAcM_seStart/fopAcM_monsSeStart. A later audio pass wires real Z2SE ids
// (note: OBJ_FALL_WATER_S / OBJ_ITEM_OUT / OBJ_LUPY_OUT have Z2 equivalents).
#define JA_SE_CV_PG_CATCH      0
#define JA_SE_CV_PG_L_CATCH    0
#define JA_SE_CV_PG_NORMAL     0
#define JA_SE_CV_PG_L_NORMAL   0
#define JA_SE_CV_PG_TURN       0
#define JA_SE_CV_PG_L_TURN     0
#define JA_SE_CV_PG_CARRY      0
#define JA_SE_CM_PG_L_JUMP     0
#define JA_SE_CM_PG_DIG        0
#define JA_SE_CM_PG_L_DIG      0
#define JA_SE_CM_PG_DIG_LANDING 0
#define JA_SE_OBJ_FALL_WATER_S 0
#define JA_SE_OBJ_ITEM_OUT     0
#define JA_SE_OBJ_LUPY_OUT     0

// --- collision attack-check (deferred combat) -------------------------------
// The port renamed CcAtInfo -> dCcU_AtInfo with a different layout (no
// pParticlePos). Rather than feed the port's real damage system a partly-init'd
// struct, the pig keeps the donor CcAtInfo verbatim and cc_at_check no-ops:
// the pig registers hits but takes no damage first pass (Pass-1 posture).
struct CcAtInfo {
    /* 0x00 */ cCcD_Obj* mpObj;       // set from mSph.GetTgHitObj()
    /* 0x04 */ cXyz*     pParticlePos;// hit-fx position (unused, no-op)
};
void cc_at_check(fopAc_ac_c* actor, CcAtInfo* atInfo);

// --- equipped-item query (deferred combat) ----------------------------------
// WW's C-button equip slot query. The port has no slot-N equivalent; returning
// dItemNo_NONE_e keeps both pig branches inert (no damage taken first pass).
int dComIfGs_getSelectEquip(int slot);

// --- J3D tex/material methods absent on the port's J3D layer -----------------
// Eye-blink tex-pattern anim + size/skin material swap. No-op first pass
// (cosmetic); the pig renders with a static eye texture + base material.
void dExtKb_setTexNoAnimator(J3DModelData* md, J3DAnmTexPattern* pat, J3DTexNoAnm* txm);
void dExtKb_setAnmTexPattern(J3DTexNoAnm* txm, J3DAnmTexPattern* pat);

// ============================================================
// §227 Pass 2 (round 3) — d_a_kb donor identifiers the port renamed away or
// dropped. Collision-source enums: the port kept WW's exact bit VALUES (its own
// actors use 0x10/0x70 group masks == WW cCcD_CoSPrm_VsEnemy/VsGrpAll) and the
// dCcD_SrcSph struct layout is byte-identical to WW's — it only deleted the
// named constants. Re-supplying the WW values lets the pig's co_sph_src static
// initializer + SetCoVsGrp calls compile byte-faithfully. Other #defines cover
// WW-only stage-branch enums that are dead in the port but must compile.
// ============================================================

// --- collision source: WW cCcD/dCcG values (verbatim from WW donor headers) --
#define AT_TYPE_UNK800          (1 << 11)   // 0x800
#define AT_TYPE_ALL             (~0)
#define AT_TYPE_WATER           (1 << 8)    // 0x100
#define AT_TYPE_UNK20000        (1 << 17)   // 0x20000
#define AT_TYPE_WIND            (1 << 21)   // 0x200000
#define AT_TYPE_UNK400000       (1 << 22)   // 0x400000
#define AT_TYPE_LIGHT           (1 << 23)   // 0x800000
#define cCcD_AtSPrm_Set_e       0x01
#define cCcD_AtSPrm_VsPlayer_e  0x04
#define cCcD_AtSPrm_VsOther_e   0x08
#define cCcD_TgSPrm_Set_e       0x01
#define cCcD_TgSPrm_IsEnemy_e   0x02
#define cCcD_CoSPrm_Set_e       0x01
#define cCcD_CoSPrm_IsOther_e   0x08
#define cCcD_CoSPrm_VsEnemy_e   0x10   // port actors pass 0x10 to SetCoVsGrp
#define cCcD_CoSPrm_VsGrpAll_e  0x70   // port actors pass 0x70 to SetCoVsGrp
#define dCcG_SE_UNK6            6
#define dCcG_AtHitMark_None_e   0      // WW dPa_name::ID_NONE == 0
#define dCcG_At_Spl_UNK6        6
#define dCcG_Tg_Spl_UNK0        0

// --- ground-attribute sentinels (port lacks per-material codes; distinct so
//     the dig-material switch parses; dExtKb_GetAttributeCode returns none) ---
#define dBgS_Attr_DIRT_e   1
#define dBgS_Attr_GRASS_e  2
#define dBgS_Attr_SAND_e   3
#define dBgS_Attr_WATER_e  4

// --- WW-only event/proc/draw-prio identifiers (dead branch / not restored) ---
#define dEvtType_OTHER_e 0x02          // WW d_event.h; passed as u16 event flag
#define fpcNm_KS_e 0xFFFD              // WW splash-drop proc; not restored (sentinel)
#define fpcDwPi_KB_e fpcDwPi_E_RD_e    // pig draw priority -> a port ground-enemy slot

// --- item drop: port fastCreateItem takes pointer speeds + no gravity ------
fopAc_ac_c* dExtKb_fastCreateItem(const cXyz* pos, int itemNo, int roomNo,
                                  const csXyz* angle, const cXyz* scale,
                                  f32 speedF, f32 speedY, f32 gravity);
// J3DTexNoAnm::setAnmIndex absent in port; no-op (tex-anm path is already no-op'd).
void dExtKb_setAnmIndex(J3DTexNoAnm* txm, u16 idx);

// ============================================================
// §232 Pass 2 — d_a_kamome (SEAGULL) direct-port donor identifiers the port lacks.
// Same doctrine as the pig: FREE functions keep the donor name; missing enum/SFX
// constants are re-supplied with their VERBATIM WW values so the logic stays
// byte-faithful. Every gap here is either a dead WW-only branch (ship-following
// flock — no KoRL boat in the port), deferred audio, or a cosmetic figurine.
// ============================================================

// --- FREE-FUNCTION shim (donor name; source stays verbatim) ------------------
// Whether an enemy alert sound is currently playing. The seagull uses it only in
// the ship-following (heisou) path, which is dead in the port (no ship ride);
// returning false = "no enemy" keeps the donor logic inert-but-faithful.
bool dComIfGp_att_chkEnemySound();

// --- player status bit (WW d_com_inf_game.h; port lacks daPyStts0_*) ---------
// "Riding the ship (KoRL)". No boat in the port, so every check is false and the
// heisou/imouto ship branches never fire — value verbatim so it still compiles.
#define daPyStts0_SHIP_RIDE_e 0x00010000

// --- collision source: WW cCcD/dCcG values the pig block didn't already cover --
// (verbatim from WW SSystem/SComponent/c_cc_d.h + d/d_cc_d.h). The port kept the
// bit VALUES + struct layout and only deleted the named constants (see §227).
#define cCcD_AtSPrm_VsEnemy_e   0x02
#define cCcD_CoSPrm_IsPlayer_e  0x04
#define dCcG_At_Spl_UNK0        0

// --- deferred audio: the WW seagull SE bank isn't loaded; 0 is a silent no-op
//     through fopAcM_seStart (WW value was JA_SE_CV_KAMOME=0x4805). ------------
#define JA_SE_CV_KAMOME 0

// --- Nintendo-Gallery figurine type (WW d_snap.h == 0x55). dSnap_RegistFig is a
//     no-op, so the value is inert; kept at the donor value for faithfulness. --
#define DSNAP_TYPE_KAMOME 0x55

// --- §244 Aryll's-sister NPC: fpcNm_NPC_LS1_e is now a REAL registered actor
//     (0x327, see f_pc_name.h). The old sentinel #define is gone; kamome's
//     search_imouto() now finds the real Aryll (the intended WW behavior). -----

// --- draw priority: the donor's fpcDwPi_KAMOME_e slot is absent. Reuse an
//     existing port ground/ambient enemy draw-prio (same choice the pig made). -
#define fpcDwPi_KAMOME_e fpcDwPi_E_RD_e

// ============================================================
// §239 Pass 1 — WW NPC FRAMEWORK (fopNpc_npc_c + dNpc_* helpers) donor gaps.
// SHARED INFRASTRUCTURE port (d_npc.cpp / d_npc.h / d_npc_event_cut.inc drop-in).
// Same doctrine as the pig/seagull: FREE functions keep the DONOR name (source
// stays verbatim); a class-method gap the port's J3D layer lacks becomes a
// dExtNpc_ ADAPTER; renamed-away enum/item constants are re-supplied with their
// VERBATIM WW values so the framework logic is byte-faithful.
// ============================================================
#include "dolphin/mtx.h"  // §239 MtxP for the setWeightAnmMtx adapter
class J3DModel;

// --- FREE-FUNCTION shims (donor names; d_npc source stays verbatim) ----------
// s32/long analog of the port's cLib_addCalcAngleS. Body is VERBATIM from the WW
// donor (SSystem/SComponent/c_lib.cpp cLib_addCalcAngleL__FPlllll) — the s32
// twin of the s16 easing the port already ships. Used by dNpc_JntCtrl_c turn math.
s32 cLib_addCalcAngleL(s32* pValue, s32 target, s32 scale, s32 maxStep, s32 minStep);

// Item-reserve queries (WW d_com_inf_game.h). The port has no reserved-item
// system, so both return the "nothing reserved" default: isGetItemReserve → FALSE
// (kills the dNpc_chkLetterPassed branch → letter never counted passed) and
// checkReserveItem → 0. Faithful-in-effect (no reserved item present).
BOOL dComIfGs_isGetItemReserve(u8 i_no);
u8 dComIfGs_checkReserveItem(u8 i_itemNo);

// --- CLASS-METHOD adapter (call-site adapted; libs/JSystem untouched) --------
// J3DModel::setWeightAnmMtx is absent on the port (only getWeightAnmMtx exists).
// The adapter mirrors the port's own J3DMtxBuffer::setAnmMtx idiom (MTXCopy into
// the weight-envelope matrix the getter returns). Used by dNpc_setShadowModel.
void dExtNpc_setWeightAnmMtx(J3DModel* self, int idx, MtxP mtx);

// --- collision source: WW dCcG value not covered by §227/§232 ----------------
// dNpc_cyl_src's SrcGObjTg SPrm. VERBATIM WW value (d/d_cc_d.h: 1 << 2); the port
// kept the same bit (OnTgNoHitMark uses OnSPrm(dCcG_TgSPrm_NoHitMark_e)==0x04).
#define dCcG_TgSPrm_NoHitMark_e 0x04

// --- item id the port lacks (WW d_item_data.h) -------------------------------
// Grandma's / father's letter. Only referenced inside dNpc_chkLetterPassed, whose
// reserve branch is dead (isGetItemReserve → FALSE), so the value is inert; a
// distinct sentinel keeps the source verbatim and collides with no port item.
#define dItemNo_FATHER_LETTER_e 0xFE

// ============================================================
// §246 Pass — d_a_npc_bm1 (Generic Ritos NPC_BM1..BM5) direct-port donor gaps.
// Same doctrine as pig/seagull/Aryll: FREE fns keep donor names (source stays
// verbatim); missing WW enum/constant/particle IDs are re-supplied as
// faithful-in-effect shims with their VERBATIM WW values. Every gap here is a
// WW subsystem the port lacks (save event flags, symbols, beast counter, letter
// send, figurine gallery, Rito feather particle FX). These WW Ritos do not spawn
// in the port, so the gated branches are RUNTIME-DEAD; the shims exist only so
// the donor state machine COMPILES byte-faithfully.
// ============================================================
class mDoExt_McaMorf;
class dDemo_actor_c;
struct msg_class;

// --- message-state accessors. The port's msg_class is a minimal stub (no WW
//     mStatus/mSelectNum message-poll fields); the port drives NPC dialogue
//     through a different flow. bm1's talk state machine polls these, but these
//     Ritos never talk in the port (runtime-dead), so the shims return safe inert
//     values: status = "still typing" (talk_1's no-op case) and selection 0. ---
u16 dExtNpcBm1_msgStatus(msg_class* m);
s16 dExtNpcBm1_msgSelectNum(msg_class* m);

// --- mDoExt_McaMorf::stopZelAnime is absent on the port's McaMorf (only the SO/2
//     variants expose it). Called on actor delete to stop the sound animation;
//     no-op adapter (the port's McaMorf sound lifecycle differs). ---
void dExtNpcBm1_stopZelAnime(mDoExt_McaMorf* m);

// §251 dExtNpcBm1_getDemoBtp RETIRED — promoted to native dDemo_actor_c::getP_BtpData
//      (Foundry decode, d_demo.cpp §251). bm1/Aryll call the native method directly.

// --- soft-dep name sentinel: GP1 is referenced ONLY inside a
//     fopAcM_GetName(x) == fpcNm_NPC_GP1_e check (no type use, no include). GP1 is
//     not ported -> sentinel that never matches. §254 ZL1 sentinel REMOVED — Tetra
//     is now a real registered actor (0x329), so bm1's ZL1 name-check finds the
//     real Tetra (intended), same as the §244 LS1 promotion. ---
#define fpcNm_NPC_GP1_e 0xFFFA

// --- draw priority: the donor's fpcDwPi_NPC_BM1_e..BM5_e slots are absent. Reuse
//     an existing port ground/ambient NPC draw-prio (same choice the pig/seagull
//     made), keeping the profile Draw-Prio field verbatim from the donor. ---
#define fpcDwPi_NPC_BM1_e fpcDwPi_E_RD_e
#define fpcDwPi_NPC_BM2_e fpcDwPi_E_RD_e
#define fpcDwPi_NPC_BM3_e fpcDwPi_E_RD_e
#define fpcDwPi_NPC_BM4_e fpcDwPi_E_RD_e
#define fpcDwPi_NPC_BM5_e fpcDwPi_E_RD_e

// --- WW save event flags (dSv_event_flag_c UNK_*/ENDLESS_NIGHT). The port's
//     dSv_event_flag_c is TP-named; these WW story bits do not exist. bm1 passes
//     them to dComIfGs_isEventBit/onEventBit as raw u16 (value == WW byte<<8|bit
//     encoding, verbatim from the WW donor d_save.h). Byte offsets are <= 0x2E, in
//     bounds of the port event-flag array; runtime-dead (no WW Rito spawns). ---
#define WWEV_UNK_0001 0x0001
#define WWEV_UNK_1102 0x1102
#define WWEV_UNK_1220 0x1220
#define WWEV_UNK_1401 0x1401
#define WWEV_UNK_1620 0x1620
#define WWEV_UNK_1808 0x1808
#define WWEV_UNK_1820 0x1820
#define WWEV_UNK_1908 0x1908
#define WWEV_UNK_1A80 0x1A80
#define WWEV_UNK_1E80 0x1E80
#define WWEV_UNK_1F40 0x1F40
#define WWEV_UNK_2001 0x2001
#define WWEV_UNK_2120 0x2120
#define WWEV_UNK_2140 0x2140
#define WWEV_UNK_2180 0x2180
#define WWEV_UNK_2202 0x2202
#define WWEV_UNK_2E04 0x2E04
#define WWEV_ENDLESS_NIGHT 0x0A02

// --- WW din-symbol query + beast (golden-feather) counter. Not ported;
//     dComIfGs_isSymbol -> FALSE and getBeastNum -> 0 keep the spawn/msg
//     branches inert-but-faithful. Values are the donor's own dSymbolIndex_e
//     (WW DP d_com_inf_game.h:1682-1686) VERBATIM — the old single define put
//     DIN at 0, which is NAYRU's slot; inert behind the FALSE-stub today, but
//     the moment isSymbol becomes real every DIN query would have read NAYRU
//     (queue row 155 / tale §892). ---
#define dSymbol_NAYRU_e 0
#define dSymbol_DIN_e 1
#define dSymbol_FARORE_e 2
#define dBeastIdx_GOLDEN_FEATHER_e 0

// --- WW actor status + attention-set flags + day/night enum the port renamed
//     away or lacks. The port renamed fopAcStts_* by value (fopAcStts_UNK_0xNNNN_e);
//     these WW names map to the SAME bit values (verbatim from WW f_op_actor.h), so
//     the profile Status field + OnStatus/OffStatus toggles are byte-faithful. The
//     port's attention flags use the fopAc_AttnFlag_* family; the WW init flags
//     fopAc_Attn_{LOCKON_TALK,ACTION_SPEAK}_e are re-supplied at their WW values.
//     dKy_daynight_check() returns BOOL in the port (0=day/1=night), matching the
//     WW dKy_TIME_{DAY,NIGHT}_e enum values. ---
#ifndef fopAcStts_UNK4000_e
#define fopAcStts_UNK4000_e   0x00004000
#endif
#define fopAcStts_SHOWMAP_e    0x00000020
#define fopAcStts_NOCULLEXEC_e 0x00000080
#define fopAcStts_UNK40000_e   0x00040000
#define fopAc_Attn_LOCKON_TALK_e  0x00000002
#define fopAc_Attn_ACTION_SPEAK_e 0x00000008
#define dKy_TIME_DAY_e   0
#define dKy_TIME_NIGHT_e 1

// --- Nintendo-Gallery figurine type ids (WW d_snap.h). dSnap_RegistFig is a
//     no-op, so the values are inert; kept at the WW values for faithfulness. ---
#define DSNAP_TYPE_UNK8D 0x8D
#define DSNAP_TYPE_UNK8E 0x8E
#define DSNAP_TYPE_UNK8F 0x8F
#define DSNAP_TYPE_UNK90 0x90
#define DSNAP_TYPE_UNK91 0x91
#define DSNAP_TYPE_UNK92 0x92
#define DSNAP_TYPE_UNK93 0x93
#define DSNAP_TYPE_UNK97 0x97

// --- WW Rito feather particle IDs (dPa_name::ID_IT_SN_BM_HANEL_*/TORIZOKU_HANE*).
//     Not in the port. Emitted only via the no-op dExtNpcBm1_particleSet adapters
//     below, which ignore the id -> value is inert (deferred cosmetic feather FX). ---
#define BM1_PA_HANEL_FLYAWAY00 0
#define BM1_PA_HANEL_LAND00    0
#define BM1_PA_TORIZOKU_HANE00 0
#define BM1_PA_TORIZOKU_HANE01 0

// --- FREE-FUNCTION shims (donor names; bm1 source stays verbatim) ------------
// WW din-symbol query -> FALSE (no symbol system).
BOOL dComIfGs_isSymbol(u8 i_no);
// WW beast/quiver counter -> 0 (golden-feather count 0; the >=20 branch is dead).
u8 dComIfGs_getBeastNum(int i_beastIdx);
// WW beast-count adjust -> no-op (nothing to spend).
void dComIfGp_setItemBeastNumCount(int i_beastIdx, s16 num);
// WW message anime-tag query -> 0xFF (no tag; anmAtr tag branch inert).
u8 dComIfGp_getMesgAnimeTagInfo();
// WW status-map clear -> no-op (postman-0 status-map not restored).
void fopAcM_ClearStatusMap(fopAc_ac_c* actor);
// WW letter-send -> no-op (dLetter subsystem not ported; event_proc case 2 dead).
void dLetter_send(u16 i_no);
// WW demo-actor lookup -> NULL (no Rito demo cutscenes; demo() else-branch dead).
dDemo_actor_c* dComIfGp_demo_getActor(u8 id);

// --- material-table entryDL adapter. The port's mDoExt_McaMorf::entryDL() takes
//     no argument (the WW updateDL(J3DMaterialTable*) recolor split away). The
//     recolor bmt (bm02/bmarm02) is DEFERRED like the pig's BMT swap; the adapter
//     ignores the table and submits the base DL (model stays visible). ---
void dExtNpcBm1_entryDL_mat(mDoExt_McaMorf* morf, J3DMaterialTable* bmt);

// --- WW Rito feather particle emit -> no-op (returns NULL emitter; every call
//     site null-checks the result). Deferred cosmetic FX (no WW feather bank in
//     the port). Two arities match the donor call sites (9-arg flyaway/land with
//     K0 colors; 7-arg hane). Pointer params are void* so GXColor*/cXyz*/csXyz*
//     all convert; overloads disambiguate by arg count. ---
JPABaseEmitter* dExtNpcBm1_particleSet(u16 id, const void* pos, const void* rot,
                                       const void* scale, int alpha, const void* cb,
                                       s8 room, const void* prmColor, const void* envColor);
JPABaseEmitter* dExtNpcBm1_particleSet(u16 id, const void* pos, const void* rot,
                                       const void* scale, int alpha, const void* cb, s8 room);

// ============================================================================
// §244  Aryll (d_a_npc_ls1) shims — the remaining WW symbols the port lacks.
//   All are cheap constants or inert-faithful stubs on Aryll's telescope-demo
//   path (dormant until Link's telescope ITEM lands) or her talk path.
// ============================================================================

// --- attention TYPE ids (WW f_op_actor.h; port lacks these two) --------------
#define fopAc_Attn_TYPE_TALK_e   0x1
#define fopAc_Attn_TYPE_SPEAK_e  0x3

// --- WW save event-flag ids (byte<<8|bit) Aryll reads/writes; add the ones bm1
//     didn't. Raw WW numbering — value-faithful, TP-label semantic caveat (as
//     §243). All on Aryll's story-gated dialogue/demo branches. ---------------
#define WWEV_UNK_0280  0x0280
#define WWEV_UNK_2A80  0x2A80
#define WWEV_UNK_0310  0x0310

// --- item button slots + telescope item no (WW d_com_inf_game.h/d_item_data.h).
//     The telescope never enters the port inventory → getSelectItem()==TELESCOPE
//     is always false (inert-faithful). --------------------------------------
#define dItemBtn_X_e        0
#define dItemBtn_Y_e        1
#define dItemBtn_Z_e        2
#define dItemNo_TELESCOPE_e 0x20

// --- Nintendo-Gallery figurine type (WW d_snap.h idx 0x49; RegistFig no-op) ---
#define DSNAP_TYPE_NPC_LS1  0x49

// --- Tetra-fly demo BGM stream (WW JA_STRM_*; bank not loaded → 0 = silent) ---
#define JA_STRM_DEMO_TETRA_FLY 0

// --- player scope-cancel no-reset flag. WW value 0x00080000 COLLIDES with the
//     port's FLG0_FAST_SWORD_CUT, so shim to 0: onNoResetFlg0(0) is a no-op and
//     the scope-cancel bit has no port consumer (telescope-dormant path). -----
#define daPyFlg0_SCOPE_CANCEL ((daPy_py_c::daPy_FLG0)0)

// --- message-anime tag setter. WAS an empty inline no-op; the donor's real
//     one-slot mailbox is now ported in d_ext_ww_actor_shims.cpp (0xFF sentinel,
//     consumer-clears protocol, module-static per the offset-stable law).
//     DECLARED here, DEFINED there — an inline {} would silently win over the
//     real definition at every include site that saw this header first. --------
void dComIfGp_setMesgAnimeTagInfo(u8 i_id);

// --- WW BMG lookup BY mMsgNo — the donor's own linear scan (skips mDataOffs==0,
//     Hylian variant via mTextboxType==12 + clearCount). NULL if not resident or
//     no such message. NOT the same space as §308's index-based
//     dExtDmesg_getMessageById: STB codes are INF1 INDICES, actor message ids
//     are mMsgNos. Public so the §324 talk path can be corrected without a
//     second copy of this scan being written elsewhere.
const char* dExtWwMsg_textByMsgNo(u16 i_msgNo);

// --- WW password render — donor fopMsgM_passwordGet (8002BE04). Message to
//     plain text: player-name tag substituted, all other tags stripped.
void dExtWwMsg_passwordGet(char* o_buf, u32 i_msgNo);

// --- Clear the anime-tag mailbox to the donor's 0xFF empty sentinel.
//     WIRED 2026-08-16 at ww_stage_loader.cpp:467 (dExtWwStage_loadStageDzs,
//     inside DUSK_WW_STAGE_SEAM), beside the §773 model-cache eviction — same
//     lifecycle, same reason: stage-scoped state dies when a new stage's
//     resources are installed. The donor keeps this tag in game-info @0x493C
//     which resets on its own; a module-static does not.
void dExtWwShims_resetMesgAnimeTag();

// --- WW "collect" (figurine/collectible) query. Port has no dSv_collect class;
//     checkCollect()→0 selects Aryll's "not collected yet" dialogue branch. ---
struct dExtLs1_collect_c { int checkCollect(int) { return 0; } };
inline dExtLs1_collect_c* dComIfGs_getpCollect() { static dExtLs1_collect_c s; return &s; }

// --- WW g_dComIfG_gameInfo.play.mScopeWipeFlag (donor d_com_inf_game.h:867,
//     accessors getScopeWipeFlag/setScopeWipeFlag) is absent in the port's
//     fixed struct. Read once as a dormant demo gate; WW-default 0.
//     NAME LEARNED at the 512 re-baseline: upstream named field_0x4978 while
//     we were pinned behind it. The shim still returns 0 -- only the NAME
//     changed, so behaviour is untouched. ----
inline u8 dExtLs1_getScopeWipeFlag() { return 0; }

// §251 dExtLs1_getDemoBtk RETIRED — promoted to native dDemo_actor_c::getP_BtkData
//      (Foundry decode, d_demo.cpp §251). Aryll calls the native method directly.

// ============================================================================
// §253  Rito Postbox (d_a_obj_toripost) shims — the WW symbols the port lacks.
// ============================================================================

// --- dLib anim-param BCK driver (WW d/d_lib.h + d_lib.cpp). The port's d_lib.h
//     lacks these; ported VERBATIM from the WW donor (generic McaMorf BCK setter,
//     no toripost-specific behavior). Uses the port's native McaMorf setAnm/isStop.
class dLib_anm_prm_c {
public:
    /* 0x00 */ s8 mAnmIdx;
    /* 0x01 */ s8 mNextPrmIdx;
    /* 0x02 */ s16 field_0x02;
    /* 0x04 */ f32 mMorf;
    /* 0x08 */ f32 mPlaySpeed;
    /* 0x0C */ int mLoopMode;
};  // Size: 0x10
void dLib_bcks_setAnm(const char* arcName, mDoExt_McaMorf* morf, s8* pAnmIdx, s8* pPrmIdx,
                      s8* pOldPrmIdx, const int* bcksTbl, const dLib_anm_prm_c* anmPrmTbl, bool force);

// --- WW mail/letter subsystem query/mutate (WW d/d_letter.h). NOT ported: the port
//     has no mail state. Stubbed EMPTY-faithful (isStock/isDelivery -> false,
//     read/autoStock -> no-op) — the SAME treatment the port already gives
//     dLetter_send (above). The postbox then presents its "no mail today" branch;
//     its OWN state machine (modeWait/Talk/Receive) runs in full.
//     >>> BRIDGE-OWED: a real port of the WW letter subsystem is required before the
//         Rito Postbox can actually stock / read / deliver letters. <<<
bool dLetter_isStock(u16 i_no);
void dLetter_read(u16 i_no);
void dLetter_autoStock(u16 i_no);
bool dLetter_isDelivery(u16 i_no);

// --- WW obj sawdust hit-FX (daObj::HitEff_kikuzu) -> no-op (cosmetic; deferred like
//     the pig/bm1 particle FX). Free-func form (donor call rewritten off daObj:: scope).
void dExtTpost_HitEff_kikuzu(fopAc_ac_c* actor, void* cyl);
// --- §879 WW camera-smoother latch (dCamera_c::SkipSmoother — donor
//     d_camera.h:618 sets m100/m101/m102=1 so the smoother snaps for one frame).
//     The receiver camera has no smoother-latch mechanism; kikuzu-class owed
//     no-op until the WW-camera lane ports the smoother (CRAWL precedent,
//     ww_cam_crawl.cpp). Sites: npc_p1 talk-start ×3.
inline void dExtWwCam_SkipSmoother() {}
// --- WW player letter-read eye nudge (daPy_py_c::onLetterReadEyeMove) -> no-op.
void dExtTpost_onLetterReadEyeMove();
// --- WW present-demo cancel / reserve-item clear the port lacks -> inert no-op.
void dComIfGp_evmng_CancelPresent();
void dComIfGs_setReserveItemEmpty();

// ============================================================================
// §254  Tetra (d_a_npc_zl1) shims — the WW symbols the port lacks. Same doctrine
// as pig/seagull/bm1/Aryll: donor names kept; every gap is a port-absent WW
// subsystem (KoRL ship, WW branch prop, omamori-glow SFX, simple-land splash FX)
// on a story/cutscene-gated path, shimmed inert-faithful so the donor state
// machine COMPILES byte-faithfully and is runtime-inert where the subsystem is
// absent. BRIDGE-OWED items are flagged in the History report.
// ============================================================================

// --- draw priority: donor fpcDwPi_NPC_ZL1_e slot absent -> reuse a port
//     ground/ambient NPC draw-prio (same choice pig/seagull/bm1/Aryll made). ---
#define fpcDwPi_NPC_ZL1_e fpcDwPi_E_RD_e

// --- Nintendo-Gallery figurine type (WW d_snap.h idx 0x72; RegistFig is a no-op
//     so the value is inert — kept at the WW donor value for faithfulness). -----
#define DSNAP_TYPE_NPC_ZL1 0x72

// --- omamori-glow SFX (WW JA_SE_ITM_*). Bank not loaded -> 0 = silent no-op
//     through mDoAud_seStart. BRIDGE-OWED: real Z2SE ids on an audio pass. ------
#define JA_SE_ITM_OMAMORI_TETLA 0
#define JA_SE_ITM_OMAMORI_BLINK 0

// --- WW save event-flag ids Tetra reads/writes (byte<<8|bit, verbatim from the
//     WW donor d_save.h). The port's dSv_event_flag_c is TP-named and has no
//     UNK_* members, so (as Aryll §244) these pass to dComIfGs_isEventBit/
//     onEventBit as raw ints. Byte offsets <= 0x38 are in-bounds of the port
//     event array; story-gated (no WW Tetra spawns yet) -> runtime-dead. -------
#define WWEV_UNK_0101 0x0101
#define WWEV_UNK_0801 0x0801
#define WWEV_UNK_0802 0x0802
#define WWEV_UNK_0804 0x0804
#define WWEV_UNK_0810 0x0810
#define WWEV_UNK_0E20 0x0E20
#define WWEV_UNK_2401 0x2401
#define WWEV_UNK_2908 0x2908
#define WWEV_UNK_2D02 0x2D02
#define WWEV_UNK_3804 0x3804

// --- WW branch prop (daBranch_c, fpcNm_BRANCH_e). NOT ported. Tetra's type-1
//     "hang on branch" path searches for fpcNm_BRANCH_e; a sentinel that never
//     matches leaves mProcId1 = ERROR, so setMtx()'s branch-attach block is
//     never entered (inert-faithful: no branch prop exists in the port). The
//     stub type exists only so the (dead) branch-attach block COMPILES.
//     >>> BRIDGE-OWED: a real daBranch prop is needed for the Outset "awake /
//         stolen sister" cutscene where Tetra hangs from the tree branch. <<< ---
#define fpcNm_BRANCH_e 0xFFF9
class daBranch_c {
public:
    MtxP getJointMtx(const char*) { return NULL; }
};

// --- WW KoRL ship (daShip_c). NOT ported (no boat in the port). Tetra's
//     departure "swoon on deck" path calls dComIfGp_getShipActor(); returning
//     NULL makes swoon_OnShip() return false, so the ship-relative transform is
//     never applied (inert-faithful: no ship exists). The stub type + accessor
//     exist only so the (dead) swoon block COMPILES.
//     >>> BRIDGE-OWED: a real daShip is needed for the pirate-ship departure /
//         rescue cutscenes where Tetra rides the deck. <<< ---------------------
class daShip_c {
public:
    Mtx mBodyMtx;
    csXyz shape_angle;
    Mtx& getBodyMtx() { return mBodyMtx; }
};
inline daShip_c* dComIfGp_getShipActor() { return NULL; }

// --- WW simple-land splash/dust emitter (dComIfGp_particle_setSimpleLand). No
//     port emitter yet (same posture as setShipTail/setToon). Returns NULL; every
//     call site null-checks the result -> the water-splash / land-dust FX is a
//     deferred cosmetic (BRIDGE-OWED, wave-emit pass). gnd is the ground poly,
//     tevStr is void* (a dKy_tevstr_c* converts implicitly). ------------------
class cBgS_PolyInfo;
inline JPABaseEmitter* dComIfGp_particle_setSimpleLand(const cBgS_PolyInfo& /*gnd*/,
    const cXyz* /*pos*/, const csXyz* /*rot*/, f32 /*a*/, f32 /*b*/, f32 /*c*/,
    void* /*tevStr*/, int* /*o_id*/, int /*kind*/) { return NULL; }

// --- WW event-manager "cancel the auto-start demo" (dComIfGp_evmng_cancelStartDemo).
//     Absent in the port -> no-op. Only reached on Tetra type-5 (optn) event
//     ordering; the auto-start demo simply isn't cancelled (minor). BRIDGE-OWED. -
inline void dComIfGp_evmng_cancelStartDemo() {}

// ============================================================================
// §806  Donor object-arc alias — the CONSUMPTION-BOUNDARY path translation for
// donor arc names that collide with the receiver's own res/Object archives.
// The §716 pattern exactly: the donor-disc reader serves the WW arc's bytes
// VERBATIM under a renamed mount point (ww_donor_disc kTpCollisionAlias), and
// WW-port consumers translate the donor name here at load time. Donor DATA
// keeps donor names untouched; nothing TP-side is shadowed.
//   "Always"   — TP core archive (alink/bg_obj/demo00/...) → "WwAlways"
//   "Kkiba_00" — TP box archive (obj_carry/burnbox/movebox) → "WwKkiba00"
// The two tables MUST stay in lockstep (ww_donor_disc/main.cpp is the server).
// ============================================================================
inline bool dExtWw_arcNameEq(const char* a, const char* b) {
    while (*a != '\0' && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}
inline const char* dExtWw_objectArcAlias(const char* i_donorArc) {
    if (i_donorArc == 0) {
        return i_donorArc;
    }
    if (dExtWw_arcNameEq(i_donorArc, "Always")) {
        return "WwAlways";
    }
    if (dExtWw_arcNameEq(i_donorArc, "Kkiba_00")) {
        return "WwKkiba00";
    }
    return i_donorArc;
}

#endif  // D_EXT_WW_ACTOR_SHIMS_H
