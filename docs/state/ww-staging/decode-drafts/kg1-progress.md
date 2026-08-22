# kg1 (Orca) decode campaign — anchor

Opened 2026-08-22 by the successor DECODER session that closed yw1 to
parks (109/119, fuzzy 98.71) the same day. kg1 = Orca, Outset room 44 —
next target under the Outset-first charter (smallest open Outset TU).

## Baseline at open

- 2/66 exact, fuzzy 1.31 (template stubs only; 44 Nonmatching markers
  incl. ctor).
- Target .o: build/GZLE01/d_a_npc_kg1/obj/d/actor/d_a_npc_kg1.o
  (3,303 disasm lines).

## Census facts (read from the target ctor before any body)

- **Base class: fopNpc_npc_c** (ctor chain: fopAc_ac_c → vt fopNpc_npc_c
  → dBgS_Acch/ObjAcch → dBgS_AcchCir → cCcD_Stts/dCcD_GStts) — the
  ym1/yw1 family; the whole session lever bank applies.
- **Fuller collision set than yw1**: dCcD_GObjInf + cM3dGAab + cM3dGCyl
  vtables in the ctor — Orca hosts the spear-training minigame (hit
  counting), expect dCcD sphere/cyl attack shapes and hit callbacks.
- **HIO derives dNpc_HIO_c** (not mDoHIO_entry_c like yw1) —
  `__ct__15daNpc_Kg1_HIO_cFv` calls `__ct__10dNpc_HIO_cFv`. Check ba1/so
  for the dNpc_HIO_c template before writing it.

## Levers to apply up front (minted ym1/yw1, all proven this session)

1. HIO ctor single-iteration-loop idiom (if prm table present).
2. btp/bck resID single-entry-table trap — check table sizes in rodata
   symbols FIRST.
3. OR-guard return for any bne+b guard pair; else-return placement.
4. Case blocks emit in SOURCE order; empty cases steer the pivot.
5. Inlines carry branches (fpcM_GetID-class) — read the callee first.
6. bool vs u8 vs BOOL: srwi=bool field · rlwinm27,24,31=u8 value ·
   clrlwi join = bool chain end-to-end · no-normalize return = bool.
7. anmMtx pair-cache: local for adjacent multVec pairs, inline single.
8. Reverse-declaration stack slots for same-kind named locals.
9. Write pool NEIGHBORS when a 99.9x cluster shares stringBase/floats.

## Batch 1: 2 -> 6/66 exact

- Base-class correction applied (header claimed fopAc_ac_c; ctor asm
  rules fopNpc_npc_c — members at 0x6C4 confirm). Return-type clashes
  with the base virtuals (getMsg u32, next_msgStatus u16) fixed in the
  same pass — the stub header had them void.
- HIO ctor EXACT: daNpc_Kg1_HIO_c = vtbl + s8/-1 + s32/-1 + u8 +
  **dNpc_HIO_c mNpc[1]** (0x28 elem, __construct_array tell — an ARRAY
  member emits construct_array even at count 1) + 2 trailing u8. Values:
  head ±2500/±7000, backbone ±2000/±8000, turn 1000/2000, attnY 35.0f,
  attnAngle 0x4000, attnDist 400.0f. Dtor chain snapped with it (6/66).

## NEXT

Rebuild header on the fopNpc template (actor size from g_profile_NPC_KG1),
carve as decoded; then _create → createInit → init ladder → CreateHeap
family → anm cluster → action ladder, mirroring the yw1 order.
