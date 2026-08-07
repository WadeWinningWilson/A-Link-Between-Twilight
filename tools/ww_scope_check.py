#!/usr/bin/env python3
"""№285 WW SCOPE CHECK — "a comment is not a scope."

WHY THIS EXISTS
---------------
Audit №284 found six WW-motivated edits sitting in SHARED TP engine files with
no runtime scope. Two of them were crash/leak classes on mainline TP
(getMyActIdx -1 into an unchecked member-function array; a room claim nothing
ever clears). The pattern that produced them is mechanical, so the guard is too:

    №93 and №62 sat TWELVE LINES APART in the same function. Both READ as
    WW-specific. Only №93 actually wrote the check.

THE LANE LAW IT ENFORCES
------------------------
A WW-motivated change inside a shared TP translation unit must be either
  (a) runtime-scoped to WW context   -> dExtWwSave_isWwHostStage(...) (or an
      equivalent gate: dKyWw_isSkyHost, a WW-only actor's own `this`), or
  (b) logging/probe only (no behavior change off the WW path).
Anything else alters vanilla Twilight Princess for every player, forever.

WHAT IT DOES
------------
Scans shared TP files (src/d, src/f_op, src/f_pc, libs/JSystem) for TARGET_PC
blocks carrying a WW lane tag (§NNN / №NNN) and reports any whose body changes
behavior without a gate in scope. Heuristic by design: it flags candidates for a
human, it does not fail a build. Files under an explicitly WW-owned path
(d_ext_*, *_ww_*, d_a_ext_*) are skipped -- those ARE the WW lane.

USAGE
    python tools/ww_scope_check.py            # report
    python tools/ww_scope_check.py --strict   # exit 1 if any finding (CI use)
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

# Windows consoles default to cp1252; the ledger sigils are non-ASCII.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).resolve().parent.parent
SCAN_DIRS = ["src/d", "src/f_op", "src/f_pc", "src/m_Do", "libs/JSystem/src"]

# Paths that ARE the WW lane -- gating is not required inside them.
WW_OWNED = re.compile(r"(d_ext_|_ww_|d_a_ext_|ww_jpa|/ext_seq/|d_kankyo_ww)", re.I)

# A WW lane tag: §NNN or №NNN (the two ledger sigils this project uses).
LANE_TAG = re.compile(r"[§№]\s*\d{2,3}")

# Gates that constitute a real runtime scope.
GATE = re.compile(
    r"(isWwHostStage|dKyWw_isSkyHost|dKyWw_domeActorsLive|DUSK_WW_[A-Z0-9_]+|"
    r"isRoomLaneRoom|dExtNpcMount_lookup|wwHost)"
)

# Lines that cannot change behavior off the WW path.
INERT = re.compile(
    r"^\s*(//|/\*|\*|\}|\{|#endif|#else|#if|$)|"
    r"DuskLog\.|OS_REPORT|OSReport|static\s+int\s+s_\w+\s*=|snprintf\("
)


def scan_file(path: Path) -> list[tuple[int, str]]:
    findings: list[tuple[int, str]] = []
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return findings

    depth = 0            # TARGET_PC nesting
    block_start = 0
    block: list[str] = []
    for i, line in enumerate(lines, 1):
        if re.match(r"\s*#if\s+TARGET_PC", line):
            if depth == 0:
                block_start, block = i, []
            depth += 1
            continue
        if depth and re.match(r"\s*#endif", line):
            depth -= 1
            if depth == 0:
                text = "\n".join(block)
                if LANE_TAG.search(text) and not GATE.search(text):
                    if any(not INERT.search(b) for b in block):
                        tag = LANE_TAG.search(text)
                        findings.append((block_start, tag.group(0) if tag else "?"))
            continue
        if depth:
            block.append(line)
    return findings


def main() -> int:
    strict = "--strict" in sys.argv
    total = 0
    for d in SCAN_DIRS:
        base = ROOT / d
        if not base.exists():
            continue
        for path in sorted(base.rglob("*.cpp")) + sorted(base.rglob("*.inc")):
            rel = path.relative_to(ROOT).as_posix()
            if WW_OWNED.search(rel):
                continue
            for line_no, tag in scan_file(path):
                print(f"{rel}:{line_no}: WW-tagged block ({tag}) in a SHARED file with no scope gate")
                total += 1

    print()
    if total:
        print(f"{total} candidate(s). Each must be WW-gated (isWwHostStage/…) or logging-only.")
        print("Audit №284 precedent: №282 №283 §295 №85 are the sanctioned fix shape.")
    else:
        print("clean — every WW-tagged block in shared files is scoped or inert.")
    return 1 if (strict and total) else 0


if __name__ == "__main__":
    raise SystemExit(main())
