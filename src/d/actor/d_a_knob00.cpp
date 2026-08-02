/**
 * d_a_knob00.cpp
 * Object - Regular doors (with knobs, not sliding)
 *
 * ============================================================================
 * §328 WW KNOB DOOR (KNOB00) — TWO IMPLEMENTATIONS, COMPILE-GATED (the gate
 * macro DUSK_WW_KNOB00_NATIVE lives in d/actor/d_a_knob00.h; default 0):
 *
 *   DUSK_WW_KNOB00_NATIVE == 1 : the §328 DIRECT PORT — body VERBATIM from the
 *   WW donor (D:/XXXXXXX/WW DP/src/d/actor/d_a_knob00.cpp, fully matched), on
 *   the §328-ported d_door base layer (src/d/d_door.cpp). VERSION blocks follow
 *   the RETAIL (#else) side like every landed WW port. Direct-port recipes at
 *   the call sites only (banners inline), mirroring lamp/mshokki §327:
 *     #1 DN-3 : models via dExtNpcMount_acquireModelData("knob", <member>) —
 *               donor res ids verified TWICE (staged Knob.residmap.csv AND the
 *               donor Knob.h/RARC parse agree): 9=door.bdl, 0xA..0x11=
 *               door_a..door_h.bdl, 5/6=dooropen{a,b}door.bck, 0x14=door.dzb
 *     DN-1    : the dBgW is Set(MOVE_BG) + Bgsp Regist(owner=this) with NO
 *               SetRoomId anywhere — donor-verbatim; the room stays at the
 *               daBg default resolution (the DN-1 sanctioned "leave default"
 *               path). The only room writes are the ACTOR's own identity from
 *               its placement data (getFRoomNo) — positively resolved, never
 *               guessed.
 *     mDoExt_bckAnm::init donor (modelData, bck, ...) -> port 7-arg
 *     eventInfo.setToolId -> setMapToolId ; evmng_getMyStaffId 1 -> 3-arg
 *     JUT_ASSERT -> NULL-guard/log ; cPhs_State -> cPhs_Step
 *     *dComIfG_Bgsp() -> reference form ; resDeleteDemo -> resDelete
 *     entrySolidHeap donor 0x2700 -> 0x80003800 (PC heap-size semantics; the
 *               §27-proven value, == TP knob20)
 *   WW save event bits/regs ride the DONOR flag block via the §303 route header
 *   (LAST include) with value-faithful WWEV_* ids. WW-only subsystems absent
 *   from the port (password input UI, Mt figure NPC, pirate ship, TAG_MK) are
 *   inert-faithful local stubs, each tagged §328 BRIDGE-OWED below.
 *
 *   DUSK_WW_KNOB00_NATIVE == 0 (DEFAULT) : the §27 stand-in (unchanged), which
 *   src/d/d_ext_npc_doors.cpp is wired to (setDoorKey/doorKey/spawnSrc API +
 *   dExtNpcDoors_tryNativeWarp on demo end). Flipping the gate requires the
 *   integration-seam decisions listed in the §328 report.
 * ============================================================================
 */

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/actor/d_a_knob00.h"  // carries the DUSK_WW_KNOB00_NATIVE gate

#if DUSK_WW_KNOB00_NATIVE

#include "d/actor/d_a_player.h"
#include "d/d_kankyo.h"
#include "m_Do/m_Do_graphic.h"
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_mng.h"          // §328 fopAcM_* helpers (toripost include set)
#include "f_pc/f_pc_manager.h"            // §328 fpcM_ERROR_PROCESS_ID_e
#include "m_Do/m_Do_ext.h"                // §328 mDoExt_J3DModel__create / modelUpdateDL
#include "m_Do/m_Do_mtx.h"                // §328 mDoMtx_stack_c
#include "SSystem/SComponent/c_math.h"    // §328 cM_ssin/cM_scos/cM_rndF
#include "d/d_ext_npc_mount.h"            // §328 DN-3 parse-at-consume model resolver
#include "d/d_ext_ww_actor_shims.h"       // §328 fopAcStts_*/SHOWMAP/TEV_TYPE_ACTOR/dSymbol_DIN/isSymbol
#include "dusk/logging.h"                 // §328 retail-assert fallbacks log instead of panicking
#include "JSystem/JKernel/JKRHeap.h"      // §328 JKR_NEW (solid-heap allocation idiom)
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include <cctype>                          // §328 donor "ctype.h" (std::tolower)
#include <cstring>                         // §328 strcpy/strcmp (retail chkPassward)
#include <cstdio>                          // §329 snprintf (door-network stamp surface)
#include "d/d_ext_npc_doors.h"             // §329 seam 2: port warp backend at openEnd

// §303 SYMBOL ROUTE — MUST BE THE LAST INCLUDE. Remaps this TU's verbatim
// dComIfGs_*EventBit / *EventReg calls (WW dSv_event indices) to the donor
// event-flag block — never TP's table.
#include "d/d_ext_save_flags_route.h"

// §328 ========================================================================
// WW-absent VALUE-FAITHFUL constants (donor numbering; §327 pattern — LOCAL to
// this TU, not the shared shims header).
// ----------------------------------------------------------------------------
// WW JAZelAudio_SE.h ids (SE bank not loaded; later audio pass maps Z2SE ids).
#define JA_SE_OBJ_DOOR_N_OPEN    0x696E
#define JA_SE_OBJ_DOOR_N_CLOSE_1 0x696F
#define JA_SE_OBJ_DOOR_N_CLOSE_2 0x6970
// WW dSv_event_flag_c scoped bits/regs -> unscoped WWEV_* (§244 pattern; value
// == donor name, verbatim WW d_save_event_flag.inc). Routed to the DONOR block
// by the route header above. (WWEV_UNK_1A80 + WWEV_ENDLESS_NIGHT come from the
// shims header — not redefined here.)
#define WWEV_UNK_2110 0x2110
#define WWEV_UNK_0520 0x0520
#define WWEV_UNK_1701 0x1701
#define WWEV_UNK_3B20 0x3B20
#define WWEV_UNK_BA0F 0xBA0F  // event REG (password index), donor UNK_BA0F
#define WWEV_UNK_1910 0x1910
#define WWEV_UNK_2D80 0x2D80
#define WWEV_UNK_3401 0x3401
// WW dSymbol_NAYRU_e == 0 (donor d_com_inf_game.h). Inert: dComIfGs_isSymbol is
// the §246 FALSE shim. (dSymbol_DIN_e comes from the shims header; its shim
// value 0 differs from the donor's 1 but is equally inert.)
#define dSymbol_NAYRU_e 0
// WW dItemNo_BOMB_BAG_e (donor d_item_data.h) — §253 toripost defined the same.
#define dItemNo_BOMB_BAG_e 0x31
// WW attention flag name -> the port's value-identical fopAc_AttnFlag_DOOR_e.
#define fopAc_Attn_ACTION_DOOR_e fopAc_AttnFlag_DOOR_e
// WW status bit (OnStatus on the Mt figure NPC — runtime-dead, stub below).
#define fopAcStts_UNK800_e 0x00000800
// Env-light TEV struct type (WW d_kankyo.h: TEV_TYPE_BG0 == 1; ACTOR == 0 from
// the shims header). Same local macro toripost/lamp §253/§327 use.
#define TEV_TYPE_BG0 1
// Donor draw-prio slot absent -> the port's knob-door slot (the §27 stand-in's
// choice; TP knob20 uses the same slot).
#define fpcDwPi_KNOB00_e fpcDwPi_KNOB20_e
// WW pirate-ship proc name — daObjPirateship is NOT ported; sentinel that never
// matches a live actor name, so the donor draw() link-id branch is inert.
#define fpcNm_Obj_Pirateship_e 0xFFF7
// ----------------------------------------------------------------------------
// §328 BRIDGE-OWED stub types/functions (inert-faithful; donor names kept so
// the source stays verbatim):
// · daNpcMt_c (WW Windfall "Mt", Maggie's Father figure-door partner): NOT
//   ported. fopAcM_searchFromName("Mt",0,0) can only find a live proc named
//   "Mt" — none exists -> every Mt branch takes the donor NULL side. The stub
//   exists so the (dead) non-NULL side COMPILES.
class daNpcMt_c : public fopAc_ac_c {
public:
    void attnOn() {}
    void attnOff() {}
};
// · daObjPirateship::Act_c (WW pirate-ship interior door host): NOT ported;
//   name sentinel above never matches -> block is dead; stub for compile only.
namespace daObjPirateship {
class Act_c : public fopAc_ac_c {
public:
    u8 m2CC;
};
}  // namespace daObjPirateship
// · WW password-input UI (Windfall bomb-shop door): the port has no
//   InputPassword subsystem. Stubs present the donor "cancelled" path
//   (OpenCheck -> 0 => msg 0x1B19 then back to the passward action), and
//   chkPassward's WWEV_UNK_2110 gate (routed, donor-block) fails first on a
//   fresh block anyway. >>> BRIDGE-OWED: real password-entry UI port. <<<
static void dComIfGp_InputPasswordOpenOn() {
    DuskLog.warn("[Knob00] §328 BRIDGE-OWED: WW password-input UI not ported (open request dropped)");
}
static s32 dComIfGp_InputPasswordOpenCheck() {
    return 0;  // donor "cancelled" result
}
static char* dComIfGp_getInputPassword() {
    static char s_empty[17];
    s_empty[0] = '\0';
    return s_empty;
}
static void fopMsgM_passwordGet(char* o_buf, int i_id) {
    (void)i_id;
    o_buf[0] = '\0';
}
// WW AGB (GBA Tingle-link) map-send flag — the subsystem does not exist on TP.
// Inert no-op keeps the donor source verbatim (twin of d_door.cpp's set-flag).
static void dComIfGp_map_clrAGBMapSendStopFlg() {}
// §328 ========================================================================

const char daKnob00_c::M_arcname[] = "Knob";  // §329b: LOAD by the staged/mounted
// spelling — the R2 overlay map is case-SENSITIVE at open (mount key 'Knob'),
// so donor "knob" error-looped resLoad and every island door silently failed
// create (observed 2026-08-01 17:11 log: fpcBs_Create×5 repeating, no [Knob00]
// lines). Post-load lookups are stricmp (d_resorce.cpp:909) so only the open
// cares. Donor spells it "knob"; §328 donor spelling; the port's
                                              // arc lookup is stricmp -> resolves
                                              // the staged Knob.arc

/* 00000078-000000B4       .text daKnob00_charactorExchange__FPc */
void daKnob00_charactorExchange(char* password) {
    while (*password) {
        u8 chr = *password;
        *password = std::tolower(chr);
        password++;
    }
}

/* 000000B4-000000D4       .text CheckCreateHeap__FP10fopAc_ac_c */
static BOOL CheckCreateHeap(fopAc_ac_c* a_this) {
    return ((daKnob00_c*)a_this)->CreateHeap();
}

/* 000000D4-000003B8       .text CreateHeap__10daKnob00_cFv */
BOOL daKnob00_c::CreateHeap() {
    // §328 DN-3 (recipe #1): donor dComIfG_getObjectRes(M_arcname,
    // dRes_INDEX_KNOB_BDL_DOOR_e /*=9*/) — res id 9 == "door.bdl" (verified
    // twice: staged Knob.residmap.csv AND donor Knob.h/RARC parse agree).
    // Donor JUT_ASSERT(VERSION_SELECT(145,145,163,163), modelData) -> NULL-guard.
    J3DModelData* modelData = dExtNpcMount_acquireModelData(M_arcname, "door.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §328 CreateHeap: '{}' member 'door.bdl' not resolvable", M_arcname);
        return FALSE;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);

    J3DModel* model = mpModel;
    if (model == NULL) {
        // §328 RETAIL (#else) side of the donor VERSION_DEMO block kept.
        return FALSE;
    }

    model->setBaseScale(scale);
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());

    // §328 donor id dRes_INDEX_KNOB_BCK_DOOROPENADOOR_e == 5 == "dooropenadoor.bck".
    J3DAnmTransform* anmTransform = (J3DAnmTransform*)dComIfG_getObjectRes(M_arcname, "dooropenadoor.bck");
    // §328 donor init(modelData, anm, true, EMode_NONE) -> port 7-arg.
    if (!mBckAnm.init(anmTransform, true, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, false)) {
        return FALSE;  // §328 retail side
    }

    m_jnt = modelData->getJointName()->getIndex("DoorDummy");
    // §328 donor JUT_ASSERT(VERSION_SELECT(169,169,187,187), m_jnt >= 0) -> guard.
    if (m_jnt < 0) {
        DuskLog.warn("[Knob00] §328 CreateHeap: DoorDummy joint missing");
        return FALSE;
    }

    // §328 donor fileIndex switch (dRes_INDEX_KNOB_BDL_DOOR_A_e..H_e = 0xA..0x11)
    // -> member names (residmap/donor Knob.h verified: 0xA=door_a.bdl .. 0x11=door_h.bdl).
    const char* fileName;
    switch (getShapeType()) {
        case 1:
            fileName = "door_b.bdl";
            break;

        case 2:
            fileName = "door_c.bdl";
            break;

        case 3:
            fileName = "door_d.bdl";
            break;

        case 4:
            fileName = "door_e.bdl";
            break;

        case 5:
            fileName = "door_f.bdl";
            break;

        case 6:
            fileName = "door_g.bdl";
            break;

        case 7:
            fileName = "door_h.bdl";
            break;

        default:
            fileName = "door_a.bdl";
            break;
    }

    modelData = dExtNpcMount_acquireModelData(M_arcname, fileName);  // §328 DN-3
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §328 CreateHeap: '{}' member '{}' not resolvable", M_arcname, fileName);
        return FALSE;
    }
    mpModel2 = mDoExt_J3DModel__create(modelData, 0x80000, 0x11020002);
    if (mpModel2 == NULL) {
        return FALSE;  // §328 retail side
    }

    mpBgW = JKR_NEW dBgW();  // §328 donor `new` -> JKR_NEW (solid-heap idiom)
    if (mpBgW == NULL) {
        return FALSE;
    }

    // §328 donor id dRes_INDEX_KNOB_DZB_DOOR_e == 0x14 == "door.dzb" (raw cBgD_t
    // — collision data, not J3D; the raw getObjectRes bytes ARE the format).
    cBgD_t* bgd = (cBgD_t*)dComIfG_getObjectRes(M_arcname, "door.dzb");

    if (bgd == NULL) {
        return FALSE;
    }
    // §334: WW dzb → TP attribute repack (bus §332/§333) before Set, same as
    // every WW-sourced collision consumer (idempotent, arcs stay verbatim).
    dExtWw_repackDzbAttributes(bgd, "knob00:door.dzb");

    calcMtx();

    // §328 DN-1 audit: donor-verbatim MOVE_BG Set with the model matrix — NO
    // SetRoomId is ever stamped on this BgW (here or anywhere in this actor);
    // the room stays at the daBg DEFAULT resolution (DN-1's sanctioned
    // leave-the-default path). Owner identity is `this` at Regist below.
    if (mpBgW->Set(bgd, cBgW::MOVE_BG_e, &mpModel->getBaseTRMtx()) != 1) {
        return TRUE;
    }
    // §329 seam 5 — port-required BG mover: on this port's dBgS, a MOVE_BG
    // owner needs the Typical CrrFunc for actors to ride/track the moving door
    // collision (TP door idiom; the §27 stand-in and TP knob20 both set it —
    // WW's dBgW installed equivalent handling inside Set, the port's does not).
    mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
    return FALSE;
}

/* 000003B8-000003D8       .text getShapeType__10daKnob00_cFv */
u8 daKnob00_c::getShapeType() {
    return getSwbit2();
}

/* 000003D8-000004B0       .text setEventPrm__10daKnob00_cFv */
void daKnob00_c::setEventPrm() {
    if (mFrontCheck == 0) {
        m2C6 = 7;
    } else {
        m2C6 = 8;
    }

    if ((mAction == 4) || (mAction == 5) || (mAction == 8) || (mAction == 10)) {
        m2C6 = 9;
    }

    if (!checkArea(SQUARE(80.0f), SQUARE(110.0f), SQUARE(250.0f))) {
        offFlag(4);
    } else {
        eventInfo.setEventId(mEventIdx[m2C6]);
        eventInfo.setMapToolId(mToolId[m2C6]);  // §328 donor setToolId -> port setMapToolId
        eventInfo.onCondition(dEvtCnd_CANDOOR_e);
    }
}

/* 000004B0-000004BC       .text getType2__10daKnob00_cFv */
u8 daKnob00_c::getType2() {
    return fopAcM_GetParam(this) >> 0x1c;
}

/* 000004BC-00000568       .text chkPassward__10daKnob00_cFv */
s32 daKnob00_c::chkPassward() {
    char acStack_24[17];
    char acStack_38[17];

    // §328 RETAIL (VERSION > VERSION_JPN) side of the donor VERSION blocks kept.
    strcpy(acStack_24, dComIfGp_getInputPassword());

    if (!dComIfGs_isEventBit(WWEV_UNK_2110)) {  // §328 donor dSv_event_flag_c::UNK_2110 (routed)
        return -1;
    }

    s32 uVar3 = dComIfGs_getEventReg(WWEV_UNK_BA0F);  // §328 donor UNK_BA0F (routed)
    fopMsgM_passwordGet(acStack_38, uVar3 + 0x1b37);  // §328 BRIDGE-OWED stub (see banner)
    daKnob00_charactorExchange(acStack_38);
    daKnob00_charactorExchange(acStack_24);

    if (strcmp(acStack_38, acStack_24) != 0) {
        return -1;
    }
    return uVar3;
}

/* 00000568-000005DC       .text msgDoor__10daKnob00_cFv */
BOOL daKnob00_c::msgDoor() {
    if (this->mDoorType == 1) {
        return 1;
    }

    if (dKy_daynight_check() == dKy_TIME_NIGHT_e) {
        if (mDoorType == 2) {
            return 1;
        }
    } else if (mDoorType == 3) {
        return 1;
    }
    return FALSE;
}

/* 000005DC-000006F0       .text openInit__10daKnob00_cFi */
void daKnob00_c::openInit(int arg1) {
    // §328 donor bck_table of res ids {ADOOR,BDOOR,ADOOR,BDOOR} -> member names
    // (5 = dooropenadoor.bck, 6 = dooropenbdoor.bck).
    static DUSK_CONSTEXPR char DUSK_CONST* bck_table[] = { "dooropenadoor.bck", "dooropenbdoor.bck",
                                                           "dooropenadoor.bck", "dooropenbdoor.bck" };

    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(M_arcname, bck_table[arg1]);
    // §328 donor init(modelData, anm, true, EMode_NONE, 1.0f, 0, -1, true) ->
    // port 7-arg; JUT_ASSERT(VERSION_SELECT(382,382,396,396), FALSE) -> log.
    s32 iVar2 = mBckAnm.init(anm, true, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, true);
    if (iVar2 == 0) {
        DuskLog.warn("[Knob00] §328 openInit: bck '{}' init failed", bck_table[arg1]);
    }

    openInitCom(0);

    onFlag(1);

    if (arg1 >= 2) {
        mBckAnm.setFrame(34.0f);
    } else {
        dComIfG_Bgsp().Release(mpBgW);  // §328 reference form (toripost recipe)
    }
}

/* 000006F0-000009F8       .text openProc__10daKnob00_cFi */
BOOL daKnob00_c::openProc(int arg1) {
    if (mBckAnm.play()) {
        return TRUE;
    }

    if ((arg1 == 0xc) || (arg1 == 0xd)) {
        u8 bVar3 = (dKy_daynight_check() == dKy_TIME_NIGHT_e);
        u8 bVar4 = (arg1 == 0xd);
        BOOL tmp;
        if (bVar4 == bVar3) {
            tmp = TRUE;
        } else {
            tmp = FALSE;
        }

        if (mBckAnm.checkFrame(15.0f)) {
            mDoGph_gInf_c::setFadeRate(0.0f);
            if (tmp) {
                mDoGph_gInf_c::fadeOut(0.05f, g_saftyWhiteColor);
            } else {
                mDoGph_gInf_c::fadeOut(0.05f, g_blackColor);
            }
        }
    }

    if (arg1 == 0xc || arg1 == 0xe) {
        if (mBckAnm.checkFrame(28.0f)) {
            fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_OPEN, 0);
        } else if (mBckAnm.checkFrame(60.0f)) {
            fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_CLOSE_1, 0);
        } else if (mBckAnm.checkFrame(67.0f) && arg1 == 0xe) {
            fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_CLOSE_2, 0);
        }
    } else if (mBckAnm.checkFrame(25.0f)) {
        fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_OPEN, 0);
    } else if (mBckAnm.checkFrame(49.0f)) {
        fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_CLOSE_1, 0);
    } else if (mBckAnm.checkFrame(64.0f) && arg1 == 0xf) {
        fopAcM_seStart(this, JA_SE_OBJ_DOOR_N_CLOSE_2, 0);
    }

    return FALSE;
}

/* 000009F8-00000A90       .text openEnd__10daKnob00_cFv */
void daKnob00_c::openEnd() {
    offFlag(1);
    // §328 DN-1 audit: re-Regist with owner `this`, no room stamp (donor path).
    // Donor JUT_ASSERT(VERSION_SELECT(473,473,487,487), !rt) -> log.
    bool rt = dComIfG_Bgsp().Regist(mpBgW, this);  // §328 reference form
    if (rt) {
        DuskLog.warn("[Knob00] §328 openEnd: Bgsp Regist failed");
    }

    dComIfGp_map_clrAGBMapSendStopFlg();
    closeEndCom();
    // ========================================================================
    // §329 seam 2 — port warp backend. The donor ends every door-open in place
    // (closeEndCom = restart-room bookkeeping only; WW stage exits ride the
    // door's SCLS index, which the port's host stages don't carry — §322b).
    // For §27 door-NETWORK doors (stamped key) the port's own destination
    // resolver takes over here, same call the stand-in made at its demo end.
    // Unstamped (DZR-native) doors: donor-pure, no warp.
    // ========================================================================
    if (mDoorKey[0] != '\0') {
        DuskLog.info("[Knob00] §329 native openEnd → warp key='{}'", mDoorKey);
        dExtNpcDoors_tryNativeWarp(this, /*openAlreadyDone=*/true);
    }
}

/* 00000A90-00000C04       .text chkException__10daKnob00_cFv */
BOOL daKnob00_c::chkException() {
    switch (home.angle.z) {
        case 0x6A6:
            if (dComIfGs_isSymbol(dSymbol_NAYRU_e)) {  // §328 shim -> FALSE (inert)
                return 1;
            }

            if (!dComIfGs_isEventBit(WWEV_UNK_2110)) {  // §328 donor UNK_2110 (routed)
                return 1;
            }
            break;

        case 0x6A7:
            if (!dComIfGs_isEventBit(WWEV_UNK_0520)) {  // §328 donor UNK_0520 (routed)
                return 1;
            }
            break;

        case 0x6A8:
            if (!dComIfGs_isEventBit(WWEV_ENDLESS_NIGHT)) {  // §328 donor ENDLESS_NIGHT (shims 0x0A02, routed)
                return 1;
            }

            if (dComIfGs_isEventBit(WWEV_UNK_2110)) {
                return 1;
            }
            break;

        case 0x6A9:
            if (dComIfGs_isEventBit(WWEV_UNK_1701)) {  // §328 donor UNK_1701 (routed)
                return 1;
            }
            break;

        case 0x6AA:
            if (!dComIfGs_isSymbol(dSymbol_DIN_e)) {  // §328 shim -> FALSE (inert)
                return 1;
            }

            if (dComIfGs_isEventBit(WWEV_UNK_1A80)) {  // §328 donor UNK_1A80 (shims, routed)
                return 1;
            }
            break;
    }

    return 0;
}

/* 00000C04-00000C68       .text calcMtx__10daKnob00_cFv */
void daKnob00_c::calcMtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

/* 00000C68-00000D84       .text CreateInit__10daKnob00_cFv */
BOOL daKnob00_c::CreateInit() {
    tevStr.room_no = current.roomNo;  // §328 mRoomNo -> room_no (port field name)
    setAction(0);

    attention_info.position.y += 150.0f;
    eyePos.y += 150.0f;
    attention_info.flags = fopAc_Attn_ACTION_DOOR_e;

    calcMtx();
    mpBgW->Move();
    initProc(0);

    mDoorType = getType();
    if (getShapeType() == 4 && mDoorType == 1) {
        mDoorType = 5;
        // §328 BRIDGE-OWED: donor spawns the WW villa tag here —
        //   fopAcM_create(fpcNm_TAG_MK_e, 0xffffff03, &current.pos,
        //                 fopAcM_GetRoomNo(this), &current.angle, NULL, 0, NULL);
        // fpcNm_TAG_MK_e (Mrs. Marie's villa tag) is NOT registered on this
        // port; creating an unknown proc name would deref a NULL profile, so
        // the call is skipped-and-logged until TAG_MK lands.
        DuskLog.warn("[Knob00] §328 BRIDGE-OWED: WW TAG_MK villa tag not ported — donor spawn skipped");
        attention_info.position.y += 60.0f;
        eyePos.y += 60.0f;
    }

    if (getShapeType() == 6) {
        mDoorType = 6;
        home.angle.z = 0x366a;
    }
    return TRUE;
}

/* 00000D84-00000E70       .text create__10daKnob00_cFv */
// ============================================================================
// §329 port-extension setter (see header): §27 door-network stamp surface.
// ============================================================================
void daKnob00_c::setDoorKey(const char* key) {
    std::snprintf(mDoorKey, sizeof(mDoorKey), "%s", key != NULL ? key : "");
}

cPhs_Step daKnob00_c::create() {  // §328 cPhs_State -> cPhs_Step
    // §328 donor fopAcM_ct_Retail/fopAcM_ct_Demo VERSION pair -> the port's
    // single fopAcM_ct, first line (TP knob20 / §27 idiom).
    fopAcM_ct(this, daKnob00_c);
    // ========================================================================
    // §329 seam 1 — consume the §27 door-network pending-spawn params (proc/
    // src/head/joint) exactly as the stand-in did, so a network-spawned door
    // carries its key ('door:<name>' / 'door:exit:<name>') and the doors poll
    // recognizes it. DZR-placed doors have no pending spawn → donor-pure.
    // ========================================================================
    if (mDoorKey[0] == '\0' && mSpawnSrc[0] == '\0') {
        char proc[32] = {};
        char src[96] = {};
        char head[64] = {};
        char joint[32] = {};
        if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(this), proc, sizeof(proc), src,
                                          sizeof(src), head, sizeof(head), joint,
                                          sizeof(joint))) {
            if (src[0]) {
                std::snprintf(mSpawnSrc, sizeof(mSpawnSrc), "%s", src);
                if (std::strncmp(src, "door:", 5) == 0) {
                    setDoorKey(src + 5);
                }
            }
            // ================================================================
            // §329c — donor param synthesis for NETWORK doors. The donor packs
            // front/back room ids into home.angle.x (6 bits each; d_door.cpp
            // getF/BRoomNo). A §27 spawn leaves angle.x = plain pitch (0), so
            // rooms decoded 0/0 → adjoinPlayer() false for a player in any
            // other room → drawCheck 0 → alive-but-invisible (the 17:26 "still
            // no doors" incident). The DONOR'S OWN sentinel for a roomless
            // door is 0x3F (adjoinPlayer: F or B == 0x3F ⇒ always adjoin;
            // drawCheck then lights by stay-room; frontCheck falls back to the
            // angle test) — synthesize F=B=0x3F. NOT an invented layout: this
            // is the donor's encoding for exactly this case. Real per-door
            // donor params (shape/rooms/events) arrive with the kit's DZR door
            // rows — tracked; network doors use the sentinel until then.
            // ================================================================
            home.angle.x = (s16)0x0FFF;  // F=0x3F | B=0x3F<<6
        }
    }
    cPhs_Step ret = dComIfG_resLoad(&mPhase, M_arcname);
    if (ret != cPhs_COMPLEATE_e) {
        // §329b: NEVER let a create failure be silent — the §27 poll respawns
        // failed doors forever, so a quiet ERROR here = "all doors disappeared"
        // with an empty log (the exact 17:11 incident).
        if (ret == cPhs_ERROR_e) {
            static int s_errWarn = 0;
            if ((s_errWarn++ % 300) == 0) {
                DuskLog.warn("[Knob00] §329b create: resLoad('{}') ERROR — door cannot exist",
                             M_arcname);
            }
        }
        return ret;
    }

    if (fopAcM_GetRoomNo(this) == -1) {
        // §328 DN-1 audit: the ACTOR's room from its own placement data
        // (getFRoomNo <- home.angle.x) — positively resolved, donor-verbatim.
        fopAcM_SetRoomNo(this, getFRoomNo());
    }

    // §328 donor heap size 0x2700 -> 0x80003800 (PC doubles the low 24 bits;
    // §27-proven value, == TP knob20's adjust-flag + size).
    if (!fopAcM_entrySolidHeap(this, CheckCreateHeap, 0x80003800)) {
        DuskLog.warn("[Knob00] §329b create: entrySolidHeap/CreateHeap FAILED "
                     "(shape={} key='{}')", (int)getShapeType(), mDoorKey);
        return cPhs_ERROR_e;
    }

    CreateInit();
    return cPhs_COMPLEATE_e;
}

/* 00000E70-00000F30       .text setStart__10daKnob00_cFff */
void daKnob00_c::setStart(float arg1, float arg2) {
    daPy_py_c* player = (daPy_py_c*)dComIfGp_getPlayer(0);
    cXyz sp14 = current.pos;
    s16 angle = shape_angle.y + 0x7FFF;

    sp14.x += cM_ssin(angle) * arg2 - cM_scos(angle) * arg1;
    sp14.z += cM_scos(angle) * arg2 + cM_ssin(angle) * arg1;

    // §328 port setPlayerPosAndAngle takes a trailing BOOL (knob20 idiom: 0).
    player->setPlayerPosAndAngle(&sp14, player->shape_angle.y, 0);
}

/* 00000F30-00000F4C       .text setAngle__10daKnob00_cFv */
void daKnob00_c::setAngle() {
    daPy_py_c* player = (daPy_py_c*)dComIfGp_getPlayer(0);
    player->changeDemoMoveAngle(shape_angle.y + 0x7FFF);
}

/* 00000F4C-0000108C       .text adjustmentProc__10daKnob00_cFv */
BOOL daKnob00_c::adjustmentProc() {
    daPy_py_c* player = (daPy_py_c*)dComIfGp_getPlayer(0);
    s16 angle = shape_angle.y + 0x7FFF;
    cXyz sp14;
    cXyz sp08;

    sp08 = player->current.pos;
    sp14 = current.pos;

    sp14.x += cM_ssin(angle) * -70.0f;
    sp14.z += cM_scos(angle) * -70.0f;

    if (m314 > 0) {
        sp08.x = sp08.x * 0.8f + sp14.x * 0.2f;
        sp08.z = sp08.z * 0.8f + sp14.z * 0.2f;
        player->setPlayerPosAndAngle(&sp08, player->current.angle.y, 0);  // §328 3-arg
        m314--;
    } else {
        player->setPlayerPosAndAngle(&sp14, player->current.angle.y, 0);  // §328 3-arg
        return TRUE;
    }
    return FALSE;
}

/* 0000108C-00001334       .text demoProc__10daKnob00_cFv */
BOOL daKnob00_c::demoProc() {
    s32 demoAction;
    BOOL ret = FALSE;

    demoAction = getDemoAction();

    if (dComIfGp_evmng_getIsAddvance(mStaffId)) {
        switch(demoAction) {
            case 9:
                setStart(0.0f, -70.0f);
                break;

            case 0x11:
                setStart(-43.0f, -14.8f);
                if (mpBgW->ChkUsed()) {
                    dComIfG_Bgsp().Release(mpBgW);  // §328 reference form
                }
                break;

            case 0x12:
                setStart(46.0f, -86.0f);
                if (mpBgW->ChkUsed()) {
                    dComIfG_Bgsp().Release(mpBgW);  // §328 reference form
                }
                break;

            case 0xc:
                openInit(0);
                break;

            case 0xd:
                openInit(1);
                break;

            case 0xe:
                openInit(2);
                break;

            case 0xf:
                openInit(3);
                break;

            case 10:
                setAngle();
                break;

            case 0xb:
                m314 = 10;
                break;

            case 0x10:
                m2D0.init(home.angle.z);
                break;
        }
    }

    switch (demoAction) {
        case 12:
        case 13:
        case 14:
        case 15:
            if (checkFlag(1)) {
                if (openProc(demoAction)) {
                    openEnd();
                    dComIfGp_evmng_cutEnd(mStaffId);
                }
            } else {
                dComIfGp_evmng_cutEnd(mStaffId);
            }
            break;

        case 11:
            if (adjustmentProc()) {
                dComIfGp_evmng_cutEnd(mStaffId);
            }
            break;

        case 16:
            if (mAction == 9) {
                // §328 Mt is unported (stub above): searchFromName finds no
                // live "Mt" -> donor NULL side runs.
                daNpcMt_c* mt = (daNpcMt_c*)fopAcM_searchFromName("Mt", 0, 0);
                if (mt != NULL) {
                    if (m2D0.proc(&mt->attention_info.position)) {
                        ret = 1;
                    }
                } else if (m2D0.proc(&attention_info.position)) {
                    ret = 1;
                }
            } else if (m2D0.proc(&attention_info.position)) {
                ret = 1;
            }
            break;

        default:
            dComIfGp_evmng_cutEnd(mStaffId);
            break;
    }

    return ret;
}

/* 00001334-00001398       .text demoProc2__10daKnob00_cFv */
BOOL daKnob00_c::demoProc2() {
    BOOL ret = FALSE;
    switch (getDemoAction()) {
        case 16:
            ret = TRUE;
            break;

        default:
            dComIfGp_evmng_cutEnd(mStaffId);
            break;
    }
    return ret;
}

/* 00001398-000013F0       .text actionWait__10daKnob00_cFv */
BOOL daKnob00_c::actionWait() {
    if (eventInfo.checkCommandDoor()) {
        initOpenDemo(0);
        setAction(2);
        demoProc();
    } else {
        setEventPrm();
    }
    return TRUE;
}

/* 000013F0-00001488       .text actionDemo__10daKnob00_cFv */
BOOL daKnob00_c::actionDemo() {
    if (dComIfGp_evmng_endCheck(mEventIdx[m2C6])) {
        if (m2C6 != 7 && m2C6 != 8) {
            setAction(1);
            dComIfGp_event_reset();
            shape_angle.y = current.angle.y;
        }
    } else {
        demoProc();
    }
    return TRUE;
}

/* 00001488-0000151C       .text actionTalk__10daKnob00_cFv */
BOOL daKnob00_c::actionTalk() {
    if (demoProc()) {
        if (mAction == 9) {
            daNpcMt_c* mt = (daNpcMt_c*)fopAcM_searchFromName("Mt", 0, 0);
            if (mt != NULL) {
                mt->attnOff();
            }
        }
        nextAction();
        dComIfGp_event_reset();
        shape_angle.y = current.angle.y;
    }
    return TRUE;
}

/* 0000151C-00001574       .text actionTalkWait__10daKnob00_cFv */
BOOL daKnob00_c::actionTalkWait() {
    if (eventInfo.checkCommandDoor()) {
        initOpenDemo(0);
        setAction(3);
        demoProc();
    } else {
        setEventPrm();
    }
    return TRUE;
}

/* 00001574-000017AC       .text actionPassward2__10daKnob00_cFv */
BOOL daKnob00_c::actionPassward2() {
    switch (m316) {
        case 0:
            if (demoProc2()) {
                m316++;
                if (!dComIfGs_isEventBit(WWEV_UNK_3B20)) {  // §328 donor UNK_3B20 (routed)
                    dComIfGs_onEventBit(WWEV_UNK_3B20);
                    dComIfGs_setEventReg(WWEV_UNK_BA0F, (s8)cM_rndF(6.0f));  // §328 donor UNK_BA0F (routed)
                }
                int tmp = dComIfGs_getEventReg(WWEV_UNK_BA0F);
                int tmp2 = tmp;
                m2D0.init(tmp2 + 0x1b1a);
            }
            break;

        case 1:
            if (m2D0.proc(&attention_info.position)) {
                m316++;
                dComIfGp_InputPasswordOpenOn();  // §328 BRIDGE-OWED stub (see banner)
            }
            break;

        case 2:
            switch (dComIfGp_InputPasswordOpenCheck()) {  // §328 BRIDGE-OWED stub -> 0 (cancel)
                case 1:
                    if (chkPassward() == -1) {
                        m316 = 10;
                        m2D0.init(0x1b19);
                    } else {
                        dComIfGs_onEventBit(WWEV_UNK_1910);  // §328 donor UNK_1910 (routed)
                        m316++;
                        m2D0.init(0x1b18);
                    }
                    break;

                case 0:
                    m316 = 10;
                    m2D0.init(0x1b19);
                    break;
            }
            break;

        case 3:
            if (m2D0.proc(&attention_info.position)) {
                setAction(1);
                dComIfGp_event_reset();
                shape_angle.y = current.angle.y;
            }
            break;

        case 10:
            if (m2D0.proc(&attention_info.position)) {
                setAction(5);
                dComIfGp_event_reset();
                shape_angle.y = current.angle.y;
            }
            break;
    }
    return TRUE;
}

/* 000017AC-00001830       .text actionVilla__10daKnob00_cFv */
BOOL daKnob00_c::actionVilla() {
    if (eventInfo.checkCommandDoor()) {
        initOpenDemo(0);
        setAction(7);
        demoProc();
    } else if (dComIfGs_isEventBit(WWEV_UNK_2D80)) {  // §328 donor UNK_2D80 (routed)
        setAction(1);
    } else {
        setEventPrm();
    }
    return TRUE;
}

/* 00001830-00001890       .text actionPassward__10daKnob00_cFv */
BOOL daKnob00_c::actionPassward() {
    if (eventInfo.checkCommandDoor()) {
        initOpenDemo(0);
        setAction(6);
        actionPassward2();
        m316 = 0;
    } else {
        setEventPrm();
    }
    return TRUE;
}

/* 00001890-00001A24       .text actionInit__10daKnob00_cFv */
BOOL daKnob00_c::actionInit() {
    if (!mpBgW->ChkUsed()) {
        // §328 DN-1 audit: Regist(owner=this), no room stamp (donor path).
        dComIfG_Bgsp().Regist(mpBgW, this);  // §328 reference form
    }

    if (mDoorType == 6 && !dComIfGs_isEventBit(WWEV_UNK_3401)) {  // §328 donor UNK_3401 (routed)
        actionFigure();
        setAction(10);
        return TRUE;
    }

    if (mDoorType == 5 && !dComIfGs_isEventBit(WWEV_UNK_2D80)) {
        actionVilla();
        setAction(8);
        return TRUE;
    }

    if (mDoorType == 4) {
        if (!dComIfGs_isEventBit(WWEV_UNK_1910)) {
            setAction(5);
            actionPassward();
            return TRUE;
        }

        if (dComIfGs_checkGetItem(dItemNo_BOMB_BAG_e)) {
            setAction(11);
            return TRUE;
        }
    }

    if (msgDoor() && !chkException()) {
        actionTalkWait();
        setAction(4);
        return TRUE;
    }

    actionWait();
    setAction(1);
    return TRUE;
}

/* 00001A24-00001B04       .text actionFigure__10daKnob00_cFv */
BOOL daKnob00_c::actionFigure() {
    if (eventInfo.checkCommandDoor()) {
        daNpcMt_c* mt = (daNpcMt_c*)fopAcM_searchFromName("Mt", 0, 0);
        if (mt != NULL) {
            dComIfGp_event_setTalkPartner(mt);
            fopAcM_OnStatus(mt, fopAcStts_UNK800_e);
            mt->attnOn();
        }

        initOpenDemo(0);
        setAction(9);
        demoProc();
    } else if (dComIfGs_isEventBit(WWEV_UNK_3401)) {
        setAction(1);
    } else {
        setEventPrm();
    }
    return TRUE;
}

/* 00001B04-00001B0C       .text actionDead__10daKnob00_cFv */
BOOL daKnob00_c::actionDead() {
    return TRUE;
}

/* 00001B0C-00001CA0       .text draw__10daKnob00_cFv */
BOOL daKnob00_c::draw() {
    // §328 RETAIL (#else) side of the donor VERSION_DEMO block kept.
    if (drawCheck(0) == 0) {
        if (getShapeType() == 1) {
            fopAcM_OffStatus(this, fopAcStts_SHOWMAP_e);
        }
        return TRUE;
    }

    if (getShapeType() == 1) {
        fopAcM_OffStatus(this, fopAcStts_SHOWMAP_e);
    }

    fpc_ProcID iVar1 = fopAcM_GetLinkId(this);
    if (iVar1 != fpcM_ERROR_PROCESS_ID_e) {
        fopAc_ac_c* ac = fopAcM_SearchByID(iVar1);
        // §328 pirate ship unported: the name sentinel never matches -> inert.
        if (fopAcM_IsActor(ac) && fopAcM_GetName(ac) == fpcNm_Obj_Pirateship_e) {
            daObjPirateship::Act_c* pirateShip = (daObjPirateship::Act_c*)ac;
            if (pirateShip->m2CC == 0) {
                return TRUE;
            }
        }
    }

    if (getShapeType() == 5) {
        g_env_light.settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    } else {
        g_env_light.settingTevStruct(TEV_TYPE_BG0, &current.pos, &tevStr);
    }

    g_env_light.setLightTevColorType(mpModel2, &tevStr);
    J3DModelData* modelData = mpModel->getModelData();
    mBckAnm.entry(modelData);
    mpModel->calc();
    mpModel2->setBaseTRMtx(mpModel->getAnmMtx(m_jnt));
    mDoExt_modelUpdateDL(mpModel2);
    return TRUE;
}

/* 00001CA0-00001CC0       .text daKnob00_Draw__FP10daKnob00_c */
static BOOL daKnob00_Draw(daKnob00_c* i_this) {
    return ((daKnob00_c*)i_this)->draw();
}

BOOL daKnob00_c::execute() {
    switch(checkExecute()) {
        case 0:
            setAction(0);
            break;

        case 2:
            switch(mAction) {
                case 0:
                    actionInit();
                    break;

                case 1:
                    actionWait();
                    break;

                case 2:
                    actionDemo();
                    break;

                case 4:
                    actionTalkWait();
                    break;

                case 3:
                    actionTalk();
                    break;

                case 5:
                    actionPassward();
                    break;

                case 6:
                    actionPassward2();
                    break;

                case 8:
                    actionVilla();
                    break;

                case 7:
                    actionTalk();
                    break;

                case 10:
                    actionFigure();
                    break;

                case 9:
                    actionTalk();
                    break;

                default:
                    actionDead();
                    break;
            }
            break;

        case 1:
            startDemoProc();
            demoProc();
            break;

        default:
            // §328 donor JUT_ASSERT(VERSION_SELECT(1169,1169,1183,1183), FALSE)
            // — retail no-op; log keeps the port faithful without panicking.
            DuskLog.warn("[Knob00] §328 execute: unexpected checkExecute result");
            break;

    }

    mRoomNo2 = dComIfGp_roomControl_getStayNo();
    if (getType2() == 1) {
        calcMtx();
        if (mpBgW != NULL && mpBgW->ChkUsed()) {
            mpBgW->Move();
        }
    }
    return TRUE;
}


/* 00001CC0-00001E78       .text daKnob00_Execute__FP10daKnob00_c */
static BOOL daKnob00_Execute(daKnob00_c* i_this) {
    return i_this->execute();
}

/* 00001E78-00001E80       .text daKnob00_IsDelete__FP10daKnob00_c */
static BOOL daKnob00_IsDelete(daKnob00_c*) {
    return TRUE;
}

/* 00001E80-00001F50       .text daKnob00_Delete__FP10daKnob00_c */
static BOOL daKnob00_Delete(daKnob00_c* i_this) {
    // §328 RETAIL (VERSION > VERSION_DEMO) heap guard kept.
    if (i_this->heap != NULL)
    {
        if (i_this->mpBgW != NULL && i_this->mpBgW->ChkUsed()) {
            dComIfG_Bgsp().Release(i_this->mpBgW);  // §328 reference form
        }
    }

    // §328 donor dComIfG_resDeleteDemo -> the port's dComIfG_resDelete (the
    // demo-heap flavor does not exist on this port; same arc-release semantics).
    dComIfG_resDelete(&i_this->mPhase, daKnob00_c::M_arcname);

    // §328 donor explicit `i_this->~daKnob00_c();` omitted — the port framework
    // owns actor lifetime like its TP siblings (knob20 performs no manual dtor;
    // calling it here would double-destruct on this port).
    return TRUE;
}

/* 00001F50-00001F70       .text daKnob00_Create__FP10fopAc_ac_c */
static cPhs_Step daKnob00_Create(fopAc_ac_c* i_this) {  // §328 cPhs_State -> cPhs_Step
    return ((daKnob00_c*)i_this)->create();
}

static DUSK_CONST actor_method_class l_daKnob00_Method = {
    (process_method_func)daKnob00_Create,
    (process_method_func)daKnob00_Delete,
    (process_method_func)daKnob00_Execute,
    (process_method_func)daKnob00_IsDelete,
    (process_method_func)daKnob00_Draw,
};

// §328 definition must match the port's extern decl (f_pc_profile_lst.h:
// `extern actor_process_profile_definition DUSK_CONST g_profile_KNOB00;`) —
// DUSK_PROFILE/DUSK_CONST added; every field value is donor-verbatim except the
// draw-prio slot (absent on the port -> knob-door slot, see local define).
DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_KNOB00 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_KNOB00_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daKnob00_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB00_e,  // §328 donor slot absent -> port knob-door slot
    /* Actor SubMtd */ &l_daKnob00_Method,
    /* Status       */ fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,  // §328 WW names, shims values (0x4000|0x40000)
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_6_e,
};

#else  // DUSK_WW_KNOB00_NATIVE == 0 — §27 stand-in (unchanged below)

/**
 * §27 first decomp port (WW regular knob door).
 *
 * Minimal Outset cut: two-model swing + DoorK10 DEFAULT_KNOB_* + ExtNpc warp.
 * Omitted (quest-only): password, villa, Mt figure, pirateship, msgDoor, WW event bits.
 * No res/Object/Knob.h — string resource names only (§27 D4).
 *
 * WW Knob BDLs are bmd3 — must go through dExtNpcMount_acquireModelData (same as
 * NPC_KNOB). Raw dComIfG_getObjectRes cast → J3DModelData* AVs (log 021350).
 */

#if TARGET_PC

#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_mount.h"
#include "d/d_kankyo.h"
#include "d/d_meter2_info.h"
#include "dusk/logging.h"
#include "f_op/f_op_actor_mng.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "m_Do/m_Do_mtx.h"
#include <cstring>

namespace {

static const char* const kArc = "Knob";  // mod arcs/Knob.arc → Object/Knob.arc
static const char* const kEvArc = "DoorK10";

static int CheckCreateHeap(fopAc_ac_c* i_this) {
    return static_cast<daKnob00_c*>(i_this)->CreateHeap();
}

}  // namespace

void daKnob00_c::setDoorKey(const char* key) {
    if (key == NULL) {
        mDoorKey[0] = '\0';
        return;
    }
    std::snprintf(mDoorKey, sizeof(mDoorKey), "%s", key);
}

int daKnob00_c::CreateHeap() {
    // Controller (tiny joint host) + visible door_a — match npc_knob.ini.
    // Outset spawn params encode doorIndex in bits 8+, NOT WW shape type.
    J3DModelData* modelData = dExtNpcMount_acquireModelData(kArc, "door.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §27 missing/unparseable door.bdl in '{}'", kArc);
        return 0;
    }
    mpModel = mDoExt_J3DModel__create(modelData, 0, 0x11020203);
    if (mpModel == NULL) {
        DuskLog.warn("[Knob00] §27 door.bdl model create failed");
        return 0;
    }
    mpModel->setBaseScale(scale);
    calcMtx();

    J3DAnmTransform* anm =
        (J3DAnmTransform*)dComIfG_getObjectRes(kArc, "dooropenadoor.bck");
    if (anm == NULL || mBck.init(anm, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, false) == 0) {
        DuskLog.warn("[Knob00] §27 dooropenadoor.bck init failed");
        return 0;
    }

    JUTNameTab* names = modelData->getJointName();
    mJoint = names != NULL ? names->getIndex("DoorDummy") : -1;
    if (mJoint < 0) {
        DuskLog.warn("[Knob00] §27 DoorDummy joint missing");
        return 0;
    }

    modelData = dExtNpcMount_acquireModelData(kArc, "door_a.bdl");
    if (modelData == NULL) {
        DuskLog.warn("[Knob00] §27 visual door_a.bdl missing/unparseable");
        return 0;
    }
    mpModel2 = mDoExt_J3DModel__create(modelData, 0x80000, 0x11020002);
    if (mpModel2 == NULL) {
        DuskLog.warn("[Knob00] §27 door_a.bdl model create failed");
        return 0;
    }

    cBgD_t* bgd = (cBgD_t*)dComIfG_getObjectRes(kArc, "door.dzb");
    if (bgd == NULL) {
        DuskLog.warn("[Knob00] §27 door.dzb missing — continue without BG");
        mpBgW = NULL;
        return 1;
    }
    mpBgW = JKR_NEW dBgW();
    if (mpBgW == NULL) {
        return 0;
    }
    mBck.entry(mpModel->getModelData());
    mpModel->calc();
    calcMtx();
    if (mpBgW->Set(bgd, cBgW::MOVE_BG_e, &mpModel->getBaseTRMtx()) == 1) {
        DuskLog.warn("[Knob00] §27 door.dzb Set failed");
        return 0;
    }
    mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
    return 1;
}

void daKnob00_c::calcMtx() {
    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    mDoMtx_stack_c::YrotM(current.angle.y);
    if (mpModel != NULL) {
        mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
    }
    MTXCopy(mDoMtx_stack_c::get(), mBgMtx);
}

bool daKnob00_c::bindEvents() {
    if (mEvBound) {
        return mEvtFront >= 0 || mEvtBack >= 0;
    }
    int st = dComIfG_resLoad(&mEvPhase, kEvArc);
    if (st != cPhs_COMPLEATE_e) {
        return false;
    }
    eventInfo.setArchiveName(kEvArc);
    mEvtFront = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_KNOB_DOOR_F_OPEN", 0xff);
    mEvtBack = dComIfGp_getEventManager().getEventIdx(this, "DEFAULT_KNOB_DOOR_B_OPEN", 0xff);
    mEvBound = 1;
    mEvtOrdered = -1;
    mStaffId = -1;
    DuskLog.info("[Knob00] §27 DoorK10 bound F={} B={}", (int)mEvtFront, (int)mEvtBack);
    return mEvtFront >= 0 || mEvtBack >= 0;
}

int daKnob00_c::frontCheck() const {
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player == NULL) {
        return 0;
    }
    cXyz playerDist = player->current.pos - current.pos;
    mDoMtx_stack_c::YrotS(-current.angle.y);
    mDoMtx_stack_c::multVec(&playerDist, &playerDist);
    return playerDist.z > 0.0f ? 0 : 1;
}

void daKnob00_c::setEventPrm() {
    if (!bindEvents()) {
        return;
    }
    const int side = frontCheck();
    const s16 evt = (side == 0) ? mEvtFront : mEvtBack;
    if (evt < 0) {
        return;
    }
    mEvtOrdered = evt;
    eventInfo.setArchiveName(kEvArc);
    eventInfo.setEventId(evt);
    eventInfo.setMapToolId(0xff);
    eventInfo.onCondition(dEvtCnd_CANDOOR_e);
}

int daKnob00_c::getDemoAction() {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[16] = {
        "WAIT",           "SETSTART",      "SETANGLE",       "ADJUSTMENT",
        "OPEN_PUSH",      "OPEN_PULL",     "OPEN_PUSH2",     "OPEN_PULL2",
        "OPEN_PUSH_STOP", "OPEN_PULL_STOP","TALK",           "TALK_END",
        "SETSTART_PUSH",  "SETSTART_PULL", "DEMO_OPEN",      "DEMO_CLOSE",
    };
    return dComIfGp_evmng_getMyActIdx(mStaffId, action_table, 16, 0, 0);
}

void daKnob00_c::startOpenAnim(int side) {
    if (mOpenStarted || mpModel == NULL) {
        return;
    }
    const char* bck = (side == 0) ? "dooropenadoor.bck" : "dooropenbdoor.bck";
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(kArc, bck);
    if (anm == NULL) {
        anm = (J3DAnmTransform*)dComIfG_getObjectRes(kArc, "dooropenadoor.bck");
    }
    if (anm == NULL) {
        return;
    }
    mBck.init(anm, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1, true);
    mOpenStarted = 1;
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
    }
    DuskLog.info("[Knob00] §27 OPEN anim '{}'", bck);
}

bool daKnob00_c::openAnimDone() {
    if (!mOpenStarted) {
        return false;
    }
    return mBck.play() != 0;
}

void daKnob00_c::initOpenDemo() {
    mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
    mOpenStarted = 0;
    mAction = ACTION_DEMO;
    DuskLog.info("[Knob00] §27 demo BEGIN staff={} evt={}", mStaffId, (int)mEvtOrdered);
}

void daKnob00_c::demoProc() {
    if (mStaffId == -1) {
        return;
    }
    const int demoAction = getDemoAction();
    const int side = frontCheck();
    if (dComIfGp_evmng_getIsAddvance(mStaffId) != 0) {
        switch (demoAction) {
        case 4:
        case 5:
        case 6:
        case 7:
            startOpenAnim(side);
            break;
        default:
            break;
        }
    }
    switch (demoAction) {
    case 4:
    case 5:
    case 6:
    case 7:
        if (!mOpenStarted) {
            startOpenAnim(side);
            if (!mOpenStarted) {
                dComIfGp_evmng_cutEnd(mStaffId);
            }
            break;
        }
        if (openAnimDone()) {
            dComIfGp_evmng_cutEnd(mStaffId);
        }
        break;
    default:
        dComIfGp_evmng_cutEnd(mStaffId);
        break;
    }
}

int daKnob00_c::create() {
    fopAcM_ct(this, daKnob00_c);
    mpModel = NULL;
    mpModel2 = NULL;
    mpBgW = NULL;
    mJoint = -1;
    mAction = ACTION_WAIT;
    mEvBound = 0;
    mOpenStarted = 0;
    mArcRetained = 0;
    mEvtFront = -1;
    mEvtBack = -1;
    mEvtOrdered = -1;
    mStaffId = -1;
    mDoorKey[0] = '\0';
    mSpawnSrc[0] = '\0';

    char proc[32] = {};
    char src[96] = {};
    char head[64] = {};
    char joint[32] = {};
    if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(this), proc, sizeof(proc), src, sizeof(src),
                                      head, sizeof(head), joint, sizeof(joint))) {
        if (src[0]) {
            std::snprintf(mSpawnSrc, sizeof(mSpawnSrc), "%s", src);
            if (std::strncmp(src, "door:", 5) == 0) {
                setDoorKey(src + 5);
            }
        }
    }

    int phase = dComIfG_resLoad(&mPhase, kArc);
    if (phase == cPhs_ERROR_e) {
        DuskLog.warn("[Knob00] §27 arc '{}' load ERROR", kArc);
        return cPhs_ERROR_e;
    }
    if (phase != cPhs_COMPLEATE_e) {
        return phase;
    }
    // Match daKnob20: adjust flag + size (PC doubles the low 24 bits).
    if (!fopAcM_entrySolidHeap(this, CheckCreateHeap, 0x80003800)) {
        DuskLog.warn("[Knob00] §27 entrySolidHeap FAILED key='{}'", mDoorKey);
        return cPhs_ERROR_e;
    }
    dExtNpcMount_retainArc(kArc);
    mArcRetained = 1;
    if (mpBgW != NULL && dComIfG_Bgsp().Regist(mpBgW, this)) {
        DuskLog.warn("[Knob00] §27 Bgsp Regist failed");
    }
    attention_info.position = current.pos;
    attention_info.position.y += 150.0f;
    eyePos = attention_info.position;
    attention_info.flags = fopAc_AttnFlag_DOOR_e;
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    fopAcM_SetMin(this, -200.0f, 0.0f, -200.0f);
    fopAcM_SetMax(this, 200.0f, 300.0f, 200.0f);
    DuskLog.info("[Knob00] §27 COMPLEATE key='{}' src='{}'", mDoorKey, mSpawnSrc);
    return cPhs_COMPLEATE_e;
}

int daKnob00_c::execute() {
    bindEvents();
    calcMtx();
    if (mpBgW != NULL) {
        mpBgW->Move();
    }

    if (mAction == ACTION_DEMO) {
        dMeter2Info_onGameStatus(2);
        if (mEvtOrdered >= 0 && dComIfGp_evmng_endCheck(mEvtOrdered)) {
            mAction = ACTION_WAIT;
            mStaffId = -1;
            dComIfGp_event_reset();
            DuskLog.info("[Knob00] §27 demo END → warp key='{}'", mDoorKey);
            dExtNpcDoors_tryNativeWarp(this, /*openAlreadyDone=*/true);
            mEvtOrdered = -1;
            mOpenStarted = 0;
            if (mpBgW != NULL) {
                dComIfG_Bgsp().Regist(mpBgW, this);
            }
            return 1;
        }
        mStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
        demoProc();
        return 1;
    }

    if (eventInfo.checkCommandDoor()) {
        if (mEvtOrdered < 0) {
            setEventPrm();
        }
        initOpenDemo();
        return 1;
    }

    // Arm Open prompt when Link is near (TP linear area ≈ №91).
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player != NULL) {
        const f32 dx = player->current.pos.x - current.pos.x;
        const f32 dz = player->current.pos.z - current.pos.z;
        if (dx * dx + dz * dz < 110.0f * 110.0f) {
            setEventPrm();
        }
    }
    return 1;
}

int daKnob00_c::draw() {
    if (mpModel == NULL) {
        return 1;
    }
    g_env_light.settingTevStruct(0x10, &current.pos, &tevStr);
    g_env_light.setLightTevColorType_MAJI(mpModel, &tevStr);
    mBck.entry(mpModel->getModelData());
    mDoExt_modelUpdateDL(mpModel);
    mBck.remove(mpModel->getModelData());
    if (mpModel2 != NULL && mJoint >= 0) {
        mpModel2->setBaseTRMtx(mpModel->getAnmMtx(mJoint));
        g_env_light.setLightTevColorType_MAJI(mpModel2, &tevStr);
        mDoExt_modelUpdateDL(mpModel2);
    }
    return 1;
}

int daKnob00_c::Delete() {
    if (mpBgW != NULL) {
        dComIfG_Bgsp().Release(mpBgW);
        mpBgW = NULL;
    }
    dComIfG_resDelete(&mPhase, kArc);
    if (mArcRetained) {
        dExtNpcMount_releaseArc(kArc);
        mArcRetained = 0;
    }
    if (mEvBound) {
        dComIfG_resDelete(&mEvPhase, kEvArc);
    }
    return 1;
}

static int daKnob00_Create(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->create();
}
static int daKnob00_Delete(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->Delete();
}
static int daKnob00_Execute(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->execute();
}
static int daKnob00_Draw(void* i_this) {
    return static_cast<daKnob00_c*>(i_this)->draw();
}
static int daKnob00_IsDelete(void* i_this) {
    (void)i_this;
    return 1;
}

static DUSK_CONST actor_method_class l_daKnob00_Method = {
    (process_method_func)daKnob00_Create,  (process_method_func)daKnob00_Delete,
    (process_method_func)daKnob00_Execute, (process_method_func)daKnob00_IsDelete,
    (process_method_func)daKnob00_Draw,
};

DUSK_PROFILE actor_process_profile_definition DUSK_CONST g_profile_KNOB00 = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 7,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_KNOB00_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daKnob00_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_KNOB20_e,
    /* Actor SubMtd */ &l_daKnob00_Method,
    /* Status       */ fopAcStts_UNK_0x40000_e | fopAcStts_UNK_0x4000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_CUSTOM_e,
};

#endif  // TARGET_PC

#endif  // DUSK_WW_KNOB00_NATIVE
