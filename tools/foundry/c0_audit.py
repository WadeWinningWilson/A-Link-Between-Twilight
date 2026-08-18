#!/usr/bin/env python3
# ============================================================================
# c0_audit.py — INVISIBLE CONTROL BYTES IN SOURCE AND DOCS.
#
# ---------------------------------------------------------------------------
# WHY THIS EXISTS. The handoff's §4 records the hazard in prose: unquoted
# heredoc delimiters have injected literal backspaces and NUL bytes into
# source and docs "at least six times — invisible to Read, unmatchable by
# Edit, and they silently moved measurement counts."
#
# The seventh was found on 2026-08-15, by hand, and only because a gate audit
# disagreed with a handoff. A literal 0x08 sat one character past the
# negative-lookahead alternation in `sig_diff.py`'s declaration regex:
#
#     (?!(?:return|if|else|while|for|switch|case|do|new|delete|throw)\x08)
#
# The lookahead therefore asserted "not one of these keywords FOLLOWED BY A
# BACKSPACE". No source line contains a backspace, so **the assertion always
# succeeded and the call-site filter was inert for the tool's entire
# existence** — 52 call sites scored as declarations for one symbol.
#
# **A written warning did not catch six of these. A rule that is not a command
# is a rule that gets broken** (control.py's own thesis). So the warning
# becomes a sweep.
#
# ---------------------------------------------------------------------------
# WHAT COUNTS. Tab (0x09), LF (0x0A) and CR (0x0D) are legitimate. Everything
# else below 0x20, plus DEL (0x7F), is not — not in C++, not in Python, not in
# Markdown. There is no false-positive class here worth tolerating: none of
# these bytes has a legitimate reason to be typed into this repo.
#
# REPORT-ONLY. It never edits. Removing a byte changes meaning (`'\0'` that
# collapsed to a raw NUL wants the ESCAPE restored, not the byte deleted), so
# the repair is always a human's call.
#
# Usage:
#   c0_audit.py                 sweep the default surfaces
#   c0_audit.py <path> [...]    sweep specific files or directories
#
# Exit 0 = clean · 1 = invisible bytes present · 2 = could not run.
# ============================================================================
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]

# The surfaces where an injection has actually landed or would do real damage:
# every instrument, and every doc a lane measures from.
DEFAULT_TARGETS = [
    (HERE, ("*.py",)),
    (REPO / "docs", ("*.md",)),
]

NAMES = {0x00: "NUL", 0x07: "BEL", 0x08: "BS", 0x0B: "VT", 0x0C: "FF",
         0x1A: "SUB", 0x1B: "ESC", 0x7F: "DEL"}


def suspect(b):
    return b < 0x09 or b in (0x0B, 0x0C) or 0x0E <= b <= 0x1F or b == 0x7F


# ============================================================================
# THE BOM, ADDED 2026-08-15 AFTER IT GOT PAST THIS TOOL. A PowerShell
# `Set-Content -Encoding UTF8` prepended EF BB BF to `lane_watch.py` and the
# file stopped parsing — `SyntaxError: invalid non-printable character
# U+FEFF`. This sweep reported the same file CLEAN in the same minute,
# because U+FEFF is not a C0 byte.
#
# **The tool was right and useless: correct about the class it checked, and
# silent about a byte from the identical failure mode** (invisible to Read,
# introduced by a shell ferry, breaks the file). Same shape as a gate that
# measures a narrower surface than the claim it supports.
# ============================================================================
BOM = b"\xef\xbb\xbf"


def scan(path):
    """[(byte, offset, line, context)] for one file."""
    try:
        raw = path.read_bytes()
    except OSError as e:
        print("  UNREADABLE %s (%s)" % (path, e))
        return []
    found = []
    if raw.startswith(BOM):
        found.append((-1, 0, 1, "UTF-8 BOM (EF BB BF) at offset 0 — Python "
                                "refuses to parse this file"))
    for i, b in enumerate(raw):
        if not suspect(b):
            continue
        line = raw.count(b"\n", 0, i) + 1
        ctx = raw[max(0, i - 40):i + 14].decode("utf-8", "replace")
        ctx = ctx.replace("\n", "\\n").replace("\r", "\\r")
        found.append((b, i, line, ctx))
    return found


def collect(args):
    files, seen = [], set()
    if args:
        for a in args:
            p = Path(a)
            if p.is_file():
                if p not in seen:
                    seen.add(p)
                    files.append(p)
            elif p.is_dir():
                for q in sorted(p.rglob("*")):
                    if q.is_file() and q not in seen:
                        seen.add(q)
                        files.append(q)
            else:
                print("  NOT FOUND %s" % a)
        return files
    for root, pats in DEFAULT_TARGETS:
        if not root.exists():
            continue
        for pat in pats:
            for q in sorted(root.rglob(pat)):
                # controls/ holds DELIBERATE injections — sweeping them by
                # default would train the reader to ignore a real hit.
                if not q.is_file() or "controls" in q.parts or q in seen:
                    continue
                seen.add(q)
                files.append(q)
    return files


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    files = collect(args)
    if not files:
        print("no files to sweep")
        return 2

    print("C0 AUDIT — invisible control bytes (tab/LF/CR excluded)")
    print("%d file(s)%s\n" % (len(files),
                              "" if args else "  [default surfaces; "
                                              "controls/ excluded]"))
    hits = 0
    for p in files:
        found = scan(p)
        if not found:
            continue
        hits += 1
        try:
            label = p.relative_to(REPO)
        except ValueError:
            label = p
        print("HIT %s" % label)
        by_byte = {}
        for b, off, line, ctx in found:
            by_byte.setdefault(b, []).append((off, line, ctx))
        for b, rows in sorted(by_byte.items()):
            if b == -1:
                print("    BOM     x%d" % len(rows))
                for _off, _line, ctx in rows:
                    print("        %s" % ctx)
                continue
            print("    0x%02X %-3s x%d" % (b, NAMES.get(b, "?"), len(rows)))
            for off, line, ctx in rows[:5]:
                print("        line %d (offset %d)" % (line, off))
                print("          ...%s..." % ctx)
            if len(rows) > 5:
                print("        (+%d more)" % (len(rows) - 5))

    print()
    if hits:
        print("**%d file(s) carry invisible bytes.** Read is blind to these and"
              % hits)
        print("Edit cannot match them. Repair at the SOURCE of the ferry, and")
        print("check whether an escape collapsed (`'\\0'` -> raw NUL) rather")
        print("than deleting the byte — deletion can silently change meaning.")
    else:
        print("clean — no invisible control bytes in %d file(s)" % len(files))
    return 1 if hits else 0


if __name__ == "__main__":
    sys.exit(main())
