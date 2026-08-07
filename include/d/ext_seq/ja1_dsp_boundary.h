#ifndef D_EXT_SEQ_JA1_DSP_BOUNDARY_H
#define D_EXT_SEQ_JA1_DSP_BOUNDARY_H

// ============================================================
// §368 JAudio1 DSP-voice boundary — THE ONE BOUNDARY of the A3 channel
// port (bus §362/§363/§366 campaign). On GC the donor TChannel
// (JASChannel.cpp) drives the DSP voice through two surfaces:
//   1. DSPInterface::DSPBuffer — the 0x180-byte DSP channel register
//      block (donor JASDSPInterface.h/.cpp): wave/pitch/mixer/filter/
//      pause register writes + cache flush.
//   2. TDSPChannel — the voice-slot lifecycle (donor JASDSPChannel.h):
//      alloc/free/play/stop/priority/callback.
// The PORT's JAudio2 realizes the SAME register block on PC as
// JASDsp::TChannel (libs/JSystem/include/JSystem/JAudio2/
// JASDSPInterface.h), consumed by the software DSP in
// src/dusk/audio/DuskDsp.cpp and mixed out through the game's PC audio
// sink. Field-for-field the JA1 DSPBuffer and the PC JASDsp::TChannel
// are the same 0x180 register layout (verified against donor
// JASDSPBuf.cpp/JASDSPInterface.cpp vs receiver JASDSPInterface.cpp).
//
// REALIZATION CHOSEN (§368):
//   - DSPBuffer ops DELEGATE to the same PC facility JA2 uses: each
//     donor op either calls the identical JASDsp::TChannel method or,
//     where JA1's op shape has no JA2 counterpart (the 3-arg mixer
//     writes carrying the volume-ramp delay byte, setMixerVolumeOnly,
//     setMixerInitDelayMax, JA1's setAutoMixer fxmix packing), performs
//     the donor-verbatim register write on the same JASDsp::TChannel
//     block. No DSP behavior is invented — every write is the donor
//     JASDSPInterface.cpp write, landing in the registers DuskDsp reads.
//   - TDSPChannel lifecycle is REAL after A4 (§369): the donor
//     JASDSPChannel.cpp machinery (DSPCH array + alloc/free/play/stop/
//     breakLower + the per-subframe updateAll ager) runs verbatim in
//     ja1_jasdspchannel.cpp against a RESERVED SLICE of the SAME
//     JASDsp voice array DuskDsp mixes (voices [kJa1DspVoiceFirst,
//     kJa1DspVoiceFirst+kJa1DspVoiceCount)). The JA2 allocator is
//     fenced off that slice (JASDSPChannel::setJa1ReservedRange —
//     data-driven, count 0 gate-OFF = JA2 untouched). WHY a reserved
//     slice and not the JA2 allocator interface: the two lifecycle
//     protocols are incompatible (JA1: int(*)(TDSPChannel*, u32) with
//     CB-interval aging + priority-time aging on raw voice registers;
//     JA2: CB_START/CB_PLAY/CB_STOP/CB_DROP driven by JASDSPChannel::
//     updateProc which issues its own playStart/flush) — sharing per
//     voice would mean inventing a shim state machine, rejected per
//     the §368 rationale. The reserved slice keeps BOTH donor state
//     machines donor-pure over disjoint voices of one mixer.
// Gate-OFF (DUSK_JA1_NATIVE=0): initAll is never called, the reserved
// range stays empty, the JA2 pool and the bridge are untouched.
// ============================================================

#include "dolphin/types.h"
#include "d/ext_seq/ja1_boundary.h"              // JA1_BOUNDARY_STUB + JUT shims
#include "JSystem/JAudio2/JASDSPInterface.h"     // ===== §368: JASDsp::TChannel — the PC-realized DSP voice registers
#include "JSystem/JAudio2/JASWaveInfo.h"         // ===== §368: JASWaveInfo — PC shape of the wave-info block

namespace JAudio1 {
// §374d (strip at A5): first-N mixer-write value probe, defined in ja1_kernel.cpp.
void ja1DspProbe374d(const char* what, int a, int b, int c);

class TChannel;

// ============================================================
// §368 namespace Driver — donor JASDriverIF.h + JASDSPInterface.h
// fragments the channel layer consumes. PanMatrix_/Wave_/Clamp01 are
// donor-verbatim; the config getters are A4 stubs returning the donor
// JASDriverIF.cpp retail static initializers.
// ============================================================
namespace Driver {
    // donor JASDriverIF.h:8 (verbatim)
    struct PanMatrix_ {
        /* 0x00 */ f32 mSound;
        /* 0x04 */ f32 mEffect;
        /* 0x08 */ f32 mChannel;
    };

    // donor JASDSPInterface.h Driver::Wave_ — SAME block as donor
    // JASWaveBank.h TWaveInfo (the two donor decls alias one layout;
    // merged here with the informative TWaveInfo field names kept as
    // comments — §368 named adaptation "merged Wave_/TWaveInfo decl").
    struct Wave_ {
        /* 0x00 */ u8 field_0x0;      // mBlockType
        /* 0x01 */ u8 field_0x1;      // wave format (COMP_BLOCK* index)
        /* 0x02 */ u8 field_0x2;      // base key
        /* 0x03 */ u8 field_0x3;
        /* 0x04 */ f32 field_0x4;     // sample rate
        /* 0x08 */ int field_0x8;     // mWavePtrOffs
        /* 0x0C */ int field_0xc;
        /* 0x10 */ int field_0x10;    // loop flag
        /* 0x14 */ int field_0x14;    // loop start sample
        /* 0x18 */ int field_0x18;    // loop end sample
        /* 0x1C */ int field_0x1c;    // sample count
        /* 0x20 */ s16 field_0x20;    // mpLast
        /* 0x22 */ s16 field_0x22;    // mpPenult
        /* 0x24 */ u32* field_0x24;   // wave-loaded ptr table (checked [0]!=0)
        /* 0x28 */ int field_0x28;
    };

    // donor JASDriverIF.h:22 (verbatim)
    inline f32 Clamp01(f32 value) {
        if (value <= 0.0f) {
            return 0.0f;
        }
        if (value >= 1.0f) {
            return 1.0f;
        }
        return value;
    }

    // donor JASChannel.cpp data (defined in ja1_jaschannel.cpp)
    extern u8 calc_sw_table[27][3];
    // donor JASDriverTables.cpp data (defined in ja1_jaschannel.cpp)
    extern f32 C5BASE_PITCHTABLE[128];
    // donor JASChannelMgr.cpp rodata (defined in ja1_jaschannel.cpp)
    extern u8 polys_table[16];

    // ===== §369: donor JASDriverIF.cpp rows are REAL (verbatim
    // variables + getters in ja1_kernel.cpp; retail inits kept:
    // JAS_SYSTEM_OUTPUT_MODE=1 stereo, JAS_UPDATE_INTERVAL=1,
    // MAX_MIXERLEVEL=0x2ee0, MAX_AUTOMIXERLEVEL=0x7fff). The values
    // are identical to the A3 stub returns — zero behavioral delta,
    // the log-once diagnostics simply retire.
    extern u32 JAS_SYSTEM_OUTPUT_MODE;
    extern u8 JAS_UPDATE_INTERVAL;
    extern u16 MAX_MIXERLEVEL;
    extern u16 MAX_AUTOMIXERLEVEL;
    u32 getOutputMode();
    void setOutputMode(u32);
    u8 getUpdateInterval();
    u16 getChannelLevel();
    u16 getAutoLevel();
}  // namespace Driver

// ============================================================
// §369 A4 — the JA1 voice slice of the PC JASDsp pool (see file
// header, "reserved slice" rationale). Donor pool size was 64 voices
// carrying ALL WW audio; on the receiver the ONE mixer pool is shared
// with the live JA2 stack (TP SE keep playing on WW host stages), so
// gate-ON splits it: JA2 keeps [0,32), JA1 owns [32,64).
// ============================================================
constexpr u32 kJa1DspVoiceFirst = 32;
constexpr u32 kJa1DspVoiceCount = 32;

// ============================================================
// §368 DSPInterface::DSPBuffer — the donor register-write surface,
// realized on the PC JASDsp::TChannel voice. Method-by-method
// realization (D = delegates to the identical JA2 method,
// R = donor-verbatim register write on the same block):
//   setWaveInfo        D (JA1 Wave_ fields mapped onto JASWaveInfo —
//                         both bodies verified line-identical)
//   setOscInfo         D          setPitch            D
//   setPauseFlag       D          flushChannel        D (-> flush())
//   setFilterMode      D          setFIR8FilterParam  D
//   setDistFilter      D          initAutoMixer       D
//   setBusConnect      D (JA2 carries the AVOID_UB 0xFF guard the GC
//                         donor hit as harmless UB in dolby mode)
//   setIIRFilterParam  R via JASDsp::setFilterTable(iir, p, 4) — the
//                         donor writes 4 taps, JA2's method writes 8
//   setMixerVolume     R (JA2 2-arg drops the donor ramp-delay byte)
//   setMixerVolumeOnly R          setMixerInitVolume  R
//   setMixerInitDelayMax R (register 0x0E; JA2 names it _unused3)
//   setAutoMixer       R (JA1 packs fxmix<<8; JA2 packs fxmix<<8|fxmix<<1)
// mPcVoice is attached by A4 (voice slot arbitration); until then no
// DSPBuffer instance exists because TDSPChannel::alloc stubs to NULL.
// ============================================================
namespace DSPInterface {

class DSPBuffer {
public:
    DSPBuffer() : mPcVoice(NULL) {}

    // donor JASDSPInterface.cpp:126 — donor-verbatim register writes on
    // the PC block (== PC TChannel::init() + flush; the one extra PC
    // register init() zeroes, mSamplePosition, is donor-zeroed by
    // setWaveInfo/play before any playback — §369 note, no delta).
    void allocInit() {
        mPcVoice->mPauseFlag       = 0;  // donor field_0xc
        mPcVoice->mIsFinished      = 0;  // donor field_0x2
        mPcVoice->mForcedStop      = 0;  // donor field_0x10a
        mPcVoice->mIsActive        = 0;  // donor field_0x0
        mPcVoice->mAutoMixerBeenSet = 0; // donor field_0x58
        mPcVoice->initFilter();
        mPcVoice->flush();
    }
    // donor JASDSPInterface.cpp:137 — delegated (identical body)
    void playStart() { mPcVoice->playStart(); }

    // donor JASDSPInterface.cpp:156 — delegated, field-mapped
    void setWaveInfo(Driver::Wave_* param_1, u32 param_2, u32 param_3) {
        JASWaveInfo info;
        info.mWaveFormat      = param_1->field_0x1;
        info.mBaseKey         = param_1->field_0x2;
        info.mLoopFlag        = (u8)param_1->field_0x10;
        info.mSampleRate      = param_1->field_0x4;
        info.mLoopStartSample = param_1->field_0x14;
        info.mLoopEndSample   = param_1->field_0x18;
        info.mSampleCount     = param_1->field_0x1c;
        info.mpLast           = param_1->field_0x20;
        info.mpPenult         = param_1->field_0x22;
        mPcVoice->setWaveInfo(info, param_2, param_3);
    }
    // donor JASDSPInterface.cpp:205 — delegated (identical body)
    void setOscInfo(u32 param_1) { mPcVoice->setOscInfo(param_1); }
    // donor JASDSPInterface.cpp:212 — delegated (identical body)
    void initAutoMixer() { mPcVoice->initAutoMixer(); }
    // donor JASDSPInterface.cpp:222 — donor-verbatim register write
    // (JA2's setAutoMixer packs fxmix<<8|fxmix<<1; donor packs fxmix<<8)
    void setAutoMixer(u16 param_1, u8 param_2, u8 param_3, u8 param_4, u8 param_5) {
        ja1DspProbe374d("autoMix", param_1, param_2, param_4);  // §374d strip at A5
        mPcVoice->mAutoMixerPanDolby = (u16)(param_2 << 8 | param_3);
        mPcVoice->mAutoMixerFxMix    = (u16)(param_4 << 8);
        mPcVoice->mAutoMixerVolume   = param_1;
        mPcVoice->mAutoMixerBeenSet  = 1;
        (void)param_5;
    }
    // donor JASDSPInterface.cpp:230 — delegated (identical body)
    void setPitch(u16 param_1) { mPcVoice->setPitch(param_1); }
    // donor JASDSPInterface.cpp:238 — donor-verbatim register write
    // (register 0x0E, mixer ramp delay max; JA2 names it _unused3)
    void setMixerInitDelayMax(u8 param_1) { mPcVoice->_unused3 = param_1; }
    // donor JASDSPInterface.cpp:243 — donor-verbatim register write
    // (JA2's 2-arg setMixerInitVolume zeroes the ramp-delay byte)
    void setMixerInitVolume(u8 param_1, s16 param_2, u8 param_3) {
        ja1DspProbe374d("mixInit", param_1, param_2, param_3);  // §374d strip at A5
        JASDsp::OutputChannelConfig& cfg = mPcVoice->mOutputChannels[param_1];
        cfg.mCurrentVolume  = param_2;
        cfg.mTargetVolume   = param_2;
        cfg.mVolumeProgress = (u16)(param_3 << 8 | param_3);
    }
    // donor JASDSPInterface.cpp:251 — donor-verbatim register write
    void setMixerVolume(u8 param_1, s16 param_2, u8 param_3) {
        ja1DspProbe374d("mixVol", param_1, param_2, param_3);  // §374d strip at A5
        if (mPcVoice->mForcedStop) {
            return;
        }
        JASDsp::OutputChannelConfig& cfg = mPcVoice->mOutputChannels[param_1];
        cfg.mTargetVolume   = param_2;
        cfg.mVolumeProgress = (u16)(param_3 << 8 | (cfg.mVolumeProgress & 0xff));
    }
    // donor JASDSPInterface.cpp:261 — donor-verbatim register write
    void setMixerVolumeOnly(u8 param_1, s16 param_2) {
        if (mPcVoice->mForcedStop) {
            return;
        }
        mPcVoice->mOutputChannels[param_1].mTargetVolume = param_2;
    }
    // donor JASDSPInterface.cpp:269 — delegated (identical body)
    void setPauseFlag(u8 param_1) { mPcVoice->setPauseFlag(param_1); }
    // donor JASDSPInterface.cpp:274 — delegated (flushChannel -> flush)
    void flushChannel() { mPcVoice->flush(); }
    // donor JASDSPInterface.cpp:293 — delegated (identical body)
    void setFilterMode(u16 param_1) { mPcVoice->setFilterMode(param_1); }
    // donor JASDSPInterface.cpp:309 — donor writes 4 IIR taps (JA2's
    // method writes 8); routed through the same PC setFilterTable
    void setIIRFilterParam(s16* param_1) {
        JASDsp::setFilterTable(mPcVoice->iir_filter_params, param_1, 4);
    }
    // donor JASDSPInterface.cpp:314 — delegated (identical body)
    void setFIR8FilterParam(s16* param_1) { mPcVoice->setFIR8FilterParam(param_1); }
    // donor JASDSPInterface.cpp:319 — delegated (JA2 writes iir[4] =
    // the same register 0x150 the donor writes as field_0x150)
    void setDistFilter(s16 param_1) { mPcVoice->setDistFilter(param_1); }
    // donor JASDSPInterface.cpp:324 — delegated (identical body incl.
    // connect_table; JA2 adds the AVOID_UB guard for the 0xFF dolby case)
    void setBusConnect(u8 param_1, u8 param_2) {
        ja1DspProbe374d("busCon", param_1, param_2, 0);  // §374d strip at A5
        mPcVoice->setBusConnect(param_1, param_2);
    }

    // ===== §368: the PC voice this donor-shaped register surface
    // writes through. Attached by A4 when the JA1 voice allocator gets
    // its slice of the JASDsp voice pool.
    JASDsp::TChannel* mPcVoice;
};

// ===== §369: donor JASDSPInterface.cpp:28 getDSPHandle — returns the
// DSPBuffer wrapping JA1 slice voice param_1 (attached at
// TDSPChannel::initAll; body in ja1_jasdspchannel.cpp).
DSPBuffer* getDSPHandle(u8);

}  // namespace DSPInterface

// ============================================================
// §368/§369 TDSPChannel — donor JASDSPChannel.h voice-slot lifecycle,
// REAL after A4: donor JASDSPChannel.cpp bodies verbatim in
// ja1_jasdspchannel.cpp against the reserved JASDsp voice slice (see
// file header for the pool policy; DSPCH array size = the slice count,
// each donor literal 64 adapted to kJa1DspVoiceCount — §369).
// §368 x64 named adaptation kept: the donor "sign" field field_0x8 is
// a u32 holding a TChannel*; widened to uintptr_t.
// ============================================================
class TDSPChannel {
public:
    TDSPChannel() {
        field_0xc = NULL;
        mCallback = NULL;
    }
    ~TDSPChannel() {}

    // donor JASDSPChannel.cpp — real bodies in ja1_jasdspchannel.cpp
    void init(u8);
    int allocate(uintptr_t);  // ===== §368/§369: donor u32 sign — x64 widening
    void free();
    bool forceStop();
    void forceDelete();
    void play();
    void stop();
    void onUpdate(u32);
    static void initAll();
    static TDSPChannel* alloc(u32, uintptr_t);  // ===== §368: donor u32 sign — x64 widening
    static int free(TDSPChannel*, uintptr_t);   // ===== §368: donor u32 sign — x64 widening
    static TDSPChannel* getLower();
    static TDSPChannel* getLowerActive();
    static BOOL breakLower(u8);
    static bool breakLowerActive(u8);
    static void updateAll();
    static int getNumBreak();

    // donor JASDSPChannel.h inlines (verbatim — plain field state)
    u8 getNumber() { return mNumber; }
    u8 getStatus() { return mStatus; }
    u8 getPriority() { return mPriority; }
    void setPriority(u8 priority) { mPriority = priority; }
    u16 getPriorityTime() { return mPriorityTime; }
    void setPriorityTime(u16 time) { mPriorityTime = time; }
    u16 getCBInterval() { return mCBInterval; }
    void setCBInterval(u16 interval) { mCBInterval = interval; }
    void decCBInterval() { mCBInterval--; }
    TChannel* getLogicalChannel() {
        if (mCallback != NULL) {
            return (TChannel*)field_0x8;  // donor: the alloc "sign" is the logical channel
        } else {
            return NULL;
        }
    }

    /* 0x00 */ u8 mNumber;
    /* 0x01 */ u8 mStatus;
    /* 0x02 */ u8 field_0x2;
    /* 0x03 */ u8 mPriority;
    /* 0x04 */ u16 mPriorityTime;
    /* 0x06 */ u16 mCBInterval;
    /* 0x08 */ uintptr_t field_0x8;  // ===== §368: donor u32 "sign" (holds a TChannel*) — x64 widening
    /* 0x0C */ DSPInterface::DSPBuffer* field_0xc;
    /* 0x10 */ int (*mCallback)(TDSPChannel*, u32);

    // donor JASDSPChannel.h statics (defined in ja1_jasdspchannel.cpp)
    static int smnFree;
    static TDSPChannel* DSPCH;
    static int smnUse;
};

// donor JASDSPChannel.h globals — donor names kept inside JAudio1
// (defined in ja1_jasdspchannel.cpp; consumed only by the donor
// DSP-overload kill heuristic, compiled out on PC per the receiver's
// own precedent — see TU).
extern u32 history[10];  // ===== §369: donor OSTick -> u32 (same width; PC has no OSTick currency here)
extern u32 old_time;
extern f32 DSP_LIMIT_RATIO;

}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_DSP_BOUNDARY_H */
