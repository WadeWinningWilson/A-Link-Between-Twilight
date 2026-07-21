# Find knob/door actors + SCLS stride-13 in F_SP103
import struct
import sys

sys.path.insert(0, r"C:\Users\xxxxx\Documents\dusklight\tools\demo_cut_content")
from dzr_placements import rarc_members

OUT = r"C:\Users\xxxxx\Documents\dusklight\tools\_tmp_fado_door2.out.txt"
lines = []

needles = [
    b"knob00",
    b"Knob00",
    b"KNOB00",
    b"door00",
    b"Door00",
    b"DoorW00",
    b"doorW00",
    b"knob20",
    b"kd00\x00",
    b"Dbdoor",
    b"dbdoor",
]

for room in ("R00_00.arc", "R01_00.arc", "STG_00.arc"):
    path = rf"D:\XXXXXXX\Ex TP\files\res\Stage\F_SP103\{room}"
    for mname, blob in rarc_members(path):
        for nd in needles:
            idx = 0
            while True:
                i = blob.find(nd, idx)
                if i < 0:
                    break
                ctx = blob[max(0, i - 4) : i + 0x40]
                lines.append(f"{room}/{mname} @{i:#x} needle={nd!r}")
                # Align to possible ACTR start (name at i)
                if i + 0x20 <= len(blob):
                    name = blob[i : i + 8].split(b"\0")[0]
                    params = struct.unpack_from(">I", blob, i + 8)[0]
                    x, y, z = struct.unpack_from(">fff", blob, i + 0xC)
                    ax, ay, az = struct.unpack_from(">hhh", blob, i + 0x18)
                    lines.append(
                        f"  ACTR@name: {name!r} p={params:#010x} "
                        f"pos=({x:.2f},{y:.2f},{z:.2f}) "
                        f"msgNo={ax & 0xffff} ay={ay} az={az} "
                        f"exit={(params >> 25) & 0x3f} model={(params >> 5) & 7}"
                    )
                lines.append("  ctx=" + ctx.hex())
                idx = i + 1

# SCLS stride 13
lines.append("--- SCLS stride13 F_SP103/R00 ---")
for mname, blob in rarc_members(r"D:\XXXXXXX\Ex TP\files\res\Stage\F_SP103\R00_00.arc"):
    if mname != "room.dzr":
        continue
    n = struct.unpack_from(">I", blob, 0)[0]
    for i in range(n):
        tag = blob[4 + i * 12 : 8 + i * 12]
        num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
        if tag != b"SCLS":
            continue
        for j in range(num):
            e = off + j * 13
            name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
            b = blob[e + 8 : e + 13]
            lines.append(
                f"[{j:2d}] {name:8s} spawn={b[0]:3d} room={b[1]:3d} "
                f"b2={b[2]:#04x} b3={b[3]:#04x} b4={b[4]:#04x}"
            )

# Also check R01 SCLS for Link house etc
lines.append("--- SCLS stride13 F_SP103/R01 ---")
for mname, blob in rarc_members(r"D:\XXXXXXX\Ex TP\files\res\Stage\F_SP103\R01_00.arc"):
    if mname != "room.dzr":
        continue
    n = struct.unpack_from(">I", blob, 0)[0]
    for i in range(n):
        tag = blob[4 + i * 12 : 8 + i * 12]
        num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
        if tag != b"SCLS":
            continue
        for j in range(num):
            e = off + j * 13
            name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
            b = blob[e + 8 : e + 13]
            lines.append(
                f"[{j:2d}] {name:8s} spawn={b[0]:3d} room={b[1]:3d} "
                f"b2={b[2]:#04x} b3={b[3]:#04x} b4={b[4]:#04x}"
            )

# Search all stage for knob00 to confirm name spelling
lines.append("--- global knob00 hits in Stage (first 40) ---")
import os

STAGE = r"D:\XXXXXXX\Ex TP\files\res\Stage"
count = 0
for stage in sorted(os.listdir(STAGE)):
    sdir = os.path.join(STAGE, stage)
    if not os.path.isdir(sdir):
        continue
    for fn in sorted(os.listdir(sdir)):
        if not fn.lower().endswith(".arc"):
            continue
        try:
            for mname, blob in rarc_members(os.path.join(sdir, fn)):
                for nd in (b"knob00", b"Knob00"):
                    if nd in blob:
                        lines.append(f"HIT {stage}/{fn}/{mname} needle={nd!r}")
                        count += 1
                        if count >= 40:
                            break
                if count >= 40:
                    break
        except Exception as ex:
            lines.append(f"fail {stage}/{fn}: {ex}")
        if count >= 40:
            break
    if count >= 40:
        break

with open(OUT, "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")
print("wrote", OUT, "lines", len(lines))
