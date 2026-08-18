# d_a_npc_so decode campaign (queue-next after ob1) — KICKOFF

**Read p2-progress.md CONTINUATION PREAMBLE + ob1-progress.md rounds 3-15
FIRST** — they carry the charter, monitor discipline, toolchain recipes,
the complete MWCC lesson bank (~35 lessons), the transform-search harness
(WWDP tools/decoder_transform_search.py), and the inline-lever campaign
spec that both p2 and ob1's endgames await.

## Identity + shape (kickoff recon, 2026-08-18)
- daNpc_So_c = THE FISHMAN (Esa/bait event, offsetDive/Swim/Appear,
  modeEventMapopen = the sea-chart opener, Outset presence). 187 fns,
  77 local/weak, text 0x4AF4, literal-vintage (round-17 recon).
- Base fopNpc_npc_c — ctor pattern IDENTICAL to ob1/km1 (vt@0x6c0,
  jnt 0x29b/0x29c, EventCut 0x2cc/0x2d0/0x32c, ObjAcch/AcchCir/Stts/Cyl
  standard offsets). The ob1 header is the closest template; expect a
  second EventCut/PathRun-region layout — read the ctor tail for
  derived-member inits.
- Naming style DIFFERS from ob1: modeXxxInit/modeXxx pairs (~20 modes:
  Wait Hide Jump Swim NearSwim EventFirst{Wait,,End} EventEsa
  EventMapopen EventBow Talk Disappear Debug GetRupee EventTriForce...)
  + _CB-suffixed static callbacks (searchEsa_CB, nodeControl_CB,
  XyCheckCB/XyEventCB = fopMsg XY callbacks!, createHeap_CB).
- 3 text sections → post-__sinit region B EXISTS: expect demanded-late
  constructs (cut-family .inc treatment per p2 round 7 OR HIO-in-cpp
  placement per p2 round 14 — read the region-B .fn list first).
- ⚠ configure.py:1577 `ActorRel(NonMatching, "d_a_npc_so")` LACKS the
  vintage wiring — FIRST STEP: add extra_cflags=["-DSQRTF_CONST_LITERALS"]
  + switch the cpp to dolzel_rel_lit.h (round-17 recon says so IS
  literal-vintage; VERIFY via rodata 0.5/3.0 doubles before trusting).

## Kickoff checklist (ob1 playbook, proven this session)
1. Vintage wiring (above) + build the skeleton .o; objdiff baseline.
2. Ctor asm → derived member map; header rewrite on the ob1 pattern
   (fopNpc base + typed tail); tc canary after ANY shared-header edit.
3. Smallest-fns-first with per-fn objdiff; sibling-verbatim where a
   matched sibling has the same machine shape (km1/bm1 lessons: copy
   the SOURCE, not the idea).
4. Read caller test shapes for bool/BOOL/int returns (clrlwi. = bool,
   cmpwi = int) BEFORE guessing signatures — saved ~10 iterations on ob1.
5. Assert strings + res-enum headers (res/Object/So.h in assets/) give
   original names — use them from the start.
6. Pool/data order LAST (l_evn-style file statics position, GXColor
   debug trio at _draw tail, de-const .data tables) then REL gate under
   TEMP Matching flip (NEVER cite a NonMatching-config hash).
