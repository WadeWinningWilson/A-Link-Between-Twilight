# Aryll — Outset

Link's little sister. First WW character to fully perform in the port (opening cutscene). This is her
index; deep procedures live in the shared recipes, chronology in the ledger.

## Aryll native-port doc set (Librarian-curated pointers)

The port's Aryll coverage is spread across several subtrees; this is the index. Pointers only — the content lives in each doc.

- [voice-recipe.md](voice-recipe.md) — opening-cutscene **voice** (LIVE; verified chain §85–93b).
- [cutscene-audio-scoping.md](../../../../cutscene-audio-scoping.md) — **§2 opening music / Outset-theme transition is LIVE**; its **§1 voice is SUPERSEDED** by `voice-recipe.md` (History ruling 2026-07-29).
- [outset-intro-layers.md](../../../../../gaming%20systems/layers/outset-intro-layers.md) — awake / walk / telescope **layer census** (verified reference).
- [outset-followup-plan.md](../../outset-followup-plan.md) — layer-activation flags + **telescope-quest** checklist (live TODO plan; status refresh pending per History).
- [Cookbook recipes 10–15](../../../../../WW-Restoration-Cookbook-CANONICAL.md) — the cutscene-NPC **playbook** (storyboard/.stb binding).
- [port-kits/HISTORY-KIT-FEEDBACK.md](../../../../port-kits/HISTORY-KIT-FEEDBACK.md) — Aryll port-kit **field notes**.
- [tale-step-in-step-hypotheses.md](../../../../../state/tale-step-in-step-hypotheses.md) — *tangential* (Grandma tale; cites the Aryll cutscene as a discriminator).

---

## Identity

| | |
|---|---|
| Census code | `Ls1` |
| Proc | `NPC_LS` |
| Socket | `NPC_HENNA0`, `socket_arg=5` (arg 5 disambiguates her from other HENNA0 islanders — Ko1=8, Ob1=7) |
| Donor actor | `daNpc_Ls1_c` — `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ls1.cpp` |
| Identity basis | ✓ decomp-sourced **and** user-confirmed (not a guess) |
| Manifest | `<mod>/npc/npc_ls.ini` |
| OBJNAME row | `OBJNAME("Ls1", fpcNm_NPC_HENNA0_e, 5)` in `d_stage.cpp` (needed for storyboard binding) |

## Assets

| Kind | File | Notes |
|---|---|---|
| Model | `Ls.arc` → `ls.bdl` | 39 textures; adapted BDL4→BMD3 |
| Companion | `lshand.bdl` | her hand (joint-slave) |
| Idle/talk anims | `ls_wait01.bck` / `ls_talk01.bck` | manifest keys (behaviour only) |
| Dialogue | `folk.ls` | |
| Cutscene arc | `Demo02.arc` (`arcs/` + `arcs_lib/`) | holds `awake.stb` + her demo anim set |

### Face (texture-plane system — see [face-expression-recipe](../../../../face-expression-recipe.md))

| Feature | Material(s) | Texture | Draw mode |
|---|---|---|---|
| Eyes | `SC_eyeL` / `SC_eyeR` | `ls_eye` (IA4) | BLEND — floating alpha plane |
| Pupil | (masked) | `ls_hitomi` | UV-offset gaze via `eye_ctrl` matAnm |
| Eyebrows | `SC_mayuL` / `SC_mayuR` | `ls_mayu` | OPAQUE cutout |
| Mouth | `SC_kuchi` | `ls_kuchi.1`–`.8` | OPAQUE, on the head mesh |

Idle blink = `maba.btp` (index 1, *mabataki*), donor `play_btp_anm` random 60–90f timer.

## Opening cutscene — `awake`

Runs end to end: donor event → merged stage list → order → archive → storyboard → camera pan →
dialogue → **cast performs**. Full pipeline in [cutscene-recipe](../../../../cutscene-recipe.md) and
[cutscene-animation-recipe](../../../../cutscene-animation-recipe.md).

**Demo expression beats** (from `§56 BEAT` decode — BTP resIDs in `Demo02`):

| resID | Expression |
|---|---|
| `0x39` | `bwait` (blink-wait / idle) |
| `0x3A` | `kyoro` (look-around) |
| `0x3B` | `kyorob` (look-around b) |
| `0x3C` | (frame 1200) |
| `0x3D` | `run` |
| `0x35`, `0x32` | (later beats) |

Facial expression + mouth swap via the **prm / `getDemoIDData`** channel (triple `(0,0,1)`: BTP
first, BTK second), resolved from the **demo arc** by resource index — not the actor's own arc.
The open mouth is `SC_kuchi` **index 18 = `ls_kuchi.3`** (C8 paletted); closed is index 19
(`ls_kuchi.4`, CMPR).

## Status

| Item | State |
|---|---|
| Bind + perform (walk, animate) | ✅ working |
| Camera pan / hand-back | ✅ working |
| Idle blink (both eyes) | ✅ working |
| Facial expressions (brows, mouth) via demo | ✅ working |
| Dialogue boxes | ✅ render; line-order + remaining lines pending (data) |
| **Open-mouth texture garbled** | ⛔ **open** — see below |
| Voice / SE (JSND track) | ⏳ **REQUIRED — step 1 of 2** (Housing owns audio; after Outset music) |
| Telescope prop (carry pose) | ✅ ported+confirmed (№218/№219 — donor handR × T×R recipe; rides storyboards) |
| Telescope state behaviors | ⏳ **REQUIRED — step 2 of 2** (user, 2026-07-22): raised look-through pose T(5.7,−17.5,−1) via the donor's m841 state + mTelescopeScale 0/1 show/hide gate. NOT optional — drop-your-ISO-in fidelity means the donor's state machine is the spec |
| Pupil gaze (`eye_ctrl`) | ⏳ not ported (fixed stare) |

### RESOLVED — garbled open mouth (№216)

Root cause was NOT Aurora's decode (proven correct) and NOT the data (byte-identical to donor):
on PC every material load emits the full correct texture bind, but the material's compiled display
list still carried compile-time texture attrs that re-executed AFTER it — wiping the format back to
whatever frame the material was compiled with. Vanilla TP never trips this (no material changes
texture format at runtime); WW's `SC_kuchi` swaps CMPR↔C8 across BTP frames. Fix: `loadTexNo`
(J3DTevs.cpp) emits nothing on TARGET_PC — `loadTexture()`'s bind stands. Fixes Tetra's brows and
every future mixed-format BTP too. BTP pin removed; `Demo02.arc` restored from backup.

### Formerly open bug — garbled open mouth (historical notes)

At the beach/look-around beat the open mouth (`ls_kuchi.3`, C8/RGB565) renders as a purple+noise
box. **Proven not our fault:** the texture is byte-identical to the donor and decodes correctly to a
pink ovular mouth (decoded PNG confirms); the palette *decode* and the *bind* are both proven correct
(`§57`: `tlutReady=1 resolved=1 handle=1`). It's an **Aurora runtime bug** in how a C8/RGB565
paletted texture is rendered *downstream* of bind — **same root cause as Tetra's eyebrows**.
Investigation live in the ledger; a swap-isolation experiment is in flight (mouth pinned to steady
C8 to bisect swap-vs-material). Backup of the pin: `Demo02.arc.pre-kuchi18-bak`.

## Ledger

Chronological detail in [cut-actors-demo-restore.md](../../../../../state/cut-actors-demo-restore.md):
- Cutscene pipeline: №150–№173
- Face/animation: №184–№197
- Mouth-texture bug: №198–№216 (RESOLVED)
- Telescope + dialogue authoring: №217–№219

## Voice

**Opening-cutscene voice: PASS — WITH CAUTION** (2026-07-22). Full chain, caution ledger, and
the reproduction recipe for other characters: [voice-recipe.md](voice-recipe.md).
