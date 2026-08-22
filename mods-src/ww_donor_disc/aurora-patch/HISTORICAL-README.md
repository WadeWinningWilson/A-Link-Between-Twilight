> **CURRENT PROTOCOL: see [`AURORA-PATCH-LEDGER.md`](AURORA-PATCH-LEDGER.md).**
> Per user order 2026-08-22, all aurora code goes into a tracked `.patch`
> file FIRST, ledger row before application, with a redundancy check against
> upstream. This README's content below is **historical** (Integrator's
> loss-protection capture at the pre-rebase `81f12f31` state) — not
> rewritten, kept for the record it already is. `AURORA-PATCH-0001` is the
> current, byte-verified equivalent for what is actually live today.

# AURORA PATCH BACKUPS — uncommitted submodule work, captured 2026-08-17

> Written by **INTEGRATOR** during Phase 0/1, **before** any submodule
> operation. Both work-sets below were **uncommitted, in a submodule, in two
> different checkouts, and tracked by nothing.** A `git submodule update` after
> the Phase-1 fast-forward would have tried to check aurora out from
> `81f12f31` to `cf3ffc98` straight over them.
>
> This estate has already lost hours of uncommitted work to a stray `checkout`.
> These are the insurance, not the deliverable.

Both work-sets sit on the **same** aurora base commit,
`81f12f31d23ec822d8bde2031c91e94c470911eb` (`origin/fix-png-122-g81f12f3`).

| patch | source checkout | files | scope |
|---|---|---|---|
| `dusklight-main-aurora-81f12f31.patch` | `dusklight-main/extern/aurora` | 4 | 49 insertions — skip-draw + TEV alpha-op |
| `fork-aurora-81f12f31.patch` | `dusklight/extern/aurora` | 13 | 995 insertions — the above **plus** display-list / FIFO / command-processor work |

The fork's set is a **superset** of `dusklight-main`'s on the four shared files
(`lib/gfx/pipeline_cache.cpp`, `lib/gx/pipeline.cpp`, `lib/gx/shader.cpp`,
`lib/webgpu/gpu.cpp`).

## Why these four files matter — they are not incidental

They convert Dawn/WebGPU **shader and pipeline validation failures from FATAL
into skip-draw**, and fix a real TEV bug:

- `gpu.cpp` — the uncaptured-error callback no longer `FATAL`s on
  `ShaderModule` / `WGSL` / `RenderPipeline` validation errors; it logs and
  continues.
- `pipeline.cpp` — a null shader module returns an empty pipeline and logs,
  instead of proceeding.
- `pipeline_cache.cpp` — `get_pipeline` returns false on a null handle, so a
  failed `CreateRenderPipeline` becomes a skipped draw.
- `shader.cpp` — **`tev_alpha_op` was emitting colour-path `.r`/`.rg`/`.rgb`
  swizzles on operands that are already `f32` scalars**, which is invalid WGSL
  and made `CreateShaderModule` FATAL on `COMP_A8` / R8-on-alpha materials. Now
  the compare ops are expressed as scalar `select(...)`.

**Read that against `HANDOFF-INTEGRATOR.md` §7.4:** *"The `lwood` draw crash is
UNROOTED. Deterministic on every submitting boot, no CPU exception at all — the
backend is Dawn/WebGPU, and a device-lost kills the process at submission."*
This is work aimed squarely at that failure class. **Losing it would have cost
the root-cause trail, not just the code.**

## Restore

```
cd <checkout>/extern/aurora
git checkout 81f12f31d23ec822d8bde2031c91e94c470911eb
git apply <this-dir>/<patch>
```

Both patches were verified by `git apply --check --reverse` against the tree
they came from at capture time — they are faithful, not approximate. **A first
capture attempt silently grabbed the fork's *superproject* diff instead of
aurora's; the reverse-apply check is what caught it.** Do not skip that check
when re-capturing.

## Still open

Neither work-set is committed to any branch. **Patches in a directory are a
backup, not version control** — the durable fix is a branch in each aurora
checkout. Owner: user/Integrator.
