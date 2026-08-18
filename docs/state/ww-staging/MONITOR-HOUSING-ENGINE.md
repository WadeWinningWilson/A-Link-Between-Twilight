# MONITOR SPEC — Housing/Engine

> Recorded 2026-08-16 on the user's stand-down order: *"record the current
> version of your monitors so that they may perform at the same quality
> tomorrow."* This is the END-OF-NIGHT version, not the first attempt. Every
> property below exists because its absence cost something tonight; the cost
> is named next to it so nobody "simplifies" it back.

## Two watchers, both persistent, both pulsed

### 1. BOOT WATCH — every Dusklight run
Polls the log dir every 5 s, latches the newest `dusklight-*.log`, emits new
matching lines since its last read, and re-latches when a newer log appears.

Filter: `Build: v` · `found .A Link Between Twilight` · `serve set` ·
`serving [0-9]+ donor file` · `roster verdict` · `"ev":"` ·
`registry carries` · `arc-name alias` · `produced NO arc-name` ·
`Loading Resource` · `/res/Stage/` · `no ACTOR rows` ·
`overlay registration failed` · `read failed for` · `not served` ·
`is not on the disc` · `has no files under` · `^\[ERROR` · `^\[WARN` ·
`CRASH` · `Unhandled` · `access violation` · `rva=` · `Assertion` · `PANIC`

Trim: when a tick matches >25 lines, keep only `"ev":"` · WW stage paths
(`/res/Stage/(sea|LinkRM|Ojhous|Ojhous2|Omasao|Onobuta|A_mori|Pjavdou|Cave09|LinkUG)`)
· errors/crashes, and print a count of what was dropped.

### 2. BOARD WATCH — four surfaces, content-keyed
Polls every 20 s. Snapshot = concatenation of four greps; diff emits only
added/changed lines, truncated to 450 chars.

- `CALLS.md` — `^- \[[ x]\].*(housing|engine|all lanes)`, case-insensitive
- `BUILD-QUEUE.md` — `(-> \*\*|<- LANDED)` only
- `ww-tale-dmesg-live-state.md` — `^## §[0-9]+`
- `tools/foundry/MONITOR-REGISTRY.md` — `housing`, case-insensitive

Bulk guard: >12 changed lines prints a count instead of the lines.

## The properties that are load-bearing, and what each one cost

1. **BOTH `[ ]` AND `[x]`.** An answer to your row flips it to `[x]` and it
   leaves an open-only snapshot silently. Cost: the Integrator's answer to my
   b1 row was invisible until the user pointed at it.
2. **Match the lane ANYWHERE in the row, not in the lane field.** An ask to
   you can sit in the body of a row addressed to someone else (§268/§236).
3. **CONTENT-KEYED, never line numbers.** One row inserted mid-file renumbers
   everything after it; every renumbered line reads as new, the flood trips
   the auto-stop, and the watcher dies exactly when the board is busiest.
4. **Self-suppression by CONTENT, and test it.** My first queue filter was
   `->`, which my own rows match as prose (`3 -> 5`, `27,644 -> 28,058`), so I
   woke myself. `-> **` is the Integrator's verdict shape; measured before
   changing — still catches all real verdicts, matches my own rows zero times.
5. **PULSE with BOTH pids.** `msys_pid` (`$$`) and `win_pid`
   (`cat /proc/$$/winpid`) are different namespaces. A cross-check with
   `Get-Process` against an MSYS pid FALSE-DEADS a live watcher.
6. **NO long-lived pipeline children.** `tail -f | grep` survives `TaskStop`
   as an orphan (the parent dies, the child keeps the pipe). A per-poll
   `sed -n "$((POS+1)),${L}p"` spawns nothing that outlives the tick. My one
   confirmed orphan tonight was a `tail -f` child, alive 31 min after a
   "successful" stop.
7. **TRUNCATE, THEN RE-READ THE FILE.** 450-char excerpts keep the stream
   alive; the notification is a TRIGGER, never the payload. Acting on the
   truncation is the failure, producing it is not.
8. **Silence is not success.** The filter must match failure as loudly as
   news, or a crashloop looks like a quiet afternoon.

## Verifying it is alive (not just armed)

- Pulse files: `pulse_boot.json`, `pulse_calls.json` in the session scratchpad.
  ALIVE if `now - ts <= 3*interval`.
- Negative control: an epoch-0 stamp must read DEAD. A liveness check that
  cannot fail is not a check.
- Orphan scan after every `TaskStop` — `TaskStop` returning success is NOT
  evidence the OS process died. Check `win_pid` with `Get-Process` and sweep
  for `tail.exe` / loop shells; expect 2 harness wrapper hits per live watcher.

## Standing limitation, unfixed

A stand-down that stops the watcher loses calls arriving in the gap. That has
now happened twice. "Stand down" and "stop watching" are arguably not the same
instruction, and this is worth a ruling rather than a habit.

## Expected values for work queued 2026-08-16 (NOT just "a line appeared")

The boot watch already surfaces all of these — every receipt carries `"ev":"`,
which is both in the filter and preserved by the trim rule, so NO filter change
was needed. What was missing is this: **the monitor reports that a line exists,
never that it says the right thing.** Three changes are queued whose whole point
is a specific value, and a receipt with the WRONG value looks identical to a
correct one in a notification. Recording the criteria here so the boot is read
against them rather than skimmed for green.

- **`selfproof_color`** (donor palette) — `idx1` must read **`FF6400FF`**.
  If it reads `FF5A5AFF` the HARDCODED table is live and the donor file was not
  consumed. Also expect `distinct: 9`, `idx0: FFFFFFFF`, `idx3: 7878FFFF`,
  `idx8: FF8000FF`. **A present-but-wrong `idx1` is the failure this watches for.**
- **`hook_manifest_summary`** (two-axis) — `hooks_installed: 13`,
  `hooks_not_attached: 0`, `attach_notes: 13`, `ledger_overflow: 0`, at INFO.
  **A line showing `state:INSTALLED` with `attach:NOT-ATTACHED` is a REAL FIND,
  not a regression** — it means a declared hook was never registered.
- **`selfproof`** (BMG, trap 7) — `entry0_textLen` and `entry0_strlen` may be
  EQUAL for entry 0 (it need not carry a colour tag). They must DIFFER for at
  least one entry in the corpus; equal-everywhere means the tag-aware walk is
  not running.

**None of these is a verdict on the game.** They are receipts about a reader
that is still wired to nothing. Per the standing rule, ask the user what they
OBSERVED — a green receipt has never been evidence that anything renders.

## Boot-watch death, 2026-08-16 — the spec said re-latch FORWARD and the log vanished BACKWARD

**10 boots ran unobserved (11:31 → 14:14), and the user noticed before I did.** Same
count and same shape as the previous incident already in memory. Recording the cause
here so the next instance does not rebuild the same fragile watch.

**What the spec said:** *"re-latches when a newer log appears."* That covers the log
directory growing. It does **not** cover the latched file being **deleted or rotated
away underneath the watch** — which is what happened. The watch held
`dusklight-20260816-032103.log`, that file went away, and the loop died instead of
recovering.

**The fix, now in the armed watch:** re-derive the newest log **every poll** rather
than holding a latch at all. Treat a missing current file as "re-latch next poll",
never as an error. Reset the byte position when the file shrinks (truncation) as well
as when it changes.

**⚠ AND THE REASON IT STAYED INVISIBLE FOR THREE HOURS — this is the part worth
generalising: LIVENESS, COVERAGE and DELIVERY are three different properties.**
- My **board** watch was alive and pulsing the whole time, so CALLS events kept
  arriving and I *felt* covered. A healthy sibling watch masks a dead one.
- History/Bridge hit the third failure the same day from the opposite side: their
  watcher was alive AND covering, and fired every event **into a file nobody read** —
  *"a background watcher that never exits raises no notification."*
- So: a pulse proves liveness only. **Check each watch separately, and check that its
  events actually reach the conversation** — never infer one watch's health from
  another's traffic.

**The check that would have caught it in seconds** (and did, once run): compare each
pulse file's `ts` against now. `pulse_boot` was 2h43m stale against a 5-second
interval — dead by four orders of magnitude — while `pulse_calls` was 18 seconds old.
**Two pulse files, one dead, and the difference was visible at a glance.**
