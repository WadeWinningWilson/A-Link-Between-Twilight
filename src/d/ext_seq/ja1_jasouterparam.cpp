// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASOuterParam.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 outer parameters — donor-verbatim port of WW retail
// JSystem/JAudio/JASOuterParam.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept). Phase A4.5 of the native JA1 campaign (bus
// §362/§363/§366). TTrack::TOuterParam is the game-side parameter
// mailbox (volume/pitch/fxmix/dolby/pan/tempo + FIR filter) each track
// consumes via checkOuterSwitch/getOuterUpdate in updateTrackAll.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept (the OUTERPARAM_* flag enum
// already lives in ja1_jastrack.h per the §367 namespace wrap).
// Donor include map: JASTrack.h -> d/ext_seq/ja1_jastrack.h.
// This TU retires the A2-support TOuterParam stub fence in
// ja1_jastrack.h.
// ============================================================

#include "d/ext_seq/ja1_jastrack.h"

namespace JAudio1 {

/* 8027DFD8-8027E020       .text __ct__Q38JASystem6TTrack11TOuterParamFv */
TTrack::TOuterParam::TOuterParam() {
    field_0x0 = 0;
    field_0x2 = 0;
    mVolume = 0.0f;
    mPitch = 0.0f;
    mFxmix = 0.0f;
    mDolby = 0.0f;
    mPan = 0.0f;
    mTempo = 0.0f;
    for (int i = 0; i < 8; i++) {
        field_0x1c[i] = 0;
    }
}

/* 8027E020-8027E030       .text initExtBuffer__Q38JASystem6TTrack11TOuterParamFv */
void TTrack::TOuterParam::initExtBuffer() {
    field_0x0 = 0;
    field_0x2 = 0;
}

/* 8027E030-8027E038       .text setOuterSwitch__Q38JASystem6TTrack11TOuterParamFUs */
void TTrack::TOuterParam::setOuterSwitch(u16 param_1) {
    field_0x0 = param_1;
}

/* 8027E038-8027E054       .text checkOuterSwitch__Q38JASystem6TTrack11TOuterParamFUs */
bool TTrack::TOuterParam::checkOuterSwitch(u16 param_1) {
    return field_0x0 & param_1;
}

/* 8027E054-8027E05C       .text setOuterUpdate__Q38JASystem6TTrack11TOuterParamFUs */
void TTrack::TOuterParam::setOuterUpdate(u16 param_1) {
    field_0x2 = param_1;
}

/* 8027E05C-8027E064       .text getOuterUpdate__Q38JASystem6TTrack11TOuterParamFv */
u16 TTrack::TOuterParam::getOuterUpdate() {
    return field_0x2;
}

/* 8027E064-8027E074       .text getIntFirFilter__Q38JASystem6TTrack11TOuterParamFUc */
s16 TTrack::TOuterParam::getIntFirFilter(u8 param_1) {
    return field_0x1c[param_1];
}

/* 8027E074-8027E110       .text setParam__Q38JASystem6TTrack11TOuterParamFUcf */
void TTrack::TOuterParam::setParam(u8 param_1, f32 param_2) {
    f32* var1;
    switch (param_1) {
    case OUTERPARAM_Volume:
        var1 = &mVolume;
        break;
    case OUTERPARAM_Pitch:
        var1 = &mPitch;
        break;
    case OUTERPARAM_Fxmix:
        var1 = &mFxmix;
        break;
    case OUTERPARAM_Dolby:
        var1 = &mDolby;
        break;
    case OUTERPARAM_Pan:
        var1 = &mPan;
        break;
    case OUTERPARAM_Tempo:
        var1 = &mTempo;
        break;
    default:
        return;
    }
    *var1 = param_2;
    field_0x2 |= param_1;
}

/* 8027E110-8027E12C       .text onSwitch__Q38JASystem6TTrack11TOuterParamFUs */
void TTrack::TOuterParam::onSwitch(u16 param_1) {
    field_0x0 |= param_1;
    field_0x2 |= param_1;
}

/* 8027E12C-8027E170       .text setFirFilter__Q38JASystem6TTrack11TOuterParamFPs */
void TTrack::TOuterParam::setFirFilter(s16* param_1) {
    field_0x2 |= 0x80;
    field_0x0 |= 0x80;
    for (u8 i = 0; i < 8; i++) {
        field_0x1c[i] = param_1[i];
    }
}

}  // namespace JAudio1
