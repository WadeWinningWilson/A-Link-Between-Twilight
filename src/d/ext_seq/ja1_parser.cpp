/**
 * JA1 TSeqParser dialect — dispatch matches WW JASSeqParser::parseSeq.
 * Ownership of voices stays on Ja1Track / ExtSeqSpace (№89).
 */
#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_event_dump.h"
#include "d/ext_seq/ja1_track.h"

#include <cstdio>
#include <cstring>

namespace ExtSeq {
namespace {

void dumpEmit(const char* event, const char* noteParam, const char* velocity) {
    if (Ja1EventDump::active()) {
        Ja1EventDump::emit(event, noteParam, velocity);
    }
}

void dumpEmitU(const char* event, u32 noteParam) {
    if (!Ja1EventDump::active()) {
        return;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%u", noteParam);
    Ja1EventDump::emit(event, buf, "");
}

}  // namespace

bool Ja1Parser::conditionCheck(Ja1Track* track, u8 param_2) {
    const u16 val = track->mRegisterParam.getFlag();
    switch (param_2 & 0xF) {
    case 0:
        return true;
    case 1:
        return val == 0;
    case 2:
        return val != 0;
    case 3:
        return val == 1;
    case 4:
        return val >= 0x8000;
    case 5:
        return val < 0x8000;
    default:
        return false;
    }
}

int Ja1Parser::cmdOpenTrack(Ja1Track* track, u32* args) {
    const u32 param1 = args[0];
    const u32 param2 = args[1];
    const u8 b1 = param1 & 0xF;
    u8 b2 = (param1 >> 6) & 3;
    if (param1 & 0x20) {
        b2 = 4;
    }
    if (Ja1EventDump::active()) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "%u:%u", static_cast<unsigned>(b1), param2);
        dumpEmit("open_track", buf, "");
        Ja1EventDump::queueOpen(b1, param2);
        return 0;
    }
    Ja1Track* child = track->openChild(b1, b2);
    if (child == nullptr) {
        return 0;
    }
    child->start(track->getSeq()->getBase(), param2);
    return 0;
}

int Ja1Parser::cmdCall(Ja1Track* track, u32* args) {
    (void)args;
    u8 flag = track->getSeq()->readByte();
    u32 data;
    if (flag & 0x80) {
        u8 cData = track->getSeq()->readByte();
        data = track->readReg16(cData);
        if (flag & 0x40) {
            u32 offs = (flag & 0x20) ? track->readReg16(track->getSeq()->readByte())
                                     : track->getSeq()->read24();
            data = track->getSeq()->get24(offs + data * 3);
        }
    } else {
        data = track->getSeq()->read24();
    }
    if (Ja1EventDump::active()) {
        if (!(flag & 0x80)) {
            dumpEmitU("call", data);
        } else {
            dumpEmit("call", "", "");
        }
    }
    if (conditionCheck(track, flag)) {
        track->getSeq()->call(data);
    }
    return 0;
}

int Ja1Parser::cmdRet(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        dumpEmit("ret", "", "");
    }
    if (conditionCheck(track, static_cast<u8>(args[0]))) {
        if (track->getSeq()->mLoopIndex == 0) {
            return 3;
        }
        track->getSeq()->ret();
    }
    return 0;
}

int Ja1Parser::cmdJmp(Ja1Track* track, u32* args) {
    (void)args;
    u8 flag = track->getSeq()->readByte();
    u32 data;
    if (flag & 0x80) {
        u8 cData = track->getSeq()->readByte();
        if (flag & 0x40) {
            data = track->readReg16(cData) & 0xffff;
            u32 offs = (flag & 0x20) ? track->readReg16(track->getSeq()->readByte())
                                     : track->getSeq()->read24();
            data = track->getSeq()->get24(offs + data * 3);
        } else {
            data = track->readReg32(cData);
        }
    } else {
        data = track->getSeq()->read24();
    }
    if (Ja1EventDump::active()) {
        if (!(flag & 0x80)) {
            dumpEmitU("jmp", data);
        } else {
            dumpEmit("jmp", "", "");
        }
    }
    if (conditionCheck(track, flag)) {
        if (Ja1EventDump::active() && !(flag & 0x80) && track->getSeq()->getBase() != nullptr) {
            const u32 cur = static_cast<u32>(track->getSeq()->mCurrentFilePtr -
                                             track->getSeq()->getBase());
            if (data < cur) {
                if (Ja1EventDump::noteSawBackwardJmp(data)) {
                    track->close();
                    return 3;
                }
                Ja1EventDump::markBackwardJmp(data);
            }
        }
        track->getSeq()->jump(data);
    }
    return 0;
}

int Ja1Parser::cmdLoopS(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        // Cap expansions like Bridge so streams stay finite/comparable.
        u32 remain = args[0];
        if (remain == 0) {
            remain = 1;
        }
        const u32 capped = remain > 256u ? 256u : remain;
        if (capped != remain) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%u/%u", capped, remain);
            dumpEmit("loop_s", buf, "");
        } else {
            dumpEmitU("loop_s", capped);
        }
        track->getSeq()->loopStart(capped);
        return 0;
    }
    track->getSeq()->loopStart(args[0]);
    return 0;
}

int Ja1Parser::cmdLoopE(Ja1Track* track, u32* args) {
    (void)args;
    if (Ja1EventDump::active()) {
        dumpEmit("loop_e", "", "");
    }
    track->getSeq()->loopEnd();
    return 0;
}

int Ja1Parser::cmdWait(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        dumpEmitU("wait", args[0]);
        Ja1EventDump::markExplicitWait();
    }
    track->getSeq()->wait(static_cast<s32>(args[0]));
    return args[0] != 0 ? 1 : 0;
}

int Ja1Parser::cmdWait(Ja1Track* track, u8 flag) {
    const int end = flag == 0x80 ? 1 : 2;
    int val = 0;
    for (int i = 0; i < end; i++) {
        val <<= 8;
        val |= track->getSeq()->readByte();
    }
    if (Ja1EventDump::active()) {
        dumpEmitU("wait", static_cast<u32>(val));
        Ja1EventDump::markExplicitWait();
    }
    track->getSeq()->mWaitTimer = val;
    return val != 0 ? 1 : 0;
}

int Ja1Parser::cmdNoteOff(Ja1Track* track, u8 flag) {
    if (flag == 0xF9) {
        u32 r30 = track->getSeq()->readByte();
        u32 rdata2 = track->exchangeRegisterValue(r30 & 0x7) & 0xFF;
        flag = static_cast<u8>(rdata2 + 0x80);
        if (r30 & 0x80) {
            flag |= 0x08;
        }
    }
    int r6 = 0;
    if (flag & 0x08) {
        r6 = track->getSeq()->readByte();
        if (r6 > 100) {
            r6 = (r6 - 98) * 20;
        }
    }
    if (Ja1EventDump::active()) {
        dumpEmitU("note_off", static_cast<u32>(flag & 0x7));
    }
    track->noteOff(flag & 0x7, static_cast<u16>(r6));
    return 0;
}

int Ja1Parser::cmdNoteOn(Ja1Track* track, u8 note) {
    u32 r27 = track->getSeq()->readByte();
    if (r27 & 0x80) {
        note = static_cast<u8>(track->exchangeRegisterValue(note));
    }
    note = static_cast<u8>(note + track->getTranspose());

    u8 r25 = track->getSeq()->readByte();
    if (r25 & 0x80) {
        r25 = static_cast<u8>(track->exchangeRegisterValue(r25 & 0x7F));
    }

    u8 noteid = r27 & 0x7;
    u8 r23 = 0;
    u32 r24 = 0;
    if (!noteid) {
        r23 = track->getSeq()->readByte();
        if (r23 & 0x80) {
            r23 = static_cast<u8>(track->exchangeRegisterValue(r23 & 0x7F));
        }
        const int count = (r27 >> 3) & 0x3;
        for (int i = 0; i < count; i++) {
            r24 <<= 8;
            r24 |= track->getSeq()->readByte();
        }
        if (count == 1 && (r24 & 0x80)) {
            r24 = track->exchangeRegisterValue(r24 & 0x7F);
        }
    } else {
        if ((r27 >> 3) & 0x3) {
            r24 = track->getSeq()->readByte();
            if (r24 & 0x80) {
                r24 = track->exchangeRegisterValue(r24 & 0x7F);
            }
        } else {
            r24 = 0;
        }
        r23 = 100;
    }

    if (Ja1EventDump::active()) {
        char nbuf[16];
        char vbuf[16];
        std::snprintf(nbuf, sizeof(nbuf), "%u", static_cast<unsigned>(note & 0x7F));
        // Match Bridge: empty velocity when sourced from register (high bit path already
        // resolved above — emit resolved value when we have one).
        std::snprintf(vbuf, sizeof(vbuf), "%u", static_cast<unsigned>(r25));
        dumpEmit("note_on", nbuf, vbuf);
    }

    s32 time = -1;
    if (static_cast<s32>(r24) != -1) {
        time = track->seqTimeToDspTime(static_cast<s32>(r24), r23);
    }
    if (!track->mIsPaused || !(track->mPauseStatus & 0x10)) {
        track->noteOn(noteid, note, r25, time, 0);
    }
    track->mNoteMgr.setBaseTime(static_cast<s32>(r24));
    track->mNoteMgr.setLastNote(note);
    if (r24 == 0xFFFFFFFF) {
        return 0;
    }
    track->getSeq()->wait(r24 ? static_cast<s32>(r24) : -1);
    return 1;
}

int Ja1Parser::cmdSetParam(Ja1Track* track, u8 param_2) {
    u8 flag = track->getSeq()->readByte();
    s16 data = 0;
    switch (param_2 & 0xC) {
    case 0:
        data = static_cast<s16>(track->readReg16(track->getSeq()->readByte()));
        break;
    case 4:
        data = track->getSeq()->readByte();
        break;
    case 8: {
        u8 byte = track->getSeq()->readByte();
        data = (byte & 0x80) ? static_cast<s16>(byte << 8)
                             : static_cast<s16>((byte << 8) | (byte << 1));
        break;
    }
    case 12:
        data = static_cast<s16>(track->getSeq()->read16());
        break;
    }
    int val = 0;
    switch (param_2 & 0x3) {
    case 0:
        val = -1;
        break;
    case 1:
        val = track->readReg16(track->getSeq()->readByte());
        break;
    case 2:
        val = track->getSeq()->readByte();
        break;
    case 3:
        val = track->getSeq()->read16();
        break;
    }
    if (Ja1EventDump::active()) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "0x%02X", static_cast<unsigned>(0x90 | (param_2 & 0xF)));
        dumpEmit("set_param", buf, "");
    }
    track->setParam(flag, static_cast<int>(data) / 32767.0f, val);
    return 0;
}

int Ja1Parser::cmdTempo(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        dumpEmitU("tempo", args[0]);
    }
    track->setTempo(static_cast<u16>(args[0]));
    return 0;
}

int Ja1Parser::cmdTimeBase(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        dumpEmitU("timebase", args[0]);
    }
    track->setTimebase(static_cast<u16>(args[0]));
    return 0;
}

int Ja1Parser::cmdFinish(Ja1Track* track, u32* args) {
    (void)args;
    if (Ja1EventDump::active()) {
        dumpEmit("finish", "", "");
    }
    track->close();
    return 3;
}

int Ja1Parser::cmdTranspose(Ja1Track* track, u32* args) {
    if (Ja1EventDump::active()) {
        dumpEmitU("transpose", args[0]);
    }
    track->setTranspose(static_cast<s32>(args[0]));
    return 0;
}

int Ja1Parser::cmdCloseTrack(Ja1Track* track, u32* args) {
    const u32 track_no = args[0] & 0xF;
    if (Ja1EventDump::active()) {
        dumpEmitU("close_track", track_no);
    }
    if (track_no < Ja1Track::kMaxChildren && track->mChildren[track_no]) {
        track->mChildren[track_no]->close();
        track->mChildren[track_no] = nullptr;
    }
    return 0;
}

int Ja1Parser::cmdNop(Ja1Track* track, u32* args) {
    (void)track;
    (void)args;
    return 0;
}

int Ja1Parser::Cmd_Process(Ja1Track* track, u8 op, u16 extra) {
    // WW Arglist[op-0xC0]: {count, format} — format packs 2 bits/arg
    // (0=u8, 1=u16, 2=u24, 3=reg). Keep the cursor aligned even for nops.
    static const u16 kArgCount[64] = {
        0, 2, 2, 1, 0, 0, 1, 1, 0, 1, 0, 2, 2, 1, 1, 1, 0, 2, 2, 0, 1, 1, 1, 2, 5, 1,
        1, 1, 1, 2, 1, 2, 1, 0, 0, 0, 2, 1, 1, 1, 0, 0, 1, 5, 4, 1, 1, 3, 1, 1, 3, 1,
        1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,
    };
    static const u16 kArgFmt[64] = {
        0x0000, 0x0008, 0x0008, 0x0002, 0x0000, 0x0000, 0x0000, 0x0002, 0x0000, 0x0001,
        0x0000, 0x0000, 0x000C, 0x0000, 0x0000, 0x0003, 0x0000, 0x000C, 0x000C, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0008, 0x0155, 0x0000, 0x0000, 0x0000, 0x0001, 0x0004,
        0x0000, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0001, 0x0001,
        0x0000, 0x0000, 0x0002, 0x0000, 0x0055, 0x0002, 0x0002, 0x0000, 0x0000, 0x0000,
        0x0028, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000,
        0x0000, 0x0001, 0x0001, 0x0000,
    };

    if (op < 0xC0) {
        track->close();
        return 3;
    }
    const u8 idx = static_cast<u8>(op - 0xC0);
    u32 args[8]{};
    u16 fmt = static_cast<u16>(kArgFmt[idx] | extra);
    const u16 n = kArgCount[idx];
    for (u16 i = 0; i < n; i++) {
        u32 r3 = 0;
        switch (fmt & 0x3) {
        case 0:
            r3 = track->getSeq()->readByte();
            break;
        case 1:
            r3 = track->getSeq()->read16();
            break;
        case 2:
            r3 = track->getSeq()->read24();
            break;
        case 3:
            r3 = track->exchangeRegisterValue(track->getSeq()->readByte());
            break;
        }
        args[i] = r3;
        fmt = static_cast<u16>(fmt >> 2);
    }

    switch (op) {
    case 0xC1:
        return cmdOpenTrack(track, args);
    case 0xC4:
        return cmdCall(track, args);
    case 0xC6:
        return cmdRet(track, args);
    case 0xC8:
        return cmdJmp(track, args);
    case 0xC9:
        return cmdLoopS(track, args);
    case 0xCA:
        return cmdLoopE(track, args);
    case 0xCF:  // Wait (indexed form) — Arglist[15]
        return cmdWait(track, args);
    case 0xD8:
        return cmdTranspose(track, args);
    case 0xD9:
        return cmdCloseTrack(track, args);
    case 0xFD:  // Tempo (WW dialect)
        return cmdTempo(track, args);
    case 0xFE:  // TimeBase
        return cmdTimeBase(track, args);
    case 0xFF:
        return cmdFinish(track, args);
    default:
        // Consumed per Arglist — emit generic for §59 dump; nop for playback.
        if (Ja1EventDump::active()) {
            char name[16];
            char params[96];
            std::snprintf(name, sizeof(name), "cmd_%02X", op);
            params[0] = '\0';
            for (u16 i = 0; i < n; i++) {
                char one[24];
                std::snprintf(one, sizeof(one), "%s%u", i ? "," : "", args[i]);
                std::strncat(params, one, sizeof(params) - std::strlen(params) - 1);
            }
            dumpEmit(name, params, "");
        }
        return 0;
    }
}

int Ja1Parser::parseSeq(Ja1Track* track) {
    while (true) {
        if (track->getSeq()->mCurrentFilePtr == nullptr ||
            track->getSeq()->mRawFilePtr == nullptr) {
            return -1;
        }
        if (Ja1EventDump::active()) {
            Ja1EventDump::markOpcodeOff(track);
        }
        u8 flag = track->getSeq()->readByte();
        int r29 = 0;
        if (!(flag & 0x80)) {
            r29 = cmdNoteOn(track, flag);
        } else if (((flag & 0xF0) == 0x80) && !(flag & 0x07)) {
            r29 = cmdWait(track, flag);
        } else if (((flag & 0xF0) == 0x80) || flag == 0xF9) {
            r29 = cmdNoteOff(track, flag);
        } else {
            switch (flag & 0xF0) {
            case 0x90:
                r29 = cmdSetParam(track, flag & 0xF);
                break;
            case 0xA0:
                track->writeRegParam(flag & 0xF);
                break;
            case 0xB0:
                // RegCmd — read nested op; treat as Cmd_Process of next byte
                {
                    u8 nested = track->getSeq()->readByte();
                    if ((flag >> 3) & 1) {
                        nested = static_cast<u8>(track->exchangeRegisterValue(nested));
                    }
                    if (Ja1EventDump::active()) {
                        char buf[16];
                        std::snprintf(buf, sizeof(buf), "0x%02X", nested);
                        dumpEmit("reg_cmd", buf, "");
                    }
                    r29 = Cmd_Process(track, nested, 0);
                }
                break;
            default:
                r29 = Cmd_Process(track, flag, 0);
                break;
            }
        }
        if (r29 == 0) {
            continue;
        }
        if (r29 == 1) {
            break;
        }
        if (r29 == 3) {
            return -1;
        }
    }
    return 0;
}

}  // namespace ExtSeq
