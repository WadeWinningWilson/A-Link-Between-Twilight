#pragma once

#include "JSystem/JAudio2/JASDSPInterface.h"

#include <array>
#include <cassert>

#include "SDL3/SDL_audio.h"
#include <span>

// ReSharper disable once CppUnusedIncludeDirective
#include "global.h"

namespace dusk::audio {
    constexpr int SampleRate = 32000;

    enum class OutputChannel : u8 {
        LEFT,
        RIGHT,
        OutputChannel_MAX
    };

    /**
     * Data stored by DSP implementation for each DSP channel.
     */
    struct ChannelAuxData {
        s16 hist1;
        s16 hist0;

        // Used for debugging tools.
        u32 resetCount;

        /**
         * Previous volume values, per output channel.
         * Used to avoid clicking when volumes change.
         * Set to NaN after channel reset, indicating that initial volume value is previous.
         */
        f32 prevVolume[static_cast<int>(OutputChannel::OutputChannel_MAX)];

        f32& PrevVolume(OutputChannel channel) {
            assert(channel < OutputChannel::OutputChannel_MAX);
            return prevVolume[static_cast<int>(channel)];
        }

        // buffer for decoding before resampling, size is chosen based on how many input samples we would need to fetch for the highest possible pitch
        // to fill one subframe of output samples after resampling
        static constexpr int DECODE_BUF_SIZE = 2048;
        s16 decodeBuf[DECODE_BUF_SIZE];
        int decodeBufCount;

        // basically stores our position between resamplePrev and decodeBuf[0] so we don't lose that fractional resampler position next subframe
        f32 resamplePos;
        // last consumed sample from decodeBuf
        s16 resamplePrev;

        // phase of oscillator channels
        u16 oscPhase;

        // low pass previous state
        f32 prev_lp_out;  // out[n-1]
        f32 prev_lp_in;   // in[n-1]

        // biquad state
        f32 biq_in1; // in[n-1]
        f32 biq_in2; // in[n-2]
        f32 biq_out1; // out[n-1]
        f32 biq_out2; // out[n-2]
    };

    extern ChannelAuxData ChannelAux[DSP_CHANNELS];

    /**
     * Data storage for a single subframe and output channel's worth of samples.
     */
    using DspSubframe = std::array<f32, DSP_SUBFRAME_SIZE>;

    /**
     * Data storage for a single subframe's worth of samples, across all output channels.
     */
    struct OutputSubframe {
        static constexpr int NUM_CHANNELS = static_cast<int>(OutputChannel::OutputChannel_MAX);

        std::array<DspSubframe, NUM_CHANNELS> channels;

        DspSubframe& operator[](OutputChannel channel) {
            assert(channel < OutputChannel::OutputChannel_MAX);
            return channels[static_cast<int>(channel)];
        }
    };

    /**
     * Initialize the DSP system, creating data storage needed for channels and such.
     */
    void DspInit();

    /**
     * Render a subframe of audio with the current DSP state.
     */
    void DspRender(OutputSubframe& subframe);

    /**
     * Get the amount of samples a single "block" of this channel's data has.
     */
    constexpr u32 BlockSamples(const JASDsp::TChannel& channel) {
        return channel.mSamplesPerBlock;
    }

    /**
     * Get the amount of bytes a single "block" of this channel's data has.
     */
    constexpr u32 BlockBytes(const JASDsp::TChannel& channel) {
        if (channel.mSamplesPerBlock == 1) {
            if (channel.mBytesPerBlock == 16) {
                return 2;
            }
            if (channel.mBytesPerBlock == 8) {
                return 1;
            }

            CRASH("Unknown format");
        }

        return channel.mBytesPerBlock;
    }

    /**
     * Apply a volume level to audio data.
     * Interpolates across the two provided volume levels to avoid clicking.
     */
    void ApplyVolume(std::span<f32> dst, std::span<f32> src, f32 startVolume, f32 endVolume);

    extern f32 MasterVolume;
    extern f32 PrevMasterVolume;
    extern bool EnableReverb;
    extern bool DumpAudio;
    extern bool EnableHrtf;
    extern f32 HrtfGain;

    void setHurricaneSpinSeLoopActive(bool active);
    void registerHurricaneSpinWaveAram(u32 aramAddress);
    void excludeHurricaneSpinWaveAram(u32 aramAddress);
    void clearHurricaneSpinWaveRegistration();
    bool isHurricaneSpinWaveExcluded(u32 aramAddress);

    // ============================================
    // NEW CODE — ALBW Port / Dusklight
    // Custom-audio shadow-wave redirect.
    //
    // Lets a downloaded audio mod be toggled ON/OFF at RUNTIME (not just at boot):
    // vanilla wave banks stay resident in ARAM as the untouched base, the mod's
    // byte-compatible twin of each resident bank is held in a plain RAM buffer,
    // and the live toggle picks which one the mixer decodes from. It is a pointer
    // swap at the sample-fetch point in ReadChannelSamplesChunk() — no audio
    // subsystem re-init, no section-heap rebuild, the audio thread is never
    // touched. Both sets are always resident, so the toggle is instant.
    //
    // Registry is a plain fixed array read lock-free on the audio thread, exactly
    // mirroring the hurricane-spin registration convention above (register/free
    // happen rarely on the DVD/load thread; a torn read at worst blips one frame
    // between vanilla and mod, which is inaudible).
    // ============================================

    // Kill switch: set to 0 to compile the whole shadow feature out (mixer then
    // always decodes from vanilla ARAM, exactly as stock). Shared by DuskDsp.cpp,
    // custom_assets.cpp, and the JASWaveArcLoader hooks.
#ifndef D_ALBW_AUDIO_SHADOW
#define D_ALBW_AUDIO_SHADOW 1
#endif

    // ============================================
    // NEW CODE — ALBW Port (toggle-corruption fix, 2026-07-11)
    // Mod waves live in a VIRTUAL address space: remap_voice hands a new note
    // kShadowVirtualBase + aramBase + modOffset instead of a raw in-bank ARAM
    // address. The mixer then routes each fetch BY ADDRESS ALONE — virtual =
    // mod buffer, real = vanilla ARAM — with no global flag consulted at fetch
    // time. Why: the old design gated fetches on setShadowActive(), so a
    // mid-session toggle re-routed IN-FLIGHT voices to the other data set at
    // their now-meaningless offsets (mod offsets read against vanilla bytes and
    // vice versa) → audibly corrupted sustained/looping notes until they died.
    // With per-note address latching, live voices keep their correct source
    // forever (mod twins are pooled for the run, never freed) and the active
    // flag only gates NEW noteOns (checked in custom_assets::remap_voice).
    // ARAM is ≤ ~24 MB, so 0x40000000 can never collide with a real address.
    // ============================================
    constexpr u32 kShadowVirtualBase = 0x40000000u;

    // №31-B: ExtSeq stamps JASChannel::mBankDisposeID with this address so the
    // TP mix path can detect WW-owned voices without a d/→JSystem dependency.
    extern char g_extSeqOwnerTag;
    inline bool isExtSeqOwned(const void* bankDisposeId) {
        return bankDisposeId == &g_extSeqOwnerTag;
    }

    /**
     * §81 — WW type-7 Fxline → freeverb + ExtSeq AutoMixer send.
     * scene: 0 = field (F_DL*), 1 = interior (R_DL*). Donor Fx::init scene index.
     * busGain01 / room01 / damp01 are derived from FxlineConfig_ bus/coefs/buf sizes.
     */
    void applyExtSeqFxScene(u8 scene, f32 busGain01, f32 room01, f32 damp01);
    /** Current ExtSeq→reverb send (0..1), applied as initFxmix on ExtSeq voices. */
    f32 getExtSeqFxSend();
    /** SoundTable BGM vol_u8/127 master (default 1 until control table loads). */
    void setExtSeqMasterVol(f32 vol01);
    f32 getExtSeqMasterVol();

    // Register a resident bank's mod twin: [aramBase, aramBase+size) -> buf
    // (addressed by new notes as kShadowVirtualBase + aramBase + offset).
    void registerShadowWave(u32 aramBase, u32 size, const u8* buf);
    // Drop a bank's mod twin (called when the vanilla bank is erased).
    void unregisterShadowWave(u32 aramBase);
    // Flip whether NEW notes remap to the mod set (follows the Custom Models
    // toggle; in-flight notes keep the source their address latched at noteOn).
    void setShadowActive(bool active);
    bool shadowActive();
    // VIRTUAL address (>= kShadowVirtualBase) -> equivalent pointer into the
    // mod twin buffer; REAL address -> nullptr ("decode from vanilla ARAM").
    // A stale virtual address (bank erased mid-note) also returns nullptr —
    // the fetch site must then SILENCE the voice, never fall back to ARAM
    // (the virtual offset would be out of bounds there).
    const u8* resolveShadowWave(u32 waveAramAddress);
}
