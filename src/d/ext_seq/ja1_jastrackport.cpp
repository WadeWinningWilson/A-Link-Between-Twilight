// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASTrackPort.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================
// §370 JAudio1 track ports — donor-verbatim port of WW retail
// JSystem/JAudio/JASTrackPort.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept; the donor's own /* Nonmatching */ marker on init is
// carried). Phase A4.5 of the native JA1 campaign (bus §362/§363/§366).
// TTrackPort is the 16-slot game<->sequence mailbox (cmdReadPort/
// cmdWritePort on the BMS side, writePortAppDirect on the game side).
// Namespace convention (§363/§367): donor JASystem::* flattens into
// JAudio1::* with donor symbol names kept.
// Donor include map: JASTrackPort.h -> d/ext_seq/ja1_boundary.h (the
// donor-verbatim class layout lives there since A2); JUTAssert.h ->
// the ja1_boundary.h JUT shims.
// This TU retires the A2-support TTrackPort stub fence in
// ja1_boundary.h.
// ============================================================

#include "d/ext_seq/ja1_boundary.h"

namespace JAudio1 {

const u32 MAX_PORTS = 16;

/* 802842A8-802842E8       .text init__Q28JASystem10TTrackPortFv */
void TTrackPort::init() {
    /* Nonmatching */
    for (int i = 0; i < 16; i++) {
        mImportFlag[i] = 0;
        mExportFlag[i] = 0;
        mValue[i] = 0;
    }
}

/* 802842E8-8028437C       .text readImport__Q28JASystem10TTrackPortFi */
u16 TTrackPort::readImport(int port_num) {
    JUT_ASSERT(31, port_num >= 0 && port_num < MAX_PORTS);
    mImportFlag[port_num] = 0;
    return mValue[port_num];
}

/* 8028437C-80284414       .text readExport__Q28JASystem10TTrackPortFi */
u16 TTrackPort::readExport(int port_num) {
    JUT_ASSERT(38, port_num >= 0 && port_num < MAX_PORTS);
    mExportFlag[port_num] = 0;
    return mValue[port_num];
}

/* 80284414-802844AC       .text writeImport__Q28JASystem10TTrackPortFiUs */
void TTrackPort::writeImport(int port_num, u16 value) {
    JUT_ASSERT(45, port_num >= 0 && port_num < MAX_PORTS);
    mImportFlag[port_num] = 1;
    mValue[port_num] = value;
}

/* 802844AC-80284548       .text writeExport__Q28JASystem10TTrackPortFiUs */
void TTrackPort::writeExport(int port_num, u16 value) {
    JUT_ASSERT(51, port_num >= 0 && port_num < MAX_PORTS);
    mExportFlag[port_num] = 1;
    mValue[port_num] = value;
}

}  // namespace JAudio1
