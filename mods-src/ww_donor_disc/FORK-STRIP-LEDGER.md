# FORK STRIP LEDGER — what left the fork, why, and how the plugin delivers it

era: plugin-delivery
<!-- era rationale: release-hygiene protocol for the fork's mod release | Foundry, from a direct user order 2026-08-21 -->

## The order (user, 2026-08-21, verbatim intent)

> Whenever an item is ported plugin-side only, the corresponding fork item
> must be **stripped**, and documented **outside of the fork repo** — in exact
> terms: what was stripped, what it was for, and how it was delivered
> plugin-side. The fork is getting its mod release soon. Applies
> **retroactively** to items already proven plugin-side.

## Why this file lives HERE

"Outside of the fork repo" is satisfied mechanically, not by trust:
`mods-src/ww_donor_disc/` matches the generated strip set's `(^|/)ww_` rule,
so **nothing in this directory ever reaches the public fork remote** — the
same never-push mechanism that already protects the plugin source (precedent:
`75c12dda7e`, and the Zone model at CALLS §row 361/362). It stays committed
locally for loss protection, and it ships nowhere. A ledger about what the
fork release must not contain must itself not be in that release.

## Protocol (binding for every plugin-side port from now on)

1. **Trigger:** an item is **proven** plugin-side — receipt-backed on stock
   `dusklight-main` (the four tests of CALLS row 494: stock source · no
   `files.cmake` cargo · zero-bake by **payload-absence** · consume-boundary
   translation only). Not "compiles", not "should work" — proven.
2. **Ledger row FIRST, strip second.** The row names the exact fork paths
   (files, or line-ranges for partial strips), what the item did, and the
   plugin mechanism that replaces it. A strip with no row is a deletion, not a
   migration.
3. **Strip as a DEDICATED commit** per item (or coherent item group), no
   content changes mixed in — same reasoning as the repo-layout ruling's
   `git mv` rule: the removal diff must be reviewable alone. **Commit subject
   begins `strip:`** — that prefix is what lets `strip_ledger_lint.py` find
   strip commits mechanically for the reverse check; an unprefixed strip
   commit is invisible to the lint and therefore a protocol violation itself.
4. **Sequencing:** strip commits land **AFTER** the held milestone commit
   (user hold: commit when lwood lands). Stripping into an uncommitted
   worktree would tangle the milestone diff with removals.
5. **Scope guard:** strips remove **WW-derived fork items superseded by the
   plugin** — never receiver/vanilla code, never Zone-1 mod-folder content
   (already outside the fork), and never the plugin's own `ww` A/B arms.
   The standing "never substitute — add and label" rule is about porting INTO
   a tree; this is the user-ordered release-hygiene inverse, and it applies
   only where the plugin demonstrably owns the behavior.
6. **VANILLA-TP GUARD (user order, 2026-08-21):** TP has similar functions —
   a symbol sweep alone must never authorize a cut. Before any partial strip
   inside a receiver TU, **three checks, all recorded in the row**:
   ① the TU's **`KIT-LINEAGE` banner** read (a `native-port`/receiver TU gets
   arm-only cuts, never body cuts); ② the arm's **WW gate named** — the block
   must sit behind a WW-scoped condition (`dExtWwSave_isWwHostStage`, a
   `Ww*`-symbol branch, a WW env gate) rather than merely *using* a
   WW-sounding name, because TP-native code adjacent to a WW arm matches the
   same grep; ③ the **seam tracker row + `NEVER-PUSH-STRIP-SET.md`
   declared-lineage basis** cross-checked for the paths. A partial row moves
   MAPPED → STRIPPED only with all three cited. Whole-TU rows need ① and ③.

## The two strips are different operations (History/Bridge, merged 2026-08-21)

*(Authored by History/Bridge in a parallel ledger built when this file's
never-push placement defeated their search — merged here verbatim-in-substance
with attribution; their copy is now a pointer. The duplication was the COST of
this file's placement, and this section is the part of their build that this
file lacked.)*

| | **PUSH-STRIP** | **FORK-STRIP** (this file) |
|---|---|---|
| instrument | `docs/NEVER-PUSH-STRIP-SET.md` (111 entries) | this file |
| means | do not PUSH this file upstream | DELETE this fork TU; the plugin now serves it |
| reversible | yes — file stays on disk | **no — the implementation is gone** |
| basis | UNION of filename convention AND declared lineage (user-ruled 2026-08-07) | a PROVEN plugin counterpart, measured |

A file listed in `NEVER-PUSH-STRIP-SET.md` is **not** thereby fork-strippable —
that list is a push gate and says nothing about whether the plugin serves the
file. **Being on it is not the safeguard.**

**And the seam tracker cannot carry this either** (History/Bridge, enumerated
across all 123 rows): the schema has **no strip field**, and
`destination: PLUGIN` is an **ownership adjudication** — where a TU *belongs*,
not evidence the plugin *implements* it. **Reading `destination: PLUGIN` +
`linked: REGISTERED` as "safe to strip" is a failure mode this ledger exists to
prevent.** See the vegetation entry below for the live example.

## NOT-STRIPPABLE entries (adjudicated, never bulk-filled)

### `src/d/actor/d_a_ext_vegetation.cpp` — **NOT-STRIPPABLE**
*(Adjudicated History/Bridge 2026-08-21 on Housing/Engine's measurement;
merged verbatim-in-substance.)*

- Seam row `s7461b719`: `destination: PLUGIN`, `linked: REGISTERED`,
  `portable: UNKNOWN` — adjudication, not implementation evidence.
- Push gate: listed `NEVER-PUSH-STRIP-SET.md:167`. Irrelevant to fork-strip;
  recorded so the distinction is not re-litigated.
- **Plugin counterpart: NONE**, measured across `mods-src/ww_donor_disc/`:
  `vegetation` → zero source hits; `GRASS` → two comments, no `fpcNm_GRASS_e`
  routing; `kusa` → only the `kDefaultObjectArcs` roster string
  (`main.cpp:85`). No donor-grass actor exists plugin-side.
- Why deletion would be severe: the only DN-11-compliant grass server in the
  estate (native 31×31 `M_kusa05_RGBATEX` vs donor 64×64 `l_Txa_ob_kusa_aTEX`
  off a different vertex set — *"shared architecture is not shared assets"*).
- What WOULD unblock a strip: a plugin donor-grass actor serving
  `kusax1/kusax7/kusax21`, proven drawing. The ASSET half is already present
  (`Kusa` in the disc-served roster) — **what is missing is the actor, not
  the archive.**
- **AMENDED 2026-08-21 (History/Bridge), on the user's own sighting — the
  condition above is NECESSARY BUT NOT SUFFICIENT, and acting on it alone
  would make the island worse.** The user confirmed **TP grass, flowers and
  rupees VISIBLY PRESENT on Outset**. They are created PRE-WARP (`GRASS_e`
  ×118 covers grass *and* flowers; `ITEM_e` ×11 the rupees) and **persist
  across the stage change** — created-before plus present-after is the only
  reading left. **So this is a LIVE DN-11 VIOLATION on the shipping product,
  not a compliant absence**, and *"a missing prop is preferable to a foreign
  one"* does not apply: nothing is missing, the wrong thing is there.
  **Porting donor grass while the TP population still stands would put BOTH
  on the island.** The strip condition is therefore: a plugin donor-grass
  actor **AND** a stage-change purge of the pre-warp population (Housing/
  Engine's surface, upstream of the port). **Do not read the bullet above as
  a green light on its own.**

## Row format

| field | meaning |
|---|---|
| **item** | the behavior/system, in plain terms |
| **fork paths** | exact files (or file:line ranges) stripped |
| **was for** | what it did in the fork build |
| **plugin delivery** | mechanism in `ww_donor_disc` that replaces it (file + gate/receipt) |
| **unblock condition** | REQUIRED for any row not yet `STRIPPED` — what exactly would move it there, written NOW while the reasoning is fresh. *(Back-ported 2026-08-22 from `AURORA-PATCH-LEDGER.md`'s required removal condition — History/Bridge: "an entry without an exit condition is a permanent entry by default, and a ledger full of those is a graveyard rather than a work list.")* |
| **strip commit** | SHA of the dedicated removal commit |
| **status** | `UNMAPPED` → `MAPPED` → `STRIPPED` (or `KEEP` with a reason) |

## Retroactive seed — items proven plugin-side by the Outset milestone

**MAPPED-MECHANICAL, 2026-08-21 (Foundry):** the fork lines were identified
outright from what is on disk — the user's correction that origin + tracker +
plugin tracing + Engine's proven traces suffice was right. Method: `git
ls-files` for WW-named TUs, then a symbol sweep (`WwSky` / `WwAlways` /
`dKyWw_` / `[Uu]sonami` / `WwWind`) with hit-blocks clustered at ≤20-line
gaps. **The fork's `src/d/ext_plugin/` directory names its files IDENTICALLY
to the plugin's** (`ww_cam_data.cpp`, `ww_room_loader.cpp`, …), so whole-TU
rows are unambiguous. Line anchors are as-measured today and will drift —
the SYMBOLS are the durable anchor; re-run the sweep before cutting.

Owning-lane confirmation is still wanted **only for the partial rows** (arms
inside receiver TUs), where the cut line matters. Whole-TU rows need none.

| item | fork paths | was for | plugin delivery | unblock condition | strip commit | status |
|---|---|---|---|---|---|---|
| WW sky / vrbox / vrkumo env | **Whole-TU:** `src/d/d_kankyo_ww_sky.cpp` · `src/d/d_kankyo_ww.cpp` · `src/d/d_kankyo_ww_vrkumo.inc`. **Partial (WW arms in receiver TUs):** `d_kankyo_wether.cpp` (67 hits / 17 blocks, e.g. 982–1061, 1480–1510, 1773–1803) · `d_a_vrbox.cpp` (23/6: 230–239, 276–301, 330–349) · `d_a_vrbox2.cpp` (6/3: 598–606) · `d_kankyo_rain.cpp` (10/6: 1928–2034) | WW skybox / cloud ring / vrkumo on fork exe | `ww_sky.cpp` / `ww_vrbox.cpp` / `ww_vrkumo.cpp` | Housing/Engine confirms the partial-row cut lines against the vanilla-TP guard (protocol step 6); whole-TU rows need no further unblock | — | MAPPED (partials need Housing/Engine cut-line confirm) |
| WW kankyo / wind | **Whole-TU:** `src/d/d_kankyo_ww_wind.cpp`. **Partial:** `d_kankyo.cpp` (5/3: 1258, 2665–2670, 9440) · `d_particle.cpp` (41/5: 1494–1813 — wind/particle arms) | WW ambient color + wind on fork exe | `ww_kankyo.cpp` / `ww_kankyo_wind.cpp` | Housing/Engine confirms the partial-row cut lines (protocol step 6) | — | MAPPED (partials need confirm) |
| Waves / shore | **Partial only:** `d_kankyo_wether.cpp` usonami blocks (within 1825–1862) + wave arms in the same TU's block set above | WW sea surface on fork exe | `ww_wave.cpp` / `ww_shore.cpp` | Housing/Engine names the exact cut for every block (all-partial row — no whole-TU floor to fall back on) | — | MAPPED (all-partial — Housing/Engine names the exact cut) |
| WW cam ports | **Whole-TU, name-identical:** `src/d/ext_plugin/ww_cam_data.cpp` + `.inc` · `ww_cam_select.cpp` · `ww_cam_crawl.cpp` | WW camera behaviors on fork exe | `ww_cam_data.cpp` / `ww_cam_select.cpp` / `ww_cam_crawl.cpp` (same names, plugin-side) | None outstanding — whole-TU, name-identical, mapping is the unblock; ready for the dedicated `strip:` commit once the milestone commit lands | — | MAPPED |
| Room44 stage/room serve + BDL consume + worldize | **Whole-TU:** `src/d/ext_plugin/ww_room_loader.cpp` · `ww_stage_loader.cpp` | Outset stage load on fork exe | `registry.cpp` (`PORT —` banners; `WW_BDL_CONSUME`, `bdl_consume_gate`) + `donor_disc.cpp` | None outstanding — whole-TU, ready for the dedicated `strip:` commit once the milestone commit lands | — | MAPPED |
| lwood / Alwd | **Whole-TU:** `src/d/actor/d_a_lwood.cpp` (KIT-banner TU; carries `dKyWw_`/wind arms) | trees on fork exe | `registry.cpp` lwood port (R2–R6 template) — **draw still open (Dawn)** | A USER-CONFIRMED DRAW on a non-crashed Outset (the standing acceptance test, not a mechanism explaining the logs) — the live lwood bisect on CALLS is this row's unblock condition in progress | — | BLOCKED-ON-LWOOD (mapped, not strippable) |
| akabe | **NONE — `git ls-files` finds no akabe TU in the fork tree.** | — | `registry.cpp` akabe port (`akabe_created` receipt) | N/A — no fork counterpart exists to strip; this row is a permanent record, not a pending item | — | NO-FORK-COUNTERPART (nothing to strip; row kept as the record) |

**Not strip candidates** (named so nobody "helps"): the 35-TU `ja1_*` audio
stack and other Zone-2 systems the plugin does **not** yet deliver; the 7
tree-side `d_a_ext_*`/`d_a_ww_*` actors until each has a proven plugin
counterpart; receiver files with KIT-DONOR banners (`d_a_vrbox`, `d_stage`,
…) — those are the receiver's own TUs with lineage notes, not fork WW cargo
(their **WW arms** are strippable per the partial rows above; the TUs are not).

**Also not yet candidates — the rest of `src/d/ext_plugin/`** (measured in the
same sweep): `ww_itemmdl_dispatch.cpp` · `ww_item_data.cpp` + `.inc` ·
`ww_tsubo_data.cpp` · `ww_npcmount_dispatch.cpp` · `ww_import_gate.cpp` ·
`ww_layer_select.cpp` · `ww_event_names.cpp` · `ww_misc_dispatch.cpp` ·
`ww_profile_register.cpp`. The plugin does not deliver those systems yet.
`ww_profile_register.cpp` is the closest call — the plugin's `registry.cpp`
does register Outset's actors above-enum — but the fork file registers more
than the plugin's current roster, so it strips **per-entry as each actor's
plugin port is proven**, not as a TU. When one of these systems lands
plugin-side, its row enters the table above via the protocol.

## Housing/Engine confirmation of the PARTIAL rows (2026-08-21)

Foundry asked this lane for cut lines on the arms inside receiver TUs. **The
answer is that most of them are not line ranges at all — and the ones that are,
have a shape that makes the cut mechanical.**

**Method:** symbol sweep clustered at <=20-line gaps (as Foundry's), then each
cluster resolved to its **enclosing function**, then every candidate tested
against the **stock tree** — the only test that separates fork cargo from
receiver code.

### A. WHOLE-FUNCTION cuts — the function exists ONLY in the fork

Each verified absent from `dusklight-main/src/` (0 files):

| function | fork TU |
|---|---|
| `wwLoadWaveCalmIni` · `dKyw_ww_host_wind_onStage` · `dKyr_ww_wind_move` | `d_kankyo_wether.cpp` |
| `dKankyo_wave_Packet::draw` · `wave_move` | `d_kankyo_wether.cpp` / `d_kankyo_rain.cpp` |
| `dKyWwVrkumo_draw` | `d_kankyo_rain.cpp` |
| `daVrbox_ww_color_set` | `d_a_vrbox.cpp` |
| `dPa_control_c::ensureWwCommonRes` · `dPa_tearResFallbackRM` | `d_particle.cpp` |
| `dKyWw_isSkyHost` (8 fork files) · `dKy_usonami_set` | — |

**Cut the function, not a line range.** Line anchors drift; a function boundary
does not.

### B. IN-FUNCTION arms — and they are all the SAME SHAPE

The remaining hits sit inside receiver functions, and every one reads:

```
if (dKyWw_isSkyHost()) {   // WW leg
    ...
} else {                   // vanilla, preserved byte-for-byte
    ...
}
```

`d_kankyo.cpp:2665` (`setLight`), `:1258` (`envcolor_init`),
`d_kankyo_wether.cpp:2075` (`dKyw_wind_set`), `d_a_vrbox.cpp:35`
(`daVrbox_Draw`) — **strip = delete the WW leg and unwrap the `else`.** The
receiver path was never overwritten, so there is nothing to reconstruct. That is
the "add and label" rule paying its dividend at removal time.

### C. ONE ROW THAT IS **NOT** STRIPPABLE — and it is in the partial set

**`drawWave` EXISTS IN THE STOCK TREE.** It is receiver code with WW arms inside
it, not fork cargo. Cutting it as a whole-TU or whole-function row would remove
vanilla behaviour — precisely what the ledger's scope guard forbids. **Its arms
are strippable by the shape in B; the function is not.**

### D. A HYPOTHESIS THIS LANE KILLED BEFORE PUBLISHING IT

I suspected the **`[Uu]sonami`** anchor over-captured vanilla TP code (the name
reads native, and `mpTexUsonami` / `dKy_usonami_set` sit in receiver-named
functions). **Tested against the stock tree: `usonami` appears there ZERO times.
It is fork-added and Foundry's anchor stands.** Recorded because a plausible
correction that turns out to be wrong is worth exactly as much as one that is
right, and this one would have blocked a valid row.

### E. Count difference, stated so it is not read as a correction

My sweep used a wider symbol set (`WW_`, `wwKankyo`, `WwFoam`, `WwShore`,
`WwWave` added) and reports more hits than Foundry's — `d_kankyo_wether.cpp`
**97 vs 67**, `d_kankyo_rain.cpp` **25 vs 10**, `d_particle.cpp` **48 vs 41**.
**Different sets, not a defect in either.** Mine likely over-captures; the
whole-function and stock-absence tests above are what make a row safe, not the
hit count.

## Lint hook (Foundry-owed)

A ledger check belongs in the instrument estate: every `STRIPPED` row must
name a commit SHA that exists and touches only the row's paths; every strip
commit in the fork must have a ledger row. Not built yet — named here so it
is owed, not implied.
