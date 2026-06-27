# Sumo Combat — design & state handoff

**Status:** WIP / dev spike. A dev-only **SumoTest** toggle swaps Link's body to the shirtless sumo model in the field. Spike **passed** (sumo Link moves/attacks/rides with Link's animations). Open issue: **held equipment (sword/shield/items) does not draw** while the sumo body is worn. Not save-persisting; not committed.

**Chat origin:** ALBW shop-navigation feature work (Claude). Broader sumo research/spike notes were kept in that chat's handoff.

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
