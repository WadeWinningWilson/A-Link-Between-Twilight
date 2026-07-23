#ifndef D_EXT_SEQ_SPACE_H
#define D_EXT_SEQ_SPACE_H

/**
 * Housing §52 (B) — dual-parser BGM for foreign-host stages.
 *
 * THREE COMPONENTS (ownership BEFORE parser — №89):
 *
 * 1) Space-identity gate
 *    Only foreign-host stages (`dExtWwSave_isWwHostStage`) may select the JA1
 *    path. TP stages keep JAudio2 exclusively. Silence beats a foreign sound.
 *
 * 2) Parser ownership (№89 invariant) — IMPLEMENTED AS CODE, not comments
 *    ExtSeqSpace owns every JA1 track and every JASChannel* those tracks start.
 *    On stage leave / host change it MUST stopOwned() before JAudio2 scene BGM
 *    may resume. Never hand a JA1 handle to JAudio2 teardown.
 *
 * 3) JA1 TSeqParser path
 *    Donor BMS bytes play through ExtSeq::Ja1Parser (WW dialect). Voices use
 *    §53 IBNK/WSYS/.aw residency (shadow-wave virtual addresses); ExtSeq owns
 *    every channel. Bank register/unregister nests inside stopOwned/startOwned.
 */

#include <types.h>

#if TARGET_PC

/** Poll: gate, ownership transitions, package load, JA1 tick / handoff. */
void dExtSeqSpace_poll();

/**
 * When true, stock `mDoAud_sceneBgmStart` must not run — ExtSeq owns BGM on
 * this foreign host (or is silencing until the package is ready).
 */
bool dExtSeqSpace_shouldSuppressJa2Bgm();

/**
 * Cutscene / bus handoff into the current foreign-host field BMS (island or
 * house from stage). Fades prior JA2 (`mDoAud_bgmStop`) or prior ExtSeq root
 * volume over `fadeFrames` (default 30 = WW prologue `stop(30)`), then starts.
 * Opening music source (seq vs stream) is NOT selected here — stream playback
 * stays blocked until History confirms stream.
 */
void dExtSeqSpace_requestHandoffToField(u32 fadeFrames = 30);

/**
 * §60b — offline Ja1Parser event dump (no audio). Reads `*.file=` paths from
 * packageRoot/manifest.ini, writes seq_events_engine_<stem>.csv beside the
 * package root. Returns 0 on success.
 */
int dExtSeqSpace_cliDumpEvents(const char* packageRoot);

#endif  // TARGET_PC

#endif  // D_EXT_SEQ_SPACE_H
