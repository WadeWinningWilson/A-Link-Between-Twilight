#!/usr/bin/env python3
# ============================================================================
# tier2_selftest.py — known-answer control for tier2_census.classify_file.
#
# SELF-DESCRIBING: expectations are parsed from the fixture's own `EXPECT`
# annotations, so line numbers cannot drift out from under the test — the
# first version hardcoded them and two "failures" were the fixture moving,
# not the classifier failing. An expectation that lives next to the thing it
# describes is the load-bearing-declaration principle applied to the test.
#
# Exit 0 = every expectation held (control FIRED) · 1 = classifier blind.
# ============================================================================
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

import tier2_census as T2

FIX = HERE / "tier2_fixture.cpp"
RE_EXPECT = re.compile(r"//\s*EXPECT\s+(B|A|C|not counted)")

expect = {}
for i, ln in enumerate(FIX.read_text(encoding="utf-8").splitlines(), 1):
    m = RE_EXPECT.search(ln)
    if m:
        expect[i] = None if m.group(1) == "not counted" else m.group(1)

got = {ln: (cls, host) for ln, cls, host in T2.classify_file(FIX)}

fails = 0
for line in sorted(expect):
    want = expect[line]
    have = got.get(line)
    if want is None:
        ok = have is None
        detail = "counted as %s" % (have,) if have else "not counted"
    else:
        ok = have is not None and have[0] == want
        detail = "%s" % (have,)
    print("  [%s] line %-3d expect %-12s -> %s"
          % ("ok " if ok else "FAIL", line, want or "not-counted", detail))
    fails += 0 if ok else 1

# host-name quality: the C row's host must be a real identifier, not the
# one-letter capture the greedy RE_TABLE produced ("table e")
c_hosts = [h for (cls, h) in got.values() if cls == "C"]
if c_hosts and any(len(h) < 3 for h in c_hosts):
    print("  [FAIL] C-class host name degenerate: %r" % c_hosts)
    fails += 1
elif c_hosts:
    print("  [ok ] C-class host names are citable: %r" % c_hosts)

print()
if fails:
    print("**%d expectation(s) FAILED — the classifier is blind in that "
          "direction; its counts are not citable.**" % fails)
    sys.exit(1)
print("ALL %d expectations held — control FIRED in every direction."
      % (len(expect) + (1 if c_hosts else 0)))
sys.exit(0)
