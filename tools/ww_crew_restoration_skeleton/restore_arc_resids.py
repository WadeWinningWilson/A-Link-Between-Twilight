#!/usr/bin/env python3
"""§268: restore DONOR numeric res-ids in adapted mod arcs (History's ferry #1).

THE BUG CLASS
-------------
Actor code resolves resources numerically: `getIDRes(arc, dRes_INDEX_*)` /
`getObjectIDRes` match the RARC FILE-ENTRY ID (u16 at entry+0). Arcs adapted
IN PLACE (adapt_bdl_arcs.adapt_arc) keep donor ids; arcs REBUILT through
pack_rarc get sequential ids (add_file counts payloads) — every donor numeric
lookup then returns the WRONG member: ba1 create_Anm crash + setAnm garbage
stretch (§266-bis), and the same latent bug in any other rebuilt arc.

THE FIX (data, not engine)
--------------------------
Patch the adapted arc's entry table in place: for every member whose NAME
exists in the donor arc, restore the donor's id. Members the mod ADDED (no
donor counterpart) are moved above the donor id range on collision. No
repack — only u16 id fields change, so offsets/sizes/hashes stay untouched.
Also emits residmap.csv (name, donor_id) per arc as the belt-and-suspenders
map History asked for as option (b).

DEFAULT IS AUDIT (no writes). --fix patches (one-time .pre-resid-bak).

Usage:
  restore_arc_resids.py [--arc Ba] [--fix]
     (default: audit EVERY mod arc that has a donor res/Object counterpart)
"""
from __future__ import annotations

import csv
import os
import struct
import sys
from pathlib import Path

from adapt_bdl_arcs import be32, yaz0_dec

MOD_ARCS = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
            / "model_replacements" / "WW-Crew-Restoration" / "arcs")
DONOR_OBJ = Path(r"D:/XXXXXXX/Ex WW/files/res/Object")


def entries(raw: bytearray) -> list[tuple[int, str, int]]:
    """[(entry_offset, name, id)] for file entries (not dirs)."""
    if bytes(raw[:4]) == b"Yaz0":
        raise SystemExit("compressed arc — decompress path not expected here")
    info = 0x20
    n = be32(raw, info + 0x08)
    ent = info + be32(raw, info + 0x0C)
    strs = info + be32(raw, info + 0x14)
    out = []
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", raw, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", raw, e + 6)[0]
        nm = bytes(raw[strs + no: raw.index(b"\0", strs + no)]).decode(
            "ascii", "replace")
        fid = struct.unpack_from(">H", raw, e)[0]
        out.append((e, nm, fid))
    return out


def load(path: Path) -> bytearray:
    d = bytearray(path.read_bytes())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)
    return d


def audit_one(name: str, fix: bool) -> tuple[int, int]:
    mod_p = MOD_ARCS / f"{name}.arc"
    don_p = DONOR_OBJ / f"{name}.arc"
    mod = load(mod_p)
    don = load(don_p)
    donor_ids = {nm: fid for _, nm, fid in entries(don)}
    mod_ents = entries(mod)
    mismatch, patched = [], 0
    used = set(donor_ids.values())
    next_free = (max(used) + 1) if used else 0
    for e, nm, fid in mod_ents:
        want = donor_ids.get(nm)
        if want is None:                      # mod-added member
            if fid in used:                   # collides with a donor id
                mismatch.append((nm, fid, f"added->relocate:{next_free}"))
                if fix:
                    struct.pack_into(">H", mod, e, next_free)
                next_free += 1
            continue
        if fid != want:
            mismatch.append((nm, fid, want))
            if fix:
                struct.pack_into(">H", mod, e, want)
                patched += 1
    # the residmap belt: name -> donor id, whatever the arc currently holds
    map_p = MOD_ARCS / f"{name}.residmap.csv"
    with open(map_p, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["member", "donor_res_id"])
        for nm, fid in sorted(donor_ids.items(), key=lambda kv: kv[1]):
            w.writerow([nm, fid])
    if mismatch:
        print(f"{name}.arc: {len(mismatch)} id mismatches"
              + (" — PATCHED" if fix else " (audit only)"))
        for nm, have, want in mismatch[:12]:
            print(f"    {nm}: {have} -> {want}")
        if fix:
            bak = mod_p.with_suffix(mod_p.suffix + ".pre-resid-bak")
            if not bak.is_file():
                bak.write_bytes(mod_p.read_bytes())
            mod_p.write_bytes(bytes(mod))
            # verify with the same reader
            chk = {nm: fid for _, nm, fid in entries(load(mod_p))}
            for nm, want in donor_ids.items():
                if nm in chk:
                    assert chk[nm] == want, f"verify failed on {nm}"
            print(f"    verified: all donor-named members carry donor ids")
    else:
        print(f"{name}.arc: OK (all donor-named member ids match donor)")
    return len(mismatch), patched


def main() -> int:
    only = (sys.argv[sys.argv.index("--arc") + 1]
            if "--arc" in sys.argv else None)
    fix = "--fix" in sys.argv
    names = ([only] if only else
             sorted(p.stem for p in MOD_ARCS.glob("*.arc")
                    if (DONOR_OBJ / p.name).is_file()
                    and not p.stem.endswith(".pre-resid-bak")))
    total = 0
    for n in names:
        try:
            m, _ = audit_one(n, fix)
            total += m
        except SystemExit as ex:
            print(f"{n}.arc: SKIP ({ex})")
    print(f"\n{len(names)} arcs audited; {total} total id mismatches"
          + ("" if fix else " — rerun with --fix to patch"))
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
