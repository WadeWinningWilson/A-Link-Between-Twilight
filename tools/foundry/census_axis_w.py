#!/usr/bin/env python3
# ============================================================================
# census_axis_w.py — AXIS W, interface width (census spec §4). THE PLUGIN ABI.
#
# MEASURE, per subsystem:
#   (i)  ENTRY POINTS  — symbols the subsystem defines that code OUTSIDE it
#                        calls, with every call site as file:line
#   (ii) CALLBACKS     — receiver-native symbols the subsystem calls back out
#                        to, with every call site as file:line
#
# The union of the interface edges of all WHOLESALE-verdict subsystems IS the
# hook interface the receiver must expose (spec §4). That is why the plugin
# architecture cannot be costed today and can be the moment this runs.
#
# ---------------------------------------------------------------------------
# EVERY NUMBER THIS AXIS PRODUCES IS A **LOWER BOUND**. STATED, NOT IMPLIED.
#
# Axis C's edge sets are structurally incomplete and W inherits all of it:
#
#   1. INDIRECT DISPATCH. 16 of 16 direct-port actor TUs, and most others, use
#      virtual calls / method tables (`actor_method_class`, `process_method_func`).
#      Static analysis cannot see through them. Spec §10 is explicit: report
#      UNKNOWN where dispatch is indirect, never internal. A subsystem reached
#      only through a vtable has an entry point this axis CANNOT count.
#   2. MEMBER CALLS. `obj->foo(` targets a method on an object's type, which a
#      free-symbol index does not resolve. Axis C v6 gave these their own class
#      rather than miscounting them; here they are simply invisible.
#   3. IMPLICIT COUPLING. Spec §10's own limit: the census sees CALLS, not
#      shared global state. `g_env_light`-style contracts and the §113 PAL0
#      stash are real interface surface that no call graph shows.
#   4. RESIDUAL UNRESOLVED. Axis C has 128 `unresolved` edges (renamed from
#      `receiver-absent`, which overclaimed: they sample as stdlib symbols
#      the index cannot resolve). Any that are genuinely receiver-native are
#      missing callbacks here.
#
# Consequence for the ABI estimate, in the direction that matters: **the true
# hook interface is WIDER than this axis reports, never narrower.** An ABI
# costed from these numbers is a floor. Anyone sizing Stage D from it must
# treat it as such — which is exactly the misuse the posture doc was written
# about ("a true statement about one thing read as a statement about another").
#
# ---------------------------------------------------------------------------
# RECEIPT STANDARD (HousingTemp §3a, non-negotiable): every interface edge
# carries named file and line. "It's gated" is not a receipt, and comments
# claiming a gate have been wrong before — so this records the call site, which
# cannot be wrong about itself.
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402  (shared classifier + grouping)

REPO = C.REPO
CALL_LINE = re.compile(r"\b([A-Za-z_]\w{2,})\s*\(")


def _defined_symbols(idx, subsystem_files):
    """Symbols this subsystem exports UNAMBIGUOUSLY.

    A name qualifies only when every declaration of it in the indexed tree sits
    inside this subsystem. If any other file also declares it, a call site
    naming it cannot be attributed here -- see the module docstring for the
    JPA `memcpy`/`find`/`pos` case that made this rule necessary.

    Returns (exports, ambiguous): ambiguous names are surfaced, not discarded,
    because "this subsystem's exports collide with common identifiers" is a real
    property of its interface rather than a measurement error."""
    out, ambiguous = {}, {}
    for name, decls in idx.items():
        here = [d for d in decls if d in subsystem_files]
        if not here:
            continue
        # Platform beats local for attribution. A name the platform layer also
        # declares (memcpy, be16, be32 ...) is not this subsystem's export, and
        # its call sites across the build are not this subsystem's interface.
        if any(any(d.startswith(pr) for pr in C.PLATFORM_ROOTS) for d in decls):
            continue
        # A member function is not an entry point: it cannot be called
        # without an object of its type, so it is not something a receiver
        # "enters the subsystem through". Only free-scope declarations qualify.
        if not any(name in C.FREE_DECLS.get(d, set()) for d in here):
            continue
        elsewhere = [d for d in decls if d not in subsystem_files]
        if elsewhere:
            ambiguous[name] = {"in_subsystem": here[0],
                               "also_declared_in": sorted(set(elsewhere))[:4],
                               "other_decl_count": len(set(elsewhere))}
            continue
        out[name] = here[0]
    return out, ambiguous


def run(roster, all_sources, axis_c_result):
    idx = C.build_decl_index()

    # subsystem -> its files, and the header/impl files that belong to it
    subs = {}
    for rel in roster:
        subs.setdefault(C.group_of(rel), []).append(rel)

    scope = {}
    for sub, files in subs.items():
        fileset = set(files)
        # Same per-file parallel-path scoping as Axis C v7. Directory
        # membership is not subsystem membership -- that error is what produced
        # 13,195 "entry points" for a 4-file subsystem.
        own = set()
        for f in files:
            fp = f.replace(chr(92), "/")
            stem = Path(fp).stem
            parent = str(Path(fp).parent).replace(chr(92), "/")
            cands = {parent + "/" + stem + ".h", parent + "/" + stem + ".hpp"}
            if fp.startswith("src/"):
                ip = parent.replace("src/", "include/", 1)
                cands |= {ip + "/" + stem + ".h", ip + "/" + stem + ".hpp"}
            if "/src/" in fp:
                ip = parent.replace("/src/", "/include/", 1)
                cands |= {ip + "/" + stem + ".h", ip + "/" + stem + ".hpp"}
            own |= cands
        all_decl = {d for v in idx.values() for d in v}
        scope[sub] = fileset | (own & all_decl)

    _pairs = {sub: _defined_symbols(idx, scope[sub]) for sub in subs}
    exports = {s: v[0] for s, v in _pairs.items()}
    ambiguous = {s: v[1] for s, v in _pairs.items()}

    # ---- one pass over the WHOLE build: who calls whom, with line numbers ---
    # Scanning only the roster would miss the receiver's calls INTO the
    # subsystem, which is precisely half of what this axis measures.
    callsites = {}
    for rel in all_sources:
        p = REPO / rel
        if not p.is_file():
            continue
        try:
            txt = C._callable_text(p.read_text(encoding="utf-8", errors="replace"))
        except OSError:
            continue
        for m in CALL_LINE.finditer(txt):
            n = m.group(1)
            line = txt[:m.start()].count(chr(10)) + 1
            callsites.setdefault(n, []).append((rel, line))

    out = {}
    for sub, files in sorted(subs.items()):
        fileset = set(files)
        entry = {}
        for name, decl in exports[sub].items():
            sites = [(f, l) for f, l in callsites.get(name, [])
                     if f not in fileset]
            if sites:
                entry[name] = {"declared_in": decl, "sites": sites[:8],
                               "site_count": len(sites)}

        # Ambiguous exports that are actually called from outside: these are
        # entry points we cannot ATTRIBUTE with certainty, not entry points we
        # have shown do not exist. They set the upper bound.
        amb_called = {}
        for name, info in ambiguous[sub].items():
            sites = [(f, l) for f, l in callsites.get(name, [])
                     if f not in fileset]
            if sites:
                amb_called[name] = {"in_subsystem": info["in_subsystem"],
                                    "also_declared_in": info["also_declared_in"],
                                    "sites": sites[:4],
                                    "site_count": len(sites)}

        cb = {}
        cedges = axis_c_result.get(sub, {}).get("edges", {})
        for name, e in cedges.items():
            if e["class"] != "receiver-native":
                continue
            sites = [(f, l) for f, l in callsites.get(name, []) if f in fileset]
            cb[name] = {"declared_in": e["declared_in"], "sites": sites[:8],
                        "site_count": len(sites)}

        out[sub] = {
            "files": files,
            "entry_points": entry,
            "callbacks": cb,
            "ambiguous_exports": ambiguous[sub],
            "ambiguous_called_externally": amb_called,
            "width_lower_bound": len(entry) + len(cb),
            "width_upper_bound": len(entry) + len(cb) + len(amb_called),
            "indirect_dispatch_tus": axis_c_result.get(sub, {})
                                                  .get("indirect_dispatch_tus", 0),
            "member_call_edges": axis_c_result.get(sub, {})
                                              .get("counts", {})
                                              .get("member-call", 0),
        }
    return out
