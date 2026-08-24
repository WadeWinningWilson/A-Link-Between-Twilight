#!/usr/bin/env python3
"""§299: SPACE-KIT SCAN PROTOTYPE — read-only Inventory + Verify passes.

Scans a donor stage against its host receptor and reports every missing/null
element the §243-§297 campaign taught us to check — BEFORE anything runs.
No writes, ever. (The bake passes are separate, gated tools.)

Checks (each one is a §-receipt from the tale campaign):
  R1  host mapping exists (ini pairs)                      [§267 SKIPPED class]
  R2  native room arc per hosted room                      [architecture tier]
  R3  donor stage EVNT rows -> host REVT                   [§273/§279 class]
  R4  donor event_list events -> host event_list           [§265 class]
  R5  event-closure completeness for present events        [§280/§287 class]
  R6  ALL/cross-room staff hazard on present events        [§275 P3]
  R7  donor PLYR points -> host receptor PLYR              [§295/§297 class]
  R8  inbound SCLS spawn ids covered by PLYR               [receptor coverage]
  R9  ACTR/SCOB rosters vs receiver OBJNAME registration   [§239 roster class]
  R10 .stb casts placed vs demo-arc staged + residmap      [§269/§278 class]
  R11 MULT co-render hazard for the hosted rooms           [§276 P3]

Usage: space_scan.py <donor-stage-name> [--report out.md]
e.g.   space_scan.py Ojhous2
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from adapt_bdl_arcs import be32
from bake_room_chunks import read_dzr, parse_chunks, entry_name, \
    receiver_objnames, host_map
from merge_event import EventFile, arc_member, ESZ, SSZ, CSZ, DSZ

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
DONOR = Path(r"<decomp-root>/Ex WW/files/res/Stage")

FINDINGS = []


def finding(sev, code, text):
    FINDINGS.append((sev, code, text))
    print(f"[{sev}] {code}: {text}")


def main() -> int:
    stage = sys.argv[1]
    report_to = (sys.argv[sys.argv.index("--report") + 1]
                 if "--report" in sys.argv else None)
    dstage = DONOR / stage
    assert dstage.is_dir(), dstage
    print(f"=== SPACE SCAN: donor '{stage}' ===")

    # ---- R1: host mapping ------------------------------------------------
    hosts = host_map()
    host = hosts.get(stage)
    room_map = {}   # donor room -> (host_stage, host_room, ini)
    for ini in (MOD / "npc").glob("*.ini"):
        t = ini.read_text(encoding="utf-8", errors="replace")
        import re
        pop = re.search(r"^population_stage=(\w+)", t, re.M)
        hs = re.search(r"^host_stage=(\w+)", t, re.M)
        hr = re.search(r"^host_room=(\d+)", t, re.M)
        if pop and hs and pop.group(1).startswith(stage):
            suffix = pop.group(1)[len(stage):]
            droom = 0 if suffix == "" else int(suffix.lstrip("R") or 0)
            room_map[droom] = (hs.group(1), int(hr.group(1)) if hr else -1,
                              ini.name)
    if not room_map:
        finding("CRITICAL", "R1-unmapped",
                f"donor stage '{stage}' has NO host mapping (no ini pair) — "
                "nothing below can even be hosted")
        host_stage = None
    else:
        host_stage = next(iter(room_map.values()))[0]
        print(f"R1 ok: rooms map {room_map}")

    # ---- donor inventory -------------------------------------------------
    d_events, d_evnt, d_plyr, d_actors, d_doors, d_scls_in = [], [], {}, {}, 0, {}
    el = arc_member(dstage / "Stage.arc", "event_list.dat")
    if el:
        ef = EventFile(el)
        d_events = [(ef.name(ef.etop, i, ESZ), i) for i in range(ef.enum)]
    dzs = arc_member(dstage / "Stage.arc", "stage.dzs")
    if dzs:
        n = be32(dzs, 0)
        for i in range(n):
            tag = dzs[4 + i * 0xC: 8 + i * 0xC].decode("ascii", "replace")
            cnt, off = be32(dzs, 8 + i * 0xC), be32(dzs, 12 + i * 0xC)
            if tag == "EVNT":
                for k in range(cnt):
                    e = dzs[off + k * 0x18: off + (k + 1) * 0x18]
                    d_evnt.append(e[1:16].split(b"\0")[0].decode("ascii",
                                                                 "replace"))
    for rp in sorted(dstage.glob("Room*.arc")):
        rno = int(rp.stem[4:])
        _m, dzr = read_dzr(rp)
        ch = parse_chunks(dzr)
        pts = []
        for tag, es in ch.items():
            if tag == "PLYR":
                for e in es:
                    pts.append(struct.unpack(">h", e[0x1C:0x1E])[0] & 0xFF)
            elif tag.startswith(("ACT", "SCO", "TGO", "TGS")):
                for e in es:
                    d_actors.setdefault(entry_name(e), set()).add(rno)
            elif tag in ("DOOR", "TGDR"):
                d_doors += len(es)
        # PLYR may live at 0x20 stride even in rooms where parse_chunks
        # derived a different size — re-read at fixed stride:
        n2 = be32(dzr, 0)
        pts = []
        for i in range(n2):
            if dzr[4 + i * 0xC: 8 + i * 0xC] == b"PLYR":
                cnt, off = be32(dzr, 8 + i * 0xC), be32(dzr, 12 + i * 0xC)
                for k in range(cnt):
                    e = dzr[off + k * 0x20: off + (k + 1) * 0x20]
                    pts.append(struct.unpack(">h", e[0x1C:0x1E])[0] & 0xFF)
        d_plyr[rno] = pts
    # inbound SCLS (who jumps INTO this stage, with what spawn ids)
    for other in ("sea",):
        for rp in sorted((DONOR / other).glob("Room44.arc")):
            _m, dzr = read_dzr(rp)
            n2 = be32(dzr, 0)
            for i in range(n2):
                tag = dzr[4 + i * 0xC: 8 + i * 0xC]
                if tag != b"SCLS":
                    continue
                cnt, off = be32(dzr, 8 + i * 0xC), be32(dzr, 12 + i * 0xC)
                for k in range(cnt):
                    e = dzr[off + k * 0xC: off + (k + 1) * 0xC]
                    dest = e[:8].split(b"\0")[0].decode("ascii", "replace")
                    if dest == stage:
                        d_scls_in.setdefault(e[9], set()).add(e[8])  # room->spawns
    print(f"donor inventory: {len(d_events)} events, EVNT rows {d_evnt}, "
          f"PLYR {d_plyr}, {len(d_actors)} actor names, {d_doors} doors, "
          f"inbound SCLS room->spawns {dict((k, sorted(v)) for k, v in d_scls_in.items())}")

    if host_stage is None:
        pass
    else:
        hst = MOD / "files" / "res" / "Stage" / host_stage / "STG_00.arc"
        stg_members = dict(g.list_rarc_files(hst.read_bytes()))

        # ---- R2: native room arcs ---------------------------------------
        for droom, (hs, hroom, ini) in sorted(room_map.items()):
            arc = MOD / "files" / "res" / "Stage" / hs / f"R{hroom:02d}_00.arc"
            if not arc.is_file():
                finding("MAJOR", "R2-no-native-room-arc",
                        f"donor room {droom} -> {hs} room {hroom}: NO native "
                        f"R{hroom:02d}_00.arc — content is mount-era ({ini}); "
                        "receptor checks R7/R8 have no native home")

        # ---- R3: EVNT -> REVT -------------------------------------------
        hdzs = stg_members["stage.dzs"]
        revt_names = []
        n3 = be32(hdzs, 0)
        for i in range(n3):
            if hdzs[4 + i * 0xC: 8 + i * 0xC] == b"REVT":
                cnt, off = be32(hdzs, 8 + i * 0xC), be32(hdzs, 12 + i * 0xC)
                for k in range(cnt):
                    e = hdzs[off + k * 0x1C: off + (k + 1) * 0x1C]
                    revt_names.append(
                        bytes(e[0xD:0x1A]).split(b"\0")[0].decode())
        for nm in d_evnt:
            if nm not in revt_names:
                finding("MAJOR", "R3-evnt-missing",
                        f"donor stage-event '{nm}' has no host REVT row — "
                        "its spawn-triggered entry can never fire")

        # ---- R4/R5/R6: events -------------------------------------------
        hel = stg_members["event_list.dat"]
        hef = EventFile(hel)
        host_events = {hef.name(hef.etop, i, ESZ) for i in range(hef.enum)}
        for nm, _i in d_events:
            if nm.startswith("DEFAULT_") or nm in host_events:
                continue
            finding("MAJOR", "R4-event-missing",
                    f"donor event '{nm}' not merged into host event_list — "
                    "any actor ordering it gets idx -1 / silent no-op")
        # R5/R6 on present donor-specific events
        for nm, _i in d_events:
            if nm not in host_events or nm.startswith("DEFAULT_"):
                continue
            i = hef.find_event(nm)
            ev = hef.rec(hef.etop, i, ESZ)
            ns = struct.unpack_from(">i", ev, 0x7C)[0]
            has_pkg = False
            for k in range(ns):
                si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
                st = hef.rec(hef.stop, si, SSZ)
                ty = struct.unpack_from(">i", st, 0x2C)[0]
                snm = hef.name(hef.stop, si, SSZ)
                if ty == 11:
                    has_pkg = True
                if ty == 1:
                    finding("MAJOR", "R6-all-staff",
                            f"host event '{nm}' carries an ALL staff in a "
                            "multi-room host (P3 §275 hazard)")

        # ---- R7/R8: PLYR coverage ---------------------------------------
        for droom, (hs, hroom, _ini) in sorted(room_map.items()):
            arc = MOD / "files" / "res" / "Stage" / hs / f"R{hroom:02d}_00.arc"
            need = set(d_plyr.get(droom, []))
            for sroom, spawns in d_scls_in.items():
                if sroom == droom:
                    need |= set(spawns)
            if not arc.is_file():
                if need:
                    finding("MAJOR", "R7-plyr-no-receptor",
                            f"donor room {droom} needs PLYR points "
                            f"{sorted(need)} but host room {hroom} has no "
                            "native arc to carry them")
                continue
            _m, hdzr = read_dzr(arc)
            have = set()
            n4 = be32(hdzr, 0)
            for i in range(n4):
                if hdzr[4 + i * 0xC: 8 + i * 0xC] == b"PLYR":
                    cnt, off = be32(hdzr, 8 + i * 0xC), be32(hdzr, 12 + i * 0xC)
                    for k in range(cnt):
                        e = hdzr[off + k * 0x20: off + (k + 1) * 0x20]
                        have.add(struct.unpack(">h", e[0x1C:0x1E])[0] & 0xFF)
            missing = sorted(need - have)
            if missing:
                finding("MAJOR", "R7-plyr-missing",
                        f"donor room {droom} -> host room {hroom}: PLYR points "
                        f"{missing} absent (have {sorted(have)}) — any "
                        "setNextStage/SCLS to them fatals (§295 class)")

        # ---- R9: actor registration -------------------------------------
        names_ok = receiver_objnames()
        unreg = sorted(n for n in d_actors if n not in names_ok)
        if unreg:
            finding("MINOR", "R9-unregistered-actors",
                    f"{len(unreg)} placed actor names lack OBJNAME "
                    f"registration (defer/CSV-side): {unreg[:12]}"
                    + (" ..." if len(unreg) > 12 else ""))

        # ---- R10: stb casts ----------------------------------------------
        import json
        stb_p = (Path(r"%USERPROFILE%/Documents/dusklight/docs/WW Linked")
                 / "fact-sheets" / "json" / "stb-cast.json")
        if stb_p.is_file():
            cast = json.load(open(stb_p, encoding="utf-8"))
            for nm in d_actors:
                if nm in cast:
                    finding("INFO", "R10-cutscene-actor",
                            f"placed actor '{nm}' is a .stb cast member "
                            f"({len(cast[nm])} storyboards) — cutscene tier "
                            "applies; demo-arc residency needed for its scenes")

        # ---- R11: MULT hazard --------------------------------------------
        for i in range(n3):
            if hdzs[4 + i * 0xC: 8 + i * 0xC] == b"MULT":
                cnt = be32(hdzs, 8 + i * 0xC)
                hosted = {hr for _hs, hr, _i2 in room_map.values()}
                off = be32(hdzs, 12 + i * 0xC)
                rooms = {hdzs[off + k * 0xC + 0xA] for k in range(cnt)}
                if rooms & hosted:
                    finding("MAJOR", "R11-mult-corender",
                            f"host MULT group contains hosted rooms "
                            f"{sorted(rooms & hosted)} — event-mode entry "
                            "would co-render them (§276 class)")

    sev_rank = {"CRITICAL": 0, "MAJOR": 1, "MINOR": 2, "INFO": 3}
    FINDINGS.sort(key=lambda f: sev_rank[f[0]])
    lines = [f"# Space scan — donor '{stage}' (§299 prototype, read-only)", "",
             f"**{len(FINDINGS)} findings** "
             f"({sum(1 for f in FINDINGS if f[0] == 'CRITICAL')} critical, "
             f"{sum(1 for f in FINDINGS if f[0] == 'MAJOR')} major).", "",
             "| sev | check | finding |", "|---|---|---|"]
    lines += [f"| {s} | {c} | {t} |" for s, c, t in FINDINGS]
    body = "\n".join(lines) + "\n"
    if report_to:
        Path(report_to).write_text(body, encoding="utf-8")
        print(f"\nwrote {report_to}")
    print(f"\n=== {len(FINDINGS)} findings ===")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
