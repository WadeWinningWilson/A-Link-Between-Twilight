#!/usr/bin/env python3
# ============================================================================
# tool_index.py - EVERY INSTRUMENT IN tools/foundry, ONE LINE EACH.
#
# ---------------------------------------------------------------------------
# WHY. `workflow.py` is the MAP - the 7 phases a port walks through - and it
# deliberately names only the ~27 tools that are workflow STEPS. It reports the
# rest as "unlisted specialist tools" and there are SEVENTY-FIVE of them.
#
# **A lane cannot use what it cannot find.** Today that cost real work twice:
# `l2c_equiv.py` carried its own four-way `--selftest` for its whole life while
# every lane, me loudest, called it uncontrolled and cited it as the estate's
# exposure; and lanes have been asked to run tools they had no index for.
#
# So this generates the index FROM THE FILES, never by hand. A hand-written
# list of 102 tools is wrong the day after it is written - the same defect as
# a bare gate count in a durable doc.
#
# For each tool it reports the purpose line, whether it declares a negative
# control in `control.py`, and whether it carries its own `--selftest`. **That
# third column is the one to read before writing a new control: the control may
# already exist and want only a registry entry.**
#
# Usage:  tool_index.py            print the index
#         tool_index.py --write    regenerate tools/foundry/TOOL-INDEX.md
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUT = HERE / "TOOL-INDEX.md"

# a purpose line: `# name.py - what it does` (em dash or hyphen)
RE_PURPOSE = re.compile(r"^#\s*([\w.]+\.py)\s*[-—:]\s*(.+?)\s*$")
RE_USAGE = re.compile(r"^#?\s*[Uu]sage:\s*(.+?)\s*$")


def workflow_named():
    """Tool names workflow.py lists as steps."""
    txt = (HERE / "workflow.py").read_text(encoding="utf-8", errors="replace")
    return set(re.findall(r'\("([\w.]+\.py)"', txt))


def controlled():
    """Tools with a declared negative control in control.py's registry.

    TWO SOURCES, because one was blind (Integrator 2026-08-16):
      (a) the script path a control invokes  — `HERE / "<tool>.py"`
      (b) the REGISTRY KEY itself            — `"<tool>": {`

    (a) alone credited only controls that run the tool DIRECTLY. Seven controls
    run a FIXTURE out of `controls/` instead — `row_baseline_deleted.py`,
    `tier2_selftest.py`, `decomp_watch_flip.py`, and four data fixtures — so the
    path match credited the fixture and the TOOL read as UNCONTROLLED. That is
    the exact `l2c_equiv` failure this index was written after: a tool carrying
    a working control while every reader called it uncontrolled — reproduced
    here, in the tool that reports controlledness.

    The registry KEY is the tool's own name by convention, so (b) catches the
    fixture-based ones. Keys with no matching file (e.g. the pulse writers) just
    never intersect the tool list and cost nothing.
    """
    txt = (HERE / "control.py").read_text(encoding="utf-8", errors="replace")
    by_path = set(re.findall(r'HERE / "([\w.]+\.py)"', txt))
    by_key = {k + ".py" for k in re.findall(r'^\s{4}"(\w+)":\s*\{', txt, re.M)}
    return by_path | by_key


def describe(path):
    purpose, usage, selftest = "", "", False
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return "(unreadable)", "", False
    if "--selftest" in text or "def selftest" in text:
        selftest = True
    for ln in text.splitlines()[:60]:
        if not purpose:
            m = RE_PURPOSE.match(ln)
            # only accept a purpose line naming THIS file
            if m and m.group(1).lower() == path.name.lower():
                purpose = m.group(2)
                continue
        if not usage:
            m = RE_USAGE.match(ln)
            if m and m.group(1) and not m.group(1).startswith("#"):
                usage = m.group(1)
    if not purpose:
        # ================================================================
        # SECOND CONVENTION: the MODULE DOCSTRING (Integrator 2026-08-16).
        # RE_PURPOSE only recognised a `# <name>.py — purpose` banner, so every
        # tool that documents itself in a docstring scored UNDOCUMENTED.
        # MEASURED: 33 of the 34 "undocumented" tools carry a docstring purpose
        # and ZERO are genuinely bare — including `rel_decomp.py`, whose
        # chain-of-custody covenant is the governing rule for our own decoding
        # and which read as undocumented while being cited as the addendum.
        # **The index under-reported its own headline number by 33**, which is
        # the same class as the seven fixture-based controls above: an
        # instrument recognising one convention and reporting the rest absent.
        # ================================================================
        m = re.match(r'\s*(?:#![^\n]*\n)?\s*(?:"""|\'\'\')\s*(.{10,200}?)(?:\.\s|\n)',
                     text, re.S)
        if m:
            purpose = " ".join(m.group(1).split())
    if not purpose:
        # ================================================================
        # NO FALLBACK. This used to guess: it took the first prose comment
        # in the file and printed it as the tool's purpose. That produced
        # EIGHT confidently wrong descriptions - `shim_ledger` came out as
        # "strip comments so commented-out decls don't register",
        # `template_name` as "setStt(s8): stores its arg into the state-
        # number field", `jpc_crosscheck` as "Py3.14 compat: gclib uses...".
        # **Those read as descriptions and were internal code comments**, and
        # I used that output to nominate tools for merging - a redundancy
        # call built on invented evidence.
        # A missing purpose is now REPORTED as missing. An index that admits
        # a gap is usable; one that fills gaps with plausible text is not.
        # ================================================================
        return None, usage, selftest
    return purpose, usage, selftest


def main():
    named, ctl = workflow_named(), controlled()
    tools = sorted(p for p in HERE.glob("*.py") if p.name != "tool_index.py")

    rows = []
    for p in tools:
        purpose, usage, selftest = describe(p)
        rows.append((p.name, purpose, usage, selftest,
                     p.name in named, p.name in ctl))

    steps = [r for r in rows if r[4]]
    spec = [r for r in rows if not r[4]]

    lines = []
    add = lines.append
    add("# TOOL INDEX - tools/foundry")
    add("")
    add("> **GENERATED by `python tools/foundry/tool_index.py --write`. Do not")
    add("> hand-edit** - a hand-kept list of %d tools is wrong the day after"
        % len(rows))
    add("> it is written. Regenerate it instead.")
    add(">")
    add("> `workflow.py` is the MAP: the phases a port walks, naming only the")
    add("> tools that are workflow STEPS. **This is the INVENTORY** - every")
    add("> instrument that exists, including the specialist ones the map")
    add("> deliberately leaves out. A lane cannot use what it cannot find.")
    add(">")
    add("> **`ctl`** = a negative control is declared in `control.py`.")
    add("> **`self`** = the tool carries its own `--selftest` or fixture.")
    add("> **Read `self` before writing a new control** - `l2c_equiv` had one")
    add("> all along while every lane called it uncontrolled.")
    add("")
    undocumented = [r[0] for r in rows if r[1] is None]
    add("Totals: **%d tools** - %d workflow steps, %d specialist - "
        "%d with a declared control, %d carrying a self-test."
        % (len(rows), len(steps), len(spec),
           sum(1 for r in rows if r[5]), sum(1 for r in rows if r[3])))
    add("")
    if undocumented:
        add("> **%d of %d tools have NO PURPOSE LINE** and are listed below as"
            % (len(undocumented), len(rows)))
        add("> `** UNDOCUMENTED **`. They are not described here because this")
        add("> index will not guess - an earlier version did, and printed eight")
        add("> internal code comments as if they were descriptions, which is")
        add("> what a redundancy review then got wrong. **A tool nobody can")
        add("> describe is a tool nobody can tell apart from another one.**")
        add(">")
        add("> Fix by adding one line to the file's banner:")
        add("> `# <name>.py - <what it does>`")
        add("")

    for title, group in (("Workflow steps (named by `workflow.py`)", steps),
                         ("Specialist instruments (not workflow steps)", spec)):
        add("## %s" % title)
        add("")
        add("| tool | ctl | self | what it does |")
        add("|---|:--:|:--:|---|")
        for name, purpose, _usage, selftest, _n, has_ctl in group:
            desc = ("** UNDOCUMENTED **" if purpose is None
                    else purpose.replace("|", "\\|"))
            add("| `%s` | %s | %s | %s |"
                % (name, "yes" if has_ctl else "", "yes" if selftest else "",
                   desc))
        add("")

    text = "\n".join(lines) + "\n"
    if "--write" in sys.argv:
        OUT.write_text(text, encoding="utf-8", newline="\n")
        print("wrote %s (%d tools indexed)" % (OUT.name, len(rows)))
        return 0
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
