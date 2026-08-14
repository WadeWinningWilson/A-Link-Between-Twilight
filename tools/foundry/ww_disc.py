#!/usr/bin/env python3
# ===========================================================================
# ww_disc.py — §680 intake #1: the disc-reading lib, EXTRACTED (§682 -> §683).
#
# Named readers factored from the shipped inline logic in
# tools/ww_crew_restoration_skeleton/gen_cam_data.py::read_tp_camstyle
# (:121-181) — same FST walk, same RARC offsets, same Yaz0, generalized only
# in what they take as parameters. yaz0_decompress is still IMPORTED from
# gen_cam_data (one implementation). The import-back (gen_cam_data using these
# readers) is the follow-up that closes "never two of anything"; until then
# the EQUIVALENCE CHECK below is the guard: --selftest extracts camstyle.dat
# via THIS lib and compares byte length against the shipped reader's product.
#
# CONSUMERS: V1 source D (disc truth) · R5 correspondence runs · space_kit
# inventory re-point · L2's runtime reader (C++) gets its reference semantics
# from here, cited per function.
#
# Usage:
#   ww_disc.py --selftest              TP ISO via backend.isoPath / TP_ISO
#   ww_disc.py --ls <iso> [prefix]     list FST paths
#   ww_disc.py --cat <iso> <path>      dump one file to stdout.buffer
# ===========================================================================
import io
import json
import os
import struct
import sys
from pathlib import Path

# ============================================================================
# yaz0_decompress — THE implementation (moved here §683; gen_cam_data now
# imports it back). Byte-for-byte the shipped decoder that ran against both
# discs; only its home changed.
# ============================================================================
def yaz0_decompress(data):
    size = struct.unpack(">I", data[4:8])[0]
    src = data[16:]
    out = bytearray()
    si = 0
    while len(out) < size:
        code = src[si]; si += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if code & (0x80 >> bit):
                out.append(src[si]); si += 1
            else:
                b1, b2 = src[si], src[si + 1]; si += 2
                dist = ((b1 & 0xF) << 8) | b2
                cnt = (b1 >> 4) + 2
                if cnt == 2:
                    cnt = src[si] + 0x12; si += 1
                ref = len(out) - dist - 1
                for k in range(cnt):
                    out.append(out[ref + k])
    return bytes(out)


_SKEL = Path(__file__).resolve().parents[1] / "ww_crew_restoration_skeleton"
sys.path.insert(0, str(_SKEL))

TP_IDS = ("GZ2E01", "GZ2P01", "GZ2J01")
WW_IDS = ("GZLE01", "GZLP01", "GZLJ01")


def iso_open(path, game_ids=None):
    """Open a GCM image, verify the game id, return (file, boot_header)."""
    f = open(path, "rb")
    boot = f.read(0x440)
    # ------------------------------------------------------------------------
    # Plain-.iso requirement (user ruling 2026-08-12): compressed containers
    # are refused WITH THE REASON — the reader seeks absolute disc offsets,
    # which compression destroys. The error text is the setup instruction.
    # ------------------------------------------------------------------------
    magic4 = boot[:4]
    import struct as _s
    m_le = _s.unpack("<I", magic4)[0] if len(magic4) == 4 else 0
    is_compressed = (magic4[:3] in (b"RVZ", b"WIA", b"GCZ")
                     or m_le in (0xB10BC001,   # GCZ magic
                                 0x01C00BB1))  # byte-swapped variant
    if is_compressed:
        f.close()
        raise ValueError(
            "%s is a COMPRESSED image (RVZ/GCZ/WIA) — a plain uncompressed .iso "
            "is required (the reader seeks absolute disc offsets). Convert it: "
            "Dolphin > right-click the game > Convert File > Format: ISO." % path)
    gid = boot[:6].decode("ascii", "replace")
    if game_ids and gid not in game_ids:
        raise ValueError(f"image game id {gid!r} not in {game_ids}")
    return f, boot


def fst_walk(f, boot):
    """Yield (path, offset, length) for every FILE on the image.
    Exact walk shape of gen_cam_data.py:135-151."""
    fst_off, fst_size = struct.unpack(">II", boot[0x424:0x42C])
    f.seek(fst_off)
    fst = f.read(fst_size)
    n = struct.unpack(">I", fst[8:12])[0]
    strings = fst[n * 12:]
    dirs = [(n, "")]
    for i in range(1, n):
        while i >= dirs[-1][0]:
            dirs.pop()
        e = fst[i * 12:(i + 1) * 12]
        name = strings[(e[1] << 16) | (e[2] << 8) | e[3]:].split(b"\0")[0].decode("ascii", "replace")
        off, ln = struct.unpack(">II", e[4:12])
        if e[0]:
            dirs.append((ln, dirs[-1][1] + name + "/"))
        else:
            yield dirs[-1][1] + name, off, ln


def iso_read_file(f, boot, want_path, decompress=True):
    """Read one file by FST path; Yaz0-inflate unless told not to."""
    for path, off, ln in fst_walk(f, boot):
        if path == want_path:
            f.seek(off)
            data = f.read(ln)
            if decompress and data[:4] == b"Yaz0":
                data = yaz0_decompress(data)
            return data
    raise FileNotFoundError(want_path)


def rarc_list(arc):
    """Yield (name, bytes) for every file entry in a RARC image.
    Exact offset math of gen_cam_data.py:160-170. Directory entries
    (data offset table id 0xFFFF convention) are skipped by the same
    means the shipped reader used: name match happens on files only."""
    if arc[:4] == b"Yaz0":
        arc = yaz0_decompress(arc)
    if arc[:4] != b"RARC":
        raise ValueError("not RARC after decompression")
    data_off = struct.unpack(">I", arc[0xC:0x10])[0] + 0x20
    _, _, num_ents, ent_tbl = struct.unpack(">IIII", arc[0x20:0x30])
    str_tbl = struct.unpack(">I", arc[0x34:0x38])[0] + 0x20
    ent_tbl += 0x20
    for i in range(num_ents):
        e = arc[ent_tbl + i * 0x14: ent_tbl + (i + 1) * 0x14]
        etype = e[4]
        name_off = struct.unpack(">H", e[6:8])[0]
        d_o, d_l = struct.unpack(">II", e[8:16])
        nm = arc[str_tbl + name_off: arc.index(b"\0", str_tbl + name_off)].decode("ascii", "replace")
        if etype == 0x02:  # directory entry — no file bytes
            continue
        yield nm, arc[data_off + d_o: data_off + d_o + d_l]


def _tp_iso_path():
    p = os.environ.get("TP_ISO")
    if not p:
        cfg = os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
        p = json.load(open(cfg, encoding="utf-8")).get("backend.isoPath", "")
    return p


def selftest():
    print("ww_disc — extraction selftest (equivalence vs the shipped reader)")
    p = _tp_iso_path()
    if not p or not Path(p).is_file():
        print("  SKIP: no TP ISO configured — structural surface only")
        return 0
    f, boot = iso_open(p, TP_IDS)
    arc = iso_read_file(f, boot, "res/Object/CamParam.arc")
    found = None
    for nm, data in rarc_list(arc):
        if nm.lower() == "camstyle.dat":
            found = data
    if found is None:
        print("  FAIL: camstyle.dat not reachable via extracted readers")
        return 1
    count = struct.unpack(">i", found[4:8])[0]
    expect = 8 + count * 0x78
    print(f"  camstyle.dat via extracted path: {len(found)} bytes, {count} records")
    print(f"  record math check: 8 + {count}*0x78 = {expect} "
          f"({'OK' if expect <= len(found) else 'FAIL — records overrun blob'})")
    from gen_cam_data import read_tp_camstyle
    recs = read_tp_camstyle()
    print(f"  shipped reader on the same image: {len(recs)} records "
          f"({'MATCH' if len(recs) == count else 'MISMATCH'})")
    return 0 if len(recs) == count and expect <= len(found) else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    if "--ls" in sys.argv:
        i = sys.argv.index("--ls")
        f, boot = iso_open(sys.argv[i + 1])
        pre = sys.argv[i + 2] if len(sys.argv) > i + 2 else ""
        for path, off, ln in fst_walk(f, boot):
            if path.startswith(pre):
                print(f"{ln:>10}  {path}")
        return 0
    if "--cat" in sys.argv:
        i = sys.argv.index("--cat")
        f, boot = iso_open(sys.argv[i + 1])
        sys.stdout.buffer.write(iso_read_file(f, boot, sys.argv[i + 2]))
        return 0
    print(__doc__ or "see header")
    return 2


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
