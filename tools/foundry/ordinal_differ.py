#!/usr/bin/env python3
# ============================================================================
# ordinal_differ.py — L4: the SHARED ORDINAL TABLE gate (CALLS row, tale §911).
#
# THE BLIND SPOT IT CLOSES (Integrator's proof): dEvent_exception_c read GREEN
# at all three existing levels — TU exists (L1), every symbol resolves (L2),
# no registry row covers it (L3) — while 12 of 13 WW arrival ids resolved to
# the WRONG event. L3 catches SYMBOL-PRESENT-BUT-SYSTEM-UNARMED; this catches
# SYSTEM-PRESENT-BUT-ITS-ORDINAL-TABLE-IS-THE-RECEIVER'S: two lineages sharing
# an index space with different vocabularies.
#
# THE TAXONOMY IS HOUSING'S CORRECTION (tale §910), VERIFIED HERE BEFORE
# ENCODING — the discriminator is at the CALL SITE, not the table:
#   (A) TRUE ORDINAL   a data-derived id SUBSCRIPTS the array
#                      (receiver d_event_manager.cpp:144 `soecial_names[
#                      mEventInfoIdx - 201]`) -> test LENGTH + ORDER
#   (B) NAME-LOOKUP    the array is handed to a strcmp loop; the index is an
#                      OUTPUT (d_event_manager.cpp:1246-1259 getMyActIdx
#                      strcmps action[i] and returns i) -> test SET MEMBERSHIP
#                      ONLY. Order/length divergence here is INERT.
#   (C) NON-TABLE      the vocabulary is not a table at all (§909's
#                      `035door`->35->DEMO_DOOR_e). **NO DIFFER CAN EVER SEE
#                      THIS.** Printed on every run so a clean report is never
#                      read as "all vocabularies agree".
# Sweeping B with A's test yields confident false verdicts in both directions,
# which is why the corpus definition mattered more than the tool.
#
# Usage: ordinal_differ.py <receiver.cpp> [donor.cpp]   (donor path inferred)
# Read-only. Exit 0 clean · 1 findings · 2 could-not-run (№31-C).
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
DONOR_SRC = Path("D:/XXXXXXX/WW DP/src")

# MACRO-TOLERANT (calibration receipt, first run): the receiver's camera table
# is `static DUSK_CONSTEXPR char DUSK_CONST* ActionNames[34]` — a literal
# `static const char*` pattern reported 0 tables and a CLEAN verdict on a file
# that has one. A differ that cannot see a table reports agreement it never
# tested, which is the exact false-clean class this whole level exists to kill.
RE_TABLE = re.compile(
    r"static\s+[\w\s]*?\bchar\b[\w\s]*?\*\s*(\w+)\s*\[\s*(\d*)\s*\]"
    r"\s*=\s*\{(.*?)\}\s*;", re.S)
RE_ENTRY = re.compile(r'"((?:[^"\\]|\\.)*)"')
RE_FUNC = re.compile(r"^[\w:*&<>\s]*?(\w+)::(\w+)\s*\([^;{]*\)\s*\{"
                     r"|^[\w*&<>\s]+?(\w+)\s*\([^;{]*\)\s*\{", re.M)


CTRL = {"if", "for", "while", "switch", "else", "do", "return", "catch"}


def enclosing_function(text, pos):
    """Nearest preceding definition — control keywords excluded (the first run
    reported `if::soecial_names`: an `if (...) {` matched the definition
    shape). Pairing keys on this name, so a wrong name = a missed pair."""
    best = None
    for m in RE_FUNC.finditer(text[:pos]):
        nm = m.group(2) or m.group(3) or m.group(1)
        if nm and nm not in CTRL:
            best = nm
    return best or "(file scope)"


def tables(path):
    """[(func, name, entries, classification, evidence)] for one TU."""
    text = path.read_text(encoding="utf-8", errors="replace")
    out = []
    for m in RE_TABLE.finditer(text):
        name, _decl_n, body = m.group(1), m.group(2), m.group(3)
        entries = RE_ENTRY.findall(body)
        if len(entries) < 2:
            continue
        func = enclosing_function(text, m.start())
        # CLASS DISCRIMINATOR — at the call site (Housing §910), verified:
        #   subscript with a NON-loop expression  -> A
        #   bare name passed as a call argument   -> B
        subs = re.findall(r"\b%s\s*\[([^\]]+)\]" % re.escape(name), text)
        data_subs = [s for s in subs
                     if not re.fullmatch(r"\s*[ijkn]\s*", s)
                     and not re.fullmatch(r"\s*\d+\s*", s)]
        passed = re.search(r"\(\s*[^)]*\b%s\b\s*[,)]" % re.escape(name), text)
        # ROUND-TRIP TEST (correction receipt): a bare subscript is NOT enough.
        # d_camera's `var_r29 = getMyActIdx(..., ActionNames, ...)` followed by
        # `ActionNames[var_r29]` subscripts with the LOOKUP'S OWN OUTPUT — that
        # is class B displaying its result, and the first discriminator called
        # it A (a confident false positive of exactly the kind §910 warned
        # about; Housing's hand-call was right, the tool was wrong). So: if a
        # subscript variable is ASSIGNED FROM a call that receives this table,
        # the subscript is an output, not a data-derived id.
        roundtrip = set()
        for s in data_subs:
            for ident in re.findall(r"[A-Za-z_]\w*", s):
                if re.search(r"\b%s\s*=\s*\w+\s*\([^;]*\b%s\b" % (re.escape(ident),
                                                                  re.escape(name)), text):
                    roundtrip.add(s)
        genuine = [s for s in data_subs if s not in roundtrip]
        if genuine:
            cls, why = "A", "data-derived subscript: %s[%s]" % (name, genuine[0].strip())
        elif passed:
            extra = (" (its subscripts are the lookup's own output — round-trip "
                     "display, not an id path)" if roundtrip else "")
            cls, why = "B", ("handed to a call (name-lookup): %s%s"
                             % (passed.group(0).strip()[:50], extra))
        else:
            cls, why = "UNKNOWN", "no subscript and no call use found — classify by hand"
        out.append((func, name, entries, cls, why))
    return out


def sweep():
    """Every receiver d/ TU with a donor counterpart — the corpus answer.
    Class A findings are mis-resolve candidates; class B set-gaps are
    silent-no-op candidates; class C stays invisible (printed every run)."""
    a_hits, b_hits, scanned = [], [], 0
    for recv in sorted((REPO / "src" / "d").rglob("*.cpp")):
        rel = recv.resolve().relative_to((REPO / "src").resolve())
        donor = DONOR_SRC / rel
        if not donor.is_file():
            continue
        scanned += 1
        rt, dt = tables(recv), tables(donor)
        for rfunc, rname, rentries, rcls, _why in rt:
            cand = [d for d in dt if d[0] == rfunc] or [d for d in dt if d[1] == rname]
            if not cand:
                continue
            dentries = cand[0][2]
            if rcls == "A":
                div = next((i for i in range(min(len(rentries), len(dentries)))
                            if rentries[i] != dentries[i]), None)
                if div is not None or len(rentries) != len(dentries):
                    a_hits.append((rel, rfunc, rname, div, len(dentries), len(rentries)))
            elif rcls == "B":
                missing = [e for e in dentries if e not in set(rentries)]
                if missing:
                    b_hits.append((rel, rfunc, rname, missing))
    print("ORDINAL SWEEP — %d TU(s) with donor counterparts" % scanned)
    print("\nCLASS A (true ordinal — MIS-RESOLVE candidates):")
    for rel, f, n, div, dl, rl in a_hits or []:
        print("  %s %s::%s — donor %d vs receiver %d, diverge at idx %s"
              % (rel, f, n, dl, rl, div if div is not None else "(length only)"))
    if not a_hits:
        print("  (none)")
    print("\nCLASS B (name-lookup — SET-GAP, silent no-op candidates):")
    for rel, f, n, missing in b_hits or []:
        print("  %s %s::%s — donor names absent receiver-side: %s"
              % (rel, f, n, ", ".join(missing[:6])))
    if not b_hits:
        print("  (none)")
    print("\n  CLASS C stays INVISIBLE (§910) — non-table vocabularies "
          "(§909's `035door`->35) are unreachable by any differ.")
    return 1 if (a_hits or b_hits) else 0


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "--sweep":
        return sweep()
    if len(sys.argv) < 2:
        print("usage: ordinal_differ.py <receiver.cpp> [donor.cpp] | --sweep")
        return 2
    recv = Path(sys.argv[1])
    if not recv.is_file():
        print("receiver TU not found: %s" % recv)
        return 2
    if len(sys.argv) > 2:
        donor = Path(sys.argv[2])
    else:
        rel = recv.resolve().relative_to((REPO / "src").resolve())
        donor = DONOR_SRC / rel
    print("ORDINAL DIFFER  receiver=%s" % recv.name)
    if not donor.is_file():
        print("  UNRESOLVED-LOOKUP: no donor TU at %s (not proof of agreement)" % donor)
        return 2
    rt, dt = tables(recv), tables(donor)
    print("  tables: receiver %d · donor %d" % (len(rt), len(dt)))
    findings = 0
    used = set()
    for rfunc, rname, rentries, rcls, rwhy in rt:
        cand = [d for d in dt if d[0] == rfunc] or \
               [d for d in dt if d[1] == rname] or \
               [d for d in dt if d[1].rstrip("s_names table") == rname.rstrip("s_names table")]
        if not cand:
            note = ("class B tables are typically declared by the CALLER actor, "
                    "not the lookup TU — sweep the caller" if rcls == "B"
                    else "UNKNOWN, not clean")
            print("\n  [%s] %s::%s — no donor counterpart in this TU (%s)"
                  % (rcls, rfunc, rname, note))
            findings += 1
            continue
        dfunc, dname, dentries, dcls, _dwhy = cand[0]
        used.add(dname)
        print("\n  [class %s] %s::%s  vs donor %s::%s" % (rcls, rfunc, rname, dfunc, dname))
        print("      %s" % rwhy)
        if rcls == "A":
            if len(rentries) != len(dentries):
                print("      LENGTH: donor %d vs receiver %d" % (len(dentries), len(rentries)))
            div = next((i for i in range(min(len(rentries), len(dentries)))
                        if rentries[i] != dentries[i]), None)
            if div is None and len(rentries) == len(dentries):
                print("      [OK] identical order and length — ids resolve alike")
            else:
                findings += 1
                if div is not None:
                    print("      [MISRESOLVE] agree to idx %d, diverge at idx %d:"
                          % (div - 1, div))
                    print("          donor[%d]=%s   receiver[%d]=%s"
                          % (div, dentries[div], div, rentries[div]))
                    print("      every donor id >= %d resolves to the WRONG name here"
                          % div)
                else:
                    print("      [TRUNCATION] orders agree but lengths differ — donor "
                          "ids past %d have no receiver entry" % (min(len(rentries), len(dentries)) - 1))
        elif rcls == "B":
            missing = [e for e in dentries if e not in set(rentries)]
            print("      order/length are INERT here (index is an output, §910)")
            if missing:
                findings += 1
                print("      [SET-GAP] donor name(s) absent from the receiver set: %s"
                      % ", ".join(missing[:8]))
                print("      consequence: silent no-op on lookup, not a mis-resolve")
            else:
                print("      [OK] every donor name exists in the receiver set")
        else:
            findings += 1
            print("      [UNKNOWN] classify at the call site before trusting any verdict")
    print("\n  CLASS C REMINDER (always, §910): vocabularies that are NOT tables "
          "are\n  INVISIBLE to this tool — §909's `035door`->35->DEMO_DOOR_e has no "
          "table in\n  either codebase. A clean report here NEVER means "
          "'all vocabularies agree'.")
    print("ORDINAL DIFFER %s" % ("CLEAN" if not findings else "FINDINGS: %d" % findings))
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
