"""Count ALL models in Demo*.arc (not just leftover-tagged)."""
from __future__ import annotations

import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "companion_mod"))
from extract_rarc import parse_rarc  # noqa: E402

# local yaz0 from scan script
import importlib.util

spec = importlib.util.spec_from_file_location(
    "scan", Path(__file__).resolve().parent / "scan_demo_leftovers.py"
)
scan = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(scan)

obj = Path(r"D:/XXXXXXX/Ex TP/files/res/Object")
by_sha: dict[str, list[tuple[str, str, int]]] = defaultdict(list)
arcs = 0
files = 0
for arc in sorted(obj.glob("Demo*.arc")):
    arcs += 1
    raw = arc.read_bytes()
    rarc = scan.ensure_rarc(raw)
    for _fid, name, _off, size, payload in parse_rarc(rarc):
        files += 1
        low = name.lower()
        if not (low.endswith(".bmd") or low.endswith(".bdl")):
            continue
        by_sha[scan.sha16(payload)].append((arc.name, name, size))

print(f"Demo arcs: {arcs}")
print(f"All file entries: {files}")
print(f"Unique BMD/BDL: {len(by_sha)}")
# subject tokens
subjects = defaultdict(int)
for _sha, lst in by_sha.items():
    name = lst[0][1].lower()
    for key in (
        "link",
        "midna",
        "wl",
        "wolf",
        "wb",
        "ctz",
        "zant",
        "ganon",
        "epona",
        "horse",
        "kantera",
        "kusari",
        "henkei",
        "original",
        "high",
        "tmp",
    ):
        if key in name:
            subjects[key] += 1
print("subject hits among unique models:", dict(sorted(subjects.items())))
print("--- sample unique names (first 80) ---")
for i, (_sha, lst) in enumerate(
    sorted(by_sha.items(), key=lambda kv: (kv[1][0][0], kv[1][0][1]))
):
    if i >= 80:
        print("...")
        break
    arc, name, size = lst[0]
    print(f"{size:8}  x{len(lst):3}  {arc:14}  {name}")
