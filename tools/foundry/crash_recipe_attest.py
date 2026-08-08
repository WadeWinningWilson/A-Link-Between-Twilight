#!/usr/bin/env python3
# ===========================================================================
# crash_recipe_attest.py — POSITIVE CONTROL for the two surviving V5 recipes.
#
# WHY THIS EXISTS
# V5 shipped 9 crash recipes, 7 were refused, and the 2 survivors have only
# ever printed PASS. A lint that has never been shown to FIRE is unattested:
# "no violations" and "the detector is broken" produce identical output, and
# the campaign already spent a day on patterns that matched what someone
# pictured rather than what the tree contains (`g_profile_[A-Z0-9_]+`
# truncating `Obj_Mshokki`; a case-sensitive `hasTex1` missing `bmtHasTex1`).
#
# The scope test told us which recipes are lintable. It says nothing about
# whether the regex that implements one actually works. That is this file.
#
# WHAT IT TESTS — THE SHIPPED OBJECTS, NOT A TRANSCRIPTION
# It imports `crash_recipe_lint.RULES` and `kit_laws.law6/law7` and runs the
# real patterns through the real comment-stripping path. A copy of the regex
# here would attest the copy. Both tools carry the SAME two patterns in two
# places, so this doubles as the drift check between them.
#
# Fixtures are both directions:
#   POSITIVE  the defect shape MUST be flagged  (proves the law can fire)
#   NEGATIVE  the prescribed fix MUST NOT be    (proves it will not cry wolf)
#
# Read-only. Touches no source. Usage: crash_recipe_attest.py [-v]
# Exit 0 = every fixture behaved as specified; 1 = a law is mis-detecting.
# ===========================================================================
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402
import crash_recipe_lint as V5  # noqa: E402
import kit_laws as KL  # noqa: E402


# ===========================================================================
# FIXTURES
# Each: (name, recipe id, code, expect_flag, why this case is here)
# The code is written the way the donor writes it — the shapes come from the
# recipes' own crash narratives (§229-§234, symbolicated on the pig port).
# ===========================================================================
FIXTURES = [
    # --- recipe 3 / law 6: pointer truncation ------------------------------
    ("setUserArea donor form", 3,
     "void init(void) {\n"
     "    fopAcM_SetUserArea(this, (u32)this);\n"
     "}\n",
     True, "the exact donor line the recipe was written from"),

    ("bare u32 cast of this", 3,
     "void calc(void) {\n"
     "    u32 tag = (u32)this;\n"
     "}\n",
     True, "the cast is the defect wherever it appears, not just in setUserArea"),

    ("s32 spelling", 3,
     "void calc(void) {\n"
     "    s32 tag = (s32)this;\n"
     "}\n",
     True, "signed spelling truncates identically; a u32-only pattern would miss it"),

    ("spaced cast", 3,
     "void calc(void) {\n"
     "    u32 tag = ( u32 ) this;\n"
     "}\n",
     True, "clang-format can space a cast; whitespace must not hide the defect"),

    ("uintptr_t — the prescribed fix", 3,
     "void init(void) {\n"
     "    fopAcM_SetUserArea(this, (uintptr_t)this);\n"
     "}\n",
     False, "flagging the fix is how a lint gets ignored by its second run"),

    ("u32 cast of a NON-this expression", 3,
     "void calc(void) {\n"
     "    u32 frames = (u32)thisFrameCount;\n"
     "}\n",
     False, "word-boundary control: `this` must not match inside an identifier"),

    ("defect quoted in a comment", 3,
     "// Donor wrote setUserArea((u32)this); we use uintptr_t instead.\n"
     "void init(void) {\n"
     "    fopAcM_SetUserArea(this, (uintptr_t)this);\n"
     "}\n",
     False, "a banner naming the forbidden shape must not read as a violation"),

    # --- recipe 8 / law 7: raw HIO child calls -----------------------------
    ("raw createChild", 8,
     "void init(void) {\n"
     "    mDoHIO_createChild(\"npc_bm1\", &mHIO);\n"
     "}\n",
     True, "references debug-only mDoHIO_root — LNK2019 in the retail PC build"),

    ("raw deleteChild", 8,
     "void del(void) {\n"
     "    mDoHIO_deleteChild(mHIOId);\n"
     "}\n",
     True, "the unregister side fails to link identically to the register side"),

    ("CREATE_CHILD macro — the prescribed fix", 8,
     "void init(void) {\n"
     "    mDoHIO_CREATE_CHILD(\"npc_bm1\", &mHIO);\n"
     "}\n",
     False, "the macro no-ops outside DEBUG; it is the routing the recipe asks for"),

    ("HIO field read", 8,
     "void calc(void) {\n"
     "    f32 s = mHIO.mSpeed;\n"
     "}\n",
     False, "law 7's own docstring: the tuning FIELDS stay readable"),
]


def _v5_verdict(rid, code):
    """Run the SHIPPED V5 rule the way scan() runs it — same comment strip,
    same exempt handling. A local re-implementation would attest itself."""
    stripped = C._callable_text(code)
    for r_id, _why, find, exempt in V5.RULES:
        if r_id != rid:
            continue
        if not find.search(stripped):
            return False
        if exempt is not None and exempt.search(stripped):
            return False
        return True
    raise SystemExit(f"FATAL: recipe {rid} is not in crash_recipe_lint.RULES — "
                     f"the active set changed and this attestation is stale.")


def _law_verdict(rid, code):
    """Run the SHIPPED kit_laws law. Laws 6/7 are the same two recipes carried
    in a second tool; disagreement here IS the drift finding."""
    fn = {3: KL.law6_ptrtrunc, 8: KL.law7_hio_macro}[rid]
    verdict, _why = fn(code, KL.strip_comments(code))
    return verdict == "VIOLATION"


def main():
    verbose = "-v" in sys.argv
    print("V5 ATTESTATION — positive control for the 2 surviving recipes")
    print("  recipe 3 = pointer truncation   (kit_laws law 6)")
    print("  recipe 8 = raw mDoHIO child     (kit_laws law 7)\n")

    fails, drift = [], []
    for name, rid, code, expect, note in FIXTURES:
        got_v5 = _v5_verdict(rid, code)
        got_law = _law_verdict(rid, code)

        ok = (got_v5 == expect) and (got_law == expect)
        if got_v5 != got_law:
            drift.append((name, rid, got_v5, got_law))
        if not ok:
            fails.append((name, rid, expect, got_v5, got_law))

        want = "FLAG" if expect else "pass"
        mark = "  ok  " if ok else " FAIL "
        print(f"  [{mark}] recipe {rid}  {want:4}  {name}")
        if verbose or not ok:
            print(f"            V5={'FLAG' if got_v5 else 'pass'}  "
                  f"law={'FLAG' if got_law else 'pass'}   {note}")

    pos = sum(1 for f in FIXTURES if f[3])
    neg = len(FIXTURES) - pos
    print(f"\n  {len(FIXTURES)} fixtures: {pos} positive, {neg} negative")

    if drift:
        print("\n  DRIFT — the two tools carry the same recipe and DISAGREE:")
        for name, rid, a, b in drift:
            print(f"    recipe {rid}  {name}: V5={a} kit_laws={b}")

    if fails:
        print(f"\n  {len(fails)} FIXTURE(S) FAILED. A law that cannot fire is "
              f"not a law that holds — it is silence that reads like one.")
        return 1

    print("\n  ATTESTED. Both laws fire on the defect, stay quiet on the fix,")
    print("  and the two tools agree. The 2-of-9 count is now measured in both")
    print("  directions, not just in the direction that prints nothing.")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
