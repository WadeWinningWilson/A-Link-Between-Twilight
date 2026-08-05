# WW De-Mount Census — ladder step 4 work-list (Housing, 2026-08-04)

Source: population/actor_map.ini (60 census sections). Columns: routing as authored
today; LIGHTING = whether the TU is on the native §405-§407 chain (17-TU sweep list);
KIT = candidate for Foundry's gated actor kit re-emit (lineage tag + laws lint).
Classification legend: native = spawns a real actor profile · manifest/mount = drawn
or stood-in by mount machinery -> DE-MOUNT TARGET.

| census | proc/socket | kind | arc/model |
|---|---|---|---|
| Pirates | NPC_P2 | - | - |
| P1a | NPC_P1 | - | - |
| P1b | NPC_P1 | - | - |
| P2b | NPC_P2 | - | - |
| Mk | NPC_MK | - | - |
| Zl1 | NPC_ZL | - | - |
| Ls1 | NPC_LS | - | - |
| Ob1 | NPC_OB | - | - |
| Ko1 | NPC_KO | - | - |
| Ko2 | NPC_KO2 | - | - |
| Ym1 | NPC_YM | - | - |
| Ym2 | NPC_YM2 | - | - |
| Yw1 | NPC_YW | - | - |
| Ah | NPC_AH | - | - |
| Aj1 | NPC_AJ | - | - |
| Bm1 | NPC_BM | - | - |
| Dk | NPC_DK | - | - |
| Pig | NPC_KB | - | - |
| kani | NPC_KN | - | - |
| Bb | NPC_BB | - | - |
| Kamome | NPC_KAMOME | - | - |
| Ba1 | NPC_BA | - | - |
| Ji1 | NPC_JI | - | - |
| pflower | NPC_EXTVEG | - | - |
| pflwrx7 | NPC_EXTVEG | - | - |
| flower | NPC_EXTVEG | - | - |
| flwr7 | NPC_EXTVEG | - | - |
| flwr17 | NPC_EXTVEG | - | - |
| kotubo | NPC_PTUBO | - | - |
| ootubo1 | NPC_PTUBO | - | - |
| Tpost | NPC_TPOST | - | - |
| Oyashi | NPC_OYASHI | - | - |
| Kanban | NPC_KANBAN | - | - |
| Throck | NPC_PIWA | - | - |
| koisi1 | NPC_KOISI | - | - |
| agbTBOX | NPC_BOXA | - | - |
| item | NPC_VLUPY | - | - |
| mo2 | NPC_MO2 | - | - |
| Bk | NPC_BK | - | - |
| lwood | NPC_LWOOD | - | - |
| Otana | NPC_OTANA | - | - |
| Otble | NPC_OTBLE | - | - |
| Plant | NPC_PLANT | - | - |
| Paper | NPC_OPAPER | - | - |
| Lamp | NPC_LAMP | - | - |
| bridge | NPC_EXTSPAN | - | - |
| Ksaku | NPC_KSAKU | - | - |
| Ajav | NPC_AJAV | - | - |
| Auzu | NPC_AUZU | - | - |
| Ekao | NPC_EKAO | - | - |
| Vdora | NPC_VDORA | - | - |
| Akabe | NPC_AKABE | - | - |
| Akabe10 | NPC_AKABE | - | - |
| ikada_h | NPC_IKADA | - | - |
| esa | NPC_ESA | - | - |
| kusax1 | NPC_EXTVEG | - | - |
| kusax7 | NPC_EXTVEG | - | - |
| kusax21 | NPC_EXTVEG | - | - |
| swood | NPC_EXTVEG | - | - |
| swood3 | NPC_EXTVEG | - | - |

## Notes for the kit pass
- Lit TUs already native (16 + d_door): ba1, bm1, esa, kamome, kb, knob00, lamp, ls1, mirror, mshokki, otble, spc_item01, swhit0, toripost, ww_demo00, zl1.
- Every row whose proc is empty '(mount)' or manifest-kind is a step-4 de-mount target:
  re-emit through the gated actor kit (KIT-LINEAGE: native-port), donor-receipt-first
  (DECOMP-FIRST rule), lighting law 1 auto-enforced by kit_laws.
- bridge (plank_span) already native-port + §425 lit. doors.ini/bridges.ini rows ride
  their own actors, not this list.

## SOCKET CLASSIFICATION (the real step-4 split, from npc/*.ini manifests)
NATIVE already (19 manifests, real profiles): npc_ba(BA), npc_kamome, npc_kb, npc_ls,
mk, p2, npc_zl, npc_tpost, npc_ext_span, npc_ext_veg, ext_bg10 + ext_bg0-9 (NPC_KDK).
DE-MOUNT TARGETS (51 manifests, ALL on the NPC_HENNA0 adapter socket — the audition
stand-in): gnd, md, npc_ah, npc_aj, npc_ajav, npc_akabe, npc_auzu, npc_bb, npc_bk,
npc_bm, npc_boxa/b/c, npc_bridge, npc_cc, npc_dk, npc_ekao, npc_esa, npc_ikada, npc_ji,
npc_kanban, npc_kn, npc_knob, npc_ko, npc_ko2, npc_koisi, npc_krock, npc_ksaku,
npc_lamp, npc_lwood, npc_mo2, npc_ob, npc_opaper, npc_otana, npc_otble, npc_oyashi,
npc_p1, npc_piwa, npc_plant, npc_pt, npc_ptubo, npc_q1/q2/q3, npc_vdora, npc_vlupy,
npc_yaflw, npc_yaflw_w, npc_ym, npc_ym2, npc_yw.
Step-4 execution: per actor, DECOMP-FIRST donor read → gated kit re-emit
(KIT-LINEAGE: native-port) → direct-port spawn switch (the §228 KB / §232 KAMOME /
§244 LS pattern: add the socket to the resolver table, flip the manifest socket) →
Foundry lint sweep → playtest batch. Note: several targets' TUs are ALREADY lit
natively (esa, lamp, otble, knob, ba1...) — their de-mount is spawn-path only.
