# Handoff: parry / Focused Arts crash (walk-away after parry)

**For:** the Focused Arts / parry-rework chat.
**From:** the Sumo Outfit chat (2026-06-27). Sumo work is committed (`18ab01e930`) and is **not** the suspect — see "Sumo ties" below.

## Symptom

- **Sumo Outfit OFF**, normal combat. Plain **Ordon** Link.
- Walked up to an enemy, **parried**, walked away — and a beat later the game **crashed**.
- The delay ("parry, then walk away, *then* crash") points at a **deferred cleanup / state transition**, not the parry hit itself: e.g. a parry VFX/SFX teardown, an FA-meter/HUD update, or the FA-gain-on-parry path firing after the action resolves.

## Important context

This was the **first time the parry / Focused Arts system was actually exercised this session**. Those features are gated behind config toggles that had been **off all session**:

- `game.focusedArtsTest` ("Focused Arts Test") — gates FA-on-hit.
- `game.flurryRush` ("FlurryTEST", requires Focused Arts Test) — gates Flurry Rush.

They were re-enabled just before the crash. So this is freshly-exercised WIP from the "parry rework, focused arts, HUD meter baseline" line of work, not something that was running and stable earlier.

## Repro (best guess)

1. Editor → ALBW → enable **Focused Arts Test** (and **FlurryTEST** if relevant).
2. Sumo Outfit **OFF**. Any normal outfit (seen with Ordon).
3. Approach an enemy, **parry**, then move away. Crash follows shortly after.

Not yet confirmed whether it needs FlurryTEST on, a specific enemy, or a successful vs. whiffed parry — worth pinning down first.

## Where to look first

- Parry VFX/SFX lifecycle (spawn on parry, teardown on state exit).
- FA meter / HUD update on parry gain.
- Any model/joint/actor reference in the parry or FA-gain path that could dangle after the parry state ends.
- A fopen-append file trace through the parry → FA-gain → cleanup path will localize it fast (the sumo chat's trace infra was removed, so add your own).

## Sumo ties: **not likely** (but not fully ruled out)

- `dAlbwSumoTest_exec()` **early-returns the moment the Sumo Outfit setting is off**, so the sumo module is completely inert during normal (sumo-off) combat. The crash was sumo-off.
- The only sumo-related change that touches a non-sumo code path is the **equipment-joint remap** in `daAlink_c::changeLink()` (`src/d/actor/d_a_alink_wolf.inc`). As of `18ab01e930` it overrides the hand/item joints **only when the sumo body flag (`FLG2_UNK_80000`) is set** — it does nothing for normal clothes, and normal-play equipment placement was verified correct. A parry doesn't run `changeLink`.
- So sumo involvement is improbable. If you want to fully exclude it: reproduce on a build/commit *before* the sumo equipment work (`cb68a9ad46`), or temporarily stub `changeLink`'s sumo `if (checkNoResetFlg2(FLG2_UNK_80000))` block — equipment placement should be unaffected since that block is sumo-only.

## Build

`cmd /c build_run.bat` → `build/windows-msvc-relwithdebinfo/dusklight.exe` (RelWithDebInfo).

---

## FA chat investigation (2026-06-27)

### Crash logs (same evening as handoff)

Ten sessions under `%APPDATA%\TwilitRealm\Dusklight\logs\` on 2026-06-27. Two recurring signatures:

| Signature | Fault addr | When | Likely layer |
|-----------|------------|------|--------------|
| **A** (builds 354–358) | `0xffffffffffffffff` | Often mid-field, no initTexObj burst | Null deref — suspect `daPy_getPlayerActorClass()` in `lopFaMeterActive()` (FA lilac meter draw) |
| **B** (build 359) | `0x25800010039` (garbage) | Right after long `initTexObj` burst (24×24 hakusha spur icons) | `JUTTexture::initTexObj` during bash-charge HUD draw — parry adds charge → linger keeps HUD alive ~4s after walk-away |

Logs also show **config saves mid-session** (editor toggles) and **clothes model reloads** (`ml.bmd`, `zl.bmd`) — matches “freshly re-enabled FA/FlurryTEST this session.”

### Parry path (confirmed)

Successful parry in `d_a_alink_damage.inc` → `dShield_onShieldHit` → `dMeter2_addALBWFraction(1, 6)` + bash charge++. No direct Flurry hook on parry. Parry VFX (`dShield_playParrySuccessFeedback`) is **off** (`kTestingParryReworkEnabled = false`).

Deferred window that fits “parry, walk away, then crash”:

1. **Shield bash HUD linger** — 120 frames after guard ends; draw calls `applyParryIcons` / hakusha `draw` (initTexObj spam in logs).
2. **FA lilac meter** — first draw after enabling `focusedArtsTest` mid-session (was not calling `dFocusedArts_onStageLoad` on toggle).
3. **Orphan Flurry state** — if FlurryTEST was on and a perfect dodge armed rush then you walked off Z-lock, `dFlurryRush_update` kept 0.1× sim + `keepLock` until start gate expired.

### Sumo: still not the lead suspect

Handoff reasoning holds. Logs show clothes-arc reload around crashes, but that is generic `setClothesChange` / outfit work, not sumo-on combat.

### Fixes landed (FA chat, same session)

- `lopFaMeterActive()` — null-check player before wolf test.
- `dFocusedArts_update()` — reset runtime bank/fill when `focusedArtsTest` toggles mid-session.
- `dFlurryRush_update()` — end rush + restore 1.0× sim when FlurryTEST off, Z-lock lost pre-attack, or existing target-lost paths.
- `dShield_drawBashCharges()` — tear down bash HUD when parry combat toggled off; validate itemicon TIMG before `changeTexture`.

### Repro matrix still worth pinning

1. FA on, FlurryTEST **off**, parry only → walk away (isolates bash HUD + ALBW fraction).
2. FA + FlurryTEST **on**, parry only (no dodge) → walk away.
3. Same with **Spur Only** vs **Spur+Shield** parry icon mode (isolates `applyParryIcons`).

If still crashes, add a short-lived `OS_REPORT` trace: parry hit → charge++ → linger start → bash draw enter/exit → FA meter draw.
