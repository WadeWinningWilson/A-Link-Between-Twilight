# WW-layer build exclusion — GENERATED, do not hand-edit.
# regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-cmake
# tool_sha256:      a9a77b4d2f434fb5a661964879e79e478cc2023410a46d6e7f02311fa3eb53a1
# files_cmake_sha256: 2dfc9529e916bb8931451d1427cfa41c16af94b3851bf023680b00f254aa6c2e  (EOL-normalized)
# count: 78
#
# BASIS: WHAT MOVES TO THE PLUGIN (§576) = declared donor lineage
# (native-port + bridge-owed) PLUS host-plumbing that serves the WW
# layer. Step 20 ruled the end state is a prebuilt plugin, so the
# question here is NOT 'does this contain donor code' -- that is the
# covenant question, owned by Tier-1 -- but 'does this move out of
# dusklight.exe'. A WW audio bridge moves whether or not it contains
# a single donor line. Lineage cannot answer that: `host-plumbing`
# describes an ALBW dialogue box and a WW audio bridge alike.
#
# The earlier FILENAME basis was wrong in both directions (§573);
# lineage-only then left the bridges in while removing what they call,
# which is where 30 of the 61 remaining unresolved symbols came from.
#
# KEPT — host-plumbing serving ANOTHER layer (2):
#   src/d/d_albw_dialogue.cpp  --  self-declared "NEW CODE — ALBW Port (Native Dialogue Box)" — a DIFFERENT port, not WW
#   src/d/d_ext_mod_flags.cpp  --  self-declared "WW-agnostic" twice — general mod flag/quest infrastructure
#
# UNREVIEWED host-plumbing (8) — KEPT by default.
# If one of these is a WW bridge the build FAILS TO LINK, which is
# loud; the opposite default drops receiver code silently.
#   libs/JSystem/src/JAudio2/JASChannel.cpp
#   src/d/d_event_data.cpp
#   src/d/d_event_manager.cpp
#   src/d/ext_plugin/ww_itemmdl_dispatch.cpp
#   src/d/ext_plugin/ww_misc_dispatch.cpp
#   src/d/ext_plugin/ww_npcmount_dispatch.cpp
#   src/dusk/boot_stage.cpp
#   src/f_pc/f_pc_profile_lst.cpp
#
# STILL PARTIAL, and for a reason no basis can fix: `mixed` TUs are
# donor lines INSIDE receiver-owned files. Dropping them would remove
# the receiver, so they are listed below as LEG-STRIP REQUIRED and
# left in the build. Excluding this set does NOT make the build
# WW-free -- it removes the separable stacks. Step 19 owns the rest.
#
# LEG-STRIP REQUIRED (12 mixed TUs, NOT excluded here):
#   src/d/actor/d_a_bg.cpp
#   src/d/actor/d_a_demo00.cpp
#   src/d/actor/d_a_swhit0.cpp
#   src/d/actor/d_a_vrbox.cpp
#   src/d/actor/d_a_vrbox2.cpp
#   src/d/d_demo.cpp
#   src/d/d_ext_dmesg.cpp
#   src/d/d_ext_save_flags.cpp
#   src/d/d_kankyo_ww.cpp
#   src/d/d_particle.cpp
#   src/d/d_stage.cpp
#   src/f_op/f_op_msg_mng.cpp
#
# Matched the OLD filename rules but declare no donor content (7),
# so they are no longer excluded. Listed so the change is auditable:
#   src/d/d_albw_dialogue.cpp
#   src/d/d_ext_mod_flags.cpp
#   src/d/ext_plugin/ww_cam_data.cpp
#   src/d/ext_plugin/ww_cam_select.cpp
#   src/d/ext_plugin/ww_itemmdl_dispatch.cpp
#   src/d/ext_plugin/ww_misc_dispatch.cpp
#   src/d/ext_plugin/ww_npcmount_dispatch.cpp
set(WW_LAYER_FILES
    src/d/actor/d_a_esa.cpp
    src/d/actor/d_a_ext_ep.cpp
    src/d/actor/d_a_ext_plank_span.cpp
    src/d/actor/d_a_ext_vegetation.cpp
    src/d/actor/d_a_kamome.cpp
    src/d/actor/d_a_kb.cpp
    src/d/actor/d_a_knob00.cpp
    src/d/actor/d_a_lamp.cpp
    src/d/actor/d_a_lwood.cpp
    src/d/actor/d_a_npc_ba1.cpp
    src/d/actor/d_a_npc_bm1.cpp
    src/d/actor/d_a_npc_jb1.cpp
    src/d/actor/d_a_npc_ls1.cpp
    src/d/actor/d_a_npc_p1.cpp
    src/d/actor/d_a_npc_zl1.cpp
    src/d/actor/d_a_obj_mshokki.cpp
    src/d/actor/d_a_obj_otble.cpp
    src/d/actor/d_a_obj_paper.cpp
    src/d/actor/d_a_obj_plant.cpp
    src/d/actor/d_a_obj_shelf.cpp
    src/d/actor/d_a_obj_toripost.cpp
    src/d/actor/d_a_spc_item01.cpp
    src/d/actor/d_a_tag_kb_item.cpp
    src/d/actor/d_a_tag_so.cpp
    src/d/actor/d_a_ww_demo00.cpp
    src/d/actor/d_a_ww_item.cpp
    src/d/actor/d_a_ww_shutter.cpp
    src/d/actor/d_a_ww_tsubo.cpp
    src/d/d_door.cpp
    src/d/d_ext_npc_doors.cpp
    src/d/d_ext_npc_mount.cpp
    src/d/d_ext_npc_population.cpp
    src/d/d_ext_room_verify.cpp
    src/d/d_ext_seq_space.cpp
    src/d/d_ext_tree.cpp
    src/d/d_ext_ww_actor_shims.cpp
    src/d/d_kankyo_ww_sky.cpp
    src/d/d_kankyo_ww_wind.cpp
    src/d/d_npc.cpp
    src/d/d_ww_itemmdl_pc.cpp
    src/d/d_ww_itemmdl_test.cpp
    src/d/ext_evt/evt1_boundary.cpp
    src/d/ext_evt/evt1_event_data.cpp
    src/d/ext_evt/evt1_event_manager.cpp
    src/d/ext_line/mdoext1_3dline.cpp
    src/d/ext_plugin/ww_cam_crawl.cpp
    src/d/ext_plugin/ww_event_names.cpp
    src/d/ext_plugin/ww_import_gate.cpp
    src/d/ext_plugin/ww_item_data.cpp
    src/d/ext_plugin/ww_layer_select.cpp
    src/d/ext_plugin/ww_profile_register.cpp
    src/d/ext_plugin/ww_room_loader.cpp
    src/d/ext_plugin/ww_stage_loader.cpp
    src/d/ext_plugin/ww_tsubo_data.cpp
    src/d/ext_seq/ja1_bank.cpp
    src/d/ext_seq/ja1_event_dump.cpp
    src/d/ext_seq/ja1_jasbank.cpp
    src/d/ext_seq/ja1_jasbnkparser.cpp
    src/d/ext_seq/ja1_jascalc.cpp
    src/d/ext_seq/ja1_jaschannel.cpp
    src/d/ext_seq/ja1_jasdspchannel.cpp
    src/d/ext_seq/ja1_jasnotemgr.cpp
    src/d/ext_seq/ja1_jasouterparam.cpp
    src/d/ext_seq/ja1_jasplayer.cpp
    src/d/ext_seq/ja1_jasregisterparam.cpp
    src/d/ext_seq/ja1_jasseqctrl.cpp
    src/d/ext_seq/ja1_jasseqparser.cpp
    src/d/ext_seq/ja1_jastrack.cpp
    src/d/ext_seq/ja1_jastrackinterrupt.cpp
    src/d/ext_seq/ja1_jastrackport.cpp
    src/d/ext_seq/ja1_kernel.cpp
    src/d/ext_seq/ja1_native.cpp
    src/d/ext_seq/ja1_oscillator.cpp
    src/d/ext_seq/ja1_parser.cpp
    src/d/ext_seq/ja1_seq_ctrl.cpp
    src/d/ext_seq/ja1_track.cpp
    src/d/ww_jpa.cpp
    src/d/ww_jpa_bind.cpp
)
