"""Foundry F6 pilot: regenerate Bridge-known JPC facts offline with gclib.
Claims under test (bus §192/§196):
  A. common.jpc contains 0x0031 (windline), 0x03DA (grass cut), 0x03DB (grass run)
  B. Pscene011.jpc contains 0x8315 and 0x833D
  C. Pscene011.jpc does NOT contain 0x89D6/0x89D7 (those are TP ids)
"""
import io, sys
# Py3.14 compat: gclib uses the private dataclasses._recursive_repr removed in 3.13+.
import dataclasses, reprlib
if not hasattr(dataclasses, "_recursive_repr"):
    dataclasses._recursive_repr = reprlib.recursive_repr()
from gclib.jpc import JPC

PART = r"D:\XXXXXXX\Ex WW\files\res\Particle"

def load(name):
    with open(f"{PART}\\{name}", "rb") as f:
        data = f.read()
    try:
        return JPC(io.BytesIO(data))
    except TypeError:
        return JPC(data)

def ids_of(jpc):
    for attr in ("particles",):
        parts = getattr(jpc, attr, None)
        if parts is not None:
            out = []
            for p in parts:
                pid = getattr(p, "particle_id", None)
                if pid is None:
                    print("particle attrs:", [a for a in dir(p) if not a.startswith("_")][:40])
                    sys.exit(1)
                out.append(pid)
            return out
    print("jpc attrs:", [a for a in dir(jpc) if not a.startswith("_")][:40])
    sys.exit(1)

common = load("common.jpc")
p011 = load("Pscene011.jpc")
cids, pids = set(ids_of(common)), set(ids_of(p011))
print(f"common.jpc: {len(cids)} particles | Pscene011.jpc: {len(pids)} particles")

checks = [
    ("A: 0x0031 in common",  0x0031 in cids),
    ("A: 0x03DA in common",  0x03DA in cids),
    ("A: 0x03DB in common",  0x03DB in cids),
    ("A+: 0x03DC in common", 0x03DC in cids),
    ("B: 0x8315 in Pscene011", 0x8315 in pids),
    ("B: 0x833D in Pscene011", 0x833D in pids),
    ("C: 0x89D6 ABSENT from Pscene011", 0x89D6 not in pids),
    ("C: 0x89D7 ABSENT from Pscene011", 0x89D7 not in pids),
]
fail = 0
for label, ok in checks:
    print(("PASS  " if ok else "FAIL  ") + label)
    fail += (not ok)
print("Pscene011 full id list:", sorted(hex(i) for i in pids))
sys.exit(1 if fail else 0)
