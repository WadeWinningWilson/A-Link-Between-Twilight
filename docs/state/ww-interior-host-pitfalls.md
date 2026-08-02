# WW story events in a TP interior — architecture + pitfalls (canonical)

> Reference for hosting Wind Waker story cutscenes inside a Twilight Princess
> host stage/room (the "WW-Crew-Restoration" pattern). Written from the Grandma
> `tale.stb` port (§263–§274); the failure modes here are **general**, not
> Grandma-specific. Read before adding a SECOND event to any hosted room.
>
> Companions: [grandma-native-tale.md](grandma-native-tale.md) (the live worklist),
> [../DO-NOT.md](../DO-NOT.md) (the hard-stop registry), and the WW Cutscene-NPC
> Playbook (cookbook recipes 10–15).

---

## 1. The architecture that actually scales

We are hosting WW story events in **one TP room space** (`R_DL01`, Grandma's
interior; WW `LinkRM` content). The load-bearing fact:

**TP's event manager runs exactly ONE event at a time.** `dComIfGp_getEventManager()`
has a single run-slot (`isOrderOK` / `getRunEventName` / `orderStartDemo`). So
multiple story events in one room **never run concurrently — they queue.** Events
do not fight *during* playback. Everything that breaks is in **selection** and
**residency**, not concurrency.

A single TP room *can* host many WW story events, but it demands four things:

1. **Every event gated to a mutually-exclusive trigger** — a story flag, a region/
   distance trigger, an A-press, a "not-yet-seen" bit. Exactly one event eligible at
   any moment. (Violating this is Pitfall A below — it is what crashed us.)
2. **Every event scoped to the one room.** No cross-room staff/camera. One roaming
   camera pan flips the stage into all-room event mode → every hosted interior loads
   and renders on top of the others (Pitfall B).
3. **One shared demo arc per space.** `dStage_roomControl_c::getDemoArcName()` is a
   **single global slot**. Bundle every `.stb` for the room into ONE demo archive
   (WW's LBNK room-bank pattern). Do NOT give each event its own arc (Pitfall D).
4. **The WW event-flag namespace** (still owed). Multi-event = story progression =
   WW event bits. Without a WW flag namespace, sequencing misfires (Pitfall E).

Items 1–3 are authoring discipline. **Item 4 is real engine work still owed** and is
the true ceiling between "one demo works" and "a room full of story beats works."

---

## 2. Pitfall catalog

### A. Ungated event fires on plain arrival → all-room load + OOM/freeze  ⚠️ CONFIRMED §274
**Symptom:** entering the interior loaded **all 6 rooms of `R_DL01` at once**,
re-decoded the arrival room 3×, and either crashed (heap OOM) or rendered every
interior overlapping in one space.

**Root:** Foundry's §272 **REVT bake** added a map-event to `STG_00.arc` (+96 bytes,
`19488→19584`). With the REVT present, the incoming **door-opening event** (`Knob00`
§27 staff=3) **continues into `R_DL01` as a full event-demo** instead of ending into
gameplay. Event-mode entry loads **every room** the event could touch (rooms 0–5),
and the arrival room (0) gets decoded by multiple `room_of_scene` procs.

**Diagnosis receipt:** the *only* diff between the clean 21:52 build and the broken
22:51 build was that +96-byte REVT chunk. `STG_00.arc.pre-revt-bak` (1 room, works) vs
`STG_00.arc.revt-6room-bak` (6 rooms, overlaps). Same door, same entry, same code.

**Lesson:** a REVT/event must be authored to fire **only on its specific trigger**
(the §273 pending-demo gate), NOT be reachable by a generic arrival/door event. An
ungated REVT turns every plain arrival into an all-room event-demo. This IS
requirement #1 above, and it is the exact failure you would see repeated, multiplied,
if several events go in ungated.

**REFINED MECHANISM (§275 — chunk analysis, decisive).** The overlap is `MULT` + REVT,
not the event staff. Per-arc DZS chunk entry counts:

| arc | MULT | RTBL | REVT | rooms loaded |
|-----|------|------|------|--------------|
| pre-revt (WORKS) | **6** | 6 | none | 1 |
| gated/scoped     | **6** | 6 | 2    | 6 (overlap) |

- `MULT` (multi-room stitch group, `dStage_Mult_info` = `{transX,transY,angle,roomNo}`)
  has **6 entries even in the working 1-room arc.** The stage always defines a 6-room
  stitched group whose entries sit at overlapping coords. **Gameplay entry ignores MULT
  and loads only the arrival room** — no overlap.
- Adding REVT (the ONLY chunk that changed) flips the stage into **event-mode entry**,
  which loads the **full MULT group** → all 6 rooms co-render → pile-up.
- The tale event does NOT fire (gate holds, no `§273`). The all-room load is at
  **stage-entry**, so staff-scoping and the trigger-gate are both irrelevant to it —
  the trigger is whatever engages event-mode on arrival (the donor **replay-switch,
  REVT id 0**, is the prime suspect).

**Two fixes were on the table; (2) is the only structurally-available one (§275 resolved
by Foundry):** (1) author the REVT with NO stage-entry/auto/replay event was
**unavailable** — TP forces a REVT id to equal its own array index (`setStartDemo` stores
the mapToolID that `getEventName` then uses as a direct index), so the tale entries MUST
be ids 0/1 and cannot be moved out of probe range. And the probe surface is real:
`isStageEvent(param_0)` matches any REVT entry by `field_0x4`, so a door-event camera
probing a low id hits **id 0 by construction**. The id side is unfixable as data — the
fix lives on the consequence side. (2) **trim the host MULT group to room 0** so even
when an entry-time probe trips event-mode, the group it loads contains only room 0 — the
pile-up cannot happen, whatever fires. The WW host uses only room 0 and future hosted
events are single-room, so trimming is safe.

**RULE (canonical):** *a hosted stage's MULT group must contain only rooms that may
co-render; a REVT-bearing host with an untrimmed MULT is one probe away from the §274
pile-up.* Shipped via `mult_trim.py` (6 → 1, re-read verified).

**Host-hardening sequence (repeatable for any dungeon-campaign hosted room):** on the
md5-verified clean base, in order — (a) `scope_event_staff.py` (strip `TYPE_ALL`/
cross-room staff, protects the event-RUN side), (b) `mult_trim.py` (MULT → only
co-renderable rooms, protects the entry/probe side), (c) REVT re-bake (ids 0/1, donor
replay-switch intact). Together these make an n-event dungeon just data: n REVT rows + n
flag-gated triggers + one bundled arc, on a host that cannot pile up.

**Diagnostic method (reusable):** read DZS chunk entry counts with the 4-byte big-endian
`count` right after each chunk tag; compare working vs broken arcs. `MULT=6` in the
working 1-room arc vs `REVT none→2` as the only delta is what isolated this.

**ACTUAL ROOT (§277, proven) — the bake RE-LAID-OUT the dzs and invalidated RTBL's baked
offsets.** Residency is decided ONLY by `loadRoom(RTBL[arrivalRoom], true)` looping the
arrival room's roomRead row (`d_stage.cpp:340`; MULT/REVT/events never create room procs —
so MULT-trim and staff-scope could never fix the load). A runtime probe (№93,
`d_stage.cpp` loadRoom) logging the row the engine actually reads gave the A/B proof:
- clean arc: `RTBL row arrival=0 count=1 bytes=[c0]` (correct: room 0, ChkBg).
- baked v1 arc: `count=128` with garbage bytes (roomRead structs read as room indices).

RTBL carries **three levels of absolute file offsets** (room table → entry pointers → room
lists). The v1 REVT bake **rebuilt the dzs layout**, which silently invalidated every one of
those offsets → room 0's row pointed into garbage → all rooms load (overlap) AND the corrupt
stay-room table tears room 0 back out (collision drops, Link falls). In-place editors (staff
scope, MULT trim) were always safe; the **relayout** was the landmine.

**STANDING LAW — never re-layout a `.dzs`/`.dzr` that contains pointered chunks. Append
offset-stable, or edit in place.** This is the same class as [[reference_j3d_pointer_fix]]
(J3D buffers with baked offsets don't tolerate relocation). The §277 v2 fix is *offset-stable
append*: relocate only the lowest self-contained pointer-free chunk (STAG) verbatim to EOF,
reuse the freed space for the grown REVT entry, leave every other byte at its original offset
so RTBL and all pointered chunks stay valid by construction (with a safety-stop if the lowest
chunk isn't STAG). Verify any REVT re-bake with the №93 probe: **room 0 must read `count=1`.**

**Current mitigation:** `STG_00.arc` reverted to `pre-revt-bak` (single-room, no
overlap, no crash). The tale therefore does not trigger (no REVT to order) — **freezes/
does-nothing**, which is the accepted state while Foundry re-bakes a **gated** REVT.

### B. Multi-room host stage re-inits the player once PER room → heap OOM  ⚠️ CONFIRMED §90/§91
When a host stage IS multi-room (or is forced multi-room by Pitfall A), each room runs
`dStage_playerInit`. Two traps compound:

- **№86 reconcile (REMOVED as №90):** an old belt forced `stageRoom → startRoom`
  whenever they differed and no player existed yet. It defeated the native guard
  (`startRoom != stageRoom → skip`), so EVERY non-arrival room created another
  22 KB `ALINK` + `METER2`. 6 rooms → 6 players → `JKRExpHeap.cpp:245` OOM. The
  `roomLoader` pin (`setRoomNo(param_2)` before PLYR decodes) already makes `stageRoom`
  authoritative — the reconcile was obsolete AND harmful. **Do not reinstate it.**
- **№91 player-create latch (ADDED):** `dStage_actorCreate` queues `ALINK` **async** —
  `dComIfGp_getPlayer(0)` stays NULL for several room decodes. So the arrival room
  decoded by multiple procs re-created the player before `havePlayer` flipped. The latch
  enforces TP's **one-player** invariant across the async window: once the create is
  queued, further `playerInit` passes for the same arrival skip until Link materialises
  or the stage identity changes. PC-only (native TP is synchronous and never needed it).

**Lesson:** on this PC port, any per-room engine action that assumes synchronous player
creation can duplicate under async load. Guard state that must be created "once per
arrival" with a latch keyed on stage identity, not on a bare `getPlayer() != NULL`.

### C. Cutscene NPC renders "all over the place" / at world origin  ⚠️ RECURRING — Aryll → Grandma
**This is the pitfall the user specifically flagged: the rendering bug Grandma had is
the SAME one Aryll (Ls1) had before her.** It will hit every new WW cutscene NPC.

**Symptom:** the model spawns but draws stretched/scattered; a joint probe reads every
bone at `(0,0,0)`.

**Root:** `_draw` was missing **`mpMorf->modelCalc()`** (cookbook Recipe 2/13). Two
different matrix passes are easy to conflate:
- `mpMorf->calc()` composes **animation** matrices (joint anim).
- `mpMorf->modelCalc()` composes the **render/world** matrices from the base transform.

Without `modelCalc()` the whole model draws at the world origin — nothing positions it.
Add `modelCalc()` **before the btp/material entry** in `_draw` (zl1 precedent
`d_a_npc_zl1.cpp:2624`). Render ORDER matters: `modelCalc()` before btp, or the
scripted-face texture swap composes against stale matrices.

**Why it recurred:** the Aryll/Ls1 fix was not carried into ba1's `_draw`. Any WW
cutscene NPC created from a fresh McaMorf must include the `modelCalc()` call — it is
NOT optional and NOT implied by `calc()`.

**How it was found (method, reusable):** a **multi-hypothesis render probe** (one build
instrumenting ~10 candidate causes at once, per the user's standing directive) —
NOT one-cause-per-build bisecting. It read the joints at `(0,0,0)` and exonerated the
arc/id path, node callback, McaMorf ctor, and endianness in a single pass.

**Adjacent render/anim traps cleared on the same actor (keep on the checklist):**
- **Lighting too bright** — `settingTevStruct(TEV_TYPE_ACTOR, …)`, not the raw `0` type.
- **`anm_prm_c` endianness** — packed `s8` fields (`mBtpNum`/`mAnmNum`) in a big-endian
  `u32` table read the wrong LE byte after reinterpret (floats survive, sub-word bytes
  don't). Reorder the struct for LE. (§260/§266.)
- **McaMorf with a NULL create-anim** (zl1 pattern) when the anim is set later by the STB.

### D. The demo-arc name is a SINGLE global slot
`dStage_roomControl_c::getDemoArcName()` holds **one** arc name. `§48` (opening/
`Demo02`) and `§273` (tale/`Demo01`) already retarget it. Two events needing *different*
demo arcs resident simultaneously evict each other, and `getStbDemoData` returns NULL for
the loser (the "ordered and accepted, but nothing plays" signature).

**Lesson:** bundle **all** of a room's storyboards + cast anims into ONE demo archive and
address individual `.stb` by name inside it. Do not give each event a private arc that
fights for the slot. `ensureDemoArcResident()` retargets safely only because we never need
two arcs at once — that assumption breaks the moment two events do.

### E. WW event bits read TP's table → wrong flags  ⚠️ OWED (the real ceiling)
WW actors call `dComIfGs_isEventBit(WW-index)`, but the TP port routes those indices to
**TP's** event table where they mean unrelated things. For Grandma we hand-bridged
`init_BA1_0` to the pre-tale state (§266). Her progression/dialogue reads
(`0x608/0xE20/0x740/0x780/0x601/0x602/0x2A20/0x2A80`) STILL hit TP's table (wrong-message
risk).

**For MULTIPLE events this is the blocker:** "event 2 must not fire before event 1",
"already-seen" suppression, and branch conditions all live in WW event bits. Until a **WW
event-flag namespace** exists (every WW story actor reads WW flags, not TP's), multi-event
sequencing will misfire — events firing out of order or repeating. Affects ANY WW
story-driven actor, not just ba1.

---

## 3. Current state (§274)
- `STG_00.arc` = `pre-revt-bak` (single-room, verified byte-identical, md5
  `68b40b2f65c000192b518a5f148d673c`). Interior enters clean; **tale does not play**
  (no REVT to order — freezes/does-nothing, accepted interim).
- Foundry's REVT bake preserved as `STG_00.arc.revt-6room-bak` for a corrected re-bake.
- Code fixes kept (inert with one room): №90 reconcile removal + №91 player latch
  (`d_stage.cpp`), §273 tale-entry wire (`d_ext_npc_mount.cpp`, no-ops until REVT returns).

## 4. Ferry to Foundry (the ask)
The REVT must be re-baked so it fires **only** on the tale trigger (the §273 pending-demo
path), never reachable by the generic door/arrival event. Concretely: the tale event must
not be eligible during a plain `R_DL01` arrival, and its staff/camera must stay **inside
room 0** so the stage never flips to all-room mode. Once gated, we re-point `STG_00.arc` at
the corrected bake and re-run the §273 acceptance. See the bus §274.
