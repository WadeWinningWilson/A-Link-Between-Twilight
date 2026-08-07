#!/usr/bin/env python3
# ============================================================================
# b2d_control.py — the negative control for B2d's partition check (§568).
#
# WHY THIS EXISTS AS A FILE RATHER THAN A TRANSCRIPT
# §566 reported a fault injection and Foundry, correctly, would not take it on
# the strength of the report: they had confirmed B2d is STRONG and that it
# PASSES, but not that it can report the other value. §529 is the precedent —
# the fold detector passed four fixtures and was badly wrong — so "I ran a
# control" is not itself a control. A cited run is evidence only to whoever
# watched it. This is runnable by anyone, and it fails loudly if B2d ever stops
# being able to fail.
#
# WHAT IT CONTROLS
# B2d's invariant: every RPPN waypoint belongs to exactly one RPAT path, and
# each path's declared count equals the points resolving to it. The control
# mutates `read_paths()` in the extractor — the function that builds the
# association — and asserts the invariant REJECTS each mutation.
#
# Mutation testing rather than fixtures, because the association is a property
# of the real arcs. A fixture would only prove the checker works on data I
# authored to make it work.
#
# WHAT IT DOES NOT DO
# It does not write anything, and it does not modify the extractor on disk: the
# source is mutated in memory and exec'd into a scratch namespace.
#
# Usage: b2d_control.py            (exit 0 = every mutation was caught)
# ============================================================================
import os
import pathlib
import sys

# REPO is resolved from this file's location, which breaks the moment the
# harness is COPIED somewhere to be tested -- exactly what Foundry did, and the
# copy died with "extractor not found" before it ever reached the anchor check
# it was written to exercise. A control that only runs from one directory is a
# control most people will run wrong. `--repo` overrides it, and `--selftest`
# needs neither the repo nor the live install.
def _repo_from_argv():
    if "--repo" in sys.argv:
        return pathlib.Path(sys.argv[sys.argv.index("--repo") + 1]).resolve()
    return pathlib.Path(__file__).resolve().parents[2]


REPO = _repo_from_argv()
EXTRACTOR = REPO / "tools/ww_crew_restoration_skeleton/extract_amori_census.py"
LIVE = pathlib.Path(os.path.expandvars("%APPDATA%")) / (
    "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")

# (label, find, replace, what a MISS would mean in shipped output)
MUTATIONS = [
    ("drop the last path",
     "        paths.append((k, num, next_id, arg0, closed))",
     "        if k == pat_n - 1:\n            continue\n"
     "        paths.append((k, num, next_id, arg0, closed))",
     "a whole route's points ship orphaned while paths.csv looks populated"),
    ("off-by-one in the run length",
     "        for j in range(num):",
     "        for j in range(max(0, num - 1)):",
     "the last point of every route loses its membership"),
    ("off-by-one in the first index",
     "        for j in range(num):",
     "        for j in range(1, num + 1):",
     "every route shifted by one point — plausible, and wrong"),
    ("absolute offset instead of RPPN-relative",
     '        first = struct.unpack_from(">I", blob, e + 8)[0] // 0x10',
     '        first = struct.unpack_from(">I", blob, e + 8)[0]',
     "THE BUG §566 NEARLY SHIPPED: 38 of 67 paths mis-grouped"),
    ("all paths claim point 0",
     '        first = struct.unpack_from(">I", blob, e + 8)[0] // 0x10',
     "        first = 0",
     "routes overlap; points claimed many times and most claimed never"),
]


def load(src_text):
    ns = {}
    exec(compile(src_text, str(EXTRACTOR), "exec"), ns)
    return ns


def partition_holds(ns) -> bool:
    """B2d's invariant, evaluated over EVERY stage in the live arcs.

    Deliberately re-stated here rather than imported from lane_queue: a control
    that calls the thing it is controlling proves only that the call works.
    Agreement between the two is asserted separately, at the end.
    """
    read_paths, be32 = ns["read_paths"], ns["be32"]
    list_rarc, decomp = ns["list_rarc"], ns["maybe_decompress"]
    checked = 0
    for d in ("arcs", "arcs_lib"):
        if not (LIVE / d).is_dir():
            continue
        for arc in sorted((LIVE / d).glob("*.arc")):
            try:
                data = decomp(arc.read_bytes())
                members = list(list_rarc(data))
            except Exception:
                continue
            for name, off, size in members:
                if not name.endswith((".dzr", ".dzs")):
                    continue
                blob = data[off:off + size]
                if len(blob) < 8:
                    continue
                n = be32(blob, 0)
                if n <= 0 or n > 200 or 4 + n * 12 > len(blob):
                    continue
                counts = {}
                for i in range(n):
                    o = 4 + i * 12
                    counts[blob[o:o + 4].decode("ascii", "replace")] = be32(blob, o + 4)
                if "RPPN" not in counts:
                    continue
                checked += 1
                try:
                    paths, pnt_path = read_paths(blob)
                except Exception:
                    return False
                if set(pnt_path) != set(range(counts["RPPN"])):
                    return False
                for pid, num, *_ in paths:
                    if sum(1 for v in pnt_path.values() if v[0] == pid) != num:
                        return False
    return checked > 0


def run_mutations(src, partition, mutations, loader=load):
    """Apply each mutation to `src` and require `partition` to REJECT it.

    Factored out so --selftest can drive the same loop with synthetic inputs.
    Returns (ok, baseline).
    """
    base = partition(loader(src))
    print(f"  {'PASS' if base is True else 'FAIL'}  baseline (unmutated) -> "
          f"{base}  (expect True)\n")
    if base is not True:
        print("  baseline FAILED — the control cannot distinguish anything "
              "until this passes.")
        return False, base
    ok = True
    for label, find, repl, consequence in mutations:
        if find not in src:
            # The mutation no longer applies -> the control has rotted against
            # the code. That is a FAILURE, not something to skip quietly: a
            # harness whose anchors stopped matching silently tests nothing.
            print(f"  FAIL  {label}\n        anchor not found — control is "
                  f"STALE against the extractor")
            ok = False
            continue
        got = partition(loader(src.replace(find, repl, 1)))
        caught = got is False
        ok &= caught
        print(f"  {'PASS' if caught else 'FAIL'}  {label} -> {got}  (expect False)")
        if consequence:
            print(f"        if missed: {consequence}")
    return ok, base


def selftest() -> int:
    """Verify the HARNESS's own failure modes. Needs no repo and no arcs.

    This is the leg Foundry could not attest: their copy could not resolve the
    extractor, so it exited before testing rot-detection, and the exit code they
    saw was `tail`'s rather than Python's. Both are fair traps, and the second
    is why this prints its verdict as text as well as returning a code.

    Fixtures ARE right here, unlike in the main control: this exercises the
    harness's plumbing, not a property of the arcs.
    """
    print("B2D CONTROL — SELFTEST (harness plumbing; no repo or arcs needed)\n")
    ok = True

    def check(name, got, want):
        nonlocal ok
        good = got is want
        ok &= good
        print(f"  {'PASS' if good else 'FAIL'}  {name}: {got} (expect {want})")
        return good

    ident = lambda s: {"v": s}          # "loader"
    holds = lambda ns: ns["v"] == "A"   # "partition"

    print("  [1] a mutation that genuinely changes behaviour must be CAUGHT")
    got, _ = run_mutations("A", holds, [("flip A->B", "A", "B", "")], loader=ident)
    check("caught", got, True)

    print("\n  [2] an anchor that no longer matches must FAIL, not be skipped")
    got, _ = run_mutations("A", holds, [("stale", "NOT_IN_SOURCE", "x", "")], loader=ident)
    check("reported stale", got, False)

    print("\n  [3] a checker that can never say False must FAIL "
          "(the always-passes trap — §529's fold detector)")
    blind = lambda ns: True
    got, _ = run_mutations("A", blind, [("flip A->B", "A", "B", "")], loader=ident)
    check("blind checker surfaced", got, False)

    print("\n  [4] a broken baseline must abort rather than score mutations")
    got, base = run_mutations("Z", holds, [("any", "Z", "Y", "")], loader=ident)
    check("aborted on bad baseline", got, False)

    print("\n  [5] missing inputs must report NOT RUN, never a pass")
    # Point the real run at a path that cannot exist and confirm it refuses.
    # This called main() once, which dispatches on --selftest and so recursed
    # forever the moment the extractor was genuinely absent -- i.e. exactly in
    # the copied-harness case this flag exists to serve. Found by running the
    # copy, which is the reason to test from where the user will actually be.
    global EXTRACTOR
    real_ext = EXTRACTOR
    EXTRACTOR = pathlib.Path("no/such/extractor.py")
    try:
        check("extractor absent -> non-zero", run_real() != 0, True)
    finally:
        EXTRACTOR = real_ext

    print(f"\n  SELFTEST VERDICT: {'ALL PASS' if ok else 'FAILURE'}   "
          f"(exit {0 if ok else 1} — read THIS line, not a piped exit code)")
    return 0 if ok else 1


def main() -> int:
    # Dispatch only. Kept free of the control's own logic so selftest can drive
    # `run_real()` directly instead of re-entering the dispatcher.
    if "--selftest" in sys.argv:
        return selftest()
    return run_real()


def run_real() -> int:
    if not EXTRACTOR.is_file():
        print(f"B2D CONTROL: NOT RUN — extractor not found at {EXTRACTOR}. "
              f"This is UNKNOWN, not a pass (№31-C).\n"
              f"  If you copied this harness, pass --repo <dusklight-root>, "
              f"or run --selftest which needs neither.")
        return 1
    if not LIVE.is_dir():
        # Cannot run -> say so. An uncontrolled control must not read as a pass.
        print("B2D CONTROL: NOT RUN — live install absent. This is UNKNOWN, "
              "not a pass (№31-C).")
        return 1
    src = EXTRACTOR.read_text(encoding="utf-8")

    print("B2D CONTROL — mutation testing the RPAT/RPPN partition\n")
    ok, base = run_mutations(src, partition_holds, MUTATIONS)
    if base is not True:
        return 1

    # The control implements the invariant independently; assert the registered
    # test agrees with it on the real data, so passing here means passing there.
    print()
    try:
        sys.path.insert(0, str(REPO / "tools/foundry"))
        import lane_queue
        reg = lane_queue._b2d_paths()
        agree = reg is base
        ok &= agree
        print(f"  {'PASS' if agree else 'FAIL'}  registered B2d agrees with this "
              f"control -> {reg} vs {base}")
    except Exception as exc:
        ok = False
        print(f"  FAIL  could not compare against registered B2d: {exc}")

    print("\n  VERDICT:", "ALL MUTATIONS CAUGHT" if ok else "CONTROL FAILURE")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
