#ifndef D_EXT_SEQ_JA1_SEQ_CTRL_H
#define D_EXT_SEQ_JA1_SEQ_CTRL_H

#include <types.h>

namespace ExtSeq {

/** WW JASystem::TSeqCtrl — bytecode cursor over a BMS image. */
class Ja1SeqCtrl {
public:
    void init();
    void start(void* base, u32 offset);
    int loopEnd();
    bool waitCountDown();
    bool callIntr(void* addr);
    bool retIntr();
    u16 get16(u32 offset) const;
    u32 get24(u32 offset) const;
    u32 get32(u32 offset) const;
    u16 read16();
    u32 read24();

    void call(u32 offset) {
        mLoopStartPositions[mLoopIndex++] = mCurrentFilePtr;
        mCurrentFilePtr = mRawFilePtr + offset;
    }
    void clrIntr() { mPreviousFilePtr = NULL; }
    u8* getAddr(u32 offset) { return mRawFilePtr + offset; }
    u8* getBase() { return mRawFilePtr; }
    u8 getByte(u32 offset) const { return mRawFilePtr[offset]; }
    u16 getLoopCount() const {
        return mLoopIndex == 0 ? 0 : mLoopTimers[mLoopIndex - 1];
    }
    s32 getWait() const { return mWaitTimer; }
    void jump(u32 offset) { mCurrentFilePtr = mRawFilePtr + offset; }
    void loopStart(u32 timer) {
        mLoopStartPositions[mLoopIndex] = mCurrentFilePtr;
        mLoopTimers[mLoopIndex++] = static_cast<u16>(timer);
    }
    u8 readByte() { return *mCurrentFilePtr++; }
    bool ret() {
        mCurrentFilePtr = mLoopStartPositions[--mLoopIndex];
        return true;
    }
    void wait(s32 timer) { mWaitTimer = timer; }

    u8* mRawFilePtr = nullptr;
    u8* mCurrentFilePtr = nullptr;
    s32 mWaitTimer = 0;
    u32 mLoopIndex = 0;
    u8* mLoopStartPositions[8]{};
    u16 mLoopTimers[8]{};
    int field_0x40 = 0;
    u8* mPreviousFilePtr = nullptr;
};

}  // namespace ExtSeq

#endif
