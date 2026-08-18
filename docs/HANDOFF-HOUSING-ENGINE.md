# Handoff — Housing Security (+ Engine duty)

> *(Cross-reference added 2026-08-16 by Housing/Engine, on the Librarian's new-handoff
> audit. Five files answer to "the Housing handoff" and none cited all the others — a
> successor could read one and never learn the rest existed. Nothing was merged or
> rewritten; only this block was added, identically, to all five. Any older "the
> current one is…" pointer below this block predates it.)*
>
> | doc | kind | dated |
> |---|---|---|
> | [`HANDOFF-HOUSING-ENGINE-2026-08-16.md`](HANDOFF-HOUSING-ENGINE-2026-08-16.md) | **CURRENT INSTANCE HANDOFF** — §1002 six-section order plus a §7 monitor exemplar: owned, half-done, do-not-trust, and the unresolved 10–29 vs 25–29 tag-range contradiction. **Start here for present state.** | 2026-08-16 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-14.md`](HANDOFF-HOUSING-ENGINE-2026-08-14.md) | **PRIOR INSTANCE HANDOFF** (§1002 order, tale §1003) — superseded for state by the 08-16 file; still owns the pre-08-16 history it records. | 2026-08-14 |
> | [`HANDOFF-HOUSING-ENGINE.md`](HANDOFF-HOUSING-ENGINE.md) | **PRE-ORDER HANDOFF** (written before §1002 existed; has none of its sections) — superseded twice over; pre-08-11 history only. | 2026-08-10 |
> | [`HOUSING-HANDOFF.md`](HOUSING-HANDOFF.md) | **RETIRED-INSTANCE RECORD** (user ruling) — the trust-failure account and the banked-state receipts of the pre-08-04 campaign; not current state. | 2026-08-04 |
> | [`HOUSINGTEMP-HANDOFF.md`](HOUSINGTEMP-HANDOFF.md) | **HOUSINGTEMP LANE CHARTER** (user ruling) — a DIFFERENT lane, paired against Housing to catch what every lane misses; listed because its name says handoff. | 2026-08-05 |
>
> **Picking this lane up: read the 2026-08-16 handoff for state, [`LANES.md`](LANES.md)
> + [`DO-NOT.md`](DO-NOT.md) for the rules, and the older files only for the history
> each owns.** Whether the older files should merge or archive is a Librarian/user
> ruling, deliberately not taken here.

> **SUPERSEDED — do not use this as the lane handoff.** It predates the §1002 order and
> has none of its six sections. The current one is
> [HANDOFF-HOUSING-ENGINE-2026-08-14.md](HANDOFF-HOUSING-ENGINE-2026-08-14.md)
> (tale §1003), and its **DO NOT TRUST** section lists published numbers of mine that are
> wrong — including one that has since become doctrine. Read this file only for
> pre-08-11 history.

**Written:** 2026-08-10 · **Tree at handoff:** `e57708ea7b` · **Branch:** `integrate/dusk-api-coexist`

---

## 0. Read this first — the lane discrepancy

[LANES.md](LANES.md) line 20 defines **Housing Security** as *"Containment audits only —
covenant gate, commit / mod-folder hygiene, and negative-controls on Bridge's tooling.
**Builds nothing.**"*

**That is not what this chat has been doing.** The user assigned this chat **Engine** duty
in addition to Housing, and the whole WW native-rooms campaign (room seam, stage seam,
lighting translators, camera port) was built here. Do not read the charter and conclude you
should stop building. You hold **both**:

- **Housing Security** — containment audits, covenant gate, commit/mod-folder hygiene,
  negative-controls on Bridge's and Foundry's instruments.
- **Engine** — the WW porting work itself.

If the user ever splits these again, the charter row is the authority for the Housing half.

---

## 1. Standing rules that override defaults

These came from the user directly and are non-negotiable. Several were learned the hard way.

| Rule | Meaning |
|---|---|
| **Zero-bake, byte-identical** | No content edits to donor assets anywhere. Every donor→receiver difference translates at the **consumption boundary** in receiver code. |
| **No legs** | A "leg" = a receiver TU referencing a WW symbol. Detected by `build_ww_excluded.bat`. A stub that makes the gate link **hides** a leg — it does not remove one. |
| **Parallel donor stacks, never legs** | Donor code lives in its own WW-layer TUs, classified in `ww_layer_manifest.py`. |
| **No mounts, no bridges** | Whatever isn't native, make it native. Bridges are labelled and owed, never endpoints. |
| **Port full state machines** | Never park donor state-driven behaviour as "optional". The donor's state machine IS the spec. |
| **Never substitute — add and label** | Porting is additive. Never delete receiver code to make room for donor code. |
| **If a system would fix an issue, PORT IT** | Always. |
| **Nothing is dropped** | User ruling, 2026-08-10. When a donor concept has no receiver counterpart, add sockets — do not discard. |
| **Never ship WW assets or code** | dusklight cannot under any circumstances ship them. See `docs/NEVER-PUSH-STRIP-SET.md`. |
| **Do not push** | Nothing is pushed until the porting work is complete. This is a fork; origin ≠ dusklight main. |
| **Aurora debug instrumentation** | RETAINED until the ropes are resolved (user ruling, 2026-08-07). |
| **`patches/aurora/local/working-tree.patch`** | Never submit this. |
| **Every reply ends with a WHOSE-TURN block** | Naming the lane owning each next action. |
| **Comment style** | `============` block comment headers on all new/modified code. |
| **Cache wipe after every rebuild** | `dawn_cache.db*` / `pipeline_cache.db*` in `%AppData%\TwilitRealm\Dusklight`, unprompted. |

---

## 2. Gates — run all of these before claiming done

```bash
python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --check
```
```bash
python tools/foundry/banner_lint.py
```
```bash
build_ww_excluded.bat
```

`build_ww_excluded.bat` must report **`LINKED CLEAN`** — and it must get there **by removal,
not by stub**. If you changed `files.cmake`, regenerate the exclusion list first or configure
fails with a STALE stamp:

```bash
python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-cmake
```

A new WW-layer TU needs **four** registrations or the gates fail:
`files.cmake` · `ww_layer_manifest.py` (WW-owned dict) · `docs/NEVER-PUSH-STRIP-SET.md`
Tier-1 (both the `.cpp` **and** the `.h`) · and the banner block.

Build: `build_run.bat` (RelWithDebInfo).

---

## 3. Docs — where the orders live

| Doc | What it holds |
|---|---|
| [DO-NOT.md](DO-NOT.md) | **Hard-stop registry.** Read the relevant entry BEFORE writing code on a surface it names. No AI instance may self-approve an exception — escalate. **DN-1**: never stamp a guessed room id on BG owners. **DN-3**: no mount-time BDL parsing; use the consume-time cached resolver. |
| [AGENT_INDEX.md](AGENT_INDEX.md) | Live state map + multi-agent protocol. Orient here. |
| [LANES.md](LANES.md) | Lane charters. See §0 above for the discrepancy. |
| [NEVER-PUSH-STRIP-SET.md](NEVER-PUSH-STRIP-SET.md) | Tier-1 strip set. Every WW-layer file must be listed. |
| [AGENTS.md](../AGENTS.md) | Shared agent rules. |
| [Interconnected Chats/INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) | Cross-lane bus. Push donor-faithful ready deliverables as `§N`, unprompted. |

**Donor sources:** decomp source `D:\XXXXXXX\WW DP\src` · arcs `D:\XXXXXXX\Ex WW` ·
debug linker maps `D:\XXXXXXX\WW Debug maps\tww_debug_maps\debug` · Winditor `D:\XXXXXXX\Winditor`.
**Rule: DECOMP-FIRST.** Read how the donor's own actor does it before fixing from the receiver
side. Skipping this cost 4 debug rounds once already.

---

## 4. Where the work actually stands

### Landed and verified

**Outset Island is walkable from byte-identical vanilla arcs.** `Stage: sea`, `R44_00/room.dzr`,
24 PLYR, room 44, player executing and moving. Lighting translation runs (`EnvR=52 Colo=10 Pale=57`).

**Camera algorithm port — committed `e57708ea7b`.** The donor CRAWL algorithm, ported as a
parallel donor stack:
- [ww_cam_crawl.cpp](../src/d/ext_plugin/ww_cam_crawl.cpp) — the algorithm + its DMC latch
- Receiver reaches it through `dCamera_setExtraEngineHook`, a NULL-default extension point
  naming no donor, added in [d_camera.h](../include/d/d_camera.h) / [d_camera.cpp](../src/d/d_camera.cpp)
- Installs from **inside** the layer via `dExtWwRoom_installHooks` (the boundary symbol the
  receiver already names), so `d_ext_npc_mount.cpp` never learns a donor name
- All gates pass; `LINKED CLEAN` by removal

### The decode that backs it

Receiver algorithm enum = donor's **minus {HUNG, CRAWL}**. Derived from shared style 4CCs,
then confirmed outright by `dCamera_c::engine_tbl` ([d_camera.cpp:201](../src/d/d_camera.cpp:201)),
which states the same twelve in order then continues into eight TP-specific algorithms.
**CRAWL, HUNG, VOMIT, SHIELD are confirmed absent from the receiver.**

```
0 letCamera  1 chaseCamera  2 lockonCamera  3 talktoCamera  4 subjectCamera
5 fixedPosition  6 fixedFrame  7 towerCamera  8 rideCamera  9 manualCamera
10 eventCamera  11 hookshotCamera │ 12+ TP-only
```

**Trap avoided — do not undo this.** Receiver `mStyleSettle` (0x158) has *exactly* the donor
`dCamera_DMC_system` layout `{u8@0, cSAngle@2, cSAngle@4}`. It is **not** the DMC record.
All 87 receiver uses are offset 0x0 as a settle flag, its two angle fields are written nowhere
in the repo, and a differential across `subjectCamera`/`lockonCamera` independently binds that
flag to the donor's `m100/m101/m102`. Layout agreement is shared-ancestor coincidence; usage
disagrees. The DMC latch is therefore **state the WW layer owns**.

### Blocked / not yet wired

> **✅ DATA HALF LANDED 2026-08-10 — bus §673.** `ww_cam_data.cpp` + generated
> `ww_cam_data.inc` (gen_cam_data.py from donor source): 145 styles (BE camstyle.dat-shaped
> blob, algs mapped, sockets 20-27), 63 donor-axis type rows, 35 BG type names — resident +
> self-tested, selecting nothing. The generator independently reproduced this section's
> decode (Subject slots, CC02 0x0C2, LN17 0x042) and added CC01 (0x082) as a third CRAWL
> style. **Mode axes are NOT positionally equivalent past slot 4** (measured; see §673) —
> rows stay donor-axis. Remaining: the donor mode-selector port + status-bit vocabulary
> translation (user ruled FULL).

> **✅ SELECTOR HALF BUILT 2026-08-10 — bus §674.** `ww_cam_select.cpp` ports donor
> nextType/nextMode/latch onto the donor axis through the new NULL-default
> `dCamera_setSelectHook`; data sources swap to the WW blobs on WW host stages and restore on
> leave. m1AE = receiver `field_0x223` (measured); receiver `mFakeAngleSys`/`setUSOAngle` IS
> the donor DMC system (crawl-TU alignment check owed). ModeFix honored as translated input.
> Sockets logged. All gates green. **Playtest owed** — camera feel on Outset is the verdict.

**Nothing selects algorithm 20 yet.** The style + type data is the other half:

- **Style records** (fully decoded, ready to write): CC02 flags `0x0C2`, LN17 flags `0x042`,
  identical 30-float param sets. Donor order `{id, alg, params[30], flags}` → receiver
  `{id, u16 alg, u16 flags, f32 params[28]}`. Dropped donor params 28/29 are
  `LOCKON_FOVY_MIN/MAX`; crawl reads only 0,1,3,4,5,10,25, so it loses nothing.
- **Type entry** — blocked on a measurement. `dCamera_type_data.field_0x18[2][11]` is
  **`[mIsWolf][mCurMode]`** — 11 modes doubled for wolf form, **not** 22 situations.
  The real gap is **20 donor modes → 11 receiver modes**.
  Donor `Subject` non-NONE slots: 0=LN17, 4=SS01, 10=SX01, 11=SY01, 13=CC02, 14=SN15, 16=EN00.
  **LN17 at slot 0 is in range and is also a CRAWL style** — that is the reachable path,
  needing no new sockets.
  Slots 11/13/14/16 are past the receiver's mode range. **Nothing is dropped** (user ruling):
  add sockets. But sockets alone are inert — `mCurMode` is driven by the receiver's mode
  machine and never exceeds 10, so the donor's **mode selector** must come across too.

**Owed:** `m1AE` gates two crawl branches; the donor's own `d_camera.cpp` only ever writes it
`0` and its `= 1` setter is outside the camera. Running with it 0 is the donor's own behaviour
when unset — not an invention — but find the setter.

---

## 5. Next work order — the donor disc reader (user-approved 2026-08-10)

> **⚠ SUPERSEDED IN PART — build L2 as a PLUGIN, not a receiver leg** (Librarian-stamped 2026-08-06, per the planning-instance L2 reversal; ruling **A** stands). The steps below add `wwIsoPath` to `settings.h`/`settings.cpp` + a receiver-tree TU + a prelaunch field — the exact receiver-leg class ruling (A) exists to eliminate. **Corrected design = L2a:** same isoPath *pattern*, plugin *home* — the plugin declares its own config var (SDK config service `register_var`), does disc I/O plugin-side, and hooks `wwRoom_aliasArcFileName` via `DEFINE_HOOK`; **zero receiver changes, no `settings.cpp` leg, no receiver TU.** **Prelaunch REVERSED:** the earlier "visible-but-optional field" is a receiver UX leg — instead a quiet config key + the 19c load-time gate refusing legibly (log/ui/overlay). **Before committing the seam (Foundry / 19a–19b):** confirm `wwRoom_aliasArcFileName` is reachable as a hook target (not file-local / ICF-folded). **Read L2a + the reversed-prelaunch note first:** [ttw-methods-review.md](WW%20Linked/ttw-methods-review.md) Band-0 L-series table — **L2a row (line 241)**, **prelaunch reversal (line 248)**; ferried as **bus §670**.

> **✅ BUILT 2026-08-10 per L2a — see bus §671.** Plugin `mods-src/ww_donor_disc/`
> (`wwDonorDisc`), zero receiver changes, R3 boot+fst roster gate wired, all gates green,
> ON-ROSTER verified against the user's image by an independent Python implementation.
> The steps below are the SUPERSEDED receiver-side design — kept for the decision record
> only. **PLAYTESTS A+B PASSED 2026-08-10; staged donor arcs RETIRED** (moved to the mod
> folder's `_retired_staged_donor/`, all 103 verified byte-identical to the roster first).
> **Step 6 CORRECTION:** there is NO camtype.dat on the WW disc — that file is a TP-ism.
> The donor's camera type table is compiled-in source data (`d/d_cam_type.cpp` in the
> decomp), so the type/style data ports as WW-layer code like CRAWL did; the disc reader
> has no role in the camera work. See bus §672.

> **⤷ FINAL — row-14 ruling (user 2026-08-11 · bus §711 · Librarian supersession stamp).** The
> WW-named `wwIsoPath` receiver-leg in the steps below is **WITHDRAWN**. Landed design =
> **`backend.extraIsoPath`** — a **generic, WW-agnostic** host field in `settings.h`/`settings.cpp`
> that the plugin **consumes** (HOST KEY WINS; the mod-scoped var demoted to dev fallback). This
> **RECONCILES** the §670 reversal rather than reversing it: the reversal objected to a *WW-named*
> receiver leg, and a generic donor/extra-disc field has no WW in its name (must not contain `ww`).
> **Prelaunch surfacing = a named Engine follow-on** (visible-but-optional row, its own focused
> pass), not built here. Build = Engine.

**Decision:** the mod already requires the user's own TP disc, so requiring their own **WW
disc** adds no burden — and it means dusklight ships and stages **zero** donor bytes.
Structurally true, not maintained by discipline.

**Images confirmed present:**
```
D:\Taking a Dumpy\Minerva_Myrient\Redump\Nintendo - GameCube - NKit RVZ [zstd-19-128k]\
    Legend of Zelda, The - The Wind Waker (USA).iso    1.36 GB   ← plain, uncompressed
    Legend of Zelda, The - The Wind Waker (USA).rvz    0.80 GB
    Legend of Zelda, The - Twilight Princess (USA).iso           ← the configured TP image
```
> **Shell gotcha:** that folder name contains `[...]`, which PowerShell parses as a wildcard.
> Use `-LiteralPath` or the Bash tool. Silent empty listings cost a round here.

**Mirror the TP pattern, do not invent a second mechanism:**
- `backend.isoPath` — `ConfigVar<std::string>` at [settings.h:605](../include/dusk/settings.h:605),
  defaulted [settings.cpp:242](../src/dusk/settings.cpp:242), registered
  [settings.cpp:531](../src/dusk/settings.cpp:531), UI in
  [prelaunch.cpp:307](../src/dusk/ui/prelaunch.cpp:307)

**Steps:**
1. Add `backend.wwIsoPath` on that exact pattern. Empty default = off = WW layer inert.
2. New WW-layer TU `ww_donor_disc.cpp` holds the mount. Register in all four places (§2).
3. Route `wwRoom_aliasArcFileName` at the donor disc instead of staged files — same vanilla
   names, same bytes, different source. That function is already the single resolution point.
4. Verify Outset still loads with `res/Stage/sea/` **deleted**.
5. Retire the 101 staged `sea` arcs + `bmgres`/`bmgresh` from the tree *and* the strip set.
6. `camtype.dat` then comes off the same reader → unblocks the camera type work above.

**Caveat:** the donor disc is user-supplied. Missing / wrong-region / corrupt must be a normal
handled condition with a clear message, never an assert. `prelaunch.cpp` already solves this
for TP — mirror it.

**Open question for the user:** does the donor disc become a **prelaunch requirement**
(surfaced beside the TP disc, so a missing WW disc is explained up front), or stay a quiet
config key that only WW stages need? First is friendlier; second keeps WW fully optional.

---

## 6. Longer queue (Outset)

- Grass stand-ins — 153 of 193 resolving Outset placements are TP `fpcNm_GRASS_e` aliases
- The **282** unported Outset placements (of 475; a sparse island is expected, not a fault)
- "Island disappears at certain angles/heights" — **unexplained**. `daBg` has no
  `fopAcStts_CULL_e`, so frustum culling is ruled out. Leading untested hypothesis:
  room-gated BG draw with the airborne player's room flickering 44/−1.
- Sky correctness unconfirmed; `vrbox2` (clouds/haze/false sea) not done
- K0 konstant colours not delivered — needs WW tevstr setup (§407 ported only the point-light half)
- `FieldCushion` camera type — blocked on CRAWL **and** HUNG
- WW/TP camera toggle — user wants it once the port is viable
- Move §619's №21 bake edit fully off `adapt_room_arcs.py`

## 7. Launch

```bash
run_outset.bat
```
Then in-game: **Warp tab → "Dev stage (--stage)" → `sea room 44 layer -1`**. Slow first load
(fifty rooms — `sea` is every island at once; trimming would be editing, so the whole stage comes in).
