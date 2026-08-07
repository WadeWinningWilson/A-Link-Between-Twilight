#!/usr/bin/env python3
# ============================================================================
# leg_debt.py — census step 8, ARTIFACT 3: the leg-migration debt list.
#
# Spec §7: "the leg-migration debt = every WW branch inside a receiver-owned TU,
# each with the hook it would need to become. This is the Engine work item list."
#
# ---------------------------------------------------------------------------
# THE LEG MARKER ALREADY EXISTS — THIS DOES NOT INVENT ONE
#
# I was about to add a `KIT-LEG` marker. Engine's F3 `KIT-DONOR-HUNK`, landed in
# E3, already marks exactly this: donor-derived lines inside an otherwise
# receiver-owned file. That IS a leg. A second marker would be a second roster
# that drifts from the first — census spec §1's own failure mode, and the reason
# the banner spec extended §426 instead of inventing a format.
#
# So this reads what Engine declared and derives the debt from it.
#
# ---------------------------------------------------------------------------
# WHAT THIS MEASURES, AND WHAT IT LEAVES TO ENGINE
#
# Foundry measures the SITE; Engine designs the HOOK. For each leg this reports
# the file, the line span, the donor object and status, and the ENCLOSING
# RECEIVER FUNCTION — which is where a hook call-out would have to go. It does
# not name or shape the hook: that is an interface design decision, and inventing
# a name here would put Foundry's guess into Engine's work item.
#
# ---------------------------------------------------------------------------
# UNMEASURED DEBT IS REPORTED, NOT OMITTED
#
# A file with donor evidence, a `mixed`/`host-plumbing` lineage and NO hunk
# markers has debt this tool cannot see. Reporting only the declared hunks would
# make an unbannered file look debt-free — silence read as a measurement, which
# is the failure this campaign has hit at every axis. Those files are listed
# under UNMEASURED with a count of zero and an explicit UNKNOWN.
#
# Read-only. Usage: leg_debt.py [--md]
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import banner_lint as BL  # noqa: E402
import census_axis_c as C  # noqa: E402
import census_axis_d as D  # noqa: E402
import ww_census as WC  # noqa: E402

HUNK_END = re.compile(r"^\s*//\s*KIT-DONOR-HUNK-END\s*$")
HUNK = re.compile(r"^\s*//\s*KIT-DONOR-HUNK:\s*(\S+)(?:\s+(\S+))?\s*$")


SIG_START = re.compile(r"^[ \t]*[A-Za-z_][\w:*&<>,~\s]*?([A-Za-z_]\w{2,})\s*\(")


def _scope_map(lines):
    """line index -> (enclosing function name, its line) or (None, None).

    Forward parse. D-2: a definition's `{` may sit on a LATER line than its name
    -- `dDemo_setDemoData(...)` wraps its parameter list and opens on :458 while
    the name is on :457. Requiring both on one line dropped that frame silently
    and reported everything inside it as file scope. A candidate signature is
    therefore carried forward until a `{` opens it or a `;` proves it a mere
    declaration."""
    out = {}
    stack = []          # (name, line_no, depth_at_open)
    depth = 0
    pending = None      # (name, line_no) awaiting its opening brace
    for i, line in enumerate(lines):
        code = line.split("//")[0]
        out[i] = (stack[-1][0], stack[-1][1]) if stack else (None, None)

        if pending is None:
            m = SIG_START.match(code)
            if m and not code.lstrip().startswith(("#", "return", "if", "for",
                                                   "while", "switch")):
                pending = (m.group(1), i + 1)

        if pending is not None:
            if "{" in code:
                stack.append((pending[0], pending[1], depth))
                out[i] = (pending[0], pending[1])
                pending = None
            elif ";" in code:
                pending = None      # a declaration, not a definition

        depth += code.count("{") - code.count("}")
        while stack and depth <= stack[-1][2]:
            stack.pop()
    return out


def collect():
    srcs = WC.load_build_sources() or []
    rows = [WC.classify_tu(s) for s in srcs]
    roster = WC.roster_union(rows)    # HT-27: one predicate, not a copy
    status_map = D.donor_status() or {}

    legs, unmeasured, checked_clean = [], [], []
    for rel in roster:
        b = BL.read_banner(rel) or {}
        lineage = (b.get("KIT-LINEAGE") or [None])[0]
        donors = b.get("KIT-DONOR", [])
        p = C.REPO / rel
        if not p.is_file():
            continue
        lines = p.read_text(encoding="utf-8-sig", errors="replace").splitlines()

        found = []
        scopes = _scope_map(lines)
        open_at = None
        for i, line in enumerate(lines):
            m = HUNK.match(line)
            if m:
                open_at = (i, m.group(1), m.group(2))
                continue
            if HUNK_END.match(line) and open_at is not None:
                start, donor, status = open_at
                fn, fn_line = scopes.get(start, (None, None))
                found.append({
                    "tu": rel, "donor": donor,
                    "declared_status": status,
                    "donor_status": status_map.get(donor, "UNKNOWN"),
                    "start": start + 1, "end": i + 1,
                    "span_lines": i - start - 1,
                    "enclosing_fn": fn,
                    "enclosing_fn_line": fn_line,
                    "scope": "function" if fn else "file-scope",
                    "lineage": lineage,
                })
                open_at = None
        if found:
            legs += found
        elif "per-hunk" in donors:
            # D-3: claims per-hunk provenance but declares no hunk. This IS
            # unmeasured debt -- the file says "look at the markers" and there
            # are none.
            unmeasured.append((rel, lineage,
                               "declares KIT-DONOR: per-hunk but carries no "
                               "hunk marker — debt UNKNOWN, not zero"))
        elif "none" in donors:
            # D-3: a CHECKED claim, not an unknown. HousingTemp inspected these
            # line by line at E2 and found no donor content. Filing a checked
            # claim as unknown debt inverts the point of having looked.
            checked_clean.append(rel)
        elif lineage in ("mixed", "host-plumbing"):
            unmeasured.append((rel, lineage,
                               "mixed/host-plumbing lineage with neither hunk "
                               "markers nor a KIT-DONOR claim — UNKNOWN"))
    return legs, unmeasured, checked_clean, roster


def main():
    legs, unmeasured, checked_clean, roster = collect()
    print("STEP 8 ARTIFACT 3 — LEG-MIGRATION DEBT (Engine work-item list)")
    print(f"source: KIT-DONOR-HUNK markers declared by Engine (E3), "
          f"{len(roster)} roster TUs\n")

    by_tu = {}
    for l in legs:
        by_tu.setdefault(l["tu"], []).append(l)

    print(f"{'TU':38s} {'legs':>4} {'lines':>6}  donor objects")
    print("-" * 92)
    for tu, ls in sorted(by_tu.items()):
        donors = sorted({l["donor"] for l in ls})
        print(f"{tu[:38]:38s} {len(ls):4d} {sum(l['span_lines'] for l in ls):6d}"
              f"  {', '.join(donors)}")
    in_fn = sum(1 for l in legs if l["enclosing_fn"])
    print(f"\nTOTAL: {len(legs)} legs across {len(by_tu)} TUs, "
          f"{sum(l['span_lines'] for l in legs)} donor-derived lines")
    print(f"SCOPE SPLIT: {len(legs) - in_fn} file-scope / {in_fn} in-function"
          f"   <- cite THIS, never a count made alongside it (D-1)\n")

    print("PER-LEG — site, span, and the receiver function a hook would sit in")
    print("(Foundry measures the site; Engine designs the hook.)\n")
    for tu, ls in sorted(by_tu.items()):
        print(f"  {tu}")
        for l in ls:
            where = (f"in {l['enclosing_fn']}() @:{l['enclosing_fn_line']}"
                     if l["enclosing_fn"] else "at FILE SCOPE (table/initialiser "
                                               "— a different hook shape)")
            print(f"     :{l['start']}-{l['end']} ({l['span_lines']} lines) "
                  f"<- {l['donor']} [{l['donor_status']}]")
            print(f"        {where}")

    if unmeasured:
        print(f"\nUNMEASURED — debt this tool cannot see ({len(unmeasured)}):")
        for rel, lin, why in unmeasured:
            print(f"  {rel} [{lin}] — {why}")
    else:
        print("\nUNMEASURED: none.")
    if checked_clean:
        print(f"\nCHECKED CLEAN — declared `KIT-DONOR: none` after inspection "
              f"({len(checked_clean)}). Not unmeasured; looked at and found to "
              f"carry no donor content:")
        for rel in checked_clean[:12]:
            print(f"  {rel}")
        if len(checked_clean) > 12:
            print(f"  … {len(checked_clean) - 12} more")

    print("\nLIMITS. This counts DECLARED legs only. A donor-derived line nobody "
          "marked is invisible here, exactly as an unbannered TU is invisible to "
          "Axis D — so this is a FLOOR on the debt, never a total.")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
