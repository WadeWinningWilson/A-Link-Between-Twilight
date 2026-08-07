#ifndef D_EXT_SEQ_JA1_OSCILLATOR_H
#define D_EXT_SEQ_JA1_OSCILLATOR_H

// ============================================================
// §363 JAudio1 oscillator/envelope engine — donor-verbatim port of
// WW retail JSystem/JAudio/JASOscillator.h (D:/XXXXXXX/WW DP).
// Donor SYMBOL names are kept, wrapped in namespace JAudio1 so the
// class cannot collide with TP JAudio2's JASOscillator (§328 same-name
// lesson). Phase A of the native JA1 audio campaign (bus §362): this
// class is the JA1-true envelope AUTHORITY — Phase A converts its
// per-track Osc_ data onto the JA2 voice backend at noteOn; Phase B+
// drives voices from this engine directly and diffs JA1-computed
// envelope values against what JA2 plays.
// ============================================================

#include "dolphin/types.h"

namespace JAudio1 {
    class TOscillator {
    public:
        struct Osc_ {
            /* 0x00 */ u8 field_0x0;
            /* 0x04 */ f32 field_0x4;
            /* 0x08 */ s16* table;
            /* 0x0C */ s16* rel_table;
            /* 0x10 */ f32 field_0x10;
            /* 0x14 */ f32 field_0x14;
        };

        TOscillator() { init(); }
        ~TOscillator() {}
        void init();
        void initStart();
        f32 getOffset();
        bool forceStop();
        bool release();
        f32 calc(s16*);

        const Osc_* getOsc() const { return mOsc; }
        void setOsc(const Osc_* osc) { mOsc = osc; }
        BOOL isOsc() { return mOsc != NULL ? TRUE : FALSE; }
        void releaseDirect(u16 param_0) { mDirectRelease = param_0; }
        void bankOscToOfs() {}
        void getEffectorKind() const {}
        void getOscMode() const {}

        static s16 oscTableForceStop[6];
        static const f32 relTableSampleCell[17];
        static const f32 relTableSqRoot[17];
        static const f32 relTableSquare[17];

        /* 0x00 */ const Osc_* mOsc;
        /* 0x04 */ u8 mState;
        /* 0x05 */ u8 field_0x5;
        /* 0x06 */ u16 field_0x6;
        /* 0x08 */ f32 mReleaseRate;
        /* 0x0C */ f32 mPhase;
        /* 0x10 */ f32 mTargetPhase;
        /* 0x14 */ f32 mPhaseChangeRate;
        /* 0x18 */ u16 mDirectRelease;
        /* 0x1C */ f32 mInitialReleaseRate;
    };

    // ============================================================
    // §363 donor envelope default tables — JASystem::Player statics,
    // donor-verbatim from WW JASPlayer_impl.cpp:65-79 and :148-164.
    // Only the oscillator/envelope family is ported (Phase A scope);
    // the rest of Player stays donor-side.
    // ============================================================
    namespace Player {
        extern const TOscillator::Osc_ sAdsrDef;
        extern const TOscillator::Osc_ sEnvelopeDef;
        extern const TOscillator::Osc_ sVibratoDef;
        extern const TOscillator::Osc_ sTremoroDef;
        extern s16 sAdsTable[12];
        extern s16 sRelTable[6];
        extern s16 sVibTable[18];
        extern s16 sTreTable[18];
    }  // namespace Player
}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_OSCILLATOR_H */
