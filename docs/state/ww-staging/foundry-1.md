# foundry-1 — 19a v2: the per-TU readiness table (READY for integration)

> L8's blocker, discharged. Evidence: `foundry-1-evidence.md` (50/50 rows,
> fan-out compiled). Taxonomy per v1 (§520/§521) with v2's corrections:
> **(a)=0 until SDK services exist** (the ~60 `dusk::` sites are the
> prerequisite list), **(c)=0 on this roster**, **GX = fourth link family**
> (needs its own ruling), profile-DATA coupling has no a/b/c/e cell.
> Roster: 62 excluded + 11 mixed = 73 TUs.

## PRECONDITIONS (before any wave — each is a named blocker, not a vibe)

| P | what | evidence |
|---|---|---|
| P1 | Export-surface verification: diff `d_ext_ww_actor_shims`' 21 names against `symbol_manifest.py <image>` | every (b) count is NOT-VERIFIED until this runs |
| P2 | SDK services for `dusk::` (settings/ConfigPath/custom_assets/audio/ui) | ~60 sites; densest npc_mount ×14, itemmdl_pc ×12, ja1_bank ×11 |
| P3 | GX family ruling (link vs re-export vs shim) | ~470 sites in 6 TUs + d_particle.h:148 inline leak |
| P4 | Unwind 2 receiver→WW inversions (d_stage → ww_stage_loader/npc_mount; d_particle → ww_jpa) | hardest structural blocker |
| P5 | 3 excluded/KEPT pairing rulings (ja1_jaschannel/JASChannel · evt1/d_event_* · profile_register/f_pc_profile_lst) | History |
| P6 | L2a re-spec against setter hooks (dRes_setArcFileNameHook, dStage_setPlyrParamHook) | §701 finding, Housing/Engine |

## WAVE 0 — leaves, movable on P1 alone

| TU | (b) uniq | riders |
|---|---|---|
| ww_import_gate | 0 | №31-C semantics move WITH it |
| ww_jpa | 0 | — |
| ww_cam_crawl | 1 | — |
| ww_profile_register | 0 | profile-DATA seam ruling (P5) first |
| d_ext_room_verify | 2 | №31-C; npc_mount includes its header (intra-plugin later, fine) |
| d_ext_save_flags | 0 | RE-REVIEW: on mixed list but data-only leg — probable misclassification |

## WAVE 1 — low-coupling, after P1 (+P3 where GX-marked)

esa (13u) · spc_item01 (16u) · d_ext_npc_doors (17u; inherits №31-C) ·
lamp (18u) · ww_actor_shims (21u — move EARLY: it IS the import table) ·
otble (22u) · npc_population (7u; heaviest f_pc surface) ·
mdoext1_3dline (5u; GX×33 → P3) · ww_stage_loader (2u; **held by P4**)

## WAVE 2 — units that move whole

| unit | members | seam |
|---|---|---|
| ja1 stack | 22 ja1_* + d_ext_seq_space | dusk::audio (P2) + P5 ruling |
| evt1 stack | 3 evt1_* | P5 ruling; zero dusk:: — cleanest group |
| kankyo pair | d_kankyo_ww (mixed) + d_kankyo_ww_sky | GX×155 → P3 |
| demo triple | ww_demo00 + d_a_demo00 (mixed) + d_demo (mixed) | leg-strip first |

## WAVE 3 — (b)-heavy actors, rank by export coverage after P1

kamome (39u; (e)-dominant, earliest) · d_door (44u; **held by f_op_msg_mng
leg**) · knob00 (47u) · toripost (50u) · kb (58u) · ba1 (59u) · bm1 (69u) ·
ls1 (74u; msg leg) · zl1 (80u) · d_npc (11u but .inc inclusion + msg leg)

## WAVE 4 — hub + heavy plumbing, LAST

ww_room_loader (seams infrastructure; rider TU) · d_ww_itemmdl_pc (+dispatch
seam; dusk:: ×12) · veg/tree (GX) · ww_jpa_bind (ctor/vtable link class) ·
**d_ext_npc_mount (378 sites, 108u, couples four stacks — the terminal TU)**

## №31-C carriers

d_ext_room_verify (source) · ww_import_gate (source) · d_ext_npc_mount
(inherits). Rule text: absent manifest ⇒ UNKNOWN, never PASS — rides each row.

## Housing leaf-list verdict

Partially confirmed (Wave 0 = real leaves) and partially overturned:
ww_stage_loader and mdoext1_3dline look leaf-like by (b) count but are held by
P4/P3 — the per-TU table sees what a leaf-list cannot.
