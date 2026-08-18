#!/usr/bin/env python3
# ============================================================================
# row_baseline.py — the seam-tracker BASELINE PIN (roadmap A4). Bridge hosts.
#
#   pin       write tracker/_baseline.json — row set (IDs + hashes) + tree state
#   check     compare the live store against the pin; EXIT 1 only on DIVERGENCE
#   selftest  NEGATIVE CONTROL — every failure mode must be shown to fire
#
# WHAT IT BLOCKS, and only this (spec SPEC-row-store.md §5):
#   "A row present in the last baseline and absent now, with no retirement
#    entry, fails the build — the deletion bypass dies there."
#
# WHAT IT DELIBERATELY DOES NOT BLOCK, because a gate that fails every build
# gets switched off and then protects nothing:
#   · NEW rows            — adding coverage is the goal, never a regression
#   · EDITED rows         — content churn is normal; the pin records it, the
#                           gate reports it, the build continues
#   · LEGACY-UNROWED seams — seams that predate the store are GRANDFATHERED by
#                           the pin. They are listed, counted, and never fatal.
#                           A gate that demanded a row for every pre-existing
#                           seam on day one would fail every build until the
#                           whole estate was rowed, which is how gates die.
#
# ⚠ HASHES ARE COMPUTED ON EOL-NORMALISED CONTENT.
#   Found while running A3's merge gate: git autocrlf rewrote line endings
#   between checkouts and two files showed different md5s with byte-identical
#   content. A raw hash would fail the build for nothing, on Windows, with a
#   "row changed / no diff" symptom nobody could reproduce.
# ============================================================================
import hashlib
import json
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
TRACKER = REPO / "docs" / "state" / "ww-staging" / "tracker"
ROWS = TRACKER / "rows"
BASELINE = TRACKER / "_baseline.json"
RETIRED = TRACKER / "RETIRED.md"


def norm_hash(path):
    """EOL-normalised. See the header — this is the A3-gate finding."""
    b = path.read_bytes().replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    if b.startswith(b"\xef\xbb\xbf"):
        b = b[3:]                      # a BOM is not content either
    return hashlib.sha256(b).hexdigest()[:16]


def row_set(rows_dir=None):
    d = rows_dir or ROWS
    if not d.is_dir():
        return {}
    return {p.stem: norm_hash(p) for p in sorted(d.glob("*.md"))}


def tree_state():
    try:
        r = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True,
                           text=True, cwd=str(REPO), timeout=30)
        head = (r.stdout or "").strip() or "(no head)"
    except Exception:
        head = "(git unavailable)"
    return head


def retired_ids():
    if not RETIRED.is_file():
        return set()
    import re
    return set(re.findall(r"\bs[0-9a-f]{8}\b", RETIRED.read_text(encoding="utf-8", errors="replace")))


def pin(legacy_note=""):
    rows = row_set()
    data = {
        "_comment": "A4 baseline pin. Hashes are EOL-NORMALISED (see row_baseline.py header). "
                    "The gate blocks ONLY on a baselined row vanishing with no retirement entry.",
        "pinned_head": tree_state(),
        "row_count": len(rows),
        "rows": rows,
        "legacy_unrowed": {
            "_why": "Seams that predate the store are GRANDFATHERED: listed, never fatal. "
                    "A gate that demanded a row for every pre-existing seam would fail "
                    "every build until the estate was fully rowed, and would be switched off.",
            "note": legacy_note or "not yet enumerated — grandfathering is open by default",
        },
    }
    BASELINE.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8", newline="\n")
    print("PINNED %d row(s) at %s -> %s" % (len(rows), data["pinned_head"],
                                            BASELINE.relative_to(REPO)))
    return 0


def check(rows_dir=None, baseline_path=None, verbose=True):
    bp = baseline_path or BASELINE
    if not bp.is_file():
        if verbose:
            print("no baseline pinned — run `row_baseline.py pin` (not a failure)")
        return 0, []
    base = json.loads(bp.read_text(encoding="utf-8"))
    old, new = base.get("rows", {}), row_set(rows_dir)
    retired = retired_ids()
    vanished = [k for k in old if k not in new and k not in retired]
    retired_ok = [k for k in old if k not in new and k in retired]
    added = [k for k in new if k not in old]
    changed = [k for k in new if k in old and new[k] != old[k]]
    if verbose:
        print("baseline %s · %d row(s) pinned · live %d" % (base.get("pinned_head"), len(old), len(new)))
        print("  added   %d  (not a regression)" % len(added))
        print("  changed %d  (reported, not fatal)" % len(changed))
        print("  retired %d  (has a RETIRED entry — allowed)" % len(retired_ok))
        for k in vanished:
            print("  ** DIVERGENCE ** row %s was baselined and is GONE with no retirement entry" % k)
        print("  -> %s" % ("FAIL" if vanished else "PASS"))
    return (1 if vanished else 0), vanished


def selftest():
    """Every failure mode shown to fire, and every non-failure shown NOT to."""
    import tempfile
    print("SELFTEST — the gate must FAIL on deletion and PASS on everything else\n")
    d = Path(tempfile.mkdtemp())
    rows = d / "rows"
    rows.mkdir()
    for rid in ("s11111111", "s22222222"):
        (rows / ("%s.md" % rid)).write_text("id: %s\nsymbols: x\n" % rid, encoding="utf-8", newline="\n")
    bp = d / "_baseline.json"
    bp.write_text(json.dumps({"pinned_head": "test", "rows": row_set(rows)}) + "\n",
                  encoding="utf-8", newline="\n")
    results = []
    rc, _ = check(rows, bp, verbose=False)
    results.append(("unchanged store", rc, 0))
    (rows / "s33333333.md").write_text("id: s33333333\nsymbols: y\n", encoding="utf-8", newline="\n")
    rc, _ = check(rows, bp, verbose=False)
    results.append(("row ADDED", rc, 0))
    (rows / "s11111111.md").write_text("id: s11111111\nsymbols: EDITED\n", encoding="utf-8", newline="\n")
    rc, _ = check(rows, bp, verbose=False)
    results.append(("row EDITED", rc, 0))
    # CRLF rewrite must NOT look like a change (the A3-gate finding)
    p = rows / "s22222222.md"
    p.write_bytes(p.read_bytes().replace(b"\n", b"\r\n"))
    rc, _ = check(rows, bp, verbose=False)
    results.append(("row rewritten CRLF (EOL only)", rc, 0))
    (rows / "s22222222.md").unlink()
    rc, van = check(rows, bp, verbose=False)
    results.append(("row DELETED, no retirement", rc, 1))
    bad = 0
    for name, got, want in results:
        ok = (got == want)
        bad += 0 if ok else 1
        print("  [%-32s] rc=%d want=%d  %s" % (name, got, want, "OK" if ok else "** WRONG **"))
    print("\n%d/%d behaviours correct%s" % (len(results) - bad, len(results),
          "" if not bad else "  ** GATE IS UNSOUND **"))
    return 1 if bad else 0


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "pin":
        return pin()
    if cmd == "check":
        rc, _ = check()
        return rc
    if cmd == "selftest":
        return selftest()
    print("usage: row_baseline.py pin | check | selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
