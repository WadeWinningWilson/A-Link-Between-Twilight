#!/usr/bin/env python3
# ============================================================================
# dusktap_dmesg.py — DuskTap golden-trace probe: donor dMesg + tale lifecycle
#
# Attaches to a RUNNING mainline Dolphin playing donor WW (US, GZLE01) and
# frame-samples the message system + demo/event lifecycle, emitting a
# transition-stamped CSV + live console timeline. Read-only observation —
# no writes, no patches, no savestate manipulation.
#
# Every address is symbol-exact from the decomp (config/GZLE01/symbols.txt)
# and every offset from donor headers — no scanned/guessed addresses:
#   g_dComIfG_gameInfo .bss:0x803C4C08          (symbols.txt:17472)
#     .play                +0x12A0              (d_com_inf_game.h:898)
#     .play.mEvtCtrl       +0x3F38              (d_com_inf_game.h:711)
#        .mEventId           +0xD8  s16         (d_event.h)
#        .mEventInfoIdx      +0xDF  u8
#        .mEventFlag         +0xE8  u16         (control-lock word)
#     .play.mDemo (ptr)    +0x4828              (d_com_inf_game.h:759)
#        dDemo_manager_c: mControl* +0x04, mMesgControl* +0x18,
#        mCurFile* +0xD0, mFrame +0xD4, mFrameNoMsg +0xD8, mMode +0xDC
#                                              (d_demo.h:339-351)
#        TControl: suspend cache _54 +0x54     (JStudio stb.h)
#     .play.mpPlayerPtr[0] +0x48AC             (d_com_inf_game.h:774)
#        fopAc_ac_c.current +0x1F8 (pos f32 x3, then csXyz angle)
#   dMesg_gpControl .sbss:0x803F7014           (symbols.txt:19095)
#     dMesg_tControl: mLineLength[4] +0x44 f32, mLineCount +0x54,
#     mInitFontSize +0x5C, mNowFontSize +0x60, mCharSpace +0x64,
#     mTextBoxWidth +0x6C                      (d_mesg.h)
#   nowMesgCode .sbss:0x803F7030               (symbols.txt:19104)
#
# Usage (foundry python):
#   <decomp-root>\foundry-py312\Scripts\python.exe dusktap_dmesg.py [out.csv]
# Start Dolphin + the donor ISO first; probe attaches and follows.
# Stop with Ctrl+C — summary is printed and CSV closed cleanly.
# ============================================================================
import csv
import struct
import sys
import time

import dolphin_memory_engine as dme

GAMEINFO = 0x803C4C08
PLAY = GAMEINFO + 0x12A0
EVT = PLAY + 0x3F38
DEMO_PTR = PLAY + 0x4828
PLAYER_PTR = PLAY + 0x48AC
MSG_CTRL_PTR = 0x803F7014
NOW_MESG_CODE = 0x803F7030

VALID = range(0x80000000, 0x81800000)


def rd(addr, size):
    return dme.read_bytes(addr, size)


def u8(a):
    return rd(a, 1)[0]


def u16(a):
    return struct.unpack(">H", rd(a, 2))[0]


def s16(a):
    return struct.unpack(">h", rd(a, 2))[0]


def u32(a):
    return struct.unpack(">I", rd(a, 4))[0]


def s32(a):
    return struct.unpack(">i", rd(a, 4))[0]


def f32(a):
    return struct.unpack(">f", rd(a, 4))[0]


def cstr(a, n=32):
    if a not in VALID:
        return ""
    b = rd(a, n)
    return b.split(b"\0")[0].decode("ascii", "replace")


def sample():
    """One frame-sample of the full probe set. None-safe on every pointer."""
    s = {"t": time.time()}
    s["msg_code"] = s32(NOW_MESG_CODE)

    mc = u32(MSG_CTRL_PTR)
    if mc in VALID:
        s["line_count"] = s32(mc + 0x54)
        s["font_init"] = s32(mc + 0x5C)
        s["font_now"] = s32(mc + 0x60)
        s["char_space"] = s32(mc + 0x64)
        s["box_width"] = s32(mc + 0x6C)
        s["line_len"] = tuple(round(f32(mc + 0x44 + 4 * i), 1) for i in range(4))
    else:
        s["line_count"] = s["font_init"] = s["font_now"] = None
        s["char_space"] = s["box_width"] = None
        s["line_len"] = None

    s["evt_id"] = s16(EVT + 0xD8)
    s["evt_info_idx"] = u8(EVT + 0xDF)
    s["evt_flag"] = u16(EVT + 0xE8)

    dm = u32(DEMO_PTR)
    if dm in VALID:
        s["demo_file"] = cstr(u32(dm + 0xD0))
        s["demo_frame"] = s32(dm + 0xD4)
        s["demo_frame_nomsg"] = s32(dm + 0xD8)
        s["demo_mode"] = s32(dm + 0xDC)
        tc = u32(dm + 0x04)
        s["stb_suspend"] = s32(tc + 0x54) if tc in VALID else None
    else:
        s["demo_file"] = ""
        s["demo_frame"] = s["demo_frame_nomsg"] = s["demo_mode"] = None
        s["stb_suspend"] = None

    pl = u32(PLAYER_PTR)
    if pl in VALID:
        s["link_pos"] = tuple(round(f32(pl + 0x1F8 + 4 * i), 1) for i in range(3))
        # actor_place: pos +0x00 (cXyz), angle +0x0C (csXyz x,y,z) —
        # f_op_actor.h:212; angle.y = +0x0C + 2
        s["link_angle_y"] = s16(pl + 0x1F8 + 0x0C + 2)
    else:
        s["link_pos"] = None
        s["link_angle_y"] = None
    return s


# fields whose CHANGE constitutes a timeline event (pos excluded — too noisy;
# it is still recorded on every emitted row for context)
KEYS = ["msg_code", "line_count", "font_now", "char_space", "box_width",
        "line_len", "evt_id", "evt_info_idx", "evt_flag",
        "demo_file", "demo_mode", "stb_suspend"]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else (
        "%USERPROFILE%/Documents/ww-arc-staging/dusktap_dmesg_%d.csv"
        % int(time.time()))
    print("waiting for Dolphin (launch the donor ISO whenever)…")
    while True:
        dme.hook()
        if dme.is_hooked():
            break
        time.sleep(2)
    print("hooked. logging ->", out)

    cols = ["t", "demo_frame", "demo_frame_nomsg"] + KEYS + [
        "link_pos", "link_angle_y", "changed"]
    f = open(out, "w", newline="", encoding="utf-8")
    w = csv.DictWriter(f, fieldnames=cols)
    w.writeheader()

    prev = None
    n = 0
    try:
        while True:
            try:
                s = sample()
            except RuntimeError:
                # emulator paused/closed mid-run: keep the CSV, re-wait
                if not dme.is_hooked():
                    print("Dolphin gone — waiting for it to return…")
                    while not dme.is_hooked():
                        time.sleep(2)
                        dme.hook()
                    print("re-hooked")
                else:
                    time.sleep(0.25)
                continue
            changed = ([k for k in KEYS if prev and s[k] != prev[k]]
                       if prev else KEYS)
            if changed:
                row = {k: s.get(k) for k in cols if k != "changed"}
                row["changed"] = "|".join(changed)
                w.writerow(row)
                f.flush()
                hl = ", ".join(f"{k}={prev[k] if prev else '?'}->{s[k]}"
                               for k in changed[:4])
                print(f"[{s['demo_frame']}] {hl}")
                n += 1
            prev = s
            time.sleep(1 / 120)  # 2x frame rate — no transition skipped
    except KeyboardInterrupt:
        pass
    finally:
        f.close()
        print(f"\n{n} transitions -> {out}")


if __name__ == "__main__":
    main()
