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

    # ========================================================================
    # ROWS ARE BLOCKS, NOT LINES (Librarian, 2026-08-16).
    #
    # This loop was per-LINE, which was correct for every row a LANE has ever
    # written — one row, one physical line. **Then a TOOL filed one.**
    # `decomp_sync.py`'s row carries a fenced `decomp_watch` transcript and
    # occupies 21 physical lines.
    #
    # MEASURED against that live row, with it marked answered and aged past the
    # cutoff: **1 line moved to the archive and 18 non-blank lines stayed
    # behind** — an unmatched ``` fence orphaned in CALLS.md (it would swallow
    # everything rendered after it) and the row's own attribution/date footer
    # stranded, belonging to no row. The archive would hold a headline with its
    # evidence amputated; CALLS.md would hold evidence with no ask.
    #
    # **Both files corrupted, silently, by a tool whose whole covenant is
    # "nothing is ever deleted."** It would have been true line-by-line and
    # false row-by-row.
    #
    # The file's real grammar: a row runs from its `- [` line until the NEXT
    # `- [` line or EOF. Rotate that block. Anything before the first row (the
    # header) is never a row and always stays.
    # ========================================================================
    first_row = next((i for i, l in enumerate(lines) if l.startswith("- [")),
                     len(lines))
    keep, move = lines[:first_row], []
    blocks = []
    for i in range(first_row, len(lines)):
        if lines[i].startswith("- ["):
            blocks.append([lines[i]])
        elif blocks:
            blocks[-1].append(lines[i])
        else:                      # unreachable given first_row, kept explicit
            keep.append(lines[i])
    moved_rows = 0
    for blk in blocks:
        if blk[0].startswith("- [x]") and row_date(blk[0]) < cutoff:
            move.extend(blk)
            moved_rows += 1
        else:
            keep.extend(blk)
    multiline = sum(1 for b in blocks if len(b) > 1)

    active = sum(1 for l in lines if l.startswith("- [ ]"))
    stay_ans = sum(1 for l in keep if l.startswith("- [x]"))
    print("CALLS.md: %d row(s) active · %d answered staying (<%d day(s) old) · "
          "%d answered to archive" % (active, stay_ans, keep_days, moved_rows))
    if multiline:
        print("  (%d row(s) span multiple physical lines - rotated as WHOLE "
              "BLOCKS; a per-line rotate would amputate them)" % multiline)
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
        f.write("\n## rotated %s (%d row(s), %d line(s))\n\n"
                % (stamp, moved_rows, len(move)))
        for ln in move:
            f.write(ln + "\n")

    new_text = "\n".join(keep).rstrip() + "\n"
    io.open(CALLS, "w", encoding="utf-8", newline="\r\n").write(new_text)

    # ========================================================================
    # VERIFY. Two checks, and the SECOND one is the real one.
    #
    # THE COUNT CHECK (`active N->N`) was this tool's only verification and it
    # is a BEFORE-STATE check: it needs a snapshot taken before the change, so
    # only the process performing the rotation can ever run it, and only once.
    #
    # THE INVARIANT CHECK is History/Bridge's, banked here on Housing
    # Security's framing (2026-08-17): **"mine needed the before-state; yours
    # is a standing invariant."** `CALLS-ARCHIVE.md` must contain ZERO open
    # rows — not because a count balanced, but because an answered-row archive
    # containing an unanswered row is *definitionally* broken.
    #
    # WHY THAT IS STRICTLY BETTER: anyone can run it, at any time, with no
    # history and no trust in the rotator. It states the property you actually
    # care about ("nothing unanswered was archived") instead of a proxy the
    # property happens to imply. Same distinction as `--symbol SAFE` proving a
    # name resolves rather than a call working.
    #
    #   grep -c '^- \[ \]' docs/state/ww-staging/CALLS-ARCHIVE.md   # must be 0
    # ========================================================================
    after = CALLS.read_text(encoding="utf-8", errors="replace")
    arch = ARCHIVE.read_text(encoding="utf-8", errors="replace")
    lost = [l for l in move if l not in arch]
    act_after = len(re.findall(r"(?m)^- \[ \] ", after))
    open_in_archive = len(re.findall(r"(?m)^- \[ \] ", arch))
    ok = not lost and act_after == active and open_in_archive == 0
    print("rotated %d row(s) (%d line(s)) -> %s"
          % (moved_rows, len(move), ARCHIVE.name))
    print("verify: active %d->%d · archived-intact %s · %s"
          % (active, act_after, "yes" if not lost else "NO (%d lost!)" % len(lost),
             "OK" if ok else "FAILED — restore from git"))
    print("INVARIANT: open rows in the archive = %d %s"
          % (open_in_archive,
             "(must be 0 — re-runnable by anyone, needs no before-state)"
             if open_in_archive == 0 else
             "*** AN UNANSWERED ROW WAS ARCHIVED — RESTORE FROM GIT ***"))
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
