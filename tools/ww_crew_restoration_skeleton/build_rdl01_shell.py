#!/usr/bin/env python3

# ============================================================================
"""Build mod-side thin interiors host stage R_DL01 (№79/№81).

Writes under the live WW-Crew-Restoration Layer-A tree:
  files/res/Stage/R_DL01/STG_00.arc
  files/res/Stage/R_DL01/R00_00.arc

STG is cloned from a tiny TP interiors stage (R_SP300), with STAG save-table
patched to placeholder index 15. Under №81 EXTENSION-FIRST that index is NOT
the progression home — native writers refuse while WW is active; the extension
router will own R_DL* progression later. Room 0 is a thin RARC shell (№62 style);
LinkRM mounts on room-objects-ready.

Usage:
  build_rdl01_shell.py            (no arguments)

  Inputs   : <mod>/files/res/Stage/F_SP115/R02_00.arc — the thin room shell cloned
             for room 0; must already exist, this script does not build it
             D:/XXXXXXX/Ex TP/files/res/Stage/R_SP300/STG_00.arc — TP template
             (Yaz0 accepted and decompressed in-process)
  Outputs  : <mod>/files/res/Stage/R_DL01/STG_00.arc — R_SP300 clone, STAG save
             table patched to placeholder index 15
             <mod>/files/res/Stage/R_DL01/R00_00.arc — thin room-0 shell
  Idempotent: yes, and DESTRUCTIVELY so — both files are rewritten from source on
             every run and NO backup is taken.
  Order    : FIRST in the R_DL01 chain — and it must NOT be re-run after
             grow_rdl01_stg.py. What it writes is the R_SP300 clone with only the
             STAG save index patched; the room growth is NOT reapplied and no
             backup is taken, so a late re-run silently discards the 6-room
             stage.dzs and native room streaming stops requesting rooms 1-5.
             This is the one ANTI-EDGE in the chain: not "runs before", but
             "must never run after".
  R1 note  : F_SP115/R02_00.arc is a donor-of-shape from another part of the tree.
             A recipe must order whatever produces F_SP115 ahead of this step, or
             this step aborts with SystemExit — loudly, which is the right failure.
"""
from __future__ import annotations
# ============================================================================
# TIER-3 QUARANTINE (kit audit + OUTPUT LAW wiring). This tool writes donor-
# format bytes and belongs to the era the project is leaving. It must NOT run
# in the disc-native porting wave. Kept RE-RUNNABLE per the No116 ceremony:
# pass --tier3-override; the run is then LEDGERED so no donor-format write
# happens silently. IMPORT-SAFE: gates only under __main__, so Tier-1 kits
# that import helpers from this file are unaffected.
# ============================================================================
import sys as _q_sys
if __name__ == "__main__":
    if "--tier3-override" not in _q_sys.argv:
        _q_sys.stderr.write(
            "TIER-3 QUARANTINED (kit audit): retired from the disc-native wave. "
            "Deliberate rerun: --tier3-override (ledgered).\n")
        raise SystemExit(3)
    _q_sys.argv.remove("--tier3-override")
    import io as _q_io
    from pathlib import Path as _q_P
    _q_led = _q_P(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging" / "OUTPUT-LEDGER.md"
    with _q_io.open(_q_led, "a", encoding="utf-8", newline="\r\n") as _q_f:
        _q_f.write("| TIER3-OVERRIDE RUN: %s | deliberate rerun | quarantine stands |\n"
                   % _q_P(__file__).name)

import os
import struct
import sys
from pathlib import Path


def yaz0_decompress(data: bytes) -> bytes:
    assert data[:4] == b"Yaz0"
    size = struct.unpack(">I", data[4:8])[0]
    src = memoryview(data)[16:]
    dst = bytearray()
    i = 0
    while len(dst) < size:
        code = src[i]
        i += 1
        for bit in range(8):
            if len(dst) >= size:
                break
            if code & (0x80 >> bit):
                dst.append(src[i])
                i += 1
            else:
                b1 = src[i]
                b2 = src[i + 1]
                i += 2
                dist = ((b1 & 0x0F) << 8) | b2
                copy = b1 >> 4
                if copy == 0:
                    copy = src[i] + 0x12
                    i += 1
                else:
                    copy += 2
                for _ in range(copy):
                    dst.append(dst[-dist - 1])
    return bytes(dst)


def copy_r02_shell(r02_path: Path, note: bytes) -> bytes:
    data = bytearray(r02_path.read_bytes())
    start = data.find(b"thin room shell for N62 R02")
    if start < 0:
        start = data.find(b"thin room shell")
    if start < 0:
        raise SystemExit(f"not a thin shell arc: {r02_path}")
    body = note if note.endswith(b"\n") else note + b"\n"
    end = len(data)
    space = end - start
    if len(body) > space:
        body = body[:space]
    data[start : start + len(body)] = body
    for i in range(start + len(body), end):
        data[i] = 0
    data = data.replace(b"R02_00", b"R00_00")
    data = data.replace(b"R02_", b"R00_")
    return bytes(data)


def patch_stg_save_tbl(rarc: bytearray, save_index: int) -> None:
    """Patch stage.dzs STAG field_0x09 save table (bits 1..5)."""
    stag_tag = rarc.find(b"STAG")
    if stag_tag < 0:
        raise SystemExit("STAG not found in STG rarc")
    for back in range(0, 0x200, 4):
        start = stag_tag - back
        if start < 4:
            break
        n = struct.unpack(">I", rarc[start : start + 4])[0]
        if n == 0 or n > 64:
            continue
        if rarc[start + 4 : start + 8] != b"STAG":
            continue
        data_off = struct.unpack(">I", rarc[start + 12 : start + 16])[0]
        stag_data = start + data_off
        if stag_data + 0x0A >= len(rarc):
            continue
        f09 = rarc[stag_data + 9]
        key = f09 & 1
        rarc[stag_data + 9] = ((save_index & 0x1F) << 1) | key
        print(
            f"patched STAG saveTbl {(f09 >> 1) & 0x1F} -> {save_index} "
            f"(keyDisp={key}) at {stag_data:#x}"
        )
        return
    raise SystemExit("failed to patch STAG")


def main() -> int:
    appdata = Path(os.environ["APPDATA"])
    mod = appdata / "TwilitRealm" / "Dusklight" / "model_replacements" / "WW-Crew-Restoration"
    out_dir = mod / "files" / "res" / "Stage" / "R_DL01"
    out_dir.mkdir(parents=True, exist_ok=True)

    r02 = mod / "files" / "res" / "Stage" / "F_SP115" / "R02_00.arc"
    if not r02.is_file():
        raise SystemExit(f"missing thin R02 shell to clone: {r02}")

    stg_src = Path(r"D:\XXXXXXX\Ex TP\files\res\Stage\R_SP300\STG_00.arc")
    if not stg_src.is_file():
        raise SystemExit(f"missing template STG: {stg_src}")

    raw = stg_src.read_bytes()
    rarc = bytearray(yaz0_decompress(raw) if raw[:4] == b"Yaz0" else raw)
    # Placeholder only — №81 extension owns WW progression; writers refuse meanwhile.
    patch_stg_save_tbl(rarc, 15)

    stg_out = out_dir / "STG_00.arc"
    stg_out.write_bytes(rarc)
    print("wrote", stg_out, "bytes", stg_out.stat().st_size)

    room = copy_r02_shell(r02, b"thin room shell for N81 R_DL01 room0")
    r00_out = out_dir / "R00_00.arc"
    r00_out.write_bytes(room)
    print("wrote", r00_out, "bytes", r00_out.stat().st_size)
    print("OK — mod-side Stage/R_DL01 ready (Layer A overlay)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
