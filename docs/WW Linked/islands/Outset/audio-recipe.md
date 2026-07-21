# WW audio — the Outset recipe

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

## Scaling note

`sea/stage.dzs` carries `SCLS:212` and `RTBL:50` — ~50 sea rooms. If those share instrument banks the
way Outset's interiors shared `n_zelda_0.aw`, **most islands cost a manifest entry rather than a
port.** Run step 2 across several island BGM ids early: the bank-sharing map tells you the real size
of the remaining audio work before anyone commits to it.
