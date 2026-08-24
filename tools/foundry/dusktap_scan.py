#!/usr/bin/env python3
# ============================================================================
# dusktap_scan.py — §371b LIVE WORLD CENSUS, signature-scan edition.
#
# WHY THIS REPLACES THE TREE WALK: two attempts to enumerate via the donor's
# process lists produced wrong rosters (first walked `prev` and died at one
# node; second found 8-9 nodes whose mpData was not a process). Rather than
# keep guessing list internals on the user's runtime, this scans game RAM for
# PROCESS STRUCTURES DIRECTLY and validates each candidate against fields we
# have hard receipts for — no list layout involved anywhere.
#
# Candidate test (all must hold; donor receipts in f_pc_base.h / f_pc_profile.h):
#   base_process_class: mProcName s16 @0x08, mProfName s16 @0x0E,
#                       mpProf ptr @0x10, mpPcMtd ptr @0xA8
#   process_profile_definition: mProcName s16 @0x08, mSize s32 @0x10
#   1) mpProf points into RAM and is 4-aligned
#   2) profile.mProcName == process.mProcName   <-- the strong filter
#   3) 0 < mProcName <= 0x240 (fpcNm_* range)
#   4) profile.mSize between 0x20 and 0x8000 (sane allocation)
#   5) mpPcMtd points into RAM
# Actor extras (fopAc_ac_c): current @0x1F8 (pos f32x3, roomNo u8 @+0x12),
# mParameters @0xB0. Position sanity-checked before reporting.
#
# Names: donor f_pc_name.h enum (index -> fpcNm_*), plus d_stage.cpp OBJNAME
# rows so each process shows what room data would CALL it.
#
# Usage: dusktap_scan.py [--watch] [--room-only]
# Read-only. Never writes game memory.
# ============================================================================
import csv
import re
import struct
import sys
import time
from collections import Counter
from pathlib import Path

import dolphin_memory_engine as dme

DONOR = Path("<decomp-root>/WW DP")
OUT = Path("%USERPROFILE%/Documents/ww-arc-staging")
RAM_START, RAM_END = 0x80000000, 0x81800000
CHUNK = 0x100000


def proc_names():
    txt = (DONOR / "include/f_pc/f_pc_name.h").read_text(encoding="utf-8",
                                                         errors="replace")
    return {int(m.group(1), 16): m.group(2)
            for m in re.finditer(r"/\*\s*0x([0-9A-Fa-f]+)\s*\*/\s*(fpcNm_\w+)", txt)}


def objnames():
    txt = (DONOR / "src/d/d_stage.cpp").read_text(encoding="utf-8",
                                                  errors="replace")
    out = {}
    for m in re.finditer(r'OBJNAME\("([^"]+)",\s*(fpcNm_\w+)', txt):
        out.setdefault(m.group(2), []).append(m.group(1))
    return out


def snapshot():
    """One pass of game RAM, chunked, as (base, bytes) pairs."""
    for base in range(RAM_START, RAM_END, CHUNK):
        try:
            yield base, dme.read_bytes(base, CHUNK)
        except RuntimeError:
            continue


def in_ram(v):
    return RAM_START <= v < RAM_END


def scan(names, objs):
    """Signature-scan every 4-byte-aligned offset for validated processes."""
    mem = {}
    for base, blob in snapshot():
        mem[base] = blob
    if not mem:
        return None

    def rd(addr, n):
        b = mem.get(addr & ~(CHUNK - 1))
        if b is None:
            return None
        off = addr & (CHUNK - 1)
        if off + n > CHUNK:
            return None
        return b[off:off + n]

    def u32(a):
        v = rd(a, 4)
        return struct.unpack(">I", v)[0] if v else None

    def s16(a):
        v = rd(a, 2)
        return struct.unpack(">h", v)[0] if v else None

    def f32(a):
        v = rd(a, 4)
        return struct.unpack(">f", v)[0] if v else None

    found = []
    for base, blob in mem.items():
        for off in range(0, CHUNK - 0x200, 4):
            # cheap first gate: mProcName in range, read straight from the blob
            pname = struct.unpack_from(">h", blob, off + 0x08)[0]
            if pname <= 0 or pname > 0x240:
                continue
            prof = struct.unpack_from(">I", blob, off + 0x10)[0]
            if not in_ram(prof) or prof & 3:
                continue
            pprof_name = s16(prof + 0x08)
            if pprof_name != pname:            # THE strong filter
                continue
            psize = u32(prof + 0x10)
            if psize is None or not (0x20 <= psize <= 0x8000):
                continue
            mtd = struct.unpack_from(">I", blob, off + 0xA8)[0]
            if not in_ram(mtd):
                continue
            addr = base + off
            params = struct.unpack_from(">I", blob, off + 0xB0)[0]
            pos = room = None
            px, py, pz = (f32(addr + 0x1F8), f32(addr + 0x1FC), f32(addr + 0x200))
            if None not in (px, py, pz) and all(abs(v) < 1e7 for v in (px, py, pz)) \
               and (px or py or pz):
                pos = (round(px, 1), round(py, 1), round(pz, 1))
                r = rd(addr + 0x1F8 + 0x12, 1)
                if r and r[0] < 64:
                    room = r[0]
            found.append({
                "proc": names.get(pname, f"proc_{pname:#x}"),
                "dzr_names": ",".join(objs.get(names.get(pname, ""), [])),
                "addr": f"{addr:08x}",
                "params": f"{params:08x}",
                "x": pos[0] if pos else "", "y": pos[1] if pos else "",
                "z": pos[2] if pos else "", "room": room if room is not None else "",
            })
    return found


def report(rows, tag):
    print(f"\n===== LIVE CENSUS {tag} — {len(rows)} validated processes =====")
    c = Counter(r["proc"] for r in rows)
    for sym, n in c.most_common(80):
        sample = next((r for r in rows if r["proc"] == sym and r["x"] != ""), None)
        where = f"  @({sample['x']},{sample['y']},{sample['z']}) room={sample['room']}" \
                if sample else ""
        dzr = rows[[r["proc"] for r in rows].index(sym)]["dzr_names"]
        print(f"  {sym:26s} x{n:<3d}{('  dzr: ' + dzr) if dzr else '':28s}{where}")


def main():
    watch = "--watch" in sys.argv
    names, objs = proc_names(), objnames()
    print(f"symbols: {len(names)} proc names, {len(objs)} OBJNAME procs")
    print("waiting for Dolphin…")
    while True:
        dme.hook()
        if dme.is_hooked():
            break
        time.sleep(2)
    print("hooked.")

    out = OUT / "dusktap_scan_census.csv"
    fields = ["stamp", "proc", "dzr_names", "addr", "params", "x", "y", "z", "room"]
    f = open(out, "w", newline="", encoding="utf-8")
    w = csv.DictWriter(f, fieldnames=fields)
    w.writeheader()
    last = None
    try:
        while True:
            rows = scan(names, objs)
            if rows is None:
                print("UNKNOWN — no readable RAM (emulator paused/closed?)")
                time.sleep(2)
                if not dme.is_hooked():
                    print("waiting for Dolphin…")
                    while not dme.is_hooked():
                        time.sleep(2)
                        dme.hook()
                continue
            key = tuple(sorted(Counter(r["proc"] for r in rows).items()))
            if key != last:
                last = key
                stamp = time.strftime("%H:%M:%S")
                report(rows, stamp)
                for r in rows:
                    r["stamp"] = stamp
                    w.writerow(r)
                f.flush()
                print(f"  -> {len(rows)} rows appended ({out.name})")
            if not watch:
                break
            time.sleep(4)
    except KeyboardInterrupt:
        pass
    finally:
        f.close()
        print(f"\ncensus CSV: {out}")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
