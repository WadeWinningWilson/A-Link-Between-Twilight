#!/usr/bin/env python3
# ===========================================================================
# ww_kit_layout.py - THE LAYOUT LAW. A kit may only emit a port at the path
# its DONOR uses, and only when it can NAME that donor unambiguously.
#
# DN-15 made the layout a rule and ww_layout_gate.py enforces it AT COMMIT.
# That is a backstop, not a cure: it catches a violation after a kit has
# already produced it and someone has already built on it. This makes the
# violation impossible to EMIT, which is the same move kit_output_law.py made
# for baked donor bytes.
#
# THE STATE THIS WAS WRITTEN INTO, measured 2026-08-22 and worth recording
# because it is worse than it looks:
#   · NO kit emits a KIT-* header at all. Every file the kits produce starts
#     life non-conforming and someone hand-fixes it later, or nobody does.
#   · actor_kit.py performs NINE raw writes and imports kit_output_law ZERO
#     times - while kit_output_law's own header claims "the retooled kits
#     (actor_kit, ...) route here". **A law with a false adoption claim is
#     worse than no law: it is a law everyone believes is running.**
#
# THE RULE THIS ENFORCES
#   A kit may write a WW port ONLY IF it can name exactly ONE donor unit that
#   DEFINES the thing being ported. Then, and only then, the destination is
#   that unit's own path and the file carries a complete KIT block.
#
# WHY "DEFINES" AND NOT "MENTIONS" - this is the expensive lesson, paid twice
# in one day. `usonami` was attributed to d_a_sea because the symbol APPEARS
# there; the donor defines it in d_kankyo and d_a_sea merely calls it.
# Resolution here therefore requires a DEFINITION (a body), never a mention.
# A search that cannot tell "mentions" from "owns" returns the wrong file
# confidently, and confidently-wrong is what a kit then bakes into a path.
#
# FAILURE IS LOUD AND IS THE DEFAULT. Ambiguous donor (0 or >1 definitions)
# raises LayoutLawViolation with the candidates listed. A kit cannot proceed
# on a guess, because a guessed path is indistinguishable from a known one
# once the file exists.
#
# Usage as a tool:  ww_kit_layout.py --selftest
#                   ww_kit_layout.py --audit     (which kits bypass this law)
# ===========================================================================
import io
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
DONOR_SRC = Path(r"D:\XXXXXXX\WW DP\src")
WW_SRC = "src/ww"
WW_INC = "include/ww"


class LayoutLawViolation(RuntimeError):
    """Raised INSTEAD of writing. Never downgraded to a warning: a warning on
    a write that already happened is a report, not a law."""


# --- donor resolution -----------------------------------------------------

_DEF = r'^[A-Za-z_][\w:<>,\s\*&]*?\b%s\s*\([^;]*?\)\s*(?:const\s*)?\{'


def defining_units(symbol, donor_src=None):
    """Donor units that DEFINE `symbol` (body required), as unit paths.

    Mentions do not count. See the header: this is the `usonami` rule."""
    root = Path(donor_src or DONOR_SRC)
    if not root.is_dir():
        raise LayoutLawViolation(
            "donor tree not readable at %s - REFUSING rather than guessing a "
            "path. An unreachable donor is not an absent constraint." % root)
    pat = re.compile(_DEF % re.escape(symbol), re.M)
    hits = []
    for p in root.rglob("*.cpp"):
        try:
            t = io.open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
        t = re.sub(r'//[^\n]*', '', t)
        if pat.search(t):
            hits.append(p.relative_to(root).as_posix())
    return sorted(hits)


def resolve_unit(symbol, donor_src=None):
    """The ONE donor unit that owns `symbol`, or refuse with the candidates."""
    hits = defining_units(symbol, donor_src)
    if len(hits) == 1:
        return re.sub(r'\.cpp$', '', hits[0])
    if not hits:
        raise LayoutLawViolation(
            "NO DONOR UNIT DEFINES '%s'.\n"
            "  A kit may not place a port whose donor it cannot name. Either the\n"
            "  symbol is spelled differently in the donor, or the thing being\n"
            "  ported is RECEIVER-SIDE and does not belong under %s/ at all."
            % (symbol, WW_SRC))
    raise LayoutLawViolation(
        "AMBIGUOUS DONOR for '%s' - %d units define it:\n%s\n"
        "  Refusing to choose. A guessed path is indistinguishable from a known\n"
        "  one once the file exists." % (symbol, len(hits),
                                         "\n".join("    " + h for h in hits)))


# --- lawful placement -----------------------------------------------------

def port_paths(unit):
    """(source, header) paths a port of `unit` is allowed to occupy."""
    return "%s/%s.cpp" % (WW_SRC, unit), "%s/%s.h" % (WW_INC, unit)


def kit_header(unit, lineage, donor_ref, symbols="", status="UNKNOWN"):
    """The KIT block, complete by construction.

    STATUS DEFAULTS TO UNKNOWN AND THAT IS DELIBERATE. A kit has not compared
    anything against the donor; emitting `Matching` would be a measurement
    nobody took. Only a lane that actually diffed may raise it."""
    if not lineage or not donor_ref:
        raise LayoutLawViolation(
            "KIT block needs a lineage AND a donor ref. An absent ref is how "
            "three different donor pins ended up in one repo with nothing "
            "checking that a cited ref is the tree a claim came from.")
    sym = (" " + symbols) if symbols else ""
    return ("// KIT-LINEAGE: %s\n// KIT-DONOR: %s.cpp%s\n"
            "// KIT-DONOR-REF: %s\n// KIT-DONOR-STATUS: %s\n"
            % (lineage, unit, sym, donor_ref, status))


def lawful_port_write(repo_root, symbol, body, lineage, donor_ref,
                      header_body=None, symbols="", donor_src=None, kit="?"):
    """Place a port where its donor says, or refuse. Returns written paths."""
    unit = resolve_unit(symbol, donor_src)
    src_rel, inc_rel = port_paths(unit)
    root = Path(repo_root)
    out = []
    block = kit_header(unit, lineage, donor_ref, symbols)
    for rel, content in ((src_rel, body), (inc_rel, header_body)):
        if content is None:
            continue
        dest = root / rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        io.open(dest, "w", encoding="utf-8", newline="").write(block + content)
        out.append(rel)
    print("[layout-law] %s -> %s (donor %s)" % (kit, ", ".join(out), unit))
    return out


# --- who bypasses this law ------------------------------------------------

KIT_FILES = ("actor_kit.py", "enemy_port_kit.py", "port_kit.py",
             "space_kit.py", "kit_batch_rate.py")


def audit():
    """Name every kit that can still emit a port without passing through here.

    The user's ask was failure output for kits without clear organization;
    this is that, pointed at the kits themselves rather than their output."""
    print("KIT LAYOUT AUDIT - who can still emit a non-conforming port?\n")
    bad = 0
    for name in KIT_FILES:
        p = HERE / name
        if not p.is_file():
            print("  %-20s MISSING" % name)
            continue
        t = io.open(p, encoding="utf-8", errors="replace").read()
        uses = "ww_kit_layout" in t
        raw = len(re.findall(r'write_text\(|open\([^)]*[\'"]w', t))
        emits_kit = "KIT-DONOR" in t
        state = "ADOPTED" if uses else "*** BYPASSES THE LAW ***"
        if not uses:
            bad += 1
        print("  %-20s %-26s raw writes=%-3d emits KIT block=%s"
              % (name, state, raw, "yes" if emits_kit else "NO"))
    print()
    if bad:
        print("  %d kit(s) can still emit a port that DN-15 will reject at commit." % bad)
        print("  Adoption is: import ww_kit_layout, replace the raw write with")
        print("  lawful_port_write(...). The law refuses an unnameable donor, so a")
        print("  kit cannot proceed on a guess.")
    else:
        print("  Every kit routes through the layout law.")
    return 1 if bad else 0


def selftest():
    import tempfile
    tmp = Path(tempfile.mkdtemp(prefix="layout_law_"))
    donor = tmp / "donor"
    (donor / "d" / "actor").mkdir(parents=True)
    (donor / "d" / "d_kankyo.cpp").write_text(
        "void dKy_usonami_set(f32 v) {\n  return;\n}\n", encoding="utf-8")
    (donor / "d" / "actor" / "d_a_sea.cpp").write_text(
        "void other() {\n  dKy_usonami_set(1.0f);\n}\n", encoding="utf-8")
    (donor / "d" / "dupe_a.cpp").write_text("void twice() {\n}\n", encoding="utf-8")
    (donor / "d" / "dupe_b.cpp").write_text("void twice() {\n}\n", encoding="utf-8")
    repo = tmp / "repo"
    repo.mkdir()
    checks = []

    # the usonami case: a CALL SITE must not win
    try:
        unit = resolve_unit("dKy_usonami_set", donor)
        checks.append(("call site does not win", unit == "d/d_kankyo"))
    except LayoutLawViolation:
        checks.append(("call site does not win", False))

    # ambiguity refuses
    try:
        resolve_unit("twice", donor)
        checks.append(("ambiguous donor REFUSES", False))
    except LayoutLawViolation:
        checks.append(("ambiguous donor REFUSES", True))

    # unknown refuses
    try:
        resolve_unit("nowhere_at_all", donor)
        checks.append(("unnameable donor REFUSES", False))
    except LayoutLawViolation:
        checks.append(("unnameable donor REFUSES", True))

    # lawful write lands at the donor's path with a complete block
    wrote = lawful_port_write(repo, "dKy_usonami_set", "// body\n",
                              "native-port", "zeldaret/tww@0000000",
                              donor_src=donor, kit="selftest")
    p = repo / "src/ww/d/d_kankyo.cpp"
    txt = p.read_text(encoding="utf-8") if p.is_file() else ""
    checks.append(("lands at donor's own path", wrote == ["src/ww/d/d_kankyo.cpp"]))
    checks.append(("emits a complete KIT block",
                   all(k in txt for k in ("KIT-LINEAGE", "KIT-DONOR:",
                                          "KIT-DONOR-REF", "KIT-DONOR-STATUS"))))
    checks.append(("status defaults to UNKNOWN", "KIT-DONOR-STATUS: UNKNOWN" in txt))

    # a KIT block without a ref refuses
    try:
        kit_header("d/d_kankyo", "native-port", "")
        checks.append(("missing donor ref REFUSES", False))
    except LayoutLawViolation:
        checks.append(("missing donor ref REFUSES", True))

    print("SELFTEST - the law must REFUSE more often than it permits")
    for name, ok in checks:
        print("  %-30s %s" % (name, "PASS" if ok else "*** FAIL ***"))
    good = all(o for _, o in checks)
    print("  SELFTEST %s" % ("PASSED - it refuses guesses and places the rest."
                             if good else "*** FAILED ***"))
    import shutil
    shutil.rmtree(tmp, ignore_errors=True)
    return 0 if good else 1


if __name__ == "__main__":
    if "--audit" in sys.argv:
        sys.exit(audit())
    sys.exit(selftest())
