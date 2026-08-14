#!/usr/bin/env python3
# ============================================================================
# workflow.py — THE MAP. One command that tells any instance how work flows
# here and which instrument answers which question.
#
# WHY IT EXISTS AS A TOOL AND NOT A DOC: the preflight banner listed the gate
# set by hand and went stale within a day of six tools landing — a map that
# stopped matching the territory, which is the exact rot the inventory design
# is meant to prevent. So this map SELF-CHECKS: every tool it names must
# exist, and every tools/foundry/*.py it does NOT name is counted as
# UNLISTED. A map cannot silently point at a dead tool, and it cannot
# silently hide a live one (№31-C, applied to documentation).
#
# Usage: workflow.py            the map
#        workflow.py --check    verify only (exit 1 if a named tool is gone)
# ============================================================================
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]

# PHASES — the order work actually moves in. Each row: tool, the question it
# answers. Tools are named ONCE, in the phase where a lane first needs them.
PHASES = [
    ("0. THE RULES (read before touching a guarded surface)", [
        ("dn_registry.py", "which DO-NOT guards this file/symbol? "
                           "(`touch <path|symbol>`; charter text stays in docs/DO-NOT.md)"),
    ]),
    ("1. PICK — what should be ported next", [
        ("port_planner.py", "ranked queue: class x readiness x demand, Outset-first "
                            "-> PORT-QUEUE.md (+ stage layout classes)"),
        ("backlog.py", "what is BLOCKED and by which system; promotion rule fires "
                       "when a system goes green -> BACKLOG.md"),
    ]),
    ("2. CHECK BEFORE PORTING — one command runs the battery", [
        ("port_preflight.py", "THE CARD a queue row must carry: readiness + DN-3 cast "
                              "worklist + name-gap triage + deps gate + demand "
                              "(+ --post: laws, substitution nets, compile, solidity)"),
        ("decomp_status.py", "donor TU MATCHED/EQUIVALENT/NONMATCHING (paren-aware; "
                             "compound expressions included)"),
        ("port_deps.py", "systems registry + services boundary + --solidity "
                         "+ --unlocks (who rides a system's wave) + --adjudicate"),
    ]),
    ("3. BUILD THE PORT — the mechanical parts", [
        ("port_kit.py", "K1 res-header generation off the disc (golden-gated) · "
                        "K2 registration scaffold (the section-747 pattern)"),
        ("space_kit.py", "donor disc rooms/placements/warp rows"),
        ("ww_disc.py", "the donor disc reader itself (FST, RARC, Yaz0)"),
    ]),
    ("4. GATES — nothing queues until these pass", [
        ("kit_laws.py", "laws 1-10 incl. DN-3 raw-buffer cast + DN-10-S substitution"),
        ("compile_gate.py", "does the RECEIVER have the API this code calls, in this "
                            "shape? (tier 1 existence, tier 2 real cl /Zs)"),
        ("ordinal_differ.py", "L4: shared ordinal tables (A ordinal / B name-lookup / "
                              "C invisible — class C is stated on every run)"),
        ("kit_output_law.py", "no kit may write a donor-format file that is not "
                              "disc-verbatim"),
    ]),
    ("5. VERIFY + MEASURE", [
        ("l2c_equiv.py", "runtime-served bytes vs the disc, member by member"),
        ("coverage_join.py", "EXERCISED vs CEILING (placement-weighted) — is the "
                             "bottleneck us or upstream?"),
        ("room_layout.py", "how a stage's rooms relate (stacked/nested/identical-box)"),
        ("scls_census.py", "which stages are reachable but undeclared (the Fairy04 class)"),
    ]),
    ("6. COMMUNICATE — the surfaces, not the prose", [
        ("staging.py", "`alloc <bus> <lane>` BEFORE posting a section; `verify` finds "
                       "unallocated/collided sections"),
        ("calls_rotate.py", "keeps CALLS.md the size of the work (answered rows -> "
                            "CALLS-ARCHIVE.md, never deleted)"),
    ]),
]

RULES = [
    "CALLS.md is the ONLY call surface. A row triggers a lane; a WHOSE-TURN "
    "mention is narrative. Answer by checking the box and citing where.",
    "The INTEGRATOR is the only builder. Lanes queue to BUILD-QUEUE.md with "
    "donor evidence; min-batch-5 port rows per build; rows arrive carded.",
    "DN-10 binds every task: read the donor's own system FIRST. A write-up "
    "citing only receiver files has not cleared step 1.",
    "UNKNOWN is never PASS (№31-C). Absent evidence is reported, not assumed.",
    "Allocate section numbers before posting; cite as '<bus> §N', never bare.",
]


def main():
    check = "--check" in sys.argv
    named = set()
    missing = []
    for _phase, rows in PHASES:
        for tool, _why in rows:
            named.add(tool)
            if not (HERE / tool).is_file():
                missing.append(tool)
    present = {p.name for p in HERE.glob("*.py")}
    unlisted = sorted(present - named - {"workflow.py"})

    if not check:
        print("=" * 74)
        print("DUSKLIGHT WW-PORT WORKFLOW — tools/foundry/ is the ONE instruments home")
        print("=" * 74)
        for phase, rows in PHASES:
            print("\n%s" % phase)
            for tool, why in rows:
                mark = " " if (HERE / tool).is_file() else "!"
                print("  %s %-20s %s" % (mark, tool, why))
        print("\nSTANDING RULES")
        for r in RULES:
            print("  · %s" % r)
        print("\nSURFACES")
        print("  · docs/state/ww-staging/CALLS.md         calls (your trigger)")
        print("  · docs/state/ww-staging/BUILD-QUEUE.md   queue to the Integrator")
        print("  · docs/state/ww-staging/PORT-QUEUE.md    what to port next")
        print("  · docs/state/ww-staging/BACKLOG.md       blocked x blocking system")
        print("  · docs/DO-NOT.md                         the hard stops")
        print("  · the two buses                          narrative + rulings")
    if missing:
        print("\n[STALE MAP] named but MISSING: %s" % ", ".join(missing))
    print("\nmap: %d tool(s) named across %d phase(s) · %d unlisted specialist "
          "tool(s) in tools/foundry (not workflow steps)"
          % (len(named), len(PHASES), len(unlisted)))
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
