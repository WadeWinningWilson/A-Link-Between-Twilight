#!/usr/bin/env python3
# ============================================================================
# row_store.py — the seam-tracker row store (roadmap A3). Bridge hosts.
#
# `index`     regenerate tracker/INDEX.md as a PURE FUNCTION of the row files
# `validate`  check every row against tracker/_schema.json, refuse loudly
# `mint`      print a fresh random row ID
# `selftest`  NEGATIVE CONTROL — feed it bad rows and require refusal
#
# WHY EACH PROPERTY EXISTS (spec SPEC-row-store.md; receipts, not preferences):
#  · one file per row      -> git is the allocator; the tale bus's single
#                             authored surface produced 26 section collisions
#  · minted random IDs     -> a counter needs an allocator, an allocator needs
#                             a lane to remember it
#  · symbol is a FIELD     -> renames must not orphan a row (B2b vs B2-b2)
#  · index is GENERATED    -> regeneration merges nothing; it is a function
#  · domains ENUMERATE THE BOUNDED SIDE -> an unknown value FAILS, never
#                             buckets silently; and it fails PESSIMISTICALLY
#  · provenance NEVER stored -> parsed live from KIT-LINEAGE (fifth-roster)
#  · citations carry a READ TIMESTAMP -> on a live tree, "I read X" and
#                             "it is Y" are both true (WwPilot 0 -> 2)
#  · c0 sweep before index -> a BS byte disabled a regex for a tool's whole
#                             life; BOMs broke parsing twice
# ============================================================================
import json
import os
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
TRACKER = REPO / "docs" / "state" / "ww-staging" / "tracker"
ROWS = TRACKER / "rows"
SCHEMA = TRACKER / "_schema.json"
INDEX = TRACKER / "INDEX.md"

FIELD = re.compile(r"^([a-z][a-z0-9-]*):\s*(.*)$")
CITE_TS = re.compile(r"\b(read|run)\s+\d{4}-\d{2}-\d{2}")


def load_schema():
    return json.loads(SCHEMA.read_text(encoding="utf-8"))


def parse_row(path):
    """key: value, plus 'key:' introducing an indented block. ASCII keys."""
    raw = path.read_bytes()
    fields, key, block = {}, None, []
    for line in raw.decode("utf-8", "replace").splitlines():
        if line[:1] in (" ", "\t", "-") and key:
            block.append(line.strip())
            continue
        m = FIELD.match(line)
        if m:
            if key and block:
                fields.setdefault(key, "\n".join(block))
            key, val = m.group(1), m.group(2).strip()
            block = []
            if val and val != "|":
                fields[key] = val
                key = None
        elif not line.strip():
            continue
    if key and block:
        fields.setdefault(key, "\n".join(block))
    return fields


def c0_clean(paths):
    """Refuse the index on ANY invisible byte or BOM. Uses the estate's tool."""
    if not paths:
        return True, "no rows"
    r = subprocess.run([sys.executable, str(HERE / "c0_audit.py")] + [str(p) for p in paths],
                       capture_output=True, text=True, encoding="utf-8",
                       errors="replace", cwd=str(REPO))
    out = (r.stdout or "") + (r.stderr or "")
    return ("clean" in out and "carry invisible bytes" not in out), out.strip().splitlines()[-1:]


def validate(verbose=True):
    s = load_schema()
    files = sorted(ROWS.glob("*.md")) if ROWS.is_dir() else []
    errs = []
    seen_ids = {}
    for p in files:
        f = parse_row(p)
        rid = f.get("id", "")
        if not re.match(s["id"]["pattern"], rid):
            errs.append("%s: id %r does not match %s" % (p.name, rid, s["id"]["pattern"]))
        if rid in seen_ids:
            errs.append("%s: duplicate id %s (also %s)" % (p.name, rid, seen_ids[rid]))
        seen_ids[rid] = p.name
        if p.stem != rid:
            errs.append("%s: filename must equal id (%s)" % (p.name, rid))
        for req in s["required_fields"]:
            if not f.get(req):
                errs.append("%s: missing required field %r" % (p.name, req))
        for fld in ("doorway", "destination"):
            v = f.get(fld)
            if v and v not in s[fld]["values"]:
                errs.append("%s: %s=%r NOT IN DECLARED DOMAIN %s"
                            % (p.name, fld, v, s[fld]["values"]))
        for axis, allowed in s["axes"].items():
            if axis.startswith("_"):
                continue
            v = f.get(axis.lower())
            if v and v not in allowed:
                errs.append("%s: axis %s=%r NOT IN INVENTORY-SCHEMA vocabulary %s"
                            % (p.name, axis, v, allowed))
        for cond, req in s["required_if"].items():
            if cond.startswith("_"):
                continue
            k, _, want = cond.partition("==")
            if f.get(k) == want:
                for r in req:
                    if not f.get(r):
                        errs.append("%s: %s=%s requires %r (spec: assumed-necessary does not ship)"
                                    % (p.name, k, want, r))
        # ------------------------------------------------------------------
        # CROSS-AXIS: a NONMATCHING source can NEVER yield a BYTE-TRUE claim.
        #
        # This is the FILING half of the §801 enforcement (2026-08-16). The
        # Integrator holds the BUILD half: a BUILD-QUEUE row derived from the
        # WwJMessage family stamped BYTE-TRUE is refused at their gate. Foundry's
        # join: "refusal at filing AND at build means laundering has to fail
        # twice to succeed once."
        #
        # WHY IT IS A RULE AND NOT A REVIEWER'S EYE: §801 admits a NonMatching
        # donor body as a BEHAVIOURAL spec under labelling. "Behaviourally
        # specified" and "byte-true" are the two claims that ruling exists to
        # keep apart, and they are one careless field apart in this very file.
        # A row that sets both has laundered a NonMatching provenance into a
        # matched one - silently, and downstream of every instrument that would
        # otherwise have caught it.
        #
        # NOT keyed on filenames (processor.cpp / resource.cpp): the general form
        # is what is true. Byte-identical output cannot come from a source that
        # does not itself byte-match, whatever TU it lives in.
        # ------------------------------------------------------------------
        # WIDENED 2026-08-16 while reviewing SPEC-byte-true-verified.md. The first
        # cut refused only BYTE-TRUE and let `EQUIVALENT` through — but the spec's
        # derivation rule (§3.4) CAPS a NONMATCHING/EQUIVALENT source at DIVERGENT,
        # and EQUIVALENT is better than DIVERGENT. So layer 1 (filing) was strictly
        # more permissive than layer 3 (derivation): a row could be FILED with a
        # claim the deriver would never emit.
        #
        # Three layers only defend anything if they agree on what they forbid. A
        # gap between them is not defence in depth — it is the widest layer's rule
        # wearing three names. Verified with a control before and after: the
        # EQUIVALENT row passed a clean 114-row validate, which is how the hole
        # was found rather than argued.
        if f.get("portable") == "NONMATCHING" and f.get("byte-true") in ("BYTE-TRUE", "EQUIVALENT"):
            errs.append("%s: REFUSED — portable=NONMATCHING with byte-true=%s. A "
                        "non-byte-matching source caps at DIVERGENT (§801 labelling; "
                        "SPEC-byte-true-verified §3.4 derivation cap).\n"
                        "    THIS REFUSAL HAS TWO READINGS — check which one you are:\n"
                        "      (1) LAUNDERING: the row claims fidelity the source cannot "
                        "support. The cap stands; file DIVERGENT.\n"
                        "      (2) A HALF-ADVANCED PAIR: the TU genuinely reached upstream "
                        "`Equivalent` (DECODER-BRIEFING §2b — functionally correct, not "
                        "byte-exact, a REAL outcome and not a failure) and only ONE axis "
                        "moved. The two axes advance TOGETHER:\n"
                        "          portable: NONMATCHING -> DECOMPILED   (it is decoded now)\n"
                        "          byte-true: DIVERGENT  -> EQUIVALENT   (correct, not exact)\n"
                        "      DECOMPILED + EQUIVALENT is FILEABLE and is the correct record "
                        "for a §2b Equivalent TU. Leaving portable at NONMATCHING is what "
                        "trips this gate, not the EQUIVALENT claim itself."
                        % (p.name, f.get("byte-true")))

        # ------------------------------------------------------------------
        # ONE TU PER ROW (the comma ruling, Foundry 2026-08-16, decided on
        # History/Bridge's ask when the drain's last TU fell to two tools
        # reading a multi-TU field differently - drain_derive split on the
        # comma, ww_ratchet did not, and the sheet's cannot-disagree promise
        # broke on exactly that). One TU per row is also what per-axis truth
        # REQUIRES: History's own s43cacbf5 split proved a Matching and a
        # NonMatching TU under one `portable:` forces a pessimistic floor in
        # prose. The 3 pre-ruling multi-TU rows were split the same day
        # (RETIRED.md carries the transitions) - per-TU decomp_status turned
        # their floors into real verdicts.
        # ------------------------------------------------------------------
        if "," in f.get("tu", ""):
            errs.append("%s: REFUSED — tu lists multiple TUs (%r). ONE TU per "
                        "row: per-axis states are per-TU claims, and every "
                        "enumerator on the board reads tu as one path."
                        % (p.name, f.get("tu", "")[:60]))

        prov = f.get("provenance", "")
        if prov and prov != "see-file":
            errs.append("%s: provenance is NEVER stored (got %r) — parsed live from KIT-LINEAGE"
                        % (p.name, prov))
        for line in f.get("citations", "").splitlines():
            line = line.lstrip("- ").strip()
            if line and not CITE_TS.search(line):
                errs.append("%s: citation lacks a read/run timestamp: %s" % (p.name, line[:60]))
            # ----------------------------------------------------------------
            # A CITED REPO PATH MUST EXIST. The timestamp rule made citations
            # DATED; it never made them REAL. `drain_derive.py` hardcoded
            # `src/d/ext_plugin/registry.cpp` in its citation template — a path
            # that does not exist — and 92 rows shipped a correct FINDING with
            # an unverifiable RECEIPT, passing validate every time.
            #
            # A citation exists to let someone else re-derive the claim. One
            # pointing at nothing is worse than none: it looks checked.
            #
            # Only in-repo paths are checked — donor paths (D:/XXXXXXX/...) are
            # on another volume that may legitimately be absent on another
            # machine, and refusing those would make the store unusable rather
            # than honest.
            # ----------------------------------------------------------------
            for tok in re.findall(r"(?:^|[\s,])((?:src|include|tools|docs|libs|mods-src)/[\w./+-]+)", line):
                if not (REPO / tok).exists():
                    errs.append("%s: citation names a path that DOES NOT EXIST: %s"
                                % (p.name, tok))
    ok, tail = c0_clean(files)
    if not ok:
        errs.append("c0_audit REFUSED the row set: %s" % tail)
    if verbose:
        print("rows: %d · errors: %d" % (len(files), len(errs)))
        for e in errs:
            print("  REFUSED  %s" % e)
        if not errs:
            print("  all rows valid; index may be generated")
    return files, errs


def lineage_of(sym_file):
    """provenance parsed LIVE, never stored."""
    p = REPO / sym_file
    if not p.is_file():
        return "(no file)"
    for line in p.read_text(encoding="utf-8", errors="replace").splitlines()[:60]:
        m = re.match(r"^﻿?\s*//\s*KIT-LINEAGE:\s*(\S+)", line)
        if m:
            return m.group(1)
    return "(untagged)"


def baseline_gate():
    """A4's check, invoked HERE — at the point rows are WRITTEN.

    INVOCATION-POINT DECISION (History/Bridge 2026-08-16, on the Integrator's
    catch that A4 was wired to nothing): the deletion gate fires when the index
    is regenerated, NOT in the build path. Regenerating the index is the one
    step anyone editing rows must take, so the person who deleted a row is the
    person the gate stops, at the moment they do it.

    A build-time stop would fire on SOMEONE ELSE'S row deletion and read as
    "my build broke for no reason" — the Integrator's words, and §839 makes
    them the only builder, so a build-stop is their policy call, not mine.
    `row_baseline.py check` stays available for that if they want it.
    """
    try:
        import importlib.util
        spec = importlib.util.spec_from_file_location("rb", HERE / "row_baseline.py")
        rb = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(rb)
    except Exception as e:
        print("  (baseline gate unavailable: %s — index continues)" % e)
        return 0, []
    rc, vanished = rb.check(verbose=False)
    return rc, vanished


def build_index():
    files, errs = validate(verbose=False)
    if errs:
        print("INDEX REFUSED — %d validation error(s). Run `row_store.py validate`." % len(errs))
        for e in errs[:10]:
            print("  %s" % e)
        return 1
    rc, vanished = baseline_gate()
    if rc:
        print("INDEX REFUSED — BASELINE DIVERGENCE. These rows were pinned and are")
        print("now gone with no entry in tracker/RETIRED.md:")
        for v in vanished:
            print("    %s" % v)
        print("  RETIRED is a state transition, never a deletion (spec §5). Either")
        print("  add the retirement entry or restore the row; do not re-pin to hide it.")
        return 1
    rows = []
    for p in files:
        f = parse_row(p)
        tu = f.get("tu", "")
        rows.append((f.get("id", ""), f.get("symbols", ""), f.get("doorway", ""),
                     f.get("destination", ""), lineage_of(tu) if tu else "-", p.name))
    lines = [
        "# SEAM TRACKER — INDEX (GENERATED, DO NOT EDIT)",
        "",
        "> Generated by `tools/foundry/row_store.py index` as a PURE FUNCTION of",
        "> `rows/*.md`. Hand edits are lost on regeneration. Authored content lives",
        "> ONLY in the per-row files. `provenance` is parsed LIVE from each TU's",
        "> `KIT-LINEAGE` header — never stored in a row (fifth-roster rule).",
        "",
        "| id | symbols | doorway | destination | provenance (live) | file |",
        "|---|---|---|---|---|---|",
    ]
    for r in sorted(rows):
        lines.append("| `%s` | `%s` | %s | %s | %s | [%s](rows/%s) |"
                     % (r[0], r[1], r[2], r[3], r[4], r[5], r[5]))
    lines += ["", "%d row(s)." % len(rows)]
    INDEX.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print("INDEX written: %d row(s) -> %s" % (len(rows), INDEX.relative_to(REPO)))
    return 0


def check_schema(verbose=True):
    """THE FLAG `_schema.json` ALREADY PROMISED, AND DID NOT HAVE.

    Its own header says the axis states are transcribed from INVENTORY-SCHEMA
    §4 and that "if §4 changes, this file is wrong and the generator's
    --check-schema flag says so."  **No tool in the estate implemented that
    flag** (Librarian, 2026-08-16). The transcription was CORRECT when checked
    — 5 of 5 axes matched — but nothing was keeping it correct, which is a
    guard named in prose and absent in mechanism: the shape this repo has spent
    a week cataloguing.

    INVENTORY-SCHEMA.md is the AUTHORITY (Librarian custody); _schema.json is a
    machine-readable transcription. On disagreement the JSON is wrong by
    definition — this never edits the markdown."""
    doc = REPO / "docs" / "state" / "ww-staging" / "INVENTORY-SCHEMA.md"
    if not doc.is_file():
        print("*** cannot check: %s not found ***" % doc)
        return 2
    text = doc.read_text(encoding="utf-8", errors="replace")

    # §4's table rows: | 4.1 | **PORTABLE** | question | `A` · `B` … | src |
    want = {}
    for m in re.finditer(r"^\|\s*4\.\d\s*\|\s*\*\*([A-Z-]+)\*\*\s*\|[^|]*\|([^|]*)\|",
                         text, re.M):
        axis = m.group(1).strip()
        states = re.findall(r"`([A-Z][A-Z-]*)`", m.group(2))
        # RELINQUISHED carries a footnote marker in the table; strip nothing else.
        if states:
            want[axis] = states

    schema = load_schema()
    # Skip `_`-prefixed comment keys — `validate()` already does this and my
    # first version did not, so it read the `_why` STRING as an axis and
    # `list()`-ed it into 160 single characters, reporting DRIFT on a clean
    # schema. A drift detector that cries drift is the false positive that
    # gets the detector ignored. Also refuse anything that is not a list
    # rather than coercing it.
    have = {k: list(v) for k, v in (schema.get("axes") or {}).items()
            if not k.startswith("_") and isinstance(v, list)}

    print("SCHEMA TRANSCRIPTION CHECK — _schema.json vs INVENTORY-SCHEMA §4")
    print("  authority : %s" % doc.relative_to(REPO))
    print("  transcript: %s" % SCHEMA.relative_to(REPO))
    if not want:
        print("  ** COULD NOT PARSE §4's axis table — reporting UNKNOWN, not OK **")
        print("  (a parser that cannot see the authority must not bless the copy)")
        return 2

    bad = 0
    for axis in sorted(set(want) | set(have)):
        w, h = want.get(axis), have.get(axis)
        if w is None:
            print("  [EXTRA ] %-10s in JSON, absent from §4: %s" % (axis, h)); bad += 1
        elif h is None:
            print("  [MISSING] %-9s in §4, absent from JSON: %s" % (axis, w)); bad += 1
        elif set(w) != set(h):
            print("  [DRIFT ] %-10s §4=%s  json=%s" % (axis, w, h)); bad += 1
        elif verbose:
            print("  [ OK   ] %-10s %d state(s)" % (axis, len(w)))
    print()
    if bad:
        print("  %d axis/axes DRIFTED. **_schema.json is wrong by definition** —"
              " INVENTORY-SCHEMA is the authority. Fix the JSON, never the md." % bad)
        return 1
    print("  transcription matches on all %d axes." % len(want))
    return 0


def selftest():
    """NEGATIVE CONTROL: every check must be shown to REFUSE."""
    import tempfile
    s = load_schema()
    cases = [
        ("bad id", "id: nope\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\n"),
        ("undeclared doorway", "id: s00000001\nsymbols: x\ndoorway: MAYBE\ndestination: PLUGIN\ncreated: X\n"),
        ("undeclared destination", "id: s00000002\nsymbols: x\ndoorway: EXISTS\ndestination: SOMEWHERE\ncreated: X\n"),
        ("non-schema axis state", "id: s00000003\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\nlinked: DECLARED\n"),
        ("PATCH without negative-control", "id: s00000004\nsymbols: x\ndoorway: ABSENT-unhookable\ndestination: PATCH\ncreated: X\n"),
        ("stored provenance", "id: s00000005\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\nprovenance: native-port\n"),
        ("citation without timestamp", "id: s00000006\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\ncitations:\n  - src/foo.cpp:1\n"),
        # the s801 laundering gate (History/Bridge 2026-08-16) - demonstrated
        # live at install (inject -> REFUSED -> delete) and carried HERE as a
        # permanent case, because a control demonstrated once and not carried
        # is a control that can silently die (the A4 lesson).
        ("laundered NONMATCHING as BYTE-TRUE", "id: s00000007\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\nportable: NONMATCHING\nbyte-true: BYTE-TRUE\n"),
        # the comma ruling: one TU per row (case carried same-change)
        ("multi-TU tu field", "id: s00000008\nsymbols: x\ntu: src/a.cpp, src/b.cpp\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\n"),
        # the WIDENED laundering rule (History/Bridge, on the byte-true spec
        # review): EQUIVALENT from a NONMATCHING source is also refused -
        # layer 1 must forbid exactly what layer 3's cap forbids. Their fix
        # was verified live (inject -> refuse -> delete); this carries it,
        # same as case 7 carried the original.
        ("laundered NONMATCHING as EQUIVALENT", "id: s00000009\nsymbols: x\ndoorway: EXISTS\ndestination: PLUGIN\ncreated: X\nportable: NONMATCHING\nbyte-true: EQUIVALENT\n"),
    ]
    print("SELFTEST — each case MUST be refused (a check that cannot fail is worthless)\n")
    bad = 0
    for name, body in cases:
        d = Path(tempfile.mkdtemp())
        rid = re.search(r"id:\s*(\S+)", body).group(1)
        (d / ("%s.md" % rid)).write_text(body, encoding="utf-8", newline="\n")
        global ROWS
        keep, ROWS = ROWS, d
        _, errs = validate(verbose=False)
        ROWS = keep
        status = "REFUSED" if errs else "**ACCEPTED — CHECK IS BLIND**"
        if not errs:
            bad += 1
        print("  [%-32s] %s" % (name, status))
        if errs:
            print("        %s" % errs[0][:110])
    print("\n%d/%d checks demonstrated live%s" % (len(cases) - bad, len(cases),
          "" if not bad else "  ** %d BLIND **" % bad))
    return 1 if bad else 0


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "validate"
    if cmd == "mint":
        print("s%s" % os.urandom(4).hex())
        return 0
    if cmd == "validate":
        _, errs = validate()
        return 1 if errs else 0
    if cmd == "index":
        return build_index()
    if cmd == "check-schema":
        return check_schema()
    if cmd == "selftest":
        return selftest()
    print("usage: row_store.py validate | index | mint | selftest | check-schema")
    return 2


if __name__ == "__main__":
    sys.exit(main())
