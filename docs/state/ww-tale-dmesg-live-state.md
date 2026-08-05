# WW Grandma-tale — native dMesg + post-tale spawn: LIVE STATE (History handoff)

Handoff note so a fresh History instance (or this one) resumes cleanly. Durable context lives
in the bus (`docs/WW Linked/ww-bridge-tool-interconnected.md` §310–§316), the decode
(`docs/WW Linked/dmsg-native-decode.md`), and `grandma-native-tale.md`. This file is only the
*live thread* on top of those.

## Covenant reminder (a lapse happened — do not repeat)
We adhere to WW vanilla systems ONLY. No compensating/invented layout or reposition code.
If the port diverges from the donor, the fault is in our rendering/wiring — fix THAT, never
paper over it. (Lapse: I widened the dMesg textbox wrap bounds to hide a mid-word wrap — a
non-vanilla layout. REVERTED. The correct move was the diagnostic alone.)

## Native dMesg — DONE (all vanilla, in `src/d/d_ext_dmesg.cpp`)
Text off real `zel_00.bmg`; tags consumed; authored `0x0A` line-breaks + 4-line pagination;
colour; player-name; SE; metrics from a ported `g_msgHIO` + `setCommonData`; `d00` talk / `d09`
item box by `mTextboxType`; `clothes.bti` icon from `itemicon.arc`; arrow `arwAnime` +
dot `dotAnime` animations. Guard fix (§313 v2): `dExtDmesg_isBoxActive()` stops the №89 door
arrival-G-guard from force-ending a held tale box, without touching arrival-camera (that was the
Outset-camera regression from v1 — fixed).

## TWO diagnostics live in the current build — AWAITING one tale run's log
1. **§316 textbox metrics** (`d_ext_dmesg.cpp`, `setCommonData`): logs applied-vs-nominal font
   size, bounds width, font cell width. Question it answers: is the port rendering text WIDER
   than the donor at the same nominal 25 (→ the true cause of the mid-word wrap), or is bounds
   width wrong? Fix follows the data — a vanilla metric correction, NOT a wider box.
2. **§317 Link pos** (`d_ext_npc_mount.cpp`, `dExtWw_pollDemoMessage`): logs the REAL Link's
   `current.pos` + `demoActorID` + facing, sampled through the tale, at `demo END`, and for ~2s
   after. Target = Foundry §315 `(−289,375,83) @0x8000`.

## Post-tale spawn — CORRECTED diagnosis (agent's first map was wrong)
"Spawn behind the door" = Link ends at PLYR point 200 (the tale ENTRY re-entrance point,
`d_a_npc_ba1.cpp:1011` `setNextStage(...0xC8...)`), not the demo's final transform.
- Agent claimed Link isn't bound → WRONG. Log proves `§48 JSGFindObject actor='Link' → FOUND`:
  the real `daAlink` IS a demo actor, driven by `procCoToolDemo` (`d_a_alink_demo.inc:1548`,
  byte-identical to donor `daPy_lk_c::dProcTool`). The platform Link during the tale IS the real
  Link.
- So it is a TRAJECTORY question, which §317 resolves: (a) he reaches `(−289,375,83)` then a
  teardown snaps him back to point 200, OR (b) he's never taken fully there. Different vanilla
  fixes. Do NOT hardcode `(−289,375,83)` — the donor has NO respawn at tale end; it inherits the
  demo's final transform because the STB drives the real actor and `demoEnd`/`endProc`
  (`d_event.cpp:443/589`) don't reposition. The fix restores that inheritance, whatever breaks it.
- Foundry (§315) confirmed: code-side reposition = bridge; kit rebuild carries it verbatim. But
  user wants the tale proven END-TO-END now, before kit — so find the vanilla in-code cause.

## Key file:line map (from the mapping agent, verified against the log)
- Point-200 entry trigger: `src/d/actor/d_a_npc_ba1.cpp:1009–1011`
- Point→transform apply (spawn): `src/d/d_stage.cpp:1854–1897` (`dStage_playerInit`)
- Re-entry demo start: `src/d/actor/d_a_alink.cpp:5135–5178`
- Real-Link demo write-back (== donor): `src/d/actor/d_a_alink_demo.inc:1548–1568`; dispatch gate
  `:139` (`demoMode == DEMO_NEW_ANM0_e`)
- Teardown (no reposition): `src/d/d_event.cpp:443–460`, `:589–617`, `:1056–1069`
- Demo cast bind: `src/d/d_demo.cpp:1039–1106` (`JSGFindObject` → `fopAcM_searchFromName` →
  `appendActor` sets `demoActorID`)
- Donor inherit mechanism: `D:/XXXXXXX/WW DP/src/d/actor/d_a_player_dproc.inc:181–197`

## §317/§287 LOG RESULT (2026-08-01) — spawn root = tale never FINISHES
Ran the tale; the diagnostics answered both questions:
- **§316 text:** `setFont=25 realFontXY=25x25 boundsW=510 hbind=2 cellW=24`. Font IS the donor 25
  (applied). So the mid-word wrap is NOT a size bug — a subtler render-width issue (bounds 510,
  left-bound). Text deferred behind the spawn.
- **§317 spawn:** at `demo END` Link is at **exactly (−289,375,83) @0x8000** — the tale drives him
  to the correct donor final transform. He is displaced to the door area (−341,375,113) only
  AFTER, during a post-tale event churn.
- **§287 ROOT:** `finishCheck FAIL — mFlags[0]=9 NOT set`. The tale event never finishes because
  event flag 9 is never set → no clean teardown → it re-`PACKAGE PLAY`s + churns
  (`onTypeChange 41→40`) → that churn moves Link off the platform. So "spawn behind door" is a
  SYMPTOM of the tale not ending — the tracked teardown lead, now located.
- **Mechanism:** a cut sets its flag on completion — `dEvDtFlagSet(cut->getFlagId())`
  (`d_event_data.cpp:1787`), the event manager mirror at `d_event_manager.cpp:1308`. The port HAS
  this; the only OTHER setter is `forceFinish()` (:445, a bridge — DO NOT USE). So flag 9's cut
  simply never completes.
- **Candidate (strong):** Foundry §304 lists `d_act3 DATA_ID(9)` @frame 680 (give/handover) as a
  post-message beat. `d_act3` is a `WW_DEMO00` demo-double (routed at `d_demo.cpp:1069` §271) that
  does not fire that real cut's data-id beat → flag 9 (== the DATA_ID 9) never sets. This is the
  Phase-2 cast-binding gap (retire d_act doubles / bind the real clothes-prop cast), NOT a
  coordinate and NOT forceFinish.
- **Next:** confirm which staff/cut carries `flagId==9` (verify the d_act3 DATA_ID(9) hypothesis vs
  the donor event def + the §287 log), then make that beat fire the flag the vanilla way (bind the
  real cast so its cut completes, or make the WW_DEMO00 twin honor the DATA_ID flag beat). May
  ferry to Foundry if it is event-data (staff/flags) — §287's own note says "after Foundry
  completes tale_1's staff/flags."

## §318 RESULT + §306b FIX (2026-08-01) — the flag-9 blocker was my own §306b alias
§318 cut-owner probe named the stuck cut: `PACKAGE PLAY flagId=3 flagSet=0 next=7` (Ba1/CAMERA/Link
cuts progress/terminal). PACKAGE plays tale.stb and never ends → the chain never reaches the
downstream finish flag (9). WHY, from the log: `§306 dEvDt_Next_Stage → NEXT STAGE 'R_DL01'` +
`§306b tale exit alias LinkRM → host 'R_DL01'`. The tale staff's exit stage is **LinkRM = the tale's
OWN home**. My earlier §306b aliased LinkRM → the host and returned "next stage exists", so the
PACKAGE end-fork SKIPPED `dDemo_c::end()` (mode stuck at 2, PLAY cut never ended, flag 3/9 never set)
AND `setNextStage(R_DL01)` reloaded the stage → tale re-triggered off the point-200 spawn → churn →
Link shoved (−289,375,83)→(−341,375,113).

**FIX (donor-faithful, `d_event_data.cpp` §306b):** the donor's "exit to LinkRM while on LinkRM" is a
no-op — stay in place, demo ends, Link inherits the transform (§315). Since we host the tale on the
current stage, treat the `LinkRM` exit as **NO transition** (`stage = NULL` → `dEvDt_Next_Stage`
returns 0) → PACKAGE end-fork calls `dDemo_c::end()` → mode 0 → `cutEnd` → flag 3 sets → chain
advances → finish flag 9 sets → clean teardown, Link untouched at (−289,375,83). No reload, no churn,
no forceFinish, no coordinate. Built green; AWAITING verify run.

Verify on next run: `§306b … NO transition, clean end`; `§318 PACKAGE PLAY … flagSet=1` (then next
cut); `§287` finishCheck STOPS failing (or passes); `§317 demo END` Link stays at (−289,375,83) with
NO post-end drift to (−341,…). Then strip §317/§318/§282/§287/§316 + older probes at acceptance.

## FOUNDRY GOLDEN TRACE (bus §317, tale-golden-trace-run1.md) — oracle diff checklist
Foundry captured a LIVE DONOR run. It confirms our teardown direction and corrects two things
source-reading could not:
1. **Font is 23, not 25.** Retail is the SECOND DEMO_SELECT arg (0x17=23); 0x19=25 is the kiosk-demo
   value. Every donor box measured 23. APPLIED: `dExtDmesg_msgHIO_c::mFontSize = 23.0f`. This is the
   real cause of the mid-word wrap (a +2pt overrun), NOT the box width — the §316 widen was correctly
   reverted. (§316 diag confirmed 25 was applied cleanly; the value itself was wrong.)
2. **Suspend counter is SIGNED by design.** Fast dismissals fire the box-close release BEFORE the
   authored suspend arrives → counter −1 → the suspend cancels it to 0 → the tale never pauses. The
   port must NOT clamp the counter at 0. OUR BOX currently gates release on `susp > 0`
   (d_ext_dmesg.cpp update) — that is the non-donor clamp; remove it (always unsuspend(1) on final-page
   A/B) + verify JStudio TControl suspend/unsuspend does not clamp. NOT YET DONE (separable box-pacing
   change; deferred behind the teardown verify so it doesn't confound it).
Confirmed observed fact: box order + storyboard-clock frames match §304 byte-exact; page turns happen
while suspended (only final close releases); **teardown returns control IN PLACE at the STB end
transform (~−289,375,83), NO respawn, NO reload** → our behind-the-door spawn is DEFINITIVELY host-only
(what §306b + §319 fix). d09 item-box mid-tale still on the list.

**Six-point oracle diff (History's queue):** (1) font 23 ✅applied; (2) signed suspend counter ⬜;
(3) storyboard-clock sequencing (matches); (4) page-turn-while-held (matches); (5) in-place control
return (§306b+§319+§319b, verifying); (6) d09 item-box mid-tale ⬜.

## §320 — the fade-in blocker: port-added WAIT gate in cutEnd (DECOMP-CONFIRMED)
Full teardown chain traced: fade-in ← endProc ← mEventStatus 5 ← eventFlag 8 (dEvt_control_c::reset)
← finish flag ← PACKAGE cutEnd sets its flag ← ... and cutEnd was NO-OPPING. Root: the port's
`dEvent_manager_c::cutEnd` (d_event_manager.cpp:1294) had an ADDED
`if (dComIfGp_getEvent()->getMode() == dEvt_mode_WAIT_e) return;` gate. The DONOR's cutEnd
(d_event_manager.cpp:549) has NO such gate — it just `flagSet(cut->getFlagId())` after `staffId==-1`.
At tale end the PACKAGE PLAY case calls `cutEnd(PACKAGE)` right AFTER `dDemo_c::remove()` (§319), by
which point the event control has dropped to WAIT mode → the gate no-op'd → PACKAGE flag 3 never set →
`§318 PACKAGE PLAY flagSet=0` + `§287 finishCheck` fail + `§285 mEventStatus=1` forever, faded-out.
FIX: disabled the gate (`if (false && …)`), keeping the staffId/mCurrentEvId null-safety. Matches the
donor. **TESTED — did NOT resolve the fade-in.** Correct donor deviation to remove, but the tale still
hangs faded-out. So the WAIT-gate was not the (whole) blocker. See HISTORY-HANDOFF.md §3 for the open
leads (does cutEnd(PACKAGE) now set flag 3? finish flag downstream? mEventStatus path? mount-doubles vs
native ba1 path?). This is now a fresh-eyes problem — do not trust the prior chain as complete.

Prior end-path fixes that ALL had to land first: §306b (LinkRM exit = no transition, no reload),
§319 (dDemo_c::remove not end — full teardown), §319b (getCamera null-guard after remove), §279 probe
strip (crashed on freed demo). Font 23 (Foundry §317) also in. Signed-suspend-counter (Foundry #2) +
d09 mid-tale still queued.

## §321 — TEARDOWN ROOT CAUSE FOUND (2026-08-01, bus §321): baked REVT exit byte
The §320 test log, read correctly, resolves the stall — and shows §320 WORKED:
- After `mode=2` → §306b → `remove()`, §287 AND §318 went silent while §285 kept printing
  `mEventStatus=1 runEvt='TALE_DEMO'`. Since `getRunEventName()` requires a live START event,
  `finishCheck()` was still running every frame — silent because it now PASSES (flag 3 set by the
  un-gated cutEnd → chain advanced → finish flag 9 SET). §318/§287 sit on paths the finish branch
  skips.
- The finish branch (Sequencer, TYPE_STB/ZEV) then hits `if (mapdata->field_0x7 != 0xFF)
  → sceneChange(exitId)` and only does `closeProc + reset` when `exitId == -1`. The §273-baked
  REVT records for TALE_DEMO/TALE_DEMO2 in `R_DL01/stage.dzs` had **field_0x7 = 0x00 and
  field_0x9 = 0x00** (zero-filled) → TP read "take exit 0 at finish" → `sceneChange(0)` looped
  forever (no-op — `isEnableNextStage` stayed 0), event never closed, eventFlag 8 never set,
  `mEventStatus` stuck at 1, screen stayed faded out. §282 had printed `field_0x7=0x0` all along.
- DONOR PROOF: WW `mainProc` (WW DP d_event_manager.cpp:300) has NO exit fork — finish ==
  `closeProc` in place. TP's encoding of that is 0xFF = no exit.
- **PATCHED (data, not code):** both entries `f7/f9 0x00→0xff` in
  `model_replacements/WW-Crew-Restoration/files/res/Stage/R_DL01/STG_00.arc` (verified by
  re-dump; backup `STG_00.arc.pre-321-exitff-bak`). No rebuild — the §320 exe is correct.
- Foundry owes the bake-default fix (bus §321) so the kit rebuild carries 0xFF.

## §322 (2026-08-01, bus §322) — §321 run verdict + §306b REVERTED (donor teardown = same-stage reload)
The §321 run (log 12:36) PASSED mechanically (teardown completed, Link pinned at (−289,375,83),
control returned, tale_1 boxes byte-matched the golden trace) — but the screen never faded back
in; the user was playing behind black. Root: **tale.stb authors a 20f black fade-out at its own
end** (d_act3 channel-9 beat @680 — §304's "give/handover" reading was wrong; channel 9 = the
demo00 black-fade channel), and the donor restores the screen via the **exit transition's
wipe-in**: WW `dEvDt_Next_Stage` has NO same-stage no-op (WW DP d_event_data.cpp:14-53), so
`Stage='LinkRM', StartCode=0` fires a same-stage RELOAD (wipe 5) that IS the donor teardown —
event killed mid-flight (flag 9 never runs on this path), DEFAULT_START (= golden "evt 49") on
arrival, spawn 0 sits exactly at the STB end transform (−289,375,83 @0x8000; verified in donor
room.dzr AND already baked into R_DL01's room.dzr). §306b was an invented-premise deviation
(same class as §316) — REVERTED; §322 in `d_event_data.cpp` aliases LinkRM→host and lets the
transition fire. Churn guards that §306b was compensating for are both in place: per-frame
re-fire guard (§306 `!isEnableNextStage`) + spawn 0 carries no tale trigger (trigger = point 200
only). §319/§320/§321 remain correct for genuinely exit-less events.

## §322b (2026-08-01, bus §322b) — reload+wipe-in VERIFIED; arrival spawn corrected
§322 run 13:48: wipe-in worked (screen restored) but donor StartCode 0 resolved the host DOOR
spawn (host id 0 = port door convention, (−255,0,1125)) → Link fell into the void + the door
spawn's event byte fired KNOB_START → letterboxes stuck. PLYR decode: spawn id = byte 0x1D,
event index = byte 0x08 (this byte is how point 200 auto-fires TALE_DEMO). Fix: donor talk-spot
(id 0x00, (−290,375,85) @0x8000, evt 0xff — the golden return transform) baked into R_DL01
room.dzr as point 0xCB (backup R00_00.arc.pre-322b-talkspot-bak) + §322b maps donor start 0 →
0xCB inside the LinkRM alias. Built 13:55, caches wiped.

## ✅ CLOSED (2026-08-01): USER-CONFIRMED CLEAN on the §322b build
Tale runs end-to-end donor-clean: nine boxes at golden frames → authored fade-out → same-stage
reload → wipe-in → control at the talk spot, no letterboxes, no churn. Generalized porting
playbook ferried to Foundry: **bus §323** (lifecycle, teardown archetypes A/B, data checklist,
failure-mode catalog, method lessons). R_DL01 kit rebuild is GREEN-LIT.

## §324 (2026-08-01, bus §324) — native dMesg GENERALIZED to live NPC talk; BUILT, awaiting playtest
The §308 box now serves ALL WW NPC dialogue in Native style (toggle preserved, sits after
"Lies of Link HUD" in settings). Mechanism: the TP dMsgObject keeps running the lifecycle every
WW actor polls (talk() → MSG_DISPLAYED/BOX_CLOSED), but its single input funnel `isSend()` is
gated while the native box presents (native owns A/B + WW pagination; one synthetic "A" fed on
final-page dismissal; 2-frame consume-once latch) and its draw is suppressed (beside the rental
suppression). Text native-first: zel_00.bmg by the actor's own msg id, catalog fallback. Chains
via the 2-arg CONTINUE overload (10-frame window). Debts: selects hand back to the TP box
(visible skin switch); item-icon map beyond clothes.bti. Files: d_ext_dmesg.h/.cpp (§324 talk
mode), d_msg_object.cpp (isSend gate + draw gate), d_ext_npc_mount.cpp (injectTalkText native
path + injectTalkChain), f_op_msg_mng.cpp (chain hook), ui/settings.cpp (help text). Built
green (full pass, exe 14:3x), caches wiped. Playtest: Grandma + postbox talk in both styles.

## §325 (2026-08-01) — Foundry ferry ACKED: Grandma's-room actor ports queued
Lamp (d_a_lamp.cpp, arc STAGED → History next) → mshokki (MPot/MOsara/MKoppu, ONE actor prm
0/1/2; arc NOT staged → Foundry ask) → SPitem (d_a_spc_item01.cpp, quest-load-bearing §312
LOOK_SHIELD chain; arc NOT staged → Foundry ask). KNOB00 = USER ruling first. swood = Housing.
All donors fully matched. §324 native talk verified on Grandma; exterior NPC talk still needs one
check (dmesg residency may be interior-gated). Details bus §325.

## §326 (2026-08-01) — USER rulings: KNOB00 always-native; toggle = all WW NPCs both worlds
KNOB00 → native d_a_knob00 port queued (port door wiring = named bridge until then). Toggle
coverage audited: R_DL* + F_DL* already fully wired for §324 (residency/inject/openTalk share
the isWwHostStage predicate); F_SP115 is the only staged-stage outside the predicate — widen
only on evidence of a WW talker there. One exterior verification talk still owed. Bus §326.

## Whose turn (2026-08-01, post-close)
- FOUNDRY: bus §323 §2 checklist into the kit bake (REVT exit defaults, PLYR closure incl.
  teardown spawns + id-collision policy, ALL/dummy staff, channel-9 timeline correction);
  kit rebuild green-lit.
- HISTORY (next session): probe strip (HANDOFF §7 + stale §50 truncation warning), signed
  suspend counter (Foundry #2), d09 mid-tale box (#6), tale_2 verify pass, then the user's
  message-system notes.
- USER: relay §323 to Foundry.
- HISTORY: on PASS — strip probes (HANDOFF §7; §50's "TRUNCATION" heuristic is stale, remove it
  too — donor END @694 ≈ port fnm 709), then signed-suspend-counter (#2), d09 item box (#6), the
  tale_2 finish (same §322 path — its staff carries the same LinkRM/0 exit), then the user's
  message-system notes.
- FOUNDRY: §321 REVT bake-default action stands; note §304's DATA_ID(9) = fade channel
  reinterpretation for tale_2/Ba1_Get_Itm timelines; kit rebuild downstream of the tale proving.

## §327 ONE-PASS PORT PLAN (user order 2026-08-01): Lamp + mshokki + SPitem + knob00
Write ALL FOUR ports now; unstaged arcs (Mshokki, SPitem's) just idle until Foundry stages them.
Per actor (cookbook recipes 1-9): donor include/src from D:/XXXXXXX/WW DP/{include/d/actor,src/d/actor}/
→ port include/d/actor/ + src/d/actor/ (dolzel.h not dolzel_rel.h; res/Object/<Arc>.h donor res
ids → use residmap.csv ids or raw indices w/ comment; §-tagged block comments; WW particle ids
via dPa_name — if unresolved at runtime that's the §205/§206 Foundry particle surface, log not
bridge; JA_SE_* WW sound ids: follow toripost/ls1 precedent) → register: f_pc_name.h next slot
(0x32C+), f_pc_profile_lst.h/.cpp (extern g_profile_*), files.cmake, d_stage.cpp l_objectName
rows (donor rows: Lamp→? check donor; MPot/MOsara/MKoppu→Obj_Mshokki 0/1/2 @WW d_stage.cpp:1131;
SPitem→spc_item01?; KNOB00→knob00). ALSO: remove/park the population's Lamp→NPC_LAMP stand-in
mapping (d_ext_npc_population.cpp "spawn #1 name=Lamp proc=NPC_LAMP") so the native row wins —
that mapping is the bridge being retired. knob00 (944 ln donor): native door per §326 ruling —
integrate with care around the port door wiring (leave port doors working on non-WW paths; knob00
owns WW host doors); donor lamp_class header READ (34 ln, simple, above fopAc_ac_c + dPa follow
callback + LIGHT_INFLUENCE + dCcD sphere). Donor lamp uses TEV_TYPE_BG0, param 0 = static.
Build via build_run.bat + cache wipe; DON'T commit (user commits). Bus entry §327 on completion.

## §327/§328 DONE (2026-08-01 17:04): one-pass ports ALL LANDED, final build GREEN
Lamp/mshokki/SPitem live (slots 0x32C-E) + d_door layer live + knob00 fully ported behind
DUSK_WW_KNOB00_NATIVE (default 0 — §27 stand-in byte-identical until seam wiring). /O2 verified
after both CMake re-runs; caches wiped. Foundry staging queue: Mshokki/Vshin/Key/Hkyo arcs.
Known gap: loft lamp absent until census→OBJNAME spawn wiring (stand-in parked). Full detail +
gate-flip seam list: bus §327/§328.

## §329 DONE (2026-08-01 17:09): NATIVE knob00 ON — build green, caches wiped
Gate flipped + 5 seams wired (stamp surface, warp hook at openEnd, event-arc donor-pure w/
DoorK10 runtime watch, double-spawn watch, port BG mover). Kill switch: DUSK_WW_KNOB00_NATIVE=0.
Playtest = full overview: door open/warp/collision + tale + native talk + lamp gap + FPS.

## §329b (2026-08-01): ALL ISLAND DOORS DISAPPEARED on the §329 build — root + fix
Root: native M_arcname donor spelling "knob" vs R2 overlay mount key 'Knob' — the overlay OPEN is
case-sensitive (post-load lookups are stricmp) → resLoad error-looped SILENTLY → create died →
§27 poll respawned forever (log signature: repeating fpcBs_Create, zero [Knob00] lines). Fix:
load by staged spelling "Knob" (donor spelling noted in comment) + two §329b warns so create
failures can never be silent again (resLoad ERROR + entrySolidHeap/shape). Next run
discriminates: resLoad-ERROR warn = arc content; entrySolidHeap warn = donor param decode; no
warns + doors = fixed; no warns + no doors = spawn path.

## §329c (2026-08-01 17:31): doors alive-but-INVISIBLE — donor room-bit params synthesized
Second door incident, new signature: spawns fired ONCE (creates now succeed post-§329b), zero
[Knob00] lines → alive but never drawn. Root: donor packs front/back ROOM IDS into home.angle.x
(6 bits each, d_door.cpp getF/BRoomNo); §27 spawns leave angle.x = pitch 0 → rooms 0/0 →
adjoinPlayer() false (player in room 44) → drawCheck 0. Fix: for network-stamped doors,
synthesize the DONOR'S OWN roomless sentinel F=B=0x3F (home.angle.x=0x0FFF) — donor encoding,
not invented layout; visual pitch unaffected (calcMtx is Y-rot only); checkExecute passes via
frontCheckOld + mRoomNo2 spawn-time stay latch. Real per-door donor params (shape/rooms/events)
arrive with the kit's DZR door rows — tracked. Built 17:31, caches wiped. Watch items: door
EVENT binding at first use (DoorK10 seam 3), mRoomNo2 if doors ever spawn from another room.

## §329d (2026-08-01 17:43): interior CRASH = Ba1 duplication loop from the Lamp park — UN-PARKED
Crash root (log 17:36, no crash frame needed — the loop is the log): census:Lamp was the ONLY
countLiveCensus-countable actor in Grandma's room (counter = census-src mounts of the
HENNA0/MK/P2/KDK proc set); §327's park → live=0 forever → №94 self-heal re-ran the population
EVERY cycle → new NPC_BA1 per cycle (pids 550,551,...) → memory death. Fix: park gate flipped
back (#if 0) with re-park preconditions documented (native census→OBJNAME lamp spawn + liveness
decoupling). Stand-in lamp RETURNS (visible again) until then. Built 17:43, caches wiped.
STILL OPEN — door-use anim: native knob00 opens/warps but TP Link doesn't play the door-open
walk animation (the stand-in drove Link via TP's DoorK10/SHUTTER_DOOR event; the donor's Link
staff cuts are WW-anim-vocabulary). Seam-3 decision next session: bind the native door's demo to
the TP-side door event (receptor-allows line) vs port WW's Link door cuts.

## §334 IN PROGRESS: WW→TP collision-attribute table (top work item; user PROCEED given)
Receipts: bus §332 (bit layouts: WW attCode=inf1 bits16-20 via atr_conv, WW special=bits12-15;
TP att0=12-15, att1=16-18 (1-4=SINK), groundCode=19-23; otble inf1=0x000200FF proof).
Ruling: bus §333 + user follow-up — true-equivalent materials → TP MATERIAL standable;
no-clean-equivalent (VOID/boundary class) → TP slip/no-stand class; ambiguous → ASK user
(user expects few/none; per-code specific edits acceptable if WW-accessible surfaces turn out
TP-inaccessible). DONE: donor atr_conv READ (WW DP d_bg_s.cpp:193): 0=NORMAL 1=DIRT 2=WOOD
3=STONE 4=GRASS 5=GIANT_FLOWER 6=LAVA 7=DIRT 8=VOID 9=DAMAGE A=CARPET B=SAND C/D/E=WOOD F=ICE
10=WOOD 11=METAL 12=DIRT 13=WATER 14=METAL 15=FREEZE 16=ELECTRICITY 17=WATERFALL 18=METAL
19=CARPET 1A=WOOD 1B-1F=NORMAL. NEXT: (1) TP vocabulary — groundCode value meanings (alink
checks 6=hang-class? 8; GetMtrlSndId footstep table) + att0/att1 semantics + THE SLIP-CLASS
encoding (how TP marks near-OOB slippable polys — find in d_bg/alink slip checks); (2) write
dExtWw_repackDzbAttributes(cBgD_t*) — idempotent (marker or repack-once registry), applied at
WW dzb consumption: knob00 CreateHeap door.dzb + mount :3357 collision loads; staged arcs stay
byte-verbatim; (3) full 0x20-row table §334-commented with per-row receipt; (4) build, report
table to user for the acceptance ruling (= DN-1 furniture activation too). THEN: tsubo port
(d_a_tsubo, 19 Orca placements, Actor-Kit driver). Also queued: WW-demo-driven Link door anim
(§330b ruling), census→OBJNAME + liveness decouple, DoorK10 watch, probe strip.

## §334b (2026-08-01 19:01): half-fix root = otble's OWN Set site was unwired — wired, built green
User report: lava anim on first contact, then recovery + clean stand. Root: d_a_obj_otble.cpp
Sets otble.dzb/otble_l.dzb itself (Okmono arc) — the one WW dzb consumer outside the §334 wire
set (repo-wide sweep confirms: all other unwired Set sites are TP-native actors, correctly NOT
repacked). Wired with the same pre-Set repack. Recovery-afterward explained: Set stores the dzb
by pointer + attribute reads are live; TP's deep-sink limit popped Link out. Expected now: no
lava anim at all; log shows `§334 dzb attribute repack 'otble.dzb': N standable, 0 slip`.
Exe 19:01, caches wiped. Awaiting user playtest → acceptance ruling (= DN-1 furniture).

## §334c (2026-08-01 19:51): "worse — always lava" run REFRAMED by receipts; probe armed
19:21 run receipts: all wired sites repacked (room.dzb 82st/3slip + 30st/0slip, akabe, door) —
but NO otble.dzb line: the otble ACTOR never spawned (Okmono mounted, NPC_OTBLE provider exists,
zero otble create). So the user's un-walkable "table" surface is NOT otble collision. Open
hypotheses: (a) the surface is one of room.dzb's 3 SLIP tris (WW hazard codes — possibly
legitimate donor fireplace/water, possibly wrong); (b) an UNWIRED boundary (native room/stage
collision path); (c) something reading pre-§334 cached values. Armed §334c ground-attr probe
(change-only, every 15f, in pollDemoMessage): logs att0/att1/groundCode + owning proc under
Link. One run standing on the bad spot names the surface + boundary. Built 19:51, caches wiped.

## §335 DIAGNOSIS (2026-08-01, log 19:53): mid-tale control return = TALE DOUBLE-START, not the trigger
User report: tale fired "at ground floor", ended mid-get-item, control returned. Log facts:
1. TRIGGER IS DONOR-CORRECT: door entry (playerInit point=0, ground floor y=0) → ba1's donor
   order fires (tale_1, §284b) → START_TALE1 warps to loft (SECOND playerInit point=200) →
   TALE_DEMO runs at the loft. "Triggered at ground floor" is the donor's own design (order
   then warp) — NOT the region trigger (log: "RegionTrig 0 trigger(s) ready").
2. THE REAL DEFECT — DOUBLE/TRIPLE DEMO START: (a) §317 "demo END frame 0" + §50 TRUNCATION
   right after point-200 arrival = an ABORTED first TALE_DEMO instance; (b) a second instance
   runs boxes 539(fnm20)/540(fnm73); (c) "demo END frame 117" with Link at (-341,375,113) and
   demoActorID=1 — that instance DIES mid-stream (= the user's control return); (d) box 3095
   still shows at fnm=144 — a THIRD instance/continuation. §284b START_TALE1 kept looping
   through it all (tale_1 order event never closed). This is the §278b/§175 double-start class
   ("old bug thought rid of") — the point-200 trigger and/or ba1's order fired more than once,
   and overlapping TALE_DEMO instances truncate each other (one event at a time architecture).
3. Open leads for next session: (i) why the frame-0 abort (first instance starts before Demo01
   resident / cast pre-spawn ready — §278b gate); (ii) does the point-200 spawn-param trigger
   re-fire after the aborted instance (once-latch missing?); (iii) does tale_1's order event
   survive the point-200 reload when it shouldn't (§284b loop); (iv) demoActorID 0 vs 1 swap
   between instances. Instrument: one probe logging every TALE_DEMO order/start/END with
   instance counter + who ordered (spawn-param vs ba1 vs re-fire) + demo mode transitions.
NOTE: §334 attribute table CONFIRMED WORKING this run (table walkable, no lava). §334c probe
live. Do NOT ship fixes for the double-start without the (i)-(iv) probe run.

## §336 PROBES BUILT (2026-08-01 20:09) — trigger-gate geometry + demo instance chain
User corrections to §335: (1) the ground-floor fire is NOT donor design — donor never fires
from the 1st floor → the §271 check_useFairyArea reconstruction (3D dist < mUseFairyDist1=500,
byte-faithful donor value) is MISSING A DONOR GATE (500 3D radius reaches through the floor:
Grandma y=375, ground y≈0, dist≈380-400 → fires). Donor ba1 = Nonmatching stubs; the gate must
come from Foundry decode receipts (candidates: mUseFairyDist0 second threshold [currently 0.0 —
suspicious!], Y-band, room-part/BG check, angle gate). (2) The double-start = wrongful ground
fire + re-fire on point-200 arrival. PROBES (strip at §336 acceptance):
- §336a d_a_npc_ba1 check_useFairyArea: dist/thr/thr0/dY/link-pos/ba1-pos/tmp310/fire, every
  30f within 1.5×radius + always on fire.
- §336b d_demo.cpp dDemo_c::start: instance # + runEvt + prior mode + Link pos on EVERY start.
Run protocol: fresh entry through the front door → walk the ground floor near/below Grandma →
ladder → loft → let the tale run. Log then yields: exact wrongful-fire geometry (what gate
blocks it), the full order→start→END instance chain, and the frame-117 death owner.
NOTE mUseFairyDist0=0.0 in the byte-faithful table — a ZERO second threshold strongly suggests
the donor uses dist0 differently (e.g., must be BEYOND dist0? a disabled band?) or the table
mapping drifted a slot — check the §260 byte-faithful mapping against the donor float pool.

## §337 PROBE RUN RESULTS (2026-08-01, log 20:11) — three answers, one new boundary named
1. §336a WRONGFUL FIRE RECEIPTED: dist=498 thr=500 dY=-359 link=(124,0,86) ba1=(-225,375,-18)
   — Link on the ground floor, 359 BELOW Grandma, fires at 498<500. The donor gate the §271
   reconstruction lacks is now a precise Foundry decode ask: what blocks the donor fire at
   dY≈-360 (Y-band? floor/room-part check? mUseFairyDist0 (=0!) used as a band?).
2. §336b CHAIN CLEAN THIS RUN: single TALE_DEMO instance, started at the loft (-289,375,83)
   after the warp → "cutscene worked perfectly". The §335 double-start is timing-dependent
   (frame-0 abort did not recur). "Control returned a little" = the order→warp→arrival gap.
3. §334c NAMES THE LAVA-SINK BOUNDARY: ground under Link reads att1=2/3 at many interior AND
   exterior positions, owner proc=0x242 (=578 = the ROOM BG actor, cf. §63 bgName=578) — the
   KIT-BAKED ROOM collision registers through the NATIVE room-BG path (d_a_bg/dStage), which is
   NOT a §334-wired boundary → un-repacked WW bits. Run-to-run inconsistency = overlapping
   colliders (repacked mount piece vs un-repacked room BG) racing for the ground poly. ALSO
   decoded: TP sink needs att0==3 (alink sinkProc gate) — WW STONE(3) polys are the actual sink
   spots (att0=3 receipts at (-135,474,373) + (39,69,23)); att1=2-only spots don't sink.
FIX DIRECTION (next session, NOT blind): wire dExtWw_repackDzbAttributes at the room-BG
consumption for KIT-BAKED WW rooms ONLY — repacking TP-template room dzbs would corrupt TP
attributes, so the WW-room identification must come from Foundry's bake manifest (which room
arcs are WW-baked, both stages — exterior y=0 spots also read WW bits). FOUNDRY ASKS: (a) the
donor ba1 gate decode (item 1 receipts attached), (b) the WW-baked room list / marker for (3).

## §338 BUILT (2026-08-01 21:40): donor cylinder trigger + §334d write-verify probe
1. TRIGGER FIXED DONOR-TRUE (Foundry REL disassembly): check_useFairyArea is now a CYLINDER —
   XZ-only distance < mUseFairyDist1 (500) AND separate |dY| < 100.0 band (both rel float-pool
   confirmed). Blocks the ground-floor fire (dY=359) AND the old mid-ladder case (dY≈150).
   The §271 {20,-16,0} offset removed (absent from the disassembly; origin flagged). The zeroed
   mUseFairyDist0 = the donor's 0.0 Y-replacement rodata misread as a threshold — NOT a param.
2. LAVA-SINK INVESTIGATION REDIRECTED: §337's owner proc 0x242 = fpcNm_NPC_KDK_e = a MOUNT
   piece (NOT d_a_bg) — Foundry's path-keyed claim holds, all mount Set sites are wired and
   receipted. New prime suspect: the repack's WRITE may not land — first-ever code writing
   through OFFSET_PTR+BE(u32) (reads correct → receipts count fine; store may no-op/mis-swap).
   §334d write-verify added: logs ti[0] post-store word per repack. Next run discriminates:
   post att1 nonzero (beyond slip) = WRITE FAILURE → re-implement store via raw byte write
   honoring the BE layout; post clean = write OK → hunt an unwired KDK-mount Set elsewhere.
3. Foundry's ww_dzb_roster.csv = audit/negative-control; path-keyed wire confirmed sufficient
   pending the §334d verdict. Probe strips (§334c/§334d/§336a/§336b) ride the acceptance.
Exe 21:40, caches wiped. USER: one run — enter house (trigger must NOT fire downstairs now),
loft → tale, stand on a former sink spot; log answers via §334d + §334c + §336a/b.

## §338b/§334e (2026-08-01 21:54): trigger CONFIRMED GOOD; sink narrowed to one un-receipted dzb
User run on §338: trigger donor-clean (no downstairs fire, §336a count=1 legit), most surfaces
mountable; ONE thin loft railing still sinks. §334d verdict: WRITES LAND (post-store words
clean) — the repack works wherever it runs. Cross-check vs Foundry's ww_dzb_roster.csv (11 BG
manifests, ALL collision='room.dzb', from DIFFERENT WW arcs: Outset/LinkRM/LinkUG/Ojhous2R1/
Ojhous2/Omasao/Onobuta/Omori/Pjavdou/A_mori/Cave09): this run receipted only TWO distinct
room.dzb (82-tri + 30-tri) — R_DL01 hosts SIX; the ground-floor/railing piece (prime suspect:
ext_bg2/LinkUG) shows RAW WW bits (att0=3/att1=2 = un-repacked STONE) ⇒ its Set never passed a
wired site this session (all 5 wire sites verified present; how it registered un-Set/un-receipted
is THE open question — candidates: cached-BgW reuse re-Regist without re-Set, or a create branch
that skips useBgHeapInit). §334e probe added: §334c now logs the owning mount's manifest ARC +
collision member — the next run binds the railing poly to its exact roster row + receipt state.
Exe 21:54, caches wiped. USER: stand on the railing once; the §334c line with arc='...' closes it.

## §339 DIAGNOSIS (2026-08-01, log 21:57): the "staying-power" mid-tale break = THE EXIT DOOR
§336b: SINGLE tale instance, started at the loft ✓ (cylinder trigger clean, fire at dY=-91 ✓).
The break: §285 sequence TALE_DEMO → item box 3095 shown → runEvt='KNOB_START' (status 1→5,
isEnableNextStage=1 = the §329 door warp ARMED) → 'DEFAULT_TALK'. The interior exit knob
(door:exit:linkrm) sits ~55u from the tale platform ((-341,375,113) vs (-289,375,83)); the
§329c always-adjoin sentinel (F=B=0x3F) keeps its checkExecute/CANDOOR armed everywhere, and the
NATIVE knob00 lacks the in-event arming gate — so the A dismissing the item box (or the instant
after) fed the door's armed command → door event seized the slot → tale evicted → control
returned mid-get-item. Explains "staying power": any tale run where Link/player A-presses within
the door's range breaks the same way; runs where the box dismissed cleanly worked.
FIX DIRECTION (next session): gate the native door's event ARMING while an event runs —
DECOMP-FIRST the donor idiom first: how donor knob00/TP knob20 gate eventInfo door commands
during demos (likely eventInfo.onCondition skipped under event_runCheck, or checkCommandDoor
naturally gated by the event manager — find why the port's event system granted the door command
mid-tale at all), and whether §329c's 0x3F sentinel over-widens checkExecute (donor rooms would
gate the door OFF the loft — the kit's real per-door room params retire the sentinel). Also
§334e railing bind still owed (user hit this bug before reaching the railing).

## §340 ARBITRATION VERDICT (2026-08-01): gate EXISTS (d_event.cpp:1111 mEventStatus==0 &&
entry()) — no preemption possible. CORRECTED CHAIN: the tale event DIED at the item-box beat
(fnm~144) FIRST; the player's dismissal A near the door (55u — Link's platform is in door-use
range) then dispatched the queued door order into the mEventStatus==0 gap → KNOB_START. The
§339 "door evicts tale" framing inverts cause: root = WHY the tale event terminates at the
get-item beat (old §335 truncation family — mode/finish path dying while the box is up), with
the door merely exposing it. The KNOB_START line's isEnableNextStage=1 needs attribution
(§322 tale exit firing early vs the door's §329 warp). NEXT PROBE (one build): log EVERY
mEventStatus transition (old→new + runEvt + dDemo mode + m_frameNoMsg) — the frame the tale
leaves status 1 mid-box names its killer directly. THEN the no-guards plan (§339) stands:
donor room params (kit) retire the sentinel; no arbitration fix needed (gate already donor-true).

## §341 PROBE SET BUILT (2026-08-01 22:18) — 10-hypothesis instrumentation for the mid-box tale death
Per §340: the tale leaves status 1 at the get-item beat; the door merely fills the vacancy
(arbitration gate donor-true, d_event.cpp:1111). One build, three sites, ten discriminators:
- §341a (d_event.cpp Step): EVERY mEventStatus transition — old→new, runEvt, demo mode, f/fnm,
  susp, isEnableNextStage (H2 early §322 exit), evFlag8 (H5), msg-kill presence (H8).
- §341b (d_demo.cpp update): the forward()==0 STALL write → mode 2, with f/fnm/susp (H3
  truncation + H7 suspend desync visible in the same line).
- §341c (d_demo.cpp end()/remove() entry): H4 teardown-caller attribution with frame state.
- H1 (finish flags 3/9 early): read from the live §318/§287 prints adjacent to the transition.
- H6 (early cutEnd): §318 flagSet flip timing. H9 (probe interference): §334c/§324 lines
  interleave in the same log. H10 (what filled the gap): the §341a transition into the next
  event's status 1 names it.
Exe 22:18, caches wiped. USER: one tale run to the get-item beat (break or no break, the
transition lines around fnm≈144 answer). Then: root fix → railing §334e → THE BIG STRIP
(§334c/d/e, §336a/b, §341a/b/c + §323 queue).

## §342/§334f (2026-08-01 22:24): TALE RAN FULL (fnm 709 natural end!) + lava staying-power ROOT-FIXED
Run 22:19 verdicts (§341/§334e probes):
1. TALE COMPLETE: §341c remove() at f=822 fnm=709 = the donor natural end (golden ≈694+lead);
   ran past get-item. The §340 mid-box death did NOT recur this run — H1-H10 chain shows the
   §322 teardown flowing donor-true (TALE_DEMO→close→DEFAULT_START→NORMAL_COMEBACK, evFlag8 at
   the right beats). The user's brief end-of-tale control blip = the §322 reload gap
   (order→warp→arrival) — donor-adjacent; WATCH, not a defect yet. Mid-box killer NOT yet
   reproduced under §341 — probes STAY until one breaking run is captured.
2. LAVA STAYING-POWER ROOT (§334e receipts): bad polys bind to arc='LinkRM' room.dzb — the
   RECEIPTED piece. Mechanism: §322 tale reload purges+reloads the room arc; raw bytes land at
   the RECYCLED heap address; the pointer-keyed s_done registry said done → skip → raw WW bits
   post-reload (glitch "twice" = once per reload; J3D pointer-fix law biting via my own guard).
   FIX §334f: s_done REMOVED — repack runs at every Set, idempotent by construction (pass-1
   clears make pass-N no-ops; slip signature wwAtt==0+gnd==4+att0==0 preserved). Exe 22:24.
USER RUN NEXT: (a) tale once more (watch for the mid-box break — §341 armed), (b) lava spots
after the tale completes (post-reload = the §334f proof), (c) the railing.

## §343 RECLASSIFIED (user ruling 2026-08-01): mid-sequence control blips = DEFECT, not watch
Standing rule recorded: ANY player-controlled movement during ANY cutscene/sequence is a defect
(not vanilla). Applies to the §342 "reload gap" (order→warp→arrival) blip AND any §341-visible
status-0 window with the player live mid-sequence. Fix direction (donor-comparable): the donor
holds input across an event-ordered setNextStage — the whole ordered chain is control-locked
end-to-end (event mode / player proc lock persists into the transition; wipe engages before any
input frame). The port's §341a chain shows status 1→0 'NO DATA' windows between chain links —
each such window with Link live is the defect surface. Find where the donor keeps the lock
(dEvt mode across setNextStage / scene-change input lock timing) and restore it — no port-side
input-freeze bridge. §341 probes already capture the windows (status transitions + runEvt);
next breaking/blip run feeds the exact frames.

## §344 (2026-08-01 22:40): mid-box killer CORNERED to the change() grant; lava CONFIRMED FIXED
Run 22:28 (§341 armed): §334f CONFIRMED (user could not trigger lava). The break line captured:
§341a status 1→0 DIRECT (never via 5) runEvt='TALE_DEMO' demoMode=1(STILL PLAYING) f=118 fnm=98
susp=0 nextStage=0 evFlag8=0 — H2/H3/H4/H5/H7 ELIMINATED (§341b silent, §341c silent at that
frame). The ONLY direct 1→0 path in Step = the change() mechanism (d_event.cpp:619, order flag
0xE00 from mChangeActor) — an event-CHANGE order granted MID-TALE kills the event with the STB
mid-flight. §344 probe added at the grant: requester proc + flags + evType + runEvt. Exe 22:40.
NEXT RUN names the change requester → then the donor-compare: how the donor gates change()
during a running demo (or which actor wrongly submits 0xE00 mid-tale — mChangeActor identity is
part of the question: who set themselves as change actor). NOTE fnm=98 sits between boxes
540(@73) and 3095(@144) — near the STB's Link SHAPE/clothes beat; the demoActorID=1 +
angY=0x9194 drift in the §317 END line may point at Link's demo-binding churn as the submitter.

## §345 REFILLED 10-HYPOTHESIS SET (2026-08-01 22:44) — survivor kept, 9 new, full coverage
Standing rule (user): always 10 live hypotheses w/ probe coverage; survivors keep slots; refill
on elimination; CONFIRM the source, never assume. Current set for the mid-box tale kill:
S1  change() grant kills the tale (survivor, UNCONFIRMED) ......... §344 grant log
H2  the 1→0 writer is dEvt remove(), not change() ................. §345a remove() log
H3  the 1→0 writer is the Step close-path misfiring ............... §341a context (2→0 vs 1→0)
H4  reset()'s RE-ENTRANCE mechanism (field_0xec!=0xFF queues an 0x201
    change order + arms mChangeActor) is the source ............... §345b reset logs
H5  submitter = daAlink (demo churn / proc change) ................ §345c reqProc
H6  submitter = ba1 ............................................... §345c reqProc
H7  submitter = knob00/door ....................................... §345c reqProc
H8  stale mChangeActor from an earlier event (door/awake era) ..... §345b setter logs (both)
H9  clothes/SHAPE beat (setClothesChange churn) submits ........... §345c timestamp vs §305 beats
H10 message machinery (§324/§201 box paths) submits ............... §345c timestamp vs §311/§324
Exe 22:44, caches wiped. Break run wanted: the §341a 1→0 line + whichever of
§344/§345a/§345b/§345c fired in the same window = CONFIRMATION, not inference.

## §346 CONFIRMED CHAIN (2026-08-01, log 22:45): the mid-box killer = SCENE DELETION mid-tale
§345a CONFIRMED H2 (S1 change() FALSIFIED — no §344/§345b/c fired): the 1→0 writer is
dEvt_control_c::remove(), whose ONLY caller is d_s_play.cpp:961 — inside SCENE DELETE (stage
transition teardown: dStage_Delete + Bgsp Dt + particle removeScene). So the tale dies because
A STAGE TRANSITION EXECUTES MID-TALE. Prime suspect (ties §335 lead iii): tale_1's order event
survives its warp; its START_TALE1 cut loops (§284b spam, §335 log) and RE-ARMS setNextStage
(point 200) — one re-arm lands while TALE_DEMO plays → reload → scene delete → kill. The door
break variant (KNOB_START killed by its own §329 warp scene-delete) shows benign-shaped in the
same log — the DOOR case is expected (warp = transition); the TALE case is the defect.
NEXT (10-hyp refill around the new survivor): probe every setNextStage/changeScene arm with
caller tag + runEvt + demoMode (who arms mid-tale: START_TALE1 re-fire? §322 early? №171 resume?
alink point-trigger? ba1 re-order? door?) — one build, all arm-sites tagged. Then donor-compare:
does the donor's order event end at its warp (tale_1 must DIE at the reload; §284b loop says
ours does not). ALSO: §341a kill line's runEvt='NO DATA' hints mCurrentEvId dropped BEFORE the
scene delete — sequencing detail for the probe.

## §347 ARM-PROBE SET BUILT (2026-08-01 22:53) — refilled 10 around the confirmed scene-delete
S1 (confirmed §346): a stage transition's scene delete kills the tale. WHO ARMS IT — the set:
H2 ba1 START_TALE1 re-fire (0xC8) mid-tale .......... §347b tag + §347a sig point=200
H3 ba1 START_TALE2 (0xCA) ........................... §347b tag
H4 §322 tale-exit early ............................. §347a sig point=203(0xCB)+'R_DL01'
H5 door armNativeStageChange mid-tale ............... §347c tag + §347a sig point=0
H6 alink startDemo/point machinery .................. §347a sig (unusual point)
H7 №171 owed-resume / stuck-message machinery ....... §347a (any arm w/ evRun=1 + its absence
                                                      in b/c tags = untagged armer)
H8 reset() re-entrance 0x201→sceneChange ............ §345b (live) + §347a
H9 Sequencer REVT-exit sceneChange (§321 family) .... §347a via changeScene4Event→set
H10 pre-armed transition surviving INTO the tale .... §347a timestamps (ARM before TALE start;
                                                      §341a nextStage field corroborates)
Central: §347a logs EVERY first-arm (stage/point/wipe/evRun/runEvt/demoMode/fnm) at
dStage_nextStage_c::set — the enabled-latch means every effective arm prints exactly once.
Exe 22:53, caches wiped. Break run wanted: the §347a ARM line with evRun=1 + runEvt='TALE_DEMO'
mid-fnm names the armer; its §347b/c tag (or absence) attributes the call site. Then the donor
lifecycle fix (order event must die at its warp) + THE BIG STRIP.

## §348/§334g (2026-08-01 23:00): WATER REGRESSION FIXED + a full clean tale run recorded
1. Run 22:53: FULL CLEAN tale run (no §347a mid-tale ARM — the armer didn't fire this run;
   §347 set stays armed until one break confirms).
2. WATER REGRESSION (user: swim became void) = §334's OWN wrong-semantic row: WW WATER(0x13)/
   WATERFALL(0x17) were in the slip set (ground=4 void/slide) — but TP waterness is GROUP-level
   (dzb Grp water bit → WATER_GRP, d_bg_s_grp_pass_chk.h; groups preserved by the bake), so
   water polys need NEUTRAL per-poly attrs; ground=4 overrode swim as fall-void. §334g moves
   0x13/0x17 → standable/neutral. Slip set now: LAVA/VOID/DAMAGE/FREEZE/ELECTRICITY only.
   Table doc + the §334 bus entry need this amendment at acceptance time. Exe 23:00.
USER: (a) swim check anywhere previously swimmable, (b) keep running tales for the §347 armer,
(c) the loft railing §334e bind is STILL owed (never yet tested post-§334f).

## §349 (2026-08-01, log 23:01): railing FIXED, water FIXED, windows BOUNDED, §322 exit donor-perfect
Run verdicts: railing walkable (§334f), water swims (§334g), no break — and the §343 "minor
regain" = TWO bounded mEventStatus==0 windows inside the ordered chain:
  W1 (larger): tale_1 close (§341a 1→0 @9349) → TALE_DEMO start (@9374) — the order→demo gap;
     START_TALE1 §347b re-ARM spam (9045-65, evRun=1, latch-blocked = benign noise but marks
     the loop that §335 flagged).
  W2 (short): TALE_DEMO close (@10271) → DEFAULT_START (@10276) — the post-reload gap.
MILESTONE: §347a shows the §322 exit firing donor-perfect (point=203 wipe=5 demoMode=2 fnm=709).
PROBE CAVEAT discovered: §341a is a SAMPLER — intra-Step multi-transitions collapse (a fast
1→5→2→0 reads as "1→0"), so the 9349 "direct 1→0" may be a legit fast full-close. NEXT probe
refinement: write-site transition logs (at the 3 writers) + g_Counter frame stamps to measure
window length in FRAMES and sequence the close exactly. FIX direction (donor-shape, §343): the
ordered chain must be GAPLESS — decomp question: does the donor hold the closing event open
until the transition executes, or hold input lock across the gap? Read the donor order-event
lifecycle (WW d_event.cpp Step-equivalent + order flags 0x201/0xE00 semantics) BEFORE coding.

## §350 WINDOW PROBE SET BUILT (2026-08-01 23:24) — 10-hyp board for the §343 control windows
S1 windows real (§349 survivor). H2 fast-full-close vs H3 direct-writer → §350b site tags at
the 5→2 (endProc) and 2→0 (close) writes, frame-stamped; §344/§345a live for direct writers.
H4 window length in frames → §350a gFrm stamps on every §341a transition. H5 wipe-late →
§350a fade= field. H6 donor-holds-event vs H7 donor-holds-input → the port sequence from
§350a/b feeds the donor lifecycle read (decomp next session). H8 W2 arrival-late → §350c
entry() GRANT gFrm vs playerInit CREATE line. H9 START_TALE1 loop widens W1 → §347b (live) +
§350b ordering. H10 what input leaks → §350a linkSpd= (nonzero speed inside a window = movement
accepted). Exe 23:24, caches wiped.
RUN PROTOCOL: one full tale; during BOTH gaps (pre-tale, post-tale) hold a movement direction —
linkSpd + gFrm deltas then measure each window and what leaked. The donor-compare then decides
the gapless mechanism (hold-event vs hold-input) and the fix implements THAT, natively.

## §351 MEASUREMENTS (2026-08-01, log 23:25): status windows are 1-FRAME — the regain is INSIDE status 1
§350 verdicts: W1 = 1 frame (gFrm 3298→3299, linkSpd 0), W2 = 1 frame (4141→4142, linkSpd 0),
arrival event holds 26f then closes clean. H4 ANSWERED: the status-0 windows are sub-perceptual
— NOT the user's regain. REFILLED lead (now S1): the real window sits INSIDE status 1 — after
the STB's authored END releases Link's demo proc (daAlink dProcTool exits at fnm 709; §317
post-end samplers showed free movement there in earlier logs) but BEFORE the §322 reload's
scene delete executes. Screen is in the authored fade; input is live; event status never moves
— invisible to §341a BY DESIGN. Donor difference candidate: donor's fade(20f)+prompt transition
covers the gap OR donor Link stays proc-locked while a transition pends. NEXT PROBE SET (build
next session, 10-hyp): stamp §347a ARM with gFrm; stamp scene-delete entry (d_s_play:961 region)
with gFrm; sample Link proc + input-accept + speed every frame from demo-END to scene-delete
(bounded window sampler); donor read: daPy proc lifecycle at STB END with pending nextStage.

## §352 GAP PROBE SET BUILT (2026-08-01 23:34) — 10-hyp board around the in-status-1 gap
S1 (§351 survivor): the regain sits between the STB's END (Link's demo proc releases) and the
scene delete. Board + coverage:
H2 ARM→delete latency is the gap ........ §352a ARM gFrm + §352b SCENE DELETE gFrm
H3 Link's proc exits at STB END ......... §352c per-frame sampler (speed+pos through the gap)
H4 movement leaks (magnitude/duration) .. §352c linkSpd per frame, 240-sample bound
H5 transition needs N Step/wipe frames .. §352a/b delta
H6 donor holds the EVENT open ........... port sequence feeds next-session donor read
H7 donor holds INPUT .................... same read, opposite sign
H8 screen not actually covered .......... §352c fade= per frame (mDoGph vs the demo00 actor fade)
H9 gap is §322-exit-path-only ........... context (sampler arms only on nextStage+mode2)
H10 sampler interference ................ read-only sampler; §352c gap CLOSED line = self-check
Exe 23:34, caches wiped. USER: one tale run, hold a direction through the tale's end; the
gap[N] lines measure everything. On the verdict: donor daPy/dEvt lifecycle read → the gapless
fix → THE BIG STRIP (now §334c/d/e §336ab §341abc §344 §345abc §347abc §350abc §352abc).

## §353 (2026-08-01 23:47): BREAK CAUGHT CLEAN — killer is a dEvt remove() with NO scene-delete
Golden log 23:40: §345a `dEvt remove() while status=1 runEvt='TALE_DEMO'` at fnm=106 → §341a
1→0 (gFrm 2340) → tale dead mid-box. CRITICAL: NO §352b scene-delete stamp adjacent (the paired
§352b+§345a pattern appears ONLY on legit stage loads) — so this remove() did NOT come through
d_s_play:967 (the only compiled wrapper caller). Remaining paths: (a) the dEvt_control_c CTOR
(runs remove(); the play-info is global — a re-init on the live object would kill the running
event; №90/91 re-init class), (b) a SECOND dEvt_control_c instance somewhere whose member
remove() fires our probe (red herring) while the real 1→0 write is elsewhere. §353 ctor tag
added (status(pre)+gFrm) — the next break discriminates ctor-vs-wrapper-vs-phantom. ALSO noted:
three §345a-without-§352b at KNOB_START/'NO DATA' moments = same unexplained path (door uses
trigger it too — reproducible more often via doors?). Exe 23:47, caches wiped.
USER: door in/out a few times + one tale — §353/§345a/§352b pattern will name the path.

## §354 (2026-08-01, log 23:48): NO INPUT LEAKS — the "regain" is a PRESENTATION gap (fade missing)
§352c verdict: the gap = 31 frames, linkSpd=0.0 EVERY frame, pos pinned (-289,375,83) — user held
a direction (protocol) and Link never moved ⇒ input is already locked donor-true. H8 CONFIRMED:
fade=0 through the whole gap — the STB's authored fade-out (d_act3 channel-9 → startFadeOut ~20f
@680) is NOT covering the screen at the gap ⇒ the visible camera/HUD revert reads as "regain".
§353: ZERO ctor lines — ctor path ELIMINATED for the stamp-less §345a removes (phantom-instance
or paired-at-distance question remains OPEN for the full-break killer; three stamp-less removes
in this log too). NEXT PROBE SET (§355, build immediately per standing rule): (1) demo00 twin
channel-9 beat log (dir/frames/gFrm) — did the fade fire; (2) tag ALL mDoGph offFade()/
setFadeRate-clear callers (doors armNativeStageChange:323 is a named suspect) — who cleared it;
(3) fade-state change-only sampler (isFade edges w/ gFrm); (4) for the full-break thread: log
d_s_play delete PHASE entries (multi-frame phase spread could separate §352b from §345a) +
stack-side: count dEvt_control_c instances (address log in §345a: `this` pointer! — one line
change distinguishes global vs phantom). Board refills around: fade-cleared-by-door (S),
fade-never-fired, fade-fired-then-reset-by-reload, offFade at arm, phantom instance, phase-split
pairing, + 4 more on the fade-owner chain.

## §356 CORRECTION (user, 2026-08-01): TWO DISTINCT DEFECTS — do not conflate
1. CLEAN-RUN GAP (§352c measured): ~31f, input LOCKED, missing fade — presentation-class.
2. FULL BREAK (get-item kill): the §345a stamp-less remove() KILLS the event → status 0 →
   TOTAL control return — user has walked the whole room while camera+letterboxes stayed
   frozen on a suspended Hero's Clothes model (prior sessions). REAL control, not visual.
   The §352c "no leak" finding applies ONLY to defect 1. Both threads independently
   instrumented on exe 23:56 (§355 fade chain; §345a this+gFrm identity).

## §357 (2026-08-02, log 23:58): phantom ELIMINATED; ch9 fade FIRES but machinery never engages
Clean run (user: neither defect). §345a this= identical across all five removes → ONE global
instance; phantom theory DEAD (remaining: §352b pairing-at-distance check for the stamp-less
ones). FADE THREAD: §355d proves the STB's ch9 startFadeOut(20f) FIRED — but ZERO §355c isFade
edges followed and the gap reads fade=0 ⇒ either (a) port startFadeOut is gated/stubbed on this
path, (b) an offFade raced it, or (c) isFade()==mid-transition-only and a HELD black reads 0
(gap might actually be covered — sample fadeRate, not isFade). NEXT (code-read first, no probe):
read mDoGph_gInf_c::startFadeOut/isFade/getFadeRate impls on PC; then §358 probe = fadeRate
sampler in the gap + §352b/§345a pairing counter. The clean-gap "regain" question rides on (c).

## §358 BUILT (2026-08-02 00:03): the TWO-FADER discovery + the deciding probe
CODE READ: mDoGph_gInf_c::startFadeOut(int) routes to JFWDisplay's JUTFader (the system overlay
fader; EStatus None/Wait/FadeIn/FadeOut; draws when status!=Wait) — a DIFFERENT system from the
gInf fadeOut(f32)/mFadeRate/isFade() the §352c/§355c probes sampled. THE PROBES WERE BLIND to
the fader the STB's ch9 beat actually drives. Wait==idle/clear (touch_controls reads it so).
DECIDING QUESTION: after ch9's 20f fadeOut completes, does the JUT fader HOLD (≠Wait, keeps
drawing black through the gap = screen covered, gap truly presentation-free) or SELF-CLEAR to
Wait (screen uncovers = the §354 visible gap)? §358 adds jutSt= + gInf rate= to every §352c gap
sample. One tale run decides. If it self-clears: donor-compare JUTFader hold semantics (WW's
fader holds until the next startFadeIn/scene wipe) and fix the hold natively. Exe 00:03.

## §359 (2026-08-02, log 00:09): GAP MECHANISM CONFIRMED — no fader holds across the seam; PIVOT POSTURE
§358 verdict: jutSt=0 (None) + rate=0.00 through all 31 gap frames — the ch9 fade-out (§355d
proved the call) runs its 20f and FULLY CLEARS; neither fader covers the STB→reload seam. The
§354 presentation gap = a fader-hold semantics gap (donor: fader/overlap holds black until the
scene wipe engages). BOUNDED FIX (first History item of the Foundry era): donor JUTFader
advance()/control() read — does WW's completed FadeOut hold (draw persists) vs the port's reset
to None; restore the hold natively. NOT a pivot blocker.
PIVOT POSTURE (user decision pending): Foundry porting work CAN proceed on this foundation IF
the full break instrumentation stays ARMED in every build (change-only logs, negligible cost):
§345a(this+gFrm)/§352b/§341a/§353 catch the intermittent killer PASSIVELY during any future
tale run — no dedicated hunts needed; the next strike self-documents. The break remains the
known shaky plank: intermittent, mechanism cornered (a global dEvt remove() with no adjacent
scene-delete stamp; ctor+phantom eliminated; §352b pairing-at-distance = the open question).

## §361 (2026-08-02): FOUNDRY ROUTING — audio lands on History's queue; arc-name-shadow noted
Foundry's crash = THEIR pilot's arc-name shadow (WW Ep.arc staged over TP's own Ep archive →
TP actor read WW data with TP indices → death at model create). Fixed data-side; the kit driver
now permanently refuses donor arcs whose names the receiver loads (demands alias). DISTINCT
from KB-1 (the remove() kill) — do not conflate. Lesson class: OBJNAME shadow one level down;
kit playbook (§323 family) gains the arc-shadow guard as a standing rule.
ROUTING: invisible-plants/J3D render → Housing (their surface; Foundry hands receipts).
**AUDIO → HISTORY (new queue item, after tsubo):** (1) interior BGM (hosted rooms' WW music —
start/stop wiring per the established seam pattern: voice/BGM lifecycle memory
workflow_npc_sounds), (2) WW fire SFX, (3) the missing JA_SE tree/cut ids — NOTE this absorbs
the §327 SE debt (mshokki/knob donor-value passthrough ids become the real mapping pass).
Foundry keeps: audio differ + Pass-1 BGM inventory (= History's input data; wait for it).
History queue now: [user's §334 acceptance word pending] → tsubo (49 placements) → audio seam
(on Foundry's Pass-1 inventory) → KB-2 fader hold → KB-1 on next strike → probe strip.

## §362 AUDIO CAMPAIGN OPENED (2026-08-02) — full plan on bus §362
User order: native JA1 systems (songs "missing their systems"). Inventory confirmed: bridge =
JA1 data on JA2 primitives; ADSR dropped outright (parser:408 debt), §C.4 pitch path JA2-
semantic, velocity curve owed. Donor JASOscillator(280)/JASChannel(869)/JASTrack(1568) ALL
matched. DSP layer not portable (PC replaced it for JA2 already) → port JA1 SEMANTIC layers
above the PC backend. Phases: A=TOscillator envelope engine (namespaced JAudio1::) + per-track
env table + SimpleADSR wire; B=JA1 pitch math (§C.4); C=vel/pan/vibrato audit; D=Foundry
wav-compare + user ear. Findings doc = docs/WW Linked/ext-seq-audio-findings.md (§B fixed,
§C.3 fixed, §C.1 vol-probe instrumentation ready, §C.4 open primary). Phase A next.

## §363 PHASE A DONE (2026-08-02 23:09, in live exe): JA1 envelopes ported + OscRoute NOP root
TOscillator (JAudio1 ns) + donor ADSR track table + cmdSimpleADSR seeding + the DEEPER root:
cmdOscRoute 0xF0 was silently NOP'd (ADSR could never route to voices). JA1→JA2 noteOn seam
field-verified; debts: osc slots 2/3, copyOsc flags, rising-curve mirroring (Phase B authority
= the ported TOscillator). Gate: user ear (Outset + interior) + Foundry wav-compare. Then
Phase B = JA1 pitch math (§C.4 wrong frequency bands).

## §364 (2026-08-02 23:19, live exe): HALF-TEMPO ROOT FIXED — wall-clock tick stepping
tempoProbe ratio 0.500 = the §62 per-call formula assumed 60Hz caller; poll = ~30Hz sim. Every
WW song at half tempo all era (the dominant "really off"; "1.3% residual" was a measurement
artifact). Fix: ticks/sec = tempo×timebase/30 × real dt (steady_clock, 0.25s clamp, f64
accumulator); LO clamp removed, HI 48 kept. §363 envelopes were masked under it — re-gate both
with one listen. Expected probe: ratio→~1.000.

## §365 SPEC (2026-08-02): the donor's OWN tempo governor — port verbatim, let it govern
User ruling: trust only the decomp; no port-authored arithmetic; natural systems govern.
DONOR READ (JASTrack.cpp, matched):
- updateTempo (:748): field_0x368 = timebase(0x378) × tempo(0x376) / Kernel::getDacRate() × (4/3)
  [× outer tempo when switched]; children inherit parent's rate.
- The governor loop (:1478, per audio-subframe callback): field_0x364 += field_0x368;
  while (field_0x364 >= 1.0f) { -=1.0f; mainProc(); } — mainProc = one seq tick.
- Cadence: the callback runs at the JA1 audio subframe rate = DacRate/80 (≈400 Hz @32kHz — the
  classic 80-sample JAudio subframe; consistent with the §363 agent's (dacRate/80) finding).
  Sanity: ticks/s = tb×T×4/(3×Dac) × Dac/80 = tb×T/60 ✓ (= 238 t/s for i_link — matches the
  pre-§364 measured rate; §364's doubling and §B's target were both wrong).
PORT PLAN (§365, no invented math): port updateTempo + the 0x364/0x368 accumulator loop
verbatim into the ext_seq root; drive it from a DacRate/80-cadence heartbeat sourced from the
port's AUDIO domain (find the PC audio pump the JA2 path uses; hook the JA1 seqMain there —
audio-clocked like the donor, not the 30Hz game poll). REVERT §364's formula + retarget the §B
probe to tb×T/60. Then re-gate ear + Foundry differ.

## §366 RULING (user, 2026-08-02): THE VANILLA SYSTEM ITSELF — no mounting, no substitutions
Not a governor replica inside ext_seq: port WW's JASystem (JAudio1) stack WHOLESALE as the
playing system — TTrack(1568) + TSeqParser + TChannel(869) + TOscillator(§363 ✓) + TBank/
TBasicInst/BNKParser + player/heartbeat — donor-verbatim under the JAudio1 namespace, clocked
per donor (DacRate/80 subframe). ONE legitimate boundary only: the lowest audio-output layer
(where JA2 was already cut for PC — mixer/voice output), adapted at the SAME seam. ext_seq
becomes a RETIRED BRIDGE (kept as kill-switch until the native stack passes the ear+differ
gate, then stripped per the bridge doctrine). Port order: A2=TSeqParser+TTrack core (consumes
§363 TOscillator); A3=TChannel+voice→PC-backend seam; A4=heartbeat+player start/stop wired to
the game's BGM entry points (replacing ext_seq's startOwned); A5=gate (ear+differ), bridge
retirement. §364 reverts WITH A4 (the vanilla governor supersedes, not a patched formula).

## §367 A2 LANDED (2026-08-03 00:06, built green /O2 ✓): vanilla TTrack+TSeqParser in-tree
Donor-verbatim (47+60 fns, dtk markers, donor bugs kept), JAudio1 ns matching §363, boundary
layer ja1_boundary.h (full symbol→phase table in the agent report; TChannel = SILHOUETTE to be
replaced wholesale in A3). Named ports-of-necessity: JASDram pool→heap new (A4 sizes arenas),
x64 pointer-punning in cmdJmp/cmdPrintf flagged for A3/A4 decision. Bridge untouched, still
playing. NEXT: A3 = donor JASChannel.cpp(869)+JASBank/BNKParser verbatim; replace the A2
TChannel silhouette with the real donor layout; cut ONE boundary at the PC audio-output seam
(find where JA2's channel becomes audible on PC and cut JA1's TChannel at the equivalent
point, stubbed for A4). Then A4 heartbeat+entry-point swap (§364 revert rides A4), A5 gate.

## §368 A3 LANDED (2026-08-03 00:45, built green): vanilla TChannel + full bank layer in-tree
Donor-verbatim JASChannel/ChannelMgr/ChGlobal/AllocQueue + JASBank/BankMgr/BasicBank/BasicInst/
InstEffect/InstRand/InstSense/DrumSet/BNKParser (full noteOn path, key/vel regions, percussion).
THE BOUNDARY (ideal outcome): DSP register ops DELEGATE to the port's existing software DSP
(DuskDsp — same JASDsp::TChannel 0x180 block; 11 ops line-identical, 5 JA1-only shapes written
as donor-verbatim register writes). Voice-pool lifecycle = A4 stubs (JA2's allocator owns the
64-voice pool while the bridge plays; arbitration is A4's job, not a shim). Nonmatching donor
fns (BankMgr::noteOn/gateOn, ChannelMgr internals) reconstructed from GZLE01 retail asm; donor
bug kept+flagged (checkLimitStart cursor). x64 punning resolved via receiver's own intptr_t
precedent. ALSO: fixed a concurrent lane's shell-escape corruption in d_a_ext_vegetation.cpp:386
('\r' lost its backslash — the reference_shell_escape_hazard trap; one-char intent-preserving
restore; their lane should know).
NEXT A4: Kernel/driver (DacRate/subframes/heartbeat @DacRate/80 from the PC audio pump),
TDSPChannel pool arbitration vs live JA2 pool, WaveBankMgr wave-arc loading (the sample data!),
entry-point swap behind a §369 gate (default OFF until A5; §364 revert + bridge retire ride
the gate flip). Then A5: ear + Foundry differ.

## §369 A4 LANDED (2026-08-03 01:28, built green gate-OFF): driver+heartbeat+pool+waves+entry
Gate DUSK_JA1_NATIVE default 0 (ja1_native.h; knob00 pattern), all branches compile, gate-OFF
delta ZERO (verified: pool fence {0,0}, no audio hook, bridge untouched). Heartbeat = the true
audio clock: JASDriver::registerDspSyncCallback dispatched per 80-sample subframe (DacRate/80)
on the SDL audio thread (DuskAudioSystem.cpp:142 → JASAiCtrl.cpp:165) — donor pump order
subframeCallback→TDSPChannel::updateAll→aiCallback; the §365 governor runs on donor clock via
startSeq's registerSubFrameCallback. Voice pool = RESERVED SLICE [32,64) of the same DuskDsp
CH_BUF (JA2 fenced via data-driven setJa1ReservedRange; count 0 gate-OFF = bit-identical);
allocator-sharing rejected (incompatible callback protocols — donor-verbatim JASDSPChannel
ported instead). Waves: donor TWaveBank/TWaveHandle over the bridge's already-loaded arcs
(shadow virtual addresses, residency→forceStop native). Entry chain: startOwnedImmediate →
ja1Native_start → donor boot (resetCallback/initAll/newMemPool/BankMgr regist+assign/heartbeat)
→ root TTrack setSeqData+startSeq. Kernel/Player rows real incl. CUTOFF_TO_IIR_TABLE[512]
machine-diffed; 32028.5→32000 = receiver's own TARGET_PC decision (noted). A5 heads-up: bridge-
era master-vol/scene-send corrections are JA2-channel-side — native voices bypass; expect a
level difference at first ear gate. A4.5 RUNNING: the last support TUs (TSeqCtrl = THE BMS
READER, NoteMgr, OuterParam, RegisterParam, IntrMgr, TrackPort, Calc tables) — gate stays 0
until it lands; flip + §364 revert + bridge fence = the A5 build.

## §370 A4.5 LANDED (2026-08-03, built green + linked, both gate states): last support TUs real
Donor-verbatim (dtk markers, donor /* Nonmatching */ flags carried): JASSeqCtrl.cpp (THE BMS
byte reader) -> ja1_jasseqctrl.cpp; JASNoteMgr/JASOuterParam -> ja1_jasnotemgr/ja1_jasouterparam
(jastrack.h fences removed); JASRegisterParam/JASTrackInterrupt/JASTrackPort ->
ja1_jasregisterparam/ja1_jastrackinterrupt/ja1_jastrackport (boundary fences removed);
JASCalc.cpp -> ja1_jascalc.cpp — sinfT/sinfDolby2 are TABLE-DRIVEN (no baked table; donor
initSinfT builds 257-entry tables at boot), so Calc::initSinfT() is wired into the gate-ON
boot as the donor Kernel::init leg it is (JASAiCtrl.cpp:31, ja1_native.cpp) — without it real
sinfT derefs NULL. §370 adaptations (established classes only): JASDram new -> plain new;
reinterpret_cast<u32> alignment checks -> uintptr_t (§368 widening); DCZeroRange = receiver's
own PC memset realization (contract holds); M_PI = receiver global.h f32 vs donor double
(flagged, sub-audible). BOUNDARY TABLE: NOTHING REMAINS STUBBED — zero active
JA1_BOUNDARY_STUB sites anywhere (macro retained for future rows). /Zs green all 17 JA1 TUs
gate-OFF AND gate-ON; full incremental build green through link both states compiled in.
Sound-complete read: no remaining silencing symbol known — every seq-side donor row is real;
A5 = gate flip + ear/differ (expect the §369 level-difference note) + §364 revert + bridge fence.

## §371 GATE FLIPPED (2026-08-03 01:44, built green, caches wiped): THE VANILLA STACK PLAYS
A4.5 landed: ALL support TUs real (TSeqCtrl byte reader, NoteMgr, OuterParam, RegisterParam,
IntrMgr, TrackPort, full Calc family + the sinfT init boot wire) — ZERO stubs remain in the
17-TU JAudio1 port; both gate states compile+link green. DUSK_JA1_NATIVE=1: WW BGM now plays
through WW's own sequencer/voice/bank/envelope stack on the donor clock. Kill switch = gate 0
(bridge byte-identical). A5 GATE: user ear on Outset i_link + Grandma's house — KNOWN
EXPECTATION: a level (loudness) difference vs the bridge era (bridge-side master-vol/scene-send
corrections were JA2-channel-side; the donor volume law now comes from the track tree itself) —
judge TIMBRE/TEMPO/FEEL first, absolute level second. Foundry differ after ear. On PASS: strip
§364 + fence removal + bridge retirement + bus ratification. On FAIL: gate 0 restores standing.

## §372 (2026-08-03): FIRST-LIGHT CRASH SYMBOLICATED — gate back to 0 pending one bounded fix
Gate-ON crash on Outset load, symbolicated (llvm-symbolizer + PDB, standing rule): boot chain
ja1Native_start → ja1NativeInitOnce → ja1Native_registerBanks (ja1_bank.cpp:966) →
BankMgr::registBankBNK → BNKParser::createBasicBank → BNKParser::findOscPtr
(ja1_jasbnkparser.cpp:187) → TBasicInst::getParam (ja1_jasbank.cpp:387) = DEREF DEATH during
IBNK parse. PRIME SUSPECT: the §369 host-endian restore walker (ja1_bank.cpp, "convert at
load, parse verbatim") misses the OSC-pointer table region (or converts offsets the donor
parser then treats as pointers) → findOscPtr follows byte-swapped/garbage offset. FIX SHAPE
(next session, bounded): diff the walker's covered regions vs the donor IBNK layout the BNK
parser actually walks (INST/PERC/OSC/RAND/SENSE/ENV tables); the crash offset at :187 names the
first missed region. Gate = 0 (playable, bridge audio) until fixed; then re-flip = A5 retry.

## §372b (2026-08-03 01:50): FIRST-LIGHT CRASH ROOT-FIXED — GC pointer-size bzero; gate back ON
NOT the endian walker: TBasicBank::setInstCount zeroed `count * 4` bytes (donor GC pointer
size) over an x64 array of 8-byte pointers → upper half of the 128-slot inst table = heap
garbage → findOscPtr's mid-parse getInst(i) probe → getParam deref death. Fixed with
sizeof(element) at ALL FOUR sibling sites (mInstTable/mEffect×2/mOsc — swept; jastrack :1013
`*4` is a legit BMS offset, untouched). §368 widening class, §372-commented. Gate re-flipped
ON, built green, caches wiped, exe 01:50. A5 RETRY: user loads Outset — if it boots, the
listen protocol stands (tempo/timbre/feel first; absolute level difference expected).

## §373 SILENCE PIPELINE PROBES (2026-08-03 01:54): clock→seq→voice→dsp, one run discriminates
No-crash silence: boot clean (banks registered, seq "running", own=playing), bridge correctly
fenced (0 tempoProbe), zero warns — the stack is dark with no instrumentation. §373a heartbeat
first-fire+count (Kernel::subframeCallback), §373b mainProc tick count (TTrack), §373c first-4
noteOn w/ args (BankMgr), §373d first-4 TDSPChannel::play. Read: a missing STAGE names the
break: no §373a = heartbeat never registered/dispatched (ja1Kernel_installHeartbeat vs
JASDriver::registerDspSyncCallback dispatch); §373a w/o §373b = rootCallback not on the
subframe list (startSeq registration) or governor rate 0 (updateTempo inputs); §373b w/o §373c
= parser runs but noteOn path dead (bank/prog resolution); §373c w/o §373d = channel alloc
fails (pool fence/slice); all four = voices play but inaudible → mixer/volume/wave-resolve
(the §369 level-law heads-up at its extreme, or setWaveInfo virtual-address resolve). Exe
01:54, caches wiped. USER: load Outset once, ~20s, grep [JA1] §373.

## §374 (2026-08-03 01:58): silence narrowed — clock ✓ governor ✓ ONE garbage-looking noteOn, play never fires
§373 run verdict: §373a heartbeat FIRES; §373b mainProc ticks steadily (1000s); §373c exactly
ONE noteOn (bank=1 prog=0 key=60 vel=1 — vel 1 suspicious = mis-parse shaped); §373d NEVER
(TDSPChannel::play unreached). Pool init audited clean (init→status 1, alloc scans fine).
Prime suspect moved UP: the NATIVE BMS reader (A4.5 TSeqCtrl) mis-steps the stream → almost no
real notes → the one noteOn is an accidental note-shaped byte. §374a cmdNoteOn count/args +
§374b first-24 opcode trace added (diffable vs the bridge parser's golden stream — the bridge
read these exact BMS bytes correctly; seq_events_engine CSVs are the oracle). TAG NOTE: §373
number collides with Housing's live [ExtBg] §373 probes — mine renumbered §374, prefix [JA1].
GRANDMA CRASH (same run) = NOT AUDIO: symbolicated to daNpc_Ba1_c::setMtx → McaMorf::calc →
J3DJoint calcTransform — Housing's ACTIVE J3D surface (their uncommitted J3D edits are in this
tree); relay to Housing, do not chase in the audio lane. Exe 01:58. USER: Outset load ~20s
(no Grandma needed for the audio read).

## §374c (2026-08-03 02:03): PIPELINE FULLY ALIVE — silence is the LAST INCH (wave resolve or mixer)
Run 02:00 verdict: §374b opcode stream TEXTBOOK (14×0xC1 openTrack, tempo/timebase, real setup)
→ §374a cmdNoteOn real notes → [JA1] §373c noteOn ×4 real banks/progs/vels → §373d
TDSPChannel::play ×4 — EVERY stage fires (prior run's missing §373d = probe-window artifact).
Reader exonerated. Silence = voices playing inaudibly: (a) shadow-wave address unresolved →
DuskDsp's miss path SILENTLY zeroes the voice (mSamplesLeft=0, no log!) — §374c warn added
there (first-6, logs the address); (b) else = the JA1-shaped mixer writes land as zero volume
(setMixerVolume ramp-delay arg / setAutoMixer packing / bus connect) — next probe if §374c
stays silent. Exe 02:03. USER: Outset ~20s → grep '[JA1] §374c'. Present = the mint/registry
mismatch (fix in the wave-handle mint or registration under gate-ON); absent = mixer-volume
probe next (§374d: log the first mixer writes' values at the DSP wrapper).

## §374d (2026-08-03 12:32): waves RESOLVE (§374c zero) — mixer-write value probe armed
The fork's first branch died: zero §374c across a full run = every shadow wave address
resolves; the sample data is reaching the DSP. Silence = the volume realization. §374d logs
the first 24 mixer writes with values (autoMix vol/pan/fx, mixInit ch/vol/delay, mixVol,
busCon) via ja1DspProbe374d (helper in ja1_kernel.cpp; header-safe). READ: all-zero volumes =
the track volume law feeding zeros (donor calc chain — check TChannel updateMixer inputs);
sane volumes + busCon absent/wrong = output bus never connected (donor initAutoMixer/busConnect
leg not running); sane volumes + sane bus = the LAST candidate is the auto-mixer flag path
(mAutoMixerBeenSet semantics vs DuskDsp's mixer mode selection). Exe 12:32. USER: Outset ~20s.

## §375 SILENCE ROOT CONFIRMED (2026-08-03): out-of-domain bus indices → OOB connect_table → zero output
§374d verdict: volumes SANE (1556/1331), but setBusConnect args = 80/16/82/18 — BOTH the JA2
delegate AND the donor's own DSPBuffer::setBusConnect (WW DP JASDSPInterface.cpp:324, table
IDENTICAL) expect an INDEX < 12. 80 → connect_table[80] OOB → garbage mBusConnect → DuskDsp's
GetOutputConfig (expects 0x0D00/0x0D60 L/R) never matches → zero volume → THE ENTIRE SILENCE.
Everything else in the vanilla stack is PROVEN WORKING (reader/governor/notes/banks/voices/
waves/volumes). THE LAST SEAM: the JA1-port TChannel mixer setup computes wrong indices —
suspects: (1) the A4 Driver:: config-getter STUBS ("donor retail values 1/1/0x2ee0/0x7fff")
feeding mixer init with wrong-shaped config; (2) a BE mis-read of the mix-config word whose
byte/nibble the donor extracts as the index (patterns 0x50/0x10/0x52/0x12 suggest flag-bits
around a true index). NEXT (one session, bounded): read the ja1_jaschannel.cpp setBusConnect
CALL SITE vs donor JASChannel.cpp's (what the donor extracts from which config), fix at the
SOURCE (config data/read), never clamp the index. Then: SOUND, and the A5 ear at last.

## §375b THE FIX (2026-08-03): little-endian mirror of the donor's MixConfig overlays
Confirmed four-for-four: ctor defaults 0x0150/0x0210/0x0352/0x0412 — donor index lives in the
HIGH byte (1=LEFT, 2=RIGHT, 3, 4); LE misread returned the LOW bytes 0x50/0x10/0x52/0x12 =
the exact 80/16/82/18 from §374d. Donor overlays are GC-shaped twice over: big-endian byte
order AND MWERKS MSB-first bitfields. Fixed by mirroring member order, values untouched:
  - ja1_jaschannel.h MixConfig.mParts: { u8 l1:4; u8 l0:4; u8 u; } (was u/l0/l1)
  - ja1_jaschannel.cpp play() local union: { u8 lo; u8 hi; } (was hi/lo)
Closure check: every mWhole writer is host-endian (ctor literals, ChannelMgr defaults
:973-:978, TTrack::setChannelParam via the proven-byte-correct BMS reader); 0xFFFF writes
endian-neutral. Track-side unions (:182 MWERKS-only, :313 free-list) are object overlays —
endian-safe. NO other bitfield overlays exist in the 17-TU stack (swept). Expected §374d
after fix: busCon b = 1/2/3/4/0/0 → GetOutputConfig matches 0x0D00/0x0D60 → volume flows.
Bug CLASS registered alongside §372b (GC pointer-size): GC-shaped memory overlays carried
verbatim — grep unions/bitfields on every future donor TU port.

## §376 EAR PASS (2026-08-03): the vanilla JAudio1 stack PLAYS — exteriors AND interiors
User verdict: "The music is perfect now! At least to the ear... it's here for exteriors and
interiors!!" The §375 endian mirror was the last seam. The full donor stack — TTrack reader,
tempo governor (timebase×tempo/60), TSeqParser, TChannel mixer, banks/oscillators, TDSPChannel
voices — is the playing system end-to-end, DUSK_JA1_NATIVE=1. A5 gate is HALF passed: ear ✓,
Foundry wav-compare differ PENDING (the data half). Strip list (§364 wall-clock bridge step,
ext_seq bridge retirement, probe families §373a-d/§374a-d + the earlier queue) stays HELD
until the differ ratifies; kill switch remains DUSK_JA1_NATIVE=0 until then.

## §377 KB-1 ROOT-CAUSED + FIXED (2026-08-03, log 194812 gFrm=7732)
The armed passive probes caught the strike fully attributed — adjacent lines: §345a remove()
while status=1 runEvt='TALE_DEMO' + №89 force-end (arrival-G-guard) + §50 TRUNCATION at
storyboard frame 117. Killer = OUR №89 watchdog: the tale's §296 two-step re-entry re-arms it;
the 120f expiry landing on an STB DEMO beat (unprotected — §313 only held BOX beats) force-ends
the live demo. Intermittence = player text pacing decides which beat the expiry lands on. The
"stamp-less remove" mystery closes: caller was dExtNpcMount_forceEndDoorEvent, never the
scene-delete wrapper. FIX: §377 in d_ext_npc_doors.cpp pollArrival — guard also holds when
dDemo_c::getMode()==1 (storyboard actively presenting ≠ stuck residual; residuals are mode 0).
Minimal, mirrors §313's shape, other force-end legs untouched (never struck). 3600f backstop
unchanged. Built clean; KB-1 doc updated; probes stay armed for the confirming clean run.

## §378 DOOR-NATIVE CAMPAIGN OPENED (2026-08-03) — user rulings + donor spec + duplicate fix
USER RULINGS: no §377-style exemption patches (strip §377 with the guard); no census probe —
replace with native pathing; DECOMP-FIRST; delete №89 complex when the native pathing lands
(same build — deleting before it = permanent control-lock, log 194812 gFrm 1178→1296 shows a
KNOB event surviving 118f with the guard as the only release). Supersedes the old "do not
weaken forceEndDoorEvent" ruling.

DONOR SPEC (D:/XXXXXXX/WW DP): knob00 has NO stage change; Link's dProcDoorOpen only plays the
anim + cutEnd. The warp is a STAGE STAFF inside the door EVENT itself (d_event_data.cpp:20-52):
reads Stage/StartCode/RoomNo/Layer/Mode/Wipe props → dComIfGp_setNextStage. The scene reload IS
the teardown (§323 archetype) — the donor has NO watchdog because the event orders its own next
stage from within. №89 has been impersonating this staff.

DUPLICATE DOORS ROOT (user sighting confirmed): donor LinkRM room.dzr carries its OWN
KNOB00 @(-255,0,1125) + SCLS x1; since §329 put KNOB00 in the OBJNAME table the room lane
spawns it natively (log: pid 360, in donor DZR order BG→KNOB00→Lamp), then the synthetic
spawnExitKnobIfNeeded added a twin (pid 378) at the same coords. FIX §378: adopt the native
door (stamp exit key onto it, warp path follows the stamp); synthetic spawn = fallback only
for rooms with no donor door. Placements-CSV KNOB00 rows are inert (population has no KNOB00
mapping) — the DZR room lane is the real native spawner.

REMAINING (the §379 order): author door EVENTS with STAGE staffs (merge_event.py, №152
precedent) for every door pair in F_DL01/R_DL01 event_lists + wire knob00 eventInfo CANDOOR →
native event → STAGE staff warp (donor SCLS points exist: LinkRM SCLS x1); THEN delete in one
build: №89 all three legs (arrival-residual/arrival-end/G-guard), №170, §313, §377, the
synthetic arrival machinery, armNativeStageChange bridge legs made redundant.

## §378b CONFIRMING RUN (2026-08-03, log 203949): tale CLEAN end-to-end + one door
User: "No cutscene break, control returned" (= proper end-of-tale return). Log: TALE_DEMO to
fnm=709 → its own native reload ARM (point=203 wipe=5) — the §323 teardown as authored; ZERO
G-guard force-ends the whole session (only benign arrival-end clears + disarms); §378 fired
("adopt NATIVE room-lane door key=exit:linkrm") — no synthetic twin. KB-1 probes stay armed
(intermittent bug — one clean run is confidence, not closure). NOTE: Housing now uses §377
for [ExtBg] mat probes — tag collision; ours is distinguished by the [Doors] prefix.

## §379a EXTRACTION DONE (2026-08-03): WW door events merged into both host stages
User ruling: "Always fully native, as far as TP allows, per the covenant." Dumped
DEFAULT_KNOB_DOOR_F_OPEN/B_OPEN from WW sea + LinkRM Stage.arc (identical across WW stages):
  SHUTTER_ staff (type 10): SETANGLE → ADJUSTMENT → OPEN_PULL(F)/OPEN_PUSH(B) → WAIT
    (= the ported knob00 demoProc action table, cut for cut)
  Link staff (type 0): 001n_wait → 012unequip → 005wait_turn → 001n_wait →
    035door prm0=1(pull)/0(push) → 001n_wait  (= Link's native door acting; prm0 is
    dProcDoorOpen's getParam0()&1 anim select — donor-verbatim match)
  CAMERA staff (type 2): UNITRANS RelActor=@PARTNER Eye=(100,30,300) Fovy=60 Timer=20 → PAUSE
Merged via merge_event.py (№152/§273 pattern, data-side port, engine loads natively) into
F_DL01/STG_00.arc (verified idx 2/3) AND R_DL01/STG_00.arc (verified idx 7/8). No OffsetPos
in these events (scripted, relative cam) — the F_DL01 MISMATCH warning was the tool's STB
check, not applicable. Stale tool-header claim corrected: hosts did NOT already carry these.
NEXT (§379b, one build): wire knob00's dormant CANDOOR chain to fire them (remove the doors
module's direct-warp intercept), warp via door-DZB exit poly + SCLS through §334 translation,
then DELETE: №89 all legs, №170, §313, §377, §347a door arm. KNOB_START arrival side stays
(native both engines).

## §379b WIRING (2026-08-03): native door events live — two donor-shaped seams
ZERO-CODE CORE: dEvt entry (d_event.cpp:543) already prefers the native path — actor's
eventInfo eventId + getEventData != NULL → order(). The direct warp was always the
data-missing fallback; with §379a's merge the native order fires by itself. knob00's
CANDOOR chain (setEventPrm → checkCommandDoor, §329 verbatim) was never dormant — it fell
through because the events didn't exist.
SEAM 1 (d_a_knob00.cpp openProc): §379b warp at the donor's own moment — the frame-15
fadeOut that in vanilla covers Link crossing the door's exit-attributed DZB poly
(dStage_changeSceneExitId → SCLS). Hosted stages carry no SCLS bake yet → the port's
resolver issues the SAME native setNextStage there. SCLS bake retires ONLY this block.
SEAM 2 (d_ext_npc_doors.cpp pollWwDoorLinkStaff): WW Link-staff interpreter — evmng's
Alink↔Link alias (:1097) binds TP Link; 001n_wait/012unequip/005wait_turn → cutEnd (TP
unequips on door command natively; knob's SETANGLE/ADJUSTMENT place Link — donor division);
035door prm0 → changeDemoMode(DEMO_DOOR_OPEN_e, prm0) — TP procDoorOpenInit reads param0&1
right/left EXACTLY as WW dProcDoorOpen reads this same prm0 (lineage intact end-to-end);
procDoorOpen cutEnds itself.
GUARD DELETION HELD ONE MORE BUILD — receipts: every KNOB_START arrival in log 203949 ends
via our force-end (arrival-end ~17f), never natively. Departure is native now; the arrival
half (KNOB_START ending on its own staffs) is the №89-deletion gate. User ruling honored:
deletion lands with the pathing that makes it redundant — that includes arrivals.

## §379c PROBE+HEAL BUILD (2026-08-03): the exterior fallback thief, cornered per-door
User verdict on §379b: "works fantastically" — native door events + Link's door animation
CONFIRMED IN GAME (interiors + exterior ojhous F_OPEN @1490 in log 211755). Remaining thief
is DOOR-SPECIFIC, not exterior-specific: 'linkrm' (the lane=room door) fell to the "§27
native OPEN" entry fallback = unresolved/unset event id at press. §379c build (exe 21:44):
  - tryNativeWarp KNOB00 branch: FALLBACK-press probe logs evIdx7/8 pre→post + room + src,
    with a makeEventId re-run HEAL when both -1 (H1 resolve-timing; next press rides native)
  - knob00 setEventPrm: change-only gate probe (PASS/fail per door, evIdx, room, ry) — H2:
    a door whose donor checkArea/angle gate never passes can never offer CANDOOR
  H3 (room-typing/lane=room room 0 claim) discriminated by the room= fields in both probes.

## §380 DOOR-SYSTEM PORT READINESS (2026-08-04) — user ruling: port the full donor interaction system
DIAGNOSIS RATIFIED: the acting layer went verbatim in §329 (demoProc/openProc/openEnd/
adjustment/all action FUNCTIONS incl. actionInit's type routing) but the donor EXECUTOR was
replaced by a §27 hybrid execute(): custom ACTION_DEMO handling (mEvtOrdered + a THIRD warp
site at demo END), custom №91-style prompt arming, and NO dDoor_info_c::checkExecute gate.
The talk-slot misfire (§379c) is the hybrid's param scheme feeding donor type bits.

DONOR EXECUTOR SPEC (WW DP d_a_knob00.cpp:809-886, read verbatim):
  execute(): switch(checkExecute())  — 0: setAction(0) frozen re-init
                                     — 2: mAction dispatch {0:Init 1:Wait 2:Demo 3/7/9:Talk
                                          4:TalkWait 5:Passward 6:Passward2 8:Villa
                                          10:Figure default:Dead}
                                     — 1: startDemoProc()+demoProc() (other-staff demo accept)
             tail: mRoomNo2=stayNo; getType2()==1 → calcMtx+BgW Move
  checkExecute (dDoor_info_c, ALREADY PORTED): frozen-status / commanded / adjacent-room gate.

THE PORT PLAN (one build, on user go):
 1. Replace hybrid execute() with donor execute() verbatim. All callees already ported.
 2. DONOR PARAMS on §27 spawns: 0x0ffff0ff (the DZR normal-door param; swbit FF, type 0,
    eventNo FF, swbit2 FF) at doors.cpp:899 + :1363 — KNOB00 create only; the henna0 mount
    FALLBACK keeps the legacy params (its own decode, retire-listed with the mount class).
    Identity/exit-ness live ENTIRELY on the doorKey channel already ('door:'/'door:exit:'
    src → mDoorKey, №51 persist; stamp-by-position reconcile). Display-only doorIndex logs
    unaffected; §379c probe's param-bit dedup strips with the family.
 3. RETIRE port extras in the same pass: mEvtOrdered/ACTION_DEMO hybrid block, №91 prompt
    arming (donor offers via eventInfo CANDOOR + attention ACTION_DOOR natively), the
    demo-END tryNativeWarp call site (the §379b openProc fade seam is THE warp until the
    SCLS bake; §329 openEnd stamped-warp stays as the non-fade fallback).
 4. KEEP (labeled TARGET_PC seams): §379b fade-warp seam, §328 shims, makeEventId receiver
    idiom (actor-form getEventIdx), §334 dzb repack.
 5. Donor actionDemo non-reset for slots 7/8 = the reload teardown (§323) — verbatim.
 6. Sequence after: arrival-half nativization (KNOB_START ending via its own staffs) →
    №89 complex deletion (№89 3 legs, №170, §313, §377) + §347a door arm, ONE build.

## §380b PORT LANDED + CORRECTION (2026-08-04)
CORRECTION for the record: the native knob00 executor was ALREADY donor-verbatim (execute()
:1088, full checkExecute gate + dispatch) — my §380 "hybrid executor" read had picked the
gate-OFF legacy class (:1540) by mistake. The live defect was ONLY the spawn params, exactly
as §379c proved. LANDED: createKnobAt now takes (doorIndex, isExit) and builds per-actor
params — KNOB00 gets the donor's own normal-door word 0x0ffff0ff (byte-identical to WW DZR
doors: swbit FF/type 0/eventNo FF/swbit2 FF); the henna0 mount FALLBACK alone keeps the
legacy 25|idx<<8|exit encoding (its own decode; retire-listed). Identity/exit-ness ride the
doorKey channel. Kills the §379c type-corruption (linkrm type1 message door / ojhous type2
night-message door / onobuta+omasao 4/5 passward-villa) — all §27 doors become donor type-0.

## §395 LAMP CANDLE LATCH FIX (2026-08-04, bus §368/§394 ferry)
Foundry receipt (§394): lamp.json = ONE param (Amount of Swing) — no flame knob exists, so
the flameless candle is the §368 init-latch, confirmed as the sole lead. FIX (d_a_lamp.cpp):
latch mParticleInit only when particle_set returns a non-NULL TORCH emitter; NULL first
frame → №246-style warn (≤4) + retry next frame. Donor-identical when the first attempt
succeeds (donor's never fails); self-healing on the host when it does.

## §396/§397 (2026-08-04): candle root = NAME-COLLISION ids; interior exit = press RACE deadlock
§396 CANDLE: §395's success-latch fired forever ("TORCH emitter NULL ... retrying") — deeper
than a transient. Offline JPAC1-00 walks: receiver-enum 0x41 is in NO WW jpc; the DONOR values
are ID_AK_JN_TORCH=0x01EA and KAGEROU00=0x4004 (WW DP d_particle_name.h:121/:37) — the §327
port took the receiver enum's same-NAMED entries at TP's VALUES. Bug class registered next to
§372b/§375: shared-lineage enums may collide by name with different values — always verify the
DONOR's numeric value. Fix: donor literals in d_a_lamp.cpp + both ids added to the sWwCommon
wwJPA bridge table (0x01EA + 0x4004 both verified present in the staged common.jpc).
§397 INTERIOR EXIT: log 085918 — the interior TrigA PROXIMITY leg queueExit'd on the same
press that ordered native B_OPEN → pending stage change blocked endProc (№89 rule) while the
running event deferred the change = deadlock, no return to exterior. Fix: the proximity leg
stands down when the doorway's knob00 has resolved door events (native owns the press; §379b
seam warps). Leg remains for doorways with no native-event door.
OPEN ITEM — interior "Open" prompt UI missing (user report): could be a symptom of the §397
race state; retest decides. If still missing: attention-lock investigation (player room vs
door room in the attention search) next. NOTE: §380 donor params collapsed the §379c gate
probe's dedup key (all doors share param-bit 16 now) — interior gate lines absent = probe
artifact, not evidence; probe strips with the family anyway.

## §398 TALE BLACK-SCREEN probe build (2026-08-04, log 092751)
Candle CONFIRMED BURNING (§396 donor ids ratified in game). NEW defect: tale → continuous
black screen. Chain established from the log: tale ordered CLEAN (nextStage=0 at §341a
gFrm=2869) → its two-step arm STUCK (§347a ARM R_DL01 point=200 gFrm=2877, first-arm-wins)
→ pending NEVER consumed (no reload; §284b/§347b loop to end-of-log; every later set() a
silent no-op since dStage_nextStage_c::set only acts if !enabled). Sole consumer =
d_s_play.cpp:602 gate. Four starvation modes, §398 probes armed at the gate:
H1 fopOvlpM_IsPeek() stuck (suspect: the native door ENTRY's overlap wipe never closed —
   the tale was never tested on the §379b entry path before this run)
H2 dComIfG_resetToOpening  H3 fopScnM_ChangeReq returns 0 (silent refusal, retried forever)
H4 pending disabled late. Probe prints pending/pt/peek/reset per 60f while a pending exists
+ ChangeReq-refused line. NOTE tale cut_move arg audit while here: setNextStage(host, 0xC8,
0, 8, ...) puts 8 in the LAYER slot (donor "mode-8 wipe" comment) — pre-existing in all
working runs, not the regression, but flag for the donor-args re-read when the tale
stabilizes.

## §399 THE G-GUARD KILL LEG IS DELETED + seam fences (2026-08-04, log 094033)
FULL ATTRIBUTION of the regression cluster — every branch №89-guard collateral:
 1. B_OPEN killed 21f in by the 120f G-guard (gFrm=1692, adjacent №89 lines) — SCRIPTED door
    events were its third victim class (§313 exempted boxes, §377 exempted STB demos; a
    scripted event is neither). The user entered + pressed exit within the 120f window.
 2. Interior collision loss = Link's door proc killed mid-SetWallNone (donor cleanup never
    ran) — pure collateral.  3. Dead exit door = knob00 waiting on the killed event.
 4. Tale black screen = the killed door's anim finished LATE during tale_1; the §329 openEnd
    seam fired a STALE 'exit:linkrm' warp (19532) whose F_DL01 arm WON first-arm over the
    tale's point-200 arm; §398 receipts then showed ChangeReq REFUSED (2426) and peek=1
    stuck from 2481 — an overlap covering the screen with no scene change behind it.
EXECUTED (user ruling): the G-guard KILL LEG deleted outright, §313+§377 exemptions with it
(nothing left to exempt); countdown/disarm/camera bookkeeping kept; 3600f backstop unchanged.
Both knob00 warp seams (openProc fade + openEnd) fenced to fire ONLY while a
DEFAULT_KNOB_DOOR_* event is the running event — no stale arms possible. STILL STANDING for
the arrival-nativization build: the ~17f arrival-end leg (ends stalled KNOB_START arrivals),
№170, the §347a-era arms — those retire when KNOB_START ends via its own staffs (knob00
demo-accept, donor execute case 1 — already ported, needs the event-side door binding).
§398 probes stay armed (ChangeReq refusal + peek mechanics unproven under clean conditions).

## §400 FADO PLAIN-ARRIVAL (2026-08-04, log 095110)
§399 deletion surfaced the guard's LAST hidden dependent: the Fado crossing forwarded TP's
door-path `mode` → destination Link spawns startMode=10 → alink orders KNOB_START (:5149
startEvent 0xFF branch) → a door walk-in with NO door on the Outset side → stalls forever
(tail: §285 KNOB_START loop; camera never released — "as if something is loading"). The kill
leg had euthanized this phantom at ~118f on EVERY Fado warp (log 085918: KNOB_START 870→988).
FIX: the intercept passes mode 0 (plain arrival) — the crossing is ours and doorless by
design; TP doors unaffected (intercept scoped to the Fado exit). PLYR data confirmed clean
(R44 evt=0xFF) — the phantom came from mode, not stage data. Door arrivals inside the WW
world still ride the ~17f arrival-end leg until the KNOB_START nativization (KeepDoorInfo
binding — donor knob00 execute case 1 already ported, event-side binding is the remaining
work).

## §400b PLAIN-ARRIVAL DATA PATCH (2026-08-04, logs 095836/095915)
§400's mode theory was WRONG (transit line fired, KNOB_START still ordered): startMode is NOT
the warp's lastMode — it decodes from LINK'S OWN PARAM built from the PLYR entry's param word
(alink.h:3605 (param>>0xC)&0x1F). Our authored PLYR data carries startMode=10 ("arrive walking
through a knob door") on every host spawn — correct TP authoring for door points, but NO door
participates in our arrivals yet, so every arrival (door OR warp — both user sessions) ordered
a KNOB_START that can never end; the deleted kill leg was the only thing that ever ended them.
INTERIM (honest data, no watchdogs, no code): startMode 10→0 on all host arrival PLYRs —
F_DL01/R44 (0xff00a02c→0xff00002c) + R_DL01 R00-R05 (a00x→000x). Backups *.pre400b-bak.
Arrivals are PLAIN until the native walk-in exists — which is the truth of the port today.
RESTORE PLAN (the definitive §401 session): KNOB_START nativization — dStage_KeepDoorInfo
fill on door use, Link's walk-in acting, knob00 demo-accept staff binding (donor execute
case 1 already ported) — then startMode 10 returns to the data and the arrival-end leg +
№170 delete. NOTE: §400's mode-0 change is harmless (redundant) — leave until §401 tidies.

## §398c REFUSAL AUTOPSY BUILD (2026-08-04, log 100415)
Interior→exterior exit black screen. §398 receipts: exit ChangeReq REFUSED at ~gFrm 1553 with
peek=0 (the request-slot itself busy — NOT an overlap gate), THEN an overlap stuck peek=1
forever. Refusal source pinned to fpcNdRq_ChangeNode's one condition: IsPossibleTarget &&
!IsIng — a PRIOR request on the play scene is still in the node-request queue, never
completing its phases; every later transition refused; its overlay = the black screen. §398c
probe added at the refusal: logs isIng + full queue autopsy (type/name/creating_id/phase_id
per entry, ≤8) — one repro names the lingering request and its stuck phase. Suspect class:
the interior ENTRY's own change request parked in a fade phase (IsDoneOverlap) that our
native-path fade handling (§355b offFade / knob00 donor fadeOut) never satisfies — the OLD
path had the stuck-fade watchdog (mount:8560) as janitor; the native path bypasses it.

## §398d PHASE-VISIBILITY (2026-08-04, log 101155)
§398c verdict refined: the early REFUSED lines are normal frame-2+ retry noise (first
ChangeReq queues, node goes untargetable during processing) — boot and entry both show the
identical pattern AND RECOVER. The broken exit diverges AFTER the overlap reaches peek
(covered/black, holding for the swap): the working legs then Execute; the exit holds at
peek forever. The missing datum = the queued request's PHASE while peek=1 (fadeFase: 0
IsDoingOverlap / 1 IsDoneOverlap / 2 Execute / 3 ClearOverlap). §398d: fpcNdRq_DebugHead
exposes the queue head; the §398 pending line now prints q=has:type/name ph<phase> + done=.
One repro reads the parked phase directly.

## §398e NON-CONSUMING DUMP + PROBE-SAFETY FIX (2026-08-04, log 101650)
§398d verdict: the exit's scene request parks at fadeFase[1] (IsDoneOverlap) with peek=1 —
waiting the overlap's done latch. MACHINE DECODED (c_request.cpp): cReq_Is_Done READS AND
CLEARS flag1 — a one-shot consume. TWO consequences:
 1. PROBE BUG (mine): §398d's done= field called fopOvlpM_IsDone → could EAT the latch the
    scene request waits on (heisenprobe). Fixed — §398e reads flags directly, never consumes.
 2. The ORIGINAL stall (pre-§398d logs): flag1 never set = overlap phase IsComplete never
    fired = the WIPE TASK never latched covered — wipe process create-failure or its own
    request starved. §398e dumps overlap act/peek/phase/base-flags + task existence + task
    request flags per sample: one repro discriminates create-fail vs task-starved vs
    latch-eaten.
Overlap lifecycle reference (for the fix): Create→IsCreated→IsComplete(task covered →
cReq_Done(base): flag1 for scene[1])→WaitOfFadeout(peek hold; needs flag2==2 via
OverlapClr=scene[3], peektime-gated)→IsWaitOfFadeout→IsDone(base done again for scene[4]).

## §398f CONSUMER IDENTIFICATION (2026-08-04, log 102257)
§398e dump verdict: at the stuck exit the overlap had ALREADY advanced to WaitOfFadeout
(peek hold — its cover latch WAS set) but bf=0x0 — the one-shot CONSUMED, and the exit's
scene request never advanced from ph1 → the eater is another IsDoneOverlap waiter. Only one
legitimate consumer exists (scene fadeFase IsDoneOverlap) but it appears at [1] AND [4]:
hypothesis = a LEFTOVER transition request (the entry's, parked at [4] wait-for-reopen)
coexists in the queue and eats the exit's cover latch. §398f: the consumer now logs itself
(req ptr + name + phase) on every successful consume — one repro names the eater and whether
a stale request lingers from the entry.

## §402 BLACK-SCREEN ROOT + CHURN BREAKER (2026-08-04, log 102730)
FULL CHAIN, receipts end to end (§398 probe family's find):
 1. §398f/e re-read with correct field mapping (q ph = NODE-request phase, not fadeFase):
    the exit's request consumed its own cover latch fine, advanced to Execute, and PARKED AT
    THE NODE DELETE PHASE (q ph1 = fpcNdRq Delete) — the play-scene delete never completes.
 2. WHY: the interior census churn — 251 №94 re-runs / 614 fpcNm_EXT_VEG_e creates in one
    session ("spawned 2 → live=0 → re-run" every ~15f: the §394-wired swood never registers
    live in the interior). A perpetual stream of in-flight creations = the delete phase
    waits for creation drain forever → wipe holds at peek → BLACK SCREEN.
 3. Exit-only because only 'EXT_BG1' churns; exterior census registers live fine. Also
    explains the tale's black screen (tale re-enters FROM the interior — same starving
    delete), retroactively closing the §398 chain.
FIX: §402 churn breaker — 3 heal attempts per BG then stand down + warn (a heal that loops
isn't healing). ROOT (ferried next bus push): swood/EXT_VEG interior registration —
vegetation-lane surface (d_tree packet likely absent in the room-lane context).
§398f consume-log field note: prints node-request phase; consumes at node-ph 0 = fadeFase[1],
node-ph 5 = fadeFase[4] — the "second request" 0x...0890 was the exit's own (allocator
address variance), NOT a duplicate; the IsIng/IsPossibleTarget guards held.

## §403 CYCLE CRASH SYMBOLICATED (2026-08-04, log 103425) — the ledgered Ba1 setMtx crash
§402 CONFIRMED: transitions clean for multiple enter/exit cycles (black-screen chain CLOSED).
New failure after ~several cycles: arrival-camera sluggishness (door refocus post-fade), then
EXCEPTION_ACCESS_VIOLATION. Symbolicated (llvm-symbolizer, standing rule):
  daNpc_Ba1_c::_create → createInit (d_a_npc_ba1.cpp:330) → setMtx (:354) →
  mDoExt_McaMorf::calc (m_Do_ext.cpp:1420) → J3DJoint calcTransform → J3DGetTranslateRotateMtx
THE KNOWN Grandma setMtx crash (pending-relay ledger). Reproduction = interior re-entry
cycling: Ba1 re-creates per load; the session-lived model-data cache (mount acquireModelData,
publish-on-success) retains parsed J3D pointers across arc purge/remount cycles — the
J3D-pointer-fixed bug class ("never free an arc while parsed data is cached; audit
session-lived caches"). Fresh cache = first cycles clean; recycled storage = Nth create walks
freed joint data. RELAYED to Housing (bus §397+) with the full frame list — their active J3D
surface; the cache-invalidation-on-remount audit is the fix shape.

## §403b CORRECTION + CLOSURE (2026-08-04, Housing's autopsy via user)
The Ba1 setMtx cycle crash is SOLVED by Housing — root was a NAME-MAPPING bug, NOT the
stale-cache class my §403 relay attributed (correction on the record; the pointer-fix
registry keeps its precedents, but this crash was never a member). Downstream consequence
(Housing's own deliberation): the mount-time BDL-parsing migration loses its main safety
argument; remaining value = verbatim donor porting + deleting receiver-invented apparatus
(cache/pristine stash/keep-alive/purge/retain-release/№100 guard); CRITICAL PATH = the WW
lighting port (normalize_litmask 0x03→0x01 + normalize_tevregs→white adapters must become
honored values before the 68-arc restage, else every WW model darkens at once). Housing owns
it and sequenced it first. HISTORY IMPACT: extended play sessions UNBLOCKED (crash fixed);
the door campaign's stable point stands; §401 native-arrival remains History's next.

## §404 WW LIGHTING PORT (2026-08-04, user order — Housing's critical path, taken by History)
THE FINDING THAT REFRAMES THE ADAPTERS: the receiver's setLightTevColorType(J3DModel*,…) is
an EMPTY STUB — every WW-mounted model has been drawing with ZERO per-frame lighting writes;
staging's normalize_litmask/normalize_tevregs were static imitations of the donor's DYNAMIC
writes. Donor spec (WW DP d_kankyo.cpp:1763-1873): setLightTevColorType_sub rewrites each
material's chan light mask per frame (clamp mask→1; enable slot 1 only when mColorK1 live,
fed via TevKColor(1)+extra TEV stage) AND overwrites TEV C0/K0 with the tevstr's live colors
(the shipped 128-gray is a placeholder the donor never draws) + light obj slot 0 + fog.
LANDED: src/d/d_kankyo_ww.cpp (+header) donor-verbatim with labeled seams: [S1] field renames
(mColorC0→TevColor, mColorK0→TevKColor, mFogColor→FogCol — TP kept WW's tevstr layout);
[S2] no K1 source yet → donor's own else-leg disables the extra second-light stage (plight/
lamp K1 wire = follow-up); [S3] toon off (WW retail); [S4] mInitType guard skipped. Receiver
API seams: direct color fields (no .mColor), getLightInfo() ptr, view near_/far_, mXFogTbl.
17 WW TUs switched from the stub to dKyWw_setLightTevColorType (esa kamome kb knob00 lamp
mirror ba1 bm1 ls1 zl1 mshokki otble toripost spc_item01 swhit0 ww_demo00 + d_door). Mount
BG/sea paths untouched this build (the §143 sea palette hand-map independently confirms the
non-toon C0/K0 mapping). SAFE against currently-normalized arcs: mask write is idempotent
(1→1) and C0/K0 overwrite replaces the whitened placeholder with live colors.
NEXT: visual verify → THEN Housing's 68-arc restage without the adapters; K1/plight wire;
mount-BG draw extension.

## §405–§416 SKY CAMPAIGN CONSOLIDATED (2026-08-04, Housing Security)
§405-§407 tevstr feeder chain (committed a6a9ea943a): TP never writes tevstr TevColor/
TevKColor on the actor path — dKyWw_settingTevStruct now feeds all 17 WW TUs with donor-
authored types; K1/plight/eflight feeders live ([S9]-[S13]); tevstr grew mWwColorK1
(PC-only append, sizeof-safe). §408 sea + §409 island BG cut over to the native chain.
§410 dome fidelity (uso_umi register, wind-driven cloud scroll, invisibility gate).
§411-§415 THE SKY RESURRECTION — root-cause chain, each link probe-convicted (414-P1..P11):
 (1) hide_vrbox (№108) over-suppressed the entire celestial layer → dKyWw_setSkyHost flag;
 (2) wether latches host resources at frame 1, BEFORE the mount loads WW arcs; vrkumo
     hard-fails PERMANENTLY (status 99) if cloudtx not resident → re-init on flag flip +
     retry-not-latch under skyHost;
 (3) THE MASTER GATE: status bit 1 is set ONLY by daVrbox_Create on sky-model success —
     "this stage HAS a sky." Mount suppressed TP's vrbox → bit clear → checkStatus(1)=0
     killed sun/moon/stars/vrkumo, donor-correctly. §415 bridge: setSkyHost asserts the
     vrbox contract (onStatus(1)) until the native vrbox actors land (§417 next build).
§413 WW CELESTIAL TU (d_kankyo_ww_sky.cpp): donor-verbatim sun/moon-phases(save-day,
mirror trick, dayscale)/lensflare(8-elem + 16-tri burst)/starfield(16 hokuto + procedural
spiral, pure vertex color) from WwAlways IDs 0x81-0x8A; dispatched over TP's systems on
skyHost only. USER-CONFIRMED: sun visible.
§416 DONOR WIND (placeholder retired): WW default = tact-never-conducted → vec (1,0,0);
power = authored FILI GlobalWindLevel — Winditor-law receipt sea/Room44.arc param=0x0 →
level 0 → 0.3. Foam-era ambient (0.4, +Z) was wrong on both axes.
OPEN: sun peekZ viz stuck 0 (lens flare starved — occlusion never confirms; PC GXPeekZ
suspect); vrkumo shade/cluster (see §417); moya = kytag placement census (History lane).

## §417 HISTORY ASSIGNMENT (user-ruled 2026-08-04): decode WW vrkumo_move + drawVrkumo
The cumulus BEHAVIOR is the last non-donor code in the sky: WW DP's vrkumo_move
(d_kankyo_rain.cpp:1759, original 0x918 bytes @ 800937BC-800940D4) and drawVrkumo
(:3579, @ 800940D4+) are /* Nonmatching */ EMPTY STUBS — the receiver runs TP's
6-cloud-tuned evolution, which at WW count 50 produces the observed clustered/pulsing
mass (angular quads ×(1+16·elev³) balloon overhead; drift barely perceptible).
DELIVERABLE: decompiled-or-equivalent bodies of both functions (per-cloud spawn
distribution, wind drift application, alpha/size lifecycle, draw geometry + color use of
mVrkumoColor/mVrkumoCenterColor incl. CenterColor which TP dropped). Debug linker maps:
D:\XXXXXXX\WW Debug maps (372 per-actor tables) — d_kankyo_rain symbols give layout;
noclip d_kankyo_wether.ts vrkumo is the working reference reconstruction (reference-tier
ONLY per user ruling — Winditor/binary is law). Receiver swap point is exactly two
functions behind dKyWw_isSkyHost(); TP stages untouched. DOES NOT BLOCK §418 (native
vrbox actors) — cumulus keeps TP behavior until the decode lands, then drop-in.
WHOSE TURN: History (decode) · Housing (§418 native vrbox port, in progress) · user
(testing 15:32:01 build: cloud drift due +X @0.3, P11 per-cloud dumps).

## §417b HISTORY DELIVERY (2026-08-04): the vrkumo decode is DONE — ready-to-include bodies
Deliverable: src/d/d_kankyo_ww_vrkumo.inc — donor-equivalent bodies of BOTH stubs, carrying
every constant, with the evidence tiers and four decode seams ([D1] bounce timer has no
donor packet slot → TU-static; [D2] drawVrkumo's single GXColor& arg = horizon color, the
CENTER color reads from env light inside the body — receiver field mWwVrkumoCenterColor,
fed from the Winditor-LAWED Virt slot (EntityTemplates.cs:2346: …HorizonCloudColor RGBA,
CenterCloudColor RGBA, Sky/FalseSea/Horizon RGB — CenterCloudColor is authored stage data);
[D3] dome = camera-pos + (farPlane-10000)·dir, donor's Mtx arg equivalent; [D4] VRKUMO_EFF
0x10-0x1C untouched by behavior — suspected drawVrkumokage-only).
THE BEHAVIOR SPEC in brief: spawn = polar scatter (angle 0..2π, dist rnd 18000 clamped
14000+rnd 1000), speed 0.5+rnd 4, height 0.3·rndX 0.3; move = §416 wind drift (visible:
strengthVel·falloff·speed with strengthVel=4+strength·4.3; invisible: rushed, index-staggered),
edge recycle at 15000 (mirror ≤15100 / rescatter), height ladder 500·(i/100)+skyboxOffs+
(3000-1000·strength)·centerCubic, falloff=1-dist⁶, alpha bands (rim fade-in 0.05-0.2,
storm dim ·(1-0.55·strength), over-count bleed 0.005/f) + OVERHEAD FADE (centerCubic
0.88→0.98 → 0) — the anti-balloon clamps: normalPitch=min(polar/1.9,1)³ with widen
(1+16·np) top / (1+2·np) bottom, polar clamp 1.21, and the 3-layer/4-class scatter table.
BONUS LEDGER: debug map exposes drawVrkumokage (0xa18) — the cloud SHADOW pass, third
dropped function; not decoded this round.
WHOSE TURN: Housing → include from the sky TU at the dKyWw_isSkyHost() swap point (§418
build); wire mWwVrkumoCenterColor from the §407 palette conversion; drawVrkumokage optional
follow-up. History → §401 native arrival remains queued. USER → nothing owed on this item.

## §417c HOUSING WIRING RECEIPT (2026-08-04): History's vrkumo decode is LIVE
d_kankyo_ww_vrkumo.inc included in d_kankyo_rain.cpp behind dKyWw_isSkyHost() at exactly
the two-function swap point; receiver seams labeled in-place ([R1] mInst→mVrkumoEff,
[R2] wind accessor). [D2] CENTER COLOR RESTORED: №113 already carried the Winditor-lawed
CenterCloudColor in stage_vrboxcol kumo_bottom_col (converter :152) — the №144 feed now
blends it into g_env_light.vrbox_kumo_bottom_col and the draw lerps center↔horizon by
distFalloff (TP dropped the consumer, not the data). Draw body written per the decode
spec: 3 texture layers back-to-front, RASC vertex-color TEV (C1=black), dome radius
far−10000, no-Z-write. [D1] bounce timer = rain-TU static. drawVrkumokage stays ledgered.
Exe 15:48:32, gate clean. Expected on test: scattered disc, §416-wind drift, rim
mirror-recycle, overhead fade + 1.21 clamp (the anti-balloon terms), authored center
colouring. P3/P11 probes still armed for verification.
WHOSE TURN: user (test) · Housing (milestone commit §405–§418 on verify; then peekZ/
lens-flare follow-up) · History (idle unless P11 disagrees with the decode).
