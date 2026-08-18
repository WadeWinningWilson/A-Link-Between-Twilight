# NEVER-PUSH STRIP SET — covenant hard-gate (§113)

> ## ✅ TIER-1 IS NOW GENERATED, NOT MAINTAINED (2026-08-06, roadmap steps 2+3)
> **The failure this replaces:** the hand list said **34 files** while the build
> carried **46**, with **36 unlisted** — the whole `ja1_*` JAudio port, the
> `evt1_*` event stack, `mdoext1_3dline`, `d_kankyo_ww_sky`, `d_ext_tree`,
> `ww_jpa`. Authored 2026-07-25, never maintained. That is the guaranteed end
> state of a hand list guarding a boundary that does not physically exist in the
> build, so it was NOT hand-patched.
>
> Tier-1's file lists are now **derived from `files.cmake`** — the build is the
> only authority on what actually ships. Regenerate after adding any TU:
> ```
> python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-tier1
> ```
> and gate it in CI / before any push:
> ```
> python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --check   # exit 1 if drifted
> ```
> **Basis, stated so it is never overread:** sources are build-derived
> (authoritative); headers are a directory scan (indicative — a header no TU
> includes still appears). Both classify by **filename convention, NOT declared
> lineage**. Roadmap step 10's provenance banners (History) upgrade this to real
> lineage; until then this manifest answers "what is WW-shaped", not "what is
> WW-derived".
>
> ## RULING — EVERY PORTED SUBSYSTEM FILE TAKES THE `d_ww_` PREFIX
> (Housing/Engine, 2026-08-17. **Recorded here on the Librarian's
> announce-vs-record doctrine — it was filed as a CALLS row, and a scrolling
> row cannot bind work that does not exist yet.**)
>
> **THE RULE:** a newly ported WW subsystem file is named `d_ww_<subsystem>`.
> Donor fidelity stays INSIDE the TU — symbols keep their donor names.
> Receiver convention governs the BOUNDARY — the filename.
>
> **WHY IT IS A COVENANT RULE AND NOT A STYLE PREFERENCE — stated precisely,
> because the first version of this paragraph overstated it and the next
> paragraph on this very page disproved it (History/Bridge, 2026-08-17):**
>
> A donor-faithful filename **matches no filename pattern**. It is caught
> anyway — but ONLY by the *other* half of the basis: the 2026-08-07 user
> ruling widened the strip set to the **UNION of filename convention AND
> declared lineage (KIT-LINEAGE banners)**.
>
> **So the exposure is not invisibility. It is SINGLE-MECHANISM DEPENDENCE.**
> A `d_ww_`-named file is caught by filename *and* by banner — two independent
> mechanisms. A donor-named file is caught by the banner ALONE, so it ships to
> public `main` the moment that banner is absent, malformed, or unparsed —
> **and a missing banner produces no error, because a file with no banner is
> indistinguishable from a file with nothing to declare.**
>
> *(A previous version of this paragraph cited "the push gate measured blind to
> 31 banner-declaring files" as the receipt for that sentence. **That finding
> was VOIDED by its own author** — Housing/Engine, 2026-08-17: "I ruled from
> the rule instead of the artifact, and Foundry's retraction killed its
> premise." **A voided measurement propping up a live rule is worse than no
> receipt at all**, so it is removed rather than softened. The claim above
> stands on its own construction — a file with no banner and no `d_ww_` name
> is not distinguishable from a file with nothing to declare, by inspection —
> and on the 4-of-65 count recorded below, which is live.)*
>
> **MEASURED — THE SINGLE-MECHANISM SET IS 4 FILES OF 65** (History/Bridge,
> 2026-08-17, on this paragraph's own claim):
>
> - **65** files in the strip set
> - **61** caught by **filename AND banner** — two independent mechanisms
> - **4** caught by the **BANNER ALONE**, each one absent-banner away from
>   shipping: `src/d/d_kankyo_ww.cpp` · `src/d/d_kankyo_ww_sky.cpp` ·
>   `src/d/d_kankyo_ww_wind.cpp` (WW leg inside a receiver-named TU) and
>   `src/d/d_albw_dialogue.cpp` (shared ALBW/WW dialogue surface)
>
> **Recorded here rather than left in a CALLS row**, per the announce-vs-record
> rule in [Librarian.md](Librarian.md): a justification that can be counted
> should be, and the count belongs beside the claim it supports. Measurement is
> History/Bridge's; placement is the Librarian's — amend freely.

> **THIS PAGE IS ITS OWN EVIDENCE.** The generated block below marks a dozen
> files *"filename rules MISS this"* — donor content in receiver-named TUs,
> visible **only** through declared lineage. **Those are the files this rule
> exists to stop creating.** The union basis is the backstop; the prefix
> restores the belt to go with it.
>
> **RATIFIED BY EXECUTION, not by agreement** — the Integrator ran the rules
> and confirmed the gap rather than concurring with the argument.
>
> **SCOPE:** binds work not yet written, by lanes that may not yet exist.
> That is precisely the class that cannot live in a CALLS row.

> ## RULING — THE M6 GATE'S SCOPE (Housing Security, 2026-08-06, roadmap step 4)
> The greplist gate scans the exe for WW **identity literals**. It cannot detect
> ported donor **logic** or donor **data constants**, because compiled code
> carries no such strings. It has therefore been making a TRUE claim about
> strings that READS as a claim about lineage — including in every
> `gate [('Ivan', 1)] PASS` reported during the §440-§486 campaign.
>
> **Every gate run must from now on report BOTH axes, verbatim:**
> ```
> COVENANT GATE — identity literals: CLEAN (or: BREACH, <hits>)
>                 code lineage:      UNKNOWN (not measured by this gate)
> ```
> `code lineage` stays **UNKNOWN** until the census (roadmap step 8) measures it;
> it is never reported as CLEAN by this instrument. A gate that cannot see a
> class of breach must say so in its own output — the §473/§480 rule that a
> clean log must mean "never fired", applied to the covenant gate itself.
> **EMITTER SHIPPED (2026-08-06, roadmap step 4 — Housing, Bridge being absent):**
> `tools/ww_crew_restoration_skeleton/covenant_gate.py`
> ```
> python tools/ww_crew_restoration_skeleton/covenant_gate.py --strict
> ```
> Reports both axes on every run, refuses to report `code lineage` as anything
> but UNKNOWN, and with `--strict` also fails on WW-layer manifest drift — a
> gate run against a stale roster says nothing about the files the roster forgot.
> NEGATIVE-CONTROLLED: run against a planted blob containing `Outset`/`Aryll` it
> reports `BREACH {'Outset': 1, 'Aryll': 1}` and exits 1. An instrument that has
> never produced a failure is not a verified instrument.

> ⛔ **Housing Security lane. This is the covenant's last line before a public
> push.** A bad merge is reversible (`git checkout pre-dusk-api-merge`); a bad
> **push is un-sendable** — donor bytes or a WW identity literal reaching the
> public `ALBW-Dusklight` remote cannot be recalled. Everything below stays
> **fork-local** and is **never staged for a push to public `main`** until it
> has passed an individual clean-slice promotion review.
>
> Authored by Housing Security on the merged tree (branch
> `integrate/dusk-api-coexist`), 2026-07-25 — the dusk-API coexist merge has
> landed, so §113's gate is now live, not anticipatory.

## Baseline (M6 gate run, this tree)

`strings dusklight.exe | grep -iE '\b(Ivan|Outset|Aryll|Tetra|Sturgeon|Windfall|Makar|Medli|WW-Crew|bmgres|Great Sea)\b'`
→ **0 hits, all patterns. Binary covenant CLEAN.** No WW identity/place literal
reaches the exe. The strip set exists to KEEP it that way across promotions.

---

## Tier 1 — Never-push SOURCE files (the WW receiver layer)

The files listed below are the fork-local WW-restoration receiver layer. They
exist legitimately in the fork; they are **never promoted to public `main` as a
set.** A single file leaves this set only by a deliberate clean-slice review
(§5) — not by riding along in a broad push.

**The counts live in the generated block, not in this sentence.** The previous
version of this paragraph hard-coded "34 tracked files" and was wrong by twelve
within days of being written — a number in prose is a number that rots. Read the
block.

<!-- BEGIN GENERATED TIER-1 — do not hand-edit.
     Regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-tier1
     Basis: UNION of filename convention AND declared lineage (KIT-LINEAGE
     banners), user-ruled 2026-08-07 after §573 measured the filename basis
     wrong in BOTH directions. Widest set on purpose: for a never-push list
     over-inclusion costs nothing and omission cannot be recalled.
     SOURCES from files.cmake (authoritative); HEADERS from a directory scan
     (indicative — a header no TU includes still appears).
     NOT sufficient on its own: a `mixed` TU is donor lines inside a
     receiver-owned file, so listing it here does not make the file
     strippable — see the leg list in cmake/ww_layer_exclude.cmake. -->

**Sources (96) — generated from `files.cmake`:**

- `src/d/actor/d_a_bg.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_demo00.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_esa.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_ext_ep.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_ep.cpp` — filename agrees
- `src/d/actor/d_a_ext_plank_span.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_bridge.cpp` — filename agrees
- `src/d/actor/d_a_ext_vegetation.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_grass.cpp` — filename agrees
- `src/d/actor/d_a_kamome.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_kb.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_knob00.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_lamp.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_lwood.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_ba1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_bm1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_jb1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_ls1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_p1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_npc_zl1.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_mshokki.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_otble.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_paper.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_plant.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_shelf.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_obj_toripost.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_spc_item01.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_swhit0.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_tag_kb_item.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_tag_so.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_vrbox.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_vrbox2.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/actor/d_a_ww_demo00.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_demo00.cpp` — filename agrees
- `src/d/actor/d_a_ww_item.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_item.cpp` — filename agrees
- `src/d/actor/d_a_ww_shutter.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_shutter.cpp` — filename agrees
- `src/d/actor/d_a_ww_tsubo.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_tsubo.cpp` — filename agrees
- `src/d/d_albw_dialogue.cpp` — shared ALBW/WW dialogue surface (§113 note) (filename basis; declares `host-plumbing`)
- `src/d/d_demo.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/d_door.cpp` — KIT-LINEAGE `native-port` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/d_ext_dmesg.cpp` — KIT-LINEAGE `mixed`, KIT-DONOR `per-hunk` — filename agrees
- `src/d/d_ext_mod_flags.cpp` — WW-restoration subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ext_npc_doors.cpp` — WW-restoration subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ext_npc_mount.cpp` — WW-restoration subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ext_npc_population.cpp` — WW-restoration subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ext_room_verify.cpp` — WW-restoration subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ext_save_flags.cpp` — KIT-LINEAGE `mixed`, KIT-DONOR `per-hunk` — filename agrees
- `src/d/d_ext_seq_space.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `none` — filename agrees
- `src/d/d_ext_tree.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_tree.cpp` — filename agrees
- `src/d/d_ext_ww_actor_shims.cpp` — KIT-LINEAGE `bridge-owed`, KIT-DONOR `none` — filename agrees
- `src/d/d_kankyo_ww.cpp` — KIT-LINEAGE `mixed`, KIT-DONOR `per-hunk` — filename agrees
- `src/d/d_kankyo_ww_sky.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_kankyo_rain.cpp` — filename agrees
- `src/d/d_kankyo_ww_wind.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_kankyo_wether.cpp:985-1140` — filename agrees
- `src/d/d_particle.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/d_stage.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU
- `src/d/d_ww_itemmdl_pc.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/d_ww_itemmdl_test.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_evt/evt1_boundary.cpp` — JEvent1 parallel donor stack (WW events) (filename basis; declares `host-plumbing`)
- `src/d/ext_evt/evt1_event_data.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_event_data.cpp` — filename agrees
- `src/d/ext_evt/evt1_event_manager.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_event_manager.cpp` — filename agrees
- `src/d/ext_line/mdoext1_3dline.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `m_Do/m_Do_ext.cpp` — filename agrees
- `src/d/ext_plugin/ww_cam_crawl.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_cam_data.cpp` — WW-specific receiver subsystem (filename basis; declares `donor-port`)
- `src/d/ext_plugin/ww_cam_select.cpp` — WW-specific receiver subsystem (filename basis; declares `donor-port`)
- `src/d/ext_plugin/ww_event_names.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_event_manager.cpp:53-92` — filename agrees
- `src/d/ext_plugin/ww_import_gate.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_item_data.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_item_data.cpp` — filename agrees
- `src/d/ext_plugin/ww_itemmdl_dispatch.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_layer_select.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/d_com_inf_game.cpp:185-271` — filename agrees
- `src/d/ext_plugin/ww_misc_dispatch.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_npcmount_dispatch.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_profile_register.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_room_loader.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_stage_loader.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/d/ext_plugin/ww_tsubo_data.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `d/actor/d_a_tsubo.cpp` — filename agrees
- `src/d/ext_seq/ja1_bank.cpp` — JAudio1 parallel donor stack (WW audio) (filename basis; declares `host-plumbing`)
- `src/d/ext_seq/ja1_event_dump.cpp` — JAudio1 parallel donor stack (WW audio) (filename basis; declares `host-plumbing`)
- `src/d/ext_seq/ja1_jasbank.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASBank.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasbnkparser.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASBNKParser.cpp` — filename agrees
- `src/d/ext_seq/ja1_jascalc.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASCalc.cpp` — filename agrees
- `src/d/ext_seq/ja1_jaschannel.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASChannel.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasdspchannel.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASDSPChannel.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasnotemgr.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASNoteMgr.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasouterparam.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASOuterParam.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasplayer.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASPlayer_impl.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasregisterparam.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASRegisterParam.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasseqctrl.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASSeqCtrl.cpp` — filename agrees
- `src/d/ext_seq/ja1_jasseqparser.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASSeqParser.cpp` — filename agrees
- `src/d/ext_seq/ja1_jastrack.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASTrack.cpp` — filename agrees
- `src/d/ext_seq/ja1_jastrackinterrupt.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASTrackInterrupt.cpp` — filename agrees
- `src/d/ext_seq/ja1_jastrackport.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASTrackPort.cpp` — filename agrees
- `src/d/ext_seq/ja1_kernel.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASRate.cpp` — filename agrees
- `src/d/ext_seq/ja1_native.cpp` — JAudio1 parallel donor stack (WW audio) (filename basis; declares `host-plumbing`)
- `src/d/ext_seq/ja1_oscillator.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JAudio/JASOscillator.cpp` — filename agrees
- `src/d/ext_seq/ja1_parser.cpp` — KIT-LINEAGE `bridge-owed`, KIT-DONOR `none` — filename agrees
- `src/d/ext_seq/ja1_seq_ctrl.cpp` — KIT-LINEAGE `bridge-owed`, KIT-DONOR `none` — filename agrees
- `src/d/ext_seq/ja1_track.cpp` — KIT-LINEAGE `bridge-owed`, KIT-DONOR `none` — filename agrees
- `src/d/ww_jpa.cpp` — KIT-LINEAGE `native-port`, KIT-DONOR `JSystem/JParticle/JPAEmitterLoader.cpp` — filename agrees
- `src/d/ww_jpa_bind.cpp` — WW-specific receiver subsystem (filename basis; declares `host-plumbing`)
- `src/f_op/f_op_msg_mng.cpp` — KIT-LINEAGE `mixed` — **filename rules MISS this**; donor content in a receiver-named TU

**Includes (54) — directory scan, indicative:**

- `include/d/actor/d_a_e_ww.h` — WW leg inside a receiver-named TU
- `include/d/actor/d_a_ww_demo00.h` — WW-restoration actor
- `include/d/d_albw_dialogue.h` — shared ALBW/WW dialogue surface (§113 note)
- `include/d/d_ext_dmesg.h` — WW-restoration subsystem
- `include/d/d_ext_fado_door.h` — WW-restoration subsystem
- `include/d/d_ext_mod_flags.h` — WW-restoration subsystem
- `include/d/d_ext_npc_doors.h` — WW-restoration subsystem
- `include/d/d_ext_npc_mount.h` — WW-restoration subsystem
- `include/d/d_ext_npc_population.h` — WW-restoration subsystem
- `include/d/d_ext_quick_equip.h` — WW-restoration subsystem
- `include/d/d_ext_room_verify.h` — WW-restoration subsystem
- `include/d/d_ext_save_flags.h` — WW-restoration subsystem
- `include/d/d_ext_save_flags_route.h` — WW-restoration subsystem
- `include/d/d_ext_save_guard.h` — WW-restoration subsystem
- `include/d/d_ext_scope_msg.h` — WW-restoration subsystem
- `include/d/d_ext_seq_space.h` — WW-restoration subsystem
- `include/d/d_ext_status.h` — WW-restoration subsystem
- `include/d/d_ext_tree.h` — WW-restoration subsystem
- `include/d/d_ext_ww_actor_shims.h` — WW-restoration subsystem
- `include/d/d_kankyo_ww.h` — WW leg inside a receiver-named TU
- `include/d/d_kankyo_ww_sky.h` — WW leg inside a receiver-named TU
- `include/d/d_kankyo_ww_wind.h` — WW leg inside a receiver-named TU
- `include/d/d_ww_itemmdl_pc.h` — WW-specific receiver subsystem
- `include/d/d_ww_itemmdl_test.h` — WW-specific receiver subsystem
- `include/d/ext_evt/evt1_boundary.h` — JEvent1 parallel donor stack (WW events)
- `include/d/ext_line/mdoext1_3dline.h` — MDoExt1 parallel donor stack (WW 3D-line)
- `include/d/ext_plugin/ww_cam_crawl.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_cam_data.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_cam_select.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_import_gate.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_item_data.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_itemmdl_dispatch.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_itemmdl_impl_names.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_itemmdl_test_names.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_npcmount_dispatch.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_npcmount_impl_names.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_profile_register.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_room_loader.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_stage_loader.h` — WW-specific receiver subsystem
- `include/d/ext_plugin/ww_tsubo_data.h` — WW-specific receiver subsystem
- `include/d/ext_seq/ja1_bank.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_boundary.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_dsp_boundary.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_event_dump.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_jasbank.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_jaschannel.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_jasseqparser.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_jastrack.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_native.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_oscillator.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_parser.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_seq_ctrl.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ext_seq/ja1_track.h` — JAudio1 parallel donor stack (WW audio)
- `include/d/ww_jpa.h` — WW-specific receiver subsystem

<!-- END GENERATED TIER-1 -->

**Why each cluster is covenant surface:**
- `d_ext_npc_*` / `d_a_ext_*` — the WW actor mount/population/door/vegetation
  system; names and drives WW-restoration content.
- `d_ww_itemmdl_*` — the WW held/get-item model pipeline (adapts WW arcs).
- `ext_seq/*` + `d_ext_seq_space` — the parked **audio-shadow / WW sequence**
  surface (§113 "parked audio-shadow/ext-seq").
- `custom_assets` — runtime WW arc adaptation (BDL4→BMD3 retag, audio redirect).
- `d_albw_dialogue` — **shared** with the ALBW postman, but currently drives WW
  NPC boxes; keep local until the WW usage is cleanly separable.

**NOT in this set (promotable — the public ALBW project, no WW content):** all
other `d_albw_*` (meter, combat, HUD, shop, outfit, wardrobe, wolf, potion,
boss, mail, …), the `src/dusk/mods/**` loader/SDK, `dvd_asset`. These are the
public upstream's own content; they are gated by ordinary readiness, **not** by
this covenant strip set.

### Tier 1b — Never-push PLUGIN projects (outside files.cmake, so outside the
### generated list — HAND-MAINTAINED; add every WW plugin dir here)

The generated Tier-1 derives from `files.cmake` and CANNOT see mod/plugin
projects that build via `add_mod()` — they are separate CMake targets. This
subsection is their registry. Same covenant: fork-local, never pushed.

- `mods-src/ww_donor_disc/**` — L2 donor-disc reader plugin (L2a re-home, bus
  §670/§671). Serves donor files from the user's own GZLE01 image via the
  overlay service; carries donor identity strings (`bmgres`, stage names) that
  the M6 exe-greplist does NOT scan (it scans `dusklight.exe`, not `.dusk`
  packages) — which is WHY it must be listed here by hand.
- Built artifacts land in `build/**/mods/*.dusk` (untracked; never commit).
- `src/d/ext_plugin/ww_cam_data.inc` — GENERATED donor camera tables
  (gen_cam_data.py from donor d_cam_style/d_cam_type source). Outside both
  generated bases: not a files.cmake TU (it is `#include`d by
  `ww_cam_data.cpp`, which IS Tier-1) and not under `include/`. Donor data;
  never push.

## Tier 2 — Modified-TP files carrying WW references (the dangerous class)

These are **TP's own files**, edited for WW work — they look promotable but
carry WW identity/place references in comments or edits. They **cannot be
blanket-stripped** (they hold legitimate TP content). Rule: **scrub every WW
reference before ANY promotion**, and never promote the WW-specific edit blocks.

| File | Exposure (2026-07-25) |
|---|---|
| `src/d/actor/d_a_demo_item.cpp` | `Ivan`/`Outset` in comments (get-item kit path) |
| `src/d/actor/d_a_grass.cpp` | WW grass-interaction edits + refs |
| `src/d/actor/d_a_knob00.cpp` | WW door-knob edits + refs |
| `src/d/d_camera.cpp` | `Outset` / WW-space camera refs |
| `src/d/d_kankyo_rain.cpp` | `Outset` / WW env refs |
| `CMakeLists.txt` | fork-local WW plugin build hunk (`mods-src/ww_donor_disc`, guarded on `NOT DUSK_EXCLUDE_WW`) — strip the whole block before any promotion |

Re-run the locator before any promotion, it drifts:
`git grep -ilnE '\b(Ivan|Outset|Aryll|Tetra|Windfall|Sturgeon|Orca|Makar|Medli)\b' -- 'src/**' 'include/**' | grep -viE 'd_ext_|d_a_ext_|d_ww_itemmdl|custom_assets|ext_seq|d_albw'`

**The `Ivan` note:** `Ivan` is an invented internal WW-cast label ([[donor-export-naming]]
/ Ivan rule — no invented identity labels). It currently appears **only in
comments**, which the compiler strips, so it is exe-exempt per the covenant gate
(cookbook №119) and the M6 baseline is clean. It remains a **source-hygiene**
item: scrub it from any Tier-2 file before that file promotes to public source.

## Tier 3 — Data / arcs / mod folder

- **Repo audit: NO WW data is tracked.** No `.arc` / `.bdl` / `.dzb` / `.bmg` /
  WW dialogue catalog / `WW-Crew-Restoration` path is under git. (The
  `assets/DZDE01/res/Msg*/bmgres.h` files are **TP's own** stage headers, not WW.)
- The live mod folder `%AppData%/TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration`
  is a **separate local repo** (§108) — untouchable by the receiver merge and
  never part of a receiver push. WW arcs, `ww_dialogue_full.txt`, extracted
  assets, and `tools/ww_crew_restoration_skeleton` outputs live there, not here.
- Confirm before any push: `git ls-files | grep -iE '\.arc$|\.bdl$|\.dzb$|\.bmg$|ww_dialogue|Crew-Restoration'` → must stay **empty**.

---

## The M6 greplist gate (MANDATORY before any push)

**Run the manifest drift check FIRST** — a gate run against a stale roster tells
you nothing about the files the roster forgot:

```bash
python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --check
```
Exit 1 = a WW-owned TU is compiled but unlisted. Regenerate (`--emit-tier1`) and
re-read the diff before continuing; a new WW TU appearing here is expected, a
RECEIVER TU appearing here means a filename convention has been violated.

Then run on the exe **built from the exact tree about to be pushed** (a merge/reconfigure
can change what gets packaged — re-run, never trust a prior CLEAN):

```bash
EXE=build/windows-msvc-relwithdebinfo/dusklight.exe
strings -n 5 "$EXE" | grep -inE '\b(Ivan|Outset|Aryll|Tetra|Zelda-of-WW|Tetra|Sturgeon|Orca|Windfall|Makar|Medli|Valoo|Jabun|Great Sea|WW-Crew|bmgres|zel_00)\b'
```
- **CLEAN = zero hits.** Comments never reach the exe, so a hit means a WW name
  became a **string literal / arc name** — a hard covenant breach. Do not push.
- Shared TP/WW nouns (`Ganondorf`, `Bow`, `Bomb`, `Boomerang`, `Magic Armor`,
  `Link`, `Zelda`) are **exempt** (cookbook №119) — TP ships them natively. They
  are deliberately absent from the pattern above.

## Standing rule + verification procedure

1. **Push stays gated until: M6 CLEAN + Tier-3 empty + user's explicit go.** No
   AI instance self-clears this gate (mirrors [DO-NOT.md](DO-NOT.md) discipline).
2. Never `git add`/stage a Tier-1 file or a WW edit-block toward a public-`main`
   push. Promotion is per-slice (§5 mod-api-host-promote), reviewed, then the
   file leaves Tier 1 explicitly in this doc.
3. **Re-verify the strip set after ANY CMake reconfigure or further merge** —
   packaging can shift silently (§113.6). The M6 baseline is only valid for the
   tree it was run on.
4. Housing owns the gate RUN and this document; Housing **executes no push and
   no merge** (Engine/integration lane). Loss-protection (commit + tag) is the
   only git action in Housing's charter, and only on user request.

## Audit trail

- §113 spec: [WW Linked/ww-bridge-tool-interconnected.md](WW%20Linked/ww-bridge-tool-interconnected.md) §113; push gate: [state/mod-api-host-promote.md](state/mod-api-host-promote.md) §4.
- Pre-merge anchor (full-revert): tag `pre-dusk-api-merge` = `bfa264511c`.
- This strip set supersedes the §113 assessment-era "25 files" estimate with the
  **34** verified post-merge receiver files + the Tier-2 modified-TP class the
  estimate did not enumerate.

## PROBE REGISTRY (strip before push — grep the tag, delete the guarded blocks)

| tag | define | TUs | purpose | filed |
|---|---|---|---|---|
| `WwProbe903` | `WW_PROBE_903` | d_ev_camera.cpp | UNITRANS stall discriminators (styleTimer/transTimer/Timer-param); the CAMERA execution cannot be ported (donor 39/39 Nonmatching), so it is measured instead; tale §903 | 2026-08-14 |
| `WwProbe884` | `WW_PROBE_884` | d_a_npc_ls1.cpp, d_a_npc_zl1.cpp, d_a_npc_p1.cpp | run-134919 render triage (Aryll invisible / Tetra untextured / P1 draw-order correlation); 12-hypothesis set, tale §887 | 2026-08-13 |

Strip = delete every `#if WW_PROBE_884 … #endif` block + the `#define` + this row
(or set the define to 0 for a silenced interim build). `grep -rn WW_PROBE_884 src/`
must return nothing before any push (same for `WW_PROBE_903`).
