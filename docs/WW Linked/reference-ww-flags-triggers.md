# Reference — WW Flags, Triggers & Event Progression (external)

> **EXTERNAL reference from the WW speedrunning community**, not our implementation. Collected
> from [zeldaspeedruns.com/tww](https://www.zeldaspeedruns.com/tww/general-knowledge/flags-and-triggers)
> and its linked [event-flag spreadsheet](https://docs.google.com/spreadsheets/d/1JyfdZaJAe7Zv2mEMZxMt43bkJkbKhkH1woKi9Bv8slA/edit).
> Tags: **✓ verified** here · **⚠ unverified** · **⚑ directly relevant to our current work**.
>
> **Address caveat.** All memory addresses below are for the **GameCube WW binary** (JP/US). Our
> port runs on the **TP engine** with a completely different memory layout, so the *addresses* do
> not transfer. What transfers is the **flag SEMANTICS** — which event each flag represents, the
> progression order, the savewarp tiers — which is the reconstruction map for the questline.

Ledger: №195. Companion: [reference-ww-actor-memory.md](reference-ww-actor-memory.md),
[reference-ww-rendering-observations.md](reference-ww-rendering-observations.md).

---

## 1. ⚑ Animation Sets — the one that matters for cutscenes NOW

> **"There are two Animation Sets (1 and 2). Upon entering the Helmaroc fight, the game switches
> to the second Animation Set. Entering cutscenes with the WRONG set causes crashes."**

This is community-observed behaviour that maps directly onto our demo work: a cutscene expects a
specific animation set loaded, and playing it against the wrong set crashes. We have fought
cutscene/demo crashes (the `bwait`/demo-arc lifecycle). **⚠ Not yet correlated to a receiver-side
mechanism** — but when a ported cutscene crashes for no data reason, "wrong animation set loaded"
is now a hypothesis to check, not a mystery. Worth a decomp dig into how the donor gates which
demo/animation bank is resident per scene.

---

## 2. Flag families

| Family | What it controls | GC address (reference only) |
|---|---|---|
| **Event flags** | Story progression — the master flowchart. "Completely white arrows = a trigger required for another event." | array at `0x803B872C` (JP) / `0x803C522C` (US) |
| **Scene flags** | State of the currently-loaded scene | `0x803B8880` (JP) |
| **Savewarp flags** | Respawn location after save+quit — a **7-tier** system | (in the event array) |
| **KoRL flags** | Where the King of Red Lions lets you sail (gated on Wind Waker / pearls / Master Sword) | — |
| **Pirate Ship flags** | Where/when the ship appears and is boardable (day/night/Endless Night) | — |
| **Animation Sets** | Which cutscene-animation bank is loaded (§1) | — |

> **Savewarp tiers (verbatim):** *"Savewarp flags behave in tiers — the moment you trigger a
> higher-tier flag, triggering a lower-tier flag will not change your savewarp location."* Relevant
> when we build questline save/respawn behaviour: the newest-reached checkpoint wins.

---

## 3. ⚑ Outset opening / prologue event flags (questline backbone)

From the event-flag spreadsheet — the exact progression of the scene we are restoring. Addresses
are GC-reference; **use the semantics + order**.

| GC byte (offset) | Bit | Event it represents |
|---|---|---|
| `0x803B872C` (+0x00) | 0x80 | Talked to **Abe** during prologue |
| `0x803B872C` (+0x00) | 0x40 | Talked to **Mesa** during prologue |
| `0x803B872C` (+0x00) | 0x20 | Aryll **kidnapping sequence** dialogue |
| `0x803B872C` (+0x00) | 0x02 | **Outset Helmaroc & Pirates cutscene** — *set when Aryll's text pops up after the cutscene; allows getting the sword from Orca* |
| `0x803B872D` (+0x01) | 0x80 | Talked to **Sue-Belle** during prologue |
| `0x803B872D` (+0x01) | 0x08 | Started talking to **Orca** to obtain the Hero's Sword |
| `0x803B872D` (+0x01) | 0x04 | **Jumped on the first rock** during prologue |
| `0x803B872D` (+0x01) | 0x01 | **Tetra saved** in the Forest of Fairies (Outset **layer 9**, set at beginning) |
| `0x803B8750` (+0x24) | 0x01 | **Leaving Outset** cutscene / overworld pirate-ship savewarp flag (set at beginning) |

> **This is the trigger chain for our first questline.** Our opening cutscene (`awake`) is the beat
> *before* the "kidnapping sequence" (0x20) and the "Helmaroc & Pirates cutscene" (0x02). The
> "layer 9 / Tetra saved" note also confirms WW's **story-layer** mechanism (`Elst`/layer selection),
> which we already handle for env layers — the same layer system gates which actors/version of a
> scene loads. ✓ layer mechanism verified in our own lighting work; the flag-to-layer mapping is not.

---

## 4. How flags function (mechanism)

Flags are set by story events (beating a boss, obtaining an item, watching a cutscene) and checked
before later events fire — the classic gate. The KoRL table is the canonical example: each entry
lists the conditions that must hold and what becomes possible once they do. Our receiver already
has an event-flag system (`dSv_event_flag_c`, `dComIfGs_onEventBit`/`isEventBit`) and our mod adds
per-mod flags (`dExtModFlags`), so the reconstruction is **map the WW event semantics onto our own
flag storage**, never onto the GC addresses.

---

## 5. Index of the source site (for future fetches — NOT all fetched)

The site has ~150 pages; most are speedrun routes irrelevant to restoration. The **restoration-relevant**
ones, for later:

- General: Flags & Triggers (this), *Version Differences*, *Common Terms*, *Triforce Charts*,
  *Movement Mechanics*, *Damage Values*.
- Techniques: *Actor Unloading* (fetched → [reference-ww-actor-memory.md](reference-ww-actor-memory.md)).
- Overworld: *Outset Island (B7)*.
- The **event-flag spreadsheet** (full list — only the Outset subset pulled so far).

Speedrun-route, storage-glitch, and boss-strat pages are **out of scope** for the restoration and
were intentionally not fetched.

---

## №222 — Work order: the four Outset event bits, in story terms (decomp-verified)

**The variant-to-story-layer mapping IS in the donor, verbatim** — `d_com_inf_game.cpp:192-200`
(`sea` stage, Outset room): the room-layer selector is a priority ladder over exactly these bits:

| priority | bit set | resulting Outset layer |
|---|---|---|
| 1 (latest) | `0x0520` | `layer \| 4` |
| 2 | `0x0E20` | `layer \| 2` |
| 3 | `0x0101` | `9` (full override) |
| 4 | none | base layer |

Every claim below is from set/check sites; inferences are marked as such (IVAN).

### `0x0101` — first Outset arc advance ("Zl1 released from intro")
- **SET:** `d_a_tag_event.cpp:95` — trigger-tag switch case `0x2`, which simultaneously commands
  the `bk` actor (`daTag_getBk(4)->m121C = 1`).
- **CHECKED:** layer selector (→ layer 9); `d_a_npc_zl1.cpp init_ZL1_1` — while NOT set, Zl1 runs
  `demo_action1` with culling disabled (her intro-demo state).
- **Story (verified):** the beat where Zl1 leaves her intro-demo state and the island flips to
  layer 9; fired by a trigger volume that also commands the bird (`bk`) actor.
  *(Inference, unverified: the forest-rescue completion beat.)*

### `0x0E20` — second arc advance; **governs the shield** (questline-critical)
- **SET:** `d_a_tag_event.cpp:98` — trigger-tag case `0x3` (three lines below 0x0101's setter;
  a DIFFERENT tag prm, same actor).
- **CHECKED:** layer selector (→ layer|2); **`d_a_spc_item01.cpp:79` — the field SHIELD item
  REFUSES to spawn once set** (`setLoadError`); `d_a_npc_zl1.cpp init_ZL1_2` — set && !`0x2401` →
  `wait_action1`; `d_a_tag_event` action gates (case 3 → WAIT when set; case 0xA → HUNT only when set).
- **Story (verified):** the bit after which Outset moves to its second layer, the wall-shield
  field item stops existing, and Zl1 enters a shore-wait state. **This is the bit on the sword/
  shield path.** *(Inference, unverified: the bridge kidnap beat.)*

### `0x0001` — early dialogue-state bit; **setter NOT in donor actor code (open item)**
- **SET:** nowhere in `WW DP/src` — only *checked*. The setter lives outside the actor layer
  (save-init defaults, event data, or demo staff op). **Do not map it to a story beat until the
  setter is found** — that identification is the remaining work on this order.
- **CHECKED:** `d_a_npc_ji1.cpp` message trees (top-priority branch of `getMsg1stType`; the
  minigame-end path picks msg `0x96E` when set); `d_a_dk.cpp:264` — around `zelda_fly` event
  handling (case 3: if set → `field_0xAC5 = -1`).

### `0x3510` — "opening title-card reached" (file-select marker)
- **SET:** `d_menu_window.cpp:791` — inside the `awake` event handler, at message-frame `0xC8`,
  in the same breath as `setStageNameOn(0)` (the stage-name banner).
- **CHECKED:** `d_s_name.cpp:666` — file select: a save with `saveStatus != 0 && !0x3510` takes a
  special path (`field_0x1bb9 = 1`) — i.e. "this save never finished the opening".
- **Mod-flags note:** our awake restoration should set our equivalent at the same beat
  (msg-frame 0xC8) so downstream checks inherit the donor's semantics.

### Ji1 + `HOUSE_G` (the trigger context asked for)
- **Ji1's arc is `Ji`** (resource loads throughout the actor). Identity label deliberately not
  guessed (IVAN); behavioral facts below stand on their own.
- **`JA_BGM_HOUSE_G = 0x80000018`** — `0x8000_0000` base = **BGM SEQUENCE, not a stream**
  (Housing: cheap path; ExtSeq-class playback).
- **Trigger context 1 — event staff op:** `evn_sound_proc_init` (ji1:1725): the actor exposes a
  cutscene sound op driven by event-data `prm`: `0` = `bgmStop(45)` fade-out, `1` = **start
  HOUSE_G**, `2` = ending SE, `3` = vibration shock. The EVENT DATA, not actor logic, decides
  when the BGM starts inside his cutscene.
- **Trigger context 2 — minigame end:** ji1:2294 — on his minigame's end path, restart HOUSE_G,
  then choose the follow-up message by flags: `0x0520` → counted-completion messages
  (`0x997`/`0x999`), else `0x0001` → `0x96E`, else first-time `0x2F40` (sets it) → `0x951`.
- **Scene tables:** `JAIZelScene.cpp:109/209` both carry `{HOUSE_G, 0x09, 0x00}` rows — it is a
  registered scene BGM in both tables.
- **Adjacent finding (same questline):** tag_event case `0x1` sets `0x0004` and starts
  `JA_BGM_BK_FLY_DOWN` — the bird-descends music beat, two cases above the 0x0101/0x0E20 setters.
