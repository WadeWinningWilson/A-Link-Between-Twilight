#!/usr/bin/env python3
# ============================================================================
# aurora_patch_check.py - BYTE-VERIFY a tracked aurora patch, and check
# whether upstream has since absorbed it (the user's redundancy requirement).
#
# ORDERED 2026-08-22: "the code goes FIRST into a patcher... that patcher is
# then APPLIED to aurora - making sure to not copy over with redundant code
# if aurora later accepts the patcher code upstream." AURORA-PATCH-LEDGER.md
# is the row-first record; this is the instrument that keeps the ledger
# honest instead of hand-verified once and trusted forever.
#
# FIVE OPERATIONS, EACH A DISTINCT GIT-APPLY QUESTION AGAINST A REF:
#
#   byte-verify   apply the patch to a SCRATCH CHECKOUT of its stated pin,
#                 hash-compare every touched file against the intended
#                 target tree. Proves the .patch FILE is what it claims to
#                 be - the tracked artifact matches reality.
#
#   redundancy    apply the patch REVERSE (`git apply -R --check`) against a
#                 FRESH FETCH of upstream's tip. If the reverse-apply
#                 succeeds, upstream's current content already equals the
#                 patch's "after" state - the patch is REDUNDANT, applying
#                 it (or leaving it applied) duplicates what upstream now
#                 carries natively. If the forward check succeeds instead,
#                 upstream has NOT absorbed it. If NEITHER succeeds, upstream
#                 has diverged in the affected region - not a clean redundant/
#                 needed answer, flagged for a human to read.
#
#   compat        does the patch apply to a USER's tagged dusklight release,
#                 resolving each release's extern/aurora pin from the
#                 superproject tree. Answers "will this reach a real user".
#
#   apply /       INSTALL and UNINSTALL. **These two are the ONLY operations
#   unapply       that write to a real repo** - stripping a patch out of a
#                 scratch clone helps nobody; the point is to toggle it in
#                 and out of the build you actually run.
#
# THE READ-ONLY OPERATIONS NEVER TOUCH A LIVE REPO. verify/redundancy/compat
# run entirely inside throwaway clones, cloned fresh and discarded. That rule
# exists because building AURORA-PATCH-0001 without it briefly mutated the
# real dusklight-main aurora submodule's working tree (recorded in
# AURORA-PATCH-LEDGER.md's "near-miss" section).
#
# apply/unapply cannot have that isolation and so carry it as GUARDS instead:
# dry-run by default, refuse on a dirty tree, pre-check before touching
# anything, and hash-verify the result against a named ref afterward.
#
# Usage:
#   aurora_patch_check.py verify <patch> <local-aurora-repo> <pin>
#     -> byte-compares patch(pin) against <local-aurora-repo>'s CURRENT tree,
#        per file, using ONLY scratch clones.
#   aurora_patch_check.py redundancy <patch> <local-aurora-repo> [--ref REF]
#     -> fetches origin from a scratch clone of <local-aurora-repo> (which
#        must have 'origin' pointed at the real upstream), checks the patch
#        against REF (default origin/main) after the fetch.
#   aurora_patch_check.py compat <patch> <aurora-repo> <dusklight-repo> --refs v1.4.1,v1.4.0
#   aurora_patch_check.py apply   <patch> <aurora-repo> [--target REF] [--write]
#   aurora_patch_check.py unapply <patch> <aurora-repo> [--base REF] [--write]
#     -> apply/unapply are DRY-RUN unless --write is passed.
#   aurora_patch_check.py --control
# Exit 0 ran/control-passed - 1 control failed / refused - 2 bad input.
# ============================================================================
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


def git(repo, *args, check=True):
    r = subprocess.run(["git", "-C", str(repo)] + list(args),
                       capture_output=True, text=True)
    if check and r.returncode != 0 and "--check" not in args and "-C" not in args[:1]:
        pass  # caller inspects returncode where it matters; no silent swallow
    return r


def scratch_clone(source_repo, tmproot):
    """A throwaway clone -- the ONLY thing this script ever git-operates on
    besides read-only inspection of the source. Never the live submodule."""
    dest = Path(tmproot) / "clone"
    r = subprocess.run(["git", "clone", "-q", str(source_repo), str(dest)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("clone failed: %s" % r.stderr)
    return dest


def file_hashes(repo, ref, paths):
    out = {}
    for p in paths:
        r = git(repo, "rev-parse", "%s:%s" % (ref, p), check=False)
        out[p] = r.stdout.strip() if r.returncode == 0 else None
    return out


def touched_paths(patch_text):
    paths = []
    for line in patch_text.splitlines():
        if line.startswith("+++ b/"):
            paths.append(line[6:])
    return paths


def verify(patch_path, source_repo, pin, target_ref="HEAD"):
    """Byte-verify: patch applied to a scratch checkout of `pin` must equal
    `target_ref`'s content in the caller's repo, file by file."""
    patch_text = Path(patch_path).read_text(encoding="utf-8", errors="replace")
    paths = touched_paths(patch_text)
    if not paths:
        return {"ok": False, "reason": "no +++ b/ paths found in patch", "files": {}}
    with tempfile.TemporaryDirectory() as tmp:
        clone = scratch_clone(source_repo, tmp)
        git(clone, "checkout", "-q", pin)
        chk = git(clone, "apply", "--check", str(patch_path))
        if chk.returncode != 0:
            return {"ok": False, "reason": "apply --check failed on pin: %s" % chk.stderr.strip(),
                    "files": {}}
        ap = git(clone, "apply", str(patch_path))
        if ap.returncode != 0:
            return {"ok": False, "reason": "apply failed after check passed: %s" % ap.stderr.strip(),
                    "files": {}}
        got = {}
        for p in paths:
            got[p] = git(clone, "hash-object", p, check=False).stdout.strip()
        want = file_hashes(source_repo, target_ref, paths)
        files = {p: {"applied": got.get(p), "target": want.get(p),
                     "match": got.get(p) == want.get(p) and got.get(p) is not None}
                 for p in paths}
        ok = all(f["match"] for f in files.values())
        return {"ok": ok, "reason": "" if ok else "hash mismatch", "files": files}


def redundancy(patch_path, source_repo, ref="origin/main"):
    """Fetch origin fresh in a scratch clone, then test the patch against ref
    both directions. Returns one of: REDUNDANT / NEEDED / DIVERGED.

    BUG FOUND AND FIXED 2026-08-22, by control: `git clone` points the NEW
    clone's `origin` remote at the PATH IT CLONED FROM, discarding whatever
    `origin` the source repo itself had configured. Left as-is, this fetched
    from the local source repo (which may carry the very patch under test)
    instead of the real upstream that repo's `origin` names -- silently
    reporting REDUNDANT for a patch that was never checked against upstream
    at all. Fix: read the source repo's OWN origin URL first, then repoint
    the scratch clone's origin at that URL before fetching."""
    real_origin = subprocess.run(["git", "-C", str(source_repo), "remote", "get-url", "origin"],
                                 capture_output=True, text=True).stdout.strip()
    if not real_origin:
        return {"verdict": "NO-ORIGIN",
                "detail": "%s has no 'origin' remote configured - cannot reach "
                          "upstream through it" % source_repo}
    with tempfile.TemporaryDirectory() as tmp:
        clone = scratch_clone(source_repo, tmp)
        git(clone, "remote", "set-url", "origin", real_origin)
        f = git(clone, "fetch", "origin")
        if f.returncode != 0:
            return {"verdict": "FETCH-FAILED", "detail": f.stderr.strip()}
        rc = git(clone, "checkout", "-q", ref, check=False)
        if rc.returncode != 0:
            return {"verdict": "REF-NOT-FOUND", "detail": "checkout %s failed" % ref}
        rev_check = git(clone, "apply", "--check", "-R", str(patch_path), check=False)
        if rev_check.returncode == 0:
            return {"verdict": "REDUNDANT",
                    "detail": "reverse-apply against %s (%s) succeeds: upstream's "
                              "current content already matches the patch's AFTER "
                              "state." % (ref, git(clone, "rev-parse", "--short", ref).stdout.strip())}
        fwd_check = git(clone, "apply", "--check", str(patch_path), check=False)
        if fwd_check.returncode == 0:
            return {"verdict": "NEEDED",
                    "detail": "forward-apply against %s succeeds cleanly: not yet "
                              "absorbed upstream." % ref}
        return {"verdict": "DIVERGED",
                "detail": "neither direction applies cleanly against %s - upstream "
                          "has changed the affected region independently. "
                          "MANUAL REVIEW, not an automatic verdict." % ref}


def dusklight_aurora_pins(dusklight_repo, refs):
    """Resolve extern/aurora's pinned commit for each dusklight-main ref
    (tag/branch/commit). Uses `git ls-tree` on the SUPERPROJECT -- the
    gitlink entry IS the submodule pin, no submodule checkout needed."""
    pins = {}
    for ref in refs:
        r = subprocess.run(["git", "-C", str(dusklight_repo), "ls-tree", ref, "extern/aurora"],
                           capture_output=True, text=True)
        if r.returncode != 0 or not r.stdout.strip():
            pins[ref] = None
            continue
        pins[ref] = r.stdout.split()[2]
    return pins


def compat(patch_path, aurora_repo, dusklight_repo, refs):
    """USER-COMPATIBILITY CHECK, ordered 2026-08-22: 'it must ALSO check the
    user's build... we want as much compatibility with different dusklight
    versions as possible.' Upstream redundancy answers 'has aurora absorbed
    this'; this answers a DIFFERENT question -- 'for a user on THIS
    dusklight build, does the tracked patch even apply, or does that
    version's own aurora pin already carry it, or has it diverged too far.'

    Per dusklight-main ref: resolve its extern/aurora GITLINK PIN (no
    submodule checkout needed - the pin is recorded directly in the
    superproject's tree), then forward/reverse-check the patch against that
    pin in a scratch clone of `aurora_repo` (never the live one)."""
    pins = dusklight_aurora_pins(dusklight_repo, refs)
    results = {}
    with tempfile.TemporaryDirectory() as tmp:
        clone = scratch_clone(aurora_repo, tmp)
        # Best-effort: older release pins may not be in a fresh clone's
        # reachable history yet, even if `aurora_repo`'s own `origin` has
        # them (real dusklight releases fetch/tag aurora at points a plain
        # clone of the current checkout may not carry). Failure here is not
        # fatal -- each ref still gets its own PIN-UNRESOLVED if genuinely
        # absent after this.
        git(clone, "fetch", "--tags", "origin", check=False)
        for ref, pin in pins.items():
            if pin is None:
                results[ref] = {"pin": None, "verdict": "NO-EXTERN-AURORA",
                                "detail": "ref has no extern/aurora gitlink"}
                continue
            co = git(clone, "checkout", "-q", "-f", pin, check=False)
            if co.returncode != 0:
                results[ref] = {"pin": pin, "verdict": "PIN-UNRESOLVED",
                                "detail": "pin %s not present in the aurora repo "
                                          "(fetch needed)" % pin[:10]}
                continue
            rev = git(clone, "apply", "--check", "-R", str(patch_path), check=False)
            if rev.returncode == 0:
                results[ref] = {"pin": pin, "verdict": "ALREADY-HAS-IT",
                                "detail": "reverse-apply succeeds: this build's aurora "
                                          "already matches the patch's AFTER state"}
                continue
            fwd = git(clone, "apply", "--check", str(patch_path), check=False)
            if fwd.returncode == 0:
                results[ref] = {"pin": pin, "verdict": "COMPATIBLE",
                                "detail": "forward-apply succeeds cleanly against this "
                                          "build's aurora pin"}
                continue
            results[ref] = {"pin": pin, "verdict": "INCOMPATIBLE",
                            "detail": "neither direction applies - this build's aurora "
                                      "has diverged from the patch's assumptions"}
    return results


def apply_patch(patch_path, repo, target_ref=None, write=False):
    """INSTALL: apply a tracked patch INTO a real working tree.

    Mirror of unapply() with the same guards -- dry-run default, dirty-tree
    refusal, pre-check, post-verify. Added 2026-08-22 after the first live
    round-trip test had to fall back to raw `git apply` for the forward
    direction: install and uninstall must have the SAME safety shape, or the
    guarded half is only half a system."""
    return _toggle(patch_path, repo, reverse=False, verify_ref=target_ref, write=write)


def unapply(patch_path, repo, base_ref=None, write=False):
    return _toggle(patch_path, repo, reverse=True, verify_ref=base_ref, write=write)


def _toggle(patch_path, repo, reverse, verify_ref=None, write=False):
    """STRIP A TRACKED PATCH BACK OUT of a real working tree.

    Ordered 2026-08-22: "since we're tracking exactly what we're putting INTO
    a user build, it would be helpful to be able to strip it OUT as well so we
    don't have to unpatch/download a stock/other every time."

    *** THIS IS THE ONE COMMAND IN THIS FILE THAT WRITES TO A REAL REPO. ***
    Every other command operates exclusively on throwaway scratch clones. This
    one cannot -- stripping a patch out of a scratch clone helps nobody; the
    point is to toggle it out of the build you actually run. So it carries the
    guards the others get for free from isolation:

      · DRY-RUN BY DEFAULT. Reports what would happen and changes nothing
        unless `--write` is passed explicitly.
      · REFUSES ON A DIRTY TREE. If the target has uncommitted changes, an
        unapply would tangle them with the reversal and there would be no way
        to tell afterward which edit came from where. A dirty tree is a STOP,
        not a warning.
      · VERIFIES THE PATCH IS ACTUALLY APPLIED FIRST (`git apply --check -R`).
        A partially-applied or absent patch fails here rather than producing
        a half-reversed tree.
      · VERIFIES THE RESULT when `--base` is given: hash-compares every
        touched file against the base ref, so "it reversed" is proven rather
        than assumed. This is the same standard `verify` holds the forward
        direction to.

    Foundry mutated a live submodule by accident once this session (recorded
    in AURORA-PATCH-LEDGER.md's near-miss section). Every guard above exists
    because of that, not in spite of it."""
    paths = touched_paths(Path(patch_path).read_text(encoding="utf-8", errors="replace"))
    if not paths:
        return {"ok": False, "stage": "parse", "detail": "no +++ b/ paths in patch"}

    dirty = subprocess.run(["git", "-C", str(repo), "status", "--porcelain"],
                           capture_output=True, text=True).stdout.strip()
    if dirty:
        return {"ok": False, "stage": "dirty-tree",
                "detail": "target has uncommitted changes -- refusing. An unapply "
                          "would tangle them with the reversal:\n    "
                          + "\n    ".join(dirty.splitlines()[:8])}

    rflag = ["-R"] if reverse else []
    chk = git(repo, "apply", "--check", *rflag, str(patch_path), check=False)
    if chk.returncode != 0:
        return {"ok": False,
                "stage": "not-applied" if reverse else "will-not-apply",
                "detail": ("reverse-apply check FAILED -- the patch is not "
                           "cleanly applied to this tree (absent, partial, or "
                           "drifted)."
                           if reverse else
                           "apply check FAILED -- the patch does not fit this "
                           "tree. Most often that means it is ALREADY APPLIED; "
                           "it can also mean drift or a wrong base.")
                          + " Nothing changed.\n    " + chk.stderr.strip()[:400]}

    if not write:
        return {"ok": True, "stage": "dry-run",
                "detail": ("patch IS cleanly applied and WOULD reverse out of %d "
                           "file(s)." if reverse else
                           "patch WOULD apply cleanly into %d file(s).")
                          % len(paths)
                          + " Nothing changed -- pass --write to do it.",
                "files": paths}

    ap = git(repo, "apply", *rflag, str(patch_path), check=False)
    if ap.returncode != 0:
        return {"ok": False, "stage": "apply-failed",
                "detail": "%s failed AFTER its own check passed: %s"
                          % ("reverse-apply" if reverse else "apply",
                             ap.stderr.strip()[:400])}

    result = {"ok": True, "stage": "unapplied" if reverse else "applied",
              "files": paths,
              "detail": "patch %s %d file(s)"
                        % ("reversed out of" if reverse else "applied into", len(paths))}
    if verify_ref:
        mismatches = []
        for p in paths:
            got = git(repo, "hash-object", p, check=False).stdout.strip()
            want = git(repo, "rev-parse", "%s:%s" % (verify_ref, p), check=False).stdout.strip()
            if got != want or not got:
                mismatches.append(p)
        result["verified_against"] = verify_ref
        result["mismatches"] = mismatches
        if mismatches:
            result["ok"] = False
            result["detail"] += (" -- but %d file(s) do NOT match %s afterward: %s. "
                                 "The tree is NOT at the expected state."
                                 % (len(mismatches), verify_ref, ", ".join(mismatches)))
        else:
            result["detail"] += (" and ALL %d verified byte-identical to %s -- "
                                 "the tree is provably at that state."
                                 % (len(paths), verify_ref))
    return result


def control():
    """Build two tiny repos with git, entirely offline: one where a
    'downstream' patch is genuinely needed, one where 'upstream' already
    carries the same change (simulating post-absorption), and one where
    upstream diverged elsewhere. All three must be told apart correctly."""
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        base = tmp / "base"
        base.mkdir()
        subprocess.run(["git", "init", "-q", str(base)], check=True)
        subprocess.run(["git", "-C", str(base), "config", "user.email", "t@t"], check=True)
        subprocess.run(["git", "-C", str(base), "config", "user.name", "t"], check=True)
        (base / "f.txt").write_text("line1\nline2\nline3\n", encoding="utf-8")
        subprocess.run(["git", "-C", str(base), "add", "."], check=True)
        subprocess.run(["git", "-C", str(base), "commit", "-q", "-m", "pin"], check=True)
        pin = subprocess.run(["git", "-C", str(base), "rev-parse", "HEAD"],
                             capture_output=True, text=True).stdout.strip()

        downstream = tmp / "downstream"
        subprocess.run(["git", "clone", "-q", str(base), str(downstream)], check=True)
        (downstream / "f.txt").write_text("line1\nPATCHED\nline3\n", encoding="utf-8")
        subprocess.run(["git", "-C", str(downstream), "add", "."], check=True)
        subprocess.run(["git", "-C", str(downstream), "config", "user.email", "t@t"], check=True)
        subprocess.run(["git", "-C", str(downstream), "config", "user.name", "t"], check=True)
        subprocess.run(["git", "-C", str(downstream), "commit", "-q", "-m", "the fix"], check=True)
        patch = tmp / "the.patch"
        d = subprocess.run(["git", "-C", str(downstream), "diff", pin, "HEAD"],
                           capture_output=True, text=True).stdout
        patch.write_text(d, encoding="utf-8")

        print("=== byte-verify: patch(pin) must equal downstream's HEAD ===")
        v = verify(patch, downstream, pin, "HEAD")
        print("  verify ok=%s" % v["ok"])

        # "upstream" repo used as the fetch source for three redundancy cases
        for case, content, expect in (
            # "needed": upstream at the PIN's content, unchanged - the clone
            # of `base` already IS this; no write/commit needed, and trying
            # to commit a no-op diff is itself the bug this fix removes.
            ("needed", None, "NEEDED"),
            ("redundant", "line1\nPATCHED\nline3\n", "REDUNDANT"),
            ("diverged", "line1\nline2\nDIFFERENT\n", "DIVERGED"),
        ):
            up = tmp / ("upstream_%s" % case)
            subprocess.run(["git", "clone", "-q", str(base), str(up)], check=True)
            if content is not None:
                (up / "f.txt").write_text(content, encoding="utf-8")
                subprocess.run(["git", "-C", str(up), "add", "."], check=True)
                subprocess.run(["git", "-C", str(up), "config", "user.email", "t@t"], check=True)
                subprocess.run(["git", "-C", str(up), "config", "user.name", "t"], check=True)
                subprocess.run(["git", "-C", str(up), "commit", "-q", "-m", case], check=True)
            src = tmp / ("src_for_%s" % case)
            subprocess.run(["git", "clone", "-q", str(downstream), str(src)], check=True)
            subprocess.run(["git", "-C", str(src), "remote", "set-url", "origin", str(up)], check=True)
            up_branch = subprocess.run(["git", "-C", str(up), "branch", "--show-current"],
                                       capture_output=True, text=True).stdout.strip()
            r = redundancy(patch, src, "origin/%s" % up_branch)
            print("  case=%-10s verdict=%-10s expect=%-10s %s" % (
                case, r["verdict"], expect, "OK" if r["verdict"] == expect else "*** WRONG ***"))
            if r["verdict"] != expect:
                print("  CONTROL FAILED")
                return 1

        if not v["ok"]:
            print("  CONTROL FAILED (byte-verify)")
            return 1

        print("=== compat: a synthetic dusklight superproject with 4 tagged 'releases' ===")
        dusk = tmp / "dusklight_super"
        subprocess.run(["git", "init", "-q", str(dusk)], check=True)
        subprocess.run(["git", "-C", str(dusk), "config", "user.email", "t@t"], check=True)
        subprocess.run(["git", "-C", str(dusk), "config", "user.name", "t"], check=True)
        # four releases, each pointing extern/aurora (a gitlink, faked as a
        # plain file holding the pin sha - real dusklight uses an actual
        # submodule gitlink, but ls-tree's 3rd column is read identically
        # either way for this script's purposes) at one of the four aurora
        # states already built above: pin (needed), the redundant-content
        # commit, the diverged commit, and a not-yet-fetched sha.
        pin_up = subprocess.run(["git", "-C", str(tmp / "upstream_needed"), "rev-parse", "HEAD"],
                                capture_output=True, text=True).stdout.strip()
        red_up = subprocess.run(["git", "-C", str(tmp / "upstream_redundant"), "rev-parse", "HEAD"],
                                capture_output=True, text=True).stdout.strip()
        div_up = subprocess.run(["git", "-C", str(tmp / "upstream_diverged"), "rev-parse", "HEAD"],
                                capture_output=True, text=True).stdout.strip()
        fake_sha = "f" * 40
        releases = {"v1-needs-it": pin_up, "v2-already-has-it": red_up,
                   "v3-diverged": div_up, "v4-unfetched": fake_sha}
        for tag, sha in releases.items():
            (dusk / "extern").mkdir(exist_ok=True)
            subprocess.run(["git", "-C", str(dusk), "update-index", "--add", "--cacheinfo",
                           "160000,%s,extern/aurora" % sha], check=True)
            subprocess.run(["git", "-C", str(dusk), "commit", "-q", "--allow-empty",
                           "-m", tag], check=True)
            subprocess.run(["git", "-C", str(dusk), "tag", tag], check=True)
        expect_compat = {"v1-needs-it": "COMPATIBLE", "v2-already-has-it": "ALREADY-HAS-IT",
                         "v3-diverged": "INCOMPATIBLE", "v4-unfetched": "PIN-UNRESOLVED"}
        # `downstream` alone does not carry red_up/div_up's commits (they are
        # local-only to their own repos, made independently) -- exactly the
        # real-world shape of "a release's aurora pin lives in history this
        # checkout has not fetched." Build a combined repo the way the real
        # `dusklight-main/extern/aurora` already has all its own release
        # history: multiple remotes, fetched.
        combined = tmp / "aurora_combined"
        subprocess.run(["git", "clone", "-q", str(downstream), str(combined)], check=True)
        for name, src in (("un", tmp / "upstream_needed"), ("ur", tmp / "upstream_redundant"),
                          ("ud", tmp / "upstream_diverged")):
            subprocess.run(["git", "-C", str(combined), "remote", "add", name, str(src)], check=True)
            subprocess.run(["git", "-C", str(combined), "fetch", "-q", name], check=True)
        cr = compat(patch, combined, dusk, list(releases.keys()))
        all_ok = True
        for tag, exp in expect_compat.items():
            got = cr[tag]["verdict"]
            print("  %-18s verdict=%-16s expect=%-16s %s" % (
                tag, got, exp, "OK" if got == exp else "*** WRONG ***"))
            if got != exp:
                all_ok = False
        if not all_ok:
            print("  CONTROL FAILED (compat)")
            return 1

        print("=== unapply: dirty-refusal, not-applied, dry-run, and verified write ===")
        # a repo WITH the patch applied = downstream's HEAD (that IS the patched state)
        ua = tmp / "unapply_target"
        subprocess.run(["git", "clone", "-q", str(downstream), str(ua)], check=True)

        # (a) dirty tree must REFUSE
        (ua / "scratch_dirt.txt").write_text("dirt", encoding="utf-8")
        r_dirty = unapply(patch, ua, None, False)
        (ua / "scratch_dirt.txt").unlink()

        # (b) a repo WITHOUT the patch (at pin) must report not-applied
        clean_pin = tmp / "unapply_pin"
        subprocess.run(["git", "clone", "-q", str(base), str(clean_pin)], check=True)
        r_absent = unapply(patch, clean_pin, None, False)

        # (c) dry-run on the applied tree: ok, and MUST NOT change anything
        before = (ua / "f.txt").read_text(encoding="utf-8")
        r_dry = unapply(patch, ua, None, False)
        unchanged = (ua / "f.txt").read_text(encoding="utf-8") == before

        # (d) real write, verified against the base ref
        r_write = unapply(patch, ua, pin, True)
        reverted = (ua / "f.txt").read_text(encoding="utf-8") == "line1\nline2\nline3\n"

        checks = [
            ("dirty refused", (not r_dirty["ok"]) and r_dirty["stage"] == "dirty-tree"),
            ("absent detected", (not r_absent["ok"]) and r_absent["stage"] == "not-applied"),
            ("dry-run ok", r_dry["ok"] and r_dry["stage"] == "dry-run"),
            ("dry-run changed nothing", unchanged),
            ("write unapplied", r_write["ok"] and r_write["stage"] == "unapplied"),
            ("write verified vs base", r_write.get("mismatches") == []),
            ("content actually reverted", reverted),
        ]
        for name, passed in checks:
            print("  %-28s %s" % (name, "OK" if passed else "*** WRONG ***"))
        if not all(p for _, p in checks):
            print("  CONTROL FAILED (unapply)")
            return 1

        print("CONTROL PASSED - byte-verify, three redundancy verdicts, four compat")
        print("verdicts, and all seven unapply guards correct.")
        return 0


def main():
    if "--control" in sys.argv:
        return control()
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    cmd = sys.argv[1]
    if cmd == "verify":
        patch, repo, pin = sys.argv[2], sys.argv[3], sys.argv[4]
        target = sys.argv[sys.argv.index("--target") + 1] if "--target" in sys.argv else "HEAD"
        r = verify(patch, repo, pin, target)
        print("BYTE-VERIFY: %s" % ("OK - patch(%s) == %s, file-for-file" % (pin, target) if r["ok"] else r["reason"]))
        for p, f in r["files"].items():
            print("  %-40s %s" % (p, "MATCH" if f["match"] else "MISMATCH (%s vs %s)" % (f["applied"], f["target"])))
        return 0 if r["ok"] else 1
    if cmd == "redundancy":
        patch, repo = sys.argv[2], sys.argv[3]
        ref = sys.argv[sys.argv.index("--ref") + 1] if "--ref" in sys.argv else "origin/main"
        r = redundancy(patch, repo, ref)
        print("REDUNDANCY vs %s: %s" % (ref, r["verdict"]))
        print("  %s" % r["detail"])
        return 0
    if cmd == "apply":
        patch, repo = sys.argv[2], sys.argv[3]
        target = sys.argv[sys.argv.index("--target") + 1] if "--target" in sys.argv else None
        write = "--write" in sys.argv
        r = apply_patch(patch, repo, target, write)
        print("APPLY [%s]: %s" % (r["stage"], "OK" if r["ok"] else "REFUSED/FAILED"))
        print("  %s" % r["detail"])
        if not write and r["ok"]:
            print("  (dry-run is the default -- this changed nothing)")
        return 0 if r["ok"] else 1
    if cmd == "unapply":
        patch, repo = sys.argv[2], sys.argv[3]
        base = sys.argv[sys.argv.index("--base") + 1] if "--base" in sys.argv else None
        write = "--write" in sys.argv
        r = unapply(patch, repo, base, write)
        print("UNAPPLY [%s]: %s" % (r["stage"], "OK" if r["ok"] else "REFUSED/FAILED"))
        print("  %s" % r["detail"])
        if not write and r["ok"]:
            print("  (dry-run is the default -- this changed nothing)")
        return 0 if r["ok"] else 1
    if cmd == "compat":
        patch, aurora_repo, dusklight_repo = sys.argv[2], sys.argv[3], sys.argv[4]
        refs = sys.argv[sys.argv.index("--refs") + 1].split(",") if "--refs" in sys.argv else ["HEAD"]
        r = compat(patch, aurora_repo, dusklight_repo, refs)
        print("COMPAT across %d dusklight ref(s):" % len(refs))
        for ref, info in r.items():
            print("  %-16s pin=%-12s %-16s %s" % (
                ref, (info["pin"] or "-")[:10], info["verdict"], info["detail"]))
        bad = [ref for ref, i in r.items() if i["verdict"] == "INCOMPATIBLE"]
        if bad:
            print("  INCOMPATIBLE with: %s" % ", ".join(bad))
        return 0
    print("unknown command: %s" % cmd)
    return 2


if __name__ == "__main__":
    sys.exit(main())
