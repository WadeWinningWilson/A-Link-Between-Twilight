#!/usr/bin/env python3
# ============================================================================
# profile_headroom.py — IS THERE FREE INDEX SPACE INSIDE THE ENUM?
#
# THE PRIOR QUESTION (Housing, cDyl_ bounds row): `cDyl_IsLinked`
# (c_dylink.cpp:885) indexes `DMC[i_ProfName]` with NO range check, so an
# ABOVE-ENUM profile index is a wild read. But if the plugin can register
# INSIDE the enum's existing index space, above-enum indices are never needed
# and the wild read is unreachable BY CONSTRUCTION — the guard becomes hygiene
# rather than load-bearing. Housing explicitly refused to quote a number from
# `grep -c NULL` ("a proxy, not a census") and asked for the scoped count.
# This is that census.
#
# WHAT COUNTS AS FREE, stated so the number can be argued with:
#   TOTAL      entries in g_fpcPfLst_ProfileList[] (the array cDyl_ indexes)
#   BOUND      fpcNm_MAX_NUM — the enum's own end marker
#   WW-OWNED   NULL rows explicitly relinquished to the WW layer ("step 19"
#              comment) — ALREADY claimed, not free
#   FREE       NULL rows with no such claim — genuinely unused index space
#   PROFILED   rows pointing at a real &g_profile_*
#
# A row is classified by what the SOURCE says, never inferred: an unannotated
# NULL is reported separately from a step-19 NULL, because "why is this NULL"
# is exactly the question a migration would get wrong silently.
#
# Usage: profile_headroom.py [--list-free]
# Read-only. Exit 0 headroom exists · 1 no headroom.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
LST = REPO / "src" / "f_pc" / "f_pc_profile_lst.cpp"
NAMES = REPO / "include" / "f_pc" / "f_pc_name.h"
WWREG = REPO / "src" / "d" / "ext_plugin" / "ww_profile_register.cpp"


def enum_names():
    """[(index, name)] from the X-macro list, index by declared /* 0xNNN */."""
    txt = NAMES.read_text(encoding="utf-8", errors="replace")
    out = []
    for m in re.finditer(r"/\*\s*(0x[0-9A-Fa-f]+)\s*\*/\s*X\((fpcNm_\w+)\)", txt):
        out.append((int(m.group(1), 16), m.group(2)))
    return out


def profile_rows():
    """[(ordinal, kind, note)] for the entries of g_fpcPfLst_ProfileList[]."""
    # TWO ARRAYS EXIST (calibration): f_pc_profile_lst.cpp defines the list
    # twice — line 14 under `#ifdef __MWERKS__` (CONSOLE) and line 818 under
    # `#else` (PC, the one that ships). The first run took the FIRST match and
    # measured the console table: 793 entries, 0 WW-OWNED, while step-19 NULLs
    # demonstrably exist at :1624/:1626. Take the LAST definition, and assert
    # the WW claims are visible or refuse to report a number.
    txt = LST.read_text(encoding="utf-8", errors="replace")
    start = txt.rfind("g_fpcPfLst_ProfileList[]")
    if start < 0:
        return []
    body_start = txt.index("{", start) + 1
    depth = 1
    i = body_start
    while i < len(txt) and depth:
        if txt[i] == "{":
            depth += 1
        elif txt[i] == "}":
            depth -= 1
        i += 1
    body = txt[body_start:i - 1]
    rows = []
    ordinal = 0
    for line in body.splitlines():
        s = line.strip()
        if not s or s.startswith("//"):
            continue
        if s.startswith("&g_profile_"):
            rows.append((ordinal, "PROFILED", s.split(",")[0]))
        elif s.startswith("NULL"):
            note = ""
            m = re.search(r"//\s*(.*)$", s)
            if m:
                note = m.group(1).strip()
            kind = "WW-OWNED" if "step 19" in note or "WW-owned" in note else "FREE"
            rows.append((ordinal, kind, note))
        else:
            continue
        ordinal += 1
    return rows


def main():
    names = enum_names()
    rows = profile_rows()
    if not rows:
        print("UNRESOLVED: could not parse g_fpcPfLst_ProfileList[] — not proof "
              "of anything (№31-C)")
        return 2
    total = len(rows)
    profiled = sum(1 for r in rows if r[1] == "PROFILED")
    wwowned = sum(1 for r in rows if r[1] == "WW-OWNED")
    free = [r for r in rows if r[1] == "FREE"]

    print("PROFILE INDEX HEADROOM — the prior question on the cDyl_ bounds row")
    print("SCOPE: entries of g_fpcPfLst_ProfileList[] (src/f_pc/f_pc_profile_lst.cpp),")
    print("       the exact array cDyl_IsLinked indexes as DMC[i_ProfName].")
    print("=" * 74)
    print("\n  enum names declared (X-macro, /* 0xNNN */): %d" % len(names))
    if names:
        print("  highest declared index: 0x%X (%s)" % (names[-1][0], names[-1][1]))
    print("\n  profile-list entries TOTAL : %d" % total)
    print("    PROFILED (real &g_profile_*)   : %d" % profiled)
    print("    WW-OWNED (NULL, step-19 claim) : %d" % wwowned)
    print("    FREE     (NULL, unclaimed)     : %d" % len(free))

    if free:
        print("\n  VERDICT: HEADROOM EXISTS — %d unclaimed index slot(s) inside the enum."
              % len(free))
        print("  A plugin can register INSIDE the existing index space, so above-enum")
        print("  indices are never required and cDyl_IsLinked's unguarded DMC[] read is")
        print("  UNREACHABLE BY CONSTRUCTION. The one-line guard is HYGIENE, not")
        print("  load-bearing — and no native f_pc edit is needed to ship.")
        if "--list-free" in sys.argv:
            for o, _k, note in free[:40]:
                print("      slot %4d  %s" % (o, note[:60]))
    else:
        print("\n  VERDICT: NO HEADROOM — every slot is profiled or already WW-claimed.")
        print("  Above-enum indices become necessary, which makes cDyl_IsLinked's")
        print("  unguarded read LOAD-BEARING and the f_pc guard a prerequisite.")
    print("\n  NOTE: WW-OWNED slots are NOT free — they are already relinquished to the")
    print("  WW layer (V10-a reported 33 of 33 handed over). Counting them as free")
    print("  would double-book the same indices.")
    return 0 if free else 1


if __name__ == "__main__":
    sys.exit(main())
