# The oracle stack — how WW-restoration claims get accepted (P4 doctrine)

> Foundry, 2026-07-29 (bus §208; ratified program §195 P4). The standing cure for the
> human-oracle bottleneck: **demote memory to tie-breaker; promote measurement.**
> Applies to every lane's acceptance decisions, not just Foundry's instruments.

## The stack (consult top-down; higher rung wins disagreements)

1. **Decomp law** — the donor's own source (`D:\XXXXXXX\WW DP`, 72%+ and climbing; 372
   per-actor debug maps). If the donor's code says how it works, that IS the spec.
   DECOMP-FIRST rule stands: read how the donor does it before fixing the receiver.
2. **Measurement** — DuskTap (donor runtime) / DuskLog taps (receiver) / the P2 differ
   and its artifacts (profiles, censuses, JSONL corpus), DZR fact sheets, gclib decodes.
   A computed MATCH/DRIFT/UNKNOWN beats any recollection.
3. **noclip.website** — reference-implementation tier for visual census, STB playback,
   place-names. CAUTION: layer SUPERSET only — never story-layer truth (the DZR fact
   sheets now carry true per-layer rosters; prefer them).
4. **Community video** — longplays/speedruns for behavior the instruments haven't
   reached yet. Timestamped citations, IVAN rule on sightings.
5. **User memory** — tie-breaker ONLY. The user's scaling role is instrument operator
   + arbiter, not encyclopedia. Feel-judgments (game-feel, "reads right") remain
   legitimately theirs — that is arbitration, not recall.

## Rules of use

- **Accept on the highest rung available.** If a rung-1/2 source exists, a claim is not
  accepted on rungs 3-5. If only rung 5 exists, the acceptance is provisional and gets
  a backfill task (build/point an instrument at it).
- **UNKNOWN ≠ MATCH.** An instrument that saw nothing says so (differ semantics,
  §206). A silent probe is not a passing probe.
- **Every acceptance leaves an artifact.** Accepted system ⇒ measured profile /
  census / golden trace filed, so future regressions are caught by diff, not by
  someone noticing. (№31 which-bank probe = the working miniature; windline profile =
  the template.)
- **Disagreements resolve upward, with receipts.** Cite the rung: "decomp
  d_grass.cpp:236" beats "the capture suggests" beats "noclip shows" beats "I remember".
- **Instruments get audited before they judge** (Housing negative-controls; builder ≠
  auditor).

## Case receipts (why this stack, not trust)

- **Grass green (§191-§192):** memory said color-feed bug; measurement (DuskTap K0 =
  ours) + decomp (d_grass.cpp ids) proved WRONG DONOR LINEAGE — TP particle in WW
  space. Three eyeball rounds lost; one capture closed it.
- **Windline density (§206-§207):** "moves like vanilla?" converted from an eyeball
  question to a numeric law (plateau ≈1.7/s) awaiting a computed verdict.
- **Heads re-vanish (§208):** memory pattern-matched to the fixed №68 bug; the session
  log (rung 2) showed a DIFFERENT path through the same gate (L-3 auto-arm → armIf) in
  minutes, with the §190 fix verified intact in source.
