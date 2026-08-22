# AURORA PATCH LEDGER — patch file FIRST, apply SECOND

era: plugin-delivery
<!-- era rationale: release-hygiene protocol for the stock-aurora backend | Foundry, from a direct user order 2026-08-22 -->

## The order (user, 2026-08-22, verbatim intent)

> The code goes FIRST into a patcher, tracked by the same means we used for
> the fork code (or systematically similar). They go INTO the patch
> file/system/whatever format FIRST. That patcher is then APPLIED to aurora
> — making sure to not copy over with redundant code if aurora later accepts
> the patcher code upstream.

**Modeled on `mods-src/ww_donor_disc/FORK-STRIP-LEDGER.md`, direction
reversed.** Fork-strip is row-first-then-*delete*, gated on the plugin
*proving* it owns the behavior. This is row-first-then-*apply*, gated on a
real `.patch` file that verifiably reproduces the change from a named pin —
plus a check the fork-strip ledger has no analogue for: **has upstream
already absorbed this, making the local patch redundant?**

## Standing directive: hook OUT of the patcher wherever possible (user, 2026-08-22)

> If we can find ways to hook/refine any portions OUT of the tracker and
> into the plugin, that is the intended directive.

**CLARIFIED 2026-08-22 — the directive is ALL-OR-NOTHING per behaviour:**

> *"We'll identify hook out candidates, but standing directive is IF we
> can't hook all at once → patch as whole."*

So the hook-out work list is a **candidate list, not a partial-migration
plan.** If the 95 hook-reachable lines can be hooked out *and* the remaining
900 can too, the patch disappears entirely. If they cannot **all** move, the
behaviour ships as **one whole patch** — not 95 lines hooked and 900 patched.
That is the same "no weird mid-areas" coherence rule as the
patched-not-hooked ruling, applied one level down: **a single behaviour is
never split across both mechanisms**, because then neither the patch nor the
hook is the whole story and a reader cannot tell which is authoritative.

**Practical consequence:** measuring 95 lines as hook-reachable does NOT
authorize hooking those 95. It identifies a candidate. The decision point is
whether the *whole* 995 can move — and today it demonstrably cannot (900
lines sit in namespaces exposing zero symbols). **Therefore, on today's
measurement, this behaviour is a whole-patch case.**

**The hookability boundary measured for AURORA-PATCH-0002 (below) is not
just descriptive — it is the WORK LIST.** Every line at-or-above the
symbol-visible line is a candidate to be pulled OUT of any future patch and
implemented as a by-name hook in the plugin instead, which needs no entry in
this ledger at all (it ships in `ww_donor_disc`, plugin-side, ordinary
code). **A patch entry's mass should only ever be the lines that are
genuinely unreachable by hook** — not a default first answer.

**Classification complete, TWICE-CORRECTED 2026-08-22:** first pass (Foundry)
undercounted by 285 of a wrong 3,144 total, caught by History/Bridge; that
whole 3,144 total was then itself retracted by Housing/Engine — it measured
the fork's total divergence from upstream, not the WW work-set (see the
correction banner in `TRACKED-lwood-candidate-9files.md` for the full
account). **Correct, re-verified split over the real 995-line range: 95
lines / 3 files hook-reachable (~9.5%), 900 / 10 files patch-only (~90.5%),
0 unclassified.** The hook-out work list is unchanged in IDENTITY —
`GXGeometry.cpp`, `GXDispList.cpp`, `GXVert.cpp` — full detail in
`TRACKED-lwood-candidate-9files.md`.

This also means the boundary is worth re-measuring as the plugin's hook
surface or aurora's exported symbols change, not treated as fixed once
measured.

**Known gap, flagged rather than silently carried: neither `verify` nor
`redundancy` in `aurora_patch_check.py` checks that a patch actually
BUILDS.** Byte-identical extraction from a source branch is not the same
claim as "compiles against the current pin" — the user named this
explicitly: *"Patcher right now likely will break the build and will need
some refining."* A patch that byte-verifies can still fail to compile if the
pin it targets has drifted from the branch it was extracted from in ways the
nine files' own diff doesn't capture (headers, adjacent declarations, ABI).
**No `APPLIED` status should be read as `BUILDS` until a build step exists
and has run.** Adding one is on Foundry's plate.

## Reversibility: strip the patch back OUT (user order, 2026-08-22)

> *"since we're tracking exactly what we're putting INTO a user build, it
> would be helpful to be able to strip it OUT as well so we don't have to
> unpatch/download a stock/other every time."*

**`aurora_patch_check.py unapply <patch> <repo> [--base REF] [--write]`.**
Tracking what goes in is only half a system if taking it back out means
re-cloning. This closes that: a patched aurora checkout can be returned to
stock content in place.

**It is the only command in the instrument that writes to a real repo**, so
it carries guards the others get for free from scratch-clone isolation:
**dry-run by default** (must pass `--write`), **refuses on a dirty tree**
(an unapply would tangle uncommitted work with the reversal, unrecoverably),
**verifies the patch is actually applied first** (no half-reversed trees),
and **`--base` hash-verifies every touched file afterward** so "it reversed"
is proven, not assumed — the same standard `verify` holds the forward
direction to.

Control-proven across all seven guards, and dry-run-confirmed against the
live `dusklight-main/extern/aurora` (reports the 4 files would reverse
cleanly; changed nothing).

## Why a `.patch` file and not just a description

A prose description of "what changed" is not reproducible and cannot be
diffed against a moved pin. **The artifact is the diff itself** — `git diff
<pin>..<source>` over the exact files, stored as a real unified-diff file in
this directory, `git apply --check`-clean against the stated pin. Anything
less is a claim, not a tracked patch.

## Protocol

1. **Patch file FIRST.** Before any code lands in `dusklight-main`'s
   `extern/aurora`, its diff is captured as `AURORA-PATCH-NNNN-<slug>.patch`
   in this directory — generated from a **scratch clone**, never the live
   submodule (see the near-miss below; that rule exists because of it).
2. **Ledger row FIRST, apply SECOND.** The row (below) records the patch
   file, its pin, its source, and a **byte-verification**: apply the patch
   to a scratch checkout of the pin and hash-compare the result against the
   intended target tree, file by file. `NEEDS-VERIFY` blocks `APPLIED`.
3. **Apply as its own step**, distinct from any other change to
   `dusklight-main`, so the applied diff is reviewable alone.
4. **Redundancy check — the part this order adds beyond the fork-strip
   model.** Before (re-)applying, and periodically after, fetch upstream
   `encounter/aurora` and check whether the patch's hunks are now satisfied
   by upstream content at its own tip. If yes: **do not apply** (or if
   already applied locally, mark `REDUNDANT-UPSTREAM` and plan removal —
   never leave a duplicate of code upstream now carries natively). This is
   what `aurora_patch_check.py redundancy` automates.
4b. **Compat check (user order, 2026-08-22): "it must ALSO check the user's
   build... as much compatibility with different dusklight versions as
   possible."** Redundancy asks whether upstream has absorbed the fix;
   compat asks a different question — for a user on a SPECIFIC released
   dusklight version (not the unreleased tip this repo tracks), does the
   patch apply, does that release already carry it, or has that release's
   aurora diverged too far to say either. Run against every tagged release,
   not just the current tip. `aurora_patch_check.py compat` automates it,
   resolving each release's `extern/aurora` pin straight from the git
   superproject tree.
4c. **Fork-parent check (user order, 2026-08-22): "it HAS to be compatible
   with the fork mod since that is its parent."** The fork
   (`C:\Users\xxxxx\Documents\dusklight`) has its own `extern/aurora`, often
   far ahead of any tagged dusklight-main release — check the patch against
   the fork's LIVE checked-out tip specifically (not its committed gitlink,
   which lags the same way `dusklight-main`'s does), via
   `aurora_patch_check.py redundancy <patch> <fork's extern/aurora path>
   --ref <fork tip sha>`. **A `DIVERGED` result here REQUIRES manual line
   review before being read as a real conflict** — the fork's aurora work
   can carry the same fix in substance while failing a literal patch-apply
   because it sits inside a larger rewrite (renamed locals, shifted
   context). Automated tooling stops at DIVERGED by design; a human reads
   the diff from there. See AURORA-PATCH-0001's entry for a worked example.
5. **Removal condition stated at authorship time**, per the standing
   doctrine already recorded in `ww-plugin-outset.md`: *"write its removal
   condition the day it is built: delete when upstream lands the aurora
   work."* Every row states what upstream landing THIS specific patch would
   look like, so the redundancy check has a concrete thing to look for.

## Row format

| field | meaning |
|---|---|
| **id** | `AURORA-PATCH-NNNN` |
| **patch file** | path in this directory |
| **pin** | the vanilla `encounter/aurora` commit the diff applies to |
| **source** | where the change came from (a fork branch/commit, hand-authored, etc.) |
| **files** | count + total ins/del |
| **byte-verify** | scratch-apply hash-compare result, per file |
| **redundancy check** | last upstream-fetch date + result |
| **compat check** | verdict per tagged dusklight release (COMPATIBLE / ALREADY-HAS-IT / INCOMPATIBLE / PIN-UNRESOLVED) — required, not optional |
| **removal condition** | what upstream landing this looks like |
| **status** | `NEEDS-VERIFY` → `CAPTURED` (patch file byte-verified, application undecided) → `APPLIED` → `REDUNDANT-UPSTREAM` (removed) |

## Entries

### AURORA-PATCH-0001 — skip-draw + TEV alpha compare + gx logger

- **Patch file:** `AURORA-PATCH-0001-skipdraw-tev-logger.patch`
- **Pin:** `cf3ffc9886` (upstream `encounter/aurora`)
- **Source:** `dusklight-main`'s `extern/aurora` HEAD `e9ec4a701f` = pin + two
  local commits (`3eed7c2` skip-draw on shader/pipeline validation failure +
  TEV alpha compare fix; `e9ec4a7` restore the `aurora::gx` logger). Accepted
  by user ruling 2026-08-21 ("as-is for now... envelop into the plugin
  eventually").
- **Files:** 4 — `lib/gfx/pipeline_cache.cpp`, `lib/gx/pipeline.cpp`,
  `lib/gx/shader.cpp`, `lib/webgpu/gpu.cpp`. **+53 / -5.**
- **Byte-verify (2026-08-22, Foundry):** patch generated from a **scratch
  clone** of `extern/aurora`, applied to a **separate scratch checkout** of
  the pin, and hash-compared file-by-file against the real `e9ec4a7` tree —
  **all 4 files `git hash-object` IDENTICAL.** The patch file reproduces the
  accepted change exactly, reproducibly, from the stated pin.
- **Redundancy check (2026-08-22, Foundry, `aurora_patch_check.py
  redundancy` against a fresh fetch of `origin/main` from the submodule's
  own configured remote):** **`NEEDED`** — forward-apply against upstream's
  current tip succeeds cleanly; upstream has not absorbed this. Not
  redundant today. Re-run before any future decision that assumes this
  patch is still necessary — upstream moves.
- **Removal condition:** upstream `encounter/aurora` ships equivalent
  skip-draw-on-failed-pipeline-creation behavior AND the TEV alpha compare
  fix AND does not regress the `aurora::gx` logger. All three, not any one —
  a partial upstream fix does not retire this patch.
- **Compat check (2026-08-22, Foundry — user order: check the USER'S build,
  not just upstream, for compatibility across dusklight versions):**
  `aurora_patch_check.py compat` against all 9 tagged `dusklight-main`
  releases (`v1.0.0` through `v1.4.1`), resolving each release's
  `extern/aurora` gitlink pin directly from the superproject tree (no
  submodule checkout needed) — **7 genuinely distinct aurora pins across the
  9 tags, confirming this is a real compatibility axis, not a formality.**
  **RESULT: `INCOMPATIBLE` with all 9.** Neither direction applies cleanly
  against any tagged release's aurora pin — this patch was built against
  `cf3ffc9886`, which is newer than every tagged release's pin. **A user on
  any released dusklight version cannot receive this patch as-is; it is
  compatible only with builds tracking `dusklight-main`'s unreleased tip.**
  This is not a defect in the patch — it is the honest current compatibility
  surface, and it argues for the standing directive above (hook out what can
  be hooked, since a hook degrades more gracefully across versions than a
  line-numbered patch does) and for re-running this check at every future
  dusklight release.
- **Fork-parent check (2026-08-22, Foundry — user order: "it HAS to be
  compatible with the fork mod since that is its parent"). CORRECTED same
  day, on History/Bridge's catch:** the fork
  (`C:\Users\xxxxx\Documents\dusklight`, this repo) has its OWN `extern/aurora`
  gitlink — committed pin `81f12f31`, uncommitted local checkout `fc2e1cd6f9`
  on branch `ww-fork-aurora-81f12f31`. **First pass checked the LIVE
  checkout and got `ALREADY-HAS-IT`. That was checking the wrong baseline —
  `fc2e1cd6f9` is uncommitted local state, not what the fork as distributed
  ships; the shipping case the user's order names is the COMMITTED pin,
  `81f12f31`.** History/Bridge caught this directly: *"the fork SHIPS
  81f12f31, not fc2e1cd6f9, and the patched regions differ between them...
  the verdict does not transfer."* Re-run against `81f12f31` specifically:
  **`DIVERGED`, and manual review this time finds the fix GENUINELY ABSENT**
  — zero hits for every one of the four regions' signatures
  (`static_cast<bool>(pipeline)`, `CreateRenderPipeline failed`, the TEV
  `select(0.0,...)` expressions, `shaderOrPipeline`) at `81f12f31`, checked
  in an isolated scratch checkout. The `DIVERGED` (rather than a clean
  `NEEDED`) is because these same four files independently changed between
  the patch's own base `cf3ffc9886` and `81f12f31` (`pipeline_cache.cpp`
  +6/-8, `pipeline.cpp` +8/-15, `shader.cpp` +95/-171, `gpu.cpp` +48/-54) —
  real, unrelated drift, not the fix arriving early. **Corrected verdict:
  NEEDED at the shipped pin, but not cleanly patch-appliable there — a
  manual/rebased application would be required if this patch is ever taken
  to the fork specifically.** The fix DOES land, in substance, one commit
  later at `fc2e1cd6f9` (tagged `№46 F2`, matching the earlier finding) —
  but that commit is local-only and uncommitted (see AURORA-PATCH-0002's
  loss-protection note). **Lesson kept, not discarded: a `DIVERGED` verdict
  requires the manual read — the read was done correctly the second time,
  and was drawn from the wrong ref the first time. Which ref is "the fork"
  matters as much as reading past the exit code.**
- **Status: `APPLIED`, verified four ways** — byte-identical from the pin,
  confirmed `NEEDED` (not redundant) against live upstream, confirmed
  `INCOMPATIBLE` with every tagged dusklight-main release (compatible only
  with `dusklight-main` tip), confirmed `NEEDED` (not present) against the
  fork's own shipped pin `81f12f31` — present only in an uncommitted local
  commit one step ahead of what the fork actually ships. Already live in
  `dusklight-main` since before this ledger existed — retroactive seed, same
  treatment the fork-strip ledger
  gave its first entries.

### AURORA-PATCH-0002 — WW aurora work-set (13 files, +995/-27)

**CORRECTED 2026-08-22.** The heading originally read "lwood candidate (9
files, +3,144/-958)" — that measured `cf3ffc9886..fc2e1cd6f9` (upstream pin
to fork tip), which is NOT this patch's size; it is the fork's total
divergence from upstream, an unrelated, much larger number. Housing/Engine
retracted it; the correct range is `81f12f31..fc2e1cd6f9` (the fork's OWN
committed baseline to its one-commit WW work), **13 files, +995/-27** — full
detail and per-file table in `TRACKED-lwood-candidate-9files.md`.

- **Patch file:** `fork-aurora-81f12f31.patch` — pre-existing, git-tracked
  since 2026-08-17 by Integrator; discovered to already cover this exact
  range when Housing/Engine's loss-protection alarm led History/Bridge to
  reverse-apply-test it (clean, against `fc2e1cd6f9`). A duplicate capture
  Foundry wrote independently the same night, `AURORA-CAPTURE-
  ww-workset-995.patch`, was removed once found content-identical — one
  canonical file, not two. Kept its filename convention (not renamed to
  `CAPTURE`,
  not `PATCH-0002`, because it is loss-protection insurance extracted
  2026-08-22 on Housing/Engine's flag — **not** a signal that application is
  decided; renamed into the `AURORA-PATCH-NNNN` sequence only if/when it is).
- **Base:** `81f12f31` (the fork's committed `extern/aurora` gitlink).
- **Source:** the WW work-set commit `fc2e1cd6f9`, local-only and
  uncommitted on this one machine (see loss-protection note below).
- **Byte-verify (2026-08-22, Foundry):** applied to a scratch checkout of
  `81f12f31`, hash-compared against `fc2e1cd6f9` — **all 13 files MATCH.**
- **Application status: UNDECIDED.** Capturing the patch file is not
  applying it. Per the protocol, if application proceeds: this capture
  already satisfies "patch file first" — the remaining steps are a ledger
  row proper (not this placeholder), then application as its own commit.
- **Status: `CAPTURED`** (patch file exists, byte-verified, not applied
  anywhere — the correct in-between state the row-first protocol is
  designed to produce, distinct from `NEEDS-VERIFY` and `APPLIED`).

## Instrument: `aurora_patch_check.py`

Automates the byte-verify and redundancy-check steps so they are not
hand-run once and trusted forever. See `tools/foundry/aurora_patch_check.py`.

## Near-miss this ledger's protocol exists to prevent

Building AURORA-PATCH-0001, Foundry tested `git apply --check` **against
the live `dusklight-main` submodule** rather than a scratch clone — a
`git checkout <pin> -- .` + `git stash` sequence that briefly left the real
submodule's working tree in a mutated state (matching the pin instead of
HEAD) before being caught and reverted. No commit was made and the tree was
restored clean (verified: `e9ec4a7`, 0 dirty files, the pre-existing
unrelated stash from 08-17 untouched) — but it is exactly the accident
protocol step 1's "never the live submodule" rule exists to prevent, and it
happened while writing the rule. Recorded rather than smoothed over.
