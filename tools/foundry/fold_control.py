#!/usr/bin/env python3
# ============================================================================
# fold_control.py — the C3 control harness for 19c's FOLDED detector (§527).
#
# WHO MAY RUN WHAT, AND WHY THIS FILE EXISTS
# 19c's author re-spec'd FOLDED from MULTI_NAME to address-collision (§526) and
# then wrote its control (§527), declaring the conflict themselves: writing the
# control for your own spec is marking your own homework. They asked Foundry or
# Housing to execute it.
#
# Executing it turned out to be impossible: **no detector exists**. Nothing in
# src/dusk/mods or tools implements address-collision detection. So this is the
# HARNESS -- fixtures and exact expected outputs -- not a result. Foundry may
# write the harness without marking its own homework because Foundry authored
# neither the detector nor the re-spec it controls.
#
# Until a detector is supplied, every part reports NOT CONTROLLED. Per this
# campaign's standard that is recorded as its own state, never as a blank that
# later reads clean, and never as PASS.
#
# THE DETECTOR'S CONTRACT (from §526/§527)
#   detect(manifest, symmap) -> list of collisions, each naming the shared
#   address AND every symbol at it. A verdict of "FOLDED" that does not name the
#   collision set is not a receipt and fails C3a.
#     manifest : {name: (rva, flags)} -- what resolve() can see
#     symmap   : {name: rva}          -- the address map, INCLUDING symbols the
#                                        manifest does not carry (this is what
#                                        C3c turns on)
#
# Usage: fold_control.py [--detector module:function]
# ============================================================================
import importlib
import sys

# --- fixtures ---------------------------------------------------------------
# C3a: two DISTINCT symbols at one address, one of them a manifest entry.
A_MANIFEST = {"cM_ssin": (0x17F350, 0x1), "cM_scos": (0x17F330, 0x1)}
A_SYMMAP = {"cM_ssin": 0x17F350, "daFoo_c::bar": 0x17F350, "cM_scos": 0x17F330}
A_EXPECT = {0x17F350: {"cM_ssin", "daFoo_c::bar"}}

# C3b: same shape, collision removed. Every symbol at a unique address.
B_MANIFEST = dict(A_MANIFEST)
B_SYMMAP = {"cM_ssin": 0x17F350, "daFoo_c::bar": 0x17F999, "cM_scos": 0x17F330}
B_EXPECT = {}

# C3c: the collision is between a MANIFEST symbol and a NON-MANIFEST symbol.
# This is the part that separates the two plausible implementations: a
# manifest-INTERNAL comparison passes C3a and misses this entirely, which is
# exactly the `push_back` case. C3a alone cannot tell them apart.
# FIXTURE CORRECTED BEFORE THE DETECTOR WAS WRITTEN, and disclosed as an error
# of mine: the first version used `push_back` vs `std::vector<int>::push_back`,
# which is an ALIAS PAIR and therefore contradicts C3d outright. C3c's purpose is
# SCOPE -- a collision spanning a manifest and a non-manifest symbol -- not
# aliasing. Two genuinely distinct functions now carry it.
C_MANIFEST = {"push_back": (0x200000, 0x1)}
C_SYMMAP = {"push_back": 0x200000, "daFoo_c::update": 0x200000}
C_EXPECT = {0x200000: {"push_back", "daFoo_c::update"}}

# A mangled/demangled ALIAS PAIR is NOT a collision -- it is one function under
# two names, which is what 68.3% of the real manifest looks like (§525). A
# detector that flags this would refuse most of the symbol table.
D_MANIFEST = {"cM_ssin": (0x17F350, 0x1)}
D_SYMMAP = {"cM_ssin": 0x17F350, "?cM_ssin@@YAMF@Z": 0x17F350}
D_EXPECT = {}

CASES = [
    ("C3a POSITIVE — distinct symbols share an address", A_MANIFEST, A_SYMMAP,
     A_EXPECT, "FOLDED fires, naming both symbols and the address"),
    ("C3b NEGATIVE — no collision", B_MANIFEST, B_SYMMAP, B_EXPECT,
     "must NOT fire; a detector that always fires is as dead as one that never does"),
    ("C3c SCOPE — collision spans manifest and NON-manifest symbol",
     C_MANIFEST, C_SYMMAP, C_EXPECT,
     "fires; a manifest-internal comparison passes C3a and MISSES this"),
    ("C3d ALIAS — mangled/demangled pair of ONE function", D_MANIFEST, D_SYMMAP,
     D_EXPECT, "must NOT fire; this shape is 68.3% of the real manifest (§525)"),
]


def main():
    argv = sys.argv[1:]
    detector = None
    if "--detector" in argv:
        spec = argv[argv.index("--detector") + 1]
        mod, _, fn = spec.partition(":")
        detector = getattr(importlib.import_module(mod), fn or "detect")

    print("C3 CONTROL HARNESS — 19c FOLDED detector (address-collision, §526)")
    print("harness: Foundry. detector + re-spec: 19c's author. Separated on")
    print("purpose — a control that passes when run by its own author proves")
    print("nothing.\n")

    if detector is None:
        print("  DETECTOR: ABSENT — nothing in src/dusk/mods or tools implements")
        print("  address-collision detection.\n")
        for name, _m, _s, exp, why in CASES:
            print(f"  {name}")
            print(f"      expected: {why}")
            print(f"      collisions: {exp if exp else 'none'}")
            print(f"      RESULT: NOT CONTROLLED — no detector to run\n")
        print("  C3e REALITY (real ICF-on map): NOT CONTROLLED — blocked on a")
        print("  .map that is not emitted and a comparison build that does not")
        print("  exist. Must not be conflated with a-d passing.\n")
        print("  VERDICT: NOT CONTROLLED (5 of 5). This is the absence of a")
        print("  result, not a result. Re-run with --detector once one exists.")
        return 2

    ok = True
    for name, man, sym, exp, why in CASES:
        got = detector(man, sym)
        norm = {addr: set(names) for addr, names in dict(got or {}).items()}
        good = norm == exp
        ok &= good
        print(f"  {name}\n      expected {exp or 'none'}\n      got      "
              f"{norm or 'none'}\n      {'PASS' if good else 'FAIL'} — {why}\n")
    print("  C3e REALITY: NOT CONTROLLED — blocked on an ICF-on comparison build.")
    print("  VERDICT:", "a-d PASS (C3e still uncontrolled)" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
