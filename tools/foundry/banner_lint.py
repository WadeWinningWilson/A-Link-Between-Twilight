#!/usr/bin/env python3
# ============================================================================
# banner_lint.py — roadmap step 10 verifier for the WW provenance banner.
#
# Spec: docs/WW Linked/ww-provenance-banner-spec.md
#
# Checks, per TU:
#   * banner present and fields complete
#   * KIT-DONOR resolves against the donor's OWN configure.py object list
#   * KIT-DONOR-STATUS AGREES with that list
#
# The last one is the point. A banner is a human declaration, so unlike a
# measurement it can simply be wrong -- and a wrong provenance banner is worse
# than none, because the census trusts it over its own basename guessing. So the
# declaration is checked against the donor's own build description, which is the
# one authority neither lane wrote.
#
# №31-C throughout: a missing field is UNKNOWN, never assumed, and a TU with no
# banner is unmeasured rather than "probably fine".
#
# Usage:
#   banner_lint.py                 lint every roster TU
#   banner_lint.py <file> [...]    lint specific files
#   banner_lint.py --coverage      banner coverage over the roster only
# Read-only. Exit 1 on any DISAGREES, 2 if only UNKNOWNs.
#
# ⚠ EXIT-2 SATURATION (integrator WATCH, WAVE-1): since KIT-PLUGIN (V8) landed,
# every run exits 2 (86 UNKNOWN) until the first declarations are authored.
# Exit 2 means UNMEASURED, not red — do NOT wire exit!=0 into any pre-push
# gate before the roster carries declarations, or the tree is permanently red.
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402
import census_axis_d as D  # noqa: E402

FIELDS = ("KIT-LINEAGE", "KIT-DONOR", "KIT-DONOR-REF", "KIT-DONOR-STATUS")
# KIT-PLUGIN (V8) is checked separately in lint_one — not in FIELDS, so its
# absence message names the §7 spec rather than the generic line.
KP = "KIT-PLUGIN"
LINE = re.compile(r"^//\s*(KIT-[A-Z-]+):\s*(.+?)\s*$", re.M)
# F3: HUNK SCOPE. A shared receiver TU (TP's own d_stage.cpp, d_demo.cpp, ...)
# can carry donor-derived lines WITHOUT the whole file being a port. Neither
# file-level value is true for those: naming a donor path is false (the file is
# TP's), and `none` is WORSE THAN FALSE -- it tells the strip-set generator the
# file is clean. Whole-file stripping is impossible for them anyway; the build
# needs d_stage.cpp. So provenance moves to the edit site:
#
#     // KIT-DONOR-HUNK: d/d_stage.cpp Matching
#         <donor-derived lines>
#     // KIT-DONOR-HUNK-END
#
# and the file top declares `per-hunk` instead of a path. The sentinel is NOT a
# dodge: a file claiming `per-hunk` with zero hunk markers is a DISAGREES, as is
# an unbalanced marker (a missing END silently extends a hunk over TP code, which
# would over-report rather than under-report -- still wrong).
HUNK = re.compile(r"^\s*//\s*KIT-DONOR-HUNK:\s*(\S+)(?:\s+(\S+))?\s*$", re.M)
HUNK_END = re.compile(r"^\s*//\s*KIT-DONOR-HUNK-END\s*$", re.M)
PER_HUNK = "per-hunk"
# F1: a KIT-DONOR line may carry its OWN status, because a TU porting from
# several donors with divergent statuses has no single true aggregate.
#     // KIT-DONOR: d/d_flower.cpp MatchingFor
DONOR_LINE = re.compile(r"^(\S+)(?:\s+(\S+))?$")
# E4: `^//` never matched an INDENTED marker, and a donor array declared inside a
# function (ja1_parser's kArgCount/kArgFmt) is indented. Leading space allowed.
DATA = re.compile(r"^\s*//\s*KIT-DONOR-DATA:\s*(\d+)\s+(\S+)\s*(.*)$", re.M)
# census spec §6's own four buckets -- a class outside this set is a typo, and a
# typo'd class silently drops the bytes out of whichever tally the ruling reads.
DATA_CLASSES = ("gx-register-state", "lookup-table", "display-list", "asset-like")


def read_banner(path):
    """All KIT-* fields declared in a file. Multiple KIT-DONOR lines allowed."""
    p = C.REPO / path
    if not p.is_file():
        return None
    out = {}
    for m in LINE.finditer(p.read_text(encoding="utf-8-sig", errors="replace")):
        out.setdefault(m.group(1), []).append(m.group(2))
    return out


def read_hunks(path):
    """(donor, status, line_no) per KIT-DONOR-HUNK marker, plus END count."""
    p = C.REPO / path
    if not p.is_file():
        return [], 0
    txt = p.read_text(encoding="utf-8-sig", errors="replace")
    hunks = [(m.group(1), m.group(2), txt[:m.start()].count("\n") + 1)
             for m in HUNK.finditer(txt)]
    return hunks, len(HUNK_END.findall(txt))


def lint_hunks(path, status_map, donors):
    """F3: validate hunk markers the same way file-level donors are validated."""
    findings = []
    hunks, ends = read_hunks(path)
    claims_per_hunk = PER_HUNK in donors
    if claims_per_hunk and not hunks:
        findings.append(("DISAGREES", path,
                         f"KIT-DONOR says '{PER_HUNK}' but the file carries no "
                         f"KIT-DONOR-HUNK marker — the sentinel is not a way to "
                         f"leave a shared TU unmeasured"))
    if hunks and not claims_per_hunk:
        findings.append(("DISAGREES", path,
                         f"{len(hunks)} KIT-DONOR-HUNK marker(s) present but "
                         f"KIT-DONOR does not say '{PER_HUNK}' — the file top "
                         f"must point at them or the census will not look"))
    if len(hunks) != ends:
        findings.append(("DISAGREES", path,
                         f"{len(hunks)} KIT-DONOR-HUNK vs {ends} "
                         f"KIT-DONOR-HUNK-END — an unbalanced marker claims a "
                         f"span it does not own"))
    for donor, status, ln in hunks:
        if donor not in status_map:
            findings.append(("DISAGREES", path,
                             f"line {ln}: hunk donor '{donor}' is not an object "
                             f"in the donor's configure.py"))
        elif status and status != "UNKNOWN" \
                and status.split("(")[0] != status_map[donor].split("(")[0]:
            findings.append(("DISAGREES", path,
                             f"line {ln}: hunk says '{status}' but the donor "
                             f"build says '{status_map[donor]}' for {donor}"))
    return findings


def lint_one(path, status_map):
    b = read_banner(path)
    if b is None:
        return [("MISSING-FILE", path, "not on disk")]
    findings = []
    if "KIT-LINEAGE" not in b:
        return [("NO-BANNER", path,
                 "no KIT-LINEAGE — unmeasured, not clean (§31-C)")]

    for f in FIELDS:
        if f not in b:
            findings.append(("UNKNOWN", path, f"{f} absent — UNKNOWN, not assumed"))

    # ========================================================================
    # V8 KIT-PLUGIN (ww-provenance-banner-spec.md §7, WAVE-1 row 12).
    # DECLARED never inferred; missing = UNKNOWN (§31-C); enum-checked.
    # Monotonic plugin-bound invariant is enforced in main() over the run.
    # ========================================================================
    import re as _re
    kp = (b.get("KIT-PLUGIN") or [None])[0]
    if kp is None:
        findings.append(("UNKNOWN", path,
                         "KIT-PLUGIN absent — migration disposition unmeasured (§31-C)"))
    elif not _re.fullmatch(
            r"receiver-native|plugin-bound(:wave-[0-9]+)?|in-plugin|strip-set|UNKNOWN", kp):
        findings.append(("DISAGREES", path,
                         f"KIT-PLUGIN value {kp!r} outside the §7 enum"))

    donors = b.get("KIT-DONOR", [])
    aggregate = (b.get("KIT-DONOR-STATUS") or ["UNKNOWN"])[0]
    per_donor = [DONOR_LINE.match(d) for d in donors]
    if len([m for m in per_donor if m and m.group(2)]) == 0 and len(donors) > 1 \
            and aggregate != "UNKNOWN":
        findings.append(("DISAGREES", path,
                         f"{len(donors)} KIT-DONOR lines share one aggregate "
                         f"status '{aggregate}' — with multiple donors the "
                         f"aggregate must be UNKNOWN or each line must carry "
                         f"its own status (F1)"))
    for raw in donors:
        m = DONOR_LINE.match(raw)
        donor = m.group(1) if m else raw
        line_status = m.group(2) if m else None
        if donor in ("none", PER_HUNK):
            continue
        if donor not in status_map:
            findings.append(("DISAGREES", path,
                             f"KIT-DONOR '{donor}' is not an object in the "
                             f"donor's configure.py — the declaration names a "
                             f"path the donor build does not"))
            continue
        declared = line_status or aggregate
        actual = status_map[donor]
        if declared != "UNKNOWN" and declared.split("(")[0] != actual.split("(")[0]:
            findings.append(("DISAGREES", path,
                             f"KIT-DONOR-STATUS says '{declared}' but the donor "
                             f"build says '{actual}' for {donor}"))
    findings += lint_hunks(path, status_map, donors)
    findings += lint_data(path, status_map)
    findings += lint_ref(path, b)
    return findings


def donor_head():
    """The donor checkout's current commit, or None if it cannot be resolved.

    E5: the pin claims these banners were verified against a specific donor tree.
    If that checkout MOVES, every banner in the repo silently keeps asserting a
    verification that no longer corresponds to anything on disk -- a stale pin is
    worse than `unpinned`, because `unpinned` at least tells the truth.
    """
    import subprocess
    try:
        r = subprocess.run(["git", "-C", str(D.DONOR), "rev-parse", "HEAD"],
                           capture_output=True, text=True, timeout=15)
        return r.stdout.strip() or None
    except Exception:
        return None


_HEAD_CACHE = []


def lint_ref(path, banner):
    """E5: a pinned KIT-DONOR-REF must match the donor checkout it names."""
    refs = banner.get("KIT-DONOR-REF") or []
    findings = []
    if not _HEAD_CACHE:
        _HEAD_CACHE.append(donor_head())
    head = _HEAD_CACHE[0]
    for ref in refs:
        if ref == "unpinned":
            continue
        if "@" not in ref:
            findings.append(("DISAGREES", path,
                             f"KIT-DONOR-REF '{ref}' is neither 'unpinned' nor "
                             f"'<remote>@<sha>' — an unresolvable pin"))
            continue
        sha = ref.split("@", 1)[1]
        if head is None:
            # Cannot resolve the donor checkout: report UNKNOWN, never clean.
            findings.append(("UNKNOWN", path,
                             "KIT-DONOR-REF pinned but the donor checkout could "
                             "not be resolved — pin unverified, not verified"))
        elif sha != head:
            findings.append(("DISAGREES", path,
                             f"KIT-DONOR-REF pins {sha[:12]} but the donor "
                             f"checkout is at {head[:12]} — the pin is STALE and "
                             f"the banner's donor facts were verified against a "
                             f"tree that is no longer checked out"))
    return findings


def read_data(path):
    """(bytes, class, source, line) per KIT-DONOR-DATA marker."""
    p = C.REPO / path
    if not p.is_file():
        return []
    txt = p.read_text(encoding="utf-8-sig", errors="replace")
    return [(int(m.group(1)), m.group(2), m.group(3).strip(),
             txt[:m.start()].count("\n") + 1) for m in DATA.finditer(txt)]


def lint_data(path, status_map):
    """E4: a donor-data marker is only rulable if its class and donor resolve."""
    findings = []
    for nbytes, cls, src, ln in read_data(path):
        if cls not in DATA_CLASSES:
            findings.append(("DISAGREES", path,
                             f"line {ln}: KIT-DONOR-DATA class '{cls}' is not one "
                             f"of census spec §6's buckets {DATA_CLASSES}"))
        if nbytes <= 0:
            findings.append(("DISAGREES", path,
                             f"line {ln}: KIT-DONOR-DATA claims {nbytes} bytes — a "
                             f"marker that counts nothing cannot be ruled on"))
        # If the source opens with a donor-relative path, it must be a real one.
        head = src.split()[0] if src else ""
        if head.endswith((".cpp", ".c")) and head not in status_map:
            findings.append(("DISAGREES", path,
                             f"line {ln}: KIT-DONOR-DATA source '{head}' is not an "
                             f"object in the donor's configure.py"))
    return findings


def main():
    argv = [a for a in sys.argv[1:] if not a.startswith("--")]
    status_map = D.donor_status()
    if status_map is None:
        print("UNKNOWN — donor configure.py unreadable; nothing verified")
        return 2

    if argv:
        targets = argv
    else:
        import ww_census as wc
        srcs = wc.load_build_sources() or []
        rows = [wc.classify_tu(s) for s in srcs]
        targets = wc.roster_union(rows)   # HT-27: one predicate, not a copy

    banners = disagree = unknown = 0
    rows_out = []
    for t in targets:
        f = lint_one(t, status_map)
        if not f or all(k not in ("NO-BANNER",) for k, _, _ in f):
            if read_banner(t) and "KIT-LINEAGE" in (read_banner(t) or {}):
                banners += 1
        for kind, path, why in f:
            if kind == "DISAGREES":
                disagree += 1
            elif kind in ("UNKNOWN", "NO-BANNER"):
                unknown += 1
            if kind != "NO-BANNER" or "--coverage" not in sys.argv:
                rows_out.append((kind, path, why))

    if "--data" in sys.argv:
        # E4: THE number roadmap step 9's trip-wire (b) is defined on -- donor
        # bytes, not "arrays that live in roster TUs". Reported per class because
        # the trip-wire reads shape, not just a total.
        from collections import Counter
        per_class, total, rows = Counter(), 0, []
        for t_ in targets:
            for nbytes, cls, src, ln in read_data(t_):
                per_class[cls] += nbytes
                total += nbytes
                rows.append((nbytes, cls, t_, ln, src))
        print(f"DECLARED donor data: {total} bytes over {len(rows)} array(s)")
        for cls, b in per_class.most_common():
            print(f"   {b:6d}  {cls}")
        print("\nEvery byte here is DECLARED at its own declaration site and its "
              "donor verified against configure.py.\nUndeclared arrays in roster "
              "TUs are NOT counted: an array living in a WW TU is not donor data.")
        for nbytes, cls, t_, ln, src in sorted(rows, reverse=True):
            print(f"   {nbytes:6d} {cls:16s} {t_.split('/')[-1]}:{ln}  {src[:44]}")
        return 0

    if "--coverage" in sys.argv:
        # F2: measure what the caption claims. Counting KIT-LINEAGE counted a
        # tag §426 had already landed on 16 TUs, so the figure could not move as
        # E1-E3 landed -- 22% before E1 and 22% after -- while promising it
        # would. A headline number that cannot move is worse than none, because
        # progress reads as stall.
        full = sum(1 for t_ in targets
                   if all(f in (read_banner(t_) or {}) for f in FIELDS))
        lineage = sum(1 for t_ in targets
                      if "KIT-LINEAGE" in (read_banner(t_) or {}))
        missing_fields = sum(
            1 for t_ in targets for f in FIELDS
            if f not in (read_banner(t_) or {}))
        n = max(len(targets), 1)
        print(f"FULL four-field banners : {full}/{len(targets)} "
              f"({100.0 * full / n:.0f}%)   <- the figure that moves with E1-E3")
        print(f"KIT-LINEAGE only        : {lineage}/{len(targets)} "
              f"({100.0 * lineage / n:.0f}%)   <- §426 baseline, NOT progress")
        print(f"missing fields (UNKNOWN): {missing_fields}"
              f"   <- finest-grained signal; falls by 3 per TU bannered")
        print(f"D-5/D-6 close on the FULL count, not the lineage count.")
        return 0

    for kind, path, why in rows_out[:40]:
        print(f"  [{kind:11s}] {path}")
        print(f"                {why}")
    print(f"\n{len(targets)} TU(s): {banners} bannered, {disagree} DISAGREES, "
          f"{unknown} UNKNOWN/absent")
    if disagree:
        print("A banner that disagrees with the donor's own build description is "
              "a defect, not a fact.")
    return 1 if disagree else (2 if unknown else 0)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
