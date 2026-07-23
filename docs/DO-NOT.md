# DO-NOT registry — hard stops for every AI instance

**What this is:** a registry of approaches that are PERMANENTLY REJECTED in this
codebase because they caused (or would cause) cascading, hard-to-diagnose
failures. An entry here is not advice — it is a **hard stop**.

**Rules of the registry (all lanes: Cursor, History, Engine, Housing, any
future instance):**

1. **Consult BEFORE touching a listed subsystem.** Each entry names the code
   surfaces it guards. If your plan touches one, read the entry first.
2. **An entry is a hard stop, not a caution.** Implement the sanctioned path
   given in the entry. If you believe your case is genuinely different, DO NOT
   proceed — write up why (with the entry's own failure mechanism addressed
   point-by-point) and put it to the USER for an explicit go. No AI instance
   may self-approve an exception.
3. **Entries never expire.** A fix elsewhere does not retire the entry — the
   mechanism stays true even after the incident is healed.
4. **Adding entries:** any lane may add one after a confirmed root cause, with
   the user's direction. Keep the format: mechanism → blast radius → sanctioned
   path → escalation protocol → verification signature.

---

## DN-1 — NEVER stamp room id 0 (or any guessed room) onto collision/BG owners

**Forbidden act:** calling `SetRoomId(...)` (or otherwise stamping a room
identity) on a daBg/BgW/collision owner with **room 0**, a keep-slot value, a
`fopAcM_GetRoomNo()` read from an identity/GLOBAL mount, or ANY value not
positively resolved to the mount's real host room.

**Guarded surfaces:** `dBgW`/`daBg` registration, `SetRoomId`, room-lane mount
creation (`d_ext_npc_mount.cpp`), anything that makes geometry the player can
STAND ON.

**Incident (№256 → №264/№265, 2026-07-22..23):** an identity `GLOBAL_e` mount's
BG was registered with `SetRoomId(fopAcM_GetRoomNo(...))`, which on identity
mounts held the keep-slot value **0**. It looked harmless — the mount worked,
collision worked, nothing crashed.

**Why it is rejected — the failure mechanism (read all of it):**

1. **The player's room number is DERIVED FROM THE BG UNDER HIS FEET.** Stamp a
   wrong room on standable geometry and every actor standing on it silently
   inherits that room. There is no error, no crash, no log.
2. **The engine keys ENTIRE SUBSYSTEMS by the player's room.** Confirmed blast
   radius from one `SetRoomId(0)`:
   - `dEvent_manager_c::getEventIdx` matches room-typed event-list slots by
     player room (`roomNo == mEventList[type].roomNo()`) — **every stage event
     pack resolved -1**: the opening cutscene died, and
   - **door-open events resolve through the same slots** — every door on the
     island stopped opening, and
   - the pending opening held the arrival/camera system in an infinite defer
     loop (№176), and
   - none of these pointed back at the BG registration. The symptom set read
     as three unrelated bugs in three other lanes.
3. **The failure is invisible at the change site and loud everywhere else.**
   That asymmetry is WHY this entry exists: the person who breaks it will
   never see the breakage in their own test.

**The sanctioned path (№265, shipped):** resolve the mount's TRUE host room —
room-lane map → `manifest.hostRoom` → actor room **only if > 0** — via
`resolveIdentityBgHostRoom`, and stamp THAT. **If the room cannot be resolved,
leave the daBg default (`0xFF` → `GetGrpRoomId`) — an unresolved room is
handled by the engine; a WRONG room is not.** Delete/release must clear the
resolved slot by the same resolution, never by a blind `GetRoomNo` read.

**If you truly believe you need a non-host room id:** stop. Write the case up
against mechanism points 1–3 above (what stands on this BG? what inherits its
room? which room-keyed subsystems can see that actor?) and get an explicit
user go. There is no known legitimate use of `SetRoomId(0)` on standable
geometry in this codebase.

**Verification signature (how to prove you didn't break it):** with Link
standing on the geometry — `[Alink] §63 ... roomId=<host room, e.g. 44>` (NEVER
0 on a real room's floor); `§46` stage-event resolution succeeds; doors open.
The §63 probe's `roomId` field exists because of this incident — keep it.
