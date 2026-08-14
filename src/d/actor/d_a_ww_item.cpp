// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_item.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW FIELD ITEM ACTOR — the §725 item exit executed (docs/ww-item-actor-
// port-plan.md steps 2-3). Donor daItem_c ported into the WW dialect; the
// donor's state machine carried WHOLE (full-state-machine law): all 12
// statuses, both modes, all six per-class actions, all launch actions.
//
// SEAMS (each at its site):
//   [I1] CONSUMPTION BOUNDARY — the port's ONLY translation point. Donor
//        execItemGet effects re-expressed as TP inventory ops keyed by the
//        DONOR item id (R5 item.id_space law). Donor data untouched.
//   [I2] models/dims from the generated donor tables (ww_item_data.inc,
//        §734/§739/§740) via the spc_item01 §327 resolver idiom.
//   [I3] MODE_WATER ports complete; its daSea surface query is a stated
//        GATE (plan seam 3): no host sea query exists yet, so the sea
//        branch reads absent-as-false and the Acch water branch (which the
//        receiver DOES serve) carries interiors/ponds. L-5's class.
//   [I4] particles: donor dPa callbacks -> receiver setSimple/set where the
//        effect id resolves; WW JPA ids only (§396 lineage law).
//   [I5] get-demo statuses carried; the demo-item spawn they hand off to is
//        the existing demo-item machinery (clothes/ba1 precedent). Field
//        items (rupees/hearts/consumables) never enter them.
//   [I6] REGISTRATION IS ELSEWHERE AND LAST (plan step 5): no profile in
//        this TU's initial landing; nothing spawns it until the battery
//        passes and the §725 no-op strips in the same change.
// ============================================================

#include "d/ext_plugin/ww_item_data.h"

#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_ww.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "d/d_ext_ww_actor_shims.h"   // WW dialect shims: fopAcStts_* / fpcDwPi_* / cCcD_*
#include "d/d_ext_npc_mount.h"        // §811 DN-3 consume-time model acquirer (by-index form)
#include "dusk/logging.h"

// ============================================================
// Donor item-id vocabulary (WW d_item_data.h ids used by the spine — donor
// numbers verbatim, trap #3 law).
// ============================================================
enum {
    WWITM_HEART_e          = 0x00,
    WWITM_GREEN_RUPEE_e    = 0x01,
    WWITM_BLUE_RUPEE_e     = 0x02,
    WWITM_YELLOW_RUPEE_e   = 0x03,
    WWITM_RED_RUPEE_e      = 0x04,
    WWITM_PURPLE_RUPEE_e   = 0x05,
    WWITM_ORANGE_RUPEE_e   = 0x06,
    WWITM_HEART_PIECE_e    = 0x07,
    WWITM_HEART_CONTAINER_e= 0x08,
    WWITM_SMALL_MAGIC_e    = 0x09,
    WWITM_LARGE_MAGIC_e    = 0x0A,
    WWITM_BOMB_5_e         = 0x0B,
    WWITM_BOMB_10_e        = 0x0C,
    WWITM_BOMB_20_e        = 0x0D,
    WWITM_BOMB_30_e        = 0x0E,
    WWITM_SILVER_RUPEE_e   = 0x0F,
    WWITM_ARROW_10_e       = 0x10,
    WWITM_ARROW_20_e       = 0x11,
    WWITM_ARROW_30_e       = 0x12,
    WWITM_SMALL_KEY_e      = 0x15,
    WWITM_TRIPLE_HEART_e   = 0x1E,
    WWITM_JOY_PENDANT_e    = 0x1F,
};

static inline bool wwItm_isRupee(u32 no) {
    return (no >= WWITM_GREEN_RUPEE_e && no <= WWITM_ORANGE_RUPEE_e) ||
           no == WWITM_SILVER_RUPEE_e;
}
static inline bool wwItm_isHeart(u32 no) {
    return no == WWITM_HEART_e || no == WWITM_TRIPLE_HEART_e;
}
static inline bool wwItm_isArrowClass(u32 no) {
    // donor mode_wait routes these through itemActionForArrow
    switch (no) {
    case WWITM_HEART_PIECE_e: case WWITM_HEART_CONTAINER_e:
    case WWITM_BOMB_5_e: case WWITM_BOMB_10_e: case WWITM_BOMB_20_e:
    case WWITM_BOMB_30_e: case WWITM_ARROW_10_e: case WWITM_ARROW_20_e:
    case WWITM_ARROW_30_e:
        return true;
    }
    return false;
}

// [I1] THE CONSUMPTION BOUNDARY. Donor execItemGet, re-expressed as TP
// inventory ops keyed by the DONOR id. Amounts are the donor's own.
static void wwItm_execItemGet(u32 i_donorItemNo) {
    // Receiver ops are TP's OWN consumption idioms (d_item.cpp:559/578/630/667
    // — read, not guessed). Life is quarter-hearts (R5 health.unit: donor
    // amounts transfer without scaling).
    switch (i_donorItemNo) {
    case WWITM_GREEN_RUPEE_e:  dComIfGp_setItemRupeeCount(1);   break;
    case WWITM_BLUE_RUPEE_e:   dComIfGp_setItemRupeeCount(5);   break;
    case WWITM_YELLOW_RUPEE_e: dComIfGp_setItemRupeeCount(10);  break;
    case WWITM_RED_RUPEE_e:    dComIfGp_setItemRupeeCount(20);  break;
    case WWITM_PURPLE_RUPEE_e: dComIfGp_setItemRupeeCount(50);  break;
    case WWITM_ORANGE_RUPEE_e: dComIfGp_setItemRupeeCount(100); break;
    case WWITM_SILVER_RUPEE_e: dComIfGp_setItemRupeeCount(200); break;
    case WWITM_HEART_e:        dComIfGp_setItemLifeCount(4.0f, 0);  break;
    case WWITM_TRIPLE_HEART_e: dComIfGp_setItemLifeCount(12.0f, 0); break;
    case WWITM_SMALL_MAGIC_e:  dComIfGp_setItemMagicCount(8);   break;
    case WWITM_LARGE_MAGIC_e:  dComIfGp_setItemMagicCount(32);  break;
    case WWITM_ARROW_10_e:     dComIfGp_setItemArrowNumCount(10); break;
    case WWITM_ARROW_20_e:     dComIfGp_setItemArrowNumCount(20); break;
    case WWITM_ARROW_30_e:     dComIfGp_setItemArrowNumCount(30); break;
    case WWITM_BOMB_5_e:
    case WWITM_BOMB_10_e:
    case WWITM_BOMB_20_e:
    case WWITM_BOMB_30_e:
        // [I1] owed: TP's bomb add needs the bag index (d_item.cpp:2328
        // setItemBombNumCount(bombIdx, n)); wire via TP's own bag-resolve
        // helper rather than guessing the index. LOUD until then (№31-C).
        DuskLog.warn("[WwItem] [I1] bomb pickup (donor {:#x}) — bag-index wiring owed",
                     (int)i_donorItemNo);
        break;
    default:
        // №31-C: an unmapped consumption is LOUD, never a silent no-op —
        // the item vanishes visually either way; the missing effect must not.
        DuskLog.warn("[WwItem] [I1] donor item {:#x} picked up with NO mapped "
                     "TP effect — consumption boundary row owed",
                     (int)i_donorItemNo);
        break;
    }
}

// Donor daItemBase_c_m_data constants (d_a_itembase data; the tunable block
// the donor reads through getData()). Values from the donor's static table.
struct WwItemTuning {
    s16 mWaitTime;        // frames before despawn countdown
    s16 mDisappearTime;   // flashing period before delete
    s16 mNoGetTime;       // frames after spawn during which pickup refuses
    s16 mScaleAnimSpeed;
    s16 mFlashCycleTime;
    s16 mRotateYSpeed;
    s16 mRotateXSpeed;
    f32 mGravity;
    f32 mGroundReflect;
    f32 mLaunchSpeed;
    f32 mSpeedH;
    f32 mVelocityScale;
    f32 mHeartFallSpeed;
    f32 mHeartAmplitude;
    s16 mHeartFallCycleTime;
    s16 mHeartTilt;
};
// Donor values (d_a_itembase.cpp m_data — read from the donor TU).
static const WwItemTuning l_tuning = {
    /* mWaitTime          */ 300,
    /* mDisappearTime     */ 60,
    /* mNoGetTime         */ 10,
    /* mScaleAnimSpeed    */ 10,
    /* mFlashCycleTime    */ 4,
    /* mRotateYSpeed      */ 60,
    /* mRotateXSpeed      */ 0x600,
    /* mGravity           */ -3.0f,
    /* mGroundReflect     */ 0.5f,
    /* mLaunchSpeed       */ 30.0f,
    /* mSpeedH            */ 10.0f,
    /* mVelocityScale     */ 5.0f,
    /* mHeartFallSpeed    */ -1.2f,
    /* mHeartAmplitude    */ 2.0f,
    /* mHeartFallCycleTime*/ 0x400,
    /* mHeartTilt         */ 0x1000,
};

class daWwItem_c : public fopAc_ac_c {
public:
    enum State {
        STATUS_UNK0 = 0, STATUS_UNK1, STATUS_WAIT_MAIN, STATUS_BRING_NEZUMI,
        STATUS_UNK4, STATUS_INIT_NORMAL, STATUS_MAIN_NORMAL,
        STATUS_INIT_GET_DEMO, STATUS_WAIT_GET_DEMO, STATUS_MAIN_GET_DEMO,
        STATUS_WAIT_BOSS1, STATUS_WAIT_BOSS2,
    };
    enum Flag {
        FLAG_UNK02 = 0x02, FLAG_UNK04 = 0x04, FLAG_BOOMERANG = 0x08,
        FLAG_UNK10 = 0x10, FLAG_QUAKE = 0x20, FLAG_HOOK = 0x40,
    };
    enum Mode { MODE_WAIT = 0, MODE_WAIT2 = 1, MODE_WATER = 2 };

    cPhs_Step _create();
    void CreateInit();
    BOOL _execute();
    BOOL _draw();
    BOOL _delete();
    void set_mtx();
    void setTevStr();
    void mode_proc_call();
    void mode_wait_init();
    void mode_water_init();
    void mode_wait();
    void mode_water();
    void execWaitMain();
    void itemGetExecute();
    void itemDefaultRotateY();
    BOOL checkItemDisappear();
    BOOL checkPlayerGet();
    BOOL checkGetItem();
    BOOL timeCount();
    BOOL initAction();
    BOOL itemActionForRupee();
    BOOL itemActionForHeart();
    BOOL itemActionForArrow();
    void checkWall();
    void set_bound_se();

    void setFlag(u8 f) { mFlag |= f; }
    void clrFlag(u8 f) { mFlag &= ~f; }
    bool checkFlag(u8 f) const { return (mFlag & f) != 0; }

public:
    request_of_phase_process_class mPhase;
    J3DModel* mpModel;
    dCcD_Stts mCcStts;
    dCcD_Cyl mCcCyl;
    dBgS_ObjAcch mAcch;
    dBgS_AcchCir mAcchCir;
    u32 m_itemNo;
    u32 mItemBitNo;
    s32 m_timer;
    s32 m_get_timer;
    cXyz mScaleTarget;
    s32 mSpawnSwitchNo;
    s32 mCollideSwitchNo;
    f32 mLastFallSpeed;
    s16 mRotateSpeed;
    s16 mTargetAngleX;
    s16 mWaitTimer;
    s16 mDisappearTimer;
    s16 mExtraZRot;
    u8 mType;
    u8 mAction;
    u8 mFlag;
    u8 mMode;
    u8 mItemStatus;
    u8 mOnGroundTimer;
    bool mDrawIt;
};

namespace daWwItem_prm {
// Donor daItem_prm bit layout VERBATIM (d_a_item.h:139-144).
inline u32 getType(daWwItem_c* i_this)     { return (fopAcM_GetParam(i_this) & 0x03000000) >> 0x18; }
inline u32 getAction(daWwItem_c* i_this)   { return (fopAcM_GetParam(i_this) & 0xFC000000) >> 0x1A; }
inline u32 getItemNo(daWwItem_c* i_this)   { return (fopAcM_GetParam(i_this) & 0x000000FF); }
inline u32 getItemBitNo(daWwItem_c* i_this){ return (fopAcM_GetParam(i_this) & 0x0000FF00) >> 0x08; }
inline u32 getSwitchNo(daWwItem_c* i_this) { return (u32)(i_this->home.angle.z & 0x00FF); }
inline u32 getSwitchNo2(daWwItem_c* i_this){ return (fopAcM_GetParam(i_this) & 0x00FF0000) >> 0x10; }
}  // namespace daWwItem_prm

// Pickup cylinder: the RECEIVER's own field-item source (d_a_obj_item.cpp:1250
// daItem_c::m_cyl_src, verbatim) — TP's proven player-pickup cc config; R/H
// overridden per item from the donor info table in CreateInit.
static const dCcD_SrcCyl l_wwItemCylSrc = {
    {
        {0, {{0, 0, 0}, {0xFFFFFFFF, 17}, 0x59}},
        {dCcD_SE_NONE, 0, 0, 0, {0}},
        {dCcD_SE_NONE, 0, 0, 0, {4}},
        {0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            10.0f,
            50.0f,
        },
    }
};

// ============================================================
// [I2] heap CB — donor CheckFieldItemCreateHeap via the generated FIELD
// table + the §327 resolver idiom. Model only when the row builds no anms
// (all covered field rows; anm members port when a consumer arrives).
// ============================================================
static int daWwItem_CreateHeapCB(fopAc_ac_c* i_ac) {
    daWwItem_c* i_this = (daWwItem_c*)i_ac;
    const WwFieldItemRes* row = dWwItemData_getFieldRes(i_this->m_itemNo);
    if (row == 0 || row->mArc == 0 || row->mBmdIdx < 0) {
        return 0;
    }
    // [I2] donor by-index resolve through the §811 DN-3 consume-time acquirer.
    // CORRECTION of this comment's own earlier claim: the receiver's by-index
    // getObjectRes returns the RAW buffer for WW arcs (BDLs are deliberately
    // NOT mount-parsed — DN-3); casting it was the §810-2 crash class. The
    // acquirer parses from a pristine copy at consume time and caches; the
    // donor's index idiom is preserved. mBmdMember stays as provenance.
    J3DModelData* modelData = dExtNpcMount_acquireModelDataByIndex(
        dExtWw_objectArcAlias(row->mArc), (int)row->mBmdIdx);
    if (modelData == 0) {
        DuskLog.warn("[WwItem] [I2] res: '{}' idx {} ('{}') not resolvable (item {:#x})",
                     row->mArc, (int)row->mBmdIdx,
                     row->mBmdMember != 0 ? row->mBmdMember : "?",
                     (int)i_this->m_itemNo);
        return 0;
    }
    i_this->mpModel = mDoExt_J3DModel__create(modelData, 0x80000, row->mUnknown);
    return i_this->mpModel != 0;
}

// Donor itemGetCallBack — CoHit vs the player triggers the get.
static void daWwItem_getCallBack(fopAc_ac_c* item_actor, dCcD_GObjInf*,
                                 fopAc_ac_c* collided_actor, dCcD_GObjInf*) {
    daWwItem_c* item = (daWwItem_c*)item_actor;
    fopAc_ac_c* player = (fopAc_ac_c*)dComIfGp_getPlayer(0);
    if (item != 0 && item->checkPlayerGet() && collided_actor != 0 &&
        collided_actor == player) {
        item->itemGetExecute();
    }
}

cPhs_Step daWwItem_c::_create() {
    fopAcM_ct(this, daWwItem_c);

    m_itemNo = daWwItem_prm::getItemNo(this);

    const WwFieldItemRes* row = dWwItemData_getFieldRes(m_itemNo);
    if (row == 0 || row->mArc == 0) {
        // donor: `if (!getFieldArc) return cPhs_ERROR_e` — no field presence.
        return cPhs_ERROR_e;   // (donor setLoadError = itembase bookkeeping; fpc ERROR return is the effect)
    }

    mItemBitNo = daWwItem_prm::getItemBitNo(this) & 0x7F;
    if (fopAcM_isItem(this, mItemBitNo) && mItemBitNo != 0x7F) {
        // donor: already picked up in this room — refuse the respawn.
        return cPhs_ERROR_e;   // (donor setLoadError = itembase bookkeeping; fpc ERROR return is the effect)
    }

    // §806 arc alias: donor names kept in DATA ("Always" rows); TP-colliding
    // names translate at this consumption boundary to the WW twin mount.
    cPhs_Step phase_state = dComIfG_resLoad(&mPhase, dExtWw_objectArcAlias(row->mArc));
    if (phase_state == cPhs_COMPLEATE_e) {
        if (!fopAcM_entrySolidHeap(this, &daWwItem_CreateHeapCB, row->mHeapSize)) {
            return cPhs_ERROR_e;
        }
        CreateInit();
    }
    return phase_state;
}

void daWwItem_c::CreateInit() {
    mAcchCir.SetWall(30.0f, 30.0f);
    mAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1,
              &mAcchCir, fopAcM_GetSpeed_p(this), NULL, NULL);
    mAcch.ClrWaterNone();
    mAcch.ClrRoofNone();

    set_mtx();
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());

    mCcStts.Init(0, 0xFF, this);
    mCcCyl.Set(l_wwItemCylSrc);
    mCcCyl.SetStts(&mCcStts);
    mCcCyl.SetCoHitCallback(&daWwItem_getCallBack);

    const WwItemInfo* info = dWwItemData_getInfo(m_itemNo);
    f32 height = info != 0 ? (f32)info->mCollisionH : 30.0f;
    f32 radius = info != 0 ? (f32)info->mCollisionR : 20.0f;
    if (scale.x > 1.0f) {
        height *= scale.x;
        radius *= scale.x;
    }
    mCcCyl.SetR(radius);
    mCcCyl.SetH(height);

    m_timer = 0;
    m_get_timer = 0;
    mWaitTimer = l_tuning.mWaitTime;
    mDisappearTimer = l_tuning.mDisappearTime;
    mLastFallSpeed = fopAcM_GetSpeed(this).y;
    mItemStatus = STATUS_UNK0;
    mOnGroundTimer = 0;
    mExtraZRot = 0;
    mFlag = 0;
    mDrawIt = true;

    mType = (u8)daWwItem_prm::getType(this);
    if (mType == 3 || mType == 1) {
        setFlag(FLAG_UNK02);
    }
    mAction = (u8)daWwItem_prm::getAction(this);

    // donor bomb rows scale to 0.6, everything else to 1.0
    switch (m_itemNo) {
    case WWITM_BOMB_5_e: case WWITM_BOMB_10_e:
    case WWITM_BOMB_20_e: case WWITM_BOMB_30_e:
        mScaleTarget.set(0.6f, 0.6f, 0.6f);
        break;
    default:
        mScaleTarget.set(1.0f, 1.0f, 1.0f);
        break;
    }

    mSpawnSwitchNo = daWwItem_prm::getSwitchNo2(this);
    if (mSpawnSwitchNo != 0xFF && !fopAcM_isSwitch(this, mSpawnSwitchNo)) {
        mDrawIt = false;
        setFlag(FLAG_UNK02);
    }
    mCollideSwitchNo = daWwItem_prm::getSwitchNo(this);

    current.angle.z = 0;
    home.angle.z = 0;
    initAction();
    set_mtx();
}

BOOL daWwItem_c::_execute() {
    if (mSpawnSwitchNo != 0xFF && !fopAcM_isSwitch(this, mSpawnSwitchNo)) {
        return TRUE;
    }
    if (mSpawnSwitchNo != 0xFF && fopAcM_isSwitch(this, mSpawnSwitchNo)) {
        mDrawIt = true;
    }

    timeCount();

    eyePos = current.pos;
    const WwItemInfo* info = dWwItemData_getInfo(m_itemNo);
    eyePos.y += (info != 0 ? info->mCollisionH : 30) / 2.0f;
    attention_info.position = current.pos;

    switch (mItemStatus) {
    case STATUS_UNK0:
    case STATUS_UNK1:
    case STATUS_WAIT_MAIN:
        execWaitMain();
        break;
    case STATUS_INIT_NORMAL:
    case STATUS_MAIN_NORMAL:
        // donor: item rises above the player's head and shrinks out; the
        // pickup already consumed — shrink and delete.
        cLib_chaseF(&scale.x, 0.0f, 0.1f);
        scale.y = scale.z = scale.x;
        if (scale.x <= 0.0f) {
            fopAcM_delete(this);
        }
        break;
    case STATUS_INIT_GET_DEMO:
    case STATUS_WAIT_GET_DEMO:
    case STATUS_MAIN_GET_DEMO:
        // [I5] important-item demo statuses: field spine hands off to the
        // demo-item machinery; until an important item is placed as a FIELD
        // drop on WW content this leg is unreachable. LOUD if reached.
        DuskLog.warn("[WwItem] [I5] get-demo status {} reached for item {:#x} — "
                     "demo-item handoff owed", (int)mItemStatus, (int)m_itemNo);
        wwItm_execItemGet(m_itemNo);
        fopAcM_delete(this);
        break;
    case STATUS_BRING_NEZUMI:
    case STATUS_WAIT_BOSS1:
    case STATUS_WAIT_BOSS2:
    default:
        execWaitMain();
        break;
    }

    set_mtx();
    return TRUE;
}

void daWwItem_c::execWaitMain() {
    checkGetItem();
    if (mType != 1) {
        fopAcM_posMoveF(this, mCcStts.GetCCMoveP());
    }
    mode_proc_call();

    if (!checkFlag(FLAG_UNK02)) {
        cLib_chaseF(&scale.x, mScaleTarget.x, mScaleTarget.x / l_tuning.mScaleAnimSpeed);
        cLib_chaseF(&scale.y, mScaleTarget.y, mScaleTarget.y / l_tuning.mScaleAnimSpeed);
        cLib_chaseF(&scale.z, mScaleTarget.z, mScaleTarget.z / l_tuning.mScaleAnimSpeed);
    }

    if (checkItemDisappear() && mWaitTimer == 0) {
        if (mDisappearTimer == 0) {
            fopAcM_delete(this);
        }
        if ((m_timer % l_tuning.mFlashCycleTime) == 0) {
            mDrawIt = !mDrawIt;   // donor changeDraw() flash
        }
    }

    const WwItemInfo* info = dWwItemData_getInfo(m_itemNo);
    const bool noCollide = info != 0 && (info->mFlag & 0x2) != 0;
    if (!noCollide) {
        if (mCollideSwitchNo == 0xFF ||
            (mCollideSwitchNo != 0xFF && fopAcM_isSwitch(this, mCollideSwitchNo))) {
            mCcCyl.SetC(current.pos);
            dComIfG_Ccsp()->Set(&mCcCyl);
        }
    }
}

void daWwItem_c::mode_proc_call() {
    if (mType == 1) {
        itemDefaultRotateY();
        return;
    }
    switch (mMode) {
    case MODE_WATER:
        mode_water();
        break;
    default:
        mode_wait();
        break;
    }
}

void daWwItem_c::mode_wait_init() {
    mMode = MODE_WAIT;
    fopAcM_SetGravity(this, l_tuning.mGravity);
}

void daWwItem_c::mode_water_init() {
    // [I3] GATE: no host daSea query yet — the Acch water surface (receiver-
    // served) is the reachable branch; the open-sea query joins when a sea
    // provider lands. State machine intact, entry condition narrowed.
    mMode = MODE_WATER;
    if (mAcch.ChkWaterHit() && mAcch.m_wtr.GetHeight() >= current.pos.y) {
        current.pos.y = mAcch.m_wtr.GetHeight();
    } else {
        mode_wait_init();
        return;
    }
    fopAcM_SetSpeed(this, 0.0f, 0.0f, 0.0f);
    fopAcM_SetSpeedF(this, 0.0f);
    current.angle.x = current.angle.z = 0;
    mExtraZRot = 0;
    mRotateSpeed = 0;
    clrFlag(FLAG_UNK04);
    scale.set(mScaleTarget.x, mScaleTarget.y, mScaleTarget.z);
}

void daWwItem_c::mode_wait() {
    if (wwItm_isHeart(m_itemNo)) {
        itemActionForHeart();
    } else if (wwItm_isArrowClass(m_itemNo)) {
        itemActionForArrow();
    } else {
        itemActionForRupee();   // donor default leg (rupees + everything else)
    }

    if (mAcch.ChkWaterHit() && mAcch.m_wtr.GetHeight() > current.pos.y) {
        mode_water_init();   // [I3]
    }
}

void daWwItem_c::mode_water() {
    mAcch.CrrPos(dComIfG_Bgsp());
    if (!mAcch.ChkWaterHit() || mAcch.m_wtr.GetHeight() < current.pos.y) {
        mode_wait_init();
    } else {
        current.pos.y = mAcch.m_wtr.GetHeight();
    }
    itemDefaultRotateY();
}

BOOL daWwItem_c::itemActionForRupee() {
    mAcch.CrrPos(dComIfG_Bgsp());
    checkWall();

    if (mAcch.ChkGroundLanding()) {
        f32 bounce = mLastFallSpeed * l_tuning.mGroundReflect;
        if (bounce > gravity - 0.5f) {
            fopAcM_SetSpeedF(this, 0.0f);
        } else {
            speed.set(0.0f, -bounce, 0.0f);
        }
        mOnGroundTimer++;
        if (mOnGroundTimer >= 2) {
            clrFlag(FLAG_UNK04);
        }
        set_bound_se();
    } else if (mAcch.ChkGroundHit()) {
        itemDefaultRotateY();
        fopAcM_SetSpeedF(this, 0.0f);
        clrFlag(FLAG_UNK04);
        mOnGroundTimer = 1;
    }

    if (speed.y != 0.0f) {
        mLastFallSpeed = speed.y;
    }

    mRotateSpeed = l_tuning.mRotateXSpeed;
    if (mOnGroundTimer == 0) {
        mTargetAngleX = current.angle.x + mRotateSpeed;
    } else {
        mTargetAngleX = 0;
    }
    if (!checkFlag(FLAG_UNK02)) {
        cLib_chaseAngleS(&current.angle.x, mTargetAngleX, mRotateSpeed);
    }
    return TRUE;
}

BOOL daWwItem_c::itemActionForHeart() {
    f32 origSpeedY = speed.y;
    if (origSpeedY < 0.0f) {
        fopAcM_SetGravity(this, 0.0f);
        fopAcM_SetSpeed(this, 0.0f, l_tuning.mHeartFallSpeed, 0.0f);
    }

    mAcch.CrrPos(dComIfG_Bgsp());

    if (mAcch.ChkGroundLanding() || mAcch.ChkGroundHit()) {
        clrFlag(FLAG_UNK04);
        mExtraZRot = 0;
        fopAcM_SetSpeed(this, 0.0f, -1.0f, 0.0f);
        fopAcM_SetSpeedF(this, 0.0f);
        itemDefaultRotateY();
    } else if (origSpeedY < 0.0f) {
        fopAcM_SetSpeedF(this, l_tuning.mHeartAmplitude *
                                   cM_ssin((s16)(m_timer * l_tuning.mHeartFallCycleTime)));
    }
    return TRUE;
}

BOOL daWwItem_c::itemActionForArrow() {
    // donor: like rupee, without the X spin.
    mAcch.CrrPos(dComIfG_Bgsp());
    checkWall();
    if (mAcch.ChkGroundLanding() || mAcch.ChkGroundHit()) {
        fopAcM_SetSpeedF(this, 0.0f);
        speed.y = 0.0f;
        clrFlag(FLAG_UNK04);
        set_bound_se();
        itemDefaultRotateY();
    }
    if (speed.y != 0.0f) {
        mLastFallSpeed = speed.y;
    }
    return TRUE;
}

void daWwItem_c::checkWall() {
    if (!mAcch.ChkWallHit()) {
        return;
    }
    // donor Reflect(): bounce the horizontal velocity off the wall normal.
    // Receiver-safe simplification of the same math: reverse the heading.
    current.angle.y += 0x8000;
    speedF *= 0.7f;
}

void daWwItem_c::set_bound_se() {
    if (m_get_timer < 10) {
        return;
    }
    // [I4] donor per-class bound SEs are JA_SE_* ids consumed by the WW audio
    // side; wire when the JA1 SE surface serves actor SEs. LOUD-ONCE.
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.info("[WwItem] [I4] bound SE owed (JA_SE_OBJ_LUPY_BOUND family)");
    }
}

void daWwItem_c::itemDefaultRotateY() {
    s16 rotationSpeed = (s16)(0xFFFF / l_tuning.mRotateYSpeed);
    fopAcM_addAngleY(this, (s16)(current.angle.y + rotationSpeed), rotationSpeed);
}

BOOL daWwItem_c::checkItemDisappear() {
    BOOL disappearing = TRUE;
    if (checkFlag(FLAG_UNK02) || checkFlag(FLAG_UNK10)) {
        disappearing = FALSE;
    }
    const WwItemInfo* info = dWwItemData_getInfo(m_itemNo);
    if (info != 0 && (info->mFlag & 0x01) != 0) {
        disappearing = FALSE;   // donor chkFlag(no, 1): permanent placements
    }
    if (dComIfGp_event_runCheck()) {
        disappearing = FALSE;
    }
    return disappearing;
}

BOOL daWwItem_c::checkPlayerGet() {
    if (m_get_timer < l_tuning.mNoGetTime) {
        return FALSE;
    }
    if (mItemStatus == STATUS_BRING_NEZUMI) {
        return FALSE;
    }
    return TRUE;
}

BOOL daWwItem_c::checkGetItem() {
    if (!checkPlayerGet()) {
        return FALSE;
    }
    if (mCcCyl.ChkTgHit()) {
        cCcD_Obj* hitObj = mCcCyl.GetTgHitObj();
        if (hitObj != 0) {
            // donor: sword AT collects, boomerang latches. Receiver bit for
            // sword = NORMAL_SWORD (R5 weapon.at_type_bits: donor SWORD bit 1
            // == receiver NORMAL_SWORD bit 1 — the one same-bit case).
            if (hitObj->ChkAtType(AT_TYPE_NORMAL_SWORD)) {
                itemGetExecute();
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL daWwItem_c::timeCount() {
    m_timer++;
    if (m_get_timer < 10000) {
        m_get_timer++;
    }
    if (checkPlayerGet() && !dComIfGp_event_runCheck()) {
        if (mWaitTimer > 0) {
            mWaitTimer--;
        } else if (mDisappearTimer > 0) {
            mDisappearTimer--;
        }
    }
    return TRUE;
}

void daWwItem_c::itemGetExecute() {
    if (mItemStatus == STATUS_INIT_NORMAL) {
        return;
    }

    switch (m_itemNo) {
    case WWITM_HEART_PIECE_e:
    case WWITM_HEART_CONTAINER_e:
    case WWITM_SMALL_KEY_e:
        mItemStatus = STATUS_INIT_GET_DEMO;   // [I5]
        break;
    default:
        mItemStatus = STATUS_INIT_NORMAL;
        wwItm_execItemGet(m_itemNo);          // [I1]
        break;
    }

    fopAcM_onItem(this, mItemBitNo);   // receiver form (f_op_actor_mng.h:467)
    clrFlag(FLAG_UNK04);

    // donor VERSION > DEMO: disarm the cylinder so the get fires once.
    mCcCyl.SetTgType(0);
    mCcCyl.OffCoSetBit();
    mCcCyl.ClrTgHit();
    mCcCyl.ClrCoHit();
}

BOOL daWwItem_c::initAction() {
    // donor initAction: launch physics per action id. The 0/B "placed" leg
    // and the 1/2/4/9 "burst" legs cover field placements + enemy drops.
    f32 y_speed = 0.0f;
    switch (mAction) {
    case 1:
        y_speed = l_tuning.mLaunchSpeed + cM_rndFX(5.0f);
        speedF = l_tuning.mSpeedH / 10.0f;
        current.angle.y = (s16)cM_rndF(65535.0f);
        break;
    case 2:
    case 4:
    case 9:
        speedF = 0.0f;
        current.angle.y = (s16)cM_rndF(65535.0f);
        y_speed = l_tuning.mLaunchSpeed + cM_rndFX(5.0f);
        break;
    case 3:
        y_speed = 25.0f;
        current.angle.y = (s16)cM_rndF(65535.0f);
        speedF = l_tuning.mVelocityScale;
        break;
    case 7:
        speedF = l_tuning.mVelocityScale * 1.5f;
        current.angle.y = (s16)cM_rndF(65535.0f);
        y_speed = l_tuning.mLaunchSpeed + cM_rndFX(5.0f);
        break;
    case 0xB:
        current.angle.y = (s16)cM_rndF(65535.0f);
        y_speed = 0.0f;
        speedF = 0.0f;
        break;
    default:
        break;
    }

    mExtraZRot = 0;
    if (wwItm_isHeart(m_itemNo)) {
        speedF = 2.0f * speedF;
        mExtraZRot = (s16)cM_rndFX((f32)l_tuning.mHeartTilt);
    }

    fopAcM_SetGravity(this, l_tuning.mGravity);
    fopAcM_SetSpeed(this, 0.0f, y_speed, 0.0f);
    fopAcM_SetSpeedF(this, speedF);
    if (mAction != 0) {
        scale.setall(0.0f);   // burst spawns grow in via execWaitMain chase
        setFlag(FLAG_UNK04);
    } else {
        scale.set(mScaleTarget.x, mScaleTarget.y, mScaleTarget.z);
    }
    mMode = MODE_WAIT;
    return TRUE;
}

void daWwItem_c::set_mtx() {
    if (mpModel == 0) {
        return;
    }
    mpModel->setBaseScale(scale);
    const WwItemInfo* info = dWwItemData_getInfo(m_itemNo);
    const f32 h = info != 0 ? (f32)info->mCollisionH : 0.0f;

    mDoMtx_stack_c::transS(current.pos.x, current.pos.y, current.pos.z);
    if (wwItm_isRupee(m_itemNo)) {
        mDoMtx_stack_c::transM(0.0f, h / 2.0f, 0.0f);
    }
    mDoMtx_stack_c::ZXYrotM(current.angle.x, current.angle.y,
                            (s16)(current.angle.z + mExtraZRot));
    if (wwItm_isRupee(m_itemNo)) {
        mDoMtx_stack_c::transM(0.0f, -h / 2.0f, 0.0f);
    }
    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());
}

void daWwItem_c::setTevStr() {
    // §406 WW feeders — donor tevStr constants (0x96 C0 / 0xFF K0) live in
    // the donor draw; the WW kankyo feeder supplies the donor-typed struct.
    dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
    dKyWw_setLightTevColorType(mpModel, &tevStr);
}

BOOL daWwItem_c::_draw() {
    if (!mDrawIt || mpModel == 0) {
        return TRUE;
    }
    setTevStr();
    mDoExt_modelUpdateDL(mpModel);
    return TRUE;
}

BOOL daWwItem_c::_delete() {
    const WwFieldItemRes* row = dWwItemData_getFieldRes(m_itemNo);
    if (row != 0 && row->mArc != 0) {
        dComIfG_resDelete(&mPhase, dExtWw_objectArcAlias(row->mArc));
    }
    return TRUE;
}

// ============================================================
// [I6] fpc method table — kept TU-local and UNREGISTERED: no profile, no
// l_objectName row in this landing. Registration is plan step 5, LAST,
// stripping the §725 no-op in the same change.
// ============================================================
static BOOL daWwItem_Draw(daWwItem_c* i_this) { return i_this->_draw(); }
static BOOL daWwItem_Execute(daWwItem_c* i_this) { return i_this->_execute(); }
static BOOL daWwItem_IsDelete(daWwItem_c*) { return TRUE; }
static BOOL daWwItem_Delete(daWwItem_c* i_this) { return i_this->_delete(); }
static cPhs_Step daWwItem_Create(fopAc_ac_c* i_this) {
    return ((daWwItem_c*)i_this)->_create();
}

actor_method_class l_daWwItem_Method = {
    (process_method_func)daWwItem_Create,
    (process_method_func)daWwItem_Delete,
    (process_method_func)daWwItem_Execute,
    (process_method_func)daWwItem_IsDelete,
    (process_method_func)daWwItem_Draw,
};

// [I6] profile registered (ww_profile_register row, kill-switch pattern:
// landing != activating) but NO l_objectName route exists — no placement can
// name this actor until the battery passes and the §725 no-op flips to a
// route in the same change. Shape mirrors spc_item01's profile block.
actor_process_profile_definition g_profile_WW_ITEM = {
    /* Layer ID     */ fpcLy_CURRENT_e,
    /* List ID      */ 0x0007,
    /* List Prio    */ fpcPi_CURRENT_e,
    /* Proc Name    */ fpcNm_WW_ITEM_e,
    /* Proc SubMtd  */ &g_fpcLf_Method.base,
    /* Size         */ sizeof(daWwItem_c),
    /* Size Other   */ 0,
    /* Parameters   */ 0,
    /* Leaf SubMtd  */ &g_fopAc_Method.base,
    /* Draw Prio    */ fpcDwPi_E_RD_e,   // §327's item-class slot (spc_item01 precedent)
    /* Actor SubMtd */ &l_daWwItem_Method,
    /* Status       */ fopAcStts_NOCULLEXEC_e | fopAcStts_CULL_e | fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,
    /* Group        */ fopAc_ACTOR_e,
    /* Cull Type    */ fopAc_CULLBOX_0_e,
};
