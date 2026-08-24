# HANDOFF — FOUNDRY (methods & instruments)

> Written 2026-08-14 on the user's order. Six sections, capped.
> **Cross-read pairing: HOUSING.** Reader's job is one question — *what did I
> leave out that you know about?*
> Every claim carries the § it was measured at. A claim without one is older
> than it looks.

---

## 1. WHAT I OWN

**`tools/foundry/` is THE instruments home** (user order, tale §850 — the
skeleton kit was folded in; nothing lives in `scratchpad/` or per-lane copies).
**`python tools/foundry/workflow.py` is the map** — 7 phases, ~22 named tools,
standing rules, surfaces. It SELF-CHECKS: every tool it names must exist, and
any `tools/foundry/*.py` it does not name is reported UNLISTED. **Start there,
not here.**

I own **measurement and instruments. I touch no content.** Ports are
History's; hooks and systems are Housing's; builds are the Integrator's.

Standing assignment from tale §968: **verification of any hook target before it
is bound** — `upstream_conformance.py --symbol` must show it unambiguous on
BOTH our fork and vanilla upstream first.

The § allocator (`staging.py alloc <bus> <lane>`) and the CALLS hygiene tools
(`file_row.py`, `routing_check.py`, `call_receipt.py`, `calls_rotate.py`) are
mine to maintain but belong to everyone.

---

## 2. HALF-DONE

- **`cargo_rank.py`'s callable column is not wired to `sig_diff.py`.** Housing's
  recommendation (accepted, tale §977) was that it be treated as *candidates
  requiring signature comparison*, never a binding manifest. **The wiring was
  not done.** Until it is, a reader can mistake one for the other.
- **`compile_gate.py` tier 2 cannot actually compile under Git Bash.**
  `vcvars64.bat` fails to initialise there. It now reports UNRESOLVED instead of
  a false clean (tale §977), which is honest but not working. **It works from
  cmd/PowerShell.** Fixing the invocation is unfinished.
- ~~**`sig_diff.py` still emits ~4 parser artifacts**~~ **CLOSED 2026-08-15.**
  They were not cosmetic artifacts — they were false DIVERGENT verdicts from
  an unnamed-parameter bug, plus three deeper defects in the same tool. See
  the successor pass in §3; do not read this line as "a tidy-up remaining".
- **PDB archiving is not wired into `build_run.bat`.** `build_identity.py
  archive` works and prunes to 10 (~2.3 GB ceiling); the one-line hook is the
  Integrator's file and the user ruled it good-but-not-priority.
- **Old items never closed:** 12 stale `.bat` deletions awaiting the user's nod;
  the `mods-src` → `WW-Crew-Restoration\plugin\` rehome; the mod-folder move to
  `%USERPROFILE%\Documents\A LBT WW`.

---

## 3. DO NOT TRUST  ← **read this before using anything I built**

**A successor's biggest risk is not unknown work. It is trusting something I
published that is wrong.**

### SUCCESSOR PASS — 2026-08-15 (Foundry, incoming)

Ran `control.py audit` first, per the outgoing note. **It did not agree with
the note.** The note says five gates are demonstrated live; the audit said
four live and **`sig_diff` BLIND**. Everything below follows from chasing that
one disagreement.

- **`sig_diff.py` was not merely blind — it could not run at all outside Git
  Bash.** It shelled out to `grep`, which is not on PATH under PowerShell, so
  it died on `FileNotFoundError` before printing a verdict. **This is the
  vacuous compile gate with the environments swapped** (tier 2 runs from
  cmd/PowerShell and not Git Bash). The scan is now pure Python — no external
  process, and both documented grep hazards (`\b`, the drive-letter colon) are
  gone with it. **Any sig_diff verdict in the record was produced under Git
  Bash by the flawed matcher described next.**
- **A literal BACKSPACE BYTE (0x08) sat inside the declaration regex**, one
  character past the negative-lookahead alternation:
  `(?!(?:return|if|else|...|throw)\x08)`. It made the lookahead assert "not a
  keyword *followed by a backspace*" — and no source line contains a
  backspace, **so the call-site filter was inert for its entire existence.**
  For `fopAcM_GetParamBit` the receiver side scored **53 "declarations", 52 of
  them call sites** (`return fopAcM_GetParamBit(this,0,8);`). This is the
  section-4 ferry hazard landing in the one place it is undetectable: the
  file reads correctly, `Edit` cannot match it, and it silently moved
  measurement counts. **Byte-audit instruments, not just documents.**
- **The `sig_diff` control was ITSELF vacuous-prone.** Its predicate was
  `"DIVERGENT" in out` — and the vacuous output it existed to catch is
  `0 DIVERGENT of 1 checked`, which contains that substring. A blind gate
  would have been rubber-stamped RED by its own negative control. **The defect
  the registry exists to catch was committed inside the registry.** Now
  anchored to the verdict line and explicitly rejecting the zero-summary.
- **Three of the four "parser artifacts" in §2 were FALSE DIVERGENCES, not
  cosmetic.** `norm_params` stripped the last bare identifier as the parameter
  NAME; when a declaration omits the name — which receiver headers do
  constantly — it stripped the TYPE instead and returned empty.
  `cM_rndF`, `cM_rndFX` and `dBgS_GetWaterHeight` are **SAME**.
  `fopAcM_setCarryNow` is donor `BOOL` vs receiver `int` — a typedef-spelling
  flag, the `cMtx_copy` class, **not** a confirmed divergence. **A gate can
  fail vacuously as well as pass vacuously**, and a false DIVERGENT burns a
  porter's day reproducing something that was never there. A second control
  (`sig_diff_same`, on `cM_ssin`) now proves the gate can still say SAME —
  the first control only ever proved it could say DIVERGENT.
- **`fopAcM_create` was reported DONOR-ONLY, "no receiver declaration
  found".** The receiver declares it twice (`f_op_actor_mng.h:528` and `:532`)
  — the matcher required the parameter list to close on one line, and it
  wraps in both trees. **That verdict is worse than unknown: it tells a porter
  to supply the actor-spawn entry point, which exists.** Continuation lines
  are now joined.
- **The six-symbol LIST in this section is RE-VERIFIED and stands** — all six
  DIVERGENT under a matcher with controls in both directions. **One
  correction: `fopAcM_create` is arity 8 vs 9, not 8-vs-7** (4 donor / 4
  receiver overloads, no compatible pair).
- **`code_dialect` (K3) is fixed and RED.** History/Bridge declared it BLIND
  and named the fix as Foundry's: `report_manual()` probed with `re.match`,
  anchored at position 0, so every key beginning with `(` produced `None` and
  was discarded at a bare `continue`. **The DN-3 raw-cast row
  `(J3DModelData*)dComIfG_getObjectRes(...)` — the one guarding the §810-2 /
  §814 crash — could never appear in a report, so "(none present in this TU)"
  asserted nothing about it.** Rows are now probed on any identifier in the
  key, and a key with no identifier reports UNPROBEABLE rather than vanishing.
- ~~**Audit now: 7 live, 0 blind, 8 undeclared.** The eight undeclared are
  unchanged and remain the estate's real exposure.~~ **NO COUNT LIVES HERE ANY
  MORE — run `python tools/foundry/control.py audit`.** The count moved 7 → 9
  → 13 in one evening; every citation was accurate at the keystroke and all of
  them are now wrong. **A live-measured count does not belong in a durable
  document as a bare number** (Librarian's rule, and my first correction of
  this line broke it again by printing the new number). Name the command in
  anything a successor inherits as current state; keep the number only in
  things stamped to a moment, like tale entries — which is why tale §1005's
  `9 / 0 / 8` must NOT be edited. The "unchanged" was the worse
  half: `l2c_equiv` was never uncontrolled. `l2c_equiv.py --selftest` — its own
  §695 #4 law, exercising EQUIV / DIFFER / NO-REF / empty-dir — had been in the
  file the whole time and wanted a registry entry, not a control. **I cited
  `control.py`'s undeclared list as the estate's exposure without opening the
  files behind it: that list measures what is REGISTERED, not what is
  UNTESTED.** A narrower measurement than the claim it supported — the fault
  this whole section catalogues, committed while writing the section. The
  remaining undeclared tools were swept for a `--selftest` or fixture harness
  and have none, so those are real. **Before treating any name in that list as
  uncontrolled, grep the tool first — the control may already exist and want
  only a registry entry.**
- **Still injected, NOT my surface, untouched:** a `0x08` in
  `docs/WW Linked/ww-bridge-tool-interconnected.md` (line ~19407, trailing in
  the prose above the roster evidence rule — the regex itself is intact), and
  **three `0x00` plus one `0x0C` in `docs/state/cut-actors-demo-restore-archive.md`.**
  That second one is not cosmetic: at line ~103 the manifest gate condition
  reads `man.arc[0] != '<NUL>'` where the source is `'\0'` — the backslash
  collapsed on the ferry, so **the archive misstates the gate it documents.**
  Sweep them with the C0 audit before trusting either file.

- **EVERY historical `[TIER2] cl /Zs clean` from `compile_gate.py` is
  WORTHLESS.** For its entire existence it took its flags from a libjpeg-turbo
  *configure* line that merely mentions `cl.exe`, and under Git Bash `cl` never
  ran at all. It reported "verified by the real compiler" having compiled
  nothing. **No port is compile-verified on the strength of a past tier-2 pass.**
  Tier 1 is unaffected. (tale §977)
- **The signature divergence RATE is withdrawn — all three of them.** I
  published **15%**, a second pass said **9%**, the fixed matcher says **4%**.
  The first two ran on a broken matcher. **Cite no percentage.** What survives is
  a LIST: six arity/width changes that would break a by-name binding
  (`fopAcM_create` 8-vs-7, `fopMsgM_messageSet` 2-vs-3, `fopAcM_orderSpeakEvent`
  1-vs-3, `dComIfGp_setItemLifeCount` 1-vs-2, `dComIfGp_setMessageCountNumber`
  s16-vs-s32, and Housing's `fopAcM_GetParamBit`).
- **`cM_ssin` and `cM_scos` are NOT divergent.** I published them as such; they
  are byte-identical in both trees. Housing caught it. (tale §977)
- **`cMtx_copy` and `mDoMtx_multVec` are FLAGGED, NOT PROVEN.** Both are
  typedef-spelling differences (`CMtxP` vs `const Mtx`) that may be
  ABI-identical. **`sig_diff.py` compares SPELLING, not ABI.** Only
  `fopAcM_GetParamBit` is confirmed, and Housing confirmed it independently.
- **My §975 row accusing HISTORY of being silent since tale §903 was FALSE and
  is withdrawn** (tale §977). They had been working continuously.
- **"HOUSINGTEMP silent since tale §697" was also false** — HousingTemp and
  Integrator are the SAME INSTANCE (user, 2026-08-14). Fixed in
  `call_receipt.py`; the roster never recorded the merge.
- **A receipt verdict is a SNAPSHOT of a concurrently-written board.** RUN
  `call_receipt.py`; do NOT file its output as a row. Filing one turns a
  measurement into a standing accusation that keeps asserting itself after it
  stops being true — that is exactly how the §975 row went wrong.
- **Trust the per-symbol tool, not the sweeps.** Both bulk sweeps disagreed with
  `sig_diff.py <symbol>` on real symbols. Run it per binding and **read the
  printed source lines** — if the "declaration" contains a `)` mid-expression or
  a float literal, it is a call site and the verdict is garbage.

---

## 4. LOAD-BEARING ASSUMPTIONS

Things a successor would never think to question:

- **Donor arcs: `<decomp-root>\Ex WW`. Decomp SOURCE: `<decomp-root>\WW DP\src`.**
  Debug linker maps: `<decomp-root>\WW Debug maps`. **Read the donor's own system
  FIRST — DN-10.** A fix whose write-up cites only receiver files has not
  cleared step 1.
- **ZERO-BAKE: no donor asset is ever edited, anywhere.** Every donor→receiver
  difference translates at the CONSUMPTION boundary in receiver code. This is
  what lets us ship no donor bytes at all.
- **The plugin carries NO receiver headers BY DESIGN.** That is precisely what
  makes it version-independent. Any struct it needs must be a layout-compatible
  mirror authored plugin-side — and a mismatch is a FIELD MISREAD, not a build
  break.
- **Symbol resolution reads a manifest EMBEDDED INSIDE the image**
  (`imageBase + s_symdbDescriptor.rva`), **not the PE export table.** I checked
  the export table once and nearly published "MISSING upstream" for two methods
  that were present.
- **That manifest carries ANONYMOUS-NAMESPACE symbols** (tale §977). Every
  earlier estimate of "what a plugin can hook" assumed external linkage. **It
  does not bind — far more of the fork is reachable than an exports view says.**
- **`fpcNm_MAX_NUM` bounds exactly TWO things tree-wide:** `fpcPf_Get`'s OOB
  guard (`f_pc_profile.cpp:51`) and `DMC[]` (`c_dylink.cpp:807`). That is why
  the 13-slot ceiling dissolved.
- **The x64 profile chain is 0x28 / 0x38 / 0x48** (base / leaf / actor), alignof
  8. **The headers' own `/* 0x.. */` comments are GameCube 32-bit and are WRONG
  for the target on all three.** Compiler-measured with a failing control.
- **Python is `<decomp-root>/foundry-py312/Scripts/python.exe`.**
- **CONTENT AND CODE GO THROUGH Write-TOOL FILES ONLY.** Never heredocs or
  `python -c` with escapes. Unquoted delimiters have injected literal backspaces
  and NUL bytes into source and docs at least six times — invisible to Read,
  unmatchable by Edit, and they silently moved measurement counts. **Byte-audit
  every ferry (`NUL 0 BS 0`).**
- **`TaskList` DOES NOT SHOW MONITORS.** It returned *"No tasks found"* while
  THREE monitors were demonstrably running and firing. **You cannot enumerate
  your own watchers** — so a stale monitor is invisible, and duplicates
  accumulate silently. Track the task IDs the `Monitor` tool returns; that is
  the only record. **This is how I ended up with three at once** (the user
  caught the first dead one; I found the other two by hand).
- **`grep -E` in this Git Bash does not honour `\b`, and reads a trailing `\(`
  as a group opener.** Both produced silent zero-match sweeps.
- **Windows paths break `line.split(":", 2)` on grep output** — the drive-letter
  colon. Parse with a regex.

---

## 5. ERROR CLASSIFICATION  (the diagnostic the user asked for)

Sorted honestly against myself.

**METHODOLOGY — a fresh instance repeats these on day one:**
- The vacuous `compile_gate` tier 2 — **built without a control that must fail.**
- `sig_diff.py`'s three bugs — same cause: I ran a control that passed and
  shipped anyway.
- Publishing the 15% rate before trying to break it.
- Specifying the `cDyl_IsLinked` hook BACKWARDS — reasoned from the SYMBOL
  instead of the CALL PATH. **Twice** (also tale §967).
- Seven "derived view" errors: parsed the `__MWERKS__` console array instead of
  the PC one; the PE export table instead of the embedded manifest; unique names
  reported as entries; a collapsed dict queried for uniqueness; bare instead of
  qualified name matching; "native TP file" decided by FILENAME; a fix that did
  not propagate to sibling callers.

**LOST-TRACK — context; a fresh instance would not repeat these:**
- **My monitor died and I did not notice.** The user caught it.
- Stale numbers left in my own tool's banner after re-measuring.

**RECLASSIFIED ON EXAMINATION — this one looked LOST-TRACK and is not:**
- **Three monitors running at once.** I first wrote this down as context drift.
  It is not: I ran `TaskList`, got *"No tasks found"*, and **accepted that as
  authoritative while a monitor was actively firing in the same minute.** The
  tool cannot see monitors. **I trusted a gate without a control that must
  fail — the identical failure mode as the vacuous compile gate**, applied to
  someone else's tool instead of my own. A fresh instance repeats it.
  **METHODOLOGY, and it strengthens rather than weakens the verdict below.**

**VERDICT FOR THIS LANE: predominantly METHODOLOGY.**
**Retiring me would not fix it.** The fix is a rule, and it is the one thing I
would carve above the door:

> **EVERY GATE NEEDS A CONTROL THAT MUST FAIL.**
> **If you have never seen your gate go red, it has not been tested.**

That single rule would have caught the vacuous compile gate, the `cM_ssin` false
positive, and the 15% rate — three of the four worst things I shipped today.

**The counter-evidence, stated fairly: almost every error was caught within the
hour, and mostly by OTHER LANES.** Housing caught `GetParamBit` and `cM_ssin`;
the Integrator caught the duplicate-record drift; the user caught the dead
monitor. A system where errors surface fast is working, not degrading. My error
RATE was high; the ESCAPE rate was low.

---

## 6. POINTERS  (never copies — these surfaces are live)

- `python tools/foundry/workflow.py` — **the map. Start here.**
- `docs/state/ww-staging/CALLS.md` — the ONLY call surface. A row triggers a
  lane; a WHOSE-TURN mention is narrative.
- `docs/DO-NOT.md` — the hard stops. **DN-10 binds every task.**
- `docs/state/ww-staging/` — BUILD-QUEUE · PORT-QUEUE · BACKLOG ·
  SECTION-LEDGER · BUILD-IDENTITY
- The two buses — `ww-tale-dmesg-live-state.md` (tale) and
  `WW Linked/ww-bridge-tool-interconnected.md`. **Allocate before posting.**
- `docs/LANES.md` — the user's roster ruling. **Known gap: INTEGRATOR is not on
  it, despite being the sole builder under §839. Corrections are the user's.**
