#!/usr/bin/env python3
import re
import zipfile
from collections import Counter, defaultdict
from pathlib import Path

RTZ = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm-mods\MMN64HD\mm_MMN64HD.rtz"
)
PAT = re.compile(r"#([0-9A-F]{8})#(\d+)#(\d+)(?:#([0-9A-F]{8}))?_(.+)\.dds$")
PRIOR = [
    "0461217F",
    "033C6482",
    "021C1551",
    "01967F09",
    "03B80924",
    "00BB7F5C",
    "30323426",
    "00BF8A97",
]

z = zipfile.ZipFile(RTZ)
chars = [n for n in z.namelist() if n.startswith("Characters/") and n.endswith(".dds")]
suffix = Counter()
triple = Counter()
by_hash: dict[str, list] = defaultdict(list)
for n in chars:
    base = n.split("/")[-1]
    m = PAT.search(base)
    if not m:
        continue
    h, a, b, sub, suf = m.groups()
    suffix[suf] += 1
    triple[(a, b, suf)] += 1
    by_hash[h].append((a, b, sub, suf, base))

print("Character DDS:", len(chars))
print("unique rice hashes:", len(by_hash))
print("suffix counts:", dict(suffix.most_common()))
print("a#b#suffix top:", triple.most_common(10))
print("\n--- prior hash variants ---")
for h in PRIOR:
    print(h, by_hash.get(h))

# list non-json files in rtz root
meta = [n for n in z.namelist() if n.endswith((".json", ".txt", ".xml", ".ini"))]
print("\nrtz metadata files:", len(meta))
for n in sorted(meta)[:30]:
    print(" ", n)
