#!/usr/bin/env python3
# ============================================================================
# calls_rotate.py — keep CALLS.md the size of the WORK, not the history.
#
# WHY (user, 2026-08-14): CALLS.md is the surface every lane reads to find its
# trigger, and 90% of it had become answered rows — 66 answered vs 5 active,
# 79 KB of receipts in front of the 5 lines anyone needs. A trigger surface
# that costs a scroll is a trigger surface lanes start skimming (§389b).
#
# WHAT IT DOES: moves ANSWERED rows (`- [x]`) to CALLS-ARCHIVE.md, verbatim,
# append-only, date-stamped. Nothing is ever deleted — answered rows carry
# their §-citations and stay greppable one file away.
#
# RETENTION: today's answered rows STAY (--keep-days, default 1). A lane that
# filed a call this session must still be able to see its own answer in place;
# archiving that instantly would trade one confusion for another.
#
# CONCURRENCY (the single-writer lesson, learned on the playtest card): lanes
# append to this file constantly. So: hash before, hash again immediately
# before writing, REFUSE if it changed. A rotation that silently eats another
# lane's just-filed row would be worse than a long file.
#
# Usage:  calls_rotate.py [--check] [--keep-days N]
# Read-only with --check. Exit 0 done/clean · 1 would-rotate (--check) · 2 refused.
# ============================================================================
import datetime
import hashlib
import io
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"
ARCHIVE = REPO / "docs" / "state" / "ww-staging" / "CALLS-ARCHIVE.md"
RE_DATE = re.compile(r"(\d{4})-(\d{2})-(\d{2})")


def row_date(line):
    """Newest date mentioned in the row (rows carry a filed date and may carry
    an answer date). No date = treated as OLD, which is the safe direction."""
    ds = [datetime.date(int(a), int(b), int(c)) for a, b, c in RE_DATE.findall(line)]
    return max(ds) if ds else datetime.date(1970, 1, 1)


def main():
    check = "--check" in sys.argv
    keep_days = 1
    if "--keep-days" in sys.argv:
        i = sys.argv.index("--keep-days")
        if i + 1 < len(sys.argv):
            keep_days = int(sys.argv[i + 1])
    if not CALLS.is_file():
        print("CALLS.md not found")
        return 2
    raw = CALLS.read_bytes()
    before = hashlib.sha256(raw).hexdigest()
    text = raw.decode("utf-8", "replace")
    lines = text.splitlines()
    cutoff = datetime.date.today() - datetime.timedelta(days=keep_days - 1)

    keep, move = [], []
    for ln in lines:
        if ln.startswith("- [x]") and row_date(ln) < cutoff:
            move.append(ln)
        else:
            keep.append(ln)

    active = sum(1 for l in lines if l.startswith("- [ ]"))
    stay_ans = sum(1 for l in keep if l.startswith("- [x]"))
    print("CALLS.md: %d row(s) active · %d answered staying (<%d day(s) old) · "
          "%d answered to archive" % (active, stay_ans, keep_days, len(move)))
    if not move:
        print("nothing to rotate")
        return 0
    saved = len(text) - len("\n".join(keep))
    print("  would shrink %d -> %d bytes (%d%% smaller)"
          % (len(text), len("\n".join(keep)), 100 * saved // max(1, len(text))))
    if check:
        return 1

    # CONCURRENCY GUARD — re-read immediately before writing.
    if hashlib.sha256(CALLS.read_bytes()).hexdigest() != before:
        print("REFUSED: CALLS.md changed while rotating (another lane appended). "
              "Nothing written — re-run.")
        return 2

    stamp = datetime.date.today().isoformat()
    head = ("# CALLS ARCHIVE — answered rows, verbatim, append-only\n\n"
            "> Rotated out of CALLS.md by `calls_rotate.py`. Nothing here is\n"
            "> deleted or edited: each row keeps its ask, its answer, and its\n"
            "> §-citation. Grep here for 'was this ever asked/answered'.\n")
    if not ARCHIVE.is_file():
        io.open(ARCHIVE, "w", encoding="utf-8", newline="\r\n").write(head)
    with io.open(ARCHIVE, "a", encoding="utf-8", newline="\r\n") as f:
        f.write("\n## rotated %s (%d row(s))\n\n" % (stamp, len(move)))
        for ln in move:
            f.write(ln + "\n")

    new_text = "\n".join(keep).rstrip() + "\n"
    io.open(CALLS, "w", encoding="utf-8", newline="\r\n").write(new_text)

    # VERIFY — no row may vanish (rows in + rows out == rows before).
    after = CALLS.read_text(encoding="utf-8", errors="replace")
    arch = ARCHIVE.read_text(encoding="utf-8", errors="replace")
    lost = [l for l in move if l not in arch]
    act_after = len(re.findall(r"(?m)^- \[ \] ", after))
    ok = not lost and act_after == active
    print("rotated %d row(s) -> %s" % (len(move), ARCHIVE.name))
    print("verify: active %d->%d · archived-intact %s · %s"
          % (active, act_after, "yes" if not lost else "NO (%d lost!)" % len(lost),
             "OK" if ok else "FAILED — restore from git"))
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
