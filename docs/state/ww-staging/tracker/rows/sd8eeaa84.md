id:            sd8eeaa84
symbols:       daMP_c
tu:            src/d/d_demo.cpp
doorway:       ABSENT-hookable
destination:   SPLIT
portable:      NONMATCHING
linked:        NOT
provenance:    see-file
citations:
  - src/d/d_demo.cpp KIT-LINEAGE mixed (read 2026-08-16)
  - D:/XXXXXXX/WW DP/configure.py donor status NonMatching (read 2026-08-16)
  - src/f_pc/f_pc_profile_lst.cpp, src/d/ext_plugin/ww_profile_register.cpp profile absent (read 2026-08-16)
notes: |
  ADJUDICATED SPLIT, NOT PLUGIN. KIT-LINEAGE is `mixed`: donor and receiver hunks
  coexist in one TU, which is what SPLIT means on axis A. The per-hunk boundary is
  NOT ruled here - a mixed TU needs its hunks adjudicated individually, and rowing
  it PLUGIN would assert a wholesale ownership this file does not have.

  DERIVED by drain_derive.py - MECHANICAL HALF ONLY.
  `destination` is BLANK ON PURPOSE: axis A is an ownership judgement
  and a generated default would file an unexamined verdict that looks
  adjudicated. row_store.py validate REFUSES this row until a lane
  rules it. Do not bulk-fill.
created:       HISTORY/BRIDGE 2026-08-16 drain-derive
