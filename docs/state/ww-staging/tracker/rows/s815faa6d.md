id:            s815faa6d
symbols:       dExtWwSave_registerWwStage
tu:            src/d/d_ext_npc_mount.cpp
doorway:       ABSENT-unhookable
destination:   FORK
portable:      ABSENT
linked:        NOT
provenance:    see-file
negative-control: |
  Not required at this destination (the schema demands one only for PATCH),
  supplied anyway because the claim is falsifiable and should be: if this
  symbol is ever found in an unmodified vanilla build, this row is wrong and
  the split below is wrong with it. Measured 2026-08-16: MISSING on vanilla.
citations:
  - upstream_conformance --symbol dExtWwSave_registerWwStage MISSING on vanilla (run 2026-08-16)
  - src/d/d_ext_npc_mount.cpp:11927 registration entry point (read 2026-08-16)
  - split from sfb76e669 on Integrator's "ABSENT-unhookable vs our-code-in-the-wrong-place are different verdicts" call (read 2026-08-18)
  - configure.py-equivalent check: destination FORK was already declared in _schema.json and had ZERO users before this row (read 2026-08-18)
created:       HISTORY/BRIDGE 2026-08-18 split-of-sfb76e669
notes: |
  WHY THIS IS A SEPARATE ROW FROM sfb76e669, which used to carry both symbols
  under one verdict.

  This symbol is OURS. It is absent from vanilla BECAUSE WE WROTE IT. The
  parent row's negative control argued "absent from vanilla entirely, so there
  is nothing to hook" — and for a symbol we authored that reasoning is
  CIRCULAR. It cannot be evidence of an unhookable receiver doorway; it is
  only evidence that we have not shipped it.

  THE DOORWAY VALUE IS THE HONEST-BUT-AWKWARD PART, and it is stated rather
  than hidden: `ABSENT-unhookable` is literally true of vanilla and is a
  MALFORMED QUESTION for our own symbol. The schema requires a doorway on
  every row and offers no "not applicable", so the value is kept and this note
  carries the caveat. THE ACTIONABLE LIVES IN `destination`, NOT `doorway`:
  FORK means this is ours to move, not vanilla's to patch. That is exactly the
  distinction Integrator asked for, and it needed no new vocabulary — FORK was
  declared in _schema.json all along and simply had no users.

  DO NOT read this row as evidence about the receiver's surface. It is
  evidence about where our own code currently lives.

  ⚠ THIS ROW'S DOORWAY `[ OK ]` IS VACUOUS AND MUST NOT BE READ AS EVIDENCE.
  Measured at tools/foundry/row_doorway.py:105-108 (read 2026-08-18, Housing/
  Engine's finding, confirmed by running it): the only check that applies to an
  ABSENT-* row is

      elif doorway in DOORWAY_DOMAIN and n_up > 0:
          "declared %s but PRESENT on vanilla - class flipped under the row"

  It requires the symbol to turn up in the VANILLA image. This symbol is OURS,
  so n_up is 0 forever and that branch CANNOT FIRE. `row_doorway.py` prints
  `[ OK ] s815faa6d (ABSENT-unhookable, 1 symbol(s))` and always will,
  whatever the truth is. A gate that cannot go red is not a gate.

  WHAT IS STILL LIVE ON THIS ROW, so the OK is not entirely empty: the
  `n_own == 0` check ("gone from OUR FORK - the row cites code the tree no
  longer has") DOES apply and is meaningful. So the green means "still present
  in our fork" and carries NO doorway evidence whatsoever.

  THE REAL REMEDY IS A `doorway: N/A` VALUE for FORK rows, which is a schema
  change and not this row's to make. Until then this note is the only thing
  standing between a future reader and a green tick that means nothing.
