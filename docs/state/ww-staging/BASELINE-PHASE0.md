# BASELINE — PHASE 0 (the BEFORE side)

> ## 🔎 BEFORE YOU DESIGN AGAINST ANY SYMBOL — GREP THE PLUGIN FOR IT FIRST
> **Integrator, 2026-08-17, filed as a PROCESS defect rather than a doc defect;
> placed here because this is the file a post-merge instance opens first.**
>
> **In one day this lane designed against a surface the plugin ALREADY COVERED
> SEVEN TIMES** — a duplicate `CrrPos` hook, a duplicate `BgDraw` hook (which a
> predecessor had already proven **INLINED-DEAD**, with an inline disambiguator
> built beside it saying exactly that), and five smaller ones.
>
> **EVERY RECOVERY CAME FROM READING AN ARTIFACT. NOT ONE CAME FROM REASONING
> FURTHER.** That is the whole finding: more thought does not surface prior
> work, and the cost is paid in design effort that was never needed.
>
> ```bash
> grep -rn "<symbol>" mods-src/ src/dusk/          # is it already covered?
> ```
>
> **Mechanical, cheap, and it belongs before the design — not in a review
> after it.**


> Captured by **INTEGRATOR (successor)** on **2026-08-17**, on the user's order
> routed by Foundry with `BRIEFING-P1-VIABLE.md`. This file exists so the
> Phase-0 rule — *anything that moves after the merge is a merge artifact until
> proven otherwise* — is usable at all. **Every line is dated and sourced.
> Items that could not be captured are listed as NOT CAPTURED with the reason,
> not omitted.**

## 0. Tree state at capture

| | |
|---|---|
| branch | `integrate/dusk-api-coexist` |
| HEAD | `ab02eae123` — *Retire the Integrator lane* |
| working tree | 13 modified + 1 untracked (docs/monitor churn only) |
| **source dirt** (`src/ include/ libs/ mods-src/ files.cmake`) | **NONE — clean** |

## 1. Divergence — RE-DERIVED TODAY, not inherited

`git fetch origin` run 2026-08-17, then:

| measure | value | source |
|---|---|---|
| behind (`HEAD..origin/main`) | **2,861** | `git rev-list --count` |
| ahead (`origin/main..HEAD`) | **637** | `git rev-list --count` |
| merge base | `be82e606b2` *45 achievements* | `git merge-base` |
| divergence date | **2026-05-08** | base commit date |
| `origin/main` tip | `c880d46fb5` 2026-08-15 *d_menu_option bugfix (#2294)* | |
| `upstream/main` tip | `c8451da945` 2026-06-27 | our fork; **remotes are inverted** |

**The briefing's 2,861 is EXACT and is no longer a July number** — it re-derives
to the digit after a fetch today. History/Bridge's omitted figure was 635 ahead;
it is now **637** (the two retirement commits). Phase 1 is a **reconciliation of
637 against 2,861**, not a catch-up.

## 2. Gates and stores — captured clean

| instrument | result |
|---|---|
| `seam_gate.py check` | **PASS** — row authority intact, no unclassified WW-layer changes |
| `row_store.py validate` | **122 rows · 0 errors** |
| `staging.py` | 2 rows · 0 errors |
| `control.py` (audit) | **26 gates demonstrated live · 0 BLIND · 6 undeclared** |
| `build_identity.py current` | `FB887B64605E4D89B63356D2513450E6` age 866, pdb `dusklight.pdb` |

The six undeclared gates are `routing_check` · `call_receipt` · `kit_laws`
(1-5, 8-10) · `ordinal_differ` · `port_deps` · `fold_detect`. **Undeclared is not
passing — it is unknown**, and it is unknown on the BEFORE side too, so it does
not become a merge artifact.

## 3. Binaries on disk

| path | size | mtime | sha256 |
|---|---|---|---|
| `build/windows-msvc-relwithdebinfo/dusklight.exe` | 39,345,664 | 2026-08-17 00:12 | `ee928f2ce193511bf8c9086c28fde50fb28047edda5a438861b79b6827e2c360` |
| `build/install/dusklight.exe` | 39,333,888 | 2026-08-16 12:41 | `b6545dbc062288f7347a4d41c4a963a0d24e861e257fa0d075265da77dbec623` |
| `build/windows-msvc/dusklight.exe` | 30,870,528 | 2026-05-28 14:56 | `26c0255ecfa1575648fe25f3a0185e15a59cdf6b10988cc58bbd128392a7bdc5` |

## 4. ⚠️ THE STANDING EXE IS A DIRTY BUILD — the baseline is NOT reproducible

`build-ledger.jsonl`, last row 2026-08-17T15:58:28Z:

```
build_id FB887B64605E4D89B63356D2513450E6-866
git_head 852e4db843
git_dirty 174
```

The shipping exe was built from HEAD `852e4db843` **plus 162-174 uncommitted
paths**, and the log confirms it: `Build: v1.4.1-280-dirty (rev 852e4db843,
built Fri Aug 14 11:31:10 2026)`.

Those uncommitted paths have **since been committed** (`75c12dda7e`,
`a8f163488a`, `ab02eae123`), so the exe's source is *probably* close to HEAD.
**"Probably" is not a baseline.** `git diff 852e4db843..HEAD` over source is
**30 files / 9,560 insertions**, including `ww_stage_loader.cpp` (+258),
`ww_room_loader.cpp` (+203), `ww_event_names.cpp` (+58), `manifest.hpp` and
`CMakeLists.txt` — and there is no way to prove from the tree which of those
were already on disk at build time.

**CONSEQUENCE: a clean rebuild at `ab02eae123` is required before the merge**, or
every post-merge delta is measured against an unreproducible binary. This is the
single most load-bearing gap in the BEFORE side. Owner: Integrator.

**DISCHARGED 2026-08-17 19:40 — rebuilt clean.** `build_run.bat` at HEAD
`c4724821b7`, and **`git status` over `src/ include/ libs/ mods-src/ files.cmake
CMakeLists.txt` is EMPTY — zero source dirt**, so this binary IS reproducible
from a commit. (The build ledger records `+8` dirty: all of it monitor-pulse,
build-ledger and BUILD-IDENTITY churn, none of it source.)

| | |
|---|---|
| exe | `build/windows-msvc-relwithdebinfo/dusklight.exe` |
| size / mtime | 39,345,664 · 2026-08-17 19:40 |
| **sha256** | `e5e023ef93b5689cf44dffba12c9b4134e7027b009e211ed05cb648f60b17ab8` |
| build_id | `FB887B64605E4D89B63356D2513450E6-867` |
| link | 31,982 exports (2,567 data, 277 forwarded) from 1,528 objects |
| caches | `dawn_cache.db*` / `pipeline_cache.db*` wiped post-build |

**This — not the Aug-14 dirty exe — is the BEFORE binary.** The assert battery
and the FPS band still need a live run against it.

## 5. Assert battery — CAPTURED, AND STALE. Read the caveat.

`log_check.py` against `dusklight-20260817-005331.log`:

```
8 PASS · 2 UNTESTED · 0 FAIL     (exit 3 — SESSION NOT VERIFIED)
```

PASS: `seam-r44` · `fili-r44` · `rcam-recv` · `scls-translated` · `entry-omasao`
· `rcam-donor` · `crash-sweep` · `seam-error-sweep`.
UNTESTED: `roundtrip-ojhous` (required — hence exit 3) · `rdl02-entry`.

**This battery belongs to the DIRTY Aug-14 exe of §4, not to HEAD.** The
briefing calls for "the 8/8 assert battery"; what exists is **8 PASS / 2
UNTESTED / 0 FAIL at exit 3**, which is not 8/8 and never was. Carrying "8/8"
forward would import a number nobody measured.

## 6. NOT CAPTURED — and why

| item | why not | who can |
|---|---|---|
| **field FPS (220-250 band)** | requires live play; FPS is **not written to the log** (`grep -ci fps` = 1, the header only) | user, or an instrumented run |
| **golden traces** | no receiver-side golden-trace corpus exists on disk. The only artifact under that name is `docs/WW Linked/tale-golden-trace-run1.md`, a **donor** capture, and it is git-tracked so it survives the merge unchanged. The oracle stack's traces are produced by a RUN (`probe_differ.py` / DuskTap), not stored | Integrator, after a build |
| **assert battery at HEAD** | see §5 — the only battery is against the dirty Aug-14 exe | Integrator, after a build + playtest |

**Searched wider than `docs/` before writing "no corpus exists"** — whole tree,
`build/` excluded only for vendored googletest/zstd fixtures. (§7.7: three lanes
in one day reported the limit of their own search as a property of the world.)

## 7. Seam census — NOT re-derived, deliberately

122 rows · 0 errors · 31/31, per §7.6 item 4. Phase 2 re-measures **against**
it. It is not scrap and it was not re-run.

---

# PHASE 1 — SCOPED AGAINST THE RIGHT REPOSITORY

> **CORRECTION, 2026-08-17, written at the point of the misread rather than as a
> banner (§7.7).** My first pass scoped Phase 1 as `this fork ← origin/main` and
> predicted **443 conflicted paths**, then reported that *"the ownership map is
> not the conflict map."* **The user corrected the target: this fork is itself
> the mod. The clean dusklight that needs the merge is
> `%USERPROFILE%\Documents\dusklight-main`.**
>
> That 443-conflict prediction was a correct measurement of the **wrong
> repository** — the estate's single most-repeated failure shape (§5: *a correct
> measurement of a narrower surface than the claim it supported*). **Superseded,
> not deleted:** it remains the honest answer to *"what would it cost to
> reconcile the fork against upstream"*, which is a question P1 argues we should
> stop asking. **Do not cite 443 as the cost of Phase 1.**

## The real target, measured

`%USERPROFILE%\Documents\dusklight-main`, branch `main`:

| measure | value |
|---|---|
| HEAD | `95608438c1` — *Merge pull request #2227*, dated **2026-07-19** |
| **ahead of `origin/main`** | **0** |
| **behind `origin/main`** | **39** |
| `git merge-base --is-ancestor HEAD origin/main` | **TRUE** |
| working tree | 2 paths (`extern/aurora` modified, untracked `_build_exe.bat`) |

**This is a PURE FAST-FORWARD of 39 commits. Zero conflicts are possible** — not
"few", not "predicted to be few": a fast-forward has no merge in which to
conflict. `95608438` is also the rev the estate already records as *vanilla*.

**Phase 1 is therefore not a reconciliation of 637 against 2,861. There is
nothing to reconcile.** The briefing's *"expect conflicts precisely where the
seam work predicted them — the eight leg-carriers"* describes a fork-vs-upstream
merge that does not need to happen, because the delivery model P1 argues for is
**plugin-on-unmodified-vanilla**.

## What the 39 commits deliver — read out of the headers, not inferred

`git diff --name-status HEAD..origin/main -- sdk/`:

**ADDED:** `svc/save.h` · `svc/stage.h` · `svc/item.h` · `svc/window.h` ·
`svc/hook.hpp` · `svc/log.hpp`
**MODIFIED:** `api.h` · `hook.hpp` · `meta.hpp` · `service.hpp` ·
`sdk/CMakeLists.txt` · and every pre-existing `svc/*.h`

**Every service the briefing named is present and accounted for** — SaveService,
StageService, ItemService and WindowService are exactly the four `A` rows. They
do **not** exist at `95608438c1`; they arrive in the 39.

The ABI epoch bump is confirmed **in the header itself**:

| | `sdk/include/mods/svc/game.h` |
|---|---|
| at `95608438c1` | `#define GAME_SERVICE_MAJOR 1u` |
| at `origin/main` | `#define GAME_SERVICE_MAJOR 2u` |

The header states its own contract — *major version is the game-code ABI epoch,
bumped when game-visible struct or vtable layouts change* — and the loader
**fails mods built against the old epoch with a clear message instead of
corrupting memory.** That is the guard the briefing said already exists.
Confirmed. **Phase 5 must build no parallel mechanism.**

## What Phase 1 reduces to

```
cd %USERPROFILE%\Documents\dusklight-main
git merge --ff-only origin/main
```

39 commits, no conflicts possible. Then rebuild vanilla and re-baseline it.

**The risk in Phase 1 is not merge conflicts — it is that the fast-forward
crosses the ABI epoch boundary**, so anything built against GameService major 1
is invalidated by design. That is a build/runtime question to measure after, not
a textual-merge question to predict before.

## What this does NOT settle

- **Whether our WW plugin loads and runs against epoch-2 vanilla. Unmeasured.**
- Whether the fork still needs upstream at all. Its 2,861/637 divergence is
  real; it is simply **not on the Phase-1 critical path** if delivery is
  plugin-on-vanilla. That is the user's decision, not a measurement.
- **`extern/borealis` failed to fetch in `dusklight-main`** — *"Could not access
  submodule 'extern/borealis' at commit cf8ab52f55"*. **Flagged, not diagnosed.**
  It may bite the post-fast-forward build.

---

# PHASE 1 — EXECUTED 2026-08-17

## The fast-forward landed

```
cd %USERPROFILE%\Documents\dusklight-main
git merge --ff-only origin/main        # exit 0
```

| | before | after |
|---|---|---|
| HEAD | `95608438c1` (2026-07-19) | **`c880d46fb5`** (2026-08-15) |
| behind `origin/main` | 39 | **0** |
| conflicts | — | **none possible; none occurred** |

**Verified on disk, not inferred from the commit list:**

- `sdk/include/mods/svc/` now carries **`save.h` · `stage.h` · `item.h` ·
  `window.h`** alongside `hook.hpp` / `log.hpp`.
- Implementations are real, not headers-only: `src/dusk/mods/svc/` has
  `save.cpp` · `stage.cpp` · `item.cpp` · `window.cpp` (+ `registry.cpp`,
  `slot_map.hpp`).
- **`grep 'define GAME_SERVICE_MAJOR' sdk/include/mods/svc/game.h` → `2u`.**
  The epoch boundary is crossed.
- `version.h.in` and `src/dusk/update_check.*` are **deleted** upstream;
  `language.*`, `presentation.*`, `utilities.*` are new.

## `extern/borealis` — diagnosed, and it was never a blocker

The fetch warning *"Could not access submodule 'extern/borealis' at commit
cf8ab52f55"* was **benign**. Borealis is a **new** submodule introduced by
`13b3b68fe5 Migrate to Borealis (#2266)` (2026-08-05). Git recursed into
submodules at a commit that declares borealis while borealis was not yet cloned
locally — **you cannot init a submodule before checking out the commit that
declares it.**

`git ls-remote https://github.com/encounter/borealis.git` resolves, and the
pinned gitlink `6fd955e7e6` **is that repo's own `refs/heads/main` HEAD.**
Cloned post-fast-forward at 610 KB. Done.

**And it does NOT replace aurora** — despite the commit title. Borealis is
*"Modules for Aurora-based ports"*: cli, crash, data, disc, discord,
file_select, log, presentation, sentry, update. `CMakeLists.txt` still links
`aurora::core aurora::gx aurora::gd …` **alongside** `borealis::*`. **Anyone
reading "Migrate to Borealis" as an aurora replacement will port work that does
not need porting.**

## 🟡 THE ONE THING THE FAST-FORWARD DID NOT RESOLVE — aurora is now pinned ahead of its worktree

The superproject now pins `extern/aurora` at **`cf3ffc98`**. The checked-out
submodule is still at **`81f12f31`** — deliberately, because it carries **49
insertions of uncommitted local work** that a `git submodule update` would have
checked out straight over.

**That work was backed up as verified patches BEFORE anything moved** —
see [`aurora-patches/`](aurora-patches/README.md). Two independent work-sets,
both on `81f12f31`, both uncommitted, both tracked by nothing:
`dusklight-main` 4 files / 49 insertions, and **the fork 13 files / 995
insertions** (a superset on the four shared files).

### Cost of moving aurora to `cf3ffc98`, measured in a throwaway worktree

`81f12f31..cf3ffc98` is **39 aurora commits**, and all four locally-edited files
changed in that range (`shader.cpp` alone: 266 lines).

| file | `git apply --3way` onto `cf3ffc98` |
|---|---|
| `lib/gfx/pipeline_cache.cpp` | **clean** |
| `lib/webgpu/gpu.cpp` | **clean** |
| `lib/gx/pipeline.cpp` | **conflicts** |
| `lib/gx/shader.cpp` | **conflicts** |

**Two of four rebase for free. Two need hands** — and the conflict cause is
identifiable: upstream `4998dcc Avoid provably-redundant tev_overflow calls`
removed the `tev_overflow(...)` wrapping that the local fix wraps its operands
in. So the conflict is *signature drift around the same lines*, not a
disagreement about behaviour.

### And upstream has NOT fixed the underlying bug — checked, not assumed

`tev_alpha_op` at `cf3ffc98` is still:

```
std::string expr = tev_op(op, bias, scale, a, b, c, d, "0.0"sv);
```

— the unguarded form that routes compare ops through the colour path and emits
`.r`/`.rg`/`.rgb` swizzles on operands that are already `f32` scalars. **The
local `select(...)` scalar rewrite is original work and is still required.**
Rebasing it is not busywork that upstream made redundant.

**The throwaway worktree was removed and the aurora work re-verified intact
(4 files, 49 insertions, HEAD still `81f12f31`) after the test.**

## State at end of Phase 1

| | |
|---|---|
| `dusklight-main` | `c880d46fb5`, 0 behind, epoch 2, borealis in |
| `dusklight-main/extern/aurora` | `81f12f31` + 49 local insertions — **intentionally behind the pin** |
| the fork | untouched by all of the above |
| vanilla epoch-2 **build** | **NOT ATTEMPTED** — blocked on the aurora rebase decision |

### Which aurora target — the PIN, not "latest"

Asked and measured 2026-08-17. Three distinct positions:

| ref | commit | relation |
|---|---|---|
| worktree (where the local work sits) | `81f12f31` | 39 behind pin · **40** behind latest |
| **dusklight's pin** (recorded by `c880d46fb5`) | `cf3ffc98` | **the build target** |
| aurora `origin/main` | `0f15fb6` | **1 ahead of the pin** |

The single commit past the pin is `0f15fb6 Fix missing include <cstring>
(#240)` — one `+#include <cstring>` line in `lib/internal.hpp`, authored
**2026-08-16, the day after** dusklight's tip commit. It is unpinned because
upstream has not bumped to it, not because it was rejected.

**Rebase onto `cf3ffc98`, not `0f15fb6`:**

1. The pin is what upstream dusklight builds and CI validates. Past it, the
   submodule sits ahead of its gitlink — permanently dirty, and every resulting
   behaviour is ours to explain rather than upstream's.
2. **Phase-0 discipline: one variable at a time.** Crossing the ABI epoch is
   already the change under test. Bumping aurora past its pin in the same
   motion makes *"anything that moves after the merge is a merge artifact"*
   unusable, because we would have moved two things.
3. The gap is a portability include, not a feature. If MSVC needs it, it
   surfaces immediately as a compile error and is a one-line cherry-pick at
   that point. Not worth pre-solving.

**So the rebase brings the local work current with WHAT DUSKLIGHT EXPECTS, not
current with aurora.** Today those differ by one trivial commit.

## Aurora rebase — DONE 2026-08-17

Rebased `81f12f31 + 49 local insertions` onto the pin `cf3ffc98`.
Branch **`dusk-skipdraw-cf3ffc98`**, commit **`3eed7c2`**, in
`dusklight-main/extern/aurora`. **Worktree clean; the work is now in git, not
in a patch file.** (Local `user.name`/`user.email` were unset in the submodule
and were set **repo-locally** from the parent, not globally.)

Safety before the rebase: the patch backup was byte-compared against the live
diff (identical), **and** the work was `git stash`-ed as a second net. The stash
is still held.

### The two conflicts and how they were resolved

**`lib/gx/pipeline.cpp` — pure reformatting.** Upstream reflowed the `label`
`fmt::format` call. Took our guards with **upstream's formatting**, so future
diffs stay quiet. No semantic choice involved.

**`lib/gx/shader.cpp` — a real decision, and the naive resolution was wrong.**
The pre-rebase fix wrapped `a`/`b`/`c` in `tev_overflow_f32(...)` *inside*
`tev_alpha_op`. Upstream `4998dcc` moved that wrapping **to the caller**, where
it is applied **conditionally** via a new `tev_alpha_arg_is_normalized()`:

```cpp
const auto alpha_arg = [&](GXTevAlphaArg arg) {
  auto value = alpha_arg_reg(arg, idx, config, stage);
  if (tev_alpha_arg_is_normalized(arg, alphaNormalized)) { return value; }
  return fmt::format("tev_overflow_f32({})", value);
};
```

**Keeping our wrapping would have produced
`tev_overflow_f32(tev_overflow_f32(x))` on every non-normalized alpha arg** —
defeating the optimisation upstream had just landed, and adding redundant work
to every TEV stage. **The wrapping was deliberately dropped; the caller already
does it.** That is why the rebased diff is 48 insertions, not 49.

Two further details taken from measurement rather than preference:

- **`static` was dropped**, because upstream dropped it. Re-adding internal
  linkage on a function upstream may now declare elsewhere is the
  anon-namespace link trap — it compiles clean and fails at link, or silently
  binds the wrong symbol.
- **`DEFAULT_FATAL` moved to the head of the switch**, matching aurora's own
  idiom (`tev_op`, `color_arg_reg`, `alpha_arg_reg` all do this). The
  pre-rebase version had it trailing after a `break;`.
- `GX_TEV_COMP_A8_GT` **is** `GX_TEV_COMP_RGB8_GT` (both `14`, verified in
  `GXEnum.h`), so they cannot both be case labels. Retained as a comment.

### The bug is still upstream's, not ours to hand back yet

`tev_alpha_op` at `cf3ffc98` remains the unguarded
`tev_op(op, bias, scale, a, b, c, d, "0.0"sv)`. **Verified by reading the file
at that commit, not inferred.** The scalar-`select` rewrite is original and
still required. *(Possible upstream contribution — it is WW-free and fixes a
real WGSL validity bug. Not proposed; noted.)*

### The rebase compiled — and the compiler found one defect in it

Full vanilla build of `dusklight-main` at `c880d46fb5` with the rebased aurora:

| | |
|---|---|
| result | **1,521/1,521, linked, zero errors, zero warnings in the four touched files** |
| exe | `build/windows-msvc-relwithdebinfo/dusklight.exe` |
| size / mtime | 45,248,512 · 2026-08-17 20:02 |
| **sha256** | `9973054355d6a6eafc0d06d5ddaca90ab5f7a6d0e2effa225447e63187c923af` |
| link | 28,961 exports (2,429 data, 277 forwarded) from 1,416 objects |

**This is the first epoch-2 vanilla binary.**

The first attempt failed at step 88/1625:

```
lib/gx/pipeline.cpp(20): error C2065: 'Log': undeclared identifier
```

Upstream had deleted this TU's `static Module Log("aurora::gx")` when
`create_pipeline` stopped logging; the rebase added `Log.error()` diagnostics
without it. Restored as `constexpr Module Log{"aurora::gx"}` in an anonymous
namespace — the **current** idiom (`shader_info.cpp`, `attr_fmt.cpp`), not the
older `static Module` form the deleted line used. Committed separately as
`e9ec4a7` rather than folded into `3eed7c2`.

**`shader.cpp` compiled clean on the first attempt**, so the difficult
resolution — the one requiring a judgement about upstream's intent — was
correct, and the trivial-looking one was not. Worth remembering next time a
conflict is triaged by apparent difficulty.

**And this is the Phase-5 argument in miniature.** A compiler found in one
second a defect that the header-free `void*` model has no mechanism to catch —
the same class as the four-out-of-four authored-constant failures in
`HANDOFF-INTEGRATOR.md` §7.7. It is one data point, not a proof; it is
also exactly the data point the briefing predicted.

## Aurora branch — final state

`dusklight-main/extern/aurora`, branch **`dusk-skipdraw-cf3ffc98`**:

```
e9ec4a7  Restore the aurora::gx logger in pipeline.cpp
3eed7c2  Skip-draw on shader/pipeline validation failure; fix TEV alpha compare ops
cf3ffc9  (upstream pin)
```

53 insertions across 4 files. **Worktree clean. Nothing pushed.** The
pre-rebase stash is still held, and the `81f12f31` patches remain in
`aurora-patches/` — neither is needed now, but neither is discarded until the
build is confirmed good in play.

**The superproject will report `extern/aurora` as modified.** That is correct
and expected: the gitlink pins `cf3ffc98` and we deliberately carry two commits
past it. It is an accurate signal, not dirt to clean.

## STILL NOT DONE

- **The fork's aurora work-set (13 files / 995 insertions) has NOT been
  rebased or committed.** It is backed up as a patch only. Same hazard, larger
  body of work.
- **Nothing has been run.** The epoch-2 vanilla exe has been built, never
  launched. FPS band, assert battery and any plugin-load test are all open.

---

# THE EPOCH-2 VANILLA RUN — the plugin fails, and it is the guard working

User launched the epoch-2 build. `dusklight-20260817-200619.log`. **The run is
genuinely the new binary** — the log opens with `borealis::data` /
`borealis::log`, which did not exist before the fast-forward.

```
[INFO ] dusk::mods::loader] initializing 5 mod(s)...
[ERROR] dev.twilitrealm.shadow_mod] failed: Required service unavailable: dev.twilitrealm.dusklight.game@1 (2)
[ERROR] dev.twilitrealm.albw_meter] failed: Required service unavailable: dev.twilitrealm.dusklight.game@1 (2)
[ERROR] wwDonorDisc]              failed: Required service unavailable: dev.twilitrealm.dusklight.game@1 (2)
[INFO ] dusk::mods::loader] 2/5 mod(s) active
```

Each failing mod asks for `game@`**`1`**; the host offers **`2`**. That is the
epoch guard described in `game.h` doing exactly what it says: *reject
incompatible mods with a clear error instead of corrupting memory.*

## This is NOT a WW defect — upstream's own mod fails identically

**`shadow_mod` is dusklight's own demo mod, authored by `encounter`, sitting in
`dusklight-main`'s own build tree**, and it fails with the same message on the
same boot. So does `albw_meter`. **Three mods, three authors, one cause.**

### The discriminator is the game import, not staleness — verified

All three demo `.dusk` files are the same **Jul 22** vintage, yet two of them
loaded fine. Age therefore cannot be the discriminator. What separates them:

| mod | game headers | result |
|---|---|---|
| `ao_mod` | none | **active** |
| `template_mod` | none | **active** |
| `shadow_mod` | `#include "d/d_com_inf_game.h"` | **failed** |
| `albw_meter` | (game-coupled) | **failed** |
| `wwDonorDisc` | `FEATURES game` | **failed** |

**Any mod that touches game code needs rebuilding against epoch-2. Nothing
else does.** That is the entire blast radius, and it was predicted: *"update
effort is per ABI epoch, not per release."*

## What it takes to make the WW plugin epoch-2 — measured, and it is small

**The plugin includes ZERO game headers.** Its complete external surface is
nine SDK headers plus the C++ standard library:

```
mods/api.h · mods/hook.hpp · mods/meta.hpp · mods/service.hpp
mods/svc/{config,hook,log,overlay,ui}.h
```

It declares `game@1` **only** because `add_mod(... FEATURES game)` is required
by the hook macros, not because it consumes game types. **That is the
header-free `void*` model paying off exactly once** — there is no game-code
coupling to reconcile.

Delta on those nine headers, fork vs epoch-2 vanilla:

| header | changed lines |
|---|---|
| `mods/hook.hpp` | **195 — and it is now a DEPRECATION SHIM** |
| `mods/svc/hook.h` | 62 |
| `mods/meta.hpp` | 40 |
| `mods/svc/ui.h` | 34 |
| `mods/api.h` | 17 |
| `svc/config.h` · `svc/overlay.h` · `svc/log.h` | 14 · 14 · 13 |
| `mods/service.hpp` | 4 |

`mods/hook.hpp` at epoch 2 is three lines of `#warning "<mods/hook.hpp> is
deprecated; include <mods/svc/hook.hpp> instead"`. The replacement
`mods/svc/hook.hpp` **carries the same API** — same `DUSK_MOD_FEATURE_GAME`
gate, same `arg<T>()` / `arg_ref<T>()` helpers.

**So the migration starts as one include swap, and the compiler drives the
rest.** Nothing here requires merging upstream into the fork.

## The open architectural question — not mine to decide

To emit a `game@2` plugin, the same sources must build against **epoch-2 SDK
headers and `dusklight_imports.lib`**. Two ways, and they differ in more than
mechanics:

- **(A) Point the fork's `mods-src/ww_donor_disc` build at
  `dusklight-main/sdk`.** Smallest change. The fork stays 2,861 behind and is
  never merged, per the standing instruction. The plugin becomes the only thing
  in the fork built against current vanilla.
- **(B) Move the plugin out of the fork** into its own project that builds
  against vanilla's SDK. This is Phase 3/5 territory and it is what
  *plugin-on-unmodified-vanilla* actually means. Larger, and it is the
  direction the briefing argues for.

**(A) is a build-path change and violates nothing. (B) is a decision about
where the deliverable lives.** Recorded, not chosen.

---

# ROUTE (A) — THE PLUGIN BUILDS AND PACKAGES AT EPOCH 2

> User's sequencing, 2026-08-17: **"A, then B ultimately."**

## What was built

`mods-src/ww_donor_disc/standalone/CMakeLists.txt` — an **out-of-tree** project
building the fork's existing plugin sources against `dusklight-main`'s epoch-2
SDK. **The fork is not merged and not moved one commit.**

This uses the SDK's own documented entry point, which supports exactly this:

```cmake
add_subdirectory(<dusk>/sdk dusk-sdk EXCLUDE_FROM_ALL)
add_mod(my_mod FEATURES game SOURCES ... MOD_JSON mod.json)
```

`cmake/ModSDK.cmake` branches on `if (NOT TARGET dusklight)` specifically to
support out-of-tree mods, and on Windows takes the host import library from
`DUSK_GAME_EXE`. Pointed at our own epoch-2 `dusklight_imports.lib`, so the
plugin links against the exact binary it is built for rather than the
version-independent stub `ModSDK.cmake` would otherwise download.

**A is deliberately shaped as B.** Reaching B is a `git mv` of the sources next
to that CMakeLists plus dropping the `../` prefixes — not a rewrite.

**Result: `ww_donor_disc.dusk`, 163,246 bytes** (the failing epoch-1 artifact
was 70,394). Seven TUs compiled, linked, packaged.

## The one real API break — and the comment documenting it was the casualty

All seven TUs compiled first time. The link failed:

```
registry.cpp.obj : error LNK2005: "struct LogService const * const s_log"
                   already defined in main.cpp.obj
```

**Cause, one keyword in `sdk/include/mods/service.hpp:49`:**

```
epoch 1:  static const service_type* variable = nullptr;
epoch 2:         const service_type* variable = nullptr;
```

`IMPORT_SERVICE` went from internal to **external** linkage, and `s_log` is
imported in both `main.cpp` and `registry.cpp`. `docs/modding.md` states the
epoch-2 contract: *"A service must be imported in only one file (usually your
`mod.cpp`). Other files may simply use `svc_log`."*

**`registry.cpp` carried a comment that was correct when written and is now
exactly wrong** — it argued the imports must stay *because* `IMPORT_SERVICE`
*"expands to a static pointer… so it has internal linkage and cannot collide
across translation units"*, **citing `service.hpp:49` by line number.** The line
still exists. It no longer says that. `registry.h` carried the same claim.

Both were corrected **at the point of the misread**, old text left visible and
dated: a successor reading only the new text learns the rule; one who remembers
the old text learns that it moved.

## Both epochs still build — verified, not assumed

The fix is conditional, not a replacement, because **the fork's own in-tree
build is still epoch 1 and must stay buildable**:

```cpp
IMPORT_SERVICE(HookService, s_hook);
#if defined(__has_include) && __has_include(<mods/svc/hook.hpp>)
extern const LogService* s_log;  // epoch 2+: imported once, in main.cpp
#else
IMPORT_SERVICE(LogService, s_log);
#endif
```

**Discriminator is `<mods/svc/hook.hpp>`** — added by the 39 commits, absent
from the fork's SDK. **`MOD_ABI_VERSION` cannot be used: it is `1u` in BOTH.**
The *mod* ABI did not bump; the *game* ABI did. **Anything keying on
`MOD_ABI_VERSION` to detect this epoch takes the wrong branch silently.**

**The fork's in-tree epoch-1 build was then re-run and PASSED** — build
`FB887B64605E4D89B63356D2513450E6-868`, no errors. The edit is two-way in fact,
not just in intent.

## Left alone deliberately

- **`ww_message.cpp` imports `LogService` a third time as `s_msgLog`.** Unique
  name, so it links at both epochs — but under the epoch-2 contract it is a
  redundant import record for a service already imported in `main.cpp`. **Not
  the error, not fixed, flagged.** Widening a link fix into a rename across a TU
  is how a small change acquires unrelated risk.
- **`<mods/hook.hpp>` still emits its deprecation warning** on every epoch-2
  compile. The swap to `<mods/svc/hook.hpp>` belongs with route (B), not
  smuggled into a link fix.

## ⚠️ STAGING HAZARD — two hosts at two epochs share one mods folder

`%APPDATA%/TwilitRealm/Dusklight/mods/` is read by **both** exes, and they are
now at **different ABI epochs**:

| host | needs |
|---|---|
| the fork's exe (epoch 1) | the epoch-1 `ww_donor_disc.dusk` |
| `dusklight-main` (epoch 2) | the epoch-2 one |

**Same mod id, same folder — they cannot both live there.** The epoch-2 artifact
was therefore **left in its own build output** rather than staged over the
epoch-1 copy: staging it would silently break the fork's exe in order to fix
vanilla's. **That trade is the user's call, not a lane's.**

## NOT VERIFIED

**The epoch-2 plugin has been built and packaged. It has never been loaded.**
`game@2` is carried in the DLL's modmeta import records, not in `mod.json`, so
the only real confirmation is a boot. Until then this is *"compiles and
packages"*, which `HANDOFF-INTEGRATOR.md` §7.4 is explicit is not the same as
runs.

## STAGED 2026-08-17 20:18 — and the boot before it tested the WRONG FILE

The 20:17:19 boot failed identically to the 20:06 one. **It was not a new
failure: the epoch-2 artifact had never been staged.** The log names the file
it actually loaded —

```
[INFO | wwDonorDisc] found '...' (~\AppData\Roaming\TwilitRealm\Dusklight\mods\ww_donor_disc.dusk)
```

— which was still the **70,394-byte epoch-1** build from 08:58. The 163,246-byte
epoch-2 build was sitting in its own output directory.

**That was mine.** `HANDOFF-INTEGRATOR.md` §1 puts staging in this lane. I
identified the hazard, wrote it up, and then left the action to the user in a
two-step form. **A hazard that is documented but not acted on still costs the
boot.** Naming it because "I flagged it" reads like diligence and is not.

### What is staged now

| | |
|---|---|
| `%APPDATA%/…/mods/ww_donor_disc.dusk` | **PARKED** as `ww_donor_disc.dusk.epoch1-parked` — renamed, **not deleted**; the loader only reads `.dusk` |
| `dusklight-main/build/…/mods/ww_donor_disc.dusk` | **epoch-2, 163,246 B**, sha `273a01e6ce32756142f62042b9bc655303ecc253b527754514d7e5d475e1dec4` (hash-matched against the build output) |

Placed next to the vanilla exe rather than in `%APPDATA%` **on purpose**: that
folder is shared by both hosts, and one mod id cannot serve two ABI epochs from
it.

**CONSEQUENCE, stated because it is a regression for the other host: the fork's
epoch-1 exe now has NO WW plugin.** Restore with one rename:

```
mv ww_donor_disc.dusk.epoch1-parked ww_donor_disc.dusk
```

### `game@2` verified by construction before spending another boot

1. `FEATURES game` links `dusklight_mod_feature_game`, which compiles
   `sdk/src/game_feature.cpp`.
2. That file is two lines: `IMPORT_SERVICE(GameService, svc_game);`
3. `IMPORT_SERVICE` embeds `ServiceTraits<GameService>::major_version`.
4. The build compiled that TU **from `dusklight-main/sdk/src`** (in the build
   output), and that tree's `game.h` defines `GAME_SERVICE_MAJOR 2u`.

It also linked, which independently proves the plugin does not import
`GameService` itself and therefore does not collide with the feature TU.

**Still not a load.** The chain says `game@2`; only a boot says it runs.

---

# ✅ THE PLUGIN LOADS AT EPOCH 2 — 2026-08-17 20:20

`dusklight-20260817-202016.log`, host `c880d46fb5`:

```
[INFO | wwDonorDisc] activating mod
[INFO | wwDonorDisc] using host backend.extraIsoPath (row-14 generic field)
[INFO | wwDonorDisc] attached '…Wind Waker (USA).iso' — GZLE01, roster verdict
                     ON-ROSTER (sys/boot.bin + sys/fst.bin match), 2213 files in FST
[INFO | wwDonorDisc] arc-name alias: 'res/Stage/sea/Room0.arc' ALSO served as
                     '/res/Stage/sea/R00_00.arc' (bytes verbatim)
```

Donor disc attached, roster verified, arcs serving, DZR members parsed, the
`color.bmc` donor-palette self-proof firing, registry walking BMDL slots.
**Route (A) is confirmed by a boot, not by a build.**

## The save scare — DATA IS INTACT. Measured, not reassured.

User reported only 2 of 3 save files appearing. **Nothing was lost.**

- `01-GZ2E-gczelda2.gci`, 32,832 B, **mtime 2026-06-06** — today's boots
  **read** it and never wrote it.
- **All three slots are populated.** Parsed at `QUEST_LOG_SIZE = 0xA94`:
  slot 0 **2708/2708** non-zero · slot 1 **2542/2708** · slot 2 **2705/2708**.
- Byte-identical backup taken **before** any further boot:
  `Documents/dusklight-backups/save-protect-20260817-2025/`, sha
  `b55662b8c6e4bcbddf2d3fd72da8f5b25ae6a29e6d55d93c6c51bcdb77dd97a9`.

**The dangerous mechanism was ruled out by measurement:** a stride mismatch
would misparse slots and corrupt on write. It does not exist —
`QUEST_LOG_SIZE = 0xA94` and `CARD_FILE_SIZE = 0x8000` in **both** trees, and
`dSv_save_c` differs only by two accessor methods (`getReserve`), **no data
members**. The card layout is identical.

**The fork also does not extend the card image.** `d_save.cpp:1917` states it:
the donor event-flag block is a **sidecar**, kept out of the card *"no slot
slack, and the checksum covers in-slot bytes."*

So this is a **display/validation difference on read**, not data loss. **Cause
not yet identified — that is an open question, not a closed one.**

**⚠️ DO NOT SAVE IN VANILLA until it is identified.** Reading is proven
harmless; a write while a slot is hidden is the operation that could make the
loss real. The backup exists precisely because that risk is not zero.

## NEXT PLUGIN-SIDE WORK — Phase 4a, and this boot named its own targets

The log carries the answer at lines 304-308. **All five host probes miss:**

```
{"ev":"host_probe","i":0,"sym":"dExtWwSave_isWwHostStage",     "hit":0}
{"ev":"host_probe","i":1,"sym":"dExtWwSave_registerWwStage",   "hit":0}
{"ev":"host_probe","i":2,"sym":"dExtWwSave_isDeclaredWwStage", "hit":0}
{"ev":"host_probe","i":3,"sym":"dExtWwSave_isWwContentActive", "hit":0}
{"ev":"host_probe","i":4,"sym":"dExtWwSave_refuseNativeWrite", "hit":0}
```

This is the **ABSENT-unhookable seam**, no longer a prediction — it is measured
on a live vanilla boot.

**And it is bigger than "one blocker".** `dExtWwSave_isWwHostStage` is not a
seam, it is a **predicate consumed at 12+ call sites across five receiver
TUs** — `d_a_alink.cpp` (×3), `d_a_bg.cpp` (×5), `d_a_mirror.cpp`,
`d_a_swhit0.cpp`, `d_bg_s_acch.cpp`. On vanilla every one of those gates is
simply absent, so the WW-conditional behaviour in those files does not happen.

Foundry's parting instruction applies exactly, and **nobody has run it yet**:
*"ABSENT-unhookable is a property of the SYMBOL, not the EFFECT. The effect is
'the receiver knows this stage is WW-hosted', and vanilla HAS a stage-load
path. Hook the CONSUMER."*

**What is new since that was written:** `StageService` and `SaveService` landed
in the 39 commits. These five symbols are *stage identity* and *save
gating* — precisely those two domains. Phase 4a and Phase 3 have converged.

**Save-relevant, and worth naming after today:** `dExtWwSave_refuseNativeWrite`
at `hit:0` means **the plugin currently has no way to refuse a native save
write.** That is not today's cause — nothing was written — but it is a latent
hazard sitting in the same area as today's scare.
