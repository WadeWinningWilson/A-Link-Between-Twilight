# Tale cutscene ↔ dialogue step-in-step — hypothesis ledger

**Problem:** the Grandma tale cutscene does NOT wait for dialogue-box progression (boxes flash,
timeline plays through), and the get-item box does not visibly appear. Vanilla plays step-in-step:
the STB timeline freezes while a box is up and resumes only when the player dismisses it.

**Discipline:** [[multi-hypothesis-logging]] — every probe tests ~10 competing causes at once. Carry
survivors, refill to 10, record what each build KILLED/CONFIRMED with the discriminating value.

---

## Evidence so far

### §193 build (tale actually ran, frame 120→1320)
- `suspend` stayed ~0 the whole demo; hit **−1** at frame 840 → we UNSUSPEND more than we suspend
  (accounting broken). Timeline `gap` (frame−frameNoMsg) ≈ 0–1 → the STB spent ~1 frame total
  suspended across 1320 frames → boxes never held it.
- Advances logged `A/B=1`; `HOLD` (re-present) fired between. Boxes flashed.
- Get message 3095 fired, set catalog[3095], then immediately `HOLD` → flashed for a frame.

### §194 probe (session = "something else"; NOT NPC dialogue — user talked to no one)
- **BASELINE NEGATED (user, 2026-07-28):** the `boxSt 1→2→6→16` box was NOT NPC dialogue — the user
  spoke to nobody. It was some other message surface (menu/sign/ambient). So "native box holds" is
  **unvalidated** — do not lean on it.
- Still true & useful: **clean trigger edge** (`tB=1 hB=1` press only), `p2A/p3A=0` (PAD_1 real). Those
  reads are input-source facts independent of what the box was → H1/H2/H11 stay killed.
- Zero `active=1` / `susp!=0` — the tale flow never ran here.

### Aryll cutscene observation (user, 2026-07-28) — THE key discriminator
- In **Aryll's WW cutscene**, the **FIRST** dialogue box **waits** for the button press (step-in-step
  works once), but **every box after it does NOT hold the cutscene back** — the timeline runs on with
  the old box still up.
- Mechanism: `isSuspended() == getSuspend() > 0`. Box 1 `suspend(1)`→1 (holds ✓). An EXTRA unsuspend
  on close → **−1** (matches §193's observed susp=−1). Box 2 `suspend(1)`→0 → `isSuspended()` FALSE →
  no freeze → cutscene runs on. **The suspend WORKS; our resume OVER-FIRES and poisons the counter.**
- ⇒ **H-A (no data suspend) is now likely FALSE** — a suspend clearly exists (box 1 holds).

---

## ★ ROOT CAUSE CONFIRMED (§194 run 3, 2026-07-28) — H-E

**The port advances the demo storyboard every frame regardless of the suspend counter.**
`dDemo_c::update()` (d_demo.cpp:1178-1183):
```cpp
if (m_control->forward(1) != 0) {   // called EVERY frame, not gated by suspend
    m_frame++;                       // getFrame() advances unconditionally
    if (m_control->getSuspend() <= 0) m_frameNoMsg++;   // ONLY bookkeeping is gated
}
```
Proof in the trace: while `susp=1`, `fnm` freezes but `f` climbs (gap grew to **1092**), and actor
`NPC_LS` moved through **5 positions**. So the message-box suspend fires correctly (`susp 0→1`) but is
**cosmetic** — it halts only `frameNoMsg`, never the visual timeline. The cutscene runs through, ends
first, and orphans the box (which stalls at `boxSt=16`, never fed input to close in the demo context).

In WW, the STB's data-authored `suspend` halts the control and `forward()` bails EVERY object to
`STATUS_SUSPEND` (stb.cpp:82-90) → camera+actors FREEZE while the box is up. The port's `forward()`
(or the port's §52/§53 WW-actor read-back) does NOT honor that bail. **Fix = gate the storyboard
advance on `getSuspend()>0` (restore the vanilla freeze), OR gate the WW read-back on suspend.**

- **H-E CONFIRMED.** H-K/H-C (accounting) → **not the visual bug** (suspend value is fine; it just does
  nothing). H-B refined: the box DOES suspend; the suspend is simply ignored by playback.
- **New H-L:** the WW-actor read-back (§52/§53) may drive actors independent of the JStudio suspend-bail
  — so even fixing `forward()` might not freeze the cast unless the read-back is also gated. Needs a check.

## §195 FIX APPLIED (freeze forward on suspend, WW-guarded) — carry these if it doesn't fully fix

`dDemo_c::update()` now skips the storyboard advance while `getSuspend()>0` on a WW host stage. If the
fresh trace still shows the desync (or a new deadlock), these are the live carries + new probes:

| # | Hypothesis (post-fix) | Probe / discriminator |
|---|---|---|
| H-M | camera is driven **separately** from `forward()` → freeze holds actors but camera still pans | log demo cam eye/target during a `susp=1` freeze window; compare to actor freeze |
| H-N | freezing `forward()` also stalls the **box's own** advance → box can't reach close → **deadlock** (frozen cutscene) | `boxSt` during freeze: does it still progress 16→17→18, or stick? |
| H-O | box stalls at `boxSt=16` because **nothing feeds it A/B** in the demo (our DN-4 flow, not input) → freeze just makes the hang permanent (safety 3600f catches it) | `tA/tB` + `boxSt` during freeze; does a real press advance it? |
| H-P | our DN-4 `initWord` **re-present** keeps re-opening the box so it never naturally closes → fights the native lifecycle (H-I restated) | disable re-present; does boxSt progress to close? |
| H-Q | `forward(1)` returning 0 sets `m_mode=2` (demo end); skipping it entirely might **delay end detection** oddly | watch demo START/END frames vs the field baseline |
| H-E | **CONFIRMED + FIXED this build** — timeline advanced despite susp; gap→1092, NPC_LS moved | gap should now stay ~0 while a box is up |

Expected if the fix works: during a box, `gap` stays ~0 (timeline held), the cutscene visibly pauses,
the box closes on your A/B, then the storyboard resumes. Deadlock (H-N/O) shows as `gap=0` forever +
`boxSt` stuck + demo never ends until the 3600f safety.

## ★ §195b run (2026-07-28) — SUSPEND COUNTER REACHES 2 (H-R)

`§195b box closed → released storyboard suspend (-> 1)` — my `unsuspend(1)` left it at **1**, so
`getSuspend()` was **2**. Only ONE JMSG message fired (855 "Big Brother!") yet the control suspended
33× / 347 frames and the demo truncated at frame 179. So:
- **H-R (NEW, leading):** the awake scene has **multiple data-authored suspends** (STB control track),
  NOT one-per-message. My §195 freeze holds on ANY `susp>0`, but a single box-close releases only ONE.
  The other suspend(s) keep the freeze on → frozen Aryll. Need the donor's suspend/unsuspend map.
- **H-S:** some suspends are NON-message (camera hold / actor wait / THP), released by a different
  mechanism than a box close — freezing on them unconditionally may be wrong.
- The native box DOES call `unsuspend(1)` itself (d_msg_object.cpp:1414/1942) — so the port has a native
  release path; our DN-4 interception may be bypassing it / double-counting.
- **DECOMP awake-scene research dispatched** (suspend count, what raises/releases each, wait-mode,
  whether frame-179 is a natural end). Implement accounting against that, not against guesses.

## Cutscene message TIMING MODES (user, 2026-07-28) — REQUIRED once the freeze/release works

WW cutscenes are NOT all step-in-step. There are (at least) three per-message modes to support:
1. **Auto-both** — storyboard + box advance together automatically, NO input (box is timed).
2. **Wait-for-input** — box holds the storyboard; player A/B closes the box AND resumes (what we're
   fixing now).
3. **Input-or-timeout** — accepts input to advance, but auto-advances after a set time if none.

⇒ The suspend/freeze must be driven by the message's WAIT MODE, not applied unconditionally. Likely
encoded in the STB message command flags or the BMG message attributes (the `mTextboxType`/wait fields
seen in `f_op_msg_mng.h`). Verify each Aryll/Grandma message's mode against WW before locking behavior —
our current freeze assumes mode 2 for every box, which will be wrong for auto/timed messages.

## §195b release fix (this build)
Box-close now releases the storyboard suspend directly (`s_demoBoxArmed && !active && susp>0 →
unsuspend`), independent of the unreliable owe/finish path. Expected: player A/B closes the box **and**
the storyboard resumes (no more frozen Aryll). Probe watch: after a box, `§195b box closed → released`
should log, `susp` returns to 0, and `f` advances again. If it still hangs, H-N/H-O escalate (the box
never signals `!active`).

## Killed
- **H1 phantom trigger** — §194: trigger is a clean edge (tB=1&hB=1 on press only).
- **H2 held-vs-trig confusion** — §194: held (`hX`) and edge (`tX`) are distinct and correct.
- **H11 pad remap** — §194: p2A/p3A=0, real input on PAD_1.

## Active (10) — for the next tale-run probe

| # | Hypothesis | Status / evidence | Discriminator needed |
|---|---|---|---|
| H-K | **the suspend counter is poisoned by an over-firing resume**: box 1 suspends+holds, an extra unsuspend → −1, so box 2+ `suspend(1)`→0 (not `>0`) → no freeze. "First waits, rest don't." | **LEADING** (Aryll obs + §193 susp=−1) | per-message `susp` on the Grandma run: 1 → −1 → 0-stuck |
| H-A | tale.stb has **no data-authored suspend** → STB never freezes itself | **LIKELY FALSE** (Aryll box 1 holds ⇒ a suspend exists) | — |
| H-B | the **DN-4 `dMsgFlow_c` box never suspends** the STB — only native `dMsgObject` does | LEADING (§194 native holds; DN-4 flashed) | log susp at DN-4 box open |
| H-C | our owe/resume is **unbalanced** → susp goes −1 (over-unsuspend) | CONFIRMED §193 (susp=−1) | balanced explicit suspend removes the −1 |
| H-D | the DN-4 box **auto-completes** (doesn't hold like the native box) | LIKELY | `boxSt` during tale `active=1` |
| H-E | visual timeline **advances despite susp>0** (demo freezes on a different signal) | OPEN (never held susp>0 to test) | force susp>0, watch `gap` |
| H-F | **explicit STB `suspend(1)` on DN-4 box open** freezes it (candidate FIX) | UNTESTED | implement, observe susp held + cutscene pauses |
| H-G | **real input-edge + min-frames gate** makes boxes hold, no flash (candidate FIX) | UNTESTED | implement, observe hold |
| H-H | the tale demo **aborts/truncates early** independent of boxes (frame 0 / 855) | NEW, seen (§194 frame-0, §193 855) | instrument demo-END cause |
| H-I | **two advance drivers race** — our poll AND the box's own advance both fire | NEW | disable our advance; does the box still advance? |
| H-J | routing tale text through the **native `dMsgObject` box** (not DN-4) gives suspend+hold for free (alt FIX) | STRONG (native baseline works) | prototype native path for one tale line |

---

## Next probe/fix (§195) — staged, NOT built

Tests H-B/H-C/H-F/H-G/H-H in one build while keeping the §194 instrumentation:
1. **Explicit suspend accounting** — on DN-4 box open, `getControl()->suspend(1)` + set `s_flowSusp`;
   on final dismiss, `unsuspend(1)` + clear. Retire the owe/resume for the DN-4 path (its −1 source).
   → confirms H-B (was there no suspend?), H-C (does balance kill the −1?), H-F (does it freeze?).
2. **Real-edge + min-frames advance** — track prev `getHoldA/B`; advance only on a fresh 0→1 AND after
   ≥N on-screen frames. → confirms H-G / rules out H-I.
3. **Probe extensions** — log susp at box open; log the demo-END cause (getSuspend magnitude / frame).
   → H-H.
4. If H-B/H-F confirm the DN-4 flow simply can't drive the STB, pivot to **H-J** (native box path).

**Requires a real tale run** (walk to Grandma) — §194 captured regular dialogue only.
