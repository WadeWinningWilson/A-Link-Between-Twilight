from pathlib import Path
import struct, os

MOD = Path(os.environ["APPDATA"]) / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration"
arc = (MOD / "arcs/A_mori.arc").read_bytes()
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
data_abs = 0x20 + be32(arc, 0x0C)
info = 0x20
n = be32(arc, info + 8)
ent = info + be32(arc, info + 0xC)
strs = info + be32(arc, info + 0x14)
dzr = None
for i in range(n):
    e = ent + i * 0x14
    if struct.unpack_from(">H", arc, e + 4)[0] & 0x1100 != 0x1100:
        continue
    no = struct.unpack_from(">H", arc, e + 6)[0]
    end = arc.index(b"\0", strs + no)
    name = arc[strs + no : end].decode()
    if name != "room.dzr":
        continue
    off, size = be32(arc, e + 8), be32(arc, e + 12)
    dzr = arc[data_abs + off : data_abs + off + size]
    print("dzr size", size)
print(dzr[:128].hex())
print("--- printable 4-char runs ---")
for j in range(0, len(dzr) - 3):
    t = dzr[j : j + 4]
    if all(65 <= b <= 90 or 48 <= b <= 57 for b in t):  # A-Z0-9
        nxt = be32(dzr, j + 4) if j + 8 <= len(dzr) else -1
        if 0 < nxt < 500:
            print(f"{j:5} {t.decode()} count?={nxt}")
# Also try LinkRM from library for known-good format
for stage in ["LinkRM", "Outset"]:
    p = MOD / "arcs" / f"{stage}.arc"
    if not p.is_file():
        continue
    a = p.read_bytes()
    data_abs = 0x20 + be32(a, 0x0C)
    info = 0x20
    n = be32(a, info + 8)
    ent = info + be32(a, info + 0xC)
    strs = info + be32(a, info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", a, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", a, e + 6)[0]
        end = a.index(b"\0", strs + no)
        name = a[strs + no : end].decode()
        if name.lower() != "room.dzr":
            continue
        off, size = be32(a, e + 8), be32(a, e + 12)
        d = a[data_abs + off : data_abs + off + size]
        print(f"\n{stage} room.dzr size={size} head={d[:64].hex()}")
        for j in range(0, min(len(d), 2000) - 3):
            t = d[j : j + 4]
            if t in (b"ACTR", b"ACT0", b"SCOB", b"TGDR", b"SCLS"):
                print(f"  @{j} {t} u32={be32(d,j+4)}")
