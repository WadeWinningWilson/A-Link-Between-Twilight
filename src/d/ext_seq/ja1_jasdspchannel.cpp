// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASDSPChannel.cpp NonMatching
// KIT-DONOR: JSystem/JAudio/JASDSPInterface.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §369 JAudio1 DSP voice-slot pool — donor-verbatim port of WW retail
// JSystem/JAudio/JASDSPChannel.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept). Phase A4 of the native JA1 campaign (bus
// §362/§363/§366; History lane §369). Donor JASystem::* flattens into
// JAudio1::*, donor symbol names kept.
//
// POOL POLICY (§369 — the decision ja1_dsp_boundary.h documents):
// the donor machinery runs UNCHANGED over a RESERVED SLICE of the one
// PC JASDsp voice array DuskDsp mixes — voices [kJa1DspVoiceFirst,
// kJa1DspVoiceFirst + kJa1DspVoiceCount). initAll() fences the JA2
// allocator off the slice via JASDSPChannel::setJa1ReservedRange
// (data-driven; count stays 0 gate-OFF, so the JA2 pool is untouched
// until the gate-ON init actually runs). The JA1<->JA2 allocators are
// NOT bridged per-voice: their lifecycle protocols are incompatible
// (see ja1_dsp_boundary.h header) and a shim state machine would be
// invented behavior.
//
// PC adaptations (each marked §369):
//   - donor literal 64 (pool size) -> kJa1DspVoiceCount at every site;
//     the pool INDEX space stays donor-shaped 0..count-1, the PC voice
//     attach adds kJa1DspVoiceFirst once, in initAll.
//   - donor raw DSPBuffer register fields -> the same registers on the
//     PC JASDsp::TChannel block through DSPBuffer::mPcVoice
//     (field-for-field identical 0x180 layout, §368-verified):
//       field_0x0 -> mIsActive     field_0x2   -> mIsFinished
//       field_0x68 -> mSamplePosition
//       field_0x10a -> mForcedStop field_0x10c -> field_0x10c
//   - DSPReleaseHalt2 -> JASDsp::releaseHalt (the receiver's own PC
//     realization; a documented no-op on PC — same call JA2's
//     JASDSPChannel::updateAll makes).
//   - the donor DSP-overload kill heuristic (OSGetTick history ratio ->
//     breakLowerActive(126)) is compiled out under
//     JA1_DSP_OVERLOAD_KILL=0, the receiver's own precedent: JA2's
//     identical block is `#if !TARGET_PC` with the comment "Our audio
//     engine isn't consistent enough and hits this incorrectly"
//     (libs/JSystem/src/JAudio2/JASAiCtrl.cpp:166). Donor data rows
//     (history/old_time/DSP_LIMIT_RATIO) are ported regardless.
//   - `new (JASDram, 0x20)` -> plain new (A2/A3 heap precedent).
//   - donor u32 "sign" -> uintptr_t (§368 x64 widening, carried).
// ============================================================

#include "d/ext_seq/ja1_dsp_boundary.h"
#include "dusk/logging.h"  // §373 pipeline probes

#include "d/ext_seq/ja1_jaschannel.h"            // TDSPQueue::checkQueue (donor JASChAllocQueue.h)
#include "JSystem/JAudio2/JASDSPChannel.h"       // ===== §369: JA2 allocator fence (setJa1ReservedRange)
#include "JSystem/JAudio2/JASDSPInterface.h"     // ===== §369: JASDsp::getDSPHandle / releaseHalt (the PC voice array)
#include "dolphin/os.h"

// §369: donor DSP-overload kill heuristic — OFF per receiver precedent
// (see TU header). Flip to 1 only with the receiver's own JA2 block.
#ifndef JA1_DSP_OVERLOAD_KILL
#define JA1_DSP_OVERLOAD_KILL 0
#endif

namespace JAudio1 {

// ============================================================
// §369 DSPInterface::getDSPHandle — donor JASDSPInterface.cpp:28.
// The donor returned &sDspBuffer[param_1] (its 0x180-register array);
// the PC realization returns the DSPBuffer wrapper whose mPcVoice was
// attached to slice voice param_1 by TDSPChannel::initAll.
// ============================================================
namespace DSPInterface {

static DSPBuffer sDspBuffers[kJa1DspVoiceCount];

/* 8028A130-8028A144       .text getDSPHandle__Q28JASystem12DSPInterfaceFUc */
DSPBuffer* getDSPHandle(u8 param_1) {
    return &sDspBuffers[param_1];
}

}  // namespace DSPInterface

// donor JASDSPChannel.h globals (see ja1_dsp_boundary.h)
// KIT-DONOR-DATA: 40 lookup-table JSystem/JAudio/JASDSPChannel.cpp globals
u32 history[10] = { 1000000, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
u32 old_time;
f32 DSP_LIMIT_RATIO = 1.1f;

/* 8028963C-80289694       .text init__Q28JASystem11TDSPChannelFUc */
void TDSPChannel::init(u8 param_1) {
    mNumber = param_1;
    mStatus = 1;
    field_0x8 = 0;
    mCBInterval = 0;
    mCallback = NULL;
    mPriority = 0;
    mPriorityTime = 0;
    field_0xc = DSPInterface::getDSPHandle(param_1);
}

/* 80289694-80289720       .text allocate__Q28JASystem11TDSPChannelFUl */
int TDSPChannel::allocate(uintptr_t param_1) {  // ===== §369: donor u32 sign — x64 widening
    if (mStatus != 1) {
        return false;
    }
    BOOL r0 = (field_0x8 == 0) ? TRUE : FALSE;
    if (!r0) {
        OSReport("sign %x があるのにCH.%d はFREE\n", field_0x8, mNumber);
    }
    mStatus = 0;
    field_0x8 = param_1;
    mPriority = 1;
    field_0xc->allocInit();
    return true;
}

/* 80289720-8028973C       .text free__Q28JASystem11TDSPChannelFv */
void TDSPChannel::free() {
    mStatus = 1;
    mPriority = 0;
    mCallback = NULL;
    field_0x8 = 0;
}

TDSPChannel* TDSPChannel::DSPCH;
int TDSPChannel::smnUse;

/* 8028973C-802897E0       .text forceStop__Q28JASystem11TDSPChannelFv */
bool TDSPChannel::forceStop() {
    if (mStatus == 2) {
        return false;
    }
    if (mStatus == 1) {
        OSReport("----- JASDSPChannel::forceStop Warning! : CH_FREE\n");
        return false;
    }
    if (field_0xc->mPcVoice->mIsActive == 0) {  // ===== §369: donor field_0xc->field_0x0
        return false;
    }
    smnUse--;
    field_0xc->mPcVoice->mForcedStop = 1;  // ===== §369: donor field_0xc->field_0x10a
    field_0xc->flushChannel();
    mStatus = 2;
    return true;
}

/* 802897E0-802897F4       .text forceDelete__Q28JASystem11TDSPChannelFv */
void TDSPChannel::forceDelete() {
    field_0x8 = 0;
    mPriority = 0;
    mCallback = NULL;
}

/* 802897F4-80289844       .text play__Q28JASystem11TDSPChannelFv */
void TDSPChannel::play() {
#if TARGET_PC
    // §373d pipeline probe (dsp-voice stage) — strip at A5 acceptance.
    { static int n = 0; if (n < 4) { ++n; DuskLog.info("[JA1] §373d TDSPChannel::play #{}", n); } }
#endif
    if (!mCallback) {
        field_0xc->mPcVoice->mSamplePosition = 0;  // ===== §369: donor field_0xc->field_0x68
    }
    field_0xc->playStart();
    field_0xc->flushChannel();
}

/* 80289844-80289874       .text stop__Q28JASystem11TDSPChannelFv */
void TDSPChannel::stop() {
    field_0xc->mPcVoice->mIsActive = 0;  // ===== §369: donor field_0xc->field_0x0
    field_0xc->flushChannel();
}

/* 80289874-80289994       .text initAll__Q28JASystem11TDSPChannelFv */
void TDSPChannel::initAll() {
    static int first = 1;
    if (first == 0) {
        OSReport("---- DSPChannel::initAll : already initialized\n");
        return;
    }
    // ===== §369: attach the reserved PC voice slice BEFORE the donor
    // init loop (the DSPBuffer wrappers must resolve in init()), and
    // fence the JA2 allocator off it (data-driven; gate-OFF the range
    // stays empty because initAll is only reached from gate-ON init).
    for (u32 i = 0; i < kJa1DspVoiceCount; i++) {
        DSPInterface::sDspBuffers[i].mPcVoice = JASDsp::getDSPHandle((int)(kJa1DspVoiceFirst + i));
    }
    JASDSPChannel::setJa1ReservedRange(kJa1DspVoiceFirst, kJa1DspVoiceCount);
    DSPCH = new TDSPChannel[kJa1DspVoiceCount];  // ===== §369: donor `new (JASDram, 0x20) TDSPChannel[64]`
    JUT_ASSERT(204, DSPCH);
    OSReport("----- JASDSPChannel size : %d\n", 0x14);
    for (u8 i = 0; i < kJa1DspVoiceCount; i++) {  // ===== §369: donor 64
        DSPCH[i].init(i);
    }
    first = 0;
}

int TDSPChannel::smnFree = kJa1DspVoiceCount;  // ===== §369: donor 64

/* 80289994-80289A54       .text alloc__Q28JASystem11TDSPChannelFUlUl */
TDSPChannel* TDSPChannel::alloc(u32 param_1, uintptr_t param_2) {  // ===== §368: donor u32 sign — x64 widening
    if (param_1) {
        OSReport("----- JASDSPChannel::alloc : 多チャネルモードはサポートされていません\n");
        return NULL;
    }
    int i = 0;
    do {
        BOOL r0 = DSPCH[i].getStatus() == 1 ? TRUE : FALSE;
        if (r0 && DSPCH[i].allocate(param_2)) {
            smnFree--;
            smnUse++;
            return &DSPCH[i];
        }
        i++;
    } while(i < (int)kJa1DspVoiceCount);  // ===== §369: donor 64
    return NULL;
}

/* 80289A54-80289AF4       .text free__Q28JASystem11TDSPChannelFPQ28JASystem11TDSPChannelUl */
int TDSPChannel::free(TDSPChannel* dspch, uintptr_t param_2) {  // ===== §368: donor u32 sign — x64 widening
    if (!dspch) {
        OSReport("----- JASDSPChannel::free : NULL のチャネルを解放しようとしました\n");
        return -1;
    }
    if (dspch->field_0x8 != param_2) {
        OSReport("----- JASDSPChannel::free : BAD USERが開放を試みた\n");
        return -2;
    }
    if (dspch->getStatus() == 0) {
        smnUse--;
    }
    if (dspch->getStatus() != 1) {
        smnFree++;
    }
    dspch->free();
    return 0;
}

/* 80289AF4-80289C0C       .text getLower__Q28JASystem11TDSPChannelFv */
TDSPChannel* TDSPChannel::getLower() {
    u8 r31 = 0xff;
    u8 r30 = 0;
    u32 r29 = 0;
    for (u8 i = 0; i < kJa1DspVoiceCount; i++) {  // ===== §369: donor 64
        u8 r27;
        TDSPChannel* dspch = &DSPCH[i];
        if (dspch->getStatus() == 2) {
            continue;
        }
        if (dspch->getStatus() == 1) {
            r30 = i;
            break;
        }
        if (dspch->mCallback) {
            r27 = dspch->getPriority();
            if (r27 <= r31) {
                JUT_ASSERT(305, i == dspch->getNumber());
                if (r27 != r31 || (u32)dspch->field_0xc->mPcVoice->field_0x10c >= r29) {  // ===== §369: donor field_0x10c
                    r29 = dspch->field_0xc->mPcVoice->field_0x10c;
                    r30 = i;
                    r31 = r27;
                }
            }
        }
    }
    return &DSPCH[r30];
}

/* 80289C0C-80289D10       .text getLowerActive__Q28JASystem11TDSPChannelFv */
TDSPChannel* TDSPChannel::getLowerActive() {
    /* Nonmatching - regalloc */
    u8 i;
    u32 r27;
    u8 r29;
    u8 r28;
    u8 r30;
    r29 = 0xff;
    r28 = 0;
    r27 = 0;
    for (i = 0; i < kJa1DspVoiceCount; i++) {  // ===== §369: donor 64
        TDSPChannel* dspch = &DSPCH[i];
        if (dspch->getStatus() == 2 || dspch->getStatus() == 1) {
            continue;
        }
        r30 = dspch->getPriority();
        if (r30 <= r29) {
            JUT_ASSERT(345, i == dspch->getNumber());
            if (r30 != r29 || (u32)dspch->field_0xc->mPcVoice->field_0x10c >= r27) {  // ===== §369: donor field_0x10c
                r27 = dspch->field_0xc->mPcVoice->field_0x10c;
                r28 = i;
                r29 = r30;
            }
        }
    }
    return &DSPCH[r28];
}

/* 80289D10-80289DC8       .text breakLower__Q28JASystem11TDSPChannelFUc */
BOOL TDSPChannel::breakLower(u8 param_1) {
    TDSPChannel* dspch = getLower();
    if (dspch->getPriority() > param_1) {
        return false;
    }
    if (dspch->getStatus() != 1) {
        if (dspch->mCallback) {
            dspch->onUpdate(3);
        } else {
            OSReport("----- BreakLowerDSPchannel : Error:Callback is NULL\n");
        }
        dspch->forceStop();
    } else {
        OSReport("----- BreakLowerDSPchannel : DSP Ch is FREE %d \n", dspch->mNumber);
        return false;
    }
    return true;
}

/* 80289DC8-80289E68       .text breakLowerActive__Q28JASystem11TDSPChannelFUc */
bool TDSPChannel::breakLowerActive(u8 param_1) {
    TDSPChannel* dspch = getLowerActive();
    if (dspch->getPriority() > param_1) {
        return false;
    }
    if (dspch->getStatus() != 1) {
        if (dspch->mCallback) {
            dspch->onUpdate(3);
        }
        dspch->forceStop();
    } else {
        OSReport("----- BreakLowerActiveDSPchannel : DSP Ch is FREE %d\n", dspch->mNumber);
        return false;
    }
    return true;
}

/* 80289E68-8028A04C       .text updateAll__Q28JASystem11TDSPChannelFv */
void TDSPChannel::updateAll() {
    /* Nonmatching - instruction ordering, maybe inline related */
    DSPInterface::DSPBuffer* dspBuffer;
#if JA1_DSP_OVERLOAD_KILL
    // ===== §369: donor overload-kill heuristic — compiled out per the
    // receiver's own PC precedent (see TU header). Donor body kept for
    // the record; requires an OSGetTick + audio-thread sync-count
    // source on PC before it could ever be trusted.
    if (Kernel::getSubFrames() <= 10) {
        u32 time = OSGetTick();
        u32 var2;
        u32 var3 = time - old_time;
        old_time = time;
        var2 = Kernel::getSubFrames() - JASAudioThread::snIntCount;
        history[var2] = var3;
        if (var2) {
            if (f32(history[0]) / var3 < DSP_LIMIT_RATIO) {
                breakLowerActive(126);
            }
        }
    }
#endif
    TDSPQueue::checkQueue();
    for (u32 i = 0; i < kJa1DspVoiceCount; i++) {  // ===== §369: donor 64
        if ((i & 0x0f) == 0 && i != 0) {
            JASDsp::releaseHalt((i - 1) >> 4);  // ===== §369: donor DSPReleaseHalt2 (PC no-op, receiver's own realization)
        }
        dspBuffer = DSPCH[i].field_0xc;
        TDSPChannel* dspChannel = &DSPCH[i];
        if (dspChannel->getStatus() == 1) {
            continue;
        }
        if (dspBuffer->mPcVoice->mIsFinished != 0) {  // ===== §369: donor dspBuffer->field_0x2
            if (dspChannel->mCallback) {
                dspChannel->onUpdate(2);
            }
            dspBuffer->mPcVoice->mIsFinished = 0;  // ===== §369: donor field_0x2
            dspBuffer->mPcVoice->mIsActive = 0;    // ===== §369: donor field_0x0
            dspBuffer->flushChannel();
            if (dspChannel->getStatus() == 1) {
                continue;
            }
        }
        if (dspBuffer->mPcVoice->mForcedStop == 0) {  // ===== §369: donor field_0x10a
            dspBuffer->mPcVoice->field_0x10c++;
            if (dspBuffer->mPcVoice->field_0x10c == dspChannel->getPriorityTime() && dspChannel->mCallback) {
                dspChannel->mCallback(dspChannel, 4);
            }
        }
        if (dspChannel->mCallback) {
            if (dspChannel->getCBInterval() != 0) {
                dspChannel->decCBInterval();
            }
            if (dspChannel->getCBInterval() == 0) {
                dspChannel->onUpdate(0);
                if (dspChannel->getCBInterval() == 0) {
                    dspBuffer->mPcVoice->mIsFinished = 0;  // ===== §369: donor field_0x2
                    dspBuffer->mPcVoice->mIsActive = 0;    // ===== §369: donor field_0x0
                    dspBuffer->flushChannel();
                }
            }
        }
    }
    JASDsp::releaseHalt(3);  // ===== §369: donor DSPReleaseHalt2(3)
}

/* 8028A04C-8028A08C       .text onUpdate__Q28JASystem11TDSPChannelFUl */
void TDSPChannel::onUpdate(u32 param_1) {
    if (mCallback) {
        mCBInterval = mCallback(this, param_1);
    }
}

/* 8028A08C-8028A0C0       .text getNumBreak__Q28JASystem11TDSPChannelFv */
int TDSPChannel::getNumBreak() {
    int count = 0;
    for (int i = 0; i < (int)kJa1DspVoiceCount; i++) {  // ===== §369: donor 64
        if (DSPCH[i].getStatus() == 2) {
            count++;
        }
    }
    return count;
}

}  // namespace JAudio1
