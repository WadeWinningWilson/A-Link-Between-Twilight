// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_tsubo.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW TSUBO ACTOR — every interior's furniture (pots, barrels, skulls,
// stools, pails, spine urchins, boxes; 16 donor subtypes). Donor
// daTsubo::Act_c ported WHOLE (full-state-machine law): all 8 modes
// (hide/appear/wait/walk/carry/drop/sink/afl), the quaternion pose-tensor
// system, the carry/throw/bound physics, the per-type damage matrix, the
// spec(boko-in-pot) machinery, and the drop-item exit. Data layer =
// ww_tsubo_data (§803/§804, generated donor-verbatim).
//
// SEAMS (each at its site; DN-10 order-of-resort recorded per seam):
//   [T1] cc source: the RECEIVER's own carryable-pot config
//        (d_a_obj_carry.cpp:661 l_cyl_src — AT_TYPE_THROW_OBJ atp 2, the
//        donor's exact throw semantic under TP bits); R/H per donor row.
//   [T2] attack-type translation (port-registry trap #5: only BOMB agrees).
//        Donor At-type checks re-expressed in the receiver's own bit
//        vocabulary at the consumption boundary; donor branch STRUCTURE
//        untouched. WIND->BOOMERANG (gale), SKULL_HAMMER->IRON_BALL.
//   [T3] drop exit: donor fopAcM_createItemFromTable semantics at the
//        boundary — direct ids (0x00-0x1F) spawn the ported WW_ITEM with
//        the DONOR id + donor action code; 0x3F/0xFF = no drop (donor
//        exact, f_op_actor_mng.cpp:893); table ids 0x20-0x3E LOUD-owed
//        (WW drop-table roll port).
//   [T4] break/splash FX: WW RULES (user covenant ruling 2026-08-12 — no TP
//        content stand-ins; the first cut's TP pot-shard call was a breach,
//        reverted). ALL donor effects (shard emitters, pillars, land smoke,
//        kutani ambient) are LOUD-owed until the donor's own dPa system +
//        WW JPA bank land. Donor call sites preserved in shape.
//   [T5] audio: WW JA_SE ids + WW sound-env ids do not map into the Z2
//        space — suppressed at a single boundary (audio pass owed).
//   [T6] spec boko: machinery ported whole; spec_make_boko cannot spawn
//        (no fpcNm_BOKO_e in the port) -> LOUD, entries stay ERROR, every
//        spec loop is inert-but-faithful until the Boko enemy port lands.
//   [T7] sea/current: daSea_* through the §223 shims (no sea surface yet,
//        lane #13); room-path water current (daObj::get_path_spd) is a
//        receiver stub -> afl drifts on zero stream until the waves pass.
//   [T8] carry protocol: donor attention CARRY flag/distance re-expressed
//        as the receiver's fopAc_AttnFlag_CARRY_e + OnCarryType(LIGHT)
//        (d_a_obj_carry idiom) so the TP player's own grab machinery
//        drives the donor carry mode. getRightFootOnGround has no
//        receiver equivalent -> water-slosh SE timing keys on GrabUpEnd.
//   [T9] REGISTRATION IS ELSEWHERE (§747 pattern): fpcNm_WW_TSUBO_e
//        profile + the 12 data-keyed name routes land with this TU.
// ============================================================

#include "d/ext_plugin/ww_tsubo_data.h"

#include "SSystem/SComponent/c_angle.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_m3d_g_pla.h"
#include "d/d_a_obj.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo_ww.h"
#include "d/d_particle.h"
#include "d/d_particle_name.h"
#include "d/d_drawlist.h"   // §823 dDlst_shadowControl_c::getSimpleTex (soft shadow default)
#include "d/actor/d_a_player.h"
#include "f_op/f_op_actor_mng.h"
#include "f_op/f_op_kankyo_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include "d/d_ext_ww_actor_shims.h"   // daSea_* shims + WW dialect constants
#include "d/d_ext_npc_mount.h"        // §811 DN-3 consume-time model acquirer (by-index form)
#include "dusk/logging.h"

// ============================================================
// [T5] donor SE vocabulary — WW JA_SE ids and WW sound-env ids have no Z2
// mapping yet; one boundary suppresses them all (audio pass owed, same
// posture as the pig/seagull/item ports). The donor CALL SITES are kept.
// ============================================================
static void dWwTsubo_seStart(fopAc_ac_c* i_this, s32 i_wwSeId, u32 i_param) {
    (void)i_this; (void)i_wwSeId; (void)i_param;
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.info("[WwTsubo] [T5] WW SE suppressed (first id {:#x}) — audio pass owed",
                     (int)i_wwSeId);
    }
}
static void dWwTsubo_senv(const cXyz&, int, fpc_ProcID, int) {
    // donor dKy_Sound_set with WW sound-env ids — WW id space, owed with [T5].
}
#define JA_SE_OBJ_BOKKURI_RECOVER  0
#define JA_SE_OBJ_BOKKURI_SWELL    0
#define JA_SE_OBJ_BOKKURI_BOUND    0
#define JA_SE_OBJ_BOKKURI_PICK_UP  0
#define JA_SE_OBJ_WPOT_LIFTUP      0
#define JA_SE_LK_CONVEY_WPOT       0

class daWwTsubo_c : public fopAc_ac_c {
public:
    // donor daTsubo::Act_c 0x820 — receiver-typed field-for-field mirror.
    cPhs_Step _create();
    bool _is_delete();
    BOOL _delete();
    BOOL _execute();
    BOOL _draw();

    void prmZ_init();
    bool create_heap();
    void create_init_cull();
    void create_init_cc();
    void create_init_bgc();
    void spec_make_boko(int);
    void spec_init();
    void spec_set_actor();
    void spec_clr_actor();
    void spec_carry_spec();
    void spec_mode_carry_init();
    void spec_mode_carry(bool);
    void spec_mode_put_init();
    void spec_set_room();
    void spec_remove();
    void spec_kill();
    void spec_mtx();
    void mode_hide_init();
    void mode_hide();
    void mode_appear_init();
    void mode_appear();
    void mode_wait_init();
    void mode_wait();
    void mode_walk_init();
    void mode_walk();
    void mode_carry_init();
    void mode_carry();
    void mode_drop_init();
    void mode_drop();
    void mode_sink_init();
    void mode_sink();
    void mode_afl_init();
    void mode_afl();
    bool mode_proc_call();
    void cull_set_draw();
    void cull_set_move();
    void damaged(int, cBgS_PolyInfo*, bool, const cXyz*);
    void damaged(int, cBgS_PolyInfo*);
    void damaged_lava();
    bool damage_tg_acc();
    bool damage_cc_proc();
    bool damage_bg_proc();
    bool damage_bg_proc_directly();
    bool damage_kill_proc();
    void crr_pos(const cXyz&);
    void crr_pos_water();
    void crr_pos_lava();
    void water_tention();
    static f32 reflect(cXyz*, const cBgS_PolyInfo&, f32);
    void bound(f32);
    void moment_proc_call();
    void set_wind_vec();
    void init_mtx();
    void set_mtx();
    void set_tensor(const cXyz*);
    void init_rot_throw();
    void init_rot_clean();
    void set_tensor_hide();
    void set_tensor_appear();
    void set_tensor_wait();
    void set_tensor_walk();
    void set_tensor_carry();
    void set_tensor_drop();
    void set_tensor_sink();
    void set_tensor_afl();
    void eff_drop_water();
    void eff_land_smoke();
    void eff_break_dispatch();
    void eff_break_tsubo();
    void eff_break_barrel();
    void eff_break_stool();
    void eff_break_skull();
    void eff_break_pail();
    void eff_break_spine();
    void eff_break_hbox2S();
    void eff_break_try();
    void eff_break_pinecone();
    void eff_break_woodS();
    void eff_hit_water_splash();
    void eff_hit_lava_splash();
    void eff_kutani_set();
    void eff_kutani_init();
    bool chk_sink_lava();
    bool chk_sink_water();
    bool chk_sinkdown_water();
    void calc_drop_param(f32*, f32*, f32*) const;
    void calc_afl_param(f32*, f32*, f32*) const;
    void se_fall_water();
    void se_fall_lava();
    void se_break(cBgS_PolyInfo*);
    void se_pickup_carry();
    void se_pickup_carry_init();
    void se_pickup();
    void cam_lockoff() const;

    // donor prm accessors (d_a_tsubo.h:159-217, bit layout VERBATIM)
    int prm_get_type() const   { return daObj::PrmAbstract(this, 4, 24); }
    s32 prm_get_cull() const   { return daObj::PrmAbstract(this, 3, 28); }
    s32 prm_get_itemNo() const { return daObj::PrmAbstract(this, 6, 0); }
    s32 prm_get_itemSave() const { return daObj::PrmAbstract(this, 7, 16); }
    bool prm_get_moveBg() const { return daObj::PrmAbstract(this, 2, 14) == 1; }
    s32 prm_get_spec() const   { return daObj::PrmAbstract(this, 6, 8); }
    bool prm_get_stick() const { return daObj::PrmAbstract(this, 1, 31) != 0; }
    void prm_off_moveBg() { fopAcM_SetParam(this, fopAcM_GetParam(this) | 0xC000); }
    void prm_off_stick()  { fopAcM_SetParam(this, fopAcM_GetParam(this) & ~0x80000000); }
    void prm_set_cull_non() { fopAcM_SetParam(this, fopAcM_GetParam(this) & ~0x70000000); }
    int prmZ_get_swSave() const { return mPrmZ & 0xFF; }
    bool is_switch() const {
        return fopAcM_isSwitch(const_cast<daWwTsubo_c*>(this), prmZ_get_swSave());
    }
    bool spec_chk_prm_boko() const { return prm_get_spec() != 0x3F; }

    const WwTsuboData& data() const { return *dWwTsuboData_get((u32)mType); }
    const WwTsuboAttrSpine& attrSpine() const { return *dWwTsuboData_getAttrSpine(); }
    const WwTsuboSpecBoko& data_spec_boko(int i) const { return *dWwTsuboData_getSpecBoko(i); }

    struct M7A0 {   // donor d_a_tsubo.h:148
        fpc_ProcID m00;
        u8 m04;
        s16 m06;
        s16 m08;
        f32 m0C;
        f32 m10;
        f32 m14;
        s8 m18;
    };

    static fopAc_ac_c* M_spec_act[3];

    request_of_phase_process_class mPhase;
    J3DModel* mpModel;
    mDoExt_brkAnm* mpBrk;
    dBgS_ObjAcch mAcch;
    dBgS_AcchCir mAcchCir;
    dBgS_GndChk mGndChkYogan;   // donor dBgS_ObjGndChk_Yogan; receiver GndChk carries the cross
    f32 m4F8;
    f32 m4FC;
    f32 m500;
    bool m504;
    bool m505;
    dCcD_Cyl mCyl;
    dCcD_Stts mStts;
    s32 mType;
    s32 m678;      // mode
    u16 mPrmZ;
    u8 m67E;
    u8 m67F;
    u8 m680;
    s8 m681;
    u8 m682;
    u8 m683;
    u8 m684;
    bool m685;
    bool m686;
    cSAngle m688;
    cSAngle m68A;
    cSAngle m68C;
    cSAngle m68E;
    cSAngle m690;
    cSAngle m692;
    cXyz m6B0;
    Mtx mPoseMtx;
    f32 m6EC;
    cXyz m6F0;
    f32 m6FC;
    cXyz m700;
    u8 m798;       // kutani ambient timer ([T4]: the emitter itself is owed)
    M7A0 m7A0[3];
    cXyz m7F4;     // wind velocity
    s16 m800;      // spine kill timer
    s16 m802;
    s16 m804;
    s16 m806;
    f32 m808;
    f32 m80C;
    u8 m810;
    u8 m811;
    cXyz m814;     // hit knockback
};

fopAc_ac_c* daWwTsubo_c::M_spec_act[3];

// ============================================================
// [T1] cc source — the receiver's OWN carryable-pot config, VERBATIM from
// d_a_obj_carry.cpp:661 (TP's proven throw/attack-target/push cylinder;
// AT_TYPE_THROW_OBJ atp 2 == the donor M_cyl_src's AT_TYPE_UNK8 atp 2
// semantic). R/H overridden per donor row in create_init_cc (donor exact).
// ============================================================
static const dCcD_SrcCyl l_wwTsuboCylSrc = {
    {
        {0x0, {{AT_TYPE_THROW_OBJ, 0x2, 0x1f}, {0xd8fbfdff, 0x1f}, 0x79}},
        {dCcD_SE_THROW_OBJ, 0x1, 0x0, 0x0, 0x1},
        {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x6},
        {0x0},
    },
    {
        {
            {0.0f, 0.0f, 0.0f},
            30.0f,
            60.0f,
        }
    }
};

// [T2] receiver expression of the donor's "sword-swing family" At mask
// (d_a_tsubo.cpp:2276 — SWORD/BOKO_STICK/MACHETE/UNK800/SPIKE/UNK2000/
// DARKNUT_SWORD/MOBLIN_SPEAR). The receiver's swing vocabulary is the
// player's; enemy melee joins as WW enemies land.
#define WWTSUBO_AT_SWING_MASK                                                        \
    (AT_TYPE_NORMAL_SWORD | AT_TYPE_MASTER_SWORD | AT_TYPE_WOLF_ATTACK |             \
     AT_TYPE_WOLF_CUT_TURN)

// ============================================================
// [T3] drop exit — donor fopAcM_createItemFromTable semantics at the
// consumption boundary. Direct ids spawn the ported WW_ITEM (donor id
// space, R5 item.id_space law) with the donor action code in the donor's
// own param encoding (d_a_item.h:139 bit layout, mirrored by daWwItem_prm).
// ============================================================
static void dWwTsubo_createItemFromTable(const cXyz* i_pos, int i_itemNo, int i_itemBitNo,
                                         int i_roomNo, const csXyz* i_angle, int i_action) {
    if (i_itemNo == 0x3F || i_itemNo == 0xFF) {
        return;   // donor exact: "no item" placements drop nothing (f_op_actor_mng.cpp:893)
    }
    if (i_itemNo >= 0x20 && i_itemNo <= 0x3E) {
        // donor: life-scaled + random drop tables (dComIfGp_getItemTable) —
        // the WW table roll is its own port unit. №31-C LOUD, never silent.
        static bool s_warned = false;
        if (!s_warned) {
            s_warned = true;
            DuskLog.warn("[WwTsubo] [T3] WW drop-table id {:#x} — table-roll port owed",
                         i_itemNo);
        }
        return;
    }
    u32 params = ((u32)i_itemNo & 0xFF)              // itemNo
               | (((u32)i_itemBitNo & 0xFF) << 8)    // save bit (0x7F = none)
               | (0xFFu << 16)                       // switchNo2: none
               | (((u32)i_action & 0x3F) << 26);     // donor action code (1/2/3/7/9)
    fopAcM_create(fpcNm_WW_ITEM_e, params, i_pos, i_roomNo, i_angle, NULL, -1);
}

// ============================================================
// donor prmZ latch (d_a_tsubo.cpp:1181) — the DZR packs the switch id in
// the placement's Z angle; read once, then zero the live angles.
// ============================================================
void daWwTsubo_c::prmZ_init() {
    if (m67E != 0) {
        return;
    }
    mPrmZ = home.angle.z;
    m67E = 1;
    home.angle.z = 0;
    current.angle.z = 0;
    shape_angle.z = 0;
}

// ============================================================
// donor create_heap (d_a_tsubo.cpp:1198) — model BY INDEX from the donor
// table's own key (§762 native resolver idiom); type 7 (spine) adds the
// BRK glow anim (Kmi00x res 0x7, resolved from the donor's own res header).
// ============================================================
static int daWwTsubo_solidHeapCB(fopAc_ac_c* i_ac) {
    return ((daWwTsubo_c*)i_ac)->create_heap();
}

bool daWwTsubo_c::create_heap() {
    // §806 arc alias: donor names kept in DATA; TP-colliding names translate
    // at this consumption boundary ("Always"→"WwAlways" etc., served verbatim
    // by the donor-disc reader under the renamed mount point).
    const char* arc = dExtWw_objectArcAlias(dWwTsuboData_getArcName((u32)mType));
    // §811 DN-3: WW BDLs are NOT mount-parsed in this port — a getObjectRes
    // return is the RAW buffer and must go through the consume-time acquirer
    // (parse-from-pristine-copy + session cache). The donor's by-index resolve
    // idiom is preserved by the acquirer's index form.
    J3DModelData* modelData = dExtNpcMount_acquireModelDataByIndex(arc, (int)data().m6C);
    if (modelData == NULL) {
        DuskLog.warn("[WwTsubo] res: '{}' idx {} not resolvable (type {})",
                     arc, (int)data().m6C, (int)mType);
        return false;
    }

    mpModel = mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022);

    bool ret = false;
    if (mType == 7) {
        // donor retail: dRes_INDEX_KMI00X_BRK_KMI_00X_e == 0x7 (donor res header)
        J3DAnmTevRegKey* brk_data =
            (J3DAnmTevRegKey*)dComIfG_getObjectRes(arc, 0x7);
        if (brk_data != NULL) {
            mpBrk = new mDoExt_brkAnm();
            if (mpBrk != NULL && mpBrk->init(modelData, brk_data, 0, 2 /* LOOP */, 1.0f, 0, -1)) {
                ret = true;
            }
        }
        if (!ret) {
            DuskLog.warn("[WwTsubo] type 7 BRK init failed — spine glow anim missing");
        }
    } else {
        ret = true;
        mpBrk = NULL;
    }

    return mpModel != NULL && ret;
}

void daWwTsubo_c::cull_set_draw() {
    fopAcM_setCullSizeSphere(this, data().mCullSphX_Draw, data().mCullSphY_Draw,
                             data().mCullSphZ_Draw, data().mCullSphR_Draw);
}

void daWwTsubo_c::cull_set_move() {
    fopAcM_setCullSizeSphere(this, data().mCullSphX_Move, data().mCullSphY_Move,
                             data().mCullSphZ_Move, data().mCullSphR_Move);
}

void daWwTsubo_c::create_init_cull() {
    fopAcM_SetMtx(this, mpModel->getBaseTRMtx());
    cull_set_draw();
    // donor sea-stage far-cull tuning (stagInfo cull point) joins with the
    // sea pass (#13) — draw-sphere culling above is the working set.
}

void daWwTsubo_c::create_init_cc() {
    mStts.Init(data().m10, 0xff, this);
    mCyl.Set(l_wwTsuboCylSrc);
    mCyl.SetStts(&mStts);
    mCyl.SetR(data().mAcchCirRad);
    mCyl.SetH(data().m70);
    cXyz zeroVec(0.0f, 0.0f, 0.0f);   // donor: SetAtVec/SetTgVec(cXyz::Zero) — receiver takes mutable ref
    mCyl.SetAtVec(zeroVec);
    mCyl.SetTgVec(zeroVec);
    if (cLib_checkBit<u32>(data().mFlag, 0x4 /* DATA_FLAG_4 */)) {
        mCyl.OnTgShield();
    } else {
        mCyl.OnTgNoHitMark();
    }
}

void daWwTsubo_c::create_init_bgc() {
    mAcchCir.SetWall(30.0f, data().mAcchCirRad);
    mAcch.Set(fopAcM_GetPosition_p(this), fopAcM_GetOldPosition_p(this), this, 1,
              &mAcchCir, fopAcM_GetSpeed_p(this), fopAcM_GetAngle_p(this),
              fopAcM_GetShapeAngle_p(this));
    mAcch.ClrWaterNone();
    mAcch.ClrRoofNone();
    mAcch.SetRoofCrrHeight(data().mAcchRoofHeight);
    m4F8 = -G_CM3D_F_INF;
    m4FC = -G_CM3D_F_INF;
    m500 = -G_CM3D_F_INF;
    m504 = false;
    m505 = false;
}

// ============================================================
// donor _create (d_a_tsubo.cpp:1279) — flow preserved whole.
// ============================================================
cPhs_Step daWwTsubo_c::_create() {
    prmZ_init();
    mType = prm_get_type();

    fopAcM_ct(this, daWwTsubo_c);

    if (dWwTsuboData_get((u32)mType) == NULL) {
        return cPhs_ERROR_e;   // №31-C: OOB type refuses loudly at the accessor
    }

    // donor: DATA_FLAG_1 types (kotubo/ootubo) skip resLoad because the WW
    // Always arc is GAME-RESIDENT on the donor. The receiver's resident arc
    // is TP's Always — the WW twin (the "WwAlways" mount, §806) must be
    // resLoaded like any object arc. Boundary difference only: the donor's
    // no-load is an availability fact, not behavior.
    cPhs_Step PVar2 =
        dComIfG_resLoad(&mPhase, dExtWw_objectArcAlias(dWwTsuboData_getArcName((u32)mType)));

    if (PVar2 == cPhs_COMPLEATE_e) {
        if (fopAcM_entrySolidHeap(this, daWwTsubo_solidHeapCB, data().mHeapSize)) {
            create_init_cull();
            create_init_cc();
            create_init_bgc();
            fopAcM_SetGravity(this, data().mGravity);

            // donor DATA_FLAG_10/20 status bits (WW fopAcStts_UNK80000/UNK8000000)
            // carry the same VALUES in the receiver's status word.
            if (cLib_checkBit<u32>(data().mFlag, 0x10)) {
                fopAcM_OnStatus(this, 0x80000);
            }
            if (cLib_checkBit<u32>(data().mFlag, 0x20)) {
                fopAcM_OnStatus(this, 0x8000000);
            }

            fopAcM_posMoveF(this, NULL);
            mAcch.CrrPos(dComIfG_Bgsp());
            mAcch.ClrGroundLanding();

            bool iVar1 = prm_get_stick();
            bool iVar3 = prm_get_moveBg();
            if (iVar1 || iVar3) {
                current.pos = home.pos;
                if (iVar3 && dComIfG_Bgsp().ChkMoveBG_NoDABg(mAcch.m_gnd)) {
                    fopAcM_SetParam(this, fopAcM_GetParam(this) | 0xC000);
                    current.pos.y = mAcch.GetGroundH();
                }
            }

            m680 = 1;
            if (prm_get_spec() == 5 && !is_switch()) {
                m681 = 1;
                fopAcM_SetParam(this, fopAcM_GetParam(this) | 0xC000);
            } else {
                m681 = 0x14;
            }

            m6FC = current.pos.y;
            m6EC = 0.0f;
            m6F0 = cXyz::Zero;

            // [T8] donor attention CARRY flag/distance -> receiver protocol
            // (obj_carry idiom) so the TP player's grab machinery drives us.
            cLib_onBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
            attention_info.distances[fopAc_attn_CARRY_e] = data().mAttnDist;
            fopAcM_OnCarryType(this, fopAcM_CARRY_LIGHT);
            attention_info.position.x = current.pos.x;
            attention_info.position.y = current.pos.y + data().mAttnY;
            attention_info.position.z = current.pos.z;

            m802 = 0;
            m800 = 0;
            if (mType == 7 && !prm_get_stick()) {
                m800 = attrSpine().m30;
            }

            init_rot_clean();
            s32 iVar4 = prm_get_spec();
            if (iVar4 == 0 && !is_switch()) {
                mode_hide_init();
            } else if (iVar4 == 4) {
                mode_appear_init();
            } else {
                mode_wait_init();
            }

            m4F8 = -G_CM3D_F_INF;
            m67F = 0;
            m682 = 1;
            m683 = 0;
            m684 = 0;
            m685 = false;
            m686 = false;
            m811 = 0;

            // donor type-2 follow-particle handles + type-14 kutani ambient:
            // emitters owed with [T4]; the kutani timer stays donor-live.
            if (mType == 14) {
                eff_kutani_init();
            }
            spec_init();

            m7F4 = cXyz::Zero;
            m814 = cXyz::Zero;
            init_mtx();
            m810 = 0;
        } else {
            PVar2 = cPhs_ERROR_e;
        }
    }
    return PVar2;
}

bool daWwTsubo_c::_is_delete() {
    spec_set_actor();
    spec_remove();
    spec_clr_actor();
    return true;
}

BOOL daWwTsubo_c::_delete() {
    // donor: follow-emitter removes ([T4] owed — none created yet).
    // resDelete unconditionally — the receiver resLoads even DATA_FLAG_1
    // types (see _create: WW Always is not receiver-resident).
    dComIfG_resDelete(&mPhase, dExtWw_objectArcAlias(dWwTsuboData_getArcName((u32)mType)));
    return TRUE;
}

// ============================================================
// SPEC (boko-in-pot) machinery — donor d_a_tsubo.cpp:1420-1680, structure
// WHOLE. [T6] spec_make_boko cannot spawn (no fpcNm_BOKO_e); entries stay
// ERROR so every downstream loop is inert-but-faithful.
// ============================================================
void daWwTsubo_c::spec_make_boko(int arg1) {
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.warn("[WwTsubo] [T6] spec boko placement (spec {}) — Boko enemy port owed",
                     (int)prm_get_spec());
    }
    M7A0* ptr = &m7A0[0];
    for (s32 i = 0; i < arg1; i++, ptr++) {
        ptr->m00 = fpcM_ERROR_PROCESS_ID_e;   // donor: fopAcM_create(fpcNm_BOKO_e, ...)
        ptr->m06 = data_spec_boko(i).m00;
        ptr->m08 = ptr->m06;
        ptr->m10 = data_spec_boko(i).m04;
    }
}

void daWwTsubo_c::spec_init() {
    M7A0* ptr = &m7A0[0];
    for (s32 i = 0; i < 3; i++, ptr++) {
        ptr->m00 = fpcM_ERROR_PROCESS_ID_e;
        ptr->m04 = 0;
        ptr->m06 = 0;
        ptr->m08 = 0;
        ptr->m0C = 0.0f;
        ptr->m10 = 0.0f;
        ptr->m14 = 0.0f;
        ptr->m18 = 0;
        M_spec_act[i] = NULL;
    }

    s32 iVar2 = prm_get_spec();
    if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
        spec_make_boko(iVar2);
    }
    spec_set_actor();
    spec_carry_spec();
    spec_clr_actor();
}

void daWwTsubo_c::spec_set_actor() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m00 == fpcM_ERROR_PROCESS_ID_e) {
                    M_spec_act[i] = NULL;
                } else if (!fopAcM_SearchByID(ptr->m00, &M_spec_act[i])) {
                    ptr->m00 = fpcM_ERROR_PROCESS_ID_e;
                }
            }
        }
    }
}

void daWwTsubo_c::spec_clr_actor() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            for (s32 i = 0; i < 3; i++) {
                M_spec_act[i] = NULL;
            }
        }
    }
}

void daWwTsubo_c::spec_carry_spec() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 == 0 && M_spec_act[i] != NULL) {
                    fopAcM_setCarryNow(M_spec_act[i], 0);
                    ptr->m04 = 1;
                }
            }
        }
    }
}

void daWwTsubo_c::spec_mode_carry_init() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0 && M_spec_act[i] != NULL) {
                    fopAcM_setStageLayer(M_spec_act[i]);
                }
            }
        }
    }
}

void daWwTsubo_c::spec_mode_carry(bool arg1) {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0 && M_spec_act[i] != NULL) {
                    f32 fVar1 = data_spec_boko(i).m04;
                    if (arg1) {
                        ptr->m14 = cM_rndF(3.0f);
                        ptr->m18 = 2;
                    } else {
                        ptr->m14 -= 0.8f;
                    }

                    ptr->m10 += ptr->m14;

                    if (ptr->m10 < fVar1) {
                        if (--ptr->m18 <= 0) {
                            ptr->m10 = fVar1;
                            ptr->m14 = 0.0f;
                        } else {
                            ptr->m10 = fVar1 + (ptr->m10 - fVar1) * -0.5f;
                            ptr->m14 *= -0.5f;
                        }
                    }
                }
            }
        }
    }
}

void daWwTsubo_c::spec_mode_put_init() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0 && M_spec_act[i] != NULL) {
                    fopAcM_setRoomLayer(M_spec_act[i], fopAcM_GetRoomNo(this));
                    ptr->m0C = 0.0f;
                    ptr->m10 = data_spec_boko(i).m04;
                    ptr->m14 = 0.0f;
                }
            }
        }
    }
}

void daWwTsubo_c::spec_set_room() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0) {
                    fopAc_ac_c* boko = M_spec_act[i];
                    if (boko != NULL) {
                        s8 room = fopAcM_GetRoomNo(this);
                        boko->tevStr.room_no = room;
                        boko->tevStr.YukaCol = tevStr.YukaCol;   // donor mEnvrIdxOverride == receiver floor poly color
                        fopAcM_SetRoomNo(boko, room);
                    }
                }
            }
        }
    }
}

void daWwTsubo_c::spec_remove() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0) {
                    fopAc_ac_c* boko = M_spec_act[i];
                    if (boko != NULL) {
                        fopAcM_SetRoomNo(boko, fopAcM_GetRoomNo(this));
                        fopAcM_cancelCarryNow(boko);
                        boko->current.angle.y = shape_angle.y - ptr->m06;
                        boko->current.angle.y -= -0x8000;
                        fopAcM_SetSpeedF(boko, cM_rndF(9.0f));
                        fopAcM_GetSpeed(boko).y = cM_rndF(13.0f);
                        ptr->m00 = fpcM_ERROR_PROCESS_ID_e;
                    }
                }
            }
        }
    }
}

void daWwTsubo_c::spec_kill() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                if (ptr->m04 != 0) {
                    fopAc_ac_c* boko = M_spec_act[i];
                    if (boko != NULL) {
                        s8 room = fopAcM_GetRoomNo(this);
                        ptr->m00 = fpcM_ERROR_PROCESS_ID_e;
                        fopAcM_SetRoomNo(boko, room);
                        fopAcM_cancelCarryNow(boko);
                        fopAcM_delete(boko);
                    }
                }
            }
        }
    }
}

void daWwTsubo_c::spec_mtx() {
    if (spec_chk_prm_boko()) {
        s32 iVar2 = prm_get_spec();
        if (iVar2 == 1 || iVar2 == 2 || iVar2 == 3) {
            M7A0* ptr = &m7A0[0];
            for (s32 i = 0; i < 3; i++, ptr++) {
                fopAc_ac_c* boko = M_spec_act[i];
                if (boko != NULL) {
                    // donor: composes the boko ride matrix (daBoko_c::setMatrix)
                    // — inert until the Boko port lands ([T6]).
                    (void)ptr;
                }
            }
        }
    }
}

// ============================================================
// MODES — donor d_a_tsubo.cpp:1683-2095, ported whole.
// ============================================================
void daWwTsubo_c::mode_hide_init() {
    mCyl.OffAtSetBit();
    mCyl.OffTgSetBit();
    mCyl.OffCoSetBit();

    mAcch.SetRoofNone();
    mAcch.SetWallNone();
    mAcch.SetGrndNone();
    mAcch.SetWaterNone();
    mAcch.OffLineCheck();

    m67F = 1;
    fopAcM_SetSpeedF(this, 0.0f);
    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    m678 = 0;
}

void daWwTsubo_c::mode_hide() {
    if (is_switch()) {
        mode_appear_init();
    }
}

void daWwTsubo_c::mode_appear_init() {
    mCyl.OffAtSetBit();
    mCyl.OffTgSetBit();
    mCyl.OnCoSetBit();

    mAcch.SetRoofNone();
    mAcch.SetWallNone();
    mAcch.ClrGrndNone();
    mAcch.SetWaterNone();
    mAcch.OffLineCheck();

    m67F = 1;
    fopAcM_SetSpeedF(this, 0.0f);
    scale.setall(0.01f);
    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);

    if (prm_get_spec() == 4) {
        m808 = attrSpine().m10;
        m80C = attrSpine().m14;
        m802 = attrSpine().m18;
    } else {
        speed.y = cM_rndF(5.0f) + 29.0f;
        gravity = data().mGravity;
        m802 = 0xb;
    }
    m678 = 1;
}

void daWwTsubo_c::mode_appear() {
    if (prm_get_spec() == 4) {
        m802--;
        if (m802 == attrSpine().m1C) {
            dWwTsubo_seStart(this, JA_SE_OBJ_BOKKURI_RECOVER, 0);
        }
        if (m802 == attrSpine().m1E) {
            dWwTsubo_seStart(this, JA_SE_OBJ_BOKKURI_SWELL, 0);
        }

        if (m802 >= attrSpine().m1A) {
            cLib_addCalc(&scale.x, attrSpine().m20, attrSpine().m24, attrSpine().m28,
                         attrSpine().m2C);
            scale.y = scale.x;
            scale.z = scale.x;
        } else {
            f32 tmp = (scale.x - 1.0f);
            m808 -= tmp * attrSpine().m08;
            m808 -= m808 * attrSpine().m00;
            scale.x += m808;
            scale.z = scale.x;
            f32 tmp2 = (scale.y - 1.0f);
            m80C -= tmp2 * attrSpine().m0C;
            m80C -= m80C * attrSpine().m04;
            scale.y += m80C;
        }

        if (m802 <= 0) {
            scale.setall(1.0f);
            mode_wait_init();
        }
    } else {
        if (--m802 <= 0) {
            mode_wait_init();
        } else {
            cLib_chaseF(&scale.x, 1.0f, 0.1f);
            scale.y = scale.x;
            scale.z = scale.x;
        }
        fopAcM_posMoveF(this, mStts.GetCCMoveP());
    }
}

void daWwTsubo_c::mode_wait_init() {
    mCyl.OffAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OnCoSetBit();

    mAcch.ClrRoofNone();
    mAcch.ClrWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OffLineCheck();

    m67F = 0;
    fopAcM_SetSpeedF(this, 0.0f);
    mStts.Init(data().m10, 0xff, this);
    m802 = 0;
    m678 = 2;
}

void daWwTsubo_c::mode_wait() {
    if (prm_get_stick() != false) {
        cLib_onBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    } else if (mAcch.ChkGroundHit()) {
        if (speedF > 0.1f) {
            speedF *= 0.5f;
        } else {
            fopAcM_SetSpeedF(this, 0.0f);
        }
        fopAcM_SetGravity(this, data().mGravity);
        fopAcM_posMoveF(this, mStts.GetCCMoveP());
        cLib_onBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    } else {
        f32 sp10;
        f32 sp0C;
        f32 sp08;
        calc_drop_param(&sp10, &sp0C, &sp08);
        fopAcM_SetGravity(this, sp10);
        daObj::posMoveF_stream(this, mStts.GetCCMoveP(), &cXyz::Zero, sp0C, sp08);
        cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    }
}

void daWwTsubo_c::mode_walk_init() {
    mCyl.OffAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OnCoSetBit();

    mAcch.ClrRoofNone();
    mAcch.ClrWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OffLineCheck();

    m67F = 0;
    init_rot_clean();
    m678 = 3;
}

void daWwTsubo_c::mode_walk() {
    f32 sp10;
    f32 sp0C;
    f32 sp08;
    calc_drop_param(&sp10, &sp0C, &sp08);
    fopAcM_SetGravity(this, sp10);
    cXyz sp20(mStts.GetCCMoveP()->x, 0.0f, mStts.GetCCMoveP()->z);
    if (sp20.getSquareMag() > 0.010000001f) {
        speedF *= 0.9f;
    }

    // donor GetTriPla(m_gnd) pointer form -> receiver bool+plane form.
    cM3dGPla plane;
    bool gotPla = mAcch.ChkGroundHit() && dComIfG_Bgsp().GetTriPla(mAcch.m_gnd, &plane);
    m6F0 += m814;
    f32 fVar2;
    f32 noGradeCos;
    const cXyz* normP;
    if (gotPla) {
        normP = (const cXyz*)&plane.mNormal;
        fVar2 = data().m5C;
        noGradeCos = cM_scos(cM_deg2s((f32)data().m56 - 0.5f));
    } else {
        normP = NULL;
        fVar2 = 0.0f;
        noGradeCos = 0.0f;
    }

    daObj::posMoveF_grade(this, mStts.GetCCMoveP(), &m7F4, sp0C, sp08, normP, fVar2,
                          noGradeCos, &m6F0);

    if (mAcch.ChkGroundHit()) {
        cLib_onBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
        if (speed.abs2XZ() < 0.1f) {
            mode_wait_init();
        }
    } else {
        cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    }
}

void daWwTsubo_c::mode_carry_init() {
    mCyl.OffAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OffCoSetBit();

    mAcch.ClrRoofNone();
    mAcch.ClrWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OffLineCheck();

    m67F = 0;
    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    fopAcM_SetSpeedF(this, 0.0f);
    spec_mode_carry_init();
    if (mType == 2) {
        dWwTsubo_seStart(this, JA_SE_OBJ_WPOT_LIFTUP, 0);
    }

    m802 = 6;
    if (fopAcM_checkHookCarryNow(this)) {
        m684 = 1;
    } else {
        m684 = 0;
    }

    if (mType == 7 && prm_get_stick()) {
        m800 = attrSpine().m30;
        se_pickup_carry_init();
    }

    prm_off_stick();
    m685 = false;
    m686 = false;
    m678 = 4;
}

void daWwTsubo_c::mode_carry() {
    daPy_py_c* player = daPy_getPlayerActorClass();
    const bool tmp = m685;

    if (!m685) {
        if (prm_get_spec() == 6) {
            m685 = true;
            m686 = true;
        } else if (player->getGrabActorID() == fopAcM_GetID(this)) {
            if (player->getGrabUpStart()) {
                m685 = true;
                m686 = true;
            }
        } else {
            m685 = true;
            m686 = false;
        }
    }

    if (m685 && !tmp) {
        init_rot_clean();
        shape_angle.x = 0;
        shape_angle.z = 0;
    }

    // [T8] donor also keys on getRightFootOnGround (carry-step slosh);
    // no receiver equivalent — GrabUpEnd carries the donor intent.
    const bool bVar2 = player->getGrabUpEnd();
    if (bVar2 && mType == 2) {
        eff_drop_water();
        dWwTsubo_seStart(this, JA_SE_LK_CONVEY_WPOT, 0);
    }

    se_pickup_carry();

    if (m810 == 0) {
        fopAcM_GetSpeed(this).y = 0.0f;
    }

    if (fopAcM_checkCarryNow(this) || fopAcM_checkHookCarryNow(this)) {
        spec_mode_carry(bVar2);
    } else {
        m6FC = current.pos.y;
        if (fopAcM_GetSpeedF(this) > 0.0f && m684 == 0) {
            init_rot_throw();
            mode_drop_init();
            mode_drop();
        } else {
            // donor daObj::SetCurrentRoomNo — receiver room-from-ground.
            if (mAcch.ChkGroundHit()) {
                int room = dComIfG_Bgsp().GetRoomId(mAcch.m_gnd);
                if (room >= 0) {
                    fopAcM_SetRoomNo(this, room);
                }
            }
            spec_mode_put_init();
            m683 = 2;
            mode_wait_init();
        }
    }
}

void daWwTsubo_c::mode_drop_init() {
    daPy_py_c* player = daPy_getPlayerActorClass();
    f32 fVar1 = data().m18 + player->getSpeedF() * data().m1C;

    mCyl.OnAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OnCoSetBit();

    mAcch.ClrRoofNone();
    mAcch.ClrWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OnLineCheck();

    m67F = 0;
    mStts.Init(data().m10, 0xff, this);
    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    if (m810 == 0) {
        speed.y = data().m14;
    }
    m810 = 0;
    fopAcM_SetSpeedF(this, fVar1);
    gravity = data().mGravity;
    m678 = 5;
}

void daWwTsubo_c::mode_drop() {
    if (mAcch.ChkGroundHit()) {
        int room = dComIfG_Bgsp().GetRoomId(mAcch.m_gnd);
        if (room >= 0) {
            fopAcM_SetRoomNo(this, room);
        }
    }
    cXyz* movePos = &m7F4;
    cXyz sp14(mStts.GetCCMoveP()->x, 0.0f, mStts.GetCCMoveP()->z);
    if (sp14.getSquareMag() > 0.010000001f) {
        speedF *= 0.9f;
    }
    f32 sp10;
    f32 sp08;
    f32 sp0C;
    calc_drop_param(&sp10, &sp08, &sp0C);
    fopAcM_SetGravity(this, sp10);
    daObj::posMoveF_stream(this, mStts.GetCCMoveP(), movePos, sp08, sp0C);
}

void daWwTsubo_c::mode_sink_init() {
    mCyl.OffAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OnCoSetBit();

    mStts.Init(0xff, 0xff, this);

    mAcch.SetRoofNone();
    mAcch.SetWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OffLineCheck();
    m67F = 0;
    fopAcM_SetGravity(this, data().mGravity + data().m3C);

    f32 sqrt = std::sqrtf(SQUARE(speed.y) + SQUARE(speedF));
    if (sqrt > data().m50) {
        f32 fVar4 = data().m50 / sqrt;
        speed *= fVar4;
        speedF *= fVar4;
    }

    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);
    m678 = 6;
}

void daWwTsubo_c::mode_sink() {
    if (mAcch.ChkGroundHit()) {
        int room = dComIfG_Bgsp().GetRoomId(mAcch.m_gnd);
        if (room >= 0) {
            fopAcM_SetRoomNo(this, room);
        }
    }
    water_tention();
    daObj::posMoveF_stream(this, mStts.GetCCMoveP(), &cXyz::Zero, data().m40, data().m44);
}

void daWwTsubo_c::mode_afl_init() {
    mCyl.OffAtSetBit();
    mCyl.OnTgSetBit();
    mCyl.OnCoSetBit();
    mAcch.ClrRoofNone();
    mAcch.ClrWallNone();
    mAcch.ClrGrndNone();
    mAcch.ClrWaterNone();
    mAcch.OffLineCheck();
    m67F = 0;

    f32 sqrt = std::sqrtf(SQUARE(speed.y) + SQUARE(speedF));
    if (sqrt > data().m50) {
        f32 fVar4 = data().m50 / sqrt;
        speed *= fVar4;
        speedF *= fVar4;
    }

    cLib_offBit<u32>(attention_info.flags, fopAc_AttnFlag_CARRY_e);

    m804 = cM_rndFX(0x8000);
    m806 = cM_rndFX(0x8000);
    m678 = 7;
}

void daWwTsubo_c::mode_afl() {
    if (mAcch.ChkGroundHit()) {
        int room = dComIfG_Bgsp().GetRoomId(mAcch.m_gnd);
        if (room >= 0) {
            fopAcM_SetRoomNo(this, room);
        }
    }
    // [T7] donor daObj::get_path_spd (room-path water current) is a receiver
    // stub — zero stream until the waves/sea pass (#13).
    const cXyz* pStreamSpd = &cXyz::Zero;
    cXyz sp14 = m7F4 * data().m20;
    f32 sp10;
    f32 sp0C;
    f32 sp08;
    calc_afl_param(&sp10, &sp0C, &sp08);
    s16 tmp = (s16)cM_rndF(400.0f) + 0x514;
    m804 += tmp;
    fopAcM_SetGravity(this, sp10 + cM_ssin(m804) * 0.3f);
    water_tention();
    daObj::posMoveF_grade(this, mStts.GetCCMoveP(), pStreamSpd, sp0C, sp08, NULL, 0.0f,
                          0.0f, &sp14);
    s16 tmp2 = (s16)cM_rndF(400.0f) + 0x514;
    m806 += tmp2;
}

// ============================================================
// donor mode_proc_call (retail branch, d_a_tsubo.cpp:2098/2144).
// ============================================================
bool daWwTsubo_c::mode_proc_call() {
    typedef void (daWwTsubo_c::*ModeProcFunc)();
    static ModeProcFunc mode_proc[] = {
        &daWwTsubo_c::mode_hide,
        &daWwTsubo_c::mode_appear,
        &daWwTsubo_c::mode_wait,
        &daWwTsubo_c::mode_walk,
        &daWwTsubo_c::mode_carry,
        &daWwTsubo_c::mode_drop,
        &daWwTsubo_c::mode_sink,
        &daWwTsubo_c::mode_afl,
    };

    if ((fopAcM_checkCarryNow(this) || fopAcM_checkHookCarryNow(this)) && m678 != 4) {
        mode_carry_init();
    }

    cXyz sp08 = current.pos;
    (this->*mode_proc[m678])();
    f32 fVar7 = fopAcM_GetSpeed(this).y;
    crr_pos(sp08);

    if (!damage_bg_proc_directly()) {
        if (!prm_get_moveBg()) {
            bound(fVar7);
        }
        moment_proc_call();

        typedef void (daWwTsubo_c::*SetTensorFunc)();
        static SetTensorFunc set_tensor_proc[] = {
            &daWwTsubo_c::set_tensor_hide,
            &daWwTsubo_c::set_tensor_appear,
            &daWwTsubo_c::set_tensor_wait,
            &daWwTsubo_c::set_tensor_walk,
            &daWwTsubo_c::set_tensor_carry,
            &daWwTsubo_c::set_tensor_drop,
            &daWwTsubo_c::set_tensor_sink,
            &daWwTsubo_c::set_tensor_afl,
        };

        (this->*set_tensor_proc[m678])();

        if (m678 != 4) {
            tevStr.room_no = fopAcM_GetRoomNo(this);
            tevStr.YukaCol = dComIfG_Bgsp().GetPolyColor(mAcch.m_gnd);   // donor mEnvrIdxOverride
        }

        m7F4 *= 0.95f;
        if (m7F4.getSquareMag() < 0.1f) {
            m7F4.setall(0.0f);
        }

        m814 *= 0.6f;
        if (m814.getSquareMag() < 0.1f) {
            m814.setall(0.0f);
        }

        spec_set_room();

        if (mType == 14) {
            eff_kutani_set();
        }
        return true;
    }
    return false;
}

// ============================================================
// DAMAGE — donor d_a_tsubo.cpp:2208-2560.
// ============================================================
void daWwTsubo_c::damaged(int arg1, cBgS_PolyInfo* arg2, bool arg3, const cXyz* arg4) {
    s32 itemNo = prm_get_itemNo();
    s32 itemBitNo = prm_get_itemSave();
    csXyz sp08(0, home.angle.y, 0);
    // [T3] donor fopAcM_createItemFromTable -> the boundary helper.
    dWwTsubo_createItemFromTable(&current.pos, itemNo, itemBitNo,
                                 fopAcM_GetHomeRoomNo(this), &sp08, arg1);
    fopAcM_cancelCarryNow(this);
    fopAcM_cancelHookCarryNow(this);

    if (arg3) {
        // donor type-2 water-pot spill (fopKyM_create 0x19) — WW kankyo proc
        // space; owed with the FX pass ([T4]).
        eff_break_dispatch();
        se_break(arg2);
    } else {
        spec_kill();
    }
    (void)arg4;
}

void daWwTsubo_c::damaged(int arg1, cBgS_PolyInfo* arg2) {
    damaged(arg1, arg2, true, &current.pos);
}

void daWwTsubo_c::damaged_lava() {
    cXyz sp08;
    cXyz* pcVar2;

    if (current.pos.y < m4F8 - 80.0f) {
        sp08.set(current.pos.x, m4F8 - 80.0f, current.pos.z);
        pcVar2 = &sp08;
    } else {
        pcVar2 = &current.pos;
    }

    damaged(2, &mGndChkYogan, true, pcVar2);
}

bool daWwTsubo_c::damage_tg_acc() {
    cCcD_Obj* pcVar4 = mCyl.GetTgHitObj();
    bool iVar9 = false;
    if (pcVar4 != NULL) {
        f32 fVar1 = 1.5f;
        f32 fVar2 = 0.0f;
        // [T2] donor sword-family check under receiver bits.
        u32 type = pcVar4->GetAtType();
        if (type & WWTSUBO_AT_SWING_MASK) {
            fVar1 = 2.0f;
            fVar2 = 9.0f;
        }

        if (fVar1 > 0.0f) {
            f32 abs = m814.abs();
            cXyz sp34 = *mCyl.GetTgRVecP();
            f32 abs2 = sp34.abs();

            if (fVar1 > abs && abs2 > abs) {
                if (abs2 < fVar1) {
                    m814 = sp34;
                } else {
                    m814 = sp34 * (fVar1 / abs2);
                }
                m814.y += fVar2;
                iVar9 = true;
            }
        }

        // [T2] donor AT_TYPE_WIND (deku-leaf gust) -> receiver gale boomerang.
        if (!iVar9 && pcVar4->ChkAtType(AT_TYPE_BOOMERANG)) {
            cCcD_ShapeAttr* shapeAttr = pcVar4->GetShapeAttr();
            cXyz sp28 = cXyz::Zero;
            if (shapeAttr->GetNVec(current.pos, &sp28)) {
                fopAc_ac_c* pfVar7 = mCyl.GetTgHitAc();
                if (pfVar7 != NULL && fopAcM_GetProfName(pfVar7) == fpcNm_ALINK_e) {
                    s16 atan = cM_atan2s(sp28.x, sp28.z);
                    if (cM_scos(pfVar7->shape_angle.y - atan) > 0.866f) {
                        m814 = sp28 * 1.5f;
                        iVar9 = true;
                    }
                }
            }
        }
    }
    return iVar9;
}

bool daWwTsubo_c::damage_cc_proc() {
    bool bVar1 = false;
    if (mCyl.ChkAtHit()) {
        if (mType != 7) {
            damaged(3, NULL);
            bVar1 = true;
        }
        mCyl.ClrAtHit();
    } else if (mCyl.ChkTgHit()) {
        cCcD_Obj* pcVar4 = mCyl.GetTgHitObj();
        if (pcVar4 != NULL &&
            (!cLib_checkBit<u32>(data().mFlag, 0x10) || !pcVar4->ChkAtType(AT_TYPE_HOOKSHOT))) {
            // [T2] WIND -> gale boomerang.
            if (pcVar4->ChkAtType(AT_TYPE_BOOMERANG) && !prm_get_stick()) {
                set_wind_vec();
                if (m678 == 2) {
                    mode_walk_init();
                }
            } else {
                if (damage_tg_acc() && m678 == 2 && mType == 7) {
                    if (mType == 7 && prm_get_stick()) {
                        m800 = attrSpine().m30;
                    }
                    if (prm_get_stick()) {
                        se_pickup();
                    }
                    prm_off_stick();
                    mode_walk_init();
                }

                // [T2] donor per-type damage matrix under receiver bits:
                //   BOMB stays BOMB; SWORD -> swing family;
                //   SKULL_HAMMER -> IRON_BALL (ball & chain);
                //   UNK8 (thrown obj) -> THROW_OBJ, folded into default.
                if ((mType == 3) || (mType == 4)) {
                    if (pcVar4->ChkAtType(AT_TYPE_BOMB)) {
                        damaged(7, NULL);
                        bVar1 = true;
                    } else if (pcVar4->ChkAtType(AT_TYPE_THROW_OBJ)) {
                        damaged(3, NULL);
                        bVar1 = true;
                    } else if (pcVar4->ChkAtType(AT_TYPE_IRON_BALL)) {
                        damaged(1, NULL);
                        bVar1 = true;
                    }
                } else if ((mType == 0) || (mType == 1) || (mType == 2) ||
                           (mType == 5 || (mType == 6)) ||
                           (mType == 8 || (mType == 13 || (mType == 14))) || (mType == 15)) {
                    if (pcVar4->ChkAtType(AT_TYPE_BOMB)) {
                        damaged(7, NULL);
                        bVar1 = true;
                    } else if (pcVar4->GetAtType() & WWTSUBO_AT_SWING_MASK) {
                        damaged(1, NULL);
                        bVar1 = true;
                    } else {
                        damaged(9, NULL);
                        bVar1 = true;
                    }
                } else if (mType == 7) {
                    if (pcVar4->ChkAtType(AT_TYPE_BOMB)) {
                        damaged(7, NULL);
                        bVar1 = true;
                    } else if (pcVar4->ChkAtType(AT_TYPE_IRON_BALL)) {
                        damaged(1, NULL);
                        bVar1 = true;
                    }
                } else if (pcVar4->ChkAtType(AT_TYPE_BOMB)) {
                    damaged(7, NULL);
                    bVar1 = true;
                }
            }
        }

        if (!bVar1) {
            // donor: daObj::HitSeStart with the WW SE-table id. The receiver's
            // HitSeStart would feed a WW id into the Z2 SE space (wrong-sound
            // class, [T5]) — suppressed at the same boundary as every WW SE.
            dWwTsubo_seStart(this, data().mSoundID_Hit, 0);
            dWwTsubo_senv(current.pos, data().m62, fopAcM_GetID(this), data().m63);
            // donor HitEff kikuzu/hibana sparks — [T4] owed with the FX pass.
        }

        mCyl.ClrTgHit();
    }
    return bVar1;
}

bool daWwTsubo_c::damage_bg_proc() {
    bool uVar6 = mAcch.ChkGroundHit();
    bool cVar2 = chk_sink_water();
    bool cVar3 = chk_sink_lava();
    bool bVar7 = false;

    if (m678 == 2 || m678 == 3) {
        if (cVar2 || cVar3) {
            if (cVar2) {
                se_fall_water();
                eff_hit_water_splash();
                init_rot_clean();
                if (data().mGravity + data().m3C >= 0.0f) {
                    mode_afl_init();
                } else {
                    mode_sink_init();
                }
            } else {
                se_fall_lava();
                eff_hit_lava_splash();
                damaged_lava();
                bVar7 = true;
            }
        }
    } else if (m678 == 5) {
        if (cVar2) {
            se_fall_water();
            eff_hit_water_splash();
            if (data().mGravity + data().m3C >= 0.0f) {
                mode_afl_init();
            } else {
                mode_sink_init();
            }
        } else if (cVar3) {
            se_fall_lava();
            eff_hit_lava_splash();
            damaged_lava();
            bVar7 = true;
        } else if (mType == 7 && uVar6) {
            mode_walk_init();
        }
    } else if (m678 == 6) {
        if (chk_sinkdown_water()) {
            damaged(2, m505 ? NULL : &mAcch.m_wtr, false, &current.pos);
            bVar7 = true;
        }
    } else if (m678 == 7 && uVar6 && !chk_sink_water()) {
        mode_wait_init();
    }
    return bVar7;
}

bool daWwTsubo_c::damage_bg_proc_directly() {
    bool uVar11 = mAcch.ChkGroundHit();
    bool uVar8 = mAcch.ChkGroundLanding();
    bool iVar10 = false;

    if (m678 == 2 || m678 == 3) {
        bool uVar7 = mAcch.ChkRoofHit();
        if (uVar8 && m6FC - current.pos.y > data().m38) {
            damaged(2, NULL);
            iVar10 = true;
        } else if (uVar11 && uVar7) {
            f32 roofHeight = mAcch.GetRoofHeight();
            if (roofHeight < current.pos.y + data().mAcchRoofHeight - 10.0f &&
                mAcch.GetRoofHeight() > mAcch.GetGroundH()) {
                damaged(3, NULL);
                iVar10 = true;
            }
        }

        if (uVar11) {
            m6FC = current.pos.y;
        }
    } else if (m678 == 5) {
        bool uVar7 = mAcch.ChkRoofHit();
        bool uVar9 = mAcch.ChkWallHit();
        bool cVar5 = chk_sink_water();
        bool cVar3 = chk_sink_lava();

        s32 type = mType;
        if (type == 7) {
            if (uVar8 && m6FC - current.pos.y > data().m38) {
                damaged(2, NULL);
                iVar10 = true;
            }
            if (uVar11) {
                m6FC = current.pos.y;
            }
        } else if (uVar11) {
            damaged(2, NULL);
            iVar10 = true;
        } else if (uVar9) {
            damaged(3, &mAcchCir);
            iVar10 = true;
        } else if (uVar7) {
            damaged(3, &mAcch.m_roof);
            iVar10 = true;
        }

        if (uVar11 || uVar9 || uVar7 || cVar5 || cVar3) {
            cam_lockoff();
        }
    } else if (m678 == 7) {
        m6FC = current.pos.y;
    }

    if (m681 > 0) {
        m681--;
    } else if (uVar11) {
        if (m680 == 0) {
            if (m678 == 2 || m678 == 3 || m678 == 1 || m678 == 5) {
                if (!iVar10) {
                    if (mType == 2) {
                        eff_drop_water();
                    }

                    u32 uVar12;
                    if (mType == 7) {
                        uVar12 = 0x2d;
                    } else {
                        uVar12 = dComIfG_Bgsp().GetMtrlSndId(mAcch.m_gnd);
                    }

                    dWwTsubo_seStart(this, data().m88, uVar12);
                    if (!chk_sink_water()) {
                        eff_land_smoke();
                    }
                }
                m680 = 1;
            }
        }
    } else {
        m680 = 0;
    }
    return iVar10;
}

bool daWwTsubo_c::damage_kill_proc() {
    bool bVar1 = false;
    if (m800 > 0) {
        m800--;
        if (m800 == 0) {
            damaged(1, NULL);
            bVar1 = true;
        }
    }
    return bVar1;
}

// ============================================================
// position correction + water/lava sensing (donor retail crr_pos family,
// d_a_tsubo.cpp:2564-2662). [T7] daSea through the shims.
// ============================================================
void daWwTsubo_c::crr_pos(const cXyz& arg1) {
    cXyz sp08 = arg1;
    bool bVar1 = prm_get_moveBg();
    if (m678 == 0 || m678 == 4 || m678 == 6 || prm_get_stick()) {
        sp08 = current.pos;
        bVar1 = true;
    }

    mAcch.CrrPos(dComIfG_Bgsp());
    crr_pos_water();
    crr_pos_lava();

    if (dComIfG_Bgsp().ChkMoveBG_NoDABg(mAcch.m_gnd)) {
        if (dComIfG_Bgsp().ChkMoveBG(mAcch.m_gnd)) {
            m682 = 1;
        }

        if (prm_get_moveBg()) {
            fopAcM_SetParam(this, fopAcM_GetParam(this) | 0xC000);
            m6FC = sp08.y = mAcch.GetGroundH();
        }
    }

    if (bVar1) {
        current.pos = sp08;
    }
}

void daWwTsubo_c::crr_pos_water() {
    f32 fVar1 = mAcch.m_wtr.GetHeight();
    bool bVar5 = daSea_ChkArea(current.pos.x, current.pos.z);
    f32 fVar7 = daSea_calcWave(current.pos.x, current.pos.z);
    f32 fVar2 = current.pos.y + data().m04;

    bool bVar3 = mAcch.ChkWaterIn() && fVar2 < fVar1;
    bool bVar4 = bVar5 && fVar2 < fVar7;
    bool uVar6 = false;

    m500 = m4FC;

    if (bVar3 && bVar4) {
        if (fVar1 > fVar7) {
            bVar4 = false;
        } else {
            bVar3 = false;
        }
    }

    if (bVar3) {
        m4FC = fVar1;
        uVar6 = true;
        m505 = 0;
    } else if (bVar4) {
        m4FC = fVar7;
        uVar6 = true;
        m505 = 1;
    } else {
        m4FC = -G_CM3D_F_INF;
        m505 = 0;
    }
    m504 = uVar6;
}

void daWwTsubo_c::crr_pos_lava() {
    if (m67F != 0) {
        m4F8 = -G_CM3D_F_INF;
    } else {
        f32 fVar3 = current.pos.z;
        f32 fVar1 = old.pos.y;
        cXyz sp08(current.pos.x, fVar1 + data().m04 + 1.0f, fVar3);
        mGndChkYogan.SetPos(&sp08);
        m4F8 = dComIfG_Bgsp().GroundCross(&mGndChkYogan);
    }
}

void daWwTsubo_c::water_tention() {
    if (chk_sink_water()) {
        if (m4FC != -G_CM3D_F_INF && m500 != -G_CM3D_F_INF) {
            f32 fVar2 = m4FC - m500;
            f32 fVar1;
            if (fVar2 < 0.0f) {
                fVar1 = fVar2 * 0.8f;
            } else {
                fVar1 = fVar2 * 0.2f;
            }
            current.pos.y += fVar1;
        }
    }
}

// donor reflect (d_a_tsubo.cpp:2665) — receiver GetTriPla bool+plane form
// (the same shape the receiver's own obj_carry bound() uses).
f32 daWwTsubo_c::reflect(cXyz* pos, const cBgS_PolyInfo& pTri, f32 rebound) {
    cM3dGPla plane;
    f32 ret;
    if (dComIfG_Bgsp().GetTriPla(pTri, &plane)) {
        cXyz sp18;
        VECReflect(pos, &plane.mNormal, &sp18);
        f32 fVar5 = sp18.inprod(*(cXyz*)&plane.mNormal);
        f32 dVar4 = 1.0f - (1.0f - rebound) * fVar5;
        f32 tmp = dVar4 * pos->abs();
        *pos = sp18 * tmp;
        ret = dVar4;
    } else {
        ret = 0.0f;
    }
    return ret;
}

void daWwTsubo_c::bound(f32 arg1) {
    if (mType == 7) {
        cXyz sp28(speed.x, arg1, speed.z);
        f32 abs = sp28.abs();

        if (abs > 8.0f) {
            bool uVar8 = mAcch.ChkWallHit();
            bool uVar7 = mAcch.ChkGroundLanding();
            bool uVar5 = mAcch.ChkRoofHit();
            cXyz sp1C = sp28;

            f32 dVar9 = 0.0f;
            if (uVar8) {
                f32 dVar10 = reflect(&sp1C, mAcchCir, 0.5f);
                if (dVar10 > dVar9) {
                    dVar9 = dVar10;
                }
            }
            if (uVar7) {
                f32 dVar10 = reflect(&sp1C, mAcch.m_gnd, 0.6f);
                if (dVar10 > dVar9) {
                    dVar9 = dVar10;
                }
            }
            if (uVar5) {
                f32 dVar10 = reflect(&sp1C, mAcch.m_roof, 0.4f);
                if (dVar10 > dVar9) {
                    dVar9 = dVar10;
                }
            }

            if (uVar8 || uVar7 || uVar5) {
                speed = sp1C;
                fopAcM_SetSpeedF(this, sp1C.absXZ());
                current.angle.y = cM_atan2s(sp1C.x, sp1C.z);

                s32 uVar6;
                if (abs > 45.0f) {
                    uVar6 = 100;
                } else {
                    uVar6 = abs * dVar9 * 2.2222223f;
                }

                if (uVar6 != 0) {
                    dWwTsubo_seStart(this, JA_SE_OBJ_BOKKURI_BOUND, uVar6);
                }
            }
        }
    } else {
        if (mAcch.ChkWallHit()) {
            speedF *= 0.9f;
        }
    }
}

// donor moment_* per-type hooks are all EMPTY in retail (d_a_tsubo.cpp:
// 2742-2803); only the walk-mode ground-normal fade below is live.
void daWwTsubo_c::moment_proc_call() {
    m6F0 = cXyz::Zero;

    if (m678 == 3) {
        cM3dGPla plane;
        bool gotPla = mAcch.ChkGroundHit() && dComIfG_Bgsp().GetTriPla(mAcch.m_gnd, &plane);
        if (!gotPla) {
            cLib_addCalc0(&m6EC, 0.4f, data().mAcchRoofHeight * 0.125f);
        }
        // donor dispatches the (empty) per-type moment with the normal.
    }
}

void daWwTsubo_c::set_wind_vec() {
    cCcD_Obj* pcVar3 = mCyl.GetTgHitObj();
    // [T2] WIND -> gale boomerang.
    if (pcVar3 != NULL && pcVar3->ChkAtType(AT_TYPE_BOOMERANG)) {
        cXyz sp48 = *mCyl.GetTgRVecP();
        f32 fVar7 = sp48.getSquareMag();
        if (fVar7 > 31684.0f) {
            sp48 *= 178.0f / std::sqrtf(fVar7);
        }
        cCcD_ShapeAttr* shapeAttr = pcVar3->GetShapeAttr();
        cXyz sp3C = cXyz::Zero;
        f32 fVar1 = 1.0f;
        if (shapeAttr->GetNVec(current.pos, &sp3C)) {
            sp3C *= 45.0f;
            fopAc_ac_c* pfVar4 = mCyl.GetTgHitAc();
            if (pfVar4 != NULL && fopAcM_GetProfName(pfVar4) == fpcNm_ALINK_e) {
                s16 iVar5 = cM_atan2s(sp3C.x, sp3C.z);
                f32 cos = cM_scos(pfVar4->shape_angle.y - iVar5);
                if (cos > 0.866f) {
                    fVar1 = cos * 2.0f + 1.0f;
                } else {
                    fVar1 = 0.0f;
                }
            }
        }

        f32 fVar2;
        if (fVar7 > 0.01f) {
            fVar7 = 1.0f;
            fVar2 = 0.05f;
        } else {
            fVar7 = 0.0f;
            fVar2 = 1.0f;
        }

        m7F4 = (sp48 * fVar7) + (sp3C * fVar2) * fVar1;
    }
}

// ============================================================
// POSE TENSOR — donor d_a_tsubo.cpp:2884-3058, whole.
// ============================================================
void daWwTsubo_c::init_mtx() {
    mDoMtx_identity(mPoseMtx);
    m6B0 = cXyz::Zero;
    set_tensor(&cXyz::BaseX);
    set_mtx();
}

void daWwTsubo_c::set_mtx() {
    mpModel->setBaseScale(scale * data().mModelScale);

    s32 type = mType;
    f32 fVar1;
    if (type == 13 || type == 7) {
        fVar1 = data().mAcchRoofHeight * 0.5f * scale.y;
    } else {
        fVar1 = 0.0f;
    }

    if (m678 == 4 && m685) {
        mDoMtx_stack_c::transS(current.pos.x, current.pos.y + m6EC + fVar1, current.pos.z);
        mDoMtx_stack_c::concat(mPoseMtx);
    } else {
        mDoMtx_stack_c::transS(current.pos.x, current.pos.y + data().m04 + m6EC + fVar1,
                               current.pos.z);
        mDoMtx_stack_c::concat(mPoseMtx);
        mDoMtx_stack_c::transM(0.0f, -data().m04, 0.0f);
    }

    mpModel->setBaseTRMtx(mDoMtx_stack_c::get());

    if (mType == 2) {
        static cXyz offset_pos(0.0f, 85.0f, 0.0f);
        mDoMtx_stack_c::multVec(&offset_pos, &m700);
    }
    spec_mtx();
}

void daWwTsubo_c::set_tensor(const cXyz* pos) {
    // donor: axis-angle quaternion (m68E about `pos`) -> mPoseMtx; the
    // receiver's MTXRotAxisRad composes the identical rotation directly.
    MTXRotAxisRad(mPoseMtx, (Vec*)pos, m68E.Radian());
    mDoMtx_ZXYrotM(mPoseMtx, shape_angle.x, shape_angle.y, shape_angle.z);
}

void daWwTsubo_c::init_rot_throw() {
    m688.Val((s16)data().m28);
    m688 *= cM_rnd();
    m68A.Val((s16)cM_rndFX(0x8000));
    m68C.Val((s16)data().m2A);
    m68E.Val(cSAngle::_0);
    m690.Val((s16)data().m2C);
    m692.Val(current.angle.y);
}

void daWwTsubo_c::init_rot_clean() {
    m688.Val(cSAngle::_0);
    m68A.Val(cSAngle::_0);
    m68C.Val(cSAngle::_0);
    m68E.Val(cSAngle::_0);
    m690.Val(cSAngle::_0);
    m692.Val(current.angle.y);
}

void daWwTsubo_c::set_tensor_hide() {
    set_tensor_wait();
}

void daWwTsubo_c::set_tensor_appear() {
    set_tensor_wait();
}

void daWwTsubo_c::set_tensor_wait() {
    mDoMtx_stack_c::YrotS(m692.Val());
    mDoMtx_stack_c::XrotM(m68A.Val());
    mDoMtx_stack_c::ZrotM(m688.Val());
    cXyz sp14;
    mDoMtx_stack_c::multVecSR(&cXyz::BaseX, &sp14);
    set_tensor(&sp14);
}

void daWwTsubo_c::set_tensor_walk() {
    cXyz sp28 = current.pos - old.pos;
    f32 abs = sp28.abs() * 2.0f / data().mAcchRoofHeight;
    f32 dVar3 = m690.Radian();
    m690.Val(cM_rad2s(dVar3 + (abs - dVar3) * 0.7f));
    m68E += m690;
    m692.Val(current.angle.y);

    mDoMtx_stack_c::YrotS(m692.Val());
    mDoMtx_stack_c::XrotM(m68A.Val());
    mDoMtx_stack_c::ZrotM(m688.Val());
    cXyz sp1C;
    mDoMtx_stack_c::multVecSR(&cXyz::BaseX, &sp1C);
    set_tensor(&sp1C);
}

void daWwTsubo_c::set_tensor_carry() {
    set_tensor_wait();
}

void daWwTsubo_c::set_tensor_drop() {
    f32 dVar2 = m68C.Radian();
    m68C -= cSAngle(cM_rad2s(dVar2 * data().m30 + dVar2 * std::fabsf(dVar2) * data().m34));
    m68A += m68C;

    dVar2 = m690.Radian();
    m690 -= cSAngle(cM_rad2s(dVar2 * data().m30 + dVar2 * std::fabsf(dVar2) * data().m34));
    m68E += m690;

    mDoMtx_stack_c::YrotS(m692.Val());
    mDoMtx_stack_c::XrotM(m68A.Val());
    mDoMtx_stack_c::ZrotM(m688.Val());
    cXyz sp08;
    mDoMtx_stack_c::multVecSR(&cXyz::BaseX, &sp08);
    set_tensor(&sp08);
}

void daWwTsubo_c::set_tensor_sink() {
    f32 dVar2 = m68C.Radian();
    m68C -= cSAngle(cM_rad2s(dVar2 * data().m48 + dVar2 * std::fabsf(dVar2) * data().m4C));
    m68A += m68C;

    dVar2 = m690.Radian();
    m690 -= cSAngle(cM_rad2s(dVar2 * data().m48 + dVar2 * std::fabsf(dVar2) * data().m4C));
    m68E += m690;

    mDoMtx_stack_c::YrotS(m692.Val());
    mDoMtx_stack_c::XrotM(m68A.Val());
    mDoMtx_stack_c::ZrotM(m688.Val());
    cXyz sp08;
    mDoMtx_stack_c::multVecSR(&cXyz::BaseX, &sp08);
    set_tensor(&sp08);
}

void daWwTsubo_c::set_tensor_afl() {
    f32 dVar2 = m68C.Radian();
    m68C -= cSAngle(cM_rad2s(dVar2 * data().m48 + dVar2 * std::fabsf(dVar2) * data().m4C));
    m68A += m68C;

    dVar2 = m68E.Radian();
    m690 += cSAngle(cM_rad2s(dVar2 * -0.0005f));
    m690 += cSAngle((s16)(cM_ssin(m806) * 4.0f));

    dVar2 = m690.Radian();
    m690 -= cSAngle(cM_rad2s(dVar2 * data().m48 + dVar2 * std::fabsf(dVar2) * data().m4C));
    m68E += m690;

    mDoMtx_stack_c::YrotS(m692.Val());
    mDoMtx_stack_c::XrotM(m68A.Val());
    mDoMtx_stack_c::ZrotM(m688.Val());
    cXyz sp08;
    mDoMtx_stack_c::multVecSR(&cXyz::BaseX, &sp08);
    set_tensor(&sp08);
}

// ============================================================
// EFFECTS — [T4] pot classes route to the receiver's own pot-shard system;
// non-pot classes LOUD-once owed (WW JPA pass). Pillars native.
// ============================================================
void daWwTsubo_c::eff_drop_water() {
    // donor WPOT_PITYA follow emitters — [T4] owed (needs the WW JPA bank).
}

void daWwTsubo_c::eff_land_smoke() {
    // donor daObj::make_land_effect — [T4] owed (no receiver equivalent yet).
}

// ============================================================
// [T4] DONOR BREAK FX — the donor's own eff_break_* family, executed with
// ============================================================================
// §843 DONOR EFFECT IDS — the §396 LAMP CLASS. The receiver's ID_AK_*/ID_IT_*
// enum names carry TP VALUES; the donor's dPa_name values differ, and the WW
// supplemental router (sWwCommon / dPa_wwWindlineResRM) only serves ids in its
// table. Emitting the receiver constant (TUBOHAHEN = 0x003 receiver-side) asked
// for an id with no donor meaning AND no row — the refusal was correct, and the
// visible symptom was pots vanishing with no shards (user report, tale §843).
// Values read from WW DP include/d/d_particle_name.h at the cited lines.
// INTEGRATOR VERIFY (tale §839 gate, run before landing): offline JPAC1-00 walk
// of the staged common.jpc via the receiver's own parser field
// (ww_jpa.cpp:59 — resId = be16(ptcl + 0x18)); header declared 193 resources,
// 193 JEFF blocks found, all five ids PRESENT. Control 0x0031 present.
// ============================================================================
#define WW_ID_AK_JN_M_TUBOHAHEN  0x0017  // donor d_particle_name.h:56  (pot shards)
#define WW_ID_AK_JN_TUBOKONAGONA 0x0018  // :57                          (pot dust)
#define WW_ID_IT_JN_TR_HAHEN_A   0x03E5  // :186                         (barrel)
#define WW_ID_IT_JN_TR_HAHEN_C   0x03E7  // :188                         (stool/pail/hbox2S/woodS)
#define WW_ID_IT_JN_DOKURO00     0x03E8  // :189                         (skull/try)
// BOKKURI_MATSU00 (pinecone) donor id is 0x816A — the 0x8000 bit is the
// SCENE-pack discriminator (dPa_RM, d_particle_name.h:8), so it lives in WW's
// per-stage Pscene jpc, NOT common.jpc. The WW scene-bank serve is a SEPARATE
// unported surface: pinecone stays on the receiver id and stays OWED here.

// DONOR CONTENT through the receiver's engine services (covenant: WW rules;
// TP only as engine/interaction). The scoping receipts (tale §811-class):
// every donor effect id (AK_JN_M_TUBOHAHEN 0x003, AK_JN_TUBOKONAGONA 0x043,
// IT_JN_TR_HAHEN_A/C, IT_JN_DOKURO00, IT_SN_BOKKURI_MATSU00) exists in the
// receiver's own particle enum — the common bank is WW-lineage and kept the
// legacy entries. The shard MODEL is the donor's own (WwAlways-served
// MPM_TUBO, donor res header indices 0x31 BDL / 0x67 BTP). The daObjEff
// smoke companions are a separate donor actor — LOUD-once owed.
// ============================================================
void daWwTsubo_c::eff_break_dispatch() {
    // donor eff_break_proc table (d_a_tsubo.cpp:2222) — order verbatim.
    typedef void (daWwTsubo_c::*EffBreakProcFunc)();
    static EffBreakProcFunc eff_break_proc[] = {
        &daWwTsubo_c::eff_break_tsubo,
        &daWwTsubo_c::eff_break_tsubo,
        &daWwTsubo_c::eff_break_tsubo,
        &daWwTsubo_c::eff_break_barrel,
        &daWwTsubo_c::eff_break_stool,
        &daWwTsubo_c::eff_break_skull,
        &daWwTsubo_c::eff_break_pail,
        &daWwTsubo_c::eff_break_spine,
        &daWwTsubo_c::eff_break_hbox2S,
        &daWwTsubo_c::eff_break_try,
        &daWwTsubo_c::eff_break_try,
        &daWwTsubo_c::eff_break_try,
        &daWwTsubo_c::eff_break_try,
        &daWwTsubo_c::eff_break_pinecone,
        &daWwTsubo_c::eff_break_tsubo,
        &daWwTsubo_c::eff_break_woodS,
    };
    (this->*eff_break_proc[mType & 0xF])();
}

static void dWwTsubo_effSmokeOwed(const char* i_which) {
    // donor daObjEff::Act_c::make_*_smoke — its own effect actor, not yet
    // ported. LOUD-once; the JPA half of each break fires above it.
    static bool s_warned = false;
    if (!s_warned) {
        s_warned = true;
        DuskLog.info("[WwTsubo] [T4] daObjEff smoke owed (first: {})", i_which);
    }
}

// donor eff_break_tsubo (d_a_tsubo.cpp:3094): WW shard-model emitter
// (MPM_TUBO through the engine's model callback — the receiver form of the
// donor's dPa_J3DmodelEmitter_c) + the KONAGONA dust with the donor's
// per-type texture select (data m80) and tevStr colors.
void daWwTsubo_c::eff_break_tsubo() {
    u16 texSel = (u16)data().m80;
    // §811 DN-3: shard MODEL through the consume-time acquirer (raw-cast is
    // the crash class). The BTP stays a direct res fetch — anm types keep the
    // donor's mount-parse path (DN-3 removes only the BDL family from it).
    J3DModelData* tuboBmd =
        dExtNpcMount_acquireModelDataByIndex("WwAlways", 0x31 /* donor ALWAYS_BDL_MPM_TUBO */);
    J3DAnmTexPattern* tuboBtp =
        (J3DAnmTexPattern*)dComIfG_getObjectRes("WwAlways", 0x67 /* donor ALWAYS_BTP_MPM_TUBO */);
    cXyz pscl(data().mParticleScale.x, data().mParticleScale.y, data().mParticleScale.z);

    if (tuboBmd != NULL && tuboBtp != NULL) {
        // §826 lineage receipt: dPa_modelEcallBack IS the donor model-emitter
        // system's descendant (becomeImmortalEmitter in both ctor paths; the
        // donor's per-particle draw() loop inverted into backend callbacks;
        // model_c = the donor's J3DTexNoAnm/btp machinery internalized) — the
        // donor's `new dPa_J3DmodelEmitter_c + addModelEmitter` expressed in
        // the descendant API. Donor's own emitter NULL-check restored (the
        // donor guards its particle_set return; d_a_tsubo.cpp:3100).
        JPABaseEmitter* emitter = dComIfGp_particle_set(
            WW_ID_AK_JN_M_TUBOHAHEN, &current.pos, NULL, NULL, 0xFF,
            &dPa_modelEcallBack::getEcallback(), fopAcM_GetRoomNo(this), NULL, NULL, &pscl);
        if (emitter != NULL) {
            // ============================================================
            // [WwTsubo-probe] §832 discriminator, sight-only. setModel's
            // return was IGNORED — model_c::set returns false on a busy
            // slot (mModelData != NULL), which would be an INVISIBLE
            // shatter with every §832 elimination still true. LOUD-once
            // success receipt splits "chain executed" from "never drawn"
            // (draw-side twin probe in d_particle.cpp drawModel).
            // ============================================================
            // (7-arg int overload — the 6-arg convenience wrapper returns void
            // and forwards exactly these arguments with the 0 inserted.)
            int setOk = dPa_modelEcallBack::setModel(emitter, tuboBmd, tevStr, 3,
                                                     (void*)tuboBtp, 0, (u8)texSel);
            static bool s_probed = false;
            if (!s_probed) {
                s_probed = true;
                DuskLog.info("[WwTsubo-probe] shatter chain: emitter={} setModel={} "
                             "texSel={} bmd={} btp={} scl=({:.2f},{:.2f},{:.2f})",
                             (void*)emitter, setOk, (int)texSel, (void*)tuboBmd,
                             (void*)tuboBtp, pscl.x, pscl.y, pscl.z);
            }
            if (!setOk) {
                static bool s_setwarn = false;
                if (!s_setwarn) {
                    s_setwarn = true;
                    DuskLog.warn("[WwTsubo-probe] setModel REFUSED (busy slot?) — "
                                 "shatter emitter runs modelless = invisible break");
                }
            }
        } else {
            static bool s_warned = false;
            if (!s_warned) {
                s_warned = true;
                DuskLog.warn("[WwTsubo] [T4] TUBOHAHEN emitter refused (id {:#x}) — "
                             "resource absent from the receiver common bank?",
                             (int)ID_AK_JN_M_TUBOHAHEN);
            }
        }
    } else {
        DuskLog.warn("[WwTsubo] [T4] MPM_TUBO shard model unresolvable from WwAlways");
    }

    GXColor color;
    color.r = tevStr.TevColor.r;
    color.g = tevStr.TevColor.g;
    color.b = tevStr.TevColor.b;
    color.a = tevStr.TevColor.a;
    dComIfGp_particle_set(WW_ID_AK_JN_TUBOKONAGONA, &current.pos, NULL, NULL, 0xFF,
                          dPa_control_c::getTsuboSelectTexEcallBack(texSel),
                          fopAcM_GetRoomNo(this), &color, &tevStr.TevKColor, &pscl);
}

// donor eff_break_barrel (d_a_tsubo.cpp:3083): HAHEN_A splinters at
// half-height with K0 colors + squashed emitter scale; barrel smoke owed.
void daWwTsubo_c::eff_break_barrel() {
    cXyz sp18(current.pos.x, current.pos.y + data().mAcchRoofHeight * 0.5f, current.pos.z);
    // donor squashes the emitter (setEmitterScale 1,0.8,1) — the receiver's
    // emitter handle is opaque (no tweak surface); the donor scale rides the
    // particle-scale argument instead, tweak-exactness owed with the FX pass.
    static cXyz barrel_scl(1.0f, 0.8f, 1.0f);
    dComIfGp_particle_set(WW_ID_IT_JN_TR_HAHEN_A, &sp18, NULL, NULL, 0xFF, NULL,
                          fopAcM_GetRoomNo(this), &tevStr.TevKColor, &tevStr.TevKColor,
                          &barrel_scl);
    dWwTsubo_effSmokeOwed("barrel");
}

// donor eff_break_stool (d_a_tsubo.cpp:3127): HAHEN_C at +35y, donor
// particle scale (1, 0.8, 1) + directional speed 25; stool smoke owed.
void daWwTsubo_c::eff_break_stool() {
    cXyz sp18(current.pos.x, current.pos.y + 35.0f, current.pos.z);
    static cXyz particle_scl(1.0f, 0.8f, 1.0f);
    // donor also sets directional speed 25 — opaque emitter handle receiver-
    // side, tweak-exactness owed with the FX pass (same note as barrel).
    dComIfGp_particle_set(WW_ID_IT_JN_TR_HAHEN_C, &sp18, NULL, NULL, 0xFF, NULL,
                          fopAcM_GetRoomNo(this), &tevStr.TevKColor, &tevStr.TevKColor,
                          &particle_scl);
    dWwTsubo_effSmokeOwed("stool");
}

// donor eff_break_skull (d_a_tsubo.cpp:3139): DOKURO00 at +20y; smoke owed.
void daWwTsubo_c::eff_break_skull() {
    cXyz sp18(current.pos.x, current.pos.y + 20.0f, current.pos.z);
    dComIfGp_particle_set(WW_ID_IT_JN_DOKURO00, &sp18, NULL, NULL, 0xFF, NULL,
                          fopAcM_GetRoomNo(this), &tevStr.TevKColor, &tevStr.TevKColor, NULL);
    dWwTsubo_effSmokeOwed("skull");
}

void daWwTsubo_c::eff_break_pail() {
    eff_break_stool();   // donor: alias (d_a_tsubo.cpp:3147)
}

void daWwTsubo_c::eff_break_spine() {
    eff_break_pinecone();   // donor: alias (d_a_tsubo.cpp:3152)
}

void daWwTsubo_c::eff_break_hbox2S() {
    eff_break_stool();   // donor: alias (d_a_tsubo.cpp:3157)
}

void daWwTsubo_c::eff_break_try() {
    eff_break_skull();   // donor: alias (d_a_tsubo.cpp:3162)
}

// donor eff_break_pinecone (d_a_tsubo.cpp:3167): BOKKURI_MATSU00 at
// half-height; pinecone smoke owed.
void daWwTsubo_c::eff_break_pinecone() {
    cXyz sp18(current.pos.x, current.pos.y + data().mAcchRoofHeight * 0.5f, current.pos.z);
    dComIfGp_particle_set(ID_IT_SN_BOKKURI_MATSU00, &sp18, NULL, NULL, 0xFF, NULL,
                          fopAcM_GetRoomNo(this), &tevStr.TevKColor, &tevStr.TevKColor, NULL);
    dWwTsubo_effSmokeOwed("pinecone");
}

void daWwTsubo_c::eff_break_woodS() {
    eff_break_stool();   // donor: alias (d_a_tsubo.cpp:3175)
}

void daWwTsubo_c::eff_hit_water_splash() {
    // [T4] donor: WW's own fopKyM_createWpillar kankyo actor (pos, mBC, mC0).
    // The receiver's same-named function spawns TP's pillar actor — a TP
    // content stand-in, so NOT called (covenant ruling 2026-08-12). Owed with
    // the WW kankyo-FX pass.
}

void daWwTsubo_c::eff_hit_lava_splash() {
    // [T4] donor: WW's own fopKyM_createMpillar (pos, mC4). Same posture as
    // the water pillar above — owed, no TP stand-in.
}

void daWwTsubo_c::eff_kutani_set() {
    if (m798 != 0 && --m798 == 0) {
        // donor AK_SN_KUTANIVESSEL ambient emitter — [T4] owed.
    }
}

void daWwTsubo_c::eff_kutani_init() {
    u8 fVar1 = cM_rndF(30.0f);
    m798 = fVar1 + 1;
}

bool daWwTsubo_c::chk_sink_lava() {
    if (m67F != 0) {
        return false;
    }
    f32 fVar1 = data().m04 < 20.0f ? data().m04 : 20.0f;
    return m4F8 > current.pos.y + fVar1;
}

bool daWwTsubo_c::chk_sink_water() {
    return m504;
}

bool daWwTsubo_c::chk_sinkdown_water() {
    return m4FC != -G_CM3D_F_INF && m4FC > current.pos.y + data().mAcchRoofHeight + 50.0f;
}

void daWwTsubo_c::calc_drop_param(f32* outArg1, f32* outArg2, f32* outArg3) const {
    bool bVar5 = m4FC != -G_CM3D_F_INF;
    bool fVar5 = m4F8 > current.pos.y;

    if (bVar5 || fVar5) {
        f32 fVar7;
        if (bVar5 && fVar5) {
            if (m4F8 > m4FC) {
                fVar7 = m4F8;
            } else {
                fVar7 = m4FC;
            }
        } else {
            if (bVar5) {
                fVar7 = m4FC;
            } else {
                fVar7 = m4F8;
            }
        }

        f32 fVar3 = current.pos.y - fVar7;
        f32 fVar0 = 0.0f;

        if (fVar3 >= 0.0f) {
            fVar0 = 0.0f;
        } else if (fVar3 <= -data().mAcchRoofHeight) {
            fVar0 = 0.5f;
        } else {
            fVar0 = -fVar3 * (0.5f / data().mAcchRoofHeight);
        }

        f32 fVar6 = 1.0f - fVar0;

        *outArg2 = fVar0 * data().m40 + fVar6 * data().m20;
        *outArg3 = fVar0 * data().m44 + fVar6 * data().m24;

        f32 gravityCalc = fVar0 * data().m3C + data().mGravity;
        f32 gravityMax = data().mGravity * 0.1f;
        if (gravityCalc > gravityMax) {
            *outArg1 = gravityMax;
        } else {
            *outArg1 = gravityCalc;
        }
    } else {
        *outArg2 = data().m20;
        *outArg3 = data().m24;
        *outArg1 = data().mGravity;
    }
}

void daWwTsubo_c::calc_afl_param(f32* outArg1, f32* outArg2, f32* outArg3) const {
    bool bVar5 = m4FC != -G_CM3D_F_INF;
    bool fVar5 = m4F8 > current.pos.y;

    if (bVar5 || fVar5) {
        f32 fVar7;
        if (bVar5 && fVar5) {
            if (m4F8 > m4FC) {
                fVar7 = m4F8;
            } else {
                fVar7 = m4FC;
            }
        } else {
            if (bVar5) {
                fVar7 = m4FC;
            } else {
                fVar7 = m4F8;
            }
        }
        f32 fVar3 = current.pos.y - fVar7;
        f32 fVar0 = 0.0f;

        if (fVar3 >= 0.0f) {
            fVar0 = 0.0f;
        } else {
            if (fVar3 <= -data().mAcchRoofHeight) {
                fVar0 = 1.0f;
            } else {
                fVar0 = -fVar3 * (1.0f / data().mAcchRoofHeight);
            }
        }

        f32 fVar6 = 1.0f - fVar0;

        *outArg2 = fVar0 * data().m40 + fVar6 * data().m20;
        *outArg3 = fVar0 * data().m44 + fVar6 * data().m24;
        *outArg1 = fVar0 * data().m3C + data().mGravity;
    } else {
        *outArg2 = data().m20;
        *outArg3 = data().m24;
        *outArg1 = data().mGravity;
    }
}

// ============================================================
// SE — donor material-keyed variants preserved; emission through [T5].
// ============================================================
void daWwTsubo_c::se_fall_water() {
    dWwTsubo_seStart(this, data().mSoundID_FallWater, 0x13);
    dWwTsubo_senv(current.pos, data().m64, fopAcM_GetID(this), data().m65);
}

void daWwTsubo_c::se_fall_lava() {
    dWwTsubo_seStart(this, data().mSoundID_FallLava, 0x17);
    dWwTsubo_senv(current.pos, data().m64, fopAcM_GetID(this), data().m65);
}

void daWwTsubo_c::se_break(cBgS_PolyInfo* arg1) {
    cBgS_PolyInfo* polyInfoArr[] = {
        arg1,
        &mAcch.m_gnd,
    };

    u32 materialSoundId = 0;
    for (s32 i = 0; i < 2; i++) {
        if (polyInfoArr[i] != NULL && polyInfoArr[i]->GetBgIndex() >= 0 &&
            polyInfoArr[i]->GetBgIndex() < 0x100) {
            materialSoundId = dComIfG_Bgsp().GetMtrlSndId(*polyInfoArr[i]);
            break;
        }
    }

    dWwTsubo_seStart(this, data().mSoundID_Break, materialSoundId);
    dWwTsubo_senv(current.pos, data().m60, fopAcM_GetID(this), data().m61);
}

void daWwTsubo_c::se_pickup_carry() {
    if (mType == 7) {
        if (m811 != 0 && daPy_getPlayerActorClass()->getGrabUpStart()) {
            m811 = 0;
            se_pickup();
        }
    }
}

void daWwTsubo_c::se_pickup_carry_init() {
    if (mType == 7) {
        m811 = 1;
    }
}

void daWwTsubo_c::se_pickup() {
    if (mType == 7) {
        dWwTsubo_seStart(this, JA_SE_OBJ_BOKKURI_PICK_UP, 0);
    }
}

void daWwTsubo_c::cam_lockoff() const {
    // donor: dComIfGp_getCamera(0)->mCamera.ForceLockOff(mBsPcId) — release
    // the Z-target when the thrown pot dies. Receiver attention drops dead
    // actors on its own; the explicit force-off joins with the camera lane.
}

// ============================================================
// donor _execute (d_a_tsubo.cpp:3489) — flow preserved whole.
// ============================================================
BOOL daWwTsubo_c::_execute() {
    cull_set_move();

    bool bVar4;
    if (prm_get_spec() == 5) {
        bVar4 = is_switch() == false;
    } else {
        bVar4 = false;
    }

    if (!bVar4) {
        if (m682 != 0 || m678 != 2 || !mAcch.ChkGroundHit() || mAcch.ChkGroundLanding() ||
            prm_get_cull() == 0 || !fopAcM_cullingCheck(this) || m800 != 0 ||
            prm_get_moveBg()) {
            m682 = 0;
            BOOL bVar6 = TRUE;

            spec_set_actor();

            if (!damage_cc_proc()) {
                if (!damage_bg_proc() && !damage_kill_proc()) {
                    if (m683 != 0) {
                        m683--;
                    }

                    if (mode_proc_call()) {
                        bVar6 = FALSE;
                        set_mtx();
                        spec_carry_spec();
                        if (m678 != 0) {
                            mStts.SetRoomId(current.roomNo);
                            mCyl.SetC(current.pos);   // donor MoveCAtTg == move center+At+Tg; receiver SetC is that op
                            dComIfG_Ccsp()->Set(&mCyl);
                        }

                        if (m678 == 5 || m678 == 6 || m678 == 3 || m683 != 0) {
                            dComIfG_Ccsp()->SetMass(&mCyl, 3);
                        }

                        attention_info.position.x = current.pos.x;
                        attention_info.position.y = current.pos.y + data().mAttnY;
                        attention_info.position.z = current.pos.z;
                        eyePos = attention_info.position;
                    }
                }
            }

            spec_clr_actor();

            fopAcM_SetModel(this, m678 == 4 && m686 ? mpModel : NULL);

            if (bVar6) {
                fopAcM_delete(this);
            }
        }
    }

    cull_set_draw();
    return TRUE;
}

// ============================================================
// donor _draw (d_a_tsubo.cpp:3551) — WW-host lighting through the §406
// dKyWw feeders (the item/shutter idiom); BRK spine glow native.
// ============================================================
BOOL daWwTsubo_c::_draw() {
    bool bVar3 = true;

    switch (m678) {
    default:
        if (!prm_get_moveBg()) {
            break;
        }
        // fallthrough
    case 0:
        bVar3 = false;
        break;
    }

    if (bVar3) {
        dKyWw_settingTevStruct(TEV_TYPE_ACTOR, &current.pos, &tevStr);
        dKyWw_setLightTevColorType(mpModel, &tevStr);

        if (mpBrk != NULL) {
            s16 maxFrame = mpBrk->getBrkAnm()->getFrameMax();
            f32 fVar1 = (1.0f - (f32)m800 / attrSpine().m30) * maxFrame;
            if (m800 == 0 || fVar1 < 0.0f) {
                fVar1 = 0.0f;
            } else if (fVar1 >= maxFrame) {
                fVar1 = maxFrame - 0.001f;
            }
            mpBrk->entry(mpModel->getModelData(), fVar1);
        }

        mDoExt_modelUpdateDL(mpModel);

        if (mpBrk != NULL) {
            mpBrk->remove(mpModel->getModelData());
        }

        if (fopAcM_GetModel(this) == NULL) {
            // §823 (the §817-1 black square): the donor's setSimpleShadow2
            // trailing NULL selects WW's DEFAULT soft shadow texture
            // INTERNALLY; the receiver's setSimpleShadow renders NULL as an
            // UNTEXTURED (opaque) quad. The receiver expresses the same
            // default explicitly — getSimpleTex(), its own actors' idiom
            // (obj_carry:1967). Boundary translation, not a stand-in.
            f32 fVar1 = scale.x * data().m6E;
            if (cLib_checkBit<u32>(data().mFlag, 0x8)) {
                dComIfGd_setSimpleShadow(&current.pos, mAcch.GetGroundH(), fVar1,
                                         mAcch.m_gnd, shape_angle.y, 1.0f,
                                         dDlst_shadowControl_c::getSimpleTex());
            } else {
                dComIfGd_setSimpleShadow(&current.pos, mAcch.GetGroundH(), fVar1,
                                         mAcch.m_gnd, 0, 1.0f,
                                         dDlst_shadowControl_c::getSimpleTex());
            }
        }
    }
    return TRUE;
}

// ============================================================
// fpc glue + profile (§747 registration pattern step: profile HERE, the
// register row + name routes in their own files).
// ============================================================
static cPhs_Step daWwTsubo_Create(void* v_this) {
    return ((daWwTsubo_c*)v_this)->_create();
}
static BOOL daWwTsubo_Delete(void* v_this) {
    return ((daWwTsubo_c*)v_this)->_delete();
}
static BOOL daWwTsubo_Execute(void* v_this) {
    return ((daWwTsubo_c*)v_this)->_execute();
}
static BOOL daWwTsubo_Draw(void* v_this) {
    return ((daWwTsubo_c*)v_this)->_draw();
}
static BOOL daWwTsubo_IsDelete(void* v_this) {
    return ((daWwTsubo_c*)v_this)->_is_delete();
}

static actor_method_class l_daWwTsubo_Method = {
    (process_method_func)daWwTsubo_Create,
    (process_method_func)daWwTsubo_Delete,
    (process_method_func)daWwTsubo_Execute,
    (process_method_func)daWwTsubo_IsDelete,
    (process_method_func)daWwTsubo_Draw,
};

extern actor_process_profile_definition g_profile_WW_TSUBO;

actor_process_profile_definition g_profile_WW_TSUBO = {
    // donor g_profile_TSUBO (d_a_tsubo.cpp:3632); receiver fpcLy/fpcPi idiom.
    fpcLy_CURRENT_e,             // Layer ID
    8,                           // List ID (donor 0x0008)
    fpcPi_CURRENT_e,             // List Prio
    fpcNm_WW_TSUBO_e,            // Proc Name
    &g_fpcLf_Method.base,        // Proc SubMtd
    sizeof(daWwTsubo_c),         // Size
    0,                           // Size Other
    0,                           // Parameters
    &g_fopAc_Method.base,        // Leaf SubMtd
    fpcDwPi_E_RD_e,              // Draw Prio (donor fpcDwPi_TSUBO_e slot absent — ground-object slot, the pig/shutter choice)
    &l_daWwTsubo_Method,         // Actor SubMtd
    fopAcStts_CULL_e | fopAcStts_FREEZE_e | fopAcStts_UNK4000_e | fopAcStts_UNK40000_e,   // Status (donor verbatim)
    fopAc_ACTOR_e,               // Group
    fopAc_CULLSPHERE_CUSTOM_e,   // Cull Type (donor: custom sphere from the data rows)
};
