# Staging manifest — see docs/WW Linked/staging-protocol.md
# state: DRAFT -> READY -> (FLAGGED -> READY)* -> MERGED

| id | lane | title | state | depends-on | claims |
|---|---|---|---|---|---|
| foundry-1 | Foundry | 19a v2 per-TU table (L8 blocker) | MERGED | - | taxonomy=bus 25571; roster=ww_layer_exclude.cmake; evidence = CONDENSED decision-grade extract ONLY (foundry-1-evidence.md); full 50-row table LOST (0-byte transcript) - re-run required if per-TU detail needed; synthesis used the extract; roster corrected 62+11; synthesis DONE: P1-P6 + 5 waves ZERO-INDEXED (WAVE 0..WAVE 4) in draft |
| foundry-2 | Foundry | actor_kit + enemy_port_kit retools (audit-specced) | DRAFT | - | OUTPUT LAW landed+selftested; audit :311/:52/:529 line-verify pending |
