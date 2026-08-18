#!/usr/bin/env python3
# ============================================================================
# seam_gate.py — B1: baseline-diff + ownership classifier, FOR THE TOOLCHAIN.
#
#   check    the gate. exit 0 = build continues, 1 = build stops.
#   explain  what it would check, and why, without judging anything
#   selftest NEGATIVE CONTROL — each block reason must be shown to fire
#
# ---------------------------------------------------------------------------
# B1's three inputs, all real as of 2026-08-16:
#   · ww-ownership-map.json   the CLASSIFIER (UNKNOWN blocks)
#   · tracker/rows/           the ROW AUTHORITY
#   · row_baseline.py         the DIFF ANCHOR
#
# B2 IS NOT A SEPARATE FEATURE — IT IS THIS FILE'S OUTPUT CONTRACT:
#   THE FAILURE MESSAGE IS THE DOCUMENTATION. Every block names
#     (a) the FILE,  (b) WHY it blocked,  (c) the EXACT FIX.
#   "reconciliation failed" is banned. A failure a builder cannot act on
#   trains them to bypass the gate, and a bypassed gate protects nothing.
#
# ---------------------------------------------------------------------------
# WHY THE CLASSIFIER ENUMERATES THE MOVING SIDE (§P4, proved 2026-08-16):
#   `ww-ownership-map.json` declares 11 categories and the data uses 6 —
#   five are LATENT. Listing "what blocks" would let a new category through
#   silently; listing what is KNOWN-OK and blocking everything else fails
#   pessimistically, which is the direction a human notices.
# ============================================================================
import json
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
STAGING = REPO / "docs" / "state" / "ww-staging"
MAP = STAGING / "ww-ownership-map.json"
TRACKER = STAGING / "tracker"

# KNOWN-OK ownership categories. Anything else — including a category invented
# tomorrow — blocks. Enumerating this side is the whole point (see header).
CLASSIFIED_OK = {"ww-port", "shared-per-hunk", "albw", "albw-skins",
                 "instrument", "fork-host-plumbing", "upstream-native", "not-ww"}
BLOCKING = {"UNKNOWN-VERDICT", "dual-pending", "stale-path"}


# WW-shaped content markers for the new-to-the-universe detector. Bounded
# and named: the KIT-LINEAGE tag itself, the dWw/dExtWw prefixes (tier2's
# repaired shape), and the plugin-era WwJMessage/wwRegistry/wwMessage
# families. A marker hit on an UNMAPPED changed file blocks - matching
# broadly and surfacing loudly is s.P3; a false positive costs a worksheet
# line, a false negative costs the patch envelope.
WW_MARKERS = re.compile(r'KIT-LINEAGE|\bdWw[A-Z]|\bdExtWw|\bWwJMessage\b'
                        r'|\bwwRegistry_|\bwwMessage')


def load_map():
    if not MAP.is_file():
        return {}
    d = json.loads(MAP.read_text(encoding="utf-8", errors="replace"))
    items = d.get("files", d) if isinstance(d, dict) else d
    if isinstance(items, dict):
        items = [dict(file=k, **(v if isinstance(v, dict) else {})) for k, v in items.items()]
    return {i["file"]: (i.get("category") or "") for i in items
            if isinstance(i, dict) and i.get("file")}


def changed_files():
    """Working-tree changes vs HEAD. The gate judges what THIS build carries."""
    try:
        r = subprocess.run(["git", "status", "--porcelain"], capture_output=True,
                           text=True, cwd=str(REPO), timeout=60)
    except Exception:
        return []
    out = []
    for line in (r.stdout or "").splitlines():
        p = line[3:].strip().strip('"')
        if "->" in p:
            p = p.split("->")[-1].strip()
        out.append(p.replace("\\", "/"))
    return out


def block(problems, path, why, fix):
    problems.append({"file": path, "why": why, "fix": fix})


def check(verbose=True):
    problems = []
    omap = load_map()

    # (1) ROW AUTHORITY — a baselined row gone with no retirement.
    try:
        import importlib.util
        spec = importlib.util.spec_from_file_location("rb", HERE / "row_baseline.py")
        rb = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(rb)
        rc, vanished = rb.check(verbose=False)
        for v in vanished:
            block(problems, "docs/state/ww-staging/tracker/rows/%s.md" % v,
                  "this row was in the pinned baseline and is now gone, with no "
                  "entry in tracker/RETIRED.md",
                  "EITHER restore the file, OR add a line naming %s to "
                  "docs/state/ww-staging/tracker/RETIRED.md. Do NOT re-pin the "
                  "baseline to silence this — re-pinning is the bypass this gate exists "
                  "to close (SPEC-row-store §5)." % v)
    except Exception as e:
        block(problems, "tools/foundry/row_baseline.py",
              "the diff anchor could not be loaded: %s" % e,
              "restore tools/foundry/row_baseline.py, then re-run "
              "`python tools/foundry/seam_gate.py check`.")

    # (2) CLASSIFIER — every changed WW-layer file must have a KNOWN ownership.
    for f in changed_files():
        if not (f.startswith("src/") or f.startswith("include/")):
            continue
        if f not in omap:
            # ============================================================
            # NEW-TO-THE-UNIVERSE DETECTOR (Foundry 2026-08-16, on the
            # user's envelope assurance). This branch used to read 'not in
            # the WW universe: not this gate's business' and SKIP - which
            # meant a brand-new WW TU, or WW content landing in a
            # previously-untouched receiver file, shipped silently past the
            # only gate wired into the build. New WW work outside the map
            # is invisible to the ratchet (counts tagged TUs), the drain
            # (walks the map), and therefore the PATCH ENVELOPE. A marker
            # scan on unmapped changed files closes the envelope: every
            # WW-shaped edit is either classified or blocks.
            # ============================================================
            fp = REPO / f
            if fp.is_file():
                try:
                    txt = fp.read_text(encoding='utf-8', errors='replace')
                except OSError:
                    txt = ''
                if WW_MARKERS.search(txt):
                    block(problems, f,
                          'carries WW markers but is NOT in the ownership '
                          'map - a NEW WW-universe file that the ratchet, '
                          'the drain, and the patch envelope cannot see',
                          'adjudicate it: VERDICT row in OWNERSHIP-'
                          'WORKSHEET.md, re-cut ww-ownership-map.json, tag '
                          'the TU with KIT-LINEAGE, mint its tracker row '
                          '(row_store.py mint). If it is NOT WW work, say '
                          'so in the worksheet - a false marker hit gets a '
                          'not-ww verdict, never a silent skip.')
            continue
        cat = omap[f]
        if cat in CLASSIFIED_OK:
            continue
        why = ("ownership category %r is unresolved" % cat) if cat in BLOCKING else \
              ("ownership category %r is not in the known-good set" % cat)
        block(problems, f, why,
              "rule this file in docs/state/ww-staging/OWNERSHIP-WORKSHEET.md (VERDICT "
              "column), then have Foundry re-cut ww-ownership-map.json. If %r is a NEW "
              "and legitimate category, add it to CLASSIFIED_OK in seam_gate.py in the "
              "same change — the block is deliberate: a category nobody has classified "
              "must not ship silently." % cat)

    if verbose:
        if not problems:
            print("SEAM GATE: PASS — row authority intact, no unclassified WW-layer changes.")
        else:
            print("=" * 72)
            print("SEAM GATE: BUILD STOPPED — %d problem(s). Each one names its fix."
                  % len(problems))
            print("=" * 72)
            for i, p in enumerate(problems, 1):
                print("\n  [%d] FILE : %s" % (i, p["file"]))
                print("      WHY  : %s" % p["why"])
                print("      FIX  : %s" % p["fix"])
            print("\n" + "=" * 72)
    return (1 if problems else 0), problems


def explain():
    omap = load_map()
    print("SEAM GATE — inputs and scope (judging nothing)")
    print("  classifier : %s (%d files)" % (MAP.relative_to(REPO), len(omap)))
    print("  rows       : %s" % (TRACKER / "rows").relative_to(REPO))
    print("  anchor     : tools/foundry/row_baseline.py")
    print("  known-OK   : %s" % sorted(CLASSIFIED_OK))
    print("  blocking   : %s (+ anything unlisted)" % sorted(BLOCKING))
    ch = [f for f in changed_files() if f in omap]
    print("  changed WW-layer files in this tree: %d" % len(ch))
    return 0


def selftest():
    """Each block reason shown to fire. A gate that only passes is worthless.

    THE FIRST VERSION OF THIS TEST WAS VACUOUS AND SAID SO (2026-08-16): it
    emptied CLASSIFIED_OK against the LIVE tree, which had zero changed
    WW-layer files, so the classifier had nothing to judge and "did not fire"
    was indistinguishable from "cannot fire". A test that depends on the tree
    happening to contain the right conditions is not a test. It now INJECTS a
    synthetic changed-file set drawn from the real map.
    """
    global CLASSIFIED_OK, changed_files
    print("SELFTEST — each block reason must fire\n")
    results = []
    rc, probs = check(verbose=False)
    # LIVE TREE IS INFORMATIONAL, NOT A SOUNDNESS CASE (2026-08-16): a
    # dirty tree is not an unsound gate. The day the new-universe
    # detector landed it found two REAL unadjudicated WW files in the
    # working tree - counting that against the GATE would teach people
    # to fix the selftest instead of the tree.
    print("  [live tree: %s - informational, not a soundness case]"
          % ("clean" if rc == 0 else "%d finding(s), see check" % len(probs)))

    omap = load_map()
    victim = next((f for f, c in sorted(omap.items()) if c == "ww-port"), None)
    if victim is None:
        print("  ** cannot run classifier case: no ww-port file in the map **")
        return 1
    real_changed, keep = changed_files, CLASSIFIED_OK
    changed_files = lambda: [victim]          # synthetic condition, not the tree
    rc, probs = check(verbose=False)
    results.append(("known-good category passes", 1 if probs else 0, 0))
    CLASSIFIED_OK = set()                      # now nothing is classified
    rc, probs = check(verbose=False)
    cls = [p for p in probs if "ownership category" in p["why"]]
    results.append(("classifier blocks unclassified file", 1 if cls else 0, 1))
    CLASSIFIED_OK, changed_files = keep, real_changed
    # new-to-the-universe: an UNMAPPED src/ file carrying a WW marker
    # must block (the envelope case; fires the detector added 08-16)
    tmp = REPO / 'src' / '.seamgate_ctl_tmp.cpp'
    try:
        tmp.write_text('// KIT-LINEAGE: probe\n// dWwCtl marker\n',
                       encoding='utf-8')
        changed_files = lambda: ['src/.seamgate_ctl_tmp.cpp']
        rc, probs = check(verbose=False)
        env = [p for p in probs if 'NOT in the ownership map' in p['why']]
        results.append(('new-universe marker file blocks',
                        1 if env else 0, 1))
    finally:
        changed_files = real_changed
        if tmp.is_file():
            tmp.unlink()
    bad = 0
    for name, got, want in results:
        ok = got == want
        bad += 0 if ok else 1
        print("  [%-38s] got=%d want=%d  %s" % (name, got, want, "OK" if ok else "** WRONG **"))
    if cls:
        print("\n  sample refusal (B2 contract — file, why, fix):")
        p = cls[0]
        print("      FILE : %s" % p["file"])
        print("      FIX  : %s" % p["fix"][:100])
    print("\n%d/%d correct%s" % (len(results) - bad, len(results),
          "" if not bad else "  ** GATE UNSOUND **"))
    return 1 if bad else 0


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "check":
        rc, _ = check()
        return rc
    if cmd == "explain":
        return explain()
    if cmd == "selftest":
        return selftest()
    print("usage: seam_gate.py check | explain | selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
