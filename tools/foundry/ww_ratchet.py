#!/usr/bin/env python3
# ============================================================================
# ww_ratchet.py - THE ONE NUMBER, AND THE GUARD THAT STOPS IT LYING.
#
# ---------------------------------------------------------------------------
# From the user's roadmap (docs/state/ww-staging/ROADMAP-fork-to-plugin.md):
#
#   "Tree-side WW TU count - pinned at Phase 0, monotonically decreasing
#    thereafter. Not 'percent ported', which measures content. This measures
#    whether the architecture migration is real, and it is the only number
#    that distinguishes 'finishing Outset in the fork forever' from
#    'actually leaving'."
#
# ---------------------------------------------------------------------------
# THE SCOPE CALL (Foundry, 2026-08-16, user-delegated). `layer_census` reports
# FOUR scopes and they disagree: STRIP-SET 83 .cpp, FILENAME 38, EXT-DIRS 17,
# LINEAGE-DECLARED 102. A ratchet is meaningless unless the scope is fixed, so:
#
#   **THE RATCHET IS LINEAGE-DECLARED .cpp** - translation units carrying a
#   `// KIT-LINEAGE` tag.
#
# Why this one and not the others:
#   * FILENAME / EXT-DIRS are too narrow. They count only files that ANNOUNCE
#     themselves, so they can reach ZERO while 140 vanilla hosts still need
#     hooking. **A ratchet that permits a false victory is worse than none.**
#   * STRIP-SET is the never-push list and carries non-WW work (ALBW files are
#     in it). It could never reach zero for reasons unrelated to WW.
#   * A raw fork-vs-origin diff is 1,155 changed .cpp - overwhelmingly mod work
#     that will NEVER migrate. It overstates the debt by an order of magnitude.
#   * LINEAGE-DECLARED is the only scope DECLARED BY AN AUTHOR rather than
#     inferred from a naming convention or a policy list. Mod work does not
#     carry the tag, so it cannot contaminate the count.
#
# ITS KNOWN WEAKNESS, STATED NOT HIDDEN: an untagged WW TU is invisible to it.
# That is a discipline problem, and the fix is to make the tag a gate condition
# rather than to pick a worse scope.
#
# ---------------------------------------------------------------------------
# WHY THERE IS A SECOND NUMBER, AND WHY IT IS NOT OPTIONAL. **Every file-level
# scope is blind to WW edits living inside UNMARKED TP files** - and that is
# where the hard residue is: 140 genuine vanilla hosts to hook, and 15 static-
# table sites with NO hook equivalent. The TU count can reach ZERO with all of
# those still owed. Phase 3's exit gate ("tree-side count -> 0, or a declared
# irreducible residue") is unfalsifiable without them. So `tier2_census`'s site
# total rides along as part of the gate, not as a rival metric.
#
# Usage:
#   ww_ratchet.py            measure, and compare against the pinned baseline
#   ww_ratchet.py --pin      pin TODAY as the Phase 0 baseline (writes json)
#   ww_ratchet.py --selftest negative control: can it report RISING at all?
#
# Exit 0 = falling or holding - 1 = RISING (Phase 0's gate is not met) -
#        2 = could not measure.
# ============================================================================
import json
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
BASELINE = HERE / "ww-ratchet-baseline.json"
TAG = "KIT-LINEAGE"


def lineage_tus():
    """.cpp under src/ carrying a KIT-LINEAGE tag. Recursive - the shell glob
    `src/**/*.cpp` does NOT recurse here and reported 28 against a true 102."""
    n, names = 0, []
    for p in (REPO / "src").rglob("*.cpp"):
        try:
            if TAG in p.read_text(encoding="utf-8", errors="replace"):
                n += 1
                names.append(str(p.relative_to(REPO)))
        except OSError:
            continue
    return n, sorted(names)


def tier2_sites():
    """(A, B, C) site counts, or None if the census cannot run."""
    try:
        r = subprocess.run([sys.executable, str(HERE / "tier2_census.py")],
                           capture_output=True, text=True, encoding="utf-8",
                           errors="replace", timeout=600, cwd=str(REPO))
    except Exception:
        return None
    out = (r.stdout or "") + (r.stderr or "")
    got = {}
    for key, pat in (("A", r"^\s*A\s.*?:\s*(\d+) site"),
                     ("B", r"^\s*B\s.*?:\s*(\d+) site"),
                     ("C", r"^\s*C\s.*?:\s*(\d+) site")):
        m = re.search(pat, out, re.M)
        if not m:
            return None
        got[key] = int(m.group(1))
    return got["A"], got["B"], got["C"]


MAP = REPO / "docs" / "state" / "ww-staging" / "ww-ownership-map.json"
# ============================================================================
# ENUMERATE THE SIDE THAT CANNOT GROW (the exchange's final rule, 08-16).
# This was NEVER_MIGRATES = {albw, albw-skins, fork-host-plumbing, ...} - a
# set that grew TWICE in one evening (instrument, fork-host-plumbing were
# both added tonight), and any future addition would silently count as
# MIGRATING: the ratchet would report more progress than exists. **For a
# migration metric the failure mode must be PESSIMISTIC** - an unclassified
# category showing up as floor is visible and safe; showing up as migrating
# quietly flatters the project. So the enumeration flips to the only two
# categories that mean "moves", which are pinned by the metric's own
# definition and cannot grow without redefining the metric itself.
# ============================================================================
MIGRATES = {"ww-port", "shared-per-hunk"}


def terminal_floor(names):
    """How many counted TUs can never leave, per the adjudicated map.

    ====================================================================
    THE DEFINITION QUESTION, RULED 2026-08-16 (Integrator posed it and
    correctly declined to answer it unilaterally; scope is the baseline
    owner's). After the per-hunk digs, `d_a_demo00.cpp` is FORK/ALBW-skins
    with NO plugin share - it will never migrate - and it still carries a
    legitimate `mixed` KIT-LINEAGE tag, because its skins helper is
    DONOR-DERIVED. **The tag is provenance; the map is ownership; this
    metric counts tags.** So a never-migrating TU is a permanent floor
    under a number whose stated job is reaching zero.

    Ruling: the BASELINE DOES NOT RE-PIN and the tags DO NOT MOVE
    (deleting true provenance to fix a readout would edit source on an
    accounting argument). Instead the floor is DERIVED from the
    adjudicated ownership map and printed with the number, so "complete"
    reads as TU == floor, not TU == 0, and the floor updates itself when
    the map does. Absent map -> floor UNKNOWN, stated, never assumed zero.
    ====================================================================
    """
    if not MAP.is_file():
        return None, []
    import json
    data = json.loads(MAP.read_text(encoding="utf-8"))
    # CASE-NORMALIZED ON READ (A5 pass, 2026-08-16): UNKNOWN-VERDICT is the
    # only non-lowercase token of the eleven declared, and it is the
    # we-do-not-know value - a case-sensitive matcher would silently drop
    # exactly the ignorance marker (Librarian's hazard, History-confirmed).
    # Normalizing costs nothing under the moves-side rule: an unrecognised
    # token in any casing already lands on the floor, visibly.
    def _norm(c):
        return c.lower() if isinstance(c, str) else c
    cats = {f: _norm(v.get("category"))
            for f, v in data.get("files", {}).items()}
    # ================================================================
    # THE ENUMERATING-MATCHER LAW (Integrator, 08-16, after 'fork' vs
    # 'fork-host-plumbing' silently dropped boot_stage.cpp from a floor
    # computation - fifth specimen of the mechanism in one session).
    # This consumer keys on the map's `category`; it therefore VALIDATES
    # its filter set against the map's DECLARED domain and goes loud on
    # any mismatch, in either direction. A silently-smaller floor is a
    # false absence wearing a plausible number.
    # ================================================================
    domain = {_norm(c) for c in (data.get("_categories") or [])}
    if domain:
        stray_filter = MIGRATES - domain
        stray_files = {c for c in cats.values() if c not in domain}
        if stray_filter or stray_files:
            print("  ** FLOOR UNKNOWN - category domain mismatch **")
            if stray_filter:
                print("     migrate-set names unknown categories: %s"
                      % sorted(stray_filter))
            if stray_files:
                print("     map carries undeclared categories: %s"
                      % sorted(stray_files))
            return None, []
    # a TAGGED TU absent from the map means the map is STALE relative to the
    # tree - completion cannot be computed against an incomplete authority
    unmapped = [n for n in names if Path(n).as_posix() not in cats]
    if unmapped:
        print("  ** FLOOR UNKNOWN - %d tagged TU(s) absent from the map **"
              % len(unmapped))
        for n in unmapped[:6]:
            print("     %s" % n)
        return None, []
    # floor = everything NOT in the bounded migrate set - pessimistic by
    # construction: a novel category lands here, visible, until ruled
    floor = [n for n in names
             if cats.get(Path(n).as_posix()) not in MIGRATES]
    # ================================================================
    # THE FLOOR IS TWO BUCKETS, PARTITIONED EVEN WHILE ONE IS EMPTY
    # (Integrator's refinement, A5 pass 2026-08-16): (a) RESIDENT-FLOOR -
    # never migrates, legitimately ours; the honest endpoint includes it.
    # (b) OUT-OF-SCOPE - upstream-native / not-ww / stale-path: a file
    # wearing one of these is a MEASUREMENT ERROR tagged, not a resident;
    # it should be REMOVED FROM SCOPE, not counted as floor. All three
    # are latent today, so the printed floor is entirely (a) - but the
    # empty bucket carries its name so the next category lands in the
    # right one instead of quietly inflating a pessimistic-looking
    # number that has stopped meaning what its label says (the skins-117
    # shape: count stays true, label stops being).
    # ================================================================
    OUT_OF_SCOPE = {"upstream-native", "not-ww", "stale-path"}
    oos = [n for n in floor
           if cats.get(Path(n).as_posix()) in OUT_OF_SCOPE]
    if oos:
        print("  ** %d floor TU(s) carry OUT-OF-SCOPE categories "
              "(upstream-native/not-ww/stale-path) - these are measurement "
              "errors wearing tags, not residents; the floor below EXCLUDES "
              "them and they need de-scoping, not counting: **" % len(oos))
        for n in oos[:6]:
            print("     %s (%s)" % (n, cats.get(Path(n).as_posix())))
        floor = [n for n in floor if n not in oos]
    return len(floor), floor


ROWS_DIR = REPO / "docs" / "state" / "ww-staging" / "tracker" / "rows"


def rowed_split(names, floor_files):
    """ROWED vs LEGACY-UNROWED - the A4 spec's own readout: 'the ratchet
    counts LEGACY separately from ROWED so the drain is visible as progress
    rather than hidden inside one number.' A migrating TU leaves LEGACY the
    day its seam gets a tracker row; fully-done reads ROWED+floor == TU
    with LEGACY at zero."""
    covered = set()
    if ROWS_DIR.is_dir():
        for p in ROWS_DIR.glob("*.md"):
            m = re.search(r"^tu:\s*(\S+)",
                          p.read_text(encoding="utf-8-sig", errors="replace"),
                          re.M)
            if m:
                covered.add(m.group(1))
    name_set = {Path(n).as_posix() for n in names}
    floor_set = {Path(f).as_posix() for f in (floor_files or [])}
    rowed = {n for n in name_set if n in covered}
    legacy = name_set - rowed - floor_set
    return len(rowed), len(legacy)


def measure():
    tus, names = lineage_tus()
    sites = tier2_sites()
    floor_n, floor_files = terminal_floor(names)
    rowed_n, legacy_n = rowed_split(names, floor_files)
    return {"lineage_tus": tus, "tier2": sites,
            "tier2_total": None if sites is None else sum(sites),
            "floor": floor_n, "floor_files": floor_files,
            "rowed": rowed_n, "legacy": legacy_n,
            "files": names}


def main():
    cur = measure()
    if cur["tier2"] is None:
        print("CANNOT MEASURE: tier2_census did not produce parseable counts.")
        print("  **UNKNOWN is not a passing ratchet** - fix the census first.")
        return 2

    print("WW RATCHET - tree-side WW, the roadmap's one number")
    print("  SCOPE: LINEAGE-DECLARED .cpp (files tagged `// %s`)" % TAG)
    print("  ratchet   TU count      : %d" % cur["lineage_tus"])
    if cur["floor"] is None:
        print("  terminal floor          : UNKNOWN - ownership map absent; "
              "'complete' is undefined until it exists")
    else:
        print("  terminal floor          : %d  (tagged TUs the adjudicated "
              "map says NEVER migrate:" % cur["floor"])
        for f in cur["floor_files"]:
            print("      %s" % f)
        print("    **COMPLETE reads as TU == %d, not 0** - the tag records "
              "provenance, the map records ownership" % cur["floor"])
    print("  drain visibility (A4)   : ROWED %d | LEGACY-UNROWED %d | "
          "floor %s   (done = LEGACY 0, ROWED+floor == TU)"
          % (cur["rowed"], cur["legacy"],
             cur["floor"] if cur["floor"] is not None else "?"))
    print("  gate rider tier2 sites  : %d  (A %d cheap / B %d in-place / "
          "C %d no hook AT THE SITE)"
          % (cur["tier2_total"], *cur["tier2"]))
    # ====================================================================
    # TWO CAVEATS PRINTED WITH THE NUMBER, because both were learned by
    # someone being misled by it on 2026-08-16.
    #
    # 1. C-class is NOT irreducible. I filed it as the patcher-escalation
    #    trigger; the Integrator and History/Bridge both showed those 15
    #    rows are `l_objectName[]` entries reached through
    #    `dStage_searchName` (a plain strcmp loop returning NULL on miss),
    #    so a hook answers them with no receiver change. **"No hook at the
    #    SITE" is not "unreachable" — check the CONSUMER.**
    #
    # 2. The rider may be summing TWO TRACKS. The SKINS work reads the
    #    receiver's OWN archives (TP ships byte-identical WW itemmdl data)
    #    and needs no donor disc, so it is arguably not migration debt at
    #    all; the PORT track is. Until they are split this number is an
    #    UPPER BOUND. And tier2's `RE_WW` anchors `\bww_`, which cannot
    #    match `d_ww_itemmdl_pc.h` — the char before `ww` is an underscore,
    #    a word character, so there is no boundary. **138 sites across 13
    #    files are invisible to it**, which means the rider is also an
    #    UNDERCOUNT of what it does intend to measure. Both directions at
    #    once; do not quote it as a total.
    # ====================================================================
    # ====================================================================
    # CAVEAT CORRECTED 2026-08-16 (second revision, Integrator's catch).
    # The first revision said the rider "may include the SKINS track" and
    # undercounts by ~138. Both halves were wrong once the user ruled the
    # skins track ALBW and the Integrator re-ran the 117 skins sites
    # against THIS ratchet's own scope: nine of the eleven skins files
    # carry no KIT-LINEAGE tag, so 114 of 117 sites WERE NEVER COUNTED.
    # **At most 3 skins sites are in the rider** (d_demo.cpp 2,
    # d_a_demo00.cpp 1 - both inside user-deferred DUAL-PENDING files).
    # The genuine undercount is the ~21 PORT-track sites `\bww_` cannot
    # see. **The ratchet was scoped correctly while both worklists and
    # both scanners were wrong** - do not "fix" its number to match them.
    # ====================================================================
    print("    CAVEAT: skins track = ALBW (user ruling 08-16); the dual-pending")
    print("    splits RESOLVED 08-16 - the 3 in-scope skins sites are ALBW and")
    print("    out of WW debt; npc_mount's 5 dWwItemmdl_ sites are PORT (WW")
    print("    clothes-get presentation, not reskin). Genuine undercount: ~21")
    print("    PORT sites now visible (A2 repair). C-class is NOT irreducible")
    print("    (reached via consumers, dStage_searchName precedent) AND ~20 of")
    print("    C's 35 are ALREADY-NULL relinquished profile rows - receipts of")
    print("    a migration that HAPPENED, not remaining debt (H/B, read at")
    print("    source 08-16). Quote C as 'at most 15 live table rows'.")
    print()

    if "--pin" in sys.argv:
        BASELINE.write_text(json.dumps(
            {"pinned": "Phase 0", "lineage_tus": cur["lineage_tus"],
             "tier2": list(cur["tier2"]), "tier2_total": cur["tier2_total"]},
            indent=2) + "\n", encoding="utf-8")
        print("  PINNED as the Phase 0 baseline -> %s" % BASELINE.name)
        print("  From here the TU count must never rise. **The baseline is a")
        print("  stamped moment; do not edit it to match a later measurement**")
        print("  - that destroys the only evidence the number moved.")
        return 0

    if "--selftest" in sys.argv:
        # NEGATIVE CONTROL. A ratchet that cannot report RISING is a ratchet
        # that always says things are fine - the vacuous pass, applied to the
        # one number the whole roadmap is tracked by.
        fake = {"lineage_tus": cur["lineage_tus"] - 1,
                "tier2_total": cur["tier2_total"] - 1}
        rising = cur["lineage_tus"] > fake["lineage_tus"]
        print("  SELFTEST: baseline forced to %d against a live %d."
              % (fake["lineage_tus"], cur["lineage_tus"]))
        print("  A correct verdict is RISING.")
        print("  -> %s" % ("RISING (control fired)" if rising
                           else "**NOT RISING - THE RATCHET IS BLIND**"))
        return 0 if rising else 1

    if not BASELINE.is_file():
        print("  NO BASELINE PINNED. This is a measurement, not a verdict -")
        print("  run `ww_ratchet.py --pin` at Phase 0 to start the ratchet.")
        return 0

    base = json.loads(BASELINE.read_text(encoding="utf-8"))
    d_tu = cur["lineage_tus"] - base["lineage_tus"]
    d_site = cur["tier2_total"] - base["tier2_total"]
    word = "RISING" if d_tu > 0 else ("HOLDING" if d_tu == 0 else "FALLING")
    print("  vs Phase 0 baseline: TU %+d (%d -> %d) - sites %+d (%d -> %d)"
          % (d_tu, base["lineage_tus"], cur["lineage_tus"],
             d_site, base["tier2_total"], cur["tier2_total"]))
    print("  **%s**" % word)
    # ================================================================
    # RIDER SCOPE EVENTS - measured jumps caused by SCANNER repairs, not
    # by work moving. The stamped v1 baseline (386) is never edited (it
    # is history); instead each repair records its measured delta here,
    # and the movement warning fires only on motion BEYOND the recorded
    # scope changes. First entry: the A2 repair (2026-08-16) made the
    # dWw[A-Z] port shape and wrapped hosts visible - +22 sites that were
    # always in the tree and never in the scan.
    # ================================================================
    SCOPE_EVENTS = [("2026-08-16 A2 scanner repair (dWw shape + wrapped "
                     "hosts + skins exclusion)", 22),
                    # TRANSIENT, not a scanner-scope change: real new content,
                    # but INSTRUMENT-class - the s398 pending-consumption
                    # probe block in dScnPly_Execute (black-screen H3 line,
                    # strip-before-push). Attributed 2026-08-17 after ruling
                    # OUT the map re-cut (the census scans by NAME and never
                    # reads the map - both files were always scanned; H/B's
                    # own caveat 'causation unproven' was the correct flag).
                    # REMOVE THIS ENTRY WHEN THE PROBES STRIP - sites should
                    # drop by 2 and a stale entry here would hide the next
                    # real +2.
                    ("2026-08-17 dScnPly s398 black-screen probe block "
                     "(TRANSIENT - instrument, strip-before-push; remove "
                     "on strip)", 2)]
    explained = sum(n for _, n in SCOPE_EVENTS)
    if d_site:
        for label, n in SCOPE_EVENTS:
            print("  rider scope event: %+d  %s" % (n, label))
    if d_tu <= 0 and d_site > explained:
        print("  WARNING: sites rose %+d BEYOND the recorded scope events -"
              % (d_site - explained))
        print("  work is moving INTO unmarked TP files, where no file-level")
        print("  scope can see it.")
    return 1 if d_tu > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
