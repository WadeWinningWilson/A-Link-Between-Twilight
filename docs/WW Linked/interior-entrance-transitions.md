# Interior entrance transitions — FOUNDATIONAL KIT REQUIREMENT

> **User directive, 2026-08-11 (verbatim):** *"This needs to be a foundational
> entry for all space_kit (or whatever it may have been renamed to). All
> interiors will receive their corresponding entrance animations/events
> governed by native systems (doors/gates/boss doors/walk-throughs/
> crawl-throughs/fall-throughs/whatever else I may be ignorant of)."*

**Status of this document:** foundational. An interior port is NOT complete
when the room renders and the player stands in it. It is complete when the
player *arrives the way Wind Waker makes them arrive*. Every kit that ports,
stages, or activates an interior must satisfy the requirement below.

Governed by [DN-10](../DO-NOT.md) — the transition is a donor SYSTEM, so it is
ported, never reconstructed. No timers, no teleports, no "place the player and
fade in" stand-ins.

---

## 1. The requirement

For every interior, the port must carry **the transition actor(s) the donor
stage actually references**, and the entrance event those actors drive, so that:

1. the entrance **event/cut sequence** dispatches and **advances to its end**,
2. the actor that owns the event's staff exists and **mans it** (an unmanned
   staff stalls — see the `SHUTTER_DOOR` incident, tale §753/§756),
3. the player is **placed by the transition**, not by a raw spawn write,
4. the **exit** transition is verified too — entering and leaving are different
   code paths and a working entrance proves nothing about the exit.

**Acceptance is behavioural, not log-shaped.** A marker that fires at load time
does not demonstrate that a door opened. See the card's own pass-9 lesson.

---

## 2. The donor taxonomy — enumerated from WW's own actors

Source: `D:\XXXXXXX\WW DP\src\d\actor\`. **This list is donor-derived, not
invented.** It is the answer to "whatever else I may be ignorant of" — the
donor's own set is the complete set.

| class | donor actor | receiver status |
|---|---|---|
| door | `d_a_door10.cpp` | **unported** |
| door | `d_a_door12.cpp` | **unported** |
| door | `d_a_kddoor.cpp` | **unported** |
| door | `d_a_knob00.cpp` | PORTED |
| door | `d_a_atdoor.cpp` | **unported** |
| door | `d_a_mdoor.cpp` | **unported** |
| door | `d_a_mbdoor.cpp` | **unported** |
| door | `d_a_swtdoor.cpp` (switch-gated) | **unported** |
| door | `d_a_obj_majyuu_door.cpp` | **unported** |
| shutter | `d_a_shutter.cpp` | **unported** |
| shutter | `d_a_shutter2.cpp` | **unported** |
| fall-through | `d_a_obj_hole.cpp` | **unported** |
| ladder | `d_a_obj_ladder.cpp` | PORTED |
| stair | `d_a_lstair.cpp` | **unported** |
| stair | `d_a_obj_stair.cpp` | **unported** |
| player mode | `d_a_player_crawl.inc` (crawl-throughs) | see player port |
| player mode | `d_a_player_ladder.inc` | see player port |

**2 of 15 transition actors are ported.** Measured 2026-08-11 against
`src/d/actor/`. Re-derive with the same listing before trusting this row —
it is a snapshot, not a live gate.

**`SHUTTER_DOOR` note:** the donor's `specialCast("SHUTTER_DOOR")` tries
DOOR10 → DOOR12 → KDDOOR → KNOB00 (donor `d_event_manager.cpp:689-698`). With
only KNOB00 ported, every shutter staff in the game falls through three missing
casts before it finds anything. The first three ports in the table are
therefore not independent items — they are the rest of that chain.

---

## 2b. The ARRIVAL EVENT family — the layer above the actors

The transition actors are only half of it. The donor selects an **arrival
event** on entry, and that event is what animates the walk-in. Read from
`res/Stage/<stage>/Stage.arc → event_list.dat` (identical set in LinkRM and
Ojhous, so treat it as the standard interior family):

| arrival event | drives |
|---|---|
| `KNOB_START` / `KNOB_START_B` | knob-door walk-in (front / back) |
| `SHUTTER_START` / `SHUTTER_START_STOP` | shutter-door arrival |
| `BS_SHUTTER_START` | boss-door arrival |
| `FALL_START` | fall-through arrival (`d_a_obj_hole`) |
| `FMASTER_START` | — |
| `OPTION_CHAR_START` | — |
| `DEFAULT_START` | **plain placement, NO animation — the fallback** |

**THE EVENTS ARE NOT MISSING.** `event_list.dat` loads (188600 bytes for
Ojhous) and carries all nine. What is measured, run 221958:

- LinkRM ran `KNOB_START_B` — the knob walk-in — once the `specialCast` fix
  landed (tale §756).
- **Ojhous ran `DEFAULT_START`** — the no-animation fallback — which is exactly
  "the interior entrance animation did not occur."

> ### ⚠ RETIRED CONCLUSION (tale §901b, 2026-08-13) — read this before the paragraph below
> **"Selection, not porting" was BACKWARDS.** Selection obeys: the PLAYER maps
> PLYR `start_mode` to an arrival id correctly (donor d_a_player_main.cpp:12302-
> 12321, and §636 already translates that field). What was unported is the
> **NAME TABLE** those ids index — donor and receiver share the index space
> (201..213/214) with different tables, so 12 of 13 WW ids resolved to the wrong
> TP event. LinkRM's "correct" `KNOB_START_B` was COINCIDENCE (TP 208 =
> KNOB_START_B; WW 208 = SHUTTER_START_STOP), and Ojhous's `DEFAULT_START` was
> the mis-resolution, not a fallback. Ported WW-scoped in
> `src/d/ext_plugin/ww_event_names.cpp` (tale §901b). Measured ids: §900.
> **The paragraph below is kept as the record of what we believed and why.**

So the defect at this layer is **arrival-event SELECTION**, not porting: the
donor picks `KNOB_START_B` when the player arrives through a knob door, and the
receiver selected `DEFAULT_START`. **Do not port an event that already loads —
find what the donor uses to choose, and port that.**

**And selection is gated by §2's table:** an arrival event whose owning actor is
unported can never be chosen. With 13 of 15 transition actors unported, seven of
these nine arrival events have no route to selection today. **The two lists are
one worklist.**

---

## 3. What a kit must do

**Any kit that stages or activates an interior** (`space_kit`, `actor_kit`, and
any successor) must, per interior:

1. **Enumerate** the transition actors the stage's own DZR/DZS references —
   read them from the donor, do not assume `KNOB00`.
2. **Report** which of those are unported, and **refuse to report the interior
   complete** while any are missing. An interior whose entrance actor is
   unported is `INCOMPLETE`, never `DONE`.
3. **Never substitute.** Casting a different door class because the right one is
   unported is a DN-10 step-3 construct; it produces a room the player can enter
   and an animation that is not the game's.
4. **Name the exit** as a separate line item from the entrance.

**UNKNOWN is not clean here either:** a kit that cannot determine an interior's
transition set must say so, not pass it.

---

## 4. Evidence trail

- The `SHUTTER_DOOR` stall: ~2160 dispatches with `adv=0` because the staff had
  no owner (tale §753), root-caused to the receiver's TP-only `specialCast`
  list (tale §756).
- Ojhous, run 221958: `SETSTART_PULL` dispatched **0** times — the entrance
  event did not fire at all, and the user reports no entrance animation. A
  `KNOB00` was created in that window, so "no door actor" is NOT the confirmed
  cause; mechanism is open and owned by Housing/Engine.
- Entering worked and the exit was untested for several runs — precisely the
  asymmetry §1.4 exists to prevent.
