id:            s43cacbf5
symbols:       msg_class, fopMsgM_messageSet, fopMsgM_pane_class, msg_process_profile_definition
tu:            src/f_op/f_op_msg_mng.cpp
doorway:       ABSENT-hookable
destination:   SPLIT
portable:      NONMATCHING
linked:        NOT
provenance:    see-file
negative-control: |
  The manager is the LIFECYCLE, so its absence does not mis-draw a box - with
  W1-W3 present and this missing, every part needed to draw exists and nothing
  ever creates, advances or destroys one. Observable as a message that never
  appears, the opposite signature to a rendering fault.
citations:
  - D:/XXXXXXX/WW DP/configure.py Object(NonMatching, f_op/f_op_msg_mng.cpp) (read 2026-08-16)
  - src/f_op/f_op_msg_mng.cpp KIT-LINEAGE mixed (read 2026-08-16)
  - src/f_op/f_op_msg_mng.cpp receiver 460 lines vs donor 10205 (read 2026-08-16)
  - docs/state/ww-message-donor-spec.md 7 W4 msg_class is a leafdraw_class, 24-state machine (read 2026-08-16)
notes: |
  SPLIT FROM safe3631d (the W4 wire row) AND THE SPLIT IS THE POINT, NOT BOOKKEEPING.

  WHY IT WAS FOUND: the drain readout said LEGACY-UNROWED 1 while the sheet's own
  table looked empty, and drain_derive.py said this TU "already carries a row".
  Both were right. safe3631d listed TWO TUs in one comma-separated `tu:` field;
  drain_derive splits on the comma, ww_ratchet does not. A row spanning two TUs
  is therefore rowed under one tool and unrowed under the other.

  FIXED ON THIS SIDE RATHER THAN IN THE RATCHET - that is Foundry's instrument and
  a lane does not edit another lane's tool to make its own count come out. The
  schema wants one row per seam and this is schema-clean.

  AND IT WAS ALWAYS THE BETTER SHAPE: safe3631d carried `f_op_msg.cpp` (Matching)
  and `f_op_msg_mng.cpp` (NonMatching) under ONE `portable:` value, so the row had
  to state the pessimistic floor and enumerate the exception in prose. Now each TU
  carries its own verdict and the floor costs nothing.

  DESTINATION SPLIT, not PLUGIN: KIT-LINEAGE is `mixed`, and the receiver's 460
  lines against the donor's 10,205 are not the same system - TP moved its logic
  down into d_msg_class / d_msg_flow / d_msg_object and eleven d_msg_scrn_*
  screens. Per-hunk ownership is unruled here.
created:       HISTORY/BRIDGE 2026-08-16 drain-final
