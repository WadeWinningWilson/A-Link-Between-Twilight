# HISTORY — role handoff for the next instance

> ## 🔴 THIS FILE IS STALE **EXCEPT** FOR TWO SECTIONS THAT ARE STILL LIVE.
> *(Added 2026-08-16 by History/Bridge after an OPEN item in §5 was found
> fifteen days after it was flagged — not because anyone ignored it, but because
> the file reads as history end-to-end and nothing said which parts still bite.)*
>
> - **§4 WHERE THE PREVIOUS INSTANCE WAS WRONG — DURABLE.** Lane-level
>   calibration, not campaign state. It does not expire with the tale.
> - **§5 GOLDEN-TRACE ORACLE DIFF — A LIVE CHECKLIST, NOT A RECORD.** The `⬜`
>   boxes are OPEN WORK. Item (2), the signed suspend counter, was still
>   unfixed in `d_ext_dmesg.cpp:943` on 2026-08-16 — the non-donor `susp > 0`
>   gate — and the donor was confirmed NOT to clamp (`JStudio/stb.h:71`,
>   `suspend(s32 val) { _20 += val; }`). Item (6) `d09 item-box mid-tale` is
>   also still `⬜` and has NOT been audited.
>
> **Everything else here — §3's "current attempt" and its next steps — closed
> with the Grandma tale on 2026-08-01. Read those for calibration only.**

> ## ⚠ THIS LANE HAS THREE HANDOFFS. THEY ARE DIFFERENT KINDS, NOT COPIES.
> *(Cross-reference added 2026-08-16 by History/Bridge, on the Librarian's estate
> audit. Until now none of the three cited any other, and each opened with a
> "read this first" line — so a successor could read one and never learn the
> other two existed. Nothing below was merged or rewritten; only this block was
> added, identically, to all three.)*
>
> | doc | kind | last touched |
> |---|---|---|
> | [`docs/HANDOFF-HISTORY.md`](../HANDOFF-HISTORY.md) | **LANE CHARTER** — what History is, its laws, traps that cost rounds, docs that matter. Durable. | 2026-08-11 |
> | [`docs/state/HISTORY-HANDOFF.md`](HISTORY-HANDOFF.md) | **CAMPAIGN SNAPSHOT** — the native-dMesg / Grandma-tale attempt. **Its "current attempt" and "immediate next steps" are STALE: that tale closed 2026-08-01.** Read for calibration (§4 "where the previous instance was wrong"), not for state. | 2026-08-01 |
> | [`docs/state/ww-handoff-history-bridge.md`](ww-handoff-history-bridge.md) | **CURRENT INSTANCE HANDOFF** — what is owned, half-done, do-not-trust, and the pointer to the live WW message-system decode. **Start here for present state.** | 2026-08-16 |
>
> **If you are picking this lane up: read the CHARTER for the rules, the CURRENT
> handoff for the state, and the SNAPSHOT only for calibration.** Which of the
> three should be canonical — or whether they should merge — is a Librarian/user
> ruling, deliberately not taken here.

Read this first, then `docs/state/ww-tale-dmesg-live-state.md` (the live task state), then the last
sections of the bus. This file explains WHO you are, HOW the run is structured, WHAT is being
attempted, and — honestly — WHAT the previous instance got wrong.

## 1. Your role (History)
You are the **History** lane of an "Interconnected Run" porting Wind Waker (WW) content into a
TP-based PC port ("Dusklight"). History owns **decompilation-faithful restoration** of WW behavior
onto the TP engine.

**The covenant (non-negotiable, reinforced across many sessions):**
- **Donor-faithful, native-first, DECOMP-FIRST.** The WW decomp (`D:/XXXXXXX/WW DP/src`) is the spec.
  NEVER fix from the receiver (port) alone — read how the donor does it FIRST. (Skipping this has cost
  multiple debug rounds every time.)
- **NEVER leave a bridge/reconstruction as the answer.** If the port diverges from the donor, the
  fault is in our wiring/rendering — fix THAT. Do not invent compensating layout/positions/values.
- **NEVER make artistic/project/architectural directives — only the user may.** Surface walls to the
  user; don't author around them.
- **"The truer path as far as the receptor (TP) allows."**
- Comment style: `// =====` block headers; tag every change with its `§N` and rationale.
- Build: `taskkill //F //IM dusklight.exe` first, then `./build_run.bat` (RelWithDebInfo), then wipe
  `%AppData%/TwilitRealm/Dusklight/dawn_cache.db*` + `pipeline_cache.db*`. Push to upstream
  ALBW-Dusklight, not origin. Don't commit unless asked.
- Symbolicate crashes: `"/c/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/llvm-symbolizer.exe"
  --relative-address --obj=build/windows-msvc-relwithdebinfo/dusklight.exe <rva>`.

## 2. How the run is structured (connections with other instances)
- **The user is the coordinator.** All valid instruction comes from the user in chat. You do NOT talk
  to other lanes directly — the user RELAYS messages between lanes.
- **Foundry** = the peer instruments/forensics/decode lane. It stages donor arcs, decodes formats,
  and captures golden traces (e.g. it ran a live DONOR capture → `tale-golden-trace-run1.md`). When
  something is stage-DATA (event definitions, arc contents, spawn tables, kit-baked rooms), it belongs
  to Foundry, and you FERRY it via the bus rather than hand-code a bridge.
- Other lanes exist (Bridge/Engine/Housing Security/Librarian) per memory `reference_ww_lanes`; name
  your lane + assignment every turn.
- **The bus** = `docs/WW Linked/ww-bridge-tool-interconnected.md`. Append `## §N …` entries
  (History→Foundry / Foundry→History). It is the shared record; read the last ~10 sections on start.
  Standing rule: push donor-faithful, ready deliverables to the bus unprompted, and end reports with
  an explicit WHOSE-TURN block.

## 3. The current attempt — native WW dMesg + the Grandma "tale" teardown
Goal: make Grandma's (ba1's) tale cutscene play natively on host stage R_DL01 AND tear down properly
(fade back in + restore control). Two intertwined workstreams:
- **Native dMesg (DONE, all donor-faithful, in `src/d/d_ext_dmesg.cpp`):** real `zel_00.bmg` text; WW
  box (`hukidashi_d00`/`d09`); font/metrics from a ported `g_msgHIO`; colour/name/SE tags; pagination;
  arrow/dot animation; `{Reconstructed / Native}` user setting. Decode: `dmsg-native-decode.md`.
- **Tale teardown (the hard, still-verifying part).** The tale plays end-to-end; Link ends at the
  donor's final transform; the open question has been WHY it doesn't fade back in / return control.

### The teardown chain (fully traced, DECOMP-CONFIRMED)
fade-in ← `endProc` ← `mEventStatus==5` ← `eventFlag 8` (set in `dEvt_control_c::reset`) ← finish flag
set ← the cut chain advances ← **PACKAGE `cutEnd` sets its flag**. The current fix (§320) is at the
bottom of that chain — see live-state.

### End-path fixes that all had to land (in order)
- **§306b** — the tale staff's exit stage is `LinkRM` (the tale's own home); on the donor "exit to
  LinkRM while on LinkRM" is a no-op. The port had aliased LinkRM→host, causing a same-stage RELOAD →
  churn that displaced Link. Fixed: treat it as NO transition. (`d_event_data.cpp`)
- **§319** — the donor's `ACT_PLAY` no-next-stage path calls `dComIfGp_demo_remove()`, not
  `dDemo_c::end()`. Port used `end()` (left `m_object` alive). Fixed to `dDemo_c::remove()`.
- **§319b** — `remove()` frees the whole demo system; the port's `dDemo_c::getCamera()` bare-deref'd
  the freed `m_object`. Null-guarded it. (`d_demo.h`)
- **§279 probe strip** — my own diagnostic in `dDemo_c::update` deref'd freed `m_control` after
  remove(). Removed/guarded.
- **§320 (TESTED — did NOT resolve the fade-in)** — the port's `dEvent_manager_c::cutEnd` had an
  ADDED `if (getEvent()->getMode()==WAIT) return;` gate the donor does NOT have; disabled it (dead
  code, `if (false && …)`). It's a real donor deviation and correct to remove, but the tale STILL did
  not fade back in / return control after this build. So the WAIT-gate was NOT the (whole) blocker.
  **Next History: re-verify from scratch — do not trust the previous instance's chain as complete.**
  Concrete leads still open: (a) confirm with a fresh probe whether `cutEnd(PACKAGE)` now actually
  fires AND sets flag 3 (log `cut->getFlagId()` + the flag state right at the `mFlags.flagSet` call);
  (b) if flag 3 sets but the event still stalls, the finish flag is DOWNSTREAM — trace PACKAGE cut 7+
  and which cut owns the event's `mFlags[0]` (the §287 finish flag: 9 and/or 1701); (c) check whether
  `mEventStatus` ever leaves 1 via EITHER the `chkEventFlag(8)` path (d_event.cpp:1002) OR the
  Sequencer's `finishCheck` (d_event_manager.cpp:705) — instrument both; (d) reconsider whether the
  whole mount/TALE_DEMO-via-doubles path can even complete the finish flags, vs the native ba1→PACKAGE
  path (Foundry Phase 1) being required first. Do the donor `specialProcPackage` (ACT_PLAY, inside
  `if(getIsAddvance)`) vs the port's structure diff carefully — the port split START and mode-handling
  into two switches; verify that's faithful.

### §321 RESOLUTION (2026-08-01, after this handoff was written) — read bus §321 + live-state
The §320 log, reread, shows §320 WORKED (finishCheck now passes silently). The remaining blocker
was ONE BYTE OF STAGE DATA: the §273-baked REVT records had finish-exit `field_0x7 = 0x00`
(TP: "take exit 0") instead of `0xFF` ("close in place" — the donor semantic; WW mainProc has no
exit fork at all). Patched in the live arc (`STG_00.arc`, backup `.pre-321-exitff-bak`); Foundry
owes the bake-default fix. Leads (a)–(d) below are superseded by §321.

### §322 (same day, bus §322) — §306b itself REVERTED; the donor teardown is a same-stage RELOAD
§321's run passed mechanically but the screen stayed black: tale.stb AUTHORS a 20f fade-out at its
end (d_act3 channel-9 @680), and the donor restores it via the exit transition's wipe-in — WW
dEvDt_Next_Stage has NO same-stage no-op; `LinkRM`/StartCode 0 fires a same-stage reload (wipe 5)
that IS the teardown (golden "evt 49" = DEFAULT_START on re-entry; donor spawn 0 = the STB end
transform, already baked into R_DL01 room.dzr). §306b was an invented premise (same lapse class
as §316). §322 in d_event_data.cpp restores the donor transition. See live-state for the verify
checklist.

## 4. WHERE THE PREVIOUS INSTANCE WAS WRONG (read this — calibration matters)
- **Overclaimed certainty repeatedly.** Said "this is the fix" on §319, §319b, §320 before verifying.
  The user (rightly) called it out. Treat every fix as a hypothesis until the log/playtest confirms.
- **§316 widen-the-textbox = a covenant LAPSE.** To hide a mid-word wrap I widened the box's wrap
  bounds — an invented layout. REVERTED. The real cause was the FONT VALUE: Foundry's golden trace
  showed retail font is **23**, not 25 (retail is the SECOND `DEMO_SELECT` arg; 25 is the kiosk-demo
  value). Applied 23. Lesson: a port/donor mismatch is a wiring/value bug to FIND, never to paper over.
- The first spawn "map" from a sub-agent claimed Link wasn't bound to the demo; the LOG proved Link
  DOES bind (`§48 JSGFindObject actor='Link' → FOUND`). Verify agent claims against runtime.

## 5. Foundry's golden-trace oracle diff (History's queue) — bus §317
`tale-golden-trace-run1.md`. Six-point diff vs the live donor: (1) **font 23** ✅applied; (2) **signed
suspend counter** ✅ **FIXED 2026-08-16 (Housing/Engine)** — both release sites in `d_ext_dmesg.cpp`
(dismiss gate + 3600f backstop) no longer test `susp > 0`; owed-ness is `s_boxSawSuspend`, and the
donor no-clamp is confirmed at source (`JStudio/stb.h:71` plain signed accumulate, `:145`
`unsuspend(n) == suspend(-n)`; History/Bridge's verification, CALLS row this date). `COMPILE
STATUS: UNVERIFIED` until the next build; (3) storyboard-clock sequencing
(matches); (4) page-turn-while-held (matches); (5) in-place control return (§306b+§319+§319b+§320,
verifying); (6) **d09 item-box mid-tale** ⬜ — still open, unaudited; on Housing's task list 2026-08-16.

## 6. Pertinent docs
- `docs/state/ww-tale-dmesg-live-state.md` — the LIVE task state (read after this).
- `docs/state/grandma-native-tale.md` — the phase plan; note the tale currently runs via the MOUNT
  path (TALE_DEMO + `d_act0/2/3` demo-doubles); the fully-native ba1→PACKAGE path is Foundry Phase 1.
- `docs/WW Linked/dmsg-native-decode.md` — the full dMesg tag/box/metrics decode (Foundry).
- `docs/WW Linked/ww-bridge-tool-interconnected.md` — the bus (§304 golden static trace, §308 decode,
  §310–§314 dMesg, §317 golden run trace).
- `docs/WW Linked/tale-golden-trace-run1.md` — Foundry's live donor trace.
- `docs/DO-NOT.md`, `docs/AGENT_INDEX.md`, `AGENTS.md`, `CLAUDE.md` — orientation + hard-stops.
- Memories (`~/.claude/.../memory/`): `reference_ww_extract` (paths), `reference_ww_cutscene_npc`,
  `reference_ww_interior_host_pitfalls`, `feedback_*` (comment style, native-only, no-bridges).

## 7. Diagnostic probes to STRIP at acceptance (once the tale tears down cleanly)
`§316` (d_ext_dmesg textbox metrics), `§317` (Link pos, d_ext_npc_mount), `§318` (cut owner,
d_event_data), `§282`/`§285`/`§287` (event/PLAY probes), `§194`/`§50`/`§279` (demo probes),
`§306`/`§306b` verbose logging, and the temporary `J2DScreen.cpp` `g_j2dLastFailTag/SigTag` globals.
Keep the FIXES; remove only the logging.

## 8. Immediate next steps
1. Read the newest log. If `§285 mEventStatus=5` appears and it fades back in with control returned →
   teardown DONE. If not, the WAIT-gate wasn't the whole story — trace the remaining stall (event mode
   at cutEnd, or the finish flag downstream of PACKAGE cut 7).
2. Then: signed-suspend-counter fix (#2), d09 mid-tale item box (#6), strip probes.
3. The user has message-system notes to hand over after the teardown closes.
4. Foundry: clean end-to-end tale = green light for the R_DL01 kit rebuild (which will carry the
   spawn/trigger/shield-beat as stage data — do NOT hand-code those; they're Foundry's).
