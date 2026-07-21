# Dump unique kdoor placements from F_SP103 R00 (dedupe by pos)
import struct
import sys

sys.path.insert(0, r"C:\Users\xxxxx\Documents\dusklight\tools\demo_cut_content")
from dzr_placements import rarc_members

OUT = r"C:\Users\xxxxx\Documents\dusklight\tools\_tmp_fado_door4.out.txt"
lines = []

# Map SCLS exit index -> dest
scls = {}
for mname, blob in rarc_members(r"D:\XXXXXXX\Ex TP\files\res\Stage\F_SP103\R00_00.arc"):
    if mname != "room.dzr":
        continue
    n = struct.unpack_from(">I", blob, 0)[0]
    lines.append("chunk directory:")
    for i in range(n):
        tag = blob[4 + i * 12 : 8 + i * 12].decode("ascii", "replace")
        num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
        lines.append(f"  {tag!r} num={num} off={off:#x}")
        if tag == "SCLS":
            for j in range(num):
                e = off + j * 13
                name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
                b = blob[e + 8 : e + 13]
                scls[j] = (name, b[0], b[1])
                lines.append(f"    SCLS[{j}] -> {name} spawn={b[0]} room={b[1]}")

    # Collect all kdoor occurrences, keep unique by rounded pos
    seen = {}
    idx = 0
    while True:
        i = blob.find(b"kdoor\x00\x00\x00", idx)
        if i < 0:
            break
        params = struct.unpack_from(">I", blob, i + 8)[0]
        x, y, z = struct.unpack_from(">fff", blob, i + 0xC)
        ax, ay, az = struct.unpack_from(">hhh", blob, i + 0x18)
        msg = ax & 0xFFFF
        exit_no = (params >> 25) & 0x3F
        model = (params >> 5) & 7
        key = (round(x, 1), round(y, 1), round(z, 1), exit_no, msg)
        if key not in seen:
            dest = scls.get(exit_no)
            dest_s = f"{dest[0]} s={dest[1]} r={dest[2]}" if dest else "?"
            seen[key] = {
                "off": i,
                "params": params,
                "pos": (x, y, z),
                "msg": msg,
                "ay": ay,
                "exit": exit_no,
                "model": model,
                "dest": dest_s,
            }
        idx = i + 1

    lines.append(f"\nUnique kdoor placements ({len(seen)}):")
    # Sort by exit then x
    for k, v in sorted(seen.items(), key=lambda kv: (kv[1]["exit"], kv[1]["pos"][0])):
        x, y, z = v["pos"]
        lines.append(
            f"  exit={v['exit']:2d} msg=0x{v['msg']:04x}({v['msg']:5d}) "
            f"model={v['model']} ay={v['ay']:6d} "
            f"pos=({x:9.2f},{y:8.2f},{z:9.2f}) "
            f"p={v['params']:#010x} -> {v['dest']} @{v['off']:#x}"
        )

    # Highlight locked (msg != 0xffff) pointing at missing rooms 3 or 6
    lines.append("\nLocked doors (msgNo != 0xFFFF):")
    for k, v in sorted(seen.items(), key=lambda kv: kv[1]["exit"]):
        if v["msg"] != 0xFFFF:
            x, y, z = v["pos"]
            lines.append(
                f"  *** exit={v['exit']} msg=0x{v['msg']:04x} "
                f"pos=({x:.2f},{y:.2f},{z:.2f}) -> {v['dest']}"
            )

with open(OUT, "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")
print("wrote", OUT)
