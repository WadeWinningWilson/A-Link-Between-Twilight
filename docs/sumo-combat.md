# Sumo Combat — design & state handoff

**Status (2026-06-27):** SHIPPED & committed. The Sumo Outfit is a shop-bought, save-backed outfit that swaps Link's body to the shirtless sumo model, persists across transitions/warps/cutscenes, and is wired into the D-pad outfit quick-swap cycle. Equipment draws, the Link Hat works, and the sumo↔native / warp / cutscene crashes are fixed (commit `89ac586434`). Remaining bugs are tracked below (§ Current state) and in the interconnected doc. The historical spike notes further down are kept for reference (BLS skeleton, draw-gate fix) but predate the shop/cycle/crash work.

**Chat origin:** ALBW shop-navigation feature work (Claude). Broader sumo research/spike notes were kept in that chat's handoff.

**Interconnected chat (outfit cycle + quick-swap):** [Quick-Sumo Work.md](Interconnected%20Chats/Quick-Sumo%20Work.md) — shared API contract, save bits, cross-chat messages, the crash post-mortem, and the live bug list with the D-Pad Quick Swap session.

---

## Current state (2026-06-27) — shop outfit + quick-swap cycle

**Architecture.** Two modules:
- `d_albw_sumo_test.{h,cpp}` — the sumo *overlay* (model-swap state). `dAlbwSumoTest_exec()` runs per-frame from `daAlink_c::execute()`; drives `setClothesChange` + the `FLG2_UNK_200000/80000` flags only (never the sumo minigame). Owns arc residency (`alSumou`, cap `Kmdl`, base clothes) and the stage-transition reset.
- `d_albw_outfit.{h,cpp}` — the unified wardrobe API consumed by the D-pad cycle: `dAlbwOutfit_isOwned/equip/getActive/getNextOwned/isActive`, `setSumoWorn/isSumoWorn`, `syncWornOwnership`, and the `syncLinkModel`/`processPendingEquip` state machine (largely the Quick Swap chat's). Co-authored.

**Worn state** is per-save **bit 700** (not the old `game.sumoOutfit` AppData toggle, which is retired). Ownership = wardrobe stash bits 689 (sumo) / 691–694 (Ordon/Hero's/Zora/Magic); "own what you wear" seeds them. Cap is `game.sumoOutfitHat` (Editor toggle still visible); Fists Only = `game.sumoOutfitFists`. The master Sumo Outfit editor toggle was removed (shop + d-pad drive it).

**Crash fixes (commit `89ac586434`).** Root: `setClothesChange(0)` never clears `FLG2_UNK_200000`, so leaving sumo kept `loadModelDVD` on its **skip-path** (no clothes-arc reload) → `changeLink` built from a non-resident arc → crash. **Fix:** `applyTargetKind` clears `FLG2_UNK_200000 + FLG2_UNK_80000` *before* `setCloth` when leaving sumo, so the change runs `loadModelDVD`'s normal reload path (`resDelete`+`freeAll`+`setArcName`+reload, self-heals dangling arcs via `deleteObjectResMain`). Also: `nativeClothesResourcesReady()` resets the phase on equipped-clothes change; `setClothesChange` re-entrant guard. Reverted the `getRes`/face null-guards (band-aids that only relocated the crash).

**Verified crash-free:** quick-swap across all armors, room transitions, warps, cutscenes.

### Bug status (2026-06-27)
**Fixed this batch:** the **dual-load removal** (`nativeClothesResourcesReady()` no longer double-loads clothes arcs alongside Link's `mPhaseReq`) was the big lever — it killed the cross-base/Zora crash, the **random teleport** (#2), and unblocked **cutscene swapping** (#5). Plus the **A+C** no-op fix (#1/#6: `sLeavingSumoReload` forces one rebuild on a same-base leave) and the **weapon-flicker** fix (#3: `sShowWeapons` driven by worn intent `want && !fists`, computed through the transition, not the live `has`).
- ✅ #1 2-owned no-op · ✅ #3 weapon flicker · ✅ #4 chin strap (sumo face → Kmdl `al_face`) · ✅ #5 cutscene swap · ✅ #6 post-Zora break · ⚠️ #2 teleport: fixed for normal play, **reopened iron-boots-only**

**#4 chin strap fix:** sumo over a **Zora** base used `zl_face.bmd` (the sumo branch clears `FLG2_UNK_200000` before `changeLink`'s face block, so the condition fell to the Zora `else`). Fix: while `FLG2_UNK_80000` is set, build the face from **`al_face.bmd` in `Kmdl`** (now kept resident through sumo regardless of the hat — see `resourcesReady`), with the per-base path as fallback so it's never null.

### Open bug (Sumo-owned)
- **Distance-skip / teleport — IRON BOOTS only.** With iron boots on, **every** d-pad quick-swap flings Link a large distance **in his facing/camera direction** (consistent), for every outfit; removing the iron boots stops it entirely. Normal play is fine (dual-load fix handled that). Theory: our quick-swap runs `changeLink` (which rebuilds the iron-boots model `al_bootsH.bmd` + the heavy-boots collision state) during *active* play, whereas vanilla clothes changes are always **paused** — the rebuild collides with the live heavy-boots ground-stick → directional launch. Next: add a position log (`current.pos` before/after swap) and investigate.

---

## Agent onboarding (read before coding)

| Doc | Why |
|-----|-----|
| `docs/build-fps-guidelines.md` | Canonical build (`build_run.bat` / RelWithDebInfo), launch hygiene, never-commit list, GPU-cache crash addendum (2026-06-26) |
| `docs/commit-and-push.md` | Push to **upstream** (`WadeWinningWilson/A-Link-Between-Dusklight`), not origin; stage rules |

**Hard rules:** build with `build_run.bat` only; don't commit/push unless the user asks; FPS-chat handoff only if something breaks or FPS drops.

---

## What's implemented (builds clean, UNCOMMITTED)

### The Sumo Outfit toggles (Editor → ALBW → "Sumo Outfit" section)
- `game.sumoOutfit` (master on/off), `game.sumoOutfitHat` (wear Link's cap vs sumo headpiece), `game.sumoOutfitFists` (hide weapons, bare-knuckle) — `include/dusk/settings.h` + `src/dusk/settings.cpp`.
- UI: **RmlUi Editor → ALBW tab** (`src/dusk/ui/editor.cpp`), under a `add_section("Sumo Outfit")` group, three `editor_bool_option`s (Hat/Fists disabled when the master is off). (NOT the imgui Tools/Debug menu bar. Renamed from the earlier single "SumoTest" toggle / `game.sumoTest`.)
- **Fists Only:** `dAlbwSumoTest_showWeapons()` = applied && !fists (cached per frame); `checkSwordDraw`/`checkShieldDraw` use it to draw/hide the gear.
- **Link Hat — PARKED (crashes).** Attempt: `changeLink` sumo branch loads `Kmdl/al_head.bmd` (Link's cap) instead of `alSumou 0x33`. Result: crash. ROOT-CAUSE LEAD: the hat draws via `daAlink_headModelCallBack` → `headModelCallBack(joint_no)` (d_a_alink.cpp:2482), which indexes fixed per-joint arrays (`field_0x3040[joint_no]`, `field_0x302c[joint_no]`, `field_0x3066[joint_no-7]`). Link's cap interacts with that callback differently on the BLS body than the sumo headpiece (which has only 6 head joints, never hitting the `joint_no>=6` branch). Reverted the cap load + toggle + setting for stability. To revisit: trace headModelCallBack / setHatAngle with the cap's joints on the BLS body, or load the cap as a separate overlay model with its own callback. Effort: moderate, focused crash investigation.

### The model swap module
- `include/d/d_albw_sumo_test.h` + `src/d/d_albw_sumo_test.cpp` (`#if TARGET_PC`, in `files.cmake`).
- `dAlbwSumoTest_exec(daAlink_c*)` — per-frame driver, hooked at the top of `daAlink_c::execute()` (`src/d/actor/d_a_alink.cpp`, before `loadModelDVD()`).
- Logic: when `game.sumoTest` on & not applied, ensure `dComIfG_getObjectRes("alSumou", 0x31)` resident (else `dComIfG_resLoad(&sPhase, "alSumou")`, phased, retry next frame), then `setClothesChange(1)` → sets `FLG2_UNK_200000` → existing `loadModelDVD`/`changeLink` loads the sumo body/hat/hands (res 0x31/0x33/0x32). When off & applied, `setClothesChange(0)` reverts to the player's equipped clothes. Drives ONLY the model flag — never `setSumouReady`/the sumo minigame procs. Crash-safe (never swaps until the archive is resident).
- Limitation: the sumo flag is one-shot, so the look reverts on a stage transition — re-toggle to re-apply.

### Equipment joint remap (in place, but see Known Issue)
- `changeLink()` (`src/d/actor/d_a_alink_wolf.inc`, after the model-load branch): when `FLG2_UNK_80000` (sumo) set, remap `mLeftHandJntNo=9, mRightHandJntNo=14, mLeftItemJntNo=10, mRightItemJntNo=15` (BLS hand/weapon joints); else restore Link's `19/24`.

---

## Key discovery — the sumo body uses a DIFFERENT skeleton ("BLS")

`assets/*/res/Object/alSumou.h` defines `enum BLS_JNT`. The sumo body (BMD res 0x31) is NOT Link's normal skeleton:
- Animations still play correctly because **J3D binds animation by joint NAME**.
- But **equipment attaches by joint INDEX** (`setItemMatrix()` in d_a_alink.cpp uses `mpLinkModel->getAnmMtx(jointIndex)`): sword←`mLeftItemJntNo` (5931), shield←`mRightItemJntNo` (5967), sheath←`field_0x30b6` (5920).
- BLS indices: `HANDL=9, HANDR=14, WEAPONL=10, WEAPONR=15, WAIST=0x10, HEAD=4, BACKBONE2=2, POD=5`. Link's body uses item joints **19/24** — which on BLS are the **LEGS** (`LEGL2`/`LEGR1`). Hence the gear was buried in the legs.

---

## DRAW ISSUE — ROOT-CAUSED + FIXED (2026-06-26)

**Root cause:** the equipment draw in `daAlink_c::draw()` (d_a_alink.cpp human branch ~20302/20312) is gated by `checkSwordDraw()` (19641) and `checkShieldDraw()` (19646), whose suppression masks **fold in the sumo body flag**:
- `checkSwordDraw`: `!checkNoResetFlg2(FLG2_UNK_2080000)` where `FLG2_UNK_2080000 = 0x2000000 | 0x80000`.
- `checkShieldDraw`: `!checkNoResetFlg2(FLG2_UNK_4080000)` where `FLG2_UNK_4080000 = 0x4000000 | 0x80000`.
`FLG2_UNK_80000` (= 0x80000) is the sumo-body flag, so the moment the sumo body is on, both checks return false → sword/shield not drawn at all. (Intentional for the minigame; wrong for an outfit.) The joint remap was correct but moot while the draw is gated.

**Fix:** `checkSwordDraw`/`checkShieldDraw` (d_a_alink.cpp) now, when `dAlbwSumoTest_isOutfitActive()` (cheap bool = module `sApplied`, NOT a per-frame settings read), use `FLG2_UNK_2000000` / `FLG2_UNK_4000000` instead — i.e. drop just the sumo bit, keep the other suppression. Distinguishes the SumoTest outfit from the real minigame (minigame doesn't set `sApplied`, so its weapon-hiding is preserved). Added `dAlbwSumoTest_isOutfitActive()` to the sumo-test module.

The joint remap (items → BLS 10/15) stays — now the gear both DRAWS and attaches at the right joints.

POSSIBLE FOLLOW-UP: held items (bow etc.) draw via `checkItemDraw()` (19652) — if those still hide on sumo, check it for a similar flag gate. Also the back-sheath joint (`field_0x30b6`, left at human=2) — verify position, BLS POD=5 if off.

---

## Feature ideas (future passes)

1. **"Fists only" Sumo combat** (user idea): keep a no-weapons sumo mode for bare-knuckle/grapple combat — i.e. the current "invisible equipment" behavior becomes an intentional toggle, distinct from a weapons-visible mode once the draw issue is solved.
2. **HAT ON / OFF** (feasibility CONFIRMED, not yet built): wear Link's cap on the sumo body. The sumo branch loads the hat from `alSumou 0x33` (topknot); load `dComIfG_getObjectRes(l_kArcName /*"Kmdl"*/, "al_head.bmd")` instead for Link's cap — Kmdl is already resident (mArcName=Kmdl for sumo) and the cap attaches via its own joints to the BLS head joint (HEAD=4). Plan: convert `game.sumoTest` to `enum SumoTestMode { Off, HatOff, HatOn }` + an editor select-button (like FA Cheat / True ALBW Mode), branch the hat load on the mode.
3. **Full Sumo OUTFIT** (the real feature, EASY track — no animation rework needed): new clothes slot `dItemNo_WEAR_SUMO_e` + `checkSumouWearFlg()`, archive residency for `alSumou`, persistence (avoid the one-shot revert), and the shop Armors-tab row at the Ordon → Hero → **Sumo** → Zora → Magic → Deity order. The equipment-draw fix + joint remap are tasks within it.

---

## Files touched (this work)

| File | Change |
|------|--------|
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.sumoTest` bool |
| `src/dusk/ui/editor.cpp` | SumoTest toggle in the ALBW tab |
| `include/d/d_albw_sumo_test.h`, `src/d/d_albw_sumo_test.cpp` | swap module (new) |
| `src/d/actor/d_a_alink.cpp` | `dAlbwSumoTest_exec(this)` hook in `execute()`; `#include d_albw_sumo_test.h` |
| `src/d/actor/d_a_alink_wolf.inc` | `changeLink` equipment joint remap (BLS vs human) |
| `files.cmake` | added `src/d/d_albw_sumo_test.cpp` |
