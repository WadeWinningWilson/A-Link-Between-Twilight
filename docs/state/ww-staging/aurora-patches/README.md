# AURORA PATCHES — MOVED (pointer, not content)

era: plugin-delivery
<!-- era rationale: pointer only; content relocated to the plugin folder on user order 2026-08-22 | Foundry -->

> **THE AURORA PATCH SET IS NOT HERE. It is:**
> ### `mods-src/ww_donor_disc/aurora-patch/`

## Why it moved (user order, 2026-08-22)

> *"the patch should leave in its own folder in the plugin repo"*

**And there was a real exposure reason on top of the order.** This path —
`docs/state/ww-staging/` — is matched by **no** never-push strip rule: the
generated rules key on `(^|/)ww_` (underscore), and `ww-staging` is a
**dash**. The plugin folder `mods-src/ww_donor_disc/` **does** match, which
is why `FORK-STRIP-LEDGER.md` already lives there. So the aurora patch set
was sitting in a location that would ship in the fork release; it now sits
in one that structurally cannot. Same distinction that produced the
duplicate-ledger incident earlier — recorded here so it is not re-learned.

## What moved

| file | note |
|---|---|
| `AURORA-PATCH-LEDGER.md` | the protocol + entries (patch file FIRST, apply SECOND) |
| `AURORA-PATCH-0001-skipdraw-tev-logger.patch` | the live 53-line accepted patch, byte-verified |
| `TRACKED-lwood-candidate-9files.md` | the 995-line WW work-set measurement (filename retains its original, now-inaccurate "9files" — corrected in-content) |
| `fork-aurora-81f12f31.patch` | Integrator's 2026-08-17 capture of the WW work-set — the canonical 995-line recovery path |
| `dusklight-main-aurora-81f12f31.patch` | Integrator's 2026-08-17 loss-protection capture |
| `fork-aurora-stash-albw-pad-guard.patch` | Integrator's extraction of the orphaned pad-guard stash |
| `HISTORICAL-README.md` | the original 2026-08-17 README (renamed to free this filename for this pointer) |

Tracked files moved with `git mv` so history follows them. **Do not re-add
content here.** Rows and patches go in the plugin folder.
