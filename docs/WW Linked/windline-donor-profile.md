# Windline (0x31) — donor spawn-rate profile

> Foundry P2 artifact (bus §206), computed from the §196 capture corpus
> (`dolphin-capture-20260728-sessions1-3.log`, Outset play, 906 s). Regenerate:
> `probe_differ.py profile sessions1-3.jsonl --site emitter_create --key id/00000031 --bucket 30`

**The donor law for W-LINE density:** during active Outset exterior play the donor
creates windline emitters at a **sustained ~1.7/s (≈50 per 30 s), peak 2.1/s**, and
near-zero indoors. 500 creations total over the session.

```
bucket  n   (30 s buckets from t0=12:41:394)
    0s  27  #################
  150s  22  #############
  390s  31  ###################
  420s  51  ################################
  450s  52  #################################
  480s  51  ################################
  510s  49  ###############################
  540s  51  ################################
  570s  50  ###############################
  600s  63  ########################################
  840s  13  ########
  870s  34  #####################
  900s   6  ###
mean 0.55/s (whole session incl. interiors) · plateau ≈1.7/s · peak 2.10/s
```

**To close the verdict (F2):** Engine lands the one DuskLog line at the receiver's
emitter-create/`dPa_wwWindlineResRM` site (state-file spec), user plays ~5 min of
Outset exterior, Foundry runs the census/profile diff → MATCH/DRIFT on the user's
open "moves in all the ways vanilla does?" question, density axis. Gaps in the
histogram = interiors/menus; compare plateau-to-plateau.
