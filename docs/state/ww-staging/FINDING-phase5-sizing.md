# PHASE 5 SIZING — the two blocking subsystems are far more portable than their headline numbers

> Measured 2026-08-17 by INTEGRATOR, after Phase 4a found three of five sites
> blocked not on hookability but on **fork subsystems living tree-side**.

## The headline numbers overstate the job by ~6x

| subsystem | files referencing | distinct symbols | total call sites |
|---|---|---|---|
| `dExtNpcMount_*` | 57 | 113 | 804 |
| `dKyWw_*` | 45 | 25 | 189 |

**Those counts are CALLERS, not entangled definitions.** Both subsystems are
cohesive — each lives in its own dedicated TUs:

| subsystem | where it is DEFINED |
|---|---|
| `dKyWw_*` | `d_kankyo_ww.cpp` (10) · `d_kankyo_ww_wind.cpp` (11) + their headers |
| `dExtNpcMount_*` | `d_ext_npc_mount.cpp` (65) + header · **`src/d/ext_plugin/ww_npcmount_dispatch.cpp` (32)** |

**Part of the mount is ALREADY under `src/d/ext_plugin/`** — a third of its
definitions are staged for plugin-side by whoever wrote them.

## The real cost: only the SHARED-TU call sites need re-siting

A call site in a **fork-added TU moves with the subsystem** — no hook, no seam,
it is our file either way. Only call sites in **TUs that also exist in vanilla**
need re-siting. Split by whether the file exists in `dusklight-main`:

| subsystem | in FORK-ADDED TUs (move free) | **in SHARED TUs (need re-siting)** |
|---|---|---|
| `dExtNpcMount_*` | **609** across 43 files | **96** across **14 files** |
| `dKyWw_*` | **148** across 37 files | **41** across **8 files** |
| **total** | **757** | **137 across 22 files** |

**757 of 894 call sites — 85% — move as a unit.** The re-siting surface is
**137 call sites in 22 shared files**, and they cluster:

- **mount:** `d_a_npc_kdk` (23) · `d_a_npc_p2` (13) · `d_a_npc_mk` (13) ·
  `d_a_npc_henna0` (13) — WW NPC actors · plus `dusk/ui/warp.cpp` (11) and
  `d_s_play.cpp` (5)
- **kankyo:** `d_kankyo_wether.cpp` (22) · `d_a_vrbox` (7) · `d_kankyo_rain`
  (3) · `d_a_vrbox2` (3) · `d_kankyo` (2) · `d_a_swhit0` (2)

## Why this reframes the "last blocker"

Phase 4a found the three blocked sites were blocked on **fork code in the wrong
place**, not on host symbols we cannot reach. This sizes that move:

**Moving both subsystems plugin-side unblocks `daBg_c::createHeap` AND both
lighting sites — 3 of the 5 — in one action**, at a cost of re-siting 137 call
sites across 22 files rather than the 894/102 the raw counts imply.

Compare with the alternative: re-siting the five 4a sites one at a time leaves
the subsystems tree-side and the fork permanently un-shippable as a plugin.

## What this does NOT say

- **Cohesive is not the same as movable.** The definitions are concentrated;
  whether they compile plugin-side (types, headers, host data access) is
  unmeasured. Phase 5 is header-ful precisely to make that tractable, but it
  is a claim to test, not to assume.
- **The 137 are call sites, not solutions.** Each still needs a route — a
  hook, a native switch (as the roof clamp got), or deletion if it turns out
  to be diagnostic. Phase 4a's own census found 7 of 12 sites were
  instrumentation; the same discount may apply here and is not yet claimed.
- **Per-frame cost is untested** for anything moved across the boundary. The
  briefing's own warning stands: hot per-frame subsystems may need to stay
  tree-side, and `d_kankyo_wether.cpp`'s 22 calls are on a weather path that
  runs every frame.

---

## Triage of the 137 — the Phase-4a discount does NOT repeat. This is real surface.

Phase 4a's census found **7 of its 12** sites were pure instrumentation. I
expected a comparable discount here and said so. **It is not there.**

Classifying each shared-TU call site by whether a log macro (`DuskLog`, `logf`,
`OS_REPORT`, `printf`, `JUT_`) appears within ±2 lines:

| subsystem | diagnostic-adjacent | **BEHAVIOUR** | files |
|---|---|---|---|
| `dExtNpcMount_*` | 4 | **92** | 13 |
| `dKyWw_*` | 6 | **35** | 8 |
| **total** | **10** | **127** | **21** |

**Only 10 of 137 are diagnostics. 127 are behaviour.** The hypothesis was
wrong and the number goes the other way — worth stating plainly, because an
expected discount that fails to appear is exactly the kind of thing that
quietly gets dropped rather than reported.

### Where the behaviour actually concentrates

**MOUNT — 59 of 92 sit in four NPC actors that exist in vanilla:**
`d_a_npc_kdk` (23) · `d_a_npc_mk` (13) · `d_a_npc_p2` (13) ·
`d_a_npc_henna0` (10). Plus `dusk/ui/warp.cpp` (11) and `d_s_play.cpp` (5).

**KANKYO — `d_kankyo_wether.cpp` alone is 20 of 35**, with `d_a_vrbox`/
`d_a_vrbox2`/`d_kankyo_rain` at 3 each.

### What that does to the Phase-5 estimate

**Unchanged and still good:** 757 of 894 call sites (85%) move with the
subsystems at no re-siting cost, and both subsystems are cohesive in their own
TUs.

**Revised and worse:** the re-siting surface is **127 behavioural call sites
across 21 files**, not "137 minus a diagnostics discount". Every one needs a
route — a hook, a native switch (as the roof clamp got), or a demonstration
that it is dead.

**The concentration is the opportunity.** Four NPC actors carry 59 of the 92
mount sites, and one weather TU carries 20 of the 35 kankyo sites. **Five files
hold 79 of the 127.** If those five have a shared shape — as the twelve 4a
sites did, all being `if (isWwHostStage(...))` — the move collapses to a small
number of patterns rather than 127 individual decisions. **That shape check is
the next measurement and it has not been run.**

---

## Shape check — the 127 are NOT 127 decisions. They are ~2 patterns.

**13 distinct symbols carry 101 of the sites** across the five hot files.

### The four NPC actors are one template, copy-pasted

| file | sites | distinct symbols | top-4 |
|---|---|---|---|
| `d_a_npc_kdk` | 28 | 10 | `_c` 11 · `_create` 5 · `_takePendingSpawn` 2 · `_forceNextSpawnSrc` 2 |
| `d_a_npc_mk` | 17 | 10 | `_c` 7 · `_create` 2 · `_takePendingSpawn` 1 · `_forceNextSpawnSrc` 1 |
| `d_a_npc_p2` | **17** | **10** | **identical to mk** |
| `d_a_npc_henna0` | **17** | **10** | **identical to mk** |

**`mk`, `p2` and `henna0` are byte-for-byte identical in shape** — same site
count, same symbol count, same top-four with the same frequencies. `kdk` is the
same integration, larger (it is plainly the reference the other three were
copied from).

**So 79 sites across four files are ONE mount-integration pattern applied four
times**, not four independent jobs.

### The weather file is one predicate, repeated

`d_kankyo_wether.cpp`: **`dKyWw_isSkyHost` appears 20 times** out of 22 sites.
The other two are single calls (`_wind_set`, `_skyArcsPoll`).

That is the same shape Phase 4a found in `isWwHostStage` — **a boolean host
gate consumed at many sites** — and it takes the same treatment: the plugin
already knows the answer, so the question is where the *consumers* live, not
how to reproduce the producer.

## Revised Phase-5 estimate — and this is the one that matters

| | |
|---|---|
| raw call sites | 894 |
| move free with the subsystems (fork-added TUs) | **757 (85%)** |
| need re-siting | 137 |
| of those, behaviour | **127** (the 4a diagnostics discount did NOT repeat) |
| **of those, distinct PATTERNS in the hot five files** | **~2** — one NPC mount template (79 sites) + one sky-host predicate (20 sites) |

**79 + 20 = 99 of the 127 behavioural sites collapse into two patterns.** The
residue is ~28 scattered sites in `warp.cpp` (11), `d_s_play.cpp` (5),
`d_a_vrbox`/`vrbox2`/`d_kankyo_rain` (3 each) and a handful of singletons.

**Not yet done:** the two patterns have been identified by *symbol frequency*,
not by reading the code at each site. Identical symbol profiles strongly imply
identical integration, and in three files it is near-certain — **but "same
symbols in the same proportions" is not "same code", and that verification has
not been run.** It is cheap and it is the next step.

---

## Pattern claim VERIFIED BY READING — and it needed the refinement

The "~2 patterns" estimate above was derived from **symbol frequency**, and I
flagged that "same symbols in the same proportions" is not "same code". Checked
by comparing the actual call-site lines:

| pair | identical call-site lines |
|---|---|
| `d_a_npc_mk` vs `d_a_npc_p2` | **12 / 13** |
| `d_a_npc_mk` vs `d_a_npc_henna0` | **8 / 13** |

**The template is real** — every one opens the same way:

```cpp
dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
fopAcM_ct(a, dExtNpcMount_c);
if (dExtNpcMount_takePendingSpawn(fopAcM_GetID(a), proc, sizeof(proc), src, sizeof(src), ...
dExtNpcMount_forceNextSpawnSrc(src);
```

**But it is a template WITH VARIATION, not a uniform copy.** `henna0` diverges
on 5 of 13 lines. So the Phase-5 move is *"one pattern plus per-actor deltas"*,
not *"one transform applied four times"* — closer to the estimate than to 127
independent decisions, and further from it than my first reading claimed.

**Recording the correction rather than the convenient version:** the frequency
evidence said uniform; the code says templated-with-drift. The estimate stands
directionally and the per-file work is real.
