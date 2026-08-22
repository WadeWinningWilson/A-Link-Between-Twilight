# SAVE SLOTS — the card is HEALTHY. All three saves valid, both copies.

> User, 2026-08-17: *"only 2 of my three save files appeared."* on the first
> epoch-2 vanilla boot with the WW plugin active.

## Verdict

**No data was lost, and nothing is corrupt.** Every test the game itself
applies at the card layer passes, on all three saves AND all three backups.

| test | result |
|---|---|
| file written today? | **No** — `01-GZ2E-gczelda2.gci` mtime **2026-06-06**; today's boots read only |
| slot layout | `SAVEDATA_SIZE = 0xA94`, `CARD_FILE_SIZE = 0x8000` — **identical in both trees** |
| `dSv_save_c` | differs only by two accessor methods, **no data members** |
| **checksum, all 3 saves** | **VALID** (card `0x4008` / `0x4A9C` / `0x5530`) |
| **checksum, all 3 backups** | **VALID** (card `0x6008` / `0x6A9C` / `0x7530`) |
| `data_version` | **6**, and `SAVEDATA_VERSION` is **6 in both trees** |
| `m_Do_MemCard.cpp` fork vs vanilla | **2 cosmetic lines** (`ATTRIBUTE_ALIGN` placement, a `DUSK_GAME_DATA` annotation) |

Backup taken before any further boot:
`Documents/dusklight-backups/save-protect-20260817-2025/`, sha
`b55662b8c6e4bcbddf2d3fd72da8f5b25ae6a29e6d55d93c6c51bcdb77dd97a9`.

## ⚠️ CORRECTION — my first parse was at the wrong offset and I reported it

I first reported *"all three slots are populated (2708/2708, 2542, 2705
non-zero)"* by reading slots at **card offset 0**. That region is the
banner/icon block, which is dense image data — **it would look "populated" no
matter what the saves contained.** The conclusion happened to be right; the
measurement did not support it.

The real region is card **`0x4008`**, and the `+8` is not a guess:
`m_Do_MemCardRWmng.cpp:27` declares

```c
struct data_s { int unk_0x0; int data_version; u8 data[...]; u32 checksum; };
```

— eight bytes ahead of `data[]`, which is why `CARDRead(..., 0x4000)` plus
`&saves->data[...]` lands at `0x4008`.

**How the right offset was found: I stopped asserting one.** Two attempts at
`0x4000` returned "all six slots INVALID", which cannot be true — a card with
zero valid slots shows zero saves, not two. So the file was scanned for every
`0xA94` window where the game's own checksum invariant holds. Eight windows
matched, six of them in two consecutive runs of three, `0xA94` apart — the
saves and their backups, located by the data rather than by my assumption.

**A second bug was caught the same way:** `low += ~*d` promotes `*d` to `int`
in C, so it is `low - x - 1`, **not** the byte-wise `255 - x` I first wrote.
Both errors produced confident, wrong INVALID verdicts.

## So what IS showing 2 of 3?

**Not the card layer.** Layout, checksums, version and the card code are all
identical or valid. The difference is above it, in the file-select
presentation:

- `d_file_select.cpp:4864` is the only version gate (`!= 6`) and it passes.
- **`d_menu_save.cpp` differs by 219 lines between the trees** — substantive
  rework, including runtime JPN-region handling (`dusk::version::isRegionJpn()`)
  where the fork still uses compile-time `#if VERSION == VERSION_GCN_JPN`.

That is the next place to look, and it is a **display** question, not an
integrity one.

## Standing guidance

**Reading is proven harmless — the file was not touched by three boots.** The
remaining risk is a *write* while a slot is not displayed. Until the display
cause is identified, do not save in vanilla. The backup exists because that
risk is not zero, not because the data is currently at risk.

---

## The display mechanism, traced to the line

`d_file_select.cpp:3847`:

```cpp
int res = mFileInfo[i]->setSaveData(pSave, mDoMemCdRWm_TestCheckSumGameData(pSave), i);
if (res == -1) { mIsNoData[i] = true; }
```

And `dFile_info_c::setSaveData` (`d_file_sel_info.cpp:119`) returns:

| result | condition | shown as |
|---|---|---|
| `-1` | **checksum INVALID** | **no data — the empty slot** |
| `0` | valid + player name present | the save, with name/date/playtime |
| `1` | **valid but player name EMPTY** | **"New Quest Log"** |
| `2` | as 1, and it is the current data num | "New Quest Log" |

**`mIsNoData` is set ONLY by an invalid checksum** — and all three slots are
checksum-valid, so **none of them can be reported as an empty slot.** Whatever
the user saw, it was not the no-data path.

**That leaves `result = 1`: valid checksum, empty player name, rendered as
"New Quest Log"** — which on screen is indistinguishable from a free slot to
anyone not looking for the distinction. `d_file_select.cpp:662` treats
`mIsNoData[i] || mIsDataNew[i] != 0` together in the display path, so the two
converge visually.

### Supporting evidence, and its limit

All three slots carry real stage names (`F_SP108`, `F_SP104`/`F_SP103`,
`F_SP108`) — so all three are genuine saves, not blanks. But **slot 2 is
markedly sparser: 185 non-zero bytes against 299 and 309.**

**This is a lead, not a conclusion.** Confirming it needs the byte offset of
`getPlayer().getPlayerInfo().getPlayerName()` inside `dSv_save_c`, read out of
the three slots. That is the next step and it is cheap.

**What IS settled: no data was lost, nothing is corrupt, the card layer is
identical between the trees, and the empty-slot path is unreachable for
checksum-valid data.**

---

## 2026-08-18 — Conejo is in the same Card A folder, wrong filename

Live `01-GZ2E-gczelda2.gci` (mtime 2026-06-06) is still **Link / empty / empty**.
Zero `Conejo`, zero `F_SP121`. That file is not the missing logs.

The two Conejo quest logs are already on disk as:

`%AppData%\Roaming\TwilitRealm\Dusklight\USA\Card A\REL_Loader_v2_us.gci`

Header is `GZ2E01` / `gczelda2` — it is a Twilight Princess card image, not a
REL loader. GCI Folder mode only mounts `01-GZ2E-gczelda2.gci`, so this file
is ignored at file select. Checksums **VALID** on all three slots:

| Slot | Name | Return stage |
|------|------|----------------|
| 0 | Conejo | `F_SP121` Hyrule Field |
| 1 | Conejo | `F_SP103` Ordon |
| 2 | Link | `F_SP103` Ordon |

Restored 2026-08-18 16:55: live `01-GZ2E-gczelda2.gci` replaced with that
Conejo card (SHA `06680e00…e0f72aa8`). Pre-swap Link card kept at
`Documents/dusklight-backups/conejo-restore-20260818-1655/`.
`REL_Loader_v2_us.gci` left in place as a duplicate.

Older Conejo cards (Forest Temple / Faron, no Field) still exist under
`D:\Dolpheen Plz\Texture Packs\ZTP4K 3.0b (1080p)\User\GC\USA\Card A\`.
