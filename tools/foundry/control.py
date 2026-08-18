#!/usr/bin/env python3
# ============================================================================
# control.py — A GATE MAY NOT REPORT A PASS UNTIL IT HAS BEEN SHOWN TO FAIL.
#
# ---------------------------------------------------------------------------
# WHY THIS EXISTS — the finding all five lane handoffs converged on
# (tale §1002 exercise, 2026-08-14). Pooled methodology errors ran roughly
# 50 to 6 against context-degradation, and sorted by MECHANISM rather than
# symptom they collapse into ONE shape:
#
#     **AN OBSERVATION THAT COULD NOT HAVE COME OUT DIFFERENTLY.**
#
#   · a compile gate that never invoked the compiler          (Foundry)
#   · `cl /Zs` called a compile gate — syntax-only, never links, so four
#     LNK2005s reached the Integrator THROUGH A GREEN GATE      (Housing)
#   · "citing a compile gate I had never seen go red"           (History)
#   · a probe that sampled BEFORE the init block, so it could not answer
#     its own question                                          (History)
#   · a verification probe whose own match string was wrong     (History)
#   · grep sweeps returning zero because `\b` is unsupported     (Foundry)
#   · `TaskList` reporting "no tasks" while three monitors fired (Foundry)
#   · a name-absence grep declaring a live system unported     (Integrator)
#   · ruling on a token identity when the roster was SILENT    (Librarian)
#
# In every case the observation was real and the inference was unsupported,
# because the observation had no capacity to come out the other way.
#
# **THAT IS WHY IT WAS SYSTEMATIC ACROSS FIVE INDEPENDENT LANES: A VACUOUS
# PASS IS INDISTINGUISHABLE FROM A REAL PASS.** Care cannot detect it — care
# produces the same output either way. The only detector is deliberately
# making the instrument fail.
#
# ---------------------------------------------------------------------------
# WHY IT IS A TOOL AND NOT A RULE. Every lane INDEPENDENTLY derived this
# control and wrote it down — "every gate needs a control that must fail" ·
# "when the authority is silent, FLAG, never rule" · "assert nothing past what
# the measurement supports". **Every lane then broke their own rule, most
# within minutes of writing it.** Housing's conclusion is the one that made
# this file necessary:
#
#     "Producing doctrine substituted for changing practice. Treat my written
#      rules as evidence of what I kept getting wrong, not as evidence it was
#      fixed."
#
# The rules that WORKED today were executable (file_row.py re-reads the
# artifact; the lane-token lint rejects). The ones that failed were written.
# **So this rule becomes a command.**
#
# ---------------------------------------------------------------------------
# Usage:
#   control.py audit              every gate: has a control? does it go red?
#   control.py check <gate>       run one gate's negative control
#   control.py guard <gate> -- <cmd...>
#                                 run the CONTROL first; only if it goes red,
#                                 run <cmd> and report. Otherwise refuse.
#
# Exit 0 = controls fired as required · 1 = a gate could NOT be made to fail
# (its passes are worthless) · 2 = could not run.
# ============================================================================
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
PY = sys.executable

# ---------------------------------------------------------------------------
# THE REGISTRY. Each entry names a gate and an input that MUST make it fail.
#
# `must_fail` is the whole point: it is not "an input that fails" but "an
# input whose PASS proves the gate is blind". A gate with no entry here is
# reported UNVERIFIABLE — not clean, not broken, UNKNOWN (№31-C) — because
# nobody has ever demonstrated it can go red.
#
# REFINEMENT (2026-08-17, the completeness-test breakage): THE FIXTURE MUST
# BE A SPECIMEN OF THE EXACT GUARDED CLASS. History's s801 completeness test
# HAD a control (tripped 2/869) and was STILL blind — its statement filter
# counted the signature line, so a >32-byte EMPTY BODY (the guarded class)
# could never flag, while the control tripped on unrelated statement-less
# oddities. Six NPC "completeness" passes were the test being incapable of
# reporting anything else. A control that goes red on SOMETHING is not a
# control that goes red on THE RIGHT THING: when writing `must_fail`, ask
# "is this input a member of the class the gate exists to catch?" — if it
# merely produces red output by another route, the gate is uncontrolled.
CONTROLS = {
    "compile_gate": {
        "why": "A TU with `static_assert(sizeof(int)==1)`. If this compiles "
               "clean the gate is not invoking the compiler — the exact defect "
               "found at tale §977, where every historical TIER2 pass was "
               "vacuous.",
        "cmd": [PY, str(HERE / "compile_gate.py"),
                str(HERE / "controls" / "false_static_assert.cpp"), "--compile"],
        # the gate must NOT come back clean
        "fails_when": lambda rc, out: ("error C" in out or "UNRESOLVED" in out
                                       or "FINDINGS" in out),
        "vacuous_looks_like": "TIER2 cl /Zs clean",
    },
    "sig_diff": {
        "why": "`fopAcM_GetParamBit` is a KNOWN divergence, verified "
               "independently by Housing at source: donor takes a u32 VALUE, "
               "receiver takes an actor POINTER it dereferences. If sig_diff "
               "calls it SAME, its SAME verdicts mean nothing.",
        "cmd": [PY, str(HERE / "sig_diff.py"), "fopAcM_GetParamBit"],
        # ================================================================
        # THE CONTROL'S OWN CONTROL (2026-08-14). This predicate was
        # `"DIVERGENT" in out` — and the vacuous output it was written to
        # catch is the summary line `0 DIVERGENT of 1 checked`, which
        # CONTAINS that substring. A blind sig_diff would have been rubber-
        # stamped RED by its own negative control. **The registry entry was
        # itself an observation that could not come out the other way** —
        # the defect this whole file exists to catch, committed inside it.
        # Now: require the verdict LINE for the symbol, and explicitly
        # reject the zero-summary. Verified by running it against the
        # broken tool (crash -> BLIND) and the fixed one (RED).
        # ================================================================
        "fails_when": lambda rc, out: (
            re.search(r"^\*\*DIVERGENT\s+fopAcM_GetParamBit\b", out, re.M)
            is not None
            and "0 DIVERGENT of" not in out),
        "vacuous_looks_like": "0 DIVERGENT of 1 checked",
    },
    "sig_diff_same": {
        # ================================================================
        # THE OTHER DIRECTION — added 2026-08-14. The `sig_diff` control
        # above only proves the gate CAN say DIVERGENT. A gate that said
        # DIVERGENT for everything would pass it, and be just as worthless:
        # it burns a porter's day reproducing a divergence that is not
        # there. That is a VACUOUS FAILURE, and it is not hypothetical —
        # four symbols (cM_rndF, cM_rndFX, fopAcM_setCarryNow,
        # dBgS_GetWaterHeight) were falsely DIVERGENT because an unnamed
        # parameter normalised to the empty string.
        # **A gate needs a control in every direction it can rule.**
        # ================================================================
        "why": "`cM_ssin` is byte-identical in both trees — Housing caught "
               "sig_diff publishing it as divergent (tale §977). It MUST "
               "come back SAME. If it does not, the gate is stuck on "
               "DIVERGENT and every divergence it reports is unfalsifiable.",
        "cmd": [PY, str(HERE / "sig_diff.py"), "cM_ssin"],
        "fails_when": lambda rc, out: (
            re.search(r"^\s+SAME\s+cM_ssin\b", out, re.M) is not None
            and "DIVERGENT    cM_ssin" not in out),
        "vacuous_looks_like": "**DIVERGENT cM_ssin (everything is divergent)",
    },
    "monitor_pulse": {
        # ================================================================
        # Added 2026-08-15, arming this lane's monitor. The handoff records
        # a monitor that died unnoticed and a `TaskList` that cannot see
        # monitors at all — so the ONLY evidence a watcher is alive is the
        # stamp it writes. That makes the liveness check a gate, and a gate
        # that has never gone red certifies nothing. A pulse from 1970 must
        # read DEAD.
        # ================================================================
        "why": "controls/pulse_dead.json is stamped at epoch 0. It MUST read "
               "DEAD. If monitor_pulse calls a 1970 stamp ALIVE it is not "
               "comparing anything, and 'my monitor is alive' becomes a "
               "blanket yes — the exact blind spot that let a dead monitor "
               "run unnoticed until the user caught it.",
        "cmd": [PY, str(HERE / "monitor_pulse.py"),
                str(HERE / "controls" / "pulse_dead.json")],
        "fails_when": lambda rc, out: "[DEAD]" in out and rc == 1,
        "vacuous_looks_like": "[ALIVE] last stamp ...",
    },
    "build_ledger": {
        # B6, registered with its build (2026-08-16). Both directions on a
        # temp ledger: a recorded id must round-trip FOUND with identical
        # classification content, and a fabricated id must come back
        # UNKNOWN - never nearest-match, because a confidently wrong PDB
        # symbolicates to a confidently wrong function (the 08-12 lesson).
        "why": "The ledger is the join between a crash line and what the "
               "tree claimed at that build. If resolve can nearest-match, "
               "the join lies exactly when it matters most.",
        "cmd": [PY, str(HERE / "build_ledger.py"), "--selftest"],
        "fails_when": lambda rc, out: ("SELFTEST FIRED" in out
                                       and "FAIL" not in out),
        "vacuous_looks_like": "a fabricated build id resolving to a nearby "
                              "entry",
    },
    "row_doorway": {
        # ================================================================
        # B4, registered with its build (2026-08-16). The selftest poisons
        # symbols the metric ACTUALLY READS (the tier2-fixture lesson):
        # a nonexistent symbol declared EXISTS and a vanilla-present symbol
        # declared ABSENT must BOTH come back STALE, and a true EXISTS row
        # must hold - both directions, real manifests, no mocks.
        # ================================================================
        "why": "B4 is what keeps tracker rows TRUE, not merely present. If "
               "it cannot flag a doorway class that contradicts the images, "
               "every green row is a claim nobody is checking - the exact "
               "silent-class-flip the amendment's 5a names.",
        "cmd": [PY, str(HERE / "row_doorway.py"), "--selftest"],
        "fails_when": lambda rc, out: ("SELFTEST FIRED" in out
                                       and "2/2 required" in out),
        "vacuous_looks_like": "0 STALE on rows whose declarations are wrong",
    },
    "tier2_census": {
        # ================================================================
        # Registered 2026-08-16 with the A2 repair. Known-answer fixture,
        # every direction the classifier rules: B on single-line AND
        # WRAPPED TP hosts (wrapped hosts previously lost -> B filed as A),
        # skins API excluded BY USER RULING, the two vanilla-TP FP shapes
        # not counted, C on a table row with a CITABLE host name (the
        # greedy capture printed "table e" in real output), A on WW hosts.
        # Expectations parsed from the fixture's own EXPECT annotations -
        # the first version hardcoded line numbers and two "failures" were
        # the fixture drifting, not the classifier.
        # ================================================================
        "why": "tier2 feeds the ratchet rider and the migration worklist. "
               "The fixture holds one known answer per classification "
               "direction; any miss means that direction's counts are not "
               "citable.",
        "cmd": [PY, str(HERE / "controls" / "tier2_selftest.py")],
        "fails_when": lambda rc, out: ("ALL" in out and "FIRED" in out
                                       and "FAIL" not in out),
        "vacuous_looks_like": "expectations passing with FAIL rows present, "
                              "or an empty fixture parse",
    },
    "l2c_equiv": {
        # ================================================================
        # Declared 2026-08-15, and the finding is that IT ALREADY HAD A
        # CONTROL. `l2c_equiv.py --selftest` (its §695 #4 law) has always
        # exercised all four contracts; nobody put it in this registry, so
        # the audit reported it UNVERIFIABLE and every lane — me included —
        # repeated that it was uncontrolled. **The undeclared list measures
        # what is REGISTERED, not what is UNTESTED**, and I cited it as the
        # estate's exposure without opening the files. A narrower
        # measurement than the claim it supported, again.
        # This matters beyond bookkeeping: `l2c_equiv` is the tool the §839
        # user ruling leans on for "decomp vanilla byte true".
        # ================================================================
        "why": "`--selftest` builds fixtures for all four verdicts. The "
               "gate MUST report DIFFER on a byte-altered dump, NO-REF when "
               "nothing matches, and must refuse an empty dump dir. If it "
               "cannot produce DIFFER, every EQUIV it has ever printed is "
               "worthless — and that is the verdict the byte-trueness half "
               "of tale §839 rests on.",
        "cmd": [PY, str(HERE / "l2c_equiv.py"), "--selftest"],
        "fails_when": lambda rc, out: (
            "[DIFFER*]" in out and "[EQUIV* ]" in out
            and "[NO-REF ]" in out
            and "dump dir empty or absent" in out),
        "vacuous_looks_like": "all EQUIV, no DIFFER fixture ever produced",
    },
    "doc_index": {
        # ================================================================
        # Declared with the tool, 2026-08-16 (Librarian), because an audit
        # instrument that has never been shown to fire is exactly the thing
        # this registry exists to refuse. It earned the control twice over
        # during its own construction:
        #   1. v1 of the collision detector reported SEVENTEEN collisions,
        #      FIFTEEN of them port-kit TEMPLATE files. A permanent false
        #      positive destroys signal as thoroughly as a false negative.
        #   2. v1 of the handoff detector grouped by NAME SYMMETRY and could
        #      not see `ww-handoff-history-bridge.md` — which was the CURRENT
        #      History handoff. Caught by History/Bridge, not by me. It now
        #      groups by the LANE ROSTER: enumerate the side that cannot grow.
        # ================================================================
        "why": "`--selftest` rigs a fixture tree so every detector MUST fire "
               "— orphan, dead link, inverted-name pair, supersession pair, "
               "the larger-dead-file flag, and a lane handoff in state/ — "
               "then asserts a CLEAN tree stays silent. Without the negative "
               "half a detector that fires on everything would pass, and "
               "'0 findings' from this tool would mean nothing.",
        "cmd": [PY, str(HERE / "doc_index.py"), "--selftest"],
        "fails_when": lambda rc, out: (
            rc == 0 and "SELFTEST PASS" in out
            and "*** DID NOT FIRE ***" not in out
            and "GREEN (silent)" in out),
        "vacuous_looks_like": "0 findings on a tree nobody proved it can read",
    },
    "staging_verify": {
        # Registered 2026-08-15. It was on NO_CONTROL for its whole life while
        # carrying this harness - the third instance of that mistake tonight.
        "why": "`staging.py --selftest` feeds THREE deliberately bad rows - an "
               "illegal state, a depends-on that is not in the manifest, and a "
               "missing draft file - and the lint must reject all three. If it "
               "reports 0 errors it is not linting, and every clean staging "
               "verify is worthless.",
        "cmd": [PY, str(HERE / "staging.py"), "--selftest"],
        "fails_when": lambda rc, out: (
            "3 error(s)" in out and "illegal state" in out
            and "not in manifest" in out),
        "vacuous_looks_like": "staging lint: 3 row(s), 0 error(s)",
    },
    "decomp_status": {
        # Registered 2026-08-15, same discovery.
        # HONEST ABOUT WHAT THIS PROVES: it is the tool's OWN eight-assertion
        # harness over known inputs, including its negative cases - not a
        # control I independently drove red. Weaker evidence than the
        # compile_gate or sig_diff controls, and stronger than an empty
        # registry line claiming UNVERIFIABLE.
        "why": "`decomp_status.py --selftest` runs eight classification "
               "assertions over known TUs. All eight must hold; a False means "
               "the MATCHED/EQUIVALENT/NONMATCHING classifier has drifted and "
               "every status it reports is suspect.",
        "cmd": [PY, str(HERE / "decomp_status.py"), "--selftest"],
        "fails_when": lambda rc, out: (
            "selftest: OK" in out and "False" not in out),
        "vacuous_looks_like": "selftest reporting OK with a False in the list",
    },
    "kit_laws_recipes": {
        # ================================================================
        # The control existed as a SEPARATE FILE, outside this registry -
        # the third place a control can hide (registry / in-tool
        # `--selftest` / standalone `*_control.py`, `*_attest.py`). That
        # fragmentation is why `kit_laws` read as uncontrolled.
        # **COVERAGE IS PARTIAL AND MUST BE STATED: laws 6 and 7 of 10.**
        # Registering it does NOT make kit_laws a demonstrated gate; it
        # makes two of its ten laws demonstrated. Laws 1-5 and 8-10 stay
        # undeclared, and kit_laws stays on NO_CONTROL for that reason.
        # ================================================================
        "why": "`crash_recipe_attest.py` runs 11 fixtures - 6 positive, 5 "
               "negative - over kit_laws law 6 (pointer truncation) and law 7 "
               "(raw mDoHIO child). Both laws must FIRE on the defect and stay "
               "QUIET on the prescribed fix. A law that only ever fires, or "
               "only ever stays quiet, is measured in one direction only.",
        "cmd": [PY, str(HERE / "crash_recipe_attest.py")],
        "fails_when": lambda rc, out: (
            "ATTESTED" in out and "6 positive, 5 negative" in out
            and "[ fail" not in out),
        "vacuous_looks_like": "all fixtures ok with 0 negative fixtures run",
    },
    "watcher_census": {
        # A census that finds NOTHING looks exactly like a clean estate.
        # That is the most likely way this tool fails (PowerShell refuses,
        # the CSV shape changes) and the failure is silent, so the control
        # spawns a process that IS a watcher by the tool's own definition
        # and requires it to be found.
        "why": "`--selftest` spawns a live process carrying the watcher "
               "marker. The census MUST list it as a leaf. If it finds "
               "nothing, a clean estate and a broken enumerator are the "
               "same output.",
        "cmd": [PY, str(HERE / "watcher_census.py"), "--selftest"],
        "fails_when": lambda rc, out: "FOUND as a leaf" in out,
        "vacuous_looks_like": "0 matching process(es) — estate looks clean",
    },
    "monitor_pulse_stale_writer": {
        # ================================================================
        # Added 2026-08-15, an hour after `monitor_pulse` itself, because
        # the first control was not enough. A lane ran `lane_watch.py
        # --help`; with no help handling it ARMED, stamped the pulse, and
        # was killed by `timeout`. **The stamp was seconds old and the
        # writer was dead, so the liveness check said ALIVE while nothing
        # was watching** — a false ALIVE inside the tool built to make
        # silence falsifiable. Recency was an observation that could not
        # come out the other way.
        # The fixture must be GENERATED, not stored: a checked-in "fresh"
        # stamp goes stale and then reads DEAD for the wrong reason,
        # passing the control while proving nothing.
        # ================================================================
        "why": "A pulse stamped THIS INSTANT by a pid that cannot exist "
               "MUST read DEAD. If age alone carries the verdict, any "
               "process that stamps and exits leaves a permanent false "
               "ALIVE.",
        "cmd": [PY, str(HERE / "monitor_pulse.py"), "--selftest"],
        "fails_when": lambda rc, out: "[DEAD]" in out and "IS GONE" in out,
        "vacuous_looks_like": "[ALIVE] last stamp <seconds ago>",
    },
    "monitor_pulse_live_writer": {
        # The ALIVE direction. Both DEAD controls above would be satisfied
        # by a tool STUCK on DEAD, which cries wolf until nobody listens.
        "why": "A pulse stamped this instant by a LIVE pid (the checker's "
               "own process) MUST read ALIVE. If it reads DEAD, the pid "
               "check rejects live writers and every watcher looks dead.",
        "cmd": [PY, str(HERE / "monitor_pulse.py"), "--selftest-alive"],
        "fails_when": lambda rc, out: "[ALIVE]" in out and "IS GONE" not in out,
        "vacuous_looks_like": "[DEAD] (everything reads dead)",
    },
    "c0_audit": {
        # ================================================================
        # Added 2026-08-15. The hazard is SEVEN incidents old and had no
        # instrument — the seventh (a 0x08 that made sig_diff's call-site
        # filter inert) was found by hand, and only because a gate audit
        # disagreed with a handoff. §4 warned about it in prose for six
        # rounds. Prose did not catch it.
        # ================================================================
        "why": "controls/c0_injected.txt carries a deliberate BS, NUL, FF and "
               "a UTF-8 BOM. The sweep MUST report all four. If it calls that "
               "file clean it cannot see the byte class it exists to find, and "
               "every clean sweep it has produced is worthless. The BOM is in "
               "the fixture because it GOT PAST this tool once: a PowerShell "
               "`Set-Content -Encoding UTF8` broke lane_watch.py while the "
               "sweep called the same file clean in the same minute.",
        "cmd": [PY, str(HERE / "c0_audit.py"),
                str(HERE / "controls" / "c0_injected.txt")],
        "fails_when": lambda rc, out: (
            "0x08" in out and "0x00" in out and "0x0C" in out
            and "BOM" in out and "clean" not in out),
        "vacuous_looks_like": "clean — no invisible control bytes",
    },
    "symbol_safety": {
        "why": "`setRoomLayerOverride` is one of OUR fork's WW additions and "
               "MUST come back MISSING on vanilla upstream. If the checker "
               "says SAFE for a symbol vanilla does not have, every SAFE "
               "verdict is a blanket yes.",
        "cmd": [PY, str(HERE / "upstream_conformance.py"),
                "--symbol", "setRoomLayerOverride"],
        "fails_when": lambda rc, out: "MISSING" in out,
        "vacuous_looks_like": "vanilla upstream SAFE",
    },
    "file_row_lint": {
        "why": "A row addressed to a lane that does not exist MUST be "
               "rejected. If the lint accepts `MARKETING`, it is not "
               "enforcing the roster.",
        "cmd": [PY, str(HERE / "file_row.py"), "add",
                "--text", "- [ ] MARKETING | control row | x | 2026-01-01"],
        "fails_when": lambda rc, out: rc == 2 and "lint" in out.lower(),
        "vacuous_looks_like": "FILED OK",
    },
    "code_dialect": {
        "why": "K3 (dialect_codemod.py) scanned over a TU that CONTAINS the "
               "donor form of R5's CONTROL row — and of the DN-3 LAW row "
               "`(J3DModelData*)dComIfG_getObjectRes(...)`. report_manual() "
               "probes with `re.match` anchored at position 0, so every key "
               "starting with `(` yields token=None and is dropped at the bare "
               "`continue`. If the marker is absent from the report, K3's "
               "'(none present in this TU)' means nothing for that whole row "
               "class — including the row guarding the §810-2/§814 raw-cast "
               "crash. THIS ENTRY RECORDS NO STANDING VERDICT ON PURPOSE: I "
               "first wrote 'DECLARED BLIND' here, and it was false within the "
               "hour because Foundry repaired K3's probe while I was typing it "
               "(re.match -> re.search + a PARTIAL report line). A registry "
               "that carries a cached verdict is the vacuous-pass defect in "
               "the file built to catch it. Run `control.py check "
               "code_dialect` and read the live result; do not cite this "
               "string. — History/Bridge, 2026-08-15",
        "cmd": [PY, str(HERE / "dialect_codemod.py"), "--scan",
                str(HERE / "controls" / "dialect_unprobeable_key.cpp")],
        "fails_when": lambda rc, out: "WW_DIALECT_CONTROL_MUST_APPEAR" in out,
        "vacuous_looks_like": "NOT automated — porter's call (0): "
                              "(none present in this TU)",
    },
    "decomp_watch": {
        # ================================================================
        # Registered 2026-08-16 by the INTEGRATOR, on the user's order to
        # finish the join. The tool is new, so registering it in the same
        # change that built it - the A4 lesson was that a sound, selftested
        # gate sat OUTSIDE this audit and nothing would have noticed if it
        # stopped being able to fail.
        # ================================================================
        "why": "A pinned baseline where one TU is NONMATCHING and the live "
               "ledger has it Matching. That flip is the ONE event "
               "decomp_watch exists to catch - it is what discharges a "
               "reconstruction debt under the verbatim gate's addendum "
               "(our own decoding is admissible, traced, and REPLACED when "
               "the official decomp lands). If the tool reports no movement "
               "on that fixture, then 'owed and will be replaced' has "
               "nothing behind it and every clean run is worthless. The "
               "fixture builds its own baseline in a temp dir and never "
               "touches the live tracker.",
        "cmd": [PY, str(HERE / "controls" / "decomp_watch_flip.py")],
        "fails_when": lambda rc, out: "CONTROL OK" in out and "DETECTED" in out,
        "vacuous_looks_like": "decomp_watch PASS while a TU it tracks went "
                              "NONMATCHING -> Matching upstream",
    },
    "row_baseline": {
        # ================================================================
        # Registered 2026-08-16 by History/Bridge, on the Integrator's
        # catch: A4 was built, sound, selftested — and OUTSIDE the meta
        # gate. Their argument was my own line turned back on me ("a gate
        # that only ever passes is worthless"): nothing would ever have
        # noticed if row_baseline stopped being able to fail.
        # ================================================================
        "why": "A pinned store with one row DELETED and no retirement entry. "
               "row_baseline MUST report divergence — that deletion is the "
               "single thing it exists to block (SPEC-row-store §5, 'the "
               "deletion bypass dies there'). If it passes this fixture, the "
               "bypass is open and every green check it has ever printed is "
               "worthless. Fixture builds its own store in a temp dir and "
               "never touches the live tracker.",
        "cmd": [PY, str(HERE / "controls" / "row_baseline_deleted.py")],
        "fails_when": lambda rc, out: "CONTROL OK" in out and "DETECTED" in out,
        "vacuous_looks_like": "row_baseline PASS on a store missing a baselined row",
    },
    "boot_join": {
        # ================================================================
        # Registered 2026-08-16 by FOUNDRY, in the SAME CHANGE that built
        # the tool - SPEC-boot-join.md s6 demands it by name ("A4 was
        # sound, selftested, and outside the meta-gate, and nothing would
        # have noticed if it stopped being able to fail").
        # ================================================================
        "why": "The join feeds LINKED/EXERCISED from a boot log, and every "
               "failure in its class has been a FALSE CLEAN: a NOT-INSTALLED "
               "hook read as a measurement (setStageRes), an INSTALLED-but-"
               "never-attached hook read as alive (on_getRes). The selftest "
               "drives all of spec s6: NOT-INSTALLED must derive NOT and "
               "flag a REGISTERED row; attached+idle must derive REGISTERED-"
               "NOT-EXERCISED (never NOT, never EXERCISED); an ABSENT log "
               "must REFUSE, not pass; INSTALLED without an attach receipt "
               "must derive UNKNOWN, never REGISTERED; and every emitted "
               "value must carry src (INVENTORY-SCHEMA s1 - a value with "
               "no src is UNKNOWN).",
        "cmd": [PY, str(HERE / "boot_join.py"), "--selftest"],
        "fails_when": lambda rc, out: ("CONTROL OK" in out
                                       and "DETECTED" in out
                                       and "BLIND" not in out),
        "vacuous_looks_like": "boot_join reporting AGREE/clean on a log "
                              "whose hooks never installed or never attached",
    },
    "queue_verdict_lint": {
        # ================================================================
        # Registered 2026-08-16 by FOUNDRY, same change that built it, on
        # the THIRD specimen of the /Zs-cited-as-verification class
        # (Housing's own count, CALLS "Q2 ACTIONED"): the first case
        # receipt in this file's header, the registry.cpp latent link
        # error, and five open queue rows stamped "cl /Zs EXIT=0" by an
        # author who had written "/Zs proves syntax only" into memory the
        # same morning. Knowing a thing is not the same as having a rule;
        # a rule nobody checks mechanically is a note.
        # ================================================================
        "why": "An OPEN queue row stamped 'cl /Zs EXIT=0' with no "
               "UNVERIFIED marker reads as a compile verdict and is not "
               "one - /Zs never links, and four LNK2005s already reached "
               "the Integrator through exactly that green. The selftest "
               "rigs that row and requires the flag, requires the honest "
               "UNVERIFIED form to pass, and requires a missing queue to "
               "REFUSE rather than read as clean.",
        "cmd": [PY, str(HERE / "queue_verdict_lint.py"), "--selftest"],
        "fails_when": lambda rc, out: ("CONTROL OK" in out
                                       and "DETECTED" in out
                                       and "BLIND" not in out),
        "vacuous_looks_like": "queue clean while an open row carries "
                              "'/Zs EXIT=0' dressed as verification",
    },
    "ppc_sha1_gate": {
        # ================================================================
        # Registered 2026-08-17 by FOUNDRY the day the Decoder lane's
        # first SHA1-clean REL landed (charter commitment: "I register
        # the perturbed-byte control the day their hash lands"). The (A)
        # gate is the lane's SOLE grading by user ruling.
        # ================================================================
        "why": "BYTE-TRUE via donor-toolchain REL identity is only a gate "
               "if it can go red. The fixture re-derives the pristine "
               "hash against build.sha1 (never trusts a row) AND flips "
               "one byte on a scratch copy, which MUST break the hash - "
               "two artifacts hashing identically after a byte flip would "
               "mean the comparison is not reading bytes. Absent "
               "REL/toolchain -> UNKNOWN, refused, never clean.",
        "cmd": [PY, str(HERE / "controls" / "ppc_sha1_gate.py")],
        "fails_when": lambda rc, out: ("CONTROL OK" in out
                                       and "DETECTED" in out
                                       and "BLIND" not in out),
        "vacuous_looks_like": "a BYTE-TRUE stamp from a hash comparison "
                              "that never read the artifact's bytes",
    },
    "bytetrue_recheck": {
        # ================================================================
        # Registered 2026-08-17 by FOUNDRY, same change that built it.
        # History/Bridge re-ran their own msdan2 certification BY HAND
        # "because the tree had moved under it" and named the principle —
        # a certification is only as good as its last re-measure. Named
        # in prose, absent in mechanism; this is the mechanism.
        # ================================================================
        "why": "A `byte-true: BYTE-TRUE` row is a claim about an artifact "
               "AT A MOMENT, and a shared-header edit can un-match a "
               "previously-matching TU project-wide (why upstream ships "
               "`ninja changes`). Our rows are a SEPARATE claim surface "
               "nothing re-checks, so a row can become a lie with nobody "
               "editing it. The selftest poisons a claim's hash against a "
               "REAL artifact and requires DRIFT, and requires a BYTE-TRUE "
               "row with no artifact reference to be flagged rather than "
               "skipped.",
        "cmd": [PY, str(HERE / "bytetrue_recheck.py"), "--selftest"],
        "fails_when": lambda rc, out: ("CONTROL OK" in out
                                       and "DETECTED" in out
                                       and "BLIND" not in out),
        "vacuous_looks_like": "a BYTE-TRUE row still reading clean after "
                              "the artifact it names has changed",
    },
    "file_row_verify": {
        "why": "Verifying a string that is not in CALLS.md MUST report NOT "
               "FOUND. If it reports FOUND, the filer's own verification is "
               "the `echo \"filed\"` failure it was built to replace.",
        "cmd": [PY, str(HERE / "file_row.py"), "verify",
                "--match", "ZZZ-NO-SUCH-ROW-CONTROL-ZZZ"],
        "fails_when": lambda rc, out: "NOT FOUND" in out,
        "vacuous_looks_like": "FOUND n row(s)",
    },
}

# Gates that exist and have NO declared control. Naming them is the point:
# this is the estate's real exposure, and it was invisible until tonight.
# ---------------------------------------------------------------------------
# **THIS LIST MEASURES WHAT IS REGISTERED, NOT WHAT IS UNTESTED.** `l2c_equiv`
# sat here for its whole life while `l2c_equiv.py --selftest` — its own §695 #4
# law, exercising EQUIV / DIFFER / NO-REF / empty-dir — had been in the file
# all along. Nobody had opened it, and every lane (me loudest) cited the list
# as the estate's exposure. **Before treating a name below as uncontrolled,
# grep the tool for `--selftest` or a fixture harness; the control may already
# exist and want only a registry entry.**
NO_CONTROL = [
    "routing_check.py", "call_receipt.py",
    # PARTIAL, and the qualifier is the point: laws 6-7 ARE demonstrated by
    # `crash_recipe_attest` (registry entry `kit_laws_recipes`). The other
    # eight are not. Pulling it off this list entirely would trade one
    # overstatement for its mirror image.
    "kit_laws.py (laws 6-7 demonstrated; 1-5, 8-10 undeclared)",
    "ordinal_differ.py", "port_deps.py",
    # fold_detect's control EXISTS as fold_control.py and does NOT pass:
    # it reports "DETECTOR: ABSENT - nothing in src/dusk/mods or tools
    # implements address-collision detection" (exit 2), while
    # tools/foundry/fold_detect.py is on the workflow map. Either the
    # harness looks in the wrong place or it wants a PLUGIN-side detector
    # that does not exist. **Unresolved, so it is listed, not registered.**
    "fold_detect.py (fold_control.py reports DETECTOR ABSENT - unresolved)",
]


def has_selftest(name):
    """Does this tool carry its own falsifiability harness?

    ========================================================================
    THE LIST ABOVE IS HAND-WRITTEN AND HAS BEEN WRONG THREE TIMES IN ONE
    EVENING. `l2c_equiv` sat on it while carrying a four-way `--selftest`;
    then `decomp_status.py --selftest` and `staging.py def selftest()` turned
    out to be real too. **I cited that list to other lanes as the estate's
    exposure on all three occasions.**
    It is a claim about controls that was never itself checked - which is
    this file's own thesis, broken inside this file, for the third time.
    So the audit now READS THE FILES instead of trusting the list.
    ========================================================================
    """
    path = HERE / name.split()[0]
    if not path.is_file():
        return None
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    if path.name == "control.py":
        return False        # it only MENTIONS selftests; it is the registry
    # bool() IS LOAD-BEARING. This read `... or re.search(...)`, which returns
    # None - not False - when neither matches, and the caller printed None as
    # "file not found". **Five tools that exist were reported missing**, in the
    # patch written to stop this file mislabelling things. A tri-state where
    # one value quietly means two different conditions, printed as a verdict.
    return "--selftest" in text or bool(re.search(r"^def selftest", text, re.M))


def run(cmd, timeout=420):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True,
                           encoding="utf-8", errors="replace", timeout=timeout,
                           cwd=str(REPO))
        return r.returncode, (r.stdout or "") + (r.stderr or "")
    except Exception as e:
        return -1, "CONTROL COULD NOT RUN: %s" % e


def check(name):
    spec = CONTROLS[name]
    rc, out = run(spec["cmd"])
    red = bool(spec["fails_when"](rc, out))
    return red, rc, out, spec


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "audit"

    if cmd == "check" and len(sys.argv) > 2:
        name = sys.argv[2]
        if name not in CONTROLS:
            print("no control declared for %r. Declared: %s"
                  % (name, ", ".join(sorted(CONTROLS))))
            return 2
        red, rc, out, spec = check(name)
        print("CONTROL %s — %s" % (name, "WENT RED (gate is live)" if red
                                  else "**DID NOT FAIL — GATE IS BLIND**"))
        print("  why: %s" % spec["why"])
        print("  exit %d" % rc)
        for ln in out.splitlines()[:14]:
            print("    %s" % ln[:150])
        if not red:
            print("  **A pass from this gate proves nothing. Do not cite it.**")
        return 0 if red else 1

    if cmd == "guard":
        # control.py guard <gate> -- <command...>
        if "--" not in sys.argv or len(sys.argv) < 3:
            print("usage: control.py guard <gate> -- <command...>")
            return 2
        name = sys.argv[2]
        real = sys.argv[sys.argv.index("--") + 1:]
        if name not in CONTROLS:
            print("*** REFUSED *** no negative control is declared for %r." % name)
            print("    An undemonstrated gate cannot certify anything. Declare a")
            print("    control in control.py's registry first.")
            return 1
        red, rc, out, spec = check(name)
        if not red:
            print("*** REFUSED — THE GATE IS BLIND ***")
            print("  Its negative control did NOT fail, so it cannot distinguish")
            print("  a pass from a no-op. **I have not run your command**, because")
            print("  its result would be indistinguishable from a vacuous one.")
            print("  control: %s" % spec["why"])
            return 1
        print("control %s WENT RED — gate is live. Running the real check.\n" % name)
        rc2, out2 = run(real)
        print(out2[:4000])
        return rc2

    # audit
    print("CONTROL AUDIT — can each gate be made to report failure?")
    print("A gate that has never gone red cannot certify anything.\n")
    blind = 0
    for name in sorted(CONTROLS):
        red, rc, out, spec = check(name)
        mark = "RED " if red else "BLIND"
        print("  [%s] %-16s %s" % (mark, name,
                                   "control fired as required" if red
                                   else "**CONTROL DID NOT FAIL**"))
        if not red:
            blind += 1
            print("           looks like: %r" % spec["vacuous_looks_like"])
            print("           every pass from this gate is worthless until fixed")
    print()
    print("  NO CONTROL DECLARED (%d) — UNVERIFIABLE, not clean (№31-C):"
          % len(NO_CONTROL))
    latent = []
    for n in NO_CONTROL:
        st = has_selftest(n)
        mark = ""
        if st is True:
            mark = "   <- HAS ITS OWN --selftest ALREADY; needs a REGISTRY " \
                   "ENTRY, not a control"
            latent.append(n)
        elif st is None:
            mark = "   <- file not found under tools/foundry"
        print("      %-22s%s" % (n, mark))
    if latent:
        print()
        print("  **%d of the %d above are MISLABELLED.** They carry a "
              "falsifiability" % (len(latent), len(NO_CONTROL)))
        print("  harness today and are being reported as unverifiable only "
              "because")
        print("  nobody registered them. Declare them before citing this list "
              "as exposure.")
    print()
    print("  %d gate(s) demonstrated live · %d BLIND · %d undeclared"
          % (len(CONTROLS) - blind, blind, len(NO_CONTROL)))
    print("  **An undeclared gate is not a passing gate. It is an unknown one.**")
    return 1 if blind else 0


if __name__ == "__main__":
    sys.exit(main())
