#!/usr/bin/env python3
# ============================================================================
# crash_recipe_lint.py — V5: the direct-port crash recipes, as a source scan.
#
# TARGET RULED (Foundry, 2026-08-07): the CANONICAL cookbook's live
# "DIRECT-PORT CRASH RECIPES" section — 9 recipes. NOT the 17-row failure-class
# table, which exists only in a frozen DO-NOT-WRITE fork. A lint whose source
# cannot be corrected ossifies at its first mistake, and CANONICAL distilled
# those 17 into these 9 deliberately.
#
# Every recipe below is a bug that SHIPPED and cost a playtest cycle to find
# (symbolicated + playtest-confirmed on the d_a_kb pig port, bus §229-§234).
#
# ---------------------------------------------------------------------------
# THE LINTABILITY CRITERION, AND WHY 6 OF 9 ARE REFUSED
#
# A recipe is lintable iff its failure has a SYNTACTIC SIGNATURE IN THE PORTED
# TU: a call present or absent, two calls in the wrong order, a call missing its
# guard. It is NOT lintable if detection needs runtime state, a value judgment,
# or another TU's behaviour.
#
# **A recipe that is only PROBABLY detectable classifies as NOT lintable.** That
# rule is not caution, it is the §586 lesson: a guard shaped by the one failure
# its author imagined reported READY while an actor was missing. A lint that
# fires on a heuristic trains people to ignore it, and one that silently misses
# is worse than absent because it produces a green result someone trusts.
#
#   LINTABLE over ported TUs   3, 8      <- 2 of 9, not the 4 first claimed
#   LINTABLE over DONOR tree   9         (a PRE-PORT gate, different target)
#   NOT LINTABLE               1, 2, 4, 5, 6, 7
#
# The count went 4 -> 2 because both routed hits were FALSE on the first run
# (Engine, §588). Two laws that hold beat four that cry wolf, and the refusal
# records below are what make the downgrade cheap.
#
# Read-only. Usage: crash_recipe_lint.py [--all] [--donor PATH]
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402
import ww_census as WC  # noqa: E402

# --- LINTABLE: a signature in the ported TU ---------------------------------
# Each rule is (id, what shipped, find, exempt-if-also-present).
# `exempt` is the routing/guard the recipe prescribes: a TU that already does
# the right thing must not be flagged, or the lint becomes noise on day one.
RULES = [

    (3, "pointer truncated to 32 bits — donor setUserArea((u32)this) chops the "
        "actor pointer on x86_64 and the joint callback reads a wild address.",
     re.compile(r"\(\s*[us]32\s*\)\s*this\b|setUserArea\s*\(\s*\(\s*[us]32\s*\)"),
     None),


    (8, "raw mDoHIO_createChild/deleteChild — these reference the debug-only "
        "mDoHIO_root, unlinked in the retail PC build -> LNK2019. Use the "
        "mDoHIO_CREATE_CHILD / mDoHIO_DELETE_CHILD macros.",
     re.compile(r"\bmDoHIO_(?:create|delete)Child\s*\("),
     None),
]

# --- REFUSED, with the reason recorded --------------------------------------
# Kept in the file rather than dropped: a recipe absent from the lint reads as
# "already handled" unless the refusal is visible.
# THE SCOPE TEST (Engine, §588) — supersedes "is this a code shape?"
#
#   A recipe is lintable iff its INVARIANT LIVES ENTIRELY INSIDE THE SCOPE THE
#   LINT CAN SEE.
#
# My criterion was "does it have a syntactic signature?" -- necessary, not
# sufficient. Recipes 1, 2 and 5 ALL have one, which is why all three passed it,
# and all three were false. The invariant is what escapes scope:
#   1  DATAFLOW      getObjectRes result -> a model consumer
#   2  FUNCTION      modelCalc() in _execute(), entryDL() in _draw()
#   5  LEXICAL       the guard is 15 lines up, as a hand-rolled byte scan
# One principle refuses all three. I refused 1 for its specific reason and
# shipped 2 and 5 with the same defect in a different dress.
REFUSED = [
    (2, "Render needs modelCalc", "SCOPE. `modelCalc()` IS called -- "
        "d_a_npc_bm1.cpp:858-865, on every morf -- from `_execute()`, while "
        "`entryDL()` is in `_draw()`. That is the CORRECT architecture: calc "
        "where calc belongs, draw where draw belongs. The invariant spans a "
        "function boundary the lint cannot see."),
    (5, "BMT TEX1 guard", "SCOPE, PLUS A BUG OF MINE. The guard exists -- "
        "`bmtHasTex1`, a byte scan for 'T','E','X','1', added 2026-08-02, five "
        "days before the scan -- 15 lines above the call. Two failures: the "
        "invariant is lexically distant, AND my exemption pattern was "
        "CASE-SENSITIVE (`hasTex1` never matches `bmtHasTex1`). That is the "
        "second case-sensitivity defect I shipped in one day, after "
        "`g_profile_[A-Z0-9_]+` truncated Obj_Mshokki."),
    (1, "Model = raw at consume", "REFUSED BY ITS OWN FIRST RUN. Written as "
        "'flag dComIfG_getObjectRes', it fired on 8 TUs — and the recipe "
        "itself says 'Anims (BCK/BAS) are fine raw', so getObjectRes is the "
        "general resource fetch and the safe uses are the majority. The hazard "
        "is the result reaching a MODEL consumer, which is DATAFLOW; file-level "
        "co-occurrence cannot separate 'this call feeds a model' from 'this TU "
        "also happens to mention McaMorf elsewhere'. Flagging all 8 would have "
        "taught readers to ignore the lint by its second run. Needs a "
        "dataflow-aware pass, which is a different instrument."),
    (4, "Carry orientation", "CROSS-TU + SEMANTIC. The exception lives in the "
        "RECEIVER (d_a_alink_grab.inc forces shape_angle.x every carry frame), "
        "not in the ported TU, and knowing an actor is carried in a special "
        "pose is a design fact about the actor, not a token in its source."),
    (6, "Water-check offset", "NEEDS A VALUE JUDGMENT. SetWaterCheckOffset is "
        "greppable, but whether its value is WRONG depends on whether the "
        "actor swims or is thrown and how deep the local sea is. Flagging "
        "every value below the port swimmers' 10000 would fire on legitimately "
        "shallow cases — a heuristic, and heuristics train people to ignore "
        "the lint."),
    (7, "P13 state-tap gate", "NOT A DEFECT SIGNATURE. This is a PROCEDURE — "
        "run duskStateTap -> dusktap_to_jsonl -> state_gate. There is nothing "
        "in source to detect; a TU that never ran the tap looks identical to "
        "one that did and passed."),
]

# --- Recipe 9: a PRE-PORT gate over the DONOR tree, not the ported TU --------
STUB_THRESHOLD = 20


def donor_is_stub(path):
    """Recipe 9. >=20 `Nonmatching` bodies means an un-decompiled SKELETON:
    there is nothing to port, and copying it emits non-compiling empty
    functions. Seagull 0, pig 3; stubs 73-134."""
    p = Path(path)
    if not p.is_file():
        return None, "donor not found"
    n = len(re.findall(r"Nonmatching", p.read_text(encoding="utf-8",
                                                   errors="replace")))
    return n >= STUB_THRESHOLD, f"{n} Nonmatching bodies"


def scan():
    srcs = WC.load_build_sources() or []
    rows = [WC.classify_tu(s) for s in srcs]
    roster = WC.roster_union(rows)
    hits = []
    for rel in roster:
        p = C.REPO / rel
        if not p.is_file():
            continue
        txt = p.read_text(encoding="utf-8", errors="replace")
        # Comments are stripped: a recipe QUOTED in a header comment is not a
        # violation, and this file's own text would otherwise trip every rule.
        code = C._callable_text(txt)
        for rid, why, find, exempt in RULES:
            if not find.search(code):
                continue
            if exempt is not None and exempt.search(code):
                continue
            line = code[:find.search(code).start()].count(chr(10)) + 1
            hits.append((rid, rel, line, why))
    return hits, len(roster)


def main():
    if "--donor" in sys.argv:
        # ====================================================================
        # V9 (ferry §762): the gate is TWO-AXIS now. configure.py is the
        # donor's own authoritative ledger (AXIS A, ActorRel caveat honored in
        # decomp_status.py); the source-marker count stays as AXIS B texture.
        # Disagreement is FLAGGED, never adjudicated — V1's principle.
        # ====================================================================
        import decomp_status
        arg = sys.argv[sys.argv.index("--donor") + 1]
        name = arg.replace(chr(92), "/").rsplit("/", 1)[-1].replace(".cpp", "")
        rc = decomp_status.report(name)
        stub, why = donor_is_stub(arg) if "/" in arg or chr(92) in arg else (None, "axis B via decomp_status above")
        if stub is not None:
            print(f"  (legacy path heuristic: {why})")
        return rc

    hits, n = scan()
    print("V5 — DIRECT-PORT CRASH RECIPES AS LINT")
    print(f"  target: the CANONICAL cookbook's 9 live recipes")
    print(f"  scanned: {n} roster TUs\n")
    print(f"  RULES ACTIVE: {len(RULES)} of 9 "
          f"({', '.join(str(r[0]) for r in RULES)})")
    for rid, name, why in REFUSED:
        print(f"  RECIPE {rid} NOT LINTED — {name}: {why[:72]}...")
    print(f"  RECIPE 9 is a PRE-PORT gate over the DONOR tree: --donor <path>\n")

    if not hits:
        print("  no violations.")
        print("  A zero here covers 2 of 9 recipes over ported TUs. It is NOT "
              "'the recipes are satisfied' — 6 are unlintable by construction "
              "and 1 targets the donor tree. Silence is scoped, not clean.")
        return 0
    for rid, rel, line, why in sorted(hits):
        print(f"  RECIPE {rid}  {rel}:{line}")
        print(f"      {why}")
    print(f"\n  {len(hits)} violation(s). Each is a bug that already shipped "
          f"once and cost a playtest cycle.")
    return 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
