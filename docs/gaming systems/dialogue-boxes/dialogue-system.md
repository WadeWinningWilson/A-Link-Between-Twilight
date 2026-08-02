# The dialogue-box system

How WW/TP puts text on screen, and the three boxes/paths that matter for the restoration.

## ① What the donor does — the native message machine

One singleton `dMsgObject_c` process (`src/d/d_msg_object.cpp`) drives all dialogue. Its per-frame
state machine is `dMsgObject_c::_execute()` (`:434`), a `switch (getStatusLocal())` over box states:
`waitProc`(1) → `openProc`(2) → `outnowProc`(6) → `outwaitProc`(5) → … → `finishProc`(16) →
`endProc`(17) → `deleteProc`(19). The wait states read the pad **directly**
(`isSend`, `:1749-1766` → `mDoCPd_c::getTrigA/B(0)`) — un-gated by `event_runCheck`, so the same box
works in the field **and** during a demo.

**Screen classes** (chosen by `mFukiKind` in `talkStartInit`, `:1513-1518`): `dMsgScrnTalk_c` (talk),
`dMsgScrnItem_c` (get-item, fukiKind 9, with icon), plus boss/explain/howl/jimaku/select/etc. All share
the text pipeline: the sequence processor fills `pRef->getTextPtr()`, and `_draw` re-pushes it into the
screen every frame (`:571`).

**Box kind is per-message**, derived from BMG `mTextboxType==9` → item box else talk
(`d_mesg.cpp:1958-1964`). There is **no sticky global** in the donor.

## The three paths in the port (know which is which)

| Path | What it is | Input during a demo | Use |
|---|---|---|---|
| **Native demo** `setDemoMessage`/`fopMsgM_messageSetDemo` → `_execute` | The game's own demo-message machine | **Direct pad read — works** | ★ Demo/cutscene dialogue (§201) |
| **`dMsgFlow_c`** (Shade-Watcher / **0x1324 code-text flow**) | Field-dialogue flow; `initWord` injects text | **Event-routed — STALLS** | Field NPC talk; **NOT** demos |
| **ALBW post-man box** (`dALBWDialogue_c`) | The ALBW-style box | n/a | **FORBIDDEN** (DN-4) — crops WW text |

## WW catalog vs BMG (why we inject text)

WW dialogue lines live in a **catalog** — `population/ww_dialogue_full.txt`, keyed `[id]`
(`lookupWwDialogueCatalogLine`, `d_ext_npc_mount.cpp`) — **not** in the TP BMG. So we can't
`setDemoMessage(id)` and have the native box read the right text. Instead we use **code-text
injection**: `dMsgObject_setWord(text)` stores our string in `mWord[200]` (`d_msg_object.h:324`), and
the BMG **code-text entry** (index **4900** = the `0x1324` flow's target) carries a `MSGTAG_UNK_53`
control tag whose sequence-processor case pushes `mWord` instead of BMG bytes (`d_msg_class.cpp:1158`).
So: `setWord(text)` + `fopMsgM_messageSetDemo(4900)` = native box, native wait, native unsuspend, our
text. This is the §201 pivot; details in
[../cutscene-work/demo-message-step-in-step.md](../cutscene-work/demo-message-step-in-step.md).

The `field_0x4cc=1` flag (set by the demo path) keeps `mNoDemoFlag=0` so `endProc` routes to status 19
(`deleteProc` → `unsuspend`) rather than status 18 (flow-pickup) — `d_msg_object.cpp:1362`. That flag is
the whole difference between "native demo box" and "field flow box".

## Pagination

WW's own page-break control codes were dropped at extraction, so one catalog entry can be several donor
pages of flat text. We **pre-wrap** to TP's US box rules (`mountPaginate`, 4 lines / ~38 cols, whole-word
carry, sentence carry) — the "Shade Watcher rule". Multi-page demo messages pre-charge the suspend
counter by N-1 so the storyboard holds across all pages.

## ② Traps
See [traps.md](traps.md) — the ALBW crop, the `dMsgFlow_c` demo stall, the sticky-flag reroute, the
BMG-index-vs-catalog-id confusion, and the `mFukiKind` change-block trap.

## ③ Correct method
Demo dialogue → **native path** (`setWord` + `messageSetDemo(4900)`). Field dialogue → `dMsgFlow_c`.
Never the ALBW post-man box (DN-4).

## ④ Imperfect
Text is injected (code-text), not authored in the BMG; per-message timing modes not yet honored;
pagination uses TP rules not WW's own codes.
