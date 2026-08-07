#!/usr/bin/env python3
# ============================================================================
# census_selftest.py — FALSIFICATION tests for the census's own guards.
#
# §495 recorded that three self-checks had never been made to fail:
#   * the from_cpp guard in census_axis_c.build_decl_index()
#   * invariant 1 — Axis W callbacks must equal Axis C receiver-native edges
#   * invariant 2 — every Axis W entry point is declared in a subsystem file
#
# "An invariant that has never been made to fail isn't yet known to be live."
# That is exactly right, and it is the same standard applied to Axis B's
# falsifiability control: a check that cannot fail is not a check.
#
# LANE NOTE. Negative-controlling the census is HousingTemp's job and stays
# theirs — the lane that built an instrument cannot audit it, and this campaign
# has the receipts: Foundry found the directory-scoping defect by inspection but
# needed HousingTemp's planted subsystem to find that definitions were being
# counted as call edges, a one-directional error inspection had missed twice.
# What this file does is narrower and IS Foundry's: proving that the guards
# WRITTEN INTO Foundry's own tools actually fire. A guard that silently never
# fires is worse than no guard, because it is read as coverage.
#
# Each test deliberately BREAKS a precondition and asserts the guard notices.
# Passing here means "the guard is live", not "the census is correct".
#
# Usage: census_selftest.py        (exit 0 = every guard fired as designed)
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402
import census_axis_w as W  # noqa: E402

PLANT_SRC = "src/_selftest_plant/plant.cpp"
PLANT_HDR = "include/_selftest_plant/plant.h"


def _write_plant():
    (C.REPO / "src" / "_selftest_plant").mkdir(parents=True, exist_ok=True)
    (C.REPO / "include" / "_selftest_plant").mkdir(parents=True, exist_ok=True)
    (C.REPO / PLANT_HDR).write_text(
        "#ifndef SELFTEST_PLANT_H\n#define SELFTEST_PLANT_H\n"
        "void selftestPlant_alpha(void);\nvoid selftestPlant_beta(void);\n"
        "#endif\n", encoding="utf-8")
    (C.REPO / PLANT_SRC).write_text(
        '#include "_selftest_plant/plant.h"\n'
        '#include "d/d_com_inf_game.h"\n'
        "void selftestPlant_alpha(void) {\n"
        "    selftestPlant_beta();\n"
        "    dComIfGp_getPlayer(0);\n}\n"
        "void selftestPlant_beta(void) {\n"
        "    selftestPlant_alpha();\n"
        "    dComIfGs_isSwitch(1, 2);\n}\n", encoding="utf-8")


def _remove_plant():
    import shutil
    for d in ("src/_selftest_plant", "include/_selftest_plant"):
        shutil.rmtree(C.REPO / d, ignore_errors=True)


def test_from_cpp_guard():
    """Break definition matching; the guard must refuse to return an index."""
    saved = C.DEFN
    C.DEFN = re.compile(r"^\x01NEVERMATCHES\x01$")
    try:
        C.build_decl_index()
    except RuntimeError as e:
        return True, f"fired: {str(e)[:60]}..."
    except Exception as e:  # pragma: no cover
        return False, f"raised the WRONG error: {type(e).__name__}: {e}"
    else:
        return False, "DID NOT FIRE — index returned with zero .cpp symbols"
    finally:
        C.DEFN = saved


def _plant_results():
    if not any(rx.pattern == r"/_selftest_plant/" for rx, _ in C.GROUPS):
        C.GROUPS.insert(0, (re.compile(r"/_selftest_plant/"), "SELFTEST-PLANT"))
    cres = C.run([PLANT_SRC])
    wres = W.run([PLANT_SRC], [PLANT_SRC], cres)
    return cres, wres


def inv1(cres, wres):
    """Axis W callbacks == Axis C receiver-native edges, per subsystem."""
    bad = []
    for k in cres:
        a = cres[k]["counts"].get("receiver-native", 0)
        b = len(wres.get(k, {}).get("callbacks", {}))
        if a != b:
            bad.append((k, a, b))
    return bad


def inv2(wres):
    """Every entry point's declaring file belongs to its subsystem."""
    bad = []
    for k, v in wres.items():
        scope = set(v["files"])
        for n, e in v["entry_points"].items():
            d = e["declared_in"]
            if d in scope:
                continue
            stem_ok = any(Path(d).stem == Path(f).stem for f in v["files"])
            if not stem_ok:
                bad.append((k, n, d))
    return bad


def test_invariant_live(name, fn, perturb):
    """An invariant is live only if it passes clean data AND fails broken data."""
    cres, wres = _plant_results()
    clean = fn(cres, wres)
    if clean:
        return False, f"reported a violation on CLEAN data: {clean[:2]}"
    broken = perturb(cres, wres)
    dirty = fn(*broken)
    if not dirty:
        return False, "did NOT detect deliberately broken data — not live"
    return True, f"clean pass, broken detected ({len(dirty)} violation(s))"


def main():
    _write_plant()
    results = []
    try:
        results.append(("from_cpp guard", *test_from_cpp_guard()))

        def perturb1(cres, wres):
            k = next(iter(cres))
            cres[k]["counts"]["receiver-native"] = \
                cres[k]["counts"].get("receiver-native", 0) + 7
            return cres, wres
        results.append(("invariant 1 (W callbacks == C receiver-native)",
                        *test_invariant_live("inv1", inv1, perturb1)))

        def perturb2(cres, wres):
            k = next(iter(wres))
            wres[k]["entry_points"]["__phantom__"] = {
                "declared_in": "src/somewhere/else.cpp", "sites": [], "site_count": 1}
            return cres, wres
        results.append(("invariant 2 (entry point declared in subsystem)",
                        *test_invariant_live("inv2", lambda c, w: inv2(w), perturb2)))
    finally:
        _remove_plant()

    print("CENSUS SELF-TEST — do Foundry's own guards actually fire?\n")
    ok = True
    for name, passed, detail in results:
        print(f"  [{'LIVE ' if passed else 'DEAD '}] {name}")
        print(f"           {detail}")
        ok &= passed
    print("\n" + ("all guards live" if ok else
                  "AT LEAST ONE GUARD IS DEAD — it reads as coverage and is not"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
