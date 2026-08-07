// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §369 A4 — gate-ON native player entry points (task 5). This is
// port-side integration code (no donor TU): it performs the donor's
// BOOT ORDER and SEQUENCE-START CALL CHAIN using only ported donor
// rows, and is a hard no-op while DUSK_JA1_NATIVE == 0.
//
// Donor boot order followed (§369):
//   Kernel::init            -> Kernel::resetCallback();
//                              Calc::initSinfT();
//     (donor JASAiCtrl.cpp:28/31 — §370 A4.5: initSinfT is the donor
//      leg that builds the sinfT/sinfDolby2 tables now that the real
//      table-driven JASCalc.cpp is ported; the remaining donor legs
//      initSystem/portCmdInit are the GC AI/DSP bring-up the PC driver
//      already did)
//   Driver::init            -> TDSPChannel::initAll();
//                              TGlobalChannel::init();
//     (donor JASDriverIF.cpp:16-19 — its DSPInterface::initBuffer()
//      leg is the shared PC voice array DuskDsp already initialized)
//   JAIBasic::initDriver    -> TTrack::newMemPool(0x100)
//     (donor JAIBasic.cpp:154, systemTrackMax retail = 0x100; the
//      donor's registerSeqCallback(setParameterSeqSync) leg is the
//      unported JAI parameter-sync layer — sCallBackFunc stays NULL,
//      which TTrack::mainProc handles as the donor no-callback path)
//   JAIBankWave::init       -> ExtSeq::ja1Native_registerBanks()
//     (ja1_bank.cpp — donor BankMgr::init/registBankBNK/assignWaveBank
//      idiom over the bridge-loaded slices)
//   heartbeat               -> JAudio1::ja1Kernel_installHeartbeat()
//     (ja1_kernel.cpp — donor TDSP_DACBuffer::updateDSP chain on the
//      PC subframe pump)
//
// Donor sequence-start call chain (the only start idiom JASTrack.cpp
// exposes; no other donor TU calls these symbols — the JAI layer above
// them is decomp-empty):
//   TTrack* root = new TTrack;      // donor pool operator new
//                                   //  (memory from newMemPool)
//   root->setSeqData(bms, size, 0); // init() + mSeqCtrl.start(bms,0)
//                                   //  + updateTrackAll(); state -> 2
//   root->startSeq();               // state -> 1 + Kernel::
//                                   //  registerSubFrameCallback(
//                                   //  rootCallback, root) — the §365
//                                   //  governor now runs per subframe
// Stop: root->stopSeq() — state -> 3; the next rootCallback tick runs
// stopSeqMain() (updateSeq + close: noteOffAll + releaseChannelAll)
// and deregisters by returning -1. Voice teardown is therefore async
// on the audio clock, the donor's own semantics; the bridge's
// unregister path is UAF-safe against it (DuskDsp resolveShadowWave
// silences stale virtual addresses; the Wave_::field_0x24 residency
// flag force-stops survivors — see ja1_bank.cpp).
//
// Threading (§369): start/stop take JASCriticalSection — the same
// recursive mutex the SDL audio pump holds across a rendered frame —
// so the whole start chain is atomic against the subframe heartbeat.
// ============================================================

#include "d/ext_seq/ja1_native.h"

#include "d/ext_seq/ja1_boundary.h"
#include "d/ext_seq/ja1_dsp_boundary.h"
#include "d/ext_seq/ja1_jasbank.h"
#include "d/ext_seq/ja1_jaschannel.h"
#include "d/ext_seq/ja1_jastrack.h"

#include "JSystem/JAudio2/JASCriticalSection.h"
#include "dolphin/os.h"

namespace ExtSeq {

namespace {

JAudio1::TTrack* s_nativeRoot = NULL;
bool s_nativeInited = false;

/**
 * §369 one-time gate-ON boot (donor order — see TU header).
 */
bool ja1NativeInitOnce() {
    if (s_nativeInited) {
        return true;
    }
    JAudio1::Kernel::resetCallback();      // donor Kernel::init leg (JASAiCtrl.cpp:28)
    JAudio1::Calc::initSinfT();            // ===== §370 A4.5: donor Kernel::init leg (JASAiCtrl.cpp:31) — builds the real sinfT/sinfDolby2 tables
    JAudio1::TDSPChannel::initAll();       // donor Driver::init leg (JASDriverIF.cpp:17)
    JAudio1::TGlobalChannel::init();       // donor Driver::init leg (JASDriverIF.cpp:18)
    JAudio1::TTrack::newMemPool(0x100);    // donor JAIBasic.cpp:154 (systemTrackMax retail)
    if (!ja1Native_registerBanks()) {      // donor JAIBankWave::init idiom (ja1_bank.cpp)
        OSReport("ExtSeq §369: native bank registration failed — native start aborted\n");
        return false;
    }
    if (!JAudio1::ja1Kernel_installHeartbeat()) {  // donor updateDSP chain on the PC pump
        OSReport("ExtSeq §369: heartbeat install failed — native start aborted\n");
        return false;
    }
    s_nativeInited = true;
    return true;
}

}  // namespace

bool ja1Native_start(const u8* bms, u32 size) {
    if (!DUSK_JA1_NATIVE) {
        return false;  // §369 THE GATE — inert gate-OFF
    }
    if (bms == NULL || size == 0) {
        return false;
    }
    JASCriticalSection cs;  // §369: atomic vs the subframe heartbeat
    if (!ja1NativeInitOnce()) {
        return false;
    }
    if (s_nativeRoot == NULL) {
        // donor pool allocation (TTrack::operator new draws from the
        // newMemPool free list; its TOuterParam pairing from
        // newMemPool's assignExtBuffer persists — donor pattern).
        s_nativeRoot = new JAudio1::TTrack();
        if (s_nativeRoot == NULL) {
            OSReport("ExtSeq §369: TTrack pool empty — native start aborted\n");
            return false;
        }
    } else {
        // Restart on the SAME root (donor setSeqData begins with
        // init()); make sure a prior run is closed first.
        s_nativeRoot->stopSeq();
        if (s_nativeRoot->field_0x37e != 0) {
            s_nativeRoot->stopSeqMain();  // donor stop path body (synchronous close)
        }
    }
    // donor player-start idiom (see TU header). setSeqData's trailing
    // args are ignored by the donor body (donor signature kept).
    if (!s_nativeRoot->setSeqData(const_cast<u8*>(bms), (s32)size, 0)) {
        return false;
    }
    if (!s_nativeRoot->startSeq()) {
        return false;
    }
    OSReport("ExtSeq §369: native JA1 player started (seq %u bytes)\n", size);
    return true;
}

void ja1Native_stop(const char* reason) {
    if (!DUSK_JA1_NATIVE) {
        return;  // §369 THE GATE
    }
    JASCriticalSection cs;
    if (s_nativeRoot == NULL) {
        return;
    }
    // donor stop: state -> 3; the next audio-clock tick runs
    // stopSeqMain (noteOffAll + releaseChannelAll) and deregisters.
    s_nativeRoot->stopSeq();
    OSReport("ExtSeq §369: native JA1 player stop requested (%s)\n",
             reason != NULL ? reason : "?");
}

bool ja1Native_active() {
    if (!DUSK_JA1_NATIVE) {
        return false;
    }
    return s_nativeRoot != NULL && s_nativeRoot->field_0x37e == 1;
}

}  // namespace ExtSeq
