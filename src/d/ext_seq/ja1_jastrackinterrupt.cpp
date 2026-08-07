// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASTrackInterrupt.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 track interrupts — donor-verbatim port of WW retail
// JSystem/JAudio/JASTrackInterrupt.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept; the donor's own /* Nonmatching */ marker on resetInter
// is carried). Phase A4.5 of the native JA1 campaign (bus
// §362/§363/§366). TIntrMgr is the per-track software-interrupt table:
// setIntr arms a vector (a BMS offset), request latches it, checkIntr
// pops the lowest pending one for TTrack::tryInterrupt's seqCtrl
// callIntr jump, timerProcess drives the REQUEST_UNK_6 timer vector.
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept (RequestId enum lives in
// ja1_boundary.h since A2).
// Donor include map: JASTrackInterrupt.h -> d/ext_seq/ja1_boundary.h
// (the donor-verbatim class layout lives there since A2).
// This TU retires the A2-support TIntrMgr stub fence in ja1_boundary.h.
// ============================================================

#include "d/ext_seq/ja1_boundary.h"

namespace JAudio1 {

/* 80284118-80284158       .text init__Q28JASystem8TIntrMgrFv */
void TIntrMgr::init() {
    field_0x0 = 1;
    field_0x1 = 0;
    field_0x2 = 0;
    field_0x3 = 0;
    field_0x4 = 0;
    field_0x8 = 0;
    for (int i = 0; i < 8; i++) {
        field_0xc[i] = NULL;
    }
}

/* 80284158-8028417C       .text request__Q28JASystem8TIntrMgrFUl */
void TIntrMgr::request(u32 param_1) {
    if ((field_0x2 & 1 << param_1) == 0) {
        return;
    }
    field_0x1 |= 1 << param_1;
}

/* 8028417C-802841A0       .text setIntr__Q28JASystem8TIntrMgrFUlPv */
void TIntrMgr::setIntr(u32 param_1, void* param_2) {
    field_0x2 |= 1 << param_1;
    field_0xc[param_1] = param_2;
}

/* 802841A0-802841B8       .text resetInter__Q28JASystem8TIntrMgrFUl */
void TIntrMgr::resetInter(u32 param_1) {
    /* Nonmatching */
    field_0x2 &= ~(1 << param_1);
}

/* 802841B8-80284224       .text checkIntr__Q28JASystem8TIntrMgrFv */
void* TIntrMgr::checkIntr() {
    if (field_0x0 == 0) {
        return NULL;
    }
    u32 r4 = field_0x2 & field_0x1;
    for (u32 i = 0; r4; i++) {
        if (r4 & 1) {
            field_0x1 &= ~(1 << i);
            return field_0xc[i];
        }
        r4 >>= 1;
    }
    return NULL;
}

/* 80284224-802842A8       .text timerProcess__Q28JASystem8TIntrMgrFv */
void TIntrMgr::timerProcess() {
    if (field_0x4 == 0) {
        return;
    }
    field_0x4--;
    if (field_0x4 != 0) {
        return;
    }
    request(REQUEST_UNK_6);
    if (field_0x3) {
        field_0x3--;
        if (field_0x3) {
            field_0x4 = field_0x8;
        }
    } else {
        field_0x4 = field_0x8;
    }
}

}  // namespace JAudio1
