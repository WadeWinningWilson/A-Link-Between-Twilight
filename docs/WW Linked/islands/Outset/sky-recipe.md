# Sky & Environment Recipe — donor islands

> ## READ FIRST — check the DONOR DECOMP, not just the receiver
>
> **Decomp source: `D:\XXXXXXX\WW DP\src`**  (arcs: `D:\XXXXXXX\Ex WW`)
>
> The receiver's source tells you **what** an API is. Only the donor tells you **how its
> own actors called it** — flags, argument order, order of operations. Those are not
> derivable from the receiver side, and guessing them looks exactly like a working port
> until it silently isn't.
>
> **This cost four debug rounds once already (ledger №177–№181).** The cast was invisible
> in the opening cutscene. Every receiver-side probe read green — actor bound, enables set,
> position correct, draw running, matrix written. The fault was that
> `dDemo_setDemoData(...)` was called with flags reasoned out as `0xEE` instead of the
> donor's actual **`106`**. The extra `ENABLE_SCALE_e` bit assigned
> `scale = demo_actor->getScale()` = **(0,0,0)** — the model was scaled out of existence
> while every position measurement stayed correct. The answer was sitting in
> `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ls1.cpp` the entire time.
>
> **Before instrumenting a receiver-side theory:**
>
> 1. Find the donor's equivalent actor/system (`d_a_npc_*.cpp` for NPCs,
>    `d_demo.cpp` / `d_event*.cpp` for cutscenes).
> 2. Copy its call shape **verbatim** — flags, argument order, sequencing.
> 3. Cite the donor function in a comment at the ported call site.
>
> Outset is only partially decompiled, so expect gaps — but struct definitions and call
> sites are reliable, and are the fastest route from "it doesn't work" to "here is the
> parameter I got wrong".



> ## ⚠ READ FIRST — `OffsetPos` applies to EVERY Great Sea space
>
> A donor event's `PACKAGE: PLAY` cut carries an **`OffsetPos`** that is handed
> straight to `dDemo_c::start(demo_data, xyzdata, offsetAngY)`
> ([d_event_data.cpp:1291](../../../src/d/d_event_data.cpp)). It is the origin the
> storyboard stages its **cast** from.
>
> **It comes across from the donor VERBATIM when an event is merged, and donor
> world coordinates are not receiver world coordinates.** Outset's opening carries
> `OffsetPos = -220000, 0, 320000` while the island itself sits near `-195000` — a
> **~24,600-unit gap in X**. A cast staged off the wrong origin lands in open ocean,
> and on screen that is indistinguishable from *"the actor never appears"*.
>
> **This affects every island, every interior and every NPC that is ever added**, not
> just the space it was first found on. Whenever you merge a donor event:
>
> 1. Decode the PLAY cut and read its `OffsetPos`.
> 2. Compare it against the receiver-space position of the scene (the camera
>    `FIXEDFRM`/`STBWAIT` Center is a good reference — it is usually already correct).
> 3. If they disagree, the cast will stage away from the camera. **Fix the field in
>    the merged `event_list.dat` — it is DATA, no rebuild required.**
>
> A camera that frames the right spot proves nothing about where the cast is: the two
> come from different fields. Verified-wrong beats assumed-right.
> Ledger: №165, №175.



Written after getting Outset's sky from "blown out" to correct. It is written
for **any** donor island, not just Outset: nothing below is Outset-specific
except the worked example values. Reuse it verbatim for the rest of the sea.

Companion: [WW-Restoration-Cookbook.md](../../../WW-Restoration-Cookbook.md) §4
(the conversion) and §7 (how to cost a port). This doc is the *sky* slice, with
the failure modes attached to their symptoms.

---

## 0. What a working sky actually requires

Five separate things must all be right. Each one failed independently during
Outset, and each produced a **different** wrong picture, which is what makes
this worth writing down:

| # | requirement | symptom when wrong |
|---|---|---|
| 1 | converted `Env0/Col0/PAL0/VRB0` tables in the stage | blown-out / wrong ambient, wrong fog |
| 2 | correct `VRB0` **record stride** | first entry fine, later bands garbage |
| 3 | the dome models mounted and drawn | flat clear-colour sky, nothing but haze |
| 4 | `g_env_light.vrbox_*` populated from `VRB0` | magenta/black nonsense colours |
| 5 | each dome model actually *taking* its colour | one dome right, another still white |

A partial fix moves the picture without fixing it. Diagnose by which of the five
the current symptom belongs to, rather than by trying colours.

---

## 1. Convert the tables

Run `tools/ww_crew_restoration_skeleton/convert_lighting.py`. It reads the donor
room's chain and writes the receiver's four tables into the mod-side stage arc.

The chain, and it is the same in both engines:

```
EnvR/Env0 [room]  ->  pselect id
Colo/Col0 [pselect].palette_id[band]  ->  palette id
Pale/PAL0 [palette].vrboxcol_id       ->  vrbox colour id
Virt/VRB0 [vrboxcol]                  ->  sky/kumo/kasumi colours
```

**Record sizes differ and this matters:**

| donor | receiver | note |
|---|---|---|
| `EnvR` 0x08 | `Env0` 0x41 | per-room → palette sets |
| `Colo` 0x0C | `Col0` 0x0C | identical layout, straight copy |
| `Pale` 0x2C | `PAL0` 0x34 | +6 trailing fields with no donor source |
| `Virt` 0x24 | `VRB0` **0x15** | reordered; **21 bytes, do NOT pad to 0x18** |

### 1a. `PAL0`'s six trailing fields are not optional

They have no donor source, and **zero is not neutral**. Every native outdoor
stage sets them. Sampled from `F_SP00/102/103/108/121`:

```
bg_light_influence   = 100
cloud_shadow_density = 45
unk_0x2f             = 200   (0 on indoor stages)
bloom_tbl_id         = 4 + band      <-- cycles 4..9 per 6-band group
BG1/2/3_amb_alpha    = 255,255,255
```

A zero `bloom_tbl_id` selects a *different bloom table* — that alone reads as
"blown out" even when every colour is correct.

### 1b. The `VRB0` stride trap

`stage_vrboxcol_info_class` declares `Size: 0x18` in the header, but its fields
end at `0x15` and the PC build's `sizeof` is **21**. Emitting 24-byte records
puts entry 0 in the right place and skews every later entry by 3 bytes more
than the last — so the *day* band, which is index 2, reads garbage while the
first band looks fine.

**Verify, do not assume.** Read the shipped file back at the engine's stride:

```python
for i in range(n):
    r = dzs[vrb0_off + i*0x15 : vrb0_off + i*0x15 + 21]
    sky = tuple(r[0:3]); kumo_top = tuple(r[3:6]); kasumi_in = tuple(r[17:20])
```

Every entry must decode to plausible colours matching the donor's `Virt`. Do
not measure stride from the chunk span — chunks are padded, so `span/n` lies.

### 1c. Which band is "day"

Both engines' light schedules make **slot 2** the long daytime band
(receiver `{135-240 -> 2,2}`, donor `{150-270 -> 2,2}`), and time runs 0..360
for 24 h, so **hour 15 = daytime 225 = band 2**. When checking a screenshot
against the table, that is the row to compare.

---

## 2. Mount the dome

The donor sky is four separate models, drawn in this order:

```
vr_sky          <- vrbox_sky_col
vr_uso_umi      <- vrbox_sky_col          (horizon sea band)
vr_kasumi_mae   <- vrbox_kasumi_inner_col (near haze)
vr_back_cloud   <- vrbox_kumo_top_col     (clouds, drawn +100 Y)
```

The receiver's own vrbox is suppressed on these stages (`hide_vrbox = true` for
the island/forest mounts), so **if the donor dome is not mounted you get the
clear colour and nothing else** — a flat white sky with a faint haze band.

---

## 3. Two adapter traps specific to sky models

**3a. Do not let `normalize_tevregs` touch `vr_*`.** The adapter promotes a
50 %-grey TEV placeholder to white, which is right for characters and terrain
because their runtime overwrites those registers per frame. For a sky dome the
TEV register *is* the colour, so whitening paints the dome white. `adapt_arc`
passes `skip_tevregs=True` for `vr_*` — **any dome arc adapted before that guard
landed still carries whitened registers, so re-adapt it if in doubt.**

**3b. Baking colours into the file does not work, and the reason is useful.**
Writing the colour into the material's TEV register in the arc leaves it inert:
the draw path never pushes material state unless something calls
`material->change()`. The native `daVrbox_color_set` does
`setCullMode(0)` + `change()` + `setTevColor(...)` **every frame**. Sky colour
is a per-frame runtime write, full stop.

---

## 4. Drive the colours per frame

Mirror `daVrbox_color_set` (`d_a_vrbox.cpp`): for each dome model call
`model->calc()`, then on its material `setCullMode(0)`, `change()`,
`setTevColor(0, &c)`. Feed each model from the `g_env_light.vrbox_*` field
listed in §2.

Because the converted `VRB0` carries all bands (3 weather sets × 6 time bands),
driving this per frame gives **time-of-day skies for free** — no extra work.

---

## 5. Symptom → cause (the fast path)

| what you see | where it is |
|---|---|
| flat white sky, faint haze only | dome not mounted, or `hide_vrbox` with no replacement (§2) |
| whole sky washed out / blown | `PAL0` trailing fields zeroed, esp. `bloom_tbl_id` (§1a) |
| first band fine, others nonsense | `VRB0` stride padded to 0x18 (§1b) |
| magenta / black / impossible colours | `g_env_light.vrbox_*` not populated from `VRB0` (§4) |
| **one dome right, another still white** | per-model material issue on the wrong dome (§6) |
| colours right but never change with time | driving from a baked value, not per frame (§3b) |

---

## 6. When one dome is right and another is not

This is the subtle one and it is worth recognising quickly, because it looks
identical to "the sky is broken" while actually meaning **almost everything is
working**.

Outset's case: `VRB0[2].kumo_top = (255,255,255)` and clouds rendered white —
correct. `VRB0[2].sky_col = (80,120,255)` and the dome rendered white — wrong.
Same table, same frame, same apply function. That combination proves the data
*and* the feed are fine and isolates the fault to one model.

Check, cheapest first:

1. **Material count** — `getModelData()->getMaterialNum()` per dome. If a dome
   has more than one material, or its visible surface is not material 0, the
   colour is being written to a material nobody sees.
2. **Register select** — confirm the material's TEV stage actually consumes the
   register being written, rather than assuming register 0.
3. **Texture dominance** — if the stage is `TEXC`-only with no `C0`/`RASC`
   term, no register can affect it and the dome is always its texture's colour
   (see §3a).

---

## 7. Checklist for the next island

1. Convert tables; confirm `VRB0` at **0x15** by reading the shipped file back.
2. Confirm `PAL0` trailing fields are the native-sampled values, not zero.
3. Mount the four dome models; confirm the receiver vrbox is suppressed.
4. Confirm the dome arc was adapted with `skip_tevregs` for `vr_*`.
5. Drive `vrbox_*` per frame with `change()` + `setTevColor`.
6. Compare a **daytime** screenshot against `VRB0[band 2]`, not against memory.

If all six hold, the sky is correct and will follow time of day.

---

## 8. Wind (Ferry F / №280) — there is NO donor-authored per-stage wind direction

WW hosts render on a foreign shell stage, so the room `FILI` wind reads **0** and the
wave/grass panes don't drift. Engine's `dKyw_ww_host_wind_onStage` forces an ambient wind
(`pow≈0.4`) for WW hosts. The **direction** is History's value — and the decomp answer is
that **the donor authors none**, so the placeholder is already correct.

**Decomp (`dKyw_wind_set`, `D:\XXXXXXX\WW DP\src\d\d_kankyo_wether.cpp:986`):**
- `FILI` supplies the wind **LEVEL only** (`GlobalWindLevel`: 0→strength 0.3, 1→0.6, 2→0.9).
  It does **not** carry a direction.
- Direction = `tact_wind` (the **Wind Waker baton** — player-controlled) **or** `evt_wind`
  (kytag / tornado events). With neither set:
  `tact_y=0 → wind_vec = (cos·cos, sin, cos·sin) = (1,0,0)` = **+X = `angY=0`**.
- `wind_vec.x = cos(tact_x)·cos(tact_y)`, `.y = sin(tact_x)`, `.z = cos(tact_x)·sin(tact_y)`
  — so `angY` (= `tact_y`) rotates the XZ direction: `0→+X`, `0x4000→+Z`, `0x8000→−X`.
- Outset's only env tag, **`ky_tag1`** (`sea/Room44`, at `-200000,-5000,321000`), has params
  `0xffffffff` (all-default) and angle `(0,0,0)` — it authors **no** wind direction.

**Why `angY=0` is the correct Outset value, not a placeholder to replace:** at the Outset
intro Link **has no Wind Waker yet** (it comes from the King of Red Lions much later), so
`tact_wind` is unset and the wind *is* the ambient default `angY=0` (+X). Nothing to replace.

**Notes:** the donor default *strength* for level-0 is **0.3** (Engine set `0.4` — close;
Engine's call). And `angY` being decomp-`0`, the gate step "panes drift vs noclip Room44" is
the visual arbiter — if `+X` ever disagrees with the museum, that is a deliberate **aesthetic
tune**, not an authored number (none exists in the data). Do not invent one.
