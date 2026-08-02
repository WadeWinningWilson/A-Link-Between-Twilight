# WW Restoration Cookbook — CANONICAL

> **📦 ARCHIVAL — user ruling 2026-07-27 (Librarian-stamped).** Retained as the **record of how the
> project's restoration work began**. It proved too truncated/catch-all to capture the project's full
> scope, so **going-forward "how it works" documentation is per-system** in
> [gaming systems/](gaming%20systems/README.md) — not this one file. No new *process* content is added here.
>
> **⚠ LAWS STILL IN FORCE — not dead doctrine.** The standing laws in `## ADMITTED DOCTRINE` below
> (Receiver Covenant, №31 / №31-B / №31-C, the Ivan rule, §113-STASH LAW, donor-export naming) are
> **live and cited elsewhere** (DN-2, the gate/verify tooling, LIBRARIAN-QUEUE). Archival status applies
> to the process-manual *framing*, **NOT** to these laws. **Librarian flag → user (pending):** the
> permanent home for these laws and the pending-corroboration index, now that per-system docs are the
> model — migrate to a dedicated doctrine doc / DO-NOT.md, or keep citing them from here? Not my call.

**This is the only writable cookbook.** The two prior copies forked (542 diff lines, bus §106) and
neither can prove accuracy; both are frozen and marked `UNCERTAIN ACCURACY — DO NOT WRITE`:
[docs/WW-Restoration-Cookbook.md](WW-Restoration-Cookbook.md) ·
[docs/WW Linked/WW-Restoration-Cookbook.md](WW%20Linked/WW-Restoration-Cookbook.md).

**Admission protocol (user-ruled 2026-07-23):** an entry enters this book only via one of —
1. **RECEIPT** — a traceable ratification (user words + bus §/№ + date);
2. **BILATERAL** — present in both frozen copies with equivalent meaning (still cite where);
3. **RE-VERIFICATION** — re-proven against source (decomp/tool/measurement), cited.

Entries below carry their admission route. Anything not yet here is *pending*, not void — see the
corroboration index at the bottom.

---

## ADMITTED DOCTRINE

### The Receiver Covenant + Space-Purity core (№31)
**Route: RECEIPT** — user's founding decree (Housing charter, session ba0af71d; enforced all
session; encoded as a tool in commit `16ec60c842`).
- The vanilla exe + game tree contain ZERO WW content — no WW bytes, no WW file names, no WW
  dialogue strings. All WW content lives ONLY in the mod folder. `greplist.txt` gates builds
  (`ww_bridge gate`; M5a/M6).
- WW spaces get only WW assets; TP spaces only TP assets; no cross-pollination ever (rupees
  included — WW Vlupy visual, TP wallet credit). A missing prop is always preferable to a foreign
  one.
- NEVER COMMIT: WW arcs, anything under the mod folder, WW-named files.

### №31-B — Purity covers SHAPING, not just supply
**Route: RECEIPT** — user 2026-07-20: "№31 should be written for assets, music, lighting,
everything." Any receiver stage that modifies donor content on its way to the player is a purity
surface. Corollary: verify at the stage the player perceives, not the stage easiest to hash.
(Case receipts: grass colour via TP kankyo; TP velocity curve squaring donor audio.)

### №31-C — A check that cannot run must report UNKNOWN, never CLEAN
**Route: RECEIPT** — user 2026-07-21/22: "Put it in cookbook" (bus §61/§61c/§74 lineage). Green
must prove the check executed. Corollary: a pass must state what it inspected (path, count,
version) so it is falsifiable from its own output. (Origin: silent ported guards + the gate's own
empty-greplist false-CLEAN, fixed Bridge 0.19.0.)

### IVAN RULE — no invented identity labels
**Route: BILATERAL + RECEIPT** (referenced in both frozen copies; enforced/ratified continuously;
violation case receipts bus §47, §89). Names and resemblance are never evidence. Labels stay
`? (unverified)` until locked by decomp or user identity pass.
- *Etymology (user receipt 2026-07-22, bus §104):* "Ivan" was the color test — the color/lighting
  test subject's name during the demo-item work. `Ivan` is a greplist marker: the nickname must
  never appear in shipping code. The lighting recipe formerly nicknamed "Ivan/boots" is the
  **NEUTRAL-AMBIENT RECIPE**: `settingTevStruct(TEV_TYPE 0)` + neutral ambient, no MAJI, no warm
  tint (`d_a_demo_item.cpp:519` era).

### Donor-export naming law
**Route: RECEIPT** — user 2026-07-22 (bus §89/§89c; memory `donor-export-naming`). Donor-derived
exports carry the donor's own addressing as filenames (`IsleLink_0_wave025.wav`); hypothesis and
fan labels never reach filenames; interpretation goes in sidecars.

### Document-lifecycle rules (this book's own hygiene)
**Route: RECEIPT** — user rulings 2026-07-23 (bus §106/§107):
- Before assigning a bus §-number, grep the tip for the current maximum.
- Doctrine edits happen HERE only. Frozen copies are read-only sources.
- "PROVEN PATH" pointers must carry date + version; consumers diff against current standard
  before cloning (assessment rec 3, user-accepted by adoption of this book).


### №113-STASH LAW — PAL0 plight_col[2] is the sea-K0 stash: never "repair" it
**Route: RECEIPT** — user ratification 2026-07-26 (bus §148/§149), after the trap bit TWICE
(№270 restore ↔ §112 re-inject: an inter-lane fix-revert loop).
- F_DL01's `STG_00.arc` PAL0 **deliberately carries the sea K0 color stashed in `plight_col[2]`**
  (№113 convert; read by `dKy_get_seacolor` via `dungeonlight_col[2]` — `d_kankyo.cpp:9458`).
- **SCOPE EXTENSION (2026-07-27, bus §182):** slot **`plight_col[1]` = BG0_K0** is ALSO a №113
  stash and load-bearing (windline alpha `d_kankyo_wether.cpp:1559` + grass cut-VFX color, Ferry
  V-b). The whitelist/carry-forward duty covers BOTH slots [1] and [2], not [2] alone.
- **It LOOKS like garbage** as a point-light color. **It is LOAD-BEARING** — the wave panes' and
  the shore water's blue both die without it.
- Therefore: no lane "fixes" strange plight2 values in this PAL0; hash/diff tooling must WHITELIST
  the stash bytes; any PAL0 restore must carry the stash forward.
- **Valid-exception clause (user, at ratification):** the law yields only if (a) we ERRED — the
  stash itself is shown wrong against the donor, or (b) WW VANILLA is shown to require that slot —
  in either case: evidence first, cross-lane sign-off (Engine + History + Housing), THEN change,
  and this entry is updated — never a silent unilateral "repair."
- Ratchet target (§106): graduate to a tool check (stash-presence assert in the gate/verify path)
  so the law enforces itself.

### DIRECT-PORT CRASH RECIPES — WW actor → port (the pig playbook)
**Route: RECEIPT** — every item below was symbolicated + playtest-confirmed on the `d_a_kb` (pig)
direct port, 2026-07-29 (bus §229–§234). Full detail + code sites: `WW Linked/direct-port-plan.md`.
Any future WW-actor direct port (Outset NPCs, seagulls, enemies) hits these — apply pre-emptively,
do NOT re-discover each crash:
1. **Model = raw at consume (DN-3).** `getObjectRes(arc, model)` returns RAW bytes, not a fixed
   `J3DModelData`. Route through `dExtNpcMount_acquireModelData(arc, name)` (parse-at-consume) or
   McaMorf derefs `getMaterialNodePointer(0)` on raw → AV. Anims (BCK/BAS) are fine raw (morf binds).
2. **Render needs `modelCalc`, not just `entryDL`.** The port SPLIT donor `mDoExt_McaMorf::updateDL()`
   into `modelCalc()` (setFrame+setMtxCalc+`model->calc()`) and `entryDL()`; `play()` does NOT calc.
   Draw must do `modelCalc()` → `dComIfGd_setList()` → `entryDL()`, else invisible (degenerate joints).
3. **64-bit pointer truncation.** Donor `setUserArea((u32)this)` chops the actor ptr on x86_64 →
   joint callback reads a wild ptr → AV. Use `(uintptr_t)this`. Audit every `(u32)`/`(s32)` cast of a
   pointer in ported donor code.
4. **Carry orientation.** Link's `d_a_alink_grab.inc` forces `grabActor->shape_angle.x = shape_angle.x`
   every carry frame (TP holds objects upright). A donor actor carried in a special pose (WW pig =
   upside-down) must be excepted by proc name so it keeps its own `carry_move` orientation.
5. **BMT color = material-only swap.** WW `.bmt` color variants (pig `pg_*.bmt`) carry NO `TEX1` — the
   color is a material TEV/konst value (K2 = spot layer). Bake via `acquireModelDataBmt`, but GUARD
   `replaceTextures` on the bmt actually having a `TEX1` chunk, else it swaps in an empty texture table
   → materials' texture indices out of range → `indexToPtr` AV. The material copy alone applies color.
6. **Water-check offset spans the port's deep sea.** Donor `SetWaterCheckOffset` values assume WW's
   shallow near-shore water; the port's Outset sea is a ~5000-unit column. Use the port swimmers' value
   (Link/`d_a_e_fs` = 10000) or a thrown actor free-falls through the surface to the sea floor.
7. **P13 gate is the acceptance instrument.** `duskStateTap(tag, action, mode)` on transition →
   `dusktap_to_jsonl.py` → `state_gate.py` vs the donor statemap. MATCH = every entered state value is
   donor-legal. (`run_state_tap.bat` arms it.) Actors with NPC set_action states use `state_gate.py
   --npc` (roster gate); actors with no discrete states (flight-math, e.g. seagull) need no tap.
8. **HIO (debug tuning) actors → use the MACROS, not the raw calls.** Donor actors with a
   `JORReflexible` HIO class (seagull `kamomeHIO_c`) call `mDoHIO_createChild`/`mDoHIO_deleteChild`
   directly — these reference the debug-only `mDoHIO_root`, UNLINKED in the retail PC build →
   LNK2019. Use the port's `mDoHIO_CREATE_CHILD(name,node)` / `mDoHIO_DELETE_CHILD(no)` macros
   (no-op `(-1)`/`(void)0` in non-DEBUG). The HIO tuning FIELDS (`l_xHIO.mNN`) are fine to read —
   only the register/unregister CALLS need the macro.
9. **PRE-PORT GATE — is the donor actually DECOMPILED?** `zeldaret/tww` is ~72% done; ~half the
   NPCs are un-decompiled SKELETONS (every body `/* Nonmatching */`, class members an unnamed
   `u8 m290[…]` blob). There is nothing to port — copying verbatim emits non-compiling empty
   functions. **Check BEFORE spawning any port:** `grep -c "Nonmatching" <donor>.cpp` — <20 ≈
   decompiled, ≥20 = stub (seagull=0, pig=3; stubs=73-134). A stub also **fools the base-class
   tier**: its scaffold header defaults to `public fopAc_ac_c` and the kit reports "100% coverage,
   0 missing" (it references nothing external because it does nothing). So the roster needs TWO
   gates — base-class AND decompiled — and for the WW Outset villagers they are ANTI-correlated
   (the `fopAc_ac_c` ones are stubs; the decompiled ones are `fopNpc_npc_c`). Un-decompiled donors
   need upstream decomp (Bridge lane), not a History direct-port. STOP + report, never fabricate.

### CUTSCENE-NPC RECIPES — the second playbook (a WW NPC that binds to a `.stb` storyboard)
**Route: RECEIPT** — symbolicated + playtest-confirmed on `d_a_npc_ls1` (Aryll), 2026-07-30 (bus §247).
Recipes 1–9 make an NPC *exist*; these make it *perform in a cutscene*. Full detail:
`WW Linked/port-kits/HISTORY-KIT-FEEDBACK.md` (Aryll saga). **Rule zero: for a cutscene actor the
`demo_*` / `getDemo*` shims are LOAD-BEARING, not inert — a NULL that is safe for a non-demoing actor
(bm1) crashes or blanks a cutscene actor.**
10. **Demo binding = the vanilla `l_objectName` row, not a mount stand-in.** A storyboard binds its cast
    via `JSGFindObject(name)` → `dStage_searchName(name)` → `(procName, argument)` → a live actor
    matching BOTH. Restore the donor's `d_stage.cpp` row verbatim (`OBJNAME("Ls1", fpcNm_NPC_LS1_e,
    255)`); `argument` is `s8`, so `255 == (s8)-1`, matching the census `fopAcM_create(…, -1)`. If the
    port row points the name at the `NPC_HENNA0` audition-mount host, replace it once the native actor
    exists.
11. **`dComIfGp_demo_getActor` must return the real actor.** Shimming it to NULL (fine for non-demoing
    actors) NULL-derefs in a live `demo()`. Return `dDemo_c::getActor(id)` (guard `id == 0`).
12. **`dDemo_setDemoData` must set `current.angle`, not only `shape_angle`.** WW NPCs render from
    `mAngle = current.angle` in `setMtx()`. The port's `d_demo.cpp` dropped `current.angle` from the
    `ENABLE_ROTATE` branch → cutscene orientation computed then discarded. Donor sets
    `current.angle = shape_angle = getRatate()`. (Shared engine file — fixes every WW demo actor.)
13. **Face render ORDER: `modelCalc()` BEFORE the btp/btk entry.** `modelCalc()` re-runs the material
    calc and resets the texture pattern, so entering the face btp/btk *before* it blanks the eye/mouth
    panes. Order: `modelCalc → btp.entry → btk.entry → setList → entryDL → btk.remove → btp.remove`.
14. **Cross-actor model arc = silent create-hang.** WW loads some models from a *sister* arc (Aryll's
    telescope from Link's arc). The port has no plain `Link` arc (only `LinkRM`/`LinkUG`); the mod
    bundles the model in the actor's own arc. `acquireModelData("Link", …)` → NULL → a `CreateHeap`
    sub-step fails → `_create` returns `cPhs_ERROR` forever → actor torn down with **no crash, no log**.
    Source cross-actor models from the actor's own `mArcName`. Verify every arc string resolves.
15. **Scripted face expressions = the demo prm channel (donor `getP_BtpData` is a Nonmatching stub).**
    Per-beat expressions live in the DEMO archive (`Demo02`, `dStage_roomControl_c::getDemoArcName()`),
    not the actor's arc. Drain `dDemo_actor_c::getDemoIDData` (one full loop resets it); facial entries
    are tagged `(0,0,1)` — FIRST = BTP, SECOND = BTK; resolve each via `getObjectIDRes(demoArc, resID)`.
    **Rebind only on resID CHANGE** or the per-frame `demo()` re-init freezes the face at frame 0. The
    audition mount already proved this (`d_ext_npc_mount.cpp:8908`); the native resolver lifts its logic.
    (The stub can be *finished* from this behavioral spec — `mBtpId`/`mBtkId` @0x60/0x64 are the decode
    lead: init'd to -1, never set by the port.)

---

## PENDING CORROBORATION INDEX (not yet law — verify, then move up)

| candidate | lives in | corroboration route |
|---|---|---|
| **Presentation-Parity Principle (№255/№256)** | root copy only | History verifies against №-ledger №255/№256 receipts → RECEIPT |
| **Baseline-Gate Law (FPS)** — "an FPS measurement without a baseline check is not a measurement"; oracle set: menu ≈288 / Hyrule Field 250+; gate before any FPS session | proposed bus §153; used successfully §155 | awaiting explicit user ratification |
| №31-D "completeness before attribution" | proposed bus §82 | awaiting explicit user ratification |
| DECOMP-FIRST banner (№177–№181 case) | linked copy | matches memory `reference_ww_extract`; History confirms ledger №176–№181 → RECEIPT |
| OffsetPos banner (№165/№175) | linked copy | History confirms ledger entries → RECEIPT |
| №22 split transform, №39 division of labor, FAIL-CLOSED, №50-B, remaining §0 doctrine | linked copy | per-entry ledger receipts (History) or re-verification (owning lane) |
| Process/pipeline chapters (steps 0–N), scaling notes | both copies, diverged | owning lanes re-verify against current practice; stale steps die here instead of migrating |

**Assignment:** each lane corroborates the entries it owns; the user ratifies on receipt; Housing
moves ratified entries up and records each move in the bus. Migration is incremental — no
deadline theater; the index shrinking is the metric.
