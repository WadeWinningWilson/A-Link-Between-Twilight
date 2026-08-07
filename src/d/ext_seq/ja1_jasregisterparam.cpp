// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASRegisterParam.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 register parameters — donor-verbatim port of WW retail
// JSystem/JAudio/JASRegisterParam.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept). Phase A4.5 of the native JA1 campaign (bus
// §362/§363/§366). TRegisterParam is the per-track BMS register file
// (bank/program in field_0xc, pan powers, indirect-address slots) the
// parser's reg commands read/write.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept.
// Donor include map: JASRegisterParam.h -> d/ext_seq/ja1_boundary.h
// (the donor-verbatim class layout lives there since A2);
// dolphin/types.h -> receiver dolphin/types.h (via ja1_boundary.h).
// This TU retires the A2-support TRegisterParam stub fence in
// ja1_boundary.h.
// ============================================================

#include "d/ext_seq/ja1_boundary.h"

namespace JAudio1 {

/* 8027E2C0-8027E310       .text __ct__Q28JASystem14TRegisterParamFv */
TRegisterParam::TRegisterParam() {
    field_0x0[0] = 0;
    field_0x0[1] = 0;
    field_0x0[2] = 0;
    field_0x0[3] = 0;
    field_0x0[4] = 0;
    field_0x0[5] = 0;
    field_0xc = 0;
    field_0xe = 0;
    field_0x1a = 0;
    mPanPower[0] = 0;
    mPanPower[1] = 0;
    mPanPower[2] = 0;
    mPanPower[3] = 0;
    mPanPower[4] = 0;
    field_0x20[0] = 0;
    field_0x20[1] = 0;
    field_0x20[2] = 0;
    field_0x20[3] = 0;
}

/* 8027E310-8027E378       .text init__Q28JASystem14TRegisterParamFv */
void TRegisterParam::init() {
    field_0x0[0] = 0;
    field_0x0[1] = 0;
    field_0x0[2] = 0;
    field_0x0[3] = 0;
    field_0x0[4] = 0;
    field_0x0[5] = 0;
    field_0xc = 0xf0;
    field_0xe = 0x0c;
    field_0x1a = 0x40;
    mPanPower[0] = 0;
    mPanPower[1] = 1;
    mPanPower[2] = 1;
    mPanPower[3] = 0x7fff;
    mPanPower[4] = 0x4000;
    field_0x20[0] = 0;
    field_0x20[1] = 0;
    field_0x20[2] = 0;
    field_0x20[3] = 0;
}

/* 8027E378-8027E3E0       .text inherit__Q28JASystem14TRegisterParamFRCQ28JASystem14TRegisterParam */
void TRegisterParam::inherit(const TRegisterParam& param_1) {
    field_0x0[0] = 0;
    field_0x0[1] = 0;
    field_0x0[2] = 0;
    field_0x0[3] = 0;
    field_0x0[4] = 0;
    field_0x0[5] = 0;
    field_0xc = param_1.field_0xc;
    field_0xe = param_1.field_0xe;
    field_0x1a = param_1.field_0x1a;
    for (int i = 0; i < 5; i++) {
        mPanPower[i] = param_1.mPanPower[i];
    }
    field_0x20[0] = 0;
    field_0x20[1] = 0;
    field_0x20[2] = 0;
    field_0x20[3] = 0;
}

/* 8027E3E0-8027E3EC       .text getBankNumber__Q28JASystem14TRegisterParamCFv */
u8 TRegisterParam::getBankNumber() const {
    return field_0xc >> 8 & 0xff;
}

/* 8027E3EC-8027E3F8       .text getProgramNumber__Q28JASystem14TRegisterParamCFv */
u8 TRegisterParam::getProgramNumber() const {
    return field_0xc & 0xff;
}

}  // namespace JAudio1
