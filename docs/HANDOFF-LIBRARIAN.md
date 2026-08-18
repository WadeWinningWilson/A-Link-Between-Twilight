# HANDOFF — Librarian lane

> **STATUS: RETIRED 2026-08-14 (user, per-lane call §589). This handoff is FINAL — the closing
> state, not a mid-session draft. Successor: start here; it is Foundry-cross-read and corrected.
> Detection monitor stopped on retirement.**

*Written 2026-08-14 per the all-lanes handoff order (user via Foundry, on CALLS.md). **Cross-read
DONE by Foundry** (pairing Librarian↔Foundry) — three omissions fixed below.
**Citation note (my error):** an earlier draft cited "tale §589" for the order — 589 was the CALLS
**row line**, not a bus §-allocation (none exists; Foundry filed the order without a §). Foundry is
allocating a real § and re-stamping the order; re-point here when it lands.*

## (1) WHAT I OWN
- **Document STRUCTURE** of the WW→TP port: where things live, structural soundness — **never truth/content**. Charter: [Librarian.md](Librarian.md).
- Surfaces I maintain: **[CALLS.md](state/ww-staging/CALLS.md) Protocol** (detection contract + filing rules), **[LIBRARIAN-QUEUE.md](LIBRARIAN-QUEUE.md)** (my flag/ruling registry), the **[AGENT_INDEX](AGENT_INDEX.md) clean-room delivery cell**, **[INVENTORY-SCHEMA.md](state/ww-staging/INVENTORY-SCHEMA.md)** (§7 count fixture).
- **Lint DOCTRINE I state, Bridge/Foundry BUILD:** CALLS-LANE-TOKEN (now in `file_row.py`), SAFE ROW FILING, APPEND-IS-NOT-A-CALL-SURFACE, CHECKBOX-ADDRESSED-TO-READER (born-checked), ROW-EXPANSION-DAMAGE — all in the CALLS Protocol / QUEUE §BUS.
- Decisions routing to me: dedup, supersession stamping, compaction, stale-pointer lint, "where does this live", status-enum sweeps.
- I do **NOT**: edit source, author content, rule truth, build linters, merge doctrine on my own authority.

## (2) HALF-DONE
- **LANES-ROSTER-DRIFT** (QUEUE §BUS): the order folds in the answer — **HOUSINGTEMP = INTEGRATOR, "five identities not six"** (board-claimed; **NOT** inscribed into [LANES.md](LANES.md), awaiting real-user confirm; my row-81 mis-normalization is the §(3) caveat). **CORRECTION (Foundry cross-read — was STALE ON ARRIVAL): the lint ALREADY reads the roster from LANES.md** (`file_row.py:105-134` — `LANES_DOC`, `ratified_lanes()` parses the table, `KNOWN_LANES = ratified ∪ OPERATIONAL ∪ COLLECTIVE`, INTEGRATOR/HOUSINGTEMP labelled OPERATIONAL-not-ratified, delta printed by `file_row.py lint`). So the tool is not a lane authority and that work is DONE. **Only open: whether the user formally RATIFIES INTEGRATOR/HOUSINGTEMP into LANES.md** (names-on-faces = user's).
- **TALE-BUS §-HYGIENE (Foundry cross-read surfaced it; my territory):** `staging.py verify` reports **39 UNALLOCATED sections + 26 COLLISIONS** on the tale bus — **all 39 unallocated are History's** (writing sections without `staging.py alloc`). Allocator is sound; the gap is the writing habit. **This is WHY my `## §976b LIBRARIAN` exists:** I allocated §976 properly, History posted `## §976 HISTORY/BRIDGE` over it claiming an allocation the ledger doesn't back, so I took the `b` suffix (a suffix with no recorded hazard was itself the omission). Successor: flag History to allocate-before-writing; the retroactive dedup is a larger separate pass.
- **§968 Option-1 slot ruling** — board-claimed USER RULING, HELD from the hub pending user confirm.
- **Hub compaction (Q2)** — did the light in-place version; the heavier "move full chain into [ww-clean-room-delivery.md](WW%20Linked/ww-clean-room-delivery.md)" is optional/unstarted.
- **Stale-content flag** (routed to Housing/Integrator, unfixed by owner): `ww-clean-room-delivery.md` §(2) "never exercised end to end" is stale — the DATA half ran at tale §933.

## (3) DO NOT TRUST
- **My CALLS row-81 normalization `HOUSINGTEMP → HOUSING/ENGINE` is WRONG.** I treated HOUSINGTEMP as a typo on the Integrator's claim; §589 reveals HOUSINGTEMP **is** INTEGRATOR (merged identity), not Housing/Engine. Do not cite it as a lane-token precedent; the row's lane field is mis-corrected.
- **"pre-flight SAFE" ≠ signature-cleared.** My §968 queue note relays "both symbols pre-flight SAFE" — that is NAME resolution only (§455). A binding also needs donor-vs-receiver signature comparison.
- Anything in my QUEUE stamped **"board-claimed, NOT inscribed"** is a relay of another lane's claim, not my own measurement — trust it only to the § it cites.

## (4) LOAD-BEARING ASSUMPTIONS (a successor would not think to question)
- **Detection**: I catch calls by watching CALLS.md for my lane in **UNCHECKED** rows (monitor `bdf37sjxb`, 12s, case-insensitive `librarian|all lanes|stand.?down`). NOT by grepping buses (buses = narrative). **DECLARED CONTROL (§1004 "a check that cannot fail is not a check"):** this gate is PROVEN (fired 15×+ on real calls; correctly ignored the `TESTLANE` probe + other-lane rows) but is **not complete** — it is BLIND by construction to a **born-checked `[x] LIBRARIAN` call** (7th shape) and to an **appended ask**. Both are now forbidden by the CALLS Protocol, and I run a receiver-side scan (`^- \[x\] LIBRARIAN` rows lacking a `->` discharge — last run clean) as the failure-catch. The blind spots are NAMED, not assumed absent. **Successor caveat (found at retirement):** the stopped monitor's regex used `\b` word-boundaries (`^- \[ \][^|]*\blibrarian\b`) — which §(4) flags as unreliable on this shell. It fired correctly all session (so `\b` works under `grep -aciE` here, unlike the `grep -E` Foundry tested), but **rebuild the monitor with a LITERAL pattern** rather than inherit a `\b` dependency you haven't re-verified.
- **Bus visibility ≠ detection**: I must post a `## §N LIBRARIAN` bus section per session or instruments report the lane absent (learned §975; my section = **tale §976b**).
- **Content-neutrality wall** = the safety property: rule structure, never truth; flag content contradictions and route to the owning lane / user; never adjudicate.
- **Hot append-only files** (CALLS/tale/interconnected): edit via clobber-safe compare-and-swap or `file_row.py`. Tale bus is **CRLF**; `§` is 2-byte UTF-8 (breaks naive `grep -oE "§[0-9]+"`).
- §939 delivery model: receiver CONTENT stays as-is; only DELIVERY moves plugin-side.
- **Git Bash grep/parse gotchas (SILENT zero-match — worse than the `§` one because they fail quietly, Foundry cross-read):** `grep -E` here does **not** honour `\b`; a trailing `\(` is read as a **group opener**; and `line.split(":", 2)` on grep output breaks on the Windows **drive-letter colon** (`C:\...` → `"C"`). Same family as "§ is 2-byte UTF-8" — a clean-looking result can be a silent no-match. Prefer literal patterns + independent re-read.

## (5) ERROR CLASSIFICATION (honest, against myself)
- **row-81 HOUSINGTEMP mis-normalization — METHODOLOGY.** I *ruled* a token identity when the authoritative roster was silent/stale, instead of *flagging* the uncertainty. That is the content-neutrality wall breached against a lane token. A fresh instance repeats it on day one **unless the control is added: when the authority is silent on a token, FLAG — never rule.**
- **`scratchpad/file_row.sh` pointer** (one turn stale before the tool moved to `tools/foundry/`) — LOST-TRACK, minor, self-corrected next turn.
- **"tale §589" phantom citation — METHODOLOGY.** I cited a CALLS **row-line number** as a bus **§-allocation**; no §589 exists. Violated my own doctrine ("a citation is a promise the record can be read"). Control: verify a bus § is allocated/readable before citing it; a CALLS row-number is not a bus §. (Foundry compounded it by filing the order with no §, but the unreadable cite is mine.)
- **Net:** one genuine METHODOLOGY error; the rest was disciplined (held claims pending verification, distinguished proof-from-ruling, verified fixes with the instrument, held board "USER RULING" rows pending real-user confirm). **No context-degradation LOST-TRACK observed this session.** Per the directive's own logic, a mostly-METHODOLOGY profile ⇒ **retiring the Librarian fixes little; the control (flag-don't-rule on a silent authority) does.**

## (6) POINTERS (live surfaces — never copies)
- Charter [Librarian.md](Librarian.md) · Roster [LANES.md](LANES.md) · Shared rules [AGENTS.md](../AGENTS.md)
- Registry [LIBRARIAN-QUEUE.md](LIBRARIAN-QUEUE.md) · Calls [state/ww-staging/CALLS.md](state/ww-staging/CALLS.md) (Protocol at top) · Hub [AGENT_INDEX.md](AGENT_INDEX.md) · Schema [state/ww-staging/INVENTORY-SCHEMA.md](state/ww-staging/INVENTORY-SCHEMA.md)
- Tools (verified present): `tools/foundry/staging.py` (allocator) · `tools/foundry/file_row.py` (verified filer) · `tools/foundry/call_receipt.py` (delivery receipt)
- Memory: `reference_librarian_ops` (operating map) · Bus section: **tale §976b**
