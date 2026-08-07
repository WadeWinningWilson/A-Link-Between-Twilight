#ifndef D_EXT_SEQ_JA1_NATIVE_H
#define D_EXT_SEQ_JA1_NATIVE_H

// ============================================================
// §369 A4 — THE GATE + gate-ON native-player entry points.
//
// DUSK_JA1_NATIVE (pattern: d_a_knob00.h §329 gate): default 0 — the
// ext_seq bridge keeps playing exactly as today; every A4 surface is
// compiled ALWAYS (both branches) but runs only when the gate flips.
// Gate-ON: the vanilla JAudio1 stack (TTrack + TSeqParser + TChannel +
// TOscillator + bank layer, §363/§367/§368) becomes the playing
// system, clocked by the donor subframe heartbeat:
//   PC audio pump (RenderAudioSubframe -> JASDriver::updateDSP, once
//   per 80-sample DSP subframe = DacRate/80) -> JA1 kernel pump
//   (ja1_kernel.cpp) -> Kernel::subframeCallback / TDSPChannel::
//   updateAll / Kernel::aiCallback (donor TDSP_DACBuffer::updateDSP
//   order) -> TTrack::rootCallback governor (field_0x364/0x368
//   accumulator) on the donor's clock domain.
// A5 flips the gate after the ear + Foundry differ gate; §364's
// wall-clock bridge stepping is reverted at the flip (parent's call).
// ============================================================

#include "dolphin/types.h"

#ifndef DUSK_JA1_NATIVE
// §371 (A5 gate flip, 2026-08-03): native ON — WW's own JAudio1 stack plays.
// Kill switch: set 0 → the ext_seq bridge byte-identical. Bridge + §364 strip
// only AFTER the ear + differ gate passes.
// §369: default OFF — bridge remains the playing system.
#define DUSK_JA1_NATIVE 1  // §372: re-flipped — root was the GC-pointer-size bzero (half-zeroed inst table), fixed
#endif

namespace JAudio1 {
class TWaveBank;
}

namespace ExtSeq {

// ============================================================
// §369 gate-ON entry points (bodies: src/d/ext_seq/ja1_native.cpp).
// Every function is a hard no-op / false while DUSK_JA1_NATIVE == 0,
// so calling them unconditionally from the bridge is inert gate-OFF.
// ============================================================

/**
 * Start the vanilla JA1 player on a BMS buffer (gate-ON only).
 * First call performs the donor boot order (Kernel::resetCallback ->
 * TDSPChannel::initAll -> TGlobalChannel::init -> TTrack::newMemPool)
 * plus bank registration (ja1Native_registerBanks) and installs the
 * subframe heartbeat. Returns false gate-OFF or on init failure.
 */
bool ja1Native_start(const u8* bms, u32 size);

/** Stop the vanilla player (donor stopSeq path; async voice teardown). */
void ja1Native_stop(const char* reason);

/** True while the native root track is running (gate-ON only). */
bool ja1Native_active();

/**
 * §369 wave/bank supply (body: src/d/ext_seq/ja1_bank.cpp — the TU
 * that already owns the bridge-loaded IBNK/WSYS/.aw buffers).
 * Registers the bridge's parsed banks with the donor BankMgr
 * (host-endian IBNK image -> BNKParser::createBasicBank) and assigns
 * the ExtSeq-backed JAudio1::TWaveBank objects (donor JAIBankWave
 * BankMgr::init/registBankBNK/assignWaveBank idiom). Reuses the
 * bridge's loaded buffers — no arc is re-mounted.
 */
bool ja1Native_registerBanks();

/**
 * §369: the ExtSeq-backed data source behind the donor
 * JAudio1::WaveBankMgr::getWaveBank row (wsIndex = donor wave-bank
 * number; NULL = donor "no wave bank registered").
 */
JAudio1::TWaveBank* ja1NativeGetWaveBank(int wsIndex);

}  // namespace ExtSeq

namespace JAudio1 {

/**
 * §369 heartbeat install (body: src/d/ext_seq/ja1_kernel.cpp).
 * Registers the JA1 kernel pump on the PC driver's DSP-sync callback
 * list (JASDriver::registerDspSyncCallback — dispatched inside
 * JASDriver::updateDSP once per audio subframe). Idempotent.
 */
bool ja1Kernel_installHeartbeat();

}  // namespace JAudio1

#endif /* D_EXT_SEQ_JA1_NATIVE_H */
