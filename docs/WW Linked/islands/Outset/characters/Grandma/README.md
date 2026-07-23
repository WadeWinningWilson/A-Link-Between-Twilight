# Grandma (`Ba1`) — hero's-clothes scene scoping (№228)

**Questline position:** the next beat after the awake cutscene — Link visits Grandma, she gives
the hero's clothes. Carries three work fronts: her **interior positioning**, her **dialogue**,
and the **clothes handover**.

**Roadmap context (user, 2026-07-22):** the questline wires ahead of the final audio pass;
visuals before audio; Outset "done" is declared only after the full audio sweep (every cutscene,
NPC voice, ambient).

---

## 1. Interior positioning — CORRECTED (user, 2026-07-22)

- **Grandma is ALREADY present in her house interior in the mod** — the doors/interior work
  spawns Ba1 there today. The earlier draft of this section wrongly promoted the LinkRM
  census-defect note into "her placement is blocked"; the census defect explains why the ROOM
  CENSUS shows no NPCs, not whether she is placed (she is, via the manifest/population path).
- What actually remains for the questline: whether the CLOTHES SCENE requires a specific
  spot/pose/state for her (read from the scene's event data when identified), and any
  story-layer variation of her placement (№222 ladder, per-room) — both are event-data reads,
  not spawn work.

## 2. Dialogue

- Same pipeline as Aryll's awake lines (№217): donor message rows → `dialogue/` data, R6
  throughout. Her scene's message ids come from the event data / STB once identified in
  `LinkRM`'s EVNT table.
- Donor actor caveat: `d_a_npc_ba1.cpp` is largely **Nonmatching** in the decomp (demo/wait/exec
  procs are stubs) — so her flow must be read from EVENT DATA (event_list/STB + EVNT), not actor
  source. Budget accordingly.

## 3. The clothes handover — covenant analysis

- **DONOR FINDING: WW's hero's clothes are a TEXTURE SWAP, not a model.** WW Link's body model is
  one mesh; casual ⇄ hero is a `linktexS3TC` texture switch in the player (d_a_player_main.cpp
  ~12372-12382 comments; `clothes.bti` = item icon in d_item_data). There is no separate
  "clothes" model asset to hand over — the *item presentation* and the *wear state* are separate
  problems in the donor too.
- **Covenant (user):** the given clothes cannot be a TP substitute — the handover prop/item
  presentation must be WW's (№31 space purity; R6 for any text).
- **OPEN DESIGN DECISION (user input needed): what TP Link WEARS after the scene.** Readings:
  1. TP Link switches to his own native Hero's Clothes (his green tunic IS TP's "Hero's
     Clothes"; if he runs Outset in Ordon clothes until this beat, the scene grants the switch —
     TP asset on TP body, story beat from WW, no franken-model);
  2. an actual WW-style outfit on TP Link (asset work: new texture/model state).
- **Prior art either way:** the sumo outfit system (d_albw_outfit) — model/texture outfit STATE
  machine on TP Link with per-save worn bits, changeLink integration, D-pad quick-swap, cap
  recolor pipeline. The wear-state mechanics are already owned; only the asset/design question is
  open.

## 4. Cutscene player-drive observations (user, side-by-side testing)

Recorded verbatim intent: during cutscenes TP Link **rotated and froze animation at the exact
moments WW Link does** — then cycled ~"1/4 of an idle stand, paused, repeated" while standing for
the remainder.

Reading: the demo system's TRANS/ROTATE/pause channels **reach the TP player correctly** (his
storyboard participation works at the transform level). What doesn't land is the ANIMATION
channel: WW Link's demo anims (wake/stretch/walk) name WW archives that don't resolve on TP Link,
so his current TP idle gets driven by the storyboard's frame counter — a WW anim's frame range
looping over a fraction of the TP idle = the observed quarter-idle stutter-loop.

Work item (research, ties to the parked "WW Link anims on TP Link" idea): a demo-anim TRANSLATION
table — WW demo anim cue → nearest TP Link anim — applied where the player's demo data resolves
anims. Even a small table (wait/walk/surprised) would make him a convincing cutscene citizen.

## FOUND (№230): the scene's identity and mechanism

- **The clothes scene = `Ba1_Get_Itm`, an EVENT PACK in `LinkRM/Stage.arc::event_list.dat`**
  (alongside `TALE_DEMO`, `TALE_DEMO2`, `get_shield`, `LOOK_SHIELD`). NOT an STB — the donor demo
  archives hold no clothes storyboard (full inventory: tale/tale_2 [Demo01], kaizoku_zelda_fly +
  awake [Demo02], meet_tetra [03], stolensister [04 = the 0x0E20 kidnap beat], departure [05],
  maju_shinnyu [06], find_sister [07], meetshishioh/dragontale [08]). It's a talk-driven pack:
  dialogue -> her actor orders the event -> staffs play the beats. Our merge_event tooling speaks
  this format; the next step is parsing the pack's staffs/actions for her positioning, camera,
  msg ids, and the item beat.
- **The wear-state flag is `UNK_2A80`** (player_main: casual clothes unless 2A80 || NG+). In STB
  scenes the donor sets such flags from the STORYBOARD's prm channel via `d_a_demo00` — the
  cutscene Link BODY-DOUBLE — whose prm-op table maps sub-id 1 -> 2A80 (and whose SHAPE channel
  swaps the double's outfit mid-scene). For the event-pack path the setter is likely in Ba1's
  (Nonmatching) code or the pack's own action — the pack parse decides.
- `LOOK_SHIELD`/`get_shield` living in the same table means the SHIELD handover scene is
  co-located — the same read covers the next questline beat after the kidnap.

## Face estimate (№230): one WW opening expression on TP Link's rig

Chosen beat: **the wake "notice"** (the STB's own SE track names it: SLEEP → AWAKE → NOBI
(stretch) → NOTICE, №221) — WW Link's eyes closed through the first beats, then the wide-eye
notice when Aryll's voice lands.

- WW mechanism: pure texture panes — closed-eye texture swapped to wide-open; mouth texture to
  the small open "!" shape. No geometry moves.
- TP Link's rig (№229 evidence): eye/mouth texture-pattern channel (`mFaceBtpHeap`) PLUS a
  skeletal face BCK channel (`mFaceBckHeap`, per-expression loops, demo-resolvable by resource
  index at alink:8813).
- **Estimate:** the mapping is an UPGRADE, not a compromise. TP Link's own opening proves he has
  canonical sleep and surprise faces natively — so WW's closed-eyes pane maps to TP's sleep face
  (BTP eye-closed + relaxed face BCK), and the NOTICE beat maps to TP's surprise set (eye-wide
  BTP + brow/jaw BCK actually MOVING — something WW's rigid head cannot do). The translation
  table entry costs one row: WW demo face cue N -> TP face BCK/BTP pair. The face channels are
  demo-resolvable on TP's side already, so the wiring point exists; the work is the table and
  choosing TP's nearest expression per WW cue — an authoring task, not an engine one.

## THE SCENE, RECONSTRUCTED END-TO-END (№231)

**Dialogue arc = donor message rows 539-545 (consecutive block), give moment between 544 and 545.**
Row indices with a characterizing note (full text flows data->data at authoring time, R6):

| row | beat |
|---|---|
| 539 | her greeting as Link arrives (player-name insert) |
| 540 | the time-flies birthday lead-in |
| 541 | reassurance ("don't look so disappointed...") |
| 542 | the olden-days tradition — boys dressed as the hero at coming of age |
| 543 | the family shield on the wall — SEEDS THE LATER SHIELD BEAT in the same room |
| 544 | comprehension check — the last line BEFORE the give |
| — | **`Ba1_Get_Itm` pack fires: Ba1 `WAIT→GETITEM→WAIT`; Link `001n_wait→011get_item→001n_wait` (the hold-item-up pose); CAMERA staff alongside** |
| 334 | the item-get box ("received this treasured item...") |
| 329 | "HAPPY BIRTHDAY!" headline row |
| 545 | post-give: "they suit you perfectly" — proof the wear switch happens INSIDE the scene |

Neighboring rows for later states: 546 (go call Aryll — the NEXT questline hook), 548 (outfit
remark — likely lead-in variant), 552+ (post-kidnap states). Row order in the BMG matches play
order for the scene block; exact branch conditions live in Ba1's (Nonmatching) message tree and
will be settled by in-scene observation at wiring time, not guessed (IVAN).

**Implementation shape now fully known:**
1. Dialogue rows 539-544 through the demo-message pipeline (№217 pattern, data->data).
2. The pack: our merged event system already hosts event packs — `Ba1_Get_Itm` staffs are
   Ba1 + CAMERA + Link with six cuts total; the receiver equivalents are her mount's talk anims,
   the camera system, and TP Link's own get-item demo (he has a native item-get pose+jingle).
3. The give: item box (row 334 text, WW prop visual per covenant) + set our 2A80-equivalent
   mod flag; TP Link's wear switch per the user's pending design decision.
4. Row 545 as the post-give line; row 546 chains to the telescope beat (Aryll) — the questline
   continues in-sequence.

## №232 — wardrobe/store-own + vanilla-clothes integration research (COMPLETE)

**Verdict: the outfit system already owns every integration point the scene needs.**

| need | existing surface |
|---|---|
| Hero's Clothes as a wardrobe entity | `D_ALBW_OUTFIT_HEROS` — already in the outfit enum; ownership = native stash save bits (691-694) |
| The give = grant ownership | `dAlbwOutfit_recordOwnedByItemNo(itemNo)` — built for grant sites ("call alongside dMeter2_grantRentalClothes") |
| Store/own + D-pad cycle pickup | automatic once the stash bit is set — cycle walks owned kinds; Postman storage covers natives (`isStorableOutfit` = has storage bit) |
| The wear switch | `dAlbwOutfit_equip(D_ALBW_OUTFIT_HEROS)` — async-initiate, owned-only, mutual-exclusion, transition-safe |
| **Vanilla clothes changes** | `dAlbwOutfit_syncWornOwnership()` — per-frame "own what you wear": seeds the stash bit from `dComIfGs_getSelectEquipClothes()`, EXPLICITLY designed so vanilla-acquired clothes (Ordon at start, Hero's post-Faron, Zora/Magic via story) register "without per-grant-site hooks" |
| Crash class from the sumo era | already fenced: `canTouchLinkModel()` (blocks during transitions/pause/OPEN MESSAGE), `processPendingEquip()` drain, the menu-leave fix |

**Two-direction vanilla compatibility falls out for free:**
- Our scene grants first → vanilla's later Faron beat re-grants idempotently (bit already set; the
  native change itself is the same select-equip path the menu uses — no conflict).
- Vanilla grants/changes first (any order, any path — story, menu, workshop) → the own-what-you-
  wear sync records it; our scene then only needs its narrative beat (the box + flag), ownership
  already true.

**One timing wrinkle to verify at wiring:** `canTouchLinkModel()` returns false while a MESSAGE is
open — and the give happens inside a dialogue scene. So the scene's equip call QUEUES (by design)
and drains right after the item box closes — which is exactly the donor's own visible order (box →
wearing → row 545 "suit you perfectly"). Playtest confirms the drain lands before 545 opens.

**If the design decision goes to option 2 (WW-style outfit):** the enum pattern supports additions
(Deity precedent: owned-readable, out-of-cycle) — new kind + save bit + assets; everything above
still applies, only the asset work is added.

## №233 — where vanilla WW stores the clothes (answer: nowhere)

`dItemNo_FUKU_e` (0x32) is presentation-only: `VFUKU` arc (the held-up clothes-bundle prop),
`clothes.bti` icon, get-message 0x19F, `d_a_demo_item` display class; `NEW_FUKU_e` (0x37) =
assetless variant. NO status-screen, menu, or save-inventory presence anywhere in the donor —
the wear bit (2A80) IS the storage.

- 2nd wheel page: clothes stay OFF it (WW's wheel never carried them).
- WW start menu: additive design — surface the WARDROBE entry (stash bit) with `clothes.bti`.
- Handover prop = VFUKU bundle via the standard extract→adapt pipeline (covenant-clean).
- Msg id note: item table says 0x19F; BMG row 334 — different id spaces, resolve at wiring.

## №236 — pass opened: steps 1-2 DONE, step 6 resolved

- **Icon verify PASSED**: `clothes.bti` = C8 48x48 + RGB5A3 palette (Yaz0'd member); decoded
  preview = the green tunic, perfect. Conversion to the pipeline's direct RGB5A3 is mechanical.
  Icon archive survey: 108/114 C8 — the same converter covers the whole future menu-icon set.
- **`Vfuku.arc` adapted and installed** (both arc folders; vfuku.bdl TEV-normalized).
- **Get-item presentation = the bow's PROVEN PATH** (wind-waker-item-work.md): TP itemmdl.arc has
  no clothes model, so vfuku sources from OUR arc through the same CreateItemHeap-branch +
  patchModelForPc + locked TEV pipeline. LANE: itemmdl impl = Cursor; coordinate, don't solo.
- **Dialogue box**: current = ALBW-postman box (user called it); formatting gap cause verified
  (our wrap/pagination + pane binding vs TP's centered native formatter). Swap to the Shade
  Watcher's box = queued work item, cosmetic layer, must not break suspend/resume.

## №240 — step 6 SHIPPED (Cursor, 2026-07-22)

Engine presents the clothes bundle on host item `0x2F` via `getitem/clothes_bundle.ini` + overlay
arc (no WW name literals in exe). `grant_outfit:heros` fires present-demo / `DEFAULT_GETITEM`.
User verify: Grandma give → get-item pose with green bundle; wardrobe grant still works.
History: may trim `ww_ref=334` double-message / wire full Ba1 choreography next.

## №242b — vfuku crash FIX (Cursor, 2026-07-22)

Stale bow-era create (`getObjectRes` cast on BDLM/raw bmd3) → AV. Now uses mount
`acquireModelData` + boots/Ivan light/draw (no bow TEV scope). Re-verify give beat.

## №242c — BRK-init crash FIX (Cursor, 2026-07-22)

Crash #2: host `0x2F` BRK (`F_gD_rupy` idx 7) bound to vfuku mats → AV in
`searchUpdateMaterialID`. Clothes-bundle path now skips all TP anm init (donor FUKU
has none). Re-verify give → get-item pose.

## №253 — get-box kit SHIPPED (Cursor, 2026-07-22)

Whole kit rides `getitem/clothes_bundle.ini` (+ optional `clothes_bundle_text.ini`
overlay): `get_icon` (direct RGB5A3 BTI, oversized get-box alloc), `get_text` (msg
`host+0x65` body swap), `hand_offset_*` / `max_scale` for present-demo raised-hands
anchor. No WW name literals in exe. User verify: icon + text + hand seat.

## №255 — ladder climb gate SHIPPED (Cursor, 2026-07-22)

§63 r2: line check had wallCode=4 on the interior mount but `ChkWallHit==0`
blocked entry. Admit codes 4/5 without wall-hit (GLOBAL_e stays). Ladder-top
arrival cutscene still queued behind a working climb. Anchor nudge: `y=120 z=48`.

## №242 — FIRST PLAYTEST: crash + fidelity corrections (user, 2026-07-22)

**Vanilla scene corrections (user, direct game knowledge — these override №231's assumptions):**
- Grandma is on the **SECOND floor**; WW Link **climbs the ladder** to reach her.
- The scene triggers on **arriving at the ladder top** — a small cutscene — NOT talk-on-sight.
- During the cutscene she **carries the Hero's-clothes bundle in hand** (not before it) — the
  №218 attach system + a show/hide state gate (the telescope pattern, REQUIRED not optional).
- Then the get-item handover.

**Defects observed:** (1) CRASH at get-item start — Cursor's vfuku present-demo, dies at model
load/first-draw (patch-step suspect; forensics in bus doc); (2) ladder unclimbable — REGRESSION;
(3) Grandma on first floor; (4) dialogue text cropped (ALBW postman box — donor-wide fix =
native dMsgFlow_c + text injection, the Shade Watcher pattern); (5) Link freely moves during
dialogue (fixed by the same native-flow swap — speak events lock the player); (6) double-message
risk between our ww_ref=334 box and the native get-item message (trim ours when present-demo
lands).

## №243 — fidelity research COMPLETE (user items 5+6, data-proven)

**Her position (item 5) — DATA-PROVEN from `LinkRM/Room0.arc` ACTR:** four Ba1 placements,
params = her story-state variants:

| params | pos | reading |
|---|---|---|
| **0** | **(-225.0, 375.0, -55.0)** | **y=375 = SECOND FLOOR (the loft) — the birthday-scene state, as the user described** |
| 1 | (-308.1, 0, 379.1) | ground floor state |
| 3 | (575.1, 0, -27.3), ry=-10194 | ground floor state |
| 4 | (347.1, 0, 129.5), ry=-16384 | ground floor state |

Our mod currently spawns a ground-floor variant → the scene wants **params=0 upstairs**. Which
params spawns when is her actor's state logic (Nonmatching) — map by story flags at wiring, not
guessed. Same room also holds `SPitem` = the wall-shield field item (the get_shield beat's prop).

**The held bundle (item 6) — mechanism found in HER OWN ARC:**
- `ba_cloth.bdl` — the cloth-bundle model; textures decode **green-dominant** (dark folded-tunic
  green) — verified-by-convergence as the hero's-clothes bundle (green cloth + carry anim + the
  user's vanilla observation; flagged, not blindly asserted, per IVAN).
- `hold.bck` — her CARRY animation (10 KB — a substantial pose set).
- `05_ba_cut8_wait.bck` — a cutscene wait ("cut8" = the scene's demo cut).
- **Our manifest ALREADY loads `ba_cloth.bdl` as her companion model** — the remaining work is the
  STATE GATE: hidden by default, shown + `hold.bck` during the scene (the telescope's show/hide
  gate pattern — REQUIRED per [[feedback_port_full_state_machines]], now with its second customer).

**The scene trigger (user): the LADDER-TOP arrival starts a small cutscene** — not talk-on-sight.
Trigger = arrival volume at the loft edge; plays: her hold-state + dialogue arc + handover.

## №244 — two refinements (user, while Cursor wires the crash fix)

**(5 refined) DATA-DRIVEN PLACEMENT AS THE STANDING SYSTEM, not a one-off tweak.** The end goal:
follow WW's OWN mechanism — the room data carries ALL of an NPC's placements (Ba1 × 4, keyed by
params = story state) and the GAME selects which spawns as progress advances. Our spawner should do
the same: carry every param variant from the census and select by story state (our mod-flag
equivalents of the №222 ladder), so future story changes move her — and EVERY NPC — automatically.
No hand-moved positions, ever; manual placement is the anti-pattern. This generalizes: the same
params-variant selection applies island-wide as the questline advances NPCs through their states.

**(6 confirmed + a standing mystery SOLVED): the "green model in the middle of her head" —
present since her model landed — IS the mis-mounted clothes bundle.** `npc_ba.ini` mounts
`companion_model=ba_cloth.bdl` with NO `companion_mode` → default mode 0 = synced base TRMtx +
own BCK (none loaded) → the bundle renders unbound at her base matrix: the green blob. It was
never a glitch texture; it is the scene's key prop with no state gate. The scene-wiring fix
(hidden by default; shown + bound + `hold.bck` during the give) therefore ALSO fixes the visual
bug — one mechanism, two defects closed.

## №245 — placement system: the VISIBILITY axis (user refinement)

The data-driven placement contract has THREE axes, all from WW's own systems:

| axis | mechanism | evidence |
|---|---|---|
| **1. EXISTENCE (layers)** | room ACTR comes in LAYER chunks (ACT0..ACTb); the №222 story-flag ladder selects the ACTIVE layer; actors on inactive layers DO NOT EXIST | `d_com_inf_game.cpp:192-200` (the ladder) + the per-layer ACTR chunks |
| **2. POSITION/STATE (params)** | within existence, params selects the placement/state variant | Ba1 ×4 in LinkRM Room0 (№243) |
| **3. PRESENTATION (actor-state)** | an actor can EXIST as a process but be neither drawn nor collidable until its scene readies | Ls1 `telescope_proc` finds Bm1 via `searchByID` and sets `m881=1` — he IS live at the telescope beat; his hidden-until-look state is actor-internal (Nonmatching — exact draw/collision gate settled by observation, not guessed; both of the user's hypotheses remain possible for the FINAL presentation flip) |

**Symptoms this explains:** actors floating in mid-sea = layer-blind spawning (rows from inactive
layers, or merged-layer census); the postman standing visible at the mailbox pre-telescope =
missing axis 3 (he's placed right but should not be PRESENTED yet).

**Our system mirrors all three:** census rows carry layer + params; the spawner filters by the
ACTIVE layer derived from mod story flags (№222 mapping); presentation gating = per-actor
visibility state tied to scene flags (the manifest's spawn_if/unless_flag family extends to a
present_if family — hidden ⇒ no draw AND no collision AND no attention, exactly WW's contract).
No hand-tuning at any future story change — the data moves everyone.

## Order of attack (updated)

1. ~~Identify the clothes event~~ DONE (`Ba1_Get_Itm`). Next: parse the pack's staffs/actions
   (merge_event tooling) — dialogue ids, positioning, camera, the item beat, the 2A80 setter.
2. Her scene positioning/state per the pack's staff data (placement itself already exists).
3. Dialogue authoring from donor rows (pipeline exists).
4. Handover presentation (WW item prop) + the wear-state design decision → user.
5. Player anim translation table (research first, small table after).
