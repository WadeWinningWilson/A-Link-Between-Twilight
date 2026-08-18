#!/usr/bin/env python3
# ============================================================================
# file_row.py — FILE OR ANSWER A CALLS ROW, AND PROVE IT LANDED.
#
# THE FAILURE THIS EXISTS FOR (Integrator, tooling their own failure mode):
# every lane writes CALLS rows through an ad-hoc python block followed by a
# bare `echo "filed"` — and that echo runs whether or not the write happened.
# **It lied twice in one day**: once on a `SyntaxError: truncated \UXXXXXXXX
# escape` (a Windows path in a plain python string) and once on an
# AssertionError when another lane had already checked the target row. Both
# times nothing was written and the report said otherwise.
#
# THE GENERAL LESSON, which is worth more than the script: **a success message
# emitted by the same process that did the work is not evidence the work
# happened.** Verification has to re-read the artifact.
#
# WHY THIS IS A TOOL AND NOT A SCRATCHPAD SCRIPT: the Integrator's fix was
# `scratchpad/file_row.sh`. Correct shape, wrong home — scratchpad files are
# transient by definition and the estate has a standing one-transient-script
# rule, so a per-lane copy is how you get five diverging filers. Instruments
# live in tools/foundry (§850 consolidation) and every lane uses the same one.
#
# WHAT IT GUARDS BEYOND "did the write happen":
#   · AMBIGUITY — answering by substring is refused unless it matches EXACTLY
#     one row. Flipping the wrong lane's checkbox is silent and worse than
#     failing.
#   · ALREADY-ANSWERED — if the target row is already [x], that is reported as
#     a distinct outcome, not a failure and not a success. It is the exact case
#     that produced one of the two lies above.
#   · BYTE HYGIENE — re-reads as BYTES and refuses to leave NUL or backspace in
#     CALLS.md. Unquoted heredocs and shell escapes have injected invisible
#     control bytes into these files repeatedly; they are unmatchable by later
#     edits and silently break every downstream parser.
#   · CRLF is preserved; the file is never rewritten with mixed endings.
#
# ASCII-ONLY OUTPUT, learned by this tool failing its own thesis: the first
# real `add` WROTE the row correctly and then crashed printing a U+2713 tick
# on a cp1252 console. The work happened, the report died — the exact inversion
# of the failure this exists to prevent, and it would read to a caller as "the
# filing failed" and invite a duplicate. Success lines stay ASCII.
#
# Usage:
#   file_row.py add    --text "- [ ] LANE | **...** | attribution | date"
#   file_row.py answer --match "<substring unique to one open row>"
#   file_row.py verify --match "<substring>"        (read-only)
# Exit 0 verified · 1 NOT verified (nothing was reported as done) · 2 bad input.
# ============================================================================
import io
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"


def read():
    return io.open(CALLS, encoding="utf-8", newline=None).read()


def write(text):
    io.open(CALLS, "w", encoding="utf-8", newline="\r\n").write(text)


def byte_audit():
    """NUL/backspace in the artifact -> loud failure, never a silent pass."""
    raw = io.open(CALLS, "rb").read()
    nul, bs = raw.count(b"\x00"), raw.count(b"\x08")
    if nul or bs:
        print("*** BYTE HYGIENE FAILED *** NUL=%d BS=%d in CALLS.md" % (nul, bs))
        print("    Invisible control bytes are unmatchable by later edits and break")
        print("    every parser downstream. Repair before filing anything else.")
        return False
    return True


# ---------------------------------------------------------------------------
# CALLS-LANE-TOKEN LINT (Librarian doctrine, LIBRARIAN-QUEUE §BUS; named
# independently by Housing as the SOURCE-level fix for a 5-shape family of
# silent delivery failures).
#
# THE ARGUMENT FOR LINTING AT THE SOURCE rather than hardening each monitor:
# a lane field that can contain anything forces every reader to be clever, and
# they are each clever differently. Housing's monitor used `[A-Z/ ]+` and so
# silently dropped `INTEGRATOR/HousingTemp`; two of MY tools had the identical
# class. **A canonical token means no monitor has to be clever at all.**
#
# I am also a concrete offender, which is why this is not theoretical: my own
# withdrawal edit produced `- [x] ~~WITHDRAWN — FALSE…~~ HISTORY/BRIDGE, …|`,
# and routing_check then dutifully reported a LANE literally named
# "~~WITHDRAWN — FALSE, SEE TALE §977~~ HISTORY/BRIDGE, HOUSINGTEMP".
#
# It REJECTS rather than rewrites: silently normalising someone's row would
# make the tool an author, and a wrong guess about the addressee is the exact
# failure being prevented.
# THE LANE SET IS READ FROM THE ROSTER, NOT FROM MY MEMORY. Housing checked my
# first hardcoded set against docs/LANES.md — the user's tool-split ruling,
# Librarian-recorded 2026-07-25 — and it disagreed in BOTH directions. A lint
# that invents its own roster is a lane-definition authority, which this is
# emphatically not: LANES.md states "Corrections are the user's". So the
# ratified names come from the file, and anything else is declared, labelled,
# and surfaced rather than quietly blessed.
LANES_DOC = REPO / "docs" / "LANES.md"


def ratified_lanes():
    """Lane names from docs/LANES.md's table -> canonical tokens."""
    out = set()
    if not LANES_DOC.is_file():
        return out
    for ln in io.open(LANES_DOC, encoding="utf-8", errors="replace"):
        if not ln.startswith("| **"):
            continue
        name = ln.split("|")[1].strip().strip("*").strip()
        if name:
            # "Housing Security" is the ratified name; every lane writes
            # HOUSING or HOUSING/ENGINE in practice, so the first word is the
            # token. Recorded, not silently normalised — see the delta below.
            out.add(name.upper().split()[0])
    return out


# OPERATIONAL, NOT RATIFIED. These appear as addressees in live CALLS traffic
# but are absent from docs/LANES.md. They are accepted so the lint does not
# reject today's real work, and reported by `lint` so the gap stays visible
# instead of being laundered into doctrine by a tool. INTEGRATOR in particular
# is the ONLY builder under the §839 protocol and one of the most active lanes,
# yet the roster predates the role — that is a real question for the user, not
# something a linter should settle.
OPERATIONAL_LANES = {"INTEGRATOR", "HOUSINGTEMP"}
COLLECTIVE = {"ALL", "ALL LANES"}
KNOWN_LANES = (ratified_lanes() | OPERATIONAL_LANES | COLLECTIVE) or {
    "FOUNDRY", "HISTORY", "HOUSING", "HOUSINGTEMP", "INTEGRATOR", "LIBRARIAN",
    "BRIDGE", "ENGINE", "ALL", "ALL LANES"}


# ============================================================================
# ONE TOKENIZER, AND THE REASON IT IS A FUNCTION (Librarian, 2026-08-15)
#
# There were TWO. The lint split the lane field on both "/" and "," and checked
# every token; the roster-delta report at the bottom of `lint` did
# `field.split("/")[0]` — FIRST TOKEN ONLY, no comma handling. So the tool
# printed "ratified but never addressed in CALLS: BRIDGE, ENGINE" while 20+
# rows address ENGINE as the second half of "HOUSING/ENGINE", and "HISTORY/
# BRIDGE" addresses BRIDGE the same way.
#
# THE FAILURE CLASS IS THE ONE THIS WHOLE TOOLCHAIN EXISTS TO CATCH: a parser
# that could not SEE a thing reported the thing ABSENT, and the report read as
# a finding about the lanes rather than a limit of the parser. Same shape as
# the inert call-site filter in sig_diff.py and the row-81 mis-normalization.
# Absence of evidence, printed as a verdict.
#
# The pass/fail lint was never wrong — only this report line. Fixing the
# report and leaving two tokenizers in place would have preserved the ability
# to diverge again, so they are now one function with one caller contract.
# ============================================================================
def lane_tokens(row):
    """Raw addressee tokens of a CALLS row ([] if it has no parseable lane
    field). Case is PRESERVED — the lint needs it to detect miscasing."""
    if "|" not in row:
        return []
    head = row.split("|", 1)[0]
    if "]" not in head:
        return []
    field = head.split("]", 1)[1].strip()
    return [p.strip() for p in field.replace(",", "/").split("/") if p.strip()]


def lint_lane_field(row):
    """(ok, message). Enforces: every token in the lane field is a known lane,
    spelled in canonical upper case."""
    if "|" not in row:
        return False, "row has no '|' — cannot locate the lane field"
    head = row.split("|", 1)[0]
    if "]" not in head:
        return False, "row has no '- [ ]' checkbox"
    field = head.split("]", 1)[1].strip()
    if not field:
        return False, "lane field is empty — a row must name its addressee"
    toks = lane_tokens(row)
    bad, miscased = [], []
    for tk in toks:
        if tk.upper() in KNOWN_LANES:
            if tk != tk.upper():
                miscased.append(tk)
        else:
            bad.append(tk)
    if bad:
        return False, ("lane field contains %d token(s) that are not known lanes: %s\n"
                       "    field was: %r\n"
                       "    known lanes: %s\n"
                       "    Anything that is not an addressee (status markers, "
                       "strikethrough, notes)\n"
                       "    belongs in the BODY, after the first '|'."
                       % (len(bad), ", ".join(repr(b) for b in bad), field,
                          ", ".join(sorted(KNOWN_LANES))))
    if miscased:
        return False, ("lane token(s) not in canonical case: %s\n"
                       "    Use: %s\n"
                       "    Mixed case is what makes a row invisible to a "
                       "character-class matcher."
                       % (", ".join(repr(m) for m in miscased),
                          " / ".join(t.upper() for t in toks)))
    return True, "lane field OK: %s" % " / ".join(toks)


def rows_matching(text, needle, open_only=False):
    out = []
    for i, ln in enumerate(text.splitlines()):
        if not ln.startswith("- ["):
            continue
        if open_only and not ln.startswith("- [ ]"):
            continue
        if needle in ln:
            out.append((i, ln))
    return out


def opt(name):
    if name in sys.argv:
        i = sys.argv.index(name)
        if i + 1 < len(sys.argv):
            return sys.argv[i + 1]
    return None


def slurp(path):
    """Row/annotation text from a FILE — the shell-proof path.

    THE VECTOR THIS CLOSES (History, tale §981): backticked symbol names inside
    DOUBLE quotes on a CLI arg are COMMAND-SUBSTITUTED BY BASH before python
    ever sees them, so the names silently vanish. Our rows are full of `code`
    spans, so --text is a live hazard on every call. It is the §951 escape
    hazard through a new vector — not a heredoc, an argv element — which is
    why "control group, zero backticks eaten" was true for heredocs only and
    did not generalise. A file never passes through word expansion at all.
    """
    p = Path(path)
    if not p.is_file():
        print("*** FAILED *** --file %s does not exist" % path)
        return None
    return io.open(p, encoding="utf-8", newline=None).read().strip()


def flatten_note(note):
    """Collapse a note to ONE LINE. CALLS.md is one row per line.

    ========================================================================
    THE DEFECT THIS CLOSES (Integrator caught it, 2026-08-16). A multi-line
    note was appended straight onto the row, so everything after its FIRST
    PARAGRAPH became loose text in CALLS.md and the row itself carried only
    the opening sentence. A ~4,000-character answer arrived as **243 bytes**
    - "a headline and one framing sentence, with NO scope, NO baseline and
    NONE of the numbers it promises".

    **Nothing was lost and nothing was delivered**, which is the worst shape:
    the content sat orphaned in the file where no row parser would ever
    attribute it, and the filer got `ANSWERED OK`. Every multi-paragraph
    answer filed through this tool has the same wound.
    ========================================================================
    """
    return re.sub(r"\s*\n\s*", "  ", note.strip())


VALID_FLAGS = {
    # `--dry-run` validates and reports, writing NOTHING (Librarian,
    # 2026-08-16). Added after a two-line CONTROL FIXTURE was filed into the
    # live bus while testing the block-form warning: there was no way to
    # exercise `add` without publishing a row, so "test the filer" and
    # "publish" were the same action. Removed by hand, CRLF preserved, lint
    # re-run clean — but a tool whose only test mode is production will be
    # tested in production.
    "add": {"--text", "--file", "--dry-run"},
    # ANNOTATE — append evidence to a row REGARDLESS of checkbox state, and
    # verify the text landed. Needed because `answer` correctly refuses an
    # already-[x] row, which left the Integrator unable to re-file the
    # annotation my own bug ate. Appending EVIDENCE to a settled row is
    # legitimate; appending a new ASK is not (it reaches nobody — file a row).
    "annotate": {"--match", "--note", "--note-file"},
    "answer": {"--match", "--note", "--note-file"},
    "verify": {"--match"},
    "lint": {"--match"},
}


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd not in VALID_FLAGS:
        print("usage: file_row.py add    --file <path> | --text '<row>'")
        print("       file_row.py answer --match '<substring>' "
              "[--note-file <path> | --note '<text>']")
        print("       file_row.py verify --match '<substring>'")
        print("       file_row.py lint")
        return 2

    # NO SILENTLY-IGNORED ARGUMENTS. The defect History hit was not only that
    # `answer` did not append text — it was that `answer --text ...` ACCEPTED
    # the flag, discarded it, and reported success. A tool whose whole premise
    # is "verify what actually happened" must never swallow an argument it does
    # not implement. Unknown or wrong-command flags are now fatal.
    given = {a for a in sys.argv[2:] if a.startswith("--")}
    unknown = given - VALID_FLAGS[cmd]
    if unknown:
        print("*** FAILED *** %s does not accept: %s"
              % (cmd, ", ".join(sorted(unknown))))
        if cmd == "answer" and "--text" in unknown:
            print("    `answer` flips the checkbox. To ALSO append an annotation, use")
            print("    --note-file <path> (shell-proof) or --note '<text>'.")
        print("    Nothing was written. Accepted flags for %s: %s"
              % (cmd, ", ".join(sorted(VALID_FLAGS[cmd]))))
        return 2

    if cmd == "annotate":
        text = read()
        hits = rows_matching(text, opt("--match") or "@@NOMATCH@@")
        if len(hits) != 1:
            print("*** NOT ANNOTATED *** substring matches %d row(s); need exactly 1."
                  % len(hits))
            return 1
        note = slurp(opt("--note-file")) if opt("--note-file") else opt("--note")
        if note is None or not note.strip():
            print("*** FAILED *** annotate needs --note-file <path> or --note '<text>'")
            return 2
        idx, ln = hits[0]
        lines = text.splitlines()
        lines[idx] = ln.rstrip() + " -> " + flatten_note(note)
        write("\n".join(lines) + "\n")
        after = read().splitlines()
        probe = note.strip()[:40]
        if idx >= len(after) or probe not in after[idx]:
            print("*** ANNOTATION FAILED *** note is NOT in the row after the write.")
            print("    Probe looked for: %r" % probe)
            print("    Backticks inside DOUBLE quotes are eaten by bash before this")
            print("    tool sees them — use --note-file.")
            return 1
        if not byte_audit():
            return 1
        print("ANNOTATED OK  note verified present by re-reading CALLS.md")
        print("  %s" % after[idx][:110])
        return 0

    if cmd == "add":
        arg = slurp(opt("--file")) if opt("--file") else opt("--text")
        if opt("--file") and arg is None:
            return 2
    else:
        arg = opt("--match") or ("-" if cmd == "lint" else None)

    if not arg or not arg.strip():
        print("*** FAILED *** %s needs a non-empty argument (an empty one would be a "
              "silent no-op)" % cmd)
        return 2
    if not CALLS.is_file():
        print("*** FILING FAILED *** %s not found" % CALLS)
        return 1

    if cmd == "lint":
        # THE LINT GUARDS NEW ROWS; this sweeps the ones already written, so the
        # corpus's real state is a number rather than an assumption.
        bad = []
        for i, ln in enumerate(read().splitlines(), 1):
            if not ln.startswith("- ["):
                continue
            ok, why = lint_lane_field(ln)
            if not ok:
                bad.append((i, ln.split("|", 1)[0].strip(), why.splitlines()[0]))
        print("CALLS-LANE-TOKEN sweep: %d row(s) with a non-canonical lane field"
              % len(bad))
        for i, head, why in bad:
            print("  line %-5d %-52s %s" % (i, head[:52], why[:70]))
        if not bad:
            print("  every existing row's addressee parses to a known lane.")
        # SURFACE THE ROSTER DELTA. Not a verdict — the user owns LANES.md.
        rat = ratified_lanes()
        print()
        print("  LANE ROSTER (source: docs/LANES.md, the user's ruling — not this tool)")
        print("    ratified   : %s" % (", ".join(sorted(rat)) or "NONE PARSED"))
        print("    operational: %s  <- accepted but ABSENT from LANES.md"
              % ", ".join(sorted(OPERATIONAL_LANES)))
        # EVERY token, not just the first — see lane_tokens() above.
        addressed = {t.upper()
                     for ln in read().splitlines() if ln.startswith("- [")
                     for t in lane_tokens(ln)}
        unused = sorted(rat - addressed)
        if unused:
            print("    ratified but never addressed in CALLS: %s" % ", ".join(unused))
        else:
            print("    every ratified lane has been addressed at least once.")
        print("    **Corrections to the roster are the USER'S, not mine.** This tool")
        print("    reads LANES.md; it does not define lanes. INTEGRATOR is the §839")
        print("    sole builder yet is absent from the roster — a real question.")
        return 1 if bad else 0

    if cmd == "verify":
        hits = rows_matching(read(), arg)
        for _i, ln in hits:
            print("  %s" % ln[:110])
        print("%s %d row(s) match" % ("FOUND" if hits else "NOT FOUND", len(hits)))
        return 0 if hits else 1

    if cmd == "add":
        if not arg.startswith("- ["):
            print("*** FILING FAILED *** a row must start with '- [ ] LANE |'")
            return 2

        # ------------------------------------------------------------------
        # MULTI-LINE ROW CONVENTION (Librarian ruling, 2026-08-16, on Foundry's
        # routed design note). WARNS, never refuses — blocks are LEGAL.
        #
        # `calls_rotate.py` now rotates rows as BLOCKS, so a multi-line row is
        # no longer a corruption risk. But legality is not desirability: every
        # ad-hoc reader in the estate assumes one row = one line, and Foundry
        # audited their own consumers to find two that quietly do.
        #
        # THE CONVENTION: a machine filing a long transcript writes the
        # transcript to a DATED ARTIFACT and files a ONE-LINE row pointing at
        # it. The row surface stays grep-shaped; the transcript stays whole;
        # no consumer needs block-awareness. Foundry's framing, adopted:
        # **the rotate fix makes blocks legal; the pointer form keeps them rare.**
        #
        # THE LIBRARIAN'S CONDITION ON IT — the pointer must outlive the row.
        # A row that points at a scratch path is WORSE than a long row: the
        # evidence leaves the bus and lands somewhere nothing indexes, which is
        # exactly how the project-origin account became unfindable while being
        # perfectly preserved. Durable location or keep the block.
        # ------------------------------------------------------------------
        # --dry-run: validate and report, write NOTHING.
        #
        # Added 2026-08-16 after I filed a two-line CONTROL FIXTURE into the
        # live bus while testing the warning immediately below. There was no
        # way to exercise the `add` path without writing to CALLS.md, so
        # "test the filer" and "publish a row" were the same action.
        # A tool whose only test mode is production is a tool that will be
        # tested in production.
        if "--dry-run" in sys.argv:
            ok2, why2 = lint_lane_field(arg)
            n2 = arg.count("\n") + 1
            print("DRY RUN — nothing written to %s" % CALLS.name)
            print("  lane field : %s" % why2)
            print("  physical lines: %d%s" % (n2, "  (block form; see the "
                  "pointer-row convention)" if n2 > 1 else ""))
            print("  byte hygiene of the CANDIDATE TEXT: NUL=%d BS=%d"
                  % (arg.count("\x00"), arg.count("\x08")))
            return 0 if ok2 else 2

        nl = arg.count("\n")
        if nl:
            print("NOTE: this row spans %d physical lines." % (nl + 1))
            print("  Legal — calls_rotate.py rotates rows as blocks — but the")
            print("  convention for long machine output is a POINTER ROW:")
            print("    1. write the transcript to a dated artifact under")
            print("       docs/state/ww-staging/ (durable + indexed, NOT a")
            print("       scratch path — a pointer that outlives nothing is")
            print("       worse than a long row)")
            print("    2. file a ONE-LINE row citing that path")
            print("  Every ad-hoc reader in this estate assumes one row = one")
            print("  line; two of Foundry's own did. Filing anyway.")
        ok, why = lint_lane_field(arg)
        if not ok:
            print("*** FILING FAILED *** CALLS-LANE-TOKEN lint: %s" % why)
            print("    Nothing was written. A row whose addressee cannot be parsed")
            print("    reaches nobody, which is the failure this lint exists to stop.")
            return 2
        before = read()
        if arg.strip() in before:
            print("*** NOT FILED *** that exact row is already present — filing it "
                  "again would duplicate a call")
            return 1
        write(before.rstrip() + "\n" + arg.rstrip() + "\n")
        # RE-READ. This is the whole point: the check must not trust the write.
        after = read()
        if arg.strip() not in after:
            print("*** FILING FAILED *** row is not in CALLS.md after the write")
            return 1
        if not byte_audit():
            return 1
        print("FILED OK  verified by re-reading CALLS.md (%d rows total)"
              % after.count("\n- ["))
        # ------------------------------------------------------------------
        # THIS RECEIPT IS TRUE AND IT IS HALF THE JOB.
        #
        # Housing Security, 2026-08-17, after three rulings landed nowhere:
        # *"`FILED OK · verified by re-reading CALLS.md` proves an
        # announcement was published. It proves nothing about the artifact,
        # and I read it as completion three times today."*
        #
        # That is the whole mechanism of the announce-vs-record failure: the
        # receipt is genuine, it is verified, and it certifies the wrong half.
        # A careful lane reasonably stops here. So the reminder is printed AT
        # THE POINT OF THE FALSE COMPLETION, not in a doctrine doc nobody
        # re-reads. (Recorded in full in docs/Librarian.md.)
        # ------------------------------------------------------------------
        print("  ^ this verifies the ANNOUNCEMENT landed. If this row carries a")
        print("    RULING, the ARTIFACT the tools read is what makes it real —")
        print("    worksheet · ww-ownership-map.json · _schema.json · LANES.md ·")
        print("    DO-NOT.md · NEVER-PUSH-STRIP-SET.md. Write the artifact FIRST,")
        print("    then file the row citing it. If you MAY NOT write it (auditing")
        print("    another lane's artifact), name the exact file+edit and WHO")
        print("    applies it, and keep the ruling OPEN until you READ IT BACK.")
        return 0

    # answer
    text = read()
    hits = rows_matching(text, arg, open_only=True)
    if len(hits) != 1:
        already = rows_matching(text, arg)
        if not hits and already:
            # THE CASE THAT PRODUCED ONE OF THE TWO LIES: another lane already
            # checked it. Not a failure, not a success — its own outcome.
            print("ALREADY ANSWERED — %d matching row(s) are already [x]. Nothing "
                  "written." % len(already))
            for _i, ln in already:
                print("  %s" % ln[:110])
            return 0
        print("*** NOT ANSWERED *** substring matches %d OPEN rows; need exactly 1."
              % len(hits))
        print("    Refusing rather than guessing — checking the wrong lane's box is")
        print("    silent and worse than failing.")
        for _i, ln in hits:
            print("  %s" % ln[:110])
        return 1
    # OPTIONAL ANNOTATION. History's report: a caller who passes text and gets
    # "ANSWERED OK" reasonably believes the text landed. So the annotation is
    # now IMPLEMENTED, and — the actual fix — VERIFIED SEPARATELY from the
    # checkbox. Confirming a state change that is not the one the caller asked
    # for is exactly the failure this tool exists to prevent, committed by the
    # tool itself.
    note = slurp(opt("--note-file")) if opt("--note-file") else opt("--note")
    if opt("--note-file") and note is None:
        return 2
    if note is not None and not note.strip():
        print("*** FAILED *** --note/--note-file is empty; refusing a silent box-flip")
        return 2

    idx, ln = hits[0]
    lines = text.splitlines()
    lines[idx] = ln.replace("- [ ]", "- [x]", 1)
    flat = flatten_note(note) if note else None
    if flat:
        lines[idx] = lines[idx].rstrip() + " -> " + flat
    write("\n".join(lines) + "\n")

    after = read().splitlines()
    if idx >= len(after) or not after[idx].startswith("- [x]"):
        print("*** ANSWER FAILED *** checkbox is not [x] after the write")
        return 1
    # TWO INDEPENDENT CHECKS. The box being [x] says nothing about the note.
    if flat:
        # ================================================================
        # PROBE THE TAIL AND THE LENGTH, NOT JUST THE HEAD. This checked
        # `note[:40]`, so a note whose first 40 characters landed passed -
        # which is exactly how a 4,000-char answer verified OK at 243 bytes.
        # **A verification that only inspects the beginning of the thing it
        # is verifying is the `echo "filed"` failure this file exists to
        # replace, moved one level in.**
        # ================================================================
        head, tail = flat[:40], flat[-40:]
        if tail not in after[idx] or len(after[idx]) < len(flat):
            print("*** ANNOTATION TRUNCATED *** the row does not carry the "
                  "whole note.")
            print("    note is %d chars; the row gained %d."
                  % (len(flat), max(0, len(after[idx]) - len(ln))))
            print("    tail looked for: %r" % tail)
            return 1
        probe = head
        if probe not in after[idx]:
            print("*** ANNOTATION FAILED *** the checkbox flipped but the note is NOT")
            print("    in the row. The box change is real; your text is not.")
            print("    Probe looked for: %r" % probe)
            print("    If your note contained backticks inside DOUBLE quotes, bash")
            print("    command-substituted them away before this tool saw them —")
            print("    use --note-file, which never passes through word expansion.")
            return 1
    if not byte_audit():
        return 1
    print("ANSWERED OK  checkbox verified by re-reading CALLS.md%s"
          % ("; ANNOTATION verified present" if note else " (no annotation requested)"))
    print("  %s" % after[idx][:110])
    return 0


if __name__ == "__main__":
    sys.exit(main())
