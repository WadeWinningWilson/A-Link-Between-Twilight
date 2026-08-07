# HOUSINGTEMP — LANE HANDOFF (spun up 2026-08-05, user ruling)

> **User ruling, verbatim:** *"we'll make a new instance, the HousingTemp, needed for
> anything that slips by any of you."*

Read this file, then [AGENT_INDEX.md](AGENT_INDEX.md), [DO-NOT.md](DO-NOT.md),
[LANES.md](LANES.md), and [HOUSING-HANDOFF.md](HOUSING-HANDOFF.md) (the lane you are
paired against). You are **not** Housing Security. You are the lane that catches what
Housing Security — and every other lane — missed.

---

## 1. WHY THIS LANE EXISTS (the evidence, not a theory)

[LANES.md](LANES.md) carries a load-bearing rule: *"The lane that **builds** an
instrument is never the lane that **audits** it — the anti-'marking your own homework'
rule."* Bridge builds the ledger, Housing negative-controls it. Foundry builds the
instruments, Housing negative-controls them.

**The rule has a hole: nothing audits Housing Security, and nothing owns the gaps
between lanes.** On 2026-08-04/05 that hole produced five separate misses in one
session. These are your charter, and they define your detection patterns:

| # | What slipped | Who caught it | Who should have |
|---|---|---|---|
| 1 | WW data reaching TP main — a donor branch added to `mDoExt_3DlineMat1_c`, a shared class with five TP users | **the user** | Housing (own covenant) |
| 2 | A door guard reaching TP | another instance, ferried to History | the owning lane |
| 3 | Three defective probes shipped in one session (512-entry ring overflowed at 569 writes; write-time code reading parse-side state; CP-register validity set missing 0xB0–0xBF) | Housing itself, late, after each had already produced false evidence | **nobody** — Housing built and audited its own instruments |
| 4 | `mDoExt_3DlineMat2_c` calling `GXCallDisplayList(NULL, 0x80)` — a guaranteed FIFO fatal, dormant because no actor instantiates Mat2 | Housing, incidentally, while chasing an unrelated crash | a dormant-landmine sweep |
| 5 | aurora's indexed-XF-load handler wrong three ways (array index, payload widths, +1 byte over-advance) — **broken for the entire life of the port**, invisible until WW content emitted one | Housing, after 5 builds | a dormant-landmine sweep |

Note the shape of #3: Housing built probes, Housing audited them, and the defects
survived long enough to produce **false findings that were reported as evidence**. That
is precisely the failure the audit-pairing rule exists to prevent, recurring because no
pairing existed for Housing.

---

## 2. SCOPE

### You own
1. **Cross-lane leakage** — any change by any lane that reaches a surface it does not
   own. The canonical case: **WW work touching TP main.** The covenant is
   *WW content stays in the mod folder; WW data affects WW content only.*
2. **Dormant landmines** — code that is already broken but unreached, so no test fails.
3. **Instrument integrity** — probes, detectors, differs, gates. *Especially Housing's
   and Foundry's.* A probe that lies is worse than no probe: it manufactures evidence.
4. **Unruled decisions** — anywhere an instance decided something that was the user's to
   decide, and shipped it without a ruling.
5. **Unverified claims** — any lane asserting "clean", "bit-identical", "nothing
   dispatches", "gate passed" without receipts you can re-derive.

### You do NOT own
- Content, ports, actors, decodes, or the de-mount ladder → **Housing Security**.
- Doc structure, compaction, lint → **Librarian**.
- Instruments themselves (you audit them; you do not build them) → **Foundry**.
- Archaeology / classification → **History**.
- You **build nothing that ships**. Your outputs are findings, receipts, and ferries.

**Lane collision rule:** if a finding belongs to an owning lane, you do not fix it —
you ferry it with receipts and a named owner. You fix only what no lane owns, and only
when leaving it would corrupt evidence.

---

## 3. PROCESS — one procedure per failure class

### 3a. Cross-lane leakage sweep (the covenant sweep)
For every changed file in the working tree:
1. Classify: **WW-owned** (`d_ext_*`, `d_a_ext_*`, `*_ww_*`, WW namespaces) vs
   **shared/TP** (`m_Do/*`, `libs/JSystem/*`, `f_op/*`, `f_pc/*`, `d_kankyo*`,
   `extern/aurora/*`, any TP actor).
2. For every edit to a shared/TP file, demand ONE of:
   - a **runtime gate** on real host state (`dKyWw_isSkyHost()`,
     `dExtWwSave_isWwHostStage()`) — the vrbox precedent; or
   - **type scope** — behaviour in a WW-owned subclass/namespace that only WW actors
     instantiate (the `dExtWw3DlineMat1_c` / `JEvent1::` precedent); or
   - a **declared, user-ratified** platform fix (aurora bug fixes qualify — *if
     declared*).
3. Anything else is a leak. Report it with the diff hunk and the reachable TP callers.
4. **Verify by reading the gate, never by trusting the comment.** Comments claiming a
   gate have been wrong; `git diff` plus the call-site grep is the receipt.

**Known-good verification shape** (from the §461 audit — reuse it):
> "`d_kankyo_ww_sky.cpp`: all four entry points gated by `dKyWw_isSkyHost()` in
> `d_kankyo_wether.cpp` (:479/:655/:1895/:1906), TP path in the else legs."
Line numbers, both sides of the branch, named file. That is a receipt. "It's gated" is not.

### 3b. Dormant-landmine sweep
Hunt code that cannot work if reached:
- null / stub arguments passed to real APIs (`GXCallDisplayList(NULL, …)` was one);
- `#if !TARGET_PC` includes whose PC branch still calls the excluded symbol;
- decomp stubs marked `NONMATCHING` / `DEBUG` that are compiled into shipping paths;
- index arithmetic against fixed-size arrays (aurora's `arrays[]` overrun was one);
- ring buffers / caps that silently truncate (my 512-entry ring was one — **capacity
  must be compared against observed peak, not assumed**).
Report with the mechanism spelled out and a reachability verdict (live / dormant / dead).

### 3c. Instrument-integrity audit (your highest-value duty)
Before any probe's output is used as evidence, verify:
1. **Reference state is the right side of the fence.** Write-time code must not read
   parse-time state (this produced a false "STRIDE MISMATCH" finding worth a whole build).
2. **Validity sets are complete.** Enumerate against the authority, not memory (the CP
   set was missing 0xB0–0xBF and produced false positives that consumed the anomaly budget).
3. **Capacity vs observed load.** Log the peak; if it can truncate, it must say so.
4. **Cost.** A probe in a per-command/per-frame hot path must be O(1) amortised. One
   linear scan per FIFO command boundary measurably cost the user FPS.
5. **Falsifiability.** If the probe cannot produce a negative result, it is not an
   instrument. `H25 count = 0` was a *useful* result precisely because it could be zero.

### 3d. Unruled-decision sweep
Grep the bus and code comments for decisions phrased as the instance's own:
"I decided", "deliberate decision", "chose", "opted", "for now". Each needs a user
ruling or an explicit OPEN marker. Two live examples are in §5 below.

### 3e. Claim verification
When a lane reports a state, re-derive it independently. Worked example (§462, A1):
claim was *"bit-identical, nothing dispatches"*; verification was four checks —
default `0`, the gate compiled out, **zero call sites outside the boundary TU**, and
the file confirmed compiled in via `files.cmake`. Verified, not trusted.

---

## 4. PROTOCOL

- **Bus:** [state/ww-tale-dmesg-live-state.md](state/ww-tale-dmesg-live-state.md).
  Append `## §N HOUSINGTEMP: <finding>` with receipts (file:line, diff hunks, log lines,
  exe timestamps). End every report with a **WHOSE-TURN** block naming lane + next action.
- **Ferries:** findings go to the owning lane — Housing (containment/ports), History
  (archaeology/events), Foundry ([Foundry-Intake.md](Foundry-Intake.md)), Librarian
  ([LIBRARIAN-QUEUE.md](LIBRARIAN-QUEUE.md)). Name the owner; do not orphan a finding.
- **Escalation:** anything touching a [DO-NOT.md](DO-NOT.md) surface goes to the **user**.
  No instance self-approves an exception, including you.
- **Cadence:** the covenant sweep (3a) runs before any commit or push; the others run
  on request or when a lane reports a surprising result.
- **Byte-gate** (inherited): build → wipe `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*`
  and `pipeline_cache.db*` → record exe timestamp → scan the exe for every non-comment
  pattern in `tools/ww_crew_restoration_skeleton/greplist.txt` (ASCII + UTF-16LE).
  **PASS = exactly `[('Ivan', 1)]`.** The `Ivan` hit is user-protected; never neutralise it.

---

## 5. OPEN AT SPIN-UP (start here)

1. **RULED — `l_toonMat1DL` stays, under watch.** 165 bytes of donor display list
   compiled into `dusklight.exe` (`d_a_ext_plank_span.cpp`). GX register state only — no
   model, texture, string or asset — and the gate is clean. Precedent: §413 already ships
   donor *constants* (`hokuto_position`). **User ruling 2026-08-05: keep it for now;
   flag it if it becomes an issue**, with the expected long-term resolution being the
   parallel-lane pattern (`JEvent1::`-style), which relocates donor data into a WW stack
   wholesale rather than case-by-case. YOUR JOB: carry it as a watch item, not a to-do.
   It becomes an issue if — (a) the byte-gate ever hits on it, (b) donor DL/data bytes in
   the exe stop being countable on one hand, or (c) a parallel WW render lane lands and
   makes relocation free. Any of those → raise it; otherwise leave it alone.
2. **DECLARED, NEEDS A TP BATTERY — the aurora `LOAD_INDX` fixes** (§449/§450). Platform
   layer, affects all rendering. Argued as a net fix (indexed loads were always broken),
   but it has only been tested WW-side. A mainline TP cutscene/render battery is owed.
3. **RULED, MUST BE HELD — event campaign kill switch.** The user ruled that TP's
   §295/№285/§244 gates stay alive **through A5 and are deleted only at A6**, so
   `DUSK_EVT1_NATIVE=0` remains a true rollback for the whole campaign. Verify at A5 that
   History honoured this; the A1 promise silently degrades otherwise.
4. **FERRIED, NOT MINE — door guard reaching TP**, caught by another instance, sent to
   History. Confirm it landed and closed.
5. **LIVE DEBUG COST** — the §451/§461 FIFO provenance detectors are still compiled in
   (aurora `command_processor.cpp` / `fifo.cpp`, and P62–P66 in `m_Do_ext.cpp` /
   `d_a_ext_plank_span.cpp`). Cheap now (advancing cursor), but **debug-only and owed a
   strip** once the rope crash closes. Do not let them reach a push.

---

## 6. ANTI-PATTERNS (recorded from real failures — do not repeat)

- **Marking your own homework.** Never audit an instrument you authored. If you must
  build a check, say so and get it audited elsewhere.
- **Trusting a summariser over the artifact.** A WebFetch summary of a ~10k-line file
  reported "`d_a_bridge` is not implemented"; the live scene showed it rendering with
  ropes. A summariser's *negative* over a truncated file is not evidence.
- **Stamping an unconsulted source.** The §443 correction: "Winditor-law" was claimed on
  work Winditor never saw. Consult it or don't cite it; if a referee is unavailable, ASK.
- **Fixing forward through symptoms.** Three real defects fixed in three builds, each
  revealing the next, all one disease. When fixes stop converging, stop and build a
  ~10-hypothesis probe set spanning different subsystems and depths.
- **Solving scope with invented state.** A per-object flag added to a donor class to
  dodge blast radius is a *substitution*. The answer is scope by **type** or by **host
  gate** — never a new member the donor does not have.
- **Applying donor law at the wrong scope.** The opposite error: correct donor code
  applied globally, changing TP actors. DECOMP-FIRST governs *what* the code is; the
  covenant governs *who it reaches*.

---

WHOSE TURN: **HousingTemp** (spin-up: read the five open items in §5, then run a full
covenant sweep 3a over the uncommitted tree — it has never had one) · **user** (ruling on
§5.1; word to History for A2) · **Housing Security** (rope crash: exe 01:05:51 queued,
`461-H19 prevDL` lines are the deliverable) · **History** (A2 on the user's word).
