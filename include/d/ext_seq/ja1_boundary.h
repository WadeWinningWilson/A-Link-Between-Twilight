#ifndef D_EXT_SEQ_JA1_BOUNDARY_H
#define D_EXT_SEQ_JA1_BOUNDARY_H

// ============================================================
// §367/§368 JAudio1 boundary layer — carries, per donor symbol the
// ported JA1 TUs reference but whose owning donor TU is NOT ported yet,
// either a donor-verbatim layout (embedded-by-value classes) or a
// declaration + log-once stub body (donor-neutral return, ZERO invented
// behavior). Nothing in the game calls the JA1 stack yet.
// BOUNDARY TABLE STATE after A4.5 (§370): NOTHING REMAINS STUBBED —
// every row is REAL; this header now carries only donor-verbatim
// layouts/decls + the §367 shims.
//   REAL after A3 (§368 — stubs retired):
//     TChannel (A2 SILHOUETTE deleted — full donor class in
//       ja1_jaschannel.h; bodies ja1_jaschannel.cpp)
//     TChannelMgr / TGlobalChannel / TDSPQueue -> ja1_jaschannel.{h,cpp}
//     BankMgr (noteOn/noteOnOsc/gateOn/getPhysicalNumber/...) +
//       TBank/TInst/TBasicInst/TBasicBank/TInstEffect/TInstRand/
//       TInstSense/TDrumSet/BNKParser -> ja1_jasbank.{h,cpp} +
//       ja1_jasbnkparser.cpp
//   REAL after A4.5 (§370 — the last A2-support stub fences retired):
//     TSeqCtrl bodies        -> ja1_jasseqctrl.cpp   (JASSeqCtrl.cpp)
//     TTrackPort bodies      -> ja1_jastrackport.cpp (JASTrackPort.cpp)
//     TIntrMgr bodies        -> ja1_jastrackinterrupt.cpp
//                                                    (JASTrackInterrupt.cpp)
//     TRegisterParam bodies  -> ja1_jasregisterparam.cpp
//                                                    (JASRegisterParam.cpp)
//     TTrack::TNoteMgr       -> ja1_jasnotemgr.cpp   (JASNoteMgr.cpp)
//     TTrack::TOuterParam    -> ja1_jasouterparam.cpp (JASOuterParam.cpp)
//       (their ja1_jastrack.h fences removed)
//     Calc:: real bodies     -> ja1_jascalc.cpp      (JASCalc.cpp —
//       table-driven sinfT/sinfDolby2 built by initSinfT at gate-ON
//       boot, donor Kernel::init leg JASAiCtrl.cpp:31; plus imixcopy/
//       bcopyfast/bzerofast donor family)
//   REAL after A4 (§369 — stubs retired):
//     Kernel:: JASRate rows + the JASCallback.cpp subframe-callback
//       registry (verbatim bodies in ja1_kernel.cpp; heartbeat pumped
//       from the PC audio pump per DacRate/80 subframe)
//     Player:: extend8to16/pitchToCent/getRandomS32 + s_key_table +
//       CUTOFF_TO_IIR_TABLE real data (ja1_jasplayer.cpp, donor
//       JASPlayer_impl.cpp)
//     Driver:: getters -> real donor JASDriverIF.cpp rows
//       (ja1_kernel.cpp; decls in ja1_dsp_boundary.h)
//     TDSPChannel lifecycle -> real donor JASDSPChannel.cpp against a
//       reserved slice of the PC JASDsp voice pool
//       (ja1_jasdspchannel.cpp; decls in ja1_dsp_boundary.h)
//     WaveBankMgr::getWaveBank -> ExtSeq-backed TWaveBank over the
//       bridge-loaded arcs (ja1_bank.cpp; decl in ja1_jasbank.h)
// Namespace convention follows the §363 oscillator port: donor
// JASystem::* flattens into JAudio1::* with donor symbol names kept.
// ============================================================

#include "dolphin/types.h"
#include "dolphin/os.h"                 // OSReport for stub logging + donor OSReport call sites
#include "JSystem/JAudio2/JASAiCtrl.h"  // ===== §367: JASDriver::getDacRate (Kernel::getDacRate shim, §363 precedent)
#include "d/ext_seq/ja1_oscillator.h"   // §363 JAudio1::TOscillator (Osc_ used in TChannel signatures)

// ============================================================
// §367 BOUNDARY stub logging — fires once per stub site, names the
// owning phase. Stubs must never fabricate behavior; they log and
// return donor-neutral values only.
// §370 NOTE: after A4.5 NO active stub site remains anywhere in the
// JA1 port; the macro is retained for any future boundary row.
// ============================================================
#define JA1_BOUNDARY_STUB(sym, phase)                                          \
    do {                                                                       \
        static bool s_ja1BoundaryLogged = false;                               \
        if (!s_ja1BoundaryLogged) {                                            \
            s_ja1BoundaryLogged = true;                                        \
            OSReport("JA1 \xC2\xA7""367 BOUNDARY stub hit: %s (real body ports in %s)\n", \
                     sym, phase);                                              \
        }                                                                      \
    } while (0)

// ============================================================
// §367 JUTAssert shims — donor JASTrack.cpp/JASSeqParser.cpp use
// JUT_ASSERT(line, cond) / JUT_WARN(line, fmt, msg) from
// JSystem/JUtility/JUTAssert.h; the receiver tree has no JUT_ASSERT.
// PC shim: report (donor line number kept) and continue — the donor
// halt path is a GC console behavior with no PC counterpart.
// ============================================================
#ifndef JUT_ASSERT
#define JUT_ASSERT(line, cond)                                                        \
    do {                                                                              \
        if (!(cond)) {                                                                \
            OSReport("JA1 \xC2\xA7""367 JUT_ASSERT (donor line %d) failed: %s\n",     \
                     (int)(line), #cond);                                             \
        }                                                                             \
    } while (0)
#endif
#ifndef JUT_WARN
#define JUT_WARN(line, fmt, msg) \
    OSReport("JA1 \xC2\xA7""367 JUT_WARN (donor line %d): " fmt "\n", (int)(line), msg)
#endif

namespace JAudio1 {

// ============================================================
// §367 dolphin-SDK shim — donor calls OSf32tos8 (GC fast-cast
// intrinsic). The receiver's dolphin/os.h ships OSf32tou8/OSf32tos16
// only; PC path there is a plain cast, mirrored here.
// ============================================================
inline void OSf32tos8(f32* f, s8* out) { *out = (s8)*f; }

// ============================================================
// §367 JSupport inlines — donor-verbatim from WW
// JSystem/JSupport/JSupport.h:22-23 (donor has them at global scope;
// hosted in JAudio1 so the wrapped call sites resolve unchanged).
// ============================================================
inline u8 JSULoNibble(u8 param_0) { return param_0 & 0x0f; }
inline u8 JSUHiNibble(u8 param_0) { return (param_0 & 0xff) >> 4; }

// ============================================================
// §367/§369 namespace Kernel — REAL after A4 (donor JASRate.h /
// JASCallback.h; bodies in ja1_kernel.cpp).
// Clock authority rule (§363 precedent, kept): where the PC driver owns
// a live value (DAC rate, subframe count, frame samples, DAC size) the
// JA1 getter maps onto the SAME PC driver getter the JA2 stack uses —
// the JA1 stack must run on the clock of the DSP that renders it.
// Donor retail values for reference (JASRate.cpp): gDacRate=32028.5
// (PC driver: 32000.0 — the receiver's own TARGET_PC decision, no AI
// clock skew on PC), gSubFrames=7, gFrameSamples=560, gDacSize=1120.
// gOutputRate/gAiSetting have no PC counterpart — ported verbatim
// (zero-init retail rows, ja1_kernel.cpp).
// ============================================================
namespace Kernel {
    inline f32 getDacRate() { return JASDriver::getDacRate(); }      // ===== §367: §363 precedent shim
    inline u32 getSubFrames() { return JASDriver::getSubFrames(); }  // ===== §369: same authority rule (PC value 7 == donor retail 7)
    inline u32 getFrameSamples() { return JASDriver::getFrameSamples(); }  // ===== §369: PC 560 == donor retail 560
    inline u32 getDacSize() { return JASDriver::getDacSize(); }      // ===== §369: PC 1120 == donor retail 1120

    // ===== §369: donor JASRate.cpp rows with no PC counterpart —
    // ported verbatim (zero-init retail). Defined in ja1_kernel.cpp.
    extern int gOutputRate;
    extern int gAiSetting;
    inline int getOutputRate() { return gOutputRate; }  // donor JASRate.h inline (verbatim)
    inline int getAiSetting() { return gAiSetting; }    // donor JASRate.h inline (verbatim)

    // ===== §369: donor JASCallback.h — verbatim decls; bodies in
    // ja1_kernel.cpp (donor JASCallback.cpp). This is the registry the
    // heartbeat pumps: TTrack::startSeq registers rootCallback here
    // (registerSubFrameCallback -> field_0x8=0 -> dispatched by
    // aiCallback, exactly the donor's naming/dispatch pairing).
    void resetCallback();
    int checkCallback(s32 (*)(void*), void*);
    int registerDspCallback(s32 (*)(void*), void*);
    int registerSubFrameCallback(s32 (*)(void*), void*);
    void aiCallback();
    void subframeCallback();

    struct unk_callList {
        /* 0x00 */ s32 (*field_0x0)(void*);
        /* 0x04 */ void* field_0x4;
        /* 0x08 */ u32 field_0x8;
    };

    extern u32 maxCallbacksUser;
    extern unk_callList* callList;
    extern bool callbackInit;
}  // namespace Kernel

// ============================================================
// §367/§370 namespace Calc — REAL after A4.5 (donor JASCalc.h decls,
// verbatim; bodies in ja1_jascalc.cpp = donor JASCalc.cpp). sinfT/
// sinfDolby2 are TABLE-DRIVEN: JASC_SINTABLE/JASC_DOL2TABLE are built
// by initSinfT() at gate-ON boot (donor Kernel::init leg,
// JASAiCtrl.cpp:31 — wired in ja1_native.cpp) exactly like the donor;
// there is no baked table to port.
// ============================================================
namespace Calc {
    void initSinfT();
    f32 sinfT(f32);
    f32 sinfDolby2(f32);
    void imixcopy(const s16*, const s16*, s16*, s32);
    void bcopyfast(const u32*, u32*, u32);
    void bcopy(const void*, void*, u32);
    void bzerofast(void*, u32);
    void bzero(void*, u32);

    extern f32* JASC_SINTABLE;
    extern f32* JASC_DOL2TABLE;
}  // namespace Calc

// ============================================================
// §367/§369 namespace Player remainder — REAL after A4 (donor
// JASPlayer.h; verbatim bodies/data in ja1_jasplayer.cpp = donor
// JASPlayer_impl.cpp remainder). The §363 oscillator port carries the
// envelope default tables (ja1_oscillator.{h,cpp}); these are the
// NON-oscillator Player symbols.
// ============================================================
namespace Player {
    s16 extend8to16(u8);
    f32 pitchToCent(f32, f32);
    s32 getRandomS32();
    // donor JASPlayer_impl.cpp:20 (64-entry 1/64th-semitone cent table)
    extern const f32 s_key_table[64];
    // donor JASPlayer_impl.cpp:81-146 — 512 s16 (128 cutoff steps x 4
    // IIR coefs). Non-const to match the donor decl.
    extern s16 CUTOFF_TO_IIR_TABLE[512];
}  // namespace Player

// ============================================================
// §367 BOUNDARY — RequestId (donor-verbatim enum from
// JSystem/JAudio/JASTrackInterrupt.h; consumed by TTrack::mainProc /
// pause / writePortAppDirect).
// ============================================================
enum RequestId {
    REQUEST_UNK_0 = 0,
    REQUEST_UNK_1 = 1,
    REQUEST_UNK_2 = 2,
    REQUEST_UNK_3 = 3,
    REQUEST_UNK_4 = 4,
    REQUEST_UNK_5 = 5,
    REQUEST_UNK_6 = 6,
    REQUEST_UNK_7 = 7,
};

// ============================================================
// §367/§370 — TSeqCtrl, donor-verbatim layout + header inlines from
// JSystem/JAudio/JASSeqCtrl.h (TTrack embeds it by value inside a
// union, so the full donor layout must exist here). Out-of-line bodies
// REAL after A4.5: ja1_jasseqctrl.cpp (donor JASSeqCtrl.cpp).
// ============================================================
class TSeqCtrl {
public:
    void call(u32 offset) {
        mLoopStartPositions[mLoopIndex++] = mCurrentFilePtr;
        mCurrentFilePtr = mRawFilePtr + offset;
    }
    void clrIntr() { mPreviousFilePtr = 0; }
    u8* getAddr(u32 offset) { return mRawFilePtr + offset; }
    u8* getBase() { return mRawFilePtr; }
    u8 getByte(u32 offset) const { return mRawFilePtr[offset]; }
    u16 getLoopCount() const {
        if (mLoopIndex == 0) {
            return 0;
        }
        return mLoopTimers[mLoopIndex - 1];
    }
    s32 getWait() const { return mWaitTimer; }
    void isIntr() const {}
    void jump(u32 offset) {
        mCurrentFilePtr = mRawFilePtr + offset;
    }
    void loopStart(u32 timer) {
        mLoopStartPositions[mLoopIndex] = mCurrentFilePtr;
        mLoopTimers[mLoopIndex++] = timer;
    }
    u8 readByte() { return *mCurrentFilePtr++; }
    bool ret() {
        mCurrentFilePtr = mLoopStartPositions[--mLoopIndex];
        return true;
    }
    void wait(s32 timer) {
        mWaitTimer = timer;
    }

    void init();
    void start(void*, u32);
    int loopEnd();
    bool waitCountDown();
    bool callIntr(void*);
    bool retIntr();
    u16 get16(u32) const;
    u32 get24(u32) const;
    u32 get32(u32) const;
    u16 read16();
    u32 read24();

    /* 0x00 */ u8* mRawFilePtr;
    /* 0x04 */ u8* mCurrentFilePtr;
    /* 0x08 */ s32 mWaitTimer;
    /* 0x0C */ u32 mLoopIndex;
    /* 0x10 */ u8* mLoopStartPositions[8];
    /* 0x30 */ u16 mLoopTimers[8];
    /* 0x40 */ int field_0x40;
    /* 0x44 */ u8* mPreviousFilePtr;
};

// ============================================================
// §367/§370 — TTrackPort, donor-verbatim layout from
// JSystem/JAudio/JASTrackPort.h (embedded by value in TTrack).
// Out-of-line bodies REAL after A4.5: ja1_jastrackport.cpp (donor
// JASTrackPort.cpp).
// ============================================================
#define TRACKPORT_MAX (16)

class TTrackPort {
public:
    void init();
    u16 readImport(int);
    u16 readExport(int);
    void writeImport(int, u16);
    void writeExport(int, u16);

    u8 checkImport(int i) const { return mImportFlag[i]; }
    u8 checkExport(int i) const { return mExportFlag[i]; }
    u16 get(u32 i) { return mValue[i]; }

    /* 0x00 */ u8 mImportFlag[TRACKPORT_MAX];
    /* 0x10 */ u8 mExportFlag[TRACKPORT_MAX];
    /* 0x20 */ u16 mValue[TRACKPORT_MAX];
};

// ============================================================
// §367/§370 — TIntrMgr, donor-verbatim layout + header inlines from
// JSystem/JAudio/JASTrackInterrupt.h (embedded by value in TTrack).
// Out-of-line bodies REAL after A4.5: ja1_jastrackinterrupt.cpp (donor
// JASTrackInterrupt.cpp).
// ============================================================
class TIntrMgr {
public:
    void disable() { field_0x0 = 0; }
    void enable() { field_0x0 = 1; }
    void setTimer(u32 param_1, u32 param_2) {
        field_0x3 = param_1;
        field_0x4 = param_2;
        field_0x8 = param_2;
    }

    void init();
    void request(u32);
    void setIntr(u32, void*);
    void resetInter(u32);
    void* checkIntr();
    void timerProcess();

    /* 0x00 */ u8 field_0x0;
    /* 0x01 */ u8 field_0x1;
    /* 0x02 */ u8 field_0x2;
    /* 0x03 */ u8 field_0x3;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ void* field_0xc[8];
};

// ============================================================
// §368 NOTE — the A2 TChannel SILHOUETTE and the TChannelMgr boundary
// definition that lived here were REPLACED WHOLESALE in A3 by the real
// donor classes in d/ext_seq/ja1_jaschannel.h (bodies in
// ja1_jaschannel.cpp). See the boundary table in the file header.
// ============================================================

// ============================================================
// §367/§370 — TRegisterParam, donor-verbatim layout + header
// inlines from JSystem/JAudio/JASRegisterParam.h (embedded by value in
// TTrack). Out-of-line bodies REAL after A4.5:
// ja1_jasregisterparam.cpp (donor JASRegisterParam.cpp).
// ============================================================
class TRegisterParam {
public:
    static const u8 PARAM_REG_XY = 0x23;
    static const u8 PARAM_REG_AR0 = 0x28;
    static const u8 PARAM_REG_AR3 = 0x2B;

    TRegisterParam();
    void init();
    void inherit(const TRegisterParam&);
    u8 getBankNumber() const;
    u8 getProgramNumber() const;

    u32 getAddress(int index) const {
        JUT_ASSERT(130, index >= 0);
        JUT_ASSERT(131, index < 4);
        return field_0x20[index];
    }
    void setAddress(int index, u32 value) {
        JUT_ASSERT(124, index >= 0);
        JUT_ASSERT(125, index < 4);
        field_0x20[index] = value;
    }

    void getBendSense() const {}
    u16 getPanPowerBank() const {
        return mPanPower[0];
    }
    u16 getPanPowerExt() const {
        return mPanPower[1];
    }
    u16 getPanPowerOsc() const {
        return mPanPower[2];
    }
    u16 getPanPowerParent() const {
        return mPanPower[3];
    }
    u16 getPanPowerTrack() const {
        return mPanPower[4];
    }
    void getPriority() const {}
    u16 getFlag() const { return field_0x0[3]; }
    void setFlag(u16 flag) { field_0x0[3] = flag; }
    void setPanPower(int i, u16 power) { mPanPower[i] = power; }

    /* 0x00 */ u16 field_0x0[6];
    /* 0x0C */ u16 field_0xc;
    /* 0x0E */ u16 field_0xe;
    /* 0x10 */ u16 mPanPower[5];
    /* 0x1A */ u16 field_0x1a;
    /* 0x1C */ int field_0x1c;
    /* 0x20 */ u32 field_0x20[4];
};

// ============================================================
// §368 NOTE — the A2 BankMgr stubs (getPhysicalNumber/noteOn/gateOn)
// retired to the real donor bodies in d/ext_seq/ja1_jasbank.{h,cpp};
// the A2 TGlobalChannel::releaseAll stub retired to
// d/ext_seq/ja1_jaschannel.{h,cpp}.
// ============================================================

}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_BOUNDARY_H */
