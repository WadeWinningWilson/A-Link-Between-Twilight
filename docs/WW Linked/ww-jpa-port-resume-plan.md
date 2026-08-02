# WW JPA port — RESUME PLAN (written 2026-08-02, ~02:20)

Pick this up cold. Everything below is either measured or cited; nothing is assumed.

---

## 0. Where we are (verified state)

| fact | evidence |
|---|---|
| Donor `common.jpc` parses natively, in place | `[wwJPA] archive parsed natively: 193 emitters, 96 textures` — matches the offline scan of the donor file exactly |
| `0x03DA` is the CORRECT scatter id for Outset | `d_grass.cpp:229-253` — `KINDANKUSA` only for stages `kin*`/`Xboss1`; else `ID_IT_JN_O_KUSA_KEN` = 0x03DA (cut) / `_RUN` = 0x03DB |
| Resource binds complete | `0x03da init: drawP=4 calcP=4 drawE=4 batch=0 bspType=4 tevSel=3 blend=0x05d9` |
| Particles ARE created | `ptclNum 120→144`, `emitterNum 10→15` across 58 cuts |
| Draw runs on donor data | callback = NULL (§237); `JPAResource::draw → pBsp->setGX → drawP` applies BSP1's own TEV/blend/z |
| Time-scaling ruled out | user capped to 30 fps → unchanged |
| Batch path ruled out as sole cause | `batch=0` changed the failure mode, did not fix it |

**Symptom that remains:** geometry reaches the screen but renders as garbage, possibly green-tinged.
Green matters — the donor's authored `prm = (152,200,118)` is the only green in the system, so donor
colour IS arriving.

**Conclusion that follows:** the fault is shared by both draw routes ⇒ it is in what the geometry is
made of (texture / vertices), not in which draw function dispatches it.

---

## STEP 1 — Texture resolution (do this first; highest probability)

**Hypothesis.** `bspType=4` is the cross-billboard type: two quads, correct shape, textured per
`mpTDB1`. Our TDB1 points at donor bytes and indexes the **shared 96-entry** texture table we
register from the archive. If the index resolves to the wrong entry, we get correctly-shaped
geometry with garbage texels — precisely the symptom.

**Where.** `src/d/ww_jpa_bind.cpp`, in `bindResource`, right after the texture-registration loop.

**Probe.**
```cpp
// §240 step 1: does TDB1 resolve to the donor's own texture?
for (u8 i = 0; i < res->texNum; i++) {
    const u16 gi = ww_jpa::be16((const u8*)r->texIdxTable + 2 * i);
    const ww_jpa::Texture* t = arc.texture(gi);
    DuskLog.warn("[wwJPA] {:#06x} tex[{}] -> globalIdx={} name='{}'",
                 (unsigned)resId, (int)i, (int)gi, t ? t->name() : "(null)");
}
```

**Expected.** `0x03DA` → `globalIdx=12`, `name='kusa_half'` (confirmed by the offline decode).
Windline `0x0031` → `globalIdx=48`, `name='AK_wind00'`.

**Decision.**
- Names correct → texture identity is fine; go to STEP 2.
- Wrong name / null → the shared-table assumption is broken. Fix: register only the textures each
  resource needs and rewrite its TDB1 to local indices (`0..N-1`), the same shape the donor's own
  per-resource table has.
- Correct name but still garbage → suspect the BTI itself: `JPATexture` reads `ResTIMG` at
  block+0x20. Verify format/width/height decode to IA8 16×32 (that is what the offline scan read);
  if `JPATexture` disagrees, the TEX1 block header offset assumption is wrong for this archive.

---

## STEP 2 — Vertex / UV template

**Hypothesis.** `JPAResource::initBatchInfo` and `setPTev` build quad corners from
`pEsp->getScaleCenterX()/getScaleCenterY()` and tiling bits. Those ESP1 fields come through **my
flag translation** (`ww_jpa_bind.cpp` §212 table). A wrong centre pair or tiling bit yields quads
built from the wrong template — geometry present, shape wrong.

**Probe.** In `bindResource` after `res->init(heap)`:
```cpp
DuskLog.warn("[wwJPA] {:#06x} tmpl: centreX={} centreY={} tilingS={} tilingT={} "
             "vtx={} espFlagsWW={:#010x} espFlagsTP={:#010x}",
             (unsigned)resId,
             res->pEsp ? (int)res->pEsp->getScaleCenterX() : -1,
             res->pEsp ? (int)res->pEsp->getScaleCenterY() : -1,
             (int)res->pBsp->getTilingS(), (int)res->pBsp->getTilingT(),
             (int)res->mBatchInfo.vtxCount,
             (unsigned)r->esp().flags(), (unsigned)espTranslatedFlags /* keep a copy */);
```

**Compare against the donor.** WW's own accessors (`JPAExtraShape.h:83-117`): pivot X = `>>14 & 3`,
pivot Y = `>>16 & 3`. Decode `espFlagsWW` by hand and check the translated values match. Donor grass
ESP1 flags can be read offline from `common.jpc` if a second opinion is wanted.

**Decision.**
- Mismatch → fix the translation table entry; that is a one-line change with a citation.
- Match → template is fine; go to STEP 3.

---

## STEP 3 — Per-particle colour

**Hypothesis.** On the classic path, colour reaches each particle through the `JPARegist*` draw
functions (`JPARegistPrm`, `JPARegistPrmAlpha`, …), chosen by `init` from BSP1's colour flags. We
clear the anim bits in §234, so the chosen function may differ from the donor's.

**Probe.** Log which draw functions `init` selected (compare pointers against the known
`JPADraw*`/`JPARegist*` symbols) plus `mBatchInfo.hasPtclColor`.

**Decision.**
- No `JPARegist*` present → particles draw with whatever is left in the TEV registers → garbage.
  Fix: ensure the colour-enable bits we keep (`0x01`/`0x04`) map to the function `init` expects.
- Present → colour path is fine and the remaining suspect is blend/z (`blend=0x05d9`) against the
  scene's current GX state.

---

## 4. Cleanup once particles render correctly

1. Delete the converter: `wwJpa1ExtractEmitterToJpac2`, `wwJpa1ArchiveHasResId`, the `wwPutBe*`/
   `wwGetBe*` helpers, and `DUSK_JPAC_ESP1` / `DUSK_JPAC_FLD1` toggles (~250 lines,
   `src/d/d_particle.cpp`).
2. Strip probes: `§236` readouts (bind + veg), `§222` `newSimple` discriminator, `§62 cutFx` if the
   effect is accepted.
3. Retire `dPa_wwUnlitEcallBack` (§230/§231) — superseded by letting the resource draw itself.
4. Remove `run_jpac_dump.bat` and the `DUSK_JPAC_DUMP` block.
5. Re-run the rehoming sweep; commit.

## 5. Then — swood (Foundry order, currently HELD)

Held deliberately until particles render, so it is not built on an unproven base. Already done:
**all blobs extracted** (`assets/veg/d_tree__*`, sizes decomp-exact). Corrections to the order
(§231): the batcher is **`d_tree`, not `d_wood`**; it is **5 DLs + a state table**
(`l_modelStatus = {0,1,2 / 3,1,4}` → intact draws none/leaves/trunk, cut draws cut-upper/leaves/
trunk) plus a shadow pass. Work = register `swood`/`swood3`/`swood5`, implement the 3-part draw,
port the cut branch (full-state-machine law).

---

## 6. Environment toggles (all default OFF unless noted)

| var | effect |
|---|---|
| `DUSK_WW_BATCH=1` | re-enable the batch draw path for WW resources (default: classic) |
| `DUSK_WW_LIT=1` | restore the light8 callback (the purple, time-of-day tinted look) |
| `DUSK_WW_SIMPLE=1` | try the registration-gated `setSimple` path (spawns nothing; kept for diagnosis) |
| `DUSK_JPAC_ESP1=1` / `DUSK_JPAC_FLD1=1` | legacy converter block carry — dead once the converter is deleted |
| `DUSK_JPAC_DUMP=1` | dump the converted blob (legacy; `run_jpac_dump.bat`) |
| `DUSK_FPS_PROBE=0` | disable Ferry T frame buckets |

## 7. Test-loop discipline (this cost us several rounds)

1. **Always check the exe timestamp against the log's start time.** Three separate test rounds ran a
   stale binary because the build finished after the session started.
2. Wipe `dawn_cache.db*` / `pipeline_cache.db*` after every build (standing rule).
3. A silent failure is likely a silent CHANNEL: `OSReport`, `JUT_WARN` and `OS_REPORT` do **not**
   reach `dusklight.log`. Three real faults hid behind them this session. When something fails with
   no log line, check whether the code path even logs to DuskLog.
4. Diagnostic scaffolding has caused two failures of its own — strip probes once they answer.
