# WW Restoration Cookbook — the ground-up process

**What this is:** the distilled, proven pipeline for bringing a Wind Waker space to life
inside the TP port — map, interiors, doors, actors, limbs, animation, story gating.
Every step here was validated on Outset Island (W1). The chronological decision ledger
lives in [docs/state/cut-actors-demo-restore.md](state/cut-actors-demo-restore.md) (№-entries
referenced throughout); this doc is the stable HOW. The run plan lives in
[Cut-Actors-Demo-Restore-Cursor-History.md](Interconnected%20Chats/Cut-Actors-Demo-Restore-Cursor-History.md).

**Canonical references:** the WW decomp https://github.com/zeldaret/tww (AUTHORITATIVE for
actor semantics, params, arc bindings) + https://gamebanana.com/tuts/18805 (arc/actor field
reference). Consult BEFORE guessing any WW name or param.

---

## 0. Doctrine (non-negotiable)

- **SPACE-PURITY LAW (№31, user-decreed FINAL — BROADENED 2026-07-20):** WW spaces get ONLY WW
  assets; TP spaces ONLY TP assets. No cross-pollination ever — **including rupees** (WW `Vlupy`
  visual; the grant may credit the TP wallet because the *economy* is a system). A missing prop (a
  HOLE) is always preferable to a foreign one.

  **№31-B — PURITY COVERS SHAPING, NOT JUST SUPPLY.** №31 was written about *assets* and was read
  that way for months. **It applies equally to any receiver system that MODIFIES donor content on
  its way to the player** — not only to systems that supply it. Assets, **music, lighting, sound
  mixing, animation blending, physics response — everything.**

  **The test:** if donor content passes through a receiver stage that changes what is seen or heard,
  that stage is a purity surface. **Decide explicitly which engine owns it, and neutralise the other
  side to identity.**

  **Why this exists (both caught the same week, same shape, neither predicted):**
  - **Lighting:** WW grass takes colour from `dKy_tevstr_c` per room. Fed by TP's environment system,
    donor grass renders wrong — correct asset, foreign shaping.
  - **Audio:** WW sequences played through JAudio2's channel, whose volume chain applies **TP's
    velocity curve** (`JASChannel.cpp:194`, velocity applied twice ⇒ squared). Correct donor bytes,
    byte-verified against the player's extract, **still wrong at the speaker** — because purity was
    verified at the byte level and never at the audible one.

  **The trap:** each looks compliant under the old reading. Donor content ✓, receiver system ✓ —
  and §0's *"systems don't port, content ports"* actively points toward using the receiver's system.
  **The two rules collide precisely at shaping stages, and №31-B resolves the collision in favour of
  purity.**

  **Corollary:** verifying donor bytes are unmodified proves nothing about output. **Verify at the
  stage the player perceives**, not at the stage that is easiest to hash.
- **RECEIVER COVENANT:** the vanilla exe/game tree contains ZERO WW content — no bytes, no
  names, no dialogue. Everything lives in the mod folder
  (`%AppData%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\`).
  `greplist.txt` gates builds. WW arcs are NEVER committed.
- **BINARIES OUTLIVE SOURCE FIXES (audit F-3, 2026-07-19):** cleaning the source does not
  clean the exes already sitting on disk. **Any preserved/backup binary must pass the
  greplist gate before it is shared with anyone** — `build/` being gitignored stops commit
  risk, not distribution risk. Gate every exe, not just the newest; mark failures in place
  (do not rename/delete — build scripts hard-code backup paths).
- **GATE HITS ARE TRIAGED, NOT COUNTED:** TP's own vocabulary is exempt (Bokoblin/Moblin
  species, Makar/Medli *stub-REL roster labels from TP's own decomp archaeology*, TP
  particle/proc names). A real leak is WW-specific content: WW character labels for our
  actors, WW arc/member names, dialogue keys, manifest templates with WW values.
- **SYSTEMS DON'T PORT, CONTENT PORTS:** TP owns player, inventory, save, camera, combat,
  menus, doors-as-mechanism, wallet. WW supplies models, rooms, placements, dialogue,
  identity. New data classes enter via one-exemplar vetting.
- **DIVISION OF LABOR (№39):** if it's a VALUE, History wrote it; if it's a BEHAVIOR,
  Cursor built it; if it's a NAME ON A FACE, the user said it.
- **IVAN RULE:** no invented identity labels. Verify via decomp + user identity pass;
  labels stay `? (unverified)` until the user locks them. (tww comment order is NOT
  reliable proc order — the Joel/Zill inversion, №50-B.)
- **FAIL-CLOSED EVERYWHERE:** exact-match-or-refuse spawns (E1), warp fade-back guards,
  shader skip-draw-on-compile-fail, audio size-mismatch refusal. A refusal logs a line;
  a guess summons Ganondorf (literally — he is HENNA0 arg 1).

## 1. Adapting arcs (offline, once per arc)

Tool: `tools/ww_crew_restoration_skeleton/adapt_bdl_arcs.py <src> <dst>` — Yaz0-aware,
in-place RARC patch:

1. **BDL→BMD:** strip MDL3 section, retag `J3D2bdl4`→`J3D2bmd3` (the port's BDL loader is
   untested decomp code and crashes; the BMD path is battle-tested).
2. **normalize_litmask:** enabled channels → mask 0x01 (TP feeds light slot 0 only; slot 1
   garbage blacks the channel).
3. **normalize_tevregs:** TEV C-reg gray placeholder (128,128,128)→white (WW's runtime
   overwrote it per-frame; unpatched = permanent half brightness).
4. **adapt_dzb:** clear collision word0 bits 14–23 (`0x00FFC000`) — TP reads them as
   through-flags; WW data sets them all → every polygon becomes pass-through.

Runtime draw uses the WW-boots cel branch (`d_ww_itemmdl_pc.cpp` ~1525): fixed ambient +
WREG trim. Measured 1:1 vs real WW on the Ivan exemplar.

## 2. Standing up a stage (map/space)

1. **Anchor + host:** WW sea-room MODELS are cell-LOCAL (№22); the DZB is WORLD-baked.
   Model mtx = translate(host); dzb mtx = translate(host − anchor). Outset anchor =
   (-200000, 0, 300000). Manifest: `model_space=local`.
2. **Manifest** (`npc/ext_bgN.ini`): proc/socket/arg, `arc=`, `model=model.bdl`,
   `collision=room.dzb`, `host_pos=`, `anchor=`, `spawn_rel=`, `population=<csv>`,
   `population_stage=<name>`.
3. **Census export:** extract the stage's `room.dzr` placement chunks (ACTR/SCOB + layered
   ACT0..ACTb) to CSV: `layer,room,name,params,x,y,z,scale?,ry,extra`. This CSV *is* the
   population source — Nintendo's own placements, never hand-authored positions.
4. **Cache discipline:** publish-on-success only (F3); wipe `dawn_cache.db*`/
   `pipeline_cache.db*` after rebuilds.

## 3. Populating actors

`population/actor_map.ini` maps census names → engine:

- One `[section]` per census name: `proc=`, `arg=`, `unique=`, plus part keys (§5).
- **Resolution is exact-match-or-refuse (E1).** Unmapped names = logged holes.
- **Duplicates of one person on the same layer = Nintendo's event-gated alternates**
  (decideType pattern): keep the canonical row, `disable_pos=x,y,z` the alternate. They
  are NOT extra people (the two-Sue-Belles lesson, №37).
- **Sky-parked rows** (y > host + 20000) = Nintendo event parking: skip + log (№42).
- **Skip-list:** arcs you can't identify yet are commented holes — no guessing.
- **Spawn ledger (E2):** every spawn logs
  `[Spawn] src=census:<name> proc=<> arg=<> manifest=<> head=<model@joint|MISS:reason>`.
  Every debugging cycle in this project started by reading these lines.
- **Defense-in-depth:** dangerous payloads get `spawn_if_flag=` even though the resolver
  is exact (Ganondorf is gated behind `beat.gnd_duel`).

Sockets: retail stub procs (NPC_MK, P2, KDK, HENNA0…) host payloads multiplexed by
`socket_arg`. The №44 EXT_SLOT pool (one slot = one payload, `socket=AUTO` scan
assignment) structurally ends arg-collision accidents; new content should use it.

## 4. Interiors + doors

1. **SCLS decode:** the exterior stage's `room.dzr` SCLS table lists the exits in order
   ([0] LinkRM … [9] LinkUG for Outset). Door placements (TGDR, stride 0x24 — №36-D)
   carry position + rotY but **destination params are sentinel** — binding is POSITIONAL.
2. **doors.ini:** one section per destination: `ww_pos=`, `ww_ry=`, `radius=`,
   `enter_proc=EXT_BGn`, `knob=1`. The census `ry` MUST be applied — doors face as WW
   authored.
3. **Interior manifests:** each interior is its own `ext_bgN` mount (own host far from the
   exterior). `spawn_rel` from the interior's own PLYR spawn entry.
4. **Door actor is TWO models (№50-E, from tww d_a_knob00):** `door.bdl` = tiny
   animated frame/controller holding joint `DoorDummy` (the open BCK `dooropenadoor`
   plays HERE); `door_a..h` = the visible geometry drawn each frame at
   `setBaseTRMtx(controller.getAnmMtx(DoorDummy))`. Variant by shapeType; the decomp's
   default case is `door_a`. Pivot = actor position, NO hinge offset (№37).
5. **Door interaction is TP-NATIVE (№53/№60, user-ratified STANDING SPEC — "TP
   mechanism + WW content"). The seven beats of a right-feeling door:** prompt →
   instant control lock → Link's open anim + door swing/SFX → screen cover mid-anim →
   placement inside facing in → fade lifts while Link auto-walks in and the door
   closes behind → release. Feel lives in the lock, the hand-on-door, the walk-in.
   Implementation: the Knob prop is a real door
   interactible — TP's own knob-door attention/demand + Link's open-and-step-through
   demo + the WW open BCK on the controller. The pinned BG warp stays as the transition
   BACKEND (TP's stage system can't load WW stages; the BG mount is the room loader).
   Spawn prop + stamp key + register attention in ONE loop over ONE array (1:1 assert,
   boot-log all keys) — split iterations scramble key↔position pairings (№53).
6. **Exits are first-class doors, and Nintendo authored every position (№56):**
   - Interior door = the interior dzr's `KNOB00` ACTR (every house has one, with a PLYR
     spawn standing on it). The exit interactible goes THERE (`exit_door_rel`), and the
     door-entry spawn is that co-located PLYR.
   - PLYR spawn IDs live in the aux word @0x1C low byte — NOT the chunk index. IDs ≥200
     and the id-0 oddballs are EVENT/cutscene spawns (TagEv triggers sit next to them);
     never use them for door entry.
   - Return destination = the interior's own SCLS → sea spawn id → the exterior stage's
     PLYR with that id (`return_pos`/`return_ry` in the manifest). Never compute a porch.
   - Not every transition is a door: WW forest/cave entrances are WALK-THROUGH loading
     zones (`knob=0 walkthrough=1`) — houses get doorknobs, natural passages don't.
   - Door-class taxonomy (№58): `knob` / `walkthrough` / `crawlthrough` / `dropthrough`
     (+ladder). In-room extra "doors" are hatches/openings of these classes or
     cutscene-spawn markers — never assume a second entrance.
   - Same-named stage arcs can be STATE VARIANTS of one room (census-diff before
     assuming two rooms — the two Ojhous arcs are both Orca's hall); a "missing room"
     may be a separate Room1.arc the extraction never grabbed.
   - Interior mounts must be WARMED (background resLoad after the exterior mounts) —
     cold mounts blow the door-transit timeout and read as flaky interaction.
   - G-guard: leaving the interior shell's AABB without a warp teleports Link to the
     return spawn.
7. **Interior hosts park FAR offshore** (150k+, spaced 25k) so rooms are never visible
   or swimmable from the playable space — land parking WITH the exit warp, never before
   (far-sea softlock). WW-internal positions are untouched; hosts are engine bookkeeping.
8. **Warp guards (G-guard for warps, №34-P4):** fade → interior create must reach
   COMPLEATE + ground-probe hit at spawn, else ABORT and fade back to the door; fade-in
   after placement is one-shot-watchdogged. Link is never left controllable in darkness.
   All warp lanes (door AND menu) go through the same place-at-spawn + probe path.

## 5. Limbs, heads, parts (the companion system)

WW bodies are MULTI-MODEL: body + separate head/hands/props. Facts before mechanism —
dump JNT1 joint names + bind transforms for every member (scratch scripts exist;
№48/№50 pattern):

- **actor_map keys:** `head_model=`, `head_joint=` (body joint, usually `head`);
  `companion=` (second part model); `companion_slave_map=<companion_joint>:<body_joint>,…`
  for multi-attach parts (Aryll `ls_handL:handL,ls_handR:handR`; Quill
  `armLloc:armL,armRloc:armR`); `body_bmt=` for material-override variants (Joel's shirt
  `ko02.bmt` over the shared kid mesh, №50-C).
- **Attach semantics (v5, FINAL — №50-A): PARENT-COMPOSE.** Each frame, inside the
  companion's calc at the joint-callback stage (BEFORE the envelope/weighted matrix pool
  is built — №49): companion joint world = body target-joint world × companion joint's
  own LOCAL BIND matrix; descendants recompute under it.
  - NOT name-matching (only Rose's head coincidentally matches — №48).
  - NOT raw replace (kills authored root rotations — Zill's slant, №50-A).
  - NOT base-at-joint multiply (double-transforms root-origin models — the float, №47-B).
  - Post-calc patching misses envelope-skinned verts (the hair/booger stretch, №49).
- **Head selection is BY CENSUS NAME** (per-proc fixed model), never params guessing.
  Multi-head bodies (Bm's 10): decode the census row's own params low byte
  (zero-index → 01-member).
- **A body may never spawn headless**; single-head arcs attach unconditionally; every
  MISS logs its reason.

## 6. Animation

- **Idle starts at create-COMPLEATE on EVERY spawn lane** (population, door, cutactors) —
  a T-pose means no anim was bound at all (№47-A). Manifest keys: `idle=`, `talk=`
  (+ brk/btk). Talk binds on interact; idle resumes after.
- Companion parts are NOT independently animated in v1 — they ride the body via
  parent-compose (arms follow because body BCKs animate the arm stub joints).
- **Future polish (logged №50):** heads have their own synced BCK sets
  (`kohead01_wait01` ↔ `ko_wait01`; `susuri` = Zill's sniffle; `.btp` = blinks). The full
  Nintendo system plays the matching head BCK alongside the body's.
- Anim member names are per-arc — audit that every manifest's `idle=` exists in its arc
  (the wait.bck short-name refusal, №47-A fix).

## 7. Story / quest gating

- **Layers ARE the story schedule (№35-H5):** base+ACT0 = day-1; map later layers to
  quest flags in `actor_map [layers]`: ACT4-7=`qs.ah_state`, ACT8=`qs.pirates_ashore`,
  ACT9=`qs.aryll_taken`, ACTa/b=`qs.depart`. The island's population shifts with the
  beats exactly as Nintendo authored. Omit (don't blank) layers you haven't verified —
  blank = always-on.
- **Flags:** dSv_reserve_c hashed store (v2 = 616 bits). Dialogue conditionals:
  `if_flag/unless_flag/set_flag/next/else` (the Ivan met/again exemplar).
- **Per-row gates:** `spawn_if_flag=` for beats that need them regardless of layer.
- **Rupee/pickup grants** credit TP systems; visible object stays WW (§0).

## 8. Verification loop (every build)

1. History reads the smoke log BEFORE the user plays (`[Spawn]` ledger: zero MISS, zero
   REFUSED-that-shouldn't-be, `src=` populated; shader compile failures = skip-draw lines,
   never crashes).
2. User plays a scripted checklist (doors, heads, layers, one talk cycle).
3. Ferry exact log lines for any failure — root-cause from data before ANY engine
   iteration (`do not iterate blind`, №36-E).
4. Identity pass: user Cycles + Locks names into `identity.ini`; labels unverified until.
5. `greplist.txt` gate before any push (M6): no WW names/bytes in tree.

## 9. Failure classes already solved (symptom → cause)

| Symptom | Cause / rule |
|---|---|
| Crash loading WW model | BDL loader — adapt offline (§1) |
| Model renders black | litmask slot 1 / toon ramp — §1 steps 2–3 |
| Everything half-bright | TEV C-reg gray placeholder — §1 step 3 |
| Walk through everything | DZB through-bits — §1 step 4 |
| Collision offset from model | split convention: model local, dzb world (§2) |
| Wrong NPC appears (imposters) | loose resolver — E1 exact-match-or-refuse |
| Two of the same villager | event-gated alternate rows — disable_pos (§3) |
| T-pose until talked to | idle never bound on that spawn lane (§6) |
| Head/part on the ground | name-match attach never fired (§5) |
| Head floats/tilted above | base-at-joint double transform (§5) |
| Hair/soft parts stretch to ground | attach ran after envelope pool build (§5/№49) |
| One head slanted, rest fine | authored root bind rotation discarded — parent-compose (§5) |
| Identical-body characters | material override table (`body_bmt`, §5) |
| White wedge instead of door | bare controller drawn — two-model door (§4) |
| Enter door → void | placement/probe — warp guard + log ferry (§4) |
| Shader crash on new NPC | first-exercise TEV config — skip-draw guard (№46) |
| Boot crash after rebuild | stale caches — wipe dawn/pipeline cache |
