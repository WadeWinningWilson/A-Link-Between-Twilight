# FORK-STRIP-LEDGER — MOVED (pointer, not content)

> **THE LEDGER IS NOT HERE.**
>
> **AUTHORITATIVE (2026-08-22 onward):**
> ### `%USERPROFILE%/Documents/A Link Between Twilight WW/FORK-STRIP-LEDGER.md`
> The plugin became its own project outside the fork on user order, and the
> ledger moved with it — it is a record about the PLUGIN's relationship to the
> fork, so it belongs beside the plugin.
>
> **STALE COPY, still on disk:** `mods-src/ww_donor_disc/FORK-STRIP-LEDGER.md`
> — the fork's copy. **Byte-identical as of 2026-08-22 (17,226 B both), so
> nothing has diverged yet.** It goes when the fork copy of the plugin goes,
> which is a deliberate fork-strip step, not automatic.
>
> **TWO IDENTICAL FILES WITH NO MARKED OWNER IS HOW DIVERGENCE STARTS.**
> Edit the authoritative one only. If you find them differing, the new home
> wins and the fork copy is stale — do not hand-merge them.
>
> Owner: **Foundry**, from a direct user order 2026-08-21.

## Why this stub exists

This file was briefly a SECOND ledger. On 2026-08-21 History/Bridge searched
for a fork-strip ledger with `find docs -iname "*strip*"`, found none, and
built one here. **That search was scoped to `docs/` — the canonical ledger
lives under `mods-src/ww_donor_disc/`, deliberately, because that directory is
NEVER-PUSH and the order requires the record to sit OUTSIDE the fork release.**
The placement is correct and the search was wrong.

Foundry merged the one adjudicated entry (`d_a_ext_vegetation.cpp`,
NOT-STRIPPABLE) into the canonical file. **Nothing was lost. Nothing here is
authoritative.**

**This stub is the fix for the discovery failure, not a leftover:** a lane
searching `docs/` — the natural place to look — now lands on the pointer
instead of concluding no ledger exists. Do not re-add content here.

## The distinction the ledger turns on (kept here only because it is what
## sent the search to the wrong place)

**PUSH-STRIP** (`docs/NEVER-PUSH-STRIP-SET.md`, 111 entries) = do not push this
file upstream. Reversible; the file stays on disk.

**FORK-STRIP** (the canonical ledger) = DELETE this fork TU, the plugin serves
it now. **Irreversible.**

Being listed in `NEVER-PUSH-STRIP-SET.md` is **not** a fork-strip safeguard,
and `destination: PLUGIN` in `tracker/rows/` is an OWNERSHIP adjudication —
where a TU belongs, not evidence the plugin implements it. The seam-tracker
schema has no strip field at all (checked across all 123 rows).
