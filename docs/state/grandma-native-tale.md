# Grandma native tale — full-native port of the birthday tale.stb (§263)

**Decision (user, 2026-07-30):** "Full native now." Retire the mount stand-in's tale
machinery; native ba1 (`daNpc_Ba1_c`, §262, GREEN) drives the tale cutscene the
donor way. This doc is the phased worklist + the hard-stop map.

## Why the swap broke the tale (acceptance finding)
The tale.stb was **never** Grandma-ordered in the port — it was driven by the MOUNT
system: player enters `[tale_loft]` → `dExtNpcMount_pollRegionTriggers` orders
`TALE_DEMO` → `tale.stb` (Demo01.arc), binding demo-DOUBLE actors `d_act0/2/3` that
the stand-in supplied (`ensureTaleDemoDoublesReady`), plus §183–186 clothes put-on +
JMSG catalog. Flipping `OBJNAME("Ba1")` to native (§261) removed the stand-in the
machinery drove; native ba1 uses the donor trigger (`orderOtherEventId("tale_1")`),
which isn't wired to that storyboard. Region fires, Grandma slot empty → no cutscene.

## What native ba1 ALREADY has (verified in d_a_npc_ba1.cpp)
- `a_cut_tbl = {"ACTION","START_TALE1"}`; registers as event staff **"Ba1"** via
  `mEventCut.setActorInfo2("Ba1", this)` (create).
- Full cut dispatch: `privateCut` → `cut_init_START_TALE1`/`cut_move_START_TALE1`
  (STB beat driver) + `event_action`→`eMove_*` (MOV_POS/CHK_FAIRY/EYE_OFF_ZRO).
- Trigger: `wait_0()` sets `mOrderType=6` on `check_useFairyArea()` (name is an
  [INFERENCE]; it's the tale trigger-area check) → `eventOrder()` calls
  `fopAcM_orderOtherEventId(this, mEventIdTable[3]="tale_1", ...)`.
So the cut logic is DONE. The gap is the event infrastructure + binding + JMSG route.

## Hard-stop map (read before touching)
- **DN-1** — event-list slots are ROOM-TYPED; `getEventIdx` matches by player room
  (`roomNo == mEventList[type].roomNo()`). `tale_1/tale_2` MUST be registered with the
  correct room for the tale stage or `orderOtherEventId` silently no-ops.
- **DN-4** — the tale JMSG dialogue MUST render through the Shade-Watcher native path
  (`buildPages`/`mountPaginate`), NEVER the ALBW post-man box. The "cutscene excerpts"
  the user saw IS the DN-4 cropping symptom. The mount path already renders correctly
  (§183/§193) — the native path must reuse it.
- **DN-3** — no BDL parse at arc-mount (relevant only if a prop is re-added; we're
  retiring the doubles, so mostly moot — keep consume-time-only if a prop returns).

## Stage/room note (Foundry to confirm)
WW "LinkRM" content is HOSTED in the port stage **R_DL01** (npc/ext_bg1.ini:
`arc=LinkRM host_stage=R_DL01 population_stage=LinkRM`). The event_list to author is
the one the host stage reads at runtime; room typing must match the room ba1/player
occupy during the tale. Precedent: `awake` authored into F_DL01's event_list.dat by
`tools/ww_crew_restoration_skeleton/merge_event.py` (§152); `Ba1_Get_Itm` resolves today.

## STATUS (2026-07-30, §266)
- **Phase 1 — DONE (Foundry).** tale_1(idx3)/tale_2(idx4)/Use_Fairy(idx5)/Ganbaru(idx6)
  merged into R_DL01/STG_00.arc (№152 tool); DN-1 satisfied by target-selection (mRoomNo on
  the dEvDtBase_c container, not the record — Ba1_Get_Itm is the standing receipt). TALE_DEMO/
  TALE_DEMO2 mount rows untouched at idx 1/2 (Phase-5 targets).
- **Phase 2 — SATISFIED (no code change).** tale_1 staff shape = {Ba1,CAMERA,Link} == working
  Ba1_Get_Itm; native ba1's `setActorInfo2("Ba1")` binds unchanged; no OffsetPos ⇒ cast-origin
  reconcile moot (Foundry receipt).
- **Phase 3 — SATISFIED (no code change).** JMSG routing is a GLOBAL JStudio hook
  (`jstudio_tAdaptor_message::adaptor_do_MESSAGE` → `dExtWw_handleDemoMessage`, d_demo.cpp:114),
  fires for ANY storyboard regardless of orderer → the native tale.stb inherits the DN-4 path.
- **Phase 4 — PENDING RUNTIME.** STB SHAPE→Link clothes (daAlink setDemoData) is STB-driven; verify live.
- **Phase 5 — NOT STARTED (do last).** Retire mount TALE_DEMO trigger + doubles AFTER native confirmed.
- **§266 acceptance probes ADDED** to d_a_npc_ba1.cpp (event-resolve at create / trigger fire /
  order accept-reject) — tagged `[Ba1Tale] §266`, strip after acceptance. Green build.
- **OPEN RISK — coexistence conflict:** the mount TALE_DEMO region-trigger is still live (Phase 5)
  and native ba1 now also orders tale_1 → both target tale.stb. The §266 order-result probe (0=refused)
  + the existing mount log will reveal a conflict. If they collide, bring Phase-5's trigger-disable
  forward as a REVERSIBLE gate (the mount path is already broken — empty Grandma — so nothing working is lost).

## Phased worklist

- **Phase 1 — Event registration [FOUNDRY, critical path].** Author `tale_1`, `tale_2`
  (+ `Use_Fairy`, `Ba1_Get_Itm`, `Ganbaru` for completeness) into the tale host stage's
  event_list.dat via merge_event.py, room-typed (DN-1), each referencing its STB
  (tale.stb / tale_2.stb, Demo01.arc). Acceptance for this phase: `dComIfGp_evmng_getEventIdx("tale_1", 0xFF)`
  returns a valid idx from native ba1's create (line 270), and `orderOtherEventId` is accepted.
- **Phase 2 — Native cast binding [HISTORY].** Confirm the STB's Grandma staff/JStudio
  object name == "Ba1" (ba1 already `setActorInfo2("Ba1")`); add the demo `l_objectName`
  row if the STB positions her via a JStudio object track (Aryll/Ls1 recipe). Confirm
  Link + Aryll(Ls1) cast slots bind. Retire reliance on d_act0/2/3.
- **Phase 3 — JMSG native rendering [HISTORY, DN-4].** Route the tale.stb JMSG through
  `dExtWw_handleDemoMessage` → Shade-Watcher path (already the fixed path). Verify tale
  ids resolve to donor text via ww_dialogue_full.txt (real BMG indices, §183). NO post-man box.
- **Phase 4 — Clothes put-on [HISTORY, verify].** STB `ENABLE_SHAPE` → Link clothes via
  `daAlink_c::setDemoData` is STB-driven (Link-side), independent of the doubles — verify
  it fires under the native event. Confirm whether the clothes PROP (was `d_act0`=fuku_model)
  needs a JStudio object or is inherent to the STB.
- **Phase 5 — Retire the mount tale machinery [HISTORY, LAST].** Only after 1–4 verified:
  retire `pollRegionTriggers` TALE_DEMO arm + `ensureTaleDemoDoublesReady`/d_act0/2/3 +
  tale portions of `tickBa1GetEvent`. KEEP the §183/§193 JMSG catalog + Link clothes logic.
  Do this last so the working scene isn't lost mid-migration.

## Acceptance (step 6, unchanged)
Tale plays natively (ba1-ordered), Grandma speaks in full native boxes (no excerpt
crop — DN-4 signature), Link dons clothes, tale_1→tale_2 chains; probe-differ vs the
golden `tale` trace (docs/WW Linked/dolphin-captures-tale-trace-20260728.txt).

## §266 RUNTIME FINDINGS (native ba1 spawn debugging)
Chain of root causes found via symbolicated crashes + probes:
1. **Not spawning at all** — `npc_ba.ini` still routed to the HENNA0 stand-in
   (`socket=NPC_HENNA0`), not native. FIXED: `socket=BA`, `socket_arg=0` + actor_map [Ba1] arg=0.
2. **Crash on spawn** (`McaMorf::create` deref) — `create_Anm`/`create_itm_Mdl` used donor
   NUMERIC res-ids (0xA/7/9); the adapted Ba arc needs parse-at-consume BY FILENAME (DN-3).
   FIXED: `acquireModelData(mArcName,"ba.bdl")` + anim `"wait01.bck"`; item bundle deferred
   (Vfuku cross-arc, tracked).
3. **Invisible at loft placement** (torn down in CreateHeap-then-createInit) — **WW↔TP EVENT-BIT
   COLLISION.** ba1 reads its story flags via `dComIfGs_isEventBit(0x520/0x0001/0x2A20/0x608/…)`,
   but the TP-based port routes those to TP's event table where the indices are unrelated TP
   flags (0x520 SET → `init_BA1_0` gate fails → mCharType 0 loft Grandma tears down). Probe
   confirmed `bit520=1` on a fresh `awake` start. BRIDGED (§266): `init_BA1_0` forced to the
   birthday/pre-tale state so mCharType 0 spawns + self-triggers tale_1. **FULL FIX OWED:** a WW
   event-flag NAMESPACE so every WW story actor reads WW flags not TP's — ba1's dialogue/
   progression reads (0x608/0xE20/0x740/0x780/0x601/0x602/0x2A20/0x2A80) STILL hit TP's table
   (wrong-message risk). This affects ANY WW story-driven actor, not just ba1.
   - Also latent: actor_map [Ba1] `unique=1` dedupes to one placement; donor relies on 4
     variants (0/1/3/4) at 4 positions self-selecting by flag. Moot while the bridge forces mCharType 0.

## §266 MODEL MARATHON — RESOLVED (native ba1 now renders)
Native ba1 spawned but rendered "all over the place." Cleared, in order, via a multi-hypothesis
render probe (per the user's directive — stop one-per-build bisecting):
- Engine/arc/id-path INNOCENT (Foundry audit + getIDRes probe: ids resolve to the correct 38-entry
  mod arc; `create_Anm` crash was DN-3, already fixed).
- anm_prm_c ENDIANNESS fixed (packed s8 anmNum/btpNum read the wrong LE byte) — real bug, not the render one.
- Node callback + McaMorf ctor exonerated by bisect.
- **ROOT CAUSE: `_draw` was missing `mpMorf->modelCalc()`** (Recipe 2/13 — the Aryll/ls1 fix dropped for
  ba1). `calc()` does anim matrices; `modelCalc()` composes the RENDER/world matrices from the base
  transform. Without it the whole model draws at the world origin (0,0,0) — probe read every joint at
  (0,0,0). Added `modelCalc()` before the btp entry (zl1:2624 precedent). She renders.
- Lighting fixed: `settingTevStruct(0…)` → `TEV_TYPE_ACTOR` (was too bright vs other actors).

## §266 TALE FREEZE → FERRIED TO FOUNDRY (user ruling: STB cast-hang forensics)
Native ba1 triggers + orders `tale_1` (idx 771, accepted →1) + the demo STARTS (frame 0) — then HANGS:
the storyboard can't advance because its **visual cast never binds** (demo read-back `actor=NONE`, stuck
at frame 0 while the game runs frame 4800). The tale.stb positions its cast via **JStudio objects
`d_act0/2/3`** = the mount demo-doubles, which the §266 mount-trigger-disable removed. Native ba1 is the
`"Ba1"` event STAFF (cut logic), not the `d_act0` JStudio visual object → the STB waits forever. This is
the true Phase-2 cast binding. **Ferried to Foundry** for the STB-hang trace (probe-differ vs golden
tale trace) to map which cast objects bind and how native actors should replace the doubles. Trigger
left LIVE for repro; mount TALE_DEMO stays disabled.

## §274 REVT ALL-ROOM OVERLAP → REVERTED, FERRIED TO FOUNDRY (2026-07-30)
Foundry's §272 REVT bake (into `R_DL01/STG_00.arc`, +96 bytes) fixed the missing
storyboard trigger BUT introduced a worse regression: with the REVT present the
incoming door-open event (`Knob00` §27) **continues into R_DL01 as an all-room
event-demo** — the stage loaded **all 6 rooms at once** (overlapping interiors) and
re-decoded the arrival room 3×, exhausting the heap (`JKRExpHeap:245` OOM) before the
№91 latch, then rendering every interior on top of each other after it.

Root receipt: the ONLY diff between the working 21:52 build and the broken 22:51 build
was that +96-byte REVT chunk. Same door, same entry, same code.

- **Reverted** `STG_00.arc` → `pre-revt-bak` (md5 `68b40b2f65…`, byte-identical to the
  working single-room state). Interior enters clean; tale does NOT play (no REVT to
  order → freezes/does-nothing — accepted interim). Foundry's bake preserved as
  `STG_00.arc.revt-6room-bak`.
- **Engine hardening kept (inert with one room):** removed the harmful №86 playerInit
  reconcile (→ №90) and added the №91 async player-create latch in `d_stage.cpp` — both
  fix the multi-room player-duplication OOM that the all-room load exposed.
- **Full pitfall + architecture writeup:** [ww-interior-host-pitfalls.md](ww-interior-host-pitfalls.md)
  — includes the recurring **Aryll→Grandma render pitfall** (missing `mpMorf->modelCalc()`
  in `_draw`, Recipe 2/13), the single demo-arc-slot bottleneck, and the owed WW
  event-flag namespace.
- **FERRY → Foundry (bus §274):** re-bake the REVT **gated to the tale trigger only**
  (§273 pending-demo path), never reachable by the generic arrival/door event, staff +
  camera scoped to room 0 so the stage never flips to all-room mode.

**§275 RESOLVED — MULT trim (Foundry, 2026-07-31).** Staff-scoping alone did NOT stop the
overlap (chunk analysis: `MULT=6` in the working 1-room arc too; only delta was `REVT
none→2`). Root: REVT presence flips stage-ENTRY into event-mode, loading the full 6-room
MULT stitch group → pile-up, independent of any tale firing. Fix (1) "move the REVT out of
probe range" was structurally unavailable (TP forces REVT id == array index; `isStageEvent`
matches by `field_0x4`; a low-id door-camera probe hits id 0 by construction). Fix (2)
shipped: **`mult_trim.py` trimmed the host MULT group 6 → 1 (room 0 only)** — event-mode
can now only load/stitch room 0, so the pile-up is structurally impossible whatever fires.
Delivered arc = clean base + staff-scope + MULT-trim + REVT (ids 0/1); md5 `1b19479c…`,
MULT=1/REVT=2 re-verified on disk. My №90/№91 OOM protections stay live. Now at §273
acceptance.

## §278–280 native-order STB path (2026-07-31) — ROOT: tale_1 has no PACKAGE staff
On the clean RTBL arc, ba1 triggers + orders tale_1, but the demo hung at frame 0. History closed
two disabled-mount provisioning gaps on ba1's own order path — **§278** (make Demo01/tale.stb
resident + retarget the demo-arc name before ordering; the awake §48 pattern) and **§278b**
(pre-spawn the d_act0/2/3 doubles, §175). Both PROVEN satisfied in-log. Still hung.

A **10-hypothesis probe (§279)** at `dDemo_c::start`/`update` gave the definitive answer (H1):
`§279 START` fires only for the opening, NEVER for the tale; `§279 update … m_data_null=1 m_mode=0`
forever; **`[PACKAGE] PLAY` never logs**. `dDemo_c::start(tale.stb)` is called ONLY by an event's
**PACKAGE staff PLAY cut** (`d_event_data.cpp:1319`). **`tale_1` (staff {Ba1,CAMERA,Link}) has no
PACKAGE staff**, so the storyboard is never fed to the JStudio control. FERRIED §280: Foundry adds a
PACKAGE staff (FileName=tale.stb/tale_2.stb, Stage=LinkRM) to tale_1/tale_2 — then ba1's existing
order plays it, no code change. (Alt: re-wire ba1 to trigger TALE_DEMO/REVT which already has
PACKAGE.) §279 probe stays until a demo plays; strip after.

## Lane split
- FOUNDRY: Phase 1 (event registration tooling) + §274 gated REVT re-bake.
- HISTORY: Phases 2–5 + acceptance.
