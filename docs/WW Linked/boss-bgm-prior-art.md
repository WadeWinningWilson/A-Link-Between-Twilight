# Boss BGM channel-masking — prior art (FOR FUTURE USE)

**Status: NOT SCOPED. No code, no ask, no lane assignment active.** This is donor-side knowledge
captured so it is not re-derived when a **boss BGM** restoration lane opens (Jalhalla, Gohma, etc.).

**Why it is filed separately from the live audio hunt:** this mechanism does **not** apply to the
current field/interior tracks (`i_link`, `house`). That was verified, not assumed — see
[ext-seq-audio-findings.md §D](ext-seq-audio-findings.md). Boss music uses a dynamic per-channel
mute layer that field music does not; keeping it out of the live doc prevents it from being chased as
a cause of the field-music symptom (it isn't one).

> **Covenant note (Housing Security):** everything here is **WW donor knowledge** — mask values,
> sequence names, actor behaviour. It lives in this donor-side doc and must **never** be hardcoded
> into the receiver exe. When this lane opens, masks and BGM-ID tables belong in the mod package
> (`audio/`), resolved at runtime like every other WW asset — not baked into engine source.

---

## The mechanism

WW controls boss-music layers by **muting/unmuting sequence channels at gameplay events**, from a
per-BGM channel-mask table — a layer that lives **above** the BMS note stream.

- **Apply path:** `JAIZelBasic::bgmMute` (`WW DP/src/JAZelAudio/JAIZelBasic.cpp:1554`), driven by the
  boss family: `mbossBgmMuteProcess` (802A5818), `bgmMuteMtDragon`, and calls like
  `bgmMute(&mpSubBgmSound, JA_BGM_SEA_ENEMY, on/off, fade)`.
- **Trigger:** the **boss actor** at a story beat. Community-traced example: `torituki_execute` in
  `d_a_bpw.cpp` (Jalhalla's possession event) mutes the xylophone channel and unmutes the
  glockenspiel channel.
- **Unmatched:** `JAIZelBasic::bgmStart` is **not yet matched** in the WW decomp (community note), so
  the exact point where the *initial* mask is applied to a starting sequence needs its own reversing
  pass. This matters — it is where a boss BGM's default channel state is set.

## The mask format (community-supplied, US version)

Table cited at US `0x8039bab0`; per-BGM-ID entries, 16-channel bitmasks (bit set = channel on):

| BGM id | mask | effect |
|---|---|---|
| `JA_BGM_BIG_POW` (`bigpow.bms`) | `0xFFFFFFDF` | channel **6** disabled |
| `JA_BGM_UNK_140` | `0xFFFFFBFF` | channel **11** disabled |

Jalhalla specifics: intro cutscene music `JA_STRM_BPW_START` = **stream** (`Audiores/Stream/bp_start.afc`);
battle BGM `JA_BGM_BIG_POW` / `JA_BGM_UNK_140` = **sequence** (`JaiSeqs.arc/bigpow.bms`). "BPW" = the
internal name for Jalhalla ("boss pow"). Mute settings handled by `JAIZelBasic::bgmMute`.

**External reference:** LagoLunatic WW-Hacking-Docs — `Extracted Data/BGM Sequences.txt` (per-sequence
mask values). Fold into Bridge's bank/seq mapping when this lane is scoped.

**Decomp source for the mechanism — zeldaret/tww PR #1132** (draft, partly incorrect — verify, do not
trust wholesale). Matches the audio-framework functions that are `/* Nonmatching */` in our WW DP
checkout: `mbossBgmMuteProcess`, `bgmStart`, `subBgmStart`, `bgmNowBattle`, `bgmBattleGFrame`. These
are the boss/battle layer-control functions this lane needs. The layer-fade primitive is
`JAISound::setTrackVolume(line, vol, fade)` → `SeqParameter::mTrackVolumes[line]` — an outer per-line
volume the framework drives outside the BMS stream (e.g. `bgmNowKaitengiri` fades the spin-attack
track in). **Our ExtSeq port has no `mTrackVolumes` layer at all** (`composedVolume()` is BMS×parent
only), so this outer-volume mechanism must be built for boss layer-fades to work.

> **Confirmed boss-only (Engine, 2026-07-21, from PR #1132's matched `bgmStart`):** field/overworld
> BGM sets **no** per-line outer track volumes — every `setTrackVolume*` lives in battle/subBGM/mboss
> paths. So this missing layer is **harmless for field music** and belongs squarely to *this* boss
> lane, not the live field-music hunt. It was raised and dismissed as a field-music candidate in
> [ext-seq-audio-findings.md §C.2](ext-seq-audio-findings.md) — the record of that test lives there.

---

## What our port lacks (the build cost, when it comes)

Our ExtSeq player has **no channel-mask layer at all**:

- `Ja1Track::OuterParam::setOuterSwitch(u16) {}` is an **empty stub** (`include/d/ext_seq/ja1_track.h:126`).
  WW's `cmdOutSwitch` (opcode `0xDB`) drives track output routing through this.
- The channel-effect commands are nop in playback: `outSwitch` (0xDB), `volumeMode` (0xF3),
  IIR/FIR/EXT, pan-pow, envelopes. ~52 of 64 commands are unimplemented (harmless for field music,
  required for boss music).
- There is no equivalent of `JAIZelBasic::bgmMute` — no runtime channel enable/disable at all.

**So a boss BGM lane needs, roughly:** (1) a per-BGM channel-mask input in the mod package; (2) a
runtime channel enable/disable in the track/channel layer (implement `setOuterSwitch` + a mute mask);
(3) a way for a ported boss actor to toggle channels at an event (the `torituki_execute` analogue).
None of this exists; all of it is donor-faithful and specifiable from the references above.

---

## When to pick this up

Only when a boss with dynamic music layers is actually being restored. Until then this is a filed
reference, not a task. Cross-links: [ext-seq-audio-findings.md](ext-seq-audio-findings.md) (why it is
not the field-music bug), [islands/Outset/audio-recipe.md](islands/Outset/audio-recipe.md) (the
per-space porting procedure this would extend).
