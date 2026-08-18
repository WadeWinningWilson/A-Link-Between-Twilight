# OWNERSHIP ADJUDICATION WORKSHEET — the WW layer, decided file by file

> **GENERATED** by `scratchpad/gen_worksheet.py` from `layer_census`'s own
> four partitions (a fifth reimplementation would be the roster failure
> this worksheet ends). Regenerate rather than hand-edit the evidence
> columns; the VERDICT column is the adjudicators' and survives regen.
> **PROPOSALS ARE PROPOSALS** — pre-filled only where every mechanical
> signal agrees; blank where they conflict, because the conflicts are
> exactly where judgment is owed.

> **USER RULINGS ENTERED 2026-08-16:** quick-equip and the status/outfit headers are **ALBW**. **The entire skins track is ALBW work** - leftovers of WW code in retail TP (the ~21 WW inventory items) that the ALBW mod uses to reskin items like the bow and iron boots (Blender-edited renditions loaded via BMDs). It is OUT of WW migration accounting. The three `sw` overlap files are DUAL-PENDING - the user cannot delineate skins-vs-port there; History/Housing digging decides. **A regen of the evidence columns must MERGE these verdicts, not overwrite them** - the generator does not yet do this on its own.

>
> Membership key: `STRIP NAME EXTD LIN` — X = in that partition.
> commits: attribution of the file's history (ww / other / both / none).
> t2 = tier2 B-class in-place WW edit sites. Routing: **HISTORY/BRIDGE**
> = donor lineage (their R5/KIT domain) · **HOUSING** = §491 leg-carriers
> and per-hunk splits · **USER** = ALBW/dual and the skins track.

Totals: universe 162 files · **152 DISPUTED** (in some partitions, not all)
· 10 agreed (all four) · 121 of the disputed carry an evidence-agreed
proposal, 31 need genuine adjudication.

## Verdict vocabulary — TWO AXES (revised 2026-08-16, adjudication round 1)

> The single-column vocabulary broke on its first contact with real rulings:
> History/Bridge showed `WW-PORT` was an axis-A answer being read as an
> axis-B claim (the `ja1_*` bridges migrate with the WW layer AND are our
> reconstructions owed a native replacement — one column cannot say both),
> and the Integrator showed `NOT-WW?` was hiding two opposite categories
> (upstream's own file vs. our non-WW infrastructure). A verdict is now a
> pair `A/B`; existing single-term verdicts remain readable as their A axis.

**Axis A — WHERE DOES IT GO:** `PLUGIN` (migrates) · `FORK` (stays ours,
non-WW or host-side) · `UPSTREAM` (origin's file — leaves this universe) ·
`PATCH` (seam-tracker ABSENT class) · `SPLIT` (per-hunk, both destinations)

**Axis B — WHAT IS IT: PARSED FROM `KIT-LINEAGE`, NOT INVENTED HERE.**
*(Corrected 2026-08-16: the first revision of this section coined six terms
while 102 of these 162 files already carry the vocabulary in their own
headers — the fifth-roster failure, committed in the document that warns
against it. History/Bridge found it; the Integrator verified independently.)*

The in-tree value domain, measured on both scopes (109 files tree-wide, 102
inside this universe — different denominators, neither wrong):
`native-port` (56 in-universe) · `host-plumbing` (27) · `mixed` (11) ·
`donor-port` (4) · **`bridge-owed:§N` (4)** — and the bridge term carries
the owed-native-replacement citation INLINE, in the file, which outlives
this worksheet. Companions on the same lines: `KIT-DONOR:` (none /
per-hunk / named donor TU) and `KIT-DONOR-STATUS:` (Matching / MatchingFor
/ NonMatching / UNKNOWN).

Consequences: **axis B is a PARSE for the 102 tagged files; the axis-B
judgment set is the ~60 headerless files only** — where the non-WW classes
live (`ALBW`, `NATIVE-TP`, upstream files, instruments), since non-WW files
never carry a KIT header. "Done" maps to donor lineage + `KIT-DONOR-STATUS:
Matching`, never to a worksheet stamp. **One semantic question is HISTORY's
to answer, not inferred here: the precise `native-port` vs `donor-port`
distinction** — the tags are their §426 vocabulary and guessing it would
repeat tonight's label-misreading errors. The four self-declared bridges:
`d_ext_ww_actor_shims.cpp` (§223) · `ja1_parser.cpp` / `ja1_seq_ctrl.cpp` /
`ja1_track.cpp` (§369). **There are NO undeclared bridge files** — the
"~15 owed headers" queue item died with the family generalisation it was
built on (Integrator self-correction, 08-16): the other ja1 sources are
`native-port`/`host-plumbing` and already declare themselves correctly.
**The owed list is exactly these four, complete, with citations.**

Worked examples from the rulings — **read the FILE's header, never a
family generalisation** (History/Bridge withdrew "the ja1 layer is a
bridge" against the tree's own tags: **only 3 of 22 ja1 sources are
`bridge-owed`; 19 are `native-port` or `host-plumbing`** — a family-level
claim was wrong for 16 files while every per-file header was right):
`ja1_parser/seq_ctrl/track.cpp` = `PLUGIN / bridge-owed:§369` ·
the other ja1 sources = per their own headers · `d_a_tag_so` =
`PLUGIN / donor lineage, KIT-DONOR-STATUS decides "done"` ·
`d_a_grass.cpp` = **`INSTRUMENT` — CORRECTED 2026-08-16, and my earlier
`UPSTREAM-NATIVE?` here was WRONG IN A DANGEROUS DIRECTION.** I counted WW
TOKENS (2, both inside comment text) and concluded the file "differs from
upstream by a comment". **It differs by a LIVE PROBE**: a throttled
`OS_REPORT` block in `daGrass_c::execute()` (`static u32 s_gN`, fires every
120th call). **A WW-token grep cannot see a probe that never says "WW" —
wrong instrument for the question, which was a DIFF against upstream and
was never run.** I had recommended deleting "our comment"; **that would have
orphaned or removed live instrument code, and `upstream-native` would have
HIDDEN A STRIP OBLIGATION** — this file must be stripped before push, which
is the opposite of "leaves the universe". Seeded by the Integrator's token
count, amplified by me, and caught only when they diffed ·
`boot_stage.cpp` = `FORK / host-plumbing` **with the
caveat that it defines `dBootStage_add` (:111), a seam-tracker seed
symbol — the file is not WW debt, but one symbol in it is a WW doorway.**

## DISPUTED — the adjudication set (152)

| file | S N E L | commits | t2 | flags | PROPOSAL | route | VERDICT |
|---|---|---|---|---|---|---|---|
| `include/d/actor/d_a_e_ww.h` | `X . . .` | none |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/actor/d_a_ww_demo00.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_albw_dialogue.h` | `X . . .` | both |  | La | SHARED (per-hunk) | HOUSING | ALBW (Housing 08-16: zero WW refs) |
| `include/d/d_ext_dmesg.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_fado_door.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_mod_flags.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_npc_doors.h` | `X X . .` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_npc_mount.h` | `X X . .` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_npc_population.h` | `X X . .` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_quick_equip.h` | `X X . .` | other |  | - | **?** | USER | ALBW (user 08-16: quick-swap work) |
| `include/d/d_ext_room_verify.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_save_flags.h` | `X X . .` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_save_flags_route.h` | `X X . .` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_save_guard.h` | `X X . .` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_scope_msg.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_seq_space.h` | `X X . .` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_status.h` | `X X . .` | other |  | - | **?** | USER | ALBW (user 08-16: outfit-tab work) |
| `include/d/d_ext_tree.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_ext_ww_actor_shims.h` | `X X . .` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_kankyo_ww.h` | `X . . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_kankyo_ww_sky.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_kankyo_ww_wind.h` | `. X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/d_menu_ext_status.h` | `. X . .` | other |  | - | **?** | USER | ALBW (user 08-16: outfit-tab work) |
| `include/d/d_ww_itemmdl_pc.h` | `X X . .` | both |  | s | WW-SKINS | USER | ALBW-SKINS (user 08-16) |
| `include/d/d_ww_itemmdl_test.h` | `X X . .` | other |  | - | WW-PORT | HISTORY/BRIDGE | ALBW-SKINS (History 08-16: flipped from WW-PORT — namespace dWwItemmdl, editor bow-demo replay helper; matches its own .cpp sibling; skins regex needs the underscore and this header has none) |
| `include/d/ext_evt/evt1_boundary.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_line/mdoext1_3dline.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_cam_crawl.h` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_cam_data.h` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_cam_select.h` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_import_gate.h` | `X X . .` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_item_data.h` | `. X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_itemmdl_dispatch.h` | `X X . .` | other |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_itemmdl_impl_names.h` | `X X . .` | other |  | s | WW-SKINS | USER | ALBW-SKINS (user 08-16) |
| `include/d/ext_plugin/ww_itemmdl_test_names.h` | `X X . .` | other |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_npcmount_dispatch.h` | `X X . .` | other |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_npcmount_impl_names.h` | `X X . .` | other |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_profile_register.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_room_loader.h` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_stage_loader.h` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_plugin/ww_tsubo_data.h` | `. X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_bank.h` | `X . . .` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_boundary.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_dsp_boundary.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_event_dump.h` | `X . . .` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_jasbank.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_jaschannel.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_jasseqparser.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_jastrack.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_native.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_oscillator.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_parser.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_seq_ctrl.h` | `X . . .` | ww |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ext_seq/ja1_track.h` | `X . . .` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `include/d/ww_jpa.h` | `X X . .` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_bg.cpp` | `. . . X` | both | 5 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_demo00.cpp` | `X . . X` | both | 1 | sw | WW-SKINS | USER | FORK / ALBW-skins — NO plugin share. 1 skins hunk (:1700-1708 cel-shade ambient) + its donor-derived helper (:1654-1682 demoModelUsesZAtoon, KIT-DONOR-HUNK d/d_resorce.cpp MatchingFor) which serves the SKINS path. AXIS-A FORK, AXIS-B donor-derived — the axes decouple here. Rest native (History 08-16) |
| `src/d/actor/d_a_demo_item.cpp` | `X . . .` | both |  | s | WW-SKINS | USER | ALBW-SKINS edits in native TU (user 08-16) |
| `src/d/actor/d_a_esa.cpp` | `X . . X` | ww | 4 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ext_ep.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ext_plank_span.cpp` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ext_vegetation.cpp` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_grass.cpp` | `X . . .` | other |  | - | **?** | HISTORY/BRIDGE | INSTRUMENT |
| `src/d/d_ev_camera.cpp` | `X . . .` | other |  | - | **?** | HISTORY/BRIDGE | INSTRUMENT (History 08-17, seam-gate live find): NO WW port here — `grep CRAWL` returns nothing, the CRAWL port is in the ACTORS (d_a_alink/d_a_kb/d_a_npc_p1/d_a_b_gm). All 5 §-tagged sites are probes (§903/§983/§984); line 4 says `probe (strip before push) — NEVER-PUSH-STRIP-SET`. Its one isWwHostStage is the probe's own scope. Should not be in a patch envelope; it is on the strip set. |
| `src/d/d_s_play.cpp` | `X . . .` | other |  | sw | **?** | HISTORY/BRIDGE | SPLIT (History 08-17, seam-gate live find): WW-PORT hunks = dExtWw_drawDemoMessage :782 + dExtWw_pollDemoMessage :855 (§49 storyboard dialogue) · ALBW-SKINS hunks = dWwItemmdl::tickBowGetItemDemoReplay + dWwItemmdl_tickHeldBowArcMount :843-844 (skins track, user-ruled OUT of WW accounting) · rest NATIVE. NOTE: 0 isWwHostStage in the file; callees self-gate on STATE/SETTING, not stage — inert today, recorded as a known scope-shape. |
| `src/d/actor/d_a_kamome.cpp` | `X . . X` | ww | 3 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_kb.cpp` | `X . . X` | ww | 4 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_knob00.cpp` | `X . . X` | ww | 12 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_lamp.cpp` | `X . . X` | ww | 4 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_lwood.cpp` | `. . . X` | ww | 6 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_npc_ba1.cpp` | `X . . X` | both | 5 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_npc_bm1.cpp` | `X . . X` | both | 17 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_npc_jb1.cpp` | `. . . X` | ww | 5 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_npc_ls1.cpp` | `X . . X` | both | 10 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_npc_p1.cpp` | `. . . X` | ww | 13 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_npc_zl1.cpp` | `X . . X` | both | 7 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_obj_lpalm.cpp` | `. . . X` | ww | 5 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_obj_mshokki.cpp` | `X . . X` | ww | 3 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_obj_otble.cpp` | `X . . X` | ww | 5 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_obj_paper.cpp` | `. . . X` | ww |  | w | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_obj_plant.cpp` | `. . . X` | ww | 3 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_obj_shelf.cpp` | `. . . X` | ww | 4 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_obj_toripost.cpp` | `X . . X` | both | 4 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_spc_item01.cpp` | `X . . X` | ww | 4 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_swhit0.cpp` | `X . . X` | both | 3 | wL | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/actor/d_a_tag_kb_item.cpp` | `. . . X` | ww |  | w | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_tag_so.cpp` | `. . . X` | ww |  | w | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_vrbox.cpp` | `X . . X` | both | 8 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_vrbox2.cpp` | `X . . X` | both | 6 | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ww_demo00.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ww_item.cpp` | `. X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ww_shutter.cpp` | `. X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/actor/d_a_ww_tsubo.cpp` | `. X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_albw_dialogue.cpp` | `X . . X` | both |  | La | SHARED (per-hunk) | HOUSING | ALBW (Housing 08-16: only KIT boilerplate, DONOR none) |
| `src/d/d_camera.cpp` | `X . . .` | both | 3 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_demo.cpp` | `X . . X` | both |  | sw | WW-SKINS | USER | SPLIT — skins 1 hunk (emitter_create :1505-1512, get-item beam suppress) -> ALBW · port 4 WW-host-gated hunks (:121 handleDemoMessage · :140 dExtDmesg_setMessage · :497 donor transform KIT-DONOR-HUNK NonMatching · :1087 WW_DEMO00 route) -> PLUGIN · rest native (History 08-16, read per hunk) |
| `src/d/d_door.cpp` | `X . . X` | ww | 5 | wL | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_event.cpp` | `. . . X` | both | 3 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (History 08-17, adjudicated on the Integrator's global-refusal escalation). 23 added lines / 0 deleted, all additive, three sites: `dExtWwEvt_getStageEventNameForRecord` :873 and :932, `dExtWwSave_isWwHostStage(sn717)` :1186, plus the include :20. Same shape as its two siblings, so same verdict. **WW-SCOPING RULE SATISFIED, AND BY A STRONGER MECHANISM THAN THE RULE NAMES** — :873/:932 look null-gated but the resolver `wwEvntFullNameForRecord` (ww_stage_loader.cpp:367) returns NULL unless the record pointer lies INSIDE the seam's own `s_evntPool`. That is POINTER-RANGE IDENTITY: it asks "is THIS RECORD one the WW seam produced?", per record, where `isWwHostStage` only asks "are we on a WW stage?" globally — which can be true while a given record is still a TP record. A vanilla record returns NULL and the call site's `else` runs the receiver's original path untouched. **DO NOT "fix" these two sites by adding an isWwHostStage check; it would be redundant and weaker.** |
| `src/d/d_event_data.cpp` | `. . . X` | ww | 1 | wL | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_event_manager.cpp` | `. . . X` | both | 7 | w | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_ext_dmesg.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_mod_flags.cpp` | `X X . X` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_npc_doors.cpp` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_npc_mount.cpp` | `X X . X` | both |  | sw | WW-SKINS | USER | PLUGIN / WW-port throughout — NOT dual. Its 5 dWwItemmdl_ sites (:7374-:7737) are WW clothes-get MESSAGE PRESENTATION driven by donor msg 3095/4410 (native item box mFukiKind 9), i.e. PORT hunks calling the skins module's flag API. A skins MARKER is not a skins HUNK. DN-9 retirement unaffected (History 08-16) |
| `src/d/d_ext_npc_population.cpp` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_room_verify.cpp` | `X X . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_save_flags.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_seq_space.cpp` | `X X . X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_tree.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_ext_ww_actor_shims.cpp` | `X X . X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_kankyo_rain.cpp` | `X . . .` | both | 4 | - | SHARED (per-hunk) | HOUSING | SHARED per-hunk CONFIRMED (Housing 08-16: dKyWw_isSkyHost + ww_vrkumo.inc; flags UNDER-detected) |
| `src/d/d_kankyo_ww.cpp` | `X . . X` | both |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_kankyo_ww_sky.cpp` | `X X . X` | both |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_kankyo_ww_wind.cpp` | `. X . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_npc.cpp` | `. . . X` | ww |  | w | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/d_particle.cpp` | `X . . X` | both | 2 | L | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_stage.cpp` | `X . . X` | both | 8 | wL | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/d/d_ww_itemmdl_pc.cpp` | `X X . X` | both |  | sw | WW-SKINS | USER | ALBW-SKINS (user 08-16) |
| `src/d/d_ww_itemmdl_test.cpp` | `X X . X` | both |  | s | WW-SKINS | USER | ALBW-SKINS (user 08-16) |
| `src/d/ext_evt/evt1_boundary.cpp` | `X . X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_evt/evt1_event_data.cpp` | `X . X X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_evt/evt1_event_manager.cpp` | `X . X X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_line/mdoext1_3dline.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_event_names.cpp` | `. X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_item_data.cpp` | `. X X X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_layer_select.cpp` | `. X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_tsubo_data.cpp` | `. X X X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_bank.cpp` | `X . . X` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_event_dump.cpp` | `X . . X` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasbank.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasbnkparser.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jascalc.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jaschannel.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasdspchannel.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasnotemgr.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasouterparam.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasplayer.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasregisterparam.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasseqctrl.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jasseqparser.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jastrack.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jastrackinterrupt.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_jastrackport.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_kernel.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_native.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_oscillator.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_parser.cpp` | `X . . X` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_seq_ctrl.cpp` | `X . . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_seq/ja1_track.cpp` | `X . . X` | both |  | - | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/d/ww_jpa.cpp` | `X X . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ww_jpa_bind.cpp` | `X X . X` | ww |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/dusk/boot_stage.cpp` | `. . . X` | other |  | - | **?** | HISTORY/BRIDGE | FORK / host-plumbing (self-declared line 1; Integrator 08-16; dBootStage_add stays a seam seed) |
| `src/dusk/event_ext_store.cpp` | `. X . .` | ww |  | w | **?** | HISTORY/BRIDGE | WW-PORT |
| `src/f_op/f_op_msg_mng.cpp` | `X . . X` | both | 2 | wL | SHARED (per-hunk) | HOUSING | SHARED per-hunk (Housing 08-16) |
| `src/f_pc/f_pc_profile_lst.cpp` | `. . . X` | both |  | L | SHARED (per-hunk) | HOUSING | SHARED per-hunk CONFIRMED (Housing 08-16: WW_DEMO00 slot already served by dWwProfileRegister_lookup) |

## AGREED (all four partitions) — confirm-only (10)

| file | S N E L | commits | t2 | flags | PROPOSAL | route | VERDICT |
|---|---|---|---|---|---|---|---|
| `src/d/ext_plugin/ww_cam_crawl.cpp` | `X X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_cam_data.cpp` | `X X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_cam_select.cpp` | `X X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_import_gate.cpp` | `X X X X` | ww |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_itemmdl_dispatch.cpp` | `X X X X` | other |  | s | WW-SKINS | USER | ALBW-SKINS (user 08-16) |
| `src/d/ext_plugin/ww_misc_dispatch.cpp` | `X X X X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_npcmount_dispatch.cpp` | `X X X X` | other |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_profile_register.cpp` | `X X X X` | both |  | - | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_room_loader.cpp` | `X X X X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |
| `src/d/ext_plugin/ww_stage_loader.cpp` | `X X X X` | both |  | w | WW-PORT | HISTORY/BRIDGE | WW-PORT |

## Why each proposal was or was not made (per-row rationale, disputed set)

- `include/d/actor/d_a_e_ww.h` — ww-token name (underscore-safe) + 1/4 partitions
- `include/d/actor/d_a_ww_demo00.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/d_albw_dialogue.h` — §491 leg-carrier / flagged dual-purpose
- `include/d/d_ext_dmesg.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_fado_door.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_mod_flags.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_npc_doors.h` — WW code + mixed commits + 2/4 partitions
- `include/d/d_ext_npc_mount.h` — WW code + mixed commits + 2/4 partitions
- `include/d/d_ext_npc_population.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_quick_equip.h` — ext-named but NON-WW commit history - possibly an ALBW-era extension wearing the ext_ prefix
- `include/d/d_ext_room_verify.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_save_flags.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_save_flags_route.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_save_guard.h` — WW code + mixed commits + 2/4 partitions
- `include/d/d_ext_scope_msg.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_seq_space.h` — signals conflict or are thin - genuine adjudication
- `include/d/d_ext_status.h` — ext-named but NON-WW commit history - possibly an ALBW-era extension wearing the ext_ prefix
- `include/d/d_ext_tree.h` — WW-only commit history + 2/4 partitions
- `include/d/d_ext_ww_actor_shims.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/d_kankyo_ww.h` — ww-token name (underscore-safe) + 1/4 partitions
- `include/d/d_kankyo_ww_sky.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/d_kankyo_ww_wind.h` — ww-token name (underscore-safe) + 1/4 partitions
- `include/d/d_menu_ext_status.h` — ext-named but NON-WW commit history - possibly an ALBW-era extension wearing the ext_ prefix
- `include/d/d_ww_itemmdl_pc.h` — skins-track markers; uses TP's own assets, no donor disc
- `include/d/d_ww_itemmdl_test.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_evt/evt1_boundary.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_line/mdoext1_3dline.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_plugin/ww_cam_crawl.h` — ww-token name (underscore-safe) + 3/4 partitions
- `include/d/ext_plugin/ww_cam_data.h` — ww-token name (underscore-safe) + 3/4 partitions
- `include/d/ext_plugin/ww_cam_select.h` — ww-token name (underscore-safe) + 3/4 partitions
- `include/d/ext_plugin/ww_import_gate.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_item_data.h` — ww-token name (underscore-safe) + 1/4 partitions
- `include/d/ext_plugin/ww_itemmdl_dispatch.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_itemmdl_impl_names.h` — skins-track markers; uses TP's own assets, no donor disc
- `include/d/ext_plugin/ww_itemmdl_test_names.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_npcmount_dispatch.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_npcmount_impl_names.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_profile_register.h` — ww-token name (underscore-safe) + 2/4 partitions
- `include/d/ext_plugin/ww_room_loader.h` — ww-token name (underscore-safe) + 3/4 partitions
- `include/d/ext_plugin/ww_stage_loader.h` — ww-token name (underscore-safe) + 3/4 partitions
- `include/d/ext_plugin/ww_tsubo_data.h` — ww-token name (underscore-safe) + 1/4 partitions
- `include/d/ext_seq/ja1_bank.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_boundary.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_dsp_boundary.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_event_dump.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_jasbank.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_jaschannel.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_jasseqparser.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_jastrack.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_native.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_oscillator.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_parser.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_seq_ctrl.h` — signals conflict or are thin - genuine adjudication
- `include/d/ext_seq/ja1_track.h` — signals conflict or are thin - genuine adjudication
- `include/d/ww_jpa.h` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/actor/d_a_bg.cpp` — tier2 B-class host (5 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_demo00.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/actor/d_a_demo_item.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/actor/d_a_esa.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_ext_ep.cpp` — WW-only commit history + 3/4 partitions
- `src/d/actor/d_a_ext_plank_span.cpp` — WW code + mixed commits + 3/4 partitions
- `src/d/actor/d_a_ext_vegetation.cpp` — WW code + mixed commits + 3/4 partitions
- `src/d/actor/d_a_grass.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/actor/d_a_kamome.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_kb.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_knob00.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_lamp.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_lwood.cpp` — tier2 B-class host (6 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_npc_ba1.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_npc_bm1.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_npc_jb1.cpp` — tier2 B-class host (5 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_npc_ls1.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_npc_p1.cpp` — tier2 B-class host (13 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_npc_zl1.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_obj_lpalm.cpp` — tier2 B-class host (5 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_obj_mshokki.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_obj_otble.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_obj_paper.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/actor/d_a_obj_plant.cpp` — tier2 B-class host (3 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_obj_shelf.cpp` — tier2 B-class host (4 sites) - in-place edits, moves per-hunk
- `src/d/actor/d_a_obj_toripost.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_spc_item01.cpp` — WW-only commit history + 2/4 partitions
- `src/d/actor/d_a_swhit0.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/actor/d_a_tag_kb_item.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/actor/d_a_tag_so.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/actor/d_a_vrbox.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_vrbox2.cpp` — WW code + mixed commits + 2/4 partitions
- `src/d/actor/d_a_ww_demo00.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/actor/d_a_ww_item.cpp` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/actor/d_a_ww_shutter.cpp` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/actor/d_a_ww_tsubo.cpp` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/d_albw_dialogue.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/d_camera.cpp` — tier2 B-class host (3 sites) - in-place edits, moves per-hunk
- `src/d/d_demo.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/d_door.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/d_event_data.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/d_event_manager.cpp` — tier2 B-class host (7 sites) - in-place edits, moves per-hunk
- `src/d/d_ext_dmesg.cpp` — WW-only commit history + 3/4 partitions
- `src/d/d_ext_mod_flags.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/d_ext_npc_doors.cpp` — WW code + mixed commits + 3/4 partitions
- `src/d/d_ext_npc_mount.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/d_ext_npc_population.cpp` — WW code + mixed commits + 3/4 partitions
- `src/d/d_ext_room_verify.cpp` — WW-only commit history + 3/4 partitions
- `src/d/d_ext_save_flags.cpp` — WW-only commit history + 3/4 partitions
- `src/d/d_ext_seq_space.cpp` — WW code + mixed commits + 3/4 partitions
- `src/d/d_ext_tree.cpp` — WW-only commit history + 3/4 partitions
- `src/d/d_ext_ww_actor_shims.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/d_kankyo_rain.cpp` — tier2 B-class host (4 sites) - in-place edits, moves per-hunk
- `src/d/d_kankyo_ww.cpp` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/d_kankyo_ww_sky.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/d_kankyo_ww_wind.cpp` — ww-token name (underscore-safe) + 2/4 partitions
- `src/d/d_npc.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/d_particle.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/d_stage.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/d/d_ww_itemmdl_pc.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/d_ww_itemmdl_test.cpp` — skins-track markers; uses TP's own assets, no donor disc
- `src/d/ext_evt/evt1_boundary.cpp` — WW-only commit history + 3/4 partitions
- `src/d/ext_evt/evt1_event_data.cpp` — WW-only commit history + 3/4 partitions
- `src/d/ext_evt/evt1_event_manager.cpp` — WW-only commit history + 3/4 partitions
- `src/d/ext_line/mdoext1_3dline.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_plugin/ww_event_names.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/ext_plugin/ww_item_data.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/ext_plugin/ww_layer_select.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/ext_plugin/ww_tsubo_data.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/ext_seq/ja1_bank.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/ext_seq/ja1_event_dump.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/ext_seq/ja1_jasbank.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasbnkparser.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jascalc.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jaschannel.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasdspchannel.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasnotemgr.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasouterparam.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasplayer.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasregisterparam.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasseqctrl.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jasseqparser.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jastrack.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jastrackinterrupt.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_jastrackport.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_kernel.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_native.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_oscillator.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_parser.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/ext_seq/ja1_seq_ctrl.cpp` — WW-only commit history + 2/4 partitions
- `src/d/ext_seq/ja1_track.cpp` — signals conflict or are thin - genuine adjudication
- `src/d/ww_jpa.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/d/ww_jpa_bind.cpp` — ww-token name (underscore-safe) + 3/4 partitions
- `src/dusk/boot_stage.cpp` — signals conflict or are thin - genuine adjudication
- `src/dusk/event_ext_store.cpp` — signals conflict or are thin - genuine adjudication
- `src/f_op/f_op_msg_mng.cpp` — §491 leg-carrier / flagged dual-purpose
- `src/f_pc/f_pc_profile_lst.cpp` — §491 leg-carrier / flagged dual-purpose

## Standing cautions carried into this sheet

- Flags column: `s`=skins markers · `w`=WW code (comments stripped before
  matching — the `d_msg_object` lesson) · `L`=leg-carrier · `a`=albw name
  · `!`=file absent from tree.
- Name matching here does NOT use `\b` word boundaries — three defects in
  one session came from underscores being word characters.
- Commit attribution is the 2%%-false-negative heuristic; it is EVIDENCE,
  never a verdict.

---

## HISTORY/BRIDGE ADJUDICATION NOTE (2026-08-16) — all 131 rows ruled

**103 evidence-agreed proposals CONFIRMED · 28 judged · 2 verdicts outside the
vocabulary (see below).** Evidence columns untouched; HOUSING's 19 and the USER's
12 untouched.

### The 28, by class — the reason, not just the verdict

- **17 `ja1_*` (JAudio1) + `evt1_boundary.h` + `mdoext1_3dline.h` → `WW-PORT`.**
  They migrate with the WW layer. **⚠ BUT THEY ARE OUR RECONSTRUCTIONS, NOT DONOR
  PORTS** — the standing directive on this project is that bridges
  (`ja1Voice` audio redirect and its family) are *owed a native replacement* and
  stay on the owed list. **`WW-PORT` will read as "donor-faithful" to anyone who
  did not write it, and for these files that is false.** The verdict is right for
  *migration*; it is wrong as a provenance claim.
- **`d_a_tag_so.cpp` · `d_a_tag_kb_item.cpp` · `d_a_obj_paper.cpp` → `WW-PORT`.**
  These are LINEAGE-only **by design, not by weak evidence**: donor-derived
  exports keep donor-native names on this project, so a donor actor port is
  *invisible to every name-based partition by policy*. **The generator could
  encode that: LINEAGE-only + a donor TU of the same name is a strong WW-PORT
  signal, not a conflict.**
- **`d_npc.cpp` → `WW-PORT`, and it was already adjudicated in-file.** Its own
  header reads *"STEP 19: THIS TU MOVES TO THE PLUGIN. Established by comparing
  against dusklight main rather than by reading the code, which misled me twice
  — dusklight main has NO d_npc.cpp at all, and none in its files.cmake."*
- **Remaining ext-layer machinery** (`d_ext_seq_space.h`, `d_ext_mod_flags.cpp`,
  `event_ext_store.cpp`) → `WW-PORT` on agreed ext/WW evidence.

### ⚠ TWO VERDICTS THE VOCABULARY CANNOT EXPRESS — `NOT-WW?`

`src/d/actor/d_a_grass.cpp` and `src/dusk/boot_stage.cpp` **are not WW at all.**

- `d_a_grass.cpp` contains **exactly 2 WW tokens in the whole file, and BOTH are
  in one comment** (`:374` *"NATIVE baseline witness … same-run TP/WW compare"*).
  It is a native TP actor and it is in **dusklight main's own `files.cmake`**.
  Strip-set membership earned by a comment.
- `boot_stage.cpp` is dusklight's own `--stage` dev entry — `KIT-DONOR: none`, no
  counterpart in the WW tree, non-WW commit history.

**The vocabulary has no term for "a partition claims this file and it is simply
not WW."** `STALE-PATH?` means the path is gone; these paths exist and are
native. **I have written `NOT-WW?` as a PROPOSED vocabulary addition rather than
force one of the six existing terms and lose the finding.**

**This converges with two independent findings and should be treated as a class,
not three incidents:** Foundry's `d_a_arrow.cpp` (*"only WW token was a comment
about a skins folder"*) and the Integrator's three USER rows that are disputed
because the letters `WW` appear inside the phrase **`WW-agnostic`**. **Three
lanes, three files, one defect: token presence is being read as ownership, and
comments are not code.** A generator that excluded comments from the token sweep
would resolve all of them mechanically.
