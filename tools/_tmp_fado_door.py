# Temporary: dump F_SP103 knobs / SCLS for Fado door ID research
import struct
import sys

sys.path.insert(0, r"C:\Users\xxxxx\Documents\dusklight\tools\demo_cut_content")
from dzr_placements import rarc_members

OUT = r"C:\Users\xxxxx\Documents\dusklight\tools\_tmp_fado_door.out.txt"


def dump_arc(arc, label, lines):
    lines.append(f"=== {label} {arc}")
    for mname, blob in rarc_members(arc):
        if not mname.lower().endswith((".dzr", ".dzs")):
            continue
        lines.append(f"member {mname} len={len(blob)}")
        n = struct.unpack_from(">I", blob, 0)[0]
        if n == 0 or n > 0x200:
            lines.append(f"  bad chunk count {n}")
            continue
        for i in range(n):
            tag = blob[4 + i * 12 : 8 + i * 12]
            num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
            if tag == b"SCLS":
                lines.append(f"SCLS num={num} off={off:#x}")
                lines.append("hex: " + blob[off : off + num * 12].hex())
                for j in range(num):
                    e = off + j * 12
                    raw = blob[e : e + 12]
                    name = raw[:8].split(b"\0")[0].decode("ascii", "replace")
                    b = raw[8:]
                    lines.append(
                        f"  [{j:2d}] {name!r:10s} spawn={b[0]:3d} room={b[1]:3d} "
                        f"b10={b[2]:#04x} b11={b[3]:#04x}"
                    )
            if tag in (b"ACTR", b"ACT0", b"TGDR", b"SCOB"):
                lines.append(f"{tag.decode()} num={num} off={off:#x}")
                counts = {}
                for j in range(num):
                    e = off + j * 0x40
                    if e + 0x40 > len(blob):
                        break
                    name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
                    counts[name] = counts.get(name, 0) + 1
                for k, v in sorted(counts.items()):
                    lines.append(f"  count {k}: {v}")
                for j in range(num):
                    e = off + j * 0x40
                    if e + 0x40 > len(blob):
                        break
                    name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
                    nl = name.lower()
                    if ("knob" in nl) or ("door" in nl) or nl in ("kd00", "kdoor"):
                        params = struct.unpack_from(">I", blob, e + 8)[0]
                        x, y, z = struct.unpack_from(">fff", blob, e + 0xC)
                        ax, ay, az = struct.unpack_from(">hhh", blob, e + 0x18)
                        lines.append(
                            f"  DOOR[{j}] {name!r} params={params:#010x} "
                            f"pos=({x:.2f},{y:.2f},{z:.2f}) "
                            f"ang=({ax},{ay},{az}) msgNo={ax & 0xffff} "
                            f"exit={(params >> 25) & 0x3f} model={(params >> 5) & 7}"
                        )
                    # Also dump any ACTR whose angle.x (msg) is non-zero / not -1
                    # for house doors we care about locked msgs
                    elif tag in (b"ACTR", b"ACT0"):
                        ax = struct.unpack_from(">h", blob, e + 0x18)[0]
                        axu = ax & 0xffff
                        if axu not in (0, 0xFFFF) and name:
                            # only log suspicious locked-msg candidates once we know knob name
                            pass


def main():
    lines = []
    for room in ("R00_00.arc", "R01_00.arc"):
        dump_arc(
            rf"D:\XXXXXXX\Ex TP\files\res\Stage\F_SP103\{room}",
            "F_SP103",
            lines,
        )
    # Also list R_SP01 room files for the gap claim
    import os

    rsp = r"D:\XXXXXXX\Ex TP\files\res\Stage\R_SP01"
    lines.append("=== R_SP01 files: " + ", ".join(sorted(os.listdir(rsp))))
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote", OUT)


if __name__ == "__main__":
    main()
