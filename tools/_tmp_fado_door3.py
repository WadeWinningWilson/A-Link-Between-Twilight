# Parse F_SP103 doors with correct ACTR=0x20 / TGDR|SCOB=0x24 strides
import struct
import sys

sys.path.insert(0, r"%USERPROFILE%\Documents\dusklight\tools\demo_cut_content")
from dzr_placements import rarc_members

OUT = r"%USERPROFILE%\Documents\dusklight\tools\_tmp_fado_door3.out.txt"
lines = []


def dump_actors(blob, tag, num, off, stride):
    lines.append(f"{tag.decode()} num={num} stride={stride:#x}")
    for j in range(num):
        e = off + j * stride
        if e + 0x1E > len(blob):
            break
        name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
        params = struct.unpack_from(">I", blob, e + 8)[0]
        x, y, z = struct.unpack_from(">fff", blob, e + 0xC)
        ax, ay, az = struct.unpack_from(">hhh", blob, e + 0x18)
        msg = ax & 0xFFFF
        exit_no = (params >> 25) & 0x3F
        interesting = (
            "knob" in name.lower()
            or "door" in name.lower()
            or name.lower().startswith("kd")
            or msg not in (0, 0xFFFF)
            or exit_no in (3, 4, 5, 6, 7, 9)  # R_SP01-ish exits from SCLS
        )
        # Always list door-like; for others only if msg suggests lock flow
        doorlike = ("knob" in name.lower()) or ("door" in name.lower()) or (
            name.lower().startswith("kd")
        )
        if doorlike or (msg not in (0, 0xFFFF) and "Npc" in name):
            lines.append(
                f"  [{j:3d}] {name!r:10s} p={params:#010x} "
                f"pos=({x:8.1f},{y:7.1f},{z:8.1f}) "
                f"msgNo={msg:5d}/0x{msg:04x} ay={ay:6d} exit={exit_no}"
            )
        elif doorlike is False and interesting and msg not in (0, 0xFFFF):
            # skip noise
            pass
        if doorlike:
            pass


for room in ("R00_00.arc", "R01_00.arc"):
    path = rf"<decomp-root>\Ex TP\files\res\Stage\F_SP103\{room}"
    lines.append(f"=== {room}")
    for mname, blob in rarc_members(path):
        if mname != "room.dzr":
            continue
        # raw search any *door* / *knob* ascii
        for nd in (b"knob", b"Knob", b"door", b"Door", b"kdoo", b"Kdoo"):
            idx = 0
            while True:
                i = blob.find(nd, idx)
                if i < 0:
                    break
                # show surrounding 8-byte aligned name guess
                start = i
                # walk back to plausible name start (alnum)
                while start > 0 and blob[start - 1:start].isalnum():
                    start -= 1
                nm = blob[start : start + 8]
                lines.append(f"  bytehit {nd!r} @{i:#x} around={nm!r} hex={blob[start:start+0x20].hex()}")
                idx = i + 1

        n = struct.unpack_from(">I", blob, 0)[0]
        for i in range(n):
            tag = blob[4 + i * 12 : 8 + i * 12]
            num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
            if tag in (b"ACTR", b"ACT0", b"TGOB"):
                dump_actors(blob, tag, num, off, 0x20)
                # also print ALL names for ACTR
                lines.append(f"  ALL {tag.decode()} names:")
                for j in range(num):
                    e = off + j * 0x20
                    name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
                    params = struct.unpack_from(">I", blob, e + 8)[0]
                    ax = struct.unpack_from(">h", blob, e + 0x18)[0] & 0xFFFF
                    exit_no = (params >> 25) & 0x3F
                    lines.append(
                        f"    [{j:3d}] {name!r:10s} exit={exit_no:2d} msg={ax:5d} p={params:#010x}"
                    )
            if tag in (b"TGDR", b"SCOB"):
                dump_actors(blob, tag, num, off, 0x24)
                lines.append(f"  ALL {tag.decode()} names:")
                for j in range(num):
                    e = off + j * 0x24
                    name = blob[e : e + 8].split(b"\0")[0].decode("ascii", "replace")
                    params = struct.unpack_from(">I", blob, e + 8)[0]
                    ax = struct.unpack_from(">h", blob, e + 0x18)[0] & 0xFFFF
                    exit_no = (params >> 25) & 0x3F
                    x, y, z = struct.unpack_from(">fff", blob, e + 0xC)
                    lines.append(
                        f"    [{j:3d}] {name!r:10s} exit={exit_no:2d} msg={ax:5d} "
                        f"pos=({x:.1f},{y:.1f},{z:.1f}) p={params:#010x}"
                    )

with open(OUT, "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")
print("wrote", OUT)
