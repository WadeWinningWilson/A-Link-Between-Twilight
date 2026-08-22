#!/usr/bin/env python3
# ===========================================================================
# donor_shape_diff.py - DOES OUR PORT DO THE SAME THINGS IN THE SAME ORDER?
#
# WHY NOT A TEXT DIFF - established by running one first, which is the only
# honest way to disqualify a metric. Our ports live on RECEIVER storage:
#     donor   return &g_env_light.mWind.mWindVec;
#     plugin  return &s_wind.mWindVec;
# Identical behaviour, different storage, because the plugin owns its own
# channel (vanilla env_light ends at 0x1310). A line-level diff scored that
# pair at ratio 0.00 and called it DIVERGED. **A metric that reports DIVERGED
# for a faithful port reports DIVERGED for everything and therefore says
# nothing.** Five of five came back DIVERGED before I stopped and looked.
#
# WHAT THIS COMPARES INSTEAD: the SHAPE - the ordered sequence of control-flow
# constructs and CALLED FUNCTION NAMES, with operands and storage expressions
# dropped. That is translation-tolerant by construction: renaming a variable
# or moving a field cannot change it, while reordering logic, dropping a
# branch or losing a call does.
#
# WHAT IT CANNOT TELL YOU, said plainly so nobody over-reads a number: shape
# equality is NOT behavioural equality. Two functions with identical shape can
# differ in a constant, a comparison direction, or an operand order. This
# NARROWS the review - it does not replace it. A SHAPE-MATCH verdict means
# "worth a human confirming"; a SHAPE-DIVERGED verdict means "something
# structural actually changed, look here first".
#
# It also cannot see through a receiver call that WRAPS a donor call, so a
# faithful port routed via a helper reads as diverged. That is a false
# positive by design - over-flagging costs a reviewer a minute; under-flagging
# silently blesses a broken port.
#
# Usage:
#   donor_shape_diff.py <plugin.cpp> <pluginFn> <donorUnit.cpp> <donorFn>
#   donor_shape_diff.py --batch <file.tsv>     (one pair per line, tab-sep)
#   donor_shape_diff.py --selftest
# ===========================================================================
import io
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

DONOR = Path(r"D:\XXXXXXX\WW DP\src")
KEYWORDS = ("if", "else", "for", "while", "switch", "case", "return",
            "break", "continue", "goto", "do")
# calls we ignore: they are noise the port is free to add
NOISE = re.compile(r'^(?:printf|sprintf|snprintf|vsnprintf|OS\w*|JUT\w*|'
                   r'\w*[Ll]og\w*|assert\w*)$')


def extract(path, name):
    # A pair list goes stale the moment another lane lands a split. A
    # missing file is a SKIP, not a crash -- crashing loses every result
    # after it, which is how one stale row cost a whole batch earlier.
    if not Path(path).is_file():
        return None
    t = io.open(path, encoding="utf-8", errors="replace").read()
    t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
    m = re.search(r'^[A-Za-z_][\w:<>,\s\*&]*?\b%s\s*\([^;]*?\)\s*(?:const\s*)?\{'
                  % re.escape(name), t, re.M)
    if not m:
        return None
    i, d = m.end() - 1, 0
    while i < len(t):
        if t[i] == '{':
            d += 1
        elif t[i] == '}':
            d -= 1
            if d == 0:
                return re.sub(r'//[^\n]*', '', t[m.end():i])
        i += 1
    return None


def shape(body):
    """Ordered control constructs + called names. Operands dropped."""
    out = []
    for tok in re.finditer(r'\b([A-Za-z_]\w*)\s*(\()?', body):
        word, is_call = tok.group(1), bool(tok.group(2))
        if word in KEYWORDS:
            out.append(word)
        elif is_call and not NOISE.match(word):
            # strip lineage prefixes so dKyw_ / dKyWw_ compare equal
            n = re.sub(r'^(dKyWw|dKyw|dKy|daSea|da|ww)_?', '', word, flags=re.I)
            out.append("call:" + n.lower())
    return out


def classify_thin(plugin_body, donor_fn):
    """A THIN port has two opposite meanings and they must not be conflated.

    Found live 2026-08-22: `dKyWw_settingTevStruct` carries 2 shape tokens
    against the donor's 41 - ratio 0.05, the thinnest in the tree. That reads
    as a gap and IS THE OPPOSITE. Its whole body is

        g_env_light.settingTevStruct(...);   // the RECEIVER's own
        dKyWw_overlayTevStruct(...);         // our WW translation on top

    which is DN-10 done right: the receiver already implements the donor's 137
    lines, so the port DELEGATES instead of duplicating. A ranking that called
    that a gap would have sent someone to "fix" the most exemplary file in the
    tree.

    The discriminator is whether the thin body CALLS an equivalent. If it
    does, thin means DELEGATES. If it is empty or only returns, thin means
    STUB. Same number, opposite verdicts."""
    if plugin_body is None:
        return "?"
    base = re.sub(r'^(dKyWw|dKyw|dKy|daSea|da|ww)_?', '', donor_fn.split("::")[-1],
                  flags=re.I).lower()
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(', plugin_body):
        cand = re.sub(r'^(dKyWw|dKyw|dKy|daSea|da|ww)_?', '', m.group(1),
                      flags=re.I).lower()
        if cand == base:
            return "DELEGATES"
    stripped = [l.strip() for l in plugin_body.splitlines() if l.strip()]
    if not stripped or all(l.startswith("return") for l in stripped):
        return "STUB"
    return "OWN-IMPL"


def compare(pf, pn, df, dn):
    a_raw = extract(pf, pn)
    b_raw = extract(DONOR / df if not Path(df).is_absolute() else df, dn)
    if a_raw is None or b_raw is None:
        return None, ("plugin fn not found" if a_raw is None else "donor fn not found")
    import difflib
    a, b = shape(a_raw), shape(b_raw)
    if not a and not b:
        return 1.0, "both empty"
    r = difflib.SequenceMatcher(None, a, b).ratio()
    verdict = ("SHAPE-MATCH" if a == b else
               "SHAPE-NEAR" if r >= 0.85 else "SHAPE-DIVERGED")
    return r, "%s  plugin=%d donor=%d tokens" % (verdict, len(a), len(b))


def selftest():
    import tempfile
    tmp = Path(tempfile.mkdtemp(prefix="shape_"))
    d = tmp / "d"
    d.mkdir()
    # storage renamed only -> must MATCH
    (tmp / "plug.cpp").write_text(
        "void f() {\n  if (x) { doThing(a); }\n  return s_wind.v;\n}\n"
        "void g() {\n  doThing(a);\n}\n", encoding="utf-8")
    (d / "don.cpp").write_text(
        "void f() {\n  if (y) { dKyw_doThing(b); }\n  return g_env.mWind.v;\n}\n"
        "void g() {\n  doThing(a);\n  extraCall(c);\n}\n", encoding="utf-8")
    r1, v1 = compare(tmp / "plug.cpp", "f", d / "don.cpp", "f")
    r2, v2 = compare(tmp / "plug.cpp", "g", d / "don.cpp", "g")
    checks = [("storage rename still MATCHES", v1.startswith("SHAPE-MATCH")),
              ("a dropped call is caught", not v2.startswith("SHAPE-MATCH"))]
    print("SELFTEST - the metric must survive a rename and still catch a loss")
    for n, ok in checks:
        print("  %-32s %s" % (n, "PASS" if ok else "*** FAIL ***"))
    ok = all(o for _, o in checks)
    print("  SELFTEST %s" % ("PASSED" if ok else "*** FAILED ***"))
    import shutil
    shutil.rmtree(tmp, ignore_errors=True)
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    if "--batch" in sys.argv:
        src = Path(sys.argv[sys.argv.index("--batch") + 1])
        rows, skipped = [], []
        for _n, _l in enumerate(io.open(src, encoding="utf-8"), 1):
            if not _l.strip() or _l.startswith("#"):
                continue
            _p = _l.rstrip("\n").split("\t")
            # A malformed row must not take the batch down with it: one bad
            # line was losing 50 good results to a crash. Skip it LOUDLY --
            # a silently dropped row is a pair nobody knows went unchecked,
            # which is the same silence-reads-as-absence trap as everything
            # else this lane has hit today.
            if len(_p) == 4:
                rows.append(_p)
            else:
                skipped.append((_n, _l.strip()[:70]))
        print("SHAPE DIFF - plugin vs donor (%d pair(s))\n" % len(rows))
        tally = {}
        for pf, pn, df, dn in rows:
            r, v = compare(pf, pn, df, dn)
            key = v.split()[0]
            tally[key] = tally.get(key, 0) + 1
            print("  %-26s %s" % (pn, v))
        print("\n  " + " | ".join("%s %d" % kv for kv in sorted(tally.items())))
        if skipped:
            print("  *** %d MALFORMED ROW(S) SKIPPED - NOT checked and NOT passed:"
                  % len(skipped))
            for _n, _txt in skipped[:5]:
                print("      line %d: %s" % (_n, _txt))
        return 0
    if len(sys.argv) < 5:
        print(__doc__ or "see header")
        return 2
    r, v = compare(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    print("%s  (ratio %.2f)" % (v, r if r is not None else -1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
