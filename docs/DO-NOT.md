# DO-NOT registry — hard stops for every AI instance

era: era-independent
<!-- era rationale: registry rule 3: entries never expire | Librarian, 2026-08-16, user ruling "assign it by era" -->

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

## ORIGIN — where these laws come from (the user's account, verbatim)

**Placed here 2026-08-16 (Librarian ruling, A5 sweep). This is not a preface;
it is DN-9's and DN-10's first case receipt.** Every entry below carries the
user's ratification and the failure that produced it. **These two entries were
missing the failure — it happened before the registry existed, and it is the
same event for both.** Rule 3 above ("entries never expire — the mechanism
stays true even after the incident is healed") is the durability guarantee this
account needs, already written into this document.

**The user's words, held exactly, because founding history cannot be
re-derived from the tree by anyone:**

> *"'Accidental Ivan' was a quirk of the mounted era. When the outset map was
> first mounted, often the map would 'expect' certain actors or props (like
> trees, rupees, etc) to be present. However since none were mount-ported,
> random items took their place (not random actually — for fun after reviving
> the Ivan stub, I had wanted to see the actors for a few WW characters —
> Makar, Medli, WW Ganon in TP areas). One of these was the Ivan WW NPC standing
> in the place of an swood. An 'Accidental Ivan'. However Ivan's stubs were
> investigated earlier, when those stubs were built upon, followed by some
> lighting tweaks/bakes to the model to make him appear near-correct in TP
> areas, and this same lighting formula was used for the other characters first
> ported for fun (again Makar, Medli, WW Ganon, and the like), then the lighting
> was used for Outset island and following mounted era ports as TP lighting was
> still present in those areas."*

**The sequence:** (1) the Ivan stub investigated and **deliberately** revived —
the Accidental Ivan was a later quirk, not the founding event · (2) lighting
tweaks and **bakes** so he read correctly under TP lighting · (3) the same
formula reused for the fun ports (Makar, Medli, WW Ganon) · (4) the **mounted
era** — Outset mounted, unported props back-filled by whichever fun character
was loaded; Ivan stood in for an `swood`, and was later kept as the canary ·
(5) the formula carried into Outset and subsequent mounted ports while TP
lighting remained.

**WHY IT BELONGS IN THE HARD-STOP REGISTRY AND NOWHERE ELSE — the two most
binding entries in this file are the systematic retirement of the two shortcuts
in that paragraph:**

- **DN-9** forbids mounting. The mounted era is where mounting came from, and
  the Accidental Ivan is what it looks like from inside: a map asking for props
  nobody had ported, filled by whatever model happened to be loaded.
- **DN-10** forbids baking. **The founding lighting formula WAS a bake** — and
  zero-bake, DN-10's order of resort, and `kit_laws` LAW 1's dKyWw lighting
  contract are its systematic replacement.

**AND THE PART WORTH KEEPING WHEN THE INCIDENT IS FORGOTTEN: this project's
strictest laws grew out of its own first hacks, and were written by the people
who committed them.** That is the right direction for a law to grow, and it is
the answer to any future instance who reads these entries as arbitrary
severity. They are not strict because someone was cautious in the abstract.
They are strict because the estate has already paid for the alternative.

**Confirmed at source, not taken on faith:** the `swood` Ivan stood in for is a
real donor actor whose family was **later ported natively** — `d_stage.cpp`
carries `OBJNAME("swood"…)`, `swood3`, `swood5` under the §696 vegetation
banner, and the comment records that they were *previously ABSENT* from that
table. **The placeholder's slot became a real port.** Verified by History/Bridge,
2026-08-16.

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

### ⚠ SCOPE EXTENSION — SLOT [1] IS ALSO A STASH (recovered 2026-08-16)

**This entry protected `plight_col[2]` ONLY. The law is broader, and the broader
version was living in an ARCHIVAL document** — `WW-Restoration-Cookbook-CANONICAL.md`,
whose №113-STASH LAW carries a scope extension this registry never received
(user ratification 2026-07-27, bus §182). Recovered here by the Librarian on the
user's migration ruling, 2026-08-16.

**Slot `plight_col[1]` = BG0_K0 is ALSO a №113 stash and is ALSO load-bearing** —
windline alpha (`d_kankyo_wether.cpp:1559`) and the grass cut-VFX colour (Ferry
V-b) both read it. **The whitelist and carry-forward duty covers BOTH slots [1]
and [2], not [2] alone.**

**Why this is recorded as a finding and not a quiet edit:** an instance reading
this entry alone would have concluded slot [1] was unprotected and "repaired" it,
killing two effects — **and would have been following the hard-stop registry
correctly while doing it.** The narrower law was not wrong; it was incomplete,
and nothing pointed at the fuller copy. That is the exact hazard the estate spent
2026-08-15/16 cataloguing, in the one document that is supposed to be definitive.

**Valid-exception clause (user, at ratification):** the law yields only if (a) we
ERRED — the stash is shown wrong against the donor — or (b) WW VANILLA is shown
to require that slot. Either way: evidence first, cross-lane sign-off (Engine +
History + Housing), THEN change, and this entry is updated. Never a silent
unilateral "repair."

**Ratchet target (bus §106):** graduate to a tool check — a stash-presence assert
in the gate/verify path — so the law enforces itself rather than relying on
someone having read it.

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

**AMENDMENT (2026-08-04, §398 — Housing, user-directed):** THE RULE ABOVE STANDS. Two
factual corrections to its framing, both load-bearing for anyone reasoning about model
lifetime:

1. **This port DOES mount-parse the BMD family** (`BMDR`/`BMDV`/`BMDE`/`BMWR`/`BMWE`/`BMDG`;
   `BMDP`/`BMDA` debug-only) exactly as the donor does. The gap is the **BDL family only**.
   "This port parses at consume time" is true of BDL and false of BMD — the asymmetry
   explains why BMD room models were never implicated in the §373-§393 plants campaign.
2. **Until 2026-08-04 there were no BDLs to parse.** Every genuine BDL in the mod folder was
   converted to BMD offline by `tools/ww_crew_restoration_skeleton/adapt_bdl_arcs.py`
   (0 `J3D2bdl4` / 227 `J3D2bmd3` across 83 staged arcs, 68 of which still carry `.bdl`
   filenames). That tool's stated premise — *"the port's loadBinaryDisplayList … crashes on
   WW models"* — was **TESTED AND DISPROVEN** on 2026-08-04: the donor `Ba.arc`, staged
   unconverted with genuine `J3D2bdl4` and MDL3 intact, loads, renders, animates and plays
   its cutscene through the DN-3-**sanctioned consume-time path**
   (`path=loadBinaryDisplayList` ×5, no failures). Likely fixed by §374's `WwFullMat3Scope`,
   which landed six days after the adapter was written; the conversion then made its own
   premise unfalsifiable.

**What this does NOT license:** re-adding a mount-time parse. The double-parse mechanism is
unchanged and still fatal — a second parse re-fixes an already-pointer-fixed buffer. A
migration to donor-style mount-parsing requires REMOVING the consume-time layer in the same
change, per resType, and still needs the explicit user go this entry demands.

**Companion deviation flagged by the same investigation:** the adapter also rewrites donor
LIGHTING state (`normalize_litmask` 0x03→0x01, `normalize_tevregs`→white). That is a missing
native system being covered by editing donor assets, it is baked into every shipped WW model,
and it is invisible to every runtime probe. See §398 and the proposed DO-NOT on donor-data
rewriting.

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

---

## DN-10 — NEVER solve a defect with instance-authored code before the NATIVE system has been read and tried (the ORDER OF RESORT)

**User ratification, 2026-08-11 (verbatim):** *"NEVER bake, if there is an issue,
AI instances don't solve it, the native systems DO. If the issue is because of
prior work or needed receiver translating the WW vanilla systems are PROVEN not
to solve, THAT is when instances work. No guarding, no patching, no baking, no
mounting, no legs."*

Standing law over every lane and every surface. This entry generalises DN-9
(mounting) and the §723 guard ledger into the single rule they were both
instances of.

**Forbidden act:** authoring ANY of the following as the answer to a defect
before the donor's own system has been read and attempted —

- a **guard** (null-check, abort cover, state clamp, capacity mask)
- a **patch** that supplants original code rather than porting it
- a **bake** (an edited donor asset, or an invented value written into data)
- a **mount** or a **leg** (a receiver proc standing in for a donor system)
- a **translator/default/clamp** invented because a donor field "looks missing"

**Guarded surfaces:** all of them. This entry has no scope limit — that is the
point. It applies hardest where a symptom is loud and the native path is
unread: collision/BG attributes, room and stage identity, event and cut
dispatch, spawn/entry placement, camera, audio, resource lifetime.

**THE ORDER OF RESORT — the only sanctioned sequence:**

1. **The native WW vanilla disc original.** Read the donor's own implementation
   FIRST (`D:\XXXXXXX\WW DP\src`, arcs `D:\XXXXXXX\Ex WW`). If a system is
   missing, **PORT THE SYSTEM.** This is the answer in the overwhelming majority
   of cases and it is where every lane starts.
2. **Receiver translation at the CONSUMPTION BOUNDARY** — permitted ONLY where
   step 1 is *proven* not to solve it, because prior receiver work or a genuine
   donor/receiver format difference requires translating. Never an edit to donor
   bytes (see the zero-bake law); the translation lives in receiver code.
3. **Instance-authored construct** — permitted ONLY when 1 and 2 are BOTH proven
   insufficient, **with the proof written down** and the construct labelled a
   bridge carrying its strip trigger.

**What counts as PROOF (step 2/3 gate):** you read the donor's own code for this
exact surface and can NAME why it cannot apply — the prior receiver work it
collides with, or the format difference it cannot cross. **"I could not find it",
"it was faster this way", and "it works" are NOT proof.** A green log is not
proof. No AI instance may self-approve the step-3 exception; it goes to the USER.

**Why it is rejected — the failure mechanism:** an instance-authored fix treats
the SYMPTOM at the site where it is visible, which is never the site where the
native system is absent. It then becomes load-bearing, and the absence it was
hiding is now invisible to every later lane.

**Blast radius, measured on this project (the incident that produced this entry):**
the knob00 door chain accumulated **1 native fix to 8 AI-derived ones** — four
guards (§713c fork, §714 cutEnd, №269 WALL_NONE cover, §717 abort-check) plus
shims and probes. **Every one of them existed because the destination room was
never ported.** The interior landed on 2026-08-11 and the whole guard set began
retiring the same day, untouched. Eight sections of instance work; the fix was
"port the room."

**Escalation protocol:** if you believe your case is a genuine step-3, STOP.
Write the donor-side finding, the named reason the native path cannot apply, and
the strip trigger — and put it to the USER for an explicit go.

**Verification signature:** a lane's fix is DN-10-clean when its write-up names
the donor source it read. A fix whose justification cites only receiver files has
not cleared step 1.

**DN-10-S — SUBSTITUTION IS EVIDENCE, NOT A TECHNIQUE (user ratification, 2026-08-12).**
*"Why are there substitutions? Not allowed, it's a sign a system is unported, identify and port."*

A **substitution** is calling a receiver function where the donor calls a DIFFERENT donor function
of similar name/shape — `donorFn2()` answered with `receiverFn()`, arguments dropped to fit.

**It is forbidden, and more usefully: it is a DETECTOR.** Every substitution marks the exact
source line where a donor system was not ported. Treat one as a find, not a workaround:

1. **Name the donor function and its full signature.** The dropped arguments are the missing
   feature — they tell you what the receiver's stand-in cannot do.
2. **Check whether the donor system exists in the receiver at all.** If absent, it is an unported
   system and it goes on the port list under its own name.
3. **Never "adapt" by dropping arguments.** A `NULL` passed where the donor passes a texture, a
   matrix, or a callback is a silently degraded feature, and it will surface later as an art bug
   nobody connects back to the port.
4. **A comment recording the substitution does not license it.** Self-documenting the swap makes
   it auditable, not permitted — three ported actors carried `setSimpleShadow2 -> setSimpleShadow`
   in comments for many sections before anyone asked why.

**Verification signature:** grep ported TUs for `donorFn -> receiverFn` comment forms and for
receiver calls whose donor counterpart has MORE parameters. Both are cheap and both find real
unported systems.

**The incident that produced it:** WW pots rendered with opaque BLACK SQUARES under them. Cause:
`dComIfGd_setSimpleShadow2(pos, groundY, scaleXZ, floorPoly, rotY, scaleZ, pTexObj)` — which takes
a rotation, a separate Z scale, and a **texture object** — was substituted with the receiver's
`dComIfGd_setSimpleShadow(...)` and the texture passed as NULL. **An untextured shadow quad draws
black.** The visible art bug was a faithful report of an unported shadow system.

**Instruments are exempt in one direction only:** making a defect OBSERVABLE
(probes, logging, counters) is not solving it with instance code. Making it GO
AWAY is. Inverting a probe's blind early-return is instrument work; clamping the
value the probe reports is a DN-10 violation.

---

## ⓘ REGISTRY NOTE — the DN-5…DN-8 gap

**DN-5, DN-6, DN-7 and DN-8 do not exist and nothing in this file explains why**
(Librarian, 2026-08-16). They may have been retired, reserved, or never written.
**They are NOT reused below**, because silently recycling a number would make any
older citation of "DN-6" resolve to an unrelated law — the same class of harm as
a §-number collision. **A future entry takes DN-15 or higher.** If anyone knows
what 5–8 were, record it here rather than in a bus row.

---

## DN-11 — NEVER let WW content into the vanilla tree, and NEVER cross-pollinate WW and TP spaces (№31 + №31-B)

**Migrated here 2026-08-16 on the user's ruling** (*"GO with your
recommendation"*), from `WW-Restoration-Cookbook-CANONICAL.md`, which was ruled
ARCHIVAL on 2026-07-27 while these laws stayed live. **A live law housed in an
archival document is a law one cleanup away from vanishing** — and its DN-2
sibling proved the risk is real: the fuller version of that law had already
drifted out of this registry (see DN-2's scope extension).

**User ratification** (founding decree, Housing charter session ba0af71d;
encoded as a tool in `16ec60c842`), extended 2026-07-20: *"№31 should be written
for assets, music, lighting, everything."*

**Forbidden act:**
- Any WW content in the vanilla exe or game tree — **no WW bytes, no WW file
  names, no WW dialogue strings.** All WW content lives ONLY in the mod folder.
- **Cross-pollination in either direction.** WW spaces get only WW assets; TP
  spaces only TP assets. Rupees included (WW Vlupy visual, TP wallet credit).
  **A missing prop is always preferable to a foreign one.**
- Committing WW arcs, anything under the mod folder, or WW-named files.

**№31-B — purity covers SHAPING, not just supply.** *Any receiver stage that
modifies donor content on its way to the player is a purity surface.* Verify at
the stage **the player perceives**, not the stage easiest to hash. Case receipts:
grass colour altered via TP kankyo; a TP velocity curve squaring donor audio.

**NAMING RULE — POINTER, NOT A COPY.** This entry's filename requirement (the
`d_ww_` prefix on newly ported WW subsystem files, Housing/Engine 2026-08-17)
is RECORDED IN FULL at **[NEVER-PUSH-STRIP-SET.md](NEVER-PUSH-STRIP-SET.md)**,
where the strip set it governs is generated. **Read it there; it is not
restated here.**

> *This paragraph was itself a full second statement until 2026-08-17, written
> by the Librarian in the same pass as a row warning against two homes for one
> law — caught by Housing Security. Kept as a pointer because §113-STASH is
> the receipt for what divergence costs: the ARCHIVAL copy of that law turned
> out BROADER than the live registry entry, and an instance following the
> registry correctly would have "repaired" a load-bearing slot.*

**Guarded surfaces:** `greplist.txt` and the `ww_bridge gate` (M5a/M6); the push
gate in [NEVER-PUSH-STRIP-SET.md](NEVER-PUSH-STRIP-SET.md), which enforces the
push half of this law and must stay consistent with it.

**Verification signature:** a clean M6 greplist on the pushed tree's exe, and a
perceived-stage check rather than a source-stage hash.

---

## DN-12 — NEVER report CLEAN for a check that could not run (№31-C)

**Migrated here 2026-08-16 (user ruling).** **This is the most-cited law in the
estate and it had no home in the hard-stop registry** — it was quoted by four
lanes across 2026-08-15/16 while living in an archival cookbook.

**User ratification 2026-07-21/22:** *"Put it in cookbook"* (bus §61/§61c/§74).

**Forbidden act:** reporting CLEAN, PASS or SAFE from a check whose execution was
not established. **A check that cannot run reports UNKNOWN — never CLEAN.**
Absence of evidence is never converted into a verdict.

**Corollary (the falsifiability clause):** a pass must state **what it inspected**
— path, count, version — so it is falsifiable from its own output. Green must
prove the check executed.

**Mechanism:** *a vacuous pass is indistinguishable from a real pass*, so care
cannot detect it — care produces the same output either way. The only detector is
deliberately making the instrument fail. Origin: silent ported guards and the
gate's own empty-greplist false-CLEAN (fixed Bridge 0.19.0).

**Sanctioned path — this law is EXECUTABLE, use the command, not the prose:**
`tools/foundry/control.py` refuses to run a check whose gate has never been shown
to go red; `control.py audit` lists gates that cannot fail at all.

**Case receipts, 2026-08-15/16, nine instruments in one week:** an inert `\b`
regex; a `head -12` truncation read as a complete list; tier2's `\bww_` blind to
an underscore; `port_preflight`'s hardcoded donor path reporting MISSING; a
lineage gate blind to 18 of 107 files; a C-class list capped at 20 while the
count moved; a name-symmetry handoff scan that missed the current handoff; a
doc-collision detector that called a template tree a defect; and `sig_diff`'s own
negative control matching the vacuous summary line it was written to catch.
**Every one was a real observation that could not have come out differently.**

**Verification signature:** the gate has a registered control that has been
*demonstrated* to go RED, and the pass names what it inspected.

---

## DN-13 — NEVER invent an identity label (the IVAN RULE)

**Migrated here 2026-08-16 (user ruling).** Route: BILATERAL + RECEIPT; enforced
continuously, violation receipts bus §47 and §89.

**Forbidden act:** naming an actor, asset, character or system from resemblance,
convenience or a working nickname. **Names and resemblance are never evidence.**
A label stays `? (unverified)` until locked by decomp or a user identity pass.

**Etymology (user receipt 2026-07-22, bus §104):** "Ivan" was the colour/lighting
test subject during demo-item work. **`Ivan` is a greplist marker — the nickname
must never appear in shipping code.** The lighting recipe once nicknamed
"Ivan/boots" is the **NEUTRAL-AMBIENT RECIPE**: `settingTevStruct(TEV_TYPE 0)` +
neutral ambient, no MAJI, no warm tint (`d_a_demo_item.cpp:519` era).

**See also [§ ORIGIN](#origin--where-these-laws-come-from-the-users-account-verbatim):**
the Accidental Ivan is the same subject, and the reason this project has both an
identity law and a bake law.

**Verification signature:** every identity in a report is either decomp-cited or
carries `? (unverified)`. No third state.

---

## DN-14 — NEVER put a hypothesis or fan label in a donor-derived filename

**Migrated here 2026-08-16 (user ruling).** User 2026-07-22 (bus §89/§89c).

**Forbidden act:** naming a donor-derived export for what we think it is.
**Donor-derived exports carry the donor's OWN addressing as the filename**
(`IsleLink_0_wave025.wav`). Hypotheses and fan labels never reach a filename;
interpretation goes in a sidecar.

**Mechanism:** a filename is the one piece of metadata that survives every copy,
ferry and re-import — so a guess encoded there outlives the evidence that it was
a guess, and is later read as fact. (Structurally identical to the era problem in
`ESTATE-NAVIGATION.md` §6b: what a name asserts, a reader believes.)

**Verification signature:** every exported filename traces to donor addressing;
every interpretation lives in a sidecar CSV/JSON beside it.
