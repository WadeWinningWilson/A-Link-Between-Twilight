# Message-voice subsystem decode — the native path behind the ja1Voice bridge (Foundry §263)

**What this is:** the owed native-subsystem decode from bus §250 ("JA_SE_CV_* SE-system
voice"). The donor's NPC talk-voice turns out NOT to be per-actor `JA_SE_CV_*` calls at
all — neither ls1 nor bm1 fires one. It is a **message-driven dispatcher**: the BMG
message entry carries a sound id; the message window plays it through ONE universal
JAudio sound whose sequence reads the voice selector from a **sequence port**. Decoded
donor-verbatim from the binary (same bar as the getP_BtpData trio, §251/§252).

## The donor chain (every link receipted)

1. **BMG message entry field `mInitialSound`** — per-message sound id (u16, < 0x118).
2. **Window-open fire site:** `d_mesg.cpp:2007` —
   `if (entry.mInitialSound) mDoAud_messageSePlay(entry.mInitialSound, NULL,
   dComIfGp_getReverb(stayNo));` (close-sound variant in `d_msg.cpp:1914`).
3. **`JAIZelBasic::messageSePlay(u16 id, Vec* pos, s8 reverb)`** — donor `/* Nonmatching */`,
   decoded from `802A91CC-802A92CC` (64 insns, dol_disasm):
   - `if (id >= 0x118) return;` — table bound (280 entries).
   - **2D-suppression:** ids `0xB4..0xBA` and `0x104` force `pos = NULL` (always
     un-positioned).
   - Look up `charVoiceTable[id] = {u16 a, u16 b}`; `a == 0xFFFF` or `b == 0xFFFF` →
     silent return.
   - Switch on `a & 0xF000`:
     - `0x0000` → **`charVoicePlay(a, b, pos, reverb)`** (the character-voice path);
     - `0x1000 / 0x2000 / 0x3000 / 0x8000` → `seStart(b, pos, 0, 1.0f…, 0)` (plain SE);
     - anything else → return.
4. **`charVoicePlay(long a, long b, Vec* pos, s8 reverb)`** — decoded from
   `802A9120-802A91CC` (43 insns):
   - `if (mVoiceMuteFlag /*this+0x201*/ == 1) return;`
   - `u16 port = (a << 8) | b;` — character id in the high byte, line id low.
   - stop the previous voice handle (`this+0x2060`) if live;
   - `startSoundVec(0x481F, &mVoiceSound, pos, 0, 0, 4)` — **ONE dispatcher SE id
     (0x481F) for every character voice in the game**;
   - `mVoiceSound->setPortData(8, port);` `setPortData(9, reverb);` — the sequence
     behind 0x481F reads port 8 to pick the actual sample, port 9 for reverb.
5. **`charVoiceTable__11JAIZelBasic`** (named donor symbol, `.data @0x8039BC30`,
   0x460 bytes) — extracted verbatim → **`port-kits/voice/charVoiceTable.h`**
   (280 entries: 256 charVoice / 21 plain-SE / 3 silent).

## Why this settles the design

- Voices are **data, not code**: `(character << 8) | line` on a port. No per-actor
  JA_SE_CV wiring exists to port — the 405 `JA_SE_CV_*` enum ids are used by ACTORS for
  barks/cutscene SFX, while **talk voice is entirely this table + dispatcher**.
- History's ja1Voice message-redirect was the right SHAPE (message-tied) — the bridge
  can now be made **table-faithful immediately**: same 280-entry table, same 2D
  suppression, same silent ids, before any deeper audio work.

## [PORT-INTEGRATION] — the open points (History's side)

1. **Data:** does the port's WW dialogue catalog preserve `mInitialSound` per message?
   If it was dropped at conversion, re-extract it from the donor BMGs (bmc/bmg parse —
   Foundry can add a pass to the dialogue pipeline on request).
2. **Playback:** true-native needs the dispatcher semantics — play SE `0x481F` through
   the WW-audio redirect and write sequence ports 8/9 (`JAISound::setPortData`
   equivalent). If the port's JAudio layer can't write sequence ports yet, the interim
   bridge should still: look up `l_charVoiceTable[mInitialSound]`, honor 0xFFFF/bounds/
   2D-suppression verbatim, and map `(a<<8)|b` to the redirect's voice choice — bridge
   behavior becomes donor-exact even before the port-write mechanism lands.
3. The mute flag (`this+0x201`) maps to whatever the port uses to silence voices in
   cutscene skip/pause.

**Landing:** History owns the wire (dialogue + audio lanes). Acceptance: a voiced donor
message (e.g. Aryll's) opening with the same voice line the donor plays, and a
2D-suppressed id staying un-positioned.
