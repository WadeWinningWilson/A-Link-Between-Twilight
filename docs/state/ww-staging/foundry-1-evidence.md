# foundry-1-evidence — 19a v2, condensed decision-grade extract

> Full 50-row per-TU table produced by the evidence agent 2026-08-11 (92k-token
> read, 17 tool uses). This file carries the RANKING-RELEVANT extract + all
> cross-cutting findings. Roster correction: **62 excluded + 11 mixed = 73 TUs**
> (foundry-1.md said 59 — stale by 3).

## THE LOAD-BEARING FINDING (§700 rider — invalidates part of the L2a spec)

`wwRoom_aliasArcFileName` is **`static` — file-local** (ww_room_loader.cpp:548,
installed :694). It will NEVER appear in the symbol manifest, so the L2a
design's `DEFINE_HOOK` on it **cannot work as specified** (HANDOFF-HOUSING-
ENGINE.md:174 and ttw-methods-review.md:394 both flagged the doubt; confirmed).
**The reachable seam is the SETTER**: `dRes_setArcFileNameHook`
(d_resorce.h:23, d_resorce.cpp:58, external linkage) — the plugin installs its
own alias fn. Same pattern applies to `dStage_setPlyrParamHook`. **L2a must be
re-specified against the setters.** → HOUSING/ENGINE.

## Taxonomy corrections (v2 must adopt)

1. **(a) SERVICE = 0 today.** No WW TU includes sdk/include/mods/. The ~60
   "(a)-ish" hits are `dusk::` host-internal (settings/ConfigPath/custom_assets/
   audio/ui). They BECOME (a) only when SDK services exist — a prerequisite,
   not a classification. Densest: d_ext_npc_mount ×14, ja1_bank ×11,
   d_ww_itemmdl_pc ×12.
2. **(c) = 0 on this roster.** Only resolve machinery is ww_import_gate's
   INJECTED resolver. v1's "(c)=15" is not visible in these 73 files.
3. **GX is an unmodelled FOURTH link family** (~470 extern call sites:
   d_particle 191, d_kankyo_ww_sky 155, mdoext1_3dline 33, veg 30, tree 30,
   itemmdl_pc 28). Needs its own taxonomy cell or it misfiles as (e).
   NOTE: d_particle.h:148 calls GXSetTevColor in an inline member — every
   includer inherits GX link demand invisibly.
4. **Profile-table data coupling** (ww_profile_register → f_pc_profile_lst):
   linker-visible DATA, not calls — no cell in a/b/c/e. Taxonomy extension.

## Structural blockers

- **Dependency INVERSIONS (receiver→WW), hardest blockers:**
  d_stage.cpp (KEPT) includes ww_stage_loader.h AND d_ext_npc_mount.h;
  d_particle.cpp (KEPT) includes ww_jpa.h.
- **Excluded/KEPT pairings needing rulings:** ja1_jaschannel vs JASChannel
  (KEPT); evt1_event_data/manager vs d_event_data/manager (KEPT);
  ww_profile_register vs f_pc_profile_lst (KEPT).
- **Hub:** d_ext_npc_mount.cpp — 11,849 LOC, 53 headers, 378 call sites
  (108 uniq), couples audio+room-loader+dmesg+itemmdl. **Migrates last.**
- f_op_msg_mng.cpp (mixed) blocks npc_ls1, d_door, d_npc (scope-msg leg).
- Pairs that move as units: kankyo_ww + kankyo_ww_sky; d_ext_seq_space + the
  22-TU ja1 stack; demo triple (ww_demo00 + d_a_demo00 + d_demo).

## First-movers (evidence-ranked, lowest receiver coupling first)

```
ww_import_gate.cpp     0 calls, 1 hdr   №31-C rides with it
ww_jpa.cpp             0 calls, 2 hdr
d_ext_save_flags.cpp   0 calls, 1 hdr   mixed-list — probably MISCLASSIFIED (data-only leg)
ww_profile_register    0 calls          but profile-DATA coupling (see above)
ww_cam_crawl.cpp       1 call,  5 hdr
d_ext_room_verify.cpp  2 calls          №31-C; npc_mount includes its header
ww_stage_loader.cpp    2 calls          BLOCKED by d_stage inversion
```

## The ready-made export-surface probe

**d_ext_ww_actor_shims.cpp** = a hand-written import table: 297 LOC, 23 sites,
21 unique receiver names, zero macros, depended on by 12+ roster TUs. Diff its
21 names against `symbol_manifest.py <image> --find` output = the first
concrete export-surface delta. Every (b) row is NOT-VERIFIED until that run
(no checked-in exports listing; manifest is embedded post-link).

## Notable per-TU (beyond first-movers; full table in agent transcript)

- npc actors are (b)-heavy, event-bit dominant: bm1 197(69u), zl1 163(80u),
  ls1 143(74u), kb 182(58u) — late migrators, rank by export coverage.
- kamome: (e)-dominant (94 REG*/SQUARE macros vs 84 calls) — early IF its 39
  uniq (b) names export.
- ja1 aggregate: 22 TUs, ~11.1k LOC, prefix-(b)=13 but 649 member calls
  (mostly internal); true external seam = dusk::audio + d_ext_seq_space.
- evt1 aggregate: 25(14u) calls, zero host-namespace use — cleanest group.
- d_stage.cpp OBJNAME ×938 = one pure-(e) data table; isolating it shifts the
  campaign's (e) number materially.
- d_npc.cpp includes d_npc_event_cut.inc — one-TU-one-file tooling miscounts.
- knob00/ww_actor_shims have duplicate-include blocks (merged sections).
