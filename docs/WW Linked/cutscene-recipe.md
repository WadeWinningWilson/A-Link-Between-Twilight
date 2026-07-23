# Cutscene Recipe — porting a donor storyboard end to end

> ## READ FIRST — check the DONOR DECOMP, not just the receiver
>
> **Decomp source: `D:\XXXXXXX\WW DP\src`**  (arcs: `D:\XXXXXXX\Ex WW`)
>
> The receiver's source tells you **what** an API is. Only the donor tells you **how its own
> actors called it** — flags, argument order, order of operations. Guessing them looks like a
> working port until it silently isn't. Every hard bug in this recipe (№163–№184) was found in
> the donor and would have been cheaper if read first.
>
> Procedure: find the donor's equivalent (`d_a_npc_*.cpp`, `d_demo.cpp`, `d_event*.cpp`) →
> copy its call shape **verbatim** → cite the donor function in a comment at the ported site.

> ## READ FIRST — `OffsetPos` applies to EVERY Great Sea space
>
> A donor event's `PACKAGE: PLAY` cut carries **`OffsetPos`**, handed straight to
> `dDemo_c::start(demo_data, xyzdata, offsetAngY)`. It is the origin the storyboard stages its
> **cast** from, and it crosses over from the donor **verbatim** — donor world coordinates are
> not receiver world coordinates.
>
> **The camera proves nothing about the cast.** `Center`/`Eye` come from a different field and
> are usually already correct, so a scene can frame exactly the right spot while the performers
> stage into open ocean. `merge_event.py` now prints both side by side on every merge — read it.

This is the proven pipeline for bringing a donor cutscene into the port, written from the one
that shipped (Outset's opening, `awake`). Companion doc:
[cutscene-animation-recipe.md](cutscene-animation-recipe.md) for making the **cast perform**.

Ledger entries: №149–№184.

---

## 0. What you are actually porting

Five separate things, each of which can fail independently and silently:

| Layer | Lives in | Fails as |
|---|---|---|
| The **event** | stage `event_list.dat` | `getEventIdx -> -1` |
| The **order** | event manager | `ORDER -> 0` |
| The **archive** | `Demo##.arc` | ordered, nothing plays |
| The **storyboard** | `awake.stb` inside that arc | ordered, nothing plays |
| The **cast** | actor binding + read-back | plays, but nobody moves |

Diagnose in that order. Each layer below is only meaningful once the one above it is green.

---

## 1. The event container is SHARED — merge, never replace

Donor and receiver use the same structs: header `0x40`, `dEvDtEvent_c` `0xB0`, `dEvDtStaff_c`
`0x50`, `dEvDtCut_c` `0x50`, `dEvDtData_c` `0x40`. So a donor event parses with the receiver's
own reader and this is a **data merge, not a format conversion**.

```
python tools/ww_crew_restoration_skeleton/merge_event.py <event-name>
```

It copies the event's dependency closure — event → staff[] → each staff's cut chain (`mNext`)
→ each cut's data chain → each data node's payload — renumbering every index on the way in.

**Never replace the stage's `event_list.dat`.** Doors and other stage events live in it; a
wholesale overwrite takes them with it. If the stage has none, the tool starts a fresh
container (the doors' events come from the global list and are unaffected).

Verify by round-trip read, not by file size.

---

## 2. The camera cut must be `STBWAIT`, not `FIXEDFRM`

`d_camera.cpp` holds parallel `ActionNames[34]` / function-pointer tables:

- **`FIXEDFRM`** (idx 4) pins the camera to fixed Center/Eye/Fovy and **ignores the storyboard**.
- **`STBWAIT`** (idx 27) → `dCamera_c::stbWaitEvCamera`, which reads `dDemo_c::getCamera()`
  **every frame** and copies target/eye/fovy from the running storyboard.

Donor events often come across as `PAUSE -> FIXEDFRM`. If the STB owns the camera (it usually
does — check for a `JCMR` block), that static shot silently wins and the pan looks "missing".

```
python tools/ww_crew_restoration_skeleton/patch_event_cut.py <stage.arc> FIXEDFRM STBWAIT
```

Data fix, no rebuild. The tool refuses on 0 or >1 matches and refuses when the trailing bytes
are not zero padding — a blind string replace inside an arc corrupts a neighbouring field
silently.

---

## 3. The demo ARCHIVE is not the one you set on the event

**This is the trap that cost the most.** `getStbDemoData(resName)` resolves the storyboard via
`dComIfG_getObjectRes(dStage_roomControl_c::getDemoArcName(), file)` — **not** via
`eventInfo.setArchiveName()`. That name is normally populated from the room's **LBNK** chunk as
`"Demo%02d_%02d"`. An authored room dzs without LBNK leaves it **empty**, the lookup returns
NULL, and `JUT_ASSERT` is inert in release — so `dDemo_c::start(NULL, ...)` does **nothing,
silently**.

Either emit an LBNK chunk, or name the arc directly (any name works if the archive is resident
under it).

**The load is ASYNC.** `d_s_room.cpp` never uses the name the frame it is set — it spins on
`dComIfG_syncObjectRes` and retries while `phase > 0`. Ordering in the same frame you request
the archive reproduces the identical symptom one layer down.

> **Contract:** `phase > 0` retry · `phase < 0` error · `phase == 0` ready.

---

## 4. Ordering is a GATE, and a once-flag must be set by SUCCESS

`fopAcM_orderOtherEventId` refuses on its first line when `isOrderOK()` is false — that is
`mEventStatus == 0 || mEventStatus == 2`. A non-zero status means another event owns the slot
**this frame**.

```c
if (!dComIfGp_getEvent()->isOrderOK()) return;   // no latch — retry next frame
const s32 ok = fopAcM_orderOtherEventId(player, idx, 0xff, 0xffff, 40, 1);
if (ok == 0) return;                             // no latch — a refusal must not be permanent
s_ordered = true;                                // latch ONLY on success
```

**Latching on failure turns a transient busy frame into a permanent one.** This bit us exactly
once and looked like "the feature doesn't work".

Arrival code is a common competitor: our own warp/door machinery force-ends events and snaps
the camera. Gate any such snap on `dComIfGp_event_runCheck() || dDemo_c::getCamera() != NULL`,
and **defer rather than cancel** so control still hands back when the scene ends.

---

## 5. Messages: donor ids, receiver table, and a suspend contract

The `JMSG` track fires **donor message indices** on a timed schedule. Those indices are
meaningless in the receiver's table — they land on unrelated shipped strings (855 → a pumpkin
line, 80 → a quest-log string). **№31 applied to text: an unmapped id is SUPPRESSED, never
passed through.** A missing line beats a foreign one.

Choke point: `jstudio_tAdaptor_message::adaptor_do_MESSAGE` → `dMsgObject_setDemoMessage`.

**R6:** replacement lines live in `<mod>/dialogue/demo_messages.ini` (`id = line`), never in
source. The covenant gate proves it.

### The contract you inherit by taking over the box

The engine **suspends the storyboard** while a demo message is up, and the message system
resumes it on close — `dDemo_c::getControl()->unsuspend(1)` (`d_msg_object.cpp` 1412 / 1903).
Suppress the native box and you remove the **resume** while leaving the **suspend**: the scene
runs on visually while its own clock (`m_frameNoMsg`) stands still, and every later message
never fires.

Rules that came out of that:

- **Take over a display ⇒ take over its whole contract**, not just the visible half.
- **Owe the resume, never pay-or-forget.** `resumeDemoAfterMessage` silently no-ops when
  `!isSuspended()`, and the STB may suspend *after* the MESSAGE op. Clearing the debt beside a
  call that can silently no-op converts a race into a permanent failure. Clear the debt by
  **payment** only.
- **A hold timer, not a keypress, is the guarantee.** `checkDismiss()` reads
  `mDoCPd_c::getTrigA`, which the cutscene consumes — so A never arrives and the scene
  deadlocks. Auto-resume after a readable beat; A dismisses early but is never required.

---

## 6. Diagnosing: two clocks, and probes that measure the wrong thing

`dDemo_c` keeps **two** counters and confusing them costs a full debug round:

- `m_frame` ticks on every `forward()` — **a fully suspended storyboard still raises it**.
- `m_frameNoMsg` advances only while `getSuspend() <= 0`, and the WAIT cut times off *that* one.

`gap == 0` ⇒ storyboard time is live. `gap > 0` ⇒ something suspended it, and where it starts
diverging says what. **Magnitude matters:** `suspend 1` = waiting on a message; `suspend 100` =
the PLAY cut's already-finished case (`d_event_data.cpp` 1311) — opposite problems, and
`unsuspend(1)` will not clear the latter.

> **The probe lesson, learned the expensive way:** a probe that measures **intent** cannot
> falsify a claim about **output**. Position, scale, base matrix and "draw was entered" are all
> intent. See the animation recipe — a model can report a perfect position and still render
> somewhere else entirely.

---

## 7. Checklist

- [ ] Event merged (not replaced); round-trip verified
- [ ] `OffsetPos` reconciled against the scene's receiver-space position
- [ ] Camera cut is `STBWAIT` if the STB owns the camera
- [ ] Demo archive resident **and** synced (`phase == 0`) before ordering
- [ ] Order gated on `isOrderOK()`; once-flag latched on success only
- [ ] Arrival camera snap / event force-end deferred while a demo runs
- [ ] Message ids mapped in mod data, unmapped ones suppressed
- [ ] Suspend/resume debt owed and paid, with a hold-timer fallback
- [ ] Cast performs → [cutscene-animation-recipe.md](cutscene-animation-recipe.md)
