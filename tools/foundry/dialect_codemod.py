#!/usr/bin/env python3
# ============================================================================
# dialect_codemod.py — K3 of the port-automation plan (tale §849; unblocked
# 2026-08-14 when R5's CODE_DIALECT reached 47 rows).
#
# WHAT IT IS: the mechanical half of a port's dialect translation. Every row
# in R5's CODE_DIALECT is a donor token, its receiver form, and a RECEIPT.
# This applies them to a donor TU and reports what it changed and what it
# REFUSES to change — it does not port an actor, it removes the typing.
#
# WHAT IT REFUSES, by design (the semantic 30% stays History's):
#   · DROP rows ("(dropped — …)") are NEVER auto-deleted. Deleting a call is
#     a semantic act: the surrounding statement, its result use, and its
#     braces are the porter's judgment. Reported as MANUAL.
#   · Rows whose receiver form is a CHOICE ("ARRAY_SIZE or literal") are
#     reported, never guessed.
#   · Anything not in R5 is untouched and INVISIBLE to this tool — stated on
#     every run, because a codemod that looks thorough is how a port gets
#     declared done (№31-C).
#
# THE GATE (K1's precedent): --golden <donor> <receiver> regenerates the
# dialect pass over the DONOR TU and measures it against the HAND port. It
# does not demand equality — the hand port also did semantic work — it
# reports whether the codemod's mechanical changes AGREE with what the human
# did. Disagreement is the finding.
#
# Usage:
#   dialect_codemod.py --scan <donor.cpp>            what would change
#   dialect_codemod.py --apply <donor.cpp> <out.cpp> write the translated copy
#   dialect_codemod.py --golden <donor.cpp> <recv.cpp>   agreement vs hand port
# Read-only unless --apply, which writes ONLY to the named output path.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(REPO / "tools"))
import conversion_db as cdb

RE_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def mask_comments(text):
    """(masked, restore) — comments become opaque tokens.

    FIRST-RUN DEFECT (caught by the golden gate itself): the hand-ported Jabun
    reads `return fopMsg_MODE_MSG_END_e;  // port name (donor
    fopMsgStts_MSG_ENDS_e)` — the donor token survives ONLY in a comment that
    documents the translation, and the raw-text check called that a divergence.
    kit_laws already carries this law ("a banner that names a forbidden call
    must not read as a violation — that is how a linter trains people to delete
    comments"); this tool needed it twice over, because on --apply a comment
    naming the donor form is a RECEIPT and rewriting it would destroy the
    provenance the port is supposed to keep."""
    store = []

    def keep(m):
        store.append(m.group(0))
        return "\x01C%dC\x01" % (len(store) - 1)

    masked = RE_COMMENT.sub(keep, text)

    def restore(s):
        for i, c in enumerate(store):
            s = s.replace("\x01C%dC\x01" % i, c)
        return s

    return masked, restore


def code_only(text):
    return RE_COMMENT.sub(" ", text)


RE_DROP = re.compile(r"^\(dropped", re.I)
RE_CHOICE = re.compile(r"\bor\b|\?|/", re.I)
RE_IDENT = re.compile(r"^[A-Za-z_][\w:]*$")


def classify(key, fact):
    """AUTO (safe token->token) · MANUAL (drop) · REVIEW (choice/non-token)."""
    to = str(fact.value)
    if RE_DROP.match(to.strip()):
        return "MANUAL", "deletion is a semantic act — porter decides the statement"
    src_token = RE_IDENT.match(key.strip()) or key.strip().endswith("*")
    if not src_token:
        return "REVIEW", "donor side is not a plain token (expression/idiom)"
    if RE_CHOICE.search(to) or not RE_IDENT.match(to.strip().rstrip("*")):
        return "REVIEW", "receiver side is a choice or an expression, not one token"
    return "AUTO", "token -> token"


def rules():
    out = []
    for key, fact in cdb.CODE_DIALECT.items():
        kind, why = classify(key, fact)
        out.append((key.strip(), str(fact.value).strip(), fact.tier, fact.why, kind, why))
    return out


def apply_rules(text, rs):
    """Returns (new_text, [(key, to, n_hits)]) for AUTO rows only."""
    changed = []
    for key, to, _tier, _receipt, kind, _why in rs:
        if kind != "AUTO":
            continue
        if key.endswith("*"):                      # wildcard family, e.g. fopMsgStts_*
            stem = key[:-1]
            pat = re.compile(r"\b%s(\w+)\b" % re.escape(stem))
            repl_stem = to[:-1] if to.endswith("*") else to
            n = len(pat.findall(text))
            if n:
                text = pat.sub(lambda m: repl_stem + m.group(1), text)
                changed.append((key, to, n))
        else:
            pat = re.compile(r"\b%s\b" % re.escape(key))
            n = len(pat.findall(text))
            if n:
                text = pat.sub(to, text)
                changed.append((key, to, n))
    return text, changed


def report_manual(text, rs):
    hits = []
    for key, to, _t, receipt, kind, why in rs:
        if kind == "AUTO":
            continue
        probe = key[:-1] if key.endswith("*") else key
        token = re.match(r"[A-Za-z_][\w:]*", probe)
        if not token:
            continue
        n = len(re.findall(r"\b%s" % re.escape(token.group(0)), text))
        if n:
            hits.append((kind, key, to, n, why, receipt))
    return hits


def main():
    rs = rules()
    mode = sys.argv[1] if len(sys.argv) > 1 else ""
    if mode not in ("--scan", "--apply", "--golden") or len(sys.argv) < 3:
        print("usage: dialect_codemod.py --scan <donor.cpp> | --apply <donor.cpp> "
              "<out.cpp> | --golden <donor.cpp> <recv.cpp>")
        print("R5 CODE_DIALECT rows: %d (AUTO %d · MANUAL %d · REVIEW %d)"
              % (len(rs), sum(1 for r in rs if r[4] == "AUTO"),
                 sum(1 for r in rs if r[4] == "MANUAL"),
                 sum(1 for r in rs if r[4] == "REVIEW")))
        return 2
    src = Path(sys.argv[2])
    if not src.is_file():
        print("donor TU not found: %s" % src)
        return 2
    text = src.read_text(encoding="utf-8", errors="replace")

    masked, restore = mask_comments(text)
    new_masked, changed = apply_rules(masked, rs)
    new = restore(new_masked)
    manual = report_manual(code_only(text), rs)

    print("DIALECT CODEMOD (R5 CODE_DIALECT, %d rows) — %s" % (len(rs), src.name))
    print("\n  AUTO applied (%d rule(s), %d site(s)):"
          % (len(changed), sum(c[2] for c in changed)))
    for key, to, n in changed:
        print("    %-34s -> %-28s x%d" % (key, to, n))
    if not changed:
        print("    (none)")
    print("\n  NOT automated — porter's call (%d):" % len(manual))
    for kind, key, to, n, why, receipt in manual:
        print("    [%s] %-28s -> %-24s x%d  %s" % (kind, key, to[:24], n, why))
        print("           receipt: %s" % receipt[:96])
    if not manual:
        print("    (none present in this TU)")

    if mode == "--apply":
        if len(sys.argv) < 4:
            print("\n--apply needs an output path (never writes over the donor)")
            return 2
        out = Path(sys.argv[3])
        if out.resolve() == src.resolve():
            print("\nREFUSED: output == donor. The donor disc/source is immutable.")
            return 2
        out.write_text(new, encoding="utf-8", newline="\n")
        print("\nwrote %s (AUTO rows only; MANUAL/REVIEW untouched)" % out)

    if mode == "--golden":
        if len(sys.argv) < 4:
            print("\n--golden needs the receiver TU")
            return 2
        recv = Path(sys.argv[3])
        if not recv.is_file():
            print("receiver TU not found: %s" % recv)
            return 2
        rtext = code_only(recv.read_text(encoding="utf-8", errors="replace"))
        dtext = code_only(text)
        agree, disagree = [], []
        for key, to, _t, _r, kind, _w in rs:
            if kind != "AUTO":
                continue
            k = key[:-1] if key.endswith("*") else key
            t = to[:-1] if to.endswith("*") else to
            donor_has = re.search(r"\b%s" % re.escape(k), text)
            if not donor_has:
                continue
            recv_new = re.search(r"\b%s" % re.escape(t), rtext)
            recv_old = re.search(r"\b%s" % re.escape(k), rtext)
            if recv_new and not recv_old:
                agree.append((key, to))
            else:
                disagree.append((key, to, bool(recv_new), bool(recv_old)))
        print("\n  GOLDEN vs hand port %s:" % recv.name)
        print("    AGREE   %d rule(s) — the human made the same swap" % len(agree))
        for key, to in agree:
            print("      %-32s -> %s" % (key, to))
        if disagree:
            print("    DIVERGE %d — inspect before trusting the codemod here:" % len(disagree))
            for key, to, has_new, has_old in disagree:
                state = ("donor form STILL PRESENT receiver-side" if has_old
                         else "receiver form absent — hand port used another idiom")
                print("      %-32s -> %-24s %s" % (key, to, state))
        verdict = "SOUND on this TU" if agree and not disagree else \
                  ("NO OVERLAP — this TU exercises no AUTO rule" if not agree
                   else "REVIEW REQUIRED")
        print("    verdict: %s" % verdict)

    print("\n  SCOPE: only R5 rows exist to this tool. Anything not in "
          "CODE_DIALECT is\n  UNTOUCHED and UNSEEN — a clean run is not a ported actor.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
