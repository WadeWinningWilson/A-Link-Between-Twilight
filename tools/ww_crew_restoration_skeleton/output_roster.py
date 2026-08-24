#!/usr/bin/env python3
# ============================================================================
# output_roster.py — R2 (§331): the output-target hash manifest.
#
# TTW discipline, output side (ttw-methods-review.md R2): every CONVERTED
# artifact in the mod folder gets its hash pinned at an ACCEPTANCE point;
# verify recomputes and names every drift. Catches the §113-stash class
# mechanically: any lane that "repairs" bytes in an accepted output changes
# its hash and the gate names the file.
#
#   accept [label]  — hash the ENTIRE mod folder (excluding backup belts) into
#                     output_roster.csv with schema/magic header (§331 TTW
#                     inheritance: magic+version mandatory in new formats).
#                     Overwrites the previous acceptance — run at USER-VERIFIED
#                     stable points only (the label records which).
#   verify          — recompute and report OK / DRIFT / MISSING / NEW.
#                     NEW is informational (lanes add files); DRIFT and
#                     MISSING are the failures. №31-C: absence of the roster
#                     itself reports UNKNOWN, never CLEAN.
#
# Pairs with donor_roster.py (R3, source side): source pinned + output pinned
# = the whole conversion is a verified function.
# ============================================================================
import csv
import hashlib
import sys
import time
from pathlib import Path

MOD = Path("%USERPROFILE%/AppData/Roaming/TwilitRealm/Dusklight/"
           "model_replacements/WW-Crew-Restoration")
ROSTER = MOD / "output_roster.csv"
MAGIC = "WWOUT"
VERSION = 1
SKIP_SUFFIXES = (".bak",)
SKIP_CONTAINS = (".pre-", "-bak", "__pycache__", "output_roster")
SKIP_DIRS = (".vexp",)  # live tool caches inside the mod tree — not outputs


def wanted(p):
    s = p.name.lower()
    if any(part in SKIP_DIRS for part in p.parts):
        return False
    return not (s.endswith(SKIP_SUFFIXES) or any(k in s for k in SKIP_CONTAINS))


def iter_files():
    for p in sorted(MOD.rglob("*")):
        if p.is_file() and wanted(p):
            yield p.relative_to(MOD).as_posix(), p


def sha256(p):
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def accept(label):
    rows = [(rel, p.stat().st_size, sha256(p)) for rel, p in iter_files()]
    with open(ROSTER, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow([MAGIC, VERSION, time.strftime("%Y-%m-%d %H:%M"), label,
                    len(rows)])
        w.writerow(["relpath", "size", "sha256"])
        w.writerows(rows)
    print(f"accepted: {len(rows)} outputs pinned ({label})")


def verify():
    if not ROSTER.is_file():
        print("UNKNOWN — no output roster exists (never CLEAN on absence)")
        return 2
    with open(ROSTER, encoding="utf-8") as f:
        r = csv.reader(f)
        hdr = next(r)
        assert hdr[0] == MAGIC and int(hdr[1]) == VERSION, "bad roster header"
        label = hdr[3]
        next(r)
        want = {rel: (int(size), digest) for rel, size, digest in r}
    ok = drift = miss = new = 0
    have = dict(iter_files())
    for rel, (size, digest) in want.items():
        p = have.get(rel)
        if p is None:
            miss += 1
            print(f"MISSING {rel}")
        elif p.stat().st_size != size or sha256(p) != digest:
            drift += 1
            print(f"DRIFT   {rel}")
        else:
            ok += 1
    for rel in have:
        if rel not in want:
            new += 1
    print(f"\nvs acceptance '{label}': {ok} OK / {drift} DRIFT / {miss} "
          f"MISSING / {new} NEW(informational)")
    return 1 if (drift or miss) else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    if len(sys.argv) >= 2 and sys.argv[1] == "accept":
        accept(sys.argv[2] if len(sys.argv) > 2 else "unlabeled")
    elif len(sys.argv) >= 2 and sys.argv[1] == "verify":
        sys.exit(verify())
    else:
        sys.exit("usage: output_roster.py accept [label] | verify")
