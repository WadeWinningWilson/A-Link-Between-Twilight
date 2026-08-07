#!/usr/bin/env python3
# ============================================================================
# ww_rebaseline.py — roadmap step 13, mechanism C (user-ruled 2026-08-06).
#
# PURPOSE: stop forking zeldaret. When upstream improves a donor TU, re-derive
# our port against the new revision instead of hand-re-diffing copied files.
#
# ---------------------------------------------------------------------------
# WHY NO PATCHES ARE STORED, THOUGH THE SCOPE SAID "STORED PATCH"
#
# A unified diff carries donor source in its context and removed lines. Storing
# donor->ours patches in this tree would therefore ADD donor text to the tree --
# in a project whose standing constraint is that dusklight ships no WW code. It
# would also create a second artifact to keep in sync with the file it describes,
# which is census spec §1's drift failure in miniature.
#
# Nothing needs storing. Our file is already in the tree, and the donor at the
# pin is a PUBLIC REPRODUCIBLE COMMIT (step 12's ruling). So the delta is
# computable on demand, and a re-baseline is exactly a THREE-WAY MERGE:
#
#     base   = donor file @ the pinned commit   (what our port started from)
#     ours   = our file in this tree            (base + our adaptations)
#     theirs = donor file @ the new commit      (upstream's improvements)
#
# git merge-file does this natively and marks conflicts rather than guessing.
# The only state required is the pin, which E5 already put in every banner.
#
# ---------------------------------------------------------------------------
# WHAT THIS TOOL WILL NOT DO
#
# It does not write to any TU. Every mode is read-only and prints what WOULD
# happen, because a tool that rewrites 20 ported TUs in place on a bad pin is
# not recoverable from by inspection. Landing a re-baseline is a deliberate,
# reviewed act; --plan produces the material for that review.
#
# NEGATIVE CONTROL (--selftest): re-baselining onto the SAME pin must reproduce
# every TU byte-identically. base == theirs means the merge has nothing to do,
# so any diff at all is the tool being wrong, not the code. A re-baseline tool
# that cannot demonstrate a no-op is not an instrument.
#
# Usage:
#   ww_rebaseline.py --status              pin, donor reachability, tier split
#   ww_rebaseline.py --tier                Tier V / Tier D membership + numbers
#   ww_rebaseline.py --selftest            no-op control (must be byte-identical)
#   ww_rebaseline.py --plan <new-sha>      what a bump to <new-sha> would cost
# ============================================================================
import difflib
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DONOR = Path(r"D:\XXXXXXX\WW DP")
sys.path.insert(0, str(REPO / "tools" / "foundry"))

RE_REF = re.compile(r"^//\s*KIT-DONOR-REF:\s*(\S+)", re.M)
RE_DONOR = re.compile(r"^//\s*KIT-DONOR:\s*(\S+)", re.M)
RE_LIN = re.compile(r"^//\s*KIT-LINEAGE:\s*(\S+)", re.M)

# Tier V threshold. Visible, not buried: a TU is vendorable when our file is
# mostly donor text, so "upstream + our delta" is a smaller statement than the
# file itself. Below this, the port is our own work that consulted a donor and a
# merge would fight us rather than help. Scope doc §1 carries the measurement.
TIER_V_PCT = 70.0

# The port's own namespace flattenings (§363 convention). Normalising these is
# what stops a systematic rename from scoring as total divergence -- without it
# every JAudio1 TU reads as 0% donor text, which is how a real measurement turns
# into a wrong verdict.
FLATTEN = [("JAudio1::", "JASystem::"), ("JEvent1::", "JStudio::"), ("MDoExt1::", "")]


def _norm(lines):
    out = []
    for ln in lines:
        s = re.sub(r"\s+", " ", ln).strip()
        for a, b in FLATTEN:
            s = s.replace(a, b)
        if s and not s.startswith("//"):
            out.append(s)
    return out


def _read(p):
    return p.read_text(encoding="utf-8-sig", errors="replace")


def donor_head():
    r = subprocess.run(["git", "-C", str(DONOR), "rev-parse", "HEAD"],
                       capture_output=True, encoding="utf-8", errors="replace")
    return r.stdout.strip() or None


def donor_blob(sha, rel):
    """The donor file at a revision, or None if it does not exist there."""
    # encoding is EXPLICIT: text=True decodes with the locale codec (cp1252 on
    # this machine) and donor sources carry Shift-JIS comments, which raises
    # UnicodeDecodeError mid-scan and would look like "blob not found".
    r = subprocess.run(["git", "-C", str(DONOR), "show", f"{sha}:src/{rel}"],
                       capture_output=True, encoding="utf-8", errors="replace")
    return r.stdout if r.returncode == 0 else None


def roster():
    """Every native-port TU with exactly one donor, plus its measurement."""
    import ww_census as wc
    srcs = wc.load_build_sources() or []
    out = []
    for rel in sorted({s for s in srcs}):
        p = REPO / rel
        if not p.is_file():
            continue
        txt = _read(p)
        lin = RE_LIN.search(txt)
        if not lin or lin.group(1).split(":")[0] != "native-port":
            continue
        donors = [d for d in RE_DONOR.findall(txt) if d not in ("none", "per-hunk")]
        if len(donors) != 1:
            continue                      # consolidations have no 1:1 base
        ref = RE_REF.search(txt)
        base = donor_blob(ref.group(1).split("@")[1], donors[0]) if ref and "@" in ref.group(1) else None
        if base is None:
            out.append({"tu": rel, "donor": donors[0], "pct": None,
                        "ref": ref.group(1) if ref else None})
            continue
        ours, theirs = _norm(txt.splitlines()), _norm(base.splitlines())
        same = sum(b.size for b in difflib.SequenceMatcher(None, theirs, ours)
                   .get_matching_blocks())
        out.append({"tu": rel, "donor": donors[0], "ref": ref.group(1),
                    "pct": 100.0 * same / max(len(ours), 1)})
    return out


def _lf(text):
    """UTF-8 bytes with LF endings — the one representation everything compares in."""
    return text.replace(chr(13) + chr(10), chr(10)).encode("utf-8")


def merge3(ours_txt, base_txt, theirs_txt):
    """git merge-file. Returns (merged_bytes, conflict_count).

    BYTES, not text, deliberately. The first version decoded git's output with
    the locale codec; donor sources carry Shift-JIS comments, so the decode
    either corrupted the merge or returned None -- and the no-op control then
    reported FAIL on four TUs whose code was fine. A tool whose job is EXACT
    reproduction must not round-trip through a lossy codec to do it.
    """
    with tempfile.TemporaryDirectory() as d:
        d = Path(d)
        # LF throughout: our tree is CRLF on Windows (core.autocrlf=true) and the
        # donor is LF. Merging across that difference would report every line as
        # changed, so the comparison is deliberately line-ending-insensitive.
        for n, txt in (("ours", ours_txt), ("base", base_txt), ("theirs", theirs_txt)):
            (d / n).write_bytes(_lf(txt))
        r = subprocess.run(["git", "merge-file", "-p", "--diff3",
                            str(d / "ours"), str(d / "base"), str(d / "theirs")],
                           capture_output=True)
        # git merge-file exits with the number of conflicts, or <0 on error.
        return (r.stdout or b""), (r.returncode if r.returncode >= 0 else -1)


def main():
    argv = sys.argv[1:]
    head = donor_head()
    rows = roster()
    tierv = [r for r in rows if r["pct"] is not None and r["pct"] >= TIER_V_PCT]
    tierd = [r for r in rows if r["pct"] is not None and r["pct"] < TIER_V_PCT]
    unres = [r for r in rows if r["pct"] is None]

    if "--status" in argv or not argv:
        print(f"donor checkout : {DONOR}")
        print(f"donor HEAD     : {head or 'UNRESOLVED'}")
        print(f"Tier V (>= {TIER_V_PCT:.0f}% donor text, vendorable) : {len(tierv)} TU(s)")
        print(f"Tier D (< {TIER_V_PCT:.0f}%, our own work)           : {len(tierd)} TU(s)")
        if unres:
            print(f"UNRESOLVED (donor blob not found at the pin)  : {len(unres)} TU(s)")
            for r in unres:
                print(f"    {r['tu']}  <- {r['donor']}  ref={r['ref']}")
        print("\nNo TU is written by this tool. --plan shows what a bump would cost.")
        return 0

    if "--tier" in argv:
        for label, rs in (("TIER V — vendorable", tierv), ("TIER D — ours", tierd)):
            print(f"\n{label}: {len(rs)}")
            for r in sorted(rs, key=lambda x: -x["pct"]):
                print(f"   {r['pct']:5.1f}%  {r['tu']:44s} <- {r['donor']}")
        return 0

    if "--selftest" in argv:
        # base == theirs: the merge has nothing to apply, so the result must be
        # our file unchanged. Any difference is the tool, not the code.
        bad = 0
        for r in tierv:
            sha = r["ref"].split("@")[1]
            base = donor_blob(sha, r["donor"])
            ours = _read(REPO / r["tu"])
            merged, conflicts = merge3(ours, base, base)
            if merged != _lf(ours) or conflicts:
                print(f"  FAIL {r['tu']}  conflicts={conflicts}")
                bad += 1
        print(f"\nno-op control over {len(tierv)} Tier-V TU(s): "
              f"{'PASS — all byte-identical' if not bad else f'{bad} FAILED'}")
        return 1 if bad else 0

    if "--plan" in argv:
        new = argv[argv.index("--plan") + 1]
        r2 = subprocess.run(["git", "-C", str(DONOR), "rev-parse", new],
                            capture_output=True, text=True)
        if r2.returncode:
            print(f"UNKNOWN — cannot resolve '{new}' in the donor checkout. "
                  f"Fetch it first; this tool will not guess.")
            return 2
        new = r2.stdout.strip()
        print(f"re-baseline plan: -> {new[:12]}\n")
        clean = conflicted = untouched = 0
        for r in sorted(tierv, key=lambda x: -x["pct"]):
            sha = r["ref"].split("@")[1]
            base, theirs = donor_blob(sha, r["donor"]), donor_blob(new, r["donor"])
            if theirs is None:
                print(f"   GONE       {r['tu']} — donor file absent at {new[:8]}")
                continue
            if base == theirs:
                untouched += 1
                continue
            _, c = merge3(_read(REPO / r["tu"]), base, theirs)
            if c > 0:
                print(f"   CONFLICT×{c:<3d} {r['tu']}")
                conflicted += 1
            else:
                print(f"   clean       {r['tu']}")
                clean += 1
        print(f"\n{untouched} unchanged upstream · {clean} merge clean · "
              f"{conflicted} need hand review")
        return 0

    print(__doc__ or "see --status / --tier / --selftest / --plan <sha>")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
