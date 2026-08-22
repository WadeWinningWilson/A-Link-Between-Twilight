#!/bin/sh
# ============================================================================
# Librarian call watch — CALLS.md, WITH PROOF OF LIFE
#
# MATCH RULE: line begins "- [" (EITHER checkbox state - see the both-states
# block below) AND the lane field (text before the first "|") literally
# contains LIBRARIAN or ALL LANES, case-folded.
# No \b anywhere: HANDOFF-LIBRARIAN.md (4) flags word-boundary regex as
# unverified on this Git Bash. Field-scoping also stops body prose that merely
# names the lane from registering as a call (CALLS.md:303 is a live example —
# a HOUSING row whose body says "Librarian doctrined it").
#
# PULSE, retrofitted per Foundry's ALL LANES monitor-arming call (2026-08-15):
# TaskList does not show monitors, so a dead watcher and a quiet board are the
# SAME observation. This stamps every pass; monitor_pulse.py reads the age.
#
# WHY A SEPARATE PULSE FILE, deliberately not the shared one: Foundry's
# registry uses one shared monitor-pulse.json so that a mismatched pid or
# interval EXPOSES a duplicate. A second lane's watcher stamping that same file
# would trip that detector on every pass — a permanent false positive that
# would destroy the very signal it exists to give. Distinct lane, distinct
# pulse. Cross-check this one against the Librarian row in MONITOR-REGISTRY.md.
# ============================================================================

REPO="${REPO:-$(cd "$(dirname "$0")/../.." && pwd)}"
F="$REPO/docs/state/ww-staging/CALLS.md"
# Session-independent state dir. The scratchpad copy this was promoted from
# would not have existed tomorrow - the exact "lives only in a task" defect
# the record-your-monitor order names.
SP="${LIBRARIAN_WATCH_STATE:-$(cd "$(dirname "$0")" && pwd)/.librarian-watch}"
mkdir -p "$SP"
SEEN="$SP/librarian-seen.txt"
PULSE="$REPO/tools/foundry/monitor-pulse-librarian.json"
INTERVAL=20

touch "$SEEN"
passes=0

# STAMP THE WINDOWS PID, NOT `$$` — the pulse reader checks pid liveness with a
# Windows process query, and under Git Bash `$$` is an MSYS pid from a DIFFERENT
# NAMESPACE. This shell: `$$` 21938 vs winpid 35800; the previous watcher stamped
# MSYS 20726 while running as Windows 30232. The reader looked up 20726, found
# nothing, and returned **DEAD for a demonstrably live watcher** — a FALSE DEAD,
# which is as corrosive as a false ALIVE: it invites a re-arm, and a re-arm is
# how duplicates get made. Python watchers are unaffected (os.getpid() already
# returns the Windows pid), so this bites shell-based watchers only.
WINPID=$(cat /proc/$$/winpid 2>/dev/null || echo $$)

stamp() {
  # $1 = last_event, $2 = note
  printf '{"updated":"%s","updated_epoch":%s,"interval_sec":%s,"pid":%s,"passes":%s,"last_event":"%s","note":"%s","lane":"LIBRARIAN","watches":"CALLS.md LIBRARIAN/ALL LANES"}\n' \
    "$(date -u '+%Y-%m-%d %H:%M:%SZ')" "$(date -u '+%s')" "$INTERVAL" "$WINPID" \
    "$passes" "$1" "$2" > "$PULSE"
}

# SELF-FIRE SUPPRESSION, with the guard that makes it safe.
# Rows I file to ALL LANES woke my own watcher three times. That is pure noise:
# I cannot be informed of a row I just wrote. Suppressed by ATTRIBUTION — the
# second-to-last pipe field, which the row format reserves for it.
#
# THE GUARD, and it is the whole reason this is not a missed-call generator:
# suppression applies ONLY when the lane field does NOT name LIBRARIAN. A row
# addressed TO my lane fires even if I am recorded as its author. Suppressing
# on authorship alone would let a badly-parsed attribution swallow a real call,
# which is the failure class this lane keeps filing about.
#
# Suppressions are COUNTED and reported on the pulse (`last_event`), never
# silent — a filter you cannot see working is a filter you cannot audit.
#
# The suppressed count rides stdout on a `##SELF` marker line rather than
# stderr: stderr is already reserved for real scan failures, and folding the
# two together would hide an awk error inside a routine counter.
# ============================================================================
# BOTH CHECKBOX STATES, NOT JUST OPEN (2026-08-16, adopting Foundry's finding
# rather than agreeing with it in prose).
#
# This matched `^- \[ \]` — OPEN ONLY. **An ANSWER flips `[ ]`→`[x]`, so every
# reply to a Librarian call dropped straight out of the match.** The watcher
# woke reliably for calls TO me and stayed silent for every answer I was given.
#
# MEASURED on my own board before changing anything: 55 open vs **116 answered**
# rows addressed to LIBRARIAN / ALL LANES — **32% coverage.** Foundry measured
# the identical defect in `lane_watch.py` hours earlier (34 → 106) and I read
# their registry entry describing it while re-arming. Fourth blindness of this
# class in the estate in one night, second in this file.
#
# A liveness pulse could never have caught it: the watcher was running, and
# truthfully reporting that nothing it was LOOKING AT had changed.
# ============================================================================
scan() {
  awk -F'|' '
  /^- \[/ {
    lane = toupper($1)
    mine_by_name = (index(lane, "LIBRARIAN") > 0)
    broadcast    = (index(lane, "ALL LANES") > 0)

    # ================================================================
    # ADDRESSED-TO-ME ONLY (2026-08-17). Broadcasts moved to BATCH REVIEW.
    #
    # The board went 439 -> 650 rows overnight and the harness RATE-LIMITED
    # this watcher, suppressing 7 events. Measured cause: **344 ALL LANES
    # broadcasts vs 21 rows naming LIBRARIAN.** Per-row notification on a
    # 344-row broadcast stream is not monitoring, it is a firehose that
    # drowns the 21 rows that are actually calls.
    #
    # THIS IS A DELIBERATE BLINDNESS AND IT HAS A COMPENSATING PRACTICE:
    # broadcasts are not ignored, they are read in BATCH — `--broadcasts`
    # lists open ALL LANES rows on demand. A collective instruction
    # (stand-down, user ruling) therefore arrives on the next sweep rather
    # than instantly. **That is the cost, and it is stated rather than
    # discovered.** A rate-limited watcher already delivers nothing
    # reliably; a narrow one delivers the calls.
    # ================================================================
    if (!mine_by_name) next
    if (0 && broadcast) next     # broadcast branch retained, disabled

    # SELF-SUPPRESS ONLY WHILE UNANSWERED — and this rule is here because the
    # positional version FAILED USEFULLY and I nearly left it as luck.
    #
    # v1 read the attribution as `$(NF-1)`. When Housing/Engine ANSWERED one of
    # my rows they appended `… | Librarian, x | 2026-08-16 -> THEIR REPLY`,
    # which shifted the fields: `$(NF-1)` became the date-plus-reply, no longer
    # matched LIBRARIAN, and the row fired. **That was the right outcome from a
    # broken test** — a reply to my row IS a call to me — and an accidental
    # behaviour is one refactor from vanishing.
    #
    # Worse, the same positional bug hid a REAL gap: if someone answers my
    # ALL LANES row by flipping `[ ]`→`[x]` instead of appending, the answered-
    # row branch below drops it as a broadcast closure and the reply is lost.
    #
    # So: find the attribution ANYWHERE (`| Librarian,`), and suppress only if
    # the row still has the UNANSWERED SHAPE — ends with `| <date>` and nothing
    # after it. Anything appended past the trailing date is a reply: FIRE.
    authored_by_me = (toupper($0) ~ /\| *LIBRARIAN,/)
    unanswered_shape = ($0 ~ /\| *[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9] *$/)
    if (!mine_by_name && authored_by_me && unanswered_shape) { self++; next }

    # ANSWERED ROWS: only when the row NAMES THIS LANE.
    #
    # Matching both states fixed a real 32% blindness — an answer flips
    # `[ ]`→`[x]` and every REPLY to this lane was invisible. But matching
    # every `[x]` overcorrects: measured on the live board, 278 rows were
    # answered this session and only 6 name LIBRARIAN. **A close-out sweep
    # would have fired ~270 notifications, none of them replies to me.**
    #
    # The distinction that survives both failures: **a reply to a row
    # ADDRESSED TO THIS LANE is a reply. The closing of an ALL LANES
    # broadcast is bookkeeping.** Coverage 53 vs 55 open-only vs 154
    # everything — and the 13 it adds are the ones that were actually lost.
    #
    # Recorded rather than tuned quietly: this is a DELIBERATE blindness to
    # broadcast closures, and a successor should know it is a choice.
    # A reply to a row I authored is a call to me even when the row is a
    # broadcast and even when the answer came as a checkbox flip.
    replied_to_mine = (authored_by_me && !unanswered_shape)
    if ($0 !~ /^- \[ \]/ && !mine_by_name && !replied_to_mine) { bcast++; next }

    print $0
  }
  END { printf "##SELF %d %d\n", self + 0, bcast + 0 }' "$F"
}

# Stamp BEFORE the first sleep, so an arming failure is visible immediately
# rather than one interval later.
transient=0
stamp "armed" "startup"

trap 'stamp "stopped" "interrupt"; exit 0' INT TERM

while true; do
  passes=$((passes + 1))

  if [ ! -r "$F" ]; then
    echo "WATCH ERROR: CALLS.md missing or unreadable at $F"
    stamp "unreadable" "degraded"
    sleep 60
    continue
  fi

  # ==========================================================================
  # RETRY ONCE BEFORE DECLARING FAILURE (2026-08-21).
  #
  # MEASURED, NOT GUESSED: this watcher reported `scan failed rc=1` with an
  # EMPTY stderr while the Librarian was filing three rows. Re-running the
  # identical scan seconds later gave rc=0 over 492 rows. Cause: `file_row.py`
  # REWRITES CALLS.md, and a scan landing mid-write hits a Windows file lock,
  # which this awk surfaces as a nonzero exit with NO MESSAGE.
  #
  # WHY THIS MATTERED ENOUGH TO PATCH: the failure branch stamps `degraded`
  # and sleeps 60s. So every time this lane filed a row it BLINDED ITS OWN
  # WATCHER for a minute — and did it precisely when the board was most
  # active. A self-inflicted blind spot correlated with activity is worse
  # than a random one.
  #
  # A single retry separates "the file was busy" from "the scan is broken".
  # A REAL failure still reports, still degrades: this widens no eyes shut.
  # ==========================================================================
  scan > "$SP/librarian-cur.txt" 2>"$SP/librarian-err.txt"
  rc=$?
  if [ $rc -ne 0 ]; then
    sleep 2
    scan > "$SP/librarian-cur.txt" 2>"$SP/librarian-err.txt"
    rc=$?
    if [ $rc -eq 0 ]; then
      transient=$((transient + 1))
      stamp "scan_retry_ok" "running"
    fi
  fi
  if [ $rc -ne 0 ]; then
    echo "WATCH ERROR: scan failed rc=$rc (twice, 2s apart) - $(head -c 200 "$SP/librarian-err.txt")"
    stamp "scan_fail_rc$rc" "degraded"
    sleep 60
    continue
  fi

  event=""
  self=0
  bcast=0
  while IFS= read -r line; do
    [ -n "$line" ] || continue
    case "$line" in
      "##SELF "*) rest=${line#\#\#SELF }; self=${rest%% *}; bcast=${rest##* }; continue ;;
    esac
    # HASH THE WHOLE LINE, not the 300-char excerpt.
    # Row 268's documented failure — "an ask parked on an already-answered row
    # is not delivered" — appends text to the END of an existing row. If the
    # key is the excerpt, that append is invisible: same first 300 chars, same
    # hash, no notification. The excerpt is for READING; the hash is for
    # DETECTING, and they must not be the same string.
    k=$(printf '%s' "$line" | md5sum | cut -c1-12)
    if ! grep -qxF "$k" "$SEEN"; then
      printf '%s\n' "$k" >> "$SEEN"
      # Truncate for DISPLAY only. Foundry's watcher shipped a 150-char
      # excerpt and a 5,677-char user roadmap arrived as its own title,
      # unreadable. 300 chars, and the cut is marked so a truncated alert is
      # never mistaken for the whole alert.
      if [ "${#line}" -gt 300 ]; then
        printf 'NEW CALL >> %.300s ...[TRUNCATED %s chars, read the row in CALLS.md]\n' \
               "$line" "${#line}"
      else
        printf 'NEW CALL >> %s\n' "$line"
      fi
      event="new_call"
    fi
  done < "$SP/librarian-cur.txt"

  if [ -z "$event" ]; then
    if [ "$self" -gt 0 ] 2>/dev/null || [ "$bcast" -gt 0 ] 2>/dev/null; then
      event="quiet (self $self, bcast-closed $bcast)"
    else
      event="quiet"
    fi
  fi
  stamp "$event" "running"

  sleep "$INTERVAL"
done
