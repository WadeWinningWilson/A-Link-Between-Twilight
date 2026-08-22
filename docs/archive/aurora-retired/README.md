# RETIRED — aurora F1 + F2 (WW material tolerance)

era: era-independent
<!-- era rationale: retired artifact with a re-entry test; not tied to a milestone | Housing/Engine, 2026-08-22 -->

**Status: RETIRED ON THE USER'S RULING, 2026-08-22. Preserved deliberately,
disconnected from the plugin. Not applied, not built, not referenced by any
build file. Kept for a rainy day.**

`F1-F2-ww-material-tolerance.patch` — 5 files, +145/−25, cut from the fork's
aurora work-set (`81f12f31..fc2e1cd`).

## What this is

The **only load-bearing part** of what was long described as a "995-line, 13-file
aurora patch." That headline was misleading and this file exists partly to stop it
being repeated:

| portion | lines | what it actually was |
|---|---:|---|
| `command_processor.cpp`, `gx.cpp`, `GXGeometry.cpp`, `fifo.cpp/.hpp` | ~840 | **Housing/Engine's own diagnostics** — FIFO provenance ring, anomaly reporter, draw-state probe, VCD history. Built to *find* the lwood runaway. |
| **this patch** | ~145 | the functional changes |

The diagnostics' purpose was discharged the moment the lwood runaway was found and
fixed **plugin-side** (one entry per `dDlst_list_c::reset()` fill window — see
`ww_wave.cpp:944`, which documented the same hazard years before). They are not
preserved here because they debug a bug that is closed.

## What F1 and F2 do

**F1 — `lib/gx/shader.cpp`.** Stock aurora emits **invalid WGSL** for TEV
alpha-compare ops (`GX_TEV_COMP_*_GT` / `_EQ`): it applies colour-path `.r/.rg/.rgb`
swizzles to what is already an f32 scalar on the alpha path, and
`CreateShaderModule` then fails. F1 emits correct `select(...)` expressions instead,
and adds `is_texmap_sampled` so an indirect stage with an unbound texmap goes
neutral rather than failing.

**F2 — `pipeline_cache.cpp`, `pipeline.cpp`, `gpu.cpp`, and the `shader_info.cpp`
`CHECK` relaxation.** Survival hardening: a null shader/pipeline becomes
**skip-draw** instead of a process kill, and an unbound texcoord/texmap is tolerated
with a warning rather than a hard `CHECK`. Its own comment: *"shader/pipeline
validation from exotic WW materials must skip-draw, not kill the process."*

## Why it was retired

**Neither F1 nor F2 was ever lwood's fix**, and lwood was the motivating defect.
Proven offline from the user's own disc: lwood's material has two TEV stages,
`colorOp` and `alphaOp` both `GX_TEV_ADD`, and **both** `TevOrderInfo` entries
bound. Tool: `tools/foundry/j3d_bdl_tev.py`.

The question then became *who else uses these*, and the honest answer depends on
which set you measure:

| scope | COMP-op-on-alpha (F1) | unbound texmap (F2) |
|---|---:|---:|
| whole donor disc (577 arcs, 532 parsed) | 31 arcs | 85 arcs |
| **arcs actually served today** (4 donor arcs touched) | **0** | **1** (`Title`, and see caveat) |

The disc-wide figure is real but **irrelevant to whether the patch is needed** — a
condition sitting in data that is never loaded costs nothing. Against the served set
there is no demonstrated consumer. The single `Title` hit is matched **by name**, and
this project's own source documents that donor arc names *collide* with the
receiver's (`main.cpp` cites `Always.arc`), so it may be the receiver's arc entirely.
`Akabe` failed to parse and is therefore **not cleared** — one arc, named, unresolved.

## The test for bringing it back

**Not "lwood broke again."** The re-entry test is:

> **Does any arc in the CURRENTLY SERVED set use a TEV COMP op on the alpha channel,
> or carry an unbound texmap/texcoord?**

Run `python tools/foundry/j3d_bdl_tev.py <arc>` over the served set. That is one run.

This matters because the answer is a **function of port scope, not a settled fact**.
Today's served set is Outset (`sea` room 44) and its dependencies. Every time the
served set grows, the question is **re-asked** — not re-opened. The disc-wide 31/85
says the conditions genuinely exist in donor content, so a future port very likely
does reach them.

## What this patch is NOT

- Not applied anywhere. `dusklight-main/extern/aurora` is stock (`e9ec4a7`).
- Not referenced by any build file.
- Not a fix for lwood, which is fixed plugin-side and needs no aurora change.
- Not a complete record of the fork's aurora divergence — it is the functional
  subset only. The rest was instrumentation and is deliberately not preserved.
