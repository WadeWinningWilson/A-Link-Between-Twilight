#!/usr/bin/env python3
"""Covenant gate — donor proper nouns in SHIPPING string literals.

WHY THIS IS A TOOL AND NOT A NOTE
---------------------------------
The rules below were already written down, in the ledger's covenant-gate header.
They were then re-violated anyway — by the same author who wrote them — because
a hand-rolled grep with an ad-hoc term list gets re-invented every time somebody
checks, and each re-invention reintroduces the same false positives. A rule that
has to be remembered is not a rule. This is the rule.

WHAT IS ACTUALLY FORBIDDEN
--------------------------
A donor place or character name in a string literal that SHIPS: a warp-menu
label, a dialogue line, an on-screen caption, a resource path pointing outside
the mod folder. Player-facing, describing content the receiver does not ship.

WHAT IS NOT A BREACH — DO NOT RE-REPORT THESE
---------------------------------------------
1. COMMENTS. Stripped by the compiler; they never reach the binary. This tool
   strips them before matching.

2. NOUNS THE TWO GAMES SHARE. `Magic Armor`, `Bow`, `Bomb`, `Boomerang`,
   `Iron Boots`, `Hookshot`, `Bottle`, `Telescope`, `Grappling Hook` are all
   real receiver items. `Link's House` is a real receiver location (Ordon,
   R_SP01). `"magic armor bought malo mart"` is a receiver event flag.

3. DONOR-ORIGIN DATA THE RECEIVER ITSELF SHIPS. The receiver's own `itemmdl`
   archive is 21/21 byte-identical to the donor's (docs/TPHistory.md). Those
   models are vanilla receiver assets. Naming them in a debug/editor viewer
   names the RECEIVER's data, not imported content. This covers the editor's
   item dropdown and its viewer/skins section titles — `Wind Waker Item Viewer`,
   `Wind Waker Skins`, `Skull Hammer`, `Picto Box`, `Tingle Tuner`, `Deku Leaf`,
   `Spoils Bag`, `Bait Bag`, `Delivery Bag`. **These are settled. They are not
   findings. Do not raise them again.**

Usage:  python tools/check_covenant.py          (exit 1 on a real breach)
        python tools/check_covenant.py --all    (also list the allowed ones)
"""
from __future__ import annotations

import io
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Donor place / character names that must never ship in a literal.
FORBIDDEN = [
    "Outset", "Aryll", "Tetra", "Jabun", "Windfall", "Forsaken Fortress",
    "Medli", "Makar", "Komali", "Sturgeon", "Sue-Belle", "Quill", "Beedle",
    "Salvatore", "Korok", "Valoo", "Dragon Roost", "Great Sea", "Hyoi",
    "Din's Pearl", "Nayru's Pearl", "Farore's Pearl",
]

# Settled non-breaches (class 3 above): receiver-shipped donor-origin data.
# Matching is on the LITERAL, so a title containing one of these is allowed.
ALLOWED_SUBSTRINGS = [
    "Wind Waker Item Viewer", "Wind Waker Skins", "Wind Waker gear",
    "Wind Waker", "Deku Leaf", "Skull Hammer", "Picto Box", "Tingle Tuner",
    "Spoils Bag", "Bait Bag", "Delivery Bag", "Grappling Hook", "Magic Armor",
]

LIT = re.compile('"([^"]*)"')
CMT_BLOCK = re.compile("/[*].*?[*]/", re.S)
CMT_LINE = re.compile("//[^\n]*")


def sources() -> list[str]:
    def run(*args):
        return subprocess.run(["git", *args], capture_output=True, text=True,
                              cwd=ROOT).stdout.split()
    files = run("ls-files", "src", "include", "libs")
    files += run("ls-files", "--others", "--exclude-standard", "src", "include")
    return [f for f in files if f.endswith((".c", ".cpp", ".h", ".hpp", ".inc"))]


def main() -> int:
    show_all = "--all" in sys.argv
    breaches, allowed = [], []
    files = sources()
    for rel in files:
        p = ROOT / rel
        try:
            txt = io.open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        txt = CMT_LINE.sub("", CMT_BLOCK.sub("", txt))
        for i, line in enumerate(txt.splitlines(), 1):
            for m in LIT.finditer(line):
                s = m.group(1)
                hit = next((t for t in FORBIDDEN if t.lower() in s.lower()), None)
                if hit is None:
                    continue
                if any(a.lower() in s.lower() for a in ALLOWED_SUBSTRINGS):
                    allowed.append((rel, i, hit, s[:60]))
                    continue
                breaches.append((rel, i, hit, s[:60]))

    print(f"covenant gate — {len(files)} source files (tracked + untracked)")
    if show_all and allowed:
        print(f"\nallowed (receiver-shipped donor-origin data, settled — not findings):")
        for rel, i, t, s in allowed:
            print(f'    {rel}:{i} [{t}] "{s}"')
    if breaches:
        print(f"\nBREACHES ({len(breaches)}):\n")
        for rel, i, t, s in breaches:
            print(f'  {rel}:{i}  [{t}]  "{s}"')
        print("\nA donor place/character name is shipping in a literal. Move it to DATA (R6).")
        return 1
    print(f"CLEAN — 0 breaches ({len(allowed)} allowed literal(s) skipped)")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
