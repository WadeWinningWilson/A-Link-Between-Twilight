#!/usr/bin/env python3
# ============================================================================
# ww_census.py — the WW SUBSYSTEM CENSUS (posture roadmap Stage B, step 6).
#
# Spec: docs/WW Linked/ww-subsystem-census-spec.md   Posture: ww-code-lineage-posture.md
#
# ---------------------------------------------------------------------------
# WHAT THIS RUN CLAIMS, AND WHAT IT DOES NOT
#
# The spec defines five axes. This landing implements TWO of them and reports
# the other three as UNKNOWN **with a stated reason**, per №31-C: a check that
# cannot run must never report clean. Claiming five axes while three are stubs
# would reproduce, inside the census, precisely the error the census exists to
# correct — a true statement about one thing being read as a statement about
# another (posture §1, §1.1).
#
#   IMPLEMENTED  Roster (spec §1)   — generated from files.cmake, never curated
#   IMPLEMENTED  Axis B (spec §6)   — donor-data payload, the membrane check
#   UNKNOWN      Axis C (spec §2)   — graph closure
#   UNKNOWN      Axis D (spec §3)   — decomp completeness
#   UNKNOWN      Axis W (spec §4)   — interface width (this is the plugin ABI)
#   UNKNOWN      Axis P (spec §5)   — platform-layer depth (the hard veto)
#
# No WHOLESALE/PIECEWISE verdict is emitted by this landing. A verdict needs C,
# D, W and P; emitting one from the two axes that exist would be a guess with a
# table around it.
#
# ---------------------------------------------------------------------------
# THE ROSTER AXIS IS DELIBERATELY THREE COUNTS, NOT ONE
#
# Posture §1.1 records that "46 WW-owned TUs" was a true statement about
# FILENAMES read as a statement about LINEAGE, and sets a standing rule: cite it
# as "N TUs NAMED by WW convention — a floor". A single number invites exactly
# that misreading again, so this tool structurally cannot emit one. It reports:
#
#   named       matched the WW filename convention  (a FLOOR — see §1.1)
#   declared    carries a machine-readable // KIT-LINEAGE tag (§426/§427)
#   evidenced   carries in-file donor-authority markers (DIRECT PORT, WW DP, …)
#
# and the union, with the overlaps shown. The convention is structurally blind
# to the most donor-faithful work in the tree — a direct port of WW's
# d_a_lamp.cpp is *correctly* named d_a_lamp.cpp — so `named` gets blinder as
# the porting gets better. `declared` is the axis that fixes this permanently,
# which is why the KIT-LINEAGE tag is reported separately and its coverage is a
# first-class output: coverage rising is the roster becoming trustworthy.
#
# ---------------------------------------------------------------------------
# NEGATIVE CONTROLS THIS LANDING SUPPORTS (spec §9, HousingTemp runs them)
#   * Roster planted-TU:  add a WW-named TU to files.cmake, touch no list; it
#                         must appear on the next run.  --explain <path> shows
#                         exactly why any TU is or is not on the roster.
#   * Falsifiability:     --selftest-b proves Axis B can report B = 0 on a TU
#                         with no donor arrays. A scanner that cannot produce a
#                         zero is not an instrument.
#   * UNKNOWN ≠ CLEAN:    unimplemented axes emit UNKNOWN, never a pass.
#
# Read-only. Emits JSONL + a markdown table (spec §8) so runs diff.
# Usage:
#   ww_census.py                       full run -> stdout summary + artifacts
#   ww_census.py --explain <tu-path>   why this TU is / is not on the roster
#   ww_census.py --selftest-b          Axis B falsifiability control
# ============================================================================
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
FILES_CMAKE = REPO / "files.cmake"
OUT_DIR = REPO / "docs" / "state"

# ============================================================================
# HT-15 (§492) — ONE SHARED CLASSIFIER, not a second implementation.
#
# The census used to parse files.cmake and apply its OWN convention rules. That
# produced two instruments, one lane, same input, same hour, disagreeing:
# census NAMED=46 vs manifest ww_layer_count=47, denominators 1521 vs 1523.
# Spec §1 exists to prevent exactly this ("five tools would give five rosters
# that drift apart") and it happened between the generator and the census on
# day one.
#
# It was not cosmetic. My parser required the LINE to end in `.cpp`, so it
# silently dropped `src/d/d_door.cpp # §328 WW door base layer …` — a
# WW-relevant TU — purely because of a trailing comment. Housing's parser
# strips cmake comments and keeps it. A roster that drops WW files is not a
# roster.
#
# So the census no longer classifies. It CONSUMES Housing's generator
# (`ww_layer_manifest.py`, §488), which is the canonical, drift-gated
# implementation, and layers its own two extra signals (declared / evidenced) on
# top. One parser, one convention rule-set, one denominator — by construction
# rather than by agreement.
# ============================================================================
sys.path.insert(0, str(REPO / "tools" / "ww_crew_restoration_skeleton"))
try:
    import ww_layer_manifest as WWLM
except Exception as _e:  # pragma: no cover
    WWLM = None
    _WWLM_ERR = _e

RE_LINEAGE = re.compile(r"^//\s*KIT-LINEAGE:\s*"
                        r"(native-port|bridge-owed|host-plumbing)"
                        r"(?::(\S+))?\s*$", re.M)

# In-file donor-authority markers. Enumerated from the strings posture §1.1
# names, NOT from memory of what banners "usually" say — the spec's own
# completeness-of-validity-sets control (§9) applies to this list too.
DONOR_MARKERS = [
    (re.compile(r"DIRECT PORT", re.I), "DIRECT PORT"),
    (re.compile(r"VERBATIM from the WW donor", re.I), "verbatim-from-donor"),
    (re.compile(r"\bWW DP\b"), "WW DP (donor source path)"),
    (re.compile(r"donor[- ]verbatim", re.I), "donor-verbatim"),
    (re.compile(r"zeldaret[/\\]tww"), "zeldaret/tww reference"),
    (re.compile(r"\bd_a_\w+\.cpp:\d+"), "donor decomp file:line citation"),
    # E2 ADD: the form the shared receiver TUs actually use to declare a
    # donor-derived span -- "donor d/d_event_manager.cpp:1204". The rule above
    # only ever matched d_a_* actors, so d_event_manager.cpp and d_particle.cpp
    # declared donor sites that NO marker looked for. Verified the same way.
    (re.compile(r"\bdonor\s+[\w/]*\w+\.cpp", re.I), "donor path citation"),
]

_RE_CITE = re.compile(r"\b(d_a_\w+\.cpp):\d+")
_RE_DONOR_PATH = re.compile(r"\bdonor\s+[\w/]*?(\w+\.cpp)", re.I)
_DONOR_BASENAMES = None


def _cites_a_donor_object(raw):
    """True only if some cited d_a_*.cpp is an object in the donor's build.

    Lazy + cached: census_axis_d reads the donor's configure.py off a slow path,
    and this runs once per TU. Returns True (fail-open) when the donor build is
    unreadable, so a missing donor tree can never SILENTLY shrink the roster --
    losing a real WW TU is the costlier error of the two.
    """
    global _DONOR_BASENAMES
    if _DONOR_BASENAMES is None:
        try:
            import census_axis_d as _D
            sm = _D.donor_status()
            if not sm:
                return True
            import posixpath
            _DONOR_BASENAMES = {posixpath.basename(k) for k in sm}
        except Exception:
            return True
    return any(m.group(1) in _DONOR_BASENAMES for m in _RE_CITE.finditer(raw))


def _cites_a_donor_path(raw):
    """True if a `donor <path>.cpp` citation names a real donor object."""
    if _DONOR_BASENAMES is None:
        _cites_a_donor_object("")          # prime the cache
    if not _DONOR_BASENAMES:
        return True
    return any(m.group(1) in _DONOR_BASENAMES
               for m in _RE_DONOR_PATH.finditer(raw))


# ---------------------------------------------------------------------------
# Axis B — donor-data payload (spec §6, the membrane check)
#
# WHY BYTES AND NOT A BOOLEAN: the user's 2026-08-05 ruling ships a defined
# trip-wire — "donor DL/data bytes stop being countable on one hand" — so the
# instrument has to produce a NUMBER the ruling can be evaluated against, per
# class, with the evidence attached. A yes/no cannot be compared to that.
#
# Classification follows the spec's own four buckets. Anything matched that does
# not fit a bucket is reported as `unclassified` rather than forced into one:
# the tally is evidence for a user ruling, and a mis-bucketed array is worse
# than an honest "this needs a human look".
# ---------------------------------------------------------------------------
# THE PATTERN IS DERIVED FROM THE TREE, NOT FROM MEMORY.
#
# The first version of this regex missed `l_toonMat1DL` — the single donor array
# the user has already ruled on, and the very array the §5.1 trip-wire is
# defined against. Two reasons, both the same mistake:
#   * its extent is HEX (`[0xA5]`), and the pattern only accepted decimal;
#   * an alignment attribute sits between `]` and `=`
#     (`static u8 l_toonMat1DL[0xA5] ATTRIBUTE_ALIGN(32) = {`).
#
# Spec §9 names this failure class explicitly — "completeness of validity sets:
# enumerated against the donor tree, not from memory" — citing the CP-register
# set that omitted 0xB0-0xBF. I committed it anyway, in the axis whose number
# goes in front of a user ruling. A positive control is now part of --selftest-b
# so it cannot regress: an Axis B that cannot see the one array we KNOW is donor
# data has no business reporting a tally.
ARRAY_DECL = re.compile(
    r"(?:static\s+)?(?:const\s+)?"
    r"(u8|s8|u16|s16|u32|s32|f32|char|unsigned\s+char|uint8_t|int8_t)\s+"
    r"(\w+)\s*"
    r"((?:\[[^\]]*\])+)"                       # EVERY extent, not just the first
    r"(?:\s*\w+\s*\([^)]*\))*"                 # ATTRIBUTE_ALIGN(32) etc.
    r"\s*=\s*\{",
    re.M)

# HT-24: the same declaration shape WITHOUT an initialiser. Counted only to
# state the scanned population's basis -- never summed, since an uninitialised
# array has no payload by definition.
NOINIT_DECL = re.compile(
    r"(?:static\s+)?(?:const\s+)?"
    r"(?:u8|s8|u16|s16|u32|s32|f32|char|unsigned\s+char|uint8_t|int8_t)\s+"
    r"\w+\s*(?:\[[^\]]*\])+"
    r"(?:\s*\w+\s*\([^)]*\))*\s*;",
    re.M)

WIDTH = {"u8": 1, "s8": 1, "char": 1, "unsigned char": 1, "uint8_t": 1,
         "int8_t": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4, "f32": 4}


def _init_block(raw, i):
    """Text inside the initialiser braces. `i` indexes just past the opening
    `{`. Comment- and string-aware so a brace inside either cannot unbalance
    the scan."""
    depth, n = 1, len(raw)
    start = i
    while i < n and depth:
        c = raw[i]
        if c == "/" and i + 1 < n and raw[i + 1] == "/":
            j = raw.find(chr(10), i)
            i = n if j < 0 else j
            continue
        if c == "/" and i + 1 < n and raw[i + 1] == "*":
            j = raw.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if c in "\"'":
            q, i = c, i + 1
            while i < n and raw[i] != q:
                i += 2 if raw[i] == chr(92) else 1
            i += 1
            continue
        depth += (c == "{") - (c == "}")
        i += 1
    return raw[start:i - 1] if depth == 0 else None


# Operators that CANNOT appear in a constant leaf. `(` is deliberately absent:
# `dPa_RM(ID_ZI_S_DASHSAND_A)` is a macro over a constant, not a computation.
COMPUTED = (">>", "<<", "->", "++", "?")


def _blank_comments(raw):
    """Comments blanked to spaces, newlines and every offset preserved.

    A commented-out declaration is not an array. `d_a_obj_mshokki.cpp:152` keeps
    the donor's `bdl_idx[]` in a comment to document what the port replaced, and
    it was being tallied as live data."""
    out = list(raw)
    i, n = 0, len(raw)
    while i < n:
        c = raw[i]
        if c == "/" and i + 1 < n and raw[i + 1] == "/":
            while i < n and raw[i] != chr(10):
                out[i] = " "
                i += 1
        elif c == "/" and i + 1 < n and raw[i + 1] == "*":
            j = raw.find("*/", i + 2)
            j = n if j < 0 else j + 2
            while i < j:
                if raw[i] != chr(10):
                    out[i] = " "
                i += 1
        elif c in "\"'":
            q = c
            i += 1
            while i < n and raw[i] != q:
                i += 2 if raw[i] == chr(92) else 1
            i += 1
        else:
            i += 1
    return "".join(out)


def _leaf_list(block):
    """The initialiser's scalar elements, as written. Every leaf is
    comma-separated from its neighbour no matter how deep the nesting, so
    splitting on commas and discarding brace/whitespace-only fragments counts
    leaves at any dimensionality -- `{{a,b},{c,d}}` yields four."""
    s = re.sub(r"/\*.*?\*/", " ", block, flags=re.S)
    s = re.sub(r"//[^" + chr(10) + r"]*", " ", s)
    out = []
    for tok in s.split(","):
        tok = tok.strip(" " + chr(9) + chr(13) + chr(10) + "{}")
        if tok:
            out.append(tok)
    return out


def _leaves(block):
    """Number of scalar elements written down. Every leaf is comma-separated
    from its neighbour no matter how deep the nesting, so splitting on commas
    and discarding brace/whitespace-only fragments counts leaves at any
    dimensionality -- `{{a,b},{c,d}}` yields four."""
    s = re.sub(r"/\*.*?\*/", " ", block, flags=re.S)
    s = re.sub(r"//[^" + chr(10) + r"]*", " ", s)
    return sum(1 for tok in s.split(",") if tok.strip(" " + chr(9) + chr(13)
                                                      + chr(10) + "{}"))


B_CLASS = [
    (re.compile(r"DL$|DisplayList|_dl\b", re.I), "display-list"),
    (re.compile(r"Mat\d|TevColor|GXColor|_reg|register", re.I), "gx-register-state"),
    (re.compile(r"TEX|Txo_|tex_|palette|_pal\b", re.I), "asset-like"),
    (re.compile(r"table|tbl|lut|_map\b|offset|angle|sin|cos", re.I), "lookup-table"),
]


def load_build_sources():
    """Roster oracle = what the build compiles, via the SHARED parser (HT-15).

    Delegated to ww_layer_manifest.build_sources() so the census and the
    manifest can no longer disagree about the denominator. If that module is
    unavailable the census reports UNKNOWN rather than falling back to a private
    parser — a second parser is the defect, so a fallback would reintroduce it.
    """
    if WWLM is None:
        return None
    return WWLM.build_sources()


def roster_resolves():
    """HT-18 guard, run before any number is printed.

    A path that cannot be opened cannot be scanned, so every axis downstream of
    it is silently zero rather than honestly UNKNOWN -- which is exactly how the
    JSystem population disappeared without any count moving."""
    if WWLM is None or not hasattr(WWLM, "unresolved_sources"):
        return None
    return WWLM.unresolved_sources()


def classify_tu(rel):
    """Three independent axes of evidence for one TU. Never merged."""
    # (helper defined below classify_tu's callers; see _cites_a_donor_object)
    # HT-15: the NAMED signal is Housing's classify(), not a local rule-set.
    named = WWLM.classify(rel) if WWLM is not None else None
    named_strength = "shared" if named else None

    p = REPO / rel
    declared = declared_ref = None
    evidence = []
    if p.is_file():
        raw = p.read_text(encoding="utf-8", errors="replace")
        m = RE_LINEAGE.search(raw)
        if m:
            declared, declared_ref = m.group(1), m.group(2)
        for rx, label in DONOR_MARKERS:
            m2 = rx.search(raw)
            if not m2:
                continue
            # E2 FIX: a `d_a_*.cpp:NNN` citation is only DONOR evidence if the
            # cited file is an object in the donor's own build. The receiver has
            # actor files too, and citing one says nothing about lineage --
            # d_gameover.cpp reached the roster solely by citing
            # `d_a_alink.cpp:13852`, which is TP's alink and is not a donor
            # object at all. Verified against configure.py, not assumed.
            if label == "donor decomp file:line citation" \
                    and not _cites_a_donor_object(raw):
                continue
            if label == "donor path citation" and not _cites_a_donor_path(raw):
                continue
            evidence.append(label)
    # A weak name with no corroboration is NOT roster evidence — it is a
    # coincidence until something in the file says otherwise.
    named_weak = None
    return {"path": rel, "named": named, "named_weak": named_weak,
            "strength": named_strength, "declared": declared,
            "declared_ref": declared_ref, "evidence": evidence}


def load_rulings():
    """User rulings the census must consume (docs/WW Linked/ww-rulings.tsv).

    §546: the §516 NO VETO ruling lived only in prose, so the census kept
    emitting VETO-PENDING for six sections while the derivation doc recorded B2
    as CLOSED. A ruling the instrument cannot see is a ruling the instrument
    disagrees with.

    Returns [] if the file is absent -- NOT a silent 'no rulings apply', because
    the caller prints which rulings were applied, so an empty list is visible in
    the output rather than assumed."""
    f = REPO / "docs/WW Linked/ww-rulings.tsv"
    out = []
    if not f.is_file():
        return out
    for line in f.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        parts = line.split("	")
        if len(parts) >= 3 and parts[0] != "kind":
            out.append({"kind": parts[0].strip(), "scope": parts[1].strip(),
                        "section": parts[2].strip(),
                        "note": parts[3].strip() if len(parts) > 3 else ""})
    return out


def on_roster(r):
    """THE roster predicate. One definition, called by every tool (HT-27).

    A TU is on the WW-layer roster if Housing's shared classifier NAMES it, or
    it DECLARES any lineage, or it carries donor EVIDENCE.

    `declared` is tested for truth, not against `"native-port"`. The old
    value-specific test would have silently unlinted a TU bannered
    `host-plumbing` or `bridge-owed` that was neither named nor evidenced --
    declared provenance, never checked. It was latent, not violated (both
    predicates select the same 72 TUs today, symmetric difference empty), and
    this campaign's record is that absent guards become violated ones later.

    It lives here, once, because the same predicate copy-pasted across
    ww_census/banner_lint/leg_debt is five things that can drift -- the defect
    HT-15 removed when the census and the manifest disagreed about the
    denominator."""
    return bool(r["named"] or r["declared"] or r["evidence"])


def roster_union(rows):
    """Sorted roster paths from classified rows. Use this, never a local set
    comprehension."""
    return sorted({r["path"] for r in rows if on_roster(r)})


def axis_b(rel):
    """Donor-data payload tally for one TU. Returns rows + totals, or UNKNOWN.

    Rows carry `payload=False` for scratch buffers rather than being dropped, so
    the exclusion is auditable instead of invisible."""
    p = REPO / rel
    if not p.is_file():
        return None, "file not present"
    raw = _blank_comments(p.read_text(encoding="utf-8", errors="replace"))
    rows = []
    for m in ARRAY_DECL.finditer(raw):
        ctype, name, dims = m.group(1), m.group(2), m.group(3)
        width = WIDTH.get(" ".join(ctype.split()), 1)

        block = _init_block(raw, m.end())
        leaf_list = _leaf_list(block) if block is not None else None
        leaves = len(leaf_list) if leaf_list is not None else None

        # §507 OVER-COUNT FIX, decided at the DECLARATION -- the only thing
        # §507 permits, after indentation and `const` were both falsified as
        # splits. Three states, because two of them are not the same claim:
        #   False    -- no payload declared (`= {}`, or the `= {0}` zero-fill
        #               idiom standing in for a whole extent)
        #   COMPUTED -- leaves are expressions over runtime values, so there is
        #               no data here to be from anywhere; UNKNOWN, not zero
        #   True     -- elements were written down
        payload = True
        if leaves == 0:
            payload = False
        elif leaf_list:
            n_first = None
            first = re.match(r"\[\s*(0[xX][0-9a-fA-F]+|\d+)\s*\]", dims)
            if first:
                n_first = int(first.group(1), 16) \
                    if first.group(1).lower().startswith("0x") \
                    else int(first.group(1))
            if leaves == 1 and n_first and n_first > 1 \
                    and re.fullmatch(r"0|0x0+|0\.0f?", leaf_list[0]):
                payload = False          # zero-fill idiom
            elif any(op in lf for lf in leaf_list for op in COMPUTED):
                payload = "computed"

        # Extents: EVERY dimension, not just the first. Reading only the first
        # scored `a_att_dis_TBL[11][2]` as 11 bytes instead of 22.
        extents, symbolic = [], False
        for e in re.findall(r"\[([^\]]*)\]", dims):
            e = e.strip()
            if not e:
                extents.append(None)
                continue
            try:
                extents.append(int(e, 16) if e.lower().startswith("0x")
                               else int(e))
            except ValueError:
                extents.append(None)
                symbolic = True

        if extents and all(x is not None for x in extents):
            n = 1
            for x in extents:
                n *= x
            nbytes, basis = n * width, "declared-extent"
        elif leaves:
            # §507 UNDER-COUNT FIX. Unsized `[]` is the NORMAL shape of a ported
            # donor table -- `a_anm_prm_tbl$4490/4497/4553` scored 0 and hold
            # 640 bytes. The extent is recoverable EXACTLY from the initialiser,
            # so reporting 0 was a wrong number, not caution.
            nbytes, basis = leaves * width, "initialiser-leaves"
        else:
            nbytes, basis = None, "unknown"

        klass = "unclassified"
        for rx, k in B_CLASS:
            if rx.search(name):
                klass = k
                break
        line = raw[:m.start()].count(chr(10)) + 1
        if payload == "computed":
            nbytes, basis = None, "computed-initialiser"
        rows.append({"name": name, "line": line, "class": klass,
                     "bytes": nbytes, "payload": payload, "basis": basis,
                     "leaves": leaves, "ctype": ctype, "width": width,
                     "symbolic_extent": symbolic})
    return rows, None


def _gate_width(w_, c):
    """ENTRY POINTS ONLY — the surface a receiver enters the subsystem through.

    This is what spec §4 calls the interface and what wholesale-vs-piecewise
    turns on. Callbacks are the subsystem's outbound dependencies, not its gate,
    and summing them produced the misleading "JPA 15-18 vs JAudio1 115-261"
    comparison (§502 Finding A).

    Returns UNKNOWN rather than a number when there are no visible entry points
    AND the subsystem shows indirect dispatch: "no gate" and "a gate this axis
    cannot see" are different claims, and only the second one is true here."""
    n = len(w_.get("entry_points", {}))
    amb = len(w_.get("ambiguous_called_externally", {}))
    if n == 0 and c.get("indirect_dispatch_tus", 0) > 0:
        return {"value": None, "state": "UNKNOWN",
                "why": "no entry point resolves, and this subsystem uses "
                       "indirect dispatch (L1) -- the gate is unmeasured, "
                       "not narrow"}
    return {"value": [n, n + amb], "state": "MEASURED",
            "why": f"{n} unambiguous entry point(s)"
                   + (f", +{amb} ambiguous-but-called" if amb else "")}


def run_full(srcs, rows, union):
    RULINGS = load_rulings()
    """STEP 8 — all five axes, one verdict per subsystem (spec §7)."""
    import census_axis_c as AC
    import census_axis_d as AD
    import census_axis_p as AP
    import census_axis_w as AW

    cres = AC.run(union)
    wres = AW.run(union, srcs, cres)
    pres = AP.run(union, srcs)
    dres = AD.run(union)

    out = {}
    for sub in sorted(cres):
        c, w_, p = cres[sub], wres.get(sub, {}), pres.get(sub, {})
        dd = dres.get(sub, {}) if isinstance(dres, dict) else {}
        counts = c["counts"]
        resolved = sum(v for k, v in counts.items()
                       if k not in ("member-call", "unresolved"))
        recv = counts.get("receiver-native", 0)
        recv_share = (100.0 * recv / resolved) if resolved else 0.0
        veto = p.get("counts", {}).get("VETO-CANDIDATE", 0)

        # A ruling CHANGES THE VERDICT and never deletes the finding: the
        # candidate count stays in `why` and Axis P still reports it. The user
        # overruled the disposition, not the measurement (§546).
        _r = next((r for r in RULINGS if r["kind"] == "NO-VETO"
                   and r["scope"] in ("*", sub)), None)
        if veto and _r:
            verdict = "PIECEWISE"
            why = (f"Axis P surfaced {veto} platform-declared singleton "
                   f"candidate(s) — VETO RULED NO by USER §{_r['section']}, so "
                   f"not disqualifying; verdict falls through to closure")
        elif veto:
            verdict = "VETO-PENDING"
            why = (f"Axis P surfaced {veto} platform-declared singleton "
                   f"candidate(s); spec §5 reserves the ruling")
        elif c["closure_worst_pct"] >= 90.0 and recv_share < 50.0:
            verdict = "WHOLESALE"
            why = f"closure {c['closure_worst_pct']}% >= 90 (spec §2 guidance)"
        else:
            verdict = "PIECEWISE"
            why = (f"closure {c['closure_worst_pct']}-{c['closure_best_pct']}%"
                   + (f", remainder {recv_share:.0f}% receiver-native "
                      f"(class-(c)-dominated -> wholesale would duplicate the "
                      f"receiver)" if recv_share >= 50.0 else ""))

        out[sub] = {
            "verdict": verdict, "why": why,
            "closure": [c["closure_worst_pct"], c["closure_best_pct"]],
            "receiver_native_share_pct": round(recv_share, 1),
            # §502 Finding A: components, never a single conflated width.
            "gate_width": _gate_width(w_, c),
            "entry_points": len(w_.get("entry_points", {})),
            "ambiguous_external": len(w_.get("ambiguous_called_externally", {})),
            "callbacks": len(w_.get("callbacks", {})),
            "total_surface": [w_.get("width_lower_bound"),
                              w_.get("width_upper_bound")],
            "p_counts": p.get("counts", {}),
            "d_coverage_pct": dd.get("coverage_pct"),
            "d_decomp_pct": dd.get("decomp_pct"),
            "d_stubs": len(dd.get("stubs", [])),
            "d_basis": dd.get("decomp_pct_basis"),
            "indirect_dispatch_tus": c["indirect_dispatch_tus"],
            "tus": len(c["files"]),
        }
    return out, cres, wres, pres, dres


def main():
    argv = sys.argv[1:]
    srcs = load_build_sources()
    if srcs is None:
        print("UNKNOWN — files.cmake not readable; the roster oracle is absent, "
              "so nothing below could be generated. Not a clean run.")
        return 2

    if "--explain" in argv:
        want = argv[argv.index("--explain") + 1].replace("\\", "/")
        hits = [s for s in srcs if s.endswith(want) or want in s]
        if not hits:
            # HT-19 (§493): this branch used to assert "the build does not
            # compile it" -- a claim about the BUILD that the tool had not
            # checked. On libs/ paths it was confidently FALSE (JASChannel.cpp
            # is files.cmake:627). A tool may report what it sees; it may not
            # conclude what it has not tested.
            on_disk = (REPO / want).is_file()
            in_raw = want in FILES_CMAKE.read_text(encoding="utf-8",
                                                   errors="replace")
            print(f"'{want}' is not in the parsed source list.")
            print(f"  exists on disk        : {'yes' if on_disk else 'no'}")
            print(f"  appears in files.cmake: {'yes' if in_raw else 'no'}")
            if in_raw:
                print("  => PARSER DEFECT, not a roster verdict: the build names "
                      "this file but the parser did not yield it. Do not read "
                      "this as 'not compiled'.")
            else:
                print("  => the build does not name it, so the census cannot see "
                      "it. That is the roster oracle working, not a miss.")
            return 0
        for h in hits:
            c = classify_tu(h)
            print(f"\n{h}")
            if c.get("named_weak"):
                print(f"  named     : WEAK match '{c['named_weak']}' with no "
                      f"donor marker and no tag -> NOT counted (a weak pattern "
                      f"can match a receiver-owned TU by coincidence)")
            else:
                print(f"  named     : {c['named'] or 'no — filename convention does not match'}"
                      + (f"  [{c['strength']}]" if c.get('strength') else ""))
            print(f"  declared  : {c['declared'] or 'no // KIT-LINEAGE tag'}"
                  + (f" ({c['declared_ref']})" if c['declared_ref'] else ""))
            print(f"  evidenced : {', '.join(c['evidence']) if c['evidence'] else 'no donor-authority markers in file'}")
            on = on_roster(c)
            print(f"  => on WW-layer roster: {'YES' if on else 'no'}")
        return 0

    if "--full" in argv:
        rows_all = [classify_tu(s) for s in srcs]
        union = roster_union(rows_all)
        full, cres, wres, pres, dres = run_full(srcs, rows_all, union)
        print("STEP 8 — WW SUBSYSTEM CENSUS, all five axes (spec §7)")
        print(f"roster {len(union)} TUs over {len(srcs)} build sources"
              + chr(10))
        hdr = (f"{'subsystem':42s} {'verdict':13s} {'closure':>13} "
               f"{'recv%':>6} {'GATE':>9} {'surface':>9} {'D cov/dec':>11} "
               f"{'stub':>4}")
        print(hdr)
        print("-" * len(hdr))
        for sub, v in sorted(full.items(), key=lambda kv: kv[1]["verdict"]):
            cl = f"{v['closure'][0]:.1f}-{v['closure'][1]:.1f}%"
            g = v["gate_width"]
            wd = ("UNKNOWN" if g["state"] == "UNKNOWN"
                  else f"{g['value'][0]}-{g['value'][1]}")
            sf = f"{v['total_surface'][0]}-{v['total_surface'][1]}"
            dcd = f"{v['d_coverage_pct']}/{v['d_decomp_pct']}"
            print(f"{sub[:42]:42s} {v['verdict']:13s} {cl:>13} "
                  f"{v['receiver_native_share_pct']:6.1f} {wd:>9} "
                  f"{sf:>9} {dcd:>11} {v['d_stubs']:4d}")
        print(chr(10) + "VERDICT REASONING")
        for sub, v in sorted(full.items()):
            print(f"  {sub}")
            print(f"     {v['verdict']}: {v['why']}")
            g = v["gate_width"]
            print(f"     GATE: {g['state']} — {g['why']}")
            print(f"     total surface {v['total_surface'][0]}-"
                  f"{v['total_surface'][1]} = {v['entry_points']} entry + "
                  f"{v['callbacks']} callbacks + {v['ambiguous_external']} "
                  f"ambiguous. NOT a gate width.")
            if v["d_stubs"]:
                print(f"     RISK (Axis D, does not change the verdict): "
                      f"{v['d_stubs']} stub(s); {v['d_basis']}")
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        jl = OUT_DIR / "ww-census-step8.jsonl"
        with jl.open("w", encoding="utf-8") as f:
            for sub, v in sorted(full.items()):
                rec = dict(v)
                rec["subsystem"] = sub
                f.write(json.dumps(rec) + chr(10))
        print(chr(10) + f"wrote {jl.relative_to(REPO)}")
        return 0

    if "--selftest-b" in argv:
        # Falsifiability control (spec §9): the scanner must be able to say ZERO.
        # §492 control 3 came back INCONCLUSIVE: it zeroed against kit_laws.py,
        # a PYTHON file. A C-array scanner returning zero on Python source proves
        # nothing about a C++ TU -- the control could not fail, so it was not an
        # instrument. It now runs against a C++ TU that is ON the roster.
        ZERO_TU = "src/d/d_ext_room_verify.cpp"
        probe = REPO / ZERO_TU
        rows, err = axis_b(ZERO_TU)
        print("Axis B falsifiability control - C++ roster TU, no scalar arrays")
        print(f"  target: {probe.relative_to(REPO)}")
        print(f"  result: B = {len(rows) if rows is not None else 'UNKNOWN'} arrays")
        neg_ok = rows is not None and len(rows) == 0
        print("  scope: a zero here means no arrays of the SCANNED SCALAR TYPES. "
              "This TU does declare a struct array (kWwPortProcs), which the "
              "scanner deliberately does not count -- donor payload is scalar "
              "data, not receiver tables. Stated so the zero is not over-read.")
        print("  VERDICT:", "PASS — the scanner can report zero" if neg_ok else
              "FAIL — a scanner that cannot report zero is not an instrument")

        # POSITIVE control: the one array we already know is donor data.
        print(chr(10) + "Axis B positive control — the known donor display list")
        prows, _ = axis_b("src/d/ext_line/mdoext1_3dline.cpp")
        hit = [r for r in (prows or []) if "toonmat1dl" in r["name"].lower()]
        print("  target: src/d/ext_line/mdoext1_3dline.cpp :: l_toonMat1DL")
        if hit:
            h = hit[0]
            print(f"  found : line {h['line']} class={h['class']} bytes={h['bytes']}"
                  f"   (user ruling 2026-08-05: keep under watch)")
        pos_ok = bool(hit)
        print("  VERDICT:", "PASS — the scanner sees the array the trip-wire is "
              "defined against" if pos_ok else
              "FAIL — cannot see the one array we KNOW is donor data")

        # CROSS-INSTRUMENT control (§507). E4's KIT-DONOR-DATA markers are an
        # INDEPENDENT hand count of the same quantity: Engine counted the bytes,
        # Axis B parses them. This is the control the §508 leg layer lacked --
        # it can come back with the OTHER value. A hole in the regex surfaces as
        # MISSING, bad arithmetic as DISAGREE; neither can present as silence.
        import banner_lint as _BL
        srcs2 = load_build_sources() or []
        rows2 = [classify_tu(s) for s in srcs2]
        roster2 = roster_union(rows2)
        agree = dis = miss = decl_bytes = 0
        for rel in roster2:
            marks = _BL.read_data(rel)
            if not marks:
                continue
            mine, _e = axis_b(rel)
            for nbytes, cls, src, ln in (marks or []):
                decl_bytes += nbytes
                below = sorted([a for a in (mine or []) if a["line"] > ln],
                               key=lambda a: a["line"])
                if not below or below[0]["line"] - ln > 4 \
                        or below[0]["payload"] is not True:
                    miss += 1
                elif below[0]["bytes"] != nbytes:
                    dis += 1
                    print(f"     ! {rel}:{ln} {below[0]['name']}: E4 says "
                          f"{nbytes}, B says {below[0]['bytes']}")
                else:
                    agree += 1
        print(chr(10) + "Axis B cross-instrument control — vs E4 KIT-DONOR-DATA")
        print(f"  declared arrays: {agree + dis + miss}  ({decl_bytes} bytes)")
        print(f"  agree {agree}   disagree {dis}   missing from B {miss}")
        x_ok = dis == 0 and miss == 0 and agree > 0
        print("  VERDICT:", f"PASS — two independent instruments agree on all "
              f"{decl_bytes} declared bytes" if x_ok else
              "FAIL — the instruments disagree; one of them is wrong")
        return 0 if (neg_ok and pos_ok and x_ok) else 1

    # ---- full run ----------------------------------------------------------
    rows = [classify_tu(s) for s in srcs]
    named = [r for r in rows if r["named"]]
    declared = [r for r in rows if r["declared"]]
    native_declared = [r for r in rows if r["declared"] == "native-port"]
    evidenced = [r for r in rows if r["evidence"]]
    # HT-27: the roster predicate is on_roster(), not a locally re-spelled one.
    union = set(roster_union(rows))

    unresolved = roster_resolves()
    if unresolved:
        print(f"HT-18 GUARD — {len(unresolved)} build source path(s) DO NOT "
              f"RESOLVE ON DISK. Every axis over them is silently zero, "
              f"not UNKNOWN. Refusing to report numbers.")
        for u in unresolved[:5]:
            print(f"    {u}")
        return 2
    print(f"WW SUBSYSTEM CENSUS — roster axis (spec §1)")
    # §494 nit: the guard checks the FULL build source list, not just the
    # roster. Saying "roster" under-claimed its scope -- the safe direction,
    # but this lane trades in scope statements, so it says what it checks.
    print(f"  HT-18 guard: all {len(srcs)} build source paths resolve on disk")
    print(f"  build sources parsed from files.cmake : {len(srcs)}")
    print(f"  NAMED by WW convention                : {len(named)}"
          f"   <- a FLOOR, not a lineage count (posture §1.1)")
    print(f"  DECLARED by // KIT-LINEAGE tag        : {len(declared)}"
          f"   ({len(native_declared)} native-port)")
    print(f"  EVIDENCED by donor-authority markers  : {len(evidenced)}")
    print(f"  UNION (roster)                        : {len(union)}")
    print(f"  tag coverage over union               : "
          f"{100.0 * len(declared & set()) if False else 100.0 * len([r for r in rows if r['declared'] and r['path'] in union]) / max(len(union), 1):.0f}%"
          f"   <- rising coverage is the roster becoming trustworthy")

    named_only = sorted({r['path'] for r in named} - {r['path'] for r in evidenced}
                        - {r['path'] for r in native_declared})
    ev_not_named = sorted({r['path'] for r in evidenced} - {r['path'] for r in named})
    print(f"\n  donor-EVIDENCED but NOT convention-named : {len(ev_not_named)}")
    print(f"  (this set is exactly what posture §1.1 says the filename "
          f"convention is structurally blind to)")
    for p in ev_not_named[:15]:
        print(f"     {p}")
    if len(ev_not_named) > 15:
        print(f"     … {len(ev_not_named) - 15} more (see JSONL)")

    # ---- Axis B over the roster -------------------------------------------
    b_rows = []
    for path in sorted(union):
        arr, err = axis_b(path)
        if arr is None:
            continue
        for a in arr:
            a["path"] = path
            b_rows.append(a)
    b_noinit = 0
    for path in sorted(union):
        pp = REPO / path
        if pp.is_file():
            b_noinit += len(NOINIT_DECL.findall(
                _blank_comments(pp.read_text(encoding="utf-8",
                                             errors="replace"))))
    scratch = [a for a in b_rows if a["payload"] is False]
    computed = [a for a in b_rows if a["payload"] == "computed"]
    payload = [a for a in b_rows if a["payload"] is True]
    sized = [a for a in payload if a["bytes"] is not None]
    b_bytes = sum(a["bytes"] for a in sized)
    b_unsized = len(payload) - len(sized)
    by_class, by_basis = {}, {}
    for a in sized:
        by_class[a["class"]] = by_class.get(a["class"], 0) + a["bytes"]
        by_basis[a["basis"]] = by_basis.get(a["basis"], 0) + a["bytes"]

    print(f"\nAXIS B — donor-data payload (spec §6, the membrane check)")
    print(f"  arrays WITH AN INITIALISER     : {len(b_rows)}"
          f"   <- the scanned population; `= {{` is required")
    print(f"  array declarations, no init.   : {b_noinit}"
          f"   (HT-24: not scanned — no payload either way, but stated so "
          f"\"{len(b_rows)}\" is never read as \"arrays in the roster\")")
    print(f"  EXCLUDED — no payload declared : {len(scratch)}"
          f"   (`= {{}}` scratch buffers and the `= {{0}}` zero-fill idiom)")
    print(f"  EXCLUDED — computed at runtime : {len(computed)}"
          f"   (expressions over locals — UNKNOWN, never zero)")
    for a in computed:
        print(f"     {a['path']}:{a['line']} {a['name']}")
    print(f"  PAYLOAD arrays                 : {len(payload)}")
    print(f"  PAYLOAD bytes                  : {b_bytes}")
    for k, v in sorted(by_basis.items(), key=lambda kv: -kv[1]):
        print(f"     via {k:22s} {v} bytes")
    print(f"  payload of unknown size        : {b_unsized}"
          f"   <- reported, never estimated")
    for k, v in sorted(by_class.items(), key=lambda kv: -kv[1]):
        print(f"     {k:20s} {v} bytes")
    print(f"  NOTE: PAYLOAD means elements were written down at the declaration "
          f"— it does NOT mean donor ORIGIN, which only the §8 provenance banner "
          f"establishes. This tally is a superset of the declared donor bytes and "
          f"must never be read as the §5.1 trip-wire figure.")

    print(f"\nAXES NOT IMPLEMENTED IN THIS LANDING — reported UNKNOWN per №31-C")
    for ax, why in (("C (graph closure)", "closure walk not yet generalised from "
                     "enemy_port_kit actor granularity to subsystem granularity"),
                    ("D (decomp completeness)", "needs the donor tree enumerated "
                     "for stub detection — spec §9 forbids doing it from memory"),
                    ("W (interface width)", "the plugin ABI; needs C first"),
                    ("P (platform depth)", "singleton-duplication veto; needs the "
                     "subsystem grouping C produces")):
        print(f"  {ax:26s} UNKNOWN — {why}")
    print(f"\nNo WHOLESALE/PIECEWISE verdict is emitted: a verdict needs C, D, W "
          f"and P. Emitting one from two axes would be a guess with a table "
          f"around it.")

    # ========================================================================
    # HT-16 (§492) — Axis B MUST be diffable and attributable.
    #
    # The JSONL carried 72 roster records and ZERO array records, and no markdown
    # table was written at all, despite this file's own header promising one "so
    # runs diff". Two consequences, both serious:
    #   * when the donor-byte tally moves, nothing says WHICH FILE moved it, and
    #     §5.1 trip-wire (b) is defined on exactly that quantity, feeding a user
    #     ruling at step 9;
    #   * HousingTemp nearly filed "B = 0 for 72/72 TUs" as a measurement. It was
    #     an artifact of the missing records. An instrument whose SILENCE reads
    #     as a ZERO is worse than one that reports UNKNOWN.
    #
    # Every array row now carries file/line/class/bytes, and every roster TU
    # emits an explicit b_arrays count -- including zero, which becomes a RECORD
    # rather than an absence.
    # ========================================================================
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    per_tu = {}
    for a in b_rows:
        per_tu.setdefault(a["path"], []).append(a)

    jl = OUT_DIR / "ww-census.jsonl"
    with jl.open("w", encoding="utf-8") as f:
        f.write(json.dumps({"record": "run", "denominator": len(srcs),
                            "named": len(named), "declared": len(declared),
                            "evidenced": len(evidenced), "roster": len(union),
                            "axis_b_arrays": len(b_rows),
                            "axis_b_sized_bytes": b_bytes,
                            "axis_b_unsized": b_unsized,
                            "axes_unknown": ["C", "D", "W", "P"]}) + chr(10))
        for r in rows:
            if r["path"] in union:
                arr = per_tu.get(r["path"], [])
                rec = dict(r)
                rec["record"] = "tu"
                rec["b_arrays"] = len(arr)
                rec["b_sized_bytes"] = sum(a["bytes"] or 0 for a in arr)
                f.write(json.dumps(rec) + chr(10))
        for a in b_rows:
            rec = dict(a)
            rec["record"] = "array"
            f.write(json.dumps(rec) + chr(10))

    md = OUT_DIR / "ww-census-axis-b.md"
    out = ["# Axis B - donor-data payload (generated by tools/foundry/ww_census.py)",
           "",
           "Candidate arrays in WW-layer roster TUs. **Donor ORIGIN is not proven by this",
           "axis.** Sorted by bytes so the largest movers are attributable at a glance.",
           "", "| file | line | array | class | bytes |", "|---|---|---|---|---|"]
    for a in sorted(b_rows, key=lambda r: -(r["bytes"] or 0)):
        b = a["bytes"] if a["bytes"] is not None else "UNSIZED"
        out.append(f"| `{a['path']}` | {a['line']} | `{a['name']}` | {a['class']} | {b} |")
    out += ["", f"**Totals:** {len(b_rows)} arrays / {b_bytes} sized bytes / "
                f"{b_unsized} unsized (reported, never estimated).", "",
            "TUs with **zero** candidate arrays are recorded explicitly in the JSONL",
            "(`b_arrays: 0`) so a zero is a measurement, not a missing record."]
    md.write_text(chr(10).join(out) + chr(10), encoding="utf-8")

    print(f"{chr(10)}wrote {jl.relative_to(REPO)} ({len(union)} TU records + "
          f"{len(b_rows)} array records + 1 run record)")
    print(f"wrote {md.relative_to(REPO)} (Axis B table, sorted by bytes)")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
