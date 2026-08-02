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

---

## DN-2 — NEVER "repair" the №113 sea-K0 stash in PAL0 `plight_col[2]`

**Forbidden act:** treating the values in `stage.dzs` PAL0 `plight_col[2]` as
corrupt and "restoring"/zeroing/normalizing them, or letting a hash/verify tool
flag a change there as damage.

**Guarded surfaces:** `F_DL01/STG_00.arc::stage.dzs` PAL0 (and any converted WW
stage lighting), `convert_lighting.py`, any arc-integrity/hash check over stage
lighting, any "PAL0 looks wrong" diagnosis.

**Why it is rejected — the mechanism (§113 / §148):** the №113 lighting converter
**deliberately stashes the sea's K0 color** into PAL0 `plight_col[2]`. As a
*point-light* color those numbers look like garbage (blues/blacks) — **but they
ARE the water's blue endpoint**, read at runtime by `dKy_get_seacolor`. The stash
is **load-bearing**: strip it and the sea color breaks. This is documented in a
converter code comment that other lanes have no reason to have seen.

**The incident (the fix-revert loop this entry exists to stop):**
- №270 (History) saw the stash → mislabeled "corrupt PAL0" (`1d89baba`) →
  restored a stash-LESS arc (`84e512ce`) → broke the water.
- §112 (Engine) found the stash missing → re-injected it → `1d89baba` again
  (~22h later; rewrite preserved the awake event_list = §125-converter signature).
- History was about to strip it a THIRD time — caught at cycle 3 by §148.
- **Decisive proof `1d89baba` is canonical:** the sea was accepted against
  noclip while rendering from `1d89baba`. Corrupt lighting cannot match noclip.

**The sanctioned truth:** `1d89baba` = CANONICAL (stash present). `84e512ce` =
BROKEN-WATER (stash stripped; that backup is renamed
`…STASH-LESS-broken-water-DO-NOT-RESTORE.bak`). Merges are fine — `pack_rarc`
preserves the stash byte-for-byte (proven by round-trip), so a member-identity
check that requires stage.dzs to be UNCHANGED is correct and stash-safe.

**Character purple/black is a SEPARATE symptom.** If Link renders purple/black
in a restored area *by eye*, the fix is in the character light path (it must not
read the plight stash as a point light) — **never** by editing the arc. Report
it; do not touch PAL0.

**Escalation:** no AI instance may "fix" this slot. If a tool must change stage
lighting, the change goes through `convert_lighting.py` with the stash preserved,
user-directed. Hash tools must whitelist `plight_col[2]`.

---

## DN-3 — NEVER parse BDL models at arc-mount / globally in this port

**Forbidden act:** adding BDL-model parsing (`J3DModelLoaderDataBase::loadBinaryDisplayList`,
or the `'BDL '`/`'BDLL'`/`'BDLM'`/`'BDLI'`/`'BDLC'` resType cases) to the arc-mount
resource switch (`dRes_info_c::loadResource` in `d_resorce.cpp`), or to ANY other
global / mount-time layer that parses every BDL as the arc loads.

**Guarded surfaces:** `d_resorce.cpp` `loadResource` resType switch (the `'BMDL'` case
stays `#if DEBUG`); anything that introduces a mount-time / global BDL parse; the
ExtNpcMount consume-time model path (`acquireMountedModel` / `acquireBgModel` /
`s_modelDataCache` / `dExtNpcMount_acquireDemoModel`).

**Why it is rejected — the mechanism (§180/§181, 2026-07-27):** this port parses BDL
models at **CONSUME time**. ExtNpcMount's mount flow (`d_ext_npc_mount.cpp`)
`loadBinaryDisplayList`s each BDL when an actor consumes it, then caches the parsed
result. `loadBinaryDisplayList` **pointer-fixes the raw buffer IN PLACE** (offsets →
pointers). Add a SECOND parse layer at arc-mount and every BDL that also flows through
the consume path is parsed **twice** — the second call re-fixes an already-fixed buffer
→ corruption. This is the J3D-pointer-fix law again (prior receipts: sumo BMT crash,
room-lane mesh corruption).

**The incident (the regression this entry exists to stop):**
- §180 (History) found demo-arc BDLs (`fuku.bdl`) loaded RAW → a crash
  (`mDoExt_bckAnmRemove` deref of an unparsed model, fault `0x10001001000`) and "fixed"
  it by adding the donor's `'BDL '`-family cases to the arc-mount switch.
- That fix took out the ENTIRE island: opening the door to Outset → `model.bdl` (the BG)
  got parsed at mount (§180) AND by ExtNpcMount's consume path → **double-parse** →
  `EXT_BG0 … resLoad ERROR` → arc purged → **black screen, Outset would not load.**
- The demo fix and the BG break were the SAME line-family: invisible at the demo (it
  fixed that), loud on the whole island — the DN asymmetry.

**The sanctioned path (§181, Housing Approach A, shipped):** parse a BDL exactly ONCE,
at CONSUME time, through the port's cached resolver. Demo cutscene doubles use
`dExtNpcMount_acquireDemoModel(arc, id, res)` → `acquireMountedModel` (single-parse via
`s_modelDataCache`; pristine raw stashed before the in-place fix; entry **erase-only**
purged with the arc — the arc owns the buffer, no UAF). The arc-mount switch stays
BDL-free on this port. Non-PC keeps the donor mount-parse (retail parses at mount; no
consume layer to collide with).

**Why NOT a "double-parse guard" instead (Approach B, rejected by Housing):** detecting
"already parsed" on an arbitrary buffer means reading exactly the bytes the pointer-fix
destroys — it misfires both ways. And it modifies the artery every working mount flows
through (rooms, NPCs, props, BG). Equal outcome, unequal risk. Consume-time-only is one
buffer, one owner, known lifecycle.

**Escalation:** no AI instance may re-add mount-time / global BDL parsing. If a NEW
consumer needs a demo/arc BDL as a real `J3DModelData`, route it through the consume-time
cached resolver — **never cast `getObjectRes`/`getObjectIDRes` → `J3DModelData*`**. If you
believe a mount-time parse is genuinely required, STOP: write the double-parse mechanism
up point-by-point and get an explicit user go.

**Verification signature:** open the door to Outset — the island boots (no
`EXT_BG0 … resLoad ERROR`, no `release arc 'Outset' … purge`); a demo double's
`createHeap` logs `model-data cache + <arc>/demo_shape_<id>` on first use and
`session-cache hit` after (exactly one parse); `§180 modelData head` reads a heap vtable,
never `0x4A334432` (raw `J3D2`).

---

## DN-4 — NEVER present dialogue through the ALBW "post-man" box — ALWAYS the Shade Watcher native path

**Forbidden act:** routing ANY dialogue — cutscene or NPC, WW-restored or otherwise —
through the **ALBW "post-man" dialogue box** (the ALBW-style box that crops WW lines and
ignores TP's native centered line-count / word-wrap formatting).

**User directive (2026-07-27, emphatic and standing):** "NEVER use the ALBW post-man box,
EVER. Always Shade Watcher — so even if you forget, EVERY instance remembers." This entry
IS that memory. It binds every lane and every future instance.

**Guarded surfaces:** every dialogue/message presentation for restored cutscenes
(tale / awake / …) and NPCs; the renderer choice at any message choke point
(`dExtWw_handleDemoMessage`, the mount talk path, any `dMsgFlow_c` / dialogue-box init site);
any code that selects which box a message is shown in.

**Why it is rejected — the mechanism:** the ALBW post-man box crops a multi-page WW message
to its first box, ignores TP's native line-count (4/page), word-wrap, and centering — the
"excerpt" / cut-off dialogue seen on BOTH the Aryll/awake scene AND the Grandma tale. It is
a repeat offender; each time, the fix was to swap OFF it.

**The sanctioned path — the "Shade Watcher pattern":** present dialogue through TP's NATIVE
message flow — `dMsgFlow_c::initWord` + `mountPaginate` (`d_ext_npc_mount.cpp:220`; 4
lines/page, 38 cols, whole-word flow, whole-sentence carry across page breaks, blank line =
WW page break), the pattern proven by `d_a_albw_shade_watcher.cpp:1851-1872`
(`sShadeFlow.initWord`). Auto-advance ALL pages before closing, honoring the suspend/resume
contract (`dExtWw_oweDemoResume` / `pollOwedDemoResume`) so the timed STB isn't frozen. (The
demo/JMSG formatter `dALBWDialogue_c::buildPages` + the `d_ext_npc_mount.cpp:6294`
page-advance is the equivalent fixed path for STB-driven messages — also acceptable; the
post-man box is the ONLY forbidden one.)

**Escalation:** no AI instance may present dialogue on the ALBW post-man box — not "just this
once," not "as a stopgap." If you find a dialogue surface on it, swap it to the Shade Watcher
native path. There is no sanctioned exception.

**Verification signature:** dialogue renders as full TP-native centered boxes (≤4 lines/page,
word-wrapped), auto-advances through every page, and NEVER crops to a first-box excerpt.

---

## DN-9 — NEVER solve a donor system by MOUNTING a model onto a receiver proc

**User ratification, 2026-08-01:** *"we're never doing mounting again. Everything is
native with the native systems ported over."* Standing law; supersedes any
convenience argument in any lane.

**Forbidden act:** presenting donor content by hanging its MODEL on a receiver
actor/proc (ext-NPC mount providers, `NPC_*` stand-ins, attach slots) when the
donor has a real SYSTEM for it. Examples of the rejected pattern: flowers/trees
mounted as `NPC_YAFLW` instead of the donor flower/tree PACKET; props mounted as
NPC procs instead of `daObj_*` actors; any "make it look right" model swap that
leaves the donor's own actor/packet unported.

**Guarded surfaces:** `d_ext_npc_mount.cpp` providers/manifests, `population/*.ini`
actor_map entries, any new `npc_*.ini` that maps a donor placement name to a
receiver proc.

**Mechanism (why it always costs more than it saves):** a mount reproduces the
donor's APPEARANCE while its BEHAVIOUR, state machine, animation gating, physics,
particle spawns and per-frame update stay unported. Every later fidelity question
("why won't it sway / cut / drop / react?") then routes into the mount layer,
which cannot answer, and the work is thrown away when the native system finally
lands. Case receipts: grass VFX chased through THREE colour ferries (V, V-b, V-c)
before the root proved to be a TP particle drawn in WW space (§192); flowers still
inert after mount tuning because the packet was never ported (§205).

**Sanctioned path:** port the donor's own actor/packet/system (decomp is the spec,
per the full-state-machine law), register it natively, and let the donor code draw
and drive its own content. A donor system with NO receiver counterpart is a PORT
task, never a mount task.

**Escalation protocol:** if a mount seems unavoidable (e.g. the donor system depends
on an unported subsystem), DO NOT ship it as the answer — write up the dependency
chain and put it to the USER. Any temporary mount must be labelled a BRIDGE in code
and carry its native-target ticket (see the standing "always port true native
subsystems" directive).

**Verification signature:** a native port shows the donor's BEHAVIOUR without
receiver-side per-actor special-casing; the mount pattern shows correct-looking
models that do nothing the donor's do.
