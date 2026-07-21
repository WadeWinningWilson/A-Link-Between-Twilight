# Grass port — decomp analysis (for the content/engine lane)

**Status:** analysis only. **No implementation attempted** — History is actively working this system
and a competing port would recreate the two-owners problem behind the №65/69/73/83/88/89 bug family.

**Source:** `D:\XXXXXXX\WW DP\src\d\d_grass.cpp` (472 lines), USA `GZLE01`. Structure described and
cited; nothing reproduced.

---

## The headline

**Grass visibility and "skybox lighting matching vanilla" are the SAME problem, not two.**
WW's grass takes its colour from the per-room environment state the sky system also drives. Fix the
feed and both move together; fix them separately and they will keep disagreeing.

---

## 1. Grass is NOT a J3D model

`dGrass_packet_c::draw()` (`:269`) bypasses the J3D material pipeline entirely and issues raw GX:

- `j3dSys.reinitGX()` — **explicitly discards J3D's cached GX state** before drawing, precisely
  because what follows is not a J3D draw.
- `GXSetNumIndStages(0)`
- Declares its own vertex descriptors — `GX_VA_POS`, `GX_VA_CLR0`, `GX_VA_TEX0`, all **`GX_INDEX8`**
- Declares its own attribute formats (`GX_POS_XYZ/F32`, `GX_CLR_RGBA/RGBA8`, `GX_TEX_ST/F32`)
- `GFSetArray(...)` × 3 → position, colour and texcoord arrays (`mpPosArr`, `mpColorArr`,
  `mpTexCoordArr`)
- `GXCallDisplayList(mpMatDL, mMatDLSize)` — a **material display list**, not a material struct
- then iterates rooms (`:296`)

**Implication for the port.** The indexed-array setup is the fragile part: `GX_INDEX8` attributes are
meaningless unless the three `GFSetArray` calls point at live arrays. If the port renders the blades
as an ordinary J3D model — or issues the display list without first establishing descriptors, formats
and arrays — the result is nothing drawn, or garbage geometry. **This is the most likely cause of
"grass invisible."**

The blobs the port already extracts (`d_grass__l_Oba_kusa_aDL.bin`, `..._a_cutDL.bin`,
`..._aTEX.bin`) are the right assets — display lists plus texture. **The gap is draw state, not
assets.**

## 2. Colour comes from `dKy_tevstr_c`, per room

Inside the room loop:

- `dKy_tevstr_c* tevstr = dComIfGp_roomControl_getTevStr(i)` (`:298`)
- `GFSetTevColorS10(GX_TEVREG0, tevstr->mColorC0)` (`:299`)
- `GFSetTevColor(GX_TEVREG1, tevstr->mColorK0)` (`:300`)
- `dKy_GfFog_tevstr_set(tevstr)` (`:301`)

**Grass colour is injected into TEV registers from the environment state — every frame, per room.**
It does not come from material colours and it does not come from a light channel. The same
`dKy_tevstr_c` drives the environment/sky colouring, which is why the two symptoms move together.

The same fields appear in the gameplay paths too — `mColorK0` is passed to the cut/step particles at
`:80`, `:153` and to `setBatta` (`:49`) — so the room tevstr is the single colour authority for the
whole grass system.

## 3. Why the adapt tool's TEV fix is *wrong specifically here*

Cookbook §1 records: *"normalize_tevregs: TEV C-reg gray placeholder (128,128,128) → white — WW's
runtime overwrote it per-frame; unpatched = permanent half brightness."*

**`dGrass_packet_c::draw()` is that per-frame overwrite.** The static white-ing is a correct
substitute for props that never had a dynamic writer. **Grass did.** Whitening its constant register
and then not feeding live tevstr values leaves grass lit by a placeholder — plausibly washed out,
plausibly invisible depending on the TEV stages in the material DL.

**So grass is the one known case where the standard adaptation is actively counterproductive.**

## 4. What this suggests to check first (not instructions — diagnostics)

1. Are descriptors, attribute formats and all three `GFSetArray` calls issued **before** the material
   display list? Missing arrays ⇒ nothing renders.
2. Is `reinitGX()` (or the port's equivalent) called first? Without it, leftover J3D state can
   silently invalidate the raw GX setup.
3. Is a **per-room** tevstr fetched and pushed into `TEVREG0` / `TEVREG1` each frame, or is grass
   inheriting whatever the last draw left behind?
4. Does TP's kankyo produce `mColorC0` / `mColorK0` equivalents for a **mounted BG room**? Outset is
   an actor inside a host stage, so `dComIfGp_roomControl_getTevStr(roomNo)` has no direct analogue —
   **the room index a mounted BG should present is an open question**, and probably the real
   integration problem.

**Item 4 is the one worth resolving first.** Items 1–3 are mechanical; item 4 is architectural, and
it is where the room-as-actor design and WW's per-room colour model actually collide.

## 5. Scope note

Separate from `d_wood` / `d_tree` (`d_tree.cpp`, 23.5 KB) which back `swood*` — the content lane's P1
item. Grass (`kusax*`) and wood are different systems; closing one does not close the other.

**Grass remains deliberately disabled** (№128). Nothing here proposes re-enabling it — this analysis
exists to make the eventual re-enable land correctly.
