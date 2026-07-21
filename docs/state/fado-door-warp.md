# Fado’s Ordon door — unlock + warp command

**Status:** Phase A + B in tree (2026-07-20)  
**Next:** real callers (ticket / WW quest / return path)  
**Do not:** invent a retail room 3 arc as “Fado’s house” unless product wants that; do not put WW strings in the exe (№101 / R6)

**Related:** cut-actors №101 “Fado ticket”; Ordon interiors `R_SP01`; mod flags `d_ext_mod_flags`

---

## Where it is

| Piece | Value |
|-------|--------|
| Village stage | `F_SP103` room **0** |
| Door actor | `kdoor` → `d_a_door_knob00` / `daKnob20_c` (`fpcNm_KNOB20_e`) |
| Params / exit | `0x8e000107` → **exit ID 7** |
| msgNo (lock) | `home.angle.x` = **0x0191** (locked talk); other houses also have `0xffff` unlock layers — **exit 7 has locked only** |
| Position | `(-1605.02, 394.65, 550.02)`, ay `21845` |
| SCLS[7] today | `R_SP01`, spawn `0`, room **3** |
| Interior | **`R_SP01` room 3 never shipped** (also room 6 missing). Fado NPC (`d_a_npc_aru`) lives at ranch `F_SP00` / village day-1 — not in a cut interior |

Native open path: mid-anim → `onSceneChangeArea(exit7)` → `dStage_changeScene` → SCLS[7] → `dComIfGp_setNextStage`. Unlock for other Ordon houses = layer swap to `msg=0xffff`, not a key.

---

## Product shape (locked intent)

1. **Unlock** — door becomes openable when mod unlock flag is set.
2. **Warp command sink** — destination from runtime command (stage/room/spawn/layer), not hard-coded `R_SP01` r3.
3. Unlocked + **no command** → keep lock msg; `dStage_changeScene` also blocks SCLS[7].
4. Crossing stays a **native stage change** (`dComIfGp_setNextStage`).

---

## Shipped API (PC)

| API | Role |
|-----|------|
| `dFadoDoor_isUnlocked` / `setUnlocked` | Save-scoped via `dExtModFlags` — folder **`dusklight`**, key **`fado.door.unlock`** |
| `dFadoDoor_setWarpCommand` / `clear` / `hasWarpCommand` | Session sticky until clear |
| `dFadoDoor_isTargetKnob` | `F_SP103` + exit **7** |
| `dFadoDoor_tryInterceptChangeScene` | Hooked from `dStage_changeScene` (1=override, -1=block, 0=retail) |

**Header:** `include/d/d_ext_fado_door.h`  
**Impl:** folded into `src/d/d_ext_mod_flags.cpp` (no new cmake TU)  
**Door open:** `daKnob20_c::checkOpenDoor`  
**Playtest:** Warp menu → **Fado Door** section (toggle unlock / arm selection / clear command)

Command is **sticky** after transit (return path can re-point).

---

## Acceptance

1. Flag off → locked msg, no enter.  
2. Flag on, no command → no black-room load; lock msg + intercept refuse.  
3. Flag on + `setWarpCommand(...)` (or Warp UI **Arm**) → open anim → fade → that room.  
4. Second caller changes command → same door goes elsewhere.  
5. No WW name literals in door/unlock source.

---

## Phase C — Callers (future)

| Caller | Sets unlock | Sets warp command |
|--------|-------------|-------------------|
| ALBW shop “passage ticket” | yes | optional |
| WW quest spine / №101 hybrid | yes | Outset + cutscene spawn |
| Return from Outset | — | Ordon doorstep spawn on `F_SP103` |
| Debug / editor | yes (Warp UI) | any stage/room |

---

## Code anchors

| Item | Path |
|------|------|
| API | `include/d/d_ext_fado_door.h`, `src/d/d_ext_mod_flags.cpp` |
| Knob open / msg | `src/d/actor/d_a_door_knob00.cpp` |
| Scene change | `d_stage.cpp` → `dFadoDoor_tryInterceptChangeScene` |
| Warp UI playtest | `src/dusk/ui/warp.cpp` |
| Design context | `docs/state/cut-actors-demo-restore.md` №101 |
