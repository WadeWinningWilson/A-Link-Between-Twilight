from pathlib import Path
import struct, os, sys

MOD = Path(os.environ["APPDATA"]) / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration"
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
for arcname in sys.argv[1:]:
    a = (MOD / "arcs" / arcname).read_bytes()
    data_abs = 0x20 + be32(a, 0x0C)
    info = 0x20
    n = be32(a, info + 8)
    ent = info + be32(a, info + 0xC)
    strs = info + be32(a, info + 0x14)
    print("==", arcname)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", a, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", a, e + 6)[0]
        end = a.index(b"\0", strs + no)
        name = a[strs + no : end].decode("ascii", "replace")
        print(" ", name)
