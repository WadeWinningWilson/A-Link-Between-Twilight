# tale.stb — command timeline (§298 golden trace)

**Lane: Foundry (instruments/forensics). Deliverable for History's §298 ask.**
Decoded from the LIVE staged copy
(`model_replacements/WW-Crew-Restoration/arcs/Demo01.arc :: tale.stb`, 2,684 bytes),
which is **byte-identical to the donor original** (`D:\XXXXXXX\Ex WW\files\res\Object\Demo01.arc`,
md5 `4d254c72e17cc8e7647517a76f1435e1`) — this timeline IS the donor authoring, verbatim.

Instrument: `tools/ww_crew_restoration_skeleton/stb_timeline.py` (new; read-only,
№31-clean). Every parse rule transcribed from the receiver's own processor —
sequence opcodes from `TObject::process_sequence_`
([stb.cpp:190](../../libs/JSystem/src/JStudio/JStudio/stb.cpp)), paragraph decode from
`TObject_message::do_paragraph` (group `0x42`, op `0x19` = `setMessageCode`,
jstudio-object.cpp:899 + donor `JStudio_JMessage/object-message.cpp`), control-block
identity from `TParse::parseBlock_object` (block type `BLOCK_NONE (-1)` →
`TObject_control`, stb.cpp:439).

## Answer 1 — every suspend/unsuspend on the control track

The control track is: `wait` / `suspend(1)` pairs, nine times, then END. Nothing else.

| # | control-track frame | op | file offset |
|---|---|---|---|
| 1 | 29 | `suspend(1)` | +0x0A00 |
| 2 | 95 | `suspend(1)` | +0x0A08 |
| 3 | 149 | `suspend(1)` | +0x0A10 |
| 4 | 240 | `suspend(1)` | +0x0A18 |
| 5 | 270 | `suspend(1)` | +0x0A20 |
| 6 | 294 | `suspend(1)` | +0x0A28 |
| 7 | 383 | `suspend(1)` | +0x0A30 |
| 8 | 568 | `suspend(1)` | +0x0A38 |
| 9 | 694 | END | +0x0A48 |

**There are ZERO `unsuspend` ops anywhere in the file — on the control track or any
other.** (A data-authored unsuspend would be a negative-amount suspend word; none
exist.) There is also a 9th suspend at frame 668 (+0x0A40) before the END at 694.

**What suspends "at/near frame 198": nothing.** No suspend op sits at 198. The
nearest below is **#3 at frame 149**; the nearest above is #4 at frame 240. Frame
198 is a **MESSAGE** frame (see answer 3).

## Answer 2 — the release mechanism

**Message-linked box-close, exclusively.** Receipts:

- The file authors no unsuspend and no branch — release cannot come from data.
- `TControl::unsuspend(1)` = `suspend(-1)` on the control object
  ([stb.h:165](../../libs/JSystem/include/JSystem/JStudio/JStudio/stb.h)); the donor's
  only message-flow caller is `dMesg_closeProc` → `dComIfGp_demo_get()->getControl()->unsuspend(1)`
  (donor d_mesg.cpp:2112) — fired **only when the box finishes its close animation**.
- While the control object's counter is >0, `TObject::forward` parks **every** track
  in `STATUS_SUSPEND` (`control->isSuspended()` check, stb.cpp:118), and
  `TControl::isSuspended()` reads the counter **cached at the top of the previous
  forward** (`_54`, stb.cpp:353).

So the donor cadence for all nine beats is:

```
JMSG: setMessageCode(row)      ← box starts opening
  … a few frames of open-animation lead …
CTRL: suspend(1)               ← whole storyboard holds, box is up
  … player reads, dismisses …
dMesg_closeProc → unsuspend(1) ← the ONLY release; storyboard resumes
```

## Answer 3 — every MESSAGE op

Nine `setMessageCode` paragraphs on the JMSG track (group 0x42, op 0x19):

| frame | code | paired suspend | note (Grandma README §231 rows) |
|---|---|---|---|
| 21 | **539** | @29 | greeting (player-name insert) |
| 74 | **540** | @95 | birthday lead-in |
| 145 | **3095** (0x0C17) | @149 | **OUTLIER — outside the 539-545 tale block** |
| 198 | **541** | @240 | "don't look so disappointed…" |
| 247 | **542** | @270 | olden-days tradition |
| 272 | **547** | @294 | (outside 539-545; second outlier) |
| 315 | **543** | @383 | the family shield on the wall |
| 467 | **544** | @568 | (pre-give beat) |
| 620 | **545** | @668 | closing line |

Message↔suspend pairing is 1:1 and in order — nine messages, nine suspends. The
**frame-198 event is message row 541 (the 4th box), whose hold is the suspend at
240**, not a suspend of its own.

## Answer 4 — branch commands before ~198

**None.** Zero type-3 (branch/jump) sequence words exist anywhere in tale.stb — not
before 198, not after, on any track. No flag-ops (type 1) either. The file is
straight-line: waits, suspends, paragraphs, END. Nothing can route past a release.

## Diagnostic read (for History to reconcile with the runtime trace)

- With **no adaptor-driven box** (message adaptor never firing), the first hold is
  suspend #1 at control frame 29 — the storyboard would visually freeze there, with
  exactly **one outstanding suspend(1)** and `m_frame` continuing to tick (forward
  returns true while suspended, so the caller's `m_frame++` keeps counting — a
  logged `m_frame` of 198 is **wall-clock since start, not storyboard progress**).
  One outstanding suspend + frozen visuals matches ANY of the nine holds; `m_frame`
  alone cannot say which.
- **Discriminator to log next** (one probe, multi-hypothesis): each
  `setMessageCode` value as the JMSG adaptor receives it (or its absence), plus
  every transition of `mObject_control.getSuspend()`. That yields: (a) whether the
  adaptor fires at all, (b) which of the nine holds is the stuck one, (c) whether
  code **3095** (the outlier at 145 — likely a different BMG bank than the 539-545
  run) resolves — a box that fails to open for 3095 would strand suspend #3 @149
  with no close ever coming.
- Special beats also on the clock after release-chain completion: `d_act2`
  DATA_ID(4) @675, `d_act3` DATA_ID(9) @680 — **CORRECTED (§322/§323): channel 9 is the
  BLACK FADE (payload: dir byte 0=out else in, then frames) — this is the tale's ending
  fade-OUT, not "give/handover" as first written here** (channel 10 = white fade),
  control END @694.
