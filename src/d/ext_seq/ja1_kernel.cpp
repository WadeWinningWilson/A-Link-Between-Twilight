// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASRate.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASCallback.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASDriverIF.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASDSPBuf.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §369 JAudio1 kernel/driver rows + THE HEARTBEAT — Phase A4 of the
// native JA1 campaign (bus §362/§363/§366; History lane §369).
// Donor TU map (§369 named adaptation "TU consolidation", A3 precedent):
//   [1] JASRate.cpp      — the retail rate rows the boundary deferred
//                          (getters that have a live PC counterpart map
//                          onto the PC driver — §363 authority rule,
//                          see ja1_boundary.h)
//   [2] JASCallback.cpp  — donor-verbatim subframe-callback registry
//                          (the list TTrack::startSeq registers
//                          rootCallback on)
//   [3] JASDriverIF.cpp  — the config rows the channel layer consumes
//                          (JAS_SYSTEM_OUTPUT_MODE / JAS_UPDATE_INTERVAL /
//                          MAX_MIXERLEVEL / MAX_AUTOMIXERLEVEL)
//   [+] THE HEARTBEAT    — the PC realization of the donor pump site
//                          TDSP_DACBuffer::updateDSP (JASDSPBuf.cpp:56):
//                          on GC it runs once per 80-sample DSP subframe
//                          (snIntCount = gSubFrames per DAC frame) and
//                          calls, in order: DSPInterface::invalChannelAll
//                          -> Kernel::subframeCallback -> TDSPChannel::
//                          updateAll -> Kernel::aiCallback. The PC audio
//                          pump with the SAME clock domain is
//                          JASDriver::updateDSP (libs/JSystem/src/JAudio2/
//                          JASAiCtrl.cpp:155), called once per subframe
//                          from RenderAudioSubframe (src/dusk/audio/
//                          DuskAudioSystem.cpp:142). The JA1 chain hooks
//                          its DSP-SYNC callback list
//                          (JASDriver::registerDspSyncCallback ->
//                          dispatched at JASAiCtrl.cpp:165, inside the
//                          same update window JA2's own voice update
//                          runs in). invalChannelAll is already issued
//                          by JASDriver::updateDSP for the whole shared
//                          voice array — not repeated here.
// Namespace convention matches §363/§367/§368: donor JASystem::*
// flattens into JAudio1::*, donor symbol names kept.
// GATE: nothing here runs until ja1Kernel_installHeartbeat() is called
// from the gate-ON init (ja1_native.cpp). Gate-OFF this TU is inert
// data + unreferenced bodies.
// ============================================================

#include "d/ext_seq/ja1_native.h"
#include "dusk/logging.h"  // §373 pipeline probes

#include "d/ext_seq/ja1_boundary.h"
#include "d/ext_seq/ja1_dsp_boundary.h"

#include "JSystem/JAudio2/JASCriticalSection.h"  // ===== §369: PC mutual exclusion (see note at resetCallback)
#include "JSystem/JAudio2/JASDriverIF.h"         // ===== §369: JASDriver::registerDspSyncCallback (the PC hook)
#include "dolphin/os.h"

namespace JAudio1 {

// ============================================================
// §369 donor TU [1]: JASRate.cpp — the rows with no PC counterpart
// (zero-init retail). gDacRate/gSubFrames/gFrameSamples/gDacSize are
// NOT re-hosted: their getters map onto the live PC driver
// (ja1_boundary.h, §363 authority rule — one clock, the DSP's).
// ============================================================
int Kernel::gOutputRate;
int Kernel::gAiSetting;

// ============================================================
// §369 donor TU [2]: JASCallback.cpp — donor-verbatim.
// PC adaptations (each marked §369):
//   - `new (JASDram, 0)` -> plain new (A2/A3 heap precedent).
//   - OSDisableInterrupts is a PC no-op (src/dusk/OSThread.cpp:581);
//     the receiver's audio-thread mutual-exclusion facility is
//     JASCriticalSection (the same recursive mutex the SDL audio pump
//     holds across RenderNewAudioFrame). Each donor interrupt window
//     therefore ALSO takes a JASCriticalSection — the donor calls are
//     kept so the body text stays donor-shaped.
// ============================================================

u32 Kernel::maxCallbacksUser = 16;
Kernel::unk_callList* Kernel::callList;
bool Kernel::callbackInit;

/* 8027BA70-8027BB24       .text resetCallback__Q28JASystem6KernelFv */
void Kernel::resetCallback() {
    if (callbackInit != true) {
        JASCriticalSection cs;  // ===== §369: PC mutual exclusion (see TU header)
        callList = new unk_callList[maxCallbacksUser];  // ===== §369: donor `new (JASDram, 0)`
        JUT_ASSERT(58, callList != NULL);
        BOOL enable = OSDisableInterrupts();
        for (int i = 0; i < (int)maxCallbacksUser; i++) {  // ===== §369: donor `int i` vs u32 bound — cast silences x64 MSVC, donor trip count kept
            callList[i].field_0x0 = NULL;
        }
        callbackInit = true;
        OSRestoreInterrupts(enable);
    }
}

/* 8027BB24-8027BB8C       .text checkCallback__Q28JASystem6KernelFPFPv_lPv */
int Kernel::checkCallback(s32 (*param_1)(void*), void* param_2) {
    if (callbackInit == false) {
        return -1;
    }
    for (int i = 0; i < (int)maxCallbacksUser; i++) {
        if (callList[i].field_0x0 == param_1 && callList[i].field_0x4 == param_2) {
            return i;
        }
    }
    return -1;
}

/* 8027BB8C-8027BC24       .text registerDspCallback__Q28JASystem6KernelFPFPv_lPv */
int Kernel::registerDspCallback(s32 (*param_1)(void*), void* param_2) {
    if (callbackInit == false) {
        return -1;
    }
    JASCriticalSection cs;  // ===== §369: PC mutual exclusion
    BOOL enable = OSDisableInterrupts();
    int r30 = registerSubFrameCallback(param_1, param_2);
    if (r30 == -1) {
        // ===== §369: donor `#if VERSION > VERSION_DEMO` restore — the
        // receiver builds the retail lineage (VERSION=0 is TP's own
        // scheme, not WW's demo); restore kept unconditional.
        OSRestoreInterrupts(enable);
        return -1;
    };
    callList[r30].field_0x8 = 1;
    OSRestoreInterrupts(enable);
    return r30;
}

/* 8027BC24-8027BD14       .text registerSubFrameCallback__Q28JASystem6KernelFPFPv_lPv */
int Kernel::registerSubFrameCallback(s32 (*param_1)(void*), void* param_2) {
    if (callbackInit == false) {
        return -1;
    }
    JASCriticalSection cs;  // ===== §369: PC mutual exclusion
    int idx;
    for (idx = 0; idx < (int)maxCallbacksUser; idx++) {
        if (callList[idx].field_0x0 == NULL) {
            break;
        }
    }
    if (idx == (int)maxCallbacksUser) {
        OSReport("[JASKernel::registerAiCallbak] コールバック登録バッファが一杯です。\n");
        return -1;
    }
    if (checkCallback(param_1, param_2) != -1) {
        return -1;
    }
    BOOL enable = OSDisableInterrupts();
    callList[idx].field_0x0 = param_1;
    callList[idx].field_0x4 = param_2;
    callList[idx].field_0x8 = 0;
    OSRestoreInterrupts(enable);
    return idx;
}

/* 8027BD14-8027BDAC       .text aiCallback__Q28JASystem6KernelFv */
void Kernel::aiCallback() {
    if (callbackInit) {
        for (int i = 0; i < (int)maxCallbacksUser; i++) {
            if (callList[i].field_0x0 && callList[i].field_0x8 == 0) {
                int result = callList[i].field_0x0(callList[i].field_0x4);
                if (result == -1) {
                    callList[i].field_0x0 = NULL;
                }
            }
        }
    }
}

/* 8027BDAC-8027BE44       .text subframeCallback__Q28JASystem6KernelFv */
void Kernel::subframeCallback() {
#if TARGET_PC
    // §373a pipeline probe (clock stage) — strip at A5 acceptance.
    { static u64 n = 0; if (n == 0 || (n % 2000) == 0) DuskLog.info("[JA1] §373a heartbeat fire #{}", n); ++n; }
#endif
    if (callbackInit) {
        for (int i = 0; i < (int)maxCallbacksUser; i++) {
            if (callList[i].field_0x0 && callList[i].field_0x8 == 1) {
                int result = callList[i].field_0x0(callList[i].field_0x4);
                if (result == -1) {
                    callList[i].field_0x0 = NULL;
                }
            }
        }
    }
}

// ============================================================
// §369 donor TU [3]: JASDriverIF.cpp config rows — donor-verbatim
// variables + getters (retail inits; see ja1_dsp_boundary.h).
// ============================================================

u16 Driver::MAX_MIXERLEVEL = 0x2ee0;

/* 8028AAC4-8028AACC       .text getChannelLevel__Q28JASystem6DriverFv */
u16 Driver::getChannelLevel() {
    return MAX_MIXERLEVEL;
}

u16 Driver::MAX_AUTOMIXERLEVEL = 0x7fff;

/* 8028AACC-8028AAD4       .text getAutoLevel__Q28JASystem6DriverFv */
u16 Driver::getAutoLevel() {
    return MAX_AUTOMIXERLEVEL;
}

u32 Driver::JAS_SYSTEM_OUTPUT_MODE = 1;

/* 8028AAD4-8028AADC       .text setOutputMode__Q28JASystem6DriverFUl */
void Driver::setOutputMode(u32 param_1) {
    JAS_SYSTEM_OUTPUT_MODE = param_1;
}

/* 8028AADC-8028AAE4       .text getOutputMode__Q28JASystem6DriverFv */
u32 Driver::getOutputMode() {
    return JAS_SYSTEM_OUTPUT_MODE;
}

u8 Driver::JAS_UPDATE_INTERVAL = 1;

/* 8028AAE4-8028AAEC       .text getUpdateInterval__Q28JASystem6DriverFv */
u8 Driver::getUpdateInterval() {
    return JAS_UPDATE_INTERVAL;
}

// ============================================================
// §369 THE HEARTBEAT — PC realization of donor TDSP_DACBuffer::
// updateDSP (JASDSPBuf.cpp:56). Chain order is the donor's, exactly:
//   Kernel::subframeCallback();   // dsp-flagged callbacks (field_0x8==1)
//   TDSPChannel::updateAll();     // JA1 voice-slot ager over the slice
//   Kernel::aiCallback();         // subframe callbacks (field_0x8==0)
//                                 //   = TTrack::rootCallback governor
// (donor's leading DSPInterface::invalChannelAll is already issued for
// the whole shared voice array by JASDriver::updateDSP — not repeated.)
// Cadence: once per JASDriver::updateDSP call = once per 80-sample DSP
// subframe = DacRate/80 (~400 Hz @32 kHz) — the donor's clock domain
// for the field_0x364/0x368 tempo accumulator (§365).
// Runs on the SDL audio thread inside the JASCriticalSection window
// RenderNewAudioFrame holds — same mutual exclusion JA2's own voice
// update enjoys.
// ============================================================

static s32 ja1SubFramePump(void*) {
    Kernel::subframeCallback();
    TDSPChannel::updateAll();
    Kernel::aiCallback();
    return 0;  // never self-deregisters (JASCallbackMgr removes on <0)
}

bool ja1Kernel_installHeartbeat() {
    if (!DUSK_JA1_NATIVE) {
        return false;  // §369 THE GATE — inert gate-OFF
    }
    static bool s_installed = false;
    if (s_installed) {
        return true;
    }
    // §369: registered on the DSP-SYNC list (JASAiCtrl.cpp:165) so the
    // JA1 chain runs inside JASDriver::updateDSP's own update window,
    // once per subframe — the donor pump position.
    s_installed = JASDriver::registerDspSyncCallback(ja1SubFramePump, NULL);
    return s_installed;
}

}  // namespace JAudio1

// ===== §374d (strip at A5): mixer-write value probe — first 24 writes.
namespace JAudio1 {
void ja1DspProbe374d(const char* what, int a, int b, int c) {
    static int n = 0;
    if (n < 24) {
        ++n;
        DuskLog.info("[JA1] §374d {} a={} b={} c={}", what, a, b, c);
    }
}
}  // namespace JAudio1
