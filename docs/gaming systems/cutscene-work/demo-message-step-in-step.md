# Demo ↔ Dialogue step-in-step — the crown jewel

How a Wind Waker cutscene (a JStudio STB storyboard) stays **in lockstep** with its dialogue boxes: the
storyboard **freezes** while a box is up and only advances when the player dismisses it. Getting this
right took the longest single detour in the whole restoration (§193 → §201). This doc is the donor
contract, the wrong turns, the correct method, and what's still imperfect.

---

## ① What the donor does (cited — `D:\XXXXXXX\WW DP\src`)

The contract has **two control objects** in `dDemo_manager_c` (`d/d_demo.cpp:609-639`):
`mControl` = `JStudio::TControl` (the storyboard/timeline, holds the suspend counter) and
`mMesgControl` = the message system the JMSG track talks to.

**Suspend is DATA-authored; resume is CODE-driven.** This is the single most important fact:

- **Raise (only site):** the STB **control track** carries a `suspend` sequence command (opcode **type
  4**) — `JStudio/stb.cpp:186-190`, `suspend(val)`. The value is sign-extended
  (`stb.h:84-89`), so the STB can also author its **own** `unsuspend` as a negative type-4. The MESSAGE
  command itself only calls `setMessageCode` (`object-message.cpp:22-30`) — **it does not suspend.**
- **Lower (only code site):** `dMesg_closeProc → getControl()->unsuspend(1)` (`d/d_mesg.cpp:2112`), when
  the box's final page is dismissed and the close animation completes.

**The freeze mechanism** (`d/d_demo.cpp:695-708`):
```cpp
mControl->forward(1);              // ALWAYS called
mFrame++;                          // raw frame — never pauses (the camera-track clock, ~1129)
if (!mControl->isSuspended())
    mFrameNoMsg++;                 // content frame — FROZEN while suspended (the step-in-step clock)
```
Inside `TObject::forward` (`stb.cpp:82-90` / port `libs/JSystem/.../stb.cpp:118`), when
`control->isSuspended()` **every object bails to `STATUS_SUSPEND`, calls `on_wait`, and returns without
advancing its sequence** — camera and actors FREEZE. The **control object itself does not bail** (its
`getControl()` is NULL), so it keeps processing the control track and reaches the data-authored
`unsuspend`. `mFrame` keeps ticking underneath; only object *motion* is frozen.

**The box waits on the pad DIRECTLY.** `dMesg_outwaitProc` advances on `CPad_CHECK_TRIG_A(0) ||
CPad_CHECK_TRIG_B(0)` (`d/d_mesg.cpp:2037`); `closewaitProc` at `:2072`. **Not** gated by
`event_runCheck()` — so it works *during* a demo. This is the linchpin (see traps).

**Multi-page:** the box stays suspended through all pages; page-advance (`mStopFlag==1`,
`d_mesg.cpp:400-403`) loops state 6↔5 without closing; only the final page's `closeProc` unsuspends.

**Get-item box uses the identical contract** — no separate "give item" command. Box kind is chosen
per-message from BMG `mTextboxType==9` (`d_mesg.cpp:1958-1964`); see
[../get-item-boxes/get-item-box.md](../get-item-boxes/get-item-box.md).

---

## ② Traps and mistaken paths (the detour, labeled)

The port originally drove demo dialogue through `dMsgFlow_c` (the "Shade Watcher" / **0x1324 code-text
flow**) with a hand-rolled per-frame poll. Every fix below was chasing a symptom of that wrong
abstraction. Kept here so no one re-walks them.

- **TRAP — auto-advance timer (pre-§193).** The poll advanced pages on a frame timer
  (`doFlow() != 0 || frames >= 1800`). During a demo the pad is event-routed, so each single-line
  chunk auto-completed → boxes flashed, the timeline never held, and the demo **truncated** (storyboard
  end ~855/179 while the camera track runs 1129+). See [storyboard-stb.md](storyboard-stb.md).

- **MISTAKE — §195 skip-forward.** Believing the port's `forward` ignored suspend, I made
  `dDemo_c::update()` **skip `forward(1)` entirely while suspended.** This froze the picture — but also
  froze the **control track**, so the data-authored `unsuspend`/`suspend(-1)` commands never ran → the
  counter got stuck → **frozen forever.** The port's `stb.cpp:118` bail was *already* faithful; skipping
  forward broke the very mechanism that resumes it. **Reverted in §196.**

- **MISTAKE — §196/§197 explicit suspend + drain.** To force the hold I added my own `suspend(1)` on box
  open and a drain on close. This **fought the data-authored suspend/unsuspend commands** (now firing,
  since forward ran again) and spiralled the counter: box 1 drained from **17**, box 2 opened at
  **−14**. Boxes 2+ never held. **Removed in §198.**

- **TRAP — the `dMsgFlow_c` box can't close during a demo.** Its input is **event-routed**
  (`doFlow`/`messageNodeProc`, `d_msg_flow.cpp:537`); with the demo owning the pad, it never converts a
  press into a node advance, so it **stalls at `boxSt=16`** and `d_msg_object.cpp:1414`'s native
  `unsuspend` never fires. §199 hand-rolled a direct pad read + a manual `unsuspend(1)` to compensate —
  which *worked* but was exactly the hand-rolling we were told to stop doing.

- **TRAP — animation loops during the hold.** Even once the storyboard froze, an actor's morf clip
  **looped** (Aryll's wave) because our §52/§53 read-back kept advancing it. Fixed by freezing the
  read-back while suspended — see [actor-driving.md](actor-driving.md) (§197).

- **DISCIPLINE FAILURE — one-hypothesis-per-build.** For several builds I guessed a single fix at a
  time instead of instrumenting many causes at once. The user's correction ("you're no longer abiding by
  the 10 hypotheses rule") produced the §194 probe, which finally discriminated the mechanism in one
  playtest. See [../debugging-methods/multi-hypothesis-logging.md](../debugging-methods/multi-hypothesis-logging.md).

---

## ③ The correct method, in stone — §201 the native pivot

**Stop re-implementing the contract. Drive the box through the game's own demo-message state machine.**

Replace `dMsgFlow_c::initWord(page)` + the hand-rolled poll with:
```cpp
dMsgObject_setWord(page);              // inject our catalog text as code-text (mWord)
fopMsgM_messageSetDemo(4900);          // 4900 = the 0x1324 code-text BMG entry index (dumped §200)
```
Then the native `dMsgObject_c::_execute` (`d_msg_object.cpp:434`) owns everything:

- **Waits** on `mDoCPd_c::getTrigA/B(0)` directly (`isSend`, `d_msg_object.cpp:1749-1766`) — un-gated by
  `event_runCheck`, so it works during a demo.
- **Renders** the `MSGTAG_UNK_53` code-text tag → our `mWord` (`d_msg_class.cpp:1158`).
- **Unsuspends** the storyboard itself in `deleteProc` (`d_msg_object.cpp:1414`).

The decisive detail: `setDemoMessage`/`messageSetDemo` set `field_0x4cc=1` so `mNoDemoFlag` stays 0, and
`endProc` routes to **status 19 (deleteProc → unsuspend)** instead of **status 18 (flow-pickup that
stalls)** — `d_msg_object.cpp:1362`. That one flag is the entire difference between the native demo path
and the field-flow path.

**`forward` runs every frame** (§196 revert) so the native bail freezes the picture and the control
track resolves its own suspends. **Demo actors freeze** via the read-back gate (§197). **Multi-page:**
pre-charge `getControl()->suspend(N-1)` at arm time so the storyboard stays frozen across all N native
boxes, each drawing the counter down by one, resuming only after the final page.

All hand-rolled machinery — the `getTrigA` advance, the §199 `unsuspend`, the re-present HOLD — is
**deleted**. The box is native.

### Where the source lives
- `src/d/d_ext_npc_mount.cpp` — `dExtWw_handleDemoMessage` (arms + `setWord`+`messageSetDemo`),
  `dExtWw_pollDemoMessage` (multi-page chain only).
- `src/d/d_demo.cpp:1178` — `forward` always (§196); `dDemo_setDemoData` actor-freeze (§197).
- `src/d/d_msg_object.cpp` — the native `_execute`, `deleteProc:1414` unsuspend, `mFukiKind` selection.
- `libs/JSystem/.../stb.cpp:118` — the object-bail (already faithful).

---

## ④ What's still imperfect

- **Per-message timing modes — a REAL miss, driven by IN-TEXT codes (not the INF1 `drawType`).** Correction
  of an earlier wrong finding here: the INF1 `drawType` byte (0x0D) is `2` for every tale/awake message, but
  that is only the box-**type** default — it does NOT mean "always wait." The actual per-box timing is an
  **in-text `0x1A` control code** (`group 0x00`, `d_mesg.cpp` tag switch → `mStopFlag`), which our **flat
  catalog dropped**, so every restored box lost its timing and defaulted to wait-for-input. Measured from
  donor `zel_00.bmg` (`scratchpad/bmg_auto2.py`):
  - **`case 4` = AUTO-advance after N frames** (`mStopFlag=2`→`stopProc`, `d_mesg.cpp:517`). Awake **855,
    856** ("Big Brother!") carry `1a07 0000 04 00 1e` = **auto-advance, 30 frames** — user-confirmed our port
    wrongly waits.
  - **`case 7` = input-or-timeout.** Tale **539** (60f), get-item **3095/4410** (10f).
  - **no code = wait-for-input** (857–865, 540–547) — our port already correct.
- **Fix approach (chosen when built):** either (A) inject the RAW WW text WITH its `0x1A` codes as `mWord`
  so the native processor drives `mStopFlag` itself (also fixes name-insert/colour codes — but needs verifying
  TP's `d_msg_class` processes the WW codes), or (B) a per-message mode table (id→case+duration, data in
  `bmg_auto2.py`) applied as an auto-dismiss timer at inject time. (A) is more vanilla; (B) is more contained.
- **Code-text injection is a port mechanism.** WW authors the text in the BMG; we inject via 0x1324
  code-text because our lines live in the catalog. Faithful in outcome, not in data path.
- **Multi-page pagination uses TP box rules** (WW's page-break control codes were dropped at extraction).
- **Not verified frame-for-frame against the donor** — "near-vanilla" by playtest, not yet a
  side-by-side capture.
