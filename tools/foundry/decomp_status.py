#!/usr/bin/env python3
# ===========================================================================
# decomp_status.py — V9's authoritative axis: the donor's OWN configure.py.
#
# WHY (ferry §762): nothing of ours read configure.py — the donor project's
# own per-TU match ledger — and the source-text heuristic ("Nonmatching" in
# the .cpp) is ambiguous alone. V9 = TWO AXES, paired, disagreement flagged
# never adjudicated (V1's principle):
#   AXIS A  configure.py status   (this module — authoritative for linking)
#   AXIS B  source-marker count   (crash_recipe_lint.donor_is_stub — texture)
#
# THE PARSER CAVEAT, honored and measured: ActorRel(status, "name") expands to
# d/actor/<name>.cpp — an Object()-only parser silently loses ALL 415 actor
# RELs (measured on the pinned donor: 415 ActorRel + ~687 Object rows).
#
# Status decode against OUR disc version (GZLE01):
#   Matching                      -> MATCHED
#   NonMatching                   -> NONMATCHING
#   Equivalent                    -> EQUIVALENT (matches with --non-matching)
#   MatchingFor(...versions)      -> MATCHED if GZLE01 listed, else NONMATCHING
#   EquivalentFor(...versions)    -> EQUIVALENT if listed, else NONMATCHING
#
# INTEGRATOR STANDING CHECK (user directive): any port-class claim verifies
# against this —  decomp_status.py <tu-or-actor-name>  — one command.
#
# Usage:
#   decomp_status.py d_a_knob00            one TU/actor, both axes
#   decomp_status.py --summary             corpus counts by status
#   decomp_status.py --selftest
# ===========================================================================
import io
import re
import sys
from pathlib import Path

DONOR_ROOT = Path("D:/XXXXXXX/WW DP")
CONFIGURE = DONOR_ROOT / "configure.py"
OUR_VERSION = "GZLE01"

RE_OBJECT = re.compile(
    r"Object\(\s*(Matching|NonMatching|Equivalent"
    r"|MatchingFor\(([^)]*)\)|EquivalentFor\(([^)]*)\))\s*,\s*"
    r"f?\"([^\"]+\.cpp)\"")
RE_ACTORREL = re.compile(
    r"ActorRel\(\s*(Matching|NonMatching|Equivalent"
    r"|MatchingFor\(([^)]*)\)|EquivalentFor\(([^)]*)\))\s*,\s*\"([^\"]+)\"")


def _decode_expr(status_raw, version=OUR_VERSION):
    """Decode a full status EXPRESSION, including compounds.

    Briefing-v2 correction (interconnected advisory, adopted with receipt):
    the old regex could not see compound expressions like
    `EquivalentFor(...) or MatchingFor(...)` — the row VANISHED WITH NO ERROR
    (№31-C failure class; NPC_PEOPLE and its 19 DZR names were simply absent).
    Compound `A or B` = best of the alternatives for our version."""
    s = status_raw.strip()
    parts = re.split(r"\s+or\s+", s)
    best = None
    RANK = {"MATCHED": 0, "EQUIVALENT": 1, "NONMATCHING(version)": 2,
            "NONMATCHING": 3, "UNKNOWN": 4}
    for p in parts:
        p = p.strip()
        m = re.match(r"(MatchingFor|EquivalentFor)\s*\((.*)\)\s*$", p, re.S)
        if m:
            hit = version in m.group(2)
            v = ("MATCHED" if m.group(1) == "MatchingFor" else "EQUIVALENT") \
                if hit else "NONMATCHING(version)"
        else:
            v = {"Matching": "MATCHED", "NonMatching": "NONMATCHING",
                 "Equivalent": "EQUIVALENT"}.get(p, "UNKNOWN")
        if best is None or RANK[v] < RANK[best]:
            best = v
    return best or "UNKNOWN"


def parse_configure(path=CONFIGURE, version=OUR_VERSION):
    """-> {tu_path: status}. Object rows verbatim; ActorRel rows expanded to
    d/actor/<name>.cpp (the caveat — 415 entries live or die on this)."""
    txt = io.open(path, encoding="utf-8", errors="replace").read()
    # drop the helper DEFINITIONS so the def bodies don't parse as entries
    txt = re.sub(r"def (ActorRel|JSystemLib)\(.*?\n\n", "", txt, flags=re.S)
    # PAREN-AWARE SCAN (briefing-v2 technique, adopted): walk to the row's
    # top-level comma so compound/nested status expressions survive intact —
    # "scan, don't regex, over a config where terms compose."
    out = {}
    for m in re.finditer(r"\b(Object|ActorRel)\(", txt):
        i, depth = m.end(), 0
        while i < len(txt):
            c = txt[i]
            if c == "(":
                depth += 1
            elif c == ")":
                if depth == 0:
                    break
                depth -= 1
            elif c == "," and depth == 0:
                break
            i += 1
        status_raw = txt[m.end():i]
        nm = re.match(r'\s*,\s*f?"([^"]+)"', txt[i:])
        if not nm:
            continue
        name = nm.group(1)
        path_key = name if m.group(1) == "Object" else "d/actor/%s.cpp" % name
        out[path_key] = _decode_expr(status_raw, version)
    return out


def source_axis(tu_path):
    """AXIS B: the source-marker heuristic. Returns (count, verdict-text)."""
    p = DONOR_ROOT / "src" / tu_path
    if not p.is_file():
        return None, "source file absent (%s)" % p
    n = len(re.findall(r"NONMATCHING|Nonmatching", p.read_text(encoding="utf-8", errors="replace")))
    return n, ("%d non-matching markers in source" % n)


def report(name, table=None):
    table = table or parse_configure()
    # accept a bare actor name, a TU path, or a stem
    cands = [name, "d/actor/%s.cpp" % name, name + ".cpp"]
    tu = next((c for c in cands if c in table), None)
    if tu is None:
        hits = [k for k in table if name in k]
        if len(hits) == 1:
            tu = hits[0]
        elif hits:
            print("ambiguous %r: %s" % (name, hits[:6]))
            return 2
        else:
            print("%r not in configure.py (checked Object AND ActorRel rows)" % name)
            return 2
    a = table[tu]
    b_n, b_txt = source_axis(tu)
    print("V9 two-axis decomp status — %s" % tu)
    print("  AXIS A (configure.py, authoritative): %s" % a)
    print("  AXIS B (source markers):              %s" % b_txt)
    if a == "MATCHED" and b_n:
        print("  DISAGREEMENT — configure says MATCHED but source carries markers.")
        print("  Flagged, not adjudicated: read the TU before trusting either.")
        return 1
    if a.startswith("NONMATCHING") and b_n == 0 and b_n is not None:
        print("  DISAGREEMENT — configure says NONMATCHING but source is marker-free.")
        print("  Flagged, not adjudicated.")
        return 1
    print("  axes agree.")
    return 0


def summary():
    t = parse_configure()
    actors = {k: v for k, v in t.items() if k.startswith("d/actor/")}
    from collections import Counter
    print("configure.py corpus (version %s): %d TUs (%d actor RELs)"
          % (OUR_VERSION, len(t), len(actors)))
    for k, v in Counter(t.values()).most_common():
        print("  %-22s %d" % (v, k)) if False else print("  %-22s %d" % (k, v))
    return 0


def selftest():
    sample = '''
        Object(Matching,    "m_Do/m_Do_main.cpp"),
        Object(NonMatching, "m_Do/m_Do_graphic.cpp"),
        Object(MatchingFor("GZLJ01", "GZLE01"), "m_Do/m_Do_machine.cpp"),
        Object(MatchingFor("GZLJ01"), "m_Do/m_Do_other.cpp"),
        ActorRel(Matching,    "d_a_andsw2"),
        ActorRel(MatchingFor("GZLJ01", "GZLE01", "GZLP01"),    "d_a_att"),
        ActorRel(NonMatching, "d_a_stub"),
    '''
    import tempfile
    p = Path(tempfile.mkdtemp()) / "configure.py"
    io.open(p, "w", encoding="utf-8").write(sample)
    t = parse_configure(p)
    checks = [
        (t.get("m_Do/m_Do_main.cpp") == "MATCHED"),
        (t.get("m_Do/m_Do_graphic.cpp") == "NONMATCHING"),
        (t.get("m_Do/m_Do_machine.cpp") == "MATCHED"),
        (t.get("m_Do/m_Do_other.cpp") == "NONMATCHING(version)"),
        (t.get("d/actor/d_a_andsw2.cpp") == "MATCHED"),      # the ActorRel caveat
        (t.get("d/actor/d_a_att.cpp") == "MATCHED"),
        (t.get("d/actor/d_a_stub.cpp") == "NONMATCHING"),
        (len(t) == 7),
    ]
    ok = all(checks)
    print("selftest checks:", checks)
    # live-corpus count check: the caveat's whole point
    live = parse_configure()
    actors = sum(1 for k in live if k.startswith("d/actor/"))
    print("live corpus: %d TUs, %d actor RELs (caveat holds iff ~415)" % (len(live), actors))
    ok = ok and actors > 400
    print("selftest:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    if "--selftest" in sys.argv:
        raise SystemExit(selftest())
    if "--summary" in sys.argv:
        raise SystemExit(summary())
    if len(sys.argv) > 1:
        raise SystemExit(report(sys.argv[1]))
    print(__doc__ or "see header")
    raise SystemExit(2)
