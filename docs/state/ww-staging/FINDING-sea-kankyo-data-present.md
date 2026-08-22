# FINDING — stage `sea` DOES have kankyo data; the standing lead is falsified

era: era-2 (Outset served)
<!-- era rationale: concerns the currently served set (sea room 44) | Housing/Engine, 2026-08-22 -->

**Lane:** Housing/Engine · **Date:** 2026-08-22 · **Tool:**
`tools/foundry/ww_dzs_kankyo.py` (new, this finding)

## What was believed

The `WW GRASS AND FLOWERS STILL BLACK` item carried this as its live lead:

> the disputed lead is that stage `sea` has no receiver kankyo data so
> BG-consumed AmbCol reads unlit

`src/ww/d/d_kankyo.cpp` states the same thing inline, as the reasoning behind
the `WW_BG_AMBCOL` lever:

> their ambient stayed whatever `settingTevStruct` left, which for stage `sea`
> is nothing: the receiver's own kankyo has no data for a WW stage

## What the disc says

**FALSE for the donor data, which is what the plugin serves.** Read directly
from the user's own extracted disc, `files/res/Stage/sea/Stage.arc` →
`stage.dzs`:

```
chunks: STAG x1, RTBL x50, SCLS x212, EVNT x57, MULT x50,
        EnvR x52, Colo x10, Pale x57, Virt x37, RPAT x4, RPPN x40,
        ACTR x4, SCOB x50, RCAM x1, RARO x1
```

**57 Pale entries, and NOT ONE of them is all-zero** (checked, not eyeballed —
the tool flags an all-zero entry and the count of flagged entries is 0).

## The selection chain resolves, every hop in range

Simulated offline exactly as `d_kankyo.cpp:170 selectPale()` does it
(`EnvR[room] → Colo[weather] → palette_id[schedule slot] → Pale`):

| hop | value | in range? |
|---|---|---|
| `EnvR[44].pselect_id` | `[0, 1, 2, 0, 0, 0, 0, 0]` | yes (EnvR=52) |
| weather 0 → `Colo[0].palette_id` | `[0, 1, 2, 3, 4, 5, 0, 0]`, rate 0.200 | yes (Colo=10) |
| reachable Pale entries | 0, 1, 2, 3, 4, 5 | yes (Pale=57) |

Every reachable palette carries populated BG colour. Two samples:

```
Pale[2]: bg0_c0=(36,24,59)  bg0_k0=(255,255,245) bg1_c0=(255,255,255) bg1_k0=(9,99,224)
Pale[5]: bg0_c0=(49,79,113) bg0_k0=(74,140,169)  bg1_c0=(0,105,155)   bg1_k0=(7,59,100)
```

`Room44.arc` → `room.dzr` carries **no** Pale/EnvR/Colo/Virt of its own, which
is correct — those are stage-level. It carries `FILI x1`, and FILI is **not**
the palette selector: the donor reads a Toon switch from it
(`d_kankyo.cpp:92 toon_proc_check` → `dStage_FileList_dt_GetToonsw`), and the
plugin's `dKyWw_bindFili` takes only `(param >> 18) & 3` plus the sea level.

## What this does and does not establish

**ESTABLISHED (measured, offline, reproducible):** the donor's stage data for
`sea` is present and populated; the selection chain the plugin implements
resolves in-range at every hop for room 44; no reachable palette is black. So
**"there is no kankyo data for this stage" is not the explanation** for black
vegetation, and any fix premised on it is premised on something false.

**NOT ESTABLISHED — these are runtime facts and this tool cannot reach them:**

1. that `wwKankyo_bindDzs` is actually reached for `sea` at runtime. It is
   wired on the stage-load hook behind `stageBecomingWw()`
   (`src/plugin/registry.cpp:6914`), which is a runtime predicate;
2. that `dComIfGp_roomControl_getStayNo()` returns 44 when the black
   vegetation is on screen;
3. that the overlay's write reaches the daBg vegetation materials at all, or
   lands in a slot that draw actually reads.

**LIMIT OF THE METHOD, stated rather than left implicit:** the tool transcribes
its `WwPalet` / `WwEnvr` / `WwColo` layouts and chunk tags FROM THE PLUGIN's
own structs (`d_kankyo.cpp:48-104`), deliberately, so it reads the same bytes
the runtime reads. That makes it a good check of *the data* and a **poor**
check of *the layout* — if the plugin's struct is wrong, this tool is wrong in
the same direction. The `static_assert(sizeof(WwPalet) == 0x2C)` and the
plausible colour values are evidence the layout is right, not proof.

## What settles the rest — already in the staged build, no arming needed

`registry.cpp:6916` already emits, unconditionally, on every WW stage load:

```
[WwRegistry] {"ev":"pale_bind","n":..,"virt":..,"envr":..,"colo":..,"fili":..,"start":".."}
```

One boot forks it cleanly:

- **`n=57, envr=52, colo=10`** → binding works and the data is good (this
  finding), so the defect is DOWNSTREAM: the write, or what the daBg draw
  consumes. `bg_overlay` (also unconditional) then shows what was written.
- **receipt absent, or `n=0`** → binding never happened for `sea`, and that is
  the defect — not the data.

## Consequence for the `WW_BG_AMBCOL` lever

It stays OFF and its stated rationale is now doubly wrong: it was already
misconceived on the write side (`applyBgOverlay` writes TevColor/TevKColor and
deliberately never touches AmbCol, which is the ACTOR path), and its premise —
"the receiver's own kankyo has no data for a WW stage" — is falsified here for
the donor data the plugin serves. Not deleted; the reasoning stays visible.

## Reproduce

```bash
python tools/foundry/ww_dzs_kankyo.py "D:/XXXXXXX/Ex WW/files/res/Stage/sea/Stage.arc"
```
