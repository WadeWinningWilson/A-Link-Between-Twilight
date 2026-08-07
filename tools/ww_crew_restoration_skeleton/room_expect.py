#!/usr/bin/env python3
# ============================================================================
# room_expect.py — §389: emit the ROOM EXPECTATION MANIFEST (donor truth) that
# the runtime verifier checks against.
#
# WHY THIS EXISTS (user's question, and it is the right one): the "Ivan" — TP's
# same-named actor filling a slot we failed to fill — was an ACCIDENTAL
# detector. It made an absence visible. Every silent absence this campaign hit
# (arcs staged to the wrong directory, unported procs, invisible materials, a
# particle latch that failed once and stayed quiet) was invisible until a human
# noticed with their eyes and remembered vanilla WW. That is not measurement.
#
# This file is the EXPECTATION half of a real detector: for each hosted room,
# what the DONOR says should be there — every dzr actor row (all layers), with
# the receiver proc each name resolves to, and its authored position. The
# runtime half (d_ext_room_verify.cpp) walks the live actor list and reports
# PRESENT / SUBSTITUTED / MISSING per row, so an empty slot becomes a log line
# naming the donor row instead of a thing nobody notices.
#
# Sources of truth (no guessing):
#   donor dzr ACTR/SCOB/ACTn/SCOn rows  -> what the room authors
#   RECEIVER d_stage.cpp OBJNAME rows   -> which proc that name resolves to HERE
#   (a name with no receiver OBJNAME row is emitted as proc=-1 = "deferred",
#    which the verifier reports as EXPECTED-DEFERRED, never as a pass)
#
# Usage:
#   room_expect.py <donor-stage> <host-stage> <host-room> [--out <file>]
#   e.g. room_expect.py LinkRM R_DL01 0
# Writes <MOD>/npc/room_expect.csv (append/replace by host stage+room).
# ============================================================================
import csv
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from jstudio_stb import yaz0_dec, rarc_members

RECEIVER = Path("C:/Users/xxxxx/Documents/dusklight")
DONOR_STAGE = Path("D:/XXXXXXX/Ex WW/files/res/Stage")
MOD = Path("C:/Users/xxxxx/AppData/Roaming/TwilitRealm/Dusklight/"
           "model_replacements/WW-Crew-Restoration")
OUT = MOD / "npc" / "room_expect.csv"

be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]


def census_routed_names(mod_root):
    """§394 — dzr name -> proc, via the CENSUS route (population/actor_map.ini).

    THE BLIND SPOT THIS CLOSES: a donor name reaches a receiver actor by one of
    TWO routes, and this tool only knew about one. `d_stage.cpp`'s OBJNAME table
    is the engine route; `population/actor_map.ini` is the census route, where a
    section `[name]` with `proc=NPC_X` has the population spawner create that
    actor for every matching census row. `swood` travels the second route.

    Because only the first was consulted, both LinkRM `swood` rows were emitted
    as proc=-1 and the verifier reported them DEFERRED on every single run —
    while the user was reporting exactly those two plants as missing. The
    instrument was printing the answer in a column that meant "expected gap".
    A DEFERRED verdict is only honest if the tool has checked EVERY route a name
    can take.
    """
    out = {}
    p = mod_root / "population" / "actor_map.ini"
    if not p.exists():
        return out
    section = None
    for line in p.read_text(encoding="utf-8-sig", errors="replace").splitlines():
        line = line.strip()
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1]
        elif section and line.lower().startswith("proc="):
            out[section] = line.split("=", 1)[1].strip()
    return out


def census_proc_sockets(mod_root):
    """§394 — census proc token -> receiver socket, from the npc/*.ini manifests.

    `actor_map.ini` names a proc token (e.g. `NPC_EXTVEG`); the manifest that
    declares that token also declares the receiver socket it resolves to
    (`socket=EXT_VEG`), which is the enum symbol `fpcNm_EXT_VEG_e`. Reading both
    files keeps the chain data-driven — the alternative is guessing an enum
    symbol from a proc token by string surgery, which is how names get silently
    mis-resolved.
    """
    out = {}
    npc = mod_root / "npc"
    if not npc.is_dir():
        return out
    for p in sorted(npc.glob("*.ini")):
        proc = socket = None
        for line in p.read_text(encoding="utf-8-sig", errors="replace").splitlines():
            line = line.strip()
            if line.lower().startswith("proc="):
                proc = line.split("=", 1)[1].strip()
            elif line.lower().startswith("socket="):
                socket = line.split("=", 1)[1].strip()
        if proc and socket:
            out[proc] = socket
    return out


def receiver_objnames():
    """dzr name -> (proc symbol, enum index) from the RECEIVER's own table."""
    txt = (RECEIVER / "src/d/d_stage.cpp").read_text(encoding="utf-8",
                                                     errors="replace")
    names = {}
    for m in re.finditer(r'OBJNAME\("([^"]+)",\s*(fpcNm_\w+)', txt):
        names[m.group(1)] = m.group(2)
    # proc symbol -> numeric index, from the receiver's name enum
    idx = {}
    nh = (RECEIVER / "include/f_pc/f_pc_name.h").read_text(encoding="utf-8",
                                                           errors="replace")
    for m in re.finditer(r"/\*\s*0x([0-9A-Fa-f]+)\s*\*/\s*X\((fpcNm_\w+)\)", nh):
        idx[m.group(2)] = int(m.group(1), 16)
    return names, idx


def dzr_rows(arc_path):
    raw = arc_path.read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    dzr = None
    for nm, blob in rarc_members(raw):
        if nm.lower().endswith(".dzr"):
            dzr = yaz0_dec(blob) if blob[:4] == b"Yaz0" else blob
    if dzr is None:
        return []
    out = []
    n = be32(dzr, 0)
    for i in range(n):
        tag = dzr[4 + i * 12:8 + i * 12].decode("ascii", "replace")
        cnt, off = struct.unpack_from(">II", dzr, 8 + i * 12)
        base = tag[:3]
        if base not in ("ACT", "SCO", "DOO", "TGO", "TGD"):
            continue
        esz = 0x20 if tag.startswith("ACT") else 0x24
        for k in range(cnt):
            e = off + k * esz
            name = dzr[e:e + 8].split(b"\0")[0].decode("ascii", "replace")
            params = be32(dzr, e + 8)
            x, y, z = struct.unpack_from(">fff", dzr, e + 12)
            layer = "-" if tag in ("ACTR", "SCOB", "DOOR", "TGOB", "TGDR") \
                else tag[3:]
            out.append((tag, layer, name, params, x, y, z))
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) < 3:
        sys.exit("usage: room_expect.py <donor-stage> <host-stage> <host-room>")
    donor, host, room = args[0], args[1], int(args[2])
    rooms = sorted((DONOR_STAGE / donor).glob("Room*.arc"))
    if not rooms:
        sys.exit(f"no Room*.arc in {DONOR_STAGE / donor}")
    objnames, enum_idx = receiver_objnames()
    # §394 — the second route. mod root is <...>/WW-Crew-Restoration (this file
    # writes npc/room_expect.csv under it).
    mod_root = OUT.parent.parent
    census = census_routed_names(mod_root)
    sockets = census_proc_sockets(mod_root)

    rows = []
    for arc in rooms:
        for tag, layer, name, params, x, y, z in dzr_rows(arc):
            proc_sym = objnames.get(name, "")
            proc_idx = enum_idx.get(proc_sym, -1) if proc_sym else -1
            if not proc_sym and name in census:
                # Census route: actor_map [name] -> proc token -> the npc_*.ini
                # that declares it -> its socket -> the receiver enum symbol.
                # Every hop is read from data; nothing is inferred from the name.
                sock = sockets.get(census[name])
                if sock:
                    cand = f"fpcNm_{sock}_e"
                    if cand in enum_idx:
                        proc_sym, proc_idx = cand, enum_idx[cand]
            rows.append({
                "host_stage": host, "host_room": room,
                "donor_stage": donor, "donor_room": arc.stem,
                "chunk": tag, "layer": layer, "name": name,
                "params": f"{params:08x}",
                "x": round(x, 1), "y": round(y, 1), "z": round(z, 1),
                "proc": proc_sym or "-", "proc_idx": proc_idx,
            })

    fields = ["host_stage", "host_room", "donor_stage", "donor_room", "chunk",
              "layer", "name", "params", "x", "y", "z", "proc", "proc_idx"]
    existing = []
    if OUT.is_file():
        with open(OUT, encoding="utf-8") as f:
            existing = [r for r in csv.DictReader(f)
                        if not (r["host_stage"] == host
                                and int(r["host_room"]) == room)]
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(existing)
        w.writerows(rows)

    deferred = [r["name"] for r in rows if r["proc_idx"] < 0]
    print(f"{len(rows)} expected rows for {host} room {room} (donor {donor}) "
          f"-> {OUT}")
    print(f"  resolvable now: {len(rows) - len(deferred)}   "
          f"deferred (no receiver OBJNAME): {sorted(set(deferred)) or 'none'}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
