id:            s36db0fef
symbols:       dStage_getName, dStage_getName2
tu:            src/d/d_stage.cpp
doorway:       EXISTS
destination:   PLUGIN
portable:      NONMATCHING
linked:        NOT
provenance:    see-file
negative-control: |
  Hook removed: a plugin-owned actor reaching getName gets the receiver's
  proc-only fallback and is reported under ANOTHER actor's name. Detected by
  the donor-vs-receiver behaviour delta, not by any crash - the receiver
  cannot return NULL, so nothing faults and the wrong name simply ships.
citations:
  - src/d/d_stage.cpp:1786-1812 receiver body (read 2026-08-16)
  - D:/XXXXXXX/WW DP/src/d/d_stage.cpp:1280 donor body, returns "誰？" (read 2026-08-16)
  - upstream_conformance --symbol dStage_getName SAFE both images (run 2026-08-16)
created:       HISTORY/BRIDGE 2026-08-16 A3-seed
