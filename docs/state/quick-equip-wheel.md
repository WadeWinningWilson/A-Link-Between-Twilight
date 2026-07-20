# Quick-equip wheel + page 2

**Status:** Phase A playable but **two bugs/specs open** (2026-07-20 research)  
**Next:** (1) draw ring without pause flag (2) damageable Link → interrupt close+Z  
**Do not:** use B / `SELECT_ITEM_B`; expand `mItems[24]` without a dedicated save plan

---

## Open fix plan (research 2026-07-20 — no code yet)

### A — No visuals (input + SFX work)

**Cause:** `dMw_c::_draw()` only draws the item ring when `dComIfGp_isPauseFlag() == true` (`windowStatus == 2` → `dComIfGd_set2DOpa(mpMenuRing)`). Live quick-wheel skips capture/`onPauseFlag`, so `_move` runs (sounds, selection) but **draw is gated out**.

**Fix:** In `_draw`, also submit `mpMenuRing` when `dMenu_Ring_c::isQuickEquipLiveWorld()` && `windowStatus == 2`, **without** requiring pause. Keep tap/full wheel on the pause path.

**Optional:** light dim 2D overlay (no freeze capture) — product call; not required for icons.

### B — Damage while in quick menu

**Product:** Link can be hit. On hit → close quick wheel + assign **hovered** item to Z (same as release confirm).

**Blocker today:** `fopAc_Execute` **returns early for ALINK** when live quick is on → full soft-lock. No execute ⇒ no Cc/TgHit ⇒ **no damage**.

**Fix direction:**
1. **Remove ALINK execute skip.** Keep stick for the ring by suppressing Link’s move/item/attack while live (control lock, not execute skip).
2. **Interrupt hook** on real damage intake — prefer `daAlink_c::setDamagePoint` (when `i_dmgAmount > 0` after gates) or entry to `procDamageInit` / large-damage path. Avoid every `checkDamageAction` tick (too broad).
3. **Interrupt API:** e.g. `dMenu_Ring_forceQuickConfirmClose()` — assign current hover → Z, set move-end/close (same path as release). Call from damage hook only if `isQuickEquipLiveWorld()`.
4. After close, damage proc continues normally (flinch/knockback).

**Order of ops on hit:** confirm Z from hover → begin close → allow damage reaction (same frame or next).

---

## Phase A (shipped in tree)

| Piece | Behavior |
|-------|----------|
| Setting | **Quick Equip Wheel** under Extra Item Slot (`game.quickEquipWheel`, default **off**) |
| Gate | Extra Item Slot must be on (Extra Only or Extra + Quick Swap) |
| **Tap** `OPEN_ITEM_WHEEL` | Full wheel (unchanged X/Y/Z assign) — **paused** capture like vanilla |
| **Hold** ~250ms (8 sim frames) | Filtered **tools** ring, **live world** (no pause flag / no freeze capture) |
| World pace | **0.3×** sim scale via `dusk::setSimTimeScale` (70% slowdown; same hook as Flurry) |
| Link | Soft-locked (execute skipped) while live quick-wheel open |
| **Release** (hold wheel) | Assign hovered tool to **Z only** — X/Y untouched; close |
| B / Up / Down while held | Cancel, no Z change |
| Cycle speed | Slightly faster wait / cursor accel in quick mode |

**Bug fixed:** press-edge used `getActionBindHold` (false on first frame) → treated as tap-release → always full vanilla wheel. Now uses `getActionBindDown`.

**Known:** live path has no ring draw (pause gate); Link soft-lock blocks damage — see Open fix plan above.

**Touchpoints:** `d_menu_window.cpp`, `d_menu_ring.cpp`, `settings.h` / `settings.cpp` / `ui/settings.cpp`, `action_bindings` (`isQuickEquipWheelEnabled`).

### Tool allowlist (page 1)

Boomerang, spinner, iron ball, bow (+ arrow/hawk/bomb variants), hookshot / clawshot, boots, Dom Rod, lantern, fishing rods, slingshot, hawkeye, bomb bags / bomb types.

Bottles, letters, dungeon items, quest paper — **excluded** (tap wheel).

---

## Page 2 (not coded)

| | |
|--|--|
| **Likely?** | Yes — medium-low after Phase A (swap slot list mid-hold) |
| **More slots?** | **Save inventory stays 24** (`mItems[24]`). Page 2 shows another ≤24 **entries** (swords/equip bits / WW stubs), not more save slots. Past 24/page → grow ring arrays or add page 3. |

Release dispatch (planned): tools/WW → Z; swords → sword equip; shields optional.

---

## Product shape (locked)

| Input | Mode | On release |
|-------|------|------------|
| Tap | Full wheel | X/Y/Z assign |
| Hold page 1 | Tools | → **Z-slot** |
| Hold page 2 | Swords / WW | Category equip |

---

## Playtest

1. Extra Item Slot → Extra + Quick Swap; enable **Quick Equip Wheel**.
2. Tap L1 (or wheel bind) → full wheel; assign X/Y as usual.
3. Hold L1 → tools-only ring; release on spinner → Z shows spinner; X/Y unchanged.
4. Toggle Quick Equip Wheel **off** → L1 opens full wheel on press again.
5. Extra Off → setting disabled / feature inert.

---

## Related

| Doc | Role |
|-----|------|
| [d-pad-reworking.md](../d-pad-reworking.md) | Extra Item Slot + Z + L1 |
| [deku-leaf-glide-research.md](../deku-leaf-glide-research.md) | P4 → future Z / page-2 resident |
| [WW-Restoration-Cookbook.md](../WW%20Linked/WW-Restoration-Cookbook.md) | TP owns inventory |
