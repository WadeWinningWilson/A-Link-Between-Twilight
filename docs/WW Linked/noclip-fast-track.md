# noclip fast-track — formalizing systems ahead of port attempts

**The strategy (user-decreed, 2026-07-26):** noclip is NOT the end-all reference — but it is a
**fast track**: for any presentation system we haven't ported yet, read noclip's working TS
implementation FIRST to formalize the system's shape (inputs, data flow, per-frame behavior),
THEN verify against the decomp, THEN spec the port. This inverts the old order (rudimentary
attempt → debug rounds → decomp read → fix) into (working reference → decomp confirm → spec →
one build). The §126-§143 water-color saga cost ~6 rounds; a fast-tracked system should cost 1-2.

**ELEVATED to standard method (user-decreed, 2026-07-26 — CUTSCENES + LAYERS):** for any
cutscene or layer question, noclip is now the FIRST instrument — used as a live
reverse-engineering rig for what the actual WW code *does*, extracted by **object-identity match
against noclip's running scene**, never by eyeballing the render. The rig (F-2 proved it):
1. `main.viewer.scene.globals` → `frameworkGlobals.lyCurr.pcQueue` = the live actor list; each
   actor carries `profName`, `pos`, `roomLayer`, `demoActorID`, and a `morf` (animation ctrl).
2. `globals.objectNameTable` maps `pcName`→name (resolve `profName` → "Ba1", "Lamp", …).
3. `globals.resCtrl.resObj` = loaded arcs; each `{name, archive.files[], res[]}` is index-aligned
   (res[i] ↔ archive.files[i]).
4. Match a live `actor.morf.anm` (or any resource) by **`===` identity** across every arc's
   `res[i].res` → you get the EXACT arc + file the game bound, with zero inference. Cross-check by
   `duration`/`loopMode`/joint count.
5. `globals.scnPlay.demo` (`dDemo_c`) = the STB player: `.frame`, `.control`, `.parser`.
This is how F-2 proved Grandma's give-scene pose lives in `Demo01.arc`, not her actor arc — a fact
no screenshot could have told us. **The identity match IS the reverse-engineering** — it reads
what the game's own binding says. Still bound by the tier + layer rules below (contents, not
activation).

**Tier rules unchanged** ([noclip-reference.md](noclip-reference.md)): decomp is law; noclip
disambiguates; never port a noclip constant unverified.

## ⚠ KNOWN LIMITS (measured; grows as found)
1. **~~NO STORY LAYERING~~ — CORRECTED (user, 2026-07-26; source-verified §152):** noclip DOES
   expose layers — a Scenarios/Layers UI panel (`WindWakerLayer` class, per-layer visibility bits,
   `objectLayerVisible(layerMask, layer)`) covering room AND demo/cutscene-oriented layers. **You
   can view each ACT layer's placements in isolation** — which upgrades noclip from "unfiltered
   superset" to a LAYER INSPECTION instrument for History. Residual caution: noclip shows what a
   layer CONTAINS; which layer is ACTIVE in a story state is still the №222 flag→ladder (game
   logic noclip doesn't run). Contents = noclip; activation = History.
2. Gameplay absent (AI/collision/audio/save) — §145 inventory.
3. It is a renderer of DEFAULTS — time-of-day and weather are user-controlled, not schedule-driven;
   event-gated states don't occur.

## FINDINGS LOG (each entry: what noclip showed → what it fast-tracks → verification route)

### F-1 (user, 2026-07-26): A_mori — enemy IDLE ANIMATIONS play
`noclip.website/#zww/A_mori` (Forest of Fairies, Outset interior — in our census: A_mori, EVNT 6,
hosted in R_DL01): noclip renders the resident enemies WITH their light idle animations — user
reports explicitly for the **moblins** *(user's word; actor code UNVERIFIED — IVAN: A_mori's
vanilla residents need decomp/census confirmation — Bk/Bokoblin vs Mo/Moblin — before any port
uses an identity)*.
- **Fast-tracks:** the enemy/NPC idle-presentation pattern — which anim a placed enemy plays at
  rest, how noclip binds placement → actor type → default anim. Our interiors currently mount
  enemies (if at all) static; this is the reference for "placed actor plays its idle" — the same
  class as the Outset exterior folk idle work (npc-presence-recipe).
- **Verification route:** noclip `d_a.ts`/`LegacyActor.ts` entry for the actor code seen in
  A_mori's DZR → decomp `d_a_<code>.cpp` anim selection → census identity check → then spec.
- **✅ VERIFIED (History, 2026-07-26 — DZR + decomp brief):** A_mori/Room0 DZR is a MIXED enemy
  set, not one type. Decomp-confirmed identities:
  - **`Bk` ×3 → Bokoblin** (`d_a_bk.cpp` "Enemy - Bokoblin")
  - **`mo2` ×4 → Moblin** (`d_a_mo2.cpp` "Enemy - Moblin / モ２") — the user's "moblins" ARE real, but only 4 of them
  - **`gmos` ×3 → `fpcNm_GM_e`** (a distinct flying enemy; `d_stage.cpp:676` OBJNAME) — NOT a moblin
  - `Bb` ×2 (secondary; identity TBD)
  IVAN outcome: the eye read "moblins"; the DZR shows Moblin ×4 + Bokoblin ×3 + gmos ×3 — porting
  "the moblins" as one actor would have been wrong. **LAYER CAUTION (№222):** these are ACT-layer
  placements (census has A_mori `Bk` in ACT0); noclip's view is the unfiltered SUPERSET — which of
  these actually spawn depends on the story layer, which stays History's call, not noclip's.

### F-2 (History, 2026-07-26): Ba1_Get_Itm give scene — Grandma's poses live in `Demo01.arc`
> **⚠ FRAMING CORRECTED (user, 2026-07-26 — Librarian-stamped):** the pose finding below **stands**
> (Grandma's cradle pose = `ba_wait_l.bck` in `Demo01.arc`). But **`Ba1_Get_Itm` is the SOUP give**
> (prm0=0x55=SOUP_BOTTLE), **not** the clothes give — so the "Ba1_Get_Itm give scene" label above is
> wrong. This pose belongs to the **clothes/tale handover** ("Grandma's Tale", scene `tale`), the
> event the finding's own body cites. Canonical: [Grandma README](islands/Outset/characters/Grandma/README.md) §36–44.

`noclip.website/#zww/LinkRM` → demo **"Grandma's Tale"** (scene id `tale`). Problem: our mount
used `05_ba_cut8_wait.bck` (Ba.arc) for Grandma's start/cradle pose; user confirmed live it did
NOT fit, while the present pose looked right. DP decomp gives no answer — every `d_a_npc_ba1` body
is a Nonmatching stub, no static anim table survives.
- **Method (the elevated identity-match rig above):** located the active Grandma actor (`profName`
  337 = "Ba1", pid 13, pos `-226,375,-56` = our exact NPC_BA spot, `roomVisible:true`), read her
  live `morf.anm`, and matched it by `===` across every loaded arc's `res[i].res`.
- **RESULT (identity-confirmed, zero inference):** her live animation is **`ba_wait_l.bck` from
  `Demo01.arc`** — a purpose-authored DEMO clip, NOT in her actor arc `Ba.arc` at all. (`Ba.arc`'s
  `wait01`/`sleep` share the 60-frame/loop shape but fail the identity + track-signature test;
  `05_ba_cut8_wait` is 49 frames.)
- **Full give-scene set (all in `Demo01.arc`, already shipped as our `arcs_lib/Demo01.arc`):**
  Grandma wait `ba_wait_l.bck`; Grandma present `01b_ba_cut1_waitpresent_l.bck`; clothes-prop
  present `01b_vfuku_cut1_waitpresent_l.bck` (synced); prop turn/unfold `01b_vfuku_cut03_turn_o`,
  `01b_vfuku_cut03_turnwait_l`, `01_roll_fuku`. The give is a COORDINATED Grandma+clothes demo.
- **Fast-tracks:** the get-item/give cutscene animation spec. Fix = Engine adds cross-arc anim
  load (`idle_attached_arc`, mirroring `attach_arc=Vfuku`'s `dComIfG_resLoad` path); then data:
  `idle_attached_arc=Demo01` + `idle_attached=ba_wait_l.bck` in `npc_ba.ini`. Deeper fidelity
  (later): animate the clothes prop via the synced vfuku clips instead of a static attach.
- **Verification route:** DONE at the source — object identity against the running demo IS the
  proof. Confirm on screen after Engine wires the field (per visual-fix rule).

*(Add F-3, F-4… as explorations find more. Each needs: the noclip URL/scene id, what played (or
the identity-matched resource), the fast-track claim, and the verification route. User explorations
in the museum ARE a research instrument — screenshots/reports land here.)*

Cross-refs: [noclip-reference.md](noclip-reference.md) (tier + inventory) ·
[islands/Outset/README](islands/Outset/README.md) (A_mori in the interior census) ·
bus §144-§146.
