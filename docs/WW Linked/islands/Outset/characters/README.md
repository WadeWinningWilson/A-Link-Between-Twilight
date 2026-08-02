# Outset — Characters

One subfolder per character, because each restored NPC accumulates a lot of specific detail (census
code, assets, cutscene beats, animation/expression maps, open bugs) that would otherwise bury the
island-level docs.

## Convention

```
characters/
  <Name>/
    README.md      ← the character's index: identity, assets, status, open issues, ledger links
    <topic>.md     ← deep-dives that outgrow the README (e.g. cutscene beat sheet, expression map)
```

- **Naming.** Use the character's real name only when it is **user-locked or decomp-sourced**
  (Ivan rule). Until then, use the census code and mark `? (unverified)`. Aryll qualifies — her actor
  is `daNpc_Ls1_c` in the donor and the user has confirmed her.
- **What lives here vs. elsewhere.** Character-*specific* facts live here. **General, reusable
  procedures stay at the `WW Linked` level** and are linked to, not copied:
  [cutscene-recipe](../../../cutscene-recipe.md),
  [cutscene-animation-recipe](../../../cutscene-animation-recipe.md),
  [face-expression-recipe](../../../face-expression-recipe.md).
- **The ledger is not duplicated.** The chronological record stays in
  [docs/state/cut-actors-demo-restore.md](../../../../state/cut-actors-demo-restore.md); a character
  README *links* to the relevant №-entries, it does not restate them.

## Roster

Identities below are **decomp-sourced** — each WW actor file carries a top-of-file header comment
naming the character (e.g. `d_a_npc_ob1.cpp:3` = `NPC - Rose`). Per the Ivan rule that qualifies
as a valid lock basis. Bodies are Nonmatching stubs, but the header/class/profile are readable and
load-bearing. (Resolved by decomp sweep, 2026-07-26.)

| Folder | Census | Identity | Basis | Status |
|---|---|---|---|---|
| [Aryll](Aryll/README.md) | `Ls1` (`NPC_LS`) | **Aryll** | ✓ decomp `NPC - Aryll` + user | Opening cutscene performs; telescope beat drives `Bm1` (Rito) via `mBm1ProcID`; one texture bug open |
| — | `Zl1` (`NPC_ZL1`) | **Tetra** | ✓ user + prior Tetra-texture work | placed across layers 0/2/8/9/10 |
| — | `Ob1` (`NPC_OB1`) | **Rose** | ✓ decomp `NPC - Rose` | not built; pig-count + path-walk behavior |
| — | `Ym1` (`NPC_YM`) | **Mesa** (grass-cutter) | ✓ decomp `NPC - Mesa & Abe` + **user position** (x≈-194510) | `daNpc_Ym1_c`; `setKariFlg`/`set_cutGrass`; east grass, away from pigs |
| — | `Ym2` (`NPC_YM2`) | **Abe** (pig quest) | ✓ decomp + **user position** (x≈-201127, at pig pen) | same class, `YM2` profile; `chk_BlackPig`. **Shirt fix:** shares `ym.bdl` (=Mesa's shirt) → `body_bmt=ym2.bmt` added (mirrors Joel/`ko02.bmt`); awaiting visual confirm |
| — | `Aj1` (`NPC_AJ1`) | **Sturgeon** | ✓ decomp `NPC - Sturgeon` | not built; anger/smoke emote set |
| — | `Ko1` (`NPC_KO`) | **Zill** (booger) | ✓ decomp `NPC - Joel & Zill` + **user in-play lock (N50)** | booger head `kohead01`; `HanaPachi` snot rig; child link to `Ob1`/Rose |
| — | `Ko2` (`NPC_KO2`) | **Joel** | ✓ decomp + **user in-play lock (N50)** | `kohead02` + `body_bmt=ko02.bmt` (distinct shirt over shared `ko.bdl`) |
| | | | | ⚠ **decomp comment order "Joel & Zill" does NOT map to proc order** — do not re-derive; `Ko1`=Zill, `Ko2`=Joel is the locked truth |
| — | `Yw1` (`NPC_YW1`) | **Sue-Belle** | ✓ decomp `NPC - Sue-Belle` | not built; water-pot (`tsubo`) fingerprint |
| — | `Ah` (`NPC_AH`) | **Old Man Ho Ho** | ✓ decomp `NPC - Old Man Ho Ho` | not built; shore look-at idle (no telescope code in stub) |
| — | `Bm1` (`NPC_BM1`–`BM5`) | **generic Rito** (not a named human) | ✓ decomp `NPC - Generic Ritos` | reusable flying-Rito actor; the specific Rito is an instance param; this is the telescope-beat partner Aryll drives |
| — | `NpcSo` (`NPC_SO`) | **Fishman (blue fish)** | ✓ decomp `NPC - Fishman (blue fish)` | sea-chart bait creature, not an Outset resident |

### `Cb1` = Makar, `Md1` = Medli — sages, NOT Outset residents (RESOLVED, user 2026-07-26)
- `Cb1` (`NPC_CB1`) → decomp `Player - Makar`; `Md1` (`NPC_MD`) → decomp `Player - Medli`.
- The `Player -` class = **Wind-Waker-conductable partner** (Command Melody control) — that's why
  the decomp files them under "Player," and it's why they carry the heavy HIO/state machinery.
- **User: neither appears on Outset** (not law, but the working truth). So their Outset DZR
  placements are **spurious / non-resident** — do NOT spawn them as villagers. Drop or hard-gate the
  `Cb1`/`Md1` Outset rows; they are correctly *identified* (the sages), wrongly *placed*.

_Recommended `identity.ini` locks (user to apply via "Lock identity name", or tell History to):_
`[NPC_OB1] Rose · [NPC_AJ1] Sturgeon · [NPC_YW1] Sue-Belle · [NPC_AH] Old Man Ho Ho · [NPC_YM1] Mesa · [NPC_YM2] Abe`
— the **Joel/Zill** pair stays pending the in-game booger check (see the `Ko1`/`Ko2` row); the
`Cb1`/`Md1` rows should be dropped, not locked.
