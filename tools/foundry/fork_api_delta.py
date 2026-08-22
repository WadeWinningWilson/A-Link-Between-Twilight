#!/usr/bin/env python3
# ============================================================================
# fork_api_delta.py - ENUMERATE FORK-ONLY API BEFORE ANYONE PORTS A LINE.
#
# WHY. A typed (header-ful) plugin compiles against the VANILLA headers, so it
# sees VANILLA's API surface: every fork-added declaration is invisible to it.
# That is a real Phase-5 cost and it was FOUND BY ACCIDENT - a probe TU called
# `dStage_roomControl_c::getBgW`, which exists in the fork (`d_stage.h:1226`)
# and NOT on vanilla, and the compiler refused. Discovering that surface one
# compile error at a time is avoidable: it is enumerable ahead of time.
#
# THE TRADE THIS SERVES, worth restating because it is the argument for going
# typed at all: INLINING blindness returns silence indistinguishable from "the
# code never ran" - five hypotheses died on it in one night. API DIVERGENCE
# fails AT COMPILE TIME. Not a free trade, a strictly better one. This tool
# prices it.
#
# ---------------------------------------------------------------------------
# THE CONTROL IS THE POINT. A previous sweep for exactly this returned ZERO
# fork-only names and its author correctly refused to publish the zero: the
# same headers were demonstrably present and `getBgW` was demonstrably
# fork-only, so a zero could only mean the script was broken.
#
# **A SWEEP FOR DIVERGENCE THAT CANNOT DETECT A KNOWN DIVERGENCE IS NOT
# EVIDENCE OF CONVERGENCE.** So this refuses to report at all unless it first
# re-finds the seed case. --selftest asserts it; a normal run checks it too and
# exits nonzero if the control fails.
#
# Usage:
#   fork_api_delta.py [--fork PATH] [--vanilla PATH] [--selftest]
# ============================================================================
import os
import re
import sys

FORK = r"C:\Users\xxxxx\Documents\dusklight"
VANILLA = r"C:\Users\xxxxx\Documents\dusklight-main"

# The known divergence this tool must re-find or shut up: file, name.
CONTROL = ("d/d_stage.h", "getBgW")

# A declaration we care about is a NAME followed by '(' - member functions,
# free functions, macros-as-functions. Deliberately loose: this is a NAME SET
# diff, not a parse, and it is used to SCOPE work rather than to prove absence.
DECL = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")

# Words that match the pattern but are never API.
NOISE = {
    "if", "for", "while", "switch", "return", "sizeof", "defined", "else",
    "do", "case", "and", "or", "not", "static_cast", "reinterpret_cast",
    "const_cast", "dynamic_cast", "alignof", "typeid", "decltype", "noexcept",
    "explicit", "operator", "template", "typename", "public", "private",
    "protected", "class", "struct", "union", "enum", "namespace", "using",
}


def names_in(path):
    try:
        with open(path, "rb") as f:
            text = f.read().decode("utf-8", "replace")
    except OSError:
        return None
    # Strip block and line comments so commented-out API is not counted.
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    return set(m.group(1) for m in DECL.finditer(text)) - NOISE


def shared_headers(fork, vanilla, sub="include"):
    """Headers present in BOTH trees, as tree-relative posix paths."""
    out = []
    root = os.path.join(fork, sub)
    for dirpath, _dirs, files in os.walk(root):
        for fn in files:
            if not fn.endswith((".h", ".hpp")):
                continue
            full = os.path.join(dirpath, fn)
            rel = os.path.relpath(full, root).replace(os.sep, "/")
            if os.path.isfile(os.path.join(vanilla, sub, rel)):
                out.append(rel)
    return sorted(out)


def delta(fork, vanilla, sub="include"):
    rows = []
    for rel in shared_headers(fork, vanilla, sub):
        f = names_in(os.path.join(fork, sub, rel))
        v = names_in(os.path.join(vanilla, sub, rel))
        if f is None or v is None:
            continue
        only = f - v
        if only:
            rows.append((rel, sorted(only)))
    return rows


def control_holds(rows):
    want_file, want_name = CONTROL
    for rel, only in rows:
        if rel == want_file and want_name in only:
            return True
    return False


def main():
    fork, vanilla = FORK, VANILLA
    if "--fork" in sys.argv:
        fork = sys.argv[sys.argv.index("--fork") + 1]
    if "--vanilla" in sys.argv:
        vanilla = sys.argv[sys.argv.index("--vanilla") + 1]

    shared = shared_headers(fork, vanilla)
    rows = delta(fork, vanilla)

    ok = control_holds(rows)
    print("CONTROL  %s::%s fork-only ... %s"
          % (CONTROL[0], CONTROL[1], "FOUND (sweep is live)" if ok else
             "** NOT FOUND - SWEEP IS BROKEN **"))
    if not ok:
        print("REFUSING TO REPORT. A sweep for divergence that cannot detect a")
        print("KNOWN divergence is not evidence of convergence.")
        return 2

    total = sum(len(o) for _rel, o in rows)
    print("shared headers compared : %d" % len(shared))
    print("headers with fork-only  : %d" % len(rows))
    print("fork-only names (total) : %d" % total)
    print()
    if "--selftest" in sys.argv:
        return 0
    for rel, only in rows:
        print("%s  (%d)" % (rel, len(only)))
        for n in only:
            print("    %s" % n)
    return 0


if __name__ == "__main__":
    sys.exit(main())
