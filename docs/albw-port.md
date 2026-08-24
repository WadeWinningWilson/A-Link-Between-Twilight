# A Link Between Twilight (PC)

**A Link Between Twilight** is a PC build of [Dusklight](https://github.com/TwilitRealm/dusklight) with an *A Link Between Worlds*–style energy meter, death item strip, Postman rental economy, and a suite of optional combat, HUD, and economy systems. All of this mod code is gated on `#if TARGET_PC`.

Clone **[A-Link-Between-Twilight](https://github.com/WadeWinningWilson/A-Link-Between-Twilight)** and follow the **[README](../README.md)** for build steps.

This is the public Twilight tree. It does **not** ship Wind Waker actor TUs or a WW layer.

## Build flag

Enable native in-game UI at configure time:

```sh
cmake --preset windows-msvc-relwithdebinfo -DTARGET_PC_NATIVE_UI=ON
cmake --build --preset windows-msvc-relwithdebinfo
```

When `TARGET_PC_NATIVE_UI` is **OFF** (default), the rental Postman still works on PC but uses **ImGui toasts** for greeting/farewell and an **ImGui shop table** for browsing items.

When **ON**, greeting, shop, and farewell use in-game BLO screens and fonts.

---

## Settings

Pause menu → **Quality of Life** → **ALBW Settings** (and related Visuals / HUD rows):

| Setting | Default | What it does |
|---------|---------|--------------|
| **Enemy HP ×** (Normal / Mid-Boss / Boss / Final Boss) | 1× each | Scales enemy durability. Spawn-time true max-HP path exists; the live `d_cc_uty` divide path is still the one that fires on every hit. |
| **No Ammo Drops** | On | Bombs, arrows, and seeds no longer drop from enemies; magic pickups replace them. |
| **Manual Shielding** | Off | Hold **ZR** to guard without Z-target lock-on; **ZR+B** shield bash. Off = vanilla auto-guard on Z-target. |
| **Shield Parry & Bash Charges** | On | Perfect-guard timing earns bash charges and ALBW meter; failed blocks cost meter and charges. Off = traditional TP guard. |
| **Parry Master** | Off | Harder parry economy (health/stamina cost). Speedrun-locked. |
| **Focused Arts** | Off | Hidden-skill charge bank and finishers, separate from vanilla hidden-skill input. |
| **Shield Durability** | Off | Shield HP by tier; failed blocks drain it. Hylian repairs on parry. Break at 0 triggers guard break. |
| **Death Recovery Orb** | Off | After Talo is rescued, dying halves your wallet and leaves a Tear of Light at the death spot to recover part of it. Item strip and meter refill on death are unaffected. |
| **Wolf Link Combat** | Off | Bite-charge Midna arts, twilight/non-twilight damage split, non-twilight stun, low-HP bite healing, charge HUD. Off = vanilla TP wolf combat. |
| **Outfit Stats** | Off | Per-outfit stat/swim/dive rules (Zora speed and dive outside Zora tunics when on). |
| **Boss Refinement** | Off | Reworked dungeon-boss fights (Diababa, Armogohma, Fyrus, Morpheel, plus sword-gate rules on Zant/Ganon). Off = vanilla scripts. |
| **Shade's Refuge** | Off | Soulslike rest points (rest/heal + warp to Ordon). **WIP.** |
| **Realtime Potions** | Off | Drink while moving instead of pausing the world timer. **WIP.** |
| **Enemy Death Rupees** | Off | Credit rupees directly to your wallet when enemies die and when boss fights end. Vanilla drop tables are unchanged. |
| **Master Quest** | Off | Postman heart and ALBW meter capacity upgrades; halved heart-container/piece grants. |
| **Extra Item Slot** | Off | **Off** / **Extra Only** (Midna on left D-pad, Z item) / **Extra + Quick Swap** (Up = cycle sword, Right = cycle shield, Down = transform; map defaults to M / Tab). |
| **Quick Equip Wheel** | Off | Live item wheel (needs Extra Item Slot on). |
| **Soulbound Red Potion** | Off | Dedicated multi-use red potion in inventory slot 11. |
| **Boss Health Bars** | Off | Named health bar for main dungeon bosses / Ganondorf. |
| **LoP HUD** | Off | Lies-of-P style HUD layout (vanilla hearts or health bar). |

Debug / editor-only rows (item skins, wolf-art test unlocks, lock-on HP overlay, True ALBW mode) stay under Level Editor → ALBW.

Design details for shield systems: **[shield-combat.md](shield-combat.md)**.

---

## Gameplay

### ALBW energy meter

- Replaces the lantern-oil HUD with an ALBW-style stamina bar for **human Link**.
- Drains on sword swings, agility actions, and hidden-skill use; meter-gated ammo HUD clamp; passive recovery when not guarding.
- **Wolf Link is unaffected** — wolf combat uses its own optional overhaul, not this meter.
- While actively guarding, passive ALBW recovery (including idle boost) is paused; parry rewards refill via explicit meter grants.
- Exhausted movement lockout and tired-idle / heavy-state live in `d_a_alink` + `d_albw_lockout.cpp`.

### Death — item strip

- On death, eligible inventory items can be **stripped** (13 rentable slots; same mapping as the ALBW design).
- Save bits track which items were ever eligible for rental.
- **Magic Armor** and **Deity Armor** use extra eligibility rules (wallet / prior armor strip).
- Meter refills on death regardless of optional economy settings.

### Death — Recovery Orb (optional)

- Gated on **Death Recovery Orb** and progress after **Talo is rescued** (event bit F_0625).
- On real player death: wallet halved (round up), Tear of Light spawns at death location when you reload that room.
- Picking up the orb returns **50% of what was lost** (see **[albw-death-recovery-orb-brief.md](albw-death-recovery-orb-brief.md)**).

### Death — Oocoo warp (Postman service)

- After a **stripped dungeon death** where you respawn in Ordon, the Postman can sell **Cuckoo's Return** — a paid warp back to that dungeon entrance.
- Not an inventory item; appears as a special shop row when eligible.

### Postman rental shop

**How to find it:**

1. Play as **human Link** (not wolf).
2. Progress until the **Ordon Village rental Postman** is active (`getBitSW() == 0x42` on the special Postman actor in F_SP103).
3. Talk to him to open **Postman's Lending Service**.

Items appear in the shop only if they were **stripped on death** and are **not currently owned**.

**Native UI** (`TARGET_PC_NATIVE_UI=ON`):

- **13 rentable items** in ascending price order (Slingshot through Deity Armor).
- **Six visible rows** with scroll; selection follows D-pad / stick.
- **Letter-select layout** (`zelda_letter_select_6menu.blo` + `select_base.blo`): icon, name (`?????` when locked), rupee price per row; parchment description on the right.
- **Rent** with A when purchasable; **Leave** with B.
- **Native dialogue:** first visit uses three greeting pages; returning customers get one page; farewell depends on whether you rented this session.

**ImGui fallback** (`TARGET_PC_NATIVE_UI=OFF`):

- `dALBWRental_imguiDraw()` — shop window in the main loop.
- `dusk::ui::push_toast()` — greeting and farewell.

**Postman actor:** custom voice SFX, optional BGM, `evtTalk()` intercept keeps Link locked during shop. Wolf Link gets a dismissal toast and no shop.

Shop footer polish (analog-stick hint, tagline) is still WIP — see **[albw-shop-icon-alignment.md](albw-shop-icon-alignment.md)**.

### Shield combat (optional)

Three independent toggles under **ALBW Settings** (see table above):

- **Manual shielding** — Dawnlight-style guard chord (hold ZR); no auto-guard from Z-target alone.
- **Shield Parry & Bash Charges** — perfect-block window, charge bank by shield tier, bash spend rules, ALBW meter rewards/penalties.
- **Shield Durability** — separate shield-HP meter and HUD; mid-boss/boss hits scale durability loss via HP category.

ALBW recovery pause while guarding is always active on PC when the mod is built in.

### Focused Arts (optional)

When **Focused Arts** is on, hidden skills use a charge bank and finisher rules instead of vanilla one-shot input. See `d_focused_arts.cpp`.

### Wolf Link combat (optional)

When **Wolf Link Combat** is on:

- Bite charges feed Midna field attacks (howl / arm / charge upgrade via Postman shop rows).
- Dedicated wolf charge HUD; deny flash when a spend is refused.
- Twilight vs non-twilight damage split, non-twilight enemy stun, low-HP bite healing.
- Wolf form remains **outside** the ALBW energy meter economy.

### Boss Refinement (optional)

When **Boss Refinement** is on, the native boss TUs consume `d_albw_boss.cpp` at the fight boundary:

- **Diababa** — late-phase hang, retaliation poison, siphon windows.
- **Armogohma** — 4% eye chip, Dominion-Rod statue count, floor phase 3.
- **Fyrus** — golem window, ablaze/hollow phases, chip on the real bar.
- **Morpheel** — eye-hook / bubble / tentacle-grab phase machine.
- **Zant / Ganondorf** — Master-Sword (or any-sword) collider gates; not a full script rewrite.

Off = the same TUs run vanilla. Boss HP HUD is a separate toggle.

### Outfit Stats, wardrobe, mail

- **Outfit Stats** — per-tunic rules (swim/dive, etc.).
- Wardrobe / cap-wear / sumo-fists visuals.
- Junior Postman mail and soulbound potion persist through save (`d_albw_mail.cpp`, `d_save.cpp`).

### Enemy Death Rupees (optional)

When **Enemy Death Rupees** is on:

- **Additive only** — vanilla enemy drop tables (hearts, jars, ground rupees) are unchanged.
- Field kills credit the wallet via per-enemy lookup tables; boss/mid-boss **fight victories** grant once per profile name per session.
- A native **"+n"** popup beside the rupee counter shows each grant (`d_albw_rupee_popup.cpp`, drawn from `d_meter2_draw.cpp`).

### Mod loader (Dusk-API)

The public tree compiles the zip/disk mod loader and host services (no funchook). Companion-mod installers live under `companion_mods/_release/`.

---

## Key source files

| Area | Files |
|------|--------|
| Rental state, catalog, input | `src/d/d_albw_rental.cpp`, `include/d/d_albw_rental.h` |
| Native shop UI | `src/d/d_albw_shop.cpp`, `include/d/d_albw_shop.h` |
| Native talk box | `src/d/d_albw_dialogue.cpp`, `include/d/d_albw_dialogue.h` |
| Shared UI text helpers | `src/d/d_albw_ui_text.cpp`, `include/d/d_albw_ui_text.h` |
| Oocoo warp service | `src/d/d_albw_oocoo.cpp`, `include/d/d_albw_oocoo.h` |
| Death Recovery Orb | `src/d/d_albw_death_rupee.cpp`, `include/d/d_albw_death_rupee.h` |
| Enemy HP multipliers | `src/d/d_albw_hp_mult.cpp`, `include/d/d_albw_hp_mult.h` |
| Shield parry / durability / bash | `src/d/d_albw_shield.cpp`, `include/d/d_albw_shield.h` |
| Movement / combat lockout | `src/d/d_albw_lockout.cpp`, `include/d/d_albw_lockout.h` |
| Focused Arts | `src/d/d_focused_arts.cpp`, `include/d/d_focused_arts.h` |
| Wolf combat + stun | `src/d/d_albw_wolf_stun.cpp`, `include/d/d_albw_wolf_stun.h` |
| Wolf charge HUD | `src/d/d_albw_wolf_charge_hud.cpp`, `include/d/d_albw_wolf_charge_hud.h` |
| Boss refinement | `src/d/d_albw_boss.cpp`, `include/d/d_albw_boss.h` |
| Boss HP HUD | `src/d/d_albw_boss_hp_hud.cpp` |
| Outfit / wardrobe / mail | `src/d/d_albw_outfit.cpp`, `src/d/d_albw_wardrobe.cpp`, `src/d/d_albw_mail.cpp` |
| Enemy Death Rupees | `src/d/d_albw_enemy_rupee.cpp`, `include/d/d_albw_enemy_rupee.h` |
| Rupee grant HUD popup | `src/d/d_albw_rupee_popup.cpp`, `include/d/d_albw_rupee_popup.h` |
| Quick equip + Ext Status host | `src/d/d_ext_mod_flags.cpp`, `include/d/d_ext_quick_equip.h` |
| ALBW meter HUD + recovery | `src/d/d_meter2.cpp`, `src/d/d_meter2_draw.cpp` |
| Collision / kill hooks | `src/d/d_cc_uty.cpp` |
| Death strip + orb hook | `src/d/d_gameover.cpp` |
| Postman hook | `src/d/actor/d_a_npc_post.cpp` |
| Link guard / meter / wolf | `src/d/actor/d_a_alink.cpp`, `d_a_alink_guard.inc`, `d_a_alink_wolf.inc` |
| Settings UI | `src/dusk/ui/settings.cpp`, `src/dusk/settings.cpp` |
| Mod loader | `src/dusk/mods/loader/`, `sdk/include/mods/` |

---

## Next on the docket

Shipped since the v0.55 / v-0.7 write-up: upstream Dusklight through **v1.4.1+**, Focused Arts, Extra Item Slot + quick-equip wheel, Boss Refinement (Diababa / Gohma / Fyrus / Morpheel + Zant/Ganon sword gates), Outfit Stats, wolf-art shop unlocks, lockout/tired-idle, Dusk-API loader, companion-mod zip.

Still open:

1. **True enemy HP multiplier** — finish migrating off the `d_cc_uty` attack-power divide so 16× is 16× HP, not early 1-damage plateaus.
2. **Shade's Refuge / realtime potions** — still marked WIP; rest/drink loop needs a playtest pass.
3. **Deku Leaf glide** — debug toggle only.
4. **Hero's Shade secret boss** — gated, default off, not a finished fight.
5. **Shop footer / stamina-upgrade icon** — analog-stick hint, tagline, Master Quest stamina row art.
6. **Final pricing pass** — Postman catalog numbers.

---

## Maintainer notes — shop footer

Footer customization (analog stick icon, tagline, “A Rent / B Leave”) is partially done. To avoid regressions:

- **Do not** hide `fenu_t6`–`fenu_t11` on `mpMenuScreen` (row prices).
- **Do not** `hidePaneTag` / subtree-hide `menu_f6`…`menu_t11` or `menu_10n` on 6menu without `suppressFifthRowFooterOverlap()` — the **5th on-screen row** (`fenu_t10`) shares layout with footer hints.
- **Do not** `hideFooterSlotTree(wi_btn_n)` (breaks wallet rupee HUD).
- Row **4** price may use `J2DPrint` overlay in `drawRowListText`; keep that path when touching draw order.

**Future goal:** scope vanilla talk suppression to the ALBW Postman only (local suppress), not a global flag in `d_msg_object`.

## Maintainer notes — vanilla dialogue suppress

- `dALBWRental_armVanillaTalkSuppress()` / `clearVanillaTalkSuppress()` guard the global message UI during Postman talk.
- **Must** clear suppress on `advanceToClosed()`, `STATE_CLOSED`, and Postman `Delete()` — otherwise **all NPC dialogue** stops rendering after one rental session.
