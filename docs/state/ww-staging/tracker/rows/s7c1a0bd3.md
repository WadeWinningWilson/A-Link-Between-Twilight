id:            s7c1a0bd3
symbols:       daLodbg_c::loadModelData
tu:            src/d/actor/d_a_lod_bg.cpp
doorway:       EXISTS
destination:   UPSTREAM
portable:      DECOMPILED
byte-true:     EQUIVALENT
linked:        NOT
verified:      UNKNOWN
citations:
  - "D:/XXXXXXX/WW DP/src/d/actor/d_a_lod_bg.cpp marker /* Nonmatching - regalloc */ sits on loadModelData, one such marker in the TU (read 2026-08-17)"
  - "D:/XXXXXXX/WW DP/src/d/actor/d_a_lod_bg.cpp range 0000046C-00000738 = 0x2CC/716 B, guarded by #if VERSION > VERSION_DEMO (read 2026-08-17)"
  - "D:/XXXXXXX/WW DP/src/d/actor/d_a_lod_bg.cpp asserts 'size >= 0' / 'resSize != -1' / 'success' all present; decl set bin,success,dst,resSize,oldHeap,size matches the reported allocation map (read 2026-08-17)"
  - docs/DECODER-BRIEFING.md 2b Equivalent = functionally correct, not byte-exact (read 2026-08-17)
  - docs/state/ww-staging/CALLS.md DECODER objdiff row, 99.94%, rows 153/167, r27-vs-r29 (read 2026-08-17)
review-verdict: |
  HISTORY/BRIDGE REVIEW 2026-08-17 — ACCEPTED as EQUIVALENT-class, with the
  labelling corrected on one axis. Every checkable claim in the Decoder's
  report verified independently against the donor source:
    · the /* Nonmatching - regalloc */ marker EXISTS and sits on THIS function
    · exactly one such marker in the TU (their "sole unmatched fn" holds)
    · 16 function definitions (their 15/16 is consistent)
    · all three assert strings present; the declaration set bin/success/dst/
      resSize/oldHeap/size matches their allocation map one-for-one
    · the quoted diff rows corroborate their own map: target `stw r29, 0(r31)`
      with r31=mDataSize& means the TARGET holds `size` in r29 — which is the
      earliest-dead-reuse they describe. The excerpt and the map agree.

  THE ONE CORRECTION — TWO DIFFERENT THINGS ARE BOTH CALLED "Equivalent":
    · UPSTREAM `Equivalent` is CONFERRED — the maintainers accepted the source
      with an explanatory comment. Upstream has this function as
      `Nonmatching - regalloc`. It is NOT upstream-Equivalent and we must
      never report it as such.
    · OUR `byte-true: EQUIVALENT` is ASSERTED by us: functionally equivalent,
      not byte-exact. The lane is entitled to this one and §2b authorises it.
  This row files the second and records the first verbatim, so the decomp's
  own progress can later confirm or deny us. That separation is the whole
  point of the labelling the user asked for.

  NOT VERIFIED BY ME, AND IT IS THE LOAD-BEARING CLAIM: that r27 and r29 are
  BOTH dead at `size`'s allocation. Everything rests on it — if r29 is live the
  swap changes behaviour and this is not equivalent at all. Their liveness
  measurement is self-consistent (r29 dead after row 142, swap at 153/167) and
  I have no reason to doubt it, but it is THEIR measurement, not mine.

  UNFLAGGED BY THEM, NOTED HERE: the function is guarded `#if VERSION >
  VERSION_DEMO`. I checked whether that forces `EquivalentFor(...)` instead of
  plain `Equivalent` and it does NOT — the guard removes the function from demo
  builds entirely, so there is no demo variant for a claim to fail on. Recording
  the check so nobody re-opens it. It also independently explains their
  "debug map inline check inconclusive".
created:       HISTORY/BRIDGE 2026-08-17 decoder-review
