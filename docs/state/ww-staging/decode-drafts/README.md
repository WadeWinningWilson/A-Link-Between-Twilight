# PORT-GRADE DECOMP DRAFTS — the undecoded WW actor bodies

**Produced 2026-08-17 (History/Bridge) on the user's order: *"Use the project's
tools to decode any and all of the nonmatching 20+ ports."*** Pipeline is
Foundry's `tools/foundry/rel_decomp.py` §252 — `dtk split asm → m2c(ppc-mwcc-c++)
→ fopAc offset receipts` — run against the donor RELs on the user's own disc.

## Why these exist

The donor decomp ships these actors as **address-range skeletons**: a function
signature, its `.text` range, and a body that is literally `/* Nonmatching */`.
That marker does NOT mean "compiles differently" — it means **not decoded at
all**. 559 such bodies sat under the Outset port candidates.

## STATUS — read this before using any of it

> **DRAFT. NEVER `MATCH`. NEVER `BYTE-TRUE`.**
>
> These are m2c output with inferred struct layouts. Acceptance is the receiver
> oracle stack (probe differ / state taps / golden traces), not byte-matching —
> the tool's own covenant banner says so and it is right.
>
> Any tracker row derived from these stamps `portable: NONMATCHING` and
> `byte-true: DIVERGENT`. `row_store.py` refuses anything better, at filing,
> and the Integrator refuses it again at build. Both gates are live.

> ### 📍 THESE DRAFTS ARE A WAYPOINT, NOT THE DESTINATION (`DECODER-BRIEFING` §3d)
>
> Foundry's 2026-08-17 amendment re-points where decode work LANDS: upstream
> already ships `d_a_npc_ko1.cpp` as a structured scaffold — 601 lines, 139
> definitions with mangled names and address ranges, the class hierarchy, and
> the member block as a placeholder — **with a build, a diff tool and a PR path
> attached.** The decoding here is real and ko1 is genuinely `NonMatching`, but
> **the work product belongs in that scaffold, not in these `.draft.cpp` files.**
> Treat this directory as the m2c staging floor it is; do not grow it as if it
> were the deliverable.

## What is in each file

| file | fns | notes |
|---|---|---|
| `d_a_npc_ko1.draft.cpp` | 139 | Ko1 + Ko2 (one TU, two actors) |
| `d_a_npc_so.draft.cpp` | 123 | NpcSo |
| `d_a_npc_aj1.draft.cpp` | 94 | Aj1 |
| `d_a_npc_ym1.draft.cpp` | 86 | Ym1 + Ym2 (one TU, two actors) |
| `d_a_npc_yw1.draft.cpp` | 82 | Yw1 |
| `d_a_npc_ob1.draft.cpp` | 78 | Ob1 |
| `d_a_saku.draft.cpp` | 29 | Ksaku |
| `d_a_pt.draft.cpp` | 23 | Puti |
| `d_a_bridge.bridge_move.draft.cpp` | 1 | the single undecoded fn (2,672 B) |
| `rodata_tables.h` | — | 98 extracted .rodata tables the drafts reference |

**`[INFERENCE-NEEDED]` markers are the honest part.**

> ### ⚠️ THE COUNT BELOW MOVED — CORRECTED IN PLACE 2026-08-17
>
> This paragraph read **"1,224 across the set"** and that figure is **STALE**.
> The Decoder lane burned it down over 2026-08-17. Their last reported figure
> was **1,159**; my independent re-measure of the same drafts returned
> **1,162**, and **that 3-marker discrepancy was never resolved** — it is not
> a rounding difference, it is two counts of the same files that disagree.
>
> **Treat any absolute marker count in this file as a SNAPSHOT, not a current
> total.** Re-count from the drafts if the number matters.
>
> *Corrected here, at the point of use, rather than by a banner at the top:
> the correction existed in CALLS rows from the moment the burn began and
> never reached this line — a reader landing here saw 1,224 as current for a
> full day. Housing Security named the shape on the `SPEC-ww-generic-prop`
> class-C table the same day: **presence in the document is not presence at
> the point of use.** This is that failure, in my own artifact, found by
> applying their finding to myself rather than by my own keyword sweep — which
> passed this file clean, because a superseded MEASUREMENT carries no
> retraction language for a keyword to catch.*
Offsets inside `fopAc_ac_c` (0x000-0x290) carry a REAL member name and a receipt
from `f_op_actor.h`. Everything past the base is an m2c *inference* and is
labelled as such — it needs a name from sibling-template analogy or the debug
maps before it is trustworthy.

## The 17 that needed no pipeline

Seventeen bodies are **4 bytes** — one PowerPC instruction — and every one is
`void`. A void function whose entire compiled form is one instruction is `blr`:
**the function is empty in the retail game.** Decoded with certainty from size
and signature alone, no disassembly required:

`daSalvage_c::debugDraw` · `daNpc_Ym1_c::{chngAnmTag,ctrlAnmTag,ctrlAnmAtr}` ·
`daNpc_Yw1_c::{chngAnmTag,ctrlAnmTag,ctrlAnmAtr}` ·
`daNpc_Aj1_c::{chngAnmTag,ctrlAnmTag,cut_init_VIVRATE,cut_init_JMP,cut_init_SPPRISE,cut_init_LOK,cut_init_DAN}` ·
`daNpc_So_c::{modeWait,modeDebug,modeGetRupeeInit}`

**That set carries a finding:** `chngAnmTag`/`ctrlAnmTag`/`ctrlAnmAtr` are empty
in Ym1, Yw1 AND Aj1 — so the WW message anime-tag channel is **inert for those
three NPCs by donor design**, not by a gap in our port.

> ### ⚠️ NARROWED 2026-08-17 — read this before quoting the paragraph above
>
> **As first written that finding over-reached, and the Decoder lane caught it.**
> It named the three NPCs whose bodies are 4-byte `blr`s and then generalised to
> "the anime-tag channel", which invited the reading *the channel is dead
> everywhere*. **It is not.** ko1's is live:
>
> | actor | `chngAnmTag` range | size | verdict |
> |---|---|---|---|
> | **ko1** | `000017A4-000017DC` | **56 B / 14 instr** | **LIVE — real code** |
> | ym1 | `000010D0-000010D4` | 4 B / 1 instr | empty (`blr`) |
> | yw1 | `00001288-0000128C` | 4 B / 1 instr | empty (`blr`) |
> | aj1 | `00000ED8-00000EDC` | 4 B / 1 instr | empty (`blr`) |
>
> ko1 was never in the 17 because it is not 4 bytes — the sweep that produced
> this list could only ever find empties, so it **could not have discovered a
> live one.** An absence found by a probe that only looks for absences is not
> evidence of absence.
>
> **Correct statement: the tag channel is retail-empty in three of Outset's NPCs
> and LIVE in ko1.**
>
> **This is load-bearing for the port, not just for the ledger.** The anime-tag
> mailbox ported on 2026-08-17 (donor one-slot, `0xFF` sentinel, consumer-clears,
> module-static; reset wired at `src/d/ext_plugin/ww_stage_loader.cpp:467`) was
> filed as speculative — *"ports the CHANNEL, not the tag SOURCE… behaviour
> identical to today."* **With ko1 live it has a real driver the moment ko1
> lands.** Do not strip it as dead code.
