# Quick-equip wheel + page 2

**Status:** Phase A + draw/damage fix shipped (2026-07-20)  
**Next:** Page 2 (swords / WW overflow) when product asks  
**Do not:** use B / `SELECT_ITEM_B`; expand `mItems[24]` without a dedicated save plan

---

## Phase A + live-world fixes (in tree)

| Piece | Behavior |
|-------|----------|
| Setting | **Quick Equip Wheel** under Extra Item Slot (`game.quickEquipWheel`, default **off**) |
| Gate | Extra Item Slot must be on (Extra Only or Extra + Quick Swap) |
| **Tap** `OPEN_ITEM_WHEEL` | Full wheel (unchanged X/Y/Z assign) — **paused** capture like vanilla |
| **Hold** ~250ms (8 sim frames) | Filtered **tools** ring, **live world** (no pause flag / no freeze capture) |
| World pace | **0.3×** sim scale via `dusk::setSimTimeScale` (70% slowdown; same hook as Flurry) |
| Link | Control-locked (stick zeroed / `checkNextAction` early); **Cc/damage still run** |
| Draw | Live path submits ring in `dMw_c::_draw` even without pause |
| **Release** (hold wheel) | Assign hovered tool to **Z only** — X/Y untouched; close |
| **Damage** | `setDamagePoint` → `forceQuickConfirmClose()` (hover→Z + close); flinch continues |
| B / Up / Down while held | Cancel, no Z change |
| Cycle speed | Slightly faster wait / cursor accel in quick mode |

**Bug fixed earlier:** press-edge used `getActionBindHold` (false on first frame) → always full vanilla wheel. Now uses `getActionBindDown`.

**Touchpoints:** `d_menu_window.cpp`, `d_menu_ring.cpp` / `.h`, `d_a_alink` stick + `checkNextAction`, `d_a_alink_damage.inc`, `f_op_actor.cpp` (no ALINK execute skip), settings / action_bindings.

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
3. Hold L1 → tools-only ring **visible**; release on spinner → Z shows spinner; X/Y unchanged.
4. Hold wheel near enemies → take a hit → wheel closes with hovered tool on Z; Link flinches.
5. Toggle Quick Equip Wheel **off** → L1 opens full wheel on press again.
6. Extra Off → setting disabled / feature inert.

---

## Related

| Doc | Role |
|-----|------|
| [d-pad-reworking.md](../d-pad-reworking.md) | Extra Item Slot + Z + L1 |
| [deku-leaf-glide-research.md](../deku-leaf-glide-research.md) | P4 → future Z / page-2 resident |
| [WW-Restoration-Cookbook.md](../WW%20Linked/WW-Restoration-Cookbook.md) | TP owns inventory |
| [fado-door-warp.md](fado-door-warp.md) | Parallel Ordon door unlock / warp sink |
