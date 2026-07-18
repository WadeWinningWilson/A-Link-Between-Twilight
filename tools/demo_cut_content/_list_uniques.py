import csv
from collections import defaultdict
from pathlib import Path

rows = list(csv.DictReader(Path("tools/demo_cut_content/out/demo_leftovers.csv").open(encoding="utf-8")))
models = [r for r in rows if "model" in r["tags"].split(",")]
by_sha: dict[str, list] = defaultdict(list)
for r in models:
    by_sha[r["sha16"]].append(r)

print("tagged files", len(rows))
print("model rows", len(models), "unique model sha", len(by_sha))
print("=== ALL UNIQUE MODELS ===")
for _sha, lst in sorted(by_sha.items(), key=lambda kv: (kv[1][0]["arc"], kv[1][0]["name"])):
    r = lst[0]
    print(f"{r['size']:8}  x{len(lst):3}  {r['arc']:14}  {r['name']}  [{r['tags']}]")
