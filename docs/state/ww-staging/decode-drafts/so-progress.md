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

## CORRECTION (same day): vintage wiring REVERTED
- The kickoff's "literal-vintage (round-17 recon)" is WRONG for so:
  target has 0 .double literals and 0 _half/_three statics; the 14
  frsqrte chains are f32-constant normalize-family inlines, not sqrtf.
  Round-17's recon mislabeled so. Wire the vintage PCH ONLY if/when a
  genuine sqrtf site appears during decode (then re-check this).
- Lesson re-banked (3rd time this session): verify the signal yourself
  before acting on a recon row — and never let a commit message claim
  "verified" for a grep that returned nothing.

## Ctor member map (from __ct__10daNpc_So_cFv target asm, 2026-08-18)
- Class size 0xBE4 (g_profile word 5). Base fopNpc_npc_c to 0x6C4
  (standard subobjects verified: jnt 0x29b/c, EventCut 0x2cc/2d0/32c,
  ObjAcch@0x334, AcchCir@0x4F8, Stts@0x538(vt 0x550), Cyl@0x574,
  0x6ac=-1, 0x6b0=0).
- Derived: 0x6F4 dCcD_Stts #2 (GStts@0x6F8; ALSO stores r31 — a saved
  reg value into 0x6F8+? read the head for what r31 held) ·
  0x718 dCcD_Sph block (SPHERE collider — GObjInf@0x718, Aab@+0x110,
  ShapeAttr@+0x114, Sph@+0x128, dCcD_Sph vt@+0x3C; fish body) ·
  0x854 mDoExt_btpAnm (0x858/0x860 zeroed) · 0x870 dBgS_ObjAcch #2
  (vt@0x880, +0x14=r30, +0x20=r29 — saved values, read head) ·
  0xA34 dBgS_AcchCir #2 · 0xAE8 dPa_rippleEcallBack (JPACallBack →
  levelE → rippleE vt chain; 0xAEC=0 — water ripple particles) ·
  tail to 0xBE4 unknown (read member fns).
- NOTE: the ctor head saves r29/r30/r31 BEFORE the tail — they hold
  computed defaults stored into Stts#2/Acch#2 fields; decode the head
  block when writing the ctor (implicit-vs-explicit TBD: .fn weak?
  CHECK — if global+long, so has an EXPLICIT ctor unlike ob1).
- Ctor is WEAK = implicit/in-class (ob1 pattern) — the r29-r31 defaults come from member-object inline ctors, not user code. Kickoff package COMPLETE; begin decode rounds at checklist step 2 (header rewrite).
- Header restructured + virtual overrides fixed (u16/u32); BASELINE 13/187 exact, fuzzy 1.9049%. so is VERSION-CONDITIONAL (#if VERSION_DEMO in class tail) — MatchingFor rule applies at label time. Kickoff steps 1-2 DONE; decode rounds begin smallest-first.
