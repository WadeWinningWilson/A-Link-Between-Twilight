# Ladder / exterior interaction — research findings

**Pass type:** research (user-requested, 2026-07-24). Findings only, no fix built.
**Lane:** the fix is Link-acch STATE (the room-lane transition), NOT BG
registration — Cursor's registration is proven correct by the probe (§0).

## §0 — CONFIRMED CAUSE (§63+§69 probe, 2026-07-24 run)

**`FLAG_WALL_NONE` (bit 2) is SET on Link's `mLinkAcch` at the interior ladder.**
That flag makes `dBgS_Acch::CrrPos` skip the entire wall solve
(`d_bg_s_acch.cpp:368`), so `ChkWallHit` can never go true → the ladder gate
never fires. `wallCode=4` still reads because `setFrontWallType` runs its own
independent `LineCross`, not gated by the acch flag.

Probe evidence:
- Interior ladder (broken): `acchFlags=0x4120e4` → `WALL_NONE`(0x4) SET,
  `WALL_HIT`(0x10) clear, `LINE_CHECK`(0x2000) still ON, `LINE_CHECK_NONE`(0x4000) clear.
- Healthy exterior: `acchFlags=0x4020f0` → `WALL_NONE` clear, `WALL_HIT` set.
- The ladder BG itself is PERFECT: `prio=0 used=1 moveBg=0`, and the log shows
  `№256 BG GLOBAL_e PRIORITY_0 (identity) proc='EXT_BG1' arc='LinkRM'`. So BG
  registration (G1/G2/G3 below) is ruled OUT by direct measurement.
- Link is in the normal standing proc (`proc=4`, same as the healthy exterior)
  yet `WALL_NONE` persists → it is a STUCK flag, set earlier and never cleared.

**Mechanism:** in vanilla WW the door-open proc that sets this flag
(`procDoorOpenInit`, `d_a_alink_demo.inc:2698`) clears it when the open anim
completes (`procDoorOpen` → `ClrWallNone`, `:2737`). Our custom room-lane
transition teleports Link into the interior BEFORE that clear runs;
`forceLinkGroundReprobe` (`d_ext_npc_mount.cpp:4263`) clears the ground hit but
NOT `WALL_NONE`. The progressive exterior break is the same stuck flag following
Link back out. (Flag signature caveat: the interior shows `WALL_NONE` set with
`LINE_CHECK` still ON and `LINE_CHECK_NONE` OFF — which the door setter's
`OnLineCheckNone` does not match, so either the line flag was re-enabled
separately after, or a non-door setter is involved; the FIX — clear `WALL_NONE`
on transition — holds regardless of which setter left it stuck.)

**Fix locus:** extend the room-lane transition's Link reprobe to also
`ClrWallNone()` (and normalize the line-check flags), restoring the state the
interrupted door proc would have cleared. Link-state lane, not BG registration.

**CONFIRMED + REFINED (2026-07-24 test, build v1.4.1-153, no fix shipped yet):**
- Interior ladder BROKEN on BOTH donor interiors (Link's house roomId=255 AND
  Sturgeon's roomId=2), both `acchFlags=0x4120e4` (`WALL_NONE` set) — systematic,
  the shared interior-ENTRY transition, not per-ladder geometry.
- Exterior post-return `acchFlags=0x4020f0` (`WALL_NONE` clear) — the exterior
  now works NOT from a fix but because `WALL_NONE` clears once Link is back
  outside opening normal doors; the interior is where it stays stuck.
- **Exact fix:** `forceLinkGroundReprobe` (`d_ext_npc_mount.cpp:4263`) is ALREADY
  called on the entry transition (`:4905`) AND exit (`:4950`) — but it only
  `ClrGroundHit()`+`CrrPos()`, never `ClrWallNone()`. Add `mLinkAcch.ClrWallNone()`
  (and re-enable line check) there. Because it's already on both transition
  paths, this single addition fixes interior entry AND belt-and-suspenders exit.
  Donor-faithful: it restores the state `procDoorOpen`'s `ClrWallNone` (`:2737`)
  would have set had our transition not preempted the door anim by teleporting.

---

**Runtime facts under study (from prior §63 runs):**
- Interior ladder: `GetWallCode()==4` (readable) BUT `mLinkAcch.ChkWallHit()==0` → Link never grabs.
- Exterior works at spawn; after an interior visit + return, exterior interactability dies too (progressive).
- Walkthrough CAVE works throughout (plane transition — bypasses the event manager and, notably, is a different collision path).

---

## 1. What is DEFINITIVELY RULED OUT

### (a) Room ID is NOT the collision cause. PROVEN.
The entire wall/ground solve (`dBgS::WallCorrect`, `src/d/d_bg_s.cpp:1171-1213`;
`GroundCross :298-314`; `LineCross :279-296`) has **no room-id term**. A
registered BG is tested against Link iff: `ChkUsed()` + `!ChkNotReady()` +
`ChkPriority(0|1|2)` + not-self + move-bg gate. `m_roomId`/`GetGrpRoomId` are
**report-only** (used *after* a hit to name the room). So Cursor's №256/№265
room work was correct and necessary — **but for intro + doors (event
resolution), NOT for ladders.** The ladder fix is elsewhere.

### (b) The alink ladder gate is FAITHFUL to WW. Do not touch it.
Receiver `daAlink_c::setFrontWallType` (`d_a_alink.cpp:9162`, gate `:9396`,
ladder entry `:9472`) is byte-for-byte the donor's `daPy_lk_c::setFrontWallType`
(`WW DP/src/d/actor/d_a_player_main.cpp:4552`, gate `:4558`, entry `:4659`):
both require `mAcch.ChkWallHit()` true before the wall-code can select the
ladder. `GetWallCode` (a **line-check** attribute read) and `ChkWallHit` (the
**cylinder** solve) are independent subsystems — that is WHY code-readable +
hit-absent is even possible, and it is by design. **Never relax the ChkWallHit
gate** (the existing №256 comment at `d_a_alink.cpp:9333-9395` already says so).

### (c) ~~Stuck acch wall-disable state is RULED OUT.~~ ← THIS WAS WRONG (see §0).
**CORRECTION (probe, 2026-07-24):** this reasoning was flawed and the probe
disproved it — `FLAG_WALL_NONE` IS the cause (§0). The two errors: (1) I assumed
every wall-none setter also disables the line check, but the interior shows
`WALL_NONE` set with `LINE_CHECK` still ON; (2) I assumed `setFrontWallType`'s
`wallCode` read depends on the acch line flag — it does NOT, it line-casts
independently, so `wallCode=4` coexists with a disabled wall solve. Original
(wrong) reasoning kept below for the record:
`FLAG_WALL_NONE` (bit 2) skips the whole wall solve (`d_bg_s_acch.cpp:368`) and
would explain hit-absent — BUT every `SetWallNone` caller in the alink family
(`d_a_alink_demo.inc:1870` procCoOpenTreasureInit, `:2698` procDoorOpenInit,
`d_a_alink_damage.inc:1310`) **also disables the line check** (`OffLineCheck`/
`OnLineCheckNone`). A stuck wall-none would therefore force `wallCode=0` too.
The broken ladder reads `wallCode=4`, so the line check is alive ⇒ Link's acch
is in normal mode ⇒ not a stuck flag. `FLAG_MOVE_BG_ONLY` (bit 20) is set
nowhere on Link. **The wall solve IS running.**

---

## 2. What the mechanism MUST be (narrowed, not yet disambiguated)

**Key deduction:** `wallCode=4` readable ⇒ `LineCross` found the ladder poly ⇒
the interior ladder BG **is registered (`ChkUsed`) and ready (`!ChkNotReady`)**.
So the BG is in the collision table. The *only* things that make a
registered+ready BG visible to `LineCross` but invisible to `WallCorrect`:

| # | Cause | Why it fits | Discriminator |
|---|---|---|---|
| **G1** | **Priority outside [0,2].** `WallCorrect` iterates 3 passes gated on `ChkPriority(0\|1\|2)` (`d_bg_s.cpp:1189`); `LineCross` has NO priority filter (`:279`). A BG at priority >2 is line-visible, wall-invisible. | GLOBAL_e path sets PRIORITY_0 ✓, MOVE_BG path leaves default 2 ✓ — both in range, so this needs the interior to have taken NEITHER clean branch, or a priority corruption. | log the interior BG's `m_priority` after Regist; grep existing log for whether `EXT_BG1` printed the `№256 BG GLOBAL_e PRIORITY_0` line (if absent → it fell to MOVE_BG or failed the branch). |
| **G2** | **Wall-cylinder geometry.** Line probes 25+wallR *ahead*; wall solve needs Link's cylinder (r≈35, `d_a_alink.cpp:5011-5013`) to physically overlap the poly. A thin/recessed WW ladder poly can be line-reachable but cylinder-missed. | WW ladders are often recessed; explains interior-only. | log wall-cylinder centre/radius vs the resolved poly distance at §63. |
| **G3** | **Group/poly pass-through.** `WallCorrect` honors `ChkGrpThrough`/`ChkPolyThrough` (`d_bg_w.cpp:1511/1557`) with the acch's wall pass-check; `LineCross` uses its own pass config. A "through" attr on the interior ladder group suppresses the wall pass only. | explains interior-only, registration-independent | log the resolved poly's grp/poly pass attribute. |

**G1 is the leading single candidate** because it is the one cause that is
purely a registration property (Cursor's lane) rather than authored dzb
geometry — and because the exterior (same GLOBAL_e PRIORITY_0 path) works,
implicating a divergence in how the *interior* registers.

---

## 3. The PROGRESSIVE exterior break (separate finding)

Since stuck acch state is ruled out (§1c), the exterior dying **after** an
interior round-trip cannot be a Link flag — it must be **BG-table disturbance**:
the exterior EXT_BG0's slot/priority/ready-state is changed by the interior
load/unload sharing the `dComIfG_Bgsp()` 256-slot table. Prime suspects, all in
`d_ext_npc_mount.cpp`:
- The №257 stub-release path (`:6930-6940`): on a re-create, `prev = getBgW(room)`
  then `Release(prev)` if `prev->ChkUsed()` — a stale/mismatched slot pointer
  here could Release or orphan the exterior BG.
- `releaseRoomLaneMount` (`:4227-4260`) Releasing a BgW whose slot the exterior
  still references, or the exterior not being re-`Regist`'d after the return.
- Priority not re-applied on the exterior's re-Regist after the round-trip
  (ties back to G1).

**One probe settles both halves:** at the §63 site, additionally dump — for the
BG that `LineCross` resolved — `m_priority`, `ChkUsed`, `ChkMoveBg`, and
`mLinkAcch.m_flags`. On the broken interior ladder AND on the broken-exterior:
- `m_priority ∉ {0,1,2}` → **G1** (registration priority) — the fix.
- priority fine, cylinder misses poly → **G2** (dzb geometry — authored asset).
- priority fine, poly through-flag set → **G3** (dzb attribute).
- `m_flags` shows WALL_NONE/MOVE_BG_ONLY (contradicts §1c) → re-open state family.

This is a **one-line diagnostic** (extend the existing §63 print), not a fix —
deferred to Cursor per the no-edit scope of this pass.

---

## 4. Handoff

- **Do NOT** touch the alink ladder gate or relax `ChkWallHit` (§1b).
- **Do NOT** pursue room-id for ladders (§1a).
- **Cursor (BG lane):** extend §63 to print `m_priority`/`ChkUsed`/`ChkMoveBg`
  for the LineCross-resolved BG + `mLinkAcch.m_flags`; run the
  spawn→interior→ladder→exit→exterior sequence; the priority field routes it to
  G1 (registration — most likely) vs G2/G3 (authored dzb). The exterior-break
  half points at the `Bgsp` slot lifecycle across the room-lane round-trip
  (`d_ext_npc_mount.cpp:4227, 6930`).
- Full agent transcripts' key file:lines are inlined above.

---

## §158 ADDENDUM (Housing, 2026-07-26) — the "re-setter" was a PHANTOM; root = stale №237 path assumption

The §0 fix (№269 ClrWallNone in `forceLinkGroundReprobe`) shipped on all THREE call sites — but all
three are ROOM-LANE paths (`d_ext_npc_mount.cpp:5198/5255/6781`). **The build evolved to NATIVE
stage transports for interiors** (doors → `dComIfGp_setNextStage` to R_DL01,
`d_ext_npc_doors.cpp:326` — log: `transport=stage host='R_DL01' … native setNextStage only`), and
**that path calls NO reprobe/ClrWallNone.** So the entry clear never runs on the path Link actually
takes: `procDoorOpenInit` sets WALL_NONE → native transport interrupts before the door proc's own
clear → stuck at the ladder. No second setter exists; the fix was on a superseded path.

Same stale assumption (merge_event.py's №237 "interiors are BG mounts, no stage switch") also sent
Ba1_Get_Itm's merge to the wrong stage's event_list (F_DL01 vs R_DL01) — History's find. One stale
doctrine, both interior bugs. **FIX (Engine, pending):** invoke the №269 clear on native-transport
arrival into WW interiors (doors path / R_DL01 arrival hook). Full record: bus §158; §121b/§121c
for the earlier passes. §2's G1-G3 candidates and §3's probe remain valid background but are NOT
needed if the relocated clear fixes the climb.

## §160 ADDENDUM (Housing, 2026-07-26) — relocation LANDED but fires ONE FRAME TOO EARLY

Engine shipped the relocated clear (`d_ext_npc_doors.cpp:1378`, in `pollArrival`) — user retest:
ladder still dead. Run-log `dusklight-20260726-154808.log` refines the root:
- R_DL01 arrival shows NO №269 line → WALL_NONE was NOT set on the first poll frame (the clear
  ran, found nothing — the №269 log prints only when the flag was up). So the flag is NOT carried
  across the transport (player is rebuilt); the setter is on the ARRIVAL side.
- Exit back to F_DL01 logs `№269 ClrWallNone (acchFlags 0x406024 → 0x402000)` → the flag WAS set
  during the interior stay. Setter = our own arrival door demo: `:1378` clear runs FIRST, then
  `beginDoorDemoLock()` (`:1457`) → `changeDemoMode(DEMO_DOOR_OPEN)` sets WALL_NONE;
  `endDoorDemoLock`'s `cancelOriginalDemo` bypasses the proc's own clear → stuck.
- **FERRY G-2 (Engine):** add `dExtNpcMount_forceLinkGroundReprobe(player);` in the demo-completion
  block after `s_arrival.demoEnded = true;` (`d_ext_npc_doors.cpp:~1472`). Keep the `:1378` call.
  Self-verifying: №269 must log AFTER `№89 arrival demo END stage='R_DL01'` on next entry.
Full record: bus §160 item 2.

## ✅ RESOLVED (§163, 2026-07-26) — G-3 flag-only clear; thread CLOSED

G-2 (full `forceLinkGroundReprobe` post-demo) cleared the flags but its `ClrGroundHit()+CrrPos()`
reposition FROZE Link in interiors (bus §161 three-run differential). **G-3 = flag-only clear**
(`ClrWallNone` + `OffLineCheckNone`, no reprobe) at the same post-demo site — freeze cured, ladder
climbs (user-verified on the loft, run 171351: `§161 flag-only ClrWallNone (0x4060e4 → 0x4020e0)`).
Final shape: `:1378` pre-demo full reprobe (carried flags) + post-demo FLAG-ONLY clear (demo-set
flags). Root chain for the record: №237 stale "BG mount" assumption → fix shipped on room-lane
paths Link never takes (§158) → relocated but pre-demo (§160) → post-demo but with reposition
(§161) → flag-only (§163). Four passes; the discriminating instrument each time was the run log.
