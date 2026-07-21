#ifndef D_EXT_SEQ_JA1_BANK_H
#define D_EXT_SEQ_JA1_BANK_H

/**
 * §53 — WW IBNK/WSYS/.aw residency for ExtSeq.
 *
 * .aw bytes live in ExtSeq-owned RAM and are published into DuskDsp's virtual
 * wave address space (registerShadowWave). Wave addresses minted at noteOn are
 * kShadowVirtualBase + virtBase + wsysOffset — never real ARAM.
 *
 * Lifetime nests inside §52 ownership: register after stopOwned, unregister
 * only after every owned voice is released.
 */

#include <filesystem>

class JASChannel;

namespace JASDsp {
struct TChannel;
}

namespace ExtSeq {

/** Load aaf_slices + banks from a §52b package root. Idempotent. */
bool ja1Bank_loadPackage(const std::filesystem::path& packageRoot);

/** True once slices + .aw buffers are parsed and ready to register. */
bool ja1Bank_ready();

/**
 * Publish .aw buffers into the shadow-wave registry.
 * Call only after stopOwned has released every ExtSeq voice.
 */
void ja1Bank_register();

/**
 * Drop shadow-wave slots. Call only after every owned voice is released
 * (stale virtual addr → silence; never free buffers while notes sound).
 */
void ja1Bank_unregister();

/** Release buffers (after unregister). Package must reload to play again. */
void ja1Bank_clear();

/**
 * Real WAVE noteOn using IBNK region + WSYS/.aw. Returns nullptr on miss
 * (caller may silence — №91). ExtSeq owns the returned channel.
 */
JASChannel* ja1Bank_noteOn(u8 bank, u8 prog, u8 key, u8 vel, u16 prio,
                           void (*cb)(u32, JASChannel*, JASDsp::TChannel*, void*),
                           void* cbArg);

/**
 * №31-B — WW velocity→volume curve (BankMgr noteOnOsc / TChannel field_0x5c):
 *   field_0x5c = initVol * (vel/127)²
 * Bakes that into mSoundParams and sets velocity to 127 so TP's per-frame
 * re-square is identity. Marks the channel ExtSeq-owned for the mix path.
 */
void ja1Bank_applyWwVelocityCurve(JASChannel* ch, f32 initVol, u8 vel);

/** True if this channel was started by ExtSeq (WW mix path owns it). */
bool ja1Bank_isExtSeqChannel(const JASChannel* ch);

}  // namespace ExtSeq

#endif  // D_EXT_SEQ_JA1_BANK_H
