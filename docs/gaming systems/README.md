# Gaming Systems — how WW's runtime systems actually work, and how the port reproduces them

This tree is the **teachable record** of the interconnected TP/WW runtime systems we restored while
getting the Outset **awake** cutscene (Aryll) and the **tale / Hero's Clothes** cutscene (Grandma)
working. It exists because that work was long, error-prone, and full of dead ends — and because the
lessons kept living only in bus entries, memory files, and my own head. This is the "how it actually
works" view; the debt and hard-stop views live elsewhere (see **Companion docs** below).

## The one law

> **Donor = spec. We do what Wind Waker does, every time.**

Faithfulness is not a proposition and never an "option" to weigh against a port shortcut. When the
donor's behavior is known, we match it. Where we *can't* match it 1:1 (TP's engine differs), we use a
**port mechanism that reproduces the player-perceived result** and we **log that as a debt** in
[port-liberties.md](../WW%20Linked/port-liberties.md) — never as a design decision.

Corollary, learned the hard way: **DECOMP-FIRST.** Read how the donor's own code does a thing before
writing a line. Every time we skipped this it cost builds (the geometry saga, the message-index drift,
the whole step-in-step detour). Every doc here cites donor `file:line`.

## How the tree is organized

Each area doc follows the same spine where it applies: **① what the donor does (cited) → ② the traps /
mistaken paths we hit (labeled) → ③ the correct method, in stone → ④ what's still imperfect.**

| Folder | Covers |
|---|---|
| [dialogue-boxes/](dialogue-boxes/) | The native `dMsgObject_c` / `dMsg_Execute` message machine, code-text (0x1324) injection, WW catalog vs BMG, and the traps that cost us weeks |
| [get-item-boxes/](get-item-boxes/) | The clothes-get item box: `mTextboxType==9` / `mFukiKind==9`, the icon, catalog text, and the message-index drifts |
| [cutscene-work/](cutscene-work/) | **The crown jewel** — demo↔dialogue step-in-step; storyboard/STB structure; demo-actor driving; fades |
| [models-and-lighting/](models-and-lighting/) | BDL/BMD parse-timing (DN-3), cel-shade lighting (MAJI blacks ZAtoon) |
| [region-triggers/](region-triggers/) | TagEv 0x0A region-fire, SCOB scale convention, the geometry saga |
| [receiver-architecture/](receiver-architecture/) | **TP receiver optimization** — how WW content runs in TP; host-stage gating, resolvers, the "reuse the native machine" lesson |
| [debugging-methods/](debugging-methods/) | Symbolication, and the multi-hypothesis-logging discipline that finally unstuck us |

## Current status — honest

The two cutscenes are **functionally working and near-vanilla**, driven by the game's own message
state machine. They are **not yet 100% vanilla**, and this tree says so plainly wherever it's true:

- **Per-message timing modes — a REAL miss (in-text codes, not INF1 `drawType`).** Some boxes auto-advance
  in vanilla (awake 855/856 = "Big Brother!" auto-advance after 30 frames via an in-text `0x1A` case-4
  code); our flat catalog dropped those codes so every box waits for input. Confirmed against vanilla +
  measured from donor `zel_00.bmg`. Fix pending (preserve the codes, or a per-message mode table). See
  [cutscene-work/demo-message-step-in-step.md](cutscene-work/demo-message-step-in-step.md).
- **Character animation during a message wait.** Vanilla keeps the character's clip looping while the box
  waits; §197 wrongly froze it, reverted in §203 (the read-back runs so the morf loops while the JStudio
  bail holds position).
- **WW text is injected as code-text (0x1324), not authored in the BMG.** Faithful in outcome, but a
  port mechanism, not the donor's data path.
- **Demo-prop lighting** uses the port's WW-item recipe, not WW's exact demo lighting (L-2).
- Multi-page pagination uses TP box rules, because WW's own page-break control codes were dropped at
  extraction.

## Companion docs (not duplicated here)

- [../DO-NOT.md](../DO-NOT.md) — **hard stops.** Permanently rejected approaches (DN-1…DN-4). Read
  before touching anything they name.
- [../WW Linked/port-liberties.md](../WW%20Linked/port-liberties.md) — the **debt ledger** (L-1…L-4):
  every place we used a port value instead of a donor byte, with its reconciliation condition.
- [../state/tale-step-in-step-hypotheses.md](../state/tale-step-in-step-hypotheses.md) — the live
  hypothesis ledger from the step-in-step investigation (the raw problem-solving trail).
- [../WW-Restoration-Cookbook-CANONICAL.md](../WW-Restoration-Cookbook-CANONICAL.md) — the **archival
  origin** record (how the restoration work began; user-ruled archival 2026-07-27). Its *process*
  content is superseded by this tree; the standing **laws** in its ADMITTED DOCTRINE remain in force.

## The §-number system

Throughout this work, checkpoints are tagged `§N` (e.g. `§201`) in code comments and logs. The arc that
produced these docs runs roughly **§180 → §202**. Where a doc cites `§N`, that number is greppable in
the source and in playtest logs — it's the breadcrumb trail of what changed when.
