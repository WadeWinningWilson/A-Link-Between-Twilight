# #1 `albw-meter` — extraction plan (Mod SDK)

| Field | Value |
|-------|--------|
| **status** | **P1 playtest** — fixed Win64 private-member layout crash (2026-07-23); retest green bar |
| **package** | `albw-meter` (first-party `.dusk`, `FEATURES game`) |
| **parent** | [mod-api-port.md](mod-api-port.md) |
| **target host** | TwilitRealm `origin/main` Mod SDK |
| **updated** | 2026-07-22 |

---

## 0. Goal

Move the ALBW meter suite out of forked `d_meter2` / `d_a_alink` / combat modules into one native mod that loads on **stock main** Dusklight via their Mod API, preserving player-facing behavior and the locked bundle rule (all meter spenders stay in this package).

---

## 1. What ships in this `.dusk` (bundle — locked)

| Subsystem | Today (fork) | In mod |
|-----------|--------------|--------|
| Energy meter + recovery + expand | `d_meter2.cpp` statics (`sALBWMeter`, …) | Mod-owned session state |
| Lockout + perks | `d_albw_lockout.*` | Mod module |
| HS rework / combat costs | `d_albw_combat.*` + alink cut hooks | Mod + hooks |
| Focused Arts | `d_focused_arts.*` | Mod module |
| Flurry Rush | `d_albw_flurry_rush.*` + sim scale | Mod module |
| Shield parry / bash / durability / manual | `d_albw_shield.*` + guard.inc | Mod module |
| Deku Leaf meter costs | alink leaf + `dMeter2_onALBWDekuLeaf*` | Gated; include when leaf ready |
| MQ **stamina** shop tiers | `d_albw_master_quest` meter rows | Config/save in mod; UI until #10 = cheat/Config or stub shop |
| Meter / FA skill scrolls | `d_albw_skill_scroll.*` | Mod + menu_skill hooks |
| Meter HUD draw | `d_meter2_draw` kantera/ALBW path + LoP pieces **required for the bar** | Draw hooks / replace draw helpers |

**Out of this package (later candidates):** full LoP item belt, wardrobe recovery tax, Postman rental UI (#10), wolf combat (#6).

---

## 2. Host reality (main Mod SDK)

| Need | Available? | Plan |
|------|------------|------|
| Native DLL lifecycle | Yes — `mod_initialize` / `update` / `shutdown` | Own all session state; reset on init/shutdown |
| Per-frame tick | Yes — `mod_update` | FA/Flurry/lockout/shield polls that today run from alink/meter |
| Function hooks | Yes — `HookService` + `DEFINE_HOOK` / `DEFINE_HOOK_SYMBOL` | Spend gates, guard, cut, cc_uty, meter draw |
| Config toggles | Yes — `ConfigService` | Mirror `game.focusedArts`, `flurryRush`, `manualShielding`, `shieldParryCombat`, `shieldDurability`, … under `mod.albw-meter.*` (or keep names via migration note) |
| Settings UI | Yes — `UiService` | Mod settings pane; drop fork ALBW Settings section over time |
| Game headers / ABI | Yes — `FEATURES game` + empty `GameService` epoch | **Do not** require fork-only `TARGET_PC` fields on shared structs |
| Meter / stamina host API | **No** | Mod owns pool; hooks vanilla oil/kantera draw + spend call sites |
| Sim time-scale service | **No** | Flurry phase: hook/own scale carefully; document unload = scene change |
| SaveFlags service | **No** | FA/MQ stamina tiers: use reserved event bits **or** Host `mod_dir` JSON until SaveFlags exists (document risk) |

---

## 3. Architecture decisions (lock these before code)

### D1 — Meter ownership: mod-owned pool (not fork `d_meter2` statics)

Fork already keeps ALBW value in **file-static** state in `d_meter2.cpp` (good). Extraction copies that model into the mod:

- Mod exports a small **internal** C API (`albw_meter_get/can/on_*`) used only inside the `.dusk`.
- Call sites that today call `dMeter2_canALBW*` / `onALBW*` become **hooks** that divert into the mod (pre-hook skip/replace or post-adjust), or hooked wrappers at the alink/item functions that currently spend.

**Do not** ship a patch that adds PC-only members to `dMeter2_c` on main (GameService epoch landmine).

### D2 — Vanilla oil meter relationship

On main, lantern oil remains the stock system. Options:

| Option | Pros | Cons |
|--------|------|------|
| **A (preferred)** — Hide/suppress oil HUD; ALBW bar draws in its place; oil cheats ignored while mod enabled | Clear ALBW feel | Must hook oil spend so lantern doesn’t double-tax |
| B — Drive ALBW from oil value | Less draw work | Couples to infinite-oil cheat; wrong semantics |

**Decision lean: A.** Infinite oil on main must not refill ALBW unless we explicitly map it.

### D3 — Mod lives **outside** this fork tree (locked)

`albw-meter` is **not** added under this repo’s `mods/`, `sdk/`, or any path inside dusklight.

| Piece | Where |
|-------|--------|
| Mod checkout | `C:\Users\xxxxx\Documents\albw-meter` (cloned from [TwilitRealm/mod-template](https://github.com/TwilitRealm/mod-template), 2026-07-22) |
| Host (main) checkout | `C:\Users\xxxxx\Documents\dusklight-main` — built RelWithDebInfo, `DUSK_ENABLE_CODE_MODS=ON`. Exe: `build\windows-msvc-relwithdebinfo\dusklight.exe` |
| P1 `.dusk` | `C:\Users\xxxxx\Documents\albw-meter\build\mods\albw_meter.dusk` — also copied to `%APPDATA%\TwilitRealm\Dusklight\mods\` |
| This fork | Reference implementation + playtest oracle until parity; then retire engine copies |

Build the mod with FetchDusklight / `DUSKLIGHT_DIR` / `DUSKLIGHT_VERSION` pinned to **main**, never against this ALBT tree.

### D4 — Phased delivery (vertical slices)

Never “port all of #1 then enable.” Ship slices that each leave a playable game.

| Phase | Slice | Exit criteria |
|-------|-------|----------------|
| **P0** | Scaffold `.dusk` + Config/Ui + Log; load/unload on main | Mods panel enable/disable; no gameplay change |
| **P1** | Core pool + recovery + HUD draw (oil suppressed) | Human Link sees ALBW bar; wolf unaffected |
| **P2** | Spend gates: sword / sidestep / roll / back-jump / bombs / bow / sling / boom / spinner / hook / Dom Rod / armor | Exhaustion + tired idle behave; lockout **not** yet |
| **P3** | Lockout begin/end + perk suite | Empty meter → lockout perks; refill to max clears |
| **P4** | Shield module (manual / parry / bash / durability / HUD icons) | Parry grants/penalties hit mod pool |
| **P5** | HS rework + Focused Arts (combat); shop via Config/cheat until #10 | FA bank + suppress ALBW during spend |
| **P6** | Flurry Rush | Perfect-dodge slow-mo; unload requires scene change |
| **P7** | Skill scrolls + MQ stamina capacity | Collect page-2 + capacity expand |
| **P8** | Deku Leaf meter costs | Only when leaf feature ready |
| **P9** | Fork retirement | `#if TARGET_PC` meter paths no-op when mod present **or** removed after parity sign-off |

### D5 — Hook inventory (first cut — expand in P2)

Priority targets (typed `DEFINE_HOOK` where possible):

- `daAlink_c::execute` — post: lockout/FA/shield/flurry ticks (or split to existing named methods)
- Guard path — `d_a_alink_guard.inc` entry points used by `dShield_*` today
- Cut / HS — `d_a_alink_cut.inc` sites for HS cost + FA
- Item procs — bow/bomb/hook/spinner/boom/grab bombling (today’s `dMeter2_onALBW*` call sites)
- `dMeter2_c` / draw — kantera meter draw + oil update (suppress oil, draw ALBW)
- `cc_at_check` / damage — lockout ATP boost, FA damage resolve (narrow hooks)

Prefer **few wide ticks** + **narrow spend hooks** over hundreds of micro-hooks.

### D6 — Config surface (initial)

Register under mod id (names illustrative):

- `enabled` (loader-owned)
- `focused_arts`, `focused_arts_cheat`
- `flurry_rush`
- `manual_shielding`, `shield_parry_combat`, `shield_durability`
- `parry_icons_mode`, meter HUD visibility knobs that today sit under ALBW Settings
- `master_quest_stamina` (capacity tiers only)

Migration: document mapping from fork `game.*` keys; optional one-time read of old config if running on fork build.

### D7 — Soft deps / sequencing with other candidates

| Other package | Interaction |
|---------------|-------------|
| #10 `postman-rental` | FA/MQ **shop rows** live there; #1 keeps combat + Config/cheat purchase until #10 |
| #3 / #4 HP / boss bar | Independent; no hard dep |
| Outfit / wardrobe tax | **Not** in #1; when outfits port, they call mod-exported service **or** hook recovery mult |
| WW / ExtNpc | None |

**Optional later:** `EXPORT_SERVICE(AlbwMeterService)` so outfits/#10 can query lockout / suppress without deep hooks. Not required for P0–P3.

### D8 — Reload / shutdown safety

| State | Rule |
|-------|------|
| Mid-guard / mid-HS / mid-Flurry | `mod_shutdown` → force end flurry, clear FA spend, restore oil draw; prefer **disable requires scene change** if hooks stay on stack |
| Save bits for FA tiers | Persist; removing mod leaves bits (document) |
| Never hook outermost main loop | SDK unload contract |

### D9 — Covenant / hygiene

No WW literals in DLL. No donor bytes in redistributable `.dusk`. Greplist when CI exists (Housing M6).

---

## 4. Work breakdown (immediate next actions)

1. **Create external mod repo** (mod-template fork) — not inside dusklight.
2. **P0 scaffold** there: `mod.json`, lifecycle, Config+Ui stubs; pin to a main commit; build `.dusk`.
3. **Host** — stock main exe (separate clone or release); drop `.dusk` into AppData `mods/`; prove Reload.
4. **Inventory freeze** — from this fork, list `dMeter2_onALBW*` / `canALBW*` call sites as the P2 checklist (docs only here).
5. **Draw spike** on stock main; then P1–P3; shield/FA/Flurry after.

---

## 5. Risks (watch list)

| Risk | Mitigation |
|------|------------|
| Oil/ALBW double economy | D2-A; audit all oil spend paths while mod enabled |
| Hook miss on inlined spend | Prefer hooks on non-inline wrappers; symdb warnings |
| Flurry time-scale ownership | P6 last among combat; document reload |
| FA save without SaveFlags | Reserved bits or mod_dir JSON; coordinate with ALBW bit map |
| Fork still compiling old paths | Feature flag `ALBW_METER_IN_ENGINE=0` when mod ships on fork builds |
| FPS regressions | Field Ordon/Faron oracle; no unload of outer loop; RelWithDebInfo `/O2` |

---

## 6. Success definition

- Stock main + `albw-meter.dusk` enabled → ALBW bar, spends, lockout, shield parry economy, FA, Flurry match fork feel for a scripted playtest matrix.
- Disable mod → vanilla oil meter restored after scene change; no crash.
- No Nintendo/WW bytes in the package.
- Fork can drop engine copies of the suite without losing the feature when the mod is installed.

---

## 7. Host API workable? (2026-07-22)

**Yes — workable for build + load + iterate.** Evidence on `origin/main`:

| Piece | State |
|-------|--------|
| Docs | `docs/modding.md` complete |
| SDK | `sdk/include/mods/**` + `cmake/ModSDK.cmake` |
| Loader | `src/dusk/mods/**`, gated `DUSK_ENABLE_CODE_MODS` |
| Presets | `windows-msvc*` sets `DUSK_ENABLE_CODE_MODS=true` |
| In-tree demos | `mods/template_mod`, `ao_mod`, `shadow_mod` → `.dusk` under `build/mods/` |
| Out-of-tree | [TwilitRealm/mod-template](https://github.com/TwilitRealm/mod-template) + FetchDusklight + auto link-stub download |
| Runtime | Drop into `%APPDATA%\TwilitRealm\Dusklight\mods`; in-game enable/Reload |

**How we can test each #1 phase**

1. **Mod project** — external repo only (not this tree).
2. **Host exe** — stock main (release or separate main clone). **Not** this fork’s exe.
3. Pin mod’s `DUSKLIGHT_VERSION` / `DUSKLIGHT_DIR` to that host.
4. Loop: build `.dusk` → AppData `mods/` → Reload → next phase.
5. **P1 playtest (current):** main exe → Mods → Reload **ALBW Meter** → human field: green stamina bar, no yellow oil; wolf: bar hidden; pause/cutscene: fade with HUD.

**Caveats**

- `FEATURES game` mods are ABI-tied to the host epoch — match mod pin to host exe.
- This fork stays reference-only for #1; no `mods/albw_meter` here.
- Official releases are the long-term target; local main build is the lab.
