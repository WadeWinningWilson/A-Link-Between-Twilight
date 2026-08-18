id:            s9d4b60ce
symbols:       dStage_Event_dt_c, dStage_MapEvent_dt_c, dEvent_manager_c::getName
tu:            src/d/ext_plugin/ww_stage_loader.cpp
doorway:       EXISTS
destination:   PLUGIN
portable:      DECOMPILED
byte-true:     DIVERGENT
linked:        NOT
citations:
  - "D:/XXXXXXX/WW DP/include/d/d_stage.h:329 dStage_Event_dt_c, Size 0x18, mName[15] annotated @0x04 (read 2026-08-17)"
  - "D:/XXXXXXX/Ex WW/_extracted/Stage_decompressed/sea/Stage.arc DZS base 0x0B6E20, EVNT chunk 57 records @+0x388 (read 2026-08-17)"
  - include/d/d_stage.h:426 receiver union event_name[13] @0x0D; field_0x1a/switch_no @0x1A/0x1B OUTSIDE the union (read 2026-08-17)
  - include/d/d_stage.h:443 enum dStage_MapEvent_dt_type MAPTOOLCAMERA/ZEV/STB (read 2026-08-17)
  - src/d/d_event_manager.cpp:157 getName switches on type; case 1/2 return data.event_name (read 2026-08-17)
  - src/d/d_msg_object.cpp:58 dMsg_resolveGroupArchive — the WW-gated hook precedent (read 2026-08-17)
negative-control: |
  Translate the name from offset 0x04 (the value the donor header annotates) and
  every event name loses its first three characters — STOLENSISTER reads
  LENSISTER, MapToolCamera reads ToolCamera. Measured on all 57 sea records:
  57 of 57 truncate. It would not fault; it would silently match no event.
review-verdict: |
  HISTORY/BRIDGE DONOR HALF 2026-08-17, answering Housing's routed question.
  Read from REAL BYTES (sea Stage.arc, 57 EVNT records), not from the header.

  ** THE HEADER IS WRONG ABOUT THE NAME OFFSET. ** The donor annotates
  mName[15] at 0x04, but that contradicts its own following fields AND its own
  stated size — and the bytes settle it: THE NAME IS AT 0x01. Reading at 0x04
  truncates 57 of 57 records. Housing's design row repeats "name@0x04" from the
  same header; implementing it would ship a silent 3-character chop.

  FIELD MAP, measured across all 57 records:
    0x00        DEAD — constant 0xFF in all 57
    0x01..0x0F  mName, 15 bytes
    0x10, 0x11  MAP-TOOL ARM — set in EXACTLY the 16 MapToolCamera records and
                0xFF in all 41 others. Perfect correlation, both bytes.
    0x12        always meaningful (never 0xFF): 0x00 x46, 0x43 x4, 0x80 x1, 0x83 x6
    0x13        mSpawnSwitchNo — set in 23 records, values 1..116, ZERO overlap
                with the map-tool records
    0x14        ROOM NUMBER — 49 records set, every value within 0..48 (the 7x7
                sea grid). Semantic proof: STOLENSISTER, departure_DEMO,
                AJ_SPEAK, TELOP_PULOLO, awake and PUROLO_RETURN — six
                unmistakably-Outset events — ALL carry 0x2C = 44, which is the
                room id independently established for Outset.
    0x15..0x17  DEAD — constant 0xFF in all 57
  So the translator's real input is the name plus FIVE live bytes.

  THE TYPE DISCRIMINATOR IS SETTLED, AND BOTH SIDES AGREE: the donor encodes
  map-tool events as the literal name "MapToolCamera" (16 of 57 records, the
  only duplicated name); the receiver's own enum calls type 0
  dStage_MapEvent_dt_TYPE_MAPTOOLCAMERA. Same concept, one as a string, one as
  an enum. WW named events take the STB arm (type 2) — WW binds cutscenes by
  .stb, per the Aryll work.

  ** THE BLOCKER HOUSING'S FRAMING WOULD HAVE WALKED INTO: the name field
  NARROWS 15 -> 13. ** 12 of 42 distinct sea names are at or past capacity; SIX
  are genuinely 14 characters and cannot fit even unterminated:
  R29LOOK_KINDAN, R29LOOK_MANTEN, R29LOOK_TAKARA, VOLCANO_FAILED,
  getperl_komori, and departure_DEMO — that last one is an Outset event at room
  44, i.e. on the port's critical path.

  I TESTED THE OBVIOUS ESCAPE AND IT FAILS: 0x1A/0x1B sit OUTSIDE the union, so
  event_name cannot quietly run to 15 bytes — a 14-char name clobbers
  field_0x1a and a 15-char one clobbers switch_no. The container is compatible;
  the NAME FIELD IS NOT. Housing's "field mapping, not an open design question"
  is right about every field except this one.

  ROUTE (DN-10 order of resort, and it has an in-tree precedent): do NOT widen
  the receiver struct — 0x1C is TP's on-disc layout and TP data uses it. Do NOT
  shorten donor names, which is baking. Serve the full donor name from a
  plugin-side table keyed by record index behind a WW-gated hook on
  dEvent_manager_c::getName — exactly the shape of dMsg_resolveGroupArchive
  (d_msg_object.cpp:58, registered in ww_room_loader.cpp:765). getName is
  already index-driven, so the hook site is a one-line gate.
created:       HISTORY/BRIDGE 2026-08-17 evnt-donor-half
