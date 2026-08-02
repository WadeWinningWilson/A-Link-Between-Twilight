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
