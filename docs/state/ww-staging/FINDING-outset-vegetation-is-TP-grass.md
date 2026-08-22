# FINDING — the black vegetation is TP's own grass actor on a WW stage; WW's `kusa`/`pflower` are NOT PORTED

era: era-2 (Outset served)
<!-- era rationale: concerns the currently served set (sea room 44) | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Boot:** `dusklight-20260822-142942.log`

## The short version

The `WW GRASS AND FLOWERS STILL BLACK` item was being chased as a **WW lighting
defect**. It is not one. The WW colour path works. **The WW grass and flower
actors do not exist in this port at all**, and what is on Outset is **TP's own
`GRASS_e` actor**, which has no WW colour source and therefore draws unlit.

This makes it a **DN-11 cross-pollination** item (WW spaces get only WW assets)
and a **porting gap** — not a colour bug.

## What was measured

### 1. The WW colour path WORKS — every hop verified

| receipt | value | meaning |
|---|---|---|
| `pale_bind` | `n=57 virt=37 envr=52 colo=10 start="sea"` | binding works; matches the offline oracle exactly |
| `bg_overlay` | bg 0/1/3 written, e.g. `tev_c0=[138,124,137]` | real colour reaching BG materials |
| `bg_model_type` | room 44's models at types 32/33/35 → bg 0/1/3 | correctly classified |
| `actor_leg` | `amb=[116,108,124]` = `Pale[4].actor_c0` | correct to the digit |

Room 44's three models carry **8/8/1 materials** (parsed offline from
`Room44.arc`), matching the type-32/33/35 group exactly. The island is lit
correctly, which is what the user reports.

### 2. Type 10 is Link, not vegetation — FALSIFIED my own hypothesis

I built toward "the room models are mistyped" and the data refuted it. The
`type:10` group has 18/11/2 materials, matching nothing in room 44. Verified
against the **receiver**: `d_a_alink.cpp:19468` and `:19470` call
`settingTevStruct(9, …)` / `(10, …)`. Types 9/10 are **Link**, so
`applyActorOverlay` zeroing their `TevColor` is deliberate and correct.

Only five light types ever reached `dKyWw_tryWwMaji`: 0, 10, 32, 33, 35. None
of them is vegetation. Types 12/13/14 are skipped by design and are UI/demo
paths in the receiver (`d_a_alink_swindow`, `d_a_demo_item`, `d_file_select`,
`d_menu_collect`) — not vegetation either.

### 3. The WW vegetation actors are absent — measured, not inferred

Room 44's `room.dzr` places **164 vegetation objects**:

| actor | count | |
|---|---:|---|
| `kusax1` | 46 | grass (草) |
| `kusax21` | 36 | grass |
| `kusax7` | 30 | grass |
| `pflower` | 41 | flowers |
| `flwr17`, `flwr7`, `pflwrx7` | 11 | flowers |

**In the entire boot log: `kusax` appears 0 times, `pflower` 0 times, `flwr` 0
times.** No spawn, no failure, no receipt. The plugin's registry contains no
reference to any of them, and `src/ww/d/actor/` holds only `d_a_bg`,
`d_a_sea`, `d_a_vrbox`. **These actors are not ported.**

### 4. What IS on Outset is TP's grass actor

`fpcNm_GRASS_e` — the RECEIVER's grass — is created **106 times**, and the
split is the point:

- **6** before the `sea` bind (line 2360) — the pre-warp TP room
- **100** AFTER it, spanning room 44's load (line 2889)

100 TP grass actors against 112 WW `kusa*` placements. The `kusa` hits in the
log are all TP's own (`fpcNm_Obj_Yobikusa_e`, `a_kusa_rgba.bti`,
`j_umakusa.bmd`) — receiver resources, not donor ones.

## The mechanism this explains

TP's `GRASS_e` draws through its **own `J3DPacket` + `entryImm`** (the same
property that makes it immune to the lwood packet-merge hazard), so it never
passes through `dKyWw_tryWwMaji` and never receives a WW overlay. Its colour
comes from the receiver's `g_env_light`, which has nothing for a WW stage.
**Unlit → black.** It accounts for grass *and* flowers together, which a
single mistyped material would not.

## What is inference, and who settles it

**MEASURED:** everything in §1–§4 above.

**INFERRED, and the user is the only one who can confirm it:** that the black
objects the user sees ARE these 100 TP `GRASS_e` actors. The counts, the
timing, the absence of any WW vegetation, and the unlit-by-construction
mechanism all agree — but "the log says TP grass is there" and "the black
things on screen are that grass" are different claims.

## What the fix is NOT

**Not a colour patch on TP's grass.** DN-11 forbids TP assets in WW spaces
outright, and DN-10 orders the donor's own system ported rather than the
receiver's patched. Lighting TP grass correctly on Outset would make a DN-11
violation look finished.

**The fix is to port the donor's `kusa` / `pflower` actors** — which is a
porting-queue item (History), not a kankyo item. Until then the honest state
is: WW vegetation is unported, and TP vegetation is standing in.

## Reproduce

```bash
python tools/foundry/ww_dzs_kankyo.py "D:/XXXXXXX/Ex WW/files/res/Stage/sea/Room44.arc"
```
(placement census is in the same tool's chunk walk; the ACTR/SCOB name dump
is the `ACT*`/`SCO*` chunks at 0x20 bytes per entry, name at offset 0)
