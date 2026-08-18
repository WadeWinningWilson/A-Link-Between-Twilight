id:            sae9b12ef
symbols:       UNDERIVED
tu:            src/d/ww_jpa_bind.cpp
doorway:       ABSENT-hookable
destination:   PLUGIN
portable:      UNKNOWN
linked:        NOT
provenance:    see-file
citations:
  - src/d/ww_jpa_bind.cpp KIT-LINEAGE host-plumbing (read 2026-08-16)
  - D:/XXXXXXX/WW DP/configure.py donor status NOT LISTED (read 2026-08-16)
  - src/f_pc/f_pc_profile_lst.cpp, src/d/ext_plugin/ww_profile_register.cpp profile absent (read 2026-08-16)
notes: |
  ADJUDICATED PLUGIN from KIT-LINEAGE `host-plumbing`: receiver-side wiring that
  hosts donor content. Kept distinct from `native-port` - same destination,
  different author.

  DERIVED by drain_derive.py - MECHANICAL HALF ONLY.
  `destination` is BLANK ON PURPOSE: axis A is an ownership judgement
  and a generated default would file an unexamined verdict that looks
  adjudicated. row_store.py validate REFUSES this row until a lane
  rules it. Do not bulk-fill.
created:       HISTORY/BRIDGE 2026-08-16 drain-derive
