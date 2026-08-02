# Foundry — the methods lane (charter)

> Spun up 2026-07-28 on user order. Opening brief = bus §195 (P1–P4 proposal) +
> §196 (instrument inventory + lane endorsement). Handle accepted: **Foundry** —
> the lane that builds what the other lanes work with.

## Why this lane exists

The strategic problem (user, §195): methods evolve too slowly for the project's
scope. Acceptance runs through a **human oracle** — the user's eye and memory —
whose deep knowledge is Outset-centric. At current methods the user would have to
play through Wind Waker in its entirety multiple times to source accurate data,
and post-Outset content is an ocean where invisible errors could silently reset
accepted work. Foundry's mandate: **replace user playtime and user memory with
instruments and measurement**, fast enough that method capability grows *ahead*
of content need, not behind it.

**Boundary sentence:** Foundry builds instruments and doctrine. It does **no
ground-level content work** — no donor diffing for a specific fix, no census
interpretation, no restoration code. When an instrument produces data, the data
ferries to the owning lane; Foundry improves the instrument.

## Owns

- **The instrument estate:** DuskTap (patched local Dolphin at
  `D:\Dolpheen Plz\Local Dolphin`, tap roster, capture SOPs), the probe-event
  schema + offline differ (P2), the reel + save-state library and replay harness
  (P1/P5), the golden-trace library + drift gate (P3), DuskLog conventions on
  the receiver side (**spec only** — Engine lands code), and successor
  instruments.
- **The methods doctrine:** oracle-stack (P4), capture SOPs + determinism
  checklists, per-island fact sheets / knowledge-debt ledgers (P6).

**Not owned:** `ww_bridge` (Bridge's), content judgments (History), containment
gates (Housing), doc structure (Librarian), receiver implementation (Engine —
Foundry specs hooks, Engine lands them).

## Audit pairing restored

Housing built DuskTap out of necessity (§189–§190) — a standing violation of
LANES.md's "Housing builds nothing" and of builder ≠ auditor. **Ownership of
DuskTap and all future instruments transfers to Foundry.** Housing returns to
pure audit and negative-controls Foundry's instruments like any shipment: can
the differ report a broken trace? does an empty capture say UNKNOWN, not MATCH?

## The program

Ratified base (§195, endorsed §196):

- **P1 — Capture reels.** Record Dolphin input movies (.dtm) + save states
  during every already-planned vanilla session. Any future data need = replay
  with new taps, zero user play time. Save-state library per island dock.
- **P2 — Mechanized parity.** One probe-event schema across DuskTap (donor) and
  DuskLog (receiver) + a Foundry-built offline differ → "vanilla-law" becomes a
  computed verdict. Pilot on an already-accepted system (windline/waves) to
  calibrate the differ against known-good.
- **P3 — Golden traces.** Accepted system + fixed route → golden capture; diff
  after merges / before pushes → silent drift names itself (№31 which-bank
  probe = the working miniature).
- **P4 — Oracle-stack doctrine.** decomp law → DuskTap measurement → noclip →
  community video → user memory (**tie-breaker only**). The user's scaling role
  is instrument operator + arbiter, not encyclopedia.

Foundry extensions — the "past what they are" part:

- **P5 — Headless harvest farm.** DolphinNoGUI (already built) + the reel
  library + a tap ini = batch re-harvest of ANY new question against EVERY
  recorded route, overnight, unattended. This is the kill-shot on "multiple
  full playthroughs": each route is played **once, on record**; every future
  tap set replays it.
- **P6 — Island fact sheets (knowledge-debt ledger).** For each post-Outset
  area, machine-generate the donor ground truth BEFORE restoration starts:
  DZR/census data (Bridge) + one recorded sail/walk reel → emitter census, SE
  census, spawn roster, palette timeline, event traces (the §196 harvest,
  productized per island). Outset-centric memory stops being the bottleneck.
- **P7 — Regression sentinel.** Golden traces wired into the workflow as a
  drift gate: scripted receiver route after builds, DuskLog trace diffed
  against the accepted golden — invisible resets get named before a human
  replays anything. Honest limit: needs receiver-side determinism/replay work
  (Engine hook, Foundry-spec'd; roadmap, not sprint 1).
- **P8 — Instrument conversion.** Every repeated manual method gets converted
  into an owned tool — the standing mitigation for Cursor usage-gates (§195
  honest-limits).

## Honest limits (carried from §195)

Actor state machines still need decomp reads. Feel-judgments still want an eye.
Determinism is an engineering problem, not a given — every reel is validated by
record → replay → tap-log diff **before** it enters the library.

## Protocol fit

Ferries route through the user like any lane's. Bus = `WW Linked/`
[ww-bridge-tool-interconnected.md](WW%20Linked/ww-bridge-tool-interconnected.md)
(§N entries tagged **Foundry**). Live state =
[state/foundry-methods.md](state/foundry-methods.md). Housing audits Foundry's
instruments; the Librarian maintains this file's structure, not its doctrine.
Foundry holds no content turn in Interconnected Runs.
