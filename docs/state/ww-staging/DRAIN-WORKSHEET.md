# TRACKER DRAIN WORKSHEET (GENERATED - do not hand-edit)

> Regenerate: `python tools/foundry/drain_worksheet.py --write`.
> One line per LEGACY-UNROWED TU; it leaves this sheet the day a
> tracker row carries its `tu:` field. Scope and split come from
> `ww_ratchet` ITSELF (imported, not copied) so this sheet cannot
> disagree with the drain readout. `lineage` is parsed LIVE from
> each TU's own KIT-LINEAGE header - never stored (fifth-roster).
>
> **USER ORDER 2026-08-16: the drain is a driven campaign, not an
> accretion.** Done = this file lists ZERO TUs.

Standing state: **ROWED 96 · LEGACY-UNROWED 0 · floor 6** (of 102)

## LEGACY-UNROWED - the campaign

| # | TU | lineage (live) |
|---|---|---|

## ROWED - for the record

- `src/d/actor/d_a_bg.cpp`
- `src/d/actor/d_a_esa.cpp`
- `src/d/actor/d_a_ext_ep.cpp`
- `src/d/actor/d_a_ext_plank_span.cpp`
- `src/d/actor/d_a_ext_vegetation.cpp`
- `src/d/actor/d_a_kamome.cpp`
- `src/d/actor/d_a_kb.cpp`
- `src/d/actor/d_a_knob00.cpp`
- `src/d/actor/d_a_lamp.cpp`
- `src/d/actor/d_a_lwood.cpp`
- `src/d/actor/d_a_npc_ba1.cpp`
- `src/d/actor/d_a_npc_bm1.cpp`
- `src/d/actor/d_a_npc_jb1.cpp`
- `src/d/actor/d_a_npc_ls1.cpp`
- `src/d/actor/d_a_npc_p1.cpp`
- `src/d/actor/d_a_npc_zl1.cpp`
- `src/d/actor/d_a_obj_lpalm.cpp`
- `src/d/actor/d_a_obj_mshokki.cpp`
- `src/d/actor/d_a_obj_otble.cpp`
- `src/d/actor/d_a_obj_paper.cpp`
- `src/d/actor/d_a_obj_plant.cpp`
- `src/d/actor/d_a_obj_shelf.cpp`
- `src/d/actor/d_a_obj_toripost.cpp`
- `src/d/actor/d_a_spc_item01.cpp`
- `src/d/actor/d_a_swhit0.cpp`
- `src/d/actor/d_a_tag_kb_item.cpp`
- `src/d/actor/d_a_tag_so.cpp`
- `src/d/actor/d_a_vrbox.cpp`
- `src/d/actor/d_a_vrbox2.cpp`
- `src/d/actor/d_a_ww_demo00.cpp`
- `src/d/actor/d_a_ww_item.cpp`
- `src/d/actor/d_a_ww_shutter.cpp`
- `src/d/actor/d_a_ww_tsubo.cpp`
- `src/d/d_demo.cpp`
- `src/d/d_door.cpp`
- `src/d/d_event_data.cpp`
- `src/d/d_event_manager.cpp`
- `src/d/d_ext_dmesg.cpp`
- `src/d/d_ext_mod_flags.cpp`
- `src/d/d_ext_npc_doors.cpp`
- `src/d/d_ext_npc_mount.cpp`
- `src/d/d_ext_npc_population.cpp`
- `src/d/d_ext_room_verify.cpp`
- `src/d/d_ext_save_flags.cpp`
- `src/d/d_ext_seq_space.cpp`
- `src/d/d_ext_tree.cpp`
- `src/d/d_ext_ww_actor_shims.cpp`
- `src/d/d_kankyo_ww.cpp`
- `src/d/d_kankyo_ww_sky.cpp`
- `src/d/d_kankyo_ww_wind.cpp`
- `src/d/d_npc.cpp`
- `src/d/d_particle.cpp`
- `src/d/d_stage.cpp`
- `src/d/ext_evt/evt1_boundary.cpp`
- `src/d/ext_evt/evt1_event_data.cpp`
- `src/d/ext_evt/evt1_event_manager.cpp`
- `src/d/ext_line/mdoext1_3dline.cpp`
- `src/d/ext_plugin/ww_cam_crawl.cpp`
- `src/d/ext_plugin/ww_cam_data.cpp`
- `src/d/ext_plugin/ww_cam_select.cpp`
- `src/d/ext_plugin/ww_event_names.cpp`
- `src/d/ext_plugin/ww_import_gate.cpp`
- `src/d/ext_plugin/ww_item_data.cpp`
- `src/d/ext_plugin/ww_layer_select.cpp`
- `src/d/ext_plugin/ww_misc_dispatch.cpp`
- `src/d/ext_plugin/ww_npcmount_dispatch.cpp`
- `src/d/ext_plugin/ww_profile_register.cpp`
- `src/d/ext_plugin/ww_room_loader.cpp`
- `src/d/ext_plugin/ww_stage_loader.cpp`
- `src/d/ext_plugin/ww_tsubo_data.cpp`
- `src/d/ext_seq/ja1_bank.cpp`
- `src/d/ext_seq/ja1_event_dump.cpp`
- `src/d/ext_seq/ja1_jasbank.cpp`
- `src/d/ext_seq/ja1_jasbnkparser.cpp`
- `src/d/ext_seq/ja1_jascalc.cpp`
- `src/d/ext_seq/ja1_jaschannel.cpp`
- `src/d/ext_seq/ja1_jasdspchannel.cpp`
- `src/d/ext_seq/ja1_jasnotemgr.cpp`
- `src/d/ext_seq/ja1_jasouterparam.cpp`
- `src/d/ext_seq/ja1_jasplayer.cpp`
- `src/d/ext_seq/ja1_jasregisterparam.cpp`
- `src/d/ext_seq/ja1_jasseqctrl.cpp`
- `src/d/ext_seq/ja1_jasseqparser.cpp`
- `src/d/ext_seq/ja1_jastrack.cpp`
- `src/d/ext_seq/ja1_jastrackinterrupt.cpp`
- `src/d/ext_seq/ja1_jastrackport.cpp`
- `src/d/ext_seq/ja1_kernel.cpp`
- `src/d/ext_seq/ja1_native.cpp`
- `src/d/ext_seq/ja1_oscillator.cpp`
- `src/d/ext_seq/ja1_parser.cpp`
- `src/d/ext_seq/ja1_seq_ctrl.cpp`
- `src/d/ext_seq/ja1_track.cpp`
- `src/d/ww_jpa.cpp`
- `src/d/ww_jpa_bind.cpp`
- `src/f_op/f_op_msg_mng.cpp`
- `src/f_pc/f_pc_profile_lst.cpp`

## Floor (never migrates - adjudicated ownership map)

- `src/d/actor/d_a_demo00.cpp`
- `src/d/d_albw_dialogue.cpp`
- `src/d/d_ww_itemmdl_pc.cpp`
- `src/d/d_ww_itemmdl_test.cpp`
- `src/d/ext_plugin/ww_itemmdl_dispatch.cpp`
- `src/dusk/boot_stage.cpp`
