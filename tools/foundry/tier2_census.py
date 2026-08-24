#!/usr/bin/env python3
# ============================================================================
# tier2_census.py — CAN THE RECEIVER LAYER LEAVE THE EXE? The measurement that
# decides whether the user's stated product is fully reachable (tale §939).
#
# THE RULING IT SERVES: the plugin IS the product; migration is committed;
# feasibility runs first. A plugin hooks FUNCTIONS. So every WW seam sitting
# inside a NATIVE TP file falls into one of three classes, and only the third
# is a genuine blocker:
#
#   A  ADDED FUNCTION      a whole WW function living in a TP file. Moves to
#                          the plugin as-is, or is hooked trivially. CHEAP.
#   B  IN-FUNCTION EDIT    a WW branch inside a vanilla TP function. Migrates
#                          by hooking that whole function and re-implementing
#                          it plugin-side. COSTLY BUT MECHANICAL — the cost is
#                          the size of the host function, not a new mechanism.
#   C  STATIC DATA TABLE   a WW row inside a file-scope table (l_objectName is
#                          the known case). A plugin CANNOT extend a static
#                          array. NO HOOK EQUIVALENT — needs a registration
#                          seam, which is a receiver change, which is exactly
#                          what the product forbids. **THE CLASS-C COUNT IS
#                          THE ANSWER TO "IS THIS REACHABLE".**
#
# HONEST LIMITS, stated because this number will be quoted: classification is
# by enclosing construct, mechanically. A file with no WW marker is invisible
# (an unmarked WW edit cannot be found by any marker scan). Native-TP-ness is
# by filename, the same scope trap layer_census exists to expose — so the
# scope is PRINTED, not assumed.
#
# Usage: tier2_census.py [--class C] [--file <path>]
# Read-only. Exit 0.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]

# A line is WW-lineage if it names WW machinery. §N alone is NOT enough —
# section refs decorate plenty of vanilla fixes.
# WORD-BOUNDED (first-run correction): the loose forms "ww_" and "WW_"
# matched VANILLA TP text — dRes_ID_ALANM_BCK_DAMSWW_e (a TP animation row) and
# "ZI_S_ww_blur_a.jpa" (a TP particle filename) both scored as WW seams and
# inflated the class-C count, the one number this census exists to produce.
# A word boundary prevents both: no boundary exists between S and WW_,
# nor between _ and ww_, so only leading occurrences match.
# (Repair note: the first attempt at this fix went through a shell and
# deposited literal 0x08 bytes where the anchors belonged — invisible to
# every reader, and it silently BROKE alternatives so the counts moved
# for the wrong reason. Code edits go through Write-tool files only.)
# ============================================================================
# REPAIRED 2026-08-16 (roadmap A2's open half, landed on the user's go).
# Two blindnesses, both measured before touching anything:
#
# 1. `\bdWw[A-Z]` was in NO alternative, so `dWwProfileRegister_` calls in
#    native f_pc_profile_lst.cpp (21 lines) were invisible — the port-track
#    undercount. The `\b` on `ww_`/`WW_` stays: it is a DELIBERATE guard
#    (vanilla TP carries DAMSWW_e and ZI_S_ww_blur_a.jpa, the first-run FPs).
# 2. The skins API is EXCLUDED BY RULING, not by accident: the user ruled the
#    skins track ALBW (08-16), so in-scope `dWwItemmdl_`/`WwHeldSkinMode`/
#    `s_albwWw` edits are ALBW work in native files — NOT WW debt. The
#    exclusion is explicit and cited so it reads as a decision, not a gap.
#    (npc_mount's five port-track dWwItemmdl_ callers live in an ext_-named
#    file, outside this scan's scope entirely — the both-ways trap noted.)
# ============================================================================
RE_WW = re.compile(r"dExtWw|dKyWw|\bWwRoom|\bWwStage|\bww_|\bWW_|dExtNpcMount|"
                   r"fpcNm_WW|d_a_ww|dExtWwSave|wwRoom_|dExtWwCam|"
                   r"\bdWw[A-Z]\w*")
RE_SKINS_ALBW = re.compile(r"dWwItemmdl_|WwHeldSkinMode|s_albwWw|"
                           r"checkWwBowSkin")
RE_FUNC = re.compile(r"^[\w:*&<>\s]+?([\w:~]+)\s*\([^;{]*\)\s*(?:const\s*)?\{")
# LAZY type-class (2026-08-16): the greedy form ate "int l_objectNam" and
# captured "e" as the host — every C-class row in tonight's real output read
# "table e". Cosmetic, but a host name is a CITATION and citations must not
# be one letter long. Caught by the fixture control on its first run.
RE_TABLE = re.compile(r"^\s*(?:static\s+)?[\w:*&<>\s]+?\**\s*(\w+)\s*"
                      r"\[[^\]]*\]\s*=\s*\{")
WW_FN = re.compile(r"dExtWw|wwRoom|ww[A-Z]|WwRoom|dKyWw|daWwTsubo|dExtWwCam")


UPSTREAM_EXE = ("%USERPROFILE%/Documents/dusklight-main/build/"
                "windows-msvc-relwithdebinfo/dusklight.exe")
_UP = None


def upstream_symbols():
    """Names present in the build a USER runs, or None if unavailable.

    THE B-CLASS DEFINITION FIX (tale §961): 'vanilla TP file' was decided by
    FILENAME, and upstream's d_a_npc_henna0.cpp is an 820-byte STUB while ours
    is a 4,495-byte implementation — so functions WE authored scored as vanilla
    hosts needing hooks. A host is only genuinely class B if it EXISTS in the
    build a user runs; if it does not, the code is ours and migrates WITH the
    plugin. None (not empty) when the image is unavailable, so the caller can
    report UNKNOWN rather than silently reclassifying everything (№31-C).
    """
    global _UP
    if _UP is None:
        try:
            sys.path.insert(0, str(HERE))
            import symbol_manifest as SM
            occ = SM.occurrences(UPSTREAM_EXE)
            _UP = set(occ) if occ else False
        except Exception:
            _UP = False
    return _UP or None


def host_is_vanilla(fn, up):
    """Does THIS host exist upstream — asked the way a hook would ask it.

    QUALIFIED MATCHING (tale §963 correction): the first version matched the
    BARE name, so `daKnob00_c::CreateHeap` counted as a vanilla host because
    upstream has 406 other classes' `CreateHeap` — while the qualified name is
    absent there entirely (d_a_knob00.cpp is a WW PORT under a donor-style
    filename, the "WW port or TP namesake" ambiguity port_planner flags). That
    inflated the genuine-host population from 73 to 105 and with it the priced
    hook workload. A class-qualified host must match EXACTLY; only an
    unqualified name falls back to a bare match.
    """
    if up is None:
        return None                     # UNKNOWN — never assume either way
    if "::" in fn:
        return fn in up
    return fn in up


def classify_file(path):
    """[(line_no, cls, host)] for every WW-lineage line in a native TP file."""
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    out = []
    depth = 0
    host = "(file scope)"
    host_kind = "NONE"
    for i, ln in enumerate(lines, 1):
        stripped = ln.strip()
        if depth == 0:
            m = RE_TABLE.match(ln)
            if m:
                host, host_kind = m.group(1), "TABLE"
            else:
                # ============================================================
                # MULTI-LINE DEFINITIONS (2026-08-16, the sig_diff lesson
                # arriving here): RE_FUNC required the parameter list AND the
                # `{` on ONE line, so any host whose signature wraps lost its
                # identity and its WW sites filed as A ("moves cheaply")
                # instead of B ("hook + reimplement"). d_demo.cpp's four port
                # hunks sit inside such hosts — History's per-hunk dig said B,
                # this scan said A, and the dig was right. Join forward until
                # the parens balance (bounded) before asking who the host is.
                # ============================================================
                cand = ln.rstrip()
                j = i - 1
                while (cand.count("(") > cand.count(")")
                       and j + 1 < len(lines) and j - (i - 1) < 6):
                    j += 1
                    cand = cand + " " + lines[j].strip()
                m = RE_FUNC.match(cand)
                if m:
                    host = m.group(1)
                    host_kind = "WWFUNC" if WW_FN.search(host) else "TPFUNC"
        # skins API excluded BY USER RULING (ALBW track), never by accident —
        # see the RE_SKINS_ALBW note above
        if (RE_WW.search(stripped) and not stripped.startswith("//")
                and not RE_SKINS_ALBW.search(stripped)):
            if host_kind == "TABLE":
                cls = "C"
            elif host_kind == "WWFUNC":
                cls = "A"
            elif host_kind == "TPFUNC":
                cls = "B"
            else:
                cls = "A"      # file-scope decl/include: moves with the code
            out.append((i, cls, host))
        depth += ln.count("{") - ln.count("}")
        if depth <= 0:
            depth = 0
            if host_kind != "NONE" and ("}" in ln):
                host, host_kind = "(file scope)", "NONE"
    return out


def main():
    want = None
    if "--class" in sys.argv:
        i = sys.argv.index("--class")
        if i + 1 < len(sys.argv):
            want = sys.argv[i + 1].upper()
    one = None
    if "--file" in sys.argv:
        i = sys.argv.index("--file")
        if i + 1 < len(sys.argv):
            one = sys.argv[i + 1]

    files = []
    for pat in ("src/d/*.cpp", "src/d/actor/*.cpp", "src/f_op/*.cpp",
                "src/f_pc/*.cpp", "src/d/**/*.cpp"):
        for p in REPO.glob(pat):
            n = p.name
            if any(k in n for k in ("_ww_", "ww_", "_ext_", "ext_", "albw")):
                continue            # WW-OWNED file: not Tier-2, it moves whole
            files.append(p)
    files = sorted(set(files))
    if one:
        files = [p for p in files if one in p.as_posix()]

    up = upstream_symbols()
    totals = {"A": 0, "B": 0, "C": 0}
    per_file = {}
    c_sites, b_hosts = [], {}
    b_vanilla, b_ours, b_unknown = {}, {}, {}
    for p in files:
        rows = classify_file(p)
        if not rows:
            continue
        rel = p.relative_to(REPO).as_posix()
        counts = {"A": 0, "B": 0, "C": 0}
        for line, cls, host in rows:
            counts[cls] += 1
            totals[cls] += 1
            if cls == "C":
                c_sites.append((rel, line, host))
            elif cls == "B":
                b_hosts.setdefault((rel, host), 0)
                b_hosts[(rel, host)] += 1
                v = host_is_vanilla(host, up)
                bucket = (b_unknown if v is None else
                          b_vanilla if v else b_ours)
                bucket.setdefault((rel, host), 0)
                bucket[(rel, host)] += 1
        per_file[rel] = counts

    print("TIER-2 CENSUS — can the WW receiver layer leave the exe? (tale §939)")
    print("SCOPE: native TP .cpp under src/d, src/f_op, src/f_pc — files whose NAME")
    print("       carries no ww_/ext_/albw marker (WW-OWNED files move whole and are")
    print("       NOT Tier-2). Unmarked WW edits are invisible to any marker scan.")
    print("=" * 76)
    print("\n  A  added WW function in a TP file  : %4d site(s)  -> moves/hooks CHEAPLY"
          % totals["A"])
    print("  B  WW edit inside a vanilla function: %4d site(s)  -> hook + reimplement host"
          % totals["B"])
    print("  C  WW row inside a STATIC TABLE     : %4d site(s)  -> NO HOOK EQUIVALENT"
          % totals["C"])

    print("\n  B-CLASS SPLIT by whether the host EXISTS IN THE BUILD A USER RUNS")
    print("  (tale §961: filename was the wrong test — upstream stubs made our own")
    print("   functions look like vanilla hosts):")
    if up is None:
        print("    UNKNOWN — upstream image unavailable; B cannot be split, and it is")
        print("    NOT safe to assume either way.")
    else:
        vs = sum(b_vanilla.values())
        os_ = sum(b_ours.values())
        print("    GENUINE vanilla hosts -> MUST BE HOOKED : %3d host(s), %3d site(s)"
              % (len(b_vanilla), vs))
        print("    OUR OWN functions     -> move with plugin: %3d host(s), %3d site(s)"
              % (len(b_ours), os_))
        if vs + os_:
            print("    => the real hook workload is %d of %d B sites (%.0f%%)"
                  % (vs, vs + os_, 100.0 * vs / (vs + os_)))
    # --all-hosts added by the INTEGRATOR 2026-08-16 at Foundry's ask (roadmap
    # diffing step 3). The census HELD all 148 host functions and PRINTED 12.
    # Foundry's first reconciliation parsed the printed rows and reported
    # "10 host files, 173 commits-only, 0 tier2-only" — a PARSE ARTIFACT OF THE
    # DISPLAY LIMIT, which they withdrew before anyone built on it. The data was
    # never missing; only the print was. Default output is deliberately
    # unchanged so nothing downstream of the old shape breaks.
    _all_hosts = "--all-hosts" in sys.argv
    _ranked = sorted(b_hosts.items(), key=lambda kv: -kv[1])
    print("\n  B-class host functions (each must be hooked whole), %s:"
          % ("ALL %d by sites" % len(_ranked) if _all_hosts else "top 12 by sites"))
    for (rel, host), n in (_ranked if _all_hosts else _ranked[:12]):
        print("    %-40s %-28s %d" % (rel, host, n))
    print("    ... %d distinct host function(s) total%s"
          % (len(b_hosts), "" if _all_hosts else "  (re-run with --all-hosts to emit them all)"))

    print("\n  C-class sites — THE REACHABILITY QUESTION:")
    if not c_sites:
        print("    NONE FOUND. Every WW seam in a native TP file is A or B, and both")
        print("    have migration paths. Full plugin-ization is MECHANICALLY REACHABLE")
        print("    on this evidence.")
    else:
        # ALWAYS PRINT THE PER-TABLE ROLL-UP, and never truncate it.
        # Added by the INTEGRATOR 2026-08-16. I added `--all-hosts` for the
        # B-class list after a display limit produced a confident wrong
        # reconciliation — and left the C-class list capped at 20 in the tool
        # whose STATED PURPOSE is the C-class count. A2 took C from 15 to 35,
        # so 15 sites were unviewable at exactly the moment the number moved.
        # The reachability verdict depends on WHICH TABLES appear, not how
        # many rows: `l_objectName` is answerable via `dStage_searchName`, and
        # a NEW table would be a genuinely new blocker. A roll-up is bounded
        # by the number of distinct tables (small) rather than sites (not), so
        # it can be printed in full unconditionally with no flag to remember.
        _tables = {}
        for rel, line, host in c_sites:
            _tables.setdefault((rel, host), []).append(line)
        print("    BY TABLE (complete, never truncated) — the reachability answer:")
        for (rel, host), lines in sorted(_tables.items(), key=lambda kv: -len(kv[1])):
            print("      %-38s %-26s %d site(s)  first :%d"
                  % (rel, host, len(lines), min(lines)))
        print("    %d site(s) across %d distinct table(s)"
              % (len(c_sites), len(_tables)))
        _cap = len(c_sites) if "--all-c" in sys.argv else 20
        print("\n    SITES%s:" % ("" if _cap >= len(c_sites)
                                  else " (first 20 — re-run with --all-c for all)"))
        for rel, line, host in c_sites[:_cap]:
            print("    %-40s :%-6d table %s" % (rel, line, host))
        if len(c_sites) > _cap:
            print("    ... +%d more" % (len(c_sites) - _cap))
        print("\n    Each needs a REGISTRATION SEAM (a receiver change) or it cannot")
        print("    migrate. This count is the honest limit on the stated product.")

    if want:
        print("\n  --class %s detail:" % want)
        for rel, counts in sorted(per_file.items(), key=lambda kv: -kv[1].get(want, 0)):
            if counts.get(want):
                print("    %-46s %d" % (rel, counts[want]))
    print("\n  files touched: %d" % len(per_file))
    return 0


if __name__ == "__main__":
    sys.exit(main())
