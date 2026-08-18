#!/usr/bin/env python3
# ============================================================================
# bytetrue_recheck.py — RE-MEASURE EVERY CLAIMED BYTE-TRUE ROW AGAINST THE
#                       ARTIFACT IT NAMES.
#
# ---------------------------------------------------------------------------
# WHY (History/Bridge, 2026-08-17): they re-ran their own `msdan2`
# certification by hand "because the tree had moved under it", and stated the
# principle — **A CERTIFICATION IS ONLY AS GOOD AS ITS LAST RE-MEASURE.**
# That was named in prose and absent in mechanism, which is the gap this
# estate keeps rediscovering. This is the mechanism.
#
# A `byte-true: BYTE-TRUE` row is a CLAIM ABOUT AN ARTIFACT AT A MOMENT.
# Shared-header edits un-match previously-matching functions project-wide
# (which is why upstream ships `ninja changes`), and OUR tracker rows are a
# SEPARATE claim surface that nothing re-checks. A row can silently become a
# lie without anyone editing it.
#
# WHAT IT DOES: reads every tracker row carrying `byte-true: BYTE-TRUE` and a
# `src=ppc-rel-sha1:<rel>@<hash>`, re-hashes the named REL, and compares
# against BOTH the row's claimed hash AND the donor's `build.sha1`.
#
#   AGREE     artifact still hashes to the claimed value AND to the manifest
#   DRIFT     artifact exists and no longer matches            -> RED
#   MISSING   artifact absent (not rebuilt here) -> UNKNOWN, never clean
#   CIRCULAR  the manifest is locally modified   -> REFUSED (a hash matching
#             an edited manifest proves nothing — History's own check)
#
# Usage:  bytetrue_recheck.py            re-measure every claimed row
#         bytetrue_recheck.py --selftest NEGATIVE CONTROL
# Exit 0 = all AGREE · 1 = drift/refusal · 2 = nothing to check (UNKNOWN)
# ============================================================================
import hashlib
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
ROWS = REPO / "docs" / "state" / "ww-staging" / "tracker" / "rows"
DONOR = Path(r"D:/XXXXXXX/WW DP")
SHA1S = DONOR / "config" / "GZLE01" / "build.sha1"

# ---------------------------------------------------------------------------
# MATCH BROADLY, VALIDATE, SURFACE (§P3) — the first live run refused
# `s2f8ac419`, a row that carries the artifact path, the hash, the size AND
# the manifest-integrity check, purely because it wrote them as PROSE
# CITATIONS instead of the spec's canonical `src=ppc-rel-sha1:<rel>@<hash>`.
# **A re-checker that refuses the best row in the store is not strict, it is
# broken.** So: prefer the canonical form, FALL BACK to pairing a `.rel`
# filename with a 40-hex digest anywhere in the row, and report the missing
# canonical form as a NOTE rather than a refusal — the evidence is what
# matters; the syntax is what improves.
# ---------------------------------------------------------------------------
RE_SRC = re.compile(r"ppc-rel-sha1:\s*([\w.]+\.rel)\s*@\s*([0-9a-fA-F]{40})")
RE_REL = re.compile(r"([\w.]+\.rel)\b")
RE_SHA = re.compile(r"\b([0-9a-fA-F]{40})\b")


def manifest_dirty():
    """History's circularity check: is the expected-hash manifest ours?"""
    try:
        r = subprocess.run(["git", "status", "--porcelain", "--",
                            "config/GZLE01/build.sha1"],
                           capture_output=True, text=True, timeout=60,
                           cwd=str(DONOR))
        if r.returncode != 0:
            return None
        return bool((r.stdout or "").strip())
    except Exception:
        return None


def manifest_hash(rel_name):
    if not SHA1S.is_file():
        return None
    for ln in SHA1S.read_text(encoding="utf-8", errors="replace").splitlines():
        if ln.strip().endswith(rel_name):
            return ln.split()[0].lower()
    return None


def find_rel(rel_name):
    """The built artifact. dtk lays them out per-TU under build/<ver>/."""
    stem = rel_name[:-4]
    p = DONOR / "build" / "GZLE01" / stem / rel_name
    if p.is_file():
        return p
    hits = list((DONOR / "build").rglob(rel_name)) if (DONOR / "build").is_dir() else []
    return hits[0] if hits else None


def claims(rows_dir=None):
    """[(row_id, rel_name, claimed_hash)] for every BYTE-TRUE row."""
    d = Path(rows_dir) if rows_dir else ROWS
    out = []
    if not d.is_dir():
        return out
    for p in sorted(d.glob("*.md")):
        t = p.read_text(encoding="utf-8-sig", errors="replace")
        if not re.search(r"^byte-true:\s*BYTE-TRUE\s*$", t, re.M):
            continue
        m = RE_SRC.search(t)
        rid = (re.search(r"^id:\s*(\S+)", t, re.M) or [None, p.stem])[1] \
            if re.search(r"^id:\s*(\S+)", t, re.M) else p.stem
        if m:
            out.append((rid, m.group(1), m.group(2).lower(), True))
            continue
        rels, shas = RE_REL.findall(t), RE_SHA.findall(t)
        if rels and shas:
            # canonical form absent; the evidence is present anyway
            out.append((rid, rels[0], shas[0].lower(), False))
        else:
            out.append((rid, None, None, False))
    return out


def recheck(rows_dir=None, verbose=True):
    found = claims(rows_dir)
    if not found:
        if verbose:
            print("NOTHING TO CHECK — no row carries byte-true: BYTE-TRUE. "
                  "That is UNKNOWN coverage, not a clean bill.")
        return 2
    dirty = manifest_dirty()
    if dirty:
        if verbose:
            print("REFUSED — build.sha1 is LOCALLY MODIFIED. Every comparison "
                  "below would be circular (target = a copy of our own "
                  "output). Restore the manifest, then re-run.")
        return 1
    bad = 0
    noncanon = 0
    for rid, rel, claimed, canonical in found:
        if rel is None:
            print("  %s  ** NO ARTIFACT REFERENCE ** — a BYTE-TRUE row with "
                  "neither a ppc-rel-sha1 src nor a .rel+sha1 pair anywhere "
                  "cannot be re-measured (spec §3.3: no src => UNKNOWN)" % rid)
            bad += 1
            continue
        if not canonical:
            noncanon += 1
        art = find_rel(rel)
        if art is None:
            print("  %s  %-28s MISSING — artifact not built here; UNKNOWN, "
                  "not clean" % (rid, rel))
            continue
        got = hashlib.sha1(art.read_bytes()).hexdigest()
        want_mf = manifest_hash(rel)
        if got == claimed and (want_mf is None or got == want_mf):
            print("  %s  %-28s AGREE  %s" % (rid, rel, got[:12]))
        else:
            bad += 1
            print("  %s  %-28s ** DRIFT ** row=%s now=%s manifest=%s"
                  % (rid, rel, claimed[:12], got[:12],
                     (want_mf or "n/a")[:12]))
    if verbose:
        print("\n%d claim(s) re-measured%s%s"
              % (len(found),
                 "" if dirty is not None else "  (manifest integrity UNKNOWN)",
                 "" if not bad else "  ** %d PROBLEM(S) **" % bad))
        if noncanon:
            print("  note: %d row(s) carried the evidence as PROSE rather "
                  "than the spec's canonical src=ppc-rel-sha1:<rel>@<hash>. "
                  "Re-measured anyway (the evidence is what matters); "
                  "adopting the canonical form makes them machine-checkable "
                  "without a fallback." % noncanon)
    return 1 if bad else 0


def selftest():
    """NEGATIVE CONTROL — a row claiming a hash the artifact does not have
    MUST report DRIFT. A re-checker that cannot go red re-checks nothing."""
    import tempfile
    d = Path(tempfile.mkdtemp())
    real = claims()
    if not real or real[0][1] is None:
        print("SELFTEST — cannot run: no live BYTE-TRUE row with a src to "
              "model the fixture on. UNKNOWN, not a pass.")
        return 2
    rid, rel, good = real[0][0], real[0][1], real[0][2]
    poisoned = ("0" * 39) + "1"
    (d / "s00000001.md").write_text(
        "id: s00000001\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\n"
        "created: selftest\nportable: DECOMPILED\nbyte-true: BYTE-TRUE\n"
        "citations:\n  - src=ppc-rel-sha1:%s@%s (run 2026-08-17)\n"
        % (rel, poisoned), encoding="utf-8", newline="\n")
    print("SELFTEST — a poisoned claim MUST report DRIFT\n")
    rc = recheck(rows_dir=d, verbose=False)
    ok = (rc == 1)
    print("  [%-46s] %s" % ("poisoned hash on a real artifact -> DRIFT",
                            "DETECTED" if ok else "**BLIND**"))
    (d / "s00000002.md").write_text(
        "id: s00000002\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\n"
        "created: selftest\nportable: DECOMPILED\nbyte-true: BYTE-TRUE\n",
        encoding="utf-8", newline="\n")
    rc2 = recheck(rows_dir=d, verbose=False)
    ok2 = (rc2 == 1)
    print("  [%-46s] %s" % ("BYTE-TRUE row with NO src -> flagged",
                            "DETECTED" if ok2 else "**BLIND**"))
    blind = [ok, ok2].count(False)
    print("\n%s" % ("CONTROL OK - all cases DETECTED" if not blind
                    else "** %d BLIND CASE(S) **" % blind))
    return 1 if blind else 0


def main():
    if "--selftest" in sys.argv[1:]:
        return selftest()
    print("BYTE-TRUE RE-CHECK — every claim re-measured against its artifact")
    return recheck()


if __name__ == "__main__":
    sys.exit(main())
