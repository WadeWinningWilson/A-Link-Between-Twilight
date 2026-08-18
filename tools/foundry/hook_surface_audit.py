#!/usr/bin/env python3
# ============================================================================
# hook_surface_audit.py — THE AMBIGUITY RATE OVER THE POPULATION THAT MATTERS.
#
# THE ASK (Housing): §959 measured 5,076 ambiguous names in our image (~1.8%
# of the by-name surface) — a WHOLE-IMAGE rate that may be irrelevant, since
# it is dominated by the empty string (2,332) and RES_U32 (685), data-ish
# names nobody would hook. The number that governs the migration is the rate
# over the RECEIVER-LAYER population Tier-2 must actually hook. If a
# method-name-dense C++ population runs far above 1.8%, "hook the receiver
# layer by name" is a materially different proposition from the three clean
# registration-chain targets. If it comes back near zero, that is a green
# light nobody has earned yet.
#
# THE POPULATION, chosen because it is the one the work depends on: the
# GENUINE VANILLA HOSTS from tier2_census (tale §962) — 105 functions that
# exist in the build a user runs AND carry a WW edit, i.e. exactly the
# functions a migration must hook. Not the whole image, not the WW-owned
# files (those move as source and need no hook at all).
#
# Ambiguity is counted from RAW manifest entries, never the collapsed dict
# (the §960 false-SAFE lesson), and on BOTH images because a name safe in our
# fork and ambiguous upstream is a defect that only appears in a user's game.
#
# Usage: hook_surface_audit.py [--list]
# Read-only. Exit 0 clean · 1 ambiguity or gaps found · 2 could-not-run.
# ============================================================================
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(HERE))
import symbol_manifest as SM
import tier2_census as T2

OURS = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.exe"
UP = Path(T2.UPSTREAM_EXE)


def hook_population():
    """The genuine vanilla hosts a migration must hook: {(file, fn): sites}."""
    up = T2.upstream_symbols()
    if up is None:
        return None, None
    hosts = {}
    for pat in ("src/d/*.cpp", "src/d/actor/*.cpp", "src/f_op/*.cpp",
                "src/f_pc/*.cpp", "src/d/**/*.cpp"):
        for p in REPO.glob(pat):
            if any(k in p.name for k in ("_ww_", "ww_", "_ext_", "ext_", "albw")):
                continue
            for _line, cls, host in T2.classify_file(p):
                if cls != "B":
                    continue
                if T2.host_is_vanilla(host, up):
                    key = (p.relative_to(REPO).as_posix(), host)
                    hosts[key] = hosts.get(key, 0) + 1
    return hosts, up


def occ_for(image):
    o = SM.occurrences(str(image))
    return o if o else None


def verdict_for(fn, occ):
    """(state, detail) from RAW entries: SAFE / AMBIGUOUS / MISSING.

    ASK THE QUESTION A HOOK ACTUALLY ASKS (self-caught before publishing, and
    the fifth instance of this failure today): the first version matched the
    BARE name, so `daBg_c::create` scored AMBIGUOUS against 551 other classes'
    `create` — and reported a 55.2% ambiguity rate. But a hook names the
    QUALIFIED symbol, and `daBg_c::create` occurs EXACTLY ONCE. Measuring the
    bare name answers "could a careless hook go wrong", not "is this host
    hookable". Qualified names are checked exactly; only an UNQUALIFIED host
    name (no `::`) falls back to bare matching, which is the genuinely
    ambiguous case (`useHeapInit`: 198 raw entries).
    """
    if occ is None:
        return "UNKNOWN", "image unreadable"
    if "::" in fn:
        n = occ.get(fn, 0)
        if n == 1:
            return "SAFE", fn
        if n == 0:
            return "MISSING", "no manifest entry for the qualified name"
        return "AMBIGUOUS", "%d raw entries under the QUALIFIED name %s" % (n, fn)
    matches = {k: v for k, v in occ.items() if k == fn}
    if not matches:
        return "MISSING", "no manifest entry"
    total = sum(matches.values())
    if total == 1:
        return "SAFE", fn
    return "AMBIGUOUS", ("%d raw entries under the unqualified name %s — a hook "
                         "cannot pick between them" % (total, fn))


def main():
    hosts, up = hook_population()
    if hosts is None:
        print("UNRESOLVED: upstream image unavailable — the population cannot be")
        print("defined, and guessing it is what tale §961 corrected. (№31-C)")
        return 2
    ours, upo = occ_for(OURS), occ_for(UP)
    if ours is None or upo is None:
        print("UNRESOLVED: a manifest could not be read; refusing to report a rate")
        return 2

    rows = []
    for (path, fn), sites in sorted(hosts.items()):
        so, do = verdict_for(fn, ours)
        su, du = verdict_for(fn, upo)
        rows.append((path, fn, sites, so, do, su, du))

    n = len(rows)
    both_safe = [r for r in rows if r[3] == "SAFE" and r[5] == "SAFE"]
    amb = [r for r in rows if "AMBIGUOUS" in (r[3], r[5])]
    miss_up = [r for r in rows if r[5] == "MISSING"]
    split = [r for r in rows if r[3] == "SAFE" and r[5] != "SAFE"]

    print("HOOK-SURFACE AUDIT — ambiguity over the population Tier-2 must hook")
    print("population: %d GENUINE vanilla host function(s) (tale §962), i.e. hosts" % n)
    print("            that EXIST in the build a user runs AND carry a WW edit.")
    print("counted from RAW manifest entries on BOTH images.")
    print("=" * 74)
    print("\n  SAFE on both images   : %3d  (%.1f%%)" % (len(both_safe),
                                                        100.0 * len(both_safe) / max(1, n)))
    print("  AMBIGUOUS on either   : %3d  (%.1f%%)  <- the rate that governs migration"
          % (len(amb), 100.0 * len(amb) / max(1, n)))
    print("  MISSING upstream      : %3d  (%.1f%%)" % (len(miss_up),
                                                       100.0 * len(miss_up) / max(1, n)))
    print("  SAFE ours / not-SAFE upstream: %d  <- would only fail in a USER's game"
          % len(split))

    if amb:
        print("\n  AMBIGUOUS hosts (hook by address or qualify — never by bare name):")
        for path, fn, sites, so, do, su, du in amb[:15]:
            print("    %-38s %-24s ours=%s upstream=%s"
                  % (path[-38:], fn[:24], so, su))
            print("        %s" % (du if su != "SAFE" else do))
        if len(amb) > 15:
            print("    ... +%d more" % (len(amb) - 15))
    if split:
        print("\n  SPLIT-VERDICT hosts (the dangerous class — clean here, not there):")
        for path, fn, sites, so, do, su, du in split[:10]:
            print("    %-38s %-24s upstream=%s (%s)" % (path[-38:], fn[:24], su, du))

    print("\n  COMPARISON: the whole-image ambiguity rate is ~1.8%% (5,076 of 279,447)")
    print("  and is dominated by the empty string and RES_U32 — data-ish names nobody")
    print("  hooks. THIS number is the one that prices 'hook the receiver layer'.")
    if "--list" in sys.argv:
        print("\n  full population:")
        for path, fn, sites, so, _d, su, _u in rows:
            print("    %-40s %-28s %2d  %s/%s" % (path, fn, sites, so, su))
    return 1 if (amb or miss_up) else 0


if __name__ == "__main__":
    sys.exit(main())
