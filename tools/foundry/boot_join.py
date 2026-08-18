#!/usr/bin/env python3
# ============================================================================
# boot_join.py - the BOOT JOIN: feeds LINKED and EXERCISED verdicts from the
#                game's own boot log, against the seam-tracker rows.
#                (SPEC-boot-join.md, Integrator scope 2026-08-16; built by
#                Foundry the same day.)
#
# ---------------------------------------------------------------------------
# WHAT IT DOES. Reads ONE boot log (the plugin's [WwRegistry]/[WwMsg] JSON
# lines), derives per-symbol LINKED / EXERCISED states, and reports where a
# tracker row DISAGREES with what the run proved. It NEVER writes a row -
# same refusal as decomp_watch: a copied verdict is a second roster.
#
# THE ONE DISTINCTION THIS TOOL EXISTS TO KEEP (spec s4): D1's INSTALLED
# means THE SYMBOL RESOLVED. It does NOT mean a callback is attached - a hook
# declared with DEFINE_HOOK_SYMBOL and never passed to hook_add_* reports
# INSTALLED forever and can never fire (on_getRes did exactly this, and the
# person who built D1 believed it). So LINKED is derived from TWO separate
# facts and refuses to collapse them:
#     resolution  <- the hook_manifest walk        (symbol-level)
#     attachment  <- the per-seam attach receipts  (callback-level)
# manifest-INSTALLED with NO attach evidence derives UNKNOWN, never
# REGISTERED. A firing counter, however, PROVES attachment (nothing fires
# unattached), so counter>baseline upgrades attachment with a note.
#
# VERDICT VOCABULARY per (row, axis):
#   AGREE        row value and log-derived value are both known and equal
#   DISAGREE     both known, different - the only red verdict
#   UNEVIDENCED  the log carries no evidence for this symbol (absent is
#                reported, never scored - sP3: absent != unrecognised)
#   ROW-UNSET    the log proved a value the row never recorded; update the
#                row BY HAND - this tool never writes
#   RELINQUISHED row state is deliberate host hand-off; not judged here
#
# EVERY EMITTED VALUE CARRIES `src` (spec s5.5, added 2026-08-16 after the
# Librarian caught its absence; the authority is INVENTORY-SCHEMA s1: every
# axes.* value is {state, src}, and A VALUE WITH NO src IS UNKNOWN). This
# tool's src form: `boot-join:<log-name>@<build-id>`, or `@UNPINNED` when the
# build cannot be identified. The first draft of this very file reproduced
# the spec's original gap - it "named the build" in a header instead, the
# hand-rolled weaker parallel the amendment exists to kill - and was patched
# the same hour the amendment landed. Rows carrying BARE states are noted
# (UNKNOWN by the s1 rule) but their state comparison is still shown: the
# store does not yet demand src of a row (open item, Bridge custody), and a
# detector that cries red on every row today is the false positive that gets
# the detector ignored.
#
# HONEST LIMITS (spec s8), printed on every run:
#   - a log proves that run / that image / that route. A seam idle because
#     the player never walked there is NOT a defect;
#     REGISTERED-NOT-EXERCISED is a fact, never a verdict.
#   - counters were authored per-feature; unmapped seams are UNKNOWN and
#     SAYING SO IS THE POINT.
#   - no log => refuse, exit 2. A missing log is UNKNOWN, not clean - every
#     failure in this class has been a false clean.
#
# Usage:  boot_join.py report <boot.log> [--build-id <id>]
#         boot_join.py --selftest
# Exit 0 = no disagreement · 1 = disagreement(s) · 2 = refused (no/empty log)
# ============================================================================
import json
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
TRACKER = REPO / "docs" / "state" / "ww-staging" / "tracker"

# ---------------------------------------------------------------------------
# ONE ROW PARSER ON THE BOARD. row_store.py owns the row format; importing
# its parse_row instead of writing a second one is the calls_rotate lesson -
# my own private row-reader carried the same single-line assumption the
# Librarian had to fix in someone else's.
# ---------------------------------------------------------------------------
def _row_store():
    import importlib.util
    spec = importlib.util.spec_from_file_location("rs", HERE / "row_store.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


# ---------------------------------------------------------------------------
# THE EVIDENCE MAP - hand-authored, BOUNDED, and printed with the report so
# its coverage is never mistaken for the seam population's. Each entry was
# read out of mods-src/ww_donor_disc/registry.cpp on 2026-08-16 (line refs
# below). A symbol NOT in this map derives UNKNOWN - loudly, in its own
# section - because a join that silently covers only the instrumented seams
# is the partial-coverage lie the spec exists to end (s8).
#
#   attach : (event, field)   field value "MOD_OK" = callback attached;
#                             any other MOD_* = attach attempted and failed
#   counter: ("shutdown", key) read from the shutdown receipt, or
#            ("events", ev)    count of that ev's occurrences in the log
#   baseline: int             subtracted before judging exercise; name_hits
#                             carries the self-test's own +1 (registry.cpp
#                             ~2790: "Expect exactly 1 with no placement")
# ---------------------------------------------------------------------------
EVIDENCE = {
    # registry.cpp:2621 "install" + :2796 shutdown receipt
    "fpcPf_Get":         {"attach": ("install", "fpcPf_Get"),
                          "counter": ("shutdown", "pf_hits")},
    "cDyl_IsLinked":     {"attach": ("install", "cDyl_IsLinked"),
                          "counter": ("shutdown", "dyl_hits")},
    "dStage_searchName": {"attach": ("install", "dStage_searchName"),
                          "counter": ("shutdown", "name_hits"),
                          "baseline": 1},
    # registry.cpp:2361 getres_attach ("ATTACHED is not the same as D1
    # INSTALLED") + bg_model_lookup event lines as the fire counter
    "dRes_control_c::getRes": {"attach": ("getres_attach", "result"),
                               "counter": ("events", "bg_model_lookup")},
    # registry.cpp:2373 j3d_hooks (three fields, one receipt)
    "J3DModelLoaderDataBase::loadBinaryDisplayList":
        {"attach": ("j3d_hooks", "loadBinaryDisplayList"),
         "counter": ("events", "j3d_bdl")},
    "J3DModelLoaderDataBase::load":
        {"attach": ("j3d_hooks", "load"),
         "counter": ("events", "j3d_bmd")},
    # registry.cpp:2378 msgarc_hook + :1167 msgarc_set fire lines. The key
    # is the DONOR-NATIVE name: a FREE FUNCTION (Housing read it out of the
    # export table - ?dComIfGp_setMsgDtArchive@@YAX...), not a play_c
    # method as this map's first draft paraphrased it.
    "dComIfGp_setMsgDtArchive":
        {"attach": ("msgarc_hook", "setMsgDtArchive"),
         "counter": ("events", "msgarc_set")},
    # registry.cpp:270 - the second stock-side message door (Housing's
    # export-table row). Its only counter is msggroup_skipped, which counts
    # the SKIP branch alone: ONE-SIDED evidence. Non-zero proves the hook
    # fired; ZERO PROVES NOTHING (the hook may fire and never skip), so a
    # zero derives UNKNOWN, never REGISTERED-NOT-EXERCISED - sP4, metrics
    # fail pessimistic.
    "dMsgObject_c::readMessageGroupLocal":
        {"counter": ("shutdown", "msggroup_skipped"), "one_sided": True},
    # registry.cpp:2535 b2_bind + :2113 warp_request fire lines
    "dStageMgr_c::setNextStage":
        {"attach": ("b2_bind", "setNextStage"),
         "counter": ("events", "warp_request")},
}

RE_LINE = re.compile(r"\[(?:WwRegistry|WwMsg)\]\s*(\{.*\})\s*$")
# a mangled name contains the flat identifier at a boundary: '?tok@' or
# 'tok@' at start; a bare match would let dStage_getName swallow
# dStage_getName2 (token then '@' or string end is required)
def _manifest_matches(mangled, token):
    flat = token.split("::")[-1]
    for m in re.finditer(re.escape(flat), mangled):
        end = m.end()
        if (end == len(mangled) or mangled[end] in "@("):
            if m.start() == 0 or mangled[m.start() - 1] in "?@:_ ":
                return True
    return False


def parse_log(path):
    """One pass; returns facts + honesty flags. Never guesses."""
    facts = {
        # mangled name -> {"resolved": bool, "attach": str|None}
        # `attach` is the per-record field Housing added 2026-08-16 after
        # measuring the spec's premise false (attachment was never emitted;
        # the old walk keyed severity on resolution alone). Old-format logs
        # have no attach field -> None, and the strict on_getRes rule keeps
        # governing them: INSTALLED is never read as REGISTERED.
        "manifest": {},
        "manifest_ran": False,   # summary with records>0 seen
        "attach_events": {},     # ev -> {field: value}  (last wins)
        "counters": {},          # shutdown receipt ints
        "event_counts": {},      # ev -> occurrences
        "shutdown_seen": False,
        "lines_total": 0, "lines_parsed": 0, "lines_bad_json": 0,
    }
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        facts["lines_total"] += 1
        m = RE_LINE.search(line)
        if not m:
            continue
        try:
            d = json.loads(m.group(1))
        except ValueError:
            facts["lines_bad_json"] += 1
            continue
        facts["lines_parsed"] += 1
        ev = d.get("ev", "")
        facts["event_counts"][ev] = facts["event_counts"].get(ev, 0) + 1
        if ev == "hook_manifest":
            facts["manifest"][d.get("name", "")] = {
                "resolved": str(d.get("state", "")).startswith("INSTALLED"),
                "attach": d.get("attach"),
            }
        elif ev == "hook_manifest_summary":
            # records:0 with a 'why' is the BOUNDS-UNAVAILABLE case - the
            # enumeration DID NOT RUN, which is not the same as zero hooks
            # (registry.cpp:2245). manifest_ran stays False for it.
            if int(d.get("records", 0)) > 0:
                facts["manifest_ran"] = True
        elif ev in ("install", "getres_attach", "j3d_hooks", "msgarc_hook",
                    "b2_bind"):
            facts["attach_events"].setdefault(ev, {}).update(d)
        elif ev == "shutdown":
            facts["shutdown_seen"] = True
            for k, v in d.items():
                if isinstance(v, int):
                    facts["counters"][k] = v
    return facts


def derive(symbol, facts):
    """-> (linked, exercised, notes[])  each in schema vocabulary or None
    when the log simply carries nothing for this symbol."""
    ev = EVIDENCE.get(symbol)
    notes = []

    # resolution, from the manifest walk (symbol-level)
    resolved = None
    man_attach = []
    if facts["manifest_ran"]:
        hits = [rec for name, rec in facts["manifest"].items()
                if _manifest_matches(name, symbol)]
        if hits:
            resolved = all(r["resolved"] for r in hits)
            man_attach = [r["attach"] for r in hits]
            if len(hits) > 1:
                notes.append("%d manifest records matched" % len(hits))

    # attachment - PRIMARY: the manifest's own per-record `attach` field
    # (added 2026-08-16; keyed on record address plugin-side, so one record
    # is one hook). SECONDARY: the older bespoke receipts, kept as a
    # cross-check and as the only evidence on old-format logs.
    attached = None
    if man_attach and any(a is not None for a in man_attach):
        vals = [a for a in man_attach if a is not None]
        if any(str(a).startswith("UNKNOWN") for a in vals):
            attached = None
            notes.append("attach ledger overflowed - attachment is UNKNOWN, "
                         "which is never clean")
        elif all(str(a) == "ATTACHED" for a in vals):
            attached = True
        else:
            attached = False
            notes.append("manifest attach: %s"
                         % "; ".join(str(a) for a in vals if str(a) != "ATTACHED"))
    if ev and "attach" in ev:
        evname, field = ev["attach"]
        val = facts["attach_events"].get(evname, {}).get(field)
        if val is not None:
            receipt = (str(val) == "MOD_OK")
            if attached is None:
                attached = receipt
            elif attached != receipt:
                notes.append("MANIFEST ATTACH AND BESPOKE RECEIPT DISAGREE "
                             "(%s vs %s) - the log contradicts itself; "
                             "trust neither" % (attached, val))
                attached = None
            if receipt is False:
                notes.append("attach receipt: %s" % val)
                # MOD_UNAVAILABLE is the loader saying the SYMBOL was not
                # found - that is resolution evidence too
                if "MOD_UNAVAILABLE" in str(val):
                    resolved = False

    # exercise, from the counter - and a firing counter PROVES attachment
    fired = None
    if ev and "counter" in ev:
        src, key = ev["counter"]
        raw = (facts["counters"].get(key) if src == "shutdown"
               else facts["event_counts"].get(key))
        if raw is None and src == "shutdown" and not facts["shutdown_seen"]:
            notes.append("no shutdown receipt in this log - counter unmeasured")
        elif raw is not None:
            fired = (raw - ev.get("baseline", 0)) > 0
            if not fired and ev.get("one_sided"):
                fired = None
                notes.append("counter '%s' is ONE-SIDED (counts one branch "
                             "only); its zero proves nothing - exercise "
                             "stays UNKNOWN" % key)
            if fired and attached is None:
                attached = True
                notes.append("attachment proven by fire (%s=%d)" % (key, raw))
            if fired and attached is False:
                notes.append("COUNTER FIRED WITH FAILED ATTACH RECEIPT - "
                             "the log contradicts itself; trust neither")

    # LINKED (spec s4) - the collapse-refusal lives here
    if resolved is False:
        linked = "NOT"
    elif attached is True:
        linked = "REGISTERED"
    elif resolved is True and attached is False:
        linked = "PENDING"
    elif resolved is True and attached is None:
        linked = None
        notes.append("INSTALLED but attachment unevidenced - the on_getRes "
                     "trap; INSTALLED is never read as REGISTERED")
    else:
        linked = None

    # EXERCISED (spec s4)
    if linked == "REGISTERED" and fired is True:
        exercised = "EXERCISED"
    elif linked == "REGISTERED" and fired is False:
        exercised = "REGISTERED-NOT-EXERCISED"
    elif linked in ("NOT", "PENDING"):
        exercised = "NOT"
    else:
        exercised = None
    return linked, exercised, notes


def report(log_path, build_id=None, rows_dir=None, quiet=False):
    out = []
    p = Path(log_path) if log_path else None
    if p is None or not p.is_file():
        print("REFUSED - no boot log at %r. A missing log is UNKNOWN, not "
              "clean; every failure in this class has been a false clean "
              "(spec s5.3)." % (str(log_path) if log_path else "(none)"))
        return 2
    facts = parse_log(p)
    if facts["lines_parsed"] == 0:
        print("REFUSED - %s contains no [WwRegistry]/[WwMsg] emission "
              "(%d lines read). Not a clean boot: an UNOBSERVED one."
              % (p.name, facts["lines_total"]))
        return 2

    rs = _row_store()
    rows_root = Path(rows_dir) if rows_dir else (TRACKER / "rows")
    row_files = sorted(rows_root.glob("*.md")) if rows_root.is_dir() else []

    add = out.append
    src_tag = "boot-join:%s@%s" % (p.name, build_id or "UNPINNED")
    add("BOOT JOIN - %s  (%d/%d lines parsed%s)"
        % (p.name, facts["lines_parsed"], facts["lines_total"],
           ", %d bad JSON" % facts["lines_bad_json"]
           if facts["lines_bad_json"] else ""))
    add("src for EVERY value below: %s" % src_tag)
    if not build_id:
        add("  UNPINNED - this run proves things about an artifact this "
            "report cannot name; pass --build-id from build_ledger to pin it")
    if not facts["manifest_ran"]:
        add("manifest walk: DID NOT RUN (bounds unavailable or absent) - "
            "resolution evidence is UNKNOWN for every symbol, not zero")
    if not facts["shutdown_seen"]:
        add("shutdown receipt: ABSENT - run ended abnormally or log is "
            "truncated; shutdown counters are unmeasured")
    add("")

    disagreements = 0
    unsourced_rows = 0
    unevidenced_syms = []
    for rf in row_files:
        f = rs.parse_row(rf)
        rid = f.get("id", rf.stem)
        syms = [s.strip() for s in f.get("symbols", "").split(",") if s.strip()]
        row_axis = {"LINKED": f.get("linked"), "EXERCISED": f.get("exercised")}
        if row_axis["LINKED"] == "RELINQUISHED":
            add("%s  LINKED=RELINQUISHED (deliberate host hand-off; not "
                "judged here)" % rid)
            continue
        for sym in syms:
            linked, exercised, notes = derive(sym, facts)
            for axis, got in (("LINKED", linked), ("EXERCISED", exercised)):
                said = row_axis[axis]
                if got is None:
                    if sym not in unevidenced_syms:
                        unevidenced_syms.append(sym)
                    continue
                if said is None:
                    verdict = "ROW-UNSET (log proved %s; record it BY HAND " \
                              "with src=%s - this tool never writes)" \
                              % (got, src_tag)
                elif said == got:
                    verdict = "AGREE"
                    unsourced_rows += 1  # bare state; store has no src yet
                else:
                    verdict = "** DISAGREE **"
                    disagreements += 1
                    unsourced_rows += 1
                add("%s  %-9s row=%-26s log=%-26s %s"
                    % (rid, axis, said or "(unset)", got, verdict))
                for n in notes:
                    add("           note: %s" % n)

    if unevidenced_syms:
        add("")
        add("UNEVIDENCED - %d symbol(s) with NOTHING in this log (absent is "
            "reported, never scored):" % len(unevidenced_syms))
        for s in unevidenced_syms:
            add("    %s" % s)

    # ------------------------------------------------------------------
    # THE OTHER DIRECTION - FIRED-NOT-ENUMERATED (Phase B's reconcile,
    # spec: "every fired-and-not-enumerated row is a Phase-A defect;
    # iterate A until a full run produces zero"). The loop above walks
    # ROWS and derives from the log; it can never see a hook the log
    # proves live that NO row claims. This walks the log's own manifest
    # and lists every record no row's symbols match - the exit gate
    # ("two consecutive runs, zero unenumerated fires") is measured on
    # THIS list reading empty.
    # ------------------------------------------------------------------
    all_row_syms = []
    for rf in row_files:
        f = rs.parse_row(rf)
        all_row_syms += [s.strip() for s in f.get("symbols", "").split(",")
                         if s.strip()]
    unenumerated = [name for name in facts["manifest"]
                    if not any(_manifest_matches(name, sym)
                               for sym in all_row_syms)]
    if unenumerated:
        add("")
        add("** FIRED-NOT-ENUMERATED - %d manifest record(s) the log proves "
            "live that NO tracker row claims (each is a Phase-A defect per "
            "the Phase-B reconcile; the exit gate is this list EMPTY on two "
            "consecutive runs): **" % len(unenumerated))
        for name in unenumerated:
            add("    %s" % name)
    if unsourced_rows:
        add("")
        add("%d row-side value(s) compared BARE - no src on the row, so each "
            "is UNKNOWN by INVENTORY-SCHEMA s1. State comparisons are shown "
            "anyway: the store does not yet demand src of a row (open item, "
            "Bridge custody). When recording, carry src=%s."
            % (unsourced_rows, src_tag))
    add("")
    add("evidence map covers %d symbol(s); the map is bounded and hand-read "
        "from registry.cpp 2026-08-16 - a seam outside it is UNKNOWN, and "
        "saying so is the point (spec s8)." % len(EVIDENCE))
    add("REGISTERED-NOT-EXERCISED is a FACT about this run/route, never a "
        "verdict - a seam the player never reached is not a defect.")
    add("")
    add("%d disagreement(s)." % disagreements)
    if not quiet:
        print("\n".join(out))
    return 1 if disagreements else 0


# ---------------------------------------------------------------------------
# SELF-TEST - the spec's s6 controls, each shown able to go red. A gate that
# cannot fail certifies nothing; every case below is the join REFUSING or
# FLAGGING, and a silent pass on any of them prints BLIND.
# ---------------------------------------------------------------------------
FIXTURE_LOG_OK = "\n".join([
    '[WwRegistry] {"ev":"hook_manifest","name":"?dStage_searchName@@YAPAUstage_scls_info_class@@PADH@Z","state":"INSTALLED","resolved":"0x1"}',
    '[WwRegistry] {"ev":"hook_manifest","name":"?getRes@dRes_control_c@@SAPAXPBD0PAUDynName@@H@Z","state":"INSTALLED","resolved":"0x2"}',
    '[WwRegistry] {"ev":"hook_manifest_summary","records":2,"hooks_installed":2,"hooks_not_installed":0}',
    '[WwRegistry] {"ev":"install","fpcPf_Get":"MOD_OK","cDyl_IsLinked":"MOD_OK","dStage_searchName":"MOD_OK","base":666,"rows":3,"names":3}',
    '[WwRegistry] {"ev":"shutdown","pf_hits":12,"dyl_hits":0,"name_hits":1,"search_calls_total":87,"set_stage_res_calls":0}',
])
FIXTURE_LOG_NOTINST = "\n".join([
    '[WwRegistry] {"ev":"hook_manifest","name":"?dStage_searchName@@YAPAUstage_scls_info_class@@PADH@Z","state":"NOT-INSTALLED(unresolved at load)","resolved":"(nil)"}',
    '[WwRegistry] {"ev":"hook_manifest_summary","records":1,"hooks_installed":0,"hooks_not_installed":1}',
])

def _mkrow(d, rid, extra=""):
    (d / ("%s.md" % rid)).write_text(
        "id: %s\nsymbols: dStage_searchName\ntu: src/d/d_stage.cpp\n"
        "doorway: EXISTS\ndestination: PLUGIN\ncreated: selftest\n%s"
        % (rid, extra), encoding="utf-8", newline="\n")

def selftest():
    import tempfile
    print("SELFTEST - each case MUST refuse or flag (spec s6)\n")
    blind = 0
    def case(name, ok):
        nonlocal blind
        blind += 0 if ok else 1
        print("  [%-44s] %s" % (name, "DETECTED" if ok else
                                "**BLIND - CHECK CANNOT FAIL**"))

    base = Path(tempfile.mkdtemp())
    (base / "ok.log").write_text(FIXTURE_LOG_OK, encoding="utf-8")
    (base / "notinst.log").write_text(FIXTURE_LOG_NOTINST, encoding="utf-8")

    # 1. NOT-INSTALLED in the log => LINKED: NOT; a row claiming REGISTERED
    #    must be listed (spec s6 bullet 1 + 4 in one fixture)
    d1 = base / "r1"; d1.mkdir(); _mkrow(d1, "s00000011", "linked: REGISTERED\n")
    rc = report(base / "notinst.log", rows_dir=d1, quiet=True)
    case("NOT-INSTALLED vs row REGISTERED -> DISAGREE", rc == 1)

    # 2. attached + counter at baseline => REGISTERED-NOT-EXERCISED, not NOT
    #    and not EXERCISED; a row claiming EXERCISED must be listed
    d2 = base / "r2"; d2.mkdir()
    _mkrow(d2, "s00000012", "linked: REGISTERED\nexercised: EXERCISED\n")
    rc = report(base / "ok.log", rows_dir=d2, quiet=True)
    case("attached+idle counter vs row EXERCISED -> DISAGREE", rc == 1)
    l, e, _ = derive("dStage_searchName",
                     parse_log(base / "ok.log"))
    case("derivation is R-N-E, not NOT, not EXERCISED",
         l == "REGISTERED" and e == "REGISTERED-NOT-EXERCISED")

    # 3. absent log => refuse (the single most important line in the spec)
    rc = report(base / "does-not-exist.log", rows_dir=d1, quiet=True)
    case("absent log -> REFUSED exit 2", rc == 2)
    (base / "empty.log").write_text("no emission here\n", encoding="utf-8")
    rc = report(base / "empty.log", rows_dir=d1, quiet=True)
    case("log without emission -> REFUSED exit 2", rc == 2)

    # 4. THE on_getRes CONTROL: manifest INSTALLED, no attach receipt =>
    #    LINKED must derive UNKNOWN (None), never REGISTERED
    (base / "ok_no_attach.log").write_text("\n".join([
        '[WwRegistry] {"ev":"hook_manifest","name":"?getRes@dRes_control_c@@SAPAXPBD0PAUDynName@@H@Z","state":"INSTALLED","resolved":"0x2"}',
        '[WwRegistry] {"ev":"hook_manifest_summary","records":1,"hooks_installed":1,"hooks_not_installed":0}',
    ]), encoding="utf-8")
    l, e, notes = derive("dRes_control_c::getRes",
                         parse_log(base / "ok_no_attach.log"))
    case("INSTALLED w/o attach receipt -> UNKNOWN, never REGISTERED",
         l is None and any("on_getRes" in n for n in notes))

    # 5. the positive path can also fail: attach MOD_OK + counter above
    #    baseline must reach EXERCISED (derivation not stuck at R-N-E)
    (base / "fired.log").write_text(FIXTURE_LOG_OK.replace(
        '"name_hits":1', '"name_hits":9'), encoding="utf-8")
    l, e, _ = derive("dStage_searchName", parse_log(base / "fired.log"))
    case("attached+fired counter -> EXERCISED", e == "EXERCISED")

    # 6-8. THE TWO-AXIS MANIFEST (Housing 2026-08-16): the per-record
    #    `attach` field is primary evidence, and each of its three states
    #    must derive differently - PENDING is not REGISTERED, and an
    #    overflowed ledger is UNKNOWN, never clean.
    def _man(attach):
        return "\n".join([
            '[WwRegistry] {"ev":"hook_manifest","name":"?dStage_searchName@@YAPAUstage_scls_info_class@@PADH@Z","state":"INSTALLED","attach":"%s","resolved":"0x1"}' % attach,
            '[WwRegistry] {"ev":"hook_manifest_summary","records":1,"hooks_installed":1,"hooks_not_installed":0}',
        ])
    (base / "notatt.log").write_text(
        _man("NOT-ATTACHED(declared, never passed to hook_add_*)"),
        encoding="utf-8")
    l, e, _ = derive("dStage_searchName", parse_log(base / "notatt.log"))
    case("INSTALLED + NOT-ATTACHED -> PENDING, never REGISTERED",
         l == "PENDING" and e == "NOT")
    (base / "overflow.log").write_text(
        _man("UNKNOWN(attach ledger overflowed - not a clean result)"),
        encoding="utf-8")
    l, e, notes = derive("dStage_searchName", parse_log(base / "overflow.log"))
    case("attach ledger overflow -> UNKNOWN, never clean",
         l is None and any("overflowed" in n for n in notes))
    (base / "attached.log").write_text(_man("ATTACHED"), encoding="utf-8")
    l, e, _ = derive("dStage_searchName", parse_log(base / "attached.log"))
    case("manifest ATTACHED alone -> REGISTERED", l == "REGISTERED")

    # 8b. FIRED-NOT-ENUMERATED: a manifest record no row claims must be
    #     LISTED (the Phase-B reconcile direction; a join that only walks
    #     rows is blind to it by construction)
    import io as _io, contextlib as _ctx
    d4 = base / "r4"; d4.mkdir()
    _mkrow(d4, "s00000014")  # claims dStage_searchName only
    (base / "extra.log").write_text(FIXTURE_LOG_OK + "\n" +
        '[WwRegistry] {"ev":"hook_manifest","name":"?totallyNovelHook@@YAXXZ","state":"INSTALLED","attach":"ATTACHED","resolved":"0x9"}',
        encoding="utf-8")
    _buf = _io.StringIO()
    with _ctx.redirect_stdout(_buf):
        report(base / "extra.log", rows_dir=d4, quiet=False)
    case("manifest record no row claims -> FIRED-NOT-ENUMERATED",
         "FIRED-NOT-ENUMERATED" in _buf.getvalue()
         and "totallyNovelHook" in _buf.getvalue())

    # 9. ONE-SIDED COUNTERS (Housing's two-doors row): msggroup_skipped
    #    counts the skip branch only, so ATTACHED + zero must derive
    #    UNKNOWN exercise - never REGISTERED-NOT-EXERCISED (a zero that
    #    proves nothing must not be dressed as a measurement) - while a
    #    non-zero still proves EXERCISED.
    def _msgdoor(skips):
        return "\n".join([
            '[WwRegistry] {"ev":"hook_manifest","name":"?readMessageGroupLocal@dMsgObject_c@@QEAAXPEAPEAVmDoDvdThd_mountXArchive_c@@@Z","state":"INSTALLED","attach":"ATTACHED","resolved":"0x3"}',
            '[WwRegistry] {"ev":"hook_manifest_summary","records":1,"hooks_installed":1,"hooks_not_installed":0}',
            '[WwRegistry] {"ev":"shutdown","msggroup_skipped":%d}' % skips,
        ])
    (base / "oneside0.log").write_text(_msgdoor(0), encoding="utf-8")
    l, e, notes = derive("dMsgObject_c::readMessageGroupLocal",
                         parse_log(base / "oneside0.log"))
    case("one-sided counter at 0 -> exercise UNKNOWN, not R-N-E",
         l == "REGISTERED" and e is None
         and any("ONE-SIDED" in n for n in notes))
    (base / "oneside3.log").write_text(_msgdoor(3), encoding="utf-8")
    l, e, _ = derive("dMsgObject_c::readMessageGroupLocal",
                     parse_log(base / "oneside3.log"))
    case("one-sided counter fired -> EXERCISED", e == "EXERCISED")

    # 10. spec s5.5 / INVENTORY-SCHEMA s1: every emitted value carries src.
    #    A ROW-UNSET recommendation without its src is the exact gap the
    #    Librarian caught - capture stdout and require the src form present.
    import io, contextlib
    d3 = base / "r3"; d3.mkdir(); _mkrow(d3, "s00000013")  # no axes set
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        report(base / "ok.log", rows_dir=d3, quiet=False)
    case("emitted values carry src=boot-join:<log>@UNPINNED",
         "boot-join:ok.log@UNPINNED" in buf.getvalue())

    print("\n%s" % ("CONTROL OK - all cases DETECTED" if blind == 0
                    else "** %d BLIND CASE(S) **" % blind))
    return 1 if blind else 0


def main():
    argv = sys.argv[1:]
    if "--selftest" in argv or (argv and argv[0] == "selftest"):
        return selftest()
    if argv and argv[0] == "report":
        argv = argv[1:]
    build_id = None
    if "--build-id" in argv:
        i = argv.index("--build-id")
        build_id = argv[i + 1] if i + 1 < len(argv) else None
        argv = argv[:i] + argv[i + 2:]
    if not argv:
        print("usage: boot_join.py report <boot.log> [--build-id <id>] | "
              "--selftest")
        return 2
    return report(argv[0], build_id=build_id)


if __name__ == "__main__":
    sys.exit(main())
