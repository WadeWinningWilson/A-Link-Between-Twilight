# Outset (Room44) — per-layer CONTENTS census (noclip-derived)

> **Instrument:** noclip `#zww/Room44.arc` ("Outset Island"), read via the identity-match rig
> ([noclip-fast-track.md](../../../noclip-fast-track.md) — elevated method). Each live actor in
> `frameworkGlobals.lyCurr.pcQueue` carries `roomLayer`; `objectNameTable` resolves `profName`→code;
> codes verified against decomp `d_stage.cpp` `OBJNAME(...)`. Recorded History, 2026-07-26.
>
> ⚠ **CONTENTS, NOT ACTIVATION (the standing noclip wall).** noclip shows what each DZR layer
> *contains*; it does NOT run the story logic that decides *which* layer is active in a given beat.
> The user's working hypothesis — "layer 0 = during the awake cutscene, layer 1 = after" — is an
> **activation** claim noclip cannot confirm. Note in fact layer 0 contains the **pirate ship +
> pirate crew + `Zl1`**, which do not belong to the wake-up/birthday moment (pirates arrive later) —
> so layer 0 reads as a broader intro/"pirates-at-Outset, Aryll-still-present" superset, not the
> pure awake beat. Which flags gate each layer stays History's determination (cf. №222).
>
> IVAN: identities are the **verified decomp profile code**. Human names appear ONLY where our
> roster ([../characters/README.md](../characters/README.md)) already confirms them; everything else
> stays a census code marked `? (unverified)`. If a number here disagrees with
> [../inventory.md](../inventory.md), the inventory wins.
>
> **UPDATE 2026-07-26 (Follow-up A — decomp-resolved):** the villager `? (unverified)` cells below
> are now decomp-sourced (WW actor-file header comments) — see the roster:
> `Ob1`=**Rose**, `Aj1`=**Sturgeon**, `Yw1`=**Sue-Belle**, `Ah`=**Old Man Ho Ho**,
> `Ym1`/`Ym2`=**Mesa & Abe**, `Ko1`/`Ko2`=**Joel & Zill**, `Zl1`=**Tetra**, `Bm1`=**generic Rito**
> (telescope-beat partner), `NpcSo`=**Fishman**. ⚠ `Cb1`=**Makar** and `Md1`=**Medli** per decomp
> are PLAYABLE PARTNERS, not Outset residents — their placement here is a contradiction to resolve,
> not a confirmed villager.

## Layer 0 — POPULATED (full intro cast)

| Code | Decomp profile | Identity | Count |
|---|---|---|---|
| `Ls1` | `NPC_LS1` | **Aryll** (✓ decomp + user) | 1 |
| `Zl1` | `NPC_ZL1` | ? (unverified — likely Tetra) | 1 |
| `Ob1` | `NPC_OB1` | Outset villager ? (unverified) | 1 |
| `Ym1` / `Ym2` | `NPC_YM1` / `NPC_YM2` | Outset villagers ? (unverified) | 1 / 1 |
| `Aj1` | `NPC_AJ1` | Outset villager ? (unverified) | 1 |
| `Bm1` | `NPC_BM1` | Outset villager ? (unverified; Aryll's actor searches `mBm1ProcID` — see [npc-presence-recipe.md](npc-presence-recipe.md)) | 1 |
| `Ko1` / `Ko2` | `NPC_KO1` / `NPC_KO2` | Outset villagers ? (unverified) | 2 / 1 |
| `Yw1` | `NPC_YW1` | Outset villager ? (unverified) | 2 |
| `P1c` / `P2c` | `NPC_P1` / `NPC_P2` | pirate crew ? (unverified) | 3 / 2 |
| `Dk` | `fpcNm_DK_e` | ? (unverified) | 1 |
| **props** | | | |
| `Pirates` | `Obj_Pirateship` | the pirate ship | 1 |
| `Svsp` | `OBJ_IKADA` | the raft (Ikada) | 1 |
| `Touseki` | `Obj_Tousekiki` | ? (unverified) | 1 |
| `bridge` | (bridge object) | 2 |
| `Kamome` | `KAMOME` | seagulls | 4 |
| `KNOB03D` | (door) | 1 |
| `prof174` | — | UNRESOLVED (no name-table entry; likely system/effect) | 1 |

## Layer 1 — near-empty (only the raft added)

| Code | Decomp profile | Identity | Count | Position |
|---|---|---|---|---|
| `Svsp` | `OBJ_IKADA` | the raft (Ikada) | 1 | `(-203079, 1, 312923)` |

Layer 1 adds **nothing** beyond the always-layer except the docked raft. If layer 1 is indeed the
post-awake normal state, the island is simply its permanent population (below) plus the raft.

## Always layer (`-1`, present in every state) — NPCs

| Code | Decomp profile | Identity |
|---|---|---|
| `NpcSo` | `NPC_SO` | Outset villager ? (unverified) |
| `Md1` | `NPC_MD` | Outset villager ? (unverified) |
| `Cb1` | `NPC_CB1` | Outset villager ? (unverified) |

Plus the permanent props (from the same `-1` layer): palms (`Oyashi`/`Lpalm`), grass, `woodbx`,
pigs (`Pig`/`KB`), crabs (`kani`/`KN`), signs (`Kanban`), `Tpost` (Toripost), `lwood`, `Hbox2S`,
fly-items (`itemFLY`), doors (`KNOB03D`), seagulls (`Kamome`), sky (`Vrbox`/`Vrbox2`), `sea`.

## Follow-ups (History)
1. Resolve the villager human identities per-actor against the decomp `d_a_npc_*` files (IVAN) and
   fold into [../characters/README.md](../characters/README.md).
2. Determine the story-flag → layer activation for layer 0 vs 1 (DZR layer-switch / the `awake`
   demo) — the piece noclip can't answer.
3. Reconcile this per-layer contents list against [../inventory.md](../inventory.md).
