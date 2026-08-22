#!/usr/bin/env python3
# ============================================================================
# decomp_sync.py — KEEP THE OFFICIAL LEDGER FRESH, WITHOUT A LANE REMEMBERING.
#
# THE HOLE THIS FILLS (user, 2026-08-16): `decomp_watch` compares the live
# ledger against a pinned baseline — but "live" means OUR LOCAL CHECKOUT of
# zeldaret/tww. Nobody pulls, nothing ever flips, and the watcher prints a
# clean run forever. **Silence that looks like success**, which is the failure
# this estate keeps re-finding. Worse, the fix cannot depend on a lane calling
# it: the whole reason this system was rebuilt today is that `rel_decomp.py`'s
# covenant sat in the tree, undocumented and uncited, and was nearly lost.
#
#   git fetch  -> how far behind is our checkout, in commits and days
#   ff-only    -> advance it, but ONLY when that is provably safe
#   then run decomp_watch check, so a sync that moves the ledger IMMEDIATELY
#   reports which TUs flipped and which tracker rows they touch
#
# SAFETY, because this MUTATES a reference tree other tools pin against:
#   · REFUSES if the donor worktree is dirty — local work is never clobbered.
#   · FAST-FORWARD ONLY. No merge, no rebase, no autostash. If history
#     diverged, it stops and says so rather than inventing a resolution.
#   · `status` and `fetch` never touch the worktree at all.
#   · `build/GZLE01` (dtk asm) is derived from the DOL and `orig/` is the
#     user's own extracted disc — neither is source-tracked, so a
#     fast-forward of source does not invalidate them.
#
#   decomp_sync.py status     how stale are we? (no network)
#   decomp_sync.py fetch      ask upstream; report behind-count. No worktree change.
#   decomp_sync.py sync       fetch + fast-forward + re-run decomp_watch check
#   decomp_sync.py selftest   the control
#
# Exit 1 when action is owed (behind, dirty, diverged, or flips found).
# ============================================================================
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
DONOR = Path("D:/XXXXXXX/WW DP")
UPSTREAM = "https://github.com/zeldaret/tww.git"


def git(*args, timeout=180):
    """(rc, stdout) — never raises; a sync tool that dies is a sync tool that
    silently stops keeping anything fresh."""
    try:
        p = subprocess.run(["git", "-C", str(DONOR)] + list(args),
                           capture_output=True, text=True, timeout=timeout)
        return p.returncode, (p.stdout or "") + (p.stderr or "")
    except Exception as e:
        return 1, "git failed: %s" % e


def guard():
    """None if safe to touch the worktree, else the reason it is not."""
    if not DONOR.is_dir():
        return "donor checkout not present at %s" % DONOR
    rc, out = git("rev-parse", "--is-inside-work-tree")
    if rc != 0 or "true" not in out:
        return "not a git worktree: %s" % out.strip()[:120]
    rc, out = git("status", "--short")
    if rc == 0 and out.strip():
        return ("worktree is DIRTY (%d line(s)) — refusing to touch it; local "
                "work is never clobbered by an automatic sync"
                % len(out.strip().splitlines()))
    rc, out = git("remote", "get-url", "origin")
    if rc == 0 and "zeldaret/tww" not in out:
        return "origin is not zeldaret/tww (%s)" % out.strip()[:80]
    return None


def head_info():
    rc, out = git("log", "-1", "--format=%h|%ad|%s", "--date=short")
    return out.strip() if rc == 0 else "(unknown)"


def cmd_status():
    print("DECOMP SYNC — local checkout of the OFFICIAL ledger")
    print("  donor   : %s" % DONOR)
    print("  upstream: %s" % UPSTREAM)
    print("  HEAD    : %s" % head_info())
    g = guard()
    print("  safe to advance: %s" % ("YES" if g is None else "NO — " + g))
    rc, out = git("rev-list", "--count", "HEAD..origin/main")
    behind = out.strip() if rc == 0 and out.strip().isdigit() else "?"
    print("  behind origin/main by: %s commit(s)  [as of last fetch — run "
          "`fetch` for the live answer]" % behind)
    return 0 if g is None else 1


def cmd_fetch(quiet=False):
    # NO DIRTY-GUARD HERE, DELIBERATELY.
    #
    # `git fetch` writes refs and objects under .git/ and NEVER touches the
    # worktree, so a dirty tree cannot be clobbered by it. The guard used to
    # sit here too, and that made this command unrunnable during exactly the
    # period it matters: the donor worktree is dirty WHENEVER A LANE IS
    # MID-DECODE (found 2026-08-22 with Decoder's live yw1 edits + 5 scratch
    # files sitting in it). A safety rail that blocks the safe read is why
    # this check stopped being routine at all.
    #
    # `cmd_sync` keeps the guard: it runs `merge --ff-only`, which DOES touch
    # the worktree and can genuinely clobber local work.
    g = guard()
    if g is not None and not quiet:
        print("  NOTE: donor worktree is dirty (%s) — fetching anyway; fetch "
              "does not touch the worktree. `sync` still refuses." % g)
    if not quiet:
        print("fetching origin ...")
    rc, out = git("fetch", "origin", "--quiet")
    if rc != 0:
        print("  FETCH FAILED — this is UNKNOWN, not up-to-date: %s"
              % out.strip()[:200])
        return 1
    rc, out = git("rev-list", "--count", "HEAD..origin/main")
    behind = int(out.strip()) if rc == 0 and out.strip().isdigit() else -1
    if behind < 0:
        print("  could not count distance to origin/main — UNKNOWN")
        return 1
    print("  behind origin/main by %d commit(s)" % behind)

    # AHEAD-COUNT IS THE NUMBER THAT DECIDES, AND THIS ONLY REPORTED BEHIND.
    # "behind 12" sounds small and is small. What makes a rebase expensive is
    # how much of OUR work has to be replayed on top of it -- measured
    # 2026-08-22 at 346 local commits (Decoder's whole decode campaign) against
    # 12 upstream. Behind-only turns a compounding cost into a quiet one.
    rc, out = git("rev-list", "--count", "origin/main..HEAD")
    ahead = int(out.strip()) if rc == 0 and out.strip().isdigit() else -1
    if ahead > 0:
        print("  AHEAD of origin/main by %d commit(s) — local work upstream "
              "does not have" % ahead)
        if behind:
            print("  => DIVERGED. `merge --ff-only` refuses by construction, so "
                  "`sync` CANNOT clobber this work. A rebase is the only way "
                  "forward and it grows with the ahead-count.")
    if behind:
        rc, out = git("log", "-1", "--format=%h|%ad|%s", "--date=short",
                      "origin/main")
        print("  upstream HEAD: %s" % out.strip())
    return 1 if behind else 0


def cmd_sync():
    g = guard()
    if g is not None:
        print("REFUSING: %s" % g)
        return 1
    before = head_info()
    if cmd_fetch(quiet=True) == 0:
        print("  already current with origin/main — nothing to advance.")
        return 0

    rc, out = git("merge", "--ff-only", "origin/main")
    if rc != 0:
        print("  FAST-FORWARD REFUSED — history diverged, or local commits "
              "exist. Stopping rather than inventing a resolution:")
        print("   ", out.strip()[:220])
        return 1

    after = head_info()
    print("  ADVANCED")
    print("    was: %s" % before)
    print("    now: %s" % after)
    print("\n  The pinned baseline is now OLDER than the ledger, which is the "
          "point — running decomp_watch check:\n")

    sys.path.insert(0, str(HERE))
    try:
        import io as _io
        import decomp_watch as DW
        buf = _io.StringIO()
        real_out = sys.stdout
        sys.stdout = buf
        try:
            rc_watch = DW.cmd_check()
        finally:
            sys.stdout = real_out
        report = buf.getvalue()
        print(report)
        ping(before, after, report, rc_watch)
        return rc_watch
    except Exception as e:
        print("  decomp_watch could not run after sync: %s" % e)
        print("  The checkout MOVED and the flips are UNREPORTED — treat as "
              "action owed, not as clean.")
        return 1


def ping(before, after, report, rc_watch):
    """FILE A CALLS ROW — the surface every lane's monitor already watches.

    Deliberately NOT a new notification channel (user, 2026-08-16: "don't
    invent a new tool, just tie to an existing method"). `CALLS.md` is the
    lane-coordination surface, `file_row.py add` is the existing filer, and it
    carries its own byte audit, addressee lint and re-read verification — all
    of which a bespoke writer here would have to reinvent badly.

    An upstream advance is a RARE event, so one row per advance is signal, not
    noise. The row says what moved even when nothing flipped, because "we
    pulled and no status changed" is itself the thing a later reader needs in
    order to trust a clean `check`.
    """
    flipped = "NEWLY DECOMPILED: 0" not in report
    head = ("**UPSTREAM DECOMP MOVED AND OUR CHECKOUT WAS ADVANCED "
            "AUTOMATICALLY — %s**" %
            ("TUs FLIPPED, confirm-or-replace owed"
             if flipped else "no TU status changed"))
    body = (
        head + " · **This row was filed BY `decomp_sync.py`, not by a lane** "
        "— the whole point is that keeping the official ledger fresh cannot "
        "depend on someone remembering the tool exists. · **WAS:** `%s` · "
        "**NOW:** `%s` · **The ledger is `configure.py` in a pristine checkout "
        "of `zeldaret/tww` — the decomp team's own per-TU status, not anyone's "
        "annotation.** · **`decomp_watch check` said:** %s "
        "· **WHAT IS OWED IF TUs FLIPPED: our own decoding for them is DRAFT "
        "under `rel_decomp.py`'s covenant (*\"Never label a draft MATCH\"*) and "
        "must now be CONFIRMED AGAINST or REPLACED BY the official version.** "
        "That is the verbatim gate's addendum discharging itself. · **NOT "
        "AUTO-APPLIED:** no tracker row was rewritten — a copied verdict is a "
        "second roster that drifts (schema `never_stored`). A lane rules."
        % (before, after,
           # ONE LINE, ALWAYS. A row on this board is a LINE — every one of the
           # 368 human-filed rows satisfies that, and `calls_rotate.py` moves
           # rows PER LINE. My first ping embedded a fenced block and spanned
           # 22 lines; the rotator would have amputated 50 rows on its next run
           # (Librarian caught it before it fired). **A MACHINE FILER MUST BE
           # THE MOST CONSERVATIVE WRITER ON THE BOARD, NOT THE ONE THAT
           # INTRODUCES A SHAPE NO HUMAN PRODUCED** — the rotator was fixed to
           # handle blocks, and this still collapses, because relying on every
           # other tool to accommodate my novelty is the wrong direction.
           " · ".join(x.strip() for x in report.splitlines() if x.strip())
           or "(no output)"))

    row = "- [ ] ALL LANES | " + body + " | decomp_sync (automatic) | auto"
    tmp = HERE / "_decomp_sync_ping.md"
    try:
        import io as _io2
        _io2.open(tmp, "w", encoding="utf-8").write(row)
        p = subprocess.run(
            [sys.executable, str(HERE / "file_row.py"), "add", "--file", str(tmp)],
            capture_output=True, text=True, timeout=60)
        ok = "FILED OK" in (p.stdout or "")
        print("  PING: CALLS row %s" % ("filed" if ok else
                                        "FAILED TO FILE — " +
                                        (p.stdout or p.stderr or "")[:160]))
    except Exception as e:
        print("  PING FAILED: %s — the sync happened and NOBODY WAS TOLD; "
              "treat as action owed" % e)
    finally:
        try:
            tmp.unlink()
        except Exception:
            pass


def selftest():
    """A sync tool that cannot refuse is a sync tool that will clobber."""
    ok = True
    global DONOR
    real = DONOR

    DONOR = Path("D:/XXXXXXX/__definitely_not_here__")
    r = guard() is not None
    print("  [1] missing checkout refuses            : %s" % ("PASS" if r else "FAIL"))
    ok &= r

    DONOR = real
    g = guard()
    print("  [2] real checkout evaluated (no crash)  : %s"
          % ("PASS" if (g is None or isinstance(g, str)) else "FAIL"))

    # The one that matters: dirty must refuse. Proven against a temp repo so it
    # is exercised, not asserted.
    import tempfile
    t = Path(tempfile.mkdtemp(prefix="decomp_sync_ctl_"))
    subprocess.run(["git", "init", "-q", str(t)], capture_output=True)
    (t / "f.txt").write_text("dirty")
    DONOR = t
    dirty_refused = "DIRTY" in (guard() or "")
    print("  [3] DIRTY worktree refuses              : %s"
          % ("PASS" if dirty_refused else "FAIL"))
    ok &= dirty_refused

    DONOR = real
    print("  selftest: %s" % ("3/3 PASS" if ok else "FAILURES ABOVE"))
    return 0 if ok else 1


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "status":
        return cmd_status()
    if cmd == "fetch":
        return cmd_fetch()
    if cmd == "sync":
        return cmd_sync()
    if cmd in ("selftest", "--selftest"):
        return selftest()
    print(__doc__ or "")
    print("usage: decomp_sync.py status | fetch | sync | selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
