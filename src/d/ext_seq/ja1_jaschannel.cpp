// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASChannel.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASDriverTables.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASChannelMgr.cpp NonMatching
// KIT-DONOR: JSystem/JAudio/JASChGlobal.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASChAllocQueue.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §368 JAudio1 logical-channel layer — donor-verbatim port of WW retail
// JSystem/JAudio channel TUs (D:/XXXXXXX/WW DP; dtk address markers
// kept). Phase A3 of the native JA1 campaign (bus §362/§363/§366).
// Donor JASystem::* flattens into JAudio1::* (§363 convention, §328
// same-name lesson). Nothing in the game calls this TU yet; the ExtSeq
// bridge keeps playing until A4 swaps entry points.
// Donor TU map (§368 named adaptation "TU consolidation" — one
// receiver TU, donor TU sections marked):
//   [1] JASChannel.cpp       (full, incl. Driver::calc_sw_table)
//   [2] JASDriverTables.cpp  (C5BASE_PITCHTABLE excerpt) +
//       JASChannelMgr.cpp rodata (Driver::polys_table)
//   [3] JASChannelMgr.cpp    (decomp bodies; getLogicalChannel /
//       cutList / receiveAllChannels / checkLimitStart / checkLimitStop
//       were Nonmatching-EMPTY in the donor decomp — reconstructed
//       instruction-by-instruction from the donor's own retail asm,
//       build/GZLE01/asm/JSystem/JAudio/JASChannelMgr.s. Donor quirks
//       kept, incl. the NON-ADVANCING scan loops in checkLimitStart —
//       see the comment there.)
//   [4] JASChGlobal.cpp
//   [5] JASChAllocQueue.cpp
// Donor include map: JASCalc.h/JUTAssert.h -> ja1_boundary.h;
// JASDSPChannel.h/JASDSPInterface.h/JASDriverIF.h/JASDriverTables.h ->
// ja1_dsp_boundary.h (§368 ONE boundary); JSUList -> receiver JSupport.
// §368 heap adaptation (A2 precedent): every donor `new (JASDram, n)`
// is a plain new — no JA1 arena exists on the PC side until A4.
// ============================================================

#include "d/ext_seq/ja1_jaschannel.h"

#include "global.h"      // ===== §368: ARRAY_SIZE etc. (donor macros.h equivalent)
#include "dolphin/os.h"  // OSReport (donor: dolphin/os/OS.h)

namespace JAudio1 {

// ============================================================
// §368 donor TU [1]: JASChannel.cpp — full, donor-verbatim.
// ============================================================

enum CalcSource {
    CALC_Sound   = 0,
    CALC_Effect  = 1,
    CALC_Channel = 2,
};

enum CalcStyle {
    CALC_NONE   = 0, // don't add that component
    CALC_ADD    = 1, // simply add that component
    CALC_WEIGHT = 2, // add component weighted by power
};

u8 Driver::calc_sw_table[27][3] = {
    // sound, effect, channel
    { CALC_NONE, CALC_NONE, CALC_NONE },       // 0, null
    { CALC_NONE, CALC_NONE, CALC_ADD },        // 1, add only channel
    { CALC_NONE, CALC_NONE, CALC_ADD },        // 2, add only channel
    { CALC_NONE, CALC_ADD, CALC_NONE },        // 3, add only effect
    { CALC_NONE, CALC_ADD, CALC_ADD },         // 4, add effect and channel
    { CALC_NONE, CALC_ADD, CALC_WEIGHT },      // 5, add effect, weight channel
    { CALC_NONE, CALC_ADD, CALC_NONE },        // 6, add only effect
    { CALC_NONE, CALC_WEIGHT, CALC_ADD },      // 7, weight effect, add channel
    { CALC_NONE, CALC_WEIGHT, CALC_WEIGHT },   // 8, weight effect and channel
    { CALC_ADD, CALC_NONE, CALC_NONE },        // 9, add only sound
    { CALC_ADD, CALC_NONE, CALC_ADD },         // 10, add sound and channel
    { CALC_ADD, CALC_NONE, CALC_WEIGHT },      // 11, add sound, weight channel
    { CALC_ADD, CALC_ADD, CALC_NONE },         // 12, add sound and effect
    { CALC_ADD, CALC_ADD, CALC_ADD },          // 13, add all
    { CALC_ADD, CALC_ADD, CALC_WEIGHT },       // 14, add sound and effect, weight channel
    { CALC_ADD, CALC_WEIGHT, CALC_NONE },      // 15, add sound, weight effect
    { CALC_ADD, CALC_WEIGHT, CALC_ADD },       // 16, add sound and channel, weight effect
    { CALC_ADD, CALC_WEIGHT, CALC_WEIGHT },    // 17, add sound, weight effect and channel
    { CALC_ADD, CALC_NONE, CALC_NONE },        // 18, add only sound
    { CALC_WEIGHT, CALC_NONE, CALC_ADD },      // 19, weight sound, add channel
    { CALC_WEIGHT, CALC_NONE, CALC_WEIGHT },   // 20, weight sound and channel
    { CALC_WEIGHT, CALC_ADD, CALC_NONE },      // 21, weight sound, add effect
    { CALC_WEIGHT, CALC_ADD, CALC_ADD },       // 22, weight sound, add effect and channel
    { CALC_WEIGHT, CALC_ADD, CALC_WEIGHT },    // 23, weight sound and channel, weight effect
    { CALC_WEIGHT, CALC_WEIGHT, CALC_NONE },   // 24, weight sound and effect
    { CALC_WEIGHT, CALC_WEIGHT, CALC_ADD },    // 25, weight sound and effect, add channel
    { CALC_WEIGHT, CALC_WEIGHT, CALC_WEIGHT }, // 26, weight all
};

/* 8028B3E8-8028B5A4       .text init__Q28JASystem8TChannelFv */
void TChannel::init() {
    field_0x28 = NULL;
    field_0x2c = NULL;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x10 = NULL;
    field_0xc = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0xd4 = 0;
    if (!field_0x4) {
        mMixConfigs[0].mWhole = 0x150;
        mMixConfigs[1].mWhole = 0x210;
        mMixConfigs[2].mWhole = 0x352;
        mMixConfigs[3].mWhole = 0x412;
        mMixConfigs[4].mWhole = 0;
        mMixConfigs[5].mWhole = 0;
        field_0x48 = 0x10101;
        field_0x4c = 600;
        mCalcTypes[0] = CALC_WeightAll; // Pan
        mCalcTypes[1] = CALC_AddChannelOnly; // FxMix
        mCalcTypes[2] = CALC_AddChannelOnly;
    } else {
        for (int i = 0; i < 6; i++) {
            mMixConfigs[i].mWhole = field_0x4->field_0x4e[i];
        }
        field_0x48 = field_0x4->field_0x68;
        field_0x4c = field_0x4->field_0x6c;
        for (int i = 0; i < 3; i++) {
            mCalcTypes[i] = field_0x4->mCalcTypes[i];
        }
    }
    for (u32 i = 0; i < 4; i++) {
        JUT_ASSERT(155, osc[i]);
        osc[i]->setOsc(NULL);
        osc[i]->init();
    }
    mPauseFlag = 0;
    field_0xc8++;
    if (s32(field_0xc8) == 0) {
        field_0xc8 = 1;
    }
    field_0xe8 = 0;
}

/* 8028B5A4-8028B620       .text setOscillator__Q28JASystem8TChannelFUlPQ28JASystem11TOscillator */
void TChannel::setOscillator(u32 oscnum, TOscillator* param_2) {
    JUT_ASSERT(173, oscnum < (4));
    osc[oscnum] = param_2;
}

/* 8028B620-8028B6A8       .text setOscInit__Q28JASystem8TChannelFUlPCQ38JASystem11TOscillator4Osc_ */
void TChannel::setOscInit(u32 oscnum, const TOscillator::Osc_* param_2) {
    JUT_ASSERT(183, oscnum < (4));
    osc[oscnum]->setOsc(param_2);
    osc[oscnum]->initStart();
}

/* 8028B6A8-8028B73C       .text forceStopOsc__Q28JASystem8TChannelFUl */
bool TChannel::forceStopOsc(u32 numosc) {
    JUT_ASSERT(195, numosc < (4));
    return osc[numosc]->isOsc() ? osc[numosc]->forceStop() : false;
}

/* 8028B73C-8028B7D0       .text releaseOsc__Q28JASystem8TChannelFUl */
bool TChannel::releaseOsc(u32 numosc) {
    JUT_ASSERT(209, numosc < (4));
    return osc[numosc]->isOsc() ? osc[numosc]->release() : false;
}

/* 8028B7D0-8028B850       .text directReleaseOsc__Q28JASystem8TChannelFUlUs */
void TChannel::directReleaseOsc(u32 oscnum, u16 param_2) {
    JUT_ASSERT(224, oscnum < (4));
    osc[oscnum]->releaseDirect(param_2);
}

/* 8028B850-8028B8E4       .text bankOscToOfs__Q28JASystem8TChannelFUl */
f32 TChannel::bankOscToOfs(u32 oscnum) {
    JUT_ASSERT(234, oscnum < (4));
    // Probably uses inline JASystem::TOscillator::bankOscToOfs
    return osc[oscnum]->isOsc() ? osc[oscnum]->getOffset() : 1.0f;
}

/* 8028B8E4-8028BA98       .text effectOsc__Q28JASystem8TChannelFUlf */
void TChannel::effectOsc(u32 oscnum, f32 effect) {
    JUT_ASSERT(246, oscnum < (4));

    switch (osc[oscnum]->getOsc()->field_0x0) {
    case 1:
        field_0x94 *= effect;
        break;
    case 0:
        field_0x98 *= effect;
        break;
    case 2:
        effect -= 0.5; // Must be double literal to match
        mPanVec.mEffect += effect;
        mPanVec.mEffect = Driver::Clamp01(mPanVec.mEffect);
        break;
    case 3:
        mFxmixVec.mEffect += effect;
        mFxmixVec.mEffect = Driver::Clamp01(mFxmixVec.mEffect);
        break;
    case 4:
        mDolbyVec.mEffect += effect;
        mDolbyVec.mEffect = Driver::Clamp01(mDolbyVec.mEffect);
        break;
    }
}

/* 8028BA98-8028BB14       .text getOscState__Q28JASystem8TChannelCFUl */
u8 TChannel::getOscState(u32 oscnum) const {
    JUT_ASSERT(274, oscnum < (4));
    return osc[oscnum]->mState;
}

/* 8028BB14-8028BB98       .text isOsc__Q28JASystem8TChannelFUl */
BOOL TChannel::isOsc(u32 oscnum) {
    JUT_ASSERT(284, oscnum < (4));
    return osc[oscnum]->isOsc();
}

/* 8028BB98-8028BC78       .text copyOsc__Q28JASystem8TChannelFUlPQ38JASystem11TOscillator4Osc_ */
void TChannel::copyOsc(u32 oscnum, TOscillator::Osc_* param_2) {
    JUT_ASSERT(295, oscnum < (4));
    if (isOsc(oscnum)) {
        *param_2 = *osc[oscnum]->getOsc();
    } else {
        OSReport("osc[%d] is NULL\n", oscnum);
    }
}

/* 8028BC78-8028BD10       .text overwriteOsc__Q28JASystem8TChannelFUlPQ38JASystem11TOscillator4Osc_ */
void TChannel::overwriteOsc(u32 oscnum, TOscillator::Osc_* param_2) {
    JUT_ASSERT(308, oscnum < (4));
    setOscInit(oscnum, param_2);
    effectOsc(oscnum, bankOscToOfs(oscnum));
}

/* 8028BD10-8028BDA4       .text setKeySweepTarget__Q28JASystem8TChannelFUcUl */
void TChannel::setKeySweepTarget(u8 key, u32 target) {
    s32 r0;

    if (field_0xc == 2 || field_0x10 == 0)
        r0 = key;
    else
        r0 = key + 0x3C - field_0x10->field_0x2;

    if (r0 < 0)
        r0 = 0;
    else if (r0 > 0x7F)
        r0 = 0x7F;

    f32 val = Driver::C5BASE_PITCHTABLE[r0];
    val *= field_0x50;
    if (target == 0) {
        field_0x58 = val;
        field_0x2c = NULL;
        return;
    }

    field_0x9c = val;
    field_0xa2 = target;
    field_0x2c = &TChannel::extraUpdate;
}

/* 8028BDA4-8028BDAC       .text setPauseFlag__Q28JASystem8TChannelFUc */
void TChannel::setPauseFlag(u8 param_1) {
    mPauseFlag = param_1;
}

/* 8028BDAC-8028BDBC       .text setPauseFlagReq__Q28JASystem8TChannelFUc */
void TChannel::setPauseFlagReq(u8 param_1) {
    mPauseFlag = param_1;
    field_0x3 = 1;
}

/* 8028BDBC-8028BE64       .text setPanPower__Q28JASystem8TChannelFffff */
void TChannel::setPanPower(f32 param_1, f32 param_2, f32 param_3, f32 param_4) {
    f32 px = param_1 + param_2 + param_3;
    if (px == 0.0f) {
        OSReport("----- JASChannel::setPanPower : px == 0.0\n");
        px = 1.0f;
    }
    mPanPower.mSound = param_1 / px;
    mPanPower.mEffect = param_2 / px;
    mPanPower.mChannel = param_3 / px;
}

/* 8028BE64-8028BEB8       .text checkLogicalChannel__Q28JASystem8TChannelFv */
BOOL TChannel::checkLogicalChannel() {
    if (!field_0x10 && field_0xc == 0) {
        OSReport("----- checkLC : 波形がアサインされていません\n");
        return false;
    }
    return true;
}

/* 8028BEB8-8028BF40       .text play__Q28JASystem8TChannelFUl */
BOOL TChannel::play(u32 param_1) {
    if (param_1 == 0) {
        param_1 = -1;
    }
    field_0x30 = param_1;
    field_0x34 = field_0x30;
    field_0x28 = &updatecallLogicalChannel;
    field_0x20 = NULL;
    if (!checkLogicalChannel()) {
        return false;
    }
    TDSPQueue::enQueue(this);
    field_0x4->addListTail(this, 3);
    return true;
}

/* 8028BF40-8028BFAC       .text stop__Q28JASystem8TChannelFUs */
void TChannel::stop(u16 param_1) {
    if (!field_0x20) {
        updatecallLogicalChannel(this, 6);
    } else if (param_1 == 0) {
        updatecallLogicalChannel(this, 0);
    } else {
        directReleaseOsc(0, param_1);
        updatecallLogicalChannel(this, 0);
    }
}

/* 8028BFAC-8028C108       .text updateJcToDSP__Q28JASystem8TChannelFv */
void TChannel::updateJcToDSP() {
    DSPInterface::DSPBuffer* dspBuffer = field_0x20->field_0xc;
    if (field_0xd4) {
        for (u8 i = 0; i < 6; i++) {
            dspBuffer->setMixerVolumeOnly(i, field_0xbc[i]);
        }
        dspBuffer->setPitch(mPitch);
        dspBuffer->setPauseFlag(mPauseFlag);
        dspBuffer->flushChannel();
    } else {
        for (u8 i = 0; i < 6; i++) {
            dspBuffer->setMixerVolume(i, field_0xbc[i], field_0x4->field_0x5a[i]);
        }
        dspBuffer->setPitch(mPitch);
        if (field_0x4->field_0x61 & 0x20) {
            dspBuffer->setIIRFilterParam(field_0x4->field_0x3c);
        }
        if (field_0x4->field_0x61 & 0x1f) {
            dspBuffer->setFIR8FilterParam(field_0x4->field_0x2c);
        }
        dspBuffer->setFilterMode(field_0x4->field_0x61);
        dspBuffer->setDistFilter(field_0x4->field_0x4c);
        dspBuffer->setPauseFlag(mPauseFlag);
        field_0x20->field_0xc->flushChannel();
    }
}

/* 8028C108-8028C140       .text forceStopLogicalChannel__Q28JASystem8TChannelFv */
bool TChannel::forceStopLogicalChannel() {
    if (!field_0x20) {
        return false;
    }
    field_0x20->forceStop();
    return true;
}

/* 8028C140-8028C1C0       .text stopLogicalChannel__Q28JASystem8TChannelFv */
BOOL TChannel::stopLogicalChannel() {
    if (!field_0x20) {
        OSReport("----- stopLC : DSP Ch is not assigned\n");
        return FALSE;
    }
    field_0x20->mCallback = NULL;
    field_0x20->setCBInterval(0);
    field_0x20->stop();
    TDSPChannel::free(field_0x20, uintptr_t(this));  // ===== §368: donor u32(this) — x64 widening
    field_0x20 = NULL;
    return TRUE;
}

/* 8028C1C0-8028C3A8       .text playLogicalChannel__Q28JASystem8TChannelFv */
BOOL TChannel::playLogicalChannel() {
    if (!field_0x20) {
        OSReport("----- playLC DSP Ch が割当てられていません\n");
        return FALSE;
    }

    if (!checkLogicalChannel())
        return FALSE;

    field_0x20->mCallback = &TChannel::updatecallDSPChannel;
    field_0x20->setCBInterval(1);

    DSPInterface::DSPBuffer* buf = field_0x20->field_0xc;

    switch (field_0xc) {
    case 0:
        buf->setWaveInfo(field_0x10, field_0x14, field_0xe8);
        break;
    case 2:
        buf->setOscInfo(field_0x14);
        break;
    }

    for (u8 i = 0; i < 6; ++i) {
        // Something ungodly is occurring here.

        union {
            u16 asS16;
            // ===== §375: donor declares { hi; lo; } — big-endian GC puts
            // hi at the HIGH byte. Mirrored for little-endian x64 so
            // asP.hi keeps reading the donor's connect_table index
            // (mWhole 0x0150 → hi=1=LEFT, not lo-byte 0x50=80 → OOB).
            struct {
                u8 lo;
                u8 hi;
            } asP;
        } s;
        s.asS16 = mMixConfigs[i].mWhole;
        u32 om  = Driver::getOutputMode();

        if (om == 0) {
            switch (s.asP.hi) {
            case 8:
                s.asP.hi = 0xB;
                break;
            case 9:
                s.asP.hi = 0x2;
                break;
            }
        } else if (om == 1) {
            if (s.asP.hi == 8)
                s.asP.hi = 0xB;
        }

        buf->setBusConnect(i, s.asP.hi);
    }

    field_0xa4 = field_0x4;

    for (u32 i = 0; i < 4; ++i) {
        if (isOsc(i)) {
            effectOsc(i, bankOscToOfs(i));
        }
    }

    updateEffectorParam();
    updateJcToDSPInit();
    field_0x20->setPriority(field_0x48);
    field_0x20->setPriorityTime(field_0x4c);
    field_0x20->play();

    return TRUE;
}

/* 8028C3A8-8028C62C       .text updateEffectorParam__Q28JASystem8TChannelFv */
void TChannel::updateEffectorParam() {
    f32 pan;
    f32 fxmix;
    f32 dolby = 0.0f;

    if (field_0xa4 == field_0x4) {
        field_0xa8 = field_0x4->field_0x1c;
        field_0xac = field_0x4->field_0x18;
        mPanVec.mChannel = field_0x4->field_0x20;
        mFxmixVec.mChannel = field_0x4->field_0x24;
        mDolbyVec.mChannel = field_0x4->field_0x28;
        for (int i = 0; i < 3; i++) {
            mCalcTypes[i] = field_0x4->mCalcTypes[i];
        }
    }

    switch (Driver::getOutputMode()) {
    case 0:
        pan   = 0.5f;
        dolby = 0.0f;
        fxmix = calcEffect(&mFxmixVec, &mPanPower, mCalcTypes[1]);
        break;
    case 1:
        pan   = (mCalcTypes[0] == CALC_None) ? 0.5f : calcPan(&mPanVec, &mPanPower, mCalcTypes[0]);
        fxmix = calcEffect(&mFxmixVec, &mPanPower, mCalcTypes[1]);
        dolby = 0.0f;
        break;
    case 2:
        pan   = (mCalcTypes[0] == CALC_None) ? 0.5f : calcPan(&mPanVec, &mPanPower, mCalcTypes[0]);
        fxmix = calcEffect(&mFxmixVec, &mPanPower, mCalcTypes[1]);
        dolby = calcEffect(&mDolbyVec, &mPanPower, mCalcTypes[2]);
        break;
    }

    f32 volume = field_0xac * (field_0x5c * field_0x98);

    pan   = Driver::Clamp01(pan);
    fxmix = Driver::Clamp01(fxmix);
    dolby = Driver::Clamp01(dolby);

    mPitch = 4096.0f * (field_0xa8 * (field_0x58 * field_0x94));

    if (mMixConfigs[0].mWhole != 0xFFFF) {
        updateMixer(volume, pan, fxmix, dolby);
    } else {
        updateAutoMixer(volume, pan, fxmix, dolby);
    }
}

/* 8028C62C-8028C6C4       .text killBrokenLogicalChannels__Q28JASystem8TChannelFPQ28JASystem11TDSPChannel */
void TChannel::killBrokenLogicalChannels(TDSPChannel* dspChannel) {
    TChannelMgr* mgr;
    for (u32 i = 0; i < 256; i++) {
        TChannel* channel = TGlobalChannel::getChannelHandle(i);
        if (channel == NULL) continue;
        if (channel->field_0x20 != dspChannel) continue;
        mgr = channel->field_0x4;
        if (mgr == NULL) continue;
        channel->stopLogicalChannel();
        if (!mgr->moveListHead(channel, 0)) {
            OSReport("----- killBrokenLogicalChannels : Cutできない\n");
        }
    }
}

/* 8028C6C4-8028CABC       .text updatecallDSPChannel__Q28JASystem8TChannelFPQ28JASystem11TDSPChannelUl */
int TChannel::updatecallDSPChannel(TDSPChannel* dspChannel, u32 param_2) {
    TChannel* channel = dspChannel->getLogicalChannel();
    TChannelMgr* mgr = channel->field_0x4;

    u32 i;
    u32 r27 = 0;

    if (channel == NULL) {
        OSReport("-----Error JASDriver::commonCallbackLC DSPchのsignがNULL\n");
        dspChannel->mCallback = NULL;
        dspChannel->mPriority = 0;
        killBrokenLogicalChannels(dspChannel);
        return 0;
    }

    if (channel->field_0x20 != dspChannel) {
        if (channel->field_0x20 != NULL && channel == channel->field_0x20->getLogicalChannel()) {
            killBrokenLogicalChannels(dspChannel);
        } else {
            channel->stopLogicalChannel();
            if (!mgr->moveListHead(channel, 0)) {
                OSReport("----- updatecallDSPChannel : Cutできない\n");
            }
        }
        dspChannel->forceDelete();
        return 0;
    } else {
        if (param_2 == 2) {
            if (channel->field_0x28 != NULL) {
                channel->field_0x28(channel, 1);
            } else {
                channel->stopLogicalChannel();
                if (!mgr->moveListHead(channel, 0)) {
                    OSReport("----- updatecallDSPChannel : Cutできない\n");
                }
            }
            return 0;
        }

        if (channel->field_0x10 != NULL && channel->field_0x10->field_0x24[0] == 0) {
            dspChannel->forceStop();
            return -1;
        }

        if (param_2 == 4) {
            u8 priority = channel->getLifeTimePriority();
            if (channel->field_0x20 != NULL) {
                if (priority < channel->field_0x20->getPriority()) {
                    channel->field_0x20->setPriority(priority);
                }
            }
            return 0;
        }

        if (param_2 == 3) {
            channel->forceStopOsc(0);
            if (!mgr->moveListHead(channel, 3)) {
                OSReport("----- updatecallDSPChannel : Cutできない\n");
                return 1;
            }
            param_2 = 0;
        }

        if (param_2 == 0) {
            channel->field_0x94 = 1.0f;
            channel->field_0x98 = 1.0f;
            channel->mPanVec.mEffect = 0.5f;
            channel->mFxmixVec.mEffect = 0.0f;
            channel->mDolbyVec.mEffect = 0.0f;

            for (i = 0; i < 4; i++) {
                if (!channel->isOsc(i)) continue;
                channel->effectOsc(i, channel->bankOscToOfs(i));
                if (i == 0 && channel->getOscState(i) == 0) {
                    if (channel->field_0x28 == NULL) {
                        OSReport("----- updatecallDSPCh JC停止のためのUPDATECALLがNULL\n");
                        if (!channel->stopLogicalChannel()) {
                            dspChannel->stop();
                        }
                        if (!mgr->moveListHead(channel, 0)) {
                            OSReport("----- updatecallDSPChannel : Cutできない update\n");
                        }
                        return 0;
                    }
                    channel->field_0x28(channel, 2);
                    return 0;
                }
                r27++;
            }

            if (r27 != 0) {
                channel->updateEffectorParam();
                channel->field_0x3 = 1;
            }

            if (channel->field_0x2c != NULL) {
                if (channel->field_0x2c(channel, 0) == 1) {
                    channel->field_0x3++;
                }
            }

            u8 updateInterval = Driver::getUpdateInterval();
            if (channel->field_0x28 == NULL) {
                return updateInterval;
            }
            if (channel->field_0x34 > 0) {
                if (channel->field_0x34 > updateInterval) {
                    channel->field_0x34 -= updateInterval;
                } else {
                    channel->field_0x34 = 0;
                }
            }
        }

        if (channel->field_0x34 == 0) {
            channel->field_0x28(channel, 0);
            channel->field_0x34 = channel->field_0x30;
        }
        if (channel->field_0x3 != 0) {
            channel->updateJcToDSP();
            channel->field_0x3 = 0;
        }

        return Driver::getUpdateInterval();
    }
    dspChannel->forceDelete();
}

/* 8028CABC-8028CB88       .text calcEffect__Q28JASystem8TChannelFPCQ38JASystem6Driver10PanMatrix_PCQ38JASystem6Driver10PanMatrix_Uc */
f32 TChannel::calcEffect(const Driver::PanMatrix_* params, const Driver::PanMatrix_* power, u8 calcType) {
    f32 value = 0.0f;
    const u8* calcTypes = Driver::calc_sw_table[calcType];
    switch (calcTypes[CALC_Sound]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += params->mSound;
        break;
    case CALC_WEIGHT:
        value += params->mSound * power->mSound;
        break;
    }

    switch (calcTypes[CALC_Effect]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += params->mEffect;
        break;
    case CALC_WEIGHT:
        value += params->mEffect * power->mEffect;
        break;
    }

    switch (calcTypes[CALC_Channel]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += params->mChannel;
        break;
    case CALC_WEIGHT:
        value += params->mChannel * power->mChannel;
        break;
    }

    return value;
}

/* 8028CB88-8028CC90       .text calcPan__Q28JASystem8TChannelFPCQ38JASystem6Driver10PanMatrix_PCQ38JASystem6Driver10PanMatrix_Uc */
f32 TChannel::calcPan(const Driver::PanMatrix_* params, const Driver::PanMatrix_* power, u8 calcType) {
    f32 value = 0.0f;
    const u8* calcTypes = Driver::calc_sw_table[calcType];
    switch (calcTypes[CALC_Sound]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += (params->mSound - 0.5f);
        break;
    case CALC_WEIGHT:
        value += (params->mSound - 0.5f) * power->mSound;
        break;
    }

    switch (calcTypes[CALC_Effect]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += (params->mEffect - 0.5f);
        break;
    case CALC_WEIGHT:
        value += (params->mEffect - 0.5f) * power->mEffect;
        break;
    }

    switch (calcTypes[CALC_Channel]) {
    case CALC_NONE:
        break;
    case CALC_ADD:
        value += (params->mChannel - 0.5f);
        break;
    case CALC_WEIGHT:
        value += (params->mChannel - 0.5f) * power->mChannel;
        break;
    }

    value += 0.5f;
    return value;
}

/* 8028CC90-8028CD90       .text updateJcToDSPInit__Q28JASystem8TChannelFv */
void TChannel::updateJcToDSPInit() {
    DSPInterface::DSPBuffer* buf = field_0x20->field_0xc;

    if (mMixConfigs[0].mWhole == 0xFFFF) {
        field_0x20->field_0xc->initAutoMixer();
    } else {
        buf->setMixerInitDelayMax(field_0x4->field_0x60);
        for (u8 i = 0; i < 6; ++i)
            buf->setMixerInitVolume(i, field_0xbc[i],
                                    field_0x4->field_0x5a[i]);
    }

    buf->setPitch(mPitch);
    if (field_0x4->field_0x61 & 0x20)
        buf->setIIRFilterParam(field_0x4->field_0x3c);
    if (field_0x4->field_0x61 & 0x1F)
        buf->setFIR8FilterParam(field_0x4->field_0x2c);

    buf->setFilterMode(field_0x4->field_0x61);
    buf->setPauseFlag(mPauseFlag);
}

/* 8028CD90-8028CEA8       .text updateAutoMixer__Q28JASystem8TChannelFffff */
void TChannel::updateAutoMixer(f32 volume, f32 pan, f32 fxmix, f32 dolby) {
    field_0x20->field_0xc->setAutoMixer(
        Driver::Clamp01(volume) * Driver::getAutoLevel(),
        pan * 127.5f, dolby * 127.5f, fxmix * 127.5f,
        mMixConfigs[1].mWhole
    );
}

/* 8028CEA8-8028D128       .text updateMixer__Q28JASystem8TChannelFffff */
void TChannel::updateMixer(f32 volume, f32 pan, f32 fxmix, f32 dolby) {
    for (u32 i = 0; i < 6; i++) {
        f32 vol = volume;
        MixConfig config = mMixConfigs[i];
        if (config.mParts.u == 0) {
            field_0xbc[i] = 0;
        } else {
            f32 scale;

            if (config.mParts.l0 != 0) {
                switch (config.mParts.l0) {
                case 1:
                    scale = pan;
                    break;
                case 2:
                    scale = fxmix;
                    break;
                case 3:
                    scale = dolby;
                    break;
                case 5:
                    scale = 1.0f - pan;
                    break;
                case 6:
                    scale = 1.0f - fxmix;
                    break;
                case 7:
                    scale = 1.0f - dolby;
                    break;
                }

                vol *= Calc::sinfT(scale);
            }

            if (config.mParts.l1 != 0) {
                switch (config.mParts.l1) {
                case 1:
                    scale = pan;
                    break;
                case 2:
                    scale = fxmix;
                    break;
                case 3:
                    scale = dolby;
                    break;
                case 5:
                    scale = 1.0f - pan;
                    break;
                case 6:
                    scale = 1.0f - fxmix;
                    break;
                case 7:
                    scale = 1.0f - dolby;
                    break;
                }

                switch (config.mParts.l1) {
                case 3:
                case 7:
                    vol *= Calc::sinfDolby2(scale);
                    break;
                default:
                    vol *= Calc::sinfT(scale);
                    break;
                }
            }

            field_0xbc[i] = Driver::Clamp01(vol) * Driver::getChannelLevel();
        }
    }
}

/* 8028D128-8028D218       .text extraUpdate__Q28JASystem8TChannelFPQ28JASystem8TChannelUl */
u32 TChannel::extraUpdate(TChannel* channel, u32) {
    if (channel->field_0xa2 != 0) {
        f32 f31 = channel->field_0x9c - channel->field_0x58;
        u8 updateInterval = Driver::getUpdateInterval();
        if (channel->field_0xa2 <= updateInterval) {
            channel->field_0xa2 = 1;
        }
        f31 /= channel->field_0xa2;
        channel->field_0x58 += f31 * updateInterval;
        if ((channel->field_0xa2 - updateInterval) <= 0) {
            channel->field_0xa2 = 0;
        } else {
            channel->field_0xa2 -= updateInterval;
        }
        if (channel->field_0xa2 == 0) {
            channel->field_0x2c = NULL;
        }
        return 1;
    } else {
        return 0;
    }
}

/* 8028D218-8028D3C0       .text updatecallLogicalChannel__Q28JASystem8TChannelFPQ28JASystem8TChannelUl */
BOOL TChannel::updatecallLogicalChannel(TChannel* channel, u32 param) {
    TChannelMgr* mgr = channel->field_0x4;

    if (param == 0) {
        for (u32 i = 0; i < 4; ++i)
            channel->releaseOsc(i);

        if (channel->field_0x20)
            channel->field_0x20->mPriority = channel->getReleasePriority();

        if (!mgr->moveListTail(channel, 2)) {
            OSReport("----- updatecallLC : CUT失敗(release)\n");
        }
        channel->field_0x30 = -1;
        return FALSE;
    } else if (param == 1 || param == 2 || param == 6) {
        if (mgr->field_0x4 != 0) {
            if (mgr->cutList(channel) == -1) {
                OSReport("----- updatecallLC : CUT失敗(extra)\n");
            } else {
                --mgr->field_0x4;
                if (u32 thing = channel->field_0xcc) {
                    channel->field_0xcc = 0;
                    mgr->checkLimitStop(channel, thing);
                }
                TGlobalChannel::release(channel);
            }
        } else {
            int r29 = channel->field_0xcc;
            if (!mgr->moveListHead(channel, 0)) {
                OSReport("----- updatecallLC : CUT失敗(not extra)\n");
            }
            mgr->checkLimitStop(channel, r29);
        }

        if (param != 6)
            channel->stopLogicalChannel();
        else
            TDSPQueue::deleteQueue(channel);

        channel->field_0x1  = 0xFF;
        channel->field_0x30 = 0xFFFFFFFF;
        channel->field_0x28 = NULL;
    }
    return FALSE;
}

// ============================================================
// §368 donor TU [2]: donor data the channel/bank layer consumes.
// C5BASE_PITCHTABLE — donor JASDriverTables.cpp:10 (verbatim excerpt;
// the rest of that TU is DSP microcode filter data owned by the §368
// DSP boundary / A4). polys_table — donor JASChannelMgr.cpp rodata
// (build/GZLE01 .rodata 0x80368720, verbatim bytes).
// ============================================================
f32 Driver::C5BASE_PITCHTABLE[128] = {
    0.031250f, 0.033108f, 0.035077f, 0.037162f,
    0.039372f, 0.041713f, 0.044194f, 0.046822f,
    0.049606f, 0.052556f, 0.055681f, 0.058992f,
    0.062500f, 0.066216f, 0.070154f, 0.074325f,
    0.078745f, 0.083427f, 0.088388f, 0.093644f,
    0.099213f, 0.105112f, 0.111362f, 0.117984f,
    0.125000f, 0.132433f, 0.140308f, 0.148651f,
    0.157490f, 0.166855f, 0.176777f, 0.187288f,
    0.198425f, 0.210224f, 0.222725f, 0.235969f,
    0.250000f, 0.264866f, 0.280616f, 0.297302f,
    0.314980f, 0.333710f, 0.353553f, 0.374577f,
    0.396850f, 0.420448f, 0.445449f, 0.471937f,
    0.500000f, 0.529732f, 0.561231f, 0.594604f,
    0.629961f, 0.667420f, 0.707107f, 0.749154f,
    0.793701f, 0.840897f, 0.890899f, 0.943875f,
    1.000000f, 1.059463f, 1.122462f, 1.189207f,
    1.259921f, 1.334840f, 1.414214f, 1.498307f,
    1.587401f, 1.681793f, 1.781798f, 1.887749f,
    2.000000f, 2.118926f, 2.244924f, 2.378414f,
    2.519842f, 2.669680f, 2.828428f, 2.996615f,
    3.174803f, 3.363586f, 3.563596f, 3.775498f,
    4.000000f, 4.237853f, 4.489849f, 4.756829f,
    5.039685f, 5.339360f, 5.656855f, 5.993229f,
    6.349606f, 6.727173f, 7.127192f, 7.550996f,
    8.000000f, 8.475705f, 8.979697f, 9.513658f,
    10.079370f, 10.678720f, 11.313710f, 11.986459f,
    12.699211f, 13.454346f, 14.254383f, 15.101993f,
    16.000000f, 16.951410f, 17.959394f, 19.027315f,
    20.158739f, 21.357441f, 22.627420f, 23.972918f,
    25.398422f, 26.908691f, 28.508766f, 30.203985f,
    32.000000f, 33.902821f, 35.918789f, 38.054630f,
    40.317478f, 42.714882f, 45.254840f, 47.945835f,
};

u8 Driver::polys_table[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
    0x0A, 0x0C, 0x0E, 0x10, 0x14, 0x18, 0x1C, 0x20,
};

// ============================================================
// §368 donor TU [3]: JASChannelMgr.cpp — decomp bodies verbatim;
// the five Nonmatching-EMPTY functions reconstructed from the donor's
// own GZLE01 retail asm (JASChannelMgr.s), instruction-faithful.
// ============================================================

/* 8028D3C0-8028D4D0       .text init__Q28JASystem11TChannelMgrFv */
void TChannelMgr::init() {
    /* Nonmatching */
    field_0x8 = NULL;
    field_0xc = NULL;
    field_0x10 = NULL;
    field_0x14 = NULL;
    field_0x4 = 0;
    field_0x0 = 0;
    field_0x70 = 1;
    field_0x18 = 1.0f;
    field_0x1c = 1.0f;
    field_0x20 = 0.5f;
    field_0x24 = 0.0f;
    field_0x28 = 0.0f;
    for (int i = 0; i < 8; i++) {
        field_0x2c[i] = 0;
    }
    field_0x2c[0] = 0x7fff;
    field_0x4c = 0;
    for (int i = 0; i < 4; i++) {
        field_0x3c[i] = 0;
    }
    for (int i = 0; i < 6; i++) {
        field_0x5a[i] = 0;
    }
    field_0x60 = 0;
    field_0x3c[0] = 0x7fff;
    field_0x61 = 0;
    field_0x4e[0] = 0x150;
    field_0x4e[1] = 0x210;
    field_0x4e[2] = 0x352;
    field_0x4e[3] = 0x412;
    field_0x4e[4] = 0;
    field_0x4e[5] = 0;
    field_0x68 = 0x20103;
    field_0x6c = 600;
    mCalcTypes[0] = 13;
    mCalcTypes[1] = 13;
    mCalcTypes[2] = 13;
}

/* 8028D4D0-8028D514       .text stopAll__Q28JASystem11TChannelMgrFv */
void TChannelMgr::stopAll() {
    TChannel* channel = field_0xc;
    while (channel) {
        TChannel* r31 = channel->field_0x24;
        channel->stop(0);
        channel = r31;
    }
}

/* 8028D514-8028D558       .text stopAllRelease__Q28JASystem11TChannelMgrFv */
void TChannelMgr::stopAllRelease() {
    for (TChannel* channel = field_0x10; channel; channel = channel->field_0x24) {
        channel->forceStopOsc(0);
    }
}

/* 8028D558-8028D5D0       .text initAllocChannel__Q28JASystem11TChannelMgrFUl */
void TChannelMgr::initAllocChannel(u32 param_1) {
    if (field_0x0) {
        OSReport("----- Warning JCSにボイスが %d 残っているのでグローバルに返却します\n", field_0x0);
        TGlobalChannel::releaseAll(this);
    }
    init();
    TGlobalChannel::alloc(this, param_1);
    field_0x70 = (param_1 != 0) ? 1 : 0;
}

/* 8028D5D0-8028D778       .text getLogicalChannel__Q28JASystem11TChannelMgrFUl */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// from GZLE01 retail asm (JASChannelMgr.s :170-286), donor strings kept.
TChannel* TChannelMgr::getLogicalChannel(u32 param_1) {
    bool pauseNew = false;
    if (checkLimitStart(param_1) == 0) {
        if (param_1 & 0x10000000) {
            return NULL;
        }
        pauseNew = true;
    }
    TChannel* channel = getListHead(0);
    if (channel == NULL) {
        if (TGlobalChannel::alloc(this, 1) == 0) {
            OSReport("----- getLC グローバル論理チャンネルの残りがない\n");
            return NULL;
        }
        field_0x4++;
        channel = getListHead(0);
        if (field_0x70 == 1) {
            TChannel* victim = getListHead(2);
            if (victim == NULL) {
                victim = getListHead(1);
                if (victim == NULL) {
                    OSReport("----- getLC Error ----- fjc is NULL again\n");
                }
            }
            if (victim != NULL) {
                victim->forceStopOsc(0);
                addListHead(victim, 3);
                if (victim->field_0x20 != NULL) {
                    victim->field_0x20->forceStop();
                } else {
                    OSReport("----- getLC -----does not have DSP CH\n");
                }
            } else {
                OSReport("cannot FORCESTOP (thisの論理ボイスが飽和?)\n");
            }
        }
    }
    channel->init();
    if (pauseNew) {
        channel->mPauseFlag = 1;
        channel->field_0x3 = 1;
    }
    channel->field_0xcc = param_1;
    channel->field_0x18 = 0;
    channel->setPanPower(1.0f, 1.0f, 1.0f, 1.0f);
    return channel;
}

/* 8028D778-8028D7D8       .text moveListHead__Q28JASystem11TChannelMgrFPQ28JASystem8TChannelUl */
BOOL TChannelMgr::moveListHead(TChannel* param_1, u32 param_2) {
    if (cutList(param_1) == -1) {
        return FALSE;
    }
    addListHead(param_1, param_2);
    return TRUE;
}

/* 8028D7D8-8028D838       .text moveListTail__Q28JASystem11TChannelMgrFPQ28JASystem8TChannelUl */
BOOL TChannelMgr::moveListTail(TChannel* param_1, u32 param_2) {
    if (cutList(param_1) == -1) {
        return FALSE;
    }
    addListTail(param_1, param_2);
    return TRUE;
}

/* 8028D838-8028D8E4       .text addListHead__Q28JASystem11TChannelMgrFPQ28JASystem8TChannelUl */
void TChannelMgr::addListHead(TChannel* param_1, u32 param_2) {
    /* Nonmatching */
    TChannel** r31;
    switch (param_2) {
    case 0:
        r31 = &field_0x8;
        break;
    case 1:
        r31 = &field_0xc;
        break;
    case 2:
        r31 = &field_0x10;
        break;
    case 3:
        r31 = &field_0x14;
        break;
    default:
        r31 = NULL;
        break;
    }
    TChannel* r30 = *r31;
    if (param_1->field_0x8) {
        OSReport("RootJc Error 2\n");
    }
    param_1->field_0x8 = r31;
    *r31 = param_1;
    param_1->field_0x24 = r30;
}

/* 8028D8E4-8028D9C4       .text addListTail__Q28JASystem11TChannelMgrFPQ28JASystem8TChannelUl */
void TChannelMgr::addListTail(TChannel* param_1, u32 param_2) {
    /* Nonmatching */
    TChannel** r31;
    switch (param_2) {
    case 0:
        r31 = &field_0x8;
        break;
    case 1:
        r31 = &field_0xc;
        break;
    case 2:
        r31 = &field_0x10;
        break;
    case 3:
        r31 = &field_0x14;
        break;
    default:
        r31 = NULL;
        break;
    }
    TChannel* r30 = *r31;
    if (param_1->field_0x8) {
        OSReport("ROOTJC Error\n");
    }
    param_1->field_0x8 = r31;
    if (!r30) {
        *r31 = param_1;
        param_1->field_0x24 = NULL;
        return;
    }
    while (true) {
        TChannel* tmp = r30->field_0x24;
        if (!tmp) {
            r30->field_0x24 = param_1;
            param_1->field_0x24 = NULL;
            break;
        }
        r30 = tmp;
    }
}

/* 8028D9C4-8028DA38       .text getListHead__Q28JASystem11TChannelMgrFUl */
TChannel* TChannelMgr::getListHead(u32 param_1) {
    TChannel** r31;
    switch (param_1) {
    case 0:
        r31 = &field_0x8;
        break;
    case 1:
        r31 = &field_0xc;
        break;
    case 2:
        r31 = &field_0x10;
        break;
    case 3:
        r31 = &field_0x14;
        break;
    default:
        r31 = NULL;
        break;
    }
    TChannel* r30 = *r31;
    if (!r30) {
        return NULL;
    }
    *r31 = r30->field_0x24;
    r30->field_0x8 = NULL;
    return r30;
}

/* 8028DA38-8028DAF0       .text cutList__Q28JASystem11TChannelMgrFPQ28JASystem8TChannel */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// from GZLE01 retail asm (JASChannelMgr.s :522-574), donor strings kept.
int TChannelMgr::cutList(TChannel* param_1) {
    int count = 0;
    TChannel** head = param_1->field_0x8;
    TChannel* cur = *head;
    if (cur == NULL) {
        OSReport("cutChList Error: No Member\n");
        return -1;
    }
    if (cur == param_1) {
        *head = param_1->field_0x24;
        param_1->field_0x8 = NULL;
        return 0;
    }
    while (true) {
        if (cur == NULL) {
            OSReport("cutChList Error: Not Member\n");
            return -1;
        }
        count++;
        if (cur->field_0x24 == param_1) {
            cur->field_0x24 = param_1->field_0x24;
            param_1->field_0x8 = NULL;
            return count;
        }
        cur = cur->field_0x24;
    }
}

/* 8028DAF0-8028DC34       .text receiveAllChannels__Q28JASystem11TChannelMgrFPQ28JASystem11TChannelMgr */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// from GZLE01 retail asm (JASChannelMgr.s :578-669).
void TChannelMgr::receiveAllChannels(TChannelMgr* param_1) {
    TChannel* channel;
    while ((channel = param_1->getListHead(0)) != NULL) {
        addListHead(channel, 0);
        channel->field_0x4 = this;
    }
    while ((channel = param_1->getListHead(1)) != NULL) {
        addListHead(channel, 1);
        channel->field_0x4 = this;
        channel->field_0xd4 = 1;
    }
    while ((channel = param_1->getListHead(2)) != NULL) {
        addListHead(channel, 2);
        channel->field_0x4 = this;
        channel->field_0xd4 = 1;
    }
    while ((channel = param_1->getListHead(3)) != NULL) {
        if (TDSPQueue::deleteQueue(channel)) {
            addListHead(channel, 0);
        } else {
            addListHead(channel, 3);
        }
        channel->field_0x4 = this;
    }
    field_0x0 += param_1->field_0x0;
    param_1->field_0x0 = 0;
    field_0x4 += param_1->field_0x4;
    param_1->field_0x4 = 0;
}

/* 8028DC34-8028DDD0       .text checkLimitStart__Q28JASystem11TChannelMgrFUl */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// from GZLE01 retail asm (JASChannelMgr.s :673-796).
// DONOR BUG KEPT: the four scan loops below never advance their cursor
// (the retail binary at 8028DC74/8028DC9C/8028DD0C/8028DD5C loops on
// the same element — verified instruction-by-instruction). They are
// reached only when polys_table[(key>>24)&0xF] != 0, i.e. SE-category
// alloc keys; the BGM noteOn path always carries kind nibble 0 and
// early-returns 1. Flagged for the A4/SE review — do not "fix" without
// a donor-behavior ruling (§366 verbatim doctrine).
int TChannelMgr::checkLimitStart(u32 param_1) {
    u8 kind = param_1 >> 24;
    u8 limit = Driver::polys_table[(param_1 >> 24) & 0xF];
    if (limit == 0) {
        return 1;
    }
    u32 count = 0;
    TChannel* channel = field_0x10;
    if (kind & 0x20) {
        while (channel != NULL) {  // §368 donor non-advancing loop (see above)
            if (channel->field_0xcc == param_1 && channel->mPauseFlag == 0) {
                count++;
            }
        }
        channel = field_0xc;
        while (channel != NULL) {  // §368 donor non-advancing loop (see above)
            if (channel->field_0xcc == param_1 && channel->mPauseFlag == 0) {
                count++;
            }
        }
        if (count == limit) {
            return 0;
        }
        if (count > limit) {
            OSReport("----- checkLimitStart Why? chNum is over\n");
            return 0;
        }
        return 1;
    } else {
        TChannel* first = NULL;
        while (channel != NULL) {  // §368 donor non-advancing loop (see above)
            if (channel->field_0xcc == param_1 && channel->mPauseFlag == 0) {
                if (first == NULL) {
                    first = channel;
                }
                if (count == limit) {
                    first->forceStopLogicalChannel();
                    return 1;
                }
                count++;
            }
        }
        channel = field_0xc;
        while (channel != NULL) {  // §368 donor non-advancing loop (see above)
            if (channel->field_0xcc == param_1 && channel->mPauseFlag == 0) {
                if (first == NULL) {
                    first = channel;
                }
                if (count == limit) {
                    if (kind & 0x10) {
                        first->forceStopLogicalChannel();
                    } else {
                        first->setPauseFlagReq(1);
                    }
                    return 1;
                }
                count++;
            }
        }
        return 1;
    }
}

/* 8028DDD0-8028DE94       .text checkLimitStop__Q28JASystem11TChannelMgrFPQ28JASystem8TChannelUl */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// from GZLE01 retail asm (JASChannelMgr.s :800-857). The TChannel*
// parameter is unused in the donor binary (quirk kept).
void TChannelMgr::checkLimitStop(TChannel* param_1, u32 param_2) {
    (void)param_1;
    TChannel* channel = field_0xc;
    u8 kind = param_2 >> 24;
    u8 limit = Driver::polys_table[(param_2 >> 24) & 0xF];
    if (param_2 == 0) {
        return;
    }
    if (limit == 0) {
        return;
    }
    if (kind & 0x20) {
        for (; channel != NULL; channel = channel->field_0x24) {
            if (channel->field_0xcc == param_2 && channel->mPauseFlag == 1) {
                channel->setPauseFlagReq(0);
                return;
            }
        }
    } else {
        TChannel* last = NULL;
        for (; channel != NULL; channel = channel->field_0x24) {
            if (channel->field_0xcc == param_2 && channel->mPauseFlag == 1) {
                last = channel;
            }
        }
        if (last != NULL) {
            last->setPauseFlagReq(0);
        }
    }
}

// ============================================================
// §368 donor TU [4]: JASChGlobal.cpp — donor-verbatim (heap adaptation:
// donor `new (JASDram, 32)` -> plain new, §368 header note).
// ============================================================

TChannelMgr* TGlobalChannel::sChannelMgr;
TChannel* TGlobalChannel::sChannel;
TOscillator* TGlobalChannel::sOscillator;

/* 8028AAEC-8028AB58       .text getChannelHandle__Q28JASystem14TGlobalChannelFUl */
TChannel* TGlobalChannel::getChannelHandle(u32 ch_num) {
    JUT_ASSERT(34, ch_num < (256));
    return sChannel + ch_num;
}

/* 8028AB58-8028AD50       .text init__Q28JASystem14TGlobalChannelFv */
void TGlobalChannel::init() {
    sChannelMgr = new TChannelMgr();  // ===== §368: donor `new (JASDram, 32)`
    JUT_ASSERT(44, sChannelMgr);
    TChannelMgr* mgr = sChannelMgr;
    mgr->init();
    sChannel = new TChannel[256];  // ===== §368: donor `new (JASDram, 32)`
    JUT_ASSERT(50, sChannel);
    sOscillator = new TOscillator[1024];  // ===== §368: donor `new (JASDram, 32)`
    JUT_ASSERT(52, sOscillator);
    for (int i = 0; i < 256; i++) {
        for (u32 j = 0; j < 4; j++) {
            sChannel[i].setOscillator(j, &sOscillator[i * 4 + j]);
        }
        sChannel[i].init();
        mgr->addListHead(&sChannel[i], 0);
        sChannel[i].field_0x4 = mgr;
    }
    mgr->field_0x0 = 256;
    OSReport("----- JASChannel size : %d\n", sizeof(TChannel));
}

/* 8028AD50-8028ADE8       .text alloc__Q28JASystem14TGlobalChannelFPQ28JASystem11TChannelMgrUl */
int TGlobalChannel::alloc(TChannelMgr* param_1, u32 param_2) {
    u32 i;
    for (i = 0; i < param_2; i++) {
        TChannel* channel = sChannelMgr->getListHead(0);
        if (!channel) {
            break;
        }
        param_1->addListHead(channel, 0);
        channel->field_0x4 = param_1;
        channel->init();
    }
    param_1->field_0x0 += i;
    sChannelMgr->field_0x0 -= i;
    return i;
}

/* 8028ADE8-8028AE4C       .text release__Q28JASystem14TGlobalChannelFPQ28JASystem8TChannel */
int TGlobalChannel::release(TChannel* param_1) {
    sChannelMgr->addListHead(param_1, 0);
    param_1->field_0x4->field_0x0--;
    sChannelMgr->field_0x0++;
    param_1->field_0x4 = sChannelMgr;
    return 0;
}

/* 8028AE4C-8028AF8C       .text releaseAll__Q28JASystem14TGlobalChannelFPQ28JASystem11TChannelMgr */
int TGlobalChannel::releaseAll(TChannelMgr* param_1) {
    TChannel* channel;
    while (true) {
        channel = param_1->getListHead(0);
        if (!channel) {
            break;
        }
        sChannelMgr->addListHead(channel, 0);
        channel->field_0x4 = sChannelMgr;
    };
    while (true) {
        channel = param_1->getListHead(1);
        if (!channel) {
            break;
        }
        sChannelMgr->addListHead(channel, 1);
        channel->field_0x4 = sChannelMgr;
        channel->field_0xd4 = 1;
    }
    while (true) {
        channel = param_1->getListHead(2);
        if (!channel) {
            break;
        }
        sChannelMgr->addListHead(channel, 2);
        channel->field_0x4 = sChannelMgr;
        channel->field_0xd4 = 1;
    }
    while (true) {
        channel = param_1->getListHead(3);
        if (!channel) {
            break;
        }
        if (TDSPQueue::deleteQueue(channel)) {
            sChannelMgr->addListHead(channel, 0);
        } else {
            sChannelMgr->addListHead(channel, 3);
        }
        channel->field_0x4 = sChannelMgr;
    }
    sChannelMgr->field_0x0 += param_1->field_0x0;
    param_1->field_0x0 = 0;
    return 0;
}

// ============================================================
// §368 donor TU [5]: JASChAllocQueue.cpp — donor-verbatim.
// ============================================================

JSUList<TChannel> sDspQueueList;

/* 8028B0C4-8028B224       .text deQueue__Q28JASystem9TDSPQueueFv */
void TDSPQueue::deQueue() {
    JSULink<TChannel>* link;
    JSULink<TChannel>* next;
    u32 r31 = TDSPChannel::getNumBreak();
    for (link = sDspQueueList.getFirst(); link; link = next) {
        next = link->getNext();
        TChannel* channel = link->getObject();
        TDSPChannel* dspChannel = TDSPChannel::alloc(0, uintptr_t(channel));  // ===== §368: donor u32(channel) — x64 widening
        if (!dspChannel) {
            if (r31) {
                r31--;
                continue;
            }
            if (TDSPChannel::breakLower(channel->getNoteOnPriority())) {
                continue;
            }
            channel->field_0x28(channel, 6);
            sDspQueueList.remove(&channel->field_0xd8);
        } else {
            channel->field_0x20 = dspChannel;
            int status = channel->playLogicalChannel();
            JUT_ASSERT(69, status);
            int sst = channel->field_0x4->cutList(channel);
            JUT_ASSERT(72, sst >= 0);
            channel->field_0x4->addListTail(channel, 1);
            sDspQueueList.remove(&channel->field_0xd8);
        }

    }
}

/* 8028B224-8028B288       .text enQueue__Q28JASystem9TDSPQueueFPQ28JASystem8TChannel */
void TDSPQueue::enQueue(TChannel* param_1) {
    JSUListIterator<TChannel> it;
    for (it = sDspQueueList.getFirst(); it != sDspQueueList.getEnd(); it++) {
        if (param_1->getNoteOnPriority() > it->getNoteOnPriority()) {
            break;
        }
    }
    sDspQueueList.insert(it.mLink, &param_1->field_0xd8);
}

/* 8028B288-8028B2BC       .text deleteQueue__Q28JASystem9TDSPQueueFPQ28JASystem8TChannel */
int TDSPQueue::deleteQueue(TChannel* param_1) {
    return sDspQueueList.remove(&param_1->field_0xd8);
}

/* 8028B2BC-8028B350       .text checkQueue__Q28JASystem9TDSPQueueFv */
void TDSPQueue::checkQueue() {
    deQueue();
    JSULink<TChannel>* next;
    for (JSULink<TChannel>* link = sDspQueueList.getFirst(); link; link = next) {
        next = link->getNext();
        TChannel* channel = link->getObject();
        if (channel->field_0x30 > 0) {
            channel->field_0x30--;
        }
        if (channel->field_0x30 == 0) {
            channel->field_0x28(channel, 6);
            sDspQueueList.remove(&channel->field_0xd8);
        }
    }
}

}  // namespace JAudio1
