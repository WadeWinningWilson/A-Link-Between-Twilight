# Aurora lwood-fix candidate — tracked, NOT applied, NOT ruled on

era: plugin-delivery
<!-- era rationale: reproducibility artifact for a pending user ruling | Foundry, routed by History/Bridge CALLS row 606; MAJOR CORRECTION 2026-08-22 -->

**Status: TRACKED ONLY. This is not the accepted patch. Nothing here is
applied to `dusklight-main`. Do not build against this file without a user
ruling — see "The ruling this triggers" below.**

## ⚠ CORRECTION 2026-08-22 — the file's own headline number was wrong by 3.16×

**Filename says "9files"; the real candidate is 13 files, +995/-27, not
"3,144 lines / 9 files."** The +3,144 figure below (kept, struck through
where it appeared, for the record — never silently deleted) measured
`cf3ffc9886..fc2e1cd6f9`: **upstream's pin to the fork's tip**, which folds
in every line the fork's aurora has diverged from `encounter/aurora` on
*independent of the WW work* — a different lineage entirely, not the WW
patch's size. **Housing/Engine caught it, retracted it, and I independently
re-derived the correct range myself before touching this file** (this
estate's standing rule: verify before trusting a correction too, not just an
original claim). The real WW work-set is **exactly one commit**,
`81f12f31..fc2e1cd6f9`, and its diffstat is below, re-verified byte-for-byte.

**Everything downstream of the old number is corrected in this pass**: the
file table, the hookability split (History/Bridge's 124/3,020 also inherited
the wrong baseline and is corrected here to 95/900), and the extraction
framing. The "141 files / +7,484 deletes-most-of-tests/" divergence was
NEVER the candidate — it was proof the wrong baseline had been used, and
reading it as "extraction is hard" was the error, not a finding.

## Why this file exists

User question, 2026-08-21/22: *"are we adding more code to stock aurora?"*
History/Bridge measured the answer and routed the exact numbers here per the
user's instruction — documented and tracked so a patch reproduces cleanly.

## Where stock aurora stands today (unchanged, verified)

`dusklight-main/extern/aurora`: HEAD `e9ec4a701f`, **worktree clean (0 dirty
files)**, exactly two commits beyond the upstream pin `cf3ffc9886`:

| commit | subject |
|---|---|
| `3eed7c2` | Skip-draw on shader/pipeline validation failure; fix TEV alpha compare ops |
| `e9ec4a7` | Restore the `aurora::gx` logger in `pipeline.cpp` |

**Diffstat vs pin: 4 files, +53 / -5.** This is the patch the user ACCEPTED
on 2026-08-21 ("a patch will work just as cleanly until we decide to move
it"). It has not grown since. This is tracked in full as **AURORA-PATCH-0001**
in `AURORA-PATCH-LEDGER.md`.

## The candidate this file tracks — CORRECTED

Source: fork's `extern/aurora`, branch `ww-fork-aurora-81f12f31`. **The real
WW work is the single commit `fc2e1cd6f9` sitting directly on
`81f12f31`** (`git log --oneline 81f12f31..fc2e1cd6f9` returns exactly one
line: *"WW fork aurora work-set: display list, FIFO, command processor,
skip-draw"*). Measured against **that base**, not the upstream pin —
re-verified 2026-08-22, numstat exact:

| file | +ins | -del |
|---|---:|---:|
| `lib/gx/command_processor.cpp` | 392 | 7 |
| `lib/gx/gx.cpp` | 226 | 0 |
| `lib/gx/fifo.cpp` | 90 | 0 |
| `lib/dolphin/gx/GXGeometry.cpp` | 85 | 0 |
| `lib/gx/shader_info.cpp` | 68 | 12 |
| `lib/gx/shader.cpp` | 48 | 4 |
| `lib/gx/fifo.hpp` | 45 | 0 |
| `lib/webgpu/gpu.cpp` | 11 | 2 |
| `lib/gx/pipeline.cpp` | 11 | 1 |
| `lib/dolphin/gx/GXDispList.cpp` | 9 | 0 |
| `include/dolphin/gx/GXAurora.h` | 7 | 0 |
| `lib/gfx/pipeline_cache.cpp` | 2 | 1 |
| `lib/dolphin/gx/GXVert.cpp` | 1 | 0 |
| **TOTAL (13 files)** | **995** | **27** |

**~19× the accepted patch's line count** (995 vs 53) — not 60×. Still a real
patch, not a small one; just not the number that was published first.

**Four of these 13 files overlap with AURORA-PATCH-0001's own territory**
(`pipeline_cache.cpp`, `pipeline.cpp`, `shader.cpp`, `gpu.cpp`) — the WW
work-set's single commit folds in a broader version of the same fix
AURORA-PATCH-0001 carries, tagged internally `№46 F2` (see the ledger's
fork-parent entry for the full finding: absent at the shipped pin
`81f12f31`, present once this commit lands).

## ~~Extraction, not merge~~ — RETRACTED, this section measured the wrong thing

The struck claim below was true of `cf3ffc9886..fc2e1cd6f9` (141 files,
mostly independent divergence, unrelated to the WW work) and is **not a
property of the real candidate**, which is one clean commit on `81f12f31`
and applies to that base without conflict (self-evidently — it's a direct
child commit). Extracting it is not an "extraction problem"; it is
`git diff 81f12f31..fc2e1cd6f9`, already done — see
`fork-aurora-81f12f31.patch` in this directory, byte-verified against
`fc2e1cd6f9` file-for-file. *(A duplicate capture written by Foundry earlier
tonight, `AURORA-CAPTURE-ww-workset-995.patch`, was removed once this
pre-existing, already-tracked file was found to be content-identical.)*

~~The candidate branch diverges from the pin far beyond these nine files:
141 files, +7,484 / -12,537, and it deletes most of tests/. Merging the
branch would drag a different aurora lineage into the stock backend.~~

## Hookability boundary — RECOMPUTED against the real 995-line range

The method stands (measured 2026-08-22, Housing/Engine + History/Bridge;
independently re-verified against source by Foundry both times); the
**numbers are recomputed** because the file set changed (13 real files, not
9 wrong ones — three of the original nine, `command_processor.cpp`,
`gx.cpp`, `shader_info.cpp`, remain; the wrong inflated line counts are
replaced with the real ones; `pipeline_cache.cpp`/`pipeline.cpp`/`shader.cpp`
/`gpu.cpp` are newly included).

**A — the Dolphin GX API is hookable**, **B — aurora's internal
FIFO/command-processor/`aurora::gfx`/`aurora::webgpu` namespaces expose ZERO
mangled symbols**, **C — the display-list state machine's entry points sit
entirely on the hookable side** — all unchanged from the original
measurement (namespace/symbol facts, not line counts).

**D — the real 995 lines split against that boundary, verified 2026-08-22:**

| position | files | lines | share |
|---|---|---:|---:|
| **HOOK-REACHABLE** | `GXGeometry.cpp` +85, `GXDispList.cpp` +9, `GXVert.cpp` +1 | **95** | ~9.5% |
| **PATCH-ONLY** | `command_processor.cpp` +392, `gx.cpp` +226, `shader_info.cpp` +68, `shader.cpp` +48, `fifo.cpp` +90, `fifo.hpp` +45, `gpu.cpp` +11, `pipeline.cpp` +11, `GXAurora.h` +7, `pipeline_cache.cpp` +2 | **900** | ~90.5% |

**95 + 900 = 995 — reconciles.** `pipeline_cache.cpp` (`namespace
aurora::gfx`), `pipeline.cpp`/`shader.cpp`/`shader_info.cpp`/`gx.cpp`
(`namespace aurora::gx`), `gpu.cpp` (`aurora::gx`/`aurora::gfx`/
`aurora::webgpu`), `command_processor.cpp`/`fifo.cpp`/`fifo.hpp`
(`namespace aurora::gx::fifo`) — all confirmed internal namespaces, same
class already measured at zero mangled symbols. `GXAurora.h` is a header:
patch-only by nature.

**The hook-out work list, per the standing directive, is unchanged in
IDENTITY (same three files) but is now correctly ~9.5% of a smaller total:
`GXGeometry.cpp`, `GXDispList.cpp`, `GXVert.cpp` — 95 lines.** The remaining
900 (~90.5%) has no hook path by construction.

**Honest read, corrected:** ~90% patch-only — still dominant, still the
honest headline, but the earlier "96%, full stop" language overstated a
number that was itself built on the wrong base. The proportion barely
moved; the total did, by 3.16×.

## The ruling (user, 2026-08-22)

**PATCHED, not hooked.** Verbatim: *"We either go hooked/or patched for
aurora. I favor patched so we don't get into weird mid-areas."* The
rationale is coherence — excluding a half-hooked/half-patched split for ONE
behaviour, not excluding hooks generally; the plugin's existing by-name
hooks into the game are unaffected.

**What this does NOT settle, and is still open:** whether the candidate is
applied at all. "Patched not hooked" names the mechanism *if* it proceeds —
it is not approval, and now specifically **not approval of 995 lines**,
which is the corrected cost figure the user should weigh, not the retracted
3,144. The 2026-08-21 end-state (stock backend + one `.dusk`) is unchanged
unless the user says otherwise.

**Nothing in this file authorizes building the candidate.**

## Loss-protection note (Housing/Engine, 2026-08-22)

**The WW work-set exists in exactly one place on Earth: an uncommitted,
local-only submodule branch on this machine.** `git branch -a --contains
fc2e1cd6f9` returns one ref, a local branch; no remote carries it; the
fork's own superproject records `81f12f31`, not `fc2e1cd6f9`, as its
committed gitlink. **The `.patch` capture in this directory
(`fork-aurora-81f12f31.patch`, tracked since 2026-08-17, verified reverse-apply-clean
against `fc2e1cd6f9` by History/Bridge and Housing/Engine 2026-08-22)
is the only thing that survives if that working tree is lost** — extracted
2026-08-22 specifically because of this flag, not because application was
decided.

## Reproducibility contract (why this is tracked, not just reported)

- **Base:** `81f12f31` (the fork's OWN committed baseline — not the upstream
  pin `cf3ffc9886`, which is a different, larger distance and was the
  source of the original error).
- **Tip:** `fc2e1cd6f9`, one commit on top of the base.
- **Extraction boundary:** the 13 files in the table above, from
  `81f12f31..fc2e1cd6f9`. Re-derive with
  `git diff --numstat 81f12f31..fc2e1cd6f9` if the branch moves — do not
  assume the file list is fixed.
- **Captured, not just measured:** `fork-aurora-81f12f31.patch` (pre-existing, tracked since 2026-08-17),
  applied to a scratch checkout of `81f12f31` and hash-compared file-for-file
  against `fc2e1cd6f9` — all 13 files MATCH.
- **Verified independently, 2026-08-22 (Foundry, after Housing/Engine's
  retraction and History/Bridge's fork-parent critique):** every number in
  every table above reproduced exactly against the commands shown. This
  correction pass double-checked, not single-checked — the file was wrong
  once tonight and this rewrite does not get a pass on the same rigor.
