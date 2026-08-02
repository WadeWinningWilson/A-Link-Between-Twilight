"""Foundry: multi-type actor registry (bus §214, History's Ikada correction
generalized). One actor NAME can host many entities — subtype selected from
params (mType etc.). A name-keyed census is therefore blind; this tool emits the
game-wide registry of names whose placements carry MULTIPLE distinct params
values, with the raw params quoted verbatim (no interpretation — decoding each
actor's params fields is per-actor decomp work, see param_map.py).

Usage: multitype_registry.py <fact-sheets-json-dir> <out.md>
"""
import sys, os, json
from collections import defaultdict

def main():
    json_dir, out_md = sys.argv[1], sys.argv[2]
    by_name = defaultdict(lambda: defaultdict(int))   # name -> params -> count
    stages_of = defaultdict(set)
    for fn in sorted(os.listdir(json_dir)):
        if not fn.endswith(".json"):
            continue
        data = json.load(open(os.path.join(json_dir, fn), encoding="utf-8"))
        for arc, rows in data.items():
            for r in rows:
                if r["chunk"] not in ("ACTR", "ACT0", "ACT1", "ACT2", "ACT3",
                                      "SCOB", "TGOB", "TGSC"):
                    continue
                for e in r["entries"]:
                    name, prm = e.get("name"), e.get("params")
                    if name and prm is not None:
                        by_name[name][prm] += 1
                        stages_of[name].add(fn[:-5])
    multi = {n: p for n, p in by_name.items() if len(p) > 1}
    lines = ["# Multi-type actor registry (distinct params per name, game-wide)",
             "",
             f"{len(multi)} of {len(by_name)} placed names carry MULTIPLE distinct "
             "params values → any name-keyed census row for these is a blend of "
             "subtypes until the actor's params decode (param_map.py) splits it. "
             "Params are quoted verbatim; field meaning per actor is decomp work.",
             "",
             "| name | distinct params | placements | stages | params values (top 8) |",
             "|---|---|---|---|---|"]
    for name in sorted(multi, key=lambda n: -len(multi[n])):
        p = multi[name]
        top = ", ".join(f"`{k}`×{v}" for k, v in
                        sorted(p.items(), key=lambda kv: -kv[1])[:8])
        lines.append(f"| `{name}` | {len(p)} | {sum(p.values())} | "
                     f"{len(stages_of[name])} | {top} |")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {len(multi)} multi-type names of {len(by_name)}")

if __name__ == "__main__":
    main()
