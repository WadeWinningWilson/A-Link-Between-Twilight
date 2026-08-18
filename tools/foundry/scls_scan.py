# ============================================================================
# scls_scan.py - WHO WARPS INTO Ojhous vs Ojhous2? Walk the SCLS (exit) tables
# in the donor's own stage files. Read-only; scratchpad Yaz0 (the plugin's
# Yaz0 refusal stays intact - this is a measurement, not shipped code).
# ============================================================================
import struct
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
ROOT = Path(r"D:\XXXXXXX\Ex WW\files\res\Stage")


def yaz0(d):
    if d[:4] != b"Yaz0":
        return d
    size = struct.unpack_from(">I", d, 4)[0]
    out = bytearray()
    i = 16
    code, bits = 0, 0
    while len(out) < size:
        if bits == 0:
            code = d[i]; i += 1; bits = 8
        if code & 0x80:
            out.append(d[i]); i += 1
        else:
            b1, b2 = d[i], d[i + 1]; i += 2
            dist = ((b1 & 0xF) << 8) | b2
            copy = i and ((b1 >> 4) or (d[i] + 0x12, ))
            n = (b1 >> 4)
            if n == 0:
                n = d[i] + 0x12; i += 1
            else:
                n += 2
            src = len(out) - dist - 1
            for _ in range(n):
                out.append(out[src]); src += 1
        code <<= 1; bits -= 1
    return bytes(out)


def be32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def rarc_members(data):
    hdr = 0x20
    node_cnt = be32(data, hdr)
    node_off = be32(data, hdr + 4) + hdr
    ent_off = be32(data, hdr + 0x0C) + hdr
    str_off = be32(data, hdr + 0x14) + hdr
    fdat = be32(data, 0x0C) + hdr
    for n in range(node_cnt):
        no = node_off + n * 0x10
        first = be32(data, no + 0x0C)
        cnt = struct.unpack_from(">H", data, no + 0x0A)[0]
        for i in range(cnt):
            eo = ent_off + (first + i) * 0x14
            typ = struct.unpack_from(">H", data, eo + 4)[0]
            name_o = struct.unpack_from(">H", data, eo + 6)[0]
            end = data.index(b"\0", str_off + name_o)
            name = data[str_off + name_o:end].decode("ascii", "replace")
            if typ & 0x1100 == 0x1100 or (typ & 0x0200) == 0:
                off = be32(data, eo + 8)
                size = be32(data, eo + 0x0C)
                yield name, data[fdat + off: fdat + off + size]


def scls(dz):
    """Walk the chunk table; yield SCLS entries (stage, start, room)."""
    n = be32(dz, 0)
    for c in range(n):
        tag = dz[4 + c * 12: 8 + c * 12]
        cnt = be32(dz, 8 + c * 12)
        off = be32(dz, 12 + c * 12)
        if tag == b"SCLS":
            for e in range(cnt):
                eo = off + e * 12
                stage = dz[eo:eo + 8].split(b"\0")[0].decode("ascii", "replace")
                start, room = dz[eo + 8], dz[eo + 9]
                yield stage, start, room


def parents():
    """PARENTAGE FROM THE EXIT GRAPH: every stage whose SCLS returns to
    `sea` names its parent island's room number in its own data. Walk
    EVERY stage dir; print stage -> {sea rooms}, plus non-sea exits so
    nested parentage (cave -> island via another stage) is visible too."""
    table = {}
    others = {}
    failures = []
    for d in sorted(p for p in ROOT.iterdir() if p.is_dir()):
        rooms, ext = set(), set()
        for arc in sorted(d.glob("*.arc")):
            try:
                raw = yaz0(arc.read_bytes())
                if raw[:4] != b"RARC":
                    failures.append(str(arc)); continue
                for name, body in rarc_members(raw):
                    if not name.lower().endswith((".dzr", ".dzs")):
                        continue
                    for stage, start, room in scls(body):
                        if stage == "sea":
                            rooms.add(room)
                        elif stage and stage != d.name:
                            ext.add(stage)
            except Exception as e:
                failures.append("%s: %s" % (arc, e))
        if rooms:
            table[d.name] = rooms
        elif ext:
            others[d.name] = ext
    print("== stages returning DIRECTLY to sea (stage -> island rooms) ==")
    for k in sorted(table):
        print("  %-10s -> %s" % (k, sorted(table[k])))
    print("== stages with only non-sea exits (nested; resolve via target) ==")
    for k in sorted(others):
        print("  %-10s -> %s" % (k, sorted(others[k])))
    if failures:
        print("== %d arc(s) unreadable - listed, not skipped silently ==" % len(failures))
        for f in failures[:10]:
            print("  %s" % f)


def plyr(dz):
    """PLYR (spawn point) entries. Layout from the donor's OWN reader
    (d_stage.cpp:1428 dStage_playerInit): entries are stage_actor_data_class,
    0x20 stride - name[8], params u32, pos f32[3], angle u16[3], setID u16 -
    and THE SPAWN ID IS `(u8)angle.z` (:1457), i.e. the LOW byte of the
    third big-endian u16 at entry+0x1C -> byte @ +0x1D."""
    n = be32(dz, 0)
    for c in range(n):
        tag = dz[4 + c * 12: 8 + c * 12]
        cnt = be32(dz, 8 + c * 12)
        off = be32(dz, 12 + c * 12)
        if tag == b"PLYR":
            for e in range(cnt):
                eo = off + e * 0x20
                name = dz[eo:eo + 8].split(b"\0")[0].decode("ascii", "replace")
                spawn = dz[eo + 0x1D]
                yield name, spawn


def spawn_records(stages):
    """FULL PLYR records for named stages: point id, name, params, POSITION
    (f32[3] BE @ +0xC), angle (u16[3] @ +0x18). Layout = the donor's own
    stage_actor_data_class; id = byte @ +0x1D (dStage_playerInit:1457)."""
    for want in stages:
        d = ROOT / want
        if not d.is_dir():
            print("%s: NO SUCH STAGE DIR" % want)
            continue
        for arc in sorted(d.glob("*.arc")):
            raw = yaz0(arc.read_bytes())
            if raw[:4] != b"RARC":
                continue
            for name, body in rarc_members(raw):
                if not name.lower().endswith((".dzr", ".dzs")):
                    continue
                n = be32(body, 0)
                for c in range(n):
                    tag = body[4 + c * 12: 8 + c * 12]
                    cnt = be32(body, 8 + c * 12)
                    off = be32(body, 12 + c * 12)
                    if tag != b"PLYR":
                        continue
                    for e in range(cnt):
                        eo = off + e * 0x20
                        nm = body[eo:eo + 8].split(b"\0")[0].decode(
                            "ascii", "replace")
                        prm = be32(body, eo + 8)
                        px, py, pz = struct.unpack_from(">3f", body, eo + 0xC)
                        ax, ay, az = struct.unpack_from(">3H", body, eo + 0x18)
                        spawn = body[eo + 0x1D]
                        print("%s/%s %-8s point=%3d params=0x%08X "
                              "pos=(%.1f, %.1f, %.1f) angleY=0x%04X"
                              % (want, arc.stem, nm, spawn, prm,
                                 px, py, pz, ay))


def points():
    """Per stage: which spawn-point IDs exist, from every room/stage file."""
    for d in sorted(p for p in ROOT.iterdir() if p.is_dir()):
        found = {}
        for arc in sorted(d.glob("*.arc")):
            try:
                raw = yaz0(arc.read_bytes())
                if raw[:4] != b"RARC":
                    continue
                for name, body in rarc_members(raw):
                    if not name.lower().endswith((".dzr", ".dzs")):
                        continue
                    for pname, spawn in plyr(body):
                        found.setdefault(arc.stem, []).append((spawn, pname))
            except Exception as e:
                print("  %s UNREADABLE: %s" % (arc, e))
        if found:
            parts = []
            for room in sorted(found):
                ids = sorted({s for s, _ in found[room]})
                parts.append("%s:[%s]" % (room, ",".join(map(str, ids))))
            print("%-10s %s" % (d.name, " ".join(parts)))


def main():
    targets = []
    for sub in ("sea", "Ojhous", "Ojhous2", "LinkRM", "Omasao", "Onobuta"):
        d = ROOT / sub
        if d.is_dir():
            targets += sorted(d.glob("*.arc"))
    for arc in targets:
        raw = yaz0(arc.read_bytes())
        if raw[:4] != b"RARC":
            print("%s: not RARC after decode" % arc)
            continue
        for name, body in rarc_members(raw):
            if not name.lower().endswith((".dzr", ".dzs")):
                continue
            rows = list(scls(body))
            hits = [r for r in rows if "jhous" in r[0].lower()]
            label = "%s :: %s" % (arc.relative_to(ROOT), name)
            if hits:
                for stage, start, room in hits:
                    print("%-40s -> %-8s start %d room %d"
                          % (label, stage, start, room))
            elif arc.parent.name.startswith("Ojhous"):
                # inside the houses print ALL exits so the way OUT is seen too
                for stage, start, room in rows:
                    print("%-40s => %-8s start %d room %d"
                          % (label, stage, start, room))


if __name__ == "__main__":
    if "--parents" in sys.argv:
        parents()
    elif "--points" in sys.argv:
        points()
    elif "--spawns" in sys.argv:
        i = sys.argv.index("--spawns")
        spawn_records(sys.argv[i + 1:])
    else:
        main()
