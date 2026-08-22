# FINDING — the black vegetation is TP's own grass actor on a WW stage; WW's `kusa`/`pflower` are NOT PORTED

era: era-2 (Outset served)
<!-- era rationale: concerns the currently served set (sea room 44) | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Boot:** `dusklight-20260822-142942.log`

> ## ⚠️ CORRECTED 2026-08-22, SAME DAY — READ THIS FIRST
>
> The original text said the WW vegetation "does not spawn" and that what is on
> Outset is TP grass **standing in for** placements that never resolved. **That
> framing is WRONG and the correction matters.**
>
> **The donor's OWN name table maps the vegetation to a profile the receiver also
> has:** `d_stage.cpp:438-444` — `OBJNAME("kusax1", fpcNm_GRASS_e, ...)`,
> `"kusax7"`, `"kusax21"`, `"flwr17"`, `"pflower"` — **all → `fpcNm_GRASS_e`**.
> That name exists in BOTH games. So the WW placements DO resolve and DO spawn;
> the 100 `fpcNm_GRASS_e` creations after the sea bind **ARE the WW placements**.
> What runs is the **RECEIVER's implementation** of that profile.
>
> So the objects stand at WW's placement positions, drawn by TP's code with TP's
> models and TP's lighting. "0 log hits for `kusax`" meant only that the name is
> consumed by the lookup and never logged — NOT that nothing spawned. I read an
> absence of the string as an absence of the object.
>
> **The conclusion survives and the fix is unchanged** — WW's own vegetation is
> unported and the donor's system must be ported — but the mechanism is
> "receiver implementation behind a shared profile name", not "nothing spawned".

## How the donor actually draws it — settles the packet-hazard question

`d_a_grass.cpp` is a **SPAWNER, NOT A RENDERER**. `daGrass_Create` reads the
placement, pushes into the managers — `dComIfGp_getGrass()->newData()` (kind 0),
`getTree()->newData()` (kind 1), `getFlower()->newData()` (kinds 2/3, white and
pink) — and returns `cPhs_ERROR_e`, deleting itself immediately.

The DRAWING lives in `d_grass.cpp`, `d_flower.cpp`, `d_tree.cpp`. Both grass and
flower managers draw via **`j3dSys.getDrawBuffer(0)->entryImm(this, 0)`**
(`d_grass.cpp:414`, `d_flower.cpp:480`) — their own `J3DPacket`.

**THEREFORE: IMMUNE TO THE LWOOD PACKET-MERGE HAZARD.** They never reach
`entryMatSort`, so they cannot self-merge and need no one-entry-per-fill guard.
Porting them is a manager port (3 units + the spawner), not a guarded-actor port.

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
