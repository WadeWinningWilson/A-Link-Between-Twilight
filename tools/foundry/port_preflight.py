#!/usr/bin/env python3
# ============================================================================
# port_preflight.py — ONE command, the whole instrument battery, one card.
#
# WHY (user audit 2026-08-13): the tools existed and were underused — l2c_equiv
# unrun until tale §839, DN-3 defects landing that law 9 catches per-file,
# npc_p1's MATCHED status nearly missed. "Utmost use" cannot require
# remembering six commands. This wrapper IS the remembering.
#
#   port_preflight.py <donor actor>          before porting (History)
#   port_preflight.py <donor actor> --post <receiver.cpp>   after porting
#
# BEFORE a port it answers: is the donor MATCHED (readiness)? which get*Res
# casts must translate (DN-3 worklist)? which donor deps have no receiver
# name (name-gap triage)? what does the actor place on Outset (demand)?
# AFTER a port it adds: per-file kit_laws verdicts + the --pair substitution
# nets against the donor TU.
#
# The output is the CARD a lane attaches to its BUILD-QUEUE row — the §839
# gate re-derives regardless, but a row that arrives with its card needs one
# look, not six.  Read-only. Exit 0 = clean, 1 = findings, 2 = could-not-run
# (№31-C: UNKNOWN is reported, never silent).
# ============================================================================
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
PY = sys.executable


def run(args):
    r = subprocess.run([PY] + args, capture_output=True, text=True,
                       encoding="utf-8", errors="replace", cwd=str(HERE.parents[1]))
    return (r.stdout or "") + (r.stderr or "")


def main():
    argv = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not argv:
        print("usage: port_preflight.py <donor actor e.g. d_a_obj_paper> "
              "[--post <receiver.cpp>]")
        return 2
    actor = argv[0]
    post = None
    if "--post" in sys.argv:
        i = sys.argv.index("--post")
        if i + 1 < len(sys.argv):
            post = sys.argv[i + 1]

    findings = 0
    unknowns = 0
    print("=" * 72)
    print("PORT PREFLIGHT — %s%s" % (actor, "  (post-port vs %s)" % post if post else ""))
    print("TOOLS HOME: tools/foundry/ — the ONE instruments home (user order, tale §850).")
    print("CALLS: lane-to-lane calls go to docs/state/ww-staging/CALLS.md — ONE row per")
    print("  call, target lane checks the box. A WHOSE-TURN mention is narrative; the")
    print("  CALLS row is what triggers a lane (user order, tale §877).")
    # The hand-written gate list went stale within a day of six tools landing.
    # The map is GENERATED and self-checking now — point at it, do not restate.
    print("  THE MAP (phases, tools, standing rules): python tools/foundry/workflow.py")
    print("=" * 72)

    # 1. READINESS — decomp_status
    out = run([str(HERE / "decomp_status.py"), actor])
    m = re.search(r"(MATCHED|NONMATCHING|NO-TU)", out)
    print("\n[1 READINESS] decomp_status:")
    print("  " + (m.group(1) if m else out.strip().splitlines()[-1] if out.strip() else "UNKNOWN"))
    if m and m.group(1) != "MATCHED":
        findings += 1
        print("  -> not MATCHED: verbatim port impossible; say so in the queue row"
              " (§801 verdict class)")
    if not m:
        unknowns += 1

    # 2. DN-3 WORKLIST — donor-precast
    out = run([str(HERE / "kit_laws.py"), "--donor-precast", actor])
    print("\n[2 DN-3 CAST WORKLIST] kit_laws --donor-precast:")
    body = [ln for ln in out.splitlines() if ln.strip()]
    for ln in body[:6]:
        print("  " + ln)
    if "TRANSLATE" in out and " 0 TRANSLATE" not in out:
        findings += 1

    # 3. NAME-GAP TRIAGE — donor-deps
    out = run([str(HERE / "kit_laws.py"), "--donor-deps", actor])
    print("\n[3 NAME-GAP TRIAGE] kit_laws --donor-deps (§828: evidence, never verdicts):")
    for ln in [l for l in out.splitlines() if l.strip()][:14]:
        print("  " + ln)
    if "no-candidate" in out:
        findings += 1
        print("  -> no-candidate rows are the real port questions; resolve before queueing")

    # 3b. SYSTEM-DEPENDENCY GATE — port_deps (the palm lesson: symbol-present
    # but system-unarmed is invisible to name matching; systems are DECLARED)
    print("\n[3b DEPS GATE] port_deps (registry-status per touched system):")
    out = run([str(HERE / "port_deps.py"), actor])
    for ln in [l for l in out.splitlines() if l.strip()]:
        print("  " + ln)
    if "[HOLD]" in out or "[VERIFY]" in out:
        findings += 1

    # 4. DEMAND — PORT-QUEUE row if present
    pq = HERE.parents[1] / "docs" / "state" / "ww-staging" / "PORT-QUEUE.md"
    print("\n[4 DEMAND] PORT-QUEUE row:")
    row = None
    if pq.is_file():
        for ln in pq.read_text(encoding="utf-8", errors="replace").splitlines():
            if ("| %s |" % actor) in ln:
                row = ln
                break
    print("  " + (row if row else "(not placed on the active area — check census for demand)"))

    # 5/6. POST-PORT — per-file laws + substitution nets
    if post:
        out = run([str(HERE / "kit_laws.py"), post])
        print("\n[5 KIT LAWS] per-file verdicts (%s):" % post)
        for ln in out.splitlines():
            if "VIOLATION" in ln or "UNKNOWN" in ln or "OWED" in ln:
                print("  " + ln.strip())
                findings += "VIOLATION" in ln
                unknowns += "UNKNOWN" in ln
        out = run([str(HERE / "kit_laws.py"), "--pair", post])
        print("\n[6 SUBSTITUTION NETS] kit_laws --pair vs donor TU:")
        for ln in [l for l in out.splitlines() if l.strip()][:10]:
            print("  " + ln)
        if "SUSPECT" in out:
            findings += 1

        # 7. COMPILE GATE (CALLS row, tale §879): six actors passed every prior
        # gate and five failed to compile — receiver-API existence + shape.
        out = run([str(HERE / "compile_gate.py"), post, "--compile"])
        print("\n[7 COMPILE GATE] receiver-API existence + cl /Zs shape:")
        for ln in [l for l in out.splitlines() if l.strip()]:
            print("  " + ln)
        if "FINDINGS" in out or "UNRESOLVED" in out:
            findings += 1

        # 8. SOLIDITY (CALLS row: walk-through trees): donor collision sources
        # by REGISTRATION FAMILY (dBgW MoveBG / dBgS / cc-push / arc-dzb) vs
        # the receiver TU. Source-present != runtime-solid — the runtime half
        # (repack seam routing) is the porting lane's check.
        out = run([str(HERE / "port_deps.py"), "--solidity", actor, post])
        print("\n[8 SOLIDITY] donor collision sources vs port:")
        for ln in [l for l in out.splitlines() if l.strip()]:
            print("  " + ln)
        if "[HOLD]" in out or "[VERIFY]" in out:
            findings += 1

    print("\n" + "=" * 72)
    verdict = ("CLEAN" if findings == 0 and unknowns == 0
               else "FINDINGS: %d (unknown: %d) — attach this card to the queue row"
               % (findings, unknowns))
    print("PREFLIGHT %s" % verdict)
    return 0 if findings == 0 and unknowns == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
