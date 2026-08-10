// ============================================================================
// ww_cam_crawl.cpp — the donor CRAWL camera algorithm, ported by field.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// Donor original: dCamera_c::crawlCamera, d/d_camera.cpp:4255-4409.
//
// ---------------------------------------------------------------------------
// THE TRANSLATION TABLE
// The two lineages share this engine, so almost every symbol the donor uses
// exists in the receiver under a different name. Each row below was MEASURED,
// not assumed; the method is named where it was not a plain grep.
//
//   donor                       receiver                        how known
//   -------------------------   -----------------------------   -----------
//   m108                        mCurCamStyleTimer               differential
//   mPadId                      mPadID                          case only
//   mCameraInfoIdx              mCameraID                        offset+use
//   mCurRoomCamEntry            mRoomMapTool.mCamData            type match
//   mCurRoomArrowEntry          mRoomMapTool.mArrowData          type match
//   mCurArrowIdx                mRoomMapTool.mArrowIndex         type match
//   m100 / m101 / m102          mStyleSettle.mFinished           differential
//   mCamParam.Val/Flag          identical API                    verbatim
//   directionOf/positionOf      identical members                verbatim
//   relationalPos/lineBGCheck   identical overloads              verbatim
//
// `m108 -> mCurCamStyleTimer` came from a differential: both lineages open
// subjectCamera AND lockonCamera with the same first-frame guard `if (X == 0)`
// on that field. Two independent functions agreeing is why it is not a guess.
//
// `m100/m101/m102 -> mStyleSettle.mFinished` came the same way: the donor sets
// the triple at the settle point where the receiver sets the single flag. The
// receiver collapsed three flags into one; all 87 receiver uses are that flag.
//
// ---------------------------------------------------------------------------
// THE ONE THING THAT IS NOT A RENAME: dCamera_DMC_system
//
// The donor's DMC record is the camera-cut control latch. When the view
// jump-cuts it stores the camera azimuth AND the stick angle together, so that
// Link does not reverse direction under the player's thumb across the cut:
//
//     setDMCAngle() { f_0x0 = 1; f_0x2 = mDirection.U().Inv();
//                     f_0x4 = CPad_GET_STICK_ANGLE(mPadId); }
//
// The receiver has a struct at 0x158 (`mStyleSettle`) whose LAYOUT is exactly
// that record -- {u8 @0, cSAngle @2, cSAngle @4}. It is tempting to call them
// the same field, and doing so would have been wrong: the receiver's 87 uses
// are all of offset 0x0 as a settle flag, its two angle fields are written
// NOWHERE in the repo, and the differential above independently binds that
// same flag to the donor's m100/m101/m102. Layout agreement is a coincidence
// of a shared ancestor; usage disagrees.
//
// Binding the latch to `mStyleSettle` would therefore have written donor
// meaning into a live receiver field -- §607's "silent data, not a crash".
// So the latch is state this layer OWNS. That is not a bridge: the system is
// the donor's own, ported whole, and only its storage is parallel. It is also
// why no receiver struct changes: the crawl work area overlays the receiver's
// untyped mWork[0x100] scratch, and the latch lives here.
// ============================================================================
#include "d/ext_plugin/ww_cam_crawl.h"

#include "d/d_camera.h"
#include "d/d_cam_param.h"
#include "d/d_com_inf_game.h"
#include "d/actor/d_a_alink.h"

// ============================================================================
// DONOR CONSTANTS
// d/d_cam_param.h in the donor; both are self-describing in its enum.
// ============================================================================
enum {
    WW_CAM_PRM_FLG_UNK040 = 0x040,  // "use the room arrow"
    WW_CAM_PRM_FLG_UNK080 = 0x080,  // "tight/low crawl framing"
};

// dCamAttnStts_00000080_e in the donor's attention-status enum.
enum { WW_CAM_ATTN_STTS_80 = 0x00000080 };

// ============================================================================
// THE CRAWL WORK AREA
// Donor `dCamera_c::mWork.crawl`, verbatim, rebased to 0. The donor's union
// sits at 0x378 and this variant spans 0x28 bytes; the receiver's mWork is an
// untyped u8[0x100] at 0x3E8, so the overlay fits with room to spare and costs
// the receiver no declaration at all.
// ============================================================================
struct WwCamCrawlWork {
    /* 0x00 */ int  mTag;         // donor m378, literal 'CRWL'
    /* 0x04 */ int  mBlendLen;    // donor m37C, 10 frames, or 6 when already crawling
    /* 0x08 */ f32  mWeightLeft;  // donor m380, triangular sum of mBlendLen
    /* 0x0C */ f32  mWeight;      // donor m384, this frame's share
    /* 0x10 */ int  mFaceFlip;    // donor m388, view from behind (1) or in front (0)
    /* 0x14 */ cXyz mArrowEye;    // donor m38C, eye position taken from the room arrow
    /* 0x20 */ u8   mTight;       // donor m398, from UNK080
    /* 0x21 */ u8   mArrowSnap;   // donor m399, arrow branch armed
    /* 0x22 */ u8   mPad[2];      // donor m39A
    /* 0x24 */ int  m39C;         // donor m39C, set to 5 and never read (kept: donor state)
};

// ============================================================================
// THE DMC LATCH — donor dCamera_DMC_system, owned here. See the banner.
// Keyed by camera id because the donor's record is per-camera. Four is the
// receiver's camera count; the guard keeps a bad id from writing out of range
// rather than trusting the caller.
// ============================================================================
struct WwCamDMC {
    u8      mLatched;    // donor field_0x0
    u8      mUnused;     // donor field_0x1
    cSAngle mCamAngle;   // donor field_0x2
    cSAngle mStickAngle; // donor field_0x4
};

#define WW_CAM_DMC_MAX 4
static WwCamDMC l_dmc[WW_CAM_DMC_MAX];

static WwCamDMC* wwCam_dmc(dCamera_c* i_cam) {
    const u32 id = i_cam->mCameraID;
    return (id < WW_CAM_DMC_MAX) ? &l_dmc[id] : &l_dmc[0];
}

// Donor dCamera_c::setDMCAngle, ported whole.
static void wwCam_setDMCAngle(dCamera_c* i_cam) {
    WwCamDMC* dmc = wwCam_dmc(i_cam);
    dmc->mLatched = 1;
    dmc->mCamAngle = i_cam->mDirection.U().Inv();
    dmc->mStickAngle = cSAngle(mDoCPd_c::getStickAngle3D(i_cam->mPadID));
}

// ============================================================================
// IS THE PLAYER CRAWLING?
// The donor asks `check_owner_action(mPadId, daPyStts0_CRAWL_e)`. The receiver
// has no such status bit -- but it DOES have crawling, as procs rather than as
// a status flag (PROC_CRAWL_START/MOVE/AUTO_MOVE/END, plus crawlBgCheck). This
// is the receiver's own idiom for the same question, so it is a translation of
// the predicate, not a substitute for it.
// ============================================================================
static bool wwCam_playerIsCrawling(dCamera_c* i_cam) {
    daAlink_c* link = (daAlink_c*)i_cam->mpPlayerActor;
    if (link == NULL) {
        return false;
    }
    const int proc = link->mProcID;
    return proc == daAlink_c::PROC_CRAWL_START || proc == daAlink_c::PROC_CRAWL_MOVE ||
           proc == daAlink_c::PROC_CRAWL_AUTO_MOVE || proc == daAlink_c::PROC_CRAWL_END;
}

// ============================================================================
// THE ALGORITHM
// Donor dCamera_c::crawlCamera, d/d_camera.cpp:4255. Structure and constants
// are the donor's; only the names on the left of each access are translated.
//
// NOT PORTED, and deliberately: the donor gates two of its branches on `m1AE`,
// a u8 the donor's own d_camera.cpp only ever writes as 0 -- its `= 1` setter
// lives outside the camera and outside this port's scope. Running with it 0 is
// therefore the DONOR's behaviour whenever the flag is unset, not an invention:
// the attention-status call is skipped and the blend runs its full 10 frames.
// When the setter is found this becomes a real input. Recorded as owed.
// ============================================================================
static bool wwCam_crawlCamera(dCamera_c* i_cam, s32 i_style, int /* i_alg */) {
    WwCamCrawlWork* w = (WwCamCrawlWork*)i_cam->mWork;
    const bool m1AE = false;  // see banner above

    cXyz sp134;
    cXyz sp128;

    f32 f0 = i_cam->mCamParam.Val(i_style, 1);
    f32 f1 = i_cam->mCamParam.Val(i_style, 5);
    f32 f2 = i_cam->mCamParam.Val(i_style, 0);
    f32 f29 = i_cam->mCamParam.Val(i_style, 3);
    f32 f28 = i_cam->mCamParam.Val(i_style, 4);
    f32 f30 = i_cam->mCamParam.Val(i_style, 10);
    f32 f31 = i_cam->mCamParam.Val(i_style, 25);
    sp134.set(f0, f1, f2);

    cSAngle local_118(i_cam->directionOf(i_cam->mpPlayerActor) - i_cam->mViewCache.mDirection.U());

    if (i_cam->mCurCamStyleTimer == 0) {
        w->mTag = 'CRWL';
        w->mTight = i_cam->mCamParam.Flag(i_style, WW_CAM_PRM_FLG_UNK080) ? 1 : 0;

        w->mArrowSnap = 0;
        w->mBlendLen = 10;

        if (wwCam_playerIsCrawling(i_cam) && m1AE) {
            w->mBlendLen = 6;
        }

        w->m39C = 5;
        w->mWeightLeft = (f32)(w->mBlendLen * (w->mBlendLen + 1) >> 1);

        if (i_cam->mCamParam.Flag(i_style, WW_CAM_PRM_FLG_UNK040) == 0) {
            w->mFaceFlip = 1;
        } else if (local_118 > cSAngle::_270 && local_118 < cSAngle::_90) {
            w->mFaceFlip = 0;
        } else {
            w->mFaceFlip = 1;
        }
    } else if (i_cam->mCamParam.Flag(i_style, WW_CAM_PRM_FLG_UNK040) == 0) {
        w->mFaceFlip = 1;
    } else if (local_118 > cSAngle(-75.0f) && local_118 < cSAngle(75.0f)) {
        w->mFaceFlip = 0;
    } else if (local_118 < cSAngle(-135.0f) || local_118 > cSAngle(135.0f)) {
        w->mFaceFlip = 1;
    }

    if (m1AE) {
        dComIfGp_onCameraAttentionStatus(i_cam->mCameraID, WW_CAM_ATTN_STTS_80);
    }

    if (i_cam->mCamParam.Flag(i_style, WW_CAM_PRM_FLG_UNK040) &&
        i_cam->mRoomMapTool.mArrowIndex != 0xff && i_cam->mRoomMapTool.mCamData.field_0x12 != 0xff) {
        // The donor's arrow record carries a cXyz `position` and a csXyz
        // `angle`; the receiver flattens both into big-endian scalars. Same
        // record, different shape -- translated here rather than reinterpreted.
        local_118 = i_cam->mViewCache.mDirection.U() - (s16)i_cam->mRoomMapTool.mArrowData.angleY;
        if (local_118 > cSAngle::_270 && local_118 < cSAngle::_90) {
            w->mArrowEye.set(i_cam->mRoomMapTool.mArrowData.posX,
                             i_cam->mRoomMapTool.mArrowData.posY,
                             i_cam->mRoomMapTool.mArrowData.posZ);
            w->mArrowSnap = 1;
            i_cam->mStyleSettle.mFinished = true;
        }
    } else {
        w->mArrowSnap = 0;
    }

    if (!i_cam->mStyleSettle.mFinished) {
        w->mWeight = (f32)(s32)(w->mBlendLen - i_cam->mCurCamStyleTimer);
        f28 = w->mWeight / w->mWeightLeft;
        sp128 = i_cam->relationalPos(i_cam->mpPlayerActor, &sp134);
        i_cam->mViewCache.mCenter += (sp128 - i_cam->mViewCache.mCenter) * f28;

        if (i_cam->mCamParam.Flag(i_style, WW_CAM_PRM_FLG_UNK080)) {
            f30 = 2.0f;
        }

        f1 = i_cam->mViewCache.mDirection.R();
        f32 f29b = f1 + f28 * (f30 - f1);
        cSAngle acStack_11c(i_cam->mViewCache.mDirection.V() +
                            (i_cam->mpPlayerActor->shape_angle.x - i_cam->mViewCache.mDirection.V()) *
                                f28);

        cSAngle sp80;
        if (w->mFaceFlip) {
            sp80.Val(i_cam->directionOf(i_cam->mpPlayerActor).Inv());
        } else {
            sp80.Val(i_cam->directionOf(i_cam->mpPlayerActor));
        }

        cSAngle acStack_124(i_cam->mViewCache.mDirection.U() +
                            (sp80 - i_cam->mViewCache.mDirection.U()) * f28);
        i_cam->mViewCache.mDirection.Val(f29b, acStack_11c, acStack_124);

        i_cam->mViewCache.mEye = i_cam->mViewCache.mCenter + i_cam->mViewCache.mDirection.Xyz();
        i_cam->mViewCache.mFovy += f28 * (f31 - i_cam->mViewCache.mFovy);

        if ((s32)i_cam->mCurCamStyleTimer >= w->mBlendLen - 1) {
            i_cam->mStyleSettle.mFinished = true;
        }

        w->mWeightLeft -= w->mWeight;
        return true;
    }

    cXyz local_7c(f29, f28, f29);
    i_cam->mViewCache.mCenter +=
        (i_cam->relationalPos(i_cam->mpPlayerActor, &sp134) - i_cam->mViewCache.mCenter) * local_7c;

    bool handled = false;
    if (w->mArrowSnap == 1) {
        if (i_cam->lineBGCheck(&i_cam->mViewCache.mCenter, &w->mArrowEye, 0x7f) == 0) {
            i_cam->mViewCache.mEye = w->mArrowEye;
            i_cam->mViewCache.mDirection.Val(i_cam->mViewCache.mEye - i_cam->mViewCache.mCenter);

            if (wwCam_dmc(i_cam)->mLatched == 0) {
                wwCam_setDMCAngle(i_cam);
            }
            handled = true;
        }
    }

    if (!handled) {
        cSAngle sp78;
        cSAngle sp74;
        cSAngle sp70;
        if (w->mFaceFlip) {
            sp70.Val(i_cam->directionOf(i_cam->mpPlayerActor).Inv());
        } else {
            sp70.Val(i_cam->directionOf(i_cam->mpPlayerActor));
        }

        if (w->mTight) {
            i_cam->mViewCache.mCenter.y = i_cam->positionOf(i_cam->mpPlayerActor).y + 30.0f;
            f0 = i_cam->mViewCache.mDirection.R();
            f0 = f0 + (2.0f - f0) * 0.8f;
            sp78 = i_cam->mViewCache.mDirection.U() +
                   ((sp70 - i_cam->mViewCache.mDirection.U()) * 0.8f);
            i_cam->setFlag(0x800);
        } else {
            f0 = i_cam->mViewCache.mDirection.R();
            f0 = f0 + (f30 - f0) * 0.25f;
            sp78 = i_cam->mViewCache.mDirection.U() +
                   (sp70 - i_cam->mViewCache.mDirection.U()) * 0.2f;
        }
        sp74 = i_cam->mViewCache.mDirection.V() +
               ((i_cam->mpPlayerActor->shape_angle.x - i_cam->mViewCache.mDirection.V()) * 0.25f);
        i_cam->mViewCache.mDirection.Val(f0, sp74, sp78);
        i_cam->mViewCache.mEye = i_cam->mViewCache.mCenter + i_cam->mViewCache.mDirection.Xyz();
    }

    i_cam->mViewCache.mFovy = i_cam->mViewCache.mFovy + (f31 - i_cam->mViewCache.mFovy) * 0.5f;
    return true;
}

// ============================================================================
// INSTALL
// ============================================================================
static bool wwCam_extraEngine(dCamera_c* i_cam, s32 i_style, int i_alg) {
    if (i_alg == WW_CAM_ALG_CRAWL) {
        return wwCam_crawlCamera(i_cam, i_style, i_alg);
    }
    return false;
}

void dExtWwCam_installCrawl() {
    dCamera_setExtraEngineHook(&wwCam_extraEngine);
}
