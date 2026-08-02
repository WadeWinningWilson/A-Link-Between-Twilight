// ============================================================
// §223 Pass 1 — direct-port shims (see d_ext_ww_actor_shims.h). First-pass
// bodies: functional where cheap (ground-angle flat), no-op where cosmetic or
// gated on unrestored mechanics (figurine gallery, grab/throw, attack-collision
// bits). Pass 2 wires the real bodies as the d_a_kb source drop-in needs them.
// ============================================================
#include "d/d_ext_ww_actor_shims.h"
#include "d/d_item_data.h"  // §226 dItemNo_NONE_e (neutral equip-query result)
#include "f_op/f_op_actor_mng.h"  // §227 fopAcM_fastCreateItem (port signature)
#include "JSystem/J3DGraphAnimator/J3DModel.h"  // §239 J3DModel::getWeightAnmMtx (setter adapter)
#include "m_Do/m_Do_ext.h"        // §253 mDoExt_McaMorf::setAnm/isStop (dLib_bcks_setAnm)
#include "d/d_com_inf_game.h"     // §253 dComIfG_getObjectRes (dLib_bcks_setAnm)

// Flat ground angle (no slope-lean yet — the pig walks level first pass).
BOOL fopAcM_getGroundAngle(fopAc_ac_c* actor, csXyz* p_angle) {
    (void)actor;
    if (p_angle != NULL) {
        p_angle->x = 0;
        p_angle->y = 0;
        p_angle->z = 0;
    }
    return TRUE;
}

// Nintendo-Gallery figurine registration — not restored.
void dSnap_RegistFig(u8 /*type*/, fopAc_ac_c* /*actor*/, f32 /*sx*/, f32 /*sy*/, f32 /*sz*/) {}

// Pig size/skin material swap — Pass 2 routes through the mount BMT path.
void dExtKb_setMaterialTable(J3DModelData* /*md*/, J3DMaterialTable* /*bmt*/) {}

// Ground material code under the pig — §227 returns 0, which equals none of the
// dBgS_Attr_* sentinels (1..4), so the pig's dig-material switch is inert.
s32 dExtKb_GetAttributeCode(const cBgS_PolyInfo& /*gnd*/) {
    return 0;
}

// Attack-collision bits — only matter when the pig is hittable (deferred).
void dExtKb_OnAtHitBit(dCcD_Sph* /*sph*/) {}
void dExtKb_ClrAtSet(fopAc_ac_c* /*actor*/) {}

// Grab/throw target — carry mechanic; no grab first pass.
fopAc_ac_c* dExtKb_getGrabMissActor() {
    return NULL;
}

// --- esa (bait) sea/water shims — no sea system → bait always lands on ground.
bool daSea_ChkArea(f32 /*x*/, f32 /*z*/) {
    return false;
}
f32 daSea_calcWave(f32 /*x*/, f32 /*z*/) {
    return -1.0e30f;  // effectively no wave
}
f32 dBgS_GetWaterHeight(cXyz& /*pos*/) {
    return -1.0e30f;  // no water under the bait → ground-land path (mState 1)
}

// ============================================================
// §225 Pass 2 — pig (d_a_kb) particle emitters the port lacks. Return NULL so the
// pig's `if(cb.getEmitter()==NULL) set*(...)` guard treats them as never-emitting;
// no visual, no crash. Deferred to the wave-emit pass (same missing WW bank as the
// ship wake). No behavior depends on these — they are dust/ripple cosmetics.
// ============================================================
JPABaseEmitter* dComIfGp_particle_setShipTail(u16 /*id*/, const cXyz* /*pos*/, const csXyz* /*angle*/,
                                              const cXyz* /*scale*/, u8 /*alpha*/, dPa_rippleEcallBack* /*cb*/) {
    return NULL;
}
JPABaseEmitter* dComIfGp_particle_setToon(u16 /*id*/, const cXyz* /*pos*/, const csXyz* /*angle*/,
                                          const cXyz* /*scale*/, u8 /*alpha*/, dPa_smokeEcallBack* /*cb*/, int /*roomNo*/) {
    return NULL;
}

// ============================================================
// §226 Pass 2 — pig (d_a_kb) compile-only bodies. All no-op/sentinel: deferred
// combat (cc_at_check, equip query) and cosmetic J3D tex/material swaps that the
// port's J3D layer doesn't expose. No pig behavior depends on these first pass.
// ============================================================

// Collision attack-check — the pig registers a Tg hit but takes no damage yet.
void cc_at_check(fopAc_ac_c* /*actor*/, CcAtInfo* /*atInfo*/) {}

// Equipped-item slot query — neutral value keeps the damage branches inert.
int dComIfGs_getSelectEquip(int /*slot*/) {
    return dItemNo_NONE_e;
}

// Eye-blink tex-pattern animator + skin/size material swap — cosmetic no-ops.
void dExtKb_setTexNoAnimator(J3DModelData* /*md*/, J3DAnmTexPattern* /*pat*/, J3DTexNoAnm* /*txm*/) {}
void dExtKb_setAnmTexPattern(J3DTexNoAnm* /*txm*/, J3DAnmTexPattern* /*pat*/) {}

// §227 Item drop on dig. The port's fopAcM_fastCreateItem takes pointer speeds,
// no gravity param, and trailing (itemBitNo=-1, param_9=0, createFunc=NULL) —
// see d_a_npc_kasi_hana / d_a_npc_ykw. Gravity is dropped (port applies its own).
fopAc_ac_c* dExtKb_fastCreateItem(const cXyz* pos, int itemNo, int roomNo,
                                  const csXyz* angle, const cXyz* scale,
                                  f32 speedF, f32 speedY, f32 /*gravity*/) {
    f32 sf = speedF;
    f32 sy = speedY;
    return fopAcM_fastCreateItem(pos, itemNo, roomNo, angle, scale, &sf, &sy, -1, 0, NULL);
}

// J3DTexNoAnm::setAnmIndex — no-op; the pig's tex-pattern animator is no-op'd,
// so the per-material index never feeds a live animator.
void dExtKb_setAnmIndex(J3DTexNoAnm* /*txm*/, u16 /*idx*/) {}

// ============================================================
// §232 Pass 2 — d_a_kamome (SEAGULL) donor-gap bodies. The seagull uses this only
// on the ship-following (heisou) path, which is dead in the port (no KoRL boat).
// "No enemy sound" is the neutral default that keeps the donor logic faithful.
// ============================================================
bool dComIfGp_att_chkEnemySound() {
    return false;
}

// ============================================================
// §239 Pass 1 — WW NPC framework (fopNpc_npc_c + dNpc_*) donor-gap bodies.
// See d_ext_ww_actor_shims.h §239. cLib_addCalcAngleL is VERBATIM donor; the
// reserve-item queries return the "nothing reserved" default; the weight-anm-mtx
// setter mirrors the port's own J3DMtxBuffer::setAnmMtx MTXCopy idiom.
// ============================================================

// VERBATIM WW donor (c_lib.cpp cLib_addCalcAngleL__FPlllll) — s32 twin of the
// port's s16 cLib_addCalcAngleS easing.
s32 cLib_addCalcAngleL(s32* pValue, s32 target, s32 scale, s32 maxStep, s32 minStep) {
    s32 diff = target - *pValue;
    if (*pValue != target) {
        scale = (diff) / scale;
        if (scale > minStep || scale < -minStep) {
            if (scale > maxStep) {
                scale = maxStep;
            }
            if (scale < -maxStep) {
                scale = -maxStep;
            }
            *pValue += scale;
        } else {
            if (0 <= diff) {
                *pValue += minStep;
                if (0 >= target - *pValue) {
                    *pValue = target;
                }
            } else {
                *pValue -= minStep;
                if (0 <= target - *pValue) {
                    *pValue = target;
                }
            }
        }
    }
    return diff;
}

// Reserved-item queries — the port has no reserve system → "nothing reserved".
BOOL dComIfGs_isGetItemReserve(u8 /*i_no*/) {
    return FALSE;
}
u8 dComIfGs_checkReserveItem(u8 /*i_itemNo*/) {
    return 0;
}

// J3DModel::setWeightAnmMtx adapter — the port ships only the getter (returns the
// weight-envelope matrix pointer); copy into it exactly as J3DMtxBuffer::setAnmMtx
// does. Used by dNpc_setShadowModel to mirror the target model's skinning mtxs.
void dExtNpc_setWeightAnmMtx(J3DModel* self, int idx, MtxP mtx) {
    MTXCopy(mtx, self->getWeightAnmMtx(idx));
}

// ============================================================
// §246 d_a_npc_bm1 (Generic Ritos) donor-gap bodies. All faithful-in-effect:
// the WW subsystems they stand in for (symbols, beast counter, letter, demo,
// figurine, Rito feather FX) are not ported, and these Ritos do not spawn in the
// port, so every body here is on a runtime-dead path. See d_ext_ww_actor_shims.h.
// ============================================================
#include "m_Do/m_Do_ext.h"  // §246 mDoExt_McaMorf full type (entryDL adapter)
#include "f_op/f_op_msg.h"   // §246 fopMsg_MODE_* (msg-status shim)
#include "d/d_demo.h"        // §244 dDemo_c::getActor (real demo-actor lookup)
#include "d/d_stage.h"       // §244 dStage_roomControl_c::getDemoArcName (demo arc)
#include "d/d_com_inf_game.h" // §244 dComIfG_getObjectIDRes (resolve demo btp/btk)

// Message-state accessors — port msg_class is a stub; return inert values
// (runtime-dead: these Ritos never talk). "Still typing" is talk_1's no-op case.
u16 dExtNpcBm1_msgStatus(msg_class* /*m*/) {
    return fopMsg_MODE_MSG_TYPING_e;
}
s16 dExtNpcBm1_msgSelectNum(msg_class* /*m*/) {
    return 0;
}

// Sound-anim stop on delete — port McaMorf has no stopZelAnime → no-op.
void dExtNpcBm1_stopZelAnime(mDoExt_McaMorf* /*m*/) {}

// §251 Demo FACE-EXPRESSION reconstruction RETIRED — promoted to the true native
// subsystem. Foundry decoded the donor's getP_BtpData/getP_BtkData/getP_BrkData from
// the donor binary (docs/WW Linked/getP_BtpData-decode.md); the native methods now
// live on dDemo_actor_c (src/d/d_demo.cpp §251) and every cutscene actor (Aryll, bm1)
// calls them directly. The reconstruction here (dExtNpcBm1_getDemoBtp / dExtLs1_getDemoBtk)
// is deleted — standing directive: bridges get replaced by the native subsystem.
// (Confirmations from the decode: "rebind only on resID change" IS native donor
// behavior, and the id's 0x10000 bit natively selects the demo archive.)

// WW din-symbol query — no symbol system → "not shown".
BOOL dComIfGs_isSymbol(u8 /*i_no*/) {
    return FALSE;
}

// WW golden-feather / beast counter — none collected → 0.
u8 dComIfGs_getBeastNum(int /*i_beastIdx*/) {
    return 0;
}

// WW beast-count adjust — nothing to spend → no-op.
void dComIfGp_setItemBeastNumCount(int /*i_beastIdx*/, s16 /*num*/) {}

// WW message anime-tag query — no tag active (0xFF), so anmAtr's tag branch is inert.
u8 dComIfGp_getMesgAnimeTagInfo() {
    return 0xFF;
}

// WW postman-0 status-map clear — status-map not restored → no-op.
void fopAcM_ClearStatusMap(fopAc_ac_c* /*actor*/) {}

// WW letter send — dLetter subsystem not ported → no-op (event_proc case 2 dead).
void dLetter_send(u16 /*i_no*/) {}

// WW demo-actor lookup. Maps an actor's demoActorID → its bound dDemo_actor_c via
// the port's own accessor (d_demo.cpp:338 uses dDemo_c::getActor identically). §246
// bm1's Ritos never bind (id 0 → NULL, matching the old stub). §244 Aryll's awake /
// telescope demo DOES bind (id != 0 → the real actor) — returning NULL there NULL-
// derefs in daNpc_Ls1_c::demo()->checkEnable(). id 0 is guarded (unbound → NULL).
dDemo_actor_c* dComIfGp_demo_getActor(u8 id) {
    if (id == 0) {
        return NULL;
    }
    return dDemo_c::getActor(id);
}

// Material-table entryDL adapter — port McaMorf::entryDL() takes no arg. The
// recolor bmt (bm02/bmarm02) is DEFERRED like the pig's BMT swap; submit the base
// DL so the arm-having Rito variants still render (base palette, not the variant).
void dExtNpcBm1_entryDL_mat(mDoExt_McaMorf* morf, J3DMaterialTable* /*bmt*/) {
    if (morf != NULL) {
        morf->entryDL();
    }
}

// WW Rito feather particle emit — no feather bank in the port → no-op (NULL
// emitter; each call site null-checks the result). Deferred cosmetic FX.
JPABaseEmitter* dExtNpcBm1_particleSet(u16 /*id*/, const void* /*pos*/, const void* /*rot*/,
                                       const void* /*scale*/, int /*alpha*/, const void* /*cb*/,
                                       s8 /*room*/, const void* /*prmColor*/,
                                       const void* /*envColor*/) {
    return NULL;
}
JPABaseEmitter* dExtNpcBm1_particleSet(u16 /*id*/, const void* /*pos*/, const void* /*rot*/,
                                       const void* /*scale*/, int /*alpha*/, const void* /*cb*/,
                                       s8 /*room*/) {
    return NULL;
}

// ============================================================================
// §253  Rito Postbox (d_a_obj_toripost) shim bodies.
// ============================================================================

// dLib anim-param BCK driver — VERBATIM from the WW donor (src/d/d_lib.cpp). Generic
// McaMorf BCK setter; no toripost-specific behavior. Native port McaMorf setAnm/isStop.
void dLib_bcks_setAnm(const char* arcName, mDoExt_McaMorf* morf, s8* pAnmIdx, s8* pPrmIdx,
                      s8* pOldPrmIdx, const int* bcksTbl, const dLib_anm_prm_c* anmPrmTbl, bool force) {
    if ((*pOldPrmIdx != *pPrmIdx && anmPrmTbl[*pPrmIdx].mAnmIdx != -1) || force == true) {
        *pAnmIdx = anmPrmTbl[*pPrmIdx].mAnmIdx;
        J3DAnmTransform* bck = (J3DAnmTransform*)dComIfG_getObjectRes(arcName, bcksTbl[*pAnmIdx]);
        morf->setAnm(bck, anmPrmTbl[*pPrmIdx].mLoopMode, anmPrmTbl[*pPrmIdx].mMorf,
                     anmPrmTbl[*pPrmIdx].mPlaySpeed, 0.0f, -1.0f, NULL);
    }

    *pOldPrmIdx = *pPrmIdx;

    if (morf->isStop()) {
        if (anmPrmTbl[*pPrmIdx].mNextPrmIdx != -1 && anmPrmTbl[*pPrmIdx].mLoopMode == J3DFrameCtrl::EMode_NONE) {
            *pPrmIdx = anmPrmTbl[*pPrmIdx].mNextPrmIdx;
        }
    }
}

// WW letter subsystem — EMPTY-faithful stubs (BRIDGE-OWED: real WW mail port required).
bool dLetter_isStock(u16 /*i_no*/) { return false; }
void dLetter_read(u16 /*i_no*/) {}
void dLetter_autoStock(u16 /*i_no*/) {}
bool dLetter_isDelivery(u16 /*i_no*/) { return false; }

// Cosmetic / inert no-ops (deferred FX + unported present-demo/reserve supply).
void dExtTpost_HitEff_kikuzu(fopAc_ac_c* /*actor*/, void* /*cyl*/) {}
void dExtTpost_onLetterReadEyeMove() {}
void dComIfGp_evmng_CancelPresent() {}
void dComIfGs_setReserveItemEmpty() {}
