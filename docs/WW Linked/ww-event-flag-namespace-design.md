# WW Event-Flag Namespace — design (Foundry §301; pitfall E, the ceiling)

## The problem (receipts)
WW story actors call `dComIfGs_isEventBit / onEventBit / offEventBit` with **WW
bit indices** — routed today into **TP's** event table, where those indices mean
unrelated things. Receipts: ba1 reads `0x520/0x608/0xE20/0x740/0x780/0x601/
0x602/0x2A20/0x2A80` (variant, dialogue, progression, letter state) against TP's
table; §266 hand-bridged her pre-tale state; every WW story actor has the same
exposure. The donor's own table semantics live in donor
`d_save_event_flag.inc` (the port carries the same include family).

## The consumers (why this is the ceiling)
1. **Dialogue/variant correctness** — Grandma's message selection (post-tale
   letter bit 0x2A20 etc.); every villager's talk trees.
2. **Story-state stage variants** — §300 order B: the exterior door picks
   `Ojhous` vs `Ojhous2` by story state; layer selection generally.
3. **Multi-event sequencing** — "event 2 not before event 1", already-seen
   suppression (History's pitfall E: the gate between one-demo-works and
   a-room-full-of-story-works).
4. **Save persistence** — WW progression must survive save/load without
   colliding with TP progression bits.

## The design (address translation, not behavior change)
**One new save-side WW event-bit array + symbol-level routing for WW TUs.**

1. **Storage:** a WW event-flag block in the save extension
   (`d_ext_save_guard` precedent — WW-side save data already exists there).
   Size = the donor's own `dSv_event_c` bit-array size (donor d_save.h,
   verbatim), so donor indices address it natively. Zero-init = new-game donor
   state; persisted with the existing guard serialization.
2. **Routing (the LinkRM→R_DL01 alias pattern, applied to symbols):** ported
   WW TUs keep their donor calls VERBATIM in source; a per-TU include layer
   (`d_ext_ww_save_route.h`, included by WW story TUs only) maps
   `dComIfGs_isEventBit/onEventBit/offEventBit` → `dExtWwSv_*` which address
   the WW block. TP code and TP tables are untouched; the donor source's
   ARRANGEMENT is untouched — only the address space the symbols resolve to.
   (Same covenant class as SCLS stage-name translation: where-it-lives, never
   what-happens.)
3. **The flag manifest (Space-Kit Pass 1 feeds this):** per-space inventory of
   every WW bit read/written (scan-extractable from the drafts/ports:
   `isEventBit(0x...)` sites) — becomes the acceptance checklist and, later,
   the save-migration receipt.
4. **Migration of existing bridges:** §266's hand-bridged `init_BA1_0` state
   reverts to the donor read against the WW block; the `zeroed-slot` no-ops in
   ported TUs ([INFERENCE-slots]) get their donor reads back.

## Explicitly out of scope
- TP↔WW cross-mapping ("when TP bit X, set WW bit Y") — none exists in the
  donor; any story coupling between the two games' progressions is a design
  question for the user, not this subsystem.
- Event-manager mFlags (event-completion flags) — a different, per-run space;
  already donor-correct.

## Acceptance
1. ba1 dialogue variant correct pre/post tale WITHOUT the §266 hand-bridge
   (donor reads, WW block).
2. The tale's one-shot guard (REVT switch_no path + donor 0x2A20 letter chain)
   holds across save/load.
3. §300 order B pilot: exterior door selects Ojhous vs Ojhous2 by the WW
   story bit once both receptors exist.

## Build order
1. Donor `dSv_event_c` layout receipt (size/accessors, verbatim);
2. `dExtWwSv_*` + save-guard block + route header;
3. Route ba1's TU (first consumer), delete the §266 bridge → acceptance 1;
4. Space-Kit flag-manifest extraction (the per-space inventories);
5. Order-B pilot (variant selection) when both Ojhous receptors exist.
