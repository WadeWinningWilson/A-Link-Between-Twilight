#ifndef D_EXT_SEQ_JA1_PARSER_H
#define D_EXT_SEQ_JA1_PARSER_H

#include <types.h>

namespace ExtSeq {

class Ja1Track;

/**
 * WW JASystem::TSeqParser dialect (parseSeq dispatch).
 * Hands noteOn/noteOff to Ja1Track, which owns the resulting voices.
 */
class Ja1Parser {
public:
    int parseSeq(Ja1Track* track);

private:
    int cmdOpenTrack(Ja1Track* track, u32* args);
    int cmdCall(Ja1Track* track, u32* args);
    int cmdRet(Ja1Track* track, u32* args);
    int cmdJmp(Ja1Track* track, u32* args);
    int cmdLoopS(Ja1Track* track, u32* args);
    int cmdLoopE(Ja1Track* track, u32* args);
    int cmdWait(Ja1Track* track, u32* args);
    int cmdWait(Ja1Track* track, u8 flag);
    int cmdNoteOff(Ja1Track* track, u8 flag);
    int cmdNoteOn(Ja1Track* track, u8 note);
    int cmdSetParam(Ja1Track* track, u8 param);
    int cmdTempo(Ja1Track* track, u32* args);
    int cmdTimeBase(Ja1Track* track, u32* args);
    int cmdFinish(Ja1Track* track, u32* args);
    int cmdTranspose(Ja1Track* track, u32* args);
    int cmdCloseTrack(Ja1Track* track, u32* args);
    int cmdSimpleADSR(Ja1Track* track, u32* args);
    int cmdNop(Ja1Track* track, u32* args);
    int Cmd_Process(Ja1Track* track, u8 op, u16 extra);
    bool conditionCheck(Ja1Track* track, u8 flag);
};

}  // namespace ExtSeq

#endif
