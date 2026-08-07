#ifndef D_EXT_SEQ_JA1_JASCHANNEL_H
#define D_EXT_SEQ_JA1_JASCHANNEL_H

// ============================================================
// §368 JAudio1 logical-channel layer — donor-verbatim port of WW retail
// JSystem/JAudio headers (D:/XXXXXXX/WW DP). Phase A3 of the native JA1
// campaign (bus §362/§363/§366; replaces the A2 TChannel SILHOUETTE
// that lived in ja1_boundary.h). Namespace convention matches §363/§367:
// donor JASystem::* flattens into JAudio1::*, donor symbol names kept.
// Donor header map (§368):
//   JASChannel.h      -> TChannel (full donor layout, silhouette retired)
//   JASChannelMgr.h   -> TChannelMgr
//   JASChGlobal.h     -> namespace TGlobalChannel
//   JASChAllocQueue.h -> namespace TDSPQueue + sDspQueueList
//   JASDriverIF.h / JASDSPChannel.h / JASDSPInterface.h
//                     -> d/ext_seq/ja1_dsp_boundary.h (§368 ONE boundary)
//   JASOscillator.h   -> d/ext_seq/ja1_oscillator.h (§363)
//   JSUList.h         -> receiver JSystem/JSupport/JSUList.h (same lineage)
// Body TU: src/d/ext_seq/ja1_jaschannel.cpp (donor JASChannel.cpp +
// JASChannelMgr.cpp + JASChGlobal.cpp + JASChAllocQueue.cpp + the
// C5BASE_PITCHTABLE/polys_table donor data).
// ============================================================

#include "d/ext_seq/ja1_boundary.h"
#include "d/ext_seq/ja1_dsp_boundary.h"
#include "d/ext_seq/ja1_oscillator.h"
#include "JSystem/JSupport/JSUList.h"

namespace JAudio1 {

class TChannelMgr;

// ============================================================
// §368 TChannel — donor-verbatim from JSystem/JAudio/JASChannel.h.
// ============================================================
class TChannel {
public:
    enum CalcType {
        // NB: haven't seen others used, add from calc_sw_table in JASChannel.cpp if needed
        CALC_None           = 0,
        CALC_AddChannelOnly = 1,
        CALC_AddAll         = 13,
        CALC_WeightAll      = 26,
    };

    union MixConfig {
        u16 mWhole;
        // ===== §375: donor GC layout is big-endian with MWERKS MSB-first
        // bitfields: u = HIGH byte of mWhole (the setBusConnect table
        // index), l0 = high nibble of the low byte, l1 = low nibble.
        // x64 is little-endian with LSB-first bitfields — the member
        // order must be mirrored to keep the donor's value semantics
        // (mWhole = (u << 8) | (l0 << 4) | l1). This misread was the
        // whole-stack silence: .u returned the flag byte (0x50/0x10/…)
        // and fed connect_table[80] out of bounds.
        struct {
            u8 l1 : 4;
            u8 l0 : 4;
            u8 u;
        } mParts;
    };

    TChannel() : field_0x4(NULL), field_0x8(NULL), field_0x20(NULL), field_0x24(NULL), field_0xd8(this) {
        for (int i = 0; i < 4; i++) {
            osc[i] = NULL;
        }
    }
    ~TChannel() {}
    void init();
    void setOscillator(u32, TOscillator*);
    void setOscInit(u32, const TOscillator::Osc_*);
    bool forceStopOsc(u32);
    bool releaseOsc(u32);
    void directReleaseOsc(u32, u16);
    f32 bankOscToOfs(u32);
    void effectOsc(u32, f32);
    u8 getOscState(u32) const;
    BOOL isOsc(u32);
    void copyOsc(u32, TOscillator::Osc_*);
    void overwriteOsc(u32, TOscillator::Osc_*);
    void setKeySweepTarget(u8, u32);
    void setPauseFlag(u8);
    void setPauseFlagReq(u8);
    void setPanPower(f32, f32, f32, f32);
    BOOL checkLogicalChannel();
    BOOL play(u32);
    void stop(u16);
    void updateJcToDSP();
    bool forceStopLogicalChannel();
    BOOL stopLogicalChannel();
    BOOL playLogicalChannel();
    void updateEffectorParam();
    static void killBrokenLogicalChannels(TDSPChannel*);
    static int updatecallDSPChannel(TDSPChannel*, u32);
    static f32 calcEffect(const Driver::PanMatrix_*, const Driver::PanMatrix_*, u8);
    static f32 calcPan(const Driver::PanMatrix_*, const Driver::PanMatrix_*, u8);
    void updateJcToDSPInit();
    void updateAutoMixer(f32, f32, f32, f32);
    void updateMixer(f32, f32, f32, f32);
    static u32 extraUpdate(TChannel*, u32);
    static BOOL updatecallLogicalChannel(TChannel*, u32);

    u8 getNoteOnPriority() const { return field_0x48; }
    u8 getReleasePriority() const { return field_0x48 >> 8; }
    u8 getLifeTimePriority() const { return field_0x48 >> 0x10; }

    // TODO
    void setSkipSamples(u32) {}

    /* 0x00 */ u8 field_0x0;
    /* 0x01 */ u8 field_0x1;
    /* 0x02 */ u8 mPauseFlag;
    /* 0x03 */ u8 field_0x3;
    /* 0x04 */ TChannelMgr* field_0x4;
    /* 0x08 */ TChannel** field_0x8;
    /* 0x0C */ u8 field_0xc;
    /* 0x10 */ Driver::Wave_* field_0x10;
    /* 0x14 */ intptr_t field_0x14;  // ===== §368: donor int; holds getWavePtr() — x64 widening (u32 ARAM-offset currency preserved by DuskDsp)
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1c;
    /* 0x20 */ TDSPChannel* field_0x20;
    /* 0x24 */ TChannel* field_0x24;
    /* 0x28 */ BOOL (*field_0x28)(TChannel*, u32);
    /* 0x2C */ u32 (*field_0x2c)(TChannel*, u32);
    /* 0x30 */ int field_0x30;
    /* 0x34 */ s32 field_0x34;
    /* 0x38 */ TOscillator* osc[4];
    /* 0x48 */ u32 field_0x48;
    /* 0x4C */ u16 field_0x4c;
    /* 0x50 */ f32 field_0x50;
    /* 0x54 */ f32 field_0x54;
    /* 0x58 */ f32 field_0x58;
    /* 0x5C */ f32 field_0x5c;
    /* 0x60 */ u8 mCalcTypes[3]; // Pan, FxMix, ?
    /* 0x64 */ Driver::PanMatrix_ mPanPower;
    /* 0x70 */ Driver::PanMatrix_ mPanVec;
    /* 0x7C */ Driver::PanMatrix_ mFxmixVec;
    /* 0x88 */ Driver::PanMatrix_ mDolbyVec;
    /* 0x94 */ f32 field_0x94;
    /* 0x98 */ f32 field_0x98;
    /* 0x9C */ f32 field_0x9c;
    /* 0xA0 */ u16 mPitch;
    /* 0xA2 */ u16 field_0xa2;
    /* 0xA4 */ TChannelMgr* field_0xa4;
    /* 0xA8 */ f32 field_0xa8;
    /* 0xAC */ f32 field_0xac;
    /* 0xB0 */ MixConfig mMixConfigs[6];
    /* 0xBC */ u16 field_0xbc[6];
    /* 0xC8 */ u16 field_0xc8;
    /* 0xCC */ u32 field_0xcc;  // ===== §368: donor int; the u32 allockey (donor stores/compares it unsigned)
    /* 0xD0 */ int field_0xd0;
    /* 0xD4 */ int field_0xd4;
    /* 0xD8 */ JSULink<TChannel> field_0xd8;
    /* 0xE8 */ u32 field_0xe8;
};

// ============================================================
// §368 TChannelMgr — donor-verbatim from JSystem/JAudio/JASChannelMgr.h
// (was an A2 boundary definition; the class moved here with its real
// bodies in ja1_jaschannel.cpp).
// ============================================================
class TChannelMgr {
public:
    void init();
    void stopAll();
    void stopAllRelease();
    void initAllocChannel(u32);
    TChannel* getLogicalChannel(u32);
    BOOL moveListHead(TChannel*, u32);
    BOOL moveListTail(TChannel*, u32);
    void addListHead(TChannel*, u32);
    void addListTail(TChannel*, u32);
    TChannel* getListHead(u32);
    int cutList(TChannel*);
    void receiveAllChannels(TChannelMgr*);
    int checkLimitStart(u32);
    void checkLimitStop(TChannel*, u32);

    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ TChannel* field_0x8;
    /* 0x0C */ TChannel* field_0xc;
    /* 0x10 */ TChannel* field_0x10;
    /* 0x14 */ TChannel* field_0x14;
    /* 0x18 */ f32 field_0x18;
    /* 0x1C */ f32 field_0x1c;
    /* 0x20 */ f32 field_0x20;
    /* 0x24 */ f32 field_0x24;
    /* 0x28 */ f32 field_0x28;
    /* 0x2C */ s16 field_0x2c[8];
    /* 0x3C */ s16 field_0x3c[4];
    /* 0x44 */ int field_0x44;
    /* 0x48 */ int field_0x48;
    /* 0x4C */ s16 field_0x4c;
    /* 0x4E */ u16 field_0x4e[6];
    /* 0x5A */ u8 field_0x5a[6];
    /* 0x60 */ u8 field_0x60;
    /* 0x61 */ u8 field_0x61;
    /* 0x62 */ u8 mCalcTypes[3]; // Pan, FxMix, ?
    /* 0x68 */ int field_0x68;
    /* 0x6C */ u16 field_0x6c;
    /* 0x70 */ int field_0x70;
};

// ============================================================
// §368 TGlobalChannel — donor-verbatim from JSystem/JAudio/JASChGlobal.h
// (real bodies in ja1_jaschannel.cpp; the A2 releaseAll stub retired).
// ============================================================
namespace TGlobalChannel {
    TChannel* getChannelHandle(u32);
    void init();
    int alloc(TChannelMgr*, u32);
    int release(TChannel*);
    int releaseAll(TChannelMgr*);

    extern TChannelMgr* sChannelMgr;
    extern TChannel* sChannel;
    extern TOscillator* sOscillator;
}  // namespace TGlobalChannel

// ============================================================
// §368 TDSPQueue — donor-verbatim from JSystem/JAudio/JASChAllocQueue.h
// (real bodies in ja1_jaschannel.cpp).
// ============================================================
namespace TDSPQueue {
    void deQueue();
    void enQueue(TChannel*);
    int deleteQueue(TChannel*);
    void checkQueue();
}  // namespace TDSPQueue

extern JSUList<TChannel> sDspQueueList;

}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_JASCHANNEL_H */
