# WW audio — the Outset recipe

> ## READ FIRST — check the DONOR DECOMP, not just the receiver
>
> **Decomp source: `D:\XXXXXXX\WW DP\src`**  (arcs: `D:\XXXXXXX\Ex WW`)
>
> The receiver's source tells you **what** an API is. Only the donor tells you **how its
> own actors called it** — flags, argument order, order of operations. Those are not
> derivable from the receiver side, and guessing them looks exactly like a working port
> until it silently isn't.
>
> **This cost four debug rounds once already (ledger №177–№181).** The cast was invisible
> in the opening cutscene. Every receiver-side probe read green — actor bound, enables set,
> position correct, draw running, matrix written. The fault was that
> `dDemo_setDemoData(...)` was called with flags reasoned out as `0xEE` instead of the
> donor's actual **`106`**. The extra `ENABLE_SCALE_e` bit assigned
> `scale = demo_actor->getScale()` = **(0,0,0)** — the model was scaled out of existence
> while every position measurement stayed correct. The answer was sitting in
> `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ls1.cpp` the entire time.
>
> **Before instrumenting a receiver-side theory:**
>
> 1. Find the donor's equivalent actor/system (`d_a_npc_*.cpp` for NPCs,
>    `d_demo.cpp` / `d_event*.cpp` for cutscenes).
> 2. Copy its call shape **verbatim** — flags, argument order, sequencing.
> 3. Cite the donor function in a comment at the ported call site.
>
> Outset is only partially decompiled, so expect gaps — but struct definitions and call
> sites are reliable, and are the fastest route from "it doesn't work" to "here is the
> parameter I got wrong".



> ## ⚠ READ FIRST — `OffsetPos` applies to EVERY Great Sea space
>
> A donor event's `PACKAGE: PLAY` cut carries an **`OffsetPos`** that is handed
> straight to `dDemo_c::start(demo_data, xyzdata, offsetAngY)`
> ([d_event_data.cpp:1291](../../../src/d/d_event_data.cpp)). It is the origin the
> storyboard stages its **cast** from.
>
> **It comes across from the donor VERBATIM when an event is merged, and donor
> world coordinates are not receiver world coordinates.** Outset's opening carries
> `OffsetPos = -220000, 0, 320000` while the island itself sits near `-195000` — a
> **~24,600-unit gap in X**. A cast staged off the wrong origin lands in open ocean,
> and on screen that is indistinguishable from *"the actor never appears"*.
>
> **This affects every island, every interior and every NPC that is ever added**, not
> just the space it was first found on. Whenever you merge a donor event:
>
> 1. Decode the PLAY cut and read its `OffsetPos`.
> 2. Compare it against the receiver-space position of the scene (the camera
>    `FIXEDFRM`/`STBWAIT` Center is a good reference — it is usually already correct).
> 3. If they disagree, the cast will stage away from the camera. **Fix the field in
>    the merged `event_list.dat` — it is DATA, no rebuild required.**
>
> A camera that frames the right spot proves nothing about where the cast is: the two
> come from different fields. Verified-wrong beats assumed-right.
> Ledger: №165, №175.



**What this is:** the proven procedure for bringing a WW space's music into the port, written from
what actually shipped for Outset. **Reusable for any island or dungeon** — later spaces are the same
steps with different ids.

Outset exterior (`JA_BGM_ISLAND_LINK`) and its interiors (`JA_BGM_HOUSE`) were done together because
they share an instrument bank. Expect that pattern to repeat.

---

## 0. What makes a space cheap or expensive

Decide this **before** committing, because it changes the work by an order of magnitude.

| | cheap | expensive |
|---|---|---|
| BGM class | **sequence** (`JA_BGM_*`, id `0x8000…`) | **stream** (`JA_STRM_*`, id `0xC000…`) |
| why | plays through the JA1 path already built | `.afc` → `.ast` transcoding, not built |

**Authority:** `D:\XXXXXXX\WW DP\include\JSystem\JAudio\JAISound.h` —
`JAISoundID_Type_Sequence 0x80000000`, `JAISoundID_Type_Stream 0xC0000000`.
**118 sequence ids, 75 stream ids.** WW ships 76 `.afc` files, matching the stream count.

**If the space's theme is a stream, stop and re-scope.** Everything below assumes sequence.

---

## 1. Identify the BGM id — from the decomp, never by filename

```
D:\XXXXXXX\WW DP\include\JAZelAudio\JAZelAudio_BGM.h
```

Outset: `JA_BGM_ISLAND_LINK = 0x80000001` · `JA_BGM_HOUSE = 0x80000008`.

> **DO NOT match bank filenames to BGM symbols.** `IsleLink_0.aw` looks like `ISLAND_LINK` and **is
> not its instrument bank** — it is an area dynamic-wave bank. The auditor made exactly this mistake
> (§49); it is an IVAN-RULE violation. **Resolve banks by mechanism (step 2), never by name.**

## 2. Resolve BMS + instrument banks

```bat
python -m ww_bridge seq-banks --bgm <SYMBOL>
```

Follows the real engine chain rather than guessing: BMS bank ops (`writeRegParam`, regs `0x20`/`0x21`)
→ `BankMgr::getPhysicalNumber` (vir → phys IBNK) → WSYS `WINF` group filename in `JaiInit.aaf`.

Outset result: `i_link.bms` → banks vir 0, 2 → phys 0, 21 → **`n_zelda_0.aw` + `n2i_link_0.aw`**.
`house.bms` → bank 0 → **`n_zelda_0.aw`** (shared — which is why interiors were nearly free).

## 3. Package (offline, player-side)

```bat
python -m ww_bridge bgm-package
```

Stages to `…\WW-Crew-Restoration\audio\ww_jaudio1\`:

```
seqs/<name>.bms          raw donor bytes — never modified
banks/<bank>.aw          raw donor bytes
aaf_slices/ibnk_N.bin    instrument definitions
aaf_slices/wsys_N.bin    wave id → offset map
manifest.ini             schema=ww_bgm_jaudio1_v1
```

**Verify byte-identity against the player's extract before trusting it.** Path (B)'s whole
justification is that donor bytes ship unmodified — confirm it, don't assume it.

## 4. Engine — already generic, likely nothing to write

Shipped for Outset and **not island-specific**:

| file | role |
|---|---|
| `d_ext_seq_space.cpp` | space gate, ownership, package detect |
| `ext_seq/ja1_parser.cpp` | WW BMS dialect |
| `ext_seq/ja1_track.cpp` · `ja1_seq_ctrl.cpp` | track/sequence state |
| `ext_seq/ja1_bank.cpp` | IBNK/WSYS parse + shadow-wave residency |

**A new space should need a manifest entry, not new code.** If it needs code, that is a finding worth
recording — it means something about the space is not general.

---

## The two rules that make this work

### Ownership order — non-negotiable

```
stopOwned → gate → load → startOwned → tick
```

ExtSeq owns every JA1 track and every channel those tracks start. **Bank lifetime nests inside it:**
register after `stopOwned`, unregister only after all owned voices are released.

**Why it matters:** a voice latches its wave address at `noteOn`. Free a bank mid-note and it reads
freed memory — the use-after-free family behind six prior bugs (№65/69/73/83/88/89) and the
2026-07-19 crash. №89's invariant applies: **whatever begins a residency ends it.**

### Silence is a correct outcome

A stale virtual address returns `nullptr`, and the fetch site must **silence the voice — never fall
back to vanilla ARAM** (the virtual offset is out of bounds there). This agrees with №31/№91: **WW
spaces get WW audio or nothing. A TP sound in a WW space is a defect; silence is not.**

---

## Why №28 B10 does not block this

The audio-twin guard refuses any `.aw` twin whose byte size differs from vanilla — and `n_zelda_0.aw`
never will be size-identical. **That guard governs *twins*; this is not a twin.**

Banks register at **virtual** bases via `registerShadowWave`, and `noteOn` mints
`kShadowVirtualBase + virtBase + offset`. Nothing claims to be a vanilla bank, so nothing is refused.
**Do not try to make a WW bank pass the twin guard** — that path is a dead end by design.

---

## Verify before believing

1. **Byte-identity** — staged banks vs the player's extract (md5).
2. **Gate, both classes** — M6 forbidden names *and* the marker class (`ww_` in paths, symbols, log
   tags). **M6 alone is not a covenant statement.**
3. **Repo clean** — zero `.bms` / `.aw` / `.afc` tracked or staged. Audio is player-side only.
4. **Playtest** — entering: `[ExtSeq] startOwned`, no TP field BGM. Leaving: `stopOwned`, TP BGM
   resumes. **In that order.**
5. **Forced mid-note unregister silences** rather than crashing.

---

## Known-deferred

**Mix balance.** Oscillator output was loud relative to gameplay. **Do not tune against oscillator
tones** — real instruments change the reference. Tune only after residency is confirmed audible.

**Playback fidelity (separate from this recipe).** This doc covers *packaging and residency*. The
live investigation into playback timing/balance for `i_link`/`house` — supply verified pristine,
tick-truncation fix, the `set_param` blindness — lives in
[../../ext-seq-audio-findings.md](../../ext-seq-audio-findings.md). Boss-BGM channel masking (a
future mechanism, not needed for field/interior music) is filed in
[../../boss-bgm-prior-art.md](../../boss-bgm-prior-art.md).

## Scaling note

`sea/stage.dzs` carries `SCLS:212` and `RTBL:50` — ~50 sea rooms. If those share instrument banks the
way Outset's interiors shared `n_zelda_0.aw`, **most islands cost a manifest entry rather than a
port.** Run step 2 across several island BGM ids early: the bank-sharing map tells you the real size
of the remaining audio work before anyone commits to it.
