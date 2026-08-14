#!/usr/bin/env python3
# ============================================================================
# coverage_join.py — V10-b: the CEILING METRIC (CALLS row, tale §915).
#
# THE QUESTION IT ANSWERS, in one number pair: **is the bottleneck US or
# UPSTREAM?** "30% against a 75% ceiling means the bottleneck is us. 30%
# against a 34% ceiling means the bottleneck is tww and more lanes won't
# help." Until now that caveat could only be asserted; V10-a's emission makes
# it measurable.
#
# THE THREE LEVELS (V10-a's contract, joined here):
#   DECLARED   a lane says it is ported            (soft; not used as truth)
#   LINKED     registered with the plugin          ([V10a] ev=register rows)
#   EXERCISED  actually instantiated in real play  ([V10a] ev=resolve)
#   + UNRESOLVED = the self-generating worklist (absent this run = healthy)
#
# WEIGHTING RULE (adopted from the briefing, non-negotiable): weight by
# PLACEMENTS, never by actor count. One actor with 400 placements outranks
# twelve with three, and actor-count coverage flatters itself.
#
# CEILING = share of placements whose donor TU is portable TODAY (MATCHED or
# EQUIVALENT per the paren-aware decomp parser). That is the number no
# instrument of ours can raise — only upstream decomp progress moves it.
#
# No human types a percentage: this reads the log, the disc, and configure.py.
# Usage: coverage_join.py [logfile ...]   (default: newest log with [V10a])
# Read-only except the artifact. Exit 0.
# ============================================================================
import glob
import json
import os
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(HERE))
LOGDIR = Path(os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight")) / "logs"
RE_V10A = re.compile(r"\[V10a\]\s*(\{.*\})\s*$")


def read_feed(paths):
    """(registers, exercised_names, unresolved_names, files_seen)."""
    regs, exercised, unresolved, seen = [], set(), set(), []
    for p in paths:
        hit = False
        for ln in open(p, encoding="utf-8", errors="replace"):
            m = RE_V10A.search(ln.rstrip())
            if not m:
                continue
            try:
                ev = json.loads(m.group(1))
            except ValueError:
                continue
            hit = True
            kind = ev.get("ev")
            if kind == "register":
                regs.append(ev)
            elif kind == "resolve" and ev.get("name"):
                exercised.add(ev["name"])
            elif kind == "unresolved" and ev.get("name"):
                unresolved.add(ev["name"])
        if hit:
            seen.append(Path(p).name)
    return regs, exercised, unresolved, seen


# DECLARED runtime-name aliases (never inferred): a ported system whose
# runtime profile name does not spell its donor TU. EXT_VEG is the vegetation
# system standing in for the donor grass TU — port_planner carries the same
# alias in PORTED_ALIASES, and without this row its placements (the heaviest
# on Outset) fell out of the numerator while still being reported EXERCISED.
RUNTIME_ALIASES = {"EXT_VEG": "d_a_grass"}


def stem_candidates(profile_name):
    if profile_name in RUNTIME_ALIASES:
        return [RUNTIME_ALIASES[profile_name]]
    """Profile/runtime name -> donor TU stem candidates. Mechanical, and any
    miss is reported UNMAPPED rather than guessed (№31-C)."""
    n = profile_name.strip()
    base = re.sub(r"^(WW_|EXT_)", "", n).lower()
    return ["d_a_" + base, "d_a_" + base.replace("_", ""), "d_a_obj_" + base,
            "d_a_npc_" + base, "d_a_tag_" + base]


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    cumulative = "--all" in sys.argv
    if args:
        paths = args
    else:
        # SCOPE FIX (Housing's §920 correction + my own defect): `resolve` is
        # deduped ONE-PER-ROW-PER-RUN, so aggregating N logs silently unions
        # N runs' coverage while this tool's own artifact claims a PER-RUN
        # reading. Run 012809 emitted 2 resolve events; the first build of
        # this tool reported 13 by scanning 20 logs. Default is now the NEWEST
        # RUN CARRYING [V10a]; --all restores the cumulative union, LABELLED.
        cands = sorted(glob.glob(str(LOGDIR / "*.log")), key=os.path.getmtime,
                       reverse=True)
        if cumulative:
            paths = cands[:20]
        else:
            paths = []
            for c in cands[:40]:
                try:
                    if any("[V10a]" in ln for ln in
                           open(c, encoding="utf-8", errors="replace")):
                        paths = [c]
                        break
                except OSError:
                    continue
    regs, exercised, unresolved, seen = read_feed(paths)
    if not seen:
        print("UNRESOLVED-LOOKUP: no [V10a] lines in the newest logs — not proof "
              "of zero coverage (№31-C). Run the game on a V10-a build first.")
        return 2

    import port_planner as pp
    import decomp_status
    table = decomp_status.parse_configure()
    names = pp.objname_table()
    counts = pp.outset_placements()

    # placements -> donor stem (the planner's own identity path)
    by_stem = {}
    unknown_pl = 0
    for nm, cnt in counts.items():
        stem = names.get(nm) or names.get(nm.lower())
        if stem is None:
            unknown_pl += cnt
            continue
        by_stem[stem] = by_stem.get(stem, 0) + cnt
    total = sum(by_stem.values())

    # EXERCISED: map each runtime name to a stem present in the placement set
    ex_stems, unmapped = set(), []
    for n in sorted(exercised):
        hit = next((s for s in stem_candidates(n) if s in by_stem), None)
        if hit:
            ex_stems.add(hit)
        else:
            unmapped.append(n)
    ex_pl = sum(by_stem[s] for s in ex_stems)

    # CEILING: placements whose donor TU is portable TODAY
    ceil_stems = {s for s in by_stem
                  if table.get("d/actor/%s.cpp" % s) in ("MATCHED", "EQUIVALENT")}
    ceil_pl = sum(by_stem[s] for s in ceil_stems)

    pct = lambda a, b: (100.0 * a / b) if b else 0.0
    L = []
    L.append("# COVERAGE vs CEILING (coverage_join.py, V10-b — GENERATED)")
    L.append("")
    L.append("Feed: %s  — mode: %s" % (", ".join(seen[:4]), "CUMULATIVE (--all, union of runs)" if cumulative else "SINGLE RUN (resolve is deduped per-run)"))
    L.append("Weighting: PLACEMENTS (never actor count). Area: the planner's "
             "Outset stage set.")
    L.append("")
    L.append("| metric | placements | share |")
    L.append("|---|---|---|")
    L.append("| EXERCISED (ran in real play) | %d | **%.1f%%** |" % (ex_pl, pct(ex_pl, total)))
    L.append("| CEILING (portable today) | %d | **%.1f%%** |" % (ceil_pl, pct(ceil_pl, total)))
    L.append("| total placed | %d | 100%% |" % total)
    L.append("")
    headroom = pct(ceil_pl, total) - pct(ex_pl, total)
    if ceil_pl and ex_pl / max(1, ceil_pl) < 0.9:
        verdict = ("**THE BOTTLENECK IS US** — %.1f points of headroom sit inside "
                   "what is already portable. More porting effort converts directly." % headroom)
    else:
        verdict = ("**THE BOTTLENECK IS UPSTREAM** — exercised coverage is at or near "
                   "the ceiling; more lanes cannot raise it, only decomp progress can.")
    L.append(verdict)
    L.append("")
    L.append("## Levels (V10-a contract)")
    L.append("")
    for r in regs[:4]:
        L.append("- LINKED: layer=%s rows=%s pending=%s mismatch=%s handed_over(=relinquished, the GOOD state per §920)=%s"
                 % (r.get("layer"), r.get("rows"), r.get("pending"),
                    r.get("mismatch"), r.get("relinquished")))
    L.append("- EXERCISED names: %d (%s)" % (len(exercised), ", ".join(sorted(exercised)[:10])))
    L.append("- UNRESOLVED worklist: %s"
             % (", ".join(sorted(unresolved)) if unresolved
                else "EMPTY this run — the healthy case (empty, not absent)"))
    if unmapped:
        L.append("- UNMAPPED runtime names (reported, never guessed): %s"
                 % ", ".join(unmapped[:10]))
    if unknown_pl:
        L.append("- placements with no l_objectName row: %d (excluded from both "
                 "numerators AND the total)" % unknown_pl)
    L.append("")
    L.append("## Honest scope")
    L.append("")
    L.append("- EXERCISED counts what ONE run touched; a run that never entered a "
             "house cannot exercise its props. Coverage rises with playtest breadth, "
             "so read it per-run, not as a project total.")
    L.append("- CEILING moves only with upstream decomp. That is the number this "
             "whole metric exists to expose.")
    out = REPO / "docs" / "state" / "ww-staging" / "coverage-ceiling.md"
    out.write_text("\r\n".join(L) + "\r\n", encoding="utf-8", newline="")
    print("\n".join(L))
    print("-> %s" % out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
