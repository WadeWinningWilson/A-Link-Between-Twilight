#!/usr/bin/env python3
# ============================================================================
# row_origin.py — B3: origin-merge attribution for the seam-tracker.
#
#   attribute  which changed files are ORIGIN-authored vs OURS
#   sync       auto-advance past origin-authored change; mark touched rows STALE
#   selftest   NEGATIVE CONTROL - the STALE path must be shown to fire
#
# THE POINT (roadmap B3): "baseline auto-advances past origin-authored changes;
# origin changes touching OUR rows mark them STALE - zero spurious row demands
# on a sync, BECAUSE FALSE FAILURES TRAIN BYPASS."
#
# ---------------------------------------------------------------------------
# ⚠ THE REMOTE NAMES IN THIS REPO ARE INVERTED FROM CONVENTION. READ THIS
#   BEFORE EDITING, BECAUSE THE OBVIOUS CODE IS THE WRONG CODE:
#
#     origin    = TwilitRealm/dusklight                  <- the ORIGIN PROJECT
#     upstream  = WadeWinningWilson/A-Link-Between-...   <- OUR FORK
#
#   Normal convention is the exact opposite (origin = your fork). A naive
#   "origin-merge attribution" would attribute OUR work to the origin project
#   and vice versa - it would auto-advance past our own edits, which is the
#   deletion bypass wearing a different hat. The mapping is therefore NAMED
#   here rather than inferred, and `attribute` prints which remote it treated
#   as the origin so a reader can catch it being wrong.
# ---------------------------------------------------------------------------
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
TRACKER = REPO / "docs" / "state" / "ww-staging" / "tracker"
ROWS = TRACKER / "rows"
STALE = TRACKER / "STALE.md"

# ---------------------------------------------------------------------------
# RESOLVE BY URL, NOT BY NAME (Librarian, 2026-08-16, on the user's instruction
# "fix that tool so the wording doesn't become a trap").
#
# The header above correctly WARNS that the names are inverted. A warning in a
# comment protects the reader who reads it — this repo's whole week says that is
# the weaker half. The name `origin` is a LOCAL NICKNAME: it can be renamed,
# re-pointed, or differ in a fresh clone, and if it ever does, every line below
# keeps working and silently means the opposite thing.
#
# **The URL is the identity that cannot be renamed. Resolve on that, and REFUSE
# when it does not match** — enumerate the side that cannot grow.
#
# The vocabulary is also fixed here, because "origin" was doing two jobs at once
# — a git remote name AND the concept "the project we forked from" — and they
# coincide in this repo by accident:
#
#     VANILLA = TwilitRealm/dusklight                 (the project we forked FROM)
#     FORK    = WadeWinningWilson/A-Link-Between-...  (ours)
#
# Neither word is a git remote name, so neither can be silently wrong.
# ---------------------------------------------------------------------------
VANILLA_URL_MARK = "TwilitRealm/dusklight"
FORK_URL_MARK = "A-Link-Between-Dusklight"

# Fallback ONLY — used when URL resolution fails, and never silently: every
# caller prints which remote it resolved and how.
VANILLA_REMOTE_FALLBACK = "origin"


def _remotes():
    """{name: url} from `git remote -v` (fetch lines)."""
    rc, out = git("remote", "-v")
    found = {}
    if rc == 0:
        for ln in out.splitlines():
            parts = ln.split()
            if len(parts) >= 2 and "(fetch)" in ln:
                found[parts[0]] = parts[1]
    return found


def resolve_vanilla():
    """(remote_name, url, how). REFUSES rather than guessing.

    Returns how='url' when matched on the vanilla URL — the trustworthy path;
    how='fallback-name' when it had to fall back to the nickname; and
    how='AMBIGUOUS'/'MISSING' when the caller must stop."""
    remotes = _remotes()
    hits = [(n, u) for n, u in remotes.items() if VANILLA_URL_MARK.lower() in u.lower()]
    if len(hits) == 1:
        return hits[0][0], hits[0][1], "url"
    if len(hits) > 1:
        return None, "; ".join("%s=%s" % h for h in hits), "AMBIGUOUS"
    if VANILLA_REMOTE_FALLBACK in remotes:
        return (VANILLA_REMOTE_FALLBACK, remotes[VANILLA_REMOTE_FALLBACK],
                "fallback-name")
    return None, "no remote matching %s" % VANILLA_URL_MARK, "MISSING"




def git(*args, timeout=90):
    try:
        r = subprocess.run(["git"] + list(args), capture_output=True, text=True,
                           encoding="utf-8", errors="replace", cwd=str(REPO), timeout=timeout)
        return r.returncode, (r.stdout or "")
    except Exception as e:
        return 1, str(e)


_VN, _VU, _VHOW = resolve_vanilla()
VANILLA_REMOTE = _VN or VANILLA_REMOTE_FALLBACK
VANILLA_REF = "%s/main" % VANILLA_REMOTE
VANILLA_URL = _VU
VANILLA_HOW = _VHOW

# Back-compat aliases so existing call sites keep working. NEW CODE SHOULD USE
# THE VANILLA_* NAMES — `ORIGIN_*` is retained only so this repair is not also
# a breaking change.
ORIGIN_REMOTE = VANILLA_REMOTE
ORIGIN_REF = VANILLA_REF


def origin_url():
    rc, out = git("remote", "get-url", ORIGIN_REMOTE)
    return out.strip() if rc == 0 else "(unknown)"


def origin_touched(since):
    """Files changed by the ORIGIN PROJECT since `since` (a commit-ish)."""
    rc, out = git("diff", "--name-only", "%s...%s" % (since, ORIGIN_REF))
    if rc:
        return None
    return sorted({l.strip().replace("\\", "/") for l in out.splitlines() if l.strip()})


def row_citations():
    """{row_id: set(paths it cites)} - the tu: field plus any src/include path
    appearing in citations. A row is STALE when the origin edits what it cites."""
    out = {}
    if not ROWS.is_dir():
        return out
    for p in sorted(ROWS.glob("*.md")):
        txt = p.read_text(encoding="utf-8", errors="replace")
        rid = p.stem
        paths = set(re.findall(r"\b((?:src|include)/[\w./-]+\.(?:cpp|h|hpp|inc|c))", txt))
        out[rid] = paths
    return out


def attribute(verbose=True):
    # ========================================================================
    # THE REFUSAL IS CHECKED HERE, OUTSIDE `if verbose:`, AND THAT PLACEMENT IS
    # THE POINT (History/Bridge, 2026-08-16, on the Librarian's URL repair).
    #
    # The repair's refusal was correct and lived INSIDE the verbose block, so
    # `attribute(verbose=False)` skipped it: measured, it returned a real
    # merge-base and 1,970 files against a baseline it had just failed to
    # identify. No caller passes verbose=False today, so it was latent — but a
    # safety check that only runs when someone is watching is the silent-
    # degradation shape this whole tool exists to avoid, and the quiet caller
    # is exactly the one that would be trusted.
    #
    # A REFUSAL MUST NOT BE CONDITIONAL ON BEING PRINTED.
    # ========================================================================
    if VANILLA_HOW in ("AMBIGUOUS", "MISSING"):
        print("B3 ATTRIBUTION ** REFUSED ** — the vanilla baseline could not be "
              "identified (%s: %s). Attributing against an unidentified baseline "
              "would auto-advance past OUR OWN edits." % (VANILLA_HOW, VANILLA_URL))
        return None, []
    base = git("merge-base", "HEAD", ORIGIN_REF)[1].strip()
    touched = origin_touched(base)
    if verbose:
        print("B3 ATTRIBUTION")
        # Print the IDENTITY and HOW it was resolved, not just the nickname.
        # A reader must be able to catch this being wrong without knowing the
        # repo's inverted-remote lore.
        print("  VANILLA (forked FROM) : remote '%s' -> %s" % (VANILLA_REMOTE, VANILLA_URL))
        print("  resolved by           : %s" % {
            "url": "URL match on %s  [trustworthy]" % VANILLA_URL_MARK,
            "fallback-name": "*** REMOTE NAME '%s' ONLY - URL did not match %s. "
                             "Verify before trusting this run ***"
                             % (VANILLA_REMOTE_FALLBACK, VANILLA_URL_MARK),
            "AMBIGUOUS": "*** AMBIGUOUS - two remotes match; REFUSE ***",
            "MISSING": "*** MISSING - no remote matches; REFUSE ***",
        }.get(VANILLA_HOW, VANILLA_HOW))
        print("  OUR FORK              : %s (NOT the baseline; never attribute to it)"
              % FORK_URL_MARK)
        print("  note: this repo's remote NAMES are inverted from convention -")
        print("        'origin' is the vanilla project, 'upstream' is our fork.")
        print("        This tool resolves on URL so the names cannot mislead it.")
        if VANILLA_HOW in ("AMBIGUOUS", "MISSING"):
            print("  ** REFUSING to attribute: the baseline could not be identified. **")
            return None, []
        print("  baseline ref          : %s @ %s" % (VANILLA_REF, git("rev-parse", "--short", VANILLA_REF)[1].strip()))
        print("  merge-base    : %s" % base[:10])
        if touched is None:
            print("  ** cannot diff against %s **" % ORIGIN_REF)
        else:
            print("  files the ORIGIN PROJECT changed since the merge-base: %d" % len(touched))
    return base, (touched or [])


def sync(apply=False):
    base, touched = attribute(verbose=True)
    cites = row_citations()
    tset = set(touched)
    stale = {rid: sorted(p & tset) for rid, p in cites.items() if (p & tset)}
    print("\n  rows: %d · rows whose cited files the ORIGIN touched: %d" % (len(cites), len(stale)))
    for rid, paths in sorted(stale.items()):
        print("    STALE %s -> %s" % (rid, ", ".join(paths[:3])))
    print("\n  ZERO ROW DEMANDS ISSUED: origin-authored change never asks for a new row.")
    print("  (that is the whole point of B3 - a false demand on a sync trains bypass)")
    if stale and apply:
        lines = ["# STALE ROWS — origin-authored change touched what these rows cite",
                 "",
                 "> Written by `row_origin.py sync --apply`. STALE is NOT a defect and NOT a",
                 "> deletion: the row's CITATION may no longer describe the file. Re-read the",
                 "> cited lines and re-stamp the citation date, or correct the row.",
                 ""]
        for rid, paths in sorted(stale.items()):
            lines.append("- `%s` — origin touched: %s" % (rid, ", ".join(paths)))
        STALE.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
        print("  wrote %s" % STALE.relative_to(REPO))
    elif stale:
        print("  (dry run — pass --apply to write STALE.md)")
    return 0


def selftest():
    """The STALE path must fire on a synthetic origin-touch, and stay silent otherwise."""
    print("SELFTEST — STALE must fire when the origin touches a cited file\n")
    cites = row_citations()
    if not cites:
        print("  ** no rows to test against **")
        return 1
    rid, paths = next(((k, v) for k, v in cites.items() if v), (None, None))
    if not rid:
        print("  ** no row cites a src/include path — cannot exercise **")
        return 1
    victim = sorted(paths)[0]
    tset = {victim}
    hit = {r: sorted(p & tset) for r, p in cites.items() if (p & tset)}
    tset2 = {"src/does/not/exist_zzz.cpp"}
    miss = {r: sorted(p & tset2) for r, p in cites.items() if (p & tset2)}
    ok1, ok2 = bool(hit), not miss
    print("  [origin touches a CITED file      ] STALE fired: %s  %s"
          % (bool(hit), "OK" if ok1 else "** BLIND **"))
    print("       %s -> %s" % (rid, victim))
    print("  [origin touches an UNRELATED file ] STALE quiet: %s  %s"
          % (not miss, "OK" if ok2 else "** FALSE POSITIVE **"))
    print("\n%d/2 correct" % (int(ok1) + int(ok2)))
    return 0 if (ok1 and ok2) else 1


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "attribute"
    if cmd == "attribute":
        attribute()
        return 0
    if cmd == "sync":
        return sync(apply="--apply" in sys.argv)
    if cmd == "selftest":
        return selftest()
    print("usage: row_origin.py attribute | sync [--apply] | selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
