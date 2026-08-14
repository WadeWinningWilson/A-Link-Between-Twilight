# BUILD QUEUE — lanes queue, the Integrator builds

> **USER RULING 2026-08-12 (tale §839):** *"you are supposed to be the only
> builder. Other lanes are supposed to queue their work/ports and you reference
> a tool to verify their work is decomp vanilla byte true."*

**DO NOT BUILD.** If you are not the Integrator, you do not run `build_run.bat`,
`launch.bat`, or any `run_*.bat` that builds. You append a row here.

---

## Why (the evidence that produced this file)

One session of lanes building directly produced: exe timestamps moving mid-
investigation (a build bisect had to be withdrawn — tale §785), a tree broken by
an unannounced landing and discovered only through a build failure (§839), and
two instruments encoded on a premise that was still wrong (§826/§829 → §828).
**Every one of those is caught by a queue.** The cost of queueing is minutes;
the cost of not queueing was most of a session.

---

## How to queue

Append one row to **PENDING** below. One row per landable unit.

```
  <lane> | <what> | FILES: <paths> | DONOR: <evidence> | VERIFY: <tool hint> | <bus §>
(prefix a REAL row with `- [ ]`; this template deliberately does NOT, so it is
never counted toward MIN-BATCH-5)
```

- **DONOR** is not optional. Name the donor source you read — a TU, a disc path,
  a debug-map table. **A row whose justification cites only receiver files has
  not cleared DN-10 step 1** and will be held.
- **VERIFY** names the tool that should decide it, if you know. Guessing is
  fine; the Integrator re-derives regardless.
- If the item is an instrument or a doc, say so — those gate differently
  (instruments may be UNKNOWN-clean; content may not).

## Batch + card rules (USER-RATIFIED 2026-08-13, tale §848/§850)

1. **MIN-BATCH-5:** the Integrator builds only when **≥5 ACCEPTED port rows**
   are waiting — one build, five-plus ports, one playtest battery. (Whether
   tree-breaking FIXES ride outside the batch is the Integrator's question to
   put to the user — tale §848.)
1b. **THE UNIT IS A PORT ACTOR, NOT A ROW (Integrator gate ruling, tale §860/§862).**
   One row may carry several actors (`obj_paper`+`obj_plant` is one row, two ports).
   The user asked for *"multiple ports from them in one build"* — actors are what
   the player sees, so actors are what the batch counts.
   **ENGINE/SYSTEM rows do NOT count toward the five** (LBNK pre-resolve, `atr_conv`
   …); they are the planner's systems track and ride when ready.
   **A fix that unbreaks the tree, or closes a live defect, builds OUTSIDE the batch** —
   every port in a batch needs a working tree to be verified in.
   **THE FIVE ARE SELECTED FROM THE PLANNER'S RANKED HEAD** (`PORT-QUEUE.md` Phase 1),
   not from arrival order — user ruling: the rules "flow from" the planner.
   **Current: 3 port actors queued** — `obj_paper` · `obj_plant` · `npc_p1`. **Need 2.**
   Cheapest ranked candidates: `d_a_lwood` (27 placements) · `d_a_ki` (26) ·
   `d_a_ks` (13) · `d_a_obj_lpalm` (10) — all MATCHED, 0 markers.

2. **CARDED ROWS:** every port row arrives WITH its preflight card —
   `tools/foundry/port_preflight.py <actor> [--post <recv.cpp>]` — one
   command, the whole instrument battery. The gate re-derives regardless; a
   carded row needs one look, not six.

3. **DEP-GATE (user-ordered 2026-08-13 — the palm lesson, caught at this gate
   twice):** a port row is HELD unless every donor SYSTEM it touches is
   PORTED/DESCENDANT in `tools/foundry/port_deps.py`'s registry — OR the
   system's own row rides the SAME batch — OR the user waives it LOUDLY on the
   row. Symbol-present-but-system-unarmed (WW-WIND) is exactly what this
   catches; name matching cannot. Registry statuses move by bus receipt only.
   Reverse index: `port_deps.py --unlocks <SYSTEM>` lists a system's
   beneficiaries so they ride its wave.

## What the Integrator runs before building

| tool | question it answers |
|---|---|
| `decomp_status.py <tu>` | is the donor TU MATCHED or NONMATCHING? A NONMATCHING donor cannot be "ported verbatim" and must say so |
| `l2c_equiv.py` | are the runtime's adapted bytes IDENTICAL to the reference? (byte-truth) |
| `kit_laws.py <file>` | the laws — incl. DN-10-S name-gap triage and the cache-key declaration |
| `kit_output_law.py` | did any kit write a donor file that is not disc-verbatim? |
| `ww_disc.py` | donor ground truth for any claim about donor data |
| `room_layout.py` | does the touched stage have interacting rooms? (36 stages do) |

**Rules of the gate:**

0. **NOT VERBATIM = NOT PASSED (USER RULING 2026-08-13).** A donor TU carrying ANY
   `/* Nonmatching */` marker does not land — regardless of what `configure.py`
   says on the other axis, and regardless of whether the function is written or
   stubbed. **There is no "GO with a declaration" tier**; the Integrator invented
   one for `d_a_obj_lpalm` and it is struck.
   **AND: a non-verbatim TU that reveals an UNPORTED SYSTEM (lighting, wind, …)
   sends that system to History as its own port row.** The blocked actor is not
   the deliverable — the system under it is.

1. **Failed verification does not build.** The finding goes back to the lane
   with the receipt.
2. **UNKNOWN is held, not waved.** An item whose verification cannot run is not
   a passing item (§31-C, applied to the gate itself).
3. **The Integrator re-derives.** A lane's own green is evidence, not a verdict —
   the pairing rule (a lane never audits its own instrument) applies here too.
4. **A broken tree is drained first.** Nothing new builds on a tree that does
   not compile.

---

## PENDING

- [ ] Housing/Engine | INSTRUMENT (tale §897, ride any build): two one-shot daBg WW-host receipts in d_a_bg.cpp beside the §757 probe — [P1] model-fetch receipt at daBg's res-slot model acquisition (roomNo, arc, modelData ptr; NULL = LOUD warn), [P2] first-draw receipt at the room model's first draw dispatch (one-shot per WW-host room). With the existing §757 Regist line these cover create → model fetch → collision → draw; the A_mori fault has nowhere left to hide. | FILES: src/d/actor/d_a_bg.cpp | DONOR: n/a (receiver instrument, §757 idiom precedent) | VERIFY: instrument-class, UNKNOWN-clean; NEVER-PUSH-STRIP-SET registry | tale §897

- [ ] Housing/Engine | ONE-LINE lamp-class fix (tale §892, answering §891's shims-owner flag): d_ext_ww_actor_shims.h:387 `#define dSymbol_DIN_e 0` is WRONG — donor enum is NAYRU=0 / DIN=1 / FARORE=2 (WW DP d_com_inf_game.h:1679-1681). Inert today (behind the dComIfGs_isSymbol FALSE-stub) but a landmine the moment isSymbol becomes real — every DIN query would read NAYRU. Replace the single define with all three donor values (dSymbol_NAYRU_e 0 / dSymbol_DIN_e 1 / dSymbol_FARORE_e 2) so ww_layer_select.cpp's [Y1] seam and any future isSymbol port read donor-true. | FILES: include/d/d_ext_ww_actor_shims.h (one define → three) | DONOR: WW DP include/d/d_com_inf_game.h:1679-1681 verbatim | VERIFY: compile; value-check trivially against the donor lines | tale §892
- [ ] History/Engine | `d_a_obj_plant` + `d_a_obj_paper` ports (landed 23:08 UNQUEUED; tree was broken by it, REPAIRED and built 23:40 after the TUs were pulled — this row is the re-land request) | FILES: src/d/actor/d_a_obj_{plant,paper}.cpp/.h, f_pc_name.h, f_pc_profile_lst.h/.cpp, ww_profile_register.cpp, d_stage.cpp | DONOR: d_a_obj_plant.cpp / d_a_obj_paper.cpp exist in WW DP; Plant x2 + Paper x10 placed in Ojhous2/Room1 | VERIFY: build first — the real `error C####` is above the C4530 warning | tale §817
- [ ] Housing/Engine | WW room OBJECT-ARC PRE-RESOLVE at room load (LBNK-shaped, tale §858 RULED; closes the completion window that breaks shelf pots — engine fix, gates as instrument-class). Derive the room's object-name→arc set from its own DZR ACTR/ACTx chunks (dStage_searchName rows + the plugin's wwObjectArcs machinery) and pre-resolve BEFORE actor creation, mirroring TP's native LBNK pre-load. GATE TERMS attached by §858: (1) measure load-time on a big room (sea,44) before ship; if it spikes, scope to ACTR-referenced arcs only; (2) unresolvable arc = LOUD, never silent fall-through; (3) touches no actor. | FILES: src/d/ext_plugin/ww_room_loader.cpp (room-load hook), src/d/d_ext_npc_mount.cpp (resolver reuse) | DONOR: tale §856 receipts — donor f_pc_create_req.cpp pump semantics (no completion ordering in EITHER engine; GC disc FIFO was the ordering); run 234303 lines 4815/4843/5006 (shelf arc +160 lines after pots); TP's own LBNK idiom is the receiver-native mechanism | VERIFY: run receipt — shelf pots STAND in Ojhous2/Room1 (also closes §818's pot-floor receipt); load-time delta logged on sea,44 | tale §856/§858
- [ ] Housing/Engine | Port WW's READ-TIME attribute conversion — `atr_conv[0x20]` + `dBgS::GetAttributeCode` (bounds-miss → NORMAL) at the WW-host consumption boundary; RETIRES AS A SET: the §334/§654 in-place m_info1 repack, the §334d write-verify probe, the §654 through-clear, and the §334g water workaround (tale §857-Integrator: a guard on a substitution). LAMP-CLASS VALUE CHECK DONE (this row's precondition): the receiver has NO native dBgS_AttributeCode — TP dBgS exposes GroundCode/SpecialCode only — so the donor enum imports wholesale with no twin to collide; BUT the §227 pig shim (include/d/d_ext_ww_actor_shims.h:219-223) defines four dBgS_Attr_* names with INVENTED values (GRASS=2 vs donor 0x04, SAND=3 vs 0x0B, WATER=4 vs 0x13; DIRT=1 coincidentally right) — those defines + the constant-returning dExtKb_GetAttributeCode retire with this port (pig dig-material becomes real). SCOPE NOTE: consumer inventory is part of the item — every WW-host reader of the repacked bytes moves to the read-time accessor; TP-native paths untouched (№283). | FILES: src/d/d_bg_s.cpp + include/d/d_bg_s.h (WW-scoped accessor), src/d/d_ext_npc_mount.cpp (repack retire), include/d/d_ext_ww_actor_shims.h + src/d/d_ext_ww_actor_shims.cpp (§227 shim retire), consumers src/d/actor/d_a_kb.cpp, d_a_npc_zl1.cpp | DONOR: WW DP src/d/d_bg_s.cpp:193-234 (atr_conv table + GetAttributeCode read-time convert, never rewrites stored bytes), include/d/d_bg_s.h:28-57 (enum values incl. WATER 0x13 / WATERFALL 0x17 / METAL 0x14), GetGroundCode = separate field bits 21-25 | VERIFY: kit_laws (№283 WW-scope) + §334d probe receipts as the regression baseline before the probe itself retires | tale §857

- [ ] History/Engine | `d_a_npc_p1` port — GONZO, SENZA & NUDGE (the pirates; NOT villagers — identity corrected during the read). Donor WHOLE, 1448/1448 lines read, authored DARK from the R5 code_dialect table (§854 — first table-driven port). Registration to add at land (grep §858): fpcNm_NPC_P1_e 0x339 (next free after 0x338 Obj_Plant), WW_ROW + count-control bump, born-relinquished NULL slot, OBJNAME rows P1a/P1b/P1c -> fpcNm_NPC_P1_e arg -1 (donor routes verified via R5 object_name: sub 0/1/2 is the unread argument field; type lives in param bits 20-23), files.cmake row. Arc P1 already disc-served. | FILES (dark, ready): src/d/actor/d_a_npc_p1.cpp, include/d/actor/d_a_npc_p1.h, assets/GZ2E01/res/Object/P1.h (generated donor header, Jb precedent) | DONOR: d_a_npc_p1.cpp MATCHED 0 markers; seams [N1] DN-3 acquirer (body/heads/dora by name; anm/btp ID-fetch per the §816 law), [N2] draw-time BMT body swap -> §229 acquireModelDataBmt parse-time bake (receiver J3D lacks setMaterialTable, §227), [N4] kaji/pirateship sentinels (ship context, inert on Outset), [N7] §239 msg dialect, [N8] dEvtMode_TALK_e->dEvtCnd_CANTALK_e, [N10] checkMesgSendButton absent -> false | VERIFY: compile is the first gate (TU is compile-unverified — dark-authored under the no-build law; the §840 lesson is why this row EXISTS instead of a landing); then Outset layer battery: P1a gated on WWEV_0310 draw bit, P1b carries the gong stick on layer 0xA, all three talk with donor message trees | tale §858

- [ ] History/Engine | `d_a_lwood` port — the normal tree (planner head rank 1, 27 Outset placements). Donor WHOLE (199 lines), authored DARK, code_dialect-driven. Solid MoveBG trunk: own DZB via dBgW_NewSet + DN-1's SANCTIONED Regist idiom (no room stamp); wind-sway leaves through the receiver's SAME-LINEAGE dKyw wind (donor get_wind_spd global term native; point-wind half joins the wether pass, seam [L2]); isMonotone photo-gate absent -> false [L3]. Registration at land (grep §861-batch): fpcNm_Lwood_e next-free enum + WW_ROW + NULL slot + OBJNAME routes from R5 object_name (query fpcNm_Lwood_e — donor rows swood/swood3/swood5 already route EXT_VEG §696; the LWOOD names need the R5 query at land to avoid clobbering the veg rows) + files.cmake + arc Lwood (already disc-served). | FILES (dark): src/d/actor/d_a_lwood.cpp, include/d/actor/d_a_lwood.h, assets/GZ2E01/res/Object/Lwood.h | DONOR: MATCHED 0 markers | VERIFY: compile first (dark-authored under the no-build law); then Outset: 27 trees stand solid and sway | tale §862


- [x] History/Engine | `d_a_tag_kb_item` port — the pig dig-item tag ("TagKb", 10 Outset placements; batch slot 5 after the §864 recomposition: ki = enemy-class deferred with the combat-posture question, obj_wood = d_wood-SYSTEM-gated deferred). Donor WHOLE incl. kb_dig/dig_main from d_com_static.cpp:382-449 (retail branch), MATCHED 0 markers BOTH AXES (verbatim-clean per the user's gate ruling). Item spawn through the §227 dExtKb_fastCreateItem boundary (the pig's own shim, same donor call shape). LAND STEP RETIRES THE §225 SHIMS: remove the fpcNm_TAG_KB_ITEM_e 0xFFFE sentinel + the daTagKbItem_c stub class from d_ext_ww_actor_shims.h, re-type the pig's kb_dig call site to daTagKbItemPort_c — the pig gains its REAL dug-item food source. Registration at land (grep §864): real fpcNm_TAG_KB_ITEM_e enum row + WW_ROW + NULL slot + OBJNAME "TagKb" (donor d_stage.cpp:583; R5-verified) + files.cmake. No arc needed (tag; no model). | FILES (dark): src/d/actor/d_a_tag_kb_item.cpp, include/d/actor/d_a_tag_kb_item_port.h | VERIFY: compile; then Outset: pig digs a TagKb spot -> item pops | tale §864  <- LANDED in exe 14:40 (Integrator); row was stale in PENDING.

- [x] History/Engine (LANDED tale §882 — steps 1+2 in-tree, step 3 ABSORB/RETIRE receipts still queued below) | **WW-WIND system** (§865 user rule row: parallel systems, never TP's). NEW dark module: src/d/d_kankyo_ww_wind.cpp + include/d/d_kankyo_ww_wind.h — the donor's OWN wind field (d_kankyo_wether.cpp:985-1140 verbatim: override > evt > tact sources, FILI strength tiers 0.3/0.6/0.9, custom-power, evt-kill, smoothing) on WW-OWNED state; TP's global_wind_influence untouched. Donor accessors WW-side (dKyWw_get_wind_vec/pow/power/vecpow, evt/tact setters, pntwind ring + get_info). LAND STEPS (grep §868): (1) files.cmake row; (2) DRIVER: call dKyWw_wind_set() once per frame from the WW kankyo execute beside the other dKyWw passes; (3) ABSORB/RETIRE per §867's inventory: Ferry-F host-wind arm re-targets its donor FILI read into dKyWw_wind_setFiliLevel and its TP evt_wind carrier retires; the §192 windline block reads THIS field; TP dKyw_* bindings in donor actors become lint targets. CONSUMER RECEIPTS already in the dark tree TUs: lwood + lpalm bind dKyWw_* (lwood's [L2] now carries the donor's FULL wind+pntwind sum). | VERIFY: compile; then Outset: trees sway on the WW field (tact-default direction + FILI tier), TP stages unaffected | tale §868

- [x] History/Engine | `d_a_tag_so` port — NpcSo's zone-marker tag (6 Outset placements; the flow-keeper after ks reclassified enemy). Donor WHOLE (115 lines), verbatim-clean both axes. Passive param tag (zone id, radius 100-unit steps / 0xFF->1600 default, fan flag); NpcSo binds it when that cast-gap port lands (task #14). Donor debug fan + HIO dropped (ls1 posture, seam [S1]). Registration at land (grep §871): fpcNm_TAG_SO_e enum row + WW_ROW + NULL slot + OBJNAME route from R5 object_name (query fpcNm_TAG_SO_e) + files.cmake. No arc. | FILES (dark): src/d/actor/d_a_tag_so.cpp, include/d/actor/d_a_tag_so_port.h | VERIFY: compile; inert until NpcSo consumes it (a marker with no consumer draws nothing — expected) | tale §871  <- LANDED in exe 14:40 (Integrator); row was stale in PENDING.

- [ ] Housing/Engine | INSTRUMENT (sight-only, UNKNOWN-clean class): tale §890 CONTROL-LOSS PROBE for the interior→exterior lock (CALLS row 44, run 144157 ×2). One LOUD state-bundle at WW-host arrival (№90/№89 sites, d_ext_npc_doors.cpp pollArrival) + a 600-frame watchdog re-log if the player is still locked: s_doorDemoLocked · s_arrival{armed,withDemo,demoStarted,demoEnded,demoFramesLeft,guardFramesLeft,cameraSnapped,stage} · alink demo mode + current proc (the single most discriminating value) · dComIfGp_event mode/runCheck · evt1_isActive · dDemo mode + getCamera · dCam Active/Pause · isEnableNextStage · fopOvlpM_IsPeek · mLinkAcch flags. ~10 hypotheses one bundle (multi-hypothesis law). MECHANISM CANDIDATE (named, unproven): beginDoorDemoLock (d_ext_npc_mount.cpp:9168 — alink changeOriginalDemo + DEMO_DOOR_OPEN_e) with the arrival state machine stalling before endDoorDemoLock — pollArrival's early returns (:1539 stage mismatch, :1542 isEnableNextStage) are the suspect stalls; a TP round trip restores control because re-init clears alink's demo mode. | FILES: src/d/d_ext_npc_doors.cpp (probe at №90/№89 sites), src/d/d_ext_npc_mount.cpp (lock-state accessor if needed) | DONOR: none — the door demo lock is receiver bridge machinery (mount-era); DN-10 note recorded: the native target is the donor's own door-open staff handling, owed with the door-lane retirement | VERIFY: instrument gates UNKNOWN-clean; probe registry NEVER-PUSH-STRIP-SET | tale §890
- [ ] Housing/Engine | DATA+CODE (small, rides with the probe): tale §890 — pollArrival early-return LOUD-once receipts (the two silent `return`s at d_ext_npc_doors.cpp:1539-1544 gain one-shot logs naming which gate held the state machine and for how many frames) — turns the candidate stalls into evidence in the same run that carries the probe. | FILES: src/d/d_ext_npc_doors.cpp | DONOR: n/a (bridge instrumentation) | VERIFY: instrument-class | tale §890

## LANDED (verified, built, kept for the receipt)

- [x] **§843 TUBOHAHEN/shatter donor-id fix** — LANDED + BUILT (exe 23:40 — I first wrote 23:47; disk says 23:40, corrected).
  **GATE PASSED:** offline JPAC1-00 walk of the staged `common.jpc` using the receiver's OWN
  parser field (`ww_jpa.cpp:59` — `resId = be16(ptcl + 0x18)`), no `gclib` needed. Header declared
  **193** emitter resources, **193** JEFF blocks parsed (count asserted before trusting the walk).
  **All five donor ids PRESENT** — 0x0017 TUBOHAHEN · 0x0018 TUBOKONAGONA · 0x03E5 TR_HAHEN_A ·
  0x03E7 TR_HAHEN_C · 0x03E8 DOKURO00. Control 0x0031 (windline, §192) PRESENT.
  **EDITS:** `d_a_ww_tsubo.cpp` — donor-value defines + 5 call sites swapped; `d_particle.cpp` —
  five `sWwCommon` router rows. **Pinecone (BOKKURI_MATSU00 0x816A) deliberately NOT swapped:**
  the 0x8000 bit is the SCENE-pack discriminator, so it lives in a per-stage Pscene jpc and the WW
  scene-bank serve is a separate unported surface — left OWED with a source note rather than
  half-fixed.
  **HOW THE HOLD CLEARED:** the user pointed out a parser likely existed. It was not in Foundry's
  tools (only `jpc_crosscheck.py`, which needs `gclib`) — it was in the RECEIVER, the §233 native
  WW JPAC1 parser. Reading the project's own code beat installing a dependency.

## HELD (failed verification — reason recorded)

- [HELD] History/Engine | **`d_a_obj_lpalm`** (Oyashi palm, planner rank 4, 10 Outset
  placements) | **REASON: NOT VERBATIM — gate rule 0.** `decomp_status` two-axis:
  AXIS A `configure.py` = MATCHED, AXIS B = **1 `/* Nonmatching */` marker**, and the
  tool flags the DISAGREEMENT rather than adjudicating it. The marker sits on
  `daObjLpalm_c::_execute()` (donor `d_a_obj_lpalm.cpp:125`) — a fully WRITTEN
  wind-sway function, not a stub, so it is "not byte-exact" rather than "absent".
  **I passed it anyway as "GO with a declaration". The user struck that tier: not
  verbatim, not passed.** Pulled from `files.cmake` before any build ran.
  **SYSTEM CHECK (rule 0's second half): its dependencies `dKyw_get_wind_vec` /
  `dKyw_get_wind_pow` ARE present in the receiver (`d_kankyo_wether.h`) — so this
  actor reveals NO unported system.** Nothing for History to queue on that count;
  the block is the marker alone.
  **UNBLOCK:** the donor function reaches verbatim upstream, or History reconstructs
  it to byte-exactness by the ja1 method and re-queues.

_(otherwise none currently — §843's hold CLEARED when the receiver's own JPAC1 parser
supplied the field `gclib` was wanted for; see LANDED.)_

- [QUEUED] Housing/Engine | **V10-a emission point — ONE log-service call inside the
  plugin registry** (CALLS row 97, tale §851/§853, ferried §914). Instrument-class, not
  a port: emit a JSONL record at plugin-registry `register` / `resolve` / `unresolved`.
  **Scope, stated tightly so it cannot creep:** no receiver change, no new leg, no new
  TU — one call at an existing choke point in the registry we already own.
  **DONOR EVIDENCE: none, and deliberately none** — this is a receiver-side instrument
  on our own plugin layer, the same class as the §898 probes and §903b's `WwProbe903`.
  It ports nothing and must not be read as porting anything (DN-10 does not bite: there
  is no donor system here to port instead).
  **WHY IT IS WORTH A BUILD SLOT:** it is the unlock for live ported/linked/exercised
  status AND for the self-generating `unresolved` worklist — every later V10-b join is
  offline work on the JSONL this produces, so nothing downstream can start until one
  build carries it. **Rides the next build; does NOT justify a build of its own**, and
  it is instrument-class so it sits outside MIN-BATCH-5 (§848/§860), like the §898 set.
  **Registry: NEVER-PUSH-STRIP-SET** (probe/instrument, stripped before push).
  **NOT BUILT BY ME** (§839) — landing dark on the Integrator's gate.
  **CODE IS NOW ACTUALLY IN THE TREE (tale §918) — it was NOT when this row was
  first filed, and I told the Integrator twice that it was.** My error, theirs to
  catch, and they did (`find src include -newer <exe>` returned EMPTY). Landed at
  `src/d/ext_plugin/ww_profile_register.cpp` in `dWwProfileRegister_lookup` —
  the registry's own choke point: `register` once (row/pending/mismatch/enabled),
  `unresolved` once per PENDING row, `resolve` once per row deduped. **cl /Zs
  clean via compile_gate (TIER1 + TIER2).** `[V10a]` prefix, JSONL payload, all
  under `#if TARGET_PC`.
  **WHY `unresolved` = PENDING ROWS, not "index we do not own":** this function
  returns NULL for every TP index in the game — the common case by its own
  contract — so logging those is thousands of lines per run drowning the signal.
  A row transcribed without a bindable index is the thing genuinely owed, and it
  is exactly the self-generating worklist V10-b asked for.
  **RIDER (tale §922, Foundry-requested): JSONL key `relinquished` → `handed_over`.**
  IN THE TREE at `ww_profile_register.cpp:378-384`, **cl /Zs clean (TIER1+TIER2)** —
  stating that explicitly because the parent row's first filing said "landing dark"
  when no code existed, and I am not repeating it. One string literal; same counter,
  same meaning. The C variable `s_relinquished` keeps its name on purpose (renaming
  reaches the human `[WwProfile]` line and the selftest, neither of which feeds the
  join). **Reason: read cold, "33 of 33 relinquished" is the exact inverse of the
  truth, and V10-b's status board is where that must not be wrong.** Foundry follows
  on the same build; one consumer today, so it is one line now or a migration later.
  **RIDER 2 (tale §924, CALLS row 79 — BOOT MANIFEST, profile half).** IN THE TREE at
  `ww_profile_register.cpp`, **cl /Zs clean (TIER1+TIER2)** — stated explicitly for the
  same reason as rider 1. Emits `{"ev":"manifest","layer":"ww_profile","name":…,
  "resolved_index":<idx|-1>}` once per row at the existing deferred-report point,
  **pending rows included at -1**. **DONOR EVIDENCE: deliberately none** — receiver-side
  instrument on our own plugin layer, same class as V10-a and the §898 probes; DN-10 does
  not bite because there is no donor system here to port instead. Makes DECLARED a runtime
  fact alongside LINKED/EXERCISED and gives §491's containment ratchet a baseline the tree
  side has never produced. **ARC-ROSTER HALF NOT INCLUDED** — it lives in the disc/FST
  plugin, not here and not in `ww_room_loader.cpp` (verified: inert pass-through, no
  roster), and it needs Foundry's call on which roster is authoritative. Instrument-class,
  outside MIN-BATCH-5, NEVER-PUSH-STRIP-SET. Not built by me (§839).
