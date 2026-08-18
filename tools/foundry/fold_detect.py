#!/usr/bin/env python3
# ============================================================================
# fold_detect.py — 19c's FOLDED detector: address collision, not MULTI_NAME.
#
# REFERENCE IMPLEMENTATION. This is the logic 19c's load-time gate needs; it
# lives here so it can be controlled by tools/foundry/fold_control.py before
# being ported into the mod loader. Pure function of (manifest, symmap), no
# image and no build required -- which is the whole reason the address-collision
# re-spec (§526) is testable where MULTI_NAME was not.
#
# WHY MULTI_NAME COULD NOT BE THE TRIGGER (§525, measured)
# MULTI_NAME is set on 186,396 of 272,796 symbols -- 68.3%, averaging 1.51 names
# per address -- because the manifest stores each function under BOTH its
# decorated symbol and its demangled display name:
#     cM_ssin @0x17f350 -> ['cM_ssin', '?cM_ssin@@YAMF@Z']
# A gate refusing on that flag refuses two thirds of the symbol table. The real
# hazard is narrower: two DISTINCT functions at one address.
#
# SO THE ENTIRE DIFFICULTY IS TELLING THOSE APART, and that is what this does.
#
# AUTHORSHIP, STATED PLAINLY
# Foundry wrote both this detector and its control harness. That pairing is the
# anti-pattern this campaign keeps naming. The one real mitigation: the harness
# fixtures and their expected outputs were PUBLISHED FIRST (§528) and are
# unchanged since, except C3c -- corrected before this file existed and
# disclosed as an error, not quietly refitted. Pre-registered expectations are
# weaker than an independent auditor. **HousingTemp or Housing should still
# re-run and re-audit; a passing control run by its own author proves little.**
#
# Read-only. Usage: fold_detect.py <image.exe>   (runs against a real manifest)
# ============================================================================
import re
import sys
from collections import defaultdict
from pathlib import Path

C_IMPORTS = Path(__file__).resolve().parents[2] / "docs/WW Linked/ww-import-manifest.txt"

# `?name@...`, `??0Class@...` (ctor), `??1Class@...` (dtor), `??$name@...`
DECORATED = re.compile(r"^\?{1,2}[$_0-9A-Z]?([A-Za-z_]\w*)@")


def base_identifier(name):
    """The function's own name, with scope, decoration and templates removed.

    Two names denote the SAME function if this agrees -- that is what separates
    a mangled/demangled alias pair from two folded functions."""
    m = DECORATED.match(name)
    if m:
        return m.group(1)
    tail = name.split("::")[-1]
    tail = re.sub(r"<.*>", "", tail)        # template args
    tail = tail.split("(")[0]               # signature
    return tail.lstrip("~").strip()


def detect(manifest, symmap):
    """Addresses holding two or more DISTINCT functions.

    Returns {rva: [names]}. Reports only collisions involving at least one
    manifest symbol: a collision between two names resolve() can never return
    is not reachable by a plugin and is not this gate's business.

    An alias GROUP -- every name at the address sharing one base identifier --
    is not a collision. That is the 68.3% case, and treating it as folding is
    what would refuse most of the symbol table."""
    by_addr = defaultdict(list)
    for name, rva in symmap.items():
        by_addr[rva].append(name)

    out = {}
    for rva, names in by_addr.items():
        if len(names) < 2:
            continue
        if not any(n in manifest for n in names):
            continue
        # ALIAS TEST — counts decorated symbols; does NOT try to demangle.
        #
        # The first version compared demangled base identifiers and passed every
        # fixture while reporting 41,351 collisions on a /OPT:NOICF binary that
        # cannot fold. MSVC's special-name grammar defeated it -- `??_E...`
        # (vector deleting destructor), `??$?0...` (template ctor), `??$?8...`
        # (operator==) do not yield an identifier where a plain `?name@` does.
        # Passing fixtures proved the cases I had thought of, nothing more.
        #
        # This needs no demangling: MSVC emits ONE decorated symbol per function,
        # plus display aliases. So one decorated name at an address is one
        # function however many display forms accompany it; TWO distinct
        # decorated names is two functions sharing an address, which is folding.
        decorated = {n for n in names if n.startswith("?")}
        plain = {base_identifier(n) for n in names if not n.startswith("?")}
        if len(decorated) <= 1 and len(plain) <= 1:
            continue                    # aliases of one function
        out[rva] = sorted(names)
    return out


def main():
    if len(sys.argv) < 2:
        print("usage: fold_detect.py <image.exe>")
        return 2
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import symbol_manifest as SM

    table = SM.load(sys.argv[1])
    if table is None:
        print("NO MANIFEST EMBEDDED — UNKNOWN, not clean (№31-C).")
        return 2
    # COLLAPSED-DICT EXPOSURE, MEASURED AND STATED (tale §967 sweep): `table`
    # is keyed by NAME, so ~15.3k raw entries whose names repeat never reach
    # this map. For a FOLD detector that matters in a specific direction —
    # folding is many NAMES sharing one ADDRESS, and names lost before the map
    # is built can only cause folds to be MISSED, never invented. So this
    # tool's counts are a FLOOR. Not silently changing its algorithm here (it
    # carries its own §530 scope fix and I have not re-derived that); the
    # exposure is printed so no one reads its number as complete.
    symmap = {n: rva for n, (rva, _f) in table.items()}
    try:
        _occ = SM.occurrences(sys.argv[1]) or {}
        _lost = sum(v - 1 for v in _occ.values() if v > 1)
        if _lost:
            print("NOTE: %d raw manifest entr(ies) share a name with another and "
                  "do not\n      reach the address map. Fold counts below are a "
                  "FLOOR, not a total." % _lost)
    except Exception:
        pass

    # SCOPE FIX (§530). This passed `table` as BOTH arguments, so the
    # "involves a symbol we will resolve" filter tested whether a symbol from
    # the symbol table was in the symbol table -- always true, never filtering.
    # The 2,168 it reported was every collision in the image, not the ones that
    # can reach us, and only the separate ad-hoc check in §529 produced the
    # number the gate actually needs. The tool must produce it itself.
    #
    # `manifest` is what WE will resolve: the import list. `symmap` is the whole
    # address map. That asymmetry is the entire point of the two arguments.
    imports_path = C_IMPORTS
    if "--imports" in sys.argv:
        imports_path = Path(sys.argv[sys.argv.index("--imports") + 1])
    # C7 (§532): assert against the 21 MANIFEST ENTRIES, and verify the stamp
    # rather than a remembered number. The manifest is the authority precisely
    # because it is generated, provenance-stamped and checkable -- so checking
    # it is the point, not a formality. Comparing against the 249 expansion
    # would compare incompatible quantities and look right either way.
    import hashlib
    want, stamp, declared = [], {}, None
    if imports_path.is_file():
        for line in imports_path.read_text(encoding="utf-8").splitlines():
            if line.startswith("#"):
                if ":" in line:
                    k, _, v = line[1:].partition(":")
                    stamp[k.strip()] = v.strip()
                continue
            if line.strip():
                want.append(line.split("\t")[0])
        declared = stamp.get("count")

    gen = C_IMPORTS.parent.parent.parent / "tools/foundry/binding_plan.py"
    fresh = (stamp.get("tool_sha256") ==
             hashlib.sha256(gen.read_bytes()).hexdigest()) if gen.is_file() else None
    count_ok = declared is not None and int(declared) == len(want)

    ours = {}
    unresolved = []
    for n in want:
        cand = [k for k in table if k == n or k.endswith("::" + n)]
        if cand:
            for k in cand:
                ours[k] = table[k]
        else:
            unresolved.append(n)

    cols_all = detect(table, symmap)          # whole image, for context only
    cols = detect(ours, symmap) if ours else {}

    print(f"FOLD DETECT — {sys.argv[1]}")
    print(f"  image symbols        : {len(table)}")
    print(f"  IMPORT MANIFEST      : {len(want)} entries  <- C7 asserts against "
          f"THIS, never the expansion")
    print(f"     declared count    : {declared}  "
          f"{'OK' if count_ok else 'MISMATCH — manifest disagrees with itself'}")
    # §535: tool_sha256 alone reported FRESH on a manifest whose count was
    # already wrong (recorded 21, live tool computed 15 in the same tree). It
    # covers the GENERATOR, not the roster the count derives from. A stamp the
    # gate does not check is decoration, so the gate checks it.
    # the stamp value carries a trailing explanatory comment; take the hash
    _drv_raw = (stamp.get("derivation_sha256") or "").strip()
    drv = _drv_raw.split()[0] if _drv_raw else None
    drv_state = "ABSENT — manifest predates the derivation stamp (UNKNOWN, not clean)"
    if drv:
        try:
            sys.path.insert(0, str(Path(__file__).resolve().parent))
            import binding_plan as _BP
            live_drv = _BP.derivation_sha256()
            if live_drv is None:
                drv_state = "UNKNOWN — could not recompute (never 'clean')"
            elif live_drv == drv:
                drv_state = "FRESH — roster contents unchanged since generation"
            else:
                drv_state = ("STALE — the roster CHANGED since this manifest was "
                             "generated; the count below is not current. Regenerate.")
        except Exception as e:
            drv_state = f"UNKNOWN — recompute failed ({type(e).__name__})"
    print(f"     derivation freshness: {drv_state}")
    print(f"     generator freshness: "
          f"{'FRESH — tool_sha256 matches the live generator' if fresh else
             'STALE — regenerate before trusting any number below' if fresh is False
             else 'UNKNOWN — generator not found'}")
    print(f"     expansion         : -> {len(ours)} candidate symbols, "
          f"{len(unresolved)} unresolved. The expansion is a SUPERSET "
          f"(endswith '::name' on generic names like path/getenv), so a zero "
          f"over it is conservative — but it is NOT the C7 quantity.")
    if unresolved:
        print(f"     unresolved: {', '.join(unresolved)}")
    print(f"  collisions IN IMAGE  : {len(cols_all)}   (context, not the gate)")
    print(f"  COLLISIONS REACHING OUR IMPORTS : {len(cols)}   <- the gate's number")
    for rva, names in list(sorted(cols.items()))[:10]:
        print(f"    0x{rva:x}: {names}")
    if len(cols) > 10:
        print(f"    … {len(cols) - 10} more")
    if not cols:
        print("    none — no collision in this image reaches a symbol we resolve.")
        print("    A zero here is a MEASUREMENT, not a silence: the same detector")
        print("    fires on the C3a/C3c fixtures, and reports "
              f"{len(cols_all)} collisions")
        print("    elsewhere in this very image. Both directions demonstrated.")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
