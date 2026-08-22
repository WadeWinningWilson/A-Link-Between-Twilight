#!/usr/bin/env python3
# ============================================================================
# ww_layout_gate.py - ONE TU PER DONOR UNIT, AT THE DONOR'S OWN PATH.
#
# THE RULE, set by the user 2026-08-22 and now DN-15:
#   "Files shouldn't themselves port 2/3 things. They should each port ONE
#    thing (or follow vanilla, if vanilla keeps those lines/port items
#    separate, we do too)."
# extended the same day to bind future work:
#   "ensure that the kits (and any instances) now wire/port according to the
#    WW data organization from now on (unless explicit reasons are given)."
#
# WHY A GATE AND NOT A DOCUMENT. The reorg that produced this rule cost a
# full day: four files fused several donor units each, two donor units had
# two porters apiece, one donor routine was ported TWICE under two names in
# two files with nothing telling either author the other existed. Every one
# of those was written by someone who would have agreed with the rule if
# asked. **A rule that depends on each instance remembering it is a rule
# that decays at the rate instances are replaced.** This fails the commit.
#
# WHAT IT CHECKS, per file under the WW tree:
#   1. EXACTLY ONE distinct KIT-DONOR unit. Two is the defect the reorg
#      existed to remove.
#   2. PATH MIRRORS THE DONOR. Donor `d/actor/d_a_sea.cpp` must live at
#      `src/ww/d/actor/d_a_sea.cpp`. A file in the right directory with the
#      wrong name is how `usonami` ended up attributed to the wrong unit.
#   3. THE KIT BLOCK IS COMPLETE - LINEAGE, DONOR, DONOR-REF, DONOR-STATUS.
#      An absent status reads identically to "verified but never written
#      down", which is how UNKNOWN work gets quoted as settled.
#
# THE ESCAPE HATCH IS DELIBERATE AND MUST BE ARGUED IN-FILE:
#   // KIT-LAYOUT-EXEMPT: <reason>
# The user's own wording is "unless explicit reasons are given", so the gate
# accepts an exemption that STATES ITS REASON in the file it applies to - not
# in a side list nobody reads. `ww_cam_data.cpp` is the founding case: its
# tables are GENERATED into a blob shaped like the RECEIVER's camstyle.dat,
# so the thing it ports is that receiver format, which matches no single
# donor unit. Splitting it would mean changing the generator, not moving
# code. An exemption with an empty reason is REFUSED - that is just the
# defect with a sticker on it.
#
# Usage:
#   ww_layout_gate.py [--repo PATH]            check the whole WW tree
#   ww_layout_gate.py --staged [--repo PATH]   check only staged files (hook mode)
#   ww_layout_gate.py --control       prove it can FAIL, not only pass
# Exit 0 conforming - 1 violation(s) - 2 could not run (never a silent pass).
# ============================================================================
import io
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
DEFAULT_REPO = HERE.parents[1].parent / "A Link Between Twilight WW"
WW_ROOT = "src/ww"
KEYS = ("KIT-LINEAGE", "KIT-DONOR", "KIT-DONOR-REF", "KIT-DONOR-STATUS")


def kit_block(text):
    head = text[:8000]
    out = {}
    for k in KEYS:
        vals = re.findall(r'^//\s*%s:\s*(.+)$' % re.escape(k), head, re.M)
        out[k] = [v.strip() for v in vals]
    exempt = re.findall(r'^//\s*KIT-LAYOUT-EXEMPT:\s*(.*)$', head, re.M)
    return out, [e.strip() for e in exempt]


def donor_units(vals):
    seen = []
    for v in vals:
        m = re.match(r'(\S+?\.(?:cpp|h))', v)
        unit = re.sub(r'\.(cpp|h)$', '', m.group(1)) if m else v.split()[0]
        if unit.lower() == "none":
            continue
        if unit not in seen:
            seen.append(unit)
    return seen


def expected_path(unit):
    """Donor `d/actor/d_a_sea` -> src/ww/d/actor/d_a_sea.cpp."""
    return "%s/%s.cpp" % (WW_ROOT, unit)


def staged_files(repo):
    """Only what THIS commit touches.

    Gating the whole tree would block every unrelated commit while a split is
    mid-flight -- a gate that punishes bystanders gets disabled, and a
    disabled gate protects nothing. Gating the DIFF means you cannot ADD a
    violation while existing ones are being worked off in order."""
    import subprocess
    r = subprocess.run(["git", "-C", str(repo), "diff", "--cached",
                        "--name-only", "--diff-filter=ACMR"],
                       capture_output=True, text=True)
    return [f for f in r.stdout.splitlines()
            if f.startswith(WW_ROOT + "/") and f.endswith(".cpp")]


def check(repo, only=None):
    repo = Path(repo)
    root = repo / WW_ROOT
    if not root.is_dir():
        print("GATE COULD NOT RUN: no %s under %s" % (WW_ROOT, repo))
        print("  Reporting this rather than passing - a check that could not")
        print("  run is not a check that passed (DN-12).")
        return 2, []
    problems = []
    files = sorted(root.rglob("*.cpp"))
    if only is not None:
        keep = set(only)
        files = [f for f in files
                 if f.relative_to(repo).as_posix() in keep]
    for p in files:
        rel = p.relative_to(repo).as_posix()
        text = io.open(p, encoding="utf-8", errors="replace").read()
        block, exempt = kit_block(text)
        if exempt:
            if not any(e for e in exempt):
                problems.append((rel, "KIT-LAYOUT-EXEMPT with NO REASON - an "
                                      "exemption must argue for itself"))
            continue
        units = donor_units(block["KIT-DONOR"])
        if len(units) == 0:
            problems.append((rel, "no KIT-DONOR declared - an undeclared port "
                                  "cannot be checked against its donor"))
            continue
        if len(units) > 1:
            problems.append((rel, "ports %d donor units (%s) - DN-15 is one TU "
                                  "per donor unit" % (len(units), ", ".join(units))))
            continue
        want = expected_path(units[0])
        if rel != want:
            problems.append((rel, "donor is %s so the file belongs at %s"
                             % (units[0], want)))
        missing = [k for k in KEYS if not block[k]]
        if missing:
            problems.append((rel, "KIT block incomplete, missing %s"
                             % ", ".join(missing)))
    return (1 if problems else 0), problems


def report(code, problems, repo, files_note=""):
    if code == 2:
        return 2
    if not problems:
        print("WW LAYOUT GATE: conforming.")
        print("  Every file under %s/ ports exactly ONE donor unit, sits at that"
              % WW_ROOT)
        print("  unit's own path, and carries a complete KIT block.%s" % files_note)
        return 0
    print("WW LAYOUT GATE: *** %d VIOLATION(S) ***" % len(problems))
    for rel, why in problems:
        print("  %-46s %s" % (rel, why))
    print()
    print("DN-15: one translation unit per donor unit, at the donor's own path.")
    print("If a file genuinely cannot obey - a generated fusion, say - declare")
    print("it IN THE FILE and say why:")
    print("    // KIT-LAYOUT-EXEMPT: <reason this cannot be one unit>")
    print("An exemption with no reason is refused.")
    return 1


def control():
    """Prove the gate FAILS on each violation class. A gate that has only
    ever printed 'conforming' is indistinguishable from a gate that cannot
    print anything else -- which is DN-12's whole point."""
    import tempfile
    tmp = Path(tempfile.mkdtemp(prefix="ww_layout_control_"))
    d = tmp / WW_ROOT / "d"
    d.mkdir(parents=True)
    hdr = ("// KIT-LINEAGE: native-port\n// KIT-DONOR: %s\n"
           "// KIT-DONOR-REF: zeldaret/tww@0000000\n// KIT-DONOR-STATUS: UNKNOWN\n")
    # 1 conforming
    (d / "d_kankyo.cpp").write_text(hdr % "d/d_kankyo.cpp", encoding="utf-8")
    # 2 two donor units
    (d / "d_two.cpp").write_text(
        (hdr % "d/d_kankyo.cpp") + "// KIT-DONOR: d/d_kankyo_rain.cpp\n", encoding="utf-8")
    # 3 right dir, wrong name
    (d / "ww_misnamed.cpp").write_text(hdr % "d/d_mesg.cpp", encoding="utf-8")
    # 4 incomplete KIT block
    (d / "d_camera.cpp").write_text("// KIT-DONOR: d/d_camera.cpp\n", encoding="utf-8")
    # 5 exemption with no reason
    (d / "d_bare.cpp").write_text("// KIT-LAYOUT-EXEMPT:\n", encoding="utf-8")
    # 6 exemption WITH a reason -> must pass
    (d / "d_ok.cpp").write_text("// KIT-LAYOUT-EXEMPT: generated fusion, see WW-TREE-MAP 5a\n",
                                encoding="utf-8")
    try:
        code, problems = check(tmp)
        by = {Path(r).name: w for r, w in problems}
        cases = [
            ("conforming file passes", "d_kankyo.cpp" not in by),
            ("two donor units FAILS", "d_two.cpp" in by),
            ("wrong filename FAILS", "ww_misnamed.cpp" in by),
            ("incomplete KIT block FAILS", "d_camera.cpp" in by),
            ("reasonless exemption FAILS", "d_bare.cpp" in by),
            ("reasoned exemption passes", "d_ok.cpp" not in by),
            ("overall verdict is FAIL", code == 1),
        ]
        print("CONTROL - the gate must FAIL on each class, and PASS the other two")
        for name, ok in cases:
            print("  %-30s %s" % (name, "PASS" if ok else "*** WRONG ***"))
        ok = all(o for _, o in cases)
        print("  CONTROL %s" % ("PASSED - it can convict AND acquit."
                                if ok else "*** FAILED ***"))
        return 0 if ok else 1
    finally:
        for f in sorted(tmp.rglob("*"), reverse=True):
            f.unlink() if f.is_file() else f.rmdir()
        tmp.rmdir()


def main():
    if "--control" in sys.argv:
        return control()
    repo = DEFAULT_REPO
    if "--repo" in sys.argv:
        repo = Path(sys.argv[sys.argv.index("--repo") + 1])
    only = None
    note = ""
    if "--staged" in sys.argv:
        only = staged_files(repo)
        if not only:
            print("WW LAYOUT GATE: no WW sources staged - nothing to check.")
            return 0
        note = "  (staged only: %d file(s))" % len(only)
    code, problems = check(repo, only)
    return report(code, problems, repo, note)


if __name__ == "__main__":
    sys.exit(main())
