#!/usr/bin/env python3
# ============================================================================
# lane_queue.py — generate the open-item queue FROM THE BUS, with a stamp.
#
# WHY THIS EXISTS
# Four consecutive hand-offs re-listed work closed one to three sections
# earlier: the fold_detect main() fix, the zstandard declaration and HT-28 were
# each carried forward as open after §531 closed them, and §506/§507/§508 were
# re-listed three times after §510/§511. Nobody was careless -- the queue lived
# in each lane's memory of the last message rather than in an artifact, so every
# hand-off re-derived it from a stale snapshot.
#
# That is HT-28 one level up. HT-28 stamped Axis P's roster because a verdict
# that cannot name its input is not reproducible; a queue that cannot name its
# closures is not either.
#
# WHAT IT DOES, AND WHAT IT REFUSES TO DO
# It scans the bus for CLOSURE CLAIMS -- "closed in §N", "fixed in §N", a table
# row naming an item and a section -- and reports each item's last claimed state
# with the section that claimed it. It does NOT adjudicate: a claim recorded here
# is a claim, not a verdict, and the lane that raised the item still rules on
# whether it is satisfied. An instrument that closed items on its own say-so
# would be the reporting lane auditing itself.
#
# Read-only. Usage: lane_queue.py [--emit]
# ============================================================================
import hashlib
import re
import sys
import pathlib
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
BUS = REPO / "docs/WW Linked/ww-bridge-tool-interconnected.md"
OUT = REPO / "docs/WW Linked/ww-lane-queue.md"

# An item is a stable token lanes actually use in prose.
ITEM = re.compile(r"\b(HT-\d+|C[1-7][a-e]?|B[1-4]|D-[123]|"
                  r"§\d{3}\s+(?:Finding\s+[A-C]|D-[123]))\b")
SECTION = re.compile(r"^## §(\d+)", re.M)
CLOSED = re.compile(r"\b(closed|CLOSED|fixed|FIXED|resolved|RESOLVED|"
                    r"done|DONE)\b")
OPEN = re.compile(r"\b(open|OPEN|not started|NOT CONTROLLED|still|blocked)\b")


# ---------------------------------------------------------------------------
# ACCEPTANCE TESTS — because a prose claim is not a closure (§541).
#
# Two defects surfaced together when another lane tested two of the 16 closure
# claims this tool reported:
#
#   1. TOKEN COLLISION. `B1`-`B4` names the containment campaign's ABI blockers
#      AND the ww_bridge extractor/installer series. Two item spaces, one token
#      space. The tool merged them and reported one state for both.
#   2. A CLAIM IS NOT A TEST. `ww_bridge` as a CLI was never built; pieces of its
#      job landed in scattered tools under other names -- `gate` really exists,
#      as covenant_gate.py. So prose said "done" because functionality existed
#      SOMEWHERE, while the item's own acceptance criterion was unmet. **That is
#      how a half-built roadmap reports as finished**, and this tool amplified it.
#
# So: where an item has a checkable criterion, the CHECK OVERRIDES THE PROSE. An
# item with no check reports CLAIMED, never CLOSED -- the word has to carry its
# own evidence.
def _src(rel):
    p = REPO / rel
    return p.read_text(encoding="utf-8", errors="replace") if p.is_file() else ""


def _has(rel, *needles):
    t = _src(rel)
    return all(n in t for n in needles)


F = "tools/foundry/"
GATE = "src/d/ext_plugin/ww_import_gate.cpp"

# item -> (criterion, test, STRENGTH)
#
# STRENGTH is published because a weak test passing is not the same claim as a
# strong one passing, and collapsing them is the defect one level down from the
# one §541 caught. `behavioural` runs the thing; `source` only proves the code
# says it does; `none` means UNCHECKABLE and reports UNKNOWN, never CLOSED.
# --- §543 Housing acceptance helpers (behavioural: they RUN the tool) --------
def _leg_debt_text():
    import subprocess, sys as _s
    r = subprocess.run([_s.executable, str(REPO / "tools/foundry/leg_debt.py")],
                       capture_output=True, encoding="utf-8", errors="replace",
                       cwd=str(REPO), timeout=900)
    return r.stdout or ""


def _legs_consistent():
    import re as _re
    out = _leg_debt_text()
    m = _re.search(r"TOTAL:\s*(\d+)\s*legs", out)
    if not m:
        return None
    scoped = out.count("at FILE SCOPE") + len(_re.findall(r"^ +in \w+\(\) @", out, _re.M))
    return scoped == int(m.group(1))


def _d2():
    return "in dDemo_setDemoData()" in _leg_debt_text()


def _d3():
    out = _leg_debt_text()
    return "CHECKED CLEAN" in out and "UNMEASURED" in out


def _census():
    import sys as _s
    _s.path.insert(0, str(REPO / "tools/foundry"))
    import ww_census as _wc
    return _wc


def _ht15():
    _wc = _census()
    import ww_layer_manifest as _m
    return _wc.WWLM is _m


def _ht18():
    _wc = _census()
    srcs = _wc.load_build_sources() or []
    if not srcs:
        return None
    return all((REPO / s).is_file() for s in srcs)


def _ht23():
    import sys as _s
    _s.path.insert(0, str(REPO / "tools/foundry"))
    import census_axis_c as _c
    return "GXTexObj" in _c.build_decl_index()


# --- §544 Housing: behavioural probes for the WWB items --------------------
def _wwb_path():
    return REPO / "tools/ww_crew_restoration_skeleton/ww_bridge.py"


def _wwb_subcommand(name):
    """True only if ww_bridge dispatches `name` -- not merely mentions it."""
    f = _wwb_path()
    if not f.is_file():
        return False
    src = f.read_text(encoding="utf-8", errors="replace")
    return f'cmd == "{name}"' in src


def _wwb_verify_runs():
    """RUN it. A filename check would pass on an empty file."""
    import subprocess, sys as _s
    f = _wwb_path()
    if not f.is_file():
        return False
    r = subprocess.run([_s.executable, str(f), "verify"],
                       capture_output=True, encoding="utf-8", errors="replace",
                       cwd=str(REPO), timeout=300)
    # no args -> usage; that proves dispatch without needing a live install
    return "verify" in (r.stdout or "") and _wwb_subcommand("verify")


def _b2c_decodable():
    """B2c: every chunk type PRESENT IN THE LIVE ARCS resolves to an entry size.

    Cites the census rather than a hand-made file. `_b2_chunk_coverage` read
    `docs/state/ww-b2-chunk-coverage.txt` -- a snapshot someone produced -- so it
    could go stale against the arcs it claims to describe without anything
    noticing. The census is reproducible now, so the tags come from the ARCS.

    The invariant is deliberately open-ended: add an arc, or a chunk type inside
    one, and this tightens by itself. It cannot be satisfied by editing a list,
    which is what a hand-made file always permits.

    Returns None -- UNKNOWN, never clean (No31-C) -- when the arcs or the
    extractor are unreachable. An unrunnable census is not a passing one.

    FIXED (§564). The previous reader unpacked a chunk count from offset 0 of
    the .arc itself. These are RARC ARCHIVES; the DZR/DZS chunk table lives in a
    member INSIDE them, so every arc failed the plausibility guard, `seen_any`
    stayed False and this returned UNKNOWN against 83 real arcs.

    Worth stating why that was dangerous rather than merely wrong: UNKNOWN is
    exactly what an unreachable census SHOULD return, so the broken reader
    produced the correct-looking verdict for an incorrect reason and would have
    survived review on the artifact alone. "Cannot reach the census" and "cannot
    parse the container" were two different failures wearing one word.

    Now calls `chunk_census.scan()`, which walks RARC via the extractor's own
    `list_rarc` and classifies with the extractor's own `entry_size`. That is
    the point: a census that re-implements the container reader can disagree
    with the extractor it is auditing, and then neither is authoritative.
    """
    import importlib
    sys.path.insert(0, str(REPO / "tools/ww_crew_restoration_skeleton"))
    try:
        cc = importlib.import_module("chunk_census")
        rows, n_arcs, n_stages, skipped = cc.scan()
    except Exception:
        return None
    # No stage files parsed at all means the census did not RUN. That is
    # UNKNOWN, and it must stay distinguishable from "ran and found nothing
    # undecodable" -- collapsing them is the defect this function just had.
    if not rows or n_stages == 0:
        return None
    # An arc the census could not open has not been measured, so "every chunk
    # type present in the live arcs" is unverified for it. Passing here would
    # assert coverage of arcs nobody read -- the same silent undercount one
    # layer down. UNKNOWN, never clean.
    if skipped:
        return None
    # §565 -- PREDICATE WIDENED, on Foundry's finding. This asked only for the
    # absence of UNSIZED: a class that no longer occurs, so it reported 43/43
    # while 15 types produced nothing. Knowing a record stride is the
    # PRECONDITION for handling a chunk, not handling it -- the same "keyed to
    # the first property achieved" defect as WWB:B2 and WWB:B4.
    #
    # Now: every type is PLACED, DELIVERED, or carries an explicit disposition
    # WITH a reason. A new chunk type in a new arc has no entry, lands in
    # UNCLASSIFIED and fails -- silence is not consent.
    bad = [r for r in rows if r[4] in ("UNSIZED", "UNCLASSIFIED")]
    if bad:
        return False
    # A disposition with an empty reason is a rubber stamp, not a ruling.
    try:
        disp = cc.DISPOSITION
    except AttributeError:
        return None
    for tag, _e, _a, _s, tier in rows:
        if tier in ("NO-PLACEMENT", "DEFERRED", "DELIVERED"):
            if not disp.get(tag, ("", ""))[1].strip():
                return False
    return True


def _b2d_paths():
    """B2d: every RPPN waypoint belongs to exactly one RPAT path.

    Foundry's criterion is "every stage carrying RPPN also emits RPAT
    groupings". Checked as a PARTITION rather than as a file's existence,
    because a paths.csv with one row would satisfy the weaker reading while
    leaving 473 points orphaned -- the same "first property achieved" defect
    that has now bitten WWB:B2, WWB:B4 and B2c.

    Runs over EVERY stage in the live arcs, not just A_mori: the extractor
    processes one stage, but the invariant is a property of the data.
    """
    import os, struct, importlib
    sys.path.insert(0, str(REPO / "tools/ww_crew_restoration_skeleton"))
    ext = REPO / "tools/ww_crew_restoration_skeleton/extract_amori_census.py"
    live = pathlib.Path(os.path.expandvars("%APPDATA%")) / (
        "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")
    if not ext.is_file() or not live.is_dir():
        return None
    ns = {}
    try:
        exec(compile(ext.read_text(encoding="utf-8"), str(ext), "exec"), ns)
        read_paths, be32 = ns["read_paths"], ns["be32"]
        list_rarc, decomp = ns["list_rarc"], ns["maybe_decompress"]
    except Exception:
        return None
    checked = 0
    for d in ("arcs", "arcs_lib"):
        if not (live / d).is_dir():
            continue
        for arc in sorted((live / d).glob("*.arc")):
            try:
                data = decomp(arc.read_bytes())
                members = list(list_rarc(data))
            except Exception:
                continue
            for name, off, size in members:
                if not name.endswith((".dzr", ".dzs")):
                    continue
                blob = data[off:off + size]
                if len(blob) < 8:
                    continue
                n = be32(blob, 0)
                if n <= 0 or n > 200 or 4 + n * 12 > len(blob):
                    continue
                chunks = {}
                for i in range(n):
                    o = 4 + i * 12
                    chunks[blob[o:o + 4].decode("ascii", "replace")] = be32(blob, o + 4)
                if "RPPN" not in chunks:
                    continue
                checked += 1
                paths, pnt_path = read_paths(blob)
                # Every point owned exactly once, and every path's declared
                # count equal to the points that resolved to it.
                if set(pnt_path) != set(range(chunks["RPPN"])):
                    return False
                for pid, num, *_ in paths:
                    if sum(1 for v in pnt_path.values() if v[0] == pid) != num:
                        return False
    return None if checked == 0 else True


def _b2_chunk_coverage():
    """B2's OTHER half: every chunk type measured in the arcs is DECODABLE.

    The registered test checked the `scls` subcommand and nothing else, so once
    SCLS landed the tool reported B2 closed -- which is verbatim the failure
    §549 warned about: "Landing SCLS and calling B2 done would be the same
    defect in a new place." SCLS is one chunk type out of 43.

    Keyed to an INVARIANT, not a figure: every chunk type present in the arcs
    must resolve to an entry size. Add a chunk type and this tightens by itself;
    it cannot be satisfied by editing a count.

    NOW READS THE ARCS, NOT THE SNAPSHOT (§564). This parsed
    `docs/state/ww-b2-chunk-coverage.txt` with a regex -- a file that was
    hand-made when this test was written, so the test could pass against a
    stale description of arcs it never opened. Foundry named this as the same
    mistake as B2c's reader, and it was: both audited a derived artifact instead
    of the source. Delegates to the same census B2c uses, so the two cannot
    return different answers to the same question.
    """
    return _b2c_decodable()


def _b2_generated():
    """B2 closes when door binding is GENERATED -- checked where it belongs.

    My §544 test looked for "Generated by" in the REPO's doors.ini. That was
    wrong on my own reasoning: generating donor coordinates INTO the repo is
    exactly what trip-wire (b) exists to stop, so the correct target is the live
    install. Fixing the test to match the design rather than the reverse.
    """
    import os
    live = pathlib.Path(os.path.expandvars("%APPDATA%")) / (
        "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")
    f = live / "population" / "door_bindings.ini"
    if not f.is_file():
        return False
    head = f.read_text(encoding="utf-8", errors="replace")[:600]
    return "GENERATED from SCLS" in head and "tool_sha256" in head


ACCEPTANCE = {
    # ======================================================================
    # §543 HOUSING — the eleven checks I actually ran in §540, registered so
    # they stop being prose. By this tool's own standard my verifications were
    # claims too: I ran them, wrote the results in the bus, and the generator
    # could not see any of it. CLOSED-VERIFIED existed with zero users.
    #
    # KEYED TO INVARIANTS, NOT FIGURES — deliberately. HT-25's claim was
    # "6 -> 3" and it reads 6 -> 4 today; my own manifest authority was "21"
    # and it is 15. Both moved because the input moved, not because anything
    # broke. A test keyed to a snapshot fails spuriously on the next roster
    # change and trains people to ignore it. So each test below asserts the
    # PROPERTY the fix established, never the number it produced that day.
    # ======================================================================
    "HT-15": ("census consumes Housing's generator -- ONE classifier, not two",
              lambda: _ht15(), "behavioural"),
    "HT-18": ("build_sources() resolves every entry (count itself is free to move)",
              lambda: _ht18(), "behavioural"),
    "HT-23": ("GXTexObj RESOLVES -- the false-clear direction is closed",
              lambda: _ht23(), "behavioural"),
    # B2b-1 RETIRED (§562): its target set measured EMPTY -- the six ACT/SCO
    # layer types it named were never unextracted. An item whose scope is empty
    # cannot be closed OR failed; retiring it is the only honest disposition.
    # B2c replaces it, keyed to the arcs rather than to a snapshot of them.
    # B2d (§566, Foundry's ruling): RPAT is a hole INSIDE delivered work, not an
    # unstarted subsystem -- RPPN already ships, so deferring it leaves output
    # that looks whole and is not. Checked as a partition, not as file presence.
    "B2d": ("every RPPN waypoint belongs to exactly one RPAT path, and each "
            "path's declared count equals the points resolving to it",
            lambda: _b2d_paths(), "behavioural"),
    "B2c": ("every chunk type PRESENT IN THE LIVE ARCS resolves to an entry "
            "size -- derived from the arcs, not from a hand-made census file",
            lambda: _b2c_decodable(), "behavioural"),
    "WWB:B2": ("ww_bridge `scls` generating door binding into the LIVE install "
               "(repo must NOT gain donor coordinates -- trip-wire b) AND every "
               "chunk type measured in the arcs resolving to an entry size",
               lambda: _wwb_subcommand("scls") and _b2_generated()
                       and _b2_chunk_coverage(),
               "behavioural"),
    # SWEEP HIT (§560). Same shape as the B2 you widened: B4's scope is TWO
    # commands -- `ww_bridge verify` AND `ww_bridge gate <exe>` (§541) -- and the
    # test checked only `verify`, the first one that landed. `gate` existing
    # under a different name (covenant_gate.py) is exactly the "functionality
    # exists somewhere" reasoning that made B4 read closed while ww_bridge did
    # not exist at all. B4 asks for a ww_bridge SUBCOMMAND; a separate script is
    # not one, and the parenthetical was quietly conceding the gap it named.
    "WWB:B4": ("ww_bridge `verify` RUNS its checks AND `gate` is a ww_bridge "
               "subcommand -- both commands, not the first one to land",
               lambda: _wwb_verify_runs() and _wwb_subcommand("gate"),
               "behavioural"),
    # --- 19c gate: none of these can pass while the gate does not exist ------
    # --- 19c gate: REPOINTED to src/d/ext_plugin/ (§555) ---------------------
    # The old glob searched `src/dusk/mods/**/*import*gate*` and could never
    # match -- but the reason matters more than the miss. `src/dusk/mods/` is
    # DUSKLIGHT'S OWN mod subsystem. WW-layer code living there is precisely the
    # crossing `ww-declared-crossings.md` polices, and step 17's lesson is that
    # we do not own dusklight. So the test was not merely looking in the wrong
    # folder: **it was asserting the gate should sit where a covenant crossing
    # would be**, and it would have reported CLOSED only if someone had made
    # that crossing. A test whose PASS condition is a violation is worse than a
    # test that cannot pass.
    #
    # Each key below is a DISTINCT outcome constant, so no test can pass for
    # another's reason. C2 is the one most easily omitted: a gate that can only
    # refuse is as dead as one that can only pass.
    "C1": ("gate refuses on MISSING",
           lambda: _has(GATE, "WW_GATE_REFUSE_MISSING"), "source"),
    "C2": ("gate PASSES a clean list -- it must be able to say yes",
           lambda: _has(GATE, "WW_GATE_PASS"), "source"),
    "C4": ("absent manifest is UNKNOWN and OUTRANKS MISSING -- without a "
           "manifest the other symbols were never tested, so MISSING would "
           "assert what was not checked",
           lambda: _has(GATE, "WW_GATE_UNKNOWN_MANIFEST"), "source"),
    "C6": ("refusal is TOTAL -- FOLDED produces a whole-gate outcome, not a "
           "per-symbol skip",
           lambda: _has(GATE, "WW_GATE_REFUSE_FOLDED"), "source"),
    "C3e": ("a real ICF-on address map to test a genuine fold against",
            lambda: bool(list(REPO.glob("**/*.map"))), "behavioural"),
    "C5": ("an SDK record kind for resolve-then-CALL (not hook)",
           lambda: "MOD_META_IMPORT_SYMBOL" in _src("sdk/include/mods/api.h")
                   or "RESOLVE_SYMBOL" in _src("sdk/include/mods/api.h"),
           "behavioural"),
    "C7": ("gate asserts against the manifest COUNT and verifies its stamp",
           lambda: _has(F + "fold_detect.py", "C7 asserts against",
                        "generator freshness"), "source"),
    # --- census instrument fixes --------------------------------------------
    "HT-28": ("Axis P stamps its roster into its own output",
              lambda: _has(F + "census_axis_p.py", "__roster__", "roster_stamp"),
              "source"),
    "HT-27": ("one roster predicate, called by the other tools",
              lambda: _has(F + "ww_census.py", "def on_roster", "def roster_union")
                      and _has(F + "banner_lint.py", "roster_union")
                      and _has(F + "leg_debt.py", "roster_union"), "source"),
    "HT-26": ("DUPLICATED names the site instead of asserting 'receiver'",
              lambda: _has(F + "census_axis_p.py",
                           "instantiated outside this subsystem"), "source"),
    "HT-25": ("Axis P distinguishes a WW-declared type from a receiver one",
              lambda: _has(F + "census_axis_p.py", "SOLE-INSTANCE-WW"), "source"),
    # ---- §544: the remaining prose-only rows -----------------------------
    # Keyed to INVARIANTS per Housing's rule, never to the figure a fix produced.
    "ABI:B4": ("import surface classified by declaration OWNERSHIP, with the "
               "DuskLog logger excluded from the ABI",
               lambda: _has(F + "binding_plan.py", "WW-owned: not a host import"),
               "source"),
    "C3": ("the C3 family closes only when a-e all close; C3e is NOT CONTROLLED",
           lambda: False, "behavioural"),
    "HT-20": ("the LABEL defect fix is carried in the axis that had it",
              lambda: _has(F + "census_axis_c.py", "HT-20"), "source"),
    "HT-22": ("HT-22's fix is carried in Axis C",
              lambda: _has(F + "census_axis_c.py", "HT-22"), "source"),
    # --- UNCHECKABLE: the criterion is not known to this lane ---------------
    # These report UNKNOWN, never CLOSED and never OPEN. Guessing a criterion
    # and testing against it would manufacture a verdict, which is the defect
    # this whole layer exists to stop -- and I have already produced two false
    # failures by testing the wrong thing.
    # Criterion supplied by Housing (§550): the latch signals overflow with -1
    # and both call sites gated `n > 0`, dropping it silently.
    "HT-8": ("3dline probe latch: overflow (-1) is REPORTED at the call sites, "
             "not dropped by an `n > 0` gate",
             lambda: _has("src/m_Do/m_Do_ext.cpp", "HT-8 3dline probe UNTRACKED"),
             "source"),
    "HT-13": ("criterion not known to Foundry -- text never supplied",
              lambda: None, "none"),
    "HT-14": ("criterion not known to Foundry -- text never supplied",
              lambda: None, "none"),
    # §548: the USER ruled CLI-first with the GUI wizard as the END GOAL, so
    # B3 is no longer an undecided question -- it has a buildable criterion.
    # The wizard is NOT part of this test: B3 closes on the ENGINE the wizard
    # will drive (separable steps, machine-readable output, no interactive
    # prompts in the core). Testing for the GUI here would make B3 unclosable
    # for months and hide the CLI's own readiness.
    "WWB:B3": ("installer CLI exists with separable steps and machine-readable "
               "output (GUI wizard is a later item, not this test) — USER ruled "
               "§548",
               lambda: any("ww_bridge" in q.name
                           for q in REPO.glob("tools/**/*.py")), "behavioural"),
    "HT-24": ("Axis B states the basis of its count",
              lambda: _has(F + "ww_census.py", "arrays WITH AN INITIALISER"),
              "source"),
    "D-1": ("leg_debt prints the scope split and says to cite it",
            lambda: _has(F + "leg_debt.py", "SCOPE SPLIT"), "source"),
    "D-2": ("wrapped signatures open a frame (pending-signature parse)",
            lambda: _has(F + "leg_debt.py", "pending"), "source"),
    "D-3": ("UNMEASURED split from CHECKED CLEAN",
            lambda: _has(F + "leg_debt.py", "checked_clean"), "source"),
}

# ---------------------------------------------------------------------------
# BEHAVIOURAL PASS (--behavioural). A source test proves the code SAYS it does
# the thing; only running it proves it does. Most of these need a census pass,
# which is slow -- so ONE shared pass feeds many items rather than each test
# paying for its own. Default runs stay source-level and fast; the artifact
# records which strength was actually achieved, so a cheap run can never be
# mistaken for an expensive one.
def behavioural_pass():
    """item -> (passed, evidence). Runs the instruments and reads their OUTPUT."""
    import io, contextlib, sys as _s
    _s.path.insert(0, str(REPO / "tools/foundry"))
    out = {}

    def _cap(fn, *a, **k):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            try:
                fn(*a, **k)
            except SystemExit:
                pass
        return buf.getvalue()

    try:
        import ww_census as WC, census_axis_p as AP, leg_debt as LD
        srcs = WC.load_build_sources() or []
        rows = [WC.classify_tu(s) for s in srcs]
        roster = WC.roster_union(rows)

        # HT-28 — the stamp must be IN THE OUTPUT, not just in the source.
        pres = AP.run(roster, srcs)
        st = pres.get("__roster__")
        out["HT-28"] = (bool(st and st.get("sha256") and st.get("count")),
                        f"roster stamp emitted: {st.get('count') if st else None} TUs")

        # HT-25 / HT-26 — the classes and wording must appear in real verdicts.
        verds, reasons = set(), []
        for k, v in pres.items():
            if k == "__roster__":
                continue
            for f in v["findings"]:
                verds.add(f["verdict"])
                reasons.append(f["why"])
        # HT-21 -- Axis P's UNKNOWN path must be REACHABLE (it was dead: the
        # primitive filter swept undeclared types out the same door).
        out["HT-21"] = ("UNKNOWN" in verds,
                        "Axis P's UNKNOWN verdict is reachable in real output")
        # ABI:B2 -- the user's NO VETO ruling: nothing may remain VETO-PENDING.
        # ABI:B2 is deliberately NOT computed here: VETO-PENDING is a
        # SUBSYSTEM verdict, and `verds` holds Axis P FINDING verdicts
        # (SOLE-INSTANCE/DUPLICATED/...). Testing it here passed trivially
        # because the token can never appear -- a false PASS. It is computed
        # from `full` alongside B1/B3 below, where the verdict actually lives.
        out["HT-25"] = ("SOLE-INSTANCE-WW" in verds,
                        f"verdicts produced: {sorted(verds)}")
        out["HT-26"] = (any("instantiated outside this subsystem" in r
                            for r in reasons),
                        "DUPLICATED/VALUE-TYPE reasons name the site")

        # ABI:B1 / ABI:B3 -- read the census's own verdicts and gate states.
        # Both are BLOCKERS, so each closes only when its blocking condition is
        # gone: B1 when no gate is UNKNOWN, B3 when a WHOLESALE verdict exists.
        try:
            full, _c, _w, _p, _d = WC.run_full(srcs, rows, roster)
            vs = {d.get("verdict") for d in full.values()}
            gates = [d.get("gate_width") for d in full.values()]
            unk = any(isinstance(g, dict) and g.get("state") == "UNKNOWN"
                      for g in gates)
            out["ABI:B1"] = (not unk,
                             "gates still UNKNOWN (decode lane D-2)" if unk
                             else "every gate measured")
            out["ABI:B2"] = ("VETO-PENDING" not in vs,
                             "subsystem verdicts: "
                             + ", ".join(sorted(v for v in vs if v)))
            out["ABI:B3"] = ("WHOLESALE" in vs,
                             "verdicts present: "
                             + ", ".join(sorted(v for v in vs if v)))
        except Exception as _e:
            out["ABI:B1"] = (None, "census run_full failed: " + str(_e))
            out["ABI:B3"] = (None, "census run_full failed: " + str(_e))

        # D-1 / D-3 — leg_debt's real output.
        txt = _cap(LD.main)
        out["D-1"] = ("SCOPE SPLIT" in txt, "leg_debt prints SCOPE SPLIT")
        out["D-3"] = ("CHECKED CLEAN" in txt or "UNMEASURED" in txt,
                      "UNMEASURED reported separately from CHECKED CLEAN")

        # HT-24 — the basis line in the census's real Axis B output.
        ctxt = _cap(WC.main) if hasattr(WC, "main") else ""
        out["HT-24"] = ("arrays WITH AN INITIALISER" in ctxt,
                        "Axis B states its scanned population")
    except Exception as e:      # a pass that dies reports UNKNOWN, never clean
        out["__error__"] = (None, f"{type(e).__name__}: {e}")

    # C3a-C3d — run the FIXTURES against the detector directly. Calling
    # fold_control.main() without --detector made the harness correctly report
    # NOT CONTROLLED, and this test then read that as the ITEMS failing. A test
    # that mis-invokes its subject blames the subject.
    try:
        import fold_control as FC, fold_detect as FD
        for name, man, sym, exp, _why in FC.CASES:
            got = {a: set(n) for a, n in dict(FD.detect(man, sym) or {}).items()}
            key = name.split()[0]
            out[key] = (got == exp, f"fixture {key}: expected {exp or 'none'}")
    except Exception as e:
        for key in ("C3a", "C3b", "C3c", "C3d"):
            out[key] = (None, f"{type(e).__name__}: {e}")
    return out


# WORKSTREAM NAMESPACE. `B1`-`B4` names the containment campaign's ABI blockers
# AND the ww_bridge extractor/installer series. The token cannot disambiguate
# itself, so the SECTION it appears in does: a section discussing ww_bridge,
# SCLS, doors.ini or the installer is the WWB series; one discussing gates,
# vetoes or the decode lane is ABI. A section carrying NEITHER signal yields
# `B?`, which reports AMBIGUOUS -- refusing is correct there, because one state
# for two item spaces is worse than none.
COLLIDING = {"B1", "B2", "B3", "B4"}
WWB_SIGNAL = re.compile(r"ww_bridge|SCLS|scls|doors\.ini|DZR|installer|"
                        r"extractor|TGDR", re.I)
ABI_SIGNAL = re.compile(r"GATE UNKNOWN|decode lane|VETO|step 18|import surface|"
                        r"plugin boundary|D-2", re.I)

# An author who wrote `WWB:B2` or `ABI:B2` has already answered the question.
# ITEM matches on a \b boundary, and `:` is a non-word character, so the bare
# `B2` inside `WWB:B2` matched and the qualifier in front of it was discarded --
# the resolver then fell through to heuristics to re-derive what the text stated
# outright. An explicit prefix OUTRANKS both the line and the section: it is the
# author's word, not an inference from vocabulary.
EXPLICIT_PREFIX = re.compile(r"(WWB|ABI):$")

# A token pair written as a RANGE (`B1`-`B4`) refers to the NAMESPACE, not to
# any one item, so it carries no status for B1 or B4 individually. These appear
# in prose describing the numbering scheme itself -- which is also where CLOSED
# misfires on "resolved", meaning "disambiguated" rather than "item closed".
RANGE_MENTION = re.compile(r"`?B[1-4]`?\s*[-–—]\s*`?B[1-4]`?")


def _workstream(body):
    w, a = bool(WWB_SIGNAL.search(body)), bool(ABI_SIGNAL.search(body))
    if w and not a:
        return "WWB"
    if a and not w:
        return "ABI"
    return None


def scan():
    if not BUS.is_file():
        return None
    text = BUS.read_text(encoding="utf-8", errors="replace")
    bounds = [(m.start(), int(m.group(1))) for m in SECTION.finditer(text)]
    bounds.append((len(text), None))

    state = {}
    for i in range(len(bounds) - 1):
        start, sec = bounds[i]
        body = text[start:bounds[i + 1][0]]
        ws = _workstream(body)
        for line in body.splitlines():
            ranges = [r.span() for r in RANGE_MENTION.finditer(line)]
            for m in ITEM.finditer(line):
                key = m.group(1)
                if key in COLLIDING and any(
                    a <= m.start() and m.end() <= b for a, b in ranges
                ):
                    continue  # namespace reference, not a claim about an item
                if key in COLLIDING:
                    explicit = EXPLICIT_PREFIX.search(line[: m.start()])
                    if explicit:
                        # The author qualified it. Take their word and stop.
                        key = f"{explicit.group(1)}:{key}"
                        if CLOSED.search(line):
                            state.setdefault(key, []).append(
                                (sec, "CLOSED-CLAIMED", line.strip()[:90]))
                        elif OPEN.search(line):
                            state.setdefault(key, []).append(
                                (sec, "OPEN-CLAIMED", line.strip()[:90]))
                        continue
                    # Resolve per-LINE first, then fall back to the section.
                    # A section that discusses BOTH workstreams -- which any
                    # hand-off summarising both lanes does -- yielded `B?:` even
                    # when the individual LINE was unambiguous. The refusal was
                    # right at section granularity and simply too coarse: the
                    # sentence naming B2 alongside `ww_bridge` is not ambiguous
                    # just because a later paragraph mentions a gate.
                    lws = _workstream(line) or ws
                    key = f"{lws}:{key}" if lws else f"B?:{key}"
                if CLOSED.search(line):
                    state.setdefault(key, []).append((sec, "CLOSED-CLAIMED", line.strip()[:90]))
                elif OPEN.search(line):
                    state.setdefault(key, []).append((sec, "OPEN-CLAIMED", line.strip()[:90]))
    return state



# §543: duplicate-key guard. Seven keys were registered twice (Housing's §543
# entries and Foundry's §542 entries), and the dict literal silently kept the
# last of each -- so half the tests present were dead, with nothing said. A tool
# that exists to stop conflated items must not silently conflate its own.
def _assert_unique_acceptance():
    import re as _re
    src = Path(__file__).read_text(encoding="utf-8", errors="replace")
    blk = src[src.index("ACCEPTANCE = {"):]
    keys = _re.findall(r'^    "([^"]+)":', blk, _re.M)
    dupes = sorted({k for k in keys if keys.count(k) > 1})
    if dupes:
        raise SystemExit(
            "lane_queue: DUPLICATE ACCEPTANCE keys, later entries silently win: "
            + ", ".join(dupes))


_assert_unique_acceptance()

def main():
    state = scan()
    if state is None:
        print("BUS NOT FOUND — UNKNOWN, not empty (№31-C).")
        return 2

    beh = behavioural_pass() if "--behavioural" in sys.argv else {}
    if beh.get("__error__"):
        print(f"  BEHAVIOURAL PASS FAILED: {beh['__error__'][1]}")
        print("  -> those items report UNKNOWN, not clean (No31-C)" + chr(10))

    rows = []
    for key, hist in sorted(state.items()):
        sec, kind, quote = hist[-1]
        if key in beh:
            passed, ev = beh[key]
            kind = ("CLOSED-BEHAVIOURAL" if passed is True else
                    "OPEN-BEHAVIOURAL" if passed is False else "UNKNOWN")
            quote = f"[behavioural] {ev}"
        elif key in ACCEPTANCE:
            why, test, strength = ACCEPTANCE[key]
            try:
                passed = bool(test())
            except Exception:
                passed = None
            if passed is True:
                kind = "CLOSED-VERIFIED" if strength == "behavioural" else "CLOSED-SRC"
            elif passed is False:
                # The check overrides the prose. This is the whole point.
                kind = "OPEN-VERIFIED" if strength == "behavioural" else "OPEN-SRC"
                quote = f"acceptance FAILS [{strength}]: {why}"
            else:
                kind = "UNKNOWN"
                quote = f"acceptance UNCHECKABLE: {why}"
        elif key.startswith("B?:"):
            kind = "AMBIGUOUS"
            quote = ("token names items in two workstreams (containment "
                     "blockers and the ww_bridge series) — one state for two "
                     "item spaces is worse than none")
        elif not key.startswith("B?:"):
            kind = "CLAIMED-UNTESTED"
            quote = "no acceptance test — prose only, must not read as closed"
        rows.append((key, sec, kind, quote, len(hist)))

    openish = [r for r in rows if r[2].startswith("OPEN")]
    closed = [r for r in rows if r[2].startswith("CLOSED")]
    other = [r for r in rows if not r[2].startswith(("OPEN", "CLOSED"))]
    untested = [r for r in rows if r[2] == "CLAIMED-UNTESTED"]

    print("LANE QUEUE — last claimed state per item, derived from the bus")
    print(f"  bus sections scanned : {len(SECTION.findall(BUS.read_text(encoding='utf-8', errors='replace')))}")
    print(f"  items tracked        : {len(rows)}")
    print(f"  last claim = OPEN    : {len(openish)}")
    print(f"  last claim = CLOSED  : {len(closed)}\n")
    print("  OPEN (most recent claim):")
    for key, sec, _k, quote, n in sorted(openish, key=lambda r: -(r[1] or 0))[:24]:
        print(f"    {key:12s} §{sec}  ({n} mentions)  {quote}")

    if "--emit" in sys.argv:
        body = ["# Lane queue — generated, do not hand-edit",
                "#",
                "# Last CLAIMED state per item, derived from the bus. A claim is a",
                "# claim, not a verdict: the lane that raised an item still rules on",
                "# whether it is satisfied. This exists because four consecutive",
                "# hand-offs re-listed work closed one to three sections earlier.",
                f"# regenerate: python tools/foundry/lane_queue.py --emit",
                f"# bus_sha256: {hashlib.sha256(BUS.read_bytes()).hexdigest()}",
                f"# tool_sha256: {hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}",
                f"# items: {len(rows)}   open: {len(openish)}   closed: {len(closed)}"
                f"   ambiguous/unknown: {len(other)}",
                "# A CLAIMED state is prose; a VERIFIED state ran an acceptance test.",
                "",
                "| item | last claim | section | mentions |",
                "|---|---|---|---|"]
        for key, sec, kind, _q, n in sorted(rows, key=lambda r: (not r[2].startswith("OPEN"), r[0])):
            body.append(f"| `{key}` | {kind} | §{sec} | {n} |")
        OUT.write_text("\n".join(body) + "\n", encoding="utf-8")
        print(f"\n  EMITTED {OUT.relative_to(REPO)}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
