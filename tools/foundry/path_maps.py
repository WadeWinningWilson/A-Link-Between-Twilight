"""Foundry P12: donor path-map artifacts from the fact-sheet JSONs.

Groups RPPN waypoints under their RPAT headers (grouping arithmetic: RPAT
first_waypoint_offset / 0x10 = start index into the file's RPPN order; RPPN
DATA_SIZE = 0x10 per wwlib dzx.py) and emits per-stage polyline tables.

Usage: path_maps.py <fact-sheets-json-dir> <stage> <out.md>
       path_maps.py <fact-sheets-json-dir> --summary <out.md>
"""
import sys, os, json

def stage_paths(data):
    """Returns {arc: [ {points:[(x,y,z,action)], is_loop, next} ]}."""
    out = {}
    for arc, rows in data.items():
        rpat = [e for r in rows if r["chunk"] == "RPAT" for e in r["entries"]]
        rppn = [e for r in rows if r["chunk"] == "RPPN" for e in r["entries"]]
        paths = []
        for p in rpat:
            start = int(p.get("first_waypoint_offset", 0)) // 0x10
            n = int(p.get("num_points", 0))
            pts = rppn[start:start + n]
            paths.append({
                "is_loop": p.get("is_loop", 0),
                "next": p.get("next_path_index", 0xFFFF),
                "points": [(pt.get("x"), pt.get("y"), pt.get("z"),
                            pt.get("action_type")) for pt in pts],
                "declared": n, "got": len(pts),
            })
        if paths:
            out[arc] = paths
    return out

def main():
    json_dir = sys.argv[1]
    if sys.argv[2] == "--summary":
        lines = ["# Donor path summary (RPAT/RPPN, all stages)", "",
                 "| stage | arcs w/ paths | paths | waypoints |", "|---|---|---|---|"]
        tp = tw = 0
        for fn in sorted(os.listdir(json_dir)):
            if not fn.endswith(".json"):
                continue
            data = json.load(open(os.path.join(json_dir, fn), encoding="utf-8"))
            sp = stage_paths(data)
            np_ = sum(len(v) for v in sp.values())
            nw = sum(len(p["points"]) for v in sp.values() for p in v)
            if np_:
                lines.append(f"| `{fn[:-5]}` | {len(sp)} | {np_} | {nw} |")
                tp += np_; tw += nw
        lines.append(f"\nTotals: {tp} paths, {tw} waypoints.")
        open(sys.argv[3], "w", encoding="utf-8").write("\n".join(lines) + "\n")
        print(f"summary: {tp} paths, {tw} waypoints")
        return
    stage, out_md = sys.argv[2], sys.argv[3]
    data = json.load(open(os.path.join(json_dir, stage + ".json"), encoding="utf-8"))
    sp = stage_paths(data)
    lines = [f"# Donor paths — stage `{stage}`", ""]
    for arc, paths in sp.items():
        lines.append(f"## `{arc}` — {len(paths)} paths")
        for i, p in enumerate(paths):
            flag = "" if p["declared"] == p["got"] else \
                f" **[INFERENCE-NEEDED: {p['declared']} declared, {p['got']} resolved — offset grouping suspect]**"
            nxt = "" if p["next"] in (0xFFFF, 65535) else f" next={p['next']}"
            lines.append(f"\n### path {i} — {p['got']} pts, loop={p['is_loop']}{nxt}{flag}")
            lines.append("| # | x | y | z | action |")
            lines.append("|---|---|---|---|---|")
            for j, (x, y, z, a) in enumerate(p["points"]):
                lines.append(f"| {j} | {x} | {y} | {z} | {a} |")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {sum(len(v) for v in sp.values())} paths")

if __name__ == "__main__":
    main()
