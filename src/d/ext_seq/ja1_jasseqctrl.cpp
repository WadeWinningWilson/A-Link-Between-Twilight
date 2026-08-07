// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASSeqCtrl.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 sequence controller — donor-verbatim port of WW retail
// JSystem/JAudio/JASSeqCtrl.cpp (D:/XXXXXXX/WW DP; dtk address markers
// kept). Phase A4.5 of the native JA1 campaign (bus §362/§363/§366) —
// THE BMS BYTE READER: TSeqCtrl is the cursor every TSeqParser command
// fetch runs through; without these bodies the native player starts
// but reads nothing.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept.
// Donor include map: JASSeqCtrl.h -> d/ext_seq/ja1_boundary.h (the
// donor-verbatim class layout lives there since A2); JUTAssert.h ->
// the ja1_boundary.h JUT shims.
// This TU retires the A2-support TSeqCtrl stub fence in ja1_boundary.h.
// ============================================================

#include "d/ext_seq/ja1_boundary.h"

namespace JAudio1 {

/* 8027E3F8-8027E448       .text init__Q28JASystem8TSeqCtrlFv */
void TSeqCtrl::init() {
    mRawFilePtr = NULL;
    mCurrentFilePtr = NULL;
    mWaitTimer = 0;
    mLoopIndex = 0;
    for (int i = 0; i < 8; i++) {
        mLoopStartPositions[i] = NULL;
        mLoopTimers[i] = 0;
    }
    field_0x40 = 0;
    mPreviousFilePtr = NULL;
}

/* 8027E448-8027E45C       .text start__Q28JASystem8TSeqCtrlFPvUl */
void TSeqCtrl::start(void* param_1, u32 param_2) {
    mRawFilePtr = (u8*)param_1;
    mCurrentFilePtr = mRawFilePtr + param_2;
}

/* 8027E45C-8027E500       .text loopEnd__Q28JASystem8TSeqCtrlFv */
int TSeqCtrl::loopEnd() {
    u32 var1 = mLoopIndex;
    if (var1 == 0) {
        JUT_WARN(45, "%s", "cannot loopE for call-stack is NULL");
        return 0;
    }
    u16 var3 = mLoopTimers[var1 - 1];
    if (var3) {
        var3--;
    }
    if (var3 == 0) {
        mLoopIndex--;
        return true;
    }
    mLoopTimers[var1 - 1] = var3;
    mCurrentFilePtr = mLoopStartPositions[mLoopIndex - 1];
    return true;
}

/* 8027E500-8027E530       .text waitCountDown__Q28JASystem8TSeqCtrlFv */
bool TSeqCtrl::waitCountDown() {
    if (mWaitTimer > 0) {
        mWaitTimer--;
        if (mWaitTimer) {
            return false;
        }
    }
    return true;
}

/* 8027E530-8027E568       .text callIntr__Q28JASystem8TSeqCtrlFPv */
bool TSeqCtrl::callIntr(void* param_1) {
    if (mPreviousFilePtr) {
        return false;
    }
    mPreviousFilePtr = mCurrentFilePtr;
    mCurrentFilePtr = (u8*)param_1;
    field_0x40 = mWaitTimer;
    mWaitTimer = 0;
    return true;
}

/* 8027E568-8027E59C       .text retIntr__Q28JASystem8TSeqCtrlFv */
bool TSeqCtrl::retIntr() {
    if (!mPreviousFilePtr) {
        return false;
    }
    mWaitTimer = field_0x40;
    mCurrentFilePtr = mPreviousFilePtr;
    mPreviousFilePtr = NULL;
    return true;
}

/* 8027E59C-8027E5B4       .text get16__Q28JASystem8TSeqCtrlCFUl */
u16 TSeqCtrl::get16(u32 param_1) const {
    u16 result = getByte(param_1++) << 8;
    result |= getByte(param_1);
    return result;
}

/* 8027E5B4-8027E5DC       .text get24__Q28JASystem8TSeqCtrlCFUl */
u32 TSeqCtrl::get24(u32 param_1) const {
    u32 result = getByte(param_1++) << 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1);
    return result;
}

/* 8027E5DC-8027E614       .text get32__Q28JASystem8TSeqCtrlCFUl */
u32 TSeqCtrl::get32(u32 param_1) const {
    u32 result = getByte(param_1++) << 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1);
    return result;
}

/* 8027E614-8027E63C       .text read16__Q28JASystem8TSeqCtrlFv */
u16 TSeqCtrl::read16() {
    u16 result = readByte() << 8;
    result |= readByte();
    return result;
}

/* 8027E63C-8027E680       .text read24__Q28JASystem8TSeqCtrlFv */
u32 TSeqCtrl::read24() {
    u32 result = readByte() << 8;
    result |= readByte();
    result <<= 8;
    result |= readByte();
    return result;
}

}  // namespace JAudio1
