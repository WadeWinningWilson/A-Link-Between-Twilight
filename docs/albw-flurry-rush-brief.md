# ALBW Flurry Rush — design brief & implementation roadmap

**Purpose:** Single source of truth for **Flurry Rush** (melee perfect-dodge rush + Back Slice aerial-bow finisher), tied to **Focused Arts** spend economy. All numeric values live in **tweakable tables** below — change numbers here first, then code constants.

> Continue Flurry Rush work — read `docs/albw-flurry-rush-brief.md` first.

**Status:** Phase 0 locked (2026-06). **Phases 1–5 + snap-to-target landed** (melee perfect-dodge rush through FA spend and lunge entry). **Phase 6–8 not complete** — see [Next steps](#next-steps) below.

**Related:** [combat-refinements-handoff.md](combat-refinements-handoff.md) (FA economy), [shield-combat.md](shield-combat.md) (parry vs dodge), [albw-junior-postman-mail-handoff.md](albw-junior-postman-mail-handoff.md) (unrelated mail track), [albw-port.md](albw-port.md) (settings overview).

---

## Next steps

Work in roadmap order unless playtest reprioritizes.

| Priority | Phase | What | Gate / done when |
|----------|-------|------|------------------|
| **Now** | **6 — Polish** | Sumo + wood entry gate (`getSumouMode()` — **blocked until sumo outfit ready**); hit connect; `setSwordModel` on rush start; post-rush parry cleanup; Master/Light cap tuning; multi-enemy interrupt; HUD hint | Melee rush shippable on all sword tiers; wood follows sumo rule |
| **Next** | **7 — Aerial bow** | Back Slice T1 finisher → hover bow at apex; replaces `s_backSliceAlbwSuppressFrames`; 2.0 real s shot start gate | T3 T1 spend Back Slice → hover → aim → shot → clean exit |
| **Then** | **8 — Rollout** | More perfect-dodge enemy families; boss/wolf exclusions; player-facing docs | Content coverage + `albw-port.md` blurb |

**Playtest polish backlog** (Phase 6 — deferrable individually): hit connect · sword not drawn · post-rush parry window · wooden + sumo (future ship rule).

**Milestones remaining:** **M2** finishes after Phase 6 · **M3** = Phase 7 · **M4** = Phase 8.

---

## Setting gate

Flurry Rush is **off by default**. Requires **both**:

| Setting | Config key | Default |
|---------|------------|---------|
| Focused Arts test | `game.focusedArtsTest` | Off |
| Flurry Rush | `game.flurryRush` *(new — add to `settings.h` + UI)* | Off |

Implementation must check a single helper, e.g. `dFlurryRush_isEnabled()` → `focusedArtsTest && flurryRush`.

---

## Shared constants (global — tweak here)

| Constant | Current value | Notes |
|----------|---------------|--------|
| **Start gate duration** | **2.0 real seconds** | **Wall-clock**, FPS-independent — not a fixed frame count |
| **Start gate (melee)** | First sword attack must **begin** within **2.0 real s**; else **end rush** (fail-close) |
| **Start gate (aerial bow)** | Bow shot must be **initiated** within **2.0 real s**; else **end rush** |
| **Rush body (after start)** | Continue until **max hits** (profile table), target death, or interrupt | Start gate timer does **not** extend; only gates entry |
| **Game time scale during rush** | **1/10** normal sim pace | Applies to **simulation / animation dt** while rush active; **reverts** when rush ends |
| **ALBW meter during rush** | **No spending** | Sword swings, HS, bow, agility — no ALBW drain for the rush sequence |
| **Lock rules** | **On** | Z-target locked on rush target; immovable until rush ends |

### Real-time timer (implementation)

The **2.0 s** window must **not** use raw frame counts (`60 f` breaks at 60/120 FPS and fights 0.1× sim scale).

| Rule | Detail |
|------|--------|
| **Clock** | `std::chrono::steady_clock` at rush `begin()` — store `deadline = now + 2.0s` |
| **Per-frame test** | `steady_clock::now() >= deadline` → fail-close if `!hasStartedAttack` |
| **FPS** | Same **2.0 real seconds** at 30, 60, or 120 FPS presentation |
| **0.1× sim scale** | Slows **game dt** only; **start gate** still measures **real elapsed time** |
| **Reference only** | At 30 Hz sim pace, 2.0 real s ≈ **60 vanilla frames of wall time** — documentation shorthand, **not** the code timer |
| **Code constant** | `kFlurryStartGateRealSeconds = 2.0f` in `d_albw_flurry_rush.cpp` (tweak here) |

Optional later: `dusk::game_clock::consume_interval(this)` if the module needs frame dt; **deadline comparison stays on `steady_clock`.**

*Future tweak:* separate `kFlurryMaxBodyRealSeconds` if the rush body should also hard-cap in real time (Phase 0: **no** — only start gate is timed).

---

## Sword profiles (tweak per column independently)

All values below are **design defaults**. Code should read from one struct/table keyed by equipped sword tier.

| Profile | Wooden | Ordon | Master Sword | Light Sword |
|---------|--------|-------|--------------|-------------|
| **FA meter fill** | None (existing) | +2/step (12 hits → 1 pip) | +1/step | +1/step |
| **Spend gate** (banked pips required to *offer* rush spend) | **None** — FA-independent | **2** | **3** | **3** |
| **Bar cost** (pips consumed when rush **fires**) | **0** — independent of FA | **2** | **3** | **3** |
| **Melee max hits** (cap during one rush) | **10** | **7** | **5** | **5** |
| **Start gate** | 2.0 real s | 2.0 real s | 2.0 real s | 2.0 real s |
| **Aerial bow start gate** | N/A unless HS finisher | 2.0 real s | 2.0 real s | 2.0 real s |

### Spend gate vs bar cost

- **Spend gate:** Minimum **banked FA charges** before a perfect dodge can **consume** FA and enter Flurry Rush (wooden ignores FA entirely).
- **Bar cost:** How many pips **`beginSpendCharge`-style** deducts when the rush **actually starts** (wooden = 0).

*Example (Ordon, tier 2 purchased, max bank 2):* gate = 2 and cost = 2 → full bank required; one rush empties bank.

*Example (Master, tier 3, max bank 3):* gate = 3 and cost = 3 → full bank required; one rush empties bank.

### Wooden sword exception

- **No FA meter**, no spend gate, **no bar cost** (when wood flurry is allowed — see sumo gate below).
- **Ship intent (future):** wooden sword **alone** does **not** enter Flurry Rush. Entry requires **both** wooden sword equipped **and** **sumo outfit** on (`getSumouMode()` / sumo wear — hook TBD in Phase 6+).
- **Playtest / now:** sumo gate **not enforced** — any valid perfect dodge with wood may enter melee rush (10 hits max; **2.0 real s** start gate) for FlurryTEST.
- Wooden does **not** receive Back Slice **T1 finisher** aerial bow unless product later adds a separate rule *(currently: finisher = FA T1 spend only)*.

---

## Spend choice (FA economy interaction)

When banked charges meet **spend gate** for the current sword:

| Player action | Consumes FA per bar cost? | Enters Flurry Rush? |
|---------------|---------------------------|---------------------|
| **Hidden Skill** (existing spend sequence) | Yes (existing FA rules) | No |
| **Perfect dodge** → Flurry Rush | Yes (bar cost table) | Yes (melee mode) |
| **Both on same full bank** | **No — not both** | One activation only |

One “full bank ready” state resolves to **either** an HS spend **or** a dodge Flurry Rush, **not** both from the same bank snapshot.

*Future tweak:* optional “partial spend” (e.g. Ordon cost 1 of 2) — **not** in Phase 0.

---

## Melee Flurry Rush (perfect dodge entry)

### Prerequisites

1. `dFlurryRush_isEnabled()`
2. **Z-target lock** on a valid enemy (`checkAttentionLock()` + enemy group)
3. **Perfect dodge** — sidestep or backflip with correct timing vs incoming **melee** (detection v1: pilot enemy family, e.g. Bokoblin)
4. **FA:** bank ≥ spend gate for sword **or** wooden exception *(wood: future = sumo outfit required; testing skips sumo check)*
5. Weapon: sword equipped (not wolf, not empty hands)

### Behavior

1. Consume **bar cost** (unless wooden)
2. Enter **`FlurryRushState`** — melee mode
3. **Game time × 0.1**; **lock** on dodge target (`keepLock` + pinned target)
4. **Snap-to-target:** after dodge land, Link **streaks** toward the lock target at fixed high speed (`kFlurrySnapSpeed`); velocity drops to **zero** at sword engage range (lynchpin distance check — not a teleport). Attack window opens when snap completes.
5. **Start gate:** first attack must **begin** within **2.0 real seconds** (wall-clock) **after snap completes**
6. If **no attack started** before gate expires → **end rush** (fail-close)
7. **After first attack:** land up to **max hits** (profile table) at 0.1× sim pace until hit cap, target death, or interrupt (*confirm Link-hit interrupt in playtest*)
8. On end: restore time scale (1.0×), release lock, clear suppress flags

### ALBW during melee rush

- **No ALBW meter spending** for the duration (same as global rush rule).

---

## Back Slice T1 finisher → Aerial bow (HS entry)

### Replaces current suppress

**Back Slice T1 finisher** (*Focused Arts tier 3 purchased, T1 spend column, `CUT_TYPE_TWIRL`*) **replaces** the current **`s_backSliceAlbwSuppressFrames` (3 s no ALBW)** behavior with **aerial bow Flurry Rush**:

| Old (v1 shipped) | New (Flurry Rush) |
|------------------|-------------------|
| 90 frames ALBW suppress after twirl | **Aerial bow window** at twirl apex |
| — | **2.0 real s** to **initiate bow shot** (same start gate as melee) |
| — | **Game sim × 0.1** while rush active |
| — | **No ALBW spending** during rush |
| — | Lock / target rules same family as melee (*target = Back Slice victim*) |

Hook: `procCutFinishJumpUp` apex + `dFocusedArts_isSpecialFinisherSpendActive()`.

Exit: shot fired + land, timer expiry without shot, target death, or interrupt → restore time, call `dFocusedArts_onBackSliceFinisherEnded()` as today for damage reset.

*Future tweak:* separate hit cap for bow mode (e.g. one shot only vs multi-arrow) — Phase 0 = **must initiate shot** within **2.0 real s**; arrow count TBD in Phase 7 playtest.

---

## Lock rules (Phase 0: Yes)

While `FlurryRushState` active:

- Force attention on rush **target actor** (id stored at `begin()`)
- `dComIfGp_getAttention()->keepLock()` refreshed while active
- Block player breaking lock (R / stick rules) unless rush ends
- Camera follows existing lock-on pipeline
- Release on rush **end** only

*Future tweak:* allow soft lock vs hard lock; boss immunity.

---

## Game time scale (Phase 0: 1/10 global)

- **Not** enemy-only suspend; **simulation dt** runs at **10%** during rush (animations, enemies, Link procs)
- **Start gate** uses **real wall-clock** (2.0 s) — **not** scaled by 0.1× *(player always has 2 real seconds to commit)*
- **Revert sim scale to 1.0** immediately when rush sequence completes

*Do not* use Postman event pause or `setPauseTimer` as the primary mechanism (`setPauseTimer` is short symmetric hitlag).

*Implementation sketch (Phase 2):* multiply `dComIfGp` / main-loop sim dt by `0.1f` while `dFlurryRush_isActive()`, or hook `dusk::game_clock` sim tick; keep `steady_clock` deadline separate.

---

## Architecture (code layout)

```
d_albw_flurry_rush.h / .cpp
  dFlurryRush_isEnabled()
  dFlurryRush_getProfile(swordTier)   // reads tables above
  dFlurryRush_canSpendOnDodge(link)
  dFlurryRush_beginMelee(target)
  dFlurryRush_beginAerialBow(target)
  dFlurryRush_update()
  dFlurryRush_isActive()
  dFlurryRush_getTimeScale()          // 0.1 or 1.0
  dFlurryRush_shouldSuppressAlbwSpend()
  dFlurryRush_end(reason)

d_focused_arts.cpp
  dFocusedArts_onPerfectDodgeSpend()  // bar cost + spend mutex with HS

d_a_alink_cut.inc
  procFlurryRushInit / procFlurryRush
  procCutFinishJumpUp → aerial bow branch

d_a_alink.cpp
  procSideStepInit / procBackJumpInit → perfect dodge query
```

Shared module serves **both** entry paths with `mode: Melee | AerialBow`.

---

## Implementation roadmap

### Phase 0 — Design lock ✅

Tables in this doc. Tweak numbers here before code.

### Phase 1 — `FlurryRushState` skeleton ✅

- Module + debug overlay lines (`ImGuiMenuTools` when FA test + Flurry Rush on)
- `dFlurryRush_update()` from Link proc only — **not** on boot path
- No player-facing rush yet; `getTimeScale()` returns `1.0` until Phase 2

**Gate:** Debug `begin()` / `end()` from FA overlay.

**Boot crash note:** Title/demo death with log `unhandled tcg src 21` / exit `0xC0000409` is **AppData GPU cache** — see [build-fps-guidelines.md § GPU cache crash](build-fps-guidelines.md#addendum-gpu-cache-crash-2026-06-26). Do not revert Phase 1 for that symptom.

### Phase 2 — Time scale + lock ✅

- Global **0.1×** time while active
- `keepLock` + target pin
- `dFlurryRush_shouldSuppressAlbwSpend()` wired into meter drains

**Gate:** Sim runs at 0.1×; start gate expires at **2.0 real s** on a stopwatch; lock holds; ALBW does not drop during debug rush.

### Phase 3 — Perfect dodge detection v1 ✅

- Bokoblin (or one family) melee telegraph + dodge axis
- Sets pending perfect dodge flag for Phase 4

**Gate:** Matrix in [testing-parry-rework.md](testing-parry-rework.md) style (vertical→sidestep, horizontal→backflip).

### Phase 4 — `procFlurryRush` melee ✅

- Wooden + debug first (no FA)
- Max hits + **2.0 real s** start-gate rule
- BotW-style intersect gates on recovery (4 Link frames @ 1.0×)
- Max i-frames during rush (`mDamageTimer` refresh)

**Gate:** 10-hit wood / 7-hit Ordon feel test on one enemy.

### Phase 5 — FA spend integration ✅

- Spend gate + bar cost table (`dFlurryRush_getProfile`)
- `dFocusedArts_canPerfectDodgeSpend()` / `dFocusedArts_onPerfectDodgeSpend()` at perfect dodge
- Wooden: gate 0 / cost 0 (FA-independent); Ordon 2/2; Master/Light 3/3
- Blocked during HS spend sequence (`s_inSpendSequence`); debug begin skips FA
- Setting gate: `game.focusedArtsTest` + `game.flurryRush` (Settings → FlurryTEST)

**Gate:** Ordon 2/2 → dodge rush → 0/2; HS path still works separately after refill.

### Snap-to-target lunge ✅ *(landed with Phase 5 playtest; listed under Phase 6 scope)*

- `kFlurryPhase_SnapToTarget` in `d_a_alink_flurry.inc` — fixed high speed toward lock target; stop at engage range
- Start gate arms on **`dFlurryRush_onSnapToTargetComplete()`**, not dodge land
- Tunables: `kFlurrySnapSpeed` (85), engage padding (sword length + radius + 50)

### Phase 6 — Sword profile polish *(in progress — next)*

- Wooden flurry **sumo outfit** gate — **future; sumo not ready** (see § Wooden sword exception)
- Master/Light 5-hit cap tuning
- Multi-enemy interrupt
- HUD hint (“Flurry Rush!”)
- Playtest polish backlog (hit feel, sword draw, post-rush parry window)

### Playtest polish backlog (Phase 6+ — not blocking M2)

Known follow-ups from FlurryTEST; safe to ship later.

| Issue | Symptom | Likely area | Priority |
|-------|---------|-------------|----------|
| **Hit connect** | Not every chain swing feels like it connects | Slow-mo hurt-sphere desync, `flurryCheckSwordHit` fallback, chain gate timing | Medium |
| **Sword not drawn** | Link can attack without visible sword if sheath at rush start | `flurryBeginSwing` / `procFlurryRushInit` missing `setSwordModel()` | Low — quick fix when wanted |
| **Post-rush parry window** | For a brief period after flurry ends, incoming attacks cannot be parried; returns to normal quickly but feels janky | Rush exit / proc handoff, invuln or guard state cleanup, enemy attack CC timing vs `dFlurryRush_end` | Medium |

*Wooden + sumo outfit* entry rule is tracked separately in § Wooden sword exception and Open knobs.

### Phase 7 — Back Slice aerial bow

- Apex branch; replaces suppress frames
- **2.0 real s** to initiate shot; bow in hover
- Finisher damage reset unchanged

**Gate:** T3 T1 spend Back Slice → hover → aim → shot → clean exit.

### Phase 8 — Rollout

- More enemy families for perfect dodge
- Boss / wolf exclusions
- Update [combat-refinements-handoff.md](combat-refinements-handoff.md) spend table
- [albw-port.md](albw-port.md) player-facing blurb

---

## Milestones

| Milestone | Phases | Ship note | Status |
|-----------|--------|-----------|--------|
| **M1 — Prototype** | 1–4 | Wood + one enemy | ✅ |
| **M2 — FA economy** | 5–6 | Full spend gates + polish | **5 done** — Phase 6 remaining |
| **M3 — Back Slice bow** | 7 | Finisher replaces suppress | Not started |
| **M4 — Content** | 8 | Enemies + docs | Not started |

---

## Open knobs (explicitly tweakable later)

| Knob | Phase 0 value | Where to change |
|------|---------------|-----------------|
| Start gate (real seconds) | 2.0 | Shared constants § |
| Sim time scale during rush | 0.1 | Shared constants § |
| Snap lunge speed | 85 (`kFlurrySnapSpeed`) | `d_a_alink_flurry.inc` |
| Snap engage padding | sword length + radius + 50 | `d_a_alink_flurry.inc` |
| Spend gate per sword | 0 / 2 / 3 | Sword profiles § |
| Bar cost per sword | 0 / 2 / 3 | Sword profiles § |
| Max melee hits | 10 / 7 / 5 | Sword profiles § |
| Wooden flurry entry | **Testing:** wood only. **Future:** wood **+ sumo outfit** (`getSumouMode()`) | Phase 6+ / `d_albw_flurry_rush.cpp` |
| Perfect dodge enemy list | Bokoblin first | Phase 3 / 8 |
| Link hit interrupts rush | TBD playtest | Phase 4 |
| Sword quick-swap cancels rush | Yes — `dFlurryRush_cancelOnSwordEquipChange()` on equip change | Phase 4 |
| Bow shot start gate | ≥1 shot initiated within 2.0 real s | Phase 7 |
| Wolf / horse / mid-air edge cases | TBD | Phase 6–7 |

---

## Risk register

| Risk | Mitigation |
|------|------------|
| Global 0.1× time breaks UI/cutscenes | Rush only in field combat procs; block begin if event running |
| Full bank consumed per rush | By design (cost = gate for Ordon/Master/Light) |
| Perfect dodge false positives | One enemy family first |
| Air bow anim pop | Snap to bow wait at apex; iterate in Phase 7 |
| Double spend HS + dodge | Mutex in `dFocusedArts_onPerfectDodgeSpend` |

---

## FA spend table cross-reference (update when implementing)

Add to [combat-refinements-handoff.md](combat-refinements-handoff.md) § Spend rules:

| Action | Spends FA (bar cost)? | Flurry Rush? |
|--------|------------------------|--------------|
| Hidden Skill (spend sequence) | Yes (existing) | No |
| Perfect dodge → Flurry Rush | Yes (profile bar cost; wood 0) | Yes |
| Back Slice T1 finisher aerial bow | Uses T1 spend column (existing) | Aerial mode, same **2.0 real s** start gate |
| Same full bank | **HS or rush, not both** | — |

---

## Playtest checklist (first pass)

- [ ] Settings: FA test **On**, Flurry Rush **On**
- [ ] Wooden: perfect dodge → rush without FA bank; up to 10 hits; **2.0 real s** start gate (stopwatch)
- [ ] Ordon: bank 2/2 → dodge → cost 2 → 0/2; HS not double-fired
- [ ] Master/Light: bank 3/3 → cost 3 → 5 hits max
- [ ] No ALBW drain during rush window
- [ ] Start gate: **2.0 real s** on stopwatch at 30 / 60 / 120 FPS (sim may look slow; gate must not drift)
- [ ] Lock stays on target until end
- [ ] Back Slice T1 finisher: apex bow, **2.0 real s** to shoot, no old 3 s suppress
- [ ] Rush disabled when `game.flurryRush` Off
