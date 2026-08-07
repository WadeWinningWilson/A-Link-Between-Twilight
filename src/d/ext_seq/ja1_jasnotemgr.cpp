// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASNoteMgr.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 note manager — donor-verbatim port of WW retail
// JSystem/JAudio/JASNoteMgr.cpp (D:/XXXXXXX/WW DP; dtk address markers
// kept). Phase A4.5 of the native JA1 campaign (bus §362/§363/§366).
// TTrack::TNoteMgr owns the 8-voice note slots per track; the
// field_0x20 generation check (channel->field_0xc8 counter) is how the
// donor detects a slot whose channel was stolen/recycled underneath it.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept.
// Donor include map: JASTrack.h -> d/ext_seq/ja1_jastrack.h;
// JASChannel.h -> d/ext_seq/ja1_jaschannel.h; JUTAssert.h -> the
// ja1_boundary.h JUT shims.
// This TU retires the A2-support TNoteMgr stub fence in ja1_jastrack.h.
// ============================================================

#include "d/ext_seq/ja1_jastrack.h"
#include "d/ext_seq/ja1_jaschannel.h"

namespace JAudio1 {

static const int MULTI_MAX = 8;

/* 8027DD54-8027DD94       .text init__Q38JASystem6TTrack8TNoteMgrFv */
void TTrack::TNoteMgr::init() {
    mBaseTime = 0;
    mConnectCase = 0;
    mLastNote = 0;
    mBeforeTieMode = 0;
    for (int i = 0; i < MULTI_MAX; i++) {
        field_0x0[i] = NULL;
        field_0x20[i] = 0;
    }
}

/* 8027DD94-8027DDBC       .text endProcess__Q38JASystem6TTrack8TNoteMgrFv */
void TTrack::TNoteMgr::endProcess() {
    if (mBaseTime == 0xffffffff) {
        return;
    }
    if (mConnectCase != 0) {
        return;
    }
    field_0x0[0] = NULL;
}

/* 8027DDBC-8027DE78       .text setChannel__Q38JASystem6TTrack8TNoteMgrFiPQ28JASystem8TChannel */
void TTrack::TNoteMgr::setChannel(int index, TChannel* channel) {
    JUT_ASSERT(44, index >= 0);
    JUT_ASSERT(45, index < MULTI_MAX);
    field_0x0[index] = channel;
    field_0x20[index] = channel->field_0xc8;
}

/* 8027DE78-8027DF24       .text releaseChannel__Q38JASystem6TTrack8TNoteMgrFi */
void TTrack::TNoteMgr::releaseChannel(int index) {
    JUT_ASSERT(53, index >= 0);
    JUT_ASSERT(54, index < MULTI_MAX);
    field_0x0[index] = NULL;
}

/* 8027DF24-8027DFD8       .text getChannel__Q38JASystem6TTrack8TNoteMgrFi */
TChannel* TTrack::TNoteMgr::getChannel(int index) {
    JUT_ASSERT(61, index >= 0);
    if (index >= MULTI_MAX) {
        return NULL;
    }
    TChannel* channel = field_0x0[index];
    if (!channel) {
        return NULL;
    }
    if (field_0x20[index] != channel->field_0xc8) {
        field_0x0[index] = NULL;
        return NULL;
    }
    return channel;
}

}  // namespace JAudio1
