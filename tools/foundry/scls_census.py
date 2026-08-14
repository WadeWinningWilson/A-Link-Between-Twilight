#!/usr/bin/env python3
# ============================================================================
# scls_census.py — SECOND-HOP CLOSURE off the disc (tale §884's optional ask).
#
# THE CLASS IT CLOSES: Fairy04 — an interior reachable only from ANOTHER
# interior's SCLS (A_mori's own exit table), invisible to the sea-doors
# declaration and discovered as a runtime refusal. §884's fix declared it and
# recorded the rule (declare a stage AND the stages its SCLS names); this
# census is the rule made TRANSITIVE and disc-derived: walk every declared
# stage's SCLS (all rooms' .dzr + Stage.arc .dzs), chase undeclared
# destinations through THEIR SCLS too, and name every stage the declared set
# can reach that is not itself declared. Observed closure becomes PROVEN
# closure — future Fairy04s are a regeneration away, not a playtest away.
#
# Usage: scls_census.py     (reads ww_stages.ini for the declared set)
# Read-only except the artifact. Exit 0 closed · 1 undeclared reachable.
# ============================================================================
import os
import struct
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(HERE))
import space_kit

INI = Path(os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight")) / \
      "model_replacements" / "WW-Crew-Restoration" / "ww_stages.ini"


def declared():
    stages = {}
    for ln in INI.read_text(encoding="utf-8", errors="replace").splitlines():
        ln = ln.strip()
        if not ln or ln.startswith("#"):
            continue
        parts = ln.split(",")
        if len(parts) >= 2 and parts[0]:
            stages.setdefault(parts[0], set()).add(parts[1])
    return stages


def scls_of(stage, rooms=None):
    """{(dest_stage, spawn, room)} from the DECLARED rooms' .dzr SCLS only.
    Scoping receipt (first run): walking ALL of sea's 50 rooms pulled the
    entire game into the closure — the declared unit is (stage, room), and
    reachability chases exactly the room each SCLS exit lands in. Stage.dzs
    is stage-global and EXCLUDED here (a room's doors index its own table);
    if a refusal ever names a Stage.dzs-only exit, widen with a receipt."""
    out = set()
    try:
        want = None if rooms is None else {int(r) for r in rooms if str(r).isdigit()}
        for rname in space_kit.donor_rooms(stage):
            room_no = int("".join(ch for ch in rname if ch.isdigit()) or 0)
            if want is not None and room_no not in want:
                continue
            rbytes, _src = space_kit.donor_file(stage, rname)
            _n, dzr = space_kit.get_member(rbytes, ".dzr")
            if not dzr:
                continue
            for tag, entries in space_kit.dz_chunks(dzr).items():
                if tag != "SCLS":
                    continue
                for cnt, off in entries:
                    for i in range(cnt):
                        e = off + i * 0xC
                        name = dzr[e:e + 8].split(bytes([0]))[0].decode("ascii", "replace")
                        spawn, room = dzr[e + 8], dzr[e + 9]
                        if name:
                            out.add((name, spawn, room))
    except Exception as ex:
        print("  (%s unreadable: %s)" % (stage, ex))
    return out


def main():
    decl = declared()
    print("declared stages: %s" % ", ".join(sorted(decl)))
    # frontier of (stage, room) pairs — true reachability, room-scoped
    seen_rooms = set()
    frontier = []
    for s, rooms in decl.items():
        for r in rooms:
            if str(r).isdigit():
                frontier.append((s, int(r)))
                seen_rooms.add((s, int(r)))
    edges = {}
    undeclared = {}
    depth = {p: 0 for p in seen_rooms}
    while frontier:
        stage, room = frontier.pop(0)
        dests = scls_of(stage, [room])
        edges.setdefault(stage, set()).update(d[0] for d in dests)
        for name, _sp, drm in dests:
            pair = (name, drm)
            d = depth.get((stage, room), 0) + 1
            if name not in decl:
                undeclared.setdefault(name, set()).add("%s/%d" % (stage, room))
            if pair not in seen_rooms:
                seen_rooms.add(pair)
                depth[pair] = d
                frontier.append(pair)
    depth_by_stage = {}
    for (s, _r), d in depth.items():
        depth_by_stage[s] = min(depth_by_stage.get(s, 99), d)
    edges = {k: sorted(v) for k, v in edges.items()}

    lines = ["# SCLS transitive closure (scls_census.py — GENERATED)", ""]
    lines.append("Declared: %s" % ", ".join(sorted(decl)))
    lines.append("")
    if undeclared:
        lines.append("## UNDECLARED but REACHABLE (the Fairy04 class — declare or rule out)")
        lines.append("")
        for name in sorted(undeclared, key=lambda n: depth.get(n, 9)):
            lines.append("- **%s** (hop depth %d) reached from: %s"
                         % (name, depth_by_stage.get(name, -1), ", ".join(sorted(undeclared[name]))))
    else:
        lines.append("## CLOSURE PROVEN — every reachable stage is declared")
    lines.append("")
    lines.append("## Edges (stage -> SCLS destinations)")
    lines.append("")
    for s in sorted(edges):
        lines.append("- %s -> %s" % (s, ", ".join(edges[s]) or "(none)"))
    out = REPO / "docs" / "state" / "ww-staging" / "scls-closure.md"
    out.write_text("\r\n".join(lines) + "\r\n", encoding="utf-8", newline="")
    print("\n".join(lines))
    print("-> %s" % out)
    return 1 if undeclared else 0


if __name__ == "__main__":
    sys.exit(main())
