#!/usr/bin/env python3
# ============================================================================
# census_axis_c.py — AXIS C, graph closure (census spec §2).
#
# MEASURE: for each subsystem, walk the outbound edges and classify each as
#   (a) internal        terminates inside the subsystem
#   (b) platform        OS/DVD/math/allocator primitives the receiver also has
#   (c) receiver-native a system the receiver already owns
#   (d) receiver-absent needs new receiver code
# and report THE EDGE LIST, not only the percentage (spec §2: "shape beats
# count" — 95% closure whose remainder reaches dStage is harder than 85% whose
# remainder is all platform).
#
# ---------------------------------------------------------------------------
# CLASSIFY BY WHERE A SYMBOL IS DECLARED, NEVER BY WHAT IT IS CALLED
#
# The obvious implementation is a prefix table: dComIfG* -> receiver-native,
# OS*/DVD* -> platform. That is a guess dressed as a measurement, and this
# session has already paid for that shape four times — law 2's co-occurrence
# link, law 3's TARGET_PC match, HT-17's unanchored substring, HT-18's embedded
# path. So every edge here is resolved to the FILE THAT DECLARES IT, and the
# class follows from that file's location. The declaring file travels with the
# edge as its receipt, so any row can be audited without re-running anything.
#
# The platform set is likewise DERIVED, not remembered: it is "declared under a
# platform include root", enumerated from the tree. Spec §9's completeness-of-
# validity-sets control exists because a remembered set (the CP registers
# missing 0xB0-0xBF) produces false positives that consume an anomaly budget.
#
# ---------------------------------------------------------------------------
# `unresolved` IS AN UPPER BOUND ON SPEC §2's CLASS (d), NOT EQUAL TO IT.
#
# It was called `receiver-absent` -- spec's "needs new receiver code" -- but the
# edges in it sample as `getline`, `sort`, `strncat`, `uintptr_t`: standard
# library symbols the index cannot resolve because the C++ stdlib is not in the
# tree. Those need no receiver code at all. The label asserted a conclusion the
# evidence did not support, and it inflated precisely the bucket that argues for
# PIECEWISE. Renamed to say what it measures; any verdict must treat it as a
# ceiling on the true API gap.
#
# HONEST LIMITS, ENFORCED RATHER THAN NOTED (spec §10)
#
# Static closure cannot see runtime/vtable dispatch, so it UNDERCOUNTS edges.
# Spec §10 says report UNKNOWN where dispatch is indirect, never internal. This
# module therefore emits an explicit `unresolved` class and counts it in the
# denominator: a subsystem whose edges mostly fail to resolve gets a LOW closure
# score and a loud unresolved count, never a flattering one. Closure is reported
# as a RANGE — best case (unresolved all internal) and worst case (unresolved
# all external) — because a single number here would be exactly the kind of
# false precision the posture doc was written about.
# ============================================================================
import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]

# PLATFORM ROOTS ARE DISCOVERED, NOT NAMED.
#
# v1 hardcoded include/dolphin, include/stdlib, libs/musyx/include. NONE of the
# three exist in this tree -- the real root is libs/dolphin/include/dolphin --
# so every platform edge was misfiled as receiver-native and the axis reported
# `platform=0` for all nine subsystems. That is the same
# validity-set-from-memory failure spec §9 exists to prevent, and the third time
# this session it has been mine. A set that can be enumerated from the tree must
# be enumerated from the tree.
#
# If discovery finds nothing, the platform class reports UNKNOWN rather than
# zero -- because "no platform edges" and "I could not look" are different
# claims and only one of them is true here.
def discover_platform_roots():
    roots = []
    for d in REPO.rglob("include/dolphin"):
        if d.is_dir() and "build" not in d.parts:
            roots.append(d.parent.relative_to(REPO).as_posix())
    for extra in ("libs/musyx/include", "libs/PowerPC_EABI_Support"):
        if (REPO / extra).is_dir():
            roots.append(extra)
    return sorted(set(roots))


PLATFORM_ROOTS = discover_platform_roots()
RECEIVER_ROOTS = ["include", "src", "libs"]
# v3: the discovered platform roots must also be INDEXED. extern/aurora/include
# was being used to CLASSIFY (it is a platform root) while never being READ (it
# is outside RECEIVER_ROOTS), so every symbol aurora declares -- MTXIdentity,
# VECNormalize, VECScale -- resolved nowhere and was filed receiver-absent. A
# root you classify against but never scan is a hole with a label on it.
INDEX_ROOTS = sorted(set(RECEIVER_ROOTS) | set(PLATFORM_ROOTS))

CALL = re.compile(r"\b([A-Za-z_]\w{2,})\s*\(")
# A call written `obj.foo(` / `obj->foo(` is a MEMBER call. Its target is a
# method on some object's type, which a free-symbol index cannot resolve --
# `c_str`, `create_directories`, `filename`, `warn`, `commit` are all this
# shape (std:: and the logger). Counting them as `receiver-absent` asserted
# they "need new receiver code", which is simply false, and it inflated the
# exact bucket that drives a PIECEWISE verdict. The call SITE tells us --
# that is syntactic evidence, not a name guess -- so they get their own
# class and are excluded from the closure denominator.
MEMBER_CALL = re.compile(r"(?:\.|->)\s*([A-Za-z_]\w{2,})\s*\(")
# A declaration in a header: return-type-ish tokens then name then '('.
DECL = re.compile(r"^[^/\n]*?\b([A-Za-z_]\w{2,})\s*\([^;{]*\)\s*(?:const)?\s*[;{]",
                  re.M)
TYPE_DECL = re.compile(r"^[ \t]*(?:class|struct)[ \t]+([A-Za-z_]\w{2,})"
                       r"[ \t]*(?:final)?[ \t]*(?:[:{]|$)", re.M)
# HT-23: typedefs and forward declarations ARE declarations. Without them
# `fpc_ProcID`, `request_of_phase_process_class` and -- critically --
# `GXTexObj` resolved nowhere, and an unresolvable type cannot be vetoed.
TYPEDEF_SIMPLE = re.compile(r"^[ \t]*typedef[ \t]+[^;{}]+?([A-Za-z_]\w*)[ \t]*;", re.M)
# `typedef struct _GXTexObj { ... } GXTexObj;` names the type on its CLOSING
# brace line -- a form no declaration pattern here had ever matched.
TYPEDEF_CLOSE = re.compile(r"^[ \t]*\}[ \t]*([A-Za-z_]\w*)[ \t]*;", re.M)
FWD_DECL = re.compile(r"^[ \t]*(?:class|struct)[ \t]+([A-Za-z_]\w*)[ \t]*;", re.M)
MACRO_OBJ = re.compile(r"^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w{2,})", re.M)
MACRO = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w{2,})\s*\(", re.M)
# A definition: a name followed by (...) then an opening brace on the same or
# next line, at column 0 or after a type -- i.e. the body lives here.
DEFN = re.compile('^[ \\t]*[A-Za-z_][\\w:*&<>,~\\s]*?([A-Za-z_]\\w{2,})\\s*\\([^;()]*\\)\\s*(?:const\\s*)?\\{', re.M)
# NOTE: this pattern is built without a word-boundary escape on purpose.
# The Bash heredoc that wrote v3 collapsed the escape into a literal 0x08
# BACKSPACE byte, so DEFN hunted for a character no source contains: it
# matched every synthetic test compiled in-process and ZERO real files
# through the module. Also indent-tolerant now -- v3 anchored at column 0,
# which misses every definition inside a namespace or class body.
# Indirect dispatch markers — these make a TU's edge set structurally incomplete.
STR_LIT = re.compile(r'"(?:[^"\\]|\\.)*"')
CHR_LIT = re.compile(r"'(?:[^'\\]|\\.)*'")
INDIRECT = re.compile(r"virtual\s|->\s*\w+\s*\(|\(\s*\*\s*\w+\s*\)\s*\(|"
                      r"process_method_func|actor_method_class")


def _iter_files(roots, exts):
    for r in roots:
        base = REPO / r
        if not base.is_dir():
            continue
        for p in base.rglob("*"):
            if p.suffix in exts and p.is_file():
                yield p



# Names declared at FREE (namespace/file) scope, per file. Populated alongside
# the main index; see patch note -- a class member cannot be an entry point.
FREE_DECLS = {}
# Names that are genuine class/struct declarations. Axis P needs this to
# tell a runtime-state OBJECT from a scalar: `bool` and `u16` are not
# singletons, and comparing globals by raw type token made every primitive
# look like duplicated hardware state.
TYPE_NAMES = set()


def _split_braces(txt):
    """One brace per line.

    HT-22: `namespace { X y; }` written on ONE line was invisible -- the
    scanner tests the whole line, which begins with `namespace`, so the
    declaration inside it never matched. Same class as the namespace defect
    fixed in Axis P v2, narrower form, and no control covered it.
    """
    return txt.replace("{", "{" + chr(10)).replace("}", chr(10) + "}")


def _free_scope_names(txt):
    """Names declared outside any class/struct body, by brace-depth walk.

    Deliberately simple and conservative: when it cannot tell, it does NOT claim
    the name is free. An over-claimed export invents ABI surface, which is the
    failure this exists to stop; an under-claimed one only widens the stated
    lower bound, which is already the axis's declared direction."""
    free = set()
    depth = 0
    type_depth = None
    for line in txt.splitlines():
        s = line.strip()
        if type_depth is None and re.match(r"^(?:template[^;]*>)?\s*(?:class|struct|union)\s+\w", s):
            if "{" in line:
                type_depth = depth
        if type_depth is None:
            m = DECL.search(line)
            if m:
                free.add(m.group(1))
            m = MACRO.match(line) or MACRO_OBJ.match(line)
            if m:
                free.add(m.group(1))
        depth += line.count("{") - line.count("}")
        if type_depth is not None and depth <= type_depth:
            type_depth = None
    return free


def build_decl_index():
    """identifier -> declaring file (repo-relative), from HEADERS across the tree.

    Headers only: a declaration is what an edge can resolve against. First
    declaration wins, and the index records the file so every classification
    carries its own receipt.
    """
    # ALL declaring files, not the first one walked.
    #
    # v2 used idx.setdefault(name, rel) -- first declaration wins. A symbol
    # declared in more than one header (a forward declaration in a shared
    # header plus the real one in its own) was therefore attributed to whichever
    # file rglob reached first. `evt1_isActive` IS declared in
    # include/d/ext_evt/evt1_boundary.h and the regex DOES match it; the symbol
    # had simply been claimed by an earlier file, so JEvent1 scored internal=0
    # while its own declarations sat in the index under someone else's name.
    #
    # The error has a DIRECTION: it moves edges out of `internal` and into
    # `receiver-native`, making every subsystem look less closed than it is --
    # which is the wrong way for an axis whose output is a wholesale/piecewise
    # verdict. Keeping every declaration site lets classification prefer an
    # internal one when the symbol is genuinely the subsystem's own.
    idx = {}
    defn_hits = 0
    for p in _iter_files(INDEX_ROOTS, {".h", ".hpp"}):
        try:
            txt = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        # Strip comments BEFORE indexing. Banner-heavy headers (the WW lane
        # writes long rationale blocks) were yielding zero declarations, which
        # silently inflated `receiver-absent` -- an error with a DIRECTION:
        # it makes subsystems look less closed than they are.
        txt = re.sub(r"/\*.*?\*/", "", txt, flags=re.S)
        txt = re.sub(r"//[^\n]*", "", txt)
        rel = p.relative_to(REPO).as_posix()
        FREE_DECLS[rel] = _free_scope_names(txt)
        for m in DECL.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
        # function-like macros are edges too, and they resolve to a real file
        for m in MACRO.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
        # OBJECT-like macros too. `#define AT_TYPE_BOMB (1 << 5)` is not a
        # function, but the `(` that follows its name makes the call extractor
        # see an edge -- and with only function-like macros indexed it resolved
        # nowhere and was counted as "needs new receiver code". The WW shim
        # constants in d_ext_ww_actor_shims.h are all exactly this shape.
        for m in MACRO_OBJ.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
        for m in TYPE_DECL.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
            TYPE_NAMES.add(m.group(1))
        for m in FWD_DECL.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
            TYPE_NAMES.add(m.group(1))
        for m in TYPEDEF_CLOSE.finditer(txt):
            # a braced typedef names an OBJECT type -> assessable by Axis P
            idx.setdefault(m.group(1), []).append(rel)
            TYPE_NAMES.add(m.group(1))
        for m in TYPEDEF_SIMPLE.finditer(txt):
            # resolvable, but NOT added to TYPE_NAMES: `typedef unsigned int
            # fpc_ProcID` is a scalar alias, not runtime state.
            idx.setdefault(m.group(1), []).append(rel)

    # v3: DEFINITIONS in .cpp, not only declarations in headers.
    #
    # A static helper defined in the TU itself (acquireBgModel,
    # activateWwHostRoom, advanceHandoff, allocBag ...) has no header
    # declaration, so it resolved NOWHERE and was counted receiver-absent --
    # i.e. as "needs new receiver code", when it is the subsystem's own code.
    # That is the single largest distortion in the axis and it inflates exactly
    # the bucket that drives a PIECEWISE verdict.
    for p in _iter_files(RECEIVER_ROOTS, {".cpp", ".c"}):
        try:
            txt = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        txt = re.sub(r"/\*.*?\*/", "", txt, flags=re.S)
        txt = re.sub(r"//[^\n]*", "", txt)
        rel = p.relative_to(REPO).as_posix()
        # §495 FAIL-2: free-scope names must be recorded for .cpp as well as
        # headers. Axis W gates exports on FREE_DECLS, so a free function
        # declared only in a .cpp failed the member test and disappeared --
        # silently, because a stem-matching header usually covered for it.
        FREE_DECLS[rel] = _free_scope_names(txt)
        for m in DEFN.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
            defn_hits += 1
        # v6: macros and TYPES also live in .cpp files, and both are edge
        # targets. Evidence, not guesswork:
        #   * `#define REG0_S(i) ...` is defined in d_a_kamome.cpp -- macros
        #     were indexed from headers only, so the whole REG0_S/REG6_S/REG17_S
        #     family resolved nowhere;
        #   * `class ExtVegPacket_c : public J3DPacket` is declared at
        #     d_a_ext_vegetation.cpp:857 -- a constructor call reads as
        #     `ExtVegPacket_c(...)`, and with no type index it counted as
        #     "needs new receiver code".
        # Both were being charged against the subsystem that OWNS them.
        for m in MACRO.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
        for m in MACRO_OBJ.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
        for m in TYPE_DECL.finditer(txt):
            idx.setdefault(m.group(1), []).append(rel)
            TYPE_NAMES.add(m.group(1))

    # SELF-CHECK -- an index that resolves NOTHING from .cpp definitions is
    # broken, not empty. v3 shipped exactly that for a full pass: a mangled
    # escape turned the definition pattern into a hunt for a 0x08 byte, so every
    # local static resolved nowhere and was reported as "needs new receiver
    # code". The axis looked plausible the whole time. A cheap invariant catches
    # it; reading the regex did not, three times.
    # Count what the guard CLAIMS to measure. The previous version counted
    # symbols whose declaring file ends in .cpp, which the macro/type passes
    # keep non-zero even when definition matching is entirely broken -- so it
    # never fired, and a dead guard reads as coverage. Falsified by
    # census_selftest.py, which is why that file exists.
    if defn_hits == 0:
        raise RuntimeError(
            "AXIS C INDEX BROKEN: DEFN matched 0 definitions across the "
            "whole tree. "
            "Every subsystem-local helper would be misreported as "
            "receiver-absent. Refusing to return a usable-looking index.")
    return idx



def _callable_text(txt):
    """Text with comments, STRING/CHAR LITERALS and preprocessor lines removed.

    Log messages were manufacturing edges. `"state-selected row not chosen (not
    a fault)"` yielded an edge named `chosen`; `"...exist anywhere (real
    absence)"` yielded `anywhere`. This lane writes long explanatory log
    strings, so the noise concentrated in precisely the TUs under measurement
    and inflated their `receiver-absent` count -- the bucket that drives a
    PIECEWISE verdict. Prose inside quotes is not a call graph.

    Preprocessor lines go too: `#define AT_TYPE_BOMB (1 << 5)` is a definition,
    not a call site, so counting it as an outbound edge charged a subsystem for
    its own constants.
    """
    txt = re.sub(r"/\*.*?\*/", "", txt, flags=re.S)
    txt = re.sub(r"//[^\n]*", "", txt)
    txt = STR_LIT.sub('""', txt)
    txt = CHR_LIT.sub("''", txt)
    txt = re.sub(r"^[ \t]*#[^\n]*", "", txt, flags=re.M)
    return txt


def classify_edge(name, decl_files, subsystem_files):
    """Location -> class, over EVERY declaration site.

    Precedence: internal > platform > receiver-native. A symbol the subsystem
    declares itself is internal even if a shared header forward-declares it too;
    that is what "terminates inside the subsystem" means."""
    if not decl_files:
        return "unresolved", None
    if not PLATFORM_ROOTS:
        return "unresolved-platform-roots", decl_files[0]
    for d in decl_files:
        if d in subsystem_files:
            return "internal", d
    for d in decl_files:
        for pr in PLATFORM_ROOTS:
            if d.startswith(pr):
                return "platform", d
    return "receiver-native", decl_files[0]


# ---------------------------------------------------------------------------
# Subsystem grouping. DECLARED, and every rule is inspectable; anything that
# matches no rule lands in UNGROUPED rather than being forced into a bucket —
# a mis-grouped TU corrupts a wholesale/piecewise verdict, which is the whole
# output of this axis.
# ---------------------------------------------------------------------------
GROUPS = [
    (re.compile(r"/ja1_|/ext_seq/"), "JAudio1 / sequence lane"),
    (re.compile(r"/evt1_"), "JEvent1"),
    (re.compile(r"/ww_jpa"), "JPA (WW particle)"),
    (re.compile(r"/ext_line/"), "MDoExt1 (3D line)"),
    (re.compile(r"/d_a_ext_|/d_a_ww_"), "WW actors (ext)"),
    (re.compile(r"/d_ext_"), "WW host systems (d_ext_)"),
    (re.compile(r"_ww(_|\.)"), "WW engine legs (_ww)"),
    (re.compile(r"/libs/JSystem/"), "JSystem (receiver-owned, donor legs)"),
    # The 26 that matched nothing split cleanly along posture §1.1's own two
    # classes -- 16 whole-file donor ports wearing TP actor names, 10 receiver
    # engine TUs carrying donor legs. The split below is BY PATH, which is a
    # heuristic, not lineage: an actor could in principle be a leg-carrier.
    # Nothing checkable in the tree separates the two classes today except the
    # KIT-LINEAGE tag, and only 21% of the roster carries one -- so this
    # grouping is provisional and step 10 is what makes it evidence. Recorded
    # here rather than in a report, because a heuristic that travels without its
    # caveat becomes a fact by repetition (§1.1's own diagnosis).
]

# E3 (step 10 trigger FIRED): the two rules that used to live at the end of
# GROUPS split those 26 TUs BY PATH. The caveat above said this grouping becomes
# evidence when lineage tags reach the roster -- E2 took coverage 21% -> 100%, so
# the tag is now the partition and the path heuristic is only the fallback for a
# TU that carries no tag. L8's bias ("partition may be WRONG, not merely
# imprecise") is what this retires.
#
# The mapping is the tag vocabulary's own meaning, not a new judgement:
#   native-port                     -> a whole-file donor port  (V6)
#   mixed | host-plumbing | bridge-owed -> a receiver TU carrying legs (V8)
# `mixed` is the class E2 created for exactly this: TP's own file with donor
# hunks inside it. Under the path heuristic every `mixed` TU in src/d/actor/
# scored as a whole-file donor port, which is precisely the mis-partition L8
# warned about.
V6_NAME = "WW direct-port actors (TP-named)"
V8_NAME = "receiver engine TUs w/ donor legs"
LINEAGE_TO_GROUP = {
    "native-port": V6_NAME,
    "mixed": V8_NAME,
    "host-plumbing": V8_NAME,
    "bridge-owed": V8_NAME,
}
PROVISIONAL_GROUPS = [
    (re.compile(r"/src/d/actor/"), V6_NAME + " [provisional: no tag]"),
    (re.compile(r"/src/(d|f_op|f_pc)/"), V8_NAME + " [provisional: no tag]"),
]

# BOM-tolerant since 2026-08-16: `^//` cannot match a first line behind a
# UTF-8 BOM, which hid d_stage.cpp's header from this axis. Capture semantics
# unchanged (whole value as one token) - only the anchor is widened.
_RE_KIT_LINEAGE = re.compile("^\ufeff?//" r"\s*KIT-LINEAGE:\s*(\S+)", re.M)


def lineage_of(rel):
    """The TU's DECLARED lineage class, or None. Never inferred from the path."""
    p = REPO / rel
    if not p.is_file():
        return None
    m = _RE_KIT_LINEAGE.search(p.read_text(encoding="utf-8-sig", errors="replace"))
    return m.group(1).split(":")[0] if m else None


def group_of(rel):
    for rx, name in GROUPS:
        if rx.search("/" + rel):
            return name
    g = LINEAGE_TO_GROUP.get(lineage_of(rel))
    if g:
        return g
    # No tag: fall back to the old heuristic, still carrying its caveat, so an
    # untagged TU is visibly provisional rather than silently mixed in with
    # tag-backed ones.
    for rx, name in PROVISIONAL_GROUPS:
        if rx.search("/" + rel):
            return name
    return "UNGROUPED"


def run(roster):
    """roster: list of repo-relative TU paths. Returns per-subsystem Axis C."""
    idx = build_decl_index()
    subs = {}
    for rel in roster:
        subs.setdefault(group_of(rel), []).append(rel)

    out = {}
    for sub, files in sorted(subs.items()):
        fileset = set(files)
        # a subsystem's own headers count as internal too
        # A subsystem's own headers are internal. v1 matched by STEM only, which
        # left JEvent1 and MDoExt1 at internal=0 -- their headers live in a
        # parallel include/ tree (src/d/ext_evt/evt1_boundary.cpp ->
        # include/d/ext_evt/evt1_boundary.h), so a stem match alone missed the
        # sibling declarations that make a subsystem cohere. Directory-parallel
        # headers count too.
        # v7: a TU's own header is its PARALLEL PATH, constructed per file --
        # never its whole directory.
        #
        # v6 matched on parent directory, so a subsystem of 4 files living in
        # src/d/actor/ claimed every symbol declared in all ~500 actor TUs as
        # internal. That inflated closure for exactly the groups whose PIECEWISE
        # verdict is the output (direct-port actors, WW actors ext), and it made
        # Axis W report 13,195 exports for a 4-file subsystem -- which is how it
        # was caught. Directory membership is not subsystem membership.
        own_headers = set()
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
            own_headers |= cands
        all_decl_files = {d for v in idx.values() for d in v}
        own_headers &= all_decl_files
        internal_scope = fileset | own_headers

        edges, indirect_tus = {}, 0
        # HT-20: names this subsystem DEFINES anywhere in its own TUs. A defined
        # name can never be "needs new receiver code".
        sub_defined = {}
        for f in files:
            fp = REPO / f
            if not fp.is_file():
                continue
            try:
                dtxt = _callable_text(fp.read_text(encoding="utf-8", errors="replace"))
            except OSError:
                continue
            for m in DEFN.finditer(dtxt):
                sub_defined.setdefault(m.group(1), f)
        for f in files:
            p = REPO / f
            if not p.is_file():
                continue
            txt = _callable_text(p.read_text(encoding="utf-8", errors="replace"))
            if INDIRECT.search(txt):
                indirect_tus += 1
            members = {m.group(1) for m in MEMBER_CALL.finditer(txt)}
            # §495 FAIL-1: a DEFINITION is not a call. `void f(void) {}` matches
            # the call pattern, and an own definition always resolves internal,
            # so counting it could only ever raise closure. Skip the definition
            # OCCURRENCE (by span), never the name -- genuine internal calls to
            # the same function are real edges.
            def_spans = {m.span(1) for m in DEFN.finditer(txt)}
            for m in CALL.finditer(txt):
                if m.span(1) in def_spans:
                    continue
                n = m.group(1)
                if n in members and n not in idx:
                    # member call with no free-symbol declaration anywhere:
                    # record it, but never as evidence of an absent receiver API
                    edges.setdefault(n, ("member-call", None))
                    continue
                if n in ("if", "for", "while", "switch", "return", "sizeof",
                         "static_cast", "reinterpret_cast", "const_cast",
                         "dynamic_cast", "catch", "printf", "snprintf"):
                    continue
                if n in edges:
                    continue
                cls, where = classify_edge(n, idx.get(n, []), internal_scope)
                if cls == "unresolved" and n in sub_defined:
                    # Defined by this subsystem -> internal, with the defining
                    # file as its receipt (HT-20).
                    cls, where = "internal", sub_defined[n]
                edges[n] = (cls, where)

        counts = {}
        for cls, _ in edges.values():
            counts[cls] = counts.get(cls, 0) + 1
        # member-call edges are excluded from the closure denominator: they are
        # neither internal nor a receiver API gap, so counting them either way
        # would move a verdict on a non-fact.
        total = max(sum(v for k, v in counts.items() if k != "member-call"), 1)
        internal = counts.get("internal", 0)
        absent = counts.get("unresolved", 0)

        # Closure as a RANGE, never a single number: `receiver-absent` here means
        # "no declaration found in the tree", which is genuinely ambiguous — it
        # can be an unresolved macro, a local static, or a real gap. Reporting a
        # point estimate would launder that ambiguity into false precision.
        best = 100.0 * (internal + absent) / total
        worst = 100.0 * internal / total
        out[sub] = {
            "files": files, "edge_count": total, "counts": counts,
            "closure_best_pct": round(best, 1),
            "closure_worst_pct": round(worst, 1),
            "indirect_dispatch_tus": indirect_tus,
            "edges": {k: {"class": v[0], "declared_in": v[1]}
                      for k, v in sorted(edges.items())},
        }
    return out
