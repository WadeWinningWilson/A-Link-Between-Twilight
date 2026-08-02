# Stage-data bake pitfalls — CANONICAL (Foundry §281)

> **Read BEFORE writing any tool that edits stage/room/event data** (dzs, dzr,
> event_list.dat, RARC members). Every pitfall below was hit and receipted during
> the Grandma-tale campaign (§273–§280). The user's standing order: these are
> documented so NO later implementation (dungeons, n-event interiors, other
> islands) repeats them. Companion: History's
> [ww-interior-host-pitfalls.md](../state/ww-interior-host-pitfalls.md)
> (runtime/architecture side); this doc is the DATA-TOOLING side.

## P1 — NEVER re-layout a dzs/dzr that contains pointered chunks  ⚠️ §277, the worst one
**What happened:** revt_bake v1 rebuilt the stage.dzs layout wholesale. RTBL carries
**three levels of absolute file offsets** (`roomRead_class` → entry ptrs → room
lists, d_stage.cpp:2318). Relayout invalidated them → room0 count read 128 garbage →
event-mode entry loaded phantom rooms (compounded P3). History's A/B receipt:
same probe, two arcs, count=1 vs count=128.

**Law:** *append offset-stable or edit in place — never re-layout.* The v2 pattern:
relocate ONLY one known pointer-free chunk (STAG) to EOF to free front space for the
grown chunk table; every other byte keeps its original offset; pointered chunks stay
valid BY CONSTRUCTION. Safety-stop if the lowest chunk is not the one your safety
argument covers. (Same family as the J3D pointer-fix law: buffers with baked offsets
don't tolerate relocation.)

**Chunk pointer census (grow as learned):** POINTERED: RTBL (3-level), RCAM
(unverified — treat as pointered). SELF-CONTAINED (safe to relocate/copy):
STAG, MULT, SCLS, ACTR/ACTx, SCOB/SCOx, TGOB, TGSC, DOOR, TGDR, PLYR, EVNT/REVT,
Env0/Col0/PAL0/VRB0 (index-linked, not offset-linked).

**Open debt:** `bake_room_chunks.py` (§267) still uses v1-style rebuild for room.dzr —
it MUST get the offset-stable refit before its `--write` ever runs (flagged §277).

## P2 — event_list.dat is the EXCEPTION: rebuild is safe there
merge_event's wholesale rebuild works because **event_list cross-references are
INDICES, not file offsets** (№149/№151 — the shared container's design). Do not
generalize P1 fear to it, and do not generalize event_list's freedom to dzs/dzr.
Know which reference model a file uses before choosing edit strategy.

## P3 — donor event/stage data carries 1-room assumptions into a multi-room host  ⚠️ §275/§276
Three sub-classes, all from "donor-verbatim is not host-verbatim":
- **TYPE_ALL staff** (donor TALE_DEMO): inert scaffolding in the donor's one-room
  LinkRM; in a 6-room host it flips event mode to ALL-ROOM load → overlap pile-up.
  Scope it out on merge (`scope_event_staff.py`); planned: merge_event auto-flags
  ALL/cross-room staff when the target host is multi-room.
- **MULT stitched group**: the host shell authored a 6-room group at overlapping
  coordinates; gameplay ignores it, event-mode entry LOADS it. Trim to co-renderable
  rooms only (`mult_trim.py`). Rule: *a hosted stage's MULT group must contain only
  rooms that may co-render.* **§300 REFINEMENT — trim is not universal: MIRROR THE
  DONOR'S OWN STITCH GROUPS.** Donor Ojhous/Ojhous2 deliberately stitch rooms 0+1 at
  one origin (the Orca/Sturgeon house is ONE open volume, two floors, separate
  exterior doors). Blanket-trimming the host lost that native co-render (floors show
  a void). Correct form: the host MULT = the donor's groups translated to host room
  numbers ({2,3} for the Ojhous2 pair), gated on a geometry verification that the
  baked rooms preserve donor-local Y so floors STACK, not overlap. LinkRM's trim to
  room 0 stays correct — its donor IS 1-room.
- **SCLS room numbers**: donor room numbers are donor-stage-relative; a shared host
  needs the room-within-host map before SCLS bake (§267's [ROOM-MAP NEEDED]).

## P4 — REVT id/index duality + low-id probe collisions  ⚠️ §276
TP forces `field_0x4 == the entry's own index` (`setStartDemo` stores the id that
`getEventName` uses as a DIRECT index, d_event_manager.cpp:122). So entry ids CANNOT
be moved out of collision range, and the probe surface is real (`isStageEvent(param_0)`
matches ANY entry by field_0x4 — door-camera probes hit id 0 by construction). **Fix
the CONSEQUENCE, not the id** (P3's MULT trim makes the tripped mode harmless).
Priority byte + unknown fields: no TP REVT sample existed offline — [PORT-INTEGRATION]
flags stay until verified against a real TP stage's REVT.

## P5 — an ordered event with no PLAYER is a frame-0 idle  ⚠️ §280
**What happened:** `tale_1` merged donor-verbatim = staff {Ba1, CAMERA, Link} — NO
PACKAGE staff, because in the DONOR the stb is played by the separate TALE_DEMO
entrance event (two-step: actor event → stage re-entry → PACKAGE event). The port's
flow orders tale_1 directly; nothing loads the STB (`dDemo_c::start` fires ONLY from
a PACKAGE PLAY cut — History's 10-hypothesis probe, single-run proof). Result: event
runs, demo idles at frame 0, no fade.

**Law:** *every event chain must contain exactly one STB player (a PACKAGE staff with
PLAY→FileName) somewhere on its path, and the bake must VERIFY the chain end-to-end:
order → PACKAGE PLAY → FileName resolvable in the resident demo arc.* A merged event
that relies on a donor two-step must either keep the second step reachable or absorb
the PACKAGE staff (host adaptation, §281 — donor two-step remains the reference; if
behavioral deltas surface, e.g. reload-reset state, revert to the re-entrance chain).

### P5b — a collapsed event inherits the donor's FINISH accounting  ⚠️ §286/§287
**What happened:** with the PACKAGE staff absorbed (P5), the tale PLAYED end-to-end but
never TORE DOWN — `finishCheck()` (d_event_data.cpp:380) awaits `event.mFlags[3]`, and
tale_1's donor value (1701 = Ba1's WAIT cut) was authored for the donor's RELOAD teardown
(`setNextStage` mid-event destroys the stage; the event never finishes in place). Working
invariant across every finishing event: **`mFlags[0]` == the terminal cut flagId of the
staff that carries the performance** (TALE_DEMO→9 PACKAGE WAIT; Ba1_Get_Itm→56 Link's
last; tale_2→1712 its PLAY). **Law:** when a bake absorbs/collapses a donor multi-step
event, re-point `mFlags` at the absorbed player's terminal cut flag (mirror a working
event's accounting — never invent flag ids; cross-event flagId duplication is harmless
since flags reset per event init and only one event runs). Tool:
`set_event_finish_flag.py` (§287).

**§288 refinement — diff the FULL STAFF ROSTER, not fields:** the fade and the finish
were ONE missing-machinery root. The working reference (awake) finishes through its
CAMERA staff's `PAUSE(Stay=1) → STBWAIT` chain — the STB camera handover + fade AND the
second finish condition (`mFlags` = (PACKAGE WAIT, CAMERA STBWAIT, -1) — TWO flags). A
collapsed event must inherit the reference's WHOLE performing roster. One non-verbatim
rewrite on clone: STBWAIT's wait-flag references the event's OWN PACKAGE PLAY flagId.
(No DIRECTOR staff exists in awake — its camera chain covers handover; awake has no
black-fade.) Tool: `complete_event_camera.py` (§288, cross-file staff-chain cloner).

**§290 — the BLACK FADE's native home is a DIRECTOR staff (FADE cuts).** Driven by
`specialProcDirector` → `mDoGph_gInf_c::fadeOut` (d_event_data.cpp:1037/1207): FADE cut
data = `Rate` float (+ = to black, ends at fadeRate≥1; − = reveal, ends at !isFade();
optional `Color` RGBA). Donor reference shapes: MK_GAMESTART `FADE(+.03 ungated) →
WAIT → FADE(−.02 gated) → WAIT` (the snap-hider open); NZ_ESA `±.05` (the end dip).
Composition rule for hosted events: gate each FADE on the event's OWN flags (PLAY /
package-WAIT / STBWAIT), and put the FINAL REVEAL's flagId into `mFlags` so teardown
waits for it — never end an event behind an un-revealed screen. Tool:
`add_director_staff.py` (§290; donor-verbatim rates; collision-scanned flagIds).

**§291 — KNOW WHAT EACH GATE FLAG MEANS ON THE TIMELINE.** Fade timing signals are
other staffs' CUT-COMPLETION flags — per-event data, nothing global. The receipts:
**PACKAGE PLAY's flagId fires at the storyboard's END** (specialProcPackage ends PLAY
at demo_mode()==2) — it is a FINISH signal, never a start gate. **CAMERA PAUSE's
flagId fires at the storyboard's START** (chain-order proof: STBWAIT is PAUSE's
successor and is active DURING playback, so PAUSE completed at the handoff). §290's
reveal was wrongly gated on PLAY (tale played under black, revealed at the finish);
§291 re-gated it to PAUSE. Rule: *gate opening reveals on a handoff/start flag;
gate end-dips and restores on completion flags.* Tool: `regate_reveal.py`.

**§292 — MATCH THE REFERENCE'S CONTEXT, and NEVER prescribe timing.** §290's opening
FADE was cloned from MK_GAMESTART — a TITLE start beginning from black, where ungated
is correct. Mid-gameplay cutscenes are structured differently, and the donor pattern
is unanimous (ARRIVAL_GND / DEFAULT_NPC_NZ_ESA / MEGAMI_DEMO): **the DIRECTOR chain
opens with an ungated WAIT** (idling through control lock + letterbox), and **the first
FADE gates on an early cut boundary of the scene's driving staff** — TIMEKEEP
COUNTDOWN, the NPC's TALK cut, a DUMMY cut — never a raw frame delay. Two laws:
(a) a reference must match the CONTEXT (title-start vs mid-gameplay), not just the
shape; (b) timing constants are the donor's to dictate — an eyeballed "delay N frames"
is always the wrong ask (the user's caveat, now standing). Tool:
`rework_opening_fade.py` (tale_1: leading WAIT + opening FADE gated on Ba1's
START_TALE1 completion — the donor's own mEvTimer-paced establishment beat).

## P6 — the demo arc is a single global slot (History's pitfall D, restated for bakers)
`getDemoArcName()` holds ONE arc. Any bake that introduces an event needing a
different demo arc than the space's current one must route through the ONE bundled
arc per space (LBNK-style), never a private arc. FileName strings in PLAY cuts must
resolve in THAT arc.

## Verification doctrine (what §277 proved out — apply to every bake)
1. **Offline probes BEFORE delivery** — mirror the runtime probe in the tool
   (§277: RTBL room0 count read from delivered bytes; §281: dump the event's full
   staff/cut/data chain post-edit). Acceptance criteria proven pre-playtest.
2. **Byte-diff the untouched region** — assert everything you didn't mean to change
   is byte-identical.
3. **Re-read with the same reader** — every tool verifies its output by re-parsing.
4. **Idempotency** — rerunning any bake must be a no-op (dedup/presence checks).
5. **One-time backups** with campaign-named suffixes; never overwrite an existing bak.
6. **Runtime discriminators are 10-hypothesis probes** (standing directive) — one
   build, one run, one verdict (§280's probe collapsed the multi-round guessing game).
