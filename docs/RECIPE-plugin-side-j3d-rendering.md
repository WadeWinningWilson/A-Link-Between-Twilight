# RECIPE — drawing donor J3D geometry from a plugin, bypassing GX

era: era-independent
<!-- era rationale: API recipe against the mod SDK + JSystem; not tied to a WW milestone | Housing/Engine, 2026-08-22 -->

**What this is.** A working, measured recipe for getting a donor `J3DModel`'s
geometry onto the screen from inside a `.dusk` plugin **without going through
aurora's GX/FIFO path** — plus every trap that cost a build round getting there.
Written because the route now works and the knowledge is otherwise spread across
a night of receipts.

**What it is NOT.** A renderer. It draws untextured, unlit positions. See
[§7 Why textures are the wall](#7-why-textures-are-the-wall) before planning to
extend it.

Source of truth: `mods-src/ww_donor_disc/ww_gfx_spike.cpp` and
`mods-src/ww_donor_disc/ww_j3d_mesh.inc` (mirrored in the canonical plugin repo).

---

## 1. Acquiring GfxService

```cpp
#include <mods/svc/gfx.h>
#include <mods/svc/camera.h>
#include <webgpu/webgpu.h>

IMPORT_OPTIONAL_SERVICE_VERSION(GfxService, s_gfx, 0u);
IMPORT_OPTIONAL_SERVICE(CameraService, s_cam);
```

**TRAP 1 — never use plain `IMPORT_SERVICE` for this.** It expands with
`SERVICE_IMPORT_REQUIRED`, and a required import the host cannot satisfy makes
**the whole mod fail to load** (`sdk/include/mods/service.hpp:44`). One
unsatisfiable import takes the entire plugin offline.

**TRAP 2 — pin the minor version.** There are two SDK trees in this project with
different `gfx.h` (fork = `GFX_SERVICE_MINOR 0`, clean vanilla = `2`), and the
plugin compiles against whichever `DUSK_ROOT` points at. The unversioned macro
requests *the compiling header's* minor, so a vanilla-SDK build demands minor 2
and gets **null** from a minor-0 host — which reads as "the host has no
GfxService" when it plainly does. Pin to the lowest minor you actually use.

**TRAP 3 — `FEATURES game webgpu` in CMake.** `mods/svc/gfx.h` hard-`#error`s
without the webgpu feature. Adding the source file alone is not enough.

## 2. Registering a draw type and a stage hook

```cpp
GfxDeviceInfo info = GFX_DEVICE_INFO_INIT;
s_gfx->get_device_info(mod_ctx, &info);          // device, formats, sample_count, uses_reversed_z

GfxDrawTypeDesc dt = GFX_DRAW_TYPE_DESC_INIT;
dt.label = "my_draw";
dt.draw   = &my_draw_fn;                          // RENDER WORKER THREAD
s_gfx->register_draw_type(mod_ctx, &dt, &s_drawType);

GfxStageHookDesc sh = GFX_STAGE_HOOK_DESC_INIT;
sh.callback = &my_stage_fn;                       // GAME THREAD
s_gfx->register_stage_hook(mod_ctx, GFX_STAGE_SCENE_AFTER_TERRAIN, &sh, &s_stageHook);
```

`GfxDrawFn` is
`void(ModContext*, const GfxDrawContext*, const void* payload, size_t, void*)` —
the `GfxDrawContext*` is easy to omit and gives a silent signature mismatch.

**TRAP 4 — the stage hook is GLOBAL.** It fires on *every* stage. If your draw is
scoped to particular content, gate it or you will paint into unrelated scenes.
Evaluate the predicate on the **game thread** and carry the decision in the
payload; calling into game state from the render worker is a data race even when
the answer happens to be right.

**TRAP 5 — do not put diagnostic geometry on screen.** Receipts belong in the
log. A debug triangle in the user's viewport is never worth it.

## 3. Per-instance draw: uniforms, vertices, payload

Game thread, once per instance per frame:

```cpp
float mvp[16];
mul_and_store(cam.proj_from_world, world34, mvp);   // see TRAP 7

GfxRange uni{}, vb{};
s_gfx->push_uniform(mod_ctx, mvp,  sizeof(mvp), &uni);
s_gfx->push_verts  (mod_ctx, verts, bytes, sizeof(float) * 3, &vb);

unsigned char payload[32] = {};      // ONE documented layout, decoded in ONE place
// [0] scope flag, [4..7] uniform offset, [8..11] vertex offset,
// [12..15] vertex bytes, [16..19] vertex count
s_gfx->push_draw(mod_ctx, s_drawType, payload, sizeof(payload));
```

Render worker, in the draw callback:

```cpp
WGPUBindGroupEntry e = WGPU_BIND_GROUP_ENTRY_INIT;
e.binding = 0; e.buffer = ctx->uniform_buffer; e.offset = uniformOffset; e.size = 64;
WGPUBindGroupDescriptor bd = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
bd.layout = s_layout; bd.entryCount = 1; bd.entries = &e;
WGPUBindGroup bg = wgpuDeviceCreateBindGroup(ctx->device, &bd);

wgpuRenderPassEncoderSetPipeline(ctx->pass, s_pipeline);
wgpuRenderPassEncoderSetBindGroup(ctx->pass, 0, bg, 0, nullptr);
wgpuRenderPassEncoderSetVertexBuffer(ctx->pass, 0, ctx->vertex_buffer, vertOffset, vertBytes);
wgpuRenderPassEncoderDraw(ctx->pass, vertCount, 1, 0, 0);
wgpuBindGroupRelease(bg);
```

**TRAP 6 — payload field collisions.** The 128-byte cap is generous; do not
overlap fields between draw variants. An early version packed a vertex count over
the slot another leg used for the uniform offset, which would have drawn every
instance with matrix offset 0 — every object at the world origin, from a defect
no receipt would name.

**TRAP 7 — matrix conventions.** `CameraInfo.proj_from_world` is **column-major**,
WebGPU convention, Aurora reversed-Z already applied. J3D's `getBaseTRMtx()` is a
**3x4 row-major** matrix. Promote to 4x4, multiply, then store column-major for
WGSL.

**TRAP 8 — depth compare.** `info.uses_reversed_z` is 1 here, so world-space
geometry needs `WGPUCompareFunction_Greater`. Using `Less` silently rejects every
fragment and looks exactly like "the route does not work".

## 4. Pipeline creation

Use `info.color_format` / `info.depth_format` / `info.sample_count` from
`GfxDeviceInfo` (present at minor 0). `get_scene_target_layout` is the header's
recommended path **but is a minor-2 member** — reading it on a minor-0 host runs
off the end of the service struct.

Reference implementation for shader-module and pipeline setup:
`dusklight-main/mods/shadow_mod/src/mod.cpp:382`.

## 5. Getting triangles out of a J3D model

**J3D DOES NOT STORE TRIANGLES.** Geometry is a display list per
`J3DShapeDraw`, with vertices in separate arrays on `J3DVertexData`.

```cpp
J3DModelData* data = model->getModelData();
const J3DVertexData* vd = &data->getVertexData();          // RETURNS A REFERENCE
for (u16 s = 0; s < data->getShapeNum(); s++) {
    J3DShape* shp = data->getShapeNodePointer(s);
    const GXVtxDescList* desc = shp->getVtxDesc();          // vertex record layout
    for (u16 g = 0; g < shp->getMtxGroupNum(); g++) {
        J3DShapeDraw* sd = shp->getShapeDraw(g);
        const u8* dl = sd->getDisplayList();
        const u32 sz = sd->getDisplayListSize();
        // walk dl ...
    }
}
```

### 5a. THE BIG ONE — the display lists are NOT raw GX on this port

This PC port **re-encodes** them. lwood's lists begin with **`GX_AURORA` (0x50)**,
defined in `extern/aurora/include/dolphin/gx/GXCommandList.h:32`, carrying
subcommand **`GX_AURORA_DRAW_INDEXED` (0x0041)**. Layout, field for field from
aurora's own parser (`extern/aurora/lib/gx/dl.cpp`, the `GX_AURORA` case):

| offset | field |
|---|---|
| `[0]` | opcode, masked with `GX_OPCODE_MASK` (0xF8) |
| `[1..2]` | u16 **BE** subcommand, must be `0x0041` |
| `[3]` | draw command: low 3 bits `GXVtxFmt`, high bits **primitive type** |
| `[4..5]` | u16 **BE** vertex count |
| `[6..9]` | u32 **BE** index count |
| `[10..]` | indexCount × u16 indices, **NATIVE endian** |
| then | vtxCount × stride vertex records |

`cmdSize = 10 + indexCount*2 + vtxCount*stride`

**The primitive type still lives in the draw command** — a DRAW_INDEXED strip is
still a strip and must expand as one.

**TRAP 9 — mixed endianness inside one command.** Header fields are big-endian;
the indices are native (aurora's `DrawCmd::index()` memcpy's them with no swap,
`dl.cpp:105`). Getting that backwards is silent: valid-looking indices pointing
at the wrong vertices.

**TRAP 10 — non-primitive commands are not end-of-stream.** Skip them by their
fixed sizes: `0x00` NOP (1), `0x44` invalidate-vtx-cache (1), `0x08` load-CP (6),
`0x61` load-BP (5), `0x20/0x28/0x30/0x38` load-INDX (5 each), `0x10` load-XF
(`5 + 4*(BE16(at+1)+1)`). Treating the first of these as "stop cleanly" makes the
walk end on byte 0 and yields an empty mesh — which on screen is indistinguishable
from a model that has no geometry.

**Always record the opcode and offset that stopped a walk.** "Drew nothing" and
"stopped on opcode 0x50" are the same symptom with completely different fixes.

### 5b. Vertex record layout from the VCD

Each vertex carries one entry per enabled attribute in `GXAttr` order:
`GX_NONE` = 0 bytes, `GX_INDEX8` = 1, `GX_INDEX16` = 2, `GX_DIRECT` = 1 byte
**only for the `*MTXIDX` attributes** (`attr < GX_VA_POS`). Any other `GX_DIRECT`
is inline vertex data sized by a `GXVtxAttrFmtList` — assuming one byte
desynchronises every following vertex. Refuse rather than guess.

### 5c. Reading positions

```cpp
const GXVtxAttrFmtList* fmt = vd->getVtxAttrFmtList();   // AUTHORITATIVE
// -> f->cnt (GX_POS_XY vs XYZ), f->type (GXCompType), f->frac
u32 stride = vd->getVtxArrStride(GX_VA_POS);             // NOT comps*sizeof
u32 count  = vd->getVtxNum();                            // BOUNDS-CHECK AGAINST THIS
```

`getVtxPosType()` / `getVtxPosFrac()` are shortcuts that **do not expose the
component count at all**. Fixed-point types need `value / 2^frac`.

**TRAP 11 — bounds-check the position index.** It is read from bytes at a
computed offset inside the vertex record; if that offset is wrong the value is
arbitrary, and indexing the array with it is an **out-of-bounds read**, not
merely a bad diagnostic.

**TRAP 12 — vertices bound to a joint are in JOINT SPACE.** If any shape's VCD
carries `PNMTXIDX`, its vertices are relative to its joint, not the model root.
Geometry on the root joint looks correct untransformed, so a partial fix is very
convincing — on lwood the trunk rendered fine while the canopy collapsed to its
joint's local origin. Resolve per matrix group:
`shp->getShapeMtx(g)->getUseMtxIndex(0)` → a draw-matrix index.
**UNRESOLVED AT TIME OF WRITING:** `model->getAnmMtx(useMtxIndex)` returned
effectively identity at decode time (`joint_applied:2`, residual unchanged), so
the correct mapping from draw-matrix index to a usable joint matrix is still open.
See §6 for how to tell.

## 6. The oracles that made this debuggable

Do not eyeball geometry. Every one of these caught something eyeballing missed:

- **`J3DShape::getMin()` / `getMax()`** — the shape's own declared bounding box,
  straight from the BDL. Compare it against your decoded bbox. This is the single
  highest-value instrument here; it owes nothing to your decoder. It proved my
  decode faithful on X/Z and localised the canopy failure to one axis.
- **Score all six component permutations against that declared box** and take the
  best, rather than hand-picking an axis order. Report the residual — a residual
  that will not go to zero is telling you something is still missing. *Caveat:
  two axes with equal extents cannot be distinguished this way and the tie may
  yield a mirrored model.*
- **Offline cross-check.** Decompress the arc (Yaz0 → RARC) and parse the BDL's
  SHP1 with no game involved; if its bounds match the runtime's, you are decoding
  the bytes you think you are. Script: `scratchpad/lwood_bbox.py` pattern.
  **SHP1 shape entry is 0x28 bytes: radius at 0x0C, min at 0x10, max at 0x1C.**
- **Count what you skipped.** `shapes` that only counts successes hides dropped
  shapes; a silently skipped shape looks exactly like absent geometry.
- **Dump raw bytes when a theory needs them.** Nine floats off the position array
  settled an axis question that three rounds of inference had not.

## 7. Why textures are the wall

Everything above is **positions only**. Texturing donor J3D content through this
path requires: texcoords out of the vertex stream, decoders for GameCube texture
formats (CMPR, RGB5A3, I4, …) out of the arc's TEX1/BTI, sampler and bind-group
plumbing, and an approximation of the **fixed-function TEV** stage chain —
multi-stage combiners, texgen, colour registers, blend/compare state.

**That is reimplementing a slice of aurora's GX→WGSL translator inside a plugin,
and aurora already does all of it correctly.** Every step of this recipe was
rediscovering something the engine implements. Weigh that before extending it.

## 8. What this route DID establish

**The delivery path is sound and it is not the bottleneck.** Measured on stock
dusklight with stock aurora and **no patch applied**:

| run | result |
|---|---|
| reachability | 6,141 stage fires / 6,140 worker draws, all preconditions `MOD_OK` |
| first pixels | pipeline built, 6,999 draws, geometry visible |
| box at donor cull extents | 11,744 draws, **zero overflow** |
| real decoded mesh | 12,608 → 21,232 draws across revisions, **zero overflow** |

**The GX path dies at draw 44.** The same instances, same positions, same frame
cadence, delivered through GfxService, do not overflow. That is the finding this
work exists for, and it stands independently of whether the tree ever looked
right.
