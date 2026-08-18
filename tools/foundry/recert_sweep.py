#!/usr/bin/env python3
# ============================================================================
# recert_sweep.py - A CERTIFICATION IS ONLY AS GOOD AS ITS LAST RE-MEASURE.
#
# RESCUED FROM A DYING SCRATCHPAD (Librarian, 2026-08-17). History/Bridge
# stood down and their close-out named this as AT RISK OF LOSS: it lived in
# their session scratchpad, not in tools/, and Housing Security recorded
# "I cannot promote it - it is in another instance's scratchpad, unreachable
# from here." It was reachable from this session; promoted verbatim below.
#
# **This is the third artifact this week that existed only in session-temp**
# (the Librarian watcher, the Integrator watcher, and now this). A scratchpad
# is a workbench, not a shelf: 982 files currently sit in sibling scratchpads
# across this project, every one of which dies with its session.
#
# WHY IT MATTERS MORE THAN MOST: it is SELF-MAINTAINING. It reads every
# BYTE-TRUE row in the tracker and recovers the TU and sha1 from that row's
# own citations, so certifying a new TU adds it to the sweep automatically.
# The earlier `recert_sweep.py` it replaced carried a HARDCODED list - which
# is exactly how a certification quietly ages out when someone forgets to
# extend it. Losing this would not have lost a script; it would have lost the
# property that the list cannot go stale.
#
# LOGIC BELOW IS VERBATIM from History/Bridge. Only this header was added.
#
# Usage:  recert_sweep.py        (exit 0 = all certifications hold)
# ============================================================================
import glob, hashlib, io, os, re, subprocess, sys
sys.stdout.reconfigure(encoding='utf-8', errors='replace')

ROOT = "D:/XXXXXXX/WW DP"
ROWS = "docs/state/ww-staging/tracker/rows"

certs = []
for p in sorted(glob.glob(os.path.join(ROWS, "*.md"))):
    t = io.open(p, encoding='utf-8', errors='replace').read()
    if not re.search(r'^byte-true:\s*BYTE-TRUE\s*$', t, re.M):
        continue
    h = re.search(r'\b([0-9a-f]{40})\b', t)
    tu = re.search(r'build/GZLE01/([A-Za-z0-9_]+)/\1\.rel', t)
    certs.append((os.path.basename(p)[:-3], tu.group(1) if tu else None,
                  h.group(1) if h else None))

print("BYTE-TRUE rows in the tracker: %d\n" % len(certs))
allok = True
for row, tu, h in certs:
    if not tu or not h:
        print("  %-11s UNPARSEABLE (tu=%r hash=%r)" % (row, tu, h)); allok = False; continue
    p = os.path.join(ROOT, "build/GZLE01/%s/%s.rel" % (tu, tu))
    if not os.path.exists(p):
        print("  %-11s %-22s ARTIFACT MISSING" % (row, tu)); allok = False; continue
    now = hashlib.sha1(open(p, 'rb').read()).hexdigest()
    ok = now == h
    allok &= ok
    print("  %-11s %-22s %s" % (row, tu, "STILL VALID" if ok else "*** DRIFTED ***"))
    if not ok:
        print("      certified %s\n      now       %s" % (h, now))

print("\nALL CERTIFICATIONS HOLD: %s" % allok)

r = subprocess.run(["git", "status", "--porcelain", "config/GZLE01/build.sha1"],
                   cwd=ROOT, capture_output=True, text=True)
print("expected-hash manifest unmodified: %s" % (not r.stdout.strip()))
