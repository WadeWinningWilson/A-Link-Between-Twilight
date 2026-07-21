#ifndef D_EXT_SEQ_JA1_EVENT_DUMP_H
#define D_EXT_SEQ_JA1_EVENT_DUMP_H

#include <types.h>

namespace ExtSeq {

class Ja1Track;

/**
 * §59 / ASK 17 — diagnostic event stream from Ja1Parser (engine's own parse).
 * Columns match Bridge `seq-events`: tick,track_id,event,note_param,velocity.
 * No behaviour change when inactive.
 */
namespace Ja1EventDump {

/** True when DUSK_EXTSEQ_EVENT_DUMP is set (non-empty, not "0"). */
bool envEnabled();

bool active();

/** Offline walk of BMS via Ja1Parser; write CSV. Returns event count. */
u32 dumpBmsToCsv(const u8* data, u32 size, const char* outPath);

/** Emit one row (uses pending file_off from markOpcodeOff). */
void emit(const char* event, const char* noteParam, const char* velocity);

/** Call immediately before reading an opcode byte (Bridge file_off semantics). */
void markOpcodeOff(const Ja1Track* track);

u8 currentTrackId();
u32 currentTick();

void queueOpen(u8 childId, u32 fileOff);
bool noteSawBackwardJmp(u32 tgt);
void markBackwardJmp(u32 tgt);

/** Set by cmdWait so the walker knows a wait event was already emitted. */
void markExplicitWait();
bool consumeExplicitWait();

}  // namespace Ja1EventDump

}  // namespace ExtSeq

#endif
