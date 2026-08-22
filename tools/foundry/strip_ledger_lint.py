#!/usr/bin/env python3
# ============================================================================
# strip_ledger_lint.py - DOES THE STRIP LEDGER MATCH THE STRIP COMMITS?
#
# The fork-strip protocol (user order 2026-08-21, FORK-STRIP-LEDGER.md) says:
# ledger row FIRST, strip second, one DEDICATED commit per item with subject
# prefix `strip:`. This lint enforces both directions:
#
#   FORWARD  - every row with status STRIPPED names a commit that EXISTS, has
#              the `strip:` subject prefix, and touches ONLY paths the row
#              declares. A STRIPPED row with a bad SHA is a deletion wearing
#              a migration's clothes.
#   REVERSE  - every `strip:`-prefixed commit in history is named by some
#              STRIPPED row. A strip commit with no ledger row is the exact
#              undocumented deletion the user's order forbids.
#
# TODAY the ledger has ZERO STRIPPED rows, so the forward pass is vacuously
# green - which is precisely why --control exists and must be run first: a
# lint that has never flagged anything proves nothing by passing. Per the
# estate rule, silence is only a result once noise is demonstrated.
#
# Usage (from repo root):
#   strip_ledger_lint.py            - lint ledger vs git history
#   strip_ledger_lint.py --control  - prove both directions can flag
# Exit 0 clean/control-pass - 1 violations or control FAILED - 2 bad input.
# ============================================================================
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
LEDGER = REPO / "mods-src" / "ww_donor_disc" / "FORK-STRIP-LEDGER.md"
SHA_RX = re.compile(r"\b[0-9a-f]{7,40}\b")
PATH_RX = re.compile(r"`([^`]+\.(?:cpp|h|inc|py|cmake|txt))`")


def parse_rows(text):
    """Ledger table rows -> [(item, declared_paths, sha_or_None, status)]."""
    rows = []
    for line in text.splitlines():
        if not line.startswith("|") or line.count("|") < 6:
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        if len(cells) < 6 or cells[0] in ("item", "---") or set(cells[0]) <= {"-"}:
            continue
        item, fork_paths, _, _, strip_commit, status = cells[:6]
        m = SHA_RX.search(strip_commit)
        rows.append((item, PATH_RX.findall(fork_paths), m.group(0) if m else None,
                     status.upper()))
    return rows


def git(*args):
    r = subprocess.run(["git", "-C", str(REPO)] + list(args),
                       capture_output=True, text=True)
    return r.returncode, r.stdout


def lint(rows, strip_commits, commit_info):
    """Pure - so the control can exercise it. commit_info: sha -> (subject,
    [changed paths]) or None for a SHA git does not know. Returns violations."""
    v = []
    stripped_shas = set()
    for item, paths, sha, status in rows:
        if status != "STRIPPED":
            continue
        if sha is None:
            v.append("FORWARD: row '%s' is STRIPPED with NO commit SHA" % item[:50])
            continue
        stripped_shas.add(sha)
        info = commit_info.get(sha)
        if info is None:
            v.append("FORWARD: row '%s' names %s - commit DOES NOT EXIST" % (item[:50], sha))
            continue
        subject, changed = info
        if not subject.startswith("strip:"):
            v.append("FORWARD: %s subject lacks 'strip:' prefix ('%s')" % (sha, subject[:40]))
        declared = set(p.split("/")[-1] for p in paths)
        for ch in changed:
            if ch.split("/")[-1] not in declared and not ch.endswith("FORK-STRIP-LEDGER.md"):
                v.append("FORWARD: %s touches '%s' which row '%s' never declared"
                         % (sha, ch, item[:40]))
    for sha in strip_commits:
        if not any(sha.startswith(s) or s.startswith(sha) for s in stripped_shas):
            v.append("REVERSE: strip commit %s has NO ledger row" % sha)
    return v


def control():
    rows = [
        ("good item", ["src/d/d_kankyo_ww_sky.cpp"], "aaaaaaa", "STRIPPED"),
        ("bad-sha item", ["src/d/x.cpp"], "bbbbbbb", "STRIPPED"),
        ("mixed-commit item", ["src/d/y.cpp"], "ccccccc", "STRIPPED"),
        ("mapped item", ["src/d/z.cpp"], None, "MAPPED"),
    ]
    commit_info = {
        "aaaaaaa": ("strip: WW sky TU", ["src/d/d_kankyo_ww_sky.cpp"]),
        # bbbbbbb absent - simulates a SHA git does not know
        "ccccccc": ("strip: y", ["src/d/y.cpp", "src/d/UNRELATED.cpp"]),
    }
    strip_commits = ["aaaaaaa", "ccccccc", "ddddddd"]  # ddddddd has no row
    v = lint(rows, strip_commits, commit_info)
    expect = ("DOES NOT EXIST", "never declared", "NO ledger row")
    hit = [any(e in x for x in v) for e in expect]
    print("NEGATIVE CONTROL - planted: dead SHA, out-of-scope path, rowless commit")
    for x in v:
        print("  flagged: " + x)
    if all(hit) and len(v) == 3:
        print("  CONTROL PASSED - all three planted violations caught, nothing else.")
        return 0
    print("  *** CONTROL FAILED (%d flags, expected 3 specific ones). ***" % len(v))
    return 1


def main():
    if "--control" in sys.argv:
        return control()
    if not LEDGER.exists():
        print("ledger missing: %s" % LEDGER)
        return 2
    rows = parse_rows(LEDGER.read_text(encoding="utf-8", errors="replace"))
    rc, out = git("log", "--format=%h%x09%s", "--grep=^strip:")
    strip_commits = [l.split("\t")[0] for l in out.splitlines() if l.strip()]
    commit_info = {}
    for _, _, sha, status in rows:
        if status == "STRIPPED" and sha:
            rc, s = git("show", "--format=%s", "--name-only", sha)
            if rc != 0:
                commit_info[sha] = None
                continue
            lines = [l for l in s.splitlines() if l.strip()]
            commit_info[sha] = (lines[0] if lines else "", lines[1:])
    # drop unknown-SHA placeholders back to None sentinel handled in lint()
    commit_info = {k: v for k, v in commit_info.items() if v is not None}
    v = lint(rows, strip_commits, commit_info)
    n_stripped = sum(1 for r in rows if r[3] == "STRIPPED")
    print("STRIP LEDGER LINT - %d rows (%d STRIPPED) vs %d strip: commits"
          % (len(rows), n_stripped, len(strip_commits)))
    if n_stripped == 0 and not strip_commits:
        print("  vacuously clean: no strips have happened yet. This pass means")
        print("  NOTHING unless --control has been run and passed.")
    for x in v:
        print("  VIOLATION: " + x)
    if not v:
        print("  clean.")
    return 1 if v else 0


if __name__ == "__main__":
    sys.exit(main())
