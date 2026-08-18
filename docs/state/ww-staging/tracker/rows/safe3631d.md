id:            safe3631d
symbols:       msg_class, fopMsg_Create, fopMsg_Delete, fopMsg_Execute, fopMsg_Draw, g_fopMsg_Method, msg_process_profile_definition
tu:            src/f_op/f_op_msg.cpp
doorway:       ABSENT-hookable
destination:   PLUGIN
portable:      NONMATCHING
linked:        NOT
provenance:    see-file
negative-control: |
  W4 is the LIFECYCLE, so its absence is not a rendering fault: with W1-W3 present
  and W4 missing, every part needed to draw a box exists and nothing ever creates,
  advances or destroys one. Observable as a message that never appears rather than
  one that appears wrong - the opposite signature to a W1 failure.
citations:
  - D:/XXXXXXX/WW DP/configure.py Object(Matching, f_op/f_op_msg.cpp) (read 2026-08-16)
  - D:/XXXXXXX/WW DP/configure.py Object(NonMatching, f_op/f_op_msg_mng.cpp) (read 2026-08-16)
  - docs/state/ww-message-donor-spec.md 7.5 f_op_msg.cpp MATCHED 88 lines, donor 8002A688-8002A860 (read 2026-08-16)
  - docs/state/ww-message-donor-spec.md 7 W4 msg_class is a leafdraw_class, 24-state machine (read 2026-08-16)
notes: |
  SPLIT WIRE, PESSIMISTIC FLOOR - same treatment as W1 (sd77568d1):
    CITABLE VERBATIM : f_op_msg.cpp  (the process shell: Create/Delete/Execute/IsDelete/Draw)
    RECONSTRUCTION   : f_op_msg_mng.cpp (7,637 lines, the 24-state machine)

  THE RECEIVER ALSO HAS f_op_msg.cpp AND f_op_msg_mng.cpp - AND THAT IS A SEAM TO
  LABEL, NOT A REASON TO SKIP THE PORT (parallel-port rule; the audio precedent).
  The two managers are not the same system in any case: receiver f_op_msg_mng.cpp
  is 460 lines against the donor's 10,205, because TP moved its logic down into
  d_msg_class / d_msg_flow / d_msg_object and eleven d_msg_scrn_* screens. Porting
  WW's manager INTO the receiver's would replace a larger system with a smaller
  one; running them in parallel is the only additive shape.

  THE ONE MOVING NUMBER IN THIS CORPUS LIVES HERE. f_op_msg_mng.cpp fell from 13
  non-matching functions to 2 between 1d57f046 and 2d094c26. It is still
  NonMatching so the class does not change and the floor stays NONMATCHING - but
  the reconstruction surface in the largest TU of the port collapsed by an order
  of magnitude. HOUSING: re-read before reconstructing; most of it is now donor.
created:       HISTORY/BRIDGE 2026-08-16 message-build W4
