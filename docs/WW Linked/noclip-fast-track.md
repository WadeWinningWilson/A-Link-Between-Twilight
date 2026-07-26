# noclip fast-track — formalizing systems ahead of port attempts

**The strategy (user-decreed, 2026-07-26):** noclip is NOT the end-all reference — but it is a
**fast track**: for any presentation system we haven't ported yet, read noclip's working TS
implementation FIRST to formalize the system's shape (inputs, data flow, per-frame behavior),
THEN verify against the decomp, THEN spec the port. This inverts the old order (rudimentary
attempt → debug rounds → decomp read → fix) into (working reference → decomp confirm → spec →
one build). The §126-§143 water-color saga cost ~6 rounds; a fast-tracked system should cost 1-2.

**Tier rules unchanged** ([noclip-reference.md](noclip-reference.md)): decomp is law; noclip
disambiguates; never port a noclip constant unverified.

## ⚠ KNOWN LIMITS (measured; grows as found)
1. **NO STORY LAYERING (user-found):** noclip renders placements without the ACT0-ACTb layer
   logic — what you see is NOT filtered by story state the way the game filters. A placement
   visible in noclip may belong to a layer that never coexists with another visible one. **Layer
   truth = the census/DZR layer data + №222 flag→layer ladder, never the noclip view.**
2. Gameplay absent (AI/collision/audio/save) — §145 inventory.
3. It is a renderer of DEFAULTS — time-of-day and weather are user-controlled, not schedule-driven;
   event-gated states don't occur.

## FINDINGS LOG (each entry: what noclip showed → what it fast-tracks → verification route)

### F-1 (user, 2026-07-26): A_mori — enemy IDLE ANIMATIONS play
`noclip.website/#zww/A_mori` (Forest of Fairies, Outset interior — in our census: A_mori, EVNT 6,
hosted in R_DL01): noclip renders the resident enemies WITH their light idle animations — user
reports explicitly for the **moblins** *(user's word; actor code UNVERIFIED — IVAN: A_mori's
vanilla residents need decomp/census confirmation — Bk/Bokoblin vs Mo/Moblin — before any port
uses an identity)*.
- **Fast-tracks:** the enemy/NPC idle-presentation pattern — which anim a placed enemy plays at
  rest, how noclip binds placement → actor type → default anim. Our interiors currently mount
  enemies (if at all) static; this is the reference for "placed actor plays its idle" — the same
  class as the Outset exterior folk idle work (npc-presence-recipe).
- **Verification route:** noclip `d_a.ts`/`LegacyActor.ts` entry for the actor code seen in
  A_mori's DZR → decomp `d_a_<code>.cpp` anim selection → census identity check → then spec.

*(Add F-2, F-3… as explorations find more. Each needs: the noclip URL, what played, the
fast-track claim, and the verification route. User explorations in the museum ARE a research
instrument — screenshots/reports land here.)*

Cross-refs: [noclip-reference.md](noclip-reference.md) (tier + inventory) ·
[islands/Outset/README](islands/Outset/README.md) (A_mori in the interior census) ·
bus §144-§146.
