# Direct source-port plan — pig (`d_a_kb`) + Bokoblin (`d_a_bk`) (§223, P13c doctrine)

P13c measured the API and chose **direct source port** (pig 92.3% / Bok 85.8% receiver-native). This
doc turns the api_surface missing-lists into an executable plan, and records the reality the raw % hides.

## The reality the coverage % hides: class-methods vs free functions

A "direct port" drops in the donor `.cpp` verbatim. But `modelData->setMaterialTable(...)` (method syntax)
can't be satisfied by adding a free function — it needs the **method on the receiver class**, or the call
site adapted. Verified: the pig's "missing" identifiers are **truly absent** (not renames), and split:

**Pig (14) — free-function shims (5, easy):**
- `dComIfGp_particle_setToon`, `dComIfGp_particle_setShipTail` → toon/wake FX; **no-op stub** first pass
  (cosmetic; the pig loop doesn't depend on them).
- `dSnap_RegistFig` → figurine-collection register; **no-op** (Nintendo Gallery not restored).
- `kb_dig` → the pig's dig interaction on a `daTagKbItem`; **no-op** first pass (bait-dig deferred).
- `fopAcM_getGroundAngle` → **implement** (ground-normal probe; the pig tilts to slopes).

**Pig (14) — class-method shims (6, need a class edit OR a call-site adapter):**
- `J3DModelData::setMaterialTable` (bmt swap — pig size/skin variants) · `cc::OnAtHitBit`/`ClrAtSet`
  (attack-collision bits — only when the pig is hittable) · `dBgS::GetAttributeCode` (ground material
  under the pig) · `daPy_getPlayerActorClass()::getGrabMissActor` (carry/throw) · tex-animator
  `setAnmIndex`. **Strategy:** free-function ADAPTERS (`dExtKb_setMaterialTable(md, …)`) + minimal
  call-site edits in the ported source — keeps the receiver classes untouched (lower risk than editing
  `libs/JSystem`). 3 remaining lower-priority ones (`dComIfGs_getSelectEquip`, `onWindOff`,
  `setTexNoAnimator`) → no-op/stub.

**Then:** bring in `d_a_kb.cpp` (+ `d_a_esa.cpp`) adapted (donor `#include` → receiver headers, donor
types → receiver equivalents), register `fpcNm_KB_e`-equivalent proc (EXT_SPAN/EXT_VEG precedent:
`f_pc_name.h` X-macro next index, `f_pc_profile_lst.cpp` matching slot, `files.cmake`), retire the
hand-rolled `NPC_KB` mount hook. **Compile-iterate** — a 2,639-line drop-in surfaces include/type
mismatches in rounds; that's the bulk of the effort, not the shims.

## Bokoblin (`d_a_bk`) next step — the 31 missing are the COMBAT tier

api_surface confirms the increment split exactly: the Bokoblin's missing identifiers are almost all
**battle framework** — `initBt`/`setBtAttackData`/`setBtNowFrame`/`setBtMaxDis`/`setBkControl` (battle
data), `damage_reaction`/`enemy_fire`/`enemy_ice`/`enemy_piyo_set` (damage+status), `JntHit_create`
(joint hitboxes), `getFindFlag`/`setFindFlag` (detection state), `setMaterialTable`/`checkGrabWear`.
- **Increment A (locomotion+awareness) — SHIPPED** (hand-rolled, built): needs none of these.
- **Increment B/C (attack/damage) — these ARE the shim list.** The direct Bokoblin port is gated on the
  battle-framework shims; not startable as a clean drop-in until they exist. So the Bokoblin's honest
  "next step" is the **detection-state pair** (`getFindFlag`/`setFindFlag`) — the smallest real slice
  toward the donor state machine, upgrading the audition detection into the donor's find-flag latch.

## Honest status + recommendation

The pig direct port is a **focused multi-pass effort** (adapter shims → source drop-in → compile
rounds → verify vs `donor-statemap-kb.md` with the P13 tap). It is not a single clean build. My
**audition-tier AIs stand meanwhile** (pig walks to bait ✓, Bokoblin detects+pursues ✓ — both built,
1:45 exe). Recommendation: commit the pig port as 2–3 dedicated passes (shims+register → drop-in →
compile-fix), and keep the Bokoblin at Increment A + the find-flag slice until the pig port proves the
P13c pipeline end-to-end.

---

## Pass 2 RESULT — GREEN (both direct ports compile & link)

**Status: DONE (code-complete, green build 580/580, `dusklight.exe` linked).** The P13c
direct-port pipeline is proven end-to-end.

### esa (bait, `d_a_esa.cpp`) — 307 lines
- Verbatim donor logic; §224 adaptations: model from mod `Esa.arc` via
  `dExtNpcMount_acquireModelData`, `dComIfG_resLoad(&mPhase,"Esa")`, ripple/water no-op'd
  (bait always ground-lands — faithful-in-effect since the pig only accepts `mState==1`).
- Registered `fpcNm_ESA_e` @ 0x31F. Confirms the pig↔bait contract (`field_0x298` claim +
  `mState`) is byte-faithful — the pig reads exactly these fields.

### kb (pig, `d_a_kb.cpp`) — 2639 lines
- **Resource model — SOLVED.** `dRes_info_c::loadResource()` builds the index table for ANY
  `resLoad`'d arc, so the donor's `getObjectRes("Kb", dRes_INDEX_KB_*)` resolves natively
  against the mod `Kb.arc` (106 KB, extracted). Copied donor `Kb.h` → `assets/GZ2E01/res/Object/`.
  The pig's own `daKb_Create` (`resLoad("Kb")` + `entrySolidHeap(useHeapInit,0x3AB4)`) IS the
  port's mod-arc pattern — no Create rewrite needed.
- Registered `fpcNm_KB_e` @ 0x320, `g_profile_KB`, `files.cmake`.
- **Compile-fix: 5 rounds, ~101→61→6→0 errors.** Cascade root was `cPhs_State`→`cPhs_Step`.
- **Shims (§225–§227 in `d_ext_ww_actor_shims.*`):** ripple/smoke particle no-ops (deferred to
  wave-emit — same missing WW bank as the ship wake), `setShipTail`/`setToon` no-ops, 14× pig
  `JA_SE_*` SFX → `#define 0` (deferred audio; `OBJ_FALL_WATER_S` etc. have Z2SE equivalents to
  wire later), `dItemNo`/`dEvt`/attention-flag/demo-mode RENAMES to port names, collision
  `co_sph_src` enums `#define`d to donor bit-values (port `dCcD_SrcSph` layout byte-identical),
  `cc_at_check`/`getSelectEquip` no-op (pig invulnerable first pass — deferred combat),
  `GetAttributeCode`→0 (dig-smoke picks default color), `fastCreateItem` adapter (port has no
  gravity arg), `changeDemoMode` +3 args, retail `BREG_S` rvalue debug-reg clear neutralized.

### DEFERRED (documented, dead-but-faithful — pig's walk/seek/eat/dig loop is intact)
- Combat (takes no damage), eye-blink tex-anim, size/skin BMT swap, all pig SFX (silent),
  dig-item Tag (`fpcNm_TAG_KB_ITEM_e` sentinel — pig eats real esa bait only), water-pillar
  `scaleY`, `DEMO_SMILE_e`→TP pose #50 (WW-only feed cutscene, cosmetic).

### NOT YET WIRED (verification-gated) — the switch-over
- The native `g_profile_KB` actor is registered + compiling but **not the spawn path.** Pigs
  still spawn via the §222 audition mount-hook (`[Pig] proc=NPC_KB`). To exercise the direct
  port, flip mod `population/actor_map.ini` `[Pig] proc=NPC_KB` → `proc=KB` (one line, reversible).
  **Risk:** unverified new actor does `resLoad`+0x3AB4 solid-heap+model-by-index on the Outset
  load — if the mod `Kb.arc` resource ordering diverges from `Kb.h`, it could crash the room
  load (worse than a missing pig). **Recommend the user pull this trigger on a deliberate
  playtest**, watching for a boot/room-load crash; rollback = revert the one line. Only after
  the native pig spawns+walks should the audition hook be retired.

### §229 CRASH ROOT-CAUSE — DN-3 consume-time parse (reusable for EVERY direct port)

First native-pig spawn crashed on Outset entry. Symbolicated (llvm-symbolizer + fresh
PDB): `daKb_Create → useHeapInit → mDoExt_McaMorf::create` faulted on
`modelData->getMaterialNodePointer(0)` — a wild pointer. **Root cause = DN-3:** this port
NEVER parses BDL/BMD at arc-mount (the §180 Outset regression), so
`dComIfG_getObjectRes(arc, idx)` returns **RAW bytes**, not a fixed `J3DModelData`. The
donor read a pre-fixed model; the direct port must parse at CONSUME time.

**The rule for direct-ported WW actors (Bokoblin next, all future):**
- **Model (BDL/BMD):** never pass a raw `getObjectRes` result to McaMorf / J3DModel.
  Route through `dExtNpcMount_acquireModelData(arc, name)` (getObjectRes +
  acquireMountedModel = parse-at-consume). NULL-guard → graceful missing actor (N31).
- **Anims (BCK/BAS):** raw is FINE — they're bound through the morf/setAnm binder, which
  fixes them (mount-proven). Pass raw.
- **Tex-pattern / material (BTP/BMT):** the mount NEVER calls a method directly on a raw
  resource — it routes through a binder (`btpAnm::init(fixedModel, rawPat)`). Donor code
  that calls `searchUpdateMaterialID`/`setFrame`/`getUpdateMaterialNum` DIRECTLY on a raw
  BTP crashes. Either bind via the port's anim-init, or (if cosmetic + downstream-no-op'd,
  as the pig's eye blink is) NULL the pattern and guard the `->setFrame` call sites, and
  size `mpTexNoAnm` off the FIXED model's `getMaterialNum()`.
- **Diagnostic tell:** "Loading Resource: X.bdl" appears in the log (resLoad succeeded) yet
  it crashes in a J3D method → raw-vs-fixed, not a missing resource.

### §229 two more direct-port gotchas (McaMorf render + 64-bit) — found via symbolication

After the DN-3 model fix the pig SPAWNED but was **invisible**, then (once rendering) **crashed**.
Both root-caused with llvm-symbolizer, both reusable:

1. **McaMorf render — the port SPLIT the donor's `updateDL()`.** Donor
   `mDoExt_McaMorf::updateDL()` bundled joint-calc + draw-submit. The port has NO McaMorf
   `updateDL()`; it split into `modelCalc()` (setFrame + `setMtxCalc` + `mpModel->calc()`)
   and `entryDL()` (submit only), and `play()` does ONLY frameUpdate (NOT calc). A direct
   port that calls `play()`+`entryDL()` never computes the joint matrices → **invisible**.
   Fix = mirror the mount draw (d_ext_npc_mount.cpp:9014/9398/9410):
   `modelCalc()` → `dComIfGd_setList()` → `entryDL()`. (mount comment: "modelCalc() IS WHAT
   MAKES THE BASE MATRIX REAL".) Beware any subagent renaming donor `updateDL`→`entryDL`.

2. **64-bit pointer truncation via `setUserArea((u32)this)`.** Donor is 32-bit GameCube, so
   `(u32)this` held the whole actor pointer stored in the model's user-area and read back in
   a joint callback. On this x86_64 port `(u32)` TRUNCATES → the joint callback (`nodeCallBack`
   → `MtxPosition`/`C_MTXMultVec`) derefs a wild low-32-bit pointer → AV (fault addr looked
   like a 32-bit value, 0x559f9828). Fix = `setUserArea((uintptr_t)this)` (port API is
   `uintptr_t`; every other ported actor already does this). **Audit every direct port for
   `(u32)`/`(s32)` casts of pointers** — the donor's 32-bit int↔pointer puns are latent AVs.
