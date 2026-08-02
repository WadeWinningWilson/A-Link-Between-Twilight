"""Foundry P11: donor transition graph + receiver [Doors] verdict.

Builds the donor exit graph from the fact-sheet JSONs' SCLS entries (per stage/room/
layer: exit index -> dest stage, spawn point, room, fade), then optionally replays a
receiver dusklight log's [Doors] transport lines against it.

Covenant: donor-verbatim; port transports with no donor SCLS edge are named
PORT-ONLY (socket/liberty/bug — content lanes judge), never silently passed.

Usage:
  transition_graph.py graph <fact-sheets-json-dir> <out.md>          # the artifact
  transition_graph.py check <fact-sheets-json-dir> <dusklight.log> <out.md>
"""
import sys, os, json, re
from collections import defaultdict

DOORS_RE = re.compile(
    r"\[Doors\] enter .*?transport=stage host='([^']+)' room=(\d+) point=(\d+)")

def load_graph(json_dir):
    graph = defaultdict(list)  # (stage, room_file, layer) -> [exit dicts]
    for fn in sorted(os.listdir(json_dir)):
        if not fn.endswith(".json"):
            continue
        stage = fn[:-5]
        data = json.load(open(os.path.join(json_dir, fn), encoding="utf-8"))
        for arc, rows in data.items():
            for r in rows:
                if r["chunk"] != "SCLS":
                    continue
                exits = [e for e in r["entries"]]
                if exits:
                    graph[(stage, arc, r["layer"])] = exits
    return graph

def emit_graph(graph, out_md):
    lines = ["# Donor transition graph (SCLS, all stages)", ""]
    n_edges = 0
    for (stage, arc, layer), exits in sorted(graph.items()):
        lines.append(f"## `{stage}` / `{arc}` (layer {layer})")
        lines.append("| exit# | dest stage | spawn | room | fade |")
        lines.append("|---|---|---|---|---|")
        for i, e in enumerate(exits):
            lines.append(f"| {i} | `{e.get('dest_stage_name')}` | "
                         f"{e.get('spawn_id')} | {e.get('room_index')} | "
                         f"{e.get('fade_type')} |")
            n_edges += 1
        lines.append("")
    lines.insert(1, f"{n_edges} exit edges across {len(graph)} (stage,room,layer) tables.")
    with open(out_md, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {n_edges} edges, {len(graph)} tables")

def check(graph, log_path, out_md):
    # Destination view: donor edges into (stage-name-prefix, room, spawn)
    donor_dests = set()
    for exits in graph.values():
        for e in exits:
            donor_dests.add((str(e.get("dest_stage_name")).strip("\x00"),
                             int(e.get("room_index", -1)), int(e.get("spawn_id", -1))))
    lines = ["# P11 receiver transition check", "",
             f"log: `{log_path}`", "",
             "| host stage | room | point | donor edge exists? |", "|---|---|---|---|"]
    seen, matched, portonly = 0, 0, 0
    with open(log_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = DOORS_RE.search(line)
            if not m:
                continue
            seen += 1
            host, room, point = m.group(1), int(m.group(2)), int(m.group(3))
            ok = (host, room, point) in donor_dests
            matched += ok
            portonly += (not ok)
            lines.append(f"| `{host}` | {room} | {point} | "
                         f"{'MATCH' if ok else 'PORT-ONLY (no donor SCLS edge — socket/liberty/bug?)'} |")
    if seen == 0:
        lines.append("\n**VERDICT: UNKNOWN — no [Doors] transport lines in this log.**")
    else:
        lines.append(f"\n**{seen} transports: {matched} donor-edged, {portonly} "
                     f"PORT-ONLY.** Note: port stage names are receiver-agnostic "
                     f"sockets (covenant), so name-mismatches with donor stage codes "
                     f"are expected — [INFERENCE-NEEDED]: a socket→donor-stage mapping "
                     f"table (History/Engine own it) upgrades this from name-match to "
                     f"true edge verification.")
    with open(out_md, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {seen} transports, {matched} matched, {portonly} port-only")

def main():
    mode, json_dir = sys.argv[1], sys.argv[2]
    graph = load_graph(json_dir)
    if mode == "graph":
        emit_graph(graph, sys.argv[3])
    else:
        check(graph, sys.argv[3], sys.argv[4])

if __name__ == "__main__":
    main()
