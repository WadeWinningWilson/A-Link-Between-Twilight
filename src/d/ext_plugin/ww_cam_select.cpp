// ============================================================================
// ww_cam_select.cpp — the donor camera SELECTOR, ported by field.
//
// KIT-LINEAGE: donor-port
// KIT-DONOR: d/d_camera.cpp NonMatching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching
//
// Donor originals: dCamera_c::nextType (d/d_camera.cpp:1311-1416),
// dCamera_c::nextMode (:1053-1305), the selection latch (:785-810), and the
// donor-only halves of onTypeChange/onModeChange/onStyleChange.
//
// ---------------------------------------------------------------------------
// THE TRANSLATION TABLE (each row MEASURED, method noted; extends the
// ww_cam_crawl table)
//
//   donor                        receiver                        how known
//   --------------------------   -----------------------------   -----------
//   check_owner_action(0/1)      dComIfGp_checkPlayerStatus0/1   both are thin
//                                                                wrappers on it
//   daPyStts0/1 bit positions    SAME BITS                       differential:
//     receiver nextMode's own masks decompose into donor names — lockon-truth
//     exclusion 0xC000000 = CRAWL|UNK4000000 identical, boomerang-wait
//     0x400000 identical, jump check1 0x10 identical, aim trio 0x25000+0x40.
//   m1AE                         field_0x223                     both nextModes
//     reset it on the identical mBG height test; both lineages only ever
//     write 0 in the camera (the =1 setter is outside in BOTH — still owed).
//   mStickCPosYLast              mPadInfo.mCStick.mLastPosY      offset+use
//   mStickCValueLast             mPadInfo.mCStick.mLastValue     offset+use
//   mStickMainValueLast          mPadInfo.mMainStick.mLastValue  offset+use
//   m350 (BG camera id)          mBG.field_0xc0.field_0x34       both feed from
//                                                                GetCamMoveBG
//   mBG.m00.m58                  mBG.field_0x0.field_0x58        identical use
//   mDMCSystem / setDMCAngle     mFakeAngleSys / setUSOAngle     the receiver's
//     onStyleChange IS the donor's (same FIXED_POS/FRAME cases, same post-JPN
//     double-set guard) under the "USO" rename.
//   m110                         field_0x168                     same-style
//                                                                compare, both
//   mCamSetup.m098 / m09C        donor ctor defaults 60.0 / 0.3  (receiver's
//   mCamSetup.mCstick.m00/.m04   0.2 / 0.95                      dCamSetup_c
//     diverged; the donor ctor values are embedded with citation d_cam_param
//     .cpp:108-109,331-332 rather than guessed against renamed fields)
//
// DONOR-ONLY WORK FIELDS (no receiver counterpart; the layer owns them, the
// crawl-TU DMC-latch precedent): m144/m184 (C-stick free-look latch), m19B,
// m254, m118/m114 (reset points preserved for the engines that will read
// them when their algorithms port).
//
// SOCKETS (nothing is dropped; each logs on first visit):
//   seagull possession (daNpc_kam_c::m_hyoi_kamome) — kamome hyoi unported
//   Medli lockon special-case (fpcNm_NPC_MD_e)      — Medli unported
//   Medli mirror-guard half of isPlayerGuarding      — same
//   donor m350==0x11 map-tool side-resolve           — needs RCAM side effects
//   SetTypeForce (m524/m528)                         — donor actors call it;
//                                                      none ported yet
//
// MODE MIRROR. The donor mode (0..19) is authoritative, layer-held. Receiver
// mCurMode is mirrored through the measured correspondence (0-6 identity,
// 10->7, 11->8, 15->9, else->0) ONLY so receiver-side readers outside the
// replaced block see coherent values; no selection decision reads the mirror.
// ============================================================================

#include "d/ext_plugin/ww_cam_select.h"

#if TARGET_PC

#include <cstring>
#include <vector>

#include "d/ext_plugin/ww_cam_data.h"

#include "d/d_camera.h"
#include "d/d_cam_param.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_save_guard.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_boomerang.h"  // daBoomerang_c IS-A fopAc_ac_c
#include "dusk/logging.h"

namespace {

// ============================================================================
// Donor status-bit names used below, verbatim values (d_com_inf_game.h, donor).
// ============================================================================
enum {
    kStts0_Hang = 0x00000100,          // daPyStts0_HANG_e
    kStts0_BowAim = 0x00001000,        // daPyStts0_BOW_AIM_e
    kStts0_Subject = 0x00002000,       // daPyStts0_SUBJECT_e
    kStts0_HookshotAim = 0x00004000,   // daPyStts0_HOOKSHOT_AIM_e
    kStts0_ShipRide = 0x00010000,      // daPyStts0_SHIP_RIDE_e
    kStts0_RopeAim = 0x00020000,       // daPyStts0_ROPE_AIM_e
    kStts0_BoomerangAim = 0x00080000,  // daPyStts0_BOOMERANG_AIM_e
    kStts0_Swim = 0x00100000,          // daPyStts0_SWIM_e
    kStts0_Telescope = 0x00200000,     // daPyStts0_TELESCOPE_LOOK_e
    kStts0_BoomerangWait = 0x00400000, // daPyStts0_BOOMERANG_WAIT_e
    kStts0_Unk80 = 0x00000080,
    kStts0_Unk800000 = 0x00800000,
    kStts0_Unk1000000 = 0x01000000,
    kStts0_Unk4000000 = 0x04000000,
    kStts0_Crawl = 0x08000000,         // daPyStts0_CRAWL_e
    kStts0_Unk80000000 = 0x80000000,
    // dAttention "judgement" combination the boomerang-wait branch excludes.
    kStts0_Judgement = 0x37A02371,
    kStts1_WindWaker = 0x00000001,
    kStts1_Unk2 = 0x00000002,
    kStts1_Unk4 = 0x00000004,
    kStts1_PictoBox = 0x00000008,
    kStts1_Unk10 = 0x00000010,
    kStts1_DekuLeafFly = 0x00000020,
    kStts1_Unk80 = 0x00000080,
    kStts1_Guard = 0x00080000,         // daPyStts1_UNK80000_e (isPlayerGuarding)
    kStts0_Unk400 = 0x00000400,
    kStts0_Unk4 = 0x00000004,
    kStts0_Unk2 = 0x00000002,
    kStts0_Unk1 = 0x00000001,
    kStts0_Unk10 = 0x00000010,
    kStts0_Unk20 = 0x00000020,
    kStts0_Unk40 = 0x00000040,
};

// Donor dCamSetup defaults, embedded with citation (d_cam_param.cpp:331-332,
// :108-109 in the donor) — see the translation table header.
constexpr f32 kStickFreeR = 60.0f;       // m098 (USA)
constexpr f32 kStickCArm = 0.3f;         // m09C
constexpr f32 kCstickRelease = 0.2f;     // mCstick.m00
constexpr f32 kCstickEngage = 0.95f;     // mCstick.m04

// ============================================================================
// DONOR FREE-LOOK (mode 12) — DISABLED, a measured translation not a drop.
//
// Playtest receipt (2026-08-10): touching the right stick armed donor mode 12
// (m144=0 -> MM01 -> receiver manualCamera alg 9), which latched permanently —
// the donor exit test (stick released && mDirection.R() < 60) reads a camera
// DISTANCE that never goes below 60, and the receiver's manual engine expects
// ITS OWN arming state (mManualMode, armed inside chaseCamera on PC) which the
// donor machine never sets. Meanwhile the receiver's chase engine already
// provides right-stick orbit NATIVELY on PC — the donor's separate mode-12
// machinery duplicates that feature in a dialect the receiver engine cannot
// receive. So free-look stays the receiver's in-engine implementation; the
// donor machinery below is preserved behind this switch for the day the donor
// MANUAL engine ports (then mode 12 becomes reachable again with its own
// engine on a socket).
// ============================================================================
constexpr bool kEnableDonorFreeLook = false;

// ============================================================================
// Layer-owned donor state (see header comment).
// ============================================================================
struct WwCamSelState {
    bool attached = false;
    // Saved receiver data-source pointers, restored on detach.
    dCamStyleData::StyleData* savedStyleData = NULL;
    int savedStyleNum = 0;
    dCamera_type_data* savedTypeData = NULL;
    int savedTypeNum = 0;
    int savedType = 0;
    int savedMode = 0;
    int savedStyle = 0;
    // Donor selection state.
    int dMode = 0;
    u8 m144 = 1;   // C-stick free-look latch: 1 = normal, 0 = free-look armed
    u8 m184 = 0;
    u8 m19B = 0;
    u8 m254 = 0;
    u8 m118 = 0, m114 = 0;  // reset points kept for future engine ports
    int forcedType = 0xFF;      // donor m524 (SetTypeForce socket)
    fopAc_ac_c* forcedTarget = NULL;  // donor m528
    // WW-table indices resolved at attach.
    int typeEvent = -1, typeBoat = -1, typeWater = -1, typeKeep = -1;
    int mapToolType = -1;
    int logBudget = 40;  // transition log lines per attach (playtest probe)
};
WwCamSelState s_st;

int wwTypeFromName(const char* i_name) {
    int count = 0;
    const dExtWwCamType* types = dExtWwCam_types(&count);
    for (int i = 0; i < count; i++) {
        if (std::strcmp(types[i].name, i_name) == 0) {
            return i;
        }
    }
    return 0xFF;
}

const dExtWwCamType& wwType(int i_idx) {
    int count = 0;
    const dExtWwCamType* types = dExtWwCam_types(&count);
    return types[(i_idx >= 0 && i_idx < count) ? i_idx : 0];
}

void logTransition(dCamera_c* i_cam, const char* i_what, int i_type, int i_dMode, int i_style) {
    if (s_st.logBudget <= 0) {
        return;
    }
    s_st.logBudget--;
    const u32 id = (u32)i_cam->mCamParam.Id(i_style);
    DuskLog.info("[WwCamSel] {}: type {} '{}' dMode {} style {} ({}{}{}{}) alg {}", i_what,
                 i_type, wwType(i_type).name, i_dMode, i_style, (char)((id >> 24) & 0xFF),
                 (char)((id >> 16) & 0xFF), (char)((id >> 8) & 0xFF), (char)(id & 0xFF),
                 i_cam->mCamParam.Algorythmn(i_style));
}

// Donor mode (0..19) -> receiver mode mirror (see header).
int mirrorMode(int i_dMode) {
    if (i_dMode >= 0 && i_dMode <= 6) {
        return i_dMode;
    }
    switch (i_dMode) {
    case 10: return 7;
    case 11: return 8;
    case 15: return 9;
    default: return 0;
    }
}

// ============================================================================
// Attach/detach — swap the camera's data sources for the WW tables. The blob
// layouts are the receiver's own file shapes, so only pointers move; on
// detach the receiver's camtype/camstyle data is restored and a style
// re-latch is forced.
// ============================================================================
void attach(dCamera_c* i_cam) {
    int blobSize = 0;
    const unsigned char* styleBlob = (const unsigned char*)dExtWwCam_styleDatBlob(&blobSize);
    int shadowSize = 0;
    const unsigned char* shadowBlob = (const unsigned char*)dExtWwCam_typeShadowBlob(&shadowSize);
    int typeCount = 0;
    dExtWwCam_types(&typeCount);

    // ------------------------------------------------------------------------
    // WRITABLE COPIES, once per session. The receiver engines MUTATE their
    // style records at runtime (dCamParam_c::SetFlag — subjectCamera writes
    // mFlags at d_camera.cpp:7153; SetVal exists too). TP's camstyle.dat is
    // heap data, so those writes are normal there — but the generated blobs
    // are const .rdata, and the first first-person aim crashed on the write
    // (§677 receipt). Runtime mutations persisting across attach/detach
    // within a session matches the receiver's own loaded-data lifetime.
    // ------------------------------------------------------------------------
    static std::vector<unsigned char> s_styleCopy;
    static std::vector<unsigned char> s_shadowCopy;
    if (s_styleCopy.empty()) {
        s_styleCopy.assign(styleBlob, styleBlob + blobSize);
        s_shadowCopy.assign(shadowBlob, shadowBlob + shadowSize);
    }
    styleBlob = s_styleCopy.data();
    shadowBlob = s_shadowCopy.data();
    s_st.savedStyleData = i_cam->mCamParam.mCamStyleData;
    s_st.savedStyleNum = i_cam->mCamParam.mStyleNum;
    s_st.savedTypeData = i_cam->mCamTypeData;
    s_st.savedTypeNum = i_cam->mCamTypeNum;
    s_st.savedType = i_cam->mCurType;
    s_st.savedMode = i_cam->mCurMode;
    s_st.savedStyle = i_cam->mCamStyle;

    i_cam->mCamParam.mCamStyleData = (dCamStyleData::StyleData*)(styleBlob + 8);
    i_cam->mCamParam.mStyleNum = dExtWwCam_styleCount();
    i_cam->mCamTypeData = (dCamera_type_data*)(shadowBlob + 8);
    i_cam->mCamTypeNum = typeCount;

    s_st.typeEvent = wwTypeFromName("Event");
    s_st.typeBoat = wwTypeFromName("Boat");
    s_st.typeWater = wwTypeFromName("Water");
    s_st.typeKeep = wwTypeFromName("Keep");
    // Donor default type: the stage map tool's camera name resolved against
    // the DONOR table (no memo field write — receiver memoization would leak
    // receiver indices across the swap); "Field" when absent, per the donor's
    // own initialize default.
    s_st.mapToolType = 0xFF;
    if (i_cam->mStageCamTool.mCameraIndex != 0xFF) {
        s_st.mapToolType = wwTypeFromName(i_cam->mStageCamTool.mCamData.m_cam_type);
    }
    if (s_st.mapToolType == 0xFF) {
        s_st.mapToolType = wwTypeFromName("Field");
    }

    s_st.dMode = 0;
    s_st.m144 = 1;
    s_st.m184 = 0;
    s_st.m254 = 0;
    s_st.forcedType = 0xFF;
    s_st.forcedTarget = NULL;
    s_st.logBudget = 40;
    i_cam->mCurType = s_st.mapToolType;
    i_cam->mCurMode = 0;
    i_cam->mCamStyle = -1;  // force the style latch this same frame
    s_st.attached = true;
    DuskLog.info("[WwCamSel] attached: {} styles / {} donor-axis types live; default type "
                 "'{}'", dExtWwCam_styleCount(), typeCount, wwType(s_st.mapToolType).name);
}

void detach(dCamera_c* i_cam) {
    if (!s_st.attached) {
        return;
    }
    i_cam->mCamParam.mCamStyleData = s_st.savedStyleData;
    i_cam->mCamParam.mStyleNum = s_st.savedStyleNum;
    i_cam->mCamTypeData = s_st.savedTypeData;
    i_cam->mCamTypeNum = s_st.savedTypeNum;
    i_cam->mCurType = s_st.savedType;
    i_cam->mCurMode = s_st.savedMode;
    i_cam->mCamStyle = s_st.savedStyle;
    i_cam->mCamParam.Change(i_cam->mCamStyle);
    s_st.attached = false;
    DuskLog.info("[WwCamSel] detached: receiver camera data restored");
}

// ============================================================================
// Donor helpers, receiver feeds.
// ============================================================================
u32 ownerAction(dCamera_c* i_cam, u32 i_mask) {
    return dComIfGp_checkPlayerStatus0(i_cam->mPadID, i_mask);
}

u32 ownerAction1(dCamera_c* i_cam, u32 i_mask) {
    return dComIfGp_checkPlayerStatus1(i_cam->mPadID, i_mask);
}

// Donor isPlayerGuarding minus the Medli-mirror half (socket).
bool playerGuarding(dCamera_c* i_cam) {
    return ownerAction1(i_cam, kStts1_Guard) != 0;
}

// Donor get_boomerang_actor: thin wrapper both lineages; receiver dialect.
fopAc_ac_c* boomerangActor(dCamera_c* i_cam) {
    daAlink_c* link = daAlink_getAlinkActorClass();
    if ((fopAc_ac_c*)link == i_cam->mpPlayerActor && link != NULL) {
        return static_cast<fopAc_ac_c*>(link->getThrowBoomerangActor());
    }
    return NULL;
}

// ============================================================================
// Donor nextType (d_camera.cpp:1311-1416), ported. Feeds per the table.
// ============================================================================
int nextTypeWw(dCamera_c* i_cam) {
    int next = i_cam->mCurType;

    if (dComIfGp_evmng_cameraPlay() || i_cam->chkFlag(0x20000000)) {
        next = s_st.typeEvent;
        if (i_cam->mCurType != next) {
            i_cam->clrFlag(0x200000);
            if (i_cam->mCurType != s_st.typeEvent) {
                // Donor stashes the pre-event type for the restore in
                // styleEvCamera — the receiver reads the SAME field.
                i_cam->mEventData.field_0xc = i_cam->mCurType;
            }
        }
        return next;
    }

    if (i_cam->mpPlayerActor == NULL) {
        return next;
    }

    if (i_cam->mCurType == s_st.typeEvent) {
        next = i_cam->mEventData.field_0xc;
        i_cam->mEventData.field_0xc = -1;
        if (next < 0) {
            next = s_st.mapToolType;
        }
    }

    // SOCKET: seagull possession (daNpc_kam_c::m_hyoi_kamome) — hyoi unported.
    if ((ownerAction(i_cam, kStts0_Unk1000000 | kStts0_ShipRide) ||
         ownerAction1(i_cam, kStts1_Unk80)) &&
        s_st.forcedType == 0xFF)
    {
        next = s_st.typeBoat;
        return next;
    }

    // RCAM: donor stage idx overrides room idx; resolve entry NAMES against
    // the donor table, never through the receiver's memo field.
    u32 idx;
    if (i_cam->mStageCamTool.mCameraIndex != 0xFF) {
        idx = i_cam->mStageCamTool.mCameraIndex;
    } else {
        idx = i_cam->mRoomMapTool.mCameraIndex;
    }

    if (s_st.forcedType != 0xFF) {
        next = s_st.forcedType;
        if (s_st.forcedTarget != NULL) {
            i_cam->mpLockonTarget = s_st.forcedTarget;
        }
    } else if (idx == 0xFF) {
        // BG-attribute path: donor m350 = receiver mBG.field_0xc0.field_0x34,
        // both fed by GetCamMoveBG on the ground polygon.
        int bgCount = 0;
        const char* const* bgNames = dExtWwCam_bgTypeNames(&bgCount);
        const int bgIdx = i_cam->mBG.field_0xc0.field_0x34;
        if (bgIdx > 0) {
            if (bgIdx < bgCount) {
                const int fromBg = wwTypeFromName(bgNames[bgIdx]);
                if (fromBg != 0xFF && fromBg != s_st.typeKeep) {
                    next = fromBg;
                }
                // SOCKET: donor bgIdx==0x11 re-resolves map-tool id 0 for the
                // player's room (side effects on the room cam entry) — needs
                // the RCAM side-effect port.
            }
        } else if (ownerAction(i_cam, kStts0_Swim)) {
            next = s_st.typeWater;
        } else {
            next = s_st.mapToolType;
        }
    } else if (idx == 0x1FF) {
        if (ownerAction1(i_cam, kStts1_DekuLeafFly)) {
            next = s_st.mapToolType;
        } else if (ownerAction(i_cam, kStts0_Swim)) {
            next = s_st.typeWater;
        }
        if (i_cam->mCurType == s_st.typeBoat) {
            next = s_st.typeWater;
        }
    } else {
        // Map-tool entry at the SELECTED index — donor GetCameraTypeFromMapToolID
        // resolves the entry of the table the index CAME FROM: the STAGE table
        // when the stage index is live, the room table otherwise. The first
        // build resolved the receiver's room-cached entry unconditionally,
        // which on Outset holds a Subject (crawl-zone) record — the whole
        // island then ran LN17/alg-20 as its standing camera (the fixed,
        // non-following view at the pier / by the houses). Donor sea behavior
        // is FieldCushion here.
        const char* toolName = (i_cam->mStageCamTool.mCameraIndex != 0xFF)
                                   ? i_cam->mStageCamTool.mCamData.m_cam_type
                                   : i_cam->mRoomMapTool.mCamData.m_cam_type;
        const int fromTool = wwTypeFromName(toolName);
        if (fromTool == s_st.typeKeep) {
            next = i_cam->mCurType;
        } else if (fromTool != 0xFF) {
            next = fromTool;
        } else {
            next = s_st.mapToolType;
        }
    }

    s_st.forcedType = 0xFF;
    s_st.forcedTarget = NULL;
    return next;
}

// ============================================================================
// Donor nextMode (d_camera.cpp:1053-1305), ported. Structure preserved; each
// receiver feed is from the translation table. Donor masks verbatim (the bit
// positions are shared-ancestor-stable — measured, see header).
// ============================================================================
int nextModeWw(dCamera_c* i_cam) {
    dAttention_c* attn = dComIfGp_getAttention();
    int next = s_st.dMode;
    const cXyz playerPos = i_cam->mpPlayerActor->current.pos;

    if (dComIfGp_evmng_cameraPlay()) {
        return next;
    }

    if (i_cam->mBG.field_0x0.field_0x58 > playerPos.y) {
        i_cam->field_0x223 = 0;  // donor m1AE, same reset both lineages
    }

    switch (s_st.dMode) {
    case 4: case 10: case 11: case 13: case 14:
        s_st.m144 = 1;
        s_st.m184 = 0;
        break;
    case 5: case 6:
        s_st.m144 = 1;
        s_st.m184 = 0;
        // fallthrough (donor)
    case 1:
        i_cam->mpLockonTarget = NULL;
        // fallthrough (donor)
    default:
        if (s_st.m19B) {
            s_st.m144 = 1;
            s_st.m184 = 0;
        } else if (kEnableDonorFreeLook && i_cam->mPadInfo.mCStick.mLastPosY <= 0.0f &&
                   i_cam->mPadInfo.mCStick.mLastValue > kStickCArm) {
            s_st.m144 = 0;
        } else if (s_st.dMode == 0 || s_st.dMode == 0x13) {
            if (!(i_cam->mPadInfo.mMainStick.mLastValue >= 0.5f || attn->LockonTruth() ||
                  ownerAction(i_cam, kStts0_Swim)))
            {
                if (s_st.m184 == 1) {
                    if (i_cam->mPadInfo.mCStick.mLastPosY < kCstickRelease) {
                        s_st.m184 = 0;
                    }
                } else if (i_cam->mPadInfo.mCStick.mLastPosY > kCstickEngage) {
                    s_st.m184 = 1;
                }
            }
        }
        break;
    case 12:
        if ((i_cam->mPadInfo.mCStick.mLastValue < 0.01f &&
             i_cam->mDirection.R() < kStickFreeR) ||
            i_cam->chkFlag(0x80000000))
        {
            s_st.m144 = 1;
            s_st.m184 = 0;
        } else if (s_st.m19B != 0) {
            s_st.m144 = 1;
            s_st.m184 = 0;
        }
        break;
    }

    if (i_cam->chkFlag(0x4000000)) {
        if (s_st.m144 == 0) {
            s_st.m254 |= 1;
        }
        if (ownerAction(i_cam, kStts0_Unk80000000)) {
            i_cam->setFlag(0x8000);
        }
        s_st.m144 = 1;
        i_cam->clrFlag(0x4000000);
    }

    // SOCKET: donor Medli lockon special-case (fpcNm_NPC_MD_e -> m144=1,
    // curMode=0) — Medli unported.

    int curMode = s_st.dMode;
    // ------------------------------------------------------------------------
    // RECEIVER-DIALECT INPUT, translated: TP Link forces camera modes through
    // ModeFix (mForcedMode, a RECEIVER mode 0..10; 11 = none). The donor's own
    // players do the same through their force path, so honoring it is donor
    // structure with a receiver feed — mapped receiver->donor through the
    // measured correspondence (identity 0-6, 7->10, 8->11, 9->15).
    // ------------------------------------------------------------------------
    int forcedDonor = -1;
    switch (i_cam->mForcedMode) {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6:
        forcedDonor = i_cam->mForcedMode;
        break;
    case 7: forcedDonor = 10; break;
    case 8: forcedDonor = 11; break;
    case 9: forcedDonor = 15; break;
    default: break;  // 11 = none
    }
    if (forcedDonor >= 0 && wwType(i_cam->mCurType).styles[forcedDonor] >= 0) {
        next = forcedDonor;
    } else if (curMode == 12 && s_st.m144 != 0) {
        next = 0;
    } else if (ownerAction(i_cam, kStts0_Telescope) || ownerAction1(i_cam, kStts1_PictoBox)) {
        next = 0xE;
    } else if (ownerAction(i_cam, kStts0_Unk80000000 | kStts0_Unk80)) {
        next = 0x11;
    } else if (ownerAction(i_cam, kStts0_Unk800000)) {
        next = s_st.m144 == 0 ? 0xC : 0x12;
    } else if (ownerAction1(i_cam, kStts1_Unk10)) {
        next = 0xF;
    } else if (ownerAction(i_cam, kStts0_Subject)) {
        next = 4;
    } else if (ownerAction(i_cam, kStts0_RopeAim | kStts0_HookshotAim | kStts0_BowAim) &&
               !attn->Lockon()) {
        next = 10;
    } else if (ownerAction(i_cam, kStts0_BoomerangAim) && !attn->Lockon()) {
        next = 11;
    } else if (s_st.m144 == 0) {
        next = 12;
    } else if (ownerAction1(i_cam, kStts1_Unk2)) {
        next = 5;
    } else if (ownerAction1(i_cam, kStts1_Unk4)) {
        next = 6;
    } else if (ownerAction(i_cam, kStts0_Unk40 | kStts0_Unk20)) {
        next = 6;
    } else if (ownerAction(i_cam, kStts0_Unk40 | kStts0_Unk20 | kStts0_Unk1)) {
        next = 5;
    } else if (ownerAction(i_cam, kStts0_Unk400 | kStts0_Unk4 | kStts0_Unk2) && curMode != 12) {
        if (i_cam->mpLockonTarget != NULL) {
            next = 8;
        }
    } else if (attn->LockonTruth() &&
               !ownerAction(i_cam, kStts0_Crawl | kStts0_Unk4000000)) {
        next = 2;
    } else if (attn->Lockon()) {
        next = 1;
    } else if (ownerAction(i_cam, kStts0_BoomerangWait) &&
               !ownerAction(i_cam, kStts0_Judgement & ~kStts0_Unk1000000) &&
               !ownerAction1(i_cam, kStts1_Unk10 | kStts1_WindWaker)) {
        i_cam->mpLockonTarget = boomerangActor(i_cam);
        next = 2;
        i_cam->mLockOnActorID = -1;
    } else if (playerGuarding(i_cam)) {
        next = 19;
    } else if (i_cam->mLockOnActorID != -1) {
        if (i_cam->mpLockOnActor != NULL) {
            next = 2;
            i_cam->mpLockonTarget = i_cam->mpLockOnActor;
        } else {
            next = 0;
            i_cam->mLockOnActorID = -1;
        }
    } else {
        if (curMode == 12) {
            if (s_st.m144 != 0) {
                next = 0;
            }
        } else {
            next = 0;
        }
    }

    if (next != 2) {
        i_cam->mLockOnActorID = -1;
    }

    const dExtWwCamType& type = wwType(i_cam->mCurType);
    if (next == 12 && type.styles[next] < 0) {
        next = s_st.dMode;
        if (i_cam->mCurType != s_st.typeEvent && i_cam->mCurType != s_st.typeBoat) {
            s_st.m254 |= 1;
        }
        s_st.m144 = 1;
    }

    if (type.styles[next] >= 0) {
        if (next == 1) {
            i_cam->setFlag(0x100000);
        }
        return next;
    }

    return s_st.dMode;
}

// Donor ChangeModeOK, donor-axis.
bool changeModeOkWw(dCamera_c* i_cam, int i_dMode) {
    if (dComIfGp_evmng_cameraPlay() || i_cam->chkFlag(0x20000000)) {
        return false;
    }
    return wwType(i_cam->mCurType).styles[i_dMode] >= 0;
}

// ============================================================================
// THE HOOK — the donor selection latch (d_camera.cpp:785-810) against the
// donor tables, mirrored into receiver fields, with receiver onTypeChange/
// onModeChange/onStyleChange doing the shared-engine bookkeeping (they are
// the donor helpers in receiver dialect — measured, see header) and the
// donor-only latch bits applied layer-side.
// ============================================================================
bool wwCamSelect(dCamera_c* i_cam) {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || !dExtWwSave_isWwHostStage(stage)) {
        if (s_st.attached) {
            detach(i_cam);
        }
        return false;
    }
    if (!s_st.attached) {
        attach(i_cam);
    }

    // --- type latch --------------------------------------------------------
    const int nextType = nextTypeWw(i_cam);
    if (nextType != i_cam->mCurType && nextType >= 0) {
        // Donor onTypeChange's own half: a FIXED/EVENT/CRAWL/HOOKSHOT slot-0
        // style on the incoming type cancels free-look and re-bases the mode.
        if (s_st.m144 == 0) {
            const s16 style0 = wwType(nextType).styles[0];
            if (style0 >= 0) {
                switch (i_cam->mCamParam.Algorythmn(style0)) {
                case 5:   // fixedPosition
                case 6:   // fixedFrame
                case 10:  // event (receiver index)
                case 20:  // CRAWL socket
                case 11:  // hookshot
                    s_st.dMode = 0;
                    s_st.m144 = 1;
                    break;
                }
            }
        }
        i_cam->onTypeChange(i_cam->mCurType, nextType);  // receiver bookkeeping
        logTransition(i_cam, "type change", nextType, s_st.dMode,
                      wwType(nextType).styles[0] >= 0 ? wwType(nextType).styles[0] : 0);
        i_cam->mCurType = nextType;
        i_cam->mNextType = nextType;
    }

    // --- mode latch (donor axis) ------------------------------------------
    const int nextMode = nextModeWw(i_cam);
    if (nextMode != s_st.dMode && wwType(i_cam->mCurType).styles[nextMode] >= 0) {
        i_cam->onModeChange(mirrorMode(s_st.dMode), mirrorMode(nextMode));
        if (nextMode == 12 && s_st.dMode != nextMode) {
            s_st.m254 |= 2;  // donor onModeChange's own half
        }
        s_st.dMode = nextMode;
        i_cam->mCurCamStyleTimer = 0;
    }
    if (!changeModeOkWw(i_cam, s_st.dMode)) {
        s_st.dMode = 0;
    }
    i_cam->mCurMode = mirrorMode(s_st.dMode);
    i_cam->mNextMode = i_cam->mCurMode;

    // --- style latch -------------------------------------------------------
    const s16 style = wwType(i_cam->mCurType).styles[s_st.dMode];
    if (style >= 0 && i_cam->mCamStyle != style) {
        if (i_cam->mCamStyle >= 0) {
            i_cam->onStyleChange(i_cam->mCamStyle, style);  // = donor's, USO=DMC
            // Donor onStyleChange's own half: entering SUBJECT/CRAWL/HOOKSHOT
            // (or the DMC-latching fixed algorithms) cancels free-look.
            switch (i_cam->mCamParam.Algorythmn(style)) {
            case 4: case 5: case 6: case 11: case 20:
                if (s_st.m144 == 0) {
                    s_st.m144 = 1;
                }
                break;
            }
        }
        i_cam->mCamStyle = style;
        i_cam->mCamParam.Change(i_cam->mCamStyle);
        i_cam->setFlag(0x200);
        logTransition(i_cam, "style change", i_cam->mCurType, s_st.dMode, style);
    }

    return true;
}

}  // namespace

void dExtWwCam_installSelect(void) {
    dCamera_setSelectHook(&wwCamSelect);
    DuskLog.info("[WwCamSel] selection hook installed (WW host stages only; donor axis)");
}

#endif  // TARGET_PC
