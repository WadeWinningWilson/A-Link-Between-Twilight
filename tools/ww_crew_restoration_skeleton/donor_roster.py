#!/usr/bin/env python3
# ============================================================================
# donor_roster.py — R3 (§331): the sanctioned-donor-dump roster.
#
# USER RULING (§332): the sanctioned dump is NTSC-U — self-identified by the
# dump's own sys/boot.bin game code **GZLE01** ("THE LEGEND OF ZELDA The Wind
# Waker", NTSC-U). Every golden, baseline, census and golden trace in the
# campaign silently assumed this dump; this roster makes the assumption a
# checkable fact (TTW known-source discipline, ttw-methods-review.md R3).
#
#   build   — hash the sanctioned dump (sys/* + files/res/** + files/
#             Audiores/**) into donor_roster_GZLE01.csv (relpath,size,sha256;
#             header carries game code + counts). Checked into the repo:
#             hashes are structural metadata, content-wall clean.
#   verify <dir> — hash a candidate donor tree/drop and report per file:
#             OK / MISMATCH / MISSING / OFF-ROSTER (never CLEAN on absence —
#             №31-C: a file the roster does not know is UNKNOWN, not fine).
#
# Read-only against the dump; writes only the roster CSV.
# ============================================================================
import csv
import hashlib
import struct
import sys
from pathlib import Path

DUMP = Path("D:/XXXXXXX/Ex WW")
ROSTER = Path(__file__).parent / "donor_roster_GZLE01.csv"
SCOPE = ["sys", "files/res", "files/Audiores"]


def game_code(root):
    b = (root / "sys/boot.bin").read_bytes()[:6]
    return b.decode("ascii", "replace")


def iter_files(root):
    for scope in SCOPE:
        base = root / scope
        if not base.is_dir():
            continue
        for p in sorted(base.rglob("*")):
            if p.is_file():
                yield p.relative_to(root).as_posix(), p


def sha256(p):
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def build():
    code = game_code(DUMP)
    assert code == "GZLE01", f"dump is {code}, not the sanctioned GZLE01"
    rows = []
    for rel, p in iter_files(DUMP):
        rows.append((rel, p.stat().st_size, sha256(p)))
        if len(rows) % 200 == 0:
            print(f"  …{len(rows)} files hashed")
    with open(ROSTER, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow([f"# donor roster — sanctioned dump {code} (user ruling "
                    f"§332); {len(rows)} files; schema_version=1"])
        w.writerow(["relpath", "size", "sha256"])
        w.writerows(rows)
    print(f"roster: {len(rows)} files -> {ROSTER.name} (game code {code})")


def verify(target):
    target = Path(target)
    want = {}
    with open(ROSTER, encoding="utf-8") as f:
        r = csv.reader(f)
        next(r)  # header comment
        next(r)  # column row
        for rel, size, digest in r:
            want[rel] = (int(size), digest)
    ok = miss = bad = extra = 0
    for rel, (size, digest) in want.items():
        p = target / rel
        if not p.is_file():
            miss += 1
            print(f"MISSING   {rel}")
            continue
        if p.stat().st_size != size or sha256(p) != digest:
            bad += 1
            print(f"MISMATCH  {rel}")
            continue
        ok += 1
    known = set(want)
    for rel, p in iter_files(target):
        if rel not in known:
            extra += 1
            print(f"OFF-ROSTER {rel}  (UNKNOWN — not sanctioned-dump content)")
    print(f"\n{ok} OK / {bad} MISMATCH / {miss} MISSING / {extra} OFF-ROSTER "
          f"vs roster {ROSTER.name}")
    return 1 if (bad or miss) else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    if len(sys.argv) >= 2 and sys.argv[1] == "build":
        build()
    elif len(sys.argv) >= 3 and sys.argv[1] == "verify":
        sys.exit(verify(sys.argv[2]))
    else:
        sys.exit("usage: donor_roster.py build | verify <donor-tree>")
