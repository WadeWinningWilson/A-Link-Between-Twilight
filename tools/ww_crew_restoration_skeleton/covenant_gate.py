#!/usr/bin/env python3
"""§490 roadmap step 4 — the covenant gate, WITH ITS SCOPE DECLARED.

THE DEFECT THIS FIXES
---------------------
The M6 greplist scans the shipped exe for WW IDENTITY LITERALS. It cannot see
ported donor LOGIC or donor DATA CONSTANTS, because compiled code carries no
such strings. For the whole §440-§486 campaign it was reported as
`gate [('Ivan', 1)] PASS` — a true statement about strings that reads as a
statement about lineage. Two different claims; one instrument.

So this gate reports BOTH axes on every run, and `code lineage` is NEVER
reported CLEAN by this tool. It stays UNKNOWN until the census (roadmap step 8)
measures it. A gate that cannot see a class of breach must say so in its own
output — the same rule this project applied to its probes (§473/§480): a clean
log must mean "never fired", not "never looked".

Ruling: NEVER-PUSH-STRIP-SET.md, "RULING — THE M6 GATE'S SCOPE".

USAGE
  python covenant_gate.py [--exe PATH] [--strict]
    --strict  also fail when the WW-layer manifest has drifted (roadmap step 3),
              because a gate run against a stale roster says nothing about the
              files the roster forgot.
EXIT
  0 = identity literals CLEAN
  1 = BREACH (or, with --strict, manifest drift)
  2 = could not run (missing exe / greplist)
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
GREPLIST = Path(__file__).resolve().parent / "greplist.txt"
DEFAULT_EXE = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.exe"
MANIFEST_TOOL = Path(__file__).resolve().parent / "ww_layer_manifest.py"

# The Ivan hit is user-protected: it is the §389 SUBSTITUTED log literal, kept
# deliberately so "Ivans stick out if verifiers ever fall flat". Never neutralise.
EXPECTED = {"Ivan": 1}


def patterns() -> list[str]:
    """Non-comment, non-blank greplist entries.

    The file MUST stay LF-ended: with CRLF a naive reader greps for "Ivan\\r" and
    every pattern returns zero — a FALSE CLEAN that hides real leaks (observed
    2026-07-19). Stripping per-line defends against that regardless.
    """
    out: list[str] = []
    for line in GREPLIST.read_text(encoding="utf-8", errors="replace").splitlines():
        s = line.strip()
        if s and not s.startswith("#"):
            out.append(s)
    return out


def scan(exe: Path) -> dict[str, int]:
    blob = exe.read_bytes()
    hits: dict[str, int] = {}
    for pat in patterns():
        n = blob.count(pat.encode("ascii", "ignore")) + blob.count(pat.encode("utf-16-le"))
        if n:
            hits[pat] = n
    return hits


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    ap.add_argument("--strict", action="store_true")
    args = ap.parse_args()

    if not args.exe.exists():
        print(f"COVENANT GATE — cannot run: exe not found at {args.exe}")
        return 2
    if not GREPLIST.exists():
        print(f"COVENANT GATE — cannot run: greplist not found at {GREPLIST}")
        return 2

    hits = scan(args.exe)
    breach = {k: v for k, v in hits.items() if EXPECTED.get(k) != v}

    literals = "CLEAN" if not breach else f"BREACH {breach}"
    print("COVENANT GATE — identity literals: " + literals)
    print("                code lineage:      UNKNOWN (not measured by this gate)")
    print(f"  exe: {args.exe}")
    print(f"  hits: {hits or '{}'}   expected: {EXPECTED}")
    print("  NOTE: 'identity literals CLEAN' means no WW name became a string in")
    print("        the binary. It says NOTHING about ported donor logic or donor")
    print("        data constants — those are invisible to a string scan and are")
    print("        measured by the census (roadmap step 8), not here.")

    rc = 1 if breach else 0

    if args.strict and MANIFEST_TOOL.exists():
        proc = subprocess.run([sys.executable, str(MANIFEST_TOOL), "--check"],
                              capture_output=True, text=True)
        drifted = proc.returncode != 0
        print("                WW-layer manifest: "
              + ("DRIFTED — regenerate with --emit-tier1" if drifted else "in sync"))
        if drifted:
            rc = 1

    return rc


if __name__ == "__main__":
    sys.exit(main())
