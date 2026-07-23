# Cutscene audio — Aryll voice + opening music (SCOPING, currently applicable)

**The two asks (user, 2026-07-22):** with Aryll's cutscene portion essentially done, the first
cutscene needs (1) **Aryll's character voice**, and (2) **the first music track, transitioning into
the Outset Island theme** (`i_link`, which we already play).

> ## PRIORITY ORDER (user-decreed, 2026-07-22)
>
> 1. **FIRST: opening music + transition into the Outset theme.**
> 2. **WITH IT: the Outset theme itself** — `i_link` playback is still not right (§B tempo fix
>    awaits re-test; "layers louder" is at the runtime-ramp-interpretation stage). A transition
>    into a still-imperfect theme is half a deliverable.
> 3. **VOICE IS PARKED.** No Aryll voice work — no cue extraction, no wave sourcing, no trigger
>    wiring — until 1 and 2 land. The §1 scoping below is retained for when it unparks.

**This doc scopes both from donor evidence.** It names what is verified, what is unidentified, and
which identification decides the cost. Nothing here is built; lane assignments at the end.

> **The one decision-driving unknown:** whether the opening music is a **stream** (`.afc`) or a
> **sequence** (`.bms`). Sequence → our ExtSeq player already handles it → roughly a manifest entry.
> Stream → **we have no stream playback path at all** (verified) → a new engine surface.
> **Identify before building anything.**

---

## 1. Aryll's voice

### Donor mechanism (verified)

- Character voices are **SE-system events**: the `JA_SE_CV_*` family in
  `WW DP/include/JAZelAudio/JAZelAudio_SE.h` (CV = character voice). Dozens of families exist
  (`CV_AJ`, `CV_BB`, `CV_BO`, …, plus demo-scoped `CV_D26`, `CV_D35`).
- **Aryll (`LS`) has exactly two CV ids:** `JA_SE_CV_LS_D23_SMILE = 0x4983`,
  `JA_SE_CV_LS_D23_LIFTED = 0x4984`.
- **Her own actor plays none of them** — `d_a_npc_ls1.cpp` contains zero SE/voice calls. Cutscene
  voice cues are triggered by the demo/event layer, not the NPC actor.

### UNIDENTIFIED — and it gates the clip list (IVAN rule applies)

- Both Aryll ids are tagged **`D23`** — a demo index. `LIFTED` *suggests* the Forsaken Fortress
  kidnap scene, **not** the opening lookout scene — **suggestion is not identification.**
- **Aryll's voice DOES play in the donor opening — user-confirmed (2026-07-22, direct knowledge of
  the game).** The open question is narrower than first scoped: not *whether* cues exist, but
  **which SE ids the opening fires and through which mechanism** — the D23 pair (if D23 is the
  opening — still unverified), a different CV/demo-scoped family, or message-system voice tied to
  her dialogue.
- **History reads the opening demo/event data for its sound cues** (when voice unparks) to name the
  exact ids — extraction and wiring wait on that list.

### Receiver path (mostly exists)

- The **shadow-wave system is committed and working** (`registerShadowWave` → DSP routes
  `kShadowVirtualBase + …`, `custom_assets.cpp:2374-2442`): custom waves already reach TP's audio
  hardware path. Voice clips are just waves.
- In-house prior art: the ALBW **NPC voice workflow** (memory: `workflow_npc_sounds`) already wired
  voice audio to an NPC once — wave arc location, load/erase lifecycle, diagnostic logging pattern.
- **Missing piece:** demo-timeline trigger wiring (fire SE X at cue Y in our authored cutscene) —
  Engine, once History names the cues.
- **Wave sourcing:** which donor bank/WSYS carries the `CV_LS` waves is **unidentified** — Bridge's
  `bank-map`/`seq-banks` tooling is the instrument for this.

---

## 2. The opening music + transition

### Donor mechanism (verified)

- **Prime candidate id:** `JA_STRM_PROLOGUE = 0xC0000000` — the *first* stream id in
  `JAZelAudio_BGM.h` (`0xC0000000` = `JAISoundID_Type_Stream` base).
- **The transition contract exists in the donor:** `JAIZelBasic::bgmStreamPlay()`
  (`JAIZelBasic.cpp:282`) — on `JA_STRM_PROLOGUE`, `mpMainBgmSound->stop(30)` (30-frame fade) and
  clears `field_0x1f3c`. I.e. WW's own opening does *stream in, fade the sequence world out* — and
  the reverse handoff into the island theme is the model for our "transitions into Outset theme."

### ~~UNIDENTIFIED × 1~~ — **CLOSED (History, 2026-07-22): the awake cutscene requests NO BGM/stream at all.**

Verified at every layer of the donor, not by enum names:

1. **The storyboard (`awake.stb`), decoded exhaustively — 7 blocks:** JFVB (function values),
   JCMR `camera`, JSND `SE`, JACT `Link`, JACT `Ls1`, JMSG `message`, Control. **The only audio
   track is JSND `SE`**, and its entire content is four timed writes to sound register `0x799`
   (companion reg `0x5C2`=0 each time): ids **`0x1880` → `0x1881` → `0x1882` → `0x1883`** at
   cumulative frames **1129 → 1430 → 1720 → 2260**. Those ids are
   `JA_SE_LK_V_D47_SLEEP / AWAKE / NOBI / NOTICE` (`JAZelAudio_SE.h:417-420`) — **Link's wake-up
   voice one-shots** (sleep-murmur, wake, stretch, notice; D47 = this demo). There is **no
   BGM/stream opcode anywhere in the STB.** The Control block is 13 suspend ops — the message
   suspend/resume contract we already implement (№172).
2. **Aryll's actor doesn't start music for this scene either.** `d_a_npc_ls1.cpp`'s only stream is
   `JA_STRM_DEMO_TETRA_FLY`, prepared in `telescope_proc` and fired in `checkOrder` case 0 = her
   event **"zelda_fly"** (her table: `zelda_fly / omedeto / get_telescope / eTalk` — the awake
   event is stage-owned and not in it).
3. **The name-obvious candidates belong to OTHER scenes.** `JA_STRM_DEMO_01_01` (→ `1tale.afc`,
   Bridge map) is prepared/played by `dScnOpen_proc_c` (`d_s_open_sub.cpp:147`) — the **pre-title
   storybook scene**, its own scene proc. `JA_STRM_PROLOGUE` (→ `e3title.afc`) is not requested by
   anything in the opening flow we restore.

**Therefore: during the awake demo the donor's music is whatever the stage scene-BGM system is
already playing — Outset's island theme, a SEQUENCE, via `JAIZelBasic` scene BGM.**

> **§72 REFINEMENT (2026-07-22) — WHICH island theme is the new fact.** User footage: waves ambience
> → **hook medley** → theme. `JAIZelBasic::setScene` selects Outset's music from **4 variants + 2
> silence states by event bits**; the **fresh-game state plays `JA_BGM_ISLAND_LINK_0` (0x80000038)**,
> not the base `0x80000001` our manifest ships. Grandma's house likewise has `HOUSE_G` (0x80000018,
> started by Grandma's own actor) beside the base `HOUSE`. **The medley is most plausibly
> `ISLAND_LINK_0`'s own arrangement — **Bridge confirmed DISTINCT:** `i_linkin.bms` 19552 b
> (sha `73cf4396…`) ≠ base `i_link.bms` 13088 b; staged twin-OK in `ww_jaudio1` (tool 0.23.0).
> User listen still the ear acceptance. Full table: bus §72b.

~~2. Stream id → `.afc`~~ — **CLOSED** by Bridge `stream-map` (BSM in `JaiInit.aaf`). LagoLunatic
   `BGM Sequences.txt` is sequence-only. Authority: AAF type 5 → `streamList_t[]`, index=`id&0x3FF`.
   **`JA_STRM_PROLOGUE` → `e3title.afc`** (not `1tale.afc`; that file is `JA_STRM_DEMO_01_01`).
   `BPW_START` → `bp_start.afc` cross-checks community. Full CSV: tool `reports/stream_map.csv`.

### Receiver gap (verified)

- **No stream playback exists anywhere in our audio work** — zero AFC/HardStream references in
  `ext_seq/`, `d_ext_seq_space.cpp`, `custom_assets.cpp`, and no TP-side `JASHardStream` in the libs
  we build. TP is JAudio2; its streaming is a different system and nothing is wired.
- Our ExtSeq owned-space system now has a **BGM transition hook** (`OwnState::Handoff`, default
  30-frame fade of prior JA2 or ExtSeq, then start next BMS) — needed regardless of
  stream-vs-sequence for the handoff into `i_link`. Opening **source** (seq vs stream) still waits
  on History; **no AFC playback** until that answer is "stream."

### Cost table — why identification comes first

| if the opening music is… | cost |
|---|---|
| a **sequence** (`JA_BGM_*` → `.bms`) | **cheap** — package manifest entry + transition hook; ExtSeq already plays it |
| a **stream** (`JA_STRM_*` → `.afc`) | **expensive** — a new playback surface (AFC decode/feed), or a design decision to re-encode; plus the transition hook |

---

## 3. Covenant (Housing Security)

- Voice waves, `.afc` files, and any stream-id→filename mapping are **donor content → mod package
  only**. The §67 pattern is the law here: **no WW file names in the exe** — stream/voice names
  resolve from `manifest.ini` keys exactly as the seq stems now do.
- The **audio twin invariant (№28 B10)** extends to every new payload class: shipped `.afc`/wave
  bytes must be byte-size-identical to donor (Check D will cover them once they exist).
- №31: opening music and Aryll's voice belong to WW spaces/cutscenes only.

---

## 4. Lane assignments — re-cut to the priority order

### Active now (priorities 1–2)

- **History:** ONE question first — **which BGM/stream id does the opening cutscene request?**
  (The voice-cue question is parked with the voice work.)
- **Bridge:** ~~stream-id→`.afc`~~ **DONE** (`stream-map` 0.22.0). No further Bridge ask on mapping.
- **Engine:** (1) ExtSeq **transition hook** — **SHIPPED** (`OwnState::Handoff`, default fade 30,
  `dExtSeqSpace_requestHandoffToField`; JA2 `mDoAud_bgmStop(N)` / ExtSeq root-vol fade). Opening
  **source** wiring still waits on History. (2) **`i_link` quality** — §C.1 offline shows **zero**
  timed volume ramps (`moveTime=-1` snaps only); live probe via `DUSK_EXTSEQ_VOL_PROBE`. (3) stream
  playback **only if** History's answer is "stream," as a designed decision, not a reflex.
- **User:** the §B tempo re-test (Outset + Grandma's) is still the outstanding ears-check on the
  theme itself — it is priority 2's acceptance test.
- **Housing Security:** covenant gates when new payload classes land (twin invariant on `.afc`/waves,
  manifest-driven naming, no exe literals per §67).

### Parked (do not start)

- **All Aryll voice work** — cue identification, `CV_LS` wave-bank location, package keys for voice,
  SE trigger wiring. §1 above is the scoping to resume from. Unparks when the intro+transition ship
  **and** the Outset theme passes the user's ear.

**Order of operations: History's answer is in (see §2): NO stream — the cheap row of the cost table
is the real one. Engine transition hook **SHIPPED**; continue the `i_link` absolute-level /
`composedVolume` thread. The stream surface stays unbuilt until a scene that actually streams is
restored (pre-title storybook `DEMO_01_01`/`1tale.afc`, or Aryll's `zelda_fly` event `DEMO_TETRA_FLY`).
BONUS for the voice unpark: the STB's SE track IS the demo's cue list — four LINK voice one-shots
(`0x1880-0x1883`) at frames 1129/1430/1720/2260. Aryll's own calls are NOT in the STB's SE track —
her opening voice must ride another mechanism (likely message-tied), to be identified at unpark.**
