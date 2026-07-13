# ALBW Junior Postman mail — design & implementation handoff

**Purpose:** Preserve the agreed **Ordonians' Junior Postman** onboarding-mail design so new chats can implement it without reopening the long research thread.

> Continue Junior Postman / ALBW mail delivery — read `docs/albw-junior-postman-mail-handoff.md` first.

**Status:** Phase 0 proof slice **partially implemented** (North Faron deliver Postman + runtime letter slot 16). Text / spacing polish ongoing. See also **Hidden Skills scrolls** note below (research only — not implemented).

**Related:** [albw-port.md](albw-port.md), [shield-combat.md](shield-combat.md), [combat-refinements-handoff.md](combat-refinements-handoff.md), [wolf-combat-layers-research.md](wolf-combat-layers-research.md), [TrueALBWWorld.md](TrueALBWWorld.md), [hud-performance-handoff.md](hud-performance-handoff.md).

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

---

## Related note — Hidden Skills “scrolls” for ALBW combat tutorials (research only)

**Date:** 2026-07-11. **Status:** research only — **not implemented.**  
**Why here:** Same Collection UX family as Junior Postman mail (list → open parchment/scroll → read). Letters already have spare slots; Hidden Skills looked empty but is actually a different capacity story.

### Vanilla Hidden Skills tab (facts)

| Item | Value |
|------|--------|
| Menu | Collection → Hidden Skills (`d_menu_skill.cpp`, archive `skillres.arc`) |
| Visible rows | **Exactly 7** BLO panes (`let_00_n` … `let_06_n`) |
| Unlock flags | `F_0338`–`F_0344` only (Hero’s Shade secret techniques) |
| Spare list slots | **None** — all 7 are real skills when finished |
| Detail text | Single `getString()` into ~**4-line** pane (`mg_e4lin`); buffer `0x200`; **no** L/R body pages |
| List paging | **Stub only:** `mRemainder` is computed like letter page-count (`total / 7`) but **never read**; `mSkillNum` capped at 7 |

**Contrast with Letters:** `letter_data[64]` + save bitfield — vanilla uses slots **0–15**, ALBW mail uses **16+** (48 spare). Hidden Skills has no equivalent spare table.

**Do not steal `F_0345+` for new scrolls** — those are live story flags. Free N/A event bits near mail: **`F_0814`–`F_0820`** (~7). Junior Postman already uses **`F_0812` / `F_0813`**.

### Recommended approach: page 2+ of Hidden Skills (not new BLO rows)

Finish the half-built list paging (letter-menu pattern):

1. **Page 0** — vanilla’s seven Hero’s Shade scrolls (unchanged).
2. **Page 1+** — ALBW tutorial / combat-explain scrolls, same 7 visible panes reused.
3. Wire **L/R** (or stick) using dormant `mRemainder` + a page index (mirror `d_menu_letter.cpp`).
4. Optional: runtime title/body strings (Junior Postman / `initWord` precedent) so drafts don’t require BMG edits.
5. Extend `isSkillIconVisible()` to OR tutorial bits if the collect icon should appear before any golden-wolf skill.

Heavier alternative (new 8th+ BLO rows) is usually unnecessary if paging works.

Detail multipage is a **separate** follow-up: long tutorials may need letter-style `getStringPage` inside `zelda_ougi_info.blo`; short explainers can fit the 4-line pane.

### How new scrolls unlock (proposed)

Two complementary grant paths — same Collection tab, different triggers:

| Path | When | Examples |
|------|------|----------|
| **Mail / story** | Junior Postman bundle, settings snapshot, zone queue | Early ALBW intro, shield/parry pamphlets (may stay as **Letters** or also mirror as scrolls) |
| **Shop purchase** | After buying the related upgrade / service at rental Postman | Tutorial scroll appears on Hidden Skills **page 2+** once the system is owned |

**Shop-tied scroll ideas (align with planned systems):**

| Scroll topic | Purchase / system gate | Related docs |
|--------------|------------------------|--------------|
| **Focused Arts** (what tiers/bank/spend mean) | After buying **Focused Arts tier 1** (or first FA shop row) | [combat-refinements-handoff.md](combat-refinements-handoff.md) — shop already sells FA tiers 1–3 under Upgrades & Services when FA test is on |
| **Wolf Link charges** (Midna field attacks / charge economy) | After buying a **planned Wolf charge** upgrade / unlock (not shipped yet) | [wolf-combat-layers-research.md](wolf-combat-layers-research.md), [albw-port.md](albw-port.md) Wolf Link Combat |

Other combat scrolls (parry, FA spend columns, Flurry, etc.) can use the same page-2+ slots, gated by settings and/or later shop rows.

**Grant model sketch:** on successful shop buy (or mail queue), set an unused event bit (`F_0814+`) and optionally a “new / unread” cue if we add one later. `getSkillNum()` / page fill must become **entry-list aware** (not vanilla’s “count of F_0338–F_0344 fills slots 0..N”), or page 0 and page 1+ use separate tables.

### Capacity sketch

| Resource | Budget |
|----------|--------|
| List pages | Unlimited in theory; UI reuses 7 panes |
| Free event bits (first pass) | ~7 (`F_0814`–`F_0820`) without save-format growth |
| Detail length | Short = one pane; long = add letter-style body paging |

### Suggested phases (scrolls only)

| Phase | Work |
|-------|------|
| **S0** | Research ✅ (this section). |
| **S1** | Resurrect skill-list L/R paging; page 0 = vanilla; page 1 = ALBW ✅ (plumbing landed 2026-07-12). |
| **S2** | ALBW Energy Meter always first on page 2; FA I–III appear from `dFocusedArts_getPurchasedTier()` ✅. |
| **S3** | Wolf charge purchase → wolf scroll (when shop row exists). |
| **S4** | Optional detail multipage + unread markers; more scrolls as systems ship. |

**Implementation notes (S1/S2):**

- `src/d/d_albw_skill_scroll.cpp` — catalog (meter + FA tiers).
- `src/d/d_menu_skill.cpp` — L/R pages; runtime list/detail strings on ALBW page.
- FA scrolls need **no extra event bits** — visibility tracks shop purchased tiers.
- Collect icon (`isSkillIconVisible`) also ORs ALBW scroll presence so the tab shows before any Hero’s Shade skill.

### Key files (scrolls)

| File | Role |
|------|------|
| `src/d/d_menu_skill.cpp` / `include/d/d_menu_skill.h` | List/detail UI; dead `mRemainder`; hardcoded `[7]` |
| `src/d/d_menu_collect.cpp` | `isSkillIconVisible()` — F_0338–F_0344 OR |
| `src/d/d_albw_mail.*` | Precedent for runtime Collection text + unused event bits |
| `src/d/d_albw_shop.cpp` / rental purchase path | Future grant site after FA / wolf shop buys |
| `src/d/d_focused_arts.cpp` | FA tiers / `dFocusedArts_getPurchasedTier()` |
| `include/d/d_save_bit_labels.inc` | Reserve `F_0814+` for scroll gates |

### Explicit non-goals (scrolls v1)

- Replacing or renaming vanilla’s seven Hero’s Shade scrolls
- Using story flags `F_0345+` as scroll gates
- Putting full combat handbooks only in Hidden Skills without multipage detail (prefer short scroll + deep Letters if needed)
- Implementing shop-grant before FA / Wolf shop rows exist (stub bits OK; wire buys in **S3**)
