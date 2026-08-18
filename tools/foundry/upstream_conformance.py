#!/usr/bin/env python3
# ============================================================================
# upstream_conformance.py — 19b's ACTUAL PURPOSE: does the plugin's binding
# surface exist in VANILLA UPSTREAM, not just in our fork?
#
# THE STEP'S OWN WORDS: "run against our fork, pure upstream, and every future
# release — the diff between runs is what turns 'works on any dusklight build'
# from assumption into evidence." Housing's 19b run resolved our hook targets
# against OUR fork's export list, which proves our build exports them. The
# delivery claim is about the build a USER has, and the user pointed at the
# clean upstream checkout for exactly that reason.
#
# READ-ONLY BY CONSTRUCTION: this parses the upstream exe's EXPORT TABLE. It
# writes nothing, launches nothing, and never touches that tree's mods folder
# — the two acts awaiting the user's go are not needed to answer this.
#
# WHAT IT ANSWERS: for each symbol the plugin must bind, is it RESOLVED in the
# upstream image, or MISSING? A MISSING symbol means the plugin cannot bind
# that seam on a stock build — which is the delivery claim failing, and the
# most important thing to learn EARLY rather than after a migration.
#
# Usage: upstream_conformance.py [<exe>]   (default: the clean upstream copy)
# Exit 0 all resolved · 1 something missing · 2 could-not-run (№31-C).
# ============================================================================
import re
import struct
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

UPSTREAM = Path("C:/Users/xxxxx/Documents/dusklight-main/build/"
                "windows-msvc-relwithdebinfo/dusklight.exe")

# The binding surface the migration depends on, each with WHY it matters.
TARGETS = [
    ("fpcPf_Get", "the single index->profile choke point; its own in-tree "
                  "comment specifies the plugin hook (tale §947)"),
    ("dStage_searchName", "placement-route lookup; pre-hook + fall-through "
                          "lets the 15 WW OBJNAME rows leave l_objectName"),
    ("dStage_getName", "the reverse lookup of the same table"),
    ("setBaseTRMtx", "a genuine (c)-set host import (J3DModel), 58 sites"),
    ("getManager", "a genuine (c)-set host import (JUTDbPrint)"),
    ("cDyl_IsLinked", "the unguarded DMC[] accessor — present upstream too?"),
]


def exports(path):
    """Exported names from a PE image's export directory."""
    data = path.read_bytes()
    if data[:2] != b"MZ":
        return None
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    if data[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        return None
    coff = e_lfanew + 4
    n_sec = struct.unpack_from("<H", data, coff + 2)[0]
    opt_size = struct.unpack_from("<H", data, coff + 16)[0]
    opt = coff + 20
    magic = struct.unpack_from("<H", data, opt)[0]
    dd = opt + (0x70 if magic == 0x20B else 0x60)
    exp_rva, exp_size = struct.unpack_from("<II", data, dd)
    if not exp_rva:
        return set()
    sect = opt + opt_size

    def off(rva):
        for i in range(n_sec):
            s = sect + i * 40
            va = struct.unpack_from("<I", data, s + 12)[0]
            vsz = struct.unpack_from("<I", data, s + 8)[0]
            raw = struct.unpack_from("<I", data, s + 20)[0]
            if va <= rva < va + max(vsz, 1):
                return raw + (rva - va)
        return None

    base = off(exp_rva)
    if base is None:
        return set()
    n_names = struct.unpack_from("<I", data, base + 24)[0]
    names_rva = struct.unpack_from("<I", data, base + 32)[0]
    names_off = off(names_rva)
    out = set()
    for i in range(n_names):
        nr = struct.unpack_from("<I", data, names_off + i * 4)[0]
        no = off(nr)
        if no is None:
            continue
        end = data.index(b"\0", no)
        out.add(data[no:end].decode("ascii", "replace"))
    return out


def check_symbol(name):
    """Is this name safe to hook BY NAME — on BOTH images?

    Added because §959's finding (5,076 ambiguous names) came with advice
    Housing had no way to follow: "verify a future target occurs exactly once"
    is not actionable without a query. Reports per image: resolvable at all,
    and whether the name identifies exactly ONE function.
    """
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import symbol_manifest as SM
    own = Path(__file__).resolve().parents[2] / "build" / \
        "windows-msvc-relwithdebinfo" / "dusklight.exe"
    verdict = 0
    for label, img in (("our fork", own), ("vanilla upstream", UPSTREAM)):
        if not img.is_file():
            print("  %-16s UNRESOLVED — image not found (not proof of absence)" % label)
            verdict = 2
            continue
        # RAW occurrences, never the collapsed dict (self-caught false SAFE:
        # RES_U32 reported unique while the manifest holds 685 of them).
        occ = SM.occurrences(str(img))
        if occ is None:
            print("  %-16s UNRESOLVED — no manifest in this image" % label)
            verdict = 2
            continue
        matches = {k: n for k, n in occ.items()
                   if k == name or k.endswith("::" + name)}
        total = sum(matches.values())
        hits = sorted(matches)
        if total > len(hits):
            print("  %-16s AMBIGUOUS — %d raw entries under %d name(s): %s"
                  % (label, total, len(hits),
                     ", ".join("%s x%d" % (k, matches[k]) for k in hits[:3])[:70]))
            print("  %-16s repeated entries share one name; by-name binding cannot"
                  % "")
            print("  %-16s pick between them. Hook by address, or qualify." % "")
            verdict = max(verdict, 1)
            continue
        if not hits:
            print("  %-16s MISSING — no manifest entry; cannot hook by this name" % label)
            verdict = max(verdict, 1)
        elif len(hits) == 1:
            print("  %-16s SAFE — resolves to exactly one: %s" % (label, hits[0][:56]))
        else:
            print("  %-16s AMBIGUOUS — %d matches: %s" % (label, len(hits),
                                                          ", ".join(hits[:4])[:70]))
            print("  %-16s a hook on this bare name does not identify one function;"
                  % "")
            print("  %-16s qualify it with the class, or hook by address." % "")
            verdict = max(verdict, 1)
    return verdict


def main():
    if len(sys.argv) > 2 and sys.argv[1] == "--symbol":
        print("HOOK-NAME SAFETY — %s" % sys.argv[2])
        return check_symbol(sys.argv[2])
    exe = Path(sys.argv[1]) if len(sys.argv) > 1 else UPSTREAM
    if not exe.is_file():
        print("UNRESOLVED-LOOKUP: %s not found — NOT proof the symbols are "
              "missing (№31-C)" % exe)
        return 2
    ex = exports(exe)
    if ex is None:
        print("UNRESOLVED-LOOKUP: %s is not a readable PE image" % exe.name)
        return 2
    # THE BINDER'S ACTUAL SURFACE (correction caught before publishing): the PE
    # export table is NOT what by-name resolution uses. dusklight embeds a
    # SYMBOL MANIFEST — 279,447 entries in our fork, 272,796 upstream — and
    # that is what resolve() reads. Checking exports alone reported
    # J3DModel::setBaseTRMtx and JUT*::getManager as MISSING when both are
    # present as CLASS METHODS; the export table carries free functions only.
    # Both surfaces are reported now: exports = link-time visibility,
    # manifest = by-name binding. They answer different questions.
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    try:
        import symbol_manifest as SM
        man = SM.load(str(exe))
    except Exception:
        man = None
    if man is None:
        print("NOTE: image carries NO embedded symbol manifest — by-name resolution")
        print("      is unavailable here, which is a DIFFERENT claim from 'the name")
        print("      is absent' (№31-C).")
        man = {}
    print("UPSTREAM CONFORMANCE — 19b against the build a USER would have")
    print("image : %s" % exe)
    print("        %d bytes · %d exported name(s) · %d manifest entry(ies)"
          % (exe.stat().st_size, len(ex), len(man)))
    print("READ-ONLY: export table + embedded manifest parsed; nothing written,")
    print("           nothing launched, that tree's mods folder untouched.")
    print("=" * 74)
    missing = []
    for sym, why in TARGETS:
        # anchored on the mangled-name boundary, never substring — the
        # dStage_getName / dStage_getName2 lesson (Housing, tale §948)
        pat = re.compile(r"^\?%s@@" % re.escape(sym))
        exp_hits = sorted(n for n in ex if pat.match(n))
        man_hits = sorted(k for k in man if k == sym or k.endswith("::" + sym))
        if exp_hits or man_hits:
            where = ("export+manifest" if exp_hits and man_hits
                     else "export only" if exp_hits else "MANIFEST only")
            shown = (exp_hits[0] if exp_hits else man_hits[0])[:50]
            print("\n  [RESOLVED] %-18s %-50s [%s]" % (sym, shown, where))
            if len(man_hits) > 1:
                print("             manifest matches %d names (%s): endswith('::name')"
                      % (len(man_hits), ", ".join(man_hits[:3])))
                print("             is CLASS-BLIND — the right class is the caller's to confirm.")
        else:
            print("\n  [MISSING ] %-18s absent from BOTH export table and manifest" % sym)
            missing.append(sym)
        print("             why: %s" % why)
    print("\n" + "=" * 74)
    if missing:
        print("VERDICT: %d target(s) MISSING upstream: %s" % (len(missing), ", ".join(missing)))
        print("A missing symbol means the plugin cannot bind that seam on a stock")
        print("build — the delivery claim fails there, and it is far cheaper to")
        print("learn now than after a migration.")
    else:
        print("VERDICT: EVERY binding target RESOLVES in vanilla upstream.")
        print("'Works on any dusklight build' stops being an assumption for this")
        print("surface and becomes evidence — on the build a user actually has.")
    print("\nLIMIT: resolution proves the binder can FIND the symbol in this image.")
    print("It does not prove the hook path RUNS (first-use risk stands), and it")
    print("says nothing about builds not tested here.")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
