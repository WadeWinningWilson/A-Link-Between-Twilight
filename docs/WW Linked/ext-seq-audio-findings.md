# ExtSeq audio — diagnosis state (CURRENTLY APPLICABLE)

**Scope:** the JAudio1 ExtSeq sequence player that drives WW field/interior BGM on the receiver —
`src/d/d_ext_seq_space.cpp`, `src/d/ext_seq/ja1_parser.cpp`, `ja1_track.cpp`, `ja1_bank.cpp`.
Live test tracks: **`i_link`** (Outset exterior) and **`house`** (Grandma's interior).

This is the **active bug-hunt state**, not a procedure — for *how to port* a space's audio see
[islands/Outset/audio-recipe.md](islands/Outset/audio-recipe.md). Boss-BGM channel masking is a
separate future concern — see [boss-bgm-prior-art.md](boss-bgm-prior-art.md).

> **Method rule earned this session (№31-B / №31-C in practice):** verify at the stage the player
> perceives, not the stage easiest to hash; and an unchecked inference is not a result. Seven
> hypotheses died on this problem — four earlier, plus DAC-rate, frame-clock, and stream-desync.
> Every claim below cites the file/line or the measured data that proves it. Causal claims are
> Engine's to confirm before anything is fixed toward them.

---

## The two reported symptoms (user, live)

1. **Timing** — "some parts arrive at the wrong time / sooner than they should."
2. **Balance** — "some layers sound louder than they should be."
3. "…or it's one of those and it's impacting how I hear the other."

**These are now believed INDEPENDENT.** The mechanism that would have coupled them (a stream desync)
is ruled out — see §D. Timing had one confirmed cause (§B, fixed); balance has one surviving target
(§C, open).

---

## §A — SUPPLY IS PROVEN PRISTINE (closed)

The content lane is **eliminated** from this investigation. Every stage from donor bytes to decoded
event stream verifies:

| stage | check | result |
|---|---|---|
| bank `.aw` files | md5 vs donor `Audiores\Banks\` | **byte-identical** (`n2i_link_0` `bf702fb8…`, `n_zelda_0` `6dcbf41a…`) |
| IBNK/WSYS slices | verbatim sub-range search in donor `JaiInit.aaf` | **all 4 exact, unique offsets** |
| instrument volume | Bridge `ibnk-vol-check`, two independent readers | **genuine `1.0` on every INST** (43 total) — not a decode fallback |
| event stream | engine dump vs Bridge decode | **byte-identical** (`i_link` 5215, `house` 1978) |

**Consequence:** every remaining defect is receiver-side — tick→time conversion, or `set_param`
application. Nothing to fix in the donor data or the packaging.

---

## §B — TIMING: integer tick truncation (CONFIRMED, Engine fixed)

`d_ext_seq_space.cpp` `tickOwned()` advanced the sequence by an **integer** tick count per frame with
no fractional carry:

```cpp
steps = static_cast<u32>((tempo * timebase) / 1800);   // integer division, remainder discarded
```

Measured against the shipped `tempo_map.csv`:

| bms | tempo | timebase | true ticks/frame | played | error |
|---|---:|---:|---:|---:|---:|
| `i_link` | 119 | 120 | 7.933 | 7 | **−11.8 %** |
| `house` | 146 | 120 | 9.733 | 9 | **−7.5 %** |

- The diagnostic block at `:243-266` computed the **correct `f32`** rate and logged it; playback used
  the truncated int. *The rate reported was not the rate played* — №31-C in live code.
- Independent of frame rate (identical at a perfect 60 fps), so the earlier "high FPS plays it fast"
  refutation never touched it.
- **Fix (Engine):** fixed-point remainder carry on the same `/1800` target; `[1,48]` clamps retained
  with first-hit LO/HI warns. **Not** a DAC-rate change (that hypothesis is dead, §58).

**Status: fixed, awaiting user re-test** (Outset + Grandma's, tempo/feel).

---

## §C — BALANCE: `set_param` target visibility (§C.1 SHIPPED 2026-07-22)

`set_param` is the second-largest event class (1140 in i_link, 402 in house) and, after §A and §D,
the **only** remaining BMS mechanism that can shape per-layer balance for these tracks.

**Verified correct (against the donor, not assumed):**
- Dispatch + application present: `0x9x` → `cmdSetParam` → `track->setParam(flag, data/32767.0f, val)`.
- `cmdSetParam` is **byte-faithful** to `JASSeqParser.cpp:751-793` (the `case 4` raw-byte read that
  looked like a scaling bug is WW's own behaviour; also unreachable for these songs).
- `TIMED_PARAMS = 18` matches the donor `TTrack` layout exactly (no bound mismatch).

### §C.1 — [SHIPPED 2026-07-22] dump emit + `:252` warn

**Was:** dump emitted opcode width-mode and discarded `flag`; `:252` OOB drop was silent.

**Now (dump/log only — no behaviour change):**
1. `ja1_parser.cpp` `cmdSetParam` emits Bridge 0.21.0 columns — `note_param`=target (`flag`),
   `velocity`=`data/32767.0` at 6 dp (empty when reg-indirect).
2. `ja1_track.cpp:252` first-hit `DuskLog.warn` when `target >= TIMED_PARAMS` (still no-ops the write).

**Re-dump / diff (`--extseq-dump` → `seq_events_engine_{i_link,house}.csv` vs Bridge goldens):**
| stem | events | set_param | target hist (engine) | OOB ≥18 | row diffs |
|---|---|---|---|---|---|
| i_link | 5215 | 1140 | 0:1080, 1:12, 2:24, 3:12, 4:12 | **none** | 18 — velocity ±1 ULP only (`0.543291` vs `0.543290`) |
| house | 1978 | 402 | 0:372, 1:6, 2:12, 3:6, 4:6 | **none** | 5 — same ULP class |

Schema/tick/track/event/target **match**. No `:252` warn fired on dump (no OOB targets in these BMS).
~95% of `set_param` is target **0** (volume). ULP drift is printf rounding, not a decode miss.

**Offline ramp companion (2026-07-22):** dump also writes `seq_vol_ramps_<stem>.csv`
(`tick,track_id,target,value,move_time`) — golden 5-column schema unchanged. Tool:
`tools/extseq_vol_ramp_hist.py`.

| stem | vol rows | snap (move≤0) | timed ramp (move>0) |
|---|---|---|---|
| i_link | 1080 | **1080** | **0** |
| house | 372 | **372** | **0** |

Every volume write uses `moveTime=-1` (opcode time-width 0 → WW snap). **"Layers louder" is not a
broken timed-ramp** in these BMS — it is sustained absolute levels / composition.

**Live absolute-level thread (input still required):**
- Env: `DUSK_EXTSEQ_VOL_PROBE=1` (launcher: `run_extseq_vol_probe.bat`)
- Logs: `setParam` raw→ + `afterSnap composed`, `noteOn` chVol/composed, tree walk
  (`post-start` + every ~1s for 15s then every 10s) with raw/tgt/composed/mode/voices per
  active track
- Capture against **confirmed Outset `i_link`** (History: opening has no stream — stage theme
  is the correct song). Grep log for `§C.1 volProbe`.

**Offline absolute map** (`tools/extseq_vol_abs_levels.py` on `seq_vol_ramps_*.csv`): per-track
raw max/uniques + early last → mode-0 composed estimate (child² × root²).

**Status: instrumentation ready; waiting on a live `VOL_PROBE=1` play of confirmed `i_link`.**

### §C.3 — [FOUND 2026-07-22, bus §76] the parent-volume cascade is OURS, not WW's

Probe capture (549 lines, against confirmed `i_linkin`) + donor source, both verified:

- **Donor** (`JASTrack.cpp:530-533` and `:637-644`): channel volume = **own track volume²**
  (mode 0) × outer param. **No parent factor anywhere** — `mParent` is init-inheritance only.
- **Ours** (`ja1_track.cpp:283`): `composedVolume() = own² × parent.composedVolume()` — a
  recursive cascade that is a **port invention**. Probe proves it live on every note:
  `chVol 0.3836 = 0.8740² × 0.7086²` exact.
- **Measured warp:** root raw `1.0 → 0.0 → 0.7086 → 0.9606` ⇒ our child multiplier
  `1.0 → 0.0 → 0.5021 → 0.9228`; donor's is constant 1.0. All 23 captured notes at **ratio
  0.502** of donor level; the root=0 window silences entrances outright. The BMS writes root
  volume freely because in WW those writes are **inert for children** (root has no voices).
- Coherent with every symptom: swallowed entrances ("arrives late"), ~2× swell across the early
  song, per-note level depending on trigger time, dense passages worst, defect follows the player.

**FIX (Engine, SHIPPED 2026-07-22):** `composedVolume() = own²` only — parent recursion
removed; mirrors `JASTrack.cpp:530`. Outer-param volume switch still unwired (unused for
field BGM / §C.2). Acceptance: probe re-run (`chVol == raw²` regardless of root) + user ear
on `i_linkin`. §66 covered WRITE; this was the READ side — №31-B.

### §C.4 — key-region wave selection (NEW primary thread, 2026-07-22)

wav-compare Headline 3: DL attacks sit in wrong frequency bands (1–2.5 kHz ↑, 2.5–6 kHz ↓) —
candidate = wrong wave/register, not volume. **Static table audit SHIPPED:**

- Engine writes `seq_key_regions_engine.csv` on package load / `--extseq-dump`
- `tools/extseq_key_region_audit.py` diffs vs Bridge `ibnk_initvol.csv`
- **Result: MATCH** (111/111 rows; engine slot 1 ≡ WW bank 21)

Lookup algorithm already matches donor `TBasicInst::getParam` (`key <= highKey`, then
`vel <= maxVel`). **Tables are not the bug.** Primary thread continues on the **runtime** path:
which `(bank,prog)` the BMS selects, `setKey(key - baseKey)`, init pitch / sample-rate, live
`DUSK_EXTSEQ_KEY_AUDIT=1` noteOn lines.

### §B addendum — 1.3% residual tempo query (2026-07-22)

Loop-period A/B (alignment-immune): DL ≈ **1.3% slow** vs WW after the §B remainder-carry fix
(pre-fix was −11.8%). `/1800` formula unchanged pending evidence. Engine now logs
`[ExtSeq] §B tempoProbe` every ~10s: `wall_ticks/s` vs `target@60fps` and implied frame rate.
Hypotheses: (1) game poll below 60 Hz, (2) Dolphin not independently clocked, (3) small formula
vs DAC-subframe wall clock. **Do not retune `/1800` until tempoProbe + a controlled capture agree.**


### §C.2 — [RESOLVED — DISMISSED for field music, 2026-07-21] outer per-line track-volume layer

> **RESOLUTION (Engine, from PR #1132's matched `bgmStart`, reference-only):** a plain field BGM
> does **NOT** initialise per-line outer track volumes. `bgmStart` contains **zero**
> `setTrackVolume`/`setTrackVolumeU7` calls; the field start path is `startSoundVec` + an optional
> *sequence-level* `setVolume(calcMainBgmVol())`; Outset-style IDs (`JA_BGM_ISLAND_LINK*`) only set
> an internal flag (`field_0x1f3c`). Every `setTrackVolume*` lives in battle/subBGM paths.
> **Therefore the missing `mTrackVolumes` factor is a HARMLESS gap for field music** (like the
> channel-mask gap in §D) and moves to the boss-lane build list —
> [boss-bgm-prior-art.md](boss-bgm-prior-art.md). **§C.1 stays the sole "layers louder" target.**
>
> *(The `setVolume(calcMainBgmVol())` on the field path is a **uniform** main-BGM scale — it moves
> all layers together and so cannot produce "some layers louder than others." Noted for
> completeness; not a target. Whether our port applies an equivalent master scale is an absolute-
> loudness question, not a per-layer-balance one — out of scope for this symptom.)*

The candidate below is preserved as the record of how it was raised and tested. **Do not act on it
for field music.**

_Original candidate (raised 2026-07-21):_ the outer per-line track-volume layer our port lacks

Surfaced via zeldaret/tww **PR #1132** (draft — take with salt). WW carries **two** volume layers:

1. the BMS `set_param` volume our port applies (§C above), and
2. an **outer per-line track-volume array** — `JAISound::setTrackVolume(line, vol, fade)` writes
   `SeqParameter::mTrackVolumes[line]` (`JAISound.cpp:401`), a separate multiplier the *game code*
   drives outside the BMS stream.

**Confirmed structural gap:** our `Ja1Track::composedVolume()` (`ja1_track.cpp:283`) is only
`BMS_volume² × parent.composedVolume()` — **there is no `mTrackVolumes[line]` factor anywhere in the
port** (grep returns nothing). If WW starts any field track at a non-default outer volume, our port
plays it at full and that track is louder — *exactly* the symptom, and **invisible in the
byte-identical event stream** because it lives above the BMS (consistent with §A pristine + "we've
seen nothing").

**Why it is a candidate, not a conclusion:** whether field music (`i_link`/`house`) actually sets
non-default outer track volumes is **UNVERIFIED**. The `setTrackVolume` calls found in the decomp are
all **battle/boss-scoped** (`bgmNowKaitengiri`, `mbossBgmNearByProcess`). The function that would
settle it is **`bgmStart` / `subBgmStart`** — and **both are `/* Nonmatching */` stubs in our WW DP
checkout**, so we cannot currently read whether a plain field BGM initialises outer volumes.

**This is precisely what PR #1132 provides:** it matches `bgmStart`, `subBgmStart`, `bgmNowBattle`,
`bgmBattleGFrame`, `mbossBgmMuteProcess` — the exact functions stubbed in our checkout.

**ENGINE ASK (open, research):** from PR #1132's matched `bgmStart`/`subBgmStart` (as *reference to
verify*, not authority — draft, partly wrong), determine whether a non-battle field BGM sets
per-line outer track volumes. **If yes** → the port needs an outer `mTrackVolumes` layer folded into
`composedVolume()`, and this is a strong "layers louder" cause. **If no** → the gap is real but
harmless for field music (like the channel-mask gap in §D), and §C.1 remains the sole target.
**Do not build the outer layer until this yes/no is answered** — eighth-hypothesis discipline.

---

## §D — WHAT WAS RULED OUT (so it is not re-chased)

- **Stream desync — DEAD, with proof.** Our port implements ~12 of WW's 64 commands directly and
  nops the rest, **but** mirrors WW's `Arglist` via a 64-entry `kArgCount`/`kArgFmt` table
  (`ja1_parser.cpp:400`), so nop'd commands still consume the correct arg bytes. Proven end-to-end
  by §A's byte-identical stream (a wrong length would have desynced Bridge's diff). This is the
  mechanism that would have *coupled* the two symptoms; its death is why they are treated as
  independent.
- **Channel mute masks — NOT PRESENT in these tracks.** The only exotic opcodes in i_link/house are
  `E6`=`cmdVibDepthMidi`, `E7`=`cmdSyncCPU`, `F4`=`cmdVibPitch` (vibrato + sync). **No `outSwitch`
  (0xDB), no `volumeMode` (0xF3), no envelope, no mask.** The `bgmMute` channel-mask mechanism is
  boss-only — see [boss-bgm-prior-art.md](boss-bgm-prior-art.md). Dropping vibrato makes notes less
  expressive, **not louder** — it cannot be the balance symptom.
- **DAC-rate (§58), frame-clock, TP-mixer interference, velocity-exponent** — all previously killed
  by capture/measurement.

**Known fidelity gaps that are real but NOT implicated here** (they matter for richer/boss BGM later,
not for i_link/house): `setOuterSwitch(u16) {}` is an empty stub (`ja1_track.h:126`); ~52/64 commands
nop in playback. For these two tracks the only nop'd commands actually present are vibrato and sync.

---

## §E — VARIANT CAVEAT on all listening tests (added 2026-07-22, bus §72)

The donor **selects among multiple arrangements per space by story state**: Outset = 4 variants + 2
silence states via `setScene` event bits (**fresh game plays `JA_BGM_ISLAND_LINK_0` 0x80000038**, not
our packaged base `0x80000001`); Grandma's house has `HOUSE_G` (0x80000018, actor-started) beside
base `HOUSE`; island themes are **day-gated** (`startIsleBgm` → `checkDayTime`).

**Consequence for this doc:** every "sounds off" judgement to date compared our playback of the
**base** arrangements against ear/footage memory of whatever variant the real game plays in that
story state. §B (truncation) and §C (instrumentation) findings are measured facts and stand — but
the **residual** perceptual wrongness cannot be attributed to the player until the *correct variant*
is packaged and compared. **Do not tune the player against the wrong song.** Bridge id→BMS mapping +
`_0` extraction is the unblock (bus §72).

> **§E RESOLVED (2026-07-22, bus §75):** the correct variant (`i_linkin.bms`, ear-confirmed as the
> footage song) was played through our player — **and the symptoms persist in its theme portion:
> same layering wrongness, same inter-instrument timing offset, across two different BMS files.**
> The defect follows the player, not the file. Tuning against `i_linkin` is now tuning against the
> right song. Sharpened observables for the live thread: (a) inter-INSTRUMENT relative timing is
> off, not just overall tempo; (b) symptoms concentrate in dense passages — the medley's simple
> intro fared best; (c) user caveat retained: symptoms may color each other's perception.
> Current target unchanged: absolute levels / `composedVolume` (`DUSK_EXTSEQ_VOL_PROBE=1`).

---

## Current status board

| item | state |
|---|---|
| supply pristine (§A) | **closed** |
| timing truncation (§B) | **fixed; 1.3% residual open — `§B tempoProbe` logs wall rate** |
| parent-volume cascade (§C.3 / §76) | **SHIPPED — `composedVolume` = own² only** |
| key-region static table (§C.4) | **MATCH vs Bridge; runtime path secondary** |
| ExtSeq reverb / FX (§81) | **SHIPPED — type-7 → freeverb + AutoMixer; expect `fxProbe … WET`** |
| SoundTable BGM master (§81) | **SHIPPED — `vol_u8/127` on `composedVolume` (~0.472 Outset)** |
| outer track-volume layer (§C.2) | **dismissed for field** |
| ExtSeq bus transition hook | **shipped** |
| desync / channel-mask (§D) | **ruled out for these tracks** |
| runtime key/`setKey` | **paused** (§82) until post-FX ear |

**Next concrete step:** (1) user A/B + ear on F_DL01 after §81; (2) §76 cascade unconfounded;
(3) `§B tempoProbe` on a long sit; (4) runtime key/`setKey` stays paused until then.

### §81 — ExtSeq reverb + SoundTable vol: **SHIPPED (PC approx)**

Pre-port measurement (kept for history): BMS fxmix all 0; `setInitFxmix(0)`; mixConfig
`0x150` → no AutoMixer; DuskDsp never saw ExtSeq → **DRY**.

Port (donor law, package `control/` CSVs from Bridge 0.25.0):

| piece | path |
|---|---|
| type-7 scene 0/1 | `aaf_fx_scenes.csv` → max \|bus\|/32768 send, buf→room, coef[6]→damp → `applyExtSeqFxScene` |
| AutoMixer feed | `mixConfig[0]=0xffff` + `composedFxmix` = timed-sum + scene send |
| SoundTable master | `aaf_soundtable_bgm.csv` `vol_over_127` → `setExtSeqMasterVol` × `composedVolume` |
| stage→scene | F_DL* = scene 0, R_DL* = scene 1 |

Expect logs: `§81 loaded type-7 FX scenes`, `§81 applyFxScene … send≈0.5`,
`§81 SoundTable masterVol=0.4724`, `§81 fxProbe … WET`. Runtime chases remain paused.
