# Dialogue-box traps (labeled)

Every one of these cost real builds. Read before touching the message path.

## TRAP — the ALBW post-man box crops WW text (DN-4, HARD STOP)
`dALBWDialogue_c` (the ALBW-style box) crops WW's longer lines, ignores TP's centered formatting, and
never locks the player. **Never** use it as the presenter for WW dialogue. Always the native TP box
(`dMsgScrnTalk_c`, via `dMsgFlow_c` in the field or the native demo path in cutscenes). See
[../../DO-NOT.md](../../DO-NOT.md) DN-4.

## TRAP — `dMsgFlow_c` input is event-routed → stalls during a demo
`dMsgFlow_c::doFlow` advances its box only when the owning actor pumps the node graph
(`messageNodeProc`, `d_msg_flow.cpp:537`). During a JStudio demo the pad is owned by the event, so a
press never becomes a node advance: the box **stalls at `boxSt=16`** and its native `unsuspend`
(`d_msg_object.cpp:1414`) never fires → the storyboard hangs. **Lesson:** for demo dialogue, use the
**native demo path** (`setDemoMessage`/`messageSetDemo`), whose `_execute` reads the pad directly. Do
not drive demo boxes through `dMsgFlow_c`.

## TRAP — the sticky presentation-flag reroute (§187)
We once left a "clothes-get presentation" flag armed after the get, so the **item box hijacked the rest
of the cutscene's dialogue** ("doesn't read as its own text" = rerouted talk text). The donor has **no
sticky routing flag** — it re-derives box kind per message (`d_mesg.cpp:1958-1964`). Any presentation
flag we use as a port mechanism must be **armed narrowly and cleared once the box has opened and
closed** (see [../get-item-boxes/get-item-box.md](../get-item-boxes/get-item-box.md) §202).

## TRAP — BMG index vs catalog id confusion
Two different id spaces bit us repeatedly:
- **`setDemoMessage(index)`** takes an **entries index** → `entries[index].message_id`
  (`getMessageIDAlways`, `d_msg_object.cpp:839`).
- The **`dMsgFlow_c`** path resolves `msg_no` from **INF1** (a different table,
  `setNormalMsg`/`setSelectMsg`, `d_msg_flow.cpp:493/442`).
- Our **catalog** is keyed by **donor message id** (e.g. `[3095]`), a third space.

We burned builds sending the wrong number to the wrong table. **Rule:** the code-text entry index is
**4900** (dumped from the 0x1324 flow, §200); catalog lookups use the donor id; never assume the three
coincide.

## TRAP — the `mFukiKind` override trapped in the change-block (§202)
`mFukiKind=9` (force item box) originally ran only inside `if (mFukiKind != mpRefer->getFukiKind())`
(`d_msg_object.cpp:1440`). Our code-text box is fukiKind 0; if the previous box was also 0 (always true
by the time the get fires), the block never ran and the get rendered as a **talk box**. **Fix:** compute
the *wanted* kind (with the presentation override) **before** the change check so the 0→9 transition
itself triggers the screen rebuild.

## TRAP — a vestigial "clear" that undid the arm (§202)
The 3095/4410 block armed the presentation flag, then **fell through** into a path that unconditionally
`endClothesGetPresentation()`'d it *before the box opened*. **Fix:** the clear must skip the get
messages that just armed it, and the idle-poll that clears the flag must only count down **after** the
box has been seen open (else it clears in the gap between arm and open).

## Meta-trap — patching the wrong abstraction
Most of the above are symptoms of driving demo dialogue through `dMsgFlow_c` (a field-flow) instead of
the native demo machine. The durable fix was architectural (§201), not another patch. When a box
"almost works but stalls/flashes/reroutes," check **which path** is driving it before adding code.
