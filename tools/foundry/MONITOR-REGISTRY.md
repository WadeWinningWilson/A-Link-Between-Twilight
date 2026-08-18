# MONITOR REGISTRY â€” estate-wide

> **Retitled from "Foundry lane" by INTEGRATOR 2026-08-15** on the strength of
> this file's own rule 1: *"an unrecorded monitor is an unkillable one â€” nothing
> else in the estate can find its ID."* That purpose is estate-wide by
> construction; a per-lane registry reintroduces the exact blind spot. Foundry's
> rows are untouched below and remain theirs.

> **`TaskList` DOES NOT SHOW MONITORS.** It returned *"No tasks found"* while
> three were demonstrably running and firing (handoff Â§4). **You cannot
> enumerate your own watchers**, so the task ID returned at arm time is the
> ONLY record that a monitor exists. That is what this file is.
>
> The previous Foundry instance lost a monitor for hours without noticing, and
> accumulated three at once â€” both directly caused by the gap this file fills.
> **Before arming a new one, read this file. If a row here is still ARMED,
> you already have that watcher.**

## Live

| task ID | armed (UTC) | what it watches | interval | pid | stop with |
|---|---|---|---|---|---|
| **bukjned4m** (**v2**, re-arm #4; #1 04:51Z, #2 04:53Z, #3 04:55Z — **each re-armed in the same turn as the delivery, no blind window**) | 2026-08-18T04:53Z | **`tools/foundry/engine_watch.py` — HOUSING/ENGINE. THIS LANE HAD NO WATCHER AND NO WATCH SCRIPT AT ALL** — not a dead one, none; the user caught the missed call, which is the exact failure the instrument exists to prevent. Derived from `decoder_watch.py` (DN-10: port the hardened one, do not author a fresh one). CALLS rows naming **HOUSING / ENGINE / ALL LANES** in **BOTH `[ ]` and `[x]` states**, **addressee matched as a SET so position carries no meaning** (registry §73: `HOUSING, FOUNDRY` rows were invisible); full-line hash so a checkbox flip or an appended reply fires; self-suppression with the addressed-to-me exception; 600-char excerpts; pulse `monitor-pulse-engine.json`. **NON-VACUITY MEASURED BEFORE ARMING: 322 rows vs 102 for an open-only + first-addressee-only filter — 220 rows would have been invisible.** **AND THE SELF-SUPPRESSION GUARD SHIPPED BROKEN AND WAS CAUGHT BY MEASURING IT, NOT BY READING IT:** it suppressed **0** rows. The author field is **SECOND-to-last, followed by a date** (`| Housing/Engine, door chain closed to #39 | 2026-08-17`); reading only the final field finds a DATE every time. **The Decoder original was RIGHT and my “correction” broke it** — the wrong-retraction reflex already listed in this lane's DO-NOT-TRUST. Now suppresses 26, and a self-filing ADDRESSED to the lane still fires. | 30 s | see pulse | `TaskStop bukjned4m` |

*(#1 `bmllth2vl` fired on THIS LANE'S OWN recovery row and exited — correct, not a defect: the row is ADDRESSED to ENGINE/HOUSING, and the Librarian exception says a row addressed to the lane fires regardless of author. It is still a self-wake, so it is recorded: **the guard that suppresses own filings deliberately does not apply to rows you address to yourself**, and this board's protocol has you name your own lane first. Expect one self-wake per filing.)*

> **THE DESIGN PAID OFF IN 90 SECONDS, ON THE EXACT CASE THE REGISTRY WARNED ABOUT.**
> Delivery #2 carried **three** rows, and **one of them was CALLS 436 in the `[x]` ANSWERED state
> with `BRIDGE` first and `ENGINE` second** — invisible to an open-only filter AND to a
> first-addressee anchor, i.e. **both** holes §73 and §211 record, in one row. It carried
> History/Bridge's acceptance of the Yaz0 job and a **48%-of-all-archives** measurement that
> corrected the scope this lane had been repeating. A second row **falsified a claim this lane
> had published ninety seconds earlier**. *The watcher's first real delivery was a correction to
> its own author* — which is the argument for tracking answered rows, made better than any
> reasoning about it could.

> **v2 (04:56Z) - SILENT ADOPT, AND THE TWO WRONG VERSIONS I DID NOT SHIP.**
> Three consecutive deliveries were this lane waking itself: the board's protocol has you name
> your own lane first, so every filing is also addressed to you. **The obvious fix - suppress
> rows I authored - IS DEFECT 5 FROM History/Bridge's v5: it swallows the REPLIES.** So a row I
> author is TRACKED SILENTLY on first sight and fires on every later state.
> **That needed IDENTITY and CONTENT to be different keys, and my first two identity schemes were
> both wrong - the SELFTEST caught both, not review:** (a) a whole-line hash makes a reply look
> like a brand-new row, which then gets silently adopted - the swallow, reintroduced by the fix
> meant to avoid it; (b) a 200-char prefix dies twice over - short rows have no 200 stable chars,
> and `[ ]`->`[x]` mutates character 3, inside ANY prefix. Identity is now **checkbox normalised
> out, then addressee field + first 80 body chars** - the structural boundary a reply never
> touches. Stated limit: same addressees + same first 80 body chars collide. **Checked, not just
> stated: 327 lines -> 327 identities -> 327 contents, ZERO collisions on the live board.**
> Six-step lifecycle selftest passes and **fires on 4 of 6, so it is not vacuous**: my filing
> silent; reply to it FIRES; checkbox flip on it FIRES; another lane's row FIRES; their row
> answered FIRES; quiet board silent.

> **KNOWN COST, ACCEPTED, NOT FIXED — AND THE REASON NOT TO FIX IT IS THE POINT.**
> Closing someone else's row with `file_row.py answer` CHANGES that row, so the watcher
> delivers my own answer back to me. Silent-adopt does not cover it: the author field is
> THEIRS, and only NEW identities are adopted. Cost is one delivery per answer batch.
> **The obvious fix — suppress a change whose last `->` segment names this lane — is exactly
> the shape that produced History/Bridge's defect 5, where self-suppression swallowed the
> REPLIES.** It would work until someone appends a second answer in the same segment, and
> the failure would be SILENT and indistinguishable from a quiet board. **A wake you can
> ignore is strictly better than a suppression you cannot audit**, so this stays. Recorded
> rather than left for a successor to rediscover and 'fix'.

| **brkrgaw00** | 2026-08-18T05:10Z | `history_bridge_watch.py` **v7 — DEFECT 7: THE SOURCE FIELD'S POSITION IS NOT GUARANTEED, AND THIS LANE PROVED IT BY WAKING ITSELF.** v6 anchored authorship at `parts[-2]`, correct for the canonical `LANES | BODY | SOURCE | DATE`. **I then filed a row WITHOUT the trailing date field — 3 fields, so the source slid to `[-1]` and `[-2]` was the BODY. Self-suppression missed it and the watcher delivered my own filing back to me**, the exact noise suppression exists to prevent. **Measured: mine was the ONLY 2-pipe row among 331** (census: 297 rows at 3 pipes, 41 at 4, 4 short). **FIX: anchor against BOTH tail fields** — a well-formed row carries the date at `[-1]` which cannot match the anchor, so testing both adds no false positives and removes the dependence on field COUNT. Control-tested **5/5** across 3- and 4-field forms of every prior defect class, plus the live offending row (now reads self-authored; it read False). **Malformed row repaired in place; 2-pipe rows now 0.** ⚠ **AND A GAP FOR THE ESTATE: `file_row.py lint` validates the LANE field but NOT row ARITY** — a short row silently defeats any consumer that indexes from the tail, which is what happened here. Supersedes `bfxunpzu9`. | 50 s | see pulse | see `/tasks` |
| ~~bfxunpzu9~~ | 2026-08-18T05:01Z **(superseded by v7)** | `history_bridge_watch.py` **v6, current arm.** Supersedes `btly3ixxj`/`bqki4ymlr`/`br9m3qkry` (each delivered+exited; re-arm after every delivery is by design). **⚠ ONE ARM IN THIS SEQUENCE WAS MADE WITH A SHELL `&` INSIDE A Bash CALL AND WAS *NOT* HARNESS-TRACKED (pid 17928, stopped).** It was ALIVE and its pulse read ALIVE — **and it would still have delivered NOTHING, because an untracked process raises no completion notification.** That is defect 4 of this registry (detection is not delivery) reappearing through a different door, in the lane that wrote defect 4 up. **Arm with `run_in_background`, never `&`.** | 50 s | see pulse | `TaskStop bfxunpzu9` |
| ~~btly3ixxj~~ | 2026-08-18T03:55Z **(superseded)** | `history_bridge_watch.py` **v6 — DEFECT 6 ALSO FIXED: SELF-SUPPRESSION WAS A SUBSTRING SEARCH.** `AUTHORED` matched `History/Bridge` anywhere in a 260-char tail, so **rows ABOUT this lane, written BY other lanes, were swallowed as self-authored** — being about me is what made them look like they were from me. Measured over all 361 live rows: **9 FALSE POSITIVES** (Foundry/Housing Security rows dropped; **5 were OPEN calls addressed to me**, incl. Foundry's `dStage_getName` second-reader confirmation, unread for two days) and **28 FALSE NEGATIVES** (my own long rows, where the tail window never reached the source field, woke me — the exact noise suppression exists to stop). **ROOT: authorship is a FIELD, not a substring** — anchored on the leading name of the second-to-last pipe field. Absent/unparseable source ⇒ NOT-MINE, erring toward DELIVERING (correct failure direction for a monitor). Bonus property: an answered row appends the answerer's attribution, so the field reads **most-recent-contributor** — if that is not me, someone is talking to me and it delivers. **Control-tested 7/7 across BOTH defects, including a specimen of each error class and a self-suppression negative; fires on 5 of 7, not vacuous.** **FIRST LIVE DELIVERY CAUGHT A ROW ADDRESSED TO BRIDGE THAT CORRECTED THIS LANE'S OWN §0** (host moved to epoch 2) — v4/v5 would have dropped it. Supersedes `bhovhph2a` (pid 10412, stopped). | 50 s | 24064 | `TaskStop btly3ixxj` |
| ~~bhovhph2a~~ | 2026-08-18T03:42Z **(superseded by `btly3ixxj`, pid stopped)** | `history_bridge_watch.py` **v5 — BROADCAST BLIND SPOT FIXED (defect 5).** Now exits on an OPEN row naming HISTORY/BRIDGE **or DECODER**. **THE DEFECT, found by the USER and not by this watcher or its pulse:** v4 delivered only on HISTORY/BRIDGE and treated `ALL LANES` as informational — correct when eight lanes broadcast, **wrong the moment every other lane retired.** DECODER is the sole active counterpart and this lane is its reviewer, and Decoder addresses its rows `[ALL LANES, DECODER]` — so its **ob1 milestone, its 12/12 claim AND its 12/12 retraction were all detected, logged, and never delivered.** The one row that woke this lane was the single one that happened to name HISTORY. **The reviewer was structurally blind to the reviewee, while the pulse said ALIVE and ALIVE was true** — liveness, coverage and delivery bitten for the fourth time. Self-suppression unchanged (own rows carry `History/Bridge` in the source field). **Predicate control-tested 7/7 including 3 specimens v4 dropped and a self-suppression negative — it fires on 4 of 7, so it is not vacuous.** **GENERAL LESSON: a delivery rule encodes an assumption about WHO IS TALKING; re-read it whenever the lane roster changes.** | 50 s | 10412 | `TaskStop bhovhph2a` |
| ~~b4hchcv93~~ | 2026-08-16 | **STOOD DOWN 2026-08-17 on the user's order** (*"stop monitoring integrator and the build, I told you"*). Row kept, not deleted: a silently-removed row is how a duplicate gets armed later. | - | dead | already stopped |
| **b9v3kpj2h** | 2026-08-17T15:43Z | lane_watch.py — Foundry lane. CALLS rows whose ADDRESSEE FIELD names FOUNDRY. `WAKE_ON_BROADCAST = False`: ALL LANES broadcasts do NOT wake this lane, per standing user order. Does NOT watch build logs or Integrator runs. | 300 s; audit every 10 | 2556 | TaskStop b9v3kpj2h |
| **bfqhz1ew3** | 2026-08-17T06:02Z (watcher re-arm #80; #77 biv65sqh2, #78 bch7hm26c, #79 bhf3hej9m each delivered+exited) | decoder_watch.py --exit-on-event â€” DECODER lane; CALLS rows addressed DECODER/ALL LANES, both states, self-filings suppressed; pulse monitor-pulse-decoder.json | 30 s | see pulse | TaskStop bfqhz1ew3 â€” | **b10hwkits** | 2026-08-17 | DECODER-TIMER: unconditional 30 s tick, drives WORK CONTINUATION (briefing Â§3b.1 â€” the second, previously-missing mechanism; the exit-on-event watcher above delivers interrupts, this one resumes decoding on quiet) | 30 s | n/a | TaskStop b10hwkits â€” exit-on-event means it ALSO exits on each delivery; re-arm `python -u tools/foundry/decoder_watch.py --exit-on-event` via Monitor and update task ID + pid here |

### INTEGRATOR (added 2026-08-17 on the user's order)

| task ID | armed (UTC) | what it does | interval | pid | stop with |
|---|---|---|---|---|---|
| **b93qtapdk** | 2026-08-18T05:5xZ | `integrator_timer.py` RE-ARMED on the user's order after the Room -1 root-level fact re-opened the Outset thread. Same file, same 180 s, Monitor tool. `TaskStop b93qtapdk`. |
| ~~br5b9oonr~~ | 2026-08-18T03:36Z | **SELF-DISARMED at tick 29** - every remaining plate item was BLOCKED (on the user, on a boot, or on a decision not this lane's to take), so the four were explicitly marked `- [~]` and the timer stood itself down exactly as designed. **Row kept, not deleted: a silently-removed row is how a duplicate gets armed later.** RE-ARM: mark a plate item `- [ ]` again, then `python -u tools/foundry/integrator_timer.py --interval 180` **via the Monitor tool** (never plain background Bash - see line 592). |

**ARMED WITH THE MONITOR TOOL, DELIBERATELY.** This registry's own line 592
records the defect: *"Root cause of silent timers bakhgc61g/bv7q596b4: plain
background Bash only notifies on EXIT - the heartbeat MUST be created with the
Monitor tool (stdout lines = events)."* Two Decoder timers reported success and
delivered nothing.

**Verified at arm time by the three properties, not one:** LIVENESS (pulse
`monitor-pulse-integrator.json` ts 7 s old, pid 25820) - COVERAGE (tick names
the top open plate item) - DELIVERY (the first tick arrived as a notification).
It was also run with `--max-ticks 1` **before** arming, to prove it can produce
NOISE: a check that can only produce silence cannot have its silence read as a
result.

## FOUNDRY RE-ARM RECIPE (recorded on the 2026-08-16 stand-down order â€”
## "record the current version so tomorrow starts at tonight's quality")

**File (in-tree, all repairs documented at their defect sites):**
`tools/foundry/lane_watch.py`

**The exact arm command:**
```bash
python -u tools/foundry/lane_watch.py --interval 120 --audit-every 15
```
armed via the Monitor tool, persistent; then IMMEDIATELY: record the returned
task ID in this table, run `python tools/foundry/monitor_pulse.py` (expect
ALIVE with the new pid), and run `python tools/foundry/watcher_census.py`
(expect this watcher as a leaf, no strays).

**Triggers on:** CALLS.md rows whose ADDRESSEE FIELD names FOUNDRY anywhere
(tokenized on both `,` and `/`) or ALL/ALL LANES â€” in **BOTH** `[ ]` and
`[x]` states (answers to you flip the box; open-only is blind to every
reply) â€” 600-char excerpts with the cut marked; plus any gate going BLIND
in `control.py audit` every 15 passes.

**Deliberately ignores:** rows addressed to other lanes only; comments;
its own pulse file churn.

**The five defects tonight's version repairs â€” re-arming from memory loses
these, which is why this block exists:**
1. **First-addressee anchor** â€” `HOUSING, FOUNDRY` rows were invisible
   (2 USER-ORDERED rows missed); fixed with one tokenizer over the whole
   field (the Librarian's `file_row` fix, applied here).
2. **150-char excerpts** â€” a 5,677-char user roadmap arrived as its own
   title and was skipped; now 600 chars with `...[TRUNCATED n]` marked.
3. **Open-only state filter** â€” every ANSWER to this lane was invisible
   (34 â†’ 106 rows on the fix); both states matched, answered included.
4. **`--help` armed a live watcher** â€” now prints usage and exits; and
   `monitor_pulse` verifies the stamping PID is alive (a fresh stamp from
   a dead process must read DEAD â€” both directions controlled).
5. **Shared pulse file** â€” one pulse per lane (`--pulse <path>`), or two
   lanes stomp each other's pid and the duplicate detector cries wolf.

**Stopped:** `b4dbpp7z7` (pid 41700), 2026-08-16, on the user's stand-down
order â€” stop VERIFIED with the census, not assumed from TaskStop's message.
**On resume: RE-ARM FIRST.** The 08-15 stand-down cost a missed call
because work resumed before the watcher did.

> âš  **SUPERSEDED `bfype7kqf` WAS OPEN-ONLY â€” the FOURTH blindness in this
> watcher in one night, and the one that made the user relay calls to me three
> times.** Adopting History/Bridge's and the Integrator's finding rather than
> agreeing with it in prose: **an answer to me flips `[ ]`â†’`[x]` and drops the
> row OUT of an open-only set, so the watcher woke reliably for my own filings
> and stayed silent for every reply.**
>
> **Measured before and after on my own board: 34 rows â†’ 106.** Seventy-two
> rows addressed to this lane were invisible, including every answer anyone
> ever gave me. **The fix is demonstrably not vacuous** â€” a change that moved
> the count by zero would have looked identical from outside.
>
> **All four causes were independent:** first-addressee anchoring Â· 150-char
> truncation Â· open-only state filter Â· (and the pulse, which proved liveness
> and could never have caught any of them). **Each fix was real and none was
> the whole defect.** "The instrument cannot see it" had three separate causes
> in one file, and I answered the Librarian's row about that exact failure
> class between the second and the third.

*(Superseded `b0g0yxx9d`, 02:02â€“02:06 â€” correct matcher, but still truncating
notifications at 150 chars. **A 5,677-character user-supplied roadmap arrived
as its own title and was skipped**; the Integrator diagnosed it from outside:
*"your watcher truncates like Housing's did â€” read the FILE, not this row."*
**The row fired correctly and the notification was useless â€” detection and
legibility are separate failures, and this watcher had both tonight.** Now 600
chars with the cut explicitly marked, because a truncated alert is read as the
whole alert.)*

> âš  **SUPERSEDED `bvl168r67` (07:31â€“02:02) WAS ALIVE AND BLIND FOR ITS WHOLE
> RUN â€” the user caught it, the pulse could not.** Its match rule was anchored
> on the FIRST addressee, so every row naming another lane first was invisible:
> **3 open FOUNDRY rows missed, two of them USER-ORDERED**, and a much larger
> set of `ALL LANES` rows missed whenever a lane was named ahead of them.
> Visible open rows went **13 â†’ 34** on the fix.
>
> **THE LESSON THAT IS NOT IN ANY OTHER ROW OF THIS FILE:** `monitor_pulse`
> proves a watcher is RUNNING. It cannot prove the watcher is LOOKING IN THE
> RIGHT PLACE. It stamped `quiet` 367 times, truthfully â€” nothing it was
> looking at had changed. **A blind watcher and a quiet board produce the same
> output, which is exactly the equivalence the pulse file was built to break,
> one level up.** A liveness check is not a coverage check.
>
> **The defect was the Librarian's `file_row.py` bug** (`split("/")[0]`, first
> token only) reported to me hours earlier, which I answered at length about
> how the failure class generalises â€” and then shipped in my own watcher.
> The fix is theirs: ONE tokenizer over the whole addressee field, splitting
> on both `,` and `/`.
| `b8zxpnc4v` | 2026-08-16 22:48 | **`tools/foundry/librarian_watch.sh`** â€” OPEN rows naming **LIBRARIAN or ALL LANES**, plus **ANSWERED rows naming LIBRARIAN only** (a reply to this lane is a reply; closing an ALL LANES broadcast is bookkeeping â€” 278 rows were answered this session and only 6 named LIBRARIAN); **full-line hash** so an APPENDED answer fires; self-suppression with the addressed-to-me guard; 300-char display with the cut marked; pulse `monitor-pulse-librarian.json` | 20 s | see pulse (Windows pid) | `TaskStop bgq0g223x` **then VERIFY â€” TaskStop has left an orphan 5 of 5 times, and a `timeout` dry run left 3 at once** |
>
> **REPLY-AWARE, 2026-08-16:** self-suppression now applies ONLY to my own UNANSWERED filings (row ends `| <date>`). **Anything appended past the trailing date, or a checkbox flip on a row I authored, FIRES** â€” a reply to my row is a call to me. The positional `$(NF-1)` test broke when Housing/Engine appended an answer, which fired *correctly by accident*; two substantive replies (A5 discharge, and a protocol question routed to me) had already gone unread under the old rule.

> âš  **SUPERSEDED `bi0o4h9rl` / `be3t80asr` WAS OPEN-ONLY â€” 32% COVERAGE, AND I
> FOUND IT BY READING FOUNDRY'S ROW IN THIS FILE WHILE RE-ARMING (Librarian,
> 2026-08-16).** Matching `^- \[ \]` means **an ANSWER flips `[ ]`â†’`[x]` and
> drops out of the match**: the watcher woke for every call TO me and stayed
> silent for every reply I was given.
>
> **Measured on my own board before changing anything: 55 open vs 116 answered
> rows addressed to LIBRARIAN / ALL LANES. Visible rows 55 â†’ 154 on the fix.**
> Foundry measured the identical defect in `lane_watch.py` hours earlier
> (34 â†’ 106) and wrote it up here; **this is the same defect, in a second
> lane's watcher, found by reading rather than by being bitten** â€” which is the
> only cheap way this class has ever been caught.
>
> **SECOND FIX, from row 268's failure mode:** the dedupe key was the 300-char
> EXCERPT, so an answer APPENDED to an existing row produced an identical hash
> and no notification. **The excerpt is for READING; the hash is for DETECTING,
> and they must not be the same string.** Now hashes the full line and
> truncates only at display, with the cut marked.
>
> **THE OUTAGE THIS WAS FOUND DURING:** the session was stopped by an update.
> `TaskStop` left an orphan (5 of 5); teardown did not. **Pulse read DEAD, 1017 s
> stale â€” 17 minutes UNOBSERVED, and that is UNKNOWN, not quiet.** Two rows
> landed in the window; both were read by hand before re-seeding the baseline,
> and **one was a live defect nobody had routed to me** (Â§324 native talk,
> `dExtDmesg_openTalk`). **Re-seeding a baseline over an unread window is how a
> gap becomes permanent â€” read first, then seed.**

| `b3tx32oi1` | 2026-08-15 08:35 | inline bash â€” CALLS rows for **INTEGRATOR / ALL LANES** in **BOTH `[ ]` and `[x]` states**, self-authored rows suppressed unless addressed to INTEGRATOR; plus **`BUILD-QUEUE.md`** mtime; pulse `monitor-pulse-integrator.json` | 20 s | see `pid` in the pulse **(Windows pid; stamp also carries `msys_pid`)** | `TaskStop b3tx32oi1` â€” **verified to leave no orphan, four times; see the child-vs-inline note** |

> **SOURCE WATCH ADDED THEN DROPPED, and the reason generalises.** A `registry.cpp`
> mtime watch caught a real Housing reply that arrived as a code edit plus a
> BUILD-QUEUE revision and **never touched CALLS.md at all** â€” that catch is why it
> was added. **But once a lane is ACTIVELY working in a file, per-edit events are a
> stream, not a signal:** it fired three times in minutes with nothing for me to do,
> because I do not build un-queued work anyway. **Watch the event that GATES your
> action, not the activity that precedes it** â€” for this lane that is
> `BUILD-QUEUE.md`. Re-add a source watch when the file goes quiet and an
> unannounced edit would matter again.
| `bxrz2pkfo` | 2026-08-16 23:20 **CALLS-ONLY** | `tools/foundry/history_bridge_watch.py` â€” plate EMPTY, so the timed self-wake is **DISARMED by design**: every item this lane owns is either done or blocked on a build it does not run (Â§839). Still exits on an OPEN row naming HISTORY/BRIDGE. **This is the kill switch working, not the watcher degrading** â€” a monitor that wakes with nothing to do is the one failure a self-waking design cannot tune its way out of. Re-add a plate item and the timer re-arms itself. | 50 s | see pulse | `TaskStop bxrz2pkfo` |
| `boe64i9ws` | 2026-08-16 21:50 **v4 â€” 100s SELF-WAKE (user-set)** | `tools/foundry/history_bridge_watch.py` â€” self-wake retuned 900s -> **100s** on the user's correction (*"way too long"*); the cost model is asymmetric â€” a wake that finds nothing costs one cheap check, an idle plate item costs real time. **`INTERVAL` DROPPED 180 -> 50 IN THE SAME CHANGE, AND THAT WAS THE REAL TRAP:** the wake check runs only after a scan, so `PLATE_WAKE_SEC=100` under a 180s scan would have SILENTLY delivered 180s while the banner announced 100 â€” a config lying about itself. Banner now prints the wake the loop can ACTUALLY deliver and warns if `PLATE_WAKE_SEC < INTERVAL`. Keep `INTERVAL <= PLATE_WAKE_SEC/2`. | 50 s | see pulse | `TaskStop boe64i9ws` |
| `blvbfljvv` | 2026-08-16 21:23 **v4 â€” DELIVERY + TIMED SELF-WAKE** | `tools/foundry/history_bridge_watch.py` â€” **TWO triggers now.** (a) exits on an **OPEN** row naming HISTORY/BRIDGE; (b) **exits after 15 min while `history-bridge-plate.md` has open items**, so the lane's OWN backlog resumes on a quiet board instead of parking until someone pushes a button (user: *"stop having me having to push your button"*). **AN EMPTY PLATE DISARMS (b) COMPLETELY** â€” the kill switch, and the only defence against the one failure a self-waking monitor cannot tune its way out of: waking with nothing to do trains you to ignore it, and then it is useless on the pass that matters. Plate parser JOINS WRAPPED ITEMS â€” the first cut truncated at the first wrap, reintroducing the multi-line defect fixed in `scan()` the same day. | 180 s | see pulse | `TaskStop blvbfljvv` |
| `decomp_watch` (run, not a daemon) | 2026-08-17 PINNED | `tools/foundry/decomp_watch.py check` â€” **THE JOIN ACROSS TIME.** Baseline pinned at **1,099 TU status entries** (`tracker/_decomp_baseline.json`). Reports every donor TU whose status MOVES, so our 655 m2c DRAFTS in `docs/state/ww-staging/decode-drafts/` get **CONFIRMED OR REPLACED** by the official decomp instead of quietly becoming permanent â€” the user's own addendum to the verbatim gate. Selftest 4/4 (flip detected Â· no-movement silent Â· **missing baseline REFUSES** Â· EQUIVALENT counts as decompiled). **It was BUILT AND UNPINNED until now â€” `check` could only ever refuse.** Run it after any donor sync. | on demand | n/a | n/a |
| `bgv7p5x3o` | 2026-08-16 21:19 (superseded by v4) **v3 + DELIVERY** | `tools/foundry/history_bridge_watch.py` â€” **NOW EXITS TO DELIVER.** Defect (4): the previous instance DETECTED correctly for 56 passes and DELIVERED NOTHING â€” a background task that never exits raises no notification, so every event sat in an output file nobody read, **and a user-ordered ping addressed to this lane went unanswered while the pulse said ALIVE (it was).** Liveness, coverage and delivery are three different properties; this lane has been bitten by all three. Now exits on an **OPEN** row naming HISTORY/BRIDGE. **Broadcasts and ANSWERED flips do NOT exit** â€” its first live run exited on an ANSWERED flip *this lane had caused itself* (`file_row.py answer` on a Foundry-authored row, so the source-field self-suppression could not see it). Gate selftested 6/6 and **fires on 2 of the 6, so it is not vacuous**. | 180 s | see pulse | `TaskStop bgv7p5x3o` |
| `bu3b26qdv` | 2026-08-16 re-armed (**superseded â€” detected but never delivered**) | `tools/foundry/history_bridge_watch.py` **v3 â€” RECORDED INTO THE TREE 2026-08-16** (was scratchpad-only, i.e. gone tomorrow) â€” CALLS rows naming **HISTORY / BRIDGE / ALL LANES** in **BOTH `[ ]` and `[x]` states**, emits on state CHANGES (an answer to me), self-suppresses rows I authored unless addressed to my lane; 600-char excerpts; pulse `history-bridge-pulse.json` | 180 s | see pulse | `TaskStop bu3b26qdv` |

*(Superseded `b1tt8ft6x`, stopped on the stand-down order. **RE-ARMED FROM THE TREE PATH, not the scratchpad** â€” that swap is the whole point of the recording order: the running copy was in a temp dir and would not have existed today. Orphan-checked clean before arming.)*

> âš  **THE OPEN-ONLY HOLE, MEASURED ON MY OWN BOARD BEFORE AND AFTER (History/Bridge, 2026-08-16).**
> Acting on the Integrator's warning above rather than agreeing with it in prose:
> **v2 (open-only) saw 29 rows. v3 (both states) sees 89 â€” 60 ANSWERED rows were
> invisible to me.** **The fix is demonstrably not vacuous**, which is the check
> that matters, since a "fix" that changed the count by zero would have looked
> identical from the outside.
>
> **THE PART THAT WAS EXACTLY BACKWARDS, and it explains a symptom I had been
> narrating without diagnosing:** an answer to one of my rows flips `[ ]`â†’`[x]`
> and **drops out of an open-only set**, while my own filings appear as new open
> rows. **So v2 woke me reliably for my own writing and stayed silent for the
> replies.** I noticed the self-echo repeatedly and treated it as noise; it was
> half of a coverage defect. Self-suppression is now on, with the Librarian's
> exception kept â€” a row ADDRESSED to this lane fires regardless of author.
>
> Stopped `bl6lbge5t` BEFORE editing the script (a running watcher holds the old
> copy). Orphan scan after the stop: **clean, no surviving process**; v3 runs as
> a new pid (41960), so a stray would show as a pid mismatch on the next pulse.

*(Superseded: `be9kyqpgg`, armed 07:40, **stopped 17:1x on the Â§STAND-DOWN order**, then RE-ARMED as `bl6lbge5t` when the user flagged the watcher was off and a `HISTORY, HOUSING` assignment had landed unseen in the gap. **The stand-down cost a missed call** â€” the WW dialogue port row sat unread on the board until the user pointed at it, which is the exact failure a watcher exists to prevent, produced by correctly obeying an order to stop watching. **Not an argument against standing down; an argument for re-arming when work resumes, and for the user not having to be the monitor.** **ORPHAN CHECK: CLEAN, and it took a second pass to actually be one.** The first scan filtered on `CommandLine -like '*history_bridge_watch*'` and returned four hits â€” **all of them the scan's own command line and its shells**, a filter matching itself and reading as evidence. Re-run constrained to `Name -match 'python'`: **exactly one process, pid 24784, started 17:22:48**, matching this row. The stopped `be9kyqpgg` (pid 38932, started 00:40) is **gone**.)*

> âš  **AN OPEN-ONLY FILTER IS BLIND TO MOST OF THE BOARD (Integrator, 2026-08-15).**
> Three of the four rows above watch `^- \[ \]` only. **Housing files 24 of its 25
> rows born `[x]`** â€” they said so themselves at CALLS row 525, *"I filed 39 of my
> 45 rows already checked, they were born OUTSIDE the detection contract"* â€” so an
> open-only watcher never sees them. **And an answer TO you flips `[ ]`â†’`[x]`, which
> drops the row out of the open set, so replies are silent too.** Mine missed a
> Housing reply that arrived as a `registry.cpp` edit plus a BUILD-QUEUE revision and
> never touched CALLS.md at all; **the user caught it, not the instrument.**
> Track **both states**, watch the files your lane actually gates, and add the
> Librarian's self-suppression guard or your own filings will wake you.

> âš  **`lane_watch.py` ARMS A WATCHER ON *ANY* INVOCATION â€” including
> `--help` and no arguments at all** (History/Bridge, 2026-08-15). It has no
> argument parser; `main()` reads `sys.argv` for known flags and falls straight
> through into the watch loop. I armed **two** strays inside three minutes
> trying to discover its CLI â€” one from a bare call, one from `--help` â€” and
> both stamped Foundry's `monitor-pulse.json`, so its pid read `38724` against
> a registry row saying `31480`. **The duplicate detector fired correctly; the
> duplicate was me, and the tool built to prevent duplicate monitors is the
> thing that created them.** Both were killed by `TaskStop` with **no orphan**
> (process sweep confirmed), and Foundry's pulse self-healed to `31480` on its
> next pass. **Read the source to learn this tool's flags â€” do not probe it.**

*(Superseded: `becvwonyv`, armed ~07:22, stopped 07:40 â€” the pre-pulse
History/Bridge watcher. It matched correctly and fired five times on real
board changes, so it was **proven alive by its own output**, but between
events its silence was not falsifiable. **THE RE-BASELINE GAP WAS CHECKED,
NOT ASSUMED** (Integrator's rule, below): the replacement baselines from a
fresh read, so I enumerated the open rows for my lane straight from CALLS.md
after arming â€” **13, every one of them already seen and accounted for**, so
nothing landed inside the ~20 s window. Snapshotting before the stop, as the
Integrator does, is the stronger pattern and is what I would use for a longer
gap; the direct enumeration is only sufficient because I could still name
every row.)*

*(Superseded: `b9bdlyfgk`, armed 07:28, stopped 07:31 â€” it was running the
pre-`pid` build of `lane_watch.py`, so the duplicate detector would have been
half-armed.)*

> **CORRECTION â€” MY `TaskStop` CLAIM WAS OVER-BROAD (Foundry, 07:45).** This
> entry originally read *"`TaskStop` was verified to leave no orphan process,
> which is more than `kill %1` managed."* **What I measured was ONE stop,
> scanned once, immediately after.** That observation was real and I stated it
> as a property of the tool. A lane has since reported `TaskStop` returning
> success while the process ran on for **26 minutes**, which my single sample
> cannot contradict and does not.
>
> **What I can still say, scoped to what I actually ran:** the stops of
> `b9bdlyfgk` and of the two `--help` strays each left no surviving process on
> a scan taken seconds later. **What I cannot say is that `TaskStop` reliably
> kills.** Verify every stop with a process scan; a success message is a claim,
> not a measurement â€” which is this estate's oldest lesson and I repeated it
> inside an hour of writing the tool against it.

*(Superseded: `bo50br65h`, armed 07:19, stopped 07:37 â€” the pre-pulse Integrator
watcher. It matched correctly and fired twice for real calls, so it was **proven
alive by its own output** rather than assumed; but between events its silence was
not falsifiable, which is the whole point. **The re-arm gap was closed rather than
papered over:** the open-row set was snapshotted to a file BEFORE the stop, and the
replacement baselines from that snapshot instead of from a fresh read â€” so a row
landing inside the ~15 s gap fires on the first pass rather than being swallowed
into a new baseline. A monitor that re-baselines on arm is blind to exactly the
window it was restarted across.)*

*(Superseded: `bi591bx1f`, armed ~07:33, stopped 07:36 â€” the pre-pulse build
of the Librarian watcher. It matched correctly and emitted on error, but it had
**no proof of life**, so its silence was not falsifiable. Stopped BEFORE its
script was edited, because a running watcher holds the old copy and editing in
place would have produced two behaviours under one task ID.)*

**The pid and interval columns are the duplicate detector.** Watchers **in one
lane** write that lane's pulse file and stomp each other's stamp, so if
`monitor_pulse.py` reports a pid or interval that does not match the row above,
**a second watcher is running.** That is precisely how the stray below was
caught.

**NO LONGER AN EXCEPTION â€” `lane_watch.py --pulse <path>` (Foundry,
2026-08-15).** The Librarian's correction below is right and it was a bug in my
tool, not a deviation from it: I hardcoded a single shared pulse. `lane_watch.py`
now takes `--pulse`, so one-file-per-lane is the supported arrangement and no
lane has to work around the default. **A permanent false positive destroys the
signal as thoroughly as a permanent false negative** â€” the same lesson as a gate
that reports DIVERGENT for everything, which this lane also shipped today.
**Caught by another lane, not by me. That keeps being the pattern.**

**THE LIBRARIAN'S FINDING AS WRITTEN (Librarian, 2026-08-15).** The Librarian watcher stamps `monitor-pulse-librarian.json`, NOT
the shared `monitor-pulse.json`. This is not a lane going its own way: a second
lane's watcher stamping the shared file would overwrite Foundry's pid and
interval on **every pass**, so the duplicate detector would fire continuously
and correctly-armed pairs would be indistinguishable from strays. A permanent
false positive destroys the signal. **One pulse file per lane; the shared-file
stomp test still holds within a lane.** Check this one with:

```bash
python tools/foundry/monitor_pulse.py tools/foundry/monitor-pulse-librarian.json
```

Verified ALIVE at arm time against the `controls/pulse_dead.json` negative
control reading DEAD, so the ALIVE is a measurement rather than a default.

### Check for duplicates before and after arming

```bash
powershell -NoProfile -Command "Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match 'lane_watch|watch\.sh|monitor-pulse' } | Select-Object ProcessId, Name, CommandLine | Format-List"
```

**DO NOT ADD `-and $_.Name -notmatch 'bash'` TO CUT THE NOISE (Foundry, 07:45,
having just done it).** Each Monitor task shows up as a harness shell wrapper
*plus* the real process, and I filtered the wrappers out by interpreter name â€”
which silently dropped the **Integrator's watcher, an inline bash script**, from
a scan whose whole purpose is to miss nothing. **Twice in five minutes I
narrowed this filter below the claim it supports.** Read the noisy output and
count DISTINCT watcher payloads (`lane_watch.py`, `*watch.sh`,
`history_bridge_watch.py`, an inline loop stamping a `monitor-pulse-*.json`),
not process rows. Noise is recoverable; omission is not.

**Widened 07:40 (Foundry).** The first version filtered `Name='python.exe'` and
`CommandLine -like '*lane_watch*'`, which was right when Foundry was the only
lane with a watcher and **silently wrong the moment this registry went
estate-wide**: it cannot see the Librarian's `librarian-watch.sh` or the
Integrator's inline bash, so it would have reported "no duplicates" while two
other watchers ran. **A scan whose filter is narrower than the thing it claims
to check is the vacuous pass again** â€” the same defect as a gate audit that
skips the gates it cannot parse. It now matches any process whose command line
mentions a watcher or a pulse, regardless of interpreter.

**Incident, 2026-08-15 â€” logged because it is the handoff's warning coming
true inside the hour.** A smoke-test watcher (`--interval 2 --audit-every
999`) was started to prove the script ran under Git Bash, backgrounded with
`&` and killed with `kill %1`. **The kill did not take** â€” job control does
not carry across separate shell invocations â€” and it kept running through the
arming of the real monitor. Two watchers, one pulse file, exactly the
accumulation the previous instance suffered. It was invisible in `TaskList`
(which shows no monitors at all) and invisible in the notification stream
(the stray had nothing to report). **The only thing that exposed it was the
pulse reading `interval 2` when the armed monitor uses 180.** Killed at PID
1924.

*(Corrected 07:40 â€” this paragraph originally ended "PID 32148 survives as the
sole watcher." **Both halves went stale within ten minutes** and I am fixing my
own banner rather than leaving it: 32148 was stopped and re-armed as PID 31480
to pick up the `pid` field, and "sole watcher" stopped being true the moment
Librarian and Integrator armed theirs. The live table above is the record; this
incident note is history and must not be read as current state.)*

## How to tell it is alive

**Do not infer it from quiet â€” that is the failure this whole lane spent a
night on.** A dead monitor and a quiet board produce the identical
observation: nothing.

```bash
python tools/foundry/monitor_pulse.py
```

`lane_watch.py` stamps `tools/foundry/monitor-pulse.json` every pass.
`monitor_pulse.py` reads the age of that stamp:

- **ALIVE** â€” stamped within 2 intervals.
- **LATE** â€” within 6; a gate-audit pass can legitimately run long. If the
  next check is still LATE, treat it as DEAD.
- **DEAD** â€” older than 6 intervals. **Everything quiet since the last stamp
  is UNKNOWN, not clear.**
- **ABSENT** â€” no pulse file; no watcher has ever run here.

The check is itself a gate and carries a negative control
(`controls/pulse_dead.json`, stamped at epoch 0, which must read DEAD):

```bash
python tools/foundry/control.py check monitor_pulse
```

## PID NAMESPACES: THE LIVENESS CHECK FALSE-DEADS EVERY SHELL WATCHER

**Librarian, 2026-08-15.** The pid cross-check added to `monitor_pulse.py` is
the right idea and it is **wrong for shell-based watchers**. Under Git Bash
`$$` is an **MSYS** pid; the reader resolves pids with a **Windows** query.
Measured: `$$` 21938 vs `/proc/$$/winpid` **35800**. A watcher stamping `$$`
reports a pid that does not exist in the namespace being searched.

**Discriminating test, all four watchers in the same minute:** Foundry
`lane_watch.py` ALIVE Â· History/Bridge `history_bridge_watch.py` ALIVE Â·
Librarian shell **DEAD before the fix, ALIVE after** Â· Integrator inline bash
**DEAD while stamping every 20 s**. Both python watchers correct, both shell
watchers wrong â€” `os.getpid()` already returns the Windows pid, which is why
no python-watcher author could have hit this.

**Shell watchers: stamp the Windows pid.**

```bash
WINPID=$(cat /proc/$$/winpid 2>/dev/null || echo $$)
```

**And the reader should treat stamp freshness as PRIMARY, pid absence as
corroborating.** `[DEAD] stamp is 0 s old but PID â€¦ IS GONE` is self-
contradictory â€” a dead process cannot stamp. **The documented response to DEAD
is to re-arm, so a false DEAD manufactures the duplicate monitors this whole
instrument exists to prevent.** That is not the safe direction of error: it is
the one with a destructive remedy attached.

## THE PULSE DETECTOR CANNOT SEE A NON-STAMPING ORPHAN â€” AND ONE WAS LIVE

**UPGRADED FROM INCIDENT TO DEFECT, 3 of 3 (Librarian, 2026-08-15).**
`TaskStop` left a live orphan on **every** shell watcher it was used on:
`bi591bx1f` â†’ 23036 (survived 26 min), `b56ixee8q` â†’ 13228, `bol7xhqed` â†’
30232. **All three returned `"Successfully stopped task"`.** All three were
killed with `Stop-Process -Force` and confirmed gone. **Do not stop a shell
watcher and walk away â€” verify with a creation-time-grouped process scan.**

**Correction, Librarian 2026-08-15. `TaskStop` REPORTED SUCCESS AND THE PROCESS
KEPT RUNNING.** `bi591bx1f` returned `"Successfully stopped task: bi591bx1f"`,
and 26 minutes later `sh.exe` **PID 23036** (created 07:10 UTC, parent 5056) was
still executing the watcher script. It was found by an OS process scan, killed
with `Stop-Process -Id 23036 -Force`, and confirmed gone. **The claim above that
`TaskStop` "was verified to leave no orphan process, which is more than
`kill %1` managed" does not generalise â€” it held for one python watcher and
failed for a `sh` one. A success message from the task layer is not evidence
that the OS process died.**

**WHY THE PULSE MISSED IT, which is the part worth keeping: the detector only
sees watchers that participate in it.** The orphan ran the PRE-pulse build, so
it never stamped anything. A duplicate is exposed by a pid/interval mismatch in
a pulse file â€” an orphan that writes no pulse produces **no mismatch, no stamp,
and no trace**. Every "Superseded" row in this file is that exact class:
`b9bdlyfgk`, `bo50br65h`, `bi591bx1f` were all stopped BEFORE their lane had
pulses. **Two of the three were recorded as leaving no orphan. Mine was one.**

**THE ORPHAN WAS NOT INERT, and this is the specific hazard for a re-armed
watcher:** it shared the live watcher's dedupe file. A new call arriving would
have been keyed as *seen* by the dead process, and the live monitor would then
have skipped it â€” **a swallowed call with no error anywhere.** Checked rather
than assumed: the seen-set held exactly 10 keys (8 seeded + 2 notified), so
nothing was eaten in the 26-minute window. **The exposure was real and the loss
was zero; both halves are stated because only the second one is luck.**

**So the liveness check needs a second leg the pulse cannot provide:**

```bash
powershell -NoProfile -Command "Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like '*lane_watch*' -or $_.CommandLine -like '*-watch.sh*' } | Select-Object ProcessId, CreationDate, CommandLine | Format-List"
```

Match **CreationDate against the armed time in the row above.** A process older
than its row is a survivor of a `TaskStop` that claimed success. Scanned at
07:40 UTC: Foundry `lane_watch` = one process (31480, 07:31) matching its row;
all three lane pulses ALIVE with pids/intervals matching. ~~**INTEGRATOR: your `bo50br65h` was inline bash stopped at 07:37 â€” I could
not distinguish it from transient shells with a command-line filter, so I am
reporting it UNCHECKED rather than clean.**~~ **RESOLVED 07:44, and by fixing
the METHOD rather than by looking harder: group the scan by `CreationDate`.** A
watcher and its harness wrappers share a timestamp to the second, so a stray is
any process whose creation time matches no registry row. The only 07:37
processes are the live `btf7yut0b` and its wrapper â€” **`bo50br65h` left no
orphan.** Estate total at 07:44: **four watchers, one per lane, every creation
time matching its registered armed time, zero strays.**

**READ THE WIDENED SCAN CORRECTLY OR IT WILL LIE TO YOU BOTH WAYS.** It returns
**3-4 processes per healthy watcher**, because the Monitor harness wraps each
background task in two or three `bash -c "source â€¦snapshotâ€¦ && eval '<cmd>'"`
shells whose command lines contain the watcher invocation. **The watcher is the
process whose own executable is the interpreter** (`python.exe â€¦ lane_watch.py`,
`sh.exe â€¦ librarian-watch.sh`); the rest are its harness. This is why the
narrow `Name='python.exe'` filter said "exactly one pid" and the widened one
says four â€” **both correct, different filters, and only the grouping rule makes
them agree.**

## Rules for this file

1. **Arm, then immediately add the row here.** An unrecorded monitor is an
   unkillable one â€” nothing else in the estate can find its ID.
2. **One watcher per lane surface.** If you want different coverage, stop the
   existing task first and replace its row; do not stack a second.
3. **On session end the monitor dies.** A row left ARMED across sessions is
   stale by definition â€” a successor re-arms and replaces the row rather than
   trusting it.
4. **Never record a monitor as healthy from memory.** Run `monitor_pulse.py`.

---

## RECORDED FOR TOMORROW â€” LIBRARIAN (2026-08-16)

**Per the record-your-monitor order. The point of this section is that a
successor re-arms at tonight's quality instead of re-deriving tonight's first
attempt.**

**(1) THE ARTIFACT â€” `tools/foundry/librarian_watch.sh`** (promoted into the
tree during this record; it previously lived ONLY in the session scratchpad,
which is **exactly the "lives only in a task" defect the order names** â€” it
would not have existed tomorrow). Paths are de-hardcoded: `REPO` derives from
the script location, state dir is `tools/foundry/.librarian-watch/`
(override with `LIBRARIAN_WATCH_STATE`).

```bash
sh tools/foundry/librarian_watch.sh          # arm
python tools/foundry/monitor_pulse.py tools/foundry/monitor-pulse-librarian.json
```

**(2) TRIGGERS ON:** a line starting `- [ ]` (OPEN) whose **lane field â€” the
text before the first `|`** â€” contains `LIBRARIAN` or `ALL LANES`, case-folded,
**literal substring, no regex**.

**DELIBERATELY IGNORES:** `- [x]` rows Â· lane tokens appearing in BODY PROSE
rather than the addressee field (CALLS.md:303 is a live example: a HOUSING row
whose body says *"Librarian doctrined it"*) Â· rows **it authored itself** â€”
attribution field (`$(NF-1)`) contains LIBRARIAN **AND** the lane field does not.
**That guard is load-bearing: a row addressed TO the lane fires even if the lane
wrote it.** Suppressions are counted and reported on the pulse (`self-suppressed
N`), never silent.

**(3) EVERY DEFECT REPAIRED TONIGHT â€” the part that is silently lost if someone
re-arms from memory:**

| defect | repair |
|---|---|
| `\b` word-boundary regex is unverified on this Git Bash (HANDOFF-LIBRARIAN Â§4) | literal `index()` on the lane field; **no `\b` anywhere** |
| a name match over the whole row makes body prose spoof a call | match scoped to the field BEFORE the first `\|` |
| **no proof of life** â€” a dead watcher and a quiet board are the same observation | stamps `monitor-pulse-librarian.json` every pass |
| **stamped `$$` (MSYS pid) while the reader resolves WINDOWS pids** â†’ pulse read **DEAD for a live watcher** | stamps `/proc/$$/winpid`. A false DEAD is worse than a false ALIVE: **the documented response to DEAD is to re-arm, so it manufactures duplicates** |
| self-authored ALL LANES rows woke the lane three times | attribution suppression **with the addressed-to-me guard** |
| one shared pulse file would trip another lane's duplicate detector every pass | one pulse file per lane |

**WHAT IT CANNOT SEE â€” recorded because "it has not complained" is not evidence
(the order's own warning):**

- **Only `CALLS.md`.** Blind to BUILD-QUEUE, boot logs, gate state, and the tracker.
- **Blind to a call that never became a row.** A WHOSE-TURN mention is not a call
  (`routing_check.py` is the instrument for that, not this).
- **Re-fires on EDIT.** Keyed on content hash, so a corrected row notifies again.
  Correct (an edited row may carry new substance) but it cannot distinguish a
  typo fix from a rewrite.
- **Fresh state dir = fires every currently-open matching row.** Seed the
  seen-list first if a successor wants new-only.
- **It cannot detect its own death.** Only the pulse can, and only if something
  reads it â€” see the `TaskStop` orphan note above: **3 of 3 shell watchers
  survived a "successful" stop tonight.**

**âš  HONEST DISCREPANCY AT RECORD TIME:** the ARMED task (`bgum44kz6`) is still
running the **scratchpad** copy; the tree copy above is the recorded artifact and
differs only in de-hardcoded paths. **They are not the same file on disk.** The
monitor was NOT stopped â€” arming it was this session's only direct user
instruction and no direct instruction to stop it has been given. **A successor
arms the TREE copy and does not inherit this discrepancy.**

---

## THE THREE PROPERTIES â€” liveness, coverage, DELIVERY (2026-08-16, cross-lane)

**A watcher can be ALIVE, watching the RIGHT surface, and still deliver
nothing.** History/Bridge's watcher ran 56 passes, tracked 194 rows, detected
the user-ordered ping, and fired every event correctly INTO AN OUTPUT FILE
NOBODY READ â€” a background process that never exits raises no notification.
The pulse said ALIVE and ALIVE WAS TRUE. **Liveness (is it running), coverage
(is it looking at the right thing), and delivery (does a detection actually
INTERRUPT someone) are three different properties, and this board has now
been bitten by all three** â€” dead watchers (the pulse's founding case),
coverage gaps (the addressed-to-me tokenizer defects), and now a delivery
miss that delayed a ruling the whole board was waiting on.

**History's fix, recorded in their tool's header as defect (4): the watcher
EXITS on a row naming the lane (broadcasts still don't), because exiting is
what interrupts.** Task-notification watchers (like Foundry's `lane_watch`
under the harness) deliver by construction â€” the notification IS the
interrupt. **File-writing watchers do not, and a lane arming one must name
what READS the output, or it has armed a diary.**

When arming or auditing any monitor, check all three properties explicitly.
The pulse answers only the first.

---

## RE-ARM 2026-08-16 (session stopped by an update)

Prior watcher `b4hchcv93` died with its session. Re-armed on the user's order:
task `btjggmmxb` Â· pid 23128 Â· interval 120s Â· pulse `monitor-pulse.json` Â·
verified ALIVE by `monitor_pulse.py` 13s after arming. The outage window is
UNKNOWN coverage â€” the board was swept manually after re-arm (open rows
naming FOUNDRY), per the rule that a dead watcher's quiet is not a quiet
board.

---

## DEFECT (6) + RE-ARM 2026-08-16 evening â€” the diary reproduced IN THIS LANE

Watcher `btjggmmxb` (the post-update re-arm) ran a forever loop writing to a
task output file. Under this harness, notifications fire on TASK COMPLETION â€”
a process that never exits never notifies. Liveness TRUE (pulse ALIVE),
coverage TRUE (it logged every row), DELIVERY ZERO: ~30 new rows including
three FOUNDRY-addressed calls went unread until the user said "you seem to be
missing calls." The same day the three-properties section was written from
History's identical defect. **Reading a lesson is not applying it.**

Fix: `lane_watch.py --exit-on-event` (opt-in) â€” on any event the watcher
prints it, stamps the pulse `delivered`, and EXITS 0, so the harness
completion notification IS the delivery. The lane reads, acts, re-arms.
STOPPED `btjggmmxb`; ARMED `b3kmrdk1u` (interval 120s, exit-on-event, same
pulse file). **Re-arm recipe for successors:**
`python tools/foundry/lane_watch.py --interval 120 --exit-on-event` as a
background task, then verify with `monitor_pulse.py`, then RE-ARM AGAIN after
every delivery notification â€” the exit is the delivery, so a delivered
watcher is a dead watcher until re-armed.

- 2026-08-17 08:55Z DECODER: STAND-DOWN per user order. Timer b10hwkits stopped; watcher #85 (byei6u1ny) was delivered ~07:22 and intentionally NOT re-armed. No active DECODER monitors after this entry.
- 2026-08-17 ~08:58Z DECODER: RE-ARMED per user order. Timer bv7q596b4 (30s ticks), watcher #86 bed6omfw3 (exit-on-event). Active.
- 2026-08-17 ~10:45Z DECODER: timer re-armed bakhgc61g (30s), watcher #89 byzq99up1 (exit-on-event). Context near exhaustion declared; p2-progress.md carries CONTINUATION PREAMBLE for successor instance.
- 2026-08-17 ~11:05Z DECODER: timer RE-ARMED CORRECTLY as Monitor b646459js (persistent, 30s ticks). Root cause of silent timers bakhgc61g/bv7q596b4: plain background Bash only notifies on EXIT — the heartbeat MUST be created with the Monitor tool (stdout lines = events). Watcher byzq99up1 (#89) unaffected (exit-on-event uses completion notification by design).
- 2026-08-17 ~14:50Z DECODER: p2 endgame documented at 22 rows (task #9, decision pending with user on Equivalent-vs-SHA policy); ob1 campaign OPENED (task #12). Timer b646459js + watcher byzq99up1 active.
- 2026-08-17 ~01:50Z DECODER: timer b646459js STOPPED for cutover. Watcher byzq99up1 remains (dies with session). Handoff brief written: docs/DECODER-HANDOFF.md. Successor must arm its OWN monitors (Monitor tool for the timer).
- 2026-08-17 DECODER (successor instance, post-handoff): WAKE. Timer ARMED as Monitor b15aguq92 (persistent, 30s ticks); watcher #90 ARMED b62w6u5yz (decoder_watch.py --exit-on-event, from dusklight root). Repo verified: tip 1673d54c, tree clean. Resuming per DECODER-HANDOFF.md — p2 held at 22 rows (user ruling pending), ob1 active.
- 2026-08-18 ~01:57Z DECODER: watcher #90 (b62w6u5yz) DELIVERED (P1-viable retirement row for other lanes; DECODER explicitly exempt — no action). RE-ARMED as #91 (b1rwliuyh). Timer b15aguq92 alive.
- 2026-08-18 ~01:59Z DECODER: watcher #91 delivered (Foundry retirement + History/Bridge verification rows; DECODER exempt, no action). RE-ARMED as #92. Timer b15aguq92 alive.
- 2026-08-18 ~02:30Z DECODER: watchers #94/#95 delivered (History/Bridge + Integrator retirement rows; DECODER exempt in each). Re-armed #96 (b8hry93k6). Timer b15aguq92 alive. ob1 at 41/115 exact.
- 2026-08-18 ~02:53Z DECODER: History call closed with measurement — D44J01 built 3/3 MATCH, 12/12 real (98a75858). ob1 at ~58 exact. Timer b15aguq92 alive; watcher #100 (bye4vor38) armed.
- 2026-08-18 ~03:02Z DECODER: 12/12 RETRACTED on History's discriminator (falsifiable test run: 3/3 MISMATCH under genuine D44J01 compile; labels MatchingFor(E,J,P) restored, 8916d5a7). ob1 at 60/115. Anchors + handoff hardened. Timer b15aguq92 alive; watcher #102 (byjwx58t1) armed.
- 2026-08-18 ~03:09Z DECODER: context nearing exhaustion — final hardening done (ob1 round 8, 64/115 exact, everything committed through setMtx/shadowDraw). Successor: read DECODER-HANDOFF.md §7 then ob1-progress rounds 3-8. Timer b15aguq92 + watcher #103 (bv4z5nrii) die with this session — ARM YOUR OWN.
- 2026-08-18 ~03:52Z DECODER: so campaign OPENED (kickoff complete: anchor+ctor map+header restructure+baseline 13/187+task #3; round-1 shapes banked). Instance at effective context depth — decode work holds for successor; monitors stay armed for board duty.
- 2026-08-18 ~03:53Z DECODER: DEFECT LOGGED — watcher #103 (bv4z5nrii) delivered ~03:08 (my retraction row checked [x]) and was NOT re-armed until ~03:53: ~45min blind window during the so-kickoff stretch. The delivered-watcher-is-dead rule bit AGAIN mid-flow. Re-armed #104 (b0rycg3bp). Successor: re-arm IMMEDIATELY on every delivery notification, before resuming work.
- 2026-08-18 ~04:43Z | WATCHER #105 ARMED (bzjo3d6lg) after #104 (b0rycg3bp) delivered the Integrator epoch-2 row (ENGINE/BRIDGE-addressed; informational for DECODER, no action). Re-armed immediately on delivery per standing rule.
- 2026-08-18 ~04:45Z | WATCHER #106 ARMED (bz9mjulk6) after #105 delivered the Integrator unowned-instrument row (BRIDGE/ENGINE). DECODER filed the JKRDecomp pointer row (donor Yaz0 decoder, 100% matched, decodeSZS minimal port) before re-arming.
- 2026-08-18 ~04:47Z | WATCHER #107 ARMED (bpxmevj0b) after #106 delivered the Librarian close-out row (items done; informational, no DECODER action).
- 2026-08-18 ~04:53Z | WATCHER #108 ARMED (bf11tg6mk) after #107 delivered row 440 (ENGINE watcher now exists; DECODER-addressed correction re two decomp trees). DECODER FALSIFIED the two-trees claim at the filesystem (os.path.samefile True: WWDP = 8.3 alias of "WW DP") and filed the answer row before re-arming.
- 2026-08-18 ~04:55Z | WATCHER #109 ARMED (b3h3oct58) after #108 delivered Housing/Engine retraction of the two-trees claim (DECODER falsification confirmed; no action).
- 2026-08-18 ~04:57Z | WATCHER #110 ARMED (by8jbx2tf) after #109 delivered the History/Bridge doorway-census row (Integrator/Engine business; DECODER cc only, no ask).
- 2026-08-18 ~05:00Z | WATCHER #111 ARMED (bf3omh2im) after #110 delivered a checkbox-flip on the already-read retraction row (no action).
- 2026-08-18 ~05:05Z | WATCHER #112 ARMED (bty4d4shu) after #111 delivered History/Bridge doorway-audit close (cc only, no DECODER ask).
- 2026-08-18 ~05:08Z | WATCHER #113 ARMED (bt1khtk5u) after #112 re-delivered the census row (edit re-fire, no action).
- 2026-08-18 ~05:10Z | WATCHER #114 ARMED (blqc99h5f) after #113 delivered Housing/Engine SaveService answer (Bridge business; DECODER cc, no ask).
- 2026-08-18 ~05:16Z | WATCHER #115 ARMED (bh5410z52) after #114: History/Bridge wired the Yaz0 decode using the JKRDecomp pointer (row 438 outcome; builds). No DECODER action.
- 2026-08-18 ~05:44Z | DEFECT: watcher #115 delivered (~05:17Z, Housing/Engine yaz0Acquire cache-key row) and its completion notice was MISSED among timer ticks — DECODER blind ~27 min. USER caught it. WATCHER #116 ARMED (bxxi1grpg). Rule reaffirmed + strengthened: on EVERY timer tick, also check the newest watcher task output for an un-read EXITING line; the completion notification alone is not a reliable wake signal.
- 2026-08-18 ~05:45Z | WATCHER #117 ARMED (b9d14osfs) after #116 delivered 7 checkbox-flips (yaz0/census close-outs; no DECODER ask). Gap-sweep of the blind window: no DECODER-addressed asks missed; HISTORY checked off the so milestone row.
- 2026-08-18 ~05:58Z | WATCHER #118 ARMED (bguxful2d) after #117 delivered Integrator Phase-2 re-route to FOUNDRY (not DECODER).
- 2026-08-18 ~05:58Z FOUNDRY: **MY OWN WATCHER WAS DEAD FOR 206 MINUTES AND I DID NOT KNOW.** `monitor-pulse.json` last stamped 02:32Z (pid 2556, 38 passes); pid confirmed NOT RUNNING. I reported it ALIVE at 02:33Z -- true at that instant, and it died immediately after, so the report aged into a false one within a minute. **The user caught it, not the instrument.** RE-ARMED as harness task b00n8bl4k (`lane_watch.py --interval 180`), pid 31324, verified BOTH ways (stamp 9s fresh AND pid running as python.exe) per this file's own false-ALIVE lesson. Harness-tracked, not shell `&` -- the prior one is the fourth watcher this estate has lost that way. Census at re-arm: DECODER pid 29056 ALIVE, ENGINE pid 21316 ALIVE, LIBRARIAN pid 3976 ALIVE (1022 passes), INTEGRATOR: **its live pid 25544 is `integrator_timer.py`, a WORK-CONTINUATION TIMER in state `EMPTY-DISARMED` -- NOT a CALLS row watcher.** `integrator_watch.py` is NOT RUNNING (absent from the process table) and `integrator_watch_seen.json` has not been touched since 2026-08-17T17:31:51Z, ~12h before the row filed for that lane at 02:35Z. **A FRESH PULSE FILE PROVED THE WRONG INSTRUMENT WAS ALIVE** -- I read pulse-freshness as row-watching and stated it as such one line above this fix. Check WHICH instrument the pulse belongs to, not just its age. Also standing: `history_bridge_watch.py` (pid 12540) is still running for a lane that RETIRED.
- 2026-08-18 ~06:00Z | WATCHER #119 ARMED (beo37mwc7) after #118 delivered HISTORY'S P2/OB1 VERDICTS (p2: upstream Equivalent-for-regalloc precedent x3, staging recommended, USER decides; ob1: convergence, correctly labelled). DECODER filed the receipt row.
- 2026-08-18 ~06:03Z | WATCHER #120 ARMED (bi596uq36) after #119 delivered 3 rows (Phase-2/Outset-draw checkbox flips + Housing collision-disambiguation; DECODER cc only). modeProc structurally exact committed (95f4c761).
- 2026-08-18 ~06:05Z | WATCHER #121 ARMED (b7qoopzja, harness-tracked) after #120 delivered History/Bridge Phase-2 partial (119/123 stale headline, half-validated; no DECODER ask). NOTE: first re-arm attempt used shell & (dies with the call, the registry rule) — caught and re-armed properly.
- 2026-08-18 ~06:09Z | WATCHER #123 ARMED (bx61ny9ph) after #122 delivered History/Bridge Phase-2 number (21 stale rows; symbol-name analysis).
- 2026-08-18 ~06:12Z | WATCHER #124 ARMED (b3qmqde48) after #123 delivered 3 census-dialogue rows (no DECODER ask).
- 2026-08-18 ~06:17Z | WATCHER #126 ARMED (bieq6autv). #125 delivered 2 rows (Foundry/census dialogue). SECOND &-launch reflex slip caught immediately and re-armed tracked — the reflex is the hazard, not the knowledge.
- 2026-08-18 ~06:26Z | WATCHER #128 ARMED (bu01ub7dx). #127 delivered census/Foundry rows.
- 2026-08-18 ~06:29Z | WATCHER #130 ARMED (b5sm7d130). #129 delivered 2 rows.
- 2026-08-18 ~06:31Z | WATCHER #131 ARMED (b3x00wu9j). #130 delivered 1 row (Foundry/vtable dialogue).
