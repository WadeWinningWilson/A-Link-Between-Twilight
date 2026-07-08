# BMD Reskin Tool — reusing original animations on a Blender-edited rigged model

**Tool:** [`tools/bmd_reskin/bmd_reskin.py`](../tools/bmd_reskin/bmd_reskin.py) (standalone Python 3 + numpy)
**First use:** Armogohma phase-3 reveal model (2026-07-08) — fixed the mangled legs.
**Applies to:** *any* rigged BMD you edit in Blender but still want to drive with the **original game's animations** (BCK/ANK1).

---

## 1. The problem this solves

To edit a rigged Nintendo model (move an eye, delete a face-plate, …) you must round-trip it
through Blender: `original.bmd → (SuperBMD) → DAE → Blender → DAE → (SuperBMD) → custom.bmd`.

That round-trip **silently re-derives every bone's local coordinate frame.** Blender bones are
forced to point down their own **+Y axis with a "roll"**, which is a different bookkeeping of the
same skeleton than the original BMD stored. The result:

- Bone **positions** survive → the model stands correctly **at rest**, so nothing looks wrong.
- Bone **frames** (orientation/roll) are re-derived on **every** joint — even ones you never touched.

The original game animations rotate each joint *relative to its rest frame* ("rotate joint N by 30°
about **its** axis"). Because Blender redefined every axis, the same keyframes now swing limbs the
wrong way. It is **invisible until the model animates**, and it hits long multi-joint chains (legs,
tails) hardest while simple parts (a torso) can coincidentally survive.

**You cannot fix this in Blender or SuperBMD.** Blender's `+Y`-along-bone convention is baked in, and
SuperBMD's `--transform_mode` (Basic/Xsi/Maya/Mask) does **not** change the JNT1 decomposition. The
only fix is a post-export binary pass — this tool.

### Why it's a "reskin" and not just a JNT1 patch (the key J3D fact)

From the J3D source (`libs/JSystem/.../J3DMtxBuffer.cpp::calcDrawMtx`):

- **Rigid** verts (`DRW1` weighted-flag = 0) → draw matrix is `viewMtx · jointWorld` with **no
  inverse-bind**. So rigid verts are stored **in joint-local space** — welded to the (wrong) frame.
- **Weighted** verts (EVP1 envelopes) → `jointWorld · inverseBind`, stored in **model space**.

So changing a joint's frame **requires re-expressing its rigid vertices** (and rebuilding EVP1
inverse-binds), not just editing JNT1. That's what makes this a reskin.

---

## 2. What the tool does

For every joint **except** the root and any `--keep` joints (moved/added joints — see below):

1. Set the frame to the **vanilla** joint's frame (kept in the source model's root space, e.g. the
   `+90°X` that SuperBMD's `--rotate` parks on the root — so the model still displays upright).
2. Re-express each **rigid** vertex and its normal by `S_J = targetWorld_J⁻¹ · sourceWorld_J`
   (`new = S_J · old`) — a pure rotation, because the positions already coincide.
3. Carry each **EVP1** weighted inverse-bind to the new frame with the **same** `S_J`:
   `newInvBind_J = S_J · oldInvBind_J`. **Do NOT** set it to `inv(targetWorld_J)` — the stored
   inverse-binds are *not* `inv(world_rest)` (they use a different bind convention; here they were
   90° off in rotation). Getting this wrong silently deforms only the weighted region under
   animation (here: the abdomen sank ~800 units underground while everything else was perfect).

`--keep` joints (and the root) keep their **world** transform instead (their local is recomputed so
they still hang correctly off the now-vanilla-framed parent). Use this for joints you deliberately
moved/added — e.g. Gohma's eye/eyelids relocated to the mouth (those are separately code-anchored, so
the game overrides them at runtime anyway).

The BMD size is unchanged (values only), so it's an in-place byte patch of JNT1 / VTX1 / EVP1.

---

## 3. Offline verification (trust it before you build)

The tool proves correctness **without launching the game**, via two invariants:

| Check | Meaning | Gohma result |
|---|---|---|
| **Rigid rest-position drift** | every rigid vertex's model-space rest position after reskin vs before | **0.0032 u** (mesh preserved) |
| **Weighted (EVP1) rest residual** | per-joint `targetWorld_J · newInvBind == sourceWorld_J · oldInvBind` | **0.00005** (weighted region preserved) |
| **Frame-match to vanilla** | every reskinned joint's world frame vs vanilla's (in root space) | **0.0001** (animates identically to vanilla) |

Rest-preservation (rigid **and** weighted) ⇒ the mesh is intact. Frame-match ⇒ animations authored
for vanilla frames drive it exactly as they drive vanilla. Together they mathematically guarantee
correct animation. The tool prints `==> PASS` only when all three are within tolerance.

**Lesson learned the hard way:** the first pass verified only *rigid* rest and *skeleton* animation
(both perfect) but skipped the weighted region — so a bad EVP1 formula shipped and sank the abdomen.
Always verify rigid **and** weighted, and if in doubt animate a real BCK offline (see the
`bck_test.py` approach: apply the game's own `.bck` to reskinned vs vanilla skeletons — every joint
must match `R90 · vanilla` to ~0, and weighted-vert bounds must stay near the body).

---

## 4. Usage

```
python tools/bmd_reskin/bmd_reskin.py \
    --source  <blender-edited.bmd> \
    --vanilla <original-game.bmd> \
    --out     <reskinned.bmd> \
    --keep    0x15,0x16,0x17 \    # joints you moved/added; root always kept
    --evp     1,2,3               # joints referenced by EVP1 weighted envelopes
```

Defaults are wired for the Gohma reveal model, so a bare `python tools/bmd_reskin/bmd_reskin.py`
reskins `tools/bmd_reskin/B_gm_37_prereskin_SOURCE.bmd` → `B_gm_37_reskinned.bmd` and prints the PASS.

**Deploy** (loose-BMD models — no game rebuild needed): copy the reskinned BMD over the
`model_replacements/<folder>/<name>.bmd`, wipe `dawn_cache.db*` + `pipeline_cache.db*`, playtest.

### Finding `--keep` and `--evp` for a new model
- `--keep`: the joints you intentionally moved or added in Blender (anything whose vanilla frame you
  do **not** want to adopt). If you moved nothing structurally, `--keep` can be empty.
- `--evp`: run the tool once; the `BMD.evp_indices` list (joints referenced by weighted envelopes) is
  parsed from EVP1. Pass those joint indices so their inverse-binds get rebuilt. Getting this wrong
  only affects the (usually small) soft-weighted region, and the verify step will flag drift there.

### CRITICAL gotcha — never reskin an already-reskinned file
Reskinning is **not idempotent**: it assumes the source has Blender frames. Always run it from the
**pre-reskin** Blender export (`--source`), and deploy the output separately. Keep the pre-reskin BMD
as the canonical source (here: `tools/bmd_reskin/B_gm_37_prereskin_SOURCE.bmd`). Re-running on the
deployed/reskinned file double-transforms and corrupts the mesh.

---

## 5. BMD parsing notes (for extending the tool)

- **Find sections via the header chain** (numSections at `0x0C`, walk sequentially from `0x20` by
  size) — **not** by searching for the 4-char magics. After writing new floats into VTX1, a vertex's
  bytes can coincidentally spell `JNT1`/`EVP1` and a content search will latch onto the wrong offset.
- **Positions and normals are indexed separately** in SHP1 primitives, into **separate arrays**.
- **Vertex component formats differ per attribute** (read them from the VTX1 attribute-format list):
  Gohma has Position = F32 XYZ (12 B/vert), **Normal = S16 XYZ, frac 14** (6 B/vert, value = s16/16384),
  Tex0 = S16 (4 B). Assuming F32 normals sprays writes past the array into later sections.
- **Rotation order is ZYX** (`R = Rz·Ry·Rx`), confirmed by FK matching vanilla to 0.3 u.
- SHP1 vertex→joint map: per shape, per packet, read the packet's matrix table (`matrixData` →
  `matrixIndexTable` slice of DRW1 indices); each primitive vertex's `PNMTXIDX/3` selects the slot →
  DRW1 entry → rigid joint or weighted envelope.

---

## 6. Alternatives considered (and why reskin won)

- **Fix it in Blender / SuperBMD flags** — impossible; Blender can't hold vanilla frames.
- **Binary-patch JNT1 only** — breaks the mesh; rigid verts are joint-local (see §1).
- **Runtime per-joint correction in the actor callback** — full FK re-computation every frame for
  60+ joints; expensive and complex.
- **Retarget the animations (custom BCKs) instead of the model** — viable but touches every shared
  anim and needs custom BCK authoring/loading; reskinning the one model is simpler and self-verifying.

---

---

## 7. Companion tool — `bmd_addtex.py` (inject textures a material needs)

**Tool:** [`tools/bmd_reskin/bmd_addtex.py`](../tools/bmd_reskin/bmd_addtex.py)

Same pipeline limitation, different symptom: **SuperBMD only embeds the mesh *diffuse* textures** it
finds in the DAE. Any texture a material samples that *isn't* a mesh diffuse — environment/reflection
maps, emboss/bump maps, indirect textures — is silently dropped and (in the `--mat` path) redirected
to a diffuse. On Armogohma's eye that turned the environment-mapped iris **pure white**: the eye's
TEV stage 1 computes `(TexMap1·iris + TexMap1.alpha) × 2`; vanilla's TexMap1 is the low-alpha env
sphere `goma_kankyo`, but the redirect pointed it at the **opaque** `goma_eye01` (alpha = 1), so the
stage clamped to white.

`bmd_addtex` fixes it by copying the missing textures from the original game BMD and repointing the
material — no re-encoding (image data is copied byte-for-byte from the donor):

1. **TEX1 rebuild**: `[section header][BTI headers][image data][string table]`, adding the donor
   textures. Copy each donor BTI header verbatim and fix **only** `dataOffset` (@0x1C, relative to the
   header). `paletteNumEntries` (@0x0A) must be 0 (non-paletted); the palette-offset field is then dead.
2. **MAT3 repoint**: **append** an extended texNo block (`old entries + new TEX1 indices`, e.g.
   `[0,1,2,3,4]`) at the section end, repoint the texNo header offset (index 15 of the MAT3 offset
   table) to it, and patch the target material struct's texNo-index slots (@struct+0x84) to the new
   block indices. Appending (not inserting) means **no other MAT3 block shifts** — only one header
   offset and the material's slots change. Other materials' slots still resolve (the leading block
   entries are unchanged).
3. Both MAT3 and TEX1 are the **last two sections**, so the tail is rebuilt and the file-size header
   updated; earlier sections are untouched.

Usage: `python tools/bmd_reskin/bmd_addtex.py --target <bmd> --donor <vanilla.bmd> --out <bmd>`
(defaults wired for the Gohma eye: donor textures `goma_kankyo,eye_enbos` → material `goma2_eye_mat01`
slots 1,2). It verifies offline: texture count/names, the material's resolved TEX1 indices, and that
injected image bytes match the donor. **Run order:** `source → bmd_reskin → bmd_addtex → deploy`
(reskin touches JNT1/VTX1/EVP1, addtex touches MAT3/TEX1 — disjoint, and addtex re-checks the reskin
is intact).

---

*Related:* [Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md) §12 (the reveal model this first
fixed), [Custom-Model-API-Work.md](Custom-Model-API-Work.md) (Layer-B loose-BMD delivery),
`memory/armogohma_phase3_blender.md` (Blender workflow + this fix in the project memory).
