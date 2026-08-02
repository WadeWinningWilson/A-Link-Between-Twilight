# The Space Kit — complete-receptor hosting for donor interiors (Foundry §298)

> **The reflection that demands it:** getting ONE cutscene (Grandma's tale) to play in
> ONE hosted room consumed §243–§297 — because every missing piece was discovered as a
> runtime failure, one crash/freeze/fatal at a time. The native game never has this
> problem: **every stage carries its COMPLETE data surface whether or not a given
> playthrough touches it.** The Space Kit's charter is exactly that: port the donor
> space's ENTIRE receptor up front, so anything the donor code asks for is already
> waiting — the way the native game defers it.

> **⟳ REFINED per bus §323 (tale CLOSED end-to-end, 2026-08-01).** Sections 1–3 below are
> the original §298 design — correct on COVERAGE. What the closed tale added — and what the
> kit must bake, not discover — is in **§0 (the teardown-archetype classifier)** and the
> field-level rules woven into Passes 1–3 (REVT exit bytes, PLYR byte layout, prm channel 9,
> id-collision policy). Read §0 first: **classifying an event's teardown archetype from its
> staff data is now Pass-1's first act, before anything else is baked.** Full receipts: bus
> §323, §304/§317 golden traces, §318–§322b fix chain, `docs/state/ww-tale-dmesg-live-state.md`.

## §0. TEARDOWN ARCHETYPES — classify EVERY event here FIRST (the §323 keystone)

The single discovery that closed the tale after 20+ sections: **every WW event ends in one of
two ways, and which one is READABLE FROM ITS STAFF DATA before any runtime.** Debugging finish
flags before establishing the archetype cost weeks (the donor never even evaluates finish flags
on archetype A). The kit MUST decide this in Pass 1 and bake the matching requirements.

**Decider (Pass-1 mechanical): does any staff carry a `Stage` + `StartCode`?**

- **A — EXIT-FUL** (the tale; any staff has `Stage`+`StartCode`, *even same-stage*): on STB end
  (demo mode 2), `dEvDt_Next_Stage` fires **unconditionally** — the donor has **no** same-stage
  no-op — so a **reload IS the teardown**. The event is killed mid-flight (finish flags never
  evaluated); the arrival **wipe-in restores any STB-authored fade**; `DEFAULT_START` runs on
  re-entry; the **landing spawn sits AT the STB end transform** (the donor's continuity trick).
  → Kit bakes: a landing PLYR point at the STB end transform (evt byte `0xff`) — and accepts
     that fade channels in the STB REQUIRE this path to undo them. (The same-stage reload ALIAS
     is port-side CODE (§322, d_event_data.cpp) — not bakeable data; the kit only VERIFIES the
     dependency is landed, via the Pass-4 manifest. — Foundry scope trim, §325)
- **B — EXIT-LESS** (no `Stage` in any staff): on STB end → `demo_remove()` → mode 0 → `cutEnd`
  sets the PLAY cut's flag → a **WAIT cut carries the event finish flag** → `finishCheck` → close
  **in place**. The **REVT exit bytes MUST be `0xFF`** or TP's Sequencer `sceneChange`-loops
  forever (§321).
  → Kit bakes: the WAIT finish-flag cut in event_list (donor TALE_DEMO: PLAY flag 3 → WAIT flag
     9), REVT exit bytes `0xFF`.

**Why this ordering matters (method lesson §323.5.4):** the donor never needed the tale's "flag
9" at end — it exits via reload (A). Weeks went to making a finish path work that the donor
bypasses. **Archetype first, from staff data; finish-flag debugging only for B.**

**§0b — NATIVE TRIGGERS are kit surface too (user directive, §361).** The tale's last bug was a
TRIGGER-GATE reconstruction error (3D sphere where the donor authors a CYLINDER — XZ dist +
|ΔY|<100 band, §335 disassembly). Two trigger classes, both kit-covered:
- **Data-authored** (TagEv/SCOB rows): already in the cast bake — and the TRIGGER VOLUME IS THE
  SCOB SCALE (r = scale.x·100, h = scale.y·100, §312); Pass-3 asserts scale bytes survive.
- **Actor-code** (XyCheck family): the donor idiom is the CYLINDER (dEvt_info_c::setXyCheckCB —
  XZ radius + separate Y band). Pass-4's per-actor manifest flags any ported trigger actor whose
  gate was reconstructed rather than disassembled — a 3D-distance test in a ported trigger is a
  REVIEW item until receipted against the donor rel (the §335 disasm recipe makes this cheap:
  files/maps/<rel>.map names the function, capstone reads it).

The lifecycle both archetypes share (kit reproduces this pipeline):
```
NPC order (actor code) → order event's warp cut: setNextStage(stage, entryPoint)
  → RELOAD → arrival PLYR point whose EVENT BYTE auto-fires the REVT event (0xC8 → REVT[0])
  → REVT event = PACKAGE staff PLAY cut → STB plays (cast binds via JSGFindObject)
  → STB end → archetype A (reload) or B (remove→cutEnd→WAIT-flag→close-in-place)
```

## 1. Everything the tale actually required (the receipts, grouped)

### A. Actor code (the performers)
| # | subsystem | § |
|---|---|---|
| 1 | fopNpc_npc_c + dNpc_* framework | §243 |
| 2 | d_a_npc_ba1 itself (un-decompiled → dtk+m2c pipeline: rel_decomp, jtbl pre-pass, **rodata tables** — anm_prm, l_evn_tbl, ptmf action tables) | §256–§260 |
| 3 | WW demo00 (the d_act puppet actor — TP's twin misreads WW STB data) | §271 |
| 4 | Actor-integration fix classes: staff-claim `this` (§270), modelCalc render (pitfall C), TEV_TYPE_ACTOR lighting, anm_prm endianness, `getMyActIdx` 0-vs-−1 (§295) | various |

### B. Cutscene/demo engine (the storyboard machinery)
| 5 | dDemo scripted-face trio (getP_Btp/Btk/BrkData) | §251/§252 |
| 6 | dDemo_setDemoData current.angle | §249 |
| 7 | STB cast binding + d_act on-demand creation (existed; mapped) | §269 |
| 8 | Demo-arc residency (single global slot) | §278 |

### C. Event data (the script)
| 9 | Event registration into the host list (merge_event: tale_1/tale_2/Use_Fairy/Ganbaru; TALE_DEMO/2) | §265 |
| 10 | Event-semantics knowledge: PACKAGE = the STB player; CAMERA PAUSE/STBWAIT; DIRECTOR FADE; mFlags finish accounting; flag-timeline meanings (PLAY=end, PAUSE=start) | §280–§292 (knowledge kept; collapse reverted §293) |
| 11 | Port event-manager fix: getMyActIdx return convention | §295 |

### D. Stage data (the receptor — where most §s went)
| 12 | REVT stage-events (donor EVNT → TP REVT, id==index, switch guard) | §273/§277 |
| 13 | PLYR spawn points (donor re-entrance ids — **the §297 lesson: ALL donor points, not the ones currently used**) | §297 |
| 14 | MULT trimmed to co-renderable rooms | §276 |
| 15 | ALL-staff scoped out (1-room donor data in multi-room host) | §275 |
| 16 | RTBL integrity (the offset-stable bake law) | §277 |
| 17 | SCLS/DOOR/ACTR native population (tool built, --write parked on room-map) | §267 |

### E. Entry/transition
| 18 | setNextStage host-name mapping + entry wire (setStartDemo per pending spawn) | §272/§273 |
| 19 | The mode-8 wipe (the donor's own fade — arg verbatim) | §293 |

### F. Dialogue/audio (owed / staged)
| 20 | WW event-flag namespace (dialogue variants, progression, multi-event sequencing) | OWED — the ceiling |
| 21 | JMSG native rendering (DN-4 path) | History's phase |
| 22 | Message voice (charVoiceTable + dispatcher SE decoded, data staged) | §263 |

### G. Supporting data integrity
| 23 | Arc res-ids (donor-exact; audit + residmap belts) | §268 |
| 24 | Demo01 staging + FileName resolvability | §278/§281 |

## 2. The kit answer: a NEW kit class — the SPACE KIT

The Actor Kit (enemy/NPC + cutscene tier) covers **performers' code**. The decomp
pipeline covers **un-decompiled sources**. Nothing covers **the receptor** — and D/E
above is where this campaign bled. The Space Kit is that missing class: **one command
per donor stage → hosted space, porting the COMPLETE donor data surface.**

### Composition (nearly all pieces already exist as §-proven tools)
**Pass 1 — INVENTORY (donor-authoritative):** parse the donor stage wholesale: every
EVNT row, every PLYR point (all of them), every SCLS/DOOR/ACTR/SCOB (all layers), the
full event_list, every .stb cast + demo-arc contents, dialogue/message ids, event bits
read/written (the flag namespace's per-space manifest).

*§323 additions — Pass 1 now also decodes and classifies:*
- **Teardown archetype per event** (§0): scan each event's staffs for a `Stage`+`StartCode`
  → A (exit-ful) else B (exit-less). Emit it into the manifest; Passes 2/3 branch on it.
- **PLYR byte layout** (room.dzr PLYR, 0x20/entry): `byte 0x08` = **REVT event index**
  (`0xff` = none — this is the native cutscene AUTO-TRIGGER), `byte 0x1D` = **spawn id**,
  floats `0x0C` = pos, s16 `0x1A` = angY. Record every point's (event-byte, spawn-id, xform).
- **REVT field layout** (stage.dzs REVT, 0x1C/entry): `byte 0x7` = finish-exit, `byte 0x9`
  = skip-exit, type byte (0=maptool / 1=ZEV / 2=STB) drives the skip-proc choice.
- **STB demo00 prm channel semantics** (per cast, for the fade/give dependency map): `4` =
  WW save event-bits (parse-and-drop), `5` = item-give (BRIDGE-OWED, deferred), `6` =
  monotone, `7` = rumble/shock, **`9` = BLACK FADE / `10` = WHITE FADE** (payload: dir byte
  0=out else in, then frames). A **fade channel present ⇒ the event MUST be archetype A** (only
  the reload wipe-in restores the screen) — Pass 3 asserts this pairing. (§304's "channel 9 =
  give/handover" was WRONG; it is the ending fade-out — a week's misread, now corrected.)

**Pass 2 — BAKE (compose the campaign's tools):**
- `merge_event` — **every donor event for the space**, not a hand-picked subset. *§323:* keep
  the donor's **ALL/dummy staff** (port merge dropped it — harmless so far but not donor-verbatim);
  for **archetype-B** events, ensure the **WAIT cut carries the event finish flag** (donor
  TALE_DEMO: PLAY flag 3 → WAIT flag 9) — without it, B never closes.
- `revt_bake` — every EVNT row (offset-stable, RTBL-probed). *§323 CRITICAL:* **exit bytes
  `0x7`/`0x9` = `0xFF` unless donor-authored** — zero-fill means "take exit 0 at finish" =
  infinite `sceneChange` loop (§321). Set the **type byte deliberately** (0/1/2 — the tale was
  baked ZEV, flagged §321).
- `plyr_append` — **every donor PLYR point** (§297's law: data waiting to be called). *§323:*
  **teardown/continuity spawns sit AT the STB end transform** with **event byte `0xff`** (e.g.
  the tale's (−290,375,85) @0x8000) — bake them verbatim; a teardown spawn that carries an event
  byte, or lands on the entry point (0xC8), **re-triggers the event in a loop**.
- **id-collision remap** *(§323, new bake step):* the host's port door convention owns **point
  `0`** (`armNativeStageChange`); donor StartCodes that collide get **remapped ids + a translation
  in the stage alias** (tale: donor `0` → host `0xCB`, §322b). Pass 1 detects the collisions;
  Pass 2 applies the remap uniformly per space. (An alternative — re-iding the HOST's port doors
  to the donor's scheme to free id 0 — is a port-code convention change, OUTSIDE the kit's
  data-receptor charter; if wanted, that is a user ruling on the port side, not a kit bake
  option. — Foundry scope trim, §325)
- `bake_room_chunks` — SCLS/DOOR/ACTR/SCOB (offset-stable refit REQUIRED first). *§323:* host has
  **no SCLS** (doors are port-wired); any donor event `'ID'` param (SCLS-indexed exit) needs the
  host SCLS baked or the same alias treatment.
- `scope_event_staff` + `mult_trim` — host-context hardening, auto-applied.
- demo-arc bundling (ONE arc per space — pitfall D/P6) + residmap audit. *§323:* the demo arc must
  be **resident at the entry point**; **fade-box resources** (`blackfadebox.bdl`, `fade_*.brk`)
  ride in it — required for archetype-A wipe-in to restore the screen.

*Port-side code this bake DEPENDS ON (already landed, donor-verbatim — the kit bakes data that
pairs with these):* §306 (`dEvDt_Next_Stage` returns FALSE on no Stage/StartCode), §319/§319b
(exit-less end = `dDemo_c::remove()` + getCamera null-guard), §320 (removed port-added WAIT gate in
`cutEnd`), §321 (REVT `0xFF`), §322/§322b (same-stage reload alias + StartCode 0→0xCB remap).

**Pass 3 — VERIFY (the offline battery, all §-proven probes composed):**
- RTBL room-count probe; untouched-region byte-diffs; same-reader re-dumps;
- **event-closure completeness** (the §280/§287 class, mechanized): every event's
  chain has its player (PACKAGE FileName resolvable IN the bundled arc), staff
  rosters complete vs donor, finish-flag accounting donor-verbatim;
- **receptor-coverage cross-check** (the §295/§297 class, mechanized): every spawn
  point referenced by ANY setNextStage/SCLS/EVNT in the space exists in PLYR; every
  event name referenced by any actor's l_evn_tbl resolves; every REVT id coherent;
- collision scans (flag ids, res ids, OBJNAME shadows).

*§323 archetype asserts (the failure catalog, mechanized — each turns a former runtime
freeze into a bake-time failure):*
- **A-fade pairing:** any event whose STB uses prm channel 9/10 (fade) MUST be archetype A
  (exit-ful) — else the reload never restores the screen → "hangs faded-out forever,
  mEventStatus=1."
- **REVT exit-byte assert:** every REVT exit byte is `0xFF` unless donor-authored → else
  "finishCheck passes but sceneChange spams forever" (§321).
- **B-closure assert:** every archetype-B event has a WAIT cut carrying a finish flag → else
  "PACKAGE PLAY flag never sets / never closes."
- **teardown-spawn assert:** every archetype-A landing spawn has event byte `0xff` AND is NOT
  the entry point → else "tale re-triggers in a loop."
- **StartCode-collision assert:** no donor StartCode resolves a colliding host id (door=0) →
  else "reload lands wrong: void fall + KNOB_START + letterboxes" (§322b).
- **archetype-first gate:** the report leads with each event's archetype (from staff data), so
  a human never debugs finish flags on an archetype-A event (the §323.5.4 lesson, enforced).

**Pass 4 — DEPENDENCY MANIFEST (cross-kit):** the space's ACTR/cast roster linked to
Actor-Kit status per actor (ported/drafted/stub), and the engine-subsystem checklist
(A/B-class items: landed vs owed) — so a space declares what CODE it needs, and rooms
that lack a performer degrade visibly (deferred list), never by crash.

> **THE COINCIDENCE RULE (user ruling, §328): a kit bake LANDS only when its room's
> deferred list is EMPTY.** Before any Space Kit work lands on a room (and, once the
> kit does multiple rooms, on any room in the batch), Pass 4's non-ported item list is
> emitted FIRST and ferried to the owning lanes (History / Housing / Foundry per item).
> Scratch bakes, dry-runs, and verify batteries run any time — but the LIVE landing
> coincides with full cast coverage, so a kit-baked room never ships part-populated.
> The deferred list is the work order; the bake is the finish line.

### The coverage guarantee (the user's requirement, §298)
A Space-Kitted room carries **the donor's complete data surface**: rooms that never
play a cutscene still carry their EVNT/PLYR/event-list data — exactly how the native
game defers it. The failure mode this kills is the §-by-§ discovery loop: "ordered but
no player" (§280), "point not in PLYR" (§295), "no REVT to order" (§279), "arc not
resident" (§278) all become **bake-time verification failures instead of runtime
freezes.**

### What stays outside the Space Kit
- Performer code → Actor Kit (+ decomp pipeline for stubs); linked via Pass 4.
- Engine subsystems (B-class) → decode/port campaign items, one-time each, tracked
  on the owed ledger.
- The WW event-flag namespace (F.20) — engine work, THE prerequisite for multi-event
  sequencing in any Space-Kitted room; Pass 1's flag manifest feeds its design.

## 2b. The HYBRID HOSTING RULE (user ruling, §302) — and why the kit stays one kit

**The rule (mechanical, Pass 1 auto-decides):** donor RTBL count > 1 or a donor MULT
group → the donor stage gets a **DEDICATED host stage** (donor structure verbatim:
rooms 1:1, MULT verbatim, EVNT→REVT whole, event_list merged whole, PLYR verbatim —
only the stage-name alias translates). Single-room donor stages → **PACKED host**
(TP's own R_SP01/R_SP109 interior pattern; room/SCLS remap + host-context
translations apply). Dungeons naturally fall on the dedicated side.

**Why quests work ACROSS shapes:** exactly two things cross hosting shapes, and both
are shape-agnostic:
1. **Quest state = the WW event-flag namespace** (save-side bits) — a flag set by the
   tale in packed R_DL01 room 0 is read by Orca's dialogue in dedicated R_DL02 as a
   pure bit read; the block has no notion of stages. (One more reason the namespace
   is the critical path — it is the carrier of ALL cross-room quest flow.)
2. **Movement = stage transitions** (SCLS/setNextStage) — a transition into a packed
   room and one into a dedicated stage are the SAME native mechanism; only the dest
   name in the translation map differs. The player cannot tell the shapes apart.
Also unchanged: the ONE-event-at-a-time law (the run slot is global, not per-stage).

**Kit impact — one kit, two bake paths:** Pass 1 reads donor RTBL/MULT and selects
the path; Pass 2's dedicated path is the SIMPLER one (verbatim-heavy, fewer
translations — no room remap, no MULT trim, no ALL-staff scope needed when the host
mirrors the donor 1-room-set); Pass 2's packed path keeps the §-proven translations.
Pass 3 verify and Pass 4 manifest are identical across shapes. The scan gains rule
check **R12: donor stage hosted in the WRONG shape** (Ojhous2-packed is the first
finding of this class). Dedicated path adds one bake step: host-stage identity
registration (parametrized shell builder — R_DL01's own tool, reused).

## 3. Build order (proposal)
1. **§0 archetype classifier** (Pass-1 first act) + the PLYR/REVT byte decoders — cheapest
   code, highest §-prevention: it front-loads the one decision the whole teardown hinges on;
2. `bake_room_chunks` offset-stable refit (P1 debt — blocks everything's --write);
3. Pass 3 verifiers — closure-completeness + receptor-coverage + the **§323 archetype asserts**
   (fade-pairing, REVT-0xFF, B-closure, teardown-spawn, StartCode-collision);
4. Pass 1 inventory + Pass 4 manifest;
5. Driver composing the passes; **R_DL01/LinkRM as the pilot — now a REGRESSION check, not a
   discovery run:** the tale is CLOSED donor-clean (§323), so the kit's first run must classify
   it archetype A, bake the same-stage reload + (−290,375,85)@0x8000 landing + REVT `0xFF` + the
   demo-arc fade box, and reproduce the proven end-to-end result byte-for-byte. Any deviation is
   a kit bug, measured against a known-good target.

## 4. §323 method lessons baked into kit practice
1. **Read the donor function before inventing a premise.** Both covenant lapses in the tale
   saga (box-widening, "same-stage exit is a no-op") explained a port behavior by a donor
   mechanism *nobody had read*. The kit's asserts cite donor file:line, not inferred behavior.
2. **Cross-check event ids against the stage event table before trusting a negative.** The golden
   trace's "no reload at exit" was an instrument blind spot (a same-stage reload behind black onto
   an identical transform is invisible to a transform probe) — but `DEFAULT_START`/"evt 49" in the
   same capture was the reload's fingerprint. Pass 3 verifiers name event ids, not just transforms.
3. **Archetype BEFORE finish flags.** Enforced by the archetype-first gate (Pass 3).
