# Aryll — opening-cutscene voice recipe

> ## ⚠ STATUS: PASS — WITH CAUTION (user-labeled, 2026-07-22)
>
> **What passed (user in-game confirmation):** every one of the right voice lines lands in the
> awake cutscene, at the right beats, **not audibly reverby** (donor law: Outset exterior RTBL
> reverb byte = 0 → bone-dry; bus §93b).
>
> **Why CAUTION, not clean PASS — the open items:**
> 1. **Vanilla-true VOLUME unverified.** She plays at "full SE level" (engine §87 choice). The
>    donor scales every sound through SoundTable volume classes (SE ≈ 127 vs BGM 60 — bus §83);
>    whether her one-shots match donor loudness relative to music has **not been measured**.
>    User: "probably an easier fix for later."
> 2. **§92 tail_test verdict never delivered** — the WAV-export playback question (local players
>    swallowing short-file tails) is unresolved. Does NOT affect in-game (engine plays raw `.aw`
>    bytes), but the diagnostic-listening toolchain keeps its question mark.
> 3. **HappyGasp unmatched** in the dayjo correlation (8/9) — one Aryll clip's index unknown.
> 4. Her **non-cutscene barks** (Hah→005, Gah→007, Worried→006, Hi→008) are identified but
>    unwired — future gameplay-dialogue work.
>
> **Do not remove the CAUTION until (1) is measured against donor and the user's ear accepts a
> volume-checked scene.**

---

## The working chain (all verified, bus §85–§93b)

| stage | fact |
|---|---|
| Trigger | message-tied: BMG entry `mInitialSound` (u8 @ +0x11) fires on message open — `d_msg.cpp:1914` |
| Her five lines | STB JMSG indices `0x357, 0x358, 0x050, 0x359, 0x35A` (INDEX into INF1, **not** mMsgNo!) |
| Cue values | 104, 105, 106, **0 (silent — donor design)**, 107 — dual-decode verified |
| Resolution | charVoiceTable @ DOL `0x8039BC30`: `unk1=0x19=25` = **AAF bank cid 25 = IsleLink_0.aw** |
| The waves | `IsleLink_0.aw` (md5-identical, 296,544 b) waves **25=Hoy1, 28=Hoy2, 26=Hiee, 27=Giggle** — NCC≥0.98 vs dayjo + 22.05 kHz + sample-length triple corroboration |
| Payload | `audio/ww_jaudio1/voice/`: bank + `ibnk_217.bin`/`wsys_217.bin` (verbatim AAF slices) + cue map in manifest — Check D green (bus §91) |
| Wiring | `dExtWw_handleDemoMessage` → `ja1Voice_onDemoMessageOpen` → shadow-wave one-shot; **cue map is package data** — no ids in the exe (§67 pattern) |
| Reverb | donor law `getReverb(room)/127` from stage RTBL (`field_0x1 & 0x7F`); Outset exterior = **0** |
| Volume | full SE level (⚠ the CAUTION item — see above) |

## Lessons banked while building this (do not re-learn)

- **Four id-space traps**: AAF cid ≠ inner id; STB msg ids are INF1 *indices*, not mMsgNo;
  port low byte ≠ wave index; right table can be resolved against the wrong bank.
- **Bank organization is scene-residency**, not scene-type: IsleLink = island vocabulary
  (cutscene + gameplay barks in one bank); CharVoice = combat/creature pool (Bokoblins, pirates).
- **Naming law** (user-decreed): exports carry WW vanilla addressing (`IsleLink_0_wave025.wav`);
  hypothesis labels never reach filenames; fan labels never reach filenames.
- **Type-6 AAF = SE concurrency limits** (4 slots/category), NOT reverb — donor polyphony budget
  for future SE work.
- The **short clips are authentically short** (WSYS sample_count == dayjo's own rips, NCC 1.0000).

## Reproducing this for the next character (the actual recipe)

1. Find the lines: STB JMSG track → INF1 **indices** → `mInitialSound` per entry (0 = silent line).
2. Resolve the bank: charVoiceTable entry → `unk1` = AAF cid; map cid → inner ids → `.aw` name.
3. Identify waves: dayjo/NCC correlation + rate/length fingerprints (both, independently).
4. Stage: bank + ibnk/wsys slices, vanilla names → Housing Check D (md5 + verbatim + per-wave).
5. Ordering-listen on vanilla-named extractions BEFORE wiring (labels ≠ proof).
6. Wire NOTHING in code: cue map rides the package; engine reads it.
7. Reverb from RTBL per room; volume per SoundTable class (measure — see CAUTION).

Cross-refs: bus §85–§93b (`ww-bridge-tool-interconnected.md`) · [audio-recipe](../../audio-recipe.md)
· [ext-seq-audio-findings](../../../../ext-seq-audio-findings.md) (the theme thread this work
deliberately never touched).
