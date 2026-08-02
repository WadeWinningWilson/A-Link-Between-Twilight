#!/usr/bin/env python3
"""§273: bake the TALE entrance events into R_DL01's stage.dzs as a native
REVT chunk — the user's ALWAYS-NATIVE ruling executed (§272 option A).

DONOR TRUTH -> TP FORM
----------------------
Donor LinkRM stage.dzs EVNT rows: TALE_DEMO (idx 0) and TALE_DEMO2 (idx 4),
each with mSpawnSwitchNo = 0x02 (donor d_stage.h:335 — the one-shot
"already played by spawn" guard). TP's native equivalent is the REVT chunk ->
dStage_MapEventInfo_c entries (dStage_MapEvent_dt_c, 0x1C — d_stage.h:412),
loaded by dStage_stEventInfoInit (d_stage.cpp:2764) and consumed by
dEvent_exception_c::setStartDemo / getEventName + dEvt_control_c::
searchMapEventData (match on field_0x4 == mapToolID; switch_no already-done
guard honored natively).

FIELD TRANSLATION (receipts):
  type       = 1 (dStage_MapEvent_dt_TYPE_ZEV — event-list event by name)
  field_0x4  = mapToolID == THE ENTRY'S OWN INDEX (TP convention forced by
               getEventName's entries[mEventInfoIdx] indexing after
               setStartDemo stores the id): TALE_DEMO=0, TALE_DEMO2=1.
               ba1's cut adaptation passes 0 (donor 0xC8 branch) / 1 (0xCA).
  priority   = 0  [PORT-INTEGRATION: no TP REVT sample offline — verify
               against any original TP stage's REVT before shipping wide]
  event_name = donor name verbatim (fits char[13])
  switch_no  = 0x02 (donor mSpawnSwitchNo verbatim — replay guard preserved)
  all other bytes 0  [PORT-INTEGRATION: same sample-verify note]

Idempotent: existing REVT entries with the same event_name are left alone.
Backup: stage.dzs' arc gets a one-time .pre-revt-bak.

Usage: revt_bake.py [--dry-run]
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from adapt_bdl_arcs import be32

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"

# (event_name, mapToolID/index, switch_no) — donor EVNT receipts (§272/§273)
ENTRIES = [("TALE_DEMO", 0, 0x02), ("TALE_DEMO2", 1, 0x02)]
ENTRY_SIZE = 0x1C
TYPE_ZEV = 1


def build_entry(name: str, map_id: int, switch_no: int) -> bytes:
    e = bytearray(ENTRY_SIZE)
    e[0] = TYPE_ZEV
    e[4] = map_id            # field_0x4 — searchMapEventData key
    e[6] = 0                 # priority [PORT-INTEGRATION: sample-verify]
    nm = name.encode("ascii")
    assert len(nm) <= 13, name
    e[0xD:0xD + len(nm)] = nm
    e[0x1B] = switch_no      # donor mSpawnSwitchNo verbatim (replay guard)
    return bytes(e)


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    dzs = bytearray(members["stage.dzs"])

    n = be32(dzs, 0)
    existing_names = set()
    revt_idx = None
    for i in range(n):
        tag = dzs[4 + i * 0xC: 8 + i * 0xC].decode("ascii", "replace")
        cnt, off = be32(dzs, 8 + i * 0xC), be32(dzs, 12 + i * 0xC)
        if tag == "REVT":
            revt_idx = i
            for k in range(cnt):
                e = dzs[off + k * ENTRY_SIZE: off + (k + 1) * ENTRY_SIZE]
                existing_names.add(
                    bytes(e[0xD:0x1A]).split(b"\0")[0].decode("ascii",
                                                              "replace"))
    todo = [t for t in ENTRIES if t[0] not in existing_names]
    print(f"stage.dzs: {n} chunks; REVT "
          f"{'present' if revt_idx is not None else 'absent'}; "
          f"{len(todo)} entries to bake: {[t[0] for t in todo]}")
    if not todo:
        print("nothing to do (idempotent)")
        return 0
    if revt_idx is not None:
        raise SystemExit("REVT already exists — merge-into-existing not "
                         "implemented yet; inspect by hand (safety stop)")

    # ---- §277 OFFSET-STABLE APPEND (the RTBL lesson) --------------------
    # v1 rebuilt the dzs layout; RTBL carries THREE levels of absolute file
    # offsets (roomRead_class -> entry ptrs -> room lists, d_stage.cpp:2318)
    # and relayout corrupted them (History's count=128 receipt, §276-bis).
    # v2 moves NOTHING except the ONE lowest self-contained chunk:
    #   - STAG (fixed stage_stag_info struct, pointer-free) sits first in the
    #     data region; its blob relocates verbatim to EOF,
    #   - the freed front space holds the grown table's new REVT entry,
    #   - EVERY other byte of the file stays at its original offset, so all
    #     pointered chunks (RTBL, RCAM, ...) remain valid BY CONSTRUCTION.
    table_end = 4 + n * 0xC
    offs = []
    for i in range(n):
        tag = bytes(dzs[4 + i * 0xC: 8 + i * 0xC])
        cnt, off = be32(dzs, 8 + i * 0xC), be32(dzs, 12 + i * 0xC)
        offs.append([tag, cnt, off])
    lowest = min(offs, key=lambda e: e[2])
    if lowest[0] != b"STAG":
        raise SystemExit(f"lowest chunk is {lowest[0]!r}, not STAG — the "
                         "relocation-safety argument only covers STAG; stop")
    stag_off = lowest[2]
    stag_end = min(o for _t, _c, o in offs if o > stag_off)
    stag_blob = bytes(dzs[stag_off:stag_end])
    if stag_end - stag_off < 0xC:
        raise SystemExit("freed region smaller than one table entry — stop")

    out = bytearray(dzs)
    # STAG blob -> EOF (4-aligned), REVT entries after it
    while len(out) % 4:
        out.append(0)
    new_stag_off = len(out)
    out += stag_blob
    new_revt_off = len(out)
    new_entries = b"".join(build_entry(*t) for t in todo)
    out += new_entries
    # grown table: count, STAG entry retargeted, REVT entry into freed space
    struct.pack_into(">I", out, 0, n + 1)
    for i in range(n):
        if bytes(out[4 + i * 0xC: 8 + i * 0xC]) == b"STAG":
            struct.pack_into(">I", out, 12 + i * 0xC, new_stag_off)
    out[table_end:table_end + 0xC] = b"REVT" + struct.pack(
        ">II", len(todo), new_revt_off)
    for b in range(table_end + 0xC, stag_end):
        out[b] = 0

    # ---- verification battery -------------------------------------------
    # (a) untouched region byte-identical
    assert bytes(out[stag_end:len(dzs)]) == bytes(dzs[stag_end:]),         "untouched region changed — abort"
    # (b) offline RTBL probe (mirror of History's No93): room0 count must be 1
    for i in range(n + 1):
        if bytes(out[4 + i * 0xC: 8 + i * 0xC]) == b"RTBL":
            rcnt, roff = be32(out, 8 + i * 0xC), be32(out, 12 + i * 0xC)
            e0 = be32(out, roff)          # entry 0 ptr -> roomRead_data
            room0_count = out[e0]
            print(f"RTBL probe: {rcnt} rooms; room0 data @{e0:#x} "
                  f"count={room0_count}")
            assert room0_count == 1, "RTBL room0 count != 1 — abort"
    # verify with the same reader
    m = be32(out, 0)
    ok = False
    for i in range(m):
        if bytes(out[4 + i * 0xC: 8 + i * 0xC]) == b"REVT":
            cnt, off = be32(out, 8 + i * 0xC), be32(out, 12 + i * 0xC)
            got = [bytes(out[off + k * ENTRY_SIZE + 0xD:
                             off + k * ENTRY_SIZE + 0x1A]).split(b"\0")[0]
                   for k in range(cnt)]
            print(f"verify: REVT cnt={cnt} names={got} "
                  f"ids={[out[off + k * ENTRY_SIZE + 4] for k in range(cnt)]} "
                  f"switch={[out[off + k * ENTRY_SIZE + 0x1B] for k in range(cnt)]}")
            ok = (cnt == len(todo))
    if not ok:
        raise SystemExit("verify failed")
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "stage.dzs"]
    files.append(("stage.dzs", bytes(out)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-revt-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
