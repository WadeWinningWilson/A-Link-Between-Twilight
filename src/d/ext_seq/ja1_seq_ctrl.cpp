// KIT-LINEAGE: bridge-owed:§369
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
#include "d/ext_seq/ja1_seq_ctrl.h"

namespace ExtSeq {

void Ja1SeqCtrl::init() {
    mRawFilePtr = nullptr;
    mCurrentFilePtr = nullptr;
    mWaitTimer = 0;
    mLoopIndex = 0;
    for (int i = 0; i < 8; i++) {
        mLoopStartPositions[i] = nullptr;
        mLoopTimers[i] = 0;
    }
    field_0x40 = 0;
    mPreviousFilePtr = nullptr;
}

void Ja1SeqCtrl::start(void* param_1, u32 param_2) {
    mRawFilePtr = static_cast<u8*>(param_1);
    mCurrentFilePtr = mRawFilePtr + param_2;
}

int Ja1SeqCtrl::loopEnd() {
    if (mLoopIndex == 0) {
        return 0;
    }
    u16 var3 = mLoopTimers[mLoopIndex - 1];
    if (var3) {
        var3--;
    }
    if (var3 == 0) {
        mLoopIndex--;
        return 1;
    }
    mLoopTimers[mLoopIndex - 1] = var3;
    mCurrentFilePtr = mLoopStartPositions[mLoopIndex - 1];
    return 1;
}

bool Ja1SeqCtrl::waitCountDown() {
    if (mWaitTimer > 0) {
        mWaitTimer--;
        if (mWaitTimer) {
            return false;
        }
    }
    return true;
}

bool Ja1SeqCtrl::callIntr(void* param_1) {
    if (mPreviousFilePtr) {
        return false;
    }
    mPreviousFilePtr = mCurrentFilePtr;
    mCurrentFilePtr = static_cast<u8*>(param_1);
    field_0x40 = mWaitTimer;
    mWaitTimer = 0;
    return true;
}

bool Ja1SeqCtrl::retIntr() {
    if (!mPreviousFilePtr) {
        return false;
    }
    mWaitTimer = field_0x40;
    mCurrentFilePtr = mPreviousFilePtr;
    mPreviousFilePtr = nullptr;
    return true;
}

u16 Ja1SeqCtrl::get16(u32 param_1) const {
    u16 result = getByte(param_1++) << 8;
    result |= getByte(param_1);
    return result;
}

u32 Ja1SeqCtrl::get24(u32 param_1) const {
    u32 result = getByte(param_1++) << 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1);
    return result;
}

u32 Ja1SeqCtrl::get32(u32 param_1) const {
    u32 result = getByte(param_1++) << 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1++);
    result <<= 8;
    result |= getByte(param_1);
    return result;
}

u16 Ja1SeqCtrl::read16() {
    u16 result = readByte() << 8;
    result |= readByte();
    return result;
}

u32 Ja1SeqCtrl::read24() {
    u32 result = readByte() << 8;
    result |= readByte();
    result <<= 8;
    result |= readByte();
    return result;
}

}  // namespace ExtSeq
