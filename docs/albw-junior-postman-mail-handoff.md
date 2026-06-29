# ALBW Junior Postman mail — design & implementation handoff

**Purpose:** Preserve the agreed **Ordonians' Junior Postman** onboarding-mail design so new chats can implement it without reopening the long research thread.

> Continue Junior Postman / ALBW mail delivery — read `docs/albw-junior-postman-mail-handoff.md` first.

**Status:** Design + feasibility confirmed (2026-06). **No implementation yet.**

**Related:** [albw-port.md](albw-port.md), [shield-combat.md](shield-combat.md), [TrueALBWWorld.md](TrueALBWWorld.md), [hud-performance-handoff.md](hud-performance-handoff.md).

---

## Roles (do not merge)

| Role | NPC | Where | Job |
|------|-----|-------|-----|
| **Author** | Ordonians' Junior Postman | Stationed in **Ordon** (`F_SP103`) — visible NPC optional v2 | Writes onboarding letters; **sender name** on parchment |
| **Courier** | **Regular Postman** (vanilla `TYPE_DELIVER`) | **Preset routes only** (first: North Faron) | Run-in cutscene, delivers mail, runs off |
| **Shopkeeper** | Rental Postman (`getBitSW() == 0x42`) | Ordon outside Link's house | Death rental / Oocoo / upgrades — **not** tutorial mail |

Junior Postman never needs to appear in the first delivery cutscene. The senior Postman carries a bundle **from** the Junior Postman.

---

## First delivery beat (narrative)

**When:** Early Faron arc — after speaking to imprisoned Talo (`F_0601`), before saving Talo (`F_0625`). Player is heading toward Forest Temple rescue.

**Where:** **North Faron Woods** — `F_SP108` **room 6**, near the bridge area Midna later warps back (`area flag 0x1204` — “Midna text warp to N faron for bridge”). This is **before** first Hero's Shade golden-wolf lessons (hidden-skill bit chain not started).

**Cutscene (senior Postman dialogue — custom text):**

1. Run-in (`cutDeliver` motion + Postman BGM).
2. Custom lines, e.g. “HEEEYYYYYY!!! … I nearly lost you in these woods! … It's a letter from Ordon's new Junior Postman! … Onward to mail!”
3. Letter pull (vanilla flow `0x13` → `event038`).
4. Run off; one-shot flag set.

**Result:** Multiple letters appear in **Collection → Letters** (ALBW core + optional setting pamphlets). Player continues Talo / Forest Temple uninterrupted.

**Do not** tie first mail to death location or Ordon return — death can happen anywhere; full cutscene needs a **fixed zone**.

---

## Trigger mechanics (vanilla “ring”)

Delivery is **not** “walk near Postman.” It is:

1. Hidden **`TYPE_DELIVER`** Postman in the room (`daNpc_Post_c`, spawn param `0x0100`, **not** `0x4200` rental shop).
2. Up to **four `daTag_EvtArea` type 21** tags in the same room (large XZ ellipse — the “ring”).
3. Postman binds tags in `srchActors()` via `getEvtAreaTagP(21, i)`.
4. When Link enters the ring **and** `dMeter2Info_getNewLetterNum() > 0` **and** `eventInfo.chkCondition(1)`, Postman starts `EVT_DELIVER` → `cutDeliver()`.

**Runtime spawn precedent:** rental Postman in `src/d/d_s_room.cpp` (`fopAcM_create(fpcNm_NPC_POST_e, …)`). Same pattern for deliver Postman + EvtArea tags; coords from F5 Player Info overlay.

**One-shot:** custom save flag + EvtArea off-switch / `getBitSW2()` so delivery never repeats.

---

## Letter inventory (multi-grant)

Vanilla slots **0–15** are used. ALBW mail uses **`letter_data[16…]`** (empty today).

Each slot needs:

- `mSubject`, `mName` (Junior Postman), `mText` — MSG ids
- `mEventFlag` — unused letter pending bit

**Queue:** set event bits when Link enters approach zone (snapshot settings).

**Grant:** `dMeter2Info_recieveLetter()` loops **all** pending slots in one `event038` call — multi-letter bundle is supported.

**Caveat:** `dMeter2Info_setNewLetterSender()` only names one sender when exactly one letter is pending. With a bundle, **name Junior Postman in custom delivery dialogue**; do not rely on vanilla flow `query048`.

---

## Settings-conditional pamphlets

At **queue time** (zone entry), read `dusk::settings` and set event bits only for applicable letters:

| Always queue | If setting on |
|--------------|---------------|
| Welcome / ALBW intro | Manual shielding → shield letter |
| ALBW energy meter | Shield parry → parry letter |
| Death strip + rental foreshadow | Shield durability → durability letter |
| | Quick swap, wolf combat, LoP HUD, etc. (lower priority) |

Fixed letter bodies; subset queued by flags. Snapshot at queue — no runtime MSG rewriting.

**Suggested first bundle size:** 3–4 letters in cutscene; detail lives in Collection UI.

---

## Custom dialogue — use Shade Watcher pattern (not rental shop overlay)

### Preferred: `dMsgFlow_c::initWord` inside `cutDeliver`

Shade Watcher (`src/d/actor/d_a_albw_shade_watcher.cpp`) injects English at runtime with **no BMG edit**:

```cpp
sFlow.initWord(this, "Line of dialogue…", 0xFF, 0, NULL);
// if select UI needed (usually NOT for Postman delivery):
dMsgObject_setSelectWord(0, "Rest");
dMsgObject_setSelectWordFlag(2);  // after initWord
if (sFlow.doFlow(this, NULL, 0)) { /* page done */ }
```

Uses engine flow **`0x1324`** via `dMsgFlow_c::initWord()` in `src/d/d_msg_flow.cpp`.

**Hook point:** `daNpc_Post_c::cutDeliver()` **case 5** today calls `initTalk(0x14, NULL)` (generic). For ALBW bundle delivery, branch on spawn param / `getBitSW()` and chain **`initWord` pages** instead.

**Keep case 8:** `mFlow.init(NULL, 0x13, 0, NULL)` for letter pull + fanfare + `event038`.

### Do not use for cutscene: `dALBWDialogue_c`

Rental Postman shop uses `dALBWDialogue_c` (`src/d/d_albw_dialogue.cpp`) for greet/farewell **outside** vanilla `talkProc` — avoids BLO table corruption. That path is wrong for **`cutDeliver`** where case 5 already runs through `talkProc` / `mFlow`.

---

## Safe implementation checklist (Shade Watcher lessons)

| Rule | Why |
|------|-----|
| **`setSelectWordFlag(0)` after any select `doFlow`** | Stale select state broke game-over Yes/No (Shade Watcher bugfix comment) |
| **Set select words only after `initWord`** | `initWord` clears select buffers |
| **Delivery bundle: no select UI** | Avoids cross-talk with save menu / other message flows |
| **Preload Postman voice arc at custom spawn** | Already done for rental Postman (`ALBW_POST_SFX` in `d_a_npc_post.cpp`) — repeat for deliver spawn |
| **Separate `getBitSW()` from rental `0x42`** | Shop talk intercept must not hijack deliver actor |
| **TRUETEST fallback trigger** | TRUETEST skips Talo quest; use alternate first-field / Ordon trigger if `F_0601` never fires |
| **Measure coords in room 6** | Type 21 scale is ×1000 on XZ — ring size matters |

---

## Quest / story gates

| Flag | Meaning |
|------|---------|
| `F_0601` | Spoke to imprisoned Talo — **on** for first delivery |
| `F_0625` | Saved Talo — rental Postman unlock uses this (separate system) |
| Custom `ALBW_MAIL_DELIVERED` | Reserve bit or unused event — one-shot off |

**North Faron path note:** direct Talo → Forest Temple route may skip room 6. Playtest from imprisoned Talo to `D_MN01`; add south approach ring if needed.

---

## Suggested implementation phases

### Phase 0 — Proof slice

- One letter slot (16), one event bit, one MSG triplet
- `F_SP108` room 6: deliver Postman + one type-21 EvtArea
- `cutDeliver` case 5: two `initWord` lines; case 8 unchanged
- Gate: `F_0601 && !delivered`

### Phase 1 — Full bundle

- Letter slots 16–22, settings queue helper
- Full Junior Postman copy (Collection bodies)
- One-shot + pending-bit cleanup

### Phase 2 — Polish

- Junior Postman visible NPC in Ordon (talk stub)
- TRUETEST alternate trigger
- Optional toast: “Mail is waiting in North Faron”

---

## Key files

| File | Role |
|------|------|
| `src/d/actor/d_a_npc_post.cpp` | `TYPE_DELIVER`, `cutDeliver`, EvtArea wait loop, rental intercept |
| `src/d/actor/d_a_tag_evtarea.cpp` | Type 21 area shape |
| `src/d/d_meter2_info.cpp` | `letter_data[]`, `getNewLetterNum`, `recieveLetter` |
| `src/d/d_msg_flow.cpp` | `initWord`, `event038` |
| `src/d/actor/d_a_albw_shade_watcher.cpp` | **`initWord` reference implementation** |
| `src/d/d_s_room.cpp` | Runtime actor spawn pattern |
| `include/dusk/settings.h` | Settings gates for conditional letters |
| `include/dusk/map_loader_definitions.h` | North Faron = `F_SP108` room 6 |

---

## Explicit non-goals (v1)

- Delivery at death site
- Per-letter custom Postman spoken lines (one delivery script is enough)
- Replacing Collection letter reader UI
- Using rental Postman (`0x42`) as courier
