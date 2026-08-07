/**
 * d_a_ext_ep.cpp
 * §394 — the donor WW `d_a_ep` TORCH, **Type 1 only** (brazier-less flame).
 *
 * ============================================================================
 * WHY THIS EXISTS, AND WHY IT IS SMALL
 *
 * Grandma's room authors one `bonbori` SCOB row with params `ffffff81`. Decoded
 * against the donor's own `daEp_Create` (d_a_ep.cpp:643-652) — and independently
 * against Winditor's `ep.json` parameter template, which agrees field for field:
 *
 *   Type          = params & 0x3F        = 1   "does not have brazier"
 *   mbHasGa       = (params >> 6) & 1    = 0   no fireflies
 *   mbHasObm      = (params >> 7) & 1    = 1   heap-size only, brazier branch only
 *   mOnSwitchNo   = params >> 0x18       = 0xFF  no switch -> LIT BY DEFAULT
 *
 * The receiver has been filling that slot with TP's own `Ep`, which draws a
 * torch STAND. The donor row says there is no stand. That mismatch is the
 * "Ivan" the §389 verifier flagged as SUBSTITUTED — a receiver actor rendering
 * geometry the data explicitly excludes. Not scale, not placement.
 *
 * TYPE 1 IS THE CHEAP BRANCH, and that is a donor fact rather than a shortcut:
 * in `daEp_Create` the whole `if (mType == 0 || mType == 3)` block — solid heap,
 * model create, `dCcD_Cyl`, `dBgS_ObjAcch` — DOES NOT RUN for type 1, and
 * `daEp_Draw` draws the model only for types 0/3. What type 1 does run:
 *   resLoad("Ep") -> mStts.Init + the dCcD_Sph fire-damage sphere
 *   -> daEp_CreateInit -> dKy_plight_set (the light) -> flame particle
 *   -> ep_draw()'s alpha-model glow.
 *
 * ============================================================================
 * BANNERED DELTAS FROM THE DONOR (each one, with its reason)
 *
 *   #1 SCOPE   : types 0, 2 and 3 are NOT ported. This TU refuses to create for
 *                any type but 1 rather than half-serving a brazier — a partial
 *                brazier would be a second Ivan, drawn by us instead of TP.
 *   #2 Ga      : the firefly pair (`mbHasGa`, ga_draw) is not ported. Our row
 *                has the bit CLEAR, so porting it would be unexercised code
 *                with no placement to validate it against.
 *   #3 SWITCH  : `SHOKUDAI_SWITCH` event begin/move/end is not ported. Our row
 *                is `mOnSwitchNo == 0xFF` (lit by default, never switched), and
 *                the donor's switch path also runs only for types != 3.
 *   #4 ASSERT  : donor JUT_ASSERT -> NULL-guard (port asserts panic; retail
 *                donor asserts compile out — guarding is the faithful behavior).
 *   #5 INERT   : LANDED INERT. No OBJNAME row, no actor_map section, so nothing
 *                creates this actor yet. Activation means repointing `bonbori`
 *                away from the receiver's own `fpcNm_EP_e` in `d_stage.cpp` —
 *                a receiver-table edit, user-gated (§329 precedent).
 * ============================================================================
 */
// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_ep.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include "d/d_com_inf_game.h"
#include "d/d_cc_d.h"
#include "d/d_drawlist.h"
#include "d/d_kankyo.h"
#include "d/d_particle.h"
#include "d/d_particle_name.h"
#include "d/d_s_play.h"  // g_regHIO (REG reads)
#include "d/d_ext_ww_actor_shims.h"
#include "SSystem/SComponent/c_phase.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "dusk/logging.h"

extern aurora::Module DuskLog;

// Donor REG reads. The receiver ships no global REG0_S macro; d_a_kamome.cpp
// established the local-define precedent for donor actors that read the same
// register file, and this is that define verbatim.
// §394 delta #8 — REG reads folded to their RETAIL values. The donor gates its
// flame rate on `REG0_S(7) + 7`, an HIO debug-tuning register that is ZERO in a
// retail build, so retail behaviour is exactly 7. The receiver exposes no
// equivalent register file, and carrying a fake one would add a knob the donor
// does not have at retail. 7 is the donor's shipped number, not a chosen one.
static const s16 kEpFireRateRetail = 7;  // donor: REG0_S(7) + 7, REG0_S == 0

namespace daExtEp {

// Donor `ep_class`, reduced to the fields the TYPE-1 path actually touches.
// Field names are the donor's so the two can be read side by side; nothing is
// renamed for taste.
class Act_c : public fopAc_ac_c {
public:
    cPhs_Step _create();
    BOOL _delete();
    BOOL _execute();
    BOOL _draw();

    request_of_phase_process_class mPhase;
    Mtx mAlphaModelMtx;
    f32 mLightPower;
    f32 m4E0;
    f32 mAlphaModelScale;
    f32 mAlphaModelScaleTarget;
    f32 mAlphaModelAlpha;
    f32 mAlphaModelAlphaTarget;
    s16 mAlphaModelRotX;
    s16 mAlphaModelRotY;
    s16 mTimers[4];
    s16 m7D4;
    s32 m4D0;
    u8 mType;
    u8 mOnSwitchNo;
    u8 mbNoEp;
    u8 mGroundCheckTimer;
    LIGHT_INFLUENCE mLight;
    cXyz mPosTop;
};

// Donor `ep_draw` (d_a_ep.cpp:41) — the flame GLOW.
//
// §394 delta #6 — DEFERRED, NOT SUBSTITUTED. The donor emits its glow through
// `dComIfGd_setAlphaModel(dDlst_alphaModel_c::TYPE_SPHERE | TYPE_TWO_SPHERES,
// mtx, alpha)` after `dComIfGd_setAlphaModelColor({0xEB,0x7D,0x00,0x00})`.
// **The receiver has no `dDlst_alphaModel_c` and no `setAlphaModel` at all** —
// it is a WW draw-list primitive with no counterpart here.
//
// Reaching for the nearest-looking receiver effect would be exactly the
// mimicry the standing directive forbids: it would look lit and be a bridge.
// So the glow is OWED, recorded here with the donor's own numbers so the port
// is a lookup rather than a re-derivation, and the flame ships meanwhile as the
// particle + dynamic light — both of which ARE donor-native.
//
// Owed: port `dDlst_alphaModel_c` (TYPE_SPHERE / TYPE_TWO_SPHERES) into the
// receiver draw list, then restore this function body verbatim.
void ep_draw(Act_c* i_this) {
    (void)i_this;
}

// Donor `daEp_CreateInit` (d_a_ep.cpp:553), minus `daEp_set_mtx` — that function
// touches the model matrix only on the 0/3 branch, so there is nothing for it to
// do here (delta #1).
void CreateInit(Act_c* i_this) {
    i_this->m4D0 = 0;

    fopAcM_SetMtx(i_this, i_this->mAlphaModelMtx);
    fopAcM_SetMin(i_this, -160.0f, -160.0f, -160.0f);
    fopAcM_SetMax(i_this, 160.0f, 160.0f, 160.0f);
    i_this->mAlphaModelRotX = cM_rndF(0x8000);
    i_this->mAlphaModelRotY = cM_rndF(0x8000);
    i_this->attention_info.position.x = i_this->current.pos.x;
    i_this->attention_info.position.y = i_this->current.pos.y + 100.0f;
    i_this->attention_info.position.z = i_this->current.pos.z;
    i_this->eyePos.x = i_this->current.pos.x;
    i_this->eyePos.y = i_this->current.pos.y + 130.0f;
    i_this->eyePos.z = i_this->current.pos.z;

    i_this->mbNoEp = 0;
}

cPhs_Step Act_c::_create() {
    // ========================================================================
    // §394 delta #7 — FIRE-DAMAGE SPHERE DEFERRED (owed, not substituted).
    // The donor sets a dCcD_Sph whose Tg types are
    //   AT_TYPE_FIRE | AT_TYPE_UNK20000 | AT_TYPE_FIRE_ARROW | AT_TYPE_WIND |
    //   AT_TYPE_UNK400000
    // (d_a_ep.cpp:586). The receiver defines AT_TYPE_UNK20000, AT_TYPE_WIND and
    // AT_TYPE_UNK400000 (via d_ext_ww_actor_shims.h) but has **no AT_TYPE_FIRE
    // and no AT_TYPE_FIRE_ARROW at all** — those bits do not exist here.
    //
    // Picking plausible bit values would be a guess wearing a donor's name, and
    // guessed constants are precisely what has cost this campaign its worst
    // rounds. So the sphere is OWED with its donor type list recorded above; the
    // flame ships now as particle + light, which is what makes it VISIBLE, and
    // arrow-lighting / burn-on-touch land when the AT bits are resolved.
    // ========================================================================

    const cPhs_Step ret = dComIfG_resLoad(&mPhase, "Ep");
    if (ret != cPhs_COMPLEATE_e) {
        return ret;
    }

    fopAcM_ct(this, Act_c);  // §394 delta #4b: receiver has no ct_Retail/ct_Demo split

    mType = fopAcM_GetParam(this) & 0x3F;
    if (mType == 0x3F) {
        mType = 0;  // donor normalisation (d_a_ep.cpp:644)
    }
    mOnSwitchNo = fopAcM_GetParam(this) >> 0x18;

    // §394 delta #1 — SCOPE. Only the brazier-less flame is ported. Anything
    // else must not be half-served: a partial brazier would be a second Ivan.
    if (mType != 1) {
        DuskLog.warn("[ExtEp] §394 type {} is not ported (only type 1, the "
                     "brazier-less flame) — refusing rather than half-drawing",
                     (int)mType);
        return cPhs_ERROR_e;
    }


    // NOTE: the donor's `if (mType == 0 || mType == 3)` block is deliberately
    // absent — no solid heap, no model, no cylinder, no dBgS_ObjAcch. That is
    // the donor's own control flow for type 1, not an omission.

    CreateInit(this);

    mTimers[3] = 20000;
    dKy_plight_set(&mLight);
    mGroundCheckTimer = cM_rndF(255.0f);
    mLightPower = 0.0f;
    m4E0 = scale.x;
    m4D0 = 4;  // lit by default: our row has no switch (mOnSwitchNo == 0xFF)
    m7D4 = 0;
    mAlphaModelAlpha = 0.0f;
    mAlphaModelAlphaTarget = 1.0f;

    mPosTop = current.pos;
    mPosTop.y += 40.0f;

    DuskLog.info("[ExtEp] §394 type-1 flame created at ({:.0f},{:.0f},{:.0f}) "
                 "switch={:#04x} (lit by default)",
                 current.pos.x, current.pos.y, current.pos.z, (unsigned)mOnSwitchNo);
    return cPhs_COMPLEATE_e;
}

BOOL Act_c::_delete() {
    dKy_plight_cut(&mLight);
    return TRUE;
}

BOOL Act_c::_execute() {
    mbNoEp = 0;

    // Donor state 4 (the burning steady state, d_a_ep.cpp:261-298) reduced to
    // the type-1 path: the switch/event arms and the brazier collision are
    // deltas #2/#3 and do not appear.
    cLib_addCalc2(&mLightPower, m4E0, 0.5f, 0.2f);

    cXyz pos = current.pos;
    pos.y += 20.0f;
    if (m7D4 < kEpFireRateRetail) {
        GXColor fireK = {0xFF, 0x96, 0x32, 0xFF};
        dComIfGp_particle_setSimple(ID_AK_JN_O_FIRE00, &pos, 0xFF, fireK, fireK, 1, 0.0f);
    }
    if (m7D4 > 0) {
        m7D4--;
    }

    // Donor light: colour and power follow the flame.
    mLight.mPosition = mPosTop;
    mLight.mColor.r = 0xFF;
    mLight.mColor.g = 0x96;
    mLight.mColor.b = 0x32;
    mLight.mColor.a = 0xFF;
    mLight.mPow = mLightPower * 400.0f;
    mLight.mFluctuation = 0;

    cLib_addCalc2(&mAlphaModelAlpha, mAlphaModelAlphaTarget, 0.5f, 0.2f);

    return TRUE;
}

BOOL Act_c::_draw() {
    // Donor daEp_Draw for type 1: the model block is skipped entirely; only the
    // flame glow is emitted.
    if (mbNoEp != 1) {
        ep_draw(this);
    }
    return TRUE;
}

// --- process method plumbing ------------------------------------------------
BOOL Create(void* v) { return ((Act_c*)v)->_create(); }
BOOL Delete(void* v) { return ((Act_c*)v)->_delete(); }
BOOL Execute(void* v) { return ((Act_c*)v)->_execute(); }
BOOL Draw(void* v) { return ((Act_c*)v)->_draw(); }
BOOL IsDelete(void*) { return TRUE; }

actor_method_class Table = {
    (process_method_func)Create, (process_method_func)Delete, (process_method_func)Execute,
    (process_method_func)IsDelete, (process_method_func)Draw,
};

}  // namespace daExtEp

// §394 delta #5 — LANDED INERT. This profile exists and links, but no OBJNAME
// row and no actor_map section reference it, so nothing creates it. Activation
// = repointing `bonbori` off the receiver's own fpcNm_EP_e, which is a
// receiver-table edit and therefore the user's call.
actor_process_profile_definition g_profile_EXT_EP = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,  // donor g_profile_EP list id, verbatim
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_EXT_EP_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daExtEp::Act_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_EP_e,
    /* Actor SubMtd */ &daExtEp::Table,
    /* Status       */ fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};
