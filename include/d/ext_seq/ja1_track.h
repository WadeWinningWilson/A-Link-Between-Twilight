#ifndef D_EXT_SEQ_JA1_TRACK_H
#define D_EXT_SEQ_JA1_TRACK_H

#include "d/ext_seq/ja1_seq_ctrl.h"

class JASChannel;

namespace ExtSeq {

class Ja1Parser;

/**
 * Slim JA1 track host for ExtSeqSpace.
 * Owns JASChannel* voices it starts (№89 — stopOwned releases them).
 */
class Ja1Track {
public:
    static constexpr int kMaxChildren = 16;
    static constexpr int kMaxVoices = 8;

    void init();
    void close();
    void start(u8* base, u32 offset);
    Ja1Track* openChild(u8 idx, u8 /*flags*/);
    Ja1Track* getChild(u8 idx) { return (idx < kMaxChildren) ? mChildren[idx] : nullptr; }
    Ja1Track* getParent() { return mParent; }
    Ja1SeqCtrl* getSeq() { return &mSeq; }

    int noteOn(u8 voice, s32 key, s32 vel, s32 /*gate*/, u32 /*prio*/);
    bool noteOff(u8 voice, u16 /*release*/);
    void noteOffAll();
    void gateOn(u8 /*voice*/, s32 /*key*/, s32 /*vel*/, s32 /*time*/) {}

    void writeRegParam(u8 param);
    void writeRegDirect(u8 reg, u16 value);
    u16 readReg16(u8 reg);
    u32 readReg32(u8 reg);
    u32 exchangeRegisterValue(u8 reg);

    int getTranspose() const;
    void setTranspose(s32 t) { mTranspose = static_cast<s8>(t); }
    void setTempo(u16 t);
    void setTimebase(u16 t);
    /** Pull children onto this track's tempo/timebase (WW updateTempo). */
    void updateTempo();
    /** Sequence-wide: walk to root (WW timebase is not independently per-track). */
    u16 getTempo() const;
    u16 getTimebase() const;

    u16 readSelfPort(u32) { return 0; }
    void writeSelfPort(u32, u16) {}
    u16 checkImport(u32) { return 0; }
    u16 checkExport(u32) { return 0; }

    void setParam(u8 target, f32 value, int moveTime);
    void updateTimedParam();
    void applyVoiceParams();
    /** applyVoiceParams on this track and every active child (bus fade). */
    void applyVoiceParamsDeep();
    f32 timedCurrent(u8 target) const {
        return (target < TIMED_PARAMS) ? mTimed[target].mCurrent : 0.0f;
    }
    f32 timedTarget(u8 target) const {
        return (target < TIMED_PARAMS) ? mTimed[target].mTarget : 0.0f;
    }
    u8 volumeMode() const { return mVolumeMode; }
    int activeVoiceCount() const;
    /**
     * §C.1 absolute-level probe — dump/log only. Walks this track + active
     * children: raw timed vol, composedVolume (mode² × parent), voice count.
     */
    void logVolProbeTree(const char* tag) const;
    f32 composedVolume() const;
    f32 composedPan() const;
    f32 composedFxmix() const;
    f32 composedDolby() const;

    /** Reset setParam probe budget + log tree after BMS start (no-op if env off). */
    static void volProbeOnSeqStart(Ja1Track* root);

    void oscSetupSimple(u32) {}
    void oscSetupSimpleEnv(u32, u32) {}
    void updateTrack(u32) {}
    void connectBus(u32, u32) {}
    void setPauseStatus(u32) {}
    void setVolumeMode(u32 mode) { mVolumeMode = static_cast<u8>(mode); }
    void flushAll() { noteOffAll(); }
    void tryInterrupt() {}
    void setPanPower(int, f32) {}
    void assignExtBuffer(void*) {}
    void oscSetupFull(u32, u32, u32) {}
    void setOscRoute(u8, u8) {}
    s32 seqTimeToDspTime(s32 t, u8) { return t; }

    // WW TTrack timed-param targets (JASTrack.h).
    enum TimedTarget : u8 {
        TIMED_Volume = 0,
        TIMED_Pitch = 1,
        TIMED_Fxmix = 2,
        TIMED_Pan = 3,
        TIMED_Dolby = 4,
        TIMED_PARAMS = 18,
    };

    struct MoveParam {
        f32 mCurrent = 0.0f;
        f32 mTarget = 0.0f;
        f32 mMoveTime = 0.0f;
        f32 mMoveAmount = 0.0f;
    };

    struct NoteMgr {
        u8 lastNote = 0;
        u8 connectCase = 0;
        s32 baseTime = 0;
        u8 getLastNote() const { return lastNote; }
        void setLastNote(u8 n) { lastNote = n; }
        u8 getConnectCase() const { return connectCase; }
        void setConnectCase(u8 c) { connectCase = c; }
        void setBaseTime(s32 t) { baseTime = t; }
        void setBeforeTieMode(bool) {}
        JASChannel* getChannel(int) { return nullptr; }
    };

    struct RegParam {
        u16 r[32]{};
        u16 flag = 0;
        u16 bankProg = 0;  // hi=bank vir, lo=prog — WW field_0xc shape
        void init() {
            for (auto& v : r) {
                v = 0;
            }
            flag = 0;
            bankProg = 0x00F0;  // bank 0, prog 0xF0 (osc) default-ish
        }
        u16 getFlag() const { return flag; }
        void setFlag(u16 f) { flag = f; }
        u8 getBankNumber() const { return static_cast<u8>((bankProg >> 8) & 0xFF); }
        u8 getProgramNumber() const { return static_cast<u8>(bankProg & 0xFF); }
    };

    struct OuterParam {
        void setFirFilter(s16*) {}
        void setOuterSwitch(u16) {}
        void onSwitch(u16) {}
    };

    struct IntrMgr {
        void setIntr(u32, u8*) {}
        void resetInter(u32) {}
        void enable() {}
        void disable() {}
        void setTimer(u32, u32) {}
    };

    struct ChannelUpdater {
        void stopAllRelease() {}
    };

    Ja1SeqCtrl mSeq;
    NoteMgr mNoteMgr;
    RegParam mRegisterParam;
    OuterParam mOuter;
    IntrMgr mIntrMgr;
    ChannelUpdater mChannelUpdater;
    OuterParam* getOuterParam() { return &mOuter; }

    Ja1Track* mParent = nullptr;
    Ja1Track* mChildren[kMaxChildren]{};
    JASChannel* mVoices[kMaxVoices]{};
    MoveParam mTimed[TIMED_PARAMS]{};
    s8 mTranspose = 0;
    u16 mTempo = 120;
    u16 mTimebase = 48;
    u8 mVolumeMode = 0;  // 0 = square volume (WW default)
    u8 field_0x387 = 0;
    bool mIsPaused = false;
    u8 mPauseStatus = 0;
    bool mActive = false;
    /** WW TTrack::field_0x36c — packed hierarchy id (openTrack depth nibble). */
    u32 mTrackId = 0;
    u8 mOscRoute[4]{};
    // ADSR stubs referenced by cmdSimpleADSR (unused by i_link path typically)
    void* field_0x2cc[2]{};
    u16 field_0x304[16]{};
    u16 field_0x374 = 0;

    /** Tick one wait unit; if ready, run parser until next wait. */
    void tick(Ja1Parser& parser);

private:
    void initTimed();
};

}  // namespace ExtSeq

#endif
