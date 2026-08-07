#!/usr/bin/env python3
# ============================================================================
# census_axis_p.py — AXIS P, platform-layer depth (census spec §5).
#
# A HARD VETO, NOT A SCORE. Any subsystem that duplicates singleton runtime
# state the receiver must own singularly is PIECEWISE regardless of how well it
# scores on C, D and W (spec §5).
#
# MEASURE: does the subsystem define file-scope MUTABLE runtime state whose TYPE
# the receiver also instantiates?
#
# ---------------------------------------------------------------------------
# WHY THIS AXIS REFUSES TO DECIDE ONE THING
#
# Spec §5 draws a line the tree cannot draw for itself: two JAudio stacks
# coexist fine (separate players over separate data, already shipping) while two
# J3D implementations, two kernels or two allocators do not. The difference is
# not visible in the code -- both look like "a type instantiated twice". It is a
# judgment about which resources the hardware forces to be singular.
#
# So this axis reports EVIDENCE and a severity, never a final verdict:
#
#   VETO-CANDIDATE  the type is declared under a PLATFORM root -- device,
#                   allocator, kernel, render state. These are the ones the
#                   receiver must own singularly, so duplication is the class
#                   spec §5 vetoes.
#   DUPLICATED      the type is declared in a receiver-owned file and is
#                   instantiated both inside and outside the subsystem. Real
#                   duplication, but whether it MUST be singular is the ruling
#                   spec §5 reserves -- JAudio is exactly this shape and is
#                   sanctioned.
#   VALUE-TYPE      the receiver instantiates the type at many independent
#                   file-scope sites, so it is data (cXyz, TColor), not a
#                   singleton. A singleton has one instance -- definitional,
#                   not a judgment.
#   SOLE-INSTANCE   a RECEIVER-declared type of which this subsystem holds
#                   the only file-scope instance in the build. NOT
#                   duplication -- it was mislabelled DUPLICATED until
#                   Finding A, which put 7 of 8 findings into a §5 ruling
#                   queue with nothing to rule on. Reported because the type
#                   must exist across a plugin boundary.
#   OWN-STATE       the type is the subsystem's own. Not duplication.
#   UNKNOWN         the type could not be resolved. Per №31-C this is never
#                   reported as clean: an unresolved type is exactly where a
#                   duplicated allocator would hide.
#
# A false positive here kills a wholesale candidate that should have shipped, so
# the tool states what it found and who must rule, rather than ruling itself.
#
# ---------------------------------------------------------------------------
# HONEST LIMITS
#   * Singletons reached through pointers/factories (`getInstance()`) are not
#     file-scope objects and are INVISIBLE here.
#   * `const` tables are excluded deliberately -- static read-only data is not
#     runtime state and cannot be fought over.
#   * Function-local statics are out of scope: they are not the shared-global
#     class this axis is about.
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402

REPO = C.REPO

# A file-scope object definition: optional storage class, a type token, a name,
# no parentheses before the terminator (that would be a function declaration).
GLOBAL_DEF = re.compile(
    r"^(?P<stor>static\s+|extern\s+)?"
    r"(?P<type>[A-Za-z_][\w:]*(?:\s*<[^;{}]*>)?)\s+"
    r"(?P<name>[A-Za-z_]\w*)\s*"
    r"(?:\[[^\];]*\])?"
    r"\s*(?:=[^;]*)?;",
    re.M)

# Declared, not tuned. Above this many independent receiver-side file-scope
# instantiations, a type is data rather than a singleton (see patch note).
VALUE_TYPE_INSTANCES = 3

SKIP_TYPES = {
    # declarations, not object definitions
    "return", "typedef", "using", "namespace", "class", "struct", "union",
    "enum", "template", "friend", "public", "private", "protected", "case",
    "else", "const", "inline", "virtual", "explicit", "operator",
}


NS_OPEN = re.compile(r"^\s*namespace\b[^{]*\{")


def _file_scope_globals(txt):
    """Mutable file-scope objects, INCLUDING those inside namespace bodies.

    v1 skipped anything at brace depth > 0, which excluded every global declared
    in the idiomatic `namespace { ... }` block -- i.e. most of them -- and
    returned zero findings for five subsystems. On a veto axis, a false zero
    clears a subsystem that was never examined, so block KIND is tracked rather
    than raw depth: namespaces keep file scope, class/struct/function bodies do
    not.

    `const`/`constexpr` are excluded deliberately: read-only data is not runtime
    state and cannot be fought over."""
    out = []
    stack = []          # one entry per open brace: True = namespace
    for line in C._split_braces(txt).splitlines():
        stripped = line.strip()
        at_file_scope = all(stack)
        if at_file_scope and stripped and not stripped.startswith(("#", "//")):
            if "const " not in stripped and "constexpr" not in stripped:
                m = GLOBAL_DEF.match(stripped)
                if m:
                    ty = m.group("type").split("::")[-1].strip()
                    if ty not in SKIP_TYPES and not ty.endswith(")"):
                        out.append((ty, m.group("name"), m.group("stor") or ""))
        opens = line.count("{")
        closes = line.count("}")
        if opens:
            is_ns = bool(NS_OPEN.match(line))
            for i in range(opens):
                stack.append(is_ns and i == 0)
        for _ in range(closes):
            if stack:
                stack.pop()
    return out


def _decl_owner_map(subs, all_sources):
    """Resolve who DECLARES a header: a WW subsystem, or the receiver.

    Two claims, in order of strength:
      1. STEM MATCH -- `src/<d>/<stem>.cpp` owns `<d>/<stem>.h` and the
         `include/` mirror of it. This is Axis W's `own` rule, ported; P lacked
         it entirely, which is why a subsystem's own types read as receiver ones.
      2. EXCLUSIVE-DIRECTORY MIRROR -- the include mirror of a source directory,
         but ONLY when every build source in that directory belongs to the one
         subsystem. Needed because a type can be declared in a header with no
         same-stem .cpp (`Ja1Track` in ja1_track.h, `DSPBuffer` in
         ja1_dsp_boundary.h), and refused for shared dirs like `src/d` per Axis
         C v7's directory-is-not-membership finding.

    Returns (stem_owner: header -> sub, dir_owner: include-dir -> sub)."""
    owner_of_src = {}
    for sub, files in subs.items():
        for f in files:
            owner_of_src[f.replace(chr(92), "/")] = sub

    dir_owners = {}
    for s in all_sources:
        fp = s.replace(chr(92), "/")
        d = fp.rsplit("/", 1)[0] if "/" in fp else ""
        dir_owners.setdefault(d, set()).add(owner_of_src.get(fp))

    def _mirror(d):
        if d.startswith("src/"):
            return d.replace("src/", "include/", 1)
        if "/src/" in d:
            return d.replace("/src/", "/include/", 1)
        return None

    stem_owner, dir_owner = {}, {}
    for sub, files in subs.items():
        for f in files:
            fp = f.replace(chr(92), "/")
            parent = fp.rsplit("/", 1)[0] if "/" in fp else ""
            stem = fp.rsplit("/", 1)[-1].rsplit(".", 1)[0]
            for base in (parent, _mirror(parent)):
                if base:
                    for ext in (".h", ".hpp"):
                        stem_owner[base + "/" + stem + ext] = sub
            # exclusivity gate -- the whole point of the rule
            if dir_owners.get(parent) == {sub}:
                m = _mirror(parent)
                if m:
                    dir_owner[m] = sub
    return stem_owner, dir_owner


def _owner_of_decl(decl, stem_owner, dir_owner):
    """The subsystem that declares `decl`, or None for the receiver."""
    d = decl.replace(chr(92), "/")
    if d in stem_owner:
        return stem_owner[d]
    parent = d.rsplit("/", 1)[0] if "/" in d else ""
    return dir_owner.get(parent)


def run(roster, all_sources):
    idx = C.build_decl_index()

    subs = {}
    for rel in roster:
        subs.setdefault(C.group_of(rel), []).append(rel)
    roster_set = set(roster)
    stem_owner, dir_owner = _decl_owner_map(subs, all_sources)

    def _owner_label(path):
        o = _owner_of_decl(path, stem_owner, dir_owner)
        return o if o else "the receiver"

    # Where is each type instantiated at file scope, across the whole build?
    inst = {}
    for rel in all_sources:
        p = REPO / rel
        if not p.is_file():
            continue
        try:
            txt = C._callable_text(p.read_text(encoding="utf-8", errors="replace"))
        except OSError:
            continue
        for ty, name, stor in _file_scope_globals(txt):
            inst.setdefault(ty, []).append((rel, name, stor))

    # HT-28: STAMP THE ROSTER INTO THE OUTPUT.
    #
    # Axis P's verdicts depend on which roster it is handed, and nothing in the
    # output said which. Two lanes ran this tool on the same tree in the same
    # hour and got different §5 ruling queues -- SOLE-INSTANCE 3 vs 4 -- because
    # one passed the 47-entry manifest and the other the 72-entry union. The
    # union is correct: the manifest roster excludes the leg-carriers, which is
    # exactly where a receiver type owned on both sides shows up.
    #
    # Neither lane could have caught it from the output, because the output did
    # not carry its own input. An unrecorded input and unrecorded state are the
    # same defect, and a verdict that cannot name its roster is not reproducible.
    import hashlib
    roster_stamp = {
        "count": len(roster_set),
        "sha256": hashlib.sha256(
            chr(10).join(sorted(roster_set)).encode("utf-8")).hexdigest()[:16],
        "subsystems": len(subs),
        "note": ("§5 ruling-queue verdicts are relative to THIS roster; a "
                 "different roster is a different queue, not a disagreement"),
    }

    out = {"__roster__": roster_stamp}
    for sub, files in sorted(subs.items()):
        fileset = set(files)
        findings, counts = [], {}
        for ty, sites in inst.items():
            mine = [s for s in sites if s[0] in fileset]
            if not mine:
                continue
            # HT-21: three outcomes, not two.
            #
            # Only class/struct types can be a duplicated singleton -- a scalar
            # global is not shared hardware state, and treating it as one made
            # `bool s_loaded` collide with 251 unrelated bools. But the v3 filter
            # discarded ANY type not in TYPE_NAMES, which swept undeclared types
            # out through the same door and made the UNKNOWN path unreachable.
            #
            # An unresolvable type is precisely where a duplicated allocator
            # would hide, so it is reported, never dropped.
            unresolved_type = ty not in C.TYPE_NAMES and ty not in idx
            if ty not in C.TYPE_NAMES and not unresolved_type:
                continue  # declared alias/scalar -- not a singleton type
            theirs = [s for s in sites if s[0] not in fileset
                      and s[0] not in roster_set]

            decls = idx.get(ty, [])
            if unresolved_type or not decls:
                verdict, why = ("UNKNOWN",
                                "type resolves to no declaration anywhere in "
                                "the indexed tree -- cannot be shown clear, and "
                                "an unresolvable type is where a duplicated "
                                "allocator would hide")
            elif any(any(d.startswith(pr) for pr in C.PLATFORM_ROOTS)
                     for d in decls):
                verdict, why = ("VETO-CANDIDATE",
                                "platform-declared type (device/allocator/"
                                "kernel/render state)")
            elif len(theirs) > VALUE_TYPE_INSTANCES:
                verdict, why = ("VALUE-TYPE",
                                f"instantiated outside this subsystem at "
                                f"{len(theirs)} independent file-scope sites -- "
                                f"a singleton has one instance, so this is data, "
                                f"not shared runtime state")
            elif theirs:
                # HT-26: the old reason ASSERTED "receiver" without testing it.
                # For a spec §5 ruling queue "the receiver also owns one" and
                # "another WW subsystem also owns one" are different questions
                # with different answers, so name the site and let the reader
                # see which.
                where = ", ".join(f"{f}:{n}" for f, n, _s in theirs[:3])
                verdict, why = ("DUPLICATED",
                                f"instantiated outside this subsystem, at "
                                f"{where}")
            elif any(d in fileset or Path(d).stem in {Path(f).stem for f in files}
                     for d in decls):
                verdict, why = "OWN-STATE", "type belongs to this subsystem"
            else:
                # Finding A: reachable only when `theirs` is EMPTY, so nothing
                # is duplicated -- this subsystem holds the ONLY file-scope
                # instance in the build. Labelling it DUPLICATED contradicted
                # this module's own definition and put 7 of 8 findings into a
                # human ruling queue that had nothing to rule on.
                # HT-25: the old reason asserted "receiver-declared", and that
                # clause was false for half the list. Resolve the declaring
                # owner instead of assuming it.
                owners = {_owner_of_decl(d, stem_owner, dir_owner)
                          for d in decls}
                ww_owners = {o for o in owners if o}
                if ww_owners and None not in owners:
                    other = sorted(ww_owners)
                    if other == [sub]:
                        verdict, why = ("OWN-STATE",
                                        "type is declared by this subsystem's "
                                        "own headers")
                    else:
                        verdict = "SOLE-INSTANCE-WW"
                        why = (f"declared by WW subsystem "
                               f"{' / '.join(other)}, not by the receiver -- it "
                               f"travels WITH that subsystem across a plugin "
                               f"boundary and owes the receiver side nothing")
                else:
                    verdict, why = ("SOLE-INSTANCE",
                                    "receiver-declared type, and this subsystem "
                                    "holds the only file-scope instance in the "
                                    "build -- not duplication; relevant only "
                                    "because the type must exist across a plugin "
                                    "boundary")

            counts[verdict] = counts.get(verdict, 0) + 1
            if verdict in ("VETO-CANDIDATE", "DUPLICATED", "UNKNOWN",
                           "SOLE-INSTANCE", "SOLE-INSTANCE-WW"):
                findings.append({
                    "type": ty, "verdict": verdict, "why": why,
                    "declared_in": decls[0] if decls else None,
                    "here": mine[:3], "receiver_sites": theirs[:3],
                    "receiver_site_count": len(theirs),
                })

        findings.sort(key=lambda f: {"VETO-CANDIDATE": 0, "DUPLICATED": 1,
                                     "UNKNOWN": 2,
                                     "SOLE-INSTANCE": 3,
                                     "SOLE-INSTANCE-WW": 4}.get(f["verdict"], 5))
        out[sub] = {"files": files, "counts": counts, "findings": findings}
    return out
