#ifndef D_EXT_SEQ_JA1_JASBANK_H
#define D_EXT_SEQ_JA1_JASBANK_H

// ============================================================
// §368 JAudio1 bank/instrument layer — donor-verbatim port of WW retail
// JSystem/JAudio bank headers (D:/XXXXXXX/WW DP). Phase A3 of the
// native JA1 campaign (bus §362/§363/§366). Namespace convention
// matches §363/§367: donor JASystem::* flattens into JAudio1::*.
// Donor header map (§368):
//   JASInst.h        -> TInstParam / TInst
//   JASBank.h        -> TBank
//   JASBasicInst.h   -> TBasicInst (+TKeymap/TVeloRegion)
//   JASBasicBank.h   -> TBasicBank
//   JASInstEffect.h  -> TInstEffect
//   JASInstRand.h    -> TInstRand
//   JASInstSense.h   -> TInstSense
//   JASDrumSet.h     -> TDrumSet
//   JASBankMgr.h     -> namespace BankMgr
//   JASBNKParser.h   -> namespace BNKParser
//   JASWaveBank.h    -> TWaveInfo / TWaveHandle / TWaveBank
//   JASWaveBankMgr.h -> namespace WaveBankMgr (§369 A4 REAL —
//                       getWaveBank resolves over the bridge-loaded
//                       arcs, body in ja1_bank.cpp; the donor DVD/ARAM
//                       loading TUs JASWaveArcLoader.cpp/JASWSParser.cpp
//                       stay unported: the bridge's buffers ARE the
//                       residency)
// Body TUs: src/d/ext_seq/ja1_jasbank.cpp (bank/inst/mgr) and
// src/d/ext_seq/ja1_jasbnkparser.cpp (donor JASBNKParser.cpp).
// ============================================================

#include "d/ext_seq/ja1_boundary.h"
#include "d/ext_seq/ja1_dsp_boundary.h"  // Driver::Wave_ (the wave-info block)
#include "d/ext_seq/ja1_oscillator.h"

class JKRHeap;

namespace JAudio1 {

class TChannel;
class TChannelMgr;
class TWaveArc;

// ============================================================
// §368 wave-bank interface — donor JASWaveBank.h. TWaveInfo is the
// SAME block donor JASDSPInterface.h declares as Driver::Wave_ (the
// two donor decls alias one layout; unified here — §368 named
// adaptation "merged Wave_/TWaveInfo decl"). getWavePtr follows the
// receiver's own JA2 x64 adaptation (const void* -> intptr_t).
// ============================================================
typedef Driver::Wave_ TWaveInfo;

class TWaveHandle {
public:
    virtual ~TWaveHandle() {}
    virtual const TWaveInfo* getWaveInfo() const = 0;
    virtual intptr_t getWavePtr() const = 0;  // ===== §368: donor const void*; receiver JA2 precedent
};

class TWaveBank {
public:
    TWaveBank() {}
    virtual ~TWaveBank() {}
    virtual TWaveHandle* getWaveHandle(u32) const = 0;
    virtual TWaveArc* getWaveArc(int) = 0;
};

// ============================================================
// §368/§369 namespace WaveBankMgr (donor JASWaveBankMgr.h) — REAL
// after A4: getWaveBank resolves over the wave arcs the ExtSeq bridge
// already loads/registers (body in ja1_bank.cpp — the ExtSeq-backed
// TWaveBank; the donor DVD/ARAM wave-arc loading side is NOT ported,
// the bridge's loaded buffers ARE the residency). NULL stays the donor
// "no wave bank registered" path; BankMgr::assignWaveBank handles it.
// ============================================================
namespace WaveBankMgr {
    TWaveBank* getWaveBank(int);
}  // namespace WaveBankMgr

// ============================================================
// §368 TInstParam / TInst — donor-verbatim from JSystem/JAudio/JASInst.h.
// ============================================================
struct TInstParam {
    TInstParam() {
        field_0x0 = 0;
        field_0x4 = 0;
        mOscData = NULL;
        mOscCount = 0;
        field_0x10 = 1.0f;
        field_0x14 = 1.0f;
        field_0x18 = 1.0f;
        field_0x1c = 1.0f;
        field_0x20 = 0.5f;
        field_0x24 = 0.0f;
        field_0x28 = 0.0f;
        field_0x2c = 0.5f;
        field_0x30 = 0.0f;
        field_0x34 = 0.0f;
        field_0x38 = 0;
        field_0x3a = 0;
        field_0x3c = 0;
        field_0x40 = 0;
    }

    /* 0x00 */ u8 field_0x0;
    /* 0x04 */ int field_0x4;
    /* 0x08 */ TOscillator::Osc_** mOscData;
    /* 0x0C */ u32 mOscCount;
    /* 0x10 */ f32 field_0x10;
    /* 0x14 */ f32 field_0x14;
    /* 0x18 */ f32 field_0x18;
    /* 0x1C */ f32 field_0x1c;
    /* 0x20 */ f32 field_0x20;
    /* 0x24 */ f32 field_0x24;
    /* 0x28 */ f32 field_0x28;
    /* 0x2C */ f32 field_0x2c;
    /* 0x30 */ f32 field_0x30;
    /* 0x34 */ f32 field_0x34;
    /* 0x38 */ u8 field_0x38;
    /* 0x3A */ u16 field_0x3a;
    /* 0x3C */ int field_0x3c;
    /* 0x40 */ int field_0x40;
};

class TInst {
public:
    TInst() {}
    virtual ~TInst() {}
    virtual bool getParam(int, int, TInstParam*) const = 0;
    virtual u32 getType() const = 0;
};

// ============================================================
// §368 TBank — donor-verbatim from JSystem/JAudio/JASBank.h.
// ============================================================
class TBank {
public:
    TBank() {
        field_0x4 = 0;
    }
    virtual ~TBank() {}
    virtual TInst* getInst(int) const = 0;
    virtual u32 getType() const = 0;

    static JKRHeap* getCurrentHeap();

    static JKRHeap* sCurrentHeap;

    /* 0x04 */ TWaveBank* field_0x4;
};

// ============================================================
// §368 TInstEffect family — donor-verbatim from JASInstEffect.h /
// JASInstRand.h / JASInstSense.h.
// ============================================================
class TInstEffect {
public:
    TInstEffect() { mTarget = 0; }
    virtual f32 getY(int, int) const = 0;
    void setTarget(int);

    /* 0x04 */ u8 mTarget;
};

class TInstRand : public TInstEffect {
public:
    TInstRand() {
        field_0x8 = 1.0f;
        field_0xc = 0.0f;
    }
    virtual f32 getY(int, int) const;

    /* 0x08 */ f32 field_0x8;
    /* 0x0C */ f32 field_0xc;
};

class TInstSense : public TInstEffect {
public:
    TInstSense() {
        field_0x8 = 0;
        field_0x9 = 60;
        field_0xc = 1.0f;
        field_0x10 = 1.0f;
    }
    virtual f32 getY(int, int) const;
    void setParams(int, int, f32, f32);

    u8 field_0x8;
    u8 field_0x9;
    f32 field_0xc;
    f32 field_0x10;
};

// ============================================================
// §368 TBasicInst — donor-verbatim from JSystem/JAudio/JASBasicInst.h.
// ============================================================
class TBasicInst : public TInst {
public:
    struct TVeloRegion {
        /* 0x00 */ s32 mBaseVel;
        /* 0x04 */ s32 field_0x04;
        /* 0x08 */ f32 field_0x08;
        /* 0x0C */ f32 field_0x0c;
    };

    class TKeymap {
    public:
        TKeymap() {
            mBaseKey = -1;
            mVeloRegionCount = 0;
            mVelomap = NULL;
        }
        ~TKeymap();
        void setVeloRegionCount(u32);
        TVeloRegion* getVeloRegion(int);
        const TVeloRegion* getVeloRegion(int) const;

        /* 0x00 */ int mBaseKey;
        /* 0x04 */ u32 mVeloRegionCount;
        /* 0x08 */ TVeloRegion* mVelomap;
    };

    TBasicInst();
    ~TBasicInst();
    bool getParam(int, int, TInstParam*) const;
    virtual int getKeymapIndex(int) const;
    void setKeyRegionCount(u32);
    void setEffectCount(u32);
    void setEffect(int, TInstEffect*);
    void setOscCount(u32);
    void setOsc(int, TOscillator::Osc_*);
    TKeymap* getKeyRegion(int);
    u32 getType() const { return 'BSIC'; }

    /* 0x04 */ f32 field_0x4;
    /* 0x08 */ f32 field_0x8;
    /* 0x0C */ TInstEffect** mEffect;
    /* 0x10 */ u32 mEffectCount;
    /* 0x14 */ TOscillator::Osc_** mOsc;
    /* 0x18 */ u32 mOscCount;
    /* 0x1C */ u32 mKeyRegionCount;
    /* 0x20 */ TKeymap* mKeymap;
};

// ============================================================
// §368 TBasicBank — donor-verbatim from JSystem/JAudio/JASBasicBank.h.
// ============================================================
class TBasicBank : public TBank {
public:
    TBasicBank();
    ~TBasicBank();
    void setInstCount(u32);
    void setInst(int, TInst*);
    TInst* getInst(int) const;
    u32 getType() const { return 'BSIC'; }

    /* 0x08 */ TInst** mInstTable;
    /* 0x0C */ u32 mInstCount;
};

// ============================================================
// §368 TDrumSet — donor-verbatim from JSystem/JAudio/JASDrumSet.h.
// ============================================================
class TDrumSet : public TInst {
public:
    class TPerc {
    public:
        TPerc();
        ~TPerc();
        void setEffectCount(u32);
        void setVeloRegionCount(u32);
        TBasicInst::TVeloRegion* getVeloRegion(int);
        void setEffect(int, TInstEffect*);
        void setRelease(u32);

        /* 0x00 */ f32 field_0x0;
        /* 0x04 */ f32 field_0x4;
        /* 0x08 */ f32 field_0x8;
        /* 0x0C */ u16 field_0xc;
        /* 0x10 */ TInstEffect** mEffect;
        /* 0x14 */ u32 mEffectCount;
        /* 0x18 */ u32 mVelomapCount;
        /* 0x1C */ TBasicInst::TVeloRegion* mVelomap;
    };

    TDrumSet() {}
    ~TDrumSet();
    bool getParam(int, int, TInstParam*) const;
    TPerc* getPerc(int);
    u32 getType() const { return 'PERC'; }

    static const u32 sPercCount = 128;

    /* 0x04 */ TPerc field_0x4[sPercCount];
};

// ============================================================
// §368 namespace BankMgr — donor-verbatim from JSystem/JAudio/
// JASBankMgr.h. The A2 boundary stubs (getPhysicalNumber / noteOn /
// gateOn) retire here — real bodies in ja1_jasbank.cpp.
// ============================================================
namespace BankMgr {
    void init(int);
    bool registBank(int, TBank*);
    bool registBankBNK(int, void*);
    TBank* getBank(int);
    u16 getPhysicalNumber(u16);
    void setVir2PhyTable(u32, int);
    bool assignWaveBank(int, int);
    f32 clamp01(f32);
    TChannel* noteOn(TChannelMgr*, int, int, u8, u8, u32);
    TChannel* noteOnOsc(TChannelMgr*, int, u8, u8, u32);
    void gateOn(TChannel*, u8, u8, u32);

    extern s32 sTableSize;
    extern TBank** sBankArray;
    extern u16* sVir2PhyTable;

    extern s16 OSC_RELEASE_TABLE[6];
    extern TOscillator::Osc_ OSC_ENV;
}  // namespace BankMgr

// ============================================================
// §368 namespace BNKParser — donor-verbatim from JSystem/JAudio/
// JASBNKParser.h (the IBNK -> TBasicBank/TDrumSet builder).
// ============================================================
namespace BNKParser {
    struct THeader {
        /* 0x000 */ u8 field_0x0[0x24];
        /* 0x024 */ u32 mInstOffsets[0x80];
        /* 0x224 */ u8 field_0x224[0x190];
        /* 0x3B4 */ u32 mPercOffsets[12];
    };
    struct TInst {
        /* 0x00 */ u8 field_0x0[8];
        /* 0x08 */ f32 field_0x8;
        /* 0x0C */ f32 field_0xC;
        /* 0x10 */ u32 mOscOffsets[2];
        /* 0x18 */ u32 mRandOffsets[2];
        /* 0x20 */ u32 mSenseOffsets[2];
        /* 0x28 */ u32 mKeyRegionCount;
        /* 0x2C */ u32 mKeymapOffsets[2];
    };
    struct TKeymap {
        /* 0x00 */ u8 field_0x0;
        /* 0x04 */ u32 field_0x4;
        /* 0x08 */ u32 mVmapOffsets[1];
    };
    struct TOsc {
        /* 0x00 */ u8 field_0x0;
        /* 0x04 */ f32 field_0x4;
        /* 0x08 */ u32 field_0x8;
        /* 0x0C */ u32 field_0xC;
        /* 0x10 */ f32 field_0x10;
        /* 0x14 */ f32 field_0x14;
    };
    struct TPerc {
        /* 0x000 */ u32 mMagic;
        /* 0x004 */ u8 field_0x4[0x84];
        /* 0x088 */ u32 mPmapOffsets[0x80];
        /* 0x288 */ s8 field_0x288[0x80];
        /* 0x308 */ u16 field_0x308[0x80];
    };
    struct TPmap {
        /* 0x00 */ f32 field_0x0;
        /* 0x04 */ f32 field_0x4;
        /* 0x08 */ u32 mRandOffsets[2];
        /* 0x10 */ u32 mVeloRegionCount;
        /* 0x14 */ u32 mVeloRegionOffsets[1];
    };
    struct TRand {
        /* 0x00 */ u8 field_0x0;
        /* 0x04 */ f32 field_0x4;
        /* 0x08 */ f32 field_0x8;
    };
    struct TSense {
        /* 0x00 */ u8 field_0x0;
        /* 0x01 */ u8 field_0x1;
        /* 0x02 */ u8 field_0x2;
        /* 0x04 */ f32 field_0x4;
        /* 0x08 */ f32 field_0x8;
    };
    struct TVmap {
        /* 0x00 */ u8 field_0x0;
        /* 0x04 */ u32 field_0x4;
        /* 0x08 */ f32 field_0x8;
        /* 0x0C */ f32 field_0xC;
    };

    TBasicBank* createBasicBank(void*);
    TOscillator::Osc_* findOscPtr(TBasicBank*, THeader*, TOsc*);
    s16* getOscTableEndPtr(s16*);

    extern u32 sUsedHeapSize;
}  // namespace BNKParser

}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_JASBANK_H */
